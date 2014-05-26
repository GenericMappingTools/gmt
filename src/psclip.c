/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: psclip reads one or many xy-files and draws polygons just like psclip
 * with the exception that these polygons are set up to act as clip
 * paths for subsequent plotting.  psclip uses the even-odd winding
 * rule to decide what's inside and outside the path.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"psclip"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Initialize or terminate polygonal clip paths"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BJKOPRUVXYbcdfghipstxy" GMT_OPT("EZMm")

struct PSCLIP_CTRL {
	struct C {	/* -C */
		bool active;
		int n;	/* Number of levels to undo [1] */
	} C;
	struct N {	/* -N */
		bool active;
	} N;
	struct T {	/* -T */
		bool active;
	} T;
};

void *New_psclip_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCLIP_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSCLIP_CTRL);
	C->C.n = 1;	/* Default undoes one level of clipping */

	return (C);
}

void Free_psclip_Ctrl (struct GMT_CTRL *GMT, struct PSCLIP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_psclip_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the psclip synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psclip -C[a|<n>] [-K] [-O]  OR\n");
	GMT_Message (API, GMT_TIME_NONE, "\tpsclip <table> %s %s [%s]\n", GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-K] [-N] [-O] [-P] [-T] [%s] [%s]\n", GMT_Jz_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, GMT_di_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\n", GMT_c_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-C Undo existing clip-paths; no file is needed.  -R, -J are not required (unless -B is used).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Terminates all clipping; optionally append how many clip levels to restore [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<xy-files> is one or more polygon files.  If none, standard input is read.\n");
	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Use the outside of the polygons and the map boundary as clip paths.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set clip path for the entire map frame.  No input file is required.\n");
	GMT_Option (API, "U,V,X,bi2,di,c,f,g,h,i,p,t,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_psclip_parse (struct GMT_CTRL *GMT, struct PSCLIP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psclip and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Turn clipping off */
				Ctrl->C.active = true;
				Ctrl->C.n = PSL_ALL_CLIP;
				switch (opt->arg[0]) {
					case 'a': Ctrl->C.n = PSL_ALL_CLIP; break;
					case '\0': Ctrl->C.n = PSL_ALL_CLIP; break;	/* Default anyway */
					default:
						if (isdigit ((int)opt->arg[0]))
							Ctrl->C.n = atoi (&opt->arg[0]);
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Correct syntax is -C[<n>]\n");
							n_errors++;
						}
						break;
				}
				break;
			case 'N':	/* Use the outside of the polygons as clip area */
				Ctrl->N.active = true;
				break;
			case 'T':	/* Select map clip */
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->C.active) {
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	}
	if (Ctrl->T.active) Ctrl->N.active = true;	/* -T implies -N */
	if (Ctrl->T.active && n_files) GMT_Report (API, GMT_MSG_NORMAL, "Warning: Option -T ignores all input files\n");

	if (Ctrl->N.active && GMT->current.map.frame.init) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Option -B cannot be used in combination with Options -N or -T. -B is ignored.\n");
		GMT->current.map.frame.draw = false;
	}

	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void gmt_terminate_clipping (struct GMT_CTRL *C, struct PSL_CTRL *PSL, int n)
{
	switch (n) {
		case PSL_ALL_CLIP:
			PSL_endclipping (PSL, n);	/* Reduce clipping to none */
			GMT_Report (C->parent, GMT_MSG_VERBOSE, "Restore ALL clip levels\n");
			break;
		default:
			PSL_endclipping (PSL, n);	/* Reduce clipping by n levels [1] */
			GMT_Report (C->parent, GMT_MSG_VERBOSE, "Restore %d clip levels\n", n);
			break;
	}
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psclip_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psclip (void *V_API, int mode, void *args)
{
	int error = 0;

	double x0, y0;

	struct PSCLIP_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psclip_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psclip_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psclip_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_psclip_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psclip_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the psclip main code ----------------------------*/

	if (Ctrl->C.active)
		GMT->current.ps.nclip = MIN (-1, -Ctrl->C.n);	/* Program terminates n levels of prior clipping */
	else
		GMT->current.ps.nclip = +1;		/* Program adds one new level of clipping */

	if (Ctrl->C.active && !GMT->current.map.frame.init) {
		PSL = GMT_plotinit (GMT, options);
		gmt_terminate_clipping (GMT, PSL, Ctrl->C.n);	/* Undo previous clip-path(s) */
		GMT_plotend (GMT);
		GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");
		Return (EXIT_SUCCESS);
	}

	/* Here we have -R -J etc to deal with */
	
	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	PSL = GMT_plotinit (GMT, options);
	if (Ctrl->C.active) gmt_terminate_clipping (GMT, PSL, Ctrl->C.n);	/* Undo previous clip-path(s) */
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */
	GMT_map_basemap (GMT);

	if (!Ctrl->C.active) {	/* Start new clip_path */
		unsigned int tbl;
		bool first = !Ctrl->N.active;
		uint64_t row, seg;
		double *x = NULL, *y = NULL;
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;

		GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
		if (Ctrl->N.active) GMT_map_clip_on (GMT, GMT->session.no_rgb, 1);	/* Must clip map */

		if (!Ctrl->T.active) {
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POLY, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {
				Return (API->error);	/* Register data input */
			}
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
				Return (API->error);
			}

			for (tbl = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
					S = D->table[tbl]->segment[seg];		/* Current segment */
					if (D->alloc_mode == GMT_ALLOCATED_EXTERNALLY) {	/* Cannot store results in the read-only input array */
						x = GMT_memory (GMT, NULL, S->n_rows, double);
						y = GMT_memory (GMT, NULL, S->n_rows, double);
					}
					else {	/* Reuse input arrays */
						x = S->coord[GMT_X];	y = S->coord[GMT_Y];	/* Short hand for x,y columns */
					}

					for (row = 0; row < S->n_rows; row++) {
						GMT_geo_to_xy (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], &x0, &y0);
						x[row] = x0; y[row] = y0;
					}
					PSL_beginclipping (PSL, x, y, (int)S->n_rows, GMT->session.no_rgb, first);
					first = 0;
					if (D->alloc_mode == GMT_ALLOCATED_EXTERNALLY) {	/* Free temp arrays */
						GMT_free (GMT, x);	GMT_free (GMT, y);
					}
				}
			}
			if (GMT_Destroy_Data (API, &D) != GMT_OK) {
				Return (API->error);
			}
		}

		/* Finalize the composite polygon clip path */
		PSL_beginclipping (PSL, NULL, NULL, 0, GMT->session.no_rgb, 2 + first);
	}

	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

	Return (GMT_OK);
}
