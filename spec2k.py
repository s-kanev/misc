#!/usr/bin/env python
import os.path
import re
import subprocess, shlex

#Configuration params
specDir = '/group/vlsiarch/skanev/cpu2000'
specTune = 'peak'
specExt = 'llvm_O3'

def GetDir(bmk, suite):
    return os.path.join(specDir, suite, bmk, 'exe/run') 

regionRx = re.compile("^region")

class BenchmarkRun(object):

    def __init__(self, bmk, suite, executable, args, input, output, error, name):
        self.bmk = bmk
        self.suite = suite
        self.executable = '../../binaries/%s_%s.%s' % (executable, specTune, specExt)
        self.args = args
        self.input = input
        self.output = output
        self.error = error
        self.directory = GetDir(bmk, suite)
        self.name = "%s.%s" % (self.bmk, name)

    def GetPinPointsFFwd(self):
        ppfile = os.path.join(self.directory, self.name + ".pintool.1.pp")
        res = []
        with open(ppfile) as f:
            for line in f:
                m = regionRx.match(line)
                if m:
                    res.append(int(line.split()[6]))
        return res

    def GetCommand(self):
        cmd = "./" + self.executable
        cmd += " " + self.args
        if self.input:
            cmd = "cat " + self.input  + " | " + cmd
#        if self.output:
#            cmd += " > " + self.output
#        if self.error:
#            cmd += " 2> " + self.error
        return cmd

    def Exec(self):
        cmd = "%s %s" % (self.executable, self.args)
        print "Executing:", cmd

        if self.input != "":
            stdin = open(os.path.join(self.directory, self.input), "r")
        else:
            stdin = None

        if self.output != "":
            stdout = open(os.path.join(self.directory, self.output), "w")
        else:
            stdout = None

        if self.error != "":
            stderr = open(os.path.join(self.directory, self.error), "w")
        else:
            stderr = None

        child = subprocess.Popen(shlex.split(cmd), stdin=stdin, stdout=stdout, stderr=stderr, close_fds=True, cwd=self.directory)
        retcode = child.wait()

        if retcode == 0:
            print "Child completed successfully"
        else:
            print "Child failed! Error code: %d" % retcode

###########################################################
runs = [
        BenchmarkRun('177.mesa', 'CFP2000', 'mesa', '-frames 1000 -meshfile mesa.in -ppmfile mesa.ppm', '', '', '', 'ref'),
        BenchmarkRun('179.art', 'CFP2000', 'art', '-scanfile c756hel.in -trainfile1 a10.img -trainfile2 hc.img -stride 2 -startx 110 -starty 200 -endx 160 -endy 240 -objects 10', '', 'ref.1.out', 'ref.1.err', 'ref.1'),
        BenchmarkRun('179.art', 'CFP2000', 'art', '-scanfile c756hel.in -trainfile1 a10.img -trainfile2 hc.img -stride 2 -startx 470 -starty 140 -endx 520 -endy 180 -objects 10', '', 'ref.2.out', 'ref.2.err', 'ref.2'),
        BenchmarkRun('183.equake', 'CFP2000', 'equake', '', 'inp.in', 'inp.out', 'inp.err', 'ref'),
        BenchmarkRun('188.ammp', 'CFP2000', 'ammp', '', 'ammp.in', 'ammp.out', 'ammp.err', 'ref'),

        BenchmarkRun('164.gzip', 'CINT2000', 'gzip', 'input.source 60', '', 'input.source.out', 'input.source.err', 'source'),
        BenchmarkRun('164.gzip', 'CINT2000', 'gzip', 'input.log 60', '', 'input.log.out', 'input.log.err', 'log'),
        BenchmarkRun('164.gzip', 'CINT2000', 'gzip', 'input.graphic 60', '', 'input.graphic.out', 'input.graphic.err', 'graphic'),
        BenchmarkRun('164.gzip', 'CINT2000', 'gzip', 'input.random 60', '', 'input.random.out', 'input.random.err', 'random'),
        BenchmarkRun('164.gzip', 'CINT2000', 'gzip', 'input.program 60', '', 'input.program.out', 'input.program.err', 'program'),
        BenchmarkRun('175.vpr', 'CINT2000', 'vpr', 'net.in arch.in place.out dum.out -nodisp -place_only -init_t 5 -exit_t 0.005 -alpha_t 0.9412 -inner_num 2', '', 'place_log.out', 'place_log.err', 'place'),
        BenchmarkRun('175.vpr', 'CINT2000', 'vpr', 'net.in arch.in place.in route.out -nodisp -route_only -route_chan_width 15 -pres_fac_mult 2 -acc_fac 1 -first_iter_pres_fac 4 -initial_pres_fac 8', '', 'route_log.out', 'route_log.err', 'route'),
        BenchmarkRun('176.gcc', 'CINT2000', 'cc1', '166.i -o 166.s', '', '166.out', '166.err', '166'),
        BenchmarkRun('176.gcc', 'CINT2000', 'cc1', '200.i -o 200.s', '', '200.out', '200.err', '200'),
        BenchmarkRun('176.gcc', 'CINT2000', 'cc1', 'expr.i -o expr.s', '', 'expr.out', 'expr.err', 'expr'),
        BenchmarkRun('176.gcc', 'CINT2000', 'cc1', 'integrate.i -o integrate.s', '', 'integrate.out', 'integrate.err', 'integrate'),
        BenchmarkRun('176.gcc', 'CINT2000', 'cc1', 'scilab.i -o scilab.s', '', 'scilab.out', 'scilab.err', 'scilab'),
        BenchmarkRun('181.mcf', 'CINT2000', 'mcf', 'inp.in', '', 'inp.out', 'inp.err', 'ref'),
        BenchmarkRun('186.crafty', 'CINT2000', 'crafty', '', 'crafty.in', 'crafty.out', 'crafty.err', 'ref'),
        BenchmarkRun('197.parser', 'CINT2000', 'parser', '2.1.dict -batch', 'ref.in', 'ref.out', 'ref.err', 'ref'),
        BenchmarkRun('253.perlbmk', 'CINT2000', 'perlbmk', '-I./lib diffmail.pl 2 550 15 24 23 100', '', '2.550.15.24.23.100.out', '2.550.15.24.23.100.err', 'diffmail'),
        BenchmarkRun('253.perlbmk', 'CINT2000', 'perlbmk', '-I. -I./lib makerand.pl', '', 'makerand.out', 'makerand.err', 'makerand'),
        BenchmarkRun('253.perlbmk', 'CINT2000', 'perlbmk', '-I./lib splitmail.pl 850 5 19 18 1500', '', '850.5.19.18.1500.out', '850.5.19.18.1500.err', 'splitmail1'),
        BenchmarkRun('253.perlbmk', 'CINT2000', 'perlbmk', '-I./lib splitmail.pl 704 12 26 16 836', '', '704.12.26.16.83.out', '704.12.26.16.83.err', 'splitmail2'),
        BenchmarkRun('253.perlbmk', 'CINT2000', 'perlbmk', '-I./lib splitmail.pl 535 13 25 24 1091', '', '535.13.25.24.1091.out', '535.13.25.24.1091.err', 'splitmail3'),
        BenchmarkRun('253.perlbmk', 'CINT2000', 'perlbmk', '-I./lib splitmail.pl 957 12 23 26 1014', '', '957.12.23.26.1014.out', '957.12.23.26.1014.err', 'splitmail4'),
        BenchmarkRun('254.gap', 'CINT2000', 'gap', '-l ./ -q -m 192M', 'ref.in', 'ref.out', 'ref.err', 'ref'),
        BenchmarkRun('255.vortex', 'CINT2000', 'vortex', 'lendian1.raw', '', 'vortex1.out', 'vortex1.err', 'ref1'),
#        BenchmarkRun('255.vortex', 'CINT2000', 'vortex', 'lendian2.raw', '', 'vortex2.out', 'vortex2.err', 'ref2'),
#        BenchmarkRun('255.vortex', 'CINT2000', 'vortex', 'lendian3.raw', '', 'vortex3.out', 'vortex3.err', 'ref3'),
        BenchmarkRun('256.bzip2', 'CINT2000', 'bzip2', 'input.source 58', '', 'input.source.out', 'input.source.err', 'source'),
        BenchmarkRun('256.bzip2', 'CINT2000', 'bzip2', 'input.graphic 58', '', 'input.graphic.out', 'input.graphic.err', 'graphic'),
        BenchmarkRun('256.bzip2', 'CINT2000', 'bzip2', 'input.program 58', '', 'input.program.out', 'input.program.err', 'program'),
        BenchmarkRun('300.twolf', 'CINT2000', 'twolf', 'ref', '', 'ref.stdout', 'ref.stderr', 'ref'),
       ]

###########################################################
def GetRun(name):
    res = None
    for curr_run in runs:
        if curr_run.name == name:
            res = curr_run
            break
    return res
