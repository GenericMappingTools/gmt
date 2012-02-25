/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This file deals with HDF5 attributes, but more so.

   $Id$
*/
#include <nc_tests.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_atts2.h5"
#define REF_FILE_NAME "tst_xplatform2_3.nc"
#define MAX_LEN 80
#define NUM_ATTS 1
#define ATT_NAME "King_John"
#define NUM_OBJ 3

int
main()
{
   printf("\n*** Checking HDF5 attribute functions some more.\n");
#ifdef EXTRA_TESTS
   printf("*** Opening tst_xplatform2_3.nc...");
   {
      hid_t fileid, grpid, attid;
      hid_t file_typeid1[NUM_OBJ], native_typeid1[NUM_OBJ];
      hid_t file_typeid2, native_typeid2;
      hsize_t num_obj, i;
      H5O_info_t obj_info;
      char obj_name[NC_MAX_NAME + 1];

      /* Open one of the netCDF test files. */
      if ((fileid = H5Fopen(REF_FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* How many objects in this group? */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      if (num_obj != NUM_OBJ) ERR;

      /* For each object in the group... */
      for (i = 0; i < num_obj; i++)
      {
	 /* Get the name. */
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC,
				i, &obj_info, H5P_DEFAULT) < 0) ERR_RET;
	 if (H5Lget_name_by_idx(grpid, ".", H5_INDEX_NAME, H5_ITER_INC, i,
				obj_name, NC_MAX_NAME + 1, H5P_DEFAULT) < 0) ERR_RET;
	 printf(" reading type %s ", obj_name);
	 if (obj_info.type != H5O_TYPE_NAMED_DATATYPE) ERR_RET;

	 /* Get the typeid. */
	 if ((file_typeid1[i] = H5Topen2(grpid, obj_name, H5P_DEFAULT)) < 0) ERR_RET;
	 if ((native_typeid1[i] = H5Tget_native_type(file_typeid1[i], H5T_DIR_DEFAULT)) < 0) ERR_RET;
      }

      /* There is one att: open it by index. */
      if ((attid = H5Aopen_idx(grpid, 0)) < 0) ERR;

      /* Get file and native typeids. */
      if ((file_typeid2 = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid2 = H5Tget_native_type(file_typeid2, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Close the attribute. */
      if (H5Aclose(attid) < 0) ERR;

      /* Close the typeids. */
      if (H5Tclose(file_typeid2) < 0) ERR_RET;
      if (H5Tclose(native_typeid2) < 0) ERR_RET;
      for (i = 0; i < NUM_OBJ; i++)
      {
	 if (H5Tclose(file_typeid1[i]) < 0) ERR_RET;
	 if (H5Tclose(native_typeid1[i]) < 0) ERR_RET;
      }

      /* Close the group and file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Opening tst_xplatform2_3.nc again...");
   {
      hid_t fileid, grpid, attid, file_typeid, native_typeid;
      hid_t file_typeid2, native_typeid2;
      hsize_t num_obj;

      /* Open one of the netCDF test files. */
      if ((fileid = H5Fopen(REF_FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* There is one att: open it by index. */
      if ((attid = H5Aopen_idx(grpid, 0)) < 0) ERR;

      /* Get file and native typeids. */
      if ((file_typeid = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid = H5Tget_native_type(file_typeid, H5T_DIR_DEFAULT)) < 0) ERR;

      /* Now getting another copy of the native typeid will fail! WTF? */
      if ((file_typeid2 = H5Aget_type(attid)) < 0) ERR;
      if ((native_typeid2 = H5Tget_native_type(file_typeid, H5T_DIR_DEFAULT)) < 0) ERR;
      
      /* Close the attribute. */
      if (H5Aclose(attid) < 0) ERR;

      /* Close the typeids. */
      if (H5Tclose(file_typeid) < 0) ERR;
      if (H5Tclose(native_typeid) < 0) ERR;
      if (H5Tclose(file_typeid2) < 0) ERR;
      if (H5Tclose(native_typeid2) < 0) ERR;

      /* Close the group and file. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
#endif
   FINAL_RESULTS;
}
