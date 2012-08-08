/* This is a quickie tester for netcdf-4. 
   $Id$
*/

#include <tests.h>

#define FILE_NAME "../nc_test/nc_test_netcdf4.nc"

int
main()
{
   int ncid;
   int retval; 
   size_t start[1] = {0}, count[1] = {2};
   ptrdiff_t stride[1] = {2};
   short short_data[256];
   float float_data[256];

   printf("\n*** Testing with file %s\n", FILE_NAME);

   /*nc_set_log_level(4);*/

   printf("*** Testing whether we can open and read from it...");
   if ((retval = nc_open(FILE_NAME, 0, &ncid))) ERR;
   if ((retval = nc_get_vars_short(ncid, 35, start, count, stride, 
				   short_data)) != NC_ERANGE) ERR;
   if ((retval = nc_get_vars(ncid, 35, start, count, stride, short_data))) ERR;
   if ((retval = nc_close(ncid))) ERR;

   SUMMARIZE_ERR;

   FINAL_RESULTS;
}
