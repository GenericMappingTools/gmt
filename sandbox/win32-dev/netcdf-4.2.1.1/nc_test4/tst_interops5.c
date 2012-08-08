/* This is part of the netCDF package. Copyright 2005-2011, University
   Corporation for Atmospheric Research/Unidata.  See COPYRIGHT file
   for conditions of use.

   Test that HDF5 and NetCDF-4 can read and write the same file.
*/
#include <config.h>
#include <nc_tests.h>
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_interops5.h5"

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF5/NetCDF-4 interoperability...\n");
   printf("*** testing HDF5 compatibility...");
   {
#define GRPA_NAME "grpa"
#define VAR_NAME "vara"
#define NDIMS 2
      int nrowCur = 7;               /* current size */
      int ncolCur = 3;
      int nrowMax = nrowCur + 0;     /* maximum size */
      int ncolMax = ncolCur + 0;

      hid_t xdimId;
      hid_t ydimId;
      hsize_t xscaleDims[1];
      hsize_t yscaleDims[1];
      hid_t xdimSpaceId, spaceId;
      hid_t fileId;
      hid_t fapl; 
      hsize_t curDims[2];
      hsize_t maxDims[2];
      hid_t dataTypeId, dsPropertyId, grpaId, grpaPropId, dsId;
      hid_t ydimSpaceId;
      const char * dimNameBase
	 = "This is a netCDF dimension but not a netCDF variable.";
      char dimNameBuf[1000];
      char *varaName = "/grpa/vara";
      short amat[nrowCur][ncolCur];
      int ii, jj;

      xscaleDims[0] = nrowCur;
      yscaleDims[0] = ncolCur;
      if ((xdimSpaceId = H5Screate_simple(1, xscaleDims, NULL)) < 0) ERR;

      /* With the SEMI close degree, the HDF5 file close will fail if
       * anything is left open. */
      if ((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI)) ERR;

      /* Create file */
      if((fileId = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, 
			     H5Pcreate(H5P_FILE_CREATE), fapl)) < 0) ERR;  
      if (H5Pclose(fapl) < 0) ERR;

      /* Create data space */
      curDims[0] = nrowCur;
      curDims[1] = ncolCur;
      maxDims[0] = nrowMax;
      maxDims[1] = ncolMax;
      if ((spaceId = H5Screate_simple(2, curDims, maxDims)) < 0) ERR;

      if ((dataTypeId = H5Tcopy(H5T_NATIVE_SHORT)) < 0) ERR;

      if ((dsPropertyId = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;

      if ((grpaPropId = H5Pcreate(H5P_GROUP_CREATE)) < 0) ERR;
      if ((grpaId = H5Gcreate2(fileId, GRPA_NAME, H5P_DEFAULT, 
			       grpaPropId, H5P_DEFAULT)) < 0) ERR;
      if (H5Pclose(grpaPropId) < 0) ERR;

      /* Create vara dataset */
      if ((dsId = H5Dcreate2(fileId, varaName, dataTypeId, spaceId, 
			     H5P_DEFAULT, dsPropertyId, 
			     H5P_DEFAULT)) < 0) ERR;   

      if (H5Pclose(dsPropertyId) < 0) ERR;
      if (H5Tclose(dataTypeId) < 0) ERR;
      if ((ydimSpaceId = H5Screate_simple(1, yscaleDims, NULL)) < 0) ERR; 

      /* Create xdim dimension dataset */
      if ((xdimId = H5Dcreate2(fileId, "/xdim", H5T_IEEE_F32BE, 
			       xdimSpaceId, H5P_DEFAULT, H5P_DEFAULT,
			       H5P_DEFAULT)) < 0) ERR; 

      if (H5Sclose(xdimSpaceId) < 0) ERR;

      /* Create ydim dimension dataset */
      if ((ydimId = H5Dcreate2(fileId, "/ydim", H5T_IEEE_F32BE,
			       ydimSpaceId, H5P_DEFAULT, H5P_DEFAULT,
			       H5P_DEFAULT)) < 0) ERR; 
      if (H5Sclose(ydimSpaceId) < 0) ERR;

      /* Create xdim scale */
      sprintf(dimNameBuf, "%s%10d", dimNameBase, nrowCur);
      if (H5DSset_scale(xdimId, dimNameBuf) < 0) ERR;

      /* Create ydim scale */
      sprintf(dimNameBuf, "%s%10d", dimNameBase, ncolCur);
      if (H5DSset_scale(ydimId, dimNameBuf) < 0) ERR;

      /* Attach dimension scales to the dataset */
      if (H5DSattach_scale(dsId, xdimId, 0) < 0) ERR;

      if (H5DSattach_scale(dsId, ydimId, 1) < 0) ERR;

      /* Close stuff. */
      if (H5Dclose(xdimId) < 0) ERR;
      if (H5Dclose(ydimId) < 0) ERR;
      if (H5Dclose(dsId) < 0) ERR;
      if (H5Gclose(grpaId) < 0) ERR;
      if (H5Sclose(spaceId) < 0) ERR;
      if (H5Fclose(fileId) < 0) ERR;

      /* Create some data */
      for (ii = 0; ii < nrowCur; ii++) 
	 for (jj = 0; jj < ncolCur; jj++) 
	    amat[ii][jj] = 100 * ii + jj;

      /* Re-open file */
      if ((fileId = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpaId = H5Gopen2(fileId, GRPA_NAME, H5P_DEFAULT)) < 0) ERR;
      if ((dsId = H5Dopen2(grpaId, varaName,  H5P_DEFAULT)) < 0) ERR;

      /* Write dataset */
      if (H5Dwrite(dsId, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
		   amat) < 0) ERR;

      /* Write dimension values for both xdim, ydim */
      short xydimMat[ nrowCur >= ncolCur ? nrowCur : ncolCur];
      for (ii = 0; ii < nrowCur; ii++) 
	 xydimMat[ii] = 0;    /*#### 100 * ii; */

      /* Write xdim */
      if ((xdimId = H5Dopen2(fileId, "/xdim", H5P_DEFAULT)) < 0) ERR;
      if (H5Dwrite(xdimId, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL,
		   H5P_DEFAULT, xydimMat) < 0) ERR;
      if (H5Dclose(xdimId) < 0) ERR;

      /* Write ydim */
      if ((ydimId = H5Dopen2(fileId, "/ydim", H5P_DEFAULT)) < 0) ERR;
      if (H5Dwrite(ydimId, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL,
		   H5P_DEFAULT, xydimMat) < 0) ERR;
      if (H5Dclose(ydimId) < 0) ERR;

      if (H5Dclose(dsId) < 0) ERR;
      if (H5Gclose(grpaId) < 0) ERR;
      if (H5Fclose(fileId) < 0) ERR;

      {
	 int ncid, grpid, nvars, ngatts, ndims, unlimdimid, ngrps;
	 char name_in[NC_MAX_NAME + 1];
	 nc_type xtype_in;
	 int ndims_in, natts_in, dimid_in[NDIMS];

/*	 nc_set_log_level(5);*/
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	 if (nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
	 if (ndims != 2 || nvars != 0 || ngatts != 0 || unlimdimid != -1) ERR;
	 if (nc_inq_grps(ncid, &ngrps, &grpid)) ERR;
	 if (ngrps != 1) ERR;
	 if (nc_inq(grpid, &ndims, &nvars, &ngatts, &unlimdimid)) ERR;
	 if (ndims != 0 || nvars != 1 || ngatts != 0 || unlimdimid != -1) ERR;

	 if (nc_inq_var(grpid, 0, name_in, &xtype_in, &ndims_in, dimid_in, 
			&natts_in)) ERR;
	 if (strcmp(name_in, VAR_NAME) || xtype_in != NC_SHORT || ndims_in != NDIMS ||
	     dimid_in[0] != 0 || dimid_in[1] != 1 || natts_in != 0) ERR;
      
	 if (nc_close(ncid)) ERR;
      }
   }
   SUMMARIZE_ERR;
#ifdef USE_SZIP
   printf("*** testing HDF5 compatibility with szip...");
   {

#define DEFLATE_LEVEL 9
#define MAX_NAME 100   
#define NUM_CD_ELEM 10
/* HDF5 defines this... */
#define DEFLATE_NAME "deflate"
#define DIM1_LEN 3000
#define GRP_NAME "George_Washington"
#define BATTLE_RECORD "Battle_Record"

      hid_t fileid, grpid, spaceid, datasetid;
      int data_out[DIM1_LEN], data_in[DIM1_LEN];
      hsize_t dims[1] = {DIM1_LEN};
      H5Z_filter_t filter;
      int num_filters;
      hid_t propid;
      unsigned int flags, cd_values[NUM_CD_ELEM], filter_config;
      size_t cd_nelems = NUM_CD_ELEM;
      size_t namelen = MAX_NAME;
      char name[MAX_NAME + 1], name_in[MAX_NAME + 1];
      int ncid, ndims_in, nvars_in, ngatts_in, unlimdimid_in, ngrps_in;
      int dimid_in[1], natts_in;

      nc_type xtype_in;
      int i;

      for (i = 0; i < DIM1_LEN; i++)
	 data_out[i] = i;
      
      /* Open file and create group. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;
      
      /* Write an array of bools, with szip compression. */
      if ((propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_layout(propid, H5D_CHUNKED)) ERR;
      if (H5Pset_chunk(propid, 1, dims)) ERR;
      if (H5Pset_szip(propid, H5_SZIP_EC_OPTION_MASK, 32)) ERR;
      if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
      if ((datasetid = H5Dcreate(grpid, BATTLE_RECORD, H5T_NATIVE_INT, 
				 spaceid, propid)) < 0) ERR;
      if (H5Dwrite(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
		   data_out) < 0) ERR;
      if (H5Dclose(datasetid) < 0 ||
	  H5Pclose(propid) < 0 ||
	  H5Sclose(spaceid) < 0 ||
	  H5Gclose(grpid) < 0 ||
	  H5Fclose(fileid) < 0)
	 ERR;

      /* Open the file with netCDF and check it. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 0 || nvars_in != 0 || ngatts_in != 0 || unlimdimid_in != -1) ERR;
      if (nc_inq_grps(ncid, &ngrps_in, &grpid)) ERR;
      if (ngrps_in != 1) ERR;
      if (nc_inq(grpid, &ndims_in, &nvars_in, &ngatts_in, &unlimdimid_in)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || ngatts_in != 0 || unlimdimid_in != -1) ERR;

      /* Check the variable. */
      if (nc_inq_var(grpid, 0, name_in, &xtype_in, &ndims_in, dimid_in,
      		     &natts_in)) ERR;
      if (strcmp(name_in, BATTLE_RECORD) || xtype_in != NC_INT || ndims_in != 1 ||
      	  dimid_in[0] != 0 || natts_in != 0) ERR;

      /* Check the data. */
      if (nc_get_var(grpid, 0, data_in)) ERR;
      for (i = 0; i < DIM1_LEN; i++)
	 if (data_in[i] != data_out[i]) ERR;
      
      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
#endif /* USE_SZIP */
   FINAL_RESULTS;
}

