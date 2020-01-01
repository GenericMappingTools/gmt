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
 * Author:	Paul Wessel
 * Date:	10-Oct-2018
 * Version:	6 API
 *
 * Brief synopsis: gmt inset determines dimensions and offsets for a figure inset.
 * It has two modes of operation:
 *	1) Initialize a new inset, which determines dimensions and sets parameters:
 *	   gmt inset begin -D<params> [-F<panel>] [-M<margins>] [-N] [-V]
 *	   Sugsequent plot calls will be placed in the inset window.
 *	2) Exit inset mode, which resets plot origin and previous -R -J parameters:
 *	   gmt inset end [-V]
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"inset"
#define THIS_MODULE_MODERN_NAME	"inset"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Manage figure inset setup and completion"
#define THIS_MODULE_KEYS	">X}"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS	"JRVXY"

/* Control structure for inset */

#define INSET_CM_TO_INCH 	(1.0/2.54)
#define INSET_BEGIN		1
#define INSET_END		0

struct INSET_CTRL {
	struct In {	/* begin | end */
		bool active;
		unsigned int mode;	/* INSET_BEGIN|END*/
	} In;
	struct D {	/* -D[g|j|n|x]<refpoint>+w<width>[<unit>][/<height>[<unit>]][+j<justify>[+o<dx>[/<dy>]][+t] or <xmin>/<xmax>/<ymin>/<ymax>[+r][+t][+u] */
		bool active;
		struct GMT_MAP_INSET inset;
	} D;
	struct F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		/* The panel is a member of GMT_MAP_INSET */
	} F;
	struct M {	/* -M<margin>[u] | <xmargin>[u]/<ymargin>[u]  | <wmargin>[u]/<emargin>[u]/<smargin>[u]/<nmargin>[u]  */
		bool active;
		double margin[4];
	} M;
	struct N {	/* -N  (no clip) */
		bool active;
	} N;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct INSET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct INSET_CTRL);
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct INSET_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	if (C->D.inset.refpoint) gmt_free_refpoint (GMT, &C->D.inset.refpoint);
	gmt_M_free (GMT, C->D.inset.panel);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s begin -D%s |\n\t-D%s\n", name, GMT_INSET_A, GMT_INSET_B);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F%s] [-M<margins>] [-N] [%s] [%s]\n\n", GMT_PANEL, GMT_V_OPT, GMT_PAR_OPT);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s end [%s]\n\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	gmt_mapinset_syntax (API->GMT, 'D', "Design a simple map inset as specified below:");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the map inset.", 3);
	GMT_Message (API, GMT_TIME_NONE, "\t-M Allows for space around the inset. Append a uniform <margin>, separate <xmargin>/<ymargin>,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side [no margin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not clip anything that exceeds the map inset boundaries [Default will clip].\n");
	GMT_Option (API, "V,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct INSET_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to inset and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	struct GMT_OPTION *opt = NULL;

	opt = options;	/* The first argument is the inset command */
	if (opt->option != GMT_OPT_INFILE) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No inset command given\n");
		return GMT_PARSE_ERROR;
	}
	if (!strncmp (opt->arg, "begin", 5U)) {	/* Initiate new inset */
		Ctrl->In.mode = INSET_BEGIN;
	}
	else if (!strncmp (opt->arg, "end", 3U))
		Ctrl->In.mode = INSET_END;
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Not an inset command: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	opt = opt->next;	/* Position to the next argument */

	while (opt) {	/* Process all the options given */

		switch (opt->option) {

			case 'D':	/* Draw map inset */
				Ctrl->D.active = true;
				n_errors += gmt_getinset (GMT, 'D', opt->arg, &Ctrl->D.inset);
				break;

			case 'F':
				Ctrl->F.active = true;
				if (gmt_getpanel (GMT, opt->option, opt->arg, &(Ctrl->D.inset.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel for the map inset", 3);
					n_errors++;
				}
				break;
	
			case 'M':	/* inset margins */
				Ctrl->M.active = true;
				if (opt->arg[0] == 0) {	/* Gave nothing */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -M: No margins given.\n");
					n_errors++;
				}
				else {	/* Process 1, 2, or 4 margin values */
					k = GMT_Get_Values (GMT->parent, opt->arg, Ctrl->M.margin, 4);
					if (k == 1)	/* Same page margin in all directions */
						Ctrl->M.margin[XHI] = Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = Ctrl->M.margin[XLO];
					else if (k == 2) {	/* Separate page margins in x and y */
						Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = Ctrl->M.margin[XHI];
						Ctrl->M.margin[XHI] = Ctrl->M.margin[XLO];
					}
					else if (k != 4) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -M: Bad number of margins given.\n");
						n_errors++;
					}
					/* Since GMT_Get_Values returns in default project length unit, convert to inch */
					for (k = 0; k < 4; k++) Ctrl->M.margin[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				}
				break;

			case 'N':	/* Turn off clipping  */
				Ctrl->N.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
		opt = opt->next;
	}
	
	if (Ctrl->In.mode == INSET_BEGIN) {
		/* Was -R -J given */
		n_errors += gmt_M_check_condition (GMT, GMT->common.J.active && !GMT->common.R.active[RSET], "Syntax error -J: Requires -R as well!\n");
		if (GMT->common.J.active) {	/* Compute map height */
			if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) n_errors++;
		}
	}
	else {	/* gmt inset end was given, when -D -F -M -N are not allowed */
		if (Ctrl->D.active) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -D: Not valid for gmt inset end.\n");
			n_errors++;
		}
		if (Ctrl->F.active) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -F: Not valid for gmt inset end.\n");
			n_errors++;
		}
		if (Ctrl->M.active) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -M: Not valid for gmt inset end.\n");
			n_errors++;
		}
		if (Ctrl->N.active) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -N: Not valid for gmt inset end.\n");
			n_errors++;
		}
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_inset (void *V_API, int mode, void *args) {
	int error = 0, fig, k;
	bool exist;
	char file[PATH_MAX] = {""}, ffile[PATH_MAX] = {""}, Bopts[GMT_LEN256] = {""};
	FILE *fp = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMT_OPTION *options = NULL;
	struct INSET_CTRL *Ctrl = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "module inset is not available in classic mode\n");
		bailout (GMT_NOT_MODERN_MODE);
	}

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the inset main code ----------------------------*/

	fig = gmt_get_current_figure (API);	/* Get current figure number */

	if ((k = gmt_set_psfilename (GMT)) == GMT_NOTSET) {	/* Get hidden file name for PS */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No active plot file to append to\n");
		Return (GMT_ERROR_ON_FOPEN);
	}

	sprintf (file, "%s/gmt.inset.%d", API->gwf_dir, fig);	/* Inset information file */

	exist = !access (file, F_OK);	/* Determine if inset information file exists */
	if (Ctrl->In.mode == INSET_BEGIN && exist) {	/* Inset information file already exists which is a failure */
		GMT_Report (API, GMT_MSG_NORMAL, "In begin mode but inset information file already exists: %s\n", file);
		Return (GMT_RUNTIME_ERROR);
	}
	else if (Ctrl->In.mode == INSET_END && !exist) {	/* Inset information file does not exist which is a failure */
		GMT_Report (API, GMT_MSG_NORMAL, "In end mode and information file does not exist: %s\n", file);
		Return (GMT_RUNTIME_ERROR);
	}

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	if (Ctrl->In.mode == INSET_BEGIN) {	/* Determine and save inset attributes */
		/* Here we need to compute dimensions and save those plus current -R -J to the inset information file,
		 * then inset a gsave command, translate origin to the inset, adjust for any margins, compute new scales/widths and maybe
		 * draw the panel. */
		
		char *cmd = NULL;

		/* OK, no other inset set for this figure (or panel).  Save graphics state before we draw the inset */
		PSL_command (PSL, "V %% Begin inset\n");

		gmt_draw_map_inset (GMT, &Ctrl->D.inset, !Ctrl->N.active);	/* Draw the inset background */

		/* Set the new origin as indicated */
		
		PSL_setorigin (PSL, Ctrl->D.inset.refpoint->x + Ctrl->M.margin[XLO], Ctrl->D.inset.refpoint->y + Ctrl->M.margin[YLO], 0.0, PSL_FWD);	/* Shift plot a bit */
		
		/* First get the -B options in place before inset was called */
		sprintf (ffile, "%s/gmt%d.%s/gmt.frame", API->session_dir, GMT_MAJOR_VERSION, API->session_name);
		if ((fp = fopen (ffile, "r")) == NULL)
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "No file %s with frame information - no adjustments made\n", ffile);
		fgets (Bopts, PATH_MAX, fp);
		gmt_chop (Bopts);
		fclose (fp);
		
		/* Write out the inset information file */
		
		if ((fp = fopen (file, "w")) == NULL) {	/* Not good */
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot create inset file %s\n", file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		fprintf (fp, "# Figure inset information file\n");
		cmd = GMT_Create_Cmd (API, options);
		fprintf (fp, "# Command: %s %s\n", THIS_MODULE_CLASSIC_NAME, cmd);
		gmt_M_free (GMT, cmd);
		fprintf (fp, "# ORIGIN: %g %g\n", Ctrl->D.inset.refpoint->x, Ctrl->D.inset.refpoint->y);
		fprintf (fp, "# DIMENSION: %g %g\n", Ctrl->D.inset.dim[GMT_X], Ctrl->D.inset.dim[GMT_Y]);
		fprintf (fp, "# MARGINS: %g %g %g %g\n", Ctrl->M.margin[XLO], Ctrl->M.margin[XHI], Ctrl->M.margin[YLO], Ctrl->M.margin[YHI]);
		fprintf (fp, "# REGION: %s\n", GMT->common.R.string);
		fprintf (fp, "# PROJ: %s\n", GMT->common.J.string);
		fprintf (fp, "# FRAME: %s\n", Bopts);
		fclose (fp);
		GMT_Report (API, GMT_MSG_DEBUG, "inset: Wrote inset settings to information file %s\n", file);
		gmt_reset_history (GMT);	/* Prevent gmt from copying previous -R -J history to this inset */
	}
	else {	/* INSET_END */
		/* Here we need to finish the inset with a grestore and restate the original -R -J in the history file,
		 * and finally remove the inset information file */

		char tag[GMT_LEN16] = {""}, legend_justification[4] = {""};
		double legend_width = 0.0, legend_scale = 1.0;

		if (gmt_get_legend_info (API, &legend_width, &legend_scale, legend_justification)) {	/* Unplaced legend file */
			char cmd[GMT_LEN64] = {""};
			/* Default to white legend with 1p frame offset 0.2 cm from selected justification point [TR] */
			snprintf (cmd, GMT_LEN64, "-Dj%s+w%gi+o0.2c -F+p1p+gwhite -S%g", legend_justification, legend_width, legend_scale);
			if ((error = GMT_Call_Module (API, "legend", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Failed to place legend on current inset figure\n");
				Return (error);
			}
		}

		PSL_command (PSL, "PSL_inset_clip 1 eq {cliprestore /PSL_inset_clip 0 def} if\n");	/* Restore graphics state to what it was before the map inset */
		PSL_command (PSL, "U %% End inset\n");	/* Restore graphics state to what it was before the map inset */

		/* Remove the inset history file */
		gmt_history_tag (API, tag);
		sprintf (ffile, "%s/%s.%s", API->gwf_dir, GMT_HISTORY_FILE, tag);
		gmt_remove_file (GMT, ffile);
		/* Restore the old frame B setting to what it was before inset begin was called, if any */
		if ((fp = fopen (file, "r"))) {	/* There is a gmt.frame file */
			while (fgets (Bopts, PATH_MAX, fp) && strncmp (Bopts, "# FRAME: ", 9U));	/* Wind to reading the frame setting */
			gmt_chop (Bopts);
			fclose (fp);	/* Done reading the gmt.frame file */
			if (!strncmp (Bopts, "# FRAME: ", 9U) && strlen (Bopts) > 9 && Bopts[9]) {	/* Got a previously saved -B frame setting */
				sprintf (ffile, "%s/gmt%d.%s/gmt.frame", API->session_dir, GMT_MAJOR_VERSION, API->session_name);
				if ((fp = fopen (ffile, "w")) == NULL) {	/* Not good */
					GMT_Report (API, GMT_MSG_NORMAL, "Cannot create frame file %s\n", ffile);
					Return (GMT_ERROR_ON_FOPEN);
				}
				GMT_Report (API, GMT_MSG_DEBUG, "inset: Restore previous frame in %s\n", ffile);
				/* Restore the previous frame setting in gmt.frame */
				fprintf (fp, "%s\n", &Bopts[9]);
				fclose (fp);
			}
		}
		/* Remove the inset information file */
		gmt_remove_file (GMT, file);
		GMT_Report (API, GMT_MSG_DEBUG, "inset: Removed inset file\n");
		gmt_reload_history (API->GMT);
	}
	
	gmt_plotend (GMT);

	Return (error);
}
