/*
  Copyright 2007, UCAR/Unidata
  See COPYRIGHT file for copying and redistribution conditions.

  This program benchmarks the write and read of some radar files with
  different chunking and compression parameters set.

  This program only works on classic model netCDF files. That is,
  groups, user-defined types, and other new netCDF-4 features are not
  handled by this program. (Input files may be in netCDF-4 format, but
  they must conform to the classic model for this program to work.)

  For the 3.7 and 4.0 netCDF releases, this program is not expected
  for general use. It may be made safer and more general in future
  releases, but for now, users should use this code with caution.

  $Id$
*/
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> /* Extra high precision time info. */
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef USE_PARALLEL
#include <mpi.h>
#endif
#include <nc_tests.h> /* The ERR macro is here... */
#include <netcdf.h> 

#define MILLION 1000000
#define BAD -99
#define NOMEM -98
#define MAX_VO 50  /* Max number of var options on command line. */
#define MAX_DIMS 7 /* Max dim for variables in input file. */

/* This struct holds data about what options we want to apply to
 * variable in the created file. (Chunking, compression, etc.) */
typedef struct {
      int varid;
      int ndims;
      int deflate_num;
      int shuffle;
      size_t chunksize[MAX_DIMS];
      int endian;
      size_t start[MAX_DIMS], count[MAX_DIMS], inc[MAX_DIMS];
} VAR_OPTS_T;

/* This macro prints an error message with line number and name of
 * test program. */
#define ERR1(n) do {						  \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d - %s\n", \
	__FILE__, __LINE__, nc_strerror(n));			 \
return n; \
} while (0)

#ifdef USE_PARALLEL
/* Error handling code for MPI calls. */
#define MPIERR(e) do { \
MPI_Error_string(e, err_buffer, &resultlen); \
printf("MPI error, line %d, file %s: %s\n", __LINE__, __FILE__, err_buffer); \
MPI_Finalize(); \
return 2; \
} while (0) 
#endif

/* This function will fill the start and count arrays for the reads
 * and writes. */
static int 
get_starts_counts(int ndims, size_t *dimlen, int p, int my_rank, 
		  int slow_count, int use_scs, VAR_OPTS_T *vo, 
		  int *num_steps, int *start_inc, int *slice_len, 
		  size_t *last_count, size_t *start, size_t *count)
{
   int extra_step = 0;
   int total[NC_MAX_DIMS];
   int total_len;
   int s, d;

   /* User has specified start/count/inc for this var. Parallel runs
    * not allowed yet. */
   if (use_scs)
   {
      /* Set the starts and counts for each dim, the len of the slice,
       * the total len of the data, and the total extent of the
       * dataset in each dimension. */
      for (d = 0, *slice_len = 1, total_len = 1; d < vo->ndims; d++)
      {
	 start[d] = vo->start[d];
	 count[d] = vo->count[d];
	 (*slice_len) *= count[d];
	 total_len *= dimlen[d];
      }

      /* The start increment is provided by the user. */
      *start_inc = vo->inc[0];

      /* How many steps to write/read these data? */
      *num_steps = total_len / (*slice_len);

      /* Init this for the total extent in each dim. */
      for (d = 0; d < vo->ndims; d++)
	 total[d] = 0;

      /* Check our numbers if we apply increments to start, and read
       * count, for this many steps. */
      for (s = 0; s < *num_steps; s++)
      {
	 for (d = 0; d < vo->ndims; d++)
	 {
	    total[d] += count[d];
	    if (total[d] >= dimlen[d])
	       break;
	 }
	 if (d != vo->ndims)
	    break;
      }

      /* If the numbers didn't come out clean, then figure out the
       * last set of counts needed to completely read the data. */
      if (s == (*num_steps) - 1)
	 *last_count =  count[0];
      else
      {
	 (*num_steps)++;
	 *last_count =  dimlen[0] - total[0];
      }
   }
   else
   {
      *start_inc = dimlen[0]/slow_count;
      while (*start_inc * slow_count < dimlen[0])
	 (*start_inc)++;
      *slice_len = *start_inc;
      start[0] = *start_inc * my_rank;
      if (start[0] > dimlen[0])
      {
	 fprintf(stderr, "slow_count too large for this many processors, "
		 "start_inc=%d, slow_count=%d, p=%d, my_rank=%d start[0]=%ld\n", 
		 *start_inc, slow_count, p, my_rank, start[0]);
	 return 2;
      }
      count[0] = *start_inc;
      for (d = 1; d < ndims; d++)
      {
	 start[d] = 0;
	 count[d] = dimlen[d];
	 *slice_len *= dimlen[d];
      }
      *num_steps = (float)dimlen[0] / (*start_inc * p);
      if ((float)dimlen[0] / (*start_inc * p) != dimlen[0] / (*start_inc * p))
      {
	 extra_step++;
	 (*num_steps)++;
      }

      if (p > 1)
      {
	 if (!extra_step)
	    *last_count = 0;
	 else
	 {
	    int left;
	    left = dimlen[0] - (*num_steps - 1) * *start_inc * p;
	    if (left > (my_rank + 1) * *start_inc)
	       *last_count = *start_inc;
	    else
	    {
	       if (left - my_rank * *start_inc < 0)
		  *last_count = 0;
	       else
		  *last_count = left - my_rank * *start_inc;
	    }
	 }
      }
      else
	 *last_count = dimlen[0] - (*num_steps - 1) * *start_inc;
   }
   return 0;
}

/* This function finds the size of a file. */
static size_t
file_size(char* name)
{
   struct stat stbuf;
   stat(name, &stbuf);
   return stbuf.st_size;
}

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

/* Check attribute number a of variable varid in copied file ncid2 to ensure
 * it is the same as the corresponding attribute in original file ncid1. */
static int
check_att(int ncid1, int ncid2, int varid, int a)
{
   int typeid, typeid2;
   size_t len, len2, typelen;
   char name[NC_MAX_NAME + 1];
   void *d = NULL, *d2 = NULL;
   int ret = 0;

   /* Check the metadata about the metadata - name, type, length. */
   if ((ret = nc_inq_attname(ncid1, varid, a, name)))
      return ret;
   if ((ret = nc_inq_att(ncid1, varid, name, &typeid, &len)))
      return ret;
   if ((ret = nc_inq_att(ncid2, varid, name, &typeid2, &len2)))
      return ret;
   if (len != len2 || typeid != typeid2) 
      return BAD;
   if ((ret = nc_inq_type(ncid1, typeid, NULL, &typelen)))
      return ret;

   /* Get the two attributes, if they are non-zero. */
   if (len)
   {
      if(!(d = malloc(typelen * len)))
	 return NOMEM;
      if(!(d2 = malloc(typelen * len)))
      {
	 ret = NOMEM;
	 goto exit;
      }
      if ((ret = nc_get_att(ncid1, varid, name, d)))
	 goto exit;
      if ((ret = nc_get_att(ncid2, varid, name, d2)))
	 goto exit;
      
      /* Are they the same? */
      if (memcmp(d, d2, typelen * len))
	 ret = BAD;
   }

  exit:
   /* Free up our resources. */
   if (d)
      free(d);
   if (d2)
      free(d2);

   return ret;
}

/* Do two files contain the same data and metadata? */
static int 
cmp_file(char *file1, char *file2, int *meta_read_us, int *data_read_us,
	 int use_par, int par_access, int do_cmp, int p, int my_rank, 
	 int slow_count, int verbose, int num_vo, VAR_OPTS_T *vo, int use_scs)
{
   int ncid1, ncid2;
   int unlimdimid, unlimdimid2;
   char name[NC_MAX_NAME + 1], name2[NC_MAX_NAME + 1];
   size_t len, len2;
#ifdef USE_PARALLEL
   double ftime;
#endif
   struct timeval start_time, end_time, diff_time;
   void *data = NULL, *data2 = NULL;
   int a, v, d;
   nc_type xtype, xtype2;
   int nvars, ndims, dimids[NC_MAX_DIMS], natts, real_ndims;
   int nvars2, ndims2, dimids2[NC_MAX_DIMS], natts2;
   size_t *count = NULL, *start = NULL;
   int slice_len = 1;
   size_t *dimlen = NULL, type_size = 0;
   size_t last_count;
   int start_inc;
   int num_steps, step;
   int ret = NC_NOERR;

   /* Note in the code below I only want to time stuff for file2. */

   /* Read the metadata for both files. */
   if (use_par)
   {
#ifdef USE_PARALLEL
      if ((ret = nc_open_par(file1, 0, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid1)))
	 ERR1(ret);
      MPI_Barrier(MPI_COMM_WORLD);
      ftime = MPI_Wtime();
      if ((ret = nc_open_par(file2, 0, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid2)))
	 ERR1(ret);
      *meta_read_us += (MPI_Wtime() - ftime) * MILLION;
#else
      return NC_EPARINIT;
#endif
   }
   else
   {
      if ((ret = nc_open(file1, 0, &ncid1)))
	 ERR1(ret);
      if (gettimeofday(&start_time, NULL)) ERR;
      if ((ret = nc_open(file2, 0, &ncid2)))
	 ERR1(ret);
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      *meta_read_us += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
   }
   if (verbose)
      printf("%d: reading metadata took %d micro-seconds\n",
	     my_rank, *meta_read_us);

   /* Check the counts of dims, vars, and atts. */
   if ((ret = nc_inq(ncid1, &ndims, &nvars, &natts, &unlimdimid)))
      ERR1(ret);
   if ((ret = nc_inq(ncid1, &ndims2, &nvars2, &natts2, &unlimdimid2)))
      ERR1(ret);
   if (ndims != ndims2 || nvars != nvars2 || natts != natts2 ||
       unlimdimid != unlimdimid2)
      ERR1(BAD);

   /* Check dims. */
   for (d = 0; d < ndims; d++)
   {
      if ((ret = nc_inq_dim(ncid1, d, name, &len))) 
	 ERR1(ret);
      if ((ret = nc_inq_dim(ncid2, d, name2, &len2)))
	 ERR1(ret);
      if (len != len2 || strcmp(name, name2)) 
	 ERR1(BAD);
   }

   /* Check global atts. */
   for (a = 0; a < natts; a++)
      if ((ret = check_att(ncid1, ncid2, NC_GLOBAL, a)))
	 ERR1(ret);

   /* Check the variables. */
   for (v = 0; v < nvars; v++)
   {
      /* Learn about this var in both files. */
      if ((ret = nc_inq_var(ncid1, v, name, &xtype, &ndims, dimids, &natts)))
	 return ret;
      if ((ret = nc_inq_var(ncid2, v, name2, &xtype2, &ndims2, dimids2, &natts2)))
	 return ret;

      /* Check var metadata. */
      if (strcmp(name, name2) || xtype != xtype2 || ndims != ndims2 || natts != natts2)
	 return BAD;
      for (d = 0; d < ndims; d++)
	 if (dimids[d] != dimids2[d])
	    return BAD;

      /* Check the attributes. */
      for (a = 0; a < natts; a++)
	 if ((ret = check_att(ncid1, ncid2, v, a)))
	    ERR1(ret);

      /* Check the data, one slice at a time. (slicing along slowest
       * varying dimension.) */

      /* Allocate memory for our start and count arrays. If ndims = 0
	 this is a scalar, which I will treat as a 1-D array with one
	 element. */
      real_ndims = ndims ? ndims : 1;
      if (!(start = malloc(real_ndims * sizeof(size_t))))
	 ERR1(NC_ENOMEM);
      if (!(count = malloc(real_ndims * sizeof(size_t))))
	 ERR1(NC_ENOMEM);

      /* The start array will be all zeros, except the first element,
	 which will be the slice number. Count will be the dimension
	 size, except for the first element, which will be one, because
	 we will copy one slice at a time. For this we need the var
	 shape. */
      if (!(dimlen = malloc(real_ndims * sizeof(size_t))))
	 ERR1(NC_ENOMEM);
      for (d=0; d<ndims; d++)
	 if ((ret = nc_inq_dimlen(ncid1, dimids[d], &dimlen[d])))
	    ERR1(ret);

      /* If this is a scalar, then set the dimlen to 1. */
      if (ndims == 0)
	 dimlen[0] = 1;

      if ((ret = get_starts_counts(ndims, dimlen, p, my_rank, slow_count, use_scs, 
				   &vo[v], &num_steps, &start_inc, &slice_len, 
				   &last_count, start, count)))
	 return ret;
      if (verbose)
	 printf("%d: num_steps=%d, start_inc=%d, slice_len=%d, last_count=%ld\n", 
		my_rank, num_steps, start_inc, slice_len, last_count);

      /* If there are no records, we're done. */
      if (!dimlen[0])
	 goto exit;

      /* Find the size of this type. */
      if ((ret = nc_inq_type(ncid1, xtype, NULL, &type_size)))
	 return ret;

      /* I will read all this data the same way I eat a large pizze -
       * one slice at a time. */
      if (!(data = malloc(slice_len * type_size)))
	 ERR1(NC_ENOMEM);
      if (!(data2 = malloc(slice_len * type_size)))
	 ERR1(NC_ENOMEM);
   
      /* Check the var data for each slice. */
/*      for (step = 0; !ret && step < num_steps; step++)*/
      for (step = 0; !ret && step < num_steps; step++)
      {
	 if (step == num_steps - 1 && last_count)
	    count[0] = last_count;

	 /* Read data from file1. */
	 if (nc_get_vara(ncid1, v, start, count, data)) ERR;

	 /* Read data from file2. */
#ifdef USE_PARALLEL
	 ftime = MPI_Wtime();      
#else
	 if (gettimeofday(&start_time, NULL)) ERR;
#endif
	 if (nc_get_vara(ncid2, v, start, count, data2)) ERR;
#ifdef USE_PARALLEL
	 *data_read_us += (MPI_Wtime() - ftime) * MILLION;
#else
	 if (gettimeofday(&end_time, NULL)) ERR;
	 if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	 *data_read_us += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
#endif
	 if (verbose)
	    printf("%d: reading copy step %d, var %d took %d micro-seconds\n",
		   my_rank, step, v, *data_read_us);

	 /* Check data. */
	 if (do_cmp)
	    if (memcmp(data, data2, slice_len * type_size))
	       ERR1(BAD);

	 /* Increment the start index for the slowest-varying
	  * dimension. */
	 start[0] += start_inc;
      }
      
     exit:
      if (data) free(data);
      if (data2) free(data2);
      if (dimlen) free(dimlen);
      if (start) free(start);
      if (count) free(count);
   }

   if ((ret = nc_close(ncid1)))
      ERR1(ret);
   if ((ret = nc_close(ncid2)))
      ERR1(ret);

   return 0;
}

/* Copy a netCDF file, changing cmode if desired, applying chuncking,
 * deflate, shuffle, and endianness parameters if desired. */
static
int copy_file(char *file_name_in, char *file_name_out, int cmode_out,
	      int num_vo, VAR_OPTS_T *vo, int *meta_read_us, int *meta_write_us, 
	      int *data_read_us, int *data_write_us, int *in_format, int use_par, 
	      int par_access, long long *num_bytes, int p, int my_rank, 
	      int slow_count, int verbose, int use_scs, int endianness,
	      int convert_unlim)
{
   int ncid_in, ncid_out;
   int natts, nvars, ndims, unlimdimid;
   char name[NC_MAX_NAME + 1];
   size_t len;
   size_t last_count;
   int a, v, d;
   int ret;
   struct timeval start_time, end_time, diff_time;
#ifdef USE_PARALLEL
   double ftime;
#endif

   if (use_par)
   {
#ifdef USE_PARALLEL
      ftime = MPI_Wtime();      
      if ((ret = nc_open_par(file_name_in, 0, MPI_COMM_WORLD, MPI_INFO_NULL, &ncid_in)))
	 ERR1(ret);
      *meta_read_us += (MPI_Wtime() - ftime) * MILLION;
#else
      return NC_EPARINIT;
#endif
   }
   else
   {
      if (gettimeofday(&start_time, NULL)) ERR;
      if ((ret = nc_open(file_name_in, 0, &ncid_in)))
	 ERR1(ret);
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      *meta_read_us += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
   }
   if (verbose)
      printf("%d: reading metadata took %d micro-seconds.\n", my_rank, *meta_read_us);

   /* Only classic model files may be used as input. */
   if ((ret = nc_inq_format(ncid_in, in_format)))
      ERR1(ret);
   if (*in_format == NC_FORMAT_NETCDF4)
      ERR1(NC_ENOTNC3);

   if (strlen(file_name_out))
   {
      if (use_par)
      {
#ifdef USE_PARALLEL
	 if ((ret = nc_create_par(file_name_out, cmode_out, MPI_COMM_WORLD, 
				  MPI_INFO_NULL, &ncid_out)))
	    ERR1(ret);
#else
	 return NC_EPARINIT;
#endif
      }
      else
      {
#define SIXTEEN_MEG 16777216
#define PREEMPTION .75
#define NELEMS 7919
	 if ((ret = nc_set_chunk_cache(SIXTEEN_MEG, NELEMS, PREEMPTION)))
	    ERR1(ret);
	 if ((ret = nc_create(file_name_out, cmode_out, &ncid_out)))
	    ERR1(ret);
      }
   }
      
   if ((ret = nc_inq(ncid_in, &ndims, &nvars, &natts, &unlimdimid)))
      ERR1(ret);

   if (strlen(file_name_out))
   {
      /* Copy dims. */
      for (d = 0; d < ndims; d++)
      {
	 if ((ret = nc_inq_dim(ncid_in, d, name, &len))) 
	    ERR1(ret);
	 if (convert_unlim)
	 {
	    if ((ret = nc_def_dim(ncid_out, name, len, NULL)))
	       ERR1(ret);
	 }
	 else
	 {
	    if ((ret = nc_def_dim(ncid_out, name, 
				  (d == unlimdimid) ? NC_UNLIMITED : len, 
				  NULL)))
	       ERR1(ret);
	 }
      }
      
      /* Copy global atts. */
      for (a = 0; a < natts; a++)
      {
	 if (nc_inq_attname(ncid_in, NC_GLOBAL, a, name)) ERR;
	 if (nc_copy_att(ncid_in, NC_GLOBAL, name, ncid_out, NC_GLOBAL)) ERR;
      }
      
      /* Copy the variable metadata. */
      for (v = 0; v < nvars; v++)
      {
	 char name[NC_MAX_NAME + 1];
	 char att_name[NC_MAX_NAME + 1];
	 nc_type xtype;
	 int ndims, dimids[NC_MAX_DIMS], natts;
	 int varid_out;
	 int a, o1;
	 int ret = NC_NOERR;
	 
	 /* Learn about this var. */
	 if ((ret = nc_inq_var(ncid_in, v, name, &xtype, &ndims, dimids, &natts)))
	    return ret;
	 
	 /* Create the output var. */
	 if (nc_def_var(ncid_out, name, xtype, ndims, dimids, &varid_out)) ERR;
	 
	 /* Set the output endianness. For simplicity in this program,
	  * all vars get the same endianness. But there's no reason why
	  * this couldn't be varied from var to var, though it is hard to
	  * see why one would do so. */
	 if (endianness)
	    if (nc_def_var_endian(ncid_out, varid_out, endianness)) ERR;
	 
	 /* Sent chunking and compression if specified in the var options. */
	 for (o1 = 0; o1 < num_vo; o1++)
	    if (vo[o1].varid == v)
	    {
	       if (vo[o1].chunksize[0])
	       {
		  if (nc_def_var_chunking(ncid_out, v, 0, vo[o1].chunksize)) ERR;
	       }
	       else
	       {
		  if (nc_def_var_chunking(ncid_out, v, 1, NULL)) ERR;
	       }
	       if (vo[o1].deflate_num != -1)
		  if (nc_def_var_deflate(ncid_out, v, vo[o1].shuffle, 1, vo[o1].deflate_num)) ERR;
	       break;
	    }
	 
	 /* Copy the attributes. */
	 for (a=0; a<natts; a++)
	 {
	    if (nc_inq_attname(ncid_in, v, a, att_name)) ERR;
	    if (nc_copy_att(ncid_in, v, att_name, ncid_out, varid_out)) ERR;
	 }
      }
      
#ifdef USE_PARALLEL
      ftime = MPI_Wtime();      
#else
      if (gettimeofday(&start_time, NULL)) ERR;
#endif
      if ((ret = nc_enddef(ncid_out)))
	 ERR1(ret);
#ifdef USE_PARALLEL
      *meta_write_us += (MPI_Wtime() - ftime) * MILLION;
#else
      if (gettimeofday(&end_time, NULL)) ERR;
      if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
      *meta_write_us += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
#endif
      
      if (verbose)
	 printf("%d: copying %d vars, %d global atts, and %d dims took %d micro-seconds\n",
		my_rank, nvars, natts, ndims, *meta_write_us);
   }

   /* Copy the variable data. */
   for (v = 0; v < nvars; v++)
   {
      char name[NC_MAX_NAME + 1];
      nc_type xtype;
      int ndims, dimids[NC_MAX_DIMS], natts, real_ndims;
      int d;
      void *data = NULL;
      size_t *count = NULL, *start = NULL;
      int slice_len = 1;
      size_t *dimlen = NULL;
      int ret = NC_NOERR;
      size_t type_size;
      char type_name[NC_MAX_NAME+1];
      int start_inc;
      int step, num_steps;
      int var_num_bytes;

      /* Learn about this var. */
      if ((ret = nc_inq_var(ncid_in, v, name, &xtype, &ndims, dimids, &natts)))
	 return ret;

      /* Later on, we will need to know the size of this type. */
      if ((ret = nc_inq_type(ncid_in, xtype, type_name, &type_size)))
	 return ret;

      /* Allocate memory for our start and count arrays. If ndims = 0
	 this is a scalar, which I will treat as a 1-D array with one
	 element. */
      real_ndims = ndims ? ndims : 1;

      /* Get the variable shape information. */
      if (!(dimlen = malloc(real_ndims * sizeof(size_t))))
	 ERR1(NC_ENOMEM);
      for (d = 0; d < ndims; d++)
	 if ((ret = nc_inq_dimlen(ncid_in, dimids[d], &dimlen[d])))
	    ERR1(ret);

      if (!(start = malloc(real_ndims * sizeof(size_t))))
	 ERR1(NC_ENOMEM);
      if (!(count = malloc(real_ndims * sizeof(size_t))))
	 ERR1(NC_ENOMEM);

      /* If this is really a scalar, then set the dimlen to 1. */
      if (ndims == 0)
	 dimlen[0] = 1;

      /* Get the start and count arrays, and also the increment of the
       * start array zeroth element, the number of read steps, the
       * lenght of a slice in number of elements, and the count needed
       * for the final read, in the cases where the length of the
       * zeroth dimension is not evenly divisible by slow_count. The
       * variable slow_count is the number of elements in the slowest
       * varying (i.e. the zeroth) dimension to read at one time. For
       * vars with an unlimited dimension, this is the number of
       * records to read at once. */
      if ((ret = get_starts_counts(ndims, dimlen, p, my_rank, slow_count, use_scs, 
				   &vo[v], &num_steps, &start_inc, &slice_len,
				   &last_count, start, count)))
	 return ret;
      if (verbose)
	 printf("%d: num_steps=%d, start_inc=%d, slice_len=%d, last_count=%ld\n", 
		my_rank, num_steps, start_inc, slice_len, last_count);

      /* If there are no records, we're done. */
      if (!dimlen[0])
	 goto exit;

      /* Allocate memory for one slice. */
      if (!(data = malloc(slice_len * type_size)))
	 return NC_ENOMEM;
   
      /* Copy the var data one slice at a time. */
      for (step = 0; !ret && step < num_steps; step++)
      {
	 /* Make sure count is not too big. */
	 if (step == num_steps - 1 && last_count)
	    count[0] = last_count;

/* 	 for (d=0; d<ndims; d++) */
/* 	    printf("start[%d]=%d count[%d]=%d dimlen[%d]=%d, step=%d\n", */
/* 		   d, start[d], d, count[d], d, dimlen[d], step); */

	 /* Read input data. */
#ifdef USE_PARALLEL
	 ftime = MPI_Wtime();      
#else
	 if (gettimeofday(&start_time, NULL)) ERR;
#endif
	 if ((ret = nc_get_vara(ncid_in, v, start, count, data)))
	    ERR1(ret);

#ifdef USE_PARALLEL
	 *data_read_us += (MPI_Wtime() - ftime) * MILLION;
#else
	 if (gettimeofday(&end_time, NULL)) ERR;
	 if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	 *data_read_us += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
#endif
	 if (verbose)
	    printf("%d: reading step %d, var %d took %d micro-seconds\n",
		   my_rank, step, v, *data_read_us);

	 /* Write the data to the output file. */
	 if (strlen(file_name_out))
	 {
#ifdef USE_PARALLEL
	    ftime = MPI_Wtime();
#else
	    if (gettimeofday(&start_time, NULL)) ERR;
#endif
	    if ((ret = nc_put_vara(ncid_out, v, start, count, data)))
	       ERR1(ret);
#ifdef USE_PARALLEL
	    *data_write_us += (MPI_Wtime() - ftime) * MILLION;
#else
	    if (gettimeofday(&end_time, NULL)) ERR;
	    if (timeval_subtract(&diff_time, &end_time, &start_time)) ERR;
	    *data_write_us += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;
#endif
	    if (verbose)
	       printf("%d: writing step %d, var %d took %d micro-seconds\n",
		      my_rank, step, v, *data_write_us);
	 }
	 
	 /* Increment start index. */
	 start[0] += start_inc;
      } /* next step */
      
      /* Calculate the data read and write rates in MB/sec. */
      for (d = 0, var_num_bytes = type_size; d < ndims; d++)
	 var_num_bytes *= dimlen[d];
      (*num_bytes) += var_num_bytes;
      
     exit:
      if (data) free(data);
      if (dimlen) free(dimlen);
      if (start) free(start);
      if (count) free(count);
   } /* next var */

   if (nc_close(ncid_in)) ERR;
   if (strlen(file_name_out))
      if (nc_close(ncid_out)) ERR;

   return NC_NOERR;
}

#define NDIMS 3
#define MAX_DEFLATE 9      
#define INPUT_FILE "/upc/share/testdata/nssl/mosaic3d_nc/tile1/20070803-2300.netcdf"
#define COLON ":"
#define COMMA ","

#define USAGE   "\
  [-v]        Verbose\n\
  [-o file]   Output file name\n\
  [-f N]      Output format (1 - classic, 2 - 64-bit offset, 3 - netCDF-4, 4 - netCDF4/CLASSIC)\n\
  [-h]        Print output header\n\
  [-c V:Z:S:C:C:C[,V:Z:S:C:C:C, etc.]] Deflate, shuffle, and chunking parameters for vars\n\
  [-t V:S:S:S[,V:S:S:S, etc.]] Starts for reads/writes\n\
  [-u V:C:C:C[,V:C:C:C, etc.]] Counts for reads/writes\n\
  [-r V:I:I:I[,V:I:I:I, etc.]] Incs for reads/writes\n\
  [-d]        Doublecheck output by rereading each value\n\
  [-m]        Do compare of each data value during doublecheck (slow for large files!)\n\
  [-p]        Use parallel I/O\n\
  [-s N]      Denom of fraction of slowest varying dimension read.\n\
  [-i]        Use MPIIO (only relevant for parallel builds).\n\
  [-l]        Convert unlimited dimensions to fixed dimensions.\n\
  [-e 1|2]    Set the endianness of output (1=little 2=big).\n\
  file        Name of netCDF file\n"

static void
usage(void)
{
   fprintf(stderr, "bm_file -v [-s N]|[-t V:S:S:S -u V:C:C:C -r V:I:I:I] -o file_out -f N -h"
	   " -c V:C:C,V:C:C:C -d -m -p -i -e 1|2 -l file\n%s", USAGE);
}

int
main(int argc, char **argv)
{
   int num_vo = 0;
   extern int optind;
   extern int opterr;
   extern char *optarg;
   char file_in[NC_MAX_NAME + 1], file_out[NC_MAX_NAME + 1] = {""};
   int c;
   int out_format, in_format, header = 0, doublecheck = 0;
   int convert_unlim = 0;
   char *str1, *str2, *token, *subtoken;
   char *saveptr1, *saveptr2;
   int i, ndims, o1;
   int cmode = 0;
   int mpiio = 0;
   int meta_read_us = 0, meta_write_us = 0, data_read_us = 0, data_write_us = 0;
   int meta_read2_us = 0, data_read2_us = 0;
   int tmeta_read_us = 0, tmeta_write_us = 0, tdata_read_us = 0, tdata_write_us = 0;
   int tmeta_read2_us = 0, tdata_read2_us = 0;
   VAR_OPTS_T vo[MAX_VO];
   int use_par = 0, par_access = 0;
   int do_cmp = 0, verbose = 0;
   int ret;
   float read_rate, write_rate, reread_rate;
   int slow_count = 10, use_scs = 0;
   int endianness = 0;
   long long num_bytes = 0;
   int p = 1, my_rank = 0;
   int v, d;

#ifdef USE_PARALLEL
   MPI_Init(&argc, &argv);
   MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &p);
#endif

   for (o1 = 0; o1 < MAX_VO; o1++)
      for (i = 0; i < MAX_DIMS; i++)
	 vo[o1].chunksize[i] = 0;

   while ((c = getopt(argc, argv, "vo:f:hc:dpms:it:u:r:e:l")) != EOF)
      switch(c) 
      {
	 case 'v':
	    verbose++;
	    break;
	 case 'o':
	    strcpy(file_out, optarg);
	    break;
	 case 'f':
	    sscanf(optarg, "%d", &out_format);
	    switch (out_format)
	    {
	       case NC_FORMAT_CLASSIC:
		  break;
	       case NC_FORMAT_64BIT:
		  cmode = NC_64BIT_OFFSET;
		  break;
	       case NC_FORMAT_NETCDF4:
		  cmode = NC_NETCDF4;
		  break;
	       case NC_FORMAT_NETCDF4_CLASSIC:
		  cmode = NC_NETCDF4|NC_CLASSIC_MODEL;
		  break;
	      default:
		 usage();
		 return 1;
	    }
	    break;
	 case 'h':
	    header++;
	    break;
	 case 'c':
	    for (num_vo = 0, str1 = optarg; ; num_vo++, str1 = NULL) 
	    {
	       int got_z = 0, got_s = 0;
	       if (num_vo > MAX_VO)
		  return 1;
	       if (!(token = strtok_r(str1, COMMA, &saveptr1)))
		  break;
               for (ndims = 0, str2 = token; ; str2 = NULL) 
	       {
		  int tmp_int;
		  if (!(subtoken = strtok_r(str2, COLON, &saveptr2)))
		     break;
		  if (str2)
		     sscanf(subtoken, "%d", &(vo[num_vo].varid));
		  else if (!got_z++)
		     sscanf(subtoken, "%d", &(vo[num_vo].deflate_num));
		  else if (!got_s++)
		     sscanf(subtoken, "%d", &(vo[num_vo].shuffle));
		  else
		  {
		     sscanf(subtoken, "%d", &tmp_int);
		     vo[num_vo].chunksize[ndims++] = tmp_int;
		  }
               }
	       vo[num_vo].ndims = ndims;
	    }
	    break;
	 case 't':
	    for (num_vo = 0, str1 = optarg; ; num_vo++, str1 = NULL) 
	    {
	       if (num_vo > MAX_VO)
		  return 1;
	       if (!(token = strtok_r(str1, COMMA, &saveptr1)))
		  break;
               for (ndims = 0, str2 = token; ; str2 = NULL) 
	       {
		  if (!(subtoken = strtok_r(str2, COLON, &saveptr2)))
		     break;
		  if (str2)
		     sscanf(subtoken, "%d", &(vo[num_vo].varid));
		  else
		     sscanf(subtoken, "%ld", &(vo[num_vo].start[ndims++]));
               }
	       vo[num_vo].ndims = ndims;
	    }
	    use_scs++;
	    break;
	 case 'u':
	    for (num_vo = 0, str1 = optarg; ; num_vo++, str1 = NULL) 
	    {
	       if (num_vo > MAX_VO)
		  return 1;
	       if (!(token = strtok_r(str1, COMMA, &saveptr1)))
		  break;
               for (ndims = 0, str2 = token; ; str2 = NULL) 
	       {
		  if (!(subtoken = strtok_r(str2, COLON, &saveptr2)))
		     break;
		  if (str2)
		     sscanf(subtoken, "%d", &(vo[num_vo].varid));
		  else
		     sscanf(subtoken, "%ld", &(vo[num_vo].count[ndims++]));
               }
	       vo[num_vo].ndims = ndims;
	    }
	    break;
	 case 'r':
	    for (num_vo = 0, str1 = optarg; ; num_vo++, str1 = NULL) 
	    {
	       if (num_vo > MAX_VO)
		  return 1;
	       if (!(token = strtok_r(str1, COMMA, &saveptr1)))
		  break;
               for (ndims = 0, str2 = token; ; str2 = NULL) 
	       {
		  if (!(subtoken = strtok_r(str2, COLON, &saveptr2)))
		     break;
		  if (str2)
		     sscanf(subtoken, "%d", &(vo[num_vo].varid));
		  else
		     sscanf(subtoken, "%ld", &(vo[num_vo].inc[ndims++]));
               }
	       vo[num_vo].ndims = ndims;
	    }
	    break;
	 case 'd':
	    doublecheck++;
	    break;
	 case 'm':
	    do_cmp++;
	    doublecheck++;
	    break;
	 case 'p':
	    use_par++;
	    break;
	 case 'i':
	    mpiio++;
	    break;
	 case 's':
	    sscanf(optarg, "%d", &slow_count);
	    break;
	 case 'e':
	    sscanf(optarg, "%d", &endianness);
	    break;
	 case 'l':
	    convert_unlim++;
	    break;
	 case '?':
	    usage();
	    return 1;
      }

   if (mpiio)
      cmode |= NC_MPIIO;

   if (use_scs)
   {
      if (use_par)
      {
	 printf("Can't use start/count/slice for paralell runs yet!\n");
	 return 2;
      }
   }
   else
   {
      if (slow_count < p)
	 slow_count = p;
      if (slow_count % p)
      {
	 printf("slow_count must be even multiple of p\n");
	 return 2;
      }
   }

   argc -= optind;
   argv += optind;

   /* If no file arguments left, print usage message. */
   if (argc < 1)
   {
      usage();
      return 0;
   }

   /* Get the name of the file to copy. */
   strcpy(file_in, argv[0]);

   /* Verbose mode seems a bit stupid, but it's really useful when you
    * are running in batch mode on a supercomputer, and can't use
    * anything else to figure out what the heck is going on. */
   if (verbose && !my_rank)
   {
      printf("copying %s to %s on %d processors with endianness %d and...\n", 
	     file_in, file_out, p, endianness);
      if (use_scs)
	 for (v = 0; v < num_vo; v++) 
	 {
	    printf("options for var %d:\n", vo[v].varid);
	    for (d = 0; d < vo[v].ndims; d++)
	       printf("start[%d]=%ld, count[%d]=%ld, inc[%d]=%ld\n",
		      d, vo[v].start[d], d, vo[v].count[d], d, vo[v].inc[d]);
	 }
      else
	 printf("slow_count=%d, doublecheck=%d\n", slow_count, doublecheck);
   }

   /* Copy the file, keeping track of the read and write times for metadata and data. */
   if ((ret = copy_file(file_in, file_out, cmode, num_vo, vo, &meta_read_us, &meta_write_us, 
			&data_read_us, &data_write_us, &in_format, use_par, par_access, 
			&num_bytes, p, my_rank, slow_count, verbose, use_scs, endianness,
			convert_unlim)))
      return ret;

   /* If the user wants a double check, make sure the data in the new
    * file is exactly the same. */
   if (doublecheck)
   {
#ifdef USE_PARALLEL
      MPI_Barrier(MPI_COMM_WORLD);
#endif      
      if ((ret = cmp_file(file_in, file_out, &meta_read2_us, &data_read2_us, 
			  use_par, par_access, do_cmp, p, my_rank, slow_count, 
			  verbose, num_vo, vo, use_scs)))
	 return ret;
   }

   if (use_par)
   {
#ifdef USE_PARALLEL
      MPI_Reduce(&meta_read_us, &tmeta_read_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      MPI_Reduce(&meta_write_us, &tmeta_write_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      MPI_Reduce(&data_read_us, &tdata_read_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      MPI_Reduce(&data_write_us, &tdata_write_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      MPI_Reduce(&data_read2_us, &tdata_read2_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
#else
      return NC_EPARINIT;
#endif
   }
   else
   {
      tmeta_read_us = meta_read_us;
      tmeta_write_us = meta_write_us;
      tdata_read_us = data_read_us;
      tdata_write_us = data_write_us;
      tmeta_read2_us = meta_read2_us;
      tdata_read2_us = data_read2_us;
   }

   if (verbose)
      printf("num_bytes=%lld tdata_read_us=%d\n", num_bytes, tdata_read_us);

   read_rate = (float)num_bytes/((float)tdata_read_us/p);
   write_rate = (float)num_bytes/((float)tdata_write_us/p);
   reread_rate = (float)num_bytes/((float)tdata_read2_us/p);
   if (verbose)
      printf("%d: read rate %g, write rate %g, reread_rate %g\n", my_rank, read_rate, 
	     write_rate, reread_rate);

   /* Print some output. */
   if (!my_rank)
   {
      /* Does the user want a text header for the data? */
      if (header)
      {
	 printf("input format, output_format, input size, output size, meta read time, "
		"meta write time, data read time, data write time, enddianness, ");
	 if (doublecheck)
	    printf("metadata reread time, data reread time, read rate, "
		   "write rate, reread rate, ");
	 else
	    printf("read rate, write rate, ");
	 if (use_par)
	    printf("num_proc, ");
	 printf("deflate, shuffle, chunksize[0], chunksize[1], chunksize[2], "
		"chunksize[3]\n");
      }

      printf("%d, %d, %ld, %ld, %d, %d, %d, %d, %d, ", in_format, out_format, file_size(file_in), 
	     file_size(file_out), tmeta_read_us, tmeta_write_us, tdata_read_us, tdata_write_us,
	     endianness);
      if (doublecheck)
	 printf("%d, %d, %g, %g, %g, ", tmeta_read2_us, tdata_read2_us, read_rate, write_rate, 
		reread_rate);
      else
	 printf("%g, %g, ", read_rate, write_rate);
      if (use_par)
	 printf("%d, ", p);
      for (o1 = 0; o1 < num_vo; o1++)
      {
	 printf("%d, %d, %d, %d, %d, %d ", vo[o1].deflate_num, vo[o1].shuffle, 
	 (int)vo[o1].chunksize[0], (int)vo[o1].chunksize[1], (int)vo[o1].chunksize[2], (int)vo[o1].chunksize[3]);
	 if (o1 != num_vo - 1)
	    printf(", ");
      }
      printf("\n");
   }

#ifdef USE_PARALLEL
   MPI_Finalize();
#endif   

   return 0;
}
