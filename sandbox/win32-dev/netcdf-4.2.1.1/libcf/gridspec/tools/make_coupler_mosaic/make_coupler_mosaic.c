/*
 * Modify land grid to match ocean grid at coast and calculate atmos/land,
 * atmos/ocean, and land/ocean overlaps using the Sutherland-Hodgeman polygon
 * clipping algorithm (Sutherland, I. E. and G. W. Hodgeman, 1974:  Reentrant
 * polygon clipping, CACM, 17(1), 32-42).
 *  Warning, when the atmos grid is cubic grid, the number of model points should be
 *  even to avoid tiling error. I will come back to solve this issue in the future.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include "constant.h"
#include "mpp.h"
#include "mpp_io.h"
#include "tool_util.h"
#include "mpp_domain.h"
#include "mosaic_util.h"
#include "create_xgrid.h"


char *usage[] = {
  "",
  "  make_coupler_mosaic --atmos_mosaic atmos_mosaic.nc --ocean_mosaic ocean_mosaic.nc ",
  "              --ocean_topog ocean_topog.nc [--land_mosaic land_mosaic.nc] ",
  "              [--sea_level #]  [--interp_method #] [--mosaic_name mosaic_name] ",
  "              [--check ]                                                       ",
  " ",
  "make_coupler_mosaic generates three exchange grids for the FMS coupler. The output ",
  "file includes exchange grid files for fluxes between atmosphere and surface (sea ice ",
  "and land), exchange grid files for runoff between land and sea ice. There might be more ",
  "than one exchange grid files between two model solo mosaic because there might be ",
  "multiple tiles in a solo mosaic. All the exchange grid information are between model ",
  "grid, not between supergrid. We assume the refinement ratio between model grid and ",
  "supergrid is 2. Currently we only output the exchange grid on T-cell. ",
  "Besides generate the exchange grid files, make_coupler_mosaic also generate the ",
  "coupler mosaic file (the file name will be mosaic_name.nc) which contains the atmos, ",
  "land and ocean mosaic path, ocean mosaic topog path and exchange grid file path. ",
  "make_coupler_mosaic expects NetCDF format input.",
  " ",
  "make_coupler_mosaic takes the following flags:",
  "",
  "REQUIRED:",
  "",
  "--atmos_mosaic atmos_mosaic.nc specify the atmosphere mosaic information. This file",
  "                               contains list of tile files which specify the grid ",
  "                               information for each tile. Each grid is required to be ",
  "                               regular lon/lat grid. The file name can not be 'mosaic.nc' ",
  "",
  "--ocean_mosaic ocean_mosaic.nc specify the ocean mosaic information. This file",
  "                               contains list of tile files which specify the grid ",
  "                               information for each tile. The file name can not be 'mosaic.nc' ",
  " ",
  "--ocean_topog ocean_topog.nc   specify the topography information for ocean mosaic.",
  "                               The field name of the topography is depth_tile# or depth when ",
  "                               ntiles = 1, The topography data is positive down.",
  " ",
  "OPTIONAL FLAGS",
  "",
  "--land_mosaic land_mosaic.nc   specify the land mosaic information. This file",
  "                               contains list of tile files which specify the grid ",
  "                               information for each tile. Each grid is required to be ",
  "                               regular lon/lat grid. When land_mosaic is not specified,",
  "                               atmosphere mosaic will be used to specify land mosaic.",
  "                               The file name can not be 'mosaic.nc'.",
  " ",
  "--interp_order #               specify the order of conservative interplation. Its value "
  "                               can be 1 ( linear order ) or 2 ( second order ) with default "
  "                               value 2.                                                     "
  "                                                                                            "
  "--sea_level #                  specify the sea level ( in meters ) and its value will be used",
  "                               to determine land/sea mask. When topography of  ",
  "                               a grid cell is less than sea level, this grid cell will be land,",
  "                               otherwise it will be ocean. Default value is 0",
  " ",
  "--mosaic_name mosaic_name      coupler mosaic name. The output coupler mosaic file will be ",
  "                               mosaic_name.nc. default value is 'mosaic'. ",
  " ",  
  "--check                        check the tiling error",
    "", 
  "A sample call to make_coupler_mosaic that makes exchange grids for atmosphere, land and ocean ",
  "mosaic (atmosphere and land are coincident) is: ",
  "",
  "  make_coupler_mosaic --atmos_mosaic atmos_mosaic.nc --ocean_mosaic ocean_mosaic.nc ",
  "                  --ocean_topog ocean_topog.nc",
  "",
  NULL };

#define D2R (M_PI/180.)
#define MAXXGRIDFILE 100 
#define MX 2000  
#define AREA_RATIO_THRESH (1.e-6)
#define TINY_VALUE (1.e-7)
#define TOLORENCE (1.e-4)

char grid_version[] = "0.2";
char tagname[] = "$Name:  $";

int main (int argc, char *argv[])
{
  int c, i, same_mosaic;
  extern char *optarg;
  char *omosaic  = NULL;
  char *amosaic  = NULL;
  char *lmosaic  = NULL;
  char *otopog   = NULL;
  char mosaic_name[STRING] = "mosaic", mosaic_file[STRING];
  char omosaic_name[STRING], amosaic_name[STRING], lmosaic_name[STRING];
  char **otile_name=NULL, **atile_name=NULL, **ltile_name=NULL;
  int x_refine = 2, y_refine = 2;
  int  interp_order = 2;
  int check = 0;
  int errflg = (argc == 1);
  char history[512];
  int nfile_lxo=0, nfile_axo=0, nfile_axl=0;
  char lxo_file[MAXXGRIDFILE][STRING];
  char axo_file[MAXXGRIDFILE][STRING];
  char axl_file[MAXXGRIDFILE][STRING];  
  char amosaic_dir[STRING], amosaic_file[STRING];
  char lmosaic_dir[STRING], lmosaic_file[STRING];
  char omosaic_dir[STRING], omosaic_file[STRING];
  char otopog_dir[STRING], otopog_file[STRING];
  int    ntile_ocn, ntile_atm, ntile_lnd;
  int    *nxo = NULL, *nyo = NULL, *nxa = NULL, *nya = NULL, *nxl = NULL, *nyl = NULL;
  double **xocn = NULL, **yocn = NULL, **xatm = NULL, **yatm = NULL, **xlnd = NULL, **ylnd = NULL;
  double **area_ocn = NULL, **area_lnd = NULL, **area_atm = NULL;
  double **omask = NULL;
  double sea_level = 0.;
  int    lnd_same_as_atm = 0;
  int    option_index = 0;
  double axo_area_sum = 0, axl_area_sum = 0;
  int    ocn_south_ext = 0;
    
  static struct option long_options[] = {
    {"atmos_mosaic",       required_argument, NULL, 'a'},
    {"land_mosaic",        required_argument, NULL, 'l'},
    {"ocean_mosaic",       required_argument, NULL, 'o'},
    {"ocean_topog",        required_argument, NULL, 't'},
    {"sea_level",          required_argument, NULL, 's'},
    {"interp_order",       required_argument, NULL, 'i'},
    {"mosaic_name",        required_argument, NULL, 'm'},
    {"check",              no_argument,       NULL, 'n'},
    {NULL, 0, NULL, 0}
  };

  mpp_init(&argc, &argv);
  mpp_domain_init();
  
  /*
   * process command line
   */

  while ((c = getopt_long(argc, argv, "i:", long_options, &option_index) ) != -1)
    switch (c) {
    case 'a': 
      amosaic = optarg;
      break;
    case 'l': 
      lmosaic = optarg;
      break;
    case 'o':
      omosaic = optarg;
      break;
    case 't':
      otopog = optarg;
      break;
    case 'i':
      interp_order = atoi(optarg);
      break;
    case 's':
      sea_level = atof(optarg);
      break;
    case 'm':
      strcpy(mosaic_name,optarg);
      break;
    case 'n':
      check = 1;
      break;
    case '?':
      errflg++;
    }
  if (errflg || !amosaic || !omosaic || !otopog) {
    char **u = usage;
    while (*u) { fprintf(stderr, "%s\n", *u); u++; }
    exit(2);
  }

  /* interp_order should be 1 or 2 */
  if(interp_order != 1 && interp_order !=2 )mpp_error("make_coupler_mosaic: interp_order should be 1 or 2");
  
  strcpy(history,argv[0]);

  for(i=1;i<argc;i++) {
    strcat(history, " ");
    strcat(history, argv[i]);
  }

  /*if lmosaic is not specifiied, assign amosaic value to it */
  if(!lmosaic) lmosaic = amosaic;
  
  /*mosaic_file can not have the same name as amosaic, lmosaic or omosaic, also the file name of
    amosaic, lmosaic, omosaic can not be "mosaic.nc"
  */
  sprintf(mosaic_file, "%s.nc", mosaic_name);
  get_file_dir_and_name(amosaic, amosaic_dir, amosaic_file);
  get_file_dir_and_name(lmosaic, lmosaic_dir, lmosaic_file);
  get_file_dir_and_name(omosaic, omosaic_dir, omosaic_file);
  get_file_dir_and_name(otopog, otopog_dir, otopog_file);
  if( !strcmp(mosaic_file, amosaic_file) || !strcmp(mosaic_file, lmosaic_file) || !strcmp(mosaic_file, omosaic_file) ) 
    mpp_error("make_coupler_mosaic: mosaic_file can not have the same name as amosaic, lmosaic or omosaic"); 
  if( !strcmp(amosaic_file, "mosaic.nc") || !strcmp(lmosaic_file, "mosaic.nc") || !strcmp(omosaic_file, "mosaic.nc") ) 
    mpp_error("make_coupler_mosaic: the file name of amosaic, lmosaic or omosaic can not be mosaic.nc"); 

  
  /*
   * Read atmosphere grid
   */
  {
    int n, m_fid, g_fid, vid, gid, tid;
    size_t start[4], nread[4];
    char dir[STRING], filename[STRING], file[2*STRING];    

    for(n=0; n<4; n++) {
      start[n] = 0;
      nread[n] = 1;
    }

    m_fid = mpp_open(amosaic, MPP_READ);
    vid = mpp_get_varid(m_fid, "mosaic");    
    mpp_get_var_value(m_fid, vid, amosaic_name);
    ntile_atm  = mpp_get_dimlen(m_fid, "ntiles");
    nxa        = (int *) malloc (ntile_atm*sizeof(int));
    nya        = (int *) malloc (ntile_atm*sizeof(int));
    xatm       = (double **) malloc( ntile_atm*sizeof(double *));
    yatm       = (double **) malloc( ntile_atm*sizeof(double *));
    area_atm   = (double **) malloc( ntile_atm*sizeof(double *));
    atile_name = (char **)malloc(ntile_atm*sizeof(char *));
    /* grid should be located in the same directory of mosaic file */
    get_file_path(amosaic, dir);
    gid = mpp_get_varid(m_fid, "gridfiles");
    tid = mpp_get_varid(m_fid, "gridtiles");
    for(n=0; n<ntile_atm; n++) {
      double *tmpx, *tmpy;
      int i, j;
      
      start[0] = n; start[1] = 0; nread[0] = 1; nread[1] = STRING;
      mpp_get_var_value_block(m_fid, gid, start, nread, filename);
      atile_name[n] = (char *)malloc(STRING*sizeof(char));
      mpp_get_var_value_block(m_fid, tid, start, nread,  atile_name[n]);
      sprintf(file, "%s/%s", dir, filename);
      g_fid = mpp_open(file, MPP_READ);
      nxa[n] = mpp_get_dimlen(g_fid, "nx");
      nya[n] = mpp_get_dimlen(g_fid, "ny");
      if(nxa[n]%x_refine != 0 ) mpp_error("make_coupler_mosaic: atmos supergrid x-size can not be divided by x_refine");
      if(nya[n]%y_refine != 0 ) mpp_error("make_coupler_mosaic: atmos supergrid y-size can not be divided by y_refine");
      nxa[n] /= x_refine;
      nya[n] /= y_refine;
      xatm[n]     = (double *)malloc((nxa[n]+1)*(nya[n]+1)*sizeof(double));
      yatm[n]     = (double *)malloc((nxa[n]+1)*(nya[n]+1)*sizeof(double));
      area_atm[n] = (double *)malloc((nxa[n]  )*(nya[n]  )*sizeof(double));
      tmpx        = (double *)malloc((nxa[n]*x_refine+1)*(nya[n]*y_refine+1)*sizeof(double));
      tmpy        = (double *)malloc((nxa[n]*x_refine+1)*(nya[n]*y_refine+1)*sizeof(double));
      vid = mpp_get_varid(g_fid, "x");
      mpp_get_var_value(g_fid, vid, tmpx);
      vid = mpp_get_varid(g_fid, "y");
      mpp_get_var_value(g_fid, vid, tmpy);      
      for(j = 0; j < nya[n]+1; j++) for(i = 0; i < nxa[n]+1; i++) {
	xatm[n][j*(nxa[n]+1)+i] = tmpx[(j*y_refine)*(nxa[n]*x_refine+1)+i*x_refine];
	yatm[n][j*(nxa[n]+1)+i] = tmpy[(j*y_refine)*(nxa[n]*x_refine+1)+i*x_refine];
      }
      free(tmpx);
      free(tmpy);      
      /*scale grid from degree to radian, because create_xgrid assume the grid is in radians */
      for(i=0; i<(nxa[n]+1)*(nya[n]+1); i++) {
	xatm[n][i] *= D2R;
	yatm[n][i] *= D2R;
      }
      get_grid_area(nxa+n, nya+n, xatm[n], yatm[n], area_atm[n]);
      mpp_close(g_fid);
    }
    mpp_close(m_fid);
  }

  /*
   * Read land grid
   */
  if (strcmp(lmosaic, amosaic) ) { /* land mosaic is different from atmosphere mosaic */
    int n, m_fid, g_fid, vid, gid, tid;
    size_t start[4], nread[4];
    char dir[STRING], filename[STRING], file[2*STRING];   

    for(n=0; n<4; n++) {
      start[n] = 0;
      nread[n] = 1;
    }
    m_fid = mpp_open(lmosaic, MPP_READ);
    vid = mpp_get_varid(m_fid, "mosaic");    
    mpp_get_var_value(m_fid, vid, lmosaic_name);
    ntile_lnd  = mpp_get_dimlen(m_fid, "ntiles");
    nxl        = (int *) malloc (ntile_lnd*sizeof(int));
    nyl        = (int *) malloc (ntile_lnd*sizeof(int));
    xlnd       = (double **) malloc( ntile_lnd*sizeof(double *));
    ylnd       = (double **) malloc( ntile_lnd*sizeof(double *));
    area_lnd   = (double **) malloc( ntile_lnd*sizeof(double *));
    ltile_name = (char **)malloc(ntile_lnd*sizeof(char *));
    /* grid should be located in the same directory of mosaic file */
    get_file_path(lmosaic, dir);
    gid = mpp_get_varid(m_fid, "gridfiles");
    tid = mpp_get_varid(m_fid, "gridtiles");    
    for(n=0; n<ntile_lnd; n++) {
      double *tmpx, *tmpy;
      int i, j;
      
      start[0] = n; start[1] = 0; nread[0] = 1; nread[1] = STRING;
      mpp_get_var_value_block(m_fid, gid, start, nread, filename);
      ltile_name[n] = (char *)malloc(STRING*sizeof(char));
      mpp_get_var_value_block(m_fid, tid, start, nread,  ltile_name[n]);
      sprintf(file, "%s/%s", dir, filename);
      g_fid = mpp_open(file, MPP_READ);
      nxl[n] = mpp_get_dimlen(g_fid, "nx");
      nyl[n] = mpp_get_dimlen(g_fid, "ny");
      if(nxl[n]%x_refine != 0 ) mpp_error("make_coupler_mosaic: land supergrid x-size can not be divided by x_refine");
      if(nyl[n]%y_refine != 0 ) mpp_error("make_coupler_mosaic: land supergrid y-size can not be divided by y_refine");
      nxl[n]      /= x_refine;
      nyl[n]      /= y_refine;
      xlnd[n]     = (double *)malloc((nxl[n]+1)*(nyl[n]+1)*sizeof(double));
      ylnd[n]     = (double *)malloc((nxl[n]+1)*(nyl[n]+1)*sizeof(double));
      area_lnd[n] = (double *)malloc((nxl[n]  )*(nyl[n]  )*sizeof(double));
      tmpx        = (double *)malloc((nxl[n]*x_refine+1)*(nyl[n]*y_refine+1)*sizeof(double));
      tmpy        = (double *)malloc((nxl[n]*x_refine+1)*(nyl[n]*y_refine+1)*sizeof(double));
      vid = mpp_get_varid(g_fid, "x");
      mpp_get_var_value(g_fid, vid, tmpx);
      vid = mpp_get_varid(g_fid, "y");
      mpp_get_var_value(g_fid, vid, tmpy);     
      for(j = 0; j < nyl[n]+1; j++) for(i = 0; i < nxl[n]+1; i++) {
	xlnd[n][j*(nxl[n]+1)+i] = tmpx[(j*y_refine)*(nxl[n]*x_refine+1)+i*x_refine];
	ylnd[n][j*(nxl[n]+1)+i] = tmpy[(j*y_refine)*(nxl[n]*x_refine+1)+i*x_refine];
      }
      free(tmpx);
      free(tmpy);
      /*scale grid from degree to radian, because create_xgrid assume the grid is in radians */
      for(i=0; i<(nxl[n]+1)*(nyl[n]+1); i++) {
	xlnd[n][i] *= D2R;
	ylnd[n][i] *= D2R;
      }
      get_grid_area(nxl+n, nyl+n, xlnd[n], ylnd[n], area_lnd[n]);
      mpp_close(g_fid);
    }
    mpp_close(m_fid);
  }
  else { /* land mosaic is same as atmosphere mosaic */
    ntile_lnd = ntile_atm;
    nxl = nxa;
    nyl = nya;
    xlnd = xatm;
    ylnd = yatm;
    area_lnd = area_atm;
    lnd_same_as_atm = 1;
    strcpy(lmosaic_name, amosaic_name);
    ltile_name = atile_name;
  }

  /*
   * Read ocean grid boundaries and mask (where water is) for each tile within the mosaic.
   */
  {
    int n, ntiles, m_fid, g_fid, t_fid, vid, gid, tid;
    size_t start[4], nread[4];
    char dir[STRING], filename[STRING], file[2*STRING];
    
    for(n=0; n<4; n++) {
      start[n] = 0;
      nread[n] = 1;
    }
    m_fid = mpp_open(omosaic, MPP_READ);
    vid = mpp_get_varid(m_fid, "mosaic");    
    mpp_get_var_value(m_fid, vid, omosaic_name);
    ntile_ocn  = mpp_get_dimlen(m_fid, "ntiles");
    nxo      = (int     *) malloc(ntile_ocn*sizeof(int));
    nyo      = (int     *) malloc(ntile_ocn*sizeof(int));
    xocn     = (double **) malloc(ntile_ocn*sizeof(double *));
    yocn     = (double **) malloc(ntile_ocn*sizeof(double *));
    area_ocn = (double **) malloc(ntile_ocn*sizeof(double *));
    otile_name = (char **) malloc(ntile_ocn*sizeof(char *));
    /* grid should be located in the same directory of mosaic file */
    get_file_path(omosaic, dir);
    gid = mpp_get_varid(m_fid, "gridfiles");
    tid = mpp_get_varid(m_fid, "gridtiles");    

    /* For the purpose of reproducing between processor count, the layout
       is set to (1, npes). */

    for(n=0; n<ntile_ocn; n++) {
      double *tmpx, *tmpy;
      int i, j;
      double min_atm_lat, min_lat;
      
      start[0] = n; start[1] = 0; nread[0] = 1; nread[1] = STRING;
      mpp_get_var_value_block(m_fid, gid, start, nread, filename);
      otile_name[n] = (char *)malloc(STRING*sizeof(char));
      mpp_get_var_value_block(m_fid, tid, start, nread,  otile_name[n]);
      sprintf(file, "%s/%s", dir, filename);
      g_fid = mpp_open(file, MPP_READ);
      nxo[n] = mpp_get_dimlen(g_fid, "nx");
      nyo[n] = mpp_get_dimlen(g_fid, "ny");
      if(nxo[n]%x_refine != 0 ) mpp_error("make_coupler_mosaic: ocean supergrid x-size can not be divided by x_refine");
      if(nyo[n]%y_refine != 0 ) mpp_error("make_coupler_mosaic: ocean supergrid y-size can not be divided by y_refine");
      tmpx        = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
      tmpy        = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
      vid = mpp_get_varid(g_fid, "x");
      mpp_get_var_value(g_fid, vid, tmpx);
      vid = mpp_get_varid(g_fid, "y");
      mpp_get_var_value(g_fid, vid, tmpy);     
   
      /* sometimes the ocean is only covered part of atmosphere, especially not cover
	 the south pole region. In order to get all the exchange grid between atmosXland,
	 we need to extend one point to cover the whole atmosphere. This need the
	 assumption of one-tile ocean. Also we assume the latitude is the along j=0
      */
      if(ntile_ocn == 1) {
	int na;
	for(i=1; i<=nxo[n]; i++) 
	  if(tmpy[i] != tmpy[i-1]) mpp_error("make_coupler_mosaic: latitude is not uniform along j=0");
	/* calculate the minimum of latitude of atmosphere grid */
	min_atm_lat = 9999; /* dummy large value */
	for(na=0; na<ntile_atm; na++) {
	  min_lat = minval_double((nxa[na]+1)*(nya[na]+1), yatm[na]);
	  if(min_atm_lat > min_lat) min_atm_lat = min_lat;
	}
	if(tmpy[0]*D2R > min_atm_lat + TINY_VALUE) { /* extend one point in south direction*/
	  ocn_south_ext = 1;
	}
      }      
      nxo[n] /= x_refine;
      nyo[n] /= y_refine;
      nyo[n] += ocn_south_ext;
      xocn[n]     = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
      yocn[n]     = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
      area_ocn[n] = (double *)malloc((nxo[n]  )*(nyo[n]  )*sizeof(double));

      for(j = 0; j < nyo[n]+1; j++) for(i = 0; i < nxo[n]+1; i++) {
	xocn[n][(j+ocn_south_ext)*(nxo[n]+1)+i] = tmpx[(j*y_refine)*(nxo[n]*x_refine+1)+i*x_refine] * D2R;
	yocn[n][(j+ocn_south_ext)*(nxo[n]+1)+i] = tmpy[(j*y_refine)*(nxo[n]*x_refine+1)+i*x_refine] * D2R;
      }
      if(ocn_south_ext==1) {
	for(i=0; i<nxo[n]+1; i++) {
	  xocn[n][i] = xocn[n][nxo[n]+1+i];
	  yocn[n][i] = min_atm_lat;
	}
      }
      free(tmpx);
      free(tmpy);
      get_grid_area(nxo+n, nyo+n, xocn[n], yocn[n], area_ocn[n]);
      mpp_close(g_fid);
    }
    mpp_close(m_fid);
    
    /* read ocean topography */
    t_fid = mpp_open(otopog, MPP_READ);
    ntiles = mpp_get_dimlen(t_fid, "ntiles");
    if(ntile_ocn != ntiles) mpp_error("make_coupler_mosaic: dimlen ntiles in mosaic file is not the same as dimlen in topog file");
    omask = (double **)malloc(ntile_ocn*sizeof(double *));
    for(n=0; n<ntile_ocn; n++) {
      char name[128];
      int nx, ny, i, j;
      double *depth;

      if(ntiles == 1)
	strcpy(name, "nx");
      else
	sprintf(name, "nx_tile%d", n+1);
      nx = mpp_get_dimlen(t_fid, name);
      if(ntiles == 1)
	strcpy(name, "ny");
      else
	sprintf(name, "ny_tile%d", n+1);
      ny = mpp_get_dimlen(t_fid, name);
      if( nx != nxo[n] || ny+ocn_south_ext != nyo[n]) mpp_error("make_coupler_mosaic: grid size mismatch between mosaic file and topog file");
      if(ntiles == 1)
	strcpy(name, "depth");
      else
        sprintf(name, "depth_tile%d", n+1);
      depth    = (double *)malloc(nx*ny*sizeof(double));
      omask[n] = (double *)malloc(nxo[n]*nyo[n]*sizeof(double));
      vid = mpp_get_varid(t_fid, name);
      mpp_get_var_value(t_fid, vid, depth);
      for(i=0; i<nxo[n]*nyo[n]; i++) omask[n][i] = 0;
      for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
	if(depth[j*nx+i] >sea_level) omask[n][(j+ocn_south_ext)*nx+i] = 1;
      }
      free(depth);
    }
    mpp_close(t_fid);
  }    

  
  /* Either omosaic is different from both lmosaic and amosaic,
     or all the three mosaic are the same
  */
  if(strcmp(omosaic_name, amosaic_name)) { /* omosaic is different from amosaic */
    if(!strcmp(omosaic_name, lmosaic_name)) mpp_error("make_coupler_mosaic: omosaic is the same as lmosaic, "
						      "but different from amosaic.");
    same_mosaic = 0;
  }
  else { /* omosaic is same as amosaic */
    if(strcmp(omosaic_name, lmosaic_name)) mpp_error("make_coupler_mosaic: omosaic is the same as amosaic, "
						      "but different from lmosaic.");
    same_mosaic = 1;
  }
    
    
  /***************************************************************************************
     First generate the exchange grid between atmos mosaic and land/ocean mosaic              
  ***************************************************************************************/
  nfile_axo = 0;
  nfile_axl = 0;
  nfile_lxo = 0;
  {
    int no, nl, na, n;
    size_t  **naxl, **naxo;
    int     ***atmxlnd_ia,   ***atmxlnd_ja,   ***atmxlnd_il,   ***atmxlnd_jl;
    int     ***atmxocn_ia,   ***atmxocn_ja,   ***atmxocn_io,   ***atmxocn_jo;
    double  ***atmxlnd_area, ***atmxlnd_dia,  ***atmxlnd_dja,  ***atmxlnd_dil,  ***atmxlnd_djl;
    double  ***atmxocn_area, ***atmxocn_dia,  ***atmxocn_dja,  ***atmxocn_dio,  ***atmxocn_djo;
    double  ***atmxocn_clon, ***atmxocn_clat, ***atmxlnd_clon, ***atmxlnd_clat;
    double   min_area; 

    naxl         = (size_t ** )malloc(ntile_atm*sizeof(size_t *));
    naxo         = (size_t ** )malloc(ntile_atm*sizeof(size_t *));
    atmxlnd_area = (double ***)malloc(ntile_atm*sizeof(double **));
    atmxlnd_ia   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxlnd_ja   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxlnd_il   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxlnd_jl   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxocn_area = (double ***)malloc(ntile_atm*sizeof(double **));
    atmxocn_ia   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxocn_ja   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxocn_io   = (int    ***)malloc(ntile_atm*sizeof(int    **));
    atmxocn_jo   = (int    ***)malloc(ntile_atm*sizeof(int    **));

    if(interp_order == 2 ) {
      atmxlnd_dia  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxlnd_dja  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxlnd_dil  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxlnd_djl  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_dia  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_dja  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_dio  = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_djo  = (double ***)malloc(ntile_atm*sizeof(double **));      
      atmxlnd_clon = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxlnd_clat = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_clon = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_clat = (double ***)malloc(ntile_atm*sizeof(double **));      
    }
    
    for(na=0; na<ntile_atm; na++) {
      naxl[na]         = (size_t * )malloc(ntile_lnd*sizeof(size_t));
      naxo[na]         = (size_t * )malloc(ntile_ocn*sizeof(size_t));
      atmxlnd_area[na] = (double **)malloc(ntile_lnd*sizeof(double *));
      atmxlnd_ia[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
      atmxlnd_ja[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
      atmxlnd_il[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
      atmxlnd_jl[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
      atmxocn_area[na] = (double **)malloc(ntile_ocn*sizeof(double *));
      atmxocn_ia[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
      atmxocn_ja[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
      atmxocn_io[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
      atmxocn_jo[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
    
      if(interp_order == 2 ) {
	atmxlnd_dia [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	atmxlnd_dja [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	atmxlnd_dil [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	atmxlnd_djl [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	atmxocn_dia [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	atmxocn_dja [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	atmxocn_dio [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	atmxocn_djo [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	atmxlnd_clon[na] = (double **)malloc(ntile_lnd*sizeof(double *));
	atmxlnd_clat[na] = (double **)malloc(ntile_lnd*sizeof(double *));
	atmxocn_clon[na] = (double **)malloc(ntile_ocn*sizeof(double *));
	atmxocn_clat[na] = (double **)malloc(ntile_ocn*sizeof(double *));      
      }

      for(nl=0; nl<ntile_lnd; nl++) {
	atmxlnd_area[na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	atmxlnd_ia  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	atmxlnd_ja  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	atmxlnd_il  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	atmxlnd_jl  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	if(interp_order == 2 ) {
	  atmxlnd_clon[na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxlnd_clat[na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxlnd_dia [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxlnd_dja [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxlnd_dil [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxlnd_djl [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	}
      }
 
      for(no=0; no<ntile_ocn; no++) {
	atmxocn_area[na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	atmxocn_ia  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	atmxocn_ja  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	atmxocn_io  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	atmxocn_jo  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));          
	if(interp_order == 2 ) {
	  atmxocn_clon[na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxocn_clat[na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxocn_dia [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxocn_dja [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxocn_dio [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	  atmxocn_djo [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	}
      }
    }
      
    for(na=0; na<ntile_atm; na++) {
      
      int      l, is, ie, js, je, la, ia, ja, il, jl, io, jo, layout[2];
      int      n0, n1, n2, n3, na_in, nl_in, no_in, n_out, n_out2;
      double   xa_min, ya_min, xo_min, yo_min, xl_min, yl_min, xa_avg;
      double   xa_max, ya_max, xo_max, yo_max, xl_max, yl_max;
      double   xarea;
      double   xa[MV], ya[MV], xl[MV], yl[MV], xo[MV], yo[MV];
      double   x_out[MV], y_out[MV];
      double   atmxlnd_x[MX][MV], atmxlnd_y[MX][MV];
      int      num_v[MX];
      int      axl_i[MX], axl_j[MX], axl_t[MX];
      double   axl_xmin[MX], axl_xmax[MX], axl_ymin[MX], axl_ymax[MX];
      double   axl_area[MX], axl_clon[MX], axl_clat[MX];
      size_t   count;
      domain2D Dom;
      
      for(nl=0; nl<ntile_lnd; nl++) naxl[na][nl] = 0;
      for(no=0; no<ntile_ocn; no++) naxo[na][no] = 0;
      layout[0] = mpp_npes();
      layout[1] = 1;
        
      mpp_define_domain2d(nxa[na]*nya[na], 1, layout, 0, 0, &Dom);
      mpp_get_compute_domain2d(Dom, &is, &ie, &js, &je );
      for(la=is;la<=ie;la++) {
	
	ia = la%nxa[na];
	ja = la/nxa[na];
	n0 = ja    *(nxa[na]+1) + ia;
	n1 = ja    *(nxa[na]+1) + ia+1;
	n2 = (ja+1)*(nxa[na]+1) + ia+1;
	n3 = (ja+1)*(nxa[na]+1) + ia;
	xa[0] = xatm[na][n0]; ya[0] = yatm[na][n0];
        xa[1] = xatm[na][n1]; ya[1] = yatm[na][n1];
	xa[2] = xatm[na][n2]; ya[2] = yatm[na][n2];
        xa[3] = xatm[na][n3]; ya[3] = yatm[na][n3];
	ya_min  = minval_double(4, ya);
	ya_max  = maxval_double(4, ya);
	na_in   = fix_lon(xa, ya, 4, M_PI);
	xa_min  = minval_double(na_in, xa);
	xa_max  = maxval_double(na_in, xa);
	xa_avg  = avgval_double(na_in, xa);
	count = 0;
	for(nl=0; nl<ntile_lnd; nl++) {
	  for(jl = 0; jl < nyl[nl]; jl ++) for(il = 0; il < nxl[nl]; il++) {
	    n0 = jl    *(nxl[nl]+1) + il;
	    n1 = jl    *(nxl[nl]+1) + il+1;
	    n2 = (jl+1)*(nxl[nl]+1) + il+1;
	    n3 = (jl+1)*(nxl[nl]+1) + il;
	    xl[0] = xlnd[nl][n0]; yl[0] = ylnd[nl][n0];
	    xl[1] = xlnd[nl][n1]; yl[1] = ylnd[nl][n1];
	    xl[2] = xlnd[nl][n2]; yl[2] = ylnd[nl][n2];
	    xl[3] = xlnd[nl][n3]; yl[3] = ylnd[nl][n3];
	    yl_min = minval_double(4, yl);
	    yl_max = maxval_double(4, yl);
            if(yl_min >= ya_max || yl_max <= ya_min ) continue;	    
	    nl_in  = fix_lon(xl, yl, 4, xa_avg);
	    xl_min = minval_double(nl_in, xl);
	    xl_max = maxval_double(nl_in, xl);
	    /* xl should in the same range as xa after lon_fix, so no need to
	       consider cyclic condition
	    */
	      	    
	    if(xa_min >= xl_max || xa_max <= xl_min ) continue;	
	    if (  (n_out = clip_2dx2d( xa, ya, na_in, xl, yl, nl_in, x_out, y_out )) > 0 ) {
	      xarea = poly_area(x_out, y_out, n_out);
	      min_area = min(area_lnd[nl][jl*nxl[nl]+il], area_atm[na][la]);
	      if( xarea/min_area > AREA_RATIO_THRESH ) {
		/*  remember the exchange grid vertices */
		for(n=0; n<n_out; n++) {
		  atmxlnd_x[count][n] = x_out[n];
		  atmxlnd_y[count][n] = y_out[n];
		}
		axl_i[count]    = il;
		axl_j[count]    = jl;
		axl_t[count]    = nl;
		num_v[count]    = n_out;
		axl_xmin[count] = minval_double(n_out, x_out);
		axl_xmax[count] = maxval_double(n_out, x_out);
		axl_ymin[count] = minval_double(n_out, y_out);
		axl_ymax[count] = maxval_double(n_out, y_out);
		axl_area[count] = 0;
		if(interp_order == 2) {
		  axl_clon[count] = 0;
		  axl_clat[count] = 0;
		}
		++count;
		if(count>MX) mpp_error("make_coupler_mosaic: count is greater than MX, increase MX");
      	      }
	    }
	  }
	}

	/* calculate atmos/ocean x-cells */
	for(no=0; no<ntile_ocn; no++) {
	  for(jo = 0; jo < nyo[no]; jo++) for(io = 0; io < nxo[no]; io++) {	
	    n0 = jo    *(nxo[no]+1) + io;
	    n1 = jo    *(nxo[no]+1) + io+1;
	    n2 = (jo+1)*(nxo[no]+1) + io+1;
	    n3 = (jo+1)*(nxo[no]+1) + io;
	    xo[0] = xocn[no][n0]; yo[0] = yocn[no][n0];
	    xo[1] = xocn[no][n1]; yo[1] = yocn[no][n1];
	    xo[2] = xocn[no][n2]; yo[2] = yocn[no][n2];
	    xo[3] = xocn[no][n3]; yo[3] = yocn[no][n3];
	    yo_min = minval_double(4, yo);
	    yo_max = maxval_double(4, yo);
	    no_in  = fix_lon(xo, yo, 4, xa_avg);
	    xo_min = minval_double(no_in, xo);
	    xo_max = maxval_double(no_in, xo);
	    if(omask[no][jo*nxo[no]+io] > 0.5) { /* over sea/ice */
	      /* xo should in the same range as xa after lon_fix, so no need to
		 consider cyclic condition
	      */
              if(xa_min >= xo_max || xa_max <= xo_min || yo_min >= ya_max || yo_max <= ya_min ) continue;	    

	      if (  (n_out = clip_2dx2d( xa, ya, na_in, xo, yo, no_in, x_out, y_out )) > 0) {
		xarea = poly_area(x_out, y_out, n_out );
		min_area = min(area_ocn[no][jo*nxo[no]+io], area_atm[na][la]);
		if(xarea/min_area > AREA_RATIO_THRESH) {
	    
		  atmxocn_area[na][no][naxo[na][no]] = xarea;
		  atmxocn_io[na][no][naxo[na][no]]   = io;
		  atmxocn_jo[na][no][naxo[na][no]]   = jo;
		  atmxocn_ia[na][no][naxo[na][no]]   = ia;
		  atmxocn_ja[na][no][naxo[na][no]]   = ja;
		  if(interp_order == 2) {
		    atmxocn_clon[na][no][naxo[na][no]] = poly_ctrlon ( x_out, y_out, n_out, xa_avg);
		    atmxocn_clat[na][no][naxo[na][no]] = poly_ctrlat ( x_out, y_out, n_out );		
		  }
		  ++(naxo[na][no]);
		  if(naxo[na][no] > MAXXGRID) mpp_error("naxo is greater than MAXXGRID, increase MAXXGRID");
		}
	      }
	    }
	    else { /* over land */
	      /* find the overlap of atmxlnd and ocean cell */
	      for(l=0; l<count; l++) {
		if(axl_xmin[l] >= xo_max || axl_xmax[l] <= xo_min || axl_ymin[l] >= ya_max || axl_ymax[l] <= ya_min ) continue;	  
		if((n_out = clip_2dx2d( atmxlnd_x[l], atmxlnd_y[l], num_v[l], xo, yo, no_in, x_out, y_out )) > 0) {
		  xarea = poly_area(x_out, y_out, n_out );
		  min_area = min(area_lnd[axl_t[l]][axl_j[l]*nxl[axl_t[l]]+axl_i[l]], area_atm[na][la]);
		  if(xarea/min_area > AREA_RATIO_THRESH) {
		    axl_area[l] += xarea;
		    if(interp_order == 2) {
		      axl_clon[l] += poly_ctrlon ( x_out, y_out, n_out, xa_avg);
		      axl_clat[l] += poly_ctrlat ( x_out, y_out, n_out);
		    }
		  }
		}
	      }
	    }
	  }
	}
	/* get the exchange grid between land and atmos. */
	for(l=0; l<count; l++) {
	  nl = axl_t[l];
	  min_area = min(area_lnd[nl][axl_j[l]*nxl[nl]+axl_i[l]], area_atm[na][la]);
	  if(axl_area[l]/min_area > AREA_RATIO_THRESH) {
	    atmxlnd_area[na][nl][naxl[na][nl]] = axl_area[l];
	    atmxlnd_ia  [na][nl][naxl[na][nl]] = ia;
	    atmxlnd_ja  [na][nl][naxl[na][nl]] = ja;
	    atmxlnd_il  [na][nl][naxl[na][nl]] = axl_i[l];
	    atmxlnd_jl  [na][nl][naxl[na][nl]] = axl_j[l];
	    if(interp_order == 2) {
	      atmxlnd_clon[na][nl][naxl[na][nl]] = axl_clon[l];
	      atmxlnd_clat[na][nl][naxl[na][nl]] = axl_clat[l];
	    }
	    ++(naxl[na][nl]);
	    if(naxl[na][nl] > MAXXGRID) mpp_error("naxl is greater than MAXXGRID, increase MAXXGRID");
	  }
	}   
      }/* end of la loop */

      mpp_delete_domain2d(&Dom);
    } /* end of na loop */

    /* calculate the centroid of model grid, as well as land_mask and ocean_mask */
    {
      double **l_area, **o_area;
      int    nl, no, ll, lo;
      l_area = (double **)malloc(ntile_lnd*sizeof(double *));
      o_area = (double **)malloc(ntile_ocn*sizeof(double *));
      for(nl =0; nl<ntile_lnd; nl++) {
	l_area[nl] = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	  l_area[nl][ll] = 0;
	}
      }
      for(no =0; no<ntile_ocn; no++) {
	o_area[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	  o_area[no][lo] = 0;
	}
      }
            
      if(interp_order == 1) {
	for(na=0; na<ntile_atm; na++) {
	  for(nl=0; nl<ntile_lnd; nl++) {
	    int nxgrid;
	  
	    nxgrid = naxl[na][nl];
	    mpp_sum_int(1, &nxgrid);
	    if(nxgrid > 0) {
	      double *g_area;
	      int    *g_il, *g_jl;
	      int    ii;
	      g_il = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_jl = (int    *)malloc(nxgrid*sizeof(int   ));	
	      g_area = (double *)malloc(nxgrid*sizeof(double));
	      mpp_gather_field_int   (naxl[na][nl], atmxlnd_il[na][nl], g_il);
	      mpp_gather_field_int   (naxl[na][nl], atmxlnd_jl[na][nl], g_jl);
	      mpp_gather_field_double(naxl[na][nl], atmxlnd_area[na][nl], g_area);
	      for(i=0; i<nxgrid; i++) {
		ii = g_jl[i]*nxl[nl]+g_il[i];
		l_area[nl][ii] += g_area[i];
	      }
	      free(g_il);
	      free(g_jl);
	      free(g_area);
	    }
	  }

	  for(no=0; no<ntile_ocn; no++) {
	    int nxgrid;
	    nxgrid = naxo[na][no];
	    mpp_sum_int(1, &nxgrid);
	    if(nxgrid > 0) {
	      double *g_area;
	      int    *g_io, *g_jo;
	      int    ii;
	      g_io = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_jo = (int    *)malloc(nxgrid*sizeof(int   ));	
	      g_area = (double *)malloc(nxgrid*sizeof(double));
	      mpp_gather_field_int   (naxo[na][no], atmxocn_io[na][no], g_io);
	      mpp_gather_field_int   (naxo[na][no], atmxocn_jo[na][no], g_jo);
	      mpp_gather_field_double(naxo[na][no], atmxocn_area[na][no], g_area);
	      for(i=0; i<nxgrid; i++) {	      
		ii = g_jo[i]*nxo[no]+g_io[i];
		o_area[no][ii] += g_area[i];
	      }
	      free(g_io);
	      free(g_jo);
	      free(g_area);
	    }
	  }
	}
      }
      else { /* interp_order == 2 */
	double **l_clon, **l_clat;
	double **o_clon, **o_clat;
	double  *a_area,  *a_clon,  *a_clat;
	int la;
      
	l_clon = (double **)malloc(ntile_lnd*sizeof(double *));
	l_clat = (double **)malloc(ntile_lnd*sizeof(double *));
	for(nl =0; nl<ntile_lnd; nl++) {
	  l_clon[nl] = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	  l_clat[nl] = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	  for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	    l_clon[nl][ll] = 0;
	    l_clat[nl][ll] = 0;
	  }
	}
	o_clon = (double **)malloc(ntile_ocn*sizeof(double *));
	o_clat = (double **)malloc(ntile_ocn*sizeof(double *));
	for(no =0; no<ntile_ocn; no++) {
	  o_clon[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	  o_clat[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	  for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	    o_clon[no][lo] = 0;
	    o_clat[no][lo] = 0;
	  }
	}	
	for(na=0; na<ntile_atm; na++) {
	  //	double *area, *clon, *clat;
      
	  a_area = (double *)malloc(nxa[na]*nya[na]*sizeof(double));
	  a_clon = (double *)malloc(nxa[na]*nya[na]*sizeof(double));
	  a_clat = (double *)malloc(nxa[na]*nya[na]*sizeof(double));
	  for(la=0; la<nxa[na]*nya[na]; la++) {
	    a_area[la] = 0;
	    a_clon[la] = 0;
	    a_clat[la] = 0;
	  }

	  for(nl=0; nl<ntile_lnd; nl++) {
	    int nxgrid;
	  
	    nxgrid = naxl[na][nl];
	    mpp_sum_int(1, &nxgrid);
	    if(nxgrid > 0) {
	      double *g_area, *g_clon, *g_clat;
	      int    *g_ia,   *g_ja,   *g_il, *g_jl;
	      int    ii;
	      g_ia = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_ja = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_il = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_jl = (int    *)malloc(nxgrid*sizeof(int   ));	
	      g_area = (double *)malloc(nxgrid*sizeof(double));
	      g_clon = (double *)malloc(nxgrid*sizeof(double));
	      g_clat = (double *)malloc(nxgrid*sizeof(double));
	      mpp_gather_field_int   (naxl[na][nl], atmxlnd_ia[na][nl], g_ia);
	      mpp_gather_field_int   (naxl[na][nl], atmxlnd_ja[na][nl], g_ja);
	      mpp_gather_field_int   (naxl[na][nl], atmxlnd_il[na][nl], g_il);
	      mpp_gather_field_int   (naxl[na][nl], atmxlnd_jl[na][nl], g_jl);
	      mpp_gather_field_double(naxl[na][nl], atmxlnd_area[na][nl], g_area);
	      mpp_gather_field_double(naxl[na][nl], atmxlnd_clon[na][nl], g_clon);
	      mpp_gather_field_double(naxl[na][nl], atmxlnd_clat[na][nl], g_clat);
	      for(i=0; i<nxgrid; i++) {
		ii = g_ja[i]*nxa[na]+g_ia[i];
		a_area[ii] += g_area[i];
		a_clon[ii] += g_clon[i];
		a_clat[ii] += g_clat[i];
		ii = g_jl[i]*nxl[nl]+g_il[i];
		l_area[nl][ii] += g_area[i];
		l_clon[nl][ii] += g_clon[i];
		l_clat[nl][ii] += g_clat[i];
	      }
	      free(g_ia);
	      free(g_ja);
	      free(g_il);
	      free(g_jl);
	      free(g_area);
	      free(g_clon);
	      free(g_clat);
	    }
	  }

	  for(no=0; no<ntile_ocn; no++) {
	    int nxgrid;
	    nxgrid = naxo[na][no];
	    mpp_sum_int(1, &nxgrid);
	    if(nxgrid > 0) {
	      double *g_area, *g_clon, *g_clat;
	      int    *g_ia,   *g_ja,   *g_io, *g_jo;
	      int    ii;
	      g_ia = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_ja = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_io = (int    *)malloc(nxgrid*sizeof(int   ));
	      g_jo = (int    *)malloc(nxgrid*sizeof(int   ));	
	      g_area = (double *)malloc(nxgrid*sizeof(double));
	      g_clon = (double *)malloc(nxgrid*sizeof(double));
	      g_clat = (double *)malloc(nxgrid*sizeof(double));
	      mpp_gather_field_int   (naxo[na][no], atmxocn_ia[na][no], g_ia);
	      mpp_gather_field_int   (naxo[na][no], atmxocn_ja[na][no], g_ja);
	      mpp_gather_field_int   (naxo[na][no], atmxocn_io[na][no], g_io);
	      mpp_gather_field_int   (naxo[na][no], atmxocn_jo[na][no], g_jo);
	      mpp_gather_field_double(naxo[na][no], atmxocn_area[na][no], g_area);
	      mpp_gather_field_double(naxo[na][no], atmxocn_clon[na][no], g_clon);
	      mpp_gather_field_double(naxo[na][no], atmxocn_clat[na][no], g_clat);
	      for(i=0; i<nxgrid; i++) {	      
		ii = g_ja[i]*nxa[na]+g_ia[i];
		a_area[ii] += g_area[i];
		a_clon[ii] += g_clon[i];
		a_clat[ii] += g_clat[i];
		ii = g_jo[i]*nxo[no]+g_io[i];
		o_area[no][ii] += g_area[i];
		o_clon[no][ii] += g_clon[i];
		o_clat[no][ii] += g_clat[i];
	      }
	      free(g_ia);
	      free(g_ja);
	      free(g_io);
	      free(g_jo);
	      free(g_area);
	      free(g_clon);
	      free(g_clat);
	    }
	  }

	  for(la=0; la<nxa[na]*nya[na]; la++) {
	    if(a_area[la] > 0) {
	      a_clon[la] /= a_area[la];
	      a_clat[la] /= a_area[la];
	    }
	  }
	
	  /* substract atmos centroid to get the centroid distance between atmos grid and exchange grid. */
	  for(nl=0; nl<ntile_lnd; nl++) {
	    for(i=0; i<naxl[na][nl]; i++) {
	      la = atmxlnd_ja[na][nl][i]*nxa[na] + atmxlnd_ia[na][nl][i];
	      atmxlnd_dia[na][nl][i] = atmxlnd_clon[na][nl][i]/atmxlnd_area[na][nl][i] - a_clon[la];
	      atmxlnd_dja[na][nl][i] = atmxlnd_clat[na][nl][i]/atmxlnd_area[na][nl][i] - a_clat[la];
	    }
	  }
	  for(no=0; no<ntile_ocn; no++) {
	    for(i=0; i<naxo[na][no]; i++) {
	      la = atmxocn_ja[na][no][i]*nxa[na] + atmxocn_ia[na][no][i];
	      atmxocn_dia[na][no][i] = atmxocn_clon[na][no][i]/atmxocn_area[na][no][i] - a_clon[la];
	      atmxocn_dja[na][no][i] = atmxocn_clat[na][no][i]/atmxocn_area[na][no][i] - a_clat[la];
	    }
	  }
	
	  free(a_area);
	  free(a_clon);
	  free(a_clat);
	}

      
	/* centroid distance from exchange grid to land grid */
	for(nl=0; nl<ntile_lnd; nl++) {
	  for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	    if(l_area[nl][ll] > 0) {
	      l_clon[nl][ll] /= l_area[nl][ll];
	      l_clat[nl][ll] /= l_area[nl][ll];
	    }
	  }
	  for(na=0; na<ntile_atm; na++) {
	    for(i=0; i<naxl[na][nl]; i++) {
	      ll = atmxlnd_jl[na][nl][i]*nxl[nl] + atmxlnd_il[na][nl][i];
	      atmxlnd_dil[na][nl][i] = atmxlnd_clon[na][nl][i]/atmxlnd_area[na][nl][i] - l_clon[nl][ll];
	      atmxlnd_djl[na][nl][i] = atmxlnd_clat[na][nl][i]/atmxlnd_area[na][nl][i] - l_clat[nl][ll];
	    }
	  }
	  free(l_clon[nl]);
	  free(l_clat[nl]);
	}

	/* centroid distance from exchange grid to ocean grid */
	for(no=0; no<ntile_ocn; no++) {
	  for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	    if(o_area[no][lo] > 0) {
	      o_clon[no][lo] /= o_area[no][lo];
	      o_clat[no][lo] /= o_area[no][lo];
	    }
	  }
	  for(na=0; na<ntile_atm; na++) {
	    for(i=0; i<naxo[na][no]; i++) {
	      lo = atmxocn_jo[na][no][i]*nxo[no] + atmxocn_io[na][no][i];
	      atmxocn_dio[na][no][i] = atmxocn_clon[na][no][i]/atmxocn_area[na][no][i] - o_clon[no][lo];
	      atmxocn_djo[na][no][i] = atmxocn_clat[na][no][i]/atmxocn_area[na][no][i] - o_clat[no][lo];
	    }
	  }
	  free(o_clon[no]);
	  free(o_clat[no]);
	}
	free(o_clon);
	free(o_clat);
	free(l_clon);
	free(l_clat);  
      }

      /* calculate ocean_frac and compare ocean_frac with omask */
      /* also write out ocn_frac */
      {
	int    io, jo;
	double ocn_frac;
	int    id_mask, fid, dims[2];
	char ocn_mask_file[STRING];
	double *mask;
	int ny;


	for(no=0; no<ntile_ocn; no++) {
	  ny = nyo[no]-ocn_south_ext;
	  mask = (double *)malloc(nxo[no]*ny*sizeof(double));
	  for(jo=0; jo<ny; jo++) for(io=0; io<nxo[no]; io++) {
	    i = (jo+ocn_south_ext)*nxo[no]+io;
	    ocn_frac = o_area[no][i]/area_ocn[no][i];
	    if( fabs(omask[no][i] - ocn_frac) > TOLORENCE ) {
	      printf("at ocean point (%d,%d), omask = %f, ocn_frac = %f, diff = %f\n",
		     io, jo, omask[no][i], ocn_frac, omask[no][i] - ocn_frac);
	      mpp_error("make_coupler_mosaic: omask is not equal ocn_frac");
	    }
	    mask[jo*nxo[no]+io] = ocn_frac;
	  }
	  if(ntile_ocn > 1)
	    sprintf(ocn_mask_file, "ocean_mask_tile%d.nc", no+1);
	  else
	    strcpy(ocn_mask_file, "ocean_mask.nc");
	  fid = mpp_open(ocn_mask_file, MPP_WRITE);
	  mpp_def_global_att(fid, "grid_version", grid_version);
	  mpp_def_global_att(fid, "code_version", tagname);
	  mpp_def_global_att(fid, "history", history);
          	  

	  dims[1] = mpp_def_dim(fid, "nx", nxo[no]); 
	  dims[0] = mpp_def_dim(fid, "ny", ny);
	  id_mask = mpp_def_var(fid, "mask", MPP_DOUBLE, 2, dims,  2, "standard_name",
				"ocean fraction at T-cell centers", "units", "none");
	  mpp_end_def(fid);
	  mpp_put_var_value(fid, id_mask, mask);
	  mpp_close(fid);
	  free(mask);
	}
      }

      /* calculate land_frac and  write out land_frac */
      {
	int    il, jl;
	int    id_mask, fid, dims[2];
	char lnd_mask_file[STRING];
	double *mask;
	
	for(nl=0; nl<ntile_lnd; nl++) {
	  mask = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	  for(jl=0; jl<nyl[nl]; jl++) for(il=0; il<nxl[nl]; il++) {
	    i = jl*nxl[nl]+il;
	    mask[i] = l_area[nl][i]/area_lnd[nl][i];
	  }
	  if(ntile_lnd > 1)
	    sprintf(lnd_mask_file, "land_mask_tile%d.nc", nl+1);
	  else
	    strcpy(lnd_mask_file, "land_mask.nc");
	  fid = mpp_open(lnd_mask_file, MPP_WRITE);
	  mpp_def_global_att(fid, "grid_version", grid_version);
	  mpp_def_global_att(fid, "code_version", tagname);
	  mpp_def_global_att(fid, "history", history);
	  dims[1] = mpp_def_dim(fid, "nx", nxl[nl]); 
	  dims[0] = mpp_def_dim(fid, "ny", nyl[nl]);
	  id_mask = mpp_def_var(fid, "mask", MPP_DOUBLE, 2, dims,  2, "standard_name",
				"land fraction at T-cell centers", "units", "none");
	  mpp_end_def(fid);
	  mpp_put_var_value(fid, id_mask, mask);
	  free(mask);
	  mpp_close(fid);
	}
      }        
      
      for(nl=0; nl<ntile_lnd; nl++) free(l_area[nl]);
      for(no=0; no<ntile_ocn; no++) free(o_area[no]);      
      free(o_area);
      free(l_area);
    }
    
  
    for(na=0; na<ntile_atm; na++) {
    /* write out atmXlnd data*/
      for(nl = 0; nl < ntile_lnd; nl++) {
	int nxgrid;
	nxgrid = naxl[na][nl];
	mpp_sum_int(1, &nxgrid);
	if(nxgrid>0) {
	  size_t start[4], nwrite[4];
	  int *gdata_int;
	  double *gdata_dbl;
	  
	  int fid, dim_string, dim_ncells, dim_two, dims[4];
	  int id_xgrid_area, id_contact, n;
	  int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
	  char contact[STRING];

	  for(i=0; i<4; i++) {
	    start[i] = 0; nwrite[i] = 1;
	  }	  	  
	  if(same_mosaic)
	    sprintf(axl_file[nfile_axl], "atm_%s_%sXlnd_%s_%s.nc", amosaic_name, atile_name[na], lmosaic_name, ltile_name[nl]);
	  else
	    sprintf(axl_file[nfile_axl], "%s_%sX%s_%s.nc", amosaic_name, atile_name[na], lmosaic_name, ltile_name[nl]);
	  sprintf(contact, "%s:%s::%s:%s", amosaic_name, atile_name[na], lmosaic_name, ltile_name[nl]);
	  fid = mpp_open(axl_file[nfile_axl], MPP_WRITE);
	  mpp_def_global_att(fid, "grid_version", grid_version);
	  mpp_def_global_att(fid, "code_version", tagname);
	  mpp_def_global_att(fid, "history", history);
	  dim_string = mpp_def_dim(fid, "string", STRING);
	  dim_ncells = mpp_def_dim(fid, "ncells", nxgrid);
	  dim_two    = mpp_def_dim(fid, "two", 2);
	  if(interp_order == 2) {
	    id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 7, "standard_name", "grid_contact_spec",
				     "contact_type", "exchange", "parent1_cell",
				     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area", 
				     "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	  }
	  else {
	    id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 5, "standard_name", "grid_contact_spec",
				     "contact_type", "exchange", "parent1_cell",
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

	  /* the index will start from 1, instead of 0 ( fortran index) */
	  for(i = 0;i < naxl[na][nl]; i++) {
	    ++(atmxlnd_ia[na][nl][i]);
	    ++(atmxlnd_ja[na][nl][i]);
	    ++(atmxlnd_il[na][nl][i]);
	    ++(atmxlnd_jl[na][nl][i]);
	  }
	  nwrite[0] = strlen(contact);
	  mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
     	  nwrite[0] = nxgrid;

	  gdata_int = (int *)malloc(nxgrid*sizeof(int));
	  gdata_dbl = (double *)malloc(nxgrid*sizeof(double));

	  mpp_gather_field_double(naxl[na][nl], atmxlnd_area[na][nl], gdata_dbl);
	  if(check) {
	    for(n=0; n<nxgrid; n++) axl_area_sum += gdata_dbl[n];
	  }
	  mpp_put_var_value(fid, id_xgrid_area, gdata_dbl);
	  mpp_gather_field_int(naxl[na][nl], atmxlnd_ia[na][nl], gdata_int);
	  mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	  mpp_gather_field_int(naxl[na][nl], atmxlnd_il[na][nl], gdata_int);
	  mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	  start[1] = 1;
	  mpp_gather_field_int(naxl[na][nl], atmxlnd_ja[na][nl], gdata_int);
	  mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	  mpp_gather_field_int(naxl[na][nl], atmxlnd_jl[na][nl], gdata_int);
	  mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	  if(interp_order == 2) {
	    start[1] = 0;
  	    mpp_gather_field_double(naxl[na][nl], atmxlnd_dia[na][nl], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
	    mpp_gather_field_double(naxl[na][nl], atmxlnd_dil[na][nl], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	    start[1] = 1;
  	    mpp_gather_field_double(naxl[na][nl], atmxlnd_dja[na][nl], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
	    mpp_gather_field_double(naxl[na][nl], atmxlnd_djl[na][nl], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	  }
	  mpp_close(fid);
	  free(gdata_int);
	  free(gdata_dbl);
	  ++nfile_axl;
	}
      } /* end of nl loop */

      /* write out atmXocn data */
      for(no = 0; no < ntile_ocn; no++) {
	int nxgrid;
	
	nxgrid = naxo[na][no];
	mpp_sum_int(1, &nxgrid);
	if(nxgrid>0) {
	  size_t start[4], nwrite[4];
	  int *gdata_int;
	  double *gdata_dbl;
	  int fid, dim_string, dim_ncells, dim_two, dims[4];
	  int id_xgrid_area, id_contact, n;
	  int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;	  
	  char contact[STRING];

	  for(i=0; i<4; i++) {
	    start[i] = 0; nwrite[i] = 1;
	  }
	  
	  if(same_mosaic)
	    sprintf(axo_file[nfile_axo], "atm_%s_%sXocn_%s_%s.nc", amosaic_name, atile_name[na], omosaic_name, otile_name[no]);
	  else
	    sprintf(axo_file[nfile_axo], "%s_%sX%s_%s.nc", amosaic_name, atile_name[na], omosaic_name, otile_name[no]);
	  
	  sprintf(contact, "%s:%s::%s:%s", amosaic_name, atile_name[na], omosaic_name, otile_name[no]);
	  fid = mpp_open(axo_file[nfile_axo], MPP_WRITE);
	  mpp_def_global_att(fid, "grid_version", grid_version);
          mpp_def_global_att(fid, "code_version", tagname);
	  mpp_def_global_att(fid, "history", history);
	  dim_string = mpp_def_dim(fid, "string", STRING);
	  dim_ncells = mpp_def_dim(fid, "ncells", nxgrid);
	  dim_two    = mpp_def_dim(fid, "two", 2);
	  if(interp_order == 2) {
	    id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 7, "standard_name", "grid_contact_spec",
				   "contact_type", "exchange", "parent1_cell",
				   "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area", 
				   "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	  }
	  else {
	    id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 5, "standard_name", "grid_contact_spec",
				   "contact_type", "exchange", "parent1_cell",
				   "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area" );
	  }
	  dims[0] = dim_ncells; dims[1] = dim_two;
	  id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
	  id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
	  id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
				      "exchange_grid_area", "units", "m2");
	  if(interp_order == 2) {
	    id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name",
					"distance_from_parent1_cell_centroid");
	    id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name",
					"distance_from_parent2_cell_centroid");
	  }
	  mpp_end_def(fid);

	  /* the index will start from 1, instead of 0 ( fortran index) */
	  for(i = 0;i < naxo[na][no]; i++) {
	    ++(atmxocn_ia[na][no][i]);
	    ++(atmxocn_ja[na][no][i]);
	    ++(atmxocn_io[na][no][i]);
	    atmxocn_jo[na][no][i] += 1-ocn_south_ext; /* possible one artificial j-level is added at south end */
	  }

          nwrite[0] = strlen(contact);
	  mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	
	  nwrite[0] = nxgrid;

	  gdata_int = (int *)malloc(nxgrid*sizeof(int));
	  gdata_dbl = (double *)malloc(nxgrid*sizeof(double));

	  mpp_gather_field_double(naxo[na][no], atmxocn_area[na][no], gdata_dbl);
	  if(check) {
	    for(n=0; n<nxgrid; n++) axo_area_sum += gdata_dbl[n];
	  }
	  mpp_put_var_value_block(fid, id_xgrid_area, start, nwrite, gdata_dbl);
	  mpp_gather_field_int(naxo[na][no], atmxocn_ia[na][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	  mpp_gather_field_int(naxo[na][no], atmxocn_io[na][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	  start[1] = 1;
	  mpp_gather_field_int(naxo[na][no], atmxocn_ja[na][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	  mpp_gather_field_int(naxo[na][no], atmxocn_jo[na][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	  if(interp_order == 2) {
	    start[1] = 0;
  	    mpp_gather_field_double(naxo[na][no], atmxocn_dia[na][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
	    mpp_gather_field_double(naxo[na][no], atmxocn_dio[na][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	    start[1] = 1;
  	    mpp_gather_field_double(naxo[na][no], atmxocn_dja[na][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
	    mpp_gather_field_double(naxo[na][no], atmxocn_djo[na][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	  }
	  mpp_close(fid);
     	  free(gdata_int);
	  free(gdata_dbl);
	  ++nfile_axo;
	}
      } /* end of no loop */
      
    } /* end of na loop */

    /*release the memory */
    for(na=0; na<ntile_atm; na++) {
      for(nl=0; nl<ntile_lnd; nl++) {
	free(atmxlnd_area[na][nl]);
	free(atmxlnd_ia  [na][nl]);
	free(atmxlnd_ja  [na][nl]);
	free(atmxlnd_il  [na][nl]);
	free(atmxlnd_jl  [na][nl]);
	if(interp_order == 2) {
	  free(atmxlnd_clon[na][nl]);
	  free(atmxlnd_clat[na][nl]);
	  free(atmxlnd_dia [na][nl]);
	  free(atmxlnd_dja [na][nl]);
	  free(atmxlnd_dil [na][nl]);
	  free(atmxlnd_djl [na][nl]);
	}
      }
      free(atmxlnd_area[na]);
      free(atmxlnd_ia  [na]);
      free(atmxlnd_ja  [na]);
      free(atmxlnd_il  [na]);
      free(atmxlnd_jl  [na]);
      if(interp_order == 2) {
	free(atmxlnd_clon[na]);
	free(atmxlnd_clat[na]);
	free(atmxlnd_dia [na]);
	free(atmxlnd_dja [na]);
	free(atmxlnd_dil [na]);
	free(atmxlnd_djl [na]);
      }
      for(no=0; no<ntile_ocn; no++) {
	free(atmxocn_area[na][no]);
	free(atmxocn_ia  [na][no]);
	free(atmxocn_ja  [na][no]);
	free(atmxocn_io  [na][no]);
	free(atmxocn_jo  [na][no]);
	if(interp_order == 2) {
	  free(atmxocn_clon[na][no]);
	  free(atmxocn_clat[na][no]);
	  free(atmxocn_dia [na][no]);
	  free(atmxocn_dja [na][no]);
	  free(atmxocn_dio [na][no]);
	  free(atmxocn_djo [na][no]);
	}
      }
      free(atmxocn_area[na]);
      free(atmxocn_ia  [na]);
      free(atmxocn_ja  [na]);
      free(atmxocn_io  [na]);
      free(atmxocn_jo  [na]);
      if(interp_order == 2) {
	free(atmxocn_clon[na]);
	free(atmxocn_clat[na]);
	free(atmxocn_dia [na]);
	free(atmxocn_dja [na]);
	free(atmxocn_dio [na]);
	free(atmxocn_djo [na]);
      }
      free(naxl[na]);
      free(naxo[na]);
    }    
    free(atmxlnd_area);
    free(atmxlnd_ia  );
    free(atmxlnd_ja  );
    free(atmxlnd_il  );
    free(atmxlnd_jl  );
    free(atmxocn_area);
    free(atmxocn_ia  );
    free(atmxocn_ja  );
    free(atmxocn_io  );
    free(atmxocn_jo  );   
    if(interp_order == 2) {
      free(atmxlnd_clon);
      free(atmxlnd_clat);
      free(atmxlnd_dja );
      free(atmxlnd_dia );
      free(atmxlnd_dil );
      free(atmxlnd_djl );
      free(atmxocn_clon);
      free(atmxocn_clat);
      free(atmxocn_dia );
      free(atmxocn_dja );
      free(atmxocn_dio );
      free(atmxocn_djo );
    }
    free(naxl);
    free(naxo);
  }
  if(mpp_pe() == mpp_root_pe()) printf("\nNOTE from make_coupler_mosaic: Complete the process to create exchange grids "
				       "for fluxes between atmosphere and surface (sea ice and land)\n" );
  
  /***************************************************************************************
     Then generate the exchange grid between land mosaic and ocean mosaic
     if land mosaic is different from atmos mosaic
  ***************************************************************************************/
  nfile_lxo = 0;
  if( !lnd_same_as_atm ) {
    int     no, nl, ll, lo;
    size_t  **nlxo;
    int     ***lndxocn_il, ***lndxocn_jl, ***lndxocn_io, ***lndxocn_jo;
    double  ***lndxocn_area, ***lndxocn_dil, ***lndxocn_djl, ***lndxocn_dio, ***lndxocn_djo;
    double  ***lndxocn_clon, ***lndxocn_clat;
    double  min_area;

    nlxo         = (size_t ** )malloc(ntile_lnd*sizeof(size_t * ));
    lndxocn_area = (double ***)malloc(ntile_lnd*sizeof(double **));
    lndxocn_il   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
    lndxocn_jl   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
    lndxocn_io   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
    lndxocn_jo   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
    if(interp_order == 2) {
      lndxocn_dil  = (double ***)malloc(ntile_lnd*sizeof(double **));
      lndxocn_djl  = (double ***)malloc(ntile_lnd*sizeof(double **));
      lndxocn_dio  = (double ***)malloc(ntile_lnd*sizeof(double **));
      lndxocn_djo  = (double ***)malloc(ntile_lnd*sizeof(double **));
      lndxocn_clon = (double ***)malloc(ntile_lnd*sizeof(double **));
      lndxocn_clat = (double ***)malloc(ntile_lnd*sizeof(double **));
    }
    for(nl=0; nl<ntile_lnd; nl++) {
      nlxo        [nl] = (size_t * )malloc(ntile_ocn*sizeof(size_t  ));
      lndxocn_area[nl] = (double **)malloc(ntile_ocn*sizeof(double *));
      lndxocn_il  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
      lndxocn_jl  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
      lndxocn_io  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
      lndxocn_jo  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
      if(interp_order == 2) {
	lndxocn_dil [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	lndxocn_djl [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	lndxocn_dio [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	lndxocn_djo [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	lndxocn_clon[nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	lndxocn_clat[nl] = (double **)malloc(ntile_ocn*sizeof(double *));
      }
      for(no=0; no<ntile_ocn; no++) {
	lndxocn_area[nl][no] = (double *)malloc(MAXXGRID*sizeof(double));
	lndxocn_il  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	lndxocn_jl  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	lndxocn_io  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	lndxocn_jo  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));          
	if(interp_order == 2 ) {  
	  lndxocn_dil[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	  lndxocn_djl[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	  lndxocn_dio[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	  lndxocn_djo[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	  lndxocn_clon[nl][no] = (double *)malloc(MAXXGRID*sizeof(double *));
	  lndxocn_clat[nl][no] = (double *)malloc(MAXXGRID*sizeof(double *));
	}
      }
    }
 
    for(nl=0; nl<ntile_lnd; nl++) {
      int      il, jl, io, jo, is, ie, js, je, layout[2];
      int      n0, n1, n2, n3, nl_in, no_in, n_out, nxgrid;
      double   xarea, xctrlon, xctrlat;
      double   xl_min, yl_min, xo_min, yo_min, xl_avg;
      double   xl_max, yl_max, xo_max, yo_max;
      double   xl[MV], yl[MV], xo[MV], yo[MV], x_out[MV], y_out[MV];
      domain2D Dom;

      for(no=0; no<ntile_ocn; no++) nlxo[nl][no] = 0;
      
      layout[0] = mpp_npes();
      layout[1] = 1;
        
      mpp_define_domain2d(nxl[nl]*nyl[nl], 1, layout, 0, 0, &Dom);
      mpp_get_compute_domain2d(Dom, &is, &ie, &js, &je );
      for(ll=is;ll<=ie;ll++) {
	il = ll%nxl[nl];
	jl = ll/nxl[nl];
	n0 = jl    *(nxl[nl]+1) + il;
	n1 = jl    *(nxl[nl]+1) + il+1;
	n2 = (jl+1)*(nxl[nl]+1) + il+1;
	n3 = (jl+1)*(nxl[nl]+1) + il;
	xl[0] = xlnd[nl][n0]; yl[0] = ylnd[nl][n0];
	xl[1] = xlnd[nl][n1]; yl[1] = ylnd[nl][n1];
	xl[2] = xlnd[nl][n2]; yl[2] = ylnd[nl][n2];
	xl[3] = xlnd[nl][n3]; yl[3] = ylnd[nl][n3];
	yl_min  = minval_double(4, yl);
	yl_max  = maxval_double(4, yl);
	nl_in   = fix_lon(xl, yl, 4, M_PI);
	xl_min  = minval_double(nl_in, xl);
	xl_max  = maxval_double(nl_in, xl);
	xl_avg  = avgval_double(nl_in, xl);      
	for(no=0; no<ntile_ocn; no++) {
	  for(jo = 0; jo < nyo[no]; jo++) for(io = 0; io < nxo[no]; io++) if(omask[no][jo*nxo[no]+io] > 0.5) {	
	    n0 = jo    *(nxo[no]+1) + io;
	    n1 = jo    *(nxo[no]+1) + io+1;
	    n2 = (jo+1)*(nxo[no]+1) + io+1;
	    n3 = (jo+1)*(nxo[no]+1) + io;
	    xo[0] = xocn[no][n0]; yo[0] = yocn[no][n0];
	    xo[1] = xocn[no][n1]; yo[1] = yocn[no][n1];
	    xo[2] = xocn[no][n2]; yo[2] = yocn[no][n2];
	    xo[3] = xocn[no][n3]; yo[3] = yocn[no][n3];
	    yo_min = minval_double(4, yo);
	    yo_max = maxval_double(4, yo);
            if(yo_min >= yl_max || yo_max <= yl_min ) continue;	    
	    no_in  = fix_lon(xo, yo, 4, xl_avg);
	    xo_min = minval_double(no_in, xo);
	    xo_max = maxval_double(no_in, xo);
	    /* xo should in the same range as xa after lon_fix, so no need to
	       consider cyclic condition
	    */
	    if(xl_min >= xo_max || xl_max <= xo_min ) continue;
	    if (  (n_out = clip_2dx2d( xl, yl, nl_in, xo, yo, no_in, x_out, y_out )) > 0 ){
	      xarea = poly_area(x_out, y_out, n_out );
	      min_area = min(area_ocn[no][jo*nxo[no]+io], area_lnd[nl][ll] );
	      if(xarea/min_area > AREA_RATIO_THRESH ) {
		lndxocn_area[nl][no][nlxo[nl][no]] = xarea;
		lndxocn_io[nl][no][nlxo[nl][no]]   = io;
		lndxocn_jo[nl][no][nlxo[nl][no]]   = jo;
		lndxocn_il[nl][no][nlxo[nl][no]]   = il;
		lndxocn_jl[nl][no][nlxo[nl][no]]   = jl;
		if(interp_order == 2) {
		  lndxocn_clon[nl][no][nlxo[nl][no]] = poly_ctrlon ( x_out, y_out, n_out, xl_avg);
		  lndxocn_clat[nl][no][nlxo[nl][no]] = poly_ctrlat ( x_out, y_out, n_out );
		}
		++(nlxo[nl][no]);
		if(nlxo[nl][no] > MAXXGRID) mpp_error("nlxo is greater than MAXXGRID, increase MAXXGRID");
	      }
	    }
	  } /* end of io, jo loop */
	} 
      } /* end of ll( or il, jl) loop */
      mpp_delete_domain2d(&Dom);
    }/* for(nl=0; nl<ntile_lnd; nl++) */

    /* calculate the centroid of model grid. */
    if(interp_order == 2) {
      double *l_area, *l_clon, *l_clat;
      double **o_area, **o_clon, **o_clat;
 
      o_area = (double **)malloc(ntile_ocn*sizeof(double *));
      o_clon = (double **)malloc(ntile_ocn*sizeof(double *));
      o_clat = (double **)malloc(ntile_ocn*sizeof(double *));
      for(no =0; no<ntile_ocn; no++) {
	o_area[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	o_clon[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	o_clat[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	  o_area[no][lo] = 0;
	  o_clon[no][lo] = 0;
	  o_clat[no][lo] = 0;
	}
      }

      for(nl=0; nl<ntile_lnd; nl++) {
	l_area = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	l_clon = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	l_clat = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	  l_area[ll] = 0;
	  l_clon[ll] = 0;
	  l_clat[ll] = 0;
	}

	for(no=0; no<ntile_ocn; no++) {
	  int nxgrid;
	  nxgrid = nlxo[nl][no];
	  mpp_sum_int(1, &nxgrid);
	  if(nxgrid > 0) {
	    double *g_area, *g_clon, *g_clat;
	    int    *g_il,   *g_jl,   *g_io, *g_jo;
	    int    ii;
	    g_il = (int    *)malloc(nxgrid*sizeof(int   ));
	    g_jl = (int    *)malloc(nxgrid*sizeof(int   ));
	    g_io = (int    *)malloc(nxgrid*sizeof(int   ));
	    g_jo = (int    *)malloc(nxgrid*sizeof(int   ));	
	    g_area = (double *)malloc(nxgrid*sizeof(double));
	    g_clon = (double *)malloc(nxgrid*sizeof(double));
	    g_clat = (double *)malloc(nxgrid*sizeof(double));
	    mpp_gather_field_int   (nlxo[nl][no], lndxocn_il[nl][no], g_il);
	    mpp_gather_field_int   (nlxo[nl][no], lndxocn_jl[nl][no], g_jl);
	    mpp_gather_field_int   (nlxo[nl][no], lndxocn_io[nl][no], g_io);
	    mpp_gather_field_int   (nlxo[nl][no], lndxocn_jo[nl][no], g_jo);
	    mpp_gather_field_double(nlxo[nl][no], lndxocn_area[nl][no], g_area);
	    mpp_gather_field_double(nlxo[nl][no], lndxocn_clon[nl][no], g_clon);
	    mpp_gather_field_double(nlxo[nl][no], lndxocn_clat[nl][no], g_clat);
	    for(i=0; i<nxgrid; i++) {	      
	      ii = g_jl[i]*nxl[nl]+g_il[i];
	      l_area[ii] += g_area[i];
	      l_clon[ii] += g_clon[i];
	      l_clat[ii] += g_clat[i];
	      ii = g_jo[i]*nxo[no]+g_io[i];
	      o_area[no][ii] += g_area[i];
	      o_clon[no][ii] += g_clon[i];
	      o_clat[no][ii] += g_clat[i];
	    }
	    free(g_il);
	    free(g_jl);
	    free(g_io);
	    free(g_jo);
	    free(g_area);
	    free(g_clon);
	    free(g_clat);
	  }
	}
	for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	  if(l_area[ll] > 0) {
	    l_clon[ll] /= l_area[ll];
	    l_clat[ll] /= l_area[ll];
	  }
	}
	/* substract land centroid to get the centroid distance between land grid and exchange grid. */
	for(no=0; no<ntile_ocn; no++) {
	  for(i=0; i<nlxo[nl][no]; i++) {
	    ll = lndxocn_jl[nl][no][i]*nxl[nl] + lndxocn_il[nl][no][i];
	    lndxocn_dil[nl][no][i] = lndxocn_clon[nl][no][i]/lndxocn_area[nl][no][i] - l_clon[ll];
	    lndxocn_djl[nl][no][i] = lndxocn_clat[nl][no][i]/lndxocn_area[nl][no][i] - l_clat[ll];
	  }
	}
	
	free(l_area);
	free(l_clon);
	free(l_clat);	
      }

      /* centroid distance from exchange grid to ocean grid */
      for(no=0; no<ntile_ocn; no++) {
	for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	  if(o_area[no][lo] > 0) {
	    o_clon[no][lo] /= o_area[no][lo];
	    o_clat[no][lo] /= o_area[no][lo];
	  }
	}
	for(nl=0; nl<ntile_lnd; nl++) {
	  for(i=0; i<nlxo[nl][no]; i++) {
	    lo = lndxocn_jo[nl][no][i]*nxo[no] + lndxocn_io[nl][no][i];
	    lndxocn_dio[nl][no][i] = lndxocn_clon[nl][no][i]/lndxocn_area[nl][no][i] - o_clon[no][lo];
	    lndxocn_djo[nl][no][i] = lndxocn_clat[nl][no][i]/lndxocn_area[nl][no][i] - o_clat[no][lo];
	  }
	}
	free(o_area[no]);
	free(o_clon[no]);
	free(o_clat[no]);
      }

      free(o_area);
      free(o_clon);
      free(o_clat);
    }

    /* write out lndXocn data */
    for(nl = 0; nl < ntile_lnd; nl++) {
       for(no = 0; no < ntile_ocn; no++) {
	 int nxgrid;
	 
	/* get total number of exchange grid on all the pes */
	 nxgrid = nlxo[nl][no];
	mpp_sum_int(1, &nxgrid);
	
	if(nxgrid >0) {
	  size_t start[4], nwrite[4];
	  int *gdata_int;
	  double *gdata_dbl;
	  
	  char contact[STRING];
	  int fid, dim_string, dim_ncells, dim_two, dims[4];
	  int id_contact, id_xgrid_area, n;
	  int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;

	  for(i=0; i<4; i++) {
	    start[i] = 0; nwrite[i] = 1;
	  }	  	  
	  
	  if(same_mosaic)
	    sprintf(lxo_file[nfile_lxo], "lnd_%s_%sXocn_%s_%s.nc", lmosaic_name, ltile_name[nl], omosaic_name, otile_name[no]);
	  else
	    sprintf(lxo_file[nfile_lxo], "%s_%sX%s_%s.nc", lmosaic_name, ltile_name[nl], omosaic_name, otile_name[no]);
	  sprintf(contact, "%s:%s::%s:%s", lmosaic_name, ltile_name[nl], omosaic_name, otile_name[no]);

	  fid = mpp_open(lxo_file[nfile_lxo], MPP_WRITE);
	  mpp_def_global_att(fid, "grid_version", grid_version);
          mpp_def_global_att(fid, "code_version", tagname);
	  mpp_def_global_att(fid, "history", history);
	  dim_string = mpp_def_dim(fid, "string", STRING);
	  dim_ncells = mpp_def_dim(fid, "ncells", nxgrid);
	  dim_two    = mpp_def_dim(fid, "two", 2);
	  if(interp_order == 2) {
	    id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 7, "standard_name", "grid_contact_spec",
				     "contact_type", "exchange", "parent1_cell",
				     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area", 
				     "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	  }
	  else {
	    id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 5, "standard_name", "grid_contact_spec",
				     "contact_type", "exchange", "parent1_cell",
				     "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area" );
	  }
	  dims[0] = dim_ncells; dims[1] = dim_two;
	  id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent1_cell_indices");
	  id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent2_cell_indices");
	  id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
				      "exchange_grid_area", "units", "m2");

	  if(interp_order == 2) {
	    id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
	    id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
	  }
	  mpp_end_def(fid);

	  /* the index will start from 1, instead of 0 ( fortran index) */
	  for(i = 0;i < nlxo[nl][no]; i++) {
	    ++(lndxocn_il[nl][no][i]);
	    ++(lndxocn_jl[nl][no][i]);
	    ++(lndxocn_io[nl][no][i]);
	    lndxocn_jo[nl][no][i] += 1 - ocn_south_ext; /* one artificial j-level may be added in the south end */
	  }

	  nwrite[0] = strlen(contact);
	  mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	  nwrite[0] = nxgrid;

	  gdata_int = (int *)malloc(nxgrid*sizeof(int));
	  gdata_dbl = (double *)malloc(nxgrid*sizeof(double));

	  mpp_gather_field_double(nlxo[nl][no], lndxocn_area[nl][no], gdata_dbl);
	  mpp_put_var_value(fid, id_xgrid_area, gdata_dbl);
	  mpp_gather_field_int(nlxo[nl][no], lndxocn_il[nl][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	  mpp_gather_field_int(nlxo[nl][no], lndxocn_io[nl][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	  start[1] = 1;
	  mpp_gather_field_int(nlxo[nl][no], lndxocn_jl[nl][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	  mpp_gather_field_int(nlxo[nl][no], lndxocn_jo[nl][no], gdata_int);
	  mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	  if(interp_order == 2) {
	    start[1] = 0;
  	    mpp_gather_field_double(nlxo[nl][no], lndxocn_dil[nl][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
	    mpp_gather_field_double(nlxo[nl][no], lndxocn_dio[nl][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	    start[1] = 1;
  	    mpp_gather_field_double(nlxo[nl][no], lndxocn_djl[nl][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
	    mpp_gather_field_double(nlxo[nl][no], lndxocn_djo[nl][no], gdata_dbl);
	    mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	  }
	  mpp_close(fid);
	  free(gdata_int);
	  free(gdata_dbl);
	  ++nfile_lxo;
	}
       } /* for(no=0; no<ntile_ocn; no++) */
    } /* for(nl=0; nl<ntile_lnd; nl++) */

    /* release the memory */
    for(nl=0; nl<ntile_lnd; nl++) {
      for(no=0; no<ntile_ocn; no++) {
	free(lndxocn_area[nl][no]);
	free(lndxocn_il  [nl][no]);
	free(lndxocn_jl  [nl][no]);
	free(lndxocn_io  [nl][no]);
	free(lndxocn_jo  [nl][no]);
	if(interp_order == 2) {
	  free(lndxocn_clon[nl][no]);
	  free(lndxocn_clat[nl][no]);
	  free(lndxocn_dil [nl][no]);
	  free(lndxocn_djl [nl][no]);
	  free(lndxocn_dio [nl][no]);
	  free(lndxocn_djo [nl][no]);
	}
      }
      free(lndxocn_area[nl]);
      free(lndxocn_il  [nl]);
      free(lndxocn_jl  [nl]);
      free(lndxocn_io  [nl]);
      free(lndxocn_jo  [nl]);
      if(interp_order == 2) {
	free(lndxocn_clon[nl]);
	free(lndxocn_clat[nl]);
	free(lndxocn_dil [nl]);
	free(lndxocn_djl [nl]);
	free(lndxocn_dio [nl]);
	free(lndxocn_djo [nl]);
      }
      free(nlxo[nl]);
    }
    free(nlxo);
    free(lndxocn_area);
    free(lndxocn_il  );
    free(lndxocn_jl  );
    free(lndxocn_io  );
    free(lndxocn_jo  );   
    if(interp_order == 2) {
      free(lndxocn_clon);
      free(lndxocn_clat);
      free(lndxocn_dil );
      free(lndxocn_djl );
      free(lndxocn_dio );
      free(lndxocn_djo );
    }

    if(mpp_pe() == mpp_root_pe()) printf("\nNOTE from make_coupler_mosaic: Complete the process to create exchange grids "
					 "for runoff between land and sea ice.\n" );
  }
  else {
    nfile_lxo = nfile_axo;
    for(i=0; i<nfile_axo; i++) strcpy(lxo_file[i], axo_file[i]);
    if(mpp_pe() == mpp_root_pe()) printf("\nNOTE from make_coupler_mosaic: Since lmosaic is the same as amosaic, "
					 "no need to compute the exchange grid between lmosaic and omosaic, "
					 "simply use the exchange grid between amosaic and omosaic.\n");    
  }

  if(mpp_pe() == mpp_root_pe()) {
    int n, i;
    double axo_area_frac;
    double axl_area_frac;
    double tiling_area;
    double *atm_area;
    double atm_area_sum;

    if(check) {
      /* for cubic grid, when number of model points is odd, some grid cell area will be negative,
	 need to think about how to solve this issue in the future */
      /*      atm_area_sum = 4*M_PI*RADIUS*RADIUS; */
      atm_area_sum = 0;
      for(n=0; n<ntile_atm; n++) {
	atm_area = (double *)malloc(nxa[n]*nya[n]*sizeof(double));
	get_grid_area(&nxa[n], &nya[n], xatm[n], yatm[n], atm_area);
	for(i=0; i<nxa[n]*nya[n]; i++) atm_area_sum += atm_area[i];
	free(atm_area);
      }
      axo_area_frac = axo_area_sum/atm_area_sum*100;
      axl_area_frac = axl_area_sum/atm_area_sum*100;
      tiling_area   = (atm_area_sum-axo_area_sum-axl_area_sum)/atm_area_sum*100;    
      printf("\nNOTE: axo_area_sum is %f and ocean fraction is %f%%\n", axo_area_sum, axo_area_frac);
      printf("NOTE: axl_area_sum is %f and land  fraction is %f%%\n", axl_area_sum, axl_area_frac);
      printf("NOTE: tiling error is %f%%\n", tiling_area );
    }
  }
  
  /*Fianlly create the coupler mosaic file mosaic_name.nc */
  {
    int fid, dim_string, dim_axo, dim_lxo, dim_axl, dims[4], n;
    size_t start[4], nwrite[4];
    int id_lmosaic_dir, id_lmosaic_file, id_omosaic_dir, id_omosaic_file;
    int id_amosaic_dir, id_amosaic_file, id_otopog_dir, id_otopog_file;
    int id_xgrids_dir, id_axo_file, id_lxo_file, id_axl_file;
    int id_amosaic, id_lmosaic, id_omosaic;
    
    fid = mpp_open(mosaic_file, MPP_WRITE);
    mpp_def_global_att(fid, "grid_version", grid_version);
    mpp_def_global_att(fid, "code_version", tagname);
    mpp_def_global_att(fid, "history", history);
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
    for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1; }
    
    nwrite[0] = strlen(amosaic_dir);
    mpp_put_var_value_block(fid, id_amosaic_dir, start, nwrite, amosaic_dir);
    nwrite[0] = strlen(amosaic_file);
    mpp_put_var_value_block(fid, id_amosaic_file, start, nwrite, amosaic_file);
    nwrite[0] = strlen(amosaic_name);
    mpp_put_var_value_block(fid, id_amosaic, start, nwrite, amosaic_name);
    nwrite[0] = strlen(lmosaic_dir);
    mpp_put_var_value_block(fid, id_lmosaic_dir, start, nwrite, lmosaic_dir);
    nwrite[0] = strlen(lmosaic_file);
    mpp_put_var_value_block(fid, id_lmosaic_file, start, nwrite, lmosaic_file);
    nwrite[0] = strlen(lmosaic_name);
    mpp_put_var_value_block(fid, id_lmosaic, start, nwrite, lmosaic_name);
    nwrite[0] = strlen(omosaic_dir);
    mpp_put_var_value_block(fid, id_omosaic_dir, start, nwrite, omosaic_dir);
    nwrite[0] = strlen(omosaic_file);
    mpp_put_var_value_block(fid, id_omosaic_file, start, nwrite, omosaic_file);
    nwrite[0] = strlen(omosaic_name);
    mpp_put_var_value_block(fid, id_omosaic, start, nwrite, omosaic_name);
    nwrite[0] = strlen(otopog_dir);
    mpp_put_var_value_block(fid, id_otopog_dir, start, nwrite, otopog_dir);
    nwrite[0] = strlen(otopog_file);
    mpp_put_var_value_block(fid, id_otopog_file, start, nwrite, otopog_file);
    nwrite[0] = 1;

    for(n=0; n<nfile_axo; n++) {
      start[0] = n; nwrite[1] = strlen(axo_file[n]);
      mpp_put_var_value_block(fid, id_axo_file, start, nwrite, axo_file[n]);
    }
    for(n=0; n<nfile_axl; n++) {
      start[0] = n; nwrite[1] = strlen(axl_file[n]);
      mpp_put_var_value_block(fid, id_axl_file, start, nwrite, axl_file[n]);
    }
    for(n=0; n<nfile_lxo; n++) {
      start[0] = n; nwrite[1] = strlen(lxo_file[n]);
      mpp_put_var_value_block(fid, id_lxo_file, start, nwrite, lxo_file[n]);
    }    
    mpp_close(fid);
  }
  
  if(mpp_pe()== mpp_root_pe())printf("\n***** Congratulation! You have successfully run make_coupler_mosaic\n");
  mpp_end();

  return 0;
  
} /* main */


