# Keyword spotting on PULP platforms - a tutorial

Keyword Spotting (KWS) represents the task of processing an utterance and recognizing a keyword from a predefined set.
KWS is also known as closed-vocabulary automated speech recognition. 
Applications relying on KWS, such as voice-activated virtual assistants or sound source localization, target extreme-edge embedded systems, while the sensors (i.e., microphones) are also located on tinyML platforms.
It is thus natural to aim to also perform keyword spotting at the extreme edge, on platforms such as the GAP9 MCU.

Consider a Depthwise-Separable Convolutional Neural Network (DS-CNN) that you pretrained on a KWS dataset such as Google Speech Commands. 
In this tutorial we will understand the main steps required to use the pretrained network to process a 1-second input acquired on-board and to classify the utterance.

## Quantization

We will employ quantlib for this purpose.

```
python quantize.py --net DSCNN --fix_channels --word_align_channels --clip_inputs
```

The resulting quantized model, saved in .onnx format, together with the per-layer activations. A configuration file, required for hardware deployment, is additionally generated. You can find the files in `export/`.

## Deployment

We employ dory for generating the C code of our quantized network:

```
./dory_gen.sh gap_sdk 3 gvsoc 0 1 generate export 8
```

## Keyword Spotting on PULP

We deploy our network using:

```
./deploy.sh gvsoc 1 0
```
The application reads an input, computes the MFCCs, then performs inference. The parameters represent the target platform (gvsoc/board), the input source (0 - microphone/1 - .wav stored in L3), the MFCC source (0 - online computation/1 - precomputed MFCCs stored in input.h).


## TODOs

- [x] Merge conda environments
- [x] Fix Quantlib-generated network inference with cmake
- [x] Implement on-board inference. Maybe change debugger.
- [ ] Add pretrained network.
- [ ] Test quantization flow.
- [ ] Test with Gapmod 2.0 on EVK board 3.1. 
- [ ] Improve README
- [ ] Add student tasks. 

## Dependencies
- [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk_private) - 21ad5c40 (release v5.11.0)
- Note that we use [ARM-USB-OCD-H programmer](https://github.com/analogdevicesinc/openocd/blob/master/tcl/interface/ftdi/olimex-arm-usb-ocd-h.cfg)