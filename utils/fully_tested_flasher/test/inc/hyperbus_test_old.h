#include <pulp.h>

#define UDMA_HYPERBUS_OFFSET 0x1a102000+128*9
#define HYPERBUS_DEVICE_NUM 4

static inline void udma_hyper_setup(){
  pulp_write32(0x1a102000, 1 << HYPERBUS_DEVICE_NUM); // clock for the hyper bus module is activated
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x4C, 0x00 ); // 2D TRAN is deactivated
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x28, 0x01 ); // REG_T_EN_LATENCY_ADD
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x2C, 0xffffffff ); // REG_T_CS_MAX
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x48, 0x00); // Flash memory is selected
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x24, 0x06); // T latench access is set to 16 cycles

}

static inline void udma_hyper_flash_setup(){
  pulp_write32(0x1a102000, 1 << HYPERBUS_DEVICE_NUM); // clock for the hyper bus module is activated
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x4C, 0x00 ); // 2D TRAN is deactivated
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x28, 0x00 ); // REG_T_EN_LATENCY_ADD
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x2C, 0xffffffff ); // REG_T_CS_MAX
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x48, 0x01); // Flash memory is selected
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x24, 0x0f); // T latench access is set to 16 cycles
}
static inline void udma_hyper_sleep(){
  int a;
  a = pulp_read32(0x1a102000);
  pulp_write32(0x1a102000, (1 << HYPERBUS_DEVICE_NUM)^a ); // Clock gating is activated
}

// Burst write is conducted for the hyper flash. len <- burst length in bytes, ext_addr <- start address of the external memory, l2_addr <- start_address of the L2 memory, page_bound <- page boundary in the external memory
static inline void udma_hyper_dwrite(unsigned int len, unsigned int ext_addr, unsigned int l2_addr, unsigned int page_bound){

  switch(page_bound){
     case 128: 
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x00 ); // page boundary is set to every 128 bytes
        break;
     case 256:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x01 ); // page boundary is set to every 256 bytes
        break;
     case 512:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x02 ); // page boundary is set to every 128 bytes
        break;
     case 1024:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x03 ); // page boundary is set to every 256 bytes
        break;
     default:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x04 ); // page boundary is not considered
  }

  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x0C, l2_addr ); // Data address 
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x10, len );     // Data size to be sent
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x1C, ext_addr );     // Data size to be sent
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x18, 0x01);     // Write is declared for the external mem
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x14, 0x14);     // Write transaction is kicked
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x14, 0x00);     // Write transaction information is reset
}

// Word write for Hyper flash
static inline void udma_hyperflash_wwrite(unsigned int ext_addr, short int data){
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x04 ); // page boundary is not considered
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x4C, 0x00 ); // 2D TRAN is deactivated
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x28, 0x00 ); // REG_T_EN_LATENCY_ADD
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x1C, ext_addr<<1); // Address for the external memory
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x40, data); // Data to be written
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x18, 0x00); // Write is declared for the external mem
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x14, 0x14); // Write transaction is kicked
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x14, 0x00); // Write transaction information is reset
  
}


// Burst write for Hyper flash. Page boundary consideration and byte addressing mode are NOT supported. nb_word <- burst length in words, hyper_waddr <- start address of the hyperflash (word addressing), l2_addr <- start_address for L2
static inline void udma_hyperflash_bwrite(unsigned int nb_word, unsigned int hyper_waddr, unsigned int l2_addr){
    udma_hyperflash_wwrite(0x555, 0x00aa);
    udma_hyperflash_wwrite(0x2aa, 0x0055);
    udma_hyperflash_wwrite(hyper_waddr, 0x0025);
    udma_hyperflash_wwrite(hyper_waddr, nb_word-1);
    for(int i=0; i< nb_word; i++ ){
       udma_hyperflash_wwrite(hyper_waddr+i, *((short int *) l2_addr+i));
      // printf("%d th data= %x \n", i,  *((short int *) l2_addr+i));
    }
    udma_hyperflash_wwrite(hyper_waddr, 0x0029);
    rt_time_wait_us(1000);

}

static inline void udma_hyperflash_erase(unsigned int sector_addr){
    udma_hyperflash_wwrite(0x555, 0x00aa);
    udma_hyperflash_wwrite(0x2aa, 0x0055);
    udma_hyperflash_wwrite(0x555, 0x0080);
    udma_hyperflash_wwrite(0x555, 0x00aa);
    udma_hyperflash_wwrite(0x2aa, 0x0055);
    udma_hyperflash_wwrite(sector_addr, 0x0030);
    rt_time_wait_us(1000); 

}

static inline void udma_hyperflash_cperase(){
    udma_hyperflash_wwrite(0x555, 0x00aa);
    udma_hyperflash_wwrite(0x2aa, 0x0055);
    udma_hyperflash_wwrite(0x555, 0x0080);
    udma_hyperflash_wwrite(0x555, 0x00aa);
    udma_hyperflash_wwrite(0x2aa, 0x0055);
    udma_hyperflash_wwrite(0x555, 0x0010);
    rt_time_wait_us(1000);

}


// Linear read is conducted. len <- burst length in bytes, ext_addr <- start address of the external memory, l2_addr <- start_address of the L2 memory, page_bound <- page boundary in the external memory
//

static inline void udma_hyper_dread(unsigned int len, unsigned int ext_addr, unsigned int l2_addr, unsigned int page_bound){
  
  switch(page_bound){
     case 128:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x00 ); // page boundary is set to every 128 bytes
        break;
     case 256:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x01 ); // page boundary is set to every 256 bytes
        break;
     case 512:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x02 ); // page boundary is set to every 128 bytes
        break;
     case 1024:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x03 ); // page boundary is set to every 256 bytes
        break;
     default:
        pulp_write32(UDMA_HYPERBUS_OFFSET + 0x20, 0x04 ); // page boundary is not considered
  }

  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x00, l2_addr ); // Data address 
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x04, len );     // Data size to be sent
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x1C, ext_addr );     // Data size to be sent
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x18, 0x05);     // Read is declared for the external mem
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x08, 0x14);     // Read transaction is kicked
  pulp_write32(UDMA_HYPERBUS_OFFSET + 0x08, 0x00);     // Write transaction information is reset
}


// Outputs the number of transactions which is remaining the FIFO
static inline int udma_hyper_nb_tran(){
  int a;
  return pulp_read32(UDMA_HYPERBUS_OFFSET + 0x58) >> 1;
}

// If the hyperbus module is doing something.
static inline int udma_hyper_busy(){
  return pulp_read32(UDMA_HYPERBUS_OFFSET + 0x58) & 0x00000001;
}

static inline void udma_hyper_wait(){
   while(udma_hyper_busy()){
   }
}
