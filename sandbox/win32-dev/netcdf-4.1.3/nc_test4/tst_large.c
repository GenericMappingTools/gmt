/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_large.nc"
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
   printf("**** testing simple fill value attribute creation...");
   {
      int ncid, varid, dimids[NUMDIMS];
      size_t index[NUMDIMS] = {0, 0};
      signed char vals[DIM2];
      signed char char_val_in;
      size_t start[NUMDIMS] = {0, 0}, count[NUMDIMS] = {1, DIM2};
      int j;

      /* Create phony data. */
      for (j = 0; j < DIM2; j++) 
	 vals[j] = 9 * (j + 11); /* note vals[j] is 99 when j==0 */

      /* Create file with 2 dims and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;
      if (nc_def_dim(ncid, "dim1", DIM1, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "dim2", DIM2, &dimids[1])) ERR;
      if (nc_def_var(ncid, "var", NC_BYTE, NUMDIMS, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;

      /* Write one slice, then close. */
      if (nc_put_vara_schar(ncid, varid, start, count, vals)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen and read a value. */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
/*       if (nc_inq_varid(ncid, "var", &varid)) ERR; */
/*       if (nc_get_var1_schar(ncid, varid, index, &char_val_in)) ERR; */
/*       if (char_val_in != 99)	/\* see above, the value written when start[0]==0, j==0 *\/ */
/* 	 ERR; */
/*       if (nc_close(ncid)) ERR; */
   }
   SUMMARIZE_ERR;

#ifdef USE_PARALLEL
   MPI_Finalize();
#endif   
   FINAL_RESULTS;
}


