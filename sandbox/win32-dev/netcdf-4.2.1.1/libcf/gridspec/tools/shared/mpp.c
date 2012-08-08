#include <stdlib.h>
#include <stdio.h>
#ifdef use_libMPI
#include <mpi.h>
#endif
#include "mpp.h"


/****************************************************
         global variables
*****************************************************/
int npes, root_pe, pe;
int *pelist=NULL;
const int tag = 1;
#ifdef use_libMPI  
MPI_Request *request;
#endif

/**************************************************************
                     void mpp_init()
     this routine will create communicator.
***************************************************************/

void mpp_init(int *argc, char ***argv)
{
  int n;
  
#ifdef use_libMPI
  MPI_Init(argc, argv); 
  MPI_Comm_rank(MPI_COMM_WORLD,&pe);
  MPI_Comm_size(MPI_COMM_WORLD,&npes);
  request = (MPI_Request *)malloc(npes*sizeof(MPI_Request));
  for(n=0; n<npes; n++) request[n] = MPI_REQUEST_NULL;
#else
  pe = 0;
  npes = 1;
#endif
  pelist = (int *)malloc(npes*sizeof(int));
  for(n=0; n<npes; n++) pelist[n] = n;
  root_pe = 0;
}; /* mpp_init */

/***********************************************************
               void mpp_end()
     This routine will terminate the parallel.
************************************************************/

void mpp_end()
{
#ifdef use_libMPI   
  MPI_Finalize();
#endif  
} /* mpp_end */

/*****************************************************************
           int mpp_pe()
      Returns processor ID.
******************************************************************/

int mpp_pe()
{
  return pe;
}; /* mpp_pe */


/**************************************************************
                 int mpp_npes()
      Returns processor count for current pelist.
**************************************************************/

int mpp_npes()
{
  return npes;
}; /* mpp_npes */

/*************************************************************
               int mpp_root_pe()
    return root processor of current pelist
*************************************************************/

int mpp_root_pe()
{
  return root_pe;
}; /* mpp_root_pe */

/*************************************************************
               int* mpp_get_pelist()
    return current pelist
*************************************************************/

int* mpp_get_pelist()
{
  return pelist;
}; /* mpp_get_pelist */

/************************************************************
    void mpp_sync_self()
 this is to check if current PE's outstanding puts are complete
*************************************************************/
void mpp_sync_self() {
  int n;
#ifdef use_libMPI     
  MPI_Status status;
  
  for(n=0; n<npes; n++) {
    if(request[n] != MPI_REQUEST_NULL) MPI_Wait( request+n, &status );
  }
#endif
  
}

/*************************************************************
    void mpp_send_double(const double* data, int size, int to_pe)
      send data to "to_pe"
*************************************************************/

void mpp_send_double(const double* data, int size, int to_pe)
{
#ifdef use_libMPI    
  MPI_Status status;
  /* make sure only one message from pe->to_pe in queue */
  if(request[to_pe] != MPI_REQUEST_NULL) {
    MPI_Wait( request+to_pe, &status );
  }
    
  MPI_Isend(data, size, MPI_DOUBLE, to_pe, tag, MPI_COMM_WORLD, request+to_pe);
#endif
  
}; /* mpp_send_double */


/*************************************************************
    void mpp_send_int(const int* data, int size, int to_pe)
      send data to "to_pe"
*************************************************************/

void mpp_send_int(const int* data, int size, int to_pe)
{
#ifdef use_libMPI    
  MPI_Status status;
  if(request[to_pe] != MPI_REQUEST_NULL) {
    MPI_Wait( request+to_pe, &status );
  }  

  MPI_Isend(data, size, MPI_INT, to_pe, tag, MPI_COMM_WORLD, request+to_pe);
#endif
  
}; /* mpp_send_int */

/***********************************************************
    void mpp_recv_double(double* data, int size, int from_pe)
     receive data from "from_pe"
***********************************************************/

void mpp_recv_double(double* data, int size, int from_pe)
{
#ifdef use_libMPI      
  MPI_Status status;
  MPI_Recv(data, size, MPI_DOUBLE, from_pe, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
#endif  
}; /* mpp_recv_double */

/***********************************************************
    void mpp_recv_int(int* data, int size, int from_pe)
     receive data from "from_pe"
***********************************************************/

void mpp_recv_int(int* data, int size, int from_pe)
{
#ifdef use_libMPI      
  MPI_Status status;
  MPI_Recv(data, size, MPI_INT, from_pe, MPI_ANY_TAG, MPI_COMM_WORLD,&status);
#endif  
}; /* mpp_recv_int */


/*******************************************************************************
  int mpp_sum_int(int count, int *data)
  sum integer over all the pes.
*******************************************************************************/
void mpp_sum_int(int count, int *data)
{

#ifdef use_libMPI
  int i;
  int *sum;
  sum = (int *)malloc(count*sizeof(int));
  MPI_Allreduce(data, sum, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  for(i=0; i<count; i++)data[i] = sum[i];
  free(sum);
#endif


}; /* mpp_sum_int */

/*******************************************************************************
  int mpp_sum_double(int count, double *data)
  sum double over all the pes.
*******************************************************************************/
void mpp_sum_double(int count, double *data)
{

#ifdef use_libMPI
  int i;
  double *sum;
  sum = (double *)malloc(count*sizeof(double));
  MPI_Allreduce(data, sum, count, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  for(i=0; i<count; i++)data[i] = sum[i];
  free(sum);  
#endif


}; /* mpp_sum_double */


/***********************************************************
    void mpp_error(char *str)
    error handler: will print out error message and then abort
***********************************************************/

void mpp_error(char *str)
{
  fprintf(stderr, "Error from pe %d: %s\n", pe, str );
#ifdef use_libMPI      
  MPI_Abort(MPI_COMM_WORLD, -1);
#else
  exit(1);
#endif  
}; /* mpp_error */

