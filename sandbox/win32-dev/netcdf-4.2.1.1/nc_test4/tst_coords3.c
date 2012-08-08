/* This is part of the netCDF package. Copyright 2010 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test netcdf-4 coordinate variables and dimensions with an example
   from the CF conventions.

 Here's the example from the CF conventions:
dimensions:
  xc = 128 ;
  yc = 64 ;
  lev = 18 ;
variables:
  float T(lev,yc,xc) ;
    T:long_name = "temperature" ;
    T:units = "K" ;
    T:coordinates = "lon lat" ;
  float xc(xc) ;
    xc:axis = "X" ;
    xc:long_name = "x-coordinate in Cartesian system" ;
    xc:units = "m" ;
  float yc(yc) ;
    yc:axis = "Y" ;
    yc:long_name = "y-coordinate in Cartesian system" ;
    yc:units = "m" ;
  float lev(lev) ;
    lev:long_name = "pressure level" ;
    lev:units = "hPa" ;
  float lon(yc,xc) ;
    lon:long_name = "longitude" ;
    lon:units = "degrees_east" ;
  float lat(yc,xc) ;
    lat:long_name = "latitude" ;
    lat:units = "degrees_north" ;

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_coords3.nc"

#define NDIMS 3
#define NVARS 6
#define XC_NAME "xc"
#define YC_NAME "yc"
#define LEV_NAME "lev"
#define T_NAME "T"
#define LON_NAME "lon"
#define LAT_NAME "lat"
#define XC_LEN 128
#define YC_LEN 64
#define LEV_LEN 18
#define DATA_NDIMS 3
#define COORD_NDIMS 2

/* For the attributes. */
#define LONG_NAME "long_name"
#define TEMPERATURE "temperature"
#define UNITS "units"
#define KELVIN "K"
#define COORDINATES_NAME "coordinates"
#define LONLAT_COORDINATES "lon lat"
#define AXIS "axis"
#define X_NAME "X"
#define X_LONG_NAME "x-coordinate in Cartesian system"
#define METER "m"
#define Y_NAME "Y"
#define Y_LONG_NAME "y-coordinate in Cartesian system"
#define LEV_LONG_NAME "pressure level"
#define LON_LONG_NAME "longitude"
#define LAT_LONG_NAME "latitude"
#define DEGREES_EAST "degrees_east"
#define DEGREES_NORTH "degrees_north"
#define HPA "hPa"
#define SAMPLE_VALUE 0.5

int
check_cf_data(int ncid)
{
   float temp_in[YC_LEN][XC_LEN];
   size_t start[DATA_NDIMS] = {0, 0, 0}, count[DATA_NDIMS] = {1, YC_LEN, XC_LEN};
   float xc_in[XC_LEN], yc_in[YC_LEN];
   int x, y;

   /* Get the record we wrote. */
   if (nc_get_vara_float(ncid, 0, start, count, (float *)temp_in)) ERR_RET;
   for (y = 0; y < YC_LEN; y++)
      for (x = 0; x < XC_LEN; x++)
	 if (temp_in[y][x] != SAMPLE_VALUE) ERR_RET;

   /* Get the XC data. */
   if (nc_get_var_float(ncid, 1, xc_in)) ERR_RET;
   for (x = 0; x < XC_LEN; x++)
      if (xc_in[x] != SAMPLE_VALUE) ERR_RET;

   /* Get the YC data. */
   if (nc_get_var_float(ncid, 2, yc_in)) ERR_RET;
   for (y = 0; y < YC_LEN; y++)
      if (yc_in[y] != SAMPLE_VALUE) ERR_RET;

   return 0;
}

int
check_cf_metadata(int ncid)
{
 
   int nvars_in, varids_in[NVARS];
   int nvars, ndims, ngatts, unlimdimid;
   int ndims_in, natts_in, dimids_in[NDIMS];
   char var_name_in[NC_MAX_NAME + 1], dim_name_in[NC_MAX_NAME + 1];
   size_t len_in;
   nc_type xtype_in;
   int v;
   char att_text_in[NC_MAX_NAME + 1];

   /* Right number of objects? */
   if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR_RET;
   if (nvars != NVARS || ndims != NDIMS || ngatts != 0 || unlimdimid != -1) ERR_RET;

   /* Right number of varids? */
   if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR_RET;
   if (nvars_in != NVARS) ERR_RET;
   for (v = 0; v < NVARS; v++)
      if (varids_in[v] != v) ERR_RET;

   /* Right number of dimids? */
   if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR_RET;
   if (ndims_in != NDIMS) ERR_RET;

   /* Check dimensions. */
   if (nc_inq_dim(ncid, 0, dim_name_in, &len_in)) ERR;
   if (strcmp(dim_name_in, XC_NAME) || len_in != XC_LEN) ERR;
   if (nc_inq_dim(ncid, 1, dim_name_in, &len_in)) ERR;
   if (strcmp(dim_name_in, YC_NAME) || len_in != YC_LEN) ERR;
   if (nc_inq_dim(ncid, 2, dim_name_in, &len_in)) ERR;
   if (strcmp(dim_name_in, LEV_NAME) || len_in != LEV_LEN) ERR;

   /* Check variable T. */
   if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR_RET;
   if (strcmp(var_name_in, T_NAME) || xtype_in != NC_FLOAT || ndims_in != NDIMS ||
       dimids_in[0] != 2 || dimids_in[1] != 1 || dimids_in[2] != 0 || natts_in != 3) ERR_RET;
   if (nc_get_att_text(ncid, 0, LONG_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, TEMPERATURE, strlen(TEMPERATURE))) ERR;
   if (nc_get_att_text(ncid, 0, UNITS, att_text_in)) ERR;
   if (strncmp(att_text_in, KELVIN, strlen(KELVIN))) ERR;
   if (nc_get_att_text(ncid, 0, COORDINATES_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, LONLAT_COORDINATES, strlen(LONLAT_COORDINATES))) ERR;

   /* Check variable xc. */
   if (nc_inq_var(ncid, 1, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR_RET;
   if (strcmp(var_name_in, XC_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
       dimids_in[0] != 0 || natts_in != 3) ERR_RET;
   if (nc_get_att_text(ncid, 1, AXIS, att_text_in)) ERR;
   if (strncmp(att_text_in, X_NAME, strlen(X_NAME))) ERR;
   if (nc_get_att_text(ncid, 1, LONG_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, X_LONG_NAME, strlen(X_LONG_NAME))) ERR;
   if (nc_get_att_text(ncid, 1, UNITS, att_text_in)) ERR;
   if (strncmp(att_text_in, METER, strlen(METER))) ERR;

   /* Check variable yc. */
   if (nc_inq_var(ncid, 2, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR_RET;
   if (strcmp(var_name_in, YC_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
       dimids_in[0] != 1 || natts_in != 3) ERR_RET;
   if (nc_get_att_text(ncid, 2, AXIS, att_text_in)) ERR;
   if (strncmp(att_text_in, Y_NAME, strlen(Y_NAME))) ERR;
   if (nc_get_att_text(ncid, 2, LONG_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, Y_LONG_NAME, strlen(Y_LONG_NAME))) ERR;
   if (nc_get_att_text(ncid, 2, UNITS, att_text_in)) ERR;
   if (strncmp(att_text_in, METER, strlen(METER))) ERR;

   /* Check variable lev. */
   if (nc_inq_var(ncid, 3, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR_RET;
   if (strcmp(var_name_in, LEV_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
       dimids_in[0] != 2 || natts_in != 2) ERR_RET;
   if (nc_get_att_text(ncid, 3, LONG_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, LEV_LONG_NAME, strlen(LEV_LONG_NAME))) ERR;
   if (nc_get_att_text(ncid, 3, UNITS, att_text_in)) ERR;
   if (strncmp(att_text_in, HPA, strlen(HPA))) ERR;

   /* Check variable lon. */
   if (nc_inq_var(ncid, 4, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR_RET;
   if (strcmp(var_name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != COORD_NDIMS ||
       dimids_in[0] != 1 || dimids_in[1] != 0 || natts_in != 2) ERR_RET;
   if (nc_get_att_text(ncid, 4, LONG_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, LON_LONG_NAME, strlen(LON_LONG_NAME))) ERR;
   if (nc_get_att_text(ncid, 4, UNITS, att_text_in)) ERR;
   if (strncmp(att_text_in, DEGREES_EAST, strlen(DEGREES_EAST))) ERR;

   /* Check variable lat. */
   if (nc_inq_var(ncid, 5, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR_RET;
   if (strcmp(var_name_in, LAT_NAME) || xtype_in != NC_FLOAT || ndims_in != COORD_NDIMS ||
       dimids_in[0] != 1 || dimids_in[1] != 0 || natts_in != 2) ERR_RET;
   if (nc_get_att_text(ncid, 5, LONG_NAME, att_text_in)) ERR;
   if (strncmp(att_text_in, LAT_LONG_NAME, strlen(LAT_LONG_NAME))) ERR;
   if (nc_get_att_text(ncid, 5, UNITS, att_text_in)) ERR;
   if (strncmp(att_text_in, DEGREES_NORTH, strlen(DEGREES_NORTH))) ERR;

   return 0;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing with CF example http://cf-pcmdi.llnl.gov/documents/cf-conventions/1.4/ch05s02.html....\n");
   printf("**** simple test with only metadata");
   {
      int ncid, dimids[NDIMS], varids[NVARS], data_dimids[DATA_NDIMS]; 
      int coord_dimids[COORD_NDIMS];

      /* Create a netcdf-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions. */
      if (nc_def_dim(ncid, XC_NAME, XC_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, YC_NAME, YC_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, LEV_NAME, LEV_LEN, &dimids[2])) ERR;

      /* Define variables. Start with T. */
      data_dimids[0] = dimids[2];
      data_dimids[1] = dimids[1];
      data_dimids[2] = dimids[0];
      if (nc_def_var(ncid, T_NAME, NC_FLOAT, DATA_NDIMS, data_dimids, &varids[0])) ERR;
      if (nc_put_att_text(ncid, varids[0], LONG_NAME, strlen(TEMPERATURE),
			  TEMPERATURE)) ERR;      
      if (nc_put_att_text(ncid, varids[0], UNITS, strlen(KELVIN), KELVIN)) ERR;      
      if (nc_put_att_text(ncid, varids[0], COORDINATES_NAME, strlen(LONLAT_COORDINATES),
			  LONLAT_COORDINATES)) ERR;      

      /* Define xc variable. */
      if (nc_def_var(ncid, XC_NAME, NC_FLOAT, 1, &dimids[0], &varids[1])) ERR;
      if (nc_put_att_text(ncid, varids[1], AXIS, strlen(X_NAME), X_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[1], LONG_NAME, strlen(X_LONG_NAME),
			  X_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[1], UNITS, strlen(METER), METER)) ERR;      

      /* Define yc variable. */
      if (nc_def_var(ncid, YC_NAME, NC_FLOAT, 1, &dimids[1], &varids[2])) ERR;
      if (nc_put_att_text(ncid, varids[2], AXIS, strlen(Y_NAME), Y_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[2], LONG_NAME, strlen(Y_LONG_NAME),
			  Y_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[2], UNITS, strlen(METER), METER)) ERR;      

      /* Define lev variable. */
      if (nc_def_var(ncid, LEV_NAME, NC_FLOAT, 1, &dimids[2], &varids[3])) ERR;
      if (nc_put_att_text(ncid, varids[3], LONG_NAME, strlen(LEV_LONG_NAME),
			  LEV_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[3], UNITS, strlen(HPA), HPA)) ERR;      

      /* Define lon variable. */
      coord_dimids[0] = dimids[1];
      coord_dimids[1] = dimids[0];
      if (nc_def_var(ncid, LON_NAME, NC_FLOAT, COORD_NDIMS, coord_dimids, &varids[4])) ERR;
      if (nc_put_att_text(ncid, varids[4], LONG_NAME, strlen(LON_LONG_NAME),
			  LON_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[4], UNITS, strlen(DEGREES_EAST), DEGREES_EAST)) ERR;      

      /* Define lat variable. */
      if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, COORD_NDIMS, coord_dimids, &varids[5])) ERR;
      if (nc_put_att_text(ncid, varids[5], LONG_NAME, strlen(LAT_LONG_NAME),
			  LAT_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[5], UNITS, strlen(DEGREES_NORTH), DEGREES_NORTH)) ERR;      

      /* Check the metadata. */
      if (check_cf_metadata(ncid)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check the order of variables and dimensions. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /* Check the metadata. */
      if (check_cf_metadata(ncid)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** test with data writes...");
   {
      int ncid, dimids[NDIMS], varids[NVARS], data_dimids[DATA_NDIMS]; 
      int coord_dimids[COORD_NDIMS];
      float temp[YC_LEN][XC_LEN], xc[XC_LEN], yc[YC_LEN];
      size_t start[DATA_NDIMS] = {0, 0, 0}, count[DATA_NDIMS] = {1, YC_LEN, XC_LEN};
      int x, y;

      /* Initialize some fake data. */
      for (y = 0; y < YC_LEN; y++)
	 for (x = 0; x < XC_LEN; x++)
	    temp[y][x] = SAMPLE_VALUE;
      for (x = 0; x < XC_LEN; x++)
	 xc[x] = SAMPLE_VALUE;
      for (y = 0; y < YC_LEN; y++)
	 yc[y] = SAMPLE_VALUE;

      /* Create a netcdf-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions. */
      if (nc_def_dim(ncid, XC_NAME, XC_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, YC_NAME, YC_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, LEV_NAME, LEV_LEN, &dimids[2])) ERR;

      /* Define variables. Start with T. */
      data_dimids[0] = dimids[2];
      data_dimids[1] = dimids[1];
      data_dimids[2] = dimids[0];
      if (nc_def_var(ncid, T_NAME, NC_FLOAT, DATA_NDIMS, data_dimids, &varids[0])) ERR;
      if (nc_put_att_text(ncid, varids[0], LONG_NAME, strlen(TEMPERATURE),
			  TEMPERATURE)) ERR;      
      if (nc_put_att_text(ncid, varids[0], UNITS, strlen(KELVIN), KELVIN)) ERR;      
      if (nc_put_att_text(ncid, varids[0], COORDINATES_NAME, strlen(LONLAT_COORDINATES),
			  LONLAT_COORDINATES)) ERR;      

      /* Define xc variable. */
      if (nc_def_var(ncid, XC_NAME, NC_FLOAT, 1, &dimids[0], &varids[1])) ERR;
      if (nc_put_att_text(ncid, varids[1], AXIS, strlen(X_NAME), X_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[1], LONG_NAME, strlen(X_LONG_NAME),
			  X_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[1], UNITS, strlen(METER), METER)) ERR;      

      /* Define yc variable. */
      if (nc_def_var(ncid, YC_NAME, NC_FLOAT, 1, &dimids[1], &varids[2])) ERR;
      if (nc_put_att_text(ncid, varids[2], AXIS, strlen(Y_NAME), Y_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[2], LONG_NAME, strlen(Y_LONG_NAME),
			  Y_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[2], UNITS, strlen(METER), METER)) ERR;      

      /* Define lev variable. */
      if (nc_def_var(ncid, LEV_NAME, NC_FLOAT, 1, &dimids[2], &varids[3])) ERR;
      if (nc_put_att_text(ncid, varids[3], LONG_NAME, strlen(LEV_LONG_NAME),
			  LEV_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[3], UNITS, strlen(HPA), HPA)) ERR;      

      /* Define lon variable. */
      coord_dimids[0] = dimids[1];
      coord_dimids[1] = dimids[0];
      if (nc_def_var(ncid, LON_NAME, NC_FLOAT, COORD_NDIMS, coord_dimids, &varids[4])) ERR;
      if (nc_put_att_text(ncid, varids[4], LONG_NAME, strlen(LON_LONG_NAME),
			  LON_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[4], UNITS, strlen(DEGREES_EAST), DEGREES_EAST)) ERR;      

      /* Define lat variable. */
      if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, COORD_NDIMS, coord_dimids, &varids[5])) ERR;
      if (nc_put_att_text(ncid, varids[5], LONG_NAME, strlen(LAT_LONG_NAME),
			  LAT_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[5], UNITS, strlen(DEGREES_NORTH), DEGREES_NORTH)) ERR;      

      /* Write some data to T. */
      if (nc_put_vara_float(ncid, 0, start, count, (const float *)temp)) ERR;
      /* Write data to XC. */
      if (nc_put_var_float(ncid, 1, xc)) ERR;
      /* Write data to YC. */
      if (nc_put_var_float(ncid, 2, yc)) ERR;

      /* Check the metadata and data. */
      if (check_cf_metadata(ncid)) ERR;
      if (check_cf_data(ncid)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check the order of variables and dimensions. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /* Check the metadata and data. */
      if (check_cf_metadata(ncid)) ERR;
      if (check_cf_data(ncid)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** test with data writes without enddefs...");
   {
      int ncid, dimids[NDIMS], varids[NVARS], data_dimids[DATA_NDIMS]; 
      int coord_dimids[COORD_NDIMS];
      float temp[YC_LEN][XC_LEN], xc[XC_LEN], yc[YC_LEN];
      size_t start[DATA_NDIMS] = {0, 0, 0}, count[DATA_NDIMS] = {1, YC_LEN, XC_LEN};
      int x, y;

      /* Initialize some fake data. */
      for (y = 0; y < YC_LEN; y++)
	 for (x = 0; x < XC_LEN; x++)
	    temp[y][x] = SAMPLE_VALUE;
      for (x = 0; x < XC_LEN; x++)
	 xc[x] = SAMPLE_VALUE;
      for (y = 0; y < YC_LEN; y++)
	 yc[y] = SAMPLE_VALUE;

      /* Create a netcdf-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions. */
      if (nc_def_dim(ncid, XC_NAME, XC_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, YC_NAME, YC_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, LEV_NAME, LEV_LEN, &dimids[2])) ERR;

      /* Define variables. Start with T. */
      data_dimids[0] = dimids[2];
      data_dimids[1] = dimids[1];
      data_dimids[2] = dimids[0];
      if (nc_def_var(ncid, T_NAME, NC_FLOAT, DATA_NDIMS, data_dimids, &varids[0])) ERR;
      if (nc_put_att_text(ncid, varids[0], LONG_NAME, strlen(TEMPERATURE),
			  TEMPERATURE)) ERR;      
      if (nc_put_att_text(ncid, varids[0], UNITS, strlen(KELVIN), KELVIN)) ERR;      
      if (nc_put_att_text(ncid, varids[0], COORDINATES_NAME, strlen(LONLAT_COORDINATES),
			  LONLAT_COORDINATES)) ERR;      

      /* Write some data to T. */
      if (nc_put_vara_float(ncid, 0, start, count, (const float *)temp)) ERR;

      /* Define xc variable. */
      if (nc_def_var(ncid, XC_NAME, NC_FLOAT, 1, &dimids[0], &varids[1])) ERR;
      if (nc_put_att_text(ncid, varids[1], AXIS, strlen(X_NAME), X_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[1], LONG_NAME, strlen(X_LONG_NAME),
			  X_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[1], UNITS, strlen(METER), METER)) ERR;      

      /* Write data to XC. */
      if (nc_put_var_float(ncid, 1, xc)) ERR;

      /* Define yc variable. */
      if (nc_def_var(ncid, YC_NAME, NC_FLOAT, 1, &dimids[1], &varids[2])) ERR;
      if (nc_put_att_text(ncid, varids[2], AXIS, strlen(Y_NAME), Y_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[2], LONG_NAME, strlen(Y_LONG_NAME),
			  Y_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[2], UNITS, strlen(METER), METER)) ERR;      

      /* Write data to YC. */
      if (nc_put_var_float(ncid, 2, yc)) ERR;

      /* Define lev variable. */
      if (nc_def_var(ncid, LEV_NAME, NC_FLOAT, 1, &dimids[2], &varids[3])) ERR;
      if (nc_put_att_text(ncid, varids[3], LONG_NAME, strlen(LEV_LONG_NAME),
			  LEV_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[3], UNITS, strlen(HPA), HPA)) ERR;      

      /* Define lon variable. */
      coord_dimids[0] = dimids[1];
      coord_dimids[1] = dimids[0];
      if (nc_def_var(ncid, LON_NAME, NC_FLOAT, COORD_NDIMS, coord_dimids, &varids[4])) ERR;
      if (nc_put_att_text(ncid, varids[4], LONG_NAME, strlen(LON_LONG_NAME),
			  LON_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[4], UNITS, strlen(DEGREES_EAST), DEGREES_EAST)) ERR;      

      /* Define lat variable. */
      if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, COORD_NDIMS, coord_dimids, &varids[5])) ERR;
      if (nc_put_att_text(ncid, varids[5], LONG_NAME, strlen(LAT_LONG_NAME),
			  LAT_LONG_NAME)) ERR;      
      if (nc_put_att_text(ncid, varids[5], UNITS, strlen(DEGREES_NORTH), DEGREES_NORTH)) ERR;      

      /* Check the metadata and data. */
      if (check_cf_metadata(ncid)) ERR;
      if (check_cf_data(ncid)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check the order of variables and dimensions. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

      /* Check the metadata and data. */
      if (check_cf_metadata(ncid)) ERR;
      if (check_cf_data(ncid)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}






