/* ========================================================================== */
/* ========================================================================== */
/*                      
 * Generate regions around pinpoints that start and end near a function call
 * for easier and leaner instrumentation later.
 *            Svilen Kanev, 2011
 */
/* ========================================================================== */
/* ========================================================================== */

#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <unistd.h>

#include "pin.H"
#include "instlib.H"

#include "func_point.h"

using namespace INSTLIB;
using namespace std;


KNOB<string> KnobFuncSitesFile(KNOB_MODE_WRITEONCE,   "pintool",
        "func_file", "", "File where function points are written");

/* Icount at last function call */
UINT64 lastCount;
/* Address of last function call */
ADDRINT lastRoutine = 0;

/* ========================================================================== */
/* Pinpoint related */
// Track the number of instructions executed
ICOUNT icount;
// Track the number of calls for each function
map<ADDRINT, UINT64> rtn_counters;

// Contains knobs and instrumentation to recognize start/stop points
CONTROL control;

FuncPoint *lastFuncPoint;
list<FuncPoint*> allPoints;

ofstream outfile;


/* ========================================================================== */
VOID PPointHandler(CONTROL_EVENT ev, VOID * v, CONTEXT * ctxt, VOID * ip, THREADID tid)
{
    cerr << " ip: " << hex << ip << " "; 
    cerr <<  dec << " Inst. Count " << icount.Count(tid) << " ";

    switch(ev)
    {
      case CONTROL_START:
        cerr << "Start ";


        if(control.PinPointsActive())
        {
            lastFuncPoint = new FuncPoint();
            lastFuncPoint->index = control.CurrentPp(tid);
            lastFuncPoint->start_icount = lastCount;
            lastFuncPoint->start_func_addr = lastRoutine;
            lastFuncPoint->start_func_crossings = rtn_counters[lastRoutine];
            lastFuncPoint->weight_times_1000 = control.CurrentPpWeightTimesThousand();

            cerr << "PinPoint: " << control.CurrentPp(tid) << endl;
            cerr << "Last count: " << lastCount << " Diff: " << icount.Count() - lastCount << endl;
            cerr << "Last routine: " << hex << lastRoutine << dec << " crossings : " << rtn_counters[lastRoutine] << endl;
        }
        break;

      case CONTROL_STOP:
        cerr << "Stop ";

        if(control.PinPointsActive())
        {
            lastFuncPoint->end_icount = lastCount;
            lastFuncPoint->end_func_addr = lastRoutine;
            lastFuncPoint->end_func_crossings = rtn_counters[lastRoutine];
            allPoints.push_back(lastFuncPoint);

            cerr << "PinPoint: " << control.CurrentPp(tid) << endl;
            cerr << "Last count: " << lastCount << " Diff: " << icount.Count() - lastCount << endl;
            cerr << "Last routine: " << hex << lastRoutine << dec << " crossings : " << rtn_counters[lastRoutine] << endl;
        }
        break;

      default:
        ASSERTX(false);
        break;
    }
}

VOID Fini(INT32 code, VOID* arg)
{
    (VOID) arg;
    list<FuncPoint*>::iterator it;
    for (it = allPoints.begin(); it != allPoints.end(); it++)
        outfile << *(*it);
}

/* Remember every function call, its address and the number of calls so far */
VOID RtnStats(ADDRINT _lastRoutine)
{
    lastCount = icount.Count();
    lastRoutine = _lastRoutine;
    rtn_counters[_lastRoutine]++;
}

/* Analysis -- every function call */
VOID Routine(RTN rtn, VOID* arg)
{
    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE,
                    AFUNPTR(RtnStats),
                    IARG_ADDRINT, RTN_Funptr(rtn),
                    IARG_END);
    RTN_Close(rtn);
}

/* ========================================================================== */
INT32 main(INT32 argc, CHAR **argv)
{

    PIN_Init(argc, argv);
    PIN_InitSymbols();

    PIN_AddFiniFunction(Fini, 0);

    RTN_AddInstrumentFunction(Routine, 0);

    // Try activate pinpoints alarm, must be done before PIN_StartProgram
    if(control.CheckKnobs(PPointHandler, 0) != 1) {
        cerr << "Error reading control parametrs, exiting." << endl;
        return 1;
    }

    if(KnobFuncSitesFile.Value().empty()) {
        cerr << "No output file specified, exiting." << endl;
        return 1;
    }
    outfile.open(KnobFuncSitesFile.Value().c_str());


    icount.Activate();

    PIN_StartProgram();
    return 0;
}
