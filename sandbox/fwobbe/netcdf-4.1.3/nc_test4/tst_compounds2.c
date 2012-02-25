/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 compound type feature. 

   $Id$
*/

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>

#define FILE_NAME "tst_compounds2.nc"

int
main(int argc, char **argv)
{

    printf("\n*** Testing netcdf-4 user defined type functions, even more.\n"); 
    printf("*** testing compound var containing byte arrays of various size..."); 
   {
#define DIM1_LEN 1      
#define ARRAY_LEN (NC_MAX_NAME + 1)
      int ncid;
      size_t len;
      nc_type xtype, type_id;
      int dim_sizes[] = {ARRAY_LEN};
      int i, j;

      struct s1
      {
	    unsigned char x[ARRAY_LEN];
	    float y;
      };
      struct s1 data_out[DIM1_LEN], data_in[DIM1_LEN];
      char *dummy;

      printf("array len=%d... ", ARRAY_LEN);

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s1), DIM1_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s1) * DIM1_LEN); 
      free(dummy); 

      /* Create some phony data. */   
      for (i = 0; i < DIM1_LEN; i++)
      {
	 data_out[i].y = 99.99;
	 for (j = 0; j < ARRAY_LEN; j++)
	    data_out[i].x[j] = j;
      }

      /* Create a file with a nested compound type attribute and variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; 

      /* Now define the compound type. */
      if (nc_def_compound(ncid, sizeof(struct s1), "c", &type_id)) ERR;
      if (nc_insert_array_compound(ncid, type_id, "x",
				   NC_COMPOUND_OFFSET(struct s1, x), NC_UBYTE, 1, dim_sizes)) ERR;
      if (nc_insert_compound(ncid, type_id, "y",
			     NC_COMPOUND_OFFSET(struct s1, y), NC_FLOAT)) ERR;

      /* Write it as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, "a1", type_id, DIM1_LEN, data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Read the att and check values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, "a1", data_in)) ERR;
      for (i=0; i<DIM1_LEN; i++)
      {
	 if (data_in[i].y != data_out[i].y) ERR;
	 for (j = 0; j < ARRAY_LEN; j++)
	    if (data_in[i].x[j] != data_out[i].x[j]) ERR_RET;
      }
      
      /* Use the inq functions to learn about the compound type. */
      if (nc_inq_att(ncid, NC_GLOBAL, "a1", &xtype, &len)) ERR;
      if (len != DIM1_LEN) ERR;
      
      /* Finish checking the containing compound type. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing compound var on different machines...");
   {
#define DIM1_LEN 1
#define NAME1 "x"
#define NAME2 "y"
      int ncid;
      size_t len;
      nc_type xtype, type_id;
      char field_name_in[NC_MAX_NAME + 1];
      size_t offset_in;
      int field_ndims;
      nc_type field_typeid;
      int i;

      struct s1
      {
	    float x;
	    double y;
      };
      struct s1 data_out[DIM1_LEN], data_in[DIM1_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s1), DIM1_LEN)))
	 return NC_ENOMEM;
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s1) * DIM1_LEN);
      free(dummy);

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
	 data_out[i].x = 1;
	 data_out[i].y = -2;
      }

      /* Create a file with a nested compound type attribute and variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Now define the compound type. */
      if (nc_def_compound(ncid, sizeof(struct s1), "c", &type_id)) ERR;
      if (nc_insert_compound(ncid, type_id, NAME1,
			     NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, type_id, NAME2,
			     NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;

      /* Write it as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, "a1", type_id, DIM1_LEN, data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Read the att and check values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, "a1", data_in)) ERR;
      for (i = 0; i < DIM1_LEN; i++)
	 if (data_in[i].x != data_out[i].x || data_in[i].y != data_out[i].y) ERR;
      
      /* Use the inq functions to learn about the compound type. */
      if (nc_inq_att(ncid, NC_GLOBAL, "a1", &xtype, &len)) ERR;
      if (len != DIM1_LEN) ERR;
      if (nc_inq_compound_field(ncid, xtype, 0, field_name_in,
          &offset_in, &field_typeid, &field_ndims, NULL)) ERR;
      if (strcmp(field_name_in, NAME1) || field_typeid != NC_FLOAT || field_ndims) ERR;
      printf("offset x: %d  ", (int)offset_in);
      if (nc_inq_compound_field(ncid, xtype, 1, field_name_in,
          &offset_in, &field_typeid, &field_ndims, NULL)) ERR;
      if (strcmp(field_name_in, NAME2) || field_typeid != NC_DOUBLE || field_ndims) ERR;
      printf("offset y: %d NC_COMPOUND_OFFSET(struct s1, y): %d ", (int)offset_in,
      (int)NC_COMPOUND_OFFSET(struct s1, y));
      
      /* Finish checking the containing compound type. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing compound var containing compound var, on different machines...");
   {
#define DIM1_LEN 1
#define S1_NAME_X "x"
#define S1_NAME_Y "y"
#define S2_NAME_S1 "s1"
#define NUM_TYPES 2
#define INNER_TYPE_NAME "c"
#define OUTER_TYPE_NAME "d"
#define ATT_NAME "a1"

      int ncid;
      size_t len;
      nc_type inner_typeid = 0, outer_typeid, s1_typeid, s2_typeid;
      char field_name_in[NC_MAX_NAME + 1];
      size_t offset_in;
      int field_ndims;
      nc_type field_typeid;
      int i;

      struct s1
      {
	    float x;
	    double y;
      };
      struct s2
      {
	    struct s1 s1;
      };
      struct s2 data_out[DIM1_LEN], data_in[DIM1_LEN];
      int ntypes, typeids[NUM_TYPES];
      size_t size_in, nfields_in;
      char name_in[NC_MAX_NAME + 1];
      int t;
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s2), DIM1_LEN)))
	 return NC_ENOMEM;
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s2) * DIM1_LEN);
      free(dummy);

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
	 data_out[i].s1.x = 1;
	 data_out[i].s1.y = -2;
      }

      /* Create a file with a nested compound type attribute and variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Now define the inner compound type. */
      if (nc_def_compound(ncid, sizeof(struct s1), INNER_TYPE_NAME, &s1_typeid)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, S1_NAME_X,
			     NC_COMPOUND_OFFSET(struct s1, x), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, s1_typeid, S1_NAME_Y,
			     NC_COMPOUND_OFFSET(struct s1, y), NC_DOUBLE)) ERR;

      /* Define the outer compound type. */
      if (nc_def_compound(ncid, sizeof(struct s2), OUTER_TYPE_NAME, &s2_typeid)) ERR;
      if (nc_insert_compound(ncid, s2_typeid, S2_NAME_S1,
			     NC_COMPOUND_OFFSET(struct s2, s1), s1_typeid)) ERR;

      /* Write it as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, s2_typeid, DIM1_LEN, data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Read the att and check values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      for (i = 0; i < DIM1_LEN; i++)
	 if (data_in[i].s1.x != data_out[i].s1.x || data_in[i].s1.y != data_out[i].s1.y) ERR;
      
      /* Use the inq functions to learn about the compound type. */
      if (nc_inq_typeids(ncid, &ntypes, typeids)) ERR;
      if (ntypes != NUM_TYPES) ERR;
      for (t = 0; t < NUM_TYPES; t++)
      {
	 if (nc_inq_compound(ncid, typeids[t], name_in, &size_in, &nfields_in)) ERR;
	 if (!strcmp(name_in, INNER_TYPE_NAME))
	    inner_typeid = typeids[t];
      }

      /* What type is the attribute? */
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &outer_typeid, &len)) ERR;
      if (len != DIM1_LEN) ERR;
      if (nc_inq_compound_field(ncid, outer_typeid, 0, field_name_in,
          &offset_in, &field_typeid, &field_ndims, NULL)) ERR;
      if (strcmp(field_name_in, S2_NAME_S1) || field_typeid != inner_typeid || field_ndims) ERR;
      if (nc_inq_compound_field(ncid, field_typeid, 1, field_name_in,
          &offset_in, &field_typeid, &field_ndims, NULL)) ERR;
      if (strcmp(field_name_in, S1_NAME_Y) || field_typeid != NC_DOUBLE || field_ndims) ERR;
      
      /* Finish checking the containing compound type. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


