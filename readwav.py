from scipy.io import wavfile

def main():

    # samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/ch01_crop_meeting_1s.wav')
    # samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/dude_miaowing_1s.wav')

    samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/restaurant_ch01.wav')
    wavfile.write('restaurant_ch01_mancrop.wav', samplerate, data[:16000])

    # with open('ch01_crop_meeting_1s.txt', 'w') as fp:
    #     for item in data:
    #         fp.write(str(item)+ ", ")
    # print('Done')


if __name__ == "__main__":
    main()