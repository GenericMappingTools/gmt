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
#define FILE7_NAME "tst_utf8.nc"
#define UNITS "units"
#define NDIMS 1
#define NX 18

int
main(int argc, char **argv)
{
   int ncid, dimid, varid;
   int dimids[NDIMS];

   /* (unnormalized) UTF-8 encoding for Unicode 8-character "Hello" in Greek */
   unsigned char name_utf8[] = {
       0xCE, 0x9A,	  /* GREEK CAPITAL LETTER KAPPA  : 2-bytes utf8 */
       0xCE, 0xB1,	  /* GREEK SMALL LETTER LAMBDA   : 2-bytes utf8 */
       0xCE, 0xBB,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
       0xCE, 0xB7,	  /* GREEK SMALL LETTER ETA      : 2-bytes utf8 */
       0xCE, 0xBC,	  /* GREEK SMALL LETTER MU       : 2-bytes utf8 */
       0xE1, 0xBD, 0xB3,  /* GREEK SMALL LETTER EPSILON
                             WITH TONOS                  : 3-bytes utf8 */
       0xCF, 0x81,	  /* GREEK SMALL LETTER RHO      : 2-bytes utf8 */
       0xCE, 0xB1,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
       0x00
   };

/* Name used for dimension, variable, and attribute value */
#define UNAME ((char *) name_utf8)
#define UNAMELEN (sizeof name_utf8)

   char name_in[UNAMELEN + 1], strings_in[UNAMELEN + 1];
   nc_type att_type;
   size_t att_len;
   int res;
   
   printf("\n*** Testing UTF-8.\n");
   printf("*** creating UTF-8 test file %s...", FILE7_NAME);
  if ((res = nc_create(FILE7_NAME, NC_CLOBBER, &ncid)))
       ERR;

   /* Define dimension with Unicode UTF-8 encoded name */
   if ((res = nc_def_dim(ncid, UNAME, NX, &dimid)))
       ERR;
   dimids[0] = dimid;

   /* Define variable with same name */
   if ((res = nc_def_var(ncid, UNAME, NC_CHAR, NDIMS, dimids, &varid)))
       ERR;

   /* Create string attribute with same value */
   if ((res = nc_put_att_text(ncid, varid, UNITS, UNAMELEN, UNAME)))
       ERR;

   if ((res = nc_enddef(ncid)))
       ERR;

   /* Write string data, UTF-8 encoded, to the file */
   if ((res = nc_put_var_text(ncid, varid, UNAME)))
       ERR;

   if ((res = nc_close(ncid)))
       ERR;

   /* Check it out. */
   
   /* Reopen the file. */
   if ((res = nc_open(FILE7_NAME, NC_NOWRITE, &ncid)))
       ERR;
   if ((res = nc_inq_varid(ncid, UNAME, &varid)))
       ERR;
   if ((res = nc_inq_varname(ncid, varid, name_in)))
       ERR;
   {
       /* Note, name was normalized before storing, so retrieved name
	  won't match original unnormalized name.  Check that we get
	  normalized version, instead.  */
       
       /* NFC normalized UTF-8 for Unicode 8-character "Hello" in Greek */
       unsigned char norm_utf8[] = {
	   0xCE, 0x9A,	  /* GREEK CAPITAL LETTER KAPPA  : 2-bytes utf8 */
	   0xCE, 0xB1,	  /* GREEK SMALL LETTER LAMBDA   : 2-bytes utf8 */
	   0xCE, 0xBB,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
	   0xCE, 0xB7,	  /* GREEK SMALL LETTER ETA      : 2-bytes utf8 */
	   0xCE, 0xBC,	  /* GREEK SMALL LETTER MU       : 2-bytes utf8 */
	   0xCE, 0xAD,    /* GREEK SMALL LETTER EPSILON WITH TONOS 
			                                 : 2-bytes utf8 */
	   0xCF, 0x81,	  /* GREEK SMALL LETTER RHO      : 2-bytes utf8 */
	   0xCE, 0xB1,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
	   0x00
       };
#define NNAME ((char *) norm_utf8)
#define NNAMELEN (sizeof norm_utf8)
       if ((res = strncmp(NNAME, name_in, NNAMELEN)))
	   ERR;
   }
   if ((res = nc_inq_att(ncid, varid, UNITS, &att_type, &att_len)))
       ERR;
   if ((res = att_type != NC_CHAR || att_len != UNAMELEN))
       ERR;
   /* We don't normalize data or attribute values, so get exactly what was put */
   if ((res = nc_get_att_text(ncid, varid, UNITS, strings_in)))
       ERR;
   strings_in[att_len] = '\0';
   if ((res = strncmp(UNAME, strings_in, UNAMELEN)))
       ERR;
   if ((res = nc_close(ncid)))
       ERR;

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
