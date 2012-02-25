/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test internal netcdf-4 file code. 
   $Id$
*/

#include <config.h>
#include <stdio.h>
#include <nc_tests.h>
#include "netcdf.h"
#include <hdf5.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <string.h>

#define NDIMS 1
#define FILE_NAME "tst_mem.nc"
#define NUM_TRIES 20000
#define TIME_NAME "time"
#define SFC_TEMP_NAME "sfc_temp"

void
get_mem_used2(int *mem_used)
{
   char buf[30];
   FILE *pf;
   
   snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
   pf = fopen(buf, "r");
   if (pf) {
      unsigned size; /*       total program size */
      unsigned resident;/*   resident set size */
      unsigned share;/*      shared pages */
      unsigned text;/*       text (code) */
      unsigned lib;/*        library */
      unsigned data;/*       data/stack */
      /*unsigned dt;          dirty pages (unused in Linux 2.6)*/
      fscanf(pf, "%u %u %u %u %u %u", &size, &resident, &share, 
	     &text, &lib, &data);
      *mem_used = data;
   }
   else
      *mem_used = -1;
  fclose(pf);
}

int main(void)
{
   int ncid, sfc_tempid;
   float data;
   int dimid;
   size_t l_index[NDIMS] = {10000};
   int mem_used, mem_used1;
   int i;

   printf("\n*** Testing netcdf-4 memory use with unlimited dimensions.\n");
   printf("*** testing with user-contributed code...");
      
   if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
   if (nc_def_dim(ncid, TIME_NAME, NC_UNLIMITED, &dimid)) ERR;
   if (nc_def_var(ncid, SFC_TEMP_NAME, NC_FLOAT, NDIMS, &dimid, &sfc_tempid)) ERR;

   /* Write data each 100ms*/
   get_mem_used2(&mem_used);
   for (i = 0; i < NUM_TRIES; i++) 
   {
      data = 25.5 + l_index[0];
      if (nc_put_var1_float(ncid, sfc_tempid, l_index, (const float*) &data)) ERR;
      l_index[0]++;
      get_mem_used2(&mem_used1);
      if (!(i%100) && mem_used1 - mem_used)
	 printf("delta %d bytes of memory for try %d\n", mem_used1 - mem_used, i);
   }

   if (nc_close(ncid)) ERR;
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
