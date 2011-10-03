#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include <perfmon/pfmlib_perf_event.h>
#include "perf_util.h"

static const char *default_counters[]={
    "PERF_COUNT_HW_CPU_CYCLES",
    "PERF_COUNT_HW_INSTRUCTIONS",
    "PERF_COUNT_HW_BRANCH_INSTRUCTIONS",
    "PERF_COUNT_HW_BRANCH_MISSES",
    "PERF_COUNT_HW_CACHE_L1D:ACCESS",
    "PERF_COUNT_HW_CACHE_L1D:MISS",
    NULL
};

perf_event_desc_t *fds = NULL;
int num_fds = 0;
uint64_t *counter_values = NULL;

int init_counters(const char** counters) {
    /* Initialize pfm library */
    int i,ret;

    ret = pfm_initialize();

    if (ret != PFM_SUCCESS) {
        fprintf(stderr, "Cannot initialize libpfm: %s\n", pfm_strerror(ret));
        return -1;
    }

    ret = perf_setup_argv_events(counters, &fds, &num_fds);
    if (ret || !num_fds) {
        fprintf(stderr, "Cannot setup events\n");
        return -1;
    }

    fds[0].fd = -1;
    for (i=0; i < num_fds; i++) {
        /* request timing information necessary for scaling */
        fds[i].hw.read_format = PERF_FORMAT_SCALE;
        fds[i].hw.disabled = 1; /* start paused */

        fds[i].fd = perf_event_open(&fds[i].hw, 0, -1, -1, 0);
        if (fds[i].fd == -1) {
            fprintf(stderr, "Cannot open event %d\n", i);
            return -1;
        }
    }

    counter_values = (uint64_t*) malloc(num_fds * sizeof(uint64_t));
}

int deinit_counters() {
    free(fds);

    free(counter_values);

    /* Free libpfm rsources */
    pfm_terminate();
}

void start_counters() {
    int ret;

    ret = prctl(PR_TASK_PERF_EVENTS_ENABLE);
    if (ret)
        fprintf(stderr, "prctl(enable) failed\n");
}

uint64_t *stop_counters() {
    int ret, i;
    uint64_t values[3];

    ret = prctl(PR_TASK_PERF_EVENTS_DISABLE);
    if (ret)
        fprintf(stderr, "prctl(disable) failed\n");

    /*
    * now read the results. We use pfp_event_count because
    * libpfm guarantees that counters for the events always
    * come first.
    */
    memset(values, 0, sizeof(values));

    for (i=0; i < num_fds; i++) {
        uint64_t val;
        double ratio;

        ret = read(fds[i].fd, values, sizeof(values));
        if (ret < sizeof(values)) {
            if (ret == -1)
                err(1, "cannot read results: %s", strerror(errno));
            else
                warnx("could not read event%d", i);
        }

        /*
         * scaling is systematic because we may be sharing the PMU and
         * thus may be multiplexed
         */
        val = perf_scale(values);
        ratio = perf_scale_ratio(values);

        counter_values[i] = val;
#ifdef VERBOSE
        printf("%'20"PRIu64" %s (%.2f%% scaling, ena=%'"PRIu64", run=%'"PRIu64")\n",
            val,
            fds[i].name,
            (1.0-ratio)*100.0,
            values[1],
            values[2]);
#endif

            close(fds[i].fd);
    }

    return counter_values;
}

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

    stop_counters();
    deinit_counters();

    return 0;
}
