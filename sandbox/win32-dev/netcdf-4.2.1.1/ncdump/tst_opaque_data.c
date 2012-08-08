/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with an opaque type and opaque data for ncdump to read.

   $Id$
*/

#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>

#define FILE3_NAME "tst_opaque_data.nc"
#define TYPE3_NAME "raw_obs_t"
#define TYPE3_SIZE 11
#define DIM3_NAME "time"
#define DIM3_LEN 5
#define VAR3_NAME "raw_obs"
#define VAR3_RANK 1
#define ATT3_NAME "_FillValue"
#define ATT3_LEN  1

int
main(int argc, char **argv)
{
   int ncid;
   int dimid, varid;
   nc_type typeid;
   char name_in[NC_MAX_NAME+1];
   int class_in;
   size_t size_in;

   int i;

   int var_dims[VAR3_RANK];
   unsigned char sensor_data[DIM3_LEN][TYPE3_SIZE] = {
       {1,2,3,4,5,6,7,8,9,10,11},
       {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa},
       {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
       {0xca, 0xfe, 0xba, 0xbe, 0xca, 0xfe, 0xba, 0xbe, 0xca, 0xfe, 0xba},
       {0xcf, 0x0d, 0xef, 0xac, 0xed, 0x0c, 0xaf, 0xe0, 0xfa, 0xca, 0xde}
   };
   unsigned char missing_val[TYPE3_SIZE] = {
       0xca, 0xfe, 0xba, 0xbe, 0xca, 0xfe, 0xba, 0xbe, 0xca, 0xfe, 0xba
   };
   unsigned char val_in[TYPE3_SIZE];

   printf("\n*** Testing opaque types.\n");
   printf("*** creating opaque test file %s...", FILE3_NAME);
   if (nc_create(FILE3_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

   /* Create an opaque type. */
   if (nc_def_opaque(ncid, TYPE3_SIZE, TYPE3_NAME, &typeid)) ERR;

   /* Declare a time dimension */
   if (nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimid)) ERR;

   /* Declare a variable of the opaque type */
   var_dims[0] = dimid;
   if (nc_def_var(ncid, VAR3_NAME, typeid, VAR3_RANK, var_dims, &varid)) ERR;

   /* Create and write a variable attribute of the opaque type */
   if (nc_put_att(ncid, varid, ATT3_NAME, typeid, ATT3_LEN, missing_val)) ERR;
   if (nc_enddef(ncid)) ERR;
   /* Store some data of the opaque type */
   if(nc_put_var(ncid, varid, sensor_data)) ERR;
   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Check it out. */
   
   /* Reopen the file. */
   if (nc_open(FILE3_NAME, NC_NOWRITE, &ncid)) ERR;

   /* Get info with the generic inquire for user-defined types */
   if (nc_inq_user_type(ncid, typeid, name_in, &size_in, NULL,
			   NULL, &class_in)) ERR;
   if (strcmp(name_in, TYPE3_NAME) || 
       size_in != TYPE3_SIZE ||
       class_in != NC_OPAQUE) ERR;
   /* Get the same info with the opaque-specific inquire function */
   if (nc_inq_opaque(ncid, typeid, name_in, &size_in)) ERR;
   if (strcmp(name_in, TYPE3_NAME) || 
       size_in !=  TYPE3_SIZE) ERR;

   if (nc_inq_varid(ncid, VAR3_NAME, &varid)) ERR;

   if (nc_get_att(ncid, varid, ATT3_NAME, &val_in)) ERR;
   if (memcmp(val_in, missing_val, TYPE3_SIZE) != 0) ERR;

   for (i = 0; i < DIM3_LEN; i++) {
       size_t index[VAR3_RANK];
       index[0] = i;
       if(nc_get_var1(ncid, varid, index, val_in)) ERR;
       if (memcmp(val_in, sensor_data[i], TYPE3_SIZE) != 0) ERR;
   }
   
   if (nc_close(ncid)) ERR; 
   
   
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

