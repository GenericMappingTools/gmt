/*
Copyright 2007, UCAR/Unidata
See COPYRIGHT file for copying and redistribution conditions.

This program benchmarks the write and read of reasonable large files,
with different chunking parameters set.

$Id$
*/
#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <math.h>

/* This is one megabyte (2^20), in decimal. */
#define MEGABYTE 1048576

/* Define this to debug. */
#define DIM_LEN (MEGABYTE*32)
/*#define DIM_LEN (1024*8)*/

/* Size, in bytes, of a double. */
#define DOUBLE_SIZE 8

/* We will create this file. */
#define FILE_NAME "bm_chunking.nc"

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


int
main(int argc, char **argv)
{
   struct timeval start_time, end_time, diff_time;
   printf("\n*** Benchmarking chunking.\n");

   printf("\n*** Trying different chunksizes in 4 var, 1D file with "
	  "large dimension (%d)...\n", DIM_LEN);
   {
#define DIM_NAME "Some_really_long_dimension"
#define NUMDIMS 1
#define NUMVARS 4
#define NUM_ATTEMPTS 10
#define NUM_VALUES_TO_WRITE (DIM_LEN/1024)

      int ncid, dimids[NUMDIMS], varid[NUMVARS], chunksize[NUMDIMS];
      char var_name[NUMVARS][NC_MAX_NAME + 1] = {"England", "Ireland", "Scotland", "Wales"};
      size_t start[NUMDIMS], count[NUMDIMS] = {NUM_VALUES_TO_WRITE};
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype;
      char name_in[NC_MAX_NAME + 1];
      size_t len;
      double pi = 3.1459;
      double data[NUM_VALUES_TO_WRITE], data_in[NUM_VALUES_TO_WRITE];
      int a, i, j; 

      /* Generate phoney data. */
      for (i = 0; i < NUM_VALUES_TO_WRITE; i++)
	 data[i] = pi * i;

      /* Try NUM_ATTEMPTS different values of chunksize, timing each one. */
      for (a = 0; a < NUM_ATTEMPTS; a++)
      {
	 /* Create a netCDF netCDF-4/HDF5 format file, with 4 vars. */
	 if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
	 if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;
	 if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, dimids)) ERR;
       
	 /* According to Quncey, a square chunk around 1 MB in size is a
	  * reasonable idea. */
	 chunksize[0] = DIM_LEN/pow(2, a+6);
	 for (i = 0; i < NUMVARS; i++)
	 {
	    if (nc_def_var(ncid, var_name[i], NC_DOUBLE, NUMDIMS, 
			   dimids, &varid[i])) ERR;
	    if (nc_def_var_chunking(ncid, i, NC_CHUNKED, chunksize)) ERR;
	 }
	 if (nc_enddef(ncid)) ERR;
	  
	 /* Write the data and time it. */
	 if (gettimeofday(&start_time, NULL))
	    ERR;
	 for (start[0] = 0; start[0] < DIM_LEN; start[0] += NUM_VALUES_TO_WRITE)
	    for (i = 0; i < NUMVARS; i++)
	       if (nc_put_vara_double(ncid, i, start, count, data)) ERR;

	 if (nc_close(ncid)) ERR;
	 if (gettimeofday(&end_time, NULL)) ERR;
	 if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	 printf("chunksize %d write time %d sec %d usec\n", chunksize[0], 
		(int)diff_time.tv_sec, (int)diff_time.tv_usec);
	  
	 /* Reopen and check the file metadata. */
	 if (nc_open(FILE_NAME, 0, &ncid)) ERR;
	 if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
	 if (ndims != NUMDIMS || nvars != NUMVARS || natts != 0 || unlimdimid != -1) ERR;
	 if (nc_inq_dimids(ncid, &ndims, dimids, 1)) ERR;
	 if (ndims != 1 || dimids[0] != 0) ERR;
	 if (nc_inq_dim(ncid, 0, name_in, &len)) ERR;
	 if (strcmp(name_in, DIM_NAME) || len != DIM_LEN) ERR;
	 for (i = 0; i < NUMVARS; i++)
	 {
	    if (nc_inq_var(ncid, i, name_in, &xtype, &ndims, dimids, &natts)) ERR;
	    if (strcmp(name_in, var_name[i]) || xtype != NC_DOUBLE || ndims != 1 || 
		dimids[0] != 0 || natts != 0) ERR;
	 }

	 /* Read the data and time it. */
	 if (gettimeofday(&start_time, NULL)) ERR;
	 for (start[0] = 0; start[0] < DIM_LEN; start[0] += NUM_VALUES_TO_WRITE)
	    for (i = 0; i < NUMVARS; i++)
	       if (nc_get_vara_double(ncid, i, start, count, data_in)) ERR;
	 if (gettimeofday(&end_time, NULL)) ERR;
	 if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	 printf("chunksize %d read time %d sec %d usec\n", chunksize[0], 
		(int)diff_time.tv_sec, (int)diff_time.tv_usec);

	 /* Reread, and check the data (but don't time this operation). */
	 for (start[0] = DIM_LEN - NUM_VALUES_TO_WRITE; start[0] > 0; start[0] -= NUM_VALUES_TO_WRITE)
	    for (i = 0; i < NUMVARS; i++)
	    {
	       if (nc_get_vara_double(ncid, i, start, count, data_in)) ERR;
	       for (j = 0; j < NUM_VALUES_TO_WRITE; j++)
		  if (data[j] != data_in[j]) ERR;
	    }
	  

	 if (nc_close(ncid)) ERR;
      }
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

