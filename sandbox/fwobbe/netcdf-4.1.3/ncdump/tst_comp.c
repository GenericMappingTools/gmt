/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with a compound type and compound data for ncdump to read.

   $Id$
*/


#include <nc_tests.h>
#include <netcdf.h>
#include <string.h>

#define FILE6_NAME "tst_comp.nc"
#define TYPE6_NAME "obs_t"
#define DIM6_NAME "n"
#define DIM6_LEN 3
#define VAR6_NAME "obs"
#define VAR6_RANK 1
#define ATT6_NAME "_FillValue"
#define ATT6_LEN  1

int
main(int argc, char **argv)
{
   int ncid;
   int dimid, varid;
   nc_type typeid;
   char name_in[NC_MAX_NAME+1];
   int i;

   int var_dims[VAR6_RANK];

   typedef struct obs_t {
       char day;
       short elev;
       int count;
       float relhum;
       double time;
       unsigned char category;
       unsigned short id;
       unsigned int particularity;
       long long attention_span;
   } obs_t ;

   obs_t obsdata[DIM6_LEN] = {
       {15, 2, 1, 0.5, 3600.01, 0, 0, 0, 0LL},
       {-99, -99, -99, -99.0f, -99.0, 255, 65535, 4294967295U, 
	-9223372036854775806LL},
       {20, 6, 3, 0.75, 5000.01, 200, 64000, 4220002000U, 9000000000000000000LL }
   };

   obs_t missing_val = {-99, -99, -99, -99, -99, 255, 65535, 4294967295U, 
			-9223372036854775806LL};
   obs_t val_in;
   size_t size_in;
   size_t nfields_in;
   nc_type class_in;

   printf("\n*** Testing compound types.\n");
   printf("*** creating compound test file %s...", FILE6_NAME);
   if (nc_create(FILE6_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

   /* Create a compound type. */
   if (nc_def_compound(ncid, sizeof(obs_t), TYPE6_NAME, &typeid)) ERR;
   if (nc_insert_compound(ncid, typeid, "day", 
			  NC_COMPOUND_OFFSET(struct obs_t, day), NC_BYTE)) ERR;
   if (nc_insert_compound(ncid, typeid, "elev", 
			  NC_COMPOUND_OFFSET(struct obs_t, elev), NC_SHORT)) ERR;
   if (nc_insert_compound(ncid, typeid, "count", 
			  NC_COMPOUND_OFFSET(struct obs_t, count), NC_INT)) ERR;
   if (nc_insert_compound(ncid, typeid, "relhum", 
			  NC_COMPOUND_OFFSET(struct obs_t, relhum), 
			  NC_FLOAT)) ERR;
   if (nc_insert_compound(ncid, typeid, "time", 
			  NC_COMPOUND_OFFSET(struct obs_t, time), 
			  NC_DOUBLE)) ERR;
   if (nc_insert_compound(ncid, typeid, "category", 
			  NC_COMPOUND_OFFSET(struct obs_t, category), 
			  NC_UBYTE)) ERR;
   if (nc_insert_compound(ncid, typeid, "id", 
			  NC_COMPOUND_OFFSET(struct obs_t, id), 
			  NC_USHORT)) ERR;
   if (nc_insert_compound(ncid, typeid, "particularity", 
			  NC_COMPOUND_OFFSET(struct obs_t, particularity), 
			  NC_UINT)) ERR;
   if (nc_insert_compound(ncid, typeid, "attention_span", 
			  NC_COMPOUND_OFFSET(struct obs_t, attention_span), 
			  NC_INT64)) ERR;

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
   if (nc_open(FILE6_NAME, NC_NOWRITE, &ncid)) ERR;

   /* Get info with the generic inquire for user-defined types */
   if (nc_inq_user_type(ncid, typeid, name_in, NULL, NULL, 
			NULL, &class_in)) ERR;
   if (strcmp(name_in, TYPE6_NAME) || 
       class_in != NC_COMPOUND) ERR;

   /* Get the same info with the compound-specific inquire function */
   if (nc_inq_compound(ncid, typeid, name_in, &size_in, &nfields_in)) ERR;
   if (strcmp(name_in, TYPE6_NAME) || 
       size_in != sizeof(obs_t) ||
       nfields_in != 9) ERR;

   if (nc_inq_varid(ncid, VAR6_NAME, &varid)) ERR;

   /* Read in attribute value and check it */
   if (nc_get_att(ncid, varid, ATT6_NAME, &val_in)) ERR;
   if (val_in.day != missing_val.day ||
       val_in.elev != missing_val.elev ||
       val_in.count != missing_val.count ||
       val_in.relhum != missing_val.relhum || 
       val_in.time != missing_val.time ||
       val_in.category != missing_val.category ||
       val_in.id != missing_val.id ||
       val_in.particularity != missing_val.particularity ||
       val_in.attention_span != missing_val.attention_span ) ERR;

   /* Read in each value and check */
   for (i = 0; i < DIM6_LEN; i++) {
       size_t index[VAR6_RANK];
       index[0] = i;
       if (nc_get_var1(ncid, varid, index, (void *) &val_in)) ERR;
       if (val_in.day != obsdata[i].day) ERR;
       if (val_in.elev != obsdata[i].elev) ERR;
       if (val_in.count != obsdata[i].count) ERR;
       if (val_in.relhum != obsdata[i].relhum) ERR;
       if (val_in.time != obsdata[i].time) ERR;
       if (val_in.category != obsdata[i].category) ERR;
       if (val_in.id != obsdata[i].id) ERR;
       if (val_in.particularity != obsdata[i].particularity) ERR;
       if (val_in.attention_span != obsdata[i].attention_span) ERR;
   }
   
   if (nc_close(ncid)) ERR; 
   
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

