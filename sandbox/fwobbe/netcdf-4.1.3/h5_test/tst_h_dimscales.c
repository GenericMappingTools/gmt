/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   $Id$
*/
#include <err_macros.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_dimscales.h5"
#define STR_LEN 255
#define MAX_DIMS 255

herr_t alien_visitor(hid_t did, unsigned dim, hid_t dsid, 
		     void *visitor_data)
{
   char name1[STR_LEN], name2[STR_LEN];
   H5G_stat_t statbuf;

   (*(hid_t *)visitor_data) = dsid;
   if (H5Iget_name(did, name1, STR_LEN) < 0) ERR;
   if (H5Iget_name(dsid, name2, STR_LEN) < 0) ERR;
/*    printf("visiting did 0x%x dim %d dsid 0x%x name of did %s \n",  */
/* 	  did, dim, dsid, name1); */
/*    printf("name of dsid: %s\n", name2); */

   if (H5Gget_objinfo(did, ".", 1, &statbuf) < 0) ERR;
/*   printf("statbuf.fileno = %d statbuf.objno = %d\n", 
     statbuf.fileno, statbuf.objno);*/
   
   return 0;
}

int
rec_scan_group(hid_t grpid)
{
   hid_t spaceid, datasetid = 0, child_grpid;
   hsize_t num_obj, i;
   int obj_class;
   char obj_name[STR_LEN + 1];
   htri_t is_scale;
   int num_scales;
   hsize_t dims[MAX_DIMS], max_dims[MAX_DIMS];
   int ndims, d;
   
   /* Loop through datasets to find variables. */
   if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
   for (i=0; i<num_obj; i++)
   {
      /* Get the type (i.e. group, dataset, etc.), and the name of
       * the object. */
      if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
      if (H5Gget_objname_by_idx(grpid, i, obj_name, STR_LEN) < 0) ERR;
      /*printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n", 
	obj_class, obj_name);*/

      /* Deal with groups and datasets, ignore the rest. */
      switch(obj_class)
      {
	 case H5G_GROUP:
	    if ((child_grpid = H5Gopen(grpid, obj_name)) < 0) ERR;
	    if (rec_scan_group(child_grpid)) ERR;
	    break;
	 case H5G_DATASET:
	    /* Open the dataset. */
	    if ((datasetid = H5Dopen(grpid, obj_name)) < 0) ERR;
	    /*printf("\nobj_name %s\n", obj_name);*/

	    /* Get the dimensions of this dataset. */
	    if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	    if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;
	    if (ndims > MAX_DIMS) ERR;
	    if (H5Sget_simple_extent_dims(spaceid, dims, max_dims) < 0) ERR;

	    /* Is this a dimscale? */
	    if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	    if (is_scale)
	    {
	       /*printf("dimension scale! Hoorah for the Pirate King!\n");*/
	    }
	    else
	    {
	       int visitor_data = 0;
		  
	       /* Here's how to get the number of scales attached
		* to the dataset's dimension 0. */
	       if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
	       if (num_scales != 1) ERR;

	       /* Go through all dimscales for this var and learn about them. */
	       for (d = 0; d < ndims; d++)
		  if (H5DSiterate_scales(datasetid, d, NULL, alien_visitor,
					 &visitor_data) < 0) ERR;
	    }
	    if (H5Dclose(datasetid) < 0) ERR;
	    break;
	 case H5G_TYPE:
	    printf("found a type!\n");
	    break;
	 case H5G_LINK:
	    printf("found a link! Yikes!\n");
	    break;
	 default:
	    printf("Unknown object class %d!", obj_class);
      }
   }

   return 0;
}

int
main()
{
   printf("\n*** Checking HDF5 dimension scales.\n");
#define GRP_NAME "simple_scales"
#define DIMSCALE_NAME "dimscale"
#define NAME_ATTRIBUTE "Billy-Bob"
#define VAR1_NAME "var1"
#define VAR2_NAME "var2"
#define VAR3_NAME "var3"
#define DIM1_LEN 3
#define DIM2_LEN 2
#define FIFTIES_SONG "Mamma said they'll be days like this. They'll be days like this, my mamma said."

   printf("*** Creating simple dimension scales file...");
   {
      hid_t fileid, grpid, dimscaleid;
      hid_t dimscale_spaceid, var1_spaceid, var3_spaceid;
      hid_t var1_datasetid, var2_datasetid, var3_datasetid;
      hsize_t dims[2] = {DIM1_LEN, DIM2_LEN};
      hsize_t dimscale_dims[1] = {DIM1_LEN};

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create our dimension scale. Use the built-in NAME attribute
       * on the dimscale. */
      if ((dimscale_spaceid = H5Screate_simple(1, dimscale_dims, 
					       dimscale_dims)) < 0) ERR;
      if ((dimscaleid = H5Dcreate(grpid, DIMSCALE_NAME, H5T_NATIVE_INT, 
				  dimscale_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(dimscaleid, NAME_ATTRIBUTE) < 0) ERR;

      /* Create a 1D variable which uses the dimscale. Attach a label
       * to this scale. */
      if ((var1_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((var1_datasetid = H5Dcreate(grpid, VAR1_NAME, H5T_NATIVE_INT, 
				      var1_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(var1_datasetid, dimscaleid, 0) < 0) ERR;
      if (H5DSset_label(var1_datasetid, 0, FIFTIES_SONG) < 0) ERR;

      /* Create a 1D variabls that doesn't use the dimension scale. */
      if ((var2_datasetid = H5Dcreate(grpid, VAR2_NAME, H5T_NATIVE_INT, 
				      var1_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Create a 2D dataset which uses the scale for one of its
       * dimensions. */
      if ((var3_spaceid = H5Screate_simple(2, dims, dims)) < 0) ERR;
      if ((var3_datasetid = H5Dcreate(grpid, VAR3_NAME, H5T_NATIVE_INT, 
				      var3_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(var3_datasetid, dimscaleid, 0) < 0) ERR;

      /* Close up the shop. */
      if (H5Dclose(dimscaleid) < 0 ||
	  H5Dclose(var1_datasetid) < 0 ||
	  H5Dclose(var2_datasetid) < 0 ||
	  H5Dclose(var3_datasetid) < 0 ||
	  H5Sclose(var1_spaceid) < 0 ||
	  H5Sclose(var3_spaceid) < 0 ||
	  H5Sclose(dimscale_spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* HELP! If you are reading this in the future, and time
       * machines have been invented, please come back to July 10,
       * 2005, the Java Java coffee shop in Lafayette, 8:00 am MST +-
       * 20 minutes. Bring back some advanced weapons systems to
       * destroy the sound system here, which is playing 50's rock and
       * roll. Do-op, do-op, la-ma la-ma, ding dong. Save me!!! (Mind
       * you, James Brown is a different story!) */
   }
   SUMMARIZE_ERR;
   printf("*** Checking that simple dimscale file can be read...");
   {
      hid_t fileid, grpid, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;

      /* Reopen the file and group. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      
      /* Loop through datasets to find variables. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of the
	  * object. Confusingly, this is a different type than the type
	  * of a variable. This type might be better called "class" or
	  * "type of type"  */
	 if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(grpid, i, obj_name, STR_LEN) < 0) ERR;
	 /*printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n", obj_class, obj_name);*/

	 /* Deal with groups and datasets. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:

	       /*Close the last datasetid, if one is open. */
	       if (datasetid > 0)
	       {
		  H5Dclose(datasetid);
		  datasetid = 0;
	       }
	       
	       if ((datasetid = H5Dopen(grpid, obj_name)) < 0) ERR;
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale && strcmp(obj_name, DIMSCALE_NAME)) ERR;
	       if (is_scale)
	       {
		  char nom_de_quincey[STR_LEN+1];

		  /* A dimscale comes with a NAME attribute, in
		   * addition to its real name. */
		  if (H5DSget_scale_name(datasetid, nom_de_quincey, 
					 STR_LEN) < 0) ERR;
		  if (strcmp(nom_de_quincey, NAME_ATTRIBUTE)) ERR;

		  /*printf("found scale %s, NAME %s\n", obj_name, nom_de_quincey);*/

	       }
	       else
	       {
		  char label[STR_LEN+1];

		  /* Here's how to get the number of scales attached
		   * to the dataset. I would think that this would
		   * return 0 scales for a dataset that doesn't have
		   * scales, but instead it errors. So take an error
		   * to be the same as no dimension scales. */
		  num_scales = H5DSget_num_scales(datasetid, 0);
		  if (strcmp(obj_name, VAR1_NAME) == 0 && num_scales != 1) ERR;
		  if (strcmp(obj_name, VAR2_NAME) == 0 && num_scales > 0) ERR;
		  if (strcmp(obj_name, VAR3_NAME) == 0 && num_scales != 1) ERR;
		  
		  /* There's also a label for dimension 0 of var1. */
		  if (strcmp(obj_name, VAR1_NAME) == 0)
		  {
		     if (H5DSget_label(datasetid, 0, label, STR_LEN) < 0) ERR;
		     if (strcmp(label, FIFTIES_SONG)) ERR;
		  }
	       }
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
      if (H5Dclose(datasetid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating simple dimension scales file with lots of datasets...");

#define NUM_DATASETS 500
   {
      hid_t fileid, grpid, dimscaleid;
      hid_t dimscale_spaceid, var1_spaceid;
      hid_t var1_datasetid[NUM_DATASETS];
      hsize_t dims[2] = {DIM1_LEN, DIM2_LEN};
      hsize_t dimscale_dims[1] = {DIM1_LEN};
      char var_name[STR_LEN + 1];
      int v;

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create our dimension scale. Use the built-in NAME attribute
       * on the dimscale. */
      if ((dimscale_spaceid = H5Screate_simple(1, dimscale_dims, 
					       dimscale_dims)) < 0) ERR;
      if ((dimscaleid = H5Dcreate(grpid, DIMSCALE_NAME, H5T_NATIVE_INT, 
				  dimscale_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(dimscaleid, NAME_ATTRIBUTE) < 0) ERR;

      /* Create many 1D datasets which use the dimscale. */
      if ((var1_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      for (v = 0; v < NUM_DATASETS; v++)
      {
	 sprintf(var_name, "var_%d", v);
	 if ((var1_datasetid[v] = H5Dcreate(grpid, var_name, H5T_NATIVE_INT, 
					    var1_spaceid, H5P_DEFAULT)) < 0) ERR;
	 if (H5DSattach_scale(var1_datasetid[v], dimscaleid, 0) < 0) ERR;
      }

      /* Close up the shop. */
      for (v = 0; v < NUM_DATASETS; v++)
	 if (H5Dclose(var1_datasetid[v]) < 0) ERR;
      if (H5Dclose(dimscaleid) < 0 ||
	  H5Sclose(var1_spaceid) < 0 ||
	  H5Sclose(dimscale_spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating a file with an unlimited dimension scale...");

   {
      hid_t fileid, grpid, spaceid, datasetid, dimscaleid, cparmsid;
      hsize_t dims[1] = {1}, maxdims[1] = {H5S_UNLIMITED};

      /* Create file and group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;

      if ((spaceid = H5Screate_simple(1, dims, maxdims)) < 0) ERR;

      /* Modify dataset creation properties, i.e. enable chunking  */
      if ((cparmsid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(cparmsid, 1, dims) < 0) ERR;

      /* Create our dimension scale, as an unlimited dataset. */
      if ((dimscaleid = H5Dcreate(grpid, DIMSCALE_NAME, H5T_NATIVE_INT, 
				  spaceid, cparmsid)) < 0) ERR;
      if (H5DSset_scale(dimscaleid, NAME_ATTRIBUTE) < 0) ERR;

      /* Create a variable which uses it. */
      if ((datasetid = H5Dcreate(grpid, VAR1_NAME, H5T_NATIVE_INT, 
				 spaceid, cparmsid)) < 0) ERR;
      if (H5DSattach_scale(datasetid, dimscaleid, 0) < 0) ERR;
      if (H5DSset_label(datasetid, 0, "dimension label") < 0) ERR;

      /* Close up the shop. */
      if (H5Dclose(dimscaleid) < 0 ||
	  H5Dclose(datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
#ifdef EXTRA_TESTS
   printf("*** Checking that unlimited dimscale file can be read...");

   {
      hid_t fileid, grpid, spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;
      hsize_t dims[1], maxdims[1];

      /* Reopen the file and group. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      
      /* Loop through datasets to find variables. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(grpid, i, obj_name, STR_LEN) < 0) ERR;
	 /*printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n", obj_class, obj_name);*/

	 /* Deal with groups and datasets. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:

	       /*Close the last datasetid, if one is open. */
	       if (datasetid > 0)
	       {
		  H5Dclose(datasetid);
		  datasetid = 0;
	       }
	       
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen(grpid, obj_name)) < 0) ERR;

	       /* This should be an unlimited dataset. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, maxdims) < 0) ERR;
	       if (maxdims[0] != H5S_UNLIMITED) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale && strcmp(obj_name, DIMSCALE_NAME)) ERR;
	       if (is_scale)
	       {
		  char nom_de_quincey[STR_LEN+1];

		  /* A dimscale comes with a NAME attribute, in
		   * addition to its real name. */
		  if (H5DSget_scale_name(datasetid, nom_de_quincey, STR_LEN) < 0) ERR;
		  /*printf("found scale %s, NAME %s\n", obj_name, nom_de_quincey);*/

	       }
	       else
	       {
		  char label[STR_LEN+1];
		  int visitor_data = 0;

		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn about them. */
		  if (H5DSiterate_scales(datasetid, 0, NULL, alien_visitor, 
					 &visitor_data) < 0) ERR;
		  
		  /* There's also a label for dimension 0. */
		  if (H5DSget_label(datasetid, 0, label, STR_LEN) < 0) ERR;

		  /*printf("found non-scale dataset %s, label %s\n", obj_name, label);*/
	       }
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
      if (H5Dclose(datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating some 3D datasets using shared dimscales...");

   {
#define NDIMS 3
#define TIME_DIM 0      
#define LAT_DIM 1
#define LON_DIM 2
#define LAT_LEN 2
#define LON_LEN 3
#define LAT_NAME "Lat"
#define LON_NAME "Lon"
#define TIME_NAME "Time"
#define PRES_NAME "Pressure"
#define TEMP_NAME "Temperature"

      hid_t fileid, grpid, lat_spaceid, lon_spaceid, time_spaceid, spaceid;
      hid_t lat_scaleid, lon_scaleid, time_scaleid;
      hid_t pres_dsid, temp_dsid, cparmsid;
      hsize_t dims[NDIMS], max_dims[NDIMS];

      /* Create file and group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;

      /* Create 3 1D spaces for the 3 dimension scale datasets. Time
       * starts out as size 0. It's an unlimited dimension scale. */
      dims[0] = 0;
      max_dims[0] = H5S_UNLIMITED;
      if ((time_spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;
      dims[0] = LAT_LEN;
      max_dims[0] = LAT_LEN;
      if ((lat_spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;
      dims[0] = LON_LEN;
      max_dims[0] = LON_LEN;
      if ((lon_spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;

      /* Enable chunking for unlimited time scale.  */
      if ((cparmsid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      dims[TIME_DIM] = 1;
      if (H5Pset_chunk(cparmsid, 1, dims) < 0) ERR;

      /* Create our dimension scales. */
      if ((time_scaleid = H5Dcreate(grpid, TIME_NAME, H5T_NATIVE_INT,
				    time_spaceid, cparmsid)) < 0) ERR;
      if (H5DSset_scale(time_scaleid, TIME_NAME) < 0) ERR;
      if ((lat_scaleid = H5Dcreate(grpid, LAT_NAME, H5T_NATIVE_FLOAT,
				   lat_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lat_scaleid, LAT_NAME) < 0) ERR;
      if ((lon_scaleid = H5Dcreate(grpid, LON_NAME, H5T_NATIVE_FLOAT,
				   lon_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(lon_scaleid, LON_NAME) < 0) ERR;

      /* Create a space coresponding to these three dimensions. */
      dims[TIME_DIM] = 0;
      dims[LAT_DIM] = LAT_LEN;
      dims[LON_DIM] = LON_LEN;
      max_dims[TIME_DIM] = H5S_UNLIMITED;
      max_dims[LAT_DIM] = LAT_LEN;
      max_dims[LON_DIM] = LON_LEN;
      if ((spaceid = H5Screate_simple(NDIMS, dims, max_dims)) < 0) ERR;

      /* Create two variables which use them, and attach the dimension scales. */
      dims[TIME_DIM] = 1;
      if (H5Pset_chunk(cparmsid, NDIMS, dims) < 0) ERR;
      if ((pres_dsid = H5Dcreate(grpid, PRES_NAME, H5T_NATIVE_FLOAT,
				 spaceid, cparmsid)) < 0) ERR;
      if (H5DSattach_scale(pres_dsid, time_scaleid, 0) < 0) ERR;
      if (H5DSattach_scale(pres_dsid, lat_scaleid, 1) < 0) ERR;
      if (H5DSattach_scale(pres_dsid, lon_scaleid, 2) < 0) ERR;
      if (H5DSset_label(pres_dsid, TIME_DIM, TIME_NAME) < 0) ERR;
      if (H5DSset_label(pres_dsid, LAT_DIM, LAT_NAME) < 0) ERR;
      if (H5DSset_label(pres_dsid, LON_DIM, LON_NAME) < 0) ERR;
      if ((temp_dsid = H5Dcreate(grpid, TEMP_NAME, H5T_NATIVE_FLOAT,
				 spaceid, cparmsid)) < 0) ERR;
      if (H5DSattach_scale(temp_dsid, time_scaleid, 0) < 0) ERR;
      if (H5DSattach_scale(temp_dsid, lat_scaleid, 1) < 0) ERR;
      if (H5DSattach_scale(temp_dsid, lon_scaleid, 2) < 0) ERR;
      if (H5DSset_label(temp_dsid, TIME_DIM, TIME_NAME) < 0) ERR;
      if (H5DSset_label(temp_dsid, LAT_DIM, LAT_NAME) < 0) ERR;
      if (H5DSset_label(temp_dsid, LON_DIM, LON_NAME) < 0) ERR;

      /* Close up the shop. */
      if (H5Dclose(pres_dsid) < 0 ||
	  H5Dclose(temp_dsid) < 0 ||
	  H5Dclose(lat_scaleid) < 0 ||
	  H5Dclose(lon_scaleid) < 0 ||
	  H5Dclose(time_scaleid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking 3D datasets created with shared dimscales...");

   {
      hid_t fileid, grpid, spaceid = 0, datasetid = 0;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[STR_LEN + 1];
      htri_t is_scale;
      int num_scales;
      hsize_t dims[NDIMS], max_dims[NDIMS];
      int d;

      /* Reopen the file and group. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      
      /* Loop through datasets to find variables. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the type (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(grpid, i, obj_name, STR_LEN) < 0) ERR;
	 /*printf("\nEncountered: HDF5 object obj_class %d obj_name %s\n", obj_class, obj_name);*/

	 /* Deal with groups and datasets. */
	 switch(obj_class)
	 {
	    case H5G_GROUP:
	       break;
	    case H5G_DATASET:
	       /* Open the dataset. */
	       if ((datasetid = H5Dopen(grpid, obj_name)) < 0) ERR;
	       /*printf("\nobj_name %s\n", obj_name);*/

	       /* Get the dimensions of this dataset. */
	       if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
	       if (H5Sget_simple_extent_dims(spaceid, dims, max_dims) < 0) ERR;

	       /* Is this a dimscale? */
	       if ((is_scale = H5DSis_scale(datasetid)) < 0) ERR;
	       if (is_scale)
	       {
		  char nom_de_quincey[STR_LEN+1];

		  /* A dimscale comes with a NAME attribute, in
		   * addition to its real name. */
		  if (H5DSget_scale_name(datasetid, nom_de_quincey, 
					 STR_LEN) < 0) ERR;
		  /*printf("found scale %s, NAME %s id 0x%x\n", obj_name, 
		    nom_de_quincey, datasetid);*/

		  /* Check size depending on name. */
		  if ((!strcmp(obj_name, LAT_NAME) && dims[TIME_DIM] != LAT_LEN) ||
		      (!strcmp(obj_name, LON_NAME) && dims[TIME_DIM] != LON_LEN) ||
		      (!strcmp(obj_name, TIME_NAME) && 
		       max_dims[TIME_DIM] != H5S_UNLIMITED)) ERR;

	       }
	       else
	       {
		  char label[STR_LEN+1];
		  int visitor_data = 0;
		  
		  /* SHould have these dimensions... */
		  if (dims[TIME_DIM] != 0 || dims[LAT_DIM] != LAT_LEN || 
		      dims[LON_DIM] != LON_LEN) ERR;
		  if (max_dims[TIME_DIM] != H5S_UNLIMITED) ERR;

		  /* Here's how to get the number of scales attached
		   * to the dataset's dimension 0. */
		  if ((num_scales = H5DSget_num_scales(datasetid, 0)) < 0) ERR;
		  if (num_scales != 1) ERR;

		  /* Go through all dimscales for this var and learn
		   * about them. What I want is the dataset id of each
		   * dimscale. Then... */
		  for (d = 0; d < NDIMS; d++)
		     if (H5DSiterate_scales(datasetid, d, NULL, alien_visitor,
					    &visitor_data) < 0) ERR;
		  /*printf("visitor_data: 0x%x\n", visitor_data);*/
		  
		  /* There's also a label for each dimension. */
		  if (H5DSget_label(datasetid, 0, label, STR_LEN) < 0) ERR;
		  if (strcmp(label, TIME_NAME)) ERR;
		  if (H5DSget_label(datasetid, 1, label, STR_LEN) < 0) ERR;
		  if (strcmp(label, LAT_NAME)) ERR;
		  if (H5DSget_label(datasetid, 2, label, STR_LEN) < 0) ERR;
		  if (strcmp(label, LON_NAME)) ERR;
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
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Creating 3D datasets using shared dimscales in groups...");

   {
#define FATHER "Adam"
#define GOOD_CHILD "Able"
#define BAD_CHILD "Cain"
#define DISTANCE_LEN 3
#define SMELLINESS_NAME "Smelliness"
#define DISTANCE_NAME "Distance"
#define TIME_NAME "Time"
#define TIME_DIM 0      
#define SMELLINESS_DIM 1
#define DISTANCE_DIM 2
#define GOAT_NAME "Billy_goat_gruff"
#define CAMEL_NAME "Grumpy_the_camel"

      hid_t fileid, smelliness_spaceid, distance_spaceid, time_spaceid, spaceid;
      hid_t adam_grpid, able_grpid, cain_grpid;
      hid_t time_scaleid, smelliness_scaleid, distance_scaleid;
      hid_t goat_dsid, camel_dsid, cparmsid;
      hsize_t dims[NDIMS], max_dims[NDIMS];

      /* Create file and group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
			      H5P_DEFAULT)) < 0) ERR;
      if ((adam_grpid = H5Gcreate(fileid, FATHER, 0)) < 0) ERR;
      if ((able_grpid = H5Gcreate(adam_grpid, GOOD_CHILD, 0)) < 0) ERR;
      if ((cain_grpid = H5Gcreate(adam_grpid, BAD_CHILD, 0)) < 0) ERR;

      /* Create 3 1D spaces for the 3 dimension scale datasets. Time
       * and smelliness starts out as 0. They are unlimited dimension
       * scales. */
      dims[0] = 0;
      max_dims[0] = H5S_UNLIMITED;
      if ((time_spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;
      dims[0] = 0;
      max_dims[0] = H5S_UNLIMITED;
      if ((smelliness_spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;
      dims[0] = DISTANCE_LEN;
      max_dims[0] = DISTANCE_LEN;
      if ((distance_spaceid = H5Screate_simple(1, dims, max_dims)) < 0) ERR;

      /* Enable chunking for unlimited time and smelliness scale. */
      if ((cparmsid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      dims[0] = 1;
      if (H5Pset_chunk(cparmsid, 1, dims) < 0) ERR;

      /* Create our dimension scales. */
      if ((time_scaleid = H5Dcreate(adam_grpid, TIME_NAME, H5T_NATIVE_INT,
				    time_spaceid, cparmsid)) < 0) ERR;
      if (H5DSset_scale(time_scaleid, TIME_NAME) < 0) ERR;
      if ((smelliness_scaleid = H5Dcreate(adam_grpid, SMELLINESS_NAME, H5T_NATIVE_FLOAT,
					  smelliness_spaceid, cparmsid)) < 0) ERR;
      if (H5DSset_scale(smelliness_scaleid, SMELLINESS_NAME) < 0) ERR;
      if ((distance_scaleid = H5Dcreate(adam_grpid, DISTANCE_NAME, H5T_NATIVE_FLOAT,
					distance_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(distance_scaleid, DISTANCE_NAME) < 0) ERR;

      /* Create a space coresponding to these three dimensions. */
      dims[TIME_DIM] = 0;
      dims[SMELLINESS_DIM] = 0;
      dims[DISTANCE_DIM] = DISTANCE_LEN;
      max_dims[TIME_DIM] = H5S_UNLIMITED;
      max_dims[SMELLINESS_DIM] = H5S_UNLIMITED;
      max_dims[DISTANCE_DIM] = DISTANCE_LEN;
      if ((spaceid = H5Screate_simple(NDIMS, dims, max_dims)) < 0) ERR;

      /* Set up chunking for our 3D vars. */
      dims[TIME_DIM] = 1;
      dims[SMELLINESS_DIM] = 1;
      if (H5Pset_chunk(cparmsid, NDIMS, dims) < 0) ERR;

      /* Create two variables which use them, and attach the dimension scales. */
      if ((goat_dsid = H5Dcreate(able_grpid, GOAT_NAME, H5T_NATIVE_FLOAT,
				 spaceid, cparmsid)) < 0) ERR;
      if (H5DSattach_scale(goat_dsid, time_scaleid, 0) < 0) ERR;
      if (H5DSattach_scale(goat_dsid, smelliness_scaleid, 1) < 0) ERR;
      if (H5DSattach_scale(goat_dsid, distance_scaleid, 2) < 0) ERR;
      if ((camel_dsid = H5Dcreate(cain_grpid, CAMEL_NAME, H5T_NATIVE_FLOAT,
				  spaceid, cparmsid)) < 0) ERR;
      if (H5DSattach_scale(camel_dsid, time_scaleid, 0) < 0) ERR;
      if (H5DSattach_scale(camel_dsid, smelliness_scaleid, 1) < 0) ERR;
      if (H5DSattach_scale(camel_dsid, distance_scaleid, 2) < 0) ERR;

      /* Close up the shop. */
      if (H5Dclose(goat_dsid) < 0 ||
	  H5Dclose(camel_dsid) < 0 ||
	  H5Dclose(smelliness_scaleid) < 0 ||
	  H5Dclose(distance_scaleid) < 0 ||
	  H5Dclose(time_scaleid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(cain_grpid) < 0 ||
	  H5Gclose(able_grpid) < 0 ||
	  H5Gclose(adam_grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking 3D datasets in groups created with shared dimscales...");

   {
      hid_t fileid, grpid;

      /* Reopen the file and group. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, FATHER)) < 0) ERR;

      /* If we can't scan the group, crash into a flaming heap of
       * smoking, smoldering rubbish. */
      if (rec_scan_group(grpid)) ERR;
      
      /* Close up the shop. */
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
#endif
   FINAL_RESULTS;
}

