#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include "constant.h"
#include "mpp.h"
#include "mpp_domain.h"
#include "mpp_io.h"

const int MAXBOUNDS = 100;
const int STRINGLEN = 255;

const int MAXTILE = 100;
const int MAXCONTACT = 100;
const int SHORTSTRING = 32;
const char mosaic_version[] = "0.2";

#define PI M_PI
  
double *x, *y, *dx, *dy, *area, *angle_dx, *angle_dy; 
int nx, ny, nxp, nyp;
char *old_file = NULL;

int main(int argc, char* argv[])
{
/* ----------------------------- */
  extern char *optarg;
  char *pch=NULL, history[512], entry[1280];
  char tilefile[MAXTILE][STRING], tiletype[MAXTILE][SHORTSTRING];
  char tile_name[MAXTILE][STRING];
  int ntiles=0, nfiles=0, ncontact=0;
  double periodx=0, periody=0;
  int contact_tile1[MAXCONTACT], contact_tile2[MAXCONTACT];
  int contact_tile1_istart[MAXCONTACT], contact_tile1_iend[MAXCONTACT];
  int contact_tile1_jstart[MAXCONTACT], contact_tile1_jend[MAXCONTACT];
  int contact_tile2_istart[MAXCONTACT], contact_tile2_iend[MAXCONTACT];
  int contact_tile2_jstart[MAXCONTACT], contact_tile2_jend[MAXCONTACT];
  char mosaic_name[128] = "atmos_mosaic";
  char mosaic_dir[256]  = "./" ;
  char grid_descriptor[128] = "";
  int c, i, n, m, l, errflg, check=0;

  char atmos_name[128]="atmos", land_name[128]="land", ocean_name[128]="ocean";
  char agrid_file[128], lgrid_file[128], ogrid_file[128];

  int is_coupled_grid = 0, is_ocean_only =1; 
  int interp_order=1;
  
  int option_index;
  static struct option long_options[] = {
    {"input_file",       required_argument, NULL, 'o'},
    {"mosaic_dir",       required_argument, NULL, 'd'},    
    {NULL, 0, NULL, 0}
  };

/* ----------------------------- */

  mpp_init(&argc, &argv); /* Initilize */ 
  //mpp_domain_init();  

  while ((c = getopt_long(argc, argv, "h", long_options, &option_index) ) != -1)
    switch (c) {
      case 'o':
	old_file = optarg;
	break;
      case 'd':
        strcpy(mosaic_dir, optarg);
	break;
    }

  if(!old_file) mpp_error("Usage:\n \t \t transfer_to_mosaic --input_file input_file.nc");
  if(mpp_field_exist(old_file, "AREA_ATMxOCN") ) is_coupled_grid = 1;
  if(mpp_field_exist(old_file, "AREA_ATM") ) is_ocean_only = 0;
  if(mpp_field_exist(old_file, "DI_ATMxOCN") ) interp_order= 2;
// -----------------ocean_hgrid.nc---------------------------------[
// Ocean
{
  int nx_C, ny_C, nx_T, ny_T, nvertex;
  int fid_old, vid;
  
  double *x_C, *y_C, *x_T, *y_T;
  double *x_vert_T, *y_vert_T;


  double *ds_00_02_C, *ds_00_20_C, *ds_01_11_C, *ds_01_21_C, *ds_02_22_C, 
         *ds_10_11_C, *ds_10_12_C, *ds_11_21_C, *ds_11_12_C, *ds_20_22_C, 
         *ds_01_11_T, *ds_01_21_T, *ds_02_22_T, *ds_10_11_T, *ds_10_12_T, 
         *ds_11_12_T, *ds_11_21_T, *ds_20_22_T, 
         *ds_01_21_E, *ds_10_12_N, *angle_C; 

 
  int i,j,k, i1, i2, j1, j2, k0, k1, k2, k3;
  int ji, ji0, ji1, ji2, ji3;
  int j1i1, j1i2, j2i1, j2i2; 
  double unknown = 0.0; /* double unknown = -1.0E+10; */

  fid_old = mpp_open(old_file, MPP_READ);
  nx_C = mpp_get_dimlen(fid_old, "grid_x_C");
  ny_C = mpp_get_dimlen(fid_old, "grid_y_C");
  nx_T = mpp_get_dimlen(fid_old, "grid_x_T");
  ny_T = mpp_get_dimlen(fid_old, "grid_y_T");
  nvertex = mpp_get_dimlen(fid_old, "vertex");

  printf("\nReading file: %s \n", old_file); 


/* Reading Ocean Grid Variables */
  x_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
  y_C = (double *) malloc(ny_C * nx_C * sizeof(double));
  x_T = (double *) malloc(ny_T * nx_T * sizeof(double));
  y_T = (double *) malloc(ny_T * nx_T * sizeof(double));
  x_vert_T = (double *) malloc(nvertex*ny_T*nx_T*sizeof(double));
  y_vert_T = (double *) malloc(nvertex*ny_T*nx_T*sizeof(double));
  ds_01_11_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
  ds_10_11_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
  ds_11_21_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
  ds_11_12_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
  ds_01_11_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
  ds_10_11_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
  ds_11_12_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
  ds_11_21_T = (double *) malloc(ny_T * nx_T * sizeof(double));
  ds_02_22_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
  ds_20_22_T = (double *) malloc(ny_T * nx_T * sizeof(double));
  angle_C    = (double *) malloc(ny_C * nx_C * sizeof(double));
  
  vid = mpp_get_varid(fid_old, "x_C"); mpp_get_var_value(fid_old, vid, x_C);
  vid = mpp_get_varid(fid_old, "y_C"); mpp_get_var_value(fid_old, vid, y_C);
  vid = mpp_get_varid(fid_old, "x_T"); mpp_get_var_value(fid_old, vid, x_T);
  vid = mpp_get_varid(fid_old, "y_T"); mpp_get_var_value(fid_old, vid, y_T);
  vid = mpp_get_varid(fid_old, "x_vert_T"); mpp_get_var_value(fid_old, vid, x_vert_T);
  vid = mpp_get_varid(fid_old, "y_vert_T"); mpp_get_var_value(fid_old, vid, y_vert_T);
  vid = mpp_get_varid(fid_old, "ds_01_11_C"); mpp_get_var_value(fid_old, vid, ds_01_11_C);  
  vid = mpp_get_varid(fid_old, "ds_10_11_C"); mpp_get_var_value(fid_old, vid, ds_10_11_C);
  vid = mpp_get_varid(fid_old, "ds_11_21_C"); mpp_get_var_value(fid_old, vid, ds_11_21_C);    
  vid = mpp_get_varid(fid_old, "ds_11_12_C"); mpp_get_var_value(fid_old, vid, ds_11_12_C);
  vid = mpp_get_varid(fid_old, "ds_01_11_T"); mpp_get_var_value(fid_old, vid, ds_01_11_T);  
  vid = mpp_get_varid(fid_old, "ds_10_11_T"); mpp_get_var_value(fid_old, vid, ds_10_11_T);
  vid = mpp_get_varid(fid_old, "ds_11_21_T"); mpp_get_var_value(fid_old, vid, ds_11_21_T);    
  vid = mpp_get_varid(fid_old, "ds_11_12_T"); mpp_get_var_value(fid_old, vid, ds_11_12_T);  
  vid = mpp_get_varid(fid_old, "ds_02_22_T"); mpp_get_var_value(fid_old, vid, ds_02_22_T);    
  vid = mpp_get_varid(fid_old, "ds_20_22_T"); mpp_get_var_value(fid_old, vid, ds_20_22_T);  
  vid = mpp_get_varid(fid_old, "angle_C"); mpp_get_var_value(fid_old, vid, angle_C);  
  mpp_close(fid_old);
  
  nx = 2*nx_T;  nxp = nx +1;
  ny = 2*ny_T;  nyp = ny +1;
  x = (double *) malloc(nxp*nyp * sizeof(double));
  y = (double *) malloc(nxp*nyp* sizeof(double));
  dx = (double *) malloc(nx*nyp * sizeof(double));
  dy = (double *) malloc(nxp*ny * sizeof(double));
  area = (double *) malloc(nx*ny * sizeof(double));
  angle_dx = (double *) malloc(nxp*nyp * sizeof(double));

  for(j = 0; j<ny_T; j++) {
  for(i = 0; i<nx_T; i++) {

	i2=2*(i+1); i1=i2-1;   j2=2*(j+1); j1=j2-1;

  	j1i1=j1*nxp+i1; j1i2=j1*nxp+i2; j2i2=j2*nxp+i2; j2i1=j2*nxp+i1; 
	ji=j*nx_T+i;
	ji0=ji; ji1=ji0+nx_T*ny_T; ji2=ji1+nx_T*ny_T; ji3=ji2+nx_T*ny_T;

        x[j1i1] = x_T[ji];
        x[j1i2] = 0.5*(x_vert_T[ji1]+x_vert_T[ji2]);
        x[j2i2] = x_vert_T[ji2];
        x[j2i1] = 0.5*(x_vert_T[ji3]+x_vert_T[ji2]);

        y[j1i1] = y_T[ji];
        y[j1i2] = 0.5*(y_vert_T[ji1]+y_vert_T[ji2]);
        y[j2i2] = y_vert_T[ji2];
        y[j2i1] = 0.5*(y_vert_T[ji3]+y_vert_T[ji2]);

/*      Mapping distance to new mosaic grid format from old grid

	       dx(1,1)=                  dx(2,1)=           
   Ci-1,j------ds_11_21_C(i-1,j)---+-----ds_01_11_C(i,j)--------Ci,j
	|                          |                            |
	|                          |                            |
	|                          |                            |
	|                     ds_11_12_T                   ds_10_11_C(i,j)
	|                          |                            |
	|         dx(1,1)=         |          dx(2,1)=          |
	+---------ds_01_11_T------Tij---------ds_11_21_T--------+
	|                          |                            |
	|                          |                            |
	|                          |                            |
	|                     ds_11_12_T                   ds_11_12_C(i,j-1)
	|                          |                            |
	|                          |                            |
 Ci-1,j-1--------------------------+----------------------------Ci,j-1

*/
	dx[(2*j+1)*nx  + (2*i)]   = ds_01_11_T[j*nx_T + i];
	dx[(2*j+1)*nx  + (2*i+1)] = ds_11_21_T[j*nx_T + i];
	if(i>0) dx[(2*j+2)*nx  + (2*i)]   = ds_11_21_C[j*nx_T + (i-1)];
	dx[(2*j+2)*nx  + (2*i+1)] = ds_01_11_C[j*nx_T + i];

	dy[(2*j  )*nxp + (2*i+1)]   = ds_10_11_T[j*nx_T + i];
	if(j>0) dy[(2*j  )*nxp + (2*i+2)]   = ds_11_12_C[(j-1)*nx_T + i];
	dy[(2*j+1)*nxp + (2*i+1)]   = ds_11_12_T[j*nx_T + i];
	dy[(2*j+1)*nxp + (2*i+2)]   = ds_10_11_C[j*nx_T + i];

	angle_dx[(2*j+1)*nxp + 2*i+1] = unknown;
	angle_dx[(2*j+1)*nxp + 2*i+2] = unknown;
	angle_dx[(2*j+2)*nxp + 2*i+1] = unknown;
	angle_dx[(2*j+2)*nxp + 2*i+2] = angle_C[j*nx_C + i ]; 

	if (j==0) {
           x[i1] = 0.5*(x_vert_T[ji0]+x_vert_T[ji1]);
           x[i2] = x_vert_T[ji1];
           y[i1] = 0.5*(y_vert_T[ji0]+y_vert_T[ji1]);
           y[i2] = y_vert_T[ji1];

	   dx[ (2*i+0)] = unknown;
	   dx[ (2*i+1)] = unknown;
	   /* dy[(2*j  )*nxp + (2*i+2)]   = unknown; */
	   dy[(2*j  )*nxp + (2*i+2)]   = ds_20_22_T[j*nx_T + i] - ds_10_11_C[j*nx_T + i];

	   angle_dx[2*i+1] = unknown;
	   angle_dx[2*i+2] = unknown;
	}

	if (i==0) {
           x[j1*nxp] = 0.5*(x_vert_T[ji0]+x_vert_T[ji3]);
           x[j2*nxp] = x_vert_T[ji3];
           y[j1*nxp] = 0.5*(y_vert_T[ji0]+y_vert_T[ji3]);
           y[j2*nxp] = y_vert_T[ji3];

	   /* dx[(2*j+2)*nx  + (2*i)]   = unknown;*/
	   dx[(2*j+2)*nx  + (2*i)]   = ds_02_22_T[j*nx_T + i] - ds_01_11_C[j*nx_T + i];
	   dy[(2*j  )*nxp ]    = unknown;
	   dy[(2*j+1)*nxp ]    = unknown;

  	   angle_dx[(2*j+1)*nxp] = unknown;
  	   angle_dx[(2*j+2)*nxp] = unknown;
	}
	if (i==0 && j==0) {
           x[0] = x_vert_T[ji0]; y[0] = y_vert_T[ji0];
  	   angle_dx[0] = unknown;
	}

  }
  }

  //for(j = 0; j<nyp; j++) { for(i = 0; i<nxp; i++) { if(angle_dx[j*nxp+i]>0.0) printf("%d %d %f\n ",i,j,angle_dx[j*nxp+i]); } }
  //for(j = 0; j<ny_T; j++) { printf("%f ",y_C[j*nx_T+(nx_T+0)/4]); }
  //for(i = 0; i<nx_T; i++) { if(y_T[(ny_T-1)*nx_T+i]>85.) printf("%f ",y_T[(ny_T-1)*nx_T+i]); }
  //printf("\n\n");
  //for(j = 0; j<nyp; j++) { printf("%f ",y[j*nxp+(nxp-1)/4]); }
  //for(i = 0; i<nxp; i++) { if(y[(nyp-1)*nxp+i]>85.) printf("%f ",y[(nyp-1)*nxp+i]); }


  /* write ocean-grid data */
  /*-------ocean_hgrid.nc-------*/
  {
    int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
    int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp;
    char tile_spec_version[] = "0.2", geometry[] = "spherical", discretization[] = "logically_rectangular", conformal[]="true";
    char outfile[128] = "", tilename[128]="";
    size_t start[4], nwrite[4];
    
	  sprintf(outfile, "%s_hgrid.nc",ocean_name);
	  printf("Writing %s\n", outfile);
          printf("\t nx_C=%d, ny_C=%d, x_T=%d, ny_T=%d \n", nx_C, ny_C,nx_T, ny_T); 
          sprintf(tilename, "tile%d", 1);
          fid = mpp_open(outfile, MPP_WRITE);

          dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
          dimlist[1] = mpp_def_dim(fid, "nxp", nxp);
          dimlist[2] = mpp_def_dim(fid, "nyp", nyp);
          dimlist[3] = mpp_def_dim(fid, "nx", nx);
          dimlist[4] = mpp_def_dim(fid, "ny", ny);

          id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec",
                                "tile_spec_version", tile_spec_version, "geometry", geometry,
                                "discretization", discretization, "conformal", conformal );


          dims[0] = dimlist[2]; dims[1] = dimlist[1];
          id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude",
                             "units", "degree_east");
          id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude",
                             "units", "degree_north");

          dims[0] = dimlist[2]; dims[1] = dimlist[3];
          id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance",
                             "units", "meters");
          dims[0] = dimlist[4]; dims[1] = dimlist[1];
          id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance",
                             "units", "meters");

          dims[0] = dimlist[2]; dims[1] = dimlist[1];
          id_angle_dx= mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_vertex_x_angle_WRT_geographic_east",
                             "units", "degrees_east");

          dims[0] = dimlist[4]; dims[1] = dimlist[3];
          id_area= mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area",
                             "units", "m2");

          mpp_end_def(fid);

	  for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 0; }
	  nwrite[0] = strlen(tilename);
	  mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );
          mpp_put_var_value(fid, id_x, x);
          mpp_put_var_value(fid, id_y, y);
          mpp_put_var_value(fid, id_dx, dx);
          mpp_put_var_value(fid, id_dy, dy);
          mpp_put_var_value(fid, id_angle_dx, angle_dx);
          mpp_close(fid);
  }
}
// ----------------------------------------------------------------]
// -----------------atmos_hgrid.nc---------------------------------[
// Atmosphere
  int nxba, nyba, nxta, nyta, nxap, nyap;
if(is_coupled_grid) {
  double *x_atmos, *y_atmos;
  double *xba, *yba, *xta, *yta;
  int i,j, fid_old, vid;

  fid_old = mpp_open(old_file, MPP_READ);
  nxba = mpp_get_dimlen(fid_old, "xba");
  nyba = mpp_get_dimlen(fid_old, "yba");
  nxta = mpp_get_dimlen(fid_old, "xta");
  nyta = mpp_get_dimlen(fid_old, "yta");
  xba = (double *) malloc(nxba * sizeof(double)); 
  yba = (double *) malloc(nyba * sizeof(double)); 
  xta = (double *) malloc(nxta * sizeof(double)); 
  yta = (double *) malloc(nyta * sizeof(double)); 
  vid = mpp_get_varid(fid_old, "xba"); mpp_get_var_value(fid_old, vid, xba);
  vid = mpp_get_varid(fid_old, "yba"); mpp_get_var_value(fid_old, vid, yba);
  vid = mpp_get_varid(fid_old, "xta"); mpp_get_var_value(fid_old, vid, xta);
  vid = mpp_get_varid(fid_old, "yta"); mpp_get_var_value(fid_old, vid, yta);
  mpp_close(fid_old);
  
  nxap = nxba + nxta; nyap = nyba + nyta; 
  x_atmos = (double *) malloc(nxap*nyap * sizeof(double));
  y_atmos = (double *) malloc(nxap*nyap * sizeof(double));

  for(i = 0; i<nxba; i++) x_atmos[2*i]   = xba[i]; for(i = 0; i<nxta; i++) x_atmos[2*i+1]   = xta[i]; 

  for(j = 0; j<nyba; j++) y_atmos[2*j*nxap]   = yba[j]; 
  for(j = 0; j<nyta; j++) y_atmos[(2*j+1)*nxap]   = yta[j]; 

  for(j = 1; j<nyap; j++) for(i = 0; i<nxap; i++) x_atmos[j*nxap + i]   = x_atmos[i];
  for(i = 1; i<nxap; i++) for(j = 0; j<nyap; j++) y_atmos[j*nxap + i]   = y_atmos[j*nxap];

  /* write Atmosphere-grid data */
  {
    int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
    int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp;
    char tile_spec_version[] = "0.2", geometry[] = "spherical", discretization[] = "logically_rectangular", conformal[]="true";
    char outfile[128] = "", tilename[128]="";
    size_t start[4], nwrite[4];
    
          sprintf(outfile, "%s_hgrid.nc", atmos_name);
	  printf("Writing %s\n", outfile);
          printf("\t nxba=%d, nyba=%d, nxta=%d, nyta=%d \n", nxba, nyba,nxta, nyta); 

          sprintf(tilename, "tile%d", 1);
          fid = mpp_open(outfile, MPP_WRITE);
          dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
          dimlist[1] = mpp_def_dim(fid, "nxp", nxap);
          dimlist[2] = mpp_def_dim(fid, "nyp", nyap);
          dimlist[3] = mpp_def_dim(fid, "nx", nxap-1);
          dimlist[4] = mpp_def_dim(fid, "ny", nyap-1);
          id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec", "tile_spec_version", 
			tile_spec_version, "geometry", geometry, "discretization", discretization, "conformal", conformal );
          dims[0] = dimlist[2]; dims[1] = dimlist[1];
          id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude", "units", "degree_east");
          id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude", "units", "degree_north");

          /* Following variables are not defined */
          dims[0] = dimlist[2]; dims[1] = dimlist[3];
          id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance", "units", "meters");
          dims[0] = dimlist[4]; dims[1] = dimlist[1];
          id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance", "units", "meters");
          dims[0] = dimlist[2]; dims[1] = dimlist[1];
          id_angle_dx= mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_vertex_x_angle_WRT_geographic_east",
                             "units", "degrees_east");
          dims[0] = dimlist[4]; dims[1] = dimlist[3];
          id_angle_dx= mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area", "units", "m2");
	  /* ------------------------------------------- */
          mpp_end_def(fid);
	  for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 0; }
	  nwrite[0] = strlen(tilename);
          mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );
          mpp_put_var_value(fid, id_x, x_atmos);
          mpp_put_var_value(fid, id_y, y_atmos);
          mpp_close(fid);
  }
  free(xba); free(yba); free(xta); free(yta);  free(x_atmos); free(y_atmos);
}
// ----------------------------------------------------------------]

//.................. Land .......................
//..................land_hgrid.nc................
 int nxbl, nybl, nxtl, nytl,  nxlp, nylp;
if(is_coupled_grid){
  double *x_land, *y_land;
  double *xbl, *ybl, *xtl, *ytl;
  int i,j, fid_old, vid;

  fid_old = mpp_open(old_file, MPP_READ);
  
  nxbl = mpp_get_dimlen(fid_old, "xbl");
  nybl = mpp_get_dimlen(fid_old, "ybl");
  nxtl = mpp_get_dimlen(fid_old, "xtl");
  nytl = mpp_get_dimlen(fid_old, "ytl");
  xbl = (double *) malloc(nxbl * sizeof(double)); 
  ybl = (double *) malloc(nybl * sizeof(double)); 
  xtl = (double *) malloc(nxtl * sizeof(double)); 
  ytl = (double *) malloc(nytl * sizeof(double)); 
  vid = mpp_get_varid(fid_old, "xbl"); mpp_get_var_value(fid_old, vid, xbl);
  vid = mpp_get_varid(fid_old, "ybl"); mpp_get_var_value(fid_old, vid, ybl);
  vid = mpp_get_varid(fid_old, "xtl"); mpp_get_var_value(fid_old, vid, xtl);
  vid = mpp_get_varid(fid_old, "ytl"); mpp_get_var_value(fid_old, vid, ytl);
  mpp_close(fid_old);
  
  nxlp = nxbl + nxtl; nylp = nybl + nytl; 
  x_land  = (double *) malloc(nxlp*nylp * sizeof(double));
  y_land  = (double *) malloc(nxlp*nylp * sizeof(double));
  
  for(i = 0; i<nxbl; i++) x_land[2*i]   = xbl[i]; 
  for(i = 0; i<nxtl; i++) x_land[2*i+1]   = xtl[i]; 

  for(j = 0; j<nybl; j++) y_land[2*j*nxlp]   = ybl[j]; 
  for(j = 0; j<nytl; j++) y_land[(2*j+1)*nxlp]   = ytl[j]; 

  for(j = 1; j<nylp; j++) for(i = 0; i<nxlp; i++) x_land[j*nxlp + i]   = x_land[i];

  for(i = 1; i<nxlp; i++) 
	for(j = 0; j<nylp; j++) 
		y_land[j*nxlp + i]   = y_land[j*nxlp];

  /* write Land-grid data */
  {
    int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
    int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp;
    char tile_spec_version[] = "0.2", geometry[] = "spherical", discretization[] = "logically_rectangular", conformal[]="true";
    char outfile[128] = "", tilename[128]="";
    size_t start[4], nwrite[4];
    
          sprintf(outfile, "%s_hgrid.nc",land_name);
	  printf("Writing %s\n", outfile);
          printf("\t nxbl=%d, nybl=%d, nxtl=%d, nytl=%d \n", nxbl, nybl,nxtl, nytl); 

          sprintf(tilename, "tile%d", 1);
        fid = mpp_open(outfile, MPP_WRITE);
          dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
          dimlist[1] = mpp_def_dim(fid, "nxp", nxlp);
          dimlist[2] = mpp_def_dim(fid, "nyp", nylp);
          dimlist[3] = mpp_def_dim(fid, "nx", nxlp-1);
          dimlist[4] = mpp_def_dim(fid, "ny", nylp-1);
          id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec", "tile_spec_version", 
			tile_spec_version, "geometry", geometry, "discretization", discretization, "conformal", conformal );
          dims[0] = dimlist[2]; dims[1] = dimlist[1];
          id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude", "units", "degree_east");
          id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude", "units", "degree_north");

          /* Following variables are not defined */
          dims[0] = dimlist[2]; dims[1] = dimlist[3];
          id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance", "units", "meters");
          dims[0] = dimlist[4]; dims[1] = dimlist[1];
          id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance", "units", "meters");
          dims[0] = dimlist[2]; dims[1] = dimlist[1];
          id_angle_dx= mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_vertex_x_angle_WRT_geographic_east",
                             "units", "degrees_east");
          dims[0] = dimlist[4]; dims[1] = dimlist[3];
          id_angle_dx= mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area", "units", "m2");
          /* ------------------------------------------- */

          mpp_end_def(fid);
	  for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 0; }
	  nwrite[0] = strlen(tilename);
          mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );
          mpp_put_var_value(fid, id_x, x_land);
          mpp_put_var_value(fid, id_y, y_land);
          mpp_close(fid);
  }
  free(xbl); free(ybl); free(xtl); free(ytl);  free(x_land); free(y_land);
}
//-------------------------------ocean_vgrid.nc----------------------------------
{ 
  double *z, *zb, *zt;
  int nzb, nzt, nz, i,j, fid_old, vid;

  fid_old = mpp_open(old_file, MPP_READ);
  nzb = mpp_get_dimlen(fid_old, "zb");
  nzt = mpp_get_dimlen(fid_old, "zt");
  zb = (double *) malloc(nzb * sizeof(double)); 
  zt = (double *) malloc(nzt * sizeof(double)); 
  vid = mpp_get_varid(fid_old, "zb"); mpp_get_var_value(fid_old, vid, zb);
  vid = mpp_get_varid(fid_old, "zt"); mpp_get_var_value(fid_old, vid, zt);
  mpp_close(fid_old);
  
  nz = nzb + nzt + 1; 
  z = (double *) malloc(nz * sizeof(double));
  z[0]=0.0; 
  for(i = 0; i<nzt; i++) { z[2*i + 1]   = zt[i]; z[2*i+2]   = zb[i]; } 

  {
    int fid, dim, varid;
    char outfile[128] = "";

    	sprintf(outfile, "%s_vgrid.nc",ocean_name);
	printf("Writing %s\n", outfile);
        printf("\t nzb=%d, nzt=%d\n", nzb, nzt); 

    	fid = mpp_open(outfile, MPP_WRITE);
    	  dim  = mpp_def_dim(fid, "nzv", nz);
    	  varid = mpp_def_var(fid, "zeta", MPP_DOUBLE, 1, &dim, 2, "standard_name", "vertical_grid_vertex", "units", "meters");
    	  mpp_end_def(fid);
    	  mpp_put_var_value(fid, varid, z);
    	mpp_close(fid);
  }
  free(zb); free(zt); free(z);
}


//-------------------------------topog.nc----------------------------------
{ 
  double *depth_t;
  int nx, ny, ntiles, i,j, fid_old, vid;

  fid_old = mpp_open(old_file, MPP_READ);  
  nx= mpp_get_dimlen(fid_old, "grid_x_T");
  ny= mpp_get_dimlen(fid_old, "grid_y_T");
  depth_t= (double *) malloc(nx*ny* sizeof(double));
  vid = mpp_get_varid(fid_old, "depth_t");  mpp_get_var_value(fid_old, vid, depth_t);
  mpp_close(fid_old);
  
  {
    int fid, dim[2], varid, ntiles = 1, dim_ntile;
    char outfile[128] = "";
        sprintf(outfile, "topog.nc");
	printf("Writing %s\n", outfile);
        printf("\t nx=%d, ny=%d\n", nx, ny);
        fid = mpp_open(outfile, MPP_WRITE);
	dim_ntile = mpp_def_dim(fid, "ntiles", 1);
          dim[1]  = mpp_def_dim(fid, "nx", nx);
          dim[0]  = mpp_def_dim(fid, "ny", ny);
          varid = mpp_def_var(fid, "depth", MPP_DOUBLE, 2, dim, 2, "standard_name", "topographic depth at T-cell centers", "units", "meters");
          mpp_end_def(fid);
          mpp_put_var_value(fid, varid, depth_t);
        mpp_close(fid);
  }
  free(depth_t); 
}

//................atmos_mosaic.nc............................
 if(is_coupled_grid)  {
    char str[STRING], outfile[STRING];
    int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
    int id_contact_index, id_griddir, id_gridfiles, dim[2];

    size_t start[4], nwrite[4];

//--------Initilize values --------------
    ntiles = 1, ncontact = 0; 

    sprintf(mosaic_name, "%s_mosaic",atmos_name);
    sprintf(outfile, "%s.nc", mosaic_name);
    printf("Writing %s\n", outfile);

    sprintf(tile_name[0], "tile%d", 1);
    sprintf(tilefile[0], "%s_hgrid.nc",atmos_name);
    fid = mpp_open(outfile, MPP_WRITE);

    contact_tile1_istart[0]=nxba-1; contact_tile1_iend[0]=nxba-1; 
    contact_tile1_jstart[0]=1; contact_tile1_jend[0]=nyba-1; 

    contact_tile2_istart[0]=1; contact_tile2_iend[0]=1; 
    contact_tile2_jstart[0]=1; contact_tile2_jend[0]=nyba-1;
//--------Initilize values --------------

    dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
    if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
    dim_string = mpp_def_dim(fid, "string", STRING);

    id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 5, "standard_name",
                            "grid_mosaic_spec", "mosaic_spec_version", mosaic_version,
                            "children", "gridtiles", "contact_regions", "contacts",
                            "grid_descriptor", grid_descriptor);
    dim[0] = dim_ntiles; dim[1] = dim_string;
    id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
                               "standard_name", "grid_file_location");
    id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
    id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

    if(ncontact>0) {
      dim[0] = dim_ncontact; dim[1] = dim_string;
      id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
                                "contact_type", "boundary", "alignment", "true",
                                "contact_index", "contact_index", "orientation", "orient");
      id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
                                     "starting_ending_point_index_of_contact");

    }

    //mpp_def_global(fid, "history", history);
    mpp_end_def(fid);

    /* write out data */
    for(i=0; i<4; i++) {
      start[i] = 0; nwrite[i] = 1;
    }
    nwrite[0] = strlen(mosaic_name);
    mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_griddir, start, nwrite, mosaic_dir);
    nwrite[0] = 1;
    for(n=0; n<ntiles; n++) {
      start[0] = n; nwrite[1] = strlen(tile_name[n]);
      mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
      nwrite[1] = strlen(tilefile[n]);
      mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
    }

    for(n=0; n<ncontact; n++) {
      sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
              tile_name[contact_tile2[n]]);
      start[0] = n; nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
      sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
              contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
              contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
      nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
    }
    mpp_close(fid);
  }

//................land_mosaic.nc............................
 if(is_coupled_grid)  {
    char str[STRING], outfile[STRING];
    int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
    int id_contact_index, id_griddir, id_gridfiles, dim[2];

    size_t start[4], nwrite[4];

//--------Initilize values --------------
    ntiles = 1, ncontact = 1; 
    contact_tile1_istart[1]=1; contact_tile1_iend[1]=1;
    contact_tile1_jstart[1]=1; contact_tile1_jend[1]=nytl;

    contact_tile2_istart[1]=nxtl; contact_tile2_iend[1]=nxtl;
    contact_tile2_jstart[1]=1;    contact_tile2_jend[1]=nytl;
    
    sprintf(mosaic_name, "%s_mosaic",land_name);
    sprintf(outfile, "%s.nc", mosaic_name);
    printf("Writing %s\n", outfile);

    sprintf(tile_name[0], "tile%d", 1);
    sprintf(tilefile[0], "%s_hgrid.nc",land_name);
    fid = mpp_open(outfile, MPP_WRITE);

    contact_tile1_istart[0]=nxbl-1; contact_tile1_iend[0]=nxbl-1; 
    contact_tile1_jstart[0]=1; contact_tile1_jend[0]=nybl-1; 

    contact_tile2_istart[0]=1; contact_tile2_iend[0]=1; 
    contact_tile2_jstart[0]=1; contact_tile2_jend[0]=nybl-1;
//--------Initilize values --------------

    dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
    if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
    dim_string = mpp_def_dim(fid, "string", STRING);

    id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 5, "standard_name",
                            "grid_mosaic_spec", "mosaic_spec_version", mosaic_version,
                            "children", "gridtiles", "contact_regions", "contacts",
                            "grid_descriptor", grid_descriptor);
    dim[0] = dim_ntiles; dim[1] = dim_string;
    id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
                               "standard_name", "grid_file_location");
    id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
    id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

    if(ncontact>0) {
      dim[0] = dim_ncontact; dim[1] = dim_string;
      id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
                                "contact_type", "boundary", "alignment", "true",
                                "contact_index", "contact_index", "orientation", "orient");
      id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
                                     "starting_ending_point_index_of_contact");

    }

    //mpp_def_global(fid, "history", history);
    mpp_end_def(fid);

    /* write out data */
    for(i=0; i<4; i++) {
      start[i] = 0; nwrite[i] = 1;
    }
    nwrite[0] = strlen(mosaic_name);
    mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_griddir, start, nwrite, mosaic_dir);
    nwrite[0] = 1;
    for(n=0; n<ntiles; n++) {
      start[0] = n; nwrite[1] = strlen(tile_name[n]);
      mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
      nwrite[1] = strlen(tilefile[n]);
      mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
    }

    for(n=0; n<ncontact; n++) {
      sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
              tile_name[contact_tile2[n]]);
      start[0] = n; nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
      sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
              contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
              contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
      nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
    }
    mpp_close(fid);
  }

/* ...................... ocean_mosaic.nc .................*/
  {
    char str[STRING], outfile[STRING], x_boundary_type1[255]="", y_boundary_type1[255]="";
    int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
    int id_contact_index, id_griddir, id_gridfiles, dim[2], itmp, fid_old;

    size_t start[4], nwrite[4];

//--------Initilize values --------------
    ntiles = 1, ncontact = 0; 

    sprintf(mosaic_name, "%s_mosaic",ocean_name);
    sprintf(outfile, "%s.nc", mosaic_name);
    printf("Writing %s\n", outfile);

    sprintf(tile_name[0], "tile%d", 1);
    sprintf(tilefile[0], "%s_hgrid.nc", ocean_name);
    
    /* ........... Read Boundary Type ................ */
    fid_old = mpp_open(old_file, MPP_READ);
    mpp_get_global_att(fid_old, "x_boundary_type",x_boundary_type1);
    mpp_get_global_att(fid_old, "y_boundary_type",y_boundary_type1);
    mpp_close(fid_old);
    if (strcmp(x_boundary_type1, "cyclic")==0) {
	n = ncontact; ncontact = ncontact + 1; 
    	contact_tile1_istart[n]=360; contact_tile1_iend[n]=360; 
    	contact_tile1_jstart[n]=1; contact_tile1_jend[n]=180; 

    	contact_tile2_istart[n]=1; contact_tile2_iend[n]=1; 
    	contact_tile2_jstart[n]=1; contact_tile2_jend[n]=180;
	printf("\t x_boundary_type = %s, ncontact=%d \n",x_boundary_type1,ncontact);
    }
    
    if (strcmp(y_boundary_type1, "fold_north_edge")==0) {
	n = ncontact; ncontact = ncontact + 1; 
        contact_tile1_istart[n]=1; contact_tile1_iend[n]=180;
        contact_tile1_jstart[n]=180; contact_tile1_jend[n]=180;

        contact_tile2_istart[n]=360; contact_tile2_iend[n]=181;
        contact_tile2_jstart[n]=180; contact_tile2_jend[n]=180;
	printf("\t y_boundary_type = %s, ncontact=%d \n",y_boundary_type1,ncontact);
    }

    fid = mpp_open(outfile, MPP_WRITE);

//--------Initilize values --------------

    dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
    if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
    dim_string = mpp_def_dim(fid, "string", STRING);

    id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 5, "standard_name",
                            "grid_mosaic_spec", "mosaic_spec_version", mosaic_version,
                            "children", "gridtiles", "contact_regions", "contacts",
                            "grid_descriptor", grid_descriptor);
    dim[0] = dim_ntiles; dim[1] = dim_string;
    id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
                               "standard_name", "grid_file_location");
    id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
    id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

    if(ncontact>0) {
      dim[0] = dim_ncontact; dim[1] = dim_string;
      id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
                                "contact_type", "boundary", "alignment", "true",
                                "contact_index", "contact_index", "orientation", "orient");
      id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
                                     "starting_ending_point_index_of_contact");

    }

    //mpp_def_global(fid, "history", history);
    mpp_end_def(fid);

    /* write out data */
    for(i=0; i<4; i++) {
      start[i] = 0; nwrite[i] = 1;
    }
    nwrite[0] = strlen(mosaic_name);
    mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_griddir, start, nwrite, mosaic_dir);
    nwrite[0] = 1;
    for(n=0; n<ntiles; n++) {
      start[0] = n; nwrite[1] = strlen(tile_name[n]);
      mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
      nwrite[1] = strlen(tilefile[n]);
      mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
    }

    for(n=0; n<ncontact; n++) {
      sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
              tile_name[contact_tile2[n]]);
      start[0] = n; nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
      sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
              contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
              contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
      nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
    }
    mpp_close(fid);
  }
/* ...................... atmosXland.nc .................*/
  if(is_coupled_grid)  {
  const char version[] = "0.2";
  char lxo_file[STRING], axo_file[STRING], axl_file[STRING];
  char amosaic_file[STRING], lmosaic_file[STRING], omosaic_file[STRING];
  char otopog_file[STRING];

  char atile_name[255], ltile_name[255], otile_name[255];
  char amosaic_name[255], lmosaic_name[255], omosaic_name[255];
  int  fid_old, vid;
  
  strcpy(atile_name,"tile1"); strcpy(ltile_name,"tile1"); strcpy(otile_name,"tile1");
  strcpy(amosaic_name,"atmos"); strcpy(lmosaic_name,"land"); strcpy(omosaic_name,"ocean");

//---------------
    int no, nl, na, n;
    size_t  naxl, naxo, naxl_step;
    int     *atmxlnd_ia, *atmxlnd_ja, *atmxlnd_il, *atmxlnd_jl;
    int     *atmxocn_ia, *atmxocn_ja, *atmxocn_io, *atmxocn_jo;
    int     *lndxocn_il, *lndxocn_jl, *lndxocn_io, *lndxocn_jo;
    double  *atmxlnd_area, *atmxlnd_di, *atmxlnd_dj;
    double  *atmxocn_area, *atmxocn_di, *atmxocn_dj;
    double  *lndxocn_area, *lndxocn_di, *lndxocn_dj;

    fid_old = mpp_open(old_file, MPP_READ);
    naxl = mpp_get_dimlen(fid_old, "i_atmXlnd");
//----------------
 
      /* write out atmXlnd data*/
        if(naxl>0) {
          size_t start[4], nwrite[4];
                                                                                                                                                          
          int fid, dim_string, dim_ncells, dim_two, dims[4];
          int id_xgrid_area, id_contact, n;
          int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
          char contact[STRING];
                                                                                                                                                          
          for(i=0; i<4; i++) {
            start[i] = 0; nwrite[i] = 1;
          }
          sprintf(axl_file, "%s_mosaicX%s_mosaic.nc", amosaic_name, lmosaic_name);
          printf("Writing %s_mosaicX%s_mosaic.nc\n", amosaic_name, lmosaic_name);
          sprintf(contact, "%s_mosaic:%s::%s_mosaic:%s", amosaic_name, atile_name, lmosaic_name, ltile_name);
          fid = mpp_open(axl_file, MPP_WRITE);
          //mpp_def_global(fid, "history", history);
          dim_string = mpp_def_dim(fid, "string", STRING);
          dim_ncells = mpp_def_dim(fid, "ncells", naxl);
          dim_two    = mpp_def_dim(fid, "two", 2);
          if(interp_order == 2) {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 8, "standard_name", "grid_contact_spec",
                                     "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
                                     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area",
                                     "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
          }
          else {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 6, "standard_name", "grid_contact_spec",
                                     "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
                                     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
          }
                                                                                                                                                          
          dims[0] = dim_ncells; dims[1] = dim_two;
          id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
          id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
          id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
                                       "exchange_grid_area", "units", "m2");
          if(interp_order == 2) {
            id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
            id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
          }
          mpp_end_def(fid);

	  for(i=0; i<4; i++) {start[i] = 0; nwrite[i] = 1;}
	  nwrite[0] = strlen(contact);
          mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
          nwrite[0] = naxl;

          atmxlnd_area = (double *)malloc(naxl*sizeof(double)); 
          atmxlnd_ia   = (int    *)malloc(naxl*sizeof(int   )); 
          atmxlnd_ja   = (int    *)malloc(naxl*sizeof(int   )); 
          atmxlnd_il   = (int    *)malloc(naxl*sizeof(int   )); 
          atmxlnd_jl   = (int    *)malloc(naxl*sizeof(int   )); 
	  vid = mpp_get_varid(fid_old, "AREA_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_area);
	  vid = mpp_get_varid(fid_old, "I_ATM_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_ia);
	  vid = mpp_get_varid(fid_old, "J_ATM_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_ja);
	  vid = mpp_get_varid(fid_old, "I_LND_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_il);
	  vid = mpp_get_varid(fid_old, "J_LND_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_jl);
	  
	  for(i=0;i<naxl;i++)atmxlnd_area[i] = atmxlnd_area[i] * 4.0 * PI * RADIUS * RADIUS; 

          mpp_put_var_value(fid, id_xgrid_area, atmxlnd_area);
          mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxlnd_ia);
          mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxlnd_il);
          start[1] = 1;
          mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxlnd_ja);
          mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxlnd_jl);
          if(interp_order == 2) {
            atmxlnd_di = (double *)malloc(naxl*sizeof(double)); 
            atmxlnd_dj = (double *)malloc(naxl*sizeof(double)); 
	    vid = mpp_get_varid(fid_old, "DI_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_di);
	    vid = mpp_get_varid(fid_old, "DJ_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_dj);
            start[1] = 0;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxlnd_di);
            start[1] = 1;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxlnd_dj);
          }
	  
          mpp_close(fid);
          free(atmxlnd_area);
        }
	mpp_close(fid_old);
  }
/* ...................... atmosXocean.nc .................*/
  if(is_coupled_grid)  {
  const char version[] = "0.2";
  char lxo_file[STRING];
  char axo_file[STRING];
  char axl_file[STRING];
  char amosaic_file[STRING];
  char lmosaic_file[STRING];
  char omosaic_file[STRING];
  char otopog_file[STRING];

  char atile_name[255], ltile_name[255], otile_name[255];
  char amosaic_name[255], lmosaic_name[255], omosaic_name[255];

  strcpy(atile_name,"tile1"); strcpy(ltile_name,"tile1"); strcpy(otile_name,"tile1");
  strcpy(amosaic_name,"atmos"); strcpy(lmosaic_name,"land"); strcpy(omosaic_name,"ocean");

//---------------
    int no, nl, na, n;
    size_t  naxl, naxo, naxl_step;
    int     *atmxlnd_ia, *atmxlnd_ja, *atmxlnd_il, *atmxlnd_jl;
    int     *atmxocn_ia, *atmxocn_ja, *atmxocn_io, *atmxocn_jo;
    int     *lndxocn_il, *lndxocn_jl, *lndxocn_io, *lndxocn_jo;
    double  *atmxlnd_area, *atmxlnd_di, *atmxlnd_dj;
    double  *atmxocn_area, *atmxocn_di, *atmxocn_dj;
    double  *lndxocn_area, *lndxocn_di, *lndxocn_dj;
    int fid_old, vid;

    fid_old = mpp_open(old_file, MPP_READ);
    naxo = mpp_get_dimlen(fid_old, "i_atmXocn");
//----------------
 
      /* write out atmXlnd data*/
        if(naxo>0) {
          size_t start[4], nwrite[4];
                                                                                                                                                          
          int fid, dim_string, dim_ncells, dim_two, dims[4];
          int id_xgrid_area, id_contact, n;
          int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
          char contact[STRING];
                                                                                                                                                          
          for(i=0; i<4; i++) {
            start[i] = 0; nwrite[i] = 1;
          }
          sprintf(axo_file, "%s_mosaicX%s_mosaic.nc", amosaic_name, omosaic_name);
          printf("Writing %s_mosaicX%s_mosaic.nc\n", amosaic_name, omosaic_name);
          sprintf(contact, "%s_mosaic:%s::%s_mosaic:%s", amosaic_name, atile_name, omosaic_name, otile_name);
          fid = mpp_open(axo_file, MPP_WRITE);
          //mpp_def_global(fid, "history", history);
          dim_string = mpp_def_dim(fid, "string", STRING);
          dim_ncells = mpp_def_dim(fid, "ncells", naxo);
          dim_two    = mpp_def_dim(fid, "two", 2);
          if(interp_order == 2) {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 8, "standard_name", "grid_contact_spec",
                                     "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
                                     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area",
                                     "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
          }
          else {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 6, "standard_name", "grid_contact_spec",
                                     "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
                                     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
          }
                                                                                                                                                          
          dims[0] = dim_ncells; dims[1] = dim_two;
          id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
          id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
          id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
                                       "exchange_grid_area", "units", "m2");
          if(interp_order == 2) {
            id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
            id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
          }
          mpp_end_def(fid);

	  for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1;}
	  nwrite[0] = strlen(contact);
          mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
          nwrite[0] = naxo;

          atmxocn_area = (double *)malloc(naxo*sizeof(double)); 
          atmxocn_ia   = (int    *)malloc(naxo*sizeof(int   )); 
          atmxocn_ja   = (int    *)malloc(naxo*sizeof(int   )); 
          atmxocn_io   = (int    *)malloc(naxo*sizeof(int   )); 
          atmxocn_jo   = (int    *)malloc(naxo*sizeof(int   )); 
	  vid = mpp_get_varid(fid_old, "AREA_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_area);
	  vid = mpp_get_varid(fid_old, "I_ATM_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_ia);
	  vid = mpp_get_varid(fid_old, "J_ATM_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_ja);
	  vid = mpp_get_varid(fid_old, "I_OCN_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_io);
	  vid = mpp_get_varid(fid_old, "J_OCN_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_jo);
	  
          for(i=0;i<naxo;i++)atmxocn_area[i] = atmxocn_area[i] * 4.0 * PI * RADIUS * RADIUS; 

          mpp_put_var_value(fid, id_xgrid_area, atmxocn_area);
          mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxocn_ia);
          mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxocn_io);
          start[1] = 1;
          mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxocn_ja);
          mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxocn_jo);
          if(interp_order == 2) {
            atmxocn_di = (double *)malloc(naxo*sizeof(double)); 
            atmxocn_dj = (double *)malloc(naxo*sizeof(double)); 
	    vid = mpp_get_varid(fid_old, "DI_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_di);
	    vid = mpp_get_varid(fid_old, "DJ_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_dj);
            start[1] = 0;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxocn_di);
            start[1] = 1;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxocn_dj);
          }
          mpp_close(fid);
          free(atmxocn_area);
        }
	mpp_close(fid_old);
  }
/* ...................... landXocean.nc .................*/
  if(is_coupled_grid)  {
  const char version[] = "0.2";
  char lxo_file[STRING];
  char axo_file[STRING];
  char axl_file[STRING];
  char amosaic_file[STRING];
  char lmosaic_file[STRING];
  char omosaic_file[STRING];
  char otopog_file[STRING];

  char atile_name[255], ltile_name[255], otile_name[255];
  char amosaic_name[255], lmosaic_name[255], omosaic_name[255];

  strcpy(atile_name,"tile1"); strcpy(ltile_name,"tile1"); strcpy(otile_name,"tile1");
  strcpy(amosaic_name,"atmos"); strcpy(lmosaic_name,"land"); strcpy(omosaic_name,"ocean");

//---------------
    int no, nl, na, n;
    size_t  naxl, naxo, nlxo, naxl_step;
    int     *atmxlnd_ia, *atmxlnd_ja, *atmxlnd_il, *atmxlnd_jl;
    int     *atmxocn_ia, *atmxocn_ja, *atmxocn_io, *atmxocn_jo;
    int     *lndxocn_il, *lndxocn_jl, *lndxocn_io, *lndxocn_jo;
    double  *atmxlnd_area, *atmxlnd_di, *atmxlnd_dj;
    double  *atmxocn_area, *atmxocn_di, *atmxocn_dj;
    double  *lndxocn_area, *lndxocn_di, *lndxocn_dj;
    int     fid_old, vid;

    fid_old = mpp_open(old_file, MPP_READ);
    nlxo = mpp_get_dimlen(fid_old , "i_lndXocn");
//----------------
 
      /* write out lndXocn data*/
        if(nlxo>0) {
          size_t start[4], nwrite[4];
                                                                                                                                                          
          int fid, dim_string, dim_ncells, dim_two, dims[4];
          int id_xgrid_area, id_contact, n;
          int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
          char contact[STRING];
                                                                                                                                                          
          for(i=0; i<4; i++) {
            start[i] = 0; nwrite[i] = 1;
          }
          sprintf(lxo_file, "%s_mosaicX%s_mosaic.nc", lmosaic_name, omosaic_name);
          printf("Writing %s_mosaicX%s_mosaic.nc\n", lmosaic_name, omosaic_name);
          sprintf(contact, "%s_mosaic:%s::%s_mosaic:%s", lmosaic_name, ltile_name, omosaic_name, otile_name);
          fid = mpp_open(lxo_file, MPP_WRITE);
          //mpp_def_global(fid, "history", history);
          dim_string = mpp_def_dim(fid, "string", STRING);
          dim_ncells = mpp_def_dim(fid, "ncells", nlxo);
          dim_two    = mpp_def_dim(fid, "two", 2);
          if(interp_order == 2) {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 8, "standard_name", "grid_contact_spec",
                                     "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
                                     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area",
                                     "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
          }
          else {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 6, "standard_name", "grid_contact_spec",
                                     "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
                                     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
          }
                                                                                                                                                          
          dims[0] = dim_ncells; dims[1] = dim_two;
          id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
          id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
          id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
                                       "exchange_grid_area", "units", "m2");
          if(interp_order == 2) {
            id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
            id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
          }
          mpp_end_def(fid);

	  for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1;}
	  nwrite[0] = strlen(contact);
          mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
          nwrite[0] = nlxo;

          lndxocn_area = (double *)malloc(nlxo*sizeof(double)); 
          lndxocn_il   = (int    *)malloc(nlxo*sizeof(int   )); 
          lndxocn_jl   = (int    *)malloc(nlxo*sizeof(int   )); 
          lndxocn_io   = (int    *)malloc(nlxo*sizeof(int   )); 
          lndxocn_jo   = (int    *)malloc(nlxo*sizeof(int   )); 
	  vid = mpp_get_varid(fid_old, "AREA_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_area);
	  vid = mpp_get_varid(fid_old, "I_LND_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_il);
	  vid = mpp_get_varid(fid_old, "J_LND_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_jl);
	  vid = mpp_get_varid(fid_old, "I_OCN_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_io);
	  vid = mpp_get_varid(fid_old, "J_OCN_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_jo);
	  
          for(i=0;i<nlxo;i++)lndxocn_area[i] = lndxocn_area[i] * 4.0 * PI * RADIUS * RADIUS; 

          mpp_put_var_value(fid, id_xgrid_area, lndxocn_area);
          mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, lndxocn_il);
          mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, lndxocn_io);
          start[1] = 1;
          mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, lndxocn_jl);
          mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, lndxocn_jo);
          if(interp_order == 2) {
            lndxocn_di = (double *)malloc(nlxo*sizeof(double)); 
            lndxocn_dj = (double *)malloc(nlxo*sizeof(double)); 
	    vid = mpp_get_varid(fid_old, "DI_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_di);
	    vid = mpp_get_varid(fid_old, "DJ_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_dj);
            start[1] = 0;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, lndxocn_di);
            start[1] = 1;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, lndxocn_dj);
          }
          mpp_close(fid);
          free(lndxocn_area);
        }
	mpp_close(fid_old);
  }

/* ...................... mosaic.nc .................*/
  {
  const char version[] = "0.2";
  char lxo_file[STRING], axo_file[STRING], axl_file[STRING], mosaic_file[STRING];
  char amosaic_file[STRING], lmosaic_file[STRING], omosaic_file[STRING];
  char amosaic_name[STRING], lmosaic_name[STRING], omosaic_name[STRING];
  char otopog_file[STRING];

    int fid, dim_string, dim_axo, dim_lxo, dim_axl, dims[4], n;
    size_t start[4], nwrite[4];
    int id_lmosaic_dir, id_lmosaic_file, id_omosaic_dir, id_omosaic_file;
    int id_amosaic_dir, id_amosaic_file, id_otopog_dir, id_otopog_file;
    int id_xgrids_dir, id_axo_file, id_lxo_file, id_axl_file;
    int id_amosaic, id_lmosaic, id_omosaic;

    int nfile_axo = 1, nfile_axl = 1, nfile_lxo = 1; 

    strcpy(mosaic_file,"mosaic.nc");
    printf("Writing %s\n", mosaic_file);

    fid = mpp_open(mosaic_file, MPP_WRITE);
  //  mpp_def_global(fid, "history", history);
    dim_string = mpp_def_dim(fid, "string", STRING);
    dim_axo = mpp_def_dim(fid, "nfile_aXo", nfile_axo);
    dim_axl = mpp_def_dim(fid, "nfile_aXl", nfile_axl);
    dim_lxo = mpp_def_dim(fid, "nfile_lXo", nfile_lxo);
    id_amosaic_dir  = mpp_def_var(fid, "atm_mosaic_dir", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "directory_storing_atmosphere_mosaic");
    id_amosaic_file = mpp_def_var(fid, "atm_mosaic_file", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "atmosphere_mosaic_file_name");
    id_amosaic      = mpp_def_var(fid, "atm_mosaic", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "atmosphere_mosaic_name");
    id_lmosaic_dir  = mpp_def_var(fid, "lnd_mosaic_dir", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "directory_storing_land_mosaic");
    id_lmosaic_file = mpp_def_var(fid, "lnd_mosaic_file", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "land_mosaic_file_name");
    id_lmosaic      = mpp_def_var(fid, "lnd_mosaic", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "land_mosaic_name");
    id_omosaic_dir  = mpp_def_var(fid, "ocn_mosaic_dir", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "directory_storing_ocean_mosaic");
    id_omosaic_file = mpp_def_var(fid, "ocn_mosaic_file", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "ocean_mosaic_file_name");
    id_omosaic      = mpp_def_var(fid, "ocn_mosaic", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "ocean_mosaic_name");
    id_otopog_dir   = mpp_def_var(fid, "ocn_topog_dir", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "directory_storing_ocean_topog");
    id_otopog_file  = mpp_def_var(fid, "ocn_topog_file", MPP_CHAR, 1, &dim_string,
                                  1, "standard_name", "ocean_topog_file_name");
    /* since exchange grid is created in this tool, we may add command line option to specify where to store the output */
    /*    id_xgrids_dir = mpp_def_var(fid, "xgrids_dir", MPP_CHAR, 1, &dim_string,
          1, "standard_name", "directory_storing_xgrids"); */
    dims[0] = dim_axo; dims[1] = dim_string;
    id_axo_file = mpp_def_var(fid, "aXo_file", MPP_CHAR, 2, dims, 1, "standard_name", "atmXocn_exchange_grid_file");
    dims[0] = dim_axl; dims[1] = dim_string;
    id_axl_file = mpp_def_var(fid, "aXl_file", MPP_CHAR, 2, dims, 1, "standard_name", "atmXlnd_exchange_grid_file");
    dims[0] = dim_lxo; dims[1] = dim_string;
    id_lxo_file = mpp_def_var(fid, "lXo_file", MPP_CHAR, 2, dims, 1, "standard_name", "lndXocn_exchange_grid_file");
    mpp_end_def(fid);

strcpy(otopog_file,"topog.nc");

sprintf(amosaic_name,"%s_mosaic",atmos_name); sprintf(lmosaic_name,"%s_mosaic",land_name); sprintf(omosaic_name,"%s_mosaic",ocean_name);
sprintf(amosaic_file,"%s.nc",amosaic_name); sprintf(lmosaic_file,"%s.nc",lmosaic_name); sprintf(omosaic_file,"%s.nc",omosaic_name);

sprintf(axl_file,"%sX%s.nc", amosaic_name, lmosaic_name); 
sprintf(axo_file,"%sX%s.nc", amosaic_name, omosaic_name); 
sprintf(lxo_file,"%sX%s.nc", lmosaic_name, omosaic_name); 

    for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1; }
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_amosaic_dir, start, nwrite, mosaic_dir);
    nwrite[0] = strlen(amosaic_file);
    mpp_put_var_value_block(fid, id_amosaic_file, start, nwrite, amosaic_file);
    nwrite[0] = strlen(amosaic_name);
    mpp_put_var_value_block(fid, id_amosaic, start, nwrite, amosaic_name);
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_lmosaic_dir, start, nwrite, mosaic_dir);
    nwrite[0] = strlen(lmosaic_file);
    mpp_put_var_value_block(fid, id_lmosaic_file, start, nwrite, lmosaic_file);
    nwrite[0] = strlen(lmosaic_name);
    mpp_put_var_value_block(fid, id_lmosaic, start, nwrite, lmosaic_name);
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_omosaic_dir, start, nwrite, mosaic_dir);
    nwrite[0] = strlen(omosaic_file);
    mpp_put_var_value_block(fid, id_omosaic_file, start, nwrite, omosaic_file);
    nwrite[0] = strlen(omosaic_name);
    mpp_put_var_value_block(fid, id_omosaic, start, nwrite, omosaic_name);
    nwrite[0] = strlen(mosaic_dir);
    mpp_put_var_value_block(fid, id_otopog_dir, start, nwrite, mosaic_dir);
    nwrite[0] = strlen(otopog_file);
    mpp_put_var_value_block(fid, id_otopog_file, start, nwrite, otopog_file);
    for(i=0; i<4; i++) {
      start[i] = 0; nwrite[i] = 1;
    }
      start[0] = 0; nwrite[1] = strlen(axo_file);
      mpp_put_var_value_block(fid, id_axo_file, start, nwrite, axo_file);

      start[0] = 0; nwrite[1] = strlen(axl_file);
      mpp_put_var_value_block(fid, id_axl_file, start, nwrite, axl_file);

      start[0] = 0; nwrite[1] = strlen(lxo_file);
      mpp_put_var_value_block(fid, id_lxo_file, start, nwrite, lxo_file);
    mpp_close(fid);
  }



  return;
  mpp_end();
  
};  /* end of main */

