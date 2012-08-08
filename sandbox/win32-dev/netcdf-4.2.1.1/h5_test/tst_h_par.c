/* This is part of the netCDF package.
   Copyright 20057 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Test HDF5 file code. These are not intended to be exhaustive tests,
   but they use HDF5 the same way that netCDF-4 does, so if these
   tests don't work, than netCDF-4 won't work either.

   This files tests parallel I/O.

   $Id$
*/
#include <nc_tests.h>
#include <hdf5.h>

/* Defining USE_MPE causes the MPE trace library to be used (and you
 * must also relink with -llmpe -lmpe). This causes clog2 output to be
 * written, which can be converted to slog2 (by the program
 * clog2TOslog2) and then used in the analysis program jumpshot. */
/*#define USE_MPE 1*/

#ifdef USE_MPE
#include <mpe.h>
#endif /* USE_MPE */

#define FILE_NAME "tst_h_par.h5"
#define VAR_NAME "HALs_memory"
#define NDIMS 1
#define MILLION 1000000
#define DIM2_LEN 16000000
#define SC1 100000 /* slice count. */

int
main(int argc, char **argv)
{
   int p, my_rank;

#ifdef USE_MPE
   int s_init, e_init, s_define, e_define, s_write, e_write, s_close, e_close;
#endif /* USE_MPE */

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &p);

#ifdef USE_MPE
   MPE_Init_log();
   s_init = MPE_Log_get_event_number(); 
   e_init = MPE_Log_get_event_number(); 
   s_define = MPE_Log_get_event_number(); 
   e_define = MPE_Log_get_event_number(); 
   s_write = MPE_Log_get_event_number(); 
   e_write = MPE_Log_get_event_number(); 
   s_close = MPE_Log_get_event_number(); 
   e_close = MPE_Log_get_event_number(); 
   MPE_Describe_state(s_init, e_init, "Init", "red");
   MPE_Describe_state(s_define, e_define, "Define", "yellow");
   MPE_Describe_state(s_write, e_write, "Write", "green");
   MPE_Describe_state(s_close, e_close, "Close", "purple");
   MPE_Start_log();
   MPE_Log_event(s_init, 0, "start init");
#endif /* USE_MPE */

   if (!my_rank)
      printf("*** Creating file for parallel I/O read, and rereading it...");
   {
      hid_t fapl_id, fileid, whole_spaceid, dsid, slice_spaceid, whole_spaceid1, xferid;
      hsize_t start[NDIMS], count[NDIMS];
      hsize_t dims[1];
      int data[SC1], data_in[SC1];
      int num_steps;
      double ftime;
      int write_us, read_us;
      int max_write_us, max_read_us;
      float write_rate, read_rate;
      int i, s;

      /* We will write the same slice of random data over and over to
       * fill the file. */
      for (i = 0; i < SC1; i++)
	 data[i] = rand();
      
#ifdef USE_MPE
      MPE_Log_event(e_init, 0, "end init");
      MPE_Log_event(s_define, 0, "start define file");
#endif /* USE_MPE */

      /* Create file. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL) < 0) ERR;
      if ((fileid = H5Fcreate(FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, 
			      fapl_id)) < 0) ERR;

      /* Create a space to deal with one slice in memory. */
      dims[0] = SC1;
      if ((slice_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

      /* Create a space to write all slices. */
      dims[0] = DIM2_LEN;
      if ((whole_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

      /* Create dataset. */
      if ((dsid = H5Dcreate1(fileid, VAR_NAME, H5T_NATIVE_INT, 
      whole_spaceid, H5P_DEFAULT)) < 0) ERR;

      /* Use collective write operations. */
      if ((xferid = H5Pcreate(H5P_DATASET_XFER)) < 0) ERR;
      if (H5Pset_dxpl_mpio(xferid, H5FD_MPIO_COLLECTIVE) < 0) ERR;

#ifdef USE_MPE
      MPE_Log_event(e_define, 0, "end define file");
      if (my_rank)
	 sleep(my_rank);
#endif /* USE_MPE */

      /* Write the data in num_step steps. */
      ftime = MPI_Wtime();      
      num_steps = (DIM2_LEN/SC1) / p;
      for (s = 0; s < num_steps; s++)
      {
#ifdef USE_MPE
	 MPE_Log_event(s_write, 0, "start write slab");
#endif /* USE_MPE */

	 /* Select hyperslab for write of one slice. */
	 start[0] = s * SC1 * p + my_rank * SC1;
	 count[0] = SC1;
	 if (H5Sselect_hyperslab(whole_spaceid, H5S_SELECT_SET, 
	 start, NULL, count, NULL) < 0) ERR;
	 
	 if (H5Dwrite(dsid, H5T_NATIVE_INT, slice_spaceid, whole_spaceid, 
	 xferid, data) < 0) ERR;

#ifdef USE_MPE
	 MPE_Log_event(e_write, 0, "end write file");
#endif /* USE_MPE */
      }
      write_us = (MPI_Wtime() - ftime) * MILLION;
      MPI_Reduce(&write_us, &max_write_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!my_rank)
      {
	 write_rate = (float)(DIM2_LEN * sizeof(int))/(float)max_write_us;      
	 printf("\np=%d, write_rate=%g", p, write_rate);
      }
      
#ifdef USE_MPE
      MPE_Log_event(s_close, 0, "start close file");
#endif /* USE_MPE */

      /* Close. These collective operations will allow every process
       * to catch up. */
      if (H5Dclose(dsid) < 0 ||
      H5Sclose(whole_spaceid) < 0 ||
      H5Sclose(slice_spaceid) < 0 ||
      H5Pclose(fapl_id) < 0 ||
      H5Fclose(fileid) < 0)
	 ERR;

#ifdef USE_MPE
      MPE_Log_event(e_close, 0, "end close file");
#endif /* USE_MPE */

      /* Open the file. */
      if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) ERR;
      if (H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL) < 0) ERR;


      if (H5Pset_libver_bounds(fapl_id, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0) ERR;
      if ((fileid = H5Fopen(FILE_NAME, H5F_ACC_RDONLY, fapl_id)) < 0) ERR;

      /* Create a space to deal with one slice in memory. */
      dims[0] = SC1;
      if ((slice_spaceid = H5Screate_simple(NDIMS, dims, NULL)) < 0) ERR;

      /* Open the dataset. */
      if ((dsid = H5Dopen(fileid, VAR_NAME)) < 0) ERR;
      if ((whole_spaceid1 = H5Dget_space(dsid)) < 0) ERR;

      ftime = MPI_Wtime();      
      
      /* Read the data, a slice at a time. */
      for (s = 0; s < num_steps; s++)
      {
	 /* Select hyperslab for read of one slice. */
	 start[0] = s * SC1 * p + my_rank * SC1;
	 count[0] = SC1;
	 if (H5Sselect_hyperslab(whole_spaceid1, H5S_SELECT_SET, 
	 start, NULL, count, NULL) < 0) 
	 {
	    ERR;
	    return 2;
	 }

	 if (H5Dread(dsid, H5T_NATIVE_INT, slice_spaceid, whole_spaceid1, 
	 H5P_DEFAULT, data_in) < 0) 
	 {
	    ERR;
	    return 2;
	 }

/* 	 /\* Check the slice of data. *\/ */
/* 	 for (i = 0; i < SC1; i++) */
/* 	    if (data[i] != data_in[i])  */
/* 	    { */
/* 	       ERR; */
/* 	       return 2; */
/* 	    } */
      }
      read_us = (MPI_Wtime() - ftime) * MILLION;
      MPI_Reduce(&read_us, &max_read_us, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!my_rank)
      {
	 read_rate = (float)(DIM2_LEN * sizeof(int))/(float)max_read_us;      
	 printf(", read_rate=%g\n", read_rate);
      }
      
      /* Close down. */
      if (H5Dclose(dsid) < 0 ||
      H5Sclose(slice_spaceid) < 0 ||
      H5Sclose(whole_spaceid1) < 0 ||
      H5Pclose(fapl_id) < 0 ||
      H5Fclose(fileid) < 0)
	 ERR;
   }
   if (!my_rank)
      SUMMARIZE_ERR;

   MPI_Finalize();

   if (!my_rank)
      FINAL_RESULTS;
   return 0;
}
