/* This is part of the netCDF package.  Copyright 2005-2011,
   University Corporation for Atmospheric Research/Unidata. See
   COPYRIGHT file for conditions of use.

   Test that NetCDF-4 can read HDF4 files.
*/
#include <config.h>
#include <nc_tests.h>
#include <hdf5.h>
#include <H5DSpublic.h>
#include <mfhdf.h>

#define FILE_NAME "tst_interops2.h4"

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF4/NetCDF-4 interoperability...\n");
   printf("*** testing that netCDF can read a HDF4 file with some ints...");
   {
#define PRES_NAME "pres"
#define LAT_LEN 3
#define LON_LEN 2
#define DIMS_2 2

      int32 sd_id, sds_id;
      int32 dim_size[DIMS_2] = {LAT_LEN, LON_LEN};
      int32 start[DIMS_2] = {0, 0}, edge[DIMS_2] = {LAT_LEN, LON_LEN};
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in;
      size_t len_in;
      int data_out[LAT_LEN][LON_LEN], data_in[LAT_LEN][LON_LEN];
      size_t nstart[DIMS_2] = {0, 0}, ncount[DIMS_2] = {LAT_LEN, LON_LEN};
      size_t nindex[DIMS_2] = {0, 0};
      int scalar_data_in = 0;
      int i, j;

      /* Create some data. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    data_out[i][j] = j;

      /* Create a file with one SDS, containing our phony data. */
      sd_id = SDstart(FILE_NAME, DFACC_CREATE);
      sds_id = SDcreate(sd_id, PRES_NAME, DFNT_INT32, DIMS_2, dim_size);
      if (SDwritedata(sds_id, start, NULL, edge, (void *)data_out)) ERR;
      if (SDendaccess(sds_id)) ERR;
      if (SDend(sd_id)) ERR;

      /* Now open with netCDF and check the contents. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; 
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR; 
      if (ndims_in != 2 || nvars_in != 1 || natts_in != 0 || unlimdim_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, NULL, &len_in)) ERR;
      if (len_in != LAT_LEN) ERR;
      if (nc_inq_dim(ncid, 1, NULL, &len_in)) ERR;
      if (len_in != LON_LEN) ERR;
      
      /* Read the data through a vara function from the netCDF API. */
      if (nc_get_vara(ncid, 0, nstart, ncount, data_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    if (data_in[i][j] != data_out[i][j]) ERR;

      /* Reset for next test. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    data_in[i][j] = -88;

      /* Read the data through a vara_int function from the netCDF API. */
      if (nc_get_vara_int(ncid, 0, nstart, ncount, data_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    if (data_in[i][j] != data_out[i][j]) ERR;

      /* Reset for next test. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    data_in[i][j] = -88;

      /* Read the data through a var_int function from the netCDF API. */
      if (nc_get_var_int(ncid, 0, data_in)) ERR;
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	    if (data_in[i][j] != data_out[i][j]) ERR;

      /* Read the data through a var1 function from the netCDF API. */
      for (i = 0; i < LAT_LEN; i++)
	 for (j = 0; j < LON_LEN; j++)
	 {
	    nindex[0] = i;
	    nindex[1] = j;
	    if (nc_get_var1(ncid, 0, nindex, &scalar_data_in)) ERR;
	    if (scalar_data_in != data_out[i][j]) ERR;
	    scalar_data_in = -88; /* reset */
	    if (nc_get_var1_int(ncid, 0, nindex, &scalar_data_in)) ERR;
	    if (scalar_data_in != data_out[i][j]) ERR;
	 }

      if (nc_close(ncid)) ERR; 
   }
   SUMMARIZE_ERR;
   printf("*** testing with a more complex HDF4 file...");
   {
#define Z_LEN 3
#define Y_LEN 2
#define X_LEN 5
#define DIMS_3 3
#define NUM_TYPES 8

      int32 sd_id, sds_id;
      int32 dim_size[DIMS_3] = {Z_LEN, Y_LEN, X_LEN};
      int dimids_in[DIMS_3];
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in;
      size_t len_in;
      nc_type type_in;
      int hdf4_type[NUM_TYPES] = {DFNT_FLOAT32, DFNT_FLOAT64, 
				  DFNT_INT8, DFNT_UINT8, DFNT_INT16, 
				  DFNT_UINT16, DFNT_INT32, DFNT_UINT32};
      int netcdf_type[NUM_TYPES] = {NC_FLOAT, NC_DOUBLE, 
				  NC_BYTE, NC_UBYTE, NC_SHORT, 
				  NC_USHORT, NC_INT, NC_UINT};
      char tmp_name[NC_MAX_NAME + 1], name_in[NC_MAX_NAME + 1];
      char dim_name[NC_MAX_NAME + 1][DIMS_3] = {"z", "y", "x"};
      int d, t;

      /* Create a HDF4 SD file. */
      sd_id = SDstart (FILE_NAME, DFACC_CREATE);

      /* Create some HDF4 datasets. */
      for (t = 0; t < NUM_TYPES; t++)
      {
	 sprintf(tmp_name, "hdf4_dataset_type_%d", t);
	 if ((sds_id = SDcreate(sd_id, tmp_name, hdf4_type[t], 
				DIMS_3, dim_size)) == FAIL) ERR;	    
	 /* Set up dimensions. By giving them the same names for each
	  * dataset, I am specifying that they are shared
	  * dimensions. */
	 for (d = 0; d < DIMS_3; d++)
	 {
	    int32 dimid;
	    if ((dimid = SDgetdimid(sds_id, d)) == FAIL) ERR;
	    if (SDsetdimname(dimid, dim_name[d])) ERR;
	 }
	 if (SDendaccess(sds_id)) ERR;
      }
      if (SDend(sd_id)) ERR;

      /* Open the file with netCDF and check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != DIMS_3 || nvars_in != NUM_TYPES || natts_in != 0 || unlimdim_in != -1) ERR;
      if (nc_inq_dim(ncid, 0, NULL, &len_in)) ERR;
      if (len_in != Z_LEN) ERR;
      if (nc_inq_dim(ncid, 1, NULL, &len_in)) ERR;
      if (len_in != Y_LEN) ERR;
      if (nc_inq_dim(ncid, 2, NULL, &len_in)) ERR;
      if (len_in != X_LEN) ERR;
      for (t = 0; t < NUM_TYPES; t++)
      {
	 if (nc_inq_var(ncid, t, name_in, &type_in, &ndims_in, 
			dimids_in, &natts_in)) ERR;
	 if (type_in != netcdf_type[t] || ndims_in != DIMS_3 ||
	     dimids_in[0] != 0 || dimids_in[2] != 2 || dimids_in[2] != 2 || 
	     natts_in != 0) ERR;
      }
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

