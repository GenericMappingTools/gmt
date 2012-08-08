/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_vars3.nc"
#define NDIMS1 1
#define D_SMALL "small_dim"
#define D_SMALL_LEN 16
#define D_MEDIUM "medium_dim"
#define D_MEDIUM_LEN 65546
#define D_LARGE "large_dim"
#define D_LARGE_LEN 1048586
#define V_SMALL "small_var"
#define V_MEDIUM "medium_var"
#define V_LARGE "large_var"
#define D_MAX_ONE_D 16384

int
main(int argc, char **argv)
{

   printf("\n*** Testing netcdf-4 variable functions, some more.\n");
   printf("**** testing definition of coordinate variable after endef/redef...");
   {
#define NX 6
#define NY 36
#define ZD1_NAME "zD1"
#define D2_NAME "D2"

      int ncid, x_dimid, y_dimid, varid2;
      char name_in[NC_MAX_NAME + 1];

      /* Create file with two dims, two 1D vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, ZD1_NAME, NX, &x_dimid)) ERR;
      if (nc_def_dim(ncid, D2_NAME, NY, &y_dimid)) ERR;
      if (nc_enddef(ncid)) ERR;

      /* Go back into define mode and add a coordinate variable. Now
       * dimensions will be out of order. Thanks for confusing my poor
       * library. Why can't you just make up your bloody mind? */
      if (nc_redef(ncid)) ERR;
      if (nc_def_var(ncid, ZD1_NAME, NC_DOUBLE, NDIMS1, &x_dimid, &varid2)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen file and check the name of the first dimension. Even
       * though you've changed the order after doing a redef, you will
       * still expect to get D1_NAME. I sure hope you appreciate how
       * hard you are making life for a poor C library, just trying to
       * do its best in a demanding world. Next time, why don't you
       * try and be a little bit more considerate? Jerk. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, ZD1_NAME)) ERR;
      if (nc_inq_dimname(ncid, 1, name_in)) ERR;
      if (strcmp(name_in, D2_NAME)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing definition of coordinate variable with some data...");
   {
#define NX 6
#define NY 36
#define V1_NAME "V1"
#define D1_NAME "D1"
#define D2_NAME "D2"

      int ncid, x_dimid, y_dimid, varid1, varid2;
      int nvars, ndims, ngatts, unlimdimid, dimids_in[2], natts;
      double data_outx[NX], data_outy[NY];
      int x, y, retval;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;

      /* Create some pretend data. */
      for (x = 0; x < NX; x++)
     	 data_outx[x] = x;
      for (y = 0; y < NY; y++)
     	 data_outy[y] = y;
     
      /* Create file with two dims, two 1D vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D1_NAME, NX, &x_dimid)) ERR;
      if (nc_def_dim(ncid, D2_NAME, NY, &y_dimid)) ERR;
      if (nc_def_var(ncid, V1_NAME, NC_DOUBLE, NDIMS1, &y_dimid, &varid1)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_redef(ncid)) ERR;
      if (nc_def_var(ncid, D1_NAME, NC_DOUBLE, NDIMS1, &x_dimid, &varid2)) ERR;

/*       if (nc_put_var_double(ncid, varid1, &data_outy[0])) ERR; */
/*       if (nc_put_var_double(ncid, varid2, &data_outx[0])) ERR; */
/*       if (nc_sync(ncid)) ERR; */

      /* Check the file. */
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 2 || ndims != 2 || ngatts != 0 || unlimdimid != -1) ERR;
      
      /* Check the dimensions. */
      if (nc_inq_dimids(ncid, &ndims, dimids_in, 1)) ERR;
      if (ndims != 2 || dimids_in[0] != x_dimid || dimids_in[1] != y_dimid) ERR;
      if (nc_inq_dim(ncid, dimids_in[0], name_in, &len_in)) ERR;
      if (strcmp(name_in, D1_NAME) || len_in != NX) ERR;
      if (nc_inq_dim(ncid, dimids_in[1], name_in, &len_in)) ERR;
      if (strcmp(name_in, D2_NAME) || len_in != NY) ERR;

      /* Check the variables. */
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, V1_NAME) || xtype_in != NC_DOUBLE || ndims != 1 ||
	  natts != 0 || dimids_in[0] != y_dimid) ERR;
      if (nc_inq_var(ncid, 1, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, D1_NAME) || xtype_in != NC_DOUBLE || ndims != 1 ||
	  natts != 0 || dimids_in[0] != x_dimid) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Reopen and check the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 2 || ndims != 2 || ngatts != 0 || unlimdimid != -1) ERR;
      
      /* Check the dimensions. */
      if (nc_inq_dimids(ncid, &ndims, dimids_in, 1)) ERR;
      if (ndims != 2 || dimids_in[0] != x_dimid || dimids_in[1] != y_dimid) ERR;
      if (nc_inq_dim(ncid, dimids_in[0], name_in, &len_in)) ERR;
      if (strcmp(name_in, D1_NAME) || len_in != NX) ERR;
      if (nc_inq_dim(ncid, dimids_in[1], name_in, &len_in)) ERR;
      if (strcmp(name_in, D2_NAME) || len_in != NY) ERR;

      /* Check the variables. */
      if (nc_inq_var(ncid, 0, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, V1_NAME) || xtype_in != NC_DOUBLE || ndims != 1 ||
	  natts != 0 || dimids_in[0] != y_dimid) ERR;
      if (nc_inq_var(ncid, 1, name_in, &xtype_in, &ndims, dimids_in, &natts)) ERR;
      if (strcmp(name_in, D1_NAME) || xtype_in != NC_DOUBLE || ndims != 1 ||
	  natts != 0 || dimids_in[0] != x_dimid) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   printf("**** testing endianness of compound type variable...");
   {
#define COMPOUND_NAME "Billy-Bob"
#define BILLY "Billy"
#define BOB "Bob"
#define VAR_NAME1 "Buddy-Joe"
#define NDIMS 2
#define TEXT_LEN 15
      int ncid, nvars_in, varids_in[1], typeid, varid;
      int nvars, ndims, ngatts, unlimdimid;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;
      struct billy_bob
      {
	    int billy;
	    int bob;
      };

      /* Create a netcdf-4 file with scalar compound var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct billy_bob), COMPOUND_NAME, &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, BILLY, NC_COMPOUND_OFFSET(struct billy_bob, billy), NC_INT)) ERR;
      if (nc_insert_compound(ncid, typeid, BOB, NC_COMPOUND_OFFSET(struct billy_bob, bob), NC_INT)) ERR;
      if (nc_def_var(ncid, VAR_NAME1, typeid, 0, NULL, &varid)) ERR;
      if (nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 1 || ndims != 0 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 1 || varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, VAR_NAME1) || xtype_in <= NC_STRING || ndims_in != 0 ||
	  natts_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing that fixed vars with no filter end up being contiguous...");
   {
#define VAR_NAME2 "Yoman_of_the_Guard"
#define NDIMS 2
#define D0_NAME1 "Tower_warders_under_orders"
#define D0_LEN 55
#define D1_NAME1 "When_our_gallent_Norman_Foes"
#define D1_LEN 99
      int ncid, varid;
      int nvars, ndims, ngatts, unlimdimid;
      int dimids[NDIMS], contig;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;

      /* Create a netcdf-4 file with 2D fixed var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D0_NAME1, D0_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, D1_NAME1, D1_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR_NAME2, NC_UINT64, NDIMS, dimids, &varid)) ERR;
      if (nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 1 || ndims != 2 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, VAR_NAME2) || xtype_in != NC_UINT64 || ndims_in != 2 ||
	  natts_in != 0) ERR;
      if (nc_inq_var_chunking(ncid, varid, &contig, NULL)) ERR;
      if (!contig) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing typeless access for classic model...");
   {
#define RANK_P 3
#define LEN 4
      int ncid, dimids[RANK_P], time_id, p_id;
      int ndims, dimids_in[RANK_P];
      
      double data[1] = {3.14159};
      size_t start[1] = {0}, count[1] = {1};
      static float P_data[LEN];
      size_t cor[RANK_P] = {0, 1, 0};
      size_t edg[RANK_P] = {1, 1, LEN};
      int i;

      /* Create a 3D test file. */
      if (nc_create(FILE_NAME, NC_CLASSIC_MODEL|NC_NETCDF4, &ncid)) ERR;

      /* define dimensions */
      if (nc_def_dim(ncid, "Time", NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "X", 4, &dimids[2])) ERR;
      if (nc_def_dim(ncid, "Y", 3, &dimids[1])) ERR;

      /* define variables */
      if (nc_def_var(ncid, "Time", NC_DOUBLE, 1, dimids, &time_id)) ERR;
      if (nc_def_var(ncid, "P", NC_FLOAT, RANK_P, dimids, &p_id)) ERR;
      if (nc_enddef(ncid)) ERR;

      /* Add one record in coordinate variable. */
      if (nc_put_vara(ncid, time_id, start, count, data)) ERR;

      /* The other variable should show an increase in size, since it
       * uses the unlimited dimension. */
      if (nc_inq_var(ncid, 1, NULL, NULL, &ndims, dimids_in, NULL)) ERR;
      if (ndims != 3 || dimids_in[0] != 0 || dimids_in[1] != 2 || dimids_in[2] != 1) ERR;

      /* Read the record of non-existant data. */
      if (nc_get_vara(ncid, 1, cor, edg, P_data)) ERR;
      for (i = 0; i < LEN; i++)
	 if (P_data[i] != NC_FILL_FLOAT) ERR;

      /* That's it! */
      if (nc_close(ncid)) ERR;

      /* Reopen the file and read the second slice. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_var(ncid, 1, NULL, NULL, &ndims, dimids_in, NULL)) ERR;
      if (ndims != 3 || dimids_in[0] != 0 || dimids_in[1] != 2 || dimids_in[2] != 1) ERR;
      if (nc_get_vara(ncid, 1, cor, edg, P_data)) ERR;
      for (i = 0; i < LEN; i++)
	 if (P_data[i] != NC_FILL_FLOAT) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing large number of vars with unlimited dimension...");
   {
#define D0_NAME3 "dim0"
#define NUM_VARS 1000

      int ncid, varid;
      int nvars, ndims, ngatts, unlimdimid;
      int dimid;
      char var_name[NC_MAX_NAME + 1];
      int v;

      /* Create a netcdf-4 file with lots of 1D unlim dim vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D0_NAME3, NC_UNLIMITED, &dimid)) ERR;
      for (v = 0; v < NUM_VARS; v++)
      {
	 sprintf(var_name, "var_%d", v);
	 if (nc_def_var(ncid, var_name, NC_INT, 1, &dimid, &varid)) ERR_RET;
	 if (nc_set_var_chunk_cache(ncid, varid, 0, 0, 0.75)) ERR_RET;
      }
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != 1 || ngatts != 0 || unlimdimid != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
/* #ifdef USE_SZIP */
/*    printf("**** testing that szip works..."); */
/*    { */
/* #define NDIMS1 1 */
/* #define D_SMALL "small_dim" */
/* #define D_SMALL_LEN1 100 */
/* #define D_MEDIUM "medium_dim" */
/* #define D_MEDIUM_LEN1 D_SMALL_LEN1 * 2 */
/* #define D_LARGE "large_dim" */
/* #define D_LARGE_LEN1 D_SMALL_LEN1 * 4 */
/* #define V_SMALL "small_var" */
/* #define V_MEDIUM "medium_var" */
/* #define V_LARGE "large_var" */

/*       int ncid; */
/*       int nvars, ndims, ngatts, unlimdimid; */
/*       int ndims_in, natts_in, dimids_in; */
/*       int small_dimid, medium_dimid, large_dimid; */
/*       int small_varid, medium_varid, large_varid; */
/*       char var_name_in[NC_MAX_NAME + 1]; */
/*       nc_type xtype_in; */
/*       int options_mask_in, bits_per_pixel_in; */
/*       long long small_data[D_SMALL_LEN1], small_data_in[D_SMALL_LEN1]; */
/*       long long medium_data[D_MEDIUM_LEN1], medium_data_in[D_MEDIUM_LEN1]; */
/*       long long large_data[D_LARGE_LEN1], large_data_in[D_LARGE_LEN1]; */
/*       int i; */

/*       for (i = 0; i < D_SMALL_LEN1; i++) */
/* 	 small_data[i] = i; */
/*       for (i = 0; i < D_MEDIUM_LEN1; i++) */
/* 	 medium_data[i] = i; */
/*       for (i = 0; i < D_LARGE_LEN1; i++) */
/* 	 large_data[i] = i; */

/*       /\* Create a netcdf-4 file with three dimensions. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_dim(ncid, D_SMALL, D_SMALL_LEN1, &small_dimid)) ERR; */
/*       if (nc_def_dim(ncid, D_MEDIUM, D_MEDIUM_LEN1, &medium_dimid)) ERR; */
/*       if (nc_def_dim(ncid, D_LARGE, D_LARGE_LEN1, &large_dimid)) ERR; */

/*       /\* Add three vars. Turn on szip for two of them. *\/ */
/*       if (nc_def_var(ncid, V_SMALL, NC_INT64, NDIMS1, &small_dimid, &small_varid)) ERR; */

/*       if (nc_def_var(ncid, V_MEDIUM, NC_INT64, NDIMS1, &medium_dimid, &medium_varid)) ERR; */
/*       if (nc_def_var_szip(ncid, medium_varid, NC_SZIP_EC_OPTION_MASK, 32)) ERR; */

/*       if (nc_def_var(ncid, V_LARGE, NC_INT64, NDIMS1, &large_dimid, &large_varid)) ERR; */
/*       if (nc_def_var_szip(ncid, large_varid, NC_SZIP_NN_OPTION_MASK, 16)) ERR; */

/*       /\* Write data. *\/ */
/*       if (nc_put_var_longlong(ncid, small_varid, small_data)) ERR; */
/*       if (nc_put_var_longlong(ncid, medium_varid, medium_data)) ERR; */
/*       if (nc_put_var_longlong(ncid, large_varid, large_data)) ERR; */

/*       if (nc_close(ncid)) ERR; */

/*       /\* Open the file and check. *\/ */
/*       if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR; */
/*       if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR; */
/*       if (nvars != 3 || ndims != 3 || ngatts != 0 || unlimdimid != -1) ERR; */
/*       if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, &dimids_in, &natts_in)) ERR; */
/*       if (strcmp(var_name_in, V_SMALL) || xtype_in != NC_INT64 || ndims_in != 1 || */
/* 	  natts_in != 0) ERR; */
      
/*       /\* Make sure we have the szip settings we expect. *\/ */
/*       if (nc_inq_var_szip(ncid, small_varid, &options_mask_in, &bits_per_pixel_in)) ERR; */
/*       if (options_mask_in != 0 || bits_per_pixel_in !=0) ERR; */
/*       if (nc_inq_var_szip(ncid, medium_varid, &options_mask_in, &bits_per_pixel_in)) ERR; */
/*       if (!(options_mask_in & NC_SZIP_EC_OPTION_MASK) || bits_per_pixel_in != 32) ERR; */
/*       if (nc_inq_var_szip(ncid, large_varid, &options_mask_in, &bits_per_pixel_in)) ERR; */
/*       if (!(options_mask_in & NC_SZIP_NN_OPTION_MASK) || bits_per_pixel_in != 16) ERR; */

/*       if (nc_close(ncid)) ERR; */
/*    } */
/*    SUMMARIZE_ERR; */
/* #endif */
   FINAL_RESULTS;
}






