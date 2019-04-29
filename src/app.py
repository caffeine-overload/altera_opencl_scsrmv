import argparse as ap
from os import listdir
import os
from os.path import isfile, join, basename
import re

parser = ap.ArgumentParser(description="Benchmark Your CL")
parser.add_argument('-D', '--dir', dest="bin_dir")
parser.add_argument('-n', '--num', dest="reps")
parser.add_argument('-b', '--binary', dest="bin", action="store_true")
parser.add_argument('-p', '--platform', dest="plat")
parser.add_argument('-d', '--device', dest="dev")
parser.add_argument('-v', '--verbose', dest="verbose", action="store_true")
parser.add_argument('-H', '--host-mem', dest='host_mem')
parser.add_argument('-r', '--reps', dest='reps')
parser.add_argument('-s', '--signalfiledir', dest='sigfile')
parser.add_argument('-T', '--datatype', dest='datatype')
parser.add_argument('-P', '--padding', dest="pad")
parser.add_argument('-F', '--filter', dest='filter')

args = parser.parse_args()
reps = args.reps or 1


files = [join(args.bin_dir, f) for f in listdir(args.bin_dir) if isfile(join(args.bin_dir, f)) and not "special" in os.path.splitext(basename(f))[0]]
if args.filter:
    if args.verbose:
        print("Filtering by")
        print(args.filter)
    files = [f for f in files if args.filter in f]

allsigfiles = [join(args.sigfile, f) for f in listdir(args.sigfile) if isfile(join(args.sigfile, f))]


if args.verbose:
    print(files)


for f in files:
    for l in allsigfiles:
        runstr = "../bin/a {} {} {} {} {} {} {} {} {} {}".format(
            f,
            l,
            "b" if args.bin else "s",
            args.plat,
            args.dev,
            '1',
            args.host_mem,
            args.datatype,
            args.pad,
            args.reps
        )
        if args.verbose:
            print(runstr)
        os.system(runstr)

#./bin/a kernel_bin/preprocessed/fir_1_int_basic.cl s 32 20 1 1 1 1 1 1 d 1 0 int ./data/data1.txt

#python app.py --dir ../kernel_bin/preprocessed/ --filter int --platform 1 --device 0 --lengths 32 --reps 10 --datatype int --signalfile ../data/data1.txt