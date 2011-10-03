#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "wrapped_pfm.h"

static const char *default_counters[]={
    "PERF_COUNT_HW_CPU_CYCLES",
    "PERF_COUNT_HW_INSTRUCTIONS",
    "PERF_COUNT_HW_BRANCH_INSTRUCTIONS",
    "PERF_COUNT_HW_BRANCH_MISSES",
    "PERF_COUNT_HW_CACHE_L1D:ACCESS",
    "PERF_COUNT_HW_CACHE_L1D:MISS",
    NULL
};

int fib(int n)
{
  if(n<=1)
    return 1;
  else
    return fib(n-1)+fib(n-2);
}

int main(int argc, char** argv) {

    init_counters(default_counters);
    start_counters();

    int i;
    for (i=0; i<33; i++)
        fib(i);

    long long *counters = stop_counters();

    for (i=0; i<6; i++)
        printf("%s: %lld\n", default_counters[i], counters[i]);
    deinit_counters();

    return 0;
}
