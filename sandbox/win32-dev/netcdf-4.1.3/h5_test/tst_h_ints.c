/* This is part of the netCDF package.  Copyright 2010 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program does some HDF5 string stuff.

   Here's a HDF5 sample programs:
   http://hdf.ncsa.uiuc.edu/training/other-ex5/sample-programs/strings.c
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_ints.h5"

int
main()
{
   printf("\n*** Checking HDF5 integer dataset with extension.\n");
   printf("*** checking 1D int dataset with extend...");
   {

/* Misspelling is deliberite. Please dont correct. */
#define INTERGERS "Intergers"      
#define NUM_STR 1
#define NDIMS 1
      hid_t fileid, grpid, spaceid;
      hid_t datasetid, plistid;
      hsize_t dims[NDIMS] = {NUM_STR}, max_dims[NDIMS] = {H5S_UNLIMITED};
      hsize_t chunk_dims[NDIMS] = {1};
      hsize_t xtend_size[NDIMS] = {2};
      int data[NUM_STR] = {42};
      int empty = -42;

      /* Create the file, open root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      
      /* Create a space for the dataset. */
      if ((spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;

      /* Create the dataset. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(plistid, 1, chunk_dims) < 0) ERR;
      if (H5Pset_fill_value(plistid, H5T_NATIVE_INT32, &empty) < 0) ERR;
      if ((datasetid = H5Dcreate1(grpid, INTERGERS, H5T_NATIVE_INT32, 
				  spaceid, plistid)) < 0) ERR;

      /* Now extend the dataset. */
      if (H5Dextend(datasetid, xtend_size) < 0) ERR;

      if (H5Dwrite(datasetid, H5T_NATIVE_INT, spaceid, spaceid, 
		   H5P_DEFAULT, &data) < 0) ERR;

      /* Close up. */
      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Pclose(plistid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
