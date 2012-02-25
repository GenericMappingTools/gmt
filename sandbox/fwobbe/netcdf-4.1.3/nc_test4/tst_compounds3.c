/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 compound type feature, even more. 

   $Id$
*/

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>

#define FILE_NAME "tst_compounds3.nc"
#define S1_PACKED_NAME "s1_packed_compound_type_with_boring_name"
#define S1_NAME "s1_compound_type_unwhimsiclaly_named"
#define I_NAME "i_of_little_quirkiness"
#define J_NAME "j_with_no_originality"
#define DIM_NAME "intentionally_unimaginatevely_named_dimension"
#define DIM_LEN 1
#define VAR_NAME "deliberately_boring_variable"

typedef struct g1_c_t {
   float x;
   double y;
} g1_c_t;
typedef struct g2_d_t {
   g1_c_t s1;
} g2_d_t;

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 compound types even more.\n");
   printf("*** testing compound variable create from packed struct...");
   {
      int ncid, typeid, varid;
      size_t nfields;
      int dimid;
      int ndims, nvars, natts, unlimdimid;
      char name_in[NC_MAX_NAME + 1];
      size_t size;
      nc_type xtype, field_xtype;
      int dimids[] = {0};
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      int i;

      struct s1_packed
      {
	 short i;
	 long long j;
      };
      /* This packing extension works with GNU compilers... */
      /* } __attribute__ ((__packed__));*/
      struct s1
      {
	 short i;
	 long long j;
      };
      struct s1_packed *data;
      struct s1 *data_in;

      if (!(data = calloc(sizeof(struct s1_packed), DIM_LEN))) ERR;
      if (!(data_in = calloc(sizeof(struct s1), DIM_LEN))) ERR;

      /* Create some phony data. */
      for (i = 0; i < DIM_LEN; i++)
      {
	 data[i].i = 100;
	 data[i].j = 1000000000000LL;
      }

      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1_packed), S1_PACKED_NAME, &typeid)) ERR;
      if (nc_inq_compound(ncid, typeid, name_in, &size, &nfields)) ERR;
      if (size != sizeof(struct s1_packed) || strcmp(name_in, S1_PACKED_NAME) || nfields) ERR;
      if (nc_insert_compound(ncid, typeid, I_NAME, NC_COMPOUND_OFFSET(struct s1_packed, i),
			     NC_SHORT)) ERR;
      if (nc_insert_compound(ncid, typeid, J_NAME, NC_COMPOUND_OFFSET(struct s1_packed, j),
			     NC_INT64)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a peek. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;

      /* Check the var and its type. */
      if (nc_inq_var(ncid, 0, name_in, &xtype, &ndims, dimids, &natts)) ERR;
      if (strcmp(name_in, VAR_NAME) || ndims != 1 || natts != 0 || dimids[0] != 0) ERR;
      if (nc_inq_compound(ncid, xtype, name_in, &size, &nfields)) ERR;
      if (nfields != 2 || size != sizeof(struct s1) || strcmp(name_in, S1_PACKED_NAME)) ERR;
      if (nc_inq_compound_field(ncid, xtype, 0, name_in, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (field_ndims) ERR;
      if (strcmp(name_in, I_NAME) || offset != NC_COMPOUND_OFFSET(struct s1, i) ||
	  (field_xtype != NC_SHORT || field_ndims != 0)) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name_in, &offset, &field_xtype, &field_ndims,
				field_sizes)) ERR;
      if (strcmp(name_in, J_NAME) || offset != NC_COMPOUND_OFFSET(struct s1, j) ||
	  field_xtype != NC_INT64) ERR;
      
      /* Now check the data. */
      if (nc_get_var(ncid, varid, data_in)) ERR;
      for (i = 0; i < DIM_LEN; i++)
	 if (data[i].i != data_in[i].i || data[i].j != data_in[i].j) ERR;
      if (nc_close(ncid)) ERR;

      free(data);
      free(data_in);
   }
   SUMMARIZE_ERR;
   printf("*** testing compound attribute with Dennis...");
   {
#define GROUP1_NAME "g1"
#define GROUP2_NAME "g2"
#define TYPE1_NAME "t1"
#define TYPE2_NAME "t2"
#define ATT_NAME "a1"
#define ATT_LEN 1

      int root_grp, g1_grp, g2_grp;
      int g1_c_t_typ, g2_d_t_typ, type1id, type2id;
      char name_in[NC_MAX_NAME + 1], full_name[NC_MAX_NAME + 1];
      size_t size_in;
      g2_d_t *a1_att, *a1_att_in;
      int i;

      if (!(a1_att = calloc(sizeof(g2_d_t), ATT_LEN))) ERR;
      if (!(a1_att_in = calloc(sizeof(g2_d_t), ATT_LEN))) ERR;

      for (i = 0; i < ATT_LEN; i++)
      {
	 a1_att[i].s1.x = 13.3;
	 a1_att[i].s1.y = 13.3;
      }
      /* Create a file with two groups, define a type in each group,
       * and write an att of that type in the root group. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &root_grp)) ERR;
      if (nc_def_grp(root_grp, GROUP1_NAME, &g1_grp)) ERR;
      if (nc_def_grp(root_grp, GROUP2_NAME, &g2_grp)) ERR;
      if (nc_def_compound(g1_grp, sizeof(g1_c_t), TYPE1_NAME, &g1_c_t_typ)) ERR;
      if (nc_insert_compound(g1_grp, g1_c_t_typ, "x", NC_COMPOUND_OFFSET(g1_c_t,x), NC_FLOAT)) ERR;
      if (nc_insert_compound(g1_grp, g1_c_t_typ, "y", NC_COMPOUND_OFFSET(g1_c_t,y), NC_DOUBLE)) ERR;
      if (nc_def_compound(g2_grp, sizeof(g2_d_t), TYPE2_NAME, &g2_d_t_typ)) ERR;
      if (nc_insert_compound(g2_grp, g2_d_t_typ, "s1", NC_COMPOUND_OFFSET(g2_d_t,s1), g1_c_t_typ)) ERR;
      if (nc_put_att(root_grp, NC_GLOBAL, ATT_NAME, g2_d_t_typ, ATT_LEN, a1_att)) ERR;
      if (nc_close(root_grp)) ERR;

      /* Check the file. */
      if (nc_open(FILE_NAME, NC_WRITE, &root_grp)) ERR;

      /* Check the attribute. */
      if (nc_get_att(root_grp, NC_GLOBAL, ATT_NAME, a1_att_in)) ERR;
      if (a1_att_in[0].s1.x != a1_att[0].s1.x ||
	  a1_att_in[0].s1.y != a1_att[0].s1.y) ERR;

      /* Check the type in grp1. */
      if (nc_inq_grp_ncid(root_grp, GROUP1_NAME, &g1_grp)) ERR;
      if (nc_inq_typeid(g1_grp, TYPE1_NAME, &type1id)) ERR;
      if (nc_inq_type(g1_grp, type1id, name_in, &size_in)) ERR;
      if (strcmp(name_in, TYPE1_NAME) || size_in != sizeof(g1_c_t)) ERR;

      /* Check the type in grp2. */
      if (nc_inq_grp_ncid(root_grp, GROUP2_NAME, &g2_grp)) ERR;
      if (nc_inq_typeid(g2_grp, TYPE2_NAME, &type2id)) ERR;
      if (nc_inq_type(g2_grp, type2id, name_in, &size_in)) ERR;
      if (strcmp(name_in, TYPE2_NAME) || size_in != sizeof(g2_d_t)) ERR;

      /* This fails because it's not a fully-qualified name. */
      sprintf(full_name, "%s/%s", GROUP2_NAME, TYPE2_NAME);
      if (nc_inq_typeid(root_grp, full_name, &type2id) != NC_EINVAL) ERR;

      /* Check the type using it's full name. */
      sprintf(full_name, "/%s/%s", GROUP2_NAME, TYPE2_NAME);
/*      if (nc_inq_typeid(root_grp, full_name, &type2id)) ERR;       */
/*       if (nc_inq_type(g2_grp, type2id, name_in, &size_in)) ERR; */
/*       if (strcmp(name_in, TYPE2_NAME) || size_in != sizeof(g2_d_t)) ERR; */

      /* Check that a type in grp1 can be identified in grp2 */
      type1id = -1;
      if (nc_inq_typeid(g2_grp, TYPE1_NAME, &type1id)) ERR;
      if (nc_inq_type(g2_grp, type1id, name_in, &size_in)) ERR;
      if (strcmp(name_in, TYPE1_NAME) || size_in != sizeof(g1_c_t)) ERR;

      /* We are done! */
      if (nc_close(root_grp)) ERR;
      free(a1_att);
      free(a1_att_in);
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


