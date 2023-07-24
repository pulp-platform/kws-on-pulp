# Copyright (C) 2021 ETH Zurich
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# SPDX-License-Identifier: Apache-2.0
# ==============================================================================
#
# Author: Cristian Cioflan, ETH (cioflanc@iis.ee.ethz.ch)


import dataset
import torch
import csv

from utils import confusion_matrix, npy_to_txt
from copy import deepcopy

import torch.nn.functional as F
import numpy as np

from torch.utils.data import DataLoader


class Train():

    def __init__(self, audio_processor, training_parameters, model, device):
        self.audio_processor = audio_processor
        self.training_parameters = training_parameters
        self.model = model
        self.device = device

        # Training hyperparameters
        self.criterion = torch.nn.CrossEntropyLoss()
        # intitial_lr = 0.001 # ORIGINAL
        intitial_lr = 0.0001 # GVSOC
        # self.optimizer = torch.optim.Adam(model.parameters(), lr = intitial_lr) # ORIGINAL
        self.optimizer = torch.optim.SGD(model.parameters(), lr = intitial_lr) # GVSOC
        lambda_lr = lambda epoch: 1 if epoch<15 else 1/5 if epoch < 25 else 1/10 if epoch<35 else 1/20 # ORIGINAL
        self.scheduler = torch.optim.lr_scheduler.LambdaLR(self.optimizer, lambda_lr)


    def evaluate_mfcc(self, model = None, mode='validation', paths = [], labels = [], statistics = False, integer = False, save = False):
        # Validate model

        training_parameters = self.training_parameters
        training_parameters['batch_size'] = 1
        data = dataset.AudioGenerator(mode, self.audio_processor, training_parameters)
        model.eval()  

        correct = 0
        total = 0

        with torch.no_grad():
            # inputs, labels = data[0]

            inputs = []

            mfcc = []
            # read file
            for path in paths:
                with open(path, 'r') as fd:
                    reader = csv.reader(fd)
                    for row in reader:
                        mfcc.append([int(x) for x in row[:490]])

            inputs = torch.FloatTensor(mfcc)
            print (inputs.shape)

            labels = labels

            inputs = torch.reshape(inputs, (-1,1,49,10)).to(self.device)
            # inputs = torch.Tensor(inputs[:,None,:,:]).to(self.device)
            labels = torch.Tensor(labels).long().to(self.device)
            model = model.to(self.device)  

            if (integer):
                model = model.cpu()
                inputs = inputs * 255./255 
                inputs = inputs.type(torch.uint8).type(torch.float).cpu()           

            if (save):
                model = model.cpu()
                inputs = inputs.type(torch.uint8).type(torch.float).cpu()
                outputs = F.softmax(model(inputs, save), dim=1)
                outputs = outputs.to(self.device)
                npy_to_txt(-1, inputs.int().cpu().detach().numpy())
            else:
                outputs = F.softmax(model(inputs), dim=1)
                outputs = outputs.to(self.device)

            _, predicted = torch.max(outputs, 1)

            # if (mode == 'tinytest'):
            #     print ("labels: ", str(labels))
            #     print ("predictions: ", str(predicted))

            total += labels.size(0)
            correct += (predicted == labels).sum().item()

            if statistics == True:
                conf_matrix(labels, predicted, self.training_parameters)

        print('Accuracy of the network on the %s set: %.2f %%' % (mode, 100 * correct / total))
        return(100 * correct / total)

        
    def validate(self, model = None, mode='validation', batch_size=-1, statistics = False, integer = False, save = False):
        # Validate model

        data = dataset.AudioGenerator(mode, self.audio_processor, self.training_parameters)

        if (batch_size != -1):
            val_dataloader = DataLoader(data, batch_size = batch_size, shuffle=False)
        else:
            val_dataloader = DataLoader(data, batch_size = self.training_parameters['batch_size'], shuffle=False)


        model.eval()  
        model = model.to(self.device)  

        correct = 0
        total = 0
        minibatch = 0

        for minibatch, input in enumerate(val_dataloader):
            with torch.no_grad():


                inputs, labels = input[0], input[1]

                permutation = torch.randperm(len(inputs))    
                indices = permutation[:]

                batched_inputs, batched_labels = inputs[indices].to(self.device), labels[indices].to(self.device)

                if (save):
                    f = open('batched_inputs_float.txt', "a")
                    for elem in torch.flatten(batched_inputs).cpu().detach().numpy():
                        f.write (str(elem) + ",\\\n")
                    f.write ("--------------------------------------------------"+ "\\\n")
                    f.close()

                if (integer):
                    model = model.cpu()
                    batched_inputs = batched_inputs * 255./255 
                    batched_inputs = batched_inputs.type(torch.uint8).type(torch.float).cpu()           

                if (save):
                    model = model.cpu()
                    batched_inputs = batched_inputs.type(torch.uint8).type(torch.float).cpu()

                    f = open('batched_inputs_int.txt', "a")
                    for elem in torch.flatten(batched_inputs).cpu().detach().numpy():
                        f.write (str(elem) + ",\\\n")
                    f.write ("--------------------------------------------------"+ "\\\n")
                    f.close()

                    batched_outputs = model(batched_inputs, save)

                    f = open('batched_outputs_int.txt', "a")
                    for elem in torch.flatten(batched_outputs).cpu().detach().numpy():
                        f.write (str(elem) + ",\\\n")
                    f.write ("--------------------------------------------------"+ "\\\n")
                    f.close()

                    outputs = F.softmax(batched_outputs, dim=1)

                    f = open('batched_outputs_softmax_int.txt', "a")
                    for elem in torch.flatten(outputs).cpu().detach().numpy():
                        f.write (str(elem) + ",\\\n")
                    f.write ("--------------------------------------------------"+ "\\\n")
                    f.close()

                    outputs = outputs.to(self.device)
                    npy_to_txt(-1, batched_inputs.int().cpu().detach().numpy())
                else:

                    outputs = F.softmax(model(batched_inputs), dim=1)
                    outputs = outputs.to(self.device)

                _, predicted = torch.max(outputs, 1)

                # if (mode == 'tinytest'):
                #     print ("labels: ", str(labels))
                #     print ("predictions: ", str(predicted))
                #     print ("Length: ", str(len(labels)))

                total += batched_labels.size(0)
                correct += (predicted == batched_labels).sum().item()

                if statistics == True:
                    conf_matrix(batched_labels, predicted, self.training_parameters)


                # Print information every 20 minibatches
                if minibatch % 20 == 0: 
                    print('[%3d / %3d]   accuracy: %.3f' % (minibatch, int(len(data)/self.training_parameters['batch_size']), 100 * correct / total))

            if (batch_size == 1):
                # export model only needs one element
                break


        print('Accuracy of the network on the %s set: %.2f %%' % (mode, 100 * correct / total))
        return(100 * correct / total)


    def train(self, model, mode):
        # Train model
        best_acc = 0
        
        for epoch in range(0, self.training_parameters['epochs']):

            print("Epoch: ", str(epoch))

            data = dataset.AudioGenerator(mode, self.audio_processor, self.training_parameters)
            train_dataloader = DataLoader(data, batch_size = self.training_parameters['batch_size'], shuffle=False)
            model.train()
            self.scheduler.step()

            running_loss = 0.0
            total = 0
            correct = 0   
            minibatch = 0

            for minibatch, input in enumerate(train_dataloader):

                inputs, labels = input[0], input[1]

                permutation = torch.randperm(len(inputs)) 
                indices = permutation[:]

                batched_inputs, batched_labels = inputs[indices].to(self.device), labels[indices].to(self.device)

                f = open('batched_inputs.txt', "a")
                for elem in torch.flatten(batched_inputs).cpu().detach().numpy():
                    f.write (str(elem) + ",\\\n")
                f.write ("--------------------------------------------------"+ "\\\n")
                f.close()

                # Zero out the parameter gradients after each mini-batch
                self.optimizer.zero_grad()

                # Train, compute loss, update optimizer
                model = model.to(self.device)

                # save output
                batched_outputs = model(batched_inputs)

                f = open('batched_outputs.txt', "a")
                for elem in torch.flatten(batched_outputs).cpu().detach().numpy():
                    f.write (str(elem) + ",\\\n")
                f.write ("--------------------------------------------------"+ "\\\n")
                f.close()


                # save softmax
                outputs = F.softmax(model(batched_inputs), dim=1)

                f = open('batched_softmax_outputs.txt', "a")
                for elem in torch.flatten(outputs).cpu().detach().numpy():
                    f.write (str(elem) + ",\\\n")
                f.write ("--------------------------------------------------"+ "\\\n")
                f.close()

                loss = self.criterion(outputs, batched_labels)


                # save loss
                f = open('batched_loss_outputs.txt', "a")
                f.write (str(loss.cpu().detach().numpy()) + ",\\\n")
                f.write ("--------------------------------------------------"+ "\\\n")
                f.close()

                loss.backward()
                self.optimizer.step()

                # Compute training statistics
                running_loss += loss.item()
                _, predicted = torch.max(outputs.data, 1)
                total += batched_labels.size(0)
                correct += (predicted == batched_labels).sum().item()

                if (len(data) == 100): # tinytrain
                
                    if minibatch % 1 == 0: 
                        print('[%3d / %3d] loss: %.3f  accuracy: %.3f' % (minibatch, int(len(data)/self.training_parameters['batch_size']), running_loss / 10, 100 * correct / total))
                        running_loss = 0.0
                else:
                    # Print information every 20 minibatches
                     if minibatch % 20 == 0: 
                        print('[%3d / %3d] loss: %.3f  accuracy: %.3f' % (minibatch, int(len(data)/self.training_parameters['batch_size']), running_loss / 10, 100 * correct / total))
                        running_loss = 0.0

            val_acc = self.validate(model, 'validation', 128)

            # Save best performing network
            if (val_acc > best_acc):
                best_acc = val_acc
                if (self.training_parameters['freezebb']):
                    PATH = './model_finetune_tinytrain_freezebb_nobias_acc_' + str(best_acc) + '.pth'
                else:
                    PATH = './model_finetune_tinytrain_nobias_acc_' + str(best_acc) + '.pth'
                torch.save(model.state_dict(), PATH)

        if (self.training_parameters['freezebb']):
            PATH = './model_finetune_tinytrain_freezebb_nobias.pth'
        else:
            PATH = './model_finetune_tinytrain_nobias.pth'
        torch.save(model.state_dict(), PATH)
        
