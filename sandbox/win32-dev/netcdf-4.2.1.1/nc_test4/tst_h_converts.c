/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   $Id$
*/
#include <nc_tests.h>
#include <limits.h>

#define FILE_NAME "tst_h_converts.h5"
#define GRP_NAME "my_group"
#define INT_VAR_NAME "int_var"
#define FLOAT_VAR_NAME "float_var"
#define DOUBLE_VAR_NAME "double_var"
#define SCHAR_VAR_NAME "schar_var"
#define DIM1_LEN 3


/* This function is called when there is an overflow in a read/write
 * operation. Pass the fill value in as user_data. */
H5T_conv_ret_t
except_func(int except_type, hid_t src_id, hid_t dst_id, 
	    void *src_buf, void *dst_buf, void *user_data);

int
main()
{
   hid_t fileid, grpid, spaceid, datasetid, schar_datasetid;
   hid_t float_datasetid, double_datasetid;
   hid_t xfer_plistid;
   int int_data_out[DIM1_LEN] = {-666, 0, 666};
   int int_data_in[DIM1_LEN];
   unsigned char uchar_data_out[DIM1_LEN] = {0, 128, 255};
   unsigned char uchar_data_in[DIM1_LEN];
   int overflow_count = 0;
   float float_data_out[DIM1_LEN] = {-.1, 9999.99, 100.001}, float_data_in[DIM1_LEN];
/*   float float_data_out2[DIM1_LEN] = {-2147483649.0, -2147483649.0, 2147483648.0};*/
   float float_data_out2[DIM1_LEN] = {INT_MAX, INT_MAX, INT_MAX + 1290.0};
   double double_data_out2[DIM1_LEN] = {(double)INT_MIN - 1.0, INT_MAX, (double)INT_MAX + 129.0};
   double double_data_out3[DIM1_LEN] = {-128, 127, 127};
   hsize_t dims[1];
   int i;

   printf("\n*** Checking HDF5 data type conversions.\n");
   printf("*** creating test file...");

   /* Open file and create group. */
   if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			   H5P_DEFAULT)) < 0) ERR;
   if ((grpid = H5Gcreate(fileid, GRP_NAME, 0)) < 0) ERR;

   /* Create a data transfer property lists, and set up an exception
    * function which will be called whenever a conversion exception
    * happens (i.e. an overflow, truncation, or loss of precision
    * during data conversion). */
   if ((xfer_plistid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;
   if (H5Pset_type_conv_cb(xfer_plistid, except_func, &overflow_count) < 0) ERR;

   /* Write small arrays of ints, signed chars, and floats. Just to be
    * tricky, write the uchar data out to the schar dataset. This will
    * cause overflows. */
   dims[0] = DIM1_LEN;
   if ((spaceid = H5Screate_simple(1, dims, dims)) < 0) ERR;
   if ((datasetid = H5Dcreate(grpid, INT_VAR_NAME, H5T_NATIVE_INT, 
			      spaceid, H5P_DEFAULT)) < 0) ERR;
   if ((schar_datasetid = H5Dcreate(grpid, SCHAR_VAR_NAME, H5T_NATIVE_SCHAR, 
				    spaceid, H5P_DEFAULT)) < 0) ERR;
   if ((double_datasetid = H5Dcreate(grpid, DOUBLE_VAR_NAME, H5T_NATIVE_DOUBLE, 
				    spaceid, H5P_DEFAULT)) < 0) ERR;
   if ((float_datasetid = H5Dcreate(grpid, FLOAT_VAR_NAME, H5T_NATIVE_FLOAT, 
				    spaceid, H5P_DEFAULT)) < 0) ERR;
   if (H5Dwrite(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, xfer_plistid, 
		int_data_out) < 0) ERR;
   if (H5Dwrite(schar_datasetid, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, 
		xfer_plistid, uchar_data_out) < 0) ERR;
/*   if (H5Dwrite(double_datasetid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
     xfer_plistid, double_data_out) < 0) ERR;*/
   if (H5Dwrite(float_datasetid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, 
		xfer_plistid, float_data_out) < 0) ERR;

   /* This should have produced 2 overflows. */
   if (overflow_count != 2) ERR;
   overflow_count = 0;

   if (H5Dwrite(schar_datasetid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
		xfer_plistid, double_data_out3) < 0) ERR;
   /* This should have produced 0 overflows. */
   if (overflow_count != 0) ERR;
   overflow_count = 0;

   if (H5Dclose(datasetid) < 0 ||
       H5Dclose(schar_datasetid) < 0 ||
       H5Dclose(double_datasetid) < 0 ||
       H5Dclose(float_datasetid) < 0 ||
       H5Sclose(spaceid) < 0) ERR;

   SUMMARIZE_ERR;

   /* Make sure ints can be converted into floats. Read the int
    * dataset into an array of floats, and make sure the results match
    * netcdf's conversion rules (which are exactly the C typecasting
    * rules). */
   printf("*** reading int dataset as floats...");
   if ((datasetid = H5Dopen1(grpid, INT_VAR_NAME)) < 0) ERR;
   if (H5Dread(datasetid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, float_data_in) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;
   for (i=0; i<DIM1_LEN; i++)
      if (float_data_in[i] != (float)int_data_out[i]) ERR;

   /* This should have produced no overflows. */
   if (overflow_count != 0) ERR;
   overflow_count = 0;

   SUMMARIZE_ERR;

   /* Make sure floats can be converted into schars. Read the float
    * dataset into an array of uchar. */
   printf("*** reading float dataset as uchars...");
   if ((datasetid = H5Dopen1(grpid, FLOAT_VAR_NAME)) < 0) ERR;
   if (H5Dread(datasetid, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, 
	       xfer_plistid, uchar_data_in) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;

   /* This should have produced 2 overflows. */
   if (overflow_count != 2) ERR;
   overflow_count = 0;

   for (i=0; i<DIM1_LEN; i++)
      if (uchar_data_in[i] != (unsigned char)float_data_out[i]) ERR;

   SUMMARIZE_ERR;
   overflow_count = 0;

   /* Now write three out of range doubles to the int. This should
    * generate three exceptions. */
   printf("*** writing out of range doubles to int dataset...");
   if ((datasetid = H5Dopen1(grpid, INT_VAR_NAME)) < 0) ERR;
   if (H5Dwrite(datasetid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	       xfer_plistid, double_data_out2) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;

   /* This should cause three overflows. */
   if (overflow_count != 3) ERR;
   SUMMARIZE_ERR;
   overflow_count = 0;

   /* Now write three out of range floats to the int. */
   printf("*** writing out of range floats to int dataset...");
   if ((datasetid = H5Dopen1(grpid, INT_VAR_NAME)) < 0) ERR;
   if (H5Dwrite(datasetid, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, 
	       xfer_plistid, float_data_out2) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;

   /* This should cause three overflow. */
   if (overflow_count != 3) ERR;
   overflow_count = 0;

   /* Reread the data and display it. */
   if ((datasetid = H5Dopen1(grpid, INT_VAR_NAME)) < 0) ERR;
   if (H5Dread(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, int_data_in) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;
   
   SUMMARIZE_ERR;
   overflow_count = 0;

   /* Now try some doubles. */
   printf("*** writing doubles...");
   if ((datasetid = H5Dopen1(grpid, DOUBLE_VAR_NAME)) < 0) ERR;
   if (H5Dwrite(datasetid, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	       xfer_plistid, double_data_out3) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;

   /* This should cause zero overflows. */
   if (overflow_count != 0) ERR;
   overflow_count = 0;

   /* Reread the data. */
   if ((datasetid = H5Dopen1(grpid, INT_VAR_NAME)) < 0) ERR;
   if (H5Dread(datasetid, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, 
	       H5P_DEFAULT, int_data_in) < 0) ERR;
   if (H5Dclose(datasetid) < 0) ERR;
   
   /* Close up shop. */
   if (H5Fclose(fileid) < 0 ||
       H5Gclose(grpid) < 0) ERR;

   SUMMARIZE_ERR;

   FINAL_RESULTS;
}

/* This function is called when there is an overflow in a read/write
 * operation. Pass the fill value in as user_data. */
H5T_conv_ret_t
except_func(int except_type, hid_t src_id, hid_t dst_id, 
	    void *src_buf, void *dst_buf, void *user_data)
{
   /*printf("except_func: except_type %d\n", except_type);*/

   if(except_type == H5T_CONV_EXCEPT_RANGE_HI ||
      except_type == H5T_CONV_EXCEPT_RANGE_LOW)
   {
      if (H5Tequal(dst_id, H5T_STD_I8BE) > 0 ||
	  H5Tequal(dst_id, H5T_STD_I8LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(signed char *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(signed char *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(signed char *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(signed char *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(signed char *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(signed char *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(signed char *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(signed char *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(signed char *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(signed char *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_U8BE) ||
	       H5Tequal(dst_id, H5T_STD_U8LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(unsigned char *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(unsigned char *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(unsigned char *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(unsigned char *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(unsigned char *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(unsigned char *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(unsigned char *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(unsigned char *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(unsigned char *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(unsigned char *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_I16BE) > 0 ||
	       H5Tequal(dst_id, H5T_STD_I16LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(short *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(short *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(short *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(short *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(short *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(short *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(short *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(short *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(short *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(short *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_U16BE) ||
	       H5Tequal(dst_id, H5T_STD_U16LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(unsigned short *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(unsigned short *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(unsigned short *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(unsigned short *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(unsigned short *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(unsigned short *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(unsigned short *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(unsigned short *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(unsigned short *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(unsigned short *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_I32BE) > 0 ||
	       H5Tequal(dst_id, H5T_STD_I32LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(int *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(int *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(int *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(int *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(int *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(int *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(int *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(int *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(int *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(int *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_U32BE) ||
	       H5Tequal(dst_id, H5T_STD_U32LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(unsigned int *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(unsigned int *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(unsigned int *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(unsigned int *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(unsigned int *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(unsigned int *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(unsigned int *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(unsigned int *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(unsigned int *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(unsigned int *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_I64BE) > 0 ||
	       H5Tequal(dst_id, H5T_STD_I64LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(signed long long *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(signed long long *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(signed long long *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(signed long long *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(signed long long *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(signed long long *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(signed long long *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(signed long long *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(signed long long *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(signed long long *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_STD_U64BE) ||
	       H5Tequal(dst_id, H5T_STD_U64LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(unsigned long long *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(unsigned long long *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(unsigned long long *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(unsigned long long *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(unsigned long long *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(unsigned long long *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(unsigned long long *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(unsigned long long *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(unsigned long long *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(unsigned long long *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_IEEE_F32BE) > 0 ||
	       H5Tequal(dst_id, H5T_IEEE_F32LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(float *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(float *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(float *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(float *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(float *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(float *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(float *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(float *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(float *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(float *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }	 
      }
      else if (H5Tequal(dst_id, H5T_IEEE_F64BE) ||
	       H5Tequal(dst_id, H5T_IEEE_F64LE) > 0)
      {
	 if (H5Tequal(src_id, H5T_STD_I8BE) > 0 ||
	     H5Tequal(src_id, H5T_STD_I8LE) > 0)
	    *(double *)dst_buf = *(signed char *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U8BE) ||
		  H5Tequal(src_id, H5T_STD_U8LE) > 0)
	    *(double *)dst_buf = (unsigned char)*(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I16BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I16LE) > 0)
	    *(double *)dst_buf = *(signed short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U16BE) ||
		  H5Tequal(src_id, H5T_STD_U16LE) > 0)
	    *(double *)dst_buf = *(unsigned short *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I32BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I32LE) > 0)
	    *(double *)dst_buf = *(signed int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U32BE) ||
		  H5Tequal(src_id, H5T_STD_U32LE) > 0)
	    *(double *)dst_buf = *(unsigned int *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_I64BE) > 0 ||
		  H5Tequal(src_id, H5T_STD_I64LE) > 0)
	    *(double *)dst_buf = *(signed long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_STD_U64BE) ||
		  H5Tequal(src_id, H5T_STD_U64LE) > 0)
	    *(double *)dst_buf = *(unsigned long long *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F32BE) > 0 ||
		  H5Tequal(src_id, H5T_IEEE_F32LE) > 0)
	    *(double *)dst_buf = *(float *)src_buf;
	 else if (H5Tequal(src_id, H5T_IEEE_F64BE) ||
		  H5Tequal(src_id, H5T_IEEE_F64LE) > 0)
	    *(double *)dst_buf = *(double *)src_buf;
	 else
	 {
	    printf("Unknown type!\n");
	    return H5T_CONV_UNHANDLED;
	 }
      }
      else
      {
	 printf("Unknown type!\n");
	 return H5T_CONV_UNHANDLED;
      }	 

      (*(int *)user_data)++;
      return H5T_CONV_HANDLED;
   }

   return H5T_CONV_UNHANDLED;
}

