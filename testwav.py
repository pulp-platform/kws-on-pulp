import soundfile as sf
import numpy as np
from scipy.io import wavfile


def main():

    sf_loader, _ = sf.read('build/test_gap.wav')
    print ("------------------- float64 -------------------")
    print ("mean: ", np.mean(sf_loader))
    print ("max: ", np.max(sf_loader))
    print ("min: ", np.min(sf_loader))
    print ("median: ", np.median(sf_loader))
    print ("power", np.sum(np.multiply(sf_loader, sf_loader))/len(sf_loader))
    print ("energy", np.sum(np.multiply(sf_loader, sf_loader)))


    wf_sr, wf_loader = wavfile.read('build/test_gap.wav')
    print ("------------------- int16 -------------------")
    print ("mean: ", np.mean(wf_loader))
    print ("max: ", np.max(wf_loader))
    print ("min: ", np.min(wf_loader))
    print ("median: ", np.median(wf_loader))
    print ("multiply: ", np.multiply(wf_loader, wf_loader))
    print ("sum: ",  np.sum(np.multiply(wf_loader, wf_loader)))
    print ("power", np.sum(np.multiply(wf_loader, wf_loader))/len(wf_loader))
    print ("energy", np.sum(np.multiply(wf_loader, wf_loader)))




    # right_94de6a6a_nohash_4 as float64 with soundfile read
    # mean:  5.201148986816406e-05
    # max:  0.789398193359375
    # min:  -0.583038330078125
    # median:  0.0
    # power 0.005667186734324787
    # energy 90.67498774919659

    # right_94de6a6a_nohash_4 as int16 with wavfile read
    # mean:  1.7043125
    # max:  25867
    # min:  -19105
    # median:  0.0
    # power 729.0850625
    # energy 11665361


    # test_gap.wav as float64 with soundfile read
    # mean:  -0.0009324932098388672
    # max:  0.09576416015625
    # min:  -0.073486328125
    # median:  -0.002685546875
    # power 0.00025094540970167144
    # energy 4.015126555226743

    # test_gap.wav as int16 with wavfile read read
    # mean:  -30.5559375
    # max:  3138
    # min:  -2408
    # median:  -88.0
    # multiply:  [     0    100  18225 ... -31744  12569 -20592]
    # sum:  92132703
    # power 5758.2939375
    # energy 92132703


    # norm = (denorm - min) / (max - min)
    # denorm = norm * (max-min) + min

    print ("------------------- Norm -------------------")

    bmina = (np.max(sf_loader) - np.min(sf_loader))
    # bmina = 2
    scaler = (wf_loader - np.min(wf_loader).astype(np.float64))/(np.max(wf_loader).astype(np.float64) - np.min(wf_loader).astype(np.float64))
    scaled = bmina * scaler


    wf_loader_norm = scaled + np.min(sf_loader)
    # wf_loader_norm = scaled + 1

    print ("mean: ", np.mean(wf_loader_norm))
    print ("max: ", np.max(wf_loader_norm))
    print ("min: ", np.min(wf_loader_norm))
    print ("median: ", np.median(wf_loader_norm))
    print ("power", np.sum(np.multiply(wf_loader_norm, wf_loader_norm))/len(wf_loader_norm))
    print ("energy", np.sum(np.multiply(wf_loader_norm, wf_loader_norm)))


    print ("------------------- Denorm -------------------")

    denorm_scaler = (int(np.max(wf_loader)) - int(np.min(wf_loader))) / (np.max(sf_loader) - np.min(sf_loader))
    denormer = (sf_loader - np.min(sf_loader)) * denorm_scaler + int(np.min(wf_loader))

    print ("mean: ", np.mean(denormer))
    print ("max: ", np.max(denormer))
    print ("min: ", np.min(denormer))
    print ("median: ", np.median(denormer))
    print ("multiply: ", np.multiply(denormer, denormer))

    print ("diff: ", np.sum(np.subtract(wf_loader, denormer)))

    print ("wf_loader_type: ", wf_loader.dtype)
    print ("denormer_type: ", denormer.dtype)

    print ("sum int16: ",  np.sum(np.multiply(denormer.astype(np.int16), denormer.astype(np.int16))))
    print ("sum int32: ",  np.sum(np.multiply(denormer.astype(np.int32), denormer.astype(np.int32))))
    print ("sum float32: ",  np.sum(np.multiply(denormer.astype(np.float32), denormer.astype(np.float32))))
    print ("sum float64: ",  np.sum(np.multiply(denormer, denormer)))
    print ("power", np.sum(np.multiply(denormer, denormer))/len(denormer))
    print ("energy", np.sum(np.multiply(denormer, denormer)))










if __name__=="__main__":
    main()