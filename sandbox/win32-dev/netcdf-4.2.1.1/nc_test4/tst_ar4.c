/* 
Copyright 2009, UCAR/Unidata
See COPYRIGHT file for copying and redistribution conditions.

This program tests netcdf-4 performance with some AR-4 3D data.

$Id$
*/

#include <nc_tests.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define MEGABYTE 1048576
#define HALF_MEG (MEGABYTE/2)
#define MILLION 1000000
#define SIXTEEN_MEG 16777216
#define FOUR_MEG (SIXTEEN_MEG/4)
#define THIRTY_TWO_MEG (SIXTEEN_MEG * 2)
#define SIXTY_FOUR_MEG (SIXTEEN_MEG * 4)
#define ONE_TWENTY_EIGHT_MEG (SIXTEEN_MEG * 8)

	 /* From the data file we are using:

netcdf pr_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12 {
dimensions:
	lon = 256 ;
	lat = 128 ;
	bnds = 2 ;
	time = UNLIMITED ; // (1560 currently)
variables:
	double lon_bnds(lon, bnds) ;
	double lat_bnds(lat, bnds) ;
	double time_bnds(time, bnds) ;
	double time(time) ;
		time:calendar = "noleap" ;
		time:standard_name = "time" ;
		time:axis = "T" ;
		time:units = "days since 0000-1-1" ;
		time:bounds = "time_bnds" ;
		time:long_name = "time" ;
	double lat(lat) ;
		lat:axis = "Y" ;
		lat:standard_name = "latitude" ;
		lat:bounds = "lat_bnds" ;
		lat:long_name = "latitude" ;
		lat:units = "degrees_north" ;
	double lon(lon) ;
		lon:axis = "X" ;
		lon:standard_name = "longitude" ;
		lon:bounds = "lon_bnds" ;
		lon:long_name = "longitude" ;
		lon:units = "degrees_east" ;
	float pr(time, lat, lon) ;
		pr:comment = "Created using NCL code CCSM_atmm_2cf.ncl on\n",
			" machine mineral" ;
		pr:missing_value = 1.e+20f ;
		pr:_FillValue = 1.e+20f ;
		pr:cell_methods = "time: mean (interval: 1 month)" ;
		pr:history = "(PRECC+PRECL)*r[h2o]" ;
		pr:original_units = "m-1 s-1" ;
		pr:original_name = "PRECC, PRECL" ;
		pr:standard_name = "precipitation_flux" ;
		pr:units = "kg m-2 s-1" ;
		pr:long_name = "precipitation_flux" ;
		pr:cell_method = "time: mean" ;

*/

/* Subtract the `struct timeval' values X and Y, storing the result in
   RESULT.  Return 1 if the difference is negative, otherwise 0.  This
   function from the GNU documentation. */
static int
timeval_subtract (result, x, y)
   struct timeval *result, *x, *y;
{
   /* Perform the carry for the later subtraction by updating Y. */
   if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / MILLION + 1;
      y->tv_usec -= MILLION * nsec;
      y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > MILLION) {
      int nsec = (x->tv_usec - y->tv_usec) / MILLION;
      y->tv_usec += MILLION * nsec;
      y->tv_sec -= nsec;
   }

   /* Compute the time remaining to wait.
      `tv_usec' is certainly positive. */
   result->tv_sec = x->tv_sec - y->tv_sec;
   result->tv_usec = x->tv_usec - y->tv_usec;

   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
}

#define USAGE   "\
  [-v]        Verbose\n\
  [-h]        Print output header\n\
  [-t]        Do a time-series read\n\
  [-c CACHE_SIZE]        Set the HDF5 chunk cache to this size before read\n\
  file        Name of netCDF file\n"

static void
usage(void)
{
   fprintf(stderr, "tst_ar4 -v -h -t -c CACHE_SIZE file\n%s", USAGE);
}

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

int 
main(int argc, char **argv)
{
   extern int optind;
   extern int opterr;
   extern char *optarg;
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

   while ((c = getopt(argc, argv, "vhtc:")) != EOF)
      switch(c) 
      {
	 case 'v':
	    verbose++;
	    break;
	 case 'h':
	    header++;
	    break;
	 case 't':
	    timeseries++;
	    break;
	 case 'c':
	    sscanf(optarg, "%d", &cache);
	    break;
	 case '?':
	    usage();
	    return 1;
      }
      
   argc -= optind;
   argv += optind;
      
   /* If no file arguments left, print usage message. */
   if (argc < 1)
   {
      usage();
      return 0;
   }
      
   /* Print the header if desired. */
   if (header)
   {
      printf("cs[0]\tcs[1]\tcs[2]\tcache(MB)\tdeflate\tshuffle");
      if (timeseries)
	 printf("\t1st_read_ser(us)\tavg_read_ser(us)\n");
      else
	 printf("\t1st_read_hor(us)\tavg_read_hor(us)\n");
   }

#define PREEMPTION .75
      /* Also tried NELEMS of 2500009*/
#define NELEMS 7919
   if (nc_set_chunk_cache(cache, NELEMS, PREEMPTION)) ERR;
   if (nc_open(argv[0], 0, &ncid)) ERR;

   /* Check to make sure that all the dimension information is
    * correct. */
   if (nc_inq_varid(ncid, DATA_VAR_NAME, &varid)) ERR;
   if (nc_inq_dim(ncid, LON_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "lon") || len != LON_LEN) ERR;
   if (nc_inq_dim(ncid, LAT_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "lat") || len != LAT_LEN) ERR;
   if (nc_inq_dim(ncid, BNDS_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "bnds") || len != BNDS_LEN) ERR;
   if (nc_inq_dim(ncid, TIME_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "time") || len != TIME_LEN) ERR;
   if (nc_inq_var(ncid, varid, NULL, NULL, &ndims, dimid, NULL)) ERR;
   if (ndims != NDIMS3 || dimid[0] != TIME_DIMID || 
       dimid[1] != LAT_DIMID || dimid[2] != LON_DIMID) ERR;

   /* Get info about the main data var. */
   if (nc_inq_var_chunking(ncid, varid, &storage, cs)) ERR;
   if (nc_inq_var_deflate(ncid, varid, &shuffle, &deflate, 
			  &deflate_level)) ERR;

   if (timeseries)
   {
      /* Read the var as a time series. */
      start[0] = 0;
      start[1] = 0;
      start[2] = 0;
      count[0] = TIME_LEN;
      count[1] = 1;
      count[2] = 1;
      
      /* Read the first timeseries. */
      if (gettimeofday(&start_time, NULL)) ERR;
      if (nc_get_vara_float(ncid, varid, start, count, ts_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      read_1_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

      /* Read all the rest. */
      if (gettimeofday(&start_time, NULL)) ERR;
      for (start[1] = 0; start[1] < LAT_LEN; start[1]++)
	 for (start[2] = 1; start[2] < LON_LEN; start[2]++)
	    if (nc_get_vara_float(ncid, varid, start, count, ts_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      avg_read_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec + read_1_us) / 
	 (LAT_LEN * LON_LEN);
   }
   else
   {
      /* Read the data variable in horizontal slices. */
      start[0] = 0;
      start[1] = 0;
      start[2] = 0;
      count[0] = 1;
      count[1] = LAT_LEN;
      count[2] = LON_LEN;

      /* Read (and time) the first one. */
      if (gettimeofday(&start_time, NULL)) ERR;
      if (nc_get_vara_float(ncid, varid, start, count, hor_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      read_1_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

      /* Read (and time) all the rest. */
      if (gettimeofday(&start_time, NULL)) ERR;
      for (start[0] = 1; start[0] < TIME_LEN; start[0]++)
	 if (nc_get_vara_float(ncid, varid, start, count, hor_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      avg_read_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec + 
		     read_1_us) / TIME_LEN;
   }

   /* Close file. */
   if (nc_close(ncid)) ERR;

   /* Print results. */
   printf("%d\t%d\t%d\t%.1f\t\t%d\t%d\t\t",
	  (int)cs[0], (int)cs[1], (int)cs[2], 
	  (storage == NC_CHUNKED) ? (cache/(float)MEGABYTE) : 0, 
	  deflate, shuffle);
   if (timeseries)
      printf("%d\t\t%d\n", (int)read_1_us, (int)avg_read_us);
   else
      printf("%d\t\t%d\n", (int)read_1_us, (int)avg_read_us);

   return 0;
}

