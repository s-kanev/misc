#include <fstream>
#include <iomanip>
#include "stdint.h"

#include "perflib.h"
#include "wrapped_pfm.h"

const int NCOUNTERS = 2;
const char *default_counters[NCOUNTERS+1]={
//    "PERF_COUNT_HW_BRANCH_INSTRUCTIONS",
//    "PERF_COUNT_HW_BRANCH_MISSES",
//    "PERF_COUNT_HW_CPU_CYCLES",
//    "PERF_COUNT_HW_INSTRUCTIONS",
//    "PERF_COUNT_HW_CACHE_L1D:ACCESS",
//    "PERF_COUNT_HW_CACHE_L1D:MISS",
//    "PERF_COUNT_HW_CACHE_L1I:ACCESS",
//    "PERF_COUNT_HW_CACHE_L1I:MISS",
    "L2_RQSTS",
    "L2_LINES_IN",
    NULL
};

static uint64_t *counters;

void start_counters(void)
{
    pfm_start_counters();
}

void stop_counters(bool reset_counters)
{
    counters = pfm_stop_counters(reset_counters);
}

void pause_counters(void)
{
    counters = pfm_pause_counters();
}

void init_counters(void)
{
    pfm_init_counters(default_counters);
}

void print_counters(std::ofstream& ofile)
{
    int i;
    for(i=0; i<NCOUNTERS; i++) {
        ofile << default_counters[i] << ": " << counters[i] << std::endl;
    }
    ofile.close();
}
