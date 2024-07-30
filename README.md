# ODDA for KWS on GAP9

This project enables the deployment of a keyword spotting neural network on GAP9 for inference and training.

## Preliminary steps

```
git submodule update --init
cd kws-on-pulp
git submodule update --init
cd dory
git submodule update --init
cd ../..
cd kws-on-gap9/
git submodule update --init
cd ..
```

### Pretrain ONNX model

Train the model, export it in FP32, and quantize it to INT8 through Nemo. Note: set the model accordingly in `example.json`.
```
cd kws-on-pulp/quantization
python main.py --config_file example.json
cd ../..
```

Alternatively, a pretrained model can be exported to FP32 and then quantized to INT8 through Quantlib. Note: set the model accordingly in `config_env.json`.
```
cd kws-on-gap9/
python quantize.py --net DSCNN --fix_channels --word_align_channels --clip_inputs --pretrained path/to/model.pth --config_net_file config_dscnn_hierarchic_tqt_8b.json
cd ..
``` 

During pretraing, the number of MFCCs can be set. They should coincide with the settings in the `MfccConfig.json`. `mfcc_bank_cnt`, `n_mels`, and `n_dct` should have the same value (e.g., 10 and 40 are tested values).

### [INFERENCE] Generate DORY-based C code for GAP9

```
cd kws-on-pulp/dory/
./deploy_dory.sh gap_sdk 3 gvsoc 2 DSCNN_DIR_DEST DSCNN_DIR_SRC 8 1 Quantlab
```

* target sdk: gap_sdk, pulp_sdk
* highest memory level: 3, 2
* target platform: gvsoc, board
* computational unit: 0 (PULP GVSOC), 1 (GAP9 single-/multi-core), 2 (GAP9 NE16 accelerator)
* network destination directory
* network source directory
* number of cores
* number of trainable layers, deployed separately with PULP-Trainlib
* quantization tool: NEMO, Quantlab

Note that Trainlib requires L1 space, which should be taken from Dory. For now you have to manually modify the `L1.dimension` in `dory/dory/Hardware_targets/PULP/GAP9/HW_description.json`. 

* DSCNN S: 110000
* DSCNN M:  90000
* DSCNN L:  60000

### [TRAIN] Generate PULP TrainLib-based C code

To generate the FP32 C code for the trainable segment of the network, run:

```
cd pulp-trainlib/
./codegen.sh ./ DSCNN_DIR_DEST DSCNN_DIR_SRC/model_fp32.onnx MatMul # generates net.{h,c}, initdefines.h, iodata.{h}
```

* network destination directory path
* network destination directory name
* pretrained model path
* name of (first) trainable layer

The content of `net.{c,h}` needs to be modified using examples in the repo. The value of `eps_in` must be changed and can be found in the network source directory (see `pretrained model path`), in `epsilon.txt` for the `PACT_IntegerAvgPool2d` layer.

## [INFERENCE] Run on GAP9

To run the network on GVSOC:
```
./deploy.sh gvsoc 1 1 1 0 WORK
```

To run the network on GAP9 with the Evaluation Kit:
```
./deploy.sh board 0 0 0 0 WORK
```

To understand the runtime parameters:
```
./deploy.sh -h
```

This currently integrates inference and user-indicated training. An inference-only mode should be ensured.

=======

### Configuration

To use the Vesper microphone on GAP9mod
* Jumper on J7
* Connect CN9.1 and CN9.2

configmenu:
* GAP9_EVK_AUDIO
* Gap9mod V1.0b
* Evaluation kit (V1.3)

On-board configurations:
* GAPmod V2.0
* EVK v3.1

Host configurations:
* GAP SDK PRIVATE, release v5.11.0
* AlmaLinux release 8.8
* GCC 8.5.0 (locally setting 9.2.0)


