/*--------------------------------------------------------------------
 *
 *    Copyright (c) 2004-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *		22-MAR-2018: Added -am8|16 for recompute mag using diur
 *
 *
 */

#include "gmt_dev.h"
#include "mgd77.h"

#define THIS_MODULE_CLASSIC_NAME	"mgd77list"
#define THIS_MODULE_MODERN_NAME	"mgd77list"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Extract data from MGD77 files"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:RVbdhj"

#define MGD77_FMT  "drt,id,tz,year,month,day,hour,dmin,lat,lon,ptc,twt,depth,bcc,btc,mtf1,mtf2,mag,msens,diur,msd,gobs,eot,faa,nqc,sln,sspn"
#define MGD77_ALL  "drt,id,time,lat,lon,ptc,twt,depth,bcc,btc,mtf1,mtf2,mag,msens,diur,msd,gobs,eot,faa,nqc,sln,sspn"
#define MGD77T_FMT "id,tz,date,hhmm,lat,lon,ptc,nqc,twt,depth,bcc,btc,bqc,mtf1,mtf2,mag,msens,diur,msd,mqc,gobs,eot,faa,gqc,sln,sspn"
#define MGD77T_ALL "id,time,lat,lon,ptc,nqc,twt,depth,bcc,btc,bqc,mtf1,mtf2,mag,msens,diur,msd,mqc,gobs,eot,faa,gqc,sln,sspn"
#define MGD77_GEO  "time,lat,lon,twt,depth,mtf1,mtf2,mag,gobs,faa"
#define MGD77_AUX  "dist,azim,cc,vel,weight"
#define MGD77_DAT  "drt,tz,year,month,day,hour,dmin,lat,lon,ptc,twt,depth,bcc,btc,mtf1,mtf2,mag,msens,diur,msd,gobs,eot,faa,nqc,id,sln,sspn"

#define ADJ_CT	0
#define ADJ_DP	1
#define ADJ_GR	2
#define ADJ_MG	3

/* Carter adjustment options */
#define CT_U_MINUS_DEPTH		1
#define CT_U_MINUS_CARTER		2
#define CT_UCORR_MINUS_CARTER_TU	4
#define CT_UCORR_CARTER_TU_MINUS_DEPTH	8

/* FAA adjustment options */
#define GR_FAA_STORED			1
#define GR_OBS_MINUS_NGRAV		2
#define GR_OBS_PLUS_EOT_MINUS_NGRAV	4
#define GR_OBS_PLUS_CEOT_MINUS_NGRAV	8

/* Depth adjustment options */
#define DP_DEPTH_STORED			1
#define DP_TWT_X_V			2
#define DP_TWT_X_V_MINUS_CARTER		4

/* Mag adjustment options */
#define MG_MAG_STORED			1
#define MG_MTF1_MINUS_IGRF		2
#define MG_MTF2_MINUS_IGRF		4
#define MG_MTF1_PLUS_DIUR_MINUS_IGRF	8
#define MG_MTF2_PLUS_DIUR_MINUS_IGRF	16

#define N_D	0	/* These are indices for -N subsets */
#define N_S	1
#define Q_A	0	/* These are indices for -Q subsets */
#define Q_C	1
#define Q_V	2

struct MGD77LIST_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct MGD77LIST_A {	/* -A */
		bool active;
		bool force;
		bool cable_adjust;
		bool cable_adjust_coord;
		bool fake_times;
		int code[4];
		int GF_version;
		double sound_speed;
		double sensor_offset;
	} A;
	struct MGD77LIST_D {	/* -D */
		bool active;
		bool mode;	/* true to skip recs with time == NaN */
		double start;	/* Start time */
		double stop;	/* Stop time */
	} D;
	struct MGD77LIST_E {	/* -E */
		bool active;
	} E;
	struct MGD77LIST_F {	/* -F */
		bool active;
		char *flags;
	} F;
	struct MGD77LIST_G {	/* -G */
		bool active;
		uint64_t start;	/* Start rec */
		uint64_t stop;	/* Stop rec */
	} G;
	struct MGD77LIST_I {	/* -I */
		bool active;
		unsigned int n;
		char code[3];
	} I;
	struct MGD77LIST_L {	/* -L */
		bool active;
		char *file;
	} L;
	struct MGD77LIST_N {	/* -N */
		bool active[2];
		char unit[2][2];
	} N;
	struct MGD77LIST_Q {	/* -Qa|c|v */
		bool active[3], c_abs;
		double min[3];
		double max[3];
	} Q;
	struct MGD77LIST_S {	/* -S */
		bool active;
		double start;	/* Start dist */
		double stop;	/* Stop dist */
	} S;
	struct T {	/* -T */
		bool active;
		int mode;	/* May be -1 */
	} T;
	struct MGD77LIST_W {	/* -W */
		bool active;
		double value;
	} W;
	struct MGD77LIST_Z {	/* -Z[n|p] */
		bool active;
		bool mode;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77LIST_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct MGD77LIST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.GF_version = MGD77_NOT_SET;
	C->D.stop = C->S.stop = DBL_MAX;	/* No upper limit on time and distance */
	C->G.stop = UINTMAX_MAX;		/* No limit on stop record */
	C->N.unit[N_D][0] = 'k';	/* Default is -Ndk */
	C->N.unit[N_S][0] = GMT_MAP_DIST_UNIT;	/* Default is -Nse */
	C->Q.max[Q_A] = 360.0;		/* Max azimuth limit */
	C->Q.min[Q_A] = -360.0;	C->Q.max[Q_A] = 360.0;		/* Min/max course change limits */
	C->Q.max[Q_V] = DBL_MAX;	/* No upper speed limit */
	C->T.mode = MGD77_NOT_SET;
	C->W.value = 1.0;	/* Default weight */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MGD77LIST_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->F.flags);
	gmt_M_str_free (C->L.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <cruise(s)> -F<dataflags>[,<tests>] [-Ac|d|f|m|t[<code>][+f]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Da<startdate>] [-Db<stopdate>] [-E] [-Ga<startrec>] [-Gb<stoprec>] [-I<code>]\n\t[-L[<corrtable.txt>]] [-N[s|p]]] [-Qa|v<min>/<max>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-Sa<startdist>] [-Sb<stopdist>]\n\t[-T[m|e]] [%s] [-W<Weight>] [-Z[n|p] [%s] [%s] [-h] [%s] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT, GMT_do_OPT, GMT_j_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	MGD77_Cruise_Explain (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t-F <dataflags> is a comma-separated string made up of one or more of these abbreviations\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (for standard MGD77 files - use mgd77list to probe for other columns in MGD77+ files).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   >Track information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     time:    Choose between Absolute time [default], Relative time, or fractional year.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       atime: Absolute time (formatted according to FORMAT_DATE_OUT, FORMAT_CLOCK_OUT).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       rtime: Relative time (formatted according to FORMAT_FLOAT_OUT and TIME_SYSTEM (or TIME_EPOCH, TIME_UNIT)).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       ytime: Absolute time as decimal year (formatted according to FORMAT_FLOAT_OUT).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       year:  Record year.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       month: Record month (1-12).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       day :  Record day of month (1-31).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       hour:  Record hour(0-23).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       min:   Record minute (0-59).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       sec:   Record second (0-60).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       dmin:  Decimal minute (0-59.xxxx).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       hhmm:  Clock hhmm.xxxx (0-2359.xxxx).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       date:  yyyymmdd string.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       tz :   Time zone adjustment in hours (-13 to +12).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     lon:     Longitude (formatted according to FORMAT_GEO_OUT).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     lat:     Latitude (formatted according to FORMAT_GEO_OUT).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     id:      Survey leg ID [string_output].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ngdcid:  NGDC ID [TEXTSTRING].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     recno:   Record number.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   >Derived navigational information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     dist:    Along-track distances (see -j for method and -N for units).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     azim:    Track azimuth (Degrees east from north).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     cc:      Course change, i.e., change in azimuth (Degrees east from north).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     vel:     Ship velocity (m/s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   >Geophysical Observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     twt:     Two-way travel-time (s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     depth:   Corrected bathymetry (m) [Also see -Z].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mtf1:    Magnetic Total Field Sensor 1 (gamma, nTesla).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mtf2:    Magnetic Total Field Sensor 2 (gamma, nTesla).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mag:     Magnetic residual anomaly (gamma, nTesla).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     gobs:    Observed gravity (mGal).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     faa:     Free-air gravity anomaly (mGal).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   >Codes, Corrections, and Information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     drt:     Data record type [5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ptc:     Position type code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     bcc:     Bathymetric correction code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     btc:     Bathymetric type code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     msens:   Magnetic sensor for residual field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     msd:     Magnetic sensor depth/altitude (m).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     diur:    Magnetic diurnal correction (gamma, nTesla).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     eot:     Stored Eotvos correction (mGal).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     sln:     Seismic line number string [TEXTSTRING].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     sspn:    Seismic shot point number string [TEXTSTRING].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     nqc:     Navigation quality code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   >Computed Information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     carter:  Carter correction from twt (m).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     igrf:    International Geomagnetic Reference Field (gamma, nTesla).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ceot:    Calculated Eotvos correction (mGal).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ngrav:   IGF, or Theoretical (Normal) Gravity Field (mGal).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     weight:  Report weight as specified in -W [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  The data are written in the order specified in <dataflags>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Shortcut flags are.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mgd77:   The full set of all 27 fields in the MGD77 specification.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mgd77t:  The full set of all 26 columns in the MGD77T specification.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     geo:     time,lon,lat + the 7 geophysical observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     all:     As mgd77 but with time items written as a date-time string.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     allt:    As mgd77t but with time items written as a date-time string.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     dat:     As mgd77t but in plain table file order.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append + to include the 5 derived quantities dist, azim, cc, vel, and weight [see -W]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    [Default is all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Abbreviations in UPPER CASE will suppress records where any such column is NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  (Note that -E is a shorthand to set all abbreviations to upper case).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Optionally, append comma-separated logical tests that data columns must pass to be output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  (Note: These checks do not applied to derived or computed data columns).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Format is <flag><OP><value>, where flag is any of the dataflags above, and <OP> is\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  one of the operators <, <=, =, >=, >, |, and !=.  <value> is the limit you are testing,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  including NaN (with = and != only).  If <flag> is UPPERCASE the test MUST be passed;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  else at least ONE of the tests must pass for output to take place.  When using operators\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  involving characters <, >, and |, put entire argument to -F in single quotes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Finally, for MGD77+ files you may optionally append : followed by one or more comma-\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  separated -+|-<col> terms.  This compares specific E77 bitflags for each listed column\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  + means bit must be 1, - means it must be 0.  All bit tests given must be passed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  By default, MGD77+ files with error bit flags will use the flags to suppress bad data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Turn this behavior off by append : with no arguments.  For controlling systematic\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  corrections encoded in MGD77+ files, see -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Adjust some data values before output. Append c|d|f|m|t to select field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c<code>[,<v>] Adjust field carter. <v>, the sound velocity in water, is taken from\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     the MGD77 header (or 1500 if invalid); optionally append your <v> (in m/s)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Here, C(twt) is Carter correction, U(twt,v) is uncorrected depth (given <v>).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     TC(z) is twt from inverse Carter correction, TU(z,v) is twt from uncorrected depth.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       c1 return difference between U(twt,v) and depth [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       c2 return difference between U(twt,v) and Carter(twt).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       c4 return difference between (uncorrected) depth and Carter (TU(depth,v)).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       c8 return difference between U(TC(depth),v) and depth.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d<code>[,<v>] Adjust field depth. <v> is optional sound speed in water (m/s)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       d1 return depth as stored in file [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       d2 return calculated uncorrected depth U(twt,v).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       d4 return calculated corrected depth Carter (twt,v).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f<code>[,<field>] Adjust field faa. <field>, the IGF reference field, is taken\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     from the MGD77 header (or 4 if invalid); optionally append your <field> from.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1 = Heiskanen 1924 formula.\n\t       ");
	MGD77_IGF_text (API->GMT, API->GMT->session.std[GMT_ERR], 1);
	GMT_Message (API, GMT_TIME_NONE, "\t     2 = International 1930 formula.\n\t       ");
	MGD77_IGF_text (API->GMT, API->GMT->session.std[GMT_ERR], 2);
	GMT_Message (API, GMT_TIME_NONE, "\t     3 = International 1967 formula.\n\t       ");
	MGD77_IGF_text (API->GMT, API->GMT->session.std[GMT_ERR], 3);
	GMT_Message (API, GMT_TIME_NONE, "\t     4 = International 1980 formula.\n\t       ");
	MGD77_IGF_text (API->GMT, API->GMT->session.std[GMT_ERR], 4);
	GMT_Message (API, GMT_TIME_NONE, "\t       f1 return faa as stored in file [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       f2 return difference gobs - ngrav.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       f4 return difference gobs + eot - ngrav.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       f8 return difference gobs + ceot - ngrav.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m<code> Adjust field mag.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       m1  return mag as stored in file [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       m2  return difference mtfx - igrf, where x = msens (or 1 if undefined).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       m4  return difference mtfx - igrf, where x != msens (or 2 if undefined).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       m8  return difference mtfx + diur - igrf, where x = msens (or 1 if undefined).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       m16 return difference mtfx + diur - igrf, where x != msens (or 2 if undefined).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       mc<offset> Apply cable tow distance correction to mtf1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t will compute fake times for cruises with known duration but lacking record times.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f to force selected anomalies to be recalculated even when the original\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   anomaly is NaN [Default honors NaNs in existing anomalies].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D List from a<date> (given as yyyy-mm-ddT[hh:mm:ss]) [Start of cruise]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   up to b<date> (given as yyyy-mm-ddT[hh:mm:ss]) [End of cruise].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If A|B is used instead or a|b then records with no time are excluded from output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Output records that exactly matches the requested geophysical information in -F\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will output all record that matches at least one column].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G List from given a<record> [Start of cruise] up to given b<record> [End of cruise].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Ignore certain data file formats from consideration. Append combination of act to ignore\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (a) MGD77 ASCII, (c) MGD77+ netCDF, (m) MGD77T ASCII, or (t) plain table files. [Default ignores none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the default file mgd77_corrections.txt in $MGD77_HOME is assumed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Append (d)istances or (s)peed, and your choice for unit. Choose among.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e Metric units I (meters, m/s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f British/US units I (feet, feet/s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   k Metric units II (km, km/hr).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   M British/US units II (miles, miles/hr).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n Nautical units (nautical miles, knots).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   u Old US units (survey feet, sfeets).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is -Ndk -Nse].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Return data whose azimuth (-Qa) or velocity (-Qv) fall inside specified range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qa<min_az>/<max_az>, where <min_az> < <max_az> [all azimuths, i.e., 0/360].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qc<min_cc>/<max_cc>, where <min_cc> < <max_cc> [all course changes, i.e., -360/360].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Use -QC to use abs value |cc| in the test [0/360].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qv<min_vel>[/<max_vel>], where <max_vel> is optional [all velocities, i.e., 0/infinity].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Velocities are given in m/s unless changed by -Ns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Return data inside the specified region only [0/360/-90/90].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Begin list from a<startdist>; append unit from %s [meter] [Start of cruise]\n", GMT_LEN_UNITS2_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   End list at b<stopdist> [End of cruise].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Turn OFF the otherwise automatic adjustment of values based on correction terms\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   stored in the mgd77+ file (option has no effect on plain MGD77 ASCII files).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m or e to indicate the MGD77 data set or the extended columns set [Default is both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For controlling application of point bit flags, see -F and the : modifier discussion.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set weight for these data [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Append n to report bathymetry and msd as negative depths [Default is positive -Zp].\n");
	GMT_Option (API, "bo,do");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t-h Write header record with column information [Default is no header].\n");
	GMT_Option (API, "j,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MGD77LIST_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mgd77list and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	int code;
	char *t = NULL, buffer[GMT_BUFSIZ] = {""}, *c = NULL;
	double dist_scale;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjustment flags */
				Ctrl->A.active = true;
				k = 0;
				if (opt->arg[k] == '+') {	/* Recalculate anomalies even if original anomaly == NaN [Default leaves NaNs unchanged] */
					Ctrl->A.force = true;
					k++;
				}
				else if ((c = strstr (opt->arg, "+f"))) {
					Ctrl->A.force = true;
					c[0] = '\0';	/* Chop of modifier */
				}
				switch (opt->arg[k]) {
					case 'c':	/* Carter correction adjustment */
						code = opt->arg[k+1] - '0';
						if (code < 1 || code > 11) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Ac<code>: <code> must be 1,2,4,8 or binary combination.\n");
							n_errors++;
						}
						if (opt->arg[k+2] == ',') {
							Ctrl->A.sound_speed = atof (&opt->arg[k+3]);
							if (Ctrl->A.sound_speed < 1400.0 || Ctrl->A.sound_speed > 1600.0) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -Ac<code>,<speed>: <speed> in m/s in the 1400-1600 range.\n");
								n_errors++;
							}
						}
						Ctrl->A.code[ADJ_CT] |= code;
						break;
					case 'd':	/* depth adjustment */
						code = opt->arg[k+1] - '0';
						if (code < 1 || code > 7) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Ad<code>: <code> must be 1,2,4 or binary combination.\n");
							n_errors++;
						}
						if (opt->arg[k+2] == ',') {
							Ctrl->A.sound_speed = atof (&opt->arg[k+3]);
							if (Ctrl->A.sound_speed < 1400.0 || Ctrl->A.sound_speed > 1600.0) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -Ad<code>,<speed>: <speed> in m/s in the 1400-1600 range.\n");
								n_errors++;
							}
						}
						Ctrl->A.code[ADJ_DP] |= code;
						break;
					case 'f':	/* faa adjustment */
						code = opt->arg[k+1] - '0';
						if (code < 1 || code > 15) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Af<code>: <code> must be 1,2,4,8 or binary combination.\n");
							n_errors++;
						}
						if (opt->arg[k+2] == ',') {
							Ctrl->A.GF_version = atoi (&opt->arg[k+3]);
							if (Ctrl->A.GF_version < MGD77_IGF_HEISKANEN || Ctrl->A.GF_version > MGD77_IGF_1980) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -Af<code>,<field>: Select <field> is 1-4 range.\n");
								n_errors++;
							}
						}
						Ctrl->A.code[ADJ_GR] |= code;
						break;
					case 'm':	/* mag adjustment */
						if (opt->arg[k+1] == 'c') {
							MGD77_Set_Unit (GMT, &opt->arg[k+2], &dist_scale, 1);
							Ctrl->A.cable_adjust = true;
							Ctrl->A.sensor_offset = atof (&opt->arg[k+2]) * dist_scale;
							if (Ctrl->A.sensor_offset < 0.0) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -Amc: Cable length offset must be positive or zero.\n");
								n_errors++;
							}
						}
						else {
							code = atoi (&opt->arg[k+1]);
							if (code < 1 || code > 33) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -Am<code>: <code> must be 1,2,4,8,16 or binary combination.\n");
								n_errors++;
							}
							Ctrl->A.code[ADJ_MG] |= code;
						}
						break;
					case 't':	/* fake time requires */
						Ctrl->A.fake_times = true;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -A<flag>: <flag> must be c, d, g, m, or t.\n");
						n_errors++;
						break;
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'C':	/* Distance calculation flag */
				if (gmt_M_compat_check (API->GMT, 6)) {
					GMT_Report (API, GMT_MSG_COMPAT, "The -C option is deprecated; use the GMT common option -j<mode> instead\n");
					if (!strchr ("cefg", (int)opt->arg[0])) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Flag must be c, f, g, or e\n");
						n_errors++;
					}
					else
						gmt_parse_j_option (GMT, opt->arg);
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Unrecognized option -C\n");
					n_errors++;
				}
				break;

			case 'D':		/* Assign start/stop times for sub-section */
				Ctrl->D.active = true;
				switch (opt->arg[0]) {
				 	case 'A':		/* Start date, skip records with time = NaN */
						Ctrl->D.mode = true;
						/* Fall through on purpose to 'a' */
				 	case 'a':		/* Start date */
						t = &opt->arg[1];
						if (t && gmt_verify_expectations (GMT, GMT_IS_ABSTIME, gmt_scanf (GMT, t, GMT_IS_ABSTIME, &Ctrl->D.start), t)) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Da: Start time (%s) in wrong format\n", t);
							n_errors++;
						}
						break;
					case 'B':		/* Stop date, skip records with time = NaN */
						Ctrl->D.mode = true;
						/* Fall through on purpose to 'b' */
					case 'b':		/* Stop date */
						t = &opt->arg[1];
						if (t && gmt_verify_expectations (GMT, GMT_IS_ABSTIME, gmt_scanf (GMT, t, GMT_IS_ABSTIME, &Ctrl->D.stop), t)) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Db: Stop time (%s) in wrong format\n", t);
							n_errors++;
						}
						break;
					default:
						n_errors++;
						break;
				}
				break;

			case 'E':	/* Exact parameter match */
				Ctrl->E.active = true;
				break;

			case 'F':	/* Selected output fields */
				Ctrl->F.active = true;
				strncpy (buffer, opt->arg, GMT_BUFSIZ-1);
				if (!strcmp (buffer, "mgd77")) strncpy (buffer, MGD77_FMT, GMT_BUFSIZ);
				if (!strcmp (buffer, "mgd77+")) {
					strncpy (buffer, MGD77_FMT, GMT_BUFSIZ-1);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				if (!strcmp (buffer, "mgd77t")) strncpy (buffer, MGD77T_FMT, GMT_BUFSIZ);
				if (!strcmp (buffer, "mgd77t+")) {
					strncpy (buffer, MGD77T_FMT, GMT_BUFSIZ-1);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				if (!strcmp (buffer, "all")) strncpy (buffer, MGD77_ALL, GMT_BUFSIZ);
				if (!strcmp (buffer, "all+")) {
					strncpy (buffer, MGD77_ALL, GMT_BUFSIZ-1);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				if (!strcmp (buffer, "allt")) strncpy (buffer, MGD77T_ALL, GMT_BUFSIZ);
				if (!strcmp (buffer, "allt+")) {
					strncpy (buffer, MGD77T_ALL, GMT_BUFSIZ-1);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				if (!strcmp (buffer, "geo")) strncpy (buffer, MGD77_GEO, GMT_BUFSIZ);
				if (!strcmp (buffer, "geo+")) {
					strncpy (buffer, MGD77_GEO, GMT_BUFSIZ-1);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				if (!strcmp (buffer, "dat")) strncpy (buffer, MGD77_DAT, GMT_BUFSIZ);
				if (!strcmp (buffer, "dat+")) {
					strncpy (buffer, MGD77_DAT, GMT_BUFSIZ-1);
					strcat (buffer, ",");
					strcat (buffer, MGD77_AUX);
				}
				Ctrl->F.flags = strdup (buffer);
				break;

			case 'G':		/* Assign start/stop records for sub-section */
				Ctrl->G.active = true;
				switch (opt->arg[0]) {
				 	case 'a':		/* Start record */
						Ctrl->G.start = atol (&opt->arg[1]);
						break;
					case 'b':		/* Stop record */
						Ctrl->G.stop = atol (&opt->arg[1]);
						break;
					default:
						n_errors++;
						break;
				}
				break;

			case 'I':
				Ctrl->I.active = true;
				if (Ctrl->I.n < 3) {
					if (strchr ("acmt", (int)opt->arg[0]))
						Ctrl->I.code[Ctrl->I.n++] = opt->arg[0];
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -I Bad modifier (%c): Use -Ia|c|m|t!\n", opt->arg[0]);
						n_errors++;
					}
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -I: Can only be applied 0-2 times\n");
					n_errors++;
				}
				break;

			case 'L':	/* Crossover correction table */
				Ctrl->L.active = true;
				Ctrl->L.file = strdup (opt->arg);
				break;

			case 'N':	/* Nautical units (knots, nautical miles) */
				if (opt->arg[1] == 'm' && gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -N: Unit m for miles is deprecated; use unit M instead\n");
					opt->arg[1] = 'M';
				}
				switch (opt->arg[0]) {
					case 'd':	/* Distance unit selection */
						Ctrl->N.active[N_D] = true;
						Ctrl->N.unit[N_D][0] = opt->arg[1];
						if (!strchr (GMT_LEN_UNITS2, (int)Ctrl->N.unit[N_D][0])) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Nd: Unit must be among %s\n", GMT_LEN_UNITS2_DISPLAY);
							n_errors++;
						}
						break;
					case 's':	/* Speed unit selection */
						Ctrl->N.active[N_S] = true;
						Ctrl->N.unit[N_S][0] = opt->arg[1];
						if (!strchr (GMT_LEN_UNITS2, (int)Ctrl->N.unit[N_S][0])) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Nd: Unit must be among %s\n", GMT_LEN_UNITS2_DISPLAY);
							n_errors++;
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -N: Syntax is -Nd|s<unit>\n");
						n_errors++;
						break;
				}
				break;

			case 'Q':		/* Assign min/max values for speeds or azimuth */
				switch (opt->arg[0]) {
					case 'a':	/* Azimuth min/max */
						if (sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->Q.min[Q_A], &Ctrl->Q.max[Q_A]) != 2) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Qa: append min/max azimuth limits [0/360]\n");
							n_errors++;
						}
						Ctrl->Q.active[Q_A] = true;
						break;
					case 'C':	/* Course change min/max using absolute value of cc */
						Ctrl->Q.c_abs = true;
						/* Fall through on purpose to 'c' */
					case 'c':	/* Course change min/max */
						if (sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->Q.min[Q_C], &Ctrl->Q.max[Q_C]) != 2) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Qc: append min/max course change limits [-360/+360]\n");
							n_errors++;
						}
						Ctrl->Q.active[Q_C] = true;
						break;
					case 'v':	/* Velocity min/max */
						code = sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->Q.min[Q_V], &Ctrl->Q.max[Q_V]);
						if (code == 1)
							Ctrl->Q.max[Q_V] = DBL_MAX;
						else if (code <= 0) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Qv: append min[/max] velocity limits [0]\n");
							n_errors++;
						}
						Ctrl->Q.active[Q_V] = true;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Syntax is -Qa|c|v<min>/<max>\n");
						n_errors++;
						break;
				}
				break;

			case 'S':		/* Assign start/stop position for sub-section (converted to meters) */
				Ctrl->S.active = true;
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
				Ctrl->T.active = true;
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
						GMT_Report (API, GMT_MSG_ERROR, "Option -T: append m, e, or neither\n");
						n_errors++;
						break;
				}
				break;
			case 'W':		/* Assign a weight to these data */
				Ctrl->W.active = true;
				Ctrl->W.value = (!strcmp (opt->arg, "NaN")) ? GMT->session.d_NaN : atof (opt->arg);
				break;

			case 'Z':		/* -Zn is negative down for depths */
				Ctrl->Z.active = true;
				switch (opt->arg[0]) {
					case '-':	case 'n':	Ctrl->Z.mode = true;	break;
					case '+':	case 'p':	Ctrl->Z.mode = false;	break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -Z: append n or p\n");
						n_errors++;
						break;
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->D.start > 0.0 && Ctrl->S.start > 0.0, "Options -D and S: Cannot specify both start time AND start distance\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.stop < DBL_MAX && Ctrl->S.stop < DBL_MAX, "Options D and S: Cannot specify both stop time AND stop distance\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.value <= 0.0, "Option -W: weight must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.start > Ctrl->S.stop, "Option -S: Start distance exceeds stop distance!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active[Q_A] && Ctrl->Q.min[Q_A] >= Ctrl->Q.max[Q_A], "Option -Qa: Minimum azimuth equals or exceeds maximum azimuth!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active[Q_C] && Ctrl->Q.min[Q_C] >= Ctrl->Q.max[Q_C], "Option -Qc: Minimum course change equals or exceeds maximum course change!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active[Q_V] && (Ctrl->Q.min[Q_V] >= Ctrl->Q.max[Q_V] || Ctrl->Q.min[Q_V] < 0.0), "Option -Qv: Minimum velocity equals or exceeds maximum velocity or is negative!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.start > Ctrl->D.stop, "Option -D: Start time exceeds stop time!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int separate_aux_columns (struct MGD77_CONTROL *F, char *fx_setting, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist) {
	unsigned int i, j, k, n_aux;
	int this_aux;

	fx_setting[0] = '\0';
	for (i = k = n_aux = 0; i < F->n_out_columns; i++) {
		for (j = 0, this_aux = MGD77_NOT_SET; j < N_MGD77_AUX && this_aux == MGD77_NOT_SET; j++)
			if (!strcmp (auxlist[j].name, F->desired_column[i])) this_aux = j;
		if (this_aux == MGD77_NOT_SET) {	/* Just pass other columns through */
			if (k) strcat (fx_setting, ",");
			strcat (fx_setting, F->desired_column[i]);
			k++;
		}
		else
		{	/* Found a request for an auxiliary column  */
			aux[n_aux].type = auxlist[this_aux].type;
			aux[n_aux].text = auxlist[this_aux].text;
			aux[n_aux].pos = k;
			auxlist[this_aux].requested = true;
			n_aux++;
		}
	}
	return (n_aux);
}

GMT_LOCAL int augment_aux_columns (int n_items, char **item_name, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist, int n_aux) {
	/* This adds additional aux columns that are required by the correction table and not already requested by other means (e.g. -F) */
	int i, j, k, this_aux, n;

	for (i = k = 0, n = n_aux; i < n_items; i++) {
		for (j = 0, this_aux = MGD77_NOT_SET; j < N_MGD77_AUX && this_aux == MGD77_NOT_SET; j++)
			if (!strcmp (auxlist[j].name, item_name[i])) this_aux = j;
		if (this_aux != MGD77_NOT_SET && !auxlist[this_aux].requested) {	/* Found a request for an auxiliary column not yet requested  */
			aux[n].type = auxlist[this_aux].type;
			aux[n].text = auxlist[this_aux].text;
			aux[n].pos = k;
			auxlist[this_aux].requested = true;
			n++;
		}
	}
	return (n);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mgd77list (void *V_API, int mode, void *args) {
	int i, c, id, k, time_column, lon_column, lat_column, error = 0;
	int t_col, x_col, y_col, z_col, e_col = 0, m_col = 0, f_col = 0;
	int ms_col = 0, md_col = 0, twt_col = 0, g_col = 0, m1_col = 0, m2_col = 0;
	int sep_flag, n_paths;

	unsigned int select_option, n_out = 0, argno, n_cruises = 0, kx, n_items = 0;
	unsigned int kk, n_sub, n_out_columns, n_cols_to_process, n_aux, pos, use;

	uint64_t rec, prevrec;

	enum GMT_enum_geometry geometry;

	bool negative_depth = false, negative_msd = false, need_distances, need_time;
	bool string_output = false, need_depth = false, PDR_wrap, has_prev_twt = false;
	bool need_lonlat = false, first_cruise = true, need_twt = false, this_limit_on_time;
	bool need_date, need_sound = false, lonlat_not_NaN, first_warning = true;
	bool first_time_on_sensor_offset = true;

	char fx_setting[GMT_BUFSIZ] = {""}, **list = NULL, **item_names = NULL;
	char *tvalue[MGD77_MAX_COLS], *aux_tvalue[N_MGD77_AUX], record[GMT_BUFSIZ] = {""}, word[GMT_LEN32] = {""};

	double IGRF[7], correction, prev_twt = 0, d_twt, twt_pdrwrap_corr, this_cc;
	double dist_scale, vel_scale, ds, ds0 = 0.0, dt, cumulative_dist, aux_dvalue[N_MGD77_AUX];
	double i_sound_speed = 0.0, date = 0.0, g, m, z, v, twt, prev_az = 0.0, next_az;
	double *cumdist = NULL, *cumdist_off = NULL, *mtf_bak = NULL, *mtf_int = NULL;
	double *dvalue[MGD77_MAX_COLS], *out = NULL;

	struct MGD77_CONTROL M;
	struct MGD77_DATASET *D = NULL;
	struct MGD77_AUX_INFO aux[N_MGD77_AUX];
	struct GMT_GCAL cal;
	struct MGD77_CARTER Carter;
	struct MGD77_CORRTABLE **CORR = NULL;
	struct MGD77_AUXLIST auxlist[N_MGD77_AUX] = {
		{ "dist",    MGD77_AUX_DS, false, false, "d(km)"},
		{ "azim",    MGD77_AUX_AZ, false, false, "azimuth"},
		{ "cc",      MGD77_AUX_CC, false, false, "ccourse"},
		{ "vel",     MGD77_AUX_SP, false, false, "v(m/s)"},
		{ "year",    MGD77_AUX_YR, false, false, "year"},
		{ "month",   MGD77_AUX_MO, false, false, "month"},
		{ "day",     MGD77_AUX_DY, false, false, "day"},
		{ "hour",    MGD77_AUX_HR, false, false, "hour"},
		{ "min",     MGD77_AUX_MI, false, false, "minute"},
		{ "dmin",    MGD77_AUX_DM, false, false, "dec-minute"},
		{ "sec",     MGD77_AUX_SC, false, false, "second"},
		{ "date",    MGD77_AUX_DA, true,  false, "date"},
		{ "hhmm",    MGD77_AUX_HM, false, false, "hourmin"},
		{ "weight",  MGD77_AUX_WT, false, false, "weight"},
		{ "drt",     MGD77_AUX_RT, false, false, "rectype"},
		{ "igrf",    MGD77_AUX_MG, false, false, "IGRF"},
		{ "carter",  MGD77_AUX_CT, false, false, "Carter"},
		{ "ngrav",   MGD77_AUX_GR, false, false, "IGF"},
		{ "ceot",    MGD77_AUX_ET, false, false, "ceot"},
		{ "recno",   MGD77_AUX_RN, true,  false, "recno"},
		{ "ngdcid",  MGD77_AUX_ID, true,  false, "NGDC-ID"}
	};
	struct GMT_RECORD *Out = NULL;
	struct MGD77LIST_CTRL *Ctrl = NULL;
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
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the mgd77list main code ----------------------------*/

	/* Initialize MGD77 output order and other parameters*/

	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */
	if (Ctrl->I.active) MGD77_Process_Ignore (GMT, 'I', Ctrl->I.code);
	aux_dvalue[MGD77_AUX_WT] = Ctrl->W.value;			/* Default weight */
	aux_dvalue[MGD77_AUX_RT] = 5.0;					/* Default record type */
	if (!Ctrl->F.active) Ctrl->F.flags = strdup (MGD77_ALL);	/* Default is a full MGD77 record */
	negative_msd = negative_depth = Ctrl->Z.mode;			/* Follow the -Z option to start with */
	if (!GMT->common.j.active) GMT->common.j.mode = GMT_GREATCIRCLE;

	if (Ctrl->T.active) {	/* Turn off automatic corrections */
		if (Ctrl->T.mode == MGD77_NOT_SET)	/* Both sets */
			M.use_corrections[MGD77_M77_SET] = M.use_corrections[MGD77_CDF_SET] = false;
		else if (Ctrl->T.mode == MGD77_M77_SET) /* MGD77 set */
			M.use_corrections[MGD77_M77_SET] = false;
		else	/* extra CDF set */
			M.use_corrections[MGD77_CDF_SET] = false;
	}

	/* Check that the options selected are mutually consistent */

	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (n_paths <= 0) {
		GMT_Report (API, GMT_MSG_ERROR, "No cruises given\n");
		Return (GMT_NO_INPUT);
	}

	if (M.adjust_time) Ctrl->D.start = MGD77_time2utime (GMT, &M, Ctrl->D.start);	/* Convert to Unix time if need be */
	if (M.adjust_time) Ctrl->D.stop  = MGD77_time2utime (GMT, &M, Ctrl->D.stop);
	if (Ctrl->L.active) {	/* Scan the ephemeral correction table for needed auxiliary columns */
		char path[PATH_MAX] = {""};
		if (!Ctrl->L.file) {	/* Try default correction table */
			sprintf (path, "%s/mgd77_corrections.txt", M.MGD77_HOME);
			if (access (path, R_OK)) {
				GMT_Report (API, GMT_MSG_ERROR, "No default MGD77 Correction table (%s) found!\n", path);
				MGD77_Path_Free (GMT, (uint64_t)n_paths, list);
				Return (GMT_FILE_NOT_FOUND);
			}
			Ctrl->L.file = path;
		}
		n_items = MGD77_Scan_Corrtable (GMT, Ctrl->L.file, list, n_paths, M.n_out_columns, M.desired_column, &item_names, 2);
	}

	select_option = MGD77_RESET_CONSTRAINT | MGD77_RESET_EXACT;	/* Make sure these start at zero */
	if (Ctrl->E.active) select_option |= MGD77_SET_ALLEXACT;	/* Sets all columns listed as "must be present" */
	MGD77_Select_Columns (GMT, Ctrl->F.flags, &M, select_option);	/* This is the list of columns the user ultimately wants output */
	if (M.time_format == GMT_IS_RELTIME) M.adjust_time = true;
	n_out_columns = M.n_out_columns;				/* This is the total number of columns in the final output */
	if (MGD77_Get_Column (GMT, "depth", &M) == MGD77_NOT_SET) negative_depth = false;	/* Just so we don't accidentally access dvalue[z_col] further down in the loop */
	if (MGD77_Get_Column (GMT, "msd", &M) == MGD77_NOT_SET) negative_msd = false;	/* Just so we don't accidentally access dvalue[m_col] further down in the loop */
	n_aux = separate_aux_columns (&M, fx_setting, aux, auxlist);				/* Determine which auxiliary columns are requested (if any) */
	if (Ctrl->L.active) {
		n_aux = augment_aux_columns ((int)n_items, item_names, aux, auxlist, (int)n_aux);	/* Determine which auxiliary columns are needed by -L */
		for (kk = 0; kk < n_items; kk++) gmt_M_free (GMT, item_names[kk]);
		if (n_items) gmt_M_free (GMT, item_names);
		MGD77_Free_Table (GMT, n_items, item_names);
	}
	aux_tvalue[MGD77_AUX_ID] = gmt_M_memory (GMT, NULL, GMT_LEN64, char);	/* Just in case */
	aux_tvalue[MGD77_AUX_DA] = gmt_M_memory (GMT, NULL, GMT_LEN64, char);	/* Just in case */
	use = (M.original) ? MGD77_ORIG : MGD77_REVISED;

	/* Most auxiliary columns depend on values in the data columns.  If the user did not specify the
	   required data columns then we must append them to make sure we have access to the values we need
	   to calculate the auxiliary values. Also, so limit tests on data records (e.g., distances, region,
	   or time) also implies the need for certain data columns such as time, lon, and lat.
	 */

	if (Ctrl->A.code[ADJ_GR] & GR_OBS_PLUS_CEOT_MINUS_NGRAV || auxlist[MGD77_AUX_ET].requested) {	/* Computing Eotvos requires heading and speed */
		auxlist[MGD77_AUX_AZ].requested = true;
		auxlist[MGD77_AUX_SP].requested = true;
	}
	need_distances = (Ctrl->S.active || auxlist[MGD77_AUX_SP].requested || auxlist[MGD77_AUX_DS].requested || auxlist[MGD77_AUX_AZ].requested || auxlist[MGD77_AUX_CC].requested);	/* Distance is requested */
	need_lonlat = (auxlist[MGD77_AUX_MG].requested || auxlist[MGD77_AUX_GR].requested || auxlist[MGD77_AUX_CT].requested || Ctrl->A.code[ADJ_MG] > MG_MAG_STORED || Ctrl->A.code[ADJ_DP] & DP_TWT_X_V_MINUS_CARTER || Ctrl->A.code[ADJ_CT] > CT_U_MINUS_DEPTH || Ctrl->A.code[ADJ_GR] > GR_FAA_STORED || Ctrl->A.fake_times || Ctrl->A.cable_adjust);	/* Need lon, lat to calculate reference fields or Carter correction */
	need_time = (auxlist[MGD77_AUX_YR].requested || auxlist[MGD77_AUX_MO].requested || auxlist[MGD77_AUX_DY].requested ||
	             auxlist[MGD77_AUX_HR].requested || auxlist[MGD77_AUX_MI].requested || auxlist[MGD77_AUX_SC].requested ||
	             auxlist[MGD77_AUX_DM].requested || auxlist[MGD77_AUX_HM].requested || auxlist[MGD77_AUX_DA].requested ||
	             auxlist[MGD77_AUX_MG].requested || Ctrl->A.code[ADJ_MG] > MG_MAG_STORED);
	n_sub = 0;	/* This value will hold the number of columns that we will NOT printout (they are only needed to calculate auxiliary values) */
	if (need_distances || need_lonlat) {	/* Must make sure we get lon,lat if they are not already requested */
		 if (MGD77_Get_Column (GMT, "lat", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",lat"), n_sub++;	/* Append lat to requested list */
		 if (MGD77_Get_Column (GMT, "lon", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",lon"), n_sub++;	/* Append lon to requested list */
	}
	if ((Ctrl->D.active || need_time || auxlist[MGD77_AUX_SP].requested) && MGD77_Get_Column (GMT, "time", &M) == MGD77_NOT_SET) strcat (fx_setting, ",time"), n_sub++;	/* Append time to requested list */
	need_twt = (auxlist[MGD77_AUX_CT].requested || (Ctrl->A.code[ADJ_CT] > 0 && Ctrl->A.code[ADJ_CT] <= CT_U_MINUS_CARTER) ||
	            (Ctrl->A.code[ADJ_DP] > DP_DEPTH_STORED));
	if (need_twt) {	/* Want to estimate Carter corrections */
		 if (MGD77_Get_Column (GMT, "twt", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",twt"), n_sub++;	/* Must append twt to requested list */
		MGD77_carter_init (GMT, &Carter);	/* Initialize Carter machinery */
	}
	need_depth = ((Ctrl->A.code[ADJ_CT] & (CT_U_MINUS_DEPTH | CT_UCORR_MINUS_CARTER_TU | CT_UCORR_CARTER_TU_MINUS_DEPTH)) ||
	              (Ctrl->A.code[ADJ_DP] & DP_DEPTH_STORED));
	if (need_depth) {                   /* Need depth*/
		if (MGD77_Get_Column (GMT, "depth", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",depth"), n_sub++;	/* Must append depth to requested list */
	}
	if (Ctrl->A.code[ADJ_GR] > GR_FAA_STORED) {     /* Need gobs */
		if (MGD77_Get_Column (GMT, "gobs", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",gobs"), n_sub++;	/* Must append gobs to requested list */
	}
	if (Ctrl->A.code[ADJ_GR] & GR_OBS_PLUS_EOT_MINUS_NGRAV) {    /* Need stored eot */
		if (MGD77_Get_Column (GMT, "eot", &M) == MGD77_NOT_SET) strcat (fx_setting, ",eot"), n_sub++;	/* Must append eot to requested list */
	}
	if (Ctrl->A.code[ADJ_MG] > MG_MAG_STORED) {     /* Need mtf1,2, and msens */
		if (MGD77_Get_Column (GMT, "mtf1", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",mtf1"), n_sub++;	/* Must append mtf1 to requested list */
		if (MGD77_Get_Column (GMT, "mtf2", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",mtf2"), n_sub++;	/* Must append mtf2 to requested list */
		if (MGD77_Get_Column (GMT, "msens", &M) == MGD77_NOT_SET)
		 	strcat (fx_setting, ",msens"), n_sub++;	/* Must append msens to requested list */
		if (Ctrl->A.code[ADJ_MG] >= 8) {     /* Need mtf1,2, msens and diur */
			if (MGD77_Get_Column (GMT, "diur", &M) == MGD77_NOT_SET)
		 		strcat (fx_setting, ",diur"), n_sub++;	/* Must append diur to requested list */
		}
	}
	else if (Ctrl->A.cable_adjust)
		 if (MGD77_Get_Column (GMT, "mtf1", &M) == MGD77_NOT_SET) strcat (fx_setting, ",mtf1"), n_sub++;	/* Must append mtf1 to requested list */

	/* If logical tests are specified we must make sure the required columns are included as auxiliary */
	for (kk = 0; kk < M.n_constraints; kk++) {
		if (MGD77_Get_Column (GMT, M.Constraint[kk].name, &M) != MGD77_NOT_SET) continue;	/* OK, already included */
		strcat (fx_setting, ",");
		strcat (fx_setting, M.Constraint[kk].name);	/* Must add to our list */
		n_sub++;
	}
	need_sound = (((Ctrl->A.code[ADJ_CT] & (CT_U_MINUS_DEPTH | CT_U_MINUS_CARTER | CT_UCORR_CARTER_TU_MINUS_DEPTH)) || Ctrl->A.code[ADJ_DP] & DP_TWT_X_V) && Ctrl->A.sound_speed == 0.0);
	Ctrl->A.sound_speed *= 0.5;	/* Takes care of the 2 in 2-way travel time */
	MGD77_Select_Columns (GMT, fx_setting, &M, 0);	/* Only deal with col names - leave constraints/exacts unchanged from last call */
	n_cols_to_process = M.n_out_columns - n_sub;

	MGD77_Set_Unit (GMT, Ctrl->N.unit[N_D], &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */
	MGD77_Set_Unit (GMT, Ctrl->N.unit[N_S], &vel_scale,  -1);	/* Sets output scale for distances using in velocities */
	switch (Ctrl->N.unit[N_S][0]) {
		case 'e':
			vel_scale /= dist_scale;			/* Must counteract any distance scaling to get meters. dt is in sec so we get m/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(m/s)");
			break;
		case 'f':
			vel_scale /= dist_scale;			/* Must counteract any distance scaling to get feet. dt is in sec so we get feet/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(feet/s)");
			break;
		case 'k':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get km. dt is in sec so 3600 gives km/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(km/hr)");
			break;
		case 'M':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(mi/hr)");
			break;
		case 'n':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(kts)");
			break;
		case 'u':
			vel_scale /= dist_scale;			/* Must counteract any distance scaling to get survey feet. dt is in sec so we get survey feet/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(sfeet/s)");
			break;
	}
	switch (Ctrl->N.unit[N_D][0]) {
		case 'e':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(m)");
			break;
		case 'f':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(feet)");
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
		case 'u':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(surv.feet)");
			break;
	}

	gmt_init_distaz (GMT, GMT_MAP_DIST_UNIT, GMT->common.j.mode, GMT_MAP_DIST);

	Ctrl->S.start *= dist_scale;	Ctrl->S.stop *= dist_scale;	/* Convert the meters to the same units used for cumulative distances */
	if (Ctrl->A.cable_adjust) Ctrl->A.sensor_offset *= dist_scale;


	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		char path[PATH_MAX] = {""};
		if (!Ctrl->L.file) {	/* Try default correction table */
			sprintf (path, "%s/mgd77_corrections.txt", M.MGD77_HOME);
			if (access (path, R_OK)) {
				GMT_Report (API, GMT_MSG_ERROR, "No default MGD77 Correction table (%s) found!\n", path);
				Return (GMT_FILE_NOT_FOUND);
			}
			Ctrl->L.file = path;
		}
		MGD77_Parse_Corrtable (GMT, Ctrl->L.file, list, n_paths, M.n_out_columns, M.desired_column, 2, &CORR);
	}
	if (n_paths > 1) gmt_set_segmentheader (GMT, GMT_OUT, true);

	if (strstr (Ctrl->F.flags, "id") || strstr (Ctrl->F.flags, "ngdcid") || strstr (Ctrl->F.flags, "sln")
	    || strstr (Ctrl->F.flags, "sspn") || strstr (Ctrl->F.flags, "date") || strstr (Ctrl->F.flags, "recno"))
		string_output = true;

	geometry = (string_output) ? GMT_IS_NONE : GMT_IS_POINT;
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	if (!string_output) GMT_Set_Columns (API, GMT_OUT, n_out_columns, GMT_COL_FIX_NO_TEXT);

	for (argno = 0; argno < (unsigned int)n_paths; argno++) {		/* Process each ID */

		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;

		GMT_Report (API, GMT_MSG_INFORMATION, "Now processing cruise %s\n", list[argno]);

		D = MGD77_Create_Dataset (GMT);

		error = MGD77_Read_Header_Record (GMT, list[argno], &M, &D->H);
		if (error) {
			if (error == MGD77_ERROR_NOSUCHCOLUMN)
				GMT_Report (API, GMT_MSG_WARNING, "One or more requested columns not present in cruise %s - skipping\n", list[argno]);
			else
				GMT_Report (API, GMT_MSG_ERROR, "Failure while reading header sequence for cruise %s - skipping\n", list[argno]);
			MGD77_Free_Dataset (GMT, &D);
			continue;
		}

		/* Having the header we can process -F and assign indices that refers to this particular data set */


		if (first_cruise) {
			for (kk = 0, string_output = false; kk < n_cols_to_process; kk++) {	/* Prepare GMT output formatting machinery */
				if (D->H.info[M.order[kk].set].col[M.order[kk].item].text) string_output = true;
			}
			if (auxlist[MGD77_AUX_ID].requested || auxlist[MGD77_AUX_DA].requested) string_output = true;
			if (string_output && GMT->common.b.active[1]) {
				GMT_Report (API, GMT_MSG_ERROR, "Cannot specify binary output with text fields\n");
				MGD77_Free_Dataset (GMT, &D);
				Return (GMT_RUNTIME_ERROR);
			}
			if (string_output)
				Out = gmt_new_record (GMT, NULL, record);
			else {
				out = gmt_M_memory (GMT, NULL, n_out_columns, double);
				Out = gmt_new_record (GMT, out, NULL);
			}
		}

		if (MGD77_Read_Data (GMT, list[argno], &M, D)) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading data set for cruise %s\n", list[argno]);
			MGD77_Free_Dataset (GMT, &D);
			Return (GMT_DATA_READ_ERROR);
		}
		MGD77_Close_File (GMT, &M);

		/* The 1*, 2*, 3* below is just there to ensure we don't end up with multiple cases all == MGD77_NOT_SET */
		time_column = ((i = MGD77_Get_Column (GMT, "time", &M)) != MGD77_NOT_SET && M.order[i].set == MGD77_M77_SET) ? M.order[i].item : 1 * MGD77_NOT_SET;
		lon_column  = ((i = MGD77_Get_Column (GMT, "lon",  &M)) != MGD77_NOT_SET && M.order[i].set == MGD77_M77_SET) ? M.order[i].item : 2 * MGD77_NOT_SET;
		lat_column  = ((i = MGD77_Get_Column (GMT, "lat",  &M)) != MGD77_NOT_SET && M.order[i].set == MGD77_M77_SET) ? M.order[i].item : 3 * MGD77_NOT_SET;

		if (time_column != MGD77_NOT_SET && GMT->common.b.active[GMT_OUT] && gmt_M_is_verbose (GMT, GMT_MSG_WARNING) && first_warning) {	/* Warn that binary time output is in Unix secs */
			GMT_Report (API, GMT_MSG_WARNING, "For binary output, time is stored as seconds since 1970 (Use TIME_SYSTEM=Unix to decode)\n");
			first_warning = false;
		}
		for (kk = kx = pos = 0; pos < n_out_columns; kk++, pos++) {	/* Prepare GMT output formatting machinery */
			while (kx < n_aux && aux[kx].pos == kk) {	/* Insert formatting for auxiliary column (none are special) */
				gmt_set_column (GMT, GMT_OUT, pos, GMT_IS_FLOAT);
				pos++, kx++;
			}
			if (kk >= n_cols_to_process) continue;	/* Don't worry about helper columns that won't be printed */
			c  = M.order[kk].set;
			id = M.order[kk].item;
			if (c == MGD77_M77_SET && id == time_column)	/* Special time formatting */
				gmt_set_column (GMT, GMT_OUT, pos, M.time_format);
			else if (c == MGD77_M77_SET && id == lon_column)	/* Special lon formatting */
				gmt_set_column (GMT, GMT_OUT, pos, GMT_IS_LON);
			else if (c == MGD77_M77_SET && id == lat_column)	/* Special lat formatting */
				gmt_set_column (GMT, GMT_OUT, pos, GMT_IS_LAT);
			else 		/* Everything else is float (not true for the 3 strings though but dealt with separately) */
				gmt_set_column (GMT, GMT_OUT, pos, GMT_IS_FLOAT);
		}

		if (first_cruise && !GMT->common.b.active[GMT_OUT] && GMT->current.setting.io_header[GMT_OUT]) {	/* Write out header record */
			for (kk = kx = pos = 0, sep_flag = 10; pos < n_out_columns; kk++, pos++) {
				while (kx < n_aux && aux[kx].pos == kk) {	/* Insert auxiliary column */
					gmt_cat_to_record (GMT, record, auxlist[aux[kx].type].header, GMT_OUT, sep_flag);
					pos++, kx++;
					sep_flag = 1;
				}
				if (kk >= n_cols_to_process) continue;
				c  = M.order[kk].set;
				id = M.order[kk].item;
				gmt_cat_to_record (GMT, record, auxlist[aux[kx].type].header, GMT_OUT, sep_flag);
				sprintf (word, "%7s", D->H.info[c].col[id].abbrev);
				gmt_cat_to_record (GMT, record, word, GMT_OUT, sep_flag);
				sep_flag = 1;
			}
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
		}
		first_cruise = false;

		if (n_paths > 1)	/* Write segment header between each cruise */
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, list[argno]);

		aux_dvalue[MGD77_AUX_DS] = cumulative_dist = ds = 0.0;
		if (auxlist[MGD77_AUX_ID].requested) strncpy (aux_tvalue[MGD77_AUX_ID], M.NGDC_id, GMT_LEN64);

		t_col = MGD77_Get_Column (GMT, "time",   &M);
		x_col = MGD77_Get_Column (GMT, "lon",    &M);
		y_col = MGD77_Get_Column (GMT, "lat",    &M);
		z_col = MGD77_Get_Column (GMT, "depth",  &M);
		if (need_twt) twt_col = MGD77_Get_Column (GMT, "twt",  &M);
		if (Ctrl->A.code[ADJ_GR]) f_col = MGD77_Get_Column (GMT, "faa",  &M);
		if (Ctrl->A.code[ADJ_GR] > GR_FAA_STORED) g_col = MGD77_Get_Column (GMT, "gobs",  &M);
		if (Ctrl->A.code[ADJ_GR] & GR_OBS_PLUS_EOT_MINUS_NGRAV) e_col = MGD77_Get_Column (GMT, "eot",  &M);
		if (Ctrl->A.code[ADJ_MG]) m_col = MGD77_Get_Column (GMT, "mag",  &M);
		if (Ctrl->A.code[ADJ_MG] > MG_MAG_STORED || Ctrl->A.cable_adjust) {	/* Need more magnetics items */
			m1_col = MGD77_Get_Column (GMT, "mtf1",  &M);
			m2_col = MGD77_Get_Column (GMT, "mtf2",  &M);
			ms_col = MGD77_Get_Column (GMT, "msens",  &M);
			md_col = MGD77_Get_Column (GMT, "diur",  &M);
		}
		if ((auxlist[MGD77_AUX_GR].requested || (Ctrl->A.code[ADJ_GR] > GR_FAA_STORED)) && Ctrl->A.GF_version == MGD77_NOT_SET) {
			Ctrl->A.GF_version = D->H.mgd77[use]->Gravity_Theoretical_Formula_Code - '0';
			if (Ctrl->A.GF_version < MGD77_IGF_HEISKANEN || Ctrl->A.GF_version > MGD77_IGF_1980) {
				GMT_Report (API, GMT_MSG_WARNING, "Invalid Gravity Theoretical Formula Code (%c) - default to %d\n", D->H.mgd77[use]->Gravity_Theoretical_Formula_Code, MGD77_IGF_1980);
				Ctrl->A.GF_version = MGD77_IGF_1980;
			}
		}
		for (kk = 0; kk < M.n_out_columns; kk++) {
			dvalue[kk] = D->values[kk];
			tvalue[kk] = D->values[kk];
		}

		this_limit_on_time = Ctrl->D.active;	/* Since we might change it below */
		if (time_column != MGD77_NOT_SET && D->H.no_time) {	/* Cannot know if ASCII MGD77 don't have time until after reading */
			bool faked = false;
			if (Ctrl->A.fake_times) {	/* Try to make fake times based on duration and distances */
				faked = MGD77_fake_times (GMT, &M, &(D->H), dvalue[x_col], dvalue[y_col], dvalue[t_col], D->H.n_records);
				if (faked) GMT_Report (API, GMT_MSG_WARNING, "Time column for cruise %s created from distances and duration\n", list[argno]);
			}
			if (!faked) {
				GMT_Report (API, GMT_MSG_WARNING, "Time column not present in cruise %s - set to NaN\n", list[argno]);
				if (this_limit_on_time) GMT_Report (API, GMT_MSG_WARNING, "-D limits cannot be used for cruise %s\n", list[argno]);
			}
			if (!faked && !Ctrl->D.mode) this_limit_on_time = false;	/* To avoid pointless tests against NaN in loop */
		}
		if (need_sound) {	/* We opted to go with the value in the header [or 1500] */
			v = atof (D->H.mgd77[use]->Bathymetry_Assumed_Sound_Velocity) * 0.1;
			Ctrl->A.sound_speed = 0.5 * ((v < 1400.0 || v > 1600.0) ? 1500.0 : v);
		}

		if (Ctrl->A.sound_speed > 0.0) i_sound_speed = 1.0 / Ctrl->A.sound_speed;

		if (Ctrl->L.active) MGD77_Init_Correction (GMT, CORR[argno], dvalue);	/* Initialize origins if needed */

		has_prev_twt = PDR_wrap = false;
		twt_pdrwrap_corr = 0.0;

		/* Start processing records  */

		prevrec = UINTMAX_MAX;	/* Not determined */
		for (rec = 0; rec < D->H.n_records; rec++) {

			/* Compute accumulated distance along track (Great circles or Flat Earth) */

			if (Ctrl->A.cable_adjust && rec == 0) {
			/* For the cable correction we need to know ALL cumulative distances. So compute them now. */
				uint64_t rec_;
				cumdist = gmt_M_memory(GMT, NULL, D->H.n_records, double);
				mtf_bak = gmt_M_memory(GMT, NULL, D->H.n_records, double);       /* We need a copy */
				mtf_int = gmt_M_memory(GMT, NULL, D->H.n_records, double);       /* And another to store reinterped mtf1 */
				cumdist_off = gmt_M_memory(GMT, NULL, D->H.n_records, double);   /* To put positions where mag was really measured */
				lonlat_not_NaN = !( gmt_M_is_dnan (dvalue[x_col][0]) || gmt_M_is_dnan (dvalue[y_col][0]));
				prevrec = 0;
				mtf_bak[0] = dvalue[m1_col][0];
				for (rec_ = 1; rec_ < D->H.n_records; rec_++) {	/* Very bad luck if first rec has NaNs in coords */
					ds = dist_scale * gmt_distance (GMT, dvalue[x_col][rec_], dvalue[y_col][rec_], dvalue[x_col][prevrec], dvalue[y_col][prevrec]);
					cumulative_dist += ds;
					cumdist[rec_] = cumulative_dist;
					mtf_bak[rec_] = dvalue[m1_col][rec_];	/* Make a copy */
					if (lonlat_not_NaN) prevrec = rec_;
				}
				prevrec = UINTMAX_MAX;	/* Reset for eventual reuse in (need_distances) */
				cumulative_dist = 0;
			}

			if (need_distances) {
				lonlat_not_NaN = !( gmt_M_is_dnan (dvalue[x_col][rec]) || gmt_M_is_dnan (dvalue[y_col][rec]));
				if (rec == 0) {	/* Azimuth at 1st point set to azimuth of 2nd point since there is no previous point */
					if (auxlist[MGD77_AUX_AZ].requested) aux_dvalue[MGD77_AUX_AZ] = gmt_az_backaz (GMT, dvalue[x_col][1], dvalue[y_col][1], dvalue[x_col][0], dvalue[y_col][0], true);
					if (auxlist[MGD77_AUX_CC].requested) {	/* Course change requires previous azimuth but none is available yet */
						aux_dvalue[MGD77_AUX_CC] = GMT->session.d_NaN;
						prev_az = (auxlist[MGD77_AUX_AZ].requested) ? aux_dvalue[MGD77_AUX_AZ] : gmt_az_backaz (GMT, dvalue[x_col][1], dvalue[y_col][1], dvalue[x_col][0], dvalue[y_col][0], true);
					}
					ds0 = dist_scale * gmt_distance (GMT, dvalue[x_col][1], dvalue[y_col][1], dvalue[x_col][0], dvalue[y_col][0]);
				}
				else {		/* Need a previous point to calculate distance and heading */
					if (lonlat_not_NaN && prevrec != UINTMAX_MAX) {	/* We have to records with OK lon,lat and can compute a distance from the previous OK point */
						ds = dist_scale * gmt_distance (GMT, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[x_col][prevrec], dvalue[y_col][prevrec]);
						if (auxlist[MGD77_AUX_AZ].requested) aux_dvalue[MGD77_AUX_AZ] = (auxlist[MGD77_AUX_CC].requested) ? prev_az : gmt_az_backaz (GMT, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[x_col][prevrec], dvalue[y_col][prevrec], true);
						cumulative_dist += ds;
						aux_dvalue[MGD77_AUX_DS] = cumulative_dist;
					}
					else {
						aux_dvalue[MGD77_AUX_DS] = GMT->session.d_NaN;
						if (auxlist[MGD77_AUX_AZ].requested) aux_dvalue[MGD77_AUX_AZ] = GMT->session.d_NaN;
					}
					if (auxlist[MGD77_AUX_CC].requested) {	/* Course change requires previous and next azimuth */
						if (rec < (D->H.n_records - 1)) {
							next_az = gmt_az_backaz (GMT, dvalue[x_col][rec+1], dvalue[y_col][rec+1], dvalue[x_col][rec], dvalue[y_col][rec], true);
							gmt_M_set_delta_lon (prev_az, next_az, aux_dvalue[MGD77_AUX_CC]);
							prev_az = next_az;
						}
						else	/* No next azimuth possible */
							aux_dvalue[MGD77_AUX_CC] = GMT->session.d_NaN;
					}
				}
				if (auxlist[MGD77_AUX_SP].requested) {
					if (rec == 0 || prevrec == UINTMAX_MAX) {	/* Initialize various counters */
						dt = dvalue[t_col][1] - dvalue[t_col][0];
						aux_dvalue[MGD77_AUX_SP] = (gmt_M_is_dnan (dt) || dt == 0.0) ? GMT->session.d_NaN : vel_scale * ds0 / dt;
					}
					else {		/* Need a previous point to calculate speed */
						dt = dvalue[t_col][rec] - dvalue[t_col][prevrec];
						aux_dvalue[MGD77_AUX_SP] = (gmt_M_is_dnan (dt) || dt == 0.0) ? GMT->session.d_NaN : vel_scale * ds / dt;
					}
				}
				if (lonlat_not_NaN) prevrec = rec;	/* This was a record with OK lon,lat; make it the previous point for distance calculations */
			}
			if (auxlist[MGD77_AUX_ET].requested) aux_dvalue[MGD77_AUX_ET] = MGD77_Eotvos (GMT, dvalue[y_col][rec], aux_dvalue[MGD77_AUX_SP], aux_dvalue[MGD77_AUX_AZ]);
			if (auxlist[MGD77_AUX_RN].requested) aux_dvalue[MGD77_AUX_RN] = (double)rec;

			/* Check if rec no, time or distance falls outside specified ranges */

			if (Ctrl->G.active && (rec < Ctrl->G.start || rec > Ctrl->G.stop)) continue;
			if (Ctrl->S.active && (cumulative_dist < Ctrl->S.start || cumulative_dist >= Ctrl->S.stop)) continue;
			if (Ctrl->D.mode && gmt_M_is_dnan (dvalue[t_col][rec])) continue;
			if (this_limit_on_time && (dvalue[t_col][rec] < Ctrl->D.start || dvalue[t_col][rec] >= Ctrl->D.stop)) continue;
			if (GMT->common.R.active[RSET]) {	/* Check is lat/lon is outside specified area */
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
			if (Ctrl->Q.active[Q_C]) {	/* Check if we are outside course change range */
				this_cc = (Ctrl->Q.c_abs) ? fabs (aux_dvalue[MGD77_AUX_CC]) : aux_dvalue[MGD77_AUX_CC];
				if (this_cc < Ctrl->Q.min[Q_C] || this_cc > Ctrl->Q.max[Q_C]) continue;
			}
			/* Check if it passes any given column data constraints */

			if (!MGD77_Pass_Record (GMT, &M, D, rec)) continue;	/* Failed the test */

			/* This record will now be printed out */

			if (need_time) {	/* Need auxiliary time columns such as year, days etc, hence we get the calendar first, then use MGD77_cal_to_fyear */
				MGD77_gcal_from_dt (GMT, &M, dvalue[t_col][rec], &cal);	/* No adjust for TZ; this is GMT UTC time */
				aux_dvalue[MGD77_AUX_YR] = (double)cal.year;
				aux_dvalue[MGD77_AUX_MO] = (double)cal.month;
				aux_dvalue[MGD77_AUX_DY] = (double)cal.day_m;
				aux_dvalue[MGD77_AUX_HR] = (double)cal.hour;
				aux_dvalue[MGD77_AUX_MI] = (double)cal.min;
				aux_dvalue[MGD77_AUX_SC] = cal.sec;
				aux_dvalue[MGD77_AUX_DM] = cal.min + cal.sec / 60.0;
				aux_dvalue[MGD77_AUX_HM] = 100.0 * cal.hour + aux_dvalue[MGD77_AUX_DM];
				date = MGD77_cal_to_fyear (GMT, &cal);	/* Get date as decimal year */
				if (auxlist[MGD77_AUX_DA].requested) sprintf (aux_tvalue[MGD77_AUX_DA], "%04d%02d%02d", cal.year, cal.month, cal.day_m);
				need_date = false;
			}
			else
				need_date = true;

			if (auxlist[MGD77_AUX_MG].requested) {	/* Evaluate IGRF */
				double date = 0.0;
				date = MGD77_cal_to_fyear (GMT, &cal);	/* Get date as decimal year */
				aux_dvalue[MGD77_AUX_MG] = (MGD77_igrf10syn (GMT, 0, date, 1, 0.0, dvalue[x_col][rec], dvalue[y_col][rec], IGRF)) ? GMT->session.d_NaN : IGRF[MGD77_IGRF_F];
			}

			if (auxlist[MGD77_AUX_GR].requested)	/* Evaluate Theoretical Gravity Model */
				aux_dvalue[MGD77_AUX_GR] = MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);

			/* --------------------------------------------------------------------------------------------------- */
			/*                 See if we have a request to adjust the carter value                                 */
			/* --------------------------------------------------------------------------------------------------- */
			if (auxlist[MGD77_AUX_CT].requested) {	/* Carter is one of the output columns */
				if (Ctrl->A.code[ADJ_CT]) {	/* We have requested some adjustment to the carter value */
					aux_dvalue[MGD77_AUX_CT] = GMT->session.d_NaN;
					if (Ctrl->A.code[ADJ_CT] & CT_U_MINUS_DEPTH)	/* Try uncorr. depth - obs. depth */
						aux_dvalue[MGD77_AUX_CT] = dvalue[twt_col][rec] * Ctrl->A.sound_speed - dvalue[z_col][rec];	/* Factor of 2 dealt with earlier */
					if ((Ctrl->A.code[ADJ_CT] & CT_U_MINUS_CARTER) && gmt_M_is_dnan (aux_dvalue[MGD77_AUX_CT])) {	/* Try uncorr. depth - Carter depth */
						MGD77_carter_depth_from_xytwt (GMT, dvalue[x_col][rec], dvalue[y_col][rec], 1000.0 * dvalue[twt_col][rec], &Carter, &z);
						aux_dvalue[MGD77_AUX_CT] = dvalue[twt_col][rec] * i_sound_speed - z;
					}
					if ((Ctrl->A.code[ADJ_CT] & CT_UCORR_MINUS_CARTER_TU) && gmt_M_is_dnan (aux_dvalue[MGD77_AUX_CT])) {	/* Try uncorr. depth - inferred Carter depth */
						twt = dvalue[z_col][rec] * i_sound_speed;	/* Factor of 2 dealt with earlier */
						MGD77_carter_depth_from_xytwt (GMT, dvalue[x_col][rec], dvalue[y_col][rec], twt, &Carter, &z);
						aux_dvalue[MGD77_AUX_CT] = dvalue[z_col][rec] - z;
					}
					if ((Ctrl->A.code[ADJ_CT] & CT_UCORR_CARTER_TU_MINUS_DEPTH) && gmt_M_is_dnan (aux_dvalue[MGD77_AUX_CT])) {	/* Try inferred uncorr. depth - obs. depth */
						MGD77_carter_twt_from_xydepth (GMT, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[z_col][rec], &Carter, &twt);
						z = twt * Ctrl->A.sound_speed;
						aux_dvalue[MGD77_AUX_CT] = z - dvalue[z_col][rec];
					}
				}
				else {
					twt = 1000.0 * dvalue[twt_col][rec];
					aux_dvalue[MGD77_AUX_CT] = MGD77_carter_correction (GMT, dvalue[x_col][rec], dvalue[y_col][rec], twt, &Carter);
				}
				if (!negative_depth) aux_dvalue[MGD77_AUX_CT] = -aux_dvalue[MGD77_AUX_CT];	/* Since we report correction to be ADDED */
			}

			/* --------------------------------------------------------------------------------------------------- */
			/*                 See if we have a request to adjust the depth value                                  */
			/* --------------------------------------------------------------------------------------------------- */
			if (z_col != MGD77_NOT_SET && Ctrl->A.code[ADJ_DP]) {
				z = GMT->session.d_NaN;
				if (Ctrl->A.code[ADJ_DP] & DP_DEPTH_STORED)	/* Try obs. depth */
					z = dvalue[z_col][rec];
				if ((Ctrl->A.code[ADJ_DP] & DP_TWT_X_V) && gmt_M_is_dnan (z))	/* Try uncorr. depth */
					z = dvalue[twt_col][rec] * i_sound_speed;
				if ((Ctrl->A.code[ADJ_DP] & DP_TWT_X_V_MINUS_CARTER) && gmt_M_is_dnan (z)) {	/* Try Carter depth */
					twt = dvalue[twt_col][rec];
					if (!gmt_M_is_dnan (twt)) {	/* OK, valid twt */
						if (has_prev_twt) {	/* OK, may look at change in twt */
							d_twt = twt - prev_twt;
							if (fabs (d_twt) > TWT_PDR_WRAP_TRIGGER) {
								twt_pdrwrap_corr += copysign (TWT_PDR_WRAP, -d_twt);
								if (!PDR_wrap) GMT_Report (API, GMT_MSG_WARNING, "PDR travel time wrap detected for cruise %s\n", list[argno]);
								PDR_wrap = true;
							}
						}
						has_prev_twt = true;
						prev_twt = twt;
					}
					twt += twt_pdrwrap_corr;
					MGD77_carter_depth_from_xytwt (GMT, dvalue[x_col][rec], dvalue[y_col][rec], 1000.0 * twt, &Carter, &z);
				}
				if (Ctrl->A.force || !gmt_M_is_dnan(dvalue[z_col][rec])) dvalue[z_col][rec] = z;
			}

			/* --------------------------------------------------------------------------------------------------- */
			/*                 See if we have a request to adjust the faa value                                    */
			/* --------------------------------------------------------------------------------------------------- */
			if (f_col != MGD77_NOT_SET && Ctrl->A.code[ADJ_GR]) {
				g = GMT->session.d_NaN;
				if (Ctrl->A.code[ADJ_GR] & GR_FAA_STORED)	/* Try faa */
					g = dvalue[f_col][rec];
				if ((Ctrl->A.code[ADJ_GR] & GR_OBS_MINUS_NGRAV) && gmt_M_is_dnan (g))	/* Try gobs - ngrav */
					g = dvalue[g_col][rec] - MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);
				if ((Ctrl->A.code[ADJ_GR] & GR_OBS_PLUS_EOT_MINUS_NGRAV )&& gmt_M_is_dnan (g))	/* Try gobs + eot - ngrav */
					g = dvalue[g_col][rec] + dvalue[e_col][rec] - MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);
				if ((Ctrl->A.code[ADJ_GR] & GR_OBS_PLUS_CEOT_MINUS_NGRAV) && gmt_M_is_dnan (g))	/* Try gobs + pred_eot - ngrav */
					g = dvalue[g_col][rec] + MGD77_Eotvos (GMT, dvalue[y_col][rec], aux_dvalue[MGD77_AUX_SP], aux_dvalue[MGD77_AUX_AZ]) - MGD77_Theoretical_Gravity (GMT, dvalue[x_col][rec], dvalue[y_col][rec], (int)Ctrl->A.GF_version);
				if (Ctrl->A.force || !gmt_M_is_dnan(dvalue[f_col][rec])) dvalue[f_col][rec] = g;
			}

			/* --------------------------------------------------------------------------------------------------- */
			/*                 See if we have a request to adjust the mag value                                  */
			/* --------------------------------------------------------------------------------------------------- */
			if (m_col != MGD77_NOT_SET && Ctrl->A.code[ADJ_MG]) {
				m = GMT->session.d_NaN;
				if (Ctrl->A.code[ADJ_MG] & MG_MAG_STORED)	/* Try mag as is */
					m = dvalue[m_col][rec];
				if ((Ctrl->A.code[ADJ_MG] & MG_MTF1_MINUS_IGRF) && gmt_M_is_dnan (m)) {	/* Try mtf 1st - igrf */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = false;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m2_col : m1_col;
					m = MGD77_Recalc_Mag_Anomaly_IGRF (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], false);
				}
				if ((Ctrl->A.code[ADJ_MG] & MG_MTF2_MINUS_IGRF) && gmt_M_is_dnan (m)) {	/* Try mtf 2nd - igrf */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = false;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m1_col : m2_col;
					m = MGD77_Recalc_Mag_Anomaly_IGRF (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], false);
				}
				if ((Ctrl->A.code[ADJ_MG] & MG_MTF1_PLUS_DIUR_MINUS_IGRF) && gmt_M_is_dnan (m)) {	/* Try mtf 1st + diur - igrf */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = false;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m2_col : m1_col;
					m = dvalue[md_col][rec] + MGD77_Recalc_Mag_Anomaly_IGRF (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], false);
				}
				if ((Ctrl->A.code[ADJ_MG] & MG_MTF2_PLUS_DIUR_MINUS_IGRF) && gmt_M_is_dnan (m)) {	/* Try mtf 2nd + diur - igrf */
					if (need_date) {	/* Did not get computed already */
						date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
						need_date = false;
					}
					i = irint (dvalue[ms_col][rec]);
					k = (i == 2) ? m1_col : m2_col;
					m = dvalue[md_col][rec] + MGD77_Recalc_Mag_Anomaly_IGRF (GMT, &M, date, dvalue[x_col][rec], dvalue[y_col][rec], dvalue[k][rec], false);
				}
				if (Ctrl->A.force || !gmt_M_is_dnan(dvalue[m_col][rec])) dvalue[m_col][rec] = m;
			}

			/* --------------------------------------------------------------------------------------------------- */
			/*                See if we have a request to adjust for magnetometer offset                           */
			/* --------------------------------------------------------------------------------------------------- */
			if (m1_col != MGD77_NOT_SET && Ctrl->A.cable_adjust) {
				if (Ctrl->A.sensor_offset == 0)             /* Accept also this case to easy life with script writing */
					dvalue[m1_col][rec] = mtf_bak[rec];         /* Means, copy mtf1 into mtf2 */
				else {
					if (first_time_on_sensor_offset) {  /* At first time here we interpolate ALL mtf1 at offset pos */
						int n, *ind = NULL;
						uint64_t k_off, last_k = 0;
						bool clean = true;
						double off_rescue = 0.0001;
						double *cumdist_off_cl = NULL, *cumdist_cl = NULL, *mtf_int_cl = NULL, *mtf_cl = NULL;

						for (k_off = 1; k_off < D->H.n_records; k_off++) {
							/* Often cruises have repeated points that will prevent gmt_intpol usage because dx = 0
							   We will workaround it by adding a epsilon (.1 meter) to the repeated pt. However,
							   often the situation is further complicated because repeat points can come in large
							   packs. For those cases we add an increasingly small offset. But when the number of
							   repetitions are large, even this strategy fails and we get error from gmt_intpol */
							if ((cumdist[k_off] - cumdist[k_off-1]) == 0.0) {
								if ((k_off - last_k) == 1) {
									off_rescue -= 0.000001;	/* Slightly and incrementally reduce the move away offset */
								}
								else {
									off_rescue = 0.0001;	/* Reset it to the one-repetition-only value */
								}
								cumdist[k_off-1] -= off_rescue;
								last_k = k_off;
							}
							else
								off_rescue = 0.0001;     /* Reset it to the one-repetition-only value */
						}

						for (k_off = 0; k_off < D->H.n_records; k_off++) {
							cumdist_off[k_off] = cumdist[k_off] + Ctrl->A.sensor_offset;
							if (clean && gmt_M_is_dnan (mtf_bak[k_off])) clean = false;
						}

						/* --------------- Attack the NaNs problem -----------------*/
						if (clean)		/* Nice, no NaNs at sight */
							gmt_intpol(GMT, cumdist, mtf_bak, D->H.n_records, D->H.n_records, cumdist_off, mtf_int, GMT->current.setting.interpolant);
						else {
							/* Need to allocate these auxiliary vectors */
							ind = gmt_M_memory(GMT, NULL, D->H.n_records, int);
							cumdist_cl = gmt_M_memory(GMT, NULL, D->H.n_records, double);
							cumdist_off_cl = gmt_M_memory(GMT, NULL, D->H.n_records, double);
							mtf_cl = gmt_M_memory(GMT, NULL, D->H.n_records, double);
							mtf_int_cl = gmt_M_memory(GMT, NULL, D->H.n_records, double);

							for (k_off = n = 0; k_off < D->H.n_records; k_off++) {
								ind[k_off] = !gmt_M_is_dnan (mtf_bak[k_off]);  /* Find indices of valid values */
								if (ind[k_off]) {
									cumdist_cl[n] = cumdist[k_off];          /* Copy valid values into a contiguous vec */
									cumdist_off_cl[n] = cumdist_off[k_off];
									mtf_cl[n] = mtf_bak[k_off];
									n++;
								}
							}
							gmt_intpol(GMT, cumdist_cl, mtf_cl, n, n, cumdist_off_cl, mtf_int_cl, GMT->current.setting.interpolant);
							for (k_off = n = 0; k_off < D->H.n_records; k_off++) {
								if (ind[k_off])
									mtf_int[k_off] = mtf_int_cl[n++];
								else
									mtf_int[k_off] = GMT->session.d_NaN;
							}
						}

						dvalue[m1_col][rec] = mtf_int[rec];
						/* We can free these right now because they won't be used anymore for this file */
						gmt_M_free (GMT, ind);
						gmt_M_free (GMT, cumdist_cl);         gmt_M_free (GMT, cumdist_off_cl);
						gmt_M_free (GMT, mtf_cl);             gmt_M_free (GMT, mtf_int_cl);
						first_time_on_sensor_offset = false;
					}
					else                               /* All other times, just pull out current val of interped mtf1 */
						dvalue[m1_col][rec] = mtf_int[rec];
				}
			}

			if (negative_depth) dvalue[z_col][rec] = -dvalue[z_col][rec];
			if (negative_msd) dvalue[m_col][rec] = -dvalue[m_col][rec];

			if (string_output) {	/* Must do it col by col and deal with the requested string(s) */
				record[0] = 0;	/* Start with blank record */
				for (kk = kx = pos = 0, sep_flag = 10; pos < n_out_columns; kk++, pos++) {
					while (kx < n_aux && aux[kx].pos == kk) {	/* Insert auxiliary column */
						if (aux[kx].text)
							gmt_cat_to_record (GMT, record, aux_tvalue[aux[kx].type], GMT_OUT, sep_flag);	/* Format our output x value */
						else
							gmt_add_to_record (GMT, record, aux_dvalue[aux[kx].type], pos, GMT_OUT, sep_flag);	/* Format our output x value */
						sep_flag = 1;
						kx++, pos++;
					}
					if (kk >= n_cols_to_process) continue;
					c  = M.order[kk].set;
					id = M.order[kk].item;
					if (D->H.info[c].col[id].text) {
						strncpy (word, &tvalue[kk][rec*D->H.info[c].col[id].text], D->H.info[c].col[id].text);
						word[D->H.info[c].col[id].text] = 0;
						gmt_cat_to_record (GMT, record, word, GMT_OUT, sep_flag);	/* Format our output x value */
					}
					else if (c == MGD77_M77_SET && id == time_column) {	/* Time */
						if (gmt_M_type (GMT, GMT_OUT, pos) == GMT_IS_FLOAT) {	/* fractional year */
							if (need_date) {	/* Did not get computed already */
								date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
								need_date = false;
							}
						}
						else if (M.adjust_time)
							date = MGD77_utime2time (GMT, &M, dvalue[t_col][rec]);
						else
							date = dvalue[t_col][rec];
						gmt_add_to_record (GMT, record, date, pos, GMT_OUT, sep_flag);	/* Format our output time value */
					}
					else {
						correction = (Ctrl->L.active) ? MGD77_Correction (GMT, CORR[argno][kk].term, dvalue, aux_dvalue, rec) : 0.0;
						gmt_add_to_record (GMT, record, dvalue[kk][rec] - correction, pos, GMT_OUT, sep_flag);	/* Format our output time value */
					}
					sep_flag = 1;
				}
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
			else {	/* Use GMT output machinery which can handle binary output, if requested */
				for (kk = kx = pos = 0; pos < n_out_columns; kk++, pos++) {
					while (kx < n_aux && aux[kx].pos == kk) {	/* Insert auxiliary column */
						out[pos] = aux_dvalue[aux[kx].type];
						pos++, kx++;
					}
					if (kk >= n_cols_to_process) continue;
					c  = M.order[kk].set;
					id = M.order[kk].item;
					if (c == MGD77_M77_SET && id == time_column) {	/* This is the time column */
						if (gmt_M_type (GMT, GMT_OUT, pos) == GMT_IS_FLOAT) {	/* fractional year */
							if (need_date) {	/* Did not get computed already */
								date = MGD77_time_to_fyear (GMT, &M, dvalue[t_col][rec]);
								need_date = false;
							}
							out[pos] = date;
						}
						else if (M.adjust_time)
							out[pos] = MGD77_utime2time (GMT, &M, dvalue[t_col][rec]);
						else
							out[pos] = dvalue[t_col][rec];
					}
					else {
						correction = (Ctrl->L.active) ? MGD77_Correction (GMT, CORR[argno][kk].term, dvalue, aux_dvalue, rec) : 0.0;
						out[pos] = dvalue[kk][rec] - correction;
					}
				}
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
			n_out++;
		}

		if (cumdist) {
			gmt_M_free (GMT, cumdist_off);	/* Free and reset for eventual reuse */
			gmt_M_free (GMT, cumdist);	/* Free and reset for eventual reuse */
			gmt_M_free (GMT, mtf_bak);	/* Free and reset for eventual reuse */
			gmt_M_free (GMT, mtf_int);	/* Free and reset for eventual reuse */
		}
		MGD77_Free_Dataset (GMT, &D);
		n_cruises++;
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (!string_output) gmt_M_free (GMT, out);
	gmt_M_free (GMT, aux_tvalue[MGD77_AUX_ID]);
	gmt_M_free (GMT, aux_tvalue[MGD77_AUX_DA]);
	gmt_M_free (GMT, Out);

	GMT_Report (API, GMT_MSG_INFORMATION, "Returned %d output records from %d cruises\n", n_out, n_cruises);

	MGD77_Path_Free (GMT, (uint64_t)n_paths, list);
	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, n_paths);
	MGD77_end (GMT, &M);

	Return (GMT_NOERROR);
}
