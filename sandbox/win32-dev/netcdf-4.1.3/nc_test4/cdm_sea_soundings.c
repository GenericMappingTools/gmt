/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   The cdm_* tests confirm complience with the Common Data Model. This
   file creates some sample data structures to hold sea soundings.

   $Id$
*/
#include <nc_tests.h>

#define FILE_NAME "cdm_sea_soundings.nc"
#define DIM_NAME "Sounding"
#define DIM_LEN 3

int
main(int argc, char **argv)
{
   int ncid, dimid, varid, temp_typeid, sounding_typeid;
   struct sea_sounding
   {
	 int sounding_no;
	 nc_vlen_t temp_vl;
   } data[DIM_LEN];
   int i, j;

   /* Create phony data. */
   for (i = 0; i < DIM_LEN; i++)
   {
      if (!(data[i].temp_vl.p = malloc(sizeof(float) * (i + 1))))
	 return NC_ENOMEM;
      for (j = 0; j < i + 1; j++)
	 ((float *)(data[i].temp_vl.p))[j] = 23.5 - j;
      data[i].temp_vl.len = i + 1;
   }

   printf("\n*** Testing netcdf-4 CDM compliance: sea soundings.\n");
   printf("*** creating simple sea sounding file...");
      
   /* Create a netcdf-4 file. */
   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

   /* Create the vlen type, with a float base type. */
   if (nc_def_vlen(ncid, "temp_vlen", NC_FLOAT, &temp_typeid)) ERR;

   /* Create the compound type to hold a sea sounding. */
   if (nc_def_compound(ncid, sizeof(struct sea_sounding),
		       "sea_sounding", &sounding_typeid)) ERR;
   if (nc_insert_compound(ncid, sounding_typeid, "sounding_no",
			  NC_COMPOUND_OFFSET(struct sea_sounding, sounding_no),
			  NC_INT)) ERR;
   if (nc_insert_compound(ncid, sounding_typeid, "temp_vl",
			  NC_COMPOUND_OFFSET(struct sea_sounding, temp_vl),
			  temp_typeid)) ERR;

   /* Define a dimension, and a 1D var of sea sounding compound type. */
   if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
   if (nc_def_var(ncid, "fun_soundings", sounding_typeid, 1,
		  &dimid, &varid)) ERR;

   /* Write our array of phone data to the file, all at once. */
   if (nc_put_var(ncid, varid, data)) ERR;

   /* We're done, yipee! */
   if (nc_close(ncid)) ERR;

   SUMMARIZE_ERR;

   /* Free the memory the phony data are using. */
    for (i = 0; i < DIM_LEN; i++)
       free(data[i].temp_vl.p); 

   /* Print out our number of errors, if any, and exit badly. */
   if (total_err)
   {
      printf("%d errors detected! Sorry!\n", total_err);
      return 2;
   }
   
   printf("*** Tests successful!\n");
   return 0;
}


