/*
 *	Copyright (c) 2015-2021 by P. Wessel and J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 */

/* 1. Must include mex.h since we use mxArray */
#include "mex.h"
/* 2. Must list extern GMTMEX mexfunction available from libgmt */
extern void GMT_mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]);
/* 3. The minimal gateway function called by Matlab */
void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	GMT_mexFunction (nlhs, plhs, nrhs, prhs);
}
