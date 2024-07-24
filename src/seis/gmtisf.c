/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2024 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *		Joaquim Luis
 * Date:	24-JUL-2024
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "longopt/readisf_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtisf"
#define THIS_MODULE_MODERN_NAME	"gmtisf"
#define THIS_MODULE_LIB		"seis"
#define THIS_MODULE_PURPOSE	"Read seismicity data in the ISF formated file"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RV"

#include "read_isf.c"

/* Control structure */

struct READISF_CTRL {
	struct READISF_In {   /* Input files */
		bool active;
		char *file;
	} In;
	struct READISF_D {   /* For date control */
		bool active;
		bool two_dates;
		struct GMT_GCAL date1;
		struct GMT_GCAL date2;
	} D;
	struct READISF_F {   /* Fault mechanism */
		bool active;
		bool aki;
	} F;
	struct READISF_N {   /* No times */
		bool active;
	} N;
};


static void *New_Ctrl(struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct READISF_CTRL *C;

	C = gmt_M_memory(GMT, NULL, 1, struct READISF_CTRL);

	return (C);
}

static void Free_Ctrl(struct GMT_CTRL *GMT, struct READISF_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free(GMT, C->In.file);
	gmt_M_free(GMT, C);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	/* This displays the pssac synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage(API, 0, "usage: %s isffile [-Ddate1[/date2]] [%s] [-F[a]] [-N] [%s]\n", name, GMT_Rgeoz_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return EXIT_FAILURE;

	GMT_Message(API, GMT_TIME_NONE, "  OPTIONAL ARGUMENTS:\n");
	GMT_Option(API, "R");
	GMT_Usage(API, 1, "\n-Ddate1[/date2]");
	GMT_Usage(API, -2, "Limit the output to data >= date1, or between date1 and date2. <date> must be in ISO format, e.g, 2000-04-25");
	GMT_Usage(API, 1, "\n-F[a]");
	GMT_Usage(API, -2, "Select only events that have focal mechanisms. The default is Global CMT convention. Append 'a' for the AKI convention.");
	GMT_Usage(API, 1, "\n-N");
	GMT_Usage(API, -2, "The default is to output time information [year month day hour minute] as the last 5 columns. Use this option to skip those last 5 columns.");

	return EXIT_FAILURE;
}

static int parse(struct GMT_CTRL *GMT, struct READISF_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pssac and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, pos = 0;
	int i, j, k;
	size_t n_alloc = 0, len;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, p[GMT_BUFSIZ] = {""};
	char  *pch = NULL;
	double t;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option(API, Ctrl->D.active);
				if ((pch = strchr(opt->arg, '/')) != NULL) {	/* Have two dates */
					gmt_scanf_arg(GMT, (&pch[1]), GMT_IS_ABSTIME, false, &t);
					gmt_gcal_from_dt(GMT, t, &Ctrl->D.date2);	/* Convert t to a complete calendar structure */
					pch[0] = '\0';				/* Hide the second date */
					Ctrl->D.two_dates = true;
				}
				gmt_scanf_arg(GMT, &opt->arg[0], GMT_IS_ABSTIME, false, &t);
				gmt_gcal_from_dt(GMT, t, &Ctrl->D.date1);	/* Convert t to a complete calendar structure */
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option(API, Ctrl->F.active);
				if (opt->arg[0] == 'a') Ctrl->F.aki = true;	
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option(API, Ctrl->N.active);
				break;

			/* Processes program-specific parameters */
		}
	}

	/* Check that the options selected are mutually consistent */
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single ISF file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {gmt_M_free_options(mode); return (code);}
#define Return(code) {Free_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtisf(void *V_API, int mode, void *args) {	/* High-level function that implements the pssac task */

	FILE  *fp;
	char   timfix,epifix,depfix,antype,loctype,magind;
	char  *etype, *author, *origid, *magtype, line[ISF_LINE_LEN];
	char **mag_t, evid[ISF_LINE_LEN], region[ISF_LINE_LEN];
	char   f_type[6], f_plane[6];
    bool   got_event = false, event_end, tensor_end, got_region = false, out_of_date;
	int    error = GMT_NOERROR;
	int	   i, in, mag_c = 0, event_c, idx_min_rms, np, ns, n_out_cols;
	int	   export_aki = false, export_cmt = false, export_tensor = false;
	int	   yyyy,mm,dd,hh,mi,ss,msec,strike,ndef,nsta,gap;
	int   *years, *months, *days, *hours, *minutes;
	float  stime,sdobs,lat,lon,depth,smaj,smin,sdepth,mindist,maxdist;
	float  mag, magerr;
	float *rms, *lats, *lons, *depths, *mags;
	float  scalar_moment, fclvd, mrr, mtt, mpp, mrt, mtp, mpr;
	float  scalar_moment_unc, fclvd_unc, mrr_unc, mtt_unc, mpp_unc, mrt_unc, mtp_unc, mpr_unc, duration;
	float  strike1, dip1, rake1, strike2, dip2, rake2;
	float  t_val, t_azim, t_pl, b_val, b_azim, b_pl, p_val, p_azim, p_pl;
	double west = 0.0, east = 0.0, south = 0.0, north = 0.0;

	/* Moment tensor variables */
	int	scale_factor, nsta1, nsta2, ncomp1, ncomp2, got_momten_line1, got_momten_line2, centroid;

	double out[16];
	struct READISF_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct GMT_RECORD *Out = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/
	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);	/* Return the purpose of program */
	options = GMT_Create_Options(API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage(API, options, 0, usage)) != GMT_NOERROR) bailout(error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout(API->error); /* Save current state */
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_Ctrl(GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse(GMT, Ctrl, options)) != 0) Return(error);

	/*---------------------------- This is the pssac main code ----------------------------*/

	/* I Don't remember where size 100 came from. ISC site example in fortran also uses 100 */
	etype   = gmt_M_memory(GMT, NULL, ISF_ETYPE_LEN, char *);
	author  = gmt_M_memory(GMT, NULL, ISF_AUTHOR_LEN, char *);
	origid  = gmt_M_memory(GMT, NULL, ISF_ORIGID_LEN, char *);
	magtype = gmt_M_memory(GMT, NULL, ISF_MAGTYPE_LEN, char *);
	mags    = gmt_M_memory(GMT, NULL, 100, float *);
	rms     = gmt_M_memory(GMT, NULL, 100, float *);
	lats    = gmt_M_memory(GMT, NULL, 100, float *);
	lons    = gmt_M_memory(GMT, NULL, 100, float *);
	depths  = gmt_M_memory(GMT, NULL, 100, float *);
	years   = gmt_M_memory(GMT, NULL, 100, int *);
	months  = gmt_M_memory(GMT, NULL, 100, int *);
	days    = gmt_M_memory(GMT, NULL, 100, int *);
	hours   = gmt_M_memory(GMT, NULL, 100, int *);
	minutes = gmt_M_memory(GMT, NULL, 100, int *);
	mag_t = gmt_M_memory(GMT, NULL, 100, char *);
	for (i = 0; i < 100; i++)
		mag_t[i] = gmt_M_memory(GMT, NULL, GMT_LEN16, char);
	
	n_out_cols = Ctrl->F.active ? 11+5 : 4+5;
	if (Ctrl->F.aki) n_out_cols = 7+5;		/* The Aki convention uses 7 columns */
	if (Ctrl->N.active) n_out_cols -= 5;

	gmt_set_geographic(GMT, GMT_OUT);	/* Output lon/lat */

	if (GMT_Init_IO(API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) 	/* Establishes data output */
		Return(API->error);

	if (GMT_Begin_IO(API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) 	/* Enables data output and sets access mode */
		Return(API->error);

	if ((error = GMT_Set_Columns(API, GMT_OUT, n_out_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);
	for (i = 2; i < n_out_cols; i++) GMT->current.io.col_type[GMT_OUT][i] = GMT_IS_FLOAT;
			
	if ((fp = fopen(Ctrl->In.file, "rt")) == NULL) {
		GMT_Report(GMT->parent, GMT_MSG_ERROR, "Unable to open file %s [permission trouble?]\n", Ctrl->In.file);
		return GMT_NOTSET;
	}

	if (GMT->common.R.active[0]) {
		got_region = true;
		west = GMT->common.R.wesn[XLO];
		east = GMT->common.R.wesn[XHI];
		south = GMT->common.R.wesn[YLO];
		north = GMT->common.R.wesn[YHI];
	}

	Out = gmt_new_record(GMT, out, NULL);

	if (Ctrl->F.active) {			/* Focal mechanims */
		event_end = tensor_end = false;
		while (fgets (line, ISF_LINE_LEN, fp) != NULL) {
			gmt_chop(line);         /* Chops off any CR and/or LF */
			if (!read_event_id(line, evid, region)) continue;
			if(!read_origin_head(line)) {
				event_c = read_event_data(fp,line,&yyyy,&mm,&dd,&hh,&mi,&ss,&msec,&timfix,&stime,&sdobs,
				                          &lat,&lon,&epifix,&smaj,&smin,&strike,&depth,&depfix,&sdepth,&ndef,
				                          &nsta,&gap,&mindist,&maxdist,&antype,&loctype,etype,author,origid,
				                          lats,lons,rms,depths,years,months,days,hours,minutes,&idx_min_rms);
				if (event_c > 0) got_event = true;
				if (!read_momten_head_1(line)) goto L1;		/* Awfull, I know */
			}
			else if (!read_origin_centroid(line)) {		/* Harvard Moment Tensor event */
				i = 0;
				got_momten_line1 = got_momten_line2 = false;
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) 
					if (!read_momten_head_1(line)) i++; 
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) 
					if (!read_momten_head_2(line)) i++; 
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) {
					if (!read_momten_line_1(line, &scale_factor, &scalar_moment, &fclvd, &mrr, &mtt,
					                        &mpp, &mrt, &mtp, &mpr, &nsta1, &nsta2, author))
						got_momten_line1 = true;
				}
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) {
					if (!read_momten_line_2(line,&scalar_moment_unc,&fclvd_unc,&mrr_unc,&mtt_unc,
 					                        &mpp_unc,&mrt_unc,&mtp_unc,&mpr_unc,&ncomp1,&ncomp2,&duration))
						got_momten_line2 = true;
				}
				centroid = true;
			}
			else if (!read_momten_head_1(line)) {
				got_momten_line1 = false;
L1:
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) 
					read_momten_head_2(line); 
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) {
					if (!read_momten_line_1(line, &scale_factor, &scalar_moment, &fclvd, &mrr, &mtt,
					                        &mpp, &mrt, &mtp, &mpr, &nsta1, &nsta2, author))
						got_momten_line1 = true;
				}
				centroid = false;
			}
			else if (!read_fault_plane_head(line)) {
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) 
					read_fault_plane (line, f_type, &strike1, &dip1, &rake1, &np, &ns, f_plane, author);
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) 
					read_fault_plane (line, f_type, &strike2, &dip2, &rake2, &np, &ns, f_plane, author);
			}
			else if (!read_axes_head(line)) {
				if (fgets(line, ISF_LINE_LEN, fp) != NULL) 
					if (!read_axes(line, &scale_factor, &t_val, &t_azim, &t_pl, &b_val, &b_azim,
					               &b_pl, &p_val, &p_azim, &p_pl,author));
						tensor_end = true;
			}
			else if (tensor_end && !read_netmag_head(line)) {
				mag_c = read_mags(fp,line,magtype,&magind,&mag,&magerr,&nsta,author,origid,mag_t,mags);
				event_end = true;
			}

			if (got_event && event_end && tensor_end) {
				/* The reported values respect the last entry in the event */
				lon = lons[idx_min_rms];
				lat = lats[idx_min_rms];
				if (got_region && (lon < west || lon > east || lat < south || lat > north)) {
					got_event = event_end = false;
					mag_c = 0;
					continue;
				}
				if (Ctrl->F.aki) {
					if (mag_c >= 1) 	/* Have multiple magnitudes. Choose one */
						mag = select_mag(mag_c, mags, mag_t);
					else
						mag = 0;
				}

				depth = depths[idx_min_rms];
				if (depth == ISF_NULL) depth = 0;
				yyyy = years[idx_min_rms];
				mm = months[idx_min_rms];
				dd = days[idx_min_rms];
				got_event = event_end = tensor_end = false;
				mag_c = 0;

				out[GMT_X] = lon;	out[GMT_Y] = lat;	out[2] = depth;
				if (Ctrl->F.aki) {
					out[3] = strike1;	out[4] = dip1;	out[5] = rake1;		out[6] = mag;	
				}
				else {
					out[3] = strike1;	out[4] = dip1;	out[5] = rake1;
					out[6] = strike2;	out[7] = dip2;	out[8] = rake2;	out[9] = scalar_moment;	out[10] = scale_factor;
					out[11] = yyyy;		out[12] = mm;	out[13] = dd;	out[14] = hh;	out[15] = mi;
				}
				GMT_Put_Record(GMT->parent, GMT_WRITE_DATA, Out);
			}
		}
	}
	else {					/* Just the event data (no focal mechanism) */
		event_end = true;
		while (fgets (line, ISF_LINE_LEN, fp) != NULL) {
			gmt_chop(line);         /* Chops off any CR and/or LF */
			if (!read_event_id(line, evid, region)) continue;
			if (!read_origin_head(line)) {
				event_c = read_event_data(fp,line,&yyyy,&mm,&dd,&hh,&mi,&ss,&msec,&timfix,&stime,&sdobs,
				                          &lat,&lon,&epifix,&smaj,&smin,&strike,&depth,&depfix,&sdepth,&ndef,
				                          &nsta,&gap,&mindist,&maxdist,&antype,&loctype,etype,author,origid,
				                          lats,lons,rms,depths,years,months,days,hours,minutes,&idx_min_rms);
				if (event_c > 0) got_event = true;
			}
			else if (!read_netmag_head(line)) {
				mag_c = read_mags(fp,line,magtype,&magind,&mag,&magerr,&nsta,author,origid,mag_t,mags);
				event_end = true;
			}
			if (got_event && event_end) {
				/* Select the event detection that has the minimum RMS */
				lon = lons[idx_min_rms];
				lat = lats[idx_min_rms];
				if (got_region && (lon < west || lon > east || lat < south || lat > north)) {
					got_event = event_end = false;	mag_c = 0;
					continue;
				}
				depth = depths[idx_min_rms];
				if (depth == ISF_NULL) depth = 0;
				yyyy = years[idx_min_rms];
				mm = months[idx_min_rms];
				dd = days[idx_min_rms];
				hh = hours[idx_min_rms];
				mi = minutes[idx_min_rms];
				
				/* See if user set date bounds */
				if (Ctrl->D.active) {
					out_of_date = false;
					if (!(Ctrl->D.date1.year >= yyyy && Ctrl->D.date1.month >= mm && Ctrl->D.date1.day_m >= dd && Ctrl->D.date1.hour >= hh))
						out_of_date = true;
					if (!out_of_date && Ctrl->D.two_dates) {
						if (!(Ctrl->D.date2.year <= yyyy && Ctrl->D.date2.month <= mm && Ctrl->D.date2.day_m <= dd && Ctrl->D.date2.hour <= hh))
							out_of_date = true;
					}
					if (out_of_date) {
						got_event = event_end = false;	mag_c = 0;
						continue;
					}
				}

				if (mag_c >= 1) 	/* Have multiple magnitudes. Choose one */
					mag = select_mag(mag_c, mags, mag_t);
				else
					mag = 0;
				if (depth == ISF_NULL) depth = 0;

				got_event = event_end = false;
				mag_c = 0;
				out[GMT_X] = lon;	out[GMT_Y] = lat;	out[2] = depth;	out[3] = mag;
				out[4] = yyyy;		out[5] = mm;		out[6] = dd;	out[7] = hh;	out[8] = mi;
				GMT_Put_Record(GMT->parent, GMT_WRITE_DATA, Out);
			}
		}
	}

	fclose(fp);
	gmt_M_free(GMT, etype);		gmt_M_free(GMT, author);	gmt_M_free(GMT, origid);	gmt_M_free(GMT, magtype);
	gmt_M_free(GMT, mags);		gmt_M_free(GMT, rms);		gmt_M_free(GMT, lats);		gmt_M_free(GMT, lons);
	gmt_M_free(GMT, depths);	gmt_M_free(GMT, years);		gmt_M_free(GMT, months);	gmt_M_free(GMT, days);
	gmt_M_free(GMT, hours);		gmt_M_free(GMT, minutes);
	for (i = 0; i < 100; i++)	gmt_M_free(GMT, mag_t[i]);
	gmt_M_free(GMT, mag_t);
	gmt_M_free(GMT, Out);

	if (GMT_End_IO(API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return(API->error);
	}

	Return(GMT_OK);
}
