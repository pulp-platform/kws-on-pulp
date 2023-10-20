from scipy.io import wavfile
import random

import matplotlib.pyplot as plt

def main():

    # samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/ch01_crop_meeting_1s.wav')
    # samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/dude_miaowing_1s.wav')

    samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/build/recording_utterance.wav')

    plt.plot(data)

    plt.show()


if __name__=="__main__":
    main()