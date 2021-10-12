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
    ├── deploy.sh
    ├── LICENSE
    └── README.md
```

More in detail, [NEMO](https://github.com/pulp-platform/nemo) is a library minimizing Deep Neural Networks, with the goal of deploying on ultra-low power, resource-constrained platforms. [DORY](https://github.com/pulp-platform/dory) is a tool performing automatic deployment of Deep Neural Networks on hardware-constrained devices. The `quantization/` directory contains Python scripts aimed at training and testing the model on Google Speech Commands v2 dataset, followed by quantizing said model. `deploy.sh` is a Bash script used to deploy the quantized model on GAPUINO GAP8 and run it on the GVSOC of GAP8 from GreenWaves.
The framework was tested on CentOS 7.6.1810, using GCC 4.9.1 and Python 3.6.13.

### Installation

#### NEMO
The requirements and installation guide are available [here](https://github.com/pulp-platform/nemo).

#### DORY
The requirements and installation guide are available [here](https://github.com/pulp-platform/dory). As mentioned in the README, you will need to install [GAP SDK](https://github.com/GreenWaves-Technologies/gap_sdk): its prerequisites, the full installation, and the virtual platform.

#### Quantization
To install the packages required to run the model's training (in PyTorch) and the quantization that follows, a conda environment can be created from `environment.yml` by running:
```
conda env create -f environment.yml
```
To run the main script, use the command:
```
python main.py
```

### Contributor
Cristian Cioflan, ETH Zurich, [cioflanc@iis.ee.ethz.ch](cioflanc@iis.ee.ethz.ch)


### License
The code is released under Apache 2.0, see the LICENSE file in the root of this repository for details.
