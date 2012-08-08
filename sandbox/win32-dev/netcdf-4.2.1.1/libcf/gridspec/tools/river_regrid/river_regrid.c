#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "mpp.h"
#include "mpp_io.h"
#include "create_xgrid.h"
#include "constant.h"
#include "mosaic_util.h"
#include "tool_util.h"

char *usage[] = {
  "",
  "  river_regrid --mosaic mosaic_grid --river_src river_src_file [--output output_file] ",
  "                                                                                    ",
  "river_regrid will remap river network data from global regular lat-lon grid onto any  ",
  "other grid (includes regular lat-lon grid and cubic grid ), which is specified        ",
  "through option --mosaic. The river network source data is specified through option    ",
  "--river_src.                                                                          ",
  "                                                                                      ",
  "river_regrid takes the following flags:                                               ",
  "                                                                                      ",
  "REQUIRED:                                                                             ",
  "                                                                                      ",
  "--mosaic    mosaic_grid     specify the mosaic file of destination grid. This mosaic  ",
  "                            file should be a coupler mosaic file, which contains link ",
  "                            to land solo mosaic and the exchange grid file.           ",
  "                                                                                      ",
  "--river_src river_src_file  specify the river network source data file. The data is   ",
  "                            assumed on regular lat-lon grid and the longitude is      ",
  "                            assumed from 0 to 360 degree and latitude is assumed      ",
  "                            from -90 to 90 degree.                                    ",
  "                                                                                      ",
  "OPTIONAL FLAGS                                                                        ",
  "                                                                                      ",
  "--output output_file        specify the output file base name. the suffix '.nc'       ",
  "                            should not be included in the output_file. The default    ",
  "                            value is river_output. For one tile mosaic, the actual    ",
  "                            result will be $output_file.nc. For multiple tile mosaic, ",
  "                            the result will be $output.tile#.nc.                      ",
  "                                                                                      ",  
  NULL
};

/* const double LARGE_VALUE       = 1e20; */
/* const int    X_CYCLIC          = 1; */
/* const int    Y_CYCLIC          = 2; */
/* const int    CUBIC_GRID        = 4; */
/* const int    x_refine          = 2; */
/* const int    y_refine          = 2; */
/* const double EPSLN             = 1.e-4; */
/* const double MIN_AREA_RATIO    = 1.e-6; */
/* const double D2R               = M_PI/180.; */
/*const char   subA_name[]       = "subA";*/
/* const char   tocell_name[]     = "tocell"; */
/* const char   travel_name[]     = "travel"; */
/* const char   basin_name[]      = "basin"; */
/* const char   cellarea_name[]   = "cellarea"; */
/* const char   celllength_name[] = "celllength"; */
/* const char   landfrac_name[]   = "land_frac"; */
/* const char   tagname[]         = "$Name:  $"; */
/* const char   version[]         = "0.1"; */
/* const int    ncells = 3; */
char   xaxis_name[128];
char   yaxis_name[128];
/* char   gridx_name[] = "grid_x"; */
/* char   gridy_name[] = "grid_y"; */
/* char   x_name[]     = "x"; */
/* char   y_name[]     = "y"; */
/* double suba_cutoff = 1.e12;   */


/* typedef struct { */
/*   int nx; */
/*   int ny; */
/*   double *xt; */
/*   double *yt; */
/*   double *xb; */
/*   double *yb; */
/*   double *xb_r; */
/*   double *yb_r; */
/*   double *area; */
/*   double *landfrac; */
/*   double *subA; */
/*   double *cellarea; */
/*   double *celllength; */
/*   int    *tocell; */
/*   int    *travel; */
/*   int    *basin; */
/*   int    *dir; */
/*   int    *last_point; */
/*   double subA_missing; */
/*   double cellarea_missing; */
/*   double celllength_missing; */
/*   int    tocell_missing; */
/*   int    travel_missing; */
/*   int    basin_missing; */
/*   char   filename[128]; */
/* } river_type; */

//inline void swap(void *x, void *y, size_t l);
void qsort_index(double array[], int start, int end, int rank[]);
void get_source_data(const char *src_file, river_type *river_data);
void get_mosaic_grid(const char *coupler_mosaic, const char *land_mosaic,
		     int ntiles, river_type *river_data, unsigned int *opcode);
void init_river_data(int ntiles, river_type *river_out, const river_type * const river_in);
void calc_max_subA(const river_type *river_in, river_type *river_out,
		   int ntiles, unsigned int opcode);
void update_halo_double(int ntiles, double **data, int nx, int ny, unsigned int opcode);
void update_halo_int(int ntiles, int **data, int nx, int ny, unsigned int opcode);
int adjust_lon(double x[], double tlon);
void calc_tocell(int ntiles, river_type *river_out, unsigned int opcode );
void calc_river_data(int ntiles, river_type* river_data, unsigned int opcode  );
void sort_basin(int ntiles, river_type* river_data);

void check_river_data( );
void write_river_data(const char *river_src_file, const char *output_file,
		      river_type* river_out, const char *history, int ntiles);
double distance(double lon1, double lat1, double lon2, double lat2);

int main(int argc, char* argv[])
{
  unsigned int opcode = 0;  
  int          option_index, c;
  char         *mosaic_file     = NULL;
  char         *river_src_file  = NULL;
  char         output_file[256] = "river_output";
  int          ntiles, n;
  char         land_mosaic_dir[256];
  char         land_mosaic[256];
  char         land_mosaic_file[256];  
  char         history[1024];
  
  river_type river_in;
  river_type *river_out; /* may be more than one tile */

  int errflg = (argc == 1);

  int    sizeof_int  = 0;
  int    sizeof_double = 0;

  static struct option long_options[] = {
    {"mosaic_grid",       required_argument, NULL, 'a'},
    {"river_src_file",    required_argument, NULL, 'b'},
    {"output",            required_argument, NULL, 'c'},
    {0, 0, 0, 0},
  };      

  mpp_init(&argc, &argv);
  sizeof_int = sizeof(int);
  sizeof_double = sizeof(double);
  
  /* currently we are assuming the tool is always run on 1 processor */
  if(mpp_npes() > 1) mpp_error("river_regrid: parallel is not supported yet, try running one single processor");
  while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
    switch (c) {
    case 'a':
      mosaic_file  = optarg;
      break;
    case 'b':
      river_src_file = optarg;
      break;
    case 'c':
      strcpy(output_file,optarg);
      break;
    case '?':
      errflg++;
      break;
    }
  }
  
  if (errflg) {
    char **u = usage;
    while (*u) { fprintf(stderr, "%s\n", *u); u++; }
    exit(2);
  }       

  /* check the arguments */
  if( !mosaic_file    ) mpp_error("fregrid: mosaic_grid is not specified");
  if( !river_src_file ) mpp_error("fregrid: river_source_file is not specified");

  strcpy(history,argv[0]);
  for(n=1;n<argc;n++)  {
    strcat(history, " ");
    strcat(history, argv[n]); 
  }

#ifdef test_qsort  
  {
    /* test qsort_index here */
    int    i, size = 7;
    double array[]={12,5,18,25,14, 4,11};
    int    rank[]={1,2,3,4,5,6,7};

    qsort_index(array, 0, size-1, rank);
    for(i=0; i<size; i++) printf("value = %f, rank = %d \n", array[i], rank[i]);
  }
#endif
  
  /* First read the source data and source grid */
  get_source_data(river_src_file, &river_in);

  /* get the mosaic grid information */
  {
    int fid, vid;
    get_file_path(mosaic_file, land_mosaic_dir);
    fid = mpp_open(mosaic_file, MPP_READ);
    vid = mpp_get_varid(fid, "lnd_mosaic_file");
    mpp_get_var_value(fid, vid, land_mosaic_file);
    sprintf(land_mosaic, "%s/%s", land_mosaic_dir, land_mosaic_file);
    mpp_close(fid);
    fid = mpp_open(land_mosaic, MPP_READ);
    ntiles = mpp_get_dimlen( fid, "ntiles");
    mpp_close(fid);
  }
  river_out = (river_type *)malloc(ntiles*sizeof(river_type));
  
  get_mosaic_grid(mosaic_file, land_mosaic, ntiles, river_out, &opcode);

  init_river_data(ntiles, river_out, &river_in);
  
  calc_max_subA(&river_in, river_out, ntiles, opcode );

  calc_tocell(ntiles, river_out, opcode );

  calc_river_data(ntiles, river_out, opcode );

  sort_basin(ntiles, river_out);
  
  check_river_data(ntiles, river_out);
  
  write_river_data(river_src_file, output_file, river_out, history, ntiles);
  
  printf("Successfully running river_regrid and the following output file are generated.\n");
  for(n=0; n<ntiles; n++) printf("****%s\n", river_out[n].filename);
  
  mpp_end();

  return 0;  

} /* end of main */


/* /\*------------------------------------------------------------------------------ */
/*   void get_source_data(char *src_file) */
/*   read the source file to get the source grid and river network data */
/*   -----------------------------------------------------------------------------*\/ */
/* void get_source_data(const char *src_file, river_type *river_data) */
/* { */
/*   int nx, ny, nxp, nyp; */
/*   int i, j; */
/*   int fid, vid; */
/*   double dbl_missing; */

/*   fid = mpp_open(src_file, MPP_READ); */
/*   vid = mpp_get_varid(fid, subA_name); */
  
/*   mpp_get_var_dimname(fid, vid, 1, xaxis_name); */
/*   mpp_get_var_dimname(fid, vid, 0, yaxis_name); */
/*   nx  = mpp_get_dimlen(fid, xaxis_name ); */
/*   ny  = mpp_get_dimlen(fid, yaxis_name ); */
/*   nxp = nx + 1; */
/*   nyp = ny + 1; */
/*   river_data->nx  = nx; */
/*   river_data->ny  = ny; */
  
/*   river_data->xt   = (double *)malloc(nx*sizeof(double)); */
/*   river_data->yt   = (double *)malloc(ny*sizeof(double)); */
/*   river_data->xb   = (double *)malloc(nxp*sizeof(double));  */
/*   river_data->yb   = (double *)malloc(nyp*sizeof(double)); */
/*   river_data->xb_r = (double *)malloc(nxp*sizeof(double));  */
/*   river_data->yb_r = (double *)malloc(nyp*sizeof(double)); */
/*   river_data->subA = (double *)malloc(nx*ny*sizeof(double)); */
/*   mpp_get_var_att(fid, vid, "missing_value", &(river_data->subA_missing) ); */
/*   vid = mpp_get_varid(fid, tocell_name); */
/*   mpp_get_var_att(fid, vid, "missing_value", &dbl_missing ); */
/*   river_data->tocell_missing = dbl_missing; */
/*   vid = mpp_get_varid(fid, travel_name); */
/*   mpp_get_var_att(fid, vid, "missing_value", &dbl_missing ); */
/*   river_data->travel_missing = dbl_missing; */
/*   vid = mpp_get_varid(fid, basin_name); */
/*   mpp_get_var_att(fid, vid, "missing_value", &dbl_missing ); */
/*   river_data->basin_missing = dbl_missing; */
/*   vid = mpp_get_varid(fid, cellarea_name); */
/*   mpp_get_var_att(fid, vid, "missing_value", &(river_data->cellarea_missing) ); */
/*   vid = mpp_get_varid(fid, celllength_name); */
/*   mpp_get_var_att(fid, vid, "missing_value", &(river_data->celllength_missing) ); */

/*   vid = mpp_get_varid(fid, subA_name); */
/*   mpp_get_var_value(fid, vid, river_data->subA); */
/*   vid = mpp_get_varid(fid, xaxis_name); */
/*   mpp_get_var_value(fid, vid, river_data->xt); */
/*   vid = mpp_get_varid(fid, yaxis_name); */
/*   mpp_get_var_value(fid, vid, river_data->yt); */
/*   mpp_close(fid); */
  
/*   for(i=1; i<nx; i++ ) river_data->xb[i] = 0.5*(river_data->xt[i-1]+river_data->xt[i]); */
/*   for(j=1; j<ny; j++ ) river_data->yb[j] = 0.5*(river_data->yt[j-1]+river_data->yt[j]); */
/*   river_data->xb[0] = 2*river_data->xt[0] - river_data->xb[1]; */
/*   river_data->yb[0] = 2*river_data->yt[0] - river_data->yb[1]; */
/*   river_data->xb[nx] = 2*river_data->xt[nx-1] - river_data->xb[nx-1]; */
/*   river_data->yb[ny] = 2*river_data->yt[ny-1] - river_data->yb[ny-1]; */

/*   /\* make sure the xb is in the range [0,360] and yb is in the range [-90,90] *\/ */
/*   if(fabs(river_data->xb[0]) > EPSLN) mpp_error("river_regrid: The starting longitude of grid bound is not 0"); */
/*   if(fabs(river_data->xb[nx]-360) > EPSLN) mpp_error("river_regrid: The ending longitude of grid bound is not 360");   */
/*   if(fabs(river_data->yb[0]+90) > EPSLN) mpp_error("river_regrid: The starting latitude of grid bound is not -90"); */
/*   if(fabs(river_data->yb[ny]-90) > EPSLN) mpp_error("river_regrid: The ending latitude of grid bound is not 90");   */
/*   river_data->xb[0]  = 0.; */
/*   river_data->xb[nx] = 360.; */
/*   river_data->yb[0]  = -90.; */
/*   river_data->yb[ny] = 90.; */
/*   for(j=0; j<nyp; j++) river_data->yb_r[j] = river_data->yb[j]*D2R; */
/*   for(i=0; i<nxp; i++) river_data->xb_r[i] = river_data->xb[i]*D2R; */

/* } */

/* /\*---------------------------------------------------------------------- */
/*    read the grid of detination mosaic.  */
/*    void get_mosaic_grid(char *file) */
/*    where file is the coupler mosaic file. */
/*    --------------------------------------------------------------------*\/ */
/* void get_mosaic_grid(const char *coupler_mosaic, const char *land_mosaic, int ntiles, river_type *river_data, unsigned int *opcode) */
/* { */
/*   int    n_xgrid_files, nx, ny, nxp, nyp, ni, nj, nip, njp; */
/*   int    n, m, i, j, ii, jj, nxp2, nyp2; */
/*   size_t start[4], nread[4]; */
/*   char   dir[STRING], gridfile[STRING], tilefile[STRING]; */
/*   char   tilename[STRING], land_mosaic_name[STRING]; */
/*   char   **xgrid_file; */
/*   double *x, *y; */
/*   double **pxt, **pyt; */
/*   char *pfile; */
/*   int  m_fid, m_vid, g_fid, g_vid; */

/*   /\* coupler_mosaic, land_mosaic, and exchange grid file should be located in the same directory *\/ */
/*   get_file_path(coupler_mosaic, dir); */
  
/*   m_fid = mpp_open(coupler_mosaic, MPP_READ); */
/*   m_vid = mpp_get_varid(m_fid, "lnd_mosaic"); */
/*   mpp_get_var_value(m_fid, m_vid, land_mosaic_name); */
  
/*   /\* get the exchange grid file name *\/ */
/*   n_xgrid_files = mpp_get_dimlen(m_fid, "nfile_aXl"); */
/*   xgrid_file = (char **)malloc(n_xgrid_files*sizeof(char *)); */

/*   m_vid = mpp_get_varid(m_fid, "aXl_file"); */
/*   for(n=0; n<n_xgrid_files; n++) { */
/*     xgrid_file[n] = (char *)malloc(STRING*sizeof(char)); */
/*     start[0] = n; */
/*     start[1] = 0; */
/*     nread[0] = 1; */
/*     nread[1] = STRING;     */
/*     mpp_get_var_value_block(m_fid, m_vid, start, nread, xgrid_file[n]); */
/*   } */
/*   mpp_close(m_fid); */
/*   m_fid = mpp_open(land_mosaic, MPP_READ); */
/*   m_vid = mpp_get_varid(m_fid, "gridfiles"); */
  
/*   for( n = 0; n < ntiles; n++ ) { */
/*     double *area; */
    
/*     start[0] = n; */
/*     start[1] = 0; */
/*     nread[0] = 1; */
/*     nread[1] = STRING; */
/*     mpp_get_var_value_block(m_fid, m_vid, start, nread, tilefile); */
/*     sprintf(gridfile, "%s/%s", dir, tilefile); */
/*     g_fid = mpp_open(gridfile, MPP_READ); */
/*     ni = mpp_get_dimlen(g_fid, "nx"); */
/*     nj = mpp_get_dimlen(g_fid, "ny"); */
/*     nip = ni + 1; */
/*     njp = nj + 1; */
/*     x = (double *)malloc(nip*njp*sizeof(double)); */
/*     y = (double *)malloc(nip*njp*sizeof(double)); */
/*     g_vid = mpp_get_varid(g_fid, "x"); */
/*     mpp_get_var_value(g_fid, g_vid, x); */
/*     g_vid = mpp_get_varid(g_fid, "y"); */
/*     mpp_get_var_value(g_fid, g_vid, y); */
/*     mpp_close(g_fid); */
/*     if( ni%x_refine != 0 ) mpp_error("river_regrid: supergrid x-size can not be divided by x_refine"); */
/*     if( nj%y_refine != 0 ) mpp_error("river_regrid: supergrid y-size can not be divided by y_refine"); */
/*     nx   = ni/x_refine; */
/*     ny   = nj/y_refine; */
/*     nxp  = nx + 1; */
/*     nyp  = ny + 1; */
/*     nxp2 = nx + 2; */
/*     nyp2 = ny + 2; */
/*     river_data[n].nx       = nx; */
/*     river_data[n].ny       = ny; */
/*     river_data[n].xt       = (double *)malloc(nxp2*nyp2*sizeof(double));         */
/*     river_data[n].yt       = (double *)malloc(nxp2*nyp2*sizeof(double)); */
/*     river_data[n].xb       = (double *)malloc(nxp*nyp*sizeof(double)); */
/*     river_data[n].yb       = (double *)malloc(nxp*nyp*sizeof(double)); */
/*     river_data[n].xb_r     = (double *)malloc(nxp*nyp*sizeof(double)); */
/*     river_data[n].yb_r     = (double *)malloc(nxp*nyp*sizeof(double));     */
/*     river_data[n].area     = (double *)malloc(nx*ny*sizeof(double)); */
/*     river_data[n].landfrac = (double *)malloc(nx*ny*sizeof(double));     */
/*     area                   = (double *)malloc(nx*ny*sizeof(double)); */
    
/*     /\* copy the data from super grid to fine grid *\/ */
/*     for(i=0; i<nx*ny; i++) area[i] = 0.0; */
/*     for(j = 0; j < ny; j++) for(i = 0; i < nx; i++) { */
/*       ii = i+1; */
/*       jj = j+1; */
/*       river_data[n].xt[jj*nxp2+ii] = x[(j*y_refine+1)*nip+i*x_refine+1]; */
/*       river_data[n].yt[jj*nxp2+ii] = y[(j*y_refine+1)*nip+i*x_refine+1]; */
/*     } */
/*     for(j = 0; j < nyp; j++) for(i = 0; i < nxp; i++) { */
/*       river_data[n].xb[j*nxp+i] = x[(j*y_refine)*nip+i*x_refine]; */
/*       river_data[n].yb[j*nxp+i] = y[(j*y_refine)*nip+i*x_refine]; */
/*     }      */
/*     free(x); */
/*     free(y); */
/*     /\* calculate cell area *\/ */

/*     for(m=0; m<nxp*nyp; m++) { */
/*       river_data[n].xb_r[m] = river_data[n].xb[m] * D2R; */
/*       river_data[n].yb_r[m] = river_data[n].yb[m] * D2R; */
/*     } */
/*     get_grid_area(&nx, &ny, river_data[n].xb_r, river_data[n].yb_r, river_data[n].area); */

/*     /\* calculate the land fraction *\/ */
/*     sprintf(tilename, "X%s_tile%d", land_mosaic_name, n+1); */
/*     for(m=0; m<n_xgrid_files; m++) { */
/*       if(strstr(xgrid_file[m],tilename)) { */
/* 	int    nxgrid, l; */
/* 	int    *i1, *j1, *i2, *j2; */
/* 	double *xgrid_area; */
/* 	char filewithpath[512]; */
	
/* 	sprintf(filewithpath, "%s/%s", dir, xgrid_file[m]); */
/* 	g_fid = mpp_open(filewithpath, MPP_READ); */
/* 	nxgrid = mpp_get_dimlen(g_fid, "ncells"); */
/* 	mpp_close(g_fid); */
/* 	i1         = (int    *)malloc(nxgrid*sizeof(int)); */
/* 	j1         = (int    *)malloc(nxgrid*sizeof(int));	   */
/* 	i2         = (int    *)malloc(nxgrid*sizeof(int)); */
/* 	j2         = (int    *)malloc(nxgrid*sizeof(int)); */
/* 	xgrid_area = (double *)malloc(nxgrid*sizeof(double)); */
/* 	read_mosaic_xgrid_order1(filewithpath, i1, j1, i2, j2, xgrid_area); */
/* 	for(l=0; l<nxgrid; l++) area[j2[l]*nx+i2[l]] += xgrid_area[l]; */
/* 	free(i1); */
/* 	free(j1); */
/* 	free(i2); */
/* 	free(j2); */
/*       } */
/*     } */

/*     for(m=0; m<nx*ny; m++) area[m] *= 4*M_PI*RADIUS*RADIUS; */
/*     for(m=0; m<nx*ny; m++) { */
/*       river_data[n].landfrac[m] = area[m]/river_data[n].area[m]; */
/*       /\* consider truncation error *\/ */
/*       if(fabs(river_data[n].landfrac[m]-1) < EPSLN) river_data[n].landfrac[m] = 1; */
/*       if(fabs(river_data[n].landfrac[m])   < EPSLN) river_data[n].landfrac[m] = 0; */
/*       if(river_data[n].landfrac[m] > 1 || river_data[n].landfrac[m] < 0) */
/* 	mpp_error("river_regrid: land_frac should be between 0 or 1"); */
/*     } */
/*     free(area); */
/*   } /\* n = 0, ntiles *\/ */

/*   mpp_close(m_fid); */
  
/*   /\* currently we are assuming all the times have the same grid size *\/ */
/*   for(n=1; n<ntiles; n++) { */
/*     if(river_data[n].nx != river_data[0].nx || river_data[n].ny != river_data[0].ny ) */
/*       mpp_error("river_regrid: all the tiles should have the same grid size"); */
/*   } */
  
/*   /\*---------------------------------------------------------------------------- */
/*     get boundary condition, currently we are assuming the following case, */
/*     solid walls: number of contact will be 0. */
/*     x-cyclic:    number of contact will be 1. */
/*     y-cyclic:    number of contact will be 1. */
/*     torus:       number of contact will be 2. */
/*     cubic:       number of contact will be 12. */
/*     --------------------------------------------------------------------------*\/ */
/*   { */
/*     int ncontact; */
/*     int *tile1, *tile2; */
/*     int *istart1, *iend1, *jstart1, *jend1; */
/*     int *istart2, *iend2, *jstart2, *jend2; */
    
/*     ncontact = read_mosaic_ncontacts(land_mosaic); */
/*     tile1   = (int *)malloc(ncontact*sizeof(int)); */
/*     tile2   = (int *)malloc(ncontact*sizeof(int)); */
/*     istart1 = (int *)malloc(ncontact*sizeof(int)); */
/*     iend1   = (int *)malloc(ncontact*sizeof(int)); */
/*     jstart1 = (int *)malloc(ncontact*sizeof(int)); */
/*     jend1   = (int *)malloc(ncontact*sizeof(int)); */
/*     istart2 = (int *)malloc(ncontact*sizeof(int)); */
/*     iend2   = (int *)malloc(ncontact*sizeof(int)); */
/*     jstart2 = (int *)malloc(ncontact*sizeof(int)); */
/*     jend2   = (int *)malloc(ncontact*sizeof(int)); */
/*     if(ncontact >0) { */
/*       read_mosaic_contact(land_mosaic, tile1, tile2, istart1, iend1, */
/* 			  jstart1, jend1, istart2, iend2, jstart2, jend2); */
/*       if(ncontact <= 2) { /\* x-cyclic of y-cyclic *\/ */
/* 	if(ntiles !=1) mpp_error("river_regrid: number of tiles must be 1 for single contact"); */
/* 	for(n=0; n<ncontact; n++) { */
/* 	  if(istart1[n] == iend1[n] && istart2[n] == iend2[n] ) /\* x_cyclic *\/ */
/* 	    *opcode |= X_CYCLIC; */
/* 	  else if(jstart1[n] == jend1[n] && istart2[n] == iend2[n] ) /\* x_cyclic *\/ */
/* 	    *opcode |= Y_CYCLIC; */
/* 	  else */
/* 	    mpp_error("river_regrid: for one-tile mosaic, the boundary condition should be either x-cyclic or y-cyclic"); */
/* 	} */
/*       } */
/*       else if(ncontact == 12) { */
/* 	if(ntiles != 6) mpp_error("river_regrid: the mosaic must be a 6-tile cubic grid for 12 contacts."); */
/* 	*opcode |= CUBIC_GRID; */
/*       } */
/*       else */
/* 	mpp_error("river_regrid: the number of contact should be either 0, 1, 2 or 6"); */
/*     } */

/*     free(tile1); */
/*     free(tile2); */
/*     free(istart1); */
/*     free(iend1); */
/*     free(jstart1); */
/*     free(jend1); */
/*     free(istart2); */
/*     free(iend2); */
/*     free(jstart2); */
/*     free(jend2); */
/*   } */

/*   /\* update halo of xt and yt *\/ */
/*   pxt = (double **)malloc(ntiles*sizeof(double *)); */
/*   pyt = (double **)malloc(ntiles*sizeof(double *)); */
/*   for(n=0; n<ntiles; n++) { */
/*     pxt[n] = river_data[n].xt; */
/*     pyt[n] = river_data[n].yt; */
/*   } */
/*   update_halo_double(ntiles, pxt, nx, ny, *opcode); */
/*   update_halo_double(ntiles, pyt, nx, ny, *opcode); */
/*   free(pxt); */
/*   free(pyt); */
  
/* }; /\* get_mosaic_grid *\/ */

/* /\*------------------------------------------------------------------------------ */
/*   void init_river_data */
/*   allocate memory to river data and initialize these river data with missing value */
/*   ----------------------------------------------------------------------------*\/ */
/* void init_river_data(int ntiles, river_type *river_out, const river_type * const river_in) */
/* { */
/*   int nx, ny, nxp2, nyp2, n, i; */
/*   int tocell_missing, subA_missing, travel_missing, basin_missing; */
/*   double cellarea_missing, celllength_missing; */
  
/*   nx = river_out->nx; */
/*   ny = river_out->ny; */
/*   nxp2 = nx + 2; */
/*   nyp2 = ny + 2; */
/*   subA_missing   = river_in->subA_missing; */
/*   tocell_missing = river_in->tocell_missing; */
/*   travel_missing = river_in->travel_missing; */
/*   basin_missing  = river_in->basin_missing; */
/*   cellarea_missing = river_in->cellarea_missing; */
/*   celllength_missing = river_in->celllength_missing; */
/*   for(n=0; n<ntiles; n++) { */
/*     river_out[n].subA_missing   = subA_missing; */
/*     river_out[n].tocell_missing = tocell_missing; */
/*     river_out[n].travel_missing = travel_missing; */
/*     river_out[n].basin_missing  = basin_missing; */
/*     river_out[n].cellarea_missing = cellarea_missing; */
/*     river_out[n].celllength_missing  = celllength_missing; */
/*     river_out[n].subA       = (double *)malloc(nxp2*nyp2*sizeof(double)); */
/*     river_out[n].tocell     = (int    *)malloc(nxp2*nyp2*sizeof(int   )); */
/*     river_out[n].travel     = (int    *)malloc(nxp2*nyp2*sizeof(int   )); */
/*     river_out[n].basin      = (int    *)malloc(nxp2*nyp2*sizeof(int   )); */
/*     river_out[n].dir        = (int    *)malloc(nxp2*nyp2*sizeof(int   )); */
/*     river_out[n].cellarea   = (double *)malloc(nx  *ny  *sizeof(double)); */
/*     river_out[n].celllength = (double *)malloc(nx  *ny  *sizeof(double)); */
/*     river_out[n].last_point = (int    *)malloc(nx  *ny  *sizeof(int   )); */
/*     for(i=0; i<nxp2*nyp2; i++) { */
/*       river_out[n].subA  [i] = subA_missing; */
/*       river_out[n].travel[i] = travel_missing; */
/*       river_out[n].basin [i] = basin_missing; */
/*       river_out[n].tocell[i] = tocell_missing; */
/*       river_out[n].dir[i]    = -1; */
/*     } */
/*     for(i=0; i<nx*ny; i++) { */
/*       river_out[n].cellarea[i]   = cellarea_missing; */
/*       river_out[n].celllength[i] = celllength_missing; */
/*       river_out[n].last_point[i] =  0; */
/*     } */
/*   } */

/* } */


/* /\*------------------------------------------------------------------------------ */
/*   void calc_max_subA(const river_type *river_in, river_type *river_out, int ntiles) */
/*   find max value of suba in each model grid cell. */
/*   ----------------------------------------------------------------------------*\/ */
/* void calc_max_subA(const river_type *river_in, river_type *river_out, int ntiles, unsigned int opcode) */
/* { */
/*   int nx_in, ny_in; */
/*   int nx_out, ny_out, nxp_out, nxp1, nyp1, nxp2, nyp2; */
/*   int n, i, j, i1, j1, ii, jj, n_out; */
/*   double missing, ll_y, ur_y, ll_x, ur_x; */
/*   double xv1[4], yv1[4], xv2[20], yv2[20]; */
/*   double *xb_in, *yb_in; */
/*   double *xb_out, *yb_out; */
/*   double **psubA; */
  
/*   nx_in = river_in->nx; */
/*   ny_in = river_in->ny; */
/*   xb_in = river_in->xb_r; */
/*   yb_in = river_in->yb_r; */
/*   missing = river_out->subA_missing; */
  
/*   for(n=0; n<ntiles; n++) { */
/*     nx_out   = river_out[n].nx; */
/*     ny_out   = river_out[n].ny; */
/*     nxp2     = nx_out + 2; */
/*     nyp2     = ny_out + 2;     */
/*     nxp_out  = nx_out+1; */
/*     xb_out   = river_out[n].xb_r; */
/*     yb_out   = river_out[n].yb_r; */
/*     for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) { */
/*       j1 = j+1; */
/*       i1 = i+1; */
/*       /\* set partial land cell and ocean cell to have large subA value *\/ */
/*       if(river_out[n].landfrac[j*nx_out+i] < 1) { */
/* 	river_out[n].subA[j1*nxp2+i1] = LARGE_VALUE; */
/* 	continue; */
/*       } */
      
/*       xv1[0] = xb_out[j*nxp_out+i]; */
/*       xv1[1] = xb_out[j*nxp_out+i1]; */
/*       xv1[2] = xb_out[j1*nxp_out+i1]; */
/*       xv1[3] = xb_out[j1*nxp_out+i];  */
/*       yv1[0] = yb_out[j*nxp_out+i]; */
/*       yv1[1] = yb_out[j*nxp_out+i1]; */
/*       yv1[2] = yb_out[j1*nxp_out+i1]; */
/*       yv1[3] = yb_out[j1*nxp_out+i];  */
      
/*       for(jj=0; jj<ny_in; jj++) { */
/* 	ll_y = yb_in[jj]; */
/* 	ur_y = yb_in[jj+1]; */
/* 	if (  (yv1[0]<=ll_y) && (yv1[1]<=ll_y) */
/* 	      && (yv1[2]<=ll_y) && (yv1[3]<=ll_y) ) continue; */
/* 	if (  (yv1[0]>=ur_y) && (yv1[1]>=ur_y) */
/* 	      && (yv1[2]>=ur_y) && (yv1[3]>=ur_y) ) continue; */
	
/* 	for(ii=0; ii<nx_in; ii++) { */
/* 	  if(river_in->subA[jj*nx_in+ii] == missing) continue; */
/* 	  ll_x = xb_in[ii]; */
/* 	  ur_x = xb_in[ii+1]; */
/* 	  /\* adjust xv1 to make sure it is in the range as ll_x and ur_x *\/ */
/* 	  adjust_lon(xv1, 0.5*(ll_x + ur_x) ); */
/* 	  if ( (xv1[0]<=ll_x) && (xv1[1]<=ll_x) && (xv1[2]<=ll_x) && (xv1[3]<=ll_x) ) continue; */
/* 	  if ( (xv1[0]>=ur_x) && (xv1[1]>=ur_x) && (xv1[2]>=ur_x) && (xv1[3]>=ur_x) ) continue;	   */
/* 	  if ( (n_out = clip ( xv1, yv1, 4, ll_x, ll_y, ur_x, ur_y, xv2, yv2 )) > 0 ) { */
/* 	    double xarea;  */
/* 	    xarea = poly_area (xv2, yv2, n_out ); */
/* 	    if( xarea/river_out[n].area[j*nx_out+i] < MIN_AREA_RATIO ) continue; */
/* 	    if(river_in->subA[jj*nx_in+ii] > river_out[n].subA[j1*nxp2+i1]) */
/* 	      river_out[n].subA[j1*nxp2+i1] = river_in->subA[jj*nx_in+ii]; */
/* 	  } */
/* 	} */
/*       } */
/*     } */
/*   } */
	
/*   /\* fill the halo of subA *\/ */
/*   psubA = (double **)malloc(ntiles*sizeof(double *)); */
/*   for(n=0; n<ntiles; n++) psubA[n] = river_out[n].subA;  */
/*   update_halo_double(ntiles, psubA, river_out->nx, river_out->ny, opcode);  */
/*   free(psubA); */
/* };/\* calc_max_subA *\/ */

/* /\*------------------------------------------------------------------------------ */
/*   void update_halo_double(int ntiles, double *data, int nx, int ny, unsigned int opcode) */
/*   We assume all the tiles have the same size. */
/*   -----------------------------------------------------------------------------*\/ */
/* void update_halo_double(int ntiles, double **data, int nx, int ny, unsigned int opcode) */
/* { */
/*   int nxp1, nyp1, nxp2, i, j, n; */
/*   int te, tw, ts, tn; */
  
/*   nxp1   = nx + 1; */
/*   nyp1   = ny + 1; */
/*   nxp2   = nx + 2; */
/*   if(opcode & X_CYCLIC) { */
/*     for(j=1; j<nyp1; j++) { */
/*       data[0][j*nxp2]      = data[0][j*nxp2+nx];     /\* West *\/ */
/*       data[0][j*nxp2+nxp1] = data[0][j*nxp2+1];      /\* east *\/ */
/*     } */
/*   } */
/*   if(opcode & Y_CYCLIC) { */
/*     for(i=1; i<nyp1; i++) { */
/*       data[0][i]           = data[0][ny*nxp2+i];     /\* south *\/ */
/*       data[0][nyp1*nxp2+i] = data[0][nxp2+i];        /\* north *\/ */
/*     } */
/*   } */
/*   if(opcode & X_CYCLIC && opcode & Y_CYCLIC) { */
/*     data[0][0]              = data[0][ny*nxp2];      /\* southwest *\/ */
/*     data[0][nxp1]           = data[0][ny*nxp2+1];    /\* southeast *\/     */
/*     data[0][nyp1*nxp2+nxp1] = data[0][nxp2+1];       /\* northeast *\/ */
/*     data[0][nyp1*nxp2]      = data[0][nxp2+nx];      /\* northwest *\/      */
/*   } */

/*   if(opcode & CUBIC_GRID) { */
/*     for(n=0; n<ntiles; n++) { */
      
/*       if(n%2) { /\* tile 2 4 6 *\/ */
/* 	tw = (n+5)%ntiles; te = (n+2)%ntiles; ts = (n+4)%ntiles; tn = (n+1)%ntiles; */
/* 	for(j=1; j<nyp1; j++) { */
/* 	  data[n][j*nxp2]      = data[tw][j*nxp2+nx];     /\* west *\/ */
/* 	  data[n][j*nxp2+nxp1] = data[te][nxp2+(nxp1-j)]; /\* east *\/ */
/* 	} */
/* 	for(i=1; i<nxp1; i++) { */
/* 	  data[n][i]           = data[ts][(nyp1-i)*nxp2+nx]; /\* south *\/ */
/* 	  data[n][nyp1*nxp2+i] = data[tn][nxp2+i];           /\* north *\/ */
/* 	} */
/*       } else {  /\* tile 1, 3, 5 *\/ */
/*        tw = (n+4)%ntiles; te = (n+1)%ntiles; ts = (n+5)%ntiles; tn = (n+2)%ntiles; */
/* 	for(j=1; j<nyp1; j++) { */
/* 	  data[n][j*nxp2]      = data[tw][ny*nxp2+nxp1-j]; /\* west *\/ */
/* 	  data[n][j*nxp2+nxp1] = data[te][j*nxp2+1];       /\* east *\/ */
/* 	} */
/* 	for(i=1; i<nxp1; i++) { */
/* 	  data[n][i]           = data[ts][ny*nxp2+i];       /\* south *\/ */
/* 	  data[n][nyp1*nxp2+i] = data[tn][(nyp1-i)*nxp2+1]; /\* north *\/ */
/* 	} */
/*       } */
/*     } */
/*   } */
  
/* };/\* update_halo *\/ */

/* void update_halo_int(int ntiles, int **data, int nx, int ny, unsigned int opcode) */
/* { */
/*   double **ldata; */
/*   int n, i, j, nxp2, nyp2; */
  
/*   nxp2 = nx + 2; */
/*   nyp2 = ny + 2; */


  
/*   ldata = (double **)malloc(ntiles*sizeof(double *)); */
/*   for(n=0; n<ntiles; n++) { */
/*     ldata[n] =  (double *)malloc(nxp2*nyp2*sizeof(double)); */
/*     for(i=0; i<nxp2*nyp2; i++) ldata[n][i] = data[n][i]; */
/*   } */
/*   update_halo_double(ntiles, ldata, nx, ny, opcode); */
/*   for(n=0; n<ntiles; n++) { */
/*     for(i=0; i<nxp2*nyp2; i++) data[n][i] = ldata[n][i]; */
/*     free(ldata[n]); */
/*   } */
/*   free(ldata); */
  
/* } */

/* int adjust_lon(double x[], double tlon) */
/* { */
/*   double x_sum, dx; */
/*   int i, npts = 4; */

/*   x_sum = x[0]; */
/*   for (i=1;i<npts;i++) { */
/*     double dx = x[i]-x[i-1]; */

/*     if      (dx < -M_PI) dx = dx + 2*M_PI; */
/*     else if (dx >  M_PI) dx = dx - 2*M_PI; */
/*     x_sum += (x[i] = x[i-1] + dx); */
/*   } */

/*   dx = (x_sum/npts)-tlon; */
/*   if      (dx < -M_PI) for (i=0;i<npts;i++) x[i] += 2*M_PI; */
/*   else if (dx >  M_PI) for (i=0;i<npts;i++) x[i] -= 2*M_PI; */

/* }; /\* adjust_lon *\/ */

/* /\*------------------------------------------------------------------------------ */
/*   Find the tocell value for the new grid. */
/*   void calc_tocell( ) */
/*   ----------------------------------------------------------------------------*\/ */
/* void calc_tocell(int ntiles, river_type *river_data, unsigned int opcode )   */
/* { */
/*   const int out_flow[] = { 8, 4, 2, 16, -1, 1, 32, 64, 128}; */
/*   const int out_dir[]  = { 3, 2, 1, 4,  -1, 0, 5,   6,   7}; */
/*   int n, nx, ny, nxp2, ioff, joff; */
/*   int i, j, ii, jj, iget, jget, im1, jm1; */
/*   double tval, subA_missing, subA, subA_me; */
/*   int **ptocell, **pdir; */
  
/*   nx = river_data->nx; */
/*   ny = river_data->ny; */
/*   nxp2 = nx + 2; */
/*   subA_missing = river_data->subA_missing; */
/*   ptocell = (int **)malloc(ntiles*sizeof(int *)); */
/*   pdir    = (int **)malloc(ntiles*sizeof(int *)); */
/*   for(n=0; n<ntiles; n++) { */
/*     ptocell[n] = river_data[n].tocell; */
/*     pdir   [n] = river_data[n].dir; */
/*   } */
  
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*       jm1 = j - 1; */
/*       im1 = i - 1; */
/*       if(river_data[n].landfrac[jm1*nx+im1] == 0) { */
/* 	/\* do nothing *\/ */
/*       } */
/*       else if(river_data[n].landfrac[jm1*nx+im1] < 1) { */
/* 	ptocell[n][j*nxp2+i] = 0; */
/*       } */
/*       else { */
/* 	subA_me = river_data[n].subA[j*nxp2+i]; */
/* 	tval= -999 ;  iget= -1 ;  jget= -1; */
/* 	if(subA_me > suba_cutoff ) { /\* tocell will be land cell with larger subA value, instead of neighboring ocean cell. *\/ */
/* 	  for(joff=0; joff<ncells; joff++) { */
/* 	    jj = jm1+joff; */
/* 	    for(ioff=0; ioff<ncells; ioff++) { */
/* 	      ii = im1+ioff; */
/* 	      if(ioff == 1 && joff == 1) continue; */
/* 	      subA = river_data[n].subA[jj*nxp2+ii]; */
/* 	      if(subA > subA_me) { */
/* 		if(tval == -999 ) { */
/* 		  if(subA > tval) { */
/* 		    iget = ioff; */
/* 		    jget = joff; */
/* 		    tval = subA; */
/* 		  } */
/* 		} */
/* 		else if(tval < LARGE_VALUE) { */
/* 		  if(subA > tval && subA < LARGE_VALUE) { */
/* 		    iget = ioff; */
/* 		    jget = joff; */
/* 		    tval = subA; */
/* 		  } */
/* 		} */
/* 		else { */
/* 		  iget = ioff; */
/* 		  jget = joff; */
/* 		  tval = subA; */
/* 		} */
/* 	      } */
/* 	    } */
/* 	  } */
/* 	} */
/* 	else { */
/* 	  for(joff=0; joff<ncells; joff++) { */
/* 	    jj = jm1+joff; */
/* 	    for(ioff=0; ioff<ncells; ioff++) { */
/* 	      ii = im1+ioff; */
/* 	      if(ioff == 1 && joff == 1) continue; */

/* 	      subA = river_data[n].subA[jj*nxp2+ii]; */
/* 	      if( subA != subA_missing ) { */
/* 		if( subA >= tval) { */
/* 		  iget = ioff; */
/* 		  jget = joff; */
/* 		  tval = subA; */
/* 		} */
/* 	      } */
/* 	    } */
/* 	  } */
/* 	} */
/* 	if(iget >= 0 && jget >= 0) { */
/* 	  if(tval > river_data[n].subA[j*nxp2+i]) { */
/* 	    ptocell[n][j*nxp2+i] = out_flow[jget*ncells+iget]; */
/* 	    pdir   [n][j*nxp2+i] = out_dir [jget*ncells+iget]; */
/* 	    if(pow(2,pdir[n][j*nxp2+i]) != ptocell[n][j*nxp2+i] ) */
/* 	      mpp_error("river_regrid: pow(2,dir) should equal to tocell"); */
/* 	  } */
/* 	  else */
/* 	    ptocell[n][j*nxp2+i] = 0; */
/* 	} */
/* 	else  */
/* 	  ptocell[n][j*nxp2+i] = 0; */
/*       } */
/*     } */
/*   } */
/*   update_halo_int(ntiles, ptocell, nx, ny, opcode);    */
/*   update_halo_int(ntiles, pdir, nx, ny, opcode); */
/*   free(ptocell); */
/*   free(pdir); */
  
/* };/\* calc_tocell *\/ */


/* /\*------------------------------------------------------------------------------ */
/*   void calc_river_data() */
/*   calculate travel and basin according to tocell, as well as celllength, cellarea. */
/*   For each basin, one and only one river point will have tocell = 0, this point will */
/*   have travel = 1. */
/*   ----------------------------------------------------------------------------*\/ */
/* void calc_river_data(int ntiles, river_type* river_data, unsigned int opcode ) */
/* { */
/*   int    nx, ny, nxp2, nyp2, n, i, j, ii, jj, nxp, nyp, im1, jm1, ioff, joff; */
/*   int    cur_basin, cur_travel; */
/*   int    not_done, dir, i_dest, j_dest; */
/*   int    basin_missing, travel_missing, tocell_missing; */
/*   int    maxtravel, maxbasin, travelnow; */
/*   double subA_missing; */
/*   int    **pbasin, **ptravel, **pdir; */
/*   double **psubA; */
/*   double xv[4], yv[4]; */
  
/*   const int di[] = {1,1,0,-1,-1,-1,0,1}; */
/*   const int dj[] = {0,-1,-1,-1,0,1,1,1}; */
/*   nx = river_data->nx; */
/*   ny = river_data->ny; */
/*   nxp  = nx+1; */
/*   nyp  = ny+1; */
/*   nxp2 = nx+2; */
/*   nyp2 = ny+2; */
/*   basin_missing  = river_data->basin_missing; */
/*   subA_missing   = river_data->subA_missing; */
/*   tocell_missing = river_data->tocell_missing; */
/*   cur_basin = 0; */

/*   /\* set up pointer *\/ */
/*   pbasin  = (int **)malloc(ntiles*sizeof(int *)); */
/*   ptravel = (int **)malloc(ntiles*sizeof(int *)); */
/*   pdir    = (int **)malloc(ntiles*sizeof(int *)); */
/*   psubA   = (double **)malloc(ntiles*sizeof(double *)); */
/*   for(n=0; n<ntiles; n++) { */
/*     pbasin [n] = river_data[n].basin; */
/*     ptravel[n] = river_data[n].travel; */
/*     psubA  [n] = river_data[n].subA; */
/*     pdir   [n] = river_data[n].dir; */
/*   } */


/*   /\* reinitialize subA *\/ */
/*   for(n=0; n<ntiles; n++) { */
/*     for(i=0; i<nxp2*nyp2; i++) psubA[n][i] = subA_missing;     */
/*   } */
  
/*   /\* calculate celllength and cellarea *\/ */
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=0; j<ny; j++) for(i=0; i<nx; i++) { */
/*       if(river_data[n].landfrac[j*nx+i] > 0) { */
/* 	ii = i+1; */
/* 	jj = j+1; */

/* 	xv[0] = river_data[n].xb_r[j*nxp+i]; */
/* 	xv[1] = river_data[n].xb_r[j*nxp+ii]; */
/* 	xv[2] = river_data[n].xb_r[jj*nxp+ii]; */
/* 	xv[3] = river_data[n].xb_r[jj*nxp+i]; */
/* 	yv[0] = river_data[n].yb_r[j*nxp+i]; */
/* 	yv[1] = river_data[n].yb_r[j*nxp+ii]; */
/* 	yv[2] = river_data[n].yb_r[jj*nxp+ii]; */
/* 	yv[3] = river_data[n].yb_r[jj*nxp+i]; */
/* 	river_data[n].cellarea[j*nx+i] = river_data[n].area[j*nx+i]*river_data[n].landfrac[j*nx+i]; */
/* 	dir = pdir[n][jj*nxp2+ii]; */
/* 	if( dir >= 0) { */
/* 	  i_dest = ii + di[dir]; */
/* 	  j_dest = jj + dj[dir];	 */
/* 	  river_data[n].celllength[j*nx+i] = distance(river_data[n].xt[jj*nxp2+ii], */
/* 						      river_data[n].yt[jj*nxp2+ii], */
/* 						      river_data[n].xt[j_dest*nxp2+i_dest], */
/* 						      river_data[n].yt[j_dest*nxp2+i_dest] ); */
/* 	} */
/* 	else */
/* 	  river_data[n].celllength[j*nx+i] = 0; */
/*       } */
/*     } */
/*   } */

  
/*   /\* define the basinid and travel for the coast point *\/ */
  
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*       if(river_data[n].tocell[j*nxp2+i] == 0) { */
/* 	pbasin[n] [j*nxp2+i] = ++cur_basin; */
/* 	ptravel[n][j*nxp2+i] = 0; */
/* 	river_data[n].last_point[(j-1)*nx+i-1] = 1; */
/*       } */
/*       else if(river_data[n].tocell[j*nxp2+i] > 0){ */
/* 	dir = pdir[n][j*nxp2+i]; */
/* 	i_dest = i + di[dir]; */
/* 	j_dest = j + dj[dir]; */
/* 	if(river_data[n].tocell[j_dest*nxp2+i_dest] == tocell_missing) { */
/* 	  pbasin[n] [j*nxp2+i] = ++cur_basin; */
/* 	  ptravel[n][j*nxp2+i] = 1; */
/* 	  river_data[n].last_point[(j-1)*nx+i-1] = 1; */
/* 	} */
/*       } */
/*     } */
/*   } */

/*   update_halo_int(ntiles, pbasin, nx, ny, opcode); */
/*   update_halo_int(ntiles, ptravel, nx, ny, opcode); */
  
/*   /\* then define the travel and basin for all other points *\/ */
/*   cur_travel = 0; */

/*   not_done = 1; */
/*   while(not_done) { */
/*     not_done = 0; */
/*     for(n=0; n<ntiles; n++) { */
/*       for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/* 	dir = pdir[n][j*nxp2+i]; */
/* 	if( dir >= 0 && pbasin[n][j*nxp2+i] == basin_missing ) { */
/*           i_dest = i + di[dir]; */
/* 	  j_dest = j + dj[dir]; */
/* 	  if( ptravel[n][j_dest*nxp2+i_dest] == cur_travel ) { */
/* 	    not_done = 1; /\* still have points need to be updateed *\/ */
/* 	    if(pbasin[n][j_dest*nxp2+i_dest] == basin_missing) { */
/* 	      mpp_error("river_grid: the tocell should have valid basin value"); */
/* 	    } */
/* 	    pbasin[n][j*nxp2+i] = pbasin[n][j_dest*nxp2+i_dest]; */
/* 	    ptravel[n][j*nxp2+i] = cur_travel+1; */
/* 	  } */
/* 	} */
/*       } */
/*     } */
/*     cur_travel++; */
/*     update_halo_int(ntiles, pbasin, nx, ny, opcode); */
/*     update_halo_int(ntiles, ptravel, nx, ny, opcode);     */
/*   } */

/*   /\* figure out maximum travel and maximum basin*\/ */
/*   maxtravel = -1; */
/*   maxbasin  = -1; */
/*   basin_missing = river_data->basin_missing; */
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {     */
/*       maxtravel = max(maxtravel, ptravel[n][j*nxp2+i]); */
/*       maxbasin  = max(maxbasin,  pbasin[n][j*nxp2+i]); */
/*     } */
/*   } */

/*   for(travelnow=maxtravel; travelnow>=0; travelnow--) { */
/*     for(n=0; n<ntiles; n++) for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*       jm1 = j - 1; */
/*       im1 = i - 1; */
/*       if(ptravel[n][j*nxp2+i] == travelnow) { */
/* 	psubA[n][j*nxp2+i] = river_data[n].cellarea[jm1*nx+im1]; */
/* 	/\* add the subA of from cell *\/ */
/* 	for(joff=0; joff<ncells; joff++) for(ioff=0; ioff<ncells; ioff++) { */
/* 	  if(ioff == 1 && joff == 1) continue; */
/* 	  jj = jm1 + joff; */
/* 	  ii = im1 + ioff; */
/* 	  dir = pdir[n][jj*nxp2+ii]; */
/* 	  /\* consider about the rotation when it is on the boundary *\/ */
/* 	  if(dir >=0 ) { */
/* 	    if( opcode & CUBIC_GRID ) { */
/* 	      if(ii == 0) { /\* west *\/  */
/* 		if(n%2==0) dir = (dir+2)%8;  /\* tile 1 3 5 *\/ */
/* 	      } */
/* 	      else if(ii == nxp) { /\*east *\/ */
/* 		if(n%2==1) dir = (dir+2)%8;  /\* tile 2 4 6 *\/ */
/* 	      } */
/* 	      else if(jj == 0) { /\* south *\/ */
/* 		if(n%2==1) dir = (dir+6)%8;  /\* tile 2 4 6 *\/ */
/* 	      } */
/* 	      else if(jj == nyp) { /\*north *\/ */
/* 		if(n%2==0) dir = (dir+6)%8;  /\* tile 1 3 5 *\/ */
/* 	      } */
/* 	    } */
/* 	    i_dest = ii + di[dir]; */
/* 	    j_dest = jj + dj[dir]; */
/* 	    if(i_dest == i && j_dest == j) psubA[n][j*nxp2+i] += psubA[n][jj*nxp2+ii]; */
/* 	  } */
/* 	} */
/*       } */
/*     } */
/*     update_halo_double(ntiles, psubA, nx, ny, opcode); */
/*   } */
  
/*   free(pbasin); */
/*   free(ptravel); */
/*   free(psubA); */
/*   free(pdir); */
/* }; /\*  calc_travel *\/ */

/* /\*------------------------------------------------------------------------------ */
/*   void check_river_data( ) */
/*   check to make sure all the river points have been assigned travel and basin value */
/*   and all the ocean points will have missing value. */
/*   ----------------------------------------------------------------------------*\/ */
  
/* void check_river_data(int ntiles, river_type *river_data ) */
/* { */
/*   int maxtravel = -1; */
/*   int maxbasin  = -1; */
/*   int nx, ny, nxp2, nyp2; */
/*   int n, i, j, im1, jm1, ioff, joff, ii, jj; */
/*   int tocell, travel, basin; */
/*   int tocell_missing, travel_missing, basin_missing; */
/*   int ncoast_full_land, nsink; */
/*   double subA, subA_missing; */
/*   int *ncoast; */
  
/*   /\* print out maximum travel and number of rivers *\/ */
/*   maxtravel = -1; */
/*   maxbasin  = -1; */
/*   nx = river_data->nx; */
/*   ny = river_data->ny; */
/*   nxp2 = nx + 2; */
/*   nyp2 = ny + 2; */
/*   subA_missing = river_data->subA_missing; */
/*   tocell_missing = river_data->tocell_missing; */
/*   basin_missing = river_data->basin_missing; */
/*   travel_missing = river_data->travel_missing; */
  
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {     */
/*       maxtravel = max(maxtravel, river_data[n].travel[j*nxp2+i]); */
/*       maxbasin  = max(maxbasin, river_data[n].basin[j*nxp2+i]); */
/*     } */
/*   } */
/*   printf("==> NOTE from river_regrid: maximum travel is %d and maximum basin is %d.\n", maxtravel, maxbasin); */

/*   ncoast_full_land = 0; */
/*   nsink = 0; */
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*       jm1 = j - 1; */
/*       im1 = i -1; */
/*       subA   = river_data[n].subA  [j*nxp2+i]; */
/*       tocell = river_data[n].tocell[j*nxp2+i]; */
/*       travel = river_data[n].travel[j*nxp2+i]; */
/*       basin  = river_data[n].basin [j*nxp2+i]; */
/*       if( river_data[n].landfrac[jm1*nx+im1] == 0) { */
/* 	if(tocell!= tocell_missing || travel != travel_missing || */
/* 	   basin != basin_missing || subA != subA_missing) { */
/* 	  printf("At ocean points (i=%d,j=%d), subA = %f, tocell = %d, travel = %d, basin = %d.\n ", */
/* 		 i, j, subA, tocell, travel, basin); */
/* 	  mpp_error("river_regrid, subA, tocell, travel, or basin is not missing value for some ocean points"); */
/* 	} */
/*       } */
/*       else { */
/* 	if(tocell == tocell_missing || travel == travel_missing || */
/* 	   basin == basin_missing || subA == subA_missing) { */
/* 	  printf("At river points (i=%d,j=%d), subA = %f, tocell = %d, travel = %d, basin = %d.\n ", */
/* 		 i, j, subA, tocell, travel, basin); */
/* 	  mpp_error("river_regrid, subA, tocell, travel, or basin is missing value for some river points"); */
/* 	} */
/*       } */
/*       /\* check if the points with land_frac == 1 and tocell=0, those points should be sink.*\/ */
/*       if(river_data[n].landfrac[jm1*nx+im1] == 1 && tocell == 0) { */
/* 	for(joff=0; joff<ncells; joff++) { */
/* 	  jj = jm1+joff; */
/* 	  for(ioff=0; ioff<ncells; ioff++) { */
/* 	    ii = im1+ioff; */
/*             if(ioff == 1 && joff == 1) continue; */
/* 	    if(river_data[n].tocell[jj*nxp2+ii] == tocell_missing) { */
/* 	      ncoast_full_land++; */
/* 	      printf("At point (%d,%d), tocell = 0 and landfrac = 1 is a coast point\n", i, j); */
/* 	      goto done_check; */
/* 	    } */
/* 	  } */
/* 	} */
/* 	 printf("At point (%d,%d), tocell = 0 and landfrac = 1 is a sink point\n", i, j); */
/*         nsink++; */
/*       done_check: continue;	 */
/*       } */
/*     } */
/*   } */

/*   if(ncoast_full_land > 0) */
/*     printf("Warning from river_regrid: there are %d coast points is a full land cell\n", ncoast_full_land); */
/*   else */
/*     printf("NOTE from river_regrid: there are no coast points is a full land cell\n"); */

/*   if(nsink > 0) */
/*     printf("Warning from river_regrid: there are %d sink points is a full land cell\n", nsink); */
/*   else */
/*     printf("NOTE from river_regrid: there are no sink points is a full land cell\n"); */
  
/*   /\* check river travel to make sure there is one and only one point with travel = 0. *\/ */
/*   ncoast = (int *)malloc(maxbasin*sizeof(int)); */
/*   for(n=0; n<maxbasin; n++) ncoast[n] = 0; */
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*       if(river_data[n].last_point[(j-1)*nx+i-1] ) ++ncoast[river_data[n].basin[j*nxp2+i]-1]; */
/*     } */
/*   } */
/*   for(n=0; n<maxbasin; n++) { */
/*     if(ncoast[n] <= 0) { */
/*       printf("river with basin = %d has no point with travel = 0.\n", n+1); */
/*       mpp_error("river_regrid: some river has no point with travel = 0"); */
/*     } */
/*     else if(ncoast[n] >1) { */
/*       printf("river with basin = %d has more than one point with travel = 0.\n", n+1); */
/*       mpp_error("river_regrid: some river has more than one point with travel = 0"); */
/*     } */
/*   } */
/*   free(ncoast); */
  

/* }; /\* check_river_data *\/ */

/* /\*------------------------------------------------------------------------------ */
/*   void sort_basin(int ntiles, river_type* river_data); */
/*   sorting the basin according to subA */
/*   The river with larger subA at coast point will get smaller basinid. */
/*   The basinid will be reorganized. */
/*   -----------------------------------------------------------------------------*\/ */
/* void sort_basin(int ntiles, river_type* river_data) */
/* { */

/*   int    nx, ny, nxp2, nyp2, i, j, n; */
/*   int    maxbasin, basin_missing, basin; */
/*   int    *rank, *indx; */
/*   double subA_missing, *maxsuba; */
  
/*   river_type *river_tmp; */

/*   nx   = river_data->nx; */
/*   ny   = river_data->ny; */
/*   nxp2 = nx + 2; */
/*   nyp2 = ny + 2; */
/*   river_tmp = (river_type *)malloc(ntiles*sizeof(river_type)); */

/*   /\* calculate maximum basin *\/ */
/*   maxbasin  = -1; */
/*   basin_missing = river_data->basin_missing; */
/*   for(n=0; n<ntiles; n++) { */
/*     for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {     */
/*       maxbasin  = max(maxbasin,  river_data[n].basin[j*nxp2+i]); */
/*     } */
/*   }   */

/*   /\* copy basinid data to the tmp data *\/ */
/*   for(n=0; n<ntiles; n++) { */
/*     river_tmp[n].basin = (int *)malloc(nxp2*nyp2*sizeof(int)); */
/*     for(i=0; i<nxp2*nyp2; i++) { */
/*       river_tmp[n].basin[i] = river_data[n].basin[i]; */
/*     } */
/*   } */

/*   /\* calculate maximum subA for each basin *\/ */
/*   subA_missing = river_data->subA_missing; */
/*   maxsuba = (double *)malloc(maxbasin*sizeof(double)); */
/*   indx    = (int    *)malloc(maxbasin*sizeof(int   )); */
/*   rank    = (int    *)malloc(maxbasin*sizeof(int   )); */
/*   for(n=0; n<maxbasin; n++) { */
/*     maxsuba[n] = subA_missing; */
/*     indx[n]    = n; */
/*     rank[n]    = -1; */
/*   } */
/*   for(n=0; n<ntiles; n++) for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*     if(river_data[n].last_point[(j-1)*nx+i-1]) { /\* coast point *\/ */
/*       basin = river_tmp[n].basin[j*nxp2+i]; */
/*       if( basin > maxbasin || basin < 1) mpp_error("river_regrid: basin should be between 1 and maxbasin"); */
/*       maxsuba[basin-1] = river_data[n].subA[j*nxp2+i]; */
/*     } */
/*   } */

/*   /\* make sure maxsuba is assigned properly *\/ */
/*   for(n=0; n<maxbasin; n++) { */
/*     if(maxsuba[n] == subA_missing) mpp_error("river_regrid: maxsuba is not assigned for some basin"); */
/*   } */
  
/*   /\* sort maxsuba to get the index rank *\/ */
/*   qsort_index(maxsuba, 0, maxbasin-1, indx); */
/*   for(n=0; n<maxbasin; n++) rank[indx[n]] = n; */
/*   for(n=0; n<maxbasin; n++) { */
/*     if(rank[n] < 0) mpp_error("river_regrid: rank should be no less than 0"); */
/*   } */
  
/*   /\* now assign basin according to the index rank *\/ */
/*   for(n=0; n<ntiles; n++) for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) { */
/*     basin = river_tmp[n].basin[j*nxp2+i]; */
/*     if(basin != basin_missing) { */
/*       if( basin > maxbasin || basin < 1) mpp_error("river_regrid: basin should be a positive integer no larger than maxbasin"); */
/*       river_data[n].basin[j*nxp2+i] = maxbasin - rank[basin-1]; */
/*     } */
/*   } */
  
/*   /\* release the memory *\/ */
/*   for(n=0; n<ntiles; n++) free(river_tmp[n].basin); */
/*   free(river_tmp); */
/*   free(rank); */
/*   free(maxsuba); */

/* };/\* sort_basin *\/ */



/* /\*------------------------------------------------------------------------------ */
/*   void write_river_data() */
/*   write out river network output data which is on land grid. */
/*   ----------------------------------------------------------------------------*\/ */
/* void write_river_data(const char *river_src_file, const char *output_file, river_type* river_data, const char *history, int ntiles) */
/* { */
/*   double *subA, *yt, *xt; */
/*   int    *tocell, *travel, *basin; */
/*   int id_subA, id_subA_in, id_tocell, id_tocell_in; */
/*   int id_travel, id_travel_in, id_basin, id_basin_in; */
/*   int id_cellarea, id_celllength, id_landfrac; */
/*   int id_cellarea_in, id_celllength_in, id_x, id_y; */
/*   int id_xaxis, id_yaxis, id_xaxis_in, id_yaxis_in; */
/*   int fid, fid_in, dimid[2]; */
/*   int n, i, j, ii, jj, nx, ny, nxp2; */
  
/*   fid_in = mpp_open(river_src_file, MPP_READ); */
/*   id_subA_in = mpp_get_varid(fid_in, subA_name); */
/*   id_tocell_in = mpp_get_varid(fid_in, tocell_name); */
/*   id_travel_in = mpp_get_varid(fid_in, travel_name); */
/*   id_basin_in = mpp_get_varid(fid_in, basin_name); */
/*   id_cellarea_in = mpp_get_varid(fid_in, cellarea_name); */
/*   id_celllength_in = mpp_get_varid(fid_in, celllength_name); */
/*   id_xaxis_in = mpp_get_varid(fid_in, xaxis_name); */
/*   id_yaxis_in = mpp_get_varid(fid_in, yaxis_name); */
/*   for(n=0; n<ntiles; n++) { */
/*     if(ntiles>1) */
/*       sprintf(river_data[n].filename, "%s.tile%d.nc", output_file, n+1); */
/*     else */
/*       sprintf(river_data[n].filename, "%s.nc", output_file); */

/*     nx = river_data[n].nx; */
/*     ny = river_data[n].ny; */
/*     nxp2 = nx + 2; */
/*     fid = mpp_open(river_data[n].filename, MPP_WRITE); */
/*     mpp_def_global_att(fid, "version", version);   */
/*     mpp_def_global_att(fid, "code_version", tagname); */
/*     mpp_def_global_att(fid, "history", history); */
/*     dimid[1] = mpp_def_dim(fid, gridx_name, nx); */
/*     dimid[0] = mpp_def_dim(fid, gridy_name, ny); */
/*     id_xaxis = mpp_def_var(fid, gridx_name, MPP_DOUBLE, 1, &(dimid[1]), 0); */
/*     id_yaxis = mpp_def_var(fid, gridy_name, MPP_DOUBLE, 1, dimid, 0); */
/*     mpp_copy_var_att(fid_in, id_xaxis_in, fid, id_xaxis); */
/*     mpp_copy_var_att(fid_in, id_yaxis_in, fid, id_yaxis); */
/*     id_subA = mpp_def_var(fid, subA_name, MPP_DOUBLE, 2, dimid , 0); */
/*     mpp_copy_var_att(fid_in, id_subA_in, fid, id_subA); */
/*     id_tocell = mpp_def_var(fid, tocell_name, MPP_INT, 2, dimid, 0); */
/*     mpp_copy_var_att(fid_in, id_tocell_in, fid, id_tocell); */
/*     id_travel = mpp_def_var(fid, travel_name, MPP_INT, 2, dimid , 0); */
/*     mpp_copy_var_att(fid_in, id_travel_in, fid, id_travel); */
/*     id_basin = mpp_def_var(fid, basin_name, MPP_INT, 2, dimid , 0); */
/*     mpp_copy_var_att(fid_in, id_basin_in, fid, id_basin); */
/*     id_cellarea = mpp_def_var(fid, cellarea_name, MPP_DOUBLE, 2, dimid , 0); */
/*     mpp_copy_var_att(fid_in, id_cellarea_in, fid, id_cellarea); */
/*     id_celllength = mpp_def_var(fid, celllength_name, MPP_DOUBLE, 2, dimid , 0); */
/*     mpp_copy_var_att(fid_in, id_celllength_in, fid, id_celllength); */
/*     id_landfrac = mpp_def_var(fid, landfrac_name, MPP_DOUBLE, 2, dimid, 2, */
/* 			      "long_name", "land fraction", "units", "none"); */
/*     id_x        = mpp_def_var(fid, x_name, MPP_DOUBLE, 2, dimid, 2, */
/* 			      "long_name", "Geographic longitude", "units", "degree_east"); */
/*     id_y        = mpp_def_var(fid, y_name, MPP_DOUBLE, 2, dimid, 2, */
/* 			      "long_name", "Geographic latitude", "units", "degree_north");     */
/*     mpp_end_def(fid); */
/*     xt = (double *)malloc(nx*sizeof(double)); */
/*     yt = (double *)malloc(ny*sizeof(double)); */
/*     /\* */
/*        for lat-lon grid, actual lon, lat will be written out, */
/*        for cubic grid, index will be written out */
/*     *\/ */
/*     if(ntiles == 1) { */
/*       for(i=0; i<nx; i++) xt[i] = river_data[n].xt[nxp2+i+1]; */
/*       for(j=0; j<ny; j++) yt[j] = river_data[n].yt[(j+1)*nxp2+1]; */

/*     } */
/*     else { */
/*       for(i=0; i<nx; i++) xt[i] = i+1; */
/*       for(j=0; j<ny; j++) yt[j] = j+1; */
/*     } */
/*     mpp_put_var_value(fid, id_xaxis, xt); */
/*     mpp_put_var_value(fid, id_yaxis, yt); */
/*     free(xt); */
/*     free(yt); */

/*     /\* all the fields is on data domain, so need to copy to compute domain *\/ */
/*     subA = (double *)malloc(nx*ny*sizeof(double)); */
/*     travel = (int *)malloc(nx*ny*sizeof(int)); */
/*     basin  = (int *)malloc(nx*ny*sizeof(int)); */
/*     tocell = (int *)malloc(nx*ny*sizeof(int)); */
/*     xt = (double *)malloc(nx*ny*sizeof(double)); */
/*     yt = (double *)malloc(nx*ny*sizeof(double)); */
/*     for(j=0; j<ny; j++) for(i=0; i<nx; i++) { */
/*       ii = i + 1; */
/*       jj = j + 1; */
/*       subA[j*nx+i] = river_data[n].subA[jj*nxp2+ii]; */
/*       tocell[j*nx+i] = river_data[n].tocell[jj*nxp2+ii]; */
/*       travel[j*nx+i] = river_data[n].travel[jj*nxp2+ii]; */
/*       basin [j*nx+i] = river_data[n].basin [jj*nxp2+ii]; */
/*       xt    [j*nx+i] = river_data[n].xt    [jj*nxp2+ii]; */
/*       yt    [j*nx+i] = river_data[n].yt    [jj*nxp2+ii]; */
/*     } */
    
/*     mpp_put_var_value(fid, id_subA, subA); */
/*     mpp_put_var_value(fid, id_tocell, tocell); */
/*     mpp_put_var_value(fid, id_travel, travel); */
/*     mpp_put_var_value(fid, id_basin, basin); */
/*     mpp_put_var_value(fid, id_x, xt); */
/*     mpp_put_var_value(fid, id_y, yt); */
/*     mpp_put_var_value(fid, id_cellarea, river_data[n].cellarea); */
/*     mpp_put_var_value(fid, id_celllength, river_data[n].celllength); */
/*     mpp_put_var_value(fid, id_landfrac, river_data[n].landfrac); */
/*     mpp_close(fid); */
/*     free(subA); */
/*     free(tocell); */
/*     free(basin); */
/*     free(travel); */
/*     free(xt); */
/*     free(yt); */
/*   } */
/*   mpp_close(fid_in); */

/* };/\* write_river_data *\/ */


/* /\*------------------------------------------------------------------------------ */
/*   double distance(double lon1, double lat1, double lon2, double lat2) */
/*   find the distance of any two grid. */
/*   ----------------------------------------------------------------------------*\/ */
/* double distance(double lon1, double lat1, double lon2, double lat2) */
/* { */
/*   double s1, s2, dx; */
/*   double dist; */
    
/*   dx = lon2 - lon1; */
/*   if(dx > 180.) dx = dx - 360.; */
/*   if(dx < -180.) dx = dx + 360.; */

/*   if(lon1 == lon2) */
/*     dist = fabs(lat2 - lat1) * D2R; */
/*   else if(lat1 == lat2)  */
/*     dist = fabs(dx) * D2R * cos(lat1*D2R); */
/*   else {   /\* diagonal distance *\/ */
/*     s1 =  fabs(dx)  * D2R * cos(lat1*D2R); */
/*     s2 =  fabs(lat2 - lat1) * D2R; */
/*     dist = sqrt(s1*s1+s2*s2); */
/*   } */
/*   dist *= RADIUS; */

/*   return dist; */
/* }     */

/* /\*------------------------------------------------------------------------------ */
/*   void qsort_index(double array[], int start, int end, int rank[]) */
/*   sort array in increasing order and get the rank of each member in the original array */
/*   the array size of array and rank will be size.  */
  
/*   ----------------------------------------------------------------------------*\/ */
/* #define swap(a,b,t) ((t)=(a),(a)=(b),(b)=(t)) */
/* void qsort_index(double array[], int start, int end, int rank[]) */
/* { */
/*    double pivot; */
/*    int tmp_int; */
/*    double tmp_double; */

/*    if (end > start) { */
/*       int l = start + 1; */
/*       int r = end; */
/*       int pivot_index = (start+(end-start)/2); */
/*       swap(array[start], array[pivot_index], tmp_double); /\*** choose arbitrary pivot ***\/ */
/*       swap(rank[start],  rank[pivot_index],  tmp_int); */
/*       pivot = array[start]; */
/*       while(l < r) { */
/*          if (array[l] <= pivot) { */
/*             l++; */
/*          } else { */
/* 	   while(l < r && array[r] >= pivot) /\*** skip superfluous swaps ***\/ */
/* 	     { */
/* 	       --r; */
/* 	     } */
/*             swap(array[l], array[r], tmp_double); */
/* 	    swap(rank[l],  rank[r],  tmp_int); */
/*          } */
/*       } */
/*       if(l != end || array[end] > pivot ) { */
/* 	l--; */
/*         swap(array[start], array[l], tmp_double); */
/*         swap(rank[start],  rank[l],  tmp_int); */
/*         qsort_index(array, start, l, rank); */
/*         qsort_index(array, r, end, rank); */
/*       } */
/*       else { /\* the first element is the largest one *\/ */
/* 	swap(array[start], array[l], tmp_double); */
/*         swap(rank[start],  rank[l],  tmp_int); */
/*         qsort_index(array, start, l-1, rank); */
/*       } */
/*    } */
/* } */
 


