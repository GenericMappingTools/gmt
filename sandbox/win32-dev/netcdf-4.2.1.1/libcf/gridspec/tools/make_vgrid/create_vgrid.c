#include <stdlib.h>
#include "create_vgrid.h"
#include "tool_util.h"

/***********************************************************************
   void create_vgrid(int nbnds, double *bnds, int *nz, char center, double *zeta)
   This routine is used to create vertical grid. The created grid is on super grid.
   the refinement is assumed to be 2.

   INPUT:
      nbnds:  number of vertical regions for varying resolution.      
      bnds:   boundaries for defining vertical regions of varying resolution.
              The size of bnds is nz.
      nz:     Number of model grid points for each vertical regions of varying resolution.
              The size of nz will be nz-1
      center: grid cell center location. Its value can be 'N', 'T' or 'C' with default
              value 'N'. When the value is 'N', supergrid location will be calculated.
              When the value is 'T', model grid corner location will be calculated,
              other grid location of the supergrid will be derived through T-cell
              centered condition. When the value is 'C', model grid corner location will
              be calculated, other grid location of the supergrid will be derived through
              C-cell centered condition.

   OUTPUT:
      zeta:   created vertical grid location.

***********************************************************************/

void create_vgrid(int nbnds, double *bnds, int *nz, double *zeta, const char *center)
{
  int i, np;
  double *zb=NULL;

  
  zb = compute_grid_bound(nbnds, bnds, nz, &np, center);
  np++;
  
  for(i=0; i<np; i++) zeta[i] =zb[i];

  free(zb);
    
  
}; /* create_vgrid */
