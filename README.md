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

python quantize_victor.py # more mature
```

We obtain a quantized model, saved in .onnx format, together with the per-layer activations. A configuration file, required for hardware deployment, is additionally generated. You can find the files in `export/`.

## Deployment

We employ dory for generating the C code of our quantized network:

```
./dory_gen.sh gap_sdk 3 gvsoc 0 1 generate export 8
```
The parameters represent, in order:
- the target SDK
- the highest memory available in the hierarchy (L2/L3)
- the target platform
- the mfcc computation (online/offline)
- the main computational unit. 0 - PULP GVSOC, 1 - GAP9 cluster (recommended), 2 - GAP9 NE16 accelerator (recommended)
- the destination directory for the generated code
- the source directory of the pretrained network
- the number of cores to perform inference on


## Keyword Spotting on PULP

We deploy our network using:

```
./deploy.sh gvsoc 0 0
```
The application reads an input, computes the MFCCs, then performs inference. The parameters represent, in order:
- the target platform (gvsoc/board). Make sure to selec the desired platform in the configmenu as well. The other options (e.g., Gapmod v1.0, EVK board 1.3) can stay as they are.
- the input source (0 - microphone/1 - .wav stored in L3). For on-board data acquisition, make sure to place a jumper on J7 and a jumper on PIN 1-2 of CN9.
- the MFCC source (0 - online computation/1 - precomputed MFCCs stored in input.h).


## TODOs

- [x] Merge conda environments
- [x] Fix Quantlib-generated network inference with cmake
- [x] Implement on-board inference. Maybe change debugger.
- [x] Add pretrained network.
- [x] Tested backbone inference and classification with Gapmod 2.0 on EVK board 3.1.
- [ ] Fix quantization accuracy drop.
- [x] Fix data acquision on EVK board 3.1. 
- [x] Fix .wav saving with release v5.17.0 of GAP SDK.
- [ ] Create VM Ubuntu 22.04 with preinstalled GAP SDK v5.17.0, the current repository, and (some) keyword spotting data for quantization calibration and validation.
- [ ] Improve README.
- [ ] Add student tasks. 

## Dependencies
### Gapmod 1.0 with EVK board 1.2
- [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk_private) - 21ad5c40 (release v5.11.0)
- [ARM-USB-OCD-H programmer](https://github.com/analogdevicesinc/openocd/blob/master/tcl/interface/ftdi/olimex-arm-usb-ocd-h.cfg)
### Gapmod 2.0 with EVK board 3.1
- [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk_private) - 6b88f1e2 (release v5.17.0)
- Direct USB programming

## Authors
* Cristian Cioflan <<a href="mailto:cioflanc@iis.ee.ethz.ch">cioflanc@iis.ee.ethz.ch</a>>

Special thanks to Philip Wiese for publishing [this example](https://github.com/pulp-platform/quantlab/tree/main/examples/fx_integerization).

## License

The code is released under Apache 2.0, see the LICENSE file in the root of this repository for details.