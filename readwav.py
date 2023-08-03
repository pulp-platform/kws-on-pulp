import soundfile as sf
from scipy.io import wavfile

def main():

    sf_loader, _ = sf.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/ch01_crop_meeting_1s.wav')
    print (sf_loader)

    
    samplerate, data = wavfile.read('/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/ch01_crop_meeting_1s.wav')
    print (data)


    with open('ch01_crop_meeting_1s.txt', 'w') as fp:
        for item in data:
            fp.write(str(item)+ ", ")
    print('Done')


if __name__ == "__main__":
    main()