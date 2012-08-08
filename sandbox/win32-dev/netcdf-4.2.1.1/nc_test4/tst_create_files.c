/*
 Copyright 2007, UCAR/Unidata
 See COPYRIGHT file for copying and redistribution conditions.

 This program creates a test file.

 $Id$
*/
#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* We will create this file. */
#define FILE_NAME "tst_floats_1D.nc"

int
main(int argc, char **argv)
{

#ifdef USE_PARALLEL
   MPI_Init(&argc, &argv);
#endif
    printf("\n*** Create some files for testing benchmarks.\n");

#ifdef LARGE_FILE_TESTS
    printf("*** Creating very large 3D file...");
    {
#define LARGE_VAR_NAME "Really_Large_Variable"
#define LARGE_FILE "tst_large.nc"
#define NDIMS3 3
#define D0 400
#define D1 2000
#define D2 2000

       int ncid, dimids[NDIMS3], varid;
       size_t start[NDIMS3], count[NDIMS3];
       size_t dim_len[NDIMS3] = {D0, D1, D2};
       int chunk_sizes[NDIMS3] = {1, D1, D2};
       float *data;
       char file_name[NC_MAX_NAME * 2 + 1];
       int d, i; 

       /* Initialize the data to random floats. */
       if (!(data = (float *)malloc(D1 * D2 * sizeof(float)))) ERR;
       for (i = 0; i < D1 * D2; i++)
	  data[i] = (float)rand();

       /* User TEMP_LARGE as the directory. */
       if (strlen(TEMP_LARGE) + strlen(LARGE_FILE) > NC_MAX_NAME * 2) ERR;
       sprintf(file_name, "%s/%s", TEMP_LARGE, LARGE_FILE);

       /* Create file with 3 dims, one variable. */
       if (nc_create(file_name, NC_NETCDF4|NC_CLASSIC_MODEL, &ncid)) ERR;
       if (nc_def_dim(ncid, "d0", D0, &dimids[0])) ERR;
       if (nc_def_dim(ncid, "d1", D1, &dimids[1])) ERR;
       if (nc_def_dim(ncid, "d2", D2, &dimids[2])) ERR;
       if (nc_def_var(ncid, LARGE_VAR_NAME, NC_FLOAT, NDIMS3, dimids, &varid)) ERR;
       if (nc_def_var_chunking(ncid, varid, NULL, chunk_sizes, NULL)) ERR;
       if (nc_enddef(ncid)) ERR;

       /* Write the data one slice at a time. */
       start[0] = 0;
       count[0] = 1;
       for (d = 1; d < NDIMS3; d++)
       {
	  start[d] = 0;
	  count[d] = dim_len[d];
       }
       for ( ; start[0] < D0; (start[0])++)
	  if (nc_put_vara_float(ncid, varid, start, count, (const float *) data)) 
	     ERR_RET;

       /* Close up shop. */
       if (nc_close(ncid)) ERR;
       free(data);
    }
    SUMMARIZE_ERR;
#endif /* LARGE_FILE_TESTS */

    printf("*** Creating a file with floats...");
    {
#define DIM_NAME "stupidity"
#define NUMDIMS 1
#define DIMLEN 10000

       int ncid, dimids[NUMDIMS], varid;
       char var_name[NC_MAX_NAME + 1] = {"Billy-Bob"};
       int ndims, nvars, natts, unlimdimid;
       nc_type xtype;
       char name_in[NC_MAX_NAME + 1];
       size_t len;
       float data[DIMLEN], data_in[DIMLEN];
       int i; 

       for (i = 0; i < DIMLEN; i++)
	  data[i] = ((float)rand() / (float)(RAND_MAX));

       /* Create a netCDF netCDF-4/HDF5 format file, with 1 var. */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;*/
       if (nc_create(FILE_NAME, 0, &ncid)) ERR;
       if (nc_def_dim(ncid, DIM_NAME, DIMLEN, dimids)) ERR;
       if (nc_def_var(ncid, var_name, NC_FLOAT, NUMDIMS, dimids, &varid)) ERR;
       if (nc_enddef(ncid)) ERR;
       if (nc_put_var_float(ncid, varid, data)) ERR;
       if (nc_close(ncid)) ERR;
       
       /* Reopen and check the file. */
       if (nc_open(FILE_NAME, 0, &ncid)) ERR;
       if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
       if (ndims != NUMDIMS || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
       if (nc_inq_dimids(ncid, &ndims, dimids, 1)) ERR;
       if (ndims != 1 || dimids[0] != 0) ERR;
       if (nc_inq_dim(ncid, 0, name_in, &len)) ERR;
       if (strcmp(name_in, DIM_NAME) || len != DIMLEN) ERR;
       if (nc_inq_var(ncid, 0, name_in, &xtype, &ndims, dimids, &natts)) ERR;
       if (strcmp(name_in, var_name) || xtype != NC_FLOAT || ndims != 1 || 
	   dimids[0] != 0 || natts != 0) ERR;
       if (nc_get_var_float(ncid, 0, data_in)) ERR;
       for (i = 0; i < DIMLEN; i++)
	  if (data_in[i] != data[i]) ERR;
       
       if (nc_close(ncid)) ERR;
    }

    SUMMARIZE_ERR;
    printf("*** Creating files of various dimensions with various types...");
    {
#define TOTAL_SIZE 100000
#define MAX_DIMS 6
#define VAR_NAME "Unimaginatively_Named_Variable"
#define MAX_TYPES 3

       int ncid, dimids[MAX_DIMS], varid;
       char dim_name[NC_MAX_NAME + 1], file_name[NC_MAX_NAME + 1];
       char type_name[MAX_TYPES][NC_MAX_NAME + 1] = {"floats", "ints", "shorts"};
       int typeid[MAX_TYPES] = {NC_FLOAT, NC_INT, NC_SHORT};
       size_t len;
       float fdata[TOTAL_SIZE];
       int idata[TOTAL_SIZE];
       short sdata[TOTAL_SIZE];
       void *data[MAX_TYPES];
       int ndims;
       int i, d, t; 

       /* Initialize the data to random floats. */
       for (i = 0; i < TOTAL_SIZE; i++)
       {
	  fdata[i] = (float)rand();
	  idata[i] = rand();
	  sdata[i] = (short)rand();
       }
       data[0] = fdata;
       data[1] = idata;
       data[2] = sdata;

       /* Do the file creation process below for each type. */
       for (t = 0; t < MAX_TYPES; t++)
       {
	  /* Create MAX_DIMS files, each with different number of
	   * dimensions. */
	  for (ndims = 1; ndims <= MAX_DIMS; ndims++)
	  {
	     sprintf(file_name, "tst_%s2_%dD.nc", type_name[t], ndims);
	     if (nc_create(file_name, 0, &ncid)) ERR;
	     for (len = pow(TOTAL_SIZE, (float)1/ndims), d = 0; d < ndims; d++)
	     {
		sprintf(dim_name, "dim_%d", d);
		if (nc_def_dim(ncid, dim_name, len, &dimids[d])) ERR;
	     }
	     if (nc_def_var(ncid, VAR_NAME, typeid[t], ndims, dimids, &varid)) ERR;
	     if (nc_enddef(ncid)) ERR;
	     if (nc_put_var(ncid, varid, data[t])) ERR;
	     if (nc_close(ncid)) ERR;
	  } /* next ndims. */
       } /* next type */
    }

    SUMMARIZE_ERR;
    printf("*** Creating file like in http://hdfeos.org/workshops/ws06/presentations/Pourmal/HDF5_IO_Perf.pdf...");
    {
#define XLEN 256
#define YLEN 256
#define ZLEN 1024
#define NDIMS 3
#define E_VAR_NAME "Like_Elenas_Benchmark"
#define ELENA_FILE_NAME "tst_elena_int_3D.nc"
#define E_TYPE_SIZE 4       

       int ncid, dimids[NDIMS], varid;
       int *idata;
       int i; 

       /* Initialize data to random int between 0 and 255. */
       if (!(idata = malloc(XLEN * YLEN * ZLEN * E_TYPE_SIZE)))
	  return NC_ENOMEM;
       for (i = 0; i < XLEN * YLEN * ZLEN; i++)
	  idata[i] = rand() % 255;

       /* Create a 3D file with one var. */
       if (nc_create(ELENA_FILE_NAME, NC_CLOBBER, &ncid)) ERR;
       if (nc_def_dim(ncid, "z", ZLEN, &dimids[0])) ERR;
       if (nc_def_dim(ncid, "y", YLEN, &dimids[1])) ERR;
       if (nc_def_dim(ncid, "x", XLEN, &dimids[2])) ERR;
       if (nc_def_var(ncid, E_VAR_NAME, NC_INT, NDIMS, dimids, &varid)) ERR;
       if (nc_enddef(ncid)) ERR;

       /* Write the data. */
       if (nc_put_var(ncid, varid, idata)) ERR;
       if (nc_close(ncid)) ERR;
    }

    SUMMARIZE_ERR;
    printf("*** Creating super simple file to test non-sequential reads...");
    {
#define DIM_LEN 10
#define NDIMS1 1
#define S_VAR_NAME "My_Favorite_Numbers_in_order"
#define S_FILE_NAME "tst_simple.nc"

       int ncid, dimids[NDIMS1], varid;
       int data[DIM_LEN];
       int i; 

       /* Initialize data to my favorite numbers. */
       for (i = 0; i < DIM_LEN; i++)
	  data[i] = i;

       /* Create a file with one var. */
       if (nc_create(S_FILE_NAME, NC_CLOBBER, &ncid)) ERR;
       if (nc_def_dim(ncid, "a", DIM_LEN, &dimids[0])) ERR;
       if (nc_def_var(ncid, S_VAR_NAME, NC_INT, NDIMS1, dimids, &varid)) ERR;
       if (nc_enddef(ncid)) ERR;

       /* Write the data. */
       if (nc_put_var(ncid, varid, data)) ERR;
       if (nc_close(ncid)) ERR;
    }

    SUMMARIZE_ERR;
    printf("*** Creating very simple 3D file...");
    {
#define SIMPLE_VAR_NAME "Paul_Mau_Dib"
#define MAX_TYPES 3

       int ncid, dimids[MAX_DIMS], varid;
       char dim_name[NC_MAX_NAME + 1], file_name[NC_MAX_NAME + 1];
       char type_name[MAX_TYPES][NC_MAX_NAME + 1] = {"floats", "ints", "shorts"};
       int typeid[MAX_TYPES] = {NC_FLOAT, NC_INT, NC_SHORT};
       size_t len;
       float fdata[TOTAL_SIZE];
       int idata[TOTAL_SIZE];
       short sdata[TOTAL_SIZE];
       void *data[MAX_TYPES];
       int ndims;
       int i, d, t; 

       /* Initialize the data to random floats. */
       for (i = 0; i < TOTAL_SIZE; i++)
       {
	  fdata[i] = (float)rand();
	  idata[i] = rand();
	  sdata[i] = (short)rand();
       }
       data[0] = fdata;
       data[1] = idata;
       data[2] = sdata;

       /* Do the file creation process below for each type. */
       for (t = 0; t < MAX_TYPES; t++)
       {
	  /* Create MAX_DIMS files, each with different number of
	   * dimensions. */
	  for (ndims = 1; ndims <= MAX_DIMS; ndims++)
	  {
	     sprintf(file_name, "tst_%s2_%dD.nc", type_name[t], ndims);
	     if (nc_create(file_name, 0, &ncid)) ERR;
	     for (len = pow(TOTAL_SIZE, (float)1/ndims), d = 0; d < ndims; d++)
	     {
		sprintf(dim_name, "dim_%d", d);
		if (nc_def_dim(ncid, dim_name, len, &dimids[d])) ERR;
	     }
	     if (nc_def_var(ncid, SIMPLE_VAR_NAME, typeid[t], ndims, dimids, &varid)) ERR;
	     if (nc_enddef(ncid)) ERR;
	     if (nc_put_var(ncid, varid, data[t])) ERR;
	     if (nc_close(ncid)) ERR;
	  } /* next ndims. */
       } /* next type */
    }
    SUMMARIZE_ERR;

#ifdef USE_PARALLEL
   MPI_Finalize();
#endif   
    FINAL_RESULTS;
}

