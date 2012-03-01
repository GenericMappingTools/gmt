/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with array member of compound type and nested
   compound data for ncdump to read.

   $Id$
*/


#include <nc_tests.h>
#include <netcdf.h>
#include <string.h>

#define FILE9_NAME "tst_comp2.nc"
#define TYPE_NAME "vecmat_t"
#define DIM_NAME "n"
#define DIM_LEN 3
#define VAR_NAME "obs"
#define VAR_RANK 1
#define ATT_NAME "_FillValue"
#define ATT_LEN  1

int
main(int argc, char **argv)
{
   int ncid;
   int dimid, varid;
   nc_type typeid;
   char name_in[NC_MAX_NAME+1];
   int i, j, k;

   int var_dims[VAR_RANK];

#define VEC_LEN 3
#define MAT_DIM0 2
#define MAT_DIM1 3
#define MON_LEN 3
#define NFIELDS 4		/* number of fields in struct and compound type */
   struct vecmat_t {
       char dow;
       char mon[MON_LEN];
       short vec[VEC_LEN];
       float mat[MAT_DIM0][MAT_DIM1];
   };

   struct vecmat_t data[DIM_LEN] = {
       {
	   'S', 
	   "jan",
	   {1, 2, 3}, 
	   {{4, 5, 6}, {7, 8, 9}}
       },
       {
	   'M', 
	   "feb",
	   {11, 12, 13}, 
	   {{4.25, 5.25, 6.25}, {7.25, 8.25, 9.25}}
       },
       {
	   'T', 
	   "mar",
	   {21, 22, 23}, 
	   {{4.5, 5.5, 6.5}, {7.5, 8.5, 9.5}}
       }
   };

   struct vecmat_t missing_val = {
       '?',
       "---",
       {-1, -2, -3}, 
       {{-4.0f, -5.0f, -6.0f}, {-7.0f, -8.0f, -9.0f}}
   };
   struct vecmat_t val_in;
   size_t size_in;
   size_t nfields_in;
   nc_type class_in;
   int vec_sizes[] = {VEC_LEN};
   int mat_sizes[] = {MAT_DIM0, MAT_DIM1};
   int mon_sizes[] = {MON_LEN};

   printf("\n*** Testing compound types some more.\n");
   printf("*** creating another compound test file %s...", FILE9_NAME);
   if (nc_create(FILE9_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

   /* Create a compound type. */
   if (nc_def_compound(ncid, sizeof(struct vecmat_t), TYPE_NAME, &typeid)) ERR;
   if (nc_insert_compound(ncid, typeid, "day",
				NC_COMPOUND_OFFSET(struct vecmat_t, dow),
				NC_CHAR)) ERR;
   if (nc_insert_array_compound(ncid, typeid, "mnth",
				NC_COMPOUND_OFFSET(struct vecmat_t, mon),
				NC_CHAR, 1, mon_sizes)) ERR;
   if (nc_insert_array_compound(ncid, typeid, "vect",
				NC_COMPOUND_OFFSET(struct vecmat_t, vec), 
				NC_SHORT, 1, vec_sizes)) ERR;
   if (nc_insert_array_compound(ncid, typeid, "matr",
				NC_COMPOUND_OFFSET(struct vecmat_t, mat), 
				NC_FLOAT, 2, mat_sizes)) ERR;

   /* Declare a dimension for number of obs */
   if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;

   /* Declare a variable of the compound type */
   var_dims[0] = dimid;
   if (nc_def_var(ncid, VAR_NAME, typeid, VAR_RANK, var_dims, &varid)) ERR;

   /* Create and write a variable attribute of the compound type */
   if (nc_put_att(ncid, varid, ATT_NAME, typeid, ATT_LEN, (void *) &missing_val)) ERR;
   if (nc_enddef(ncid)) ERR;

   /* Store data, writing all values in one call */
   if(nc_put_var(ncid, varid, data)) ERR;

   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Check it out. */
   
   /* Reopen the file. */
   if (nc_open(FILE9_NAME, NC_NOWRITE, &ncid)) ERR;

   /* Get info with the generic inquire for user-defined types */
   if (nc_inq_user_type(ncid, typeid, name_in, NULL, NULL, 
			NULL, &class_in)) ERR;
   if (strcmp(name_in, TYPE_NAME) || 
       class_in != NC_COMPOUND) ERR;

   /* Get the same info with the compound-specific inquire function */
   if (nc_inq_compound(ncid, typeid, name_in, &size_in, &nfields_in)) ERR;
   if (strcmp(name_in, TYPE_NAME) || 
       size_in != sizeof(struct vecmat_t) ||
       nfields_in != NFIELDS) ERR;

   if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;

   /* Read in attribute value and check it */
   if (nc_get_att(ncid, varid, ATT_NAME, &val_in)) ERR;
   for(i=0; i < VEC_LEN; i++) {
       if (val_in.vec[i] != missing_val.vec[i]) ERR;
   }
   for(i=0; i < MAT_DIM0; i++) {
       for(j=0; j < MAT_DIM1; j++) {
	   if (val_in.mat[i][j] != missing_val.mat[i][j]) ERR;
       }
   }
   for(i=0; i < MON_LEN; i++) {
       if (val_in.mon[i] != missing_val.mon[i]) ERR;
   }
   /* Read in each value and check */
   for (k = 0; k < DIM_LEN; k++) {
       size_t index[VAR_RANK];
       index[0] = k;
       if (nc_get_var1(ncid, varid, index, (void *) &val_in)) ERR;
       if (val_in.dow != data[k].dow) ERR;
       for(i=0; i < MON_LEN; i++) {
	   if (val_in.mon[i] != data[k].mon[i]) ERR;
       }
       for(i=0; i < VEC_LEN; i++) {
	   if (val_in.vec[i] != data[k].vec[i]) ERR;
       }
       for(i=0; i < MAT_DIM0; i++) {
	   for(j=0; j < MAT_DIM1; j++) {
	       if (val_in.mat[i][j] != data[k].mat[i][j]) ERR;
	   }
       }
   }
   
   if (nc_close(ncid)) ERR; 
   
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

