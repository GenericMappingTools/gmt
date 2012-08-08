/* This example program is part of Unidata's netCDF library for
   scientific data access. 

   This program demonstrates various ways to create netCDF dimensions
   and variables.

   We will create a dataset with 4 variables. We'll store a 3D surface
   temperature (lat x lon x timestep), 4D pressure (lat x lon x height x
   timestep), a 2D initial temperature (lat x lon) and a 3D initial
   pressure (lat x lon x height). All variables will be stored as
   single precision floating point.

   All variables are intended to share dimensions. For example, the
   latitude axis is the same for all of them.

   We'll also include the coordinate axis data for three of the four
   dimensions, that is, labels for the lat, lon, and height axes.

   Finally, we'll use some attributes to store some metadata about the
   variables, the units. Also we'll use a file-level, or global,
   attribute to record some information about the dataset as a whole.

   Ed Hartnett, 6/3/4
   $Id$
*/

#include <netcdf.h>
#include <stdio.h>
#include <string.h>

/* This macro handles errors by outputting a message to stdout and
   then exiting. */
#define NC_EXAMPLE_ERROR 2 /* This is the exit code for failure. */
#define BAIL(e) do { \
printf("Bailing out in file %s, line %d, error:%s.\n", \
__FILE__, __LINE__, nc_strerror(e)); \
return NC_EXAMPLE_ERROR; \
} while (0) 

#define FILENAME "test.nc"
#define NUMDIMS 4
#define NUMVARS 4

/* We'll need to specify the dimensionality of our netCDF
   variables. These will allow us to do so with less confusion. */
#define NUMDIMS_1D 1
#define NUMDIMS_2D 2
#define NUMDIMS_3D 3
#define NUMDIMS_4D 4

/* The following will be the variable names in the netCDF file. */
#define TEMP_VARNAME "sfc_temp"
#define PRES_VARNAME "pressure"
#define INIT_TEMP_VARNAME "initial_temp"
#define INIT_PRES_VARNAME "initial_pressure"

/* These will be names and values of attributes. */
#define UNITS "units"
#define CELSIUS "celsius"
#define MILIBARS "milibars"
#define HISTORY "history"
#define HISTSTR "These data were produced by a thousand typing monkeys"

/* These will be the names of the dimensions, and also the names of
   the variables that will hold the coordinate axis labels. */
#define TIME_NAME "timestep"
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define HEIGHT_NAME "height"

/* These are the lengths of each dimension. */
#define LAT_LEN 7
#define LON_LEN 10
#define HEIGHT_LEN 5
#define TIME_LEN 3

/* These will be used to create some phoney data. */
#define PHONEY_LON_START (-120.)
#define PHONEY_LON_MULT (.5)
#define PHONEY_LAT_START (40.)
#define PHONEY_LAT_MULT (2.5)
#define PHONEY_HEIGHT_INC 100
#define PHONEY_TEMP_START (10.5)
#define PHONEY_TEMP_DIV 20
#define PHONEY_PRES_START (1013.0)
#define PHONEY_PRES_DIV 1

int
main()
{
   /* NetCDF IDs for the file, dimensions, and variables. We define
      three extra varids for the coordinate axis data. */
   int ncid, time_dimid, lat_dimid, lon_dimid, height_dimid;
   int temp_varid, pres_varid, itemp_varid, ipres_varid;
   int lat_varid, lon_varid, height_varid;

   /* We'll reuse this array every time we define a variable. */
   int dimids[NUMDIMS];

   /* Coordinate axis data (ex. what specific longitude values do we
      have data for?) */
   float lat[LAT_LEN], lon[LON_LEN];
   int height[HEIGHT_LEN];

   /* Phoney variable data. In the real world, the temp and pres
      variables might be the output of a model or assimilation system,
      and the itemp and ipres initial values of the temp and pres
      variables. For temp and pres we will reserve enough memory to
      store one timestep, and keep using the same timestep's worth of
      data over and over again. */
   float temp[LAT_LEN][LON_LEN];
   float itemp[LAT_LEN][LON_LEN];
   float pres[LAT_LEN][LON_LEN][HEIGHT_LEN];
   float ipres[LAT_LEN][LON_LEN][HEIGHT_LEN];

   /* We need these to specify the array subsections we'll be writing
      in the temp and pres variables. */
   size_t start[NUMDIMS], count[NUMDIMS];

   float *fp, *ifp;
   int rec, i, res; 

   /* Create a bunch of phoney data so we have something to write in
      the example file. */
   for (i=0; i<LON_LEN; i++)
      lon[i] = PHONEY_LON_START + (i * PHONEY_LON_MULT);
   for (i=0; i<LAT_LEN; i++)
      lat[i] = PHONEY_LAT_START + (i * PHONEY_LAT_MULT);
   for (i=0; i<HEIGHT_LEN; i++)
      height[i] = i * PHONEY_HEIGHT_INC;
   for (fp=(float *)temp, ifp=(float *)itemp, i=0; 
	i<LAT_LEN*LON_LEN; i++)
      *fp++ = (*ifp++ = PHONEY_TEMP_START) + i/PHONEY_TEMP_DIV;
   for (fp=(float *)pres, ifp=(float *)ipres, i=0; 
	i<LAT_LEN*LON_LEN*HEIGHT_LEN; i++)
      *fp++ = (*ifp++ = PHONEY_PRES_START) - i/PHONEY_PRES_DIV;

   /* Create the netCDF file. */
   if ((res = nc_create(FILENAME, NC_CLOBBER, &ncid)))
      BAIL(res);

   /* Store a history string as a file-level attribute. This could
      store the name of the person or lab producing the data, or any
      information that applies to the dataset as a whole. */
   if ((res = nc_put_att_text(ncid, NC_GLOBAL, HISTORY, 
			      strlen(HISTSTR), HISTSTR)))
      BAIL(res);

   /* Define dimensions. The functions will return a dimension ID to
      the appropriate dim ID variable for later reference. Note that
      we define the time dimension with a length of NC_UNLIMITED. This
      means we can arbitrarily add timesteps to variables with a time
      dimension. */
   if ((res = nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &time_dimid)))
      BAIL(res);
   if ((res = nc_def_dim(ncid, LAT_NAME, LAT_LEN, &lat_dimid)))
      BAIL(res);
   if ((res = nc_def_dim(ncid, LON_NAME, LON_LEN, &lon_dimid)))
      BAIL(res);
   if ((res = nc_def_dim(ncid, HEIGHT_NAME, HEIGHT_LEN, 
			 &height_dimid)))
      BAIL(res);
   
   /* Define the variables. Each variable's dimension ids are set in
      the dimids array before the call to nc_def_var. The first two
      variables share three dimensions, time, lat and lon. */
   dimids[0] = time_dimid;
   dimids[1] = lat_dimid;
   dimids[2] = lon_dimid;
   if ((res = nc_def_var(ncid, TEMP_VARNAME, NC_FLOAT, NUMDIMS_3D, 
			 dimids, &temp_varid)))
      BAIL(res);
   
   /* For pressure we also add a 4th dimension: height. */
   dimids[3] = height_dimid;
   if ((res = nc_def_var(ncid, PRES_VARNAME, NC_FLOAT, NUMDIMS_4D, 
			 dimids, &pres_varid)))
      BAIL(res);

   /* The initial temp and pressure are not unlimited dimensions, and
      they don't have a time dimension. */
   dimids[0] = lat_dimid;
   dimids[1] = lon_dimid;
   if ((res = nc_def_var(ncid, INIT_TEMP_VARNAME, NC_FLOAT, 
			 NUMDIMS_2D, dimids, &itemp_varid)))
      BAIL(res);
   dimids[2] = height_dimid;
   if ((res = nc_def_var(ncid, INIT_PRES_VARNAME, NC_FLOAT, 
			 NUMDIMS_3D, dimids, &ipres_varid)))
      BAIL(res);

   /* We'll store the units as attributes for the temperature and
      pressure variables. */
   if ((res = nc_put_att_text(ncid, pres_varid, UNITS, 
			      strlen(MILIBARS), MILIBARS)))
      BAIL(res);
   if ((res = nc_put_att_text(ncid, temp_varid, UNITS, 
			      strlen(CELSIUS), CELSIUS)))
      BAIL(res);
   

   /* In order to store the coordinate axis data we need to define a
      netCDF variable for each axis we want to store data for. In our
      case, we'll store latitudes, longitudes, and heights. We'll
      leave the time dimension as unitless timesteps, with no
      coordinate data specifying what exact times these data apply
      for. In each case, by netCDF convention, the variable name will
      be the same as the dimension, it will be one-dimensional, with
      the length of the dimension. As it's dimension, it will be
      assigned the dimension that it represents. */
   if ((res = nc_def_var(ncid, LAT_NAME, NC_FLOAT, NUMDIMS_1D, 
			 &lat_dimid, &lat_varid)))
      BAIL(res);
   if ((res = nc_def_var(ncid, LON_NAME, NC_FLOAT, NUMDIMS_1D, 
			 &lon_dimid, &lon_varid)))
      BAIL(res);
   if ((res = nc_def_var(ncid, HEIGHT_NAME, NC_INT, NUMDIMS_1D, 
			 &height_dimid, &height_varid)))
      BAIL(res);

   /* We're finished defining metadata. We have to call nc_enddef
      before writing any data. */
   if ((res = nc_enddef(ncid)))
      BAIL(res);

   /* Write out our coordinate axis data for lat, lon, and height. */
   if ((res = nc_put_var_float(ncid, lat_varid, lat)))
      BAIL(res);
   if ((res = nc_put_var_float(ncid, lon_varid, lon)))
      BAIL(res);
   if ((res = nc_put_var_int(ncid, height_varid, height)))
      BAIL(res);

   /* Write out the two fixed-size variables. Since these were not
      defined with an unlimited dimension, they cannot be changed in
      size in the file. We'll write each of them out all at once. */
   if ((res = nc_put_var_float(ncid, itemp_varid, (float *)itemp)))
      BAIL(res);
   if ((res = nc_put_var_float(ncid, ipres_varid, (float *)ipres)))
      BAIL(res);

   /* Let's simulate the action of some data-producing system by
      writing some records of temp and pres data, one record at a
      time. In the real world, we wouldn't be writing the same data
      over and over again for every timestep, and we need not know in
      advance how many records there will be. */

   /* The start and count array specify the array we want to
      write. Specify the information in the same order in which you
      defined the dimensions. */
   start[1] = 0;
   start[2] = 0;
   start[3] = 0;
   count[0] = 1; 
   count[1] = LAT_LEN;
   count[2] = LON_LEN;
   count[3] = HEIGHT_LEN;

   for (rec=0; rec<TIME_LEN; rec++)
   {
      /* To write just one record of data, we'll use the
	 nc_put_vara_<type> function, which can write any array subset
	 of the variable. */
      start[0] = rec; /* We want to write *this* record. */
      if ((res = nc_put_vara_float(ncid, temp_varid, start, count, 
				   (float *)temp)))
	 BAIL(res);
      if ((res = nc_put_vara_float(ncid, pres_varid, start, count,
				   (float *)pres)))
	 BAIL(res);
      /* We're done writing one record. */
   } 

   /* Close the file. We're done, so we can go out and see the new
      Harry Potter movie! */
   if ((res = nc_close(ncid)))
      BAIL(res);

   return 0;
}

