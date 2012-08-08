/* This is part of the netCDF package.  Copyright 2011 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test netcdf-4 syncs. 
*/

#include <nc_tests.h>

#define FILE_NAME "tst_sync.nc"
#define DIM_NAME "x"
#define VAR1_NAME "var1"
#define VAR2_NAME "var2"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 variable syncing.\n");
   printf("**** testing that sync works in netCDF-4...");
   {
      int ncid, var1_id, var2_id, var3_id, dimid;
      int ndims, nvars, natts, unlimdimid, dimid_in;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];

      /* Create a file with one dim, and two vars that use it. For
       * fun, do a sync between the def_var calls. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, 1, &dimid)) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_FLOAT, 1, &dimid, &var1_id)) ERR;
      if (nc_sync(ncid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_FLOAT, 1, &dimid, &var2_id)) ERR;
      
      /* Now define a coordinate variable for the dimension. */
      if (nc_def_var(ncid, DIM_NAME, NC_FLOAT, 1, &dimid, &var3_id)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 2, name_in, &xtype_in, &ndims, &dimid_in, &natts)) ERR;
      if (strcmp(name_in, DIM_NAME) || xtype_in != NC_FLOAT || ndims != 1 || 
	  dimid_in != 0 || natts != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing that sync works in netCDF-4, with test contributed by Jeff W....");
   {
      int ncid, var1_id, var2_id, var3_id, dimid;

      /* Create a file with one dim and two variables. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, 1, &dimid)) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_FLOAT, 1, &dimid, &var1_id)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_FLOAT, 1,&dimid, &var2_id)) ERR;

      /* Sync the file and add a coordinate variable. */
      if (nc_sync(ncid)) ERR;
      if (nc_def_var(ncid, DIM_NAME, NC_FLOAT, 1, &dimid, &var3_id)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing that sync works in netCDF-4, with test contributed by Jeff W....");
   {
      int ncid, var1_id, var2_id, var3_id, dimid;
      float xx;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, 1, &dimid)) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_FLOAT, 1, &dimid, &var1_id)) ERR;
      xx = 1.0;
      if (nc_put_var_float(ncid, var1_id, &xx)) ERR;
      if (nc_sync(ncid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_FLOAT, 1,&dimid, &var2_id)) ERR;
      xx = 2.0;
      if (nc_put_var_float(ncid, var2_id, &xx)) ERR;
      if (nc_sync(ncid)) ERR;
      if (nc_def_var(ncid, DIM_NAME, NC_FLOAT, 1, &dimid, &var3_id)) ERR;
      xx = 3.0;
      if (nc_put_var_float(ncid, var3_id, &xx)) ERR;
      if (nc_sync(ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}






