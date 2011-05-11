/*--------------------------------------------------------------------
 *	$Id: mgd77list_func.c,v 1.11 2011-05-11 04:01:54 guru Exp $
 *
 *    Copyright (c) 2004-2011 by P. Wessel
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mgd77list produces ASCII listings of <ngdc-id>.mgd77 files. The *.mgd77
 * files distributed from NGDC contain along-track geophysical observations
 * such as bathymetry, gravity, and magnetics, and the user may extract
 * any combination of these parameters as well as four generated quantities
 * such as distance (in km), heading, velocity (m/s), and weight by using
 * the -F option.  The order of the choices given to the -F option is used
 * to determine the sequence in which the parameters will be printed out.
 * If -F is not specified, the default will output all the data.  E.g. to
 * create an input file for surface, use -Flon,lat,depth (for bathymetry).
 *
 * To select a sub-section of the track, specify the start/endpoints by:
 *	1) Start-time (yyyy-mm-ddT[hh:mm:ss]) OR start-distance (km)
 *	2) Stop-time  (yyyy-mm-ddT[hh:mm:ss]) OR stop-distance (km)
 * To select data inside an area, use the -R option.
 * To start output with a header string, use -H.
 * To separate each data set with a segment header string, use -m.
 *
 * Author:	Paul Wessel
 * Date:	19-JUN-2004
 * Version:	1.0 Based somewhat on the old gmtlist.c
 *		31-MAR-2006: Changed -X to -L to avoid GMT collision
 *		23-MAY-2006: Added -Q for limits on speed/azimuths
 *		21-FEB-2008: Added -Ga|b<rec> for limits on rec range
 *
 *
 */
 
#include "gmt_mgd77.h"
#include "mgd77.h"

#define MGD77_ALL "id,time,lat,lon,ptc,twt,depth,bcc,btc,mtf1,mtf2,mag,msens,diur,msd,gobs,eot,faa,nqc,sln,sspn"
#define MGD77_GEO "time,lat,lon,twt,depth,mtf1,mtf2,mag,gobs,faa"
#define MGD77_AUX "dist,azim,vel,weight"

#define ADJ_CT	0
#define ADJ_DP	1
#define ADJ_GR	2
#define ADJ_MG	3

#define N_D	0	/* These are indices for -N subsets */
#define N_S	1
#define Q_A	0	/* These are indices for -Q subsets */
#define Q_V	1

struct MGD77LIST_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG code[4];
		GMT_LONG force;
		GMT_LONG GF_version;
		GMT_LONG fake_times;
		double sound_speed;
	} A;
	struct C {	/* -C */
		GMT_LONG active;
		GMT_LONG mode;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		GMT_LONG mode;	/* TRUE to skip recs with time == NaN */
		double start;	/* Start time */
		double stop;	/* Stop time */
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
		char *flags;
	} F;
	struct G {	/* -G */
		GMT_LONG active;
		GMT_LONG start;	/* Start rec */
		GMT_LONG stop;	/* Stop rec */
	} G;
	struct I {	/* -I */
		GMT_LONG active;
		GMT_LONG n;
		char code[3];
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		char *file;
	} L;
	struct N {	/* -N */
		GMT_LONG active[2];
		char unit[2][2];
	} N;
	struct Q {	/* -Q */
		GMT_LONG active[2];
		double min[2];
		double max[2];
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		double start;	/* Start dist */
		double stop;	/* Stop dist */
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		GMT_LONG mode;
	} T;
	struct W {	/* -W */
		GMT_LONG active;
		double value;
	} W;
	struct Z {	/* -Z[-|+] */
		GMT_LONG active;
		GMT_LONG mode;
	} Z;
};

void *New_mgd77list_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77LIST_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MGD77LIST_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->A.GF_version = MGD77_NOT_SET;
	C->C.mode = 2;
	C->D.stop = C->S.stop = DBL_MAX;	/* No upper limit on time and distance */
	C->G.stop = INT_MAX;		/* No limit on stop record */
	C->N.unit[N_D][0] = 'k';	/* Default is -Ndk */
	C->N.unit[N_S][0] = GMT_MAP_DIST_UNIT;	/* Default is -Nse */
	C->Q.max[Q_V] = DBL_MAX;	/* No upper speed limit */
	C->Q.max[Q_A] = 360.0;		/* Max azimuth limit */
	C->T.mode = MGD77_NOT_SET;
	C->W.value = 1.0;	/* Default weight */	
	return ((void *)C);
}

void Free_mgd77list_Ctrl (struct GMT_CTRL *GMT, struct MGD77LIST_CTRL *C) {	/* Deallocate control structure */
	if (C->F.flags) free ((void *)C->F.flags);
	if (C->L.file) free ((void *)C->L.file);
	GMT_free (GMT, C);	
}

GMT_LONG GMT_mgd77list_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT,"mgd77list %s - Extract data from MGD77 files\n\n", MGD77_VERSION);
	GMT_message (GMT,"usage: mgd77list <cruise(s)> -F<dataflags>[,<tests>] [-A[+]c|d|f|m|t[code]] [-Cf|g|e] [-Da<startdate>] [-Db<stopdate>] [-E]\n");
	GMT_message (GMT, "\t[-Ga<startrec>] [-Gb<stoprec>] [-H] [-I<code>] [-L[<corrtable.txt>]] [-N[s|p][e|k|n|M]]] [-Qa|v<min>/<max>] [%s]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-Sa<startdist>[unit]] [-Sb<stopdist>[unit]] [-T[m|e]] [-V] [-W<Weight>] [-Z[+|-] [%s]\n\n", GMT_bo_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	MGD77_Cruise_Explain (GMT);
	GMT_message (GMT, "\t-F <dataflags> is a comma-separated string made up of one or more of these abbreviations\n");
	GMT_message (GMT, "\t   (for standard MGD77 files - use mgd77list to probe for other columns in MGD77+ files):\n");
	GMT_message (GMT, "\t   >Track information:\n");
	GMT_message (GMT, "\t     time:    Choose between Absolute time [default], Relative time, or fractioan year:\n");
	GMT_message (GMT, "\t       atime: Absolute time (formatted according to OUTPUT_DATE_FORMAT, OUTPUT_CLOCK_FORMAT)\n");
	GMT_message (GMT, "\t       rtime: Relative time (formatted according to D_FORMAT and TIME_SYSTEM (or TIME_EPOCH, TIME_UNIT))\n");
	GMT_message (GMT, "\t       ytime: Absolute time as decimal year (formatted according to D_FORMAT)\n");
	GMT_message (GMT, "\t       year:  Record year\n");
	GMT_message (GMT, "\t       month: Record month (1-12)\n");
	GMT_message (GMT, "\t       day :  Record day of month (1-31)\n");
	GMT_message (GMT, "\t       hour:  Record hour(0-23)\n");
	GMT_message (GMT, "\t       min:   Record minute (0-59)\n");
	GMT_message (GMT, "\t       sec:   Record second (0-60)\n");
	GMT_message (GMT, "\t     lon:     Longitude (formatted according to OUTPUT_DEGREE_FORMAT)\n");
	GMT_message (GMT, "\t     lat:     Latitude (formatted according to OUTPUT_DEGREE_FORMAT)\n");
	GMT_message (GMT, "\t     id:      Survey leg ID [TEXTSTRING]\n");
	GMT_message (GMT, "\t     ngdcid:  NGDC ID [TEXTSTRING]\n");
	GMT_message (GMT, "\t     dist:    Along-track distances (see -C for method and -N for units)\n");
	GMT_message (GMT, "\t     azim:    Track azimuth (Degrees east from north)\n");
	GMT_message (GMT, "\t     vel:     Ship velocity (m/s)\n");
	GMT_message (GMT, "\t   >Geophysical Observations:\n");
	GMT_message (GMT, "\t     twt:     Two-way traveltime (s)\n");
	GMT_message (GMT, "\t     depth:   Corrected bathymetry (m) [Also see -Z]\n");
	GMT_message (GMT, "\t     mtf1:    Magnetic Total Field Sensor 1 (gamma, nTesla)\n");
	GMT_message (GMT, "\t     mtf2:    Magnetic Total Field Sensor 2 (gamma, nTesla)\n");
	GMT_message (GMT, "\t     mag:     Magnetic residual anomaly (gamma, nTesla)\n");
	GMT_message (GMT, "\t     gobs:    Observed gravity (mGal)\n");
	GMT_message (GMT, "\t     faa:     Free-air gravity anomaly (mGal)\n");
	GMT_message (GMT, "\t   >Codes, Corrections, and Information:\n");
	GMT_message (GMT, "\t     ptc:     Position type code\n");
	GMT_message (GMT, "\t     bcc:     Bathymetric correction code\n");
	GMT_message (GMT, "\t     btc:     Bathymetric type code\n");
	GMT_message (GMT, "\t     carter:  Carter correction from twt\n");
	GMT_message (GMT, "\t     msens:   Magnetic sensor for residual field\n");
	GMT_message (GMT, "\t     msd:     Magnetic sensor depth/altitude (m)\n");
	GMT_message (GMT, "\t     diur:    Magnetic diurnal correction (gamma, nTesla)\n");
	GMT_message (GMT, "\t     igrf:    International Geomagnetic Reference Field (gamma, nTesla)\n");
#ifdef USE_CM4
	GMT_message (GMT, "\t     cm4:     Comprehensive Model CM4 Geomagnetic Reference Field (gamma, nTesla)\n");
#endif
	GMT_message (GMT, "\t     eot:     Eotvos correction (mGal)\n");
	GMT_message (GMT, "\t     ngrav:   IGF, or Theoretical (Normal) Gravity Field (mGal)\n");
	GMT_message (GMT, "\t     sln:     Seismic line number string [TEXTSTRING]\n");
	GMT_message (GMT, "\t     sspn:    Seismic shot point number string [TEXTSTRING]\n");
	GMT_message (GMT, "\t     weight:  Give weight specified in -W\n");
	GMT_message (GMT, "\t     nqc:     Navigation quality code\n");
	GMT_message (GMT, "\t  The data are written in the order specified in <dataflags>\n");
	GMT_message (GMT, "\t  Shortcut flags are:\n");
	GMT_message (GMT, "\t     all:     All the columns defined in the data set\n");
	GMT_message (GMT, "\t     mgd77:   The full set of all 27 columns in the MGD77 specification\n");
	GMT_message (GMT, "\t     geo:     time,lon,lat + the 7 geophysical observations\n");
	GMT_message (GMT, "\t    Append + to include the 4 derived quantities dist, azim, vel, and weight [see -W]\n");
	GMT_message (GMT, "\t    [Default is all]\n");
	GMT_message (GMT, "\t  Abbreviations in UPPER CASE will suppress records where any such column is NaN.\n");
	GMT_message (GMT, "\t  (Note that -E is a shorthand to set all abbreviations to upper case).\n");
	GMT_message (GMT, "\t  Optionally, append comma-separated logical tests that columns must pass to be output.\n");
	GMT_message (GMT, "\t  Format is <flag><OP><value>, where flag is any of the dataflags above, and <OP> is\n");
	GMT_message (GMT, "\t  one of the operators <, <=, =, >=, >, |, and !=.  <value> is the limit you are testing,\n");
	GMT_message (GMT, "\t  including NaN (with = and != only).  If <flag> is UPPERCASE the test MUST be passed;\n");
	GMT_message (GMT, "\t  else at least ONE of the tests must pass for output to take place.  When using operators\n");
	GMT_message (GMT, "\t  involving characters <, >, and |, put entire argument to -F in single quotes.\n");
	GMT_message (GMT, "\t  Finally, for MGD77+ files you may optionally append : followed by one or more comma-\n");
	GMT_message (GMT, "\t  separated -+|-<col> terms.  This compares specific bitflags for each listed column\n");
	GMT_message (GMT, "\t  + means bit must be 1, - means it must be 0.  All bit tests given must be passed.\n");
	GMT_message (GMT, "\t  By default, MGD77+ files with error bit flags will use the flags to suppress bad data.\n");
	GMT_message (GMT, "\t  Turn this behavior off by append : with no arguments.\n");
	GMT_message (GMT, "\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-A Adjust some data values before output. Append c|d|f|m|t to select field:\n");
	GMT_message (GMT, "\t   c<code>[,<v>] Adjust field carter. <v>, the sound velocity in water, is taken from\n");
	GMT_message (GMT, "\t     the MGD77 header (or 1500 if invalid); optionally append your <v> (in m/s)\n");
	GMT_message (GMT, "\t     Here, C(twt) is Carter correction, U(twt,v) is uncorrected depth (given <v>).\n");
	GMT_message (GMT, "\t     TC(z) is twt from inverse Carter correction, TU(z,v) is twt from uncorrected depth.\n");
	GMT_message (GMT, "\t       c1 return difference between U(twt,v) and depth [Default].\n");
	GMT_message (GMT, "\t       c2 return difference between U(twt,v) and Carter(twt).\n");
	GMT_message (GMT, "\t       c4 return difference between (uncorrected) depth and Carter (TU(depth,v)).\n");
	GMT_message (GMT, "\t       c8 return difference between U(TC(depth),v) and depth.\n");
	GMT_message (GMT, "\t   d<code>[,<v>] Adjust field depth. <v> is optional sound speed in water (m/s)\n");
	GMT_message (GMT, "\t       d1 return depth as stored in file [Default].\n");
	GMT_message (GMT, "\t       d2 return calculated uncorrected depth U(twt,v).\n");
	GMT_message (GMT, "\t       d4 return calculated corrected depth Carter (twt,v).\n");
	GMT_message (GMT, "\t   f<code>[,<field>] Adjust field faa. <field>, the IGF reference field, is taken\n");
	GMT_message (GMT, "\t     from the MGD77 header (or 4 if invalid); optionally append your <field> from\n");
	GMT_message (GMT, "\t     1 = Heiskanen 1924 formula:\n\t       ");
	MGD77_IGF_text (GMT, GMT->session.std[GMT_ERR], 1);
	GMT_message (GMT, "\t     2 = International 1930 formula:\n\t       ");
	MGD77_IGF_text (GMT, GMT->session.std[GMT_ERR], 2);
	GMT_message (GMT, "\t     3 = International 1967 formula:\n\t       ");
	MGD77_IGF_text (GMT, GMT->session.std[GMT_ERR], 3);
	GMT_message (GMT, "\t     4 = International 1980 formula:\n\t       ");
	MGD77_IGF_text (GMT, GMT->session.std[GMT_ERR], 4);
	GMT_message (GMT, "\t       f1 return faa as stored in file [Default].\n");
	GMT_message (GMT, "\t       f2 return difference gobs - ngrav.\n");
	GMT_message (GMT, "\t       f4 return difference gobs + eot - ngrav.\n");
	GMT_message (GMT, "\t   m<code> Adjust field mag.\n");
	GMT_message (GMT, "\t       m1 return mag as stored in file [Default].\n");
	GMT_message (GMT, "\t       m2 return difference mtfx - igrf, where x = msens (or 1 if undefined).\n");
	GMT_message (GMT, "\t       m4 return difference mtfx - igrf, where x != msens (or 2 if undefined).\n");
#ifdef USE_CM4
	GMT_message (GMT, "\t       m8 return difference mtfx - cm4, where x = msens (or 1 if undefined).\n");
	GMT_message (GMT, "\t       m16 return difference mtfx - cm4, where x != msens (or 2 if undefined).\n");
#endif
	GMT_message (GMT, "\t   t will compute fake times for cruises with known duration but lacking record times\n");
	GMT_message (GMT, "\t   The optional -A+ means selected anomalies will be recalculated even when the original\n");
	GMT_message (GMT, "\t   anomaly is NaN [Default honors NaNs in existing anomalies]\n");
	GMT_message (GMT, "\t-C Select procedure for along-track distance and azimuth calculations:\n");
	GMT_message (GMT, "\t   f Flat Earth\n");
	GMT_message (GMT, "\t   g Great circle [Default]\n");
	GMT_message (GMT, "\t   e Ellipsoidal (geodesic) using current ellipsoid\n");
	GMT_message (GMT, "\t-Da<date> lists from date (given as yyyy-mm-ddT[hh:mm:ss]) [Start of cruise]\n");
	GMT_message (GMT, "\t  b<date> lists up to date (given as yyyy-mm-ddT[hh:mm:ss]) [End of cruise]\n");
	GMT_message (GMT, "\t  If A|B is used instead or a|b then records with no time are excluded from output\n");
	GMT_message (GMT, "\t-E Only records that exactly matches the requested geophysical information in -F will be used.\n");
	GMT_message (GMT, "\t   [Default will output all record that matches at least one column]\n");
	GMT_message (GMT, "\t-Ga<rec> lists from given record [Start of cruise]\n");
	GMT_message (GMT, "\t  b<rec> lists up to given record [End of cruise]\n");
	GMT_message (GMT, "\t-H Write one header record with column names\n");
	GMT_message (GMT, "\t-I Ignore certain data file formats from consideration. Append combination of act to ignore\n");
	GMT_message (GMT, "\t   (a) MGD77 ASCII, (c) MGD77+ netCDF, or (t) plain table files. [Default ignores none]\n");
	GMT_message (GMT, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_message (GMT, "\t   the default file mgd77_corrections.txt in $MGD77_HOME is assumed.\n");
	GMT_message (GMT, "\t-N Append (d)istances or (s)peed, and your choice for unit. Choose among:\n");
	GMT_message (GMT, "\t   e Metric units I (meters, m/s)\n");
	GMT_message (GMT, "\t   k Metric units II (km, km/hr)\n");
	GMT_message (GMT, "\t   M British/US units (miles, miles/hr)\n");
	GMT_message (GMT, "\t   n Nautical units (nautical miles, knots)\n");
	GMT_message (GMT, "\t   [Default is -Ndk -Nse]\n");
	GMT_message (GMT, "\t-Q Only return data whose azimuth (-Qa) or velocity (-Qv) fall inside specified range:\n");
	GMT_message (GMT, "\t   -Qa<min_az>/<max_az>, where <min_az> < <max_az> [all azimuths, i.e. 0/360]\n");
	GMT_message (GMT, "\t   -Qv<min_vel>[/<max_vel>], where <max_vel> is optional [all velocities, i.e., 0/infinity]\n");
	GMT_message (GMT, "\t      Velocities are given in m/s unless changed by -Ns\n");
	GMT_message (GMT, "\t-R Only return data inside the specified region [0/360/-90/90]\n");
	GMT_message (GMT, "\t-Sa<dist> lists from dist  (in m; append k, m, or n) [Start of the cruise]\n");
	GMT_message (GMT, "\t-Sb<dist> lists up to dist (in m; append k, m, or n) [End of the cruise]\n");
	GMT_message (GMT, "\t-T turns OFF the otherwise automatic adjustment of values based on correction terms\n");
	GMT_message (GMT, "\t   stored in the mgd77+ file (option has no effect on plain MGD77 ASCII files).\n");
	GMT_message (GMT, "\t   Append m or e to indicate the MGD77 data set or the extended columns set. [Default is both]\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Sets weight for these data [1]\n");
	GMT_message (GMT, "\t-Z Append - to report bathymetry & msd as negative depths [Default is positive -Z+]\n");
	GMT_explain_options (GMT, "D0.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_mgd77list_parse (struct GMTAPI_CTRL *C, struct MGD77LIST_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to mgd77list and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, code;
	char *t = NULL, buffer[GMT_BUFSIZ];
	double dist_scale;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjustment flags */
				Ctrl->A.active = TRUE;
				k = 0;
				if (opt->arg[k] == '+') {	/* Recalculate anomalies even if original anomaly == NaN [Default leaves NaNs unchanged] */
					Ctrl->A.force = TRUE;
					k++;
				}
				switch (opt->arg[k]) {
					case 'c':	/* Carter correction adjustment */
						code = opt->arg[k+1] - '0';
						if (code < 1 || code > 11) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Ac<code>.  <code> must be 1,2,4,8 or binary combination.\n");
							n_errors++;
						}
						if (opt->arg[k+2] == ',') {
							Ctrl->A.sound_speed = atof (&opt->arg[k+3]);
							if (Ctrl->A.sound_speed < 1400.0 || Ctrl->A.sound_speed > 1600.0) {
								GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Ac<code>,<speed>.  <speed> in m/s in the 1400-1600 range.\n");
								n_errors++;
							}
						}
						Ctrl->A.code[ADJ_CT] |= code;
						break;
					case 'd':	/* depth adjustment */
						code = opt->arg[k+1] - '0';
						if (code < 1 || code > 7) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Ad<code>.  <code> must be 1,2,4 or binary combination.\n");
							n_errors++;
						}
						if (opt->arg[k+2] == ',') {
							Ctrl->A.sound_speed = atof (&opt->arg[k+3]);
							if (Ctrl->A.sound_speed < 1400.0 || Ctrl->A.sound_speed > 1600.0) {
								GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Ad<code>,<speed>.  <speed> in m/s in the 1400-1600 range.\n");
								n_errors++;
							}
						}
						Ctrl->A.code[ADJ_DP] |= code;
						break;
					case 'f':	/* faa adjustment */
						code = opt->arg[k+1] - '0';
						if (code < 1 || code > 3) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Af<code>.  <code> must be 1-3.\n");
							n_errors++;
						}
						if (opt->arg[k+2] == ',') {
							Ctrl->A.GF_version = atoi (&opt->arg[k+3]);
							if (Ctrl->A.GF_version < MGD77_IGF_HEISKANEN || Ctrl->A.GF_version > MGD77_IGF_1980) {
								GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Af<code>,<field>.  Select <field> is 1-4 range.\n");
								n_errors++;
							}
						}
						Ctrl->A.code[ADJ_GR] |= code;
						break;
					case 'm':	/* mag adjustment */
						code = atoi (&opt->arg[k+1]);
						if (code < 1 || code > 31) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Am<code>.  <code> must be 1,2,4 or binary combination.\n");
							n_errors++;
						}
						Ctrl->A.code[ADJ_MG] |= code;
						break;
					case 't':	/* fake time requires */
						Ctrl->A.fake_times = TRUE;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR -A<flag>.  <flag> must be c, d, g, m, or t.\n");
						n_errors++;
						break;
				}
				break;

			case 'C':	/* Distance calculation flag */
				Ctrl->C.active = TRUE;
				if (opt->arg[0] == 'f') Ctrl->C.mode = 1;
				if (opt->arg[0] == 'g') Ctrl->C.mode = 2;
				if (opt->arg[0] == 'e') Ctrl->C.mode = 3;
				if (Ctrl->C.mode < 1 || Ctrl->C.mode > 3) {
					GMT_report (GMT, GMT_MSG_FATAL, "ERROR -C: Flag must be f, g, or e\n");
					n_errors++;
				}
				break;

			case 'D':		/* Assign start/stop times for sub-section */
				Ctrl->D.active = TRUE;
				switch (opt->arg[0]) {
				 	case 'A':		/* Start date, skip records with time = NaN */
						Ctrl->D.mode = TRUE;
				 	case 'a':		/* Start date */
						t = &opt->arg[1];
						if (t && GMT_verify_expectations (GMT, GMT_IS_ABSTIME, GMT_scanf (GMT, t, GMT_IS_ABSTIME, &Ctrl->D.start), t)) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Da: Start time (%s) in wrong format\n", t);
							n_errors++;
						}
						break;
					case 'B':		/* Stop date, skip records with time = NaN */
						Ctrl->D.mode = TRUE;
					case 'b':		/* Stop date */
						t = &opt->arg[1];
						if (t && GMT_verify_expectations (GMT, GMT_IS_ABSTIME, GMT_scanf (GMT, t, GMT_IS_ABSTIME, &Ctrl->D.stop), t)) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Db : Stop time (%s) in wrong format\n", t);
							n_errors++;
						}
						break;
					default:
						n_errors++;
						break;
				}
				break;

			case 'E':	/* Exact parameter match */
				Ctrl->E.active = TRUE;
				break;

			case 'F':	/* Selected output fields */
				strcpy (buffer, opt->arg);
				if (!strcmp (buffer, "all")) strcpy (buffer, MGD77_ALL);
				if (!strcmp (buffer, "all+")) {
					strcpy (buffer, MGD77_ALL);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				if (!strcmp (buffer, "mgd77")) strcpy (buffer, MGD77_GEO);
				if (!strcmp (buffer, "mgd77+")) {
					strcpy (buffer, MGD77_GEO);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				Ctrl->F.flags = strdup (buffer);
				break;

			case 'G':		/* Assign start/stop records for sub-section */
				Ctrl->G.active = TRUE;
				switch (opt->arg[0]) {
				 	case 'a':		/* Start record */
						Ctrl->G.start = atoi (&opt->arg[1]);
						break;
					case 'b':		/* Stop record */
						Ctrl->G.stop = atoi (&opt->arg[1]);
						break;
					default:
						n_errors++;
						break;
				}
				break;

			case 'I':
				Ctrl->I.active = TRUE;
				if (Ctrl->I.n < 3) {
					if (strchr ("act", (int)opt->arg[0]))
						Ctrl->I.code[Ctrl->I.n++] = opt->arg[0];
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Option -I Bad modifier (%c). Use -Ia|c|t!\n", opt->arg[0]);
						n_errors++;
					}
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Option -I: Can only be applied 0-2 times\n");
					n_errors++;
				}
				break;

			case 'L':	/* Crossover correction table */
				Ctrl->L.file = strdup (opt->arg);
				Ctrl->L.active = TRUE;
				break;

			case 'N':	/* Nautical units (knots, nautical miles) */
#ifdef GMT_COMPAT
				if (opt->arg[1] == 'm') {
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning -N: Unit m for miles is deprecated; use unit M instead\n");
					opt->arg[1] = 'M';
				}
#endif
				switch (opt->arg[0]) {
					case 'd':	/* Distance unit selection */
						Ctrl->N.active[N_D] = TRUE;
						Ctrl->N.unit[N_D][0] = opt->arg[1];
						if (!strchr ("ekMn", (int)Ctrl->N.unit[N_D][0])) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Nd: Unit must be e, k, M, or n\n");
							n_errors++;
						}
						break;
					case 's':	/* Speed unit selection */
						Ctrl->N.active[N_S] = TRUE;
						Ctrl->N.unit[N_S][0] = opt->arg[1];
						if (!strchr ("ekMn", (int)Ctrl->N.unit[N_S][0])) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Nd: Unit must be e, k, M, or n\n");
							n_errors++;
						}
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR -N: Syntax is -Nd|s<unit>\n");
						n_errors++;
						break;
				}
				break;

			case 'Q':		/* Assign min/max values for speeds or azimuth */
				switch (opt->arg[0]) {
					case 'a':	/* Azimuth min/max */
						if (sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->Q.min[Q_A], &Ctrl->Q.max[Q_A]) != 2) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Qa: append min/max azimuth limits [0/360]\n");
							n_errors++;
						}
						Ctrl->Q.active[Q_A] = TRUE;
						break;
					case 'v':	/* Velocity min/max */
						code = sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->Q.min[Q_V], &Ctrl->Q.max[Q_V]);
						if (code == 1)
							Ctrl->Q.max[Q_V] = DBL_MAX;
						else if (code <= 0) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Qv: append min[/max] velocity limits [0]\n");
							n_errors++;
						}
						Ctrl->Q.active[Q_V] = TRUE;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR -Q: Syntax is -Qa|v<min>/<max>\n");
						n_errors++;
						break;
				}
				break;

			case 'S':		/* Assign start/stop position for sub-section (converted to meters) */
				Ctrl->S.active = TRUE;
				if (opt->arg[0] == 'a') {		/* Start position */
					MGD77_Set_Unit (GMT, &opt->arg[1], &dist_scale, 1);
					Ctrl->S.start = atof (&opt->arg[1]) * dist_scale;
				}
				else if (opt->arg[0] == 'b') {	/* Stop position */
					MGD77_Set_Unit (GMT, &opt->arg[1], &dist_scale, 1);
					Ctrl->S.stop = atof (&opt->arg[1]) * dist_scale;
				}
				else
					n_errors++;
				break;

			case 'T':	/* Disable automatic corrections */
				Ctrl->T.active = TRUE;
				switch (opt->arg[0]) {
					case '\0':	/* Both sets */
						Ctrl->T.mode = MGD77_NOT_SET;
						break;
					case 'm':	/* MGD77 set */
						Ctrl->T.mode = MGD77_M77_SET;
						break;
					case 'e':	/* extra CDF set */
						Ctrl->T.mode = MGD77_CDF_SET;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR -T: append m, e, or neither\n");
						n_errors++;
						break;
				}
				break;
			case 'W':		/* Assign a weight to these data */
				Ctrl->W.active = TRUE;
				Ctrl->W.value = (!strcmp (opt->arg, "NaN")) ? GMT->session.d_NaN : atof (opt->arg);
				break;

			case 'Z':		/* -Z- is negative down for depths */
				Ctrl->Z.active = TRUE;
				Ctrl->Z.mode = (opt->arg[0] == '-') ? TRUE : FALSE;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->D.start > 0.0 && Ctrl->S.start > 0.0, "Syntax error: Cannot specify both start time AND start distance\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.stop < DBL_MAX && Ctrl->S.stop < DBL_MAX, "Syntax error: Cannot specify both stop time AND stop distance\n");
	n_errors += GMT_check_condition (GMT, GMT->common.R.active && GMT_check_region (GMT, GMT->common.R.wesn), "Syntax error: Region set incorrectly\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.value <= 0.0, "Syntax error: -W weight must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.start > Ctrl->S.stop, "Syntax error -S: Start distance exceeds stop distance!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active[Q_A] && Ctrl->Q.min[Q_A] >= Ctrl->Q.max[Q_A], "Syntax error -Qa: Minimum azimuth equals or exceeds maximum azimuth!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active[Q_V] && (Ctrl->Q.min[Q_V] >= Ctrl->Q.max[Q_V] || Ctrl->Q.min[Q_V] < 0.0), "Syntax error -Qv: Minimum velocity equals or exceeds maximum velocity or is negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.start > Ctrl->D.stop, "Syntax error ERROR -D: Start time exceeds stop time!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG separate_aux_columns (struct MGD77_CONTROL *F, char *fx_setting, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist)
{
	GMT_LONG i, j, k, this_aux, n_aux;
	
	fx_setting[0] = '\0';
	for (i = k = n_aux = 0; i < F->n_out_columns; i++) {
		for (j = 0, this_aux = MGD77_NOT_SET; j < N_MGD77_AUX && this_aux == MGD77_NOT_SET; j++) if (!strcmp (auxlist[j].name, F->desired_column[i])) this_aux = j;
		if (this_aux == MGD77_NOT_SET) {	/* Just pass other columns through */
			if (k) strcat (fx_setting, ",");
			strcat (fx_setting, F->desired_column[i]);
			k++;
		}
		else
		{	/* Found a request for an auxillary column  */
			aux[n_aux].type = auxlist[this_aux].type;
			aux[n_aux].text = auxlist[this_aux].text;
			aux[n_aux].pos = k;
			auxlist[this_aux].requested = TRUE;
			n_aux++;
		}
	}
	return (n_aux);
}

GMT_LONG augment_aux_columns (int n_items, char **item_name, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist, int n_aux)
{
	/* This adds additional aux colums that are required by the correction table and not already requested by other means (e.g. -F) */
	GMT_LONG i, j, k, this_aux, n;
	
	for (i = k = 0, n = n_aux; i < n_items; i++) {
		for (j = 0, this_aux = MGD77_NOT_SET; j < N_MGD77_AUX && this_aux == MGD77_NOT_SET; j++) if (!strcmp (auxlist[j].name, item_name[i])) this_aux = j;
		if (this_aux != MGD77_NOT_SET && !auxlist[this_aux].requested) {	/* Found a request for an auxillary column not yet requested  */
			aux[n].type = auxlist[this_aux].type;
			aux[n].text = auxlist[this_aux].text;
			aux[n].pos = k;
			auxlist[this_aux].requested = TRUE;
			n++;
		}
	}
	return (n);
}

#define Return(code) {Free_mgd77list_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); GMT_exit (code);}

GMT_LONG GMT_mgd77list (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, c, id, k, kx, pos, argno, n_cruises = 0, n_paths, use, n_items = 0, t_pos = MGD77_NOT_SET;
	GMT_LONG t_col, x_col, y_col, z_col, e_col = 0, m_col = 0, f_col = 0, g_col = 0, m1_col = 0, m2_col = 0;
	GMT_LONG n_sub, n_out_columns, n_cols_to_process, n_aux, select_option, time_column, lon_column, lat_column;
	GMT_LONG rec, prevrec, n_out = 0, error = FALSE, string_output = FALSE, need_depth = FALSE, PDR_wrap;
	GMT_LONG negative_depth = FALSE,  negative_msd = FALSE, need_distances, need_time, ms_col = 0, twt_col = 0;
	GMT_LONG need_lonlat = FALSE, first_cruise = TRUE, need_twt = FALSE, this_limit_on_time;
	GMT_LONG need_date, need_sound = FALSE, lonlat_not_NaN, first_warning = TRUE, has_prev_twt = FALSE;
	
	char fx_setting[GMT_BUFSIZ], **list = NULL, **item_names = NULL;
	char *tvalue[MGD77_MAX_COLS], *aux_tvalue[N_MGD77_AUX];
	
	double IGRF[7], correction, prev_twt = 0, d_twt, twt_pdrwrap_corr, *out = NULL;
	double dist_scale, vel_scale, ds, dt, cumulative_dist, aux_dvalue[N_MGD77_AUX];
	double i_sound_speed = 0.0, date = 0.0, g, m, z, v, twt, *dvalue[MGD77_MAX_COLS];
	
	struct MGD77_CONTROL M;
	struct MGD77_DATASET *D = NULL;
	struct MGD77_AUX_INFO aux[N_MGD77_AUX];
	struct GMT_gcal cal;
	struct MGD77_CARTER Carter;
	struct MGD77_CORRTABLE **CORR = NULL;
	struct MGD77_AUXLIST auxlist[N_MGD77_AUX] = {
		{ "dist",    MGD77_AUX_DS, 0, 0, "d(km)"},
		{ "azim",    MGD77_AUX_AZ, 0, 0, "azimuth"},
		{ "vel",     MGD77_AUX_SP, 0, 0, "v(m/s)"},
		{ "year",    MGD77_AUX_YR, 0, 0, "year"},
		{ "month",   MGD77_AUX_MO, 0, 0, "month"},
		{ "day",     MGD77_AUX_DY, 0, 0, "day"},
		{ "hour",    MGD77_AUX_HR, 0, 0, "hour"},
		{ "min",     MGD77_AUX_MI, 0, 0, "minute"},
		{ "sec",     MGD77_AUX_SC, 0, 0, "second"},
		{ "weight",  MGD77_AUX_WT, 0, 0, "weight"},
		{ "drt",     MGD77_AUX_RT, 0, 0, "rectype"},
		{ "igrf",    MGD77_AUX_MG, 0, 0, "IGRF"},
		{ "carter",  MGD77_AUX_CT, 0, 0, "Carter"},
		{ "ngrav",   MGD77_AUX_GR, 0, 0, "IGF"},
		{ "ngdcid",  MGD77_AUX_ID, 1, 0, "NGDC-ID"}
#ifdef USE_CM4
	, { "cm4",  MGD77_AUX_CM, 0, 0, "CM4"}
#endif
	};
#ifdef USE_CM4
	struct MGD77_CM4 CM4;
#endif
	struct MGD77LIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (options && options->option == '?') return (GMT_mgd77list_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_mgd77list_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_mgd77list", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRb", "hm", options))) Return ((int)error);
	Ctrl = (struct MGD77LIST_CTRL *) New_mgd77list_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_mgd77list_parse (API, Ctrl, options))) Return ((int)error);
	
	/*---------------------------- This is the mgd77list main code ----------------------------*/

	/* Initialize MGD77 output order and other parameters*/
	
	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */
	if (Ctrl->I.active) MGD77_Process_Ignore (GMT, 'I', Ctrl->I.code);
	aux_dvalue[MGD77_AUX_WT] = Ctrl->W.value;			/* Default weight */
	aux_dvalue[MGD77_AUX_RT] = 5.0;					/* Default record type */
	if (!Ctrl->F.active) Ctrl->F.flags = strdup (MGD77_ALL);	/* Default is a full MGD77 record */
	negative_msd = negative_depth = Ctrl->Z.mode;			/* Follow the -Z option to start with */

	if (Ctrl->T.active) {	/* Turn off automatic corrections */
		if (Ctrl->T.mode == MGD77_NOT_SET)	/* Both sets */
			M.use_corrections[MGD77_M77_SET] = M.use_corrections[MGD77_CDF_SET] = FALSE;
		else if (Ctrl->T.mode == MGD77_M77_SET) /* MGD77 set */
			M.use_corrections[MGD77_M77_SET] = FALSE;
		else	/* extra CDF set */
			M.use_corrections[MGD77_CDF_SET] = FALSE;
	}
	
	/* Check that the options selected are mutually consistent */
	
	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (n_paths == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: No cruises given\n");
		Return (EXIT_FAILURE);
	}

	if (M.adjust_time) Ctrl->D.start = MGD77_time2utime (GMT, &M, Ctrl->D.start);	/* Convert to Unix time if need be */
	if (M.adjust_time) Ctrl->D.stop  = MGD77_time2utime (GMT, &M, Ctrl->D.stop);
	if (Ctrl->L.active) {	/* Scan the ephemeral correction table for needed auxilliary columns */
		char path[GMT_BUFSIZ];
		if (!Ctrl->L.file) {	/* Try default correction table */
			sprintf (path, "%s%cmgd77_corrections.txt", M.MGD77_HOME, DIR_DELIM);
			if (access (path, R_OK)) {
				GMT_report (GMT, GMT_MSG_FATAL, "No default MGD77 Correction table (%s) found!\n", path);
				Return (EXIT_FAILURE);
			}
			Ctrl->L.file = path;
		}
		n_items = MGD77_Scan_Corrtable (GMT, Ctrl->L.file, list, (int)n_paths, M.n_out_columns, (char **)M.desired_column, &item_names, 2);
	}
	
	select_option = MGD77_RESET_CONSTRAINT | MGD77_RESET_EXACT;	/* Make sure these start at zero */
	if (Ctrl->E.active) select_option |= MGD77_SET_ALLEXACT;			/* Sets all columns listed as "must be present" */
	MGD77_Select_Columns (GMT, Ctrl->F.flags, &M, (int)select_option);		/* This is the list of columns the user ultimately wants output */
	n_out_columns = M.n_out_columns;				/* This is the total number of columns in the final output */
	if (MGD77_Get_Column (GMT, "depth", &M) == MGD77_NOT_SET) negative_depth = FALSE;	/* Just so we don't accidently access dvalue[z_col] further down in the loop */
	if (MGD77_Get_Column (GMT, "msd", &M) == MGD77_NOT_SET) negative_msd = FALSE;	/* Just so we don't accidently access dvalue[m_col] further down in the loop */
	n_aux = separate_aux_columns (&M, fx_setting, aux, auxlist);				/* Determine which auxillary columns are requested (if any) */
	if (Ctrl->L.active) {
		n_aux = augment_aux_columns ((int)n_items, item_names, aux, auxlist, (int)n_aux);	/* Determine which auxillary columns are needed by -L */
		for (i = 0; i < n_items; i++) GMT_free (GMT, item_names[i]);
		if (n_items) GMT_free (GMT, item_names);
	}
	aux_tvalue[MGD77_AUX_ID] = GMT_memory (GMT, NULL, GMT_TEXT_LEN64, char);	/* Just in case */
	use = (M.original) ? MGD77_ORIG : MGD77_REVISED;
	
	/* Most auxillary columns depend on values in the data columns.  If the user did not specify the required data columns
	 * then we must append them to make sure we have access to the values we need to calculate the auxillary values.
	 * Also, so limit tests on data records (e.g., distances, region, or time) also implies the need for certain data
	 * columns such as time, lon, and lat.
	 */
	 
	need_distances = (Ctrl->S.active || auxlist[MGD77_AUX_SP].requested || auxlist[MGD77_AUX_DS].requested || auxlist[MGD77_AUX_AZ].requested);	/* Distance is requested */
	need_lonlat = (auxlist[MGD77_AUX_MG].requested || auxlist[MGD77_AUX_GR].requested || auxlist[MGD77_AUX_CT].requested || Ctrl->A.code[ADJ_MG] > 1 || Ctrl->A.code[ADJ_DP] & 4 || Ctrl->A.code[ADJ_CT] >= 2 || Ctrl->A.code[ADJ_GR] > 1 || Ctrl->A.fake_times);	/* Need lon, lat to calculate reference fields or Carter correction */
	need_time = (auxlist[MGD77_AUX_YR].requested || auxlist[MGD77_AUX_MO].requested || auxlist[MGD77_AUX_DY].requested || auxlist[MGD77_AUX_HR].requested || auxlist[MGD77_AUX_MI].requested || auxlist[MGD77_AUX_MG].requested || (Ctrl->A.code[ADJ_MG] > 1));
#ifdef USE_CM4
	if (auxlist[MGD77_AUX_CM].requested) need_lonlat = need_time = TRUE;
#endif

	n_sub = 0;	/* This value will hold the number of columns that we will NOT printout (they are only needed to calculate auxillary values) */
	if (need_distances || need_lonlat) {	/* Must make sure we get lon,lat if they are not already requested */
		 if (MGD77_Get_Column (GMT, "lat", &M) == MGD77_NOT_SET) strcat (fx_setting, ",lat"), n_sub++;	/* Append lat to requested list */
		 if (MGD77_Get_Column (GMT, "lon", &M) == MGD77_NOT_SET) strcat (fx_setting, ",lon"), n_sub++;	/* Append lon to requested list */
	}
	if ((Ctrl->D.active || need_time || auxlist[MGD77_AUX_SP].requested) && MGD77_Get_Column (GMT, "time", &M) == MGD77_NOT_SET) strcat (fx_setting, ",time"), n_sub++;	/* Append time to requested list */
	need_twt = (auxlist[MGD77_AUX_CT].requested || (Ctrl->A.code[ADJ_CT] > 0 && Ctrl->A.code[ADJ_CT] < 3) || (Ctrl->A.code[ADJ_DP] > 1));
	if (need_twt) {	/* Want to estimate Carter corrections */
		 if (MGD77_Get_Column (GMT, "twt", &M) == MGD77_NOT_SET) strcat (fx_setting, ",twt"), n_sub++;	/* Must append twt to requested list */
		MGD77_carter_init (GMT, &Carter);	/* Initialize Carter machinery */
	}
	need_depth = ((Ctrl->A.code[ADJ_CT] & (1 | 3 | 8)) || (Ctrl->A.code[ADJ_DP] & 1));
	if (need_depth) {	/* Need depth*/
		 if (MGD77_Get_Column (GMT, "depth", &M) == MGD77_NOT_SET) strcat (fx_setting, ",depth"), n_sub++;	/* Must append depth to requested list */
	}
	if (Ctrl->A.code[ADJ_GR] > 1) {	/* Need gobs */
		 if (MGD77_Get_Column (GMT, "gobs", &M) == MGD77_NOT_SET) strcat (fx_setting, ",gobs"), n_sub++;	/* Must append gobs to requested list */
	}
	if (Ctrl->A.code[ADJ_GR] == 3) {	/* Need eot */
		 if (MGD77_Get_Column (GMT, "eot", &M) == MGD77_NOT_SET) strcat (fx_setting, ",eot"), n_sub++;	/* Must append eot to requested list */
	}
	if (Ctrl->A.code[ADJ_MG] > 1) {	/* Need mtf1,2, and msens */
		 if (MGD77_Get_Column (GMT, "mtf1", &M) == MGD77_NOT_SET) strcat (fx_setting, ",mtf1"), n_sub++;	/* Must append mtf1 to requested list */
		 if (MGD77_Get_Column (GMT, "mtf2", &M) == MGD77_NOT_SET) strcat (fx_setting, ",mtf2"), n_sub++;	/* Must append mtf2 to requested list */
		 if (MGD77_Get_Column (GMT, "msens", &M) == MGD77_NOT_SET) strcat (fx_setting, ",msens"), n_sub++;	/* Must append msens to requested list */
	}
	/* If logical tests are specified we must make sure the required columns are included as auxillary */
	for (i = 0; i < M.n_constraints; i++) {
		if (MGD77_Get_Column (GMT, M.Constraint[i].name, &M) != MGD77_NOT_SET) continue;	/* OK, already included */
		strcat (fx_setting, ",");
		strcat (fx_setting, M.Constraint[i].name);	/* Must add to our list */
		n_sub++;
	}
	need_sound = (((Ctrl->A.code[ADJ_CT] & (1 | 2 | 8)) || Ctrl->A.code[ADJ_DP] & 2) && Ctrl->A.sound_speed == 0.0);
	Ctrl->A.sound_speed *= 0.5;	/* Takes care of the 2 in 2-way travel time */
	MGD77_Select_Columns (GMT, fx_setting, &M, 0);	/* Only deal with col names - leave constraints/exacts unchanged from last call */
	n_cols_to_process = M.n_out_columns - n_sub;
	
	MGD77_Set_Unit (GMT, Ctrl->N.unit[N_D], &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */
	MGD77_Set_Unit (GMT, Ctrl->N.unit[N_S], &vel_scale,  -1);	/* Sets output scale for distances using in velocities */
	switch (Ctrl->N.unit[N_S][0]) {
		case 'e':
			vel_scale /= dist_scale;			/* Must counteract any distance scaling to get meters. dt is in sec so we get  m/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(m/s)");
			break;
		case 'k':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get km. dt is in sec so 3600 gives  km/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(km/hr)");
			break;
		case 'M':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives  miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(mi/hr)");
			break;
		case 'n':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives  miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(kts)");
			break;
	}
	switch (Ctrl->N.unit[N_D][0]) {
		case 'e':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(m)");
			break;
		case 'k':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(km)");
			break;
		case 'M':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(miles)");
			break;
		case 'n':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(nm)");
			break;
	}

	GMT_init_distaz (GMT, GMT_MAP_DIST_UNIT, Ctrl->C.mode, GMT_MAP_DIST);

	Ctrl->S.start *= dist_scale;	Ctrl->S.stop *= dist_scale;	/* Convert the meters to the same units used for cumulative distances */

#ifdef USE_CM4
	if (auxlist[MGD77_AUX_CM].requested) MGD77_CM4_init (GMT, &M, &CM4);	/* Initialize CM4 structure */
#endif

	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		char path[GMT_BUFSIZ];
		if (!Ctrl->L.file) {	/* Try default correction table */
			sprintf (path, "%s%cmgd77_corrections.txt", M.MGD77_HOME, DIR_DELIM);
			if (access (path, R_OK)) {
				GMT_report (GMT, GMT_MSG_FATAL, "No default MGD77 Correction table (%s) found!\n", path);
				Return (EXIT_FAILURE);
			}
			Ctrl->L.file = path;
		}
		MGD77_Parse_Corrtable (GMT, Ctrl->L.file, list, (int)n_paths, M.n_out_columns, (char **)M.desired_column, 2, &CORR);
	}

	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */
	
		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;

		GMT_report (GMT, GMT_MSG_NORMAL, "Now processing cruise %s\n", list[argno]);
		
		D = MGD77_Create_Dataset (GMT);

		error = MGD77_Read_Header_Record (GMT, list[argno], &M, &D->H);
		if (error) {
			if (error == MGD77_ERROR_NOSUCHCOLUMN)
				GMT_report (GMT, GMT_MSG_FATAL, "One or more requested columns not present in cruise %s - skipping\n", list[argno]);
			else
				GMT_report (GMT, GMT_MSG_FATAL, "Error reading header sequence for cruise %s - skipping\n", list[argno]);
			MGD77_Free (GMT, D);
			continue;
		}

		/* Having the header we can process -F and assign indices that refers to this particular data set */
		
		
		if (first_cruise) {
			for (i = 0, string_output = FALSE ; i < n_cols_to_process; i++) {	/* Prepare GMT output formatting machinery */
				if (D->H.info[M.order[i].set].col[M.order[i].item].text) string_output = TRUE;
			}
			if (auxlist[MGD77_AUX_ID].requested) string_output = TRUE;
			if (string_output && GMT->common.b.active[1]) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Cannot specify binary output with text fields\n");
				MGD77_Free (GMT, D);
				Return (EXIT_FAILURE);
			}
			first_cruise = FALSE;
			if (!string_output) out = GMT_memory (GMT, NULL, n_out_columns, double);

		}
		
		if (MGD77_Read_Data (GMT, list[argno], &M, D)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading data set for cruise %s\n", list[argno]);
			MGD77_Free (GMT, D);
			Return (EXIT_FAILURE);
		}
		MGD77_Close_File (GMT, &M);
		
		/* The 1*, 2*, 3* below is just there to ensure we dont end up with multiple cases all == MGD77_NOT_SET */
		time_column = ((i = MGD77_Get_Column (GMT, "time", &M)) != MGD77_NOT_SET && M.order[i].set == MGD77_M77_SET) ? M.order[i].item : 1 * MGD77_NOT_SET;
		lon_column  = ((i = MGD77_Get_Column (GMT, "lon",  &M)) != MGD77_NOT_SET && M.order[i].set == MGD77_M77_SET) ? M.order[i].item : 2 * MGD77_NOT_SET;
		lat_column  = ((i = MGD77_Get_Column (GMT, "lat",  &M)) != MGD77_NOT_SET && M.order[i].set == MGD77_M77_SET) ? M.order[i].item : 3 * MGD77_NOT_SET;
		
		if (time_column != MGD77_NOT_SET && GMT->common.b.active[GMT_OUT] && GMT_is_verbose (GMT, GMT_MSG_NORMAL) && first_warning) {	/* Warn that binary time output is in Unix secs */
			GMT_report (GMT, GMT_MSG_NORMAL, "Warning: For binary output, time is stored as seconds since 1970 (Use TIME_SYSTEM=Unix to decode)\n");
			first_warning = FALSE;
		}
		for (i = kx = pos = 0; pos < n_out_columns; i++, pos++) {	/* Prepare GMT output formatting machinery */
			while (kx < n_aux && aux[kx].pos == i) {	/* Insert formatting for auxillary column (none are special) */
				GMT->current.io.col_type[GMT_OUT][pos] = GMT_IS_FLOAT;
				pos++, kx++;
			}
			if (i >= n_cols_to_process) continue;	/* Dont worry about helper columns that wont be printed */
			c  = M.order[i].set;
			id = M.order[i].item;
			if (id == time_column)	{/* Special time formatting */
				GMT->current.io.col_type[GMT_OUT][pos] = (c == 0) ? M.time_format : GMT_IS_FLOAT;
				t_pos = pos;	/* Output order of time */
			}
			else if (id == lon_column)	/* Special lon formatting */
				GMT->current.io.col_type[GMT_OUT][pos] = (c == 0) ? GMT_IS_LON : GMT_IS_FLOAT;
			else if (id == lat_column)	/* Special lat formatting */
				GMT->current.io.col_type[GMT_OUT][pos] = (c == 0) ? GMT_IS_LAT : GMT_IS_FLOAT;
			else 		/* Everything else is float (not true for the 3 strings though) */
				GMT->current.io.col_type[GMT_OUT][pos] = GMT_IS_FLOAT;
		}
		
		if (!GMT->common.b.active[GMT_OUT] && GMT->current.io.io_header[GMT_OUT]) {	/* Write out header record */
			fprintf (GMT->session.std[GMT_OUT], "# ");
			for (i = kx = pos = 0; pos < n_out_columns; i++, pos++) {
				while (kx < n_aux && aux[kx].pos == i) {	/* Insert auxillary column */
					fprintf (GMT->session.std[GMT_OUT], "%s", auxlist[aux[kx].type].header);
					if ((pos+1) < n_out_columns) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
					pos++, kx++;
				}
				if (i >= n_cols_to_process) continue;
				c  = M.order[i].set;
				id = M.order[i].item;
				fprintf (GMT->session.std[GMT_OUT], "%7s", D->H.info[c].col[id].abbrev);
				if ((pos+1) < n_out_columns) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
			}
			fprintf (GMT->session.std[GMT_OUT], "\n");
		}

		if (GMT->current.io.multi_segments[GMT_OUT]) {	/* Write segment header between each cruise */
			sprintf (GMT->current.io.segment_header, "%s\n", list[argno]);
			GMT_write_segmentheader (GMT, GMT->session.std[GMT_OUT], n_out_columns);
		}
		aux_dvalue[MGD77_AUX_DS] = cumulative_dist = ds = 0.0;
		if (auxlist[MGD77_AUX_ID].requested) strcpy (aux_tvalue[MGD77_AUX_ID], M.NGDC_id);
	
		t_col = MGD77_Get_Column (GMT, "time",   &M);
		x_col = MGD77_Get_Column (GMT, "lon",    &M);
		y_col = MGD77_Get_Column (GMT, "lat",    &M);
		z_col = MGD77_Get_Column (GMT, "depth",  &M);
		if (need_twt) twt_col = MGD77_Get_Column (GMT, "twt",  &M);
		if (Ctrl->A.code[ADJ_GR]) f_col = MGD77_Get_Column (GMT, "faa",  &M);
		if (Ctrl->A.code[ADJ_GR] > 1) g_col = MGD77_Get_Column (GMT, "gobs",  &M);
		if (Ctrl->A.code[ADJ_GR] == 3) e_col = MGD77_Get_Column (GMT, "eot",  &M);
		if (Ctrl->A.code[ADJ_MG]) m_col = MGD77_Get_Column (GMT, "mag",  &M);
		if (Ctrl->A.code[ADJ_MG] > 1) {	/* Need more magnetics items */
			m1_col = MGD77_Get_Column (GMT, "mtf1",  &M);
			m2_col = MGD77_Get_Column (GMT, "mtf2",  &M);
			ms_col = MGD77_Get_Column (GMT, "msens",  &M);
		}
		if ((auxlist[MGD77_AUX_GR].requested || (Ctrl->A.code[ADJ_GR] > 1 )) && Ctrl->A.GF_version == MGD77_NOT_SET) {
			Ctrl->A.GF_version = D->H.mgd77[use]->Gravity_Theoretical_Formula_Code - '0';
			if (Ctrl->A.GF_version < MGD77_IGF_HEISKANEN || Ctrl->A.GF_version > MGD77_IGF_1980) {
				GMT_report (GMT, GMT_MSG_FATAL, "Invalid Gravity Theoretical Formula Code (%c) - default to %d\n", D->H.mgd77[use]->Gravity_Theoretical_Formula_Code, MGD77_IGF_1980);
				Ctrl->A.GF_version = MGD77_IGF_1980;
			}
		}
		for (i = 0; i < M.n_out_columns; i++) {
			dvalue[i] = (double *)D->values[i];
			tvalue[i] = (char *)D->values[i];
		}

		this_limit_on_time = Ctrl->D.active;	/* Since we might change it below */
		if (time_column != MGD77_NOT_SET && D->H.no_time) {	/* Cannot know if ASCII MGD77 dont have time until after reading */
			GMT_LONG faked = FALSE;
			if (Ctrl->A.fake_times) {	/* Try to make fake times based on duration and distances */
				faked = MGD77_fake_times (GMT, &M, &(D->H), dvalue[x_col], dvalue[y_col], dvalue[t_col], D->H.n_records);
				if (faked) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Time column for cruise %s created from distances and duration\n", list[argno]);
			}
			if (!faked) {
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Time column not present in cruise %s - set to NaN\n", list[argno]);
				if (this_limit_on_time) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: -D limits cannot be used for cruise %s\n", list[argno]);
			}
			if (!faked && !Ctrl->D.mode) this_limit_on_time = FALSE;	/* To avoid pointless tests against NaN in loop */
		}
		if (need_sound) {	/* We opted to go with the value in the header [or 1500] */
			v = atof (D->H.mgd77[use]->Bathymetry_Assumed_Sound_Velocity) * 0.1;
			Ctrl->A.sound_speed = 0.5 * ((v < 1400.0 || v > 1600.0) ? 1500.0 : v);
		}
		
		if (Ctrl->A.sound_speed > 0.0) i_sound_speed = 1.0 / Ctrl->A.sound_speed;
		
		if (Ctrl->L.active) MGD77_Init_Correction (GMT, CORR[argno], dvalue);	/* Initialize origins if needed */
		
		has_prev_twt = PDR_wrap = FALSE;
		twt_pdrwrap_corr = 0.0;
		
		/* Start processing records  */
		
		for (rec = 0, prevrec = -1; rec < D->H.n_records; rec++) {
		
			/* Compute accumulated distance along track (Great circles or Flat Earth) */
		
			if (need_distances) {
				lonlat_not_NaN = !( GMT_is_dnan (dvalue[x_col][rec]) || GMT_is_dnan (dvalue[y_col][rec]));
				if (rec == 0) {	/* Azimuth at 1st point set to azimuth of 2nd point since there is no previous point */
					if (auxlist[MGD77_AUX_AZ].requested) aux_dvalue[MGD77_AUX_AZ] = GMT_az_backaz (GMT, dvalue[x_col][1], dvalue[y_col][1], dvalue[x_col][0], dvalue[y_col][0], FALSE);
				}
				else {		/* Need a previous point to calculate distance and heading */
					if (lonlat_not_NaN && prevrec >= 0) {	/* We have to records with OK lon,lat and can compute a distance from the previous OK point */
						ds = dist_scale * GMT_distance (GMT, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[x_col][prevrec], dvalue[y_col][prevrec]);
						if (auxlist[MGD77_AUX_AZ].requested) aux_dvalue[MGD77_AUX_AZ] = GMT_az_backaz (GMT, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[x_col][prevrec], dvalue[y_col][prevrec], FALSE);
						cumulative_dist += ds;
						aux_dvalue[MGD77_AUX_DS] = cumulative_dist;
					}
					else {
						aux_dvalue[MGD77_AUX_DS] = GMT->session.d_NaN;
						if (auxlist[MGD77_AUX_AZ].requested) aux_dvalue[MGD77_AUX_AZ] = GMT->session.d_NaN;
					}
				}
				if (auxlist[MGD77_AUX_SP].requested) {
					if (rec == 0 || prevrec < 0) {	/* Initialize various counters */
						dt = dvalue[t_col][1] - dvalue[t_col][0];
						if (auxlist[MGD77_AUX_SP].requested) aux_dvalue[MGD77_AUX_SP] = (GMT_is_dnan (dt) || dt == 0.0) ? GMT->session.d_NaN : vel_scale * ds / dt;
					}
					else {		/* Need a previous point to calculate speed */
						dt = dvalue[t_col][rec] - dvalue[t_col][prevrec];
						if (auxlist[MGD77_AUX_SP].requested) aux_dvalue[MGD77_AUX_SP] = (GMT_is_dnan (dt) || dt == 0.0) ? GMT->session.d_NaN : vel_scale * ds / dt;
					}
				}
				if (lonlat_not_NaN) prevrec = rec;	/* This was a record with OK lon,lat; make it the previous point for distance calculations */
			}
			
			/* Check if rec no, time or distance falls outside specified ranges */
		
			if (Ctrl->G.active && (rec < Ctrl->G.start || rec > Ctrl->G.stop)) continue;
			if (Ctrl->S.active && (cumulative_dist < Ctrl->S.start || cumulative_dist >= Ctrl->S.stop)) continue;
			if (Ctrl->D.mode && GMT_is_dnan (dvalue[t_col][rec])) continue;
			if (this_limit_on_time && (dvalue[t_col][rec] < Ctrl->D.start || dvalue[t_col][rec] >= Ctrl->D.stop)) continue;
			if (GMT->common.R.active) {	/* Check is lat/lon is outside specified area */
				if (dvalue[y_col][rec] < GMT->common.R.wesn[YLO] || dvalue[y_col][rec] > GMT->common.R.wesn[YHI]) continue;
				while (dvalue[x_col][rec] > GMT->common.R.wesn[XHI]) dvalue[x_col][rec] -= 360.0;
				while (dvalue[x_col][rec] < GMT->common.R.wesn[XLO]) dvalue[x_col][rec] += 360.0;
				if (dvalue[x_col][rec] > GMT->common.R.wesn[XHI]) continue;
			}
			
			if (Ctrl->Q.active[Q_V]) {	/* Check if we are outside velocity range */
				if (aux_dvalue[MGD77_AUX_SP] < Ctrl->Q.min[Q_V] || aux_dvalue[MGD77_AUX_SP] > Ctrl->Q.max[Q_V]) continue;
			}
			
			if (Ctrl->Q.active[Q_A]) {	/* Check if we are outside azimuth range */
				while (aux_dvalue[MGD77_AUX_AZ] > Ctrl->Q.min[Q_A]) aux_dvalue[MGD77_AUX_AZ] -= 360.0;	/* Wind down to be sure az < min azimuth */
				while (aux_dvalue[MGD77_AUX_AZ] < Ctrl->Q.min[Q_A]) aux_dvalue[MGD77_AUX_AZ] += 360.0;	/* Now add 360 until we pass min azimuth */	
				if (aux_dvalue[MGD77_AUX_AZ] > Ctrl->Q.max[Q_A]) continue;				/* Outside azimuth range */
			}
			/* Check if it passes any given column data constraints */
			
			if (!MGD77_Pass_Record (GMT, &M, D, rec)) continue;	/* Failed the test */

			/* This record will now be printed out */
		
			if (need_time) {	/* Need auxillary time columns such as year, days etc, hence we get the calendar first, then use MGD77_cal_to_fyear */
				MGD77_gcal_from_dt (GMT, &M, dvalue[t_col][rec], &cal);	/* No adjust for TZ; this is GMT UTC time */
				aux_dvalue[MGD77_AUX_YR] = (double)cal.year;
				aux_dvalue[MGD77_AUX_MO] = (double)cal.month;
				aux_dvalue[MGD77_AUX_DY] = (double)cal.day_m;
				aux_dvalue[MGD77_AUX_HR] = (double)cal.hour;
				aux_dvalue[MGD77_AUX_MI] = (double)cal.min;
				aux_dvalue[MGD77_AUX_SC] = (double)cal.sec;
				date = MGD77_cal_to_fyear (GMT, &cal);	/* Get date as decimal year */
				need_date = FALSE;
			}
			else
				need_date = TRUE;
			
			if (auxlist[MGD77_AUX_MG].requested) {	/* Evaluate IGRF */
				double date = 0.0;
				date = MGD77_cal_to_fyear (GMT, &cal);	/* Get date as decimal year */
				aux_dvalue[MGD77_AUX_MG] = (MGD77_igrf10syn (GMT, 0, date, 1, 0.0, dvalue[x_col][rec], dvalue[y_col][rec], IGRF)) ? GMT->session.d_NaN : IGRF[MGD77_IGRF_F];
			}
#ifdef USE_CM4
			if (auxlist[MGD77_AUX_CM].requested) {	/* Evaluate CM4 */
				double date;
				date = MGD77_cal_to_fyear (GMT, &cal);	/* Get date as decimal year */
/* Change this --> */		aux_dvalue[MGD77_AUX_MG] = (MGD77_igrf10syn (GMT, 0, date, 1, 0.0, dvalue[x_col][rec], dvalue[y_col][rec], IGRF)) ? GMT->session.d_NaN : IGRF[MGD77_IGRF_F];
			}
#endif

			if (auxlist[MGD77_AUX_GR].requested)	/* Evaluate Theoretical Gravity Model */
				aux_dvalue[MGD77_AUX_GR] = MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);

			if (auxlist[MGD77_AUX_CT].requested) {	/* Carter is one of the output columns */
				if (Ctrl->A.code[ADJ_CT]) {	/* We have requested some adjustment to the carter value */
					aux_dvalue[MGD77_AUX_CT] = GMT->session.d_NaN;
					if (Ctrl->A.code[ADJ_CT] & 1)	/* Try uncorr. depth - obs. depth */
						aux_dvalue[MGD77_AUX_CT] = dvalue[twt_col][rec] * Ctrl->A.sound_speed - dvalue[z_col][rec];	/* Factor of 2 dealt with earlier */
					if (Ctrl->A.code[ADJ_CT] & 2 && GMT_is_dnan (aux_dvalue[MGD77_AUX_CT])) {	/* Try uncorr. depth - Carter depth */
						MGD77_carter_depth_from_xytwt (GMT, dvalue[x_col][rec], dvalue[y_col][rec], 1000.0 * dvalue[twt_col][rec], &Carter, &z);
						aux_dvalue[MGD77_AUX_CT] = dvalue[twt_col][rec] * i_sound_speed - z;
					}
					if (Ctrl->A.code[ADJ_CT] & 4 && GMT_is_dnan (aux_dvalue[MGD77_AUX_CT])) {	/* Try uncorr. depth - inferred Carter depth */
						twt = dvalue[z_col][rec] * i_sound_speed;	/* Factor of 2 dealt with earlier */
						MGD77_carter_depth_from_xytwt (GMT, dvalue[x_col][rec], dvalue[y_col][rec], twt, &Carter, &z);
						aux_dvalue[MGD77_AUX_CT] = dvalue[z_col][rec] - z;
					}
					if (Ctrl->A.code[ADJ_CT] & 8 && GMT_is_dnan (aux_dvalue[MGD77_AUX_CT])) {	/* Try inferred uncorr. depth - obs. depth */
						MGD77_carter_twt_from_xydepth (GMT, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[z_col][rec], &Carter, &twt);
						z = twt * Ctrl->A.sound_speed;
						aux_dvalue[MGD77_AUX_CT] = z - dvalue[z_col][rec];
					}
				}
				else {
					twt = 1000.0 * dvalue[twt_col][rec];
					aux_dvalue[MGD77_AUX_CT] = MGD77_carter_correction (GMT, dvalue[x_col][rec], dvalue[y_col][rec], twt, &Carter);
				}
				if (negative_depth) aux_dvalue[MGD77_AUX_CT] = -aux_dvalue[MGD77_AUX_CT];
			}

			if (z_col != MGD77_NOT_SET && Ctrl->A.code[ADJ_DP]) {	/* We have requested some adjustment to the depth value */
				z = GMT->session.d_NaN;
				if (Ctrl->A.code[ADJ_DP] & 1)	/* Try obs. depth */
					z = dvalue[z_col][rec];
				if (Ctrl->A.code[ADJ_DP] & 2 && GMT_is_dnan (z))	/* Try uncorr. depth */
					z = dvalue[twt_col][rec] * i_sound_speed;
				if (Ctrl->A.code[ADJ_DP] & 4 && GMT_is_dnan (z)) {	/* Try Carter depth */
					twt = dvalue[twt_col][rec];
					if (!GMT_is_dnan (twt)) {	/* OK, valid twt */
						if (has_prev_twt) {	/* OK, may look at change in twt */
							d_twt = twt - prev_twt;
							if (fabs (d_twt) > TWT_PDR_WRAP_TRIGGER) {
								twt_pdrwrap_corr += copysign (TWT_PDR_WRAP, -d_twt);
								if (!PDR_wrap) GMT_report (GMT, GMT_MSG_NORMAL, "PDR travel time wrap detected for cruise %s\n", list[argno]);
								PDR_wrap = TRUE;
							}
						}
						has_prev_twt = TRUE;
						prev_twt = twt;
					}
					twt += twt_pdrwrap_corr;
					MGD77_carter_depth_from_xytwt (GMT, dvalue[x_col][rec], dvalue[y_col][rec], 1000.0 * twt, &Carter, &z);
				}
				if (Ctrl->A.force || !GMT_is_dnan(dvalue[z_col][rec])) dvalue[z_col][rec] = z;
			}
			
			if (f_col != MGD77_NOT_SET && Ctrl->A.code[ADJ_GR]) {	/* We have requested some adjustment to the faa value */
				g = GMT->session.d_NaN;
				if (Ctrl->A.code[ADJ_GR] == 1)	/* Try faa */
					g = dvalue[f_col][rec];
				if (Ctrl->A.code[ADJ_GR] == 2 && GMT_is_dnan (g))	/* Try gobs - ngrav */
					g = dvalue[g_col][rec] - MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);
				if (Ctrl->A.code[ADJ_GR] == 3 && GMT_is_dnan (g))	/* Try gobs + eot - ngrav */
					g = dvalue[g_col][rec] + dvalue[e_col][rec] - MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);
				if (Ctrl->A.force || !GMT_is_dnan(dvalue[f_col][rec])) dvalue[f_col][rec] = g;
			}
			
			if (m_col != MGD77_NOT_SET && Ctrl->A.code[ADJ_MG]) {	/* We have requested some adjustment to the mag value */
				m = GMT->session.d_NaN;
				if (Ctrl->A.code[ADJ_MG] & 1)	/* Try mag */
					m = dvalue[m_col][rec];
				if (Ctrl->A.code[ADJ_MG] & 2 && GMT_is_dnan (m)) {	/* Try mtf 1st - igrf */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = FALSE;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m2_col : m1_col;
					m = MGD77_Recalc_Mag_Anomaly_IGRF (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], FALSE);
				}
				if (Ctrl->A.code[ADJ_MG] & 4 && GMT_is_dnan (m)) {	/* Try mtf 2nd - igrf */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = FALSE;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m1_col : m2_col;
					m = MGD77_Recalc_Mag_Anomaly_IGRF (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], FALSE);
				}
#ifdef USE_CM4
				if (Ctrl->A.code[ADJ_MG] & 8 && GMT_is_dnan (m)) {	/* Try mtf 1st - cm4 */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = FALSE;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m2_col : m1_col;
					m = MGD77_Recalc_Mag_Anomaly_CM4 (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], FALSE, &CM4);
				}
				if (Ctrl->A.code[ADJ_MG] & 16 && GMT_is_dnan (m)) {	/* Try mtf 2nd - cm4 */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = FALSE;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m1_col : m2_col;
					m = MGD77_Recalc_Mag_Anomaly_CM4 (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], FALSE, &CM4);
				}
#endif
				if (Ctrl->A.force || !GMT_is_dnan(dvalue[m_col][rec])) dvalue[m_col][rec] = m;
			}
			
			if (negative_depth) dvalue[z_col][rec] = -dvalue[z_col][rec];
			if (negative_msd) dvalue[m_col][rec] = -dvalue[m_col][rec];
			
			if (string_output) {	/* Must do it col by col and deal with the requested string(s) */
				for (i = kx = pos = 0; pos < n_out_columns; i++, pos++) {
					while (kx < n_aux && aux[kx].pos == i) {	/* Insert auxillary column */
						if (aux[kx].text)
							fprintf (GMT->session.std[GMT_OUT], "%s", aux_tvalue[aux[kx].type]);
						else
							GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], aux_dvalue[aux[kx].type], pos);
						if ((pos+1) < n_out_columns) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
						kx++, pos++;
					}
					if (i >= n_cols_to_process) continue;
					c  = M.order[i].set;
					id = M.order[i].item;
					if (D->H.info[c].col[id].text)
						for (k = 0; k < D->H.info[c].col[id].text && tvalue[i][rec*D->H.info[c].col[id].text+k]; k++) fputc ((int)tvalue[i][rec*D->H.info[c].col[id].text+k], GMT->session.std[GMT_OUT]);
					else if (id == time_column) {	/* Time */
						if (GMT->current.io.col_type[GMT_OUT][pos] == GMT_IS_FLOAT) {	/* fractional year */
							if (need_date) {	/* Did not get computed already */
								date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
								need_date = FALSE;
							}
						}
						else if (M.adjust_time)
							date = MGD77_utime2time (GMT, &M, dvalue[t_col][rec]);
						else
							date = dvalue[t_col][rec];
						GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], date, pos);
					}
					else {
						correction = (Ctrl->L.active) ? MGD77_Correction (GMT, CORR[argno][i].term, dvalue, aux_dvalue, rec) : 0.0;
						GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], dvalue[i][rec] - correction, pos);
					}
					if ((pos+1) < n_out_columns) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				}
				fprintf (GMT->session.std[GMT_OUT], "\n");
			}
			else {	/* Use GMT output machinery which can handle binary output, if requested */
				for (i = kx = pos = 0; pos < n_out_columns; i++, pos++) {
					while (kx < n_aux && aux[kx].pos == i) {	/* Insert auxillary column */
						out[pos] = aux_dvalue[aux[kx].type];
						pos++, kx++;
					}
					if (i >= n_cols_to_process) continue;
					if (pos == t_pos) {	/* This is the time column */
						if (GMT->current.io.col_type[GMT_OUT][pos] == GMT_IS_FLOAT) {	/* fractional year */
							if (need_date) {	/* Did not get computed already */
								date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
								need_date = FALSE;
							}
							out[pos] = date;
						}
						else if (M.adjust_time)
							out[pos] = MGD77_utime2time (GMT, &M, dvalue[t_col][rec]);
						else
							out[pos] = dvalue[t_col][rec];
					}
					else {
						correction = (Ctrl->L.active) ? MGD77_Correction (GMT, CORR[argno][i].term, dvalue, aux_dvalue, rec) : 0.0;
						out[pos] = dvalue[i][rec] - correction;
					}
				}
				GMT->current.io.output (GMT, GMT->session.std[GMT_OUT], n_out_columns, out);
			}
			n_out++;
		}
		MGD77_Free (GMT, D);
		n_cruises++;
	}
	
	if (!string_output) GMT_free (GMT, out);
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Returned %ld output records from %ld cruises\n", n_out, n_cruises);
	
	MGD77_Path_Free (GMT, (int)n_paths, list);
	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, (int)n_paths);
#ifdef USE_CM4
	if (auxlist[MGD77_AUX_CM].requested) MGD77_CM4_end (GMT, &CM4);	/* Free up CM4 structure */
#endif
	MGD77_end (GMT, &M);

	Return (GMT_OK);
}
