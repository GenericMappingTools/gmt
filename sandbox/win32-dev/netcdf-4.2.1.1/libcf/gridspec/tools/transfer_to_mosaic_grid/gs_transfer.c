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
   char *old_file = NULL;

   int is_coupled_grid = 0, is_ocean_only =1; 
   int interp_order=1;
   int ret;
  
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

   if ((ret = gs_transfer_to_mosaic(old_file, mosaic_dir)))
      return ret;

   mpp_end();
  
};  /* end of main */

