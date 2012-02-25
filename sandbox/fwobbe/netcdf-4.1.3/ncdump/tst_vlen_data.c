/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with a vlen type and vlen data for ncdump to read.

   $Id$
*/

#include <nc_tests.h>
#include <netcdf.h>
#include <stdlib.h>

#define FILE5_NAME "tst_vlen_data.nc"
#define TYPE5_NAME "row_of_floats"
#define TYPE5_TYPE NC_FLOAT
#define DIM5_NAME "m"
#define DIM5_LEN 5
#define VAR5_NAME "ragged_array"
#define VAR5_RANK 1
#define ATT5_NAME "_FillValue"
#define ATT5_LEN  1
#define NROWS 5

int
main(int argc, char **argv)
{
   int ncid;
   int dimid, varid;
   nc_type typeid;
   char name_in[NC_MAX_NAME+1];
   nc_type base_typeid;
   size_t base_size_in;
   int class_in;
   float value_in;

   int i, j;

   int var_dims[VAR5_RANK];
   float **array;		/* a ragged array */
   nc_vlen_t ragged_data[DIM5_LEN];
   float missing_value = -999.0;
   nc_vlen_t missing_val;
   nc_vlen_t val_in;

   printf("\n*** Testing vlens.\n");
   printf("*** creating vlen test file %s...", FILE5_NAME);
   if (nc_create(FILE5_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

   /* Create a vlen type. */
   if (nc_def_vlen(ncid, TYPE5_NAME, TYPE5_TYPE, &typeid)) ERR;

   /* Declare a dimension for number of rows */
   if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimid)) ERR;

   /* Declare a variable of the vlen type */
   var_dims[0] = dimid;
   if (nc_def_var(ncid, VAR5_NAME, typeid, VAR5_RANK, var_dims, &varid)) ERR;

   /* Create and write a variable attribute of the vlen type */
   missing_val.p = &missing_value;
   missing_val.len = 1;
   if (nc_put_att(ncid, varid, ATT5_NAME, typeid, ATT5_LEN, (void *) &missing_val)) ERR;
   if (nc_enddef(ncid)) ERR;

   /* fill in pointers to data rows in preparation for writing */
   array = (float **) malloc(NROWS * sizeof(float *));
   if(array == NULL) ERR;
   for (i = 0; i < NROWS; i++) {
       int ncolumns = NROWS - i;
       array[i] = (float *) malloc(ncolumns * sizeof(float));
       if(array[i] == NULL) ERR;
       for (j = 0; j < ncolumns; j++) {
	   array[i][j] = 10.0 * (i + 1) + j;
       }
   }
   array[4][0] = missing_value; /* overwrite last row with missing for equality test */
   
   for (i = 0; i < DIM5_LEN; i++) {
       ragged_data[i].p = array[i];
       ragged_data[i].len = NROWS - i;
   }

   /* Store data, writing all values of the ragged matrix in one call */
   if(nc_put_var(ncid, varid, ragged_data)) ERR;

   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Check it out. */
   
   /* Reopen the file. */
   if (nc_open(FILE5_NAME, NC_NOWRITE, &ncid)) ERR;

   /* Get info with the generic inquire for user-defined types */
   if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_typeid, 
			NULL, &class_in)) ERR;
   if (strcmp(name_in, TYPE5_NAME) || 
       base_size_in != sizeof(nc_vlen_t) ||
       base_typeid != NC_FLOAT ||
       class_in != NC_VLEN) ERR;

   /* Get the same info with the vlen-specific inquire function */
   if (nc_inq_vlen(ncid, typeid, name_in, &base_size_in, &base_typeid)) ERR;
   if (strcmp(name_in, TYPE5_NAME) || 
       base_size_in != sizeof(nc_vlen_t) ||
       base_typeid != NC_FLOAT) ERR;

   if (nc_inq_varid(ncid, VAR5_NAME, &varid)) ERR;

   /* Read in attribute value and check it */
   if (nc_get_att(ncid, varid, ATT5_NAME, &val_in)) ERR;
   if (val_in.len != ATT5_LEN) ERR;
   value_in = *(float *)val_in.p;
   if (value_in != missing_value) ERR;
   /* Free allocated space for attribute value when finished with it */
   if (nc_free_vlen(&val_in)) ERR;

   /* Read in each row, check its length and values */
   for (i = 0; i < DIM5_LEN; i++) {
       size_t index[VAR5_RANK];
       float *fvals;
       index[0] = i;
       if (nc_get_var1(ncid, varid, index, (void *) &val_in)) ERR;
       if (val_in.len != NROWS - i) ERR;
       fvals = (float *)val_in.p;
       for (j = 0; j < val_in.len; j++) {
	   if (fvals[j] != array[i][j] ) ERR;
       }
       if (nc_free_vlen(&val_in)) ERR;
   }

   /* Now read in all the rows at once, then check lengths and values */
   {
       nc_vlen_t vals_in[DIM5_LEN];
       float *fvals;
       size_t start[VAR5_RANK], count[VAR5_RANK];
       start[0] = 0;
       count[0] = NROWS;
       if (nc_get_vara(ncid, varid, start, count, vals_in)) ERR;
       for (i = 0; i < DIM5_LEN; i++) {
	   for (j = 0; j < vals_in[i].len; j++) {
	       fvals = (float *)vals_in[i].p;
	       if (fvals[j] != array[i][j] ) ERR;
	   }
	   if (nc_free_vlen(&vals_in[i])) ERR;
       }
   }


   if (nc_close(ncid)) ERR; 
   
   /* Free space used for sample data. */
   for (i = 0; i < NROWS; i++) 
      free(array[i]);
   free(array);
      
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

