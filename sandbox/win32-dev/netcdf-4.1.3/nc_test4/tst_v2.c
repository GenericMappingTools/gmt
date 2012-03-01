/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test internal netcdf-4 file code. 
   $Id$
*/

#include <config.h>
#include "netcdf.h"
#include <nc_tests.h>

#define FILE_NAME "tst_v2.nc"
#define NDIMS 2
#define DIM1_NAME "rise_height"
#define DIM2_NAME "saltiness"
#define DIM1_SIZE 3
#define DIM2_SIZE 5
#define VAR1_NAME "wheat_loaf"
#define VAR2_NAME "sourdough_wheat_loaf"
#define VAR3_NAME "white_loaf"

extern int ncopts;

int
main(int argc, char **argv)
{
   /*nc_set_log_level(3);*/
   printf("\n*** Testing netcdf-4 v2 API functions.\n");
   printf("*** testing simple opens and creates...");
   {
      int ncid, varid, varid_in, dimids[2];

      /* Turn off the crashing whenever there is a problem. */
      ncopts = NC_VERBOSE;

      /* Create an empty file. */
      if ((ncid = nccreate(FILE_NAME, NC_CLOBBER)) == -1) ERR;
      if (ncclose(ncid) == -1) ERR;

      /* Open the file, go into redef, and add some dims and vars. */
      if ((ncid = ncopen(FILE_NAME, NC_WRITE)) == -1) ERR;
      if (ncredef(ncid) == -1) ERR;
      if ((dimids[0] = ncdimdef(ncid, DIM1_NAME, DIM1_SIZE)) == -1) ERR;
      if ((dimids[1] = ncdimdef(ncid, DIM2_NAME, DIM2_SIZE)) == -1) ERR;
      if ((varid = ncvardef(ncid, VAR1_NAME, NC_DOUBLE, NDIMS, dimids)) == -1) ERR;
      if ((varid_in = ncvarid(ncid, VAR1_NAME)) == -1) ERR;
      if (varid_in != varid) ERR;
      if ((varid = ncvardef(ncid, VAR2_NAME, NC_INT, NDIMS, dimids)) == -1) ERR;
      if ((varid_in = ncvarid(ncid, VAR2_NAME)) == -1) ERR;
      if (varid_in != varid) ERR;
      if ((varid = ncvardef(ncid, VAR3_NAME, NC_SHORT, NDIMS, dimids)) == -1) ERR;
      if ((varid_in = ncvarid(ncid, VAR3_NAME)) == -1) ERR;
      if (varid_in != varid) ERR;
      if (ncclose(ncid) == -1) ERR;
   }

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
