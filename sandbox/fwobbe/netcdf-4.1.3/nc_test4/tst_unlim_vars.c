/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables with unlimited dimensions.
 
   $Id$
*/

#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_unlim_vars.nc"
#define SFC_TEMP_NAME "surface_temperature"
#define LAT_NAME "lat"
#define LAT_LEN 2
#define LON_NAME "lon"
#define LON_LEN 3
#define TIME_NAME "time"
#define NUM_TIMESTEPS 4
#define NDIMS 3

int
main(int argc, char **argv)
{
   int ncid, sfc_tempid;
   float data_out[NUM_TIMESTEPS][LAT_LEN][LON_LEN], data_in[NUM_TIMESTEPS][LAT_LEN][LON_LEN];
   int lat, lon, time;
   int dimids[NDIMS];
   nc_type xtype_in;
   int ndims_in, dimids_in[10], natts_in, nvars_in, unlimdimid_in;
   size_t len_in;
   char name_in[NC_MAX_NAME+1];
   size_t start[NDIMS], count[NDIMS];
   int d;

   /* Set up phony data. */
   for (time = 0; time < NUM_TIMESTEPS; time++)
      for (lat = 0; lat < LAT_LEN; lat++)
	 for (lon = 0; lon < LON_LEN; lon++)
	    data_out[time][lat][lon] = 25.5 + lat + lon + time;

   printf("\n*** Testing netcdf-4 variable with unlimited dimensions.\n");
   printf("*** Testing file with one var, one unlim dim...");
      
   /* Create a file with a 3D surface temp variable. */
   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

   /* Create three dims, one unlimited. */
   if (nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &dimids[0])) ERR;
   if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &dimids[1])) ERR;
   if (nc_def_dim(ncid, LON_NAME, LON_LEN, &dimids[2])) ERR;

   /* Define a var. */
   for (d = 0; d < NDIMS; d++)
      dimids[d] = d;
   if (nc_def_var(ncid, SFC_TEMP_NAME, NC_FLOAT, NDIMS, dimids, &sfc_tempid)) ERR;

   /* Check some metadata. */
   if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
   if (ndims_in != 3 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
   if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in,
		  &natts_in)) ERR;
   if (strcmp(name_in, SFC_TEMP_NAME) || xtype_in != NC_FLOAT ||
       ndims_in != 3 || natts_in != 0) ERR;
   for (d = 0; d < NDIMS; d++)
      if (dimids_in[d] != dimids[d]) ERR;
   if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
   if (len_in != 0 || strcmp(name_in, TIME_NAME)) ERR;
   if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
   if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
   if (nc_inq_dim(ncid, 2, name_in, &len_in)) ERR;
   if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;
   if (nc_close(ncid)) ERR;

   if (nc_open(FILE_NAME, 0, &ncid)) ERR;

   /* Check metadata. */
   if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
   if (ndims_in != 3 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
   if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in,
		  &natts_in)) ERR;
   if (strcmp(name_in, SFC_TEMP_NAME) || xtype_in != NC_FLOAT ||
       ndims_in != 3 || natts_in != 0) ERR;
   for (d = 0; d < NDIMS; d++)
      if (dimids_in[d] != dimids[d]) ERR;
   if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
   if (len_in != 0 || strcmp(name_in, TIME_NAME)) ERR;
   if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
   if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
   if (nc_inq_dim(ncid, 2, name_in, &len_in)) ERR;
   if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;

   if (nc_close(ncid)) ERR;

   if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

   /* Check metadata. */
   if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
   if (ndims_in != 3 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
   if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in,
		  &natts_in)) ERR;
   if (strcmp(name_in, SFC_TEMP_NAME) || xtype_in != NC_FLOAT ||
       ndims_in != 3 || natts_in != 0) ERR;
   for (d = 0; d < NDIMS; d++)
      if (dimids_in[d] != dimids[d]) ERR;
   if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
   if (len_in != 0 || strcmp(name_in, TIME_NAME)) ERR;
   if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
   if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
   if (nc_inq_dim(ncid, 2, name_in, &len_in)) ERR;
   if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;
   if (nc_close(ncid)) ERR;

   /* Write some data to the var.*/
   if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
   for (d = 0; d < NDIMS; d++)
      start[d] = 0;
   count[0] = NUM_TIMESTEPS;
   count[1] = LAT_LEN;
   count[2] = LON_LEN;
   if (nc_put_vara_float(ncid, 0, start, count, (float *)data_out)) ERR;

   /* Check metadata. */
   if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
   if (ndims_in != 3 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
   if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in,
		  &natts_in)) ERR;
   if (strcmp(name_in, SFC_TEMP_NAME) || xtype_in != NC_FLOAT ||
       ndims_in != 3 || natts_in != 0) ERR;
   for (d = 0; d < NDIMS; d++)
      if (dimids_in[d] != dimids[d]) ERR;
   if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
   if (len_in != NUM_TIMESTEPS || strcmp(name_in, TIME_NAME)) ERR;
   if (nc_inq_dim(ncid, 1, name_in, &len_in)) ERR;
   if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
   if (nc_inq_dim(ncid, 2, name_in, &len_in)) ERR;
   if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;

   if (nc_close(ncid)) ERR;

   if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

   /* Check data. */
   if (nc_get_vara_float(ncid, 0, start, count, (float *)data_in)) ERR;
   for (time = 0; time < NUM_TIMESTEPS; time++)
      for (lat = 0; lat < LAT_LEN; lat++)
	 for (lon = 0; lon < LON_LEN; lon++)
	    if (data_in[time][lat][lon] != data_out[time][lat][lon]) ERR;

   if (nc_close(ncid)) ERR;
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


