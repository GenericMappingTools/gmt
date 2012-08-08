/* This is part of the netCDF package. Copyright 2010 University
   Corporation for Atmospheric Research/Unidata.  See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a compressible test file for nccopy to compress.

   $Id$
*/

#include <config.h>
#include <nc_tests.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_inflated.nc"
#define FILENAME2 "tst_inflated4.nc"
#define DIM_NAME "dim1"
#define DIM1_LEN 10000
#define VAR1_RANK 1
#define VAR_NAME "var"

int
main(int argc, char **argv) { /* create a compressible file, for testing */

    int ncid;
    int dimid, varid;
    int var1_dims[VAR1_RANK];	/* variable shapes */
    int var1_data[DIM1_LEN];	/* data to write */
    int i;

    printf("*** Creating compressible test files %s, %s...", FILENAME, FILENAME2);
    if (nc_create(FILENAME, NC_CLOBBER, &ncid)) ERR;
    if (nc_def_dim(ncid, "dim1", DIM1_LEN, &dimid)) ERR;
    var1_dims[0] = dimid;
    if (nc_def_var(ncid, "var1", NC_INT, VAR1_RANK, var1_dims, &varid)) ERR;
    if (nc_enddef (ncid)) ERR;
    for(i=0; i < DIM1_LEN; i++) {
	var1_data[i] = i;
    }
    if (nc_put_var(ncid, varid, var1_data)) ERR;
    if (nc_close(ncid)) ERR;

    if (nc_create(FILENAME2, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
    if (nc_def_dim(ncid, "dim1", DIM1_LEN, &dimid)) ERR;
    var1_dims[0] = dimid;
    if (nc_def_var(ncid, "var1", NC_INT, VAR1_RANK, var1_dims, &varid)) ERR;
    if (nc_enddef (ncid)) ERR;
    for(i=0; i < DIM1_LEN; i++) {
	var1_data[i] = i;
    }
    if (nc_put_var(ncid, varid, var1_data)) ERR;
    if (nc_close(ncid)) ERR;

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
