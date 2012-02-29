#include "pin.H"
#include "instlib.H"

#include "perflib.h"

using namespace INSTLIB;
using namespace std;

KNOB<string> KnobOutFile(KNOB_MODE_WRITEONCE,   "pintool",
        "out_file", "", "File to store counter values");

ofstream ofile;

extern UINT32 flush_cache();
extern KNOB<BOOL> KnobFlushCaches;

/* ========================================================================== */
extern "C" VOID harness_start()
{
    if (KnobFlushCaches.Value())
        flush_cache();
    start_counters();
}

/* ========================================================================== */
extern "C" VOID harness_stop()
{
    pause_counters();
    print_counters(ofile);
    PIN_ExitProcess(0);
}

extern "C" VOID harness_init()
{
    init_counters();

    ofile.open(KnobOutFile.Value().c_str());
    if(ofile.fail()) {
        cerr << "Couldn't open out file: " << KnobOutFile.Value() << endl;
        PIN_ExitProcess(1);
    }

    start_counters();
    stop_counters(1);
}
