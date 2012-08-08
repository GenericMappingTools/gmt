#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <string.h>
#include "mosaic_util.h"
#include "tool_util.h"
#include "constant.h"
#include "mpp_io.h"
#include "create_hgrid.h"
#define  D2R (M_PI/180.)
#define  R2D (180./M_PI)

/***********************************************************************
  void create_grid_from_file( char *file, int *nlon, int *nlat, double *x, double *y, double *dx,
                              double *dy, double *area, double *angle_dx )
   the grid location is defined through ascii file. calculate cell length,
   cell area and rotation angle  
************************************************************************/
void create_grid_from_file( char *file, int *nlon, int *nlat, double *x, double *y, double *dx, double *dy,
           		    double *area, double *angle_dx )
{
  double *xb, *yb, *xt, *yt, *xc, *yc;
  double p1[4], p2[4];
  int nx, ny, nxp, nyp, ni, nj, i, j, n;
  FILE *pFile;
  char mesg[256], txt[128];
  
  /************************************************************
     identify the grid_file is ascii file or netcdf file,
     if the file name contains ".nc", it is a netcdf file,
     otherwise it is ascii file.
  *********************************************************/
  nx  = *nlon;
  ny  = *nlat;
  nxp = nx + 1;
  nyp = ny + 1;
  if(strstr(file, ".nc") ) {
    int fid, vid;

    ni  = *nlon/2;
    nj  = *nlat/2;
    xc = (double *)malloc((ni+1)*(nj+1)*sizeof(double));
    yc = (double *)malloc((ni+1)*(nj+1)*sizeof(double));
    xt = (double *)malloc( ni   * nj   *sizeof(double));
    yt = (double *)malloc( ni   * nj   *sizeof(double));
    fid = mpp_open(file, MPP_READ);
    vid = mpp_get_varid(fid, "grid_lon");
    mpp_get_var_value(fid, vid, xc);
    vid = mpp_get_varid(fid, "grid_lat");
    mpp_get_var_value(fid, vid, yc);
    vid = mpp_get_varid(fid, "grid_lont");
    mpp_get_var_value(fid, vid, xt);
    vid = mpp_get_varid(fid, "grid_latt");
    mpp_get_var_value(fid, vid, yt);
    mpp_close(fid);
    for(j=0; j<nj+1; j++) for(i=0; i<ni+1; i++) {
      x[j*2*nxp+i*2] = xc[j*(ni+1)+i];
      y[j*2*nxp+i*2] = yc[j*(ni+1)+i];
    }
    for(j=0; j<nj; j++) for(i=0; i<ni; i++) {
      x[(j*2+1)*nxp+i*2+1] = xt[j*ni+i];
      y[(j*2+1)*nxp+i*2+1] = yt[j*ni+i];
    }
    for(j=0; j<nj+1; j++) for(i=0; i<ni; i++) {
      x[j*2*nxp+i*2+1] = (xc[j*(ni+1)+i]+xc[j*(ni+1)+i+1])*0.5;
      y[j*2*nxp+i*2+1] = (yc[j*(ni+1)+i]+yc[j*(ni+1)+i+1])*0.5;
    }    
    for(j=0; j<nj; j++) for(i=0; i<ni+1; i++) {
      x[(j*2+1)*nxp+i*2] = (xc[j*(ni+1)+i]+xc[(j+1)*(ni+1)+i])*0.5;
      y[(j*2+1)*nxp+i*2] = (yc[j*(ni+1)+i]+yc[(j+1)*(ni+1)+i])*0.5;
    }

    for(j=0; j<nyp; j++) for(i=0; i<nx; i++) {
      p1[0] = x[j*nxp+i]; p2[0] = x[j*nxp+i+1];
      p1[1] = y[j*nxp+i]; p2[1] = y[j*nxp+i+1];
      dx[j*nx+i] = great_circle_distance(p1, p2);
    }
    for(j=0; j<ny; j++) for(i=0; i<nxp; i++) {
      p1[0] = x[j*nxp+i]; p2[0] = x[(j+1)*nxp+i];
      p1[1] = y[j*nxp+i]; p2[1] = y[(j+1)*nxp+i];
      dy[j*nxp+i] = great_circle_distance(p1, p2);
    }    
    for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
      p1[0] = x[j*nxp+i]; p1[1] = x[j*nxp+i+1]; p1[2] = x[(j+1)*nxp+i+1]; p1[3] = x[(j+1)*nxp+i];
      p2[0] = y[j*nxp+i]; p2[1] = y[j*nxp+i+1]; p2[2] = y[(j+1)*nxp+i+1]; p2[3] = y[(j+1)*nxp+i];
      area[j*nx+i] = poly_area(p1, p2, 4);
    }
    /* currently set angle to 0 */
    for(j=0; j<nyp; j++) for(i=0; i<nxp; i++) angle_dx[j*nxp+i] = 0;    
    free(xt);
    free(yt);
    free(xc);
    free(yc);
  }
  else {
    pFile = fopen (file,"r");
    if(!pFile) {
      strcpy(mesg, "RegularSphericalGrid: Can not open ascii file ");
      strcat(mesg,file);
      mpp_error(mesg);
    }

    fscanf(pFile, "%s%*[^\n]",txt); /* header line (ignored) */
    xb = (double *) malloc(nxp*sizeof(double));
    yb = (double *) malloc(nyp*sizeof(double));
    for(i=0;i<nxp;i++) 
      fscanf(pFile, "%lg", xb+i); /* longitude */ 
    fscanf(pFile, "%s%*[^\n]",txt); /* header line (ignored) */   
    for(j=0;j<nyp;j++) fscanf(pFile, "%lg", yb+j); /* latitude */
    
    fclose(pFile);
    n=0;
    for(j=0; j<nyp; j++) {
      for(i=0; i<nxp; i++) {
	x[n]       = xb[i];
	y[n]       = yb[j];
	angle_dx[n++] = 0;   
      }
    }
    /* zonal length */
    n = 0;
    for(j=0; j<nyp; j++) {
      for(i=0; i<nx; i++ ) {
	dx[n++] = spherical_dist(x[j*nxp+i], y[j*nxp+i], x[j*nxp+i+1], y[j*nxp+i+1] );
      }
    }

    /* meridinal length */
    n = 0;
    for(j=0; j<ny; j++) {
      for(i=0; i<nxp; i++ ) {
	dy[n++] = spherical_dist(x[j*nxp+i], y[j*nxp+i], x[(j+1)*nxp+i], y[(j+1)*nxp+i] );
      }
    }

    /* cell area */
    n = 0;
    for(j=0; j<ny; j++) {
      for(i=0; i<nx; i++ ) {
	area[n++] = box_area(x[j*nxp+i]*D2R, y[j*nxp+i]*D2R, x[(j+1)*nxp+i+1]*D2R, y[(j+1)*nxp+i+1]*D2R );
      }
    }

    free(xb);
    free(yb);
  }
}; /* create_grid_from_file */




