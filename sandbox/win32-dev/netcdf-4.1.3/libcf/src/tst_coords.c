/* 
Copyright 2005 University Corporation for Atmospheric Research/Unidata
See COPYRIGHT file for conditions of use. See www.unidata.ucar.edu for
more info.

This file is part of the libcf package; it tests libcf coordinate
systems.

Ed Hartnett 10/1/05

$Id$
*/

#include <nc_tests.h>
#include <netcdf.h>
#include <libcf.h>

#define DIM_LEN 4
#define DIM1_NAME "lat"
#define VAR_NAME "earth"
#define COORD_SYSTEM "coord_system"
#define COORD_NAME "coord_name"
#define TRANSFORM_NAME "transform_name"
#define TRANSFORM_TYPE1 "transform_type"
#define TRANSFORM_NAME "transform_name"

#define COORDINATE_AXES "_CoordinateAxes"
#define COORDINATE_Z_IS_POSITIVE "_CoordinateZisPositive"
#define Z_UP "up"
#define Z_DOWN "down"

/* These are used to implement tests that end up like John Caron's
 * examples of coordinate systems. */
#define EARTH "earth"
#define AIR "air"
#define TIME "time"
#define LEVEL "level"
#define LEVEL_LEN 17
#define LAT "lat"
#define LAT_LEN 73
#define LON "lon"
#define LON_LEN 144
#define NDIMS 4
#define LAT_LON_COORDINATE_SYSTEM "LatLonCoordinateSytem"
#define COORDINATE_SYSTEMS "_CoordinateSystems"
#define TRANSFORM_NAME "transform_name"
#define COORDINATE_TRANSFORMS "_CoordinateTransforms"
#define LAMBERT_CONFORMAL_PROJECTION "LambertConformalProjection"
#define PROJECTION "Projection"
#define LAMBERT_CONFORMAL_CONIC "lambert_conformal_conic"

/* Test everything for classic and 64-bit offsetfiles. If netcdf-4 is
 * included, that means another whole round of testing. */
#ifdef USE_NETCDF4
#define NUM_FORMATS (4)
#else
#define NUM_FORMATS (2)
#endif

static void
test_axis(const char *testfile)
{
   int ncid, dimid, varid, dimids[1] = {0};
   int axis_type;
   int nvars, ndims, natts, unlimdimid;
   char value[NC_MAX_NAME + 1];

   /* Create a file. */
   if (nc_create(testfile, NC_CLOBBER, &ncid)) ERR;
   
   /* Create an dimension and a coordinate var to go with it. */
   if (nc_def_dim(ncid, DIM1_NAME, DIM_LEN, &dimid)) ERR;
   if (dimid != 0) ERR;
   if (nc_def_var(ncid, DIM1_NAME, NC_FLOAT, 1, dimids, &varid)) ERR;
   if (varid != 0) ERR;
   if (nccf_def_axis_type(ncid, varid, NCCF_LONGITUDE)) ERR;
   if (nccf_inq_axis_type(ncid, varid, &axis_type)) ERR;
   if (axis_type != NCCF_LONGITUDE) ERR;
   
   /* Write the file. */
   if (nc_close(ncid)) ERR;
   
   /* Reopen the file, check the axis type. */
   if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != 1 && nvars != 2 && natts != 0 && unlimdimid != -1) ERR;
   if (nccf_inq_axis_type(ncid, 0, &axis_type)) ERR;
   if (axis_type != NCCF_LONGITUDE) ERR;

   if (nc_close(ncid)) ERR;

   /* Now create a file with the HEIGHT_UP axis. */
   if (nc_create(testfile, NC_CLOBBER, &ncid)) ERR;
   
   /* Create an dimension and a coordinate var to go with it. */
   if (nc_def_dim(ncid, DIM1_NAME, DIM_LEN, &dimid)) ERR;
   if (dimid != 0) ERR;
   if (nc_def_var(ncid, DIM1_NAME, NC_FLOAT, 1, dimids, &varid)) ERR;
   if (varid != 0) ERR;
   if (nccf_def_axis_type(ncid, varid, NCCF_HEIGHT_UP)) ERR;
   if (nccf_inq_axis_type(ncid, varid, &axis_type)) ERR;
   if (axis_type != NCCF_HEIGHT_UP) ERR;
   
   /* Write the file. */
   if (nc_close(ncid)) ERR;
   
   /* Reopen the file, check the axis type. */
   if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
   if (nccf_inq_axis_type(ncid, 0, &axis_type)) ERR;
   if (axis_type != NCCF_HEIGHT_UP) ERR;
   if (nc_get_att_text(ncid, varid, COORDINATE_Z_IS_POSITIVE, 
		       value)) ERR;
   if (strcmp(value, Z_UP)) ERR;
   
   if (nc_close(ncid)) ERR; 
}

static void
test_system(const char *testfile)
{
   int ncid, dimid, axis_varid, system_varid, dimids[1] = {0};
   int axes_varids[1] = {0};
   int nvars, ndims, natts, unlimdimid;
   char name_in[NC_MAX_NAME + 1];
   int naxes_in, axes_varids_in[1];

   /* Create a file. */
   if (nc_create(testfile, NC_CLOBBER, &ncid)) ERR;

   /* Create an dimension and a coordinate var to go with it. */
   if (nc_def_dim(ncid, DIM1_NAME, DIM_LEN, &dimid)) ERR;
   if (nc_def_var(ncid, DIM1_NAME, NC_FLOAT, 1, dimids, &axis_varid)) ERR;
   if (axis_varid != 0) ERR;
   if (nccf_def_coord_system(ncid, COORD_SYSTEM, 1, axes_varids, &system_varid)) ERR;
   if (system_varid != 1) ERR;
   if (nccf_inq_coord_system(ncid, 1, name_in, &naxes_in, axes_varids_in)) ERR;
   if (strcmp(name_in, COORD_SYSTEM) || naxes_in != 1 || 
       axes_varids_in[0] != axes_varids[0]) ERR;


   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Reopen the file and check it. */
   if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != 1 && nvars != 2 && natts != 0 && unlimdimid != -1) ERR;
   if (nccf_inq_coord_system(ncid, 1, name_in, &naxes_in, 
			   axes_varids_in)) ERR;
   if (strcmp(name_in, COORD_SYSTEM) || naxes_in != 1 || 
       axes_varids_in[0] != axes_varids[0]) ERR;

   if (nc_close(ncid)) ERR; 
}

/* Test the creation of a system, and it's assignment to a data
 * variable. THis matches example 2 from John's document (with some of
 * the other attributes left out.) */
static void
test_system_assign(const char *testfile)
{
   int ncid, axis_varids[NDIMS], system_varid, dimids[NDIMS];
   int nvars, ndims, natts, unlimdimid;
   int dimids_in[NDIMS], ndims_in;
   size_t len_in;
   char name_in[NC_MAX_NAME + 1];
   char systems_in[NC_MAX_NAME + 1];
   int naxes_in, axis_varids_in[NDIMS], natts_in;
   char *dim_name[] = {TIME, LEVEL, LAT, LON};
   size_t dim_len[NDIMS] = {NC_UNLIMITED, LEVEL_LEN, LAT_LEN, LON_LEN};
   int earth_varid, air_varid;
   int varid_in;
   nc_type type_in;
   int d;

   /* Create a file. */
   if (nc_create(testfile, NC_CLOBBER, &ncid)) ERR;

   /* Create 4 dimensions, and a coordinate var to go with each. */
   for (d = 0; d < NDIMS; d++)
   {
      if (nc_def_dim(ncid, dim_name[d], dim_len[d], &dimids[d])) ERR;
      if (nc_def_var(ncid, dim_name[d], NC_FLOAT, 1, &dimids[d], 
		     &axis_varids[d])) ERR;
   }

   /* Create two data vars, earth and air. (Don't know what happened
    * to fire and water Aristotle!) */
   if (nc_def_var(ncid, EARTH, NC_FLOAT, NDIMS, dimids, 
		  &earth_varid)) ERR;
   if (nc_def_var(ncid, AIR, NC_FLOAT, NDIMS, dimids, 
		  &air_varid)) ERR;
   
   /* Unite our 4 dims in a coordinate system. */
   if (nccf_def_coord_system(ncid, LAT_LON_COORDINATE_SYSTEM, NDIMS, 
			     axis_varids, &system_varid)) ERR;

   /* Check things out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != 4 && nvars != 7 && natts != 0 && unlimdimid != dimids[3]) ERR;
   if (nc_inq_varid(ncid, LAT_LON_COORDINATE_SYSTEM, &varid_in)) ERR;
   if (varid_in != system_varid) ERR;
   if (nc_inq_var(ncid, system_varid, name_in, &type_in, &ndims_in, 
		  dimids_in, &natts_in)) ERR;
   if (strcmp(name_in, LAT_LON_COORDINATE_SYSTEM) || type_in != NC_CHAR || 
       ndims_in != 0 || natts_in != 1);
   if (nc_inq_att(ncid, system_varid, COORDINATE_AXES, &type_in, &len_in)) ERR;
   if (type_in != NC_CHAR) ERR;
   if (nccf_inq_coord_system(ncid, system_varid, name_in, &naxes_in, axis_varids_in)) ERR;
   if (strcmp(name_in, LAT_LON_COORDINATE_SYSTEM) || naxes_in != NDIMS) ERR;
   for (d = 0; d < NDIMS; d++)
      if (axis_varids_in[d] != axis_varids[d]) ERR;

   /* Assign the system to the earth and air data variables. */
   if (nccf_assign_coord_system(ncid, earth_varid, system_varid)) ERR;
   if (nccf_assign_coord_system(ncid, air_varid, system_varid)) ERR;

   /* Check to ensure that the assignments happened. */
   if (nc_inq_varnatts(ncid, earth_varid, &natts_in)) ERR;
   if (natts_in != 1) ERR;
   if (nc_inq_att(ncid, earth_varid, COORDINATE_SYSTEMS, &type_in, &len_in)) ERR;
   if (type_in != NC_CHAR) ERR;
   if (nc_inq_varnatts(ncid, air_varid, &natts_in)) ERR;
   if (natts_in != 1) ERR;
   if (nc_get_att_text(ncid, earth_varid, COORDINATE_SYSTEMS, systems_in)) ERR;
   if (strcmp(systems_in, LAT_LON_COORDINATE_SYSTEM)) ERR;
   if (nc_inq_att(ncid, air_varid, COORDINATE_SYSTEMS, &type_in, &len_in)) ERR;
   if (type_in != NC_CHAR) ERR;
   if (nc_get_att_text(ncid, air_varid, COORDINATE_SYSTEMS, systems_in)) ERR;
   if (strcmp(systems_in, LAT_LON_COORDINATE_SYSTEM)) ERR;

   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Reopen the file and check it. */
   if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != 4 && nvars != 7 && natts != 0 && unlimdimid != dimids[3]) ERR;
   if (nc_inq_var(ncid, system_varid, name_in, &type_in, &ndims_in, 
		  dimids_in, &natts_in)) ERR;
   if (strcmp(name_in, LAT_LON_COORDINATE_SYSTEM) || type_in != NC_CHAR || 
       ndims_in != 0 || natts_in != 1);
   if (nc_inq_att(ncid, system_varid, COORDINATE_AXES, &type_in, &len_in)) ERR;
   if (type_in != NC_CHAR) ERR;
   if (nccf_inq_coord_system(ncid, system_varid, name_in, &naxes_in, axis_varids_in)) ERR;
   if (strcmp(name_in, LAT_LON_COORDINATE_SYSTEM) || naxes_in != NDIMS) ERR;
   for (d = 0; d < NDIMS; d++)
      if (axis_varids_in[d] != axis_varids[d]) ERR;

   /* Check to ensure that the assignments happened. */
   if (nc_inq_varnatts(ncid, earth_varid, &natts_in)) ERR;
   if (natts_in != 1) ERR;
   if (nc_inq_att(ncid, earth_varid, COORDINATE_SYSTEMS, &type_in, &len_in)) ERR;
   if (type_in != NC_CHAR) ERR;
   if (nc_inq_varnatts(ncid, air_varid, &natts_in)) ERR;
   if (natts_in != 1) ERR;
   if (nc_get_att_text(ncid, earth_varid, COORDINATE_SYSTEMS, systems_in)) ERR;
   if (strcmp(systems_in, LAT_LON_COORDINATE_SYSTEM)) ERR;
   if (nc_inq_att(ncid, air_varid, COORDINATE_SYSTEMS, &type_in, &len_in)) ERR;
   if (type_in != NC_CHAR) ERR;
   if (nc_get_att_text(ncid, air_varid, COORDINATE_SYSTEMS, systems_in)) ERR;
   if (strcmp(systems_in, LAT_LON_COORDINATE_SYSTEM)) ERR;

   if (nc_close(ncid)) ERR; 
}

static void
test_transform(const char *testfile)
{
   int ncid, transform_varid;
   int nvars, ndims, natts, unlimdimid;
   char name_in[NC_MAX_NAME + 1];
   char transform_name_in[NC_MAX_NAME + 1], transform_type_in[NC_MAX_NAME + 1];
   size_t type_len, name_len;

   /* Create a file. */
   if (nc_create(testfile, NC_CLOBBER, &ncid)) ERR;

   /* Create a transform. */
   if (nccf_def_transform(ncid, TRANSFORM_NAME, TRANSFORM_TYPE1, 
			  TRANSFORM_NAME, &transform_varid)) ERR;
   if (transform_varid != 0) ERR;
   if (nccf_inq_transform(ncid, transform_varid, name_in, &type_len, 
			  transform_type_in, &name_len, 
			  transform_name_in)) ERR;
   if (strcmp(name_in, TRANSFORM_NAME) || type_len != strlen(TRANSFORM_TYPE1) + 1 ||
       strcmp(transform_type_in, TRANSFORM_TYPE1) || name_len != strlen(TRANSFORM_NAME) + 1 ||
       strcmp(transform_name_in, TRANSFORM_NAME)) ERR;
   
   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Reopen the file and check it. */
   if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != 0 && nvars != 1 && natts != 0 && unlimdimid != -1) ERR;
   if (nccf_inq_transform(ncid, 0, name_in, &type_len, transform_type_in, 
			&name_len, transform_name_in)) ERR;
   if (strcmp(name_in, TRANSFORM_NAME) || type_len != strlen(TRANSFORM_TYPE1) + 1 ||
       strcmp(transform_type_in, TRANSFORM_TYPE1) || name_len != strlen(TRANSFORM_NAME) + 1 ||
       strcmp(transform_name_in, TRANSFORM_NAME)) ERR;

   if (nc_close(ncid)) ERR; 
}

/* Create a coordinate system and transform, and create a file like
 * John's example 4. */
static void
test_transform_assign(const char *testfile)
{
   int ncid, axis_varids[NDIMS], system_varid, dimids[NDIMS];
   int transform_varid;
   size_t len_in, name_len, type_len;
   char name_in[NC_MAX_NAME + 1];
   char transform_in[NC_MAX_NAME + 1];
   char transform_type_in[NC_MAX_NAME + 1], transform_name_in[NC_MAX_NAME + 1];
   char *dim_name[NDIMS] = {TIME, LEVEL, LAT, LON};
   size_t dim_len[NDIMS] = {NC_UNLIMITED, LEVEL_LEN, LAT_LEN, LON_LEN};
   int axis_type[NDIMS] = {NCCF_TIME, NCCF_HEIGHT_DOWN, NCCF_LATITUDE, NCCF_LONGITUDE};
   int earth_varid, air_varid;
   int d;

   /* Create a file. */
   if (nc_create(testfile, NC_CLOBBER, &ncid)) ERR;

   /* Create 4 dimensions, and a coordinate var to go with each. Also
    * set the axis type for each axis. */
   for (d = 0; d < NDIMS; d++)
   {
      if (nc_def_dim(ncid, dim_name[d], dim_len[d], &dimids[d])) ERR;
      if (nc_def_var(ncid, dim_name[d], NC_FLOAT, 1, &dimids[d], 
		     &axis_varids[d])) ERR;
      if (nccf_def_axis_type(ncid, axis_varids[d], axis_type[d])) ERR;
   }

   /* Create two data vars, earth and air. */
   if (nc_def_var(ncid, EARTH, NC_FLOAT, NDIMS, dimids, 
		  &earth_varid)) ERR;
   if (nc_def_var(ncid, AIR, NC_FLOAT, NDIMS, dimids, 
		  &air_varid)) ERR;
   
   /* Unite our 4 dims in a coordinate system. */
   if (nccf_def_coord_system(ncid, LAT_LON_COORDINATE_SYSTEM, NDIMS, 
			   axis_varids, &system_varid)) ERR;

   /* Assign the system to the earth and air data variables. */
   if (nccf_assign_coord_system(ncid, earth_varid, system_varid)) ERR;
   if (nccf_assign_coord_system(ncid, air_varid, system_varid)) ERR;

   /* Create a coordinate transform called LambertConformalProjection. */
   if (nccf_def_transform(ncid, LAMBERT_CONFORMAL_PROJECTION, PROJECTION, 
			LAMBERT_CONFORMAL_CONIC, &transform_varid)) ERR;
   if (nccf_inq_transform(ncid, transform_varid, name_in, &len_in, 
			transform_type_in, &name_len, transform_name_in)) ERR;
   if (strcmp(name_in, LAMBERT_CONFORMAL_PROJECTION) || len_in != strlen(PROJECTION) + 1 ||
       strcmp(transform_type_in, PROJECTION) || name_len != strlen(LAMBERT_CONFORMAL_CONIC) + 1 ||
       strcmp(transform_name_in, LAMBERT_CONFORMAL_CONIC)) ERR;

   /* Assign the transform to the coordinate system. */
   if (nccf_assign_transform(ncid, system_varid, transform_varid)) ERR;
   
   /* Check it out. */
   if (nc_get_att_text(ncid, system_varid, COORDINATE_TRANSFORMS, transform_in)) ERR;
   if (strcmp(transform_in, LAMBERT_CONFORMAL_PROJECTION)) ERR;

   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Reopen the file and check it. */
   if (nc_open(testfile, NC_NOWRITE, &ncid)) ERR;
   if (nccf_inq_transform(ncid, transform_varid, name_in, &type_len, 
			transform_type_in, &name_len, transform_name_in)) ERR;
   if (strcmp(name_in, LAMBERT_CONFORMAL_PROJECTION) || type_len != strlen(PROJECTION) + 1 ||
       strcmp(transform_type_in, PROJECTION) || name_len != strlen(LAMBERT_CONFORMAL_CONIC) + 1 ||
       strcmp(transform_name_in, LAMBERT_CONFORMAL_CONIC)) ERR;
   if (nc_get_att_text(ncid, system_varid, COORDINATE_TRANSFORMS, transform_in)) ERR;
   if (strcmp(transform_in, LAMBERT_CONFORMAL_PROJECTION)) ERR;

   if (nc_close(ncid)) ERR; 
}

int
main(int argc, char **argv)
{
   int i;
   char testfile[NC_MAX_NAME + 1];

   printf("\n*** Testing coordinate systems.\n");
   /*nc_set_log_level(3);*/

   /* Go thru formats and run all tests for each of two (for netCDF-3
    * only builds), or 3 (for netCDF-4 builds) different formats. */
   for (i = NUM_FORMATS; i >= 1; i--)
   {
      switch (i) 
      {
	 case NC_FORMAT_CLASSIC:
	    nc_set_default_format(NC_FORMAT_CLASSIC, NULL);
	    fprintf(stderr, "\nSwitching to netCDF classic format.\n");
	    strcpy(testfile, "tst_coords_classic.nc");
	    break;
	 case NC_FORMAT_64BIT:
	    nc_set_default_format(NC_FORMAT_64BIT, NULL);
	    fprintf(stderr, "\nSwitching to 64-bit offset format.\n");
	    strcpy(testfile, "tst_coords_64bit.nc");
	    break;
#ifdef USE_NETCDF4
	 case NC_FORMAT_NETCDF4_CLASSIC:
	    nc_set_default_format(NC_FORMAT_NETCDF4_CLASSIC, NULL);
	    strcpy(testfile, "tst_coords_netcdf4_classic.nc");
	    fprintf(stderr, "\nSwitching to netCDF-4 format (with NC_CLASSIC_MODEL).\n");
	    break;
	 case NC_FORMAT_NETCDF4: /* actually it's _CLASSIC. */
	    nc_set_default_format(NC_FORMAT_NETCDF4, NULL);
	    strcpy(testfile, "tst_coords_netcdf4.nc");
	    fprintf(stderr, "\nSwitching to netCDF-4 format.\n");
	    break;
#endif
	 default:
	    fprintf(stderr, "Unexpected format!\n");
	    return 2;
      }

      printf("*** creating coordinate axis...");
      test_axis(testfile);
      SUMMARIZE_ERR;
      
      printf("*** creating coordinate system...");
      test_system(testfile);
      SUMMARIZE_ERR;

      printf("*** assigning a coordinate system...");
      test_system_assign(testfile);
      SUMMARIZE_ERR;

      printf("*** creating coordinate transform...");
      test_transform(testfile);
      SUMMARIZE_ERR;

      printf("*** assigning a coordinate transform...");
      test_transform_assign(testfile);
      SUMMARIZE_ERR;
   }

   FINAL_RESULTS;
}

