/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 compound type feature. 

   $Id$
*/

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>

#define FILE_NAME "tst_compounds.nc"
#define SVC_REC "Service_Record"
#define BATTLES_WITH_KLINGONS "Number_of_Battles_with_Klingons"
#define DATES_WITH_ALIENS "Dates_with_Alien_Hotties"
#define STARDATE "Stardate"
#define DIM_LEN 3
#define SERVICE_RECORD "Kirk_Service_Record"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 user defined type functions.\n");
   printf("*** testing simple compound scalar variable create...");
   {
      int ncid, typeid, varid;
      size_t nfields;
      int ndims, nvars, natts, unlimdimid;
      char name[NC_MAX_NAME + 1];
      size_t size;
      nc_type xtype, field_xtype;
      int dimids[] = {0}, fieldid;
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      struct s1 
      {
	    int i1;
	    int i2;
      };
      struct s1 data, data_in;

      /* Create some phony data. */   
      data.i1 = 5;
      data.i2 = 10;

      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &typeid)) ERR;
      if (nc_inq_compound(ncid, typeid, name, &size, &nfields)) ERR;
      if (size != sizeof(struct s1) || strcmp(name, SVC_REC) || nfields) ERR;
      if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS, 
			     NC_COMPOUND_OFFSET(struct s1, i1), NC_INT)) ERR;
      if (nc_insert_compound(ncid, typeid, DATES_WITH_ALIENS, 
			     NC_COMPOUND_OFFSET(struct s1, i2), NC_INT)) ERR;
      if (nc_def_var(ncid, SERVICE_RECORD, typeid, 0, NULL, &varid)) ERR;
      if (nc_put_var(ncid, varid, &data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a peek. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and take a more detailed look at the compound
       * type. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, name, &xtype, &ndims, dimids, &natts)) ERR;
      if (strcmp(name, SERVICE_RECORD) || ndims != 0 || natts != 0) ERR;
      if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
      if (nfields != 2 || size != sizeof(struct s1) || strcmp(name, SVC_REC)) ERR;
      if (nc_inq_compound_name(ncid, xtype, name)) ERR;
      if (strcmp(name, SVC_REC)) ERR;
      if (nc_inq_compound_size(ncid, xtype, &size)) ERR;
      if (size != sizeof(struct s1)) ERR;
      if (nc_inq_compound_nfields(ncid, xtype, &nfields)) ERR;
      if (nfields != 2) ERR;
      if (nc_inq_compound_field(ncid, xtype, 0, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (field_ndims) ERR;
      if (strcmp(name, BATTLES_WITH_KLINGONS) || offset != 0 || (field_xtype != NC_INT || field_ndims != 0)) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (strcmp(name, DATES_WITH_ALIENS) || offset != 4 || field_xtype != NC_INT) ERR;
      if (nc_inq_compound_fieldname(ncid, xtype, 1, name)) ERR;
      if (strcmp(name, DATES_WITH_ALIENS)) ERR;
      if (nc_inq_compound_fieldindex(ncid, xtype, BATTLES_WITH_KLINGONS, &fieldid)) ERR;
      if (fieldid != 0) ERR;
      if (nc_inq_compound_fieldoffset(ncid, xtype, 1, &offset)) ERR;
      if (offset != 4) ERR;
      if (nc_inq_compound_fieldtype(ncid, xtype, 1, &field_xtype)) ERR;
      if (field_xtype != NC_INT) ERR;
      if (nc_get_var(ncid, varid, &data_in)) ERR;
      if (data.i1 != data_in.i1 || data.i2 != data_in.i2) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple compound variable create...");
   {
      int ncid, typeid, varid;
      size_t nfields;
      int dimid;
      int ndims, nvars, natts, unlimdimid;
      char name[NC_MAX_NAME + 1];
      size_t size;
      nc_type xtype, field_xtype;
      int dimids[] = {0}, fieldid;
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      int i;
      struct s1 
      {
	    int i1;
	    int i2;
      };
      struct s1 data[DIM_LEN], data_in[DIM_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s1), DIM_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)data, (void *)dummy, sizeof(struct s1) * DIM_LEN); 
      free(dummy); 

      /* Create some phony data. */   
      for (i = 0; i < DIM_LEN; i++)
      {
	 data[i].i1 = 5;
	 data[i].i2 = 10;
      }

      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &typeid)) ERR;
      if (nc_inq_compound(ncid, typeid, name, &size, &nfields)) ERR;
      if (size != sizeof(struct s1) || strcmp(name, SVC_REC) || nfields) ERR;
      if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS, 
			     NC_COMPOUND_OFFSET(struct s1, i1), NC_INT)) ERR;
      if (nc_insert_compound(ncid, typeid, DATES_WITH_ALIENS, 
			     NC_COMPOUND_OFFSET(struct s1, i2), NC_INT)) ERR;
      if (nc_def_dim(ncid, STARDATE, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, SERVICE_RECORD, typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a peek. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 1 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and take a more detailed look at the compound
       * type. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, name, &xtype, &ndims, dimids, &natts)) ERR;
      if (strcmp(name, SERVICE_RECORD) || ndims != 1 || natts != 0 || dimids[0] != 0) ERR;
      if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
      if (nfields != 2 || size != sizeof(struct s1) || strcmp(name, SVC_REC)) ERR;
      if (nc_inq_compound_name(ncid, xtype, name)) ERR;
      if (strcmp(name, SVC_REC)) ERR;
      if (nc_inq_compound_size(ncid, xtype, &size)) ERR;
      if (size != sizeof(struct s1)) ERR;
      if (nc_inq_compound_nfields(ncid, xtype, &nfields)) ERR;
      if (nfields != 2) ERR;
      if (nc_inq_compound_field(ncid, xtype, 0, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (field_ndims) ERR;
      if (strcmp(name, BATTLES_WITH_KLINGONS) || offset != 0 || (field_xtype != NC_INT || field_ndims != 0)) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (strcmp(name, DATES_WITH_ALIENS) || offset != 4 || field_xtype != NC_INT) ERR;
      if (nc_inq_compound_fieldname(ncid, xtype, 1, name)) ERR;
      if (strcmp(name, DATES_WITH_ALIENS)) ERR;
      if (nc_inq_compound_fieldindex(ncid, xtype, BATTLES_WITH_KLINGONS, &fieldid)) ERR;
      if (fieldid != 0) ERR;
      if (nc_inq_compound_fieldoffset(ncid, xtype, 1, &offset)) ERR;
      if (offset != 4) ERR;
      if (nc_inq_compound_fieldtype(ncid, xtype, 1, &field_xtype)) ERR;
      if (field_xtype != NC_INT) ERR;
      if (nc_get_var(ncid, varid, data_in)) ERR;
      for (i=0; i<DIM_LEN; i++)
	 if (data[i].i1 != data_in[i].i1 || data[i].i2 != data_in[i].i2) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing REALLY simple compound variable create...");
   {
      int ncid, typeid, varid;
      int ndims, nvars, natts, unlimdimid;
      nc_type xtype;
      size_t size_in, nfields_in;
      char name_in[NC_MAX_NAME + 1];
      
      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(int), SVC_REC, &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS, 0, NC_INT)) ERR;
      if (nc_def_var(ncid, SERVICE_RECORD, typeid, 0, NULL, &varid)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a peek. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR; 
      if (ndims != 0 || nvars != 1 || natts != 0 || unlimdimid != -1) ERR; 
      if (nc_inq_vartype(ncid, 0, &xtype)) ERR;
      if (nc_inq_compound(ncid, xtype, name_in, &size_in, &nfields_in)) ERR;
      if (strcmp(name_in, SVC_REC) || size_in != sizeof(int) || nfields_in != 1) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple compound attribute create...");
   {
      int ncid, typeid;
      size_t nfields;
      int ndims, nvars, natts, unlimdimid;
      char name[NC_MAX_NAME + 1];
      size_t size, len;
      nc_type xtype, field_xtype;
      int fieldid;
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      int i;
      struct s1 
      {
	    int i1;
	    int i2;
      };
      struct s1 data[DIM_LEN], data_in[DIM_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s1), DIM_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)data, (void *)dummy, sizeof(struct s1) * DIM_LEN); 
      free(dummy); 

      /* Create some phony data. */   
      for (i=0; i<DIM_LEN; i++)
      {
	 data[i].i1 = 5;
	 data[i].i2 = 10;
      }

      /* Create a file with a global attribute of compound type. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS, 
			     NC_COMPOUND_OFFSET(struct s1, i1), NC_INT)) ERR;
      if (nc_insert_compound(ncid, typeid, DATES_WITH_ALIENS, 
			     NC_COMPOUND_OFFSET(struct s1, i2), NC_INT)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, SERVICE_RECORD, typeid, 3, data)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a peek. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and take a more detailed look at the compound
       * type. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, SERVICE_RECORD, data_in)) ERR;
      for (i = 0; i < DIM_LEN; i++)
	 if (data[i].i1 != data_in[i].i1 || data[i].i2 != data_in[i].i2) ERR;;
      if (nc_inq_att(ncid, NC_GLOBAL, SERVICE_RECORD, &xtype, &len)) ERR;
      if (len != 3) ERR;
      if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
      if (nfields != 2 || size != 8 || strcmp(name, SVC_REC)) ERR;
      if (nc_inq_compound_nfields(ncid, xtype, &nfields)) ERR;
      if (nfields != 2) ERR;
      if (nc_inq_compound_field(ncid, xtype, 0, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (strcmp(name, BATTLES_WITH_KLINGONS) || offset != 0 || field_xtype != NC_INT) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (strcmp(name, DATES_WITH_ALIENS) || offset != 4 || field_xtype != NC_INT) ERR;
      if (nc_inq_compound_fieldindex(ncid, xtype, BATTLES_WITH_KLINGONS, &fieldid)) ERR;
      if (fieldid != 0) ERR;
      if (nc_inq_compound_fieldoffset(ncid, xtype, 1, &offset)) ERR;
      if (offset != 4) ERR;
      if (nc_inq_compound_fieldtype(ncid, xtype, 1, &field_xtype)) ERR;
      if (field_xtype != NC_INT) ERR;
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing compound variable with new atomic types...");
   {
      int ncid, typeid, dimid, varid;
      size_t nfields;
      int ndims, natts;
      char name[NC_MAX_NAME + 1];
      size_t size;
      nc_type xtype, field_xtype;
      int dimids[] = {0};
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      int i;

      /* StarFleet Medical Record. */
      struct sf_med_rec
      {
	    unsigned char num_heads;
	    unsigned short num_arms;
	    unsigned int num_toes;
	    long long ago; /* in a galaxy far far away... */
	    unsigned long long num_hairs;
      };
      struct sf_med_rec med_data_out[DIM_LEN], med_data_in[DIM_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct sf_med_rec), DIM_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)med_data_out, (void *)dummy, sizeof(struct sf_med_rec) * DIM_LEN); 
      free(dummy); 

      /* Create some phony data. */   
      for (i=0; i<DIM_LEN; i++)
      {
	 /* medical data */
	 med_data_out[i].num_heads = 254;
	 med_data_out[i].num_arms = NC_FILL_USHORT - 1;
	 med_data_out[i].num_toes = NC_FILL_UINT - 1;
	 med_data_out[i].ago = NC_FILL_INT64 + 1;
	 med_data_out[i].num_hairs = NC_FILL_UINT64 - 1;
      }

      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct sf_med_rec), "SFMedRec", &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, "num_heads", 
			     NC_COMPOUND_OFFSET(struct sf_med_rec, num_heads), NC_UBYTE)) ERR;
      if (nc_insert_compound(ncid, typeid, "num_arms", 
			     NC_COMPOUND_OFFSET(struct sf_med_rec, num_arms), NC_USHORT)) ERR;
      if (nc_insert_compound(ncid, typeid, "num_toes", 
			     NC_COMPOUND_OFFSET(struct sf_med_rec, num_toes), NC_UINT)) ERR;
      if (nc_insert_compound(ncid, typeid, "ago", 
			     NC_COMPOUND_OFFSET(struct sf_med_rec, ago), NC_INT64)) ERR;
      if (nc_insert_compound(ncid, typeid, "num_hairs", 
			     NC_COMPOUND_OFFSET(struct sf_med_rec, num_hairs), NC_UINT64)) ERR;
      if (nc_def_dim(ncid, STARDATE, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, "starbase_13", typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, med_data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a look. */
      {
	 if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
	 if (nc_inq_var(ncid, 0, name, &xtype, &ndims, dimids, &natts)) ERR;
	 if (strcmp(name, "starbase_13") || ndims != 1 || natts != 0 || dimids[0] != 0) ERR;
	 if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
	 if (nfields != 5 || size != sizeof(struct sf_med_rec) || strcmp(name, "SFMedRec")) ERR;
	 if (nc_inq_compound_field(ncid, xtype, 0, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
	 if (strcmp(name, "num_heads") || offset != 0 || field_xtype != NC_UBYTE) ERR;
	 if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
	 if (strcmp(name, "num_arms") || offset != NC_COMPOUND_OFFSET(struct sf_med_rec, num_arms) || 
	     field_xtype != NC_USHORT) ERR;
	 if (nc_inq_compound_field(ncid, xtype, 2, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
	 if (strcmp(name, "num_toes") || offset != NC_COMPOUND_OFFSET(struct sf_med_rec, num_toes) || 
	     field_xtype != NC_UINT) ERR;
	 if (nc_inq_compound_field(ncid, xtype, 3, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
	 if (strcmp(name, "ago") || offset != NC_COMPOUND_OFFSET(struct sf_med_rec, ago) || 
	     field_xtype != NC_INT64) ERR;
	 if (nc_inq_compound_field(ncid, xtype, 4, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
	 if (strcmp(name, "num_hairs") || offset != NC_COMPOUND_OFFSET(struct sf_med_rec, num_hairs) || 
	     field_xtype != NC_UINT64) ERR;
	 if (nc_get_var(ncid, varid, med_data_in)) ERR;
	 for (i=0; i<DIM_LEN; i++)
	    if (med_data_in[i].num_heads != med_data_out[i].num_heads || 
		med_data_in[i].num_arms != med_data_out[i].num_arms ||
		med_data_in[i].num_toes != med_data_out[i].num_toes ||
		med_data_in[i].ago != med_data_out[i].ago ||
		med_data_in[i].num_hairs != med_data_out[i].num_hairs) ERR;
	 if (nc_close(ncid)) ERR;
      }
   }

   SUMMARIZE_ERR;
   printf("*** testing compound variable containing an array of ints...");
   {
#define NUM_DIMENSIONS 7

      int ncid, typeid, varid;
      size_t nfields;
      int dimid;
      int ndims, natts;
      char name[NC_MAX_NAME + 1];
      size_t size;
      nc_type xtype;
      int dimids[] = {0};
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      nc_type field_typeid;
      int dim_sizes[] = {NUM_DIMENSIONS};
      int i, j;

      /* Since some aliens exists in different, or more than one,
       * dimensions, StarFleet keeps track of the dimensional
       * abilities of everyone on 7 dimensions. */
      struct dim_rec
      {
	    int starfleet_id;
	    int abilities[NUM_DIMENSIONS];
      };
      struct dim_rec dim_data_out[DIM_LEN], dim_data_in[DIM_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct dim_rec), DIM_LEN)))
	 return NC_ENOMEM;
      memcpy((void *)dim_data_out, (void *)dummy, sizeof(struct dim_rec) * DIM_LEN);
      free(dummy);

      /* Create some phony data. */
      for (i=0; i<DIM_LEN; i++)
      {
	 dim_data_out[i].starfleet_id = i;
	 for (j = 0; j < NUM_DIMENSIONS; j++)
	    dim_data_out[i].abilities[j] = j;
      }


      /* Create a file with a compound type which contains an array of
       * int. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct dim_rec), "SFDimRec", &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, "starfleet_id",
			     NC_COMPOUND_OFFSET(struct dim_rec, starfleet_id), NC_INT)) ERR;
      if (nc_insert_array_compound(ncid, typeid, "abilities",
			     NC_COMPOUND_OFFSET(struct dim_rec, abilities), NC_INT, 1, dim_sizes)) ERR;
      if (nc_inq_compound_field(ncid, typeid, 1, name, &offset, &field_typeid,
				&field_ndims, field_sizes)) ERR;
      if (strcmp(name, "abilities") || offset != 4 || field_typeid != NC_INT ||
	  field_ndims != 1 || field_sizes[0] != dim_sizes[0]) ERR;
      if (nc_def_dim(ncid, STARDATE, DIM_LEN, &dimid)) ERR;
      if (nc_def_var(ncid, "dimension_data", typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, dim_data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Open the file and take a look. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_inq_var(ncid, 0, name, &xtype, &ndims, dimids, &natts)) ERR;
      if (strcmp(name, "dimension_data") || ndims != 1 || natts != 0 || dimids[0] != 0) ERR;
      if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
      if (nfields != 2 || size != sizeof(struct dim_rec) || strcmp(name, "SFDimRec")) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_typeid,
				&field_ndims, field_sizes)) ERR;
      if (strcmp(name, "abilities") || offset != 4 || field_typeid != NC_INT ||
	  field_ndims != 1 || field_sizes[0] != NUM_DIMENSIONS) ERR;
      if (nc_get_var(ncid, varid, dim_data_in)) ERR;
      for (i=0; i<DIM_LEN; i++)
      {
	 if (dim_data_in[i].starfleet_id != dim_data_out[i].starfleet_id) ERR;
	 for (j = 0; j < NUM_DIMENSIONS; j++)
	    if (dim_data_in[i].abilities[j] != dim_data_out[i].abilities[j]) ERR;
      }
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing compound var containing compound type...");
   {
      int ncid;
      size_t nfields;
      char name[NC_MAX_NAME + 1];
      size_t size, len;
      nc_type xtype, field_xtype;
      int field_ndims, field_sizes[NC_MAX_DIMS];
      size_t offset;
      nc_type svc_recid, hr_recid;
      int dim_sizes[] = {NC_MAX_NAME + 1};
      int i;

      /* The following structs are written and read as compound types by
       * the tests below. */
      struct s1
      {
	    int i1;
	    int i2;
      };
      struct s1 data[DIM_LEN];

      /* StarFleet Human Resources Department has data records for all
       * employees. */
      struct hr_rec
      {
	    int starfleet_id;
	    struct s1 svc_rec;
	    char name[NC_MAX_NAME + 1];
	    float max_temp, min_temp; /* temperature range */
	    double percent_transporter_errosion;
      };
      struct hr_rec hr_data_out[DIM_LEN], hr_data_in[DIM_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct hr_rec), DIM_LEN)))
	 return NC_ENOMEM;
      memcpy((void *)hr_data_out, (void *)dummy, sizeof(struct hr_rec) * DIM_LEN);
      free(dummy);

      /* Create some phony data. */
      for (i=0; i<DIM_LEN; i++)
      {
	 data[i].i1 = 5;
	 data[i].i2 = 10;
	 /* hr data */
	 hr_data_out[i].starfleet_id = i;
	 hr_data_out[i].svc_rec = data[i];
	 if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
	 hr_data_out[i].max_temp = 99.99;
	 hr_data_out[i].min_temp = -9.99;
	 hr_data_out[i].percent_transporter_errosion = .030303;
      }

      /* Create a file with a nested compound type attribute and variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Define the inner type first. */
      if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &svc_recid)) ERR;
      if (nc_insert_compound(ncid, svc_recid, BATTLES_WITH_KLINGONS,
			     NC_COMPOUND_OFFSET(struct s1, i1), NC_INT)) ERR;
      if (nc_insert_compound(ncid, svc_recid, DATES_WITH_ALIENS,
			     NC_COMPOUND_OFFSET(struct s1, i2), NC_INT)) ERR;

      /* Now define the containing type. */
      if (nc_def_compound(ncid, sizeof(struct hr_rec), "SF_HR_Record", &hr_recid)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "StarFleet_ID",
			     NC_COMPOUND_OFFSET(struct hr_rec, starfleet_id), NC_INT)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "Service_Record",
			     NC_COMPOUND_OFFSET(struct hr_rec, svc_rec), svc_recid)) ERR;
      if (nc_insert_array_compound(ncid, hr_recid, "Name",
				   NC_COMPOUND_OFFSET(struct hr_rec, name), NC_CHAR, 1, dim_sizes)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "Max_Temp",
			     NC_COMPOUND_OFFSET(struct hr_rec, max_temp), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "Min_Temp",
			     NC_COMPOUND_OFFSET(struct hr_rec, min_temp), NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "Percent_Transporter_Erosian",
			     NC_COMPOUND_OFFSET(struct hr_rec, percent_transporter_errosion),
			     NC_DOUBLE)) ERR;

      /* Write it as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, "HR_Records", hr_recid, DIM_LEN,
		     hr_data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Read the att and check values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, "HR_Records", hr_data_in)) ERR;
      for (i=0; i<DIM_LEN; i++)
      {
	 if (hr_data_in[i].starfleet_id != hr_data_out[i].starfleet_id ||
	     hr_data_in[i].svc_rec.i1 != hr_data_out[i].svc_rec.i1 ||
	     hr_data_in[i].svc_rec.i2 != hr_data_out[i].svc_rec.i2 ||
	     strcmp(hr_data_in[i].name, hr_data_out[i].name) ||
	     hr_data_in[i].max_temp != hr_data_out[i].max_temp ||
	     hr_data_in[i].min_temp != hr_data_out[i].min_temp ||
	     hr_data_in[i].percent_transporter_errosion !=
	     hr_data_out[i].percent_transporter_errosion) ERR;
      }
      
      /* Use the inq functions to learn about nested compound type. */
      if (nc_inq_att(ncid, NC_GLOBAL, "HR_Records", &xtype, &len)) ERR;
      if (len != DIM_LEN) ERR;
      if (nc_inq_compound(ncid, xtype, name, &size, &nfields)) ERR;
      if (nfields != 6 || size != sizeof(struct hr_rec) || strcmp(name, "SF_HR_Record")) ERR;
      if (nc_inq_compound_field(ncid, xtype, 0, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (strcmp(name, "StarFleet_ID") || offset != 0 || field_xtype != NC_INT) ERR;
      if (nc_inq_compound_field(ncid, xtype, 1, name, &offset, &field_xtype, &field_ndims, field_sizes)) ERR;
      if (strcmp(name, "Service_Record") || offset != NC_COMPOUND_OFFSET(struct hr_rec, svc_rec)) ERR;
      /* Check the internal compound type. */
      
      /* Finish checking the containing compound type. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** creating compound test file...");

   {
#define TYPE6_NAME "obs_t"
#define DIM6_NAME "n"
#define DIM6_LEN 3
#define VAR6_NAME "obs"
#define VAR6_RANK 1
#define ATT6_NAME "_FillValue"
#define ATT6_LEN  1
      int ncid;
      int dimid, varid;
      nc_type typeid;
      char name_in[NC_MAX_NAME+1];
      int i;

      int var_dims[VAR6_RANK];

      typedef struct obs_t {
	    char day ;
	    short elev;
	    int count;
	    float relhum;
	    double time;
      } obs_t ;
      obs_t obsdata[DIM6_LEN];
      char *dummy;

      obs_t missing_val;
      obs_t val_in;
      size_t size_in;
      size_t nfields_in;
      nc_type class_in;
      int ntypes;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct obs_t), DIM6_LEN)))
	 return NC_ENOMEM;
      memcpy((void *)obsdata, (void *)dummy, sizeof(struct obs_t) * DIM6_LEN);
      memcpy((void *)&missing_val, (void *)dummy, sizeof(struct obs_t));
      free(dummy);

      /* Initialize data. */
      for (i = 0; i < DIM6_LEN; i++)
      {
	 obsdata[i].day = 15 * i + 1;
	 obsdata[i].elev = 2 * i + 1;
	 obsdata[i].count = 2 * i + 1;
	 obsdata[i].relhum = 2.0 * i + 1;
	 obsdata[i].time = 2.0 * i + 1;
      }
      missing_val.day = 99;
      missing_val.elev = 99;
      missing_val.count = 99;
      missing_val.relhum = 99.;
      missing_val.time = 99.;

      if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

      /* Create a compound type. */
      if (nc_def_compound(ncid, sizeof(obs_t), TYPE6_NAME, &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, "day", NC_COMPOUND_OFFSET(struct obs_t, day),
			     NC_BYTE)) ERR;
      if (nc_insert_compound(ncid, typeid, "elev", NC_COMPOUND_OFFSET(struct obs_t, elev),
			     NC_SHORT)) ERR;
      if (nc_insert_compound(ncid, typeid, "count", NC_COMPOUND_OFFSET(struct obs_t, count),
			     NC_INT)) ERR;
      if (nc_insert_compound(ncid, typeid, "relhum", NC_COMPOUND_OFFSET(struct obs_t, relhum),
			     NC_FLOAT)) ERR;
      if (nc_insert_compound(ncid, typeid, "time", NC_COMPOUND_OFFSET(struct obs_t, time),
			     NC_DOUBLE)) ERR;

      /* Declare a dimension for number of obs */
      if (nc_def_dim(ncid, DIM6_NAME, DIM6_LEN, &dimid)) ERR;

      /* Declare a variable of the compound type */
      var_dims[0] = dimid;
      if (nc_def_var(ncid, VAR6_NAME, typeid, VAR6_RANK, var_dims, &varid)) ERR;

      /* Create and write a variable attribute of the compound type */
      if (nc_put_att(ncid, varid, ATT6_NAME, typeid, ATT6_LEN, (void *) &missing_val)) ERR;
      if (nc_enddef(ncid)) ERR;

      /* Store data, writing all values in one call */
      if(nc_put_var(ncid, varid, obsdata)) ERR;

      /* Write the file. */
      if (nc_close(ncid)) ERR;

      /* Check it out. */
   
      /* Reopen the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_typeids(ncid, &ntypes, &typeid)) ERR;
      if (ntypes != 1) ERR;

      /* Get info with the generic inquire for user-defined types */
      if (nc_inq_user_type(ncid, typeid, name_in, NULL, NULL,
			   NULL, &class_in)) ERR;
      if (strcmp(name_in, TYPE6_NAME) ||
	  class_in != NC_COMPOUND) ERR;

      /* Get the same info with the compound-specific inquire function */
      if (nc_inq_compound(ncid, typeid, name_in, &size_in, &nfields_in)) ERR;
      if (strcmp(name_in, TYPE6_NAME) ||
	  size_in != sizeof(obs_t) ||
	  nfields_in != 5) ERR;

      if (nc_inq_varid(ncid, VAR6_NAME, &varid)) ERR;

      /* Read in attribute value and check it */
      if (nc_get_att(ncid, varid, ATT6_NAME, &val_in)) ERR;
      if (val_in.day != missing_val.day ||
	  val_in.elev != missing_val.elev ||
	  val_in.count != missing_val.count ||
	  val_in.relhum != missing_val.relhum) ERR;

      /* Read in each value and check */
      for (i = 0; i < DIM6_LEN; i++) {
	 size_t index[VAR6_RANK];
	 index[0] = i;
	 if (nc_get_var1(ncid, varid, index, (void *) &val_in)) ERR;
	 /* TODO: check values */
	 if (val_in.day != obsdata[i].day) ERR;
	 if (val_in.elev != obsdata[i].elev) ERR;
	 if (val_in.count != obsdata[i].count) ERR;
	 if (val_in.relhum != obsdata[i].relhum) ERR;
	 if (val_in.time != obsdata[i].time) ERR;
      }
   
      if (nc_close(ncid)) ERR;

   }

   SUMMARIZE_ERR;
   printf("*** Now opening the ref file for this...");
   {
#define REF_FILE_NAME "ref_tst_compounds.nc"
      int ncid;
      int varid;
      nc_type typeid;
      char name_in[NC_MAX_NAME+1];
      int i;

      typedef struct obs_t {
	    char day ;
	    short elev;
	    int count;
	    float relhum;
	    double time;
      } obs_t ;

      obs_t obsdata[DIM6_LEN] = {
	 {15, 2, 1, 0.5, 3600.01},
	 {-99, -99, -99, -99.0f, -99.0},
	 {20, 6, 3, 0.75, 5000.01}
      };
      obs_t missing_val = {-99, -99, -99, -99, -99};
      obs_t val_in;
      size_t size_in;
      size_t nfields_in;
      nc_type class_in;
      int ntypes;
      char file_in[NC_MAX_NAME * 2];
      
      if (getenv("srcdir"))
      {
	 strcpy(file_in, getenv("srcdir"));
	 strcat(file_in, "/");
	 strcat(file_in, REF_FILE_NAME);
      }
      else
	 strcpy(file_in, REF_FILE_NAME);

      /* Reopen the file. */
      if (nc_open(file_in, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_typeids(ncid, &ntypes, &typeid)) ERR;
      if (ntypes != 1) ERR;

      /* Get info with the generic inquire for user-defined types */
      if (nc_inq_user_type(ncid, typeid, name_in, NULL, NULL,
			   NULL, &class_in)) ERR;
      if (strcmp(name_in, TYPE6_NAME) ||
	  class_in != NC_COMPOUND) ERR;

      /* Get the same info with the compound-specific inquire function */
      if (nc_inq_compound(ncid, typeid, name_in, &size_in, &nfields_in)) ERR;
      if (strcmp(name_in, TYPE6_NAME) || size_in != sizeof(obs_t) ||
	  nfields_in != 5) ERR;

      if (nc_inq_varid(ncid, VAR6_NAME, &varid)) ERR;

      /* Read in attribute value and check it */
      if (nc_get_att(ncid, varid, ATT6_NAME, &val_in)) ERR;
      if (val_in.day != missing_val.day ||
	  val_in.elev != missing_val.elev ||
	  val_in.count != missing_val.count ||
	  val_in.relhum != missing_val.relhum) ERR;

      /* Read in each value and check */
      for (i = 0; i < DIM6_LEN; i++) {
	 size_t index[VAR6_RANK];
	 index[0] = i;
	 if (nc_get_var1(ncid, varid, index, (void *) &val_in)) ERR;
	 /* TODO: check values */
	 if (val_in.day != obsdata[i].day) ERR;
	 if (val_in.elev != obsdata[i].elev) ERR;
	 if (val_in.count != obsdata[i].count) ERR;
	 if (val_in.relhum != obsdata[i].relhum) ERR;
	 if (val_in.time != obsdata[i].time) ERR;
      }
   
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing compound var containing char array...");
   {
#define DIM1_LEN 1
      int ncid;
      size_t len;
      nc_type xtype;
      nc_type hr_recid;
      int dim_sizes[] = {NC_MAX_NAME + 1};
      int i;

      /* StarFleet Human Resources Department has data records for all
       * employees. */
      struct hr_rec
      {
	    char name[NC_MAX_NAME + 1];
	    float max_temp;
      };
      struct hr_rec hr_data_out[DIM1_LEN], hr_data_in[DIM1_LEN];
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct hr_rec), DIM1_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)hr_data_out, (void *)dummy, sizeof(struct hr_rec) * DIM1_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
	 if (sprintf(hr_data_out[i].name, "alien_%d", i) < 0) ERR;
	 hr_data_out[i].max_temp = 99.99;
      }

      /* Create a file with a nested compound type attribute and variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Now define the compound type. */
      /*printf("sizeof(struct hr_rec)=%d\n", sizeof(struct hr_rec));*/
      if (nc_def_compound(ncid, sizeof(struct hr_rec), "SF_HR_Record", &hr_recid)) ERR;
      if (nc_insert_array_compound(ncid, hr_recid, "Name",
				   NC_COMPOUND_OFFSET(struct hr_rec, name), NC_CHAR, 1, dim_sizes)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "Max_Temp",
			     NC_COMPOUND_OFFSET(struct hr_rec, max_temp), NC_FLOAT)) ERR;

      /* Write it as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, "HR_Records", hr_recid, DIM1_LEN, hr_data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Read the att and check values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, "HR_Records", hr_data_in)) ERR;
      for (i=0; i<DIM1_LEN; i++)
	 if (strcmp(hr_data_in[i].name, hr_data_out[i].name) ||
	     hr_data_in[i].max_temp != hr_data_out[i].max_temp) ERR;
      
      /* Use the inq functions to learn about the compound type. */
      if (nc_inq_att(ncid, NC_GLOBAL, "HR_Records", &xtype, &len)) ERR;
      if (len != DIM1_LEN) ERR;
      
      /* Finish checking the containing compound type. */
      if (nc_close(ncid)) ERR;
   }

   SUMMARIZE_ERR;
   printf("*** testing compound var containing byte array...");
   {
#define DIM1_LEN 1
#define ARRAY_LEN (NC_MAX_NAME + 1)
      int ncid;
      size_t len;
      nc_type xtype;
      nc_type hr_recid;
      int dim_sizes[] = {ARRAY_LEN};
      int i, j;
      char *dummy;
      /* StarFleet Human Resources Department has data records for all
       * employees. */
      struct hr_rec
      {
	    char name[ARRAY_LEN];
	    float max_temp;
      };
      struct hr_rec hr_data_out[DIM1_LEN], hr_data_in[DIM1_LEN];

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct hr_rec), DIM1_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)hr_data_out, (void *)dummy, sizeof(struct hr_rec) * DIM1_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (i = 0; i < DIM1_LEN; i++)
      {
	 hr_data_out[i].max_temp = 99.99;
	 for (j = 0; j < ARRAY_LEN; j++)
	    hr_data_out[i].name[j] = j;
      }

      /* Create a file with a nested compound type attribute and variable. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Now define the compound type. */
      /*printf("sizeof(struct hr_rec)=%d\n", sizeof(struct hr_rec));*/
      if (nc_def_compound(ncid, sizeof(struct hr_rec), "SF_HR_Record", &hr_recid)) ERR;
      if (nc_insert_array_compound(ncid, hr_recid, "Name",
				   NC_COMPOUND_OFFSET(struct hr_rec, name), NC_UBYTE, 1, dim_sizes)) ERR;
      if (nc_insert_compound(ncid, hr_recid, "Max_Temp",
			     NC_COMPOUND_OFFSET(struct hr_rec, max_temp), NC_FLOAT)) ERR;

      /* Write it as an attribute. */
      if (nc_put_att(ncid, NC_GLOBAL, "HR_Records", hr_recid, DIM1_LEN, hr_data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Read the att and check values. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, "HR_Records", hr_data_in)) ERR;
      for (i = 0; i < DIM1_LEN; i++)
      {
	 if (hr_data_in[i].max_temp != hr_data_out[i].max_temp) ERR;
	 for (j = 0; j < ARRAY_LEN; j++)
	    if (hr_data_in[i].name[j] != hr_data_out[i].name[j]) ERR;
      }
      
      /* Use the inq functions to learn about the compound type. */
      if (nc_inq_att(ncid, NC_GLOBAL, "HR_Records", &xtype, &len)) ERR;
      if (len != DIM1_LEN) ERR;
      
      /* Finish checking the containing compound type. */
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing with user-contributed code...");
   {
#define DATA_LEN 1
      int ncid, typeid, varid, dimid;
      int dimids[] = {0};
      struct s1
      {
	 short i;
	 long long j;
      };
      struct s1 data_out[DATA_LEN], data_in[DATA_LEN];
      int idx;
      char *dummy;

      /* REALLY initialize the data (even the gaps in the structs). This
       * is only needed to pass valgrind. */
      if (!(dummy = calloc(sizeof(struct s1), DATA_LEN))) 
	 return NC_ENOMEM; 
      memcpy((void *)data_out, (void *)dummy, sizeof(struct s1) * DATA_LEN); 
      free(dummy); 

      /* Create some phony data. */
      for (idx = 0; idx < DATA_LEN; idx++)
      {
	 data_out[idx].i = 20000;
	 data_out[idx].j = 300000;
      }

      /* Create a file with a compound type. Write a little data. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_compound(ncid, sizeof(struct s1), "cmp1", &typeid)) ERR;
      if (nc_insert_compound(ncid, typeid, "i",
			     NC_COMPOUND_OFFSET(struct s1, i), NC_SHORT)) ERR;
      if (nc_insert_compound(ncid, typeid, "j",
			     NC_COMPOUND_OFFSET(struct s1, j), NC_INT64)) ERR;
      if (nc_def_dim(ncid, "phony_dim", 1, &dimid)) ERR;
      if (nc_def_var(ncid, "phony_var", typeid, 1, dimids, &varid)) ERR;
      if (nc_put_var(ncid, varid, data_out)) ERR;
      if (nc_close(ncid)) ERR;

      /* Reopen the file and check it. */
      if (nc_open(FILE_NAME, NC_WRITE, &ncid)) ERR;
      if (nc_get_var(ncid, 0, data_in)) ERR;
      for (idx = 0; idx < DATA_LEN; idx++)
      {
	 if (data_in[idx].i != data_out[idx].i ||
	     data_in[idx].j != data_out[idx].j) ERR;
      }
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


