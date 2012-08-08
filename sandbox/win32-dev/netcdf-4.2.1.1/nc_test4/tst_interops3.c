/* This is part of the netCDF package. Copyright 2005-2011, University
   Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
   for conditions of use.

   Test that NetCDF-4 can read a bunch of HDF4 files pulled in from
   the FTP site.
*/

#include <config.h>
#include <nc_tests.h>
#include <mfhdf.h>

#define FILE_NAME "tst_interops2.h4"

int
main(int argc, char **argv)
{
   printf("\n*** Testing HDF4/NetCDF-4 interoperability...\n");
   printf("*** testing that all hdf4 files can be opened...");
   {
#define NUM_SAMPLE_FILES 5
      int ncid;
      int nvars_in, ndims_in, natts_in, unlimdim_in;
      char file_name[NUM_SAMPLE_FILES][NC_MAX_NAME + 1] = {"AMSR_E_L2_Rain_V10_200905312326_A.hdf", 
							   "AMSR_E_L3_DailyLand_V06_20020619.hdf",
							   "MOD29.A2000055.0005.005.2006267200024.hdf",
							   "MYD29.A2002185.0000.005.2007160150627.hdf",
							   "MYD29.A2009152.0000.005.2009153124331.hdf"};
      size_t len_in;
      int f;
      
      for (f = 0; f < NUM_SAMPLE_FILES; f++)
      {
	 if (nc_open(file_name[f], NC_NOWRITE, &ncid)) ERR;
	 if (nc_inq(ncid, &ndims_in, &nvars_in, &natts_in, &unlimdim_in)) ERR;
	 if (nc_close(ncid)) ERR;
      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

