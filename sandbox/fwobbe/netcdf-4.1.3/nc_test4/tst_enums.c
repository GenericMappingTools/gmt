/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 enum types.

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_enums.nc"
#define DIM_LEN 4
#define NUM_MEMBERS 4
#define DIM_NAME "dim"
#define BASE_SIZE 20
#define VAR_NAME "Advice"
#define TYPE_NAME "Mysterous_Word"

int
main(int argc, char **argv)
{
   int ncid;
   nc_type typeid;
   int i;
   char name_in[NC_MAX_NAME+1];
   int ntypes, typeids[1] = {0};
   nc_type base_nc_type, base_nc_type_in;
   size_t nfields_in, num_members, base_size_in;
   int class_in;

   printf("\n*** Testing netcdf-4 enum type.\n");
   printf("*** testing creation of enum type...");
   {
      int value_in;
      /* Can't use the same name twice! */
      char member_name[NUM_MEMBERS][NC_MAX_NAME + 1] = {"Mene1", "Mene2", 
							"Tekel", "Upharsin"};
      int member_value[NUM_MEMBERS] = {0, 99, 81232, 12};

      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an enum type. */
      if (nc_def_enum(ncid, NC_INT, TYPE_NAME, &typeid)) ERR;
      for (i = 0; i < NUM_MEMBERS; i++)
	 if (nc_insert_enum(ncid, typeid, member_name[i], 
			    &member_value[i])) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int) ||
	  base_nc_type_in != NC_INT || nfields_in != NUM_MEMBERS || class_in != NC_ENUM) ERR;
      if (nc_inq_enum(ncid, typeid, name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_nc_type != NC_INT || 
	  num_members != NUM_MEMBERS) ERR;
      for (i = 0; i < NUM_MEMBERS; i++)
      {
	 if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
	 if (strcmp(name_in, member_name[i]) || value_in != member_value[i]) ERR;
	 if (nc_inq_enum_ident(ncid, typeid, member_value[i], name_in)) ERR;
	 if (strcmp(name_in, member_name[i])) ERR;
      }

      /* Write the file. */
      if (nc_close(ncid)) ERR;

      /* Reopen the file. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Get type info. */
      if (nc_inq_typeids(ncid, &ntypes, typeids)) ERR;
      if (ntypes != 1 || !typeids[0]) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, typeids[0], name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int) ||
	  base_nc_type_in != NC_INT || nfields_in != NUM_MEMBERS || class_in != NC_ENUM) ERR;
      if (nc_inq_type(ncid, typeids[0], name_in, &base_size_in)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int)) ERR;
      if (nc_inq_enum(ncid, typeids[0], name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, TYPE_NAME) || base_nc_type != NC_INT || num_members != NUM_MEMBERS) ERR;
      for (i = 0; i < NUM_MEMBERS; i++)
      {
	 if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
	 if (strcmp(name_in, member_name[i]) || value_in != member_value[i]) ERR;
	 if (nc_inq_enum_ident(ncid, typeid, member_value[i], name_in)) ERR;
	 if (strcmp(name_in, member_name[i])) ERR;
      }

      if (nc_close(ncid)) ERR; 
   }

   SUMMARIZE_ERR;

#define NUM_BRADYS 9
#define BRADYS "Bradys"
#define BRADY_DIM_LEN 3
#define ATT_NAME "brady_attribute"

   printf("*** testing enum attribute...");
   {
      char brady_name[NUM_BRADYS][NC_MAX_NAME + 1] = {"Mike", "Carol", "Greg", "Marsha",
						       "Peter", "Jan", "Bobby", "Whats-her-face",
						       "Alice"};
      unsigned char brady_value[NUM_BRADYS] = {8, 7, 6 , 5, 4, 3, 2, 1, 0};
      unsigned char data[BRADY_DIM_LEN] = {0, 4, 8};
      unsigned char value_in;
      
      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an enum type based on unsigned bytes. */
      if (nc_def_enum(ncid, NC_UBYTE, BRADYS, &typeid)) ERR;
      for (i = 0; i < NUM_BRADYS; i++)
	 if (nc_insert_enum(ncid, typeid, brady_name[i], 
			    &brady_value[i])) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, BRADYS) || base_size_in != 1 ||
	  base_nc_type_in != NC_UBYTE || nfields_in != NUM_BRADYS || class_in != NC_ENUM) ERR;
      if (nc_inq_enum(ncid, typeid, name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, BRADYS) || base_nc_type != NC_UBYTE || base_size_in != 1 ||
	  num_members != NUM_BRADYS) ERR;
      for (i = 0; i < NUM_BRADYS; i++)
      {
	 if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
	 if (strcmp(name_in, brady_name[i]) || value_in != brady_value[i]) ERR;
	 if (nc_inq_enum_ident(ncid, typeid, brady_value[i], name_in)) ERR;
	 if (strcmp(name_in, brady_name[i])) ERR;
      }

      /* Write an att of this enum type. */
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, typeid, BRADY_DIM_LEN, data)) ERR;
      
      /* Close the file. */
      if (nc_close(ncid)) ERR;

      /* Reopen. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, BRADYS) || base_size_in != 1 ||
	  base_nc_type_in != NC_UBYTE || nfields_in != NUM_BRADYS || class_in != NC_ENUM) ERR;
      if (nc_inq_enum(ncid, typeid, name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, BRADYS) || base_nc_type != NC_UBYTE || base_size_in != 1 ||
	  num_members != NUM_BRADYS) ERR;
      for (i = 0; i < NUM_BRADYS; i++)
      {
	 if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
	 if (strcmp(name_in, brady_name[i]) || value_in != brady_value[i]) ERR;
	 if (nc_inq_enum_ident(ncid, typeid, brady_value[i], name_in)) ERR;
	 if (strcmp(name_in, brady_name[i])) ERR;
      }

      if (nc_close(ncid)) ERR; 
   }

   SUMMARIZE_ERR;

#define TYPE2_NAME "cloud_type"
#define DIM2_NAME "station"
#define DIM2_LEN 5
#define VAR2_NAME "primary_cloud"
#define VAR2_RANK 1
#define ATT2_NAME "_FillValue"
#define ATT2_LEN  1

   printf("*** testing enum fill value ...");
   {
       int dimid, varid;
       size_t num_members_in;
       int class_in;
       unsigned char value_in;
       unsigned char att_value_in;

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

       if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;

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
       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

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

       if (nc_get_att(ncid, varid, ATT2_NAME, &att_value_in)) ERR;
       if (att_value_in != MISSING) ERR;

       if(nc_get_var(ncid, varid, cloud_data_in)) ERR;
       for (i = 0; i < DIM2_LEN; i++) {
	   if (cloud_data_in[i] != cloud_data[i]) ERR;
       }
   
       if (nc_close(ncid)) ERR; 
   }

   SUMMARIZE_ERR;
   printf("*** testing enum interuptus...");
   {
#define GEEKY_NAME "Galadriel"
      
      /* Create a file. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;

      /* Create an enum type based on unsigned bytes. */
      if (nc_def_enum(ncid, NC_UBYTE, GEEKY_NAME, &typeid)) ERR;

      /* Check it out. */
      if (nc_inq_user_type(ncid, typeid, name_in, &base_size_in, &base_nc_type_in,
			   &nfields_in, &class_in)) ERR;
      if (strcmp(name_in, GEEKY_NAME) || base_size_in != 1 ||
	  base_nc_type_in != NC_UBYTE || nfields_in != 0 || class_in != NC_ENUM) ERR;
      if (nc_inq_enum(ncid, typeid, name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
      if (strcmp(name_in, GEEKY_NAME) || base_nc_type != NC_UBYTE || base_size_in != 1 ||
	  num_members != 0) ERR;

      /* Close the file. */
      if (nc_close(ncid) != NC_EINVAL) ERR;

   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

