/* This example program is part of Unidata's netCDF library for
   scientific data access. 

   How about a short, but meaningful, netCDF program?

   Ed Hartnett, 6/19/4
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

#define NUMDIMS 2
#define NUMVARS 1
#define CELSIUS "celsius"
#define LAT_LEN 3
#define LON_LEN 2

int
main()
{
   int ncid, temp_varid, dimids[NUMDIMS];
   float temp[LAT_LEN][LON_LEN], *fp;
   int i, res; 

   /* Create a bunch of phoney data so we have something to write in
      the example file. */
   for (fp=(float *)temp, i=0; i<LAT_LEN*LON_LEN; i++)
      *fp++ = 10. + i/10.;

   /* Create the netCDF file. */
   if ((res = nc_create("short.nc", NC_CLOBBER, &ncid)))
      BAIL(res);

   /* Define dimensions. */
   if ((res = nc_def_dim(ncid, "latitude", LAT_LEN, dimids)))
      BAIL(res);
   if ((res = nc_def_dim(ncid, "longitude", LON_LEN, &dimids[1])))
      BAIL(res);
   
   /* Define the variable. */
   if ((res = nc_def_var(ncid, "sfc_temp", NC_FLOAT, NUMDIMS, 
			 dimids, &temp_varid)))
      BAIL(res);
   
   /* We'll store the units. */
   if ((res = nc_put_att_text(ncid, temp_varid, "units", 
			      strlen(CELSIUS), CELSIUS)))
      BAIL(res);

   /* We're finished defining metadata. */
   if ((res = nc_enddef(ncid)))
      BAIL(res);

   if ((res = nc_put_var_float(ncid, temp_varid, (float *)temp)))
      BAIL(res);

   /* We're done! */
   if ((res = nc_close(ncid)))
      BAIL(res);

   return 0;
}

