#include "dory.h"
#include "math.h"
/**
 *  @brief Gets a tile over a one-dimensional tiling grid.
 *
 *  Computes a pointer to the base of a particular tile in a one-dimensional
 *  tiling grid indexed by a (ii) index; in pseudo-Python
 *      ccn_get_tile_1d(x,ii) = x[ii*si:(ii+1)*si-1]
 *  where (si) os defining the pitch of the tiling grid in the (i) dimension.
 *
 *  @param x
 *      a pointer to the base of the 2d tiling grid.
 *  @param tile_ii
 *      the tiling index.
 *  @param tile_size_i
 *      the pitch of the tiling grid in the outer dimension, i.e. the distance
 *      between two "ticks" in the i dimension.
 *  @param data_size
 *      size of data in bytes
 */
unsigned int dory_get_tile_1d(
  unsigned x,
  int tile_ii,
  int tile_size_i,
  int data_size
) {
  unsigned int y = x + tile_ii*tile_size_i * data_size;
  return y;
}

/**
 *  @brief Gets a tile over a two-dimensional tiling grid.
 *
 *  Computes a pointer to the base of a particular tile in a two-dimensional
 *  tiling grid indexed by a (ii,jj) couple of indeces; in pseudo-Python
 *      ccn_get_tile_2d(x,ii,jj) = x[ii*si:(ii+1)*si-1,jj*sj:(jj+1)*sj-1]
 *  where (si,sj) is the couple defining the pitch of the tiling grid in the
 *  (i,j) dimensions.
 *
 *  @param *x
 *      a pointer to the base of the 2d tiling grid.
 *  @param tile_ii
 *      the tiling index in the outer dimension.
 *  @param tile_jj
 *      the tiling index in the inner dimension.
 *  @param tile_size_i
 *      the pitch of the tiling grid in the outer dimension, i.e. the distance
 *      between two "ticks" in the i dimension.
 *  @param tile_size_j
 *      the pitch of the tiling grid in the inner dimension, i.e. the distance
 *      between two "ticks" in the j dimension.
 *  @param tile_stride_j
 *      the total size of the tiling grid in the inner dimension, i.e. the
 *      number of ticks in the j dimension.
 *  @param data_size
 *      size of data in bytes
 */
unsigned int  dory_get_tile_2d(
  unsigned int x,
  int tile_ii,
  int tile_jj,
  int tile_size_i,
  int tile_size_j,
  int tile_stride_j,
  int data_size
) {
  unsigned int y = x + tile_ii*tile_size_i * tile_stride_j * data_size
                     + tile_jj*tile_size_j * data_size;
  return y;
}

/**
 *  @brief Gets a tile over a three-dimensional tiling grid.
 *
 *  Computes a pointer to the base of a particular tile in a three-dimensional
 *  tiling grid indexed by a (ii,jj,kk) triple of indeces; in pseudo-Python
 *      ccn_get_tile_3d(x,ii,jj,kk) =
 *        x[ii*si:(ii+1)*si-1, jj*sj:(jj+1)*sj-1, kk*sk:(kk+1)*sk-1]
 *  where (si,sj,sk) is the triple defining the pitch of the tiling grid in the
 *  (i,j,k) dimensions.
 *
 *  @param *x
 *      a pointer to the base of the 2d tiling grid.
 *  @param tile_ii
 *      the tiling index in the outer dimension.
 *  @param tile_jj
 *      the tiling index in the middle dimension.
 *  @param tile_kk
 *      the tiling index in the inner dimension.
 *  @param tile_size_i
 *      the pitch of the tiling grid in the outer dimension, i.e. the distance
 *      between two "ticks" in the i dimension.
 *  @param tile_size_j
 *      the pitch of the tiling grid in the middle dimension, i.e. the distance
 *      between two "ticks" in the j dimension.
 *  @param tile_size_k
 *      the pitch of the tiling grid in the inner dimension, i.e. the distance
 *      between two "ticks" in the k dimension.
 *  @param tile_stride_j
 *      the total size of the tiling grid in the middle dimension, i.e. the
 *      total number of ticks in the j dimension.
 *  @param tile_stride_k
 *      the total size of the tiling grid in the inner dimension, i.e. the
 *      total number of ticks in the k dimension.
 *  @param data_size
 *      size of data in bytes
 */
unsigned int  dory_get_tile_3d(
  unsigned int x,
  int tile_ii,
  int tile_jj,
  int tile_kk,
  int tile_size_i,
  int tile_size_j,
  int tile_size_k,
  int tile_stride_j,
  int tile_stride_k,
  int tile_overlap_i,
  int tile_overlap_j,
  int tile_overlap_k,
  int tile_offset_i,
  int tile_offset_j,
  int tile_offset_k,
  int data_size
) {
  unsigned int y = x + (tile_ii*(tile_size_i - tile_overlap_i) - tile_offset_i) * tile_stride_j * tile_stride_k * data_size / 8
                     + (tile_jj*(tile_size_j - tile_overlap_j) - tile_offset_j) * tile_stride_k * data_size / 8
                     + (tile_kk*(tile_size_k - tile_overlap_k) - tile_offset_k) * data_size / 8;
  return y;
}
// mchan_alloc(); Seems to put into a register that the DMA is about to be used. 
// mchan_free(); Puts a 0 into the DMA register.
// mchan_transfer(); Does the actual transfer.
// mchan_barrier(); Waits for the transfer to finish.

// For transfers in freeRTOS we have the following: 

/*
static inline void pi_cl_dma_memcpy(pi_cl_dma_copy_t *copy)
{
    __pi_cl_dma_1d_copy(CL_DMA_ID(0),
                        copy->ext, copy->loc, copy->size,
                        copy->dir, copy->merge, (pi_cl_dma_cmd_t *) copy);
}

static inline void pi_cl_dma_memcpy_2d(pi_cl_dma_copy_2d_t *copy)
{
    __pi_cl_dma_2d_copy(CL_DMA_ID(0),
                        copy->ext, copy->loc, copy->size,
                        copy->stride, copy->length,
                        copy->dir, copy->merge, (pi_cl_dma_cmd_t *) copy);
}

static inline void pi_cl_dma_wait(void *copy)
{
    __pi_cl_dma_wait(CL_DMA_ID(0), (pi_cl_dma_cmd_t *) copy);
}


static void mchan_transfer(
  unsigned int len, // lenght of transfer
  char type,   // direction of transfer 1. L2 to L1  0. L1 to L2
  char incr,   //  incre transfer 1. incremental 0. not incremental
  char twd_ext,  // 2-D transf
  char twd_tcdm,  // 2-D transf
  char ele,  // event line enable (set to 1)
  char ile,  // interrupt line enable (set to 0)
  char ble,  //  broadcast line enable (set to 0)
  unsigned int ext_addr, // L2 address  
  unsigned int tcdm_addr,  // L1 address
  unsigned int ext_count, // ext count (for single bit transfer set to 1) (maybe it is the length of 1-d transfer)
  unsigned int ext_stride, // ext stride(stride of the transfer)
  unsigned int tcdm_count,  // tcdm count (for single bit transfer set to 1) (maybe it is the length of 1-d transfer)
  unsigned int tcdm_stride) // tcdm stride (stride of the transfer)
  )


  1D: 
  mchan_transfer(
    DMA_copy_current.length_1d_copy * DMA_copy_current.number_of_1d_copies * DMA_copy_current.number_of_2d_copies, 
    DMA_copy_current.dir, 
    1, 
    0, 
    0, 
    1, 
    0, 
    0, 
    (unsigned int)(DMA_copy_current.ext), 
    (unsigned int)(DMA_copy_current.loc), 
    0, 
    0, 
    0, 
    0);
or:
mchan_transfer(
unsigned int len, 
char type, 
char incr, 
char twd, 
char ele, 
char ile, 
char ble, 
unsigned int ext_addr, 
unsigned int tcdm_addr, 
unsigned short int count, 
unsigned short int stride
*/