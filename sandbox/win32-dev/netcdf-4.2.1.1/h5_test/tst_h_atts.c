/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This file deals with HDF5 attributes.

   $Id$
*/
#include <nc_tests.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_atts.h5"
#define GRP_NAME "Hamlet"
#define ATT1_NAME "Hamlets_Self_Evaluation"
#define ATT2_NAME "Commentary"
#define MAX_LEN 50
#define NUM_EMPS 6
#define EMP_GRP "Emperors"
#define NC3_STRICT_ATT_NAME "_nc3_strict"

char txt[] = "O, what a rogue and peasant slave am I!\n"
"Is it not monstrous that this player here,\n"
"But in a fiction, in a dream of passion,\n"
"Could force his soul so to his own conceit\n"
"That from her working all his visage wann'd,\n"
"Tears in his eyes, distraction in's aspect,\n"
"A broken voice, and his whole function suiting\n"
"With forms to his conceit? and all for nothing!\n"
"For Hecuba!\n"
"What's Hecuba to him, or he to Hecuba,\n"
"That he should weep for her?";

int
main()
{
   hid_t fileid, grpid, spaceid, typeid, attid;
   char *txt_in;
   size_t txt_size;
   hssize_t size;
   size_t type_size;
   int ndims;
   hsize_t num_obj;

   printf("\n*** Checking HDF5 attribute functions.\n");
   printf("*** Checking HDF5 attribute ordering...");

   {
      float val = 99;
      char emp[NUM_EMPS][MAX_LEN + 1] = {"Augustus", "Tiberius", 
				     "Caligula", "Claudius", 
				     "Ne_r_o", "V.esp.asi.an"};
      char obj_name[MAX_LEN + 1];
      int e, i;

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, EMP_GRP, 0)) < 0) ERR;

      /* Create space fo zero-length attributes. */
      if ((spaceid = H5Screate(H5S_NULL)) < 0) ERR;

      /* Attach some zero-length float attributes! */
      for (e = 0; e < NUM_EMPS; e++)
      {
	 if ((attid = H5Acreate(grpid, emp[e], H5T_NATIVE_FLOAT, spaceid, 
				H5P_DEFAULT)) < 0) ERR;
	 if (H5Awrite(attid, H5T_NATIVE_FLOAT, &val) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
      }

      /* Close everything. */
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      
      /* Now open the file again and read in the attributes. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, EMP_GRP)) < 0) ERR;

      /* How many attributes are there? */
      if ((num_obj = H5Aget_num_attrs(grpid)) != NUM_EMPS) ERR;
      
      /* Make sure the names are in the correct order. */
      for (i = 0; i < num_obj; i++)
      {
	 if ((attid = H5Aopen_idx(grpid, (unsigned int)i)) < 0) ERR;
	 if (H5Aget_name(attid, MAX_LEN + 1, obj_name) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
	 if (strcmp(obj_name, emp[i])) ERR;
      }
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 attribute deletes...");
   {
      /* Create a file and open the root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Attach a text attribute with some of Hamlet's lines. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, strlen(txt) + 1) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT1_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, txt) < 0) ERR;

      /* Delete the attribute. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Adelete(grpid, ATT1_NAME) < 0) ERR;

      /* Create and write it again. */
      if ((attid = H5Acreate(grpid, ATT1_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, txt) < 0) ERR;

      /* Close everything. */
      if (H5Aclose(attid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 attributes attached to the fileid...");
   {
      /* See if we can write an attribute to the root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Attach a text attribute with some of Hamlet's lines. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, strlen(txt) + 1) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT1_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, txt) < 0) ERR;
      if (H5Aclose(attid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 attributes in a group...");
   {
      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
   
      /* Attach a text attribute with some of Hamlet's lines. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, strlen(txt) + 1) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT1_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, txt) < 0) ERR;
      if (H5Aclose(attid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Now open the file again and read in the attribute. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT1_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;

      /* Check the size of the string in the attribute. */
      if (H5Tget_class(typeid) != H5T_STRING) ERR;
      if (!(txt_size = H5Tget_size(typeid))) ERR;
      if (txt_size != strlen(txt) + 1) ERR;

      /* Now read the attribute. But if I don't malloc the memory first,
       * I get zapped with a seg-fault. Aren't strings supposed to be
       * different? */
      if (!(txt_in = malloc(txt_size+1))) ERR;
      if (H5Aread(attid, typeid, txt_in) < 0) ERR;
      if (strcmp(txt_in, txt)) ERR;
      if (strlen(txt_in) != strlen(txt)) ERR;

      /* For a scalar, ndims is 0 but simple_extent_npoints is 1. For a
       * NULL dataspace (see below), they are both 0. */
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      if ((size = H5Sget_simple_extent_npoints(spaceid)) < 0) ERR;
      if (size != 1) ERR;
      if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;
      if (ndims != 0) ERR;
      if (!(type_size = H5Tget_size(typeid))) ERR;
      if (type_size != strlen(txt) + 1) ERR;
      free(txt_in);

      if (H5Aclose(attid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 zero length attributes...");

   {
      hid_t attid1;
      float val = 99;

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0)
	 ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0)
	 ERR;
      
      /* Attach a float attribute with no data. The
       * hell with Hamlet anyway! */
      if ((spaceid = H5Screate(H5S_NULL)) < 0)
	 ERR;
      if ((typeid = H5Tcopy(H5T_NATIVE_FLOAT)) < 0)
	 ERR;
      if ((attid = H5Acreate(grpid, ATT1_NAME, 
			     typeid, spaceid, H5P_DEFAULT)) < 0)
	 ERR;
      if (H5Awrite(attid, H5T_NATIVE_FLOAT, &val) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      
      /* Now open the file again and read in the attribute. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT1_NAME)) < 0) ERR;
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      if ((size = H5Sget_simple_extent_npoints(spaceid)) < 0) ERR;
      if (size != 0) ERR;
      if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;
      if (ndims != 0) ERR;

      /* Attach a text attribute with no data. */
      if ((spaceid = H5Screate(H5S_NULL)) < 0) ERR;
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, 1) < 0) ERR;
      if ((attid1 = H5Acreate(grpid, ATT2_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_FLOAT, &val) < 0) ERR;
      if (H5Sclose(spaceid) < 0 ||
	  H5Aclose(attid) < 0 ||
	  H5Aclose(attid1) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
      
      /* Now open the file again and read in the attribute. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT2_NAME)) < 0) ERR;
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      if ((size = H5Sget_simple_extent_npoints(spaceid)) < 0) ERR;
      if (size != 0) ERR;
      if ((ndims = H5Sget_simple_extent_ndims(spaceid)) < 0) ERR;
      if (ndims != 0) ERR;
      if (H5Sclose(spaceid) < 0 ||
	  H5Aclose(attid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking how HDF5 handles a delete and recreation of dataset with atts...");

#define DIM1_LEN 3
#define VAR_NAME "Hamlet"
#define NUM_SPEECHES 3
#define MAX_SPEECH_LEN 4095
   {
      /* Why test this out? Because this happens when the netcdf-4 user
       * assigns a fill value attribute to a var. Since HDF5 demands that
       * the fill value be supplied on var creation, I have to delete the
       * dataset and recreate it, readding the attributes that existed
       * when I delete it. */
      hid_t var_spaceid = 0, datasetid = 0;
      hsize_t dims[1];
      char speech[NUM_SPEECHES][MAX_SPEECH_LEN + 1] = {
	 "A little more than kin, and less than kind!",

	 "Speak the speech, I pray you, as I pronounc'd it to you, trippingly on\n"
	 "the tongue. But if you mouth it, as many of our players do, I had as\n"
	 "live the town crier spoke my lines. Nor do not saw the air too much\n"
	 "with your hand, thus, but use all gently; for in the very torrent,\n"
	 "tempest, and (as I may say) whirlwind of your passion, you must\n"
	 "acquire and beget a temperance that may give it smoothness. O, it\n"
	 "offends me to the soul to hear a robustious periwig-pated fellow tear\n"
	 "a passion to tatters, to very rags, to split the cars of the\n"
	 "groundlings, who (for the most part) are capable of nothing but\n"
	 "inexplicable dumb shows and noise. I would have such a fellow whipp'd\n"
	 "for o'erdoing Termagant. It out-herods Herod.  Pray you avoid it.",

	 "O, I die, Horatio!\n"
	 "The potent poison quite o'ercrows my spirit.\n"
	 "I cannot live to hear the news from England,\n"
	 "But I do prophesy th' election lights\n"
	 "On Fortinbras. He has my dying voice.\n"
	 "So tell him, with th' occurrents, more and less,\n"
	 "Which have solicited- the rest is silence."};

      char speech_name[NUM_SPEECHES][MAX_LEN + 1] = {"Act_1_Scene_2", 
						     "Act_3_Scene_2", 
						     "Act_5_Scene_2"};
      char obj_name[MAX_LEN + 1];
      int i;

      /* Create a file and get its root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Create a dataset. */
      dims[0] = DIM1_LEN;
      if ((var_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, VAR_NAME, H5T_NATIVE_HBOOL, 
				 var_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Attach three text attributes with some of Hamlet's lines to
       * the dataset. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;

      for (i = 0; i < NUM_SPEECHES; i++)
      {
	 if (H5Tset_size(typeid, strlen(speech[i]) + 1) < 0) ERR;
	 if ((attid = H5Acreate(datasetid, speech_name[i], typeid, spaceid, 
				H5P_DEFAULT)) < 0) ERR;
	 if (H5Awrite(attid, typeid, speech[i]) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
      }

      if (H5Dclose(datasetid) < 0 ||
	  H5Sclose(var_spaceid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Now open the file and delete the dataset (and all its
       * attributes). */

      /* Open file, group, and dataset. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, VAR_NAME)) < 0) ERR;

      /* How many attributes are there? */
      if ((num_obj = H5Aget_num_attrs(datasetid)) != NUM_SPEECHES) ERR;
      
      /* Make sure the names are in the correct order. */
      for (i = 0; i < num_obj; i++)
      {
	 if ((attid = H5Aopen_idx(datasetid, (unsigned int)i)) < 0) ERR;
	 if (H5Aget_name(attid, MAX_LEN + 1, obj_name) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
	 if (strcmp(obj_name, speech_name[i])) ERR;
      }

      /* Delete the HDF5 dataset. */
      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Gunlink(grpid, VAR_NAME) < 0) ERR;

      /* Recreate it and add the attributes again. */
      if ((var_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, VAR_NAME, H5T_NATIVE_HBOOL, 
				 var_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Attach three text attributes with some of Hamlet's lines to
       * the dataset. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;

      for (i = 0; i < NUM_SPEECHES; i++)
      {
	 if (H5Tset_size(typeid, strlen(speech[i]) + 1) < 0) ERR;
	 if ((attid = H5Acreate(datasetid, speech_name[i], typeid, spaceid, 
				H5P_DEFAULT)) < 0) ERR;
	 if (H5Awrite(attid, typeid, speech[i]) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
      }

      /* How many attributes are there? */
      if ((num_obj = H5Aget_num_attrs(datasetid)) != NUM_SPEECHES) ERR;
      
      /* Make sure the names are in the correct order. */
      for (i = 0; i < num_obj; i++)
      {
	 if ((attid = H5Aopen_idx(datasetid, (unsigned int)i)) < 0) ERR;
	 if (H5Aget_name(attid, MAX_LEN + 1, obj_name) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
	 if (strcmp(obj_name, speech_name[i])) ERR;
      }

      if (H5Dclose(datasetid) < 0 ||
	  H5Sclose(var_spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

   }

   SUMMARIZE_ERR;
   printf("*** Checking some more simple atts...");
   {
      hid_t fcpl_id, fapl_id, hdfid, grpid;
      hid_t spaceid, attid, attid1;
      int one = 1;
      hsize_t dims[1];

      /* Create a HDF5 file. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
/*      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
	H5P_CRT_ORDER_INDEXED)) < 0) ERR;*/
      if ((hdfid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(hdfid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Write an attribute. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((attid = H5Acreate2(grpid, NC3_STRICT_ATT_NAME, H5T_NATIVE_INT, 
			      spaceid, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;

      H5Fflush(hdfid, H5F_SCOPE_GLOBAL);
      dims[0] = 1;
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid1 = H5Acreate2(grpid, "l", H5T_NATIVE_INT, spaceid, 
			       H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Aclose(attid1) < 0) ERR;
      if ((attid1 = H5Acreate2(grpid, "y", H5T_NATIVE_INT, spaceid, 
			       H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Aclose(attid1) < 0) ERR;
      if ((attid1 = H5Acreate2(grpid, "c", H5T_NATIVE_INT, spaceid, 
			       H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Aclose(attid1) < 0) ERR;

      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(hdfid) < 0) ERR;

      if ((hdfid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen2(hdfid, "/", H5P_DEFAULT)) < 0)
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(hdfid) < 0) ERR;
   }
   SUMMARIZE_ERR;

   printf("*** Checking some more simple atts even more...");
   {
      hid_t fcpl_id, fapl_id, hdfid, grpid;
      hid_t spaceid, attid, attid1;
      int one = 1;
      hsize_t dims[1];

      /* Create a HDF5 file. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_STRONG)) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
/*      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
	H5P_CRT_ORDER_INDEXED)) < 0) ERR;*/
      if ((hdfid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(hdfid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Write an attribute. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((attid = H5Acreate(grpid, NC3_STRICT_ATT_NAME, H5T_NATIVE_INT, 
			      spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;

      H5Fflush(hdfid, H5F_SCOPE_GLOBAL);
      dims[0] = 1;
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid1 = H5Acreate2(grpid, "z", H5T_NATIVE_INT, spaceid, 
			       H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Aclose(attid1) < 0) ERR;
      if ((attid1 = H5Acreate2(grpid, "y", H5T_NATIVE_INT, spaceid, 
			       H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Aclose(attid1) < 0) ERR;
      if ((attid1 = H5Acreate2(grpid, "c", H5T_NATIVE_INT, spaceid, 
			       H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
      if (H5Aclose(attid1) < 0) ERR;

      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(hdfid) < 0) ERR;

      if ((hdfid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen2(hdfid, "/", H5P_DEFAULT)) < 0)
      if (H5Pclose(fapl_id) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(hdfid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking HDF5 attribute ordering some more...");
   {
#define DIM2_LEN 2
      hid_t fileid, grpid, attid, spaceid, dimscaleid, att_spaceid;
      hid_t fcpl_id, fapl_id;
      hsize_t num_obj, dims[1];
      char obj_name[MAX_LEN + 1];
      char att_name[3][20] = {"first", "second", "third"};
      signed char b[DIM2_LEN] = {-127, 126};
      int i;

      /* Create a file and get its root group. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Write two group-level attributes containing byte attays of
       * length 2, call them "first" and "second". */
      dims[0] = DIM2_LEN;
      if ((att_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((attid = H5Acreate(grpid, att_name[0], H5T_NATIVE_UCHAR,
			     att_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, H5T_NATIVE_UCHAR, b) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;
      if ((attid = H5Acreate(grpid , att_name[1], H5T_NATIVE_UCHAR,
			     att_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, H5T_NATIVE_UCHAR, b) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;

      /* Create a dataset which will be a HDF5 dimension scale. */
      dims[0] = 1;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((dimscaleid = H5Dcreate(grpid, "D1", H5T_IEEE_F32BE,
				  spaceid, H5P_DEFAULT)) < 0)
	 ERR;
      
      /* Indicate that this is a scale. */
      if (H5DSset_scale(dimscaleid, NULL) < 0) ERR;
      
      /* Add another attribute to the group. Call it "third". */
      if ((attid = H5Acreate(grpid , att_name[2], H5T_NATIVE_UCHAR,
			     att_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, H5T_NATIVE_UCHAR, b) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;

      if (H5Dclose(dimscaleid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Sclose(att_spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Now open the file and check. Magically, the attributes "second"
       * and "first" will be out of order. This can be checked with
       * h5dump, or with the following code, which will fail if it does
       * not find three attributes, in order "first", "second", and
       * "third". */

      /* Open file, group. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR,
			    fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* How many attributes are there? */
      if ((num_obj = H5Aget_num_attrs(grpid)) != 3) ERR;
      
      /* Make sure the names are in the correct order. */
      for (i = 0; i < num_obj; i++)
      {
	 if ((attid = H5Aopen_idx(grpid, (unsigned int)i)) < 0) ERR;
	 if (H5Aget_name(attid, MAX_LEN + 1, obj_name) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
	 if (strcmp(obj_name, att_name[i])) ERR;
      }

      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking HDF5 attribute ordering with 9 attributes...(skipping for HDF5 1.8.0 beta1)");
#define NUM_SIMPLE_ATTS 9
#define ATT_MAX_NAME 2
   {
      hid_t fileid, grpid, attid, att_spaceid;
      hsize_t num_obj;
      char obj_name[MAX_LEN + 1];
      char name[NUM_SIMPLE_ATTS][ATT_MAX_NAME + 1] = {"Gc", "Gb", "Gs", "Gi", "Gf",
						      "Gd", "G7", "G8", "G9"};
      hid_t fcpl_id, fapl_id;
      int i;

      /* Set up property lists to turn on creation ordering. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
     if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;


      /* Create a file and get its root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* These will all be zero-length atts. */
      if ((att_spaceid = H5Screate(H5S_NULL)) < 0) ERR;

      for (i = 0; i < NUM_SIMPLE_ATTS; i++)
      {
	 if ((attid = H5Acreate(grpid, name[i], H5T_NATIVE_INT,
				att_spaceid, H5P_DEFAULT)) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
      }

      if (H5Sclose(att_spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

      /* Now open the file and check. */

      /* Open file, group. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
     if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR,
			    fapl_id)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* How many attributes are there? */
      if ((num_obj = H5Aget_num_attrs(grpid)) != NUM_SIMPLE_ATTS) ERR;
      
      /* Make sure the names are in the correct order. */
      for (i = 0; i < num_obj; i++)
      {
	 if ((attid = H5Aopen_idx(grpid, (unsigned int)i)) < 0) ERR;
	 if (H5Aget_name(attid, MAX_LEN + 1, obj_name) < 0) ERR;
	 if (H5Aclose(attid) < 0) ERR;
	 if (strcmp(obj_name, name[i])) ERR;
      }

      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
