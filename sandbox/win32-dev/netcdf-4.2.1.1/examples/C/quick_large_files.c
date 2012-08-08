/* This example program is part of Unidata's netCDF library for
   scientific data access. 

   This program (quickly, but not throughly) tests the large file
   features.

   Ed Hartnett, 8/11/4
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

#define NUMDIMS 1
#define NUMVARS 2
/* This dim len is the max size for an 8-byte type in 64-bit offset
   format: (2*32 - 4) / 8, that is, 4 GB minus a bit, on an 8 byte
   boundary. */
#define DIM_LEN 536870911 

int
main()
{
   int ncid, spockid, kirkid, dimids[NUMDIMS];
   double val_in, val_out = 999.99;
   size_t index[NUMDIMS] = {DIM_LEN-1};
   int i, res; 

   /* Create the netCDF 64-bit offset format file. */
   if ((res = nc_create("example.nc", NC_CLOBBER|NC_64BIT_OFFSET, &ncid)))
      BAIL(res);

   /* Turn off fill mode to speed things up. */
   if ((res = nc_set_fill(ncid, NC_NOFILL, NULL)))
       BAIL(res);

   /* Define dimension. */
   if ((res = nc_def_dim(ncid, "longdim", DIM_LEN, dimids)))
      BAIL(res);
   
   /* Define two variables. */
   if ((res = nc_def_var(ncid, "spock", NC_DOUBLE, NUMDIMS, 
			 dimids, &spockid)))
      BAIL(res);
   if ((res = nc_def_var(ncid, "kirk", NC_DOUBLE, NUMDIMS, 
			 dimids, &kirkid)))
      BAIL(res);
   
   /* We're finished defining metadata. */
   if ((res = nc_enddef(ncid)))
      BAIL(res);

   if ((res = nc_put_var1_double(ncid, kirkid, index, &val_out)))
      BAIL(res);

   /* We're done! */
   if ((res = nc_close(ncid)))
      BAIL(res);

   return 0;
}






