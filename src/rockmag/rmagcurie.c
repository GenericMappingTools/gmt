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
 * API functions to support the rmagcurie application.
 *
 * Brief synopsis: Reads temperature, magnetic-parameter pairs describing
 * one or more thermomagnetic runs (one run per data segment - e.g., one
 * heating-and-cooling cycle measured on a Kappabridge or similar). Each
 * input segment is first split into monotonic heating/cooling branches at
 * every temperature turning point (rockmag_split_branches, rockmag.c), and
 * every branch is then analyzed independently for candidate Curie-point
 * estimates:
 *
 *  - Peak: the temperature of maximum signal in the branch (always
 *    reported; cheap, and appropriate when a sharp Hopkinson peak is seen).
 *
 *  - Curie-Weiss (-C<Tmin>/<Tmax>): fits 1/(column 2) versus temperature
 *    over the given window and reports its intersection with the
 *    temperature axis. This is the method Petrovsky & Kapicka (2006,
 *    J. Geophys. Res., 111, B12S27, doi:10.1029/2006JB004507) show is
 *    physically justified for magnetic susceptibility data (chi = C/(T-Tc)
 *    above Tc, so 1/chi is linear in T with an x-intercept at Tc), unlike
 *    the two-tangent method below.
 *
 *  - Two-tangent (-B<Tmin>/<Tmax> baseline, -S<Tmin>/<Tmax> flank): the
 *    classic method of Gromme et al. (1969), fitting (column 2) directly
 *    (not inverted) over two windows and intersecting the two lines. Fully
 *    justified for induced magnetization M(T), but Petrovsky & Kapicka
 *    (2006) demonstrate on real susceptibility data that it always yields
 *    a temperature above the true Curie point - by a few degrees when a
 *    sharp Hopkinson peak is present, but by several tens of degrees for a
 *    broad, gradual transition. Kept here mainly for comparison with
 *    published values that used it, or for M(T) rather than chi(T) data.
 *
 * First module of the "rockmag" supplement for rock-magnetic data
 * (hysteresis/Day-plot and IRM/backfield coercivity-unmixing modules are
 * planned to follow, sharing rockmag.c/.h - a separate supplement from
 * "paleomag", which handles directional (Dec/Inc) data instead).
 *
 *--------------------------------------------------------------------
 */

#include "gmt_dev.h"
#include "rockmag.h"

#define THIS_MODULE_CLASSIC_NAME	"rmagcurie"
#define THIS_MODULE_MODERN_NAME	"rmagcurie"
#define THIS_MODULE_LIB		"rockmag"
#define THIS_MODULE_PURPOSE	"Estimate Curie temperature from thermomagnetic heating/cooling curves"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vbdefghioq"

struct RMAGCURIE_CTRL {
	struct RMAGCURIE_C {	/* -C<Tmin>/<Tmax> Curie-Weiss (1/column2 vs T) fit window */
		bool active;
		double min, max;
	} C;
	struct RMAGCURIE_B {	/* -B<Tmin>/<Tmax> flat high-T baseline window (two-tangent method) */
		bool active;
		double min, max;
	} B;
	struct RMAGCURIE_S {	/* -S<Tmin>/<Tmax> steep-flank window (two-tangent method) */
		bool active;
		double min, max;
	} S;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {
	struct RMAGCURIE_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct RMAGCURIE_CTRL);
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct RMAGCURIE_CTRL *C) {
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);

	GMT_Usage (API, 0, "usage: %s [<table>] [-B<Tmin>/<Tmax>] [-C<Tmin>/<Tmax>] [-S<Tmin>/<Tmax>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, -2, "Input columns are temperature (same units as -B/-C/-S, typically degrees "
		"Celsius) and a magnetic parameter (susceptibility or induced magnetization) of one "
		"thermomagnetic run per record. Use -i to remap columns if needed. Each input segment "
		"is automatically split into monotonic heating/cooling branches at every temperature "
		"turning point (e.g., one heating-and-cooling cycle becomes two branches), and every "
		"branch is analyzed independently.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C<Tmin>/<Tmax>");
	GMT_Usage (API, -2, "Fit 1/(column 2) versus temperature over this window and report its "
		"intersection with the temperature axis as the Curie-Weiss estimate of the Curie point "
		"(Petrovsky & Kapicka, 2006, J. Geophys. Res., 111, B12S27). Recommended when column 2 "
		"is a magnetic susceptibility: pick a window in the clearly paramagnetic range above "
		"the transition and check the reported R^2 (should be close to 1) - the intercept is "
		"sensitive to the choice of window, so do not rely on a single unexamined guess.");
	GMT_Usage (API, 1, "\n-B<Tmin>/<Tmax>");
	GMT_Usage (API, -2, "Flat high-temperature baseline window for the classic two-tangent method "
		"(Gromme et al., 1969). Must be combined with -S. Warning: Petrovsky & Kapicka (2006) "
		"show this method, although valid for induced magnetization M(T), systematically "
		"overestimates the Curie point when applied to susceptibility data - by a few degrees "
		"in the best case (a sharp Hopkinson peak) and by several tens of degrees when the "
		"transition is broad and gradual. Prefer -C for susceptibility data; -B/-S is offered "
		"mainly for comparison with published values that used this method, or for true M(T) "
		"data, for which the two-tangent method is physically justified.");
	GMT_Usage (API, 1, "\n-S<Tmin>/<Tmax>");
	GMT_Usage (API, -2, "Steep-flank window for the two-tangent method. Must be combined with -B.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OUTPUT:\n");
	GMT_Usage (API, -2, "One record per branch, preceded by a segment header naming the branch "
		"(heating or cooling) and its temperature range. Columns are: (1,2) temperature and value "
		"of the peak signal; (3,4,5) Curie-Weiss Tc, its R^2 and the number of points fitted (-C); "
		"(6) two-tangent Tc; (7,8) R^2 and number of points of the -B baseline fit; (9,10) R^2 and "
		"number of points of the -S flank fit; (11) number of points in the branch. Columns of a "
		"method that was not requested are NaN. The point counts are what make the R^2 next to them "
		"interpretable: a window catching only 2 points always yields R^2 = 1.");
	GMT_Option (API, "V,bi,di,e,f,g,h,i");
	GMT_Usage (API, -2, "Use -q to exclude rows (e.g., a noisy tail) before branch-splitting and fitting.");
	GMT_Option (API, "q,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct RMAGCURIE_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: let GMT_Read_Data resolve these later */
				break;

			case 'C':	/* Curie-Weiss (1/column2 vs T) fit window */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if (sscanf (opt->arg, "%lf/%lf", &Ctrl->C.min, &Ctrl->C.max) != 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -C: Could not decode Tmin/Tmax in %s\n", opt->arg);
					n_errors++;
				}
				break;

			case 'B':	/* Two-tangent flat high-T baseline window */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->B.active);
				Ctrl->B.active = true;
				if (sscanf (opt->arg, "%lf/%lf", &Ctrl->B.min, &Ctrl->B.max) != 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -B: Could not decode Tmin/Tmax in %s\n", opt->arg);
					n_errors++;
				}
				break;

			case 'S':	/* Two-tangent steep-flank window */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				if (sscanf (opt->arg, "%lf/%lf", &Ctrl->S.min, &Ctrl->S.max) != 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Could not decode Tmin/Tmax in %s\n", opt->arg);
					n_errors++;
				}
				break;

			default:
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->B.active != Ctrl->S.active,
		"Options -B and -S must be used together (the two-tangent method needs both windows)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Collect the subset of [start,stop] whose temperature falls in [lo,hi] (order-
 * independent) into x (temperature) and y (either column2, or 1/column2 when
 * invert is true - points with column2 <= 0 are skipped when inverting, since
 * a non-positive susceptibility cannot be inverted into a meaningful 1/chi).
 * Points where either column is NaN are skipped as well: a single NaN would
 * otherwise propagate through the sums and turn the whole fit into NaN, which
 * looks identical to "no fit was attempted" in the output.
 * Returns the number of points collected; x and y must hold at least stop-
 * start+1 entries. */
GMT_LOCAL uint64_t rmagcurie_collect (double *temp, double *val, uint64_t start, uint64_t stop, double lo, double hi, bool invert, double *x, double *y) {
	uint64_t i, n = 0;
	double t, v;
	if (lo > hi) gmt_M_double_swap (lo, hi);
	for (i = start; i <= stop; i++) {
		t = temp[i];	v = val[i];
		if (gmt_M_is_dnan (t) || gmt_M_is_dnan (v)) continue;
		if (t < lo || t > hi) continue;
		if (invert && v <= 0.0) continue;
		x[n] = t;
		y[n] = invert ? 1.0 / v : v;
		n++;
	}
	return n;
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_rmagcurie (void *V_API, int mode, void *args) {
	int error = 0;
	uint64_t tbl, seg, i, ib, n_pts, n_branches, n_sel, n_out = 0;
	double *temp = NULL, *val = NULL, *xbuf = NULL, *ybuf = NULL;
	double slope, intercept, r2, slope_b, intercept_b, r2_b, slope_f, intercept_f, r2_f;
	double T_peak, val_peak, Tc_cw, R2_cw, n_cw, Tc_tan, R2_base, n_base, R2_flank, n_flank, out[11];
	bool ok_b, ok_f;
	char record[GMT_LEN256] = {""}, base[GMT_LEN256] = {""};

	struct GMT_OPTION *options = NULL;
	struct GMT_DATASET *Din = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_RECORD *Out = NULL;
	struct ROCKMAG_BRANCH *branch = NULL;
	struct RMAGCURIE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);

	/* special = 1: with no options at all this module still does something useful
	 * (reads stdin and reports the peak temperature of each branch), like gmtinfo
	 * and gmtconvert, so a bare invocation must not print the usage message. */
	if ((error = gmt_report_usage (API, options, 1, usage)) != GMT_NOERROR) bailout (error);

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error);
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the rmagcurie main code ----------------------------*/

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register default input sources, unless already set */
		Return (API->error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < 2) {
		GMT_Report (API, GMT_MSG_ERROR, "Input must have at least 2 columns: temperature, magnetic parameter.\n");
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (GMT_DIM_TOO_SMALL);
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (API->error);
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, 11, GMT_COL_FIX)) != GMT_NOERROR) {
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
			if (n_pts < 2) {
				GMT_Report (API, GMT_MSG_WARNING, "Segment %" PRIu64 "/%" PRIu64 " has fewer than 2 points - skipped.\n", tbl, seg);
				continue;
			}
			temp = S->data[GMT_X];	val = S->data[GMT_Y];

			if (S->header && S->header[0])
				snprintf (base, GMT_LEN256, "%s", S->header);
			else
				snprintf (base, GMT_LEN256, "Segment %" PRIu64 "/%" PRIu64, tbl, seg);

			n_branches = rockmag_split_branches (GMT, temp, n_pts, &branch);

			xbuf = gmt_M_memory (GMT, NULL, n_pts, double);
			ybuf = gmt_M_memory (GMT, NULL, n_pts, double);

			for (ib = 0; ib < n_branches; ib++) {
				uint64_t st = branch[ib].start, sp = branch[ib].stop, bn = sp - st + 1;
				if (bn < 2) {
					GMT_Report (API, GMT_MSG_WARNING, "%s: branch %" PRIu64 " has fewer than 2 points - skipped.\n", base, ib);
					continue;
				}

				/* Seed on the first non-NaN point rather than on val[st], so a NaN at the
				 * start of a branch does not make every later comparison false and leave
				 * the peak stuck at NaN. Stays NaN only if the whole branch is NaN. */
				T_peak = val_peak = GMT->session.d_NaN;
				for (i = st; i <= sp; i++) {
					if (gmt_M_is_dnan (val[i]) || gmt_M_is_dnan (temp[i])) continue;
					if (gmt_M_is_dnan (val_peak) || val[i] > val_peak) { val_peak = val[i];	T_peak = temp[i]; }
				}

				/* n_* report how many points actually entered each fit, which is what makes
				 * the R^2 next to them interpretable: a window catching only 2 points always
				 * yields R^2 = 1. They are kept even when the fit is then skipped, since the
				 * count is precisely what explains why. NaN means the method was not asked for. */
				Tc_cw = R2_cw = n_cw = GMT->session.d_NaN;
				if (Ctrl->C.active) {
					n_sel = rmagcurie_collect (temp, val, st, sp, Ctrl->C.min, Ctrl->C.max, true, xbuf, ybuf);
					n_cw = (double)n_sel;
					if (n_sel < 2)
						GMT_Report (API, GMT_MSG_WARNING, "%s: branch %" PRIu64 " (%s) has fewer than 2 usable points in -C window (NaN and non-positive values cannot be inverted) - Curie-Weiss fit skipped.\n",
							base, ib, branch[ib].heating ? "heating" : "cooling");
					else if (!rockmag_linfit (GMT, xbuf, ybuf, n_sel, &slope, &intercept, &r2) || slope == 0.0)
						GMT_Report (API, GMT_MSG_WARNING, "%s: branch %" PRIu64 " (%s) gave a degenerate Curie-Weiss fit - skipped.\n",
							base, ib, branch[ib].heating ? "heating" : "cooling");
					else {
						Tc_cw = -intercept / slope;
						R2_cw = r2;
					}
				}

				Tc_tan = R2_base = R2_flank = n_base = n_flank = GMT->session.d_NaN;
				if (Ctrl->B.active && Ctrl->S.active) {
					n_sel = rmagcurie_collect (temp, val, st, sp, Ctrl->B.min, Ctrl->B.max, false, xbuf, ybuf);
					n_base = (double)n_sel;
					ok_b = (n_sel >= 2) && rockmag_linfit (GMT, xbuf, ybuf, n_sel, &slope_b, &intercept_b, &r2_b);
					if (ok_b) R2_base = r2_b;
					else GMT_Report (API, GMT_MSG_WARNING, "%s: branch %" PRIu64 " (%s) has fewer than 2 points in -B window - two-tangent fit skipped.\n",
						base, ib, branch[ib].heating ? "heating" : "cooling");

					n_sel = rmagcurie_collect (temp, val, st, sp, Ctrl->S.min, Ctrl->S.max, false, xbuf, ybuf);
					n_flank = (double)n_sel;
					ok_f = (n_sel >= 2) && rockmag_linfit (GMT, xbuf, ybuf, n_sel, &slope_f, &intercept_f, &r2_f);
					if (ok_f) R2_flank = r2_f;
					else GMT_Report (API, GMT_MSG_WARNING, "%s: branch %" PRIu64 " (%s) has fewer than 2 points in -S window - two-tangent fit skipped.\n",
						base, ib, branch[ib].heating ? "heating" : "cooling");

					if (ok_b && ok_f) {
						if (slope_f == slope_b)
							GMT_Report (API, GMT_MSG_WARNING, "%s: branch %" PRIu64 " (%s) has parallel -B/-S fits (no intersection) - two-tangent estimate skipped.\n",
								base, ib, branch[ib].heating ? "heating" : "cooling");
						else
							Tc_tan = (intercept_b - intercept_f) / (slope_f - slope_b);
					}
				}

				snprintf (record, GMT_LEN256, "%s %s T=[%.6g,%.6g] n=%" PRIu64, base,
					branch[ib].heating ? "heating" : "cooling", temp[st], temp[sp], bn);
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);

				out[0] = T_peak;	out[1] = val_peak;
				out[2] = Tc_cw;		out[3] = R2_cw;		out[4] = n_cw;
				out[5] = Tc_tan;	out[6] = R2_base;	out[7] = n_base;
				out[8] = R2_flank;	out[9] = n_flank;	out[10] = (double)bn;
				record[0] = '\0';
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				n_out++;
			}

			gmt_M_free (GMT, xbuf);
			gmt_M_free (GMT, ybuf);
			gmt_M_free (GMT, branch);
		}
	}

	gmt_M_free (GMT, Out);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Analyzed %" PRIu64 " branch(es).\n", n_out);

	if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) Return (API->error);

	Return (GMT_NOERROR);
}
