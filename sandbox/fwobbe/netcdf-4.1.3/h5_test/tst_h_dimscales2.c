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
#include <ncdimscale.h>

#define FILE_NAME "tst_h_dimscales2.h5"
#define DIMSCALE_NAME "dimscale"
#define VAR1_NAME "var1"
#define NDIMS 1
#define DIM1_LEN 3
#define NAME_ATTRIBUTE "dimscale_name_attribute"
#define DIMSCALE_LABEL "dimscale_label"
#define STR_LEN 255

/* typedef struct { */
/*       unsigned long 	fileno;		/\*file number			*\/ */
/*       haddr_t 		objno;		/\*object number			*\/ */
/* } HDF5_OBJID_T;  */

herr_t alien_visitor(hid_t did, unsigned dim, hid_t dsid, 
		     void *visitor_data)
{
   char name1[STR_LEN];
   H5G_stat_t statbuf;
   HDF5_OBJID_T *objid = visitor_data;

   /* This should get "/var1", the name of the dataset that the scale
    * is attached to. */
   if (H5Iget_name(did, name1, STR_LEN) < 0) ERR;
   if (strcmp(&name1[1], VAR1_NAME)) ERR;
   
   /*printf("visiting did 0x%x dim %d dsid 0x%x name of did %s \n", 
     did, dim, dsid, name1);*/

   /* Get more info on the dimscale object.*/
   if (H5Gget_objinfo(dsid, ".", 1, &statbuf) < 0) ERR;
   objid->fileno[0] = statbuf.fileno[0];
   objid->objno[0] = statbuf.objno[0];
   objid->fileno[1] = statbuf.fileno[1];
   objid->objno[1] = statbuf.objno[1];
   /*printf("for dsid: statbuf.fileno = %d statbuf.objno = %d\n", 
     statbuf.fileno, statbuf.objno);*/

   if (H5Gget_objinfo(did, ".", 1, &statbuf) < 0) ERR;
   /*printf("for did: statbuf.fileno = %d statbuf.objno = %d\n", 
     statbuf.fileno, statbuf.objno);*/
   return 0;
}

herr_t alien_visitor2(hid_t did, unsigned dim, hid_t dsid, void *visitor_data)
{
   char name1[STR_LEN];
   H5G_stat_t statbuf;
   HDF5_OBJID_T *objid = visitor_data;

   if (H5Iget_name(did, name1, STR_LEN) < 0) ERR;
   /*printf("visiting did 0x%x dim %d dsid 0x%x name of did %s \n",  
     did, dim, dsid, name1); */

   /* Get obj id of the dimscale object. THis will be used later to
    * match dimensions to dimscales. */
   if (H5Gget_objinfo(dsid, ".", 1, &statbuf) < 0) ERR;
   objid->fileno[0] = statbuf.fileno[0];
   objid->objno[0] = statbuf.objno[0];
   objid->fileno[1] = statbuf.fileno[1];
   objid->objno[1] = statbuf.objno[1];

   return 0;
}

int
main()
{
   printf("\n*** Checking HDF5 dimscales some more.\n");
   printf("*** Creating a file with one var with one dimension scale...");
   
   {
      hid_t fileid, spaceid, datasetid, dimscaleid, cparmsid;
      hsize_t dims[NDIMS] = {DIM1_LEN}, maxdims[NDIMS] = {H5S_UNLIMITED};

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
			      H5P_DEFAULT)) < 0) ERR;

      /* Create the space that will be used both for the dimscale and
       * the 1D dataset that will attach it. */
      if ((spaceid = H5Screate_simple(NDIMS, dims, maxdims)) < 0) ERR;

      /* Modify dataset creation properties, i.e. enable chunking. */
      dims[0] = 1;
      if ((cparmsid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(cparmsid, NDIMS, dims) < 0) ERR;

      /* Create our dimension scale, as an unlimited dataset. */
      if ((dimscaleid = H5Dcreate(fileid, DIMSCALE_NAME, H5T_NATIVE_INT,
				  spaceid, cparmsid)) < 0) ERR;
      if (H5DSset_scale(dimscaleid, NAME_ATTRIBUTE) < 0) ERR;

      /* Create a variable which uses it. */
      if ((datasetid = H5Dcreate(fileid, VAR1_NAME, H5T_NATIVE_INT,
				 spaceid, cparmsid)) < 0) ERR;
      if (H5DSattach_scale(datasetid, dimscaleid, 0) < 0) ERR;
      if (H5DSset_label(datasetid, 0, DIMSCALE_LABEL) < 0) ERR;

      /* Fold up our tents. */
      if (H5Dclose(dimscaleid) < 0 ||
	  H5Dclose(datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking that one var, one dimscale file can be read...");

   {
      hid_t fileid, spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      char dimscale_name[STR_LEN+1];
      htri_t is_scale;
      char label[STR_LEN+1];
      int num_scales;
      hsize_t dims[1], maxdims[1];
      H5G_stat_t statbuf;
      HDF5_OBJID_T dimscale_obj, vars_dimscale_obj;

      /* Open the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      
      /* Loop through objects in the root group. */
      if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(fileid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(fileid, i, obj_name, STR_LEN) < 0) ERR;

	 /*printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n",
	   obj_class, obj_name);*/

	 /* Deal with object based on its obj_class. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen1(fileid, obj_name)) < 0) ERR;

	       /* This should be an unlimited dataset. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR;
	       if (maxdims[0] != H5S_UNLIMITED) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale && strcmp(obj_name, DIMSCALE_NAME)) ERR;
	       if (is_scale)
	       {
		  /* A dimscale comes with a NAME attribute, in
		   * addition to its real name. */
		  if (H5DSget_scale_name(datasetid, dimscale_name, STR_LEN) < 0) ERR;
		  if (strcmp(dimscale_name, NAME_ATTRIBUTE)) ERR;

		  /* fileno and objno uniquely identify an object and a
		   * HDF5 file. */
		  if (H5Gget_objinfo(datasetid, ".", 1, &statbuf) < 0) ERR;
		  dimscale_obj.fileno[0] = statbuf.fileno[0];
		  dimscale_obj.objno[0] = statbuf.objno[0];
		  dimscale_obj.fileno[1] = statbuf.fileno[1];
		  dimscale_obj.objno[1] = statbuf.objno[1];
		  /*printf("statbuf.fileno = %d statbuf.objno = %d\n",
		    statbuf.fileno, statbuf.objno);*/

	       }
	       else
	       {
		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn about them. */
		  if (H5DSiterate_scales(datasetid, 0, NULL, alien_visitor,
					 &vars_dimscale_obj) < 0) ERR;
		  /*printf("vars_dimscale_obj.fileno = %d vars_dimscale_obj.objno = %d\n",
		    vars_dimscale_obj.fileno, vars_dimscale_obj.objno);*/
		  if (vars_dimscale_obj.fileno[0] != dimscale_obj.fileno[0] ||
		      vars_dimscale_obj.objno[0] != dimscale_obj.objno[0] ||
		      vars_dimscale_obj.fileno[1] != dimscale_obj.fileno[1] ||
		      vars_dimscale_obj.objno[1] != dimscale_obj.objno[1]) ERR;
		  
		  /* There's also a label for dimension 0. */
		  if (H5DSget_label(datasetid, 0, label, STR_LEN) < 0) ERR;

		  /*printf("found non-scale dataset %s, label %s\n", obj_name, label);*/
	       }
	       if (H5Dclose(datasetid) < 0) ERR;
	       break;
	    case H5G_TYPE:
	       break;
	    case H5G_LINK:
	       break;
	    default:
	       printf("Unknown object class %d!", obj_class);
	 }
      }

      /* Close up the shop. */
      if (H5Sclose(spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating a file with one var with two dimension scales...");
   
   {
#define LAT_LEN 3
#define LON_LEN 2
#define DIMS_2 2
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define PRES_NAME "pres"
      
      hid_t fileid, lat_spaceid, lon_spaceid, pres_spaceid;
      hid_t pres_datasetid, lat_dimscaleid, lon_dimscaleid;
      hsize_t dims[DIMS_2];

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
			      H5P_DEFAULT)) < 0) ERR;

      /* Create the spaces that will be used for the dimscales. */
      dims[0] = LAT_LEN;
      if ((lat_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = LON_LEN;
      if ((lon_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      if ((pres_spaceid = H5Screate_simple(DIMS_2, dims, dims)) < 0) ERR;

      /* Create our dimension scales. */
      if ((lat_dimscaleid = H5Dcreate(fileid, LAT_NAME, H5T_NATIVE_INT,
				      lat_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lat_dimscaleid, NULL) < 0) ERR;
      if ((lon_dimscaleid = H5Dcreate(fileid, LON_NAME, H5T_NATIVE_INT,
				      lon_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lon_dimscaleid, NULL) < 0) ERR;

      /* Create a variable which uses these two dimscales. */
      if ((pres_datasetid = H5Dcreate(fileid, PRES_NAME, H5T_NATIVE_FLOAT,
				      pres_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lat_dimscaleid, 0) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lon_dimscaleid, 1) < 0) ERR;

      /* Fold up our tents. */
      if (H5Dclose(lat_dimscaleid) < 0 ||
	  H5Dclose(lon_dimscaleid) < 0 ||
	  H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(lat_spaceid) < 0 ||
	  H5Sclose(lon_spaceid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking that one var, two dimscales file can be read...");

   {
#define NDIMS2 2
      hid_t fileid, spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;
      hsize_t dims[NDIMS2], maxdims[NDIMS2];
      H5G_stat_t statbuf;
      HDF5_OBJID_T dimscale_obj[2], vars_dimscale_obj[2];
      int dimscale_cnt = 0;
      int d, ndims;

      /* Open the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      
      /* Loop through objects in the root group. */
      if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(fileid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(fileid, i, obj_name, STR_LEN) < 0) ERR;

/* 	 printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n", */
/* 		obj_class, obj_name); */

	 /* Deal with object based on its obj_class. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen1(fileid, obj_name)) < 0) ERR;

	       /* Get space info. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR;
	       if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;
	       if (ndims > NDIMS2) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale)
	       {
		  /* fileno and objno uniquely identify an object and a
		   * HDF5 file. */
		  if (H5Gget_objinfo(datasetid, ".", 1, &statbuf) < 0) ERR;
		  dimscale_obj[dimscale_cnt].fileno[0] = statbuf.fileno[0];
		  dimscale_obj[dimscale_cnt].objno[0] = statbuf.objno[0];
		  dimscale_obj[dimscale_cnt].fileno[1] = statbuf.fileno[1];
		  dimscale_obj[dimscale_cnt].objno[1] = statbuf.objno[1];
/* 		  printf("dimscale_obj[%d].fileno = %d dimscale_obj[%d].objno = %d\n", */
/* 			 dimscale_cnt, dimscale_obj[dimscale_cnt].fileno, dimscale_cnt,  */
/* 			 dimscale_obj[dimscale_cnt].objno); */
		  dimscale_cnt++;
	       }
	       else
	       {
		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0 and 1. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;
		  if ((num_scales = H5DSget_num_scales(datasetid, 1)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn about them. */
		  for (d = 0; d < ndims; d++)
		  {
		     if (H5DSiterate_scales(datasetid, d, NULL, alien_visitor2,
		     &(vars_dimscale_obj[d])) < 0) ERR;

		     /* Verify that the object ids passed from the
		      * alien_visitor2 function match the ones we found
		      * for the lat and lon datasets. */
		     if (vars_dimscale_obj[d].fileno[0] != dimscale_obj[d].fileno[0] ||
		     vars_dimscale_obj[d].objno[0] != dimscale_obj[d].objno[0]) ERR;
		     if (vars_dimscale_obj[d].fileno[1] != dimscale_obj[d].fileno[1] ||
		     vars_dimscale_obj[d].objno[1] != dimscale_obj[d].objno[1]) ERR;
		  }
	       }
	       if (H5Dclose(datasetid) < 0) ERR;
	       if (H5Sclose(spaceid) < 0) ERR;
	       break;
	    case H5G_TYPE:
	       break;
	    case H5G_LINK:
	       break;
	    default:
	       printf("Unknown object class %d!", obj_class);
	 }
      }

      /* Close up the shop. */
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Creating a file with one var with two unlimited dimension scales...");
   {
#define U1_LEN 3
#define U2_LEN 2
#define DIMS2 2
#define U1_NAME "u1"
#define U2_NAME "u2"
#define VNAME "v1"
      
      hid_t fapl_id, fcpl_id, grpid, plistid, plistid2;
      hid_t fileid, lat_spaceid, lon_spaceid, pres_spaceid;
      hid_t pres_datasetid, lat_dimscaleid, lon_dimscaleid;
      hsize_t dims[DIMS2], maxdims[DIMS2], chunksize[DIMS2] = {10, 10};
      hid_t spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;
      H5G_stat_t statbuf;
      HDF5_OBJID_T dimscale_obj[2], vars_dimscale_obj[2];
      int dimscale_cnt = 0;
      int d, ndims;

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

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Create the spaces that will be used for the dimscales. */
      dims[0] = 0;
      maxdims[0] = H5S_UNLIMITED;
      if ((lat_spaceid = H5Screate_simple(1, dims, maxdims)) < 0) ERR;
      if ((lon_spaceid = H5Screate_simple(1, dims, maxdims)) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = 0;
      dims[1] = 0;
      maxdims[0] = H5S_UNLIMITED;
      maxdims[1] = H5S_UNLIMITED;
      if ((pres_spaceid = H5Screate_simple(DIMS2, dims, maxdims)) < 0) ERR;

      /* Set up the dataset creation property list for the two dimensions. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(plistid, 1, chunksize) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;

      /* Create our dimension scales. */
      if ((lat_dimscaleid = H5Dcreate(grpid, U1_NAME, H5T_NATIVE_INT,
				      lat_spaceid, plistid)) < 0) ERR;
      if (H5DSset_scale(lat_dimscaleid, NULL) < 0) ERR;
      if ((lon_dimscaleid = H5Dcreate(grpid, U2_NAME, H5T_NATIVE_INT,
				      lon_spaceid, plistid)) < 0) ERR;
      if (H5DSset_scale(lon_dimscaleid, NULL) < 0) ERR;

      /* Set up the dataset creation property list for the variable. */
      if ((plistid2 = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(plistid2, DIMS2, chunksize) < 0) ERR;
      if (H5Pset_attr_creation_order(plistid2, H5P_CRT_ORDER_TRACKED|
				     H5P_CRT_ORDER_INDEXED) < 0) ERR;

      /* Create a variable which uses these two dimscales. */
      if ((pres_datasetid = H5Dcreate(grpid, VNAME, H5T_NATIVE_DOUBLE, pres_spaceid,
				      plistid2)) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lat_dimscaleid, 0) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lon_dimscaleid, 1) < 0) ERR;

      /* Close down the show. */
      if (H5Pclose(fapl_id) < 0 ||
	  H5Pclose(fcpl_id) < 0 ||
	  H5Dclose(lat_dimscaleid) < 0 ||
	  H5Dclose(lon_dimscaleid) < 0 ||
	  H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(lat_spaceid) < 0 ||
	  H5Sclose(lon_spaceid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Pclose(plistid) < 0 ||
	  H5Pclose(plistid2) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Open the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      
      /* Loop through objects in the root group. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;

      for (i = 0; i < num_obj; i++)
      {
	 /*Get the type (i.e. group, dataset, etc.), and the name of
	   the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(grpid, i, obj_name, STR_LEN) < 0) ERR;

	 /* Deal with object based on its obj_class. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen1(grpid, obj_name)) < 0) ERR;

	       /* Get space info. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR;
	       if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale)
	       {
		  /* fileno and objno uniquely identify an object and a
		   * HDF5 file. */
		  if (H5Gget_objinfo(datasetid, ".", 1, &statbuf) < 0) ERR;
		  dimscale_obj[dimscale_cnt].fileno[0] = statbuf.fileno[0];
		  dimscale_obj[dimscale_cnt].objno[0] = statbuf.objno[0];
		  dimscale_obj[dimscale_cnt].fileno[1] = statbuf.fileno[1];
		  dimscale_obj[dimscale_cnt].objno[1] = statbuf.objno[1];
		  dimscale_cnt++;
	       }
	       else
	       {
		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0 and 1. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;
		  if ((num_scales = H5DSget_num_scales(datasetid, 1)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn about them. */
		  for (d = 0; d < ndims; d++)
		  {
		     if (H5DSiterate_scales(datasetid, d, NULL, alien_visitor2,
					    &(vars_dimscale_obj[d])) < 0) ERR;

		     /* Verify that the object ids passed from the
		      * alien_visitor2 function match the ones we found
		      * for the lat and lon datasets. */
		     if (vars_dimscale_obj[d].fileno[0] != dimscale_obj[d].fileno[0] ||
			 vars_dimscale_obj[d].objno[0] != dimscale_obj[d].objno[0]) ERR;
		     if (vars_dimscale_obj[d].fileno[1] != dimscale_obj[d].fileno[1] ||
			 vars_dimscale_obj[d].objno[1] != dimscale_obj[d].objno[1]) ERR;
		  }

	       }

	       if (H5Dclose(datasetid) < 0) ERR;
	       break;
	    case H5G_TYPE:
	       break;
	    case H5G_LINK:
	       break;
	    default:
	       printf("Unknown object class %d!", obj_class);
	 }
     }

      /* Check the dimension lengths. */
      {
	 hid_t spaceid1;
	 hsize_t h5dimlen[DIMS2], h5dimlenmax[DIMS2];
	 int dataset_ndims;

	 /* Check U1. */
	 if ((datasetid = H5Dopen1(grpid, U1_NAME)) < 0) ERR;
	 if ((spaceid1 = H5Dget_space(datasetid)) < 0) ERR;
	 if ((dataset_ndims = H5Sget_simple_extent_dims(spaceid1, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 1 || h5dimlen[0] != 0 || h5dimlenmax[0] != H5S_UNLIMITED) ERR;
	 if (H5Dclose(datasetid) ||
	     H5Sclose(spaceid1)) ERR;

	 /* Check U2. */
	 if ((datasetid = H5Dopen1(grpid, U2_NAME)) < 0) ERR;
	 if ((spaceid1 = H5Dget_space(datasetid)) < 0) ERR;
	 if ((dataset_ndims = H5Sget_simple_extent_dims(spaceid1, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 1 || h5dimlen[0] != 0 || h5dimlenmax[0] != H5S_UNLIMITED) ERR;
	 if (H5Dclose(datasetid) ||
	     H5Sclose(spaceid1)) ERR;
	 
	 /* Check V1. */
	 if ((datasetid = H5Dopen1(grpid, VNAME)) < 0) ERR;
	 if ((spaceid1 = H5Dget_space(datasetid)) < 0) ERR;
	 if ((dataset_ndims = H5Sget_simple_extent_dims(spaceid1, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 2 || h5dimlen[0] != 0 || h5dimlen[1] != 0 ||
	     h5dimlenmax[0] != H5S_UNLIMITED || h5dimlenmax[1] != H5S_UNLIMITED) ERR;

	 /* All done. */
	 if (H5Dclose(datasetid) ||
	     H5Sclose(spaceid1)) ERR;
      }

      /* Write two hyperslabs. */
      {
#define NUM_VALS 3
	 hid_t file_spaceid, mem_spaceid;
	 hsize_t h5dimlen[DIMS2], h5dimlenmax[DIMS2], xtend_size[DIMS2] = {1, NUM_VALS};
	 hsize_t start[DIMS2] = {0, 0};
	 hsize_t count[DIMS2] = {1, NUM_VALS};
	 double value[NUM_VALS];
	 int dataset_ndims;
	 int i;

	 /* Set up phony data. */
	 for (i = 0; i < NUM_VALS; i++)
	    value[i] = (float)i;

	 /* Open the dataset, check its dimlens. */
	 if ((datasetid = H5Dopen1(grpid, VNAME)) < 0) ERR;
	 if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;
	 if ((dataset_ndims = H5Sget_simple_extent_dims(file_spaceid, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 2 || h5dimlen[0] != 0 || h5dimlen[1] != 0 ||
	     h5dimlenmax[0] != H5S_UNLIMITED || h5dimlenmax[1] != H5S_UNLIMITED) ERR;

	 /* Extend the size of the dataset. */
	 if (H5Dextend(datasetid, xtend_size) < 0) ERR;
	 if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;

	 /* Check the size. */
	 if ((dataset_ndims = H5Sget_simple_extent_dims(file_spaceid, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 2 || h5dimlen[0] != 1 || h5dimlen[1] != NUM_VALS ||
	     h5dimlenmax[0] != H5S_UNLIMITED || h5dimlenmax[1] != H5S_UNLIMITED) ERR;

	 /* Set up the file and memory spaces. */
	 if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET,
				 start, NULL, count, NULL) < 0) ERR;
	 if ((mem_spaceid = H5Screate_simple(DIMS2, count, NULL)) < 0) ERR;

	 /* Write a slice of data. */
	 if (H5Dwrite(datasetid, H5T_NATIVE_DOUBLE, mem_spaceid, file_spaceid,
		      H5P_DEFAULT, value) < 0)

	 /* Check the size. */
	 if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;
	 if ((dataset_ndims = H5Sget_simple_extent_dims(file_spaceid, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 2 || h5dimlen[0] != 1 || h5dimlen[1] != NUM_VALS ||
	     h5dimlenmax[0] != H5S_UNLIMITED || h5dimlenmax[1] != H5S_UNLIMITED) ERR;

	 /* Extend the size of the dataset for the second slice. */
	 xtend_size[0]++;
	 if (H5Dextend(datasetid, xtend_size) < 0) ERR;
	 if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;

	 /* Set up the file and memory spaces for a second slice. */
	 start[0]++;
	 if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET,
				 start, NULL, count, NULL) < 0) ERR;
	 if ((mem_spaceid = H5Screate_simple(DIMS2, count, NULL)) < 0) ERR;

	 /* Write a second slice of data. */
	 if (H5Dwrite(datasetid, H5T_NATIVE_DOUBLE, mem_spaceid, file_spaceid,
		      H5P_DEFAULT, value) < 0)

	 /* Check the size again. */
	 if ((file_spaceid = H5Dget_space(datasetid)) < 0) ERR;
	 if ((dataset_ndims = H5Sget_simple_extent_dims(file_spaceid, h5dimlen,
							h5dimlenmax)) < 0) ERR;
	 if (dataset_ndims != 2 || h5dimlen[0] != 2 || h5dimlen[1] != NUM_VALS ||
	     h5dimlenmax[0] != H5S_UNLIMITED || h5dimlenmax[1] != H5S_UNLIMITED) ERR;

	 /* All done. */
	 if (H5Dclose(datasetid) ||
	     H5Sclose(mem_spaceid) ||
	     H5Sclose(file_spaceid)) ERR;
      }

      /* Close up the shop. */
      if (H5Sclose(spaceid)) ERR;
      if (H5Gclose(grpid) < 0 ||
      H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking dimension scales with attached dimension scales...");
   
   {
#define LAT_LEN 3
#define LON_LEN 2
#define TIME_LEN 5
#define LEN_LEN 10
#define DIMS_3 3
#define NUM_DIMSCALES1 4
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define PRES_NAME1 "z_pres"
#define TIME_NAME "time"
#define LEN_NAME "u_len"
      
      hid_t fileid, lat_spaceid, lon_spaceid, time_spaceid, pres_spaceid, len_spaceid;
      hid_t pres_datasetid, lat_dimscaleid, lon_dimscaleid, time_dimscaleid, len_dimscaleid;
      hid_t fapl_id, fcpl_id;
      hsize_t dims[DIMS_3];
      hid_t spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;
      hsize_t maxdims[DIMS_3];
      H5G_stat_t statbuf;
      HDF5_OBJID_T dimscale_obj[NUM_DIMSCALES1], vars_dimscale_obj[NUM_DIMSCALES1];
      int dimscale_cnt = 0;
      int d, ndims;

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

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Create the spaces that will be used for the dimscales. */
      dims[0] = LAT_LEN;
      if ((lat_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = LON_LEN;
      if ((lon_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = TIME_LEN;
      if ((time_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = LEN_LEN;
      if ((len_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      dims[2] = TIME_LEN;
      if ((pres_spaceid = H5Screate_simple(DIMS_3, dims, dims)) < 0) ERR;

      /* Create our dimension scales. */
      if ((lat_dimscaleid = H5Dcreate1(fileid, LAT_NAME, H5T_NATIVE_INT,
				      lat_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lat_dimscaleid, NULL) < 0) ERR;
      if ((lon_dimscaleid = H5Dcreate1(fileid, LON_NAME, H5T_NATIVE_INT,
				      lon_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lon_dimscaleid, NULL) < 0) ERR;
      if ((time_dimscaleid = H5Dcreate1(fileid, TIME_NAME, H5T_NATIVE_INT,
				      time_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(time_dimscaleid, NULL) < 0) ERR;
      if ((len_dimscaleid = H5Dcreate1(fileid, LEN_NAME, H5T_NATIVE_INT,
				      len_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(len_dimscaleid, NULL) < 0) ERR;

      /* Create a variable which uses these three dimscales. */
      if ((pres_datasetid = H5Dcreate1(fileid, PRES_NAME1, H5T_NATIVE_FLOAT,
				      pres_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lat_dimscaleid, 0) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lon_dimscaleid, 1) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, time_dimscaleid, 2) < 0) ERR;

      /* Attach a dimscale to a dimscale. Unfortunately, HDF5 does not
       * allow this. Woe is me. */
      /*if (H5DSattach_scale(time_dimscaleid, len_dimscaleid, 0) < 0) ERR;*/

      /* Fold up our tents. */
      if (H5Dclose(lat_dimscaleid) < 0 ||
	  H5Dclose(lon_dimscaleid) < 0 ||
	  H5Dclose(time_dimscaleid) < 0 ||
	  H5Dclose(len_dimscaleid) < 0 ||
	  H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(lat_spaceid) < 0 ||
	  H5Sclose(lon_spaceid) < 0 ||
	  H5Sclose(time_spaceid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Sclose(len_spaceid) < 0 ||
	  H5Pclose(fapl_id) < 0 ||
	  H5Pclose(fcpl_id) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Open the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      
      /* Loop through objects in the root group. */
      if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(fileid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(fileid, i, obj_name, STR_LEN) < 0) ERR;

 	 /* printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n",  */
/*  		obj_class, obj_name);  */

	 /* Deal with object based on its obj_class. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen1(fileid, obj_name)) < 0) ERR;

	       /* Get space info. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR;
	       if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale)
	       {
		  /* fileno and objno uniquely identify an object and a
		   * HDF5 file. */
		  if (H5Gget_objinfo(datasetid, ".", 1, &statbuf) < 0) ERR;
		  dimscale_obj[dimscale_cnt].fileno[0] = statbuf.fileno[0];
		  dimscale_obj[dimscale_cnt].objno[0] = statbuf.objno[0];
		  dimscale_obj[dimscale_cnt].fileno[1] = statbuf.fileno[1];
		  dimscale_obj[dimscale_cnt].objno[1] = statbuf.objno[1];
		  /* printf("dimscale_obj[%d].fileno = %d dimscale_obj[%d].objno = %d\n", */
/* 			 dimscale_cnt, dimscale_obj[dimscale_cnt].fileno, dimscale_cnt, */
/* 			 dimscale_obj[dimscale_cnt].objno); */
		  dimscale_cnt++;
	       }
	       else
	       {
		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0 and 1. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;
		  if ((num_scales = H5DSget_num_scales(datasetid, 1)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn about them. */
		  for (d = 0; d < ndims; d++)
		  {
		     if (H5DSiterate_scales(datasetid, d, NULL, alien_visitor2,
		     &(vars_dimscale_obj[d])) < 0) ERR;

		     /* Verify that the object ids passed from the
		      * alien_visitor2 function match the ones we found
		      * for the lat and lon datasets. */
		     if (vars_dimscale_obj[d].fileno[0] != dimscale_obj[d].fileno[0] ||
		     vars_dimscale_obj[d].objno[0] != dimscale_obj[d].objno[0]) ERR;
		     if (vars_dimscale_obj[d].fileno[1] != dimscale_obj[d].fileno[1] ||
		     vars_dimscale_obj[d].objno[1] != dimscale_obj[d].objno[1]) ERR;
		  }
	       }
	       if (H5Dclose(datasetid) < 0) ERR;
	       if (H5Sclose(spaceid) < 0) ERR;
	       break;
	    case H5G_TYPE:
	       break;
	    case H5G_LINK:
	       break;
	    default:
	       printf("Unknown object class %d!", obj_class);
	 }
      }

      /* Close up the shop. */
      if (H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking cration ordering of datasets which are also dimension scales...");
   
   {
#define LAT_LEN 3
#define LON_LEN 2
#define TIME_LEN 5
#define LEN_LEN 10
#define DIMS_3 3
#define NUM_DIMSCALES2 4
#define LAT_NAME "lat"
#define LON_NAME "lon"
#define PRES_NAME1 "z_pres"
#define TIME_NAME "time"
#define LEN_NAME "u_len"
      
      hid_t fileid, lat_spaceid, lon_spaceid, time_spaceid, pres_spaceid, len_spaceid;
      hid_t pres_datasetid, lat_dimscaleid, lon_dimscaleid, time_dimscaleid, len_dimscaleid;
      hid_t fapl_id, fcpl_id;
      hsize_t dims[DIMS_3];
      hid_t spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;
      hsize_t maxdims[DIMS_3];
      H5G_stat_t statbuf;
      HDF5_OBJID_T dimscale_obj[NUM_DIMSCALES2], vars_dimscale_obj[NUM_DIMSCALES2];
      int dimscale_cnt = 0;
      int d, ndims;

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

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      /* Create the spaces that will be used for the dimscales. */
      dims[0] = LAT_LEN;
      if ((lat_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = LON_LEN;
      if ((lon_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = TIME_LEN;
      if ((time_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      dims[0] = LEN_LEN;
      if ((len_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

      /* Create the space for the dataset. */
      dims[0] = LAT_LEN;
      dims[1] = LON_LEN;
      dims[2] = TIME_LEN;
      if ((pres_spaceid = H5Screate_simple(DIMS_3, dims, dims)) < 0) ERR;

      /* Create our dimension scales. */
      if ((lat_dimscaleid = H5Dcreate1(fileid, LAT_NAME, H5T_NATIVE_INT,
				      lat_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lat_dimscaleid, NULL) < 0) ERR;
      if ((lon_dimscaleid = H5Dcreate1(fileid, LON_NAME, H5T_NATIVE_INT,
				      lon_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lon_dimscaleid, NULL) < 0) ERR;
      if ((time_dimscaleid = H5Dcreate1(fileid, TIME_NAME, H5T_NATIVE_INT,
				      time_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(time_dimscaleid, NULL) < 0) ERR;
      if ((len_dimscaleid = H5Dcreate1(fileid, LEN_NAME, H5T_NATIVE_INT,
				      len_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(len_dimscaleid, NULL) < 0) ERR;

      /* Create a variable which uses these three dimscales. */
      if ((pres_datasetid = H5Dcreate1(fileid, PRES_NAME1, H5T_NATIVE_FLOAT,
				      pres_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lat_dimscaleid, 0) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, lon_dimscaleid, 1) < 0) ERR;
      if (H5DSattach_scale(pres_datasetid, time_dimscaleid, 2) < 0) ERR;

      /* Attach a dimscale to a dimscale. Unfortunately, HDF5 does not
       * allow this. Woe is me. */
      /*if (H5DSattach_scale(time_dimscaleid, len_dimscaleid, 0) < 0) ERR;*/

      /* Fold up our tents. */
      if (H5Dclose(lat_dimscaleid) < 0 ||
	  H5Dclose(lon_dimscaleid) < 0 ||
	  H5Dclose(time_dimscaleid) < 0 ||
	  H5Dclose(len_dimscaleid) < 0 ||
	  H5Dclose(pres_datasetid) < 0 ||
	  H5Sclose(lat_spaceid) < 0 ||
	  H5Sclose(lon_spaceid) < 0 ||
	  H5Sclose(time_spaceid) < 0 ||
	  H5Sclose(pres_spaceid) < 0 ||
	  H5Sclose(len_spaceid) < 0 ||
	  H5Pclose(fapl_id) < 0 ||
	  H5Pclose(fcpl_id) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Open the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      
      /* Loop through objects in the root group. */
      if (H5Gget_num_objs(fileid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(fileid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(fileid, i, obj_name, STR_LEN) < 0) ERR;

 	 /* printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n",  */
/*  		obj_class, obj_name);  */

	 /* Deal with object based on its obj_class. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen1(fileid, obj_name)) < 0) ERR;

	       /* Get space info. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR;
	       if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale)
	       {
		  /* fileno and objno uniquely identify an object and a
		   * HDF5 file. */
		  if (H5Gget_objinfo(datasetid, ".", 1, &statbuf) < 0) ERR;
		  dimscale_obj[dimscale_cnt].fileno[0] = statbuf.fileno[0];
		  dimscale_obj[dimscale_cnt].objno[0] = statbuf.objno[0];
		  dimscale_obj[dimscale_cnt].fileno[1] = statbuf.fileno[1];
		  dimscale_obj[dimscale_cnt].objno[1] = statbuf.objno[1];
		  /* printf("dimscale_obj[%d].fileno = %d dimscale_obj[%d].objno = %d\n", */
/* 			 dimscale_cnt, dimscale_obj[dimscale_cnt].fileno, dimscale_cnt, */
/* 			 dimscale_obj[dimscale_cnt].objno); */
		  dimscale_cnt++;
	       }
	       else
	       {
		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0 and 1. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;
		  if ((num_scales = H5DSget_num_scales(datasetid, 1)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn about them. */
		  for (d = 0; d < ndims; d++)
		  {
		     if (H5DSiterate_scales(datasetid, d, NULL, alien_visitor2,
		     &(vars_dimscale_obj[d])) < 0) ERR;

		     /* Verify that the object ids passed from the
		      * alien_visitor2 function match the ones we found
		      * for the lat and lon datasets. */
		     if (vars_dimscale_obj[d].fileno[0] != dimscale_obj[d].fileno[0] ||
		     vars_dimscale_obj[d].objno[0] != dimscale_obj[d].objno[0]) ERR;
		     if (vars_dimscale_obj[d].fileno[1] != dimscale_obj[d].fileno[1] ||
		     vars_dimscale_obj[d].objno[1] != dimscale_obj[d].objno[1]) ERR;
		  }
	       }
	       if (H5Dclose(datasetid) < 0) ERR;
	       if (H5Sclose(spaceid) < 0) ERR;
	       break;
	    case H5G_TYPE:
	       break;
	    case H5G_LINK:
	       break;
	    default:
	       printf("Unknown object class %d!", obj_class);
	 }
      }

      /* Close up the shop. */
      if (H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

