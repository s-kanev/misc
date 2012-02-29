/* ========================================================================== */
/* ========================================================================== */
/*                      
 * Harness for performance counter capture.
              Svilen Kanev, 2011
 */
/* ========================================================================== */
/* ========================================================================== */

//#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pin.H"
#include "instlib.H"

#include "util.h"

#include "duty.c" //So, judge me

using namespace std;

KNOB<UINT32> KnobNumDuty(KNOB_MODE_WRITEONCE, "pintool",
        "n", "100", "How many iterations to inject");
KNOB<string> KnobOFile(KNOB_MODE_WRITEONCE, "pintool",
        "ofile", "", "Where to dump the output file");
KNOB<string> KnobLogFile(KNOB_MODE_WRITEONCE, "pintool",
        "harness_log", "", "Log file");

INT32 socket_fd = -1;

ofstream logfile;

#define LOGMSG(msg) { logfile << msg << endl; }

//#define LOGMSG(msg) 
/* ========================================================================== */
VOID scope_send(const char *msg)
{
    ssize_t jnk = write(socket_fd, msg, strlen(msg));
    (void) jnk;
}

/* ============================================================================ */
string scope_recv(const char* smsg)
{
    string msg;
    char buff[1024];
    size_t cnt = 0;

    scope_send(smsg);

    for(;;)
    {
        bzero(buff, sizeof(buff));
        cnt = read(socket_fd, (char*) buff, sizeof(buff));
        LOGMSG(smsg << " rec " << cnt);
        /* wait until we get all the data, which is marked using a linefeed */
        msg += string(buff).substr(0, cnt);

        sleep(1);
        size_t pos = msg.find('\n');
        if (pos == string::npos)
            continue;

        /* coming here means we've received all data, so now process it */
        msg = msg.substr(0, pos);
        break;
    }

    return msg;
}

/* ============================================================================ */
int scope_connect()
{
    const char *ip = "140.247.62.253"; /* from net configuration */
    int port = 5025;             /* from manual */
    struct sockaddr_in address;
    int result;
    int len;

    /* create a socket for the client */
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(!socket_fd)
    {
        LOGMSG("Bad Socket\n");
        exit(1);
    }

    /* name the socket as agreed with the server */
    address.sin_family = AF_INET;

    /* enter the IP Address of the instrument. */
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);

    /* now connect our socket to the server's socket */
    len = sizeof(address);
    result = connect(socket_fd, (struct sockaddr *)&address, len);
    if (result == -1)
    {
        LOGMSG("Socket connection Error\n ");
        exit(1);
    }

    /*Set the TCP_IP NODELAY option to 1 */
    setsockopt(socket_fd, IPPROTO_TCP, 1, "TRUE", 1);
    sleep(1);

    return socket_fd;
}

/* ============================================================================ */
void scope_disconnect()
{
    scope_send(":STOP\r\n");

    close(socket_fd);
    sleep(1);
}

/* ===================================================================== */
void scope_sync()
{
    LOGMSG("Stall\n");
    do 
    {
        sleep(1);
    }
    while (atoi(scope_recv("*OPC?\r\n").c_str()) == 0);
    LOGMSG("Stall complete\n");
}

/* ===================================================================== */
void scope_setup()
{
    LOGMSG("Setting up SCOPE\n");

    socket_fd = scope_connect();

    scope_send("*RST\r\n");
    scope_send("*CLS\r\n");
    scope_send("STOP\r\n");
    scope_send("*SRE 32; *ESE 1;\r\n");

    scope_send(":SYST:HEAD OFF\r\n");

    scope_send(":CHAN1:RANG 250E-3; OFFS 140E-3\r\n");

    scope_send(":TIM:REF LEFT; RANG 320E-3; POS 0\r\n");

    scope_send(":TRIG:EDGE:SOUR CHAN1; SLOP POS\r\n");
    scope_send(":TRIG:LEV CHAN1,110E-3\r\n");
    scope_send(":TRIG:SWE AUTO\r\n");
    scope_send(":DISP:CONN OFF\r\n");

    //scope_send(":ACQ:MODE RTIM; SRAT 100E6; AVER OFF\r\n");
    //scope_send(":ACQ:MODE RTIM; SRAT 32000E3; AVER OFF\r\n");
    scope_send(":ACQ:MODE RTIM; SRAT 160E3; AVER ON; AVER:COUN 2\r\n");

    scope_sync();
    LOGMSG("Setting up SCOPE complete\n");

//    sleep(1);
}

/* ===================================================================== */
void scope_snapshot(void)
{
    LOGMSG("Snapshotting SCOPE\n");

/*    scope_send(":WAV:SOUR CHANNEL1\r\n");
    scope_send(":WAV:FORM ASC\r\n");
    scope_send(":WAV:STR ON\r\n");
*/
    scope_send(":DIG CHAN1\r\n");
    scope_send(":CHAN1:DISP ON\r\n");
    //scope_send(":SINGLE\r\n");
    //scope_send(":DIG\r\n");
}

/* ===================================================================== */
void scope_grab_trace(const string& ofilename)
{
    LOGMSG("Starting download from scope\n");

    scope_send(":WAV:SOUR CHANNEL1\r\n");
    scope_send(":WAV:FORM ASC\r\n");
    scope_send(":WAV:STR ON\r\n");
/*
    scope_send(":SINGLE\r\n");
    scope_send(":DIG\r\n");
*/
    scope_sync();

    LOGMSG("Sending data? request \n");
    string msg = scope_recv(":WAV:DATA?\r\n");
    vector<string> v = Tokenize(msg, ",");

    LOGMSG("Received all data\n");
    float xorig = atof(scope_recv(":WAV:XOR?\r\n").c_str());
    float xinc = atof(scope_recv(":WAV:XINC?\r\n").c_str());
    LOGMSG("Got params to process raw data \n");

    ofstream ofile;

    // dump regular histogram
    string filename = ofilename;
    LOGMSG("Downloading trace data " + filename + "\n");
    ofile.open(filename.c_str());
    ASSERTX(ofile.is_open());
    for (unsigned int i = 0; i < v.size() - 1; i++)
    {
        float binval = (i * xinc) + xorig;
        ofile << binval << ", " << atof(v[i].c_str()) << endl;
    }
    ofile.close();
}

/* ========================================================================== */
void scope_save_waveform(const string &ofilename)
{
    LOGMSG("Dumping scope waveform to file " << ofilename);
    stringstream fname;
    fname << ":DISK:SAVE:WAVEFORM CHANNEL1,\"Y:\\xiosim_data\\";
    fname << ofilename << "\",CSV,ON\r\n";
    scope_send(fname.str().c_str());
}

/* ========================================================================== */
VOID Fini(INT32 exitCode, VOID* arg)
{
//    scope_grab_trace("tst.out");
    LOGMSG("Fini\n");
}

/* ========================================================================== */
VOID sig_handler(int signal)
{
    Fini(signal, 0);
}

/* ========================================================================== */
extern "C" VOID harness_stop()
{
//    ofile.close();
    duty(6);
    LOGMSG("Stopping harness");
//    scope_grab_trace("tst.out");
    scope_save_waveform(KnobOFile.Value());
//    scope_sync();
    scope_disconnect();
    logfile.close();
    PIN_ExitProcess(0);
}

/* ========================================================================== */
extern "C" VOID harness_start()
{
    scope_snapshot();
    duty(KnobNumDuty.Value());
    //scope_send(":CHAN1:DISP ON\r\n");
//    scope_disconnect();
}

/* ========================================================================== */
extern "C" VOID harness_init()
{
    if (KnobOFile.Value().empty()) {
        cerr << "Output file missing!" << endl;
        PIN_ExitProcess(1);
    }

    if (KnobLogFile.Value().empty()) {
        cerr << "Log file missing!" << endl;
        PIN_ExitProcess(1);
    }

    logfile.open(KnobLogFile.Value().c_str());
    if (!logfile.good()) {
        cerr << "Couldn't open log file: " << KnobLogFile.Value() << endl;
        PIN_ExitProcess(1);
    }

    PIN_AddFiniFunction(Fini, NULL);
    signal(SIGINT, sig_handler);

    scope_setup();
}
