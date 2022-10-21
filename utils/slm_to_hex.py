import numpy as np
import os
import os.path
import argparse
import sys


parser = argparse.ArgumentParser(description='generate your hex from slm')

parser.add_argument("--input", dest="input_file", default=None, help="Specify input file")

args = parser.parse_args()

if args.input_file is None:
   raise Exception('Specify the input file you monster!')

delimiter=" "

with open(args.input_file, "rU") as fi:
   data = list(map(lambda x:x.split(delimiter), fi.read().strip().split("\n")))

A=np.array(data)
print (A.shape)
print (A[0])

f = open("./out.hex", 'wb+')
for i in  range(0, A.shape[0]) :
         f.write(int( A[i][1][2]+A[i][1][3],16).to_bytes(1,'little'))
         f.write(int( A[i][1][0]+A[i][1][1],16).to_bytes(1,'little'))
f.close()
