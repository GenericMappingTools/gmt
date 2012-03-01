/* This is part of the netCDF package. Copyright 2008 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test file with default fill values for variables of each type.

   $Id$
*/

#include <nc_tests.h>
#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>

#define FILE_NAME "tst_fills2.nc" 

int
main(int argc, char **argv) 
{			/* create tst_classic_fills.nc */
   printf("\n*** Testing fill values.\n");
   printf("*** testing read of string record var with no data...");
   {
#define STRING_VAR_NAME "blood_toil_tears_sweat"
#define NDIMS_STRING 1
#define DATA_START 1 /* Real data here. */

      int  ncid, varid, dimid, varid_in;
      const char *data_out[1] = {
	 "We have before us an ordeal of the most grievous kind. We have before "
	 "us many, many long months of struggle and of suffering. You ask, what "
	 "is our policy? I can say: It is to wage war, by sea, land and air, "
	 "with all our might and with all the strength that God can give us; to "
	 "wage war against a monstrous tyranny, never surpassed in the dark, "
	 "lamentable catalogue of human crime. That is our policy. You ask, what "
	 "is our aim? "
	 "I can answer in one word: It is victory, victory at all costs, victory "
	 "in spite of all terror, victory, however long and hard the road may "
	 "be; for without victory, there is no survival."};
      char *data_in;
      size_t index = DATA_START;

      /* Create file with a 1D string var. Set its fill value to the
       * empty string. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_def_dim(ncid, "sentence", NC_UNLIMITED, &dimid)) ERR;
      if (nc_def_var(ncid, STRING_VAR_NAME, NC_STRING, NDIMS_STRING, 
		     &dimid, &varid)) ERR;

      /* Check it out. */
      if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR;

      /* Write one string, leaving some blank records which will then
       * get the fill value. */
      if (nc_put_var1_string(ncid, varid_in, &index, data_out)) ERR;

      /* Get all the data from the variable. */
      if (nc_get_var1_string(ncid, varid_in, &index, &data_in)) ERR;
      if (strcmp(data_in, data_out[0])) ERR;
      free(data_in);

      if (nc_close(ncid)) ERR;

      /* Now re-open file, read data, and check values again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      data_in = NULL;
      if (nc_get_var1_string(ncid, varid_in, &index, &data_in)) ERR;
      if (strcmp(data_in, data_out[0])) ERR;
      free(data_in);

      if (nc_close(ncid)) ERR;

/*       free(data_in); */
   }
   SUMMARIZE_ERR;
/*    printf("*** testing read of string record var with no data..."); */
/*    { */
/* #define STRING_VAR_NAME "I_Have_A_Dream" */
/* #define NDIMS_STRING 1 */
/* #define FILLVALUE_LEN 1 /\* There is 1 string, the empty one. *\/ */
/* #define DATA_START 2 /\* Real data here. *\/ */

/*       int  ncid, varid, dimid, varid_in; */
/*       const char *missing_val[FILLVALUE_LEN] = {""}; */
/*       const char *missing_val_in[FILLVALUE_LEN]; */
/*       const char *data_out[1] = { */
/* 	 "With this faith, we will be able to hew out of the mountain of " */
/* 	 "despair a stone of hope. With this faith, we will be able to " */
/* 	 "transform the jangling discords of our nation into a beautiful " */
/* 	 "symphony of brotherhood. With this faith, we will be able to work " */
/* 	 "together, to pray together, to struggle together, to go to jail " */
/* 	 "together, to stand up for freedom together, knowing that we will " */
/* 	 "be free one day."}; */
/*       char *data_in; */
/*       size_t index = DATA_START; */

/*       /\* Create file with a 1D string var. Set its fill value to the */
/*        * empty string. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_dim(ncid, "sentence", NC_UNLIMITED, &dimid)) ERR; */
/*       if (nc_def_var(ncid, STRING_VAR_NAME, NC_STRING, NDIMS_STRING,  */
/* 		     &dimid, &varid)) ERR; */
/* /\*      if (nc_put_att_string(ncid, varid, "_FillValue", FILLVALUE_LEN,  */
/* 	missing_val)) ERR;*\/ */

/*       /\* Check it out. *\/ */
/*       /\* if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR; *\/ */
/*       /\* if (nc_get_att_string(ncid, varid_in, "_FillValue",  *\/ */
/*       /\* 			    (char **)missing_val_in)) ERR; *\/ */
/*       /\* if (strcmp(missing_val[0], missing_val_in[0])) ERR; *\/ */
/*       /\* if (nc_free_string(FILLVALUE_LEN, (char **)missing_val_in)) ERR; *\/ */

/*       /\* Write one string, leaving some blank records which will then */
/*        * get the fill value. *\/ */
/*       if (nc_put_var1_string(ncid, varid_in, &index, data_out)) ERR; */

/*       /\* Get all the data from the variable. *\/ */
/*       if (nc_get_var1_string(ncid, varid_in, &index, &data_in)) ERR; */
/*       if (strcmp(data_in, data_out[0])) ERR; */
/*       free(data_in); */

/*       if (nc_close(ncid)) ERR; */

/*       /\* Now re-open file, read data, and check values again. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
/*       /\* if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR; *\/ */
/*       /\* if (nc_get_att_string(ncid, varid_in, "_FillValue",  *\/ */
/*       /\* 			    (char **)missing_val_in)) ERR; *\/ */
/*       /\* if (strcmp(missing_val[0], missing_val_in[0])) ERR; *\/ */
/*       /\* if (nc_free_string(FILLVALUE_LEN, (char **)missing_val_in)) ERR; *\/ */

/*       data_in = NULL; */
/*       if (nc_get_var1_string(ncid, varid_in, &index, &data_in)) ERR; */
/*       if (strcmp(data_in, data_out[0])) ERR; */
/*       free(data_in); */

/*       if (nc_close(ncid)) ERR; */

/* /\*       free(data_in); *\/ */
/*    } */
/*    SUMMARIZE_ERR; */
/*    printf("*** testing empty fill values of a string var..."); */
/*    { */
/* #define STRING_VAR_NAME "The_String" */
/* #define NDIMS_STRING 1 */
/* #define FILLVALUE_LEN 1 /\* There is 1 string, the empty one. *\/ */
/* #define DATA_START 2 /\* Real data here. *\/ */

/*       int  ncid, varid, dimid, varid_in; */
/*       const char *missing_val[FILLVALUE_LEN] = {""}; */
/*       const char *missing_val_in[FILLVALUE_LEN]; */
/*       const char *data_out[1] = {"The evil that men do lives after them; " */
/* 				 "the good is oft interred with their bones."}; */
/*       char **data_in; */
/*       size_t index = DATA_START; */
/*       int i; */

/*       /\* Create file with a 1D string var. Set its fill value to the */
/*        * empty string. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_dim(ncid, "rec", NC_UNLIMITED, &dimid)) ERR; */
/*       if (nc_def_var(ncid, STRING_VAR_NAME, NC_STRING, NDIMS_STRING, &dimid, &varid)) ERR; */
/*       if (nc_put_att_string(ncid, varid, "_FillValue", FILLVALUE_LEN, missing_val)) ERR; */

/*       /\* Check it out. *\/ */
/*       if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR; */
/*       if (nc_get_att_string(ncid, varid_in, "_FillValue", (char **)missing_val_in)) ERR; */
/*       if (strcmp(missing_val[0], missing_val_in[0])) ERR; */
/*       if (nc_free_string(FILLVALUE_LEN, (char **)missing_val_in)) ERR; */

/*       /\* Write one string, leaving some blank records which will then */
/*        * get the fill value. *\/ */
/*       if (nc_put_var1_string(ncid, varid_in, &index, data_out)) ERR; */

/*       /\* Get all the data from the variable. *\/ */
/*       if (!(data_in = malloc((DATA_START + 1) * sizeof(char *)))) ERR; */
/*       if (nc_get_var_string(ncid, varid_in, data_in)) ERR; */

/*       /\* First there should be fill values, then the data value we */
/*        * wrote. *\/ */
/*       for (i = 0; i < DATA_START; i++) */
/* 	 if (strcmp(data_in[i], "")) ERR; */
/*       if (strcmp(data_in[DATA_START], data_out[0])) ERR; */

/*       /\* Close everything up. Don't forget to free the string! *\/ */
/*       if (nc_free_string(DATA_START + 1, data_in)) ERR; */
/*       if (nc_close(ncid)) ERR; */

/*       /\* Now re-open file, read data, and check values again. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
/*      if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR; */
/*       if (nc_get_att_string(ncid, varid_in, "_FillValue", (char **)missing_val_in)) ERR; */
/*       if (strcmp(missing_val[0], missing_val_in[0])) ERR; */
/*       /\*if (nc_free_string(FILLVALUE_LEN, (char **)missing_val_in[0])) ERR;*\/ */
/*       if (nc_close(ncid)) ERR; */
/*        free(data_in);  */
/*    } */

/*    SUMMARIZE_ERR; */
/*    printf("*** testing non-empty fill values of a string var..."); */
/*    { */
/* #define STRING_VAR_NAME2 "CASSIUS" */
/* #define FILLVALUE_LEN2 1 /\* There is 1 string in the fillvalue array. *\/ */
/* #define DATA_START2 9 /\* Real data starts here. *\/ */

/*       int  ncid, varid, dimid, varid_in; */
/*       const char *missing_val[FILLVALUE_LEN2] = {"I know that virtue to be in you, Brutus"}; */
/*       const char *missing_val_in[FILLVALUE_LEN2]; */
/*       const char *data_out[1] = {"The evil that men do lives after them; " */
/* 				 "the good is oft interred with their bones."}; */
/*       char **data_in; */
/*       size_t index = DATA_START2; */
/*       int i; */

/*       /\* Create file with a 1D string var. Set its fill value to the */
/*        * a non-empty string. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_dim(ncid, "rec", NC_UNLIMITED, &dimid)) ERR; */
/*       if (nc_def_var(ncid, STRING_VAR_NAME2, NC_STRING, NDIMS_STRING, &dimid, &varid)) ERR; */
/*       if (nc_put_att_string(ncid, varid, "_FillValue", FILLVALUE_LEN2, missing_val)) ERR; */

/*       /\* Check it out. *\/ */
/*       if (nc_inq_varid(ncid, STRING_VAR_NAME2, &varid_in)) ERR; */
/*       if (nc_get_att_string(ncid, varid_in, "_FillValue", (char **)missing_val_in)) ERR; */
/*       if (strcmp(missing_val[0], missing_val_in[0])) ERR; */
/*       if (nc_free_string(FILLVALUE_LEN2, (char **)missing_val_in)) ERR; */

/*       /\* Write one string, leaving some blank records which will then */
/*        * get the fill value. *\/ */
/*       if (nc_put_var1_string(ncid, varid_in, &index, data_out)) ERR; */

/*       /\* Get all the data from the variable. *\/ */
/*       if (!(data_in = malloc((DATA_START2 + 1) * sizeof(char *)))) ERR; */
/*       if (nc_get_var_string(ncid, varid_in, data_in)) ERR; */

/*       /\* First there should be fill values, then the data value we */
/*        * wrote. *\/ */
/*       for (i = 0; i < DATA_START2; i++) */
/* 	 if (strcmp(data_in[i], missing_val[0])) ERR; */
/*       if (strcmp(data_in[DATA_START2], data_out[0])) ERR; */

/*       /\* Close everything up. *\/ */
/*       if (nc_close(ncid)) ERR; */

/*       /\* Now re-open file, read data, and check values again. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
/*       if (nc_inq_varid(ncid, STRING_VAR_NAME2, &varid_in)) ERR; */
/*       if (nc_get_att_string(ncid, varid_in, "_FillValue", (char **)missing_val_in)) ERR; */
/*       if (strcmp(missing_val[0], missing_val_in[0])) ERR; */
/*       if (nc_free_string(FILLVALUE_LEN2, (char **)missing_val_in)) ERR; */
/*       if (nc_close(ncid)) ERR; */
/*       free(data_in); */
/*    } */
/*    printf("*** testing read of string record var with no data..."); */
/*    { */
/* #define STRING_VAR_NAME "Moon_Is_A_Harsh_Mistress" */
/* #define NDIMS_STRING 1 */
/* #define FILLVALUE_LEN 1 /\* There is 1 string, the empty one. *\/ */
/* #define DATA_START 2 /\* Real data here. *\/ */

/*       int  ncid, varid, dimid, varid_in; */
/*       char *missing_val[FILLVALUE_LEN] = {""}; */
/*       char *missing_val_in; */

/*       /\* Create file with a 1D string var. Set its fill value to the */
/*        * empty string. *\/ */
/*       if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR; */
/*       if (nc_def_dim(ncid, "Lunar_Years", NC_UNLIMITED, &dimid)) ERR; */
/*       if (nc_def_var(ncid, STRING_VAR_NAME, NC_STRING, NDIMS_STRING,  */
/* 		     &dimid, &varid)) ERR; */
/*       if (nc_put_att_string(ncid, varid, "_FillValue", FILLVALUE_LEN,  */
/* 			    missing_val)) ERR; */

/*       /\* Check it out. *\/ */
/*       if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR; */
/*       if (nc_get_att_string(ncid, varid_in, "_FillValue", &missing_val_in)) ERR; */
/*       if (strcmp(missing_val[0], missing_val_in)) ERR; */
/*       if (nc_free_string(FILLVALUE_LEN, &missing_val_in)) ERR; */

/*       /\* Get all the data from the variable. There is none! *\/ */
/*       if (nc_get_var_string(ncid, varid_in, NULL)) ERR; */
      
/*       /\* Close file. *\/ */
/*       if (nc_close(ncid)) ERR; */

/*       /\* Now re-open file, and check again. *\/ */
/*       if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR; */
/*       if (nc_inq_varid(ncid, STRING_VAR_NAME, &varid_in)) ERR; */
/*       if (nc_get_att_string(ncid, varid_in, "_FillValue", &missing_val_in)) ERR; */
/*       if (strcmp(missing_val[0], missing_val_in)) ERR; */
/*       if (nc_free_string(FILLVALUE_LEN, &missing_val_in)) ERR; */
/*       if (nc_close(ncid)) ERR; */
/*    } */
/*    SUMMARIZE_ERR; */
   FINAL_RESULTS;
}
