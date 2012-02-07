/* ========================================================================== */
/* ========================================================================== */
/*                      
 * Harness for performance counter capture.
              Svilen Kanev, 2011
 */
/* ========================================================================== */
/* ========================================================================== */

#include <iostream>
#include <iomanip>
#include <vector>

#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "pin.H"
#include "instlib.H"

#include "func_point.h"
#include "perflib.h"

using namespace INSTLIB;
using namespace std;


KNOB<string> KnobFuncSitesFile(KNOB_MODE_WRITEONCE,   "pintool",
        "func_file", "", "File to get function points");
KNOB<UINT32> KnobPointNum(KNOB_MODE_WRITEONCE,   "pintool",
        "pnum", "1", "Which point to harness");
KNOB<BOOL> KnobParsecHooks(KNOB_MODE_WRITEONCE,   "pintool",
        "parsec", "0", "Look for parsec ROI");
KNOB<string> KnobOutFile(KNOB_MODE_WRITEONCE,   "pintool",
        "out_file", "", "File to store counter values");
KNOB<BOOL> KnobFlushCaches(KNOB_MODE_WRITEONCE,   "pintool",
        "flush", "0", "Flush caches before collecting");

UINT32 start_count = 0;
UINT32 stop_count = 0;
UINT32 rtn_count = 0;

UINT32 desired_start_count;
UINT32 desired_stop_count;

AFUNPTR StartReplaced, StopReplaced, StartStopReplaced;

vector<FuncPoint*> allPoints;
FuncPoint* curr_point;

ifstream infile;
ofstream ofile;

// In 8-byte chunks
#define CACHE_SIZE (1024*1024)

// In bytes
#define LINE_SIZE 64

UINT8 dummy[LINE_SIZE * CACHE_SIZE] __attribute__ ((aligned(4096)));

/* Creates a chained list of random memory locations 
 * spaced by the cache line size. Hopping through them should render the cache
 * and prefetchers useless and always cause a miss. */
/* ========================================================================== */
VOID init_cache_flush()
{
    register UINT32 i,j;
    register ADDRINT *ptr, *old_ptr;

    memset(dummy, 0, LINE_SIZE*CACHE_SIZE * sizeof(dummy[0]));

    j = 0;
    old_ptr = (ADDRINT*) dummy;

    srand(time(NULL));
    while (j < CACHE_SIZE-1)
    {
        i = (rand() % CACHE_SIZE) * LINE_SIZE;
        ptr = (ADDRINT*)(dummy+i);
        if (*ptr == 0)
        {
            *old_ptr = (ADDRINT)ptr;
            old_ptr = ptr;
            j++;
        }
    }
    *ptr = 0;
}

/* ========================================================================== */
UINT32 flush_cache()
{
    register UINT32 i=0;
    register ADDRINT* ptr = (ADDRINT*)&dummy[0];
    register ADDRINT* old_ptr = (ADDRINT*)&dummy[0];

    while (*ptr != 0)
    {
        old_ptr = ptr;
        ptr = (ADDRINT*) *ptr;  // Get address of next cache line
                                // XXX: This should serialize execution
        *old_ptr = i;           // Overwrite cache line
        i++;
    }
    return *old_ptr;        // Return sth so we don't get optimized away
}

/* ========================================================================== */
extern "C" VOID harness_start()
{
    if (KnobFlushCaches.Value())
        flush_cache();
    cerr << "START COLLECTING" << endl;
    start_counters();
}

/* ========================================================================== */
extern "C" VOID harness_stop()
{
    cerr << "STOP COLLECTING" << endl;
    pause_counters();
    print_counters(ofile);
    PIN_ExitProcess(0);
}

/* XXX: The following functions are hand-optimized for minimal overhead.
 * The sequences compiled with -O3 result (importantly) in no stack 
 * adjustments and only clobbering eax, which is caller-save anyway. */
/* ========================================================================== */
/* Start capturing once we hit the correct call number */
VOID start_replace()
{
    __asm__ __volatile__ ("push %%eax":::);
    start_count++;
    if (start_count == desired_start_count) {
        __asm__ __volatile__ ("pusha":::);
        __asm__ __volatile__ ("call harness_start":::);
        __asm__ __volatile__ ("popa":::);
    }

    __asm__ __volatile__ ("pop %%eax":::);
    __asm__ __volatile__ ("jmp *%0"::"m"(StartReplaced):);
}
VOID start_ins()
{
    start_count++;
    if (start_count == desired_start_count)
        harness_start();
}
/* ========================================================================== */
/* Start capturing right from main */
VOID main_replace()
{
    __asm__ __volatile__ ("call harness_start":::);
    __asm__ __volatile__ ("jmp *%0"::"m"(StartReplaced):);
}

/* ========================================================================== */
/* Start capturing and exit after we find the right call number */
VOID stop_replace()
{
    __asm__ __volatile__ ("push %%eax":::);
    stop_count++;
    if (stop_count == desired_stop_count)
        __asm__ __volatile__ ("call harness_stop":::);

    __asm__ __volatile__ ("pop %%eax":::);
    __asm__ __volatile__ ("jmp *%0"::"m"(StopReplaced):);
}
VOID stop_ins()
{
    stop_count++;
    if (stop_count == desired_stop_count)
        harness_stop();
}

/* ========================================================================== */
/* Stop capturing right from _exit */
VOID exit_replace()
{
    __asm__ __volatile__ ("call harness_stop":::);
    __asm__ __volatile__ ("jmp *%0"::"m"(StopReplaced):);
}


/* ========================================================================== */
/* Same as above, but when start and stop are based on the same function counts */
VOID start_stop_replace()
{
    __asm__ __volatile__ ("push %%eax":::);
    rtn_count++;
    if (rtn_count == desired_stop_count) {
        __asm__ __volatile__ ("pusha":::);
        __asm__ __volatile__ ("call harness_stop":::);
        __asm__ __volatile__ ("popa":::);
    }
    if (rtn_count == desired_start_count) {
        __asm__ __volatile__ ("pusha":::);
        __asm__ __volatile__ ("call harness_start":::);
        __asm__ __volatile__ ("popa":::);
    }

    __asm__ __volatile__ ("pop %%eax":::);
    __asm__ __volatile__ ("jmp *%0"::"m"(StartStopReplaced):);
}
VOID start_stop_ins()
{
    rtn_count++;
    if (rtn_count == desired_stop_count)
        harness_stop();
    if (rtn_count == desired_start_count)
        harness_start();
}

/* ========================================================================== */
VOID FuncPointHooks(IMG img, VOID *v)
{
    // Separate start and stop instrumentation routines
    if (curr_point->start_func_addr != curr_point->end_func_addr)
    {
        RTN interestStartRtn;
        AFUNPTR replacementStartAddr;
        // Add instrumentation to start routine
        if (curr_point->start_func_addr == 0 &&
            curr_point->start_func_crossings == 0) {
            //Special case -- starting from beginning
                interestStartRtn = RTN_FindByName(img, "main");
                replacementStartAddr = AFUNPTR(main_replace);
            }
        else {
                interestStartRtn = RTN_FindByAddress(curr_point->start_func_addr);
                replacementStartAddr = AFUNPTR(start_replace);
            }

        if(RTN_Valid(interestStartRtn) && 
           RTN_IsSafeForProbedReplacement(interestStartRtn)) {
                StartReplaced = RTN_ReplaceProbed(interestStartRtn, replacementStartAddr);
                cerr << "Start point replaced." << endl;
        }

/*      if(!RTN_IsSafeForProbedInsertion(interestStartRtn)) {
            cerr << " Start routine cannot be probed: "
                 << hex << curr_point->start_func_addr << endl;
            exit(1);
        }
        RTN_InsertCallProbed(interestStartRtn, IPOINT_BEFORE, AFUNPTR(start_ins), IARG_END);*/

        RTN interestStopRtn;
        AFUNPTR replacementStopAddr;
        // Add instrumentation to stop routine
        if (curr_point->end_func_addr == (ADDRINT)-1 &&
            curr_point->end_func_crossings == 0) {
            // Special case -- stop routine is exit point
            interestStopRtn = RTN_FindByName(img, "_exit");
            replacementStopAddr = AFUNPTR(exit_replace);
        } else {
            interestStopRtn = RTN_FindByAddress(curr_point->end_func_addr);
            replacementStopAddr = AFUNPTR(stop_replace);
        }

        if(RTN_Valid(interestStopRtn) &&
           RTN_IsSafeForProbedReplacement(interestStopRtn)) {
            StopReplaced = RTN_ReplaceProbed(interestStopRtn, replacementStopAddr);
            cerr << "Stop point replaced." << endl;
        }

    /*  if(!RTN_IsSafeForProbedInsertion(interestStopRtn)) {
            cerr << " Stop routine cannot be probed: "
                 << hex << curr_point->end_func_addr << endl;
            exit(1);
        }
        RTN_InsertCallProbed(interestStopRtn, IPOINT_BEFORE, AFUNPTR(stop_ins), IARG_END);*/
    }
    else {
        // Add instrumentation to start-stop routine
        RTN interestRtn = RTN_FindByAddress(curr_point->start_func_addr);
        if(!RTN_Valid(interestRtn)) {
            cerr << " Start routine address invalid: "
                 << hex << curr_point->start_func_addr << endl;
            exit(1);
        }
        if(!RTN_IsSafeForProbedReplacement(interestRtn)) {
            cerr << " Start routine cannot be probed: "
                 << hex << curr_point->start_func_addr << endl;
            exit(1);
        }
        StartStopReplaced = RTN_ReplaceProbed(interestRtn, AFUNPTR(start_stop_replace));
        cerr << "Start/stop point replaced." << endl;
/*        if(!RTN_IsSafeForProbedInsertion(interestRtn)) {
            cerr << " Start routine cannot be probed: "
                 << hex << curr_point->start_func_addr << endl;
            exit(1);
        }
        RTN_InsertCallProbed(interestRtn, IPOINT_BEFORE, AFUNPTR(start_stop_ins), IARG_END);*/
    }
}

/* ========================================================================== */
VOID ParsecHooks(IMG img, VOID *v)
{
    // Add instrumentation to start routine
    RTN interestStartRtn = RTN_FindByName(img, "__parsec_roi_begin");
    if(!RTN_Valid(interestStartRtn))
        return;

    if(!RTN_IsSafeForProbedReplacement(interestStartRtn)) {
        cerr << " Start routine cannot be probed." << endl;
        exit(1);
    }
    StartReplaced = RTN_ReplaceProbed(interestStartRtn, AFUNPTR(start_replace));

    // Add instrumentation to stop routine
    RTN interestStopRtn = RTN_FindByName(img, "__parsec_roi_end");
    if(!RTN_Valid(interestStopRtn))
        return;

    if(!RTN_IsSafeForProbedReplacement(interestStopRtn)) {
        cerr << " Stop routine cannot be probed." << endl;
        exit(1);
    }
    StopReplaced = RTN_ReplaceProbed(interestStopRtn, AFUNPTR(stop_replace));
}


/* ========================================================================== */
INT32 main(INT32 argc, CHAR **argv)
{

    PIN_Init(argc, argv);
    PIN_InitSymbols();

    init_counters();

    if(!KnobParsecHooks.Value()) {
        if(KnobFuncSitesFile.Value().empty()) {
            cerr << "No function point file specified, exiting." << endl;
            return 1;
        }
        infile.open(KnobFuncSitesFile.Value().c_str());
        if(infile.fail()) {
            cerr << "Couldn't open input file: " << KnobFuncSitesFile.Value() << endl;
            return 1;
        }

        do {
            FuncPoint *point = new FuncPoint();
            infile >> *point;
            if(infile.eof() || !infile.good())
                break;

            allPoints.push_back(point);
        } while(true);

        if(KnobPointNum.Value() > allPoints.size()) {
            cerr << "Incorrect point index: " << KnobPointNum.Value() << endl;
            return 1;
        }

        curr_point = allPoints[KnobPointNum.Value()-1];
        desired_start_count = curr_point->start_func_crossings;
        desired_stop_count = curr_point->end_func_crossings;

        IMG_AddInstrumentFunction(FuncPointHooks, 0);
    }
    else {
        desired_start_count = 1;
        desired_stop_count = 1;

        IMG_AddInstrumentFunction(ParsecHooks, 0);
    }

    ofile.open(KnobOutFile.Value().c_str());
    if(ofile.fail()) {
        cerr << "Couldn't open out file: " << KnobOutFile.Value() << endl;
        return 1;
    }


    if (KnobFlushCaches.Value())
        init_cache_flush();

//    start_counters();
//    stop_counters(1);

    PIN_StartProgramProbed();

    return 0;
}
