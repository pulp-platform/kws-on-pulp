/*
 * Copyright (C) 2020 GreenWaves Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "AutoTilerLib.h"
#include "AutoTilerLibTypes.h"
#include "DSP_Generators.h"
#include "MFCC_params.h"

void MFCCConfiguration(unsigned int L1Memory)
{
  SetInlineMode(ALWAYS_INLINE);
  SetSymbolDynamics();

  SetUsedFilesNames(0, 3, "MfccBasicKernels.h", "CmplxFunctions.h", "PreProcessing.h");
  SetGeneratedFilesNames("MFCCKernels.c", "MFCCKernels.h");

  SetL1MemorySize(L1Memory);
}

int main(int argc, char **argv)
{
    if (TilerParseOptions(argc, argv)) GenTilingError("Failed to initialize or incorrect output arguments directory.\n");
    CNN_GenControl_T Tensorflow_Settings;
    CNN_InitGenCtrl(&Tensorflow_Settings);

    // Set Auto Tiler configuration, given shared L1 memory is 51200
    MFCCConfiguration(112*104);
    // Load FIR basic kernels
    LoadMFCCLibrary();

    // Generate code for MFCC applied to 49 of size FRAME_SIZE with FRAME_STEP as stride
    MFCC_Generator("Tensorflow_MFCC",                    &Tensorflow_Settings, 49, FRAME_SIZE, FRAME_STEP, N_FFT, MFCC_BANK_CNT, MFCC_COEFF_CNT, N_DCT, 0, 0, 0, USE_POWER, DATA_TYPE, 1, 0);

    GenerateTilingCode();
}
