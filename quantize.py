import torch
import argparse
import json
import os

import numpy as np

from dataclasses import dataclass, field
from functools import partial
from typing import Union, Optional
from tqdm import tqdm
from torch import nn, fx

from torch.utils.data import DataLoader
from dataset import DatasetProcessor
from datagenerator import DatasetCreator
from utils import parameter_generation
from dscnn import DSCNN

# import the DORY backend
from quantlib.backends.dory import export_net, export_dvsnet, DORYHarmonizePass
# import the PACT/TQT integerization pass
from quantlib.editing.fx.passes.pact import IntegerizePACTNetPass
from quantlib.editing.fx.util import module_of_node
from quantlib.algorithms.pact.pact_ops import *
# organize quantization functions, datasets and transforms by network
from pactnet import pact_recipe as quantize_net, get_pact_controllers as controllers_net

# TODO: Functional dataset management
mdataset = None

@dataclass
class QuantUtil:
    problem : str
    topo : str
    quantize : callable
    get_controllers : callable
    network : type
    in_shape : tuple
    eps_in : float
    D : int
    bs : int
    get_in_shape : callable
    load_dataset_fn : callable
    transform : type
    n_levels_in : int
    export_fn : callable
    code_size : int
    network_args : dict = field(default_factory=dict)
    quant_transform_args : dict = field(default_factory=dict)
#QuantUtil = namedtuple('QuantUtil', 'problem quantize get_controllers network in_shape eps_in D bs get_in_shape load_dataset_fn transform quant_transform_args n_levels_in export_fn')

# get a validation dataset from the problem name.
def get_valid_dataset(key : str, cfg : dict, quantize : str, pad_img : Optional[int] = None, clip : bool = False):
    qu = _QUANT_UTILS[key]
    load_dataset_fn = qu.load_dataset_fn

    # try:
    #     load_dataset_args = cfg['data']['valid']['dataset']['load_data_set']['kwargs']
    # except KeyError:
    #     load_dataset_args = {}
    # transform = qu.transform
    # try:
    #     transform_args = cfg['data']['valid']['dataset']['transform']['kwargs']
    # except KeyError:
    #     transform_args = {}
    # transform_args.update(qu.quant_transform_args)
    # transform_inst = transform(quantize=quantize, pad_channels=pad_img, clip=clip, **transform_args)
    # path_data = _QL_ROOTPATH.joinpath('systems').joinpath(qu.problem).joinpath('data')



    # return load_dataset_fn(partition='valid', path_data=str(path_data), n_folds=1, current_fold_id=0, cv_seed=0, transform=transform_inst, **load_dataset_args)

    return mdataset

MNISTSTATS =\
    {
        'normalize':
            {
                'mean': (0.1307, ),
                'std':  (0.3081, )
            },
        'quantize':
            {
                'min': -0.4240739,
                'max': 2.8215435,
                'eps': 0.025356385856866837
            }
    }

_MNIST_EPS = MNISTSTATS['quantize']['eps']

# batch size is per device, determined on Nvidia RTX2080. You may have to change
# this if you have different GPUs
_QUANT_UTILS = {
    'DSCNN':  QuantUtil(problem='MNIST', topo='DSCNN', quantize=quantize_net, get_controllers=controllers_net, network=DSCNN, in_shape=(1,1,49,10), eps_in=_MNIST_EPS, D=2**19, bs=256, get_in_shape=None, load_dataset_fn=DatasetProcessor.get_dataset, transform=None, quant_transform_args={'n_q':256}, n_levels_in=256, export_fn=export_net, code_size=150000)
}


# the topology directory where the specified network is defined
def get_topology_dir(key : str):
    topo = _QUANT_UTILS[key].topo
    return _QL_ROOTPATH.joinpath('systems').joinpath(get_system(key)).joinpath(topo)

# the QuantLab problem being solved by the specified network.
def get_system(key : str):
    return _QUANT_UTILS[key].problem

# the experiment config for the exp_id of the network specified by 'key'
def get_config(key : str, exp_id : int):
    config_filepath = get_topology_dir(key).joinpath(f'logs/exp{exp_id:04}/config.json')
    with open(config_filepath, 'r') as fp:
        config = json.load(fp)
    return config

def get_ckpt(key : str, exp_id : int, ckpt_id : Union[int, str]):
    ckpt_str = f'epoch{ckpt_id:03}' if ckpt_id != -1 else 'best'
    ckpt_filepath = get_topology_dir(key).joinpath(f'logs/exp{exp_id:04}/fold0/saves/{ckpt_str}.ckpt')
    return torch.load(ckpt_filepath)

def get_network(key : str, exp_id : int, ckpt_id : Union[int, str], quantized=False):
    with open('config_net_tqt_8b.json', 'r') as fp:
        cfg = json.load(fp)
    qu = _QUANT_UTILS[key]
    quant_cfg = cfg['network']['quantize']['kwargs']
    ctrl_cfg = cfg['training']['quantize']['kwargs']
    net_cfg = cfg['network']['kwargs']
    if qu.in_shape is None:
        qu.in_shape = qu.get_in_shape(cfg)
        _QUANT_UTILS[key].in_shape = qu.in_shape

    net_cfg.update(qu.network_args)
    # net = qu.network(**net_cfg)
    net = qu.network()


    print ("++++++++++++")
    print (net)
    print ("++++++++++++")

    if not quantized:
        print ("The network is not to be quantized. Returning...")
        return net.eval()
    quant_net = qu.quantize(net, **quant_cfg)

    # TODO: Load pretrained network
    # ckpt = get_ckpt(key, exp_id, ckpt_id)
    # state_dict = ckpt['net']
    # # the checkpoint may be from a nn.DataParallel instance, so we need to
    # # strip the 'module.' from all the keys
    # if all(k.startswith('module.') for k in state_dict.keys()):
    #     state_dict = {k.lstrip('module.'): v for k, v in state_dict.items()}
    # quant_net.load_state_dict(state_dict)
    # qctrls = qu.get_controllers(quant_net, **ctrl_cfg)
    # for ctrl, sd in zip(qctrls, ckpt['qnt_ctrls']):
    #     ctrl.load_state_dict(sd)

    # we don't want to train this network anymore
    return quant_net.eval()

def get_dataloader(key : str, cfg : dict, quantize : str, pad_img : Optional[int] = None, clip : bool = False):
    qu = _QUANT_UTILS[key]
    if torch.cuda.is_available():
        bs = torch.cuda.device_count() * qu.bs
    else:
        # network will be executed on CPU (not recommended!!)
        bs = 16
    ds = get_valid_dataset(key, cfg, quantize, pad_img=pad_img, clip=clip)
    return torch.utils.data.DataLoader(ds, bs)


def validate(net : nn.Module, dl : torch.utils.data.DataLoader, print_interval : int = 10, n_valid_batches : int = None):
    net = net.eval()
    # we assume that the net is on CPU as this is required for some
    # integerization passes
    if torch.cuda.is_available():
        net = net.to('cuda')
        device = 'cuda'
        if torch.cuda.device_count() != 1:
            net = nn.DataParallel(net)
    else:
        device = 'cpu'

    n_tot = 0
    n_correct = 0
    for i, (xb, yb) in enumerate(tqdm(dl)):
        yn = net(xb.to(device))
        n_tot += xb.shape[0]
        n_correct += (yn.to('cpu').argmax(dim=1) == yb).sum()
        if ((i+1)%print_interval == 0):
            print(f'Accuracy after {i+1} batches: {n_correct/n_tot}')
        if (i+1) == n_valid_batches:
            break

    print(f'Final accuracy: {n_correct/n_tot}')
    net.to('cpu')


def get_input_channels(net : fx.GraphModule):
    for node in net.graph.nodes:
        if node.op == 'call_module' and isinstance(module_of_node(net, node), (nn.Conv1d, nn.Conv2d)):
            conv = module_of_node(net, node)
            return conv.in_channels

# THIS IS WHERE THE BUSINESS HAPPENS!
def integerize_network(net : nn.Module, key : str, fix_channels : bool, dory_harmonize : bool, word_align_channels : bool, requant_node : bool = False):
    qu = _QUANT_UTILS[key]
    # All we need to do to integerize a fake-quantized network is to run the
    # IntegerizePACTNetPass on it! Afterwards, the ONNX graph it produces will
    # contain only integer operations. Any divisions in the integerized graph
    # will be by powers of 2 and can be implemented as bit shifts.
    in_shp = qu.in_shape
    int_pass = IntegerizePACTNetPass(shape_in=in_shp, eps_in=qu.eps_in, D=qu.D, n_levels_in=qu.n_levels_in, fix_channel_numbers=fix_channels, requant_node=requant_node)

    print (qu.eps_in)
    int_net = int_pass(net)
    if fix_channels:
        # we may have modified the # of input channels so we need to adjust the
        # input shape
        in_shp_l = list(in_shp)
        in_shp_l[1] = get_input_channels(int_net)
        in_shp = tuple(in_shp_l)
    if dory_harmonize:
        # the DORY harmonization pass:
        # - wraps and aligns averagePool nodes so
        #   they behave as they do in the PULP-NN kernel
        # - replaces quantized adders with DORYAdder modules which are exported
        #   as custom "QuantAdd" ONNX nodes
        dory_harmonize_pass = DORYHarmonizePass(in_shape=in_shp)
        int_net = dory_harmonize_pass(int_net)

    return int_net

def export_integerized_network(net : nn.Module, cfg : dict, key : str, export_dir : str, name : str, in_idx : int = 42, pad_img : Optional[int] = None, clip : bool = False, change_n_levels : int = None):
    qu = _QUANT_UTILS[key]
    # use a real image from the validation set
    ds = get_valid_dataset(key, cfg, quantize='int', pad_img=pad_img, clip=clip)
    test_input = ds[in_idx][0].unsqueeze(0)
    print (test_input.shape)
    if key == 'dvs_cnn':
        qu.export_fn(*net, name=name, out_dir=export_dir, eps_in=qu.eps_in, integerize=False, D=qu.D, in_data=test_input, change_n_levels=change_n_levels, code_size=qu.code_size)
    else:
        qu.export_fn(net, name=name, out_dir=export_dir, eps_in=qu.eps_in, integerize=False, D=qu.D, in_data=test_input, code_size=qu.code_size)

def export_unquant_net(net : nn.Module, cfg : dict, key : str, export_dir : str, name : str):
    out_path = Path(export_dir)
    out_path.mkdir(parents=True, exist_ok=True)
    onnx_file = f"{name}.onnx"
    onnx_path = out_path.joinpath(onnx_file)
    ds = get_valid_dataset(key, cfg, quantize='none')
    test_input = ds[42][0].unsqueeze(0)
    torch.onnx.export(net.to('cpu'),
                      test_input,
                      str(onnx_path),
                      export_params=True,
                      opset_version=10,
                      do_constant_folding=True)


# quite hacky but there is no other way

def get_new_classifier(classifier: PACTConv1d):
    new_classifier = nn.Sequential(nn.Flatten(),
                                   PACTLinear(
            in_features=classifier.in_channels*classifier.kernel_size[0],
            out_features=classifier.out_channels+1,
            bias=True,
            n_levels=classifier.n_levels,
            quantize=classifier.quantize,
            init_clip=classifier.init_clip,
            learn_clip=classifier.learn_clip,
            symm_wts=classifier.symm_wts,
            nb_std=classifier.nb_std,
            tqt=classifier.tqt,
            tqt_beta=classifier.tqt_beta,
            tqt_clip_grad=classifier.tqt_clip_grad))

    new_weights = classifier.weight.reshape(classifier.out_channels, -1)
    new_weights = torch.cat((new_weights, torch.zeros(new_weights.shape[1]).unsqueeze(0)))
    new_classifier[1].weight.data.copy_(new_weights)
    if classifier.bias is not None:
        new_classifier[1].bias.data.copy_(torch.cat((classifier.bias, torch.Tensor([0]))))
    else:
        new_classifier[1].bias.data.fill_(0)
    new_classifier[1].clip_lo = torch.nn.Parameter(torch.cat((classifier.clip_lo.squeeze(2), -torch.ones(1,1))))
    new_classifier[1].clip_hi = torch.nn.Parameter(torch.cat((classifier.clip_hi.squeeze(2), torch.ones(1,1))))
    new_classifier[1].clipping_params = classifier.clipping_params
    new_classifier[1].started = classifier.started
    return new_classifier


def main():
    
    parser = argparse.ArgumentParser()
    parser.add_argument("--net", type=str, default='DSCNNS', help='Network to quantize')
    parser.add_argument('--fix_channels', action='store_true', help='Fix channels of conv layers for compatibility with DORY')
    parser.add_argument('--no_dory_harmonize', action='store_true',
                        help='If supplied, don\'t align averagePool nodes\' associated requantization nodes and replace adders with DORYAdders')
    parser.add_argument('--word_align_channels', action='store_true',
                        help='Fix channels of conv layers so (#input_ch * #input_bits) is a multiple of 32 to work around XpulpNN HW bug')
    parser.add_argument('--requant_node', action='store_true',
                        help='Export RequantShift nodes instead of mul-add-div sequences in ONNX graph')
    parser.add_argument('--clip_inputs', action='store_true',
                        help='ghettofix to clip inputs to be unsigned')
    parser.add_argument('--config_net_file', type=str, default='config_net_tqt_8b.json', help = 'Network configuration file')
    parser.add_argument('--config_env_file', type=str, default='config_env.json', help = 'Environment configuration file')

    args = vars(parser.parse_args())

    # Parameter generation
    environment_parameters, preprocessing_parameters, training_parameters, experimental_parameters = parameter_generation(args) 

    # Device setup
    os.environ["CUDA_VISIBLE_DEVICES"] = environment_parameters['device_id']
    if torch.cuda.is_available() and environment_parameters['device'] == 'gpu':
        device = torch.device('cuda')        
    else:
        device = torch.device('cpu')
    print (torch.version.__version__)
    print (device)

    torch.manual_seed(0)
    np.random.seed(0)

    audio_processor = DatasetCreator(environment_parameters, training_parameters, preprocessing_parameters)
    # TODO: Functional dataset management
    global mdataset
    mdataset = DatasetProcessor("training", audio_processor, training_parameters, task = -1, device = device)
    mdataloader = DataLoader(mdataset, batch_size=training_parameters['batch_size'], shuffle=False, num_workers=0)

    qnet = get_network(key = args['net'], exp_id=0, ckpt_id=0, quantized=True)

    print ("*********")
    print (qnet)
    print ("*********")

    int_net = integerize_network(qnet, args['net'], args['fix_channels'], not args['no_dory_harmonize'], args['word_align_channels'], args['requant_node'])

    print ("Finished quantization.")

    if args['fix_channels']:
        pad_img = get_input_channels(int_net[0] if isinstance(int_net, tuple) else int_net)
    else:
        pad_img = None

    # if args.validate_tq:
    #     dl = get_dataloader(args['net'], exp_cfg, quantize='int', pad_img=pad_img)
    #     validate(int_net, dl, args.accuracy_print_interval, n_valid_batches=args.n_valid_batch)

    with open(args['config_net_file'], 'r') as fp:
        exp_cfg = json.load(fp)

    export_name = 'example_quantized'
    export_integerized_network(int_net, exp_cfg, args['net'], './export/', export_name, pad_img=pad_img, clip=args['clip_inputs'])
    
    # if args.export_unquant:
    #     net_unq = get_network(args['net'], exp_id, 0, quantized=False)
    #     export_unquant_net(net_unq, exp_cfg, args['net'], './export/', export_name)


if __name__ == "__main__":
    main()