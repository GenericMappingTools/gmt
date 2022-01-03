/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2009-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#include "gmt_dev.h"
#include "mgd77.h"

#define THIS_MODULE_CLASSIC_NAME	"mgd77magref"
#define THIS_MODULE_MODERN_NAME	"mgd77magref"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Evaluate the IGRF or CM4 magnetic field models"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vbdho" GMT_OPT("Hm")

struct MGD77MAGREF_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct MGD77_CM4 *CM4;
	bool copy_input;
	bool do_IGRF;
	bool do_CM4;
	bool joint_IGRF_CM4;
	struct MGD77MAGREF_A {	/* -A */
		bool active;
		bool fixed_alt;
		bool fixed_time;
		int years;
		int t_col;
		double altitude;
		double time;
	} A;
	struct MGD77MAGREF_C {	/* -C */
		bool active;
	} C;
	struct MGD77MAGREF_D {	/* -D */
		bool active;
	} D;
	struct MGD77MAGREF_F {	/* -F */
		bool active;
	} F;
	struct MGD77MAGREF_G {	/* -G */
		bool active;
	} G;
	struct MGD77MAGREF_L {	/* -L */
		bool active;
	} L;
	struct MGD77MAGREF_S {	/* -S */
		bool active;
	} S;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77MAGREF_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct MGD77MAGREF_CTRL);
	C->CM4 = calloc (1U, sizeof (struct MGD77_CM4));

	/* Initialize values whose defaults are not 0/false/NULL */

	C->do_CM4 = true;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct MGD77MAGREF_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->CM4->CM4_M.path);
	gmt_M_str_free (C->CM4->CM4_D.path);
	gmt_M_str_free (C->CM4->CM4_I.path);
	gmt_M_str_free (C->CM4);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] [-A+a<alt>+t<date>+y] [-C<cm4file>] [-D<dstfile>] [-E<f107file>] "
		"[-Frthxyzdi[/[0|9]1234567]] [-G] [-Lrtxyz[/1234]] [-Sc|l<low>/<high>] [%s "
		"[%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_h_OPT, GMT_o_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<table>");
	GMT_Usage (API, -2, "File with records that must contain <lon>, <lat>, <alt>, <time>[, other cols]. "
		"Here, (<lon>, <lat>) is the geocentric position on the ellipsoid [but see -G], "
		"<alt> is the altitude in km positive above the ellipsoid, and "
		"<time> is the time of data acquisition, in <date>T<clock> format (but see -A+y). "
		"We read standard input if no input file is given.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A+a<alt>+t<date>+y");
	GMT_Usage (API, -2, "Adjust how the input records are interpreted. Append modifiers:");
	GMT_Usage (API, 3, "+a Append <alt> to indicate a constant altitude [Default is 3rd column].");
	GMT_Usage (API, 3, "+t Append <time> to indicate a constant time [Default is 4th column].");
	GMT_Usage (API, 3, "+y Indicate times are given in decimal years [Default is ISO <date>T<clock> format].");
	GMT_Usage (API, 1, "\n-C<cm4file>");
	GMT_Usage (API, -2, "Select an alternate file with coefficients for the CM4 model [%s/umdl.CM4].",
		API->GMT->session.SHAREDIR);
	GMT_Usage (API, 1, "\n-D<dstfile>");
	GMT_Usage (API, -2, "Select an alternate file with hourly means of the Dst index for CM4 [%s/Dst_all.wdc], "
		"OR a single Dst index to apply for all records.",
		API->GMT->session.SHAREDIR);
	GMT_Usage (API, 1, "\n-E<f107file>");
	GMT_Usage (API, -2, "Select an alternate file with monthly means of absolute F10.7 solar radio flux for CM4 [%s/F107_mon.plt], "
		"OR a single solar radio flux to apply for all records.",
		API->GMT->session.SHAREDIR);
	GMT_Usage (API, 1, "\n-Frthxyzdi[/[0|9]1234567]");
	GMT_Usage (API, -2, "Dataflags is a string made up of one or more of these codes:");
	GMT_Usage (API, 3, "r: Output all input columns before adding the items below (all in nTesla).");
	GMT_Usage (API, 3, "t: List total field.");
	GMT_Usage (API, 3, "h: List horizontal field.");
	GMT_Usage (API, 3, "x: List X component.");
	GMT_Usage (API, 3, "y: List Y component.");
	GMT_Usage (API, 3, "z: List Z component.");
	GMT_Usage (API, 3, "d: List declination.");
	GMT_Usage (API, 3, "i: List inclination.");
	GMT_Usage (API, -2, "Optionally, append one or more numbers to indicate the requested field contribution(s):");
	GMT_Usage (API, 3, "0: Core field from IGRF only (no CM4 evaluation).");
	GMT_Usage (API, 3, "1: Core field.");
	GMT_Usage (API, 3, "2: Lithospheric field.");
	GMT_Usage (API, 3, "3: Primary Magnetospheric field.");
	GMT_Usage (API, 3, "4: Induced Magnetospheric field.");
	GMT_Usage (API, 3, "5: Primary ionospheric field.");
	GMT_Usage (API, 3, "6: Induced ionospheric field.");
	GMT_Usage (API, 3, "7: Toroidal field.");
	GMT_Usage (API, 3, "9: Core field from IGRF and other contributions from CM4. DO NOT USE BOTH 1 AND 9.");
	GMT_Usage (API, -2, "Note: Append several numbers to add up the different contributions. For example, "
		"-Ft/12 computes the total field due to CM4 Core and Lithospheric sources. "
		"Two special cases are allowed which mix Core field from IGRF and other sources from CM4: "
		"-Ft/934 computes Core field due to IGRF plus terms 3 and 4 from CM4. "
		"-Fxyz/934 the same as above but output the field components. "
		"The data are written out in the order specified "
		"[Default is -Frthxyzdi/1].");
	GMT_Usage (API, 1, "\n-G Specify that coordinates are geocentric [geodetic].");
	GMT_Usage (API, 1, "\n-Lrtxyz[/1234]");
	GMT_Usage (API, -2, "Compute J field vectors from certain external sources. "
		"Append a string made up of one or more of these codes:");
		GMT_Usage (API, 3, "r: Output all input columns before adding the items below (all in Ampere/m).");
		GMT_Usage (API, 3, "t: List magnitude field.");
		GMT_Usage (API, 3, "x: List X component.");
		GMT_Usage (API, 3, "y: List Y component.");
		GMT_Usage (API, 3, "z: List Z or current function Psi.");
	GMT_Usage (API, -2, "Optionally, append a number to indicate the requested J contribution(s):");
	GMT_Usage (API, -2, "1: Induced Magnetospheric field.");
	GMT_Usage (API, -2, "2: Primary ionospheric field.");
	GMT_Usage (API, -2, "3: Induced ionospheric field.");
	GMT_Usage (API, -2, "4: Poloidal field.");
	GMT_Usage (API, 1, "\n-Sc|l<low>/<high>");
	GMT_Usage (API, -2, "Limit the CM4 contributions from core and lithosphere to certain harmonic degree bands. "
		"Append c(ore) or l(ithosphere) and the <low> and <high> degrees to use [-Sc1/13 -Sl14/65].");
	GMT_Option (API, "V,bi0");
	if (gmt_M_showusage (API)) {
		GMT_Usage (API, -2, "Default is 4 input columns (unless -A is used).  Note for binary input, absolute time must "
			"be in the UNIX time-system (unless -A+y is used).");
	}
	GMT_Option (API, "bo,d,h,o,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct MGD77MAGREF_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mgd77magref and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos, n_out, lfval = 0, pos_slash = 0, nval = 0, nfval = 0, lval = 0;
	int j;
	char p[GMT_BUFSIZ] = {""}, tfixed[GMT_LEN64] = {""};
	bool do_CM4core = false;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				pos = 0;
				while ((gmt_strtok (opt->arg, "+", &pos, p))) {
					switch (p[0]) {
						case 'a':
							Ctrl->A.fixed_alt = true;
							Ctrl->A.altitude = atof (&p[1]);
							break;
						case 't':
							Ctrl->A.fixed_time = true;
							strncpy (tfixed, &p[1], GMT_LEN64-1);
							gmt_set_column_type (GMT, GMT_OUT, 3, GMT_IS_FLOAT);
							break;
						case 'y':
							Ctrl->A.years = true;
							gmt_set_column_type (GMT, GMT_IO, 2, GMT_IS_FLOAT);
							gmt_set_column_type (GMT, GMT_IO, 3, GMT_IS_FLOAT);
							break;
						default:
							break;
					}
				}
				if (Ctrl->A.fixed_time) {
					if (Ctrl->A.years)
						Ctrl->A.time = atof (tfixed);
					else
						gmt_scanf_arg (GMT, tfixed, GMT_IS_ABSTIME, false, &Ctrl->A.time);
				}
				break;
			case 'C':	/* Alternate CM4 coefficient file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->CM4->CM4_M.path);
				Ctrl->CM4->CM4_M.path = strdup (opt->arg);
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				j = 0;
				if (opt->arg[j] == '-') j++;
				if ((opt->arg[j] > 47) && (opt->arg[j] < 58)) {	/* arg is numeric -> Dst Index */
					Ctrl->CM4->CM4_D.dst[0] = atof (&opt->arg[j]);
					Ctrl->CM4->CM4_D.index = false;
				}
				else {
					gmt_M_str_free (Ctrl->CM4->CM4_D.path);
					Ctrl->CM4->CM4_D.path = strdup (opt->arg);
					Ctrl->CM4->CM4_D.load = true;
				}
				break;
			case 'E':
				if ((opt->arg[0] > 47) && (opt->arg[0] < 58)) {	/* arg is numeric -> Dst Index */
					Ctrl->CM4->CM4_I.F107 = atof (opt->arg);
					Ctrl->CM4->CM4_I.index = false;
				}
				else {
					gmt_M_str_free (Ctrl->CM4->CM4_I.path);
					Ctrl->CM4->CM4_I.path = strdup (opt->arg);
					Ctrl->CM4->CM4_I.load = true;
				}
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->CM4->CM4_F.active = Ctrl->F.active = true;

				pos_slash = 0;
				for (j = 0; opt->arg[j]; j++) {
					if (opt->arg[j] == '/') {
						pos_slash = j;
						break;
					}
					switch (opt->arg[j]) {
						case 'r':		/* Echo input record */
							Ctrl->copy_input = true;
							break;
						case 't':		/* Total field is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 0;
							break;
						case 'h':		/* Horizontal field is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 1;
							break;
						case 'x':		/* X component is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 2;
							break;
						case 'y':		/* Y component is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 3;
							break;
						case 'z':		/* Z component is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 4;
							break;
						case 'd':		/* Declination is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 5;
							break;
						case 'i':		/* Inclination is requested */
							Ctrl->CM4->CM4_F.field_components[nval++] = 6;
							break;
					}
				}
				Ctrl->CM4->CM4_F.n_field_components = (int)nval;

				if (pos_slash) {
					for (j = pos_slash; opt->arg[j]; j++) {
						switch (opt->arg[j]) {
							case '0':		/* IGRF */
								Ctrl->do_IGRF = true;
								Ctrl->do_CM4  = false;
								Ctrl->joint_IGRF_CM4 = false;
								break;
							case '1':		/* Main field 1 */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 0;
								do_CM4core = true;
								break;
							case '2':		/* Main field 2 */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 1;
								break;
							case '3':		/* Primary Magnetospheric field */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 2;
								break;
							case '4':		/* Induced Magnetospheric field */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 3;
								break;
							case '5':		/* Primary ionospheric field */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 4;
								break;
							case '6':		/* Induced ionospheric field */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 5;
								break;
							case '7':		/* Toroidal field */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 6;
								break;
							case '9':		/* Main field is computed with the IGRF */
								Ctrl->do_IGRF = false;/* No contradiction, the way will be through joint_IGRF_CM4 */
								Ctrl->do_CM4  = true;	/* Well maybe, if some other source is selected also */
								Ctrl->joint_IGRF_CM4 = true;
								break;
						}
					}
					Ctrl->CM4->CM4_F.n_field_sources = (int)nfval;
				}
				break;
			case 'G':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				Ctrl->CM4->CM4_G.geodetic = false;
				break;
			case 'L':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->CM4->CM4_L.curr = Ctrl->L.active = true;

				pos_slash = 0;
				for (j = 0; opt->arg[j]; j++) {
					if (opt->arg[j] == '/') {
						pos_slash = j;
						break;
					}
					switch (opt->arg[j]) {
						case 'r':		/* Echo input record */
							Ctrl->copy_input = true;
							break;
						case 't':		/* Total field is requested */
							Ctrl->CM4->CM4_L.curr_components[lval++] = 0;
							break;
						case 'x':		/* X component is requested */
							Ctrl->CM4->CM4_L.curr_components[lval++] = 1;
							break;
						case 'y':		/* Y component is requested */
							Ctrl->CM4->CM4_L.curr_components[lval++] = 2;
							break;
						case 'z':		/* Z component is requested */
							Ctrl->CM4->CM4_L.curr_components[lval++] = 3;
							break;
					}
				}
				Ctrl->CM4->CM4_L.n_curr_components = (int)lval;

				if (pos_slash) {
					for (j = pos_slash; opt->arg[j]; j++) {
						switch (opt->arg[j]) {
							case '1':		/* Induced Magnetospheric field */
								Ctrl->CM4->CM4_L.curr_sources[lfval++] = 0;
								break;
							case '2':		/* Primary ionospheric field */
								Ctrl->CM4->CM4_L.curr_sources[lfval++] = 1;
								break;
							case '3':		/* Induced ionospheric field */
								Ctrl->CM4->CM4_L.curr_sources[lfval++] = 2;
								break;
							case '4':		/* Poloidal field */
								Ctrl->CM4->CM4_L.curr_sources[lfval++] = 3;
								break;
						}
					}
					Ctrl->CM4->CM4_L.n_curr_sources = (int)lfval;
				}
				break;
			case 'S':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				if (opt->arg[0] == 'c') {
					j = sscanf (&opt->arg[1], "%d/%d", &Ctrl->CM4->CM4_S.nlmf[0], &Ctrl->CM4->CM4_S.nhmf[0]);
					if (j != 2) {
						GMT_Report (API, GMT_MSG_ERROR, "-Sc option usage is -Sc<low/high>\n");
						n_errors++;
					}
				}
				if (opt->arg[0] == 'l') {
					j = sscanf (&opt->arg[1], "%d/%d", &Ctrl->CM4->CM4_S.nlmf[1], &Ctrl->CM4->CM4_S.nhmf[1]);
					if (j != 2) {
						GMT_Report (API, GMT_MSG_ERROR, "-Sl option usage is -Sl<low/high>\n");
						n_errors++;
					}
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_out = 4 - (Ctrl->A.fixed_alt + Ctrl->A.fixed_time);	/* Minimum input columns (could be more) */
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0)
		GMT->common.b.ncol[GMT_IN] = n_out;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0,
			"Binary input data (-bi) must have at least %d columns\n", n_out);
	n_errors += gmt_M_check_condition (GMT, Ctrl->CM4->CM4_F.active && Ctrl->CM4->CM4_L.curr,
			"You cannot select both -F and -L options\n");
	n_errors += gmt_M_check_condition (GMT, (do_CM4core && Ctrl->do_IGRF) || (do_CM4core && Ctrl->joint_IGRF_CM4),
			"You cannot select both CM4 core (1) and IGRF as they are both core fields.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_mgd77magref (void *V_API, int mode, void *args) {
	unsigned int j, nval = 0, nfval = 0, error = 0;
	unsigned int lval = 0, lfval = 0, n_field_components, tbl;
	unsigned int n_out = 0, n_in, t_col = 3;
	bool cm4_igrf_T = false;

	size_t i, s, need = 0, n_alloc = 0;

	double the_altitude, the_time, *time_array = NULL, *alt_array = NULL, *time_years = NULL, IGRF[7], out[GMT_MAX_COLUMNS];
	double *igrf_xyz = NULL;	/* Temporary storage for the joint_IGRF_CM4 case */

	struct MGD77_CONTROL M;
	struct MGD77MAGREF_CTRL *Ctrl = NULL;
	struct GMT_DATASET *Din = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */
	MGD77_CM4_init (GMT, &M, Ctrl->CM4);	/* Presets path using strdup */
	if ((error = parse (GMT, Ctrl, options)) != 0) {
		MGD77_end (GMT, &M);
		Return (error);
	}

	/*---------------------------- This is the mgd77magref main code ----------------------------*/

	n_in = 4 - (Ctrl->A.fixed_alt + Ctrl->A.fixed_time);

	if (Ctrl->A.fixed_alt) t_col = 2;	/* Since we are missing the altitude column */

	Ctrl->CM4->CM4_D.dst = calloc (1U, sizeof(double));	/* We need at least a size of one in case a value is given in input */
	if (!Ctrl->A.fixed_time)			/* Otherwise we don't print the time */
		gmt_set_column_type (GMT, GMT_IO, t_col, GMT_IS_ABSTIME);

	/* Shorthand for these */
	nval = Ctrl->CM4->CM4_F.n_field_components;
	nfval = Ctrl->CM4->CM4_F.n_field_sources;
	lval = Ctrl->CM4->CM4_L.n_curr_components;
	lfval = Ctrl->CM4->CM4_L.n_curr_sources;

	if (nfval && Ctrl->do_IGRF) GMT_Message (API, GMT_TIME_NONE, "Warning. Source fields other than IGRF will be ignored. It's in the manual\n");

	/* ------------- Test, and take measures, if mix mode IGRF/CM4 is to be used -------------------------- */
	if (Ctrl->joint_IGRF_CM4) {
		if (nfval == 0) {	/* It means we had a -F.../9 which is exactly iqual to -F.../0 */
			Ctrl->do_IGRF = true;
			Ctrl->do_CM4  = false;
			Ctrl->joint_IGRF_CM4 = false;
		}
		else {
			cm4_igrf_T = false;
			if ( (nval == 1) && (Ctrl->CM4->CM4_F.field_components[0] == 0) ) {
				for (i = 0; i < 3; i++) Ctrl->CM4->CM4_F.field_components[i] = (int)i+2;	/* Force the x,y,z request */
				cm4_igrf_T = true;
			}
			else if (!((nval == 3) && (Ctrl->CM4->CM4_F.field_components[0] == 2) && (Ctrl->CM4->CM4_F.field_components[1] == 3) &&
						(Ctrl->CM4->CM4_F.field_components[2] == 4)) ) {
				GMT_Report (API, GMT_MSG_ERROR, "In mix CM4/IGRF mode -F option can only be -Ft[r]/... or -Fxyz[r]/...\n");
				error++;
			}

			nval = 3;
			Ctrl->CM4->CM4_F.n_field_components = (int)nval;
		}
	}
	/* ----------------------------------------------------------------------------------------------------- */

	if (error) Return (GMT_RUNTIME_ERROR);

	if (!Ctrl->CM4->CM4_F.active && !Ctrl->CM4->CM4_L.curr) Ctrl->CM4->CM4_F.active = true;

	/* Sort the order in which the parameters appear */
	if (Ctrl->CM4->CM4_F.active) {
		if (nval == 0) {		/* No components selected, default used */
			Ctrl->copy_input = true;
			Ctrl->CM4->CM4_F.n_field_components = 7;
			for (i = 0; i < 7; i++) Ctrl->CM4->CM4_F.field_components[i] = (int)i;
		}
		if (nfval == 0 && !Ctrl->joint_IGRF_CM4) {		/* No sources selected, retain only the main field */
			Ctrl->CM4->CM4_F.field_sources[0] = 0;
			Ctrl->CM4->CM4_F.n_field_sources = 1;
		}
		n_field_components = Ctrl->CM4->CM4_F.n_field_components;
	}
	else {
		if (lval == 0) {		/* Nothing selected, default used */
			Ctrl->copy_input = true;
			Ctrl->CM4->CM4_L.n_curr_components = 1;
			Ctrl->CM4->CM4_L.curr_components[0] = 0;
		}
		if (lfval == 0) {		/* Nothing selected, retain only the induced magnetospheric field */
			Ctrl->CM4->CM4_L.curr_sources[0] = 0;
			Ctrl->CM4->CM4_L.n_curr_sources = 1;
		}
		n_field_components = Ctrl->CM4->CM4_L.n_curr_components;
	}

	if (Ctrl->A.fixed_alt) {	/* A single altitude should apply to all records; set array to point to this single altitude */
		alt_array = &Ctrl->A.altitude;
		Ctrl->CM4->CM4_DATA.n_altitudes = 1;
	}
	if (Ctrl->A.fixed_time) {	/* A single time should apply to all records; set array to point to this single time */
		if (!Ctrl->A.years) {
			if (M.adjust_time) Ctrl->A.time = MGD77_time2utime (GMT, &M, Ctrl->A.time);	/* Convert to Unix time if need be */
			Ctrl->A.time = MGD77_time_to_fyear (GMT, &M, Ctrl->A.time);			/* Get decimal year */
		}
		time_array = &Ctrl->A.time;
		Ctrl->CM4->CM4_DATA.n_times = 1;
	}
	else	/* Make sure input time columns are encoded/decoded properly since here we know t_col is set. */
		gmt_set_column_type (GMT, GMT_IO, t_col, (Ctrl->A.years) ? GMT_IS_FLOAT : GMT_IS_ABSTIME);

	gmt_set_column_type (GMT, GMT_IO, t_col+1, GMT_IS_FLOAT);		/* Override any previous t_col = 3 settings */
	if (!Ctrl->copy_input) {	/* No time on output */
		gmt_set_column_type (GMT, GMT_OUT, 2, GMT_IS_FLOAT);
		gmt_set_column_type (GMT, GMT_OUT, 3, GMT_IS_FLOAT);
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}

	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < n_in) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least %d are needed\n", (int)Din->n_columns, n_in);
		Return (GMT_DIM_TOO_SMALL);
	}
	n_out = n_field_components + ((Ctrl->copy_input) ? (unsigned int)Din->n_columns : 0);
	if (cm4_igrf_T) n_out -= 2;	/* Decrease by 2 because the x,y,z were imposed internally only. i.e not for output */
	if ((error = GMT_Set_Columns (API, GMT_OUT, n_out, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}

	if (GMT->common.b.active[GMT_OUT] && GMT->common.b.ncol[GMT_OUT] > 0 && n_out > GMT->common.b.ncol[GMT_OUT]) {
		GMT_Report (API, GMT_MSG_ERROR, "Binary output must have at least %d columns (your -bo option only set %d)\n", n_out, GMT->common.b.ncol[GMT_OUT]);
		Return (GMT_RUNTIME_ERROR);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	for (tbl = 0; tbl < Din->n_tables; tbl++) {	/* Loop over all input tables */
		T = Din->table[tbl];	/* Current table */

		if (T->n_columns < n_in) {
			GMT_Report (API, GMT_MSG_ERROR, "Table %d has %d columns, but from the used options we expect %d\n",
			            tbl + 1, T->n_columns, n_in);
			continue;
		}

		for (s = 0; s < T->n_segments; s++) {	/* Process each file segment separately */
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, T->segment[s]->header);
			need = T->segment[s]->n_rows;   /* Size of output array needed in MGD77_cm4field */
			if (need > n_alloc) {           /* Need to reallocate */
				n_alloc = need;
				Ctrl->CM4->CM4_DATA.out_field = gmt_M_memory (GMT, Ctrl->CM4->CM4_DATA.out_field, n_alloc * n_field_components, double);
				if (!(Ctrl->A.years || Ctrl->A.fixed_time))
					time_years = gmt_M_memory (GMT, time_years, n_alloc, double);

				if (Ctrl->joint_IGRF_CM4)
					igrf_xyz = gmt_M_memory (GMT, igrf_xyz, n_alloc * 3, double);
			}

			if (!Ctrl->A.fixed_alt) {	/* Assign the alt_array to the provided altitude array */
				alt_array = T->segment[s]->data[GMT_Z];
				Ctrl->CM4->CM4_DATA.n_altitudes = (int)T->segment[s]->n_rows;
			}

			if (!Ctrl->A.fixed_time) {	/* Assign the time_array to the provided time array */
				Ctrl->CM4->CM4_DATA.n_times = (int)T->segment[s]->n_rows;
				if (Ctrl->A.years)
					time_array = T->segment[s]->data[t_col];
				else {	/* Must convert internal GMT time to decimal years first */
					for (i = 0; i < T->segment[s]->n_rows; i++)
						time_years[i] = MGD77_time_to_fyear (GMT, &M, T->segment[s]->data[t_col][i]);
					time_array = time_years;
				}
			}

			if (!(Ctrl->do_IGRF || Ctrl->joint_IGRF_CM4 ) && !s && time_array[0] > 2002.7) {	/* Only atmospheric terms may be reliable */
				GMT_Report (API, GMT_MSG_WARNING, "Time is outside the CM4 strict validity domain [1960.0-2002.7].\n");
				GMT_Report (API, GMT_MSG_WARNING, "\tThe secular variation estimation will be unreliable. In this"
				                                 "\n\tcase you really should use the IGRF to estimate the core contribution\n");
			}

			Ctrl->CM4->CM4_DATA.n_pts = (int)T->segment[s]->n_rows;
			if (Ctrl->do_IGRF || Ctrl->joint_IGRF_CM4) {
				int type;
				type = (Ctrl->CM4->CM4_G.geodetic) ? 1 : 2;
				for (i = 0; i < T->segment[s]->n_rows; i++) {
					the_altitude = (Ctrl->A.fixed_alt) ? alt_array[0] : alt_array[i];
					the_time = (Ctrl->A.fixed_time) ? time_array[0] : time_array[i];
					if (type == 2) the_altitude += 6371.2;
					MGD77_igrf10syn (GMT, 0, the_time, type, the_altitude, T->segment[s]->data[GMT_X][i],
							T->segment[s]->data[GMT_Y][i], IGRF);
					if (!Ctrl->joint_IGRF_CM4) {		/* IGRF only */
						int jj;
						for (jj = 0; jj < Ctrl->CM4->CM4_F.n_field_components; jj++)
							Ctrl->CM4->CM4_DATA.out_field[i*n_field_components+jj] = IGRF[Ctrl->CM4->CM4_F.field_components[jj]];
					}
					else {				/* Store the IGRF x,y,z components for later use */
						for (j = 0; j < 3; j++)
							igrf_xyz[i*3+j] = IGRF[Ctrl->CM4->CM4_F.field_components[j]];
					}
				}
			}

			if (Ctrl->do_CM4) {				/* DO CM4 only. Eval CM4 at all points */
				int err;
				if ((err = MGD77_cm4field (GMT, Ctrl->CM4, T->segment[s]->data[GMT_X],
							T->segment[s]->data[GMT_Y], alt_array, time_array)) != 0) {
					GMT_Report (API, GMT_MSG_ERROR, "this segment has a record generating an error.\n"
					                                 "Unfortunately, this means all other eventually good\n"
					                                 "records are also ignored. Fix the bad record and rerun the command.\n");
					continue;
				}
			}

			if ((Ctrl->do_CM4 || Ctrl->do_IGRF) && !Ctrl->joint_IGRF_CM4) {	/* DID CM4 or (exclusive) IGRF only. */
				for (i = 0; i < T->segment[s]->n_rows; i++) {	/* Output the requested columns */
					n_out = 0;
					if (Ctrl->copy_input)
						for (j = 0; j < T->segment[s]->n_columns; j++)
							out[n_out++] = T->segment[s]->data[j][i];
					for (j = 0; j < n_field_components; j++)
						out[n_out++] = Ctrl->CM4->CM4_DATA.out_field[i*n_field_components+j];

					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			else {					/* DID CM4 and IGRF */
				double x, y, z;
				for (i = 0; i < T->segment[s]->n_rows; i++) {	/* Output the requested columns */
					n_out = 0;
					if (Ctrl->copy_input) for (j = 0; j < T->segment[s]->n_columns; j++) out[n_out++] = T->segment[s]->data[j][i];
					if (cm4_igrf_T) {
						x = Ctrl->CM4->CM4_DATA.out_field[i*3  ] + igrf_xyz[i*3  ];
						y = Ctrl->CM4->CM4_DATA.out_field[i*3+1] + igrf_xyz[i*3+1];
						z = Ctrl->CM4->CM4_DATA.out_field[i*3+2] + igrf_xyz[i*3+2];
						out[n_out++] = sqrt(x*x + y*y + z*z);
					}
					else {
						for (j = 0; j < 3; j++)
							out[n_out++] = Ctrl->CM4->CM4_DATA.out_field[i*3+j] + igrf_xyz[i*3+j];
					}

					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}

		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	gmt_M_free (GMT, Out);
	gmt_M_str_free (Ctrl->CM4->CM4_D.dst);
	gmt_M_free (GMT, Ctrl->CM4->CM4_DATA.out_field);
	if (!(Ctrl->A.years || Ctrl->A.fixed_time)) gmt_M_free (GMT, time_years);
	if (Ctrl->joint_IGRF_CM4) gmt_M_free (GMT, igrf_xyz);

	MGD77_end (GMT, &M);

	Return (GMT_NOERROR);
}
