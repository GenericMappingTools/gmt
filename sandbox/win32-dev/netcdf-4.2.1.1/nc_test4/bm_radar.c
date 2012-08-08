/*
 Copyright 2007, UCAR/Unidata
 See COPYRIGHT file for copying and redistribution conditions.

This program benchmarks the write and read of some radar files with
different chunking and compression parameters set.

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* This is one megabyte (2^20), in decimal. */
#define MEGABYTE 1048576

/* We will create this file. */
#define FILE_NAME "bm_radar.nc"

int 
file_size(char* name)
{
   struct stat stbuf;
   stat(name, &stbuf);
   return stbuf.st_size;
}

/* Copy a netCDF file, changing cmode if desired. */
static
int copy_file(char *file_name_in, char *file_name_out, int cmode_out,
	      int *chunking, int *deflate)
{
   int ncid_in, ncid_out;
   int natts, nvars, ndims, unlimdimid;
   char name[NC_MAX_NAME + 1];
   size_t len;
   int a, v, d;

   if (nc_open(file_name_in, NC_NOWRITE, &ncid_in)) ERR;
   if (nc_create(file_name_out, cmode_out, &ncid_out)) ERR;
      
   if (nc_inq(ncid_in, &ndims, &nvars, &natts, &unlimdimid)) ERR;

   /* Copy dims. */
   for (d = 0; d < ndims; d++)
   {
      if (nc_inq_dim(ncid_in, d, name, &len)) ERR;
      if (nc_def_dim(ncid_out, name, len, NULL)) ERR;
   }

   /* Copy global atts. */
   for (a = 0; a < natts; a++)
   {
      if (nc_inq_attname(ncid_in, NC_GLOBAL, a, name)) ERR;
      if (nc_copy_att(ncid_in, NC_GLOBAL, name, ncid_out, NC_GLOBAL)) ERR;
   }

   /* Copy the variable metadata. */
   for (v = 0; v < nvars; v++)
   {
      char name[NC_MAX_NAME + 1];
      char att_name[NC_MAX_NAME + 1];
      nc_type xtype;
      int ndims, dimids[NC_MAX_DIMS], natts;
      int varid_out;
      int a;
      int retval = NC_NOERR;

      /* Learn about this var. */
      if ((retval = nc_inq_var(ncid_in, v, name, &xtype, &ndims, dimids, &natts)))
	 return retval;

      /* Create the output var. */
      if (nc_def_var(ncid_out, name, xtype, ndims, dimids, &varid_out)) ERR;

      /* Except for 1D vars, sent chunking and compression. */
      if (ndims != 1)
      {
	 if (chunking)
	    if (nc_def_var_chunking(ncid_out, v, NC_CHUNKED, chunking)) ERR;
	 if (deflate)
	    if (nc_def_var_deflate(ncid_out, v, NC_NOSHUFFLE, *deflate, *deflate)) ERR;
      }

      /* Copy the attributes. */
      for (a=0; a<natts; a++)
      {
	 if (nc_inq_attname(ncid_in, v, a, att_name)) ERR;
	 if (nc_copy_att(ncid_in, v, att_name, ncid_out, varid_out)) ERR;
      }
   }

   /* Copy the variable data. */
   for (v = 0; v < nvars; v++)
   {
      char name[NC_MAX_NAME + 1];
      nc_type xtype;
      int ndims, dimids[NC_MAX_DIMS], natts, real_ndims;
      int d;
      void *data = NULL;
      size_t *count = NULL, *start = NULL;
      size_t reclen = 1;
      size_t *dimlen = NULL;
      int retval = NC_NOERR;
      size_t type_size;
      char type_name[NC_MAX_NAME+1];

      /* Learn about this var. */
      if ((retval = nc_inq_var(ncid_in, v, name, &xtype, &ndims, dimids, &natts)))
	 return retval;

      /* Later on, we will need to know the size of this type. */
      if ((retval = nc_inq_type(ncid_in, xtype, type_name, &type_size)))
	 return retval;
      LOG((3, "type %s has size %d", type_name, type_size));

      /* Allocate memory for our start and count arrays. If ndims = 0
	 this is a scalar, which I will treat as a 1-D array with one
	 element. */
      real_ndims = ndims ? ndims : 1;
      if (!(start = nc_malloc(real_ndims * sizeof(size_t))))
	 BAIL(NC_ENOMEM);
      if (!(count = nc_malloc(real_ndims * sizeof(size_t))))
	 BAIL(NC_ENOMEM);

      /* The start array will be all zeros, except the first element,
	 which will be the record number. Count will be the dimension
	 size, except for the first element, which will be one, because
	 we will copy one record at a time. For this we need the var
	 shape. */
      if (!(dimlen = nc_malloc(real_ndims * sizeof(size_t))))
	 BAIL(NC_ENOMEM);

      /* Find out how much data. */
      for (d=0; d<ndims; d++)
      {
	 if ((retval = nc_inq_dimlen(ncid_in, dimids[d], &dimlen[d])))
	    BAIL(retval);
	 LOG((4, "nc_copy_var: there are %d data", dimlen[d]));
      }

      /* If this is really a scalar, then set the dimlen to 1. */
      if (ndims == 0)
	 dimlen[0] = 1;

      for (d=0; d<real_ndims; d++)
      {
	 start[d] = 0;
	 count[d] = d ? dimlen[d] : 1;
	 if (d) reclen *= dimlen[d];
      }

      /* If there are no records, we're done. */
      if (!dimlen[0])
	 goto exit;

      /* Allocate memory for one record. */
      if (!(data = nc_malloc(reclen * type_size)))
	 return NC_ENOMEM;
   
      /* Copy the var data one record at a time. */
      for (start[0]=0; !retval && start[0]<(size_t)dimlen[0]; start[0]++)
      {
	 if (nc_get_vara(ncid_in, v, start, count, data)) ERR;
	 if (nc_put_vara(ncid_out, v, start, count, data)) ERR;
      }
    
     exit:
      if (data) nc_free(data);
      if (dimlen) nc_free(dimlen);
      if (start) nc_free(start);
      if (count) nc_free(count);

   }

   if (nc_close(ncid_in)) ERR;
   if (nc_close(ncid_out)) ERR;

   return NC_NOERR;
}

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

#define INPUT_FILE "/upc/share/testdata/nssl/mosaic3d_nc/tile1/20070803-2300.netcdf"
#define FILE_NAME1 "bm_radar1.nc"
#define FILE_NAME2 "bm_radar2.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Benchmarking chunking and compression for radar file.\n");

   printf("\n*** Creating netCDF-4 file from netCDF-3 original...\n");
   {
#define NDIMS 3
#define NUM_TRIES 5
#define MAX_DEFLATE 9      
      struct timeval start_time, end_time, diff_time;
      int chunking[NDIMS];
      int deflate;
      int chunk_size, i, d;
      int total_time;

      printf("input size, output size, chunking[0], chunking[1], chunking[3],"
	     " chunk size, deflate, copy time usec\n");
      if (copy_file(INPUT_FILE, FILE_NAME, NC_NETCDF4, NULL, NULL)) ERR;
      for (deflate = 0; deflate < MAX_DEFLATE; deflate += 3)
      {
	 chunking[0] = 1;
	 chunking[1] = 100;
	 chunking[2] = 100;
	 for (i = 0; i < NUM_TRIES; i++)
	 {
	    if (gettimeofday(&start_time, NULL)) ERR;
	    if (copy_file(FILE_NAME, FILE_NAME1, NC_NETCDF4, chunking, &deflate)) ERR;
	    if (gettimeofday(&end_time, NULL)) ERR;
	    if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	    
	    /* Print some output. */
	    printf("%d, %d, ", file_size(FILE_NAME), file_size(FILE_NAME1));
	    for (chunk_size = 1, d = 0; d < NDIMS; d++)
	    {
	       printf("%d, ", chunking[d]);
	       chunk_size *= chunking[d];
	    }
	    
	    total_time = (int)diff_time.tv_sec * 1000 + (int)diff_time.tv_usec;
	    printf("%d, %d, %d", chunk_size, deflate, total_time);
	    printf("\n");
	    
	    chunking[0] += 1;
	    chunking[1] += 100;
	    chunking[2] += 100;
	 }
      }
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}










