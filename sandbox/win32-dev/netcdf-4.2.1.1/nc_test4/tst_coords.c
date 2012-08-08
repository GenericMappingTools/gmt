/* This is part of the netCDF package. Copyright 2009 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test netcdf-4 coordinate variables and dimensions. 

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_coords.nc"

void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

#define INST_NAME "institution"
#define INSTITUTION "NCAR (National Center for Atmospheric \nResearch, Boulder, CO, USA)"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 coordinate variables and dimensions.\n");

   printf("**** testing Jeff Whitakers reported 4.1 bug...");
   {
#define NDIMS 2
#define NLAT 10
#define NLON 20 
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define NVARS 2
#define START_LAT 25.0
#define START_LON -125.0
      {
	 int ncid, lon_dimid, lat_dimid;
	 int lat_varid, lon_varid;
	 float lats[NLAT], lons[NLON];
	 int lat, lon;
	 int nvars, ndims, ngatts, unlimdimid, nvars_in, varids_in[NVARS];
	 int ndims_in, natts_in, dimids_in[NDIMS];
	 char var_name_in[NC_MAX_NAME + 1];
	 nc_type xtype_in;

	 /* Initialize coord data. */
	 for (lat = 0; lat < NLAT; lat++)
	    lats[lat] = START_LAT + 5. * lat;
	 for (lon = 0; lon < NLON; lon++)
	    lons[lon] = START_LON + 5. * lon;
	 
	 /* Create file with two dimensions. */
	 if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
	 if (nc_def_dim(ncid, LAT_NAME, NLAT, &lat_dimid)) ERR;
	 if (nc_def_dim(ncid, LON_NAME, NLON, &lon_dimid)) ERR;

	 if (nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, &lon_dimid, &lon_varid)) ERR;

	 /* Define and write longitude coord variable. */
	 if (nc_put_var_float(ncid, lon_varid, &lons[0])) ERR;

	 /* Define and write latitude coord variable. */
	 if (nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, &lat_dimid, &lat_varid)) ERR;
	 if (nc_put_var_float(ncid, lat_varid, &lats[0])) ERR;

	 if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
	 if (nvars != 2 || ndims != 2 || ngatts != 0 || unlimdimid != -1) ERR;
	 if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
	 if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
	 if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
	 if (strcmp(var_name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 || 
	     dimids_in[0] != 1 || natts_in != 0) ERR;

	 /* Close the file. */
	 if (nc_close(ncid)) ERR;

	 /* Re-open and check the file. */
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

	 if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
	 if (nvars != 2 || ndims != 2 || ngatts != 0 || unlimdimid != -1) ERR;
	 if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
	 if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
	 if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
	 if (strcmp(var_name_in, LON_NAME) || xtype_in != NC_FLOAT || ndims_in != 1 || 
	 dimids_in[0] != 1 || natts_in != 0) ERR;

	 if (nc_close(ncid)) ERR;
      }

   }
   SUMMARIZE_ERR;
   printf("**** testing setting cache values for coordinate variables...");
   {
#define RANK_1 1
#define DIM0_NAME "d0"
#define CACHE_SIZE 1000000
#define CACHE_NELEMS 1009
#define CACHE_PREEMPTION .90

      int ncid, dimid, varid;
      char name_in[NC_MAX_NAME + 1];

      /* Create a test file with one dim and coord variable. */
      if (nc_create(FILE_NAME, NC_CLASSIC_MODEL|NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM0_NAME, NC_UNLIMITED, &dimid)) ERR;
      if (nc_def_var(ncid, DIM0_NAME, NC_DOUBLE, 1, &dimid, &varid)) ERR;
      if (nc_set_var_chunk_cache(ncid, varid, CACHE_SIZE, CACHE_NELEMS, CACHE_PREEMPTION)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_varname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, DIM0_NAME)) ERR;
      if (nc_set_var_chunk_cache(ncid, 0, CACHE_SIZE, CACHE_NELEMS, CACHE_PREEMPTION)) ERR;
      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, DIM0_NAME)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing multi-line strings in attributes...");
   {
      int ncid_classic, ncid_nc4;
      /*int attid;*/
      char att_in_classic[NC_MAX_NAME + 1], att_in_nc4[NC_MAX_NAME + 1];
      
#define FILE_CLASSIC "tst_coords_classic_att.nc"
#define FILE_NC4 "tst_coords_nc4_att.nc"
      /* Create a classic and a netcdf-4 file. */
      if (nc_create(FILE_CLASSIC, 0, &ncid_classic)) ERR;
      if (nc_create(FILE_NC4, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid_nc4)) ERR;

      /* Write an att to both. */
      if (nc_put_att_text(ncid_classic, NC_GLOBAL, INST_NAME, strlen(INSTITUTION) + 1, INSTITUTION)) ERR;
      if (nc_put_att_text(ncid_nc4, NC_GLOBAL, INST_NAME, strlen(INSTITUTION) + 1, INSTITUTION)) ERR;

      /* Close them. */
      if (nc_close(ncid_classic)) ERR;
      if (nc_close(ncid_nc4)) ERR;

      /* Reopen the files. */
      if (nc_open(FILE_CLASSIC, 0, &ncid_classic)) ERR;
      if (nc_open(FILE_NC4, 0, &ncid_nc4)) ERR;

      /* I'll show you mine, if you show me yours... */
      if (nc_get_att_text(ncid_classic, NC_GLOBAL, INST_NAME, att_in_classic)) ERR;
      if (nc_get_att_text(ncid_nc4, NC_GLOBAL, INST_NAME, att_in_nc4)) ERR;
      if (strcmp(att_in_classic, INSTITUTION) || strcmp(att_in_nc4, INSTITUTION) ||
	  strcmp(att_in_classic, att_in_nc4)) ERR;
      if (memcmp(att_in_classic, att_in_nc4, strlen(INSTITUTION) + 1)) ERR;
      
      /* Close them. */
      if (nc_close(ncid_classic)) ERR;
      if (nc_close(ncid_nc4)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing ar-4 example metadata with ncgen-generated code...");
   {
      int  stat;  /* return status */
      int  ncid;  /* netCDF id */

      /* group ids */
      int root_grp;

      /* dimension ids */
      int lon_dim;
      int lat_dim;
      int bnds_dim;
      int time_dim;

      /* dimension lengths */
      size_t lon_len = 256;
      size_t lat_len = 128;
      size_t bnds_len = 2;
      size_t time_len = NC_UNLIMITED;

      /* variable ids */
      int lon_bnds_id;
      int lat_bnds_id;
      int time_bnds_id;
      int time_id;
      int lat_id;
      int lon_id;
      int pr_id;

      /* rank (number of dimensions) for each variable */
#   define RANK_lon_bnds 2
#   define RANK_lat_bnds 2
#   define RANK_time_bnds 2
#   define RANK_time 1
#   define RANK_lat 1
#   define RANK_lon 1
#   define RANK_pr 3

      /* variable shapes */
      int lon_bnds_dims[RANK_lon_bnds];
      int lat_bnds_dims[RANK_lat_bnds];
      int time_bnds_dims[RANK_time_bnds];
      int time_dims[RANK_time];
      int lat_dims[RANK_lat];
      int lon_dims[RANK_lon];
      int pr_dims[RANK_pr];

      /* enter define mode */
      stat = nc_create(FILE_NAME, NC_NETCDF4 | NC_CLASSIC_MODEL, &ncid);
      check_err(stat,__LINE__,__FILE__);
      root_grp = ncid;

      /* define dimensions */
      stat = nc_def_dim(root_grp, "lon", lon_len, &lon_dim);
      check_err(stat,__LINE__,__FILE__);
      stat = nc_def_dim(root_grp, "lat", lat_len, &lat_dim);
      check_err(stat,__LINE__,__FILE__);
      stat = nc_def_dim(root_grp, "bnds", bnds_len, &bnds_dim);
      check_err(stat,__LINE__,__FILE__);
      stat = nc_def_dim(root_grp, "time", time_len, &time_dim);
      check_err(stat,__LINE__,__FILE__);

      /* define variables */

      lon_bnds_dims[0] = lon_dim;
      lon_bnds_dims[1] = bnds_dim;
      stat = nc_def_var(root_grp, "lon_bnds", NC_DOUBLE, RANK_lon_bnds, lon_bnds_dims, &lon_bnds_id);
      check_err(stat,__LINE__,__FILE__);

      lat_bnds_dims[0] = lat_dim;
      lat_bnds_dims[1] = bnds_dim;
      stat = nc_def_var(root_grp, "lat_bnds", NC_DOUBLE, RANK_lat_bnds, lat_bnds_dims, &lat_bnds_id);
      check_err(stat,__LINE__,__FILE__);

      time_bnds_dims[0] = time_dim;
      time_bnds_dims[1] = bnds_dim;
      stat = nc_def_var(root_grp, "time_bnds", NC_DOUBLE, RANK_time_bnds, time_bnds_dims, &time_bnds_id);
      check_err(stat,__LINE__,__FILE__);

      time_dims[0] = time_dim;
      stat = nc_def_var(root_grp, "time", NC_DOUBLE, RANK_time, time_dims, &time_id);
      check_err(stat,__LINE__,__FILE__);

      lat_dims[0] = lat_dim;
      stat = nc_def_var(root_grp, "lat", NC_DOUBLE, RANK_lat, lat_dims, &lat_id);
      check_err(stat,__LINE__,__FILE__);

      lon_dims[0] = lon_dim;
      stat = nc_def_var(root_grp, "lon", NC_DOUBLE, RANK_lon, lon_dims, &lon_id);
      check_err(stat,__LINE__,__FILE__);

      pr_dims[0] = time_dim;
      pr_dims[1] = lat_dim;
      pr_dims[2] = lon_dim;
      stat = nc_def_var(root_grp, "pr", NC_FLOAT, RANK_pr, pr_dims, &pr_id);
      check_err(stat,__LINE__,__FILE__);

      /* assign global attributes */
      { /* table_id */
	 static const char table_id_att[8] = {"Table A1"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "table_id", 8, table_id_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* title */
	 static const char title_att[34] = {"model output prepared for IPCC AR4"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "title", 34, title_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* institution */
	 static const char institution_att[66] = {INSTITUTION} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, INST_NAME, 66, institution_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* source */
	 static const char source_att[153] = {"CCSM3.0, version vector05 (2004): \natmosphere: CAM3.0, T85L26;\nocean     : POP1.4.3 (modified), gx1v3\nsea ice   : CSIM5.0, gx1v3;\nland      : CLM3.0, T85"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "source", 153, source_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* contact */
	 static const char contact_att[13] = {"ccsm@ucar.edu"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "contact", 13, contact_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* project_id */
	 static const char project_id_att[22] = {"IPCC Fourth Assessment"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "project_id", 22, project_id_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* Conventions */
	 static const char Conventions_att[6] = {"CF-1.0"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "Conventions", 6, Conventions_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* references */
	 static const char references_att[137] = {"Collins, W.D., et al., 2005:\n The Community Climate System Model, Version 3\n Journal of Climate\n \n Main website: http://www.ccsm.ucar.edu"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "references", 137, references_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* acknowledgment */
	 static const char acknowledgment_att[847] = {" Any use of CCSM data should acknowledge the contribution\n of the CCSM project and CCSM sponsor agencies with the \n following citation:\n 'This research uses data provided by the Community Climate\n System Model project (www.ccsm.ucar.edu), supported by the\n Directorate for Geosciences of the National Science Foundation\n and the Office of Biological and Environmental Research of\n the U.S. Department of Energy.'\nIn addition, the words 'Community Climate System Model' and\n 'CCSM' should be included as metadata for webpages referencing\n work using CCSM data or as keywords provided to journal or book\npublishers of your manuscripts.\nUsers of CCSM data accept the responsibility of emailing\n citations of publications of research using CCSM data to\n ccsm@ucar.edu.\nAny redistribution of CCSM data must include this data\n acknowledgement statement."} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "acknowledgment", 847, acknowledgment_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* realization */
	 static const int realization_att[1] = {8} ;
	 stat = nc_put_att_int(root_grp, NC_GLOBAL, "realization", NC_INT, 1, realization_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* experiment_id */
	 static const char experiment_id_att[46] = {"climate of the 20th Century experiment (20C3M)"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "experiment_id", 46, experiment_id_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* history */
	 static const char history_att[139] = {"Created from CCSM3 case b30.030g.ES01\n by strandwg@ucar.edu\n on Wed Sep 10 12:22:06 MDT 2008\n \n For all data, added IPCC requested metadata"} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "history", 139, history_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* comment */
	 static const char comment_att[1105] = {"This simulation was initiated from year 460 of \n CCSM3 model run b30.020.ES01 and executed on \n hardware Earth Simulator Center, JAMSTEC. The input external forcings are\nozone forcing    : mozart.o3.128x64_L18_1870-2000_c040515.nc\naerosol optics   : AerosolOptics_c040105.nc\naerosol MMR      : AerosolMass_V_128x256_clim_c031022.nc\ncarbon scaling   : carbonscaling_1870-2000_c040225.nc\nsolar forcing    : scon_lean_1870-2100_c040123.nc\nGHGs             : ghg_1870_2100_c040122.nc\nGHG loss rates   : noaamisc.r8.nc\nvolcanic forcing : VolcanicMass_1870-1999_64x1_L18_c040123.nc\nDMS emissions    : DMS_emissions_128x256_clim_c040122.nc\noxidants         : oxid_128x256_L26_clim_c040112.nc\nSOx emissions    : SOx_emissions_128x256_L2_1850-2000_c040321.nc\n Physical constants used for derived data:\n Lv (latent heat of evaporation): 2.501e6 J kg-1\n Lf (latent heat of fusion     ): 3.337e5 J kg-1\n r[h2o] (density of water      ): 1000 kg m-3\n g2kg   (grams to kilograms    ): 1000 g kg-1\n \n Integrations were performed by NCAR and CRIEPI with support\n and facilities provided by NSF, DOE, MEXT and ESC/JAMSTEC."} ;
	 stat = nc_put_att_text(root_grp, NC_GLOBAL, "comment", 1105, comment_att);
	 check_err(stat,__LINE__,__FILE__);
      }


      /* assign per-variable attributes */
      { /* calendar */
	 static const char time_calendar_att[6] = {"noleap"} ;
	 stat = nc_put_att_text(root_grp, time_id, "calendar", 6, time_calendar_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* standard_name */
	 static const char time_standard_name_att[4] = {"time"} ;
	 stat = nc_put_att_text(root_grp, time_id, "standard_name", 4, time_standard_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* axis */
	 static const char time_axis_att[1] = {"T"} ;
	 stat = nc_put_att_text(root_grp, time_id, "axis", 1, time_axis_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* units */
	 static const char time_units_att[19] = {"days since 0000-1-1"} ;
	 stat = nc_put_att_text(root_grp, time_id, "units", 19, time_units_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* bounds */
	 static const char time_bounds_att[9] = {"time_bnds"} ;
	 stat = nc_put_att_text(root_grp, time_id, "bounds", 9, time_bounds_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* long_name */
	 static const char time_long_name_att[4] = {"time"} ;
	 stat = nc_put_att_text(root_grp, time_id, "long_name", 4, time_long_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* axis */
	 static const char lat_axis_att[1] = {"Y"} ;
	 stat = nc_put_att_text(root_grp, lat_id, "axis", 1, lat_axis_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* standard_name */
	 static const char lat_standard_name_att[8] = {"latitude"} ;
	 stat = nc_put_att_text(root_grp, lat_id, "standard_name", 8, lat_standard_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* bounds */
	 static const char lat_bounds_att[8] = {"lat_bnds"} ;
	 stat = nc_put_att_text(root_grp, lat_id, "bounds", 8, lat_bounds_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* long_name */
	 static const char lat_long_name_att[8] = {"latitude"} ;
	 stat = nc_put_att_text(root_grp, lat_id, "long_name", 8, lat_long_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* units */
	 static const char lat_units_att[13] = {"degrees_north"} ;
	 stat = nc_put_att_text(root_grp, lat_id, "units", 13, lat_units_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* axis */
	 static const char lon_axis_att[1] = {"X"} ;
	 stat = nc_put_att_text(root_grp, lon_id, "axis", 1, lon_axis_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* standard_name */
	 static const char lon_standard_name_att[9] = {"longitude"} ;
	 stat = nc_put_att_text(root_grp, lon_id, "standard_name", 9, lon_standard_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* bounds */
	 static const char lon_bounds_att[8] = {"lon_bnds"} ;
	 stat = nc_put_att_text(root_grp, lon_id, "bounds", 8, lon_bounds_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* long_name */
	 static const char lon_long_name_att[9] = {"longitude"} ;
	 stat = nc_put_att_text(root_grp, lon_id, "long_name", 9, lon_long_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* units */
	 static const char lon_units_att[12] = {"degrees_east"} ;
	 stat = nc_put_att_text(root_grp, lon_id, "units", 12, lon_units_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* comment */
	 static const char pr_comment_att[60] = {"Created using NCL code CCSM_atmm_2cf.ncl on\n machine mineral"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "comment", 60, pr_comment_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* missing_value */
	 static const float pr_missing_value_att[1] = {1e+20} ;
	 stat = nc_put_att_float(root_grp, pr_id, "missing_value", NC_FLOAT, 1, pr_missing_value_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* _FillValue */
	 static const float pr_FillValue_att[1] = {1e+20} ;
	 stat = nc_put_att_float(root_grp, pr_id, "_FillValue", NC_FLOAT, 1, pr_FillValue_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* cell_methods */
	 static const char pr_cell_methods_att[30] = {"time: mean (interval: 1 month)"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "cell_methods", 30, pr_cell_methods_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* history */
	 static const char pr_history_att[20] = {"(PRECC+PRECL)*r[h2o]"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "history", 20, pr_history_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* original_units */
	 static const char pr_original_units_att[7] = {"m-1 s-1"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "original_units", 7, pr_original_units_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* original_name */
	 static const char pr_original_name_att[12] = {"PRECC, PRECL"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "original_name", 12, pr_original_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* standard_name */
	 static const char pr_standard_name_att[18] = {"precipitation_flux"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "standard_name", 18, pr_standard_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* units */
	 static const char pr_units_att[10] = {"kg m-2 s-1"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "units", 10, pr_units_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* long_name */
	 static const char pr_long_name_att[18] = {"precipitation_flux"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "long_name", 18, pr_long_name_att);
	 check_err(stat,__LINE__,__FILE__);
      }
      { /* cell_method */
	 static const char pr_cell_method_att[10] = {"time: mean"} ;
	 stat = nc_put_att_text(root_grp, pr_id, "cell_method", 10, pr_cell_method_att);
	 check_err(stat,__LINE__,__FILE__);
      }

      /* don't initialize variables with fill values */
      stat = nc_set_fill(root_grp, NC_NOFILL, 0);
      check_err(stat,__LINE__,__FILE__);

      /* leave define mode */
      stat = nc_enddef (root_grp);
      check_err(stat,__LINE__,__FILE__);

      stat = nc_close(root_grp);
      check_err(stat,__LINE__,__FILE__);

      {
#define NDIMS4 4
	 int ndims, dimids_in[NDIMS4];
	 char name_in[NC_MAX_NAME + 1];
	 char institution_att_in[NC_MAX_NAME + 1];

	 /* Now open this file and check order of dimensions. */
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

	 /* Check dimids. */
	 if (nc_inq_dimname(ncid, 0, name_in)) ERR;
	 if (strcmp(name_in, "lon")) ERR;
	 if (nc_inq_dimname(ncid, 1, name_in)) ERR;
	 if (strcmp(name_in, "lat")) ERR;
	 if (nc_inq_dimname(ncid, 2, name_in)) ERR;
	 if (strcmp(name_in, "bnds")) ERR;
	 if (nc_inq_dimname(ncid, 3, name_in)) ERR;
	 if (strcmp(name_in, "time")) ERR;

	 /* Check inq_dimids function. */
	 if (nc_inq_dimids(ncid, &ndims, dimids_in, 0)) ERR;
	 if (ndims != NDIMS4 || dimids_in[0] != 0 || dimids_in[1] != 1 ||
	     dimids_in[2] != 2 || dimids_in[3] != 3) ERR;

	 /* Check attribute with line breaks. */
	 if (nc_get_att_text(ncid, NC_GLOBAL, INST_NAME,
			     institution_att_in)) ERR;
	 if (strncmp(institution_att_in, INSTITUTION, strlen(INSTITUTION))) ERR;

	 if (nc_close(ncid)) ERR;
      }
      
   }
   SUMMARIZE_ERR;
   printf("**** testing dim order when coord vars are defined in the wrong order...");
   {
#define RANK_2 2
#define DIM0 "d0"
#define DIM1 "d1"
      int ncid, dimids[RANK_2], varid[RANK_2];
      char name_in[NC_MAX_NAME + 1];

      /* Create a test file. */
      if (nc_create(FILE_NAME, NC_CLASSIC_MODEL|NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions in order. */
      if (nc_def_dim(ncid, DIM0, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM1, 4, &dimids[1])) ERR;

      /* Define coordinate variables in a different order. */
      if (nc_def_var(ncid, DIM1, NC_DOUBLE, 1, &dimids[1], &varid[1])) ERR;
      if (nc_def_var(ncid, DIM0, NC_DOUBLE, 1, &dimids[0], &varid[0])) ERR;

      /* That's it! */
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check dimension order. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, DIM0)) ERR;
      if (nc_inq_dimname(ncid, 1, name_in)) ERR;
      if (strcmp(name_in, DIM1)) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing order of coordinate dims...");
   {
#define RANK_3 3
#define DIM1_NAME "d1"
#define DIM2_NAME "d2"
#define DIM3_NAME "d3"
      int ncid, dimids[RANK_3], varid[RANK_3];
      char name_in[NC_MAX_NAME + 1];

      /* Create a 3D test file. */
      if (nc_create(FILE_NAME, NC_CLASSIC_MODEL|NC_NETCDF4, &ncid)) ERR;

      /* Define dimensions in order. */
      if (nc_def_dim(ncid, DIM1_NAME, NC_UNLIMITED, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, 4, &dimids[1])) ERR;
      if (nc_def_dim(ncid, DIM3_NAME, 3, &dimids[2])) ERR;

      /* Define coordinate variables in a different order. */
      if (nc_def_var(ncid, DIM1_NAME, NC_DOUBLE, 1, &dimids[0], &varid[0])) ERR;
      if (nc_def_var(ncid, DIM2_NAME, NC_DOUBLE, 1, &dimids[1], &varid[1])) ERR;
      if (nc_def_var(ncid, DIM3_NAME, NC_DOUBLE, 1, &dimids[2], &varid[2])) ERR;

      /* That's it! */
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check dimension order. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_dimname(ncid, 0, name_in)) ERR;
      if (strcmp(name_in, DIM1_NAME)) ERR;
      if (nc_inq_dimname(ncid, 1, name_in)) ERR;
      if (strcmp(name_in, DIM2_NAME)) ERR;
      if (nc_inq_dimname(ncid, 2, name_in)) ERR;
      if (strcmp(name_in, DIM3_NAME)) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing coordinate vars...");
   {
#   define RANK_Coordinates_lat 1
      int ncid;
      int Coordinates_grp;
      int lat_dim;
      size_t lat_len = 3;
      int Coordinates_lat_id;

      if(nc_create(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, "Coordinates", &Coordinates_grp)) ERR;

      /* Define dimensions in root group. */
      if (nc_def_dim(ncid, "lat", lat_len, &lat_dim)) ERR;

      /* Define coordinate variable in Coordinates group. */
      if (nc_def_var(Coordinates_grp, "lat", NC_FLOAT,
		     1, &lat_dim, &Coordinates_lat_id)) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing 2D non-coordinate variable...");
   {
#define VAR_NAME "Britany"
#define NDIMS 2
#define TEXT_LEN 15
#define D0_NAME "time"
#define D1_NAME "tl"
      int ncid, nvars_in, varids_in[1];
      int time_dimids[NDIMS], time_id;
      size_t time_count[NDIMS], time_index[NDIMS] = {0, 0};
      const char ttext[TEXT_LEN]="20051224.150000";
      int nvars, ndims, ngatts, unlimdimid;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;

      /* Create a netcdf-4 file with 2D coordinate var. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;

      if (nc_def_dim(ncid, D0_NAME, NC_UNLIMITED, &time_dimids[0])) ERR;
      if (nc_def_dim(ncid, D1_NAME, TEXT_LEN, &time_dimids[1])) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_USHORT, NDIMS, time_dimids,
		     &time_id) != NC_ESTRICTNC3) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_CHAR, NDIMS, time_dimids, &time_id)) ERR;
      if (nc_enddef(ncid)) ERR;

      /* Write one time to the coordinate variable. */
      time_count[0] = 1;
      time_count[1] = TEXT_LEN;
      if (nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 1 || ndims != 2 || ngatts != 0 || unlimdimid != 0) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 1 || varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, VAR_NAME) || xtype_in != NC_CHAR || ndims_in != 2 ||
          dimids_in[0] != 0 || dimids_in[1] != 1 || natts_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing 2D coordinate variable...");
   {
#define NDIMS 2
#define TEXT_LEN 15
#define D0_NAME "time"
#define D1_NAME "tl"
      int ncid, nvars_in, varids_in[1];
      int time_dimids[NDIMS], time_id;
      size_t time_count[NDIMS], time_index[NDIMS] = {0, 0};
      const char ttext[TEXT_LEN + 1]="20051224.150000";
      int nvars, ndims, ngatts, unlimdimid;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;

      /* Create a netcdf-4 file with 2D coordinate var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D0_NAME, NC_UNLIMITED, &time_dimids[0])) ERR;
      if (nc_def_dim(ncid, D1_NAME, TEXT_LEN, &time_dimids[1])) ERR;
      if (nc_def_var(ncid, D0_NAME, NC_CHAR, NDIMS, time_dimids, &time_id)) ERR;

      /* Write one time to the coordinate variable. */
      time_count[0] = 1;
      time_count[1] = TEXT_LEN;
      if (nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 1 || ndims != 2 || ngatts != 0 || unlimdimid != 0) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 1 || varids_in[0] != 0) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, D0_NAME) || xtype_in != NC_CHAR || ndims_in != 2 ||
	dimids_in[0] != 0 || dimids_in[1] != 1 || natts_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing new order of doing things with coordinate variable...");
   {
      /* In this test:
           define a dimension
           define a variable that uses that dimension
           put values in the variable
           define coordinate values for the dimension
      */
#define VAR_NAME_BB "The_Birth_of_Britain"
#define NDIMS_1 1
#define TEXT_LEN 15
#define WINSTON_CHURCHILL "Winston_S_Churchill"
#define D0_LEN 2
#define NUM_VARS_2 2
      int ncid, nvars_in, varids_in[NUM_VARS_2];
      int dimid, varid, varid2;
      int nvars, ndims, ngatts, unlimdimid;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1];
      nc_type xtype_in;
      int data[D0_LEN] = {42, -42};

      /* Create a netcdf-4 file with 2D coordinate var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, WINSTON_CHURCHILL, NC_UNLIMITED, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, NDIMS_1, &dimid, &varid)) ERR;
      if (nc_put_var_int(ncid, varid, data)) ERR;
      if (nc_def_var(ncid, WINSTON_CHURCHILL, NC_INT, NDIMS_1, &dimid, &varid2)) ERR;
      if (nc_put_var_int(ncid, varid2, data)) ERR;

      /* Check things. */
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 2 || ndims != 1 || ngatts != 0 || unlimdimid != 0) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, VAR_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != 2 || ndims != 1 || ngatts != 0 || unlimdimid != 0) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != 2 || varids_in[0] != 0 || varids_in[1] != 1) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, VAR_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing 2D coordinate variable with dimensions defined in different order...");
   {
#define NDIMS 2
#define TEXT_LEN 15
#define D0_NAME "time"
#define D1_NAME "tl"
#define NUM_VARS 2
      int ncid, nvars_in, varids_in[NUM_VARS];
      int time_dimids[NDIMS], time_id, tl_id;
      size_t time_count[NDIMS], time_index[NDIMS] = {0, 0};
      const char ttext[TEXT_LEN + 1]="20051224.150000";
      char ttext_in[TEXT_LEN + 1];
      int nvars, ndims, ngatts, unlimdimid;
      int ndims_in, natts_in, dimids_in[NDIMS];
      char var_name_in[NC_MAX_NAME + 1], dim_name_in[NC_MAX_NAME + 1];
      size_t len_in;
      nc_type xtype_in;

      /* Create a netcdf-4 file with 2D coordinate var. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, D1_NAME, TEXT_LEN, &time_dimids[1])) ERR;
      if (nc_def_dim(ncid, D0_NAME, NC_UNLIMITED, &time_dimids[0])) ERR;
      if (nc_def_var(ncid, D0_NAME, NC_CHAR, NDIMS, time_dimids, &time_id)) ERR;
      if (nc_def_var(ncid, D1_NAME, NC_CHAR, 1, &time_dimids[0], &tl_id)) ERR;

      /* Write one time to the coordinate variable. */
      time_count[0] = 1;
      time_count[1] = TEXT_LEN;
      if (nc_put_vara_text(ncid, time_id, time_index, time_count, ttext)) ERR;

      /* Check the data. */
      if (nc_get_vara_text(ncid, time_id, time_index, time_count, ttext_in)) ERR;
      if (strncmp(ttext, ttext_in, TEXT_LEN)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS || ngatts != 0 || unlimdimid != 1) ERR;
      if (nc_inq_varids(ncid, &nvars_in, varids_in)) ERR;
      if (nvars_in != NUM_VARS || varids_in[0] != 0 || varids_in[1] != 1) ERR;
      if (nc_inq_var(ncid, 0, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(var_name_in, D0_NAME) || xtype_in != NC_CHAR || ndims_in != NDIMS ||
	dimids_in[0] != 1 || dimids_in[1] != 0 || natts_in != 0) ERR;
      if (nc_inq_dimids(ncid, &ndims_in, dimids_in, 0)) ERR;
      if (ndims_in != NDIMS || dimids_in[0] != 0 || dimids_in[1] != 1) ERR;
      if (nc_inq_dim(ncid, 0, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, D1_NAME) || len_in != TEXT_LEN) ERR;
      if (nc_inq_dim(ncid, 1, dim_name_in, &len_in)) ERR;
      if (strcmp(dim_name_in, D0_NAME) || len_in != 1) ERR;

      /* Check the data. */
      if (nc_get_vara_text(ncid, time_id, time_index, time_count, ttext_in)) ERR;
      if (strncmp(ttext, ttext_in, TEXT_LEN)) ERR;

      /* Close up. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}






