/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   This program does some benchmarking of netCDF files for the AR-5
   data.
*/

#include <nc_tests.h>
#include "netcdf.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <../ncdump/nciter.h>

#define MILLION 1000000
#define MAX_LEN 30
#define TMP_FILE_NAME "tst_files2_tmp.out"

/* This function uses the ps command to find the amount of memory in
use by the process. From the ps man page:

size SZ approximate amount of swap space that would be required if
        the process were to dirty all writable pages and then be
        swapped out. This number is very rough!
*/
void
get_mem_used1(int *mem_used)
{
   char cmd[NC_MAX_NAME + 1];
   char blob[MAX_LEN + 1] = "";
   FILE *fp;
   int num_char;

   /* Run the ps command for this process, putting output (one number)
    * into file TMP_FILE_NAME. */
   sprintf(cmd, "ps -o size= %d > %s", getpid(), TMP_FILE_NAME);
   system(cmd);

   /* Read the results and delete temp file. */
   if (!(fp = fopen(TMP_FILE_NAME, "r"))) exit;
   num_char = fread(blob, MAX_LEN, 1, fp);
   sscanf(blob, "%d", mem_used);
   fclose(fp);
   unlink(TMP_FILE_NAME);
}

int
main(int argc, char **argv)
{

#define BUFSIZE 1000000 /* access data in megabyte sized pieces */
#define THETAU_FILE "/machine/downloads/AR5_sample_data/thetao_O1.SRESA1B_2.CCSM.ocnm.2000-01_cat_2099-12.nc"   
#define NDIMS_DATA 4
   printf("\n*** Running some AR-5 benchmarks.\n");
   printf("*** testing various chunksizes for thetau file...\n");
   {
      int ncid, ncid_out;
      /*char var_buf[BUFSIZE];*/  /* buffer for variable data */
/*      nciter_t iter; */     /* opaque structure for iteration status */
/*      size_t start[NDIMS_DATA];
	size_t count[NDIMS_DATA];*/
      /*float *data = (float *)var_buf; */
      char file_out[NC_MAX_NAME + 1];
      /*int ndims, nvars, natts, unlimdimid;*/
      size_t cs[NDIMS_DATA] = {120, 4, 40, 32};

/*       /\* Open input. *\/ */
/*       if (nc_open(THETAU_FILE, NC_NOWRITE, &ncid)) ERR; */

/*       /\* Create output file. *\/ */
/*       sprintf(file_out, "thetau_%d_%d_%d_%d.nc", (int)cs[0],  */
/* 	      (int)cs[1], (int)cs[2], (int)cs[3]); */
/*       if (nc_create(file_out, NC_NOWRITE, &ncid_out)) ERR; */

/*       /\* Copy the easy ones. *\/ */
/* /\*      if (nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)) ERR; */
/* 	if (ndims != 5 || nvars != 9 || natts != 8 || unlimdimid != 0) ERR;*\/ */

/* /\*       /\\* Copy the main data payload with Russ's new nciters. *\\/ *\/ */
/* /\*       varid = 8; *\/ */
/* /\*       if (nc_get_iter(ncid, varid, BUFSIZE, &iter)) ERR; *\/ */
/* /\*       while((nvals = nc_next_iter(&iter, start, count)) > 0)  *\/ */
/* /\*       { *\/ */
/* /\* 	 /\\* read in a block of data *\\/ *\/ */
/* /\* 	 if (nc_get_vara_double(ncid, varid, start, count, data)) ERR; *\/ */

/* /\* 	 /\\* now write the changed data back out *\\/ *\/ */
/* /\* 	 if (nc_out_vara_double(ncid, varid, start, count, data)) ERR; *\/ */
/* /\*       } *\/ */
/* /\*       if (nvals < 0) ERR; *\/ */
      
/*       if (nc_close(ncid)) ERR; */
/*       if (nc_close(ncid_out)) ERR; */
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
