#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "get_contact.h"
#include "constant.h"
#include "mpp_io.h"

char *usage[] = {
   "",
   " make_solo_mosaic --num_tiles ntiles --dir directory [--mosaic_name mosaic_name] ",
   "                 [--tile_file tile_file] [--periodx #] [--periody #]             ",
   "                                                                                 ",
   "make_solo_mosaic generates Mosaic information between tiles. The mosaic          ",
   "information includes: list of tile files, list of contact region                 ",
   "specified by index, contact type.                                                ",
   "                                                                                 ",
   "make_solo_mosaic takes the following flags:                                      ",
   "                                                                                 ",
   "REQUIRED:                                                                        ",
   "                                                                                 ",
   "--num_tiles ntiles     Number of tiles in the mosaic.                            ",
   "                                                                                 ",
   "--dir directory        The directory that contains all the tile grid file.       ",
   "                                                                                 ",
   "OPTIONAL FLAGS                                                                   ",
   "                                                                                 ",
   "--mosaic_name name     mosaic name. The output file will be mosaic_name.nc.      ",
   "                       default is 'mosaic'.                                      ",
   "                                                                                 ",
   "--tile_file tile_file  Grid file name of all tiles in the mosaic. The file name  ",
   "                       should be relative file name ( exclude the absolute       "
   "                       file path). The absolute file path will be dir/tile_file. ",
   "                       If this option is not specified, the tile_file will be    ",
   "                       'horizontal_grid.tile#.nc'                                ",
   "                                                                                 ",
   "--periodx #             Specify the period in x-direction of mosaic. Default     ",
   "                        value is 0 (not periodic).                               ",
   "                                                                                 ",
   "--periody #             Specify the period in y-direction of mosaic. Default     ",
   "                        value is 0 (not periodic).                               ",  
   NULL};
 
const int MAXTILE = 100;
const int MAXCONTACT = 100;
const int SHORTSTRING = 32;
char grid_version[] = "0.2";
char tagname[] = "$Name:  $";

main (int argc, char *argv[])
{
  
   extern char *optarg;
   char *pch=NULL, *dir=NULL, history[512], entry[1280];
   char tilefile[MAXTILE][STRING], tiletype[MAXTILE][SHORTSTRING];
   char tile_name[MAXTILE][STRING];
   int ntiles=0, nfiles=0, ncontact=0;
   int *nxp, *nyp;
   double **x, **y;
   double periodx=0, periody=0;
   int contact_tile1[MAXCONTACT], contact_tile2[MAXCONTACT];
   int contact_tile1_istart[MAXCONTACT], contact_tile1_iend[MAXCONTACT];
   int contact_tile1_jstart[MAXCONTACT], contact_tile1_jend[MAXCONTACT];
   int contact_tile2_istart[MAXCONTACT], contact_tile2_iend[MAXCONTACT];
   int contact_tile2_jstart[MAXCONTACT], contact_tile2_jend[MAXCONTACT];
   char mosaic_name[128] = "solo_mosaic";
   char grid_descriptor[128] = "";
   int c, i, n, m, l, errflg;
   int ret;
  
   int option_index = 0;
   static struct option long_options[] = {
      {"mosaic_name",     required_argument, NULL, 'm'},
      {"num_tiles",       required_argument, NULL, 'n'},
      {"grid_descriptor", required_argument, NULL, 'g'},
      {"tile_file",       required_argument, NULL, 'f'},  
      {"periodx",         required_argument, NULL, 'x'},
      {"periody",         required_argument, NULL, 'y'},
      {"directory",       required_argument, NULL, 'd'},
      {NULL, 0, NULL, 0}
   };

   mpp_init(&argc, &argv);
   /* this tool must be run one processor */
   if(mpp_npes()>1) mpp_error("make_solo_mosaic: this tool must be run on one processor");
  
   errflg = (argc == 1);
   /* First read command line arguments. */

   while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
      switch (c) {
	 case 'n':
	    ntiles = atoi(optarg);
	    break;
	 case 'm':
	    strcpy(mosaic_name, optarg);
	    break;
	 case 'g':
	    strcpy(grid_descriptor, optarg);
	    break;
	 case 'f':
	    strcpy(entry, optarg); 
	    pch = strtok(entry, ", ");
	    nfiles = 0;
	    while( pch != NULL) {
	       strcpy(tilefile[nfiles++], pch);
	       pch = strtok(NULL, ", ");
	    }
	    break;
	 case 'x':
	    periodx = atof(optarg);
	    break;
	 case 'y':
	    periody = atof(optarg);
	    break; 
	 case 'd':  // path of the simple grid file.
	    dir = optarg;
	    break;
	 case '?':
	    errflg++;
      }
   }

   if (errflg || ntiles < 1 || !dir ) {
      char **u = usage;
      while (*u) { fprintf(stderr, "%s\n", *u); u++; }
      exit(2);      
   }

   strcpy(history,argv[0]);

   for(i=1;i<argc;i++) {
      strcat(history, " ");
      strcat(history, argv[i]);
   }

   if(ntiles > MAXTILE) {
      mpp_error("make_solo_mosaic: number of tiles is greater than MAXTILE.");
   }

   if ((ret = gs_make_solo_mosaic(history, ntiles, mosaic_name, grid_descriptor,
                                  tilefile, periodx, periody, dir)))
      return ret;
  
   printf("congradulation: You have successfully run make_solo_mosaic\n");

   return 0;
  
}; // end of main



