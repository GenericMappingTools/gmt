/* This is part of the netCDF package.
   Copyright 2007 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 compound types. 
*/

#include "h5_err_macros.h"
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
   struct s1 {
	 unsigned char c1;
	 double d;
   } data[DIM1_LEN];
   int i;

   printf("\n*** Checking HDF5 compound types (we're almost there kids).\n");
   printf("*** Checking packing of HDF5 compound types...");
   
   /* Open file. */
   if ((access_plist = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
   if (H5Pset_fclose_degree(access_plist, H5F_CLOSE_STRONG)) ERR;
   if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, access_plist)) < 0) ERR;

   /* Open dataset. */
   if ((datasetid = H5Dopen1(fileid, VAR_NAME)) < 0) ERR;

   /* Check space. */
   if ((spaceid = H5Dget_space(datasetid)) < 0) ERR;
   if (H5Sget_simple_extent_ndims(spaceid) != 1) ERR;
   if (H5Sget_simple_extent_npoints(spaceid) != DIM1_LEN) ERR;

   /* Get type. */
   if ((typeid = H5Dget_type(datasetid)) < 0) ERR;

/*   if ((native_type = H5Tget_native_type(typeid, H5T_DIR_DEFAULT)) < 0) ERR;*/

   /* Read the data. */
   if (H5Dread(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
	       data) < 0) ERR;

   /* Check the data. */
   for (i=0; i<DIM1_LEN; i++)
      if (data[i].c1 != 126 || data[i].d != -9999999) ERR;

   /* Release all resources. */
   if (H5Fclose(fileid) < 0) ERR;

   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
