# this is a polynomial regression, which one may use to predict 
# required cycling energy at a certain speed. Note that such 
# predictions tend to work well with polynomials of degree 4.
# 
# 
# Usage: python3 regression.py X_file.txt Y_file.txt d
# 
# where X_file.txt and Y_file.txt should each be a txt file 
# containing a series of decimals, representing speed and 
# power respectively, while d should be the degree of the 
# least-square-error polynomial Y = f(X). 
# 

import numpy as np
import sys

def poly_vander(X, n):
    return np.fliplr(np.vander(X, n))

def least_square(M, Y):
    return (np.linalg.inv((M.T)@M)) @ (M.T) @ (Y)

def poly_regression(X, Y, n):
    # create a vandermonde matrix for X
    M = np.fliplr(np.vander(X, n))

    # regression
    b = least_square(M, Y)
    return M, b


assert(len(sys.argv) == 4)
X_file, Y_file = sys.argv[1], sys.argv[2]
X_v, Y_v = np.loadtxt(X_file), np.loadtxt(Y_file)
degree = int(sys.argv[3])
assert(degree > 0)
mat, est = poly_regression(X_v, Y_v, degree)
square_err = np.sum((Y_v - mat @ est)**2)
print(f"model: {est}\nerror: {square_err}")
