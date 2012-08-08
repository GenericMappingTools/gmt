/* This is part of the netCDF package.  Copyright 2010 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program does some HDF5 string stuff.

   Here's a HDF5 sample programs:
   http://hdf.ncsa.uiuc.edu/training/other-ex5/sample-programs/strings.c
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_strings2.h5"

int
main()
{
   printf("\n*** Checking HDF5 string types even more.\n");
   printf("*** Checking string dataset with unlimited dimension...");
   {
#define VAR_NAME "Mark_Twain"
#define NDIMS 1
#define MY_CHUNK_CACHE_NELEMS 1009
#define MY_CHUNK_CACHE_SIZE 4194304
#define MY_CHUNK_CACHE_PREEMPTION .75

      hid_t fapl_id, fcpl_id, fileid, grpid, spaceid, access_plistid;
      hid_t typeid, datasetid, plistid, create_propid, dimscaleid;
      hid_t file_spaceid, mem_spaceid;
      hsize_t dims[1] = {0}, max_dims[1] = {H5S_UNLIMITED}, chunk_dims[1] = {1};
      hsize_t xtend_size[NDIMS] = {2}, start[NDIMS] = {1}, count[NDIMS] = {1};
/*      void *fillp;*/
      char *data = "A man who carries a cat by the tail learns "
	 "something he can learn in no other way.";
/*      char *empty = "";*/

      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      
      /* Set H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering in the file. */
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create the file, open root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;

      /* Create a dimension scale for unlimited dimension. */
      if ((create_propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(create_propid, NDIMS, chunk_dims) < 0) ERR;
      if ((spaceid = H5Screate_simple(NDIMS, dims, max_dims)) < 0) ERR;
      if (H5Pset_attr_creation_order(create_propid, H5P_CRT_ORDER_TRACKED|
      				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((dimscaleid = H5Dcreate1(grpid, "unlimited_dim", H5T_IEEE_F32BE,
      				   spaceid, create_propid)) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Pclose(create_propid) < 0) ERR;
      if (H5Dclose(dimscaleid) < 0) ERR;
      
      /* Create string type. */
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;
      
      /* Create a space for our dataset. 0 data but unlimited dimension. */
      if ((spaceid = H5Screate_simple(NDIMS, dims, max_dims)) < 0) ERR;

      /* Set up chunking, creation order, and cache. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(plistid, NDIMS, chunk_dims) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
/*      if (H5Pset_fill_value(plistid, typeid, &empty) < 0) ERR;*/
      if ((access_plistid = H5Pcreate(H5P_DATASET_ACCESS)) < 0) ERR;
      if (H5Pset_chunk_cache(access_plistid, MY_CHUNK_CACHE_NELEMS,
			     MY_CHUNK_CACHE_SIZE, MY_CHUNK_CACHE_PREEMPTION) < 0) ERR;

      /* Create the dataset. It has zero records. */
      if ((datasetid = H5Dcreate2(grpid, VAR_NAME, typeid, spaceid,
				  H5P_DEFAULT, plistid, access_plistid)) < 0) ERR;

      /* Now extend the dataset. */
      if (H5Dextend(datasetid, xtend_size) < 0) ERR;

      /* Select space in file to write a record. */
      if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;
      if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET,
			      start, NULL, count, NULL) < 0) ERR;

      /* Select space in memory to read from. */
      if ((mem_spaceid = H5Screate_simple(NDIMS, count, NULL)) < 0) ERR;

      /*if ((xfer_plistid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;*/

      /* Write the data. */
      if (H5Dwrite(datasetid, typeid, mem_spaceid, file_spaceid,
		   H5P_DEFAULT, &data) < 0) ERR;

      /* Close up. */
      if (H5Sclose(file_spaceid) < 0) ERR;
      if (H5Sclose(mem_spaceid) < 0) ERR;
      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Pclose(access_plistid) < 0) ERR;
      if (H5Pclose(plistid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking string dataset with unlimited dimension...");
   {
#define VAR_NAME "Mark_Twain"      
#define NDIMS 1
#define MY_CHUNK_CACHE_NELEMS 1009
#define MY_CHUNK_CACHE_SIZE 4194304
#define MY_CHUNK_CACHE_PREEMPTION .75

      hid_t fapl_id, fcpl_id, fileid, grpid, spaceid, access_plistid;
      hid_t typeid, datasetid, plistid;
      hid_t file_spaceid, mem_spaceid;
      hsize_t dims[1] = {2}, chunk_dims[1] = {1}; 
      hsize_t start[NDIMS] = {1}, count[NDIMS] = {1};
/*      void *fillp;*/
      char *data = "A man who carries a cat by the tail learns "
	 "something he can learn in no other way.";
/* Man - a creature made at the end of the week's work when God was tired. 
There are basically two types of people. People who accomplish things, and people who claim to have accomplished things. The first group is less crowded. 
To be good is noble; but to show others how to be good is nobler and no trouble. */
/*      char *empty = "";*/

      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      
      /* Set H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering in the file. */
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create the file, open root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;

      /* Create string type. */
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;
      
      /* Create a space for our dataset. */
      if ((spaceid = H5Screate_simple(NDIMS, dims, dims)) < 0) ERR;

      /* Set up chunking, creation order, and cache. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(plistid, NDIMS, chunk_dims) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
/*      if (H5Pset_fill_value(plistid, typeid, &empty) < 0) ERR;*/
      if ((access_plistid = H5Pcreate(H5P_DATASET_ACCESS)) < 0) ERR;
      if (H5Pset_chunk_cache(access_plistid, MY_CHUNK_CACHE_NELEMS,
			     MY_CHUNK_CACHE_SIZE, MY_CHUNK_CACHE_PREEMPTION) < 0) ERR;

      /* Create the dataset. It has zero records. */
      if ((datasetid = H5Dcreate2(grpid, VAR_NAME, typeid, spaceid, 
				  H5P_DEFAULT, plistid, access_plistid)) < 0) ERR;

      /* Now extend the dataset. */
      /*if (H5Dextend(datasetid, xtend_size) < 0) ERR;*/

      /* Select space in file to write a record. */
      if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;
      if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET, 
			      start, NULL, count, NULL) < 0) ERR;

      /* Select space in memory to read from. */
      if ((mem_spaceid = H5Screate_simple(NDIMS, count, NULL)) < 0) 

	 /*if ((xfer_plistid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;*/

      /* Write the data. */
      if (H5Dwrite(datasetid, typeid, mem_spaceid, file_spaceid, 
		   H5P_DEFAULT, &data) < 0)

      /* Close up. */
      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Pclose(access_plistid) < 0) ERR;
      if (H5Pclose(plistid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
