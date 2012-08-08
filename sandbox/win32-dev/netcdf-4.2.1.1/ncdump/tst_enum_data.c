/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with an enum type and enum data for ncdump to read.

   $Id$
*/

#include <nc_tests.h>
#include <netcdf.h>

#define FILE2_NAME "tst_enum_data.nc"
#define TYPE2_NAME "cloud_class_t"
#define DIM2_NAME "station"
#define DIM2_LEN 5
#define VAR2_NAME "primary_cloud"
#define VAR2_RANK 1
#define ATT2_NAME "_FillValue"
#define ATT2_LEN  1

int
main(int argc, char **argv)
{
   int ncid;
   int dimid, varid;
   nc_type typeid;
   int num_members;
   char name_in[NC_MAX_NAME+1];
   nc_type base_nc_type_in;
   size_t nfields_in, base_size_in, num_members_in;
   int class_in;
   unsigned char value_in;

   int i;

   enum clouds {		/* a C enumeration */
       CLEAR=0,
       CUMULONIMBUS=1,
       STRATUS=2,
       STRATOCUMULUS=3,
       CUMULUS=4,
       ALTOSTRATUS=5,
       NIMBOSTRATUS=6,
       ALTOCUMULUS=7,
       CIRROSTRATUS=8,
       CIRROCUMULUS=9,
       CIRRUS=10,
       MISSING=255};

   struct {
       char *name;
       unsigned char value;
   } cloud_types[] = {
       {"Clear", CLEAR}, 
       {"Cumulonimbus", CUMULONIMBUS}, 
       {"Stratus", STRATUS}, 
       {"Stratocumulus", STRATOCUMULUS}, 
       {"Cumulus", CUMULUS}, 
       {"Altostratus", ALTOSTRATUS}, 
       {"Nimbostratus", NIMBOSTRATUS}, 
       {"Altocumulus", ALTOCUMULUS}, 
       {"Cirrostratus", CIRROSTRATUS}, 
       {"Cirrocumulus", CIRROCUMULUS}, 
       {"Cirrus", CIRRUS}, 
       {"Missing", MISSING}
   };
   int var_dims[VAR2_RANK];
   unsigned char att_val;
   unsigned char cloud_data[DIM2_LEN] = {
       CLEAR, STRATUS, CLEAR, CUMULONIMBUS, MISSING};
   unsigned char cloud_data_in[DIM2_LEN];

   printf("\n*** Testing enums.\n");
   printf("*** creating enum test file %s...", FILE2_NAME);
   /*nc_set_log_level(3);*/
   if (nc_create(FILE2_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

   /* Create an enum type. */
   if (nc_def_enum(ncid, NC_UBYTE, TYPE2_NAME, &typeid)) ERR;
   num_members = (sizeof cloud_types) / (sizeof cloud_types[0]);
   for (i = 0; i < num_members; i++)
       if (nc_insert_enum(ncid, typeid, cloud_types[i].name,
			  &cloud_types[i].value)) ERR;

   /* Declare a station dimension */
   if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimid)) ERR;
   /* Declare a variable of the enum type */
   var_dims[0] = dimid;
   if (nc_def_var(ncid, VAR2_NAME, typeid, VAR2_RANK, var_dims, &varid)) ERR;
   /* Create and write a variable attribute of the enum type */
   att_val = MISSING;
   if (nc_put_att(ncid, varid, ATT2_NAME, typeid, ATT2_LEN, &att_val)) ERR;
   if (nc_enddef(ncid)) ERR;
   /* Store some data of the enum type */
   if(nc_put_var(ncid, varid, cloud_data)) ERR;
   /* Write the file. */
   if (nc_close(ncid)) ERR;

   /* Check it out. */
   
   /* Reopen the file. */
   if (nc_open(FILE2_NAME, NC_NOWRITE, &ncid)) ERR;

   if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
   if (strcmp(name_in, TYPE2_NAME) || 
       base_size_in != sizeof(unsigned char) ||
       base_nc_type_in != NC_UBYTE || 
       nfields_in != num_members || 
       class_in != NC_ENUM) ERR;
   if (nc_inq_enum(ncid, typeid, name_in, 
		   &base_nc_type_in, &base_size_in, &num_members_in)) ERR;
   if (strcmp(name_in, TYPE2_NAME) || 
       base_nc_type_in !=  NC_UBYTE || 
       num_members_in != num_members) ERR;
   for (i = 0; i < num_members; i++)
   {
       if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
       if (strcmp(name_in, cloud_types[i].name) || 
	   value_in != cloud_types[i].value) ERR;
       if (nc_inq_enum_ident(ncid, typeid, cloud_types[i].value, 
			     name_in)) ERR;
       if (strcmp(name_in, cloud_types[i].name)) ERR;
   }
   if (nc_inq_varid(ncid, VAR2_NAME, &varid)) ERR;

   if (nc_get_att(ncid, varid, ATT2_NAME, &value_in)) ERR;
   if (value_in != MISSING) ERR;

   if(nc_get_var(ncid, varid, cloud_data_in)) ERR;
   for (i = 0; i < DIM2_LEN; i++) {
       if (cloud_data_in[i] != cloud_data[i]) ERR;
   }
   
   if (nc_close(ncid)) ERR; 
   
   
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

