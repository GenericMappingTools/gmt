/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with a string data for ncdump to read.

   $Id$
*/

#include <nc_tests.h>
#include <netcdf.h>

#define FILE4_NAME "tst_string_data.nc"
#define DIM4_NAME "line"
#define DIM4_LEN 5
#define VAR4_NAME "description"
#define VAR4_RANK 1
#define ATT4_NAME "_FillValue"
#define ATT4_LEN  1

int
main(int argc, char **argv)
{
   int ncid;
   int dimid, varid;
   nc_type att_type;
   size_t att_len;

   int i;

   int var_dims[VAR4_RANK];
   const char *desc_data[DIM4_LEN] = {
       "first string", "second string", "third string", "", "last string"
   };
   const char *missing_val[ATT4_LEN] = {""};
   char *strings_in[DIM4_LEN];
   
   printf("\n*** Testing strings.\n");
   printf("*** creating strings test file %s...", FILE4_NAME);
   if (nc_create(FILE4_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
   
   /* Declare a line dimension */
   if (nc_def_dim(ncid, DIM4_NAME, DIM4_LEN, &dimid)) ERR;
   
   /* Declare a string variable */
   var_dims[0] = dimid;
   if (nc_def_var(ncid, VAR4_NAME, NC_STRING, VAR4_RANK, var_dims, &varid)) ERR;
   
   /* Create and write a variable attribute of string type */
   if (nc_put_att_string(ncid, varid, ATT4_NAME, ATT4_LEN, missing_val)) ERR;
   if (nc_enddef(ncid)) ERR;
   
   /* Store some data of string type */
   if(nc_put_var(ncid, varid, desc_data)) ERR;
   
   /* Write the file. */
   if (nc_close(ncid)) ERR;
   
   /* Check it out. */
   if (nc_open(FILE4_NAME, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq_varid(ncid, VAR4_NAME, &varid)) ERR;
   if (nc_inq_att(ncid, varid, ATT4_NAME, &att_type, &att_len)) ERR;
   if (att_type != NC_STRING || att_len != ATT4_LEN) ERR;
   if (nc_get_att_string(ncid, varid, ATT4_NAME, strings_in)) ERR;
   
   if (strcmp(strings_in[0], *missing_val) != 0) ERR;
   /* string atts should be explicitly freed when done with them */
   nc_free_string(ATT4_LEN, strings_in);
   
   if(nc_get_var_string(ncid, varid, strings_in)) ERR;
   for (i = 0; i < DIM4_LEN; i++) {
       if (strcmp(strings_in[i], desc_data[i]) != 0) ERR;
   }
   nc_free_string(DIM4_LEN, strings_in);

   /* Try reading strings in with typeless generic interface also */
   if(nc_get_var(ncid, varid, strings_in)) ERR;

   for (i = 0; i < DIM4_LEN; i++) {
       if (strcmp(strings_in[i], desc_data[i]) != 0) ERR;
   }
   nc_free_string(DIM4_LEN, strings_in);


   /* Try reading strings in with typeless generic array interface also */
   {
       size_t cor[VAR4_RANK], edg[VAR4_RANK];
       cor[0] = 0;
       edg[0] = DIM4_LEN;
       if(nc_get_vara(ncid, varid, cor, edg, strings_in)) ERR;

       for (i = 0; i < DIM4_LEN; i++) {
	   if (strcmp(strings_in[i], desc_data[i]) != 0) ERR;
       }
       nc_free_string(DIM4_LEN, strings_in);
   }

   if (nc_close(ncid)) ERR; 

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

