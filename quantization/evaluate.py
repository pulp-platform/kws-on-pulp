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


import argparse

def main():

    parser = argparse.ArgumentParser(
                        prog='ProgramName',
                        description='What the program does',
                        epilog='Text at the bottom of help')

    parser.add_argument('--file1', type=str, default='None', required=True, help='Source file')
    parser.add_argument('--file2', type=str, default='None', required=True, help='Destination file')

    parser.add_argument('--f1_sp', type=int, default=0, help='Starting position in source file',)
    parser.add_argument('--f2_sp', type=int, default=0, help='Starting position in destination file')
    parser.add_argument('--len', type=int, default=0, required=True, help='Number of elements to compare')

    parser.add_argument('--f1_skip', type=int, default=0, help='Suffixes source file')
    parser.add_argument('--f2_skip', type=int, default=0, help='Suffixes destination file') 

    parser.add_argument('--eps_in', type=float, default=0, help='Scaling factor for int-float comparisons') 


    args = parser.parse_args()

    # read files
    with open(args.file1+".txt") as file_in:
        lines_f1 = []
        for line in file_in:
            if (line.startswith('#') or line.startswith('------')):
                continue
            skip_f1 = -args.f1_skip
            lines_f1.append(line[:skip_f1])

    with open(args.file2+".txt") as file_in:
        lines_f2 = []
        for line in file_in:
            if (line.startswith('#') or line.startswith('------')):
                continue
            skip_f2 = -args.f2_skip
            lines_f2.append(line[:skip_f2])

    # select lines
    array1 = lines_f1[args.f1_sp*args.len:(args.f1_sp+1)*args.len]
    array2 = lines_f2[args.f2_sp*args.len:(args.f2_sp+1)*args.len]

    # scaling factor
    eps_in = args.eps_in

    # compute MSE
    err = 0
    norm = 0
    for idx in range (len(array1)):
        if (args.eps_in == 0):
            err += (float(array1[idx]) - float(array2[idx])) * (float(array1[idx]) - float(array2[idx]))
            norm += float(array1[idx]) * float(array2[idx])
        else:
            err += (float(array1[idx])*args.eps_in - float(array2[idx])) * (float(array1[idx])*args.eps_in - float(array2[idx]))
            norm += float(array1[idx])*args.eps_in * float(array2[idx])

    mse = err/float(len(array1))


    # compute normalized MSE
    nmse = err/norm

    print ("MSE: ", mse)
    print ("NMSE: ", nmse)



if __name__ == "__main__":
    main()