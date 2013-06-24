/*--------------------------------------------------------------------
 *    $Id$
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
 * Program to demonstrate use of GMT API to call one of the three spatial
 * data averageing modules GMT_blockmean|median|mode.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2013
 * Version:	5 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out one (or no)
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.  Choose value from mean, median, mode,
 * number of points, datasum, weightsum, or a specified quantile q.
 */

#include "gmt.h"	/* Must include this to use GMT API */
#include <string.h>

#define GMT_PROG_OPTIONS "-:>RVabfghior" "H"	/* The H is for possible compatibility with GMT4 syntax */

struct PROG_CTRL {	/* All local control options for this program (except common args) */
	struct E {	/* -E[b] */
		unsigned int active;
		unsigned int mode;
	} E;
	struct T {	/* -T<quantile> */
		unsigned int active;
		unsigned int median;
		double quantile;
	} T;
};

void * New_Prog_Ctrl () {	/* Allocate and initialize a new control structure */
	struct PROG_CTRL *C;
	
	C = calloc (1, sizeof (struct PROG_CTRL));
	
	/* Initialize values whose defaults are not 0/false/NULL */
	return (C);
}

void Free_Prog_Ctrl (struct PROG_CTRL *C) {	/* Deallocate control structure */
	free ((void *)C);	
}

int Gmtaverage_Usage (void *API, int level)
{
	GMT_Message (API, GMT_TIME_NONE, "gmtaverage - Block average (x,y,z) data tables by mean, median, or mode estimation\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtaverage [<table>] %s -Te|m|n|o|s|w|<q>\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-C] [-E[b]] [-Q] [%s] [-W[i][o]]\n\t[%s] [%s] [%s]\n\t[%s]\n\t[%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_R2_OPT, GMT_V_OPT, GMT_a_OPT, GMT_b_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Select what value you wish to report per block:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e reports median values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m reports mean values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n reports number of data points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   o reports modal values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s reports data sums.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   w reports weight sums.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <q> reports the chosen quantile (0 < q < 1).\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output center of block as location [Default is mean|median|mode of x and y, but see -Q].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend output with scale (s), low (l), and high (h) value per block, i.e.,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w])]; see -W regarding w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Here, scale is standard deviation, L1 scale, or LMS scale depending on -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For -Te|<q>: Use -Eb for box-and-whisker output (x,y,z,l,25%%q,75%%q,h[,w])\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Quicker; get median|mode z and x, y at that z [Default gets median|mode of x, y, and z.].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This option is ignored for -Tm|n|s|w.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set Weight options.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wi reads Weighted Input (4 cols: x,y,z,w) but skips w on output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wo reads unWeighted Input (3 cols: x,y,z) but writes weight sum on output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_Option (API, "a,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 columns (or 4 if -W is set).\n");
	GMT_Option (API, "bo,f,h,i,o,r,:,.");
	
	return (EXIT_FAILURE);
}

int Gmtaverage_Parse (void *API, struct PROG_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtaverage and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		if (strchr (GMT_PROG_OPTIONS, opt->option)) continue;	/* Common options already processed */
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
				Ctrl->E.active = 1;
				if (opt->arg[0] == 'b') Ctrl->E.mode = 1;
				break;	
			case 'T':	/* Select a particular output value operator */
				Ctrl->T.active = 1;		
				switch (opt->arg[0]) {
					case 'e':	/* Report medians [blockmedian] */
						Ctrl->T.median = 1;
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
						Ctrl->T.median = 1;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Bad modifier in -T option\n");
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Unrecognized argument %c%s\n", opt->option, opt->arg);
				n_errors++;
				break;
		}
	}
	
	if (Ctrl->T.active) n_errors += GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Must specify -T option\n");
	if (Ctrl->T.quantile < 0.0 || Ctrl->T.quantile >= 1.0) n_errors += GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: 0 < q < 1 for quantile in -T\n");
	if (Ctrl->E.mode && !Ctrl->T.median) n_errors += GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -Eb requires -Te|<q>\n");

	return (n_errors);
}

/* Must free allocated memory before returning */
#define Free_Options {if (GMT_Destroy_Options (API, &options) != GMT_NOERROR) exit (EXIT_FAILURE);}
#define Bailout(code) {Free_Options; exit (code);}
#define Exit(code) {Free_Prog_Ctrl (Ctrl); Bailout (code);}

int main (int argc, char *argv[])
{
	int error = 0;
	char *module = NULL;
	struct GMT_OPTION *options = NULL, *t_ptr = NULL;
	struct PROG_CTRL *Ctrl = NULL;
	void *API = NULL;

	/*---------------------------- This is the gmtaverage main code ----------------------------*/

        /* Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], 2U, 0U, NULL)) == NULL) Bailout (EXIT_FAILURE);
        /* Convert argc,argv to linked option list */
	options = GMT_Create_Options (API, argc-1, (argv+1));
        /* Handle special cases like usage and synopsis */
	if (!options || options->option == GMT_OPT_USAGE) Bailout (Gmtaverage_Usage (API, GMT_USAGE));	/* Exit the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) Bailout (Gmtaverage_Usage (API, GMT_SYNOPSIS));	/* Exit the synopsis */
	/* Parse the common command-line arguments */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Exit (EXIT_FAILURE);	/* Parse the common options */
	/* Parse the local command-line arguments */
	Ctrl = New_Prog_Ctrl ();							/* Allocate gmtaverage control structure */
	if ((error = Gmtaverage_Parse (API, Ctrl, options))) Exit (EXIT_FAILURE);	/* Parse local option arguments */

	/* Determine which value to report and use that to select the correct GMT module */
	
	t_ptr = GMT_Find_Option (API, 'T', options);	/* Find the required -T option */
	
	switch (t_ptr->arg[0]) {	/* Determine what GMT_block* module we need */
		case 'm': case 'n': case 's': case 'w':	/* Call blockmean */
			t_ptr->option = 'S';	/* Since blockmean uses -S, not -T to select type */
			module = "blockmean";
			break;
		case 'e':	/* Call blockmedian */
			GMT_Delete_Option (API, t_ptr);	/* Since -Te = -T0.5 is the default */
			module = "blockmedian";
			break;
		case 'o':	/* Call blockmode */
			GMT_Delete_Option (API, t_ptr);	/* Since no special option -T is known to blockmode */
			module = "blockmode";
			break;
		default:	/* Call blockmedian for some arbitrary quantile by passing the given -T<q> */
			module = "blockmedian";
			break;
	}
	
	/* Do the main work via the chosen module */
	if ((error = GMT_Call_Module (API, module, GMT_MODULE_OPT, options))) Exit (error); 	/* If errors then we return that next */

 	/* Free option list */
	if (GMT_Destroy_Options (API, &options)) Exit (EXIT_FAILURE);

 	/* Done with the GMT API */
	if (GMT_Destroy_Session (API) != GMT_NOERROR) Exit (EXIT_FAILURE);

	Exit (EXIT_SUCCESS);
}
