#include "pulp.h"

// Handle end of cluster request, by sending the reply to the cluster
void __pi_hyper_cluster_req_done(void *_req);

void __pi_hyper_copy_exec_mine(int channel, uint32_t addr, uint32_t hyper_addr, uint32_t size, pi_task_t *event);

void __pi_hyper_copy_mine(int channel,
  uint32_t addr, uint32_t hyper_addr, uint32_t size, pi_task_t *event, int mbr);

void __pi_hyper_cluster_req_exec_mine(pi_cl_hyper_req_t *req);

void __pi_hyper_cluster_req_done(void *_req);



void __pi_hyper_cluster_req_mine(void *_req);

void __rt_hyperram_cluster_copy_mine(rt_hyperram_t *dev,
  void *hyper_addr, void *addr, int size, rt_hyperram_req_t *req, int ext2loc);

void __cl_hyper_cluster_copy_mine(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_cl_hyper_req_t *req, int ext2loc);

void rt_hyperram_cluster_read_mine(rt_hyperram_t *dev,
  void *addr, void *hyper_addr, int size, rt_hyperram_req_t *req);

