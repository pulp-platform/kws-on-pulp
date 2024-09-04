import torch
import torch.nn as nn
import torch.optim as optim
import dump_utils as dump
import math
import numpy

# Set device
if torch.cuda.is_available():
	device = torch.device('cuda')
else:
	device = torch.device('cpu')
# Define hyperparameters
learning_rate = 0.001
batch_size = 1
epochs = 1

# LAYER 0 SIZES
l0_in_ch = 276
l0_out_ch = 12
l0_hk = 1
l0_wk = 1
l0_hin = 1
l0_win = 1
l0_hstr = 2
l0_wstr = 2
l0_hpad = 0
l0_wpad = 0

f = open('init-defines.h', 'w')
f.write('// Layer0\n')
f.write('#define Tin_C_l0 '+str(l0_in_ch)+'\n')
f.write('#define Tout_C_l0 '+str(l0_out_ch)+'\n')
f.write('#define Tker_H_l0 '+str(l0_hk)+'\n')
f.write('#define Tker_W_l0 '+str(l0_wk)+'\n')
f.write('#define Tin_H_l0 '+str(l0_hin)+'\n')
f.write('#define Tin_W_l0 '+str(l0_win)+'\n')
f.write('#define Tout_H_l0 '+str(math.floor((l0_hin-l0_hk+2*l0_hpad+l0_hstr)/l0_hstr))+'\n')
f.write('#define Tout_W_l0 '+str(math.floor((l0_win-l0_wk+2*l0_wpad+l0_wstr)/l0_wstr))+'\n')
f.write('#define Tstr_H_l0 '+str(l0_hstr)+'\n')
f.write('#define Tstr_W_l0 '+str(l0_wstr)+'\n')
f.write('#define Tpad_H_l0 '+str(l0_hpad)+'\n')
f.write('#define Tpad_W_l0 '+str(l0_wpad)+'\n')
f.close()

f = open('init-defines.h', 'a')
f.write('\n// HYPERPARAMETERS\n')
f.write('#define LEARNING_RATE '+str(learning_rate)+'\n')
f.write('#define EPOCHS '+str(epochs)+'\n')
f.write('#define BATCH_SIZE '+str(batch_size)+'\n')
f.close()


# Simple input data 
inp = torch.div(torch.ones(l0_in_ch), 1e6).to(device)

class Sumnode():
	def __init__(self, ls):
		self.MySkipNode = ls

class Skipnode():
	def __init__(self):
		self.data = 0

	def __call__(self, x):
		self.data = x
		return self.data

class DNN(nn.Module):
	def __init__(self):
		super().__init__()
		self.l0 = nn.Linear(in_features=l0_in_ch, out_features=l0_out_ch, bias=0)

	def forward(self, x):
		x = torch.reshape(x, (-1,))
		x = self.l0(x)
		return x

# Initialize network
net = DNN().to(device)
for p in net.parameters():
	nn.init.normal_(p, mean=0.0, std=1.0)
from pathlib import Path
basedir = Path(__file__).resolve().parent.parent
net.l0.weight = torch.nn.Parameter(torch.from_numpy(numpy.load(basedir / 'data/l0w.npy')).to(device), requires_grad=True)
net.zero_grad()

# Freeze weights for sparse update


# All-ones fake label 
output_test = net(inp).to(device)
label = torch.ones_like(output_test).to(device)
f = open('io_data.h', 'w')
f.write('// Init weights\n')
f.write('#define WGT_SIZE_L0 '+str(l0_in_ch*l0_out_ch*l0_hk*l0_wk)+'\n')
f.write('PI_L2 float init_WGT_l0[WGT_SIZE_L0] = {'+dump.tensor_to_string(net.l0.weight.data)+'};\n')
f.close()

optimizer = optim.SGD(net.parameters(), lr=learning_rate, momentum=0)
loss_fn = nn.CrossEntropyLoss()

train_loss_list = []
# Train the DNN
for batch in range(epochs):
	optimizer.zero_grad()
	out = torch.nn.functional.softmax(net(inp), dim=0)
	loss = loss_fn(out.reshape(1, out.shape[0]), torch.Tensor([0]).to(device).long())
	train_loss_list.append(loss)
	loss.backward()
	optimizer.step()

train_loss = torch.tensor(train_loss_list)

# Inference once after training
out = torch.nn.functional.softmax(net(inp), dim=0)

f = open('io_data.h', 'a')
f.write('// Input and Output data\n')
f.write('#define IN_SIZE 276\n')
f.write('PI_L1 float INPUT[IN_SIZE] = {'+dump.tensor_to_string(inp)+'};\n')
out_size = (int(math.floor(l0_hin-l0_hk+2*l0_hpad+l0_hstr)/l0_hstr)) * (int(math.floor(l0_win-l0_wk+2*l0_wpad+l0_wstr)/l0_wstr)) * l0_out_ch
f.write('#define OUT_SIZE '+str(out_size)+'\n')
f.write('PI_L2 float REFERENCE_OUTPUT[OUT_SIZE] = {'+dump.tensor_to_string(out)+'};\n')
f.write('PI_L1 float LABEL[OUT_SIZE] = {'+dump.tensor_to_string(label)+'};\n')
f.write('PI_L2 float TRAIN_LOSS['+str(epochs)+'] = {'+dump.tensor_to_string(train_loss)+'};\n')
f.close()
