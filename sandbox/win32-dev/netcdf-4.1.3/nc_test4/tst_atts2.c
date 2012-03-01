/* This is part of the netCDF package. Copyright 2005-2007 University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test copy of attributes. 

   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME1 "tst_atts2.nc"
#define FILE_NAME2 "tst_atts2_2.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 attribute copies.\n");
   printf("*** testing compound attribute copy to different type of same name...");
   {
#define CMP_NAME1 "Ireland"
#define I1_NAME "Cork"
#define I2_NAME "Dublin"
#define DIM_LEN 3
#define ATT_NAME3 "Rain"
#define NUM_FILES 2

      int ncid[NUM_FILES], typeid;
      char file_name[NUM_FILES][NC_MAX_NAME + 1] = {FILE_NAME1, FILE_NAME2};
      int i;
      struct s1 
      {
	    int i1;
	    int i2;
      };
      struct s1 data[DIM_LEN];
      struct s2
      {
	    short i1;
	    int i2;
      };

      /* Create some phony data. */   
      for (i = 0; i < DIM_LEN; i++)
      {
	 data[i].i1 = 32768;
	 data[i].i2 = 32767;
      }

      /* Create two files with different compound types of the same name. */
      for (i = 0; i < NUM_FILES; i++)
	 if (nc_create(file_name[i], NC_NETCDF4, &ncid[i])) ERR;

      /* Define s1 in file 1. */
      if (nc_def_compound(ncid[0], sizeof(struct s1), CMP_NAME1, &typeid)) ERR;
      if (nc_insert_compound(ncid[0], typeid, I1_NAME, 
			     NC_COMPOUND_OFFSET(struct s1, i1), NC_INT)) ERR;
      if (nc_insert_compound(ncid[0], typeid, I2_NAME, 
			     NC_COMPOUND_OFFSET(struct s1, i2), NC_INT)) ERR;

      /* Define s2 in file 2, but named the same as s1. */
      if (nc_def_compound(ncid[1], sizeof(struct s2), CMP_NAME1, &typeid)) ERR;
      if (nc_insert_compound(ncid[1], typeid, I1_NAME, 
			     NC_COMPOUND_OFFSET(struct s2, i1), NC_SHORT)) ERR;
      if (nc_insert_compound(ncid[1], typeid, I2_NAME, 
			     NC_COMPOUND_OFFSET(struct s2, i2), NC_INT)) ERR;


      /* Write an att in one file. */
      if (nc_put_att(ncid[0], NC_GLOBAL, ATT_NAME3, typeid, DIM_LEN, 
		     data)) ERR;

      /* Try to copy. It must fail, because the two types are not the
       * same. */
      if (nc_copy_att(ncid[0], NC_GLOBAL, ATT_NAME3, ncid[1], 
		      NC_GLOBAL) != NC_EBADTYPE) ERR;

      /* Close the files. */
      for (i = 0; i < NUM_FILES; i++)
	 if (nc_close(ncid[i])) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing string attribute copy...");
   {
#define ATT_NAME "Irish_Leader"
#define ATT_LEN 1      
      int ncid1, ncid2;
      const char *mc[ATT_LEN] = {"Michael Collins"};
      char *mc_in;

      /* Create a file with a string att. */
      if (nc_create(FILE_NAME1, NC_NETCDF4, &ncid1)) ERR;
      if (nc_put_att_string(ncid1, NC_GLOBAL, ATT_NAME, ATT_LEN, mc)) ERR;
      
      /* Create another file, and copy the att. */
      if (nc_create(FILE_NAME2, NC_NETCDF4, &ncid2)) ERR;
      if (nc_copy_att(ncid1, NC_GLOBAL, ATT_NAME, ncid2, NC_GLOBAL)) ERR;

      /* Close up. */ 
      if (nc_close(ncid1)) ERR;
      if (nc_close(ncid2)) ERR;

      /* Reopen file 2 and see that attribute is correct. */
      if (nc_open(FILE_NAME2, 0, &ncid2)) ERR;
      if (nc_get_att_string(ncid2, NC_GLOBAL, ATT_NAME, &mc_in)) ERR;
      if (strcmp(mc[0], mc_in)) ERR;
      if (nc_close(ncid2)) ERR;
      if (nc_free_string(ATT_LEN, &mc_in)) ERR;

  }
   SUMMARIZE_ERR;
   printf("*** testing non-string atomic attribute copy...");
   {
#define ATT_LEN 1      
#define NUM_ATT 5
      int ncid1, ncid2;
      char name[NUM_ATT][NC_MAX_NAME + 1];
      long long data = 0, data_in[NUM_ATT];
      int a;

      /* Create a file with non-string atomic atts. */
      if (nc_create(FILE_NAME1, NC_NETCDF4, &ncid1)) ERR;
      for (a = 0; a < NUM_ATT; a++)
      {
	 sprintf(name[a], "atomic_att_type_%d", a + 1);
	 nc_put_att(ncid1, NC_GLOBAL, name[a], a + 1, ATT_LEN, 
		    (void *)&data);
      }
      
      /* Create another file, and copy the att. */
      if (nc_create(FILE_NAME2, NC_NETCDF4, &ncid2)) ERR;
      for (a = 0; a < NUM_ATT; a++)
	 if (nc_copy_att(ncid1, NC_GLOBAL, name[a], ncid2, NC_GLOBAL)) ERR;

      /* Close up. */ 
      if (nc_close(ncid1)) ERR;
      if (nc_close(ncid2)) ERR;

      /* Reopen file 2 and see that attributes are present. */
      if (nc_open(FILE_NAME2, 0, &ncid2)) ERR;
      for (a = 0; a < NUM_ATT; a++)
	 nc_get_att(ncid1, NC_GLOBAL, name[a], (void *)&data_in[a]);
      if (nc_close(ncid2)) ERR;
  }
   SUMMARIZE_ERR;
   printf("*** testing simple compound attribute copy...");
   {
#define CMP_NAME "cmp"
#define I1 "i1"
#define I2 "i2"
#define DIM_LEN 3
#define ATT_NAME2 "att"
#define NUM_FILES 2

      int ncid[NUM_FILES], typeid;
      char file_name[NUM_FILES][NC_MAX_NAME + 1] = {FILE_NAME1, FILE_NAME2};
      int i;
      struct s1 
      {
	    int i1;
	    int i2;
      };
      struct s1 data[DIM_LEN], data_in[DIM_LEN];

      /* Create some phony data. */   
      for (i = 0; i < DIM_LEN; i++)
      {
	 data[i].i1 = 5;
	 data[i].i2 = 10;
      }

      /* Create two files with the same compound type. */
      for (i = 0; i < NUM_FILES; i++)
      {
	 if (nc_create(file_name[i], NC_NETCDF4, &ncid[i])) ERR;
	 if (nc_def_compound(ncid[i], sizeof(struct s1), CMP_NAME, &typeid)) ERR;
	 if (nc_insert_compound(ncid[i], typeid, I1, 
				NC_COMPOUND_OFFSET(struct s1, i1), NC_INT)) ERR;
	 if (nc_insert_compound(ncid[i], typeid, I2, 
				NC_COMPOUND_OFFSET(struct s1, i2), NC_INT)) ERR;
      }

      /* Write an att in one file and copy it. */
      if (nc_put_att(ncid[0], NC_GLOBAL, ATT_NAME2, typeid, DIM_LEN, 
		     data)) ERR;

      if (nc_copy_att(ncid[0], NC_GLOBAL, ATT_NAME2, ncid[1], 
		      NC_GLOBAL)) ERR;

      /* Close the files. */
      for (i = 0; i < NUM_FILES; i++)
	 if (nc_close(ncid[i])) ERR;

      /* Open the second file and take a peek. */
      if (nc_open(FILE_NAME1, NC_WRITE, &ncid[1])) ERR;
      if (nc_get_att(ncid[1], NC_GLOBAL, ATT_NAME2, data_in)) ERR;

      for (i=0; i<DIM_LEN; i++)
	 if (data[i].i1 != data_in[i].i1 || data[i].i2 != data_in[i].i2) ERR;
      if (nc_close(ncid[1])) ERR;
   }
   SUMMARIZE_ERR;
   printf("*** testing simple enum attribute copy...");
   {
#define DIM_LEN_10 10
#define ATT_NAME2 "att"
#define TYPE_NAME "Irish_Province_Type"
#define NUM_FILES 2
#define NUM_MEMBERS 4

      int ncid[NUM_FILES], typeid;
      char file_name[NUM_FILES][NC_MAX_NAME + 1] = {FILE_NAME1, FILE_NAME2};
      char member_name[NUM_MEMBERS][NC_MAX_NAME + 1] = {"Munster", "Connacht", 
							"Leinster", "Ulster"};
      int member_value[NUM_MEMBERS] = {0, 1, 2, 3};
      int data_in[DIM_LEN_10], data[DIM_LEN_10] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1};
      int i, j;

      /* Create two files with the same enum type. */
      for (i = 0; i < NUM_FILES; i++)
      {
	 if (nc_create(file_name[i], NC_NETCDF4, &ncid[i])) ERR;
	 if (nc_def_enum(ncid[i], NC_INT, TYPE_NAME, &typeid)) ERR;
	 for (j = 0; j < NUM_MEMBERS; j++)
	    if (nc_insert_enum(ncid[i], typeid, member_name[j], 
			       &member_value[j])) ERR;
      }

      /* Write an att in one file and copy it. */
      if (nc_put_att(ncid[0], NC_GLOBAL, ATT_NAME2, typeid, DIM_LEN_10, 
		     data)) ERR;

      if (nc_copy_att(ncid[0], NC_GLOBAL, ATT_NAME2, ncid[1], 
		      NC_GLOBAL)) ERR;

      /* Close the files. */
      for (i = 0; i < NUM_FILES; i++)
	 if (nc_close(ncid[i])) ERR;

      /* Open the second file and take a peek. */
      if (nc_open(FILE_NAME1, NC_WRITE, &ncid[1])) ERR;
      if (nc_get_att(ncid[1], NC_GLOBAL, ATT_NAME2, data_in)) ERR;

      /* Check the data. */
      for (i = 0; i < DIM_LEN_10; i++)
	 if (data[i] != data_in[i]) ERR;
      if (nc_close(ncid[1])) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


