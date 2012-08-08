/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test some things about how classic netCDF behaves.

   $Id$
*/
#include <nc_tests.h>
#include <limits.h>

#define FILE_NAME "tst_nc_converts.nc"
#define VAR_NAME "var"
#define DIM1_NAME "dim1"
#define DIM1_LEN 1

int
main()
{
   int ncid, varid, dimids[DIM1_LEN];
   double double_max_int = INT_MAX;
   float float_max_int = INT_MAX;
   int value_in;

   printf("\n*** Testing netcdf type conversion.\n");
   printf("*** testing netcdf-4...");
      
   /* Create a netcdf-4 format file one int variable. */
   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
   if (nc_def_var(ncid, VAR_NAME, NC_INT, 0, dimids, &varid)) ERR;
   if (nc_enddef(ncid)) ERR;
   if (nc_put_var_float(ncid, varid, &float_max_int)) ERR;
   /*if (nc_put_var_double(ncid, varid, &double_max_int)) ERR;*/
   if (nc_close(ncid)) ERR;

   if (nc_open(FILE_NAME, 0, &ncid)) ERR;
   if (nc_get_var(ncid, 0, &value_in)) ERR;
/*   if (value_in != INT_MAX) ERR;*/
   if (nc_close(ncid)) ERR;

   SUMMARIZE_ERR;

   printf("*** testing netcdf classic...");
   /* Create a classic format file one int variable. */
   if (nc_create(FILE_NAME, 0, &ncid)) ERR;
   if (nc_def_var(ncid, VAR_NAME, NC_INT, 0, dimids, &varid)) ERR;
   if (nc_enddef(ncid)) ERR;
/*   if (nc_put_var_float(ncid, varid, &float_max_int)) ERR;*/
   if (nc_put_var_double(ncid, varid, &double_max_int)) ERR;
   if (nc_close(ncid)) ERR;

   SUMMARIZE_ERR;

   FINAL_RESULTS;
}

