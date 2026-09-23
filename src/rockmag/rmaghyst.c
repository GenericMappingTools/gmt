/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * API functions to support the rmaghyst application.
 *
 * Brief synopsis: Reads field, magnetization pairs describing one hysteresis
 * loop per data segment and reports the standard loop parameters.
 *
 * The high-field ends of a loop are a straight line M = +/-Ms + chi*H once the
 * ferromagnetic fraction has saturated, where chi is the para-/diamagnetic
 * susceptibility of the matrix. Each wing is fitted SEPARATELY over the -F
 * window: the two fits share the slope chi but have opposite intercepts, so
 * chi is their mean slope and Ms is half the difference of their intercepts.
 * Fitting a single line through both wings at once would instead return
 * chi + Ms/Hmax, silently absorbing the ferromagnetic signal into the slope.
 * The corrected loop Mc = M - chi*H then gives Mrs (|Mc| where H = 0) and Hc
 * (|H| where Mc = 0), each averaged over the two branches.
 *
 * Both wing fits are reported (slope, R^2 and number of points) because their
 * disagreement is the main warning sign that the -F window does not lie in a
 * genuinely saturated, linear part of the loop. The hard check is Mrs/Ms: a
 * remanence larger than the saturation it is measured against is impossible,
 * and in practice means the loop never saturated within the measured field
 * range, so Ms (and any Day plot built on it) must not be trusted. This is not
 * hypothetical: a VSM limited to ~900 Oe cannot saturate many rock samples.
 *
 * Second module of the "rockmag" supplement; shares rockmag_split_branches,
 * rockmag_linfit and rockmag_crossing with rmagcurie via rockmag.c/.h.
 *
 *--------------------------------------------------------------------
 */

#include "gmt_dev.h"
#include "rockmag.h"

#define THIS_MODULE_CLASSIC_NAME	"rmaghyst"
#define THIS_MODULE_MODERN_NAME	"rmaghyst"
#define THIS_MODULE_LIB		"rockmag"
#define THIS_MODULE_PURPOSE	"Slope-corrected hysteresis loop parameters (Ms, Mrs, Hc)"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vbdefghioq"

#define RMAGHYST_DEF_FRACTION	0.5	/* Default -F window is the upper half of the measured |H| range */

struct RMAGHYST_CTRL {
	struct RMAGHYST_F {	/* -F<Hmin>/<Hmax> window in |H| for the high-field wing fits */
		bool active;
		double min, max;
	} F;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {
	struct RMAGHYST_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct RMAGHYST_CTRL);
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct RMAGHYST_CTRL *C) {
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);

	GMT_Usage (API, 0, "usage: %s [<table>] [-F<Hmin>/<Hmax>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, -2, "Input columns are the applied field and the magnetization (or magnetic "
		"moment) of one measurement per record, one complete hysteresis loop per segment, in "
		"measurement order (from +Hmax down to -Hmax and back). Use -i to remap columns if "
		"needed. The descending and ascending branches are found automatically at the field "
		"turning point.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-F<Hmin>/<Hmax>");
	GMT_Usage (API, -2, "Window in |H| used to fit the two high-field wings, from which the "
		"para-/diamagnetic slope chi and the saturation magnetization Ms are obtained "
		"[Default is the upper half of the measured |H| range]. Choose a window where the loop "
		"is genuinely saturated, i.e. where the wings are straight: compare the two wing slopes "
		"and their R^2 in the output, and treat Mrs/Ms > 1 as proof that the window (or the "
		"whole measurement) never reached saturation.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OUTPUT:\n");
	GMT_Usage (API, -2, "One record per loop, preceded by a segment header. Columns are: "
		"(1) Mrs, the remanence at H = 0; (2) Hc, the coercivity; (3) Ms; (4) Mrs/Ms; "
		"(5) chi, the mean high-field slope; (6,7,8) slope, R^2 and number of points of the "
		"positive wing fit; (9,10,11) the same for the negative wing; (12) number of points in "
		"the loop. Mrs is the only one of these that does not depend on chi (the correction "
		"vanishes at H = 0), so it survives even when the loop did not saturate.");
	GMT_Usage (API, -2, "Note: a common manual practice is to read Ms directly off the raw curve "
		"near the edge of the measured range, with no slope correction. Column 3 is not that "
		"number: the two agree only where chi is negligible or the wings have already leveled "
		"off, and can disagree substantially otherwise -- which is part of what Mrs/Ms > 1 is "
		"warning about.");
	GMT_Option (API, "V,bi,di,e,f,g,h,i,q,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct RMAGHYST_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: let GMT_Read_Data resolve these later */
				break;

			case 'F':	/* Window in |H| for the high-field wing fits */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				if (sscanf (opt->arg, "%lf/%lf", &Ctrl->F.min, &Ctrl->F.max) != 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -F: Could not decode Hmin/Hmax in %s\n", opt->arg);
					n_errors++;
				}
				break;

			default:
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->F.active && Ctrl->F.min > Ctrl->F.max) gmt_M_double_swap (Ctrl->F.min, Ctrl->F.max);
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->F.max <= 0.0,
		"Option -F: The window is given in |H|, so Hmax must be positive\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Collect the points of H[0..n-1] whose field falls in the signed range [lo,hi]
 * into x (field) and y (magnetization), skipping NaNs. One wing at a time: the
 * caller passes [+Hmin,+Hmax] for the positive wing and [-Hmax,-Hmin] for the
 * negative one, so the two fits stay independent. */
GMT_LOCAL uint64_t rmaghyst_collect (double *H, double *M, uint64_t n, double lo, double hi, double *x, double *y) {
	uint64_t i, k = 0;
	for (i = 0; i < n; i++) {
		if (gmt_M_is_dnan (H[i]) || gmt_M_is_dnan (M[i])) continue;
		if (H[i] < lo || H[i] > hi) continue;
		x[k] = H[i];	y[k] = M[i];	k++;
	}
	return k;
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_rmaghyst (void *V_API, int mode, void *args) {
	int error = 0;
	uint64_t tbl, seg, i, ib, n_pts, n_branches, n_pos, n_neg, n_out = 0;
	double *H = NULL, *M = NULL, *Mc = NULL, *xbuf = NULL, *ybuf = NULL;
	double slope_p, icept_p, r2_p, slope_n, icept_n, r2_n;
	double chi, Ms, Mrs, Hc, ratio, Hmax, lo, hi, v, sum_mrs, sum_hc, out[12];
	uint64_t n_mrs, n_hc;
	bool ok_p, ok_n;
	char record[GMT_LEN256] = {""}, base[GMT_LEN256] = {""};

	struct GMT_OPTION *options = NULL;
	struct GMT_DATASET *Din = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_RECORD *Out = NULL;
	struct ROCKMAG_BRANCH *branch = NULL;
	struct RMAGHYST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);

	/* special = 1: with no options at all this module still does something useful
	 * (reads stdin and uses the default -F window), like gmtinfo and gmtconvert. */
	if ((error = gmt_report_usage (API, options, 1, usage)) != GMT_NOERROR) bailout (error);

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error);
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the rmaghyst main code -----------------------------*/

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		Return (API->error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < 2) {
		GMT_Report (API, GMT_MSG_ERROR, "Input must have at least 2 columns: field, magnetization.\n");
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (GMT_DIM_TOO_SMALL);
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (API->error);
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, 12, GMT_COL_FIX)) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (API->error);
	}
	GMT->current.io.multi_segments[GMT_OUT] = true;	/* To ensure we can write our own segment headers */

	Out = gmt_new_record (GMT, out, record);

	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];
			n_pts = S->n_rows;
			if (n_pts < 4) {
				GMT_Report (API, GMT_MSG_WARNING, "Segment %" PRIu64 "/%" PRIu64 " has fewer than 4 points - too few for a loop, skipped.\n", tbl, seg);
				continue;
			}
			H = S->data[GMT_X];	M = S->data[GMT_Y];

			if (S->header && S->header[0])
				snprintf (base, GMT_LEN256, "%s", S->header);
			else
				snprintf (base, GMT_LEN256, "Segment %" PRIu64 "/%" PRIu64, tbl, seg);

			Hmax = 0.0;
			for (i = 0; i < n_pts; i++) {
				if (gmt_M_is_dnan (H[i])) continue;
				v = fabs (H[i]);
				if (v > Hmax) Hmax = v;
			}
			if (Hmax == 0.0) {
				GMT_Report (API, GMT_MSG_WARNING, "%s: all fields are zero or NaN - skipped.\n", base);
				continue;
			}
			lo = (Ctrl->F.active) ? Ctrl->F.min : RMAGHYST_DEF_FRACTION * Hmax;
			hi = (Ctrl->F.active) ? Ctrl->F.max : Hmax;

			n_branches = rockmag_split_branches (GMT, H, n_pts, &branch);
			if (n_branches != 2)
				GMT_Report (API, GMT_MSG_WARNING, "%s: found %" PRIu64 " monotonic field branch(es) instead of 2 - this does not look like a single loop; using the first two.\n",
					base, n_branches);

			xbuf = gmt_M_memory (GMT, NULL, n_pts, double);
			ybuf = gmt_M_memory (GMT, NULL, n_pts, double);
			Mc   = gmt_M_memory (GMT, NULL, n_pts, double);

			/* Each wing is fitted on its own: M = +Ms + chi*H on the positive side and
			 * M = -Ms + chi*H on the negative one. */
			n_pos = rmaghyst_collect (H, M, n_pts, lo, hi, xbuf, ybuf);
			ok_p = (n_pos >= 2) && rockmag_linfit (GMT, xbuf, ybuf, n_pos, &slope_p, &icept_p, &r2_p);
			n_neg = rmaghyst_collect (H, M, n_pts, -hi, -lo, xbuf, ybuf);
			ok_n = (n_neg >= 2) && rockmag_linfit (GMT, xbuf, ybuf, n_neg, &slope_n, &icept_n, &r2_n);

			chi = Ms = GMT->session.d_NaN;
			if (ok_p && ok_n) {
				chi = 0.5 * (slope_p + slope_n);
				Ms  = 0.5 * (icept_p - icept_n);
			}
			else
				GMT_Report (API, GMT_MSG_WARNING, "%s: fewer than 2 points in the %s wing of the -F window - no slope correction, so Ms, Hc and chi are undefined.\n",
					base, (ok_p) ? "negative" : ((ok_n) ? "positive" : "positive and negative"));

			/* Mrs needs no correction (chi*H vanishes at H = 0), so it is computed from the
			 * raw loop and survives even when the wings gave us nothing. */
			for (i = 0; i < n_pts; i++)
				Mc[i] = (gmt_M_is_dnan (chi)) ? M[i] : M[i] - chi * H[i];

			sum_mrs = sum_hc = 0.0;	n_mrs = n_hc = 0;
			for (ib = 0; ib < n_branches && ib < 2; ib++) {
				uint64_t st = branch[ib].start, bn = branch[ib].stop - branch[ib].start + 1;
				double at_zero;
				if (rockmag_crossing (&H[st], &Mc[st], bn, 0.0, &at_zero)) { sum_mrs += fabs (at_zero);	n_mrs++; }
				if (rockmag_crossing (&Mc[st], &H[st], bn, 0.0, &at_zero)) { sum_hc  += fabs (at_zero);	n_hc++;  }
			}
			Mrs = (n_mrs) ? sum_mrs / (double)n_mrs : GMT->session.d_NaN;
			Hc  = (n_hc)  ? sum_hc  / (double)n_hc  : GMT->session.d_NaN;
			if (n_mrs == 0) GMT_Report (API, GMT_MSG_WARNING, "%s: no branch crosses H = 0 - Mrs undefined.\n", base);
			if (n_hc == 0 && !gmt_M_is_dnan (chi)) GMT_Report (API, GMT_MSG_WARNING, "%s: the corrected loop never crosses M = 0 - Hc undefined.\n", base);

			ratio = Mrs / Ms;
			if (ratio > 1.0)
				GMT_Report (API, GMT_MSG_WARNING, "%s: Mrs/Ms = %g is physically impossible (remanence cannot exceed saturation). The loop almost certainly did not saturate within the -F window, so Ms is not usable - and neither is any Day plot built on it.\n",
					base, ratio);

			snprintf (record, GMT_LEN256, "%s |H|max=%.6g -F%.6g/%.6g n=%" PRIu64, base, Hmax, lo, hi, n_pts);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);

			out[0] = Mrs;		out[1] = Hc;		out[2] = Ms;		out[3] = ratio;
			out[4] = chi;
			out[5] = (ok_p) ? slope_p : GMT->session.d_NaN;
			out[6] = (ok_p) ? r2_p    : GMT->session.d_NaN;
			out[7] = (double)n_pos;
			out[8] = (ok_n) ? slope_n : GMT->session.d_NaN;
			out[9] = (ok_n) ? r2_n    : GMT->session.d_NaN;
			out[10] = (double)n_neg;
			out[11] = (double)n_pts;
			record[0] = '\0';
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			n_out++;

			gmt_M_free (GMT, xbuf);
			gmt_M_free (GMT, ybuf);
			gmt_M_free (GMT, Mc);
			gmt_M_free (GMT, branch);
		}
	}

	gmt_M_free (GMT, Out);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Analyzed %" PRIu64 " loop(s).\n", n_out);

	if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);

	Return (GMT_NOERROR);
}
