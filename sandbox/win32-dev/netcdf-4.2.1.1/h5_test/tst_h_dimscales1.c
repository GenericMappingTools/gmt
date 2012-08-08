/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   $Id$
*/
#include <err_macros.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_dimscales.h5"
int
main()
{
   printf("\n*** Checking HDF5 dimension scales.\n");
#define GRP_NAME "simple_scales"
#define DIMSCALE_NAME "dimscale"
#define NAME_ATTRIBUTE "Billy-Bob"
#define VAR1_NAME "var1"
#define VAR2_NAME "var2"
#define VAR3_NAME "var3"
#define DIM1_LEN 3
#define DIM2_LEN 2
#define FIFTIES_SONG "Mamma said they'll be days like this. They'll be days like this, my mamma said."

   printf("*** Creating simple dimension scales file, with scale detach...");
   {
      hid_t fileid, grpid, dimscaleid;
      hid_t dimscale_spaceid, var1_spaceid, var3_spaceid;
      hid_t var1_datasetid, var2_datasetid, var3_datasetid;
      hsize_t dims[2] = {DIM1_LEN, DIM2_LEN};
      hsize_t dimscale_dims[1] = {DIM1_LEN};

      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Create our dimension scale. Use the built-in NAME attribute
       * on the dimscale. */
      if ((dimscale_spaceid = H5Screate_simple(1, dimscale_dims, 
					       dimscale_dims)) < 0) ERR;
      if ((dimscaleid = H5Dcreate(grpid, DIMSCALE_NAME, H5T_NATIVE_INT, 
				  dimscale_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSset_scale(dimscaleid, NAME_ATTRIBUTE) < 0) ERR;

      /* Create a 1D variable which uses the dimscale. Attach a label
       * to this scale. */
      if ((var1_spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((var1_datasetid = H5Dcreate(grpid, VAR1_NAME, H5T_NATIVE_INT, 
				      var1_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(var1_datasetid, dimscaleid, 0) < 0) ERR;
      if (H5DSset_label(var1_datasetid, 0, FIFTIES_SONG) < 0) ERR;

      /* Create a 1D variabls that doesn't use the dimension scale. */
      if ((var2_datasetid = H5Dcreate(grpid, VAR2_NAME, H5T_NATIVE_INT, 
				      var1_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Create a 2D dataset which uses the scale for one of its
       * dimensions. */
      if ((var3_spaceid = H5Screate_simple(2, dims, dims)) < 0) ERR;
      if ((var3_datasetid = H5Dcreate(grpid, VAR3_NAME, H5T_NATIVE_INT, 
				      var3_spaceid, H5P_DEFAULT)) < 0) ERR;
      if (H5DSattach_scale(var3_datasetid, dimscaleid, 0) < 0) ERR;

      /* Detach the scale. */
      if (H5DSdetach_scale(var3_datasetid, dimscaleid, 0) < 0) ERR;

      /* Close up the shop. */
      if (H5Dclose(dimscaleid) < 0 ||
	  H5Dclose(var1_datasetid) < 0 ||
	  H5Dclose(var2_datasetid) < 0 ||
	  H5Dclose(var3_datasetid) < 0 ||
	  H5Sclose(var1_spaceid) < 0 ||
	  H5Sclose(var3_spaceid) < 0 ||
	  H5Sclose(dimscale_spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

