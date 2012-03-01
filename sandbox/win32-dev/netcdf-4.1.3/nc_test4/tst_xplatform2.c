/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 cross platform compound type.

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME_1 "tst_xplatform2_1.nc"
#define REF_FILE_NAME_1 "ref_tst_xplatform2_1.nc"
#define FILE_NAME_2 "tst_xplatform2_2.nc"
#define REF_FILE_NAME_2 "ref_tst_xplatform2_2.nc"
#define FILE_NAME_3 "tst_xplatform2_3.nc"
#define REF_FILE_NAME_3 "ref_tst_xplatform2_3.nc"
#define FILE_NAME_4 "tst_xplatform2_4.nc"
#define REF_FILE_NAME_4 "ref_tst_xplatform2_4.nc"

#define S1_TYPE_NAME "cmp_t"
#define X_NAME "x"
#define Y_NAME "y"
#define S1_NAME "s1"
#define S2_ATT_NAME "Runnymede"
#define S2_TYPE_NAME "date_1215"

#define DIM1_LEN 5
#define DIM2_LEN 3
#define VLEN_NAME "Magna_Carta_VLEN"      
#define VLEN_ATT_NAME "We_will_sell_to_no_man_we_will_not_deny_or_defer_to_any_man_either_Justice_or_Right"
#define TWO_TYPES 2
#define NUM_S1 4    

#define DIM3_LEN 1
#define DIM3_NAME "DIMENSION->The city of London shall enjoy all its ancient liberties and free customs, both by land and by water."
#define VAR3_NAME "VARIABLE->In future we will allow no one to levy an `aid' from his free men, except to ransom his person, to make his eldest son a knight, and (once) to marry his eldest daughter."

#define NUM_VL 1
#define S3_ATT_NAME "King_John"
#define S3_TYPE_NAME "barons"
#define VL_NAME "No scutage or aid may be levied in our kingdom without its general consent"      
#define THREE_TYPES 3

struct s1
{
   float x;
   double y;
};
struct s2
{
   struct s1 data[NUM_S1];
};
struct s3
{
   nc_vlen_t data[NUM_VL];
};

int
check_file_1(int ncid, nc_vlen_t *data_out)
{
   int ntypes_in, ndims_in;
   char name_in[NC_MAX_NAME + 1];
   size_t size_in, nfields_in, offset_in;
   nc_type field_type_in;
   nc_type typeids_in[TWO_TYPES], base_nc_type_in;
   nc_vlen_t data_in[DIM1_LEN];
   int i, j;

   /* There should be two types. */
   if (nc_inq_typeids(ncid, &ntypes_in, typeids_in)) ERR;
   if (ntypes_in != 2) ERR;

   /* The compound type is first: check it out. */
   if (nc_inq_compound(ncid, typeids_in[0], name_in, &size_in, &nfields_in)) ERR;
   if (nfields_in != 2 || strcmp(name_in, S1_TYPE_NAME) || size_in != sizeof(struct s1)) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 0, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT ||
       ndims_in) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 1, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || field_type_in != NC_DOUBLE ||
       ndims_in) ERR;

   /* How does the vlen type look? */
   if (nc_inq_vlen(ncid, typeids_in[1], name_in, &size_in, &base_nc_type_in)) ERR;
   if (strcmp(name_in, VLEN_NAME) || size_in != sizeof(nc_vlen_t) || 
       base_nc_type_in != typeids_in[0]) ERR;

   /* Now read the attribute. */
   if (nc_get_att(ncid, NC_GLOBAL, VLEN_ATT_NAME, data_in)) ERR;

   /* Did we get the correct data? */
   for (i = 0; i < DIM1_LEN; i++)
   {
      if (data_in[i].len != data_out[i].len) ERR;
      for (j = 0; j < data_in[i].len; j++)
	 if (((struct s1 *)data_in[i].p)->x != ((struct s1 *)data_out[i].p)->x ||
	     ((struct s1 *)data_in[i].p)->y != ((struct s1 *)data_out[i].p)->y) ERR_RET;
   }

   /* Free the memory that was malloced when the VLEN was read. */
   for (i = 0; i < DIM1_LEN; i++)
      free(data_in[i].p);

   /* We're done! */
   return NC_NOERR;
}
   
int
check_file_2(int ncid, struct s2 *data_out)
{
   int ntypes_in, ndims_in;
   char name_in[NC_MAX_NAME + 1];
   size_t size_in, nfields_in, offset_in;
   nc_type field_type_in;
   nc_type typeids_in[TWO_TYPES];
   struct s2 data_in[DIM2_LEN];
   int field_dims_in[1];
   int i, j;

   /* There should be two types. */
   if (nc_inq_typeids(ncid, &ntypes_in, typeids_in)) ERR;
   if (ntypes_in != TWO_TYPES) ERR;

   /* The compound type is first: check it out. */
   if (nc_inq_compound(ncid, typeids_in[0], name_in, &size_in, &nfields_in)) ERR;
   if (nfields_in != 2 || strcmp(name_in, S1_TYPE_NAME) || size_in != sizeof(struct s1)) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 0, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT ||
       ndims_in) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 1, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || field_type_in != NC_DOUBLE ||
       ndims_in) ERR;

   /* How does the containing compound type look? */
   if (nc_inq_compound(ncid, typeids_in[1], name_in, &size_in, &nfields_in)) ERR;
   if (strcmp(name_in, S2_TYPE_NAME) || size_in != sizeof(struct s2) || 
       nfields_in != 1) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[1], 0, name_in, &offset_in, &field_type_in, 
			     &ndims_in, field_dims_in)) ERR;
   if (strcmp(name_in, S1_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s2, data) ||
       field_type_in != typeids_in[0] || ndims_in != 1 || field_dims_in[0] != NUM_S1) ERR;

   /* Now read the attribute. */
   if (nc_get_att(ncid, NC_GLOBAL, S2_ATT_NAME, data_in)) ERR;

   /* Did we get the correct data? */
   for (i = 0; i < DIM2_LEN; i++)
      for (j = 0; j < NUM_S1; j++)
	 if (data_out[i].data[j].x != data_in[i].data[j].x ||
	     data_out[i].data[j].y != data_in[i].data[j].y) ERR;
   
   /* We're done! */
   return NC_NOERR;
}
   
int
check_file_3(int ncid, struct s3 *data_out)
{
   int ntypes_in, ndims_in;
   char name_in[NC_MAX_NAME + 1];
   size_t size_in, nfields_in, offset_in;
   nc_type field_type_in, base_nc_type_in;
   nc_type typeids_in[THREE_TYPES];
   struct s3 data_in[DIM3_LEN];
   int field_dims_in[1];
   int i, j, k;

   /* There should be three types. */
   if (nc_inq_typeids(ncid, &ntypes_in, typeids_in)) ERR;
   if (ntypes_in != THREE_TYPES) ERR;

   /* The s1 compound type is first: check it out. */
   if (nc_inq_compound(ncid, typeids_in[0], name_in, &size_in, &nfields_in)) ERR;
   if (nfields_in != 2 || strcmp(name_in, S1_TYPE_NAME) || size_in != sizeof(struct s1)) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 0, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT ||
       ndims_in) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 1, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || field_type_in != NC_DOUBLE ||
       ndims_in) ERR;

   /* How does the vlen type look? */
   if (nc_inq_vlen(ncid, typeids_in[1], name_in, &size_in, &base_nc_type_in)) ERR;
   if (strcmp(name_in, VLEN_NAME) || size_in != sizeof(nc_vlen_t) || 
       base_nc_type_in != typeids_in[0]) ERR;

   /* How does the containing compound type look? */
   if (nc_inq_compound(ncid, typeids_in[2], name_in, &size_in, &nfields_in)) ERR;
   if (strcmp(name_in, S3_TYPE_NAME) || size_in != sizeof(struct s3) || 
       nfields_in != 1) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[2], 0, name_in, &offset_in, &field_type_in, 
			     &ndims_in, field_dims_in)) ERR;
   if (strcmp(name_in, VL_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s3, data) ||
       field_type_in != typeids_in[1] || ndims_in != 1 || field_dims_in[0] != NUM_VL) ERR;

   /* Now read the attribute. */
   if (nc_get_att(ncid, NC_GLOBAL, S3_ATT_NAME, data_in)) ERR;

   /* Did we get the correct data? */
   for (i = 0; i < DIM3_LEN; i++)
      for (j = 0; j < NUM_VL; j++)
      {
	 if (data_in[i].data[j].len != data_in[i].data[j].len) ERR;
	 for (k = 0; k < data_out[i].data[j].len; k++)
	    if (((struct s1 *)data_in[i].data[j].p)[k].x != ((struct s1 *)data_out[i].data[j].p)[k].x ||
		((struct s1 *)data_in[i].data[j].p)[k].y != ((struct s1 *)data_out[i].data[j].p)[k].y) ERR;
      }

   /* Free our vlens. */
/*    for (i = 0; i < DIM3_LEN; i++) */
/*       for (j = 0; j < NUM_VL; j++) */
/* 	 nc_free_vlen(&(data_in[i].data[j])); */

   /* We're done! */
   return NC_NOERR;
}
   
int
check_file_4(int ncid, struct s3 *data_out)
{
   int ntypes_in, ndims_in;
   char name_in[NC_MAX_NAME + 1];
   size_t size_in, nfields_in, offset_in;
   nc_type field_type_in, base_nc_type_in;
   nc_type typeids_in[THREE_TYPES];
   struct s3 data_in[DIM3_LEN];
   int field_dims_in[1];
   int i, j, k;

   /* There should be three types. */
   if (nc_inq_typeids(ncid, &ntypes_in, typeids_in)) ERR;
   if (ntypes_in != THREE_TYPES) ERR;

   /* The s1 compound type is first: check it out. */
   if (nc_inq_compound(ncid, typeids_in[0], name_in, &size_in, &nfields_in)) ERR;
   if (nfields_in != 2 || strcmp(name_in, S1_TYPE_NAME) || size_in != sizeof(struct s1)) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 0, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, X_NAME) || offset_in != 0 || field_type_in != NC_FLOAT ||
       ndims_in) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[0], 1, name_in, &offset_in,
			     &field_type_in, &ndims_in, NULL)) ERR;
   if (strcmp(name_in, Y_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s1, y) || field_type_in != NC_DOUBLE ||
       ndims_in) ERR;

   /* How does the vlen type look? */
   if (nc_inq_vlen(ncid, typeids_in[1], name_in, &size_in, &base_nc_type_in)) ERR;
   if (strcmp(name_in, VLEN_NAME) || size_in != sizeof(nc_vlen_t) || 
       base_nc_type_in != typeids_in[0]) ERR;

   /* How does the containing compound type look? */
   if (nc_inq_compound(ncid, typeids_in[2], name_in, &size_in, &nfields_in)) ERR;
   if (strcmp(name_in, S3_TYPE_NAME) || size_in != sizeof(struct s3) || 
       nfields_in != 1) ERR;
   if (nc_inq_compound_field(ncid, typeids_in[2], 0, name_in, &offset_in, &field_type_in, 
			     &ndims_in, field_dims_in)) ERR;
   if (strcmp(name_in, VL_NAME) || offset_in != NC_COMPOUND_OFFSET(struct s3, data) ||
       field_type_in != typeids_in[1] || ndims_in != 1 || field_dims_in[0] != NUM_VL) ERR;

   /* Now read the variable. */
   if (nc_get_var(ncid, 0, data_in)) ERR;

   /* Did we get the correct data? */
   for (i = 0; i < DIM3_LEN; i++)
      for (j = 0; j < NUM_VL; j++)
      {
	 if (data_in[i].data[j].len != data_in[i].data[j].len) ERR;
	 for (k = 0; k < data_out[i].data[j].len; k++)
	    if (((struct s1 *)data_in[i].data[j].p)[k].x != ((struct s1 *)data_out[i].data[j].p)[k].x ||
		((struct s1 *)data_in[i].data[j].p)[k].y != ((struct s1 *)data_out[i].data[j].p)[k].y) ERR;
      }

   /* Free our vlens. */
/*    for (i = 0; i < DIM3_LEN; i++) */
/*       for (j = 0; j < NUM_VL; j++) */
/* 	 nc_free_vlen(&(data_in[i].data[j])); */

   /* We're done! */
   return NC_NOERR;
}
   
int
main(int argc, char **argv)
{
   int ncid;
   int i, j, k;
   nc_vlen_t *vlen_of_comp_out;
   struct s2 *comp_array_of_comp_out;
   struct s3 *comp_array_of_vlen_of_comp_out;
   char zero = 0;

   printf("\nTesting nested types across platforms.\n");

   if (!(vlen_of_comp_out = calloc(sizeof(nc_vlen_t), DIM1_LEN))) ERR;
   if (!(comp_array_of_comp_out = calloc(sizeof(struct s2), DIM2_LEN))) ERR;
   if (!(comp_array_of_vlen_of_comp_out = calloc(sizeof(struct s3), DIM3_LEN))) ERR;

   /* Create some output data: a vlen of struct s1. */
   for (i = 0; i < DIM1_LEN; i++)
   {
      vlen_of_comp_out[i].len = i + 1;
      if (!(vlen_of_comp_out[i].p = malloc(sizeof(struct s1) * vlen_of_comp_out[i].len)))
	 return NC_ENOMEM;
      for (j = 0; j < vlen_of_comp_out[i].len; j++)
      {
	 ((struct s1 *)vlen_of_comp_out[i].p)[j].x = 42.42;
	 ((struct s1 *)vlen_of_comp_out[i].p)[j].y = 2.0;
      }
   }

   /* Create some output data: a struct which holds an array of
    * struct s1. */
   for (i = 0; i < DIM2_LEN; i++)
      for (j = 0; j < NUM_S1; j++)
      {
	 comp_array_of_comp_out[i].data[j].x = 42.42;
	 comp_array_of_comp_out[i].data[j].y = 2.0;
      }

   /* Create some output data: a struct which holds an array of
    * vlen of struct s1. */
   for (i = 0; i < DIM3_LEN; i++)
      for (j = 0; j < NUM_VL; j++)
      {
	 comp_array_of_vlen_of_comp_out[i].data[j].len = i + 1;
	 if (!(comp_array_of_vlen_of_comp_out[i].data[j].p = malloc(sizeof(struct s1) * comp_array_of_vlen_of_comp_out[i].data[j].len)))
	    return NC_ENOMEM;
	 for (k = 0; k < comp_array_of_vlen_of_comp_out[i].data[j].len; k++)
	 {
	    ((struct s1 *)comp_array_of_vlen_of_comp_out[i].data[j].p)[k].x = 42.42;
	    ((struct s1 *)comp_array_of_vlen_of_comp_out[i].data[j].p)[k].y = 2.0;
	 }
      }


   printf("*** testing of vlen of compound type...");
   {
      nc_type s1_typeid, vlen_typeid;

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME_1, NC_NETCDF4, &ncid)) ERR;

      /* Create a simple compound type which has different sizes on
       * different platforms - our old friend struct s1. */
      if (nc_def_compound(ncid, sizeof(struct s1), S1_TYPE_NAME, &s1_typeid)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, X_NAME,
			     NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, Y_NAME,
			     NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;

      /* Now make a new type: a vlen of our compound type. */
      if (nc_def_vlen(ncid, VLEN_NAME, s1_typeid, &vlen_typeid)) ERR;

      /* Write the output data as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, VLEN_ATT_NAME, vlen_typeid,
      		     DIM1_LEN, vlen_of_comp_out)) ERR;

      /* /\* How does it look? *\/ */
      /* if (check_file_1(ncid, vlen_of_comp_out)) ERR; */

      /* We're done - wasn't that easy? */
      if (nc_close(ncid)) ERR;

      /* Check it out. */
      /* if (nc_open(FILE_NAME_1, NC_NOWRITE, &ncid)) ERR; */
      /* if (check_file_1(ncid, vlen_of_comp_out)) ERR; */
      /* if (nc_close(ncid)) ERR; */
   }
   SUMMARIZE_ERR;
   printf("*** testing Solaris-written vlen of compound type...");
   {
      char file_in[NC_MAX_NAME + 1];

      strcpy(file_in, "");
      if (getenv("srcdir"))
      {
	 strcat(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
      }
      strcat(file_in, REF_FILE_NAME_1);

      /* Check out the same file, generated on buddy and included with
       * the distribution. */
      if (nc_open(file_in, NC_NOWRITE, &ncid)) ERR;
      if (check_file_1(ncid, vlen_of_comp_out)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing compound type containing array of compound type...");
   {
      nc_type s1_typeid, s2_typeid;
      int dimsizes[1] = {NUM_S1};

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME_2, NC_NETCDF4, &ncid)) ERR;

      /* Create a simple compound type which has different sizes on
       * different platforms - our old friend struct s1. */
      if (nc_def_compound(ncid, sizeof(struct s1), S1_TYPE_NAME, &s1_typeid)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, X_NAME,
			     NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, Y_NAME,
			     NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;

      /* Now make a compound type that holds an array of the struct s1
       * type. */
      if (nc_def_compound(ncid, sizeof(struct s2), S2_TYPE_NAME, &s2_typeid)) ERR;
      if (nc_insert_array_compound(ncid, s2_typeid, S1_NAME,
				   NC_COMPOUND_OFFSET(struct s2, data),
				   s1_typeid, 1, dimsizes)) ERR;


      /* Write the output data as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, S2_ATT_NAME, s2_typeid,
		     DIM2_LEN, comp_array_of_comp_out)) ERR;

      /* How does it look? */
      if (check_file_2(ncid, comp_array_of_comp_out)) ERR;

      /* We're done - wasn't that easy? */
      if (nc_close(ncid)) ERR;

      /* Check it out. */
      if (nc_open(FILE_NAME_2, NC_NOWRITE, &ncid)) ERR;
      if (check_file_2(ncid, comp_array_of_comp_out)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing Solaris-written compound type containing array of compound type...");
   {
      char file_in[NC_MAX_NAME + 1];

      strcpy(file_in, "");
      if (getenv("srcdir"))
      {
	 strcat(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
      }
      strcat(file_in, REF_FILE_NAME_2);

      /* Check out the same file, generated on buddy and included with
       * the distribution. */
      if (nc_open(file_in, NC_NOWRITE, &ncid)) ERR;
      if (check_file_2(ncid, comp_array_of_comp_out)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
#ifdef EXTRA_TESTS
   printf("*** testing compound attribute containing array of vlen of compound type...");
   {
      nc_type vlen_typeid, s3_typeid, s1_typeid;
      int dimsizes[1] = {NUM_VL};

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME_3, NC_NETCDF4, &ncid)) ERR;

      /* Create a simple compound type which has different sizes on
       * different platforms - our old friend struct s1. */
      if (nc_def_compound(ncid, sizeof(struct s1), S1_TYPE_NAME, &s1_typeid)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, X_NAME,
			     NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, Y_NAME,
			     NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;

      /* Now make a new type: a vlen of our s1 compound type. */
      if (nc_def_vlen(ncid, VLEN_NAME, s1_typeid, &vlen_typeid)) ERR;

      /* Now make a compound type that holds an array of the VLEN
       * type. */
      if (nc_def_compound(ncid, sizeof(struct s3), S3_TYPE_NAME, &s3_typeid)) ERR;
      if (nc_insert_array_compound(ncid, s3_typeid, VL_NAME,
				   NC_COMPOUND_OFFSET(struct s3, data),
				   vlen_typeid, 1, dimsizes)) ERR;


      /* Write the output data as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, S3_ATT_NAME, s3_typeid,
		     DIM3_LEN, comp_array_of_vlen_of_comp_out)) ERR;

      /* How does it look? */
      if (check_file_3(ncid, comp_array_of_vlen_of_comp_out)) ERR;

      /* We're done - wasn't that easy? */
      if (nc_close(ncid)) ERR;

      /* Check it out. */
      if (nc_open(FILE_NAME_3, NC_NOWRITE, &ncid)) ERR;
      if (check_file_3(ncid, comp_array_of_vlen_of_comp_out)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
/*    printf("*** testing Solaris-written compound containing array of vlen of compound type..."); */
/*    { */
/*       /\* Check out the same file, generated on buddy and included with */
/*        * the distribution. *\/ */
/*       if (nc_open(REF_FILE_NAME_3, NC_NOWRITE, &ncid)) ERR; */
/*       if (check_file_3(ncid, comp_array_of_vlen_of_comp_out)) ERR; */
/*       if (nc_close(ncid)) ERR; */
/*    } */
/*    SUMMARIZE_ERR; */
   printf("*** testing compound variable containing array of vlen of compound type...");
   {
      nc_type vlen_typeid, s3_typeid, s1_typeid;
      int varid, dimid;
      int dimsizes[1] = {NUM_VL};

      /* Create a netCDF-4 file. */
      if (nc_create(FILE_NAME_4, NC_NETCDF4, &ncid)) ERR;

      /* Create a simple compound type which has different sizes on
       * different platforms - our old friend struct s1. */
      if (nc_def_compound(ncid, sizeof(struct s1), S1_TYPE_NAME, &s1_typeid)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, X_NAME,
			     NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, Y_NAME,
			     NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;

      /* Now make a new type: a vlen of our s1 compound type. */
      if (nc_def_vlen(ncid, VLEN_NAME, s1_typeid, &vlen_typeid)) ERR;

      /* Now make a compound type that holds an array of the VLEN
       * type. */
      if (nc_def_compound(ncid, sizeof(struct s3), S3_TYPE_NAME, &s3_typeid)) ERR;
      if (nc_insert_array_compound(ncid, s3_typeid, VL_NAME,
				   NC_COMPOUND_OFFSET(struct s3, data),
				   vlen_typeid, 1, dimsizes)) ERR;

      /* Create a dimension and a var of s3 type, then write the
       * data. */
      if (nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR3_NAME, s3_typeid, 1, &dimid, &varid)) ERR;
      if (nc_put_var(ncid, varid, comp_array_of_vlen_of_comp_out)) ERR;

      /* How does it look? */
      if (check_file_4(ncid, comp_array_of_vlen_of_comp_out)) ERR;

      /* We're done - wasn't that easy? */
      if (nc_close(ncid)) ERR;

      /* Check it out. */
      if (nc_open(FILE_NAME_4, NC_NOWRITE, &ncid)) ERR;
      if (check_file_4(ncid, comp_array_of_vlen_of_comp_out)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
#endif

   /* Free our mallocs. */
   for (i = 0; i < DIM1_LEN; i++)
      free(vlen_of_comp_out[i].p);
   for (i = 0; i < DIM3_LEN; i++)
      for (j = 0; j < NUM_VL; j++)
	 free(comp_array_of_vlen_of_comp_out[i].data[j].p);

   free(comp_array_of_comp_out);
   free(comp_array_of_vlen_of_comp_out);
   free(vlen_of_comp_out);

   FINAL_RESULTS;
}


