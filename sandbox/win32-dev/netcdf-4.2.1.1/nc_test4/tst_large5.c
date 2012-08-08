/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test large file problems reported by user. This test based on code
   contributed by Kari Hoijarvi. Thanks Kari!

   $Id$
*/

#include <config.h>
#include "netcdf.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef NSLABS
#define NSLABS 2149
#endif

#ifndef TEMP_LARGE
#define TEMP_LARGE "."
#endif

#define FILE_NAME "tst_large5.nc"

/* Error macros from ../libsrc4/nc_tests.h */
int total_err = 0, err = 0;
#define ERR do { \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
err++; \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n", \
	__FILE__, __LINE__);				    \
} while (0)
#define ERR_RET do { \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d\n", \
	__FILE__, __LINE__);				    \
return 2;                                                   \
} while (0)
#define SUMMARIZE_ERR do { \
   if (err) \
   { \
      printf("%d failures\n", err); \
      total_err += err; \
      err = 0; \
   } \
   else \
      printf("ok.\n"); \
} while (0)
#define FINAL_RESULTS do { \
   if (total_err) \
   { \
      printf("%d errors detected! Sorry!\n", total_err); \
      return 2; \
   } \
   printf("*** Tests successful!\n"); \
   return 0; \
} while (0)

int
main(int argc, char **argv)
{

#ifdef USE_PARALLEL
   MPI_Init(&argc, &argv);
#endif

   printf("\n*** Testing netcdf-4 large files.\n");
   printf("**** testing with user-contributed test...\n");
   {
#define TIME_LEN NSLABS
#define LAT_LEN 1000
#define LON_LEN 2000
#define NDIMS 3
#define VAR_NAME "the_big_enchilada"

      int ncid, varid;
      int dimids[NDIMS];
      size_t start[NDIMS] = {0, 0, 0};
      size_t count[NDIMS] = {1, LAT_LEN, LON_LEN};
      char file_name[NC_MAX_NAME * 2 + 1];
      signed char data[LAT_LEN][LON_LEN];
/* #define NUM_FORMATS 2 */
/*       int this_format[NUM_FORMATS] = {NC_64BIT_OFFSET, NC_NETCDF4}; */
/*       char format_name[NUM_FORMATS][NC_MAX_NAME + 1] =  */
/* 	 {"64-bit offset", "netCDF-4"}; */
#define NUM_FORMATS 1
      int this_format[NUM_FORMATS] = {NC_64BIT_OFFSET};
      char format_name[NUM_FORMATS][NC_MAX_NAME + 1] = 
	 {"64-bit offset"};
      int i, j, f;

      printf("NSLABS=%d, sizes: int - %d, size_t - %d, and int * - %d\n", 
	     NSLABS, sizeof(int), sizeof(size_t), sizeof(int *));

      /* Create a file with one big variable. */
      for (f = 0; f < NUM_FORMATS; f++)
      {
	 printf("\t...testing with %s\n", format_name[f]);
	 sprintf(file_name, "%s/%s", TEMP_LARGE, FILE_NAME);
	 if (nc_create(file_name, this_format[f], &ncid)) ERR;
	 if (nc_def_dim(ncid, "lat", LAT_LEN, &dimids[1])) ERR;
	 if (nc_def_dim(ncid, "lon", LON_LEN, &dimids[2])) ERR;
	 if (nc_def_dim(ncid, "time", TIME_LEN, &dimids[0])) ERR;
	 if (nc_def_var(ncid, VAR_NAME, NC_BYTE, 3, dimids, &varid)) ERR;
	 if (nc_close(ncid)) ERR;

	 /* Reopen the file and add data. */
	 if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
	 /* See if can get error writing and reading only NSLABS slabs */
	 for (start[0] = 0; start[0] < NSLABS; start[0]++)
	 {
	    /* Initialize this slab of data. */
	    for (i = 0; i < LAT_LEN; i++)
	       for (j = 0; j < LON_LEN; j++)
		  data[i][j] = (start[0] + i + j) % 19;

	    /* Write the slab. */
	    if (nc_put_vara_schar(ncid, varid, start, count, &data[0][0])) ERR;
	 }
	 if (nc_close(ncid)) ERR;

	 /* Reopen and check the file. */
	 if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;
	 if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
	 /* Just read first NSLABS slabs */
	 for (start[0] = 0; start[0] < NSLABS; start[0]++)
	 {
	    if (nc_get_vara_schar(ncid, varid, start, count, &data[0][0])) ERR;
	    for (i = 0; i < LAT_LEN; i++)
	       for (j = 0; j < LON_LEN; j++)
	       {
		  if (data[i][j] != (signed char)((start[0] + i + j) % 19)) 
		  {
		     printf("error on start[0]: %d i: %d j: %d expected %d got %d\n", 
			    start[0], i, j, (start[0] + i + j) % 19, data[i][j]);
		     ERR_RET;
		  }
	       }
	 } /* next slab to read */
      } /* next format*/

      /* Release our memory. */
      free(data);
   }
   SUMMARIZE_ERR;

#ifdef USE_PARALLEL
   MPI_Finalize();
#endif   
   FINAL_RESULTS;
}
