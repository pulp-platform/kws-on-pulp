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


#ifndef _LOCALUTIL_H_
#define _LOCALUTIL_H_

#include "Gap.h"
#include "application.h"

#define WAV_HEADER_SIZE 44 // bytes

static PI_L2 uint8_t header_buffer[WAV_HEADER_SIZE];
static struct pi_device fs_wav;
static void *wavfile;

int predict_float (void * array, int n_classes);

int predict (void * l2_buffer, int n_classes);

void dump_wav_open(char *filename, int width, int sampling_rate, int nb_channels, int size);

void dump_wav_write(void *data, int size);

void dump_wav_close();

void dump_data_write(char *filename, void *data, int size);

#endif /* LOCALUTIL_H */
