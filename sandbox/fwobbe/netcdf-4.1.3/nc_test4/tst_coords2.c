/* This is part of the netCDF package. Copyright 2009 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test netcdf-4 coordinate variables and dimensions. 

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_coords2.nc"


int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 coordinate dimensions and variables, some more.\n");

   printf("**** testing more complex 2D coordinate variable with dimensions defined in different order...");
   {
#define NDIMS 5
#define NVARS 4
#define LON_NAME "lon"
#define LAT_NAME "lat"
#define LVL_NAME "lvl"
#define TIME_NAME "time"
#define TEXT_LEN_NAME "text_len"
#define TEMP_NAME "temp"
#define PRES_NAME "pres"
#define LON_LEN 5
#define LAT_LEN 3
#define LVL_LEN 7
#define TIME_LEN 2
#define TEXT_LEN 15
#define TIME_NDIMS 2
#define DATA_NDIMS 4

      int ncid, nvars_in, varids_in[NVARS];
      int time_dimids[TIME_NDIMS];
      int dimids[NDIMS], time_id, lon_id, pres_id, temp_id;
      size_t time_count[NDIMS], time_index[NDIMS] = {0, 0};
      const char ttext[TEXT_LEN + 1]="20051224.150000";
      char ttext_in[TEXT_LEN + 1];
      int nvars, ndims, ngatts, unlimdimid;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1], dim_name_in[NC_MAX_NAME + 1];
      size_t len_in;
      nc_type xtype_in;
      int d;

      /* Create a netcdf-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions. */
      if (nc_def_dim(ncid, LON_NAME, LON_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, LVL_NAME, LVL_LEN, &dimids[2])) ERR;
      if (nc_def_dim(ncid, TIME_NAME, TIME_LEN, &dimids[3])) ERR;
      if (nc_def_dim(ncid, TEXT_LEN_NAME, TEXT_LEN, &dimids[4])) ERR;

      /* Define two coordinate variables out of order. */
      time_dimids[0] = dimids[3];
      time_dimids[1] = dimids[4];
      if (nc_def_var(ncid, TIME_NAME, NC_CHAR, TIME_NDIMS, time_dimids, &time_id)) ERR;
      if (nc_def_var(ncid, LON_NAME, NC_CHAR, 1, &dimids[0], &lon_id)) ERR;

      /* Write one time to the coordinate variable. */
      time_count[0] = 1;
      time_count[1] = TEXT_LEN;
      if (nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR;

      /* Define two data variable. */
      if (nc_def_var(ncid, PRES_NAME, NC_CHAR, DATA_NDIMS, dimids, &pres_id)) ERR;
      if (nc_def_var(ncid, TEMP_NAME, NC_CHAR, DATA_NDIMS, dimids, &temp_id)) ERR;

      /* Check the data. */
      if (nc_get_vara_text(ncid, time_id, time_index, time_count, ttext_in)) ERR;
      if (strncmp(ttext, ttext_in, TEXT_LEN)) ERR;

      /* Check the metadata. */
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NVARS || ndims != NDIMS || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != NVARS || varids_in[0] != 0 || varids_in[1] != 1 || 
	  varids_in[2] != 2 || varids_in[3] != 3) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, TIME_NAME) || xtype_in != NC_CHAR || ndims_in != TIME_NDIMS ||
	dimids_in[0] != time_dimids[0] || dimids_in[1] != time_dimids[1] || natts_in != 0) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != NDIMS) ERR;
      for (d = 0; d < NDIMS; d++)
	 if (dimids_in[d] != dimids[d]) ERR;
      if (nc_inq_dim(ncid, 0, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, LON_NAME) || len_in != LON_LEN) ERR;
      if (nc_inq_dim(ncid, 1, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, LAT_NAME) || len_in != LAT_LEN) ERR;
      if (nc_inq_dim(ncid, 2, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, LVL_NAME) || len_in != LVL_LEN) ERR;
      if (nc_inq_dim(ncid, 3, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, TIME_NAME) || len_in != TIME_LEN) ERR;
      if (nc_inq_dim(ncid, 4, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, TEXT_LEN_NAME) || len_in != TEXT_LEN) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check the order of variables and dimensions. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NVARS || ndims != NDIMS || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != NVARS || varids_in[0] != 0 || varids_in[1] != 1 || 
	  varids_in[2] != 2 || varids_in[3] != 3) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, TIME_NAME) || xtype_in != NC_CHAR || ndims_in != TIME_NDIMS ||
	dimids_in[0] != time_dimids[0] || dimids_in[1] != time_dimids[1] || natts_in != 0) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != NDIMS) ERR;
      for (d = 0; d < NDIMS; d++)
	 if (dimids_in[d] != dimids[d]) ERR;
      if (nc_inq_dim(ncid, 0, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, LON_NAME) || len_in != LON_LEN) ERR;
      if (nc_inq_dim(ncid, 1, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, LAT_NAME) || len_in != LAT_LEN) ERR;
      if (nc_inq_dim(ncid, 2, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, LVL_NAME) || len_in != LVL_LEN) ERR;
      if (nc_inq_dim(ncid, 3, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, TIME_NAME) || len_in != TIME_LEN) ERR;
      if (nc_inq_dim(ncid, 4, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, TEXT_LEN_NAME) || len_in != TEXT_LEN) ERR;

      /* Check the data. */
      if (nc_get_vara_text(ncid, time_id, time_index, time_count, ttext_in)) ERR;
      if (strncmp(ttext, ttext_in, TEXT_LEN)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}






