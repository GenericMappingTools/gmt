/* This is part of the netCDF package.  Copyright 2005-2011 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

*/

#include <err_macros.h>
#include <hdf5.h>
#include <H5DSpublic.h>
#include <ncdimscale.h>

#define FILE_NAME "tst_h_dimscales4.h5"
#define DIMSCALE_NAME "AAA_dimscale"
#define VAR1_NAME "Watson"
#define VAR2_NAME "Holmes"
#define VAR3_NAME "Moriarty"
#define NDIMS 1
#define DIM_LEN 1
#define NAME_ATTRIBUTE "dimscale_name_attribute"
#define DIMSCALE_LABEL "dimscale_label"
#define STR_LEN 255
#define NC_MAX_NAME STR_LEN
#define NC_EHDFERR 255
#define DIM_WITHOUT_VARIABLE "This is a netCDF dimension but not a netCDF variable."

/* typedef struct { */
/*       unsigned long 	fileno;		/\*file number			*\/ */
/*       haddr_t 		objno;		/\*object number			*\/ */
/* } HDF5_OBJID_T;  */

struct nc_hdf5_link_info 
{
   char name[STR_LEN];
   H5I_type_t obj_type;   
};   

static herr_t
visit_link(hid_t g_id, const char *name, const H5L_info_t *info, 
	   void *op_data)  
{
   /* A positive return value causes the visit iterator to immediately
    * return that positive value, indicating short-circuit
    * success. The iterator can be restarted at the next group
    * member. */
   int ret = 1;
   hid_t id;
   
   strncpy(((struct nc_hdf5_link_info *)op_data)->name, name, NC_MAX_NAME);
   
   /* Open this critter. */
   if ((id = H5Oopen_by_addr(g_id, info->u.address)) < 0) 
      return NC_EHDFERR;
   
   /* Is this critter a group, type, data, attribute, or what? */
   if ((((struct nc_hdf5_link_info *)op_data)->obj_type = H5Iget_type(id)) < 0)
      ret = NC_EHDFERR;

   /* Close the critter to release resouces. */
   if (H5Oclose(id) < 0)
      return NC_EHDFERR;
   
   return ret;
}

herr_t alien_visitor(hid_t did, unsigned dim, hid_t dsid, 
		     void *visitor_data)
{
   char name1[STR_LEN];
   H5G_stat_t statbuf;
   HDF5_OBJID_T *objid = visitor_data;

   /* This should get "/var1", the name of the dataset that the scale
    * is attached to. */
   /*if (H5Iget_name(did, name1, STR_LEN) < 0) ERR;*/
   
/*   printf("visiting did 0x%x dim %d dsid 0x%x name of did %s \n", 
     did, dim, dsid, name1);*/

   /* Get more info on the dimscale object.*/
   if (H5Gget_objinfo(dsid, ".", 1, &statbuf) < 0) ERR;
   objid->fileno[0] = statbuf.fileno[0];
   objid->objno[0] = statbuf.objno[0];
   objid->fileno[1] = statbuf.fileno[1];
   objid->objno[1] = statbuf.objno[1];
/*   printf("for dsid: statbuf.fileno = %d statbuf.objno = %d\n", 
     statbuf.fileno, statbuf.objno);*/

   if (H5Gget_objinfo(did, ".", 1, &statbuf) < 0) ERR;
/*   printf("for did: statbuf.fileno = %d statbuf.objno = %d\n", 
     statbuf.fileno, statbuf.objno);*/
   return 0;
}

int
main()
{
   printf("\n*** Checking HDF5 dimscales detach.\n");
   printf("*** Creating a file with two vars with one dimension scale...");
   {
      hid_t fileid, grpid, spaceid, var1_id, var2_id, dimscaleid, cparmsid;
      hid_t fcpl_id, fapl_id, create_propid, access_propid;
      hsize_t dims[NDIMS] = {DIM_LEN};
      char dimscale_wo_var[STR_LEN];
      float data = 42;

      /* Create file. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_cache(fapl_id, 0, CHUNK_CACHE_NELEMS, CHUNK_CACHE_SIZE,
		       CHUNK_CACHE_PREEMPTION) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_18, H5F_LIBVER_18) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Create dimension scale. */
      if ((create_propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_attr_creation_order(create_propid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((dimscaleid = H5Dcreate1(grpid, DIMSCALE_NAME, H5T_IEEE_F32BE,
				   spaceid, create_propid)) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Pclose(create_propid) < 0) ERR;
      sprintf(dimscale_wo_var, "%s%10d", DIM_WITHOUT_VARIABLE, DIM_LEN);
      if (H5DSset_scale(dimscaleid, dimscale_wo_var) < 0) ERR;

      /* Create a variable that uses this dimension scale. */
      if ((access_propid = H5Pcreate(H5P_DATASET_ACCESS)) < 0) ERR;
      if (H5Pset_chunk_cache(access_propid, CHUNK_CACHE_NELEMS,
   			     CHUNK_CACHE_SIZE, CHUNK_CACHE_PREEMPTION) < 0) ERR;
      if ((create_propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_fill_value(create_propid, H5T_NATIVE_FLOAT, &data) < 0) ERR;
      if (H5Pset_layout(create_propid, H5D_CONTIGUOUS) < 0) ERR;
      if (H5Pset_attr_creation_order(create_propid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((spaceid = H5Screate_simple(NDIMS, dims, dims)) < 0) ERR;
      if ((var1_id = H5Dcreate2(grpid, VAR1_NAME, H5T_NATIVE_FLOAT, spaceid, 
				H5P_DEFAULT, create_propid, access_propid)) < 0) ERR;
      if (H5Pclose(create_propid) < 0) ERR;
      if (H5Pclose(access_propid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5DSattach_scale(var1_id, dimscaleid, 0) < 0) ERR;

      /* Create another variable that uses this dimension scale. */
      if ((access_propid = H5Pcreate(H5P_DATASET_ACCESS)) < 0) ERR;
      if (H5Pset_chunk_cache(access_propid, CHUNK_CACHE_NELEMS,
   			     CHUNK_CACHE_SIZE, CHUNK_CACHE_PREEMPTION) < 0) ERR;
      if ((create_propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_fill_value(create_propid, H5T_NATIVE_FLOAT, &data) < 0) ERR;
      if (H5Pset_layout(create_propid, H5D_CONTIGUOUS) < 0) ERR;
      if (H5Pset_attr_creation_order(create_propid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((spaceid = H5Screate_simple(NDIMS, dims, dims)) < 0) ERR;
      if ((var2_id = H5Dcreate2(grpid, VAR2_NAME, H5T_NATIVE_FLOAT, spaceid, 
				H5P_DEFAULT, create_propid, access_propid)) < 0) ERR;
      if (H5Pclose(create_propid) < 0) ERR;
      if (H5Pclose(access_propid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5DSattach_scale(var2_id, dimscaleid, 0) < 0) ERR;

      /* Now detach the scales and remove the dimscale. This doesn't
       * work if I reverse the order of the statements. */
      if (H5DSdetach_scale(var2_id, dimscaleid, 0) < 0) ERR;     
      if (H5DSdetach_scale(var1_id, dimscaleid, 0) < 0) ERR;      

      /* Fold up our tents. */
      if (H5Dclose(var1_id) < 0) ERR;
      if (H5Dclose(dimscaleid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* /\* Now read the file and check it. *\/ */
      /* { */
      /* 	 hid_t fileid, spaceid = 0, datasetid = 0; */
      /* 	 hsize_t num_obj, i; */
      /* 	 int obj_class; */
      /* 	 char obj_name[STR_LEN + 1]; */
      /* 	 char dimscale_name[STR_LEN+1]; */
      /* 	 htri_t is_scale; */
      /* 	 char label[STR_LEN+1]; */
      /* 	 int num_scales; */
      /* 	 hsize_t dims[1], maxdims[1]; */
      /* 	 H5G_stat_t statbuf; */
      /* 	 HDF5_OBJID_T dimscale_obj, vars_dimscale_obj; */
      /* 	 struct nc_hdf5_link_info link_info; */
      /* 	 hsize_t idx = 0; */

      /* 	 /\* Open the file. *\/ */
      /* 	 if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR; */
      /* 	 if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR; */
      
      /* 	 /\* Loop through objects in the root group. *\/ */
      /* 	 if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR; */
      /* 	 for (i = 0; i < num_obj; i++) */
      /* 	 { */

      /* 	    if (H5Literate(grpid, H5_INDEX_CRT_ORDER, H5_ITER_INC,  */
      /* 			   &idx, visit_link, (void *)&link_info) < 0) ERR; */

      /* 	    printf("Encountered: HDF5 object link_info.name %s\n", link_info.name); */

      /* 	    /\* Deal with object based on its obj_class. *\/ */
      /* 	    switch(link_info.obj_type) */
      /* 	    { */
      /* 	       case H5I_GROUP: */
      /* 		  break; */
      /* 	       case H5I_DATASET: */
      /* 		  /\* Open the dataset. *\/ */
      /* 		  if ((datasetid = H5Dopen1(fileid, link_info.name)) < 0) ERR; */

      /* 		  if ((spaceid = H5Dget_space(datasetid)) < 0) ERR; */
      /* 		  if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR; */
      /* 		  if (maxdims[0] != DIM_LEN) ERR; */
      /* 		  if (H5Sclose(spaceid) < 0) ERR; */

      /* 		  /\* Is this a dimscale? *\/ */
      /* 		  if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR; */
      /* 		  if (is_scale && strcmp(link_info.name, DIMSCALE_NAME)) ERR; */
      /* 		  if (is_scale) */
      /* 		  { */
      /* 		     /\* A dimscale comes with a NAME attribute, in */
      /* 		      * addition to its real name. *\/ */
      /* 		     if (H5DSget_scale_name(datasetid, dimscale_name, STR_LEN) < 0) ERR; */
      /* 		     if (strcmp(dimscale_name, dimscale_wo_var)) ERR; */

      /* 		     /\* fileno and objno uniquely identify an object and a */
      /* 		      * HDF5 file. *\/ */
      /* 		     if (H5Gget_objinfo(datasetid, ".", 1, &statbuf) < 0) ERR;  */
      /* 		     dimscale_obj.fileno[0] = statbuf.fileno[0]; */
      /* 		     dimscale_obj.objno[0] = statbuf.objno[0]; */
      /* 		     dimscale_obj.fileno[1] = statbuf.fileno[1]; */
      /* 		     dimscale_obj.objno[1] = statbuf.objno[1]; */
      /* 		     /\*printf("scale statbuf.fileno = %d statbuf.objno = %d\n", */
      /* 		       statbuf.fileno, statbuf.objno);*\/ */

      /* 		  } */
      /* 		  else */
      /* 		  { */
      /* 		     /\* Here's how to get the number of scales attached */
      /* 		      * to the dataset's dimension 0. *\/ */
      /* 		     if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR; */
      /* 		     if (num_scales != 1) ERR; */

      /* 		     /\* Go through all dimscales for this var and learn about them. *\/ */
      /* 		     if (H5DSiterate_scales(datasetid, 0, NULL, alien_visitor, */
      /* 					    &vars_dimscale_obj) < 0) ERR; */
      /* 		     /\*printf("vars_dimscale_obj.fileno = %d vars_dimscale_obj.objno = %d\n", */
      /* 		       vars_dimscale_obj.fileno, vars_dimscale_obj.objno);*\/ */
      /* 		     /\* if (vars_dimscale_obj.fileno[0] != dimscale_obj.fileno[0] || *\/ */
      /* 		     /\* 	 vars_dimscale_obj.objno[0] != dimscale_obj.objno[0] || *\/ */
      /* 		     /\* 	 vars_dimscale_obj.fileno[1] != dimscale_obj.fileno[1] || *\/ */
      /* 		     /\* 	 vars_dimscale_obj.objno[1] != dimscale_obj.objno[1]) ERR; *\/ */
		  
      /* 		     /\* There's also a label for dimension 0. *\/ */
      /* 		     if (H5DSget_label(datasetid, 0, label, STR_LEN) < 0) ERR; */

      /* 		     /\*printf("found non-scale dataset %s, label %s\n", link_info.name, label);*\/ */
      /* 		  } */
      /* 		  if (H5Dclose(datasetid) < 0) ERR; */
      /* 		  break; */
      /* 	       case H5I_DATATYPE: */
      /* 		  break; */
      /* 	       default: */
      /* 		  printf("Unknown object!"); */
      /* 		  ERR; */
      /* 	    } */
      /* 	 } */

      /* 	 /\* Close up the shop. *\/ */
      /* 	 if (H5Fclose(fileid) < 0) ERR; */
      /*   }*/
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

