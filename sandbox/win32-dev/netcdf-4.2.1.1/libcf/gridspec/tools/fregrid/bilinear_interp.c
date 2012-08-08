#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "globals.h"
#include "mosaic_util.h"
#include "bilinear_interp.h"
#include "mpp_io.h"

#define min(a,b) (a<b ? a:b)
#define max(a,b) (a>b ? a:b)
#define sign(a,b)(b<0 ? -fabs(a):fabs(a))


int max_weight_index( double *var, int nvar);
double normalize_great_circle_distance(const double *v1, const double *v2);
/*double spherical_angle(double *v1, double *v2, double *v3);*/
double dist2side(const double *v1, const double *v2, const double *point);
void redu2x(const double *varfin, const double *yfin, int nxfin, int nyfin, double *varcrs,
	    int nxcrs, int nycrs, int nz, int has_missing, double missvalue);
void do_latlon_coarsening(const double *var_latlon, const double *ylat, int nlon, int nlat, int nz,    
			  double *var_latlon_crs, int finer_steps, int has_missing, double missvalue);
void do_c2l_interp(const Interp_config *interp, int nx_in, int ny_in, int nz, const Field_config *field_in,
		   int nx_out, int ny_out, double *data_out, int has_missing, double missing, int fill_missing );
void sort_index(int ntiles, int *index, double *shortest);
int get_index(const Grid_config *grid_in, const Grid_config *grid_out, int *index,
	       int i_in, int j_in, int l_in, int i_out, int j_out);
/*******************************************************************************
  void setup_bilinear_interp( )
    !------------------------------------------------------------------!
    ! calculate weights for bilinear interpolation                     !
    ! from cubed sphere to latlon grid                                 !
    !                                                                  !
    ! input:                                                           !
    ! sph_corner      cubed sphere corner location in spherical coor   !
    ! npx, npy        number of corners per tile                       !
    ! ntiles          number of tiles                                  !
    ! xlon, ylat      latlon grid coor                                 !
    ! nlon, nlat      latlon grid dimension                            !
    !                                                                  !
    ! output:                                                          !
    ! c2l_index       cubed sphere index for latlon interpolation      !
    ! c2l_weight      weights for cubsph_to_latlon interpolation       !
    ! elon_cubsph     lon unit vector for cubed sphere center          !
    ! elat_cubsph     lat unit vector for cubed sphere center          !
    ! elon_latlon     lon unit vector for latlon grid                  !
    ! elat_latlon     lat unit vector for latlon grid                  !
    !------------------------------------------------------------------!
*******************************************************************************/
void setup_bilinear_interp(int ntiles_in, const Grid_config *grid_in, int ntiles_out, const Grid_config *grid_out, 
                      Interp_config *interp, unsigned int opcode)
{
  const int max_iter = 10;
  double abs_center, dcub, dlon, dlat, coslat, distance;
  double dist1, dist2, dist3, dist4, sum;
  double *shortest;
  int    i, j, n, l, ic, jc, lc, icc, jcc, i_min, i_max, j_min, j_max, iter;
  int    n0, n1, n2, n3, n4, m0, m1;
  int    nx_in, ny_in, nx_out, ny_out, nxd, nyd;
  double v0[3], v1[3], v2[3], v3[3], v4[3];
  int    all_done;
  int    *found, *index;
  
  /* ntiles_in must be six and ntiles_out must be one */
  if(ntiles_in != 6) mpp_error("Error from bilinear_interp: source mosaic should be cubic mosaic "
			       "and have six tiles when using bilinear option");
  if(ntiles_out != 1) mpp_error("Error from bilinear_interp: destination mosaic should be "
				"one tile lat-lon grid when using bilinear option"); 
  /*-----------------------------------------------------------------!
    ! cubed sphere: cartesian coordinates of cell corners,             !
    !               cell lenghts between corners,                      !
    !               cartesian and spherical coordinates of cell centers! 
    !               calculate latlon unit vector                       !
    !-----------------------------------------------------------------*/

  /* calculation is done on the fine grid */
  nx_out   = grid_out->nx_fine;
  ny_out   = grid_out->ny_fine;
  nx_in    = grid_in->nx;       /* the cubic grid has same resolution on each face */
  ny_in    = grid_in->ny;
  nxd      = nx_in + 2;
  nyd      = ny_in + 2;

  interp->index  = (int    *)malloc(3*nx_out*ny_out*sizeof(int   ));
  interp->weight = (double *)malloc(4*nx_out*ny_out*sizeof(double));

  if( (opcode & READ) && interp->file_exist ) { /* reading from file */
    int nx2, ny2, fid, vid;

    /* check the size of the grid matching the size in remapping file */
    printf("NOTE: reading index and weight for bilinear interpolation from file.\n");
    fid = mpp_open(interp->remap_file, MPP_READ);
    nx2 = mpp_get_dimlen(fid, "nlon");
    ny2 = mpp_get_dimlen(fid, "nlat");
    printf("grid size is nx=%d, ny=%d, remap file size is nx=%d, ny=%d.\n", nx_out, ny_out, nx2, ny2);
    if(nx2 != nx_out || ny2 != ny_out ) mpp_error("bilinear_interp: size mismatch between grid size and remap file size");
    vid = mpp_get_varid(fid, "index");
    mpp_get_var_value(fid, vid, interp->index);
    vid = mpp_get_varid(fid, "weight");
    mpp_get_var_value(fid, vid, interp->weight);
    mpp_close(fid);
    return;
  }
  
  /*------------------------------------------------------------------
    find lower left corner on cubed sphere for given latlon location 
    ------------------------------------------------------------------*/
  found    = (int *)malloc(nx_out*ny_out*sizeof(int));
  index    = (int *)malloc(ntiles_in*3*sizeof(int));
  shortest = (double *)malloc(ntiles_in*sizeof(double));
  for(i=0; i<nx_out*ny_out; i++) found[i] = 0;
    
  dlon=(M_PI+M_PI)/nx_out;
  dlat=M_PI/(ny_out-1);

  for(iter=1; iter<=max_iter; iter++) {
    for(l=0; l<ntiles_in; l++) {
      for(jc=1; jc<=ny_in; jc++) for(ic=1; ic<=nx_in; ic++) {
	/*------------------------------------------------------
	  guess latlon indexes for given cubed sphere cell     
	  ------------------------------------------------------*/
	n1 = jc*nxd+ic;
	n2 = (jc+1)*nxd+ic+1;
	v1[0] = grid_in[l].xt[n1];
	v1[1] = grid_in[l].yt[n1];
	v1[2] = grid_in[l].zt[n1];
	v2[0] = grid_in[l].xt[n2];
	v2[1] = grid_in[l].yt[n2];
	v2[2] = grid_in[l].zt[n2];
	dcub=iter*normalize_great_circle_distance(v1, v2);
	j_min=max(   1,  floor((grid_in[l].latt[n1]-dcub+0.5*M_PI)/dlat)-iter+1);
	j_max=min(ny_out,ceil((grid_in[l].latt[n1]+dcub+0.5*M_PI)/dlat)+iter-1);
	if (j_min==1 || j_max==ny_out) {
	  i_min=1;
	  i_max=nx_out;
	}
        else {
	  i_min=max(   1,  floor((grid_in[l].lont[n1]-dcub)/dlon-iter+1));
	  i_max=min(nx_out,ceil((grid_in[l].lont[n1]+dcub)/dlon+iter-1));
	}
	for(j=j_min-1; j<j_max; j++) for(i=i_min-1; i<i_max; i++) {
	  n0 = j*nx_out + i;
	  /*--------------------------------------------------------------------
            for latlon cell find nearest cubed sphere cell 
            ------------------------------------------------------------------*/
	  if (!found[n0]) {
	    shortest[l]=M_PI+M_PI;
	    for(jcc=jc; jcc<=min(ny_in, jc+1); jcc++) for(icc=ic; icc<=min(nx_in, ic+1); icc++) {
	      n1 = jcc*nxd + icc;
	      n2 = j*nx_out + i;
	      v1[0] = grid_in[l].xt[n1];
	      v1[1] = grid_in[l].yt[n1];
	      v1[2] = grid_in[l].zt[n1];
	      v2[0] = grid_out->xt[n2];
	      v2[1] = grid_out->yt[n2];
	      v2[2] = grid_out->zt[n2];	      
	      distance=normalize_great_circle_distance(v1, v2);
	      if (distance < shortest[l]) {
		shortest[l]=distance;
		index[3*l]  =icc;
		index[3*l+1]=jcc;
		index[3*l+2]=l;
	      }
	    }
	    /*------------------------------------------------
              determine lower left corner                    
              ------------------------------------------------*/
	    found[n0] = get_closest_index(&(grid_in[l]), grid_out, &(interp->index[3*(j*nx_out+i)]), index[3*l],
			      index[3*l+1], index[3*l+2], i, j);
	  }
	}
      }
    }
    if (iter>1) {
      all_done = 1;
      for(i=0; i<nx_out*ny_out; i++) {
	if( !found[i]) {
	  all_done = 0;
	  break;
	}
      };
      if (all_done) break;
    }
  }
  
  /*------------------------------------------------------------------
    double check if lower left corner was found                      
    calculate weights for interpolation                              
    ------------------------------------------------------------------*/
  for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) {
    n0 = j*nx_out + i;
    m0 = 3*n0;
    m1 = 4*n0;
    if (!found[n0]) {
      printf("**************************************************************\n");
      printf("WARNING: didn't find lower left corner for (ilon,jlat) = (%d,%d)\n", i,j);
      printf("will perform expensive global sweep\n");
      printf("**************************************************************\n");
      /*---------------------------------------------------------
	for latlon cell find nearby cubed sphere cell           
	---------------------------------------------------------*/
      for(l=0; l<3*ntiles_in; l++) index[l] = 0;	
      for(l=0; l<ntiles_in; l++) shortest[l] = M_PI + M_PI;
      for(l=0; l<ntiles_in; l++) {
	for(jc=0; jc<ny_in; j++) for(ic=0; ic<nx_in; ic++) {
	  n1 = jc*nxd+ic;
	  v1[0] = grid_in[l].xt[n1];
	  v1[1] = grid_in[l].yt[n1];
	  v1[2] = grid_in[l].zt[n1];
	  v0[0] = grid_out[l].xt[n0];
	  v0[1] = grid_out[l].yt[n0];
	  v0[2] = grid_out[l].zt[n0];	    
	  distance=normalize_great_circle_distance(v1, v2);
	  if (distance<shortest[l]) {
	    shortest[l]=distance;
	    index[3*l  ]=ic;
	    index[3*l+1]=jc;
	    index[3*l+2]=l;
	  }
	}
      }

      /*---------------------------------------------------------
	determine lower left corner                             
	---------------------------------------------------------*/
      sort_index(ntiles_in, index, shortest);
      found[n0]=0;
      for(l=0; l<ntiles_in; l++) {
	if (!found[n0]) {
	  found[n0]=get_index(&(grid_in[l]), grid_out, &(interp->index[3*(j*nx_out+i)]), index[3*l],
			      index[3*l+1], index[3*l+2], i, j);
	  if (found[n0]) break;
	}
      }
      if (! found[n0] ) mpp_error("error from bilinear_interp: couldn't find lower left corner");
    }
    /*------------------------------------------------------------
      calculate shortest distance to each side of rectangle      
      formed by cubed sphere cell centers                        
      special corner treatment                                   
      ------------------------------------------------------------*/
    ic=interp->index[m0];
    jc=interp->index[m0+1];
    l =interp->index[m0+2];
    if (ic==nx_in && jc==ny_in) {
      /*------------------------------------------------------------
	calculate weights for bilinear interpolation near corner   
	------------------------------------------------------------*/
      n1 = jc*nxd+ic;
      n2 = jc*nxd+ic+1;
      n3 = (jc+1)*nxd+ic;
      v1[0] = grid_in[l].xt[n1];
      v1[1] = grid_in[l].yt[n1];
      v1[2] = grid_in[l].zt[n1];
      v2[0] = grid_in[l].xt[n2];
      v2[1] = grid_in[l].yt[n2];
      v2[2] = grid_in[l].zt[n2];
      v3[0] = grid_in[l].xt[n3];
      v3[1] = grid_in[l].yt[n3];
      v3[2] = grid_in[l].zt[n3];
      v0[0] = grid_out->xt[n0];
      v0[1] = grid_out->yt[n0];
      v0[2] = grid_out->zt[n0];	  
      dist1=dist2side(v2, v3, v0);
      dist2=dist2side(v2, v1, v0);
      dist3=dist2side(v1, v3, v0);
      interp->weight[m1]  =dist1;      /* ic,   jc    weight */
      interp->weight[m1+1]=dist2;      /* ic,   jc+1  weight */
      interp->weight[m1+2]=0.;         /* ic+1, jc+1  weight */
      interp->weight[m1+3]=dist3;      /* ic+1, jc    weight */
             
      sum=interp->weight[m1]+interp->weight[m1+1]+interp->weight[m1+3];
      interp->weight[m1]  /=sum;
      interp->weight[m1+1]/=sum;
      interp->weight[m1+3]/=sum;
    }
    else if (ic==0 && jc==ny_in) {
      /*------------------------------------------------------------
	calculate weights for bilinear interpolation near corner   
	------------------------------------------------------------*/
	
      n1 = jc*nxd+ic;
      n2 = jc*nxd+ic+1;
      n3 = (jc+1)*nxd+ic+1;
      v1[0] = grid_in[l].xt[n1];
      v1[1] = grid_in[l].yt[n1];
      v1[2] = grid_in[l].zt[n1];
      v2[0] = grid_in[l].xt[n2];
      v2[1] = grid_in[l].yt[n2];
      v2[2] = grid_in[l].zt[n2];
      v3[0] = grid_in[l].xt[n3];
      v3[1] = grid_in[l].yt[n3];
      v3[2] = grid_in[l].zt[n3];
      v0[0] = grid_out->xt[n0];
      v0[1] = grid_out->yt[n0];
      v0[2] = grid_out->zt[n0];	  
      dist1=dist2side(v3, v2, v0);
      dist2=dist2side(v2, v1, v0);
      dist3=dist2side(v3, v1, v0);             
      interp->weight[m1]  =dist1;      /* ic,   jc    weight */
      interp->weight[m1+1]=0.;         /* ic,   jc+1  weight */
      interp->weight[m1+2]=dist2;      /* ic+1, jc+1  weight */
      interp->weight[m1+3]=dist3;      /* ic+1, jc    weight */
             
      sum=interp->weight[m1]+interp->weight[m1+2]+interp->weight[m1+3];
      interp->weight[m1]  /=sum;
      interp->weight[m1+2]/=sum;
      interp->weight[m1+3]/=sum;
    }
    else if (jc==0 && ic==nx_in) {
      /*------------------------------------------------------------
	calculate weights for bilinear interpolation near corner   
	------------------------------------------------------------*/
      n1 = jc*nxd+ic;
      n2 = (jc+1)*nxd+ic;
      n3 = (jc+1)*nxd+ic+1;
      v1[0] = grid_in[l].xt[n1];
      v1[1] = grid_in[l].yt[n1];
      v1[2] = grid_in[l].zt[n1];
      v2[0] = grid_in[l].xt[n2];
      v2[1] = grid_in[l].yt[n2];
      v2[2] = grid_in[l].zt[n2];
      v3[0] = grid_in[l].xt[n3];
      v3[1] = grid_in[l].yt[n3];
      v3[2] = grid_in[l].zt[n3];
      v0[0] = grid_out->xt[n0];
      v0[1] = grid_out->yt[n0];
      v0[2] = grid_out->zt[n0];	  
      dist1=dist2side(v2, v3, v0);
      dist2=dist2side(v1, v3, v0);
      dist3=dist2side(v1, v2, v0);             	     
             
      interp->weight[m1]  =dist1;      /* ic,   jc    weight */
      interp->weight[m1+1]=dist2;      /* ic,   jc+1  weight */
      interp->weight[m1+2]=dist3;      /* ic+1, jc+1  weight */
      interp->weight[m1+3]=0.;         /* ic+1, jc    weight */
             
      sum=interp->weight[m1]+interp->weight[m1+1]+interp->weight[m1+2];
      interp->weight[m1]  /=sum;
      interp->weight[m1+1]/=sum;
      interp->weight[m1+2]/=sum;
    }
    else {
      /*------------------------------------------------------------
	calculate weights for bilinear interpolation if no corner  
	------------------------------------------------------------*/
      n1 = jc*nxd+ic;
      n2 = jc*nxd+ic+1;
      n3 = (jc+1)*nxd+ic;
      n4 = (jc+1)*nxd+ic+1;
      v1[0] = grid_in[l].xt[n1];
      v1[1] = grid_in[l].yt[n1];
      v1[2] = grid_in[l].zt[n1];
      v2[0] = grid_in[l].xt[n2];
      v2[1] = grid_in[l].yt[n2];
      v2[2] = grid_in[l].zt[n2];
      v3[0] = grid_in[l].xt[n3];
      v3[1] = grid_in[l].yt[n3];
      v3[2] = grid_in[l].zt[n3];
      v4[0] = grid_in[l].xt[n4];
      v4[1] = grid_in[l].yt[n4];
      v4[2] = grid_in[l].zt[n4];	
      v0[0] = grid_out->xt[n0];
      v0[1] = grid_out->yt[n0];
      v0[2] = grid_out->zt[n0];	  
      dist1=dist2side(v1, v3, v0);
      dist2=dist2side(v3, v4, v0);
      dist3=dist2side(v4, v2, v0);
      dist4=dist2side(v2, v1, v0);
	
      interp->weight[m1]  =dist2*dist3;      /* ic,   jc    weight */
      interp->weight[m1+1]=dist3*dist4;      /* ic,   jc+1  weight */
      interp->weight[m1+2]=dist4*dist1;      /* ic+1, jc+1  weight */
      interp->weight[m1+3]=dist1*dist2;      /* ic+1, jc    weight */
             
      sum=interp->weight[m1]+interp->weight[m1+1]+interp->weight[m1+2]+interp->weight[m1+3];
      interp->weight[m1]  /=sum;
      interp->weight[m1+1]/=sum;
      interp->weight[m1+2]/=sum;
      interp->weight[m1+3]/=sum;
    }
  }

  /* write out weight information if needed */
  if( opcode & WRITE ) {
    int fid, dim_three, dim_four, dim_nlon, dim_nlat, dims[3];
    int fld_index, fld_weight;
    
    fid = mpp_open( interp->remap_file, MPP_WRITE);
    dim_nlon = mpp_def_dim(fid, "nlon", nx_out);
    dim_nlat = mpp_def_dim(fid, "nlat", ny_out);
    dim_three = mpp_def_dim(fid, "three", 3);
    dim_four  = mpp_def_dim(fid, "four", 4);
    
    dims[0] = dim_three; dims[1] = dim_nlat; dims[2] = dim_nlon;
    fld_index = mpp_def_var(fid, "index", NC_INT, 3, dims, 0);
    dims[0] = dim_four; dims[1] = dim_nlat; dims[2] = dim_nlon;
    fld_weight = mpp_def_var(fid, "weight", NC_DOUBLE, 3, dims, 0);
    mpp_end_def(fid);
    mpp_put_var_value(fid, fld_index, interp->index);
    mpp_put_var_value(fid, fld_weight, interp->weight);
    mpp_close(fid);
  }

  /* release the memory */
  free(found);
  free(shortest);
  free(index);
  
  printf("\n done calculating interp_index and interp_weight\n");
}; /* setup_bilinear_interp */

/*----------------------------------------------------------------------------
   void do_scalar_bilinear_interp(Mosaic_config *input, Mosaic_config *output, int varid )
   interpolate scalar data to latlon,                               !
   --------------------------------------------------------------------------*/
void do_scalar_bilinear_interp(const Interp_config *interp, int vid, int ntiles_in, const Grid_config *grid_in, const Grid_config *grid_out,
			  const Field_config *field_in, Field_config *field_out, int finer_step, int fill_missing)
{
  int    nx_in, ny_in, nx_out, ny_out, nz;
  int    n, ts, tn, tw, te;
  int    has_missing;
  double missing;
  double *data_fine;
  
  /*------------------------------------------------------------------
    determine target grid resolution                                 
    ------------------------------------------------------------------*/
  nx_out     = grid_out->nx_fine;
  ny_out     = grid_out->ny_fine;
  nx_in       = grid_in->nx;
  ny_in       = grid_in->ny;
  nz          = field_in[0].var[vid].nz;
  missing     = field_in[0].var[vid].missing;
  has_missing = field_in[0].var[vid].has_missing;

  data_fine = (double *)malloc(nx_out*ny_out*nz*sizeof(double));

  do_c2l_interp(interp, nx_in, ny_in, nz, field_in, nx_out, ny_out, data_fine, has_missing, missing, fill_missing);
  do_latlon_coarsening(data_fine, grid_out->latt1D_fine, nx_out, ny_out, nz, field_out->data,
		       finer_step, has_missing, missing);
  free(data_fine);
  
}; /* do_c2l_scalar_interp */


   
/*----------------------------------------------------------------------------
   void do_vector_bilinear_interp()
   interpolate vector data to latlon,                               !
   --------------------------------------------------------------------------*/
void do_vector_bilinear_interp(Interp_config *interp, int vid, int ntiles_in, const Grid_config *grid_in, int ntiles_out, 
			  const Grid_config *grid_out, const Field_config *u_in,  const Field_config *v_in,
			  Field_config *u_out, Field_config *v_out, int finer_step, int fill_missing)
{
  Field_config *var_cubsph;
  int          nx_in, ny_in, nx_out, ny_out, nxd, nyd, nz, has_missing;
  int          i, j, k, n, n1, n2, ts, tn, tw, te;
  double       missing;
  double       *x_latlon, *y_latlon, *z_latlon, *var_latlon;
  
  nx_out      = grid_out->nx_fine;
  ny_out      = grid_out->ny_fine;
  nx_in       = grid_in->nx;
  ny_in       = grid_in->ny;
  nxd         = nx_in + 2;
  nyd         = ny_in + 2;
  nz          = u_in[0].var[vid].nz;
  missing     = u_in[0].var[vid].missing;
  has_missing = u_in[0].var[vid].has_missing;


  x_latlon   = (double *)malloc(nx_out*ny_out*nz*sizeof(double));
  y_latlon   = (double *)malloc(nx_out*ny_out*nz*sizeof(double));
  z_latlon   = (double *)malloc(nx_out*ny_out*nz*sizeof(double));
  var_latlon = (double *)malloc(nx_out*ny_out*nz*sizeof(double));
  var_cubsph = (Field_config *)malloc(ntiles_in*sizeof(Field_config));
  for(n=0; n<ntiles_in; n++) var_cubsph[n].data = (double *)malloc((nx_in+2)*(ny_in+2)*nz*sizeof(double));

  for(n=0; n<ntiles_in; n++) {
    for(k=0; k<nz; k++) for(j=0; j<nyd; j++) for(i=0; i<nxd; i++) {
      n1 = k*nxd*nyd + j*nxd + i;
      n2 = j*nxd + i;
      var_cubsph[n].data[n1] = u_in[n].data[n1]*grid_in[n].vlon_t[3*n2]+v_in[n].data[n1]*grid_in[n].vlat_t[3*n2];
    }
  }

  do_c2l_interp(interp, nx_in, ny_in, nz, var_cubsph, nx_out, ny_out, x_latlon, has_missing, missing, fill_missing);  

  for(n=0; n<ntiles_in; n++) {
    for(k=0; k<nz; k++) for(j=0; j<nyd; j++) for(i=0; i<nxd; i++) {
      n1 = k*nxd*nyd + j*nxd + i;
      n2 = j*nxd + i;
      var_cubsph[n].data[n1] = u_in[n].data[n1]*grid_in[n].vlon_t[3*n2+1]+v_in[n].data[n1]*grid_in[n].vlat_t[3*n2+1];
    }
  }
  do_c2l_interp(interp, nx_in, ny_in, nz, var_cubsph, nx_out, ny_out, y_latlon, has_missing, missing, fill_missing);  
  
  for(n=0; n<ntiles_in; n++) {
    for(k=0; k<nz; k++) for(j=0; j<nyd; j++) for(i=0; i<nxd; i++) {
      n1 = k*nxd*nyd + j*nxd + i;
      n2 = j*nxd + i;
      var_cubsph[n].data[n1] = u_in[n].data[n1]*grid_in[n].vlon_t[3*n2+2]+v_in[n].data[n1]*grid_in[n].vlat_t[3*n2+2];
    }
  }

  do_c2l_interp(interp, nx_in, ny_in, nz, var_cubsph, nx_out, ny_out, z_latlon, has_missing, missing, fill_missing);  

  for(n=0; n<ntiles_in; n++) free(var_cubsph[n].data);
  free(var_cubsph);

  
  for(k=0; k<nz; k++) for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) {
    n1 = k*nx_out*ny_out + j*nx_out + i;
    n2 = j*nx_out + i;
    var_latlon[n1] = x_latlon[n1]*grid_out->vlon_t[3*n2] +  y_latlon[n1]*grid_out->vlon_t[3*n2+1] +  z_latlon[n1]*grid_out->vlon_t[3*n2+2];
  }
  do_latlon_coarsening(var_latlon, grid_out->latt1D_fine, nx_out, ny_out, nz, u_out->data,
		       finer_step, has_missing, missing);

  for(k=0; k<nz; k++) for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) {
    n1 = k*nx_out*ny_out + j*nx_out + i;
    n2 = j*nx_out + i;
    var_latlon[n1] = x_latlon[n1]*grid_out->vlat_t[3*n2] +  y_latlon[n1]*grid_out->vlat_t[3*n2+1] +  z_latlon[n1]*grid_out->vlat_t[3*n2+2];
  }

  do_latlon_coarsening(var_latlon, grid_out->latt1D_fine, nx_out, ny_out, nz, v_out->data,
		       finer_step, has_missing, missing);

  free(x_latlon);
  free(y_latlon);
  free(z_latlon);
    
}; /* do_vector_bilinear_interp */


void do_c2l_interp(const Interp_config *interp, int nx_in, int ny_in, int nz, const Field_config *field_in,
		   int nx_out, int ny_out, double *data_out, int has_missing, double missing, int fill_missing )
{
  int i, j, k, nxd, nyd, ic, jc, ind, n1, tile;
  double d_in[4];
  
  nxd = nx_in + 2;
  nyd = ny_in + 2;
  
  if (has_missing) {
    for(k=0; k<nz; k++) for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) {
      n1      = j*nx_out+i;
      ic      = interp->index[3*n1];
      jc      = interp->index[3*n1+1];
      tile    = interp->index[3*n1+2];
      d_in[0] = field_in[tile].data[k*nxd*nyd+jc    *nxd+ic];
      d_in[1] = field_in[tile].data[k*nxd*nyd+(jc+1)*nxd+ic];
      d_in[2] = field_in[tile].data[k*nxd*nyd+(jc+1)*nxd+ic+1];
      d_in[3] = field_in[tile].data[k*nxd*nyd+jc    *nxd+ic+1];
      if (d_in[0] == missing || d_in[1] == missing || d_in[3] == missing || d_in[4] == missing ) {
	if (fill_missing) {
	  ind = max_weight_index( &(interp->weight[4*n1]), 4);
	  data_out[k*nx_out*ny_out+n1] = d_in[ind];
	}
	else {
	  data_out[k*nx_out*ny_out+n1] = missing;
	}
      }
      else {
	data_out[k*nx_out*ny_out+n1] = d_in[0]*interp->weight[4*n1] + d_in[1]*interp->weight[4*n1+1]
	  + d_in[2]*interp->weight[4*n1+2] + d_in[3]*interp->weight[4*n1+3];
      }
    }
  }
  else {
    for(k=0; k<nz; k++) for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) {
      n1      = j*nx_out+i;
      ic      = interp->index[3*n1];
      jc      = interp->index[3*n1+1];
      tile    = interp->index[3*n1+2];
      d_in[0] = field_in[tile].data[k*nxd*nyd+jc    *nxd+ic];
      d_in[1] = field_in[tile].data[k*nxd*nyd+(jc+1)*nxd+ic];
      d_in[2] = field_in[tile].data[k*nxd*nyd+(jc+1)*nxd+ic+1];
      d_in[3] = field_in[tile].data[k*nxd*nyd+jc    *nxd+ic+1];
      data_out[k*nx_out*ny_out+n1] = d_in[0]*interp->weight[4*n1] + d_in[1]*interp->weight[4*n1+1]
	+ d_in[2]*interp->weight[4*n1+2] + d_in[3]*interp->weight[4*n1+3];
    }
  }
  
}; /* do_c2l_interp */

   
/*------------------------------------------------------------------
  void sort_index()
  sort index by shortest                                         
  ----------------------------------------------------------------*/
void sort_index(int ntiles, int *index, double *shortest)
{
  int l, ll, lll, i;
  double *shortest_sort;
  int    *index_sort;

  shortest_sort = (double *)malloc(3*ntiles*sizeof(double));
  index_sort    = (int    *)malloc(  ntiles*sizeof(int   ));
  
  for(l=0; l<3*ntiles; l++)index_sort[l] = 0;
  for(l=0; l<ntiles; l++)shortest_sort[l] = M_PI+M_PI;
  for(l=0; l<ntiles; l++) {
    for(ll=0; ll<ntiles; ll++) {
      if (shortest[l]<shortest_sort[ll]) {
	for(lll=ntiles-2; lll>=ll; lll--) {
	  index_sort[3*lll+1]=index_sort[3*lll];
	  shortest_sort[lll+1]=shortest_sort[lll];
	}
	for(i=0; i<3; i++) index_sort[3*ll+i]=index[3*l+i];
	shortest_sort[ll]=shortest[l];
	break;
      }
    }
  }

  for(l=0; l<3*ntiles; l++) index[l]    = index_sort[l];
  for(l=0; l<  ntiles; l++) shortest[l] = shortest_sort[l];

  free(shortest_sort);
  free(index_sort);
  
}; /* sort_index */


/*------------------------------------------------------------------
  void get_index(ig, jg, lg)
  determine lower left corner                                    
  ----------------------------------------------------------------*/
int get_index(const Grid_config *grid_in, const Grid_config *grid_out, int *index,
	       int i_in, int j_in, int l_in, int i_out, int j_out)
{
  int    ok, n0, n1, n2, n3, n4, n5;
  int    nx_in, ny_in, nx_out, ny_out;
  double v0[3], v1[3], v2[3], v3[3], v4[3], v5[3];
  double angle_1, angle_1a, angle_1b, angle_2, angle_2a, angle_2b;
  double angle_3, angle_3a, angle_3b, angle_4, angle_4a, angle_4b;
  
  ok=1;
  nx_in  = grid_in->nx_fine;
  ny_in  = grid_in->nx_fine;
  nx_out = grid_out->nx;
  ny_out = grid_out->nx;
  n0 = j_out*nx_out + i_out;
  n1 = j_in*nx_in + i_in;
  n2 = j_in*nx_in + i_in+1;
  n3 = (j_in+1)*nx_in + i_in;
  v0[0] = grid_out->xt[n1];
  v0[1] = grid_out->yt[n1];
  v0[2] = grid_out->zt[n1];
  v1[0] = grid_in->xt[n1];
  v1[1] = grid_in->yt[n1];
  v1[2] = grid_in->zt[n1];
  v2[0] = grid_in->xt[n2];
  v2[1] = grid_in->yt[n2];
  v2[2] = grid_in->zt[n2];
  v3[0] = grid_in->xt[n3];
  v3[1] = grid_in->yt[n3];
  v3[2] = grid_in->zt[n3];
  angle_1 = spherical_angle(v1, v2, v3);
  angle_1a= spherical_angle(v1, v2, v0); 
  angle_1b= spherical_angle(v1, v3, v0);
  
  if (max(angle_1a,angle_1b)<angle_1) {
    index[0]=i_in;
    index[1]=j_in;
    index[2]=l_in;
  }
  else {
    n4 = j_in*nx_in + i_in-1;
    v4[0] = grid_in->xt[n4];
    v4[1] = grid_in->yt[n4];
    v4[2] = grid_in->zt[n4];
    angle_2 =spherical_angle(v1, v3, v4);
    angle_2a=angle_1b;
    angle_2b=spherical_angle(v1, v4, v0);
    if (max(angle_2a,angle_2b)<angle_2) {
      index[0]=i_in-1;
      index[1]=j_in;
      index[2]=l_in;
    }
    else {
      n5 = (j_in-1)*nx_in + i_in;
      v5[0] = grid_in->xt[n5];
      v5[1] = grid_in->yt[n5];
      v5[2] = grid_in->zt[n5];      
      angle_3 =spherical_angle(v1, v4, v5);
      angle_3a=angle_2b;
      angle_3b=spherical_angle(v1, v5, v0);
      if (max(angle_3a,angle_3b)<angle_3 && i_in>1 && j_in>1) {
	index[0]=i_in-1;
	index[1]=j_in-1;
	index[2]=l_in;
      }
      else {
	angle_4 =spherical_angle(v1, v5, v2);
	angle_4a=angle_3b;
	angle_4b=spherical_angle(v1, v2, v0);
	if (max(angle_4a,angle_4b)<angle_4) {
	  index[0]=i_in;
	  index[1]=j_in-1;
	  index[2]=l_in;
	}
        else {
	  ok=0;
	}
      }
    }
  }
  return ok;
  
}; /* get_index */


/*-------------------------------------------------------------
  determine lower left corner
  void get_closest_index(int ig, int jg, int lg, int ok)
  --------------------------------------------------------------*/
int get_closest_index(const Grid_config *grid_in, const Grid_config *grid_out, int *index,
		      int i_in, int j_in, int l_in, int i_out, int j_out)
{
  int found;
  double angle_11, angle_11a, angle_11b;
  double angle_22, angle_22a, angle_22b;
  double angle_33, angle_33a, angle_33b;
  double angle_44, angle_44a, angle_44b;
  double angle_1,  angle_1a,  angle_1b;
  double angle_2,  angle_2a,  angle_2b;
  double angle_3,  angle_3a,  angle_3b;
  double angle_4,  angle_4a,  angle_4b;  
  int    n0, n1, n2, n3, n4, n5, n6, n7, n8;
  int    nx_in, ny_in, nx_out, ny_out, nxd;
  double v0[3], v1[3], v2[3], v3[3], v4[3], v5[3], v6[3], v7[3], v8[3];

  found = 0;
  nx_in  = grid_in->nx;
  ny_in  = grid_in->ny;
  nxd    = nx_in + 2;
  nx_out = grid_out->nx_fine;
  ny_out = grid_out->ny_fine;
  n0     = j_out*nx_out+i_out;
  n1     = j_in*nxd+i_in;
  n2     = j_in*nxd+i_in+1;
  n3     = (j_in+1)*nxd+i_in;
  v1[0]  = grid_in->xt[n1];
  v1[1]  = grid_in->yt[n1];
  v1[2]  = grid_in->zt[n1];
  v2[0]  = grid_in->xt[n2];
  v2[1]  = grid_in->yt[n2];
  v2[2]  = grid_in->zt[n2];
  v3[0]  = grid_in->xt[n3];
  v3[1]  = grid_in->yt[n3];
  v3[2]  = grid_in->zt[n3];
  v0[0]  = grid_out->xt[n0];
  v0[1]  = grid_out->yt[n0];
  v0[2]  = grid_out->zt[n0];
  angle_1 =spherical_angle(v1, v2, v3);
  angle_1a=spherical_angle(v1, v2, v0);
  angle_1b=spherical_angle(v1, v3, v0);
  if (max(angle_1a,angle_1b) <= angle_1) {
    if (i_in==nx_in && j_in==ny_in) {
      angle_11 =spherical_angle(v2, v3, v1);
      angle_11a=spherical_angle(v2, v1, v0);
      angle_11b=spherical_angle(v2, v3, v0);
    }
    else {
      n4 = (j_in+1)*nxd+i_in+1;
      v4[0]  = grid_in->xt[n4];
      v4[1]  = grid_in->yt[n4];
      v4[2]  = grid_in->zt[n4];
      angle_11 =spherical_angle(v4, v3, v2);
      angle_11a=spherical_angle(v4, v2, v0);
      angle_11b=spherical_angle(v4, v3, v0);
    }
    if (max(angle_11a,angle_11b)<=angle_11) {
      found = 1;
      index[0]=i_in;
      index[1]=j_in;
      index[2]=l_in;
    }
  }
  else {
    n4 = j_in*nxd+i_in-1;
    v4[0]  = grid_in->xt[n4];
    v4[1]  = grid_in->yt[n4];
    v4[2]  = grid_in->zt[n4]; 
    angle_2 =spherical_angle(v1,v3,v4);
    angle_2a=angle_1b;
    angle_2b=spherical_angle(v1,v4,v0);
    if (max(angle_2a,angle_2b)<=angle_2) {
      if (i_in==1 && j_in==ny_in) {
	angle_22 =spherical_angle(v3, v1, v4);
	angle_22a=spherical_angle(v3, v4, v0);
	angle_22b=spherical_angle(v3, v1, v0);
      }
      else {
	n5 = (j_in+1)*nxd+i_in-1;
	n6 = j_in    *nxd+i_in-1;
	v5[0]  = grid_in->xt[n5];
	v5[1]  = grid_in->yt[n5];
	v5[2]  = grid_in->zt[n5];
	v6[0]  = grid_in->xt[n6];
	v6[1]  = grid_in->yt[n6];
	v6[2]  = grid_in->zt[n6];
	angle_22 =spherical_angle(v5, v3, v6);
	angle_22a=spherical_angle(v5, v6, v0);
	angle_22b=spherical_angle(v5, v3, v0);
      }
      if (max(angle_22a,angle_22b)<=angle_22) {
	found=1;
	index[0]=i_in-1;
	index[1]=j_in;
	index[2]=l_in;
      }
    }
    else {
      n5 = j_in*nxd+i_in-1;
      n6 = (j_in-1)*nxd+i_in;
      v5[0]  = grid_in->xt[n5];
      v5[1]  = grid_in->yt[n5];
      v5[2]  = grid_in->zt[n5];
      v6[0]  = grid_in->xt[n6];
      v6[1]  = grid_in->yt[n6];
      v6[2]  = grid_in->zt[n6];	  
      angle_3 =spherical_angle(v1, v5, v6);
      angle_3a=angle_2b;
      angle_3b=spherical_angle(v1, v6, v0);
      if (max(angle_3a,angle_3b)<=angle_3 && i_in>1 && j_in>1) {
	n7 = (j_in-1)*nxd+i_in-1;
	v7[0]  = grid_in->xt[n7];
	v7[1]  = grid_in->yt[n7];
	v7[2]  = grid_in->zt[n7];    
	angle_33 =spherical_angle(v7, v6, v5);
	angle_33a=spherical_angle(v7, v5, v0);
	angle_33b=spherical_angle(v7, v6, v0);
	if (max(angle_33a,angle_33b)<=angle_33) {
	  found=1;
	  index[0]=i_in-1;
	  index[1]=j_in-1;
	  index[2]=l_in;
	}
      }
      else {
	angle_4 =spherical_angle(v1, v6, v2);
	angle_4a=angle_3b;
	angle_4b=spherical_angle(v1, v2, v0);
	if (max(angle_4a,angle_4b)<=angle_4) {
	  if (i_in==nx_in && j_in==1) {
	    angle_44 =spherical_angle(v2, v1, v6);
	    angle_44a=spherical_angle(v2, v6, v0);
	    angle_44b=spherical_angle(v2, v1, v0);
	  }
	  else {
	    n8 = (j_in-1)*nxd+i_in+1;
	    v8[0]  = grid_in->xt[n8];
	    v8[1]  = grid_in->yt[n8];
	    v8[2]  = grid_in->zt[n8];   
	    angle_44 =spherical_angle(v8, v2, v6);
	    angle_44a=spherical_angle(v8, v6, v0);
	    angle_44b=spherical_angle(v8, v2, v0);
	  }
	  if (max(angle_44a,angle_44b)<=angle_44) {
	    found=1;
	    index[0]=i_in;
	    index[1]=j_in-1;
	    index[2]=l_in;
	  }
	}
      }
    }
  }
  return found;
  
}; /* get_closest_index */
	      


/*--------------------------------------------------------------------------

calculate normalized great circle distance between v1 and v2 
double normalize_great_circle_distance(v1, v2)
---------------------------------------------------------------------------*/
double normalize_great_circle_distance(const double *v1, const double *v2)
{
  double dist;

  dist=(v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
    /sqrt((v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2])                 
	  *(v2[0]*v2[0]+v2[1]*v2[1]+v2[2]*v2[2]));
  dist = sign(min(1.,fabs(dist)),dist);
  dist = acos(dist);
  return dist;
  
}; /* normalize_great_circle_distance */

/*------------------------------------------------------------------
  double spherical_angle(v1, v2, v3)

  calculate spherical angle of a triangle formed by v1, v2 and v3 at v1                                                            
  ------------------------------------------------------------------*/
/* double spherical_angle(double *v1, double *v2, double *v3) */
/* { */
/*   double angle; */
/*   double px, py, pz, qx, qy, qz, abs_p, abs_q; */

/*   /* vector product between v1 and v2 */
/*   px = v1[1]*v2[2] - v1[2]*v2[1]; */
/*   py = v1[2]*v2[0] - v1[0]*v2[2]; */
/*   pz = v1[0]*v2[1] - v1[1]*v2[0]; */
/*   /* vector product between v1 and v3  */
/*   qx = v1[1]*v3[2] - v1[2]*v3[1]; */
/*   qy = v1[2]*v3[0] - v1[0]*v3[2]; */
/*   qz = v1[0]*v3[1] - v1[1]*v3[0]; */
    
/*   /* angle between p and q */ 
/*   abs_p=px*px+py*py+pz*pz; */
/*   abs_q=qx*qx+qy*qy+qz*qz; */
/*   if (abs_p*abs_q==0.) */
/*     angle=0.; */
/*   else { */
/*     angle = (px*qx+py*qy+pz*qz)/sqrt(abs_p*abs_q); */
/*     angle = sign(min(1.,fabs(angle)),angle); */
/*     angle = acos(angle); */
/*   } */

/*   return angle; */
/* }; /* spherical_angle */

/*---------------------------------------------------------------------
  double dist2side(v1, v2, point)
  calculate shortest normalized distance on sphere                 
  from point to straight line defined by v1 and v2                 
  ------------------------------------------------------------------*/
double dist2side(const double *v1, const double *v2, const double *point)
{
  double angle, side;

  angle = spherical_angle(v1, v2, point);
  side  = normalize_great_circle_distance(v1, point);

  return (asin(sin(side)*sin(angle)));

};/* dist2side */

  
int max_weight_index( double *var, int nvar)
{

  int ind, i;

  ind = 0;

  for(i=1; i<nvar; i++) {
    if(var[i]>var[ind]) ind = i;
  }

  return ind;
}

/*------------------------------------------------------------------------------
  void do_latlon_coarsening(var_latlon, ylat, nlon, nlat, nz,     
                            var_latlon_crs, nlon_crs, nlat_crs,   
                            finer_steps, misval, varmisval)

  calculate variable on coarser latlon grid                        
  by doubling spatial resolution and preserving volume means       
  ---------------------------------------------------------------------------*/
void do_latlon_coarsening(const double *var_latlon, const double *ylat, int nlon, int nlat, int nz,    
			  double *var_latlon_crs, int finer_steps, int has_missing, double missvalue)
{

  double  *var_latlon_old, *ylat_old, *var_latlon_new;
  double  dlat;
  int     nlon_old, nlat_old, nlon_new, nlat_new, steps, i, j;
  int     nlon_crs, nlat_crs;
  
  nlon_crs=nlon/pow(2,finer_steps);
  nlat_crs=(nlat-1)/pow(2,finer_steps)+1;
  switch (finer_steps) {
  case 0:
    if (nlon_crs !=nlon || nlat_crs != nlat) mpp_error("bilinear_interp(do_latlon_coarsening): grid dimensions don't match");
    for(i=0; i<nlon*nlat*nz; i++) var_latlon_crs[i] = var_latlon[i];
    break;
  case 1:
    redu2x(var_latlon, ylat, nlon, nlat, var_latlon_crs, nlon_crs, nlat_crs, nz, has_missing, missvalue);
    break;
  default:
    nlon_new=nlon;
    nlat_new=nlat;
    for(steps=1; steps<=finer_steps; steps++) {
      nlon_old=nlon_new;
      nlat_old=nlat_new;
      var_latlon_old = (double *)malloc(nlon_old*nlat_old*nz*sizeof(double));
      ylat_old       = (double *)malloc(nlat_old*sizeof(double));
      if (steps==1) {
	for(i=0; i<nlat; i++) ylat_old[i] = ylat[i]; 
	for(i=0; i<nlon_old*nlat_old*nz; i++) var_latlon_old[i] = var_latlon[i];
      }
      else {
	dlat=M_PI/(nlat_old-1);
	ylat_old[0]=-0.5*M_PI;
	ylat_old[nlat_old-1]= 0.5*M_PI;
	for(j=1; j<nlat_old-1; j++) ylat_old[j] = ylat_old[0] + j*dlat;
	for(i=0; i<nlon_old*nlat_old*nz; i++) var_latlon_old[i] = var_latlon_new[i];
	free(var_latlon_new);
      }
          
      nlon_new=nlon_new/2;
      nlat_new=(nlat_new-1)/2+1;
      var_latlon_new = (double *)malloc(nlon_new*nlat_new*nz*sizeof(double));
      redu2x(var_latlon_old, ylat_old, nlon_old, nlat_old, var_latlon_new, nlon_new, nlat_new, nz, has_missing, missvalue);
      free(var_latlon_old);
      free(ylat_old);
    }
    for(i=0; i<nlon_new*nlat_new*nz; i++) var_latlon_crs[i] = var_latlon_new[i];
    free(var_latlon_new);
  }
}; /* do_latlon_coarsening */

/*------------------------------------------------------------------------------
  void redu2x(varfin, yfin, nxfin, nyfin, varcrs, nxcrs, nycrs)
  this routine is for reducing fvccm data by a factor of 2       
  volume averaging for all data except at the poles              
  original developer: S.-J. Lin                                  
  ----------------------------------------------------------------------------*/
void redu2x(const double *varfin, const double *yfin, int nxfin, int nyfin, double *varcrs,
	    int nxcrs, int nycrs, int nz, int has_missing, double missvalue)
{
  double  *cosp, *acosp, *vartmp;
  int     i, j, k, i2, j2, n1, n2;

  /*------------------------------------------------------------------
    calculate cosine of latitude                                   
    trick in cosp needed to maintain a constant field              
    ----------------------------------------------------------------*/
  cosp  = (double *)malloc(nyfin*sizeof(double));
  acosp = (double *)malloc(nyfin*sizeof(double));
  vartmp = (double *)malloc(nxcrs*nyfin*nz*sizeof(double));
  cosp[0]=0.;
  cosp[nyfin-1]=0.;
  for(j=1; j<nyfin-1; j++) cosp[j]  = cos(yfin[j]);
  for(j=1; j<nyfin-1; j++) acosp[j] = 1./(cosp[j]+0.5*(cosp[j-1]+cosp[j+1]));

  /*----------------------------------------------------------------
    x-sweep                                                        
    ----------------------------------------------------------------*/
  if(has_missing) {
    for(k=0; k<nz; k++) for(j=1; j<nyfin-1; j++) {
      n1 = k*nxfin*nyfin+j*nxfin;
      n2 = k*nxcrs*nyfin+j*nxcrs;
      if (varfin[n1+nxfin-1] == missvalue || varfin[n1] == missvalue || varfin[n1+1] == missvalue)
	vartmp[n2] = missvalue;
      else
	vartmp[n2] = 0.25*(varfin[n1+nxfin-1]+2.*varfin[n1]+varfin[n1+1]);
      for(i=2; i<nxfin-1; i+=2) {
	i2 = i/2;
	if (varfin[n1+i-1] == missvalue || varfin[n1+i] == missvalue || varfin[n1+i+1] == missvalue) 
	  vartmp[n2+i2] = missvalue;
        else
	  vartmp[n2+i2] = 0.25*(varfin[n1+i-1]+2.*varfin[n1+i]+varfin[n1+i+1]);
      }
    }
  }
  else {
    for(k=0; k<nz; k++) for(j=1; j<nyfin-1; j++) {
      n1 = k*nxfin*nyfin+j*nxfin;
      n2 = k*nxcrs*nyfin+j*nxcrs;
      vartmp[n2] = 0.25*(varfin[n1+nxfin-1]+2.*varfin[n1]+varfin[n1+1]);
      for(i=2; i<nxfin-1; i+=2) {
	i2 = i/2;
	vartmp[n2+i2] = 0.25*(varfin[n1+i-1]+2.*varfin[n1+i]+varfin[n1+i+1]);
      }
    }
  }
  /*---------------------------------------------------------------------
    poles:                                                         
    this code segment works for both the scalar and vector fields. 
    Winds at poles are wave-1; the follwoing is quick & dirty yet the correct way
    The skipping method. A more rigorous way is to                 
    recompute the wave-1 components for the coarser grid.          
    --------------------------------------------------------------------*/
  for(k=0; k<nz; k++) for(i=0; i<nxcrs; i++) {
    i2 = i*2;
    n1 = k*nxcrs*nycrs;
    n2 = k*nxfin*nyfin; 
    varcrs[n1+i] = varfin[n2+i2];
    varcrs[n1+(nycrs-1)*nxcrs+i] = varfin[n1+(nyfin-1)*nxfin+i2];
  }
  /*----------------------------------------------------------------
    y-sweep                                                        
    ----------------------------------------------------------------*/
  if (has_missing) {
    for(k=0; k<nz; k++) for(j=1; j<nyfin-1; j++) for(i=0; i<nxcrs; i++) {
      n1 = k*nxcrs*nyfin+j*nxcrs+i;
      if (vartmp[n1] /= missvalue) vartmp[n1] *= cosp[j];
    }
    for(k=0; k<nz; k++) for(j=2; j<nyfin-2; j+=2) {
      j2 = j/2;
      for(i=0; i<nxcrs; i++) {
	n1 = k*nxcrs*nyfin + i;
	n2 = k*nxcrs*nycrs + i;
	if (vartmp[n1+j*nxcrs] == missvalue || vartmp[n1+(j-1)*nxcrs] == missvalue
	    || vartmp[n1+(j+1)*nxcrs] == missvalue )
	  varcrs[n2+j2*nxcrs] = missvalue;
	else
	  varcrs[n2+j2*nxcrs] = acosp[j]*(vartmp[n1+j*nxcrs] + 0.5*(vartmp[n1+(j-1)*nxcrs]+ vartmp[n1+(j+1)*nxcrs]));
      }
    }
  }
  else {
    for(k=0; k<nz; k++) for(j=1; j<nyfin-1; j++) for(i=0; i<nxcrs; i++) {
      n1 = k*nxcrs*nyfin+j*nxcrs+i;
      vartmp[n1] *= cosp[j];
    }
    for(k=0; k<nz; k++) for(j=2; j<nyfin-2; j+=2) {
      j2 = j/2;
      for(i=0; i<nxcrs; i++) {
	n1 = k*nxcrs*nyfin + i;
	n2 = k*nxcrs*nycrs + i;
	varcrs[n2+j2*nxcrs] = acosp[j]*(vartmp[n1+j*nxcrs] + 0.5*(vartmp[n1+(j-1)*nxcrs]+ vartmp[n1+(j+1)*nxcrs]));
      }
    }
  }

  free(cosp);
  free(acosp);
  free(vartmp);
  
}; /*redu2x*/


