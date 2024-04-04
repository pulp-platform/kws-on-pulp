from typing import Literal, Tuple, Type, Union

from torch.nn import AvgPool2d, Dropout, Flatten, Linear, Module, Sequential, ReLU6, BatchNorm2d, Conv2d, ReLU

class TinyMLDsCnn(Module):

    def __init__(
        self,
        input_shape: Tuple[int, int, int] = (1, 33, 10),
        channels_init: int = 64,
        classes: int = 10,
    ) -> None:
        super().__init__()

        channels = channels_init
        avg_pool_kernel_size = (input_shape[1] // 2, input_shape[2] // 2)

        self.net = Sequential(
            ConvBNReLU(
                input_shape[0], channels, kernel_size=(10, 4), stride=2, padding=(4, 1)
            ),
            Dropout(0.2),
            DepthwiseSeparableConv(channels, channels, stride=1),
            # DepthwiseSeparableConv(channels, channels, stride=1),
            Dropout(0.4),
            AvgPool2d(avg_pool_kernel_size),
            Flatten(),
            Linear(channels, classes),
        )

    def forward(self, x):
        return self.net(x)


class DepthwiseSeparableConv(Module):

    def __init__(self, in_channels: int, out_channels: int, stride: int):
        super().__init__()
        self.net = Sequential(
            ConvBNReLU(
                in_channels,
                in_channels,
                kernel_size=3,
                stride=stride,
                padding=1,
                groups=in_channels,
                ReluCls=ReLU,
            ),
            ConvBNReLU(in_channels, out_channels, kernel_size=1, ReluCls=ReLU),
        )

    def forward(self, x):
        return self.net(x)


class ConvBNReLU(Module):
    def __init__(
        self,
        in_channels: int,
        out_channels: int,
        kernel_size: Union[int, Tuple[int, int]],
        stride: Union[int, Tuple[int, int]] = 1,
        padding: Union[int, Tuple[int, int], Literal["same", "valid"]] = 0,
        groups: int = 1,
        ReluCls: Type[Module] = ReLU6,
    ):
        super().__init__()
        self.net = Sequential(
            Conv2d(
                in_channels,
                out_channels,
                kernel_size=kernel_size,
                stride=stride,
                padding=padding,
                groups=groups,
            ),
            BatchNorm2d(out_channels),
            ReluCls(),
        )

    def forward(self, x):
        return self.net(x)