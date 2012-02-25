/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program does HDF5 string stuff.

   Here's a HDF5 sample programs:
   http://hdf.ncsa.uiuc.edu/training/other-ex5/sample-programs/strings.c
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_strings.h5"
#define DIM1_LEN 3
#define ATT_NAME "Stooge_Statements"
#define GRP_NAME "Stooge_Antic_Metrics"

int
main()
{
   printf("\n*** Checking HDF5 string types.\n");
   printf("*** Checking scalar string attribute...");
   {
      hid_t fileid, grpid, spaceid, typeid, attid;
      hid_t class;
      size_t type_size;
      htri_t is_str;
      char *data_in;
      char *data = "The art of war is of vital "
	 "importance to the State. It is a matter of life and death, a road either"
	 "to safety or to ruin.  Hence it is a subject of inquiry"
	 "which can on no account be neglected.";

      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create string type. */
      if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;
      
      /* Write an attribute of this type. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, &data) < 0) ERR;

      /* Close up. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      
      /* Now reopen the file and check it out. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      
      /* Given this type id, how would we know this is a string
       * attribute? */
      if ((class = H5Tget_class(typeid)) < 0) ERR;
      if (class != H5T_STRING) ERR;
      if (!(type_size = H5Tget_size(typeid))) ERR;
      if ((is_str = H5Tis_variable_str(typeid)) < 0) ERR;

      /* Make sure this is a scalar. */
      if (H5Sget_simple_extent_type(spaceid) != H5S_SCALAR) ERR;
      
      /* Read the attribute. */
      if (H5Aread(attid, typeid, &data_in) < 0) ERR;

      /* Check the data. */
      if (strcmp(data, data_in)) ERR;

      /* Free our memory. */
      free(data_in);

      /* Close HDF5 stuff. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** Checking simple HDF5 string types...");
   {   
      hid_t fileid, grpid, spaceid, typeid, attid;
      hsize_t dims[1] = {DIM1_LEN};
/*      size_t type_size;
	htri_t is_str;*/
      hid_t class;
      char *data[DIM1_LEN] = {"Ohhh!", "Ahhh!", "Wub-wub-wub!"};
      char **data_in;
      int i;

      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;

      /* Create string type. */
      if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;
   
      /* I thought the following should work instead of the H5Tcopy and
       * H5Tset_size functions above, but it doesn't. */
      /*if ((typeid = H5Tvlen_create(H5T_NATIVE_CHAR)) < 0) ERR;*/

      /* Write an attribute of this (string) type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;

      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* Now reopen the file and check it out. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;

      /* Given this type id, how would we know this is a string
       * attribute? */
      if ((class = H5Tget_class(typeid)) < 0) ERR;
      if (class != H5T_STRING) ERR;
/*      if (!(type_size = H5Tget_size(typeid))) ERR;
	if ((is_str = H5Tis_variable_str(typeid)) < 0) ERR;*/

      /* How many strings are in the string array? */
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      if (H5Sget_simple_extent_dims(spaceid, dims, NULL) < 0) ERR;
      if (!(data_in = malloc(dims[0] * sizeof(char *)))) ERR;

      /* Now read the array of strings. The HDF5 library will allocate
       * space for each string. */
      if (H5Aread(attid, typeid, data_in) < 0) ERR;

      /* Compare the values to make sure it worked... */
      for (i = 0; i < DIM1_LEN; i++)
	 if (strcmp(data[i], data_in[i])) ERR;

      /* Free the memory that HDF5 allocated. */
      for (i = 0; i < DIM1_LEN; i++)
	 free(data_in[i]);

      /* Free the memory that we allocated. */
      free(data_in);

      /* Close everything up. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking G&S compliance...");
   {
#define S1 "When Frederick was a little lad"
#define S2 "He proved so brave and daring..."

      char **data;
      char **data_in;
      hsize_t dims_in[1], dims[1] = {2};
      hid_t fileid, grpid, spaceid, typeid, attid;
      hid_t class;
      int i;

      /* Allocate space and copy strings. */
      data = malloc(sizeof(char *) * 2);
      data[0] = malloc(strlen(S1) + 1);
      strcpy(data[0], S1);
      data[1] = malloc(strlen(S2) + 1);
      strcpy(data[1], S2);

      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			   H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create string type. */
      if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;
      
      /* I thought the following should work instead of the H5Tcopy and
       * H5Tset_size functions above, but it doesn't. */
      /*if ((typeid = H5Tvlen_create(H5T_NATIVE_CHAR)) < 0) ERR;*/
      
      /* Write an attribute of this type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;

      /* Close up. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      
      /* Now reopen the file and check it out. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      
      /* Given this type id, how would we know this is a string
       * attribute? */
      if ((class = H5Tget_class(typeid)) < 0) ERR;
      if (class != H5T_STRING) ERR;

      /* How many strings are in the array? */
      if (H5Sget_simple_extent_dims(spaceid, dims_in, NULL) != 1) ERR;
      if (dims_in[0] != dims[0]) ERR;
      
      /* Allocate enough pointers to read the data. The HDF5 library
       * will allocate the space needed for each string. */
      if (!(data_in = malloc(dims_in[0] * sizeof(char *)))) ERR;

      /* Read the data. */
      if (H5Aread(attid, typeid, data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < 2; i++)
	 if (strcmp(data[i], data_in[i])) ERR;

      /* Free our memory. */
      for (i = 0; i < 2; i++)
	 free(data_in[i]);
      free(data_in);
      free(data[0]);
      free(data[1]);
      free(data);

      /* Close HDF5 stuff. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking empty strings...");
   {
      char **data;
      char **data_in;
      hsize_t dims_in[1], dims[1] = {2};
      hid_t fileid, grpid, spaceid, typeid, attid;
      hid_t class;
      int i;

      /* Allocate space and copy strings. */
      data = malloc(sizeof(char *) * 2);
      data[0] = malloc(strlen(S1) + 1);
      strcpy(data[0], "");
      data[1] = malloc(strlen(S2) + 1);
      strcpy(data[1], "");

      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			   H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create string type. */
      if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;
      
      /* I thought the following should work instead of the H5Tcopy and
       * H5Tset_size functions above, but it doesn't. */
      /*if ((typeid = H5Tvlen_create(H5T_NATIVE_CHAR)) < 0) ERR;*/
      
      /* Write an attribute of this type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;

      /* Close up. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
      
      /* Now reopen the file and check it out. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      
      /* Given this type id, how would we know this is a string
       * attribute? */
      if ((class = H5Tget_class(typeid)) < 0) ERR;
      if (class != H5T_STRING) ERR;

      /* How many strings are in the array? */
      if (H5Sget_simple_extent_dims(spaceid, dims_in, NULL) != 1) ERR;
      if (dims_in[0] != dims[0]) ERR;
      
      /* Allocate enough pointers to read the data. The HDF5 library
       * will allocate the space needed for each string. */
      if (!(data_in = malloc(dims_in[0] * sizeof(char *)))) ERR;

      /* Read the data. */
      if (H5Aread(attid, typeid, data_in) < 0) ERR;

      /* Check the data. */
      for (i = 0; i < 2; i++)
	 if (strcmp(data[i], data_in[i])) ERR;

      /* Free our memory. */
      for (i = 0; i < 2; i++)
	 free(data_in[i]);
      free(data_in);
      free(data[0]);
      free(data[1]);
      free(data);

      /* Close HDF5 stuff. */
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
