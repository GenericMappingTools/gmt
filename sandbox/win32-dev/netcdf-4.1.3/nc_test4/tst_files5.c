/* This is part of the netCDF package.
   Copyright 2010 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf files a bit. 
*/

#include <nc_tests.h>
#include "netcdf.h"

#define FILE_NAME "tst_files.nc"
#define URL "http://test.opendap.org:8080/dods/dts/test.01"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf file functions.\n");
   printf("*** Checking the new inq_path function...");
   {
      int ncid;
      size_t path_len;
      char path_in[NC_MAX_NAME + 1] = "";

      /* Test with classic file create. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_path(ncid, &path_len, path_in)) ERR;
      if (path_len != strlen(FILE_NAME) || strcmp(path_in, FILE_NAME)) ERR;
      if (nc_close(ncid)) ERR;
      strcpy(path_in, "");
      path_len = 0;

      /* Test with classic file open. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_path(ncid, &path_len, path_in)) ERR;
      if (path_len != strlen(FILE_NAME) || strcmp(path_in, FILE_NAME)) ERR;
      if (nc_close(ncid)) ERR;
      strcpy(path_in, "");

      /* Test with netCDF-4 create. */
      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_inq_path(ncid, &path_len, path_in)) ERR;
      if (path_len != strlen(FILE_NAME) || strcmp(path_in, FILE_NAME)) ERR;
      if (nc_close(ncid)) ERR;
      strcpy(path_in, "");
      path_len = 0;

      /* Test with classic file open. */
      if (nc_open(FILE_NAME, 0, &ncid)) ERR;
      if (nc_inq_path(ncid, &path_len, path_in)) ERR;
      if (path_len != strlen(FILE_NAME) || strcmp(path_in, FILE_NAME)) ERR;
      if (nc_close(ncid)) ERR;
      strcpy(path_in, "");
      path_len = 0;

/* #ifdef USE_DAP */
/*       /\* Test with URL. *\/ */
/*       if (nc_open(URL, 0, &ncid)) ERR; */
/*       if (nc_inq_path(ncid, &path_len, path_in)) ERR; */
/*       /\*if (path_len != strlen(URL) || strcmp(path_in, URL)) ERR;*\/ */
/*       if (nc_close(ncid)) ERR; */
/* #endif /\* USE_DAP *\/ */
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


