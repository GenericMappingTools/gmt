/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This file creates the same file as file tst_xplatform2_3 (created
   by tst_xplatform2.c).
*/

#include "h5_err_macros.h"
#include <hdf5.h>

#define MY_CHUNK_CACHE_SIZE 32000000
#define STR_LEN 255

/* The file we create. */
#define FILE_NAME "tst_h_atts3.h5"

#define NUM_OBJ 3
#define NUM_VL 1
#define S1_TYPE_NAME "cmp_t"
#define X_NAME "x"
#define Y_NAME "y"
#define S1_NAME "s1"
#define VLEN_TYPE_NAME "Magna_Carta_VLEN"      
#define S3_ATT_NAME "King_John"
#define S3_TYPE_NAME "barons"
#define VL_NAME "No scutage or aid may be levied in our kingdom without its general consent"      
#define ATT_LEN 1

int
main()
{
   printf("\n*** Checking HDF5 attribute functions some more.\n");
#ifdef EXTRA_TESTS
   printf("*** Creating tst_xplatform2_3.nc with HDF only...");
   {
      hid_t fapl_id, fcpl_id;
      size_t chunk_cache_size = MY_CHUNK_CACHE_SIZE;
      size_t chunk_cache_nelems = CHUNK_CACHE_NELEMS;
      float chunk_cache_preemption = CHUNK_CACHE_PREEMPTION;
      hid_t fileid, grpid, attid, spaceid;
      hid_t s1_typeid, vlen_typeid, s3_typeid;
      hid_t file_typeid1[NUM_OBJ], native_typeid1[NUM_OBJ];
      hid_t file_typeid2, native_typeid2;
      hsize_t num_obj;
      H5O_info_t obj_info;
      char obj_name[STR_LEN + 1];
      hsize_t dims[1] = {ATT_LEN}; /* netcdf attributes always 1-D. */
      struct s1
      {
	 float x;
	 double y;
      };
      struct s3
      {
	 hvl_t data[NUM_VL];
      };
      /* cvc stands for "Compound with Vlen of Compound." */
      struct s3 cvc_out[ATT_LEN];
      int i, j, k;

      /* Create some output data: a struct s3 array (length ATT_LEN)
       * which holds an array of vlen (length NUM_VL) of struct s1. */
      for (i = 0; i < ATT_LEN; i++)
	 for (j = 0; j < NUM_VL; j++)
	 {
	    cvc_out[i].data[j].len = i + 1; 
	    if (!(cvc_out[i].data[j].p = calloc(sizeof(struct s1), cvc_out[i].data[j].len))) ERR;
	    for (k = 0; k < cvc_out[i].data[j].len; k++)
	    {
	       ((struct s1 *)cvc_out[i].data[j].p)[k].x = 42.42;
	       ((struct s1 *)cvc_out[i].data[j].p)[k].y = 2.0;
	    }
	 }

      /* Create the HDF5 file, with cache control, creation order, and
       * all the timmings. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_cache(fapl_id, 0, chunk_cache_nelems, chunk_cache_size, 
		       chunk_cache_preemption) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, 
			       H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED | 
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED | 
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Create the compound type for struct s1. */
      if ((s1_typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(s1_typeid, X_NAME, offsetof(struct s1, x), 
		    H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(s1_typeid, Y_NAME, offsetof(struct s1, y), 
		    H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, S1_TYPE_NAME, s1_typeid) < 0) ERR;

      /* Create a vlen type. Its a vlen of stuct s1. */
      if ((vlen_typeid = H5Tvlen_create(s1_typeid)) < 0) ERR;
      if (H5Tcommit(grpid, VLEN_TYPE_NAME, vlen_typeid) < 0) ERR;

      /* Create the struct s3 type, which contains the vlen. */
      if ((s3_typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s3))) < 0) ERR;
      if (H5Tinsert(s3_typeid, VL_NAME, offsetof(struct s3, data), 
		    vlen_typeid) < 0) ERR;
      if (H5Tcommit(grpid, S3_TYPE_NAME, s3_typeid) < 0) ERR;

      /* Create an attribute of this new type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, S3_ATT_NAME, s3_typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, s3_typeid, cvc_out) < 0) ERR;

      /* Close the types. */
      if (H5Tclose(s1_typeid) < 0 ||
	  H5Tclose(vlen_typeid) < 0 ||
	  H5Tclose(s3_typeid) < 0) ERR;

      /* Close the att. */
      if (H5Aclose(attid) < 0) ERR;
      
      /* Close the space. */
      if (H5Sclose(spaceid) < 0) ERR;

      /* Close the group and file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Reopen the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* How many objects in this group? (There should be 3, the
       * types. Atts don't count as objects to HDF5.) */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_OBJ) ERR;

      /* For each object in the group... */
      for (i = 0; i < num_obj; i++)
      {
	 /* Get the name, and make sure this is a type. */
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
				i, &obj_info, H5P_DEFAULT) < 0) ERR;
	 if (H5Lget_name_by_idx(grpid, ".", H5_INDEX_NAME, H5_ITER_INC, i,
				obj_name, STR_LEN + 1, H5P_DEFAULT) < 0) ERR;
	 if (obj_info.type != H5O_TYPE_NAMED_DATATYPE) ERR;

	 /* Get the typeid and native typeid. */
	 if ((file_typeid1[i] = H5Topen2(grpid, obj_name, H5P_DEFAULT)) < 0) ERR;
	 if ((native_typeid1[i] = H5Tget_native_type(file_typeid1[i], 
						     H5T_DIR_DEFAULT)) < 0) ERR;
      }

      /* There is one att: open it by index. */
      if ((attid = H5Aopen_idx(grpid, 0)) < 0) ERR;

      /* Get file and native typeids of the att. */
      if ((file_typeid2 = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid2 = H5Tget_native_type(file_typeid2, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Close the attribute. */
      if (H5Aclose(attid) < 0) ERR;

      /* Close the typeids. */
      for (i = 0; i < NUM_OBJ; i++)
      {
	 if (H5Tclose(file_typeid1[i]) < 0) ERR;
	 if (H5Tclose(native_typeid1[i]) < 0) ERR;
      }
      if (H5Tclose(file_typeid2) < 0) ERR;
      if (H5Tclose(native_typeid2) < 0) ERR;

      /* Close the group and file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Deallocate our vlens. */
      for (i = 0; i < ATT_LEN; i++)
	 for (j = 0; j < NUM_VL; j++)
	    free(cvc_out[i].data[j].p);
   }
   SUMMARIZE_ERR;
   printf("*** Checking vlen of compound file...");
   {
#define NUM_OBJ_1 1
#define ATT_NAME "Poseidon"
      hid_t fapl_id, fcpl_id;
      hid_t fileid, grpid, attid, spaceid;
      hid_t vlen_typeid;
      hid_t file_typeid1[NUM_OBJ_1], native_typeid1[NUM_OBJ_1];
      hid_t file_typeid2, native_typeid2;
      hsize_t num_obj;
      H5O_info_t obj_info;
      char obj_name[STR_LEN + 1];
      hsize_t dims[1] = {ATT_LEN}; /* netcdf attributes always 1-D. */

      /* vc stands for "Vlen of Compound." */
      hvl_t vc_out[ATT_LEN];
      int i, k;

      /* Create some output data: an array of vlen (length ATT_LEN) of
       * int. */
      for (i = 0; i < ATT_LEN; i++)
      {
	 vc_out[i].len = i + 1; 
	 if (!(vc_out[i].p = calloc(sizeof(int), vc_out[i].len))) ERR;
	 for (k = 0; k < vc_out[i].len; k++)
	    ((int *)vc_out[i].p)[k] = 42;
      }
      
      /* Create the HDF5 file with creation order. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED | 
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED | 
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;
      
      /* Open the root group. */
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      
      /* Create a vlen type. Its a vlen of int. */
      if ((vlen_typeid = H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;
      if (H5Tcommit(grpid, VLEN_TYPE_NAME, vlen_typeid) < 0) ERR;
      
      /* Create an attribute of this new type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, vlen_typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, vlen_typeid, vc_out) < 0) ERR;
      
      /* Close the type. */
      if (H5Tclose(vlen_typeid) < 0) ERR;
	  
      /* Close the att. */
      if (H5Aclose(attid) < 0) ERR;
      
      /* Close the space. */
      if (H5Sclose(spaceid) < 0) ERR;

      /* Close the group and file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Reopen the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* How many objects in this group? (There should be 2, the
       * types. Atts don't count as objects to HDF5.) */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_OBJ_1) ERR;

      /* For each object in the group... */
      for (i = 0; i < num_obj; i++)
      {
	 /* Get the name, and make sure this is a type. */
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
				i, &obj_info, H5P_DEFAULT) < 0) ERR;
	 if (H5Lget_name_by_idx(grpid, ".", H5_INDEX_NAME, H5_ITER_INC, i,
				obj_name, STR_LEN + 1, H5P_DEFAULT) < 0) ERR;
	 if (obj_info.type != H5O_TYPE_NAMED_DATATYPE) ERR;

	 /* Get the typeid and native typeid. */
	 if ((file_typeid1[i] = H5Topen2(grpid, obj_name, H5P_DEFAULT)) < 0) ERR;
	 if ((native_typeid1[i] = H5Tget_native_type(file_typeid1[i],
						     H5T_DIR_DEFAULT)) < 0) ERR;
      }

      /* There is one att: open it by index. */
      if ((attid = H5Aopen_idx(grpid, 0)) < 0) ERR;

      /* Get file and native typeids of the att. */
      if ((file_typeid2 = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid2 = H5Tget_native_type(file_typeid2, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Close the attribute. */
      if (H5Aclose(attid) < 0) ERR;

      /* Close the typeids. */
      for (i = 0; i < NUM_OBJ_1; i++)
      {
	 if (H5Tclose(file_typeid1[i]) < 0) ERR;
	 if (H5Tclose(native_typeid1[i]) < 0) ERR;
      }
      if (H5Tclose(file_typeid2) < 0) ERR;
      if (H5Tclose(native_typeid2) < 0) ERR;

      /* Close the group and file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Deallocate our vlens. */
      for (i = 0; i < ATT_LEN; i++)
	 free(vc_out[i].p);
   }
   SUMMARIZE_ERR;
#endif /* EXTRA_TESTS */
   FINAL_RESULTS;
}
