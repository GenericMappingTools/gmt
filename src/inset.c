/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Date:	10-Oct-2018
 * Version:	6 API
 *
 * Brief synopsis: gmt inset determines dimensions and offsets for a figure inset.
 * It has two modes of operation:
 *	1) Initialize a new inset, which determines dimensions and sets parameters:
 *	   gmt inset begin -D<params> [-F<panel>] [-V]
 *	2) Exit inset mode:
 *	   gmt inset end [-V]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"inset"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Manage modern mode figure inset design and completion"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

/* Control structure for inset */

#define INSET_CM_TO_INCH 	(1.0/2.54)
#define INSET_BEGIN		1
#define INSET_END		0

struct INSET_CTRL {
	struct In {	/* begin | end */
		bool active;
		unsigned int mode;	/* INSET_BEGIN|END*/
	} In;
	struct D {	/* -D[g|j|n|x]<refpoint>+w<width>[<unit>][/<height>[<unit>]][+j<justify>[+o<dx>[/<dy>]][+s<file>][+t] or [<unit>]<xmin>/<xmax>/<ymin>/<ymax>[r][+s<file>][+t] */
		bool active;
		struct GMT_MAP_INSET inset;
	} D;
	struct F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		/* The panel is a member ofGMT_MAP_INSET */
	} F;
	struct M {	/* -M<margin>[u] | <xmargin>[u]/<ymargin>[u]  | <wmargin>[u]/<emargin>[u]/<smargin>[u]/<nmargin>[u]  */
		bool active;
		double margin[4];
	} M;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct INSET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct INSET_CTRL);
	for (unsigned int k = 0; k < 4; k++) C->M.margin[k] = 0.5 * GMT->session.u2u[GMT_CM][GMT_INCH];	/* 0.5 cm -> inches */
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
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s begin -D%s | -D%s\n", name, GMT_INSET_A, GMT_INSET_B);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-M<margins>] [%s] [%s]\n\n", GMT_PANEL, GMT_V_OPT, GMT_PAR_OPT);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s end [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	gmt_mapinset_syntax (API->GMT, 'D', "Design a simple map inset as specified below:");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the map inset.", 3);
	GMT_Message (API, GMT_TIME_NONE, "\t-M Allows for padding around the inset. Append a uniform <margin>, separate <xmargin>/<ymargin>,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side [0.5c].\n");
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
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Not a inset command: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	opt = opt->next;	/* Position to the next argument */
	if (Ctrl->In.mode == INSET_END && opt && !(opt->option == 'V' && opt->next == NULL)) {	/* Only -V is optional for end or set */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "inset end: Unrecognized option: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}

	while (opt) {	/* Process all the options given */

		switch (opt->option) {

			case 'D':	/* Draw map inset */
				Ctrl->D.active = true;
				n_errors += gmt_getinset (GMT, 'D', opt->arg, &Ctrl->D.inset);
				break;
			case 'F':
				Ctrl->F.active = true;
				if (gmt_getpanel (GMT, opt->option, &opt->arg[k], &(Ctrl->D.inset.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectanglar panel for the map inset", 3);
					n_errors++;
				}
				break;
	
			case 'M':	/* inset margins */
				Ctrl->M.active = true;
				if (opt->arg[0] == 0) {	/* Accept default margins */
					for (k = 0; k < 4; k++) Ctrl->M.margin[k] = 0.5 * INSET_CM_TO_INCH;
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
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_inset (void *V_API, int mode, void *args) {
	int error = 0, fig, k;
	char file[PATH_MAX] = {""};
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct INSET_CTRL *Ctrl = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Not available in classic mode\n");
		bailout (GMT_NOT_MODERN_MODE);
	}

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the inset main code ----------------------------*/

	GMT_Report (API, GMT_MSG_NORMAL, "THE INSET MODULE IS NOT YET OPERATIONAL\n");

	fig = gmt_get_current_figure (API);	/* Get current figure number */

	if ((k = gmt_set_psfilename (GMT)) == GMT_NOTSET) {	/* Get hidden file name for PS */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No workflow directory\n");
		Return (GMT_ERROR_ON_FOPEN);
	}
	if ((PSL_fopen (GMT->PSL, GMT->current.ps.filename, "a")) == NULL) {	/* Must open inside PSL DLL */
		GMT_Report (API, GMT_MSG_NORMAL, "Insert must be an overlay to an existing plot!\n");
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot open %s in append mode\n", GMT->current.ps.filename);
		Return (GMT_ERROR_ON_FOPEN);
	}
	
	sprintf (file, "%s/gmt.inset.%d", API->gwf_dir, fig);	/* Insert information file */

	if (Ctrl->In.mode == INSET_BEGIN) {	/* Determine and save inset attributes */
		/* Here we need to compute dimensions and save those plus current -R -J to the inset information file,
		 * then inset a gsave command, translate origin to the inset, adjust for any margins, compute new scales/widths and maybe
		 * draw the panel. */
		
		char *cmd = NULL;
		FILE *fpi = NULL;
		
		if (!access (file, F_OK))	{	/* Insert information file already exists which is failure */
			GMT_Report (API, GMT_MSG_NORMAL, "Insert information file already exists: %s\n", file);
			Return (GMT_RUNTIME_ERROR);
		}
		
		/* OK, no other inset set for this figure (or panel).  Save graphics state before we draw the inset */
		PSL_command (GMT->PSL, "V\n");

		gmt_draw_map_inset (GMT, &Ctrl->D.inset);	/* Draw the inset */
		
		/* Write out the inset information file */
		
		if ((fpi = fopen (file, "w")) == NULL) {	/* Not good */
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot create file %s\n", file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		fprintf (fpi, "# Figure inset information file\n");
		cmd = GMT_Create_Cmd (API, options);
		fprintf (fpi, "# Command: %s %s\n", THIS_MODULE_NAME, cmd);
		gmt_M_free (GMT, cmd);
		fprintf (fpi, "# ORIGIN: %g %g\n", 0.0, 0.0);
		fprintf (fpi, "# DIMENSION: %g %g\n", Ctrl->D.inset.dim[GMT_X], Ctrl->D.inset.dim[GMT_Y]);
		fprintf (fpi, "# REGION: %s\n", GMT->common.R.string);
		fprintf (fpi, "# PROJ: %s\n", GMT->common.J.string);
		fclose (fpi);
		GMT_Report (API, GMT_MSG_DEBUG, "inset: Wrote inset settings to information file %s\n", file);
		
	}
	else {	/* INSET_END */
		/* Here we need to finish the inset with a grestore and restate the original -R -J in the history file,
		 * and finally remove the inset information file */
		
		PSL_command (GMT->PSL, "U\n");	/* Restore graphics state to what it was before the map inset */
		/* Remove the inset information file */
		gmt_remove_file (GMT, file);
		GMT_Report (API, GMT_MSG_DEBUG, "inset: Removed inset file\n");
	}
	
	/* In either case we need to manually close the PS file */
	
	if (PSL_fclose (GMT->PSL)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to close hidden PS file %s!\n", GMT->current.ps.filename);
		Return (GMT_RUNTIME_ERROR);
	}

	Return (error);
}
