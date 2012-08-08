/* This is part of the libcf package. Copyright 2006 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test libcf vertical coordinate variable stuff.

   Ed Hartnett, 11/18/06

   $Id$
*/

#include <config.h>
#include <libcf.h>
#include <netcdf.h>
#include <nc_tests.h>

#define NLAT 3
#define NLON 2
#define NLVL 5
#define NTIME 2
#define NDIM 4   
#define TIME_UNITS "seconds since 1992-10-8"
#define TITLE "This test file is part of libcf automatic testing."
#define HISTORY "Created by tst_vert.c, a libcf test program."

int
main(int argc, char **argv)
{
   char file_name[NC_MAX_NAME + 1];

   printf("\n*** Testing libcf vertical coordinate variables.\n");
   strcpy(file_name, "tst_vert_sigma.nc");
   printf("*** creating file %s...", file_name);
   {
      int ncid, ps_vid, ptop_vid;
      int dimids[NDIM];
      int lat_vid, lon_vid, lvl_vid, time_vid;
      int lat_did, lon_did, lvl_did, time_did;
      int lvl_type_in;
      size_t len_in;
      nc_type xtype_in;
      int dimid_in, varid_in, ps_varid_in, ptop_varid_in;
      char name_in[NC_MAX_NAME + 1];
      char title_in[NC_MAX_NAME + 1], history_in[NC_MAX_NAME + 1];
      size_t title_len, history_len;

      /* Create a file and define four coordinate varibles, lat, lon,
       * sigma level, and time. */
      if (nc_create(file_name, 0, &ncid)) ERR;
      if (nccf_def_file(ncid, TITLE, HISTORY)) ERR;
      if (nccf_def_latitude(ncid, NLAT, NC_FLOAT, &lat_did, &lat_vid)) ERR;
      if (nccf_def_longitude(ncid, NLON, NC_FLOAT, &lon_did, &lon_vid)) ERR;

      /* Now define sigma level coordinage var and dim. */
      if (nccf_def_lvl_vert(ncid, CF_VERT_SIGMA, "sigma", NC_FLOAT, NLVL, 
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

      /* Check the file. */
      if (nccf_inq_file(ncid, &title_len, title_in, &history_len, 
			history_in)) ERR;
      if (title_len != strlen(TITLE) + 1 || 
	  strncmp(title_in, TITLE, strlen(TITLE))) ERR;
      if (nccf_inq_lvl_vert(ncid, name_in, &xtype_in, &len_in,
			    &lvl_type_in, &dimid_in, &varid_in)) ERR;
      if (!strcmp(name_in, "sigma") || xtype_in != NC_FLOAT ||
	  len_in != NLVL || lvl_type_in != CF_VERT_SIGMA ||
	  dimid_in != lvl_did || varid_in != lvl_vid) ERR;
      if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, 
			    &varid_in)) ERR;
      if (len_in != NLAT || xtype_in != NC_FLOAT ||
	  dimid_in != lat_did || varid_in != lat_vid) ERR;
      if (nccf_inq_longitude(ncid, &len_in, &xtype_in, &dimid_in, 
			     &varid_in)) ERR;
      if (len_in != NLON || xtype_in != NC_FLOAT ||
	  dimid_in != lon_did || varid_in != lon_vid) ERR;
      if (nccf_inq_lvl_sigma(ncid, name_in, &xtype_in, &len_in, &ps_varid_in, 
			     &ptop_varid_in, &dimid_in, &varid_in)) ERR;
      if (len_in != NLVL || xtype_in != NC_FLOAT || dimid_in != lvl_did || 
	  varid_in != lvl_vid || ps_varid_in != ps_vid || 
	  ptop_varid_in != ptop_vid) ERR;
      if (nccf_inq_time(ncid, name_in, &len_in, &xtype_in, 
			&dimid_in, &varid_in)) ERR;
      if (len_in != NC_UNLIMITED || xtype_in != NC_FLOAT || dimid_in != time_did || 
	  varid_in != time_vid) ERR;

      if (nc_close(ncid)) ERR;

      if (nc_open(file_name, 0, &ncid)) ERR;

      /* Check the file. */
      if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, 
			    &varid_in)) ERR;
      if (len_in != NLAT || xtype_in != NC_FLOAT ||
	  dimid_in != lat_did || varid_in != lat_vid) ERR;
      if (nccf_inq_longitude(ncid, &len_in, &xtype_in, &dimid_in, 
			     &varid_in)) ERR;
      if (len_in != NLON || xtype_in != NC_FLOAT ||
	  dimid_in != lon_did || varid_in != lon_vid) ERR;
      if (nccf_inq_lvl_sigma(ncid, name_in, &xtype_in, &len_in, &ps_varid_in, 
			     &ptop_varid_in, &dimid_in, &varid_in)) ERR;
      if (len_in != NLVL || xtype_in != NC_FLOAT || dimid_in != lvl_did || 
	  varid_in != lvl_vid || ps_varid_in != ps_vid || 
	  ptop_varid_in != ptop_vid) ERR;
      if (nccf_inq_time(ncid, name_in, &len_in, &xtype_in, 
			&dimid_in, &varid_in)) ERR;
      if (len_in != NC_UNLIMITED || xtype_in != NC_FLOAT || dimid_in != time_did || 
	  varid_in != time_vid) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
/*    strcpy(file_name, "tst_vert_hybrid_sigma.nc"); */
/*    printf("*** creating file %s...", file_name); */
/*    { */
/*       int ncid, ps_vid, p0_vid, a_vid, b_vid; */
/*       int dimids[NDIM]; */
/*       int lat_vid, lon_vid, lvl_vid, time_vid; */
/*       int lat_did, lon_did, lvl_did, time_did; */
/*       size_t len_in; */
/*       nc_type xtype_in; */
/*       int dimid_in, varid_in, ps_varid_in, p0_varid_in; */
/*       int a_varid_in, b_varid_in; */
/*       char name_in[NC_MAX_NAME + 1]; */

/*       /\* Create a file and define four coordinate varibles, lat, lon, */
/*        * hybrid_sigma level, and time. *\/ */
/*       if (nc_create(file_name, 0, &ncid)) ERR; */

/*       if (nccf_def_latitude(ncid, NLAT, NC_FLOAT, &lat_did, &lat_vid)) ERR; */
/*       if (nccf_def_longitude(ncid, NLON, NC_FLOAT, &lon_did, &lon_vid)) ERR; */

/*       /\* Now define hybrid_sigma level coordinage var and dim. *\/ */
/*       if (nccf_def_lvl_hybrid_sigma(ncid, "hybrid_sigma", NC_FLOAT, NLVL,  */
/* 				    &lvl_did, &lvl_vid)) ERR; */

/*       /\* Now define the time coordinate. *\/ */
/*       if (nccf_def_time(ncid, "time", NC_UNLIMITED, NC_FLOAT, TIME_UNITS,  */
/* 			"time", &time_did, &time_vid)) ERR; */

/*       /\* Before we can set up the formula_terms attribute, we need to */
/*        * define some additional variables specific to this type of */
/*        * vertical dimension. *\/ */
/*       dimids[0] = lat_did; */
/*       if (nc_def_var(ncid, "a", NC_FLOAT, 1, dimids, &a_vid)) ERR; */
/*       if (nc_def_var(ncid, "b", NC_FLOAT, 1, dimids, &b_vid)) ERR; */
/*       dimids[0] = lat_did; */
/*       dimids[1] = lon_did; */
/*       if (nc_def_var(ncid, "ps", NC_FLOAT, 2, dimids, &ps_vid)) ERR; */
/*       if (nc_def_var(ncid, "P0", NC_FLOAT, 0, NULL, &p0_vid)) ERR; */

/*       if (nccf_def_ft_hybrid_sigma(ncid, lvl_vid, a_vid, b_vid, ps_vid,  */
/* 				   p0_vid)) ERR; */

/*       /\* Check the file. *\/ */
/*       if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			    &varid_in)) ERR; */
/*       if (len_in != NLAT || xtype_in != NC_FLOAT || */
/* 	  dimid_in != lat_did || varid_in != lat_vid) ERR; */
/*       if (nccf_inq_longitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			     &varid_in)) ERR; */
/*       if (len_in != NLON || xtype_in != NC_FLOAT || */
/* 	  dimid_in != lon_did || varid_in != lon_vid) ERR; */
/*       if (nccf_inq_lvl_hybrid_sigma(ncid, name_in, &xtype_in, &len_in, &a_varid_in, &b_varid_in, &ps_varid_in, */
/* 				    &p0_varid_in, &dimid_in, &varid_in)) ERR; */
/*       if (len_in != NLVL || xtype_in != NC_FLOAT || dimid_in != lvl_did || */
/* 	  varid_in != lvl_vid || ps_varid_in != ps_vid || p0_varid_in != p0_vid || */
/* 	  a_varid_in != a_vid || b_varid_in != b_vid) ERR; */
/*       if (nccf_inq_time(ncid, name_in, &len_in, &xtype_in, */
/* 			&dimid_in, &varid_in)) ERR; */
/*       if (len_in != NC_UNLIMITED || xtype_in != NC_FLOAT || dimid_in != time_did || */
/* 	  varid_in != time_vid) ERR; */

/*       /\* Close and re-open. *\/ */
/*       if (nc_close(ncid)) ERR; */
/*       if (nc_open(file_name, 0, &ncid)) ERR; */

/*       /\* Check the file. *\/ */
/*       if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			    &varid_in)) ERR; */
/*       if (len_in != NLAT || xtype_in != NC_FLOAT || */
/* 	  dimid_in != lat_did || varid_in != lat_vid) ERR; */
/*       if (nccf_inq_longitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			     &varid_in)) ERR; */
/*       if (len_in != NLON || xtype_in != NC_FLOAT || */
/* 	  dimid_in != lon_did || varid_in != lon_vid) ERR; */
/*       if (nccf_inq_lvl_hybrid_sigma(ncid, name_in, &xtype_in, &len_in, &a_varid_in, &b_varid_in, &ps_varid_in, */
/* 				    &p0_varid_in, &dimid_in, &varid_in)) ERR; */
/*       if (len_in != NLVL || xtype_in != NC_FLOAT || dimid_in != lvl_did || */
/* 	  varid_in != lvl_vid || ps_varid_in != ps_vid || p0_varid_in != p0_vid || */
/* 	  a_varid_in != a_vid || b_varid_in != b_vid) ERR; */
/*       if (nccf_inq_time(ncid, name_in, &len_in, &xtype_in, */
/* 			&dimid_in, &varid_in)) ERR; */
/*       if (len_in != NC_UNLIMITED || xtype_in != NC_FLOAT || dimid_in != time_did || */
/* 	  varid_in != time_vid) ERR; */

/*       if (nc_close(ncid)) ERR; */

/*    } */
/*    SUMMARIZE_ERR; */
/*    printf("testing quick creation of hybrid_sigma coordinate system..."); */
/*    { */
/*       int ncid, ps_vid, p0_vid, a_vid, b_vid; */
/*       int dimids[NDIM]; */
/*       int lat_vid, lon_vid, lvl_vid, time_vid; */
/*       int lat_did, lon_did, lvl_did, time_did; */
/*       size_t len_in; */
/*       nc_type xtype_in; */
/*       int dimid_in, varid_in, ps_varid_in, p0_varid_in; */
/*       int a_varid_in, b_varid_in; */
/*       char name_in[NC_MAX_NAME + 1]; */
/*       char file_name[] = {"tst_cvars_hybrid_sigma.nc"}; */

/*       /\* Create a file and define four coordinate varibles, lat, lon, */
/*        * hybrid_sigma level, and time. *\/ */
/*       if (nc_create(file_name, 0, &ncid)) ERR; */

/*       /\* It'd be nice to do this instead... *\/ */
/*       if (nccf_def_hybrid_sigma(names, lens, types, term_names, term_types, */
/* 				term_varids, dimids, varids)) ERR; */

/*       /\* Check the file. *\/ */
/*       if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			    &varid_in)) ERR; */
/*       if (len_in != NLAT || xtype_in != NC_FLOAT || */
/* 	  dimid_in != 0 || varid_in != 0) ERR; */
/*       if (nccf_inq_longitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			     &varid_in)) ERR; */
/*       if (len_in != NLON || xtype_in != NC_FLOAT || */
/* 	  dimid_in != 1 || varid_in != 1) ERR; */
/*       if (nccf_inq_lvl_hybrid_sigma(ncid, name_in, &xtype_in, &len_in, &ps_varid_in, */
/* 			     &p0_varid_in, &dimid_in, &varid_in)) ERR; */
/*       if (len_in != NLVL || xtype_in != NC_FLOAT || dimid_in != 2 || */
/* 	  varid_in != 4 || ps_varid_in != 2 || p0_varid_in != 3) ERR; */
/*       if (nccf_inq_time(ncid, name_in, &len_in, &xtype_in, */
/* 			&dimid_in, &varid_in)) ERR; */
/*       if (len_in != NC_UNLIMITED || xtype_in != NC_FLOAT || dimid_in != 3 || */
/* 	  varid_in != 5) ERR; */

/*       /\* Close and re-open. *\/ */
/*       if (nc_close(ncid)) ERR; */
/*       if (nc_open(file_name, 0, &ncid)) ERR; */

/*       /\* Check the file. *\/ */
/*       if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			    &varid_in)) ERR; */
/*       if (len_in != NLAT || xtype_in != NC_FLOAT || */
/* 	  dimid_in != 0 || varid_in != 0) ERR; */
/*       if (nccf_inq_longitude(ncid, &len_in, &xtype_in, &dimid_in, */
/* 			     &varid_in)) ERR; */
/*       if (len_in != NLON || xtype_in != NC_FLOAT || */
/* 	  dimid_in != 1 || varid_in != 1) ERR; */
/*       if (nccf_inq_lvl_hybrid_sigma(ncid, name_in, &xtype_in, &len_in, &ps_varid_in, */
/* 			     &p0_varid_in, &dimid_in, &varid_in)) ERR; */
/*       if (len_in != NLVL || xtype_in != NC_FLOAT || dimid_in != 2 || */
/* 	  varid_in != 4 || ps_varid_in != 2 || p0_varid_in != 3) ERR; */
/*       if (nccf_inq_time(ncid, name_in, &len_in, &xtype_in, */
/* 			&dimid_in, &varid_in)) ERR; */
/*       if (len_in != NC_UNLIMITED || xtype_in != NC_FLOAT || dimid_in != 3 || */
/* 	  varid_in != 5) ERR; */

/*       if (nc_close(ncid)) ERR; */

/*    } */
/*    SUMMARIZE_ERR; */
   FINAL_RESULTS;
}


