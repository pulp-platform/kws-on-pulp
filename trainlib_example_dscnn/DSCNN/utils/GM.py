import torch
import torch.nn as nn
import torch.optim as optim
import dump_utils as dump
import math

# LAYER 0 SIZES
l0_in_ch = 64
l0_out_ch = 12
l0_hk = 1
l0_wk = 1
l0_hin = 1
l0_win = 1
l0_hstr = 1
l0_wstr = 1
l0_hpad = 0
l0_wpad = 0

f = open('initdefines.h', 'w')
f.write('#ifndef INITDEFINES_H\n')
f.write('#define INITDEFINES_H\n')
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

# Define hyperparameters
learning_rate = 0.5
batch_size = 1
epochs = 1

f = open('initdefines.h', 'a')
f.write('\n// HYPERPARAMETERS\n')
f.write('#define LEARNING_RATE '+str(learning_rate)+'\n')
f.write('#define EPOCHS '+str(epochs)+'\n')
f.write('#define BATCH_SIZE '+str(batch_size)+'\n')
f.write('#endif /* INITDEFINES_H */\n')
f.close()


# Simple input data 
inp = torch.div(torch.ones(l0_in_ch), 100000)
class DNN(nn.Module):
	def __init__(self):
		super().__init__()
		self.l0 = nn.Linear(in_features=l0_in_ch, out_features=l0_out_ch, bias=False)

	def forward(self, x):
		x = torch.reshape(x, (-1,))
		x = self.l0(x).float()
		return x

# Initialize network
net = DNN()
for p in net.parameters():
	nn.init.normal_(p, mean=0.0, std=1.0)
net.zero_grad()


# All-ones fake label 
output_test = net(inp)
label = torch.ones_like(output_test)
f = open('iodata.h', 'w')
f.write('#ifndef IODATA_H\n')
f.write('#define IODATA_H\n')
f.write('// Init weights\n')
f.write('#define WGT_SIZE_L0 '+str(l0_in_ch*l0_out_ch*l0_hk*l0_wk)+'\n')
f.write('PI_L2 float init_WGT_l0[WGT_SIZE_L0];\n')
f.close()

f = open('iodata.c', 'w')
f.write('#include "iodata.h"\n')
f.write('// Init weights\n')
f.write('init_WGT_l0[WGT_SIZE_L0] = {'+dump.tensor_to_string(net.l0.weight.data)+'};\n')
f.close()

optimizer = optim.SGD(net.parameters(), lr=learning_rate, momentum=0)
loss_fn = nn.MSELoss()

# Train the DNN
for batch in range(epochs):
	optimizer.zero_grad()
	out = net(inp)
	loss = loss_fn(out, label)
	loss.backward()
	optimizer.step()

# Inference once after training
out = net(inp)

f = open('iodata.h', 'a')
f.write('// Input and Output data\n')
f.write('#define IN_SIZE 64\n')
f.write('PI_L1 float IN_DATA[IN_SIZE];\n')
out_size = (int(math.floor(l0_hin-l0_hk+2*l0_hpad+l0_hstr)/l0_hstr)) * (int(math.floor(l0_win-l0_wk+2*l0_wpad+l0_wstr)/l0_wstr)) * l0_out_ch
f.write('#define OUT_SIZE '+str(out_size)+'\n')
f.write('PI_L2 float REFERENCE_OUTPUT[OUT_SIZE];\n')
f.write('PI_L1 float LABEL[OUT_SIZE];\n')
f.write('#endif /* IODATA_H */\n')
f.close()
f = open('iodata.c', 'a')
f.write('IN_DATA[IN_SIZE] = {'+dump.tensor_to_string(inp)+'};\n')
f.write('REFERENCE_OUTPUT[OUT_SIZE] = {'+dump.tensor_to_string(out)+'};\n')
f.write('LABEL[OUT_SIZE] = {'+dump.tensor_to_string(label)+'};\n')
f.close()
