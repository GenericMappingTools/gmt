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
    
  /*--- if file name is not specified through -f, file name will be horizontal_grid.tile#.nc */
  if(nfiles == 0) {
    if(ntiles == 1) {
      sprintf(tilefile[0],"horizontal_grid.nc");
    }
    else {
      for(n=0; n<ntiles; n++) {
	sprintf(tilefile[n],"horizontal_grid.tile%d.nc",n+1);
      }
    }
  }
  else { /* Check if ntile are matching number of grid file passed through -f */
    if( nfiles != ntiles) mpp_error("make_solo_mosaic: number of grid files specified through -n "
				   " does not equal to number of files specified through -f = ");
  }

  n = strlen(dir);
  if( dir[n-1] != '/') strcat(dir, "/");

  /*First read all the grid files.*/
  nxp = (int *)malloc(ntiles*sizeof(int));
  nyp = (int *)malloc(ntiles*sizeof(int));
  x = (double **)malloc(ntiles*sizeof(double *));
  y = (double **)malloc(ntiles*sizeof(double *));
  for(n=0; n<ntiles; n++) {
    char filepath[512];
    int fid, vid;
    sprintf(filepath, "%s%s",dir, tilefile[n]);
    fid = mpp_open(filepath, MPP_READ);
    nxp[n] = mpp_get_dimlen(fid, "nxp");
    nyp[n] = mpp_get_dimlen(fid, "nyp");
    x[n] = (double *)malloc(nxp[n]*nyp[n]*sizeof(double));
    y[n] = (double *)malloc(nxp[n]*nyp[n]*sizeof(double));
    vid = mpp_get_varid(fid, "tile");
    mpp_get_var_value(fid, vid, tile_name[n]);
    vid = mpp_get_varid(fid, "x");
    mpp_get_var_value(fid, vid, x[n]);
    vid = mpp_get_varid(fid, "y");
    mpp_get_var_value(fid, vid, y[n]);
    mpp_close(fid);
  }


  /*find the contact region between tiles, currently assume the contact region are align-contact
    There should be no contact between same directions of two tiles ( w-w, e-e, s-s, n-n)
    We assume no contact between w-s, e-n ( will added in if needed ) . */
  ncontact = 0;
  for(n=0; n<ntiles; n++) {
    for(m=n; m<ntiles; m++) {
      int count;
      int istart1[MAXCONTACT], iend1[MAXCONTACT], jstart1[MAXCONTACT], jend1[MAXCONTACT];
      int istart2[MAXCONTACT], iend2[MAXCONTACT], jstart2[MAXCONTACT], jend2[MAXCONTACT];
      count = get_align_contact(n+1, m+1, nxp[n], nyp[n], nxp[m], nyp[m], x[n], y[n], x[m], y[m],
				periodx, periody, istart1, iend1, jstart1, jend1, istart2, iend2,
				jstart2, jend2);
      if(ncontact+count>MAXCONTACT) mpp_error("make_solo_mosaic: number of contacts is more than MAXCONTACT");
      for(l=0; l<count; l++) {
	contact_tile1_istart[ncontact] = istart1[l];
	contact_tile1_iend  [ncontact] = iend1[l];
	contact_tile1_jstart[ncontact] = jstart1[l];
	contact_tile1_jend  [ncontact] = jend1[l];
	contact_tile2_istart[ncontact] = istart2[l];
	contact_tile2_iend  [ncontact] = iend2[l];
	contact_tile2_jstart[ncontact] = jstart2[l];
	contact_tile2_jend  [ncontact] = jend2[l];
	contact_tile1       [ncontact] = n;
	contact_tile2       [ncontact] = m;
	ncontact++;
      }
    }
  }

  /* write out data */
  {
    char str[STRING], outfile[STRING];
    int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
    int id_contact_index, id_griddir, id_gridfiles, dim[2];
    size_t start[4], nwrite[4];

    sprintf(outfile, "%s.nc", mosaic_name);
    fid = mpp_open(outfile, MPP_WRITE);
    /* define dimenison */
    dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
    if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
    dim_string = mpp_def_dim(fid, "string", STRING);    
    /* define variable */
    id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 4, "standard_name",
			    "grid_mosaic_spec", "children", "gridtiles", "contact_regions", "contacts",
			    "grid_descriptor", grid_descriptor);
    dim[0] = dim_ntiles; dim[1] = dim_string;
    id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
			       "standard_name", "grid_file_location");
    id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
    id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

    if(ncontact>0) {
      dim[0] = dim_ncontact; dim[1] = dim_string;
      id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
				"contact_type", "boundary", "alignment", "true",
				"contact_index", "contact_index", "orientation", "orient");
      id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
				     "starting_ending_point_index_of_contact");

    }
    mpp_def_global_att(fid, "grid_version", grid_version);
    mpp_def_global_att(fid, "code_version", tagname);
    mpp_def_global_att(fid, "history", history);
    mpp_end_def(fid);

    /* write out data */
    for(i=0; i<4; i++) {
      start[i] = 0; nwrite[i] = 1;
    }
    nwrite[0] = strlen(mosaic_name);
    mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
    nwrite[0] = strlen(dir);
    mpp_put_var_value_block(fid, id_griddir, start, nwrite, dir);
    nwrite[0] = 1;
    for(n=0; n<ntiles; n++) {
      start[0] = n; nwrite[1] = strlen(tile_name[n]);
      mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
      nwrite[1] = strlen(tilefile[n]);
      mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
    }
    
    for(n=0; n<ncontact; n++) {
      sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
	      tile_name[contact_tile2[n]]);
      start[0] = n; nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
      sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
	      contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
	      contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
      nwrite[1] = strlen(str);
      mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
    }
    mpp_close(fid);    
  }

  
  printf("congradulation: You have successfully run make_solo_mosaic\n");

  return 0;
  
}; // end of main



