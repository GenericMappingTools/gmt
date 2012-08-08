
/*********************************************************************
                    mpp.h
  This header contains subroutine for parallel programming.
  only MPI parallel is implemented. 
  Contact: Zhi.Liang@noaa.gov
 ********************************************************************/
#ifndef MPP_H_
#define MPP_H_

void mpp_init(int *argc, char ***argv);          /* start parallel programming, create communicator */
void mpp_end();           /* end of parallel programming, abort the program */
int mpp_pe();      /* return processor ID */
int mpp_root_pe(); /* return root pe of current pelist */
int mpp_npes();    /* return number of processor used */
int* mpp_get_pelist();
void mpp_send_double(const double* data, int size, int to_pe); /* send data */
void mpp_send_int(const int* data, int size, int to_pe); /* send data */
void mpp_recv_double(double* data, int size, int from_pe); /* recv data */
void mpp_recv_int(int* data, int size, int from_pe); /* recv data */
void mpp_error(char *str);
void mpp_sum_int(int count, int *data);
void mpp_sync_self();
#endif
