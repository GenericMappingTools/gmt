/*--------------------------------------------------------------------
*
*	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
*	See LICENSE.TXT file for copying and redistribution conditions.
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
 * gmtspatial performs miscellaneous geospatial operations on polygons, such
 * as truncating them against a clipping polygon, calculate areas, find
 * crossings with other polygons, etc.
 *
 * Author:	Paul Wessel
 * Date:	10-Jun-2009
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"gmtspatial"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Geospatial operations on points, lines and polygons"
#define THIS_MODULE_KEYS	"<D{,DD(=f,ND(=,TD(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:RVabdefghijos" GMT_OPT("HMm")

#define POL_IS_CW	1
#define POL_IS_CCW	0

#define GMT_W	3

#define POL_UNION		1
#define POL_INTERSECTION	2
#define POL_CLIP		3
#define POL_SPLIT		4
#define POL_JOIN		5
#define POL_HOLE		6

#define PW_TESTING
#define MIN_AREA_DIFF		0.01;	/* If two polygons have areas that differ more than 1 % of each other then they are not the same feature */
#define MIN_SEPARATION		0	/* If the two closest points for two features are > 0 units apart then they are not the same feature */
#define MIN_CLOSENESS		0.01	/* If two close segments has an mean separation exceeding 1% of segment length, then they are not the same feature */
#define MIN_SUBSET		2.0	/* If two close segments deemed approximate fits has lengths that differ by this factor then they are sub/super sets of each other */

struct DUP {	/* Holds information on which single segment is closest to the current test segment */
	uint64_t point;
	uint64_t segment;
	unsigned int table;
	int mode;
	bool inside;
	double distance;
	double mean_distance;
	double closeness;
	double setratio;
	double a_threshold;
	double d_threshold;
	double c_threshold;
	double s_threshold;
};

struct DUP_INFO {
	uint64_t point;
	int mode;
	double distance;
	double closeness;
	double setratio;
};

struct GMTSPATIAL_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -Aa<min_dist>[unit], -A[unit] */
		bool active;
		unsigned int mode;
		int smode;
		double min_dist;
		char unit;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D[pol] */
		bool active;
		int mode;
		char unit;
		char *file;
		struct DUP I;
	} D;
	struct E {	/* -E+n|p */
		bool active;
		unsigned int mode;
	} E;
	struct F {	/* -F */
		bool active;
		unsigned int geometry;
	} F;
	struct I {	/* -I[i|e] */
		bool active;
		unsigned int mode;
	} I;
	struct L {	/* -L */
		bool active;
		char unit;
		double s_cutoff, path_noise, box_offset;
	} L;
	struct N {	/* -N<file>[+a][+p>ID>][+r][+z] */
		bool active;
		bool all;	/* All points in lines and polygons must be inside a polygon for us to report ID */
		unsigned int mode;	/* 0 for reporting ID in -Z<ID> header, 1 via data column, 2 just as a report */
		unsigned int ID;	/* If 1 we use running numbers */
		char *file;
	} N;
	struct Q {	/* -Q[+c<min>/<max>][+h][+l][+p][+s[a|d]] */
		bool active;
		bool header;	/* Place dimension and centroid in segment headers */
		bool area;		/* Apply range test on dimension */
		bool sort;		/* Sort segments based on dimension */
		unsigned int mode;	/* 0 use input as is, 1 force to line, 2 force to polygon */
		unsigned int dmode;	/* for geo data: 1 = flat earth, 2 = great circle, 3 = geodesic (for distances) */
		int dir;			/* For segment sorting: -1 is descending, +1 is ascending [Default] */
		double limit[2];	/* Min and max area or length for output segments */
		char unit;
	} Q;
	struct S {	/* -S[u|i|c|j|h] */
		bool active;
		unsigned int mode;
	} S;
	struct T {	/* -T[pol] */
		bool active;
		char *file;
	} T;
};

struct PAIR {
	double node;
	uint64_t pos;
};

#ifdef __APPLE__
/* macOX has it built in, so ensure we define this flag */
#define HAVE_MERGESORT
#endif

#ifndef HAVE_MERGESORT
#include "mergesort.c"
#endif

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTSPATIAL_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GMTSPATIAL_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.unit = 'X';		/* Cartesian units as default */
	C->L.box_offset = 1.0e-10;	/* Minimum significant amplitude */
	C->L.path_noise = 1.0e-10;	/* Minimum significant amplitude */
	C->D.unit = 'X';		/* Cartesian units as default */
	C->D.I.a_threshold = MIN_AREA_DIFF;
	C->D.I.d_threshold = MIN_SEPARATION;
	C->D.I.c_threshold = MIN_CLOSENESS;
	C->D.I.s_threshold = MIN_SUBSET;
	C->Q.mode = GMT_IS_POINT;	/* Undecided on line vs poly */
	C->Q.dmode = 2;			/* Great-circle distance if not specified */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTSPATIAL_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);	
	gmt_M_str_free (C->D.file);	
	gmt_M_str_free (C->N.file);	
	gmt_M_str_free (C->T.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL unsigned int area_size (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, double *out, int geo) {
	uint64_t i;
	double wesn[4], xx, yy, size, ix, iy;
	double *xp = NULL, *yp = NULL;
	
	wesn[XLO] = wesn[YLO] = DBL_MAX;
	wesn[XHI] = wesn[YHI] = -DBL_MAX;
	
	gmt_centroid (GMT, x, y, n, out, geo);	/* Get mean location */
	
	for (i = 0; i < n; i++) {
		wesn[XLO] = MIN (wesn[XLO], x[i]);
		wesn[XHI] = MAX (wesn[XHI], x[i]);
		wesn[YLO] = MIN (wesn[YLO], y[i]);
		wesn[YHI] = MAX (wesn[YHI], y[i]);
	}
	xp = gmt_M_memory (GMT, NULL, n, double);	yp = gmt_M_memory (GMT, NULL, n, double);
	if (geo == 1) {	/* Initializes GMT projection parameters to the -JA settings */
		GMT->current.proj.projection_GMT = GMT->current.proj.projection = GMT_LAMB_AZ_EQ;
		GMT->current.proj.unit = 1.0;
		GMT->current.proj.pars[3] = 39.3700787401574814;
		GMT->common.R.oblique = false;
		GMT->common.J.active = true;
		GMT->current.setting.map_line_step = 1.0e7;	/* To avoid nlon/nlat being huge */
		GMT->current.proj.pars[0] = out[GMT_X];
		GMT->current.proj.pars[1] = out[GMT_Y];
		if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, wesn), "")) {
			gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
			return (0);
		}
	
		ix = 1.0 / GMT->current.proj.scale[GMT_X];
		iy = 1.0 / GMT->current.proj.scale[GMT_Y];
	
		for (i = 0; i < n; i++) {
			gmt_geo_to_xy (GMT, x[i], y[i], &xx, &yy);
			xp[i] = (xx - GMT->current.proj.origin[GMT_X]) * ix;
			yp[i] = (yy - GMT->current.proj.origin[GMT_Y]) * iy;
		}
	}
	else {	/* Just take out mean coordinates */
		for (i = 0; i < n; i++) {
			xp[i] = x[i] - out[GMT_X];
			yp[i] = y[i] - out[GMT_Y];
		}
	}
	
	size = gmt_pol_area (xp, yp, n);
	gmt_M_free (GMT, xp);
	gmt_M_free (GMT, yp);
	if (geo) size *= (GMT->current.map.dist[GMT_MAP_DIST].scale * GMT->current.map.dist[GMT_MAP_DIST].scale);
	out[GMT_Z] = fabs (size);
	return ((size < 0.0) ? POL_IS_CCW : POL_IS_CW);
}

GMT_LOCAL void length_size (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, double *out) {
	uint64_t i;
	double length = 0.0, mid, f, *s = NULL;
	
	assert (n > 0);

	/* Estimate 'average' position */
	
	s = gmt_M_memory (GMT, NULL, n, double);
	for (i = 1; i < n; i++) {
		length += gmt_distance (GMT, x[i-1], y[i-1], x[i], y[i]);
		s[i] = length;
	}
	mid = 0.5 * length;
	i = 0;
	while (s[i] <= mid) i++;	/* Find mid point length-wise */
	f = (mid - s[i-1]) / (s[i] - s[i-1]);
	out[GMT_X] = x[i-1] + f * (x[i] - x[i-1]);
	out[GMT_Y] = y[i-1] + f * (y[i] - y[i-1]);
	out[GMT_Z] = length;
	gmt_M_free (GMT, s);
}

GMT_LOCAL int comp_pairs (const void *a, const void *b) {
	const struct PAIR *xa = a, *xb = b;
	/* Sort on node value */
	if (xa->node < xb->node) return (-1);
	if (xa->node > xb->node) return (+1);
	return (0);
}

GMT_LOCAL void write_record (struct GMT_CTRL *GMT, double **R, uint64_t n, uint64_t p) {
	uint64_t c;
	double out[GMT_MAX_COLUMNS];
	struct GMT_RECORD Out;
	Out.data = out;	Out.text = NULL;
	for (c = 0; c < n; c++) out[c] = R[c][p];
	GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, &Out);
}

GMT_LOCAL int is_duplicate (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, struct GMT_DATASET *D, struct DUP *I, struct DUP_INFO **L) {
	/* Given single line segment S and a dataset of many line segments in D, determine the closest neighbor
	 * to S in D (call it S'), and if "really close" it might be a duplicate or slight revision to S.
	 * There might be several features S' in D close to S so we return how many near or exact matches we
	 * found and pass the details via L.  We return 0 if no duplicates are found.  If some are found then
	 * these are the possible types of result per match:
	 * +1	: S has identical duplicate in D (S == S')
	 * -1	: S has identical duplicate in D (S == S'), but is reversed
	 * +2	: S is very close to S', probably a slight revised version
	 * -2	: S is very close to S', probably a slight revised version, but is reversed
	 * +3	: Same as +2 but S is likely a subset of S'
	 * -3	: Same as -2 but S is likely a reversed subset of S'
	 * +4	: Same as +2 but S is likely a superset of S'
	 * -4	: Same as -2 but S is likely a reversed superset of S'
	 *  5   : Two lines split exactly at the Dateline
	 * The algorithm first finds the smallest separation from any point on S to the line Sp.
	 * We then reverse the situation and check the smallest separation from any point on any
	 * of the Sp candidates to the line S.
	 * If the smallest distance found exceeds I->d_threshold then we move on (not close enough).
	 * We next compute the mean (or median, with +Ccmax) closeness, defined as the ratio between
	 * the mean (or median) separation between points on S and the line Sp (or vice versa) and
	 * the length of S (or Sp).  We compute it both ways since the segments may differ in lengths
	 * and degree of overlap (i.e., subsets or supersets).
	 * NOTE: Lines that intersect obviously has a zero min distance but since we only compute
	 * distances from the nodes on one line to the nearest point along the other line we will
	 * not detect such crossings.  For most situations this should not matter much (?).
	 */
	
	bool status;
	unsigned int k, tbl, n_close = 0, n_dup = 0, mode1, mode3;
	uint64_t row, seg, pt, np, sno, *n_sum = NULL;
	int k_signed;
	double dist, f_seg, f_pt, d1, d2, closest, length[2], separation[2], close[2], *d_mean = NULL;
	double med_separation[2], med_close[2], high = 0, low = 0, use_length, use_sep, use_close, *sep = NULL;
	struct GMT_DATASEGMENT *Sp = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Determine the segments in D closest to our segment\n");
	I->distance = I->mean_distance = DBL_MAX;
	mode3 = 3 + 10 * I->inside;	/* Set gmt_near_lines modes */
	mode1 = 1 + 10 * I->inside;	/* Set gmt_near_lines modes */
	
	d_mean = gmt_M_memory (GMT, NULL, D->n_segments, double);		/* Mean distances from points along S to other lines */
	n_sum = gmt_M_memory (GMT, NULL, D->n_segments, uint64_t);	/* Number of distances from points along S to other lines */
	
	/* We first want to obtain the mean distance from one segment to all others.  We do this by computing the
	 * nearest distance from each point along our segment S to the other segments and compute the average of
	 * all those distances.  Then we reverse the search and continue to accumulate averages */
	
	/* Process each point along the trace in S and find sum of nearest distances for each segment in table */
	for (row = 0; row < S->n_rows; row++) {
		for (sno = tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++, sno++) {	/* For each segment in current table */
				if (D->table[tbl]->segment[seg]->n_rows == 0) continue;	/* Skip segments with no records (may be itself) */
				dist = DBL_MAX;	/* Reset for each line to find distance to that line */
				(void) gmt_near_a_line (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], seg, D->table[tbl]->segment[seg], mode3, &dist, &f_seg, &f_pt);
				d_mean[sno] += dist;	/* Sum of distances to this segment */
				n_sum[sno]++;		/* Number of such distances */
			}
		}
	}

	/* Must also do the reverse test: for each point for each line in the table, compute distance to S; it might be shorter */
	for (sno = tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++, sno++) {	/* For each segment in current table */
			Sp = D->table[tbl]->segment[seg];	/* This is S', one of the segments that is close to S */
			for (row = 0; row < Sp->n_rows; row++) {	/* Process each point along the trace in S and find nearest distance for each segment in table */
				dist = DBL_MAX;		/* Reset for each line to find distance to that line */
				(void) gmt_near_a_line (GMT, Sp->data[GMT_X][row], Sp->data[GMT_Y][row], seg, S, mode3, &dist, &f_seg, &f_pt);
				d_mean[sno] += dist;	/* Sum of distances to this segment */
				n_sum[sno]++;		/* Number of such distances */
			}
		}
	}
	/* Compute the average distances */
	for (sno = 0; sno < D->n_segments; sno++) {
		d_mean[sno] = (n_sum[sno] > 0) ? d_mean[sno] / n_sum[sno] : DBL_MAX;
	}
	gmt_M_free (GMT, n_sum);

	/* Process each point along the trace in S and find nearest distance for each segment in table */
	for (row = 0; row < S->n_rows; row++) {
		for (sno = tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++, sno++) {	/* For each segment in current table */
				SH = gmt_get_DS_hidden (D->table[tbl]->segment[seg]);
				dist = DBL_MAX;	/* Reset for each line to find distance to that line */
				status = gmt_near_a_line (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], seg, D->table[tbl]->segment[seg], mode3, &dist, &f_seg, &f_pt);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the line segment */
				pt = lrint (f_pt);	/* We know f_seg == seg so no point assigning that */
				if (dist < I->distance && d_mean[sno] < I->mean_distance) {	/* Keep track of the single closest feature */
					I->point = pt;
					I->segment = seg;
					I->distance = dist;
					I->mean_distance = d_mean[sno];
					I->table = tbl;
				}
				if (dist > I->d_threshold) continue;	/* Not close enough for duplicate consideration */
				if (SH->mode == 0) {
					n_close++;
					L[tbl][seg].distance = DBL_MAX;
				}
				SH->mode = 1;	/* Use mode to flag segments that are close enough */
				if (dist < L[tbl][seg].distance) {	/* Keep track of the closest feature */
					L[tbl][seg].point = pt;
					L[tbl][seg].distance = dist;
				}
			}
		}
	}
	
	/* Must also do the reverse test: for each point for each line in the table, compute distance to S; it might be shorter */
	
	for (sno = tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++, sno++) {	/* For each segment in current table */
			Sp = D->table[tbl]->segment[seg];	/* This is S', one of the segments that is close to S */
			SH = gmt_get_DS_hidden (Sp);
			for (row = 0; row < Sp->n_rows; row++) {	/* Process each point along the trace in S and find nearest distance for each segment in table */
				dist = DBL_MAX;		/* Reset for each line to find distance to that line */
				status = gmt_near_a_line (GMT, Sp->data[GMT_X][row], Sp->data[GMT_Y][row], seg, S, mode3, &dist, &f_seg, &f_pt);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the line segment */
				pt = lrint (f_pt);
				if (dist < I->distance && d_mean[sno] < I->mean_distance) {	/* Keep track of the single closest feature */
					I->point = pt;
					I->segment = seg;
					I->distance = dist;
					I->mean_distance = d_mean[sno];
					I->table = tbl;
				}
				if (dist > I->d_threshold) continue;	/* Not close enough for duplicate consideration */
				if (SH->mode == 0) {
					n_close++;
					L[tbl][seg].distance = DBL_MAX;
				}
				SH->mode = 1;	/* Use mode to flag segments that are close enough */
				if (dist < L[tbl][seg].distance) {	/* Keep track of the closest feature */
					L[tbl][seg].point = pt;
					L[tbl][seg].distance = dist;
				}
			}
		}
	}
	gmt_M_free (GMT, d_mean);
	
	if (n_close == 0)
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "No other segment found within dmax [probably due to +p requirement]\n");
	else
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Closest segment (Table %d, segment %" PRIu64 ") is %.3f km away; %d segments found within dmax\n", I->table, I->segment, I->distance, n_close);
	
	/* Here we have found the shortest distance from a point on S to another segment; S' is segment number seg */
	
	if (I->distance > I->d_threshold) return (0);	/* We found no duplicates or slightly different versions of S */
	
	/* Here we need to compare one or more S' candidates and S a bit more. */
	
	for (row = 1, length[0] = 0.0; row < S->n_rows; row++) {	/* Compute length of S once */
		length[0] += gmt_distance (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], S->data[GMT_X][row-1], S->data[GMT_Y][row-1]);
	}

	for (tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in current table */
			SH = gmt_get_DS_hidden (D->table[tbl]->segment[seg]);
			if (SH->mode != 1) continue;	/* Not one of the close segments we flagged earlier */
			SH->mode = 0;			/* Remove this temporary flag */
			
			Sp = D->table[tbl]->segment[seg];	/* This is S', one of the segments that is close to S */
			if (S->n_rows == Sp->n_rows) {	/* Exactly the same number of data points; check for identical duplicate (possibly reversed) */
				for (row = 0, d1 = d2 = 0.0; row < S->n_rows; row++) {	/* Compare each point along the trace in S and Sp and S and Sp-reversed */
					d1 += gmt_distance (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], Sp->data[GMT_X][row], Sp->data[GMT_Y][row]);
					d2 += gmt_distance (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], Sp->data[GMT_X][S->n_rows-row-1], Sp->data[GMT_Y][S->n_rows-row-1]);
				}
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "S to Sp of same length and gave d1 = %g and d2 = %g\n", d1, d2);
				if (gmt_M_is_zero (d1)) { /* Exact duplicate */
					L[tbl][seg].mode = +1;
					n_dup++;
					continue;
				}
				else if (gmt_M_is_zero (d2)) {	/* Exact duplicate but reverse order */
					L[tbl][seg].mode = -1;
					n_dup++;
					continue;
				}
			}
	
			/* We get here when S' is not an exact duplicate of S, but approximate.
			 * We compute the mean [and possibly median] separation between S' and S
			 * and the closeness ratio (separation/length). */
	
			separation[0] = 0.0;
			if (I->mode) {	/* Various items needed to compute median separation */
				sep = gmt_M_memory (GMT, NULL, MAX (S->n_rows, Sp->n_rows), double);
				low = DBL_MAX;
				high = -DBL_MAX;
			}
			for (row = np = 0; row < S->n_rows; row++) {	/* Process each point along the trace in S */
				dist = DBL_MAX;
				status = gmt_near_a_line (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], seg, Sp, mode1, &dist, NULL, NULL);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the Sp segment */
				separation[0] += dist;
				if (I->mode) {	/* Special processing for median calculation */
					sep[np] = dist;
					if (dist < low)  low  = dist;
					if (dist > high) high = dist;
				}
				np++;	/* Number of points within the overlap zone */
			}
			separation[0] = (np > 1) ? separation[0] / np : DBL_MAX;	/* Mean distance between S and S' */
			use_length = (np) ? length[0] * np / S->n_rows : length[0];	/* ~reduce length to overlap section assuming equal point spacing */
			close[0] = (np > 1) ? separation[0] / use_length : DBL_MAX;	/* Closeness as viewed from S */
			use_sep = (separation[0] == DBL_MAX) ? GMT->session.d_NaN : separation[0];
			use_close = (close[0] == DBL_MAX) ? GMT->session.d_NaN : close[0];
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
			            "S has length %.3f km, has mean separation to Sp of %.3f km, and a closeness ratio of %g [n = %" PRIu64 "/%" PRIu64 "]\n",
			            length[0], use_sep, use_close, np, S->n_rows);
			if (I->mode) {
				if (np > 1) {
					gmt_median (GMT, sep, np, low, high, separation[0], &med_separation[0]);
					med_close[0] = med_separation[0] / use_length;
				}
				else med_close[0] = DBL_MAX;
				use_sep = (np == 0 || med_separation[0] == DBL_MAX) ? GMT->session.d_NaN : med_separation[0];
				use_close = (np == 0 || med_close[0] == DBL_MAX) ? GMT->session.d_NaN : med_close[0];
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "S  has median separation to Sp of %.3f km, and a robust closeness ratio of %g\n",
				            use_sep, use_close);
			}
	
			/* Must now compare the other way */
	
			separation[1] = length[1] = 0.0;
			if (I->mode) {	/* Reset for median calculation */
				low  = +DBL_MAX;
				high = -DBL_MAX;
			}
			for (row = np = 0; row < Sp->n_rows; row++) {	/* Process each point along the trace in S' */
				dist = DBL_MAX;		/* Reset for each line to find distance to that line */
				status = gmt_near_a_line (GMT, Sp->data[GMT_X][row], Sp->data[GMT_Y][row], seg, S, mode1, &dist, NULL, NULL);
				if (row) length[1] += gmt_distance (GMT, Sp->data[GMT_X][row], Sp->data[GMT_Y][row], Sp->data[GMT_X][row-1], Sp->data[GMT_Y][row-1]);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the Sp segment */
				separation[1] += dist;
				if (I->mode) {	/* Special processing for median calculation */
					sep[np] = dist;
					if (dist < low)  low  = dist;
					if (dist > high) high = dist;
				}
				np++;	/* Number of points within the overlap zone */
			}
            		separation[1] = (np > 1) ? separation[1] / np : DBL_MAX;		/* Mean distance between S' and S */
			use_length = (np) ? length[1] * np / Sp->n_rows : length[1];	/* ~reduce length to overlap section assuming equal point spacing */
			close[1] = (np > 1) ? separation[1] / use_length : DBL_MAX;		/* Closeness as viewed from S' */
			use_sep = (separation[1] == DBL_MAX) ? GMT->session.d_NaN : separation[1];
			use_close = (close[1] == DBL_MAX) ? GMT->session.d_NaN : close[1];
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Sp has length %.3f km, has mean separation to S of %.3f km, and a closeness ratio of %g [n = %" PRIu64 "/%" PRIu64 "]\n",
				length[1], use_sep, use_close, np, Sp->n_rows);
			if (I->mode) {
				if (np > 1) {
					gmt_median (GMT, sep, np, low, high, separation[1], &med_separation[1]);
					med_close[1] = med_separation[1] / use_length;
				}
				else med_close[1] = DBL_MAX;
				gmt_M_free (GMT, sep);
				use_sep = (med_separation[1] == DBL_MAX) ? GMT->session.d_NaN : med_separation[1];
				use_close = (med_close[1] == DBL_MAX) ? GMT->session.d_NaN : med_close[1];
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Sp has median separation to S  of %.3f km, and a robust closeness ratio of %g\n",
					use_sep, use_close);
				k = (med_close[0] <= med_close[1]) ? 0 : 1;	/* Pick the setup with the smallest robust closeness */
				closest = med_close[k];		/* The longer the segment and the closer they are, the smaller the closeness */
			}
			else {
				k = (close[0] <= close[1]) ? 0 : 1;	/* Pick the setup with the smallest robust closeness */
				closest = close[k];		/* The longer the segment and the closer they are, the smaller the closeness */
			}

			if (closest > I->c_threshold) continue;	/* Not a duplicate or slightly different version of S */
			L[tbl][seg].closeness = closest;	/* The longer the segment and the closer they are the smaller the I->closeness */

			/* Compute distances from first point in S to first (d1) and last (d2) in S' and use this info to see if S' is reversed */
			d1 = gmt_distance (GMT, S->data[GMT_X][0], S->data[GMT_Y][0], Sp->data[GMT_X][0], Sp->data[GMT_Y][0]);
			d2 = gmt_distance (GMT, S->data[GMT_X][0], S->data[GMT_Y][0], Sp->data[GMT_X][Sp->n_rows-1], Sp->data[GMT_Y][Sp->n_rows-1]);
	
			L[tbl][seg].setratio = (length[0] > length[1]) ? length[0] / length[1] : length[1] / length[0];	/* Ratio of length is used to detect subset (or superset) */
	
			if (length[0] < (length[1] / I->s_threshold))
				k_signed = 3;	/* S is a subset of Sp */
			else if (length[1] < (length[0] / I->s_threshold))
				k_signed = 4;	/* S is a superset of Sp */
			else
				k_signed = 2;	/* S and S' are practically the same feature */
			L[tbl][seg].mode = (d1 < d2) ? k_signed : -k_signed;	/* Negative means Sp is reversed w.r.t. S */
			/* Last check: If two lines are split at 180 the above will find them to be approximate duplicates even
			 * though they just share one point (at 180 longitude) */
			if (L[tbl][seg].closeness < GMT_CONV8_LIMIT && L[tbl][seg].distance < GMT_CONV8_LIMIT) {
				if (fabs (S->data[GMT_X][0]) == 180.0 && (fabs (Sp->data[GMT_X][0]) == 180.0 || fabs (Sp->data[GMT_X][Sp->n_rows-1]) == 180.0))
					L[tbl][seg].mode = 5;
				else if (fabs (S->data[GMT_X][S->n_rows-1]) == 180.0 && (fabs (Sp->data[GMT_X][0]) == 180.0 || fabs (Sp->data[GMT_X][Sp->n_rows-1]) == 180.0))
					L[tbl][seg].mode = 5;
			}
			n_dup++;
		}
	}
	return (n_dup);
}

struct NN_DIST {
	double data[4];	/* Up to x,y,z,weight */
	double distance;	/* Distance to nearest neighbor */
	int64_t ID;		/* Input ID # of this point (original input record number 0,1,...)*/
	int64_t neighbor;	/* Input ID # of this point's neighbor */
};

struct NN_INFO {
	int64_t sort_rec;	/* Input ID # of this point's neighbor */
	int64_t orig_rec;	/* Rec # of this point */
};

GMT_LOCAL int compare_nn_points (const void *point_1v, const void *point_2v) {
	/*  Routine for qsort to sort NN data structure on distance.
		*/
	const struct NN_DIST *point_1 = point_1v, *point_2 = point_2v;
	
	if (gmt_M_is_dnan (point_1->distance)) return (+1);
	if (gmt_M_is_dnan (point_2->distance)) return (-1);
	if (point_1->distance < point_2->distance) return (-1);
	if (point_1->distance > point_2->distance) return (+1);
	return (0);
}

GMT_LOCAL struct NN_DIST *NNA_update_dist (struct GMT_CTRL *GMT, struct NN_DIST *P, uint64_t *n_points) {
	/* Return array of NN results sorted on smallest distances */
	int64_t k, k2, np;
	double *distance = gmt_M_memory (GMT, NULL, *n_points, double);
#ifdef DEBUG
	static int iteration = 0;
#endif
	np = *n_points;
	for (k = 0; k < (np-1); k++) {
		if (gmt_M_is_dnan (P[k].distance)) continue;	/* Skip deleted point */
		P[k].distance = DBL_MAX;
		/* We split the loop over calculation of distance from the loop over assignments since
		 * if OpenMP is used then we cannot interchange k and k2 as there may be overprinting.
		 */
#ifdef _OPENMP
#pragma omp parallel for private(k2) shared(k,np,GMT,P,distance)
#endif 
		for (k2 = k + 1; k2 < np; k2++) {
			if (gmt_M_is_dnan (P[k2].distance)) continue;	/* Skip deleted point */
			distance[k2] = gmt_distance (GMT, P[k].data[GMT_X], P[k].data[GMT_Y], P[k2].data[GMT_X], P[k2].data[GMT_Y]);
		}
		for (k2 = k + 1; k2 < np; k2++) {
			if (gmt_M_is_dnan (P[k2].distance)) continue;	/* Skip deleted point */
			if (distance[k2] < P[k].distance) {
				P[k].distance = distance[k2];
				P[k].neighbor = P[k2].ID;
			}
			if (distance[k2] < P[k2].distance) {
				P[k2].distance = distance[k2];
				P[k2].neighbor = P[k].ID;
			}
		}
	}
	gmt_M_free (GMT, distance);

	/* Prefer mergesort since qsort is not stable for equalities */
	mergesort (P, np, sizeof (struct NN_DIST), compare_nn_points);	/* Sort on small to large distances */

	for (k = np; k > 0 && gmt_M_is_dnan (P[k-1].distance); k--);	/* Skip the NaN distances that were placed at end */
	*n_points = k;	/* Update point count */
#ifdef DEBUG
	if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "===> Iteration = %d\n", iteration);
		for (k = 0; k < (int64_t)(*n_points); k++)
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%6d\tID=%6d\tNeighbor=%6d\tDistance = %.12g\n", (int)k, (int)P[k].ID, P[k].neighbor, P[k].distance);
	}
	iteration++;
#endif
	return (P);
}

GMT_LOCAL struct NN_DIST *NNA_init_dist (struct GMT_CTRL *GMT, struct GMT_DATASET *D, uint64_t *n_points) {
	/* Return array of NN results sorted on smallest distances */
	uint64_t tbl, seg, row, col, n_cols;
	int64_t k, np = 0;	/* Must be signed due to Win OpenMP retardedness */
	double *distance = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct NN_DIST *P = gmt_M_memory (GMT, NULL, D->n_records, struct NN_DIST);
	
	n_cols = MIN (D->n_columns, 4);	/* Expects lon,lat and makes room for at least z, w and other columns */
	np = (int64_t)(D->n_records * (D->n_records - 1)) / 2;
	distance = gmt_M_memory (GMT, NULL, np, double);
	for (tbl = 0, np = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			S = D->table[tbl]->segment[seg];
			for (row = 0; row < S->n_rows; row++) {
				for (col = 0; col < n_cols; col++) P[np].data[col] = S->data[col][row];	/* Duplicate coordinates */
				if (n_cols < 4) P[np].data[GMT_W] = 1.0;	/* No weight provided, set to unity */
				P[np].ID = (uint64_t)np;	/* Assign ID based on input record # from 0 */
				P[np].distance = DBL_MAX;
				/* We split the loop over calculation of distance from the loop over assignments since
		 		 * if OpenMP is used then we cannot interchange k and np as there may be overprinting.
		 		 */
#ifdef _OPENMP
#pragma omp parallel for private(k) shared(np,GMT,P,distance)
#endif 
				for (k = 0; k < np; k++) {	/* Get distances to other points */
					distance[k] = gmt_distance (GMT, P[k].data[GMT_X], P[k].data[GMT_Y], P[np].data[GMT_X], P[np].data[GMT_Y]);
				}
				for (k = 0; k < np; k++) {	/* Compare this point to all previous points */
					if (distance[k] < P[k].distance) {	/* Update shortest distance so far, and with which neighbor */
						P[k].distance = distance[k];
						P[k].neighbor = np;
					}
					if (distance[k] < P[np].distance) {	/* Update shortest distance so far, and with which neighbor */
						P[np].distance = distance[k];
						P[np].neighbor = k;
					}
				}
				np++;
			}
		}
	}
	gmt_M_free (GMT, distance);

	/* Prefer mergesort since qsort is not stable for equalities */
	mergesort (P, np, sizeof (struct NN_DIST), compare_nn_points);
	
	*n_points = (uint64_t)np;
#ifdef DEBUG
	if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "===> Initialization\n");
		for (k = 0; k < (int64_t)(*n_points); k++)
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%6d\tID=%6d\tNeighbor=%6d\tDistance = %.12g\n", (int)k, (int)P[k].ID, P[k].neighbor, P[k].distance);
	}
#endif
	return (P);
}

GMT_LOCAL int compare_nn_info (const void *point_1v, const void *point_2v) {
	/*  Routine for qsort to sort NN rec numbers structure on original record order.
		*/
	const struct NN_INFO *point_1 = point_1v, *point_2 = point_2v;
	
	if (point_1->orig_rec < point_2->orig_rec) return (-1);
	if (point_1->orig_rec > point_2->orig_rec) return (+1);
	return (0);
}

GMT_LOCAL struct NN_INFO *NNA_update_info (struct GMT_CTRL *GMT, struct NN_INFO * I, struct NN_DIST *NN_dist, uint64_t n_points) {
	/* Return revised array of NN ID lookups via sorting on neighbor IDs */
	uint64_t k;
	struct NN_INFO *info = (I) ? I : gmt_M_memory (GMT, NULL, n_points, struct NN_INFO);
	for (k = 0; k < n_points; k++) {
		info[k].sort_rec = k;
		info[k].orig_rec = int64_abs (NN_dist[k].ID);
	}

	/* Prefer mergesort since qsort is not stable for equalities */
	mergesort (info, n_points, sizeof (struct NN_INFO), compare_nn_info);

	/* Now, I[k].sort_rec will take the original record # k and return the corresponding record in the sorted array */
	return (info);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
#ifdef PW_TESTING
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-A[a<min_dist>][unit]] [-C]\n\t[-D[+f<file>][+a<amax>][+d%s][+c|C<cmax>][+l][+s<sfact>][+p]]\n\t[-E+p|n] [-F[l]] [-I[i|e]] [-L%s/<pnoise>/<offset>] [-N<pfile>[+a][+p<ID>][+r][+z]]\n\t[-Q[<unit>][+c<min>[/<max>]][+h][+l][+p][+s[a|d]]]\n", name, GMT_DIST_OPT, GMT_DIST_OPT);
#else
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-A[a<min_dist>][unit]] [-C]\n\t[-D[+f<file>][+a<amax>][+d%s][+c|C<cmax>][+l][+s<sfact>][+p]]\n\t[-E+p|n] [-F[l]] [-I[i|e]] [-N<pfile>[+a][+p<ID>][+r][+z]]\n\t[-Q[<unit>][+c<min>[/<max>]][+h][+l][+p][+s[a|d]]]\n", name, GMT_DIST_OPT);
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-Sh|i|j|s|u] [-T[<cpol>]] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_j_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Nearest Neighbor (NN) Analysis. Compute minimum distances between NN point pairs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append unit used for NN distance calculation.  Returns minimum distances and point IDs for pairs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Aa to replace close neighbor pairs with their weighted average location until\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   no point pair has a NN distance less than the specified <min_dist> distance [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Considers 3rd column as z (if present) and 4th as w, if present [weight = 1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Clip polygons to the given region box (requires -R), possibly yielding new closed polygons.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For truncation instead (possibly yielding open polygons, i.e., lines), see -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Look for (near-)duplicates in <table>, or append +f to compare <table> against <file>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Near-duplicates have a minimum point separation less than <dmax> [0] and a closeness\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   ratio (mean separation/length) less than <cmax> [0.01].  Use +d and +c to change these.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +C to use median separation instead [+c uses mean separation].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If near-duplicates have lengths that differ by <sfact> or more then they are subsets or supersets [2].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To flag duplicate polygons, the fractional difference in areas must be less than <amax> [0.01].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default we consider all points when comparing two lines.  Use +p to limit\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the comparison to points that project perpendicularly on to the other line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Orient all polygons to have the same handedness.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p for counter-clockwise (positive) or +n for clockwise (negative) handedness.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force all input segments to become closed polygons on output by adding repeated point if needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Fl instead to ensure input lines are not treated as polygons.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Compute Intersection locations between input polygon(s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append e or i for external or internal crossings only [Default is both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use uppercase E or I to consider all segments within the same table as one entity [separate].\n");
#ifdef PW_TESTING
	GMT_Message (API, GMT_TIME_NONE, "\t-L Remove tile Lines.  These are superfluous lines along the -R border.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append <gap_dist> (in m) [0], coordinate noise [1e-10], and max offset from gridline [1e-10].\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-N Determine ID of polygon (in <pfile>) enclosing each input feature.  The ID is set as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a) If OGR/GMT polygons, get polygon ID via -a for Z column, else\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     b) Interpret segment labels (-Z<value>) as polygon IDs, else\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c) Interpret segment labels (-L<label>) as polygon IDs, else\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     d) Append +p<ID> to set origin for auto-incrementing polygon IDs [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Modifier +a means all points of a feature (line, polygon) must be inside the ID polygon [mid point].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Modifier +z means append the ID as a new output data column [Default adds -Z<ID> to segment header].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Modifier +r means no table output; just reports which polygon a feature is inside.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Measure area and handedness of polygon(s) or length of line segments.  If -fg is used\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   you may append unit %s [k]; otherwise it will be based on the input Cartesian data unit.\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   We also compute polygon centroid or line mid-point.  See documentation for more information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to limit output segments to those with area or length within specified range [output all segments].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     if <max> is not given then it defaults to infinity.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +h to place the (area, handedness) or length result in the segment header on output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to consider all input as polygons and close them if necessary\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [only closed polygons are considered polygons].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +l to consider all input as lines even if closed [closed polygons are considered polygons].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s to sort segments based on area or length; append descending [ascending].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default only reports results to stdout].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Spatial manipulation of polygons; choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h for detecting holes and reversing them relative to perimeters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     i for intersection [Not implemented yet].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     j for joining polygons that were split by the Dateline [Not implemented yet].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s for splitting polygons that straddle the Dateline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     u for union [Not implemented yet].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Truncate polygons against the clip polygon <cpol>; if <cpol> is not given we require -R\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and clip against a polygon derived from the region border.\n");
	GMT_Option (API, "V,bi2,bo,d,e,f,g,h,i,j,o,s,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTSPATIAL_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_files[2] = {0, 0}, pos, n_errors = 0;
	int n;
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, txt_c[GMT_LEN64] = {""}, p[GMT_LEN256] = {""}, *s = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (gmt_M_is_geographic (GMT, GMT_IN)) Ctrl->Q.unit = 'k';	/* Default geographic distance unit is km */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files[GMT_IN]++;
				break;
			case '>':	/* Got named output file */
				if (n_files[GMT_OUT]++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Do nearest neighbor analysis */
				Ctrl->A.active = true;
				if (opt->arg[0] == 'a' || opt->arg[0] == 'A') {	/* Spatially average points until minimum NN distance is less than given distance */
					Ctrl->A.mode = (opt->arg[0] == 'A') ? 2 : 1;	/* Slow mode is an undocumented test mode */
					Ctrl->A.smode = gmt_get_distance (GMT, &opt->arg[1], &(Ctrl->A.min_dist), &(Ctrl->A.unit));
				}
				else if (((opt->arg[0] == '-' || opt->arg[0] == '+') && strchr (GMT_LEN_UNITS, opt->arg[1])) || (opt->arg[0] && strchr (GMT_LEN_UNITS, opt->arg[0]))) {
					/* Just compute NN distances and return the unique ones */
					if (opt->arg[0] == '-')	/* Flat earth calculation */
						Ctrl->A.smode = 1;
					else if (opt->arg[0] == '+')	/* Geodetic */
						Ctrl->A.smode = 3;
					else
						Ctrl->A.smode = 2;	/* Great circle */
					Ctrl->A.unit = (Ctrl->A.smode == 2) ? opt->arg[0] : opt->arg[1];
					if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Data was not geographic, revert to Cartesian settings */
						Ctrl->A.smode = 0;
						Ctrl->A.unit = 'X';
					}
				}
				else if (opt->arg[0]) {
					GMT_Report (API, GMT_MSG_NORMAL, "Bad modifier in %c in -A option\n", opt->arg[0]);
					n_errors++;
				}
				break;
			case 'C':	/* Clip to given region */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Look for duplications */
				Ctrl->D.active = true;
				pos = 0;
				while (gmt_strtok (opt->arg, "+", &pos, p)) {
					switch (p[0]) {
						case 'a':	/* Gave a new +a<dmax> value */
							GMT_Report (API, GMT_MSG_NORMAL, "+a not implemented yet\n");
							Ctrl->D.I.a_threshold = atof (&p[1]);
							break;
						case 'd':	/* Gave a new +d<dmax> value */
							Ctrl->D.mode = gmt_get_distance (GMT, &p[1], &(Ctrl->D.I.d_threshold), &(Ctrl->D.unit));
							break;
						case 'C':	/* Gave a new +C<cmax> value */
							Ctrl->D.I.mode = 1;	/* Median instead of mean */
							/* Fall through on purpose */
						case 'c':	/* Gave a new +c<cmax> value */
							if (p[1]) Ctrl->D.I.c_threshold = atof (&p[1]);	/* This allows +C by itself just to change to median */
							break;
						case 's':	/* Gave a new +s<fact> value */
							Ctrl->D.I.s_threshold = atof (&p[1]);
							break;
						case 'p':	/* Consider only inside projections */
							Ctrl->D.I.inside = 1;
							break;
						case 'f':	/* Gave a file name */
							if (gmt_check_filearg (GMT, 'D', &p[1], GMT_IN, GMT_IS_DATASET))
								Ctrl->D.file = strdup (&p[1]);
							else
								n_errors++;
							break;
					}
				}
				break;
			case 'E':	/* Orient polygons -E+n|p  (old -E-|+) */
			 	Ctrl->E.active = true;
				if (opt->arg[0] == '-' || strstr (opt->arg, "+n"))
					Ctrl->E.mode = POL_IS_CW;
				else if (opt->arg[0] == '+' || strstr (opt->arg, "+p"))
					Ctrl->E.mode = POL_IS_CCW;
				else
					n_errors++;
				break;
			case 'F':	/* Force polygon or line mode */
				Ctrl->F.active = true;
				Ctrl->F.geometry = (opt->arg[0] == 'l') ? GMT_IS_LINE : GMT_IS_POLY;
				break;
			case 'I':	/* Compute intersections between polygons */
				Ctrl->I.active = true;
				if (opt->arg[0] == 'i') Ctrl->I.mode = 1;
				if (opt->arg[0] == 'I') Ctrl->I.mode = 2;
				if (opt->arg[0] == 'e') Ctrl->I.mode = 4;
				if (opt->arg[0] == 'E') Ctrl->I.mode = 8;
				break;
#ifdef PW_TESTING
			case 'L':	/* Remove tile lines */
				Ctrl->L.active = true;
				n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				if (n >= 1) Ctrl->L.s_cutoff = atof (txt_a);
				if (n >= 2) Ctrl->L.path_noise = atof (txt_b);
				if (n == 3) Ctrl->L.box_offset = atof (txt_c);
				break;
#endif
			case 'N':	/* Determine containing polygons for features */
				Ctrl->N.active = true;
				if ((s = strchr (opt->arg, '+')) == NULL) {	/* No modifiers */
					Ctrl->N.file = strdup (opt->arg);
					continue;
				}
				s[0] = '\0';	Ctrl->N.file = strdup (opt->arg);	s[0] = '+';
				pos = 0;
				while (gmt_strtok (s, "+", &pos, p)) {
					switch (p[0]) {
						case 'a':	/* All points must be inside polygon */
							Ctrl->N.all = true;
							break;
						case 'p':	/* Set start of running numbers [0] */
							Ctrl->N.ID = (p[1]) ? atoi (&p[1]) : 1;
							break;
						case 'r':	/* Just give a report */
							Ctrl->N.mode = 1;
							break;
						case 'z':	/* Gave a new +s<fact> value */
							Ctrl->N.mode = 2;
							break;
					}
				}
				break;
			case 'Q':	/* Measure area/length and handedness of polygons */
				Ctrl->Q.active = true;
				s = opt->arg;
				if (s[0] && strchr ("-+", s[0]) && strchr (GMT_LEN_UNITS, s[1])) {	/* Since [-|+] is deprecated as of GMT 6 */
					if (gmt_M_compat_check (GMT, 6))
						GMT_Report (API, GMT_MSG_COMPAT, "Leading -|+ with unit to set flat Earth or ellipsoidal mode is deprecated; use -j<mode> instead\n");
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Signed unit is not allowed - ignored\n");
						n_errors++;
					}
				}
				if (s[0] == '-' && strchr (GMT_LEN_UNITS, s[1])) {	/* Flat earth distances */
					Ctrl->Q.dmode = 1;	Ctrl->Q.unit = s[1];	s += 2;
				}
				else if (s[0] == '+' && strchr (GMT_LEN_UNITS, s[1])) {	/* Geodesic distances */
					Ctrl->Q.dmode = 3;	Ctrl->Q.unit = s[1];	s += 2;
				}
				else if (s[0] && strchr (GMT_LEN_UNITS, s[0])) {	/* Great circle distances */
					Ctrl->Q.dmode = 2;	Ctrl->Q.unit = s[0];	s++;
				}
				pos = 0;
				while (gmt_strtok (s, "+", &pos, p)) {
					switch (p[0]) {
						case 'c':	/* Set limits on output segments based on lengths or area */
							Ctrl->Q.area = true;
							n = sscanf (&p[1], "%[^/]/%s", txt_a, txt_b);
							Ctrl->Q.limit[0] = atof (txt_a);
							if (n == 1)	/* Just got the minimum cutoff */
								Ctrl->Q.limit[1] = DBL_MAX;
							else
								Ctrl->Q.limit[1] = atof (txt_b);
							break;
						case 'l':	/* Consider input as lines, even if closed */
							Ctrl->Q.mode = GMT_IS_LINE;
							break;
						case 'p':		/* Consider input as polygones, close if necessary */
							Ctrl->Q.mode = GMT_IS_POLY;
							break;
						case 'h':	/* Place result in output header */
							Ctrl->Q.header = true;
							break;
						case 's':	/* Sort segments on output based on dimension */
							Ctrl->Q.sort = true;
							switch (p[1]) {	/* Ascending or descending sort */
								case 'a':	case '\0':	Ctrl->Q.dir = +1;	break;	/* Ascending [Default] */
								case 'd':	Ctrl->Q.dir = -1;	break;	/* Descending */
								default:
									GMT_Report (API, GMT_MSG_NORMAL, "Error -Q: Unrecognized direction %c given to modifier +s\n", p[1]);
									n_errors++;
									break;
							}
							break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Error -Q: Unrecognized modifier +%c\n", p[0]);
							n_errors++;
							break;
					}
				}
				if (strstr (s, "++") || (s[0] && s[strlen(s)-1] == '+')) {	/* Deal with the old-style single "+" to mean header */
					Ctrl->Q.header = true;
					GMT_Report (API, GMT_MSG_NORMAL, "Warning:-Q+ is interpreted as -Q+h\n");
				}
				break;
			case 'S':	/* Spatial polygon operations */
				Ctrl->S.active = true;
				if (opt->arg[0] == 'u') {
					Ctrl->S.mode = POL_UNION;
					GMT_Report (API, GMT_MSG_NORMAL, "Su not implemented yet\n");
				}
				else if (opt->arg[0] == 'i') {
					Ctrl->S.mode = POL_INTERSECTION;
					GMT_Report (API, GMT_MSG_NORMAL, "Si not implemented yet\n");
				}
				else if (opt->arg[0] == 's')
					Ctrl->S.mode = POL_SPLIT;
				else if (opt->arg[0] == 'h')
					Ctrl->S.mode = POL_HOLE;
				else if (opt->arg[0] == 'j') {
					Ctrl->S.mode = POL_JOIN;
					GMT_Report (API, GMT_MSG_NORMAL, "Sj not implemented yet\n");
				}
				else
					n_errors++;
				break;
			case 'T':	/* Truncate against polygon */
				Ctrl->T.active = true;
				Ctrl->C.active = Ctrl->S.active = true;
				Ctrl->S.mode = POL_CLIP;
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

 	if (Ctrl->E.active) Ctrl->Q.active = true;
	
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least %d columns\n", 2);
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == POL_CLIP && !Ctrl->T.file && !GMT->common.R.active[RSET], "Syntax error: -T without a polygon requires -R\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->T.active && !GMT->common.R.active[RSET], "Syntax error: -C requires -R\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && !GMT->common.R.active[RSET], "Syntax error: -L requires -R\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && Ctrl->L.s_cutoff < 0.0, "Syntax error: -L requires a positive cutoff in meters\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->D.file && gmt_access (GMT, Ctrl->D.file, R_OK), "Syntax error -D: Cannot read file %s!\n", Ctrl->D.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.file && gmt_access (GMT, Ctrl->T.file, R_OK), "Syntax error -T: Cannot read file %s!\n", Ctrl->T.file);
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtspatial (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int geometry = GMT_IS_POLY, internal = 0, external = 0, smode = GMT_NO_STRINGS;
	bool mseg = false;

	static char *kind[2] = {"CCW", "CW"};

	double out[GMT_MAX_COLUMNS];

	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_RECORD Out;
	struct GMTSPATIAL_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	/*---------------------------- This is the gmtspatial main code ----------------------------*/

	if (Ctrl->I.active) {
		switch (Ctrl->I.mode) {
			case 0:
				internal = external = 1;
				break;
			case 1:
				internal = 1;
				break;
			case 2:
				internal = 2;
				break;
			case 4:
				external = 1;
				break;
			case 8:
				external = 2;
				break;
		}
	}
			
	/* Read input data set */
	
	if (Ctrl->D.active) geometry = GMT_IS_LINE|GMT_IS_POLY;	/* May be lines, may be polygons... */
	else if (Ctrl->Q.active) geometry = Ctrl->Q.mode;	/* May be lines, may be polygons... */
	else if (Ctrl->A.active) geometry = GMT_IS_POINT;	/* NN analysis involves points */
	else if (Ctrl->F.active) geometry = Ctrl->F.geometry;	/* Forcing polygon or line mode */
	else if (Ctrl->S.active) geometry = GMT_IS_POLY;	/* Forcing polygon mode */
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (D->n_columns < 2) {
		GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)D->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}
	
	/* Allocate memory and read in all the files; each file can have many lines (-m) */
	
	if (D->n_records == 0) {	/* Empty files, nothing to do */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data records found.\n");
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	if (Ctrl->S.active && !(Ctrl->S.mode == POL_SPLIT || Ctrl->S.mode == POL_HOLE)) external = 1;
	
	gmt_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);	/* Use Cartesian calculations and user units */

	gmt_set_inside_mode (GMT, NULL, (gmt_M_is_geographic (GMT, GMT_IN)) ? GMT_IOO_SPHERICAL : GMT_IOO_CARTESIAN);
	
	/* OK, with data in hand we can do some damage */
	
	if (Ctrl->A.active) {	/* Nearest neighbor analysis. We compute distances between all point pairs and sort on minimum distance */
		uint64_t n_points, k, a, b, n, col, n_pairs;
		double A[3], B[3], w, iw, d_bar, out[7];
		struct NN_DIST *NN_dist = NULL;
		struct NN_INFO  *NN_info = NULL;
		
		gmt_init_distaz (GMT, Ctrl->A.unit, Ctrl->A.smode, GMT_MAP_DIST);	/* Set the unit and distance calculation we requested */
		
		NN_dist = NNA_init_dist (GMT, D, &n_points);		/* Return array of NN results sorted on smallest distances */		
		NN_info = NNA_update_info (GMT, NN_info, NN_dist, n_points);	/* Return array of NN ID record look-ups */
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {	/* All data now in NN_dist so free original dataset */
			gmt_M_free (GMT, NN_dist);	 gmt_M_free (GMT, NN_info);	
			Return (API->error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			gmt_M_free (GMT, NN_dist);	 gmt_M_free (GMT, NN_info);	
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			gmt_M_free (GMT, NN_dist);	 gmt_M_free (GMT, NN_info);	
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			gmt_M_free (GMT, NN_dist);	 gmt_M_free (GMT, NN_info);	
			Return (API->error);
		}
		Out.data = out;	Out.text = NULL;
		if (Ctrl->A.mode) {	/* Need to combine close neighbors until minimum distance >= min_dist, then output revised dataset */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "NNA using min separation of %g%c\n", Ctrl->A.min_dist, Ctrl->A.unit);
			n = 0;
			while (n < n_points && NN_dist[n].distance < Ctrl->A.min_dist) n++;	/* Find # of pairs that are too close together */
			while (n) {	/* Must do more combining since n pairs exceed threshold distance */
				if (Ctrl->A.mode == 2) {
					GMT_Report (API, GMT_MSG_VERBOSE, "Slow mode: Replace the single closest pair with its weighted average, then redo NNA\n");
					n = 1;
				}
				for (k = n_pairs = 0; k < n; k++) {	/* Loop over pairs that are too close */
					if (gmt_M_is_dnan (NN_dist[k].distance)) continue;	/* Already processed */
					a = k;	/* The current point */
					b = NN_info[int64_abs(NN_dist[a].neighbor)].sort_rec;	/* a's neighbor location in the sorted NN_dist array */
					GMT_Report (API, GMT_MSG_DEBUG, "Replace pair %" PRIu64 " and %" PRIu64 " with its weighted average location\n", a, b);
					w = NN_dist[a].data[GMT_W] + NN_dist[b].data[GMT_W];	/* Weight sum */
					iw = 1.0 / w;	/* Inverse weight for scaling */
					/* Compute weighted average z */
					NN_dist[a].data[GMT_Z] = iw * (NN_dist[a].data[GMT_Z] * NN_dist[a].data[GMT_W] + NN_dist[b].data[GMT_Z] * NN_dist[b].data[GMT_W]);
					if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must do vector averaging */
						gmt_geo_to_cart (GMT, NN_dist[a].data[GMT_Y], NN_dist[a].data[GMT_X], A, true);
						gmt_geo_to_cart (GMT, NN_dist[b].data[GMT_Y], NN_dist[b].data[GMT_X], B, true);
						for (col = 0; col < 3; col++) {	/* Get weighted average vector */
							A[col] = iw * (A[col] * NN_dist[a].data[GMT_W] + B[col] * NN_dist[b].data[GMT_W]);
						}
						gmt_normalize3v (GMT, A);	/* Get unit vector */
						gmt_cart_to_geo (GMT, &NN_dist[a].data[GMT_Y], &NN_dist[a].data[GMT_X], A, true);	/* Get lon, lat of the average point */
					}
					else {	/* Cartesian weighted averaging */
						NN_dist[a].data[GMT_X] = iw * (NN_dist[a].data[GMT_X] * NN_dist[a].data[GMT_W] + NN_dist[b].data[GMT_X] * NN_dist[b].data[GMT_W]);
						NN_dist[a].data[GMT_Y] = iw * (NN_dist[a].data[GMT_Y] * NN_dist[a].data[GMT_W] + NN_dist[b].data[GMT_Y] * NN_dist[b].data[GMT_W]);
					}
					NN_dist[a].data[GMT_W] = 0.5 * w;	/* Replace with the average weight */
					NN_dist[a].ID = -int64_abs (NN_dist[a].ID);	/* Negative means it was averaged with other points */
					NN_dist[b].distance = GMT->session.d_NaN;	/* Flag this point as used.  NNA_update_dist will sort it and place all NaNs at the end */
					n_pairs++;
				}
				GMT_Report (API, GMT_MSG_VERBOSE, "NNA Found %" PRIu64 " points, %" PRIu64 " pairs were too close and were replaced by their weighted average\n", n_points, n_pairs);
				NN_dist = NNA_update_dist (GMT, NN_dist, &n_points);		/* Return recomputed array of NN NN_dist sorted on smallest distances */
				NN_info = NNA_update_info (GMT, NN_info, NN_dist, n_points);	/* Return resorted array of NN ID lookups */
				n = 0;
				while (n < n_points && NN_dist[n].distance < Ctrl->A.min_dist) n++;	/* Any more pairs with distances less than the threshold? */
			}
			if ((error = GMT_Set_Columns (API, GMT_OUT, 7, GMT_COL_FIX_NO_TEXT)) != 0) {
				gmt_M_free (GMT, NN_dist);	gmt_M_free (GMT, NN_info);	
				Return (error);
			}
			if (GMT->common.h.add_colnames) {
				char header[GMT_LEN256] = {""}, *name[2][2] = {{"x", "y"}, {"lon", "lat"}};
				k = (gmt_M_is_geographic (GMT, GMT_IN)) ? 1 : 0;
				sprintf (header, "%s[0]\t%s[1]\tz[2]\tweight[3]\tNN_dist[4]\tID[5]\tNN_ID[6]", name[k][GMT_X], name[k][GMT_Y]);
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);	
			}
		}
		else {	/* Just output the NN analysis results */
			gmt_set_cartesian (GMT, GMT_OUT);	/* Since we are not writing coordinates */
			if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != 0) {
				gmt_M_free (GMT, NN_dist);	gmt_M_free (GMT, NN_info);	
				Return (error);
			}
			if (GMT->common.h.add_colnames) {
				char header[GMT_LEN64];
				sprintf (header, "NN_dist[0]\tID[1]\tNN_ID[2]");
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);	
			}
		}
		d_bar = 0.0;
		for (k = 0; k < n_points; k++) {
			d_bar += NN_dist[k].distance;
			if (Ctrl->A.mode) {	/* Want all points out, including their coordinates */
				gmt_M_memcpy (out, NN_dist[k].data, 4, double);
				col = 4;
			}
			else {	/* Just get the unique NN distances and the points involved */
				if (k && NN_dist[k].ID == NN_dist[k-1].neighbor && NN_dist[k].neighbor == NN_dist[k-1].ID) continue;	/* Skip duplicate pairs */
				col = 0;
			}
			out[col++] = NN_dist[k].distance;
			if (NN_dist[k].ID < NN_dist[k].neighbor) {
				out[col++] = (double)NN_dist[k].ID;
				out[col++] = (double)NN_dist[k].neighbor;
			}
			else {
				out[col++] = (double)NN_dist[k].neighbor;
				out[col++] = (double)NN_dist[k].ID;
			}
			GMT_Put_Record (API, GMT_WRITE_DATA, &Out);	/* Write points of NN info to stdout */
		}
		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			d_bar /= n_points;
			if (GMT->common.R.active[RSET]) {
				int geo = gmt_M_is_geographic (GMT, GMT_IN) ? 1 : 0;
				double info[3], d_expect, R_index;
				struct GMT_DATASEGMENT *S = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, 5, 2, NULL, NULL);
				S->data[GMT_X][0] = S->data[GMT_X][3] = S->data[GMT_X][4] = GMT->common.R.wesn[XLO];
				S->data[GMT_X][1] = S->data[GMT_X][2] = GMT->common.R.wesn[XHI];
				S->data[GMT_Y][0] = S->data[GMT_Y][1] = S->data[GMT_Y][4] = GMT->common.R.wesn[YLO];
				S->data[GMT_Y][2] = S->data[GMT_Y][3] = GMT->common.R.wesn[YHI];
				(void)area_size (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, info, geo);
				gmt_free_segment (GMT, &S);
				d_expect = 0.5 * sqrt (info[GMT_Z]/n_points);
				R_index = d_bar / d_expect;
				GMT_Report (API, GMT_MSG_VERBOSE, "NNA Found %" PRIu64 " points, D_bar = %g, D_expect = %g, Spatial index = %g\n", n_points, d_bar, d_expect, R_index);
			}
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "NNA Found %" PRIu64 " points, D_bar = %g\n", n_points, d_bar);
			
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			gmt_M_free (GMT, NN_dist);	
			gmt_M_free (GMT, NN_info);	
			Return (API->error);
		}
		gmt_M_free (GMT, NN_dist);	
		gmt_M_free (GMT, NN_info);	
		Return (GMT_NOERROR);
	}
	if (Ctrl->L.active) {	/* Remove tile lines only */
		int gap = 0, first, prev_OK;
		uint64_t row, seg, tbl;
		double dx, dy, DX, DY, dist;

		gmt_init_distaz (GMT, GMT_MAP_DIST_UNIT, 2, GMT_MAP_DIST);	/* Default is m using great-circle distances */
		
		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, geometry) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S = D->table[tbl]->segment[seg];
				if (S->n_rows == 0) continue;
				for (row = 1, first = true, prev_OK = false; row < S->n_rows; row++) {
					dx = S->data[GMT_X][row] - S->data[GMT_X][row-1];
					dy = S->data[GMT_Y][row] - S->data[GMT_Y][row-1];
					DX = MIN (fabs (S->data[GMT_X][row] - GMT->common.R.wesn[XLO]), fabs (S->data[GMT_X][row] - GMT->common.R.wesn[XHI]));
					DY = MIN (fabs (S->data[GMT_Y][row] - GMT->common.R.wesn[YLO]), fabs (S->data[GMT_Y][row] - GMT->common.R.wesn[YHI]));
					gap = false;
					if ((fabs (dx) < Ctrl->L.path_noise && DX < Ctrl->L.box_offset) || (fabs (dy) < Ctrl->L.path_noise && DY < Ctrl->L.box_offset)) {	/* Going along a tile boundary */
						dist = gmt_distance (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], S->data[GMT_X][row-1], S->data[GMT_Y][row-1]);
						if (dist > Ctrl->L.s_cutoff) gap = true;
					}
					if (gap) {	/* Distance exceed threshold, start new segment */
						first = true;
						if (prev_OK) write_record (GMT, S->data, S->n_columns, row-1);
						prev_OK = false;
					}
					else {
						if (first && S->header) {
							strncpy (GMT->current.io.segment_header, S->header, GMT_BUFSIZ-1);
							GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
						}
						write_record (GMT, S->data, S->n_columns, row-1);
						first = false;
						prev_OK = true;
					}
				}
				if (!gap) write_record (GMT, S->data, S->n_columns, row-1);
			}
					
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	if (Ctrl->Q.active) {	/* Calculate centroid and polygon areas or line lengths and place in segment headers */
		double out[3];
		static char *type[2] = {"length", "area"}, upper[GMT_LEN32] = {"infinity"};
		bool new_data = (Ctrl->Q.header || Ctrl->Q.sort || Ctrl->E.active);
		uint64_t seg, row_f, row_l, tbl, col, n_seg = 0, n_alloc_seg = 0;
		unsigned int handedness = 0;
		int qmode, poly = 0, geo;
		struct GMT_DATASET *Dout = NULL;
		struct GMT_DATASEGMENT *Sout = NULL;
		struct GMT_ORDER *Q = NULL;

		char line[GMT_LEN128] = {""};
		
		if (gmt_M_is_cartesian (GMT, GMT_IN) && Ctrl->Q.unit && strchr (GMT_LEN_UNITS, Ctrl->Q.unit)) {
			gmt_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg if -Q uses unit */
		}
		geo = gmt_M_is_geographic (GMT, GMT_IN);
		if (geo) gmt_init_distaz (GMT, Ctrl->Q.unit, Ctrl->Q.dmode, GMT_MAP_DIST);	/* Default is m using great-circle distances */

		if (Ctrl->Q.header) {	/* Add line length or polygon area stuff to segment header */
			qmode = Ctrl->Q.mode;	/* Don't know if line or polygon but passing GMT_IS_POLY would close any open polygon, which we want with +p */
			GMT->current.io.multi_segments[GMT_OUT] = true;	/* To ensure we can write headers */
		}
		else {
			qmode = GMT_IS_POINT;
			if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != 0) Return (error);
		}
		if (new_data) {	/* Must create an output dataset */
			enum GMT_enum_geometry gmtry;
			uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 0};	/* One table, no rows yet to avoid allocations */
			dim[GMT_COL] = D->n_columns;	/* Same number of columns as the input */
			switch (Ctrl->Q.mode) {	/* Set geometry */
				case GMT_IS_LINE:	gmtry = GMT_IS_LINE;	break;
				case GMT_IS_POLY:	gmtry = GMT_IS_POLY;	break;
				default:		gmtry = D->geometry;	break;
			}
			if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, gmtry, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				Return (API->error);
			if (Ctrl->Q.area && Ctrl->Q.limit[1] < DBL_MAX) sprintf (upper, "%.12g", Ctrl->Q.limit[1]);
			if (Ctrl->Q.sort) Q = gmt_M_memory (GMT, NULL, D->n_segments, struct GMT_ORDER);
		}
		else {	/* Just write results as one line per segment to stdout */
			if (GMT_Init_IO (API, GMT_IS_DATASET, qmode, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
				Return (API->error);
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
				Return (API->error);
			}
		}
		Out.data = out;	Out.text = NULL;
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S = D->table[tbl]->segment[seg];
				if (S->n_rows == 0) continue;
				switch (Ctrl->Q.mode) {	/* Set or determine line vs polygon type */
					case GMT_IS_LINE:	poly = 0;	break;
					case GMT_IS_POLY:	poly = 1;	break;
					default:
						poly = !gmt_polygon_is_open (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows);	/* Line or polygon */
						break;
				}
				if (poly)	/* Polygon */
					handedness = area_size (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, out, geo);
				else	/* Line */
					length_size (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, out);
				/* Must determine if this segment passes our dimension test */
				if (Ctrl->Q.area && (out[GMT_Z] < Ctrl->Q.limit[0] || out[GMT_Z] > Ctrl->Q.limit[1])) {
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input segment %s %g is outside the chosen range %g to %s\n", type[poly], out[GMT_Z], Ctrl->Q.limit[0], upper);
					continue;
				}
				if (Ctrl->Q.header) {	/* Add the information to the segment header */
					if (S->header) {
						if (poly)
							snprintf (line, GMT_LEN128, "%s -Z%.12g (area) %.12g/%.12g (centroid) %s", S->header, out[GMT_Z], out[GMT_X], out[GMT_Y], kind[handedness]);
						else
							snprintf (line, GMT_LEN128, "%s -Z%.12g (length) %.12g/%.12g (midpoint)", S->header, out[GMT_Z], out[GMT_X], out[GMT_Y]);
					}
					else {
						if (poly)
							snprintf (line, GMT_LEN128, "-Z%.12g (area) %.12g/%.12g (centroid) %s", out[GMT_Z], out[GMT_X], out[GMT_Y], kind[handedness]);
						else
							snprintf (line, GMT_LEN128, "-Z%.12g (length) %.12g/%.12g (midpoint)", out[GMT_Z], out[GMT_X], out[GMT_Y]);
					}
				}
				if (new_data) {	/* Must create the output segment and duplicate */
					if (n_alloc_seg <= n_seg) {
						n_alloc_seg = (n_seg == 0) ? D->n_segments : (n_alloc_seg < 1);
						Dout->table[0]->segment = gmt_M_memory (GMT, Dout->table[0]->segment, n_alloc_seg, struct GMT_DATASEGMENT *);
					}
					smode = (S->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
					Sout = GMT_Alloc_Segment (API, smode, S->n_rows, S->n_columns, line, NULL);
					for (col = 0; col < S->n_columns; col++) gmt_M_memcpy (Sout->data[col], S->data[col], S->n_rows, double);
					Dout->table[0]->segment[n_seg] = Sout;
					if (Ctrl->Q.sort) Q[n_seg].value = out[GMT_Z], Q[n_seg].order = n_seg;
					n_seg++;
				}
				if (poly && Ctrl->E.active && handedness != Ctrl->E.mode) {	/* Must reverse line */
					for (row_f = 0, row_l = Sout->n_rows - 1; row_f < Sout->n_rows/2; row_f++, row_l--) {
						for (col = 0; col < Sout->n_columns; col++) gmt_M_double_swap (Sout->data[col][row_f], Sout->data[col][row_l]);
					}
					handedness = Ctrl->E.mode;
				}
				if (!new_data)	/* Just write centroid and area|length to output */
					GMT_Put_Record (API, GMT_WRITE_DATA, &Out);
			}
		}
		if (new_data) {		/* Must write out a revised dataset */
			Dout->n_segments = Dout->table[0]->n_segments = n_seg;
			Dout->table[0]->segment = gmt_M_memory (GMT, Dout->table[0]->segment, n_seg, struct GMT_DATASEGMENT *);
			if (Ctrl->Q.sort) {	/* Sort on area or length and shuffle the order of segments before output */
				struct GMT_DATASEGMENT **tmp = gmt_M_memory (GMT, NULL, n_seg, struct GMT_DATASEGMENT *);
				gmt_sort_order (GMT, Q, n_seg, Ctrl->Q.dir);
				for (seg = 0; seg < n_seg; seg++) tmp[seg] = Dout->table[0]->segment[Q[seg].order];
				gmt_M_memcpy (Dout->table[0]->segment, tmp, n_seg, struct GMT_DATASEGMENT *);
				gmt_M_free (GMT, tmp);
				gmt_M_free (GMT, Q);
			}
			if (Dout->n_segments > 1) gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on "-mo" */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
				Return (API->error);
			}
			n_seg = D->n_segments - Dout->n_segments;	/* Lost segments */
			if (n_seg) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " segments were outside and %" PRIu64 " were inside the chosen %s range of %g to %s\n",
				n_seg, Dout->n_segments, type[poly], Ctrl->Q.limit[0], upper);
		}
		if (!new_data && GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	if (Ctrl->I.active || external) {	/* Crossovers between polygons */
		bool same_feature;
		unsigned int in, wtype, n_columns;
		uint64_t tbl1, tbl2, col, nx, row, seg1, seg2;
		struct GMT_XSEGMENT *ylist1 = NULL, *ylist2 = NULL;
		struct GMT_XOVER XC;
		char record[GMT_BUFSIZ] = {""}, fmt[GMT_BUFSIZ] = {""};
		struct GMT_DATASET *C = NULL;
		struct GMT_DATASEGMENT *S1 = NULL, *S2 = NULL;
		
		if (Ctrl->S.mode == POL_CLIP) {	/* Need to set up a separate table with the clip polygon */
			if (Ctrl->T.file) {
				gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */
				if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
					Return (API->error);
				}
				if (C->n_columns < 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)C->n_columns);
					Return (GMT_DIM_TOO_SMALL);
				}
				gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
			}
			else {	/* Design a table based on -Rw/e/s/n */
				uint64_t dim[GMT_DIM_SIZE] = {1, 1, 5, 2};
				if ((C = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
				S1 = C->table[0]->segment[0];
				S1->data[GMT_X][0] = S1->data[GMT_X][3] = S1->data[GMT_X][4] = GMT->common.R.wesn[XLO];
				S1->data[GMT_X][1] = S1->data[GMT_X][2] = GMT->common.R.wesn[XHI];
				S1->data[GMT_Y][0] = S1->data[GMT_Y][1] = S1->data[GMT_Y][4] = GMT->common.R.wesn[YLO];
				S1->data[GMT_Y][2] = S1->data[GMT_Y][3] = GMT->common.R.wesn[YHI];
				S1->n_rows = 5;
			}
		}
		else
			C = D;	/* Compare with itself */

		geometry = (Ctrl->S.active) ? GMT_IS_PLP : GMT_IS_NONE;
		n_columns = (Ctrl->S.active) ? (unsigned int)C->n_columns : 4;
		wtype = (Ctrl->S.active) ? GMT_COL_FIX_NO_TEXT : GMT_COL_FIX;
		if ((error = GMT_Set_Columns (API, GMT_OUT, n_columns, wtype)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, geometry) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}

		sprintf (fmt, "%s%s%s%s%s%s%s%s%%s%s%%s\n", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, \
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, \
			GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);
		Out.data = out;	Out.text = NULL;
		for (tbl1 = 0; tbl1 < C->n_tables; tbl1++) {
			for (seg1 = 0; seg1 < C->table[tbl1]->n_segments; seg1++) {
				S1 = C->table[tbl1]->segment[seg1];
				if (S1->n_rows == 0) continue;
				gmt_init_track (GMT, S1->data[GMT_Y], S1->n_rows, &ylist1);
				for (tbl2 = (Ctrl->S.mode == POL_CLIP) ? 0 : tbl1; tbl2 < D->n_tables; tbl2++) {
					for (seg2 = 0; seg2 < D->table[tbl2]->n_segments; seg2++) {
						S2 = D->table[tbl2]->segment[seg2];
						if (S2->n_rows == 0) continue;
						if (Ctrl->S.mode != POL_CLIP) {	/* So there is only one dataset being compared with itself */
							same_feature = (external == 2 || internal == 2) ? (tbl1 == tbl2) : (tbl1 == tbl2 && seg1 == seg2);	/* What constitutes the same feature */
							if (!internal && same_feature) continue;	/* Do not do internal crossings */
							if (!external && !same_feature) continue;	/* Do not do external crossings */
						}
						gmt_init_track (GMT, S2->data[GMT_Y], S2->n_rows, &ylist2);
						nx = gmt_crossover (GMT, S1->data[GMT_X], S1->data[GMT_Y], NULL, ylist1, S1->n_rows, S2->data[GMT_X], S2->data[GMT_Y], NULL, ylist2, S2->n_rows, false, gmt_M_is_geographic (GMT, GMT_IN), &XC);
						if (nx) {	/* Polygon pair generated crossings */
							uint64_t px;
							if (Ctrl->S.active) {	/* Do the spatial clip operation */
								uint64_t row0;
								bool go, first;
								double *xx = NULL, *yy = NULL, *kk = NULL;
								struct PAIR *pair = NULL;
								
								pair = gmt_M_memory (GMT, NULL, nx, struct PAIR);
								xx = gmt_M_memory (GMT, NULL, nx, double);
								yy = gmt_M_memory (GMT, NULL, nx, double);
								kk = gmt_M_memory (GMT, NULL, nx, double);
								for (px = 0; px < nx; px++) pair[px].node = XC.xnode[1][px], pair[px].pos = px;
								qsort (pair, nx, sizeof (struct PAIR), comp_pairs);
								for (px = 0; px < nx; px++) {
									xx[px] = XC.x[pair[px].pos];
									yy[px] = XC.y[pair[px].pos];
									kk[px] = XC.xnode[1][pair[px].pos];
								}
								gmt_M_free (GMT, pair);
								in = gmt_non_zero_winding (GMT, S2->data[GMT_X][0], S2->data[GMT_Y][0], S1->data[GMT_X], S1->data[GMT_Y], S1->n_rows);
								go = first = true;
								row0 = px = 0;
								while (go) {
									for (row = row0; row < S2->n_rows && row < kk[px]; row++) {
										if (!in) continue;
										for (col = 0; col < S2->n_columns; col++) out[col] = S2->data[col][row];
										if (first && GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
											if (S2->header)
												strncpy (GMT->current.io.segment_header, S2->header, GMT_BUFSIZ-1);
											else
												sprintf (GMT->current.io.segment_header, "New segment");
											GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
											first = false;
										}
										GMT_Put_Record (API, GMT_WRITE_DATA, &Out);	/* Write this to output */
									}
									/* Always output crossover point */
									if (first && GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
										if (S2->header)
											strncpy (GMT->current.io.segment_header, S2->header, GMT_BUFSIZ-1);
										else
											sprintf (GMT->current.io.segment_header, "New segment");
										GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
										first = false;
									}
									for (col = 2; col < S2->n_columns; col++) out[col] = 0.0;
									out[GMT_X] = xx[px];	out[GMT_Y] = yy[px];
									GMT_Put_Record (API, GMT_WRITE_DATA, &Out);	/* Write this to output */
									px++;
									in = !in;	/* Go from out to in or vice versa */
									if (!in) first = true;	/* Since we went outside */
									row0 = row;
									if (px == nx) {
										for (row = row0; row < S2->n_rows; row++) {
											if (!in) continue;
											for (col = 0; col < S2->n_columns; col++) out[col] = S2->data[col][row];
											if (first && GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
												if (S2->header)
													strncpy (GMT->current.io.segment_header, S2->header, GMT_BUFSIZ-1);
												else
													sprintf (GMT->current.io.segment_header, "New segment");
												GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
												first = false;
											}
											GMT_Put_Record (API, GMT_WRITE_DATA, &Out);	/* Write this to output */
										}
										go = false;
									}
								}
								gmt_M_free (GMT, xx);
								gmt_M_free (GMT, yy);
								gmt_M_free (GMT, kk);
							}
							else {	/* Just report */
								struct GMT_DATATABLE_HIDDEN *TH1 = gmt_get_DT_hidden (C->table[tbl1]), *TH2 = gmt_get_DT_hidden (C->table[tbl2]);
								if (mseg)
									sprintf (record, "%s-%" PRIu64 "%s%s-%" PRIu64, TH1->file[GMT_IN], seg1, GMT->current.setting.io_col_separator, TH2->file[GMT_IN], seg2);
								else
									sprintf (record, "%s%s%s", TH1->file[GMT_IN], GMT->current.setting.io_col_separator, TH2->file[GMT_IN]);
								Out.text = record;
								for (px = 0; px < nx; px++) {	/* Write these to output */
									out[GMT_X] = XC.x[px];  out[GMT_Y] = XC.y[px];
									out[2] = XC.xnode[0][px];   out[3] = XC.xnode[1][px];
									GMT_Put_Record (API, GMT_WRITE_DATA, &Out);
								}
							}
							gmt_x_free (GMT, &XC);
						}
						else if (Ctrl->S.mode == POL_CLIP) {	/* No crossings; see if it is inside or outside C */
							if ((in = gmt_non_zero_winding (GMT, S2->data[GMT_X][0], S2->data[GMT_Y][0], S1->data[GMT_X], S1->data[GMT_Y], S1->n_rows)) != 0) {
								/* Inside, copy out the entire polygon */
								if (GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
									if (S2->header)
										strncpy (GMT->current.io.segment_header, S2->header, GMT_BUFSIZ-1);
									else
										sprintf (GMT->current.io.segment_header, "New segment");
									GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
								}
								for (row = 0; row < S2->n_rows; row++) {
									for (col = 0; col < S2->n_columns; col++) out[col] = S2->data[col][row];
									GMT_Put_Record (API, GMT_WRITE_DATA, &Out);	/* Write this to output */
								}
							}
						}
						gmt_M_free (GMT, ylist2);
						if (Ctrl->S.mode == POL_UNION) {
							GMT_Report (API, GMT_MSG_NORMAL, "Computing polygon union not implemented yet\n");
						}
						if (Ctrl->S.mode == POL_INTERSECTION) {
							GMT_Report (API, GMT_MSG_NORMAL, "Computing polygon intersection not implemented yet\n");
						}
					}
				}
				gmt_M_free (GMT, ylist1);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		if (Ctrl->S.mode == POL_CLIP) {
			if (GMT_Destroy_Data (API, &C) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		Return (GMT_NOERROR);
	}
	
	if (Ctrl->D.active) {	/* Look for duplicates of lines or polygons */
		unsigned int n_dup, poly_D, poly_S2;
		uint64_t tbl, tbl2, col, seg, seg2;
		bool same_feature = false;
		char *kind[10] = {"approximate-reversed-superset", "approximate-reversed-subset", "approximate-reversed", "exact-reversed" , "", "exact", "approximate", "approximate-subset", "approximate-superset", "Dateline-split"};
		char record[GMT_BUFSIZ] = {""}, format[GMT_BUFSIZ] = {""}, src[GMT_BUFSIZ] = {""}, dup[GMT_BUFSIZ] = {""}, *feature[2] = {"polygon", "line"}, *from = NULL;
		char *in = "the same data set", *verdict = "NY~-+/";	/* No, Yes, Approximate, Subsection, Supersection */
		struct GMT_DATASET *C = NULL;
		struct GMT_DATASEGMENT *S1 = NULL, *S2 = NULL;
		struct DUP_INFO **Info = NULL, *I = NULL;
		
		if (Ctrl->D.file) {	/* Get trial features via a file */
			gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -D files */
			if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE|GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->D.file, NULL)) == NULL) {
				Return (API->error);
			}
			if (C->n_columns < 2) {
				GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)C->n_columns);
				Return (GMT_DIM_TOO_SMALL);
			}
			gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
			from = Ctrl->D.file;
		}
		else {
			C = D;	/* Compare with itself */
			same_feature = true;
			from = in;
			smode = (C->table[0]->segment[0]->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
			S2 = GMT_Alloc_Segment (GMT->parent, smode, 0, C->n_columns, NULL, NULL);
		}
		if (GMT_Init_IO (API, C->geometry, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
			gmt_free_segment (GMT, &S2);
			Return (API->error);	/* Registers default output destination, unless already set */
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {
			gmt_free_segment (GMT, &S2);
			Return (API->error);				/* Enables data output and sets access mode */
		}
		if (GMT_Set_Geometry (API, GMT_OUT, C->geometry) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}

		Info = gmt_M_memory (GMT, NULL, C->n_tables, struct DUP_INFO *);
		for (tbl = 0; tbl < C->n_tables; tbl++) Info[tbl] = gmt_M_memory (GMT, NULL, C->table[tbl]->n_segments, struct DUP_INFO);

		gmt_init_distaz (GMT, Ctrl->D.unit, Ctrl->D.mode, GMT_MAP_DIST);

		sprintf (format, "%%c : Input %%s %%s is an %%s duplicate of a %%s %%s in %%s, with d = %s c = %%.6g s = %%.4g",
		         GMT->current.setting.format_float_out);
		
		Out.text = record;
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S1 = D->table[tbl]->segment[seg];
				if (S1->n_rows == 0) continue;

				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Check if segment %" PRIu64 " from Table %d has duplicates:\n", seg, tbl);
				if (same_feature) {	/* We must exclude this segment from the comparison otherwise we end up finding itself as a duplicate */
					S2->n_rows = S1->n_rows;
					for (col = 0; col < S1->n_columns; col++) S2->data[col] = S1->data[col];
					S1->n_rows = 0;	/* This means it will be skipped by is_duplicate */
				}
				else
					S2 = S1;
				poly_S2 = (gmt_polygon_is_open (GMT, S2->data[GMT_X], S2->data[GMT_Y], S2->n_rows)) ? 1 : 0;
				for (tbl2 = 0; tbl2 < C->n_tables; tbl2++) gmt_M_memset (Info[tbl2], C->table[tbl2]->n_segments, struct DUP_INFO);
				n_dup = is_duplicate (GMT, S2, C, &(Ctrl->D.I), Info);	/* Returns -3, -2, -1, 0, +1, +2, or +3 */
				if (same_feature) {
					S1->n_rows = S2->n_rows;	/* Reset the count */
					if (Ctrl->D.I.table < tbl || (Ctrl->D.I.table == tbl && Ctrl->D.I.segment < seg)) n_dup = 0;	/* To avoid reporting the same pair twice */
				}
				if (n_dup == 0) {	/* No duplicate found for this segment */
					if (!same_feature) {
						(D->n_tables == 1) ? sprintf (src, "[ segment %" PRIu64 " ]", seg)  : sprintf (src, "[ table %" PRIu64 " segment %" PRIu64 " ]", tbl, seg);
						poly_D = (gmt_polygon_is_open (GMT, D->table[tbl]->segment[seg]->data[GMT_X], D->table[tbl]->segment[seg]->data[GMT_Y], D->table[tbl]->segment[seg]->n_rows)) ? 1 : 0;
						sprintf (record, "N : Input %s %s not present in %s", feature[poly_D], src, from);
						GMT_Put_Record (API, GMT_WRITE_DATA, &Out);
					}
					continue;
				}
				for (tbl2 = 0; tbl2 < C->n_tables; tbl2++) {
					for (seg2 = 0; seg2 < C->table[tbl2]->n_segments; seg2++) {
						I = &(Info[tbl2][seg2]);
						if (I->mode == 0) continue;
						/* Report on all the close/exact matches */
						poly_D = (gmt_polygon_is_open (GMT, C->table[tbl2]->segment[seg2]->data[GMT_X],
						          C->table[tbl2]->segment[seg2]->data[GMT_Y], C->table[tbl2]->segment[seg2]->n_rows)) ? 1 : 0;
						(D->n_tables == 1) ? sprintf (src, "[ segment %" PRIu64 " ]", seg)  : sprintf (src, "[ table %" PRIu64 " segment %" PRIu64 " ]", tbl, seg);
						(C->n_tables == 1) ? sprintf (dup, "[ segment %" PRIu64 " ]", seg2) : sprintf (dup, "[ table %" PRIu64 " segment %" PRIu64 " ]", tbl2, seg2);
						if (I->mode == 5)
							sprintf (record, "| : Input %s %s was separated at the Dateline from %s %s in %s", feature[poly_D], src, feature[poly_S2], dup, from);
						else
							sprintf (record, format, verdict[abs(I->mode)], feature[poly_D], src, kind[I->mode+4], feature[poly_S2],
							         dup, from, I->distance, I->closeness, I->setratio);
						GMT_Put_Record (API, GMT_WRITE_DATA, &Out);
					}
				}
			}
		}
		if (same_feature) {
			for (col = 0; col < S2->n_columns; col++) S2->data[col] = NULL;	/* Since they were not allocated */
			gmt_free_segment (GMT, &S2);
		}
		for (tbl = 0; tbl < C->n_tables; tbl++) gmt_M_free (GMT, Info[tbl]);
		gmt_M_free (GMT, Info);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		if (Ctrl->D.file && GMT_Destroy_Data (API, &C) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	if (Ctrl->C.active) {	/* Clip polygon to bounding box */
		uint64_t np, p, nx, tbl, seg, col;
		double *cp[2] = {NULL, NULL};
		struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
		if (!GMT->common.J.active) {	/* -J not specified, set one implicitly */
			/* Supply dummy linear proj for Cartesian or geographic data */
			if (gmt_M_is_geographic (GMT, GMT_IN))
				gmt_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear degree projection */
			else
				gmt_parse_common_options (GMT, "J", 'J', "x1");		/* Fake linear Cartesian projection */
		}
		if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S = D->table[tbl]->segment[seg];
				SH = gmt_get_DS_hidden (S);
				if ((np = (*GMT->current.map.clip) (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, &cp[GMT_X], &cp[GMT_Y], &nx)) == 0) {
					/* Everything is outside, let go */
					SH->mode = GMT_WRITE_SKIP;
					continue;
				}
				smode = (S->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
				if (np > S->n_rows) S = GMT_Alloc_Segment (GMT->parent, smode, np, S->n_columns, NULL, S);
				for (p = 0; p < np; p++) gmt_xy_to_geo (GMT, &cp[GMT_X][p], &cp[GMT_Y][p], cp[GMT_X][p], cp[GMT_Y][p]);
				for (col = 0; col < 2; col++) {
					gmt_M_memcpy (S->data[col], cp[col], np, double);
					gmt_M_free (GMT, cp[col]);
				}
				S->n_rows = np;
			}
		}
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	
	if (Ctrl->N.active) {	/* Report the polygons that contain the given features */
		uint64_t tbl, row, first, last, n, p, np, seg, seg2, n_inside;
		unsigned int *count = NULL, nmode;
		int ID = -1;
		char seg_label[GMT_LEN64] = {""}, record[GMT_BUFSIZ] = {""}, *kind[2] = {"Middle point", "All points"};
		struct GMT_DATASET *C = NULL;
		struct GMT_DATATABLE *T = NULL;
		struct GMT_DATASEGMENT *S = NULL, *S2 = NULL;
		struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
		
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -CN files */
		if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (C->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)C->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		nmode = (Ctrl->N.mode == 1) ? GMT_IS_NONE : GMT_IS_LINE;
		if (GMT_Init_IO (API, GMT_IS_DATASET, nmode, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, nmode) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}

		if (Ctrl->N.mode == 2) gmt_adjust_dataset (GMT, D, D->n_columns + 1);	/* Add one more output column */
		
		T = C->table[0];	/* Only one input file so only one table */
		count = gmt_M_memory (GMT, NULL, D->n_segments, unsigned int);
		Out.text = record;
		for (seg2 = 0; seg2 < T->n_segments; seg2++) {	/* For all polygons */
			S2 = T->segment[seg2];
			SH = gmt_get_DS_hidden (S2);
			if (gmt_polygon_is_hole (GMT, S2)) continue;	/* Holes are handled in gmt_inonout */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Look for points/features inside polygon segment %" PRIu64 " :\n", seg2);
			if (Ctrl->N.ID == 0) {	/* Look for polygon IDs in the data headers */
				if (SH->ogr)	/* OGR data */
					ID = irint (gmt_get_aspatial_value (GMT, GMT_IS_Z, S2));
				else if (gmt_parse_segment_item (GMT, S2->header, "-Z", seg_label))	/* Look for segment header ID */
					ID = atoi (seg_label);
				else if (gmt_parse_segment_item (GMT, S2->header, "-L", seg_label))	/* Look for segment header ID */
					ID = atoi (seg_label);
				else
					GMT_Report (API, GMT_MSG_NORMAL, "No polygon ID found; ID set to NaN\n");
			}
			else	/* Increment running polygon ID */
				ID++;

			for (tbl = p = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++, p++) {
					S = D->table[tbl]->segment[seg];
					if (S->n_rows == 0) continue;
					if (Ctrl->N.all) { first = 0; last = S->n_rows - 1; np = S->n_rows; } else { first = last = S->n_rows / 2; np = 1; }
					for (row = first, n = 0; row <= last; row++) {	/* Check one or all points if they are inside */
						n += (gmt_inonout (GMT, S->data[GMT_X][row], S->data[GMT_Y][row], S2) == GMT_INSIDE);
					}
					if (n < np) continue;	/* Not inside this polygon */
					if (count[p]) {
						GMT_Report (API, GMT_MSG_NORMAL, "Segment %" PRIu64 "-%" PRIu64 " already inside another polygon; skipped\n", tbl, seg);
						continue;
					}
					count[p]++;
					/* Here we are inside */
					if (Ctrl->N.mode == 1) {	/* Just report on which polygon contains each feature */
						sprintf (record, "%s from table %" PRIu64 " segment %" PRIu64 " is inside polygon # %d", kind[Ctrl->N.all], tbl, seg, ID);
						GMT_Put_Record (API, GMT_WRITE_DATA, &Out);
					}
					else if (Ctrl->N.mode == 2) {	/* Add ID as last data column */
						for (row = 0, n = S->n_columns-1; row < S->n_rows; row++) S->data[n][row] = (double)ID;
						GMT_Report (API, GMT_MSG_VERBOSE, "%s from table %" PRIu64 " segment %" PRIu64 " is inside polygon # %d\n", kind[Ctrl->N.all], tbl, seg, ID);
					}
					else {	/* Add ID via the segment header -Z */
						if (gmt_parse_segment_item (GMT, S->header, "-Z", NULL))
							GMT_Report (API, GMT_MSG_NORMAL, "Segment header %d-%" PRIu64 " already has a -Z flag, skipped\n", tbl, seg);
						else {	/* Add -Z<ID< to the segment header */
							char buffer[GMT_BUFSIZ] = {""}, txt[GMT_LEN64] = {""};
							buffer[0] = txt[0] = 0;
							if (S->header) { strncpy (buffer, S->header, GMT_BUFSIZ-1); gmt_M_str_free (S->header); }
							sprintf (txt, " -Z%d", ID);
							strcat (buffer, txt);
							S->header = strdup (buffer);
							GMT_Report (API, GMT_MSG_VERBOSE, "%s from table %" PRIu64 " segment %" PRIu64 " is inside polygon # %d\n", kind[Ctrl->N.all], tbl, seg, ID);
						}
					}
				}
			}
		}
		for (p = n_inside = 0; p < D->n_segments; p++) if (count[p]) n_inside++;
		if (Ctrl->N.mode != 1) {	/* Write out results */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " segments found to be inside polygons, %" PRIu64 " were outside and skipped\n", n_inside, D->n_segments - n_inside);
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &C) != GMT_NOERROR) {
			Return (API->error);
		}
		gmt_M_free (GMT, count);
		Return (GMT_NOERROR);
	}
	if (Ctrl->S.active && Ctrl->S.mode == POL_SPLIT) {	/* Split polygons at dateline */
		bool crossing;
		uint64_t n_split = 0, tbl, seg_out, seg, n_segs, kseg, n_split_tot = 0;
		uint64_t dim[GMT_DIM_SIZE] = {0, 0, 0, 0};
		struct GMT_DATASET *Dout = NULL;
		struct GMT_DATATABLE *T = NULL;
		struct GMT_DATASEGMENT **L = NULL;
		
		dim[GMT_TBL] = D->n_tables;	dim[GMT_COL] = D->n_columns;	/* Same number of tables and columns as the input */
		if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
		/* Dout has no allocated segments yet */
		Dout->n_segments = 0;
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			T = Dout->table[tbl];
			n_segs = D->table[tbl]->n_segments;
			Dout->table[tbl]->n_segments = 0;
			T->segment = gmt_M_memory (GMT, NULL, n_segs, struct GMT_DATASEGMENT *);	/* Need at least this many segments */
			for (seg = seg_out = 0; seg < D->table[tbl]->n_segments; seg++) {
				S = D->table[tbl]->segment[seg];	/* Current input segment */
				if (S->n_rows == 0) continue;	/* Just skip empty segments */
				crossing = gmt_crossing_dateline (GMT, S);
				n_split = (crossing) ? gmt_split_poly_at_dateline (GMT, S, &L) : 1;
				Dout->table[tbl]->n_segments += n_split;
				if (Dout->table[tbl]->n_segments > n_segs) {	/* Must allocate more segment space */
					uint64_t old_n_segs = n_segs;
					n_segs = Dout->table[tbl]->n_segments;
					T->segment = gmt_M_memory (GMT, T->segment, n_segs, struct GMT_DATASEGMENT *);	/* Allow more space for new segments */
					gmt_M_memset (&(T->segment[old_n_segs]), n_segs - old_n_segs,  struct GMT_DATASEGMENT *);	/* Set to NULL */
				}
				/* Here there are space for all segments if new ones are added via splitting */
				if (crossing) {
					n_split_tot++; 
					for (kseg = 0; kseg < n_split; kseg++) {
						T->segment[seg_out++] = L[kseg];	/* Add the remaining segments to the end */
					}
					gmt_M_free (GMT, L);
				}
				else {	/* Just duplicate */
					T->segment[seg_out++] = gmt_duplicate_segment (GMT, S);
				}
			}
			Dout->table[tbl]->n_segments = seg_out;
			Dout->n_segments += seg_out;
		}
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " segments split across the Dateline\n", n_split_tot);
		if (GMT_Destroy_Data (API, &Dout) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	if (Ctrl->S.active && Ctrl->S.mode == POL_HOLE) {	/* Flag polygons that are holes of others */
		uint64_t n_holes = 0, tbl1, seg1, tbl2, seg2, seg_out, k1, k2;
		uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 0};	/* Only one output table */
		unsigned int side, *inside = NULL, *kase = NULL;
		int P_handedness, H_handedness;
		bool geo;
		double out[3];
		struct GMT_DATASET *Dout = NULL;
		struct GMT_DATATABLE *T1 = NULL, *T2 = NULL;
		struct GMT_DATASEGMENT *S1 = NULL, *S2 = NULL;
		struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
		struct GMT_TBLSEG *K = NULL;
		
		inside = gmt_M_memory (GMT, NULL, D->n_segments, unsigned int);
		kase = gmt_M_memory (GMT, NULL, D->n_segments, unsigned int);
		K = gmt_M_memory (GMT, NULL, D->n_segments, struct GMT_TBLSEG);
		geo = gmt_M_is_geographic (GMT, GMT_IN);

		for (tbl1 = k1 = 0; tbl1 < D->n_tables; tbl1++) {
			T1 = D->table[tbl1];
			for (seg1 = 0; seg1 < T1->n_segments; seg1++, k1++) {
				K[k1].tbl = tbl1;	K[k1].seg = seg1;	/* Fill out the K lookup array */
				S1 = D->table[tbl1]->segment[seg1];	/* Current input segment */
				if (S1->n_rows == 0) continue;	/* Just skip empty segments */
				if (S1->header && !strcmp (S1->header, "-Ph")) continue;	/* Marked as a hole already */
				for (tbl2 = k2 = 0; tbl2 < D->n_tables; tbl2++) {
					T2 = D->table[tbl2];
					for (seg2 = 0; seg2 < T2->n_segments; seg2++, k2++) {
						if (tbl2 < tbl1) continue;	/* Avoid duplication */
						if (tbl2 == tbl1 && seg2 <= seg1) continue;	/* Avoid duplication */
						S2 = D->table[tbl2]->segment[seg2];	/* Current input segment */
						if (S2->n_rows == 0) continue;	/* Just skip empty segments */
						if (S2->header && !strcmp (S2->header, "-Ph")) continue;	/* Marked as a hole already */
						/* Here we determine if S1 is inside S2 or vice versa */
						side = gmt_inonout (GMT, S1->data[GMT_X][0], S1->data[GMT_Y][0], S2);	/* Is S1 inside S2? */
						if (side == GMT_ONEDGE) {
							GMT_Report (API, GMT_MSG_NORMAL, "Polygon A (tbl=%" PRIu64 ", seg=%" PRIu64 ") is tangent to Polygon B (tbl=%" PRIu64 ", seg=%" PRIu64 ") at (%g/%g). Skipping\n", tbl1, seg1, tbl2, seg2, S1->data[GMT_X][0], S1->data[GMT_Y][0]);
							continue;
						}
						else if (side == GMT_INSIDE) {	/* S1 is inside S2 */
							n_holes++;
							kase[k1]++;
							inside[k1] = (unsigned int)k2 + 1;	/* So 0 means not inside anything (a perimeter) */
						}
						side = gmt_inonout (GMT, S2->data[GMT_X][0], S2->data[GMT_Y][0], S1);	/* Is S2 inside S1? */
						if (side == GMT_ONEDGE) {
							GMT_Report (API, GMT_MSG_NORMAL, "Polygon B (tbl=%" PRIu64 ", seg=%" PRIu64 ") is tangent to Polygon A (tbl=%" PRIu64 ", seg=%" PRIu64 ") at (%g/%g). Skipping\n", tbl2, seg2, tbl1, seg1, S2->data[GMT_X][0], S2->data[GMT_Y][0]);
							continue;
						}
						else if (side == GMT_INSIDE) {	/* S2 is inside S1 */
							n_holes++;
							kase[k2]++;
							inside[k2] = (unsigned int)k1 + 1;	/* So 0 means not inside anything (a perimeter) */
						}
					}
				}
			}
		}
		for (k1 = 0; k1 < D->n_segments; k1++) {	/* Make sure no polygon is inside more than one other polygon */
			if (kase[k1] > 1) {
				GMT_Report (API, GMT_MSG_NORMAL, "Polygon # %d is inside more than one polygon or is both inside and contains other polygons\n", k1);
				gmt_M_free (GMT, kase);
				Return (API->error);
			}
		}
		
		/* Create an output dataset with unallocated segments since rows = 0 */
		
		dim[GMT_COL] = D->n_columns;
		if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
		T1 = Dout->table[0];	/* Only one table used for output */
		T1->segment = gmt_M_memory (GMT, NULL, D->n_segments, struct GMT_DATASEGMENT *);	/* Need this many segments */
		Dout->n_segments = T1->n_segments = D->n_segments;
		/* Shuffle segments so perimeters (inside[] == 0) are listed before their holes (inside[] > 0) */
		for (k1 = seg_out = 0; k1 < D->n_segments; k1++) {	/* Loop over all polygons */
			if (inside[k1] == 0) {	/* Perimeter polygon */
				tbl1 = K[k1].tbl;	seg1 = K[k1].seg;	/* Get the (tbl,seg) indices for the perimeter */
				S1 = D->table[tbl1]->segment[seg1];		/* Current input segment */
				/* Duplicate this polygon as next output polygon perimeter */
				T1->segment[seg_out++] = gmt_duplicate_segment (GMT, S1);
				/* Get perimeter handedness */
				P_handedness = area_size (GMT, S1->data[GMT_X], S1->data[GMT_Y], S1->n_rows, out, geo);
				for (k2 = 0; k2 < D->n_segments; k2++) {	/* Loop over all polygons */
					if (k2 == k1 || inside[k2] != (k1+1)) continue;	/* Not a hole inside this perimeter */
					tbl2 = K[k2].tbl;	seg2 = K[k2].seg;	/* Get the (tbl,seg) indices for the hole */
					/* Duplicate this polygon as next output polygon hole */
					T1->segment[seg_out++] = S2 = gmt_duplicate_segment (GMT, D->table[tbl2]->segment[seg2]);
					/* Get hole handedness */
					H_handedness = area_size (GMT, S2->data[GMT_X], S2->data[GMT_Y], S2->n_rows, out, geo);
					/* If same handedness then reverse order of polygon */
					if (H_handedness == P_handedness) {
						uint64_t row_f, row_l, col;
						for (row_f = 0, row_l = S2->n_rows - 1; row_f < S2->n_rows/2; row_f++, row_l--) {
							for (col = 0; col < S2->n_columns; col++) gmt_M_double_swap (S2->data[col][row_f], S2->data[col][row_l]);
						}
					}
					if (S2->header) {	/* Must append -Ph to existing header - need to allocate more space */
						S2->header = realloc (S2->header, strlen (S2->header) + 5U);
						strncat (S2->header, " -Ph", 4U);
					}
					else
						S2->header = strdup ("-Ph");
					SH = gmt_get_DS_hidden (S2);
					SH->pol_mode = GMT_IS_HOLE;
				}
			}
		}
		
		gmt_M_free (GMT, kase);
		gmt_M_free (GMT, inside);
		gmt_M_free (GMT, K);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Dout) != GMT_NOERROR) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " segments were holes in other polygons\n", n_holes);
	}
	
	if (Ctrl->F.active) {	/* We read as polygons to force closure, now write out revised data */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	Return (GMT_NOERROR);
}
