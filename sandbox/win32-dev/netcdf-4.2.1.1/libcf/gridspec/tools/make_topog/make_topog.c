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
  if(x_refine != 2 || y_refine != 2 ) mpp_error("Error from make_topog: x_refine and y_refine should be 2, contact developer");
  if(mpp_pe() == mpp_root_pe()) printf("NOTE from make_topog ==> x_refine is %d, y_refine is %d\n",
				       x_refine, y_refine);

  if (strcmp(topog_type,"rectangular_basin") == 0) {
    if(mpp_pe() == mpp_root_pe()) printf("NOTE from make_topog ==> the basin depth is %f\n",basin_depth);
  }
  else if (strcmp(topog_type,"gaussian") == 0) {
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n", bottom_depth );
      printf("NOTE from make_topog ==> min_depth is: %f\n", min_depth );
      printf("NOTE from make_topog ==> gauss_amp is: %f\n", gauss_amp );
      printf("NOTE from make_topog ==> gauss_scale is: %f\n", gauss_scale );
      printf("NOTE from make_topog ==> slope_x is: %f\n", slope_x );
      printf("NOTE from make_topog ==> slope_y is: %f\n", slope_y );      
    }
  }
  else if(strcmp(topog_type,"bowl") == 0) {
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n",bottom_depth);
      printf("NOTE from make_topog ==> min_depth is: %f\n",min_depth);
      printf("NOTE from make_topog ==> bowl_south is: %f\n",bowl_south);
      printf("NOTE from make_topog ==> bowl_north is: %f\n",bowl_north);
      printf("NOTE from make_topog ==> bowl_west is: %f\n",bowl_west);
      printf("NOTE from make_topog ==> bowl_east is: %f\n",bowl_east);
    }
  }
  else if(strcmp(topog_type,"idealized") == 0) {
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n",bottom_depth);
      printf("NOTE from make_topog ==> min_depth is: %f\n",min_depth);
    }
  }
  else if(strcmp(topog_type,"realistic") == 0) {
    if(!topog_file || !topog_field)
      mpp_error("Error from make_topog: when topog_type is realistic, topog_file and topog_field must be specified.");
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n",bottom_depth);
      printf("NOTE from make_topog ==> min_depth is: %f\n",min_depth);
      printf("NOTE from make_topog ==> topog_file is: %s\n", topog_file);
      printf("NOTE from make_topog ==> topog_field is: %s\n", topog_field);
      printf("NOTE from make_topog ==> scale_factor is: %f\n", scale_factor);
      printf("NOTE from make_topog ==> num_filter_pass is: %d\n", num_filter_pass);
      if(fill_first_row) printf("NOTE from make_topog ==>make first row of ocean model all land points.\n");
      if(filter_topog) printf("NOTE from make_topog ==>will apply filter to topography.\n");
      if(round_shallow) printf("NOTE from make_topog ==>Make cells land if depth is less than 1/2 "
			       "mimumim depth, otherwise make ocean.\n");
      if(fill_shallow) printf("NOTE from make_topog ==>Make cells less than minimum depth land.\n");
      if(deepen_shallow) printf("NOTE from make_topog ==>Make cells less than minimum depth equal to minimum depth.\n");
      if(smooth_topo_allow_deepening) printf("NOTE from make_topog ==>allow filter to deepen cells.\n");
    }
  }
  else {
    mpp_error("make_topog: topog_type should be rectangular_basin, gaussian, bowl, idealized or realistic");
  }
  
  if(mpp_pe() == mpp_root_pe()) {
    printf("**************************************************\n");
    printf("Begin to generate topography \n");
  }

  {
    const int STRING = 255;
    int m_fid, g_fid, vid;
    int ntiles, fid, dim_ntiles, n, dims[2];
    size_t start[4], nread[4], nwrite[4];
    int *nx, *ny, *nxp, *nyp;
    int *id_depth;
    double *depth, *x, *y;
    char **tile_files;
    char history[512], dimx_name[128], dimy_name[128], depth_name[128];
    char gridfile[256], griddir[256];
    
    /* history will be write out as global attribute
       in output file to specify the command line arguments
    */

    strcpy(history,argv[0]);

    for(i=1;i<argc;i++) {
      strcat(history, " ");
      strcat(history, argv[i]);
    }

    /* grid should be located in the same directory of mosaic file */
    get_file_path(mosaic_file, griddir);
    
    /* get mosaic dimension */
    m_fid = mpp_open(mosaic_file, MPP_READ);
    ntiles = mpp_get_dimlen( m_fid, "ntiles");
    tile_files = (char **)malloc(ntiles*sizeof(double *));
    id_depth = (int *)malloc(ntiles*sizeof(int));
    /* loop through each tile to get tile information and set up meta data for output file */
    fid = mpp_open(output_file, MPP_WRITE);
    mpp_def_global_att(fid, "grid_version", grid_version);
    mpp_def_global_att(fid, "code_version", tagname);
    mpp_def_global_att(fid, "history", history);
    dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
    nx = (int *)malloc(ntiles*sizeof(int));
    ny = (int *)malloc(ntiles*sizeof(int));
    nxp = (int *)malloc(ntiles*sizeof(int));
    nyp = (int *)malloc(ntiles*sizeof(int));   
    for( n = 0; n < ntiles; n++ ) {
      tile_files[n] = (char *)malloc(STRING*sizeof(double));
      start[0] = n;
      start[1] = 0;
      nread[0] = 1;
      nread[1] = STRING;
      vid = mpp_get_varid(m_fid, "gridfiles");
      mpp_get_var_value_block(m_fid, vid, start, nread, gridfile);
      sprintf(tile_files[n], "%s/%s", griddir, gridfile);
      g_fid = mpp_open(tile_files[n], MPP_READ);
      nx[n] = mpp_get_dimlen(g_fid, "nx");
      ny[n] = mpp_get_dimlen(g_fid, "ny");
      if( nx[n]%x_refine != 0 ) mpp_error("make_topog: supergrid x-size can not be divided by x_refine");
      if( ny[n]%y_refine != 0 ) mpp_error("make_topog: supergrid y-size can not be divided by y_refine");
      nx[n] /= x_refine;
      ny[n] /= y_refine;
      nxp[n] = nx[n] + 1;
      nyp[n] = ny[n] + 1;
      if(ntiles == 1) {
	strcpy(dimx_name, "nx");
	strcpy(dimy_name, "ny");
	strcpy(depth_name, "depth");
      }
      else {
	sprintf(dimx_name, "nx_tile%d", n+1);
	sprintf(dimy_name, "ny_tile%d", n+1);
	sprintf(depth_name, "depth_tile%d", n+1);
      }

      dims[1] = mpp_def_dim(fid, dimx_name, nx[n]); 
      dims[0] = mpp_def_dim(fid, dimy_name, ny[n]);
      id_depth[n] = mpp_def_var(fid, depth_name, NC_DOUBLE, 2, dims,  2, "standard_name",
				"topographic depth at T-cell centers", "units", "meters");
      mpp_close(g_fid);
    }
    mpp_close(m_fid);
    mpp_end_def(fid);

    /* Generate topography and write out to the output_file */
    for(n=0; n<ntiles; n++) {
      int layout[2], isc, iec, jsc, jec, nxc, nyc, ni, i, j;
      double *gdata, *tmp;
      domain2D domain;
      
      /* define the domain, each tile will be run on all the processors. */
      mpp_define_layout( nx[n], ny[n], mpp_npes(), layout);
      mpp_define_domain2d( nx[n], ny[n], layout, 0, 0, &domain);
      mpp_get_compute_domain2d( domain, &isc, &iec, &jsc, &jec);
      nxc = iec - isc + 1;
      nyc = jec - jsc + 1;
      
      depth = (double *)malloc(nxc*nyc*sizeof(double));
      x     = (double *)malloc((nxc+1)*(nyc+1)*sizeof(double));
      y     = (double *)malloc((nxc+1)*(nyc+1)*sizeof(double));
      tmp   = (double *)malloc((nxc*x_refine+1)*(nyc*y_refine+1)*sizeof(double));
      start[0] = jsc*y_refine; start[1] = isc*x_refine;
      nread[0] = nyc*y_refine+1; nread[1] = nxc*x_refine+1;
      ni       = nxc*x_refine+1;
      g_fid = mpp_open(tile_files[n], MPP_READ);
      vid = mpp_get_varid(g_fid, "x");
      mpp_get_var_value_block(g_fid, vid, start, nread, tmp);
      for(j = 0; j < nyc+1; j++) for(i = 0; i < nxc+1; i++)
	x[j*(nxc+1)+i] = tmp[(j*y_refine)*ni+i*x_refine];
      vid = mpp_get_varid(g_fid, "y");
      mpp_get_var_value_block( g_fid, vid, start, nread, tmp);
      mpp_close(g_fid);
      for(j = 0; j < nyc+1; j++) for(i = 0; i < nxc+1; i++)
	y[j*(nxc+1)+i] = tmp[(j*y_refine)*ni+i*x_refine];
      if (strcmp(topog_type,"rectangular_basin") == 0)
	create_rectangular_topog(nx[n], ny[n], basin_depth, depth);
      else if (strcmp(topog_type,"gaussian") == 0)
	create_gaussian_topog(nx[n], ny[n], x, y, bottom_depth, min_depth,
			      gauss_amp, gauss_scale, slope_x, slope_y, depth);
      else if (strcmp(topog_type,"bowl") == 0)
	create_bowl_topog(nx[n], ny[n], x, y, bottom_depth, min_depth, bowl_east,
			  bowl_south, bowl_west, bowl_north, depth);
      else if (strcmp(topog_type,"idealized") == 0)
	create_idealized_topog( nx[n], ny[n], x, y, bottom_depth, min_depth, depth);
      else if (strcmp(topog_type,"realistic") == 0)
	create_realistic_topog(nxc, nyc, x, y, topog_file, topog_field, scale_factor,
			       fill_first_row, filter_topog, num_filter_pass,
			       smooth_topo_allow_deepening, round_shallow, fill_shallow,
			       deepen_shallow, min_depth, depth );
      gdata = (double *)malloc(nx[n]*ny[n]*sizeof(double));
      mpp_global_field_double(domain, nxc, nyc, depth, gdata);
      mpp_put_var_value(fid, id_depth[n], gdata);
      free(x);
      free(y);
      free(tmp);
      free(depth);
      free(gdata);
      mpp_delete_domain2d(&domain);
    }
    mpp_close(fid);
  
    /*release memory */
    free(id_depth);
    for(n=0; n<ntiles; n++) free(tile_files[n]);
    free(tile_files);
    free(nx);
    free(ny);
    free(nxp);
    free(nyp);
  }
    
  if(mpp_pe() == mpp_root_pe() ) printf("Successfully generate %s\n",output_file);

  mpp_end();

  return 0;
}; //main
