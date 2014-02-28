/*--------------------------------------------------------------------
 *	$Id$
 *
 *    Copyright (c) 2009-2014 by J. Luis and P. Wessel
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

#define THIS_MODULE_NAME	"mgd77magref"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Evaluate the IGRF or CM4 magnetic field models"

#include "gmt_dev.h"
#include "mgd77.h"

#define GMT_PROG_OPTIONS "-Vbh" GMT_OPT("Hm")

struct MGD77MAGREF_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct MGD77_CM4 *CM4;
	bool copy_input;
	bool do_IGRF;
	bool do_CM4;
	bool joint_IGRF_CM4;
	struct MGD77_MAGREF_A {	/* -A */
		bool active;
		bool fixed_alt;
		bool fixed_time;
		int years;
		int t_col;
		double altitude;
		double time;
	} A;
	struct MGD77_MAGREF_C {	/* -C */
		bool active;
	} C;
	struct MGD77_MAGREF_D {	/* -D */
		bool active;
	} D;
	struct MGD77_MAGREF_F {	/* -F */
		bool active;
	} F;
	struct MGD77_MAGREF_G {	/* -G */
		bool active;
	} G;
	struct MGD77_MAGREF_L {	/* -L */
		bool active;
	} L;
	struct MGD77_MAGREF_S {	/* -S */
		bool active;
	} S;
};

void *New_mgd77magref_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77MAGREF_CTRL *C = NULL;

	C = GMT_memory (GMT, NULL, 1, struct MGD77MAGREF_CTRL);
	C->CM4 = calloc (1U, sizeof (struct MGD77_CM4));

	/* Initialize values whose defaults are not 0/false/NULL */

	C->do_CM4 = true;
	return (C);
}

void Free_mgd77magref_Ctrl (struct GMT_CTRL *GMT, struct MGD77MAGREF_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	free (C->CM4);
	GMT_free (GMT, C);
}

int GMT_mgd77magref_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mgd77magref [<table>] [-A+y+a<alt>+t<date>] [-C<cm4file>] [-D<dstfile>] [-E<f107file>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F<rthxyzdi[/[0|9]1234567]>] [-G] [-L<rtxyz[/1234]>] [-Sc|l<low>/<high>] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\n", GMT_b_OPT, GMT_h_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> contains records that must contain lon, lat, alt, time[, other cols].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   longitude and latitude is the geocentric position on the ellipsoid [but see -G].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   alt is the altitude in km positive above the ellipsoid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   time is the time of data aquisition, in <date>T<clock> format (but see -A+y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   We read <stdin> if no input file is given.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Adjust how the input records are interpreted. Append\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a<alt> to indicate a constant altitude [Default is 3rd column].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +t<time> to indicate a constant time [Default is 4th column].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +y to indicate times are given in decimal years [Default is ISO <date>T<clock> format].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Select an alternate file with coefficients for the CM4 model [%s/umdl.CM4].\n",
		API->GMT->session.SHAREDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t-D Select an alternate file with hourly means of the Dst index for CM4 [%s/Dst_all.wdc],\n",
		API->GMT->session.SHAREDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t   OR a single Dst index to apply for all records.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Select an alternate file with monthly means of absolute F10.7 solar radio flux for CM4 [%s/F107_mon.plt],\n",
		API->GMT->session.SHAREDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t   OR a single solar radio flux to apply for all records.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Dataflags is a string made up of 1 or more of these characters:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 r means output all input columns before adding the items below (all in nTesla).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 t means list total field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 h means list horizontal field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 x means list X component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 y means list Y component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 z means list Z component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 d means list declination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 i means list inclination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a number to indicate the requested field contribution(s):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 0 means Core field from IGRF only (no CM4 evalution).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 1 means Core field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 2 means Lithospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 3 Primary Magnetospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 4 Induced Magnetospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 5 Primary ionospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 6 Induced ionospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 7 Toroidal field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 9 means Core field from IGRF and other contributions from CM4. DO NOT USE BOTH 1 AND 9.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append several numbers to add up the different contributions. For example,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Ft/12 computes the total field due to CM4 Core and Lithospheric sources.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Two special cases are allowed which mix which Core field from IGRF and other sources from CM4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Ft/934 computes Core field due to IGRF plus terms 3 and 4 from CM4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Fxyz/934 the same as above but output the field components.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 The data is written out in the order specified in <dataflags>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 [Default is -Frthxyzdi/1]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify that coordinates are geocentric [geodetic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Compute J field vectors from certain external sources.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Dataflags is a string made up of 1 or more of these characters:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 r means output all input columns before adding the items below (all in Ampers/m).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 t means list magnitude field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 x means list X component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 y means list Y component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 z means list Z or current function Psi.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a number to indicate the requested J contribution(s)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 1 means Induced Magnetospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 2 means Primary ionospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 3 means Induced ionospheric field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t	 4 means Poloidal field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Limit the CM4 contributions from core and lithosphere to certain harmonic degree bands.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append c(ore) or l(ithosphere) and the low and high degrees to use [-Sc1/13 -Sl14/65].\n");
	GMT_Option (API, "V,bi0");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 4 input columns (unless -A is used).  Note for binary input, absolute time must\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   be in the unix time-system (unless -A+y is used).\n");
	GMT_Option (API, "bo,h,:,.");

	return (EXIT_FAILURE);
}

int GMT_mgd77magref_parse (struct GMT_CTRL *GMT, struct MGD77MAGREF_CTRL *Ctrl, struct GMT_OPTION *options)
{
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
				Ctrl->A.active = true;
				pos = 0;
				while ((GMT_strtok (opt->arg, "+", &pos, p))) {
					switch (p[0]) {
						case 'a':
							Ctrl->A.fixed_alt = true;
							Ctrl->A.altitude = atof (&p[1]);
							break;
						case 't':
							Ctrl->A.fixed_time = true;
							strncpy (tfixed, &p[1], GMT_LEN64);
							GMT->current.io.col_type[GMT_OUT][3] = GMT_IS_FLOAT;
							break;
						case 'y':
							Ctrl->A.years = true;
							GMT->current.io.col_type[GMT_IN][2] = GMT->current.io.col_type[GMT_OUT][2] = 
												GMT->current.io.col_type[GMT_IN][3] = 
												GMT->current.io.col_type[GMT_OUT][3] = 
												GMT_IS_FLOAT;
							break;
						default:
							break;
					}
				}
				if (Ctrl->A.fixed_time) {
					if (Ctrl->A.years)
						Ctrl->A.time = atof (tfixed);
					else
						GMT_scanf_arg (GMT, tfixed, GMT_IS_ABSTIME, &Ctrl->A.time);
				}
				break;
			case 'C':	/* Alternate CM4 coefficient file */
				Ctrl->C.active = true;
				free (Ctrl->CM4->CM4_M.path);
				Ctrl->CM4->CM4_M.path = strdup (opt->arg);
				break;
			case 'D':
				j = 0;
				if (opt->arg[j] == '-') j++;
				if ((opt->arg[j] > 47) && (opt->arg[j] < 58)) {	/* arg is numeric -> Dst Index */
					Ctrl->CM4->CM4_D.dst[0] = atof (&opt->arg[j]);
					Ctrl->CM4->CM4_D.index = false;
				}
				else {
					free (Ctrl->CM4->CM4_D.path);
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
					free (Ctrl->CM4->CM4_I.path);
					Ctrl->CM4->CM4_I.path = strdup (opt->arg);
					Ctrl->CM4->CM4_I.load = true;
				}
				break;
			case 'F':
				Ctrl->CM4->CM4_F.active = true;

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
				Ctrl->CM4->CM4_G.geodetic = false;
				break;
			case 'L':
				Ctrl->CM4->CM4_L.curr = true;

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
				if (opt->arg[0] == 'c') {
					j = sscanf (&opt->arg[1], "%d/%d", &Ctrl->CM4->CM4_S.nlmf[0], &Ctrl->CM4->CM4_S.nhmf[0]);
					if (j != 2) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: -Sc option usage is -Sc<low/high>\n");
						n_errors++;
					}
				}
				if (opt->arg[0] == 'l') {
					j = sscanf (&opt->arg[1], "%d/%d", &Ctrl->CM4->CM4_S.nlmf[1], &Ctrl->CM4->CM4_S.nhmf[1]);
					if (j != 2) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: -Sl option usage is -Sl<low/high>\n");
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
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0)
		GMT->common.b.ncol[GMT_IN] = n_out;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0, 
			"Syntax error: Binary input data (-bi) must have at least %d columns\n", n_out);
	n_errors += GMT_check_condition (GMT, Ctrl->CM4->CM4_F.active && Ctrl->CM4->CM4_L.curr, 
			"Syntax error: You cannot select both -F and -L options\n");
	n_errors += GMT_check_condition (GMT, (do_CM4core && Ctrl->do_IGRF) || (do_CM4core && Ctrl->joint_IGRF_CM4),
			"Syntax error: You cannot select both CM4 core (1) and IGRF as they are both core fields.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {free(Ctrl->CM4->CM4_M.path); free(Ctrl->CM4->CM4_D.path); free(Ctrl->CM4->CM4_I.path); \
	Free_mgd77magref_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mgd77magref (void *V_API, int mode, void *args)
{
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
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_mgd77magref_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_mgd77magref_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_mgd77magref_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_mgd77magref_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */
	MGD77_CM4_init (GMT, &M, Ctrl->CM4);	/* Presets path using strdup */
	if ((error = GMT_mgd77magref_parse (GMT, Ctrl, options))) {
		MGD77_end (GMT, &M);
		Return (error);
	}

	/*---------------------------- This is the mgd77magref main code ----------------------------*/

	n_in = 4 - (Ctrl->A.fixed_alt + Ctrl->A.fixed_time);

	if (Ctrl->A.fixed_alt) t_col = 2;	/* Since we are missing the altitude column */

	Ctrl->CM4->CM4_D.dst = calloc (1U, sizeof(double));	/* We need at least a size of one in case a value is given in input */
	GMT->current.io.col_type[GMT_IN][t_col] = GMT->current.io.col_type[GMT_OUT][t_col] = GMT_IS_ABSTIME;	/* By default, time is in 4th input column */

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
			else if ( !((nval == 3) && (Ctrl->CM4->CM4_F.field_components[0] == 2) && (Ctrl->CM4->CM4_F.field_components[1] == 3) && 
						(Ctrl->CM4->CM4_F.field_components[2] == 4)) ) {
				GMT_Report (API, GMT_MSG_NORMAL, "GMT ERROR. In mix CM4/IGRF mode -F option can oly be -Ft[r]/... or -Fxyz[r]/...\n");
				error++;
			}

			nval = 3;
			Ctrl->CM4->CM4_F.n_field_components = (int)nval;
		}
	}
	/* ----------------------------------------------------------------------------------------------------- */

	if (error) Return (EXIT_FAILURE);

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
		GMT->current.io.col_type[GMT_IN][t_col] = GMT->current.io.col_type[GMT_OUT][t_col] = (Ctrl->A.years) ? GMT_IS_FLOAT : GMT_IS_ABSTIME;

	GMT->current.io.col_type[GMT_IN][t_col+1] = GMT->current.io.col_type[GMT_OUT][t_col+1] = GMT_IS_FLOAT;		/* Override any previous t_col = 3 settings */
	if (!Ctrl->copy_input) GMT->current.io.col_type[GMT_OUT][2] = GMT->current.io.col_type[GMT_OUT][3] = GMT_IS_FLOAT;	/* No time on output */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}

	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	n_out = n_field_components + ((Ctrl->copy_input) ? (unsigned int)Din->n_columns : 0);
	if (cm4_igrf_T) n_out -= 2;	/* Decrease by 2 because the x,y,z were imposed internaly only. i.e not for output */
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_out)) != GMT_OK) {
		Return (error);
	}

	if (GMT->common.b.active[GMT_OUT] && GMT->common.b.ncol[GMT_OUT] > 0 && n_out > GMT->common.b.ncol[GMT_OUT]) {
		GMT_Report (API, GMT_MSG_NORMAL, "Binary output must have at least %d columns (your -bo option only set %d)\n", n_out, GMT->common.b.ncol[GMT_OUT]);
		Return (EXIT_FAILURE);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	for (tbl = 0; tbl < Din->n_tables; tbl++) {	/* Loop over all input tables */
		T = Din->table[tbl];	/* Current table */

		if (T->n_columns < n_in) {
			GMT_Report (API, GMT_MSG_NORMAL, "Table %d has %d columns, but from the used options we expect %d\n",
				tbl + 1, T->n_columns, n_in);
			continue;
		}

		for (s = 0; s < T->n_segments; s++) {	/* Process each file segment separately */
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, T->segment[s]->header);
			need = T->segment[s]->n_rows;   /* Size of output array needed in MGD77_cm4field */
			if (need > n_alloc) {           /* Need to reallocate */
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
				GMT_Message (API, GMT_TIME_NONE, "Warning: Time is outside the CM4 strict validity domain [1960.0-2002.7].\n");
				GMT_Message (API, GMT_TIME_NONE, "\tThe secular variation estimation will be unreliable. In this"
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
					MGD77_igrf10syn (GMT, 0, the_time, type, the_altitude, T->segment[s]->coord[GMT_X][i],
							T->segment[s]->coord[GMT_Y][i], IGRF);
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
				if ((err = MGD77_cm4field (GMT, Ctrl->CM4, T->segment[s]->coord[GMT_X],
							T->segment[s]->coord[GMT_Y], alt_array, time_array))) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error: this segment has a record generating an error.\n"
						"Unfortunately, this means all other eventually good\n"
						"records are also ignored. Fix the bad record and rerun the command.\n");
					continue;
				}
			}

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
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	free(Ctrl->CM4->CM4_D.dst);
	GMT_free (GMT, Ctrl->CM4->CM4_DATA.out_field);
	if (!(Ctrl->A.years || Ctrl->A.fixed_time)) GMT_free (GMT, time_years);
	if (Ctrl->joint_IGRF_CM4) GMT_free (GMT, igrf_xyz);

	MGD77_end (GMT, &M);

	Return (EXIT_SUCCESS);
}
