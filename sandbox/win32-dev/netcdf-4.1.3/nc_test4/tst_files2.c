/* This is part of the netCDF package.
   Copyright 2005 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test netcdf-4 variables. 
   $Id$
*/

#include <nc_tests.h>
#include "netcdf.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */

#define MAX_LEN 30
#define TMP_FILE_NAME "tst_files2_tmp.out"
#define FILE_NAME "tst_files2_1.nc"
#define MILLION 1000000

void *last_sbrk;

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

/* This function uses the ps command to find the amount of memory in
use by the process. From the ps man page:

size SZ approximate amount of swap space that would be required if
        the process were to dirty all writable pages and then be
        swapped out. This number is very rough!
*/
int
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
   if (!(fp = fopen(TMP_FILE_NAME, "r"))) ERR;
   num_char = fread(blob, MAX_LEN, 1, fp);
   sscanf(blob, "%d", mem_used);
   fclose(fp);
   unlink(TMP_FILE_NAME);
   return NC_NOERR;
}

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

void
get_mem_used3(int *mem_used)
{
   void *vp;
   vp = sbrk(0);
   *mem_used = ((char *)vp - (char *)last_sbrk)/1024;
}

/* Create a sample file, with num_vars 3D or 4D variables, with dim
 * lens of dim_len size. */
#define MAX_DIMS 4
int
create_sample_file(char *file_name, int ndims, int *dim_len, 
		   int num_vars, int mode, int num_recs)
{
   int ncid, dimids[MAX_DIMS], *varids;
   char varname[NC_MAX_NAME + 1];
   char dim_name[NC_MAX_NAME + 1];
   float *data_out;
   size_t start[MAX_DIMS], count[MAX_DIMS];
   int slab_nelems;
   int i, d, ret;

   if (ndims != MAX_DIMS && ndims != MAX_DIMS - 1) ERR_RET;

   /* Create a file. */
   ret = nc_create(file_name, NC_NOCLOBBER|mode, &ncid);
   if (ret == NC_EEXIST)
      return NC_NOERR;
   else if (ret)
      ERR_RET;

   /* Initialize sample data. Slab of data will be full extent of last
    * two dimensions. */
   slab_nelems = dim_len[ndims - 1] * dim_len[ndims - 2];
   if (!(data_out = malloc(slab_nelems * sizeof(float)))) ERR_RET;
   for (i = 0; i < slab_nelems; i++)
      data_out[i] = 42.42 + i;

   /* Create the dimensions. */
   for (d = 0; d < ndims; d++)
   {
      sprintf(dim_name, "dim_%d", d);
      if (nc_def_dim(ncid, dim_name, dim_len[d], &dimids[d])) ERR_RET;
   }

   /* Define num_vars variables. */
   if (!(varids = malloc(num_vars * sizeof(int)))) ERR_RET;
   for (i = 0; i < num_vars; i++)
   {
      sprintf(varname, "a_%d", i);
      if (nc_def_var(ncid, varname, NC_FLOAT, ndims, dimids, 
		     &varids[i])) ERR_RET;
   }
   
   /* Enddef required for classic files. */
   if (nc_enddef(ncid)) ERR;

   /* Set up start/count to write slabs of data. */
   for (d = 0; d < ndims; d++)
   {
      if (d < ndims - 2)
	 count[d] = 1;
      else
      {
	 start[d] = 0;
	 count[d] = dim_len[d];
      }
   }

   /* Now write some data to the vars in slabs. */
   for (i = 0; i < num_vars; i++)
   {
      if (ndims == MAX_DIMS)
      {
	 for (start[0] = 0; start[0] < (dim_len[0] ? dim_len[0] : num_recs); start[0]++)
	    for (start[1] = 0; start[1] < dim_len[1]; start[1]++)
	       if (nc_put_vara_float(ncid, varids[i], start, count, 
				     data_out)) ERR_RET;
      }
      else
      {
	 for (start[0] = 0; start[0] < (dim_len[0] ? dim_len[0] : num_recs); start[0]++)
	    if (nc_put_vara_float(ncid, varids[i], start, count, 
				  data_out)) ERR_RET;
      }
   }

   /* Free data and close file. */
   free(data_out);
   free(varids);
   if (nc_close(ncid)) ERR_RET;

   return NC_NOERR;
}

int
main(int argc, char **argv)
{

   printf("\n*** Testing netcdf-4 file functions, some more.\n");
   last_sbrk = sbrk(0);
   printf("*** testing lots of open files...\n");
   {
#define NUM_TRIES 6
      int *ncid_in;
      int mem_used, mem_used2;
      int mem_per_file;
      int num_files[NUM_TRIES] = {1, 1, 1, 1, 1, 1};
      char file_name[NUM_TRIES][NC_MAX_NAME + 1];
      int num_vars[NUM_TRIES];
      size_t cache_size[NUM_TRIES];
      int mode[NUM_TRIES];
      char mode_name[NUM_TRIES][8];
      int ndims[NUM_TRIES];
      int dim_len[NUM_TRIES][MAX_DIMS];
      int dim_4d[MAX_DIMS] = {NC_UNLIMITED, 10, 1000, 1000};
      char dimstr[30];
      char chunkstr[30];
      int num_recs[NUM_TRIES] = {1, 1, 1};
      struct timeval start_time, end_time, diff_time;
      struct timeval close_start_time, close_end_time, close_diff_time;
      int open_us, close_us, create_us;
      size_t chunksize[MAX_DIMS];
      int storage;
      int d, f, t;

      printf("dims\t\tchunks\t\tformat\tnum_files\tcache(kb)\tnum_vars\tmem(kb)\t"
	     "open_time(us)\tclose_time(us)\tcreate_time(us)\n");
      for (t = 0; t < NUM_TRIES; t++)
      {
	 /* Set up filename. */
	 sprintf(file_name[t], "tst_files2_%d.nc", t);
	 strcpy(mode_name[t], "netcdf4");
	 mode[t] = NC_NETCDF4;
	 cache_size[t] = 16000000;
	 num_vars[t] = 10;
	 ndims[t] = 4;
	 for (d = 0; d < ndims[t]; d++)
	    dim_len[t][d] = dim_4d[d];
	 
	 /* Create sample file (unless it already exists). */
	 if (gettimeofday(&start_time, NULL)) ERR;
	 if (create_sample_file(file_name[t], ndims[t], dim_len[t], num_vars[t],
				mode[t], num_recs[t])) ERR;

	 /* How long did it take? */
	 if (gettimeofday(&end_time, NULL)) ERR;
	 if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	 create_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec);

	 /* Change the cache settings. */
	 if (nc_set_chunk_cache(cache_size[t], 20000, .75)) ERR;
	 
	 /* We need storage for an array of ncids. */
	 if (!(ncid_in = malloc(num_files[t] * sizeof(int)))) ERR;

	 /* How much memory is in use now? */
 	 if (get_mem_used1(&mem_used)) ERR;
/* 	 get_mem_used2(&mem_used);
	 get_mem_used3(&mem_used);*/
	 
	 /* Open the first file to get chunksizes. */
	 if (gettimeofday(&start_time, NULL)) ERR;
	 if (nc_open(file_name[t], 0, &ncid_in[0])) ERR;
	 if (nc_inq_var_chunking(ncid_in[0], 0, &storage, chunksize)) ERR;

	 /* Now reopen this file a large number of times. */
	 for (f = 1; f < num_files[t]; f++)
	    if (nc_open(file_name[t], 0, &ncid_in[f])) ERR_RET;

	 /* How long did it take per file? */
	 if (gettimeofday(&end_time, NULL)) ERR;
	 if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	 open_us = ((int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec);

	 /* How much memory is in use by this process now? */
 	 if (get_mem_used1(&mem_used2)) ERR;

	 /* Close all netcdf files. */
	 if (gettimeofday(&close_start_time, NULL)) ERR;
	 for (f = 0; f < num_files[t]; f++)
	    if (nc_close(ncid_in[f])) ERR_RET;

	 /* How long did it take to close all files? */
	 if (gettimeofday(&close_end_time, NULL)) ERR;
	 if (timeval_subtract(&close_diff_time, &close_end_time, &close_start_time)) ERR;
	 close_us = ((int)close_diff_time.tv_sec * MILLION + (int)close_diff_time.tv_usec);

	 /* We're done with this. */
	 free(ncid_in);

	 /* How much memory was used for each open file? */
	 mem_per_file = mem_used2/num_files[t];

	 /* Prepare the dimensions string. */
	 if (ndims[t] == MAX_DIMS)
	    sprintf(dimstr, "%dx%dx%dx%d", dim_len[t][0], dim_len[t][1],
		    dim_len[t][2], dim_len[t][3]);
	 else
	    sprintf(dimstr, "%dx%dx%d", dim_len[t][0], dim_len[t][1],
		    dim_len[t][2]);

	 /* Prepare the chunksize string. */
	 if (storage == NC_CHUNKED)
	 {
	    if (ndims[t] == MAX_DIMS)
	       sprintf(chunkstr, "%dx%dx%dx%d", (int)chunksize[0], (int)chunksize[1],
		       (int)chunksize[2], (int)chunksize[3]);
	    else
	       sprintf(chunkstr, "%dx%dx%d", (int)chunksize[0], (int)chunksize[1],
		       (int)chunksize[2]);
	 }
	 else
	    strcpy(chunkstr, "contig       ");

	 /* Output results. */
	 printf("%s\t%s\t%s\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
		dimstr, chunkstr, mode_name[t], num_files[t], (int)(cache_size[t]/1024),
		num_vars[t], mem_used2, open_us, close_us, create_us);
      }
   }
  SUMMARIZE_ERR;
   printf("Test for memory consumption...\n");
   {
#define NUM_TRIES_100 100
      int ncid, i;
      int mem_used, mem_used1, mem_used2;

      get_mem_used2(&mem_used);
      mem_used1 = mem_used;
      mem_used2 = mem_used;
      printf("start: memuse= %d\t%d\t%d \n",mem_used, mem_used1,  
	     mem_used2);

      printf("bef_open\taft_open\taft_close\tused_open\tused_closed\n");
      for (i=0; i < NUM_TRIES_100; i++)
      {
	 /* Open the file. NC_NOWRITE tells netCDF we want read-only access
	  * to the file.*/

	 get_mem_used2(&mem_used);
	 nc_set_chunk_cache(10,10,.5);
	 if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
	 get_mem_used2(&mem_used1);

	 /* Close the file, freeing all resources.   ????  */
	 if (nc_close(ncid)) ERR;

	 get_mem_used2(&mem_used2);

	 if (mem_used2 - mem_used)
	    printf("try %d - %d\t\t%d\t\t%d\t\t%d\t\t%d \n", i, 
		   mem_used, mem_used1, mem_used2, mem_used1 - mem_used, 
		   mem_used2 - mem_used);
      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
