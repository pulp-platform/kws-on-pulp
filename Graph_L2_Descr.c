#include "SFU_RT.h"
unsigned int Graph_Clock[] = {
	0X00000003, /* N_CLOCK: 3, BASEID: 0 */
	0X0000000F, /* C[0], C[1], C[2], C[3] */
	0X00000103, /* CLK_CFG[0].VECT[0] Clk: PDM0 SFU_MemOut[0.0.0] */
	0X00000000, /* CLK_CFG[0].VECT[1] */
	0X00000000, /* CLK_CFG[0].VECT[2] */
	0X00000204, /* CLK_CFG[1].VECT[0] Clk: PDM1 SFU_MemOut[1.0.0] */
	0X00000000, /* CLK_CFG[1].VECT[1] */
	0X00000000, /* CLK_CFG[1].VECT[2] */
	0X00000405, /* CLK_CFG[2].VECT[0] Clk: PDM2 SFU_MemOut[2.0.0] */
	0X00000000, /* CLK_CFG[2].VECT[1] */
	0X00000000, /* CLK_CFG[2].VECT[2] */
	0X00000806, /* CLK_CFG[3].VECT[0] Clk: PDM3 SFU_MemOut[3.0.0] */
	0X00000000, /* CLK_CFG[3].VECT[1] */
	0X00000000, /* CLK_CFG[3].VECT[2] */
};
unsigned int Graph_Graph[] = {
	0XF00F0000, /* Used Blocks[0] SFU_PdmIn.0 SFU_PdmIn.1 SFU_PdmIn.2 SFU_PdmIn.3 SFU_MemOut.0 SFU_MemOut.1 SFU_MemOut.2 SFU_MemOut.3 */
	0X00000000, /* Used Blocks[1] */
	0X00000000, /* Used Blocks[2] */
	0X00000000, /* Used Blocks[3] */
	0X0000000F, /* Used Clocks[0] */
	0X00030000, /* 00000005:       SFU_PdmIn.0.0 CFG0: Routing:     SFU_MemOut.0.0.0, PDM_SAI_SEL:  3, PDM_CH_SEL:  0, CLK_SEL:  0, RT_EN: 0 */
	0X000183FF, /* 00000006:       SFU_PdmIn.0.0 CFG1: CIC_N:  7, CIC_M:  1, CIC_R: 63, CIC_SHIFT: 24 */
	0X00230001, /* 00000007:       SFU_PdmIn.1.0 CFG0: Routing:     SFU_MemOut.1.0.0, PDM_SAI_SEL:  3, PDM_CH_SEL:  0, CLK_SEL:  1, RT_EN: 0 */
	0X000183FF, /* 00000008:       SFU_PdmIn.1.0 CFG1: CIC_N:  7, CIC_M:  1, CIC_R: 63, CIC_SHIFT: 24 */
	0X00430002, /* 00000009:       SFU_PdmIn.2.0 CFG0: Routing:     SFU_MemOut.2.0.0, PDM_SAI_SEL:  3, PDM_CH_SEL:  0, CLK_SEL:  2, RT_EN: 0 */
	0X000183FF, /* 0000000A:       SFU_PdmIn.2.0 CFG1: CIC_N:  7, CIC_M:  1, CIC_R: 63, CIC_SHIFT: 24 */
	0X00630003, /* 0000000B:       SFU_PdmIn.3.0 CFG0: Routing:     SFU_MemOut.3.0.0, PDM_SAI_SEL:  3, PDM_CH_SEL:  0, CLK_SEL:  3, RT_EN: 0 */
	0X000183FF, /* 0000000C:       SFU_PdmIn.3.0 CFG1: CIC_N:  7, CIC_M:  1, CIC_R: 63, CIC_SHIFT: 24 */
	0X000003FF, /* 0000000D:      SFU_MemOut.0.0 CFG0: UDMA_TARGET_ID: 255, DATASIZE:  3, CLK_SEL:  0, RT_EN: 0 */
	0X000403FF, /* 0000000E:      SFU_MemOut.1.0 CFG0: UDMA_TARGET_ID: 255, DATASIZE:  3, CLK_SEL:  1, RT_EN: 0 */
	0X000803FF, /* 0000000F:      SFU_MemOut.2.0 CFG0: UDMA_TARGET_ID: 255, DATASIZE:  3, CLK_SEL:  2, RT_EN: 0 */
	0X000C03FF, /* 00000010:      SFU_MemOut.3.0 CFG0: UDMA_TARGET_ID: 255, DATASIZE:  3, CLK_SEL:  3, RT_EN: 0 */
};
SFU_RunTimeDescr_T Graph_RT_Descr = {
	0X0, /* Bitmask of enabled MEM_IN blocks */
	17, /* Number of 32-bit words in Graph descriptor array */
	8, /* Number of entries */
	Graph_Graph,
	Graph_Clock,
	NULL,
	{
		{0, 1,   11, 0,  0, 0,  0,  2, 3,  0,  3},  /*             In3:       SFU_PdmIn.3.0, Bound:  No, Offset:   B, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 3 */
		{0, 1,    9, 0,  0, 0,  0,  2, 2,  0,  2},  /*             In2:       SFU_PdmIn.2.0, Bound:  No, Offset:   9, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 2 */
		{0, 1,    7, 0,  0, 0,  0,  2, 1,  0,  1},  /*             In1:       SFU_PdmIn.1.0, Bound:  No, Offset:   7, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 1 */
		{0, 1,    5, 0,  0, 0,  0,  2, 0,  0,  0},  /*             In0:       SFU_PdmIn.0.0, Bound:  No, Offset:   5, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 0 */
		{1, 1,   16, 0,  0, 0,  0,  3, 3,  0,  3},  /*            Out3:      SFU_MemOut.3.0, Bound:  No, Offset:  10, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 3 */
		{1, 1,   15, 0,  0, 0,  0,  3, 2,  0,  2},  /*            Out2:      SFU_MemOut.2.0, Bound:  No, Offset:   F, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 2 */
		{1, 1,   14, 0,  0, 0,  0,  3, 1,  0,  1},  /*            Out1:      SFU_MemOut.1.0, Bound:  No, Offset:   E, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 1 */
		{1, 1,   13, 0,  0, 0,  0,  3, 0,  0,  0},  /*            Out0:      SFU_MemOut.0.0, Bound:  No, Offset:   D, MSrc:0, SClk:0, MDst:0, DClk:0, Clock: 0 */
	}
};
