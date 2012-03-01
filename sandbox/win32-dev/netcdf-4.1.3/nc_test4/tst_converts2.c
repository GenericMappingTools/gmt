/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test even more data conversions.

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_converts2.nc"
#define VAR_NAME "Monkey"
#define DIM_NAME "n"
#define DIM_LEN 5

int
main(int argc, char **argv)
{
   int ncid, varid;
   signed char schar_in, schar = -2;
   char var_name[NC_MAX_NAME+1];
   int ndims, natts, int_in;
   long long_in;
   nc_type var_type;
 
   printf("\n*** Testing more netcdf-4 data conversion.\n");
   printf ("*** Testing NC_BYTE conversions...");
   {
      /* Write a scalar NC_BYTE with value -2. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_BYTE, 0, NULL, &varid)) ERR;
      if (nc_put_var_schar(ncid, varid, &schar)) ERR;
      if (nc_close(ncid)) ERR;

      /* Now open the file and check it. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, NULL, &natts)) ERR;
      if (strcmp(var_name, VAR_NAME) || natts !=0 || ndims != 0 || 
	  var_type != NC_BYTE) ERR;
      if (nc_get_var_schar(ncid, varid, &schar_in)) ERR;
      if (schar_in != schar) ERR;
      if (nc_get_var_int(ncid, varid, &int_in)) ERR;
      if (int_in != schar) ERR;
      if (nc_get_var_long(ncid, varid, &long_in)) ERR;
      if (long_in != schar) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf ("*** Testing NC_USHORT conversions...");
   {
      /* Write a scalar NC_USHORT, converted from various types, testing a bug fix. */
       unsigned short usval = 65535;
       int ival = 65535;
       long lval = 65535;
       float fval = 65535;
       double dval = 65535;
       int dimid;
       size_t coord[1];
       unsigned short ushort_in;
       int int_in;
       long long_in;
       float float_in;
       double double_in;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_USHORT, 1, &dimid, &varid)) ERR;
      coord[0] = 0;
      if (nc_put_var1_ushort(ncid, varid, coord, &usval)) ERR;
      coord[0] = 1;
      if (nc_put_var1_int(ncid, varid, coord, &ival)) ERR;
      coord[0] = 2;
      if (nc_put_var1_float(ncid, varid, coord, &fval)) ERR;
      coord[0] = 3;
      if (nc_put_var1_double(ncid, varid, coord, &dval)) ERR; 
      coord[0] = 4;
      if (nc_put_var1_long(ncid, varid, coord, &lval)) ERR; 

      if (nc_close(ncid)) ERR;

      /* Now open the file and check it. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, NULL, &natts)) ERR;
      if (strcmp(var_name, VAR_NAME) || natts !=0 || ndims != 1 || 
	  var_type != NC_USHORT) ERR;
      coord[0] = 0;
      if (nc_get_var1_ushort(ncid, varid, coord, &ushort_in)) ERR;
      if (ushort_in != usval) ERR;
      coord[0] = 1;
      if (nc_get_var1_int(ncid, varid, coord, &int_in)) ERR;
      if (int_in != ival) ERR;
      coord[0] = 2;
      if (nc_get_var1_float(ncid, varid, coord, &float_in)) ERR;
      if (float_in != fval) ERR;
      coord[0] = 3;
      if (nc_get_var1_double(ncid, varid, coord, &double_in)) ERR;
      if (double_in != dval) ERR;
      coord[0] = 4;
      if (nc_get_var1_long(ncid, varid, coord, &long_in)) ERR;
      if (long_in != lval) ERR;

      /* This should fail. */
      coord[3] = 100;
      if (nc_get_var1_ushort(ncid, varid, &coord[3], 
			     &ushort_in) != NC_EINVALCOORDS) ERR;

      if (nc_close(ncid)) ERR;
   }
   {
      /* Write a scalar NC_USHORT that's not default fill value,
       * converted from various types. */
       unsigned short usval = 65534;
       int ival = 65534;
       long lval = 65534;
       float fval = 65534;
       double dval = 65534;
       int dimid;
       size_t coord[1];
       unsigned short ushort_in;
       int int_in;
       long long_in;
       float float_in;
       double double_in;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_USHORT, 1, &dimid, &varid)) ERR;
      coord[0] = 0;
      if (nc_put_var1_ushort(ncid, varid, coord, &usval)) ERR;
      coord[0] = 1;
      if (nc_put_var1_int(ncid, varid, coord, &ival)) ERR;
      coord[0] = 2;
      if (nc_put_var1_float(ncid, varid, coord, &fval)) ERR;
      coord[0] = 3;
      if (nc_put_var1_double(ncid, varid, coord, &dval)) ERR; 
      coord[0] = 4;
      if (nc_put_var1_long(ncid, varid, coord, &lval)) ERR; 

      if (nc_close(ncid)) ERR;

      /* Now open the file and check it. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, NULL, &natts)) ERR;
      if (strcmp(var_name, VAR_NAME) || natts !=0 || ndims != 1 || 
	  var_type != NC_USHORT) ERR;
      coord[0] = 0;
      if (nc_get_var1_ushort(ncid, varid, coord, &ushort_in)) ERR;
      if (ushort_in != usval) ERR;
      coord[0] = 1;
      if (nc_get_var1_int(ncid, varid, coord, &int_in)) ERR;
      if (int_in != ival) ERR;
      coord[0] = 2;
      if (nc_get_var1_float(ncid, varid, coord, &float_in)) ERR;
      if (float_in != fval) ERR;
      coord[0] = 3;
      if (nc_get_var1_double(ncid, varid, coord, &double_in)) ERR;
      if (double_in != dval) ERR;
      coord[0] = 4;
      if (nc_get_var1_long(ncid, varid, coord, &long_in)) ERR;
      if (long_in != lval) ERR;

      /* This should fail. */
      coord[3] = 100;
      if (nc_get_var1_ushort(ncid, varid, &coord[3], 
			     &ushort_in) != NC_EINVALCOORDS) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf ("*** Testing MAX_INT conversions...");
   {
#define X_INT_MAX	2147483647 /* from libsrc4/nc4internal.h */
      int ivalue = X_INT_MAX, ivalue_in;
      unsigned char uchar_in;
      unsigned int uivalue_in;
      short svalue_in;
      unsigned short usvalue_in;
      long long int64_in;
      unsigned long long uint64_in;
      double double_in;
      
      /* Write a scalar NC_INT with value X_MAX_INT. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, 0, NULL, &varid)) ERR;
      if (nc_put_var_int(ncid, varid, &ivalue)) ERR;
      if (nc_close(ncid)) ERR;

      /* Now open the file and check it. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, NULL, &natts)) ERR;
      if (strcmp(var_name, VAR_NAME) || natts !=0 || ndims != 0 || 
	  var_type != NC_INT) ERR;
      if (nc_get_var_schar(ncid, varid, &schar_in) != NC_ERANGE) ERR;
      if (schar_in != (signed char)ivalue) ERR;
      if (nc_get_var_uchar(ncid, varid, &uchar_in) != NC_ERANGE) ERR;
      if (uchar_in != (unsigned char)ivalue) ERR;
      if (nc_get_var_short(ncid, varid, &svalue_in) != NC_ERANGE) ERR;
      if (svalue_in != (short)ivalue) ERR;
      if (nc_get_var_ushort(ncid, varid, &usvalue_in) != NC_ERANGE) ERR;
      if (usvalue_in != (unsigned short)ivalue) ERR;
      if (nc_get_var_int(ncid, varid, &ivalue_in)) ERR;
      if (ivalue_in != ivalue) ERR;
      if (nc_get_var_uint(ncid, varid, &uivalue_in)) ERR;
      if (uivalue_in != (unsigned int)ivalue) ERR;
      if (nc_get_var_long(ncid, varid, &long_in)) ERR;
      if (long_in != ivalue) ERR;
      if (nc_get_var_longlong(ncid, varid, &int64_in)) ERR;
      if (int64_in != ivalue) ERR;
      if (nc_get_var_ulonglong(ncid, varid, &uint64_in)) ERR;
      if (uint64_in != ivalue) ERR;
/*      if (nc_get_var_float(ncid, varid, &float_in)) ERR;
      f2 = (float)ivalue; 
      if (float_in != f2) ERR;*/
      if (nc_get_var_double(ncid, varid, &double_in)) ERR;
      if (double_in != (double)ivalue) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

