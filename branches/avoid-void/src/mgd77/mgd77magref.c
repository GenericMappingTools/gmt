/*--------------------------------------------------------------------
 *	$Id$
 *
 *    Copyright (c) 2009-2011 by J. Luis and P. Wessel
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mgd77magref produces output derived from input locations and time and
 * the CM4 or IGRF magnetic field models.
 *
 * Author:	Joaquim Luis and Paul Wessel
 * Date:	1-MAY-2009
 * Version:	1.0
 *
 */

#include "gmt_mgd77.h"
#include "mgd77.h"

struct MGD77MAGREF_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct MGD77_CM4 *CM4;
	GMT_LONG copy_input;
	GMT_LONG do_IGRF;
	GMT_LONG do_CM4;
	GMT_LONG joint_IGRF_CM4;
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG fixed_alt;
		GMT_LONG fixed_time;
		GMT_LONG years;
		GMT_LONG t_col;
		double altitude;
		double time;
	} A;
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
		GMT_LONG mode;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct G {	/* -G */
		GMT_LONG active;
	} G;
	struct I {	/* -I */
		GMT_LONG active;
		GMT_LONG n;
		char code[3];
	} I;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
};

void *New_mgd77magref_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77MAGREF_CTRL *C = NULL;

	C = GMT_memory (GMT, NULL, 1, struct MGD77MAGREF_CTRL);
	C->CM4 = calloc ((size_t)1, sizeof (struct MGD77_CM4));

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->do_CM4 = TRUE;
	return (C);
}

void Free_mgd77magref_Ctrl (struct GMT_CTRL *GMT, struct MGD77MAGREF_CTRL *C) {	/* Deallocate control structure */
	free (C->CM4);
	GMT_free (GMT, C);
}

GMT_LONG GMT_mgd77magref_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "mgd77magref %s [API] - Evaluate the IGRF or CM4 magnetic field models\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: mgd77magref [<table>] [-A+y+a<alt>+t<date>] [-C<cm4file>] [-D<dstfile>] [-E<f107file>]\n");
	GMT_message (GMT, "\t[-F<rthxyzdi[/[0|9]1234567]>] [-G] [-L<rtxyz[/1234]>] [-Sc|l<low>/<high>]\n");
	GMT_message (GMT, "\t[-V] [%s] [%s] [%s]\n\n", GMT_b_OPT, GMT_h_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<table> contains records that must contain lon, lat, alt, time[, other cols].\n");
	GMT_message (GMT, "\t   longitude and latitude is the geocentric position on the ellipsoid [but see -G].\n");
	GMT_message (GMT, "\t   alt is the altitude in km positive above the ellipsoid.\n");
	GMT_message (GMT, "\t   time is the time of data aquisition, in <date>T<clock> format (but see -A+y).\n");
	GMT_message (GMT, "\t   We read <stdin> if no input file is given.\n");
	GMT_message (GMT, "\t-A Adjust how the input records are interpreted. Append\n");
	GMT_message (GMT, "\t   +a<alt> to indicate a constant altitude [Default is 3rd column].\n");
	GMT_message (GMT, "\t   +t<time> to indicate a constant time [Default is 4th column].\n");
	GMT_message (GMT, "\t   +y to indicate times are given in decimal years [Default is ISO <date>T<clock> format].\n");
	GMT_message (GMT, "\t-C Select an alternate file with coefficients for the CM4 model [%s/umdl.CM4].\n", GMT->session.SHAREDIR);
	GMT_message (GMT, "\t-D Select an alternate file with hourly means of the Dst index for CM4 [%s/Dst_all.wdc],\n", GMT->session.SHAREDIR);
	GMT_message (GMT, "\t   OR a single Dst index to apply for all records.\n");
	GMT_message (GMT, "\t-E Select an alternate file with monthly means of absolute F10.7 solar radio flux for CM4 [%s/F107_mon.plt],\n", GMT->session.SHAREDIR);
	GMT_message (GMT, "\t   OR a single solar radio flux to apply for all records.\n");
	GMT_message (GMT, "\t-F Dataflags is a string made up of 1 or more of these characters:\n");
	GMT_message (GMT, "\t	 r means output all input columns before adding the items below (all in nTesla).\n");
	GMT_message (GMT, "\t	 t means list total field.\n");
	GMT_message (GMT, "\t	 h means list horizontal field.\n");
	GMT_message (GMT, "\t	 x means list X component.\n");
	GMT_message (GMT, "\t	 y means list Y component.\n");
	GMT_message (GMT, "\t	 z means list Z component.\n");
	GMT_message (GMT, "\t	 d means list declination.\n");
	GMT_message (GMT, "\t	 i means list inclination.\n");
	GMT_message (GMT, "\t   Append a number to indicate the requested field contribution(s):\n");
	GMT_message (GMT, "\t	 0 means Core field from IGRF only (no CM4 evalution).\n");
	GMT_message (GMT, "\t	 1 means Core field.\n");
	GMT_message (GMT, "\t	 2 means Lithospheric field.\n");
	GMT_message (GMT, "\t	 3 Primary Magnetospheric field.\n");
	GMT_message (GMT, "\t	 4 Induced Magnetospheric field.\n");
	GMT_message (GMT, "\t	 5 Primary ionospheric field.\n");
	GMT_message (GMT, "\t	 6 Induced ionospheric field.\n");
	GMT_message (GMT, "\t	 7 Toroidal field.\n");
	GMT_message (GMT, "\t	 9 means Core field from IGRF and other contributions from CM4. DO NOT USE BOTH 0 AND 9.\n");
	GMT_message (GMT, "\t   Append several numbers to add up the different contributions. For example,\n");
	GMT_message (GMT, "\t     -Ft/12 computes the total field due to CM4 Core and Lithospheric sources.\n");
	GMT_message (GMT, "\t     Two special cases are allowed which mix which Core field from IGRF and other sources from CM4.\n");
	GMT_message (GMT, "\t     -Ft/934 computes Core field due to IGRF plus terms 3 and 4 from CM4.\n");
	GMT_message (GMT, "\t     -Fxyz/934 the same as above but output the field components.\n");
	GMT_message (GMT, "\t	 The data is written out in the order specified in <dataflags>\n");
	GMT_message (GMT, "\t	 [Default is -Frthxyzdi/1]\n");
	GMT_message (GMT, "\t-G Specify that coordinates are geocentric [geodetic].\n");
	GMT_message (GMT, "\t-L Compute J field vectors from certain external sources.\n");
	GMT_message (GMT, "\t   Dataflags is a string made up of 1 or more of these characters:\n");
	GMT_message (GMT, "\t	 r means output all input columns before adding the items below (all in Ampers/m).\n");
	GMT_message (GMT, "\t	 t means list magnitude field.\n");
	GMT_message (GMT, "\t	 x means list X component.\n");
	GMT_message (GMT, "\t	 y means list Y component.\n");
	GMT_message (GMT, "\t	 z means list Z or current function Psi.\n");
	GMT_message (GMT, "\t   Append a number to indicate the requested J contribution(s)\n");
	GMT_message (GMT, "\t	 1 means Induced Magnetospheric field.\n");
	GMT_message (GMT, "\t	 2 means Primary ionospheric field.\n");
	GMT_message (GMT, "\t	 3 means Induced ionospheric field.\n");
	GMT_message (GMT, "\t	 4 means Poloidal field.\n");
	GMT_message (GMT, "\t-S Limit the CM4 contributions from core and lithosphere to certain harmonic degree bands.\n");
	GMT_message (GMT, "\t   Append c(ore) or l(ithosphere) and the low and high degrees to use [-Sc1/13 -Sl14/65].\n");
	GMT_explain_options (GMT, "VC0");
	GMT_message (GMT, "\t   Default is 4 input columns (unless -A is used).  Note for binary input, absolute time must\n");
	GMT_message (GMT, "\t   be in the unix time-system (unless -A+y is used).\n");
	GMT_explain_options (GMT, "D0fh:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_mgd77magref_parse (struct GMTAPI_CTRL *C, struct MGD77MAGREF_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to mgd77magref and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, j, pos, t_col = 3, pos_slash = 0, nval = 0, nfval = 0, lval = 0;
	GMT_LONG n_out, lfval = 0;
	char p[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				pos = 0;
				while ((GMT_strtok (C->GMT, opt->arg, "+", &pos, p))) {
					switch (p[0]) {
						case 'a':
							Ctrl->A.fixed_alt = TRUE;
							Ctrl->A.altitude = atof (&p[1]);
							t_col = 2;	/* Since we are missing the altitude column */
							break;
						case 't':
							Ctrl->A.fixed_time = TRUE;
							Ctrl->A.time = atof (&p[1]);
							GMT->current.io.col_type[GMT_OUT][3] = GMT_IS_FLOAT;
							break;
						case 'y':
							Ctrl->A.years = TRUE;
							GMT->current.io.col_type[GMT_IN][2] = GMT->current.io.col_type[GMT_OUT][2] = 
												GMT->current.io.col_type[GMT_IN][3] = 
												GMT->current.io.col_type[GMT_OUT][3] = 
												GMT_IS_FLOAT;
							break;
						default:
							break;
					}
				}
				break;
			case 'C':	/* Alternate CM4 coefficient file */
				Ctrl->C.active = TRUE;
				free (Ctrl->CM4->CM4_M.path);
				Ctrl->CM4->CM4_M.path = strdup (opt->arg);
				break;
			case 'D':
				j = 0;
				if (opt->arg[j] == '-') j++;
				if ((opt->arg[j] > 47) && (opt->arg[j] < 58)) {	/* arg is numeric -> Dst Index */
					Ctrl->CM4->CM4_D.dst[0] = atof (&opt->arg[j]);
					Ctrl->CM4->CM4_D.index = FALSE;
				}
				else {
					free (Ctrl->CM4->CM4_D.path);
					Ctrl->CM4->CM4_D.path = strdup (opt->arg);
					Ctrl->CM4->CM4_D.load = TRUE;
				}
				break;
			case 'E':
				if ((opt->arg[0] > 47) && (opt->arg[0] < 58)) {	/* arg is numeric -> Dst Index */
					Ctrl->CM4->CM4_I.F107 = atof (opt->arg);
					Ctrl->CM4->CM4_I.index = FALSE;
				}
				else {
					free (Ctrl->CM4->CM4_I.path);
					Ctrl->CM4->CM4_I.path = strdup (opt->arg);
					Ctrl->CM4->CM4_I.load = TRUE;
				}
				break;
			case 'F':
				Ctrl->CM4->CM4_F.active = TRUE;

				pos_slash = 0;
				for (j = 0; opt->arg[j]; j++) {
					if (opt->arg[j] == '/') {
						pos_slash = j;
						break;
					}
					switch (opt->arg[j]) {
						case 'r':		/* Echo input record */
							Ctrl->copy_input = TRUE;
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
								Ctrl->do_IGRF = TRUE;
								Ctrl->do_CM4  = FALSE;
								Ctrl->joint_IGRF_CM4 = FALSE;
								break;
							case '1':		/* Main field 1 */
								Ctrl->CM4->CM4_F.field_sources[nfval++] = 0;
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
								Ctrl->do_IGRF = FALSE;/* No contradiction, the way will be through joint_IGRF_CM4 */
								Ctrl->do_CM4  = TRUE;	/* Well maybe, if some other source is selected also */
								Ctrl->joint_IGRF_CM4 = TRUE;
								break;
						}
					}
					Ctrl->CM4->CM4_F.n_field_sources = (int)nfval;
				}
				break;
			case 'G':
				Ctrl->CM4->CM4_G.geodetic = FALSE;
				break;
			case 'L':
				Ctrl->CM4->CM4_L.curr = TRUE;

				pos_slash = 0;
				for (j = 0; opt->arg[j]; j++) {
					if (opt->arg[j] == '/') {
						pos_slash = j;
						break;
					}
					switch (opt->arg[j]) {
						case 'r':		/* Echo input record */
							Ctrl->copy_input = TRUE;
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
				if (opt->arg[0] == 'c') {
					j = sscanf (&opt->arg[1], "%d/%d", &Ctrl->CM4->CM4_S.nlmf[0], &Ctrl->CM4->CM4_S.nhmf[0]);
					if (j != 2) {
						GMT_report (GMT, GMT_MSG_FATAL, "Error: -Sc option usage is -Sc<low/high>\n");
						n_errors++;
					}
				}
				if (opt->arg[0] == 'l') {
					j = sscanf (&opt->arg[1], "%d/%d", &Ctrl->CM4->CM4_S.nlmf[1], &Ctrl->CM4->CM4_S.nhmf[1]);
					if (j != 2) {
						GMT_report (GMT, GMT_MSG_FATAL, "Error: -Sl option usage is -Sl<low/high>\n");
						n_errors++;
					}
				}
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_out = 4 - (Ctrl->A.fixed_alt + Ctrl->A.fixed_time);	/* Minimum input columns (could be more) */
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_out;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0, 
			"Syntax error: Binary input data (-bi) must have at least %ld columns\n", n_out);
	n_errors += GMT_check_condition (GMT, Ctrl->CM4->CM4_F.active && Ctrl->CM4->CM4_L.curr, 
			"Syntax error: You cannot select both -F and -L options\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_mgd77magref_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_mgd77magref (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG i, j, s, nval = 0, nfval = 0, error = 0, t_col = 3, n_out = 0, cm4_igrf_T = FALSE;
	GMT_LONG lval = 0, lfval = 0, n_field_components, n_alloc = 0, need = 0, tbl;

	double the_altitude, the_time, *time_array = NULL, *alt_array = NULL, *time_years = NULL, IGRF[7], out[GMT_MAX_COLUMNS];
	double *igrf_xyz = NULL;	/* Temporary storage for the joint_IGRF_CM4 case */

	struct MGD77_CONTROL M;
	struct MGD77MAGREF_CTRL *Ctrl = NULL;
	struct GMT_DATASET *Din = NULL;
	struct GMT_TABLE *T = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (options && options->option == '?') return (GMT_mgd77magref_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_mgd77magref_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_mgd77magref", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vfb", "Hm", options)) Return (API->error);
	Ctrl = New_mgd77magref_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	MGD77_CM4_init (GMT, &M, Ctrl->CM4);	/* Presets path using strdup */
	if ((error = GMT_mgd77magref_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the mgd77magref main code ----------------------------*/

	GMT_get_time_system (GMT, "unix", &(GMT->current.setting.time_system));			/* MGD77+ uses GMT's Unix time epoch */
	GMT_init_time_system_structure (GMT, &(GMT->current.setting.time_system));
	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */

	Ctrl->CM4->CM4_D.dst = calloc((size_t)(1), sizeof(double));	/* We need at least a size of one in case a value is given in input */
	GMT->current.io.col_type[GMT_IN][t_col] = GMT->current.io.col_type[GMT_OUT][t_col] = GMT_IS_ABSTIME;	/* By default, time is in 4th input column */

	/* Shorthand for these */
	nval = Ctrl->CM4->CM4_F.n_field_components;
	nfval = Ctrl->CM4->CM4_F.n_field_sources;
	lval = Ctrl->CM4->CM4_L.n_curr_components;
	lfval = Ctrl->CM4->CM4_L.n_curr_sources;

	if (nfval && Ctrl->do_IGRF) GMT_message (GMT, "Warning. Source fields other than IGRF will be ignored. It's in the manual\n");

	/* ------------- Test, and take measures, if mix mode IGRF/CM4 is to be used -------------------------- */
	if (Ctrl->joint_IGRF_CM4) {
		if (nfval == 0) {	/* It means we had a -F.../9 which is exactly iqual to -F.../0 */
			Ctrl->do_IGRF = TRUE;
			Ctrl->do_CM4  = FALSE;
			Ctrl->joint_IGRF_CM4 = FALSE;
		}
		else {
			cm4_igrf_T = FALSE;
			if ( (nval == 1) && (Ctrl->CM4->CM4_F.field_components[0] == 0) ) {
				for (i = 0; i < 3; i++) Ctrl->CM4->CM4_F.field_components[i] = (int)i+2;	/* Force the x,y,z request */
				cm4_igrf_T = TRUE;
			}
			else if ( !((nval == 3) && (Ctrl->CM4->CM4_F.field_components[0] == 2) && (Ctrl->CM4->CM4_F.field_components[1] == 3) && 
						(Ctrl->CM4->CM4_F.field_components[2] == 4)) ) {
				GMT_report (GMT, GMT_MSG_FATAL, "GMT ERROR. In mix CM4/IGRF mode -F option can oly be -Ft[r]/... or -Fxyz[r]/...\n");
				error++;
			}

			nval = 3;
			Ctrl->CM4->CM4_F.n_field_components = (int)nval;
		}
	}
	/* ----------------------------------------------------------------------------------------------------- */

	if (error) Return (EXIT_FAILURE);

	if (!Ctrl->CM4->CM4_F.active && !Ctrl->CM4->CM4_L.curr) Ctrl->CM4->CM4_F.active = TRUE;

	/* Sort the order in which the parameters appear */
	if (Ctrl->CM4->CM4_F.active) {
		if (nval == 0) {		/* No components selected, default used */
			Ctrl->copy_input = TRUE;
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
			Ctrl->copy_input = TRUE;
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
		time_array = &Ctrl->A.time;
		Ctrl->CM4->CM4_DATA.n_times = 1;
	}
	else	/* Make sure input time columns are encoded/decoded properly since here we know t_col is set. */
		GMT->current.io.col_type[GMT_IN][t_col] = GMT->current.io.col_type[GMT_OUT][t_col] = (Ctrl->A.years) ? GMT_IS_FLOAT : GMT_IS_ABSTIME;

	GMT->current.io.col_type[GMT_IN][t_col+1] = GMT->current.io.col_type[GMT_OUT][t_col+1] = GMT_IS_FLOAT;		/* Override any previous t_col = 3 settings */
	if (!Ctrl->copy_input) GMT->current.io.col_type[GMT_OUT][2] = GMT->current.io.col_type[GMT_OUT][3] = GMT_IS_FLOAT;	/* No time on output */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options)) Return ((int)API->error);	/* Registers default input sources, unless already set */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options)) Return ((int)API->error);	/* Registers default output destination, unless already set */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET)) Return ((int)API->error);			/* Enables data input and sets access mode */

	if ((Din = GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, NULL)) == NULL) Return (API->error);
	n_out = n_field_components + ((Ctrl->copy_input) ? Din->n_columns : 0);
	if (cm4_igrf_T) n_out -= 2;	/* Decrease by 2 because the x,y,z were imposed internaly only. i.e not for output */
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) Return ((int)error);
	if (GMT_End_IO (API, GMT_IN, 0)) Return ((int)API->error);	/* Disables further data input */

	if (GMT->common.b.active[GMT_OUT] && GMT->common.b.ncol[GMT_OUT] > 0 && n_out > GMT->common.b.ncol[GMT_OUT]) {
		GMT_report (GMT, GMT_MSG_FATAL, "Binary output must have at least %ld columns (your -bo option only set %ld)\n", n_out, GMT->common.b.ncol[GMT_OUT]);
		Return (EXIT_FAILURE);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC)) Return ((int)API->error);		/* Enables data output and sets access mode */

	for (tbl = 0; tbl < Din->n_tables; tbl++) {	/* Loop over all input tables */
		T = Din->table[tbl];	/* Current table */

		for (s = 0; s < T->n_segments; s++) {	/* Process each file segment separately */
			GMT_Put_Record (API, GMT_WRITE_SEGHEADER, T->segment[s]->header);
			need = T->segment[s]->n_rows;	/* Size of output array needed in MGD77_cm4field */
			if (need > n_alloc) {	/* Need to reallocate */
				n_alloc = need;
				Ctrl->CM4->CM4_DATA.out_field = GMT_memory (GMT, Ctrl->CM4->CM4_DATA.out_field, n_alloc * n_field_components, double);
				if (!(Ctrl->A.years || Ctrl->A.fixed_time))
					time_years = GMT_memory (GMT, time_years, n_alloc, double);

				if (Ctrl->joint_IGRF_CM4)
					igrf_xyz = GMT_memory (GMT, igrf_xyz, n_alloc * 3, double);
			}

			if (!Ctrl->A.fixed_alt) {	/* Assign the alt_array to the provided altitude array */
				alt_array = T->segment[s]->coord[GMT_Z];
				Ctrl->CM4->CM4_DATA.n_altitudes = (int)T->segment[s]->n_rows;
			}

			if (!Ctrl->A.fixed_time) {	/* Assign the time_array to the provided time array */
				Ctrl->CM4->CM4_DATA.n_times = (int)T->segment[s]->n_rows;
				if (Ctrl->A.years)
					time_array = T->segment[s]->coord[t_col];
				else {	/* Must convert internal GMT time to decimal years first */
					for (i = 0; i < T->segment[s]->n_rows; i++)
						time_years[i] = MGD77_time_to_fyear (GMT, &M, T->segment[s]->coord[t_col][i]);
					time_array = time_years;
				}
			}

			if (!(Ctrl->do_IGRF || Ctrl->joint_IGRF_CM4 ) && !s && time_array[0] > 2002.7) {	/* Only atmospheric terms may be reliable */
				GMT_message (GMT, "Warning: Time is outside the CM4 strict validity domain [1960-2002.7].\n");
				GMT_message (GMT, "\tThough extended here to 2009 the secular variation estimation will be unreliable.\n");
			}

			Ctrl->CM4->CM4_DATA.n_pts = (int)T->segment[s]->n_rows;
			if (Ctrl->do_IGRF || Ctrl->joint_IGRF_CM4) {
				int type;
				type = (Ctrl->CM4->CM4_G.geodetic) ? 1 : 2;
				for (i = 0; i < T->segment[s]->n_rows; i++) {
					the_altitude = (Ctrl->A.fixed_alt) ? alt_array[0] : alt_array[i];
					the_time = (Ctrl->A.fixed_time) ? time_array[0] : time_array[i];
					if (type == 2) the_altitude += 6371.2;
					MGD77_igrf10syn (GMT, 0, the_time, type, the_altitude, T->segment[s]->coord[GMT_X][i],
							T->segment[s]->coord[GMT_Y][i], IGRF);
					if (!Ctrl->joint_IGRF_CM4) {		/* IGRF only */
						for (j = 0; j < Ctrl->CM4->CM4_F.n_field_components; j++)
							Ctrl->CM4->CM4_DATA.out_field[i*n_field_components+j] = IGRF[Ctrl->CM4->CM4_F.field_components[j]];
					}
					else {				/* Store the IGRF x,y,z components for later use */
						for (j = 0; j < 3; j++)
							igrf_xyz[i*3+j] = IGRF[Ctrl->CM4->CM4_F.field_components[j]];
					}
				}
			}

			if (Ctrl->do_CM4) 				/* DO CM4 only. Eval CM4 at all points */
				MGD77_cm4field (GMT, Ctrl->CM4, T->segment[s]->coord[GMT_X], T->segment[s]->coord[GMT_Y], alt_array, time_array);

			if ((Ctrl->do_CM4 || Ctrl->do_IGRF) && !Ctrl->joint_IGRF_CM4) {	/* DID CM4 or (exclusive) IGRF only. */
				for (i = 0; i < T->segment[s]->n_rows; i++) {	/* Output the requested columns */
					n_out = 0;
					if (Ctrl->copy_input) for (j = 0; j < T->segment[s]->n_columns; j++) out[n_out++] = T->segment[s]->coord[j][i];
					for (j = 0; j < n_field_components; j++)
						out[n_out++] = Ctrl->CM4->CM4_DATA.out_field[i*n_field_components+j];

					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
				}
			}
			else {					/* DID CM4 and IGRF */
				double x, y, z;
				for (i = 0; i < T->segment[s]->n_rows; i++) {	/* Output the requested columns */
					n_out = 0;
					if (Ctrl->copy_input) for (j = 0; j < T->segment[s]->n_columns; j++) out[n_out++] = T->segment[s]->coord[j][i];
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

					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
				}
			}

		}
	}

	GMT_free (GMT, Ctrl->CM4->CM4_DATA.out_field);
	if (!(Ctrl->A.years || Ctrl->A.fixed_time)) GMT_free (GMT, time_years);
	if (Ctrl->joint_IGRF_CM4) GMT_free (GMT, igrf_xyz);

	MGD77_end (GMT, &M);

	Return (EXIT_SUCCESS);
}
