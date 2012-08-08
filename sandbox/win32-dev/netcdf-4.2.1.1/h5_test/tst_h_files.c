/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.
*/

#include "h5_err_macros.h"
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_files.h5"
#define GRP_NAME "Dectectives"
#define STR_LEN 255

int
main()
{
   printf("\n*** Checking HDF5 file functions.\n");
   printf("*** Checking HDF5 file creates and opens...");
#define OPAQUE_SIZE 20
#define OPAQUE_NAME "type"
#define ATT_NAME "att_name"
#define DIM_LEN 3
   {
      hid_t fileid, access_plist, typeid, spaceid, attid, fapl_id, grpid;
      hsize_t dims[1]; /* netcdf attributes always 1-D. */
      unsigned char data[DIM_LEN][OPAQUE_SIZE];
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      H5T_class_t class;
      size_t type_size;
      int j, k;
      hid_t tmp1;

      H5open();

      /* Initialize some data. */
      for (j = 0; j < DIM_LEN; j++)
	 for (k = 0; k < OPAQUE_SIZE; k++)
	    data[j][k] = 42;

      /* Set the access list so that closes will fail if something is
       * still open in the file. */
      tmp1 = H5P_FILE_ACCESS;
      if ((access_plist = H5Pcreate(tmp1)) < 0) ERR;
      if (H5Pset_fclose_degree(access_plist, H5F_CLOSE_SEMI)) ERR;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      access_plist)) < 0) ERR;
      /* Add an opaque type. */
      if ((typeid = H5Tcreate(H5T_OPAQUE, OPAQUE_SIZE)) < 0) ERR;
      if (H5Tcommit(fileid, OPAQUE_NAME, typeid) < 0) ERR;
      
      /* Add attribute of this type. */
      dims[0] = 3;
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(fileid, ATT_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;

      if (H5Aclose(attid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      if (H5Pclose(access_plist) < 0) ERR;

      if (H5Eset_auto(NULL, NULL) < 0) ERR;

      /* Reopen the file. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      /*if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI)) ERR;*/
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i = 0; i < num_obj; i++)
      {
	 if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(grpid, i, obj_name, 
				   STR_LEN) < 0) ERR;
	 if (obj_class != H5G_TYPE) ERR;
	 if ((typeid = H5Topen(grpid, obj_name)) < 0) ERR;
	 if ((class = H5Tget_class(typeid)) < 0) ERR;
	 if (class != H5T_OPAQUE) ERR;
	 if (!(H5Tget_size(typeid))) ERR;
      }

      /* Close everything. */
      if (H5Pclose(fapl_id)) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      /*if (H5Tclose(typeid) < 0) ERR;*/
      if (H5Fclose(fileid) < 0) ERR;

      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI)) ERR;
      if ((fileid = H5Fopen(FILE_NAME, 0, fapl_id)) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

   }
   SUMMARIZE_ERR;

   printf("*** Checking HDF5 file creates and opens some more...");
   {
      int objs;
      hid_t fileid, fileid2, grpid, access_plist;

      /* Set the access list so that closes will fail if something is
       * still open in the file. */
      if ((access_plist = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(access_plist, H5F_CLOSE_SEMI)) ERR;

      /* Create file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      access_plist)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;

      /* How many open objects are there? */
      if ((objs = H5Fget_obj_count(fileid, H5F_OBJ_ALL)) < 0) ERR;
      if (objs != 2) ERR;
      if ((objs = H5Fget_obj_count(fileid, H5F_OBJ_GROUP)) < 0) ERR;
      if (objs != 1) ERR;

      /* Turn off HDF5 error messages. */
      if (H5Eset_auto(NULL, NULL) < 0) ERR;

      /* This H5Fclose should fail, because I didn't close the group. */
      if (H5Fclose(fileid) >= 0) ERR;

      /* Now close the group first, and then the file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Now create the file again, to make sure that it really is not
       * just mearly dead, but really most sincerely dead. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      access_plist)) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* Confirm that the same file can be opened twice at the same time,
       * for read only access. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((fileid2 = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      if (H5Fclose(fileid2) < 0) ERR;

      /* Once open for read only access, the file can't be opened again
       * for write access. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if (H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT) >= 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* But you can open the file for read/write access, and then open
       * it again for read only access. */
      if ((fileid2 = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      if (H5Fclose(fileid2) < 0) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** Creating file...");
   {
#define VAR_NAME "HALs_memory"
#define NDIMS 1
#define DIM1_LEN 40000
#define SC 10000 /* slice count. */
#define MILLION 1000000

      hid_t fileid, write_spaceid, datasetid, mem_spaceid;
      hsize_t start[NDIMS], count[NDIMS];
      hsize_t dims[1];
      int *data;
      int num_steps;
      int i, s;

      /* We will write the same slice of random data over and over to
       * fill the file. */
      if (!(data = malloc(SC * sizeof(int))))
	 ERR_RET;
      for (i = 0; i < SC; i++)
	 data[i] = rand();
      
      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;

      /* Create a space to deal with one slice in memory. */
      dims[0] = SC;
      if ((mem_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

      /* Create a space to write all slices. */
      dims[0] = DIM1_LEN;
      if ((write_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

      /* Create dataset. */
      if ((datasetid = H5Dcreate1(fileid, VAR_NAME, H5T_NATIVE_INT, 
				  write_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Write the data in num_step steps. */
      num_steps = DIM1_LEN/SC;
      count[0] = SC;
      for (s = 0; s < num_steps; s++)
      {
	 /* Select hyperslab for write of one slice. */
	 start[0] = s * SC;
	 if (H5Sselect_hyperslab(write_spaceid, H5S_SELECT_SET, 
				 start, NULL, count, NULL) < 0) ERR;

	 if (H5Dwrite(datasetid, H5T_NATIVE_INT, mem_spaceid, write_spaceid, 
		      H5P_DEFAULT, data) < 0) ERR;
      }
      
      /* Close. */
      free(data);
      if (H5Dclose(datasetid) < 0 ||
	  H5Sclose(write_spaceid) < 0 ||
	  H5Sclose(mem_spaceid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }
   SUMMARIZE_ERR;
#ifdef LARGE_FILE_TESTS

#define NDIMS2 2
#define DIM1 2048
#define DIM2 2097153		/* DIM1*DIM2*sizeof(char)   > 2**32 */
#define DIM_WITHOUT_VARIABLE "This is a netCDF dimension but not a netCDF variable."
#define VAR_NAME2 "var"
#define MAX_DIMS 255

   printf("*** large file test for HDF5...");
   {
      hid_t fapl_id, fcpl_id, fileid, grpid, spaceid, datasetid;
      hid_t dim1_dimscaleid, dim2_dimscaleid, plistid, datasetid2, file_spaceid;
      hid_t mem_spaceid, xfer_plistid, native_typeid;
      hsize_t *chunksize, dims[1], maxdims[1], *dimsize, *maxdimsize;
      hsize_t fdims[MAX_DIMS], fmaxdims[MAX_DIMS];
      hsize_t start[MAX_DIMS],  count[MAX_DIMS];
      char file_name[STR_LEN + 1];
      char dimscale_wo_var[STR_LEN];
      void *bufr;
      void *fillp;

      sprintf(file_name, "%s/%s", TEMP_LARGE, FILE_NAME);
      
      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      
      /* Set latest_format in access propertly list. This ensures that
       * the latest, greatest, HDF5 versions are used in the file. */ 
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;

      /* Set H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering in the file. */
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create the file. */
      if ((fileid = H5Fcreate(file_name, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Set up the dataset creation property list for the two dimensions. */
/*      if (H5Pset_chunk(plistid, 1, chunksize) < 0) ERR;*/

      /* Create the dim1 dimscale. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      dims[0] = DIM1;
      maxdims[0] = DIM1;
      if ((spaceid = H5Screate_simple(1, dims, maxdims)) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((dim1_dimscaleid = H5Dcreate(grpid, "dim1", H5T_IEEE_F32BE,
				      spaceid, plistid)) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Pclose(plistid) < 0) ERR;
      sprintf(dimscale_wo_var, "%s%10d", DIM_WITHOUT_VARIABLE, DIM1);
      if (H5DSset_scale(dim1_dimscaleid, dimscale_wo_var) < 0) ERR;

      /* Create the dim2 dimscale. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      dims[0] = DIM2;
      maxdims[0] = DIM2;
      if ((spaceid = H5Screate_simple(1, dims, maxdims)) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((dim2_dimscaleid = H5Dcreate(grpid, "dim2", H5T_IEEE_F32BE,
				      spaceid, plistid)) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Pclose(plistid) < 0) ERR;
      sprintf(dimscale_wo_var, "%s%10d", DIM_WITHOUT_VARIABLE, DIM2);
      if (H5DSset_scale(dim2_dimscaleid, dimscale_wo_var) < 0) ERR;

      /* Now create the 2D dataset. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;      
      if (!(fillp = malloc(1))) ERR;
#define FILL_BYTE 255
      *(signed char *)fillp = FILL_BYTE;
      if (H5Pset_fill_value(plistid, H5T_NATIVE_SCHAR, fillp) < 0) ERR;
      if (!(chunksize = malloc(NDIMS2 * sizeof(hsize_t)))) ERR;
      chunksize[0] = 1024;
      chunksize[1] = 1048576;
      if (H5Pset_chunk(plistid, NDIMS2, chunksize) < 0) ERR;
      if (!(dimsize = malloc(NDIMS2 * sizeof(hsize_t)))) ERR;
      if (!(maxdimsize = malloc(NDIMS2 * sizeof(hsize_t)))) ERR;
      dimsize[0] = 2048;
      dimsize[1] = 2097153;
      maxdimsize[0] = 2048;
      maxdimsize[1] = 2097153;
      if ((spaceid = H5Screate_simple(NDIMS2, dimsize, maxdimsize)) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, VAR_NAME2, H5T_NATIVE_SCHAR, spaceid, plistid)) < 0)
      
      free(fillp);
      free(chunksize);
      free(dimsize);
      free(maxdimsize);
      if (H5Pclose(plistid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;

      if (H5DSattach_scale(datasetid, dim1_dimscaleid, 0) < 0) ERR;
      if (H5DSattach_scale(datasetid, dim2_dimscaleid, 1) < 0) ERR;
      H5Fflush(fileid, H5F_SCOPE_GLOBAL);

      /* Read a slice of data. */
      if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;
      if (H5Sget_simple_extent_dims(file_spaceid, fdims, fmaxdims) < 0) ERR;
      if (H5Sget_simple_extent_type(file_spaceid) == H5S_SCALAR) ERR;
      start[0] = 0;
      start[1] = 0;
      count[0] = 1;
      count[1] = 2097153;
      if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET, start, NULL, count, NULL) < 0) ERR;
      if ((mem_spaceid = H5Screate_simple(NDIMS2, count, NULL)) < 0) ERR;
      if ((xfer_plistid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;
      if ((native_typeid = H5Tget_native_type(H5T_NATIVE_SCHAR, H5T_DIR_DEFAULT)) < 0) ERR;
      if (!(bufr = malloc(DIM2))) ERR;
      if (H5Dwrite(datasetid, native_typeid, mem_spaceid, file_spaceid, xfer_plistid, bufr) < 0) ERR;
      free(bufr);
      if (H5Tclose(native_typeid) < 0) ERR;

      /* Close down the show. */
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;
      if (H5Dclose(dim1_dimscaleid) < 0) ERR;
      if (H5Dclose(dim2_dimscaleid) < 0) ERR;
      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

/*       /\* Reopen the file and check it. *\/ */
/*       if ((fileid = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR; */
/*       if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR; */
/*       if (num_obj) ERR; */
/*       if (H5Fclose(fileid) < 0) ERR; */

      /* Delete the huge data file we created. */
      (void) remove(file_name); 
   }
   SUMMARIZE_ERR;
#endif /* LARGE_FILE_TESTS */

   FINAL_RESULTS;
}
