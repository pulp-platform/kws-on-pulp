#include "pmsis.h"

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