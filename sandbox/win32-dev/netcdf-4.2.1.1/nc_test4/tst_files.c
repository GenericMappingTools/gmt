/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test internal netcdf-4 file code. 
   $Id$
*/

#include <config.h>
#include "netcdf.h"
#include <nc_tests.h>

#ifdef IGNORE
extern NC_FILE_INFO_T *nc_file;
#endif
int test_redef(int format);

#define FILE_NAME "tst_files.nc"
#define ATT1_NAME "MoneyOwned"
#define ATT2_NAME "Number_of_Shoes"
#define ATT3_NAME "att3"
#define DIM1_NAME "Character_Growth"
#define DIM1_LEN 10
#define DIM2_NAME "TimeInMonths"
#define DIM2_LEN 15
#define VAR1_NAME "HuckFinn"
#define VAR2_NAME "TomSawyer"
#define VAR3_NAME "Jim"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 file functions.\n");
   {
      char str[NC_MAX_NAME+1];
      
      /* Actually we never make any promises about the length of the
       * version string, but it is always smaller than NC_MAX_NAME. */
      if (strlen(nc_inq_libvers()) > NC_MAX_NAME) ERR;
      strcpy(str, nc_inq_libvers());
      printf("*** testing version %s...", str);
   }
   SUMMARIZE_ERR;
   printf("*** testing with bad inputs...");
   {
      int ncid;

      /* Make sure bad create mode causes failure. */
      /*if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;*/
      
      /* Create an empty file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, NC_MPIIO|NC_MPIPOSIX, &ncid) != NC_EINVAL) ERR;

      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_NETCDF4, &ncid) != NC_EINVAL) ERR;
      if (nc_create(FILE_NAME, NC_CLASSIC_MODEL|NC_MPIIO|NC_MPIPOSIX, &ncid) != NC_EINVAL) ERR;
      if (nc_create(FILE_NAME, NC_MPIIO|NC_MPIPOSIX, &ncid) != NC_EINVAL) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple opens and creates...");
   {
      int ncid, ncid2, ncid3, varid, dimids[2];
      int ndims, nvars, natts, unlimdimid;
      int dimids_var[1], var_type;
      size_t dim_len;
      char dim_name[NC_MAX_NAME+1], var_name[NC_MAX_NAME+1];
      unsigned char uchar_out[DIM1_LEN] = {0, 128, 255};

      /* Open and close empty file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate it again. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. Then define a new variable. Since it's
       * netcdf-4, nc_enddef isn't required - it's called
       * automatically. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_redef(ncid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR3_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open three copies of the same file. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_open(FILE_NAME, NC_WRITE, &ncid2)) ERR;
      if (nc_open(FILE_NAME, NC_WRITE, &ncid3)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq(ncid2, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq(ncid3, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 3 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;
      if (nc_close(ncid2)) ERR;
      if (nc_close(ncid3)) ERR;
      
      /* Open and close empty file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a file with one dimension and nothing else. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 0 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a simple file, and write some data to it. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_BYTE, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_uchar(ncid, varid, uchar_out) != NC_ERANGE) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 1 || strcmp(var_name, VAR1_NAME) || var_type != NC_BYTE ||
	  dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate the file. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_BYTE, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_put_var_uchar(ncid, varid, uchar_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate it, then make sure NOCLOBBER works. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_NOCLOBBER, &ncid) != NC_EEXIST) ERR;

      /* Recreate it again. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. Then define a new variable. Since it's
       * netcdf-4, nc_enddef isn't required - it's called
       * automatically. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_redef(ncid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_def_var(ncid, VAR3_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Recreate it again with netcdf-3 rules turned on. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 1, dimids, &varid)) ERR;
      if (nc_enddef(ncid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check the contents. Check that netcdf-3 rules are in effect. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid) != NC_ENOTINDEFINE) ERR;
      if (nc_close(ncid)) ERR;

      /* Check some other stuff about it. Closing and reopening the
       * file forces HDF5 to tell us if we forgot to free some HDF5
       * resource associated with the file. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 1 || strcmp(var_name, VAR1_NAME) || var_type != NC_INT ||
	  dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing more complex opens and creates...");
   {
      int ncid, varid, dimids[2];
      int ndims, nvars, natts, unlimdimid;
      int dimids_var[2], var_type;
      size_t dim_len;
      char dim_name[NC_MAX_NAME+1], var_name[NC_MAX_NAME+1];
      float float_in, float_out = 99.99;
      int int_in, int_out = -9999;

      /* Create a file, this time with attributes. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
      if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
      if (nc_def_var(ncid, VAR1_NAME, NC_INT, 2, dimids, &varid)) ERR;
      if (nc_def_var(ncid, VAR2_NAME, NC_UINT, 2, dimids, &varid)) ERR;
      if (nc_put_att_float(ncid, NC_GLOBAL, ATT1_NAME, NC_FLOAT, 1, &float_out)) ERR;
      if (nc_put_att_int(ncid, NC_GLOBAL, ATT2_NAME, NC_INT, 1, &int_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 2 || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen it and check each dim, var, and att. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
      if (dim_len != DIM1_LEN || strcmp(dim_name, DIM1_NAME)) ERR;
      if (nc_inq_dim(ncid, 1, dim_name, &dim_len)) ERR;
      if (dim_len != DIM2_LEN || strcmp(dim_name, DIM2_NAME)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 2 || strcmp(var_name, VAR1_NAME) || var_type != NC_INT ||
	  dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_inq_var(ncid, 1, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 2 || strcmp(var_name, VAR2_NAME) || var_type != NC_UINT ||
	  dimids_var[1] != dimids[1] || natts != 0) ERR;
      if (nc_get_att_float(ncid, NC_GLOBAL, ATT1_NAME, &float_in)) ERR;
      if (float_in != float_out) ERR;
      if (nc_get_att_int(ncid, NC_GLOBAL, ATT2_NAME, &int_in)) ERR;
      if (int_in != int_out) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** testing redef for netCDF classic...");
   test_redef(NC_FORMAT_CLASSIC);
   SUMMARIZE_ERR;
   printf("*** testing redef for netCDF 64-bit offset...");
   test_redef(NC_FORMAT_64BIT);
   SUMMARIZE_ERR;
   printf("*** testing redef for netCDF-4 ...");
   test_redef(NC_FORMAT_NETCDF4);
   SUMMARIZE_ERR;
   printf("*** testing redef for netCDF-4, with strict netCDF-3 rules...");
   test_redef(NC_FORMAT_NETCDF4_CLASSIC);
   SUMMARIZE_ERR;
   printf("*** testing different formats...");
   {
      int ncid;
      int format;

      /* Create a netcdf-3 file. */
      if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_CLASSIC) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a netcdf-3 64-bit offset file. */
      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_64BIT) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a netcdf-4 file. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_NETCDF4) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing CLASSIC_MODEL flag with classic formats...");
   {
      int ncid;
      int format;

      /* Create a netcdf-3 file. */
      if (nc_create(FILE_NAME, NC_CLOBBER|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_CLASSIC) ERR;
      if (nc_close(ncid)) ERR;

      /* Create a netcdf-3 64-bit offset file. */
      if (nc_create(FILE_NAME, NC_64BIT_OFFSET|NC_CLOBBER|NC_CLASSIC_MODEL, &ncid)) ERR;
      if (nc_inq_format(ncid, &format)) ERR;
      if (format != NC_FORMAT_64BIT) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing multiple open files...");
   {
#define VAR_NAME "Captain_Kirk"
#define NDIMS 1
#define NUM_FILES 30
#define TEXT_LEN 15
#define D1_NAME "tl"
      int ncid[NUM_FILES], varid;
      int dimid;
      size_t count[NDIMS], index[NDIMS] = {0};
      const char ttext[TEXT_LEN + 1] = "20051224.150000";
      char ttext_in[TEXT_LEN + 1];
      char file_name[NC_MAX_NAME + 1];
      size_t chunks[NDIMS] = {TEXT_LEN + 1};
      int f;

      /* Create a bunch of files. */
      for (f = 0; f < NUM_FILES; f++)
      {
	 sprintf(file_name, "tst_files2_%d.nc", f);
	 if (nc_create(file_name, NC_NETCDF4, &ncid[f])) ERR;
	 if (nc_def_dim(ncid[f], D1_NAME, TEXT_LEN + 1, &dimid)) ERR;
	 if (nc_def_var(ncid[f], VAR_NAME, NC_CHAR, NDIMS, &dimid, &varid)) ERR;
	 if (f % 2 == 0)
	    if (nc_def_var_chunking(ncid[f], varid, 0, chunks)) ERR;
	 
	 /* Write one time to the coordinate variable. */
	 count[0] = TEXT_LEN + 1;
	 if (nc_put_vara_text(ncid[f], varid, index, count, ttext)) ERR;
      }

      /* Read something from each file. */
      for (f = 0; f < NUM_FILES; f++)
      {
	 if (nc_get_vara_text(ncid[f], varid, index, count, (char *)ttext_in)) ERR;
	 if (strcmp(ttext_in, ttext)) ERR;
      }

      /* Close all open files. */
      for (f = 0; f < NUM_FILES; f++)
	 if (nc_close(ncid[f])) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
#define REDEF_ATT1_NAME "CANTERBURY"
#define REDEF_ATT2_NAME "ELY"
#define REDEF_ATT3_NAME "KING_HENRY_V"
#define REDEF_DIM1_NAME "performance_length"
#define REDEF_DIM1_LEN 101
/* Change illegal name from '?' to '/'; the latter will
   more likely be illegal even when the switch is made to
   escaped characters in identifiers.
*/
#define REDEF_NAME_ILLEGAL "/"
#define REDEF_DIM2_NAME "ZZ_number_of_performers_or_perhaps_actors_or_maybe_I_should_say_players_or_one_could_call_them_artists_but_one_doesnt_like_to_get"

#define REDEF_DIM2_LEN 999
#define REDEF_VAR1_NAME "Royal_Shakespeare_Company_season_of_2004"
#define REDEF_VAR2_NAME "Ms_Beths_Kindergardern_class_of_2003"
#define REDEF_VAR3_NAME "Costumed_Mice_in_my_Garage_in_the_Winter_of_my_Discontent"
#define REDEF_NDIMS 2

#define NEW_CACHE_SIZE 32000000
#define NEW_CACHE_NELEMS 2000
#define NEW_CACHE_PREEMPTION .75

int
test_redef(int format)   
{
   int ncid, varid, dimids[REDEF_NDIMS], dimids_in[REDEF_NDIMS];
   int ndims, nvars, natts, unlimdimid;
   int dimids_var[REDEF_NDIMS], var_type;
   int cflags = 0;
   size_t dim_len;
   char dim_name[NC_MAX_NAME+1], var_name[NC_MAX_NAME+1];
   float float_in;
   double double_out = 99E99;
   int int_in;
   unsigned char uchar_in, uchar_out = 255;
   short short_out = -999;
   nc_type xtype_in;
   size_t cache_size_in, cache_nelems_in;
   float cache_preemption_in;
   int ret;

   if (format == NC_FORMAT_64BIT)
      cflags |= NC_64BIT_OFFSET;
   else if (format == NC_FORMAT_NETCDF4_CLASSIC)
      cflags |= (NC_NETCDF4|NC_CLASSIC_MODEL);
   else if (format == NC_FORMAT_NETCDF4)
      cflags |= NC_NETCDF4;

   /* Change chunk cache. */
   if (nc_set_chunk_cache(NEW_CACHE_SIZE, NEW_CACHE_NELEMS, 
			  NEW_CACHE_PREEMPTION)) ERR;

   /* Create a file with two dims, two vars, and two atts. */
   if (nc_create(FILE_NAME, cflags|NC_CLOBBER, &ncid)) ERR;

   /* Retrieve the chunk cache settings, just for fun. */
   if (nc_get_chunk_cache(&cache_size_in, &cache_nelems_in, 
			  &cache_preemption_in)) ERR;
   if (cache_size_in != NEW_CACHE_SIZE || cache_nelems_in != NEW_CACHE_NELEMS ||
       cache_preemption_in != NEW_CACHE_PREEMPTION) ERR;

   /* This will fail, except for netcdf-4/hdf5, which permits any
    * name. */
   if (format != NC_FORMAT_NETCDF4)
      if ((ret = nc_def_dim(ncid, REDEF_NAME_ILLEGAL, REDEF_DIM2_LEN, 
			    &dimids[1])) != NC_EBADNAME) ERR;

   if (nc_def_dim(ncid, REDEF_DIM1_NAME, REDEF_DIM1_LEN, &dimids[0])) ERR;
   if (nc_def_dim(ncid, REDEF_DIM2_NAME, REDEF_DIM2_LEN, &dimids[1])) ERR;
   if (nc_def_var(ncid, REDEF_VAR1_NAME, NC_INT, REDEF_NDIMS, dimids, &varid)) ERR;
   if (nc_def_var(ncid, REDEF_VAR2_NAME, NC_BYTE, REDEF_NDIMS, dimids, &varid)) ERR;
   if (nc_put_att_double(ncid, NC_GLOBAL, REDEF_ATT1_NAME, NC_DOUBLE, 1, &double_out)) ERR;
   if (nc_put_att_short(ncid, NC_GLOBAL, REDEF_ATT2_NAME, NC_SHORT, 1, &short_out)) ERR;

   /* Check it out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;

   /* Close it up. */
   if (format != NC_FORMAT_NETCDF4)
      if (nc_enddef(ncid)) ERR;
   if (nc_close(ncid)) ERR;

   /* Reopen as read only - make sure it doesn't let us change file. */
   if (nc_open(FILE_NAME, 0, &ncid)) ERR;
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;

   /* This will fail. */
   ret = nc_def_var(ncid, REDEF_VAR3_NAME, NC_UBYTE, REDEF_NDIMS, 
			  dimids, &varid);
   if(format == NC_FORMAT_NETCDF4) {
	if(ret != NC_EPERM) {
	    ERR;
	}
   } else {
	if(ret != NC_ENOTINDEFINE) {
	    ERR;
	}
   }
   /* This will fail. */
   if (!nc_put_att_uchar(ncid, NC_GLOBAL, REDEF_ATT3_NAME, NC_CHAR, 1, &uchar_out)) ERR;
   if (nc_close(ncid)) ERR;      

   /* Make sure we can't redef a file opened for NOWRITE. */
   if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
   if (nc_redef(ncid) != NC_EPERM) ERR;

   /* Check it out again. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;

   if (nc_close(ncid)) ERR;      

   /* Reopen the file and check it, add a variable and attribute. */
   if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;

   /* Check it out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 2 || natts != 2 || unlimdimid != -1) ERR;

   /* Add var. */
   if ((format != NC_FORMAT_NETCDF4) && nc_redef(ncid)) ERR;
   if (nc_def_var(ncid, REDEF_VAR3_NAME, NC_BYTE, REDEF_NDIMS, dimids, &varid)) ERR;

   /* Add att. */
   ret = nc_put_att_uchar(ncid, NC_GLOBAL, REDEF_ATT3_NAME, NC_BYTE, 1, &uchar_out);
   if (format != NC_FORMAT_NETCDF4 && ret) ERR;
   else if (format == NC_FORMAT_NETCDF4 && ret != NC_ERANGE) ERR;

   /* Check it out. */
   if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
   if (ndims != REDEF_NDIMS || nvars != 3 || natts != 3 || unlimdimid != -1) ERR;
   if (nc_inq_var(ncid, 0, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR1_NAME) || xtype_in != NC_INT || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 1, var_name, &xtype_in, &ndims, dimids_in, &natts)) ERR;
   if (strcmp(var_name, REDEF_VAR2_NAME) || xtype_in != NC_BYTE || ndims != REDEF_NDIMS || 
       dimids_in[0] != dimids[0] || dimids_in[1] != dimids[1]) ERR;
   if (nc_inq_var(ncid, 2, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR3_NAME) || var_type != NC_BYTE ||
       natts != 0) ERR;

   if (nc_close(ncid)) ERR;      

   /* Reopen it and check each dim, var, and att. */
   if (nc_open(FILE_NAME, 0, &ncid)) ERR;
   if (nc_inq_dim(ncid, 0, dim_name, &dim_len)) ERR;
   if (dim_len != REDEF_DIM1_LEN || strcmp(dim_name, REDEF_DIM1_NAME)) ERR;
   if (nc_inq_dim(ncid, 1, dim_name, &dim_len)) ERR;
   if (dim_len != REDEF_DIM2_LEN || strcmp(dim_name, REDEF_DIM2_NAME)) ERR;
   if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR1_NAME) || var_type != NC_INT ||
       natts != 0) ERR;
   if (nc_inq_var(ncid, 1, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR2_NAME) || var_type != NC_BYTE ||
       natts != 0) ERR;
   if (nc_inq_var(ncid, 2, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
   if (ndims != REDEF_NDIMS || strcmp(var_name, REDEF_VAR3_NAME) || var_type != NC_BYTE ||
       natts != 0) ERR;
   if (nc_get_att_float(ncid, NC_GLOBAL, REDEF_ATT1_NAME, &float_in) != NC_ERANGE) ERR;
   if (nc_get_att_int(ncid, NC_GLOBAL, REDEF_ATT2_NAME, &int_in)) ERR;
   if (int_in != short_out) ERR;
   ret = nc_get_att_uchar(ncid, NC_GLOBAL, REDEF_ATT3_NAME, &uchar_in);
   if (format == NC_FORMAT_NETCDF4)
   {
      if (ret != NC_ERANGE) ERR;
   }
   else if (ret) ERR;
	 
   if (uchar_in != uchar_out) ERR;
   if (nc_close(ncid)) ERR;      
   return NC_NOERR;
}


