/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	15-NOV-2011
 * Version:	5 API
 *
 * Brief synopsis: gmtmercmap will make a nice Mercator map using etopo[1|2|5].
 * It also serves as a demonstration of the GMT5 API.
 *
 */

#include "gmt_dev.h"

#define MAP_BAR_GAP	"-30p"	/* Offset color bar 30 points below map */
#define MAP_BAR_HEIGHT	"8p"	/* Height of color bar, if used */
#define MAP_OFFSET	"125p"	/* Start map 125p from paper edge when colorbar is requested */
#define TOPO_INC	500.0	/* Build cpt in steps of 500 meters */

enum GMT_enum_script {GMT_BASH_MODE = 0,	/* Write Bash script */
	GMT_CSH_MODE,				/* Write C-shell script */
	GMT_DOS_MODE};				/* Write DOS script */
	
/* Control structure for gmtmercmap */

struct GMTMERCMAP_CTRL {
	struct C {	/* -C<cptfile> */
		bool active;
		char *file;
	} C;
	struct D {	/* -D[b|c|d] */
		bool active;
		int mode;
	} D;
	struct E {	/* -E[1|2|5] */
		bool active;
		int mode;
	} E;
	struct W {	/* -W<width> */
		bool active;
		double width;
	} W;
	struct S {	/* -S */
		bool active;
	} S;
};

void *New_gmtmercmap_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTMERCMAP_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTMERCMAP_CTRL);
	C->C.file = strdup ("relief");
	C->W.width = (GMT->current.setting.proj_length_unit == GMT_CM) ? 25.0 / 2.54 : 10.0;	/* 25cm (SI/A4) or 10i (US/Letter) */
	return (C);
}

void Free_gmtmercmap_Ctrl (struct GMT_CTRL *GMT, struct GMTMERCMAP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);
	GMT_free (GMT, C);
}

int GMT_gmtmercmap_usage (struct GMTAPI_CTRL *C, int level)
{
	struct GMT_CTRL *GMT = C->GMT;
	char width[4];
	if (GMT->current.setting.proj_length_unit == GMT_CM) strcpy (width, "25c"); else strcpy (width, "10i");
	GMT_message (GMT, "gmtmercmap %s [API] - Make a Mercator color map from ETOPO[1|2|5] global relief grids\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtmercmap [-C<cpt>] [-D[b|c|d]] [-E1|2|5] [-K] [-O] [-P] [%s] [-S] [%s] [%s] [-W<width>]\n", GMT_Rgeo_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s] [%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Color palette to use [relief].\n");
	GMT_message (GMT, "\t-D Dry-run: Print equivalent GMT commands instead; no map is made.\n");
	GMT_message (GMT, "\t   Append b, c, or d for Bourne shell, C-shell, or DOS syntax [Default is Bourne].\n");
	GMT_message (GMT, "\t-E Force the ETOPO resolution chosen [auto].\n");
	GMT_explain_options (GMT, "KOP");
	GMT_message (GMT, "\t-R sets the map region [Default is -180/180/-75/75].\n");
	GMT_message (GMT, "\t-S plot a color scale beneath the map [none].\n");
	GMT_message (GMT, "\t-W Specify the width of your map [%s].\n", width);
	GMT_explain_options (GMT, "UVXcnpt.");

	return (EXIT_FAILURE);
}

int GMT_gmtmercmap_parse (struct GMTAPI_CTRL *C, struct GMTMERCMAP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtmercmap and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Processes program-specific parameters */

			case 'C':	/* CPT master file */
				Ctrl->C.active = true;
				free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Just issue equivalent GMT commands in a script */
				Ctrl->D.active = true;
				switch (opt->arg[0]) {
					case 'b':  Ctrl->D.mode = GMT_BASH_MODE; break;
					case 'c':  Ctrl->D.mode = GMT_CSH_MODE;  break;
					case 'd':  Ctrl->D.mode = GMT_DOS_MODE;  break;
					default:   Ctrl->D.mode = GMT_BASH_MODE; break;
				}
				break;
			case 'E':	/* Select the ETOPO model to use */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case '1':  Ctrl->E.mode = 1; break;
					case '2':  Ctrl->E.mode = 2;  break;
					case '5':  Ctrl->E.mode = 5;  break;
					default:   n_errors++; break;
				}
				break;
			case 'W':	/* Map width */
				Ctrl->W.active = true;
				Ctrl->W.width = GMT_to_inch (GMT, opt->arg);
				break;
			case 'S':	/* Draw scale beneath map */
				Ctrl->S.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_gmtmercmap_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); exit (code);}

#define ETOPO1M_LIMIT 100	/* ETOPO1 cut-offs in degrees squared for 1 arc min */
#define ETOPO2M_LIMIT 10000	/* ETOPO2 cut-offs in degrees squared for 2 arc min */

void set_var (int mode, char *name, char *value)
{	/* Assigns the text variable given the script mode */
	switch (mode) {
		case GMT_BASH_MODE: printf ("%s=%s\n", name, value); break;
		case GMT_CSH_MODE:  printf ("set %s = %s\n", name, value); break;
		case GMT_DOS_MODE:  printf ("set %s=%s\n", name, value); break;
	}
}

void set_dim (struct GMT_CTRL *C, int mode, char *name,  double value)
{	/* Assigns the double value given the script mode and prevailing measure unit [value is passed in inches] */
	static char unit[4] = "cimp", text[GMT_TEXT_LEN256];
	double out = value * C->session.u2u[GMT_INCH][C->current.setting.proj_length_unit];
	sprintf (text, "%g%c", out, unit[C->current.setting.proj_length_unit]);
	set_var (mode, name, text);
}

char *get_var (int mode, char *name)
{	/* Places this variable where needed in the script via the assignment static variable */
	static char assignment[GMT_BUFSIZ];
	if (mode == GMT_DOS_MODE)
		sprintf (assignment, "%%%s%%", name);
	else
		sprintf (assignment, "${%s}", name);
	return (assignment);
}

int main (int argc, char **argv)
{
	int error, min, z_ID, i_ID, c_ID, t_ID;
	
	double area, z, z_min, z_max;
	
	char file[GMT_TEXT_LEN256], z_file[GMTAPI_STRLEN], i_file[GMTAPI_STRLEN];
	char cmd[GMT_BUFSIZ], c_file[GMTAPI_STRLEN], t_file[GMTAPI_STRLEN];
	static char unit[4] = "cimp";

	struct GMT_GRID *G = NULL, *I = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_TEXTSET *T = NULL;
	struct GMTMERCMAP_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = NULL;			/* GMT API control structure */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	/* Initializing new GMT session */
	if ((API = GMT_Create_Session ("gmtmercmap", 0U)) == NULL) exit (EXIT_FAILURE);
	options = GMT_Prep_Options (API, argc-1, argv+1);
	if (API->error) return (API->error);	/* Set or get option list */
	if (!options || options->option == GMTAPI_OPT_USAGE) 
		exit (GMT_gmtmercmap_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) 
		exit (GMT_gmtmercmap_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "gmtmercmap", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-VR", "BKOPUXxYycnpt>", options)) Return (API->error);
	Ctrl = New_gmtmercmap_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtmercmap_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtmercmap main code ----------------------------*/

	/* 1. If -R is not given, we must set a default map region, here -R-180/+180/-75/+75 */
	
	if (!GMT->common.R.active) {	/* Set default world region */
		GMT->common.R.wesn[XLO] = -180.0;	GMT->common.R.wesn[XHI] = +180.0;
		GMT->common.R.wesn[YLO] =  -75.0;	GMT->common.R.wesn[YHI] =  +75.0;
		GMT->common.R.active = true;
	}
	
	/* 2. Unless -E, determine approximate map area in degrees squared (just dlon * dlat), and use it to select which ETOPO?m.nc grid to use */
	
	if (Ctrl->E.active)	/* Specified the exact resolution to use */
		min = Ctrl->E.mode;
	else {	/* Determine resolution automatically from map area */
		area = (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) * (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]);
		min = (area < ETOPO1M_LIMIT) ? 1 : ((area < ETOPO2M_LIMIT) ? 2 : 5);	/* Use etopo[1,2,5]m_grd.nc depending on area */
	}
	
	sprintf (file, "etopo%dm_grd.nc", min);	/* Make the selected file name and make sure it is accessible */
	if (GMT_access (GMT, file, R_OK)) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Unable to locate file %s in the GMT search directories\n", file);
		exit (EXIT_FAILURE);
	}
	
	if (Ctrl->D.active) {	/* Just write equivalent GMT shell script instead of making a map */
		int step = 0;
		char *comment[3] = {"#", "#", "REM"};	/* Comment for csh, bash and DOS [none], respectively */
		char *proc[3] = {"C-shell", "Bash", "DOS"};	/* Name of csh, bash and DOS */
		char prefix[GMT_TEXT_LEN64], region[GMT_TEXT_LEN256], width[GMT_TEXT_LEN256];
		time_t now = time (NULL);
		
		GMT_report (GMT, GMT_MSG_VERBOSE, "Create % script that can be run to build the map\n", proc[Ctrl->D.mode]);
		if (Ctrl->D.mode == GMT_DOS_MODE)	/* Don't know how to get process ID in DOS */
			sprintf (prefix, "tmp");
		else
			sprintf (prefix, "/tmp/$$");
			
		switch (Ctrl->D.mode) {
			case GMT_BASH_MODE: printf ("#!/bin/sh\n"); break;
			case GMT_CSH_MODE:  printf ("#!/bin/csh\n"); break;
			case GMT_DOS_MODE:  printf ("REM DOS script\n"); break;
		}
		printf ("%s Produced by gmtmercmap on %s%s\n", comment[Ctrl->D.mode], ctime (&now), comment[Ctrl->D.mode]);
		
		switch (Ctrl->D.mode) {	/* Deal with noclobber first */
			case GMT_BASH_MODE: printf ("set +o noclobber\n"); break;
			case GMT_CSH_MODE:  printf ("unset noclobber\n"); break;
		}
		printf ("%s------------------------------------------\n", comment[Ctrl->D.mode]);
		printf ("%s %d. Set variables you may change later:\n", comment[Ctrl->D.mode], ++step);
		printf ("%s Name of plot file:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "map", "merc_map.ps");
		sprintf (region, "%g/%g/%g/%g", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		sprintf (width, "%g", Ctrl->W.width);
		printf ("%s Data region:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "region", region);
		printf ("%s Map width:\n", comment[Ctrl->D.mode]);
		set_dim (GMT, Ctrl->D.mode, "width", Ctrl->W.width);
		printf ("%s Intensity of illumination:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "intensity", "1");
		printf ("%s Azimuth of illumination:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "azimuth", "45");
		printf ("%s Color table:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "cpt", Ctrl->C.file);
		if (Ctrl->S.active) {	/* Plot color bar so set variables */
			printf ("%s Center position of color bar:\n", comment[Ctrl->D.mode]);
			set_dim (GMT, Ctrl->D.mode, "scale_pos", 0.5*Ctrl->W.width);
			printf ("%s Vertical shift from map to top of color bar:\n", comment[Ctrl->D.mode]);
			set_var (Ctrl->D.mode, "scale_shift", MAP_BAR_GAP);
			printf ("%s Width of color bar:\n", comment[Ctrl->D.mode]);
			set_dim (GMT, Ctrl->D.mode, "scale_width", 0.9*Ctrl->W.width);
			printf ("%s Height of color bar:\n", comment[Ctrl->D.mode]);
			set_var (Ctrl->D.mode, "scale_height", MAP_BAR_HEIGHT);
			printf ("%s Map offset from paper edge:\n", comment[Ctrl->D.mode]);
			set_var (Ctrl->D.mode, "map_offset", MAP_OFFSET);
		}
		printf ("%s------------------------------------------\n\n", comment[Ctrl->D.mode]);
		printf ("%s %d. Extract grid subset:\n", comment[Ctrl->D.mode], ++step);
		printf ("grdcut %s -R%s -G%s_topo.nc\n", file, get_var (Ctrl->D.mode, "region"), prefix);
		printf ("%s %d. Compute intensity grid for artificial illumination:\n", comment[Ctrl->D.mode], ++step);
		printf ("grdgradient %s_topo.nc -Nt%s -A%s -fg -G%s_int.nc\n", prefix, get_var (Ctrl->D.mode, "intensity"), get_var (Ctrl->D.mode, "azimuth"), prefix);
		printf ("%s %d. Determine symmetric relief range and get suitable CPT file:\n", comment[Ctrl->D.mode], ++step);
		switch (Ctrl->D.mode) {
			case GMT_BASH_MODE: printf ("T_opt=`grdinfo %s_topo.nc -Ts%g`\n", prefix, TOPO_INC); break;
			case GMT_CSH_MODE:  printf ("set T_opt = `grdinfo %s_topo.nc -Ts%g`\n", prefix, TOPO_INC); break;
			case GMT_DOS_MODE: /* Must determine the grdinfo result directly */
				if ((t_ID = GMT_Register_IO (API, GMT_IS_TEXTSET, GMT_IS_DUPLICATE, GMT_IS_NONE, GMT_OUT, NULL, T)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
				if (GMT_Encode_ID (API, t_file, t_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
				sprintf (cmd, "%s -R%s -Ts%g ->%s", file, region, TOPO_INC, t_file);			/* The grdinfo command line */
				if (GMT_grdinfo (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* This will return the -T<string> back via the T textset */
				if ((T = GMT_Retrieve_Data (API, t_ID)) == NULL) exit (EXIT_FAILURE);	/* Get pointer to that container with the input textset */
				printf ("set T_opt=%s\n", T->table[0]->segment[0]->record[0]);
				printf ("makecpt -C%s %%T_opt%% -Z > %s_color.cpt\n", get_var (Ctrl->D.mode, "cpt"), prefix);
				break;
		}
		if (Ctrl->D.mode != GMT_DOS_MODE) printf ("makecpt -C%s $T_opt -Z > %s_color.cpt\n", get_var (Ctrl->D.mode, "cpt"), prefix);
		printf ("%s %d. Make the color map:\n", comment[Ctrl->D.mode], ++step);
		printf ("grdimage %s_topo.nc -I%s_int.nc -C%s_color.cpt -JM%s", prefix, prefix, prefix, get_var (Ctrl->D.mode, "width"));
		if (GMT->common.B.active[0] || GMT->common.B.active[1]) {	/* Specified a custom -B option */
			if (GMT->common.B.active[0]) printf (" -B%s", GMT->common.B.string[0]);
			if (GMT->common.B.active[1]) printf (" -B%s", GMT->common.B.string[1]);
		}
		else
			printf (" -BaWSne");	/* Add default frame annotation */
		if (GMT->common.O.active) printf (" -O");	/* Add optional user options */
		if (GMT->common.P.active) printf (" -P");	/* Add optional user options */
		if (Ctrl->S.active || GMT->common.K.active) printf (" -K");	/* Either gave -K or implicit via -S */
		if (!GMT->common.X.active && !GMT->common.O.active) printf (" -Xc");	/* User gave neither -X nor -O so we center the map */
		if (Ctrl->S.active) {	/* May need to add some vertical offset to account for the color scale */
			if (!GMT->common.Y.active && !GMT->common.K.active) printf (" -Y%s", get_var (Ctrl->D.mode, "map_offset"));	/* User gave neither -K nor -Y so we add 0.75i offset to fit the scale */
		}
		printf (" > %s\n", get_var (Ctrl->D.mode, "map"));
		if (Ctrl->S.active) {	/* Plot color bar centered beneath map */
			printf ("%s %d. Overlay color scale:\n", comment[Ctrl->D.mode], ++step);
			printf ("psscale -C%s_color.cpt -D%s/%s/%s/%sh -Ba/:m: -O", prefix, get_var (Ctrl->D.mode, "scale_pos"), get_var (Ctrl->D.mode, "scale_shift"), get_var (Ctrl->D.mode, "scale_width"), get_var (Ctrl->D.mode, "scale_height"));	/* The psscale command line */
			if (GMT->common.K.active) printf (" -K");		/* Add optional user options */
			printf (" >> %s\n", get_var (Ctrl->D.mode, "map"));
		}
		printf ("%s %d. Remove temporary files:\n", comment[Ctrl->D.mode], ++step);
		if (Ctrl->D.mode == GMT_DOS_MODE) printf ("del %s_*.*\n", prefix); else printf ("rm -f %s_*\n", prefix);
		Return (EXIT_SUCCESS);
	}
	
	/* Here we actuallly make the map */
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Create Mercator map of area %g/%g/%g/%g with width %g%c\n",
		GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI], 
		Ctrl->W.width * GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit],
		unit[GMT->current.setting.proj_length_unit]);
		
	/* 3. Load in the subset from the selected etopo?m.nc grid */
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Read subset from %s\n", file);
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, GMT->common.R.wesn, file, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 4. Compute the illumination grid via GMT_grdgradient */
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Compute artificial illumination grid from %s\n", file);
	/* Register the topography as read-only input and register the output intensity surface to a memory location */
	if ((z_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_READONLY|GMT_IO_RESET, GMT_IS_SURFACE, GMT_IN, NULL, G)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if ((i_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, z_file, z_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if (GMT_Encode_ID (API, i_file, i_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	GMT_memset (cmd, GMT_BUFSIZ, char);
	sprintf (cmd, "%s -G%s -Nt1 -A45 -fg", z_file, i_file);			/* The grdgradient command line */
	if (GMT_grdgradient (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* This will write the intensity grid to an internal allocated container */
	if ((I = GMT_Retrieve_Data (API, i_ID)) == NULL) exit (EXIT_FAILURE);	/* Get pointer to that container with the output grid */
	
	/* 5. Determine a reasonable color range based on TOPO_INC m intervals and retrieve a CPT */
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Determine suitable color range and build CPT file\n");
	/* Round off to nearest TOPO_INC m and make a symmetric scale about zero */
	z_min = floor (G->header->z_min/TOPO_INC)*TOPO_INC;
	z_max = floor (G->header->z_max/TOPO_INC)*TOPO_INC;
	z = MAX (fabs (z_min), fabs (z_max));	/* Make it symmetrical about zero */
	/* Register the output CPT file to a memory location */
	if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_REFERENCE, GMT_IS_NONE, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, c_file, c_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	GMT_memset (cmd, GMT_BUFSIZ, char);
	sprintf (cmd, "-C%s -T%g/%g/%g -Z ->%s", Ctrl->C.file, -z, z, TOPO_INC, c_file);	/* The makecpt command line */
	if (GMT_makecpt (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);		/* This will write the output CPT to memory */
	if ((P = GMT_Retrieve_Data (API, c_ID)) == NULL) exit (EXIT_FAILURE);	/* Get pointer to the CPT stored in the allocated memory */
	
	/* 6. Now make the map */
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Generate the Mercator map\n");
	/* Register the three input sources (2 grids and 1 CPT); output is PS that goes to stdout */
	if ((z_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_READONLY|GMT_IO_RESET, GMT_IS_SURFACE, GMT_IN, NULL, G)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, z_file, z_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((i_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_READONLY|GMT_IO_RESET, GMT_IS_SURFACE, GMT_IN, NULL, I)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, i_file, i_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_READONLY|GMT_IO_RESET, GMT_IS_NONE, GMT_IN, NULL, P)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, c_file, c_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	GMT_memset (cmd, GMT_BUFSIZ, char);
	sprintf (cmd, "%s -I%s -C%s -JM%gi -BaWSne", z_file, i_file, c_file, Ctrl->W.width);	/* The grdimage command line */
	if (GMT->common.O.active) strcat (cmd, " -O");	/* Add optional user options */
	if (GMT->common.P.active) strcat (cmd, " -P");	/* Add optional user options */
	if (Ctrl->S.active || GMT->common.K.active) strcat (cmd, " -K");	/* Either gave -K or it is implicit via -S */
	if (!GMT->common.X.active && !GMT->common.O.active) strcat (cmd, " -Xc");	/* User gave neither -X nor -O so we center the map */
	if (Ctrl->S.active) {	/* May need to add some vertical offset to account for the color scale */
		if (!GMT->common.Y.active && !GMT->common.K.active) strcat (cmd, " -Y" MAP_OFFSET);	/* User gave neither -K nor -Y so we add 0.75i offset to fit the scale */
	}
	if (GMT_grdimage (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* Lay down the Mercator image */
	
	/* 7. Plot the optional color scale */
	
	if (Ctrl->S.active) {
		double x = 0.5 * Ctrl->W.width;	/* Centered beneath the map */
		GMT_report (GMT, GMT_MSG_VERBOSE, "Append color scale bar\n");
		/* Register the CPT to be used by psscale */
		if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_READONLY|GMT_IO_RESET, GMT_IS_NONE, GMT_IN, NULL, P)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
		if (GMT_Encode_ID (API, c_file, c_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
		GMT_memset (cmd, GMT_BUFSIZ, char);
		sprintf (cmd, "-C%s -D%gi/%s/%gi/%sh -Ba/:m: -O", c_file, x, MAP_BAR_GAP, 0.9*Ctrl->W.width, MAP_BAR_HEIGHT);	/* The psscale command line */
		if (GMT->common.K.active) strcat (cmd, " -K");		/* Add optional user options */
		if (GMT_psscale (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* Place the color bar */
	}
	
	/* 8. Let the GMT API garbage collection free the memory used */
	GMT_report (GMT, GMT_MSG_VERBOSE, "Mapping completed\n");
	
	Return (EXIT_SUCCESS);
}
