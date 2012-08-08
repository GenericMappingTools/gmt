/* This is part of the netCDF package.  Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This program excersizes HDF5 variable length array code.
*/

#include <err_macros.h>
#include <hdf5.h>

#define FILE_NAME "tst_h_vl.h5"
#define DIM1_LEN 3
#define ATT_NAME "att_name"
#define GROUP_NAME "grp1"

int
main()
{
   printf("\n*** Checking HDF5 VLEN types.\n");
   printf("*** Checking simple HDF5 variable length types...");
   {
      hid_t fileid, grpid, spaceid, typeid, attid;
      hsize_t dims[1] = {DIM1_LEN};
      hvl_t data[DIM1_LEN], data_in[DIM1_LEN];
      int *phoney;
      int i, j;
      size_t size;
      
      /* Create some phoney data, an array of struct s1, which holds a
       * pointer to a variable length array of int. */
      for (i=0; i<DIM1_LEN; i++)
      {
	 if (!(phoney = malloc(sizeof(int) * (i+1)))) ERR;
	 for (j=0; j<i+1; j++)
	    phoney[j] = -99;
	 data[i].p = phoney;
	 data[i].len = i+1;
      }
      
      /* Open file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gcreate(fileid, GROUP_NAME, 0)) < 0) ERR;

      /* Create VLEN type. */
      if ((typeid =  H5Tvlen_create(H5T_NATIVE_INT)) < 0) ERR;

      /* Although it's a vlen of ints, the size is rouned up to 8. */
      if (!(size = H5Tget_size(typeid))) ERR;
      if (size < 8) ERR;

      /* Write an attribute of this vlen type. */
      if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR;
      if ((attid = H5Acreate(grpid, ATT_NAME, typeid, spaceid, 
			     H5P_DEFAULT)) < 0) ERR;
      if (H5Awrite(attid, typeid, data) < 0) ERR;
      if (H5Aclose(attid) < 0) ERR;
      if (H5Tclose(typeid) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* Reopen the file and read the vlen data. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen(fileid, GROUP_NAME)) < 0) ERR;
      if ((attid = H5Aopen_name(grpid, ATT_NAME)) < 0) ERR;
      if ((spaceid = H5Aget_space(attid)) < 0) ERR;
      if ((typeid = H5Aget_type(attid)) < 0) ERR;
      if (H5Aread(attid, typeid, data_in) < 0) ERR;

      /* Check the data */
      for (i=0; i<DIM1_LEN; i++)
      {
	 if (data[i].len != i+1) ERR;
	 for (j = 0; j < i+1; j++)
	    if (((int *)(data[i].p))[j] != -99) ERR;
      }

      /* Free the memory allocated above. */
      for (i=0; i<DIM1_LEN; i++)
	 free(data[i].p);

      /* HDF5 allocated memory to store the data. Free that memory. */
      if (H5Dvlen_reclaim(typeid, spaceid, H5P_DEFAULT, data_in) < 0) ERR;

      /* Close everything. */
      if (H5Aclose(attid) < 0 ||
	  H5Sclose(spaceid) < 0 || 
	  H5Tclose(typeid) < 0 || 
	  H5Gclose(grpid) < 0 || 
	  H5Fclose(fileid) < 0) ERR;
   }

   SUMMARIZE_ERR;
/*    printf("*** Checking array of compound holding a vlen..."); */

/*    { */
/*       hid_t vlen_typeid, compound_typeid, spaceid, datasetid; */
/*       struct sea_sounding */
/*       { */
/* 	    int sounding_no; */
/* 	    hvl_t temp_vl; */
/*       } data[DIM1_LEN]; */
/*       float *phoney; */
/*       int i, j; */
      
/*       /\* Create phoney data. *\/ */
/*       for (i=0; i<DIM1_LEN; i++) */
/*       { */
/* 	  if (!(phoney = malloc(sizeof(float) * (i+1)))) ERR; */
/* 	 for (j=0; j<i+1; j++) */
/* 	    phoney[j] = 23.5 - j; */
/* 	 data[i].temp_vl.p = phoney; */
/* 	 data[i].temp_vl.len = i+1; */
/*       } */

/*       /\* Create file. *\/ */
/*       if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,  */
/* 			      H5P_DEFAULT)) < 0) ERR; */
/*       if ((grpid = H5Gcreate(fileid, "grp1", 0)) < 0) ERR; */
      
/*       /\* Create VLEN type. *\/ */
/*       if ((vlen_typeid =  H5Tvlen_create(H5T_NATIVE_FLOAT)) < 0) ERR; */

/*       /\* Create a compound type that holds the vlen type. *\/ */
/*       if ((compound_typeid = H5Tcreate(H5T_COMPOUND,  */
/* 				       sizeof(struct sea_sounding))) < 0) ERR; */
/*       if (H5Tinsert(compound_typeid, "sounding_no", HOFFSET(struct sea_sounding, sounding_no),  */
/* 		    H5T_NATIVE_INT) < 0) ERR; */
/*       if (H5Tinsert(compound_typeid, "temp_vl", HOFFSET(struct sea_sounding, temp_vl),  */
/* 		    vlen_typeid) < 0) ERR; */
/*       if (H5Tcommit(grpid, "sea_sounding_type", compound_typeid) < 0) ERR; */

/*       if ((spaceid = H5Screate_simple(1, dims, NULL)) < 0) ERR; */
/*       if ((datasetid = H5Dcreate(grpid, "sea_sounding_dataset", compound_typeid,  */
/* 				 spaceid, H5P_DEFAULT)) < 0) ERR; */
/*       if (H5Dwrite(datasetid, compound_typeid, H5S_ALL, H5S_ALL, H5P_DEFAULT,  */
/* 		   data) < 0) ERR; */

/*       if (H5Dclose(datasetid) < 0) ERR; */
/*       if (H5Tclose(compound_typeid) < 0) ERR; */
/*       if (H5Tclose(vlen_typeid) < 0) ERR; */
/*       if (H5Sclose(spaceid) < 0) ERR; */
/*       if (H5Gclose(grpid) < 0) ERR; */
/*       if (H5Fclose(fileid) < 0) ERR; */
/*    } */

/*    SUMMARIZE_ERR; */

   FINAL_RESULTS;
}
