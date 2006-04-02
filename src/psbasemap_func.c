/*--------------------------------------------------------------------
 *	$Id: psbasemap_func.c,v 1.1 2006-04-02 08:48:45 pwessel Exp $
 *
 *	Copyright (c) 1991-2006 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * API functions to support the psbasemap application.
 *
 * Author:	Paul Wessel
 * Date:	28-MAR-2006
 * Version:	5
 *
 */

#include "psbasemap.h"

int GMT_psbasemap_cmd (struct GMTAPI_CTRL *API, int n_args, char *args[])
{
	/* This is a front end that provides a text-command interface to psbasemap.  It is used
	 * by the FORTRAN interface and the stand-alone application tools to prepare for the
	 * execution via the GMT API version of psbasemap.
	 * Arguments:
	 * API:		Pointer to the GMT API control structure for the current session
	 * n_args:	Number of text arguments in args
	 * args:	Array of text arguments to the program
	 */

	int error;
	struct GMT_OPTION *options = NULL;
	
	/* Parse the command line text arguments and receive linked list of options */
	
	if ((error = GMTAPI_Parse_Args (n_args, args, &options))) return (error);
	
	/* Call the GMT program via the API */
	
	error = GMT_psbasemap (API, options);
	return (error);
}

int GMT_psbasemap (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	/* API:		Pointer to the GMT API control structure for the current session */
	/* options:	Linked list of options to use with this program */
	
	if (API == NULL) return (GMTAPI_NOT_A_SESSION);
	
	if (options == NULL) {	/* Give full program usage */
		psbasemap_usage (API, FALSE);
		return (GMTAPI_OK);
	}

	if (options->option == '\0') {	/* Give program synopsis only */
		psbasemap_usage (API, TRUE);
		return (GMTAPI_OK);
	}
	
	/* Run the program function */
	
	if (psbasemap_function (API, options)) return (GMTAPI_RUNTIME_ERROR);
	
	return (GMTAPI_OK);	/* No worries! */
}

void psbasemap_usage (struct GMTAPI_CTRL *API, BOOLEAN synopsis_only)
{
	/* This displays the psbasemap synopsis and optionally full usage information */

	fprintf (stderr,"psbasemap %s [API] - To plot PostScript basemaps\n\n", GMT_VERSION);
	fprintf (stderr, "usage: psbasemap -B<tickinfo> -J<params> -R<west/east/south/north> [-Eaz/el] [-G<fill>] [-K]\n");
	fprintf (stderr, "\t[-L[f][x]<lon0>/<lat0>[/<slon>]/<slat>/<length>[m|n|k][:label:<just>][+p<pen>][+f<fill>]]\n");
	fprintf (stderr, "\t[-O] [-P] [-T[f|m][x]<lon0>/<lat0>/<size>[/<info>][:w,e,s,n:][+<gints>[/<mints>]]]\n");
	fprintf (stderr, "\t[-U[dx/dy/][label]] [-V] [-X<xshift>] [-Y<yshift>] [-Z<zlevel>] [-c<ncopies>]\n\n");

	if (synopsis_only) return;

	GMT_explain_option ('B');
	GMT_explain_option ('J');
	GMT_explain_option ('R');
	fprintf (stderr, "\n\tOPTIONS:\n");
	fprintf (stderr, "\t-E set azimuth and elevation of viewpoint for 3-D perspective [180/90]\n");
	fprintf (stderr, "\t-G fill inside of basemap with the specified fill\n");
	GMT_explain_option ('K');
	fprintf (stderr, "\t-L draws a simple map scaLe centered on <lon0>/<lat0>.  Use -Lx to specify Cartesian\n");
	fprintf (stderr, "\t   coordinates instead.  Scale is calculated at latitude <slat>; optionally give longitude\n");
	fprintf (stderr, "\t   <slon> [Default is central longitude].  <length> is in km, or [nautical] miles if [n] m\n");
	fprintf (stderr, "\t   is appended.  -Lf draws a \"fancy\" scale [Default is plain]. By default, the label is set\n");
	fprintf (stderr, "\t   to the distance unit and placed on top [t].  Use the :label:<just> mechanism to specify another\n");
	fprintf (stderr, "\t   label (or - to keep the default) and placement (t,b,l,r, and u - to use the label as a unit.\n");
	fprintf (stderr, "\t   Append +p<pen> and/or +f<fill> to draw/paint a rectangle beneath the scale [no rectangle]\n");
	GMT_explain_option ('O');
	GMT_explain_option ('P');
	fprintf (stderr, "\t-T draws a north-poinTing rose centered on <lon0>/<lat0>.  Use -Tx to specify Cartesian\n");
	fprintf (stderr, "\t   coordinates instead.  -Tf draws a \"fancy\" rose [Default is plain].  Give rose diameter\n");
	fprintf (stderr, "\t   <size> and optionally the west, east, south, north labels desired [W,E,S,N].\n");
	fprintf (stderr, "\t   For fancy rose, specify as <info> the kind you want: 1 draws E-W, N-S directions [Default],\n");
	fprintf (stderr, "\t   2 adds NW-SE and NE-SW, while 3 adds WNW-ESE, NNW-SSE, NNE-SSW, and ENE-WSW.\n");
	fprintf (stderr, "\t   For Magnetic compass rose, specify -Tm.  Use the optional <info> = <dec>/<dlabel> (where <dec> is\n");
	fprintf (stderr, "\t   the magnetic declination and <dlabel> is a label for the magnetic compass needle) to plot\n");
	fprintf (stderr, "\t   directions to both magnetic and geographic north [Default is just geographic].\n");
	fprintf (stderr, "\t   If the North label = \'*\' then a north star is plotted instead of the label.\n");
	fprintf (stderr, "\t   Append +<gints>/<mints> to override default annotation/tick interval(s) [10/5/1/30/5/1].\n");
	GMT_explain_option ('U');
	GMT_explain_option ('V');
	GMT_explain_option ('X');
	fprintf (stderr, "\t-Z For 3-D plots: Set the z-level of map [0]\n");
	GMT_explain_option ('c');
	GMT_explain_option ('.');
}

int psbasemap_parse (struct GMTAPI_CTRL *API, struct PSBASEMAP_CTRL *CTRL, struct GMT_OPTION *options)
{
	/* This parses the options provided to psbasemap and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n_errors = 0;
	struct GMT_OPTION *opt;
	
	memset ((void *)CTRL, 0, sizeof (struct PSBASEMAP_CTRL));	/* Zero the control */
	
	GMT_init_fill (&CTRL->fill, -1, -1, -1);	/* Then set non-zero default values */

	for (opt = options; opt; opt = opt->next) {
	
		switch (opt->option) {

			/* Common parameters */

			case 'B':
			case 'J':
			case 'K':
			case 'O':
			case 'P':
			case 'R':
			case 'U':
			case 'V':
			case 'X':
			case 'x':
			case 'Y':
			case 'y':
			case 'c':
			case '\0':
				n_errors += GMT_get_common_args (opt->arg, &CTRL->w, &CTRL->e, &CTRL->s, &CTRL->n);
				break;

			/* Supplemental options */

			case 'E':
				sscanf (opt->arg, "%lf/%lf", &z_project.view_azimuth, &z_project.view_elevation);
				break;

			case 'G':
				if (GMT_getfill (opt->arg, &CTRL->fill)) {
					GMT_fill_syntax ('G');
					n_errors++;
				}
				CTRL->paint = TRUE;
				break;

			case 'L':
				n_errors += GMT_getscale (opt->arg, &CTRL->ms);
				break;

			case 'T':
				n_errors += GMT_getrose (opt->arg, &CTRL->mr);
				break;

			case 'Z':
				if (opt->arg) {
					CTRL->new_z_level = atof (opt->arg);
					CTRL->set_z = TRUE;
				}
				break;

			/* Illegal options */

			default:
				n_errors++;
				GMT_default_error (opt->option);
				break;
		}
	}

	if (!project_info.region_supplied) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR:  Must specify -R option\n", GMT_program);
		n_errors++;
	}
	if (!(frame_info.plot || CTRL->ms.plot || CTRL->mr.plot || CTRL->paint)) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR:  Must specify at least one of -B, -G, -L, -T\n", GMT_program);
		n_errors++;
	}
	if (z_project.view_elevation <= 0.0 || z_project.view_elevation > 90.0) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -E option:  Enter elevation in 0-90 range\n", GMT_program);
		n_errors++;
	}

	return (n_errors);
}

int psbasemap_function (struct GMTAPI_CTRL *API, struct GMT_OPTION *head)
{
	struct PSBASEMAP_CTRL CTRL;	/* Control structure specific to program */
	int argc;
	char **argv;
	
	/* This is the psbasemap main code */
	
	/* Parse the program-specific arguments */
	
	if (psbasemap_parse (API, &CTRL, head)) return (GMTAPI_PARSE_ERROR);
	
	argc = GMTAPI_Generate_Args (&argv, head);	/* For now, needed by ecfho_command and history */
	
	GMT_put_history (argc, argv);	/* Update .gmtcommands4 or pass via API */

	if (gmtdefs.verbose) fprintf (stderr, "psbasemap: Constructing basemap\n");

	GMT_map_setup (CTRL.w, CTRL.e, CTRL.s, CTRL.n);

	ps_plotinit (CNULL, gmtdefs.overlay, gmtdefs.page_orientation, gmtdefs.x_origin, gmtdefs.y_origin,
		gmtdefs.global_x_scale, gmtdefs.global_y_scale, gmtdefs.n_copies,
		gmtdefs.dpi, GMT_INCH, gmtdefs.paper_width, gmtdefs.page_rgb, gmtdefs.encoding.name, GMT_epsinfo (argv[0]));

	GMT_echo_command (argc, argv);
	if (gmtdefs.unix_time) GMT_timestamp (argc, argv);

	if (project_info.three_D) ps_transrotate (-z_project.xmin, -z_project.ymin, 0.0);

	if (CTRL.paint) {
		double *x, *y;
		int np, donut;
		np = GMT_map_clip_path (&x, &y, &donut);
		GMT_fill (x, y, (1 + donut) * np, &CTRL.fill, FALSE);
		GMT_free ((void *)x);
		GMT_free ((void *)y);
	}

	if (CTRL.set_z) project_info.z_level = CTRL.new_z_level;

	GMT_map_basemap ();

	if (CTRL.ms.plot) GMT_draw_map_scale (&CTRL.ms);

	if (CTRL.mr.plot) GMT_draw_map_rose (&CTRL.mr);

	if (project_info.three_D) ps_rotatetrans (z_project.xmin, z_project.ymin, 0.0);
	ps_plotend (gmtdefs.last_page);

	GMTAPI_Free_Args (argc, argv);

	return (GMTAPI_OK);
}
