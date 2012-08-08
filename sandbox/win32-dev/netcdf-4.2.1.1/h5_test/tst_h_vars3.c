/* This is part of the netCDF package. Copyright 2009 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   Test HDF5 dataset code, even more. These are not intended to be
   exhaustive tests, but they use HDF5 the same way that netCDF-4
   does, so if these tests don't work, than netCDF-4 won't work
   either.
*/

#include "h5_err_macros.h"
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_vars3.h5"

int
main()
{
   printf("\n*** Checking HDF5 variable functions even more.\n");

   printf("*** Checking string variable fill value...");
   {
#define NUM_AMENDMENTS 27
#define NA 2
#define MAX_LEN 1024
#define GRP_NAME1 "Constitution"
#define VAR_NAME1 "Amendments"

      char *data[NA];
      char amend[NA][MAX_LEN] = {"Congress shall make no law respecting an "
				 "establishment of religion, or prohibiting the "
				 "free exercise thereof; or abridging the freedom "
				 "of speech, or of the press; or the right of the "
				 "people peaceably to assemble, and to petition "
				 "the government for a redress of grievances.",
				 " A well regulated militia, being necessary to the "
				 "security of a free state, the right of the people "
				 "to keep and bear arms, shall not be infringed."};
      hsize_t dims[1] = {NA};
      hid_t fileid, grpid, spaceid, typeid, datasetid, plistid;
      int i;
      char fill_value[] = "";
      char *f1 = fill_value;

      for (i = 0; i < NA; i++)
	 data[i] = amend[i];

      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME1, 0)) < 0) ERR;

      /* Create string type. */
      if ((typeid =  H5Tcopy(H5T_C_S1)) < 0) ERR;
      if (H5Tset_size(typeid, H5T_VARIABLE) < 0) ERR;

      /* Write an dataset of this type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((plistid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_fill_value(plistid, typeid, &f1) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, VAR_NAME1, typeid,
				 spaceid, plistid)) < 0) ERR;
      if (H5Dwrite(datasetid, typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		   data) < 0) ERR;

      if (H5Dclose(datasetid) < 0 ||
	  H5Pclose(plistid) < 0 ||
	  H5Tclose(typeid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;

   }

   SUMMARIZE_ERR; 
   FINAL_RESULTS;
}
