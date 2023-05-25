#include "SFU_RT.h"
unsigned int Graph_Clock[] = {
	0X00000000, /* N_CLOCK: 0, BASEID: 0 */
	0X00000001, /* C[0] */
	0X00000103, /* CLK_CFG[0].VECT[0] Clk: PDM0 SFU_MemOut[0.0.0] */
	0X00000000, /* CLK_CFG[0].VECT[1] */
	0X00000000, /* CLK_CFG[0].VECT[2] */
};
unsigned int Graph_Graph[] = {
	0X10010000, /* Used Blocks[0] SFU_PdmIn.0 SFU_MemOut.0 */
	0X00000000, /* Used Blocks[1] */
	0X00000000, /* Used Blocks[2] */
	0X00000000, /* Used Blocks[3] */
	0X00000001, /* Used Clocks[0] */
	0X00030000, /* 00000005:       SFU_PdmIn.0.0 CFG0: Routing:     SFU_MemOut.0.0.0, PDM_SAI_SEL:  3, PDM_CH_SEL:  0, CLK_SEL:  0, RT_EN: 0 */
	0X000183FF, /* 00000006:       SFU_PdmIn.0.0 CFG1: CIC_N:  7, CIC_M:  1, CIC_R: 63, CIC_SHIFT: 24 */
	0X000003FF, /* 00000007:      SFU_MemOut.0.0 CFG0: UDMA_TARGET_ID: 255, DATASIZE:  3, CLK_SEL:  0, RT_EN: 0 */
};
SFU_RunTimeDescr_T Graph_RT_Descr = {
	0X0, /* Bitmask of enabled MEM_IN blocks */
	8, /* Number of 32-bit words in Graph descriptor array */
	2, /* Number of entries */
	Graph_Graph,
	Graph_Clock,
	{
		{0, 1,    5, 0,  0, 0,  0,  2, 0,  0,  0},  /*             In1:       SFU_PdmIn.0.0, Bound:  No, Offset:   5, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 0 */
		{1, 1,    7, 0,  0, 0,  0,  3, 0,  0,  0},  /*            Out1:      SFU_MemOut.0.0, Bound:  No, Offset:   7, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 0 */
	}
};
