/////////////////////////////////////////////////////////////////////////// PULP Flasher ////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <pulp.h>
#include "rt/rt_api.h"

// Includes the Hayate's Library to communicate with HyperBus
#include "hyperbus_test.h"

// To ouput the results of flash writing (VERY SLOW!!)
#define WRITE_AND_READ

// Defines the Flash Buffer size to allocate in L2
#define N 100
#define MAX_BUFF_SIZE (N*1024)

// This address is shared with OPENOCD script. If you would change it you must change also in that script
#define FETCH_L2_STATUS_BUFFER 0x1c003000

// Sets the frequency of FPGA
int __rt_fpga_fc_frequency = 10000000;
int __rt_fpga_periph_frequency = 10000000;
unsigned int __rt_iodev_uart_baudrate = 115200;

////////////////////////////////////////////// Status Buffer struct //////////////////////
// Settable from Host side
//   [0] FLAG image_ready;
//   [1] FLAG flash_run;
// Settable from Board side
//   [2] FLAG flasher_ready;
// Parameters from Board side
//   [3] ADDRESS flash_buffer_addr;
//   [4] SIZE max_buff_size;
// Parameters from Host side
//   [5] ADDRESS flash_addr;
//   [6] SIZE flash_size;
//////////////////////////////////////////////////////////////////////////////////////////

// Allocates the Status Buffer in L2
volatile RT_L2_DATA int StatusBuffer[7];
volatile int * L2_Buffer_Read;


int main()
{
  int blockSize = 0;
  int flashAddr = 0;
  int number_of_tranferts = 0;

  int write_size_byte = 32 * sizeof(char);
  int write_size_hword = (write_size_byte >> 1);

  int wr_done_hword = 0;
  int wr_done_byte = 0;

  // Initializes the Status Buffer Board side
  StatusBuffer[2] = 0;

  // Points to Status Buffer in L2
  *(int *) FETCH_L2_STATUS_BUFFER = StatusBuffer;

  // Initializes the Flash Buffer
  StatusBuffer[3] = (unsigned char *) rt_alloc(RT_ALLOC_FC_DATA, MAX_BUFF_SIZE); 
  StatusBuffer[4] = MAX_BUFF_SIZE;

  unsigned char * tmp_buffer;
  tmp_buffer = (unsigned char *) rt_alloc(RT_ALLOC_FC_DATA, 8*32); 

  // Configures the HyperBus communication
  udma_hyper_flash_setup(); 
  udma_hyperflash_wwrite(0x555, 0x0071, 0);
  
  // Erases the HyperFlash
  /*  
  for (int i=0; i< 14; i++) {
    printf("erasing sector %d \n", i);
    udma_hyperflash_erase(i,0);
    rt_time_wait_us(100000000);
  }
   */
  printf ("Flasher: Starting the PULP-Open Flashing\n");
  L2_Buffer_Read = (char *) rt_alloc(RT_ALLOC_FC_DATA, MAX_BUFF_SIZE);

  // Establishes the synchronization
  while(StatusBuffer[1] != 1)
  {
    rt_time_wait_us(1000);
  }

  //flashAddr = (StatusBuffer[5] >> 1);
  int max_addr_write = 0;
  int max_addr_read = 0;

  // while(StatusBuffer[1] == 1 && stop == 0)
  while(StatusBuffer[1] == 1)
  {
    // Sets the Board flag to ready
    StatusBuffer[2] = 1;

    // Waits the portion of image from Host side
    while (StatusBuffer[0] != 1)
    {
      rt_time_wait_us(1000);
    }
    // Sets the Board flag to busy
    StatusBuffer[2] = 0;

    // Exports the flash address and the block size (word addressing) from Status Buffer    
    flashAddr = (StatusBuffer[5] >> 1); // Moved outside the loop
    blockSize = StatusBuffer[6];

    int dummy;

 
    // Writes the HyperFlash
    printf("Flasher: Flashing the portion of image of size %d from L2 buffer address %X to HyperFlash address (word addressing) %X\r\n", blockSize, StatusBuffer[3], flashAddr);
    while(wr_done_hword < (blockSize / sizeof(short int)))
    {

      for (int i = 0; i < write_size_byte; i++) {
        tmp_buffer[i] = *((char *)StatusBuffer[3] + wr_done_byte + i);
        
      }     
      
      //udma_hyperflash_bwrite(write_size_hword, flashAddr + wr_done_hword, StatusBuffer[3] + wr_done_byte,0);
      
      // if (wr_done_hword < 64)
      //     udma_hyperflash_bwrite(write_size_hword, flashAddr + max_addr_write, tmp_buffer, 0); // Cioflan
      // else
      //     udma_hyperflash_bwrite(write_size_hword, flashAddr + wr_done_hword, tmp_buffer, 0); // Cioflan2

      udma_hyperflash_bwrite(write_size_hword, flashAddr + wr_done_hword, tmp_buffer, 0); // Cioflan

      rt_time_wait_us(20000);
      

#ifdef WRITE_AND_READ
      // udma_hyper_dread(32, flashAddr + wr_done_hword, (unsigned int) L2_Buffer_Read + wr_done_byte, 128, 0);
       
      // if (wr_done_hword < 64)
      //     udma_hyper_dread(write_size_byte, flashAddr + max_addr_read, (unsigned int) L2_Buffer_Read, 128, 0); // Cioflan
      // else 
      //     udma_hyper_dread(write_size_byte, flashAddr + wr_done_byte, (unsigned int) L2_Buffer_Read, 128, 0); // Cioflan2

      udma_hyper_dread(write_size_byte, (flashAddr << 1) + wr_done_byte, (unsigned int) L2_Buffer_Read, 128); // Cioflan

#endif
      // // Waits until the writing is completed
      udma_hyper_wait(0);

#ifdef WRITE_AND_READ

      for(int i=0; i<write_size_byte; i++)
      {
        if(*((char*)StatusBuffer[3] + wr_done_byte + i) != *((char*)L2_Buffer_Read + i))
        {
          // printf ("wr_done_byte: %i\r\n", wr_done_byte);
          // printf ("wr_done_hword: %i\r\n", wr_done_hword);
          printf("at index %d: error -> %X instead of %X\r\n", wr_done_byte + i, *((char *)L2_Buffer_Read + i), *((char *)StatusBuffer[3] + wr_done_byte + i));
        }
        else
        {
          dummy = 3;
          // printf("at index %d: ok -> %X\r\n", wr_done_byte + i, *((char *)L2_Buffer_Read + i));
        } 
      }

#endif
      wr_done_byte  += write_size_byte ;  // 32
      wr_done_hword += write_size_hword;  // 16

      if (max_addr_read < wr_done_byte){  // Before we had <; Stupid, I guess
        max_addr_read = wr_done_byte;
      } 
      if (max_addr_write < wr_done_hword){
        max_addr_write = wr_done_hword;
      }     
    }

    wr_done_byte = 0;
    wr_done_hword = 0;
    max_addr_read = 0;
    max_addr_write = 0;
    rt_time_wait_us(10000);

    number_of_tranferts++;
  }

  StatusBuffer[2] = 1;

  // Closes the Hyperbus communication
  printf("Flasher: Terminating the PULP-Open Flashing with %d transfert(s)\r\n", number_of_tranferts);

  udma_hyper_sleep();

  return 0;
}
