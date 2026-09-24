
import argparse
import torchaudio
import torch
import soundfile as sf
import numpy as np


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("--wavfile", type=str, default="../testing/yes_563aa4e6_nohash_2.wav", help=".wav file to read.")
    parser.add_argument("--cmfcc", type=str, default="../build/mfccdump.dat", help="MFCC generated in C.")
    args = parser.parse_args()

    # Load wav
    sf_loader, _ = sf.read(args.wavfile)
    wav_file = torch.from_numpy(sf_loader).float()


    melkwargs={ 'n_fft':1024, 'win_length':640, 'hop_length':320,
                             'f_min':20, 'f_max':4000, 'n_mels':40}

    mfcc_transformation = torchaudio.transforms.MFCC(n_mfcc=10, sample_rate=16000, melkwargs=melkwargs, log_mels=True, norm='ortho')
    data = mfcc_transformation(wav_file)
    data = data[:,:49]

    # Load C-generated MFCC
    cdata = []
    with open(args.cmfcc, 'rb') as file:
        while 1:
            char = file.read(1)          
            if not char: 
                break
            cdata.append(ord(char))


    data = data.numpy().transpose().flatten()

    # Rescale
    for i in range(len(cdata)):
        cdata[i] = (cdata[i] - 128)/0.1118  # 0.1118 = (pow(2, -1) * sqrt(0.05))

    # Compare - QSNR
    qsnr = 0   
    mse = 0
    gtsum = 0 
    prsum = 0

    for i in range(len(cdata)):
        mse = mse + (data[i]-cdata[i])*(data[i]-cdata[i])
        prsum = prsum + (cdata[i])*(cdata[i])
        gtsum = gtsum + (data[i])*(data[i])

    if (gtsum < mse):
        qserrnr = -10*np.log10(mse/gtsum)
        qquantsnr = -10*np.log10(prsum/gtsum)
    else:
        qserrnr =  10*np.log10(gtsum/mse)
        qquantsnr = 10*np.log10(gtsum/prsum)
    

    print("qserrnr: ", qserrnr)
    print("qquantsnr: ", qquantsnr)





if __name__ == "__main__":
    main()


