/* This is part of the libcf package. Copyright 2006 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test libcf coordinate variable stuff.

   Ed Hartnett, 10/1/06

   $Id$
*/

#include <config.h>
#include <libcf.h>
#include <netcdf.h>
#include <nc_tests.h>

#define FILE_NAME "tst_cvars.nc"

int
main(int argc, char **argv)
{
   printf("\n*** Testing libcf coordinate variable stuff.\n");

#define NDIMS 1
#define NLATS 20

   printf("*** testing coordinate variable creation...");
   {
      int ncid, varid, dimid, varid_in, dimid_in;
      int l, lat[NLATS];
      size_t len_in;
      nc_type xtype_in;

      /* Create some latitudes. */
      for (lat[0] = 40, l = 1; l < NLATS; l++)
	 lat[l] = lat[l-1] + 2;

      /* CReate a file and use nccd_def_file to annotate it. */
      if (nc_create(FILE_NAME, 0, &ncid)) ERR;
      if (nccf_def_latitude(ncid, NLATS, NC_INT, &dimid, 
			    &varid)) ERR;

      /* Check the file. */
      if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, 
			    &varid_in)) ERR;
      if (len_in != NLATS || xtype_in != NC_INT ||
	  dimid_in != 0 || varid_in != 0) ERR;

      if (nc_close(ncid)) ERR;

      if (nc_open(FILE_NAME, 0, &ncid)) ERR;

      /* Check the file. */
      if (nccf_inq_latitude(ncid, &len_in, &xtype_in, &dimid_in, 
			    &varid_in)) ERR;
      if (len_in != NLATS || xtype_in != NC_INT || 
	  dimid_in != 0 || varid_in != 0) ERR;

      if (nc_close(ncid)) ERR;

   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}


