/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf files a bit. 
*/

#include <nc_tests.h>
#include "netcdf.h"

#define URL "http://test.opendap.org:8080/dods/dts/test.01"
#define FILE_NAME "tst_files6.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf file functions some more.\n");
#ifdef USE_DAP
#ifdef ENABLE_DAP_REMOTE_TESTS
   printf("*** testing simple opendap open/close...");
   {
      int ncid;

      /* Test with URL. */
      if (nc_open(URL, 0, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
#endif /*ENABLE_DAP_REMOTE_TESTS*/
#endif /* USE_DAP */
   printf("*** testing Jeff Whitaker's test...");
   {
#define DIM_NAME "xc"      
#define DIM_LEN 134      
#define VAR_NAME1 "var1"
#define VAR_NAME2 "var2"
      
      int ncid, dimid, varid1, varid2, dimid_in;
      int ndims_in, natts_in;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;

      if (nc_create(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME1, NC_FLOAT, 1, &dimid, &varid1)) ERR;
      if (nc_def_var(ncid, VAR_NAME2, NC_FLOAT, 1, &dimid, &varid2)) ERR;
      if (nc_def_var(ncid, DIM_NAME, NC_FLOAT, 1, &dimid, &varid2)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open and check. */
      if (nc_open(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM_NAME) || len_in != DIM_LEN) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, 
		     &dimid_in, &natts_in)) ERR;
      if (strcmp(name_in, VAR_NAME1) || xtype_in != NC_FLOAT || 
	  ndims_in != 1 || dimid_in != 0 || natts_in != 0) ERR;
      if (nc_inq_var(ncid, 1, name_in, &xtype_in, &ndims_in, 
		     &dimid_in, &natts_in)) ERR;
      if (strcmp(name_in, VAR_NAME2) || xtype_in != NC_FLOAT || 
	  ndims_in != 1 || dimid_in != 0 || natts_in != 0) ERR;
      if (nc_inq_var(ncid, 2, name_in, &xtype_in, &ndims_in, 
		     &dimid_in, &natts_in)) ERR;
      if (strcmp(name_in, DIM_NAME) || xtype_in != NC_FLOAT || 
	  ndims_in != 1 || dimid_in != 0 || natts_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


