#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <netcdf.h>
#include "create_hgrid.h"
#include "mpp.h"
#include "mpp_domain.h"
#include "mpp_io.h"

#define MAXBOUNDS 100
#define STRINGLEN 255
  
/* char grid_version[] = "0.2"; */
/* char tagname[] = "$Name:  $"; */

char *usage[] = {
   "",
   "                                                                                 ",
   "                                                                                 ",
   "                    Usage of gs_make_hgrid                                       ",
   "                                                                                 ",
   "   gs_make_hgrid --grid_type grid_type --my_grid_file my_grid_file               ",
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
   int errflg, c, i;  
   int option_index;
   int ret;

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

   /* process command line */
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

   /* Do everything. */
   if ((ret = gs_make_hgrid(grid_type, nlat, nlon, nxbnds0, nybnds0, nxbnds1, 
                            nybnds1, nxbnds2, nybnds2, lat_join, nratio, simple_dx, 
                            simple_dy, ntilex, ntiley, gridname, center, history, xbnds, ybnds)))
      return ret;

   if(mpp_pe() == mpp_root_pe()) printf("generate_grid is run successfully. \n");

   mpp_end();

   return 0;
  
};  /* end of main */


