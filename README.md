# Keyword Spotting on PULP

## Introduction

KWS on PULP is a framework for keyword spotting (KWS) targeting PULP platforms, using NEMO for quantizing the models and DORY for deployment. Parallel Ultra-Low Power (PULP) platform is an open-source efficient RISC-V architecture. The objective of keyword spotting (KWS) is to detect a set of predefined keywords within a stream of user utterances. When the goal is to deploy the keyword spotting on an embedded device, the classification accuracy of the system does not represent the only objective anymore, but instead the constrained computational resources and the time limitations have to be taken into account as well. To achieve a three-party trade-off, we developed a KWS system based on a Depthwise Separable Convolutional Neural Network (DS-CNN). The model is quantized using NEMO and deployed on PULP platforms through the usage of DORY.

## Keyword Spotting Pipeline

### Project structure 

The project's structure is the following:

```
.
└── kws-on-pulp/
    ├── dory/
    │   ├── dory_examples/
    │   ├── images/
    │   ├── pulp-nn/
    │   ├── pulp-nn-1d/
    │   ├── pulp-nn-mixed/
    │   ├── templates/
    │   ├── LICENSE.txt
    │   ├── Model_deployment.py
    │   ├── ONNX_management.py
    │   ├── README.md
    │   ├── template.py
    │   └── tiling.py
    ├── nemo/
    │   ├── doc/
    │   ├── nemo/
    │   ├── tests/
    │   ├── var/
    │   ├── LICENSE
    │   ├── README.md
    │   ├── requirements.txt
    │   └── setup.py
    ├── quantization/
    │   ├── dataset.py
    │   ├── environment.yml
    │   ├── main.py
    │   ├── model.py
    │   ├── train.py
    │   └── utils.py
    ├── application/
    │   ├── main.c
    │   ├── Makefile
    │   ├── MfccModel.c
    │   └── MfccModel.mk
    ├── deploy.sh
    ├── LICENSE
    └── README.md
```

More in detail, [NEMO](https://github.com/pulp-platform/nemo) is a library minimizing Deep Neural Networks, with the goal of deploying on ultra-low power, resource-constrained platforms. [DORY](https://github.com/pulp-platform/dory) is a tool performing automatic deployment of Deep Neural Networks on hardware-constrained devices. The `quantization/` directory contains Python scripts aimed at training and testing the model on Google Speech Commands v2 dataset, followed by quantizing said model. `application/` directory contains the files required to generate the source code to compute the input features (MFCC) on the target platforms, relying on Greenwaves' autotiler; it also contains the source code to test together the MFCC computation and the model inference. `deploy.sh` is a Bash script used to deploy the quantized model on PULP-OPEN using PULP_SDK or on GAPUINO GAP8 using GAP_SDK or and run it on the GVSOC from GreenWaves.
The framework was tested on CentOS 7.6.1810, using GCC 4.9.1 and Python 3.6.13. 

### Installation

#### NEMO
The requirements and installation guide are available [here](https://github.com/pulp-platform/nemo).

#### DORY
The requirements and installation guide are available [here](https://github.com/pulp-platform/dory). In order to use the MFC coefficients as inputs, use the ```input_features``` branch. Depending on the target platform, you will need to: a) PULP-OPEN: install [PULP SDK](https://github.com/pulp-platform/pulp-sdk): its prerequisites, the full installation, and the virtual platform. ; b) GAPUINO GAP8: install [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk): its prerequisites, the full installation, and the virtual platform. 

#### Quantization
To install the packages required to run the model's training (in PyTorch) and the quantization that follows, a conda environment can be created from `environment.yml` by running:
```
conda env create -f environment.yml
```
#### MFCC computation

The on-device MFCC computation relies on Greenwaves autotiler, which is not part of this distribution. To make use of it, you will need to install [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk): its prerequisites and the autotiler (```make autotiler```). The framework has been tested using ```release-v4.0.0```tag.

### Example
To run the main script, use the command:
```
python main.py
```

To deploy the KWS system on the target device, use the command:
```
./deploy.sh sdk
```
For instance, to deploy the model using MFCC inputs on PULP-OPEN using PULP-SDK and to test it on GVSOC, use the command:
```
./deploy.sh pulp_sdk
```

### Contributor
Cristian Cioflan, ETH Zurich, [cioflanc@iis.ee.ethz.ch](cioflanc@iis.ee.ethz.ch)


### License
The code is released under Apache 2.0, see the LICENSE file in the root of this repository for details.
