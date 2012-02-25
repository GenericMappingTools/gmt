/* This is part of the libcf package. Copyright 2007 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test libcf variable subsetting stuff.

   Ed Hartnett, 10/1/06

   $Id$
*/

#include <config.h>
#include <libcf.h>
#include <netcdf.h>
#include <nc_tests.h>

int
main(int argc, char **argv)
{
   printf("\n*** Testing libcf data examples.\n");

#define EXAMPLE_FILE "/upc/share/ed/libcf_data/cami_0000-09-01_2x2.5_L26_c030918_USGS.nc"
   printf("*** looking at %s...", EXAMPLE_FILE);
   {
      int ncid, did, vid;
      char formula_terms[CF_MAX_FT_LEN + 1];
      int positive_up;
      char name[NC_MAX_NAME + 1];
      size_t len, ft_len;
      nc_type xtype;

      /* Check the file. */
      if (nc_open(EXAMPLE_FILE, 0, &ncid)) ERR;
      if (nccf_inq_latitude(ncid, &len, &xtype, &did, &vid)) ERR;
      if (len != 91 || xtype != NC_DOUBLE || did != 0 || vid != 1) ERR;
      if (nccf_inq_longitude(ncid, &len, &xtype, &did, &vid)) ERR;
      if (len != 144 || xtype != NC_DOUBLE || did != 1 || vid != 2) ERR;
      if (nccf_inq_lvl(ncid, name, &len, &xtype, &ft_len, formula_terms, 
		       &positive_up, &did, &vid)) ERR;
      if (len != 26 || xtype != NC_DOUBLE || did != 4 || vid != 6 || positive_up != 0) ERR;
      
      
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;

   FINAL_RESULTS;
}


