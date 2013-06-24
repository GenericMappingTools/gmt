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

#include "gmt.h"
#include <math.h>
#include <string.h>
#include <time.h>

#define GMT_PROG_OPTIONS "->BKOPRUVXYcnptxy"

#define MAP_BAR_GAP	"-30p"	/* Offset color bar 30 points below map */
#define MAP_BAR_HEIGHT	"8p"	/* Height of color bar, if used */
#define MAP_OFFSET	"125p"	/* Start map 125p from paper edge when colorbar is requested */
#define TOPO_INC	500.0	/* Build cpt in steps of 500 meters */

enum GMT_enum_script {GMT_BASH_MODE = 0,	/* Write Bash script */
	GMT_CSH_MODE,				/* Write C-shell script */
	GMT_DOS_MODE};				/* Write DOS script */
	
/* Control structure for gmtmercmap */

struct PROG_CTRL {
	struct C {	/* -C<cptfile> */
		unsigned int active;
		char *file;
	} C;
	struct D {	/* -D[b|c|d] */
		unsigned int active;
		int mode;
	} D;
	struct E {	/* -E[1|2|5] */
		unsigned int active;
		int mode;
	} E;
	struct W {	/* -W<width> */
		unsigned int active;
		double width;
	} W;
	struct S {	/* -S */
		unsigned int active;
	} S;
};

void *New_Prog_Ctrl (unsigned int length_unit) {	/* Allocate and initialize a new control structure */
	struct PROG_CTRL *C;

	C = calloc (1, sizeof (struct PROG_CTRL));
	C->C.file = strdup ("relief");
	C->W.width = (length_unit == 0) ? 25.0 / 2.54 : 10.0;	/* 25cm (SI/A4) or 10i (US/Letter) */
	return (C);
}

void Free_Prog_Ctrl (struct PROG_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);
	free ((void*)C);
}

int Gmtmercmap_Usage (void *API, unsigned int length_unit, int level)
{
	char width[4];
	if (length_unit == 0) strcpy (width, "25c"); else strcpy (width, "10i");
	GMT_Message (API, GMT_TIME_NONE, "gmtmercmap - Make a Mercator color map from ETOPO[1|2|5] global relief grids\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtmercmap [-C<cpt>] [-D[b|c|d]] [-E1|2|5] [-K] [-O] [-P]\n\t[%s] [-S] [%s] [%s]\n", GMT_R2_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W<width>] [%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette to use [relief].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Dry-run: Print equivalent GMT commands instead; no map is made.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append b, c, or d for Bourne shell, C-shell, or DOS syntax [Default is Bourne].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Force the ETOPO resolution chosen [auto].\n");
	GMT_Option (API, "K,O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-R sets the map region [Default is -180/180/-75/75].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S plot a color scale beneath the map [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify the width of your map [%s].\n", width);
	GMT_Option (API, "U,V,X,c,n,p,t,.");

	return (EXIT_FAILURE);
}

int Gmtmercmap_Parse (void *API, struct PROG_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtmercmap and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		if (strchr (GMT_PROG_OPTIONS, opt->option)) continue;	/* Common options already processed */

		switch (opt->option) {
			/* Processes program-specific parameters */

			case 'C':	/* CPT master file */
				Ctrl->C.active = 1;
				free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Just issue equivalent GMT commands in a script */
				Ctrl->D.active = 1;
				switch (opt->arg[0]) {
					case 'b':  Ctrl->D.mode = GMT_BASH_MODE; break;
					case 'c':  Ctrl->D.mode = GMT_CSH_MODE;  break;
					case 'd':  Ctrl->D.mode = GMT_DOS_MODE;  break;
					default:   Ctrl->D.mode = GMT_BASH_MODE; break;
				}
				break;
			case 'E':	/* Select the ETOPO model to use */
				Ctrl->E.active = 1;
				switch (opt->arg[0]) {
					case '1':  Ctrl->E.mode = 1; break;
					case '2':  Ctrl->E.mode = 2;  break;
					case '5':  Ctrl->E.mode = 5;  break;
					default:   n_errors++; break;
				}
				break;
			case 'W':	/* Map width */
				Ctrl->W.active = 1;
				GMT_Get_Value (API, opt->arg, &Ctrl->W.width);
				break;
			case 'S':	/* Draw scale beneath map */
				Ctrl->S.active = 1;
				break;

			default:	/* Report bad options */
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Unrecognized argument %c%s\n", opt->option, opt->arg);
				n_errors++;
				break;
		}
	}

	return (n_errors);
}

#define Exit(code) {Free_Prog_Ctrl (Ctrl); exit (code);}

#define ETOPO1M_LIMIT 100	/* ETOPO1 cut-offs in degrees squared for 1 arc min */
#define ETOPO2M_LIMIT 10000	/* ETOPO2 cut-offs in degrees squared for 2 arc min */

double cm2unit[4] = {1.0, 1.0/2.54, 0.01, 72.0/2.54};

void set_var (int mode, char *name, char *value)
{	/* Assigns the text variable given the script mode */
	switch (mode) {
		case GMT_BASH_MODE: printf ("%s=%s\n", name, value); break;
		case GMT_CSH_MODE:  printf ("set %s = %s\n", name, value); break;
		case GMT_DOS_MODE:  printf ("set %s=%s\n", name, value); break;
	}
}

void set_dim (int mode, unsigned int length_unit, char *name, double value)
{	/* Assigns the double value given the script mode and prevailing measure unit [value is passed in inches] */
	static char unit[4] = "cimp", text[256];
	//double out = value * cm2unit[length_unit];
	double out = value;
	sprintf (text, "%g%c", out, unit[length_unit]);
	set_var (mode, name, text);
}

char *get_var (int mode, char *name)
{	/* Places this variable where needed in the script via the assignment static variable */
	static char assignment[BUFSIZ];
	if (mode == GMT_DOS_MODE)
		sprintf (assignment, "%%%s%%", name);
	else
		sprintf (assignment, "${%s}", name);
	return (assignment);
}

int main (int argc, char **argv)
{
	int error, min, z_ID, i_ID, c_ID, t_ID;
	unsigned int B_active, K_active, O_active, P_active, X_active, Y_active;
	unsigned int length_unit = 0;
	
	double area, z, z_min, z_max, wesn[4];
	
	char file[256], z_file[GMT_STR16], i_file[GMT_STR16];
	char cmd[BUFSIZ], c_file[GMT_STR16], t_file[GMT_STR16], def_unit[16];
	static char unit[4] = "cimp";

	struct GMT_GRID *G = NULL, *I = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_TEXTSET *T = NULL;
	struct PROG_CTRL *Ctrl = NULL;
	struct GMT_OPTION *options = NULL;
	void *API = NULL;			/* API control structure */

	/*----------------------- Standard module initialization and parsing ----------------------*/

        /* Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], 2U, 0U, NULL)) == NULL) exit (EXIT_FAILURE);
        /* Convert argc,argv to linked option list */
	options = GMT_Create_Options (API, argc-1, (argv+1));

	if (!options || options->option == GMT_OPT_USAGE) 
		exit (Gmtmercmap_Usage (API, length_unit, GMT_USAGE));		/* Exit the usage message */
	if (options && options->option == GMT_OPT_SYNOPSIS) 
		exit (Gmtmercmap_Usage (API, length_unit, GMT_SYNOPSIS));	/* Exit the synopsis */

	/* Parse the common command-line arguments */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) exit (EXIT_FAILURE);	/* Parse the common options */
	GMT_Get_Default (API, "PROJ_LENGTH_UNIT", def_unit);
	if (!strcmp (def_unit, "cm")) length_unit = 0;
	else if (!strcmp (def_unit, "inch")) length_unit = 1;
	else if (!strcmp (def_unit, "m")) length_unit = 2;
	else if (!strcmp (def_unit, "point")) length_unit = 3;

	Ctrl = New_Prog_Ctrl (length_unit);	/* Allocate and initialize a new control structure */
	if ((error = Gmtmercmap_Parse (API, Ctrl, options))) Exit (error);

	/*---------------------------- This is the gmtmercmap main code ----------------------------*/

	/* 1. If -R is not given, we must set a default map region, here -R-180/+180/-75/+75 */
	
	if (GMT_Get_Common (API, 'R', wesn) == GMT_NOTSET){	/* Get or set default world region */
		wesn[GMT_XLO] = -180.0;	wesn[GMT_XHI] = +180.0;
		wesn[GMT_YLO] =  -75.0;	wesn[GMT_YHI] =  +75.0;
	}
	B_active = (GMT_Get_Common (API, 'B', NULL) == 0);	/* 1 if -B was specified */
	K_active = (GMT_Get_Common (API, 'K', NULL) == 0);	/* 1 if -K was specified */
	O_active = (GMT_Get_Common (API, 'O', NULL) == 0);	/* 1 if -O was specified */
	P_active = (GMT_Get_Common (API, 'P', NULL) == 0);	/* 1 if -P was specified */
	X_active = (GMT_Get_Common (API, 'X', NULL) == 0);	/* 1 if -X was specified */
	Y_active = (GMT_Get_Common (API, 'Y', NULL) == 0);	/* 1 if -Y was specified */
	
	/* 2. Unless -E, determine approximate map area in degrees squared (just dlon * dlat), and use it to select which ETOPO?m.nc grid to use */
	
	if (Ctrl->E.active)	/* Specified the exact resolution to use */
		min = Ctrl->E.mode;
	else {	/* Determine resolution automatically from map area */
		area = (wesn[GMT_XHI] - wesn[GMT_XLO]) * (wesn[GMT_YHI] - wesn[GMT_YLO]);
		min = (area < ETOPO1M_LIMIT) ? 1 : ((area < ETOPO2M_LIMIT) ? 2 : 5);	/* Use etopo[1,2,5]m_grd.nc depending on area */
	}

	sprintf (file, "etopo%dm_grd.nc", min);	/* Make the selected file name and make sure it is accessible */
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to locate file %s in the GMT search directories\n", file);
		Exit (EXIT_FAILURE);
	}
	
	if (Ctrl->D.active) {	/* Just write equivalent GMT shell script instead of making a map */
		int step = 0;
		char *comment[3] = {"#", "#", "REM"};	/* Comment for csh, bash and DOS [none], respectively */
		char *proc[3] = {"C-shell", "Bash", "DOS"};	/* Name of csh, bash and DOS */
		char prefix[64], region[256], width[256];
		struct GMT_OPTION *opt = NULL;
		time_t now = time (NULL);
		
		GMT_Report (API, GMT_MSG_VERBOSE, "Create % script that can be run to build the map\n", proc[Ctrl->D.mode]);
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
		sprintf (region, "%g/%g/%g/%g", wesn[GMT_XLO], wesn[GMT_XHI], wesn[GMT_YLO], wesn[GMT_YHI]);
		sprintf (width, "%g", Ctrl->W.width);
		printf ("%s Data region:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "region", region);
		printf ("%s Map width:\n", comment[Ctrl->D.mode]);
		set_dim (Ctrl->D.mode, length_unit, "width", Ctrl->W.width);
		printf ("%s Intensity of illumination:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "intensity", "1");
		printf ("%s Azimuth of illumination:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "azimuth", "45");
		printf ("%s Color table:\n", comment[Ctrl->D.mode]);
		set_var (Ctrl->D.mode, "cpt", Ctrl->C.file);
		if (Ctrl->S.active) {	/* Plot color bar so set variables */
			printf ("%s Center position of color bar:\n", comment[Ctrl->D.mode]);
			set_dim (Ctrl->D.mode, length_unit, "scale_pos", 0.5*Ctrl->W.width);
			printf ("%s Vertical shift from map to top of color bar:\n", comment[Ctrl->D.mode]);
			set_var (Ctrl->D.mode, "scale_shift", MAP_BAR_GAP);
			printf ("%s Width of color bar:\n", comment[Ctrl->D.mode]);
			set_dim (Ctrl->D.mode, length_unit, "scale_width", 0.9*Ctrl->W.width);
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
				if ((t_ID = GMT_Register_IO (API, GMT_IS_TEXTSET, GMT_IS_DUPLICATE, GMT_IS_NONE, GMT_OUT, NULL, T)) == GMT_NOTSET) exit (EXIT_FAILURE);
				if (GMT_Encode_ID (API, t_file, t_ID) != GMT_NOERROR) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
				sprintf (cmd, "%s -R%s -Ts%g ->%s", file, region, TOPO_INC, t_file);			/* The grdinfo command line */
				if (GMT_Call_Module (API, "grdinfo", GMT_MODULE_CMD, cmd) != GMT_NOERROR) exit (EXIT_FAILURE);	/* This will return the -T<string> back via the T textset */
				if ((T = GMT_Retrieve_Data (API, t_ID)) == NULL) exit (EXIT_FAILURE);	/* Get pointer to that container with the input textset */
				printf ("set T_opt=%s\n", T->table[0]->segment[0]->record[0]);
				printf ("makecpt -C%s %%T_opt%% -Z > %s_color.cpt\n", get_var (Ctrl->D.mode, "cpt"), prefix);
				break;
		}
		if (Ctrl->D.mode != GMT_DOS_MODE) printf ("makecpt -C%s $T_opt -Z > %s_color.cpt\n", get_var (Ctrl->D.mode, "cpt"), prefix);
		printf ("%s %d. Make the color map:\n", comment[Ctrl->D.mode], ++step);
		printf ("grdimage %s_topo.nc -I%s_int.nc -C%s_color.cpt -JM%s", prefix, prefix, prefix, get_var (Ctrl->D.mode, "width"));
		if (!B_active)
			printf (" -Ba -BWSne");	/* Add default frame annotation */
		else {	/* Loop over arguments and find all -B options */
			for (opt = options; opt; opt = opt->next)
				if (opt->option == 'B') printf (" -B%s", opt->arg);
		}
		if (O_active) printf (" -O");	/* Add optional user options */
		if (P_active) printf (" -P");	/* Add optional user options */
		if (Ctrl->S.active || K_active) printf (" -K");	/* Either gave -K or implicit via -S */
		if (!X_active && !O_active) printf (" -Xc");	/* User gave neither -X nor -O so we center the map */
		if (Ctrl->S.active) {	/* May need to add some vertical offset to account for the color scale */
			if (!Y_active && !K_active) printf (" -Y%s", get_var (Ctrl->D.mode, "map_offset"));	/* User gave neither -K nor -Y so we add 0.75i offset to fit the scale */
		}
		printf (" > %s\n", get_var (Ctrl->D.mode, "map"));
		if (Ctrl->S.active) {	/* Plot color bar centered beneath map */
			printf ("%s %d. Overlay color scale:\n", comment[Ctrl->D.mode], ++step);
			printf ("psscale -C%s_color.cpt -D%s/%s/%s/%sh -Bxa -By+lm -O", prefix, get_var (Ctrl->D.mode, "scale_pos"), get_var (Ctrl->D.mode, "scale_shift"), get_var (Ctrl->D.mode, "scale_width"), get_var (Ctrl->D.mode, "scale_height"));	/* The psscale command line */
			if (K_active) printf (" -K");		/* Add optional user options */
			printf (" >> %s\n", get_var (Ctrl->D.mode, "map"));
		}
		printf ("%s %d. Remove temporary files:\n", comment[Ctrl->D.mode], ++step);
		if (Ctrl->D.mode == GMT_DOS_MODE) printf ("del %s_*.*\n", prefix); else printf ("rm -f %s_*\n", prefix);
		Exit (EXIT_SUCCESS);
	}
	
	/* Here we actuallly make the map */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Create Mercator map of area %g/%g/%g/%g with width %g%c\n",
		wesn[GMT_XLO], wesn[GMT_XHI], wesn[GMT_YLO], wesn[GMT_YHI], 
		Ctrl->W.width, unit[length_unit]);
		
	/* 3. Load in the subset from the selected etopo?m.nc grid */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Read subset from %s\n", file);
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, file, G)) == NULL) Exit (EXIT_FAILURE);

	/* 4. Compute the illumination grid via GMT_grdgradient */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Compute artificial illumination grid from %s\n", file);
	/* Register the topography as read-only input and register the output intensity surface to a memory location */
	if ((z_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE|GMT_IO_RESET, GMT_IS_SURFACE, GMT_IN, NULL, G)) == GMT_NOTSET) Exit (EXIT_FAILURE);
	if ((i_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMT_NOTSET) Exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, z_file, z_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if (GMT_Encode_ID (API, i_file, i_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	memset (cmd, 0, BUFSIZ);
	sprintf (cmd, "%s -G%s -Nt1 -A45 -fg", z_file, i_file);			/* The grdgradient command line */
	if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* This will write the intensity grid to an internal allocated container */
	if ((I = GMT_Retrieve_Data (API, i_ID)) == NULL) Exit (EXIT_FAILURE);	/* Get pointer to that container with the output grid */
	
	/* 5. Determine a reasonable color range based on TOPO_INC m intervals and retrieve a CPT */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Determine suitable color range and build CPT file\n");
	/* Round off to nearest TOPO_INC m and make a symmetric scale about zero */
	z_min = floor (G->header->z_min/TOPO_INC)*TOPO_INC;
	z_max = floor (G->header->z_max/TOPO_INC)*TOPO_INC;
	z = fabs (z_min);
	if (fabs (z_max) > z) z = fabs (z_max);	/* Make it symmetrical about zero */
	/* Register the output CPT file to a memory location */
	if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_REFERENCE, GMT_IS_NONE, GMT_OUT, NULL, NULL)) == GMT_NOTSET) Exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, c_file, c_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	memset (cmd, 0, BUFSIZ);
	sprintf (cmd, "-C%s -T%g/%g/%g -Z ->%s", Ctrl->C.file, -z, z, TOPO_INC, c_file);	/* The makecpt command line */
	if (GMT_Call_Module (API, "makecpt", GMT_MODULE_CMD, cmd) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* This will write the output CPT to memory */
	if ((P = GMT_Retrieve_Data (API, c_ID)) == NULL) Exit (EXIT_FAILURE);	/* Get pointer to the CPT stored in the allocated memory */
	
	/* 6. Now make the map */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Generate the Mercator map\n");
	/* Register the three input sources (2 grids and 1 CPT); output is PS that goes to stdout */
	if ((z_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE|GMT_IO_RESET, GMT_IS_SURFACE, GMT_IN, NULL, G)) == GMT_NOTSET) Exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, z_file, z_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((i_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE|GMT_IO_RESET, GMT_IS_SURFACE, GMT_IN, NULL, I)) == GMT_NOTSET) Exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, i_file, i_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_REFERENCE|GMT_IO_RESET, GMT_IS_NONE, GMT_IN, NULL, P)) == GMT_NOTSET) Exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, c_file, c_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	memset (cmd, 0, BUFSIZ);
	sprintf (cmd, "%s -I%s -C%s -JM%g%c -Ba -BWSne", z_file, i_file, c_file, Ctrl->W.width, unit[length_unit]);	/* The grdimage command line */
	if (O_active) strcat (cmd, " -O");	/* Add optional user options */
	if (P_active) strcat (cmd, " -P");	/* Add optional user options */
	if (Ctrl->S.active || K_active) strcat (cmd, " -K");	/* Either gave -K or it is implicit via -S */
	if (!X_active && !O_active) strcat (cmd, " -Xc");	/* User gave neither -X nor -O so we center the map */
	if (Ctrl->S.active) {	/* May need to add some vertical offset to account for the color scale */
		if (!Y_active && !K_active) strcat (cmd, " -Y" MAP_OFFSET);	/* User gave neither -K nor -Y so we add 0.75i offset to fit the scale */
	}
	if (GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, cmd) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Lay down the Mercator image */
	
	/* 7. Plot the optional color scale */
	
	if (Ctrl->S.active) {
		double x = 0.5 * Ctrl->W.width;	/* Centered beneath the map in these units */
		GMT_Report (API, GMT_MSG_VERBOSE, "Append color scale bar\n");
		/* Register the CPT to be used by psscale */
		if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_REFERENCE|GMT_IO_RESET, GMT_IS_NONE, GMT_IN, NULL, P)) == GMT_NOTSET) Exit (EXIT_FAILURE);
		if (GMT_Encode_ID (API, c_file, c_ID) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
		memset (cmd, 0, BUFSIZ);
		sprintf (cmd, "-C%s -D%g%c/%s/%g%c/%sh -Bxa -By+lm -O", c_file, x, unit[length_unit], MAP_BAR_GAP, 0.9*Ctrl->W.width, unit[length_unit], MAP_BAR_HEIGHT);	/* The psscale command line */
		if (K_active) strcat (cmd, " -K");		/* Add optional user options */
		if (GMT_Call_Module (API, "psscale", GMT_MODULE_CMD, cmd) != GMT_NOERROR) Exit (EXIT_FAILURE);	/* Place the color bar */
	}
	
	/* 8. Let the GMT API garbage collection free the memory used */
	GMT_Report (API, GMT_MSG_VERBOSE, "Mapping completed\n");
	
	Exit (EXIT_SUCCESS);
}
