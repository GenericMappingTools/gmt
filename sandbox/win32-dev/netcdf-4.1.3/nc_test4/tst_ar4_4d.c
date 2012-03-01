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

../ncdump/ncdump -h -s thetao_O1.SRESA1B_2.CCSM.ocnm.2000-01_cat_2099-12.nc 
netcdf thetao_O1.SRESA1B_2.CCSM.ocnm.2000-01_cat_2099-12 {
dimensions:
	lon = 320 ;
	lat = 395 ;
	depth = 40 ;
	bnds = 2 ;
	time = UNLIMITED ; // (1200 currently)
variables:
	double lon_bnds(lon, bnds) ;
	double lat_bnds(lat, bnds) ;
	double depth_bnds(depth, bnds) ;
	double time_bnds(time, bnds) ;
	double lon(lon) ;
		lon:axis = "X" ;
		lon:standard_name = "longitude" ;
		lon:bounds = "lon_bnds" ;
		lon:long_name = "Longitude" ;
		lon:units = "degrees_east" ;
	double lat(lat) ;
		lat:axis = "Y" ;
		lat:standard_name = "latitude" ;
		lat:bounds = "lat_bnds" ;
		lat:long_name = "Latitude" ;
		lat:units = "degrees_north" ;
	double depth(depth) ;
		depth:axis = "Z" ;
		depth:standard_name = "depth" ;
		depth:positive = "down" ;
		depth:units = "m" ;
		depth:bounds = "depth_bnds" ;
	double time(time) ;
		time:calendar = "noleap" ;
		time:standard_name = "time" ;
		time:axis = "T" ;
		time:units = "days since 0000-1-1" ;
		time:bounds = "time_bnds" ;
	float thetao(time, depth, lat, lon) ;
		thetao:comment = "Created using NCL code CCSM_ocnm_2cf.ncl on\n",
			" machine mineral" ;
		thetao:missing_value = 1.e+20f ;
		thetao:long_name = "sea_water_potential_temperature" ;
		thetao:cell_methods = "time: mean (interval: 1 month)" ;
		thetao:history = "Interpolated to regular grid from dipole grid,\n",
			"TEMP+273.15" ;
		thetao:units = "K" ;
		thetao:original_units = "C" ;
		thetao:original_name = "TEMP" ;
		thetao:standard_name = "sea_water_potential_temperature" ;
		thetao:_FillValue = 1.e+20f ;

// global attributes:
		:table_id = "Table O1" ;
		:title = "model output prepared for IPCC AR4" ;
		:institution = "NCAR (National Center for Atmospheric \n",
			"Research, Boulder, CO, USA)" ;
		:source = "CCSM3.0, version beta19 (2004): \n",
			"atmosphere: CAM3.0, T85L26;\n",
			"ocean     : POP1.4.3 (modified), gx1v3\n",
			"sea ice   : CSIM5.0, gx1v3;\n",
			"land      : CLM3.0, T85" ;
		:contact = "ccsm@ucar.edu" ;
		:project_id = "IPCC Fourth Assessment" ;
		:Conventions = "CF-1.0" ;
		:references = "Collins, W.D., et al., 2005:\n",
			" The Community Climate System Model, Version 3\n",
			" Journal of Climate\n",
			" \n",
			" Main website: http://www.ccsm.ucar.edu" ;
		:acknowledgment = " Any use of CCSM data should acknowledge the contribution\n",
			" of the CCSM project and CCSM sponsor agencies with the \n",
			" following citation:\n",
			" \'This research uses data provided by the Community Climate\n",
			" System Model project (www.ccsm.ucar.edu), supported by the\n",
			" Directorate for Geosciences of the National Science Foundation\n",
			" and the Office of Biological and Environmental Research of\n",
			" the U.S. Department of Energy.\'\n",
			"In addition, the words \'Community Climate System Model\' and\n",
			" \'CCSM\' should be included as metadata for webpages referencing\n",
			" work using CCSM data or as keywords provided to journal or book\n",
			"publishers of your manuscripts.\n",
			"Users of CCSM data accept the responsibility of emailing\n",
			" citations of publications of research using CCSM data to\n",
			" ccsm@ucar.edu.\n",
			"Any redistribution of CCSM data must include this data\n",
			" acknowledgement statement." ;
		:realization = 2 ;
		:experiment_id = "720 ppm stabilization experiment (SRES A1B)" ;
		:history = "Created from CCSM3 case b30.040b\n",
			" by strandwg@ucar.edu\n",
			" on Sun Apr 24 22:35:53 MDT 2005\n",
			" \n",
			" For all data, added IPCC requested metadata" ;
		:comment = "This simulation was initiated from year 2000 of \n",
			" CCSM3 model run b30.030b and executed on \n",
			" hardware bluesky.ucar.edu. The input external forcings are\n",
			"ozone forcing    : A1B.ozone.128x64_L18_1991-2100_c040528.nc\n",
			"aerosol optics   : AerosolOptics_c040105.nc\n",
			"aerosol MMR      : AerosolMass_V_128x256_clim_c031022.nc\n",
			"carbon scaling   : carbonscaling_A1B_1990-2100_c040609.nc\n",
			"solar forcing    : Fixed at 1366.5 W m-2\n",
			"GHGs             : ghg_ipcc_A1B_1870-2100_c040521.nc\n",
			"GHG loss rates   : noaamisc.r8.nc\n",
			"volcanic forcing : none\n",
			"DMS emissions    : DMS_emissions_128x256_clim_c040122.nc\n",
			"oxidants         : oxid_128x256_L26_clim_c040112.nc\n",
			"SOx emissions    : SOx_emissions_A1B_128x256_L2_1990-2100_c040608.nc\n",
			" Physical constants used for derived data:\n",
			" Lv (latent heat of evaporation): 2.501e6 J kg-1\n",
			" Lf (latent heat of fusion     ): 3.337e5 J kg-1\n",
			" r[h2o] (density of water      ): 1000 kg m-3\n",
			" g2kg   (grams to kilograms    ): 1000 g kg-1\n",
			" \n",
			" Integrations were performed by NCAR and CRIEPI with support\n",
			" and facilities provided by NSF, DOE, MEXT and ESC/JAMSTEC." ;
		:_Format = "classic" ;
}
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
  [-h]        Print output header\n\
  [-t]        Do a time-series read\n\
  [-v]        Vertical profile read\n\
  [-c CACHE_SIZE]        Set the HDF5 chunk cache to this size before read\n\
  file        Name of netCDF file\n"

static void
usage(void)
{
   fprintf(stderr, "tst_ar4 -h -t -v -c CACHE_SIZE file\n%s", USAGE);
}

#define NDIMS4 4
#define DATA_VAR_NAME "thetao"
#define NUM_CACHE_TRIES 1
#define LON_DIMID 0
#define LAT_DIMID 1
#define DEPTH_DIMID 2
#define BNDS_DIMID 3
#define TIME_DIMID 4
#define LON_LEN 320
#define LAT_LEN 395
#define BNDS_LEN 2
#define DEPTH_LEN 40
#define TIME_LEN 1200
#define NUM_TS 1
#define MAX_READ_COUNT 100

int 
main(int argc, char **argv)
{
   extern int optind;
   extern int opterr;
   extern char *optarg;
   int c, header = 0, vertical_profile = 0, timeseries = 0;
   int ncid, varid, storage;
   char name_in[NC_MAX_NAME + 1];
   size_t len;
   size_t cs[NDIMS4] = {0, 0, 0, 0};
   int cache = MEGABYTE;
   int ndims, dimid[NDIMS4];
   float hor_data[LAT_LEN * LON_LEN];
   float vert_data[DEPTH_LEN];
   int read_1_us, avg_read_us;
   float ts_data[TIME_LEN];
   size_t start[NDIMS4], count[NDIMS4];
   int deflate, shuffle, deflate_level;
   struct timeval start_time, end_time, diff_time;
   int read_count = 0, num_reads;

   while ((c = getopt(argc, argv, "vhtc:")) != EOF)
      switch(c) 
      {
	 case 'v':
	    vertical_profile++;
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
      else if (vertical_profile)
	 printf("\t1st_read_vert(us)\tavg_read_vert(us)\n");
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
   if (nc_inq_dim(ncid, DEPTH_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "depth") || len != DEPTH_LEN) ERR;
   if (nc_inq_dim(ncid, BNDS_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "bnds") || len != BNDS_LEN) ERR;
   if (nc_inq_dim(ncid, TIME_DIMID, name_in, &len)) ERR;
   if (strcmp(name_in, "time") || len != TIME_LEN) ERR;
   if (nc_inq_var(ncid, varid, NULL, NULL, &ndims, dimid, NULL)) ERR;
   if (ndims != NDIMS4 || dimid[0] != TIME_DIMID || 
       dimid[1] != DEPTH_DIMID || dimid[2] != LAT_DIMID || 
       dimid[3] != LON_DIMID) ERR;

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
      start[3] = 0;
      count[0] = TIME_LEN;
      count[1] = 1;
      count[2] = 1;
      count[3] = 1;
      
      /* Read the first timeseries. */
      if (gettimeofday(&start_time, NULL)) ERR;
      if (nc_get_vara_float(ncid, varid, start, count, ts_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      read_1_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

      /* Read all the rest. */
      if (gettimeofday(&start_time, NULL)) ERR;
      for (start[1] = 0; read_count < MAX_READ_COUNT && start[1] < LAT_LEN; start[1]++)
	 for (start[2] = 1; read_count < MAX_READ_COUNT && start[2] < LON_LEN; start[2]++)
	    for (start[3] = 1; read_count < MAX_READ_COUNT && start[3] < DEPTH_LEN; start[3]++)
	    {
	       if (nc_get_vara_float(ncid, varid, start, count, ts_data)) ERR_RET;
	       read_count++;
	    }
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      num_reads = (read_count == MAX_READ_COUNT) ? MAX_READ_COUNT : (LAT_LEN * LON_LEN * DEPTH_LEN);
      avg_read_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec + read_1_us) / 
	 num_reads;
   }
   else if (vertical_profile)
   {
      /* Read the var as a vertical profile. */
      start[0] = 0;
      start[1] = 0;
      start[2] = 0;
      start[3] = 0;
      count[0] = 1;
      count[1] = DEPTH_LEN;
      count[2] = 1;
      count[3] = 1;
      
      /* Read the first vertical profile. */
      if (gettimeofday(&start_time, NULL)) ERR;
      if (nc_get_vara_float(ncid, varid, start, count, vert_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      read_1_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

      /* Read all the rest. */
      if (gettimeofday(&start_time, NULL)) ERR;
/*       for (start[0] = 0; read_count < MAX_READ_COUNT && start[1] < LAT_LEN; start[1]++) */
/*       for (start[1] = 0; read_count < MAX_READ_COUNT && start[1] < LAT_LEN; start[1]++) */
/* 	 for (start[2] = 1; read_count < MAX_READ_COUNT && start[2] < LON_LEN; start[2]++) */
/* 	    for (start[] = 1; read_count < MAX_READ_COUNT && start[3] < DEPTH_LEN; start[3]++) */
/* 	    { */
/* 	       if (nc_get_vara_float(ncid, varid, start, count, vert_data)) ERR_RET; */
/* 	       read_count++; */
/* 	    } */
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      num_reads = (read_count == MAX_READ_COUNT) ? MAX_READ_COUNT : (LAT_LEN * LON_LEN * DEPTH_LEN);
      avg_read_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec + read_1_us) / 
	 num_reads;
   }
   else
   {
      /* Read the data variable in horizontal slices. */
      start[0] = 0;
      start[1] = 0;
      start[2] = 0;
      start[3] = 0;
      count[0] = 1;
      count[1] = 1;
      count[2] = LAT_LEN;
      count[3] = LON_LEN;

      /* Read (and time) the first one. */
      if (gettimeofday(&start_time, NULL)) ERR;
      if (nc_get_vara_float(ncid, varid, start, count, hor_data)) ERR_RET;
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      read_1_us = (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

      /* Read (and time) all the rest. */
      if (gettimeofday(&start_time, NULL)) ERR;
      for (start[0] = 0; read_count < MAX_READ_COUNT && start[0] < TIME_LEN; start[0]++)
	 for (start[1] = 1; read_count < MAX_READ_COUNT && start[1] < DEPTH_LEN; start[1]++)
	 {
	    if (nc_get_vara_float(ncid, varid, start, count, hor_data)) ERR_RET;
	    read_count++;
	 }
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      num_reads = (read_count == MAX_READ_COUNT) ? MAX_READ_COUNT : TIME_LEN;
      avg_read_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec + 
		     read_1_us) / num_reads;
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

