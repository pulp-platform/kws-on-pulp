# ODDA for KWS on GAP9

This project enables the deployment of a keyword spotting neural network on GAP9 for inference and training.

## Preliminary steps

### Pretrain ONNX model

```
git clone https://github.com/pulp-platform/kws-on-pulp
cd quantization
python main.py
```
### [INFERENCE] Generate DORY-based C code for GAP8

```
cd /usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/exercise6/curr/
./deploy_dscnn_pulpsdkl2.sh gap_sdk 3 gvsoc 0
```

### [INFERENCE] GAP8-GAP9 conversion

```
cd /usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/exercise6/curr/application_dscnn_gap9/
./converter.sh
cp -r /usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/exercise6/curr/application_dscnn_gap9/hex tiny_denoiser/DORY_network/
cp -r /usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/exercise6/curr/application_dscnn_gap9/inc tiny_denoiser/DORY_network/
cp -r /usr/scratch/wetterhorn/cioflanc/mlonmcu_exercise6/exercise6/curr/application_dscnn_gap9/src tiny_denoiser/DORY_network/
rm tiny_denoiser/DORY_network/src/main.c
```

## [INFERENCE] Run on GAP9

To run the network on GVSOC:
```
./deploy_gvsoc.sh
```

To run the network on GAP9 with the Evaluation Kit:
```
./deploy_gvsoc.sh
```

### [TRAIN] Generate PULP TrainLib-based C code for GAP8

WIP
=======

### Board configuration

To use the Vesper microphone on GAP9mod
* Jumper on J7
* Connect CN9.1 and CN9.2

Select:
* GAP9_EVK_AUDIO
* GWT Board: Gap9mod V1.0b & Evaluation kit (V2.0) -- only for CMakeList


