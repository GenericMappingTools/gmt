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
#define THIS_MODULE_PURPOSE	"Plot day-light terminators and other sun-light parameters"
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
	struct PSSOL_G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;	
	struct PSSOL_I {	/* -I info about solar stuff */
		bool active;
		bool position;
		double lon, lat;
	} I;
	struct PSSOL_T {	/* -T terminator options */
		bool active;
	} T;
	struct PSSOL_W {	/* -W<pen> */
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

int GMT_pssolar_usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psimage synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pssolar [%s] [-I[lon/lat]]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-K] [-O]\n", GMT_J_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-P] [%s] [%s]\n", GMT_Rgeoz_OPT, GMT_U_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W<outlinepen>] [%s] [%s] [%s]\n\t[%s] [%s]\n\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B");
	GMT_Option (API, "J-Z,K");
	GMT_Option (API, "O,P,R,U,V");
	GMT_pen_syntax (API->GMT, 'W', "Specify outline pen attributes [Default is no outline].", 0);
	GMT_Option (API, "X,c,p");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Requires -R and -J for proper functioning).\n");
	GMT_Option (API, "t,.");

	return (EXIT_FAILURE);
}

int GMT_pssolar_parse (struct GMT_CTRL *GMT, struct PSSOLAR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psimage and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int j;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'G':		/* Set fill for symbols or polygon */
				Ctrl->G.active = true;
				if (!opt->arg[0] || GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " "); n_errors++;
				}
				break;
			case 'I':	/* Infos */
				Ctrl->I.active = true;
				if (opt->arg[0]) {	/* Also gave shifts */
					n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->I.lon, &Ctrl->I.lat) != 2,
					                                 "Syntax error: Expected -I[<lon>/<lat>]\n");
					Ctrl->I.position = true;
				}
				break;
			case 'T':	/*  */
				break;
			case 'W':	/*  */
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

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (!GMT->common.J.active) {	/* When no projection specified, use fake linear projection */
		GMT_parse_common_options (GMT, "J", 'J', "X1d");
		GMT->common.J.active = true;
	}
	if (!GMT->common.R.active) {	/*  */
		GMT_parse_common_options (GMT, "R", 'R', "-180/180/-90/90");
		GMT->common.R.active = true;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pssolar_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int solar_params (struct GMT_CTRL *GMT, struct PSSOLAR_CTRL *Ctrl, struct SUN_PARAMS *Sun) {
	/* Adapted from https://github.com/joa-quim/mirone/blob/master/utils/solar_params.m  */
	int    TZ, year, month;
	struct tm *UTC; 
	time_t right_now = time (NULL); 
	double JC, JD, UT, L, M, C, var_y, r, sz, theta, lambda, obliqCorr, meanObliqEclipt;
	double EEO, HA_Sunrise, TrueSolarTime, SolarDec;
	double radius = 90.833;		/* Sun radius (16' + 34.5' from the light refraction effect) */

	TZ = 0;
	UTC = gmtime (&right_now);

	//UTC->tm_mday = 26;	UTC->tm_hour = 17;	UTC->tm_min = 49;	UTC->tm_sec = 30;
	UT = UTC->tm_hour + (double)UTC->tm_min / 60 + (double)UTC->tm_sec / 3600;

	/*  From http://scienceworld.wolfram.com/astronomy/JulianDate.html */
	/* tm_mon is 0-11, so add 1 for 1-12 range, tm_year is years since 1900, so add 1900, but tm_mday is 1-31 so use as is */
	year = UTC->tm_year + 1900;
	month = UTC->tm_mon + 1;
	JD = 367.0 * year - floor(7.0 * (year + floor((month + 9.0) / 12) ) / 4) -		/* Julian day */
		floor(3.0 * (floor((year + (month - 9.0) / 7) / 100) + 1 ) / 4) +
		floor((275.0 * month) / 9) + UTC->tm_mday + 1721028.5 + (UT / 24);

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
	int    j, hour, min, error = 0;
	int    n_pts = 360;						/* Number of points for the terminators */
	char   record[GMT_LEN256] = {""};
	double *xx = NULL, *yy = NULL;
	double *lons = NULL, *lats = NULL;		/* To hold the terminator polygon */

	struct PSSOLAR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;			/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct SUN_PARAMS *Sun = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pssolar_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pssolar_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pssolar_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pssolar_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pssolar_parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psimage main code ----------------------------*/
	
	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_OFF) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	Sun = GMT_memory (GMT, NULL, 1, struct SUN_PARAMS);
	solar_params (GMT, Ctrl, Sun);

	if (Ctrl->I.active) {
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
	}
	else {
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
		if ((PSL = GMT_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

		GMT_get_smallcircle (GMT, -Sun->HourAngle, Sun->SolarDec, Sun->radius, n_pts, &lons, &lats);
		
		xx = GMT_memory (GMT, NULL, n_pts, double);
		yy = GMT_memory (GMT, NULL, n_pts, double);
		for (j = 0; j < n_pts; j++) {		/* Convert to inches/cm */
			GMT_geo_to_xy (GMT, lons[j], lats[j], &xx[j], &yy[j]);
		}

		if (Ctrl->G.active) {
			GMT_setfill (GMT, &Ctrl->G.fill, false);
			PSL_plotpolygon (PSL, xx, yy, n_pts);
		}
		if (Ctrl->W.active) {
			GMT_setpen (GMT, &Ctrl->W.pen);
			PSL_plotline (PSL, xx, yy, n_pts, PSL_MOVE + PSL_STROKE);
		}

		GMT_map_basemap (GMT);
		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_plotend (GMT);
		GMT_free (GMT, xx);
		GMT_free (GMT, yy);
	}

	GMT_free (GMT, Sun);

	Return (GMT_OK);
}
