/*--------------------------------------------------------------------
 *    $Id$
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
 * API functions to support the gmtaverage application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2011
 * Version:	5 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out one (or no)
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.  Choose value from mean, median, mode,
 * number of points, datasum, weightsum, or a specified quantile q.
 */

#include "gmt.h"

struct GMTAVERAGE_CTRL {	/* All control options for this program (except common args) */
	struct E {	/* -E[b] */
		GMT_LONG active;
		GMT_LONG mode;
	} E;
	struct T {	/* -T<quantile> */
		GMT_LONG active;
		GMT_LONG median;
		double quantile;
	} T;
};

void * New_gmtaverage_Ctrl (struct GMT_CTRL *G) {	/* Allocate and initialize a new control structure */
	struct GMTAVERAGE_CTRL *C;
	
	C = GMT_memory (G, NULL, 1, struct  GMTAVERAGE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	return (C);
}

void Free_gmtaverage_Ctrl (struct GMT_CTRL *G, struct  GMTAVERAGE_CTRL *C) {	/* Deallocate control structure */
	GMT_free (G, C);	
}

GMT_LONG GMT_gmtaverage_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtaverage %s [API] - Block average (x, y, z) data tables by mean, median, or mode estimation\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtaverage [<table>] %s -Te|m|n|o|s|w|<q>\n", GMT_I_OPT);
	GMT_message (GMT, "\t%s [-C] [-E[b]] [-Q] [%s] [-W[i][o]]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t-T Select what value you wish to report per block:\n");
	GMT_message (GMT, "\t   e reports median values.\n");
	GMT_message (GMT, "\t   m reports mean values.\n");
	GMT_message (GMT, "\t   n reports number of data points.\n");
	GMT_message (GMT, "\t   o reports modal values.\n");
	GMT_message (GMT, "\t   s reports data sums.\n");
	GMT_message (GMT, "\t   w reports weight sums.\n");
	GMT_message (GMT, "\t   <q> reports the chosen quantile (0 < q < 1).\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-C Output center of block as location [Default is mean|median|mode of x and y, but see -Q].\n");
	GMT_message (GMT, "\t-E Extend output with scale (s), low (l), and high (h) value per block, i.e.,\n");
	GMT_message (GMT, "\t   output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w]); see -W regarding w.\n");
	GMT_message (GMT, "\t   Here, scale is standard deviation, L1 scale, or LMS scale depending on -T.\n");
	GMT_message (GMT, "\t   For -Te|<q>: Use -Eb for box-and-whisker output (x,y,z,l,25%%q,75%%q,h[,w])\n");
	GMT_message (GMT, "\t-Q Quicker; get median|mode z and x, y at that z [Default gets median|mode of x, y, and z.].\n");
	GMT_message (GMT, "\t   This option is ignored for -Tm|n|s|w.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Set Weight options.\n");
	GMT_message (GMT, "\t   -Wi reads Weighted Input (4 cols: x,y,z,w) but skips w on output.\n");
	GMT_message (GMT, "\t   -Wo reads unWeighted Input (3 cols: x,y,z) but writes weight sum on output.\n");
	GMT_message (GMT, "\t   -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_explain_options (GMT, "C0");
	GMT_message (GMT, "\t   Default is 3 columns (or 4 if -W is set).\n");
	GMT_explain_options (GMT, "D0fhioF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtaverage_parse (struct GMTAPI_CTRL *C, struct GMTAVERAGE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtaverage and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			/* Skip options that will be handled by the GMT_block* functions later */

			case '<':	/* Skip input files */
			case 'C':	/* Report center of block instead */
			case 'F':	/* Select pixel registration [gridline] */
			case 'I':	/* Get block dimensions */
			case 'Q':	/* Quick mode for median|mode z */
			case 'W':	/* Use in|out weights */
				break;
				
			/* Processes gmtaverage-specific parameters */

			case 'E':	/* Report extended statistics, where blockmedian has an extra modifier */
				Ctrl->E.active = TRUE;
				if (opt->arg[0] == 'b') Ctrl->E.mode = 1;
				break;	
			case 'T':	/* Select a particular output value operator */
				Ctrl->T.active = TRUE;		
				switch (opt->arg[0]) {
					case 'e':	/* Report medians [blockmedian] */
						Ctrl->T.median = TRUE;
						break;
					case 'm':	/* Report means [blockmean] */
					case 'n':	/* Report number of points [blockmean] */
					case 'o':	/* Report mode [blockmode] */
					case 's':	/* Report data sums [blockmean] */
					case 'w':	/* Report weight sums [blockmean] */
						break;
					case '0':	/* Look for a number in 0 <= q <= 1 range, e.g. 0.xxx, .xxx, or 1 [blockmedian] */
					case '1':
					case '.':
						Ctrl->T.quantile = atof (opt->arg);
						Ctrl->T.median = TRUE;
						break;
					default:
						Ctrl->T.median = 2;	/* Flag bad modifier */
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, !Ctrl->T.active, "Syntax error: Must specify -T option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.median == 2, "Syntax error: Bad modifier in -T option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.quantile < 0.0 || Ctrl->T.quantile >= 1.0,
			"Syntax error: 0 < q < 1 for quantile in -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode && !Ctrl->T.median, "Syntax error: -Eb requires -Te|<q>\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtaverage_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gmtaverage (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE;

	struct GMT_OPTION *options = NULL;
	struct GMTAVERAGE_CTRL *Ctrl = NULL;
	struct GMT_OPTION *t_ptr = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	PFL func = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmtaverage_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmtaverage_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtaverage", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRbf:", "aghior>" GMT_OPT("H"), options)) Return (API->error);
	Ctrl = New_gmtaverage_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtaverage_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtaverage main code ----------------------------*/

	/* Determine which value to report and use that to select correct GMT module */
	
	t_ptr = GMT_Find_Option (API, 'T', options);	/* Find the required -T option */
	
	switch (t_ptr->arg[0]) {	/* Determine what GMT_block* module we need */
		case 'm': case 'n': case 's': case 'w':	/* Call blockmean */
			t_ptr->option = 'S';	/* Since blockmean uses -S, not -T to select type */
			func = GMT_blockmean;
			break;
		case 'e':	/* Call blockmedian */
			GMT_Delete_Option (API, t_ptr);	/* Since -Te = -T0.5 is the default */
			func = GMT_blockmedian;
			break;
		case 'o':	/* Call blockmode */
			GMT_Delete_Option (API, t_ptr);	/* Since no special option -T is known to blockmode */
			func = GMT_blockmode;
			break;
		default:	/* Call blockmedian for some arbitrary quantile by passing the given -T<q> */
			func = GMT_blockmedian;
			break;
	}
	
	error = func (API, options);	/* If errors then we return that next */
	Return (error);
}
