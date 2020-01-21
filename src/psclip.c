/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: psclip reads one or many xy-files and draws polygons just like psclip
 * with the exception that these polygons are set up to act as clip
 * paths for subsequent plotting.  psclip uses the even-odd winding
 * rule to decide what's inside and outside the path.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psclip"
#define THIS_MODULE_MODERN_NAME	"clip"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Initialize or terminate polygonal clip paths"
#define THIS_MODULE_KEYS	"<D{,>X},C-("
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdefghipqstxy" GMT_OPT("EZMmc")

struct PSCLIP_CTRL {
	struct A {	/* -A[m|p|step] */
		bool active;
		unsigned int mode;
		double step;
	} A;
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
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCLIP_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSCLIP_CTRL);
	C->C.n = 1;	/* Default undoes one level of clipping */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSCLIP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psclip synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -C[a|<n>] [-A[m|p|x|y]] [-K] [-O]  OR\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t%s <table> %s %s [%s]\n", name, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s[-N] %s%s[-T] [%s] [%s] [-W[<pen>]\n", API->K_OPT, API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] %s[%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, API->c_OPT, GMT_di_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-C Undo existing clip-paths; no file is needed and -R, -J are not required (unless -B is used).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Terminates all clipping; optionally append how many clip levels to restore [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more polygon files.  If none, standard input is read.\n");
	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Suppress connecting geographic points using great circle arcs, i.e., connect by straight lines,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   unless m or p is appended to first follow meridian then parallel, or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For Cartesian data, use -Ax or -Ay to connect first in x, then y, or vice versa.\n");
	GMT_Option (API, "<,B-,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Use the outside of the polygons and the map boundary as clip paths.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set clip path for the entire map frame.  No input file is required.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Draw clip path with given pen attributes [Default is no line].", 0);
	GMT_Option (API, "X,bi2,c,di,e,f,g,h,i,p,qi,t,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSCLIP_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'm': case 'y': Ctrl->A.mode = GMT_STAIRS_Y; break;
					case 'p': case 'x': Ctrl->A.mode = GMT_STAIRS_X; break;
#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature */
#endif
				}
				break;
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
			case 'W':		/* Set line attributes */
				Ctrl->W.active = true;
				if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 11);
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->C.active) {
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.active, "Syntax error: Cannot use -W with -C\n");
	}
	else {
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	}
	if (Ctrl->T.active) Ctrl->N.active = true;	/* -T implies -N */
	if (Ctrl->T.active && n_files) GMT_Report (API, GMT_MSG_VERBOSE, "Option -T ignores all input files\n");

	if (Ctrl->N.active && GMT->current.map.frame.init) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Option -B cannot be used in combination with Options -N or -T. -B is ignored.\n");
		GMT->current.map.frame.draw = false;
	}

	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void gmt_terminate_clipping (struct GMT_CTRL *C, struct PSL_CTRL *PSL, int n) {
	switch (n) {
		case PSL_ALL_CLIP:
			PSL_endclipping (PSL, n);	/* Reduce clipping to none */
			GMT_Report (C->parent, GMT_MSG_LONG_VERBOSE, "Restore ALL clip levels\n");
			break;
		default:
			PSL_endclipping (PSL, n);	/* Reduce clipping by n levels [1] */
			GMT_Report (C->parent, GMT_MSG_LONG_VERBOSE, "Restore %d clip levels\n", n);
			break;
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_clip (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: clip\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psclip (V_API, mode, args);
}

int GMT_psclip (void *V_API, int mode, void *args) {
	int error = 0;

	double x0, y0;

	struct PSCLIP_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psclip main code ----------------------------*/

	if (Ctrl->C.active)
		GMT->current.ps.nclip = MIN (-1, -Ctrl->C.n);	/* Program terminates n levels of prior clipping */
	else
		GMT->current.ps.nclip = +1;		/* Program adds one new level of clipping */

	if (Ctrl->C.active && !GMT->current.map.frame.init) {
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_terminate_clipping (GMT, PSL, Ctrl->C.n);	/* Undo previous clip-path(s) */
		gmt_plotend (GMT);
		Return (GMT_NOERROR);
	}

	/* Here we have -R -J etc to deal with */

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	if (Ctrl->C.active) gmt_terminate_clipping (GMT, PSL, Ctrl->C.n);	/* Undo previous clip-path(s) */
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);

	if (!Ctrl->C.active) {	/* Start new clip_path */
		unsigned int tbl, first = (Ctrl->N.active) ? 0 : 1, eo_flag = 0;
		bool duplicate;
		uint64_t row, seg, n_new;
		struct GMT_FILL no_fill;
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;

		gmt_set_line_resampling (GMT, Ctrl->A.active, Ctrl->A.mode);	/* Possibly change line resampling mode */
		gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);
#ifdef DEBUG
		/* Change default step size (in degrees) used for interpolation of line segments along great circles (if requested) */
		if (Ctrl->A.active) Ctrl->A.step = Ctrl->A.step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;
#endif

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
		if (Ctrl->N.active) {	/* Must clip map */
			eo_flag = PSL_EO_CLIP;	/* Do odd/even clipping when we first lay down the map perimeter */
			gmt_map_clip_on (GMT, GMT->session.no_rgb, 1 + eo_flag);
		}
		if (!Ctrl->T.active) {
			struct GMT_DATASET_HIDDEN *DH = NULL;
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POLY, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
				Return (API->error);	/* Register data input */
			}
			if ((error = GMT_Set_Columns (API, GMT_IN, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
				/* We don't want trailing text because we may need to resample lines below */
				Return (API->error);
			}
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
				Return (API->error);
			}
			if (D->n_columns < 2) {
				GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)D->n_columns);
				Return (GMT_DIM_TOO_SMALL);
			}
			DH = gmt_get_DD_hidden (D);
			duplicate = (DH->alloc_mode == GMT_ALLOC_EXTERNALLY && GMT->current.map.path_mode == GMT_RESAMPLE_PATH);
			if (Ctrl->W.active) {
				gmt_setpen (GMT, &Ctrl->W.pen);
				gmt_setfill (GMT, &no_fill, 1);
			}
			
			for (tbl = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
					S = D->table[tbl]->segment[seg];	/* Shortcut to current segment */
					if (duplicate) {	/* Must duplicate externally allocated segment since it needs to be resampled below */
						GMT_Report (API, GMT_MSG_DEBUG, "Must duplicate external memory polygon\n");
						S = gmt_duplicate_segment (GMT, D->table[tbl]->segment[seg]);
					}
					if (GMT->current.map.path_mode == GMT_RESAMPLE_PATH) {	/* Resample if spacing is too coarse or stair-case is requested */
						if ((n_new = gmt_fix_up_path (GMT, &S->data[GMT_X], &S->data[GMT_Y], S->n_rows, Ctrl->A.step, Ctrl->A.mode)) == 0) {
							Return (GMT_RUNTIME_ERROR);
						}
						S->n_rows = n_new;
						gmt_set_seg_minmax (GMT, D->geometry, 2, S);	/* Update min/max of x/y only */
						GMT_Report (API, GMT_MSG_DEBUG, "Resample polygon, now has %d points\n", S->n_rows);
					}
					if (Ctrl->W.active)
						gmt_geo_polygons (GMT, S);

					for (row = 0; row < S->n_rows; row++) {	/* Apply map projection */
						gmt_geo_to_xy (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], &x0, &y0);
						S->data[GMT_X][row] = x0; S->data[GMT_Y][row] = y0;
					}
					PSL_beginclipping (PSL, S->data[GMT_X], S->data[GMT_Y], (int)S->n_rows, GMT->session.no_rgb, first);
					first = 0;
					if (duplicate)	/* Free duplicate segment */
						gmt_free_segment (GMT, &S);
				}
			}
			if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
				Return (API->error);
			}
		}

		/* Finalize the composite polygon clip path */
		PSL_beginclipping (PSL, NULL, NULL, 0, GMT->session.no_rgb, 2 + first + eo_flag);
	}

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
