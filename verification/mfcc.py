
import argparse
import torchaudio
import torch
import soundfile as sf


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("--wavfile", type=str, default="../res/tinytest/backward_18f8afd5_nohash_1.wav", help=".wav file to read.")
    parser.add_argument("--cmfcc", type=str, default="../build/mfccdump.data", help="MFCC generated in C.")
    args = parser.parse_args()

    # Load wav
    sf_loader, _ = sf.read(args.wavfile)
    wav_file = torch.from_numpy(sf_loader).float()


    melkwargs={ 'n_fft':1024, 'win_length':640, 'hop_length':320,
                             'f_min':20, 'f_max':4000, 'n_mels':10}

    mfcc_transformation = torchaudio.transforms.MFCC(n_mfcc=10, sample_rate=16000, melkwargs=melkwargs, log_mels=True, norm='ortho')
    data = mfcc_transformation(wav_file)

    # Load C-generated MFCC
    with open(args.cmfcc, 'rb') as file:
        while 1:
            char = file.read(1)          
            if not char: 
                break
            print (ord(char))

    # Compare



if __name__ == "__main__":
    main()


