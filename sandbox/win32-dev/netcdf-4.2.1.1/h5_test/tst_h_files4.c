/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.
*/

#include <err_macros.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_files4.h5"
#define STR_LEN 255

/* Heavy duty test file...*/
/*#define FILE_NAME "/machine/downloads/T159_1978110112.nc4"*/

herr_t
obj_iter(hid_t o_id, const char *name, const H5O_info_t *object_info, 
	 void *op_data) 
{
   *(int *)op_data = object_info->type;
   return 1;
}

/* This is a callback function for H5Literate(). 

The parameters of this callback function have the following values or
meanings:

g_id Group that serves as root of the iteration; same value as the
H5Lvisit group_id parameter

name Name of link, relative to g_id, being examined at current step of
the iteration

info H5L_info_t struct containing information regarding that link

op_data User-defined pointer to data required by the application in
processing the link; a pass-through of the op_data pointer provided
with the H5Lvisit function call

*/
herr_t
op_func (hid_t g_id, const char *name, const H5L_info_t *info, 
	 void *op_data)  
{
   hid_t id;
   H5I_type_t obj_type;

   strcpy((char *)op_data, name);
   if ((id = H5Oopen_by_addr(g_id, info->u.address)) < 0) ERR;

/* Using H5Ovisit is really slow. Use H5Iget_type for a fast
 * answer. */
/*    if (H5Ovisit(id, H5_INDEX_CRT_ORDER, H5_ITER_INC, obj_iter,  */
/* 		(void *)&obj_class) != 1) ERR; */

   if ((obj_type = H5Iget_type(id)) < 0) ERR;
   if (H5Oclose(id) < 0) ERR;

/* Turn this on to learn what type of object you've opened. */
/*    switch (obj_type) */
/*    { */
/*       case H5I_GROUP: */
/* 	 printf("group %s\n", name); */
/* 	 break; */
/*       case H5I_DATATYPE: */
/* 	 printf("type %s\n", name); */
/* 	 break; */
/*       case H5I_DATASET: */
/* 	 printf("data %s\n", name); */
/* 	 break; */
/*       default: */
/* 	 printf("unknown class\n"); */
/*    } */
   return 1;
}

int
main()
{
   printf("\n*** Checking HDF5 file functions.\n");
   printf("*** Creating HDF5 file in the canonical netCDF-4 way...");
   {
      hid_t fapl_id, fcpl_id, fileid, grpid, fileid2;
      hsize_t num_obj;

      /* Create file access and create property lists. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      
      /* Set latest_format in access propertly list. This ensures that
       * the latest, greatest, HDF5 versions are used in the file. */ 
/*      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, 
	H5F_LIBVER_LATEST) < 0) ERR;*/

      /* Set H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering in the file. */
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Set close degree. */
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;

      /* Create the file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Close up. */
      if (H5Pclose(fapl_id) < 0 ||
	  H5Pclose(fcpl_id) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Reopen the file and check it. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;

      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, fapl_id)) < 0) ERR;
      if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR;
      if (num_obj) ERR;

      /* Open another copy of the same file. Must use the same file
       * access degree or HDF5 will not open the file. */
      if ((fileid2 = H5Fopen(FILE_NAME, H5F_ACC_RDWR, fapl_id)) < 0) ERR;

      if (H5Fclose(fileid) < 0) ERR;
      if (H5Fclose(fileid2) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Opening a HDF5 file with H5Literate...");
   {
      hid_t fapl_id, fileid, grpid;
      hsize_t idx = 0;
      char obj_name[STR_LEN + 1];
      hsize_t num_obj;
      int i;

      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_cache(fapl_id, 0, CHUNK_CACHE_NELEMS, CHUNK_CACHE_SIZE, 
		       CHUNK_CACHE_PREEMPTION) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i = 0; i < num_obj; i++)
      {
	 if (H5Literate(grpid, H5_INDEX_CRT_ORDER, H5_ITER_INC, &idx, op_func, 
			(void *)obj_name) != 1) ERR;
	 printf("encountered object %s\n", obj_name);
      }

      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Opening a HDF5 file in the canonical netCDF-4 way...");
   {
      hid_t fapl_id, fileid, grpid;
      H5_index_t idx_field = H5_INDEX_CRT_ORDER;
      H5O_info_t obj_info;
      hsize_t num_obj;
      ssize_t size;
      char obj_name[STR_LEN + 1];
      int i;

      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_cache(fapl_id, 0, CHUNK_CACHE_NELEMS, CHUNK_CACHE_SIZE, 
		       CHUNK_CACHE_PREEMPTION) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* How many objects in this group? */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i = 0; i < num_obj; i++)
      {
	 if (H5Oget_info_by_idx(grpid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 
				i, &obj_info, H5P_DEFAULT)) ERR;
	 if ((size = H5Lget_name_by_idx(grpid, ".", idx_field, H5_ITER_INC, i,
					NULL, 0, H5P_DEFAULT)) < 0) ERR;
	 if (H5Lget_name_by_idx(grpid, ".", idx_field, H5_ITER_INC, i,
				obj_name, size+1, H5P_DEFAULT) < 0) ERR;
      }

      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
