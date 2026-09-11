#!/bin/bash

export PATH=/usr/pack/gcc-9.2.0-af/linux-x64/bin:$PATH 
export PATH=/usr/local/cuda-10.1/bin:$PATH 
export LD_LIBRARY_PATH=/usr/local/cuda-10.1/targets/x86_64-linux:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/pack/gcc-9.2.0-af/linux-x64/lib/:$LD_LIBRARY_PATH 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/cioflanc/miniconda3/pkgs/mpfr-4.0.2-hb69a4c5_1/lib/

export CC=gcc-9.2.0
export CXX=g++-9.2.0

nohup python -u main.py >> example.log 2>&1 &
sleep 5
