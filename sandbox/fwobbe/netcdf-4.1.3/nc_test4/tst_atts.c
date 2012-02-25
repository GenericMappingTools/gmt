/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which writes a netCDF file with
   Unicode names encoded with UTF-8. It is the NETCDF3 equivalent
   of tst_unicode.c

   $Id$
*/

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <string.h>

/* The data file we will create. */
#define FILE_NAME "tst_atts.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netCDF-4 attributes.\n");
   printf("*** testing attribute renaming for a global attribute...");
   {
#define OLD_NAME "Constantinople"
#define NEW_NAME "Istanbul"
#define CONTENTS "Lots of people!"
      
      int ncid, attid;
      char *data_in;

      if (!(data_in = malloc(strlen(CONTENTS) + 1))) ERR;

      /* Create a file with an att. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_put_att_text(ncid, NC_GLOBAL, OLD_NAME, strlen(CONTENTS), 
			  CONTENTS)) ERR; 

      /* Rename the att. */
      if (nc_rename_att(ncid, NC_GLOBAL, OLD_NAME, NEW_NAME)) ERR; 
      
      /* Check the file. */
      if (nc_inq_attid(ncid, NC_GLOBAL, NEW_NAME, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, NC_GLOBAL, NEW_NAME, data_in)) ERR;
      if (strncmp(CONTENTS, data_in, strlen(CONTENTS))) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Reopen the file and check again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_attid(ncid, NC_GLOBAL, NEW_NAME, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, NC_GLOBAL, NEW_NAME, data_in)) ERR;
      if (strncmp(CONTENTS, data_in, strlen(CONTENTS))) ERR;
      if (nc_close(ncid)) ERR;

      free(data_in);
   }
   SUMMARIZE_ERR;
   printf("*** testing attribute renaming for a variable attribute...");
   {
#define VAR_NAME "var_name"
#define OLD_NAME1 "Constantinople"
#define NEW_NAME1 "Istanbul____________"
#define CONTENTS1 "Lots of people!"

      int ncid, attid, varid;
      char *data_in;

      if (!(data_in = malloc(strlen(CONTENTS1) + 1))) ERR;

      /* Create a file with an att. */
      if (nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)) ERR;
      if (nc_def_var(ncid, VAR_NAME, NC_INT, 0, NULL, &varid)) ERR;
      if (nc_put_att_text(ncid, varid, OLD_NAME1, strlen(CONTENTS1), 
			  CONTENTS1)) ERR; 

      /* Rename the att. */
      if (nc_rename_att(ncid, varid, OLD_NAME1, NEW_NAME1)) ERR; 
      
      /* Check the file. */
      if (nc_inq_attid(ncid, varid, NEW_NAME1, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, varid, NEW_NAME1, data_in)) ERR;
      if (strncmp(CONTENTS1, data_in, strlen(CONTENTS1))) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Reopen the file and check again. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;

      if (nc_inq_attid(ncid, varid, NEW_NAME1, &attid)) ERR;
      if (attid != 0) ERR;
      if (nc_get_att_text(ncid, varid, NEW_NAME1, data_in)) ERR;
      if (strncmp(CONTENTS1, data_in, strlen(CONTENTS1))) ERR;
      if (nc_close(ncid)) ERR;

      free(data_in);
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
