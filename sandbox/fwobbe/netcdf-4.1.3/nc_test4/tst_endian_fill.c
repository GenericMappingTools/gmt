/* This is part of the netCDF package. Copyright 2008 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with fill values for a variable of specified
   endianness.

   $Id$
*/

#include <nc_tests.h>
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

#define FILE_NAME "tst_endian_fill.nc" 
#define VAR_NAME "v1"
#define VAR2_NAME "v2"
#define VAR3_NAME "v3"
#define VAR_RANK 0
int
main(int argc, char **argv) 
{
   printf("\n*** Testing specified endiannesss fill values.\n");
   printf("*** testing simple case with int...");
   {
      int  ncid, varid, var2id, var3id;
      int data_in;
      int fill = NC_FILL_INT;

      /* Create file with a scalar int var that's big-endian. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, VAR_RANK, 0, &varid)) ERR;
      if (nc_def_var_endian(ncid, varid, NC_ENDIAN_BIG)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_INT, VAR_RANK, 0, &var2id)) ERR;
      if (nc_def_var_endian(ncid, var2id, NC_ENDIAN_LITTLE)) ERR;
      /* close without writing data, so vars 1 and 2 should contain
       * NC_FILL_INT */
      if (nc_def_var(ncid, VAR3_NAME, NC_INT, VAR_RANK, 0, &var3id)) ERR;
      if (nc_def_var_endian(ncid, var3id, NC_ENDIAN_BIG)) ERR;
      if (nc_put_var(ncid, var3id, &fill));
      if (nc_close(ncid)) ERR;
    
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_varid(ncid, VAR_NAME, &varid)) ERR;
      if (nc_inq_varid(ncid, VAR2_NAME, &var2id)) ERR;
      if (nc_get_var_int(ncid, varid, &data_in)) ERR;
      if (data_in != NC_FILL_INT) ERR;
      if (nc_get_var_int(ncid, var2id, &data_in)) ERR;
      if (data_in != NC_FILL_INT) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing short, int, int64, and unsigned...");
   {
#define NUM_TYPES_TO_CHECK 6
      int  ncid, varid[NUM_TYPES_TO_CHECK];
      int check_type[NUM_TYPES_TO_CHECK] = {NC_SHORT, NC_USHORT, NC_INT, 
					    NC_UINT, NC_INT64, NC_UINT64};
      char var_name[NUM_TYPES_TO_CHECK][NC_MAX_NAME + 1] = {"SHORT", "USHORT", 
							   "INT", "UINT", "INT64", 
							   "UINT64"};
      long long fill_value[NUM_TYPES_TO_CHECK] = {NC_FILL_SHORT, NC_FILL_USHORT, NC_FILL_INT, 
						  NC_FILL_UINT, NC_FILL_INT64, NC_FILL_UINT64};
      long long data_in;
      int t;

      /* Create file with a scalar vars. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      for (t = 0; t < NUM_TYPES_TO_CHECK; t++)
      {
	 if (nc_def_var(ncid, var_name[t], check_type[t], 0, 0, &varid[t])) ERR;
	 if (nc_def_var_endian(ncid, varid[t], NC_ENDIAN_BIG)) ERR;
      }
      if (nc_close(ncid)) ERR;
    
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      for (t = 0; t < NUM_TYPES_TO_CHECK; t++)
      {
	 int err = nc_get_var_longlong(ncid, varid[t], &data_in);
	 if(err && err != NC_ERANGE)
	   ERR;
	 if (data_in != fill_value[t])
	   ERR;
      }
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
    
   return 0;
}
