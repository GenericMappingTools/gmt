/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test large file problems reported by user. This test based on code
   contributed by Kari Hoijarvi. Thanks Kari!

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_large2.nc"
#define NUMDIMS 2		/* rank of each variable in tests */
#define DIM1 2048
#define DIM2 2097153		/* DIM1*DIM2*sizeof(char)   > 2**32 */

int
main(int argc, char **argv)
{

#ifdef USE_PARALLEL
   MPI_Init(&argc, &argv);
#endif

   printf("\n*** Testing netcdf-4 large files.\n");
   printf("**** testing with user-contributed test...\n");
   {
#define TIME_LEN 3000
#define LAT_LEN 1000
#define LON_LEN 2000
#define NDIMS 3
#define VAR_NAME "the_big_enchilada"
#define NUM_FORMATS 2

      int ncid, varid;
      int dimids[NDIMS];
      size_t start[NDIMS] = {0, 0, 0};
      size_t count[NDIMS] = {1, LAT_LEN, LON_LEN};
      char file_name[NC_MAX_NAME * 2 + 1];
      float *data;
      int this_format[NUM_FORMATS] = {NC_64BIT_OFFSET, NC_NETCDF4};
      char format_name[NUM_FORMATS][NC_MAX_NAME + 1] = 
	 {"64-bit offset", "netCDF-4"};
      int i, j, f;

      printf("sizes: int - %d, size_t - %d, and int * - %d\n", 
	     sizeof(int), sizeof(size_t), sizeof(int *));

      /* Allocate room for one slab of data. */
      if (!(data = calloc(LAT_LEN * LON_LEN, sizeof(float)))) ERR;

      /* Create a file with one big variable. */
      for (f = 0; f < NUM_FORMATS; f++)
      {
	 printf("\t...testing with %s\n", format_name[f]);
	 sprintf(file_name, "%s/%s", TEMP_LARGE, FILE_NAME);
	 if (nc_create(file_name, this_format[f], &ncid)) ERR;
	 if (nc_def_dim(ncid, "lat", LAT_LEN, &dimids[1])) ERR;
	 if (nc_def_dim(ncid, "lon", LON_LEN, &dimids[2])) ERR;
	 if (nc_def_dim(ncid, "time", TIME_LEN, &dimids[0])) ERR;
	 if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, 3, dimids, &varid)) ERR;
	 if (nc_close(ncid)) ERR;

	 /* Reopen the file and add data. */
	 if (nc_open(file_name, NC_WRITE, &ncid)) ERR;
	 for (start[0] = 0; start[0] < TIME_LEN; start[0]++)
	 {
	    /* Initialize this slab of data. */
	    for (i = 0; i < LAT_LEN; i++)
	       for (j = 0; j < LON_LEN; j++)
		  data[j + LON_LEN * i] = (start[0] + i + j) % 19;

	    /* Write the slab. */
	    if (nc_put_vara_float(ncid, varid, start, count, data)) ERR;
	 }
	 if (nc_close(ncid)) ERR;

	 /* Reopen and check the file. */
	 if (nc_open(file_name, NC_NOWRITE, &ncid)) ERR;
	 if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
	 for (start[0] = 0; start[0] < TIME_LEN; start[0]++)
	 {
	    if (nc_get_vara_float(ncid, varid, start, count, data)) ERR;
	    for (i = 0; i < LAT_LEN; i++)
	       for (j = 0; j < LON_LEN; j++)
	       {
		  if (data[j + LON_LEN * i] != (start[0] + i + j) % 19) 
		  {
		     printf("error on start[0]: %d i: %d j: %d expected %d got %g\n", 
			    start[0], i, j, (start[0] + i + j), data[j + LON_LEN * i]);
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


