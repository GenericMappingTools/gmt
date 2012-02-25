/* This is part of the netCDF package. Copyright 2005-2011, University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test NetCDF-4 files with lots of attributes on big vs. little
   endian platforms.
*/
#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_interops4.nc"
#define REF_FILE_NAME "ref_tst_interops4.nc"
#define DIM_NAME "dim_0"
#define NUM_DIMS 1
#define NUM_ATTS 20
#define NUM_VARS 20
#define ATT_LEN 30      
#define VAR_LEN 4

int att_data[ATT_LEN];
int var_data[VAR_LEN];

int
write_atts(int ncid, int varid)
{
   int a;
   char att_name[NC_MAX_NAME + 1];

   for (a = 0; a < NUM_ATTS; a++)
   {
      sprintf(att_name, "att_%d", a);
      if (nc_put_att_int(ncid, varid, att_name, NC_INT, 
			 ATT_LEN, att_data)) ERR_RET;
   }
   return NC_NOERR;
}

int
read_atts(int ncid, int varid)
{
   int a, i;
   char att_name[NC_MAX_NAME + 1];
   int att_data_in[ATT_LEN];

   for (a = 0; a < NUM_ATTS; a++)
   {
      sprintf(att_name, "att_%d", a);
      if (nc_get_att_int(ncid, varid, att_name, 
			 att_data_in)) ERR_RET;
      for (i = 0; i < ATT_LEN; i++)
	 if (att_data_in[i] != att_data[i]) ERR_RET;
      
   }
   return NC_NOERR;
}

int
write_vars(int ncid)
{
   int v;
   char var_name[NC_MAX_NAME + 1];
   /* NOT GOOD to assume the dimid, but I want to test passing a NULL
    * into last argument of nc_def_dim. */
   int dimid[NUM_DIMS] = {0};

   if (nc_def_dim(ncid, DIM_NAME, VAR_LEN, NULL)) ERR;
   for (v = 0; v < NUM_VARS; v++)
   {
      sprintf(var_name, "var_%d", v);
      if (nc_def_var(ncid, var_name, NC_INT, NUM_DIMS, 
		     dimid, NULL)) ERR_RET;
      write_atts(ncid, v);
      if (nc_put_var_int(ncid, v, var_data)) ERR;      
   }
   return NC_NOERR;
}

int
read_vars(int ncid)
{
   int v, i;
   char var_name[NC_MAX_NAME + 1], var_name_in[NC_MAX_NAME + 1];
   int var_data_in[VAR_LEN];
   nc_type xtype_in;
   int natts_in, ndims_in;

   for (v = 0; v < NUM_VARS; v++)
   {
      if (nc_inq_var(ncid, v, var_name_in, &xtype_in, &ndims_in, 
		     NULL, &natts_in)) ERR_RET;
      sprintf(var_name, "var_%d", v);
      if (strcmp(var_name, var_name_in) || xtype_in != NC_INT || 
		 ndims_in != NUM_DIMS || natts_in != NUM_ATTS) ERR_RET;
      read_atts(ncid, v);
      if (nc_get_var_int(ncid, v, var_data_in)) ERR;
      for (i = 0; i < VAR_LEN; i++)
	 if (var_data_in[i] != var_data[i]) ERR_RET;
      
   }
   return NC_NOERR;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing interoperability between big vs. little endian platforms.\n");
   printf("*** testing with file with lots of atts...");
   {
      int ncid;
      int nvars_in, ndims_in, natts_in, unlimdim_in;
      int i;

      /* Initialize data. */
      for (i = 0; i < ATT_LEN; i++)
	 att_data[i] = i;
      for (i = 0; i < VAR_LEN; i++)
	 var_data[i] = i;
      
      /* Create a file that will activate the bug in HDF5 1.8.4. */
      if (nc_create(FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;
      if (write_atts(ncid, NC_GLOBAL)) ERR;
      if (write_vars(ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check it. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != NUM_DIMS || nvars_in != NUM_VARS || 
	  natts_in != NUM_ATTS || unlimdim_in != -1) ERR;
      if (read_atts(ncid, NC_GLOBAL)) ERR;
      if (read_vars(ncid)) ERR;
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   {
      char file_in[NC_MAX_NAME + 1];
      int ncid;
      int nvars_in, ndims_in, natts_in, unlimdim_in;

      /* Open the reference version of this file, generated on a
       * big-endian platform. */
      if (getenv("srcdir"))
      {
	 strcpy(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
	 strcat(file_in, REF_FILE_NAME);
      }
      else
	 strcpy(file_in, REF_FILE_NAME);

      printf("*** testing with file %s...", file_in);
      if (nc_open(file_in, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != NUM_DIMS || nvars_in != NUM_VARS || 
	  natts_in != NUM_ATTS || unlimdim_in != -1) ERR;
      if (read_atts(ncid, NC_GLOBAL)) ERR;
      if (read_vars(ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

