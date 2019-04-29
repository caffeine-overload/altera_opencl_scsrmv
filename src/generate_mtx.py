import scipy.sparse
import random
import numpy

import argparse as ap


parser = ap.ArgumentParser(description="Generate matrix file")
parser.add_argument('-r', '--rows', dest="rows")
parser.add_argument('-c', '--cols', dest="cols")
parser.add_argument('-d', '--density', dest="density")
parser.add_argument('-i', '--int', dest="int", action="store_true")
parser.add_argument('-n', '--num', dest="num")
args = parser.parse_args()

dtype = numpy.int8 if args.int else numpy.float32


def getrand():
    return random.randint(-10, 10) if args.int else numpy.float32(random.random())


for i in range(0, int(args.num)):
    matrix = scipy.sparse.random(int(args.rows), int(args.cols), density=float(args.density), format="csr", dtype=dtype)
    alpha = getrand()
    beta = getrand()
    x = numpy.array([getrand() for a in range(0, int(args.cols))])
    y = numpy.array([getrand() for a in range(0, int(args.rows))])
    solution = (matrix * alpha).dot(x)
    solution1 = solution + (beta * y)
    print("{} {} {} {} {}".format(len(matrix.data), int(args.rows), int(args.cols), alpha, beta))
    print(" ".join([str(a) for a in matrix.data]))
    print(" ".join([str(a) for a in matrix.indices]))
    print(" ".join([str(a) for a in matrix.indptr]))
    print(" ".join([str(a) for a in x]))
    print(" ".join([str(a) for a in y]))
    print(" ".join([str(a) for a in solution1]))
