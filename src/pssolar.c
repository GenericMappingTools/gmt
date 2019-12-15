/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis:
 *
 * Author:	Joaquim Luis
 * Date:	1-JAN-2016
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"pssolar"
#define THIS_MODULE_MODERN_NAME	"solar"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot day-light terminators and other sunlight parameters"
#define THIS_MODULE_KEYS	">X},>DI,>DM@ID),MD)"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYbpto" GMT_OPT("c")

struct SUN_PARAMS {
	double EQ_time;
	double SolarNoon;
	double Sunrise;
	double Sunset;
	double Sunlight_duration;
	double SolarElevationCorrected;
	double HourAngle;
	double SolarDec;
	double SolarAzim;
	double SolarElevation;
	double radius;
};

struct PSSOLAR_CTRL {
	struct PSSOL_C {		/* -C */
		bool active;
	} C;
	struct PSSOL_G {		/* -G<fill> */
		bool active;
		bool clip;
		struct GMT_FILL fill;
	} G;
	struct PSSOL_I {		/* -I info about solar stuff */
		bool   active;
		bool   position;
		int    TZ;			/* Time Zone */
		double lon, lat;
		struct GMT_GCAL calendar;
	} I;
	struct PSSOL_M {		/* -M dumps the terminators data instead of plotting them */
		bool active;
	} M;
	struct PSSOL_N {		/* -N */
		bool active;
	} N;
	struct PSSOL_T {		/* -T terminator options */
		bool   active;
		bool   night, civil, nautical, astronomical;
		unsigned int n_terminators;
		int    which;		/* 0 = night, ... 3 = astronomical */
		int    TZ;			/* Time Zone */
		double radius[4];
		struct GMT_GCAL calendar;
	} T;
	struct PSSOL_W {		/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSOLAR_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSSOLAR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->T.radius[0] = 90.833;		/* (for example if -I only) Sun radius (16' + 34.5' from the light refraction effect) */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSSOLAR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL void parse_date_tz(char *date_tz, char **date, int *TZ) {
	unsigned int pos = 0;
	char *p;

	p = malloc(strlen(date_tz)+1);
	while ((gmt_strtok (date_tz, "+", &pos, p))) {
		switch (p[0]) {
			case 'd': date[0] = strdup(&p[1]);	break;
			case 'z': *TZ     = atoi(&p[1]);	break;
		}
	}
	free(p);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pssolar synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [%s] [-C] [-G[<fill>]] [-I[lon/lat][+d<date>][+z<TZ>]] [%s] %s\n", name, GMT_B_OPT, GMT_J_OPT, API->K_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M] [-N] %s ", API->O_OPT);
	GMT_Message (API, GMT_TIME_NONE, "%s[-T<dcna>[+d<date>][+z<TZ>]] [%s]\n", API->P_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W<pen>]\n\t[%s] [%s] [%s]\n\t%s[%s] [%s] [%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT,
	             GMT_X_OPT, GMT_Y_OPT, GMT_b_OPT, API->c_OPT, GMT_o_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Format report selected via -I in a single line of numbers only.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t   6) leave off <fill> to issue clip paths instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Print current sun position. Append lon/lat to print also the times of\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Sunrise, Sunset, Noon and length of the day.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add +d<date> in ISO format, e.g, +d2000-04-25, to compute sun parameters\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for this date. If necessary, append time zone via +z<TZ>.\n");
	GMT_Option (API, "J,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Write terminator(s) as a multisegment ASCII (or binary, see -bo) file to standard output. No plotting occurs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Use the outside of the polygons and the map boundary as clip paths.\n");
	GMT_Option (API, "O,P,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <dcna> Plot (or dump; see -M) one or more terminators defined via these flags:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d means day/night terminator.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c means civil twilight.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n means nautical twilight.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a means astronomical twilight.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add +d<date> in ISO format, e.g, +d2000-04-25, to compute terminators\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for this date. If necessary, append time zone via +z<TZ>.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Specify outline pen attributes [Default is no outline].", 0);
	GMT_Option (API, "X,b,c,o,p");
	GMT_Option (API, "t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSSOLAR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pssolar and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 */

	int    j, TZ = 0, n_files = 0, n_errors = 0;
	char  *pch = NULL, *date = NULL;
	struct GMT_OPTION *opt = NULL;
	double t;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':		/* Format -I output as a vector. No text. */
				Ctrl->C.active = true;
				break;
			case 'G':		/* Set fill for symbols or polygon */
				Ctrl->G.active = true;
				if (opt->arg[0] == '\0' || (opt->arg[0] == 'c' && !opt->arg[1]))
					Ctrl->G.clip = true;
				else if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':		/* Infos -I[x/y][+d<date>][+z<TZ>] */
				Ctrl->I.active = true;
				if (opt->arg[0]) {	/* Also gave location */
					Ctrl->I.position = true;
					if (opt->arg[0] != '+') {		/* Then it must be a location */
						n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->I.lon, &Ctrl->I.lat) != 2,
					                                     "Syntax error: Expected -I[<lon>/<lat>]\n");
					}
					if ((pch = strchr(opt->arg, '+')) != NULL) {		/* Have one or two extra options */
						parse_date_tz(pch, &date, &TZ);
						Ctrl->I.TZ = TZ;
						if (date) {
							gmt_scanf_arg (GMT, date, GMT_IS_ABSTIME, false, &t);
							gmt_gcal_from_dt (GMT, t, &Ctrl->I.calendar);	/* Convert t to a complete calendar structure */
							gmt_M_str_free (date);
						}
					}
				}
				break;
			case 'M':
				Ctrl->M.active = true;
				break;
			case 'N':	/* Use the outside of the polygon as clip area */
				Ctrl->N.active = true;
				break;
			case 'T':		/* -Tdcna[+d<date>][+z<TZ>] */
				Ctrl->T.active = true;
				gmt_M_memset (Ctrl->T.radius, 4, double);	/* Reset to nothing before parsing */
				if ((pch = strchr (opt->arg, '+')) != NULL) {	/* Have one or two extra options */
					parse_date_tz (pch, &date, &TZ);
					Ctrl->T.TZ = TZ;
					if (date) {
						gmt_scanf_arg (GMT, date, GMT_IS_ABSTIME, false, &t);
						gmt_gcal_from_dt (GMT, t, &Ctrl->T.calendar);	/* Convert t to a complete calendar structure */
						gmt_M_str_free (date);
					}
					pch[0] = '\0';	/* Chop off date setting */
				}
				if (opt->arg[0]) {
					for (j = 0; j < (int)strlen(opt->arg); j++) {
						if (opt->arg[j] == 'd')				/* Day-night terminator */
							{Ctrl->T.night = true;          Ctrl->T.radius[0] = 90.833;}
						else if (opt->arg[j] == 'c')        /* Civil terminator */
							{Ctrl->T.civil = true;          Ctrl->T.radius[1] = 90 + 6;}
						else if (opt->arg[j] == 'n')        /* Nautical terminator */
							{Ctrl->T.nautical = true;       Ctrl->T.radius[2] = 90 + 12;}
						else if (opt->arg[j] == 'a')        /* Astronomical terminator */
							{Ctrl->T.astronomical = true;   Ctrl->T.radius[3] = 90 + 18;}
					}
				}
				else 		/* Then the default */
					{Ctrl->T.night = true;		Ctrl->T.radius[0] = 90.833;}
				if (pch) pch[0] = '+';	/* Restore it */
					
				break;
			case 'W':		/* Pen */
				Ctrl->W.active = true;
				if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;

			default:		/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	for (j = 0; j < 4; j++)	/* Count requested terminators */
		if (Ctrl->T.radius[j] > 0.0) Ctrl->T.n_terminators++;	

	if (Ctrl->N.active && GMT->current.map.frame.init) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -B cannot be used in combination with Option -N. -B is ignored.\n");
		GMT->current.map.frame.draw = false;
	}
	if (!Ctrl->I.active && !Ctrl->M.active) {	/* Allow plotting without specifying -R and/or -J */
		if (!GMT->common.J.active) {	/* When no projection specified, use fake linear projection */
			gmt_parse_common_options (GMT, "J", 'J', "X14cd/0d");
			GMT->common.J.active = true;
		}
		if (!GMT->common.R.active[RSET]) {	/*  */
			gmt_parse_common_options (GMT, "R", 'R', "-180/180/-90/90");
			GMT->common.R.active[RSET] = true;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->G.clip, "Syntax error: -N requires -Gc\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.clip && Ctrl->T.n_terminators > 1, "Syntax error: Can only select one terminator when using -Gc\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 0, "Syntax error: No input files allowed\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->T.active + Ctrl->I.active) > 1, "Syntax error: Cannot combine -T and -I\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL int solar_params (struct PSSOLAR_CTRL *Ctrl, struct SUN_PARAMS *Sun) {
	/* Adapted from https://github.com/joa-quim/mirone/blob/master/utils/solar_params.m  */
	/* http://www.esrl.noaa.gov/gmd/grad/solcalc/calcdetails.html */
	/* Compute the day-night terminator and the civil, nautical and astronomical twilights
	   as well as several other solar parameters such sunrise, senset, Sun position, etc... */
	int    TZ, year, month, day, hour, min;
	struct tm *UTC;
	time_t right_now = time (NULL);
	double sec, JC, JD, UT, L, M, C, var_y, r, sz, theta, lambda, obliqCorr, meanObliqEclipt;
	double EEO, HA_Sunrise, TrueSolarTime, SolarDec, radius;

	radius = Ctrl->T.radius[Ctrl->T.which];
	TZ = (Ctrl->I.TZ != 0) ? Ctrl->I.TZ : ((Ctrl->T.TZ != 0) ? Ctrl->I.TZ : 0);

	/*  Date info may be in either of I or T options. If not, use current time. */
	if (Ctrl->I.calendar.year != 0) {
		year = Ctrl->I.calendar.year;		month = Ctrl->I.calendar.month;	day = Ctrl->I.calendar.day_m;
		hour = Ctrl->I.calendar.hour;		min   = Ctrl->I.calendar.min;	sec = Ctrl->I.calendar.sec;
	}
	else if (Ctrl->T.calendar.year != 0) {
		year = Ctrl->T.calendar.year;		month = Ctrl->T.calendar.month;	day = Ctrl->T.calendar.day_m;
		hour = Ctrl->T.calendar.hour;		min   = Ctrl->T.calendar.min;	sec = Ctrl->T.calendar.sec;
	}
	else {
		UTC   = gmtime (&right_now);
		year  = UTC->tm_year + 1900;
		month = UTC->tm_mon + 1;
		day   = UTC->tm_mday;
		hour  = UTC->tm_hour;
		min   = UTC->tm_min;
		sec   = UTC->tm_sec;
	}

	UT = hour + (double)min / 60 + sec / 3600;

	/*  From http://scienceworld.wolfram.com/astronomy/JulianDate.html */
	/* tm_mon is 0-11, so add 1 for 1-12 range, tm_year is years since 1900, so add 1900, but tm_mday is 1-31 so use as is */
	JD = 367.0 * year - floor(7.0 * (year + floor((month + 9.0) / 12) ) / 4) -		/* Julian day */
		floor(3.0 * (floor((year + (month - 9.0) / 7) / 100) + 1 ) / 4) +
		floor((275.0 * month) / 9) + day + 1721028.5 + (UT / 24);

	JC = (JD - 2451545) / 36525;		/* number of Julian centuries since Jan 1, 2000, 12 UT */
	L = 280.46645 + 36000.76983 * JC + 0.0003032 * JC * JC;		/* Sun mean longitude, degree */
	L = fmod(L, 360);
	if (L < 0) L = L + 360;
	M = 357.5291 + 35999.0503 * JC - 0.0001559 * JC * JC - 0.00000048 * JC * JC * JC;		/* mean anomaly, degrees */
	M = fmod(M, 360);
	if (M < 0) M = M + 360;

	M = M * D2R;

	C = (1.914602 - 0.004817*JC - 0.000014 * JC * JC) * sin(M);
	C = C + (0.019993 - 0.000101 * JC) * sin(2 * M) + 0.000289 * sin(3 * M);		/* Sun ecliptic longitude */
	theta = L + C;												/* Sun true longitude, degree */
	meanObliqEclipt =  23.0 + 26.0/60 + 21.448/3600 - (46.815*JC + 0.00059 * JC * JC - 0.001813 * JC * JC * JC) / 3600;
	obliqCorr = meanObliqEclipt + 0.00256 * cos((125.04 - 1934.136 * JC) * D2R);	/* Oblique correction */
	lambda = theta - 0.00569 - 0.00478 * sin((125.04 - 1934.136*JC) * D2R);			/* Sun apparent longitude */
	Sun->SolarDec = asin(sin(obliqCorr * D2R) * sin(lambda * D2R)) / D2R;			/* Sun declination */

	L = L * D2R;
	SolarDec = Sun->SolarDec * D2R;		/* Auxiliary var */

	var_y = tan(obliqCorr/2 * D2R) * tan(obliqCorr/2 * D2R);
	EEO   = 0.016708634 - JC * (0.000042037 + 0.0000001267 * JC);		/* Earth Eccentric Orbit */
	Sun->EQ_time = 4 * (var_y * sin(2*L) - 2 * EEO * sin(M) + 4 * EEO * var_y * sin(M) * cos(2*L) -
		                0.5 * var_y * var_y * sin(4*L) - 1.25*EEO*EEO*sin(2*M)) / D2R;
	HA_Sunrise  = acos(cos(radius*D2R) / (cos(Ctrl->I.lat*D2R) * cos(SolarDec)) - tan(Ctrl->I.lat*D2R) * tan(SolarDec)) / D2R;
	Sun->SolarNoon = (720 - 4 * Ctrl->I.lon - Sun->EQ_time + TZ * 60) / 1440;
	Sun->Sunrise = Sun->SolarNoon - HA_Sunrise * 4 / 1440;
	Sun->Sunset  = Sun->SolarNoon + HA_Sunrise * 4 / 1440;
	Sun->Sunlight_duration = 8 * HA_Sunrise;
	TrueSolarTime = fmod(UT/24 * 1440 + Sun->EQ_time + 4 * Ctrl->I.lon - 60 * TZ, 1440);

	if (TrueSolarTime < 0)
		Sun->HourAngle = TrueSolarTime / 4 + 180;
	else
		Sun->HourAngle = TrueSolarTime / 4 - 180;

	Sun->SolarElevation = (asin(sin(Ctrl->I.lat * D2R) * sin(SolarDec) + cos(Ctrl->I.lat * D2R) *
	                            cos(SolarDec) * cos(Sun->HourAngle * D2R))) / D2R;

	if (Sun->SolarElevation > 85)
		r = 0;
	else {
		if (Sun->SolarElevation > 5)
			r = 58.1 / tan(Sun->SolarElevation * D2R) -0.07 / (pow(tan(Sun->SolarElevation*D2R),3)) +
			    0.000086 / (pow(tan(Sun->SolarElevation*D2R), 5));
		else {
			if (Sun->SolarElevation > -0.575)		/*  If > -34.5 arcmin (refraction index effect?) */
				r = 1735.0 + Sun->SolarElevation *
				    (-518.2 + Sun->SolarElevation * (103.4 + Sun->SolarElevation * (-12.79 + Sun->SolarElevation * 0.711)));
			else
				r = -20.772 / tan(Sun->SolarElevation * D2R);
		}
	}

	Sun->SolarElevationCorrected = Sun->SolarElevation + r / 3600;

	sz = (90 - Sun->SolarElevation) * D2R;		/* Another auxiliary variable */
	if (Sun->HourAngle > 0)
		Sun->SolarAzim = fmod(acos(((sin(Ctrl->I.lat * D2R) * cos(sz)) - sin(SolarDec)) /
			                  (cos(Ctrl->I.lat * D2R) * sin(sz))) / D2R + 180, 360);
	else
		Sun->SolarAzim = fmod(540.0 - acos(((sin(Ctrl->I.lat * D2R) * cos(sz)) - sin(SolarDec)) /
			                  (cos(Ctrl->I.lat * D2R) * sin(sz))) / D2R, 360);

	Sun->radius = radius;

	return (GMT_NOERROR);
}

int GMT_solar (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		struct GMT_OPTION *options = GMT_Create_Options (API, mode, args);
		bool print_postion = false;
		if (API->error) return (API->error);    /* Set or get option list */
		print_postion = (GMT_Find_Option (API, 'I', options) != NULL);
		gmt_M_free_options (mode);
		if (!print_postion) {
			GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: solar\n");
			return (GMT_NOT_A_VALID_MODULE);
		}
	}
	return GMT_pssolar (V_API, mode, args);
}

/* --------------------------------------------------------------------------------------------------- */
int GMT_pssolar (void *V_API, int mode, void *args) {
	int     j, n, hour, min, error = 0;
	int     n_pts = 361;						/* Number of points for the terminators */
	char    record[GMT_LEN256] = {""};

	struct  PSSOLAR_CTRL *Ctrl = NULL;
	struct  GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct  GMT_OPTION *options = NULL;
	struct  PSL_CTRL *PSL = NULL;			/* General PSL internal parameters */
	struct  GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct  SUN_PARAMS *Sun = NULL;
	struct	GMT_DATASEGMENT *S = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return program's purpose */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pssolar main code ----------------------------*/

	Sun = gmt_M_memory (GMT, NULL, 1, struct SUN_PARAMS);

	if (Ctrl->I.active) {
		solar_params (Ctrl, Sun);

		if (Ctrl->C.active) {			/* Output all members of the Sun struct as a vector of doubles */
			double out[10];
			struct GMT_RECORD *Out = gmt_new_record (GMT, out, NULL);
			if ((error = GMT_Set_Columns (API, GMT_OUT, 10, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (API->error);
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
			out[0] = -Sun->HourAngle;		out[1] = Sun->SolarDec;		out[2] = Sun->SolarAzim;
			out[3] = Sun->SolarElevation;	out[4] = Sun->Sunrise;		out[5] = Sun->Sunset;
			out[6] = Sun->SolarNoon;		out[7] = Sun->Sunlight_duration;out[8] = Sun->SolarElevationCorrected;
			out[9] = Sun->EQ_time;
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			gmt_M_free (GMT, Out);
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) { 
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
		}
		else {
			struct GMT_RECORD *Out = gmt_new_record (GMT, NULL, record);
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination*/
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_TEXT) != GMT_NOERROR) {	/* Sets output geometry */
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}

			sprintf (record, "\tSun current position:    long = %f\tlat = %f", -Sun->HourAngle, Sun->SolarDec);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			sprintf (record, "\t                      Azimuth = %.4f\tElevation = %.4f", Sun->SolarAzim, Sun->SolarElevation);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			if (Ctrl->I.position) {
				if (isnan(Sun->Sunrise)) {
					sprintf(record, "\tSunrise? No, not yet, sun is under the horizon.");
					GMT_Put_Record(API, GMT_WRITE_DATA, Out);
				}
				else {
					hour = (int)(Sun->Sunrise * 24);	min = irint((Sun->Sunrise * 24 - hour) * 60);
					sprintf(record, "\tSunrise  = %02d:%02d", hour, min);	GMT_Put_Record(API, GMT_WRITE_DATA, Out);
					hour = (int)(Sun->Sunset * 24);		min = irint((Sun->Sunset * 24 - hour) * 60);
					sprintf(record, "\tSunset   = %02d:%02d", hour, min);	GMT_Put_Record(API, GMT_WRITE_DATA, Out);
					hour = (int)(Sun->SolarNoon * 24);	min = irint((Sun->SolarNoon * 24 - hour) * 60);
					sprintf(record, "\tNoon     = %02d:%02d", hour, min);	GMT_Put_Record(API, GMT_WRITE_DATA, Out);
					hour = (int)(Sun->Sunlight_duration / 60);	min = irint(Sun->Sunlight_duration - hour * 60);
					sprintf(record, "\tDuration = %02d:%02d", hour, min);	GMT_Put_Record(API, GMT_WRITE_DATA, Out);
				}
			}
			gmt_M_free (GMT, Out);
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {
				gmt_M_free (GMT, Sun);
				Return (API->error);
			}
		}
	}
	else if (Ctrl->M.active) {						/* Dump terminator(s) to stdout, no plotting takes place */
		int n_items;
		char  *terms[4] = {"Day/night", "Civil", "Nautical", "Astronomical"};
		double out[2];
		struct GMT_RECORD *Out = gmt_new_record (GMT, out, NULL);

		gmt_set_geographic (GMT, GMT_OUT);			/* Output lon/lat */
		gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output (this one is ignored here)*/
		gmt_set_tableheader (GMT, GMT_OUT, true);	/* Turn on table headers on output */
		if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (API->error);
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			gmt_M_free (GMT, Sun);
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			gmt_M_free (GMT, Sun);
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_LINE) != GMT_NOERROR) {	/* Sets output geometry */
			gmt_M_free (GMT, Sun);
			Return (API->error);
		}
		for (n = n_items = 0; n < 4; n++) if (Ctrl->T.radius[n] > 0.0) n_items++;

		for (n = 0; n < 4; n++) {				/* Loop over the number of requested terminators */
			if (Ctrl->T.radius[n] == 0) continue;		/* This terminator was not requested */
			Ctrl->T.which = n;
			solar_params (Ctrl, Sun);
			S = gmt_get_smallcircle (GMT, -Sun->HourAngle, Sun->SolarDec, Sun->radius, n_pts);
			sprintf (record, "%s terminator", terms[n]);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
			for (j = 0; j < n_pts; j++) {
				out[GMT_X] = S->data[GMT_X][j];	out[GMT_Y] = S->data[GMT_Y][j];
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			gmt_free_segment (GMT, &S);
		}
		gmt_M_free (GMT, Out);

		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) Return (API->error);
	}
	else {	/* Plotting the terminator as line, polygon, or clip path */
		double *lon = NULL, *lat = NULL, x0, y0;
		unsigned int first = (Ctrl->N.active) ? 0 : 1;
		
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) {
			gmt_M_free (GMT, Sun);
			Return (GMT_PROJECTION_ERROR);
		}
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		if (Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 1);	/* Must clip map */

		for (n = 0; n < 4; n++) {	/* Loop over the number of requested terminators */
			if (Ctrl->T.radius[n] == 0) continue;	/* This terminator was not requested */
			Ctrl->T.which = n;
			solar_params (Ctrl, Sun);
			S = gmt_get_smallcircle (GMT, -Sun->HourAngle, Sun->SolarDec, Sun->radius, n_pts);
			if (Ctrl->G.clip) {	/* Set up a clip path */
				bool must_free = true;
				if ((n_pts = (int)gmt_geo_polarcap_segment (GMT, S, &lon, &lat)) == 0) {	/* No resampling took place */
					lon = S->data[GMT_X]; lat = S->data[GMT_Y];
					n_pts = (int)S->n_rows;
					must_free = false;
				}
				for (j = 0; j < n_pts; j++) {	/* Apply map projection */
					gmt_geo_to_xy (GMT, lon[j], lat[j], &x0, &y0);
					lon[j] = x0; lat[j] = y0;
				}
				PSL_beginclipping (PSL, lon, lat, n_pts, GMT->session.no_rgb, 2 + first);
				if (must_free) {
					gmt_M_free (GMT, lon);
					gmt_M_free (GMT, lat);
				}
			}
			else {
				if (Ctrl->W.active)
					gmt_setpen (GMT, &Ctrl->W.pen);
				if (Ctrl->G.active)
					gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
				gmt_geo_polygons (GMT, S);
			}
			gmt_free_segment (GMT, &S);
		}

		gmt_map_basemap (GMT);
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);
	}

	gmt_M_free (GMT, Sun);

	Return (GMT_NOERROR);
}
