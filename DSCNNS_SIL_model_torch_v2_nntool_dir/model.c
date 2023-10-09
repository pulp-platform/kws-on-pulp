#include <stdint.h>
#include <stdio.h>
#include "AutoTilerLib.h"
#include "CNN_Generators_SQ8.h"

#include "CNN_Copy_Generators.h"





void DSCNNS_SIL_model_torch_v2Model(unsigned int L1Memory, unsigned int L2Memory, unsigned int L3Memory, unsigned int L3Flash)
{
    KernelOper_T Cop = KOP_CONV;

    // SetKernelOpts(KER_OPT_NONE, KER_OPT_BUFFER_PROMOTE);
    SetSymbolDynamics();

    SetUsedFilesNames(0, 3, "Gap.h", "DSCNNS_SIL_model_torch_v2.h", "CNN_BasicKernels_SQ8.h");
    SetGeneratedFilesNames("DSCNNS_SIL_model_torch_v2Kernels.c", "DSCNNS_SIL_model_torch_v2Kernels.h");


    SetMemoryDeviceInfos(4,
        AT_MEM_L1, L1Memory, "DSCNNS_SIL_model_torch_v2_L1_Memory", 0, 0,
        AT_MEM_L2, L2Memory, "DSCNNS_SIL_model_torch_v2_L2_Memory", 0, 1,
        AT_MEM_L3_DEFAULTRAM, L3Memory, "DSCNNS_SIL_model_torch_v2_L3_Memory", 0, 0,
        AT_MEM_L3_DEFAULTFLASH, L3Flash, "DSCNNS_SIL_model_torch_v2_L3_Flash", "DSCNNS_SIL_model_torch_v2_L3_Flash_Const.dat", 0
    );

    LoadCNN_SQ8_Library();


    // generator for Conv__66
    CNN_ConvolutionPoolAct_SQ8("S4_Conv__66", 0,
                               4, 1,
                               1, 64, 10, 49,
                               KOP_CONV, 4, 10, 1, 1, 2, 2, 1,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__68
    CNN_PoolAct_SQ8("S5_Relu__68", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_RELU);
    
    // generator for Conv__70
    CNN_ConvolutionPoolAct_SQ8("S9_Conv__70", 0,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV_DW, 3, 3, 1, 1, 1, 1, 1,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__72
    CNN_PoolAct_SQ8("S10_Relu__72", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_NONE);
    
    CNN_GenControl_T gen_ctrl_S14_Conv__73;
    CNN_InitGenCtrl(&gen_ctrl_S14_Conv__73);
    CNN_SetGenCtrl(&gen_ctrl_S14_Conv__73, "ENABLEIM2COL", AT_OPT_VAL(1));
    // generator for Conv__73
    CNN_ConvolutionPoolAct_SQ8("S14_Conv__73", &gen_ctrl_S14_Conv__73,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV, 1, 1, 1, 1, 1, 1, 0,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__75
    CNN_PoolAct_SQ8("S15_Relu__75", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_RELU);
    
    // generator for Conv__77
    CNN_ConvolutionPoolAct_SQ8("S19_Conv__77", 0,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV_DW, 3, 3, 1, 1, 1, 1, 1,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__79
    CNN_PoolAct_SQ8("S20_Relu__79", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_NONE);
    
    CNN_GenControl_T gen_ctrl_S24_Conv__80;
    CNN_InitGenCtrl(&gen_ctrl_S24_Conv__80);
    CNN_SetGenCtrl(&gen_ctrl_S24_Conv__80, "ENABLEIM2COL", AT_OPT_VAL(1));
    // generator for Conv__80
    CNN_ConvolutionPoolAct_SQ8("S24_Conv__80", &gen_ctrl_S24_Conv__80,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV, 1, 1, 1, 1, 1, 1, 0,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__82
    CNN_PoolAct_SQ8("S25_Relu__82", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_RELU);
    
    // generator for Conv__84
    CNN_ConvolutionPoolAct_SQ8("S29_Conv__84", 0,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV_DW, 3, 3, 1, 1, 1, 1, 1,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__86
    CNN_PoolAct_SQ8("S30_Relu__86", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_NONE);
    
    CNN_GenControl_T gen_ctrl_S34_Conv__87;
    CNN_InitGenCtrl(&gen_ctrl_S34_Conv__87);
    CNN_SetGenCtrl(&gen_ctrl_S34_Conv__87, "ENABLEIM2COL", AT_OPT_VAL(1));
    // generator for Conv__87
    CNN_ConvolutionPoolAct_SQ8("S34_Conv__87", &gen_ctrl_S34_Conv__87,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV, 1, 1, 1, 1, 1, 1, 0,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__89
    CNN_PoolAct_SQ8("S35_Relu__89", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_RELU);
    
    // generator for Conv__91
    CNN_ConvolutionPoolAct_SQ8("S39_Conv__91", 0,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV_DW, 3, 3, 1, 1, 1, 1, 1,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__93
    CNN_PoolAct_SQ8("S40_Relu__93", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_NONE);
    
    CNN_GenControl_T gen_ctrl_S44_Conv__94;
    CNN_InitGenCtrl(&gen_ctrl_S44_Conv__94);
    CNN_SetGenCtrl(&gen_ctrl_S44_Conv__94, "ENABLEIM2COL", AT_OPT_VAL(1));
    // generator for Conv__94
    CNN_ConvolutionPoolAct_SQ8("S44_Conv__94", &gen_ctrl_S44_Conv__94,
                               4, 1,
                               64, 64, 5, 25,
                               KOP_CONV, 1, 1, 1, 1, 1, 1, 0,
                               KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                               KOP_NONE);
    
    // generator for Relu__96
    CNN_PoolAct_SQ8("S45_Relu__96", 0,
                    64, 5, 25,
                    KOP_NONE, 0, 0, 0, 0, 0, 0, 0,
                    KOP_RELU);
    
    // generator for AveragePool__98
    CNN_GlobalPoolAct_SQ8("S47_AveragePool__98", 0,
                          64, 25, 5,
                          KOP_GLOBAL_AVGPOOL, KOP_NONE);
    
    // generator for MatMul_output
    CNN_LinearAct_SQ8("S51_MatMul_output", 0,
                      4, 1,
                      64, 12,
                      KOP_LINEAR, KOP_NONE);
    

#define GRAPH
#ifdef GRAPH
    CreateGraph("DSCNNS_SIL_model_torch_v2CNN",
        /* Arguments either passed or globals */
            CArgs(62,
                TCArgInfo("signed char * __restrict__", "Input_1", ARG_SCOPE_ARG, ARG_DIR_IN, AT_MEM_L2, AT_MEM_L2, 0),
                TCArgInfo("signed char * __restrict__", "Conv__66_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__66_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv1_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv1_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S4_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S4_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S4_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S4_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S4_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S4_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.05274 out: 0.05274  actscale: [1] actscalen: [0] a0: [0] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S5_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S5_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__70_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__70_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv2_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv2_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S9_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S9_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S9_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S9_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S9_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S9_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.06040 out: 0.06040  actscale: [1] actscalen: [0] a0: [-128] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S10_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S10_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__73_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__73_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv3_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv3_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S14_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S14_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S14_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S14_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S14_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S14_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.10774 out: 0.10774  actscale: [1] actscalen: [0] a0: [0] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S15_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S15_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__77_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__77_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv4_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv4_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S19_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S19_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S19_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S19_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S19_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S19_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.04686 out: 0.04686  actscale: [1] actscalen: [0] a0: [-128] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S20_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S20_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__80_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__80_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv5_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv5_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S24_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S24_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S24_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S24_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S24_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S24_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.07203 out: 0.07203  actscale: [1] actscalen: [0] a0: [0] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S25_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S25_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__84_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__84_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv6_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv6_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S29_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S29_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S29_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S29_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S29_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S29_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.04321 out: 0.04321  actscale: [1] actscalen: [0] a0: [-128] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S30_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S30_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__87_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__87_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv7_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv7_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S34_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S34_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S34_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S34_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S34_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S34_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.06631 out: 0.06631  actscale: [1] actscalen: [0] a0: [0] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S35_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S35_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__91_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__91_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv8_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv8_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S39_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S39_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S39_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S39_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S39_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S39_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.05714 out: 0.05714  actscale: [1] actscalen: [0] a0: [-128] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S40_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S40_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Conv__94_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Conv__94_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Constant_conv9_bias", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Constant_conv9_bias.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S44_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S44_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S44_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S44_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S44_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S44_Infos.tensor", 1, 1, 8, 0)),
                // in: 0.22572 out: 0.22572  actscale: [1] actscalen: [0] a0: [0] b0: 0 c0: 0
                TCArgInfo("signed char * __restrict__", "S45_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S45_Infos.tensor", 1, 1, 8, 0)),
                // no activation ACTSCALE: 0 ACTSCALEN: 0 GLOBAL_SUM_SCALE: [45] GLOBAL_SUM_SCALEN: [10]
                TCArgInfo("signed char * __restrict__", "S47_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S47_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Matmul_output_weights", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Matmul_output_weights.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed int * __restrict__", "Matmul_output_biases", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("Matmul_output_biases.tensor", 1, 1, 32, 0)),
                TCArgInfo("unsigned char * __restrict__", "S51_Mul_scale", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S51_Mul_scale.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "S51_Mul_shift", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S51_Mul_shift.tensor", 1, 1, 8, 0)),
                // no activation BIASN: 0 PRENORM: 0
                TCArgInfo("signed char * __restrict__", "S51_Infos", ARG_SCOPE_GLOBAL, ARG_DIR_CONSTIN, AT_MEM_L3_DEFAULTFLASH, AT_MEM_UNDEF, ConstInfo("S51_Infos.tensor", 1, 1, 8, 0)),
                TCArgInfo("signed char * __restrict__", "Output_1", ARG_SCOPE_ARG, ARG_DIR_OUT, AT_MEM_L2, AT_MEM_L2, 0)
            ),
        /* Locals, allocated dynamically */
        CArgs(19,
            TCArgInfo("signed char * __restrict__", "S4_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S5_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S9_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S10_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S14_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S15_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S19_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S20_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S24_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S25_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S29_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S30_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S34_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S35_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S39_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S40_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S44_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S45_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0),
            TCArgInfo("signed char * __restrict__", "S47_Output", ARG_SCOPE_LOCAL, ARG_DIR_INOUT, AT_MEM_UNDEF, AT_MEM_UNDEF, 0)
        )
    );



    // Node S4_Conv__66 inq -181.07<(i8-0.00)*1.41460365<179.65 forced weightsq chan<(i8-0.00)*chan<chan outq -6.75<(i8-0.00)*0.05274282<6.70 forced biasesq chan<(i32-0.00)*chan<chan
    AddNode("S4_Conv__66",
        Bindings(7,
            GNodeArg(GNA_IN, "Input_1", 0),
            GNodeArg(GNA_IN, "Conv__66_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv1_bias", 0),
            GNodeArg(GNA_OUT, "S4_Output", 0),
            GNodeArg(GNA_IN, "S4_Mul_scale", 0),
            GNodeArg(GNA_IN, "S4_Mul_shift", 0),
            GNodeArg(GNA_IN, "S4_Infos", 0)
        )
    );
    // Node Relu__68 inq -6.75<(i8-0.00)*0.05274282<6.70 forced outq -6.75<(i8-0.00)*0.05274282<6.70 forced
    AddNode("S5_Relu__68",
        Bindings(3,
            GNodeArg(GNA_IN, "S4_Output", 0),
            GNodeArg(GNA_OUT, "S5_Output", 0),
            GNodeArg(GNA_IN, "S5_Infos", 0)
        )
    );
    // Node S9_Conv__70 inq -6.75<(i8-0.00)*0.05274282<6.70 forced weightsq chan<(i8-0.00)*chan<chan outq 0.00<(i8--128.00)*0.06039751<15.40 biasesq chan<(i32-0.00)*chan<chan
    AddNode("S9_Conv__70",
        Bindings(7,
            GNodeArg(GNA_IN, "S5_Output", 0),
            GNodeArg(GNA_IN, "Conv__70_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv2_bias", 0),
            GNodeArg(GNA_OUT, "S9_Output", 0),
            GNodeArg(GNA_IN, "S9_Mul_scale", 0),
            GNodeArg(GNA_IN, "S9_Mul_shift", 0),
            GNodeArg(GNA_IN, "S9_Infos", 0)
        )
    );
    // Node Relu__72 inq 0.00<(i8--128.00)*0.06039751<15.40 outq 0.00<(i8--128.00)*0.06039751<15.40
    AddNode("S10_Relu__72",
        Bindings(3,
            GNodeArg(GNA_IN, "S9_Output", 0),
            GNodeArg(GNA_OUT, "S10_Output", 0),
            GNodeArg(GNA_IN, "S10_Infos", 0)
        )
    );
    // Node S14_Conv__73 inq 0.00<(i8--128.00)*0.06039751<15.40 weightsq chan<(i8-0.00)*chan<chan outq -13.79<(i8-0.00)*0.10773577<13.68 forced biasesq chan<(i32-0.00)*chan<chan
    AddNode("S14_Conv__73",
        Bindings(7,
            GNodeArg(GNA_IN, "S10_Output", 0),
            GNodeArg(GNA_IN, "Conv__73_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv3_bias", 0),
            GNodeArg(GNA_OUT, "S14_Output", 0),
            GNodeArg(GNA_IN, "S14_Mul_scale", 0),
            GNodeArg(GNA_IN, "S14_Mul_shift", 0),
            GNodeArg(GNA_IN, "S14_Infos", 0)
        )
    );
    // Node Relu__75 inq -13.79<(i8-0.00)*0.10773577<13.68 forced outq -13.79<(i8-0.00)*0.10773577<13.68 forced
    AddNode("S15_Relu__75",
        Bindings(3,
            GNodeArg(GNA_IN, "S14_Output", 0),
            GNodeArg(GNA_OUT, "S15_Output", 0),
            GNodeArg(GNA_IN, "S15_Infos", 0)
        )
    );
    // Node S19_Conv__77 inq -13.79<(i8-0.00)*0.10773577<13.68 forced weightsq chan<(i8-0.00)*chan<chan outq 0.00<(i8--128.00)*0.04685958<11.95 biasesq chan<(i32-0.00)*chan<chan
    AddNode("S19_Conv__77",
        Bindings(7,
            GNodeArg(GNA_IN, "S15_Output", 0),
            GNodeArg(GNA_IN, "Conv__77_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv4_bias", 0),
            GNodeArg(GNA_OUT, "S19_Output", 0),
            GNodeArg(GNA_IN, "S19_Mul_scale", 0),
            GNodeArg(GNA_IN, "S19_Mul_shift", 0),
            GNodeArg(GNA_IN, "S19_Infos", 0)
        )
    );
    // Node Relu__79 inq 0.00<(i8--128.00)*0.04685958<11.95 outq 0.00<(i8--128.00)*0.04685958<11.95
    AddNode("S20_Relu__79",
        Bindings(3,
            GNodeArg(GNA_IN, "S19_Output", 0),
            GNodeArg(GNA_OUT, "S20_Output", 0),
            GNodeArg(GNA_IN, "S20_Infos", 0)
        )
    );
    // Node S24_Conv__80 inq 0.00<(i8--128.00)*0.04685958<11.95 weightsq chan<(i8-0.00)*chan<chan outq -9.22<(i8-0.00)*0.07202510<9.15 forced biasesq chan<(i32-0.00)*chan<chan
    AddNode("S24_Conv__80",
        Bindings(7,
            GNodeArg(GNA_IN, "S20_Output", 0),
            GNodeArg(GNA_IN, "Conv__80_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv5_bias", 0),
            GNodeArg(GNA_OUT, "S24_Output", 0),
            GNodeArg(GNA_IN, "S24_Mul_scale", 0),
            GNodeArg(GNA_IN, "S24_Mul_shift", 0),
            GNodeArg(GNA_IN, "S24_Infos", 0)
        )
    );
    // Node Relu__82 inq -9.22<(i8-0.00)*0.07202510<9.15 forced outq -9.22<(i8-0.00)*0.07202510<9.15 forced
    AddNode("S25_Relu__82",
        Bindings(3,
            GNodeArg(GNA_IN, "S24_Output", 0),
            GNodeArg(GNA_OUT, "S25_Output", 0),
            GNodeArg(GNA_IN, "S25_Infos", 0)
        )
    );
    // Node S29_Conv__84 inq -9.22<(i8-0.00)*0.07202510<9.15 forced weightsq chan<(i8-0.00)*chan<chan outq 0.00<(i8--128.00)*0.04320811<11.02 biasesq chan<(i32-0.00)*chan<chan
    AddNode("S29_Conv__84",
        Bindings(7,
            GNodeArg(GNA_IN, "S25_Output", 0),
            GNodeArg(GNA_IN, "Conv__84_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv6_bias", 0),
            GNodeArg(GNA_OUT, "S29_Output", 0),
            GNodeArg(GNA_IN, "S29_Mul_scale", 0),
            GNodeArg(GNA_IN, "S29_Mul_shift", 0),
            GNodeArg(GNA_IN, "S29_Infos", 0)
        )
    );
    // Node Relu__86 inq 0.00<(i8--128.00)*0.04320811<11.02 outq 0.00<(i8--128.00)*0.04320811<11.02
    AddNode("S30_Relu__86",
        Bindings(3,
            GNodeArg(GNA_IN, "S29_Output", 0),
            GNodeArg(GNA_OUT, "S30_Output", 0),
            GNodeArg(GNA_IN, "S30_Infos", 0)
        )
    );
    // Node S34_Conv__87 inq 0.00<(i8--128.00)*0.04320811<11.02 weightsq chan<(i8-0.00)*chan<chan outq -8.49<(i8-0.00)*0.06631458<8.42 forced biasesq chan<(i32-0.00)*chan<chan
    AddNode("S34_Conv__87",
        Bindings(7,
            GNodeArg(GNA_IN, "S30_Output", 0),
            GNodeArg(GNA_IN, "Conv__87_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv7_bias", 0),
            GNodeArg(GNA_OUT, "S34_Output", 0),
            GNodeArg(GNA_IN, "S34_Mul_scale", 0),
            GNodeArg(GNA_IN, "S34_Mul_shift", 0),
            GNodeArg(GNA_IN, "S34_Infos", 0)
        )
    );
    // Node Relu__89 inq -8.49<(i8-0.00)*0.06631458<8.42 forced outq -8.49<(i8-0.00)*0.06631458<8.42 forced
    AddNode("S35_Relu__89",
        Bindings(3,
            GNodeArg(GNA_IN, "S34_Output", 0),
            GNodeArg(GNA_OUT, "S35_Output", 0),
            GNodeArg(GNA_IN, "S35_Infos", 0)
        )
    );
    // Node S39_Conv__91 inq -8.49<(i8-0.00)*0.06631458<8.42 forced weightsq chan<(i8-0.00)*chan<chan outq 0.00<(i8--128.00)*0.05713765<14.57 biasesq chan<(i32-0.00)*chan<chan
    AddNode("S39_Conv__91",
        Bindings(7,
            GNodeArg(GNA_IN, "S35_Output", 0),
            GNodeArg(GNA_IN, "Conv__91_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv8_bias", 0),
            GNodeArg(GNA_OUT, "S39_Output", 0),
            GNodeArg(GNA_IN, "S39_Mul_scale", 0),
            GNodeArg(GNA_IN, "S39_Mul_shift", 0),
            GNodeArg(GNA_IN, "S39_Infos", 0)
        )
    );
    // Node Relu__93 inq 0.00<(i8--128.00)*0.05713765<14.57 outq 0.00<(i8--128.00)*0.05713765<14.57
    AddNode("S40_Relu__93",
        Bindings(3,
            GNodeArg(GNA_IN, "S39_Output", 0),
            GNodeArg(GNA_OUT, "S40_Output", 0),
            GNodeArg(GNA_IN, "S40_Infos", 0)
        )
    );
    // Node S44_Conv__94 inq 0.00<(i8--128.00)*0.05713765<14.57 weightsq chan<(i8-0.00)*chan<chan outq -28.89<(i8-0.00)*0.22572471<28.67 forced biasesq chan<(i32-0.00)*chan<chan
    AddNode("S44_Conv__94",
        Bindings(7,
            GNodeArg(GNA_IN, "S40_Output", 0),
            GNodeArg(GNA_IN, "Conv__94_weights", 0),
            GNodeArg(GNA_IN, "Constant_conv9_bias", 0),
            GNodeArg(GNA_OUT, "S44_Output", 0),
            GNodeArg(GNA_IN, "S44_Mul_scale", 0),
            GNodeArg(GNA_IN, "S44_Mul_shift", 0),
            GNodeArg(GNA_IN, "S44_Infos", 0)
        )
    );
    // Node Relu__96 inq -28.89<(i8-0.00)*0.22572471<28.67 forced outq -28.89<(i8-0.00)*0.22572471<28.67 forced
    AddNode("S45_Relu__96",
        Bindings(3,
            GNodeArg(GNA_IN, "S44_Output", 0),
            GNodeArg(GNA_OUT, "S45_Output", 0),
            GNodeArg(GNA_IN, "S45_Infos", 0)
        )
    );
    // Node AveragePool__98 inq -28.89<(i8-0.00)*0.22572471<28.67 forced outq -5.13<(i8-0.00)*0.04006126<5.09
    AddNode("S47_AveragePool__98",
        Bindings(3,
            GNodeArg(GNA_IN, "S45_Output", 0),
            GNodeArg(GNA_OUT, "S47_Output", 0),
            GNodeArg(GNA_IN, "S47_Infos", 0)
        )
    );
    // Node MatMul_output inq -5.13<(i8-0.00)*0.04006126<5.09 weightsq chan<(i8-0.00)*chan<chan outq -38.77<(i8-0.00)*0.30287716<38.47 forced
    AddNode("S51_MatMul_output",
        Bindings(7,
            GNodeArg(GNA_IN, "S47_Output", 0),
            GNodeArg(GNA_IN, "Matmul_output_weights", 0),
            GNodeArg(GNA_IN, "Matmul_output_biases", 0),
            GNodeArg(GNA_OUT, "Output_1", 0),
            GNodeArg(GNA_IN, "S51_Mul_scale", 0),
            GNodeArg(GNA_IN, "S51_Mul_shift", 0),
            GNodeArg(GNA_IN, "S51_Infos", 0)
        )
    );
    CloseGraph();
#endif
}

int main(int argc, char **argv)

{
    if (TilerParseOptions(argc, argv)) {
            printf("Failed to initialize or incorrect output arguments directory.\n"); return 1;
    }
    DSCNNS_SIL_model_torch_v2Model(64000, 300000, 8000000, 64*1024*1024);
    GenerateTilingCode();
    return 0;
}
