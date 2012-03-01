/*
Copyright 2010, UCAR/Unidata
See COPYRIGHT file for copying and redistribution conditions.

This program benchmarks creating a netCDF file with many objects.

$Id $
*/

#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */

/* We will create this file. */
#define FILE_NAME "bm_many_atts.nc"

/* Subtract the `struct timeval' values X and Y, storing the result in
   RESULT.  Return 1 if the difference is negative, otherwise 0.  This
   function from the GNU documentation. */
int
timeval_subtract (result, x, y)
   struct timeval *result, *x, *y;
{
   /* Perform the carry for the later subtraction by updating Y. */
   if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
      y->tv_usec -= 1000000 * nsec;
      y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > 1000000) {
      int nsec = (x->tv_usec - y->tv_usec) / 1000000;
      y->tv_usec += 1000000 * nsec;
      y->tv_sec -= nsec;
   }

   /* Compute the time remaining to wait.
      `tv_usec' is certainly positive. */
   result->tv_sec = x->tv_sec - y->tv_sec;
   result->tv_usec = x->tv_usec - y->tv_usec;

   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
}


int main(int argc, char **argv)
{
    struct timeval start_time, end_time, diff_time;
    double sec;
    int nitem = 1000;		/* default number of objects of each type */
    int i;
    int ncid;
    int data[] = {42};
    int g, grp, numgrp;
    char gname[16];
    int a, numatt, an, aleft, natts;
    
    if(argc > 2) { 	/* Usage */
	printf("NetCDF performance test, writing many groups, variables, and attributes.\n");
	printf("Usage:\t%s [N]\n", argv[0]);
	printf("\tN: number of objects\n");
	return(0);
    }
    for(i = 1; i < argc; i++) {
	nitem = atoi(argv[i]);
    }
    
    /*  create new file */
    if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
    /* create N group/global attributes, printing time after every 100.
     * Because only NC_MAX_ATTRS are permitted per group, create the
     * necessary number of groups to hold nitem attributes. */
    numatt = nitem;
    a = 1;
    numgrp = (numatt - 1) / NC_MAX_ATTRS + 1;
    aleft = numatt - (NC_MAX_ATTRS * (numgrp - 1));
    if (gettimeofday(&start_time, NULL))
	ERR;

    for(g = 1; g < numgrp + 1; g++) {
	sprintf(gname, "group%d", g);
	if (nc_def_grp(ncid, gname, &grp)) ERR;
	natts = g < numgrp ? NC_MAX_ATTRS : aleft; /* leftovers on last time through */
	for(an = 1; an < natts + 1; an++) {
	    char aname[20];
	    sprintf(aname, "attribute%d", a);
	    if (nc_put_att_int(grp, NC_GLOBAL, aname, NC_INT, 1, data)) ERR;
	    if(a%100 == 0) {		/* only print every 100th attribute name */
		if (gettimeofday(&end_time, NULL)) ERR;
		if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
		sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
		printf("%s/%s\t%.3g sec\n", gname, aname, sec);
	    }
	    a++;
	}
    }
    nc_close(ncid);
    return(0);
}
