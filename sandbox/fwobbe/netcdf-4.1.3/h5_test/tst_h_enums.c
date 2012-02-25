/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program excersizes HDF5 variable length array code.

   $Id$
*/
#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_enums.h5"
#define DIM1_LEN 12
#define ATT_NAME "Browings_reverse_enumerations"
#define SIZE 9
#define GRP_NAME "Browning"
#define NUM_VALS 12
#define STR_LEN 255

/* This seems like a good sonnet for enumation:

How do I love thee? Let me count the ways.
I love thee to the depth and breadth and height
My soul can reach, when feeling out of sight
For the ends of Being and ideal Grace.
I love thee to the level of everyday's
Most quiet need, by sun and candle-light.
I love thee freely, as men strive for Right;
I love thee purely, as they turn from Praise.
I love thee with a passion put to use
In my old griefs, and with my childhood's faith.
I love thee with a love I seemed to lose
With my lost saints, --- I love thee with the breath,
Smiles, tears, of all my life! --- and, if God choose,
I shall but love thee better after death.

(I gotta say, that's one dorky poem. - Ed)
*/

int
main()
{
   printf("\n*** Checking HDF5 enum types.\n");
   printf("*** Checking simple HDF5 enum type...");
   {
      hid_t fileid, grpid, spaceid, typeid, attid;
      hsize_t dims[1] = {DIM1_LEN};
      short data[DIM1_LEN];
      short data_in[DIM1_LEN];
      int i;
      short val[NUM_VALS];
      char love_how[NUM_VALS][STR_LEN + 1] = {"Depth", "Bredth", 
						  "Height", "Level", 
						  "Freely", "Purely", 
						  "Passionately", "Lost", 
						  "Breath", "Smiles", 
						  "Tears", "After Death"};
/*      H5T_class_t type_class;*/
      size_t size;
      int num_members;
      short the_value;
      char *member_name;
      htri_t types_equal;
      hid_t base_hdf_typeid;
	 
      
      for (i=0; i < NUM_VALS; i++)
	 val[i] = i*2;
      for (i=0; i < DIM1_LEN; i++)
	 data[i] = i*2;
      
      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create enum type. */
      /* Both methods do the same thing, but Quincey says to prefer
       * H5Tcreate_enum. */
      /*if ((typeid =  H5Tcreate(H5T_ENUM, sizeof(short))) < 0) ERR;*/
      if ((typeid =  H5Tenum_create(H5T_NATIVE_SHORT)) < 0) ERR;

      /* Insert some values. */
      for (i=0; i<NUM_VALS; i++)
	 if (H5Tenum_insert(typeid, love_how[i], &val[i]) < 0) ERR;
      
      /* Write an attribute of this type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid,
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;

      /* Close everything. */
      if (H5Aclose(attid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR; 

      /* Reopen the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;

      /* Check the attribute's type. */
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
/*      if ((type_class = H5Tget_class(typeid)) != H5T_ENUM) ERR;*/
      num_members = H5Tget_nmembers(typeid);
      if (num_members != NUM_VALS) ERR;
      size = H5Tget_size(typeid);
      if (size != 2) ERR;
      if ((base_hdf_typeid = H5Tget_super(typeid)) < 0) ERR;
      if ((types_equal = H5Tequal(base_hdf_typeid, H5T_NATIVE_SHORT)) < 0) ERR;
      if (!types_equal) ERR;
      
      /* Check each value and number in the enum. */
      for (i=0; i < NUM_VALS; i++)
      {
	 if (H5Tget_member_value(typeid, i, &the_value) < 0) ERR;
	 if (the_value != val[i]) ERR;
	 member_name = H5Tget_member_name(typeid, i);
	 if (strcmp(member_name, love_how[i])) ERR;
	 free(member_name);
      }

      /* Now read the data in the attribute and make sure it's what we
       * expect it to be. */
      if (H5Aread(attid, typeid, data_in) < 0) ERR;
      for (i=0; i < DIM1_LEN; i++)
	 if (data_in[i] != data[i]) ERR;

      /* Close it all again. */
      if (H5Aclose(attid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR; 
   }
   SUMMARIZE_ERR;

   printf("*** Checking HDF5 enum type missing values...");
   {
#define NUM_LANG 4      
#define VAR_LANG_NAME "Programming_Language"
#define FV_NAME "_FillValue"
#define GRP_NAME2 "NetCDF_Programming"

      hid_t datasetid, mem_spaceid, fileid, grpid, spaceid, typeid, plistid;
      hid_t file_spaceid, attid, att_spaceid;
      hsize_t dims[1] = {DIM1_LEN};
      short data_in[DIM1_LEN];
      int i;
      short val[NUM_LANG];
      char lang[NUM_LANG][STR_LEN + 1] = {"C", "Fortran", "C++", "MISSING"};
      enum langs {CLANG=0, Fortran=1, CPP=2, MISSING=255};
      short the_value, fill_value = MISSING, data_point = CLANG;
      hsize_t start[1] = {1}, count[1] = {1};
      int num_members;
      size_t size;
      hid_t base_hdf_typeid;
/*      H5T_class_t type_class;*/
      char *member_name;
      htri_t types_equal;

      val[0] = CLANG;
      val[1] = Fortran;
      val[2] = CPP;
      val[3] = MISSING;
	 
      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME2, 0)) < 0) ERR;
      
      /* Create enum type. */
      if ((typeid =  H5Tenum_create(H5T_NATIVE_SHORT)) < 0) ERR;

      /* Insert some values. */
      for (i=0; i<NUM_LANG; i++)
	 if (H5Tenum_insert(typeid, lang[i], &val[i]) < 0) ERR;
      
      /* Create a dataset of this enum type, with fill value. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_fill_value(plistid, typeid, &fill_value) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, VAR_LANG_NAME, typeid, 
				 spaceid, plistid)) < 0) ERR;

      /* Create a netCDFstyle _FillValue attribute, though it will be
       * ignored by HDF5. */
      if ((att_spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((attid = H5Acreate(grpid, FV_NAME, typeid, att_spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, &fill_value) < 0) ERR;

      /* Write one value, the rest will end up set to MISSING. */
      if ((mem_spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((file_spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if (H5Sselect_hyperslab(file_spaceid, H5S_SELECT_SET, 
			      start, NULL, count, NULL) < 0) ERR;
      if (H5Dwrite(datasetid, typeid, mem_spaceid, file_spaceid, 
		   H5P_DEFAULT, &data_point) < 0) ERR;

      /* Close everything. */
      if (H5Sclose(spaceid) < 0 ||
	  H5Sclose(mem_spaceid) < 0 ||
	  H5Aclose(attid) < 0 ||
	  H5Sclose(file_spaceid) < 0 ||
	  H5Dclose(datasetid) < 0 ||
	  H5Pclose(plistid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR; 

      /* Reopen the file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME2)) < 0) ERR;

      /* Check the variable's type. */
      if ((datasetid = H5Dopen1(grpid, VAR_LANG_NAME)) < 0) ERR;
      if ((typeid = H5Dget_type(datasetid)) < 0) ERR;
/*      if ((type_class = H5Tget_class(typeid)) != H5T_ENUM) ERR;*/
      num_members = H5Tget_nmembers(typeid);
      if (num_members != NUM_LANG) ERR;
      size = H5Tget_size(typeid);
      if (size != sizeof(short)) ERR;
      if ((base_hdf_typeid = H5Tget_super(typeid)) < 0) ERR;
      if ((types_equal = H5Tequal(base_hdf_typeid, H5T_NATIVE_SHORT)) < 0) ERR;
      if (!types_equal) ERR;
      
      /* Check each value and number in the enum. */
      for (i=0; i < NUM_LANG; i++)
      {
	 if (H5Tget_member_value(typeid, i, &the_value) < 0) ERR;
	 if (the_value != val[i]) ERR;
	 member_name = H5Tget_member_name(typeid, i);
	 if (strcmp(member_name, lang[i])) ERR;
	 free(member_name);
      }

      /* Now read the data in the dataset and make sure it's what we
       * expect it to be. */
      if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_in) < 0) ERR;
      for (i=0; i < DIM1_LEN; i++)
	 if (i == 1)
	 {
	    if (data_in[i] != data_point) ERR;
	 }
	 else
	    if (data_in[i] != MISSING) ERR;

      /* Close it all again. */
      if (H5Dclose(datasetid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
/*    printf("*** Checking HDF5 enum interuptus..."); */
/*    { */
/* #define GRP_NAME3 "STOP!" */

/*       hid_t fileid, grpid,  typeid; */
      
/*       /\* Open file. *\/ */
/*       if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,  */
/* 			      H5P_DEFAULT)) < 0) ERR; */
/*       if ((grpid = H5Gcreate(fileid, GRP_NAME3, 0)) < 0) ERR; */
      
/*       /\* Create enum type. *\/ */
/*       if ((typeid =  H5Tenum_create(H5T_NATIVE_SHORT)) < 0) ERR; */

/*       /\* Commit the type. This doesn't work, because no members were */
/*        * specified. *\/ */
/*       if (!H5Tcommit(grpid, "name", typeid) < 0) ERR; */

/*       /\* Close everything. *\/ */
/*       if (H5Tclose(typeid) < 0 || */
/* 	  H5Gclose(grpid) < 0 || */
/* 	  H5Fclose(fileid) < 0) ERR;  */

/*    } */
/*    SUMMARIZE_ERR; */

   FINAL_RESULTS;
}
