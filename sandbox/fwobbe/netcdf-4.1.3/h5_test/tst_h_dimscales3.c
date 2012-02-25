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
#include <hdf5.h>
#include <ncdimscale.h>

#define FILE_NAME "tst_h_dimscales3.h5"
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
   printf("\n*** Checking HDF5 phony, secret, and underhanded dimscales. Shhh! Don't tell anyone!\n");
   printf("*** Creating a phony dimscale...");
   
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
		  if ((datasetid = H5Dopen(fileid, obj_name)) < 0) ERR;

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
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

