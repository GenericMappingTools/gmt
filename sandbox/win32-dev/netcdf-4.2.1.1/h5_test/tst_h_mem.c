/* This is part of the netCDF package. Copyright 2007 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 dataset code, even more. These are not intended to be
   exhaustive tests, but they use HDF5 the same way that netCDF-4
   does, so if these tests don't work, than netCDF-4 won't work
   either.
*/

#include "nc_tests.h"
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_mem.h5"
#define STR_LEN 255

const char*
nc_strerror(int ncerr)
{
    static char msg[1024];
    snprintf(msg,sizeof(msg),"error: %d\n",ncerr);
    return msg;
}

int
main()
{
   printf("\n*** Checking HDF5 memory use.\n");
   printf("*** checking HDF5 memory use writing along unlimited dimension...");
   {
#define NDIMS1 1
#define NUM_DATASETS 10000
#define CHUNKSIZE 1

      hid_t fapl_id, fcpl_id;
      hid_t datasetid[NUM_DATASETS];
      hid_t fileid, grpid, spaceid, plistid;
      hsize_t chunksize[NDIMS1], dimsize[NDIMS1], maxdimsize[NDIMS1];
      char var_name[STR_LEN + 1];
      int v;

      /* Create file, setting latest_format in access propertly list
       * and H5P_CRT_ORDER_TRACKED in the creation property list. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Open root group. */
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create 1 D data space with unlimited dimension. */
      dimsize[0] = 0;
      maxdimsize[0] = H5S_UNLIMITED;
      if ((spaceid = H5Screate_simple(NDIMS1, dimsize, maxdimsize)) < 0) ERR;

      /* Create property list. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;

      /* Set up chunksizes. */
      chunksize[0] = CHUNKSIZE;
      if (H5Pset_chunk(plistid, NDIMS1, chunksize) < 0)ERR;

      /* Create the variables. */
      for (v = 0; v < NUM_DATASETS; v++)
      {
	 sprintf(var_name, "var_%d", v);
/*	 printf("creating var %s\n", var_name);*/
	 if ((datasetid[v] = H5Dcreate(grpid, var_name, H5T_NATIVE_INT,
				    spaceid, plistid)) < 0) ERR_RET;
      }

      /* Close the datasets. */
      for (v = 0; v < NUM_DATASETS; v++)
	 if (H5Dclose(datasetid[v]) < 0) ERR_RET;

      /* Close everything. */
      if (H5Pclose(fapl_id) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

/*       if ((datasetid = H5Dopen1(grpid, SIMPLE_VAR_NAME)) < 0) ERR; */
/*       if ((spaceid = H5Dget_space(datasetid)) < 0) */
/*       if (H5Sget_simple_extent_dims(spaceid, fdims, fmaxdims) > 0) ERR; */
/*       if (H5Dread(datasetid, H5T_NATIVE_INT, H5S_ALL, */
/* 		  spaceid, H5P_DEFAULT, data_in) < 0) ERR; */

/*       /\* Check the data. *\/ */
/*       for (x = 0; x < NX; x++) */
/* 	 for (y = 0; y < NY; y++) */
/* 	    if (data_in[x][y] != data_out[x][y]) ERR_RET; */

      if (H5Pclose(fapl_id) < 0 ||
/*	  H5Dclose(datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||*/
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
