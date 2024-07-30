# Keyword Spotting on PULP

## Introduction

KWS on PULP is a framework for keyword spotting (KWS) targeting PULP platforms, using NEMO for quantizing the models and DORY for deployment. Parallel Ultra-Low Power (PULP) platform is an open-source efficient RISC-V architecture. The objective of keyword spotting (KWS) is to detect a set of predefined keywords within a stream of user utterances. When the goal is to deploy the keyword spotting on an embedded device, the classification accuracy of the system does not represent the only objective anymore, but instead the constrained computational resources and the time limitations have to be taken into account as well. To achieve a three-party trade-off, we developed a KWS system based on a Depthwise Separable Convolutional Neural Network (DS-CNN). The model is quantized using NEMO and deployed on PULP platforms through the usage of DORY.

## Keyword Spotting Pipeline

### Project structure 

The structure of the project is the following:

```
.
└── kws-on-pulp/
    ├── dory/
    ├── quantization/
    │   ├── nemo/
    │   ├── architectures/dscnn.py  
    │   ├── datagenerator.py  
    │   ├── dataset.py
    │   ├── main.py
    │   ├── model.py
    │   ├── models/
    │   ├── nemo.yml
    │   ├── reverb.py
    │   ├── wham_room.py
    │   ├── train.py
    │   └── utils.py
    ├── deploy_dory.sh
    ├── LICENSE
    └── README.md
```

More in detail, [NEMO](https://github.com/pulp-platform/nemo) is a library minimizing Deep Neural Networks, with the goal of deploying on ultra-low power, resource-constrained platforms. [DORY](https://github.com/pulp-platform/dory) is a tool performing automatic deployment of Deep Neural Networks on hardware-constrained devices. The `quantization/` directory contains Python scripts aimed at training and testing the model on Google Speech Commands v2 dataset, followed by quantizing said model. `deploy_dory.sh` is a Bash script used to deploy the quantized model on PULP-OPEN using PULP_SDK, on GAPUINO GAP8 using GAP_SDK or on GAP9 using GAP_SDK_PRIVATE and run it on the GVSOC from GreenWaves. 

The framework was tested on AlmaLinux release 8.8, using GCC 8.5.0 and Python 3.6.13. We recommend targetting GAP9 using `release v5.11.0` of GAP_SDK_PRIVATE. 

## Installation

### NEMO
The requirements and installation guide are available [here](https://github.com/pulp-platform/nemo).

### Quantization
To install the packages required to run the model's training (in PyTorch) and the quantization that follows, a conda environment can be created from `nemo.yml` by running:
```
conda env create -f nemo.yml
```

### DORY
The requirements and installation guide are available [here](https://github.com/pulp-platform/dory). Depending on the target platform, you will need to: a) PULP-OPEN: install [PULP SDK](https://github.com/pulp-platform/pulp-sdk): its prerequisites, the full installation, and the virtual platform. ; b) GAPUINO GAP8: install [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk): its prerequisites, the full installation, and the virtual platform. c) GAP9: install [GAP SDK PRIVATE](https://github.com/GreenWaves-Technologies/gap_sdk_private): its prerequisites, the full installation, and the virtual platform. 

## Example

### Pretrain ONNX model

Train the model, export it in FP32, and quantize it to INT8 through Nemo. Note: set the model accordingly in `example.json`.
```
cd kws-on-pulp/quantization
python main.py --config_file example.json
cd ../..
```

### [INFERENCE] Generate DORY-based C code for GAP9

```
cd kws-on-pulp/dory/
./deploy_dory.sh gap_sdk 3 gvsoc 2 DSCNN_DIR_DEST DSCNN_DIR_SRC 8 1 NEMO
```

* target sdk: gap_sdk, pulp_sdk
* highest memory level: 3, 2
* target platform: gvsoc, board
* computational unit: 0 (PULP GVSOC), 1 (GAP9 single-/multi-core), 2 (GAP9 NE16 accelerator)
* network destination directory
* network source directory
* number of cores
* number of trainable layers, deployed separately with PULP-Trainlib
* quantization tool: NEMO, Quantlab

### Projects
* Huami
* [EENAKWS](https://www.ai4europe.eu/business-and-industry/case-studies/eenakws-robotics-r2-keyword-spotting)
* [Towards On-device Domain Adaptation for Noise-Robust Keyword Spotting](https://ieeexplore.ieee.org/document/9869990)

### Contributor
Cristian Cioflan, ETH Zurich, [cioflanc@iis.ee.ethz.ch](cioflanc@iis.ee.ethz.ch)


### License
The code is released under Apache 2.0, see the LICENSE file in the root of this repository for details.
