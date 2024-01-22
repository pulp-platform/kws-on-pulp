# ODDA for KWS on GAP9

This project enables the deployment of a keyword spotting neural network on GAP9 for inference and training.

## Preliminary steps

### Pretrain ONNX model

```
git submodule update --init
cd quantization
python main.py
cd ..
```
### [INFERENCE] Generate DORY-based C code for GAP8

```
cd deployment/dory/dory/Hardware_targets/PULP/Backend_Kernels/ && git clone git@github.com:pulp-platform/pulp-nn.git # git submodule update --init 
cd -
./deploy_dscnn_pulpsdkl2.sh gap_sdk 3 gvsoc 0
```

### [INFERENCE] GAP8-GAP9 conversion

```
cd deployment/
./convert_gap8_to_gap9.sh destination_directory ../source_directory
cd -
```

Several changes are required to stop the inference before the classifier.
TODO: List the changes.

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


