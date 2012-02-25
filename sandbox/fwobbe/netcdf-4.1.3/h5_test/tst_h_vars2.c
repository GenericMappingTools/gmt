/* This is part of the netCDF package. Copyright 2007 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 dataset code, even more. These are not intended to be
   exhaustive tests, but they use HDF5 the same way that netCDF-4
   does, so if these tests don't work, than netCDF-4 won't work
   either.
*/

#include <err_macros.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_vars2.h5"
#define STR_LEN 255

int
main()
{
   printf("\n*** Checking HDF5 variable functions some more.\n");
   /* If HDF5 has working ordering of variables, then the following
    * test will work.*/
   printf("*** Checking HDF5 variable ordering...");

#define NUM_ELEMENTS 6
#define MAX_SYMBOL_LEN 2
#define ELEMENTS_NAME "Elements"
   {
      hid_t did[NUM_ELEMENTS], fapl_id, fcpl_id, gcpl_id;
      hsize_t num_obj;
      hid_t fileid, grpid, spaceid;
      int i;
      H5O_info_t obj_info;
      char names[NUM_ELEMENTS][MAX_SYMBOL_LEN + 1] = {"H", "He", "Li", "Be", "B", "C"};
      char name[MAX_SYMBOL_LEN + 1];
      size_t size;

      /* Create file, setting latest_format in access propertly list
       * and H5P_CRT_ORDER_TRACKED in the creation property list. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Create group, with link_creation_order set in the group
       * creation property list. */
      if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((grpid = H5Gcreate_anon(fileid, gcpl_id, H5P_DEFAULT)) < 0) ERR;
      if ((H5Olink(grpid, fileid, ELEMENTS_NAME, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Create a scalar space. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;

      /* Create the variables, one per element. */
      for (i = 0; i < NUM_ELEMENTS; i++)
      {
	 if ((did[i] = H5Dcreate(grpid, names[i], H5T_NATIVE_INT,
				 spaceid, H5P_DEFAULT)) < 0) ERR;
	 if (H5Dclose(did[i]) < 0) ERR;
      }

      if (H5Pclose(fapl_id) < 0 ||
	  H5Pclose(gcpl_id) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check the order. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, ELEMENTS_NAME)) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_ELEMENTS) ERR;
      for (i = 0; i < num_obj; i++)
      {
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
				i, &obj_info, H5P_DEFAULT) < 0) ERR;
	 if (obj_info.type != H5O_TYPE_DATASET) ERR;
	 if ((size = H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
					NULL, 0, H5P_DEFAULT)) < 0) ERR;
	 H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
				name, size+1, H5P_DEFAULT);
	 if (strcmp(name, names[i])) ERR;
      }
      if (H5Pclose(fapl_id) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 variable ordering in root group...");

#define NUM_DIMSCALES 2
#define MAX_SYMBOL_LEN 2
#define DIM1_LEN 3
#define DIMSCALE_NAME "Joe"
#define NAME_ATTRIBUTE "short"
   {
      hid_t fapl_id, fcpl_id;
      hid_t fileid, grpid;
      hsize_t num_obj;
      int i;
      H5O_info_t obj_info;
      char names[NUM_DIMSCALES][MAX_SYMBOL_LEN + 1] = {"b", "a"};
      char name[MAX_SYMBOL_LEN + 1];
      hid_t dimscaleid;
      hid_t dimscale_spaceid;
      hsize_t dimscale_dims[1] = {DIM1_LEN};
      size_t size;

      /* Create file, setting latest_format in access propertly list
       * and H5P_CRT_ORDER_TRACKED in the creation property list. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      if ((dimscale_spaceid = H5Screate_simple(1, dimscale_dims,
					       dimscale_dims)) < 0) ERR;
      /* Create the variables, one per element. */
      for (i = 0; i < NUM_DIMSCALES; i++)
      {
	 /* Create our dimension scale. Use the built-in NAME attribute
	  * on the dimscale. */
	 if ((dimscaleid = H5Dcreate(fileid, names[i], H5T_NATIVE_INT,
				     dimscale_spaceid, H5P_DEFAULT)) < 0) ERR;
	 if (H5DSset_scale(dimscaleid, NAME_ATTRIBUTE) < 0) ERR;
	 
	 if (H5Dclose(dimscaleid) < 0) ERR;
      }

      if (H5Pclose(fapl_id) < 0 ||
	  H5Pclose(fcpl_id) < 0 ||
	  H5Sclose(dimscale_spaceid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check the order. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_DIMSCALES) ERR;
      for (i = 0; i < num_obj; i++)
      {
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
				i, &obj_info, H5P_DEFAULT) < 0) ERR;
	 if (obj_info.type != H5O_TYPE_DATASET) ERR;
	 size = H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
                                	 NULL, 0, H5P_DEFAULT);
	 if (H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i,
				name, size+1, H5P_DEFAULT) < 0) ERR;
	 if (strcmp(name, names[i])) ERR;
      }
      if (H5Pclose(fapl_id) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 variable ordering flags with redef-type situations...");

#define NUM_ELEMENTS 6
#define MAX_NAME_LEN 50
#define ELEMENTS_NAME "Elements"
#define VAR_NAME "Sears_Zemansky_and_Young"
   {
      hid_t did, fapl_id, fcpl_id, gcpl_id, attid;
      hsize_t num_obj;
      hid_t fileid, grpid, spaceid;
      float val = 3.1495;
      H5O_info_t obj_info;
      char name[MAX_NAME_LEN + 1];
      size_t size;

      /* Create file, setting latest_format in access propertly list
       * and H5P_CRT_ORDER_TRACKED in the creation property list. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Create group, with link_creation_order set in the group
       * creation property list. */
      if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((grpid = H5Gcreate_anon(fileid, gcpl_id, H5P_DEFAULT)) < 0) ERR;
      if ((H5Olink(grpid, fileid, ELEMENTS_NAME, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Create a scalar space. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;

      /* Create a scalar variable. */
      if ((did = H5Dcreate(grpid, VAR_NAME, H5T_NATIVE_INT,
			   spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5Dclose(did) < 0) ERR;

      /* Flush the HDF5 buffers. */
      H5Fflush(fileid, H5F_SCOPE_GLOBAL);

      /* Delete the variable. Just to be mean. */
      if (H5Gunlink(grpid, VAR_NAME) < 0) ERR;

      /* Re-reate the scalar variable. */
      if ((did = H5Dcreate(grpid, VAR_NAME, H5T_NATIVE_INT,
			   spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Add an attribute. */
      if ((attid = H5Acreate(did, "Some_Attribute", H5T_NATIVE_FLOAT, spaceid,
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, H5T_NATIVE_FLOAT, &val) < 0) ERR;

      if (H5Aclose(attid) < 0) ERR;
      if (H5Dclose(did) < 0) ERR;

      if (H5Pclose(fapl_id) < 0 ||
	  H5Pclose(gcpl_id) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check the order. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, ELEMENTS_NAME)) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != 1) ERR;
      if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
			     0, &obj_info, H5P_DEFAULT) < 0) ERR;
      if (obj_info.type != H5O_TYPE_DATASET) ERR;
      if ((size = H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0,
				     NULL, 0, H5P_DEFAULT)) < 0) ERR;
      H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0,
                         name, size+1, H5P_DEFAULT);
      if (strcmp(name, VAR_NAME)) ERR;
      if (H5Pclose(fapl_id) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 variable compession and filters...");
   {
#define NUM_ELEMENTS 6
#define MAX_NAME_LEN 50
#define ELEMENTS_NAME "Elements"
#define VAR_NAME "Sears_Zemansky_and_Young"
#define NDIMS 2
#define NX 60
#define NY 120
#define DEFLATE_LEVEL 3
#define SIMPLE_VAR_NAME "data"

      hid_t fapl_id, fcpl_id;
      hid_t datasetid;
      hid_t fileid, grpid, spaceid, plistid;
      int data_in[NX][NY], data_out[NX][NY];
      hsize_t fdims[NDIMS], fmaxdims[NDIMS];
      hsize_t chunksize[NDIMS], dimsize[NDIMS], maxdimsize[NDIMS];
      int x, y;

      /* Create some data to write. */
      for (x = 0; x < NX; x++)
	 for (y = 0; y < NY; y++)
	    data_out[x][y] = x * NY + y;

      /* Create file, setting latest_format in access propertly list
       * and H5P_CRT_ORDER_TRACKED in the creation property list. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      dimsize[0] = maxdimsize[0] = NX;
      dimsize[1] = maxdimsize[1] = NY;
      if ((spaceid = H5Screate_simple(NDIMS, dimsize, maxdimsize)) < 0) ERR;

      /* Create property lust. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;

      /* Set up chunksizes. */
      chunksize[0] = NX;
      chunksize[1] = NY;
      if (H5Pset_chunk(plistid, NDIMS, chunksize) < 0)ERR;

      /* Set up compression. */
      if (H5Pset_deflate(plistid, DEFLATE_LEVEL) < 0) ERR;

      /* Create the variable. */
      if ((datasetid = H5Dcreate(grpid, SIMPLE_VAR_NAME, H5T_NATIVE_INT,
				 spaceid, plistid)) < 0) ERR;

      /* Write the data. */
      if (H5Dwrite(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
		   H5P_DEFAULT, data_out) < 0) ERR;

      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Pclose(fapl_id) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Now reopen the file and check the order. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      if ((datasetid = H5Dopen1(grpid, SIMPLE_VAR_NAME)) < 0) ERR;
      if ((spaceid = H5Dget_space(datasetid)) < 0)
      if (H5Sget_simple_extent_dims(spaceid, fdims, fmaxdims) > 0) ERR;
      if (H5Dread(datasetid, H5T_NATIVE_INT, H5S_ALL,
		  spaceid, H5P_DEFAULT, data_in) < 0) ERR;

      /* Check the data. */
      for (x = 0; x < NX; x++)
	 for (y = 0; y < NY; y++)
	    if (data_in[x][y] != data_out[x][y]) ERR_RET;

      if (H5Pclose(fapl_id) < 0 ||
	  H5Dclose(datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }
   SUMMARIZE_ERR;
/* I cant get this to work, I don't think it's allowed in HDF5. - Ed 7/12/7 */
/*    printf("*** Checking HDF5 scalar variable compession..."); */

/* #define MAX_NAME_LEN 50 */
/* #define DEFLATE_LEVEL 3 */
/* #define SIMPLE_VAR_NAME1 "punches" */
/*    { */
/*       hid_t fapl_id, fcpl_id; */
/*       hid_t datasetid; */
/*       hid_t fileid, grpid, spaceid, plistid; */
/*       int data_in, data_out = 42; */
/*       hsize_t chunksize = 1; */

/*       /\* Create file, setting latest_format in access propertly list */
/*        * and H5P_CRT_ORDER_TRACKED in the creation property list. *\/ */
/*       if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR; */
/*      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;*/
/*       if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR; */
/*       if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR; */
/*       if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR; */

/*       if ((grpid = H5Gopen(fileid, "/")) < 0) ERR; */

/*       if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR; */

/*       /\* Create property lust. *\/ */
/*       if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR; */

/*       if (H5Pset_chunk(plistid, 1, &chunksize) < 0)ERR; */

/*       /\* Set up compression. *\/ */
/*       if (H5Pset_deflate(plistid, DEFLATE_LEVEL) < 0) ERR; */

/*       /\* Create the variable. *\/ */
/*       if ((datasetid = H5Dcreate(grpid, SIMPLE_VAR_NAME1, H5T_NATIVE_INT, */
/* 				 spaceid, plistid)) < 0) ERR; */

/*       /\* Write the data. *\/ */
/*       if (H5Dwrite(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,  */
/* 		   H5P_DEFAULT, &data_out) < 0) ERR; */

/*       if (H5Dclose(datasetid) < 0) ERR; */
/*       if (H5Pclose(fapl_id) < 0 || */
/* 	  H5Sclose(spaceid) < 0 || */
/* 	  H5Gclose(grpid) < 0 || */
/* 	  H5Fclose(fileid) < 0) */
/* 	 ERR; */

/*       /\* Now reopen the file and check. *\/ */
/*       if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR; */
/*      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;*/
/*       if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR; */
/*       if ((grpid = H5Gopen(fileid, "/")) < 0) ERR; */

/*       if ((datasetid = H5Dopen1(grpid, SIMPLE_VAR_NAME1)) < 0) ERR; */
/*       if ((spaceid = H5Dget_space(datasetid)) < 0)  */
/*       if (H5Dread(datasetid, H5T_NATIVE_INT, H5S_ALL,  */
/* 		  spaceid, H5P_DEFAULT, &data_in) < 0) ERR; */

/*       /\* Check the data. *\/ */
/*       if (data_in != data_out) ERR; */

/*       if (H5Pclose(fapl_id) < 0 || */
/* 	  H5Dclose(datasetid) < 0 || */
/* 	  H5Gclose(grpid) < 0 || */
/* 	  H5Fclose(fileid) < 0) */
/* 	 ERR; */
/*    } */
/*    SUMMARIZE_ERR; */

   printf("*** Checking fill value of compound type...");

   {
#define VAR_NAME2 "obs"
#define REF_FILE "ref_tst_compounds.nc"
#define ATT_NAME "_FillValue"

      typedef struct obs_t {
	    char day ;
	    short elev;
	    int count;
	    float relhum;
	    double time;
      } obs_t ;

      obs_t f1, f2;
      obs_t m = {-99, -99, -99, -99, -99};
      hid_t fileid, grpid, attid, typeid, datasetid, native_typeid, propid;
      H5D_fill_value_t fill_status;
      char file_in[STR_LEN * 2];
      size_t type_size;

      if (getenv("srcdir"))
      {
	 strcpy(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
	 strcat(file_in, REF_FILE);
      }
      else
	 strcpy(file_in, REF_FILE);

      /* Open file and read fill value of the variable. */
      if ((fileid = H5Fopen(file_in, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, VAR_NAME2)) < 0) ERR;
      if ((propid = H5Dget_create_plist(datasetid)) < 0) ERR;
      if (H5Pfill_value_defined(propid, &fill_status) < 0) ERR;
      if (fill_status != H5D_FILL_VALUE_USER_DEFINED) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
      if ((native_typeid = H5Tget_native_type(typeid, H5T_DIR_DEFAULT)) < 0) ERR;
      if (!(type_size = H5Tget_size(native_typeid))) ERR;
      if (type_size != sizeof(obs_t)) ERR;
      if (H5Pget_fill_value(propid, native_typeid, &f1) < 0) ERR;
      if (f1.day != m.day || f1.elev != m.elev || f1.count != m.count ||
	  f1.relhum != m.relhum || f1.time != m.time) ERR;

      /* It's also in an attribute, netCDF style. */
      if ((attid = H5Aopen_name(datasetid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid = H5Tget_native_type(typeid, H5T_DIR_DEFAULT)) < 0) ERR;
      if (H5Aread(attid, native_typeid, &f2) < 0) ERR;
      if (f2.day != m.day || f2.elev != m.elev || f2.count != m.count ||
	  f2.relhum != m.relhum || f2.time != m.time) ERR;
      
      if (H5Dclose(datasetid) < 0 ||
	  H5Aclose(attid) < 0 ||
	  H5Tclose(native_typeid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Pclose(propid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 lots of datasets...");
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
