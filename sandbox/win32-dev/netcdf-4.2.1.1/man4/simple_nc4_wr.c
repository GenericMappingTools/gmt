/* This is part of the netCDF-4 package. Copyright 2007 University
   Corporation for Atmospheric Research/Unidata.  See COPYRIGHT file
   for conditions of use.

   This is a very simple example which demonstrates some of the
   new features of netCDF-4.0.

   We create two shared dimensions, "x" and "y", in a parent group,
   and some netCDF variables in different subgroups. The variables
   will include a compound and an enum type, as well as some of the
   new atomic types, like the unsigned 64-bit integer.

   This example demonstrates the netCDF-4 C API. This is part of the
   netCDF tutorial, which can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial.html

   To understand this example program, users should have a good
   understanding of the netCDF-3 API. See the example program
   simple_xy_wr.c for a netCDF-3 example.

   Full documentation of the netCDF-4 C API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c.html

   $Id$
*/

#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

/* This is the name of the data file we will create. */
#define FILE_NAME "simple_nc4.nc"

/* We are writing 2D data, a 6 x 12 grid. */
#define NDIMS 2
#define NX 6
#define NY 12

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) @{printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);@}

int
main()
@{
   /* When we create netCDF variables, groups, dimensions, or types,
    * we get back an ID for each one. */
   int ncid, x_dimid, y_dimid, varid1, varid2, grp1id, grp2id, typeid;
   int dimids[NDIMS];

   /* This is the data array we will write. It will be filled with a
    * progression of numbers for this example. */
   unsigned long long data_out[NX][NY];

   /* Loop indexes, and error handling. */
   int x, y, retval;

   /* The following struct is written as a compound type. */
   struct s1 
   @{
         int i1;
         int i2;
   @};
   struct s1 compound_data[NX][NY];

   /* Create some pretend data. */
   for (x = 0; x < NX; x++)
      for (y = 0; y < NY; y++)
      @{
         data_out[x][y] = x * NY + y;
         compound_data[x][y].i1 = 42;
         compound_data[x][y].i2 = -42;
      @}

   /* Create the file. The NC_NETCDF4 flag tells netCDF to
    * create a netCDF-4/HDF5 file.*/
   if ((retval = nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)))
      ERR(retval);

   /* Define the dimensions in the root group. Dimensions are visible
    * in all subgroups. */
   if ((retval = nc_def_dim(ncid, "x", NX, &x_dimid)))
      ERR(retval);
   if ((retval = nc_def_dim(ncid, "y", NY, &y_dimid)))
      ERR(retval);

   /* The dimids passes the IDs of the dimensions of the variable. */
   dimids[0] = x_dimid;
   dimids[1] = y_dimid;

   /* Define two groups, "grp1" and "grp2." */
   if ((retval = nc_def_grp(ncid, "grp1", &grp1id)))
      ERR (retval);
   if ((retval = nc_def_grp(ncid, "grp2", &grp2id)))
      ERR (retval);

   /* Define an unsigned 64bit integer variable in grp1, using dimensions
    * in the root group. */
   if ((retval = nc_def_var(grp1id, "data", NC_UINT64, NDIMS, 
                            dimids, &varid1)))
      ERR(retval);

   /* Write unsigned long long data to the file. For netCDF-4 files,
    * nc_enddef will be called automatically. */
   if ((retval = nc_put_var_ulonglong(grp1id, varid1, &data_out[0][0])))
      ERR(retval);

   /* Create a compound type. This will cause nc_reddef to be called. */
   if (nc_def_compound(grp2id, sizeof(struct s1), "sample_compound_type", 
                       &typeid))
      ERR(retval);
   if (nc_insert_compound(grp2id, typeid, "i1", 
                          offsetof(struct s1, i1), NC_INT))
      ERR(retval);
   if (nc_insert_compound(grp2id, typeid, "i2", 
                          offsetof(struct s1, i2), NC_INT))
      ERR(retval);

   /* Define a compound type variable in grp2, using dimensions
    * in the root group. */
   if ((retval = nc_def_var(grp2id, "data", typeid, NDIMS, 
                            dimids, &varid2)))
      ERR(retval);

   /* Write the array of struct to the file. This will cause nc_endef
    * to be called. */
   if ((retval = nc_put_var(grp2id, varid2, &compound_data[0][0])))
      ERR(retval);

   /* Close the file. */
   if ((retval = nc_close(ncid)))
      ERR(retval);

   printf("*** SUCCESS writing example file simple_nc4.nc!\n");
   return 0;
@}
