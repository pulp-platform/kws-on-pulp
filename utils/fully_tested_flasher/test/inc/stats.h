#ifndef __PROF_STATS__
#define __PROF_STATS__
#endif
#define PROFILING
#ifdef PROFILING
#define INIT_PROFILING()       rt_perf_t perf2;

#define START_PROFILING() \
    rt_perf_init(&perf2);                       \
    rt_perf_conf(&perf2, (1<<RT_PERF_CYCLES));          \
    rt_perf_reset(&perf2);                      \
    rt_perf_stop(&perf2);                       \
    rt_perf_start(&perf2); \

#define STOP_PROFILING() \
     rt_perf_stop(&perf2);          \
     rt_perf_save(&perf2);          \
     int cid = rt_core_id();                    \
     printf("[%d] : num_cycles: %d\n",cid,rt_perf_get(&perf2, RT_PERF_CYCLES) ); \

#else
#define INIT_PROFILING()
#define START_PROFILING()
#define STOP_PROFILING()
#endif
