/* This is part of the netCDF package.
   Copyright 2011 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 chunking. 
*/

#include <nc_tests.h>

#define FILE_NAME "tst_chunks2.nc"
#define MAX_WASTE 25.0
#define NUM_RANDOM_TESTS 3
#define NDIMS3 3

/* Calculate the waste of the chunking. A waste of 10% means the
 * chunked data is 10% larget then the unchunked data. */
static int
calculate_waste(int ndims, size_t *dimlen, size_t *chunksize, float *waste)
{
   int d;
   float chunked = 1, unchunked = 1;
   size_t *num_chunks;
   size_t chunk_size = 1;

   assert(waste && dimlen && chunksize && ndims);
   if (!(num_chunks = calloc(ndims, sizeof(size_t)))) ERR;

#ifdef PRINT_CHUNK_WASTE_REPORT
   printf("\n");
#endif
   /* Caclulate the total space taken up by the chunked data. */
   for (d = 0; d < ndims; d++)
   {
      /* How many chunks along this dimension are required to hold all the data? */
      for (num_chunks[d] = 0; (num_chunks[d] * chunksize[d]) < (dimlen[d] ? dimlen[d] : 1); 
	   num_chunks[d]++)
	 ;
      chunked *= (num_chunks[d] * chunksize[d]);
   }
   
   /* Calculate the minimum space required for this data
    * (i.e. unchunked) or one record of it. */
   for (d = 0; d < ndims; d++)
      unchunked *= (dimlen[d] ? dimlen[d] : 1);

#ifdef PRINT_CHUNK_WASTE_REPORT
   printf("size for unchunked %g elements; size for chunked %g elements\n", 
	  unchunked, chunked);
#endif

   /* Percent of the chunked file that is wasted space. */
   *waste = ((float)(chunked - unchunked) / (float)chunked) * 100.0;

#ifdef PRINT_CHUNK_WASTE_REPORT
   printf("\ndimlen\tchunksize\tnum_chunks\n");
#endif
   for (d = 0; d < ndims; d++)
   {
#ifdef PRINT_CHUNK_WASTE_REPORT
      printf("%ld\t%ld\t\t%ld\n", (long int)dimlen[d], (long int)chunksize[d], 
	     (long int)num_chunks[d]);
#endif
      chunk_size *= chunksize[d];
   }
#ifdef PRINT_CHUNK_WASTE_REPORT
   printf("size of chunk: %ld elements; wasted space: %2.2f percent\n", 
	  (long int)chunk_size, *waste);
#endif
   
   free(num_chunks);
   return 0;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 variable chunking.\n");
   printf("**** testing default chunksizes...");
   {
#define NDIMS3 3
#define NUM_VARS 1
#define Y_NAME "y"
#define X_NAME "x"
#define Z_NAME "z"
#define VAR_NAME_JOE "joe"
#define XDIM_LEN 2
#define YDIM_LEN 5
#define ZDIM_LEN 3000

      int varid, ncid, dims[NDIMS3], dims_in[NDIMS3];
      int ndims, nvars, ngatts, unlimdimid, natts;
      char name_in[NC_MAX_NAME + 1];
      nc_type type_in;
      size_t len_in[NDIMS3];
      int storage = 0;
      size_t chunksizes[NDIMS3];
      float waste = 0;

      /* Create a file with 3D var, turn on chunking, but don't provide chunksizes. */
      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
      if (nc_def_dim(ncid, X_NAME, XDIM_LEN, &dims[0])) ERR;
      if (nc_def_dim(ncid, Y_NAME, YDIM_LEN, &dims[1])) ERR;
      if (nc_def_dim(ncid, Z_NAME, ZDIM_LEN, &dims[2])) ERR;
      if (nc_def_var(ncid, VAR_NAME_JOE, NC_FLOAT, NDIMS3, dims, &varid)) ERR;
      if (nc_def_var_chunking(ncid, 0, NC_CHUNKED, NULL)) ERR;

      /* Check it out. */
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS3 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME_JOE) || type_in != NC_FLOAT || ndims != NDIMS3 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || dims_in[2] != dims[2] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in[0])) ERR;
      if (strcmp(name_in, X_NAME) || len_in[0] != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in[1])) ERR;
      if (strcmp(name_in, Y_NAME) || len_in[1] != YDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 2, name_in, &len_in[2])) ERR;
      if (strcmp(name_in, Z_NAME) || len_in[2] != ZDIM_LEN) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and check again. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
      if (nvars != NUM_VARS || ndims != NDIMS3 || ngatts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims, dims_in, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME_JOE) || type_in != NC_FLOAT || ndims != NDIMS3 ||
	  dims_in[0] != dims[0] || dims_in[1] != dims[1] || dims_in[2] != dims[2] || natts != 0) ERR;
      if (nc_inq_dim(ncid, 0, name_in, &len_in[0])) ERR;
      if (strcmp(name_in, X_NAME) || len_in[0] != XDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 1, name_in, &len_in[1])) ERR;
      if (strcmp(name_in, Y_NAME) || len_in[1] != YDIM_LEN) ERR;
      if (nc_inq_dim(ncid, 2, name_in, &len_in[2])) ERR;
      if (strcmp(name_in, Z_NAME) || len_in[2] != ZDIM_LEN) ERR;
      if (nc_inq_var_chunking(ncid, 0, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (calculate_waste(NDIMS3, len_in, chunksizes, &waste)) ERR;
      /*if (waste > MAX_WASTE) ERR;*/
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes some more for a 3D var...");
   {
#define NDIMS3 3
#define VAR_NAME "op-amp"

      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3] = {1, 11, 152750};
				  
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

      /* Create a few dimensions. */
      for (d = 0; d < NDIMS3; d++)
      {
	 sprintf(dim_name, "dim_%d", d);
	 if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
      }
      
      /* Define a var with these dimensions, and turn on chunking. */
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

      /* Check how default chunking worked. */
      if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
/*      if (waste > MAX_WASTE) ERR;*/

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes even more for a 3D var...");
   {
      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3] = {1804289383, 846930886, 1681692777};
				  
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

      /* Create a few dimensions. */
      for (d = 0; d < NDIMS3; d++)
      {
	 sprintf(dim_name, "dim_%d", d);
	 if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
      }
      
      /* Define a var with these dimensions, and turn on chunking. */
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

      /* Check how default chunking worked. */
      if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
/*      if (waste > MAX_WASTE) ERR;*/

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes even even more for a 3D var...");
   {
      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3] = {1714636915, 1957747793, 424238335};
				  
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

      /* Create a few dimensions. */
      for (d = 0; d < NDIMS3; d++)
      {
	 sprintf(dim_name, "dim_%d", d);
	 if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
      }
      
      /* Define a var with these dimensions, and turn on chunking. */
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

      /* Check how default chunking worked. */
      if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
/*      if (waste > MAX_WASTE) ERR;*/

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes some more for a 3D var...");
   {
#define NDIMS3 3
#define VAR_NAME "op-amp"

      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3] = {1967513926, 1365180540, 426};
				  
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

      /* Create a few dimensions. */
      for (d = 0; d < NDIMS3; d++)
      {
	 sprintf(dim_name, "dim_%d", d);
	 if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
      }
      
      /* Define a var with these dimensions, and turn on chunking. */
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

      /* Check how default chunking worked. */
      if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
/*      if (waste > MAX_WASTE) ERR;*/

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes for very large 3D var...");
   {
#define NDIMS3 3

      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3] = {1804289383, 846930886, 1681692777};
				  
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

      /* Create a few dimensions. */
      for (d = 0; d < NDIMS3; d++)
      {
	 sprintf(dim_name, "dim_%d", d);
	 if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
      }
      
      /* Define a var with these dimensions, and turn on chunking. */
      if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
      if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

      /* Check how default chunking worked. */
      if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
      if (storage != NC_CHUNKED) ERR;
      if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
/*      if (waste > MAX_WASTE) ERR;*/

      if (nc_close(ncid)) ERR;

      /* Open the file and check. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes some randomly sized 3D vars...");
   {
#define NDIMS3 3

      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3];
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d, t;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      for (t = 0; t < NUM_RANDOM_TESTS; t++)
      {
	 if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

	 /* Create a few dimensions. */
	 for (d = 0; d < NDIMS3; d++)
	 {
	    dim_len[d] = rand();
	    sprintf(dim_name, "dim_%d", d);
	    if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
	 }
      
	 /* Define a var with these dimensions, and turn on chunking. */
	 if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
	 if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

	 /* Check how well default chunking worked. */
	 if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
	 if (storage != NC_CHUNKED) ERR;
	 if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
	 if (waste > MAX_WASTE) ERR;

	 if (nc_close(ncid)) ERR;
      }
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes some randomly sized 3D vars, with one small dimension...");
   {
      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3];
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d, t;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      for (t = 0; t < NUM_RANDOM_TESTS; t++)
      {
	 if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

	 dim_len[0] = rand();
	 dim_len[1] = rand();
	 dim_len[2] = rand() % 1000;
	 /* Create a few dimensions. */
	 for (d = 0; d < NDIMS3; d++)
	 {
	    sprintf(dim_name, "dim_%d", d);
	    if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
	 }
      
	 /* Define a var with these dimensions, and turn on chunking. */
	 if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
	 if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

	 /* Check how well default chunking worked. */
	 if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
	 if (storage != NC_CHUNKED) ERR;
	 if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
	 if (waste > MAX_WASTE) ERR;

	 if (nc_close(ncid)) ERR;
      }
   }
   SUMMARIZE_ERR;
   printf("**** testing default chunksizes some randomly sized 3D vars, with two small dimensions...");
   {
      int varid, ncid;
      int dimids[NDIMS3];
      size_t dim_len[NDIMS3];
      int storage = 0;
      size_t chunksizes[NDIMS3];
      int d, t;
      char dim_name[NC_MAX_NAME + 1];
      float waste;

      for (t = 0; t < NUM_RANDOM_TESTS; t++)
      {
	 if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;

	 dim_len[0] = rand();
	 dim_len[1] = rand() % 1000;
	 dim_len[2] = rand() % 1000;
	 /* Create a few dimensions. */
	 for (d = 0; d < NDIMS3; d++)
	 {
	    sprintf(dim_name, "dim_%d", d);
	    if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR;
	 }
      
	 /* Define a var with these dimensions, and turn on chunking. */
	 if (nc_def_var(ncid, VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
	 if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, NULL)) ERR;

	 /* Check how well default chunking worked. */
	 if (nc_inq_var_chunking(ncid, varid, &storage, chunksizes)) ERR;
	 if (storage != NC_CHUNKED) ERR;
	 if (calculate_waste(NDIMS3, dim_len, chunksizes, &waste)) ERR;
	 if (waste > MAX_WASTE) ERR;

	 if (nc_close(ncid)) ERR;
      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
