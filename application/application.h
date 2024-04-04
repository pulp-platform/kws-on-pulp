
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifdef __EMUL__
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <string.h>
#endif


// Definitions
#define SCALE_IN denoiser_dns_Input_1_OUT_SCALE
#define SCALE_OUT denoiser_dns_Output_1_OUT_SCALE

#define DOUBLE_BUFF_SIZE (1000)
#define BUFF_SIZE (48*1000*4)
#define AUDIO_BUFFER_SIZE 16000 // (32*1024)
#define CHUNK_NUM (8)

// SAI Setup
#define STRUCT_DELAY (1)
#define SAI1         (1)
#define SAI_ID               (48)
#define SAI_SCK(itf)         (48+(itf*4)+0)
#define SAI_WS(itf)          (48+(itf*4)+1)
#define SAI_SDI(itf)         (48+(itf*4)+2)
#define SAI_SDO(itf)         (48+(itf*4)+3)

#define L2_MEMORY_SIZE 1000000

#define NORM 6

// User Push Button
#define PAD_GPIO_UPB    (PI_PAD_086)

#ifdef SILENT
# define PRINTF(...) ((void) 0)
#else
# define PRINTF printf
#endif  /* DEBUG */

#endif /* APPLICATION_H */
