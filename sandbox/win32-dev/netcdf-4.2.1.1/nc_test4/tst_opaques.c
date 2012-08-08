/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 opaque types.

   $Id$
*/

#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_opaques.nc"
#define DIM_LEN 3
#define ATT_NAME "att_name"
#define DIM_NAME "dim"
#define BASE_SIZE 20
#define VAR_NAME "var_defined_by_netcdf_user"
#define TYPE_NAME "type_defined_by_netcdf_user"

int
main(int argc, char **argv)
{
   int ncid;
   size_t size_in;
   nc_type xtype;
   unsigned char data[DIM_LEN][BASE_SIZE], data_in[DIM_LEN][BASE_SIZE];
   int i, j;

   printf("\n*** Testing netcdf-4 opaque type.\n");

   for (i=0; i<DIM_LEN; i++)
      for (j=0; j<BASE_SIZE; j++)
	 data[i][j] = 0;

   printf("*** testing scalar opaque variable...");
   {
      int varid;
      char name_in[NC_MAX_NAME+1];
      size_t nfields_in, base_size_in;
      nc_type base_nc_type_in, var_type;
      int class_in;
      char var_name[NC_MAX_NAME+1];
      int  nvars, natts, ndims, unlimdimid;

      /* Create a file that has an opaque variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_opaque(ncid, BASE_SIZE, TYPE_NAME, &xtype)) ERR;
      if (nc_inq_user_type(ncid, xtype, name_in, &base_size_in, &base_nc_type_in, &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE || 
	  base_nc_type_in != 0 || nfields_in != 0 || class_in != NC_OPAQUE) ERR;
      if (nc_inq_opaque(ncid, xtype, name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE) ERR;
      if (nc_def_var(ncid, VAR_NAME, xtype, 0, NULL, &varid)) ERR; 
      if (nc_put_var(ncid, varid, &data[0])) ERR; 
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, NULL, &natts)) ERR;
      if (ndims != 0 || strcmp(var_name, VAR_NAME) || natts != 0) ERR;
      if (nc_get_var(ncid, 0, &data_in[0])) ERR;
      for (j = 0; j < BASE_SIZE; j++)
	 if (data_in[0][j] != data[0][j]) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing opaque variable...");
   {
      int dimid, varid, dimids[] = {0};
      char name_in[NC_MAX_NAME+1];
      nc_type base_nc_type_in, var_type;
      size_t nfields_in, base_size_in;
      int class_in;
      char var_name[NC_MAX_NAME+1];
      int  nvars, natts, ndims, unlimdimid, dimids_var[1];

      /* Create a file that has an opaque variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_opaque(ncid, BASE_SIZE, TYPE_NAME, &xtype)) ERR;
      if (nc_inq_user_type(ncid, xtype, name_in, &base_size_in, &base_nc_type_in, &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE ||
	  base_nc_type_in != 0 || nfields_in != 0 || class_in != NC_OPAQUE) ERR;
      if (nc_inq_opaque(ncid, xtype, name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE) ERR;
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, xtype, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, data)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_var(ncid, 0, var_name, &var_type, &ndims, dimids_var, &natts)) ERR;
      if (ndims != 1 || strcmp(var_name, VAR_NAME) ||
	  dimids_var[0] != dimids[0] || natts != 0) ERR;
      if (nc_get_var(ncid, 0, data_in)) ERR;
      for (i=0; i<DIM_LEN; i++)
 	 for (j=0; j<BASE_SIZE; j++)
 	    if (data_in[i][j] != data[i][j]) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing *really* simple opaque attribute...");
   {

      /* Create a file that has an opaque attribute. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_opaque(ncid, BASE_SIZE, TYPE_NAME, &xtype)) ERR;

      /* Write an att. */
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, xtype, DIM_LEN, data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing opaque attribute...");
   {
      char name_in[NC_MAX_NAME+1];
      nc_type base_nc_type_in;
      size_t base_size_in;
      size_t nfields_in;
      int class_in;

      /* Create a file that has an opaque attribute. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      if (nc_def_opaque(ncid, BASE_SIZE, TYPE_NAME, &xtype)) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, xtype, name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE ||
	  base_nc_type_in != 0 || nfields_in != 0 || class_in != NC_OPAQUE) ERR;
      if (nc_inq_opaque(ncid, xtype, name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE) ERR;

      /* Write an att. */
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, xtype, DIM_LEN, data)) ERR;

      if (nc_close(ncid)) ERR;

      /* Reopen. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check it out. */
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &xtype, &size_in)) ERR;
      if (size_in != DIM_LEN) ERR;
      if (nc_inq_user_type(ncid, xtype, name_in, &base_size_in, &base_nc_type_in, &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE ||
	  base_nc_type_in != 0 || nfields_in != 0 || class_in != NC_OPAQUE) ERR;
      if (nc_inq_opaque(ncid, xtype, name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != BASE_SIZE) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      for (i=0; i<DIM_LEN; i++)
 	 for (j=0; j<BASE_SIZE; j++)
 	    if (data_in[i][j] != data[i][j]) ERR;

      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing 3 opaque types...");
   {
#define TYPE_SIZE1 20
#define NUM_TYPES 3

      char name_in[NC_MAX_NAME+1];
      nc_type base_nc_type_in;
      size_t nfields_in, base_size_in;
      nc_type otid[3];
      int class_in;
      char type_name[NUM_TYPES][NC_MAX_NAME + 1] = {"o1", "o2", "o3"};
      int  nvars, natts, ndims, unlimdimid;
      int ntypes, typeids[NUM_TYPES];
      int i;

      /* Create a file that has three opaque types. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      for (i = 0; i < NUM_TYPES; i++)
      {
	 if (nc_def_opaque(ncid, TYPE_SIZE1, type_name[i], &otid[i])) ERR;
	 if (nc_inq_user_type(ncid, otid[i], name_in, &base_size_in, &base_nc_type_in, &nfields_in, &class_in)) ERR;
	 if (strcmp(name_in, type_name[i]) || base_size_in != TYPE_SIZE1 ||
	     base_nc_type_in != 0 || nfields_in != 0 || class_in != NC_OPAQUE) ERR;
	 if (nc_inq_opaque(ncid, otid[i], name_in, &base_size_in)) ERR;
	 if (strcmp(name_in, type_name[i]) || base_size_in != TYPE_SIZE1) ERR;
      }
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_inq_typeids(ncid, &ntypes, typeids)) ERR;
      if (ntypes != NUM_TYPES) ERR;
      for (i = 0; i < NUM_TYPES; i++)
      {
	 if (nc_inq_user_type(ncid, otid[i], name_in, &base_size_in, &base_nc_type_in, &nfields_in, &class_in)) ERR;
	 if (strcmp(name_in, type_name[i]) || base_size_in != TYPE_SIZE1 ||
	     base_nc_type_in != 0 || nfields_in != 0 || class_in != NC_OPAQUE) ERR;
	 if (nc_inq_opaque(ncid, otid[i], name_in, &base_size_in)) ERR;
	 if (strcmp(name_in, type_name[i]) || base_size_in != TYPE_SIZE1) ERR;
      }
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

