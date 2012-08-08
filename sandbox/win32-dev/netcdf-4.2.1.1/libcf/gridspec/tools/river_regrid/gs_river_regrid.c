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
   int ret;

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

   strcpy(history,argv[0]);
   for(n=1;n<argc;n++)  {
      strcat(history, " ");
      strcat(history, argv[n]); 
   }

   if ((ret = gs_river_regrid(history, mosaic_file, 
                              river_src_file, output_file)))
      return ret;

   mpp_end();

   return 0;  

} /* end of main */

