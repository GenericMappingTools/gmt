/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_vars.nc"
#define VAR_BYTE_NAME "Portrait_of_Maria_Trip"
#define VAR_CHAR_NAME "Landscape_wth_stone_bridge"
#define VAR_SHORT_NAME "Self-portrait_as_a_young_man"
#define VAR_INT_NAME "Jeremiah_lamenting_the_destruction_of_Jerusalem"
#define VAR_FLOAT_NAME "The_Syndics_of_the_Drapers_Guild"
#define VAR_DOUBLE_NAME "Self-portrait_as_the_apstle_Paul"
#define VAR_UBYTE_NAME "Tobias_and_Anna_with_the_kid"
#define VAR_USHORT_NAME "The_Jewish_Bride"
#define VAR_UINT_NAME "The_prophetess_Anna"
#define VAR_INT64_NAME "Titus_as_a_Monk"
#define VAR_UINT64_NAME "The_Night_Watch"
#define DIM1_NAME "x"
#define DIM1_LEN 2
#define DIM2_NAME "y"
#define DIM2_LEN 3
#define DIM3_NAME "strlen"
#define DIM3_LEN 25
#define NC_ERR 2

/* From here down, the defines are for the creation of the
   pres_temp_4D example file, which is also created under examples,
   but here is created in all file formats, and more extensively
   tested for correctness. We are writing 4D data, a 2 x 6 x 12
   lvl-lat-lon grid, with 2 timesteps of data. */
#define NDIMS_EX 4
#define NLAT 6
#define NLON 12
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define NREC 2
#define REC_NAME "time"
#define LVL_NAME "level"
#define NLVL 2

/* Names of things. */
#define PRES_NAME "pressure"
#define TEMP_NAME "temperature"
#define UNITS "units"
#define DEGREES_EAST "degrees_east"
#define DEGREES_NORTH "degrees_north"

/* There are 4 vars, two for data, two for coordinate data. */
#define NVARS_EX 4

/* These are used to construct some example data. */
#define SAMPLE_PRESSURE 900
#define SAMPLE_TEMP 9.0
#define START_LAT 25.0
#define START_LON -125.0

/* For the units attributes. */
#define UNITS "units"
#define PRES_UNITS "hPa"
#define TEMP_UNITS "celsius"
#define LAT_UNITS "degrees_north"
#define LON_UNITS "degrees_east"
#define MAX_ATT_LEN 80

int
create_4D_example(char *file_name, int cmode) 
{
   /* IDs for the netCDF file, dimensions, and variables. */
   int ncid, lon_dimid, lat_dimid, lvl_dimid, rec_dimid;
   int lat_varid, lon_varid, pres_varid, temp_varid;
   int dimids[NDIMS_EX];

   /* The start and count arrays will tell the netCDF library where to
      write our data. */
   size_t start[NDIMS_EX], count[NDIMS_EX];

   /* Program variables to hold the data we will write out. We will only
      need enough space to hold one timestep of data; one record. */
   float pres_out[NLVL][NLAT][NLON];
   float temp_out[NLVL][NLAT][NLON];

   /* These program variables hold the latitudes and longitudes. */
   float lats[NLAT], lons[NLON];

   /* Loop indexes. */
   int lvl, lat, lon, rec, i = 0;
   
   /* Create some pretend data. If this wasn't an example program, we
    * would have some real data to write, for example, model
    * output. */
   for (lat = 0; lat < NLAT; lat++)
      lats[lat] = START_LAT + 5.*lat;
   for (lon = 0; lon < NLON; lon++)
      lons[lon] = START_LON + 5.*lon;
   
   for (lvl = 0; lvl < NLVL; lvl++)
      for (lat = 0; lat < NLAT; lat++)
	 for (lon = 0; lon < NLON; lon++)
	 {
	    pres_out[lvl][lat][lon] = SAMPLE_PRESSURE + i;
	    temp_out[lvl][lat][lon] = SAMPLE_TEMP + i++;
	 }

   /* Create the file. */
   if (nc_create(file_name, cmode, &ncid)) ERR;

   /* Define the dimensions. The record dimension is defined to have
    * unlimited length - it can grow as needed. In this example it is
    * the time dimension.*/
   if (nc_def_dim(ncid, LVL_NAME, NLVL, &lvl_dimid)) ERR;
   if (nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)) ERR;
   if (nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)) ERR;
   if (nc_def_dim(ncid, REC_NAME, NC_UNLIMITED, &rec_dimid)) ERR;

   /* Define the coordinate variables. We will only define coordinate
      variables for lat and lon.  Ordinarily we would need to provide
      an array of dimension IDs for each variable's dimensions, but
      since coordinate variables only have one dimension, we can
      simply provide the address of that dimension ID (&lat_dimid) and
      similarly for (&lon_dimid). */
   if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, &lat_dimid, 
			    &lat_varid)) ERR;
   if (nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, &lon_dimid, 
			    &lon_varid)) ERR;

   /* Assign units attributes to coordinate variables. */
   if (nc_put_att_text(ncid, lat_varid, UNITS, 
				 strlen(DEGREES_NORTH), DEGREES_NORTH)) ERR;
   if (nc_put_att_text(ncid, lon_varid, UNITS, 
				 strlen(DEGREES_EAST), DEGREES_EAST)) ERR;

   /* The dimids array is used to pass the dimids of the dimensions of
      the netCDF variables. Both of the netCDF variables we are
      creating share the same four dimensions. In C, the
      unlimited dimension must come first on the list of dimids. */
   dimids[0] = rec_dimid;
   dimids[1] = lvl_dimid;
   dimids[2] = lat_dimid;
   dimids[3] = lon_dimid;

   /* Define the netCDF variables for the pressure and temperature
    * data. */
   if (nc_def_var(ncid, PRES_NAME, NC_FLOAT, NDIMS_EX, 
			    dimids, &pres_varid)) ERR;
   if (nc_def_var(ncid, TEMP_NAME, NC_FLOAT, NDIMS_EX, 
			    dimids, &temp_varid)) ERR;

   /* Assign units attributes to the netCDF variables. */
   if (nc_put_att_text(ncid, pres_varid, UNITS, 
				 strlen(PRES_UNITS), PRES_UNITS)) ERR;
   if (nc_put_att_text(ncid, temp_varid, UNITS, 
				 strlen(TEMP_UNITS), TEMP_UNITS)) ERR;

   /* End define mode. */
   if (nc_enddef(ncid)) ERR;

   /* Write the coordinate variable data. This will put the latitudes
      and longitudes of our data grid into the netCDF file. */
   if (nc_put_var_float(ncid, lat_varid, &lats[0])) ERR;
   if (nc_put_var_float(ncid, lon_varid, &lons[0])) ERR;

   /* These settings tell netcdf to write one timestep of data. (The
      setting of start[0] inside the loop below tells netCDF which
      timestep to write.) */
   count[0] = 1;
   count[1] = NLVL;
   count[2] = NLAT;
   count[3] = NLON;
   start[1] = 0;
   start[2] = 0;
   start[3] = 0;

   /* Write the pretend data. This will write our surface pressure and
      surface temperature data. The arrays only hold one timestep worth
      of data. We will just rewrite the same data for each timestep. In
      a real application, the data would change between timesteps. */
   for (rec = 0; rec < NREC; rec++)
   {
      start[0] = rec;
      if (nc_put_vara_float(ncid, pres_varid, start, count, 
				      &pres_out[0][0][0])) ERR;
      if (nc_put_vara_float(ncid, temp_varid, start, count, 
				      &temp_out[0][0][0])) ERR;
   }

   /* Close the file. */
   if (nc_close(ncid)) ERR;

   return err;
}

int
check_4D_example(char *file_name, int expected_format)
{
   int ncid;
   int format, ndims_in, nvars_in, natts_in;
   int dimid[NDIMS_EX];
   int varid[NVARS_EX];
   char dim_name_in[NDIMS_EX][NC_MAX_NAME];
   char var_name_in[NVARS_EX][NC_MAX_NAME];
   char name_in[NC_MAX_NAME + 1];
   char units_in[NC_MAX_NAME + 1];
   nc_type xtype_in;
   size_t len_in;
   int i;

   if (nc_open(file_name, 0, &ncid)) ERR;

   /* Count stuff. */
   if (nc_inq_format(ncid, &format)) ERR;
   if (nc_inq_ndims(ncid, &ndims_in)) ERR;
   if (ndims_in != NDIMS_EX) ERR;
   if (nc_inq_nvars(ncid, &nvars_in)) ERR;
   if (nvars_in != NVARS_EX) ERR;
   if (nc_inq_natts(ncid, &natts_in)) ERR;
   if (natts_in != 0) ERR;
   if (format != expected_format) ERR;

   /* Check dimensions. */
   ndims_in = 0;
   if (nc_inq_dimids(ncid, &ndims_in, dimid, 0)) ERR;
   if (ndims_in != NDIMS_EX) ERR;
   for (i = 0; i < NDIMS_EX; i++)
   {
      if (dimid[i] != i) ERR;
      if (nc_inq_dimname(ncid, i, dim_name_in[i])) ERR;
   }
   if (strcmp(dim_name_in[0], LVL_NAME) || strcmp(dim_name_in[1], LAT_NAME) ||
       strcmp(dim_name_in[2], LON_NAME) || strcmp(dim_name_in[3], REC_NAME)) ERR;

   /* Check variables. */
   nvars_in = 0;
   if (nc_inq_varids(ncid, &nvars_in, varid)) ERR;
   if (nvars_in != NVARS_EX) ERR;
   for (i = 0; i < NVARS_EX; i++)
   {
      if (varid[i] != i) ERR;
      if (nc_inq_varname(ncid, i, var_name_in[i])) ERR;
   }
   if (strcmp(var_name_in[0], LAT_NAME) || strcmp(var_name_in[1], LON_NAME) ||
       strcmp(var_name_in[2], PRES_NAME) || strcmp(var_name_in[3], TEMP_NAME)) ERR;

   if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimid, &natts_in)) ERR;
   if (strcmp(name_in, LAT_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 || 
       dimid[0] != 1 || natts_in != 1) ERR;
   if (nc_inq_var(ncid, 1, name_in, &xtype_in, &ndims_in, dimid, &natts_in)) ERR;
   if (strcmp(name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 || 
       dimid[0] != 2 || natts_in != 1) ERR;
   if (nc_inq_var(ncid, 2, name_in, &xtype_in, &ndims_in, dimid, &natts_in)) ERR;
   if (strcmp(name_in, PRES_NAME) || xtype_in != NC_FLOAT || ndims_in != 4 || 
       dimid[0] != 3 || dimid[1] != 0 || dimid[2] != 1 || dimid[3] != 2 || 
       natts_in != 1) ERR;
   if (nc_inq_var(ncid, 3, name_in, &xtype_in, &ndims_in, dimid, &natts_in)) ERR;
   if (strcmp(name_in, TEMP_NAME) || xtype_in != NC_FLOAT || ndims_in != 4 || 
       dimid[0] != 3 || dimid[1] != 0 || dimid[2] != 1 || dimid[3] != 2 || 
       natts_in != 1) ERR;

   /* Check variable atts. */
   if (nc_inq_att(ncid, 0, UNITS, &xtype_in, &len_in)) ERR;
   if (xtype_in != NC_CHAR || len_in != strlen(DEGREES_NORTH)) ERR;
   if (nc_get_att_text(ncid, 0, UNITS, units_in)) ERR;
   if (strncmp(units_in, DEGREES_NORTH, strlen(DEGREES_NORTH))) ERR;

   if (nc_inq_att(ncid, 1, UNITS, &xtype_in, &len_in)) ERR;
   if (xtype_in != NC_CHAR || len_in != strlen(DEGREES_EAST)) ERR;
   if (nc_get_att_text(ncid, 1, UNITS, units_in)) ERR;
   if (strncmp(units_in, DEGREES_EAST, strlen(DEGREES_EAST))) ERR;

   if (nc_inq_att(ncid, 2, UNITS, &xtype_in, &len_in)) ERR;
   if (xtype_in != NC_CHAR || len_in != strlen(PRES_UNITS)) ERR;
   if (nc_get_att_text(ncid, 2, UNITS, units_in)) ERR;
   if (strncmp(units_in, PRES_UNITS, strlen(PRES_UNITS))) ERR;

   if (nc_inq_att(ncid, 3, UNITS, &xtype_in, &len_in)) ERR;
   if (xtype_in != NC_CHAR || len_in != strlen(TEMP_UNITS)) ERR;
   if (nc_get_att_text(ncid, 3, UNITS, units_in)) ERR;
   if (strncmp(units_in, TEMP_UNITS, strlen(TEMP_UNITS))) ERR;

   if (nc_close(ncid)) ERR;
   return err;
}

int
main(int argc, char **argv)
{
   int ncid, dimids[3];
   int char_varid, byte_varid, ubyte_varid, short_varid, int_varid, float_varid, double_varid;
   int ushort_varid, uint_varid, int64_varid, uint64_varid;
   int i, j;

   unsigned char ubyte_out[DIM1_LEN][DIM2_LEN] = {{1, 128, 255},{1, 128, 255}};
   signed char byte_in[DIM1_LEN][DIM2_LEN], byte_out[DIM1_LEN][DIM2_LEN] = {{-127, 1, 127},{-127, 1, 127}};
   unsigned short ushort_out[DIM1_LEN][DIM2_LEN] = {{110, 128, 255},{110, 128, 255}};
   short short_in[DIM1_LEN][DIM2_LEN], short_out[DIM1_LEN][DIM2_LEN] = {{-110, -128, 255},{-110, -128, 255}};
   int int_in[DIM1_LEN][DIM2_LEN], int_out[DIM1_LEN][DIM2_LEN] = {{0, 128, 255},{0, 128, 255}};
   float float_in[DIM1_LEN][DIM2_LEN], float_out[DIM1_LEN][DIM2_LEN] = {{-.1, 9999.99, 100.001},{-.1, 9999.99, 100.001}};
   double double_in[DIM1_LEN][DIM2_LEN], double_out[DIM1_LEN][DIM2_LEN] = {{0.02, .1128, 1090.1},{0.02, .1128, 1090.1}};
   unsigned int uint_in[DIM1_LEN][DIM2_LEN], uint_out[DIM1_LEN][DIM2_LEN] = {{0, 128, 255},{0, 128, 255}};
   long long int64_in[DIM1_LEN][DIM2_LEN], int64_out[DIM1_LEN][DIM2_LEN] = {{-111, 777, 100},{-111, 777, 100}};
   unsigned long long uint64_in[DIM1_LEN][DIM2_LEN];
   unsigned long long uint64_out[DIM1_LEN][DIM2_LEN] = {{0, 10101, 9999999},{0, 10101, 9999999}};
   char char_out[DIM1_LEN][DIM2_LEN][DIM3_LEN] = {{"lalala", "lololo", "lelele"}, {"lalala", "lololo", "lelele"}};

   printf("\n*** Testing netcdf-4 variable functions.\n");

   printf("*** testing netcdf-4 varids inq on netcdf-3 file...");
   {
      int nvars_in, varids_in[2];

      /* Create a netcdf-3 file with one dim and two vars. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimids[2])) ERR;
      if (nc_def_var(ncid, VAR_BYTE_NAME, NC_BYTE, 2, dimids, &byte_varid)) ERR;
      if (nc_def_var(ncid, VAR_CHAR_NAME, NC_CHAR, 3, dimids, &char_varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and make sure nc_inq_varids yeilds correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing simple variables...");

   {      
      /* Create a file with a variable of each type. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimids[2])) ERR;
      if (nc_def_var(ncid, VAR_BYTE_NAME, NC_BYTE, 2, dimids, &byte_varid)) ERR;
      if (nc_def_var(ncid, VAR_CHAR_NAME, NC_CHAR, 3, dimids, &char_varid)) ERR;
      if (nc_def_var(ncid, VAR_SHORT_NAME, NC_SHORT, 2, dimids, &short_varid)) ERR;
      if (nc_def_var(ncid, VAR_INT_NAME, NC_INT, 2, dimids, &int_varid)) ERR;
      if (nc_def_var(ncid, VAR_FLOAT_NAME, NC_FLOAT, 2, dimids, &float_varid)) ERR;
      if (nc_def_var(ncid, VAR_DOUBLE_NAME, NC_DOUBLE, 2, dimids, &double_varid)) ERR;
      if (nc_def_var(ncid, VAR_UBYTE_NAME, NC_UBYTE, 2, dimids, &ubyte_varid)) ERR;
      if (nc_def_var(ncid, VAR_USHORT_NAME, NC_USHORT, 2, dimids, &ushort_varid)) ERR;
      if (nc_def_var(ncid, VAR_UINT_NAME, NC_UINT, 2, dimids, &uint_varid)) ERR;
      if (nc_def_var(ncid, VAR_INT64_NAME, NC_INT64, 2, dimids, &int64_varid)) ERR;
      if (nc_def_var(ncid, VAR_UINT64_NAME, NC_UINT64, 2, dimids, &uint64_varid)) ERR;
      if (nc_put_var_schar(ncid, byte_varid, (signed char *)byte_out)) ERR;
      if (nc_put_var_text(ncid, char_varid, (char *)char_out)) ERR;
      if (nc_put_var_short(ncid, short_varid, (short *)short_out)) ERR;
      if (nc_put_var_int(ncid, int_varid, (int *)int_out)) ERR;
      if (nc_put_var_float(ncid, float_varid, (float *)float_out)) ERR;
      if (nc_put_var_double(ncid, double_varid, (double *)double_out)) ERR;
      if (nc_put_var_uchar(ncid, ubyte_varid, (unsigned char *)ubyte_out)) ERR;
      if (nc_put_var_ushort(ncid, ushort_varid, (unsigned short *)ushort_out)) ERR;
      if (nc_put_var_uint(ncid, uint_varid, (unsigned int *)uint_out)) ERR;
      if (nc_put_var_longlong(ncid, int64_varid, (long long *)int64_out)) ERR;
      if (nc_put_var_ulonglong(ncid, uint64_varid, (unsigned long long *)uint64_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check metadata. */
      {
	 nc_type xtype_in;
	 int ndims_in, dimids_in[10], natts_in, varid_in;
	 char name_in[NC_MAX_NAME+1];
	 size_t size_in;

	 if (nc_open(FILE_NAME, 0, &ncid)) ERR;
	 if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, 
			&natts_in)) ERR;
	 if (strcmp(name_in, VAR_BYTE_NAME) || xtype_in != NC_BYTE || 
	     ndims_in != 2 || natts_in != 0 || dimids_in[0] != dimids[0] ||
	     dimids_in[1] != dimids[1]) ERR;
	 if (nc_inq_varid(ncid, VAR_BYTE_NAME, &varid_in)) ERR;
	 if (varid_in != 0) ERR;
	 if (nc_inq_varid(ncid, VAR_CHAR_NAME, &varid_in)) ERR;
	 if (varid_in != 1) ERR;
	 if (nc_inq_varid(ncid, VAR_SHORT_NAME, &varid_in)) ERR;
	 if (varid_in != 2) ERR;
	 if (nc_inq_varname(ncid, 0, name_in)) ERR;
	 if (strcmp(name_in, VAR_BYTE_NAME)) ERR;
	 if (nc_inq_varname(ncid, 1, name_in)) ERR;
	 if (strcmp(name_in, VAR_CHAR_NAME)) ERR;
	 if (nc_inq_varname(ncid, 2, name_in)) ERR;
	 if (strcmp(name_in, VAR_SHORT_NAME)) ERR;
	 if (nc_inq_vartype(ncid, 0, &xtype_in)) ERR;      
	 if (xtype_in != NC_BYTE) ERR;
	 if (nc_inq_vartype(ncid, 1, &xtype_in)) ERR;      
	 if (xtype_in != NC_CHAR) ERR;
	 if (nc_inq_vartype(ncid, 2, &xtype_in)) ERR;      
	 if (xtype_in != NC_SHORT) ERR;

	 /* Check inquire of atomic types */
	 if (nc_inq_type(ncid, NC_BYTE, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "byte") || size_in != sizeof(char)) ERR;
	 if (nc_inq_type(ncid, NC_CHAR, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "char") || size_in != sizeof(char)) ERR;
	 if (nc_inq_type(ncid, NC_SHORT, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "short") || size_in != sizeof(short)) ERR;
	 if (nc_inq_type(ncid, NC_INT, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "int") || size_in != sizeof(int)) ERR;
	 if (nc_inq_type(ncid, NC_FLOAT, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "float") || size_in != sizeof(float)) ERR;
	 if (nc_inq_type(ncid, NC_DOUBLE, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "double") || size_in != sizeof(double)) ERR;
	 if (nc_inq_type(ncid, NC_UBYTE, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "ubyte") || size_in != sizeof(unsigned char)) ERR;
	 if (nc_inq_type(ncid, NC_USHORT, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "ushort") || size_in != sizeof(unsigned short)) ERR;
	 if (nc_inq_type(ncid, NC_UINT, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "uint") || size_in != sizeof(unsigned int)) ERR;
	 if (nc_inq_type(ncid, NC_INT64, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "int64") || size_in != sizeof(long long)) ERR;
	 if (nc_inq_type(ncid, NC_UINT64, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "uint64") || size_in != sizeof(unsigned long long)) ERR;
	 if (nc_inq_type(ncid, NC_STRING, name_in, &size_in)) ERR;
	 if (strcmp(name_in, "string") || size_in != sizeof(char *)) ERR;

	 if (nc_inq_typeid(ncid, "byte", &xtype_in)) ERR;
	 if (xtype_in != NC_BYTE) ERR;
	 if (nc_inq_typeid(ncid, "char", &xtype_in)) ERR;
	 if (xtype_in != NC_CHAR) ERR;
	 if (nc_inq_typeid(ncid, "short", &xtype_in)) ERR;
	 if (xtype_in != NC_SHORT) ERR;
	 if (nc_inq_typeid(ncid, "int", &xtype_in)) ERR;
	 if (xtype_in != NC_INT) ERR;
	 if (nc_inq_typeid(ncid, "float", &xtype_in)) ERR;
	 if (xtype_in != NC_FLOAT) ERR;
	 if (nc_inq_typeid(ncid, "double", &xtype_in)) ERR;
	 if (xtype_in != NC_DOUBLE) ERR;
	 if (nc_inq_typeid(ncid, "ubyte", &xtype_in)) ERR;
	 if (xtype_in != NC_UBYTE) ERR;
	 if (nc_inq_typeid(ncid, "ushort", &xtype_in)) ERR;
	 if (xtype_in != NC_USHORT) ERR;
	 if (nc_inq_typeid(ncid, "uint", &xtype_in)) ERR;
	 if (xtype_in != NC_UINT) ERR;
	 if (nc_inq_typeid(ncid, "int64", &xtype_in)) ERR;
	 if (xtype_in != NC_INT64) ERR;
	 if (nc_inq_typeid(ncid, "uint64", &xtype_in)) ERR;
	 if (xtype_in != NC_UINT64) ERR;
	 if (nc_inq_typeid(ncid, "string", &xtype_in)) ERR;
	 if (xtype_in != NC_STRING) ERR;

	 if (nc_close(ncid)) ERR;
      }

      /* Open the file and check data. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_schar(ncid, byte_varid, (signed char *)byte_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != byte_out[i][j]) ERR;
      if (nc_get_var_short(ncid, short_varid, (short *)short_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (short_in[i][j] != short_out[i][j]) ERR;
      if (nc_get_var_int(ncid, int_varid, (int *)int_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (int_in[i][j] != int_out[i][j]) ERR;
      if (nc_get_var_float(ncid, float_varid, (float *)float_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (float_in[i][j] != float_out[i][j]) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != double_out[i][j]) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != double_out[i][j]) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != double_out[i][j]) ERR;
      if (nc_get_var_uint(ncid, uint_varid, (unsigned int *)uint_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (uint_in[i][j] != uint_out[i][j]) ERR;
      if (nc_get_var_longlong(ncid, int64_varid, (long long *)int64_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (int64_in[i][j] != int64_out[i][j]) ERR;
      if (nc_get_var_ulonglong(ncid, uint64_varid, (unsigned long long *)uint64_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (uint64_in[i][j] != uint64_out[i][j]) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and read everything as double. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_double(ncid, byte_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)byte_out[i][j]) ERR;
      if (nc_get_var_double(ncid, ubyte_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)ubyte_out[i][j]) ERR;
      if (nc_get_var_double(ncid, short_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)short_out[i][j]) ERR;
      if (nc_get_var_double(ncid, ushort_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)ushort_out[i][j]) ERR;
      if (nc_get_var_double(ncid, int_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)int_out[i][j]) ERR;
      if (nc_get_var_double(ncid, uint_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)uint_out[i][j]) ERR;
      if (nc_get_var_double(ncid, float_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)float_out[i][j]) ERR;
      if (nc_get_var_double(ncid, int64_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)int64_out[i][j]) ERR;
      if (nc_get_var_double(ncid, uint64_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)uint64_out[i][j]) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and read everything as NC_BYTE. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_schar(ncid, byte_varid, (signed char *)byte_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)byte_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, ubyte_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)ubyte_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, short_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)short_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, ushort_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)ushort_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, int_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)int_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, uint_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)uint_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, float_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)float_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, int64_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)int64_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, uint64_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)uint64_out[i][j]) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

#define DEFLATE_LEVEL_4 4

   printf("*** testing simple variables with deflation...");
   {
      /* Create a file with a variable of each type. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimids[2])) ERR;
      if (nc_def_var(ncid, VAR_BYTE_NAME, NC_BYTE, 2, dimids, &byte_varid)) ERR;
      if (nc_def_var_deflate(ncid, byte_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_CHAR_NAME, NC_CHAR, 3, dimids, &char_varid)) ERR;
      if (nc_def_var_deflate(ncid, byte_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_SHORT_NAME, NC_SHORT, 2, dimids, &short_varid)) ERR;
      if (nc_def_var_deflate(ncid, short_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_INT_NAME, NC_INT, 2, dimids, &int_varid)) ERR;
      if (nc_def_var_deflate(ncid, int_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_FLOAT_NAME, NC_FLOAT, 2, dimids, &float_varid)) ERR;
      if (nc_def_var_deflate(ncid, float_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_DOUBLE_NAME, NC_DOUBLE, 2, dimids, &double_varid)) ERR;
      if (nc_def_var_deflate(ncid, double_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_UBYTE_NAME, NC_UBYTE, 2, dimids, &ubyte_varid)) ERR;
      if (nc_def_var_deflate(ncid, ubyte_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_USHORT_NAME, NC_USHORT, 2, dimids, &ushort_varid)) ERR;
      if (nc_def_var_deflate(ncid, ushort_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_UINT_NAME, NC_UINT, 2, dimids, &uint_varid)) ERR;
      if (nc_def_var_deflate(ncid, uint_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_INT64_NAME, NC_INT64, 2, dimids, &int64_varid)) ERR;
      if (nc_def_var_deflate(ncid, int64_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;
      if (nc_def_var(ncid, VAR_UINT64_NAME, NC_UINT64, 2, dimids, &uint64_varid)) ERR;
      if (nc_def_var_deflate(ncid, uint64_varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL_4)) ERR;

      if (nc_put_var_schar(ncid, byte_varid, (signed char *)byte_out)) ERR_RET;
      if (nc_put_var_text(ncid, char_varid, (char *)char_out)) ERR;
      if (nc_put_var_short(ncid, short_varid, (short *)short_out)) ERR;
      if (nc_put_var_int(ncid, int_varid, (int *)int_out)) ERR;
      if (nc_put_var_float(ncid, float_varid, (float *)float_out)) ERR;
      if (nc_put_var_double(ncid, double_varid, (double *)double_out)) ERR;
      if (nc_put_var_uchar(ncid, ubyte_varid, (unsigned char *)ubyte_out)) ERR;
      if (nc_put_var_ushort(ncid, ushort_varid, (unsigned short *)ushort_out)) ERR;
      if (nc_put_var_uint(ncid, uint_varid, (unsigned int *)uint_out)) ERR;
      if (nc_put_var_longlong(ncid, int64_varid, (long long *)int64_out)) ERR;
      if (nc_put_var_ulonglong(ncid, uint64_varid, (unsigned long long *)uint64_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check metadata. */
      {
	 nc_type xtype_in;
	 int ndims_in, dimids_in[10], natts_in, varid_in;
	 char name_in[NC_MAX_NAME+1];

	 if (nc_open(FILE_NAME, 0, &ncid)) ERR;
	 if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, 
			&natts_in)) ERR;
	 if (strcmp(name_in, VAR_BYTE_NAME) || xtype_in != NC_BYTE || 
	     ndims_in != 2 || natts_in != 0 || dimids_in[0] != dimids[0] ||
	     dimids_in[1] != dimids[1]) ERR;
	 if (nc_inq_varid(ncid, VAR_BYTE_NAME, &varid_in)) ERR;
	 if (varid_in != 0) ERR;
	 if (nc_inq_varid(ncid, VAR_CHAR_NAME, &varid_in)) ERR;
	 if (varid_in != 1) ERR;
	 if (nc_inq_varid(ncid, VAR_SHORT_NAME, &varid_in)) ERR;
	 if (varid_in != 2) ERR;
	 if (nc_inq_varname(ncid, 0, name_in)) ERR;
	 if (strcmp(name_in, VAR_BYTE_NAME)) ERR;
	 if (nc_inq_varname(ncid, 1, name_in)) ERR;
	 if (strcmp(name_in, VAR_CHAR_NAME)) ERR;
	 if (nc_inq_varname(ncid, 2, name_in)) ERR;
	 if (strcmp(name_in, VAR_SHORT_NAME)) ERR;
	 if (nc_inq_vartype(ncid, 0, &xtype_in)) ERR;      
	 if (xtype_in != NC_BYTE) ERR;
	 if (nc_inq_vartype(ncid, 1, &xtype_in)) ERR;      
	 if (xtype_in != NC_CHAR) ERR;
	 if (nc_inq_vartype(ncid, 2, &xtype_in)) ERR;      
	 if (xtype_in != NC_SHORT) ERR;
	 if (nc_close(ncid)) ERR;
      }

      /* Open the file and check data. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_schar(ncid, byte_varid, (signed char *)byte_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != byte_out[i][j]) ERR;
      if (nc_get_var_short(ncid, short_varid, (short *)short_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (short_in[i][j] != short_out[i][j]) ERR;
      if (nc_get_var_int(ncid, int_varid, (int *)int_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (int_in[i][j] != int_out[i][j]) ERR;
      if (nc_get_var_float(ncid, float_varid, (float *)float_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (float_in[i][j] != float_out[i][j]) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != double_out[i][j]) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != double_out[i][j]) ERR;
      if (nc_get_var_double(ncid, double_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != double_out[i][j]) ERR;
      if (nc_get_var_uint(ncid, uint_varid, (unsigned int *)uint_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (uint_in[i][j] != uint_out[i][j]) ERR;
      if (nc_get_var_longlong(ncid, int64_varid, (long long *)int64_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (int64_in[i][j] != int64_out[i][j]) ERR;
      if (nc_get_var_ulonglong(ncid, uint64_varid, (unsigned long long *)uint64_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (uint64_in[i][j] != uint64_out[i][j]) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and read everything as double. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_double(ncid, byte_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)byte_out[i][j]) ERR;
      if (nc_get_var_double(ncid, ubyte_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)ubyte_out[i][j]) ERR;
      if (nc_get_var_double(ncid, short_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)short_out[i][j]) ERR;
      if (nc_get_var_double(ncid, ushort_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)ushort_out[i][j]) ERR;
      if (nc_get_var_double(ncid, int_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)int_out[i][j]) ERR;
      if (nc_get_var_double(ncid, uint_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)uint_out[i][j]) ERR;
      if (nc_get_var_double(ncid, float_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)float_out[i][j]) ERR;
      if (nc_get_var_double(ncid, int64_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)int64_out[i][j]) ERR;
      if (nc_get_var_double(ncid, uint64_varid, (double *)double_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (double_in[i][j] != (double)uint64_out[i][j]) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and read everything as NC_BYTE. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_get_var_schar(ncid, byte_varid, (signed char *)byte_in)) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)byte_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, ubyte_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)ubyte_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, short_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)short_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, ushort_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)ushort_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, int_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)int_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, uint_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)uint_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, float_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)float_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, int64_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)int64_out[i][j]) ERR;
      if (nc_get_var_schar(ncid, uint64_varid, (signed char *)byte_in) != NC_ERANGE) ERR;   
      for (i = 0; i < DIM1_LEN; i++)
	 for (j = 0; j < DIM2_LEN; j++)
	    if (byte_in[i][j] != (signed char)uint64_out[i][j]) ERR;
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;

#define NDIMS4 1
#define NVARS4 1
#define DIM4_NAME "treaty_of_paris_1783"
#define DIM4_LEN 5
#define VAR_NAME4 "John_Adams"
#define DEFLATE_LEVEL 6

   printf("*** testing netcdf-4 simple variable define...");
   {
      int dimids[NDIMS4], dimids_in[NDIMS4];
      int varid, varids_in[NVARS4];
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int shuffle_in, deflate_in, deflate_level;

      /* Create a netcdf-4 file with one dim and one var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM4_NAME, DIM4_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      if (nc_def_var(ncid, VAR_NAME4, NC_INT64, NDIMS4, dimids, &varid)) ERR;
      if (nc_def_var_deflate(ncid, varid, NC_NOSHUFFLE, 1, DEFLATE_LEVEL)) ERR;
      if (varid != 0) ERR;

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS4 || nvars != NVARS4 || natts != 0 ||
	  unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != NVARS4) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims,
		     dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME4) || xtype_in != NC_INT64 ||
	  ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
      if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, 
			     &deflate_level)) ERR;
      if (shuffle_in != NC_NOSHUFFLE ||!deflate_in || 
	  deflate_level != DEFLATE_LEVEL) ERR;

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS4 || nvars != NVARS4 || natts != 0 ||
	  unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != NVARS4) ERR;
      if (varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims,
		     dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME4) || xtype_in != NC_INT64 ||
	  ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
      if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, 
			     &deflate_level)) ERR;
      if (shuffle_in != NC_NOSHUFFLE ||!deflate_in || 
	  deflate_level != DEFLATE_LEVEL) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;

#define NDIMS5 1
#define NVARS5 5
#define DIM5_NAME "treaty_of_paris_1783"
#define DIM5_LEN 5

   printf("*** testing netcdf-4 less simple variable define...");
   {
      int dimids[NDIMS5], dimids_in[NDIMS5];
      int varid[NVARS5], varids_in[NVARS5];
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      char var_name[NVARS5][NC_MAX_NAME + 1] = {"Jean-Pierre_Blanchard", "Madame_Blanchard",
						"Giffard", "Stanislas_Charles_Henri_Dupuy_de_Lome",
						"Charles_F_Ritchel"};
      int shuffle_in, deflate_in, deflate_level_in;
      int deflate_level[NVARS5];
      int i;

      /* Set up options for this var. */
      for (i = 0; i < NVARS5; i++)
	 deflate_level[i] = i;

      /* Create a netcdf-4 file with one dim and two vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
      if (dimids[0] != 0) ERR;
      for (i = 0; i < NVARS5; i++)
      {
	 if (nc_def_var(ncid, var_name[i], NC_INT64, NDIMS5, dimids, 
			&varid[i])) ERR;
	 if (varid[i] != i) ERR;
	 if (nc_def_var_deflate(ncid, varid[i], NC_SHUFFLE, 1, deflate_level[i])) ERR;
      }

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS5 || nvars != NVARS5 || natts != 0 ||
	  unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != NVARS5) ERR;
      for (i = 0; i < NVARS5; i++)
      {
	 if (varids_in[i] != i) ERR;
	 if (nc_inq_var(ncid, i, name_in, &xtype_in, &ndims,
			dimids_in, &natts)) ERR;
	 if (strcmp(name_in, var_name[i]) || xtype_in != NC_INT64 ||
	     ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
	 if (nc_inq_var_deflate(ncid, varid[i], &shuffle_in, &deflate_in, 
				&deflate_level_in)) ERR;
	 if (shuffle_in != NC_SHUFFLE || !deflate_in || 
	     deflate_level_in != deflate_level[i]) ERR;
      }

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS5 || nvars != NVARS5 || natts != 0 ||
	  unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != NVARS5) ERR;
      for (i = 0; i < NVARS5; i++)
      {
	 if (varids_in[i] != i) ERR;
	 if (nc_inq_var(ncid, i, name_in, &xtype_in, &ndims,
			dimids_in, &natts)) ERR;
	 if (strcmp(name_in, var_name[i]) || xtype_in != NC_INT64 ||
	     ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
	 if (nc_inq_var_deflate(ncid, varid[i], &shuffle_in, &deflate_in, 
				&deflate_level_in)) ERR;
	 if (shuffle_in != NC_SHUFFLE || !deflate_in || 
	     deflate_level_in != deflate_level[i]) ERR;
      }

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;

#define NVARS 5
#define NDIMS 1
#define DIM6_NAME "airship_cross_sectional_area"
#define DIM6_LEN 100
#define TEN_K_M2 10000.0
#define INCREMENT 1000.0

   printf("*** testing more complex netcdf-4 variable defines...");
   {
      int dimids[NDIMS], dimids_in[NDIMS];
      int varid[NVARS], varids_in[NVARS];
      int ndims, nvars, natts, unlimdimid;
      char var_name[NVARS][50] = {"Jean-Pierre_Blanchard", "Madame_Blanchard",
				  "Giffard", "Stanislas_Charles_Henri_Dupuy_de_Lome",
				  "Charles_F_Ritchel"};
      double data[DIM6_LEN];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int shuffle_in, deflate_in, deflate_level_in;
      int checksum_in;
      int i;

      /* Create some phoney data. */
      for (i = 1, data[0] = TEN_K_M2; i < DIM6_LEN; i++)
	 data[i] = data[i - 1] + INCREMENT;

      /* Create a netcdf-4 file with one dim and 5 NC_DOUBLE vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM6_NAME, DIM6_LEN, &dimids[0])) ERR;
      for (i = 0; i < NVARS; i++)
      {
	 if (nc_def_var(ncid, var_name[i], NC_DOUBLE, NDIMS, dimids,
			&varid[i])) ERR;
	 if (nc_def_var_deflate(ncid, varid[i], NC_NOSHUFFLE, 1, 0)) ERR;
	 if (nc_def_var_fletcher32(ncid, varid[i], NC_FLETCHER32)) ERR;
      }

      /* Check stuff. */
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS || nvars != NVARS || natts != 0 ||
	  unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != NVARS) ERR;
      for (i = 0; i < NVARS; i++)
	 if (varids_in[i] != i) ERR;
      for (i = 0; i < NVARS; i++)
      {
	 if (nc_inq_var(ncid, i, name_in, &xtype_in, &ndims,
			dimids_in, &natts)) ERR;
	 if (strcmp(name_in, var_name[i]) || xtype_in != NC_DOUBLE ||
	     ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
	 if (nc_inq_var_deflate(ncid, varid[i], &shuffle_in, &deflate_in, 
				&deflate_level_in)) ERR;
	 if (shuffle_in != NC_NOSHUFFLE || !deflate_in || deflate_level_in != 0) ERR;
	 if (nc_inq_var_fletcher32(ncid, varid[i], &checksum_in)) ERR;
	 if (checksum_in != NC_FLETCHER32) ERR;
      }

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != NDIMS || nvars != NVARS || natts != 0 ||
	  unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars, varids_in)) ERR;
      if (nvars != NVARS) ERR;
      for (i = 0; i < NVARS; i++)
	 if (varids_in[i] != i) ERR;
      for (i = 0; i < NVARS; i++)
      {
	 if (nc_inq_var(ncid, i, name_in, &xtype_in, &ndims,
			dimids_in, &natts)) ERR;
	 if (strcmp(name_in, var_name[i]) || xtype_in != NC_DOUBLE ||
	     ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
	 if (nc_inq_var_deflate(ncid, varid[i], &shuffle_in, &deflate_in, 
				&deflate_level_in)) ERR;
	 if (shuffle_in != NC_NOSHUFFLE || !deflate_in || 
	     deflate_level_in != 0) ERR;
	 if (nc_inq_var_fletcher32(ncid, varid[i], &checksum_in)) ERR;
	 if (checksum_in != NC_FLETCHER32) ERR;
      }

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
#define DIM7_LEN 2
#define DIM7_NAME "dim_7_from_Indiana"
#define VAR7_NAME "var_7_from_Idaho"
#define NDIMS 1

   printf("*** testing fill values...");
   {
      int dimids[NDIMS], dimids_in[NDIMS];
      size_t index[NDIMS];
      int varid, ndims, natts;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int shuffle_in, deflate_in, deflate_level_in;
      int checksum_in, no_fill;
      unsigned short ushort_data = 42, ushort_data_in, fill_value_in;

      /* Create a netcdf-4 file with one dim and 1 NC_USHORT var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM7_NAME, DIM7_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR7_NAME, NC_USHORT, NDIMS, dimids,
		     &varid)) ERR;

      /* Check stuff. */
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims,
		     dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR7_NAME) || xtype_in != NC_USHORT ||
	  ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
      if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, 
			     &deflate_level_in)) ERR;
      if (shuffle_in != NC_NOSHUFFLE || deflate_in) ERR;
      if (nc_inq_var_fletcher32(ncid, 0, &checksum_in)) ERR;
      if (checksum_in != NC_NOCHECKSUM) ERR;
      if (nc_inq_var_fill(ncid, 0, &no_fill, &fill_value_in)) ERR;
      if (no_fill || fill_value_in != NC_FILL_USHORT) ERR;

      /* Write the second of two values. */
      index[0] = 1;
      if (nc_put_var1_ushort(ncid, 0, index, &ushort_data)) ERR;

      /* Get the first value, and make sure we get the default fill
       * value for USHORT. */
      index[0] = 0;
      if (nc_get_var1_ushort(ncid, 0, index, &ushort_data_in)) ERR;
      if (ushort_data_in != NC_FILL_USHORT) ERR;

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check stuff. */
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims,
		     dimids_in, &natts)) ERR;
      if (strcmp(name_in, VAR7_NAME) || xtype_in != NC_USHORT ||
	  ndims != 1 || natts != 0 || dimids_in[0] != 0) ERR;
      if (nc_inq_var_deflate(ncid, 0, &shuffle_in, &deflate_in, 
			     &deflate_level_in)) ERR;
      if (shuffle_in != NC_NOSHUFFLE || deflate_in) ERR;
      if (nc_inq_var_fletcher32(ncid, 0, &checksum_in)) ERR;
      if (checksum_in != NC_NOCHECKSUM) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing more fill values...");
   {
      int dimids[NDIMS];
      size_t index[NDIMS];
      int varid;
      int no_fill;
      unsigned short ushort_data = 42, ushort_data_in, fill_value_in;

      /* Create a netcdf-4 file with one dim and 1 NC_USHORT var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM7_NAME, DIM7_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR7_NAME, NC_USHORT, NDIMS, dimids,
		     &varid)) ERR;
      if (nc_def_var_fill(ncid, varid, 1, NULL)) ERR;

      /* Check stuff. */
      if (nc_inq_var_fill(ncid, varid, &no_fill, &fill_value_in)) ERR;
      if (!no_fill) ERR;

      /* Write the second of two values. */
      index[0] = 1;
      if (nc_put_var1_ushort(ncid, varid, index, &ushort_data)) ERR;

      /* Get the first value, and make sure we get the default fill
       * value for USHORT. */
      index[0] = 0;
      if (nc_get_var1_ushort(ncid, varid, index, &ushort_data_in)) ERR;

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check stuff. */
      if (nc_inq_var_fill(ncid, varid, &no_fill, &fill_value_in)) ERR;
      if (!no_fill) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing fill values for 2D unlimited dimension variable...");
   {
#define D1_NAME "unlimited"
#define D1_TARGET 3      
#define D2_NAME "fixed"
#define D2_LEN 3
#define D2_TARGET 2
#define V1_NAME "var1"
#define ND1 2
#define TARGET_VALUE 42      

      int dimids[ND1];
      size_t index[ND1];
      int varid;
      int no_fill;
      int data = TARGET_VALUE, data_in[D1_TARGET][D2_LEN], fill_value_in;
      int i, j;

      /* Create a netcdf-4 file with one dim and 1 NC_USHORT var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D1_NAME, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, D2_NAME, D2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, V1_NAME, NC_INT, ND1, dimids, &varid)) ERR;

      /* Check stuff. */
      if (nc_inq_var_fill(ncid, varid, &no_fill, &fill_value_in)) ERR;
      if (no_fill) ERR;

      /* Write a value. */
      index[0] = D1_TARGET;
      index[1] = D2_TARGET;
      if (nc_put_var1_int(ncid, varid, index, &data)) ERR;

      /* Get the data, and check the values. */
      if (nc_get_var_int(ncid, 0, &data_in[0][0])) ERR;
      for (i = 0; i < D1_TARGET; i++)
	 for (j = 0; j < D2_LEN; j++)
	    if ((i == D1_TARGET && j == D2_TARGET && data_in[i][j] != TARGET_VALUE) ||
		data_in[i][j] != NC_FILL_INT) ERR;

      if (nc_close(ncid)) ERR;

      /* Open the file and check the same stuff. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Get the data, and check the values. */
      if (nc_get_var_int(ncid, 0, &data_in[0][0])) ERR;
      for (i = 0; i < D1_TARGET; i++)
	 for (j = 0; j < D2_LEN; j++)
	    if ((i == D1_TARGET && j == D2_TARGET && data_in[i][j] != TARGET_VALUE) ||
		data_in[i][j] != NC_FILL_INT) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing lots of variables...");
#define DIM_A_NAME "x"
#define DIM_A_LEN 10
#define NUM_VARS 2000
#define MAX_VARNAME 10
   {
      /* This simple test failed on HDF5 1.7.58, but passes just fine
       * on 1.8.0 alpha5... */
      int ncid, dimids[1], i;
      char varname[MAX_VARNAME];
      int varids[NUM_VARS];

      /* Create a file with three dimensions. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_A_NAME, DIM_A_LEN, &dimids[0])) ERR;

      /* Create a large number of variables. */
      for (i = 0; i < NUM_VARS; i++)
      {
	 sprintf(varname, "a_%d", i);
	 if (nc_def_var(ncid, varname, NC_FLOAT, 1, dimids, &varids[i])) {
	    ERR;
	    break;
	 }
      }
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

#define NC3_CLASSIC_FILE "tst_pres_temp_4D_classic.nc"
#define NC3_64BIT_OFFSET_FILE "tst_pres_temp_4D_64bit_offset.nc"
#define NC3_NETCDF4_FILE "tst_pres_temp_4D_netcdf4.nc"
#define NC3_NETCDF4_CLASSIC_FILE "tst_pres_temp_4D_netcdf4_classic.nc"

   printf("*** testing 4D example file in classic format...");
   if (create_4D_example(NC3_CLASSIC_FILE, NC_CLOBBER)) ERR;
   if (check_4D_example(NC3_CLASSIC_FILE, NC_FORMAT_CLASSIC)) ERR;      
   SUMMARIZE_ERR;

   printf("*** testing 4D example file in 64-bit offset format...");
   if (create_4D_example(NC3_64BIT_OFFSET_FILE, NC_CLOBBER|NC_64BIT_OFFSET)) ERR;
   if (check_4D_example(NC3_64BIT_OFFSET_FILE, NC_FORMAT_64BIT)) ERR;      
   SUMMARIZE_ERR;

   printf("*** testing 4D example file in netCDF-4/HDF5 format...");
   if (create_4D_example(NC3_NETCDF4_FILE, NC_CLOBBER|NC_NETCDF4)) ERR;
   if (check_4D_example(NC3_NETCDF4_FILE, NC_FORMAT_NETCDF4)) ERR;      
   SUMMARIZE_ERR;

   printf("*** testing 4D example file in netCDF-4/HDF5 format with classic model rules...");
   if (create_4D_example(NC3_NETCDF4_CLASSIC_FILE, NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL)) ERR;
   if (check_4D_example(NC3_NETCDF4_CLASSIC_FILE, NC_FORMAT_NETCDF4_CLASSIC)) ERR;      
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}


