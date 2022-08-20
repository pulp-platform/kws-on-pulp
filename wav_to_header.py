import numpy as np

import argparse

from scipy.io import wavfile




def main():

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('--file', type=str, default='file.wav', help = 'Audio file to convert')
    parser.add_argument('--sdk', type=str, default='pulp_sdk', help = 'Target SDK')
    args = vars(parser.parse_args())

    fs, data = wavfile.read(args['file'])
    size = len(data)
    f = open("wav.h", "w")
    
    f.write("// This is a header file containing the data in "+str(args['file'])+"\n")
    if (args['sdk'] == 'gap_sdk'):
        f.write("L2_DATA int16_t L2_wav_input["+str(size)+"] = {\n")
    elif (args['sdk'] == 'pulp_sdk'):
        f.write("PI_L2 int16_t L2_wav_input["+str(size)+"] = {\n")

    for i in range (0, size-1):
        f.write("    " + str(data[i])+ ",\n")
    f.write("    " + str(data[size-1])+ "\n")

    f.write("};")

    f.close()

if __name__=="__main__":
    main()

