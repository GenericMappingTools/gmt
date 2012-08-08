/** \file

Performance test from KNMI.
 
Copyright 2009, UCAR/Unidata. See \ref copyright file for copying and
redistribution conditions.
*/

#include <nc_tests.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define FILE_NAME_1 "MSGCPP_CWP_NC3.nc"
#define FILE_NAME_2 "MSGCPP_CWP_NC4.nc"

#define NDIMS3 3
#define DATA_VAR_NAME "pr"
#define NUM_CACHE_TRIES 1
#define LON_DIMID 0
#define LAT_DIMID 1
#define BNDS_DIMID 2
#define TIME_DIMID 3
#define LON_LEN 256
#define LAT_LEN 128
#define BNDS_LEN 2
#define TIME_LEN 1560
#define NUM_TS 1

extern const char* nc_strerror(int ncerr);
static int
complain(int stat)
{
    if(stat) {
        fprintf(stderr,"%s\n",nc_strerror(stat));
	fflush(stderr);
    }
    return stat;
}

static int 
read_file(char *filename)
{
#define CWP "cwp"
#define XLEN 3712
#define YLEN 3712

   int ncid, varid;
   struct timeval start_time, end_time, diff_time;
   short *data;
   int time_us;

   printf("**** reading file %s\n", filename);
   if (gettimeofday(&start_time, NULL)) ERR_RET;
   if(complain(nc_open(filename, NC_NOWRITE, &ncid))) ERR_RET;
   if (gettimeofday(&end_time, NULL)) ERR_RET;
   if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR_RET;
   time_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
   printf("File open time (us): %d\n", (int)time_us);

   if (!(data = malloc(sizeof(short) * XLEN * YLEN))) ERR;
   if (gettimeofday(&start_time, NULL)) ERR_RET;
   if (nc_inq_varid(ncid, CWP, &varid)) ERR;
   if (nc_get_var_short(ncid, varid, data)) ERR;
   if (gettimeofday(&end_time, NULL)) ERR_RET;
   if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR_RET;
   time_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
   printf("Data read time (us): %d\n", (int)time_us);
   free(data);

   if (nc_close(ncid))
      ERR_RET;
   return 0;
}

int 
main(int argc, char **argv)
{
   int c, header = 0, verbose = 0, timeseries = 0;
   int ncid, varid, storage;
   char name_in[NC_MAX_NAME + 1];
   size_t len;
   size_t cs[NDIMS3] = {0, 0, 0};
   int cache = MEGABYTE;
   int ndims, dimid[NDIMS3];
   float hor_data[LAT_LEN * LON_LEN];
   int read_1_us, avg_read_us;
   float ts_data[TIME_LEN];
   size_t start[NDIMS3], count[NDIMS3];
   int deflate, shuffle, deflate_level;
   struct timeval start_time, end_time, diff_time;

   printf("\n*** Testing netcdf-4 vs. netcdf-3 performance.\n");
   if (complain(read_file(FILE_NAME_1))) ERR; 
   if (complain(read_file(FILE_NAME_2))) ERR; 

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

