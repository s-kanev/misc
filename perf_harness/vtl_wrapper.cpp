#include <fstream>

#include "perflib.h"
#include "libittnotify.h"

__itt_frame pD;

void start_counters(void)
{
    __itt_frame_begin(pD);
//    __itt_resume();
}

void stop_counters(bool reset_counters)
{
    (void) reset_counters;
    __itt_frame_end(pD);
//    __itt_pause();
}

void pause_counters(void)
{
    __itt_frame_end(pD);
//    __itt_pause();
}

void init_counters(void)
{
    pD = __itt_frame_create("Counter frame");
}

void print_counters(std::ofstream &ofile)
{
    ofile.close();
}
