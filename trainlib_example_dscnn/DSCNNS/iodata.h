#ifndef IODATA_H
#define IODATA_H
// Init weights
#define WGT_SIZE_L0 768
PI_L2 float init_WGT_l0[WGT_SIZE_L0];
// Input and Output data
#define IN_SIZE 64
PI_L1 float IN_DATA[IN_SIZE];
#define OUT_SIZE 12
PI_L2 float REFERENCE_OUTPUT[OUT_SIZE];
PI_L1 float LABEL[OUT_SIZE];
#endif /* IODATA_H */
