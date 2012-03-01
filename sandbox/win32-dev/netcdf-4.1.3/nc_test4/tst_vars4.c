/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>

#define FILE_NAME "tst_vars4.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 variable functions, even more.\n");
   printf("**** testing Jeff's dimension problem...");
   {
#define NDIMS2 2
#define NUM_VARS 1
#define Y_NAME "y"
#define X_NAME "x"
#define VAR_NAME Y_NAME
#define XDIM_LEN 2
#define YDIM_LEN 5

      int varid, ncid, dims[NDIMS2], dims_in[NDIMS2];
      int ndims, nvars, ngatts, unlimdimid, natts;
      char name_in[NC_MAX_NAME + 1];
      nc_type type_in;
      size_t len_in;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
      if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, 2, dims, &varid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS2 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME) || type_in != NC_FLOAT || ndims != NDIMS2 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
      if (strcmp(name_in, Y_NAME)) ERR;
      if (len_in != YDIM_LEN) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS2 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME) || type_in != NC_FLOAT || ndims != NDIMS2 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, X_NAME) || len_in != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
      if (strcmp(name_in, Y_NAME)) ERR;
      if (len_in != YDIM_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}






