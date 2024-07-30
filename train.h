// Copyright (C) 2023-2024 ETH Zurich
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// SPDX-License-Identifier: Apache-2.0
// ==============================================================================
//
// Author: Cristian Cioflan, ETH (cioflanc@iis.ee.ethz.ch)


#ifndef __train_H__
#define __train_H__

#include "globals.h"

#define TRAIN_EPS 5

static float ce_loss_pre;
static float ce_loss_post;
static float ce_loss_pre_val;
static float ce_loss_post_val;
static float correct_pre_val;
static float correct_post_val;

char yes[N_MFCC_MELS * N_MFCC_WINS];
char no[N_MFCC_MELS * N_MFCC_WINS]; 
char up[N_MFCC_MELS * N_MFCC_WINS];
char down[N_MFCC_MELS * N_MFCC_WINS];
char left[N_MFCC_MELS * N_MFCC_WINS];
char right[N_MFCC_MELS * N_MFCC_WINS];
char on[N_MFCC_MELS * N_MFCC_WINS];
char off[N_MFCC_MELS * N_MFCC_WINS];
char stop[N_MFCC_MELS * N_MFCC_WINS];
char go[N_MFCC_MELS * N_MFCC_WINS];

void train();

void evaluate_largetest(int was_trained);

void evaluate_tinytest(int was_trained);

void evaluate_online(int was_trained);

#endif /* __train_H__ */
