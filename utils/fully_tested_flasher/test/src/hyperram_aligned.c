#include "hyperram_aligned.h"

// If not NULL, this task is enqueued when the current transfer is finished.
RT_FC_TINY_DATA struct pi_task *__rt_hyper_end_task_mine;

// Following variables are used to reenqueue transfers to overcome burst limit.
// This is used directly by assebly to quickly reenqueue the transfer.
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_base_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_hyper_addr_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_addr_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_repeat_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_repeat_size_mine;

// Head and tail of the queue of pending transfers which were put on hold
// as a transfer was already on-going.
RT_FC_TINY_DATA struct pi_task *__rt_hyper_pending_tasks_mine;
RT_FC_TINY_DATA struct pi_task *__rt_hyper_pending_tasks_last_mine;

// All the following are used to keep track of the current transfer when it is
// emulated due to aligment constraints.
// The interrupt handler executed at end of transfer will execute the FSM to reenqueue
// a partial transfer.
RT_FC_TINY_DATA int __rt_hyper_pending_emu_channel_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_emu_hyper_addr_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_emu_addr_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_emu_size_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_emu_size_2d_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_emu_length_mine;
RT_FC_TINY_DATA unsigned int __rt_hyper_pending_emu_stride_mine;
RT_FC_TINY_DATA unsigned char __rt_hyper_pending_emu_do_memcpy_mine;
RT_FC_TINY_DATA struct pi_task *__rt_hyper_pending_emu_task_mine;

// Local task used to enqueue cluster requests.
// We cannot reuse the task coming from cluster side as it is used by the emulation
// state machine so we copy the request here to improve performance.
struct pi_task __pi_hyper_cluster_task_mine;
pi_cl_hyper_req_t *__pi_hyper_cluster_reqs_first_mine;
pi_cl_hyper_req_t *__pi_hyper_cluster_reqs_last_mine;

int __rt_hyper_open_count_mine;

typedef struct {
  rt_extern_alloc_t alloc;
  int channel;
  int cs;
  int type;
} pi_hyper_t;

// Handle end of cluster request, by sending the reply to the cluster
void __pi_hyper_cluster_req_done(void *_req);

void __pi_hyper_copy_exec_mine(int channel, uint32_t addr, uint32_t hyper_addr, uint32_t size, pi_task_t *event)
{
    __pi_hyper_copy_aligned(channel, addr, hyper_addr, size, event);
}


void __pi_hyper_copy_mine(int channel,
  uint32_t addr, uint32_t hyper_addr, uint32_t size, pi_task_t *event, int mbr)
{
  int irq = rt_irq_disable();

  hyper_addr |= mbr;

  if (__rt_hyper_end_task_mine != NULL || __rt_hyper_pending_emu_size_mine != 0)
  {
    if (__rt_hyper_pending_tasks_mine != NULL)
    __rt_hyper_pending_tasks_last_mine->implem.next = event;
    else
      __rt_hyper_pending_tasks_mine = event;
    __rt_hyper_pending_tasks_last_mine = event;
    event->implem.next = NULL;

    event->implem.data[0] = channel;
    event->implem.data[1] = (unsigned int)addr;
    event->implem.data[2] = (unsigned int)hyper_addr;
    event->implem.data[3] = size;
  }
  else
  {
    __pi_hyper_copy_exec_mine(channel, addr, hyper_addr, size, event);
  }

  rt_irq_restore(irq);
}

void __pi_hyper_cluster_req_exec_mine(pi_cl_hyper_req_t *req)
{
  pi_hyper_t *hyper = (pi_hyper_t *)req->device->data;
  pi_task_t *event = &__pi_hyper_cluster_task_mine;
  pi_task_callback(event, __pi_hyper_cluster_req_done, (void* )req);
  __pi_hyper_copy_mine(UDMA_CHANNEL_ID(hyper->channel) + req->is_write, (uint32_t)req->addr, req->hyper_addr, req->size, event, REG_MBR0);
}

void __pi_hyper_cluster_req_done(void *_req)
{
  pi_cl_hyper_req_t *req = (pi_cl_hyper_req_t *)_req;
  req->done = 1;
  __rt_cluster_notif_req_done(req->cid);
    __pi_hyper_cluster_reqs_first_mine = req->next;

  req = __pi_hyper_cluster_reqs_first_mine;
  if (req)
  {
    __pi_hyper_cluster_req_exec_mine(req);
  }
}



void __pi_hyper_cluster_req_mine(void *_req)
{
  pi_cl_hyper_req_t *req = (pi_cl_hyper_req_t* )_req;

  int is_first = __pi_hyper_cluster_reqs_first_mine == NULL;

  if (is_first)
    __pi_hyper_cluster_reqs_first_mine = req;
  else
    __pi_hyper_cluster_reqs_last_mine->next = req;

  __pi_hyper_cluster_reqs_last_mine = req;
  req->next = NULL;

  if (is_first)
  {
    __pi_hyper_cluster_req_exec_mine(req);
  }
}

void __rt_hyperram_cluster_copy_mine(rt_hyperram_t *dev,
  void *hyper_addr, void *addr, int size, rt_hyperram_req_t *req, int ext2loc)
{
  __cl_hyper_cluster_copy_mine((struct pi_device *)dev, (uint32_t)hyper_addr, addr, size, req, ext2loc);
}

void __cl_hyper_cluster_copy_mine(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, pi_cl_hyper_req_t *req, int ext2loc)
{
  req->device = device;
  req->addr = addr;
  req->hyper_addr = hyper_addr;
  req->size = size;
  req->cid = pi_cluster_id();
  req->done = 0;
  req->is_write = (ext2loc)? 0:1;
  req->is_2d = 0;
  __rt_task_init_from_cluster(&req->event);
  pi_task_callback(&req->event, __pi_hyper_cluster_req_mine, (void* )req);
  __rt_cluster_push_fc_event(&req->event);
}

void rt_hyperram_cluster_read_mine(rt_hyperram_t *dev,
  void *addr, void *hyper_addr, int size, rt_hyperram_req_t *req)
{
  __rt_hyperram_cluster_copy_mine(dev, hyper_addr, addr, size, req, 1);
}

