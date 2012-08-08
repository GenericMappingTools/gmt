/** \file 

This program benchmarks creating a netCDF file and reading records.

Copyright 2011, UCAR/Unidata See COPYRIGHT file for copying and
redistribution conditions.
*/

#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */

/* We will create this file. */
#define FILE_NAME "bm_netcdf4_recs.nc"

int main(int argc, char **argv)
{
    struct timeval start_time, end_time, diff_time;
    double sec;
    int nitem = 10000;		/* default number of objects of each type */
    int i;
    int ncid;
    int data[] = {42};
    int g, grp, numgrp;
    char gname[16];
    int v, var, numvar, vn, vleft, nvars;
    
    int  stat;  /* return status */

    /* dimension ids */
    int basetime_dim;
    int forecast_dim;
    int bounds_dim;
    int latitude_dim;
    int longitude_dim;

    /* dimension lengths */
    size_t basetime_len = NC_UNLIMITED;
    size_t forecast_len = 32;
    size_t bounds_len = 2;
    size_t latitude_len = 121;
    size_t longitude_len = 101;

    /* variable ids */
    int temperature_2m_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_temperature_2m 4

    /* variable shapes */
    int temperature_2m_dims[RANK_temperature_2m];
    static const float temperature_2m_FillValue_att[1] = {9.96921e+36} ;
    static const float temperature_2m_missing_value_att[1] = {9.96921e+36} ;
    static const float temperature_2m_valid_min_att[1] = {180} ;
    static const float temperature_2m_valid_max_att[1] = {330} ;

    /* enter define mode */
    if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;

    /* define dimensions */
    if (nc_def_dim(ncid, "basetime", basetime_len, &basetime_dim)) ERR;
    if (nc_def_dim(ncid, "forecast", forecast_len, &forecast_dim)) ERR;
    if (nc_def_dim(ncid, "bounds", bounds_len, &bounds_dim)) ERR;
    if (nc_def_dim(ncid, "latitude", latitude_len, &latitude_dim)) ERR;
    if (nc_def_dim(ncid, "longitude", longitude_len, &longitude_dim)) ERR;

    /* define variables */
    temperature_2m_dims[0] = basetime_dim;
    temperature_2m_dims[1] = forecast_dim;
    temperature_2m_dims[2] = latitude_dim;
    temperature_2m_dims[3] = longitude_dim;
    if (nc_def_var(ncid, "temperature_2m", NC_FLOAT, RANK_temperature_2m, 
		   temperature_2m_dims, &temperature_2m_id)) ERR;

    /* assign per-variable attributes */
    if (nc_put_att_text(ncid, temperature_2m_id, "long_name", 36, "Air temperature 2m above the surface")) ERR;
    if (nc_put_att_text(ncid, temperature_2m_id, "units", 1, "K")) ERR;
    if (nc_put_att_float(ncid, temperature_2m_id, "_FillValue", NC_FLOAT, 1, temperature_2m_FillValue_att)) ERR;
    if (nc_put_att_float(ncid, temperature_2m_id, "missing_value", NC_FLOAT, 1, temperature_2m_missing_value_att)) ERR;
    if (nc_put_att_float(ncid, temperature_2m_id, "valid_min", NC_FLOAT, 1, temperature_2m_valid_min_att)) ERR;
    if (nc_put_att_float(ncid, temperature_2m_id, "valid_max", NC_FLOAT, 1, temperature_2m_valid_max_att)) ERR;
    if (nc_put_att_text(ncid, temperature_2m_id, "standard_name", 15, "air_temperature")) ERR;
    if (nc_put_att_text(ncid, temperature_2m_id, "cell_methods", 10, "area: mean")) ERR;
    if (nc_put_att_text(ncid, temperature_2m_id, "coordinates", 5, "level")) ERR;
    if (nc_close(ncid)) ERR;

    if (gettimeofday(&start_time, NULL)) ERR;
    
    return(0);
}
