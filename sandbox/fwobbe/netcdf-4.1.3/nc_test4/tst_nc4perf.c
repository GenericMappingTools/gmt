/* 
Copyright 2009, UCAR/Unidata
See COPYRIGHT file for copying and redistribution conditions.

This program tests netcdf-4 parallel I/O. These tests are based on the
needs of the NASA GMAO model, and are based on some test code from
Dennis Nadeau.

$Id$
*/

#include "nc_tests.h"

#define FILENAME "tst_nc4perf.nc"
#define NDIMS1 2
#define NDIMS2 4
#define DIMSIZE1 40
#define DIMSIZE2 61
#define DIMSIZE3 3
/*#define DIMSIZE1 540
#define DIMSIZE2 361
#define DIMSIZE3 72*/
#define TIMELEN 4
#define NUMVARS 10
#define NUM_TRIES 2
#define MEGABYTE 1048576

/* This function creates a file with 10 2D variables, no unlimited
 * dimension. */
int test_pio_2d(size_t cache_size, int facc_type, int access_flag, MPI_Comm comm, 
		MPI_Info info, int mpi_size, int mpi_rank, 
		size_t *chunk_size) 
{
   double starttime, endtime, write_time = 0, bandwidth = 0;
   int ncid;
   int dimids[NDIMS1];
   size_t start[NDIMS1], count[NDIMS1];
   float *data;
   char file_name[NC_MAX_NAME + 1];
   char var_name1[NUMVARS][NC_MAX_NAME + 1] = {"GWa", "JAd", "TJe", "JMa", "JMo", 
					       "JQA", "AJa", "MVB", "WHH", "JTy"};
   int varid1[NUMVARS];
   size_t nelems_in;
   float preemption_in;
   int j, i, t;

   /* Create some data. */
   if (!(data = malloc(sizeof(float) * DIMSIZE2 * DIMSIZE1 / mpi_size)))
      return -2;
   for (j = 0; j < DIMSIZE2; j++)
      for (i = 0; i < DIMSIZE1 / mpi_size; i++)
	 data[j * DIMSIZE1 / mpi_size + i] = (float)mpi_rank * (j + 1);

   /* Get the file name. */
   sprintf(file_name, "%s/%s", TEMP_LARGE, FILENAME);

   /* Set the cache size. */
   if (nc_get_chunk_cache(NULL, &nelems_in, &preemption_in)) ERR;
   if (nc_set_chunk_cache(cache_size, nelems_in, preemption_in)) ERR;

   for (t = 0; t < NUM_TRIES; t++)
   {
      /* Create a netcdf-4 file, opened for parallel I/O. */
      if (nc_create_par(file_name, facc_type|NC_NETCDF4, comm, 
			info, &ncid)) ERR;

      /* Create two dimensions. */
      if (nc_def_dim(ncid, "d1", DIMSIZE2, &dimids[0])) ERR;
      if (nc_def_dim(ncid, "d2", DIMSIZE1, &dimids[1])) ERR;

      /* Create our variables. */
      for (i = 0; i < NUMVARS; i++)
      {
	 if (nc_def_var(ncid, var_name1[i], NC_INT, NDIMS1,
			dimids, &varid1[i])) ERR;
	 if (chunk_size[0])
	    if (nc_def_var_chunking(ncid, varid1[i], 0, chunk_size)) ERR;
      }

      if (nc_enddef(ncid)) ERR;

      /* Set up slab for this process. */
      start[0] = 0;
      start[1] = mpi_rank * DIMSIZE1/mpi_size;
      count[0] = DIMSIZE2;
      count[1] = DIMSIZE1 / mpi_size;

      /* start parallel netcdf4 */
      for (i = 0; i < NUMVARS; i++) 
	 if (nc_var_par_access(ncid, varid1[i], access_flag)) ERR;

      starttime = MPI_Wtime();

      /* Write two dimensional float data */
      for (i = 0; i < NUMVARS; i++)
	 if (nc_put_vara_float(ncid, varid1[i], start, count, data)) ERR;

      /* Close the netcdf file. */
      if (nc_close(ncid)) ERR;

      endtime = MPI_Wtime();
      if (!mpi_rank) 
      {
	 bandwidth += ((sizeof(float) * DIMSIZE1 * DIMSIZE2 * NUMVARS) / 
		       ((endtime - starttime) * 1024 * 1024)) / NUM_TRIES;
	 write_time += (endtime - starttime) / NUM_TRIES;
      }
   }
   free(data);
   if (!mpi_rank) 
   {
      char chunk_string[NC_MAX_NAME + 1] = "";

      /* What was our chunking? */
      if (chunk_size[0])
	 sprintf(chunk_string, "%dx%d    ", (int)chunk_size[0], (int)chunk_size[1]);
      else
	 strcat(chunk_string, "contiguous");

      /* Print the results. */
      printf("%d\t\t%s\t%s\t%d\t\t%dx%d\t\t%s\t%f\t\t%f\t\t\t%d\n", mpi_size, 
	     (facc_type == NC_MPIIO ? "MPI-IO   " : "MPI-POSIX"), 
	     (access_flag == NC_INDEPENDENT ? "independent" : "collective"), 
	     (int)cache_size/MEGABYTE, DIMSIZE1, DIMSIZE2, chunk_string, 
	     write_time, bandwidth, NUM_TRIES); 
   }

   /* Delete this file. */
   remove(file_name); 

   return 0;
}

/* Both read and write will be tested */
/* Case 2: create four dimensional integer data,
   one dimension is unlimited. */
int test_pio_4d(size_t cache_size, int facc_type, int access_flag, MPI_Comm comm, 
		MPI_Info info, int mpi_size, int mpi_rank, size_t *chunk_size) 
{
   int ncid, dimuids[NDIMS2], varid2[NUMVARS];
   size_t ustart[NDIMS2], ucount[NDIMS2];
   float *udata, *tempudata;
   char file_name[NC_MAX_NAME + 1];
   char var_name2[NUMVARS][NC_MAX_NAME + 1] = {"JKP", "ZTa", "MFi", "FPi", "JBu", 
					       "ALi", "AJo", "USG", "RBH", "JAG"};
   double starttime, endtime, write_time = 0, bandwidth = 0;
   size_t nelems_in;
   float preemption_in;
   int k, j, i, t;

   udata = malloc(DIMSIZE3 * DIMSIZE2 * DIMSIZE1 / mpi_size * sizeof(int));

   /* Create phony data. */
   tempudata = udata;
   for(k = 0; k < DIMSIZE3; k++)
      for(j = 0; j < DIMSIZE2; j++)
	 for(i = 0; i < DIMSIZE1 / mpi_size; i++)
	 {
	    *tempudata = (float)(1 + mpi_rank) * 2 * (j + 1) * (k + 1);
	    tempudata++;
	 }

   /* Get the file name. */
   sprintf(file_name, "%s/%s", TEMP_LARGE, FILENAME);
      
   /* Set the cache size. */
   if (nc_get_chunk_cache(NULL, &nelems_in, &preemption_in)) ERR;
   if (nc_set_chunk_cache(cache_size, nelems_in, preemption_in)) ERR;

   for (t = 0; t < NUM_TRIES; t++)
   {
      /* Create a netcdf-4 file. */
      if (nc_create_par(file_name, facc_type|NC_NETCDF4, comm, info, 
			&ncid)) ERR;

      /* Create four dimensions. */
      if (nc_def_dim(ncid, "ud1", TIMELEN, dimuids)) ERR;
      if (nc_def_dim(ncid, "ud2", DIMSIZE3, &dimuids[1])) ERR;
      if (nc_def_dim(ncid, "ud3", DIMSIZE2, &dimuids[2])) ERR;
      if (nc_def_dim(ncid, "ud4", DIMSIZE1, &dimuids[3])) ERR;

      /* Create 10 variables. */
      for (i = 0; i < NUMVARS; i++)
	 if (nc_def_var(ncid, var_name2[i], NC_INT, NDIMS2,
			dimuids, &varid2[i])) ERR;

      if (nc_enddef(ncid)) ERR;
 
      /* Set up selection parameters */
      ustart[0] = 0;
      ustart[1] = 0;
      ustart[2] = 0;
      ustart[3] = DIMSIZE1 * mpi_rank / mpi_size;
      ucount[0] = 1;
      ucount[1] = DIMSIZE3;
      ucount[2] = DIMSIZE2;
      ucount[3] = DIMSIZE1 / mpi_size;

      /* Access parallel */
      for (i = 0; i < NUMVARS; i++)
	 if (nc_var_par_access(ncid, varid2[i], access_flag)) ERR;

      starttime = MPI_Wtime();

      /* Write slabs of phony data. */
      for(ustart[0] = 0; ustart[0] < TIMELEN; ustart[0]++)
	 for (i = 0; i < NUMVARS; i++)
	    if (nc_put_vara_float(ncid, varid2[i], ustart, ucount, udata)) ERR;

      /* Close the netcdf file. */
      if (nc_close(ncid)) ERR;

      endtime = MPI_Wtime();
      if (!mpi_rank)
      {
	 write_time += (endtime - starttime) / NUM_TRIES;
	 bandwidth += (sizeof(float) * TIMELEN * DIMSIZE1 * DIMSIZE2 * DIMSIZE3 * NUMVARS) / 
	    ((endtime - starttime) * 1024 * 1024 * NUM_TRIES);
      }
   }
   free(udata);
   if (!mpi_rank)
   {
      char chunk_string[NC_MAX_NAME + 1] = "";

      /* What was our chunking? */
      if (chunk_size[0])
	 sprintf(chunk_string, "%dx%dx%dx%d", (int)chunk_size[0], (int)chunk_size[1], 
		 (int)chunk_size[2], (int)chunk_size[3]);
      else
	 strcat(chunk_string, "contiguous");

      /* Print our results. */
      printf("%d\t\t%s\t%s\t%d\t\t%dx%dx%dx%d\t%s\t%f\t\t%f\t\t\t%d\n", mpi_size, 
	     (facc_type == NC_MPIIO ? "MPI-IO   " : "MPI-POSIX"), 
	     (access_flag == NC_INDEPENDENT ? "independent" : "collective"), 
	     (int)cache_size / MEGABYTE, TIMELEN, DIMSIZE3, DIMSIZE2, DIMSIZE1, chunk_string, write_time, 
	     bandwidth, NUM_TRIES); 
   }

   /* Delete this file. */
   remove(file_name); 

   return 0;
}

#define NUM_MODES 2
#define NUM_FACC 2
#define NUM_CHUNK_COMBOS_2D 3
#define NUM_CHUNK_COMBOS_4D 4
#define NUM_CACHE_SIZES 3

int main(int argc, char **argv)
{
   MPI_Comm comm = MPI_COMM_WORLD;
   MPI_Info info = MPI_INFO_NULL;
   int mpi_size, mpi_rank;
   int mpi_mode[NUM_MODES] = {NC_MPIIO, NC_MPIPOSIX};
   int facc_type[NUM_FACC] = {NC_INDEPENDENT, NC_COLLECTIVE};
   size_t chunk_size_2d[NUM_CHUNK_COMBOS_2D][NDIMS1] = {{0, 0}, 
							{DIMSIZE2, DIMSIZE1},
							{DIMSIZE2/2 + 1, DIMSIZE1 / 2}};
   size_t chunk_size_4d[NUM_CHUNK_COMBOS_4D][NDIMS2] = {{0, 0, 0, 0}, 
							{1, DIMSIZE3, DIMSIZE2, DIMSIZE1}, 
							{TIMELEN / 2, DIMSIZE3 / 2 + 1, DIMSIZE2 / 2 + 1, DIMSIZE1 / 2},
							{TIMELEN, DIMSIZE3, DIMSIZE2, DIMSIZE1}};
   size_t cache_size[NUM_CACHE_SIZES] = {MEGABYTE, 32 * MEGABYTE, 64 * MEGABYTE};
   int m, f, c, i;

   /* Initialize MPI. */
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

   /* Check for invalid number of processors. */
   if ((float)DIMSIZE1 / mpi_size != (int)(DIMSIZE1 / mpi_size))
   {
      printf("%d divided by number of processors must be a whole number!\n", 
	     DIMSIZE1);
      return -1;
   }

   if (!mpi_rank)
   {
      printf("*** Testing parallel IO for NASA...\n");
      printf("num_proc\tMPI mode\taccess\t\tcache (MB)\tgrid size\tchunks\tavg. write time(s)\t"
	     "avg. write bandwidth(MB/s)\tnum_tries\n");
   }

   for (i = 0; i < NUM_CACHE_SIZES; i++)
      for (m = 0; m < NUM_MODES; m++)
	 for (f = 0; f < NUM_FACC; f++)
	    for (c = 0; c < NUM_CHUNK_COMBOS_2D; c++)
	       if (test_pio_2d(cache_size[i], mpi_mode[m], facc_type[f], comm, 
			       info, mpi_size, mpi_rank, chunk_size_2d[c])) ERR;

   for (i = 0; i < NUM_CACHE_SIZES; i++)
      for (m = 0; m < NUM_MODES; m++)
	 for (f = 0; f < NUM_FACC; f++)
	    for (c = 0; c < NUM_CHUNK_COMBOS_4D; c++)
	       if (test_pio_4d(cache_size[i], mpi_mode[m], facc_type[f], comm, 
			       info, mpi_size, mpi_rank, chunk_size_4d[c])) ERR; 

   if (!mpi_rank) 
      SUMMARIZE_ERR;
   MPI_Finalize();

   if (!mpi_rank)
      FINAL_RESULTS;

   return 0;
}

