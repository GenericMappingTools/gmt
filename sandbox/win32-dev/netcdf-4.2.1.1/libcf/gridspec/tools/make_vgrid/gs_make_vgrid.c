#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "create_vgrid.h"
#include "mpp_io.h"

#define MAXBOUNDS 100

char *usage[] = {
  "",
  "                                                                                 ",
  "                                                                                 ",
  "                    Usage of make_vgrid                                          ",
  "                                                                                 ",
  "   make_vgrid --nbnds nbnds --bnds z(1),...,z(nbnds) --nz nz                     ",
  "              [--grid_name gridname]    [--center center]                        ",
  "                                                                                 ",
  "   NOTE: This program call be used to make vertical grid for FMS model.          ",
  "         It uses cubic-spline algorithm to calculate the grid cell location.     ",
  "         The output netcdf will contains information on supergrid with grid      ",
  "         size equal model grid size multipling refinement ( always 2 ).          ",
  "         make_vgrid takes the following flags                                    ",
  "                                                                                 ",
  "   Required Flags:                                                               ",
  "                                                                                 ",
  "   --nbnds nbnds             Specify number of vertical regions for varying      ",
  "                             resolution.                                         ",
  "                                                                                 ",  
  "   --bnds z(1),.,z(nbnds)    Specify boundaries for defining vertical regions of ",
  "                             varying resolution.                                 ",
  "                                                                                 ",
  "   --nz nz(1),..,nz(nbnds-1) Number of model grid points for each vertical       ",
  "                             regions of varying resolution.                       "
  "                                                                                 ",  
  "   Optional Flags:                                                               ",
  "                                                                                 ",
  " --grid_name gridname        Specify the grid name. The output grid file name    ",
  "                             will be grid_name.nc. The default value is          ",
  "                             vertical_grid.                                      ",
  "                                                                                 ",  
  " --center   center           Specify the center location of grid. The valid      ",
  "                             entry will be 'none', 't_cell' or 'c_cell' with     ",
  "                             default value 'none'. The grid refinement is        ",
  "                             assumed to be 2 in x and y-direction when center    ",
  "                             is not 'none'. 'c_cell' should be used for the grid ",
  "                             used in MOM4.                                       ",
  "                                                                                 ",
  "                                                                                 ",
  "   Example                                                                       ",      
  "                                                                                 ",
  "                                                                                 ",
  " make_vgrid --nbnds 3 --bnds 10,200,1000 --nz 10,20                             ",
  "       will make a grid file with 60 supergrid cells, 30 model grid cells        ",
  "       with refinement is 2.                                                     ",
  "",
  NULL };  

char grid_version[] = "0.2";
char tagname[] = "$Name:  $";

int main(int argc, char* argv[])
{
  int nbnds, n1, n2, i, nk;
  double bnds[MAXBOUNDS];
  int    nz[MAXBOUNDS-1];
  char gridname[128]= "vertical_grid";
  char filename[128];
  char center[32] = "none";
  char entry[512];
  char history[256];
  int errflg, c, option_index = 0;
  int ret;

  static struct option long_options[]= {  
    {"nbnds",    required_argument, NULL, 'n'},
    {"bnds",     required_argument, NULL, 'b'},
    {"nz",       required_argument, NULL, 'z'},
    {"grid_name",required_argument, NULL, 'o'},        
    {"center",   required_argument, NULL, 'c'},    
    {0, 0, 0, 0}
  };
  
  /* process command line */
  errflg = argc <4;
  nbnds = 0;
  n1 = 0;
  n2 = 0;
  while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1)
    switch (c) {
    case 'n':
      nbnds = atoi(optarg);
      break;
    case 'b':
      strcpy(entry, optarg);
      n1 = get_double_entry(entry, bnds);
      break;
    case 'z':
      strcpy(entry, optarg);
      n2 = get_int_entry(entry, nz);
      break;
    case 'o':
      strcpy(gridname, optarg);
      break;
    case 'c':
      strcpy(center, optarg);
      break;
    case '?':
      errflg++;      
    }

  if (errflg ) {
    char **u = usage;
    while (*u) { fprintf(stderr, "%s\n", *u); u++; }
    mpp_error("Wrong usage of this program, check arguments") ;
  }    

  /* define history to be the history in the grid file */
  strcpy(history,argv[0]);

  for(i=1;i<argc;i++) {
    strcat(history, " ");
    strcat(history, argv[i]);
  }

  if ((ret = gs_make_vgrid(history, nbnds, bnds, n1, n2, 
			   nz, gridname, center)))
     return ret;

  mpp_end();

  return 0;
}    
