/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

*/
#include <config.h>
#include <nc_tests.h>
#include "netcdf.h"
#include <hdf5.h>
#include <H5DSpublic.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */

#define FILE_NAME "tst_h_many_atts.h5"
#define GRP_NAME "group1"

int
main()
{
   printf("\n*** Checking many attributes in HDF5 file.\n");
   printf("*** Checking some more simple atts...\n");
   {
#define NUM_ATTS 10000
      hid_t fcpl_id, hdfid, grpid;
      hid_t spaceid, attid1;
      int one = 1;
      hsize_t dims[1] = {1};
      int i;
      char name[NC_MAX_NAME];
      struct timeval start_time, end_time, diff_time;
      double sec;

      /* Create a HDF5 file. */
      if ((fcpl_id = H5Pcreate(H5P_FILE_CREATE)) < 0) ERR;
      if (H5Pset_link_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if (H5Pset_attr_creation_order(fcpl_id, (H5P_CRT_ORDER_TRACKED |
					       H5P_CRT_ORDER_INDEXED)) < 0) ERR;
      if ((hdfid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, fcpl_id, H5P_DEFAULT)) < 0) ERR;
      if (H5Pclose(fcpl_id) < 0) ERR;

      /* Open the root group. */
      if ((grpid = H5Gopen2(hdfid, "/", H5P_DEFAULT)) < 0) ERR;

      if (gettimeofday(&start_time, NULL)) ERR;
      /* Write an attribute. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      for (i = 0; i < NUM_ATTS; i++)
      {
	 sprintf(name, "att_%d", i);
	 if ((attid1 = H5Acreate2(grpid, name, H5T_NATIVE_INT, spaceid, 
				  H5P_DEFAULT, H5P_DEFAULT)) < 0) ERR;
	 if (H5Awrite(attid1, H5T_NATIVE_INT, &one) < 0) ERR;
/*	 if (H5Aclose(attid1) < 0) ERR;*/
	 if((i + 1) % 1000 == 0) 
	 {		/* only print every 1000th attribute name */
	    if (gettimeofday(&end_time, NULL)) ERR;
	    if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	    sec = diff_time.tv_sec + 1.0e-6 * diff_time.tv_usec;
	    printf("%i\t%.3g sec\n", i + 1, sec);
	 }
      }

      /* Close everything. */
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(hdfid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
