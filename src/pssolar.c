/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis:
 *
 * Author:	Joaquim Luis
 * Date:	1-JAN-2016
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"pssolar"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot day-light terminators and other sunlight parameters"
#define THIS_MODULE_KEYS	"<II,>XO,RG-"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcptxy"

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
	struct PSSOL_G {		/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;	
	struct PSSOL_I {		/* -I info about solar stuff */
		bool   active;
		bool   position;
		int    TZ;			/* Time Zone */
		int    date_vec[6];	/* To store date as [YYYY MM DD hh mm ss] */
		char  *date;		/* To hold eventual date string */
		double lon, lat;
	} I;
	struct M {				/* -M dumps the terminators data instead of plotting them*/
		bool active;
	} M;
	struct PSSOL_T {		/* -T terminator options */
		bool   active;
		bool   night, civil, nautical, astronomical;
		int    which;		/* 0 = night, ... 3 = astronomical */
		int    TZ;			/* Time Zone */
		int    date_vec[6];	/* To store date as [YYYY MM DD hh mm ss] */
		char  *date;		/* To hold eventual date string */
		double radius[4];
	} T;
	struct PSSOL_W {		/* -W<pen> */
		bool active;
		unsigned int mode;	/* 0 = normal, 1 = -C applies to pen color only, 2 = -C applies to symbol fill & pen color */
		struct GMT_PEN pen;
	} W;
};

void *New_pssolar_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSOLAR_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSSOLAR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	return (C);
}

void Free_pssolar_Ctrl (struct GMT_CTRL *GMT, struct PSSOLAR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int parse_date_tz(char *date_tz, char **date, int *TZ) {
	/* Parse the string date_tz in the form +d<date>+z<TZ> for char date and int TZ.
	   date_tz is a copy so ok to mess with it. On the other hand, the output var 'date'
	   is strdup'd here so it must be freed later.
	*/
	char *pch;
	if ((pch = strchr(&date_tz[1], '+')) != NULL) {		/* Than we know that both are present. Just dont know the order */
		if (pch[1] == 'z') {
			*TZ = atoi(&pch[2]);
			pch[0] = '\0';
			date[0] = strdup(&date_tz[2]);
		}
		else if (pch[1] == 'd') {
			date[0] = strdup(&pch[2]);
			pch[0] = '\0';
			*TZ = atoi(&date_tz[2]);
		}
		else
			return 1;
	}
	else if (date_tz[1] == 'd')				/* So, either this one */
		date[0] = strdup(&date_tz[2]);
	else if (date_tz[1] == 'z')				/* or this */
		*TZ = atoi(&date_tz[2]);
	else
		return 1;

	return 0;
}

int GMT_pssolar_usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pssolar synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pssolar [%s] [-G<fill>] [-I[lon/lat][+d<date>][+z<TZ>]]", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "[%s] [-K] [-M] [-O]\n", GMT_J_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-P] [-T<dcna>[+d<date>][+z<TZ>]] [%s]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W<pen>]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT,
	             GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B");
	GMT_fill_syntax (API->GMT, 'G', "Specify color or pattern [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Print current sun position. Append lon/lat to print also the times of\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Sunrise, Sunset, Noon and length of the day.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add +d<date> in ISO format, e.g, +d2000-04-25, to compute sun parameters\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for this date. If necessary, append time zone via +z<TZ>.\n");
	GMT_Option (API, "J,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Write terminator(s) as a multisegment ASCII (or binary, see -bo) file to standard output. No plotting occurs.\n");	
	GMT_Option (API, "O,P,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <dcna> Plot (or dump; see -M) one or more terminators defined via these flags:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d means day/night terminator.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c means civil twilight.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n means nautical twilight.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a means astronomical twilight.\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Specify outline pen attributes [Default is no outline].", 0);
	GMT_Option (API, "X,c,p");
	GMT_Option (API, "t,.");

	return (EXIT_FAILURE);
}

int GMT_pssolar_parse (struct GMT_CTRL *GMT, struct PSSOLAR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pssolar and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 */

	int    j, TZ = 0, n_files = 0, n_errors = 0;
	char  *pch = NULL, *date = NULL, *date_tz_str = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'G':		/* Set fill for symbols or polygon */
				Ctrl->G.active = true;
				if (!opt->arg[0] || GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " "); n_errors++;
				}
				break;
			case 'I':		/* Infos -I[x/y][+d<date>][+z<TZ>] */
				Ctrl->I.active = true;
				if (opt->arg[0]) {	/* Also gave location */
					Ctrl->I.position = true;
					n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->I.lon, &Ctrl->I.lat) != 2,
					                                 "Syntax error: Expected -I[<lon>/<lat>]\n");
					if ((pch = strchr(opt->arg, '+')) != NULL) {		/* Have one or two extra options */
						date_tz_str = strdup(pch);
						pch[0] = '\0';		/* Truncates the date & time zone chunk */
						if (parse_date_tz(date_tz_str, &date, &TZ)) {
							GMT_Report(GMT->parent, GMT_MSG_NORMAL, "Error in -T option. Badly formatted date or time zone.\n");
							n_errors++;
						}
						Ctrl->I.TZ = TZ;
						if (date) Ctrl->I.date = date;
						free(date_tz_str);
					}
				}
				break;
			case 'M':
				Ctrl->M.active = true;
				break;
			case 'T':		/* -Tdcna[+d<date>][+z<TZ>] */
				Ctrl->T.active = true;
				if (opt->arg[0]) {
					if ((pch = strchr(opt->arg, '+')) != NULL) {		/* Have one or two extra options */
						date_tz_str = strdup(pch);
						pch[0] = '\0';		/* Truncates the date & time zone chunk */
						if (parse_date_tz(date_tz_str, &date, &TZ)) {
							GMT_Report(GMT->parent, GMT_MSG_NORMAL, "Error in -T option. Badly formatted date or time zone.\n");
							n_errors++;
						}
						Ctrl->T.TZ = TZ;
						if (date) Ctrl->T.date = date;
						free(date_tz_str);
					}
					for (j = 0; j < strlen(opt->arg); j++) {
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
				break;
			case 'W':		/* Pen */
				Ctrl->W.active = true;
				j = 0;
				if (opt->arg[j] == '-') {Ctrl->W.mode = 1; j++;}
				if (opt->arg[j] == '+') {Ctrl->W.mode = 2; j++;}
				if (opt->arg[j] && GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:", 3);
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "\t   A leading - applies cpt color (-C) to the pen only.\n");
					n_errors++;
				}
				break;

			default:		/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->I.active && !Ctrl->M.active) {	/* Allow plotting without specifying -R and/or -J */
		if (!GMT->common.J.active) {	/* When no projection specified, use fake linear projection */
			GMT_parse_common_options (GMT, "J", 'J', "X14cd/0d");
			GMT->common.J.active = true;
		}
		if (!GMT->common.R.active) {	/*  */
			GMT_parse_common_options (GMT, "R", 'R', "-180/180/-90/90");
			GMT->common.R.active = true;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files > 0, "Syntax error: No input files allowed\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->T.active + Ctrl->I.active) > 1, "Syntax error: Cannot combine -T and -I\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pssolar_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int solar_params (struct PSSOLAR_CTRL *Ctrl, struct SUN_PARAMS *Sun) {
	/* Adapted from https://github.com/joa-quim/mirone/blob/master/utils/solar_params.m  */
	/* ... */
	int    TZ, year, month, day, hour, min, sec;
	struct tm *UTC; 
	time_t right_now = time (NULL); 
	double JC, JD, UT, L, M, C, var_y, r, sz, theta, lambda, obliqCorr, meanObliqEclipt;
	double EEO, HA_Sunrise, TrueSolarTime, SolarDec;
	double radius = 90.833;		/* Sun radius (16' + 34.5' from the light refraction effect) */

	radius = Ctrl->T.radius[Ctrl->T.which];
	TZ = (Ctrl->I.TZ != 0) ? Ctrl->I.TZ : ((Ctrl->T.TZ != 0) ? Ctrl->I.TZ : 0);

	/*  Date info may be in either of I or T options. If not, sue current time. */
	if (Ctrl->I.date_vec[0] != 0 || Ctrl->I.date_vec[0] != 0) {
		if (Ctrl->I.date_vec[0] != 0) {
			year = Ctrl->I.date_vec[0];		month = Ctrl->I.date_vec[1];	day = Ctrl->I.date_vec[2];
			hour = Ctrl->I.date_vec[3];		min   = Ctrl->I.date_vec[4];	sec = Ctrl->I.date_vec[5];
		}
		else {
			year = Ctrl->T.date_vec[0];		month = Ctrl->T.date_vec[1];	day = Ctrl->T.date_vec[2];
			hour = Ctrl->T.date_vec[3];		min   = Ctrl->T.date_vec[4];	sec = Ctrl->T.date_vec[5];
		}
	}
	else {
		UTC   = gmtime (&right_now);
		//UTC->tm_mday = 26;	UTC->tm_hour = 17;	UTC->tm_min = 49;	UTC->tm_sec = 30;
		year  = UTC->tm_year + 1900;
		month = UTC->tm_mon + 1;
		day   = UTC->tm_mday;
		hour  = UTC->tm_hour;
		min   = UTC->tm_min;
		sec   = UTC->tm_sec;
	}

	UT = hour + (double)min / 60 + (double)sec / 3600;

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

	return (GMT_OK);
}

/* --------------------------------------------------------------------------------------------------- */
int GMT_pssolar (void *V_API, int mode, void *args) {
	int     j, n, hour, min, error = 0;
	int     n_pts = 361;						/* Number of points for the terminators */
	char    record[GMT_LEN256] = {""};

	struct  PSSOLAR_CTRL *Ctrl = NULL;
	struct  GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct  GMT_OPTION *options = NULL;
	struct  PSL_CTRL *PSL = NULL;			/* General PSL interal parameters */
	struct  GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct  SUN_PARAMS *Sun = NULL;
	struct	GMT_DATASEGMENT *S = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pssolar_usage (API, GMT_MODULE_PURPOSE));	/* Return program's purpose */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pssolar_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pssolar_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pssolar_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pssolar_parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pssolar main code ----------------------------*/
	

	Sun = GMT_memory (GMT, NULL, 1, struct SUN_PARAMS);

	if (Ctrl->I.active) {
		double out[10];

		if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK)	/* Registers default output destination, unless already set */
			Return (API->error);
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_OFF) != GMT_OK)	/* Enables data output and sets access mode */
			Return (API->error);

		solar_params (Ctrl, Sun);
		sprintf (record, "\tSun current position:    long = %f\tlat = %f", -Sun->HourAngle, Sun->SolarDec);
		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		sprintf (record, "\t                      Azimuth = %.4f\tElevation = %.4f", Sun->SolarAzim, Sun->SolarElevation);
		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		if (Ctrl->I.position) {
			hour = (int)(Sun->Sunrise * 24);	min = irint((Sun->Sunrise * 24 - hour) * 60);
			sprintf (record, "\tSunrise  = %02d:%02d", hour, min);			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			hour = (int)(Sun->Sunset * 24);		min = irint((Sun->Sunset * 24 - hour) * 60);
			sprintf (record, "\tSunset   = %02d:%02d", hour, min);			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			hour = (int)(Sun->SolarNoon * 24);	min = irint((Sun->SolarNoon * 24 - hour) * 60);
			sprintf (record, "\tNoon     = %02d:%02d", hour, min);			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			hour = (int)(Sun->Sunlight_duration / 60);	min = irint(Sun->Sunlight_duration - hour * 60);
			sprintf (record, "\tDuration = %02d:%02d", hour, min);			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) Return (API->error);

		/* Now move the needle to doubles. Output all members of the Sun struct as a vector */
		if ((error = GMT_set_cols (GMT, GMT_OUT, 10)) != GMT_OK) Return (API->error);
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK)
			Return (API->error);
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_OK) Return (API->error);
		out[0] = -Sun->HourAngle;		out[1] = -Sun->SolarDec;		out[2] = Sun->SolarAzim;
		out[3] = Sun->SolarElevation;	out[4] = Sun->Sunrise;			out[5] = Sun->Sunset;
		out[6] = Sun->SolarNoon;		out[7] = Sun->Sunlight_duration;out[8] = Sun->SolarElevationCorrected;
		out[9] = Sun->EQ_time;
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) Return (API->error);
	}

	else if (Ctrl->M.active) {						/* Dump terminator(s) to stdout, no plotting takes place */
		int n_items;
		char  *terms[4] = {"Day/night", "Civil", "Nautical", "Astronomical"};
		double out[2];

		GMT_set_geographic (GMT, GMT_OUT);			/* Output lon/lat */
		GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output (this one is ignored here)*/
		GMT_set_tableheader (GMT, GMT_OUT, true);	/* Turn on table headers on output */
		if ((error = GMT_set_cols (GMT, GMT_OUT, 2)) != GMT_OK) Return (API->error);
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK)	/* Establishes data output */
			Return (API->error);
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK)	/* Enables data output and sets access mode */
			Return (API->error);
		for (n = n_items = 0; n < 4; n++) if (Ctrl->T.radius[n] > 0.0) n_items++;

		for (n = 0; n < 4; n++) {						/* Loop over the number of requested terminators */
			if (Ctrl->T.radius[n] == 0) continue;		/* This terminator was not requested */
			Ctrl->T.which = n;
			solar_params (Ctrl, Sun);
			S = GMT_get_smallcircle (GMT, -Sun->HourAngle, Sun->SolarDec, Sun->radius, n_pts);
			if (n_items > 1) {
				sprintf (record, "%s terminator", terms[n]);
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
			}
			for (j = 0; j < n_pts; j++) {
				out[GMT_X] = S->coord[GMT_X][j];	out[GMT_Y] = S->coord[GMT_Y][j];
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}
			GMT_free_segment (GMT, &S, GMT_ALLOC_INTERNALLY);
		}

		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) Return (API->error);
	}

	else {
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
		if ((PSL = GMT_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

		for (n = 0; n < 4; n++) {	/* Loop over the number of requested terminators */
			if (Ctrl->T.radius[n] == 0) continue;	/* This terminator was not requested */
			Ctrl->T.which = n;
			solar_params (Ctrl, Sun);
			S = GMT_get_smallcircle (GMT, -Sun->HourAngle, Sun->SolarDec, Sun->radius, n_pts);
			
			if (Ctrl->W.active)
				GMT_setpen (GMT, &Ctrl->W.pen);
			if (Ctrl->G.active)
				GMT_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);

			GMT_geo_polygons (GMT, S);
			GMT_free_segment (GMT, &S, GMT_ALLOC_INTERNALLY);
		}

		GMT_map_basemap (GMT);
		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_plotend (GMT);
	}

	GMT_free (GMT, Sun);

	Return (GMT_OK);
}
