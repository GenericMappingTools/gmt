/* This is part of the netCDF package.  

   Copyright 2005-2007, University Corporation for Atmospheric
   Research/Unidata See COPYRIGHT file for conditions of use.

   This program creates a test file with groups for ncdump to read.

   $Id$
*/

#include <netcdf.h>
#include <nc_tests.h>

#define FILE_NAME "tst_group_data.nc"
#define DIM_NAME "dim"
#define DIM_LEN 4
#define DIM_LEN1 1
#define DIM_LEN2 2
#define DIM_LEN3 3
#define VAR_NAME "var"
#define VAR_RANK 1
#define VAR2_NAME "var2"
#define VAR2_RANK 3
#define ATT_NAME "units"
#define ATT_VAL "m/s"
#define ATT_VAL1 "km/hour"
#define ATT_VAL2 "cm/sec"
#define ATT_VAL3 "mm/msec"
#define GATT_NAME "title"
#define GATT_VAL "for testing groups"
#define GATT_VAL1 "in first group"
#define GATT_VAL2 "in second group"
#define GATT_VAL3 "in third group"
#define G1_NAME "g1"
#define G2_NAME "g2"
#define G3_NAME "g3"

int
main(int argc, char **argv) {
    int ncid, dimid, dimid1, dimid2, dimid3, varid, var2id;
    int var_dims[VAR_RANK], var2_dims[VAR2_RANK];
    int g1id, g2id, g3id;
    float vals[] = {1.0, 2.0, 3.0, 4.0};
    float vals2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
		     13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    
    printf("\n*** Testing groups.\n");
    printf("*** creating nested group file %s...", FILE_NAME);

    /* Create a file with nested groups. */
    if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
    /* At root level define dim, var, atts */
    if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
    var_dims[0] = dimid;
    if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
    if (nc_put_att_text(ncid, varid, ATT_NAME, strlen(ATT_VAL), ATT_VAL)) ERR;
    if (nc_put_att_text(ncid, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL), 
			GATT_VAL)) ERR;
    if (nc_enddef(ncid)) ERR;
    if (nc_put_var_float(ncid, varid, vals)) ERR;
    
    /* put dim, var, atts with same names in a group */
    if (nc_def_grp(ncid, G1_NAME, &g1id)) ERR;
    if (nc_def_dim(g1id, DIM_NAME, DIM_LEN1, &dimid1)) ERR;
    var_dims[0] = dimid1;
    if (nc_def_var(g1id, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
    if (nc_put_att_text(g1id, varid, ATT_NAME, strlen(ATT_VAL1), ATT_VAL1)) ERR;
    if (nc_put_att_text(g1id, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL1), 
			GATT_VAL1)) ERR;
    if (nc_enddef(g1id)) ERR;
    if (nc_put_var_float(g1id, varid, vals)) ERR;

    /* put dim, var, atts with same names in a second group */
    if (nc_def_grp(ncid, G2_NAME, &g2id)) ERR;
    if (nc_def_dim(g2id, DIM_NAME, DIM_LEN2, &dimid2)) ERR;
    var_dims[0] = dimid2;
    if (nc_def_var(g2id, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
    if (nc_put_att_text(g2id, varid, ATT_NAME, strlen(ATT_VAL2), ATT_VAL2)) ERR;
    if (nc_put_att_text(g2id, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL2), 
			GATT_VAL2)) ERR;
    if (nc_enddef(g2id)) ERR;
    if (nc_put_var_float(g2id, varid, vals)) ERR;
    
    /* put dim, var, atts with same names in a subgroup of second group */
    if (nc_def_grp(g2id, G3_NAME, &g3id)) ERR;
    if (nc_def_dim(g3id, DIM_NAME, DIM_LEN3, &dimid3)) ERR;
    var_dims[0] = dimid3;
    if (nc_def_var(g3id, VAR_NAME, NC_FLOAT, VAR_RANK, var_dims, &varid)) ERR;
    if (nc_put_att_text(g3id, varid, ATT_NAME, strlen(ATT_VAL3), ATT_VAL3)) ERR;
    if (nc_put_att_text(g3id, NC_GLOBAL, GATT_NAME, strlen(GATT_VAL3), 
			GATT_VAL3)) ERR;
    var2_dims[0] = dimid;
    var2_dims[1] = dimid2;
    var2_dims[2] = dimid3;
    if (nc_def_var(g3id, VAR2_NAME, NC_FLOAT, VAR2_RANK, var2_dims, &var2id)) ERR;
    if (nc_enddef(g3id)) ERR;
    if (nc_put_var_float(g3id, varid, vals)) ERR;
    if (nc_put_var_float(g3id, var2id, vals2)) ERR;

    if (nc_close(ncid)) ERR;

    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
