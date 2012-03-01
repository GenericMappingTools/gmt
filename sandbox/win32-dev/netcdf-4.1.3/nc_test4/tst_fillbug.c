/* This is part of the netCDF package.
   Copyright 2008 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test for a bug that Russ found testing fill values.

   $Id$
*/

#include <nc_tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define FILENAME "tst_fillbug.nc"
#define RANK_Time 1
#define RANK_P 3
#define LEN 4

int
main() 
{
   int ncid, dimids[RANK_P], time_id, p_id;
   int ndims, dimids_in[RANK_P];

   double data[1] = {3.14159};
   size_t start[1] = {0}, count[1] = {1};
   static float P_data[LEN];
   size_t cor[RANK_P] = {0, 1, 0};
   size_t edg[RANK_P] = {1, 1, LEN};
   int i;

   printf("\n*** Testing for a netCDF-4 fill-value bug.\n");
   printf("*** Testing fill-values...");

   /* Create a 3D test file. */
   if (nc_create(FILENAME, NC_CLOBBER|NC_NETCDF4, &ncid)) ERR;

   /* define dimensions */
   if (nc_def_dim(ncid, "Time", NC_UNLIMITED, &dimids[0])) ERR;
   if (nc_def_dim(ncid, "X", 4, &dimids[2])) ERR;
   if (nc_def_dim(ncid, "Y", 3, &dimids[1])) ERR;

   /* define variables */
   if (nc_def_var(ncid, "Time", NC_DOUBLE, 1, dimids, &time_id)) ERR;
   if (nc_def_var(ncid, "P", NC_FLOAT, RANK_P, dimids, &p_id)) ERR;

   /* Add one record in coordinate variable. */
   if (nc_put_vara(ncid, time_id, start, count, data)) ERR;

   /* The other variable should show an increase in size, since it
    * uses the unlimited dimension. */
   if (nc_inq_var(ncid, 1, NULL, NULL, &ndims, dimids_in, NULL)) ERR;
   if (ndims != 3 || dimids_in[0] != 0 || dimids_in[1] != 2 || dimids_in[2] != 1) ERR;

   /* Read the record of non-existant data. */
   if (nc_get_vara(ncid, 1, cor, edg, P_data)) ERR;
   for (i = 0; i < LEN; i++)
      if (P_data[i] != NC_FILL_FLOAT) ERR;

   /* That's it! */
   if (nc_close(ncid)) ERR;

   /* Reopen the file and read the second slice. */
   if (nc_open(FILENAME, NC_NOWRITE, &ncid)) ERR;

   if (nc_inq_var(ncid, 1, NULL, NULL, &ndims, dimids_in, NULL)) ERR;
   if (ndims != 3 || dimids_in[0] != 0 || dimids_in[1] != 2 || dimids_in[2] != 1) ERR;
   if (nc_get_vara(ncid, 1, cor, edg, P_data)) ERR;
   for (i = 0; i < LEN; i++)
      if (P_data[i] != NC_FILL_FLOAT) ERR;

   if (nc_close(ncid)) ERR;

   SUMMARIZE_ERR;
   
   FINAL_RESULTS;
   return 0;
}
