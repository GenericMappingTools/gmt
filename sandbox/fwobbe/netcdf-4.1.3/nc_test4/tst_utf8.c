/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which writes a netCDF file with
   Unicode names encoded with UTF-8. It is the NETCDF3 equivalent
   of tst_unicode.c

   $Id$
*/

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <string.h>

/* The data file we will create. */
#define FILE_NAME "tst_utf8.nc"
#define NDIMS 1
#define NX 18
#define ENUM_VALUE 2

/* (unnormalized) UTF-8 encoding for Unicode 8-character "Hello" in Greek */
char name_utf8[] = "\xCE\x9A\xCE\xB1\xCE\xBB\xCE\xB7\xCE\xBC\xE1\xBD\xB3\xCF\x81\xCE\xB1";

/* NFC normalized UTF-8 for Unicode 8-character "Hello" in Greek */
char norm_utf8[] = "\xCE\x9A\xCE\xB1\xCE\xBB\xCE\xB7\xCE\xBC\xCE\xAD\xCF\x81\xCE\xB1";

/* This is the struct for the compound type. */
struct comp {
      int i;
};

/* Given an ncid, check the file to make sure it has all the objects I
 * expect. */
int
check_nc4_file(int ncid)
{
   int varid, dimid, attnum, grpid, grpid2, grpid3, numgrps;
   int numtypes, enum_typeid, comp_typeid;
   int class_in;
   size_t att_len, size_in, num_mem, nfields_in;
   nc_type att_type, base_type_in;
   char name_in[NC_MAX_NAME + 1], strings_in[NC_MAX_NAME + 1], value;

   /* Check the group. */
   if (nc_inq_grps(ncid, &numgrps, &grpid)) ERR;   
   if (numgrps != 1) ERR;
   name_in[0] = 0;
   if (nc_inq_grpname(grpid, name_in)) ERR;   
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;

   /* Check the variable. */
   if (nc_inq_varid(grpid, name_utf8, &varid)) ERR;
   if (nc_inq_varname(grpid, varid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   if (nc_inq_varid(grpid, norm_utf8, &varid)) ERR;
   name_in[0] = 0;
   if (nc_inq_varname(grpid, varid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   if (nc_get_var(grpid, varid, strings_in)) ERR;
   if (strncmp(name_utf8, strings_in, sizeof(name_utf8))) ERR;
   strings_in[0] = '\0'; /* Reset my string buffer. */
   
   /* Check the dimension. */
   if (nc_inq_dimid(grpid, name_utf8, &dimid)) ERR;
   if (nc_inq_dimname(grpid, dimid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   if (nc_inq_dimid(grpid, norm_utf8, &dimid)) ERR;
   if (nc_inq_dimname(grpid, dimid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;

   /* Check the attribute.  We don't normalize data or attribute
    * values, so get exactly what was put for the value, but
    * normalized values for names. */
   if (nc_inq_attid(grpid, varid, norm_utf8, &attnum)) ERR;
   if (attnum) ERR;
   attnum = 99; /* Reset. */
   if (nc_inq_attid(grpid, varid, name_utf8, &attnum)) ERR;
   if (attnum) ERR;
   if (nc_inq_att(grpid, varid, norm_utf8, &att_type, &att_len)) ERR;
   if (att_type != NC_CHAR || att_len != sizeof(name_utf8)) ERR;
   if (nc_get_att_text(grpid, varid, norm_utf8, strings_in)) ERR;
   if (strncmp(name_utf8, strings_in, sizeof(name_utf8))) ERR;

   /* Check the enum type. */
   if (nc_inq_grps(grpid, &numgrps, &grpid2)) ERR;   
   if (numgrps != 1) ERR;
   if (nc_inq_typeids(grpid2, &numtypes, &enum_typeid)) ERR;
   if (numtypes != 1) ERR;
   if (nc_inq_user_type(grpid2, enum_typeid, name_in, &size_in, &base_type_in, 
			       &nfields_in, &class_in)) ERR;
   if (strncmp(norm_utf8, name_in, strlen(norm_utf8)) || size_in != 1 || 
       base_type_in != NC_BYTE || nfields_in != 1 || class_in != NC_ENUM) ERR;
   name_in[0] = size_in = base_type_in = 0;
   if (nc_inq_enum(grpid2, enum_typeid, name_in, &base_type_in, &size_in, &num_mem)) ERR;
   if (strncmp(norm_utf8, name_in, strlen(norm_utf8)) || size_in != 1 || 
       base_type_in != NC_BYTE || num_mem != 1) ERR;
   if (nc_inq_enum_member(grpid2, enum_typeid, 0, name_in, &value)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8)) || value != ENUM_VALUE) ERR;

   /* Check the compound type. */
   if (nc_inq_grps(grpid2, &numgrps, &grpid3)) ERR;   
   if (numgrps != 1) ERR;
   if (nc_inq_typeids(grpid3, &numtypes, &comp_typeid)) ERR;
   if (numtypes != 1) ERR;
   name_in[0] = 0;
   if (nc_inq_user_type(grpid3, comp_typeid, name_in, &size_in, &base_type_in, 
			&nfields_in, &class_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8)) || size_in != sizeof(struct comp) || 
       base_type_in != NC_NAT || nfields_in != 1 || class_in != NC_COMPOUND) ERR;
   size_in = nfields_in = 999;
   if (nc_inq_compound(grpid3, comp_typeid, name_in, &size_in, &nfields_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8)) || size_in != sizeof(struct comp) || 
       nfields_in != 1) ERR;
   name_in[0] = 0;
   if (nc_inq_compound_fieldname(grpid3, comp_typeid, 0, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   return NC_NOERR;
}

/* Given an ncid, check the file to make sure it has all the objects I
 * expect. */
int
check_classic_file(int ncid)
{
   int varid, dimid, attnum;
   size_t att_len;
   nc_type att_type;
   char name_in[sizeof(name_utf8) + 1], strings_in[sizeof(name_utf8) + 1];

   /* Check the variable. */
   if (nc_inq_varid(ncid, name_utf8, &varid)) ERR;
   if (nc_inq_varname(ncid, varid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   if (nc_inq_varid(ncid, norm_utf8, &varid)) ERR;
   name_in[0] = 0;
   if (nc_inq_varname(ncid, varid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   if (nc_get_var_text(ncid, varid, strings_in)) ERR;
   if (strncmp(name_utf8, strings_in, sizeof(name_utf8))) ERR;
   strings_in[0] = '\0'; /* Reset my string buffer. */
   
   /* Check the dimension. */
   if (nc_inq_dimid(ncid, name_utf8, &dimid)) ERR;
   if (nc_inq_dimname(ncid, dimid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;
   if (nc_inq_dimid(ncid, norm_utf8, &dimid)) ERR;
   if (nc_inq_dimname(ncid, dimid, name_in)) ERR;
   if (strncmp(norm_utf8, name_in, sizeof(norm_utf8))) ERR;

   /* Check the attribute.  We don't normalize data or attribute
    * values, so get exactly what was put for the value, but
    * normalized values for names. */
   if (nc_inq_attid(ncid, varid, norm_utf8, &attnum)) ERR;
   if (attnum) ERR;
   attnum = 99; /* Reset. */
   if (nc_inq_attid(ncid, varid, name_utf8, &attnum)) ERR;
   if (attnum) ERR;
   if (nc_inq_att(ncid, varid, norm_utf8, &att_type, &att_len)) ERR;
   if (att_type != NC_CHAR || att_len != sizeof(name_utf8)) ERR;
   if (nc_get_att_text(ncid, varid, norm_utf8, strings_in)) ERR;
   if (strncmp(name_utf8, strings_in, sizeof(name_utf8))) ERR;
   return NC_NOERR;
}

int
main(int argc, char **argv)
{
   printf("\n*** Testing UTF-8 names.\n");
   printf("*** creating UTF-8 names in classic model netcdf files...");
   {
      int ncid, varid, dimids[NDIMS];
      int f;

      for (f = NC_FORMAT_CLASSIC; f < NC_FORMAT_NETCDF4_CLASSIC; f++)
      {
	 if (nc_set_default_format(f, NULL)) ERR;
	 if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
      
	 /* Define various netcdf objects with a Unicode UTF-8 encoded name
	  * that must be normalized. Where possible, also use the utf8
	  * string as the value. The name will be normalized, but not the
	  * value. */
	 if (nc_def_dim(ncid, name_utf8, NX, &dimids[0])) ERR;
	 if (nc_def_var(ncid, name_utf8, NC_CHAR, NDIMS, dimids, &varid)) ERR;
	 if (nc_put_att_text(ncid, varid, name_utf8, sizeof(name_utf8), name_utf8)) ERR;
      
	 if (nc_enddef(ncid)) ERR;
      
	 /* Write var data. */
	 if (nc_put_var_text(ncid, varid, name_utf8)) ERR;
      
	 /* Check the file. */
	 check_classic_file(ncid);
      
	 if (nc_close(ncid)) ERR;
      
	 /* Reopen the file and check again. */
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	 check_classic_file(ncid);
	 if (nc_close(ncid)) ERR;
      } /* next format */
   }
   SUMMARIZE_ERR;

#define DIM1_NAME "d1"
#define VAR1_NAME "v1"
#define ATT1_NAME "a1"

   printf("*** renaming to UTF-8 names in classic model netcdf files...");
   {
      int ncid, varid, dimids[NDIMS];
      int f;

      for (f = NC_FORMAT_CLASSIC; f < NC_FORMAT_NETCDF4_CLASSIC; f++)
      {
	 if (nc_set_default_format(f, NULL)) ERR;
	 if (nc_create(FILE_NAME, NC_CLOBBER, &ncid)) ERR;
      
	 /* Create objects. */
	 if (nc_def_dim(ncid, DIM1_NAME, NX, &dimids[0])) ERR;
	 if (nc_rename_dim(ncid, 0, name_utf8)) ERR;
 	 if (nc_def_var(ncid, name_utf8, NC_CHAR, NDIMS, dimids, &varid)) ERR; 
 	 if (nc_put_att_text(ncid, varid, ATT1_NAME, sizeof(name_utf8), name_utf8)) ERR; 
 	 if (nc_rename_att(ncid, 0, ATT1_NAME, name_utf8)) ERR; 
      
	 if (nc_enddef(ncid)) ERR;
      
	 /* Write var data. */
	 if (nc_put_var_text(ncid, varid, name_utf8)) ERR;
      
	 /* Check the file. */
	 check_classic_file(ncid);
      
	 if (nc_close(ncid)) ERR;
      
	 /* Reopen the file and check again. */
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	 check_classic_file(ncid);
	 if (nc_close(ncid)) ERR;
      } /* next format */
   }
   SUMMARIZE_ERR;

   printf("*** creating UTF-8 names in netcdf-4 file...");
   {
      int ncid, varid, grpid, comp_typeid, enum_typeid, grpid2, grpid3;
      int dimids[NDIMS];
      char my_int = ENUM_VALUE;
      
      if (nc_create(FILE_NAME, NC_NETCDF4 | NC_CLOBBER, &ncid)) ERR;
      
      /* Define various netcdf objects with a Unicode UTF-8 encoded name
       * that must be normalized. Where possible, also use the utf8
       * string as the value. The name will be normalized, but not the
       * value. */
      if (nc_def_grp(ncid, name_utf8, &grpid)) ERR;
      if (nc_def_dim(grpid, name_utf8, NX, &dimids[0])) ERR;
      if (nc_def_var(grpid, name_utf8, NC_CHAR, NDIMS, dimids, &varid)) ERR;
      if (nc_put_att_text(grpid, varid, name_utf8, sizeof(name_utf8), name_utf8)) ERR;
      
      if (nc_def_grp(grpid, "tmp", &grpid2)) ERR;
      if (nc_def_enum(grpid2, NC_BYTE, name_utf8, &enum_typeid)) ERR;
      if (nc_insert_enum(grpid2, enum_typeid, name_utf8, &my_int)) ERR;
      
      if (nc_def_grp(grpid2, "tmp", &grpid3)) ERR;
      if (nc_def_compound(grpid3, sizeof(struct comp), name_utf8, &comp_typeid)) ERR;
      if (nc_insert_compound(grpid3, comp_typeid, name_utf8, offsetof(struct comp, i), NC_INT)) ERR;
      
      /* Write var data. */
      if (nc_put_var_text(grpid, varid, name_utf8)) ERR;
      
      /* Check the file. */
      check_nc4_file(ncid);
      
      if (nc_close(ncid)) ERR;
      
      /* Reopen the file and check again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      check_nc4_file(ncid);
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
