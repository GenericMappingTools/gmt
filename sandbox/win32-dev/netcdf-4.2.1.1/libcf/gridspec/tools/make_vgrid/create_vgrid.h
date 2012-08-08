/***********************************************************************
                       creater_vgrid.h
    This header file contains interface to create vertical grid on supergrid. 
    refinement =2 is assumed in this routine.
    contact: Zhi.Liang@noaa.gov
************************************************************************/

#ifndef CREATE_VGRID_H_
#define CREATE_VGRID_H_
void create_vgrid(int nbnds, double *bnds, int *nz, double *zeta, const char *center);
#endif
