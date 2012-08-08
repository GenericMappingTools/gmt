/* This is part of the netCDF package. Copyright 2005 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Test netcdf-4 string types.

   $Id$
*/

#include <config.h>
#include <nc_tests.h>

#define FILE_NAME "tst_strings2.nc"
#define ATT_NAME "WC"

int
main(int argc, char **argv)
{
   printf("\n*** Testing netcdf-4 string type.\n");
   printf("*** testing very simple string attribute...");
   {
#define ATT_LEN 1      
      size_t att_len;
      int ndims, nvars, natts, unlimdimid;
      nc_type att_type;
      int ncid, i;
      char *data_in[ATT_LEN];
      char *data[ATT_LEN] = {"An appeaser is one who feeds a crocodile — "
			     "hoping it will eat him last."};
   

      if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
      if (nc_put_att(ncid, NC_GLOBAL, ATT_NAME, NC_STRING, ATT_LEN, data)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != ATT_LEN) ERR;
      if (nc_close(ncid)) ERR;
      
      /* Check it out. */
      if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR;
      if (ndims != 0 || nvars != 0 || natts != 1 || unlimdimid != -1) ERR;
      if (nc_inq_att(ncid, NC_GLOBAL, ATT_NAME, &att_type, &att_len)) ERR;
      if (att_type != NC_STRING || att_len != ATT_LEN) ERR;
      if (nc_get_att(ncid, NC_GLOBAL, ATT_NAME, data_in)) ERR;
      for (i = 0; i < att_len; i++)
      	 if (strcmp(data_in[i], data[i])) ERR;
      if (nc_free_string(att_len, (char **)data_in)) ERR;
      if (nc_close(ncid)) ERR;
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

