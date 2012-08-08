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
   int ret;
    
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
  
   if ((ret = gs_make_coupler_mosaic(history, amosaic, lmosaic, omosaic, otopog, 
                                     interp_order, sea_level, mosaic_name, check)))
      return ret;

   if(mpp_pe() == mpp_root_pe())
      printf("\n***** Congratulation! You have successfully run make_coupler_mosaic\n");
   mpp_end();

   return 0;
  
} /* main */


