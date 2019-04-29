import scipy.sparse
import scipy.io
import random
import numpy

import argparse as ap


parser = ap.ArgumentParser(description="Generate matrix file")
parser.add_argument('-m', '--matrixfile', dest="path")
args = parser.parse_args()

matrix = scipy.io.mmread(args.path).tocsr().astype(numpy.float32)


def getrand():
    return  numpy.float32(random.random())


alpha = 1
beta = 0
cols = matrix.shape[0]
rows = matrix.shape[1]

x = numpy.array([getrand() for _ in range(0, rows)])
y = numpy.array([getrand() for _ in range(0, cols)])
solution = (matrix * alpha).dot(x)
solution1 = solution + (beta * y)

print("{} {} {} {} {}".format(len(matrix.data),rows, cols, alpha, beta))
print(" ".join([str(a) for a in matrix.data]))
print(" ".join([str(a) for a in matrix.indices]))
print(" ".join([str(a) for a in matrix.indptr]))
print(" ".join([str(a) for a in x]))
print(" ".join([str(a) for a in y]))
print(" ".join([str(a) for a in solution1]))