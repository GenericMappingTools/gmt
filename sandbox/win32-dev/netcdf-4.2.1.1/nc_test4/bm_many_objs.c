/** \file
This program benchmarks creating a netCDF file with many objects.

Copyright 2010, UCAR/Unidata See COPYRIGHT file for copying and
redistribution conditions.
*/

#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */

/* We will create this file. */
#define FILE_NAME "bm_many_objs.nc"

int main(int argc, char **argv)
{
    struct timeval start_time, end_time, diff_time;
    double sec;
    int nitem = 10000;		/* default number of objects of each type */
    int i;
    int ncid;
    int data[] = {42};
    int g, grp, numgrp;
    char gname[16];
    int v, var, numvar, vn, vleft, nvars;
    
    if(argc > 2) { 	/* Usage */
	printf("NetCDF performance test, writing many groups and variables.\n");
	printf("Usage:\t%s [N]\n", argv[0]);
	printf("\tN: number of objects\n");
	return(0);
    }
    for(i = 1; i < argc; i++) {
	nitem = atoi(argv[i]);
    }

    /*  create new file */
    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
    if (gettimeofday(&start_time, NULL))
	ERR;
    /* create N groups, printing time after every 1000 */
    numgrp = nitem;
    for(g = 1; g < numgrp + 1; g++) {
	sprintf(gname, "group%d", g);
	if (nc_def_grp(ncid, gname, &grp)) ERR;
	if (nc_def_var(grp, "var", NC_INT, 0, NULL, &var)) ERR;
	if(nc_enddef (grp)) ERR;
	if(nc_put_var(grp, var, data)) ERR;
	if(g%1000 == 0) {		/* only print every 1000th group name */
	    if (gettimeofday(&end_time, NULL)) ERR;
	    if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	    sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
	    printf("%s\t%.3g sec\n", gname, sec);
	}
    }
    nc_close(ncid);
    
    /*  create new file */
    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
    /* create N variables, printing time after every 1000.  Because only
     * NC_MAX_VARS are permitted per group, create the necessary number
     * of groups to hold nitem variables. */
    numvar = nitem;
    v = 1;
    numgrp = (numvar - 1) / NC_MAX_VARS + 1;
    vleft = numvar - (NC_MAX_VARS * (numgrp - 1));
    if (gettimeofday(&start_time, NULL))
	ERR;
    
    for(g = 1; g < numgrp + 1; g++) {
	sprintf(gname, "group%d", g);
	if (nc_def_grp(ncid, gname, &grp)) ERR;
	nvars = g < numgrp ? NC_MAX_VARS : vleft; /* leftovers on last time through */
	for(vn = 1; vn < nvars + 1; vn++) {
	    int var;
	    char vname[20];
	    sprintf(vname, "variable%d", v);
	    if(nc_def_var(grp, vname, NC_INT, 0, NULL, &var)) ERR;
	    if(nc_put_var(grp, var, data)) ERR;
	    if(v%1000 == 0) {		/* only print every 1000th variable name */
		if (gettimeofday(&end_time, NULL)) ERR;
		if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
		sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
		printf("%s/%s\t%.3g sec\n", gname, vname, sec);
	    }
	    v++;
	}
    }
    nc_close(ncid);
    return(0);
}
