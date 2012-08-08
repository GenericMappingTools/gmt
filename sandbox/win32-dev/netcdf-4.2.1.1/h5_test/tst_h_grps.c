/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Check out HDF5 groups. 
*/

#include "h5_err_macros.h"
#include <hdf5.h>

#define FILE_NAME "tst_h_grps.h5"
#define GRP_NAME "Bubba-Joe"
#define SUB_GRP_NAME "Billy-Bob"
#define DATASET_NAME "Sally-Sue"
#define NEW_NAME "Mary-Lou"

/* NFC normalized UTF-8 for Unicode 8-character "Hello" in Greek */
unsigned char norm_utf8[] = {
   0xCE, 0x9A,	  /* GREEK CAPITAL LETTER KAPPA  : 2-bytes utf8 */
   0xCE, 0xB1,	  /* GREEK SMALL LETTER LAMBDA   : 2-bytes utf8 */
   0xCE, 0xBB,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
   0xCE, 0xB7,	  /* GREEK SMALL LETTER ETA      : 2-bytes utf8 */
   0xCE, 0xBC,	  /* GREEK SMALL LETTER MU       : 2-bytes utf8 */
   0xCE, 0xAD,    /* GREEK SMALL LETTER EPSILON WITH TONOS 
		     : 2-bytes utf8 */
   0xCF, 0x81,	  /* GREEK SMALL LETTER RHO      : 2-bytes utf8 */
   0xCE, 0xB1,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
   0x00
};

int
main()
{
   printf("\n*** Checking HDF5 group functions.\n");
   printf("*** Checking out root group...");
   {
      hid_t fileid, grpid, access_plistid;
      
      /* Open the root group of a new file. */
      if ((access_plistid = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(access_plistid, H5F_CLOSE_SEMI)) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      access_plistid)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
      
      /* Reopen file and root group. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, 
			    access_plistid)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking out H5Gmove...");
   {

      hid_t fileid, grpid;
      hid_t datasetid, spaceid;

      /* Create file with one dataset. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((spaceid = H5Screate(H5S_SCALAR)) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, DATASET_NAME, H5T_NATIVE_INT, 
				 spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5Dclose(datasetid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
      
      /* Reopen file and check, then rename dataset. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, 
			    H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, DATASET_NAME)) < 0) ERR;
      if (H5Dclose(datasetid) < 0) ERR;
      if (H5Gmove(grpid, DATASET_NAME, NEW_NAME) < 0) ERR;
      if ((datasetid = H5Dopen1(grpid, NEW_NAME)) < 0) ERR;
      if (H5Dclose(datasetid) < 0 ||
	  H5Gclose(grpid) < 0 || 
	  H5Fclose(fileid) < 0)
	 ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking out sub-groups...");

   {
      hid_t fileid, grpid, subgrpid;

      /* Create file with some nested groups. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      if ((subgrpid = H5Gcreate(grpid, SUB_GRP_NAME, 0)) < 0) ERR;
      if (H5Gclose(subgrpid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
      
      /* Reopen file and discover groups. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GRP_NAME)) < 0) ERR;
      if ((subgrpid = H5Gopen(grpid, SUB_GRP_NAME)) < 0) ERR;
      if (H5Gclose(subgrpid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking out UTF8 named sub-group...");

   {
      hid_t fileid, grpid, subgrpid;

      /* Create file with nested group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, (char *)norm_utf8, 0)) < 0) ERR;
      if ((subgrpid = H5Gcreate(grpid, SUB_GRP_NAME, 0)) < 0) ERR;
      if (H5Gclose(subgrpid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
      
      /* Reopen file and discover groups. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, (char *)norm_utf8)) < 0) ERR;
      if ((subgrpid = H5Gopen(grpid, SUB_GRP_NAME)) < 0) ERR;
      if (H5Gclose(subgrpid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** Checking out UTF8 named sub-group with group creation ordering...");

   {
      hid_t fileid, grpid, subgrpid;
      hid_t fapl_id, fcpl_id, gcpl_id;

      /* Create file with nested group. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, fapl_id)) < 0) ERR;

      if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED|H5P_CRT_ORDER_INDEXED) < 0) ERR;
      if ((grpid = H5Gcreate_anon(fileid, gcpl_id, H5P_DEFAULT)) < 0) ERR;
      if ((H5Olink(grpid, fileid, (char *)norm_utf8, H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;

      if ((subgrpid = H5Gcreate(grpid, SUB_GRP_NAME, 0)) < 0) ERR;
      if (H5Gclose(subgrpid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
      
      /* Reopen file and discover groups. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, (char *)norm_utf8)) < 0) ERR;
      if ((subgrpid = H5Gopen(grpid, SUB_GRP_NAME)) < 0) ERR;
      if (H5Gclose(subgrpid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
















