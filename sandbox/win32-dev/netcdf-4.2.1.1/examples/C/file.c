/* This example program is part of Unidata's netCDF library for
   scientific data access. 

   This program demonstrates various ways to create a netCDF file,
   open an existing file, and close a file.

   Ed Hartnett, 5/29/4
   $Id$
*/

#include <netcdf.h>
#include <stdio.h>


/* This macro handles errors by outputting a message to stdout and
   then exiting. */
#define BAIL(e) do { \
printf("Bailing out in file %s, line %d, error:%s.\n", \
__FILE__, __LINE__, nc_strerror(e)); \
return e; \
} while (0) 

#define FILENAME "test.nc"
#define VARNAME "var1"
#define DIMNAME "d1"
#define NUMDIMS 1
#define DIMLEN 10
#define ERROR 2 /* exit code for example failure */

int
main()
{
   /* These are netCDF IDs for file, dimension, and variable. */
   int ncid, dimid, varid;   

   /* This array will hold one ID for each dimension in the
      variable, in this case, one. */
   int dimids[NUMDIMS];

   /* This is some one-dimensional phoney data to write and read. */
   int data_out[] = {0,1,2,3,4,5,6,7,8,9};
   int data_in[DIMLEN];

   int i, res; 

   /* Create a classic format netCDF file, overwritting any file of
      this name that may already exist. */
   if ((res = nc_create(FILENAME, NC_CLOBBER, &ncid)))
      BAIL(res);

   /* Define a dimension. The functions will return a dimension ID to
      dimid. */
   if ((res = nc_def_dim(ncid, DIMNAME, DIMLEN, &dimid)))
      BAIL(res);

   /* Define a variable. First we must specify which dimensions this
      variable uses, by adding their dimension IDs to the dimids array
      that we pass into nc_def_var. In this example, there is just one
      dimension. */
   dimids[0] = dimid;
   if ((res = nc_def_var(ncid, VARNAME, NC_INT, NUMDIMS, dimids, &varid)))
      BAIL(res);

   /* The enddef function tells the library that we are done with
      defining metadata in the newly-created file, and now want to
      write some data. */
   if ((res = nc_enddef(ncid)))
      BAIL(res);

   /* Write our phoney integer data. Since we've already specified the
      shape of this variable, we only need to provide a pointer to the
      start of the data. */
   if ((res = nc_put_var_int(ncid, varid, data_out)))
      BAIL(res);

   /* Close the file. This flushes all buffers and frees any resources
      associated with the file. We are closing the file here so that
      we can demonstrate nc_open. Usually we wouldn't close the file
      until done with it. */
   if ((res = nc_close(ncid)))
      BAIL(res);

   /* Now open the file for read-only access. */
   if ((res = nc_open(FILENAME, NC_NOWRITE, &ncid)))
      BAIL(res);

   /* Find the varid that represents our data. */
   if ((res = nc_inq_varid(ncid, VARNAME, &varid)))
      BAIL(res);

   /* Read the data, all in one lump. */
   if ((res = nc_get_var_int(ncid, varid, data_in)))
      BAIL(res);

   /* Ensure we got the data we expected. */
   for (i=0; i<DIMLEN; i++)
      if (data_in[i] != data_out[i])
      {
	 fprintf(stderr, "Unexpected value!\n");
	 return ERROR;
      }
   
   /* Close the file again. */
   if ((res = nc_close(ncid)))
      BAIL(res);

   return 0;
}

