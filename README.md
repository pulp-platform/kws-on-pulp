# Keyword spotting on PULP platforms - a tutorial

Keyword Spotting (KWS) represents the task of processing an utterance and recognizing a keyword from a predefined set.
KWS is also known as closed-vocabulary automated speech recognition. 
Applications relying on KWS, such as voice-activated virtual assistants or sound source localization, target extreme-edge embedded systems, while the sensors (i.e., microphones) are also located on tinyML platforms.
It is thus natural to aim to also perform keyword spotting at the extreme edge, on platforms such as the GAP9 MCU.

Consider a Depthwise-Separable Convolutional Neural Network (DS-CNN) that you pretrained on a KWS dataset such as Google Speech Commands. 
In this tutorial we will understand the main steps required to use the pretrained network to process a 1-second input acquired on-board and to classify the utterance.

## Quantization

We will employ quantlib for this purpose.

The resulting quantized model, saved in .onnx format, together with the per-layer activations. A configuration file, required for hardware deployment, is additionally generated. You can find the files in `export/`.

## Deployment

We employ dory for this purpose.

