/*! \file
Read a simple file, with some of the features of netCDF-4.

This is a very simple example which demonstrates some of the new
features of netCDF-4.0.

This example reads a simple file created by simple_nc4_wr.c. This is
intended to illustrate the use of the netCDF-4 C API.

This is part of the netCDF package. Copyright 2006-2011 University
Corporation for Atmospheric Research/Unidata. See COPYRIGHT file for
conditions of use. Full documentation of the netCDF can be found at
http://www.unidata.ucar.edu/software/netcdf/docs.
*/
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

/* This is the name of the data file we will read. */
#define FILE_NAME "simple_nc4.nc"

/* We are reading 2D data, a 6 x 12 grid. */
#define NX 6
#define NY 12

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

int
main()
{
   /* There will be netCDF IDs for the file, each group, and each
    * variable. */
   int ncid, varid1, varid2, grp1id, grp2id;

   unsigned long long data_in[NX][NY];

   /* Loop indexes, and error handling. */
   int x, y, retval;

   /* The following struct is written as a compound type. */
   struct s1 
   {
         int i1;
         int i2;
   };
   struct s1 compound_data[NX][NY];

   /* Open the file. NC_NOWRITE tells netCDF we want read-only access
    * to the file.*/
   if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
      ERR(retval);

   /* Get the group ids of our two groups. */
   if ((retval = nc_inq_ncid(ncid, "grp1", &grp1id)))
      ERR(retval);
   if ((retval = nc_inq_ncid(ncid, "grp2", &grp2id)))
      ERR(retval);

   /* Get the varid of the uint64 data variable, based on its name, in
    * grp1. */
   if ((retval = nc_inq_varid(grp1id, "data", &varid1))) 
      ERR(retval);

   /* Read the data. */
   if ((retval = nc_get_var_ulonglong(grp1id, varid1, &data_in[0][0])))
      ERR(retval);

   /* Get the varid of the compound data variable, based on its name,
    * in grp2. */
   if ((retval = nc_inq_varid(grp2id, "data", &varid2))) 
      ERR(retval);

   /* Read the data. */
   if ((retval = nc_get_var(grp2id, varid2, &compound_data[0][0])))
      ERR(retval);

   /* Check the data. */
   for (x = 0; x < NX; x++)
      for (y = 0; y < NY; y++)
      {
         if (data_in[x][y] != x * NY + y ||
             compound_data[x][y].i1 != 42 ||
             compound_data[x][y].i2 != -42)
            return ERRCODE;
      }

   /* Close the file, freeing all resources. */
   if ((retval = nc_close(ncid)))
      ERR(retval);

   printf("*** SUCCESS reading example file %s!\n", FILE_NAME);
   return 0;
}
