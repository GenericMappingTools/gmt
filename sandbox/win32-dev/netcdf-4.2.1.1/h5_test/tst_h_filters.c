#include "err_macros.h"
#include <hdf5.h>
#include <H5DSpublic.h>

#define FILE_NAME "tst_h_filters.h5"
#define DIM_LEN 10
#define NDIMS 1
#define VAR1_NAME "very_important_data"
#define FILTER_NAME "ed_compress_2000"
#define FILTER_ID (H5Z_FILTER_RESERVED + 1)
#define NUM_FILTER_PARAM 2

size_t 
ed_compress_2000_filter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], 
			size_t nbytes, size_t *buf_size, void **buf) 
{
   char *new_buf;
   char *old_buf = (char *)*buf;
   int i;
   
/*   printf("ed_compress_2000_filter called: type 0x%x bits %d nbytes %d buf_size %d\n", 
     cd_values[0], cd_values[1], nbytes, *buf_size);*/
   if (flags & H5Z_FLAG_REVERSE)
   {
/*      printf("reading\n");*/
      if (!(new_buf = malloc(*buf_size * 2))) ERR;
      for (i = 0; i < *buf_size * 2; i++)
	 new_buf[i] = old_buf[i/2];
      free(*buf);
      *buf = new_buf;
      *buf_size *= 2;   
   }
   else
   {
/*      printf("writing\n");*/
      if (!(new_buf = malloc(*buf_size/2))) ERR;
      for (i = 0; i < *buf_size/2; i++)
	 new_buf[i] = old_buf[i*2];
      free(*buf);
      *buf = new_buf;
      *buf_size /= 2;
   }
   /* Return number of bytes in buf for success, 0 for failure. */
   return *buf_size;
}

int
main()
{
   printf("\n*** Checking HDF5 filter.\n");
   printf("*** Checking EDPEG2000 compression filter...");
   {
      hid_t fileid, grpid, spaceid, var1_id, create_propid;
      hsize_t dims[NDIMS] = {DIM_LEN}, chunksize = 1;
      int data_out[DIM_LEN], data_in[DIM_LEN];
      H5Z_class2_t fclass;
      unsigned int filter_param[NUM_FILTER_PARAM] = {H5T_NATIVE_INT32, 12};
      int i;

      /* Initialize some data. */
      for (i = 0; i < DIM_LEN; i++)
	 data_out[i] = i;

      /* Create file. */
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT,
                              H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;

      /* Register filter. */
      fclass.version = H5Z_CLASS_T_VERS;
      fclass.id = FILTER_ID;
      fclass.encoder_present = 1;
      fclass.decoder_present = 1;
      fclass.can_apply = NULL;
      fclass.set_local = NULL;
      fclass.filter = &ed_compress_2000_filter;
      if (H5Zregister(&fclass) < 0) ERR;

      /* Create dataset. */
      if ((spaceid = H5Screate_simple(NDIMS, dims, dims)) < 0) ERR;
      if ((create_propid = H5Pcreate(H5P_DATASET_CREATE)) < 0) ERR;
      if (H5Pset_chunk(create_propid, NDIMS, &chunksize) < 0) ERR;
      if (H5Pset_filter(create_propid, FILTER_ID, H5Z_FLAG_OPTIONAL, 
			NUM_FILTER_PARAM, filter_param) < 0) ERR;
      if ((var1_id = H5Dcreate2(grpid, VAR1_NAME, H5T_NATIVE_INT32, spaceid, 
				H5P_DEFAULT, create_propid, H5P_DEFAULT)) < 0) ERR;
      if (H5Dwrite(var1_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_out) < 0) ERR;
      if (H5Sclose(spaceid) < 0) ERR;
      if (H5Dclose(var1_id) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;

      /* Read file. */
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) ERR;
      if ((grpid = H5Gopen2(fileid, "/", H5P_DEFAULT)) < 0) ERR;
      if ((var1_id = H5Dopen2(grpid, VAR1_NAME, H5P_DEFAULT)) < 0) ERR;
      if (H5Dread(var1_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_in) < 0) ERR;
      /*for (i = 0; i < DIM_LEN; i++) */
	 /*printf("new data[%d]=%d\n", i, data_in[i]);*/

      if (H5Dclose(var1_id) < 0) ERR;
      if (H5Gclose(grpid) < 0) ERR;
      if (H5Fclose(fileid) < 0) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
