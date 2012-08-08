#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "topog.h"
#include "mpp.h"
#include "mpp_domain.h"
#include "mpp_io.h"
#include "tool_util.h"

char *usage[] = {
  "",
  "                                                                                         ",
  "                                                                                         ",
  "                    Usage of make_topog                                                  ",
  "                                                                                         ",
  "   make_topog --mosaic mosaic_file [--topog_type topog_type]  [x_refine #] [y_refine #]  ",
  "              [--basin_depth #] [--bottom_depth #] [--min_depth #]                       ",
  "              [--topog_file topog_file]  [--topog_field topog_field]                     ",
  "              [--scale_factor #] [--num_filter_pass #] [--gauss_amp #]                   ",
  "              [--gauss_scale #] [--slope_x #] [--slope_y #] [--bowl_south #]             ",
  "              [--bowl_north #] [--bowl_west #] [--bowl_east #]                           ",
  "              [--fill_first_row] [--filter_topog] [--round_shallow]                      ",
  "              [--fill_shallow] [--deepen_shallow] [--smooth_topo_allow_deepening]        ",
  "              [--output output_file [--help]                                             ",
  "                                                                                         ",
  "   make_topog can generate topography for any Mosaic. The output file                    ",
  "   will contains the topography for each tile in the Mosaic. The field name in           ",
  "   the output topography file will be depth_tile# and it is positive down.               ",
  "   The topography data will be defined on model grid, the model grid size will be        ",
  "   supergrid grid size divided by refinement (x_refine, y_refine, default is 2).         ",
  "   --mosaic is a required option and all other options are optional, but                 ",
  "   some options are required depending on the choice of topog_type.                      ",
  "   Below specify the option (required or non-required) needed for every kind             ",
  "   of topog_type. when topog_type (--topog_type) is                                      ",
  "                                                                                         ",
  "     1. 'realistic':          --topog_file and --topog_field must be specified,          ",
  "                              --bottom_depth, --min_depth, --scale_factor,               ",
  "                              --num_filter_pass, --flat_bottom                           ",
  "                              --fill_first_row --adjust_topo --filter_topog              ",
  "                              --round_shallow] --fill_shallow --deepen_shallow]          ",
  "                              --smooth_topo_allow_deepening are optional arguments.      ",
  "     2.  'rectangular_basin': --basin_depth are optional arguments. Set basin_depth      ",
  "                                to 0 to get all land topography.                         ",
  "     3.  'gaussian':          --bottom_depth, --min_depth --gauss_amp, --gauss_scale,    ",
  "                              --slope_x, --slope_y are optional arguments.               ",
  "     4.. 'bowl':              --bottom_depth, --min_depth, --bowl_south, --bowl_north,   ",
  "                              --bowl_west, --bowl_east are optional arguments.           ",
  "     5. 'idealized':          --bottom_depth, --min_depth are optional arguments.        ",
  "                                                                                         ",      
  "   generate_mosaic_topo take the following flags                                         ",
  "                                                                                         ",
  "   --mosaic mosaic_file          Specify the mosaic file where topography data located.  ",
  "                                                                                         ",
  "   --topog_type topog_type       Specify type of topography. Its value can be            ",
  "                                 'realistic', 'rectangular_basin', 'gaussian', 'bowl'    ",
  "                                 or 'idealized'. The default value is 'realistic'.       ",
  "                                                                                         ",
  "   --x_refine #                  specify the refinement ratio of model grid vs supergrid ",
  "                                 ins x-directin. default value 2.                        ",
  "                                                                                         ",
  "   --y_refine #                  specify the refinement ratio of model grid vs supergrid ",
  "                                 ins y-directin. default value 2.                        ",
  "                                                                                         ",
  "   --basin_depth #               Specify the basin depth when topog_type is              ",
  "                                 'rectangular_basin'. Default value is 5000 meter.       ",
  "                                                                                         ",
  "   --topog_file topog_file       Specify name of topograhy file (e.g. scripps,           ",
  "                                 navy_topo, ...)                                         ",
  "                                                                                         ",
  "   --topog_field topog_field     Specify name of topography field name in topog_file.    ",
  "                                                                                         ",
  "   --bottom_depth #              Specify maximum depth (or bottom depth) of ocean.       ",
  "                                 default value is 5000 meter.                            ",
  "                                                                                         ",
  "   --min_depth #                 Specify minimum depth of ocean.                         ",
  "                                 default value is 10 meter.                              ",
  "                                                                                         ",  
  "   --scale_factor #              Specify scaling factor for topography data (e.g. -1 to  ",
  "                                 flip sign or 0.01 to convert from centimeters).         ",
  "                                 default value is 1.                                     ",
  "                                                                                         ",
  "   --num_filter_pass #           Specify number of passes of spatial filter              ",
  "                                 default value is 1.                                     ",  
  "                                                                                         ",
  "   --gauss_amp #                 specify height of gaussian bump as percentage of ocean  ",
  "                                 depth. default value is 0.5.                            ",
  "                                                                                         ",
  "   --gauss_scale #               Specify width of gaussian bump as percentag e of        ",
  "                                 basin width. Default value is 0.25.                     ",
  "                                                                                         ",
  "   --slope_x #                   Specify rise of the ocean floor to the east for         ",
  "                                 the gaussian bump. Default value is 0.                  ",
  "                                                                                         ",
  "   --slope_y #                   Specify rise of the ocean floor to the north for        ",
  "                                 the gaussian bump. Default value is 0.                  ",
  "                                                                                         ",
  "   --bowl_south #                Specify southern boundary of Winton bowl.               ",
  "                                 Default value is 60.                                    ",
  "                                                                                         ",
  "   --bowl_north #                Specify northern boundary of Winton bowl.               ",
  "                                 Default value is 70.                                    ",  
  "                                                                                         ",
  "   --bowl_west #                 Specify western boundary of Winton bowl.                ",
  "                                 Default value is 0.                                     ",
  "                                                                                         ",  
  "   --bowl_east #                 Specify eastern boundary of Winton bowl.                ",
  "                                 Default value is 20.                                    ",
  "                                                                                         ",
  "   --fill_first_row              when specified, make first row of ocean model all       ",
  "                                 land points for ice model.                              ",
  "                                                                                         ",
  "   --filter_topog                When specified, apply filter to topography.             ",
  "                                                                                         ",
  "   --round_shallow               When specified, Make cells land if depth is less        ",
  "                                 than 1/2 mimumim depth, otherwise make ocean.           ",
  "                                                                                         ",
  "   --fill_shallow                When specified, Make cells less than minimum            ",
  "                                 depth land.                                             ",
  "                                                                                         ",
  "   --deepen_shallow              When specified, Make cells less than minimum            ",
  "                                 depth equal to minimum depth.                           ",
  "                                                                                         ",
  "   --smooth_topo_allow_deepening when specified, allow filter to deepen cells.           ",
  "                                                                                         ",
  "   --output output_file          The created netcdf file that contains mosaic            ",
  "                                 topography. Default value is 'topog.nc'                 ",
  "   --help                        Print out this message and then exit.                   ",
  "                                                                                         ",   
  "   Example                                                                               ", 
  "                                                                                         ",
  "   1. Generate 'realistic' topography                                                    ",
  "   > make_topog --mosaic mosaic.nc --topog_type realistic                                ",
  "                           --topog_file /archive/fms/mom4/input_data/OCCAM_p5degree.nc   ",
  "                              --topog_field TOPO --scale_factor -1                       ",
  "                                                                                         ",
  "   2. Generate 'rectangular_basin' topography (with uniform topography 200 meters).      ",
  "   > make_topog --mosaic mosaic.nc --topog_type  rectangular_basin            ",
  "                           --basin_depth 200                                             ",
  "                                                                                         ",
  "                                                                                         "
  "",
  NULL };

char grid_version[] = "0.2";
char tagname[] = "$Name:  $";

int main(int argc, char* argv[])
{
  char   *mosaic_file = NULL, *topog_file = NULL, *topog_field = NULL;
  char   topog_type[32] = "realistic", output_file[32] = "topog.nc";
  int    num_filter_pass = 1;
  int    x_refine = 2, y_refine = 2;
  double basin_depth = 5000, bottom_depth = 5000, min_depth = 10, scale_factor = 1;
  double gauss_amp = 0.5, gauss_scale = 0.25, slope_x = 0, slope_y = 0;
  double bowl_south = 60, bowl_north = 70, bowl_west = 0, bowl_east = 20;
  int    flat_bottom = 0, fill_first_row = 0;
  int    filter_topog = 0, round_shallow = 0, fill_shallow = 0;
  int    deepen_shallow = 0, smooth_topo_allow_deepening = 0;
  int    errflg = (argc == 1);
  int    option_index, i, c;
  int ret;
  
  /*
   * process command line
   */

  static struct option long_options[] = {
    {"mosaic",                      required_argument, NULL, 'a'},
    {"topog_type",                  required_argument, NULL, 'b'},
    {"x_refine",                    required_argument, NULL, 'X'},
    {"y_refine",                    required_argument, NULL, 'Y'},
    {"basin_depth",                 required_argument, NULL, 'c'},
    {"topog_file",                  required_argument, NULL, 'd'},
    {"topog_field",                 required_argument, NULL, 'e'},
    {"bottom_depth",                required_argument, NULL, 'f'},
    {"min_depth",                   required_argument, NULL, 'g'},
    {"scale_factor",                required_argument, NULL, 'i'},
    {"num_filter_pass",             required_argument, NULL, 'j'},
    {"gauss_amp",                   required_argument, NULL, 'k'},
    {"gauss_scale",                 required_argument, NULL, 'l'},
    {"slope_x",                     required_argument, NULL, 'm'},    
    {"slope_y",                     required_argument, NULL, 'n'},
    {"bowl_south",                  required_argument, NULL, 'p'},
    {"bowl_north",                  required_argument, NULL, 'q'},
    {"bowl_west",                   required_argument, NULL, 'r'},
    {"bowl_east",                   required_argument, NULL, 's'},
    {"fill_first_row",              no_argument,       NULL, 't'},
    {"filter_topog",                no_argument,       NULL, 'u'},    
    {"round_shallow",               no_argument,       NULL, 'v'},
    {"fill_shallow",                no_argument,       NULL, 'w'},
    {"deepen_shallow",              no_argument,       NULL, 'x'},
    {"smooth_topo_allow_deepening", no_argument,       NULL, 'y'},
    {"output",                      required_argument, NULL, 'o'},
    {"help",                        no_argument,       NULL, 'h'},
    {0, 0, 0, 0},
  };

  /* start parallel */

  mpp_init(&argc, &argv);

  mpp_domain_init();  
   
  while ((c = getopt_long(argc, argv, "", long_options, &option_index)) != -1)
    switch (c) {
    case 'a':
      mosaic_file = optarg;
      break;      
    case 'b':
      strcpy(topog_type, optarg);
      break;
    case 'X':
      x_refine = atoi(optarg);
      break;
    case 'Y':
      y_refine = atoi(optarg);
      break;      
    case 'c':
      basin_depth = atof(optarg);
      break;
    case 'd':
      topog_file = optarg;
      break;
    case 'e':
      topog_field = optarg;
      break;
    case 'f':
      bottom_depth = atof(optarg);
      break; 
    case 'g':
      min_depth = atof(optarg);
      break;
    case 'i':
      scale_factor = atof(optarg);
      break;
    case 'j':
      num_filter_pass = atoi(optarg);
      break; 
    case 'k':
      gauss_amp = atof(optarg);
      break;
    case 'l':
      gauss_scale = atof(optarg);
      break;
    case 'm':
      slope_x = atof(optarg);
      break;
    case 'n':
      slope_y = atof(optarg);
      break;
    case 'p':
      bowl_south = atof(optarg);
      break;
    case 'q':
      bowl_north = atof(optarg);
      break;
    case 'r':
      bowl_west  = atof(optarg);
      break;
    case 's':
      bowl_east  = atof(optarg);
      break;
    case 't':
      fill_first_row = 1;
      break;
    case 'u':
      filter_topog = 1;
      break;
    case 'v':
      round_shallow = 1;
      break;
    case 'w':
      fill_shallow = 1;
      break;
    case 'x':
      deepen_shallow = 1;
      break;
    case 'y':
      smooth_topo_allow_deepening = 1;
      break;
    case 'o':
      strcpy(output_file,optarg);
      break; 
    case '?':
      errflg++;
      break;
    }

  if (errflg || !mosaic_file ) {
    char **u = usage;
    while (*u) { fprintf(stderr, "%s\n", *u); u++; }
    exit(2);
  }

  /* Write out arguments value  */
  if(mpp_pe() == mpp_root_pe()) printf("NOTE from make_topog ==> the topog_type is: %s\n",topog_type);

  strcpy(history,argv[0]);
  
  for(i=1;i<argc;i++) {
     strcat(history, " ");
     strcat(history, argv[i]);
  }

  if ((ret = gs_make_topog(history, mosaic_file, topog_type, x_refine, y_refine, 
                           basin_depth,  topog_file, bottom_depth, min_depth, scale_factor, 
                           num_filter_pass, gauss_amp, gauss_scale, slope_x, slope_y, 
                           bowl_south, bowl_north, bowl_west, bowl_east, fill_first_row, 
                           filter_topog, round_shallow, fill_shallow, deepen_shallow,
                           smooth_topo_allow_deepening, output_file)))
     return ret;

    
  if(mpp_pe() == mpp_root_pe() ) printf("Successfully generate %s\n",output_file);

  mpp_end();

  return 0;
}; //main
