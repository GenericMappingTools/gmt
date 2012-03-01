/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 dimensions. 

   $Id$
*/
#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_dims.nc"
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define LEVEL_NAME "level"
#define TIME_NAME "time"
#define DIM5_NAME "twilight_zone"
#define DIM_NAME "dim"
#define LAT_LEN 1
#define LON_LEN 2
#define LEVEL_LEN 3
#define TIME_LEN 4
#define DIM5_LEN 5
#define LAT_DIMID 0
#define LON_DIMID 1
#define LEVEL_DIMID 2
#define TIME_DIMID 3
#define LAT_VARID 0
#define LON_VARID 1
#define PRES_VARID 2
#define ELEV_VARID 3
#define HP_VARID 4
#define PRES_NAME "pressure"
#define ELEV_NAME "Elevation"
#define HP_NAME "Number_of_Harry_Potter_Books"
#define BUBBA "Bubba"

#define MAX_DIMS 5
      
int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 dimensions.\n");
   printf("*** Testing that netcdf-4 dimids inq works on netcdf-3 file...");
   {
      int ncid, dimid;
      int ndims_in, dimids_in[MAX_DIMS];

      /* Create a netcdf-3 file with one dim. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &dimid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and make sure nc_inq_dimids yeilds correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
      if (ndims_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing that netcdf-4 dimids inq works on more complex netcdf-3 file...");
   {
      int ncid, dimid;
      int lon_dimid;
      int ndims_in, dimids_in[MAX_DIMS];

      /* Create a netcdf-3 file with three dim. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_def_dim(ncid, LEVEL_NAME, NC_UNLIMITED, &dimid)) ERR;
      if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &dimid)) ERR;
      if (nc_def_dim(ncid, LON_NAME, LON_LEN, &lon_dimid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and make sure nc_inq_dimids yeilds correct
       * result. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 3 || dimids_in[0] != 0 || dimids_in[1] != 1 ||
	 dimids_in[2] != 2) ERR;
      if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Testing file with just one dimension...");
   {
      int ncid, dimid;
      int ndims_in, dimids_in[MAX_DIMS];
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int varid_in;

      /* Create a file with one dim and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &dimid)) ERR;

      /* Check out what we've got. */
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1) ERR;
      if (nc_inq_dimid(ncid, LAT_NAME, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
      if (ndims_in != 0) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen and check it out again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1) ERR;
      if (nc_inq_dimid(ncid, LAT_NAME, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
      if (ndims_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Testing renaming of one dimension...");
   {
      int ncid, dimid, varid_in;
      char name_in[NC_MAX_NAME + 1];
      size_t len_in;
      int ndims_in, dimids_in[MAX_DIMS];

      /* Reopen the file with one dim, and change the name of the dim. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_rename_dim(ncid, 0, BUBBA)) ERR;

      /* Check out what we've got. */
      dimid = 0;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, BUBBA)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1) ERR;
      if (dimids_in[0] != 0) ERR;
      if (nc_inq_dimid(ncid, BUBBA, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, BUBBA)) ERR;
      if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen and check out what we've got again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, BUBBA)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_dimid(ncid, BUBBA, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, BUBBA)) ERR;
      if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing file with just one unlimited dimension...");
   {
      int ncid, dimid;
      int ndims_in, dimids_in[MAX_DIMS];
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int unlimdimid_in;
      int nunlimdims_in;

      /* Create a file with one unlimied dim and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, LEVEL_NAME, NC_UNLIMITED, &dimid)) ERR;

      /* Check it out before the enddef. */
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != NC_UNLIMITED || strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_unlimdim(ncid, &unlimdimid_in)) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq_unlimdims(ncid, &nunlimdims_in, &unlimdimid_in)) ERR;
      if (nunlimdims_in != 1 || unlimdimid_in != 0) ERR;
      
      /* Automatically enddef and close. */
      if (nc_close(ncid)) ERR;

      /* Reopen and check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != NC_UNLIMITED || strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_unlimdim(ncid, &unlimdimid_in)) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq_unlimdims(ncid, &nunlimdims_in, &unlimdimid_in)) ERR;
      if (nunlimdims_in != 1 || unlimdimid_in != 0) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
#define ROMULUS "Romulus"
#define REMUS "Remus"
#define DIMS2 2   
   printf("*** Testing file with two unlimited dimensions...");
   {
      int ncid, dimid[DIMS2];
      int ndims_in, dimids_in[DIMS2];
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int unlimdimid_in[DIMS2];
      int nunlimdims_in;

      /* Create a file with two unlimited dims and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, REMUS, NC_UNLIMITED, &dimid[0])) ERR;
      if (nc_def_dim(ncid, ROMULUS, NC_UNLIMITED, &dimid[1])) ERR;

      /* Check it out before the enddef. */
      if (nc_inq_dim(ncid, dimid[0], name_in, &len_in)) ERR;
      if (len_in != NC_UNLIMITED || strcmp(name_in, REMUS)) ERR;
      if (nc_inq_dim(ncid, dimid[1], name_in, &len_in)) ERR;
      if (len_in != NC_UNLIMITED || strcmp(name_in, ROMULUS)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 2 || dimids_in[0] != 0 || dimids_in[1] != 1) ERR;
      if (nc_inq_unlimdim(ncid, &unlimdimid_in[0])) ERR;
      if (unlimdimid_in[0] != 1) ERR;
      if (nc_inq_unlimdims(ncid, &nunlimdims_in, unlimdimid_in)) ERR;
      if (nunlimdims_in != 2 || unlimdimid_in[0] != 1 || unlimdimid_in[1] != 0) ERR;
      
      /* Automatically enddef and close. */
      if (nc_close(ncid)) ERR;

      /* Reopen and check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing ordering of dimensions...");
   {
#define A_NAME "a"
#define B_NAME "b"
#define A_LEN 50
#define B_LEN 92
#define A_DIMID 1
#define B_DIMID 0

      int ncid;
      int ndims_in, dimids_in[MAX_DIMS];
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int dimid_a, dimid_b;

      /* Create a file with two dims and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, B_NAME, B_LEN, &dimid_b)) ERR;
      if (nc_def_dim(ncid, A_NAME, A_LEN, &dimid_a)) ERR;
      if (dimid_b != B_DIMID || dimid_a != A_DIMID) ERR;

      /* Check out what we've got. */
      if (nc_inq_dim(ncid, dimid_a, name_in, &len_in)) ERR;
      if (len_in != A_LEN || strcmp(name_in, A_NAME)) ERR;
      if (nc_inq_dim(ncid, dimid_b, name_in, &len_in)) ERR;
      if (len_in != B_LEN || strcmp(name_in, B_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 2 || dimids_in[0] != 0 || dimids_in[1] != 1) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check it out again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_dim(ncid, B_DIMID, name_in, &len_in)) ERR;
      if (len_in != B_LEN || strcmp(name_in, B_NAME)) ERR;
      if (nc_inq_dim(ncid, A_DIMID, name_in, &len_in)) ERR;
      if (len_in != A_LEN || strcmp(name_in, A_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 2 || dimids_in[0] != 0 ||
	  dimids_in[1] != 1) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing file with just one unlimited dimension and one var...");
   {
      int ncid, dimid, dimids[MAX_DIMS];
      int level_varid;
      int natts_in, ndims_in, dimids_in[MAX_DIMS];
      nc_type xtype_in;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int unlimdimid_in;
      size_t start[MAX_DIMS], count[MAX_DIMS];
      int varid_in, nvars_in, nunlimdims_in;
      unsigned long long uint64_data[1] = {42};

      /* Create a file with one unlimied dim and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, LEVEL_NAME, NC_UNLIMITED, &dimid)) ERR;
      if (dimid != 0) ERR;
      dimids[0] = dimid;
      if (nc_def_var(ncid, LEVEL_NAME, NC_UINT64, 1, dimids, &level_varid)) ERR;
      if (level_varid != 0) ERR;

      /* Check it out before enddef. */
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != 0 || strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_unlimdim(ncid, &unlimdimid_in)) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq_unlimdims(ncid, &nunlimdims_in, &unlimdimid_in)) ERR;
      if (nunlimdims_in != 1 || unlimdimid_in != 0) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_varid(ncid, LEVEL_NAME, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LEVEL_NAME) || xtype_in != NC_UINT64 || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;
      if (nc_inq_varname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, LEVEL_NAME)) ERR;

      /* Now write one record of data to the var. */
      start[0] = 0;
      count[0] = 1;
      if (nc_put_vara_ulonglong(ncid, 0, start, count, uint64_data)) ERR;

      /* Check dimension informaiton again. Now the length of this
       * dimension should be one. */
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != 1 || strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
      if (len_in != 1) ERR;
      if (nc_inq_unlimdim(ncid, &unlimdimid_in)) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq_unlimdims(ncid, &nunlimdims_in, &unlimdimid_in)) ERR;
      if (nunlimdims_in != 1 || unlimdimid_in != 0) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_varid(ncid, LEVEL_NAME, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LEVEL_NAME) || xtype_in != NC_UINT64 || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;
      if (nc_inq_varname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, LEVEL_NAME)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != 1 || strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_unlimdim(ncid, &unlimdimid_in)) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq_unlimdims(ncid, &nunlimdims_in, &unlimdimid_in)) ERR;
      if (nunlimdims_in != 1 || unlimdimid_in != 0) ERR;
      if (unlimdimid_in != 0) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != 0) ERR;
      if (nc_inq_varid(ncid, LEVEL_NAME, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LEVEL_NAME) || xtype_in != NC_UINT64 || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;
      if (nc_inq_varname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing adding a coordinate var to file with dimension...");
   {
      int ncid, dimid, dimids[MAX_DIMS];
      int natts_in, ndims_in, dimids_in[MAX_DIMS];
      nc_type xtype_in;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int unlimdimid_in;
      int nvars_in, dim5_varid;

      /* Create a file with one dim and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimid)) ERR;

      /* Check out what we've got. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 0 || natts_in != 0 ||
	 unlimdimid_in != -1) ERR;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (len_in != DIM5_LEN || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check it out again. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 0 || natts_in != 0 ||
	 unlimdimid_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != DIM5_LEN || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;

      /* Add a coordinate var for this dimension. */
      dimids[0] = 0;
      if (nc_def_var(ncid, DIM5_NAME, NC_FLOAT, 1, dimids, &dim5_varid)) ERR;

      /* Check it out. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 ||
	 unlimdimid_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != DIM5_LEN || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, DIM5_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check it out again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 ||
	 unlimdimid_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != DIM5_LEN || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, DIM5_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing adding a coordinate var to file with unlimited dimension...");
   {
      int ncid, dimid, dimids[MAX_DIMS];
      int natts_in, ndims_in, dimids_in[MAX_DIMS];
      nc_type xtype_in;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int unlimdimid_in;
      size_t start[MAX_DIMS], count[MAX_DIMS], index[MAX_DIMS];
      unsigned short data[2] = {42, 24}, data_in[2];
      int nvars_in, hp_varid, dim5_varid;

      /* Create a file with one dim and one var. This time make
       * it an unlimited dim. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM5_NAME, NC_UNLIMITED, &dimid)) ERR;
      if (dimid != 0) ERR;
      dimids[0] = dimid;
      if (nc_def_var(ncid, HP_NAME, NC_USHORT, 1, dimids, &hp_varid)) ERR;
      if (hp_varid != 0) ERR;

      /* Check out what we've got. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 ||
	 unlimdimid_in != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != 0 || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, HP_NAME) || xtype_in != NC_USHORT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;

      /* Add a record to the HP variable. */
      start[0] = 0;
      count[0] = 1;
      if (nc_put_vara(ncid, hp_varid, start, count, data)) ERR;

      /* Check to ensure dimension grew. */
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != 1 || strcmp(name_in, DIM5_NAME)) ERR;

      /* Reread the value just written. */
      index[0] = 0;
      if (nc_get_var1_ushort(ncid, hp_varid, index, data_in)) ERR;
      if (data_in[0] != data[0]) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check it out again. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 ||
	 unlimdimid_in != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != 1 || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;

      /* Add a coordinate var for this dimension. */
      dimids[0] = 0;
      if (nc_def_var(ncid, DIM5_NAME, NC_FLOAT, 1, dimids, &dim5_varid)) ERR;

      /* Check it out. */
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 2 || natts_in != 0 ||
	 unlimdimid_in != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != 1 || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, dim5_varid, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, DIM5_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and check it out again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 2 || natts_in != 0 ||
	 unlimdimid_in != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in)) ERR;
      if (len_in != 1 || strcmp(name_in, DIM5_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1 || dimids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, dim5_varid, name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, DIM5_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating file with 1 data var, 2 dims, and 2 coord. vars...");

   {
      int ncid, dimids[MAX_DIMS];
      int lat_dimid, lon_dimid, lat_varid, lon_varid;
      int pres_varid;
      char name_in[NC_MAX_NAME + 1];
      int natts_in, ndims_in, dimids_in[MAX_DIMS];
      nc_type xtype_in;
      size_t len_in;
      float lat[LAT_LEN], lon[LON_LEN];
      float lat_in[LAT_LEN], lon_in[LON_LEN];
      double pres[LAT_LEN][LON_LEN], pres_in[LAT_LEN][LON_LEN];
      int i, j;

      /* Lats and lons suitable for some South American data. */
      for (lat[0] = 40.0, i = 1; i < LAT_LEN; i++)
	 lat[i] = lat[i - 1] + .5;
      for (lon[0] = 20.0, i = 1; i < LON_LEN; i++)
	 lon[i] = lon[i - 1] + 1.5;

      /* Some phoney 2D pressure data. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    pres[i][j] = 1013.1 + j;
   
      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define lat and lon dimensions, with associated variables. */
      if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &lat_dimid)) ERR;
      dimids[0] = lat_dimid;
      if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, dimids, &lat_varid)) ERR;
      if (nc_def_dim(ncid, LON_NAME, LON_LEN, &lon_dimid)) ERR;
      dimids[0] = lon_dimid;
      if (nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, dimids, &lon_varid)) ERR;

      /* Define a 2D variable called pressure, with NC_DOUBLE on a lat
       * lon grid. */
      dimids[0] = lat_dimid;
      dimids[1] = lon_dimid;
      if (nc_def_var(ncid, PRES_NAME, NC_DOUBLE, 2, dimids, &pres_varid)) ERR;

      /* Check our dimensions. */
      if (nc_inq_dim(ncid, lat_dimid, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dim(ncid, lon_dimid, name_in, &len_in)) ERR;
      if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;
      if (nc_inq_var(ncid, lat_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LAT_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != lat_dimid || natts_in != 0) ERR;
      if (nc_inq_var(ncid, lon_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != lon_dimid || natts_in != 0) ERR;

      /* Check our data variable. */
      if (nc_inq_var(ncid, pres_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, PRES_NAME) || xtype_in != NC_DOUBLE || ndims_in != 2 ||
	  dimids_in[0] != lat_dimid || dimids_in[1] != lon_dimid || natts_in != 0) ERR;
      
      /* Write our latitude and longitude values. This writes all
       * metadata to disk too. */
      if (nc_put_var_float(ncid, lat_varid, lat)) ERR;
      if (nc_put_var_float(ncid, lon_varid, lon)) ERR;

      /* Write our 2D pressure values. */
      if (nc_put_var_double(ncid, pres_varid, (double *)pres)) ERR;

      /* Check our latitude and longitude values. */
      if (nc_get_var(ncid, lat_varid, lat_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 if (lat[i] != lat_in[i]) ERR;
      if (nc_get_var_float(ncid, lon_varid, lon_in)) ERR;
      for (i = 0; i < LON_LEN; i++)
	 if (lon[i] != lon_in[i]) ERR;

      /* Check our pressure values. */
      if (nc_get_var_double(ncid, pres_varid, (double *)pres_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    if (pres[i][j] != pres_in[i][j]) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it out again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check our dimensions. */
      if (nc_inq_dim(ncid, lat_dimid, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dim(ncid, lon_dimid, name_in, &len_in)) ERR;
      if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;
      if (nc_inq_var(ncid, lat_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LAT_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != lat_dimid || natts_in != 0) ERR;
      if (nc_inq_var(ncid, lon_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != lon_dimid || natts_in != 0) ERR;

      /* Check our data variable. */
      if (nc_inq_var(ncid, pres_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, PRES_NAME) || xtype_in != NC_DOUBLE || ndims_in != 2 ||
	  dimids_in[0] != lat_dimid || dimids_in[1] != lon_dimid || natts_in != 0) ERR;
      
      /* Check our latitude and longitude values. */
      if (nc_get_var(ncid, lat_varid, lat_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 if (lat[i] != lat_in[i]) ERR;
      if (nc_get_var_float(ncid, lon_varid, lon_in)) ERR;
      for (i = 0; i < LON_LEN; i++)
	 if (lon[i] != lon_in[i]) ERR;

      /* Check our pressure values. */
      if (nc_get_var_double(ncid, pres_varid, (double *)pres_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    if (pres[i][j] != pres_in[i][j]) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating file with 3 data vars, 4 dims, and 2 coord. vars...");

   {
      int ncid, lat_dimid, dimids[MAX_DIMS];
      int level_dimid, time_dimid, elev_varid;
      int lat_varid, lon_dimid, lon_varid, pres_varid, hp_varid;
      double pres[LAT_LEN][LON_LEN][LEVEL_LEN][TIME_LEN];
      double pres_in[LAT_LEN][LON_LEN][LEVEL_LEN][TIME_LEN];
      unsigned short hp[LAT_LEN][LON_LEN][TIME_LEN];
      unsigned short hp_in[LAT_LEN][LON_LEN][TIME_LEN];
      unsigned long long elev[LAT_LEN][LON_LEN], elev_in[LAT_LEN][LON_LEN];
      size_t start[4], count[4];
      int nvars, ndims, natts, unlimdimid;
      int natts_in, ndims_in, dimids_in[MAX_DIMS];
      nc_type xtype_in;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      float lat[LAT_LEN], lon[LON_LEN];
      float lat_in[LAT_LEN], lon_in[LON_LEN];
      int i, j, k, l;

      /* Some phony 4D pressure data. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    for (k = 0; k < LEVEL_LEN; k++)
	       for (l = 0; l <TIME_LEN; l++)
		  pres[i][j][k][l] = 1013.1 + j;
   
      /* Some phony 3D hp data. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    for (l = 0; l <TIME_LEN; l++)
	       hp[i][j][l] = 100 + l;
   
      /* Some phony 2D elevaton data. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    elev[i][j] = 1010101022223333ULL  + i + j;

      /* Some phony 1D lats and lons. */
      for (i = 0; i < LAT_LEN; i++)
	 lat[i] = i * 5.;
      for (i = 0; i < LON_LEN; i++)
	 lon[i] = i * 5.;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define lat, lon, level, and timestep dimensions, with
       * associated coordinate variables for lat and lon only. Time is
       * an unlimited dimension. */
      if (nc_def_dim(ncid, LAT_NAME, LAT_LEN, &lat_dimid)) ERR;
      if (lat_dimid != LAT_DIMID) ERR;
      dimids[0] = lat_dimid;
      if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, dimids, &lat_varid)) ERR;
      if (lat_varid != LAT_VARID) ERR;
      if (nc_def_dim(ncid, LON_NAME, LON_LEN, &lon_dimid)) ERR;
      if (lon_dimid != LON_DIMID) ERR;
      dimids[0] = lon_dimid;
      if (nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, dimids, &lon_varid)) ERR;
      if (lon_varid != LON_VARID) ERR;
      if (nc_def_dim(ncid, LEVEL_NAME, LEVEL_LEN, &level_dimid)) ERR;
      if (level_dimid != LEVEL_DIMID) ERR;
      if (nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &time_dimid)) ERR;
      if (time_dimid != TIME_DIMID) ERR;

      /* Define a 4D NC_DOUBLE variable called pressure. */
      dimids[0] = lat_dimid;
      dimids[1] = lon_dimid;
      dimids[2] = level_dimid;
      dimids[3] = time_dimid;
      if (nc_def_var(ncid, PRES_NAME, NC_DOUBLE, 4, dimids, &pres_varid)) ERR;
      if (pres_varid != PRES_VARID) ERR;

      /* Define a 2D variable for surface elevation. Use NC_INT64
       * because our units for this is Angstroms from Earth's
       * Center. */
      if (nc_def_var(ncid, ELEV_NAME, NC_INT64, 2, dimids, &elev_varid)) ERR;
      if (elev_varid != ELEV_VARID) ERR;
      
      /* Define a 3D NC_USHORT variable to store the number of Harry
       * Potter books in this grid square at this time (ignore HP
       * books in airplanes, dirigibles, hot air balloons, space
       * capsules, hang-gliders, parachutes, and flying on brooms). */
      dimids[2] = time_dimid;
      if (nc_def_var(ncid, HP_NAME, NC_USHORT, 3, dimids, &hp_varid)) ERR;
      if (hp_varid != HP_VARID) ERR;

      /* Did all our stuff make it into the internal metadata model
       * intact? */
      /* Check our dimensions. */
      if (nc_inq_dim(ncid, LAT_DIMID, name_in, &len_in)) ERR;
      if (len_in != LAT_LEN || strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dim(ncid, LON_DIMID, name_in, &len_in)) ERR;
      if (len_in != LON_LEN || strcmp(name_in, LON_NAME)) ERR;
      if (nc_inq_dim(ncid, LEVEL_DIMID, name_in, &len_in)) ERR;
      if (len_in != LEVEL_LEN || strcmp(name_in, LEVEL_NAME)) ERR;
      if (nc_inq_dim(ncid, TIME_DIMID, name_in, &len_in)) ERR;
      if (len_in != 0 || strcmp(name_in, TIME_NAME)) ERR;

      /* Check our coordinate variables. */
      if (nc_inq_var(ncid, LAT_VARID, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LAT_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != LAT_DIMID || natts_in != 0) ERR;
      if (nc_inq_var(ncid, LON_VARID, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 ||
	  dimids_in[0] != LON_DIMID || natts_in != 0) ERR;

      /* Check our data variables. */
      if (nc_inq_var(ncid, PRES_VARID, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, PRES_NAME) || xtype_in != NC_DOUBLE || ndims_in != 4 ||
	  dimids_in[0] != LAT_DIMID || dimids_in[1] != LON_DIMID ||
	  dimids_in[2] != LEVEL_DIMID || dimids_in[3] != TIME_DIMID ||
	  natts_in != 0) ERR;
      if (nc_inq_var(ncid, ELEV_VARID, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, ELEV_NAME) || xtype_in != NC_INT64 || ndims_in != 2 ||
	  dimids_in[0] != LAT_DIMID || dimids_in[1] != LON_DIMID ||
	  natts_in != 0) ERR;
      if (nc_inq_var(ncid, HP_VARID, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, HP_NAME) || xtype_in != NC_USHORT || ndims_in != 3 ||
	  dimids_in[0] != LAT_DIMID || dimids_in[1] != LON_DIMID ||
	  dimids_in[2] != TIME_DIMID || natts_in != 0) ERR;
      
      /* Write our latitude and longitude values. This writes all
       * metadata to disk too. */
      if (nc_put_var_float(ncid, lat_varid, lat)) ERR;
      if (nc_put_var_float(ncid, lon_varid, lon)) ERR;

      /* Write our 4D pressure, elevation, and hp data. But this
       * should do nothing for pressure and hp, because these are
       * record vars, and nc_put_var_* doesn't do anything to record
       * vars, because it doesn't know how big the var is supposed to
       * be. */
      if (nc_put_var_double(ncid, pres_varid, (double *)pres)) ERR;
      if (nc_put_var_ulonglong(ncid, elev_varid, (unsigned long long *)elev)) ERR;
      if (nc_put_var_ushort(ncid, hp_varid, (unsigned short *)hp)) ERR;

      /* Check our latitude and longitude values. */
      if (nc_get_var(ncid, lat_varid, lat_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 if (lat[i] != lat_in[i]) ERR;
      if (nc_get_var_float(ncid, lon_varid, lon_in)) ERR;
      for (i = 0; i < LON_LEN; i++)
	 if (lon[i] != lon_in[i]) ERR;

      /* Make sure our pressure and hp variables are still
       * empty. get_var calls will return no error, but fetch no
       * data. */
      if (nc_inq_var(ncid, pres_varid, name_in, &xtype_in, &ndims_in,
		     dimids_in, &natts_in)) ERR;
      if (nc_inq_dim(ncid, dimids_in[3], NULL, &len_in)) ERR;
      if (len_in != 0) ERR;
/*      if (nc_get_var_double(ncid, pres_varid, (double *)pres_in)) ERR;*/
      if (nc_inq_var(ncid, hp_varid, NULL, NULL, &ndims_in,
		     dimids_in, NULL)) ERR;
      if (nc_inq_dim(ncid, dimids_in[2], NULL, &len_in)) ERR;
      if (len_in != 0) ERR;
/*      if (nc_get_var_ushort(ncid, hp_varid, (unsigned short *)hp_in)) ERR;*/

      /* Now use nc_put_vara to really write pressure and hp
       * data. Write TIME_LEN (4) records of each. */
      start[0] = start[1] = start[2] = start[3] = 0;
      count[0] = LAT_LEN;
      count[1] = LON_LEN;
      count[2] = LEVEL_LEN;
      count[3] = TIME_LEN;
      if (nc_put_vara(ncid, pres_varid, start, count, 
		      (double *)pres)) ERR;
      count[2] = TIME_LEN;
      if (nc_put_vara(ncid, hp_varid, start, count,
		      (double *)hp)) ERR;

      /* Check our pressure values. */
      if (nc_get_var_double(ncid, pres_varid, (double *)pres_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    for (k = 0; k < LEVEL_LEN; k++)
	       for (l = 0; l <TIME_LEN; l++)
		  if (pres[i][j][k][l] != pres_in[i][j][k][l]) ERR;

      /* Check our elevation values. */
      if (nc_get_var_ulonglong(ncid, elev_varid, (unsigned long long *)elev_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    if (elev[i][j] != elev_in[i][j]) ERR;

      /* Check our hp values. */
      if (nc_get_var_ushort(ncid, hp_varid, (unsigned short *)hp_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    for (l = 0; l <TIME_LEN; l++)
	       if (hp[i][j][l] != hp_in[i][j][l]) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it out again. At the moment, we
       * can't count on the varids and dimids being the same. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 4 || nvars != 5 || natts != 0) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Testing file with dims and only some coordinate vars...");
#define NDIMS_EX 4
#define NLAT 6
#define NLON 12
#define LAT_NAME_EX "latitude"
#define LON_NAME_EX "longitude"
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
   {
      /* IDs for the netCDF file, dimensions, and variables. */
      int ncid, lon_dimid, lat_dimid;
      int lon_varid;
      int ndims_in;
      int dimid[NDIMS_EX];
      char dim_name_in[NDIMS_EX][NC_MAX_NAME];
      int i;

      /* Create the file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define the dimensions. */
      if (nc_def_dim(ncid, LAT_NAME_EX, NLAT, &lat_dimid)) ERR;
      if (nc_def_dim(ncid, LON_NAME_EX, NLON, &lon_dimid)) ERR;

      /* Define a coordinate var. */
      if (nc_def_var(ncid, LON_NAME_EX, NC_FLOAT, 1, &lon_dimid,
			       &lon_varid)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, 0, &ncid)) ERR;

      /* Check dimensions. */
      ndims_in = 0;
      if (nc_inq_dimids(ncid, &ndims_in, dimid, 0)) ERR;
      if (ndims_in != 2) ERR;
      for (i = 0; i < 2; i++)
      {
	 if (dimid[i] != i) ERR;
	 if (nc_inq_dimname(ncid, i, dim_name_in[i])) ERR;
      }
      if (strcmp(dim_name_in[0], LAT_NAME_EX) ||
	  strcmp(dim_name_in[1], LON_NAME_EX)) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Testing file with just one very long dimension...");
   {
#define VERY_LONG_LEN (size_t)4500000000LL
      int ncid, dimid;
      int ndims_in, dimids_in[MAX_DIMS];
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int varid_in;

      if (SIZEOF_SIZE_T == 8)
      {
	 /* Create a file with one dim and nothing else. */
	 if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
	 if (nc_def_dim(ncid, LAT_NAME, VERY_LONG_LEN, &dimid)) ERR;

	 /* Check it out. */
	 if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
	 if (len_in != ((SIZEOF_SIZE_T == 8) ? VERY_LONG_LEN : NC_MAX_UINT) || 
	     strcmp(name_in, LAT_NAME)) ERR;
	 if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
	 if (ndims_in != 1) ERR;
	 if (nc_inq_dimid(ncid, LAT_NAME, &varid_in)) ERR;
	 if (varid_in != 0) ERR;
	 if (nc_inq_dimname(ncid, 0, name_in)) ERR;
	 if (strcmp(name_in, LAT_NAME)) ERR;
	 if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
	 if (len_in != ((SIZEOF_SIZE_T == 8) ? VERY_LONG_LEN : NC_MAX_UINT)) ERR;
	 if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
	 if (ndims_in != 0) ERR;
	 if (nc_close(ncid)) ERR;

	 /* Reopen and check it out again. */
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	 /* Check it out. */
	 if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
	 if (len_in != ((SIZEOF_SIZE_T == 8) ? VERY_LONG_LEN : NC_MAX_UINT) || 
	     strcmp(name_in, LAT_NAME)) ERR;
	 if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
	 if (ndims_in != 1) ERR;
	 if (nc_inq_dimid(ncid, LAT_NAME, &varid_in)) ERR;
	 if (varid_in != 0) ERR;
	 if (nc_inq_dimname(ncid, 0, name_in)) ERR;
	 if (strcmp(name_in, LAT_NAME)) ERR;
	 if (nc_inq_dimlen(ncid, 0, &len_in)) ERR;
	 if (len_in != ((SIZEOF_SIZE_T == 8) ? VERY_LONG_LEN : NC_MAX_UINT)) ERR;
	 if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
	 if (ndims_in != 0) ERR;
	 if (nc_close(ncid)) ERR;
      }
   }
   SUMMARIZE_ERR;
   printf("*** Testing reference file with just one very long dimension...");
   {
#define REF_FILE_NAME "ref_tst_dims.nc"
      int ncid, dimid = 0;
      int ndims_in, dimids_in[MAX_DIMS];
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      int varid_in;
      char file_in[NC_MAX_NAME + 1];
      int ret;

      strcpy(file_in, "");
      if (getenv("srcdir"))
      {
	 strcat(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
      } 
      strcat(file_in, REF_FILE_NAME);

      /* Reopen and check it out again. */
      if (nc_open(file_in, NC_NOWRITE, &ncid)) ERR;

      /* Check it out. */
      ret = nc_inq_dim(ncid, dimid, name_in, &len_in);
      if ((SIZEOF_SIZE_T >= 8 && ret) ||
	  (SIZEOF_SIZE_T < 8 && ret != NC_EDIMSIZE)) ERR;
      if (SIZEOF_SIZE_T < 8)
      {
	 if (len_in != NC_MAX_UINT) ERR;
      }
      else
      {
	 if (len_in != VERY_LONG_LEN) ERR;
      }
      if (strcmp(name_in, LAT_NAME)) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != 1) ERR;
      if (nc_inq_dimid(ncid, LAT_NAME, &varid_in)) ERR;
      if (varid_in != 0) ERR;
      if (nc_inq_unlimdims(ncid, &ndims_in, dimids_in)) ERR;
      if (ndims_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

