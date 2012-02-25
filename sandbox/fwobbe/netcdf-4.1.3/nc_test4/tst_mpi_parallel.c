/* This is a quickie tester for netcdf-4. 
   
   This just excersizes MPI file I/O to make sure everything's working
   properly. If this doesn't work, netcdf/HDF5 parallel I/O also won't
   work.

   $Id$
*/

#include <nc_tests.h>
#include <mpi.h>

#define FILE "tst_mpi_parallel.bin"

int
main(int argc, char **argv)
{
   /* MPI stuff. */
   MPI_File fh;
   int my_rank, mpi_size;
   int data_in;
   MPI_Status status;

   /* Initialize MPI. */
   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   /*MPI_Get_processor_name(mpi_name, &mpi_namelen);*/
   /*printf("mpi_name: %s size: %d rank: %d\n", mpi_name, 
     mpi_size, my_rank);*/

   if (my_rank == 0)
   {
      printf("\n*** Testing basic MPI file I/O.\n");
      printf("*** testing file create with parallel I/O with MPI...");
   }

   if (MPI_File_open(MPI_COMM_WORLD, FILE, MPI_MODE_RDWR | MPI_MODE_CREATE,
                     MPI_INFO_NULL, &fh) != MPI_SUCCESS) ERR;
   if (MPI_File_seek(fh, my_rank * sizeof(int), MPI_SEEK_SET) != MPI_SUCCESS) ERR;
   if (MPI_File_write(fh, &my_rank, 1, MPI_INT, &status) != MPI_SUCCESS) ERR;
   if (MPI_File_close(&fh) != MPI_SUCCESS) ERR;

   /* Reopen and check the file. */
   if (MPI_File_open(MPI_COMM_WORLD, FILE, MPI_MODE_RDONLY,
                     MPI_INFO_NULL, &fh) != MPI_SUCCESS) ERR;
   if (MPI_File_seek(fh, my_rank * sizeof(int), MPI_SEEK_SET) != MPI_SUCCESS) ERR;
   if (MPI_File_read(fh, &data_in, 1, MPI_INT, &status) != MPI_SUCCESS) ERR;
   if (data_in != my_rank) ERR;
   if (MPI_File_close(&fh) != MPI_SUCCESS) ERR;

   /* Shut down MPI. */
   MPI_Finalize();

   if (my_rank == 0)
   {
      SUMMARIZE_ERR;
      FINAL_RESULTS;
   }
   return 0;
}
