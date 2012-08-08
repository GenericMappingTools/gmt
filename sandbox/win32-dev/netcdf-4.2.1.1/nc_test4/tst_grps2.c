/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 group code some more.

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_grps.nc"
#define DIM1_NAME "kingdom"
#define DIM1_LEN 3
#define DIM2_NAME "year"
#define DIM2_LEN 5
#define VAR1_NAME "Number_of_Beheadings_in_Family"
#define DYNASTY "Tudor"
#define HENRY_VII "Henry_VII"
#define MARGARET "Margaret"
#define JAMES_V_OF_SCOTLAND "James_V_of_Scotland"
#define MARY_I_OF_SCOTLAND "Mary_I_of_Scotland"
#define JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND "James_VI_of_Scotland_and_I_of_England"
#define MAX_SIBLING_GROUPS 10
#define NUM_CASTLES_NAME "Number_of_Castles"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 group functions.\n");
   printf("*** testing use of unlimited dim in parent group...");
   {
#define NDIMS_IN_VAR 1
#define NDIMS_IN_FILE 2
#define BABE_LIMIT 3
#define DIM_NAME1 "Influence"
#define DIM_NAME2 "Babe_Factor"
#define VAR_NAME1 "Court_of_Star_Chamber"
#define VAR_NAME2 "Justice_of_the_Peace"
#define VAR_NAME3 "Bosworth_Field"
      int ncid, dimid1, dimid2, varid1, varid2, varid3, henry_vii_id;
      int grpid_in, varid_in1, varid_in2, varid_in3;
      nc_type xtype_in;
      int ndims_in, dimids_in[NDIMS_IN_FILE], dimid1_in, natts;
      char name_in[NC_MAX_NAME + 1];
      size_t len_in, index[NDIMS_IN_VAR] = {0};
      long long value = NC_FILL_INT64 + 1, value_in;

      /* Create a file with an unlimited dim and a limited, used by
       * variables in child groups. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME1, NC_UNLIMITED, &dimid1)) ERR;
      if (nc_def_dim(ncid, DIM_NAME2, BABE_LIMIT, &dimid2)) ERR;
      if (nc_def_grp(ncid, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_var(henry_vii_id, VAR_NAME1, NC_INT64, NDIMS_IN_VAR, &dimid1, &varid1)) ERR;
      if (nc_def_var(henry_vii_id, VAR_NAME2, NC_INT64, NDIMS_IN_VAR, &dimid1, &varid2)) ERR;
      if (nc_def_var(henry_vii_id, VAR_NAME3, NC_INT64, NDIMS_IN_VAR, &dimid2, &varid3)) ERR;
      
      /* Check it out. Find the group by name. */
      if (nc_inq_ncid(ncid, HENRY_VII, &grpid_in)) ERR;

      /* Ensure that dimensions in parent are visible and correct. */
      if (nc_inq_dimids(grpid_in, &ndims_in, dimids_in, 1)) ERR;
      if (ndims_in != NDIMS_IN_FILE || dimids_in[0] != dimid1 || dimids_in[1] != dimid2) ERR;
      if (nc_inq_dim(grpid_in, dimids_in[0], name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM_NAME1) || len_in != 0) ERR;
      if (nc_inq_dim(grpid_in, dimids_in[1], name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM_NAME2) || len_in != BABE_LIMIT) ERR;

      /* Check the vars in the group. */
      if (nc_inq_varid(grpid_in, VAR_NAME1, &varid_in1)) ERR;
      if (nc_inq_varid(grpid_in, VAR_NAME2, &varid_in2)) ERR;
      if (nc_inq_varid(grpid_in, VAR_NAME3, &varid_in3)) ERR;
      if (varid_in1 != varid1 || varid_in2 != varid2 || varid_in3 != varid3) ERR;
      if (nc_inq_var(grpid_in, varid1, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME1) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR ||
          dimid1_in != dimid1 || natts != 0) ERR;
      if (nc_inq_var(grpid_in, varid2, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME2) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR ||
          dimid1_in != dimid1 || natts != 0) ERR;
      if (nc_inq_var(grpid_in, varid3, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME3) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR ||
          dimid1_in != dimid2 || natts != 0) ERR;

      /* Write one value to one variable. */
      if (nc_put_var1_longlong(grpid_in, varid_in1, index, &value)) ERR;

      /* Read one value from the second unlim dim variable. It should
       * be the fill value. */
      if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in)) ERR;
      if (value_in != NC_FILL_INT64) ERR;

      /* Read one value from the variable with limited dim. It should
       * be the fill value. */
      if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in)) ERR;
      if (value_in != NC_FILL_INT64) ERR;

      /* Attempt to read beyond end of dimensions to generate error. */
      index[0] = BABE_LIMIT;
      if (nc_get_var1_longlong(grpid_in, varid_in1, index, &value_in) != NC_EINVALCOORDS) ERR;
      if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in) != NC_EINVALCOORDS) ERR;
      if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in) != NC_EINVALCOORDS) ERR;

      if (nc_close(ncid)) ERR;

      /* Check it out again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Find the group by name. */
      if (nc_inq_ncid(ncid, HENRY_VII, &grpid_in)) ERR;

      /* Ensure that dimensions in parent are visible and correct. */
      if (nc_inq_dimids(grpid_in, &ndims_in, dimids_in, 1)) ERR;
      if (ndims_in != NDIMS_IN_FILE || dimids_in[0] != dimid1 || dimids_in[1] != dimid2) ERR;
      if (nc_inq_dim(grpid_in, dimids_in[0], name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM_NAME1) || len_in != 1) ERR;
      if (nc_inq_dim(grpid_in, dimids_in[1], name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM_NAME2) || len_in != BABE_LIMIT) ERR;

      /* Check the vars in the group. */
      if (nc_inq_varid(grpid_in, VAR_NAME1, &varid_in1)) ERR;
      if (nc_inq_varid(grpid_in, VAR_NAME2, &varid_in2)) ERR;
      if (nc_inq_varid(grpid_in, VAR_NAME3, &varid_in3)) ERR;
      if (varid_in1 != varid1 || varid_in2 != varid2 || varid_in3 != varid3) ERR;
      if (nc_inq_var(grpid_in, varid1, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME1) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR ||
          dimid1_in != dimid1 || natts != 0) ERR;
      if (nc_inq_var(grpid_in, varid2, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME2) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR ||
          dimid1_in != dimid1 || natts != 0) ERR;
      if (nc_inq_var(grpid_in, varid3, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME3) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR ||
          dimid1_in != dimid2 || natts != 0) ERR;

      /* Read one value from the second unlim dim variable. It should
       * be the fill value. */
      index[0] = 0;
      if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in)) ERR;
      if (value_in != NC_FILL_INT64) ERR;

      /* Read one value from the variable with limited dim. It should
       * be the fill value. */
      if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in)) ERR;
      if (value_in != NC_FILL_INT64) ERR;

      /* Attempt to read beyond end of dimensions to generate error. */
      index[0] = BABE_LIMIT;
      if (nc_get_var1_longlong(grpid_in, varid_in1, index, &value_in) != NC_EINVALCOORDS) ERR;
      if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in) != NC_EINVALCOORDS) ERR;
      if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in) != NC_EINVALCOORDS) ERR;

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing groups and unlimited dimensions...");
   {
      int ncid;
      int henry_vii_id;
      int tudor_id;
      int dimids_in[MAX_SIBLING_GROUPS], ndims_in;
      int num_grps;
      int dimid, dynasty, varid;
      size_t len_in;
      int natts_in;
      int grpids_in[10];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data_out[DIM1_LEN] = {0, 2, 6}, data_in[DIM1_LEN];
      size_t start[1] = {0}, count[1] = {3};
      int j;

      /* Create one group, with one var, which has one dimension, which is unlimited. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;
      if (nc_def_dim(tudor_id, DIM1_NAME, NC_UNLIMITED, &dimid)) ERR;
      if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_var(henry_vii_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_vara_int(henry_vii_id, varid, start, count, data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Now check the file to see if the dimension and variable are
       * there. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(dynasty, &num_grps, grpids_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_dim(grpids_in[0], 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR;
      if (nc_inq_var(grpids_in[0], 0, name_in, &xtype_in, &ndims_in, dimids_in,
		     &natts_in)) ERR;
      if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;
      if (nc_get_vara_int(grpids_in[0], 0, start, count, data_in)) ERR;
      for (j=0; j<DIM1_LEN; j++)
	 if (data_in[j] != data_out[j]) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

