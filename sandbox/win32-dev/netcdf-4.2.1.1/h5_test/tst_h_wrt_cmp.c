/* This is part of the netCDF package.
   Copyright 2007 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 compound types. 
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_wrt_cmp.h5"
#define DIM1_LEN 3
#define COMPOUND_NAME "cmp"
#define VAR_NAME "var"

int
main()
{
   hid_t fileid, access_plist, spaceid, typeid;
   hid_t datasetid;
   hsize_t dims[1];
   char dummy[] = "                                 ";
   struct s1 
   {
      unsigned char c1;
      double d;
   } data[DIM1_LEN];
   int i;

   /* Initialize our data space. This is required to keep valgrind
    * happy. It makes no difference to the execution of the
    * program. */
   for (i = 0; i < DIM1_LEN; i++)
      memcpy((void *)(&data[i]), (void *)dummy, sizeof(struct s1));

   /* Now init our values. */
   for (i = 0; i < DIM1_LEN; i++)
   {
      data[i].c1 = 126;
      data[i].d = -9999999;
   }

   printf("\n*** Checking HDF5 compound types (even more so).\n");
   printf("*** Checking packing of HDF5 compound types...");
   
   /* Open file and create group. */
   if ((access_plist = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
   if (H5Pset_fclose_degree(access_plist, H5F_CLOSE_STRONG)) ERR;
   if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			   access_plist)) < 0) ERR;

   /* Create a simple compound type. */
   if ((typeid = H5Tcreate(H5T_COMPOUND, sizeof(struct s1))) < 0) ERR;
   if (H5Tinsert(typeid, "c1", HOFFSET(struct s1, c1), H5T_NATIVE_UCHAR) < 0) ERR;
   if (H5Tinsert(typeid, "d", HOFFSET(struct s1, d), H5T_NATIVE_DOUBLE) < 0) ERR;
   if (H5Tcommit(fileid, COMPOUND_NAME, typeid) < 0) ERR;

   /* Create a space. */
   dims[0] = DIM1_LEN;
   if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;

   /* Create a dataset of this compound type. */
   if ((datasetid = H5Dcreate(fileid, VAR_NAME, typeid, spaceid, 
			      H5P_DEFAULT)) < 0) ERR;

   /* Write some data. */
   if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
		data) < 0) ERR;

   /* Release all resources. */
   if (H5Pclose(access_plist) < 0) ERR;
   if (H5Tclose(typeid) < 0) ERR;
   if (H5Sclose(spaceid) < 0) ERR;
   if (H5Fclose(fileid) < 0) ERR;

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
