# ----------------------------------------------------------------------
#
# File: quantUtils.py
#
# Last edited: 11.04.2024
#
# Copyright (C) 2024, ETH Zurich and University of Bologna.
#
# Author:
# - Victor Jung, jungvi@iis.ee.ethz.ch, ETH Zurich
# - Moritz Scherer, scheremo@iis.ee.ethz.ch, ETH Zurich
# - Philip Wiese, pwiese@iis.ee.ethz.ch, ETH Zurich
#
# ----------------------------------------------------------------------
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License); you may
# not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import torch 
import builtins
from typing import List

from quantlib.editing.fx.passes import SequentialPass
from quantlib.editing.fx.passes.pact import AddTreeReplacementPass, \
    ApproximateSoftmaxPass, ConcatTreeReplacementPass, HomogeneousFakeQuantPass, \
    MatmulReplacementPass, MulReplacementPass, InsertActivationsAfterLinearsPass, \
    TruedivReplacementPass, CanonicalizeLayerNormPass, ApproximateGELUPass


def fakeTrain(model, fakeBatch, epoch, optimizer, quantControllers=[], scheduler=None, device="cpu"):
    model.train()

    for ctrlr in quantControllers:
        ctrlr.step_pre_training_batch(epoch, optimizer)

    optimizer.zero_grad()
    _ = model(*fakeBatch)

def fakeValidate(model, fakeBatch, epoch, scheduler=None, device="cpu"):
    model.eval()
    _ = model(*fakeBatch)

    if scheduler is not None:
        scheduler.step()

def getAbsMinAbsMax(tensor, n_levels=256):
    if tensor.numel() == 0:
        return torch.tensor(0), torch.tensor(1)

    _max = tensor.max()
    _min = tensor.min()

    if _max == 0 and _min == 0:
        _max = torch.tensor(1)

    absMax = torch.max(_max, torch.abs(_min))

    if _min == 0:
        absMin = torch.tensor(0)
    else:
        absMin = -absMax / ((n_levels // 2) - 1) * (n_levels // 2)

    return absMin.type_as(tensor), absMax.type_as(tensor)

def getAdhocEpsList(n_levels: int = 256, *inputTensors) -> List[float]:

    epsList = []

    for tensor in inputTensors:

        absMin, absMax = getAbsMinAbsMax(tensor)

        eps = (absMax - absMin) / (n_levels - 1)
        epsList.append(eps)

    return epsList

def roundTensors(fakeBatch, eps_in):

    allRounded = []

    for tensor, eps in zip(fakeBatch, eps_in):
        # Skip if tensor is empty
        if tensor.numel() == 0:
            allRounded.append(tensor)
            continue

        absMin, absMax = getAbsMinAbsMax(tensor)

        eps = eps.type_as(tensor)

        rounded = torch.trunc(torch.clamp(tensor, min=absMin, max=absMax) / eps) * eps
        allRounded.append(rounded)

    return allRounded

def matchSizeNode(node):
    return node.op == "call_method" and node.target == "size"

def matchShapeNode(node):
    return node.op == "call_function" and node.target == builtins.getattr


class ViTCanonAndApprox(SequentialPass):

    def __init__(self, _ActQuantArgs, _LinearQuantArgs, _PactifyQuantArgs, _IntegerQuantArgs):
        _passes = []
        _passes.append(CanonicalizeLayerNormPass())
        _passes.append(HomogeneousFakeQuantPass(convArgs={},
                                                        actArgs=_PactifyQuantArgs,
                                                        linearArgs=_LinearQuantArgs))
        _passes.append(ApproximateGELUPass())
        _passes.append(ApproximateSoftmaxPass(mode="I-BERT"))
        _passes.append(MatmulReplacementPass())
        _passes.append(TruedivReplacementPass())
        _passes.append(AddTreeReplacementPass(**_ActQuantArgs))
        _passes.append(MulReplacementPass())
        _passes.append(ConcatTreeReplacementPass(**_IntegerQuantArgs))
        _passes.append(InsertActivationsAfterLinearsPass(**_ActQuantArgs))
        super().__init__(*_passes, name_prefix="_VIT_APPROXIMATE_PASS")
