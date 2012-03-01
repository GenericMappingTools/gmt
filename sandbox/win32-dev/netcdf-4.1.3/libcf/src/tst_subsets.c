/* This is part of the libcf package. Copyright 2007 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test libcf variable subsetting stuff.

   Ed Hartnett, 10/1/06

   $Id$
*/

#include <config.h>
#include <libcf.h>
#include <netcdf.h>
#include <nc_tests.h>

#define NLONS_HEMI 180
#define NLATS_HEMI 90
#define NLONS_SIMPLE 1
#define NLATS_SIMPLE 1
#define NDIMS_2 2

int
main(int argc, char **argv)
{
   printf("\n*** Testing libcf geographic subsetting.\n");

#define NLONS 2
#define NLATS 3
#define NLVLS 7
#define NDIMS 4
#define MAX_REC 3
#define TIME_UNITS "seconds since 1992-10-8"
#define TITLE "Libcf Automatic Testing Test File: tst_subset.nc."
#define HISTORY "Created by tst_subsets.c, a libcf test program."
#define FILE_NAME_SIMPLE_SIGMA "tst_subsets_simple_sigma.nc"

   printf("*** testing index size stuff...");
   {
      int ncid;
      int time_vid, lvl_vid, lat_vid, lon_vid;
      int time_did, lvl_did, lat_did, lon_did;
      int l, lat[NLATS], lon[NLONS], dimids[NDIMS];
      int ps_vid, ptop_vid, data_vid, data_vid_in;
      float lat_bounds[] = {40.0, 40.005}, lon_bounds[] = {80.0, 80.005};
      float bad_lat_bounds[] = {10.0, 20.0}, bad_lon_bounds[] = {50.0, 60.005};
      int nlat_in, nlon_in;
      float data[NLVLS][NLATS][NLONS];
      float data_in[NLVLS][NLATS][NLONS];
      size_t start[NDIMS], count[NDIMS];
      int i, j, k;

      /* Create some latitudes. */
      for (lat[0] = 40, l = 1; l < NLATS; l++)
	 lat[l] = lat[l-1] + 2;
      for (lon[0] = 80, l = 1; l < NLONS; l++)
	 lon[l] = lon[l-1] + 2;

      /* Create some phoney data. */
      for (i = 0; i < NLVLS; i++)
	 for (j = 0; j < NLATS; j++)
	    for (k = 0; k < NLONS; k++)
	       data[i][j][k] = i + j + k;

      /* CReate a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME_SIMPLE_SIGMA, 0, &ncid)) ERR;
      if (nccf_def_file(ncid, TITLE, HISTORY)) ERR;
      if (nccf_def_latitude(ncid, NLATS, NC_INT, &lat_did, 
			    &lat_vid)) ERR;
      if (nccf_def_longitude(ncid, NLONS, NC_INT, &lon_did, 
			     &lon_vid)) ERR;

      /* Now define sigma level coordinage var and dim. */
      if (nccf_def_lvl_sigma(ncid, "sigma", NC_FLOAT, NLVLS, 
			     &lvl_did, &lvl_vid)) ERR;

      /* Now define the time coordinate. */
      if (nccf_def_time(ncid, "time", NC_UNLIMITED, NC_FLOAT, TIME_UNITS, 
			"time", &time_did, &time_vid)) ERR;

      /* Before we can set the sigma level formula terms, we
       * need to define two variables, the surface pressure and PTOP
       * (a scalar).*/
      dimids[0] = lat_did;
      dimids[1] = lon_did;
      if (nc_def_var(ncid, "ps", NC_FLOAT, 2, dimids, &ps_vid)) ERR;
      if (nc_def_var(ncid, "PTOP", NC_FLOAT, 0, NULL, &ptop_vid)) ERR;

      /* Save the ps and ptop info in the formula terms attribute. */
      if (nccf_def_ft_sigma(ncid, lvl_vid, ps_vid, ptop_vid)) ERR;

      /* Now create a data variable. */
      dimids[0] = time_did;
      dimids[1] = lvl_did;
      dimids[2] = lat_did;
      dimids[3] = lon_did;
      if (nc_def_var(ncid, "data", NC_FLOAT, NDIMS, dimids, &data_vid)) ERR;

      /* End define mode. */
      if (nc_enddef(ncid)) ERR;

      /* Write lats and lons. */
      if (nc_put_var_int(ncid, lat_vid, lat)) ERR;      
      if (nc_put_var_int(ncid, lon_vid, lon)) ERR;      

      /* Now write some data, one record at a time. */
      start[1] = start[2] = start[3] = 0;
      count[0] = 1;
      count[1] = NLVLS;
      count[2] = NLATS;
      count[3] = NLONS;
      for (start[0] = 0; start[0] < MAX_REC; start[0]++)
	 if (nc_put_vara(ncid, data_vid, start, count, &data)) ERR;
      
      if (nc_close(ncid)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME_SIMPLE_SIGMA, 0, &ncid)) ERR;
      if (nc_inq_varid(ncid, "data", &data_vid_in)) ERR;
      if (data_vid_in != data_vid) ERR;
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS || nlon_in != NLONS) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_bounds, &nlat_in, bad_lon_bounds, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, lat_bounds, &nlat_in, lon_bounds, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != 1) ERR;
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing simple subsetting case, with 2D data around CONUS...");
#define NLONS_W 180
#define NLATS_N 90
#define NLONS_SIMPLE 1
#define NLATS_SIMPLE 1
#define NDIMS_2 2
#define DATA_VAR_NAME_SIMPLE "Simon"
#define NLONS_CONUS 56 
#define NLATS_CONUS 15 
#define FILE_NAME_NW "tst_subsets_nw.nc"
   {
      int ncid, lat_vid, lon_vid, lat_did, lon_did, data_vid;
      int lat[NLATS_N], lon[NLONS_W], dimids[NDIMS_2];
      float simple_lat_range[] = {30.0, 30.5}, simple_lon_range[] = {-1.0, -0.5};
      float simple_lat_range2[] = {50.0, 50.5}, simple_lon_range2[] = {-179.0, -178.5};
      float bad_lat_range[] = {-40.0, -30.5}, bad_lon_range[] = {1.0, 2.5};
      float conus_lat_range[] = {30.0, 44.5}, conus_lon_range[] = {-130.0, -74.5}; 
      int conus_lat_index[] = {30, 44}, conus_lon_index[] = {75, 130}; 
      int nlat_in, nlon_in;
      float data[NLATS_N][NLONS_W];
      float data_in[NLATS_SIMPLE][NLONS_SIMPLE];
      float data_conus[NLATS_CONUS][NLONS_CONUS];
      int l, j, k;

      /* Create some latitudes for the Western Hem. */
      for (lat[0] = 0, l = 1; l < NLATS_N; l++)
	 lat[l] = lat[l-1] + 1;
      for (lon[0] = 0, l = 1; l < NLONS_W; l++)
	 lon[l] = lon[l-1] - 1;

      /* Create some phoney data. */
      for (j = 0; j < NLATS_N; j++)
	 for (k = 0; k < NLONS_W; k++)
	    data[j][k] = (j + k) * 2.5;

      /* Create a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME_NW, 0, &ncid)) ERR;
      if (nccf_def_file(ncid, TITLE, HISTORY)) ERR;
      if (nccf_def_latitude(ncid, NLATS_N, NC_INT, &lat_did, 
			    &lat_vid)) ERR;
      if (nccf_def_longitude(ncid, NLONS_W, NC_INT, &lon_did, 
			     &lon_vid)) ERR;

      /* Now create a data variable. */
      dimids[0] = lat_did;
      dimids[1] = lon_did;
      if (nc_def_var(ncid, DATA_VAR_NAME_SIMPLE, NC_FLOAT, NDIMS_2, dimids, 
		     &data_vid)) ERR;

      /* End define mode. */
      if (nc_enddef(ncid)) ERR;

      /* Write lats and lons. */
      if (nc_put_var_int(ncid, lat_vid, lat)) ERR;      
      if (nc_put_var_int(ncid, lon_vid, lon)) ERR;      

      /* Now write some 2D data. */
      if (nc_put_var_float(ncid, data_vid, &data[0][0])) ERR;
      
      if (nc_close(ncid)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME_NW, 0, &ncid)) ERR;
      if (nc_inq_varid(ncid, DATA_VAR_NAME_SIMPLE, &data_vid)) ERR;

      /* All lons, one lat. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != NLONS_W) ERR;

      /* All lats, one lon. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, simple_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_N || nlon_in != 1) ERR;

      /* These should all yeild no subset. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, bad_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 180) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, bad_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, simple_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 1) ERR;

      /* No ranges, so get the entire thing. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_N || nlon_in != NLONS_W) ERR;

      /* Read a small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, simple_lon_range, 
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[(int)simple_lat_range[0]][abs((int)simple_lon_range[0])])
	 ERR;

      /* Read a different small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range2, &nlat_in, simple_lon_range2, 
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[(int)simple_lat_range2[0]][abs((int)simple_lon_range2[0])])
	 ERR;

      /* Read a big subset (approximate CONUS). */
      if (nccf_get_vara(ncid, data_vid, conus_lat_range, &nlat_in, conus_lon_range, 
			&nlon_in, 0, 0, data_conus)) ERR;
      if (nlat_in != NLATS_CONUS || nlon_in != NLONS_CONUS) ERR;
      for (j = 0; j < conus_lat_index[1] - conus_lat_index[0]; j++)
	 for (k = 0; k < conus_lat_index[1] - conus_lat_index[0]; k++)
	    if (data_conus[j][k] != 
		data[j + conus_lat_index[0]][k + conus_lon_index[0]]) ERR;
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing simple subsetting case, with 2D data around South America...");
#define NLONS_W 180
#define NLATS_S 90
#define NLONS_SIMPLE 1
#define NLATS_SIMPLE 1
#define NDIMS_2 2
#define NLONS_SA 50
#define NLATS_SA 81 
#define FILE_NAME_SW "tst_subsets_sw.nc"
   {
      int ncid, lat_vid, lon_vid, lat_did, lon_did, data_vid;
      int lat[NLATS_S], lon[NLONS_W], dimids[NDIMS_2];
      float simple_lat_range[] = {-30.0, -29.5}, simple_lon_range[] = {-1.0, -0.5};
      float simple_lat_range2[] = {-50.0, -49.5}, simple_lon_range2[] = {-179.0, -178.5};
      float bad_lat_range[] = {20.0, 30.5}, bad_lon_range[] = {1.0, 2.5};
      float sa_lat_range[] = {-80.0, 0}, sa_lon_range[] = {-80.0, -30.5}; 
      int sa_lat_index[] = {0, 81}, sa_lon_index[] = {31, 81}; 
      int nlat_in, nlon_in;
      float data[NLATS_S][NLONS_W];
      float data_in[NLATS_SIMPLE][NLONS_SIMPLE];
      float data_sa[NLATS_SA][NLONS_SA];
      int l, j, k;

      /* Create some latitudes for the South Western quarter of the
       * Earth. */
      for (lat[0] = 0, l = 1; l < NLATS_S; l++)
	 lat[l] = lat[l-1] - 1;
      for (lon[0] = 0, l = 1; l < NLONS_W; l++)
	 lon[l] = lon[l-1] - 1;

      /* Create some phoney data. */
      for (j = 0; j < NLATS_S; j++)
	 for (k = 0; k < NLONS_W; k++)
	    data[j][k] = (j + k) * -10.5;

      /* Create a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME_SW, 0, &ncid)) ERR;
      if (nccf_def_file(ncid, TITLE, HISTORY)) ERR;
      if (nccf_def_latitude(ncid, NLATS_S, NC_INT, &lat_did, 
			    &lat_vid)) ERR;
      if (nccf_def_longitude(ncid, NLONS_W, NC_INT, &lon_did, 
			     &lon_vid)) ERR;

      /* Now create a data variable. */
      dimids[0] = lat_did;
      dimids[1] = lon_did;
      if (nc_def_var(ncid, DATA_VAR_NAME_SIMPLE, NC_FLOAT, NDIMS_2, dimids, 
		     &data_vid)) ERR;

      /* End define mode. */
      if (nc_enddef(ncid)) ERR;

      /* Write lats and lons. */
      if (nc_put_var_int(ncid, lat_vid, lat)) ERR;      
      if (nc_put_var_int(ncid, lon_vid, lon)) ERR;      

      /* Now write some 2D data. */
      if (nc_put_var_float(ncid, data_vid, &data[0][0])) ERR;
      
      if (nc_close(ncid)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME_SW, 0, &ncid)) ERR;
      if (nc_inq_varid(ncid, DATA_VAR_NAME_SIMPLE, &data_vid)) ERR;

      /* All lons, one lat. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != NLONS_W) ERR;

      /* All lats, one lon. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, simple_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_S || nlon_in != 1) ERR;

      /* These should all yeild no subset. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, bad_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 180) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, bad_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, simple_lon_range, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 1) ERR;

      /* No ranges, so get the entire thing. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, NULL, &nlon_in, 
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_S || nlon_in != NLONS_W) ERR;

      /* Read a small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, simple_lon_range, 
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[abs((int)simple_lat_range[0])][abs((int)simple_lon_range[0])])
	 ERR;

      /* Read a different small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range2, &nlat_in, simple_lon_range2, 
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[abs((int)simple_lat_range2[0])][abs((int)simple_lon_range2[0])])
	 ERR;

      /* Read a big subset (approximate SA). */
      if (nccf_get_vara(ncid, data_vid, sa_lat_range, &nlat_in, sa_lon_range, 
			&nlon_in, 0, 0, data_sa)) ERR;
      if (nlat_in != NLATS_SA || nlon_in != NLONS_SA) ERR;
      for (j = 0; j < NLATS_SA; j++)
	 for (k = 0; k < NLONS_SA; k++)
	    if (data_sa[j][k] != 
		data[j + sa_lat_index[0]][k + sa_lon_index[0]]) ERR;
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing simple subsetting case, with 2D data around Australia...");
#define NLONS_HEMI 180
#define NLATS_HEMI 90
#define NLONS_SIMPLE 1
#define NLATS_SIMPLE 1
#define NDIMS_2 2
#define NLATS_AUS 36
#define NLONS_AUS 46
#define FILE_NAME_SE "tst_subsets_se.nc"
   {
      int ncid, lat_vid, lon_vid, lat_did, lon_did, data_vid;
      int lat[NLATS_HEMI], lon[NLONS_HEMI], dimids[NDIMS_2];
      float simple_lat_range[] = {-30.0, -29.5}, simple_lon_range[] = {1.0, 1.5};
      float simple_lat_range2[] = {-50.0, -49.5}, simple_lon_range2[] = {178.0, 178.5};
      float bad_lat_range[] = {20.0, 30.5}, bad_lon_range[] = {-10.0, -2.5};
      float aus_lat_range[] = {-45.0, -10.0}, aus_lon_range[] = {110.0, 155.0};
      int aus_lat_index[] = {10, 35}, aus_lon_index[] = {110, 45};
      int nlat_in, nlon_in;
      float data[NLATS_HEMI][NLONS_HEMI];
      float data_in[NLATS_SIMPLE][NLONS_SIMPLE];
      float data_aus[NLATS_AUS][NLONS_AUS];
      int l, j, k;

      /* Create some latitudes for the South Eastern quarter of the
       * Earth. */
      for (lat[0] = 0, l = 1; l < NLATS_HEMI; l++)
	 lat[l] = lat[l-1] - 1;
      for (lon[0] = 0, l = 1; l < NLONS_HEMI; l++)
	 lon[l] = lon[l-1] + 1;

      /* Create some phoney data. */
      for (j = 0; j < NLATS_HEMI; j++)
	 for (k = 0; k < NLONS_HEMI; k++)
	    data[j][k] = (j + k) * -10.5;

      /* Create a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME_SE, 0, &ncid)) ERR;
      if (nccf_def_file(ncid, TITLE, HISTORY)) ERR;
      if (nccf_def_latitude(ncid, NLATS_HEMI, NC_INT, &lat_did,
			    &lat_vid)) ERR;
      if (nccf_def_longitude(ncid, NLONS_HEMI, NC_INT, &lon_did,
			     &lon_vid)) ERR;

      /* Now create a data variable. */
      dimids[0] = lat_did;
      dimids[1] = lon_did;
      if (nc_def_var(ncid, DATA_VAR_NAME_SIMPLE, NC_FLOAT, NDIMS_2, dimids,
		     &data_vid)) ERR;

      /* End define mode. */
      if (nc_enddef(ncid)) ERR;

      /* Write lats and lons. */
      if (nc_put_var_int(ncid, lat_vid, lat)) ERR;
      if (nc_put_var_int(ncid, lon_vid, lon)) ERR;

      /* Now write some 2D data. */
      if (nc_put_var_float(ncid, data_vid, &data[0][0])) ERR;
      
      if (nc_close(ncid)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME_SE, 0, &ncid)) ERR;
      if (nc_inq_varid(ncid, DATA_VAR_NAME_SIMPLE, &data_vid)) ERR;

      /* All lons, one lat. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, NULL, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != NLONS_HEMI) ERR;

      /* All lats, one lon. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, simple_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_HEMI || nlon_in != 1) ERR;

      /* These should all yeild no subset. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, bad_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, NULL, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 180) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, bad_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, simple_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 1) ERR;

      /* No ranges, so get the entire thing. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, NULL, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_HEMI || nlon_in != NLONS_HEMI) ERR;

      /* Read a small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, simple_lon_range,
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[abs((int)simple_lat_range[0])][abs((int)simple_lon_range[0])])
	 ERR;

      /* Read a different small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range2, &nlat_in, simple_lon_range2,
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[abs((int)simple_lat_range2[0])][abs((int)simple_lon_range2[0])])
	 ERR;

      /* Read a big subset (approximatly Australia). */
      if (nccf_get_vara(ncid, data_vid, aus_lat_range, &nlat_in, aus_lon_range,
			&nlon_in, 0, 0, data_aus)) ERR;
      if (nlat_in != NLATS_AUS || nlon_in != NLONS_AUS) ERR;
      for (j = 0; j < NLATS_AUS; j++)
	 for (k = 0; k < NLONS_AUS; k++)
	    if (data_aus[j][k] !=
		data[j + aus_lat_index[0]][k + aus_lon_index[0]]) 
	    {
	       ERR;
	       break;
	    }
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple subsetting case, with 2D data around Japan...");
#define NLATS_JAPAN 16
#define NLONS_JAPAN 21
#define FILE_NAME_NE "tst_subsets_ne.nc"
   {
      int ncid, lat_vid, lon_vid, lat_did, lon_did, data_vid;
      int lat[NLATS_HEMI], lon[NLONS_HEMI], dimids[NDIMS_2];
      float simple_lat_range[] = {30.0, 30.5}, simple_lon_range[] = {1.0, 1.5};
      float simple_lat_range2[] = {50.0, 50.5}, simple_lon_range2[] = {178.0, 178.5};
      float bad_lat_range[] = {-20.0, -10.5}, bad_lon_range[] = {-10.0, -2.5};
      float japan_lat_range[] = {30.0, 45.0}, japan_lon_range[] = {130.0, 150.0};
      int japan_lat_index[] = {30, 45}, japan_lon_index[] = {130, 150};
      int nlat_in, nlon_in;
      float data[NLATS_HEMI][NLONS_HEMI];
      float data_in[NLATS_SIMPLE][NLONS_SIMPLE];
      float data_aus[NLATS_JAPAN][NLONS_JAPAN];
      int l, j, k;

      /* Create some latitudes for the South Eastern quarter of the
       * Earth. */
      for (lat[0] = 0, l = 1; l < NLATS_HEMI; l++)
	 lat[l] = lat[l-1] + 1;
      for (lon[0] = 0, l = 1; l < NLONS_HEMI; l++)
	 lon[l] = lon[l-1] + 1;

      /* Create some phoney data. */
      for (j = 0; j < NLATS_HEMI; j++)
	 for (k = 0; k < NLONS_HEMI; k++)
	    data[j][k] = (j + k) * -10.5;

      /* Create a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME_NE, 0, &ncid)) ERR;
      if (nccf_def_file(ncid, TITLE, HISTORY)) ERR;
      if (nccf_def_latitude(ncid, NLATS_HEMI, NC_INT, &lat_did,
			    &lat_vid)) ERR;
      if (nccf_def_longitude(ncid, NLONS_HEMI, NC_INT, &lon_did,
			     &lon_vid)) ERR;

      /* Now create a data variable. */
      dimids[0] = lat_did;
      dimids[1] = lon_did;
      if (nc_def_var(ncid, DATA_VAR_NAME_SIMPLE, NC_FLOAT, NDIMS_2, dimids,
		     &data_vid)) ERR;

      /* End define mode. */
      if (nc_enddef(ncid)) ERR;

      /* Write lats and lons. */
      if (nc_put_var_int(ncid, lat_vid, lat)) ERR;
      if (nc_put_var_int(ncid, lon_vid, lon)) ERR;

      /* Now write some 2D data. */
      if (nc_put_var_float(ncid, data_vid, &data[0][0])) ERR;
      
      if (nc_close(ncid)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME_NE, 0, &ncid)) ERR;
      if (nc_inq_varid(ncid, DATA_VAR_NAME_SIMPLE, &data_vid)) ERR;

      /* All lons, one lat. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, NULL, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != NLONS_HEMI) ERR;

      /* All lats, one lon. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, simple_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_HEMI || nlon_in != 1) ERR;

      /* These should all yeild no subset. */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, bad_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 1 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, NULL, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 180) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, bad_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 0) ERR;
      if (nccf_get_vara(ncid, data_vid, bad_lat_range, &nlat_in, simple_lon_range, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != 0 || nlon_in != 1) ERR;

      /* No ranges, so get the entire thing. */
      if (nccf_get_vara(ncid, data_vid, NULL, &nlat_in, NULL, &nlon_in,
			0, 0, data_in)) ERR;
      if (nlat_in != NLATS_HEMI || nlon_in != NLONS_HEMI) ERR;

      /* Read a small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range, &nlat_in, simple_lon_range,
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[abs((int)simple_lat_range[0])][abs((int)simple_lon_range[0])])
	 ERR;

      /* Read a different small subset (1 value). */
      if (nccf_get_vara(ncid, data_vid, simple_lat_range2, &nlat_in, simple_lon_range2,
			&nlon_in, 0, 0, data_in)) ERR;
      if (nlat_in != NLATS_SIMPLE || nlon_in != NLONS_SIMPLE) ERR;
      if (data_in[0][0] != data[abs((int)simple_lat_range2[0])][abs((int)simple_lon_range2[0])])
	 ERR;

      /* Read a big subset (approximatly Australia). */
      if (nccf_get_vara(ncid, data_vid, japan_lat_range, &nlat_in, japan_lon_range,
			&nlon_in, 0, 0, data_aus)) ERR;
      if (nlat_in != NLATS_JAPAN || nlon_in != NLONS_JAPAN) ERR;
      for (j = 0; j < NLATS_JAPAN; j++)
	 for (k = 0; k < NLONS_JAPAN; k++)
	    if (data_aus[j][k] !=
		data[j + japan_lat_index[0]][k + japan_lon_index[0]]) 
	    {
	       ERR;
	       break;
	    }
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


