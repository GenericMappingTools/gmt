/* This is part of the netCDF package.
   Copyright 2008 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which writes a netCDF file with
   single- and double-precision NaNs and infinities.

   $Id$
*/
#include <nc_tests.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <netcdf.h>
#include "isnan.h"

#define FILE8_NAME "tst_nans.nc"
#define FV_NAME _FillValue	/* defined in netcdf.h */
#define FV_NVALS 1
#define ATT_NAME "att"
#define NDIMS 1
#define DIM_NAME "dim"
#define NVALS 3
#define F_NAME "fvar"
#define D_NAME "dvar"

int
main(int argc, char **argv)
{
    int ncid, dimid, fvarid, dvarid;
    float fvals[NVALS], fvals_in[NVALS];
    double dvals[NVALS], dvals_in[NVALS];

    float fnan = 0.f/0.f;
    double dnan = 0.0/0.0;
    float fpinf = 1.0f/0.0f;
    float fninf = -fpinf;
    double dpinf = 1.0/0.0;
    double dninf = -dpinf;
    nc_type att_type;
    size_t att_len;
    float att_fvals[NVALS];
    double att_dvals[NVALS];

    printf("\n*** Testing NaN\n");
    printf("*** creating NaN test file %s...", FILE8_NAME);
    if (nc_create(FILE8_NAME, NC_CLOBBER, &ncid)) ERR;
    
    if (nc_def_dim(ncid, DIM_NAME, NVALS, &dimid)) ERR;
    
    if (nc_def_var(ncid, F_NAME, NC_FLOAT, NDIMS, &dimid, &fvarid)) ERR;
    if (nc_def_var(ncid, D_NAME, NC_DOUBLE, NDIMS, &dimid, &dvarid)) ERR;

    fvals[0] = fninf;
    fvals[1] = fnan;
    fvals[2] = fpinf;
    dvals[0] = dninf;
    dvals[1] = dnan;
    dvals[2] = dpinf;
    
    /* Create float and double attributes */
    if (nc_put_att_float(ncid, fvarid, FV_NAME, NC_FLOAT, FV_NVALS, &fnan)) ERR;
    if (nc_put_att_float(ncid, fvarid, ATT_NAME, NC_FLOAT, NVALS, fvals)) ERR;
    if (nc_put_att_double(ncid, dvarid, FV_NAME, NC_DOUBLE, FV_NVALS, &dnan)) ERR;
    if (nc_put_att_double(ncid, dvarid, ATT_NAME, NC_DOUBLE, NVALS, dvals)) ERR;
    
    if (nc_enddef(ncid)) ERR;
   
    /* Write float and double data */
   if (nc_put_var_float(ncid, fvarid, fvals)) ERR;
   if (nc_put_var_double(ncid, dvarid, dvals)) ERR;
   
   if (nc_close(ncid)) ERR;
   
   /* Check it out. */
   
   /* Reopen the file. */
   if (nc_open(FILE8_NAME, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq_varid(ncid, F_NAME, &fvarid)) ERR;
   if (nc_inq_varid(ncid, D_NAME, &dvarid)) ERR;
   /* Check the values of the float attributes */
   if (nc_inq_att(ncid, fvarid, FV_NAME, &att_type, &att_len)) ERR;
   if (att_type != NC_FLOAT || att_len != FV_NVALS) ERR;
   if (nc_get_att_float(ncid, fvarid, FV_NAME, att_fvals)) ERR;
   if (!isnan(att_fvals[0])) ERR;
   if (nc_get_att_float(ncid, fvarid, ATT_NAME, att_fvals)) ERR;
   if (!(isinf(att_fvals[0]) && att_fvals[0] < 0)) ERR;
   if (!isnan(att_fvals[1])) ERR;
   if (!(isinf(att_fvals[2]) && att_fvals[2] > 0)) ERR;
   /* Check the values of double attributes */
   if (nc_inq_att(ncid, dvarid, FV_NAME, &att_type, &att_len)) ERR;
   if (att_type != NC_DOUBLE || att_len != FV_NVALS) ERR;
   if (nc_get_att_double(ncid, dvarid, FV_NAME, att_dvals)) ERR;
   if (!isnan(att_dvals[0])) ERR;
   if (nc_get_att_double(ncid, dvarid, ATT_NAME, att_dvals)) ERR;
   if (!(isinf(att_dvals[0]) && att_dvals[0] < 0)) ERR;
   if (!isnan(att_dvals[1])) ERR;
   if (!(isinf(att_dvals[2]) && att_dvals[2] > 0)) ERR;
   /* Check values of float data */
   if (nc_get_var_float(ncid, fvarid, fvals_in)) ERR;
   if (!(isinf(fvals_in[0]) && fvals_in[0] < 0)) ERR;
   if (!isnan(fvals_in[1])) ERR;
   if (!(isinf(fvals_in[2]) && fvals_in[2] > 0)) ERR;
   /* Check values of double data */
   if (nc_get_var_double(ncid, dvarid, dvals_in)) ERR;
   if (!(isinf(dvals_in[0]) && dvals_in[0] < 0)) ERR;
   if (!isnan(dvals_in[1])) ERR;
   if (!(isinf(dvals_in[2]) && dvals_in[2] > 0)) ERR;

   if (nc_close(ncid)) ERR;
   
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
