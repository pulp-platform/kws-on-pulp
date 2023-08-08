from scipy.io import wavfile
import random

def main():

    # samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/ch01_crop_meeting_1s.wav')
    # samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/dude_miaowing_1s.wav')

    samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/washing_ch01.wav')
    start1 = random.randint(0, len(data)-16000)
    start2 = random.randint(0, len(data)-16000)
    start3 = random.randint(0, len(data)-16000)
    start4 = random.randint(0, len(data)-16000)
    start5 = random.randint(0, len(data)-16000)
    wavfile.write('washing_ch01_mancrop1.wav', samplerate, data[start1:start1+16000])
    wavfile.write('washing_ch01_mancrop2.wav', samplerate, data[start2:start2+16000])
    wavfile.write('washing_ch01_mancrop3.wav', samplerate, data[start3:start3+16000])
    wavfile.write('washing_ch01_mancrop4.wav', samplerate, data[start4:start4+16000])
    wavfile.write('washing_ch01_mancrop5.wav', samplerate, data[start5:start5+16000])

    # with open('ch01_crop_meeting_1s.txt', 'w') as fp:
    #     for item in data:
    #         fp.write(str(item)+ ", ")
    # print('Done')


if __name__ == "__main__":
    main()