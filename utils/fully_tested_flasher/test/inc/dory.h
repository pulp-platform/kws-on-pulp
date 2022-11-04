
#include "mchan_test.h"
#include "pulp.h"
unsigned int dory_get_tile_1d(
  unsigned x,
  int tile_ii,
  int tile_size_i,
  int data_size
);
unsigned int dory_get_tile_2d(
  unsigned int x,
  int tile_ii,
  int tile_jj,
  int tile_size_i,
  int tile_size_j,
  int tile_stride_j,
  int data_size
);
unsigned int dory_get_tile_3d(
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
);

unsigned int dory_get_tile_4d(
  unsigned int x,
  int tile_ii,
  int tile_jj,
  int tile_kk,
  int tile_ll,
  int tile_size_i,
  int tile_size_j,
  int tile_size_k,
  int tile_size_l,
  int tile_stride_j,
  int tile_stride_k,
  int tile_stride_l,
  int tile_offset_i,
  int tile_offset_j,
  int tile_offset_k,
  int tile_offset_l,
  int data_size
);
void dory_dma_memcpy_3d(
  unsigned int ext,
  unsigned int loc,
  unsigned short size,
  unsigned short stride_1,
  unsigned short stride_0,
  unsigned short length_2,
  unsigned short length_0,
  rt_dma_dir_e dir,
  int merge,
  rt_dma_copy_t *copy
);

void  dory_dma_memcpy_3d_custom(
  unsigned int ext,
  unsigned int loc,
  unsigned short size,
  unsigned short stride_1,
  unsigned short stride_0,
  unsigned short length_2,
  unsigned short length_0,
  unsigned int dir,
  unsigned int *id
);