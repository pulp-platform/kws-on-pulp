# ODDA for KWS on GAP9

This project enables the deployment of a keyword spotting neural network on GAP9 for inference and training.

## Preliminary steps

```
git submodule update --init
```

### Pretrain ONNX model

Train the model, export it in FP32, and quantize it to INT8 through Nemo. 
```
cd kws-on-pulp/quantization
python main.py
cd ..
```

Alternatively, a pretrained model can be exported to FP32 and then quantized to INT8 through Quantlib.
```
cd kws-on-gap9/
python quantize.py --net DSCNN --fix_channels --word_align_channels --clip_inputs
cd ..
``` 

### [INFERENCE] Generate DORY-based C code for GAP9

```
cd dory/
git submodule update --init 
cd -
./deploy_dory.sh gap_sdk 3 gvsoc 0 2 DSCNN_DIR_DEST DSCNN_DIR_SRC 8

# Integrate DORY-gen code into ours
cd ..
cp -r kws-on-pulp/DSCNN_DIR_DEST/ .
rm DSCNN_DIR_DEST/src/main.c
```

Note that the DORY-generated C code currently allows setting the number of `n_frozen_layers` in `dory/Hardware_targets/PULP/PULP_gvsoc/Templates/network_c_template.c`. This should be passed as external paramater during code generation, also accounting for the number of non-parametrizable operations (e.g., AvgPool, Identity).



## [INFERENCE] Run on GAP9

To run the network on GVSOC:
```
./deploy.sh gvsoc 1 1 1 0 WORK
```

To run the network on GAP9 with the Evaluation Kit:
```
./deploy.sh board 0 0 0 0 WORK
```

To understand the runtime parameters:
```
./deploy.sh -h
```

This currently integrates inference and user-indicated training. An inference-only mode should be ensured.

### [TRAIN] Generate PULP TrainLib-based C code

To generate the FP32 C code for the trainable segment of the network, run:

```
cd pulp-trainlib/
./codegen.sh path/to/dest/ network/ path/to/model.onnx # generates net.{h,c}, initdefines.h, iodata.{h}
```

=======

### Board configuration

To use the Vesper microphone on GAP9mod
* Jumper on J7
* Connect CN9.1 and CN9.2

Select:
* GAP9_EVK_AUDIO
* GWT Board: Gap9mod V1.0b & Evaluation kit (V2.0) -- only for CMakeList


