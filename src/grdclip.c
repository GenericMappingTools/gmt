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
 * API functions to support the gmtconvert application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: Read a grid file and sets all values < the user-supplied
 * lower limit to the value <below>, and all values > the user-supplied
 * upper limit to the value <above>.  above/below can be any number,
 * including NaN.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdclip"
#define THIS_MODULE_MODERN_NAME	"grdclip"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Clip the range of grid values"
#define THIS_MODULE_KEYS	"<G{,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RV"

enum Grdclip_cases {
	GRDCLIP_BELOW	= 1,
	GRDCLIP_BETWEEN	= 2,
	GRDCLIP_ABOVE	= 4
};

/* Control structure for grdclip */

struct GRDCLIP_RECLASSIFY {
	gmt_grdfloat low, high, between;
	uint64_t n_between;
	bool replace;	/* true if low == high */
};

struct GRDCLIP_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct S {	/* -Sa<high/above>, -Sb<low/below>, -Si<low/high/between>, -Sr<old>/<new> */
		bool active;
		unsigned int mode;
		unsigned int n_class;
		unsigned int n_replace;
		gmt_grdfloat high, above;
		gmt_grdfloat low, below;
		struct GRDCLIP_RECLASSIFY *class;
		struct GRDCLIP_REPLACE *replace;
	} S;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCLIP_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDCLIP_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
			
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDCLIP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);	
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C->S.class);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> -G<outgrid> [%s] [-Sa<high>/<above>]\n", name, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Sb<low>/<below>] [-Si<low>/<high>/<between>] [-Sr<old>/<new>] [%s] [%s]\n\n", GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\t<ingrid> is a single grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name of output grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-Sa will set all data > high to the <above> value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Sb will set all data < low to the <below> value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Si will set all data >= low and <= high to the <between> value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Sr will set all data == old to the <new> value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    <above>, <below>, <between>, and <new> can be any number, including NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Choose at least one -S option; -Si -Sr may be repeated.\n");
	GMT_Option (API, "V,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int compare_classes (const void *point_1v, const void *point_2v) {
	/*  Needed to sort classes on low value. */
	const struct GRDCLIP_RECLASSIFY *point_1 = point_1v, *point_2 = point_2v;
	
	if (point_1->low < point_2->low) return (-1);
	if (point_1->low > point_2->low) return (+1);
	return (0);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDCLIP_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, n_class = 0;
	int n, n_to_expect;
	size_t n_alloc = GMT_TINY_CHUNK;
	char txt[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Output filename */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'S':	/* Set limits */
				Ctrl->S.active = true;
				n_to_expect = 2;
				switch (opt->arg[0]) {
				case 'a':
					Ctrl->S.mode |= GRDCLIP_ABOVE;
#ifdef DOUBLE_PRECISION_GRID
					n = sscanf (&opt->arg[1], "%lf/%s", &Ctrl->S.high, txt);
#else
					n = sscanf (&opt->arg[1], "%f/%s", &Ctrl->S.high, txt);
#endif
					if (n != n_to_expect) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Sa option: Expected -Sa<high>/<above>, <above> may be set to NaN\n");
						n_errors++;
					}
					else 
						Ctrl->S.above = (txt[0] == 'N' || txt[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (txt);
					break;
				case 'b':
					Ctrl->S.mode |= GRDCLIP_BELOW;
#ifdef DOUBLE_PRECISION_GRID
					n = sscanf (&opt->arg[1], "%lf/%s", &Ctrl->S.low, txt);
#else
					n = sscanf (&opt->arg[1], "%f/%s", &Ctrl->S.low, txt);
#endif
					if (n != n_to_expect) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Sb option: Expected -Sb<low>/<below>, <below> may be set to NaN\n");
						n_errors++;
					}
					else
						Ctrl->S.below = (txt[0] == 'N' || txt[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (txt);
					break;
				case 'i':
					n_to_expect = 3;	/* Since only two for -Sr */
					/* Fall through on purpose to 'r' */
				case 'r':
					Ctrl->S.mode |= GRDCLIP_BETWEEN;
					if (n_class == Ctrl->S.n_class) {	/* Need more memory */
						n_alloc <<= 2;
						Ctrl->S.class = gmt_M_memory (GMT, Ctrl->S.class, n_alloc, struct GRDCLIP_RECLASSIFY);
					}
					if (n_to_expect == 3) {
#ifdef DOUBLE_PRECISION_GRID
						n = sscanf (&opt->arg[1], "%lf/%lf/%s", &Ctrl->S.class[n_class].low, &Ctrl->S.class[n_class].high, txt);
#else
						n = sscanf (&opt->arg[1], "%f/%f/%s", &Ctrl->S.class[n_class].low, &Ctrl->S.class[n_class].high, txt);
#endif
						if (n != n_to_expect) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Si option: Expected -Si<low>/<high>/<between>, <between> may be set to NaN\n");
							n_errors++;
						}
						else
							Ctrl->S.class[n_class].between = (txt[0] == 'N' || txt[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (txt);
					}
					else {
#ifdef DOUBLE_PRECISION_GRID
						n = sscanf (&opt->arg[1], "%lf/%s", &Ctrl->S.class[n_class].low, txt);
#else
						n = sscanf (&opt->arg[1], "%f/%s", &Ctrl->S.class[n_class].low, txt);
#endif
						if (n != n_to_expect) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Sr option: Expected -Sr<old>/<new>, <new> may be set to NaN\n");
							n_errors++;
						}
						else
							Ctrl->S.class[n_class].between = (txt[0] == 'N' || txt[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (txt);
						Ctrl->S.class[n_class].high = Ctrl->S.class[n_class].low;
						Ctrl->S.class[n_class].replace = true;
					}
					if (Ctrl->S.class[n_class].low > Ctrl->S.class[n_class].high) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Si option: <low> cannot exceed <high>!\n");
						n_errors++;
					}
					n_class++;
					break;
				default:
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Expected -Sa<high>/<above>, -Sb<low>/<below>, -Si<low>/<high>/<between> or -Si<old>/<new>\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (Ctrl->S.mode & GRDCLIP_BETWEEN) {	/* Reallocate, sort and check that no classes overlap */
		unsigned int k;
		Ctrl->S.class = gmt_M_memory (GMT, Ctrl->S.class, n_class, struct GRDCLIP_RECLASSIFY);
		Ctrl->S.n_class = n_class;
		qsort (Ctrl->S.class, Ctrl->S.n_class, sizeof (struct GRDCLIP_RECLASSIFY), compare_classes);
		for (k = 1; k < Ctrl->S.n_class; k++) {
			if (Ctrl->S.class[k].low < Ctrl->S.class[k-1].high) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Si option: Reclassification case %d overlaps with case %d\n", k, k-1);
				n_errors++;
			}
			if (!Ctrl->S.class[k].replace && (gmt_M_is_fnan (Ctrl->S.class[k].low) || gmt_M_is_fnan (Ctrl->S.class[k-1].high))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Si option: Reclassification case %d contains NaN as high or low value\n", k);
				n_errors++;
			}
		}
		if (Ctrl->S.mode & GRDCLIP_ABOVE && Ctrl->S.high < Ctrl->S.class[Ctrl->S.n_class-1].high) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Si option: Your highest reclassification case overlaps with your -Sa selection\n");
			n_errors++;
		}
		if (Ctrl->S.mode & GRDCLIP_BELOW && Ctrl->S.low > Ctrl->S.class[0].low) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Si option: Your lowest reclassification case overlaps with your -Sb selection\n");
			n_errors++;
		}
	}
	if ((Ctrl->S.mode & GRDCLIP_ABOVE) && (Ctrl->S.mode & GRDCLIP_BELOW) && (Ctrl->S.high < Ctrl->S.low)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Your -Sa selection overlaps with your -Sb selection\n");
		n_errors++;
	}
	
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.active, "Syntax error -G option is mandatory\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.mode, "Syntax error -S option: Must specify at least one of -Sa, -Sb, -Si, -Sr\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdclip (void *V_API, int mode, void *args) {
	unsigned int row, col, k;
	int error = 0;
	bool new_grid, go = false;
	
	uint64_t ij, n_above = 0, n_below = 0;
	
	double wesn[4];
	
	struct GRDCLIP_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *Out = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdclip main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grid\n");
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (gmt_M_is_subset (GMT, G->header, wesn)) gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "");	/* Subset requested; make sure wesn matches header spacing */
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL) {
		Return (API->error);	/* Get subset */
	}

	new_grid = gmt_set_outgrid (GMT, Ctrl->In.file, false, G, &Out);	/* true if input is a read-only array */

	gmt_M_grd_loop (GMT, G, row, col, ij) {
		/* Checking if extremes are exceeded (need not check NaN) */
		if (Ctrl->S.mode & GRDCLIP_ABOVE && G->data[ij] > Ctrl->S.high) {
			Out->data[ij] = Ctrl->S.above;
			n_above++;
		}
		else if (Ctrl->S.mode & GRDCLIP_BELOW && G->data[ij] < Ctrl->S.low) {
			Out->data[ij] = Ctrl->S.below;
			n_below++;
		}
		else if (Ctrl->S.mode & GRDCLIP_BETWEEN) {	/* Reclassifications */
			for (k = 0, go = true; go && k < Ctrl->S.n_class; k++) {
				if ((Ctrl->S.class[k].replace && gmt_M_is_fnan (Ctrl->S.class[k].low) && gmt_M_is_fnan (G->data[ij])) || \
				   (G->data[ij] >= Ctrl->S.class[k].low && G->data[ij] <= Ctrl->S.class[k].high)) {
					Out->data[ij] = Ctrl->S.class[k].between;
					Ctrl->S.class[k].n_between++;
					go = false;
				}
			}
		}
		else if (new_grid)
			Out->data[ij] = G->data[ij];
	}	

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Out) != GMT_NOERROR) {
		Return (API->error);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		char format[GMT_BUFSIZ] = {""}, format2[GMT_BUFSIZ] = {""}, buffer[GMT_BUFSIZ] = {""};
		strcpy (format, "%" PRIu64 " values ");
		if (Ctrl->S.mode & GRDCLIP_BELOW) {
			sprintf (buffer, "< %s set to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			strcat (format, buffer);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, n_below, Ctrl->S.low, Ctrl->S.below);
		}
		if (Ctrl->S.mode & GRDCLIP_BETWEEN) {
			strcpy (format, "%" PRIu64 " values ");
			strcpy (format2, "%" PRIu64 " values ");
			sprintf (buffer, "between %s and %s set to %s\n", GMT->current.setting.format_float_out, 
				GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			strcat (format, buffer);
			sprintf (buffer, "equal to %s set to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			strcat (format2, buffer);
			for (k = 0; k < Ctrl->S.n_class; k++) {
				if (Ctrl->S.class[k].replace)
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, format2, Ctrl->S.class[k].n_between, Ctrl->S.class[k].low, Ctrl->S.class[k].between);
				else
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, Ctrl->S.class[k].n_between, Ctrl->S.class[k].low, Ctrl->S.class[k].high, Ctrl->S.class[k].between);
			}
		}
		if (Ctrl->S.mode & GRDCLIP_ABOVE) {
			strcpy (format, "%" PRIu64 " values ");
			sprintf (buffer, "> %s set to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, n_above, Ctrl->S.high, Ctrl->S.above);
		}
	}

	Return (GMT_NOERROR);
}
