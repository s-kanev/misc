#! /usr/bin/python
import os.path
import subprocess, shlex

#Configuration params
PIN = "/home/skanev/pin/2.12/pin.sh"
TOOL = "/group/vlsiarch/skanev/misc/perf_harness/obj-ia32/harness.so"
FUNC_DIR = "/group/vlsiarch/skanev/cpu2006_sse_func"
OUT_DIR = "/group/vlsiarch/skanev/cpu2006_sse_neh_temp"

#Command-line builder and driver for the harness pintool
###########################################################
class PFMHarnessDriver(object):
    def __init__(self):
        self.cmd = PIN + " -injection child -separate_memory -t " + TOOL

    def AddFuncFile(self, fname):
        self.cmd += " -func_file " + fname

    def AddPoint(self, point_num):
        self.cmd += " -pnum " + str(point_num)

    def AddOutFile(self, fname):
        self.cmd += " -out_file " + fname

    def AddBmk(self, exe_dir, exe, args):
        exe = os.path.join(exe_dir, exe)
        self.cmd += " -- " + exe + " " + args

    def Exec(self, directory=".", redirin="", redirout="", redirerr=""):
        print "Executing: %s" % self.cmd

        if redirin != "":
            stdin = open(os.path.join(directory, redirin), "r")
        else:
            stdin = None

        if redirout != "":
            stdout = open(os.path.join(directory, redirout), "w")
        else:
            stdout = None

        if redirerr != "":
            stderr = open(os.path.join(directory, redirerr), "w")
        else:
            stderr = None

        child = subprocess.Popen(shlex.split(self.cmd), stdin=stdin, stdout=stdout, stderr=stderr, close_fds=True, cwd=directory)
        retcode = child.wait()

        if retcode == 0:
            print "Child completed successfully"
        else:
            print "Child failed! Error code: %d" % retcode

###########################################################
def RunPerfcount(bmk):
    func_file = os.path.join(FUNC_DIR, bmk.name +".func")
    npoints = sum(1 for line in open(func_file)) / 3

    for i in range(npoints):
        j = i+1
        pfm = PFMHarnessDriver()
        pfm.AddFuncFile(func_file)
        pfm.AddPoint(j)
        pfm.AddOutFile(os.path.join(OUT_DIR, bmk.name + ".perf." + str(j)))
        pfm.AddBmk(bmk.directory, bmk.executable, bmk.args)
        out_file = os.path.join(OUT_DIR, bmk.name + ".out."+str(j))
        err_file = os.path.join(OUT_DIR, bmk.name + ".err."+str(j))
        pfm.Exec(bmk.directory, redirin=bmk.input, redirout=out_file, redirerr=err_file)


###########################################################
import spec

if __name__ == "__main__":
#    RunPerfcount(spec.runs[1])
#    for run in spec.runs:
#        RunPerfcount(run)
    run = spec.GetRun("450.soplex.ref")
    RunPerfcount(run)
