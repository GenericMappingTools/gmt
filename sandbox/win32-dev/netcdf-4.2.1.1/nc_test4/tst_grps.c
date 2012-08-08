/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 group code. 
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
   printf("*** testing simple group create...");
   {
      int ncid;
      char name_in[NC_MAX_NAME + 1];
      int henry_vii_id;
      int grpid_in[MAX_SIBLING_GROUPS], varids_in[MAX_SIBLING_GROUPS];
      int dimids_in[MAX_SIBLING_GROUPS], nvars_in, ndims_in, ncid_in;
      int parent_ncid;
      char name_out[NC_MAX_NAME + 1];
      int num_grps;

      /* Create a file with one group, a group to contain data about
       * Henry VII. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_inq_grp_parent(ncid, &parent_ncid) != NC_ENOGRP) ERR;
      /* This should also work as simple "is this the root group" test */
      if (nc_inq_grp_parent(ncid, NULL) != NC_ENOGRP) ERR;
      strcpy(name_out, HENRY_VII);
      if (nc_def_grp(ncid, name_out, &henry_vii_id)) ERR;
      if (nc_inq_grp_parent(henry_vii_id, &parent_ncid)) ERR;
      if (parent_ncid != ncid) ERR;
      if (nc_inq_ncid(ncid, HENRY_VII, &ncid_in)) ERR;
      if (ncid_in != henry_vii_id) ERR;
      if (nc_inq_ncid(ncid, MARGARET, &ncid_in) != NC_ENOGRP) ERR;
      if (nc_close(ncid)) ERR;
   
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_grp_parent(ncid, &parent_ncid) != NC_ENOGRP) ERR;
      if (nc_inq_grps(ncid, &num_grps, NULL)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(ncid, NULL, grpid_in)) ERR;
      if (nc_inq_grpname(ncid, name_in)) ERR;
      if (strcmp(name_in, "/")) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (nc_inq_grp_parent(grpid_in[0], &parent_ncid)) ERR;
      if (parent_ncid != ncid) ERR;
      if (strcmp(name_in, HENRY_VII)) ERR;
      if (nc_inq_varids(grpid_in[0], &nvars_in, varids_in)) ERR;
      if (nvars_in != 0) ERR;
      if (nc_inq_varids(grpid_in[0], &ndims_in, dimids_in)) ERR;
      if (ndims_in != 0) ERR;
      if (nc_inq_ncid(ncid, HENRY_VII, &ncid_in)) ERR;
      if (ncid_in != grpid_in[0]) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing netcdf-3 and group functions...");
   {
      int ncid;
      char name_in[NC_MAX_NAME + 1];
      size_t len_in;

      /* Create a classic file. */
      if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_grpname(ncid, name_in)) ERR;
      if (strcmp(name_in, "/")) ERR;
      if (nc_inq_grpname_full(ncid, &len_in, name_in)) ERR;
      if (strcmp(name_in, "/") || len_in != 1) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
/*    printf("*** testing use of unlimited dim in parent group..."); */
/*    { */
/* #define NDIMS_IN_VAR 1 */
/* #define NDIMS_IN_FILE 2 */
/* #define BABE_LIMIT 3 */
/* #define DIM_NAME1 "Influence" */
/* #define DIM_NAME2 "Babe_Factor" */
/* #define VAR_NAME1 "Court_of_Star_Chamber" */
/* #define VAR_NAME2 "Justice_of_the_Peace" */
/* #define VAR_NAME3 "Bosworth_Field" */
/*       int ncid, dimid1, dimid2, varid1, varid2, varid3, henry_vii_id; */
/*       int grpid_in, varid_in1, varid_in2, varid_in3; */
/*       nc_type xtype_in; */
/*       int ndims_in, dimids_in[NDIMS_IN_FILE], dimid1_in, natts; */
/*       char name_in[NC_MAX_NAME + 1]; */
/*       size_t len_in, index[NDIMS_IN_VAR] = {0}; */
/*       long long value = NC_FILL_INT64 + 1, value_in; */

/*       /\* Create a file with an unlimited dim and a limited, used by */
/*        * variables in child groups. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_dim(ncid, DIM_NAME1, NC_UNLIMITED, &dimid1)) ERR; */
/*       if (nc_def_dim(ncid, DIM_NAME2, BABE_LIMIT, &dimid2)) ERR; */
/*       if (nc_def_grp(ncid, HENRY_VII, &henry_vii_id)) ERR; */
/*       if (nc_def_var(henry_vii_id, VAR_NAME1, NC_INT64, NDIMS_IN_VAR, &dimid1, &varid1)) ERR; */
/*       if (nc_def_var(henry_vii_id, VAR_NAME2, NC_INT64, NDIMS_IN_VAR, &dimid1, &varid2)) ERR; */
/*       if (nc_def_var(henry_vii_id, VAR_NAME3, NC_INT64, NDIMS_IN_VAR, &dimid2, &varid3)) ERR; */
      
/*       /\* Check it out. Find the group by name. *\/ */
/*       if (nc_inq_ncid(ncid, HENRY_VII, &grpid_in)) ERR; */

/*       /\* Ensure that dimensions in parent are visible and correct. *\/ */
/*       if (nc_inq_dimids(grpid_in, &ndims_in, dimids_in, 1)) ERR; */
/*       if (ndims_in != NDIMS_IN_FILE || dimids_in[0] != dimid1 || dimids_in[1] != dimid2) ERR; */
/*       if (nc_inq_dim(grpid_in, dimids_in[0], name_in, &len_in)) ERR; */
/*       if (strcmp(name_in, DIM_NAME1) || len_in != 0) ERR; */
/*       if (nc_inq_dim(grpid_in, dimids_in[1], name_in, &len_in)) ERR; */
/*       if (strcmp(name_in, DIM_NAME2) || len_in != BABE_LIMIT) ERR; */

/*       /\* Check the vars in the group. *\/ */
/*       if (nc_inq_varid(grpid_in, VAR_NAME1, &varid_in1)) ERR; */
/*       if (nc_inq_varid(grpid_in, VAR_NAME2, &varid_in2)) ERR; */
/*       if (nc_inq_varid(grpid_in, VAR_NAME3, &varid_in3)) ERR; */
/*       if (varid_in1 != varid1 || varid_in2 != varid2 || varid_in3 != varid3) ERR; */
/*       if (nc_inq_var(grpid_in, varid1, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR; */
/*       if (strcmp(name_in, VAR_NAME1) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR || */
/*           dimid1_in != dimid1 || natts != 0) ERR; */
/*       if (nc_inq_var(grpid_in, varid2, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR; */
/*       if (strcmp(name_in, VAR_NAME2) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR || */
/*           dimid1_in != dimid1 || natts != 0) ERR; */
/*       if (nc_inq_var(grpid_in, varid3, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR; */
/*       if (strcmp(name_in, VAR_NAME3) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR || */
/*           dimid1_in != dimid2 || natts != 0) ERR; */

/*       /\* Write one value to one variable. *\/ */
/*       if (nc_put_var1_longlong(grpid_in, varid_in1, index, &value)) ERR; */

/*       /\* Read one value from the second unlim dim variable. It should */
/*        * be the fill value. *\/ */
/*       if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in)) ERR; */
/*       if (value_in != NC_FILL_INT64) ERR; */

/*       /\* Read one value from the variable with limited dim. It should */
/*        * be the fill value. *\/ */
/*       if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in)) ERR; */
/*       if (value_in != NC_FILL_INT64) ERR; */

/*       /\* Attempt to read beyond end of dimensions to generate error. *\/ */
/*       index[0] = BABE_LIMIT; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in1, index, &value_in) != NC_EINVALCOORDS) ERR; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in) != NC_EINVALCOORDS) ERR; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in) != NC_EINVALCOORDS) ERR; */

/*       if (nc_close(ncid)) ERR; */

/*       /\* Check it out again. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */

/*       /\* Find the group by name. *\/ */
/*       if (nc_inq_ncid(ncid, HENRY_VII, &grpid_in)) ERR; */

/*       /\* Ensure that dimensions in parent are visible and correct. *\/ */
/*       if (nc_inq_dimids(grpid_in, &ndims_in, dimids_in, 1)) ERR; */
/*       if (ndims_in != NDIMS_IN_FILE || dimids_in[0] != dimid1 || dimids_in[1] != dimid2) ERR; */
/*       if (nc_inq_dim(grpid_in, dimids_in[0], name_in, &len_in)) ERR; */
/*       if (strcmp(name_in, DIM_NAME1) || len_in != 1) ERR; */
/*       if (nc_inq_dim(grpid_in, dimids_in[1], name_in, &len_in)) ERR; */
/*       if (strcmp(name_in, DIM_NAME2) || len_in != BABE_LIMIT) ERR; */

/*       /\* Check the vars in the group. *\/ */
/*       if (nc_inq_varid(grpid_in, VAR_NAME1, &varid_in1)) ERR; */
/*       if (nc_inq_varid(grpid_in, VAR_NAME2, &varid_in2)) ERR; */
/*       if (nc_inq_varid(grpid_in, VAR_NAME3, &varid_in3)) ERR; */
/*       if (varid_in1 != varid1 || varid_in2 != varid2 || varid_in3 != varid3) ERR; */
/*       if (nc_inq_var(grpid_in, varid1, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR; */
/*       if (strcmp(name_in, VAR_NAME1) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR || */
/*           dimid1_in != dimid1 || natts != 0) ERR; */
/*       if (nc_inq_var(grpid_in, varid2, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR; */
/*       if (strcmp(name_in, VAR_NAME2) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR || */
/*           dimid1_in != dimid1 || natts != 0) ERR; */
/*       if (nc_inq_var(grpid_in, varid3, name_in, &xtype_in, &ndims_in, &dimid1_in, &natts)) ERR; */
/*       if (strcmp(name_in, VAR_NAME3) || xtype_in != NC_INT64 || ndims_in != NDIMS_IN_VAR || */
/*           dimid1_in != dimid2 || natts != 0) ERR; */

/*       /\* Read one value from the second unlim dim variable. It should */
/*        * be the fill value. *\/ */
/*       index[0] = 0; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in)) ERR; */
/*       if (value_in != NC_FILL_INT64) ERR; */

/*       /\* Read one value from the variable with limited dim. It should */
/*        * be the fill value. *\/ */
/*       if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in)) ERR; */
/*       if (value_in != NC_FILL_INT64) ERR; */

/*       /\* Attempt to read beyond end of dimensions to generate error. *\/ */
/*       index[0] = BABE_LIMIT; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in1, index, &value_in) != NC_EINVALCOORDS) ERR; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in2, index, &value_in) != NC_EINVALCOORDS) ERR; */
/*       if (nc_get_var1_longlong(grpid_in, varid_in3, index, &value_in) != NC_EINVALCOORDS) ERR; */

/*       if (nc_close(ncid)) ERR; */
/*    } */

/*    SUMMARIZE_ERR; */
   printf("*** testing simple nested group creates...");
   {
      int ncid, grp_ncid;
      int henry_vii_id, margaret_id, james_v_of_scotland_id, mary_i_of_scotland_id;
      int james_i_of_england_id;
      char name_in[NC_MAX_NAME + 1];
      char full_name[NC_MAX_NAME * 10], full_name_in[NC_MAX_NAME * 10];
      int grpid_in[MAX_SIBLING_GROUPS];
      int grp_in;
      int num_grps;
      size_t len;

      /* Create a file with some nested groups in it, suitable
       * to storing information about the Tudor dynasty of England. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_grp(henry_vii_id, MARGARET, &margaret_id)) ERR;
      if (nc_def_grp(margaret_id, JAMES_V_OF_SCOTLAND, &james_v_of_scotland_id)) ERR;
      if (nc_def_grp(james_v_of_scotland_id, MARY_I_OF_SCOTLAND, &mary_i_of_scotland_id)) ERR;
      if (nc_def_grp(mary_i_of_scotland_id, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &james_i_of_england_id)) ERR;

      strcpy(full_name, "/");
      if (nc_inq_grpname_full(ncid, &len, full_name_in)) ERR;
      if (len != 1 || strcmp(full_name_in, full_name)) ERR;
      if (nc_inq_grpname_len(ncid, &len)) ERR;
      if (len != 1) ERR;
      if (nc_inq_grp_full_ncid(ncid, full_name, &grp_in)) ERR;
      if (grp_in != ncid) ERR;

      if (nc_inq_grp_ncid(ncid, HENRY_VII, NULL)) ERR;
      if (nc_inq_grp_ncid(ncid, HENRY_VII, &grp_ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, NULL)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(ncid, NULL, grpid_in)) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, HENRY_VII)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;
      strcat(full_name, HENRY_VII);
      if (nc_inq_grpname_full(grpid_in[0], &len, full_name_in)) ERR;
      if (len != strlen(HENRY_VII) + 1 || strcmp(full_name_in, full_name)) ERR;
      if (nc_inq_grp_full_ncid(ncid, full_name, &grp_in)) ERR;
      if (grp_in != grpid_in[0]) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], MARGARET, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, MARGARET)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;
      strcat(full_name, "/");
      strcat(full_name, MARGARET);
      if (nc_inq_grpname_full(grpid_in[0], &len, full_name_in)) ERR;
      if (len != strlen(full_name) || strcmp(full_name_in, full_name)) ERR;
      if (nc_inq_grp_full_ncid(ncid, full_name, &grp_in)) ERR;
      if (grp_in != grpid_in[0]) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], JAMES_V_OF_SCOTLAND, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, JAMES_V_OF_SCOTLAND)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;
      strcat(full_name, "/");
      strcat(full_name, JAMES_V_OF_SCOTLAND);
      if (nc_inq_grpname_full(grpid_in[0], &len, full_name_in)) ERR;
      if (len != strlen(full_name) || strcmp(full_name_in, full_name)) ERR;
      if (nc_inq_grp_full_ncid(ncid, full_name, &grp_in)) ERR;
      if (grp_in != grpid_in[0]) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], MARY_I_OF_SCOTLAND, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, MARY_I_OF_SCOTLAND)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;
      strcat(full_name, "/");
      strcat(full_name, MARY_I_OF_SCOTLAND);
      if (nc_inq_grpname_full(grpid_in[0], &len, full_name_in)) ERR;
      if (len != strlen(full_name) || strcmp(full_name_in, full_name)) ERR;
      if (nc_inq_grp_full_ncid(ncid, full_name, &grp_in)) ERR;
      if (grp_in != grpid_in[0]) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;
      strcat(full_name, "/");
      strcat(full_name, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND);
      if (nc_inq_grpname_full(grpid_in[0], &len, full_name_in)) ERR;
      if (len != strlen(full_name) || strcmp(full_name_in, full_name)) ERR;
      if (nc_inq_grp_full_ncid(ncid, full_name, &grp_in)) ERR;
      if (grp_in != grpid_in[0]) ERR;

      if (nc_close(ncid)) ERR;

      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_grp_ncid(ncid, HENRY_VII, &grp_ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, NULL)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(ncid, NULL, grpid_in)) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, HENRY_VII)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], MARGARET, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, MARGARET)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], JAMES_V_OF_SCOTLAND, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, JAMES_V_OF_SCOTLAND)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], MARY_I_OF_SCOTLAND, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, MARY_I_OF_SCOTLAND)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;

      if (nc_inq_grp_ncid(grpid_in[0], JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &grp_ncid)) ERR;
      if (nc_inq_grps(grpid_in[0], &num_grps, grpid_in)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(grpid_in[0], name_in)) ERR;
      if (strcmp(name_in, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND)) ERR;
      if (grpid_in[0] != grp_ncid) ERR;

      /* Close up shop. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing simple sibling group creates...");
   {
      int ncid;
      int henry_vii_id, margaret_id, james_v_of_scotland_id, mary_i_of_scotland_id;
      int james_i_of_england_id, tudor_id;
      char name_in[NC_MAX_NAME + 1];
      int grpid_in[MAX_SIBLING_GROUPS];
      int ncid_in;
      int num_grps;
      int dynasty;

      /* Create a file with one group, and beneath it a group for each Tudor. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;
      if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_grp(tudor_id, MARGARET, &margaret_id)) ERR;
      if (nc_def_grp(tudor_id, JAMES_V_OF_SCOTLAND, &james_v_of_scotland_id)) ERR;
      if (nc_def_grp(tudor_id, MARY_I_OF_SCOTLAND, &mary_i_of_scotland_id)) ERR;
      if (nc_def_grp(tudor_id, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &james_i_of_england_id)) ERR;
      if (nc_close(ncid)) ERR;

      /* Make sure we've got all the tudors where we want them. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(dynasty, name_in)) ERR;
      if (strcmp(name_in, DYNASTY)) ERR;
      if (nc_inq_grps(dynasty, &num_grps, grpid_in)) ERR;
      if (num_grps != 5) ERR;
      if (nc_inq_ncid(dynasty, HENRY_VII, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, HENRY_VII)) ERR;
      if (nc_inq_ncid(dynasty, MARGARET, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, MARGARET)) ERR;
      if (nc_inq_ncid(dynasty, JAMES_V_OF_SCOTLAND, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, JAMES_V_OF_SCOTLAND)) ERR;
      if (nc_inq_ncid(dynasty, MARY_I_OF_SCOTLAND, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, MARY_I_OF_SCOTLAND)) ERR;
      if (nc_inq_ncid(dynasty, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND)) ERR;
      if (nc_close(ncid)) ERR;
   }
   
   SUMMARIZE_ERR;
   printf("*** testing more group attributes...");

   {
      int ncid, num_grps, dynasty, ncid_in;
      int grpid_in[MAX_SIBLING_GROUPS];
      int henry_vii_id, margaret_id, james_v_of_scotland_id, mary_i_of_scotland_id;
      int james_i_of_england_id, tudor_id;
      int num_castles_henry_vii = 1, num_castles_margaret = 0, num_castles_james_v = 2;
      int num_castles_mary_i = 3, num_castles_james_vi = 4;
      char name_in[NC_MAX_NAME + 1];

      /* Create a file with one group, and beneath it a group for each
       * Tudor. We will have some attributes in the groups.*/
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;
      if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_put_att_int(henry_vii_id, NC_GLOBAL, NUM_CASTLES_NAME, NC_INT, 
			 1, &num_castles_henry_vii)) ERR;
      if (nc_def_grp(tudor_id, MARGARET, &margaret_id)) ERR;
      if (nc_put_att_int(margaret_id, NC_GLOBAL, NUM_CASTLES_NAME, NC_INT, 
			 1, &num_castles_margaret)) ERR;
      if (nc_def_grp(tudor_id, JAMES_V_OF_SCOTLAND, &james_v_of_scotland_id)) ERR;
      if (nc_put_att_int(james_v_of_scotland_id, NC_GLOBAL, NUM_CASTLES_NAME, NC_INT, 
			 1, &num_castles_james_v)) ERR;
      if (nc_def_grp(tudor_id, MARY_I_OF_SCOTLAND, &mary_i_of_scotland_id)) ERR;
      if (nc_put_att_int(mary_i_of_scotland_id, NC_GLOBAL, NUM_CASTLES_NAME, NC_INT, 
			 1, &num_castles_mary_i)) ERR;
      if (nc_def_grp(tudor_id, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, 
		     &james_i_of_england_id)) ERR;
      if (nc_put_att_int(james_i_of_england_id, NC_GLOBAL, NUM_CASTLES_NAME, 
			 NC_INT, 1, &num_castles_james_vi)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Make sure we've got all the tudors where we want them. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(dynasty, name_in)) ERR;
      if (strcmp(name_in, DYNASTY)) ERR;
      if (nc_inq_grps(dynasty, &num_grps, grpid_in)) ERR;
      if (num_grps != 5) ERR;
      if (nc_inq_ncid(dynasty, HENRY_VII, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, HENRY_VII)) ERR;
      if (nc_inq_ncid(dynasty, MARGARET, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, MARGARET)) ERR;
      if (nc_inq_ncid(dynasty, JAMES_V_OF_SCOTLAND, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, JAMES_V_OF_SCOTLAND)) ERR;
      if (nc_inq_ncid(dynasty, MARY_I_OF_SCOTLAND, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, MARY_I_OF_SCOTLAND)) ERR;
      if (nc_inq_ncid(dynasty, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &ncid_in)) ERR;
      if (nc_inq_grpname(ncid_in, name_in)) ERR;
      if (strcmp(name_in, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND)) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing groups and dimensions...");

   {
      int ncid;
      int tudor_id;
      int num_grps;
      int dimid, dimid_in, dynasty;
      size_t len_in;
      char name_in[NC_MAX_NAME + 1];

      /* Create a file with one group, and within that group, a dimension.*/
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;
      if (nc_def_dim(tudor_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Now check the file to see if the dimension is there. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grpname(dynasty, name_in)) ERR;
      if (strcmp(name_in, DYNASTY)) ERR;
      if (nc_inq_dimid(dynasty, DIM1_NAME, &dimid_in)) ERR;
      if (dimid_in != 0) ERR;
      if (nc_inq_dimname(dynasty, 0, name_in)) ERR;
      if (strcmp(name_in, DIM1_NAME)) ERR;
      if (nc_inq_dim(dynasty, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM1_NAME)) ERR;
      if (len_in != DIM1_LEN) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing groups and vars...");

   {
      int ncid, ndims_in;
      int tudor_id;
      int num_grps;
      int dimid, dynasty, varid;
      size_t len_in;
      int natts_in;
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int dimids_in[MAX_SIBLING_GROUPS];

      /* Create a file with one group, and within that group, a
       * dimension, and a variable.*/
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;
      if (nc_def_dim(tudor_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(tudor_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Now check the file to see if the dimension and variable are
       * there. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_dim(dynasty, 0, name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR;
      if (nc_inq_var(dynasty, 0, name_in, &xtype_in, &ndims_in, dimids_in, 
		     &natts_in)) ERR;
      if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	  dimids_in[0] != 0 || natts_in != 0) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing group functions in netCDF classic file...");

   {
      int ncid;
      int num_grps;

      /* Create a classic file.*/
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, NULL)) ERR;
      if (num_grps) ERR;
      if (nc_close(ncid)) ERR;

      /* Now check the file to see if the dimension and variable are
       * there. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      
      if (nc_inq_grps(ncid, &num_grps, NULL)) ERR;
      if (num_grps) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing groups and vars...");

   {
      int ncid, ndims_in;
      int henry_vii_id, margaret_id, james_v_of_scotland_id, mary_i_of_scotland_id;
      int james_i_of_england_id, tudor_id;
      int dimids_in[MAX_SIBLING_GROUPS];
      int num_grps;
      int dimid, dynasty, varid;
      size_t len_in;
      int natts_in;
      int grpids_in[10];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data_out[DIM1_LEN] = {-99, 0, 99}, data_in[DIM1_LEN];
      int i, j;

      /* Create a file with a group, DYNASTY, containing 5 groups,
       * each with a dimension and one int variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;
      /* Henry VII. */
      if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_dim(henry_vii_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(henry_vii_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(henry_vii_id, varid, data_out)) ERR;
      /* Margaret. */
      if (nc_def_grp(tudor_id, MARGARET, &margaret_id)) ERR;
      if (nc_def_dim(margaret_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(margaret_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(margaret_id, varid, data_out)) ERR;
      /* James V of Scotland. */
      if (nc_def_grp(tudor_id, JAMES_V_OF_SCOTLAND, &james_v_of_scotland_id)) ERR;
      if (nc_def_dim(james_v_of_scotland_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(james_v_of_scotland_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(james_v_of_scotland_id, varid, data_out)) ERR;
      /* Mary I of Scotland. */
      if (nc_def_grp(tudor_id, MARY_I_OF_SCOTLAND, &mary_i_of_scotland_id)) ERR;
      if (nc_def_dim(mary_i_of_scotland_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(mary_i_of_scotland_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(mary_i_of_scotland_id, varid, data_out)) ERR;
      /* James VI of Scotland and I of England. */
      if (nc_def_grp(tudor_id, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &james_i_of_england_id)) ERR;
      if (nc_def_dim(james_i_of_england_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;
      if (nc_def_var(james_i_of_england_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(james_i_of_england_id, varid, data_out)) ERR;

      /*nc_show_metadata(ncid);*/

      /* Check it out. */
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(dynasty, &num_grps, grpids_in)) ERR;
      if (num_grps != 5) ERR;
      for (i = 0; i < 5; i++)
      {
	 if (nc_inq_dim(grpids_in[i], i, name_in, &len_in)) ERR;
	 if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR;
	 if (nc_inq_var(grpids_in[i], 0, name_in, &xtype_in, &ndims_in, dimids_in, 
			&natts_in)) ERR;
	 if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	     dimids_in[0] != i || natts_in != 0) ERR;
	 if (nc_get_var_int(grpids_in[i], 0, data_in)) ERR;
	 for (j=0; j<DIM1_LEN; j++)
	    if (data_in[j] != data_out[j]) ERR;
      }

      if (nc_close(ncid)) ERR;

      /* Reopen. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      

      /* Check it out. */
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(dynasty, &num_grps, grpids_in)) ERR;
      if (num_grps != 5) ERR;
      for (i = 0; i < 5; i++)
      {
	 /* We actually get the groups in alphabetical order, so our
	  * dimid is not i. */
	 /*if (nc_inq_dim(grpids_in[i], i, name_in, &len_in)) ERR;
	   if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR;*/
	 if (nc_inq_var(grpids_in[i], 0, name_in, &xtype_in, &ndims_in, dimids_in, 
			&natts_in)) ERR;
	 if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	     natts_in != 0) ERR;
	 if (nc_get_var_int(grpids_in[i], 0, data_in)) ERR;
	 for (j=0; j<DIM1_LEN; j++)
	    if (data_in[j] != data_out[j]) ERR;
      }

      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing very simple groups and dimension scoping...");

   {
      int ncid;
      int dimids_in[MAX_SIBLING_GROUPS], nvars_in, ndims_in;
      int henry_vii_id;
      int num_grps;
      int dimid, dimid2, varid;
      size_t len_in;
      int natts_in;
      int unlimdimid_in;
      int grpids_in[10];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data_out[DIM1_LEN] = {0, 2, 6}, data_in[DIM1_LEN];
      int j;

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define two dimensions, "year" and "kingdom" in the root group. */
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimid2)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimid)) ERR;

      /* Define a HENRY_VII group. It contains a var with dim "kingdom". */
      if (nc_def_grp(ncid, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_var(henry_vii_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(henry_vii_id, varid, data_out)) ERR;
      
      /* Done! */
      if (nc_close(ncid)) ERR;

      /* Reopen the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      

      /* Check the file. */
      if (nc_inq_grps(ncid, &num_grps, &henry_vii_id)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(henry_vii_id, &num_grps, grpids_in)) ERR;
      if (num_grps != 0) ERR;
      if (nc_inq(henry_vii_id, &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 0 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != -1) ERR;
      if (nc_inq_var(henry_vii_id, 0, name_in, &xtype_in, &ndims_in, dimids_in, 
		     &natts_in)) ERR;
      if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	  dimids_in[0] != dimid || natts_in != 0) ERR;
      if (nc_inq_dim(ncid, dimid, name_in, &len_in)) ERR;
      if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR;
      if (nc_get_var_int(henry_vii_id, 0, data_in)) ERR;
      for (j=0; j<DIM1_LEN; j++)
	 if (data_in[j] != data_out[j]) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing groups and dimension scoping...");

#define NUM_GRPS 5

   {
      int ncid;
      int henry_vii_id, margaret_id, james_v_of_scotland_id, mary_i_of_scotland_id;
      int james_i_of_england_id, tudor_id;
      int dimids_in[MAX_SIBLING_GROUPS], nvars_in, ndims_in;
      int num_grps;
      int dimid, dimid2, dynasty, varid;
      size_t len_in;
      int natts_in;
      int unlimdimid_in;
      int grpids_in[10];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];
      int data_out[DIM1_LEN] = {0, 2, 6}, data_in[DIM1_LEN];
      int i, j;

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define a bunch of groups, define a variable in each, and
       * write some data in it. The vars use a shared dimension from
       * the parent group. */

      /* Create the parent group. */
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;

      /* Define two dimensions, "year" and "kingdom" in the parent group. */
      if (nc_def_dim(tudor_id, DIM2_NAME, DIM2_LEN, &dimid2)) ERR;
      if (nc_def_dim(tudor_id, DIM1_NAME, DIM1_LEN, &dimid)) ERR;

      /* Define a HENRY_VII group. It contains a var with dim "kingdom". */
      if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR;
      if (nc_def_var(henry_vii_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(henry_vii_id, varid, data_out)) ERR;
      
      /* Define a MARGARET group. */
      if (nc_def_grp(tudor_id, MARGARET, &margaret_id)) ERR;
      if (nc_def_var(margaret_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(margaret_id, varid, data_out)) ERR;

      /* Define a JAMES_V_OF_SCOTLAND group. */
      if (nc_def_grp(tudor_id, JAMES_V_OF_SCOTLAND, &james_v_of_scotland_id)) ERR;
      if (nc_def_var(james_v_of_scotland_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(james_v_of_scotland_id, varid, data_out)) ERR;

      /* Define a MARY_I_OF_SCOTLAND group. */
      if (nc_def_grp(tudor_id, MARY_I_OF_SCOTLAND, &mary_i_of_scotland_id)) ERR;
      if (nc_def_var(mary_i_of_scotland_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(mary_i_of_scotland_id, varid, data_out)) ERR;

      /* Define a JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND group. */
      if (nc_def_grp(tudor_id, JAMES_VI_OF_SCOTLAND_AND_I_OF_ENGLAND, &james_i_of_england_id)) ERR;
      if (nc_def_var(james_i_of_england_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR;
      if (nc_put_var_int(james_i_of_england_id, varid, data_out)) ERR;

      /* Done! */
      if (nc_close(ncid)) ERR;

      /* Reopen the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      

      /* Check the file. */
      if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR;
      if (num_grps != 1) ERR;
      if (nc_inq_grps(dynasty, &num_grps, grpids_in)) ERR;
      if (num_grps != NUM_GRPS) ERR;
      for (i = 0; i < NUM_GRPS; i++)
      {
	 if (nc_inq(grpids_in[i], &ndims_in, &nvars_in, &natts_in, &unlimdimid_in)) ERR;
	 if (ndims_in != 0 || nvars_in != 1 || natts_in != 0 || unlimdimid_in != -1) ERR;
	 if (nc_inq_var(grpids_in[i], 0, name_in, &xtype_in, &ndims_in, dimids_in, 
			&natts_in)) ERR;
	 if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 ||
	     dimids_in[0] != dimid || natts_in != 0) ERR;
	 if (nc_inq_dim(grpids_in[i], dimid, name_in, &len_in)) ERR;
	 if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR;
	 if (nc_get_var_int(grpids_in[i], 0, data_in)) ERR;
	 for (j=0; j<DIM1_LEN; j++)
	    if (data_in[j] != data_out[j]) ERR;
      }

      /* Close the file. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing more groups and dimension scoping...");
   {
      int ncid;
      int henry_vii_id;
      int tudor_id;
      int dimids_in[MAX_SIBLING_GROUPS], ndims_in;
      int year_did, kingdom_did, varid, dimids[2], parent_id;
      int natts_in;
      int numgrps;
      int grpids_in[10];
      nc_type xtype_in;
      char name_in[NC_MAX_NAME + 1];

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create the parent group. */
      if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR;

      /* Define two dimensions, "year" and "kingdom" in the parent group. */
      if (nc_def_dim(tudor_id, DIM1_NAME, DIM1_LEN, &kingdom_did)) ERR;
      if (nc_def_dim(tudor_id, DIM2_NAME, DIM2_LEN, &year_did)) ERR;

      /* Define a HENRY_VII group. It contains a var using both dimensions. */
      if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR;
      dimids[0] = year_did;
      dimids[1] = kingdom_did;
      if (nc_def_var(henry_vii_id, VAR1_NAME, NC_UINT64, 2, dimids, &varid)) ERR;
      if (nc_inq_var(henry_vii_id, varid, name_in, &xtype_in, &ndims_in, 
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_UINT64 || ndims_in != 2 ||
	  dimids_in[0] != year_did || dimids_in[1] != kingdom_did ||
	  natts_in != 0) ERR;

      /* Close the file. */
      if (nc_close(ncid)) ERR;
      
      /* Reopen the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      

      /* Find our group. */
      if (nc_inq_grps(ncid, &numgrps, grpids_in)) ERR;
      if (numgrps != 1) ERR;
      parent_id = grpids_in[0];
      if (nc_inq_grps(parent_id, &numgrps, grpids_in)) ERR;
      if (numgrps != 1) ERR;
      henry_vii_id = grpids_in[0];
      if (nc_inq_var(henry_vii_id, varid, name_in, &xtype_in, &ndims_in, 
		     dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_UINT64 || ndims_in != 2 ||
	  dimids_in[0] != year_did || dimids_in[1] != kingdom_did ||
	  natts_in != 0) ERR;
      /* Close the file. */
      if (nc_close(ncid)) ERR;
      
   }
   SUMMARIZE_ERR;
/*    printf("*** testing groups and unlimited dimensions..."); */
/*    { */
/*       int ncid; */
/*       int henry_vii_id; */
/*       int tudor_id; */
/*       int dimids_in[MAX_SIBLING_GROUPS], ndims_in; */
/*       int num_grps; */
/*       int dimid, dynasty, varid; */
/*       size_t len_in; */
/*       int natts_in; */
/*       int grpids_in[10]; */
/*       nc_type xtype_in; */
/*       char name_in[NC_MAX_NAME + 1]; */
/*       int data_out[DIM1_LEN] = {0, 2, 6}, data_in[DIM1_LEN]; */
/*       size_t start[1] = {0}, count[1] = {3}; */
/*       int j; */

/*       /\* Create one group, with one var, which has one dimension, which is unlimited. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_grp(ncid, DYNASTY, &tudor_id)) ERR; */
/*       if (nc_def_dim(tudor_id, DIM1_NAME, NC_UNLIMITED, &dimid)) ERR; */
/*       if (nc_def_grp(tudor_id, HENRY_VII, &henry_vii_id)) ERR; */
/*       if (nc_def_var(henry_vii_id, VAR1_NAME, NC_INT, 1, &dimid, &varid)) ERR; */
/*       if (nc_put_vara_int(henry_vii_id, varid, start, count, data_out)) ERR; */
/*       if (nc_close(ncid)) ERR; */

/*       /\* Now check the file to see if the dimension and variable are */
/*        * there. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;       */
/*       if (nc_inq_grps(ncid, &num_grps, &dynasty)) ERR; */
/*       if (num_grps != 1) ERR; */
/*       if (nc_inq_grps(dynasty, &num_grps, grpids_in)) ERR; */
/*       if (num_grps != 1) ERR; */
/*       if (nc_inq_dim(grpids_in[0], 0, name_in, &len_in)) ERR; */
/*       if (strcmp(name_in, DIM1_NAME) || len_in != DIM1_LEN) ERR; */
/*       if (nc_inq_var(grpids_in[0], 0, name_in, &xtype_in, &ndims_in, dimids_in,  */
/* 		     &natts_in)) ERR; */
/*       if (strcmp(name_in, VAR1_NAME) || xtype_in != NC_INT || ndims_in != 1 || */
/* 	  dimids_in[0] != 0 || natts_in != 0) ERR; */
/*       if (nc_get_vara_int(grpids_in[0], 0, start, count, data_in)) ERR; */
/*       for (j=0; j<DIM1_LEN; j++) */
/* 	 if (data_in[j] != data_out[j]) ERR; */
/*       if (nc_close(ncid)) ERR; */
/*    } */
/*    SUMMARIZE_ERR; */
   printf("*** testing nested groups...");
#define DIM_NAME "dim"
#define DIM_LEN 3
#define VAR_NAME "var"
#define VAR_RANK 1
#define ATT_NAME "units"
#define ATT_VAL "m/s"
#define GATT_NAME "title"
#define GATT_VAL "for testing groups"
#define G1_NAME "the_in_crowd"
#define G2_NAME "the_out_crowd"
#define G3_NAME "the_confused_crowd"
   {
      int ncid;
      int dimid, varid;
      int var_dims[VAR_RANK];
      int g1id, g2id, g3id;
    
      /* Create a file with nested groups. */
      if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
      /* At root level define dim, var, atts */
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      var_dims[0] = dimid;
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
      if (nc_put_att_text(ncid, varid, ATT_NAME, strlen(ATT_VAL), ATT_VAL)) ERR;
      if (nc_put_att_text(ncid, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL), 
			  GATT_VAL)) ERR;
    
      /* put dim, var, atts with same names in a group */
      if (nc_def_grp(ncid, G1_NAME, &g1id)) ERR;
      if (nc_def_dim(g1id, DIM_NAME, DIM_LEN, &dimid)) ERR;
      var_dims[0] = dimid;
      if (nc_def_var(g1id, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
      if (nc_put_att_text(g1id, varid, ATT_NAME, strlen(ATT_VAL), ATT_VAL)) ERR;
      if (nc_put_att_text(g1id, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL), 
			  GATT_VAL)) ERR;
      if (nc_enddef(g1id)) ERR;

      /* put dim, var, atts with same names in a second group */
      if (nc_def_grp(ncid, G2_NAME, &g2id)) ERR;
      if (nc_def_dim(g2id, DIM_NAME, DIM_LEN, &dimid)) ERR;
      var_dims[0] = dimid;
      if (nc_def_var(g2id, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
      if (nc_put_att_text(g2id, varid, ATT_NAME, strlen(ATT_VAL), ATT_VAL)) ERR;
      if (nc_put_att_text(g2id, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL), 
			  GATT_VAL)) ERR;
      if (nc_enddef(g2id)) ERR;
    
      /* put dim, var, atts with same names in a subgroup of second group */
      if (nc_def_grp(g2id, G3_NAME, &g3id)) ERR;
      if (nc_def_dim(g3id, DIM_NAME, DIM_LEN, &dimid)) ERR;
      var_dims[0] = dimid;
      if (nc_def_var(g3id, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
      if (nc_put_att_text(g3id, varid, ATT_NAME, strlen(ATT_VAL), ATT_VAL)) ERR;
      if (nc_put_att_text(g3id, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL), 
			  GATT_VAL)) ERR;
      if (nc_enddef(g3id)) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** testing nested groups, user defined types, and enddef...");

#define SCI_FI "Science_Fiction"   
#define BASE_SIZE 2
#define TYPE_NAME "The_Blob"
#define DATE_MOVIE "data_movie"
   {
      int ncid, xtype, g1id, class;
      char name_in[NC_MAX_NAME + 1];
      size_t len_in;
      unsigned char data[BASE_SIZE] = {42, 43}, data_in[BASE_SIZE];
    
      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* define subgroup with a user defined type. */
      if (nc_def_grp(ncid, SCI_FI, &g1id)) ERR;
      if (nc_def_opaque(g1id, BASE_SIZE, TYPE_NAME, &xtype)) ERR;
      if (nc_put_att(g1id, NC_GLOBAL, DATE_MOVIE, xtype, 1, &data)) ERR;

      /* Check it. */
      if (nc_inq_user_type(g1id, xtype, name_in, &len_in, NULL, NULL, &class)) ERR;
      if (strcmp(name_in, TYPE_NAME) || len_in != BASE_SIZE || class != NC_OPAQUE) ERR;
      if (nc_get_att(g1id, NC_GLOBAL, DATE_MOVIE, data_in)) ERR;
      if (data[0] != data_in[0] || data[1] != data_in[1]) ERR;

      /* Call enddef and try again. */
      if (nc_enddef(g1id)) ERR;

      if (nc_inq_user_type(g1id, xtype, name_in, &len_in, NULL, NULL, &class)) ERR;
      if (strcmp(name_in, TYPE_NAME) || len_in != BASE_SIZE || class != NC_OPAQUE) ERR;
      if (nc_get_att(g1id, NC_GLOBAL, DATE_MOVIE, data_in)) ERR;
      if (data[0] != data_in[0] || data[1] != data_in[1]) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen and recheck. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;      
      if (nc_inq_grp_ncid(ncid, SCI_FI, &g1id)) ERR;

      if (nc_inq_user_type(g1id, xtype, name_in, &len_in, NULL, NULL, &class)) ERR;
      if (strcmp(name_in, TYPE_NAME) || len_in != BASE_SIZE || class != NC_OPAQUE) ERR;
      if (nc_get_att(g1id, NC_GLOBAL, DATE_MOVIE, data_in)) ERR;
      if (data[0] != data_in[0] || data[1] != data_in[1]) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** creating file with lots of user-defined types...");
   {
      int ncid, typeid;
      int g1id, g2id, g3id;
    
      /* Create a file with nested groups. */
      if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
      if (nc_def_opaque(ncid, 10, "opaque-1", &typeid)) ERR;
      if (nc_def_vlen(ncid, "vlen-1", NC_INT, &typeid)) ERR;
      if (nc_enddef(ncid)) ERR; 

      if (nc_def_grp(ncid, G1_NAME, &g1id)) ERR;
      if (nc_def_opaque(g1id, 7, "opaque-2", &typeid)) ERR;
      if (nc_def_vlen(g1id, "vlen-2", NC_BYTE, &typeid)) ERR;
      if (nc_enddef(g1id)) ERR; 

      if (nc_def_grp(ncid, G2_NAME, &g2id)) ERR;
      if (nc_def_opaque(g2id, 4, "opaque-3", &typeid)) ERR;
      if (nc_def_vlen(g2id, "vlen-3", NC_BYTE, &typeid)) ERR;
      if (nc_enddef(g2id)) ERR; 
    
      /* put dim, var, atts with same names in a subgroup of second group */
      if (nc_def_grp(g2id, G3_NAME, &g3id)) ERR;
      if (nc_def_opaque(g3id, 13, "opaque-4", &typeid)) ERR;
      if (nc_def_vlen(g3id, "vlen-4", NC_BYTE, &typeid)) ERR;
      if (nc_enddef(g3id)) ERR; 

      if (nc_close(ncid)) ERR;

      /* Now count how many user-defined types there are */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

   }

   SUMMARIZE_ERR;
   printf("*** creating file with lots of groups...");
   {
#define PARENT_NUM_GRPS 6
#define SUB_NUM_GRPS 2  

      int ncid, g1id, sub_grpid, num_grps, g, s;
      char grp_name[NC_MAX_NAME + 1];
    
      /* Create a file with lots of groups. */
      if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
      for (g = 0; g < PARENT_NUM_GRPS; g++)
      {
	 sprintf(grp_name, "grp_%d", g);
	 if (nc_def_grp(ncid, grp_name, &g1id)) ERR;
	 for (s = 0; s < SUB_NUM_GRPS; s++)
	 {
	    sprintf(grp_name, "sub_grp_%d", s);
	    if (nc_def_grp(g1id, grp_name, &sub_grpid)) ERR;
	 }
      }

      if (nc_close(ncid)) ERR;

      /* Now count how groups there are. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_grps(ncid, &num_grps, NULL)) ERR;
      if (num_grps != PARENT_NUM_GRPS) ERR;
      if (nc_close(ncid)) ERR;

   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

