/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program excersizes HDF5 variable length array code.
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_opaques.h5"
#define DIM1_LEN 3
#define ATT_NAME "att_name"
#define SIZE 9

int
main()
{
   hid_t fileid, grpid, spaceid, typeid, attid;
   hsize_t dims[1] = {DIM1_LEN};
   char data[DIM1_LEN][SIZE];
   int i, j;
/*   size_t size;*/

   /* Create some phoney data. */
   for (i = 0; i < DIM1_LEN; i++)
      for (j = 0; j < SIZE; j++)
	 data[i][j] = 0;

   printf("\n*** Checking HDF5 opaque types.\n");
   printf("*** Checking simple HDF5 opaque types...");
   
   /* Open file. */
   if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			   H5P_DEFAULT)) < 0) ERR;
   if ((grpid = H5Gcreate(fileid, "grp1", 0)) < 0) ERR;

   /* Create opaque type. */
   if ((typeid =  H5Tcreate(H5T_OPAQUE, SIZE)) < 0) ERR;

   /* The size is rouned up to ?. */
   /*if (!(size = H5Tget_size(typeid))) ERR;
   if (size != 8) ERR;*/

   /* Write an attribute of this type. */
   if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
   if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid, 
			  H5P_DEFAULT)) < 0) ERR;
   if (H5Awrite(attid, typeid, data) < 0) ERR;
   if (H5Aclose(attid) < 0) ERR;
   if (H5Tclose(typeid) < 0) ERR;
   if (H5Gclose(grpid) < 0) ERR;
   if (H5Fclose(fileid) < 0) ERR;

   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
