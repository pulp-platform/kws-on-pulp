# Copyright (C) 2021-2024 ETH Zurich
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
# Author: Cristian Cioflan, ETH Zurich (cioflanc@iis.ee.ethz.ch)


import numpy as np
import argparse
from scipy.io import wavfile


def main():

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('--file', type=str, default='file.wav', help = 'Audio file to convert')
    parser.add_argument('--sdk', type=str, default='pulp_sdk', help = 'Target SDK')
    args = vars(parser.parse_args())

    # Read WAV
    fs, data = wavfile.read(args['file'])
    size = len(data)

    # Declare WAV array in header
    f = open("wav.h", "w")    
    f.write("// This is a header file containing the data in "+str(args['file'])+"\n")
    if (args['sdk'] == 'gap_sdk'):
        f.write("L2_DATA int16_t L2_wav_input["+str(size)+"] = {\n")
    elif (args['sdk'] == 'pulp_sdk'):
        f.write("PI_L2 int16_t L2_wav_input["+str(size)+"] = {\n")

    # Write data
    for i in range (0, size-1):
        f.write("    " + str(data[i])+ ",\n")
    f.write("    " + str(data[size-1])+ "\n")
    f.write("};")

    f.close()

if __name__=="__main__":
    main()

