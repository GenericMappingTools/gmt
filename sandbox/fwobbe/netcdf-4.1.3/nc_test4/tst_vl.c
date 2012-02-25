/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 variable length code. 

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_vl.nc"
#define FILE_NAME2 "tst_vl_2.nc"
#define DIM_LEN 3
#define VLEN_NAME "vlen_name"
#define ATT_NAME "att_name"
#define DIM_NAME "dim"
#define VAR_NAME "var"
#define PHONY_VAL -99

/* Use this struct to hold vlen data: */
/* typedef struct  { */
/* size_t len;  Length of VL data (in base type units)        */
/* void *p;     Pointer to VL data          */
/* } hvl_t; */
int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 variable length type functions.\n");
   printf("*** testing vlen (of ints) attribute...");
   {
      int ncid, typeid;
      nc_vlen_t data[DIM_LEN], data_in[DIM_LEN];
      size_t size_in;
      nc_type base_nc_type_in;
      char name_in[NC_MAX_NAME + 1];
      int *phony, class_in;
      size_t len_in;
      int i, j;
      
      /* Create phony data. */
      for (i=0; i<DIM_LEN; i++)
      {
	 if (!(phony = malloc(sizeof(int) * (i+1))))
	    return NC_ENOMEM;
	 for (j = 0; j < i + 1; j++)
	    phony[j] = PHONY_VAL;
	 data[i].p = phony;
	 data[i].len = i+1;
      }
      
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_vlen(ncid, VLEN_NAME, NC_INT, &typeid)) ERR;
      if (nc_inq_vlen(ncid, typeid, name_in, &size_in, &base_nc_type_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != sizeof(nc_vlen_t) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_inq_user_type(ncid, typeid, name_in, &size_in, &base_nc_type_in, NULL, &class_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != sizeof(nc_vlen_t) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_inq_type(ncid, typeid, name_in, &size_in)) ERR;
      if ((size_in != sizeof(nc_vlen_t) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_inq_compound(ncid, typeid, name_in, &size_in, NULL) != NC_EBADTYPE) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, typeid, DIM_LEN, data)) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Reset data to make sure it was really copied. */
      for (i=0; i<DIM_LEN; i++)
	 for (j = 0; j < i + 1; j++)
	    ((int *)(data[i].p))[j] = 0;

      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      for (i = 0; i < DIM_LEN; i++)
      {
	 if (data_in[i].len != i + 1) ERR;
	 for (j = 0; j < data_in[i].len; j++)
	    if (*((int *)data_in[i].p) != PHONY_VAL)
	    {
	       printf("*((int *)data_in[%d].p = %d (0x%x)\n", i, *((int *)data_in[i].p),
		      *((int *)data_in[i].p));
	       ERR;
	       return 2;
	    }
      }
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &typeid, &len_in)) ERR;
      if (len_in != DIM_LEN) ERR;
      if (nc_inq_vlen(ncid, typeid, name_in, &size_in, &base_nc_type_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != 4 && strcmp(name_in, VLEN_NAME)))
      {
	 printf("base_nc_type_in = %d size_in = %d name_in = %s\n", base_nc_type_in,
		(int)size_in, name_in);
	 ERR;
      }
      if (nc_inq_user_type(ncid, typeid, name_in, &size_in, &base_nc_type_in, NULL, &class_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != sizeof(nc_vlen_t) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_close(ncid)) ERR;

      /* Free the memory used in our phony data. */
      for (i=0; i<DIM_LEN; i++)
	 if (nc_free_vlen(&data[i]) || nc_free_vlen(&data_in[i])) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing vlen (of ints) variable...");
   {
      int dimid, varid, dimids[] = {0};
      char var_name[NC_MAX_NAME+1];
      int nvars, natts, ndims, unlimdimid, dimids_var[1];
      nc_type var_type;
      int ncid, typeid;
      nc_vlen_t data[DIM_LEN], data_in[DIM_LEN];
      int *phony;
      int i, j;
      
      /* Create phony data. */
      for (i=0; i<DIM_LEN; i++)
      {
	 if (!(phony = malloc(sizeof(int) * (i+1))))
	    return NC_ENOMEM;
	 for (j=0; j<i+1; j++)
	    phony[j] = PHONY_VAL;
	 data[i].p = phony;
	 data[i].len = i+1;
      }

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_vlen(ncid, "name1", NC_INT, &typeid)) ERR;
      if (nc_def_dim(ncid, DIM_NAME, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, typeid, 1, dimids, &varid)) ERR;
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
	 for (j=0; j<data_in[i].len; j++)
	    if (*((int *)data_in[i].p) != *((int *)data[i].p))
	    {
	       printf("*((int *)data_in[%d].p = %d (0x%x)\n", i, *((int *)data_in[i].p),
		      *((int *)data_in[i].p));
	       ERR;
	       return 2;
	    }
      if (nc_close(ncid)) ERR;

      /* Free the memory used in our phony data. */
      for (i=0; i<DIM_LEN; i++)
	 if (nc_free_vlen(&data[i]) || nc_free_vlen(&data_in[i])) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing vlen (of ints) attribute copy...");
   {
      int ncid1, ncid2, typeid1, typeid2;
      nc_vlen_t data[DIM_LEN], data_in[DIM_LEN];
      size_t size_in;
      nc_type base_nc_type_in;
      char name_in[NC_MAX_NAME + 1];
      int *phony, class_in;
      size_t len_in;
      int i, j;
      
      /* Create phony data. */
      for (i=0; i<DIM_LEN; i++)
      {
	 if (!(phony = malloc(sizeof(int) * (i+1))))
	    return NC_ENOMEM;
	 for (j = 0; j < i + 1; j++)
	    phony[j] = PHONY_VAL;
	 data[i].p = phony;
	 data[i].len = i+1;
      }
      
      /* Create two files with the same vlen type. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid1)) ERR;
      if (nc_create(FILE_NAME2, NC_NETCDF4, &ncid2)) ERR;
      if (nc_def_vlen(ncid1, VLEN_NAME, NC_INT, &typeid1)) ERR;
      if (nc_def_vlen(ncid2, VLEN_NAME, NC_INT, &typeid2)) ERR;
      if (nc_put_att(ncid1, NC_GLOBAL, ATT_NAME, typeid1, DIM_LEN, data)) ERR;
      if (nc_copy_att(ncid1, NC_GLOBAL, ATT_NAME, ncid2, NC_GLOBAL)) ERR;
      if (nc_close(ncid1)) ERR;
      if (nc_close(ncid2)) ERR;
      
      /* Reset data to make sure it was really copied. */
      for (i=0; i<DIM_LEN; i++)
	 for (j = 0; j < i + 1; j++)
	    ((int *)(data[i].p))[j] = 0;

      /* Check it out. */
      if (nc_open(FILE_NAME2, NC_NOWRITE, &ncid2)) ERR;
      if (nc_get_att(ncid2, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      for (i = 0; i < DIM_LEN; i++)
      {
	 if (data_in[i].len != i + 1) ERR;
	 for (j = 0; j < data_in[i].len; j++)
	    if (*((int *)data_in[i].p) != PHONY_VAL)
	    {
	       printf("*((int *)data_in[%d].p = %d (0x%x)\n", i, *((int *)data_in[i].p),
		      *((int *)data_in[i].p));
	       ERR;
	       return 2;
	    }
      }
      if (nc_inq_att(ncid2, NC_GLOBAL, ATT_NAME, &typeid2, &len_in)) ERR;
      if (len_in != DIM_LEN) ERR;
      if (nc_inq_vlen(ncid2, typeid2, name_in, &size_in, &base_nc_type_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != 4 && strcmp(name_in, VLEN_NAME)))
      {
	 printf("base_nc_type_in = %d size_in = %d name_in = %s\n", base_nc_type_in,
		(int)size_in, name_in);
	 ERR;
      }
      if (nc_inq_user_type(ncid2, typeid2, name_in, &size_in, &base_nc_type_in, NULL, &class_in)) ERR;
      if (base_nc_type_in != NC_INT || (size_in != sizeof(nc_vlen_t) || strcmp(name_in, VLEN_NAME))) ERR;
      if (nc_close(ncid2)) ERR;

      /* Free the memory used in our phony data. */
      for (i=0; i<DIM_LEN; i++)
	 if (nc_free_vlen(&data[i]) || nc_free_vlen(&data_in[i])) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

