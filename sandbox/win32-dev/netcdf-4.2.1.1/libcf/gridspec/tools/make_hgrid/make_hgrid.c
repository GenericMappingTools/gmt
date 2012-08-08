#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <netcdf.h>
#include "create_hgrid.h"
#include "mpp.h"
#include "mpp_domain.h"
#include "mpp_io.h"

const int MAXBOUNDS = 100;
const int STRINGLEN = 255;
  
char *usage[] = {
  "",
  "                                                                                 ",
  "                                                                                 ",
  "                    Usage of make_hgrid                                          ",
  "                                                                                 ",
  "   make_hgrid --grid_type grid_type --my_grid_file my_grid_file                  ",
  "                  --nxbnds nxbnds --nybnds nybnds                                ",
  "                  --xbnds x(1),...,x(nxbnds) --ybnds y(1),...,y(nybnds)          ",
  "                  --nlon nlon(1),...nlon(nxbnds-1)                               ",
  "                  --nlat nlat(1),...nlat(nybnds-1)                               ",
  "                  --lat_join lat_join --num_lon num_lon --nratio nratio          ",
  "                  --simple_dx simple_dx --simple_dy simple_dy                    ",
  "                  --ndivx ndivx --ndivy ndivy --grid_name gridname               ",
  "                  --center center                                                ",
  "                                                                                 ",
  "   NOTE: This program can generate different types of horizontal grid. The       ",
  "         output data is on supergrid ( model grid size x refinement(=2) ).       ",
  "         For 'cubic_grid', six grid files which contain the grid information     ",
  "         for each tile will be generate, otherwise one file will be generated    ",
  "         you can specify the grid type through --grid_type. The value of         ",
  "         grid_type can be 'from_file', 'spectral_grid', 'spherical_grid',        ",
  "         'conformal_cubic_grid', 'gnomonic_ed' or 'simple_cartesian_grid',       ",
  "         with default value 'spherical_grid'. --nlon and --nlat                  ",
  "         must be specified to indicate supergrid size ( for cubic_grid,          ",
  "         --nlat is not needed because nlat has the same value as nlon.           ",
  "         --ndivx and --ndivy are optional arguments with default value 1.        ",
  "         Besides --nlon, --nlat, --ndivx and --ndivy, other optional and         ",
  "         requirement arguments for each type are,                                ",
  "                                                                                 ",
  "   1. 'from_file':              --my_grid_file must be specified. The grid    ",
  "                                specified in my_grid_file should be super grid   ",
  "                                vertex.                                          ",
  "   2. 'spectral_grid':          no other optional or required arguments.         ",
  "   3. 'regular_lonlat_grid':    --nxbnds, --nybnds --xbnds, --ybnds, must be     ",
  "                                specified to define the grid bounds.             ",
  "   4. 'tripolar_grid':          --nxbnds, --nybnds, --xbnds, --ybnds, must be    ",
  "                                specified to define the grid bounds. --lat_join  ",
  "                                is optional with default value 65.               ",
  "   5  'conformal_cubic_grid':   --nratio is optional argument.                   ",
  "   6  'gnomonic_ed'          :  equal distance gnomonic cubic grid.              ",
  "   6. 'simple_cartesian_grid':  --xbnds, --ybnds must be specified to define     ",
  "                                the grid bounds location and grid size. number   ",
  "                                of bounds must be 2 in both and x and            ",
  "                                y-direction. --simple_dx and --simple_dy must be ",
  "                                specified to specify uniform cell length.        ",
  "                                                                                 ",
  "   make_hgrid take the following flags                                           ",
  "                                                                                 ",
  "   --grid_type grid_type      specify type of topography. See above for          ",
  "                              grid type option.                                  "
  "                                                                                 ",
  "   --my_grid_file file        when this flag is present, the program will read   ",
  "                              grid information from 'my_grid_file'. The file     ",
  "                              format can be ascii file or netcdf file. Multiple  ",
  "                              file entry are allowed but the number should be    ",
  "                              less than MAXBOUNDS.                                ",
  "                                                                                 ",
  "   --nxbnds nxbnds            Specify number of zonal regions for varying        ",
  "                              resolution.                                        ",
  "                                                                                 ",
  "   --nybnds nybnds            Specify number of meridinal regions for varying    ",
  "                              resolution.                                        ",
  "                                                                                 ",
  "   --xbnds x(1),.,x(nxbnds)   Specify boundaries for defining zonal regions of   ",
  "                              varying resolution. When --tripolar is present,    ",
  "                              x also defines the longitude of the two new poles. ",
  "                              nxbnds must be 2 and lon_start = x(1),             ",
  "                              lon_end = x(nxbnds) are longitude of the two       ",
  "                              new poles.                                         ",
  "                                                                                 ",
  "   --ybnds y(1),.,y(nybnds)   Specify boundaries for defining meridional         ",
  "                              regions of varying resolution                      ",
  "                                                                                 ",
  "   --nlon nlon(1),..,nlon(nxbnds-1) Number of model grid points(supergrid) for   ",
  "                                    each zonal regions of varying resolution.    ",
  "                                                                                 ",  
  "   --nlat nlat(1),..,nlat(nybnds-1) Number of model grid points(supergid) for    ",
  "                                    each meridinal regions of varying resolution.",
  "                                                                                 ",
  "   --lat_join lat_join        Specify latitude for joining spherical and rotated ",
  "                              bipolar grid. Default value is 65 degree.          ",
  "                                                                                 ",
  "   --nratio nratio            Speicify the refinement ratio when calculating     ",
  "                              cell length and area of supergrid.                 ",
  "                                                                                 ",
  "   --simple_dx dimple_dx      Specify the uniform cell length in x-direction for ",
  "                              simple cartesian grid.                             ",
  "                                                                                 ",
  "   --simple_dy dimple_dy      Specify the uniform cell length in y-direction for ",
  "                              simple cartesian grid.                             ",  
  "                                                                                 ",
  "   --ndivx ndivx              Specify number division in x-direction for each    ",
  "                              face, default value is 1.                          ",
  "                                                                                 ",
  "   --ndivy ndivy              Specify number division in y-direction for each    ",
  "                              face, default value is 1.                          ",
  "                                                                                 ",
  "   --grid_name grid_name      Specify the grid name. The output grid file name   ",
  "                              will be grid_name.nc if there is one tile and      ",
  "                              grid_name.tile#.nc if there is more than one tile. ",
  "                              The default value will be horizontal_grid.         ",
  "                                                                                 ",
  "   --center center            Specify the center location of grid. The valid     ",
  "                              entry will be 'none', 't_cell' or 'c_cell' with    ",
  "                              default value 'none'. The grid refinement is       ",
  "                              assumed to be 2 in x and y-direction when center   ",
  "                              is not 'none'. 'c_cell' should be used for the grid",
  "                              used in MOM4.                                      ",     
  "                                                                                 ",
  "   Example                                                                       ",      
  "                                                                                 ",
  "                                                                                 ",
  "   1. generating regular lon-lat grid (supergrid size 60x20)                     ",
  "      > make_hgrid --grid_type regular_lonlat_grid --nxbnd 2 --nybnd 2           ",
  "        --xbnd 0,30 --ybnd 50,60  --nlon 60 --nlat 20                            ",
  "                                                                                 ",
  "   2. generating tripolar grid with various grid resolution and C-cell centered  ",
  "      > make_hgrid --grid_type tripolar_grid --nxbnd 2 --nybnd 7 --xbnd -280,80  ",
  "                   --ybnd -82,-30,-10,0,10,30,90 --nlon 720                      ",
  "                   --nlat 104,48,40,40,48,120 --grid_name om3_grid               ",
  "                   --center c_cell  --periodx 360                                ",
  "                                                                                 ",  
  "   3. generating simple cartesian grid(supergrid size 20x20)                     ",
  "      > make_hgrid --grid_type simple_cartesian_grid --xbnd 0,30 --ybnd 50,60    ",
  "                   --nlon 20 --nlat 20  --simple_dx 1000 --simple_dy 1000        ",
  "                                                                                 ",
  "   4. generating conformal cubic grid. (supergrid size 60x60 for each tile)      ",
  "      > make_hgrid --grid_type conformal_cubic_grid --nlon 60 --nratio 2         ",
  "                                                                                 ",
  "   5. generating gnomonic cubic grid with equal_dist_face_edge                   ",
  "      > make_hgrid --grid_type gnomonic_ed --nlon 60                             ",
  "                                                                                 ",
  "   7. generating spectral grid. (supergrid size 128x64)                          ",
  "      > make_hgrid --grid_type spectral_grid --nlon 128 --nlat 64                ",
  "                                                                                 ",
  "   7. Through    user-defined grids                                              ",
  "      > make_hgrid --grid_type from_file --my_grid_file my_grid_file          ",
  "                   --nlon 4 --nlat 4                                             ",
  "                                                                                 ",
  "       contents of sample my_grid_file                                           ",
  "         The first line of my_grid_file will be text ( will be ignored)          ",     
  "         followed by nlon+1 lines of real value of x-direction supergrid bound   ",
  "         location. Then another line of text ( will be ignored), followed by     ",
  "         nlat+1 lines of real value of y-direction supergrid bound location.     ",
  "                                                                                 " ,     
  "         For example:                                                            ",
  "                                                                                 ",
  "            x-grid                                                               ",
  "            0.0                                                                  ",
  "            5.0                                                                  ",
  "            10.0                                                                 ",
  "            15.0                                                                 ",
  "            20.0                                                                 ",
  "            y-grid                                                               ",
  "            -10                                                                  ",
  "            10                                                                   ",
  "            20                                                                   ",
  "            30                                                                   ",
  "            40                                                                   ", 
  "                                                                                 ",
  "                                                                                 ",
  "",
  NULL };

char grid_version[] = "0.2";
char tagname[] = "$Name:  $";

int main(int argc, char* argv[])
{
  int  nratio = 1;
  int  ndivx[] = {1,1,1,1,1,1};
  int  ndivy[] = {1,1,1,1,1,1};
  char method[32] = "conformal";
  char orientation[32] = "center_pole";
  int  nxbnds=2, nybnds=2, nxbnds0=0, nybnds0=0, nxbnds1=0, nybnds1=0, nxbnds2=0, nybnds2=0;
  double xbnds[MAXBOUNDS], ybnds[MAXBOUNDS];
  int nlon[MAXBOUNDS-1], nlat[MAXBOUNDS-1];
  char grid_type[128]="regular_lonlat_grid";
  char my_grid_file[MAXBOUNDS][STRINGLEN];
  double lat_join=65.;
  double simple_dx=0, simple_dy=0;
  int nx, ny, nxp, nyp, ntiles=1, ntilex=0, ntiley=0, ntiles_file;
  double *x=NULL, *y=NULL, *dx=NULL, *dy=NULL, *angle_dx=NULL, *angle_dy=NULL, *area=NULL;
  
  char history[2560];
  char gridname[32] = "horizontal_grid";
  char center[32] = "none";
  char geometry[32] = "spherical";
  char projection[32] = "none";
  char arcx[32] = "small_circle";
  char north_pole_tile[32] = "0.0 90.0";
  char north_pole_arcx[32] = "0.0 90.0";
  char discretization[32]  = "logically_rectangular";
  char conformal[32]       = "true";
  char mesg[256], str[128];
  char entry[MAXBOUNDS*STRINGLEN];
  int isc, iec, jsc, jec, nxc, nyc, layout[2];
  domain2D domain;
  int n, errflg, c, i;  
  int option_index;

  static struct option long_options[] = {
    {"grid_type",       required_argument, NULL, 'a'},
    {"my_grid_file",    required_argument, NULL, 'b'},
    {"nxbnds",          required_argument, NULL, 'c'},
    {"nybnds",          required_argument, NULL, 'd'},
    {"xbnds",           required_argument, NULL, 'e'},
    {"ybnds",           required_argument, NULL, 'f'},
    {"nlon",            required_argument, NULL, 'g'},
    {"nlat",            required_argument, NULL, 'i'},
    {"lat_join",        required_argument, NULL, 'j'},
    {"nratio",          required_argument, NULL, 'k'},
    {"simple_dx",       required_argument, NULL, 'l'},
    {"simple_dy",       required_argument, NULL, 'm'},
    {"ndivx",           required_argument, NULL, 'o'},
    {"ndivy",           required_argument, NULL, 'p'},
    {"grid_name",       required_argument, NULL, 'q'},
    {"center",          required_argument, NULL, 'r'},
    {"help",            no_argument,       NULL, 'h'},    
    {0, 0, 0, 0},
  };

  /* start parallel */
  mpp_init(&argc, &argv);
  mpp_domain_init();  

  /*
   * process command line
   */
  errflg = argc <3;

  while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
    switch (c) {
    case 'a':
      strcpy(grid_type, optarg);
      break;
    case 'b':
      strcpy(entry, optarg);
      tokenize(entry, ",", STRINGLEN, MAXBOUNDS, my_grid_file, &ntiles_file);
      break;
    case 'c':
      nxbnds0 = atoi(optarg);
      break;
    case 'd':
      nybnds0 = atoi(optarg);
      break;        
    case 'e':
      strcpy(entry, optarg);
      nxbnds1 = get_double_entry(entry, xbnds);
      break;
    case 'f':
      strcpy(entry, optarg);
      nybnds1 = get_double_entry(entry, ybnds);
      break;
    case 'g':
      strcpy(entry, optarg);
      nxbnds2 = get_int_entry(entry, nlon);
      break;
    case 'i':
      strcpy(entry, optarg);
      nybnds2 = get_int_entry(entry, nlat);
      break;
    case 'j':
      lat_join = atof(optarg);
      break; 
    case 'k':
      nratio = atoi(optarg);
      break;      
    case 'l':
      simple_dx = atof(optarg);
      break;
    case 'm':
      simple_dy = atof(optarg);
      break;
    case 'o':
      strcpy(entry, optarg);
      ntilex = get_int_entry(entry, ndivx);
      break;
    case 'p':
      strcpy(entry, optarg);
      ntiley = get_int_entry(entry, ndivy);
      break;      
    case 'q':
      strcpy(gridname, optarg);
      break;
    case 'r':
      strcpy(center, optarg);
      break;
    case 'h':
      errflg++;
      break;
    case '?':
      errflg++;      
    }      
  }
  
  if (errflg ) {
    char **u = usage;
    while (*u) { fprintf(stderr, "%s\n", *u); u++; }
    exit(2);
  }  

  /* define history to be the history in the grid file */
  strcpy(history,argv[0]);

  for(i=1;i<argc;i++) {
    strcat(history, " ");
    strcat(history, argv[i]);
  }

  if(mpp_pe() == mpp_root_pe() ) printf("==>NOTE: the grid type is %s\n",grid_type);


  /* check the command-line arguments to make sure the value are suitable */

  if( strcmp(grid_type,"regular_lonlat_grid") ==0 ) {
    nxbnds = nxbnds0; nybnds = nybnds0;
    if( nxbnds <2 || nybnds < 2) mpp_error("make_hgrid: grid type is 'regular_lonlat_grid', "
					   "both nxbnds and nybnds should be no less than 2");
    if( nxbnds != nxbnds1 || nxbnds != nxbnds2+1 )
      mpp_error("make_hgrid: grid type is 'regular_lonlat_grid', nxbnds does"
		"not match number of entry in xbnds or nlon");
    if( nybnds != nybnds1 || nybnds != nybnds2+1 )
      mpp_error("make_hgrid: grid type is 'regular_lonlat_grid', nybnds does "
		"not match number of entry in ybnds or nlat");
  }
  else if( strcmp(grid_type,"tripolar_grid") ==0 ) {
    strcpy(projection, "tripolar");
    nxbnds = nxbnds0; nybnds = nybnds0;
    if( nxbnds != 2) mpp_error("make_hgrid: grid type is 'tripolar_grid', nxbnds should be 2");
    if( nybnds < 2) mpp_error("make_hgrid: grid type is 'tripolar_grid', nybnds should be no less than 2");
    if( nxbnds != nxbnds1 || nxbnds != nxbnds2+1 )
      mpp_error("make_hgrid: grid type is 'tripolar_grid', nxbnds does not match number of entry in xbnds or nlon");
    if( nybnds != nybnds1 || nybnds != nybnds2+1 )
      mpp_error("make_hgrid: grid type is 'tripolar_grid', nybnds does not match number of entry in ybnds or nlat");
  }
  else if( strcmp(grid_type,"from_file") ==0 ) {
    /* For ascii file, nlon and nlat should be specified through --nlon, --nlat
       For netcdf file, grid resolution will be read from grid file
    */
    
    if(ntiles_file == 0) mpp_error("make_hgrid: grid_type is 'from_file', but my_grid_file is not specified");
    ntiles = ntiles_file;
    for(n=0; n<ntiles; n++) {
      if(strstr(my_grid_file[n],".nc") ) {
	/* get the grid size for each tile, the grid is on model grid, should need to multiply by 2 */
	int fid;
	fid = mpp_open(my_grid_file[n], MPP_READ);
	nlon[n] = mpp_get_dimlen(fid, "grid_xt")*2;
	nlat[n] = mpp_get_dimlen(fid, "grid_yt")*2;
	mpp_close(fid);
      }
      else {
	if(nxbnds2 != ntiles || nybnds2 != ntiles ) mpp_error("make_hgrid: grid type is 'from_file', number entry entered "
						"through --nlon and --nlat should be equal to number of files "
							  "specified through --my_grid_file");
      }
    }
    	/* for simplify purpose, currently we assume all the tile have the same grid size */
    for(n=1; n<ntiles; n++) {
      if( nlon[n] != nlon[0] || nlat[n] != nlat[0])  mpp_error("make_hgrid: grid_type is from_file, all the tiles should "
							       "have same grid size, contact developer");
    }
  }
  else if( strcmp(grid_type,"simple_cartesian_grid") ==0) {
    strcpy(geometry, "planar");
    strcpy(north_pole_tile, "none");
    if(nxbnds1 != 2 || nybnds1 != 2 ) mpp_error("make_hgrid: grid type is 'simple_cartesian_grid', number entry entered "
						"through --xbnds and --ybnds should be 2");
    if(nxbnds2 != 1 || nybnds2 != 1 ) mpp_error("make_hgrid: grid type is 'simple_cartesian_grid', number entry entered "
						"through --nlon and --nlat should be 1");
    if(simple_dx == 0 || simple_dy == 0) mpp_error("make_hgrid: grid_type is 'simple_cartesian_grid', "
						   "both simple_dx and simple_dy both should be specified");
  }
  else if ( strcmp(grid_type,"spectral_grid") ==0 ) {
    if(nxbnds2 != 1 || nybnds2 != 1 ) mpp_error("make_hgrid: grid type is 'spectral_grid', number entry entered "
						"through --nlon and --nlat should be 1");    
  }
  else if( strcmp(grid_type,"conformal_cubic_grid") ==0 ) {
    strcpy(projection, "cube_gnomonic");
    strcpy(conformal, "FALSE");
    if(nxbnds2 != 1 ) mpp_error("make_hgrid: grid type is 'conformal_cubic_grid', number entry entered "
				"through --nlon should be 1");
    if(nratio < 1) mpp_error("make_hgrid: grid type is 'conformal_cubic_grid', nratio should be a positive integer");
  }
  else if(  !strcmp(grid_type,"gnomonic_ed") ) {
    strcpy(projection, "cube_gnomonic");
    strcpy(conformal, "FALSE");
    if(nxbnds2 != 1 ) mpp_error("make_hgrid: grid type is 'gnomonic_cubic_grid', number entry entered "
				"through --nlon should be 1");
  }
  else {
    mpp_error("make_hgrid: only grid_type = 'regular_lonlat_grid', 'tripolar_grid', 'from_file', "
	      "'gnomonic_ed', 'conformal_cubic_grid', 'simple_cartesian_grid' and "
	      "'spectral_grid' is implemented");  
  }
  
  /* get super grid size */

  if( !strcmp(grid_type,"gnomonic_ed") || !strcmp(grid_type,"conformal_cubic_grid") ) {
    nx = nlon[0];
    ny = nx;
  }
  else {
    nx = 0;
    ny = 0;
    for(n=0; n<nxbnds-1; n++) nx += nlon[n];
    for(n=0; n<nybnds-1; n++) ny += nlat[n];  
  }
  nxp = nx + 1;
  nyp = ny + 1;

  if( !strcmp(grid_type,"gnomonic_ed") || !strcmp(grid_type,"conformal_cubic_grid") ) {
    ntiles = 6;
    /* Cubic grid is required to run on single processor.*/
    if(mpp_npes() > 1) mpp_error( "make_hgrid: cubic grid generation must be run one processor, contact developer");
  }
  /* Currently we restrict nx can be divided by ndivx and ny can be divided by ndivy */
  if(ntilex >0 && ntilex != ntiles) mpp_error("make_hgrid: number of entry specified through --ndivx does not equal ntiles");
  if(ntiley >0 && ntiley != ntiles) mpp_error("make_hgrid: number of entry specified through --ndivy does not equal ntiles");   
  for(n=0; n<ntiles; n++) {
    if( nx%ndivx[n] ) mpp_error("make_hgrid: nx can not be divided by ndivx");
    if( ny%ndivy[n] ) mpp_error("make_hgrid: ny can not be divided by ndivy");
  }

  if(strcmp(center,"none") && strcmp(center,"c_cell") && strcmp(center,"t_cell") )
    mpp_error("make_hgrid: center should be 'none', 'c_cell' or 't_cell' ");
  
  /* set up domain decomposition, x and y will be on global domain and
     other fields will be on compute domain. 
  */

  mpp_define_layout( nx, ny, mpp_npes(), layout);
  mpp_define_domain2d( nx, ny, layout, 0, 0, &domain);
  mpp_get_compute_domain2d(domain, &isc, &iec, &jsc, &jec);
  nxc = iec - isc + 1;
  nyc = jec - jsc + 1;

  /* create grid information */
  x        = (double *) malloc(nxp*nyp*ntiles*sizeof(double));
  y        = (double *) malloc(nxp*nyp*ntiles*sizeof(double));
  dx       = (double *) malloc(nxc*(nyc+1)*ntiles*sizeof(double));
  dy       = (double *) malloc((nxc+1)*nyc*ntiles*sizeof(double));
  area     = (double *) malloc(nxc    *nyc*ntiles*sizeof(double));
  angle_dx = (double *) malloc((nxc+1)*(nyc+1)*ntiles*sizeof(double));
  if( strcmp(conformal,"true") !=0 )angle_dy = (double *) malloc(nxp*nyp*ntiles*sizeof(double));
  
  if(strcmp(grid_type,"regular_lonlat_grid") ==0) 
    create_regular_lonlat_grid(&nxbnds, &nybnds, xbnds, ybnds, nlon, nlat, &isc, &iec, &jsc, &jec,
			       x, y, dx, dy, area, angle_dx, center);
  else if(strcmp(grid_type,"tripolar_grid") ==0) 
    create_tripolar_grid(&nxbnds, &nybnds, xbnds, ybnds, nlon, nlat, &lat_join, &isc, &iec, &jsc, &jec,
			 x, y, dx, dy, area, angle_dx, center);
  else if( strcmp(grid_type,"from_file") ==0 ) {
    for(n=0; n<ntiles; n++) {
      int n1, n2, n3, n4;
      n1 = n * nxp * nyp;
      n2 = n * nx  * nyp;
      n3 = n * nxp * ny;
      n4 = n * nx  * ny;
      create_grid_from_file(my_grid_file[n], &nx, &ny, x+n1, y+n1, dx+n2, dy+n3, area+n4, angle_dx+n1);
    }
  }
  else if(strcmp(grid_type,"simple_cartesian_grid") ==0) 
    create_simple_cartesian_grid(xbnds, ybnds, &nx, &ny, &simple_dx, &simple_dy, &isc, &iec, &jsc, &jec,
				 x, y, dx, dy, area, angle_dx );
  else if(strcmp(grid_type,"spectral_grid") ==0 )
    create_spectral_grid(&nx, &ny, &isc, &iec, &jsc, &jec, x, y, dx, dy, area, angle_dx );
  else if(strcmp(grid_type,"conformal_cubic_grid") ==0 ) 
    create_conformal_cubic_grid(&nx, &nratio, method, orientation, x, y, dx, dy, area, angle_dx, angle_dy );
  else if(strcmp(grid_type,"gnomonic_ed") ==0 ) 
    create_gnomonic_cubic_grid(grid_type, &nx, x, y, dx, dy, area, angle_dx, angle_dy );
  
  /* write out data */
  {
    int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
    int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp, m;
    size_t start[4], nwrite[4];
    double *tmp, *gdata;
    char tilename[128] = "";
    char outfile[128] = "";
    
    l = 0;
    for(n=0 ; n< ntiles; n++) {
      for(j=0; j<ndivy[n]; j++) {
	for(i=0; i<ndivx[n]; i++) {
	  ++l;
	  sprintf(tilename, "tile%d", l);
	  if(ntiles>1)
	    sprintf(outfile, "%s.tile%d.nc", gridname, l);
	  else
	    sprintf(outfile, "%s.nc", gridname);
	  fid = mpp_open(outfile, MPP_WRITE);
	  /* define dimenison */
	  ni = nx/ndivx[n];
	  nj = ny/ndivy[n];
	  nip = ni + 1;
	  njp = nj + 1;
	  dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
	  dimlist[1] = mpp_def_dim(fid, "nx", ni);
	  dimlist[2] = mpp_def_dim(fid, "ny", nj);
	  dimlist[3] = mpp_def_dim(fid, "nxp", nip);
	  dimlist[4] = mpp_def_dim(fid, "nyp", njp);
	  /* define variable */
	  if( strcmp(north_pole_tile, "none") == 0) /* no north pole, then no projection */
	    id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 4, "standard_name", "grid_tile_spec",
				  "geometry", geometry, "discretization", discretization, "conformal", conformal );
	  else if( strcmp(projection, "none") == 0) 
	    id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec",
				  "geometry", geometry, "north_pole", north_pole_tile, "discretization",
				  discretization, "conformal", conformal );
	  else
	    id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 6, "standard_name", "grid_tile_spec",
				  "geometry", geometry, "north_pole", north_pole_tile, "projection", projection,
				  "discretization", discretization, "conformal", conformal );
	  dims[0] = dimlist[4]; dims[1] = dimlist[3];
	  id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude",
			     "units", "degree_east");
	  id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude",
			     "units", "degree_north");
	  dims[0] = dimlist[4]; dims[1] = dimlist[1];
	  id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance",
			      "units", "meters");
	  dims[0] = dimlist[2]; dims[1] = dimlist[3];
	  id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance",
			      "units", "meters");
	  dims[0] = dimlist[2]; dims[1] = dimlist[1];
	  id_area = mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area",
				"units", "m2" );
	  dims[0] = dimlist[4]; dims[1] = dimlist[3];
	  id_angle_dx = mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name",
				    "grid_vertex_x_angle_WRT_geographic_east", "units", "degrees_east");
	  if(strcmp(conformal, "true") != 0)
	    id_angle_dy = mpp_def_var(fid, "angle_dy", MPP_DOUBLE, 2, dims, 2, "standard_name",
				      "grid_vertex_y_angle_WRT_geographic_north", "units", "degrees_north");
	  if( strcmp(north_pole_arcx, "none") == 0)
	    id_arcx = mpp_def_var(fid, "arcx", MPP_CHAR, 1, dimlist, 1, "standard_name", "grid_edge_x_arc_type" );
	  else
	    id_arcx = mpp_def_var(fid, "arcx", MPP_CHAR, 1, dimlist, 2, "standard_name", "grid_edge_x_arc_type",
				  "north_pole", north_pole_arcx );
	  mpp_def_global_att(fid, "grid_version", grid_version);
	  mpp_def_global_att(fid, "code_version", tagname);
	  mpp_def_global_att(fid, "history", history);
      
	  mpp_end_def(fid);
	  for(m=0; m<4; m++) { start[m] = 0; nwrite[m] = 0; }
	  nwrite[0] = strlen(tilename);
	  mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );

          tmp = get_subregion(nxp, x+n*nxp*nyp, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	  mpp_put_var_value(fid, id_x, tmp);
	  free(tmp);
          tmp = get_subregion(nxp, y+n*nxp*nyp, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	  mpp_put_var_value(fid, id_y, tmp);
	  free(tmp);
	  gdata = (double *)malloc(nx*nyp*sizeof(double));
	  mpp_global_field_double(domain, nxc, nyc+1, dx+n*nx*nyp, gdata);
	  tmp = get_subregion( nx, gdata, i*ni, (i+1)*ni-1, j*nj, (j+1)*nj);
	  mpp_put_var_value(fid, id_dx, tmp);
	  free(tmp);
	  free(gdata);
	  gdata = (double *)malloc(nxp*ny*sizeof(double));
	  mpp_global_field_double(domain, nxc+1, nyc, dy+n*nxp*ny, gdata);
	  tmp = get_subregion( nxp, gdata, i*ni, (i+1)*ni, j*nj, (j+1)*nj-1);
	  mpp_put_var_value(fid, id_dy, tmp);
	  free(tmp);
	  free(gdata);	  
	  gdata = (double *)malloc(nx*ny*sizeof(double));
	  mpp_global_field_double(domain, nxc, nyc, area+n*nx*ny, gdata);
	  tmp = get_subregion( nx, gdata, i*ni, (i+1)*ni-1, j*nj, (j+1)*nj-1);
	  mpp_put_var_value(fid, id_area, tmp);
	  free(tmp);
	  free(gdata);
	  gdata = (double *)malloc(nxp*nyp*sizeof(double));
	  mpp_global_field_double(domain, nxc+1, nyc+1, angle_dx+n*nxp*nyp, gdata);
	  tmp = get_subregion( nxp, gdata, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	  mpp_put_var_value(fid, id_angle_dx, tmp);
	  free(tmp);
	  free(gdata);
	  
	  if(strcmp(conformal, "true") != 0) {
	    gdata = (double *)malloc(nxp*nyp*sizeof(double));
	    mpp_global_field_double(domain, nxc+1, nyc+1, angle_dy+n*nxp*nyp, gdata);
	    tmp = get_subregion( nxp, gdata, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	    mpp_put_var_value(fid, id_angle_dy, tmp);
	    free(tmp);
	    free(gdata);
	  }
	  nwrite[0] = strlen(arcx);
	  mpp_put_var_value_block(fid, id_arcx, start, nwrite, arcx );
	  mpp_close(fid);
	}
      }
    }
  }

  free(x);
  free(y);
  free(dx);
  free(dy);
  free(area);
  free(angle_dx);
  if(strcmp(conformal, "true") != 0) free(angle_dy);
  if(mpp_pe() == mpp_root_pe()) printf("generate_grid is run successfully. \n");

  mpp_end();

  return 0;
  
};  /* end of main */


