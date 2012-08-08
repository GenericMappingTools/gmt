/*
 Copyright 2007, UCAR/Unidata
 See COPYRIGHT file for copying and redistribution conditions.

 This program (quickly, but not throughly) tests the large file
 features. It turns off fill mode to quickly create an 8 gb file, and
 write one value is written, nothing is read.

 $Id$
*/
#include <config.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <stdio.h>
#include <string.h>

/* Size, in bytes, of a double. */
#define DOUBLE_SIZE 8

/* This is one megabyte (2^20), in decimal. */
#define MEGABYTE 1048576

/* This is the magic number for classic format limits: 2 GiB - 4
   bytes. */
#define MAX_CLASSIC_BYTES 2147483644

/* This is the magic number for 64-bit offset format limits: 4 GiB - 4
   bytes. */
#define MAX_64OFFSET_BYTES 4294967292

/* Handy for constucting tests. */
#define QTR_CLASSIC_MAX (MAX_CLASSIC_BYTES/4)

/* We will create this file. */
#define FILE_NAME "tst_large.nc"

int
main(int argc, char **argv)
{
   char file_name[NC_MAX_NAME + 1];

#ifdef USE_PARALLEL
   MPI_Init(&argc, &argv);
#endif

    printf("\n*** Testing really large files in netCDF-4/HDF5 format, quickly.\n");

    printf("*** Testing create of simple, but large, file...");
    {
#define DIM_NAME "Time_in_nanoseconds"
#define NUMDIMS 1
#define NUMVARS 4

       int ncid, dimids[NUMDIMS], varid[NUMVARS];
       size_t chunksize[NUMDIMS];
       char var_name[NUMVARS][NC_MAX_NAME + 1] = {"England", "Scotland", "Ireland", "Wales"};
       size_t index[NUMDIMS] = {QTR_CLASSIC_MAX-1};
       int ndims, nvars, natts, unlimdimid;
       nc_type xtype;
       char name_in[NC_MAX_NAME + 1];
       size_t len;
       double pi = 3.1459, pi_in;
       int i; 

       /* Create a netCDF netCDF-4/HDF5 format file, with 4 vars. */
	sprintf(file_name, "%s/%s", TEMP_LARGE, FILE_NAME);
       if (nc_create(file_name, NC_NETCDF4, &ncid)) ERR;
       if (nc_set_fill(ncid, NC_NOFILL, NULL)) ERR;
       if (nc_def_dim(ncid, DIM_NAME, QTR_CLASSIC_MAX, dimids)) ERR;
       chunksize[0] = MEGABYTE/DOUBLE_SIZE;
       for (i = 0; i < NUMVARS; i++)
       {
	  if (nc_def_var(ncid, var_name[i], NC_DOUBLE, NUMDIMS, 
			 dimids, &varid[i])) ERR;
	  if (nc_def_var_chunking(ncid, i, 0, chunksize)) ERR;
       }
       if (nc_enddef(ncid)) ERR;
       for (i = 0; i < NUMVARS; i++)
	  if (nc_put_var1_double(ncid, i, index, &pi)) ERR;
       if (nc_close(ncid)) ERR;
       
       /* Reopen and check the file. */
       if (nc_open(file_name, 0, &ncid)) ERR;
       if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
       if (ndims != NUMDIMS || nvars != NUMVARS || natts != 0 || unlimdimid != -1) ERR;
       if (nc_inq_dimids(ncid, &ndims, dimids, 1)) ERR;
       if (ndims != 1 || dimids[0] != 0) ERR;
       if (nc_inq_dim(ncid, 0, name_in, &len)) ERR;
       if (strcmp(name_in, DIM_NAME) || len != QTR_CLASSIC_MAX) ERR;
       for (i = 0; i < NUMVARS; i++)
       {
	  if (nc_inq_var(ncid, i, name_in, &xtype, &ndims, dimids, &natts)) ERR;
	  if (strcmp(name_in, var_name[i]) || xtype != NC_DOUBLE || ndims != 1 || 
	      dimids[0] != 0 || natts != 0) ERR;
	  if (nc_get_var1_double(ncid, i, index, &pi_in)) ERR;
	  if (pi_in != pi) ERR;
       }
       if (nc_close(ncid)) ERR;
    }

    SUMMARIZE_ERR;

#ifdef USE_PARALLEL
   MPI_Finalize();
#endif   

    FINAL_RESULTS;
}

