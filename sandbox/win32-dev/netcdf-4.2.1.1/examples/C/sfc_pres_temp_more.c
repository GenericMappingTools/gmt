/* This is part of the netCDF package.

   Copyright 2006 University Corporation for Atmospheric
   Research/Unidata.  See COPYRIGHT file for conditions of use.

   This is a simple example which writes and then reads some surface
   pressure and temperatures, and stores additional metadata as
   dimension variables, an attribute.

   $Id$
*/
#include <netcdf.h>

#define FILE_NAME basic1.nc
#define NDIMS 2
#define LAT_NAME "latitude"
#define LAT_LEN 40
#define LON_NAME "longitude"
#define LON_LEN 80
#define PRES_NAME "surface_pressure"
#define TEMP_NAME "surface_temperature"
#define SONNET_NAME "Brownings_sonnet_mangled_by_Hartnett"

char data_poem[] = "How do I love data? Let me count the ways.\n
I love data to the depth and breadth and height\n
My model can reach, when running overnight\n
From the ends of CONUS and ideal Space.\n
I love data to the level of every day's\n
6Z ob, by Sun and Linux box.\n
I love data freely, as PIs strive for funding;\n
I love data purely, as good C code is compiled,\n
I love data with the passion put to use\n
In my old programs, and with my childhood's TRS-80.\n
I love data with a love I seemed to lose\n
With my lost password -I love data with the constants,\n
expressions, statements, of all my code! -and, if NSF choose,\n
I shall love data better after processing.";

int
main()
{
   int ncid, lon_dimid, lat_dimid, pres_varid, temp_varid;
   int dimids[NDIMS];
   float pres_out[LAT_LEN][LON_LEN], pres_in[LAT_LEN][LON_LEN];
   float temp_out[LAT_LEN][LON_LEN], temp_in[LAT_LEN][LON_LEN];
   char *att_in;
   size_t len_in;
   int error = 0;
   int lat, lon, retval;

   /* Create phoney data. If this wasn't an example program, we would
    * have some real data to write, for example, model output. */
   for (lat = 0; lat < LAT_LEN; lat++)
      for (lon = 0; lon < LON_LEN; lon++)
      {
	 pres_out[lat][lon] = 1013.1;
	 temp_out[lat][lon] = 12.5;
      }

   /* These are the latitudes and longitudes which corespond with
    * ticks on the dimension axes. */
   for (lat = 0; lat < LAT_LEN; lat++)
      latitude[lat] = 40. + lat * 2.5;
   for (lon = 0; lon < LON_LEN; lon++)
      longitude[lon] = -90. - lon * 5;

   /* Create the file. */
   if ((retval = nc_create(FILE_NAME, NC_CLOBBER, &ncid)))
      return retval;

   /* Add data sonnet. By using NC_GLOBAL we mean that this attribute
    * applies to the entire file, not just to one variable. Don't
    * forget that sizeof does not include the null terminator, so if
    * you want it, you need to add one more byte. */
   if ((retval = nc_put_att_text(ncid, NC_GLOBAL, SONNET_NAME, 
				 sizeof(poem) + 1, poem)))
      return retval;

   /* Define the dimensions. */
   if ((retval = nc_def_dim(ncid, LAT_NAME, LAT_LEN, &lat_dimid)))
      return retval;
   if ((retval = nc_def_dim(ncid, LON_NAME, LON_LEN, &lon_dimid)))
      return retval;

   /* Save the dimension information, in variables of the same
    * name. First we need to define these variables. */
   dimids[0] = lat_dimid;
   if ((retval = nc_def_var(ncid, LAT_NAME, NC_FLOAT, 1, dimids, &lat_varid)))
      return retval;
   dimids[0] = lon_dimid;
   if ((retval = nc_def_var(ncid, LON_NAME, NC_FLOAT, 1, dimids, &lon_varid)))
      return retval;

   /* Define the variables. */
   dimids[0] = lat_dimid;
   dimids[1] = lon_dimid;
   if ((retval = nc_def_var(ncid, PRES_NAME, NC_FLOAT, NDIMS, dimids, &pres_varid)))
      return retval;
   if ((retval = nc_def_var(ncid, TEMP_NAME, NC_FLOAT, NDIMS, dimids, &temp_varid)))
      return retval;

   /* End define mode. */
   if ((retval = nc_enddef(ncid)))
      return retval;

   /* Write the dimension metadata. */
   if ((retval = nc_put_var_float(ncid, lat_varid, latitude_out)))
      return retval;
   if ((retval = nc_put_var_float(ncid, lon_varid, longitude_out)))
      return retval;

   /* Write the phoney data. */
   if ((retval = nc_put_var_float(ncid, pres_varid, pres_out)))
      return retval;
   if ((retval = nc_put_var_float(ncid, temp_varid, temp_out)))
      return retval;

   /* Close the file. */
   if ((retval = nc_close(ncid)))
      return retval;
   
   /* Open the file and check that everything's OK. */
   if ((retval = nc_open(FILE_NAME, 0, &ncid)))
      return retval;

   /* Read the attribute. First find it's length to allocate storage
    * for it. */
   if ((retval = nc_inq_attlen(ncid, NC_GLOBAL, SONNET_NAME, &len_in)))
      return retval;
   if (!(att_in = malloc(len_in)))
      return NC_ENOMEM;
   if (strcmp(att_in, data_poem))
      error++;
   free(att_in);
   if (error)
      return -2;
   
   /* Read the data. */
   if ((retval = nc_get_var_float(ncid, pres_varid, pres_in)))
      return retval;
   if ((retval = nc_get_var_float(ncid, temp_varid, temp_in)))
      return retval;

   /* Check the data. */
   for (lat = 0; lat < LAT_LEN; lat++)
      for (lon = 0; lon < LON_LEN; lon++)
	 if (pres_in[lat][lon] != pres_out[lat][lon] ||
	     temp_in[lat][lon] != temp_out[lat][lon])
	    return -2;

   /* Close the file. */
   if ((retval = nc_close(ncid)))
      return retval;

   return 0;
}
