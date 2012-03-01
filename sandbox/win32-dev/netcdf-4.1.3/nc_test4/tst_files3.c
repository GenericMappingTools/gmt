/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test internal netcdf-4 file code. 
   $Id$
*/

#include <config.h>
#include <stdio.h>
#include <nc_tests.h>
#include "netcdf.h"
#include <hdf5.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <string.h>

#define NDIMS1 1
#define NDIMS 3   
#define FILE_NAME "tst_files3.nc"
#define X_LEN 120
#define Y_LEN 64
#define Z_LEN 128
#define NUM_TRIES 200

int dump_file2(const float *data, int docompression, int usedefdim)
{
   int ncmode, ncid, dimids[NDIMS], var;
   size_t start[NDIMS] = {0, 0, 0};
   size_t count[NDIMS] = {1, 1, Z_LEN};
/*   size_t count[NDIMS] = {X_LEN, Y_LEN, Z_LEN};*/

   ncmode = NC_CLOBBER|NC_NETCDF4;

   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR_RET;
   if (nc_def_dim(ncid, "time", X_LEN, &dimids[0])) ERR_RET;
   if (nc_def_dim(ncid, "lat", Y_LEN, &dimids[1])) ERR_RET;
   if (nc_def_dim(ncid, "lon", Z_LEN, &dimids[2])) ERR_RET;
   if (nc_def_var(ncid, "test", NC_FLOAT, NDIMS, dimids, &var)) ERR_RET;
   if (docompression) 
     if (nc_def_var_deflate(ncid, var, 1, 1, 1)) ERR_RET;
   if (nc_enddef(ncid)) ERR_RET;
   for (start[0] = 0; start[0] < X_LEN; start[0]++)
      for (start[1] = 0; start[1] < Y_LEN; start[1]++)
	 if (nc_put_vara_float(ncid, var, start, count, data)) ERR_RET;
   if (nc_close(ncid)) ERR_RET;

   return 0;
}

int dump_file(const float *data, int docompression, int usedefdim)
{
   int ncmode, ncid, dimids[NDIMS], var;
   size_t start[NDIMS] = {0, 0, 0}, count[NDIMS] = {X_LEN, Y_LEN, Z_LEN};
   ptrdiff_t stride[NDIMS] = {1, 1, 1};

   ncmode = NC_CLOBBER|NC_NETCDF4;

   if (nc_create(FILE_NAME, ncmode, &ncid)) ERR_RET;
   if (nc_def_dim(ncid, "time", X_LEN, &dimids[0])) ERR_RET;
   if (nc_def_dim(ncid, "lat", Y_LEN, &dimids[1])) ERR_RET;
   if (nc_def_dim(ncid, "lon", Z_LEN, &dimids[2])) ERR_RET;
   if (nc_def_var(ncid, "test", NC_FLOAT, NDIMS, dimids, &var)) ERR_RET;
   if (docompression) 
      if (nc_def_var_deflate(ncid, var, 1, 1, 1)) ERR_RET;
   if (nc_enddef(ncid)) ERR_RET;
   if (nc_put_vars_float(ncid, var, start, count, stride, data)) ERR_RET;
   if (nc_close(ncid)) ERR_RET;

   return 0;
}

int dump_file3(const float *data, int docompression, int usedefdim)
{
   int ncmode, ncid, dimids[NDIMS], var;
   size_t start[NDIMS] = {0, 0, 0}, count[NDIMS] = {X_LEN, Y_LEN, Z_LEN};
   ptrdiff_t stride[NDIMS] = {1, 1, 1};

   ncmode = NC_CLOBBER|NC_NETCDF4;

   if (nc_create(FILE_NAME, ncmode, &ncid)) ERR_RET;
   if (nc_def_dim(ncid, "time", X_LEN, &dimids[0])) ERR_RET;
   if (nc_def_dim(ncid, "lat", Y_LEN, &dimids[1])) ERR_RET;
   if (nc_def_dim(ncid, "lon", Z_LEN, &dimids[2])) ERR_RET;
   if (nc_def_var(ncid, "test", NC_FLOAT, NDIMS, dimids, &var)) ERR_RET;
   if (docompression) 
      if (nc_def_var_deflate(ncid, var, 1, 1, 1)) ERR_RET;
   if (nc_enddef(ncid)) ERR_RET;
   if (nc_put_vars_float(ncid, var, start, count, stride, data)) ERR_RET;
   if (nc_close(ncid)) ERR_RET;

   return 0;
}

int dump_hdf_file(const float *data, int docompression)
{
   hid_t file_id, dataset_id, dataspace_id, propid;
   hid_t file_spaceid, mem_spaceid, access_plistid, xfer_plistid;
   herr_t status;
   hsize_t dims[NDIMS] = {X_LEN, Y_LEN, Z_LEN};
   hsize_t start[NC_MAX_DIMS] = {0, 0, 0};
   hsize_t count[NC_MAX_DIMS] = {1, 1, Z_LEN};

   /* create file */
   file_id = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC,
		       H5P_DEFAULT, H5P_DEFAULT);

   /* create property for dataset */
   propid = H5Pcreate(H5P_DATASET_CREATE);

   if (docompression) 
   {
      if (H5Pset_layout(propid, H5D_CHUNKED) < 0) ERR;
      if (H5Pset_chunk(propid, NDIMS, dims) < 0) ERR;
/*     values[0]=9; */
/*     status = H5Pset_filter(propid, H5Z_FILTER_DEFLATE,0,1,&values[0]); */
/*     printf("deflat estatus is: %i\n",status); */
      /* sets defalte level */
      if (H5Pset_deflate(propid, 1)) ERR;
   }
   if ((file_spaceid = H5Screate_simple(NDIMS, dims, dims)) < 0) ERR;

   /* Set up the cache. */
   if ((access_plistid = H5Pcreate(H5P_DATASET_ACCESS)) < 0) ERR;
   if (H5Pset_chunk_cache(access_plistid, CHUNK_CACHE_NELEMS,
			  CHUNK_CACHE_SIZE, CHUNK_CACHE_PREEMPTION) < 0) ERR;

   /* Create the dataset. */
   if ((dataset_id = H5Dcreate2(file_id, "dset", H5T_NATIVE_FLOAT, file_spaceid, 
				H5P_DEFAULT, propid, access_plistid)) < 0) ERR;

/*   if ((file_spaceid = H5Dget_space(dataset_id)) < 0) ERR;*/
   if ((mem_spaceid = H5Screate_simple(NDIMS, count, NULL)) < 0) ERR;
   if ((xfer_plistid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;

   /* Write the dataset. */
   for (start[0] = 0; start[0] < X_LEN; start[0]++)
      for (start[1] = 0; start[1] < Y_LEN; start[1]++)
      {
	 if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET, start, NULL, 
				 count, NULL) < 0) ERR_RET;
	 if (H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, mem_spaceid, file_spaceid, 
		      xfer_plistid, data) < 0) ERR_RET;
      }

   /* Close property lists. */
   if (H5Pclose(propid) < 0) ERR;
   if (H5Pclose(access_plistid) < 0) ERR;
   if (H5Pclose(xfer_plistid) < 0) ERR;

   /* Close spaces. */
   if (H5Sclose(file_spaceid) < 0) ERR;
   if (H5Sclose(mem_spaceid) < 0) ERR;

   /* End access to the dataset and release resources used by it. */
   if (H5Dclose(dataset_id) < 0) ERR;

   /* close file */
   if (H5Fclose(file_id) < 0) ERR;
   return 0;
}

void
get_mem_used2(int *mem_used)
{
   char buf[30];
   FILE *pf;
   
   snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
   pf = fopen(buf, "r");
   if (pf) {
      unsigned size; /*       total program size */
      unsigned resident;/*   resident set size */
      unsigned share;/*      shared pages */
      unsigned text;/*       text (code) */
      unsigned lib;/*        library */
      unsigned data;/*       data/stack */
      /*unsigned dt;          dirty pages (unused in Linux 2.6)*/
      fscanf(pf, "%u %u %u %u %u %u", &size, &resident, &share, 
	     &text, &lib, &data);
      *mem_used = data;
   }
   else
      *mem_used = -1;
  fclose(pf);
}

int main(void)
{
   float data[X_LEN * Y_LEN * Z_LEN];
   int i;
 
   printf("\n*** Testing netcdf-4 file functions with caching.\n");

  /* Initialize data. */
   for (i = 0; i < (X_LEN * Y_LEN * Z_LEN); i++)
      data[i] = i;
   printf("*** testing a bunch of file writes with compressed data...\n");
   {
      int mem_used, mem_used1;

      printf("*** testing netcdf-4 writes...\n");
      for (i = 0; i < NUM_TRIES; i++) 
      {
	 get_mem_used2(&mem_used);
	 if (dump_file3(data, 1, 0)) ERR_RET;
	 get_mem_used2(&mem_used1);
	 if (mem_used1 - mem_used)
	    printf("delta %d bytes of memory for try %d\n", mem_used1 - mem_used, i);
      }
      printf("*** testing HDF5 writes...\n");
      for (i = 0; i < NUM_TRIES; i++)
      {
	 get_mem_used2(&mem_used);
	 if (dump_hdf_file(data, 1)) ERR_RET;
	 get_mem_used2(&mem_used1);
	 if (mem_used1 - mem_used)
	    printf("delta %d bytes of memory for try %d\n", mem_used1 - mem_used, i);
      }
      printf("*** testing netcdf-4 writes again...\n");
      for (i = 0; i < NUM_TRIES; i++)
      {
	 get_mem_used2(&mem_used);
	 if (dump_file2(data, 1, 0)) ERR_RET;
	 get_mem_used2(&mem_used1);
	 if (mem_used1 - mem_used)
	    printf("delta %d bytes of memory for try %d\n", mem_used1 - mem_used, i);
      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
