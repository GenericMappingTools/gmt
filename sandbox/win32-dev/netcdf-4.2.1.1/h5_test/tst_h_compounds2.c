/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This program deals with HDF5 compound types.

   $Id$
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_compounds2.h5"
#define REF_FILE_IN "ref_tst_h_compounds2.h5"
#define STR_LEN 255

int
main()
{
   printf("\n*** Checking HDF5 compound types some more.\n");
   printf("*** Checking HDF5 compound attribute which contains a simple compound type...");
   {
#define DIM_CMP_LEN 1
#define ATT_NAME1 "The_Nutmeg_of_Consolation"
#define NUM_TYPES 2
#define INNER_TYPE_NAME "s1"
#define OUTER_TYPE_NAME "d"

      /* This struct will be embeddeded in another. */
      struct s1
      {
            float x;
            double y;
      };
      struct s2
      {
            struct s1 s1;
      };
      struct s2 data_out[DIM_CMP_LEN], data_in[DIM_CMP_LEN];
      hid_t fileid, grpid, typeid_inner, typeid_outer, spaceid, attid;
      hid_t obj_hdf_typeid[NUM_TYPES], obj_native_typeid[NUM_TYPES];
      hid_t inner_type_native_typeid;
      hid_t att_typeid, att_native_typeid;
      hsize_t dims[1];
      hsize_t num_obj, i_obj;
      char obj_name[STR_LEN + 1];
      H5O_info_t obj_info;
      hid_t fapl_id, fcpl_id;
      htri_t equal;
      char file_in[STR_LEN * 2];
      char *dummy;
      int i;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s2), DIM_CMP_LEN))) ERR;
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s2) * DIM_CMP_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM_CMP_LEN; i++)
      {
         data_out[i].s1.x = 1.0;
         data_out[i].s1.y = -2.0;
      }

      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create file and get root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create the inner compound type. */
      if ((typeid_inner = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
      if (H5Tinsert(typeid_inner, "x", HOFFSET(struct s1, x), H5T_NATIVE_FLOAT) < 0) ERR;
      if (H5Tinsert(typeid_inner, "y", HOFFSET(struct s1, y), H5T_NATIVE_DOUBLE) < 0) ERR;
      if (H5Tcommit(grpid, INNER_TYPE_NAME, typeid_inner) < 0) ERR;

      /* Create a compound type containing a compound type. */
      if ((typeid_outer = H5Tcreate(H5T_COMPOUND, sizeof(struct s2))) < 0) ERR;
      if (H5Tinsert(typeid_outer, INNER_TYPE_NAME, HOFFSET(struct s2, s1), typeid_inner) < 0) ERR;
      if (H5Tcommit(grpid, OUTER_TYPE_NAME, typeid_outer) < 0) ERR;

      /* Create a space. */
      dims[0] = DIM_CMP_LEN;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create an attribute of this compound type. */
      if ((attid = H5Acreate2(grpid, ATT_NAME1, typeid_outer, spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      /* Write some data to the attribute. */
      if (H5Awrite(attid, typeid_outer, data_out) < 0) ERR;
      
      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(typeid_outer) < 0 ||
          H5Tclose(typeid_inner) < 0 ||
          H5Sclose(spaceid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the file and get the type of the attribute. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", ATT_NAME1, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((att_typeid = H5Aget_type(attid)) < 0) ERR;
      if ((att_native_typeid = H5Tget_native_type(att_typeid, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Find the HDF ID of the inner type. */
      if ((inner_type_native_typeid = H5Tget_member_type(att_native_typeid, 0)) < 0) ERR;

      /* Check the data. */
      if (H5Aread(attid, att_native_typeid, data_in) < 0) ERR;
      for (i = 0; i < DIM_CMP_LEN; i++)
         if (data_out[i].s1.x != data_in[i].s1.x ||
             data_out[i].s1.y != data_in[i].s1.y) ERR;

      /* Now iterate through the objects in the file, finding the two
       * defined compound types. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i_obj = 0; i_obj < num_obj; i_obj++)
      {
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 
				i_obj, &obj_info, H5P_DEFAULT) < 0) ERR;
	 if (H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 
				i_obj, obj_name, STR_LEN + 1, H5P_DEFAULT) < 0) ERR;

	 /* Deal with groups and datasets. */
	 if (obj_info.type == H5O_TYPE_NAMED_DATATYPE)
	 {
	    /* Learn about the user-defined type. */
	    if ((obj_hdf_typeid[i_obj] = H5Topen2(grpid, obj_name, 
						  H5P_DEFAULT)) < 0) ERR;
	    if ((obj_native_typeid[i_obj] = H5Tget_native_type(obj_hdf_typeid[i_obj], 
							       H5T_DIR_DEFAULT)) < 0) ERR;

	    /* If this is the inner type, the obj_native_typeid
	     * should be equal to the inner_type_native_typeid. */
	    if (!strcmp(obj_name, INNER_TYPE_NAME))
	    {
	       if ((equal = H5Tequal(obj_native_typeid[i_obj], 
				     inner_type_native_typeid)) < 0) ERR;
	       if (!equal) ERR;
	    }
	 }
      }

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(att_typeid) < 0 ||
          H5Tclose(att_native_typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

      /* Now open the reference file, created on buddy, and check it
       * all again. This is the cross-platform part! */
      if (getenv("srcdir"))
      {
         strcpy(file_in, getenv("srcdir"));
         strcat(file_in, "/");
         strcat(file_in, REF_FILE_IN);
      } 
      else
         strcpy(file_in, REF_FILE_IN);

      if ((fileid = H5Fopen(file_in, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_by_name(grpid, ".", ATT_NAME1, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if ((att_typeid = H5Aget_type(attid)) < 0) ERR;
      if ((att_native_typeid = H5Tget_native_type(att_typeid, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Check the data. */
      if (H5Aread(attid, att_native_typeid, data_in) < 0) ERR;
      for (i = 0; i < DIM_CMP_LEN; i++)
         if (data_out[i].s1.x != data_in[i].s1.x ||
             data_out[i].s1.y != data_in[i].s1.y) ERR;

      /* Find the HDF ID of the inner type. */
      if ((inner_type_native_typeid = H5Tget_member_type(att_native_typeid, 0)) < 0) ERR;

      /* Now iterate through the objects in the file, finding the two
       * defined compound types. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i_obj = 0; i_obj < num_obj; i_obj++)
      {
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i_obj, &obj_info, 
				H5P_DEFAULT) < 0) ERR;
	 if (H5Lget_name_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i_obj, obj_name, 
				STR_LEN + 1, H5P_DEFAULT) < 0) ERR;

	 /* Deal with groups and datasets. */
	 if (obj_info.type == H5O_TYPE_NAMED_DATATYPE)
	 {
	    /* Learn about the user-defined type. */
	    if ((obj_hdf_typeid[i_obj] = H5Topen2(grpid, obj_name, H5P_DEFAULT)) < 0) ERR;
	    if ((obj_native_typeid[i_obj] = H5Tget_native_type(obj_hdf_typeid[i_obj], 
							       H5T_DIR_DEFAULT)) < 0) ERR;

	    /* If this is the inner type, the obj_native_typeid
	     * should be equal to the inner_type_native_typeid. */
	    if (!strcmp(obj_name, INNER_TYPE_NAME))
	    {
	       if ((equal = H5Tequal(obj_native_typeid[i_obj], inner_type_native_typeid)) < 0) ERR;
	       if (!equal) ERR;
	    }
	 }
      }

      /* Release all resources. */
      if (H5Aclose(attid) < 0 ||
          H5Tclose(att_typeid) < 0 ||
          H5Tclose(att_native_typeid) < 0 ||
          H5Gclose(grpid) < 0 ||
          H5Fclose(fileid) < 0) ERR;

   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
