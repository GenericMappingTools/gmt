/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Use HDF5 to read c0.nc, a file created by ncdump. This check was
   added to detect a problem in the early HDF5 1.8.0 releases.

   $Id$
*/
#include <nc_tests.h>
#include <hdf5.h>

#define FILE_NAME "c0.nc"
#define MAX_NAME 1024

int
main()
{
   printf("\n*** Checking HDF5 file c0.nc.\n");
   printf("*** Checking HDF5 objcts...");

   {
      hid_t fileid, grpid;
      hsize_t num_obj, i;
      int obj_class;
      char obj_name[MAX_NAME];

      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR; 
      if ((grpid = H5Gopen(fileid, "/")) < 0) ERR;

      /* Find the variables. Read their metadata and attributes. */
      if (H5Gget_num_objs(grpid, &num_obj) < 0) ERR;
      for (i=0; i<num_obj; i++)
      {
	 /* Get the class (i.e. group, dataset, etc.), and the name of
	  * the object. */
	 if ((obj_class = H5Gget_objtype_by_idx(grpid, i)) < 0) ERR;
	 if (H5Gget_objname_by_idx(grpid, i, obj_name, MAX_NAME) < 0) ERR;
      }

      if (H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
