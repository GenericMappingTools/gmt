/* This is part of the netCDF package.  Copyright 2010 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test that HDF5 and NetCDF-4 can read and write the same file.

   $Id$
*/
#include <config.h>
#include <nc_tests.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_interops6.h5"

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF5/NetCDF-4 interoperability yet again...\n");
   printf("*** Checking scalar string attribute...");
   {
#define ATT_NAME "Stooge_Statements"
      hid_t fcpl_id, fileid, grpid, spaceid, typeid, attid;
      hid_t class;
      size_t type_size;
      htri_t is_str;
      char *data_in;
      char *data = "The art of war is of vital "
	 "importance to the State. It is a matter of life and death, a road either"
	 "to safety or to ruin.  Hence it is a subject of inquiry"
	 "which can on no account be neglected.";

      /* Create create property list. */
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      
      /* Set H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering in the file. */
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      
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
      if (H5Pclose(fcpl_id) < 0) ERR;
      
      /* Now reopen the file and check it out. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      
      /* Given this type id, how would we know this is a string
       * attribute? */
      if ((class = H5Tget_class(typeid)) < 0)
	 return NC_EHDFERR;
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
   printf("*** Checking a HDF5 file with scalar, fixed-length string dataset...");
   {
#define VAR_NAME "Marcus_Aurelius"
      hid_t fcpl_id, fileid, grpid, spaceid, typeid, datasetid, plistid;
      int ncid, nvars_in, ndims_in, natts_in, unlimdim_in, type_in;
      size_t size_in;
      char *data = "Thou art no dissatisfied, I suppose, because "
	 "thou weighest only so many liters and not three hundred. Be not "
	 "dissatisfied then that thou must live only so many years and not more; "
	 "for as thou art satisfied with the amount of substance which has "
	 "been assigned to thee, so be content with the time.";
      char *empty = "";
      char *data_in2, name_in[NC_MAX_NAME + 1];

      /* Create create property list. */
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      
      /* Set H5P_CRT_ORDER_TRACKED in the creation property list. This
       * turns on HDF5 creation ordering in the file. */
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;

      /* Create the file, open root group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      
      /* Create string type. */
      if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, strlen(data) + 1) < 0) ERR;
      
      /* Create a scalar space. */
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;

      /* Write an scalar dataset of this type. */
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
/*      if (H5Pset_fill_value(plistid, typeid, &empty) < 0) ERR;*/
      if ((datasetid = H5Dcreate2(grpid, VAR_NAME, typeid, spaceid, 
				  H5P_DEFAULT, plistid, H5P_DEFAULT)) < 0) ERR;
      if (H5Dwrite(datasetid, typeid, spaceid, spaceid, H5P_DEFAULT, 
		   data) < 0) ERR;

      /* Close up. */
      if (H5Dclose(datasetid) < 0) ERR; 
      if (H5Pclose(plistid) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* Read the file with netCDF-4. */
/*       nc_set_log_level(6); */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
      if (ndims_in != 0 || nvars_in != 1 || natts_in != 0 || unlimdim_in != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims_in, NULL, &natts_in)) ERR;
/*      if (strcmp(name_in, VAR_NAME) || type_in != NC_STRING ||
	ndims_in != 0 || natts_in != 0) ERR;*/
      /*if (nc_get_var_string(ncid, 0, &data_in2)) ERR;
      if (strcmp(data_in2, data)) ERR;
      if (nc_free_string(size_in, &data_in2)) ERR;*/
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
/*    printf("*** Checking a HDF5 file with scalar, fixed-length string dataset..."); */
/*    { */
/* #define VAR_NAME "Gettysburg Address" */
/*       hid_t fapl_id, fcpl_id, fileid, grpid, spaceid, typeid, datasetid, plistid; */
/*       int ncid, nvars_in, ndims_in, natts_in, unlimdim_in, type_in; */
/*       size_t size_in; */
/*       char data[] = "Four score and seven years ago our fathers brought forth on " */
/* 	 "this continent, a new nation, conceived in Liberty, and dedicated to " */
/* 	 "the proposition that all men are created equal. Now we are engaged " */
/* 	 "in a great civil war, testing whether that nation, or any nation so " */
/* 	 "conceived and so dedicated, can long endure. We are met on a great " */
/* 	 "battle-field of that war. We have come to dedicate a portion of that " */
/* 	 "field, as a final resting place for those who here gave their lives " */
/* 	 "that that nation might live. It is altogether fitting and proper that " */
/* 	 "we should do this. But, in a larger sense, we can not dedicate -- we " */
/* 	 "can not consecrate -- we can not hallow -- this ground. The brave men, " */
/* 	 "living and dead, who struggled here, have consecrated it, far above our " */
/* 	 "poor power to add or detract. The world will little note, nor long " */
/* 	 "remember what we say here, but it can never forget what they did here. " */
/* 	 "It is for us the living, rather, to be dedicated here to the unfinished " */
/* 	 "work which they who fought here have thus far so nobly advanced. It is " */
/* 	 "rather for us to be here dedicated to the great task remaining before " */
/* 	 "us -- that from these honored dead we take increased devotion to that " */
/* 	 "cause for which they gave the last full measure of devotion -- that we " */
/* 	 "here highly resolve that these dead shall not have died in vain -- that " */
/* 	 "this nation, under God, shall have a new birth of freedom -- and that " */
/* 	 "government of the people, by the people, for the people, shall not " */
/* 	 "perish from the earth."; */
/*       char *empty = ""; */
/*       char *data_in2, name_in[NC_MAX_NAME + 1]; */

/*       /\* Create file access and create property lists. *\/ */
/*       if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR; */
/*       if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR; */
      
/*       /\* Set latest_format in access propertly list. This ensures that */
/*        * the latest, greatest, HDF5 versions are used in the file. *\/ */
/*       if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR; */

/*       /\* Set H5P_CRT_ORDER_TRACKED in the creation property list. This */
/*        * turns on HDF5 creation ordering in the file. *\/ */
/*       if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED | */
/* 					       H5P_CRT_ORDER_INDEXED)) < 0) ERR; */
/*       if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED | */
/* 					       H5P_CRT_ORDER_INDEXED)) < 0) ERR; */

/*       /\* Create the file, open root group. *\/ */
/*       if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR; */
/*       if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR; */
      
/*       /\* Create string type. *\/ */
/*       if ((typeid = H5Tcopy(H5T_C_S1)) < 0) ERR; */
/*       if (H5Tset_size(typeid, strlen(data) + 1) < 0) ERR; */
      
/*       /\* Create a scalar space. *\/ */
/*       if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR; */

/*       /\* Write an scalar dataset of this type. *\/ */
/*       if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR; */
/*       if (H5Pset_fill_value(plistid, typeid, &empty) < 0) ERR; */
/*       if ((datasetid = H5Dcreate1(grpid, VAR_NAME, typeid, */
/* 				  spaceid, plistid)) < 0) ERR; */
/*       if (H5Dwrite(datasetid, typeid, spaceid, spaceid, */
/* 		   H5P_DEFAULT, data) < 0) ERR; */

/*       /\* Close up. *\/ */
/*       if (H5Dclose(datasetid) < 0) ERR; */
/*       if (H5Pclose(fapl_id) < 0) ERR; */
/*       if (H5Pclose(fcpl_id) < 0) ERR; */
/*       if (H5Pclose(plistid) < 0) ERR; */
/*       if (H5Tclose(typeid) < 0) ERR; */
/*       if (H5Gclose(grpid) < 0) ERR; */
/*       if (H5Fclose(fileid) < 0) ERR; */

/*       /\* Read the file with netCDF-4. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
/*       if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR; */
/*       if (ndims_in != 0 || nvars_in != 1 || natts_in != 0 || unlimdim_in != -1) ERR; */
/*       if (nc_inq_var(ncid, 0, name_in, &type_in, &ndims_in, NULL, &natts_in)) ERR; */
/*       if (strcmp(name_in, VAR_NAME) || type_in != NC_STRING ||  */
/* 	  ndims_in != 0 || natts_in != 0) ERR; */
/*       if (nc_get_var_string(ncid, 0, &data_in2)) ERR; */
/*       if (strcmp(data_in2, data)) ERR; */
/*       if (nc_free_string(size_in, &data_in2)) ERR; */
/*       if (nc_close(ncid)) ERR; */
/*    } */
/*    SUMMARIZE_ERR; */
   FINAL_RESULTS;
}

