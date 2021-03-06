// Copyright © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.
#pragma once

// leastsquares perfoms batch least-squares regression, solving
// argmin_x | b - Ax |.  A is a matrix in row-major order
// (the standard C ordering) with nrows rows and ncols
// columns.  B is a vector with nrows elements and x
// is used to return the result, it must have ncols elements.
//
// false is returned on error, otherwise true is returned.
bool leastsquares(int nrows, int ncols, double A[], double b[], double x[]);