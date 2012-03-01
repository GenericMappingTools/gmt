/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 cross platform compound type.

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_xplatform.nc"

/* This file comes from the ncdump directory, where it is also used
 * for testing. */
#define IN_FILE_NAME_3 "ref_tst_compounds3.nc"
#define IN_FILE_NAME_4 "ref_tst_compounds4.nc"

#define CMP_TYPE_NAME "cmp_t"
#define X_NAME "x"
#define Y_NAME "y"

struct s1
{
      float x;
      double y;
};

int
main(int argc, char **argv)
{
   printf("\n*** Testing cross-platform compound data.\n");
   printf("*** testing define of struct s1 compound type...");
   {
      int ncid, ntypes_in, ndims_in;
      char name_in[NC_MAX_NAME + 1];
      size_t size_in, nfields_in, offset_in;
      nc_type field_type_in, cmp_typeid;

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1), CMP_TYPE_NAME, &cmp_typeid)) ERR;
      if (nc_insert_compound(ncid, cmp_typeid, X_NAME, 
          NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, cmp_typeid, Y_NAME, 
          NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;
      if (nc_close(ncid)) ERR;

      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq_typeids(ncid, &ntypes_in, &cmp_typeid)) ERR;
      if (ntypes_in != 1) ERR;
      if (nc_inq_compound(ncid, cmp_typeid, name_in, &size_in, &nfields_in)) ERR;
      if (nfields_in != 2 || strcmp(name_in, CMP_TYPE_NAME) || size_in != sizeof(struct s1)) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid, 0, name_in, &offset_in, 
          &field_type_in, &ndims_in, NULL)) ERR;
      if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT || 
          ndims_in) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid, 1, name_in, &offset_in, 
          &field_type_in, &ndims_in, NULL)) ERR;
      if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || 
          field_type_in != NC_DOUBLE || ndims_in) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** reading ref_tst_compound3.nc...");
   {
     /* Here is the file:
     netcdf ref_tst_compounds3 {
     types:
       compound cmp_t {
         float x ;
         double y ;
       }; // cmp_t
     dimensions:
     	n = 1 ;
     variables:
     	cmp_t var(n) ;
     data:
      var = {1, -2} ;
     }
      */
      
      int ncid, cmp_typeid, ntypes_in, ndims_in, dimids_in[1], natts_in;
      int nvars_in, unlimdimid;
      char name_in[NC_MAX_NAME + 1];
      size_t size_in, nfields_in, offset_in;
      nc_type field_type_in;
      struct s1 data_in[1];
      char file_in[NC_MAX_NAME + 1];

      strcpy(file_in, "");
      if (getenv("srcdir"))
      {
	 strcat(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
      } 
      strcat(file_in, "../ncdump/");
      strcat(file_in, IN_FILE_NAME_3);

      /* Check it out. But this version of the type was created on a
       * Sun, so has different size and offsets. */
      if (nc_open(file_in, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid)) ERR;
      if (ndims_in != 1 || nvars_in != 1 || natts_in != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, name_in, &cmp_typeid, &ndims_in, dimids_in, &natts_in)) ERR;
      if (strcmp(name_in, "var") || ndims_in != 1 || natts_in) ERR;

      /* Learn about the compound type. */
      if (nc_inq_typeids(ncid, &ntypes_in, &cmp_typeid)) ERR;
      if (ntypes_in != 1) ERR;
      if (nc_inq_compound(ncid, cmp_typeid, name_in, &size_in, &nfields_in)) ERR;
      if (nfields_in != 2 || strcmp(name_in, CMP_TYPE_NAME) || size_in != sizeof(struct s1)) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid, 0, name_in, &offset_in, 
          &field_type_in, &ndims_in, NULL)) ERR;
      if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT || 
          ndims_in) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid, 1, name_in, &offset_in, 
          &field_type_in, &ndims_in, NULL)) ERR;
      if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || field_type_in != NC_DOUBLE || 
          ndims_in) ERR;

      /* Read the data. */
      if (nc_get_var(ncid, 0, data_in)) ERR;
      if (data_in[0].x != 1.0 || data_in[0].y != -2.0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** reading ref_tst_compound4.nc...");
   {
     /* Here is the file:
        netcdf ref_tst_compounds4 {
        types:
          compound c {
            float x ;
            double y ;
          }; // c
          compound d {
            c s1 ;
          }; // d

        // global attributes:
        		d :a1 = {{1, -2}} ;
        }
      */
#define NUM_TYPES 2
#define CMP_TYPE_NAME_C "c"
#define CMP_TYPE_NAME_D "d"
#define ATT_NAME "a1"
#define S1_NAME "s1"
      int ncid, cmp_typeid[NUM_TYPES], ntypes_in, ndims_in, natts_in;
      int nvars_in, unlimdimid;
      char name_in[NC_MAX_NAME + 1];
      size_t size_in, nfields_in, offset_in;
      nc_type field_type_in;
      struct s2
      {
	    struct s1 s1;
      };
      struct s2 data_in[1];
      char file_in[NC_MAX_NAME + 1];

      strcpy(file_in, "");
      if (getenv("srcdir"))
      {
	 strcat(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
      } 
      strcat(file_in, "../ncdump/");
      strcat(file_in, IN_FILE_NAME_4);

      /* Check it out. But this version of the type was created on a
       * Sun, so has different size and offsets. */
      if (nc_open(file_in, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdimid)) ERR;
      if (ndims_in != 0 || nvars_in != 0 || natts_in != 1 || unlimdimid != -1) ERR;

      /* Learn about the compound type. */
      if (nc_inq_typeids(ncid, &ntypes_in, cmp_typeid)) ERR;
      if (ntypes_in != 2) ERR;

      /* Check the inner type. */
      if (nc_inq_compound(ncid, cmp_typeid[0], name_in, &size_in, &nfields_in)) ERR;
      if (nfields_in != 2 || strcmp(name_in, CMP_TYPE_NAME_C) || size_in != sizeof(struct s1)) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid[0], 0, name_in, &offset_in,
          &field_type_in, &ndims_in, NULL)) ERR;
      if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT ||
          ndims_in) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid[0], 1, name_in, &offset_in,
          &field_type_in, &ndims_in, NULL)) ERR;
      if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || field_type_in != NC_DOUBLE ||
          ndims_in) ERR;

      /* Check the outer type. */
      if (nc_inq_compound(ncid, cmp_typeid[1], name_in, &size_in, &nfields_in)) ERR;
      if (nfields_in != 1 || strcmp(name_in, CMP_TYPE_NAME_D) || size_in != sizeof(struct s2)) ERR;
      if (nc_inq_compound_field(ncid, cmp_typeid[1], 0, name_in, &offset_in,
          &field_type_in, &ndims_in, NULL)) ERR;
/*      if (strcmp(name_in, S1_NAME) || offset_in != 0 || field_type_in != cmp_typeid[0] || ndims_in) ERR;*/

      /* Read the data. */
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      if (data_in[0].s1.x != 1.0 || data_in[0].s1.y != -2.0) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

