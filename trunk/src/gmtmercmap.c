/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	15-NOV-2011
 * Version:	5 API
 *
 * Brief synopsis: gmtmercmap will make a nice Mercator map using etopo[1|2|5].
 *
 */

#include "pslib.h"
#include "gmt.h"

/* Control structure for gmtmercmap */

struct GMTMERCMAP_CTRL {
	struct C {	/* -C<cptfile> */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct W {	/* -W<width> */
		GMT_LONG active;
		double width;
	} W;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
};

void *New_gmtmercmap_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTMERCMAP_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTMERCMAP_CTRL);
	C->C.file = strdup ("relief");
	C->W.width = 6.0;
	return (C);
}

void Free_gmtmercmap_Ctrl (struct GMT_CTRL *GMT, struct GMTMERCMAP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_gmtmercmap_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtmercmap %s [API] - Make a Mercator color map from ETOPO[1|2|5] global relief grids\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtmercmap [-C<cpt>] [-D] [-K] [-O] [-P] [-R<w/e/s/n>] [-S] [-W<width>]\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Color palette to use [relief].\n");
	GMT_message (GMT, "\t-D Dry-run: Only print GMT commands instead; no map is made.\n");
	GMT_explain_options (GMT, "KOP");
	GMT_message (GMT, "\t-R sets the map region [Default is -180/180/-75/75].\n");
	GMT_message (GMT, "\t-S plot a color scale beneath the map [none].\n");
	GMT_message (GMT, "\t-W Specify the width of your map [6i].\n");
	GMT_explain_options (GMT, "UVXcfnpt.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtmercmap_parse (struct GMTAPI_CTRL *C, struct GMTMERCMAP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtmercmap and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Processes program-specific parameters */

			case 'C':	/* CPT file */
				Ctrl->C.active = TRUE;
				free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Just issue equivalent GMT commands */
				Ctrl->D.active = TRUE;
				break;
			case 'W':	/* Map width */
				Ctrl->W.active = TRUE;
				Ctrl->W.width = GMT_to_inch (GMT, opt->arg);
				break;
			case 'S':	/* Draw scale beneath map */
				Ctrl->S.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_gmtmercmap_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); exit (code);}

#define ETOPO1M_LIMIT 100
#define ETOPO2M_LIMIT 10000

int main (int argc, char **argv)
{
	GMT_LONG error, min, z_ID, i_ID, c_ID;
	
	double area, z, z_min, z_max;
	
	char file[GMT_TEXT_LEN256], z_file[GMTAPI_STRLEN], i_file[GMTAPI_STRLEN], c_file[GMTAPI_STRLEN];
	char cmd[GMT_BUFSIZ];

	struct GMT_GRID *G = NULL, *I = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMTMERCMAP_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = NULL;			/* GMT API control structure */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	/* Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMTAPI_GMTPSL)) == NULL) exit (EXIT_FAILURE);
	if ((options = GMT_Prep_Options (API, argc-1, argv+1)) == NULL) exit (EXIT_FAILURE);	/* Set or get option list */
	if (!options || options->option == GMTAPI_OPT_USAGE) Return (GMT_gmtmercmap_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) Return (GMT_gmtmercmap_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtmercmap", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-VR", "KOPUXxYycnpt>", options)) Return (API->error);
	Ctrl = New_gmtmercmap_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtmercmap_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtmercmap main code ----------------------------*/

	/* 1. If -R is not given, we set a default map region */
	
	if (!GMT->common.R.active) {	/* Set default world region */
		GMT->common.R.wesn[XLO] = -180.0;	GMT->common.R.wesn[XHI] = +180.0;
		GMT->common.R.wesn[YLO] =  -75.0;	GMT->common.R.wesn[YHI] =  +75.0;
		GMT->common.R.active = TRUE;
	}
	
	/* 2. Determine map area in degrees squared, and use it to select which ETOPO?m.nc grid to use */
	
	area = (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) * (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]);
	min = (area < ETOPO1M_LIMIT) ? 1 : ((area < ETOPO2M_LIMIT) ? 2 : 5);	/* Use etopo[1,2,5]m_grd.nc depending on area */
	
	/* 3. Load in the subset from the selected etopo?m.nc grid */
	
	sprintf (file, "etopo%ldm_grd.nc", min);	/* Make the selected file name */
	
	if (Ctrl->D.active) {
		printf ("#!/bin/sh\n");
		printf ("ps=merc_map.ps\n");
		printf ("grdcut %s -R%g/%g/%g/%g -G$$_topo.nc\n", file, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		printf ("grdgradient $$_topo.nc -Nt1 -A45 -fg -G$$_int.nc\n");
		printf ("T_opt=`grdinfo $$_topo.nc -Ts500`\n");
		printf ("makecpt -C%s $T_opt -Z > $$_color.cpt\n", Ctrl->C.file);
		printf ("grdimage $$_topo.nc -I$$_int.nc -C$$_color.cpt -JM%gi -BaWSne", Ctrl->W.width);
		if (GMT->common.O.active) printf (" -O");	/* Add optional user options */
		if (GMT->common.P.active) printf (" -P");	/* Add optional user options */
		if (Ctrl->S.active || GMT->common.K.active) printf (" -K");	/* Either gave -K or implicit via -S */
		if (Ctrl->S.active) {	/* May need to add some vertical offset to account for the colro scale */
			if (!GMT->common.Y.active && !GMT->common.K.active) printf (" -Y1.75i");	/* User gave neither -K nor -Y so we add 0.75i offset to fit the scale */
		}
		printf (" > $ps\n");
		if (Ctrl->S.active) {
			double x, y;
			x = 0.5 * Ctrl->W.width;	/* Centered beneath the map */
			y = -0.4;			/* Offset vertically 0.4i downwards */
			printf ("psscale -C$$_color.cpt -D%gi/%gi/%gi/0.1ih -Ba/:m: -O", x, y, 0.9*Ctrl->W.width);	/* The psscale command line */
			if (GMT->common.K.active) printf (" -K");		/* dd optional user options */
			printf (" >> $ps\n");
		}
		printf ("rm -f $$_*\n");
		Return (EXIT_SUCCESS);
	}
	
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT->common.R.wesn, GMT_GRID_ALL, file, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 4. Compute the illumination grid via GMT_grdgradient */
	
	/* Register the topography as read-only input and register the output intensity surface to a memory location */
	if ((z_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_READONLY, GMT_IS_SURFACE, GMT_IN, G, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if ((i_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, z_file, z_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if (GMT_Encode_ID (API, i_file, i_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (cmd, "%s -G%s -Nt1 -A45 -fg", z_file, i_file);			/* The grdgradient command line */
	if (GMT_grdgradient (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* This will write the intensity grid to an internal allocated container */
	if ((I = GMT_Retrieve_Data (API, i_ID)) == NULL) exit (EXIT_FAILURE);	/* Get pointer to that container with the output grid */
	
	/* 5. Determine a reasonable color range based on 500 m intervals and retrieve a CPT */
	
	/* Round off to nearest 500 m and make a symmetric scale about zero */
	z_min = floor (G->header->z_min/500.0)*500.0;	z_max = floor (G->header->z_max/500.0)*500.0;
	z = MAX (fabs (z_min), fabs (z_max));	/* Make it symmetrical about zero */
	/* Register the output CPT file to a memory location */
	if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_REF, GMT_IS_POINT, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, c_file, c_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (cmd, "-C%s -T%g/%g/500 -Z ->%s", Ctrl->C.file, -z, z, c_file);	/* The makecpt command line */
	if (GMT_makecpt (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);		/* This will write the output CPT to memory */
	if ((P = GMT_Retrieve_Data (API, c_ID)) == NULL) exit (EXIT_FAILURE);	/* Get pointer to the CPT stored in the allocated memory */
	
	/* 6. Now make the map */
	
	/* Register the three input sources (2 grids and 1 CPT); output is PS that goes to stdout */
	if ((z_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_READONLY, GMT_IS_SURFACE, GMT_IN, G, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, z_file, z_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((i_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_READONLY, GMT_IS_SURFACE, GMT_IN, I, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, i_file, i_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_READONLY, GMT_IS_POINT, GMT_IN, P, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, c_file, c_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (cmd, "%s -I%s -C%s -JM%gi -BaWSne", z_file, i_file, c_file, Ctrl->W.width);/* The grdimage command line */
	if (GMT->common.O.active) strcat (cmd, " -O");	/* Add optional user options */
	if (GMT->common.P.active) strcat (cmd, " -P");	/* Add optional user options */
	if (Ctrl->S.active || GMT->common.K.active) strcat (cmd, " -K");	/* Either gave -K or implicit via -S */
	if (Ctrl->S.active) {	/* May need to add some vertical offset to account for the colro scale */
		if (!GMT->common.Y.active && !GMT->common.K.active) strcat (cmd, " -Y1.75i");	/* User gave neither -K nor -Y so we add 0.75i offset to fit the scale */
	}
	if (GMT_grdimage (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* Lay down the Mercator image */
	
	/* 7. Plot the optional color scale */
	
	if (Ctrl->S.active) {
		double x, y;
		x = 0.5 * Ctrl->W.width;	/* Centered beneath the map */
		y = -0.4;			/* Offset vertically 0.4i downwards */
		/* Register the CPT to be used by psscale */
		if ((c_ID = GMT_Register_IO (API, GMT_IS_CPT, GMT_IS_READONLY, GMT_IS_POINT, GMT_IN, P, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
		if (GMT_Encode_ID (API, c_file, c_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
		sprintf (cmd, "-C%s -D%gi/%gi/%gi/0.1ih -Ba/:m: -O", c_file, x, y, 0.9*Ctrl->W.width);	/* The psscale command line */
		if (GMT->common.K.active) strcat (cmd, " -K");		/* dd optional user options */
		if (GMT_psscale (API, 0, cmd) != GMT_OK) exit (EXIT_FAILURE);	/* Place the color bar */
	}
	
	/* 8. Let the GMT API garbage collection free the memory used */
	
	Return (EXIT_SUCCESS);
}
