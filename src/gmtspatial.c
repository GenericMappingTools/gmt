/*--------------------------------------------------------------------
*    $Id$
*
*	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
*	Contact info: gmt.soest.hawaii.edu
*--------------------------------------------------------------------*/
/* 
 * gmtspatial performs miscellaneous geospatial operations on polygons, such
 * as truncating them against a clipping polygon, calculate areas, find
 * crossings with other polygons, etc.
 *
 * Author:	Paul Wessel
 * Date:	10-Jun-2009
 * Version:	5 API
 */

#include "gmt.h"

EXTERN_MSC GMT_LONG GMT_wesn_clip (struct GMT_CTRL *GMT, double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx);
void GMT_duplicate_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *Sin, struct GMT_LINE_SEGMENT *Sout);

#define POL_IS_CW 1
#define POL_IS_CCW 0

#define POL_UNION		0
#define POL_INTERSECTION	1
#define POL_CLIP		2
#define POL_SPLIT		3

#define PW_TESTING
#define MIN_AREA_DIFF		0.01;	/* If two polygons have areas that differ more than 1 % of each other then they are not the same feature */
#define MIN_SEPARATION		0	/* If the two closest points for two features are > 0 units apart then they are not the same feature */
#define MIN_CLOSENESS		0.01	/* If two close segments has an mean separation exceeding 1% of segment legnth, then they are not the same feature */
#define MIN_SUBSET		2.0	/* If two close segments deemed approximate fits has lengths that differ by this factor then they are sub/super sets of each other */

struct DUP {
	uint64_t point;
	GMT_LONG segment;
	GMT_LONG table;
	GMT_LONG mode;
	GMT_LONG inside;
	double distance;
	double closeness;
	double setratio;
	double a_threshold;
	double d_threshold;
	double c_threshold;
	double s_threshold;
};

struct DUP_INFO {
	uint64_t point;
	GMT_LONG mode;
	double distance;
	double closeness;
	double setratio;
};

struct GMTSPATIAL_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct D {	/* -D[pol] */
		GMT_LONG active;
		GMT_LONG mode;
		char unit;
		char *file;
		struct DUP I;
	} D;
	struct E {	/* -D[-|+] */
		GMT_LONG active;
		GMT_LONG mode;
	} E;
	struct I {	/* -I[i|e] */
		GMT_LONG active;
		GMT_LONG mode;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG mode;
		char unit;
		double s_cutoff, path_noise, box_offset;
	} L;
	struct N {	/* -N<file>[+a][+p>ID>][+r][+z] */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 for reporting ID in -Z<ID> header, 1 via data column, 2 just as a report */
		GMT_LONG all;	/* All points in lines and polygons must be inside a polygon for us to report ID */
		GMT_LONG start;	/* First ID for running polygon IDs */
		GMT_LONG ID;	/* If 1 we use running numbers */
		char *file;
	} N;
	struct Q {	/* -Q[+] */
		GMT_LONG active;
		GMT_LONG mode;
		char unit;
	} Q;
	struct S {	/* -S[u|i|c] */
		GMT_LONG active;
		GMT_LONG mode;
	} S;
	struct T {	/* -T[pol] */
		GMT_LONG active;
		char *file;
	} T;
};

struct PAIR {
	double node;
	GMT_LONG pos;
};

void *New_gmtspatial_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTSPATIAL_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GMTSPATIAL_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->L.box_offset = 1.0e-10;	/* Minimum significant amplitude */
	C->L.path_noise = 1.0e-10;	/* Minimum significant amplitude */
	C->D.I.a_threshold = MIN_AREA_DIFF;
	C->D.I.d_threshold = MIN_SEPARATION;
	C->D.I.c_threshold = MIN_CLOSENESS;
	C->D.I.s_threshold = MIN_SUBSET;
	return (C);
}

void Free_gmtspatial_Ctrl (struct GMT_CTRL *GMT, struct GMTSPATIAL_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);	
	if (C->D.file) free (C->D.file);	
	if (C->N.file) free (C->N.file);	
	if (C->T.file) free (C->T.file);	
	GMT_free (GMT, C);	
}

double area (double x[], double y[], GMT_LONG n)
{
	GMT_LONG i;
	double area, xold, yold;
	
	/* Trapezoidal area calculation.
	 * area will be +ve if polygon is CW, negative if CCW */
	
	area = yold = 0.0;
	xold = x[n-1];	yold = y[n-1];
	
	for (i = 0; i < n; i++) {
		area += (xold - x[i]) * (yold + y[i]);
		xold = x[i];	yold = y[i];
	}
	return (0.5 * area);
}

void centroid (struct GMT_CTRL *GMT, double x[], double y[], GMT_LONG n, double *pos, GMT_LONG geo)
{	/* Estimate mean position */
	GMT_LONG i, k;
	
	n--; /* Skip 1st point since it is repeated as last */
	if (n <= 0) return;

	if (geo) {	/* Geographic data, must use vector mean */
		double P[3], M[3];
		GMT_memset (M, 3, double);
		for (i = 0; i < n; i++) {
			GMT_geo_to_cart (GMT, y[i], x[i], P, TRUE);
			for (k = 0; k < 3; k++) M[k] += P[k];
		}
		GMT_normalize3v (GMT, M);
		GMT_cart_to_geo (GMT, &pos[GMT_Y], &pos[GMT_X], M, TRUE);
	}
	else {	/* Cartesian mean position */
		pos[GMT_X] = pos[GMT_Y] = 0.0;
		for (i = 0; i < n; i++) {
			pos[GMT_X] += x[i];
			pos[GMT_Y] += y[i];
		}
		pos[GMT_X] /= n;	pos[GMT_Y] /= n;
	}
}

GMT_LONG area_size (struct GMT_CTRL *GMT, double x[], double y[], GMT_LONG n, double *out, GMT_LONG *geo)
{
	GMT_LONG i;
	double wesn[4], xx, yy, size, ix, iy;
	double *xp = NULL, *yp = NULL;
	
	wesn[XLO] = wesn[YLO] = DBL_MAX;
	wesn[XHI] = wesn[YHI] = -DBL_MAX;
	
	centroid (GMT, x, y, n, out, *geo);	/* Get mean location */
	
	for (i = 0; i < n; i++) {
		wesn[XLO] = MIN (wesn[XLO], x[i]);
		wesn[XHI] = MAX (wesn[XHI], x[i]);
		wesn[YLO] = MIN (wesn[YLO], y[i]);
		wesn[YHI] = MAX (wesn[YHI], y[i]);
	}
	xp = GMT_memory (GMT, NULL, n, double);	yp = GMT_memory (GMT, NULL, n, double);
	if (*geo == 1) {	/* Initializes GMT projection parameters to the -JA settings */
		GMT->current.proj.projection = GMT_LAMB_AZ_EQ;
		GMT->current.proj.unit = 1.0;
		GMT->current.proj.pars[3] = 39.3700787401574814;
		GMT->common.R.oblique = FALSE;
		GMT->common.J.active = TRUE;
		GMT->current.setting.map_line_step = 1.0e7;	/* To avoid nlon/nlat being huge */
		GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;
		GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
		GMT->current.proj.pars[0] = out[GMT_X];
		GMT->current.proj.pars[1] = out[GMT_Y];
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, wesn), "")) return (0);
	
		ix = 1.0 / GMT->current.proj.scale[GMT_X];
		iy = 1.0 / GMT->current.proj.scale[GMT_Y];
	
		for (i = 0; i < n; i++) {
			GMT_geo_to_xy (GMT, x[i], y[i], &xx, &yy);
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
	
	size = area (xp, yp, n);
	GMT_free (GMT, xp);
	GMT_free (GMT, yp);
	if (geo) size *= (GMT->current.map.dist[GMT_MAP_DIST].scale * GMT->current.map.dist[GMT_MAP_DIST].scale);
	out[GMT_Z] = fabs (size);
	return ((size < 0.0) ? POL_IS_CCW : POL_IS_CW);
}

void length_size (struct GMT_CTRL *GMT, double x[], double y[], GMT_LONG n, double *out)
{
	GMT_LONG i;
	double length = 0.0, mid, f, *s = NULL;
	
	/* Estimate 'average' position */
	
	s = GMT_memory (GMT, NULL, n, double);
	for (i = 1; i < n; i++) {
		length += GMT_distance (GMT, x[i-1], y[i-1], x[i], y[i]);
		s[i] = length;
	}
	mid = 0.5 * length;
	i = 0;
	while (s[i] <= mid) i++;	/* Find mid point length-wise */
	f = (mid - s[i-1]) / (s[i] - s[i-1]);
	out[GMT_X] = x[i-1] + f * (x[i] - x[i-1]);
	out[GMT_Y] = y[i-1] + f * (y[i] - y[i-1]);
	out[GMT_Z] = length;
	GMT_free (GMT, s);
}

int comp_pairs (const void *a, const void *b) {
	struct PAIR *xa, *xb;
	xa = (struct PAIR *)a;
	xb = (struct PAIR *)b;
	/* Sort on node value */
	if (xa->node < xb->node) return (-1);
	if (xa->node > xb->node) return (+1);
	return (0);
}

void write_record (struct GMT_CTRL *GMT, double **R, GMT_LONG n, GMT_LONG p)
{
	GMT_LONG c;
	double out[GMT_MAX_COLUMNS];
	for (c = 0; c < n; c++) out[c] = R[c][p];
	GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);
}

GMT_LONG GMT_is_duplicate (struct GMT_CTRL *GMT, struct GMT_LINE_SEGMENT *S, struct GMT_DATASET *D, struct DUP *I, struct DUP_INFO **L)
{
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
	
	GMT_LONG tbl, seg, np, n_close = 0, n_dup = 0, status, mode1, mode3;
	uint64_t k, row, pt;
	double dist, f_seg, f_pt, d1, d2, closest, length[2], separation[2], close[2];
	double med_separation[2], med_close[2], high = 0, low = 0, use_length, *sep = NULL;
	struct GMT_LINE_SEGMENT *Sp = NULL;
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Determine the segments in D closest to our segment\n");
	I->distance = DBL_MAX;
	mode3 = 3 + 10 * I->inside;	/* Set GMT_near_lines modes */
	mode1 = 1 + 10 * I->inside;	/* Set GMT_near_lines modes */
	
	/* Process each point along the trace in S and find nearest distance for each segment in table */
	for (row = 0; row < S->n_rows; row++) {
		for (tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in current table */
				dist = DBL_MAX;	/* Reset for each line to find distance to that line */
				status = GMT_near_a_line (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], seg, D->table[tbl]->segment[seg], mode3, &dist, &f_seg, &f_pt);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the line segment */
				pt = lrint (f_pt);	/* We know f_seg == seg so no point assigning that */
				if (dist < I->distance) {	/* Keep track of the single closest feature */
					I->point = pt;
					I->segment = seg;
					I->distance = dist;
					I->table = tbl;
				}
				if (dist > I->d_threshold) continue;	/* Not close enough for duplicate consideration */
				if (D->table[tbl]->segment[seg]->mode == 0) {
					n_close++;
					L[tbl][seg].distance = DBL_MAX;
				}
				D->table[tbl]->segment[seg]->mode = 1;	/* Use mode to flag segments that are close enough */
				if (dist < L[tbl][seg].distance) {	/* Keep track of the closest feature */
					L[tbl][seg].point = pt;
					L[tbl][seg].distance = dist;
				}
			}
		}
	}
	
	/* Must also do the reverse test: for each point for each line in the table, compute distance to S; it might be shorter */
	
	for (tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in current table */
			Sp = D->table[tbl]->segment[seg];	/* This is S', one of the segments that is close to S */
			for (row = 0; row < Sp->n_rows; row++) {	/* Process each point along the trace in S and find nearest distance for each segment in table */
				dist = DBL_MAX;		/* Reset for each line to find distance to that line */
				status = GMT_near_a_line (GMT, Sp->coord[GMT_X][row], Sp->coord[GMT_Y][row], seg, S, mode3, &dist, &f_seg, &f_pt);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the line segment */
				pt = lrint (f_pt);
				if (dist < I->distance) {	/* Keep track of the single closest feature */
					I->point = pt;
					I->segment = seg;
					I->distance = dist;
					I->table = tbl;
				}
				if (dist > I->d_threshold) continue;	/* Not close enough for duplicate consideration */
				if (D->table[tbl]->segment[seg]->mode == 0) {
					n_close++;
					L[tbl][seg].distance = DBL_MAX;
				}
				D->table[tbl]->segment[seg]->mode = 1;	/* Use mode to flag segments that are close enough */
				if (dist < L[tbl][seg].distance) {	/* Keep track of the closest feature */
					L[tbl][seg].point = pt;
					L[tbl][seg].distance = dist;
				}
			}
		}
	}
	
	if (n_close == 0)
		GMT_report (GMT, GMT_MSG_VERBOSE, "No other segment found within dmax [probably due to +p requirement]\n");
	else
		GMT_report (GMT, GMT_MSG_VERBOSE, "Closest segment (Table %ld, segment %ld) is %.3f km away; %ld segments found within dmax\n", I->table, I->segment, I->distance, n_close);
	
	/* Here we have found the shortest distance from a point on S to another segment; S' is segment number seg */
	
	if (I->distance > I->d_threshold) return (0);	/* We found no duplicates or slightly different versions of S */
	
	/* Here we need to compare one or more S' candidates and S a bit more. */
	
	for (row = 1, length[0] = 0.0; row < S->n_rows; row++) {	/* Compute length of S once */
		length[0] += GMT_distance (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], S->coord[GMT_X][row-1], S->coord[GMT_Y][row-1]);
	}

	for (tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table to compare it to */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in current table */
			if (D->table[tbl]->segment[seg]->mode != 1) continue;	/* Not one of the close segments we flagged earlier */
			D->table[tbl]->segment[seg]->mode = 0;			/* Remove this temporary flag */
			
			Sp = D->table[tbl]->segment[seg];	/* This is S', one of the segments that is close to S */
			if (S->n_rows == Sp->n_rows) {	/* Exacly the same number of data points; check for identical duplicate (possibly reversed) */
				for (row = 0, d1 = d2 = 0.0; row < S->n_rows; row++) {	/* Compare each point along the trace in S and Sp and S and Sp-reversed */
					d1 += GMT_distance (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], Sp->coord[GMT_X][row], Sp->coord[GMT_Y][row]);
					d2 += GMT_distance (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], Sp->coord[GMT_X][S->n_rows-row-1], Sp->coord[GMT_Y][S->n_rows-row-1]);
				}
				GMT_report (GMT, GMT_MSG_DEBUG, "S to Sp of same length and gave d1 = %g and d2 = %g\n", d1, d2);
				if (GMT_IS_ZERO (d1)) { /* Exact duplicate */
					L[tbl][seg].mode = +1;
					n_dup++;
					continue;
				}
				else if (GMT_IS_ZERO (d2)) {	/* Exact duplicate but reverse order */
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
				sep = GMT_memory (GMT, NULL, MAX (S->n_rows, Sp->n_rows), double);
				low = DBL_MAX;
				high = -DBL_MAX;
			}
			for (row = np = 0; row < S->n_rows; row++) {	/* Process each point along the trace in S */
				dist = DBL_MAX;
				status = GMT_near_a_line (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], seg, Sp, mode1, &dist, NULL, NULL);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the Sp segment */
				separation[0] += dist;
				if (I->mode) {	/* Special processing for median calculation */
					sep[np] = dist;
					if (dist < low)  low  = dist;
					if (dist > high) high = dist;
				}
				np++;	/* Number of points within the overlap zone */
			}
			separation[0] = (np) ? separation[0] / np : DBL_MAX;		/* Mean distance between S and S' */
			use_length = (np) ? length[0] * np / S->n_rows : length[0];	/* ~reduce length to overlap section assuming equal point spacing */
			close[0] = (np) ? separation[0] / use_length : DBL_MAX;		/* Closeness as viewed from S */
			GMT_report (GMT, GMT_MSG_VERBOSE, "S has length %.3f km, has mean separation to Sp of %.3f km, and a closeness ratio of %g [n = %ld/%ld]\n", length[0], separation[0], close[0], np, S->n_rows);
			if (I->mode) {
				if (np) {
					GMT_median (GMT, sep, np, low, high, separation[0], &med_separation[0]);
					med_close[0] = med_separation[0] / use_length;
				}
				else med_close[0] = DBL_MAX;
				GMT_report (GMT, GMT_MSG_VERBOSE, "S  has median separation to Sp of %.3f km, and a robust closeness ratio of %g\n", med_separation[0], med_close[0]);
			}
	
			/* Must now compare the other way */
	
			separation[1] = length[1] = 0.0;
			if (I->mode) {	/* Reset for median calculation */
				low  = +DBL_MAX;
				high = -DBL_MAX;
			}
			for (row = np = 0; row < Sp->n_rows; row++) {	/* Process each point along the trace in S' */
				dist = DBL_MAX;		/* Reset for each line to find distance to that line */
				status = GMT_near_a_line (GMT, Sp->coord[GMT_X][row], Sp->coord[GMT_Y][row], seg, S, mode1, &dist, NULL, NULL);
				if (row) length[1] += GMT_distance (GMT, Sp->coord[GMT_X][row], Sp->coord[GMT_Y][row], Sp->coord[GMT_X][row-1], Sp->coord[GMT_Y][row-1]);
				if (!status && I->inside) continue;	/* Only consider points that project perpendicularly within the Sp segment */
				separation[1] += dist;
				if (I->mode) {	/* Special processing for median calculation */
					sep[np] = dist;
					if (dist < low)  low  = dist;
					if (dist > high) high = dist;
				}
				np++;	/* Number of points within the overlap zone */
			}
			separation[1] = (np) ? separation[1] / np : DBL_MAX;		/* Mean distance between S' and S */
			use_length = (np) ? length[1] * np / Sp->n_rows : length[1];	/* ~reduce length to overlap section assuming equal point spacing */
			close[1] = (np) ? separation[1] / use_length : DBL_MAX;		/* Closeness as viewed from S' */
			GMT_report (GMT, GMT_MSG_VERBOSE, "Sp has length %.3f km, has mean separation to S of %.3f km, and a closeness ratio of %g [n = %ld/%ld]\n", length[1], separation[1], close[1], np, Sp->n_rows);
			if (I->mode) {
				if (np) {
					GMT_median (GMT, sep, np, low, high, separation[1], &med_separation[1]);
					med_close[1] = med_separation[1] / use_length;
				}
				else med_close[1] = DBL_MAX;
				GMT_free (GMT, sep);
				GMT_report (GMT, GMT_MSG_VERBOSE, "Sp has median separation to S  of %.3f km, and a robust closeness ratio of %g\n", med_separation[1], med_close[1]);
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
			d1 = GMT_distance (GMT, S->coord[GMT_X][0], S->coord[GMT_Y][0], Sp->coord[GMT_X][0], Sp->coord[GMT_Y][0]);
			d2 = GMT_distance (GMT, S->coord[GMT_X][0], S->coord[GMT_Y][0], Sp->coord[GMT_X][Sp->n_rows-1], Sp->coord[GMT_Y][Sp->n_rows-1]);
	
			L[tbl][seg].setratio = (length[0] > length[1]) ? length[0] / length[1] : length[1] / length[0];	/* Ratio of length is used to detect subset (or superset) */
	
			if (length[0] < (length[1] / I->s_threshold))
				k = 3;	/* S is a subset of Sp */
			else if (length[1] < (length[0] / I->s_threshold))
				k = 4;	/* S is a superset of Sp */
			else
				k = 2;	/* S and S' are practically the same feature */
			L[tbl][seg].mode = (d1 < d2) ? k : -k;	/* Negative means Sp is reversed w.r.t. S */
			n_dup++;
		}
	}
	return (n_dup);
}

GMT_LONG GMT_gmtspatial_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtspatial %s [API] - Do geospatial operations on lines and polygons\n\n", GMT_VERSION);
#ifdef PW_TESTING
	GMT_message (GMT, "usage: gmtspatial [<table>] [-C] [-D[+f<file>][+a<amax>][+d%s][+c|C<cmax>][+s<sfact>][+p]]\n\t[-E+|-] [-I[i|e]] [-L%s/<pnoise>/<offset>] [-Q[<unit>][+]]\n", GMT_DIST_OPT, GMT_DIST_OPT);
#else
	GMT_message (GMT, "usage: gmtspatial [<table>] [-C] [-D[+f<file>][+a<amax>][+d%s][+c|C<cmax>][+s<sfact>][+p]]\n\t[-E+|-] [-I[i|e]] [%s] [-N<pfile>[+a][+p<ID>][+z]] [-Q[<unit>][+]]\n", GMT_DIST_OPT);
#endif
	GMT_message (GMT, "\t[%s] [-Su|i] [-T[<cpol>]] [-V[l]] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-C Clip polygons to the given region box (requires -R), yielding a closed polygon.\n");
	GMT_message (GMT, "\t   For truncation instead (yielding open polygons, i.e., lines), see -T.\n");
	GMT_message (GMT, "\t-D Look for duplicates or near-duplicates in the data, or compare against <file> (if given).\n");
	GMT_message (GMT, "\t   Near-duplicates have a min point separation less than <dmax> [0] and closeness\n");
	GMT_message (GMT, "\t   (mean separation/length ratio) less than <cmax> [0.01].\n");
	GMT_message (GMT, "\t   If near-duplicates has lengths that differ by <sfact> then they are subset/supersets [2].\n");
	GMT_message (GMT, "\t   For polygons the fractional difference in areas must be less than <amax> [0.01].\n");
	GMT_message (GMT, "\t   Use +C to use median separation [+c uses mean separation].\n");
	GMT_message (GMT, "\t   By default we consider all points when comparing two lines.  Use +p to limit\n");
	GMT_message (GMT, "\t   the comparison to points that project perpendicularly on to the other line.\n");
	GMT_message (GMT, "\t-E Orient all polygons to have the same handedness.\n");
	GMT_message (GMT, "\t   Append + for counter-clockwise or - for clockwise handedness.\n");
	GMT_message (GMT, "\t-I Compute Intersection locations between input polygon(s).\n");
	GMT_message (GMT, "\t   Append e or i for external or internal crossings only [Default is both].\n");
	GMT_message (GMT, "\t   Use uppercase E or I to consider all segments within the same table as one entity [separate].\n");
#ifdef PW_TESTING
	GMT_message (GMT, "\t-L Remove tile Lines.  These are superfluous lines along the -R border.\n");
	GMT_message (GMT, "\t   Append gap_dist (in m) [0], coordinate noise [1e-10], and max offset from gridline [1e-10].\n");
#endif
	GMT_message (GMT, "\t-N Determine ID of polygon (in <pfile>) enclosing each input feature.  The ID is set as follows:\n");
	GMT_message (GMT, "\t     a) If OGR/GMT polygons, get polygon ID via -a for Z column, else\n");
	GMT_message (GMT, "\t     b) Interpret segment labels (-Z<value>) as polygon IDs, else\n");
	GMT_message (GMT, "\t     c) Interpret segment labels (-L<label>) as polygon IDs, else\n");
	GMT_message (GMT, "\t     d) Append +p<ID> to set origin for running polygon IDs [0].\n");
	GMT_message (GMT, "\t   Modifier +a means all points of a feature (line, polygon) must be inside the ID polygon [mid point].\n");
	GMT_message (GMT, "\t   Modifier +z means append the ID as a new output data column [Default adds -Z<ID> to segment header].\n");
	GMT_message (GMT, "\t   Modifier +r means no table output; just reports which polygon a feature is inside.\n");
	GMT_message (GMT, "\t-Q Measure area and handedness of polygon(s) or length of line segments.  If -fg is used\n");
	GMT_message (GMT, "\t   you may append unit %s [k]; otherwise it will be based on the input data units.\n", GMT_LEN_UNITS_DISPLAY);
	GMT_message (GMT, "\t   We also compute polygon centroid or line mid-point.\n");
	GMT_message (GMT, "\t   Use -Q+ to place the (area, handedness) or length result in the segment header\n");
	GMT_message (GMT, "\t   on output [Default only reports results to stdout].\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Spatial manipulation of polygons; choose among:\n");
	GMT_message (GMT, "\t   i for intersection.\n");
	GMT_message (GMT, "\t   u for union.\n");
	GMT_message (GMT, "\t   s for splitting polygons that straddle the Dateline.\n");
	GMT_message (GMT, "\t   j for joining polygons that were split by the Dateline.\n");
	GMT_message (GMT, "\t-T Truncate polygons against the clip polygon <cpol>; if none is given we require -R\n");
	GMT_message (GMT, "\t   and clip against a polygon derived from the region border.\n");
	GMT_explain_options (GMT, "VfghC2D0io:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtspatial_parse (struct GMTAPI_CTRL *C, struct GMTSPATIAL_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_files[2] = {0, 0}, pos, n, n_errors = 0;
	char txt_a[GMT_TEXT_LEN64], txt_b[GMT_TEXT_LEN64], txt_c[GMT_TEXT_LEN64], p[GMT_TEXT_LEN256], *s = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	if (GMT_is_geographic (GMT, GMT_IN)) Ctrl->Q.unit = 'k';	/* Default geographic distance unit is km */
	
	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				n_files[GMT_IN]++;
				break;
			case '>':	/* Got named output file */
				if (n_files[GMT_OUT]++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Clip to given region */
				Ctrl->C.active = TRUE;
				break;
			case 'D':	/* Look for duplications */
				Ctrl->D.active = TRUE;
				pos = 0;
				while (GMT_strtok (opt->arg, "+", &pos, p)) {
					switch (p[0]) {
						case 'a':	/* Gave a new +a<dmax> value */
							GMT_report (GMT, GMT_MSG_FATAL, "+a not implemented yet\n");
							Ctrl->D.I.a_threshold = atof (&p[1]);
							break;
						case 'd':	/* Gave a new +d<dmax> value */
							Ctrl->D.mode = GMT_get_distance (GMT, &p[1], &(Ctrl->D.I.d_threshold), &(Ctrl->D.unit));
							break;
						case 'C':	/* Gave a new +C<dmax> value */
							Ctrl->D.I.mode = 1;	/* Median instead of mean */
						case 'c':	/* Gave a new +c<dmax> value */
							Ctrl->D.I.c_threshold = atof (&p[1]);
							break;
						case 's':	/* Gave a new +s<fact> value */
							Ctrl->D.I.s_threshold = atof (&p[1]);
							break;
						case 'p':	/* Consider only inside projections */
							Ctrl->D.I.inside = 1;
							break;
						case 'f':	/* Gave a file name */
							Ctrl->D.file = strdup (&p[1]);
							break;
					}
				}
				break;
			case 'E':	/* Orient polygons */
			 	Ctrl->E.active = TRUE;
				if (opt->arg[0] == '-')
					Ctrl->E.mode = POL_IS_CW;
				else if (opt->arg[0] == '+')
					Ctrl->E.mode = POL_IS_CCW;
				else
					n_errors++;
				break;
			case 'I':	/* Compute intersections between polygons */
				Ctrl->I.active = TRUE;
				if (opt->arg[0] == 'i') Ctrl->I.mode = 1;
				if (opt->arg[0] == 'I') Ctrl->I.mode = 2;
				if (opt->arg[0] == 'e') Ctrl->I.mode = 4;
				if (opt->arg[0] == 'E') Ctrl->I.mode = 8;
				break;
#ifdef PW_TESTING
			case 'L':	/* Remove tile lines */
				Ctrl->L.active = TRUE;
				n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				if (n >= 1) Ctrl->L.s_cutoff = atof (txt_a);
				if (n >= 2) Ctrl->L.path_noise = atof (txt_b);
				if (n == 3) Ctrl->L.box_offset = atof (txt_c);
				break;
#endif
			case 'Q':	/* Measure area/length and handedness of polygons */
				Ctrl->Q.active = TRUE;
				if (strchr (opt->arg, '+')) Ctrl->Q.active = 2;
				if (opt->arg[0] && opt->arg[0] != '+') Ctrl->Q.unit = opt->arg[0];				
				break;
			case 'N':	/* Determine containing polygons for features */
				Ctrl->N.active = TRUE;
				if ((s = strchr (opt->arg, '+')) == NULL) {	/* No modifiers */
					Ctrl->N.file = strdup (opt->arg);
					continue;
				}
				s[0] = '\0';	Ctrl->N.file = strdup (opt->arg);	s[0] = '+';
				pos = 0;
				while (GMT_strtok (s, "+", &pos, p)) {
					switch (p[0]) {
						case 'a':	/* All points must be inside polygon */
							Ctrl->N.all = TRUE;
							break;
						case 'p':	/* Set start of running numbers [0] */
							Ctrl->N.ID = 1;
							Ctrl->N.start = (p[1]) ? atoi (&p[1]) - 1 : -1;	/* We increment before use */
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
			case 'S':	/* Spatial assembly */
				Ctrl->S.active = TRUE;
				if (opt->arg[0] == 'u')
					Ctrl->S.mode = POL_UNION;
				else if (opt->arg[0] == 'i')
					Ctrl->S.mode = POL_INTERSECTION;
				else if (opt->arg[0] == 'c')
					Ctrl->S.mode = POL_CLIP;
				else if (opt->arg[0] == 's')
					Ctrl->S.mode = POL_SPLIT;
				else
					n_errors++;
				break;
			case 'T':	/* Truncate against polygon */
				Ctrl->T.active = TRUE;
				Ctrl->C.active = Ctrl->S.active = TRUE;
				Ctrl->S.mode = POL_CLIP;
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

 	if (Ctrl->E.active) Ctrl->Q.active = TRUE;
	
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, n_files[GMT_IN] == 0, "Syntax error: No input file specified\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least %d columns\n", 2);
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == POL_CLIP && !Ctrl->T.file && !GMT->common.R.active, "Syntax error: -T without a polygon requires -R\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && !Ctrl->T.active && !GMT->common.R.active, "Syntax error: -C requires -R\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && !GMT->common.R.active, "Syntax error: -L requires -R\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && Ctrl->L.s_cutoff < 0.0, "Syntax error: -L requires a positive cutoff in meters\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->D.file && GMT_access (GMT, Ctrl->D.file, R_OK), "Syntax error -D: Cannot read file %s!\n", Ctrl->D.file);
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.file && GMT_access (GMT, Ctrl->T.file, R_OK), "Syntax error -T: Cannot read file %s!\n", Ctrl->T.file);
	n_errors += GMT_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtspatial_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gmtspatial (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	uint64_t i, p;
	GMT_LONG j, k, error = 0, in, c, mseg = FALSE, geometry = GMT_IS_POLY;
	GMT_LONG internal = FALSE, external = FALSE;

	static char *kind[2] = {"CCW", "CW"};

	double out[GMT_MAX_COLUMNS];

	struct GMT_DATASET *D = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;
	struct GMTSPATIAL_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmtspatial_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmtspatial_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtspatial", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRbf:", "ghios" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_gmtspatial_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtspatial_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtspatial main code ----------------------------*/

	if (Ctrl->I.active) {
		switch (Ctrl->I.mode) {
			case 0:
				internal = external = TRUE;
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
	
	if (Ctrl->D.active) geometry = GMT_IS_LINE;
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Read Input tables\n");
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	
	/* Allocate memory and read in all the files; each file can have many lines (-m) */
	
	if (D->n_records == 0) {	/* Empty files, nothing to do */
		GMT_report (GMT, GMT_MSG_NORMAL, "No data records found.\n");
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
		Return (EXIT_SUCCESS);
	}
	
	if (Ctrl->S.active && Ctrl->S.mode != POL_SPLIT) external = TRUE;
	
	GMT_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);	/* Use Cartesian calculations and user units */
	
	/* OK, with data in hand we can do some damage */
	
	if (Ctrl->L.active) {	/* Remove tile lines only */
		GMT_LONG gap = 0, first, prev_OK;
		uint64_t p;
		double dx, dy, DX, DY, dist;

		GMT_init_distaz (GMT, GMT_MAP_DIST_UNIT, 2, GMT_MAP_DIST);	/* Default is m using great-circle distances */
		
		for (k = 0; k < D->n_tables; k++) {
			for (j = 0; j < D->table[k]->n_segments; j++) {
				S = D->table[k]->segment[j];
				if (S->n_rows == 0) continue;
				for (p = 1, first = TRUE, prev_OK = FALSE; p < S->n_rows; p++) {
					dx = S->coord[GMT_X][p] - S->coord[GMT_X][p-1];
					dy = S->coord[GMT_Y][p] - S->coord[GMT_Y][p-1];
					DX = MIN (fabs (S->coord[GMT_X][p] - GMT->common.R.wesn[XLO]), fabs (S->coord[GMT_X][p] - GMT->common.R.wesn[XHI]));
					DY = MIN (fabs (S->coord[GMT_Y][p] - GMT->common.R.wesn[YLO]), fabs (S->coord[GMT_Y][p] - GMT->common.R.wesn[YHI]));
					gap = FALSE;
					if ((fabs (dx) < Ctrl->L.path_noise && DX < Ctrl->L.box_offset) || (fabs (dy) < Ctrl->L.path_noise && DY < Ctrl->L.box_offset)) {	/* Going along a tile boundary */
						dist = GMT_distance (GMT, S->coord[GMT_X][p], S->coord[GMT_Y][p], S->coord[GMT_X][p-1], S->coord[GMT_Y][p-1]);
						if (dist > Ctrl->L.s_cutoff) gap = TRUE;
					}
					if (gap) {	/* Distance exceed threshold, start new segment */
						first = TRUE;
						if (prev_OK) write_record (GMT, S->coord, S->n_columns, p-1);
						prev_OK = FALSE;
					}
					else {
						if (first) {
							strcpy (GMT->current.io.segment_header, S->header);
							GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
						}
						write_record (GMT, S->coord, S->n_columns, p-1);
						first = FALSE;
						prev_OK = TRUE;
					}
				}
				if (!gap) write_record (GMT, S->coord, S->n_columns, p-1);
			}
					
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
		Return (EXIT_SUCCESS);
	}
	
	if (Ctrl->Q.active) {	/* Calculate centroid and polygon areas or line lengths and place in segment headers */
		double out[3];
		GMT_LONG handedness = 0, c, p, mode, poly, geo = GMT_is_geographic (GMT, GMT_IN);

		char line[GMT_BUFSIZ];
		
		if (GMT_is_geographic (GMT, GMT_IN)) GMT_init_distaz (GMT, Ctrl->Q.unit, 2, GMT_MAP_DIST);	/* Default is m using great-circle distances */

		if (Ctrl->Q.active == 2) {
			mode = GMT_IS_POLY;
		}
		else {
			mode = GMT_IS_POINT;
			if ((error = GMT_set_cols (GMT, GMT_OUT, 3))) Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, mode, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		
		for (k = 0; k < D->n_tables; k++) {
			for (j = 0; j < D->table[k]->n_segments; j++) {
				S = D->table[k]->segment[j];
				if (S->n_rows == 0) continue;
				if (GMT_polygon_is_open (GMT, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows)) {	/* Line */
					length_size (GMT, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows, out);
					poly = FALSE;
				}
				else {	/* Polygon */
					handedness = area_size (GMT, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows, out, &geo);
					poly = TRUE;
				}
				if (Ctrl->Q.active == 2) {
					if (poly && Ctrl->E.active && handedness != Ctrl->E.mode) {	/* Must reverse line */
						for (i = 0, p = S->n_rows - 1; i < S->n_rows/2; i++, p--) {
							for (c = 0; c < S->n_columns; c++) d_swap (S->coord[c][i], S->coord[c][p]);
						}
						handedness = Ctrl->E.mode;
					}
					if (S->header) {
						if (poly)
							sprintf (line, "%s -A%.12g -C%.12g/%.12g %s", S->header, out[GMT_Z], out[GMT_X], out[GMT_Y], kind[handedness]);
						else
							sprintf (line, "%s -D%.12g -M%.12g/%.12g", S->header, out[GMT_Z], out[GMT_X], out[GMT_Y]);
						free (S->header);
					}
					else {
						if (poly)
							sprintf (line, "%c -A%.12g -C%.12g/%.12g %s", GMT->current.setting.io_seg_marker[GMT_OUT], out[GMT_Z], out[GMT_X], out[GMT_Y], kind[handedness]);
						else
							sprintf (line, "%s -D%.12g -M%.12g/%.12g", S->header, out[GMT_Z], out[GMT_X], out[GMT_Y]);
					}
					S->header = strdup (line);
				}
				else
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write area or length to output */
			}
		}
		/* Write out results */
		if (Ctrl->Q.active == 2) {
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, Ctrl->Out.file, D) != GMT_OK) {
				Return (API->error);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
		Return (EXIT_SUCCESS);
	}
	
	if (Ctrl->I.active || external) {	/* Crossovers between polygons */
		GMT_LONG k1, k2, j1, j2, nx, same;
		struct GMT_XSEGMENT *ylist1 = NULL, *ylist2 = NULL;
		struct GMT_XOVER XC;
		char T1[GMT_BUFSIZ], T2[GMT_BUFSIZ], fmt[GMT_BUFSIZ];
		struct GMT_DATASET *C = NULL;
		struct GMT_LINE_SEGMENT *S1 = NULL, *S2 = NULL;
		
		if (Ctrl->S.mode == POL_CLIP) {	/* Need to set up a separate table with the clip polygon */
			if (Ctrl->T.file) {
				if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->T.file, NULL)) == NULL) {
					Return (API->error);
				}
			}
			else {	/* Design a table based on -Rw/e/s/n */
				GMT_LONG dim[4] = {1, 1, 2, 5};
				if ((C = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) Return (API->error);
				S1 = C->table[0]->segment[0];
				S1->coord[GMT_X][0] = S1->coord[GMT_X][3] = S1->coord[GMT_X][4] = GMT->common.R.wesn[XLO];
				S1->coord[GMT_X][1] = S1->coord[GMT_X][2] = GMT->common.R.wesn[XHI];
				S1->coord[GMT_Y][0] = S1->coord[GMT_Y][1] = S1->coord[GMT_Y][4] = GMT->common.R.wesn[YLO];
				S1->coord[GMT_Y][2] = S1->coord[GMT_Y][3] = GMT->common.R.wesn[YHI];
			}
		}
		else
			C = D;	/* Compare with itself */
			
		if ((error = GMT_set_cols (GMT, GMT_OUT, C->n_columns)) != GMT_OK) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POLY, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}

		sprintf (fmt, "%s%s%s%s%s%s%s%s%%s%s%%s\n", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, \
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, \
			GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);

		for (k1 = 0; k1 < C->n_tables; k1++) {
			for (j1 = 0; j1 < C->table[k1]->n_segments; j1++) {
				S1 = C->table[k1]->segment[j1];
				if (S1->n_rows == 0) continue;
				GMT_init_track (GMT, S1->coord[GMT_Y], S1->n_rows, &ylist1);
				for (k2 = 0; k2 < D->n_tables; k2++) {
					for (j2 = 0; j2 < D->table[k2]->n_segments; j2++) {
						S2 = D->table[k2]->segment[j2];
						if (S2->n_rows == 0) continue;
						if (Ctrl->S.mode != POL_CLIP) {
							same = (external == 2 || internal == 2) ? (k1 == k2) : (k1 == k2 && j1 == j2);	/* What constitues the same feature */
							if (!internal && same) continue;	/* Do not do internal crossings */
							if (!external && !same) continue;	/* Do not do external crossings */
						}
						GMT_init_track (GMT, S2->coord[GMT_Y], S2->n_rows, &ylist2);
						nx = GMT_crossover (GMT, S1->coord[GMT_X], S1->coord[GMT_Y], NULL, ylist1, S1->n_rows, S2->coord[GMT_X], S2->coord[GMT_Y], NULL, ylist2, S2->n_rows, FALSE, &XC);
						if (nx) {	/* Polygon pair generated crossings */
							if (Ctrl->S.active) {	/* Do the spatial clip operation */
								uint64_t p0, px;
								GMT_LONG c, go, first;
								double *xx = NULL, *yy = NULL, *kk = NULL;
								struct PAIR *pair = NULL;
								
								pair = GMT_memory (GMT, NULL, nx, struct PAIR);
								xx = GMT_memory (GMT, NULL, nx, double);
								yy = GMT_memory (GMT, NULL, nx, double);
								kk = GMT_memory (GMT, NULL, nx, double);
								for (px = 0; px < nx; px++) pair[px].node = XC.xnode[1][px], pair[px].pos = px;
								qsort (pair, nx, sizeof (struct PAIR), comp_pairs);
								for (px = 0; px < nx; px++) {
									xx[px] = XC.x[pair[px].pos];
									yy[px] = XC.y[pair[px].pos];
									kk[px] = XC.xnode[1][pair[px].pos];
								}
								GMT_free (GMT, pair);
								in = GMT_non_zero_winding (GMT, S2->coord[GMT_X][0], S2->coord[GMT_Y][0], S1->coord[GMT_X], S1->coord[GMT_Y], S1->n_rows);
								go = first = TRUE;
								p0 = px = 0;
								while (go) {
									for (p = p0; p < S2->n_rows && p < kk[px]; p++) {
										if (!in) continue;
										for (c = 0; c < S2->n_columns; c++) out[c] = S2->coord[c][p];
										if (first && GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
											if (S2->header)
												strcpy (GMT->current.io.segment_header, S2->header);
											else
												sprintf (GMT->current.io.segment_header, "New segment");
											GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
											first = FALSE;
										}
										GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
									}
									/* Always output crossover point */
									if (first && GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
										if (S2->header)
											strcpy (GMT->current.io.segment_header, S2->header);
										else
											sprintf (GMT->current.io.segment_header, "New segment");
										GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
										first = FALSE;
									}
									for (c = 2; c < S2->n_columns; c++) out[c] = 0.0;
									out[GMT_X] = xx[px];	out[GMT_Y] = yy[px];
									GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
									px++;
									in = !in;	/* Go from out to in or vice versa */
									if (!in) first = TRUE;	/* Since we went outside */
									p0 = p;
									if (px == nx) {
										for (p = p0; p < S2->n_rows; p++) {
											if (!in) continue;
											for (c = 0; c < S2->n_columns; c++) out[c] = S2->coord[c][p];
											if (first && GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
												if (S2->header)
													strcpy (GMT->current.io.segment_header, S2->header);
												else
													sprintf (GMT->current.io.segment_header, "New segment");
												GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
												first = FALSE;
											}
											GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
										}
										go = FALSE;
									}
								}
								GMT_free (GMT, xx);
								GMT_free (GMT, yy);
								GMT_free (GMT, kk);
							}
							else {	/* Just report */
								if (mseg){
									sprintf (T1, "%s-%ld", C->table[k1]->file[GMT_IN], j1);
									sprintf (T2, "%s-%ld", D->table[k2]->file[GMT_IN], j2);
								}
								else {
									strcpy (T1, C->table[k1]->file[GMT_IN]);
									strcpy (T2, D->table[k2]->file[GMT_IN]);
								}
								for (i = 0; i < nx; i++) printf (fmt, XC.x[i], XC.y[i], (double)XC.xnode[0][i], (double)XC.xnode[1][i], T1, T2);
							}
							GMT_x_free (GMT, &XC);
						}
						else if (Ctrl->S.mode == POL_CLIP) {	/* No crossings; see if it is inside or outside C */
							if ((in = GMT_non_zero_winding (GMT, S2->coord[GMT_X][0], S2->coord[GMT_Y][0], S1->coord[GMT_X], S1->coord[GMT_Y], S1->n_rows))) {
								/* Inside, copy out the entire polygon */
								if (GMT->current.io.multi_segments[GMT_OUT]) {	/* Must find unique edges to output only once */
									if (S2->header)
										strcpy (GMT->current.io.segment_header, S2->header);
									else
										sprintf (GMT->current.io.segment_header, "New segment");
									GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
								}
								for (p = 0; p < S2->n_rows; p++) {
									for (c = 0; c < S2->n_columns; c++) out[c] = S2->coord[c][p];
									GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
								}
							}
						}
						GMT_free (GMT, ylist2);
						if (Ctrl->S.mode == POL_UNION) {
						}
						if (Ctrl->S.mode == POL_INTERSECTION) {
						}
					}
				}
				GMT_free (GMT, ylist1);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
		if (Ctrl->S.mode == POL_CLIP) {
			if (Ctrl->T.file) {
				if (GMT_Destroy_Data (API, GMT_ALLOCATED, &C) != GMT_OK) {
					Return (API->error);
				}
			}
			else
				GMT_free_dataset (GMT, &C);
		}
		Return (EXIT_SUCCESS);
	}
	
	if (Ctrl->D.active) {	/* Look for duplicates of lines or polygons */
		GMT_LONG tbl, seg, tbl2, seg2, k, n_dup, same = FALSE, poly_D, poly_S2;
		char *kind[9] = {"approximate-reversed-superset", "approximate-reversed-subset", "approximate-reversed", "exact-reversed" , "", "exact", "approximate", "approximate-subset", "approximate-superset"};
		char record[GMT_BUFSIZ], src[GMT_BUFSIZ], dup[GMT_BUFSIZ], *feature[2] = {"polygon", "line"}, *from = NULL;
		char *in = "the same data set", *verdict = "NY~-+";	/* No, Yes, Approximate, Subsection, Supersection */
		struct GMT_DATASET *C = NULL;
		struct GMT_LINE_SEGMENT *S1 = NULL, *S2 = NULL;
		struct DUP_INFO **Info = NULL, *I = NULL;
		
		if (Ctrl->D.file) {	/* Get trial features via a file */
			if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->D.file, NULL)) == NULL) {
				Return (API->error);
			}
			from = Ctrl->D.file;
		}
		else {
			C = D;	/* Compare with itself */
			same = TRUE;
			from = in;
			S2 = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);
			GMT_alloc_segment (GMT, S2, 0, C->n_columns, TRUE);
		}
		Info = GMT_memory (GMT, NULL, C->n_tables, struct DUP_INFO *);
		for (tbl = 0; tbl < C->n_tables; tbl++) Info[tbl] = GMT_memory (GMT, NULL, C->table[tbl]->n_segments, struct DUP_INFO);
			
		if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_TEXT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {
			Return (API->error);	/* Registers default output destination, unless already set */
		}
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT) != GMT_OK) {
			Return (API->error);				/* Enables data output and sets access mode */
		}

		GMT_init_distaz (GMT, Ctrl->D.unit, Ctrl->D.mode, GMT_MAP_DIST);

		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S1 = D->table[tbl]->segment[seg];
				if (S1->n_rows == 0) continue;
				
				GMT_report (GMT, GMT_MSG_VERBOSE, "Check if segment %ld from Table %ld has duplicates:\n", seg, tbl);
				if (same) {	/* We must exclude this segment from the comparison otherwise we end up finding itself as a duplicate */
					S2->n_rows = S1->n_rows;
					for (k = 0; k < S1->n_columns; k++) S2->coord[k] = S1->coord[k];
					S1->n_rows = 0;	/* This means it will be skipped by GMT_is_duplicate */
				}
				else
					S2 = S1;
				poly_S2 = (GMT_polygon_is_open (GMT, S2->coord[GMT_X], S2->coord[GMT_Y], S2->n_rows));
				for (tbl2 = 0; tbl2 < C->n_tables; tbl2++) GMT_memset (Info[tbl2], C->table[tbl2]->n_segments, struct DUP_INFO);
				n_dup = GMT_is_duplicate (GMT, S2, C, &(Ctrl->D.I), Info);	/* Returns -3, -2, -1, 0, +1, +2, or +3 */
				if (same) {
					S1->n_rows = S2->n_rows;	/* Reset the count */
					if (Ctrl->D.I.table < tbl || (Ctrl->D.I.table == tbl && Ctrl->D.I.segment < seg)) n_dup = 0;	/* To avoid reporting the same pair twice */
				}
				if (n_dup == 0) continue;	/* No duplicates */
				for (tbl2 = 0; tbl2 < C->n_tables; tbl2++) {
					for (seg2 = 0; seg2 < C->table[tbl2]->n_segments; seg2++) {
						I = &(Info[tbl2][seg2]);
						if (I->mode == 0) continue;
						/* Report on all the close/exact matches */
						poly_D = (GMT_polygon_is_open (GMT, C->table[tbl2]->segment[seg2]->coord[GMT_X], C->table[tbl2]->segment[seg2]->coord[GMT_Y], C->table[tbl2]->segment[seg2]->n_rows));
						sprintf (src, "[ table %ld segment %ld ]", tbl, seg);
						(C->n_tables == 1) ? sprintf (dup, "[ segment %ld ]", seg2) : sprintf (dup, "[ table %ld segment %ld ]", tbl2, seg2);
						sprintf (record, "%c : Input %s %s is an %s duplicate of a %s %s in %s, with d = %.3f c = %.6g s = %.4g", 
							verdict[GMT_abs(I->mode)], feature[poly_D], src, kind[I->mode+4], feature[poly_S2], dup, from, I->distance, I->closeness, I->setratio);
						GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					}
				}
			}
		}
		if (same) {
			for (k = 0; k < S2->n_columns; k++) S2->coord[k] = NULL;	/* Since they were not allocated */
			GMT_free_segment (GMT, S2);
		}
		for (tbl = 0; tbl < C->n_tables; tbl++) GMT_free (GMT, Info[tbl]);
		GMT_free (GMT, Info);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
		if (Ctrl->D.file && GMT_Destroy_Data (API, GMT_ALLOCATED, &C) != GMT_OK) {
			Return (API->error);
		}
		Return (EXIT_SUCCESS);
	}
	
	if (Ctrl->C.active) {	/* Clip polygon to bounding box */
		uint64_t np, p;
		GMT_LONG c, nx;
		double *cp[2] = {NULL, NULL};
		if (!GMT->common.J.active) {	/* -J not specified, set one implicitly */
			/* Supply dummy linear proj */
			GMT->current.proj.projection = GMT->current.proj.xyz_projection[GMT_X] = GMT->current.proj.xyz_projection[GMT_Y] = GMT_LINEAR;
			GMT->current.proj.pars[0] = GMT->current.proj.pars[1] = 1.0;
			GMT->common.J.active = TRUE;
		}
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
		for (k = 0; k < D->n_tables; k++) {
			for (j = 0; j < D->table[k]->n_segments; j++) {
				S = D->table[k]->segment[j];
				if ((np = GMT_wesn_clip (GMT, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows, &cp[GMT_X], &cp[GMT_Y], &nx)) == 0) continue;
				if (np > S->n_rows) GMT_alloc_segment (GMT, S, np, S->n_columns, FALSE);
				for (p = 0; p < np; p++) GMT_xy_to_geo (GMT, &cp[GMT_X][p], &cp[GMT_Y][p], cp[GMT_X][p], cp[GMT_Y][p]);
				for (c = 0; c < 2; c++) {
					GMT_memcpy (S->coord[c], cp[c], np, double);
					GMT_free (GMT, cp[c]);
				}
				S->n_rows = np;
			}
		}
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, Ctrl->Out.file, D) != GMT_OK) {
			Return (API->error);
		}
	}
	
	if (Ctrl->N.active) {	/* Report the polygons that contain the given features */
		uint64_t row, first, last, n, p, np;
		GMT_LONG tbl, seg, seg2, ID = -1, n_inside, *count = NULL;
		char seg_label[GMT_TEXT_LEN64], record[GMT_BUFSIZ], *kind[2] = {"Middle point", "All points"};
		struct GMT_DATASET *C = NULL;
		struct GMT_TABLE *T = NULL;
		struct GMT_LINE_SEGMENT *S = NULL, *S2 = NULL;
		
		if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, Ctrl->N.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Ctrl->N.mode == 1) {	/* Just report on which polygon contains each feature */
			if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_TEXT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default output destination, unless already set */
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
				Return (API->error);
			}
		}
		else {	/* Regular data output */
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default output destination, unless already set */
				Return (API->error);
			}
		}
		if (Ctrl->N.mode == 2) GMT_adjust_dataset (GMT, D, D->n_columns + 1);	/* Add one more output column */
		
		T = C->table[0];	/* Only one input file so only one table */
		for (seg2 = 0; seg2 < T->n_segments; seg2++) {	/* For all polygons */
			S2 = T->segment[seg2];
			if (GMT_polygon_is_hole (S2)) continue;	/* Holes are handled in GMT_inonout */
			GMT_report (GMT, GMT_MSG_VERBOSE, "Look for points/features inside polygon segment %ld :\n", seg2);
			if (Ctrl->N.ID == 0) {	/* Look for polygon IDs in the data headers */
				if (S2->ogr)	/* OGR data */
					ID = (GMT_LONG)GMT_get_aspatial_value (GMT, GMT_IS_Z, S2);
				else if (GMT_parse_segment_item (GMT, S2->header, "-Z", seg_label))	/* Look for segment header ID */
					ID = atoi (seg_label);
				else if (GMT_parse_segment_item (GMT, S2->header, "-L", seg_label))	/* Look for segment header ID */
					ID = atoi (seg_label);
				else
					GMT_report (GMT, GMT_MSG_FATAL, "No polygon ID found; ID set to NaN\n");
			}
			else	/* Increment running polygon ID */
				ID++;

			count = GMT_memory (GMT, NULL, D->n_segments, GMT_LONG);
			for (tbl = p = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++, p++) {
					S = D->table[tbl]->segment[seg];
					if (S->n_rows == 0) continue;
					if (Ctrl->N.all) { first = 0; last = S->n_rows - 1; np = S->n_rows; } else { first = last = S->n_rows / 2; np = 1; }
					for (row = first, n = 0; row <= last; row++) {	/* Check one or all points if they are inside */
						n += (GMT_inonout (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], S2) == GMT_INSIDE);
					}
					if (n < np) continue;	/* Not inside this polygon */
					if (count[p]) {
						GMT_report (GMT, GMT_MSG_FATAL, "Segment %ld-%ld already inside another polygon; skipped\n", tbl, seg);
						continue;
					}
					count[p]++;
					/* Here we are inside */
					if (Ctrl->N.mode == 1) {	/* Just report on which polygon contains each feature */
						sprintf (record, "%s from table %ld segment %ld is inside polygon # %ld", kind[Ctrl->N.all], tbl, seg, ID);
						GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					}
					else if (Ctrl->N.mode == 2) {	/* Add ID as last data column */
						for (row = 0, n = S->n_columns-1; row < S->n_rows; row++) S->coord[n][row] = (double)ID;
						GMT_report (GMT, GMT_MSG_NORMAL, "%s from table %ld segment %ld is inside polygon # %ld\n", kind[Ctrl->N.all], tbl, seg, ID);
					}
					else {	/* Add ID via the segment header -Z */
						if (GMT_parse_segment_item (GMT, S->header, "-Z", NULL))
							GMT_report (GMT, GMT_MSG_FATAL, "Segment header %ld-%ld already has a -Z flag, skipped\n", tbl, seg);
						else {	/* Add -Z<ID< to the segment header */
							char buffer[GMT_BUFSIZ], txt[GMT_TEXT_LEN64];
							buffer[0] = txt[0] = 0;
							if (S->header) { strcpy (buffer, S->header); free (S->header); }
							sprintf (txt, " -Z%ld", ID);
							strcat (buffer, txt);
							S->header = strdup (buffer);
							GMT_report (GMT, GMT_MSG_NORMAL, "%s from table %ld segment %ld is inside polygon # %ld\n", kind[Ctrl->N.all], tbl, seg, ID);
						}
					}
				}
			}
		}
		for (p = n_inside = 0; p < D->n_segments; p++) if (count[p]) n_inside++;
		if (Ctrl->N.mode != 1) {	/* Write out results */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, Ctrl->Out.file, D) != GMT_OK) {
				Return (API->error);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld segments found to be inside polygons, %ld were outside and skipped\n", n_inside, D->n_segments - n_inside);
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &C) != GMT_OK) {
			Return (API->error);
		}
		Return (EXIT_SUCCESS);
	}
	if (Ctrl->S.active) {	/* Do geospatial operations */
		GMT_LONG n_split = 0, tbl, seg, n_segs, crossing, dim[4] = {0, 1, 0, 0};
		struct GMT_DATASET *Dout = NULL;
		struct GMT_TABLE *T = NULL;
		struct GMT_LINE_SEGMENT **L = NULL;
		
		dim[0] = D->n_tables;	dim[2] = D->n_columns;
		if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) Return (API->error);
		Dout->n_segments = 0;
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			T = Dout->table[tbl];
			n_segs = D->table[tbl]->n_segments;
			Dout->table[tbl]->n_segments = 0;
			for (j = seg = 0; j < D->table[tbl]->n_segments; j++) {
				S = D->table[tbl]->segment[j];
				if (S->n_rows == 0) continue;	/* Just skip empty segments */
				crossing = GMT_crossing_dateline (GMT, S);
				n_split = (crossing) ? GMT_split_poly_at_dateline (GMT, S, &L) : 1;
				Dout->table[tbl]->n_segments += n_split;
				if (Dout->table[tbl]->n_segments > n_segs) {
					GMT_LONG old_n_segs = n_segs;
					n_segs = Dout->table[tbl]->n_segments;
					T->segment = GMT_memory (GMT, T->segment, n_segs, struct GMT_LINE_SEGMENT *);	/* Allow more space for new segments */
					GMT_memset (&(T->segment[old_n_segs]), n_segs - old_n_segs,  struct GMT_LINE_SEGMENT *);	/* Set to NULL */
				}
				if (crossing) {
					for (k = 0; k < n_split; k++) {
						GMT_free_segment (GMT, T->segment[seg]);
						T->segment[seg++] = L[k];	/* Add the remaining segments to the end */
					}
					GMT_free (GMT, L);
				}
				else {	/* Just duplicate */
					GMT_duplicate_segment (GMT, S, T->segment[seg++]);
				}
			}
			Dout->table[tbl]->n_segments = seg;
			Dout->n_segments += seg;
		}
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, Ctrl->Out.file, Dout) != GMT_OK) {
			Return (API->error);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld segments split across the Dateline\n", n_split);
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &Dout) != GMT_OK) {
			Return (API->error);
		}
	}
	
	Return (EXIT_SUCCESS);
}
