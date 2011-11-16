/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 *			G M T _ S U P P O R T . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_support.c contains code used by most GMT programs
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 *
 * Modules in this file:
 *
 *	GMT_akima		Akima's 1-D spline
 *	GMT_BC_init		Initialize BCs for a grid or image
 *	GMT_grd_BC_set		Set two rows of padding according to bound cond for grid
 *	GMT_image_BC_set	Set two rows of padding according to bound cond for image
 *	gmt_check_rgb		Check rgb for valid range
 *	GMT_chop		Chops off any CR or LF at end of string
 *	GMT_chop_ext		Chops off the trailing .xxx (file extension)
 *	gmt_cmyk_to_rgb		Corvert CMYK to RGB
 *	gmt_comp_double_asc	Used when sorting doubles into ascending order [checks for NaN]
 *	gmt_comp_float_asc	Used when sorting floats into ascending order [checks for NaN]
 *	gmt_comp_int_asc	Used when sorting ints into ascending order
 *	GMT_contours		Subroutine for contouring
 *	GMT_cspline		Natural cubic 1-D spline solver
 *	GMT_csplint		Natural cubic 1-D spline evaluator
 *	GMT_delaunay		Performs a Delaunay triangulation
 *	GMT_free		Memory deallocation
 *	GMT_get_annot_label	Construct degree/minute label
 *	GMT_get_annot_offset	Return offset in inches for text annotation
 *	GMT_get_index		Return color table entry for given z
 *	GMT_get_fill_from_z	Return fill type for given z
 *	GMT_get_format		Find # of decimals and create format string
 *	GMT_get_rgb_from_z	Return rgb for given z
 *	GMT_get_plot_array	Allocate memory for plotting arrays
 *	GMT_getfill		Decipher and check fill argument
 *	GMT_getinc		Decipher and check increment argument
 *	GMT_getpen		Decipher and check pen argument
 *	GMT_getrgb		Decipher and check color argument
 *	gmt_hsv_to_rgb		Convert HSV to RGB
 *	GMT_init_fill		Initialize fill attributes
 *	GMT_init_pen		Initialize pen attributes
 *	GMT_illuminate		Add illumination effects to rgb
 *	GMT_intpol		1-D interpolation
 *	gmt_lab_to_rgb		Corvert CIELAB LAB to RGB
 *	gmt_lab_to_xyz		Convert CIELAB LAB to XYZ
 *	GMT_malloc		Memory management
 *	GMT_memory		Memory allocation/reallocation
 *	GMT_non_zero_winding	Finds if a point is inside/outside a polygon
 *	GMT_putpen		Encode pen argument into textstring
 *	GMT_read_cpt		Read color palette file
 *	gmt_rgb_to_cmyk		Convert RGB to CMYK
 *	gmt_rgb_to_hsv		Convert RGB to HSV
 *	gmt_rgb_to_lab		Convert RGB to CMYK
 *	gmt_rgb_to_xyz		Convert RGB to CIELAB XYZ
 *	GMT_sample_cpt		Resamples the current cpt table based on new z-array
 *	gmt_smooth_contour	Use Akima's spline to smooth contour
 *	GMT_strlcmp		Compares strings (ignoring case) until first reaches null character
 *	GMT_strrcmp		Compares strings (ignoring case) from back to front
 *	GMT_strtok		Reiterant replacement of strtok
 *	gmt_trace_contour	Function that trace the contours in GMT_contours
 *	gmt_polar_adjust	Adjust label justification for polar projection
 *	gmt_xyz_to_rgb		Convert CIELAB XYZ to RGB
 *	gmt_xyz_to_lab		Convert CIELAB XYZ to LAB
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

EXTERN_MSC GMT_LONG GMT_grd_is_global (struct GMT_CTRL *C, struct GRD_HEADER *h);
EXTERN_MSC double GMT_distance_type (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE, GMT_LONG id);
EXTERN_MSC char * GMT_getuserpath (struct GMT_CTRL *C, const char *stem, char *path);	/* Look for user file */

/** @brief XYZ color of the D65 white point */
#define WHITEPOINT_X	0.950456
#define WHITEPOINT_Y	1.0
#define WHITEPOINT_Z	1.088754

/** 
 * @brief sRGB gamma correction, transforms R to R'
 * http://en.wikipedia.org/wiki/SRGB
 */
#define GAMMACORRECTION(t) (((t) <= 0.0031306684425005883) ? (12.92*(t)) : (1.055*pow((t), 0.416666666666666667) - 0.055))

/** 
 * @brief Inverse sRGB gamma correction, transforms R' to R 
 */
#define INVGAMMACORRECTION(t) (((t) <= 0.0404482362771076) ? ((t)/12.92) : pow(((t) + 0.055)/1.055, 2.4))

/** 
 * @brief CIE L*a*b* f function (used to convert XYZ to L*a*b*)
 * http://en.wikipedia.org/wiki/Lab_color_space
 */
#define LABF(t)	 ((t >= 8.85645167903563082e-3) ? pow(t,0.333333333333333) : (841.0/108.0)*(t) + (4.0/29.0))
#define LABINVF(t) ((t >= 0.206896551724137931) ? ((t)*(t)*(t)) : (108.0/841.0)*((t) - (4.0/29.0)))

unsigned char GMT_color_rgb[GMT_N_COLOR_NAMES][3] = {	/* r/g/b of X11 colors */
#include "gmt_color_rgb.h"
};

struct GMT_PEN_NAME {	/* Names of pens and their thicknesses */
	char name[16];
	double width;
};

struct GMT_PEN_NAME GMT_penname[GMT_N_PEN_NAMES] = {		/* Names and widths of pens */
#include "gmt_pennames.h"
};


#ifdef DEBUG
void gmt_memtrack_add (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr, void *prev_ptr, GMT_LONG size);
void gmt_memtrack_sub (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr);
#ifdef NEW_DEBUG
struct MEMORY_ITEM * gmt_memtrack_find (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, void *addr);
#else
GMT_LONG gmt_memtrack_find (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, void *ptr);
#endif
#endif

#ifndef HAVE_QSORT_R
double *GMT_x2sys_Y;	/* Must use global variable if there is no qsort_r on this system */
#elif defined(__APPLE__) || defined(__FreeBSD__)
/* Wonderful news: BSD and GLIBC has different argument order in qsort_r */
void qsort_r(void *base, size_t nel, size_t width, void *thunk, int (*compar)(void *, const void *, const void *));
#elif defined(WIN32)
/* More Wonderful news: Windows is a mix of BSD and GLIBC */
void qsort_r(void *base, size_t nel, size_t width, int (*compar)(void *, const void *, const void *), void *thunk);
#else
void qsort_r(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *, void *), void *thunk);
#endif

/*----------------------------------------------------------------------------- */
#ifdef GMT_QSORT
/* Need to replace OS X's qsort with one that works for 64-bit data */
#include "gmt_qsort.c"
#endif

void gmt_rgb_to_hsv (struct GMT_CTRL *C, double rgb[], double hsv[])
{
	double diff;
	GMT_LONG i, imax = 0, imin = 0;

	/* This had checks using rgb value in doubles (e.g. (max_v == xr)), which failed always on some compilers.
	   Changed to integer logic: 2009-02-05 by RS.
	*/
	hsv[3] = rgb[3];	/* Pass transparency unchanged */
	for (i = 1; i < 3; i++) {
		if (rgb[i] > rgb[imax]) imax = i;
		if (rgb[i] < rgb[imin]) imin = i;
	}
	diff = rgb[imax] - rgb[imin];
	hsv[0] = 0.0;
	hsv[1] = (rgb[imax] == 0.0) ? 0.0 : diff / rgb[imax];
	hsv[2] = rgb[imax];
	if (hsv[1] == 0.0) return;	/* Hue is undefined */
	hsv[0] = 120.0 * imax + 60.0 * (rgb[(imax + 1) % 3] - rgb[(imax + 2) % 3]) / diff;
	if (hsv[0] < 0.0) hsv[0] += 360.0;
	if (hsv[0] > 360.0) hsv[0] -= 360.0;
}

void gmt_hsv_to_rgb (struct GMT_CTRL *C, double rgb[], double hsv[])
{
	int i;
	double h, f, p, q, t, rr, gg, bb;

	rgb[3] = hsv[3];	/* Pass transparency unchanged */
	if (hsv[1] == 0.0)
		rgb[0] = rgb[1] = rgb[2] = hsv[2];
	else {
		h = hsv[0];
		while (h >= 360.0) h -= 360.0;
		while (h < 0.0) h += 360.0;
		h /= 60.0;
		i = (int) floor (h);
		f = h - i;
		p = hsv[2] * (1.0 - hsv[1]);
		q = hsv[2] * (1.0 - (hsv[1] * f));
		t = hsv[2] * (1.0 - (hsv[1] * (1.0 - f)));
		switch (i) {
			case 0:
				rr = hsv[2]; gg = t; bb = p;
				break;
			case 1:
				rr = q; gg = hsv[2]; bb = p;
				break;
			case 2:
				rr = p; gg = hsv[2]; bb = t;
				break;
			case 3:
				rr = p; gg = q; bb = hsv[2];
				break;
			case 4:
				rr = t; gg = p; bb = hsv[2];
				break;
			default:
				rr = hsv[2]; gg = p; bb = q;
				break;
		}

		rgb[0] = (rr < 0.0) ? 0.0 : rr;
		rgb[1] = (gg < 0.0) ? 0.0 : gg;
		rgb[2] = (bb < 0.0) ? 0.0 : bb;
	}
}

void gmt_rgb_to_cmyk (struct GMT_CTRL *C, double rgb[], double cmyk[])
{
	/* Plain conversion; with default undercolor removal or blackgeneration */
	/* RGB is in 0-1, CMYK will be in 0-1 range */

	int i;

	cmyk[4] = rgb[3];	/* Pass transparency unchanged */
	for (i = 0; i < 3; i++) cmyk[i] = 1.0 - rgb[i];
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Default Black generation */
	if (cmyk[3] < GMT_CONV_LIMIT) cmyk[3] = 0.0;

	/* To implement device-specific blackgeneration, supply lookup table K = BG[cmyk[3]] */

	for (i = 0; i < 3; i++) {
		cmyk[i] -= cmyk[3];		/* Default undercolor removal */
		if (cmyk[i] < GMT_CONV_LIMIT) cmyk[i] = 0.0;
	}

	/* To implement device-specific undercolor removal, supply lookup table u = UR[cmyk[3]] */
}

void gmt_cmyk_to_rgb (struct GMT_CTRL *C, double rgb[], double cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* CMYK is in 0-1, RGB will be in 0-1 range */

	int i;

	rgb[3] = cmyk[4];	/* Pass transparency unchanged */
	for (i = 0; i < 3; i++) rgb[i] = 1.0 - cmyk[i] - cmyk[3];
}

void gmt_cmyk_to_hsv (struct GMT_CTRL *C, double hsv[], double cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* CMYK is in 0-1, RGB will be in 0-1 range */

	double rgb[4];

	gmt_cmyk_to_rgb (C, rgb, cmyk);
	gmt_rgb_to_hsv (C, rgb, hsv);
}

/*
 * Transform sRGB to CIE XYZ with the D65 white point
 *
 * Poynton, "Frequently Asked Questions About Color," page 10
 * Wikipedia: http://en.wikipedia.org/wiki/SRGB
 * Wikipedia: http://en.wikipedia.org/wiki/CIE_1931_color_space
 */
void gmt_rgb_to_xyz (struct GMT_CTRL *C, double rgb[], double xyz[])
{
	double R, G, B;
	R = INVGAMMACORRECTION(rgb[0]);
	G = INVGAMMACORRECTION(rgb[1]);
	B = INVGAMMACORRECTION(rgb[2]);
	xyz[0] = 0.4123955889674142161*R  + 0.3575834307637148171*G + 0.1804926473817015735*B;
	xyz[1] = 0.2125862307855955516*R  + 0.7151703037034108499*G + 0.07220049864333622685*B;
	xyz[2] = 0.01929721549174694484*R + 0.1191838645808485318*G + 0.9504971251315797660*B;
}

void gmt_xyz_to_rgb (struct GMT_CTRL *C, double rgb[], double xyz[])
{
	double R, G, B, min;
	R = ( 3.2406 * xyz[0] - 1.5372 * xyz[1] - 0.4986 * xyz[2]);
	G = (-0.9689 * xyz[0] + 1.8758 * xyz[1] + 0.0415 * xyz[2]);
	B = ( 0.0557 * xyz[0] - 0.2040 * xyz[1] + 1.0570 * xyz[2]);
	
	min = MIN(MIN(R, G), B);
	
	/* Force nonnegative values so that gamma correction is well-defined. */
	if (min < 0) {
		R -= min;
		G -= min;
		B -= min;
	}

	/* Transform from RGB to R'G'B' */
	rgb[0] = GAMMACORRECTION(R);
	rgb[1] = GAMMACORRECTION(G);
	rgb[2] = GAMMACORRECTION(B);
}

/**
 * Convert CIE XYZ to CIE L*a*b* (CIELAB) with the D65 white point
 *
 * Wikipedia: http://en.wikipedia.org/wiki/Lab_color_space
 */

void gmt_xyz_to_lab (struct GMT_CTRL *C, double xyz[], double lab[])
{
	double X, Y, Z;
	X = LABF( xyz[0] / WHITEPOINT_X );
	Y = LABF( xyz[1] / WHITEPOINT_Y );
	Z = LABF( xyz[2] / WHITEPOINT_Z );
	lab[0] = 116 * Y - 16;
	lab[1] = 500 * (X - Y);
	lab[2] = 200 * (Y - Z);
}

void gmt_lab_to_xyz (struct GMT_CTRL *C, double xyz[], double lab[])
{	
	xyz[0] = WHITEPOINT_X * LABINVF( lab[0] + lab[1]/500 );
	xyz[1] = WHITEPOINT_Y * LABINVF( (lab[0] + 16)/116 );
	xyz[2] = WHITEPOINT_Z * LABINVF( lab[0] - lab[2]/200 );
}

void gmt_rgb_to_lab (struct GMT_CTRL *C, double rgb[], double lab[])
{
	/* RGB is in 0-1, LAB will be in ??? range */

	double xyz[3];

	gmt_rgb_to_xyz (C, rgb, xyz);
	gmt_xyz_to_lab (C, xyz, lab);
}

void gmt_lab_to_rgb (struct GMT_CTRL *C, double rgb[], double lab[])
{
	double xyz[3];
	gmt_lab_to_xyz (C, xyz, lab);
	gmt_xyz_to_rgb (C, rgb, xyz);
}

#define gmt_is_fill(C,word) (!strcmp(word,"-") || gmt_is_pattern (word) || gmt_is_color (C, word))

GMT_LONG GMT_get_prime_factors (struct GMT_CTRL *C, GMT_LONG n, GMT_LONG *f)
{
	/* Fills the integer array f with the prime factors of n.
	 * Returns the number of locations filled in f, which is
	 * one if n is prime.
	 *
	 * f[] should have been malloc'ed to enough space before
	 * calling prime_factors().  We can be certain that f[32]
	 * is enough space, for if n fits in a long, then n < 2**32,
	 * and so it must have fewer than 32 prime factors.  I think
	 * that in general, ceil(log2((double)n)) is enough storage
	 * space for f[].
	 *
	 * Tries 2,3,5 explicitly; then alternately adds 2 or 4
	 * to the previously tried factor to obtain the next trial
	 * factor.  This is done with the variable two_four_toggle.
	 * With this method we try 7,11,13,17,19,23,25,29,31,35,...
	 * up to a maximum of sqrt(n).  This shortened list results
	 * in 1/3 fewer divisions than if we simply tried all integers
	 * between 5 and sqrt(n).  We can reduce the size of the list
	 * of trials by an additional 20% by removing the multiples
	 * of 5, which are equal to 30m +/- 5, where m >= 1.  Starting
	 * from 25, these are found by alternately adding 10 or 20.
	 * To do this, we use the variable ten_twenty_toggle.
	 *
	 * W. H. F. Smith, 26 Feb 1992, after D.E. Knuth, vol. II  */

	GMT_LONG current_factor = 0;	/* The factor currently being tried  */
	GMT_LONG max_factor;		/* Don't try any factors bigger than this  */
	GMT_LONG n_factors = 0;		/* Returned; one if n is prime  */
	GMT_LONG two_four_toggle = 0;	/* Used to add 2 or 4 to get next trial factor  */
	GMT_LONG ten_twenty_toggle = 0;	/* Used to add 10 or 20 to skip_five  */
	GMT_LONG skip_five = 25;	/* Used to skip multiples of 5 in the list  */
	GMT_LONG base_factor[3] = {2, 3, 5};	/* Standard factors to try */
	GMT_LONG m;			/* Used to keep a working copy of n  */
	GMT_LONG k;			/* Counter */

	/* Initialize m and max_factor  */

	m = GMT_abs (n);
	if (m < 2) return (0);
	max_factor = (GMT_LONG)floor(sqrt((double)m));

	/* First find the 2s, 3s, and 5s */
	for (k = 0; k < 3; k++) {
		current_factor = base_factor[k];
		while (!(m % current_factor)) {
			m /= current_factor;
			f[n_factors++] = current_factor;
		}
		if (m == 1) return (n_factors);
	}

	/* Unless we have already returned we now try all the rest  */

	while (m > 1 && current_factor <= max_factor) {

		/* Current factor is either 2 or 4 more than previous value  */

		if (two_four_toggle) {
			current_factor += 4;
			two_four_toggle = 0;
		}
		else {
			current_factor += 2;
			two_four_toggle = 1;
		}

		/* If current factor is a multiple of 5, skip it.  But first,
			set next value of skip_five according to 10/20 toggle:  */

		if (current_factor == skip_five) {
			if (ten_twenty_toggle) {
				skip_five += 20;
				ten_twenty_toggle = 0;
			}
			else {
				skip_five += 10;
				ten_twenty_toggle = 1;
			}
			continue;
		}

		/* Get here when current_factor is not a multiple of 2,3 or 5:  */

		while (!(m % current_factor)) {
			m /= current_factor;
			f[n_factors++] = current_factor;
		}
	}

	/* Get here when all factors up to floor(sqrt(n)) have been tried.  */

	if (m > 1) f[n_factors++] = (GMT_LONG)m;	/* m is an additional prime factor of n  */

	return (n_factors);
}

/* These BLK functions are used in both blockmedian and blockmode and are
 * thus defined here to avoid duplication of code.
 * They are not used anywhere else.  Prototypes are listed in both
 * main programs. [PW, 25-MAR-2006].
 * 64-bit Ready.
 */

enum GMT_enum_blocks {BLK_X = 0, 
	BLK_Y	= 1,
	BLK_Z	= 2};

struct BLK_DATA {
	double	 a[4];	/* a[0] = x, a[1] = y, a[2] = z, a[3] = w  */
	GMT_LONG i;	/* Index to data value */
};

/* Sort on index, then the specified item a[0,1,2] = x, y, z */
int BLK_compare_sub (const void *point_1, const void *point_2, int item)
{
	struct BLK_DATA *p1 = (struct BLK_DATA *)point_1, *p2 = (struct BLK_DATA *)point_2;

	/* First sort on bin index i */
	if (p1->i < p2->i) return (-1);
	if (p1->i > p2->i) return (+1);
	/* OK, comparing values in the same bin */
	if (p1->a[item] < p2->a[item]) return (-1);
	if (p1->a[item] > p2->a[item]) return (+1);
	/* Values are the same, return 0 */
	return (0);
}

/* Sort on index, then x */
int BLK_compare_x (const void *point_1, const void *point_2)
{
	return (BLK_compare_sub (point_1, point_2, BLK_X));
}

/* Sort on index, then y */
int BLK_compare_y (const void *point_1, const void *point_2)
{
	return (BLK_compare_sub (point_1, point_2, BLK_Y));
}

/* Sort on index, then z */
int BLK_compare_index_z (const void *point_1, const void *point_2)
{
	return (BLK_compare_sub (point_1, point_2, BLK_Z));
}

int gmt_comp_double_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	GMT_LONG bad_1, bad_2;
	double *point_1 = (double *)p_1, *point_2 = (double *)p_2;

	bad_1 = GMT_is_dnan ((*point_1));
	bad_2 = GMT_is_dnan ((*point_2));

	if (bad_1 && bad_2) return (0);
	if (bad_1) return (+1);
	if (bad_2) return (-1);

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_float_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	GMT_LONG bad_1, bad_2;
	float *point_1 = (float *)p_1, *point_2 = (float *)p_2;

	bad_1 = GMT_is_fnan ((*point_1));
	bad_2 = GMT_is_fnan ((*point_2));

	if (bad_1 && bad_2) return (0);
	if (bad_1) return (+1);
	if (bad_2) return (-1);

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_ulong_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	GMT_ULONG *point_1 = (GMT_ULONG *)p_1, *point_2 = (GMT_ULONG *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_long_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	GMT_LONG *point_1 = (GMT_LONG *)p_1, *point_2 = (GMT_LONG *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_uint_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	unsigned int *point_1 = (unsigned int *)p_1, *point_2 = (unsigned int *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_int_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	int *point_1 = (int *)p_1, *point_2 = (int *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_ushort_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	unsigned short int *point_1 = (unsigned short int *)p_1, *point_2 = (unsigned short int *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_short_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	short int *point_1 = (short int *)p_1, *point_2 = (short int *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_uchar_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	unsigned char *point_1 = (unsigned char *)p_1, *point_2 = (unsigned char *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

int gmt_comp_char_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	char *point_1 = (char *)p_1, *point_2 = (char *)p_2;

	if ((*point_1) < (*point_2)) return (-1);
	if ((*point_1) > (*point_2)) return (+1);
	return (0);
}

void GMT_sort_array (struct GMT_CTRL *C, void *base, GMT_LONG n, GMT_LONG type)
{	/* Front function to call qsort on all <type> array into ascending order */
	size_t width[GMTAPI_N_TYPES] = {
		sizeof(unsigned char),		/* GMTAPI_CHAR */
		sizeof(char),			/* GMTAPI_USHORT */
		sizeof(unsigned short int),	/* GMTAPI_SHORT */
		sizeof(short int),		/* GMTAPI_UINT */
		sizeof(unsigned int),		/* GMTAPI_INT */
		sizeof(int),			/* GMTAPI_LONG */
		sizeof(GMT_ULONG),		/* GMTAPI_ULONG */
		sizeof(GMT_LONG),		/* GMTAPI_FLOAT */
		sizeof(float),			/* GMTAPI_DOUBLE */
		sizeof(double)};		/* GMTAPI_TEXT */
	PFI compare[GMTAPI_N_TYPES] = {
		gmt_comp_uchar_asc,		/* GMTAPI_CHAR */
		gmt_comp_char_asc,		/* GMTAPI_USHORT */
		gmt_comp_ushort_asc,		/* GMTAPI_SHORT */
		gmt_comp_short_asc,		/* GMTAPI_UINT */
		gmt_comp_uint_asc,		/* GMTAPI_INT */
		gmt_comp_int_asc,		/* GMTAPI_LONG */
		gmt_comp_ulong_asc,		/* GMTAPI_ULONG */
		gmt_comp_long_asc,		/* GMTAPI_FLOAT */
		gmt_comp_float_asc,		/* GMTAPI_DOUBLE */
		gmt_comp_double_asc};		/* GMTAPI_TEXT */
		
	qsort (base, (size_t)n, width[type], compare[type]);
}

const char * GMT_strerror (GMT_LONG err)
{
	/* Returns the error string for a given error code "err"
	 * Passes "err" on to nc_strerror if the error code is not one we defined */
	switch (err) {
		case GMT_GRDIO_FILE_NOT_FOUND:
			return "Could not find file";
		case GMT_GRDIO_OPEN_FAILED:
			return "Could not open file";
		case GMT_GRDIO_CREATE_FAILED:
			return "Could not create file";
		case GMT_GRDIO_STAT_FAILED:
			return "Could not stat file";
		case GMT_GRDIO_READ_FAILED:
			return "Could not read from file";
		case GMT_GRDIO_WRITE_FAILED:
			return "Could not write to file";
		case GMT_GRDIO_SEEK_FAILED:
			return "Failed to fseek on file";
		case GMT_GRDIO_NOT_RAS:
			return "Not a Sun raster file";
		case GMT_GRDIO_NOT_8BIT_RAS:
			return "Not a standard 8-bit Sun raster file";
		case GMT_GRDIO_PIPE_CODECHECK:
			return "Cannot guess grid format type if grid is passed via pipe";
		case GMT_GRDIO_UNKNOWN_FORMAT:
			return "Not a supported grid format";
		case GMT_GRDIO_UNKNOWN_ID:
			return "Unknown grid format id number";
		case GMT_GRDIO_UNKNOWN_TYPE:
			return "Unknown grid data type";
		case GMT_GRDIO_NOT_SURFER:
			return "Not a valid Surfer 6|7 grid file";
		case GMT_GRDIO_SURF7_UNSUPPORTED:
			return "This Surfer 7 format (full extent or with break lines) is not supported";
		case GMT_GRDIO_BAD_IMG_LAT:
			return "Must specify max latitude for img file";
		case GMT_GRDIO_DOMAIN_VIOLATION:
			return "Tried to read beyond grid domain";
		case GMT_GRDIO_NO_2DVAR:
			return "No 2-D variable in file";
		case GMT_GRDIO_NO_VAR:
			return "Named variable does not exist in file";
		case GMT_GRDIO_BAD_DIM:
			return "Named variable is not 2-, 3-, 4- or 5-D";
		case GMT_GRDIO_NC_NO_PIPE:
			return "NetCDF-based I/O does not support piping";
		case GMT_GRDIO_GRD98_XINC:
			return "GRD98 format requires n = 1/x_inc to be an integer";
		case GMT_GRDIO_GRD98_BADMAGIC:
			return "GRD98 grid file has wrong magic number";
		case GMT_GRDIO_GRD98_BADLENGTH:
			return "GRD98 grid file has wrong length";
		case GMT_GRDIO_RI_OLDBAD:
			return "Use grdedit -A on your grid file to make region and increments compatible";
		case GMT_GRDIO_RI_NEWBAD:
			return "Please select compatible -R and -I values";
		case GMT_GRDIO_BAD_XINC:
			return "Grid x increment <= 0.0";
		case GMT_GRDIO_BAD_XRANGE:
			return "Subset x range <= 0.0";
		case GMT_GRDIO_BAD_YINC:
			return "Grid y increment <= 0.0";
		case GMT_GRDIO_BAD_YRANGE:
			return "Subset y range <= 0.0";
		case GMT_GRDIO_RI_NOREPEAT:
			return "Pixel format grids do not have repeating rows or columns";
		case GMT_IO_BAD_PLOT_DEGREE_FORMAT:
			return "Unacceptable PLOT_DEGREE_FORMAT template. A not allowed";
		case GMT_CHEBYSHEV_NEG_ORDER:
			return "GMT_chebyshev given negative degree";
		case GMT_CHEBYSHEV_BAD_DOMAIN:
			return "GMT_chebyshev given |x| > 1";
		case GMT_MAP_EXCEEDS_360:
			return "Map region exceeds 360 degrees";
		case GMT_MAP_BAD_ELEVATION_MIN:
			return "\"South\" (min elevation) is outside 0-90 degree range";
		case GMT_MAP_BAD_ELEVATION_MAX:
			return "\"North\" (max elevation) is outside 0-90 degree range";
		case GMT_MAP_BAD_LAT_MIN:
			return "South is outside -90 to +90 degree range";
		case GMT_MAP_BAD_LAT_MAX:
			return "North is outside -90 to +90 degree range";
		case GMT_MAP_NO_REGION:
			return "No map region selected";
		case GMT_MAP_NO_PROJECTION:
			return "No projection selected";
		case GMT_MAP_BAD_DIST_FLAG:
			return "Wrong flag passed to GMT_dist_array";
		case GMT_MAP_BAD_MEASURE_UNIT:
			return "Bad measurement unit.  Choose among " GMT_DIM_UNITS_DISPLAY;
		default:	/* default passes through to NC error */
			return nc_strerror ((int)err);
	}
}

GMT_LONG GMT_err_func (struct GMT_CTRL *C, GMT_LONG err, GMT_LONG fail, char *file, char *fname, int line)
{
	if (err == GMT_NOERROR) return (err);

	/* When error code is non-zero: print error message */
	if (GMT_MSG_FATAL > C->current.setting.verbose)
		{ /* Stay silent */ }
	else if (file && file[0])
#ifdef DEBUG
		GMT_message (C, "%s:%s:%d: %s [%s]\n", C->init.progname, fname, line, GMT_strerror(err), file);
#else
		GMT_message (C, "%s: %s [%s]\n", C->init.progname, GMT_strerror(err), file);
#endif
	else
#ifdef DEBUG
		GMT_message (C, "%s:%s:%d: %s\n", C->init.progname, fname, line, GMT_strerror(err));
#else
		GMT_message (C, "%s: %s\n", C->init.progname, GMT_strerror(err));
#endif
	/* Pass error code on or exit */
	if (fail)
		GMT_exit (EXIT_FAILURE);
	else
		return (err);
}

GMT_LONG gmt_check_irgb (int irgb[], double rgb[])
{
	if ((irgb[0] < 0 || irgb[0] > 255) || (irgb[1] < 0 || irgb[1] > 255) || (irgb[2] < 0 || irgb[2] > 255)) return (TRUE);
	rgb[0] = GMT_is255 (irgb[0]);
	rgb[1] = GMT_is255 (irgb[1]);
	rgb[2] = GMT_is255 (irgb[2]);
	return (FALSE);
}

GMT_LONG gmt_check_rgb (double rgb[])
{
	return ((rgb[0] < 0.0 || rgb[0] > 1.0) || (rgb[1] < 0.0 || rgb[1] > 1.0) || (rgb[2] < 0.0 || rgb[2] > 1.0));
}

GMT_LONG gmt_check_hsv (struct GMT_CTRL *C, double hsv[])
{
	return ((hsv[0] < 0.0 || hsv[0] > 360.0) || (hsv[1] < 0.0 || hsv[1] > 1.0) || (hsv[2] < 0.0 || hsv[2] > 1.0));
}

GMT_LONG gmt_check_cmyk (double cmyk[])
{
	int i;
	for (i = 0; i < 4; i++) cmyk[i] *= 0.01;
	for (i = 0; i < 4; i++) if (cmyk[i] < 0.0 || cmyk[i] > 1.0) return (TRUE);
	return (FALSE);
}

void GMT_init_fill (struct GMT_CTRL *C, struct GMT_FILL *fill, double r, double g, double b)
{	/* Initialize FILL structure */

	/* Set whole structure to null (0, 0.0) */
	GMT_memset (fill, 1, struct GMT_FILL);
	/* Non-null values: */
	fill->b_rgb[0] = fill->b_rgb[1] = fill->b_rgb[2] = 1.0;
	fill->rgb[0] = r, fill->rgb[1] = g, fill->rgb[2] = b;
}

GMT_LONG GMT_getfill (struct GMT_CTRL *C, char *line, struct GMT_FILL *fill)
{
	GMT_LONG n, end, pos, i, error = 0;
	double fb_rgb[4];
	char f, word[GMT_TEXT_LEN256];

	if (!line) { GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getfill\n"); GMT_exit (EXIT_FAILURE); }

	/* Syntax:   -G<gray>, -G<rgb>, -G<cmyk>, -G<hsv> or -Gp|P<dpi>/<image>[:F<rgb>B<rgb>]   */
	/* Note, <rgb> can be r/g/b, gray, or - for masks.  optionally, append @<transparency> [0] */

	GMT_init_fill (C, fill, -1.0, -1.0, -1.0);	/* Initialize fill structure */
	GMT_chop (C, line);	/* Remove trailing CR, LF and properly NULL-terminate the string */
	if (!line[0]) return (FALSE);	/* No argument given: we are out of here */

	if ((line[0] == 'p' || line[0] == 'P') && isdigit((int)line[1])) {	/* Image specified */
		n = sscanf (&line[1], "%" GMT_LL "d/%s", &fill->dpi, fill->pattern);
		if (n != 2) error = 1;
		for (i = 0, pos = -1; fill->pattern[i] && pos == -1; i++) if (fill->pattern[i] == ':') pos = i;
		if (pos > -1) fill->pattern[pos] = '\0';
		fill->pattern_no = atoi (fill->pattern);
		if (fill->pattern_no == 0) fill->pattern_no = -1;
		fill->use_pattern = TRUE;

		/* See if fore- and background colors are given */

		for (i = 0, pos = -1; line[i] && pos == -1; i++) if (line[i] == ':') pos = i;
		pos++;

		if (pos > 0 && line[pos]) {	/* Gave colors */
			while (line[pos]) {
				f = line[pos++];
				if (line[pos] == '-')	/* Signal for transpacency masking */
					fb_rgb[0] = fb_rgb[1] = fb_rgb[2] = -1,	fb_rgb[3] = 0;
				else {
					end = pos;
					while (line[end] && !(line[end] == 'F' || line[end] == 'B')) end++;
					strncpy (word, &line[pos], (size_t)(end - pos));
					word[end - pos] = '\0';
					if (GMT_getrgb (C, word, fb_rgb)) {
						GMT_report (C, GMT_MSG_FATAL, "Colorizing value %s not recognized!\n", word);
						GMT_exit (EXIT_FAILURE);
					}
				}
				if (f == 'f' || f == 'F')
					GMT_rgb_copy (fill->f_rgb, fb_rgb);
				else if (f == 'b' || f == 'B')
					GMT_rgb_copy (fill->b_rgb, fb_rgb);
				else {
					GMT_report (C, GMT_MSG_FATAL, "Colorizing argument %c not recognized!\n", f);
					GMT_exit (EXIT_FAILURE);
				}
				while (line[pos] && !(line[pos] == 'F' || line[pos] == 'B')) pos++;
			}
		}

		/* If inverse, simply flip the colors around */
		if (line[0] == 'p') {
			GMT_rgb_copy (fb_rgb, fill->f_rgb);
			GMT_rgb_copy (fill->f_rgb, fill->b_rgb);
			GMT_rgb_copy (fill->b_rgb, fb_rgb);
		}
	}
	else {	/* Plain color or shade */
		error = GMT_getrgb (C, line, fill->rgb);
		fill->use_pattern = FALSE;
	}

	return (error);
}

GMT_LONG gmt_char_count (char *txt, char c)
{
	int i = 0, n = 0;
	while (txt[i]) if (txt[i++] == c) n++;
	return (n);
}

GMT_LONG GMT_getrgb_index (struct GMT_CTRL *C, double rgb[])
{
	/* Find the index of preset RGB triplets (those with names)
	   Return -1 if none found */

	int i;
	unsigned char irgb[3];

	/* First convert from 0-1 to 0-255 range */
	for (i = 0; i < 3; i++) irgb[i] = GMT_u255(rgb[i]);

	/* Compare all RGB codes */
	for (i = 0; i < GMT_N_COLOR_NAMES; i++) {
		if (GMT_color_rgb[i][0] == irgb[0] && GMT_color_rgb[i][1] == irgb[1] && GMT_color_rgb[i][2] == irgb[2]) return (i);
	}

	/* Nothing found */
	return (-1);
}

GMT_LONG GMT_colorname2index (struct GMT_CTRL *C, char *name)
{
	/* Return index into structure with colornames and r/g/b */

	GMT_LONG k;
	char Lname[GMT_TEXT_LEN64];

	strcpy (Lname, name);
	GMT_str_tolower (Lname);
	k = GMT_hash_lookup (C, Lname, C->session.rgb_hashnode, GMT_N_COLOR_NAMES, GMT_N_COLOR_NAMES);

	return (k);
}

GMT_LONG GMT_getrgb (struct GMT_CTRL *C, char *line, double rgb[])
{
	int n, i, count, irgb[3];
	double hsv[4], cmyk[5];
	char buffer[GMT_TEXT_LEN64], *t = NULL;

	if (!line) {
		GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getrgb\n");
		GMT_exit (EXIT_FAILURE);
	}
	if (!line[0]) return (FALSE);	/* Nothing to do - accept default action */

	rgb[3] = hsv[3] = cmyk[4] = 0.0;	/* Default is no transparency */
	if (line[0] == '-') {
		rgb[0] = -1.0; rgb[1] = -1.0; rgb[2] = -1.0;
		return (FALSE);
	}

	strcpy (buffer, line);	/* Make local copy */
	if ((t = strstr (buffer, "@")) && strlen (t) > 1) {	/* User requested transparency via @<transparency> */
		double transparency = atof (&t[1]);
		if (transparency < 0.0 || transparency > 100.0)
			GMT_report (C, GMT_MSG_FATAL, "Representation of transparency (%s) not recognized. Using default [0 or opaque].\n", line);
		else
			rgb[3] = hsv[3] = cmyk[4] = transparency / 100.0;	/* Transparency is in 0-1 range */
		t[0] = '\0';	/* Chop off transparency for the rest of this function */
	}
	if (buffer[0] == '#') {	/* #rrggbb */
		n = sscanf (buffer, "#%2x%2x%2x", (unsigned int *)&irgb[0], (unsigned int *)&irgb[1], (unsigned int *)&irgb[2]);
		return (n != 3 || gmt_check_irgb (irgb, rgb));
	}

	/* If it starts with a letter, then it could be a name */

	if (isalpha ((unsigned char) buffer[0])) {
		if ((n = (int)GMT_colorname2index (C, buffer)) < 0) {
			GMT_report (C, GMT_MSG_FATAL, "Colorname %s not recognized!\n", buffer);
			return (TRUE);
		}
		for (i = 0; i < 3; i++) rgb[i] = GMT_is255 (GMT_color_rgb[n][i]);
		return (FALSE);
	}

	/* Definitely wrong, at this point, is something that does not end in a number */

	if (!isdigit((unsigned char) buffer[strlen(buffer)-1])) return (TRUE);

	count = (int)gmt_char_count (buffer, '/');

	if (count == 3) {	/* c/m/y/k */
		n = sscanf (buffer, "%lf/%lf/%lf/%lf", &cmyk[0], &cmyk[1], &cmyk[2], &cmyk[3]);
		if (n != 4 || gmt_check_cmyk (cmyk)) return (TRUE);
		gmt_cmyk_to_rgb (C, rgb, cmyk);
		return (FALSE);
	}

	if (count == 2) {	/* r/g/b or h/s/v */
		if (C->current.setting.color_model & GMT_HSV) {	/* h/s/v */
			n = sscanf (buffer, "%lf/%lf/%lf", &hsv[0], &hsv[1], &hsv[2]);
			if (n != 3 || gmt_check_hsv (C, hsv)) return (TRUE);
			gmt_hsv_to_rgb (C, rgb, hsv);
			return (FALSE);
		}
		else {		/* r/g/b */
			n = sscanf (buffer, "%lf/%lf/%lf", &rgb[0], &rgb[1], &rgb[2]);
			rgb[0] /= 255.0 ; rgb[1] /= 255.0 ; rgb[2] /= 255.0;
			return (n != 3 || gmt_check_rgb (rgb));
		}
	}

	if (gmt_char_count (buffer, '-') == 2) {		/* h-s-v */
		n = sscanf (buffer, "%lf-%lf-%lf", &hsv[0], &hsv[1], &hsv[2]);
		if (n != 3 || gmt_check_hsv (C, hsv)) return (TRUE);
		gmt_hsv_to_rgb (C, rgb, hsv);
		return (FALSE);
	}

	if (count == 0) {	/* gray */
		n = sscanf (buffer, "%lf", &rgb[0]);
		rgb[0] /= 255.0 ; rgb[1] = rgb[2] = rgb[0];
		return (n != 1 || gmt_check_rgb (rgb));
	}

	/* Get here if there is a problem */

	return (TRUE);
}

GMT_LONG gmt_gethsv (struct GMT_CTRL *C, char *line, double hsv[])
{
	int n, i, count, irgb[3];
	double rgb[4], cmyk[5];
	char buffer[GMT_TEXT_LEN64], *t = NULL;

	if (!line) { GMT_report (C, GMT_MSG_FATAL, "No argument given to gmt_gethsv\n"); GMT_exit (EXIT_FAILURE); }
	if (!line[0]) return (FALSE);	/* Nothing to do - accept default action */

	rgb[3] = hsv[3] = cmyk[4] = 0.0;	/* Default is no transparency */
	if (line[0] == '-') {
		hsv[0] = -1.0; hsv[1] = -1.0; hsv[2] = -1.0;
		return (FALSE);
	}

	strcpy (buffer, line);	/* Make local copy */
	if ((t = strstr (buffer, "@")) && strlen (t) > 1) {	/* User requested transparency via @<transparency> */
		double transparency = atof (&t[1]);
		if (transparency < 0.0 || transparency > 100.0)
			GMT_report (C, GMT_MSG_FATAL, "Representation of transparency (%s) not recognized. Using default [0 or opaque].\n", t);
		else
			rgb[3] = hsv[3] = cmyk[4] = transparency / 100.0;	/* Transparency is in 0-1 range */
		t[0] = '\0';	/* Chop off transparency for the rest of this function */
	}

	if (buffer[0] == '#') {	/* #rrggbb */
		n = sscanf (buffer, "#%2x%2x%2x", (unsigned int *)&irgb[0], (unsigned int *)&irgb[1], (unsigned int *)&irgb[2]);
		if (n != 3 || gmt_check_irgb (irgb, rgb)) return (TRUE);
		gmt_rgb_to_hsv (C, rgb, hsv);
		return (FALSE);
	}

	/* If it starts with a letter, then it could be a name */

	if (isalpha ((unsigned char) buffer[0])) {
		if ((n = (int)GMT_colorname2index (C, buffer)) < 0) {
			GMT_report (C, GMT_MSG_FATAL, "Colorname %s not recognized!\n", buffer);
			return (TRUE);
		}
		for (i = 0; i < 3; i++) rgb[i] = GMT_is255 (GMT_color_rgb[n][i]);
		gmt_rgb_to_hsv (C, rgb, hsv);
		return (FALSE);
	}

	/* Definitely wrong, at this point, is something that does not end in a number */

	if (!isdigit ((unsigned char) buffer[strlen(buffer)-1])) return (TRUE);

	count = (int)gmt_char_count (buffer, '/');

	if (count == 3) {	/* c/m/y/k */
		n = sscanf (buffer, "%lf/%lf/%lf/%lf", &cmyk[0], &cmyk[1], &cmyk[2], &cmyk[3]);
		if (n != 4 || gmt_check_cmyk (cmyk)) return (TRUE);
		gmt_cmyk_to_hsv (C, hsv, cmyk);
		return (FALSE);
	}

	if (count == 2) {	/* r/g/b or h/s/v */
		if (C->current.setting.color_model & GMT_HSV) {	/* h/s/v */
			n = sscanf (buffer, "%lf/%lf/%lf", &hsv[0], &hsv[1], &hsv[2]);
			return (n != 3 || gmt_check_hsv (C, hsv));
		}
		else {		/* r/g/b */
			n = sscanf (buffer, "%d/%d/%d", &irgb[0], &irgb[1], &irgb[2]);
			if (n != 3 || gmt_check_irgb (irgb, rgb)) return (TRUE);
			gmt_rgb_to_hsv (C, rgb, hsv);
			return (FALSE);
		}
	}

	if (gmt_char_count (buffer, '-')  == 2) {		/* h-s-v */
		n = sscanf (buffer, "%lf-%lf-%lf", &hsv[0], &hsv[1], &hsv[2]);
		return (n != 3 || gmt_check_hsv (C, hsv));
	}

	if (count == 0) {	/* gray */
		n = sscanf (buffer, "%d", &irgb[0]);
		irgb[1] = irgb[2] = irgb[0];
		if (n != 1 || gmt_check_irgb (irgb, rgb)) return (TRUE);
		gmt_rgb_to_hsv (C, rgb, hsv);
		return (FALSE);
	}

	/* Get here if there is a problem */

	return (TRUE);
}

void GMT_enforce_rgb_triplets (struct GMT_CTRL *C, char *text, GMT_LONG size)
{
	/* Purpose is to replace things like @;lightgreen; with @r/g/b; which PSL_plottext understands.
	 * Likewise, if @transparency is appended we change it to =transparency so PSL_plottext can parse it */

	int i, j, k = 0, n, last = 0, n_slash;
	double rgb[4];
	char buffer[GMT_BUFSIZ], color[GMT_TEXT_LEN256], *p = NULL;

	if (!strchr (text, '@')) return;	/* Nothing to do since no espace sequence in string */

	while ((p = strstr (text, "@;"))) {	/* Found a @; sequence */
		i = (int)(p - text) + 2;	/* Position of first character after @; */
		for (j = last; j < i; j++, k++) buffer[k] = text[j];	/* Copy everything from last stop up to the color specification */
		text[i-1] = 'X';	/* Wipe the ; so that @; wont be found a 2nd time */
		if (text[i] != ';') {	/* Color info now follows */
			n = i;
			n_slash = 0;
			while (text[n] && text[n] != ';') {	/* Find end of the color info and also count slashes */
				if (text[n] == '/') n_slash++;
				n++;
			}
			if (n_slash != 2) {	/* r/g/b not given, must replace whatever it was with a r/g/b triplet */
				text[n] = '\0';	/* Temporarily terminate string so getrgb can work */
				GMT_getrgb (C, &text[i], rgb);
				text[n] = ';';	/* Undo damage */
				if (rgb[3] > 0.0)
					sprintf (color, "%f/%f/%f=%d", GMT_t255(rgb), irint (100.0 * rgb[3]));	/* Format triplet w/ transparency */
				else
					sprintf (color, "%f/%f/%f", GMT_t255(rgb));	/* Format triplet only */
				for (j = 0; color[j]; j++, k++) buffer[k] = color[j];	/* Copy over triplet and update buffer pointer k */
			}
			else	/* Already in r/g/b format, just copy */
				for (j = i; j < n; j++, k++) buffer[k] = text[j];
			i = n;	/* Position of terminating ; */
		}
		buffer[k++] = ';';	/* Finish the specification */
		last = i + 1;	/* Start of next part to copy */
	}
	i = last;	/* Finish copying everything left in the text string */
	while (text[i]) buffer[k++] = text[i++];
	buffer[k++] = '\0';	/* Properly terminate buffer */

	if (k > size) GMT_report (C, GMT_MSG_FATAL, "GMT_enforce_rgb_triplets: Replacement string too long - truncated\n");
	strncpy (text, buffer, (size_t)k);	/* Copy back the revised string */
}

GMT_LONG gmt_is_pattern (char *word) {
	/* Returns TRUE if the word is a pattern specification P|p<dpi>/<pattern>[:B<color>[F<color>]] */

	if (strchr (word, ':')) return (TRUE);			/* Only patterns may have a colon */
	if (!(word[0] == 'P' || word[0] == 'p')) return FALSE;	/* Patterns must start with P or p */
	if (!strchr (word, '/')) return (FALSE);		/* Patterns separate dpi and pattern with a slash */
	/* Here we know we start with P|p and there is a slash - this can only be a pattern specification */
	return (TRUE);
}

GMT_LONG gmt_is_color (struct GMT_CTRL *C, char *word)
{
	GMT_LONG i, k, n, n_hyphen = 0;

	/* Returns TRUE if we are sure the word is a color string - else FALSE.
	 * color syntax is <gray>|<r/g/b>|<h-s-v>|<c/m/y/k>|<colorname>.
	 * NOTE: we are not checking if the values are kosher; just the pattern  */

	n = strlen (word);
	if (n == 0) return (FALSE);

	if (word[0] == '#') return (TRUE);		/* Probably #rrggbb */
	if (GMT_colorname2index (C, word) >= 0) return (TRUE);	/* Valid color name */
	if (strchr(word,'t')) return (FALSE);		/* Got a t somewhere */
	if (strchr(word,':')) return (FALSE);		/* Got a : somewhere */
	if (strchr(word,'c')) return (FALSE);		/* Got a c somewhere */
	if (strchr(word,'i')) return (FALSE);		/* Got a i somewhere */
	if (strchr(word,'m')) return (FALSE);		/* Got a m somewhere */
	if (strchr(word,'p')) return (FALSE);		/* Got a p somewhere */
	for (i = k = 0; word[i]; i++) if (word[i] == '/') k++;
	if (k == 1 || k > 3) return (FALSE);	/* No color spec takes only 1 slash or more than 3 */
	n--;
	while (n >= 0 && (word[n] == '-' || word[n] == '.' || isdigit ((int)word[n]))) {
		if (word[n] == '-') n_hyphen++;
		n--;	/* Wind down as long as we find -,., or digits */
	}
	return ((n == -1 && n_hyphen == 2));	/* TRUE if we only found h-s-v and FALSE otherwise */
}

GMT_LONG gmt_getfonttype (struct GMT_CTRL *C, char *name)
{
	GMT_LONG i;

	if (!name[0]) return (-1);
	if (!isdigit ((unsigned char) name[0])) {	/* Does not start with number. Try font name */
		for (i = 0; i < C->session.n_fonts && strcmp (name, C->session.font[i].name); i++);
		return ((i == C->session.n_fonts) ? -1 : i);
	}
	if (!isdigit ((unsigned char) name[strlen(name)-1])) return (-1);	/* Starts with digit, ends with something else: cannot be */
	return (atoi (name));
}

GMT_LONG gmt_is_fontname (struct GMT_CTRL *C, char *word) {
	/* Returns TRUE if the word is one of the named fonts */
	GMT_LONG i;

	if (!word[0]) return (FALSE);
	for (i = 0; i < C->session.n_fonts && strcmp (word, C->session.font[i].name); i++);
	if (i == C->session.n_fonts) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_getfont (struct GMT_CTRL *C, char *buffer, struct GMT_FONT *F)
{
	GMT_LONG i, k, n;
	double pointsize;
	char size[GMT_TEXT_LEN256], name[GMT_TEXT_LEN256], fill[GMT_TEXT_LEN256], line[GMT_BUFSIZ], *s = NULL;

	if (!buffer) {
		GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getfont\n");
		GMT_exit (EXIT_FAILURE);
	}

	strcpy (line, buffer);	/* Work on a copy of the arguments */
	GMT_chop (C, line);	/* Remove trailing CR, LF and properly NULL-terminate the string */

	/* Processes font settings given as [size][,name][,fill][=pen] */

	GMT_memset (size, GMT_TEXT_LEN256, char);
	GMT_memset (name, GMT_TEXT_LEN256, char);
	GMT_memset (fill, GMT_TEXT_LEN256, char);

	F->form = 1;	/* Default is to fill the text with a solid color */
	if ((s = strchr (line, '='))) {	/* Specified an outline pen */
		s[0] = 0;	/* Chop of this modifier */
		if (GMT_getpen (C, &s[1], &F->pen))
			GMT_report (C, GMT_MSG_FATAL, "Representation of font outline pen not recognized - ignored.\n");
		else
			F->form |= 2;	/* Turn on outline font flag */
	}
	for (i = 0; line[i]; i++) if (line[i] == ',') line[i] = ' ';	/* Replace , with space */
	n = sscanf (line, "%s %s %s", size, name, fill);
	for (i = 0; line[i]; i++) if (line[i] == ' ') line[i] = ',';	/* Replace space with , */
	if (n == 2) {	/* Could be size,name or size,fill or name,fill */
		if (line[i-1] == ',') {		/* Must be size,name, so we can continue */
		}
		else if (line[0] == ',') {	/* ,name,fill got stored in size,name */
			strcpy (fill, name);
			strcpy (name, size);
			size[0] = '\0';
		}
		else if (gmt_is_fill (C, name)) {	/* fill got stored in name */
			strcpy (fill, name);
			name[0] = '\0';
			if (gmt_is_fontname (C, size)) {	/* name got stored in size */
				strcpy (name, size);
				size[0] = '\0';
			}
		}
	}
	else if (n == 1) {	/* Could be size or name or fill */
		if (line[0] == ',' && line[1] == ',') {	/* ,,fill got stored in size */
			strcpy (fill, size);
			size[0] = '\0';
		}
		else if (line[0] == ',') {		/* ,name got stored in size */
			strcpy (name, size);
			size[0] = '\0';
		}
		else if (gmt_is_fill (C, size)) {	/* fill got stored in size */
			strcpy (fill, size);
			size[0] = '\0';
		}
		else if (gmt_is_fontname (C, size)) {	/* name got stored in size */
			strcpy (name, size);
			size[0] = '\0';
		}
		/* Unstated else branch means we got size stored correctly */
	}
	/* Unstated else branch means we got all 3: size,name,fill */

	/* Assign font size, type, and fill, if given */
	if (!size[0] || size[0] == '-') { /* Skip */ }
	else if ((pointsize = GMT_convert_units (C, size, GMT_PT, GMT_PT)) < GMT_SMALL)
		GMT_report (C, GMT_MSG_FATAL, "Representation of font size not recognised. Using default.\n");
	else
		F->size = pointsize;
	if (!name[0] || name[0] == '-') { /* Skip */ }
	else if ((k = gmt_getfonttype (C, name)) >= 0)
		F->id = k;
	else
		GMT_report (C, GMT_MSG_FATAL, "Representation of font type not recognized. Using default.\n");
	if (!fill[0]) { /* Skip */ }
	else if (fill[0] == '-') {	/* Want no fill */
		F->form &= 2;	/* Turn off fill font flag set initially */
		GMT_rgb_copy (F->fill.rgb, C->session.no_rgb);
	}
	else {	/* Decode fill and set flags */
		if (GMT_getfill (C, fill, &F->fill)) GMT_report (C, GMT_MSG_FATAL, "Representation of font fill not recognized. Using default.\n");
		if (F->fill.use_pattern) F->form &= 2, F->form |= 4;	/* Flag that font fill is a pattern and not solid color */
	}
	if (F->form == 0) {
		GMT_report (C, GMT_MSG_FATAL, "Cannot turn off both font fill and font outline.  Reset to font fill.\n");
		F->form = 1;
	}
	return (FALSE);
}

char *GMT_putfont (struct GMT_CTRL *C, struct GMT_FONT F)
{
	/* GMT_putfont creates a GMT textstring equivalent of the specified font */

	static char text[GMT_BUFSIZ];

	if (F.form & 2)
		sprintf (text, "%gp,%s,%s=%s", F.size, C->session.font[F.id].name, GMT_putfill (C, &F.fill), GMT_putpen (C, F.pen));
	else
		sprintf (text, "%gp,%s,%s", F.size, C->session.font[F.id].name, GMT_putfill (C, &F.fill));
	return (text);
}

GMT_LONG gmt_name2pen (char *name)
{
	/* Return index into structure with pennames and width, for given name */

	GMT_LONG i, k;
	char Lname[GMT_TEXT_LEN64];

	strcpy (Lname, name);
	GMT_str_tolower (Lname);
	for (i = 0, k = -1; k < 0 && i < GMT_N_PEN_NAMES; i++) if (!strcmp (Lname, GMT_penname[i].name)) k = i;

	return (k);
}

GMT_LONG gmt_pen2name (double width)
{
	/* Return index into structure with pennames and width, for given width */

	GMT_LONG i, k;

	for (i = 0, k = -1; k < 0 && i < GMT_N_PEN_NAMES; i++) if (GMT_eq (width, GMT_penname[i].width)) k = i;

	return (k);
}

void GMT_init_pen (struct GMT_CTRL *C, struct GMT_PEN *pen, double width)
{
	/* Sets default black solid pen of given width in points */
	GMT_memset (pen, 1, struct GMT_PEN);
	pen->width = width;
}

GMT_LONG gmt_getpenwidth (struct GMT_CTRL *C, char *line, struct GMT_PEN *P) {
	GMT_LONG n;

	/* SYNTAX for pen width:  <floatingpoint>[p|i|c|m] or <name> [fat, thin, etc] */

	if (!line || !line[0]) return (GMT_NOERROR);	/* Nothing given, return */
	if ((line[0] == '.' && (line[1] >= '0' && line[1] <= '9')) || (line[0] >= '0' && line[0] <= '9')) {
		/* Pen thickness with optional unit at end */
		P->width = GMT_convert_units (C, line, GMT_PT, GMT_PT);
	}
	else {	/* Pen name was given - these refer to fixed widths in points */
		if ((n = gmt_name2pen (line)) < 0) {
			GMT_report (C, GMT_MSG_FATAL, "Pen name %s not recognized!\n", line);
			GMT_exit (EXIT_FAILURE);
		}
		P->width = GMT_penname[n].width;
	}
	return (GMT_NOERROR);
}

#if 0
/* NOT USED ?? */
GMT_LONG gmt_penunit (struct GMT_CTRL *C, char c, double *pen_scale)
{
	GMT_LONG unit;
	*pen_scale = 1.0;
	if (c == 'p')
		unit = GMT_PT;
	else if (c == 'i')
		unit = GMT_INCH;
	else if (c == 'c')
		unit = GMT_CM;
	else {	/* For pens, the default unit is dpi; must apply scaling to get inch first */
		unit = GMT_INCH;
		(*pen_scale) = 1.0 / C->PSL->init.dpi;
	}
	return (unit);
}
#endif

GMT_LONG gmt_getpenstyle (struct GMT_CTRL *C, char *line, struct GMT_PEN *P) {
	GMT_LONG i, n, pos, unit = GMT_PT;
	double width;
	char tmp[GMT_TEXT_LEN256], string[GMT_BUFSIZ], ptr[GMT_BUFSIZ];

	if (!line || !line[0]) return (GMT_NOERROR);	/* Nothing to do */
	n = strlen (line) - 1;
	if (strchr (GMT_DIM_UNITS, line[n]))	/* Separate unit given to style string */
		unit = GMT_unit_lookup (C, line[n], C->current.setting.proj_length_unit);

	width = (P->width < GMT_SMALL) ? GMT_PENWIDTH : P->width;
	if (isdigit ((int)line[0])) {	/* Specified numeric pattern will start with an integer */
		GMT_LONG c_pos;

		for (i = 1, c_pos = 0; line[i] && c_pos == 0; i++) if (line[i] == ':') c_pos = i;
		if (c_pos == 0) {
			GMT_report (C, GMT_MSG_FATAL, "Warning: Pen style %s do not follow format <pattern>:<phase>. <phase> set to 0\n", line);
			P->offset = 0.0;
		}
		else {
			line[c_pos] = ' ';
			sscanf (line, "%s %lf", P->style, &P->offset);
			line[c_pos] = ':';
		}
		for (i = 0; P->style[i]; i++) if (P->style[i] == '_') P->style[i] = ' ';

		/* Must convert given units to points */

		GMT_memset (string, GMT_BUFSIZ, char);
		pos = 0;
		while ((GMT_strtok (C, P->style, " ", &pos, ptr))) {
			sprintf (tmp, "%g ", (atof (ptr) * C->session.u2u[unit][GMT_PT]));
			strcat (string, tmp);
		}
		string[strlen (string) - 1] = 0;
		if (strlen (string) >= GMT_PEN_LEN) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Pen attributes too long!\n");
			GMT_exit (EXIT_FAILURE);
		}
		strcpy (P->style, string);
		P->offset *= C->session.u2u[unit][GMT_PT];
	}
	else  {	/* New way of building it up with - and . */
		P->style[0] = '\0';
		P->offset = 0.0;
		for (i = 0; line[i]; i++) {
			if (line[i] == '-') { /* Dash */
				sprintf (tmp, "%g %g ", 8.0 * width, 4.0 * width);
				strcat (P->style, tmp);
			}
			else if (line[i] == '.') { /* Dot */
				sprintf (tmp, "%g %g ", width, 4.0 * width);
				strcat (P->style, tmp);
			}
		}
		P->style[strlen(P->style)-1] = '\0';	/* Chop off trailing space */
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_is_penstyle (char *word)
{
	GMT_LONG n;

	/* Returns TRUE if we are sure the word is a style string - else FALSE.
	 * style syntax is a|o|<pattern>:<phase>|<string made up of -|. only>[<unit>] */

	n = strlen (word);
	if (n == 0) return (FALSE);

	n--;
	if (strchr (GMT_DIM_UNITS, word[n])) n--;	/* Reduce length by 1; the unit character */
	if (n < 0) return (FALSE);		/* word only contained a unit character? */
	if (n == 0) {
		if (word[0] == '-' || word[0] == 'a' || word[0] == '.' || word[0] == 'o') return (TRUE);
		return (FALSE);	/* No other 1-char style patterns possible */
	}
	if (strchr(word,'t')) return (FALSE);	/* Got a t somewhere */
	if (strchr(word,':')) return (TRUE);	/* Got <pattern>:<phase> */
	while (n >= 0 && (word[n] == '-' || word[n] == '.')) n--;	/* Wind down as long as we find - or . */
	return ((n == -1));	/* TRUE if we only found -/., FALSE otherwise */
}

GMT_LONG GMT_getpen (struct GMT_CTRL *C, char *buffer, struct GMT_PEN *P)
{
	GMT_LONG i, n;
	char width[GMT_TEXT_LEN256], color[GMT_TEXT_LEN256], style[GMT_TEXT_LEN256], line[GMT_BUFSIZ];

	if (!buffer || !buffer[0]) return (FALSE);		/* Nothing given: return silently, leaving P in tact */

	strcpy (line, buffer);	/* Work on a copy of the arguments */
	GMT_chop (C, line);	/* Remove trailing CR, LF and properly NULL-terminate the string */
	if (!line[0]) return (FALSE);		/* Nothing given: return silently, leaving P in tact */

	/* Processes pen specifications given as [width[<unit>][,<color>[,<style>[t<unit>]]][@<transparency>] */

	GMT_memset (width, GMT_TEXT_LEN256, char);
	GMT_memset (color, GMT_TEXT_LEN256, char);
	GMT_memset (style, GMT_TEXT_LEN256, char);
	for (i = 0; line[i]; i++) if (line[i] == ',') line[i] = ' ';	/* Replace , with space */
	n = sscanf (line, "%s %s %s", width, color, style);
	for (i = 0; line[i]; i++) if (line[i] == ' ') line[i] = ',';	/* Replace space with , */
	if (n == 2) {	/* Could be width,color or width,style or color,style */
		if (line[0] == ',') {	/* ,color,style got stored in width,color */
			strcpy (style, color);
			strcpy (color, width);
			width[0] = '\0';
		}
		else if (gmt_is_penstyle (color)) {	/* style got stored in color */
			strcpy (style, color);
			color[0] = '\0';
			if (gmt_is_color (C, width)) {	/* color got stored in width */
				strcpy (color, width);
				width[0] = '\0';
			}
		}
	}
	else if (n == 1) {	/* Could be width or color or style */
		if (line[0] == ',' && line[1] == ',') {	/* ,,style got stored in width */
			strcpy (style, width);
			width[0] = '\0';
		}
		else if (line[0] == ',') {		/* ,color got stored in width */
			strcpy (color, width);
			width[0] = '\0';
		}
		else if (gmt_is_penstyle (width)) {	/* style got stored in width */
			strcpy (style, width);
			width[0] = '\0';
		}
		else if (gmt_is_color (C, width)) {	/* color got stored in width */
			strcpy (color, width);
			width[0] = '\0';
		}
		/* Unstated else branch means we got width stored correctly */
	}
	/* Unstated else branch means we got all 3: width,color,style */

	/* Assign width, color, style if given */
	if (gmt_getpenwidth (C, width, P)) GMT_report (C, GMT_MSG_FATAL, "Representation of pen width (%s) not recognized. Using default.\n", width);
	if (GMT_getrgb (C, color, P->rgb)) GMT_report (C, GMT_MSG_FATAL, "Representation of pen color (%s) not recognized. Using default.\n", color);
	if (gmt_getpenstyle (C, style, P)) GMT_report (C, GMT_MSG_FATAL, "Representation of pen style (%s) not recognized. Using default.\n", style);

	return (FALSE);
}

char *GMT_putpen (struct GMT_CTRL *C, struct GMT_PEN pen)
{
	/* GMT_putpen creates a GMT textstring equivalent of the specified pen */

	static char text[GMT_BUFSIZ];
	GMT_LONG i, k;

	k = gmt_pen2name (pen.width);
	if (pen.style[0]) {
		if (k < 0)
			sprintf (text, "%.5gp,%s,%s:%.5gp", pen.width, GMT_putcolor (C, pen.rgb), pen.style, pen.offset);
		else
			sprintf (text, "%s,%s,%s:%.5gp", GMT_penname[k].name, GMT_putcolor (C, pen.rgb), pen.style, pen.offset);
		for (i = 0; text[i]; i++) if (text[i] == ' ') text[i] = '_';
	}
	else
		if (k < 0)
			sprintf (text, "%.5gp,%s", pen.width, GMT_putcolor (C, pen.rgb));
		else
			sprintf (text, "%s,%s", GMT_penname[k].name, GMT_putcolor (C, pen.rgb));

	return (text);
}

#if 0
/* NOT USED ?? */
GMT_LONG gmt_is_penwidth (struct GMT_CTRL *C, char *word)
{
	GMT_LONG n;

	/* Returns TRUE if we are sure the word is a penwidth string - else FALSE.
	 * width syntax is <penname> or <floatingpoint>[<unit>] */

	n = strlen (word);
	if (n == 0) return (FALSE);

	n--;
	if (strchr (GMT_DIM_UNITS, word[n])) n--;	/* Reduce length by 1; the unit character */
	if (n < 0) return (FALSE);		/* word only contained a unit character? */
	if (gmt_name2pen (word) >= 0) return (TRUE);	/* Valid pen name */
	while (n >= 0 && (word[n] == '.' || isdigit((int)word[n]))) n--;	/* Wind down as long as we find . or integers */
	return ((n == -1));	/* TRUE if we only found floating point FALSE otherwise */
}
#endif

#define GMT_INC_IS_FEET		1
#define GMT_INC_IS_M		2
#define GMT_INC_IS_KM		4
#define GMT_INC_IS_MILES	8
#define GMT_INC_IS_NMILES	16
#define GMT_INC_IS_NNODES	32
#define GMT_INC_IS_EXACT	64
#define GMT_INC_UNITS		31

GMT_LONG GMT_getinc (struct GMT_CTRL *C, char *line, double inc[])
{	/* Special case of getincn use where n is two. */

	GMT_LONG n;

	/* Syntax: -I<xinc>[m|s|e|f|k|k|M|n|+|=][/<yinc>][m|s|e|f|k|k|M|n|+|=]
	 * Units: d = arc degrees
	 * 	  m = arc minutes
	 *	  s = arc seconds [was c]
	 *	  e = meter [Convert to degrees]
	 *	  f = feet [Convert to degrees]
	 *	  M = Miles [Convert to degrees]
	 *	  k = km [Convert to degrees]
	 *	  n = nautical miles [Convert to degrees]
	 * Flags: = = Adjust -R to fit exact -I [Default modifies -I to fit -R]
	 *	  + = incs are actually nx/ny - convert to get xinc/yinc
	 */

	if (!line) { GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getinc\n"); GMT_exit (EXIT_FAILURE); }

	n = GMT_getincn (C, line, inc, 2);
	if (n == 1) {	/* Must copy y info from x */
		inc[GMT_Y] = inc[GMT_X];
		C->current.io.inc_code[GMT_Y] = C->current.io.inc_code[GMT_X];	/* Use exact inc codes for both x and y */
	}

	if (C->current.io.inc_code[GMT_X] & GMT_INC_IS_NNODES && C->current.io.inc_code[GMT_X] & GMT_INC_UNITS) {
		GMT_report (C, GMT_MSG_FATAL, "Error: number of x nodes cannot have units\n");
		GMT_exit (EXIT_FAILURE);
	}
	if (C->current.io.inc_code[GMT_Y] & GMT_INC_IS_NNODES && C->current.io.inc_code[GMT_Y] & GMT_INC_UNITS) {
		GMT_report (C, GMT_MSG_FATAL, "Error: number of y nodes cannot have units\n");
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_getincn (struct GMT_CTRL *C, char *line, double inc[], GMT_LONG n)
{
	GMT_LONG last, i, pos;
	char p[GMT_BUFSIZ];
	double scale = 1.0;

	/* Deciphers dx/dy/dz/dw/du/dv/... increment strings with n items */

	if (!line) { GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getincn\n"); GMT_exit (EXIT_FAILURE); }

	GMT_memset (inc, n, double);

	i = pos = C->current.io.inc_code[GMT_X] = C->current.io.inc_code[GMT_Y] = 0;

	while (i < n && (GMT_strtok (C, line, "/", &pos, p))) {
		last = strlen (p) - 1;
		if (p[last] == '=') {	/* Let -I override -R */
			p[last] = 0;
			if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_EXACT;
			last--;
		}
		else if (p[last] == '+' || p[last] == '!') {	/* Number of nodes given, determine inc from domain (! added since documentation mentioned this once... */
			p[last] = 0;
			if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_NNODES;
			last--;
		}
		switch (p[last]) {
			case 'd':	/* Gave arc degree */
				p[last] = 0;
				break;
			case 'm':	/* Gave arc minutes */
				p[last] = 0;
				scale = GMT_MIN2DEG;
				break;
#ifdef GMT_COMPAT
			case 'c':
				GMT_report (C, GMT_MSG_COMPAT, "Warning: Second interval unit c is deprecated; use s instead\n");
#endif
			case 's':	/* Gave arc seconds */
				p[last] = 0;
				scale = GMT_SEC2DEG;
				break;
			case 'e':	/* Gave meters along mid latitude */
				p[last] = 0;
				if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_M;
				break;
			case 'f':	/* Gave feet along mid latitude */
				p[last] = 0;
				if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_FEET;
				break;
			case 'k':	/* Gave km along mid latitude */
				p[last] = 0;
				if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_KM;
				break;
			case 'M':	/* Gave miles along mid latitude */
				p[last] = 0;
				if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_MILES;
				break;
			case 'n':	/* Gave nautical miles along mid latitude */
				p[last] = 0;
				if (i < 2) C->current.io.inc_code[i] |= GMT_INC_IS_NMILES;
				break;
			default:	/* No special flags or units */
				scale = 1.0;
				break;
		}
		if ((sscanf(p, "%lf", &inc[i])) != 1) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Unable to decode %s as a floating point number\n", p);
			GMT_exit (EXIT_FAILURE);
		}
		inc[i] *= scale;
		i++;	/* Goto next increment */
	}

	return (i);	/* Returns the number of increments found */
}

GMT_LONG GMT_get_distance (struct GMT_CTRL *C, char *line, double *dist, char *unit)
{
	/* Accepts a distance length with optional unit character.  The
	 * recognized units are:
	 * e (meter), f (feet), M (miles), n (nautical miles), k (km)
	 * and d (arc degree), m (arc minutes), s (arc seconds).
	 * If no unit is found it means Cartesian data, unless -fg is set,
	 * in which we default to meters.
	 * Passes back the radius, the unit, and returns distance_flag:
	 * flag = 0: Units are user Cartesian. Use Cartesian distances
	 * flag = 1: Unit is d|e|f|k|m|M|n|s. Use Flat-Earth distances.
	 * flag = 2: Unit is d|e|f|k|m|M|n|s. Use great-circle distances.
	 * flag = 3: Unit is d|e|f|k|m|M|n|s. Use geodesic distances.
	 * One of 2 modifiers may be prepended to the distance to control how
	 * spherical distances are computed:
	 *   - means less accurate; use Flat Earth approximation (fast).
	 *   + means more accurate; use geodesic distances (slow).
	 * Otherwise we use great circle distances (intermediate) [Default].
	 * The calling program must call GMT_init_distaz with the
	 * distance_flag and unit to set up the GMT_distance functions.
	 * Distances computed will be in the unit selected.
	 */
	GMT_LONG last, d_flag = 1, start = 1, way;
	char copy[GMT_TEXT_LEN64];

	/* Syntax:  -S[-|+]<radius>[d|e|f|k|m|M|n|s]  */

	if (!line) { GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_get_distance\n"); return (-1); }

	strcpy (copy, line);
	*dist = C->session.d_NaN;

	switch (copy[0]) {	/* Look for modifers -/+ to set how spherical distances are computed */
		case '-':	/* Want flat Earth calculations */
			way = 0;
			break;
		case '+':	/* Want geodesic distances */
			way = 2;
			break;
		default:	/* Default is great circle distances */
			way = 1;
			start = 0;
			break;
	}

	/* Extract the distance unit, if any */

	last = strlen (line) - 1;
	if (strchr (GMT_LEN_UNITS GMT_OPT("c"), (int)copy[last])) {	/* Got a valid distance unit */
		*unit = copy[last];
#ifdef GMT_COMPAT
		if (*unit == 'c') {
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Unit c is deprecated; use s instead\n");
			*unit = 's';
		}
#endif
		copy[last] = '\0';	/* Chop off the unit */
	}
	else if (!strchr ("0123456789.", (int)copy[last])) {	/* Got an invalid distance unit */
		GMT_report (C, GMT_MSG_FATAL, "Invalid distance unit (%c). Choose among %s\n", (int)copy[last], GMT_LEN_UNITS_DISPLAY);
		return (-1);
	}
	else if (start == 1 || GMT_is_geographic (C, GMT_IN))	/* Indicated a spherical calculation mode (-|+) or -fg but appended no unit; default to meter */
		*unit = GMT_MAP_DIST_UNIT;
	else {	/* Cartesian, presumably */
		*unit = 'X';
		d_flag = way = 0;
	}

	/* Get the specified length */
	if ((sscanf (&copy[start], "%lf", dist)) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Unable to decode %s as a floating point number.\n", &copy[start]);
		return (-2);
	}

	if ((*dist) < 0.0) return (-3);	/* Quietly flag a negative distance */

	return (d_flag + way);	/* Range 0-3 */
}

void GMT_RI_prepare (struct GMT_CTRL *C, struct GRD_HEADER *h)
{
	/* This routine adjusts the grid header. It computes the correct nx, ny, x_inc and y_inc,
	   based on user input of the -I option and the current settings of x_min, x_max, y_min, y_max and registration.
	   On output the grid boundaries are always gridline or pixel oriented, depending on registration.
	   The routine is not run when nx and ny are already set.
	*/
	GMT_LONG one_or_zero;
	double s;

	one_or_zero = !h->registration;
	h->xy_off = 0.5 * h->registration;	/* Use to calculate mean location of block */

	/* XINC AND XMIN/XMAX CHECK FIRST */

	/* Adjust x_inc */

	if (C->current.io.inc_code[GMT_X] & GMT_INC_IS_NNODES) {	/* Got nx */
		h->inc[GMT_X] = GMT_get_inc (C, h->wesn[XLO], h->wesn[XHI], irint(h->inc[GMT_X]), h->registration);
		GMT_report (C, GMT_MSG_VERBOSE, "Given nx implies x_inc = %g\n", h->inc[GMT_X]);
	}
	else if (C->current.io.inc_code[GMT_X] & GMT_INC_UNITS) {	/* Got funny units */
		switch (C->current.io.inc_code[GMT_X] & GMT_INC_UNITS) {
			case GMT_INC_IS_FEET:	/* feet */
				s = METERS_IN_A_FOOT;
				break;
			case GMT_INC_IS_KM:	/* km */
				s = METERS_IN_A_KM;
				break;
			case GMT_INC_IS_MILES:	/* miles */
				s = METERS_IN_A_MILE;
				break;
			case GMT_INC_IS_NMILES:	/* nmiles */
				s = METERS_IN_A_NAUTICAL_MILE;
				break;
			case GMT_INC_IS_M:	/* Meter */
			default:
				s = 1.0;
				break;
		}
		h->inc[GMT_X] *= s / (C->current.proj.DIST_M_PR_DEG * cosd (0.5 * (h->wesn[YLO] + h->wesn[YHI])));	/* Latitude scaling of E-W distances */
		GMT_report (C, GMT_MSG_VERBOSE, "Distance to degree conversion implies x_inc = %g\n", h->inc[GMT_X]);
	}
	if (!(C->current.io.inc_code[GMT_X] & (GMT_INC_IS_NNODES | GMT_INC_IS_EXACT))) {	/* Adjust x_inc to exactly fit west/east */
		s = h->wesn[XHI] - h->wesn[XLO];
		h->nx = irint (s / h->inc[GMT_X]);
		s /= h->nx;
		h->nx += (int)one_or_zero;
		if (fabs (s - h->inc[GMT_X]) > 0.0) {
			h->inc[GMT_X] = s;
			GMT_report (C, GMT_MSG_VERBOSE, "Given domain implies x_inc = %g\n", h->inc[GMT_X]);
		}
	}

	/* Determine nx */

	h->nx = GMT_grd_get_nx (C, h);

	if (C->current.io.inc_code[GMT_X] & GMT_INC_IS_EXACT) {	/* Want to keep x_inc exactly as given; adjust x_max accordingly */
		s = (h->wesn[XHI] - h->wesn[XLO]) - h->inc[GMT_X] * (h->nx - one_or_zero);
		if (fabs (s) > 0.0) {
			h->wesn[XHI] -= s;
			GMT_report (C, GMT_MSG_VERBOSE, "x_max adjusted to %g\n", h->wesn[XHI]);
		}
	}

	/* YINC AND YMIN/YMAX CHECK SECOND */

	/* Adjust y_inc */

	if (C->current.io.inc_code[GMT_Y] & GMT_INC_IS_NNODES) {	/* Got ny */
		h->inc[GMT_Y] = GMT_get_inc (C, h->wesn[YLO], h->wesn[YHI], irint(h->inc[GMT_Y]), h->registration);
		GMT_report (C, GMT_MSG_VERBOSE, "Given ny implies y_inc = %g\n", h->inc[GMT_Y]);
	}
	else if (C->current.io.inc_code[GMT_Y] & GMT_INC_UNITS) {	/* Got funny units */
		switch (C->current.io.inc_code[GMT_Y] & GMT_INC_UNITS) {
			case GMT_INC_IS_FEET:	/* feet */
				s = METERS_IN_A_FOOT;
				break;
			case GMT_INC_IS_KM:	/* km */
				s = METERS_IN_A_KM;
				break;
			case GMT_INC_IS_MILES:	/* miles */
				s = METERS_IN_A_MILE;
				break;
			case GMT_INC_IS_NMILES:	/* nmiles */
				s = METERS_IN_A_NAUTICAL_MILE;
				break;
			case GMT_INC_IS_M:	/* Meter */
			default:
				s = 1.0;
				break;
		}
		h->inc[GMT_Y] = (h->inc[GMT_Y] == 0.0) ? h->inc[GMT_X] : h->inc[GMT_Y] * s / C->current.proj.DIST_M_PR_DEG;
		GMT_report (C, GMT_MSG_VERBOSE, "Distance to degree conversion implies y_inc = %g\n", h->inc[GMT_Y]);
	}
	if (!(C->current.io.inc_code[GMT_Y] & (GMT_INC_IS_NNODES | GMT_INC_IS_EXACT))) {	/* Adjust y_inc to exactly fit south/north */
		s = h->wesn[YHI] - h->wesn[YLO];
		h->ny = irint (s / h->inc[GMT_Y]);
		s /= h->ny;
		h->ny += (int)one_or_zero;
		if (fabs (s - h->inc[GMT_Y]) > 0.0) {
			h->inc[GMT_Y] = s;
			GMT_report (C, GMT_MSG_VERBOSE, "Given domain implies y_inc = %g\n", h->inc[GMT_Y]);
		}
	}

	/* Determine ny */

	h->ny = GMT_grd_get_ny (C, h);

	if (C->current.io.inc_code[GMT_Y] & GMT_INC_IS_EXACT) {	/* Want to keep y_inc exactly as given; adjust y_max accordingly */
		s = (h->wesn[YHI] - h->wesn[YLO]) - h->inc[GMT_Y] * (h->ny - one_or_zero);
		if (fabs (s) > 0.0) {
			h->wesn[YHI] -= s;
			GMT_report (C, GMT_MSG_VERBOSE, "y_max adjusted to %g\n", h->wesn[YHI]);
		}
	}
	
	h->r_inc[GMT_X] = 1.0 / h->inc[GMT_X];
	h->r_inc[GMT_Y] = 1.0 / h->inc[GMT_Y];
}

struct GMT_PALETTE * GMT_create_palette (struct GMT_CTRL *C, GMT_LONG n_colors)
{
	/* Makes an empty palette table */
	struct GMT_PALETTE *P = NULL;
	P = GMT_memory (C, NULL, 1, struct GMT_PALETTE);
	if (n_colors > 0) P->range = GMT_memory (C, NULL, n_colors, struct GMT_LUT);
	P->n_colors = n_colors;
	P->alloc_mode = GMT_ALLOCATED;	/* So GMT_* modules can free this memory. */
	
	return (P);
}

GMT_LONG GMT_free_cpt_ptr (struct GMT_CTRL *C, struct GMT_PALETTE *P)
{
	GMT_LONG i;
	if (!P) return (GMT_NOERROR);
	/* Frees all memory used by this palette but does not free the palette itself */
	for (i = 0; i < P->n_colors; i++) {
		GMT_free (C, P->range[i].label);
		GMT_free (C, P->range[i].fill);
	}
	for (i = 0; i < 3; i++) GMT_free (C, P->patch[i].fill);
	GMT_free (C, P->range);
	/* Use free() to free the headers since they were allocated with strdup */
	for (i = 0; i < P->n_headers; i++) if (P->header[i]) free (P->header[i]);
	P->n_headers = P->n_colors = 0;
	GMT_free (C, P->header);
	return (GMT_NOERROR);
}

GMT_LONG gmt_copy_palette_hdrs (struct GMT_CTRL *C, struct GMT_PALETTE *P_to, struct GMT_PALETTE *P_from)
{
	GMT_LONG hdr;
	if (P_from->n_headers == 0) return (GMT_NOERROR);	/* Nothing to do */
	/* Must duplicate the header records */
	P_to->n_headers = P_from->n_headers;
	if (P_to->n_headers) P_to->header = GMT_memory (C, NULL, P_from->n_headers, char *);
	for (hdr = 0; hdr < P_from->n_headers; hdr++) P_to->header[hdr] = strdup (P_from->header[hdr]);
	return (GMT_NOERROR);
}

GMT_LONG GMT_copy_palette (struct GMT_CTRL *C, struct GMT_PALETTE *P_to, struct GMT_PALETTE *P_from)
{
	GMT_LONG i;
	/* Makes the specified palette the current palette */
	GMT_free_cpt_ptr (C, P_to);	/* Frees everything inside P_to */
	GMT_memcpy (P_to, P_from, 1, struct GMT_PALETTE);
	P_to->range = GMT_memory (C, NULL, P_to->n_colors, struct GMT_LUT);
	GMT_memcpy (P_to->range, P_from->range, P_to->n_colors, struct GMT_LUT);
	for (i = 0; i < 3; i++) if (P_from->patch[i].fill) {
		P_to->patch[i].fill = GMT_memory (C, NULL, 1, struct GMT_FILL);
		GMT_memcpy (P_to->patch[i].fill, P_from->patch[i].fill, 1, struct GMT_FILL);
	}
	for (i = 0; i < P_from->n_colors; i++) if (P_from->range[i].fill) {
		P_to->range[i].fill = GMT_memory (C, NULL, 1, struct GMT_FILL);
		GMT_memcpy (P_to->range[i].fill, P_from->range[i].fill, 1, struct GMT_FILL);
		if (P_from->range[i].label) P_to->range[i].label = strdup (P_from->range[i].label);
	}
	C->current.setting.color_model = P_to->model = P_from->model;
	return (gmt_copy_palette_hdrs (C, P_to, P_from));
}

GMT_LONG GMT_free_palette (struct GMT_CTRL *C, struct GMT_PALETTE **P)
{
	GMT_free_cpt_ptr (C, *P);
	GMT_free (C, *P);
	*P = NULL;
	return (GMT_NOERROR);
}

GMT_LONG GMT_list_cpt (struct GMT_CTRL *C, char option)
{	/* Adds listing of available GMT cpt choices to a program's usage message */
	FILE *fpc = NULL;
	char buffer[GMT_BUFSIZ];

	GMT_getsharepath (C, "conf", "gmt_cpt", ".conf", buffer);
	if ((fpc = fopen (buffer, "r")) == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Cannot open file %s\n", buffer);
		return (EXIT_FAILURE);
	}

	GMT_message (C, "\t-%c Specify a colortable [Default is rainbow]:\n", option);
	GMT_message (C, "\t   [Original z-range is given in brackets]\n");
	GMT_message (C, "\t   -----------------------------------------------------------------\n");
	while (fgets (buffer, GMT_BUFSIZ, fpc)) if (!(buffer[0] == '#' || buffer[0] == 0)) GMT_message (C, "\t   %s", buffer);
	GMT_message (C, "\t   -----------------------------------------------------------------\n");
	fclose (fpc);

	return (GMT_NOERROR);
}

struct GMT_PALETTE * GMT_read_cpt (struct GMT_CTRL *C, void *source, GMT_LONG source_type, GMT_LONG cpt_flags)
{
	/* Opens and reads a color palette file in RGB, HSV, or CMYK of arbitrary length.
	 * Return the result as a palette struct.
	 * source_type can be GMT_IS_[FILE|STREAM|FDESC]
	 * cpt_flags is a combination of:
	 * 1 = Suppress reading BFN (i.e. use parameter settings)
	 * 2 = Make B and F equal to low and high color
	 */

	GMT_LONG n = 0, i, nread, annot, n_alloc = GMT_SMALL_CHUNK, n_hdr_alloc = 0, color_model, id, k;
	GMT_LONG gap, error = FALSE, close_file = FALSE, check_headers = TRUE, n_cat_records = 0;
	double dz;
	char T0[GMT_TEXT_LEN64], T1[GMT_TEXT_LEN64], T2[GMT_TEXT_LEN64], T3[GMT_TEXT_LEN64], T4[GMT_TEXT_LEN64];
	char T5[GMT_TEXT_LEN64], T6[GMT_TEXT_LEN64], T7[GMT_TEXT_LEN64], T8[GMT_TEXT_LEN64], T9[GMT_TEXT_LEN64];
	char line[GMT_BUFSIZ], clo[GMT_TEXT_LEN64], chi[GMT_TEXT_LEN64], c, cpt_file[GMT_BUFSIZ];
	FILE *fp = NULL;
	struct GMT_PALETTE *X = NULL;

	/* Determine input source */

	if (source_type == GMT_IS_FILE) {	/* source is a file name */
		strcpy (cpt_file, source);
		if ((fp = fopen (cpt_file, "r")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Cannot open color palette table %s\n", cpt_file);
			return (NULL);
		}
		close_file = TRUE;	/* We only close files we have opened here */
	}
	else if (source_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)source;
		if (fp == NULL) fp = C->session.std[GMT_IN];	/* Default input */
		if (fp == C->session.std[GMT_IN])
			strcpy (cpt_file, "<stdin>");
		else
			strcpy (cpt_file, "<input stream>");
	}
	else if (source_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd = source;
		if (fd && (fp = fdopen (*fd, "r")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_read_cpt\n", *fd);
			return (NULL);
		}
		if (fd == NULL) fp = C->session.std[GMT_IN];	/* Default input */
		if (fp == C->session.std[GMT_IN])
			strcpy (cpt_file, "<stdin>");
		else
			strcpy (cpt_file, "<input file descriptor>");
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_read_cpt\n", source_type);
		return (NULL);
	}

	GMT_report (C, GMT_MSG_DEBUG, "Reading CPT table from %s\n", cpt_file);

	X = GMT_memory (C, NULL, 1, struct GMT_PALETTE);

	X->range = GMT_memory (C, NULL, n_alloc, struct GMT_LUT);
	X->cpt_flags = cpt_flags;	/* Maybe limit what to do with BFN selections */
	color_model = C->current.setting.color_model;		/* Save the original setting since it may be modified by settings in the CPT file */
	/* Also: C->current.setting.color_model is used in some rgb_to_xxx functions so it must be set if changed by cpt */
	X->is_gray = X->is_bw = TRUE;	/* May be changed when reading the actual colors */

	/* Set default BFN colors; these may be overwritten by things in the CPT file */
	for (id = 0; id < 3; id++) {
		GMT_rgb_copy (X->patch[id].rgb, C->current.setting.color_patch[id]);
		gmt_rgb_to_hsv (C, X->patch[id].rgb, X->patch[id].hsv);
		if (X->patch[id].rgb[0] == -1.0) X->patch[id].skip = TRUE;
		if (X->is_gray && !GMT_is_gray (X->patch[id].rgb)) X->is_gray = X->is_bw = FALSE;
		if (X->is_bw && !GMT_is_bw(X->patch[id].rgb)) X->is_bw = FALSE;
	}

	while (!error && fgets (line, GMT_BUFSIZ, fp)) {

		if (strstr (line, "COLOR_MODEL")) {	/* cpt file overrides default color model */
			if (strstr (line, "+RGB") || strstr (line, "rgb"))
				X->model = GMT_RGB + GMT_COLORINT;
			else if (strstr (line, "RGB"))
				X->model = GMT_RGB;
			else if (strstr (line, "+HSV") || strstr (line, "hsv"))
				X->model = GMT_HSV + GMT_COLORINT;
			else if (strstr (line, "HSV"))
				X->model = GMT_HSV;
			else if (strstr (line, "+CMYK") || strstr (line, "cmyk"))
				X->model = GMT_CMYK + GMT_COLORINT;
			else if (strstr (line, "CMYK"))
				X->model = GMT_CMYK;
			else {
				GMT_report (C, GMT_MSG_FATAL, "Error: unrecognized COLOR_MODEL in color palette table %s\n", cpt_file);
				return (NULL);
			}
		}
		C->current.setting.color_model = X->model;

		c = line[0];
		if (c == '#') {	/* Possibly a header/comment record */
			if (!check_headers) continue;	/* Done with the initial header records */
			if (n_hdr_alloc == 0) X->header = GMT_memory (C, X->header, (n_hdr_alloc = GMT_TINY_CHUNK), char *);
			X->header[X->n_headers] = strdup (line);
			X->n_headers++;
			if (X->n_headers > n_hdr_alloc) X->header = GMT_memory (C, X->header, (n_hdr_alloc += GMT_TINY_CHUNK), char *);
			continue;
		}
		if (c == '\n') continue;	/* Comment or blank */
		check_headers = FALSE;	/* First non-comment record signals the end of headers */

		T1[0] = T2[0] = T3[0] = T4[0] = T5[0] = T6[0] = T7[0] = T8[0] = T9[0] = 0;
		switch (c) {
			case 'B':
				id = GMT_BGD;
				break;
			case 'F':
				id = GMT_FGD;
				break;
			case 'N':
				id = GMT_NAN;
				break;
			default:
				id = 3;
				break;
		}

		if (id <= GMT_NAN) {	/* Foreground, background, or nan color */
			if (X->cpt_flags & 1) continue; /* Suppress parsing B, F, N lines when bit 0 of X->cpt_flags is set */
			X->patch[id].skip = FALSE;
			if ((nread = sscanf (&line[2], "%s %s %s %s", T1, T2, T3, T4)) < 1) error = TRUE;
			if (T1[0] == '-')	/* Skip this slice */
				X->patch[id].skip = TRUE;
			else if (gmt_is_pattern (T1)) {	/* Gave a pattern */
				X->patch[id].fill = GMT_memory (C, NULL, 1, struct GMT_FILL);
				if (GMT_getfill (C, T1, X->patch[id].fill)) {
					GMT_report (C, GMT_MSG_FATAL, "Error: CPT Pattern fill (%s) not understood!\n", T1);
					return (NULL);
				}
				X->has_pattern = TRUE;
			}
			else {	/* Shades, RGB, HSV, or CMYK */
				if (nread == 1)	/* Gray shade */
					sprintf (clo, "%s", T1);
				else if (X->model & GMT_CMYK)
					sprintf (clo, "%s/%s/%s/%s", T1, T2, T3, T4);
				else if (X->model & GMT_HSV)
					sprintf (clo, "%s-%s-%s", T1, T2, T3);
				else
					sprintf (clo, "%s/%s/%s", T1, T2, T3);
				if (X->model & GMT_HSV) {
					if (gmt_gethsv (C, clo, X->patch[id].hsv)) error++;
					gmt_hsv_to_rgb (C, X->patch[id].rgb, X->patch[id].hsv);
				}
				else {
					if (GMT_getrgb (C, clo, X->patch[id].rgb)) error++;
					gmt_rgb_to_hsv (C, X->patch[id].rgb, X->patch[id].hsv);
				}
				if (X->is_gray && !GMT_is_gray (X->patch[id].rgb)) X->is_gray = X->is_bw = FALSE;
				if (X->is_bw && !GMT_is_bw(X->patch[id].rgb)) X->is_bw = FALSE;
			}
			continue;
		}

		/* Here we have regular z-slices.  Allowable formats are
		 *
		 * key <fill> [;<label>]	for categorical data
		 * z0 - z1 - [LUB] [;<label>]
		 * z0 pattern z1 - [LUB] [;<label>]
		 * z0 r0 z1 r1 [LUB] [;<label>]
		 * z0 r0 g0 b0 z1 r1 g1 b1 [LUB] [;<label>]
		 * z0 h0 s0 v0 z1 h1 s1 v1 [LUB] [;<label>]
		 * z0 c0 m0 y0 k0 z1 c1 m1 y1 k1 [LUB] [;<label>]
		 *
		 * z can be in any format (float, dd:mm:ss, dateTclock)
		 */

		/* First determine if a label is given */

		if ((k = (GMT_LONG)strchr (line, ';'))) {	/* OK, find the label and chop it off */
			k -= (GMT_LONG)line;	/* Position of the column */
			X->range[n].label = GMT_memory (C, NULL, strlen (line) - k, char);
			strcpy (X->range[n].label, &line[k+1]);
			GMT_chop (C, X->range[n].label);	/* Strip off trailing return */
			line[k] = '\0';				/* Chop label off from line */
		}

		/* Determine if psscale need to label these steps by looking for the optional L|U|B character at the end */

		c = line[strlen(line)-2];
		if (c == 'L')
			X->range[n].annot = 1;
		else if (c == 'U')
			X->range[n].annot = 2;
		else if (c == 'B')
			X->range[n].annot = 3;
		if (X->range[n].annot) line[strlen(line)-2] = '\0';	/* Chop off this information so it does not affect our column count below */

		nread = sscanf (line, "%s %s %s %s %s %s %s %s %s %s", T0, T1, T2, T3, T4, T5, T6, T7, T8, T9);	/* Hope to read 4, 8, or 10 fields */

		if (nread <= 0) continue;								/* Probably a line with spaces - skip */
		if (X->model & GMT_CMYK && nread != 10) error = TRUE;			/* CMYK should results in 10 fields */
		if (!(X->model & GMT_CMYK) && !(nread == 2 || nread == 4 || nread == 8)) error = TRUE;	/* HSV or RGB should result in 8 fields, gray, patterns, or skips in 4 */
		GMT_scanf_arg (C, T0, GMT_IS_UNKNOWN, &X->range[n].z_low);
		X->range[n].skip = FALSE;
		if (T1[0] == '-') {				/* Skip this slice */
			if (nread != 4) {
				GMT_report (C, GMT_MSG_FATAL, "Error: z-slice to skip not in [z0 - z1 -] format!\n");
				return (NULL);
			}
			GMT_scanf_arg (C, T2, GMT_IS_UNKNOWN, &X->range[n].z_high);
			X->range[n].skip = TRUE;		/* Don't paint this slice if possible*/
			GMT_rgb_copy (X->range[n].rgb_low,  C->current.setting.ps_page_rgb);	/* If we must paint, use page color */
			GMT_rgb_copy (X->range[n].rgb_high, C->current.setting.ps_page_rgb);
		}
		else if (gmt_is_pattern (T1)) {	/* Gave pattern fill */
			X->range[n].fill = GMT_memory (C, NULL, 1, struct GMT_FILL);
			if (GMT_getfill (C, T1, X->range[n].fill)) {
				GMT_report (C, GMT_MSG_FATAL, "Error: CPT Pattern fill (%s) not understood!\n", T1);
				return (NULL);
			}
			else if (nread == 2) {	/* Categorical cpt records with key fill [;label] */
				X->range[n].z_high = X->range[n].z_low;
				n_cat_records++;
				X->categorical = TRUE;
			}
			else if (nread == 4) {
				GMT_scanf_arg (C, T2, GMT_IS_UNKNOWN, &X->range[n].z_high);
			}
			else {
				GMT_report (C, GMT_MSG_FATAL, "Error: z-slice with pattern fill not in [z0 pattern z1 -] format!\n");
				return (NULL);
			}
			X->has_pattern = TRUE;
		}
		else {						/* Shades, RGB, HSV, or CMYK */
			if (nread == 2) {	/* Categorical cpt records with key fill [;label] */
				X->range[n].z_high = X->range[n].z_low;
				sprintf (clo, "%s", T1);
				sprintf (chi, "-");
				n_cat_records++;
				X->categorical = TRUE;
			}
			else if (nread == 4) {	/* gray shades or color names */
				GMT_scanf_arg (C, T2, GMT_IS_UNKNOWN, &X->range[n].z_high);
				sprintf (clo, "%s", T1);
				sprintf (chi, "%s", T3);
			}
			else if (X->model & GMT_CMYK) {
				GMT_scanf_arg (C, T5, GMT_IS_UNKNOWN, &X->range[n].z_high);
				sprintf (clo, "%s/%s/%s/%s", T1, T2, T3, T4);
				sprintf (chi, "%s/%s/%s/%s", T6, T7, T8, T9);
			}
			else if (X->model & GMT_HSV) {
				GMT_scanf_arg (C, T4, GMT_IS_UNKNOWN, &X->range[n].z_high);
				sprintf (clo, "%s-%s-%s", T1, T2, T3);
				sprintf (chi, "%s-%s-%s", T5, T6, T7);
			}
			else {			/* RGB */
				GMT_scanf_arg (C, T4, GMT_IS_UNKNOWN, &X->range[n].z_high);
				sprintf (clo, "%s/%s/%s", T1, T2, T3);
				sprintf (chi, "%s/%s/%s", T5, T6, T7);
			}
			if (X->model & GMT_HSV) {
				if (gmt_gethsv (C, clo, X->range[n].hsv_low)) error++;
				if (!strcmp (chi, "-"))	/* Duplicate first color */
					GMT_memcpy (X->range[n].hsv_high, X->range[n].hsv_low, 4, double);
				else if (gmt_gethsv (C, chi, X->range[n].hsv_high)) error++;
				gmt_hsv_to_rgb (C, X->range[n].rgb_low,  X->range[n].hsv_low);
				gmt_hsv_to_rgb (C, X->range[n].rgb_high, X->range[n].hsv_high);
			}
			else {
				if (GMT_getrgb (C, clo, X->range[n].rgb_low)) error++;
				if (!strcmp (chi, "-"))	/* Duplicate first color */
					GMT_memcpy (X->range[n].rgb_high, X->range[n].rgb_low, 4, double);
				else if (GMT_getrgb (C, chi, X->range[n].rgb_high)) error++;
				gmt_rgb_to_hsv (C, X->range[n].rgb_low,  X->range[n].hsv_low);
				gmt_rgb_to_hsv (C, X->range[n].rgb_high, X->range[n].hsv_high);
			}
			if (!X->categorical) {
				dz = X->range[n].z_high - X->range[n].z_low;
				if (dz == 0.0) {
					GMT_report (C, GMT_MSG_FATAL, "Error: Z-slice with dz = 0\n");
					return (NULL);
				}
				X->range[n].i_dz = 1.0 / dz;
			}
			/* Is color map continuous, gray or b/w? */
			if (X->is_gray && !(GMT_is_gray (X->range[n].rgb_low) && GMT_is_gray (X->range[n].rgb_high))) X->is_gray = X->is_bw = FALSE;
			if (X->is_bw && !(GMT_is_bw(X->range[n].rgb_low) && GMT_is_bw(X->range[n].rgb_high))) X->is_bw = FALSE;

			/* Differences used in GMT_get_rgb_from_z */
			for (i = 0; i < 4; i++) X->range[n].rgb_diff[i] = X->range[n].rgb_high[i] - X->range[n].rgb_low[i];
			for (i = 0; i < 4; i++) X->range[n].hsv_diff[i] = X->range[n].hsv_high[i] - X->range[n].hsv_low[i];

			if (X->model & GMT_HSV) {
				if (!X->is_continuous && !GMT_same_rgb(X->range[n].hsv_low,X->range[n].hsv_high)) X->is_continuous = TRUE;
			}
			else {
				if (!X->is_continuous && !GMT_same_rgb(X->range[n].rgb_low,X->range[n].rgb_high)) X->is_continuous = TRUE;
			/* When HSV is converted from RGB: avoid interpolation over hue differences larger than 180 degrees;
			   take the shorter distance instead. This does not apply for HSV color tables, since there we assume
			   that the H values are intentional and one might WANT to interpolate over more than 180 degrees. */
				if (X->range[n].hsv_diff[0] < -180.0) X->range[n].hsv_diff[0] += 360.0;
				if (X->range[n].hsv_diff[0] >  180.0) X->range[n].hsv_diff[0] -= 360.0;
			}
		}

		n++;
		if (n == n_alloc) {
			i = n_alloc;
			n_alloc <<= 1;
			X->range = GMT_memory (C, X->range, n_alloc, struct GMT_LUT);
			GMT_memset (&X->range[i], GMT_SMALL_CHUNK, struct GMT_LUT);	/* Initialize new structs to zero */
		}
	}

	if (close_file) fclose (fp);

	if (X->cpt_flags & 2) {	/* Use low and high colors as back and foreground */
		GMT_rgb_copy (X->patch[GMT_BGD].rgb, X->range[0].rgb_low);
		GMT_rgb_copy (X->patch[GMT_FGD].rgb, X->range[n-1].rgb_high);
		GMT_rgb_copy (X->patch[GMT_BGD].hsv, X->range[0].hsv_low);
		GMT_rgb_copy (X->patch[GMT_FGD].hsv, X->range[n-1].hsv_high);
	}

	if (X->categorical && n_cat_records != n) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Cannot decode %s as categorical cpt file\n", cpt_file);
		return (NULL);
	}
	if (error) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Failed to decode %s\n", cpt_file);
		return (NULL);
	}
	if (n == 0) {
		GMT_report (C, GMT_MSG_FATAL, "Error: CPT file %s has no z-slices!\n", cpt_file);
		return (NULL);
	}

	X->range = GMT_memory (C, X->range, n, struct GMT_LUT);
	X->n_colors = n;

	if (X->categorical) {	/* Set up fake ranges so CPT is continuous */
		for (i = 0; i < X->n_colors; i++) {
			X->range[i].z_high = (i == (X->n_colors-1)) ? X->range[i].z_low + 1.0 : X->range[i+1].z_low;
			dz = X->range[i].z_high - X->range[i].z_low;
			if (dz == 0.0) {
				GMT_report (C, GMT_MSG_FATAL, "Error: Z-slice with dz = 0\n");
				return (NULL);
			}
			X->range[i].i_dz = 1.0 / dz;
		}
	}

	for (i = annot = 0, gap = FALSE; i < X->n_colors - 1; i++) {
		if (X->range[i].z_high != X->range[i+1].z_low) gap = TRUE;
		annot += X->range[i].annot;
	}
	annot += X->range[i].annot;
	if (gap) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Color palette table %s has gaps - aborts!\n", cpt_file);
		return (NULL);
	}
	if (!annot) {	/* Must set default annotation flags */
		for (i = 0; i < X->n_colors; i++) X->range[i].annot = 1;
		X->range[i-1].annot = 3;
	}

	/* Reset the color model to what it was in the GMT defaults when a + is used there. */
	if (color_model & GMT_COLORINT) C->current.setting.color_model = color_model;

	if (X->n_headers < n_hdr_alloc) X->header = GMT_memory (C, X->header, X->n_headers, char *);

	return (X);
}

void GMT_cpt_transparency (struct GMT_CTRL *C, struct GMT_PALETTE *P, double transparency, GMT_LONG mode)
{
	/* Set transparency for all slices, and possibly BNF */

	GMT_LONG i;

	for (i = 0; i < P->n_colors; i++) P->range[i].hsv_low[3] = P->range[i].hsv_high[3] = P->range[i].rgb_low[3] = P->range[i].rgb_high[3] = transparency;

	if (mode == 0) return;	/* Do not want to change transparency of BFN*/

	/* Background, foreground, and nan colors */

	for (i = 0; i < 3; i++) P->patch[i].hsv[3] = P->patch[i].rgb[3] = transparency;
}

struct GMT_PALETTE * GMT_sample_cpt (struct GMT_CTRL *C, struct GMT_PALETTE *Pin, double z[], GMT_LONG nz, GMT_LONG continuous, GMT_LONG reverse, GMT_LONG log_mode, GMT_LONG no_inter)
{
	/* Resamples the current cpt table based on new z-array.
	 * Old cpt is normalized to 0-1 range and scaled to fit new z range.
	 * New cpt may be continuous and/or reversed.
	 * We write the new cpt table to stdout. */

	GMT_LONG i, j, k, upper, lower, even = FALSE;	/* even is TRUE when nz is passed as negative */
	double rgb_low[4], rgb_high[4], rgb_fore[4], rgb_back[4];
	double *x = NULL, *z_out = NULL, a, b, f, x_inc;
	double hsv_low[4], hsv_high[4], hsv_fore[4], hsv_back[4];

	struct GMT_LUT *lut = NULL;
	struct GMT_PALETTE *P = NULL;

	GMT_check_condition (C, !Pin->is_continuous && continuous, "Warning: Making a continuous cpt from a discrete cpt may give unexpected results!\n");

	if (nz < 0) {	/* Called from grd2cpt which wants equal area colors */
		nz = -nz;
		even = TRUE;
	}

	P = GMT_create_palette (C, nz - 1);
	lut = GMT_memory (C, NULL, Pin->n_colors, struct GMT_LUT);

	GMT_check_condition (C, no_inter && P->n_colors > Pin->n_colors, "Warning: Number of picked colors exceeds colors in input cpt!\n");

	/* First normalize old cpt file so z-range is 0-1 */

	b = 1.0 / (Pin->range[Pin->n_colors-1].z_high - Pin->range[0].z_low);
	a = -Pin->range[0].z_low * b;

	for (i = 0; i < Pin->n_colors; i++) {	/* Copy/normalize cpt file and reverse if needed */
		if (reverse) {
			j = Pin->n_colors - i - 1;
			lut[i].z_low = 1.0 - a - b * Pin->range[j].z_high;
			lut[i].z_high = 1.0 - a - b * Pin->range[j].z_low;
			GMT_rgb_copy (lut[i].rgb_high, Pin->range[j].rgb_low);
			GMT_rgb_copy (lut[i].rgb_low,  Pin->range[j].rgb_high);
			GMT_rgb_copy (lut[i].hsv_high, Pin->range[j].hsv_low);
			GMT_rgb_copy (lut[i].hsv_low,  Pin->range[j].hsv_high);
		}
		else {
			j = i;
			lut[i].z_low = a + b * Pin->range[j].z_low;
			lut[i].z_high = a + b * Pin->range[j].z_high;
			GMT_rgb_copy (lut[i].rgb_high, Pin->range[j].rgb_high);
			GMT_rgb_copy (lut[i].rgb_low,  Pin->range[j].rgb_low);
			GMT_rgb_copy (lut[i].hsv_high, Pin->range[j].hsv_high);
			GMT_rgb_copy (lut[i].hsv_low,  Pin->range[j].hsv_low);
		}
	}
	lut[0].z_low = 0.0;			/* Prevent roundoff errors */
	lut[Pin->n_colors-1].z_high = 1.0;

	/* Then set up normalized output locations x */

	x = GMT_memory (C, NULL, nz, double);
	if (log_mode) {	/* Our z values are actually log10(z), need array with z for output */
		z_out = GMT_memory (C, NULL, nz, double);
		for (i = 0; i < nz; i++) z_out[i] = pow (10.0, z[i]);
	}
	else
		z_out = z;	/* Just point to the incoming z values */

	if (nz < 2)	/* Want a single color point, assume range 0-1 */
		nz = 2;
	else if (even) {
		x_inc = 1.0 / (nz - 1);
		for (i = 0; i < nz; i++) x[i] = i * x_inc;	/* Normalized z values 0-1 */
	}
	else {	/* As with LUT, translate users z-range to 0-1 range */
		b = 1.0 / (z[nz-1] - z[0]);
		a = -z[0] * b;
		for (i = 0; i < nz; i++) x[i] = a + b * z[i];	/* Normalized z values 0-1 */
	}
	x[0] = 0.0;	/* Prevent bad roundoff */
	x[nz-1] = 1.0;

	/* Determine color at lower and upper limit of each interval */

	for (i = 0; i < P->n_colors; i++) {

		lower = i;
		upper = i + 1;

		if (no_inter) { /* Just pick the first n_colors */
			j = MIN (i, Pin->n_colors);
			if (Pin->model & GMT_HSV) {	/* Pick in HSV space */
				for (k = 0; k < 4; k++) hsv_low[k] = lut[j].hsv_low[k], hsv_high[k] = lut[j].hsv_high[k];
				gmt_hsv_to_rgb (C, rgb_low, hsv_low);
				gmt_hsv_to_rgb (C, rgb_high, hsv_high);
			}
			else {	/* Pick in RGB space */
				for (k = 0; k < 4; k++) rgb_low[k] = lut[j].rgb_low[k], rgb_high[k] = lut[j].rgb_low[k];
				gmt_rgb_to_hsv (C, rgb_low, hsv_low);
				gmt_rgb_to_hsv (C, rgb_high, hsv_high);
			}
		}
		else if (continuous) { /* Interpolate color at lower and upper value */

			for (j = 0; j < Pin->n_colors && x[lower] >= lut[j].z_high; j++);
			if (j == Pin->n_colors) j--;

			f = 1.0 / (lut[j].z_high - lut[j].z_low);

			if (Pin->model & GMT_HSV) {	/* Interpolation in HSV space */
				for (k = 0; k < 4; k++) hsv_low[k] = lut[j].hsv_low[k] + (lut[j].hsv_high[k] - lut[j].hsv_low[k]) * f * (x[lower] - lut[j].z_low);
				gmt_hsv_to_rgb (C, rgb_low, hsv_low);
			}
			else {	/* Interpolation in RGB space */
				for (k = 0; k < 4; k++) rgb_low[k] = lut[j].rgb_low[k] + (lut[j].rgb_high[k] - lut[j].rgb_low[k]) * f * (x[lower] - lut[j].z_low);
				gmt_rgb_to_hsv (C, rgb_low, hsv_low);
			}

			while (j < Pin->n_colors && x[upper] > lut[j].z_high) j++;

			f = 1.0 / (lut[j].z_high - lut[j].z_low);

			if (Pin->model & GMT_HSV) {	/* Interpolation in HSV space */
				for (k = 0; k < 4; k++) hsv_high[k] = lut[j].hsv_low[k] + (lut[j].hsv_high[k] - lut[j].hsv_low[k]) * f * (x[upper] - lut[j].z_low);
				gmt_hsv_to_rgb (C, rgb_high, hsv_high);
			}
			else {	/* Interpolation in RGB space */
				for (k = 0; k < 4; k++) rgb_high[k] = lut[j].rgb_low[k] + (lut[j].rgb_high[k] - lut[j].rgb_low[k]) * f * (x[upper] - lut[j].z_low);
				gmt_rgb_to_hsv (C, rgb_high, hsv_high);
			}
		}
		else {	 /* Interpolate central value and assign color to both lower and upper limit */

			a = (x[lower] + x[upper]) / 2;
			for (j = 0; j < Pin->n_colors && a >= lut[j].z_high; j++);
			if (j == Pin->n_colors) j--;

			f = 1.0 / (lut[j].z_high - lut[j].z_low);

			if (Pin->model & GMT_HSV) {	/* Interpolation in HSV space */
				for (k = 0; k < 4; k++) hsv_low[k] = hsv_high[k] = lut[j].hsv_low[k] + (lut[j].hsv_high[k] - lut[j].hsv_low[k]) * f * (a - lut[j].z_low);
			}
			else {	/* Interpolation in RGB space */
				for (k = 0; k < 4; k++) rgb_low[k] = rgb_high[k] = lut[j].rgb_low[k] + (lut[j].rgb_high[k] - lut[j].rgb_low[k]) * f * (a - lut[j].z_low);
			}
		}

		if (lower == 0) {
			GMT_rgb_copy (rgb_back, rgb_low);
			GMT_rgb_copy (hsv_back, hsv_low);
		}
		if (upper == P->n_colors) {
			GMT_rgb_copy (rgb_fore, rgb_high);
			GMT_rgb_copy (hsv_fore, hsv_high);
		}

		GMT_rgb_copy (P->range[i].rgb_low, rgb_low);
		GMT_rgb_copy (P->range[i].rgb_high, rgb_high);
		GMT_rgb_copy (P->range[i].hsv_low, hsv_low);
		GMT_rgb_copy (P->range[i].hsv_high, hsv_high);
		P->range[i].z_low = z_out[lower];
		P->range[i].z_high = z_out[upper];
		P->is_gray = (GMT_is_gray (P->range[i].rgb_low) && GMT_is_gray (P->range[i].rgb_high));
		P->is_bw = (GMT_is_bw(P->range[i].rgb_low) && GMT_is_bw (P->range[i].rgb_high));

		/* Differences used in GMT_get_rgb_from_z */
		for (k = 0; k < 4; k++) P->range[i].rgb_diff[k] = P->range[i].rgb_high[k] - P->range[i].rgb_low[k];
		for (k = 0; k < 4; k++) P->range[i].hsv_diff[k] = P->range[i].hsv_high[k] - P->range[i].hsv_low[k];

		/* When HSV is converted from RGB: avoid interpolation over hue differences larger than 180 degrees;
		   take the shorter distance instead. This does not apply for HSV color tables, since there we assume
		   that the H values are intentional and one might WANT to interpolate over more than 180 degrees. */
		if (!(P->model & GMT_HSV)) {
			if (P->range[i].hsv_diff[0] < -180.0) P->range[i].hsv_diff[0] += 360.0;
			if (P->range[i].hsv_diff[0] >  180.0) P->range[i].hsv_diff[0] -= 360.0;
		}
		f = P->range[i].z_high - P->range[i].z_low;
		if (f == 0.0) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Z-slice with dz = 0\n");
			return (NULL);
		}
		P->range[i].i_dz = 1.0 / f;
	}

	GMT_free (C, x);
	GMT_free (C, lut);
	if (log_mode) GMT_free (C, z_out);
	P->model = Pin->model;
	P->categorical = Pin->categorical;
	P->is_continuous = continuous;

	/* Background, foreground, and nan colors */

	GMT_memcpy (P->patch, Pin->patch, 3, struct GMT_BFN_COLOR);	/* Copy over BNF */

	if (reverse) {	/* Flip foreground and background colors */
		GMT_rgb_copy (rgb_low, P->patch[GMT_BGD].rgb);
		GMT_rgb_copy (P->patch[GMT_BGD].rgb, P->patch[GMT_FGD].rgb);
		GMT_rgb_copy (P->patch[GMT_FGD].rgb, rgb_low);
		GMT_rgb_copy (hsv_low, P->patch[GMT_BGD].hsv);
		GMT_rgb_copy (P->patch[GMT_BGD].hsv, P->patch[GMT_FGD].hsv);
		GMT_rgb_copy (P->patch[GMT_FGD].hsv, hsv_low);
	}

	(void) gmt_copy_palette_hdrs (C, P, Pin);
	return (P);
}

GMT_LONG GMT_write_cpt (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, GMT_LONG cpt_flags, struct GMT_PALETTE *P)
{
	/* We write the cpt table to fpr [or stdout].
	 * dest_type can be GMT_IS_[FILE|STREAM|FDESC]
	 * cpt_flags is a combination of:
	 * 1 = Do not write BFN
	 * 2 = Make B and F equal to low and high color
	 */

	GMT_LONG i, k, close_file = FALSE;
	double cmyk[5];
	char format[GMT_BUFSIZ], cpt_file[GMT_BUFSIZ], code[3] = {'B', 'F', 'N'};
	FILE *fp = NULL;

	if (dest_type == GMT_IS_FILE && !dest) dest_type = GMT_IS_STREAM;	/* No filename given, default to stdout */

	if (dest_type == GMT_IS_FILE) {	/* dest is a file name */
		strcpy (cpt_file, dest);
		if ((fp = GMT_fopen (C, cpt_file, "w")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot create file %s\n", cpt_file);
			return (EXIT_FAILURE);
		}
		close_file = TRUE;	/* We only close files we have opened here */
	}
	else if (dest_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)dest;
		if (fp == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
		if (fp == C->session.std[GMT_OUT])
			strcpy (cpt_file, "<stdout>");
		else
			strcpy (cpt_file, "<output stream>");
	}
	else if (dest_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd = dest;
		if (fd && (fp = fdopen (*fd, "w")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_write_cpt\n", *fd);
			return (EXIT_FAILURE);
		}
		if (fd == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
		if (fp == C->session.std[GMT_OUT])
			strcpy (cpt_file, "<stdout>");
		else
			strcpy (cpt_file, "<output file descriptor>");
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_write_cpt\n", dest_type);
		return (EXIT_FAILURE);
	}
	GMT_report (C, GMT_MSG_DEBUG, "Writing CPT table to %s\n", cpt_file);

	/* Start writing cpt file info to fp */

	if (!(P->model & GMT_COLORINT)) {}	/* Write nothing when color interpolation is not forced */
	else if (P->model & GMT_HSV)
		fprintf (fp, "# COLOR_MODEL = hsv\n");
	else if (P->model & GMT_CMYK)
		fprintf (fp, "# COLOR_MODEL = cmyk\n");
	else
		fprintf (fp, "# COLOR_MODEL = rgb\n");

	sprintf (format, "%s\t%%s%%c", C->current.setting.format_float_out);

	/* Determine color at lower and upper limit of each interval */

	for (i = 0; i < P->n_colors; i++) {

		/* Print out one row */

		if (P->categorical) {
			if (P->model & GMT_HSV)
				fprintf (fp, format, P->range[i].z_low, GMT_puthsv (C, P->range[i].hsv_low), '\n');
			else if (P->model & GMT_CMYK) {
				gmt_rgb_to_cmyk (C, P->range[i].rgb_low, cmyk);
				fprintf (fp, format, P->range[i].z_low, GMT_putcmyk (C, cmyk), '\n');
			}
			else if (P->model & GMT_NO_COLORNAMES)
				fprintf (fp, format, P->range[i].z_low, GMT_putrgb (C, P->range[i].rgb_low), '\n');
			else
				fprintf (fp, format, P->range[i].z_low, GMT_putcolor (C, P->range[i].rgb_low), '\n');
		}
		else if (P->model & GMT_HSV) {
			fprintf (fp, format, P->range[i].z_low, GMT_puthsv (C, P->range[i].hsv_low), '\t');
			fprintf (fp, format, P->range[i].z_high, GMT_puthsv (C, P->range[i].hsv_high), '\n');
		}
		else if (P->model & GMT_CMYK) {
			gmt_rgb_to_cmyk (C, P->range[i].rgb_low, cmyk);
			fprintf (fp, format, P->range[i].z_low, GMT_putcmyk (C, cmyk), '\t');
			gmt_rgb_to_cmyk (C, P->range[i].rgb_high, cmyk);
			fprintf (fp, format, P->range[i].z_high, GMT_putcmyk (C, cmyk), '\n');
		}
		else if (P->model & GMT_NO_COLORNAMES) {
			fprintf (fp, format, P->range[i].z_low, GMT_putrgb (C, P->range[i].rgb_low), '\t');
			fprintf (fp, format, P->range[i].z_high, GMT_putrgb (C, P->range[i].rgb_high), '\n');
		}
		else {
			fprintf (fp, format, P->range[i].z_low, GMT_putcolor (C, P->range[i].rgb_low), '\t');
			fprintf (fp, format, P->range[i].z_high, GMT_putcolor (C, P->range[i].rgb_high), '\n');
		}
	}

	/* Background, foreground, and nan colors */

	if (cpt_flags & 1) return (EXIT_SUCCESS);	/* Do not want to write BFN to the cpt file */

	if (cpt_flags & 2) {	/* Use low and high colors as back and foreground */
		GMT_rgb_copy (P->patch[GMT_BGD].rgb, P->range[0].rgb_low);
		GMT_rgb_copy (P->patch[GMT_FGD].rgb, P->range[P->n_colors-1].rgb_high);
		GMT_rgb_copy (P->patch[GMT_BGD].hsv, P->range[0].hsv_low);
		GMT_rgb_copy (P->patch[GMT_FGD].hsv, P->range[P->n_colors-1].hsv_high);
	}

	for (k = 0; k < 3; k++) {
		if (P->patch[k].skip)
			fprintf (fp, "%c\t-\n", code[k]);
		else if (P->model & GMT_HSV)
			fprintf (fp, "%c\t%s\n", code[k], GMT_puthsv (C, P->patch[k].hsv));
		else if (P->model & GMT_CMYK) {
			gmt_rgb_to_cmyk (C, P->patch[k].rgb, cmyk);
			fprintf (fp, "%c\t%s\n", code[k], GMT_putcmyk (C, cmyk));
		}
		else if (P->model & GMT_NO_COLORNAMES)
			fprintf (fp, "%c\t%s\n", code[k], GMT_putrgb (C, P->patch[k].rgb));
		else
			fprintf (fp, "%c\t%s\n", code[k], GMT_putcolor (C, P->patch[k].rgb));
	}
	return (EXIT_SUCCESS);	/* Do not want to write BFN to the cpt file */
}

GMT_LONG GMT_get_index (struct GMT_CTRL *C, struct GMT_PALETTE *P, double value)
{
	GMT_LONG index, lo, hi, mid;

	if (GMT_is_dnan (value)) return (GMT_NAN - 3);				/* Set to NaN color */
	if (value > P->range[P->n_colors-1].z_high) return (GMT_FGD - 3);	/* Set to foreground color */
	if (value < P->range[0].z_low) return (GMT_BGD - 3);	/* Set to background color */

	/* Must search for correct index */

	/* Speedup by Mika Heiskanen. This works if the colortable
	 * has been is sorted into increasing order. Careful when
	 * modifying the tests in the loop, the test and the mid+1
	 * parts are especially designed to make sure the loop
	 * converges to a single index.
	 */

	lo = 0;
	hi = P->n_colors - 1;
	while (lo != hi)
	{
		mid = (lo + hi) / 2;
		if (value >= P->range[mid].z_high)
			lo = mid + 1;
		else
			hi = mid;
	}
	index = lo;
	if (value >= P->range[index].z_low && value < P->range[index].z_high) return (index);

	/* Slow search in case the table was not sorted
	 * No idea whether it is possible, but it most certainly
	 * does not hurt to have the code here as a backup.
	 */

	index = 0;
	while (index < P->n_colors && ! (value >= P->range[index].z_low && value < P->range[index].z_high) ) index++;
	if (index == P->n_colors) index--;	/* Because we use <= for last range */
	return (index);
}

void GMT_get_rgb_lookup (struct GMT_CTRL *C, struct GMT_PALETTE *P, GMT_LONG index, double value, double *rgb)
{
	GMT_LONG i;
	double rel, hsv[4];

	if (index < 0) {	/* NaN, Foreground, Background */
		GMT_rgb_copy (rgb, P->patch[index+3].rgb);
		P->skip = P->patch[index+3].skip;
	}
	else if (P->range[index].skip) {		/* Set to page color for now */
		GMT_rgb_copy (rgb, C->current.setting.ps_page_rgb);
		P->skip = TRUE;
	}
	else {	/* Do linear interpolation between low and high colors */
		rel = (value - P->range[index].z_low) * P->range[index].i_dz;
		if (C->current.setting.color_model == GMT_HSV + GMT_COLORINT) {	/* Interpolation in HSV space */
			for (i = 0; i < 4; i++) hsv[i] = P->range[index].hsv_low[i] + rel * P->range[index].hsv_diff[i];
			gmt_hsv_to_rgb (C, rgb, hsv);
		}
		else {	/* Interpolation in RGB space */
			for (i = 0; i < 4; i++) rgb[i] = P->range[index].rgb_low[i] + rel * P->range[index].rgb_diff[i];
		}
		P->skip = FALSE;
	}
}

GMT_LONG GMT_get_rgb_from_z (struct GMT_CTRL *C, struct GMT_PALETTE *P, double value, double *rgb)
{
	GMT_LONG index = GMT_get_index (C, P, value);
	GMT_get_rgb_lookup (C, P, index, value, rgb);
	return (index);
}

GMT_LONG GMT_get_fill_from_z (struct GMT_CTRL *C, struct GMT_PALETTE *P, double value, struct GMT_FILL *fill)
{
	GMT_LONG index;
	struct GMT_FILL *f = NULL;

	index = GMT_get_index (C, P, value);

	/* Check if pattern */

	if (index >= 0 && (f = P->range[index].fill))
		GMT_memcpy (fill, f, 1, struct GMT_FILL);
	else if (index < 0 && (f = P->patch[index+3].fill))
		GMT_memcpy (fill, f, 1, struct GMT_FILL);
	else {
		GMT_get_rgb_lookup (C, P, index, value, fill->rgb);
		fill->use_pattern = FALSE;
	}
	return (index);
}

void GMT_illuminate (struct GMT_CTRL *C, double intensity, double rgb[])
{
	double di, hsv[4];

	if (GMT_is_dnan (intensity)) return;
	if (intensity == 0.0) return;
	if (fabs (intensity) > 1.0) intensity = copysign (1.0, intensity);

	gmt_rgb_to_hsv (C, rgb, hsv);
	if (intensity > 0.0) {	/* Lighten the color */
		di = 1.0 - intensity;
		if (hsv[1] != 0.0) hsv[1] = di * hsv[1] + intensity * C->current.setting.color_hsv_max_s;
		hsv[2] = di * hsv[2] + intensity * C->current.setting.color_hsv_max_v;
	}
	else {			/* Darken the color */
		di = 1.0 + intensity;
		if (hsv[1] != 0.0) hsv[1] = di * hsv[1] - intensity * C->current.setting.color_hsv_min_s;
		hsv[2] = di * hsv[2] - intensity * C->current.setting.color_hsv_min_v;
	}
	if (hsv[1] < 0.0) hsv[1] = 0.0;
	if (hsv[2] < 0.0) hsv[2] = 0.0;
	if (hsv[1] > 1.0) hsv[1] = 1.0;
	if (hsv[2] > 1.0) hsv[2] = 1.0;
	gmt_hsv_to_rgb (C, rgb, hsv);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_akima computes the coefficients for a quasi-cubic hermite spline.
 * Same algorithm as in the IMSL library.
 * Programmer:	Paul Wessel
 * Date:	16-JAN-1987
 * Ver:		v.1-pc
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

GMT_LONG GMT_akima (struct GMT_CTRL *C, double *x, double *y, GMT_LONG nx, double *c)
{
	GMT_LONG i, no;
	double t1, t2, b, rm1, rm2, rm3, rm4;

	/* Assumes that n >= 4 and x is monotonically increasing */

	rm3 = (y[1] - y[0])/(x[1] - x[0]);
	t1 = rm3 - (y[1] - y[2])/(x[1] - x[2]);
	rm2 = rm3 + t1;
	rm1 = rm2 + t1;

	/* get slopes */

	no = nx - 2;
	for (i = 0; i < nx; i++) {
		if (i >= no)
			rm4 = rm3 - rm2 + rm3;
		else
			rm4 = (y[i+2] - y[i+1])/(x[i+2] - x[i+1]);
		t1 = fabs(rm4 - rm3);
		t2 = fabs(rm2 - rm1);
		b = t1 + t2;
		c[3*i] = (b != 0.0) ? (t1*rm2 + t2*rm3) / b : 0.5*(rm2 + rm3);
		rm1 = rm2;
		rm2 = rm3;
		rm3 = rm4;
	}
	no = nx - 1;

	/* compute the coefficients for the nx-1 intervals */

	for (i = 0; i < no; i++) {
		t1 = 1.0 / (x[i+1] - x[i]);
		t2 = (y[i+1] - y[i])*t1;
		b = (c[3*i] + c[3*i+3] - t2 - t2)*t1;
		c[3*i+2] = b*t1;
		c[3*i+1] = -b + (t2 - c[3*i])*t1;
	}
	return (GMT_NOERROR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_cspline computes the coefficients for a natural cubic spline.
 * To evaluate, call GMT_csplint
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

GMT_LONG GMT_cspline (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double *c)
{
	GMT_LONG i, k;
	double ip, s, dx1, i_dx2, *u = GMT_memory (C, NULL, n, double);

	/* Assumes that n >= 4 and x is monotonically increasing */

	c[0] = c[n-1] = 0.0;	/* The other c[i] are set directly in the loop */
	for (i = 1; i < n-1; i++) {
		i_dx2 = 1.0 / (x[i+1] - x[i-1]);
		dx1 = x[i] - x[i-1];
		s = dx1 * i_dx2;
		ip = 1.0 / (s * c[i-1] + 2.0);
		c[i] = (s - 1.0) * ip;
		u[i] = (y[i+1] - y[i]) / (x[i+1] - x[i]) - (y[i] - y[i-1]) / dx1;
		u[i] = (6.0 * u[i] * i_dx2 - s * u[i-1]) * ip;
	}
	for (k = n-2; k >= 0; k--) c[k] = c[k] * c[k+1] + u[k];
	GMT_free (C, u);

	return (GMT_NOERROR);
}

double GMT_csplint (struct GMT_CTRL *C, double *x, double *y, double *c, double xp, GMT_LONG klo)
{
	GMT_LONG khi;
	double h, ih, b, a, yp;

	khi = klo + 1;
	h = x[khi] - x[klo];
	ih = 1.0 / h;
	a = (x[khi] - xp) * ih;
	b = (xp - x[klo]) * ih;
	yp = a * y[klo] + b * y[khi] + ((a*a*a - a) * c[klo] + (b*b*b - b) * c[khi]) * (h*h) / 6.0;

	return (yp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_intpol will interpolate from the dataset <x,y> onto a new set <u,v>
 * where <x,y> and <u> is supplied by the user. <v> is returned. The
 * parameter mode governs what interpolation scheme that will be used.
 * If u[i] is outside the range of x, then v[i] will contain NaN.
 *
 * input:  x = x-values of input data
 *	   y = y-values "    "     "
 *	   n = number of data pairs
 *	   m = number of desired interpolated values
 *	   u = x-values of these points
 *	  mode = type of interpolation
 *	  mode = 0 : Linear interpolation
 *	  mode = 1 : Quasi-cubic hermite spline (GMT_akima)
 *	  mode = 2 : Natural cubic spline (cubspl)
 *        mode = 3 : No interpolation (closest point)
 * output: v = y-values at interpolated points
 * PS. v must have space allocated before calling GMT_intpol
 *
 * Programmer:	Paul Wessel
 * Date:	16-MAR-2009
 * Ver:		v.3.0
 * Now y can contain NaNs and we will interpolate within segments of clean data
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

GMT_LONG gmt_intpol_sub (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG m, double *u, double *v, GMT_LONG mode)
{	/* Does the main work of interpolating a section that has no NaNs */
	GMT_LONG i, j, err_flag = 0;
	double dx, x_min, x_max, *c = NULL;

	/* Set minimum and maximum */

	if (mode == 3) {
		x_min = (3*x[0] - x[1]) / 2;
		x_max = (3*x[n-1] - x[n-2]) / 2;
	}
	else {
		x_min = x[0]; x_max = x[n-1];
	}

	/* Allocate memory for spline factors */

	if (mode == 1) {	/* Akima's spline */
		c = GMT_memory (C, NULL, 3*n, double);
		err_flag = GMT_akima (C, x, y, n, c);
	}
	else if (mode == 2) {	/* Natural cubic spline */
		c = GMT_memory (C, NULL, 3*n, double);
		err_flag = GMT_cspline (C, x, y, n, c);
	}
	if (err_flag != 0) {
		GMT_free (C, c);
		return (err_flag);
	}

	/* Compute the interpolated values from the coefficients */

	j = 0;
	for (i = 0; i < m; i++) {
		if (u[i] < x_min || u[i] > x_max) {	/* Desired point outside data range */
			v[i] = C->session.d_NaN;
			continue;
		}
		while (x[j] > u[i] && j > 0) j--;	/* In case u is not sorted */
		while (j < n && x[j] <= u[i]) j++;
		if (j == n) j--;
		if (j > 0) j--;

		switch (mode) {
			case 0:
				dx = u[i] - x[j];
				v[i] = (y[j+1]-y[j])*dx/(x[j+1]-x[j]) + y[j];
				break;
			case 1:
				dx = u[i] - x[j];
				v[i] = ((c[3*j+2]*dx + c[3*j+1])*dx + c[3*j])*dx + y[j];
				break;
			case 2:
				v[i] = GMT_csplint (C, x, y, c, u[i], j);
				break;
			case 3:
				v[i] = (u[i] - x[j] < x[j+1] - u[i]) ? y[j] : y[j+1];
				break;
		}
	}
	GMT_free (C, c);

	return (GMT_NOERROR);
}

void gmt_intpol_reverse (double *x, double *u, GMT_LONG n, GMT_LONG m)
{	/* Changes sign on x and u */
	GMT_LONG i;
	for (i = 0; i < n; i++) x[i] = -x[i];
	for (i = 0; i < m; i++) u[i] = -u[i];
}

GMT_LONG GMT_intpol (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG m, double *u, double *v, GMT_LONG mode)
{
	GMT_LONG i, this_n, this_m, start_i, start_j, stop_i, stop_j;
	GMT_LONG err_flag = 0, down = FALSE, check = TRUE, clean = TRUE;
	double dx;

	if (mode < 0) {	/* No need to check for sanity */
		check = FALSE;
		mode = -mode;
	}

	if (mode > 3) mode = 0;
	if (mode != 3 && n < 4) mode = 0;
	if (n < 2) {
		GMT_report (C, GMT_MSG_VERBOSE, "Error: need at least 2 x-values\n");
		return (EXIT_FAILURE);
	}

	if (check) {
		/* Check to see if x-values are monotonically increasing/decreasing */

		dx = x[1] - x[0];
		if (GMT_is_dnan (y[0])) clean = FALSE;
		if (dx > 0.0) {
			for (i = 2; i < n && err_flag == 0; i++) {
				if ((x[i] - x[i-1]) <= 0.0) err_flag = i;
				if (clean && GMT_is_dnan (y[i])) clean = FALSE;
			}
		}
		else {
			down = TRUE;
			for (i = 2; i < n && err_flag == 0; i++) {
				if ((x[i] - x[i-1]) >= 0.0) err_flag = i;
				if (clean && GMT_is_dnan (y[i])) clean = FALSE;
			}
		}

		if (err_flag) {
			GMT_report (C, GMT_MSG_VERBOSE, "Error: x-values are not monotonically increasing/decreasing (at record %ld)!\n", err_flag);
			return (err_flag);
		}

	}

	if (down) gmt_intpol_reverse (x, u, n, m);	/* Must flip directions temporarily */

	if (clean) {	/* No NaNs to worry about */
		err_flag = gmt_intpol_sub (C, x, y, n, m, u, v, mode);
		if (err_flag != GMT_NOERROR) return (err_flag);
		if (down) gmt_intpol_reverse (x, u, n, m);	/* Must flip directions back */
		return (GMT_NOERROR);
	}

	/* Here input has NaNs so we need to treat it section by section */

	for (i = 0; i < m; i++) v[i] = C->session.d_NaN;	/* Initialize all output to NaN */
	start_i = start_j = 0;
	while (start_i < n && GMT_is_dnan (y[start_i])) start_i++;	/* First non-NaN data point */
	while (start_i < n && start_j < m) {
		stop_i = start_i + 1;
		while (stop_i < n && !GMT_is_dnan (y[stop_i])) stop_i++;	/* Wind to next NaN point (or past end of array) */
		this_n = stop_i - start_i;	/* Number of clean points to interpolate from */
		stop_i--;			/* Now stop_i is the ID of the last usable point */
		if (this_n == 1) {		/* Not enough to interpolate, just skip this single point */
			start_i++;	/* Skip to next point (which is NaN) */
			while (start_i < n && GMT_is_dnan (y[start_i])) start_i++;	/* Wind to next non-NaN data point */
			continue;
		}
		/* OK, have enough points to interpolate; find corresponding output section */
		while (start_j < m && u[start_j] < x[start_i]) start_j++;
		if (start_j == m) continue;	/* Ran out of points */
		stop_j = start_j;
		while (stop_j < m && u[stop_j] <= x[stop_i]) stop_j++;
		this_m = stop_j - start_j;	/* Number of output points to interpolate to */
		err_flag = gmt_intpol_sub (C, &x[start_i], &y[start_i], this_n, this_m, &u[start_j], &v[start_j], mode);
		if (err_flag != GMT_NOERROR) return (err_flag);
		start_i = stop_i + 1;	/* Move to point after last usable point in current section */
		while (start_i < n && GMT_is_dnan (y[start_i])) start_i++;	/* Next section's first non-NaN data point */
	}

	if (down) gmt_intpol_reverse (x, u, n, m);	/* Must flip directions back */

	return (GMT_NOERROR);
}

void *GMT_memory_func (struct GMT_CTRL *C, void *prev_addr, GMT_LONG nelem, size_t size, char *fname, GMT_LONG line)
{
	/* Multi-functional memory allocation subroutine.
	   If prev_addr is NULL, allocate new memory of nelem elements of size bytes.
		Ignore when nelem == 0.
	   If prev_addr exists, reallocate the memory to a larger or smaller chunk of nelem elements of size bytes.
		When nelem = 0, free the memory.
	*/

	void *tmp = NULL;
	static char *m_unit[4] = {"bytes", "kb", "Mb", "Gb"};
	double mem;
	GMT_LONG k;
#if defined(WIN32) && defined(USE_MEM_ALIGNED)
	size_t alignment = 16;
#endif

	if (nelem < 0) {	/* Probably 32-bit overflow */
		GMT_report (C, GMT_MSG_FATAL, "Error: Requesting negative number of items (%ld) - exceeding 32-bit counting?\n", nelem);
#ifdef DEBUG
		GMT_report (C, GMT_MSG_FATAL, "GMT_memory called by %s from file %s on line %ld\n", C->init.progname, fname, line);
#endif
		GMT_exit (EXIT_FAILURE);
	}

	if (prev_addr) {
		if (nelem == 0) { /* Take care of n == 0 */
			GMT_free (C, prev_addr);
			return (NULL);
		}
#if defined(WIN32) && defined(USE_MEM_ALIGNED)
		if ((tmp = _aligned_realloc ( prev_addr, (size_t)(nelem * size), alignment)) == NULL)
#else
		if ((tmp = realloc ( prev_addr, (size_t)(nelem * size))) == NULL)
#endif
		{
			mem = (double)(nelem * size);
			k = 0;
			while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
			GMT_report (C, GMT_MSG_FATAL, "Error: Could not reallocate memory [%.2f %s, %ld items of %ld bytes]\n", mem, m_unit[k], nelem, (GMT_LONG)size);
#ifdef DEBUG
			GMT_report (C, GMT_MSG_FATAL, "GMT_memory [realloc] called by %s from file %s on line %ld\n", C->init.progname, fname, line);
#endif
			GMT_exit (EXIT_FAILURE);
		}
	}
	else {
		if (nelem == 0) return (NULL); /* Take care of n == 0 */
#if defined(WIN32) && defined(USE_MEM_ALIGNED)
		tmp = _aligned_malloc ((size_t)(nelem * size), alignment);
		if (tmp != NULL)
			tmp = memset(tmp, 0, (size_t)(nelem * size));
		else
#else
		if ((tmp = calloc ((size_t)nelem, size)) == NULL)
#endif
		{
			mem = (double)(nelem * size);
			k = 0;
			while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
			GMT_report (C, GMT_MSG_FATAL, "Error: Could not allocate memory [%.2f %s, %ld items of %ld bytes]\n", mem, m_unit[k], nelem, (GMT_LONG)size);
#ifdef DEBUG
			GMT_report (C, GMT_MSG_FATAL, "GMT_memory [calloc] called by %s from file %s on line %ld\n", C->init.progname, fname, line);
#endif
			GMT_exit (EXIT_FAILURE);
		}
	}
#ifdef DEBUG
	gmt_memtrack_add (C, GMT_mem_keeper, fname, line, tmp, prev_addr, (GMT_LONG)(nelem * size));
#endif
	return (tmp);
}

void GMT_free_func (struct GMT_CTRL *C, void *addr, const char *fname, const GMT_LONG line)
{
	if (addr==NULL)
	{
		/* report freeing unallocated memory */
#ifdef DEBUG
		GMT_report (C, GMT_MSG_NORMAL,
		    "GMT_free_func: %s from file %s on line %ld tried to free unallocated memory\n",
		    C->init.progname, fname, line);
#else
		GMT_report (C, GMT_MSG_DEBUG,
		    "GMT_free_func: %s tried to free unallocated memory\n");
#endif
		return; /* Do not free a NULL pointer, although allowed */
	}
#ifdef DEBUG
	gmt_memtrack_sub (C, GMT_mem_keeper, (char *)fname, line, addr);
#endif
#if defined(WIN32) && defined(USE_MEM_ALIGNED)
	_aligned_free (addr);
#else
	free (addr);
#endif
}

void * GMT_malloc_func (struct GMT_CTRL *C, void *ptr, GMT_LONG n, GMT_LONG *n_alloc, size_t element_size, char *fname, GMT_LONG line)
{
	/* GMT_malloc is used to initialize, grow, and finalize an array allocation in cases
	 * were more memory is needed as new data are read.  There are three different situations:
	 * A) Initial allocation of memory:
	 *	Signaled by passing n_alloc == 0.  This will initialize the pointer to NULL first.
	 *	Allocation size is controlled by C->session.min_meminc, unless n > 0 which then is used.
	 * B) Incremental increase in memory:
	 *	Signaled by passing n >= n_alloc.  The incremental memory is set to 50% of the
	 *	previous size, but no more than C->session.max_meminc. Note, *ptr[n] is the location
	 *	of where the next assignment will take place, hence n >= n_alloc is used.
	 * C) Finalize memory:
	 *	Signaled by passing n == 0 and n_alloc > 0.  Unused memory beyond n_alloc is freed up.
	 * You can use GMT_set_meminc to temporarily change GMT_min_mininc and GMT_reset_meminc will
	 * reset this value to the compilation default.
	 * For 32-bit systems there are safety-values to avoid 32-bit overflow.
	 * Note that n_alloc refers to the number of items to allocate, not the total memory taken
	 * up by the allocated items (which is n_alloc * element_size).
	 * module is the name of the module requesting the memory (main program or library function).
	 */

	if (*n_alloc == 0) {	/* A) First time allocation, use default minimum size, unless n > 0 is given */
		*n_alloc = (n == 0) ? C->session.min_meminc : n;
		ptr = NULL;	/* Initialize a new pointer to NULL before calling GMT_memory with it */
	}
	else if (n == 0 && *n_alloc > 0)	/* C) Final allocation, set to actual final size */
		n = *n_alloc;		/* Keep the given n_alloc */
	else if (n < *n_alloc)	/* Nothing to do, already has enough memory.  This is a safety valve. */
		return (ptr);
	else {		/* B) n >= n_alloc: Compute an increment, but make sure not to exceed GMT_LONG limit under 32-bit systems */
		size_t add;	/* The increment of memory (in items) */
		add = MAX (C->session.min_meminc, MIN (*n_alloc/2, C->session.max_meminc));	/* Suggested increment from 50% rule, but no less than C->session.min_meminc */
		*n_alloc = MIN (add + *n_alloc, LONG_MAX);	/* Limit n_alloc to LONG_MAX */
		if (n >= *n_alloc) *n_alloc = n + 1;		/* If still not big enough, set n_alloc to n + 1 */
	}

	/* Here n_alloc is set one way or another.  Do the actual [re]allocation */

	ptr = GMT_memory_func (C, ptr, *n_alloc, element_size, fname, line);

	return (ptr);
}

void GMT_set_meminc (struct GMT_CTRL *C, GMT_LONG increment)
{	/* Temporarily set the GMT_min_memic to this value; restore with GMT_reset_meminc */
	C->session.min_meminc = increment;
}

void GMT_reset_meminc (struct GMT_CTRL *C)
{	/* Temporarily set the GMT_min_memic to this value; restore with GMT_reset_meminc */
	C->session.min_meminc = GMT_MIN_MEMINC;
}

void GMT_contlabel_init (struct GMT_CTRL *C, struct GMT_CONTOUR *G, GMT_LONG mode)
{	/* Assign default values to structure */
	GMT_memset (G, 1, struct GMT_CONTOUR);	/* Sets all to 0 */
	if (mode == 1) {
		G->line_type = 1;
		strcpy (G->line_name, "Contour");
	}
	else {
		G->line_type = 0;
		strcpy (G->line_name, "Line");
	}
	sprintf (G->label_file, "%s_labels.txt", G->line_name);
	G->transparent = TRUE;
	G->spacing = TRUE;
	G->half_width = 5;
	G->label_dist_spacing = 4.0;	/* Inches */
	G->label_dist_frac = 0.25;	/* Fraction of above head start for closed labels */
	G->box = 2;			/* Rect box shape is Default */
	if (C->current.setting.proj_length_unit == GMT_CM) G->label_dist_spacing = 10.0 / 2.54;
	G->clearance[GMT_X] = G->clearance[GMT_Y] = 15.0;	/* 15 % */
	G->clearance_flag = 1;	/* Means we gave percentages of label font size */
	G->just = 6;	/* CM */
	G->font_label = C->current.setting.font_annot[0];	/* FONT_ANNOT_PRIMARY */
	G->font_label.size = 9.0;
	G->dist_unit = C->current.setting.proj_length_unit;
	G->pen = C->current.setting.map_default_pen;
	G->line_pen = C->current.setting.map_default_pen;
	GMT_rgb_copy (G->rgb, C->current.setting.ps_page_rgb);		/* Default box color is page color [nominally white] */
}

GMT_LONG GMT_contlabel_specs (struct GMT_CTRL *C, char *txt, struct GMT_CONTOUR *G)
{
	GMT_LONG k, bad = 0, pos = 0;
	char p[GMT_BUFSIZ], txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], c;
	char *specs = NULL;

	/* Decode [+a<angle>|n|p[u|d]][+c<dx>[/<dy>]][+d][+e][+f<font>][+g<fill>][+j<just>][+l<label>][+n|N<dx>[/<dy>]][+o][+p[<pen>]][+r<min_rc>][+t|T[<file>]][+u<unit>][+v][+w<width>][+=<prefix>] strings */

	for (k = 0; txt[k] && txt[k] != '+'; k++);	/* Look for +<options> strings */

	if (!txt[k]) return (0);

	/* Decode new-style +separated substrings */

	G->nudge_flag = 0;
	specs = &txt[k+1];
	while ((GMT_strtok (C, specs, "+", &pos, p))) {
		switch (p[0]) {
			case 'a':	/* Angle specification */
				if (p[1] == 'p' || p[1] == 'P')	{	/* Line-parallel label */
					G->angle_type = G->hill_label = 0;
					if (p[2] == 'u' || p[2] == 'U')		/* Line-parallel label readable when looking up hill */
						G->hill_label = +1;
					else if (p[2] == 'd' || p[2] == 'D')	/* Line-parallel label readable when looking down hill */
						G->hill_label = -1;
				}
				else if (p[1] == 'n' || p[1] == 'N')	/* Line-normal label */
					G->angle_type = 1;
				else {					/* Label at a fixed angle */
					G->label_angle = atof (&p[1]);
					G->angle_type = 2;
					GMT_lon_range_adjust (GMT_IS_M180_TO_P180_RANGE, &G->label_angle);	/* Now -180/+180 */
					while (fabs (G->label_angle) > 90.0) G->label_angle -= copysign (180.0, G->label_angle);
				}
				break;

			case 'c':	/* Clearance specification */
				k = sscanf (&p[1], "%[^/]/%s", txt_a, txt_b);
				G->clearance[GMT_X] = GMT_to_inch (C, txt_a);
				G->clearance[GMT_Y] = (k == 2 ) ? GMT_to_inch (C, txt_b) : G->clearance[GMT_X];
				G->clearance_flag = ((strchr (txt_a, '%')) ? 1 : 0);
				if (k == 0) bad++;
				break;

			case 'd':	/* Debug option - draw helper points or lines */
				G->debug = TRUE;
				break;

			case 'e':	/* Just lay down text info; Delay actual label plotting until a psclip -C call */
				G->delay = TRUE;
				break;

			case 'f':	/* Font specification */
				if (GMT_getfont (C, &p[1], &G->font_label)) bad++;
				break;

			case 'g':	/* Box Fill specification */
				if (GMT_getrgb (C, &p[1], G->rgb)) bad++;
				G->transparent = FALSE;
				break;

			case 'j':	/* Justification specification */
				txt_a[0] = p[1];	txt_a[1] = p[2];	txt_a[2] = '\0';
				G->just = GMT_just_decode (C, txt_a, 6);
				break;
#ifdef GMT_COMPAT
			case 'k':	/* Font color specification (backwards compatibility only since font color is now part of font specification */
				GMT_report (C, GMT_MSG_COMPAT, "+k<fontcolor> in contour label spec is obsolete, now part of +f<font>\n");
				if (GMT_getfill (C, &p[1], &(G->font_label.fill))) bad++;
				break;
#endif
			case 'l':	/* Exact Label specification */
				strcpy (G->label, &p[1]);
				G->label_type = 1;
				break;

			case 'L':	/* Label code specification */
				switch (p[1]) {
					case 'h':	/* Take the first string in segment headers */
						G->label_type = 2;
						break;
					case 'd':	/* Use the current plot distance in chosen units */
						G->label_type = 3;
						G->dist_unit = GMT_unit_lookup (C, p[2], C->current.setting.proj_length_unit);
						break;
					case 'D':	/* Use current map distance in chosen units */
						G->label_type = 4;
						if (p[2] && strchr ("defkMn", (int)p[2])) {	/* Found a valid unit */
							c = p[2];
							GMT_init_distaz (C, c, 1 + GMT_sph_mode (C), GMT_LABEL_DIST);
						}
						else 	/* Meaning "not set" */
							c = 0;
						G->dist_unit = (int)c;
						break;
					case 'f':	/* Take the 3rd column in fixed contour location file */
						G->label_type = 5;
						break;
					case 'x':	/* Take the first string in segment headers in the crossing file */
						G->label_type = 6;
						break;
					case 'n':	/* Use the current segment number */
						G->label_type = 7;
						break;
					case 'N':	/* Use <current file number>/<segment number> */
						G->label_type = 8;
						break;
					default:	/* Probably meant lower case l */
						strcpy (G->label, &p[1]);
						G->label_type = 1;
						break;
				}
				break;

			case 'n':	/* Nudge specification; dx/dy are increments along local line axes */
				G->nudge_flag = 1;
			case 'N':	/* Nudge specification; dx/dy are increments along plot axes */
				G->nudge_flag++;
				k = sscanf (&p[1], "%[^/]/%s", txt_a, txt_b);
				G->nudge[GMT_X] = GMT_to_inch (C, txt_a);
				G->nudge[GMT_Y] = (k == 2 ) ? GMT_to_inch (C, txt_b) : G->nudge[GMT_X];
				if (k == 0) bad++;
				break;
			case 'o':	/* Use rounded rectangle textbox shape */
				G->box = 4 + (G->box & 1);
				break;

			case 'p':	/* Draw text box outline [with optional textbox pen specification] */
				if (GMT_getpen (C, &p[1], &G->pen)) bad++;
				G->box |= 1;
				break;

			case 'r':	/* Minimum radius of curvature specification */
				G->min_radius = GMT_to_inch (C, &p[1]);
				break;

#ifdef GMT_COMPAT
			case 's':	/* Font size specification (for backward compatibility only since size is part of font specification) */
				GMT_report (C, GMT_MSG_COMPAT, "+s<fontsize> in contour label spec is obsolete, now part of +f<font>\n");
				G->font_label.size = GMT_convert_units (C, &p[1], GMT_PT, GMT_PT);
				if (G->font_label.size <= 0.0) bad++;
				break;
#endif
			case 'T':	/* Save contour label locations to given file [x y angle label] */
				G->save_labels = 1;
			case 't':	/* Save contour label locations to given file [x y label] */
				G->save_labels++;
				if (p[1]) strcpy (G->label_file, &p[1]);
				break;
				
			case 'u':	/* Label Unit specification */
				if (p[1]) strcpy (G->unit, &p[1]);
				break;

			case 'v':	/* Curved text [Default is straight] */
				G->curved_text = TRUE;
				break;

			case 'w':	/* Angle filter width [Default is 10 points] */
				G->half_width = atoi (&p[1]) / 2;
				break;

			case '=':	/* Label Prefix specification */
				if (p[1]) strcpy (G->prefix, &p[1]);
				break;

			default:
				bad++;
				break;
		}
	}

	return (bad);
}

GMT_LONG GMT_contlabel_info (struct GMT_CTRL *C, char flag, char *txt, struct GMT_CONTOUR *L)
{
	/* Interpret the contour-label information string and set structure items */
	GMT_LONG k, j = 0, error = 0;
	char txt_a[GMT_TEXT_LEN256], c, *p = NULL;

	L->spacing = FALSE;	/* Turn off the default since we gave an option */
	strcpy (L->option, &txt[1]);	 /* May need to process L->option later after -R,-J have been set */
	if ((p = strstr (txt, "+r"))) {	/* Want to isolate labels by given radius */
		*p = '\0';	/* Temporarily chop off the +r<radius> part */
		L->isolate = TRUE;
		L->label_isolation = GMT_to_inch (C, &p[2]);
	}

	L->flag = flag;
	switch (txt[0]) {
		case 'L':	/* Quick straight lines for intersections */
			L->do_interpolate = TRUE;
		case 'l':
			L->crossing = GMT_CONTOUR_XLINE;
			break;
		case 'N':	/* Specify number of labels per segment */
			L->number_placement = 1;	/* Distribution of labels */
			if (txt[1] == '-') L->number_placement = -1, j = 1;	/* Left label if n = 1 */
			if (txt[1] == '+') L->number_placement = +1, j = 1;	/* Right label if n = 1 */
		case 'n':	/* Specify number of labels per segment */
			L->number = TRUE;
			k = sscanf (&txt[1+j], "%" GMT_LL "d/%s", &L->n_cont, txt_a);
			if (k == 2) L->min_dist = GMT_to_inch (C, txt_a);
			if (L->n_cont == 0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option: Number of labels must exceed zero\n", L->flag);
				error++;
			}
			if (L->min_dist < 0.0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option: Minimum label separation cannot be negative\n", L->flag);
				error++;
			}
			break;
		case 'f':	/* fixed points file */
			L->fixed = TRUE;
			k = sscanf (&txt[1], "%[^/]/%lf", L->file, &L->slop);
			if (k == 1) L->slop = GMT_CONV_LIMIT;
			break;
		case 'X':	/* Crossing complicated curve */
			L->do_interpolate = TRUE;
		case 'x':	/* Crossing line */
			L->crossing = GMT_CONTOUR_XCURVE;
			strcpy (L->file, &txt[1]);
			break;
		case 'D':	/* Specify distances in geographic units (km, degrees, etc) */
			L->dist_kind = 1;
		case 'd':	/* Specify distances in plot units [cip] */
			L->spacing = TRUE;
			k = sscanf (&txt[j], "%[^/]/%lf", txt_a, &L->label_dist_frac);
			if (k == 1) L->label_dist_frac = 0.25;
			if (L->dist_kind == 1) {	/* Distance units other than xy specified */
				k = strlen (txt_a) - 1;
				c = (isdigit ((int)txt_a[k]) || txt_a[k] == '.') ? 0 : txt_a[k];
				L->label_dist_spacing = atof (&txt_a[1]);
				GMT_init_distaz (C, c, 1 + GMT_sph_mode (C), GMT_CONT_DIST);
			}
			else
				L->label_dist_spacing = GMT_to_inch (C, &txt_a[1]);
			if (L->label_dist_spacing <= 0.0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option: Spacing between labels must exceed 0.0\n", L->flag);
				error++;
			}
			if (L->label_dist_frac < 0.0 || L->label_dist_frac > 1.0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option: Initial label distance fraction must be in 0-1 range\n", L->flag);
				error++;
			}
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option: Unrecognized modifiler %c\n", L->flag, txt[0]);
			error++;
			break;
	}
	if (L->isolate) *p = '+';	/* Replace the + from earlier */

	return (error);
}

GMT_LONG gmt_code_to_lonlat (struct GMT_CTRL *C, char *code, double *lon, double *lat)
{
	GMT_LONG i, n, error = 0, z_OK = FALSE;

	n = strlen (code);
	if (n != 2) return (1);

	for (i = 0; i < 2; i++) {
		switch (code[i]) {
			case 'l':
			case 'L':	/* Left */
				*lon = C->common.R.wesn[XLO];
				break;
			case 'c':
			case 'C':	/* center */
				*lon = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
				break;
			case 'r':
			case 'R':	/* right */
				*lon = C->common.R.wesn[XHI];
				break;
			case 'b':
			case 'B':	/* bottom */
				*lat = C->common.R.wesn[YLO];
				break;
			case 'm':
			case 'M':	/* center */
				*lat = 0.5 * (C->common.R.wesn[YLO] + C->common.R.wesn[YHI]);
				break;
			case 't':
			case 'T':	/* top */
				*lat = C->common.R.wesn[YHI];
				break;
			case 'z':
			case 'Z':	/* z-value */
				z_OK = TRUE;
				break;
			case '+':	/* zmax-location */
				if (z_OK)
					*lon = *lat = DBL_MAX;
				else
					error++;
				break;
			case '-':	/* zmin-location */
				if (z_OK)
					*lon = *lat = -DBL_MAX;
				else
					error++;
				break;
			default:
				error++;
				break;
		}
	}
	return (error);
}

GMT_LONG GMT_contlabel_prep (struct GMT_CTRL *C, struct GMT_CONTOUR *G, double xyz[2][3])
{
	/* G is pointer to the LABELED CONTOUR structure
	 * xyz, if not NULL, have the (x,y,z) min and max values for a grid
	 */

	/* Prepares contour labeling machinery as needed */

	GMT_LONG i, k, n, error = 0, pos, n_alloc = GMT_SMALL_CHUNK;
	double x, y, step = 0.0;
	char buffer[GMT_BUFSIZ], p[GMT_BUFSIZ], txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256], txt_d[GMT_TEXT_LEN256];

	/* Maximum step size (in degrees) used for interpolation of line segments along great circles (if requested) */
	if (G->do_interpolate) step = C->current.setting.map_line_step / C->current.proj.scale[GMT_X] / C->current.proj.M_PR_DEG;

	if (G->clearance_flag) {	/* Gave a percentage of fontsize as clearance */
		G->clearance[GMT_X] = 0.01 * G->clearance[GMT_X] * G->font_label.size * C->session.u2u[GMT_PT][GMT_INCH];
		G->clearance[GMT_Y] = 0.01 * G->clearance[GMT_Y] * G->font_label.size * C->session.u2u[GMT_PT][GMT_INCH];
	}
	if (G->label_type == 5 && !G->fixed) {	/* Requires fixed file */
		error++;
		GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  Labeling option +Lf requires the fixed label location setting\n", G->flag);
	}
	if (G->label_type == 6 && G->crossing != GMT_CONTOUR_XCURVE) {	/* Requires cross file */
		error++;
		GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  Labeling option +Lx requires the crossing lines setting\n", G->flag);
	}
	if (G->spacing && G->dist_kind == 1 && G->label_type == 4 && G->dist_unit == 0) {	/* Did not specify unit - use same as in -G */
		C->current.map.dist[GMT_LABEL_DIST].func = C->current.map.dist[GMT_CONT_DIST].func;
		C->current.map.dist[GMT_LABEL_DIST].scale = C->current.map.dist[GMT_CONT_DIST].scale;
	}
	if ((G->dist_kind == 1 || G->label_type == 4) && !GMT_is_geographic (C, GMT_IN)) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  Map distance options requires a map projection.\n", G->flag);
		error++;
	}
	if (G->angle_type == 0)
		G->no_gap = (G->just < 5 || G->just > 7);	/* Don't clip contour if label is not in the way */
	else if (G->angle_type == 1)
		G->no_gap = ((G->just + 2)%4);	/* Don't clip contour if label is not in the way */

	if (G->crossing == GMT_CONTOUR_XLINE) {
		G->xp = GMT_memory (C, NULL, 1, struct GMT_TABLE);
		G->xp->segment = GMT_memory (C, NULL, n_alloc, struct GMT_LINE_SEGMENT *);
		pos = 0;
		while ((GMT_strtok (C, G->option, ",", &pos, p))) {
			G->xp->segment[G->xp->n_segments] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
			GMT_alloc_segment (C, G->xp->segment[G->xp->n_segments], (GMT_LONG)2, 2, TRUE);
			G->xp->segment[G->xp->n_segments]->n_rows = G->xp->segment[G->xp->n_segments]->n_columns = 2;
			n = sscanf (p, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
			if (n == 4) {	/* Easy, got lon0/lat0/lon1/lat1 */
				error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (C, txt_a, C->current.io.col_type[GMT_IN][GMT_X], &G->xp->segment[G->xp->n_segments]->coord[GMT_X][0]), txt_a);
				error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (C, txt_b, C->current.io.col_type[GMT_IN][GMT_Y], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][0]), txt_b);
				error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (C, txt_c, C->current.io.col_type[GMT_IN][GMT_X], &G->xp->segment[G->xp->n_segments]->coord[GMT_X][1]), txt_c);
				error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (C, txt_d, C->current.io.col_type[GMT_IN][GMT_Y], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][1]), txt_d);
			}
			else if (n == 2) { /* Easy, got <code>/<code> */
				error += gmt_code_to_lonlat (C, txt_a, &G->xp->segment[G->xp->n_segments]->coord[GMT_X][0], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][0]);
				error += gmt_code_to_lonlat (C, txt_b, &G->xp->segment[G->xp->n_segments]->coord[GMT_X][1], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][1]);
			}
			else if (n == 3) {	/* More complicated: <code>/<lon>/<lat> or <lon>/<lat>/<code> */
				if (gmt_code_to_lonlat (C, txt_a, &G->xp->segment[G->xp->n_segments]->coord[GMT_X][0], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][0])) {	/* Failed, so try the other way */
					error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (C, txt_a, C->current.io.col_type[GMT_IN][GMT_X], &G->xp->segment[G->xp->n_segments]->coord[GMT_X][0]), txt_a);
					error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (C, txt_b, C->current.io.col_type[GMT_IN][GMT_Y], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][0]), txt_b);
					error += gmt_code_to_lonlat (C, txt_c, &G->xp->segment[G->xp->n_segments]->coord[GMT_X][1], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][1]);
				}
				else {	/* Worked, pick up second point */
					error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (C, txt_b, C->current.io.col_type[GMT_IN][GMT_X], &G->xp->segment[G->xp->n_segments]->coord[GMT_X][1]), txt_b);
					error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (C, txt_c, C->current.io.col_type[GMT_IN][GMT_Y], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y][1]), txt_c);
				}
			}
			for (i = 0; i < 2; i++) {	/* Reset any zmin/max settings if used and applicable */
				if (G->xp->segment[G->xp->n_segments]->coord[GMT_X][i] == DBL_MAX) {	/* Meant zmax location */
					if (xyz) {
						G->xp->segment[G->xp->n_segments]->coord[GMT_X][i] = xyz[1][GMT_X];
						G->xp->segment[G->xp->n_segments]->coord[GMT_Y][i] = xyz[1][GMT_Y];
					}
					else {
						error++;
						GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  z+ option not applicable here\n", G->flag);
					}
				}
				else if (G->xp->segment[G->xp->n_segments]->coord[GMT_X][i] == -DBL_MAX) {	/* Meant zmin location */
					if (xyz) {
						G->xp->segment[G->xp->n_segments]->coord[GMT_X][i] = xyz[0][GMT_X];
						G->xp->segment[G->xp->n_segments]->coord[GMT_Y][i] = xyz[0][GMT_Y];
					}
					else {
						error++;
						GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  z- option not applicable here\n", G->flag);
					}
				}
			}
			if (G->do_interpolate) G->xp->segment[G->xp->n_segments]->n_rows = GMT_fix_up_path (C, &G->xp->segment[G->xp->n_segments]->coord[GMT_X], &G->xp->segment[G->xp->n_segments]->coord[GMT_Y], G->xp->segment[G->xp->n_segments]->n_rows, step, 0);
			for (i = 0; i < G->xp->segment[G->xp->n_segments]->n_rows; i++) {	/* Project */
				GMT_geo_to_xy (C, G->xp->segment[G->xp->n_segments]->coord[GMT_X][i], G->xp->segment[G->xp->n_segments]->coord[GMT_Y][i], &x, &y);
				G->xp->segment[G->xp->n_segments]->coord[GMT_X][i] = x;
				G->xp->segment[G->xp->n_segments]->coord[GMT_Y][i] = y;
			}
			G->xp->n_segments++;
			if (G->xp->n_segments == n_alloc) {
				n_alloc <<= 1;
				G->xp->segment = GMT_memory (C, G->xp->segment, n_alloc, struct GMT_LINE_SEGMENT *);
			}
		}
		if (G->xp->n_segments < n_alloc) {
			n_alloc <<= 1;
			G->xp->segment = GMT_memory (C, G->xp->segment, G->xp->n_segments, struct GMT_LINE_SEGMENT *);
		}
	}
	else if (G->crossing == GMT_CONTOUR_XCURVE) {
		G->xp = GMT_read_table (C, G->file, GMT_IS_FILE, FALSE, FALSE, FALSE);
		for (k = 0; k < G->xp->n_segments; k++) {
			for (i = 0; i < G->xp->segment[k]->n_rows; i++) {	/* Project */
				GMT_geo_to_xy (C, G->xp->segment[k]->coord[GMT_X][i], G->xp->segment[k]->coord[GMT_Y][i], &x, &y);
				G->xp->segment[k]->coord[GMT_X][i] = x;
				G->xp->segment[k]->coord[GMT_Y][i] = y;
			}
		}
	}
	else if (G->fixed) {
		FILE *fp = NULL;
		GMT_LONG n_col, len, bad_record = FALSE;
		double xy[2];

		if ((fp = GMT_fopen (C, G->file, "r")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  Could not open file %s\n", G->flag, G->file);
			error++;
		}
		n_col = (G->label_type == 5) ? 3 : 2;
		G->f_xy[GMT_X] = GMT_memory (C, NULL, n_alloc, double);
		G->f_xy[GMT_Y] = GMT_memory (C, NULL, n_alloc, double);
		if (n_col == 3) G->f_label = GMT_memory (C, NULL, n_alloc, char *);
		G->f_n = 0;
		while (GMT_fgets (C, buffer, GMT_BUFSIZ, fp)) {
			if (buffer[0] == '#' || buffer[0] == '>' || buffer[0] == '\n') continue;
			len = strlen (buffer);
			for (i = len - 1; i >= 0 && strchr (" \t,\r\n", (int)buffer[i]); i--);
			buffer[++i] = '\n';	buffer[++i] = '\0';	/* Now have clean C string with \n\0 at end */
			sscanf (buffer, "%s %s %[^\n]", txt_a, txt_b, txt_c);	/* Get first 2-3 fields */
			if (GMT_scanf (C, txt_a, C->current.io.col_type[GMT_IN][GMT_X], &xy[GMT_X]) == GMT_IS_NAN) bad_record = TRUE;	/* Got NaN or it failed to decode */
			if (GMT_scanf (C, txt_b, C->current.io.col_type[GMT_IN][GMT_Y], &xy[GMT_Y]) == GMT_IS_NAN) bad_record = TRUE;	/* Got NaN or it failed to decode */
			if (bad_record) {
				C->current.io.n_bad_records++;
				if (C->current.io.give_report && (C->current.io.n_bad_records == 1)) {	/* Report 1st occurrence */
					GMT_report (C, GMT_MSG_FATAL, "Encountered first invalid record near/at line # %ld\n", C->current.io.rec_no);
					GMT_report (C, GMT_MSG_FATAL, "Likely causes:\n");
					GMT_report (C, GMT_MSG_FATAL, "(1) Invalid x and/or y values, i.e. NaNs or garbage in text strings.\n");
					GMT_report (C, GMT_MSG_FATAL, "(2) Incorrect data type assumed if -J, -f are not set or set incorrectly.\n");
					GMT_report (C, GMT_MSG_FATAL, "(3) The -: switch is implied but not set.\n");
					GMT_report (C, GMT_MSG_FATAL, "(4) Input file in multiple segment format but the -m switch is not set.\n");
				}
				continue;
			}
			/* Got here if data are OK */

			if (C->current.setting.io_lonlat_toggle[GMT_IN]) d_swap (xy[GMT_X], xy[GMT_Y]);				/* Got lat/lon instead of lon/lat */
			GMT_map_outside (C, xy[GMT_X], xy[GMT_Y]);
			if (GMT_abs (C->current.map.this_x_status) > 1 || GMT_abs (C->current.map.this_y_status) > 1) continue;	/* Outside map region */

			GMT_geo_to_xy (C, xy[GMT_X], xy[GMT_Y], &G->f_xy[GMT_X][G->f_n], &G->f_xy[GMT_Y][G->f_n]);		/* Project -> xy inches */
			if (n_col == 3) {	/* The label part if asked for */
				G->f_label[G->f_n] = GMT_memory (C, NULL, strlen(txt_c)+1, char);
				strcpy (G->f_label[G->f_n], txt_c);
			}
			G->f_n++;
			if (G->f_n == n_alloc) {
				n_alloc <<= 1;
				G->f_xy[GMT_X] = GMT_memory (C, G->f_xy[GMT_X], n_alloc, double);
				G->f_xy[GMT_Y] = GMT_memory (C, G->f_xy[GMT_Y], n_alloc, double);
				if (n_col == 3) G->f_label = GMT_memory (C, G->f_label, n_alloc, char *);
			}
		}
		GMT_fclose (C, fp);
	}
	if (error) GMT_report (C, GMT_MSG_FATAL, "syntax error -%c:  Valid codes are [lcr][bmt] and z[+-]\n", G->flag);

	return (error);
}

void gmt_contlabel_angle (double x[], double y[], GMT_LONG start, GMT_LONG stop, double cangle, GMT_LONG n, struct GMT_LABEL *L, struct GMT_CONTOUR *G)
{
	GMT_LONG j, this_angle_type;
	double sum_x2 = 0.0, sum_xy = 0.0, sum_y2 = 0.0, dx, dy;

	if (start == stop) {	/* Can happen if we want no smoothing but landed exactly on a knot point */
		if (start > 0)
			start--;
		else if (stop < (n-1))
			stop++;
	}
	for (j = start - G->half_width; j <= stop + G->half_width; j++) {	/* L2 fit for slope over this range of points */
		if (j < 0 || j >= n) continue;
		dx = x[j] - L->x;
		dy = y[j] - L->y;
		sum_x2 += dx * dx;
		sum_y2 += dy * dy;
		sum_xy += dx * dy;
	}
	if (sum_y2 < GMT_CONV_LIMIT)	/* Line is horizontal */
		L->line_angle = 0.0;
	else if (sum_x2 < GMT_CONV_LIMIT)	/* Line is vertical */
		L->line_angle = 90.0;
	else
		L->line_angle = (GMT_IS_ZERO (sum_xy)) ? 90.0 : d_atan2d (sum_xy, sum_x2);
	this_angle_type = G->angle_type;
	if (this_angle_type == 2) {	/* Just return the fixed angle given (unless NaN) */
		if (GMT_is_dnan (cangle)) /* Cannot use this angle - default to along-line angle */
			this_angle_type = 0;
		else
			L->angle = cangle;
	}
	if (this_angle_type != 2) {	/* Must base label angle on the contour angle */
		L->angle = L->line_angle + this_angle_type * 90.0;	/* May add 90 to get normal */
		if (L->angle < 0.0) L->angle += 360.0;
		if (L->angle > 90.0 && L->angle < 270) L->angle -= 180.0;
	}
}

int gmt_sort_label_struct (const void *p_1, const void *p_2)
{
	struct GMT_LABEL **point_1 = (struct GMT_LABEL **)p_1, **point_2 = (struct GMT_LABEL **)p_2;

	if ((*point_1)->dist < (*point_2)->dist) return -1;
	if ((*point_1)->dist > (*point_2)->dist) return +1;
	return 0;
}

void gmt_contlabel_fixpath (struct GMT_CTRL *C, double **xin, double **yin, double d[], GMT_LONG *n, struct GMT_CONTOUR *G)
{	/* Sorts labels based on distance and inserts the label (x,y) point into the x,y path */
	GMT_LONG i, j, k, np;
	double *xp = NULL, *yp = NULL, *x = NULL, *y = NULL;

	if (G->n_label == 0) return;	/* No labels, no need to insert points */

	/* Sort lables based on distance along contour if more than 1 */
	if (G->n_label > 1) qsort(G->L, (size_t)G->n_label, sizeof (struct GMT_LABEL *), gmt_sort_label_struct);

	np = *n + G->n_label;	/* Length of extended path that includes inserted label coordinates */
	xp = GMT_memory (C, NULL, np, double);
	yp = GMT_memory (C, NULL, np, double);
	x = *xin;	y = *yin;	/* Input coordinate arrays */

	/* Make sure the xp, yp array contains the exact points at which labels should be placed */

	for (k = 0, i = j = 0; i < *n && j < np && k < G->n_label; k++) {
		while (i < *n && d[i] < G->L[k]->dist) {	/* Not at the label point yet - just copy points */
			xp[j] = x[i];
			yp[j] = y[i];
			j++;
			i++;
		}
		/* Add the label point */
		G->L[k]->node = j;		/* Update node since we have been adding new points */
		xp[j] = G->L[k]->x;
		yp[j] = G->L[k]->y;
		j++;
	}
	while (i < *n) {	/* Append the rest of the path */
		xp[j] = x[i];
		yp[j] = y[i];
		j++;
		i++;
	}

	GMT_free (C, x);	/* Get rid of old path... */
	GMT_free (C, y);

	*xin = xp;		/* .. and pass out new path */
	*yin = yp;

	*n = np;		/* and the new length */
}

void gmt_contlabel_addpath (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, double zval, char *label, GMT_LONG annot, struct GMT_CONTOUR *G)
{
	GMT_LONG i;
	double s = 0.0, c = 1.0, sign = 1.0;
	struct GMT_CONTOUR_LINE *L = NULL;
	/* Adds this segment to the list of contour lines */

	if (G->n_alloc == 0 || G->n_segments == G->n_alloc) {
		G->n_alloc = (G->n_alloc == 0) ? GMT_CHUNK : (G->n_alloc << 1);
		G->segment = GMT_memory (C, G->segment, G->n_alloc, struct GMT_CONTOUR_LINE *);
	}
	G->segment[G->n_segments] = GMT_memory (C, NULL, 1, struct GMT_CONTOUR_LINE);
	L = G->segment[G->n_segments];	/* Pointer to current segment */
	L->n = n;
	L->x = GMT_memory (C, NULL, L->n, double);
	L->y = GMT_memory (C, NULL, L->n, double);
	GMT_memcpy (L->x, x, L->n, double);
	GMT_memcpy (L->y, y, L->n, double);
	GMT_memcpy (&L->pen, &G->line_pen, 1, struct GMT_PEN);
	/* GMT_memcpy (L->rgb,&G->rgb, 4, double); */
	GMT_memcpy (L->rgb, &G->font_label.fill.rgb, 4, double);
	L->name = GMT_memory (C, NULL, strlen (label)+1, char);
	strcpy (L->name, label);
	L->annot = annot;
	L->z = zval;
	if (G->n_label) {	/* There are labels */
		L->n_labels = G->n_label;
		L->L = GMT_memory (C, NULL, L->n_labels, struct GMT_LABEL);
		for (i = 0; i < L->n_labels; i++) {
			L->L[i].x = G->L[i]->x;
			L->L[i].y = G->L[i]->y;
			L->L[i].line_angle = G->L[i]->line_angle;
			if (G->nudge_flag) {	/* Must adjust point a bit */
				if (G->nudge_flag == 2) sincosd (L->L[i].line_angle, &s, &c);
				/* If N+1 or N-1 is used we want positive x nudge to extend away from end point */
				sign = (G->number_placement) ? (double)L->L[i].end : 1.0;
				L->L[i].x += sign * (G->nudge[GMT_X] * c - G->nudge[GMT_Y] * s);
				L->L[i].y += sign * (G->nudge[GMT_X] * s + G->nudge[GMT_Y] * c);
			}
			L->L[i].angle = G->L[i]->angle;
			L->L[i].dist = G->L[i]->dist;
			L->L[i].node = G->L[i]->node;
			L->L[i].label = GMT_memory (C, NULL, strlen (G->L[i]->label)+1, char);
			strcpy (L->L[i].label, G->L[i]->label);
		}
	}
	G->n_segments++;
}

void GMT_contlabel_free (struct GMT_CTRL *C, struct GMT_CONTOUR *G)
{
	GMT_LONG i, j;
	struct GMT_CONTOUR_LINE *L = NULL;

	/* Free memory */

	for (i = 0; i < G->n_segments; i++) {
		L = G->segment[i];	/* Pointer to current segment */
		for (j = 0; j < L->n_labels; j++) GMT_free (C, L->L[j].label);
		GMT_free (C, L->L);
		GMT_free (C, L->x);
		GMT_free (C, L->y);
		GMT_free (C, L->name);
		GMT_free (C, L);
	}
	GMT_free (C, G->segment);
	GMT_free_table (C, G->xp);
	if (G->f_n) {	/* Array for fixed points */
		GMT_free (C, G->f_xy[GMT_X]);
		GMT_free (C, G->f_xy[GMT_Y]);
		if (G->f_label) {
			for (i = 0; i < G->f_n; i++) GMT_free (C, G->f_label[i]);
			GMT_free (C, G->f_label);
		}
	}
}

void gmt_get_radii_of_curvature (double x[], double y[], GMT_LONG n, double r[])
{
	/* Calculates radius of curvature along the spatial curve x(t), y(t) */

	GMT_LONG i, im, ip;
	double a, b, c, d, e, f, x0, y0, denom;

	for (im = 0, i = 1, ip = 2; ip < n; i++, im++, ip++) {
		a = (x[im] - x[i]);	b = (y[im] - y[i]);	e = 0.5 * (x[im] * x[im] + y[im] * y[im] - x[i] * x[i] - y[i] * y[i]);
		c = (x[i] - x[ip]);	d = (y[i] - y[ip]);	f = 0.5 * (x[i] * x[i] + y[i] * y[i] - x[ip] * x[ip] - y[ip] * y[ip]);
		denom = b * c - a * d;;
		if (denom == 0.0)
			r[i] = DBL_MAX;
		else {
			x0 = (-d * e + b * f) / denom;
			y0 = (c * e - a * f) / denom;
			r[i] = hypot (x[i] - x0, y[i] - y0);
		}
	}
	r[0] = r[n-1] = DBL_MAX;	/* Boundary conditions has zero curvature at end points so r = inf */
}

void gmt_edge_contour (struct GMT_CTRL *C, struct GMT_GRID *G, GMT_LONG col, GMT_LONG row, GMT_LONG side, double d, double *x, double *y)
{
	if (side == 0) {
		*x = GMT_grd_col_to_x (C, col+d, G->header);
		*y = GMT_grd_row_to_y (C, row, G->header);
	}
	else if (side == 1) {
		*x = GMT_grd_col_to_x (C, col+1, G->header);
		*y = GMT_grd_row_to_y (C, row-d, G->header);
	}
	else if (side == 2) {
		*x = GMT_grd_col_to_x (C, col+1-d, G->header);
		*y = GMT_grd_row_to_y (C, row-1, G->header);
	}
	else {
		*x = GMT_grd_col_to_x (C, col, G->header);
		*y = GMT_grd_row_to_y (C, row-1+d, G->header);
	}
}

void gmt_setcontjump (float *z, GMT_LONG nz)
{
/* This routine will check if there is a 360 jump problem
 * among these coordinates and adjust them accordingly so
 * that subsequent testing can determine if a zero contour
 * goes through these edges.  E.g., values like 359, 1
 * should become -1, 1 after this function */

	GMT_LONG i, jump = FALSE;
	double dz;

	for (i = 1; !jump && i < nz; i++) {
		dz = z[i] - z[0];
		if (fabs (dz) > 180.0) jump = TRUE;
	}

	if (!jump) return;

	z[0] = (float)fmod (z[0], 360.0);
	if (z[0] > 180.0) z[0] -= 360.0;
	for (i = 1; i < nz; i++) {
		if (z[i] > 180.0) z[i] -= 360.0;
		dz = z[i] - z[0];
		if (fabs (dz) > 180.0) z[i] -= (float)copysign (360.0, dz);
		z[i] = (float)fmod (z[i], 360.0);
	}
}

GMT_LONG gmt_trace_contour (struct GMT_CTRL *C, struct GMT_GRID *G, GMT_LONG test, GMT_LONG *edge, double **x, double **y, GMT_LONG col, GMT_LONG row, GMT_LONG side, GMT_LONG offset, size_t *bit, GMT_LONG *nan_flag)
{
	GMT_LONG n = 1, this_side, old_side, n_exits, opposite_side, m, n_nan, n_alloc;
	GMT_LONG ij, ij_in, side_in, edge_word, edge_bit, ij0, more, p[5];
	float z[5];
	double xk[5], *xx = NULL, *yy = NULL;
	static GMT_LONG d_col[5] = {0, 1, 0, 0, 0}, d_row[5] = {0, 0, -1, 0, 0}, d_side[5] = {0, 1, 0, 1, 0};

	/* Note: We need GMT_IJP to get us the index into the padded G->data whereas we use GMT_IJ0 to get the corresponding index for non-padded edge array */
	p[0] = p[4] = 0;	p[1] = 1;	p[2] = 1 - G->header->mx;	p[3] = -G->header->mx;	/* Padded offsets for G->data array */
	*nan_flag = 0;

	/* Check if this edge was already used */

	ij0 = GMT_IJ0 (G->header, row + d_row[side], col + d_col[side]);	/* Index to use with the edge array */
	edge_word = ij0 / 32 + d_side[side] * offset;
	edge_bit = ij0 % 32;
	if (test && (edge[edge_word] & bit[edge_bit])) return (0);

	ij_in = GMT_IJP (G->header, row, col);	/* Index to use with the padded G->data array */
	side_in = side;

	/* First check if contour cuts the starting edge */

	z[0] = G->data[ij_in+p[side]];
	z[1] = G->data[ij_in+p[side+1]];
	if (C->current.map.z_periodic) gmt_setcontjump (z, 2);

	if (!(z[0] * z[1] < 0.0)) return (0);	/* This formulation will also return if one of the z's is NaN */

	n_alloc = GMT_CHUNK;
	m = n_alloc - 2;

	xx = GMT_memory (C, NULL, n_alloc, double);
	yy = GMT_memory (C, NULL, n_alloc, double);

	gmt_edge_contour (C, G, col, row, side, z[0] / (z[0] - z[1]), &(xx[0]), &(yy[0]));
	edge[edge_word] |= bit[edge_bit];

	more = TRUE;
	do {
		ij = GMT_IJP (G->header, row, col);	/* Index to use with the padded G->data array */

		/* If this is the box and edge we started from, explicitly close the polygon and exit */

		if (n > 1 && ij == ij_in && side == side_in) {	/* Yes, we close and exit */
			xx[n-1] = xx[0]; yy[n-1] = yy[0];
			more = FALSE;
			continue;
		}

		n_exits = 0;
		old_side = side;
		for (this_side = 0; this_side < 5; this_side++) z[this_side] = G->data[ij+p[this_side]];	/* Copy the 4 corners to the z array and duplicate the 1st as a '5th' corner */
		if (C->current.map.z_periodic) gmt_setcontjump (z, 5);

		for (this_side = n_nan = 0; this_side < 4; this_side++) {	/* Loop over the 4 box sides and count possible exits */

			/* Skip if NaN encountered */

			if (GMT_is_fnan (z[this_side+1]) || GMT_is_fnan (z[this_side])) {	/* Check each side, defined by two nodes, for NaNs */
				n_nan++;
				continue;
			}

			/* Skip if no zero-crossing on this edge */

			if (z[this_side+1] * z[this_side] > 0.0) continue;

			/* Save normalized distance along edge from corner this_side to crossing of edge this_side */

			xk[this_side] = z[this_side] / (z[this_side] - z[this_side+1]);

			/* Skip if this is the entry edge of the current box (this_side == old_side) */

			if (this_side == old_side) continue;

			/* Here we have a new crossing */

			side = this_side;
			n_exits++;
		}
		xk[4] = xk[0];	/* Repeat the first value */

		if (n > m) {	/* Must try to allocate more memory */
			n_alloc <<= 1;
			m = n_alloc - 2;
			xx = GMT_memory (C, xx, n_alloc, double);
			yy = GMT_memory (C, yy, n_alloc, double);
		}

		/* These are the possible outcomes of n_exits:
		   0: No exits. We must have struck a wall of NaNs
		   1: One exit. Take it!
		   2: Two exits is not possible!
		   3: Three exits means we have entered on a saddle point
		*/

		if (n_exits == 0) {	/* We have hit a field of NaNs, finish contour */
			more = FALSE;
			*nan_flag = n_nan;
			continue;
		}
		else if (n_exits == 3) {	/* Saddle point: Turn to the correct side */
			opposite_side = (old_side + 2) % 4;	/* Opposite side */
			if (xk[old_side] + xk[opposite_side] > xk[old_side+1] + xk[opposite_side+1])
				side = (old_side + 1) % 4;
			else
				side = (old_side + 3) % 4;
		}
		gmt_edge_contour (C, G, col, row, side, xk[side], &(xx[n]), &(yy[n]));
		n++;

		/* Mark the new edge as used */

		ij0 = GMT_IJ0 (G->header, row + d_row[side], col + d_col[side]);	/* Non-padded index used for edge array */
		edge_word = ij0 / 32 + d_side[side] * offset;
		edge_bit = ij0 % 32;
		edge[edge_word] |= bit[edge_bit];

		if ((side == 0 && row == G->header->ny - 1) || (side == 1 && col == G->header->nx - 2) ||
			(side == 2 && row == 1) || (side == 3 && col == 0)) {	/* Going out of grid */
			more = FALSE;
			continue;
		}

		/* Move on to next box (col,row,side) */

		col -= (side-2)%2;
		row -= (side-1)%2;
		side = (side+2)%4;

	} while (more);

	xx = GMT_memory (C, xx, n, double);
	yy = GMT_memory (C, yy, n, double);

	*x = xx;	*y = yy;
	return (n);
}

GMT_LONG gmt_smooth_contour (struct GMT_CTRL *C, double **x_in, double **y_in, GMT_LONG n, GMT_LONG sfactor, GMT_LONG stype)
{
	/* Input n (x_in, y_in) points */
	/* n_out = sfactor * n -1 */
	/* Interpolation scheme used (0 = linear, 1 = Akima, 2 = Cubic spline, 3 = None */
	GMT_LONG i, j, k, n_out;
	double ds, t_next, *x = NULL, *y = NULL;
	double *t_in = NULL, *t_out = NULL, *x_tmp = NULL, *y_tmp = NULL, x0, x1, y0, y1;
	char *flag = NULL;

	if (sfactor == 0 || n < 4) return (n);	/* Need at least 4 points to call Akima */

	x = *x_in;	y = *y_in;

	n_out = sfactor * n - 1;	/* Number of new points */

	t_in  = GMT_memory (C, NULL, n, double);
	t_out = GMT_memory (C, NULL, n_out + n, double);
	x_tmp = GMT_memory (C, NULL, n_out + n, double);
	y_tmp = GMT_memory (C, NULL, n_out + n, double);
	flag  = GMT_memory (C, NULL, n_out + n, char);

	/* Create dummy distance values for input points, and throw out duplicate points while at it */

	t_in[0] = 0.0;
	for (i = j = 1; i < n; i++)	{
		ds = hypot ((x[i]-x[i-1]), (y[i]-y[i-1]));
		if (ds > 0.0) {
			t_in[j] = t_in[j-1] + ds;
			x[j] = x[i];
			y[j] = y[i];
			j++;
		}
	}

	n = j;	/* May have lost some duplicates */
	if (sfactor == 0 || n < 4) return (n);	/* Need at least 4 points to call Akima */

	/* Create equidistance output points */

	ds = t_in[n-1] / (n_out-1);
	t_next = ds;
	t_out[0] = 0.0;
	flag[0] = TRUE;
	for (i = j = 1; i < n_out; i++) {
		if (j < n && t_in[j] < t_next) {
			t_out[i] = t_in[j];
			flag[i] = TRUE;
			j++;
			n_out++;
		}
		else {
			t_out[i] = t_next;
			t_next += ds;
		}
	}
	t_out[n_out-1] = t_in[n-1];
	if (t_out[n_out-1] == t_out[n_out-2]) n_out--;
	flag[n_out-1] = TRUE;

	if (GMT_intpol (C, t_in, x, n, n_out, t_out, x_tmp, stype)) {
		GMT_report (C, GMT_MSG_FATAL, "GMT internal error in  gmt_smooth_contour!\n");
		GMT_exit (EXIT_FAILURE);
	}
	if (GMT_intpol (C, t_in, y, n, n_out, t_out, y_tmp, stype)) {
		GMT_report (C, GMT_MSG_FATAL, "GMT internal error in  gmt_smooth_contour!\n");
		GMT_exit (EXIT_FAILURE);
	}

	/* Make sure interpolated function is bounded on each segment interval */

	i = 0;
	while (i < (n_out - 1)) {
		j = i + 1;
		while (j < n_out && !flag[j]) j++;
		x0 = x_tmp[i];	x1 = x_tmp[j];
		if (x0 > x1) d_swap (x0, x1);
		y0 = y_tmp[i];	y1 = y_tmp[j];
		if (y0 > y1) d_swap (y0, y1);
		for (k = i + 1; k < j; k++) {
			if (x_tmp[k] < x0)
				x_tmp[k] = x0 + 1.0e-10;
			else if (x_tmp[k] > x1)
				x_tmp[k] = x1 - 1.0e-10;
			if (y_tmp[k] < y0)
				y_tmp[k] = y0 + 1.0e-10;
			else if (y_tmp[k] > y1)
				y_tmp[k] = y1 - 1.0e-10;
		}
		i = j;
	}

	GMT_free (C, x);
	GMT_free (C, y);

	*x_in = x_tmp;
	*y_in = y_tmp;

	GMT_free (C, t_in);
	GMT_free (C, t_out);
	GMT_free (C, flag);

	return (n_out);
}

GMT_LONG gmt_splice_contour (struct GMT_CTRL *C, double **x, double **y, GMT_LONG n, double *x2, double *y2, GMT_LONG n2)
{	/* Basically does a "tail -r" on the array x,y and append arrays x2/y2 */

	GMT_LONG i, j, m;
	double *x1, *y1;

	if (n2 < 2) return (n);		/* Nothing to be done when second piece < 2 points */

	m = n + n2 - 1;	/* Total length since one point is shared */

	/* Make more space */

	x1 = *x;	y1 = *y;
	x1 = GMT_memory (C, x1, m, double);
	y1 = GMT_memory (C, y1, m, double);

	/* Move first piece to the back */

	for (i = m-1, j = n-1; j >= 0; j--, i--) {
		x1[i] = x1[j];	y1[i] = y1[j];
	}

	/* Put second piece, in reverse, in the front */

	for (i = n2-2, j = 1; j < n2; j++, i--) {
		x1[i] = x2[j];	y1[i] = y2[j];
	}

	*x = x1;
	*y = y1;

	return (m);
}

void gmt_orient_contour (struct GMT_GRID *G, double *x, double *y, GMT_LONG n, GMT_LONG orient)
{
	/* Determine handedness of the contour and if opposite of orient reverse the contour */
	GMT_LONG reverse, i, j, side[2], z_dir, ij, k, k2;
	double fx[2], fy[2], dx, dy;

	if (orient == 0) return;	/* Nothing to be done when no orientation specified */
	if (n < 2) return;		/* Cannot work on a single point */

	for (k = 0; k < 2; k++) {	/* Calculate fractional node numbers from left/top */
		fx[k] = (x[k] - G->header->wesn[XLO]) * G->header->r_inc[GMT_X] - G->header->xy_off;
		fy[k] = (G->header->wesn[YHI] - y[k]) * G->header->r_inc[GMT_Y] - G->header->xy_off;
	}

	/* Get(i,j) of the lower left node in the rectangle containing this contour segment.
	   We use the average x and y coordinate for this to avoid any round-off involved in
	   working on a single coordinate. The average coordinate should always be inside the
	   rectangle and hence the floor/ceil operators will yield the LL node. */

	i = (GMT_LONG) floor (0.5 * (fx[0] + fx[1]));
	j = (GMT_LONG) ceil  (0.5 * (fy[0] + fy[1]));
	ij = GMT_IJP (G->header, j, i);
	z_dir = (G->data[ij] > 0.0) ? +1 : -1;	/* +1 if lower-left node is higher than contour value, else -1 */

	for (k = 0; k < 2; k++) {	/* Determine which edge the contour points lie on (0-3) */
		/* We KNOW that for each k, either x[k] or y[k] lies EXACTLY on a gridline.  This is used
		 * to deal with the inevitable round-off that places points slightly off the gridline.  We
		 * pick the coordinate closest to the gridline as the one that should be exactly on the gridline */

		k2 = 1 - k;	/* The other point */
		dx = fmod (fx[k], 1.0);
		if (dx > 0.5) dx = 1.0 - dx;	/* Fraction to closest vertical gridline */
		dy = fmod (fy[k], 1.0);
		if (dy > 0.5) dy = 1.0 - dy;	/* Fraction to closest horizontal gridline */
		if (dx < dy)		/* Point is on a vertical grid line (left [3] or right [1]) */
			side[k] = (fx[k] < fx[k2]) ? 3 : 1;	/* Simply check order of fx to determine which it is */
		else						/* Point must be on horizontal grid line (top [2] or bottom [0]) */
			side[k] = (fy[k] > fy[k2]) ? 0 : 2;	/* Same for fy */
	}

	switch (side[0]) {	/* Entry side */
		case 0:	/* Bottom: Regardless of exit the LL node is to the left of line vector */
			reverse = (z_dir == orient);
			break;
		case 1:	/* Right: Must check exit */
			switch (side[1]) {
				case  0:	/* Bottom: LL Node is the the right of vector */
					reverse = (z_dir != orient);
					break;
				default:	/* left and top it is to the left */
					reverse = (z_dir == orient);
					break;
			}
			break;
		case 2:	/* Top: Must check exit */
			switch (side[1]) {
				case 3:		/* Left: LL node is to the left of vector */
					reverse = (z_dir == orient);
					break;
				default:	/* Bottom and right: LL node is to the right of vector */
					reverse = (z_dir != orient);
					break;
			}
			break;
		default:/* Left: LL node is always to the right of line vector */
			reverse = (z_dir != orient);
			break;
	}

	if (reverse) {	/* Must reverse order of contour */
		for (i = 0, j = n-1; i < n/2; i++, j--) {
			d_swap (x[i], x[j]);
			d_swap (y[i], y[j]);
		}
	}
}

GMT_LONG GMT_contours (struct GMT_CTRL *C, struct GMT_GRID *G, GMT_LONG smooth_factor, GMT_LONG int_scheme, GMT_LONG orient, GMT_LONG *edge, GMT_LONG *first, double **x, double **y)
{
	/* The routine finds the zero-contour in the grd dataset.  it assumes that
	 * no node has a value exactly == 0.0.  If more than max points are found
	 * gmt_trace_contour will try to allocate more memory in blocks of GMT_CHUNK points.
	 * orient arranges the contour so that the values to the left of the contour is higher (orient = 1)
	 * or lower (orient = -1) than the contour value.
	 * Note: grd has a pad while edge does not!
	 */

	static GMT_LONG col_0, row_0, side;
	GMT_LONG n = 0, n2, row, col, n_edges, nans = 0, offset;
	double *x2 = NULL, *y2 = NULL;
	static size_t bit[32];

	n_edges = G->header->ny * (GMT_LONG) ceil (G->header->nx / 16.0);
	offset = n_edges / 2;

	/* Reset edge-flags to zero, if necessary */
	if (*first) {	/* Set col_0,row_0 for southern boundary */
		GMT_LONG i;
		GMT_memset (edge, n_edges, GMT_LONG);
		col_0 = side = 0;
		row_0 = G->header->ny - 1;
		for (i = 1, bit[0] = 1; i < 32; i++) bit[i] = bit[i-1] << 1;
		*first = FALSE;
	}

	if (side == 0) {	/* Southern boundary */
		for (col = col_0, row = row_0; col < G->header->nx-1; col++) {
			if ((n = gmt_trace_contour (C, G, TRUE, edge, x, y, col, row, 0, offset, bit, &nans))) {
				if (orient) gmt_orient_contour (G, *x, *y, n, orient);
				n = gmt_smooth_contour (C, x, y, n, smooth_factor, int_scheme);
				col_0 = col + 1;	row_0 = row;
				return (n);
			}
		}
		if (n == 0) {	/* No more crossing of southern boundary, go to next side (east) */
			col_0 = G->header->nx - 2;
			row_0 = G->header->ny - 1;
			side++;
		}
	}

	if (side == 1) {	/* Eastern boundary */
		for (col = col_0, row = row_0; row > 0; row--) {
			if ((n = gmt_trace_contour (C, G, TRUE, edge, x, y, col, row, 1, offset, bit, &nans))) {
				if (orient) gmt_orient_contour (G, *x, *y, n, orient);
				n = gmt_smooth_contour (C, x, y, n, smooth_factor, int_scheme);
				col_0 = col;	row_0 = row - 1;
				return (n);
			}
		}
		if (n == 0) {	/* No more crossing of eastern boundary, go to next side (north) */
			col_0 = G->header->nx - 2;
			row_0 = 1;
			side++;
		}
	}

	if (side == 2) {	/* Northern boundary */
		for (col = col_0, row = row_0; col >= 0; col--) {
			if ((n = gmt_trace_contour (C, G, TRUE, edge, x, y, col, row, 2, offset, bit, &nans))) {
				if (orient) gmt_orient_contour (G, *x, *y, n, orient);
				n = gmt_smooth_contour (C, x, y, n, smooth_factor, int_scheme);
				col_0 = col - 1;	row_0 = row;
				return (n);
			}
		}
		if (n == 0) {	/* No more crossing of northern boundary, go to next side (west) */
			col_0 = 0;
			row_0 = 1;
			side++;
		}
	}

	if (side == 3) {	/* Western boundary */
		for (col = col_0, row = row_0; row < G->header->ny; row++) {
			if ((n = gmt_trace_contour (C, G, TRUE, edge, x, y, col, row, 3, offset, bit, &nans))) {
				if (orient) gmt_orient_contour (G, *x, *y, n, orient);
				n = gmt_smooth_contour (C, x, y, n, smooth_factor, int_scheme);
				col_0 = col;	row_0 = row + 1;
				return (n);
			}
		}
		if (n == 0) {	/* No more crossing of western boundary, go to next side (vertical internals) */
			col_0 = 1;
			row_0 = 1;
			side++;
		}
	}

	if (side == 4) {	/* Then loop over interior boxes (vertical edges) */
		for (row = row_0; row < G->header->ny; row++) {
			for (col = col_0; col < G->header->nx-1; col++) {
				if ((n = gmt_trace_contour (C, G, TRUE, edge, x, y, col, row, 3, offset, bit, &nans))) {
					if (nans && (n2 = gmt_trace_contour (C, G, FALSE, edge, &x2, &y2, col-1, row, 1, offset, bit, &nans))) {
						/* Must trace in other direction, then splice */
						n = gmt_splice_contour (C, x, y, n, x2, y2, n2);
						GMT_free (C, x2);
						GMT_free (C, y2);
					}
					if (orient) gmt_orient_contour (G, *x, *y, n, orient);
					n = gmt_smooth_contour (C, x, y, n, smooth_factor, int_scheme);
					col_0 = col + 1;	row_0 = row;
					return (n);
				}
			}
			col_0 = 1;
		}
		if (n == 0) {	/* No more crossing of vertical internal edges, go to next side (horizontal internals) */
			col_0 = 0;
			row_0 = 1;
			side++;
		}
	}

	if (side == 5) {	/* Then loop over interior boxes (horizontal edges) */
		for (row = row_0; row < G->header->ny; row++) {
			for (col = col_0; col < G->header->nx-1; col++) {
				if ((n = gmt_trace_contour (C, G, TRUE, edge, x, y, col, row, 2, offset, bit, &nans))) {
					if (nans && (n2 = gmt_trace_contour (C, G, FALSE, edge, &x2, &y2, col-1, row, 0, offset, bit, &nans))) {
						/* Must trace in other direction, then splice */
						n = gmt_splice_contour (C, x, y, n, x2, y2, n2);
						GMT_free (C, x2);
						GMT_free (C, y2);
					}
					if (orient) gmt_orient_contour (G, *x, *y, n, orient);
					n = gmt_smooth_contour (C, x, y, n, smooth_factor, int_scheme);
					col_0 = col + 1;	row_0 = row;
					return (n);
				}
			}
			col_0 = 1;
		}
	}

	/* Nothing found */
	return (0);
}

struct GMT_LINE_SEGMENT * GMT_dump_contour (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double z)
{	/* Returns a segment with this contour */
	GMT_LONG n_cols;
	char header[GMT_BUFSIZ];
	struct GMT_LINE_SEGMENT *S = NULL;

	if (n < 2) return (NULL);

	n_cols = (GMT_is_dnan (z)) ? 2 : 3;	/* psmask only dumps xy */
	S = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
	GMT_alloc_segment (C, S, n, n_cols, TRUE);
	if (n_cols == 3)
		sprintf (header, "%g contour -Z%g", z, z);
	else
		sprintf (header, "clip contour");
	S->header = strdup (header);

	GMT_memcpy (S->coord[GMT_X], x, n, double);
	GMT_memcpy (S->coord[GMT_Y], y, n, double);
	if (n_cols == 3) GMT_setnval (S->coord[GMT_Z], n, z);
	S->n_rows = n;

	return (S);
}

char * GMT_make_filename (struct GMT_CTRL *C, char *template, GMT_LONG fmt[], double z, GMT_LONG closed, GMT_LONG count[])
{
	/* Produce a filename given the template and the running values.
	 * Here, c, d, f stands for the O/C character, the running count,
	 * and the contour level z.  When a running count is used it is
	 * incremented.  If c is not used the only count[0] is used, else
	 * we used count[0] for open and count[1] for closed contours. */

	GMT_LONG i, n_fmt;
	static char kind[2] = {'O', 'C'};
	char file[GMT_BUFSIZ];

	for (i = n_fmt = 0; i < 3; i++) if (fmt[i]) n_fmt++;
	if (n_fmt == 0) return (NULL);	/* Will write single file [or to stdout] */

	if (n_fmt == 1) {	/* Just need a single item */
		if (fmt[0])	/* Just two kinds of files, closed and open */
			sprintf (file, template, kind[closed]);
		else if (fmt[1])	/* Just files with a single unique running number */
			sprintf (file, template, count[0]++);
		else	/* Contour level file */
			sprintf (file, template, z);
	}
	else if (n_fmt == 2) {	/* Need two items, and f+c is not allowed */
		if (fmt[0]) {	/* open/close is involved */
			if (fmt[1] < fmt[0])	/* Files with a single unique running number and open/close */
				sprintf (file, template, count[closed]++, kind[closed]);
			else	/* Same in reverse order */
				sprintf (file, template, kind[closed], count[closed]++);
		}
		else if (fmt[1] < fmt[2])	/* Files with a single unique running number and contour value */
			sprintf (file, template, count[0]++, z);
		else 	/* Same in reverse order */
			sprintf (file, template, z, count[0]++);
	}
	else {	/* Combinations of three items */
		if (fmt[0] < fmt[1] && fmt[1] < fmt[2])	/* Files with c, d, and f in that order */
			sprintf (file, template, kind[closed], count[closed]++, z);
		else if (fmt[0] < fmt[2] && fmt[2] < fmt[1])	/* Files with c, f, and d in that order */
			sprintf (file, template, kind[closed], z, count[closed]++);
		else if (fmt[1] < fmt[2] && fmt[2] < fmt[0])	/* Files with d, f, c in that order */
			sprintf (file, template, count[closed]++, z, kind[closed]);
		else if (fmt[1] < fmt[0] && fmt[0] < fmt[2])	/* Files with d, c, f in that order */
			sprintf (file, template, count[closed]++, kind[closed], z);
		else if (fmt[2] < fmt[0] && fmt[0] < fmt[1])	/* Files with f, c, d in that order */
			sprintf (file, template, z, kind[closed], count[closed]++);
		else						/* Files with f, d, c in that order */
			sprintf (file, template, z, count[closed]++, kind[closed]);
	}
	return (strdup (file));
}

GMT_LONG gmt_label_is_OK (struct GMT_CTRL *C, struct GMT_LABEL *L, char *this_label, char *label, double this_dist, double this_value_dist, GMT_LONG xl, GMT_LONG fj, struct GMT_CONTOUR *G)
{
	GMT_LONG label_OK = TRUE, i, k;
	char format[GMT_TEXT_LEN256];
	struct GMT_CONTOUR_LINE *S = NULL;

	if (G->isolate) {	/* Must determine if the proposed label is withing radius distance of any other label already accepted */
		for (i = 0; i < G->n_segments; i++) {	/* Previously processed labels */
			S = G->segment[i];	/* Pointer to current segment */
			for (k = 0; k < S->n_labels; k++) if (hypot (L->x - S->L[k].x, L->y - S->L[k].y) < G->label_isolation) return (FALSE);
		}
		/* Also check labels for current segment */
		for (k = 0; k < G->n_label; k++) if (hypot (L->x - G->L[k]->x, L->y - G->L[k]->y) < G->label_isolation) return (FALSE);
	}

	switch (G->label_type) {
		case 0:
			if (label && label[0])
				strcpy (this_label, label);
			else
				label_OK = FALSE;
			break;

		case 1:
		case 2:
			if (G->label && G->label[0])
				strcpy (this_label, G->label);
			else
				label_OK = FALSE;
			break;

		case 3:
			if (G->spacing) {	/* Distances are even so use special contour format */
				GMT_get_format (C, this_dist * C->session.u2u[GMT_INCH][G->dist_unit], G->unit, CNULL, format);
				sprintf (this_label, format, this_dist * C->session.u2u[GMT_INCH][G->dist_unit]);
			}
			else {
				sprintf (this_label, C->current.setting.format_float_out, this_dist * C->session.u2u[GMT_INCH][G->dist_unit]);
			}
			break;

		case 4:
			sprintf (this_label, C->current.setting.format_float_out, this_value_dist);
			break;

		case 5:
			if (G->f_label[fj] && G->f_label[fj][0])
				strcpy (this_label, G->f_label[fj]);
			else
				label_OK = FALSE;
			break;

		case 6:
			if (G->xp->segment[xl]->label && G->xp->segment[xl]->label[0])
				strcpy (this_label, G->xp->segment[xl]->label);
			else
				label_OK = FALSE;
			break;

		case 7:
			sprintf (this_label, "%ld", (C->current.io.status & GMT_IO_SEG_HEADER) ? C->current.io.seg_no - 1 : C->current.io.seg_no);
			break;

		case 8:
			sprintf (this_label, "%ld/%ld", C->current.io.file_no, (C->current.io.status & GMT_IO_SEG_HEADER) ? C->current.io.seg_no - 1 : C->current.io.seg_no);
			break;

		default:	/* Should not happen... */
			GMT_report (C, GMT_MSG_FATAL, "ERROR in gmt_label_is_OK. Notify gmt-team@lists.hawaii.edu\n");
			GMT_exit (EXIT_FAILURE);
			break;
	}

	return (label_OK);
}

void gmt_place_label (struct GMT_CTRL *C, struct GMT_LABEL *L, char *txt, struct GMT_CONTOUR *G, GMT_LONG use_unit)
{	/* Allocates needed space and copies in the label */
	GMT_LONG n, m = 0;

	if (use_unit && G->unit && G->unit[0]) m = strlen (G->unit) + (G->unit[0] != '-');	/* Must allow extra space for a unit string */
	n = strlen (txt) + 1 + m;
	if (G->prefix && G->prefix[0]) {	/* Must prepend the prefix string */
		n += strlen (G->prefix) + 1;
		L->label = GMT_memory (C, NULL, n, char);
		if (G->prefix[0] == '-')	/* No space between annotation and prefix */
			sprintf (L->label, "%s%s", &G->prefix[1], txt);
		else
			sprintf (L->label, "%s %s", G->prefix, txt);
	}
	else {
		L->label = GMT_memory (C, NULL, n, char);
		strcpy (L->label, txt);
	}
	if (use_unit && G->unit && G->unit[0]) {	/* Append a unit string */
		if (G->unit[0] == '-')	/* No space between annotation and prefix */
			strcat (L->label, &G->unit[1]);
		else {
			strcat (L->label, " ");
			strcat (L->label, G->unit);
		}
	}
}

void gmt_hold_contour_sub (struct GMT_CTRL *C, double **xxx, double **yyy, GMT_LONG nn, double zval, char *label, char ctype, double cangle, GMT_LONG closed, struct GMT_CONTOUR *G)
{	/* The xx, yy are expected to be projected x/y inches */
	GMT_LONG i, j, start = 0;
	size_t n_alloc = GMT_SMALL_CHUNK;
	double *track_dist = NULL, *map_dist = NULL, *value_dist = NULL, *radii = NULL, *xx = NULL, *yy = NULL;
	double dx, dy, width, f, this_dist, step, stept, this_value_dist, lon[2], lat[2];
	struct GMT_LABEL *new_label = NULL;
	char this_label[GMT_BUFSIZ];

	if (nn < 2) return;

	xx = *xxx;	yy = *yyy;
	G->n_label = 0;

	/* OK, line is long enough to be added to array of lines */

	if (ctype == 'A' || ctype == 'a') {	/* Annotated contours, must find label placement */

		/* Calculate distance along contour and store in track_dist array */

		if (G->dist_kind == 1) GMT_xy_to_geo (C, &lon[1], &lat[1], xx[0], yy[0]);
		map_dist = GMT_memory (C, NULL, nn, double);	/* Distances on map in inches */
		track_dist = GMT_memory (C, NULL, nn, double);	/* May be km ,degrees or whatever */
		value_dist = GMT_memory (C, NULL, nn, double);	/* May be km ,degrees or whatever */
		radii = GMT_memory (C, NULL, nn, double);	/* Radius of curvature, in inches */

		/* We will calculate the radii of curvature at all points.  By default we dont care and
		 * will place labels at whatever distance we end up with.  However, if the user has asked
		 * for a minimum limit on the radius of curvature [Default 0] we do not want to place labels
		 * at those sections where the curvature is large.  Since labels are placed according to
		 * distance along track, the way we deal with this is to set distance increments to zero
		 * where curvature is large:  that way, there is no increase in distance over those patches
		 * and the machinery for determining when we exceed the next label distance will not kick
		 * in until after curvature drops and increments are again nonzero.  This procedure only
		 * applies to the algorithms based on distance along track.
		 */

		gmt_get_radii_of_curvature (xx, yy, nn, radii);

		map_dist[0] = track_dist[0] = value_dist[0] = 0.0;	/* Unnecessary, just so we understand the logic */
		for (i = 1; i < nn; i++) {
			/* Distance from xy */
			dx = xx[i] - xx[i-1];
			if (GMT_is_geographic (C, GMT_IN) && C->current.map.is_world && fabs (dx) > (width = GMT_half_map_width (C, yy[i-1]))) {
				width *= 2.0;
				dx = copysign (width - fabs (dx), -dx);
				if (xx[i] < width)
					xx[i-1] -= width;
				else
					xx[i-1] += width;
			}
			dy = yy[i] - yy[i-1];
			step = stept = hypot (dx, dy);
			map_dist[i] = map_dist[i-1] + step;
			if (G->dist_kind == 1 || G->label_type == 4) {
				lon[0] = lon[1];	lat[0] = lat[1];
				GMT_xy_to_geo (C, &lon[1], &lat[1], xx[i], yy[i]);
				if (G->dist_kind == 1) step = GMT_distance_type (C, lon[0], lat[0], lon[1], lat[1], GMT_CONT_DIST);
				if (G->label_type == 4) stept = GMT_distance_type (C, lon[0], lat[0], lon[1], lat[1], GMT_LABEL_DIST);
			}
			if (radii[i] < G->min_radius) step = stept = 0.0;	/* If curvature is too great we simply don't add up distances */
			track_dist[i] = track_dist[i-1] + step;
			value_dist[i] = value_dist[i-1] + stept;
		}
		GMT_free (C, radii);

		/* G->L array is only used so we can later sort labels based on distance along track.  Once
		 * GMT_contlabel_draw has been called we will free up the memory as the labels are kept in
		 * the linked list starting at G->anchor. */

		G->L = GMT_memory (C, NULL, n_alloc, struct GMT_LABEL *);

		if (G->spacing) {	/* Place labels based on distance along contours */
			double last_label_dist, dist_offset, dist;

			dist_offset = (closed && G->dist_kind == 0) ? (1.0 - G->label_dist_frac) * G->label_dist_spacing : 0.0;	/* Label closed contours longer than frac of dist_spacing */
			last_label_dist = 0.0;
			i = 1;
			while (i < nn) {

				dist = track_dist[i] + dist_offset - last_label_dist;
				if (dist > G->label_dist_spacing) {	/* Time for label */
					new_label = GMT_memory (C, NULL, 1, struct GMT_LABEL);
					f = (dist - G->label_dist_spacing) / (track_dist[i] - track_dist[i-1]);
					if (f < 0.5) {
						new_label->x = xx[i] - f * (xx[i] - xx[i-1]);
						new_label->y = yy[i] - f * (yy[i] - yy[i-1]);
						new_label->dist = map_dist[i] - f * (map_dist[i] - map_dist[i-1]);
						this_value_dist = value_dist[i] - f * (value_dist[i] - value_dist[i-1]);
					}
					else {
						f = 1.0 - f;
						new_label->x = xx[i-1] + f * (xx[i] - xx[i-1]);
						new_label->y = yy[i-1] + f * (yy[i] - yy[i-1]);
						new_label->dist = map_dist[i-1] + f * (map_dist[i] - map_dist[i-1]);
						this_value_dist = value_dist[i-1] + f * (value_dist[i] - value_dist[i-1]);
					}
					this_dist = G->label_dist_spacing - dist_offset + last_label_dist;
					if (gmt_label_is_OK (C, new_label, this_label, label, this_dist, this_value_dist, (GMT_LONG)(-1), (GMT_LONG)(-1), G)) {
						gmt_place_label (C, new_label, this_label, G, !(G->label_type == 0 || G->label_type == 3));
						new_label->node = i - 1;
						gmt_contlabel_angle (xx, yy, i - 1, i, cangle, nn, new_label, G);
						G->L[G->n_label++] = new_label;
						if (G->n_label == (GMT_LONG)n_alloc) {
							n_alloc <<= 1;
							G->L = GMT_memory (C, G->L, n_alloc, struct GMT_LABEL *);
						}
					}
					else	/* All in vain... */
						GMT_free (C, new_label);
					dist_offset = 0.0;
					last_label_dist = this_dist;
				}
				else	/* Go to next point in line */
					i++;
			}
			if (G->n_label == 0) GMT_report (C, GMT_MSG_VERBOSE, "Warning: Your -Gd|D option produced no contour labels for z = %g\n", zval);

		}
		if (G->number) {	/* Place prescribed number of labels evenly along contours */
			GMT_LONG nc, e_val = 0;
			double dist, last_dist;

			last_dist = (G->n_cont > 1) ? -map_dist[nn-1] / (G->n_cont - 1) : -0.5 * map_dist[nn-1];
			nc = (map_dist[nn-1] > G->min_dist) ? G->n_cont : 0;
			for (i = j = 0; i < nc; i++) {
				new_label = GMT_memory (C, NULL, 1, struct GMT_LABEL);
				if (G->number_placement && !closed) {
					e_val = G->number_placement;
					if (G->number_placement == -1 && G->n_cont == 1) {	/* Label justified with start of segment */
						f = d_atan2d (xx[0] - xx[1], yy[0] - yy[1]) + 180.0;	/* 0-360 */
						G->end_just[0] = (f >= 90.0 && f <= 270) ? 7 : 5;
					}
					else if (G->number_placement == +1 && G->n_cont == 1) {	/* Label justified with end of segment */
						f = d_atan2d (xx[nn-1] - xx[nn-2], yy[nn-1] - yy[nn-2]) + 180.0;	/* 0-360 */
						G->end_just[1] = (f >= 90.0 && f <= 270) ? 7 : 5;
					}
					else if (G->number_placement && G->n_cont > 1)	/* One of the end labels */
						e_val = (i == 0) ? -1 : +1;
					dist = (G->n_cont > 1) ? i * track_dist[nn-1] / (G->n_cont - 1) : 0.5 * (G->number_placement + 1.0) * track_dist[nn-1];
					this_value_dist = (G->n_cont > 1) ? i * value_dist[nn-1] / (G->n_cont - 1) : 0.5 * (G->number_placement + 1.0) * value_dist[nn-1];
				}
				else {
					dist = (i + 1 - 0.5 * closed) * track_dist[nn-1] / (G->n_cont + 1 - closed);
					this_value_dist = (i + 1 - 0.5 * closed) * value_dist[nn-1] / (G->n_cont + 1 - closed);
				}
				while (j < nn && track_dist[j] < dist) j++;
				if (j == nn) j--;	/* Safety precaution */
				f = ((j == 0) ? 1.0 : (dist - track_dist[j-1]) / (track_dist[j] - track_dist[j-1]));
				if (f < 0.5) {	/* Pick the smallest fraction to minimize Taylor shortcomings */
					new_label->x = xx[j-1] + f * (xx[j] - xx[j-1]);
					new_label->y = yy[j-1] + f * (yy[j] - yy[j-1]);
					new_label->dist = map_dist[j-1] + f * (map_dist[j] - map_dist[j-1]);
				}
				else {
					f = 1.0 - f;
					new_label->x = (j == 0) ? xx[0] : xx[j] - f * (xx[j] - xx[j-1]);
					new_label->y = (j == 0) ? yy[0] : yy[j] - f * (yy[j] - yy[j-1]);
					new_label->dist = (j == 0) ? 0.0 : map_dist[j] - f * (map_dist[j] - map_dist[j-1]);
				}
				if ((new_label->dist - last_dist) >= G->min_dist) {	/* OK to accept this label */
					this_dist = dist;
					if (gmt_label_is_OK (C, new_label, this_label, label, this_dist, this_value_dist, (GMT_LONG)(-1), (GMT_LONG)(-1), G)) {
						gmt_place_label (C, new_label, this_label, G, !(G->label_type == 0));
						new_label->node = (j == 0) ? 0 : j - 1;
						gmt_contlabel_angle (xx, yy, new_label->node, j, cangle, nn, new_label, G);
						if (G->number_placement) new_label->end = e_val;
						G->L[G->n_label++] = new_label;
						if (G->n_label == (GMT_LONG)n_alloc) {
							n_alloc <<= 1;
							G->L = GMT_memory (C, G->L, n_alloc, struct GMT_LABEL *);
						}
					}
					last_dist = new_label->dist;
				}
				else	/* All in vain... */
					GMT_free (C, new_label);
			}
			if (G->n_label == 0) GMT_report (C, GMT_MSG_VERBOSE, "Warning: Your -Gn|N option produced no contour labels for z = %g\n", zval);
		}
		if (G->crossing) {	/* Determine label positions based on crossing lines */
			GMT_LONG left, right, line_no;
			GMT_init_track (C, yy, nn, &(G->ylist));
			for (line_no = 0; line_no < G->xp->n_segments; line_no++) {	/* For each of the crossing lines */
				GMT_init_track (C, G->xp->segment[line_no]->coord[GMT_Y], G->xp->segment[line_no]->n_rows, &(G->ylist_XP));
				G->nx = GMT_crossover (C, G->xp->segment[line_no]->coord[GMT_X], G->xp->segment[line_no]->coord[GMT_Y], NULL, G->ylist_XP, G->xp->segment[line_no]->n_rows, xx, yy, NULL, G->ylist, nn, FALSE, &G->XC);
				GMT_free (C, G->ylist_XP);
				if (G->nx == 0) continue;

				/* OK, we found intersections for labels */

				for (i = 0; i < G->nx; i++) {
					left  = (GMT_LONG) floor (G->XC.xnode[1][i]);
					right = (GMT_LONG) ceil  (G->XC.xnode[1][i]);
					new_label = GMT_memory (C, NULL, 1, struct GMT_LABEL);
					new_label->x = G->XC.x[i];
					new_label->y = G->XC.y[i];
					new_label->node = left;
					new_label->dist = track_dist[left];
					f = G->XC.xnode[1][i] - left;
					if (f < 0.5) {
						this_dist = track_dist[left] + f * (track_dist[right] - track_dist[left]);
						new_label->dist = map_dist[left] + f * (map_dist[right] - map_dist[left]);
						this_value_dist = value_dist[left] + f * (value_dist[right] - value_dist[left]);
					}
					else {
						f = 1.0 - f;
						this_dist = track_dist[right] - f * (track_dist[right] - track_dist[left]);
						new_label->dist = map_dist[right] - f * (map_dist[right] - map_dist[left]);
						this_value_dist = value_dist[right] - f * (value_dist[right] - value_dist[left]);
					}
					if (gmt_label_is_OK (C, new_label, this_label, label, this_dist, this_value_dist, line_no, (GMT_LONG)(-1), G)) {
						gmt_place_label (C, new_label, this_label, G, !(G->label_type == 0));
						gmt_contlabel_angle (xx, yy, left, right, cangle, nn, new_label, G);
						G->L[G->n_label++] = new_label;
						if (G->n_label == (GMT_LONG)n_alloc) {
							n_alloc <<= 1;
							G->L = GMT_memory (C, G->L, n_alloc, struct GMT_LABEL *);
						}
					}
					else	/* All in vain... */
						GMT_free (C, new_label);
				}
				GMT_x_free (C, &G->XC);
			}
			GMT_free (C, G->ylist);
			if (G->n_label == 0) GMT_report (C, GMT_MSG_VERBOSE, "Warning: Your -Gx|X|l|L option produced no contour labels for z = %g\n", zval);
		}
		if (G->fixed) {	/* Prescribed point locations for labels that match points in input records */
			double dist, min_dist;
			for (j = 0; j < G->f_n; j++) {	/* Loop over fixed point list */
				min_dist = DBL_MAX;
				for (i = 0; i < nn; i++) {	/* Loop over input line/contour */
					if ((dist = hypot (xx[i] - G->f_xy[0][j], yy[i] - G->f_xy[1][j])) < min_dist) {	/* Better fit */
						min_dist = dist;
						start = i;
					}
				}
				if (min_dist < G->slop) {	/* Closest point within tolerance */
					new_label = GMT_memory (C, NULL, 1, struct GMT_LABEL);
					new_label->x = xx[start];
					new_label->y = yy[start];
					new_label->node = start;
					new_label->dist = track_dist[start];
					this_dist = track_dist[start];
					new_label->dist = map_dist[start];
					this_value_dist = value_dist[start];
					if (gmt_label_is_OK (C, new_label, this_label, label, this_dist, this_value_dist, (GMT_LONG)(-1), j, G)) {
						gmt_place_label (C, new_label, this_label, G, !(G->label_type == 0));
						gmt_contlabel_angle (xx, yy, start, start, cangle, nn, new_label, G);
						G->L[G->n_label++] = new_label;
						if (G->n_label == (GMT_LONG)n_alloc) {
							n_alloc <<= 1;
							G->L = GMT_memory (C, G->L, n_alloc, struct GMT_LABEL *);
						}
					}
					else	/* All in vain... */
						GMT_free (C, new_label);
				}
			}

			if (G->n_label == 0) GMT_report (C, GMT_MSG_VERBOSE, "Warning: Your -Gf option produced no contour labels for z = %g\n", zval);
		}
		gmt_contlabel_fixpath (C, &xx, &yy, map_dist, &nn, G);	/* Inserts the label x,y into path */
		gmt_contlabel_addpath (C, xx, yy, nn, zval, label, TRUE, G);		/* Appends this path and the labels to list */

		GMT_free (C, track_dist);
		GMT_free (C, map_dist);
		GMT_free (C, value_dist);
		/* Free label structure since info is now copied to segment labels */
		for (i = 0; i < G->n_label; i++) {
			GMT_free (C, G->L[i]->label);
			GMT_free (C, G->L[i]);
		}
		GMT_free (C, G->L);
	}
	else {   /* just one line, no holes for labels */
		gmt_contlabel_addpath (C, xx, yy, nn, zval, label, FALSE, G);		/* Appends this path to list */
	}
	*xxx = xx;
	*yyy = yy;
}

void GMT_hold_contour (struct GMT_CTRL *C, double **xxx, double **yyy, GMT_LONG nn, double zval, char *label, char ctype, double cangle, GMT_LONG closed, struct GMT_CONTOUR *G)
{	/* The xx, yy are expected to be projected x/y inches.
	 * This function just makes sure that the xxx/yyy are continuous and do not have map jumps.
	 * If there are jumps we find them and call the main gmt_hold_contour_sub for each segment
	 */

	GMT_LONG seg, first, n, *split = NULL;
	double *xs = NULL, *ys = NULL, *xin = NULL, *yin = NULL;

	if ((split = GMT_split_line (C, xxx, yyy, &nn, G->line_type)) == NULL) {	/* Just one long line */
		gmt_hold_contour_sub (C, xxx, yyy, nn, zval, label, ctype, cangle, closed, G);
		return;
	}

	/* Here we had jumps and need to call the _sub function once for each segment */

	xin = *xxx;	yin = *yyy;
	for (seg = 0, first = 0; seg <= split[0]; seg++) {	/* Number of segments are given by split[0] + 1 */
		n = split[seg+1] - first;
		xs = GMT_memory (C, NULL, n, double);
		ys = GMT_memory (C, NULL, n, double);
		GMT_memcpy (xs, &xin[first], n, double);
		GMT_memcpy (ys, &yin[first], n, double);
		gmt_hold_contour_sub (C, &xs, &ys, n, zval, label, ctype, cangle, closed, G);
		GMT_free (C, xs);
		GMT_free (C, ys);
		first = n;	/* First point in next segment */
	}
	GMT_free (C, split);
}

void GMT_get_plot_array (struct GMT_CTRL *C) {	/* Allocate more space for plot arrays */

	C->current.plot.n_alloc = (C->current.plot.n_alloc == 0) ? GMT_CHUNK : (C->current.plot.n_alloc << 1);
	C->current.plot.x = GMT_memory (C, C->current.plot.x, C->current.plot.n_alloc, double);
	C->current.plot.y = GMT_memory (C, C->current.plot.y, C->current.plot.n_alloc, double);
	C->current.plot.pen = GMT_memory (C, C->current.plot.pen, C->current.plot.n_alloc, GMT_LONG);
}

GMT_LONG GMT_get_format (struct GMT_CTRL *C, double interval, char *unit, char *prefix, char *format)
{
	GMT_LONG i, j, ndec = 0, general = FALSE;
	char text[GMT_BUFSIZ];

	if (strchr (C->current.setting.format_float_out, 'g')) {	/* General format requested */

		/* Find number of decimals needed in the format statement */

		sprintf (text, "%.12g", interval);
		for (i = 0; text[i] && text[i] != '.'; i++);
		if (text[i]) {	/* Found a decimal point */
			for (j = i + 1; text[j] && text[j] != 'e'; j++);
			ndec = j - i - 1;
			if (text[j] == 'e') {	/* Exponential notation, modify ndec */
				ndec -= atoi (&text[++j]);
				if (ndec < 0) ndec = 0;	/* since a positive exponent could give -ve answer */
			}
		}
		general = TRUE;
		strcpy (format, C->current.setting.format_float_out);
	}

	if (unit && unit[0]) {	/* Must append the unit string */
		if (!strchr (unit, '%'))	/* No percent signs */
			strncpy (text, unit, (size_t)80);
		else {
			for (i = j = 0; i < (GMT_LONG)strlen (unit); i++) {
				text[j++] = unit[i];
				if (unit[i] == '%') text[j++] = unit[i];
			}
			text[j] = 0;
		}
		if (text[0] == '-') {	/* No space between annotation and unit */
			if (ndec > 0)
				sprintf (format, "%%.%ldf%s", ndec, &text[1]);
			else
				sprintf (format, "%s%s", C->current.setting.format_float_out, &text[1]);
		}
		else {			/* 1 space between annotation and unit */
			if (ndec > 0)
				sprintf (format, "%%.%ldf %s", ndec, text);
			else
				sprintf (format, "%s %s", C->current.setting.format_float_out, text);
		}
		if (ndec == 0) ndec = 1;	/* To avoid resetting format later */
	}
	else if (ndec > 0)
		sprintf (format, "%%.%ldf", ndec);
	else if (!general) {	/* Pull ndec from given format if .<precision> is given */
		for (i = 0, j = -1; j == -1 && C->current.setting.format_float_out[i]; i++) if (C->current.setting.format_float_out[i] == '.') j = i;
		if (j > -1) ndec = atoi (&C->current.setting.format_float_out[j+1]);
		strcpy (format, C->current.setting.format_float_out);
	}
	if (prefix && prefix[0]) {	/* Must prepend the prefix string */
		if (prefix[0] == '-')	/* No space between annotation and unit */
			sprintf (text, "%s%s", &prefix[1], format);
		else
			sprintf (text, "%s %s", prefix, format);
		strcpy (format, text);
	}
	return (ndec);
}

GMT_LONG GMT_non_zero_winding (struct GMT_CTRL *C, double xp, double yp, double *x, double *y, GMT_LONG n_path)
{
	/* Routine returns (2) if (xp,yp) is inside the
	   polygon x[n_path], y[n_path], (0) if outside,
	   and (1) if exactly on the path edge.
	   Uses non-zero winding rule in Adobe PostScript
	   Language reference manual, section 4.6 on Painting.
	   Path should have been closed first, so that
	   x[n_path-1] = x[0], and y[n_path-1] = y[0].

	   This is version 2, trying to kill a bug
	   in above routine:  If point is on X edge,
	   fails to discover that it is on edge.

	   We are imagining a ray extending "up" from the
	   point (xp,yp); the ray has equation x = xp, y >= yp.
	   Starting with crossing_count = 0, we add 1 each time
	   the path crosses the ray in the +x direction, and
	   subtract 1 each time the ray crosses in the -x direction.
	   After traversing the entire polygon, if (crossing_count)
	   then point is inside.  We also watch for edge conditions.

	   If two or more points on the path have x[i] == xp, then
	   we have an ambiguous case, and we have to find the points
	   in the path before and after this section, and check them.
	   */

	GMT_LONG i, j, k, jend, crossing_count, above;
	double y_sect;

	if (n_path < 2) return (GMT_OUTSIDE);	/* Cannot be inside a null set or a point so default to outside */

	if (GMT_polygon_is_open (C, x, y, n_path)) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_non_zero_winding given non-closed polygon\n");
		GMT_exit (EXIT_FAILURE);
	}

	above = FALSE;
	crossing_count = 0;

	/* First make sure first point in path is not a special case:  */
	j = jend = n_path - 1;
	if (x[j] == xp) {
		/* Trouble already.  We might get lucky:  */
		if (y[j] == yp) return (GMT_ONEDGE);

		/* Go backward down the polygon until x[i] != xp:  */
		if (y[j] > yp) above = TRUE;
		i = j - 1;
		while (x[i] == xp && i > 0) {
			if (y[i] == yp) return (GMT_ONEDGE);
			if (!(above) && y[i] > yp) above = TRUE;
			i--;
		}

		/* Now if i == 0 polygon is degenerate line x=xp;
		   since we know xp,yp is inside bounding box,
		   it must be on edge:  */
		if (i == 0) return (GMT_ONEDGE);

		/* Now we want to mark this as the end, for later:  */
		jend = i;

		/* Now if (j-i)>1 there are some segments the point could be exactly on:  */
		for (k = i+1; k < j; k++) {
			if ((y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp)) return (GMT_ONEDGE);
		}


		/* Now we have arrived where i is > 0 and < n_path-1, and x[i] != xp.
			We have been using j = n_path-1.  Now we need to move j forward
			from the origin:  */
		j = 1;
		while (x[j] == xp) {
			if (y[j] == yp) return (GMT_ONEDGE);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}

		/* Now at the worst, j == jstop, and we have a polygon with only 1 vertex
			not at x = xp.  But now it doesn't matter, that would end us at
			the main while below.  Again, if j>=2 there are some segments to check:  */
		for (k = 0; k < j-1; k++) {
			if ((y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp)) return (GMT_ONEDGE);
		}


		/* Finally, we have found an i and j with points != xp.  If (above) we may have crossed the ray:  */
		if (above && x[i] < xp && x[j] > xp)
			crossing_count++;
		else if (above && x[i] > xp && x[j] < xp)
			crossing_count--;

		/* End nightmare scenario for x[0] == xp.  */
	}

	else {
		/* Get here when x[0] != xp:  */
		i = 0;
		j = 1;
		while (x[j] == xp && j < jend) {
			if (y[j] == yp) return (GMT_ONEDGE);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* Again, if j==jend, (i.e., 0) then we have a polygon with only 1 vertex
			not on xp and we will branch out below.  */

		/* if ((j-i)>2) the point could be on intermediate segments:  */
		for (k = i+1; k < j-1; k++) {
			if ((y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp)) return (GMT_ONEDGE);
		}

		/* Now we have x[i] != xp and x[j] != xp.
			If (above) and x[i] and x[j] on opposite sides, we are certain to have crossed the ray.
			If not (above) and (j-i)>1, then we have not crossed it.
			If not (above) and j-i == 1, then we have to check the intersection point.  */

		if (x[i] < xp && x[j] > xp) {
			if (above)
				crossing_count++;
			else if ((j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ((xp - x[i]) / (x[j] - x[i]));
				if (y_sect == yp) return (GMT_ONEDGE);
				if (y_sect > yp) crossing_count++;
			}
		}
		if (x[i] > xp && x[j] < xp) {
			if (above)
				crossing_count--;
			else if ((j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ((xp - x[i]) / (x[j] - x[i]));
				if (y_sect == yp) return (GMT_ONEDGE);
				if (y_sect > yp) crossing_count--;
			}
		}

		/* End easier case for x[0] != xp  */
	}

	/* Now MAIN WHILE LOOP begins:
		Set i = j, and search for a new j, and do as before.  */

	i = j;
	while (i < jend) {
		above = FALSE;
		j = i+1;
		while (x[j] == xp) {
			if (y[j] == yp) return (GMT_ONEDGE);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* if ((j-i)>2) the point could be on intermediate segments:  */
		for (k = i+1; k < j-1; k++) {
			if ((y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp)) return (GMT_ONEDGE);
		}

		/* Now we have x[i] != xp and x[j] != xp.
			If (above) and x[i] and x[j] on opposite sides, we are certain to have crossed the ray.
			If not (above) and (j-i)>1, then we have not crossed it.
			If not (above) and j-i == 1, then we have to check the intersection point.  */

		if (x[i] < xp && x[j] > xp) {
			if (above)
				crossing_count++;
			else if ((j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ((xp - x[i]) / (x[j] - x[i]));
				if (y_sect == yp) return (GMT_ONEDGE);
				if (y_sect > yp) crossing_count++;
			}
		}
		if (x[i] > xp && x[j] < xp) {
			if (above)
				crossing_count--;
			else if ((j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ((xp - x[i]) / (x[j] - x[i]));
				if (y_sect == yp) return (GMT_ONEDGE);
				if (y_sect > yp) crossing_count--;
			}
		}

		/* That's it for this piece.  Advance i:  */

		i = j;
	}

	/* End of MAIN WHILE.  Get here when we have gone all around without landing on edge.  */

	return ((crossing_count) ? GMT_INSIDE: GMT_OUTSIDE);
}

GMT_LONG gmt_inonout_sphpol_count (double plon, double plat, const struct GMT_LINE_SEGMENT *P, GMT_LONG count[])
{	/* Case of a polar cap */
	GMT_LONG i, in, ip, cut;
	double W, E, S, N, lon, lon1, lon2, dlon, x_lat;

	/* Draw meridian through P and count all the crossings with the line segments making up the polar cap S */

	for (i = count[0] = count[1] = 0; i < P->n_rows - 1; i++) {	/* -1, since we know last point repeats the first */
		/* Here lon1 and lon2 are the end points (in longitude) of the current line segment in S.  There are
		 * four cases to worry about:
		 * 1) lon equals lon1 (i.e., the meridian through lon goes right through lon1)
		 * 2) lon equals lon2 (i.e., the meridian through lon goes right through lon2)
		 * 3) lon lies between lon1 and lon2 and crosses the segment
		 * 4) none of the above
		 * Since we want to obtain either ONE or ZERO intersections per segment we will skip to next
		 * point if case (2) occurs: this avoids counting a crossing twice for consequtive segments.
		 */
		in = i + 1;			/* Next point index */
		/* First skip duplicate consecutive points */
		/* if (GMT_IS_ZERO (P->coord[GMT_X][i] - P->coord[GMT_X][in]) && GMT_IS_ZERO (P->coord[GMT_Y][i] - P->coord[GMT_Y][in])) continue; */
		/* First skip segments that have no actual length: consecutive points with both latitudes == -90 or +90 */
		if (fabs (P->coord[GMT_Y][i]) == 90.0 && GMT_IS_ZERO (P->coord[GMT_Y][i] - P->coord[GMT_Y][in])) continue;
		/* First deal with case when the longitude of P goes ~right through the second of the line nodes */
		lon2 = P->coord[GMT_X][in];	/* Copy the second of two longitudes since we may need to mess with them */
		if (GMT_IS_ZERO (plon - lon2) || GMT_IS_ZERO (fabs(plon - lon2) - 360.0)) continue;	/* Line goes through the 2nd node - ignore */
		lon1 = P->coord[GMT_X][i];	/* Copy the first of two longitudes since we may need to mess with them */
		if (GMT_IS_ZERO (plon - lon1) || GMT_IS_ZERO (fabs(plon - lon1) - 360.0)) {		/* Line goes through the 1st node */
			/* Must check that the two neighboring points are on either side; otherwise it is just a tangent line */
			ip = (i == 0) ? P->n_rows - 2 : i - 1;	/* Previous point (-2 because last is a duplicate of first) */
			if ((lon2 >= lon1 && P->coord[GMT_X][ip] > lon1) || (lon2 <= lon1 && P->coord[GMT_X][ip] < lon1)) continue;	/* Both on same side */
			cut = (P->coord[GMT_Y][i] > plat) ? 0 : 1;	/* node is north (0) or south (1) of P */
			count[cut]++;
			continue;
		}
		/* OK, not exactly on a node, deal with crossing a line */
		dlon = lon2 - lon1;
		if (dlon > 180.0)		/* Jumped across Greenwich going westward */
			lon2 -= 360.0;
		else if (dlon < -180.0)		/* Jumped across Greenwich going eastward */
			lon1 -= 360.0;
		if (lon1 <= lon2) {	/* Segment goes W to E (or N-S) */
			W = lon1;
			E = lon2;
		}
		else {			/* Segment goes E to W */
			W = lon2;
			E = lon1;
		}
		lon = plon;			/* Local copy of plon, below adjusted given the segment lon range */
		while (lon > W) lon -= 360.0;	/* Make sure we rewind way west for starters */
		while (lon < W) lon += 360.0;	/* Then make sure we wind to inside the lon range or way east */
		if (lon > E) continue;	/* Not crossing this segment */
		if (dlon == 0.0) {	/* Special case of N-S segment: does P lie on it? */
			if (P->coord[GMT_Y][in] < P->coord[GMT_Y][i]) {	/* Get N and S limits for segment */
				S = P->coord[GMT_Y][in];
				N = P->coord[GMT_Y][i];
			}
			else {
				N = P->coord[GMT_Y][in];
				S = P->coord[GMT_Y][i];
			}
			if (plat < S || plat > N) continue;	/* P is not on this segment */
			return (1);	/* P is on segment boundary; we are done*/
		}
		/* Calculate latitude at intersection */
		x_lat = P->coord[GMT_Y][i] + ((P->coord[GMT_Y][in] - P->coord[GMT_Y][i]) / (lon2 - lon1)) * (lon - lon1);
		if (GMT_IS_ZERO (x_lat - plat)) return (1);	/* P is on S boundary */

		cut = (x_lat > plat) ? 0 : 1;	/* Cut is north (0) or south (1) of P */
		count[cut]++;
	}

	return (0);	/* This means no special cases were detected that warranted an immediate return */
}

GMT_LONG GMT_inonout_sphpol (struct GMT_CTRL *C, double plon, double plat, const struct GMT_LINE_SEGMENT *P)
/* This function is used to see if some point P is located inside, outside, or on the boundary of the
 * spherical polygon S read by GMT_read_table.
 * Returns the following values:
 *	0:	P is outside of S
 *	1:	P is inside of S
 *	2:	P is on boundary of S
 */
{
	/* Algorithm:
	 * Case 1: The polygon S contains a geographical pole
	 *	   a) if P is beyond the far latitude then P is outside
	 *	   b) Draw meridian through P and count intersections:
	 *		odd: P is outside; even: P is inside
	 * Case 2: S does not contain a pole
	 *	   a) If P is outside range of latitudes then P is outside
	 *	   c) Draw meridian through P and count intersections:
	 *		odd: P is inside; even: P is outside
	 * In all cases, we check if P is on the outline of S
	 */

	GMT_LONG count[2];

	if (P->pole) {	/* Case 1 of an enclosed polar cap */
		if (P->pole == +1) {	/* N polar cap */
			if (plat < P->min[GMT_Y]) return (GMT_OUTSIDE);	/* South of a N polar cap */
			if (plat > P->max[GMT_Y]) return (GMT_INSIDE);	/* Clearly inside of a N polar cap */
		}
		if (P->pole == -1) {	/* S polar cap */
			if (plat > P->max[GMT_Y]) return (GMT_OUTSIDE);	/* North of a S polar cap */
			if (plat < P->min[GMT_Y]) return (GMT_INSIDE);	/* North of a S polar cap */
		}

		/* Tally up number of intersections between polygon and meridian through P */

		if (gmt_inonout_sphpol_count (plon, plat, P, count)) return (GMT_ONEDGE);	/* Found P is on S */

		if (P->pole == +1 && count[0] % 2 == 0) return (GMT_INSIDE);
		if (P->pole == -1 && count[1] % 2 == 0) return (GMT_INSIDE);

		return (GMT_OUTSIDE);
	}

	/* Here is Case 2.  First check latitude range */

	if (plat < P->min[GMT_Y] || plat > P->max[GMT_Y]) return (GMT_OUTSIDE);

	/* Longitudes are tricker and are tested with the tallying of intersections */

	if (gmt_inonout_sphpol_count (plon, plat, P, count)) return (GMT_ONEDGE);	/* Found P is on S */

	if (count[0] % 2) return (GMT_INSIDE);

	return (GMT_OUTSIDE);	/* Nothing triggered the tests; we are outside */
}

GMT_LONG gmt_inonout_sub (struct GMT_CTRL *C, double x, double y, const struct GMT_LINE_SEGMENT *S)
{	/* Front end for both spherical and Cartesian in-on-out functions */
	GMT_LONG side;

	if (GMT_is_geographic (C, GMT_IN)) {	/* Assumes these are input polygons */
		if (S->pole)	/* 360-degree polar cap, must check fully */
			side = GMT_inonout_sphpol (C, x, y, S);
		else {	/* See if we are outside range of longitudes for polygon */
			while (x > S->min[GMT_X]) x -= 360.0;	/* Wind clear of west */
			while (x < S->min[GMT_X]) x += 360.0;	/* Wind east until inside or beyond east */
			if (x > S->max[GMT_X]) return (GMT_OUTSIDE);	/* Point outside, no need to assign value */
			side = GMT_inonout_sphpol (C, x, y, S);
		}
	}
	else {	/* Cartesian case */
		if (x < S->min[GMT_X] || x > S->max[GMT_X]) return (GMT_OUTSIDE);	/* Point outside, no need to assign value */
		side = GMT_non_zero_winding (C, x, y, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows);
	}
	return (side);
}

GMT_LONG GMT_inonout (struct GMT_CTRL *C, double x, double y, const struct GMT_LINE_SEGMENT *S)
{	/* Front end for both spherical and Cartesian in-on-out functions.
 	 * Knows to check for polygons with holes as well. */
	GMT_LONG side, side_h;
	struct GMT_LINE_SEGMENT *H = NULL;

	if ((side = gmt_inonout_sub (C, x, y, S)) <= GMT_ONEDGE) return (side);	/* Outside polygon or on perimeter, we are done */
	
	/* Here, point is inside the polygon perimeter. See if there are holes */

	if (C->current.io.OGR && (H = S->next)) {	/* Must check for and skip if inside a hole */		
		side_h = GMT_OUTSIDE;	/* We are outside a hole until we are found to be inside it */
		while (side_h == GMT_OUTSIDE && H && H->ogr && H->ogr->pol_mode == GMT_IS_HOLE) {	/* Found a hole */
			/* Must check if point is inside this hole polygon */
			side_h = gmt_inonout_sub (C, x, y, H);
			H = H->next;	/* Move to next polygon hole */
		}
		if (side_h == GMT_INSIDE) side = GMT_OUTSIDE;	/* Inside one of the holes, hence outside polygon; go to next perimeter polygon */
		if (side_h == GMT_ONEDGE) side = GMT_ONEDGE;	/* On path of one of the holes, hence on polygon path; update side */
	}
	
	/* Here, point is inside or on edge, we return the value */
	return (side);
}

/* GMT always compiles the standard Delaunay triangulation routine
 * based on the work by Dave Watson.  You may also link with the triangle.o
 * module from Jonathan Shewchuk, Berkeley U. by passing the compiler
 * directive -DTRIANGLE_D. The latter is much faster and also has options
 * for Voronoi construction (which Watson's funciton does not have).
 */

#ifdef TRIANGLE_D

/*
 * New GMT_delaunay interface routine that calls the triangulate function
 * developed by Jonathan Richard Shewchuk, University of California at Berkeley.
 * Suggested by alert GMT user Alain Coat.  You need to get triangle.c and
 * triangle.h from www.cs.cmu.edu/~quake/triangle.html
 */

#define REAL double
#include "triangle.h"

/* Leave link as int**, not GMT_LONG** */
GMT_LONG GMT_delaunay_shewchuk (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, int **link)
{
	/* GMT interface to the triangle package; see above for references.
	 * All that is done is reformatting of parameters and calling the
	 * main triangulate routine.  Thanx to Alain Coat for the tip.
	 */

	GMT_LONG i, j;
	struct triangulateio In, Out, vorOut;

	/* Set everything to 0 and NULL */

	GMT_memset (&In,     1, struct triangulateio);
	GMT_memset (&Out,    1, struct triangulateio);
	GMT_memset (&vorOut, 1, struct triangulateio);

	/* Allocate memory for input points */

	In.numberofpoints = (int)n;
	In.pointlist = GMT_memory (C, NULL, 2 * n, double);

	/* Copy x,y points to In structure array */

	for (i = j = 0; i < n; i++) {
		In.pointlist[j++] = x_in[i];
		In.pointlist[j++] = y_in[i];
	}

	/* Call Jonathan Shewchuk's triangulate algorithm.  This is 64-bit safe since
	 * all the structures use 4-byte ints (longs are used internally). */

	triangulate ("zIQB", &In, &Out, &vorOut);

	*link = Out.trianglelist;	/* List of node numbers to return via link [NOT ALLOCATED BY GMT_memory] */

	if (Out.pointlist) free (Out.pointlist);
	GMT_free (C, In.pointlist);

	return (Out.numberoftriangles);
}

GMT_LONG GMT_voronoi_shewchuk (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, double *we, double **x_out, double **y_out)
{
	/* GMT interface to the triangle package; see above for references.
	 * All that is done is reformatting of parameters and calling the
	 * main triangulate routine.  Here we return Voronoi information
	 * and package the coordinates of the edges in the output arrays.
	 * The we[] array contains the min/max x (or lon) coordinates.
	 */

	/* Currently we only write the edges of a Voronoi cell but we want polygons later.
	 * Info from triangle re Voronoi polygons: "Triangle does not write a list of
	 * the edges adjoining each Voronoi cell, but you can reconstructed it straightforwardly.
	 * For instance, to find all the edges of Voronoi cell 1, search the output .edge file
	 * for every edge that has input vertex 1 as an endpoint.  The corresponding dual
	 * edges in the output .v.edge file form the boundary of Voronoi cell 1." */

	GMT_LONG i, j, k, j2, n_edges;
	struct triangulateio In, Out, vorOut;
	double *x_edge = NULL, *y_edge = NULL, dy;

	/* Set everything to 0 and NULL */

	GMT_memset (&In,     1, struct triangulateio);
	GMT_memset (&Out,    1, struct triangulateio);
	GMT_memset (&vorOut, 1, struct triangulateio);

	/* Allocate memory for input points */

	In.numberofpoints = (int)n;
	In.pointlist = GMT_memory (C, NULL, 2 * n, double);

	/* Copy x,y points to In structure array */

	for (i = j = 0; i < n; i++) {
		In.pointlist[j++] = x_in[i];
		In.pointlist[j++] = y_in[i];
	}

	/* Call Jonathan Shewchuk's triangulate algorithm.  This is 64-bit safe since
	 * all the structures use 4-byte ints (longs are used internally). */

	triangulate ("zIQBv", &In, &Out, &vorOut);

	/* Determine output size for all edges */

	n_edges = vorOut.numberofedges;
	x_edge = GMT_memory (C, NULL, 2*n_edges, double);
	y_edge = GMT_memory (C, NULL, 2*n_edges, double);

	for (i = k = 0; i < n_edges; i++, k++) {
		/* Always start at a Voronoi vertex so j is never -1 */
		j2 = 2*vorOut.edgelist[k];
		x_edge[k] = vorOut.pointlist[j2++];
		y_edge[k++] = vorOut.pointlist[j2];
		if (vorOut.edgelist[k] == -1) {	/* Infinite ray; calc intersection with region boundary */
			j2--;	/* Previous point */
			x_edge[k] = (vorOut.normlist[k-1] < 0.0) ? we[0] : we[1];
			dy = fabs ((vorOut.normlist[k] / vorOut.normlist[k-1]) * (x_edge[k] - vorOut.pointlist[j2++]));
			y_edge[k] = vorOut.pointlist[j2] + dy * copysign (1.0, vorOut.normlist[k]);
		}
		else {
			j2 = 2*vorOut.edgelist[k];
			x_edge[k] = vorOut.pointlist[j2++];
			y_edge[k] = vorOut.pointlist[j2];
		}
	}

	*x_out = x_edge;	/* List of x-coordinates for all edges */
	*y_out = y_edge;	/* List of x-coordinates for all edges */

	if (Out.pointlist) free (Out.pointlist);
	if (vorOut.pointlist) free (vorOut.pointlist);
	if (vorOut.edgelist) free (vorOut.edgelist);
	if (vorOut.normlist) free (vorOut.normlist);
	GMT_free (C, In.pointlist);

	return (n_edges);
}
#else
/* Dummy functions since not installed */
GMT_LONG GMT_delaunay_shewchuk (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, int **link)
{
	GMT_report (C, GMT_MSG_FATAL, "GMT: GMT_delaunay_shewchuk is unavailable: Shewchuk's triangle option was not selected during GMT installation");
	return (0);
	
}
GMT_LONG GMT_voronoi_shewchuk (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, double *we, double **x_out, double **y_out) {
	GMT_report (C, GMT_MSG_FATAL, "GMT: GMT_voronoi_shewchuk is unavailable: Shewchuk's triangle option was not selected during GMT installation");
	return (0);
}
#endif

/*
 * GMT_delaunay performs a Delaunay triangulation on the input data
 * and returns a list of indices of the points for each triangle
 * found.  Algorithm translated from
 * Watson, D. F., ACORD: Automatic contouring of raw data,
 *   Computers & Geosciences, 8, 97-101, 1982.
 */

/* Leave link as int**, not GMT_LONG** */
GMT_LONG GMT_delaunay_watson (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, int **link)
	/* Input point x coordinates */
	/* Input point y coordinates */
	/* Number of input points */
	/* pointer to List of point ids per triangle.  Vertices for triangle no i
	   is in link[i*3], link[i*3+1], link[i*3+2] */
{
	int *index = NULL;	/* Must be int not GMT_LONG */
	GMT_LONG ix[3], iy[3];
	GMT_LONG i, j, ij, nuc, jt, km, id, isp, l1, l2, k, k1, jz, i2, kmt, kt, done, size;
	GMT_LONG *istack = NULL, *x_tmp = NULL, *y_tmp = NULL;
	double det[2][3], *x_circum = NULL, *y_circum = NULL, *r2_circum = NULL, *x = NULL, *y = NULL;
	double xmin, xmax, ymin, ymax, datax, dx, dy, dsq, dd;

	size = 10 * n + 1;
	n += 3;

	index = GMT_memory (C, NULL, 3 * size, int);
	istack = GMT_memory (C, NULL, size, GMT_LONG);
	x_tmp = GMT_memory (C, NULL, size, GMT_LONG);
	y_tmp = GMT_memory (C, NULL, size, GMT_LONG);
	x_circum = GMT_memory (C, NULL, size, double);
	y_circum = GMT_memory (C, NULL, size, double);
	r2_circum = GMT_memory (C, NULL, size, double);
	x = GMT_memory (C, NULL, n, double);
	y = GMT_memory (C, NULL, n, double);

	x[0] = x[1] = -1.0;	x[2] = 5.0;
	y[0] = y[2] = -1.0;	y[1] = 5.0;
	x_circum[0] = y_circum[0] = 2.0;	r2_circum[0] = 18.0;

	ix[0] = ix[1] = 0;	ix[2] = 1;
	iy[0] = 1;	iy[1] = iy[2] = 2;

	for (i = 0; i < 3; i++) index[i] = (int)i;
	for (i = 0; i < size; i++) istack[i] = i;

	xmin = ymin = 1.0e100;	xmax = ymax = -1.0e100;

	for (i = 3, j = 0; i < n; i++, j++) {	/* Copy data and get extremas */
		x[i] = x_in[j];
		y[i] = y_in[j];
		if (x[i] > xmax) xmax = x[i];
		if (x[i] < xmin) xmin = x[i];
		if (y[i] > ymax) ymax = y[i];
		if (y[i] < ymin) ymin = y[i];
	}

	/* Normalize data */

	datax = 1.0 / MAX (xmax - xmin, ymax - ymin);

	for (i = 3; i < n; i++) {
		x[i] = (x[i] - xmin) * datax;
		y[i] = (y[i] - ymin) * datax;
	}

	isp = id = 1;
	for (nuc = 3; nuc < n; nuc++) {

		km = 0;

		for (jt = 0; jt < isp; jt++) {	/* loop through established 3-tuples */

			ij = 3 * jt;

			/* Test if new data is within the jt circumcircle */

			dx = x[nuc] - x_circum[jt];
			if ((dsq = r2_circum[jt] - dx * dx) < 0.0) continue;
			dy = y[nuc] - y_circum[jt];
			if ((dsq -= dy * dy) < 0.0) continue;

			/* Delete this 3-tuple but save its edges */

			id--;
			istack[id] = jt;

			/* Add edges to x/y_tmp but delete if already listed */

			for (i = 0; i < 3; i++) {
				l1 = ix[i];
				l2 = iy[i];
				if (km > 0) {
					kmt = km;
					for (j = 0, done = FALSE; !done && j < kmt; j++) {
						if (index[ij+l1] != x_tmp[j]) continue;
						if (index[ij+l2] != y_tmp[j]) continue;
						km--;
						if (j >= km) {
							done = TRUE;
							continue;
						}
						for (k = j; k < km; k++) {
							k1 = k + 1;
							x_tmp[k] = x_tmp[k1];
							y_tmp[k] = y_tmp[k1];
							done = TRUE;
						}
					}
				}
				else
					done = FALSE;
				if (!done) {
					x_tmp[km] = index[ij+l1];
					y_tmp[km] = index[ij+l2];
					km++;
				}
			}
		}

		/* Form new 3-tuples */

		for (i = 0; i < km; i++) {
			kt = istack[id];
			ij = 3 * kt;
			id++;

			/* Calculate the circumcircle center and radius
			 * squared of points ktetr[i,*] and place in tetr[kt,*] */

			for (jz = 0; jz < 2; jz++) {
				i2 = (jz == 0) ? x_tmp[i] : y_tmp[i];
				det[jz][0] = x[i2] - x[nuc];
				det[jz][1] = y[i2] - y[nuc];
				det[jz][2] = 0.5 * (det[jz][0] * (x[i2] + x[nuc]) + det[jz][1] * (y[i2] + y[nuc]));
			}
			dd = 1.0 / (det[0][0] * det[1][1] - det[0][1] * det[1][0]);
			x_circum[kt] = (det[0][2] * det[1][1] - det[1][2] * det[0][1]) * dd;
			y_circum[kt] = (det[0][0] * det[1][2] - det[1][0] * det[0][2]) * dd;
			dx = x[nuc] - x_circum[kt];
			dy = y[nuc] - y_circum[kt];
			r2_circum[kt] = dx * dx + dy * dy;
			index[ij++] = (int)x_tmp[i];
			index[ij++] = (int)y_tmp[i];
			index[ij] = (int)nuc;
		}
		isp += 2;
	}

	for (jt = i = 0; jt < isp; jt++) {
		ij = 3 * jt;
		if (index[ij] < 3 || r2_circum[jt] > 1.0) continue;
		index[i++] = index[ij++] - 3;
		index[i++] = index[ij++] - 3;
		index[i++] = index[ij++] - 3;
	}

	index = GMT_memory (C, index, i, int);
	*link = index;

	GMT_free (C, istack);
	GMT_free (C, x_tmp);
	GMT_free (C, y_tmp);
	GMT_free (C, x_circum);
	GMT_free (C, y_circum);
	GMT_free (C, r2_circum);
	GMT_free (C, x);
	GMT_free (C, y);

	return (i/3);
}

GMT_LONG GMT_voronoi_watson (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, double *we, double **x_out, double **y_out)
{
	GMT_report (C, GMT_MSG_FATAL, "GMT: No Voronoi unless you select Shewchuk's triangle option during GMT installation");
	return (0);
}

GMT_LONG GMT_delaunay (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, int **link)
{
	if (C->current.setting.triangulate == GMT_TRIANGLE_SHEWCHUK) return (GMT_delaunay_shewchuk (C, x_in, y_in, n, link));
	if (C->current.setting.triangulate == GMT_TRIANGLE_WATSON)   return (GMT_delaunay_watson    (C, x_in, y_in, n, link));
	GMT_report (C, GMT_MSG_FATAL, "GMT: GMT_delaunay: GMT_TRIANGULATE outside possible range! %ld\n", C->current.setting.triangulate);
	return (-1);
}

GMT_LONG GMT_voronoi (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, double *we, double **x_out, double **y_out)
{
	if (C->current.setting.triangulate == GMT_TRIANGLE_SHEWCHUK) return (GMT_voronoi_shewchuk (C, x_in, y_in, n, we, x_out, y_out));
	if (C->current.setting.triangulate == GMT_TRIANGLE_WATSON)   return (GMT_voronoi_watson    (C, x_in, y_in, n, we, x_out, y_out));
	GMT_report (C, GMT_MSG_FATAL, "GMT: GMT_voronoi: GMT_TRIANGULATE outside possible range! %ld\n", C->current.setting.triangulate);
	return (-1);
	
}
/*
 * This section holds functions used for setting boundary  conditions in
 * processing grd file data.
 *
 * This is a new feature of GMT v3.1.   My first draft of this (April 1998)
 * set only one row of padding.  Additional thought showed that the bilinear
 * invocation of bcr routines would not work properly on periodic end conditions
 * in this case.  So second draft (May 5-6, 1998) will set two rows of padding,
 * and I will also have to edit bcr so that it works for this case.  The GMT
 * bcr routines are currently used in grdsample, grdtrack, and grdview.
 *
 * I anticipate that later (GMT v5 ?) this code could (?) be modified to also
 * handle the boundary conditions needed by surface.
 *
 * The default boundary condition is derived from application of Green's
 * theorem to the conditions for minimizing curvature:
 * laplacian (f) = 0 on edges, and d[laplacian(f)]/dn = 0 on edges, where
 * n is normal to an edge.  We also set d2f/dxdy = 0 at corners.
 *
 * The new routines here allow the user to choose other specifications:
 *
 * Either
 *	one or both of
 *	data are periodic in (x_max - x_min)
 *	data are periodic in (y_max - y_min)
 *
 * Or
 *	data are a geographical grid.
 *
 * Periodicities assume that the min,max are compatible with the inc;
 * that is, (x_max - x_min)modulo(x_inc) ~= 0.0 within precision tolerances,
 * and similarly for y.  It is assumed that this is OK and that GMT_grd_RI_verify
 * was called during read grd and found to be OK.
 *
 * In the geographical case, if x_max - x_min < 360 we will use the default
 * boundary conditions, but if x_max - x_min >= 360 the 360 periodicity in x
 * will be used to set the x boundaries, and so we require 360modulo(x_inc)
 * == 0.0 within precision tolerance.  If y_max != 90 the north edge will
 * default, and similarly for y_min != -90.  If a y edge is at a pole and
 * x_max - x_min >= 360 then the geographical y uses a 180 degree phase
 * shift in the values, so we require 180modulo(x_inc) == 0.
 * Note that a grid-registered file will require that the entire row of
 * values representing a pole must all be equal, else d/dx != 0 which
 * is wrong.  So compatibility error-checking is built in.
 *
 * Note that a periodicity or polar boundary eliminates the need for
 * d2/dxdy = 0 at a corner.  There are no "corners" in those cases.
 *
 * Author:	W H F Smith
 * Date:	17 April 1998
 * Revised:	5  May 1998
 * Notes PW, April-2011: BCs are now set after a grid or image is read,
 * thus all programs load grids with all BCs set. Contents of old structs
 * GMT_BCR and GMT_EDGEINFO folded into GRDHEADER. Remaining functions
 * were renamed and are now
 * 	GMT_BC_init		- Determines and sets what BCs to use
 *	GMT_grd_BC_set		- Sets the BCs on a grid
 *	GMT_image_BC_set	- Sets the BCs on an image
 */

GMT_LONG GMT_BC_init (struct GMT_CTRL *C, struct GRD_HEADER *h)
{	/* Initialize grid boundary conditions based on grid header and -n settings */
	GMT_LONG i = 0, type;
	char *kind[5] = {"not set", "natural", "periodic", "geographic", "extended data"};
	
	if (h->no_BC) return (GMT_NOERROR);	/* Told not to deal with BC stuff */
	
	if (C->common.n.bc_set) {	/* Override BCs via -n+<BC> */
		while (C->common.n.BC[i]) {
			switch (C->common.n.BC[i]) {
				case 'g':	/* Geographic sets everything */
					h->gn = h->gs = TRUE;
					h->BC[0] = h->BC[1] = h->BC[2] = h->BC[3] = GMT_BC_IS_POLE;
					break;
				case 'n':	/* Natural BCs */
					if (C->common.n.BC[i+1] == 'x') { h->BC[0] = h->BC[1] = GMT_BC_IS_NATURAL; i++; }
					else if (C->common.n.BC[i+1] == 'y') { h->BC[2] = h->BC[3] = GMT_BC_IS_NATURAL; i++; }
					else h->BC[0] = h->BC[1] = h->BC[2] = h->BC[3] = GMT_BC_IS_NATURAL;
					break;
				case 'p':	/* Periodic BCs */
					if (C->common.n.BC[i+1] == 'x') { h->BC[0] = h->BC[1] = GMT_BC_IS_PERIODIC; h->nxp = -1; i++; }
					else if (C->common.n.BC[i+1] == 'y') { h->BC[2] = h->BC[3] = GMT_BC_IS_PERIODIC; h->nyp = -1; i++; }
					else { h->BC[0] = h->BC[1] = h->BC[2] = h->BC[3] = GMT_BC_IS_PERIODIC; h->nxp = h->nyp = -1; }
					break;
				default:
					GMT_report (C, GMT_MSG_FATAL, "Error: Cannot parse boundary condition %s\n", C->common.n.BC);
					return (-1);
					break;
			}
			i++;
		}
		if (h->gn && !(h->BC[0] == GMT_BC_IS_POLE && h->BC[1] == GMT_BC_IS_POLE && h->BC[2] == GMT_BC_IS_POLE && h->BC[3] == GMT_BC_IS_POLE)) {
			GMT_report (C, GMT_MSG_FATAL, "Warning: GMT boundary condition g overrides n[x|y] or p[x|y]\n");
			h->BC[0] = h->BC[1] = h->BC[2] = h->BC[3] = GMT_BC_IS_POLE;
		}
	}
	else {	/* Determine BC based on whether grid is geographic or not */
		type = (GMT_x_is_lon (C, GMT_IN)) ? GMT_BC_IS_POLE : GMT_BC_IS_NATURAL;
		for (i = 0; i < 4; i++) if (h->BC[i] == GMT_BC_IS_NOTSET) h->BC[i] = type;
	}

	/* Check if geographic conditions can be used with this grid */
	if (h->gn && !GMT_grd_is_global (C, h)) {
		/* User has requested geographical conditions, but grid is not global */
		GMT_report (C, GMT_MSG_VERBOSE, "Warning: longitude range too small; geographic boundary condition changed to natural.\n");
		h->nxp = h->nyp = 0;
		h->gn  = h->gs = FALSE;
		for (i = 0; i < 4; i++) if (h->BC[i] == GMT_BC_IS_NOTSET) h->BC[i] = GMT_BC_IS_NATURAL;
	}
	else if (GMT_grd_is_global (C, h)) {	/* Grid is truly global */
		double xtest = fmod (180.0, h->inc[GMT_X]) * h->r_inc[GMT_X];
		/* xtest should be within GMT_SMALL of zero or of one.  */
		if (xtest > GMT_SMALL && xtest < (1.0 - GMT_SMALL) ) {
			/* Error.  We need it to divide into 180 so we can phase-shift at poles.  */
			GMT_report (C, GMT_MSG_VERBOSE, "Warning: x_inc does not divide 180; geographic boundary condition changed to natural.\n");
			h->nxp = h->nyp = 0;
			h->gn  = h->gs = FALSE;
			for (i = 0; i < 4; i++) if (h->BC[i] == GMT_BC_IS_NOTSET) h->BC[i] = GMT_BC_IS_NATURAL;
		}
		else {
			h->nxp = irint (360.0 * h->r_inc[GMT_X]);
			h->nyp = 0;
			h->gn = ((fabs(h->wesn[YHI] - 90.0)) < (GMT_SMALL * h->inc[GMT_Y]));
			h->gs = ((fabs(h->wesn[YLO] + 90.0)) < (GMT_SMALL * h->inc[GMT_Y]));
			if (!h->gs) h->BC[2] = GMT_BC_IS_NATURAL;
			if (!h->gn) h->BC[3] = GMT_BC_IS_NATURAL;
		}
	}
	else {	/* Either periodic or natural */
		if (h->nxp != 0) h->nxp = (h->registration == GMT_PIXEL_REG) ? h->nx : h->nx - 1;
		if (h->nyp != 0) h->nyp = (h->registration == GMT_PIXEL_REG) ? h->ny : h->ny - 1;
	}
	GMT_report (C, GMT_MSG_VERBOSE, "Chosen boundary condition for left   edge: %s\n", kind[h->BC[0]]);
	GMT_report (C, GMT_MSG_VERBOSE, "Chosen boundary condition for right  edge: %s\n", kind[h->BC[1]]);
	GMT_report (C, GMT_MSG_VERBOSE, "Chosen boundary condition for bottom edge: %s\n", kind[h->BC[2]]);
	GMT_report (C, GMT_MSG_VERBOSE, "Chosen boundary condition for top    edge: %s\n", kind[h->BC[3]]);

	/* Set this grid's interpolation parameters */

	h->bcr_interpolant = C->common.n.interpolant;
	h->bcr_threshold = C->common.n.threshold;
	h->bcr_n = (h->bcr_interpolant == BCR_NEARNEIGHBOR) ? 1 : ((h->bcr_interpolant == BCR_BILINEAR) ? 2 : 4);
	
	return (GMT_NOERROR);
}

GMT_LONG GMT_grd_BC_set (struct GMT_CTRL *C, struct GMT_GRID *G)
{
	/* Set two rows of padding (pad[] can be larger) around data according
	   to desired boundary condition info in that header.
	   Returns -1 on problem, 0 on success.
	   If either x or y is periodic, the padding is entirely set.
	   However, if neither is true (this rules out geographical also)
	   then all but three corner-most points in each corner are set.

	   As written, not ready to use with "surface" for GMT 5, because
	   assumes left/right is +/- 1 and down/up is +/- mx.  In "surface"
	   the amount to move depends on the current mesh size, a parameter
	   not used here.

	   This is the revised, two-rows version (WHFS 6 May 1998).
	*/

	GMT_LONG bok;		/* Counter used to test that things are OK  */
	GMT_LONG mx;		/* Width of padded array; width as malloc'ed  */
	GMT_LONG mxnyp;		/* distance to periodic constraint in j direction  */
	GMT_LONG i, jmx;	/* Current i, j * mx  */
	GMT_LONG nxp2;		/* 1/2 the xg period (180 degrees) in cells  */
	GMT_LONG i180;		/* index to 180 degree phase shift  */
	GMT_LONG iw, iwo1, iwo2, iwi1, ie, ieo1, ieo2, iei1;  /* see below  */
	GMT_LONG jn, jno1, jno2, jni1, js, jso1, jso2, jsi1;  /* see below  */
	GMT_LONG jno1k, jno2k, jso1k, jso2k, iwo1k, iwo2k, ieo1k, ieo2k;
	GMT_LONG j1p, j2p;	/* j_o1 and j_o2 pole constraint rows  */
	GMT_LONG n_skip, set[4] = {TRUE, TRUE, TRUE, TRUE};
	char *kind[5] = {"not set", "natural", "periodic", "geographic", "extended data"};
	char *edge[4] = {"left  ", "right ", "bottom", "top   "};

	if (G->header->no_BC) return (GMT_NOERROR);	/* Told not to deal with BC stuff */

	for (i = n_skip = 0; i < 4; i++) {
		if (G->header->BC[i] == GMT_BC_IS_DATA) {set[i] = FALSE; n_skip++;}	/* No need to set since there is data in the pad area */
	}
	if (n_skip == 4) {	/* No need to set anything since there is data in the pad area on all sides */
		GMT_report (C, GMT_MSG_VERBOSE, "GMT_boundcond_grd_set: All boundaries set via extended data.\n");
		return (GMT_NOERROR);
	}

	/* Check minimum size:  */
	if (G->header->nx < 1 || G->header->ny < 1) {
		GMT_report (C, GMT_MSG_VERBOSE, "GMT_boundcond_grd_set requires nx,ny at least 1.\n");
		return (GMT_NOERROR);
	}

	/* Check that pad is at least 2 */
	for (i = bok = 0; i < 4; i++) if (G->header->pad[i] < 2) bok++;
	if (bok > 0) {
		GMT_report (C, GMT_MSG_VERBOSE, "GMT_boundcond_grd_set called with a pad < 2; skipped.\n");
		return (GMT_NOERROR);
	}

	/* Initialize stuff:  */

	mx = G->header->mx;
	nxp2 = G->header->nxp / 2;	/* Used for 180 phase shift at poles  */

	iw = G->header->pad[XLO];	/* i for west-most data column */
	iwo1 = iw - 1;		/* 1st column outside west  */
	iwo2 = iwo1 - 1;	/* 2nd column outside west  */
	iwi1 = iw + 1;		/* 1st column  inside west  */

	ie = G->header->pad[XLO] + G->header->nx - 1;	/* i for east-most data column */
	ieo1 = ie + 1;		/* 1st column outside east  */
	ieo2 = ieo1 + 1;	/* 2nd column outside east  */
	iei1 = ie - 1;		/* 1st column  inside east  */

	jn = mx * G->header->pad[YHI];	/* j*mx for north-most data row  */
	jno1 = jn - mx;		/* 1st row outside north  */
	jno2 = jno1 - mx;	/* 2nd row outside north  */
	jni1 = jn + mx;		/* 1st row  inside north  */

	js = mx * (G->header->pad[YHI] + G->header->ny - 1);	/* j*mx for south-most data row  */
	jso1 = js + mx;		/* 1st row outside south  */
	jso2 = jso1 + mx;	/* 2nd row outside south  */
	jsi1 = js - mx;		/* 1st row  inside south  */

	mxnyp = mx * G->header->nyp;

	jno1k = jno1 + mxnyp;	/* data rows periodic to boundary rows  */
	jno2k = jno2 + mxnyp;
	jso1k = jso1 - mxnyp;
	jso2k = jso2 - mxnyp;

	iwo1k = iwo1 + G->header->nxp;	/* data cols periodic to bndry cols  */
	iwo2k = iwo2 + G->header->nxp;
	ieo1k = ieo1 - G->header->nxp;
	ieo2k = ieo2 - G->header->nxp;

	/* Duplicate rows and columns if nx or ny equals 1 */

	if (G->header->nx == 1) for (i = jn+iw; i <= js+iw; i += mx) G->data[i-1] = G->data[i+1] = G->data[i];
	if (G->header->ny == 1) for (i = jn+iw; i <= jn+ie; i++) G->data[i-mx] = G->data[i+mx] = G->data[i];

	/* Check poles for grid case.  It would be nice to have done this
		in GMT_boundcond_param_prep() but at that point the data
		array isn't passed into that routine, and may not have been
		read yet.  Also, as coded here, this bombs with error if
		the pole data is wrong.  But there could be an option to
		to change the condition to Natural in that case, with warning.  */

	if (G->header->registration == GMT_GRIDLINE_REG) {	/* A pole can only be a grid node with gridline registration */
		if (G->header->gn) {	/* North pole case */
			bok = 0;
			if (GMT_is_fnan (G->data[jn + iw])) {	/* First is NaN so all should be NaN */
				for (i = iw+1; i <= ie; i++) if (!GMT_is_fnan (G->data[jn + i])) bok++;
			}
			else {	/* First is not NaN so all should be identical */
				for (i = iw+1; i <= ie; i++) if (G->data[jn + i] != G->data[jn + iw]) bok++;
			}
			if (bok > 0) GMT_report (C, GMT_MSG_NORMAL, "Warning: %ld (of %d) inconsistent grid values at North pole.\n", bok, G->header->nx);
		}

		if (G->header->gs) {	/* South pole case */
			bok = 0;
			if (GMT_is_fnan (G->data[js + iw])) {	/* First is NaN so all should be NaN */
				for (i = iw+1; i <= ie; i++) if (!GMT_is_fnan (G->data[js + i])) bok++;
			}
			else {	/* First is not NaN so all should be identical */
				for (i = iw+1; i <= ie; i++) if (G->data[js + i] != G->data[js + iw]) bok++;
			}
			if (bok > 0) GMT_report (C, GMT_MSG_NORMAL, "Warning: %ld (of %d) inconsistent grid values at South pole.\n", bok, G->header->nx);
		}
	}

	/* Start with the case that x is not periodic, because in that case we also know that y cannot be polar.  */

	if (G->header->nxp <= 0) {	/* x is not periodic  */

		if (G->header->nyp > 0) {	/* y is periodic  */

			for (i = iw, bok = 0; i <= ie; i++) {
				if (G->header->registration == GMT_GRIDLINE_REG && !GMT_IS_ZERO (G->data[jn+i] - G->data[js+i])) bok++;
				if (set[YHI]) {
					G->data[jno1 + i] = G->data[jno1k + i];
					G->data[jno2 + i] = G->data[jno2k + i];
				}
				if (set[YLO]) {
					G->data[jso1 + i] = G->data[jso1k + i];
					G->data[jso2 + i] = G->data[jso2k + i];
				}
			}
			if (bok > 0) GMT_report (C, GMT_MSG_NORMAL, "Warning: %ld (of %d) inconsistent grid values at South and North boundaries for repeated nodes.\n", bok, G->header->nx);

			/* periodic Y rows copied.  Now do X naturals.
				This is easy since y's are done; no corner problems.
				Begin with Laplacian = 0, and include 1st outside rows
				in loop, since y's already loaded to 2nd outside.  */

			for (jmx = jno1; jmx <= jso1; jmx += mx) {
				if (set[XLO]) G->data[jmx + iwo1] = (float)(4.0 * G->data[jmx + iw]) - (G->data[jmx + iw + mx] + G->data[jmx + iw - mx] + G->data[jmx + iwi1]);
				if (set[XHI]) G->data[jmx + ieo1] = (float)(4.0 * G->data[jmx + ie]) - (G->data[jmx + ie + mx] + G->data[jmx + ie - mx] + G->data[jmx + iei1]);
			}

			/* Copy that result to 2nd outside row using periodicity.  */
			if (set[XLO]) {
				G->data[jno2 + iwo1] = G->data[jno2k + iwo1];
				G->data[jso2 + iwo1] = G->data[jso2k + iwo1];
			}
			if (set[XHI]) {
				G->data[jno2 + ieo1] = G->data[jno2k + ieo1];
				G->data[jso2 + ieo1] = G->data[jso2k + ieo1];
			}

			/* Now set d[laplacian]/dx = 0 on 2nd outside column.  Include 1st outside rows in loop.  */
			for (jmx = jno1; jmx <= jso1; jmx += mx) {
				if (set[XLO]) G->data[jmx + iwo2] = (G->data[jmx + iw - mx] + G->data[jmx + iw + mx] + G->data[jmx + iwi1])
					- (G->data[jmx + iwo1 - mx] + G->data[jmx + iwo1 + mx]) + (float)(5.0 * (G->data[jmx + iwo1] - G->data[jmx + iw]));
				if (set[XHI]) G->data[jmx + ieo2] = (G->data[jmx + ie - mx] + G->data[jmx + ie + mx] + G->data[jmx + iei1])
					- (G->data[jmx + ieo1 - mx] + G->data[jmx + ieo1 + mx]) + (float)(5.0 * (G->data[jmx + ieo1] - G->data[jmx + ie]));
			}

			/* Now copy that result also, for complete periodicity's sake  */
			if (set[XLO]) {
				G->data[jno2 + iwo2] = G->data[jno2k + iwo2];
				G->data[jso2 + iwo2] = G->data[jso2k + iwo2];
				G->header->BC[XLO] = GMT_BC_IS_NATURAL;
			}
			if (set[XHI]) {
				G->data[jno2 + ieo2] = G->data[jno2k + ieo2];
				G->data[jso2 + ieo2] = G->data[jso2k + ieo2];
				G->header->BC[XHI] = GMT_BC_IS_NATURAL;
			}

			/* DONE with X not periodic, Y periodic case.  Fully loaded.  */
			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}

			return (GMT_NOERROR);
		}
		else {	/* Here begins the X not periodic, Y not periodic case  */

			/* First, set corner points.  Need not merely Laplacian(f) = 0
				but explicitly that d2f/dx2 = 0 and d2f/dy2 = 0.
				Also set d2f/dxdy = 0.  Then can set remaining points.  */

	/* d2/dx2 */	if (set[XLO]) G->data[jn + iwo1]   = (float)(2.0 * G->data[jn + iw] - G->data[jn + iwi1]);
	/* d2/dy2 */	if (set[YHI]) G->data[jno1 + iw]   = (float)(2.0 * G->data[jn + iw] - G->data[jni1 + iw]);
	/* d2/dxdy */	if (set[XLO] && set[YHI]) G->data[jno1 + iwo1] = -(G->data[jno1 + iwi1] - G->data[jni1 + iwi1] + G->data[jni1 + iwo1]);

	/* d2/dx2 */	if (set[XHI]) G->data[jn + ieo1]   = (float)(2.0 * G->data[jn + ie] - G->data[jn + iei1]);
	/* d2/dy2 */	if (set[YHI]) G->data[jno1 + ie]   = (float)(2.0 * G->data[jn + ie] - G->data[jni1 + ie]);
	/* d2/dxdy */	if (set[XHI] && set[YHI]) G->data[jno1 + ieo1] = -(G->data[jno1 + iei1] - G->data[jni1 + iei1] + G->data[jni1 + ieo1]);

	/* d2/dx2 */	if (set[XLO]) G->data[js + iwo1]   = (float)(2.0 * G->data[js + iw] - G->data[js + iwi1]);
	/* d2/dy2 */	if (set[YLO]) G->data[jso1 + iw]   = (float)(2.0 * G->data[js + iw] - G->data[jsi1 + iw]);
	/* d2/dxdy */	if (set[XLO] && set[YLO]) G->data[jso1 + iwo1] = -(G->data[jso1 + iwi1] - G->data[jsi1 + iwi1] + G->data[jsi1 + iwo1]);

	/* d2/dx2 */	if (set[XHI]) G->data[js + ieo1]   = (float)(2.0 * G->data[js + ie] - G->data[js + iei1]);
	/* d2/dy2 */	if (set[YLO]) G->data[jso1 + ie]   = (float)(2.0 * G->data[js + ie] - G->data[jsi1 + ie]);
	/* d2/dxdy */	if (set[XHI] && set[YLO]) G->data[jso1 + ieo1] = -(G->data[jso1 + iei1] - G->data[jsi1 + iei1] + G->data[jsi1 + ieo1]);

			/* Now set Laplacian = 0 on interior edge points, skipping corners:  */
			for (i = iwi1; i <= iei1; i++) {
				if (set[YHI]) G->data[jno1 + i] = (float)(4.0 * G->data[jn + i]) - (G->data[jn + i - 1] + G->data[jn + i + 1] + G->data[jni1 + i]);
				if (set[YLO]) G->data[jso1 + i] = (float)(4.0 * G->data[js + i]) - (G->data[js + i - 1] + G->data[js + i + 1] + G->data[jsi1 + i]);
			}
			for (jmx = jni1; jmx <= jsi1; jmx += mx) {
				if (set[XLO]) G->data[iwo1 + jmx] = (float)(4.0 * G->data[iw + jmx]) - (G->data[iw + jmx + mx] + G->data[iw + jmx - mx] + G->data[iwi1 + jmx]);
				if (set[XHI]) G->data[ieo1 + jmx] = (float)(4.0 * G->data[ie + jmx]) - (G->data[ie + jmx + mx] + G->data[ie + jmx - mx] + G->data[iei1 + jmx]);
			}

			/* Now set d[Laplacian]/dn = 0 on all edge pts, including
				corners, since the points needed in this are now set.  */
			for (i = iw; i <= ie; i++) {
				if (set[YHI]) G->data[jno2 + i] = G->data[jni1 + i] + (float)(5.0 * (G->data[jno1 + i] - G->data[jn + i]))
					+ (G->data[jn + i - 1] - G->data[jno1 + i - 1]) + (G->data[jn + i + 1] - G->data[jno1 + i + 1]);
				if (set[YLO]) G->data[jso2 + i] = G->data[jsi1 + i] + (float)(5.0 * (G->data[jso1 + i] - G->data[js + i]))
					+ (G->data[js + i - 1] - G->data[jso1 + i - 1]) + (G->data[js + i + 1] - G->data[jso1 + i + 1]);
			}
			for (jmx = jn; jmx <= js; jmx += mx) {
				if (set[XLO]) G->data[iwo2 + jmx] = G->data[iwi1 + jmx] + (float)(5.0 * (G->data[iwo1 + jmx] - G->data[iw + jmx]))
					+ (G->data[iw + jmx - mx] - G->data[iwo1 + jmx - mx]) + (G->data[iw + jmx + mx] - G->data[iwo1 + jmx + mx]);
				if (set[XHI]) G->data[ieo2 + jmx] = G->data[iei1 + jmx] + (float)(5.0 * (G->data[ieo1 + jmx] - G->data[ie + jmx]))
					+ (G->data[ie + jmx - mx] - G->data[ieo1 + jmx - mx]) + (G->data[ie + jmx + mx] - G->data[ieo1 + jmx + mx]);
			}
			/* DONE with X not periodic, Y not periodic case.  Loaded all but three cornermost points at each corner.  */

			for (i = 0; i < 4; i++) if (set[i]) {
				G->header->BC[i] = GMT_BC_IS_NATURAL;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[i], kind[G->header->BC[i]]);
			}
			return (GMT_NOERROR);
		}
		/* DONE with all X not periodic cases  */
	}
	else {	/* X is periodic.  Load x cols first, then do Y cases.  */
		if (set[XLO]) G->header->BC[XLO] = GMT_BC_IS_PERIODIC;
		if (set[XHI]) G->header->BC[XHI] = GMT_BC_IS_PERIODIC;
		for (jmx = jn, bok = 0; jmx <= js; jmx += mx) {
			if (G->header->registration == GMT_GRIDLINE_REG && !GMT_IS_ZERO (G->data[jmx+iw] - G->data[jmx+ie])) bok++;
			if (set[XLO]) {
				G->data[iwo1 + jmx] = G->data[iwo1k + jmx];
				G->data[iwo2 + jmx] = G->data[iwo2k + jmx];
			}
			if (set[XHI]) {
				G->data[ieo1 + jmx] = G->data[ieo1k + jmx];
				G->data[ieo2 + jmx] = G->data[ieo2k + jmx];
			}
		}
		if (bok > 0) GMT_report (C, GMT_MSG_NORMAL, "Warning: %ld (of %d) inconsistent grid values at West and East boundaries for repeated nodes.\n", bok, G->header->ny);

		if (G->header->nyp > 0) {	/* Y is periodic.  copy all, including boundary cols:  */
			for (i = iwo2, bok = 0; i <= ieo2; i++) {
				if (G->header->registration == GMT_GRIDLINE_REG && !GMT_IS_ZERO (G->data[jn+i] - G->data[js+i])) bok++;
				if (set[YHI]) {
					G->data[jno1 + i] = G->data[jno1k + i];
					G->data[jno2 + i] = G->data[jno2k + i];
				}
				if (set[YLO]) {
					G->data[jso1 + i] = G->data[jso1k + i];
					G->data[jso2 + i] = G->data[jso2k + i];
				}
			}
			if (bok > 0) GMT_report (C, GMT_MSG_NORMAL, "Warning: %ld (of %d) inconsistent grid values at South and North boundaries for repeated nodes.\n", bok, G->header->nx);
			/* DONE with X and Y both periodic.  Fully loaded.  */

			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}
			return (GMT_NOERROR);
		}

		/* Do north (top) boundary:  */

		if (G->header->gn) {	/* Y is at north pole.  Phase-shift all, incl. bndry cols. */
			if (G->header->registration == GMT_PIXEL_REG) {
				j1p = jn;	/* constraint for jno1  */
				j2p = jni1;	/* constraint for jno2  */
			}
			else {
				j1p = jni1;		/* constraint for jno1  */
				j2p = jni1 + mx;	/* constraint for jno2  */
			}
			for (i = iwo2; set[YHI] && i <= ieo2; i++) {
				i180 = G->header->pad[XLO] + ((i + nxp2)%G->header->nxp);
				G->data[jno1 + i] = G->data[j1p + i180];
				G->data[jno2 + i] = G->data[j2p + i180];
			}
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_POLE;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}
		}
		else {
			/* Y needs natural conditions.  x bndry cols periodic.
				First do Laplacian.  Start/end loop 1 col outside,
				then use periodicity to set 2nd col outside.  */

			for (i = iwo1; set[YHI] && i <= ieo1; i++) {
				G->data[jno1 + i] = (float)(4.0 * G->data[jn + i]) - (G->data[jn + i - 1] + G->data[jn + i + 1] + G->data[jni1 + i]);
			}
			if (set[XLO] && set[YHI]) G->data[jno1 + iwo2] = G->data[jno1 + iwo2 + G->header->nxp];
			if (set[XHI] && set[YHI]) G->data[jno1 + ieo2] = G->data[jno1 + ieo2 - G->header->nxp];


			/* Now set d[Laplacian]/dn = 0, start/end loop 1 col out,
				use periodicity to set 2nd out col after loop.  */

			for (i = iwo1; set[YHI] && i <= ieo1; i++) {
				G->data[jno2 + i] = G->data[jni1 + i] + (float)(5.0 * (G->data[jno1 + i] - G->data[jn + i]))
					+ (G->data[jn + i - 1] - G->data[jno1 + i - 1]) + (G->data[jn + i + 1] - G->data[jno1 + i + 1]);
			}
			if (set[XLO] && set[YHI]) G->data[jno2 + iwo2] = G->data[jno2 + iwo2 + G->header->nxp];
			if (set[XHI] && set[YHI]) G->data[jno2 + ieo2] = G->data[jno2 + ieo2 - G->header->nxp];

			/* End of X is periodic, north (top) is Natural.  */
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_NATURAL;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}
		}

		/* Done with north (top) BC in X is periodic case.  Do south (bottom)  */

		if (G->header->gs) {	/* Y is at south pole.  Phase-shift all, incl. bndry cols. */
			if (G->header->registration == GMT_PIXEL_REG) {
				j1p = js;	/* constraint for jso1  */
				j2p = jsi1;	/* constraint for jso2  */
			}
			else {
				j1p = jsi1;		/* constraint for jso1  */
				j2p = jsi1 - mx;	/* constraint for jso2  */
			}
			for (i = iwo2; set[YLO] && i <= ieo2; i++) {
				i180 = G->header->pad[XLO] + ((i + nxp2)%G->header->nxp);
				G->data[jso1 + i] = G->data[j1p + i180];
				G->data[jso2 + i] = G->data[j2p + i180];
			}
			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_POLE;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
		}
		else {
			/* Y needs natural conditions.  x bndry cols periodic.
				First do Laplacian.  Start/end loop 1 col outside,
				then use periodicity to set 2nd col outside.  */

			for (i = iwo1; set[YLO] && i <= ieo1; i++) {
				G->data[jso1 + i] = (float)(4.0 * G->data[js + i]) - (G->data[js + i - 1] + G->data[js + i + 1] + G->data[jsi1 + i]);
			}
			if (set[XLO] && set[YLO]) G->data[jso1 + iwo2] = G->data[jso1 + iwo2 + G->header->nxp];
			if (set[XHI] && set[YHI]) G->data[jso1 + ieo2] = G->data[jso1 + ieo2 - G->header->nxp];


			/* Now set d[Laplacian]/dn = 0, start/end loop 1 col out,
				use periodicity to set 2nd out col after loop.  */

			for (i = iwo1; set[YLO] && i <= ieo1; i++) {
				G->data[jso2 + i] = G->data[jsi1 + i] + (float)(5.0 * (G->data[jso1 + i] - G->data[js + i]))
					+ (G->data[js + i - 1] - G->data[jso1 + i - 1]) + (G->data[js + i + 1] - G->data[jso1 + i + 1]);
			}
			if (set[XLO] && set[YLO]) G->data[jso2 + iwo2] = G->data[jso2 + iwo2 + G->header->nxp];
			if (set[XHI] && set[YHI]) G->data[jso2 + ieo2] = G->data[jso2 + ieo2 - G->header->nxp];

			/* End of X is periodic, south (bottom) is Natural.  */
			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_NATURAL;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
		}

		/* Done with X is periodic cases.  */

		return (GMT_NOERROR);
	}
}

GMT_LONG GMT_image_BC_set (struct GMT_CTRL *C, struct GMT_IMAGE *G)
{
	/* Set two rows of padding (pad[] can be larger) around data according
	   to desired boundary condition info in edgeinfo.
	   Returns -1 on problem, 0 on success.
	   If either x or y is periodic, the padding is entirely set.
	   However, if neither is true (this rules out geographical also)
	   then all but three corner-most points in each corner are set.

	   As written, not ready to use with "surface" for GMT v4, because
	   assumes left/right is +/- 1 and down/up is +/- mx.  In "surface"
	   the amount to move depends on the current mesh size, a parameter
	   not used here.

	   This is the revised, two-rows version (WHFS 6 May 1998).
	   Based on GMT_boundcont_set but extended to multiple bands and using char * array
	*/

	GMT_LONG bok;		/* Counter used to test that things are OK  */
	GMT_LONG mx;		/* Width of padded array; width as malloc'ed  */
	GMT_LONG mxnyp;		/* distance to periodic constraint in j direction  */
	GMT_LONG i, jmx;	/* Current i, j * mx  */
	GMT_LONG nxp2;		/* 1/2 the xg period (180 degrees) in cells  */
	GMT_LONG i180;		/* index to 180 degree phase shift  */
	GMT_LONG iw, iwo1, iwo2, iwi1, ie, ieo1, ieo2, iei1;  /* see below  */
	GMT_LONG jn, jno1, jno2, jni1, js, jso1, jso2, jsi1;  /* see below  */
	GMT_LONG jno1k, jno2k, jso1k, jso2k, iwo1k, iwo2k, ieo1k, ieo2k;
	GMT_LONG j1p, j2p;	/* j_o1 and j_o2 pole constraint rows  */
	GMT_LONG b, nb = G->header->n_bands;
	GMT_LONG n_skip, set[4] = {TRUE, TRUE, TRUE, TRUE};
	char *kind[5] = {"not set", "natural", "periodic", "geographic", "extended data"};
	char *edge[4] = {"left  ", "right ", "bottom", "top   "};

	for (i = n_skip = 0; i < 4; i++) {
		if (G->header->BC[i] == GMT_BC_IS_DATA) {set[i] = FALSE; n_skip++;}	/* No need to set since there is data in the pad area */
	}
	if (n_skip == 4) return (GMT_NOERROR);	/* No need to set anything since there is data in the pad area on all sides */

	/* Check minimum size:  */
	if (G->header->nx < 1 || G->header->ny < 1) {
		GMT_report (C, GMT_MSG_FATAL, "Error: GMT_boundcond_image_set requires nx,ny at least 1.\n");
		return (-1);
	}

	/* Check if pad is requested */
	if (G->header->pad[0] < 2 ||  G->header->pad[1] < 2 ||  G->header->pad[2] < 2 ||  G->header->pad[3] < 2) {
		GMT_report (C, GMT_MSG_DEBUG, "Pad not large enough for BC assignments; no BCs applied\n");
		return(GMT_NOERROR);
	}

	/* Initialize stuff:  */

	mx = G->header->mx;
	nxp2 = G->header->nxp / 2;	/* Used for 180 phase shift at poles  */

	iw = G->header->pad[XLO];	/* i for west-most data column */
	iwo1 = iw - 1;		/* 1st column outside west  */
	iwo2 = iwo1 - 1;	/* 2nd column outside west  */
	iwi1 = iw + 1;		/* 1st column  inside west  */

	ie = G->header->pad[XLO] + G->header->nx - 1;	/* i for east-most data column */
	ieo1 = ie + 1;		/* 1st column outside east  */
	ieo2 = ieo1 + 1;	/* 2nd column outside east  */
	iei1 = ie - 1;		/* 1st column  inside east  */

	jn = mx * G->header->pad[YHI];	/* j*mx for north-most data row  */
	jno1 = jn - mx;		/* 1st row outside north  */
	jno2 = jno1 - mx;	/* 2nd row outside north  */
	jni1 = jn + mx;		/* 1st row  inside north  */

	js = mx * (G->header->pad[YHI] + G->header->ny - 1);	/* j*mx for south-most data row  */
	jso1 = js + mx;		/* 1st row outside south  */
	jso2 = jso1 + mx;	/* 2nd row outside south  */
	jsi1 = js - mx;		/* 1st row  inside south  */

	mxnyp = mx * G->header->nyp;

	jno1k = jno1 + mxnyp;	/* data rows periodic to boundary rows  */
	jno2k = jno2 + mxnyp;
	jso1k = jso1 - mxnyp;
	jso2k = jso2 - mxnyp;

	iwo1k = iwo1 + G->header->nxp;	/* data cols periodic to bndry cols  */
	iwo2k = iwo2 + G->header->nxp;
	ieo1k = ieo1 - G->header->nxp;
	ieo2k = ieo2 - G->header->nxp;

	/* Duplicate rows and columns if nx or ny equals 1 */

	if (G->header->nx == 1) for (i = jn+iw; i <= js+iw; i += mx) G->data[i-1] = G->data[i+1] = G->data[i];
	if (G->header->ny == 1) for (i = jn+iw; i <= jn+ie; i++) G->data[i-mx] = G->data[i+mx] = G->data[i];

	/* Check poles for grid case.  It would be nice to have done this
		in GMT_boundcond_param_prep() but at that point the data
		array isn't passed into that routine, and may not have been
		read yet.  Also, as coded here, this bombs with error if
		the pole data is wrong.  But there could be an option to
		to change the condition to Natural in that case, with warning.  */

	if (G->header->registration == GMT_GRIDLINE_REG) {	/* A pole can only be a grid node with gridline registration */
		if (G->header->gn) {	/* North pole case */
			bok = 0;
			for (i = iw+1; i <= ie; i++) for (b = 0; b < nb; b++) if (G->data[nb*(jn + i)+b] != G->data[nb*(jn + iw)+b]) bok++;
			if (bok > 0) GMT_report (C, GMT_MSG_FATAL, "Warning: Inconsistent image values at North pole.\n");
		}
		if (G->header->gs) {	/* South pole case */
			bok = 0;
			for (i = iw+1; i <= ie; i++) for (b = 0; b < nb; b++) if (G->data[nb*(js + i)+b] != G->data[nb*(js + iw)+b]) bok++;
			if (bok > 0) GMT_report (C, GMT_MSG_FATAL, "Warning: Inconsistent grid values at South pole.\n");
		}
	}

	/* Start with the case that x is not periodic, because in that case we also know that y cannot be polar.  */

	if (G->header->nxp <= 0) {	/* x is not periodic  */

		if (G->header->nyp > 0) {	/* y is periodic  */

			for (i = iw; i <= ie; i++) {
				for (b = 0; b < nb; b++) {
					if (set[YHI]) {
						G->data[nb*(jno1 + i)+b] = G->data[nb*(jno1k + i)+b];
						G->data[nb*(jno2 + i)+b] = G->data[nb*(jno2k + i)+b];
					}
					if (set[YLO]) {
						G->data[nb*(jso1 + i)+b] = G->data[nb*(jso1k + i)+b];
						G->data[nb*(jso2 + i)+b] = G->data[nb*(jso2k + i)+b];
					}
				}
			}

			/* periodic Y rows copied.  Now do X naturals.
				This is easy since y's are done; no corner problems.
				Begin with Laplacian = 0, and include 1st outside rows
				in loop, since y's already loaded to 2nd outside.  */

			for (jmx = jno1; jmx <= jso1; jmx += mx) {
				for (b = 0; b < nb; b++) {
					if (set[XLO]) G->data[nb*(jmx + iwo1)+b] = (unsigned char)irint (4.0 * G->data[nb*(jmx + iw)+b]) - (G->data[nb*(jmx + iw + mx)+b] + G->data[nb*(jmx + iw - mx)+b] + G->data[nb*(jmx + iwi1)+b]);
					if (set[XHI]) G->data[nb*(jmx + ieo1)+b] = (unsigned char)irint (4.0 * G->data[nb*(jmx + ie)+b]) - (G->data[nb*(jmx + ie + mx)+b] + G->data[nb*(jmx + ie - mx)+b] + G->data[nb*(jmx + iei1)+b]);
				}
			}

			/* Copy that result to 2nd outside row using periodicity.  */
			for (b = 0; b < nb; b++) {
				if (set[XLO]) {
					G->data[nb*(jno2 + iwo1)+b] = G->data[nb*(jno2k + iwo1)+b];
					G->data[nb*(jso2 + iwo1)+b] = G->data[nb*(jso2k + iwo1)+b];
				}
				if (set[XHI]) {
					G->data[nb*(jno2 + ieo1)+b] = G->data[nb*(jno2k + ieo1)+b];
					G->data[nb*(jso2 + ieo1)+b] = G->data[nb*(jso2k + ieo1)+b];
				}
			}

			/* Now set d[laplacian]/dx = 0 on 2nd outside column.  Include 1st outside rows in loop.  */
			for (jmx = jno1; jmx <= jso1; jmx += mx) {
				for (b = 0; b < nb; b++) {
					if (set[XLO]) G->data[nb*(jmx + iwo2)+b] = (unsigned char)irint ((G->data[nb*(jmx + iw - mx)+b] + G->data[nb*(jmx + iw + mx)+b] + G->data[nb*(jmx + iwi1)+b])
						- (G->data[nb*(jmx + iwo1 - mx)+b] + G->data[nb*(jmx + iwo1 + mx)+b]) + (5.0 * (G->data[nb*(jmx + iwo1)+b] - G->data[nb*(jmx + iw)+b])));
					if (set[XHI]) G->data[nb*(jmx + ieo2)+b] = (unsigned char)irint ((G->data[nb*(jmx + ie - mx)+b] + G->data[nb*(jmx + ie + mx)+b] + G->data[nb*(jmx + iei1)+b])
						- (G->data[nb*(jmx + ieo1 - mx)+b] + G->data[nb*(jmx + ieo1 + mx)+b]) + (5.0 * (G->data[nb*(jmx + ieo1)+b] - G->data[nb*(jmx + ie)+b])));
				}
			}

			/* Now copy that result also, for complete periodicity's sake  */
			for (b = 0; b < nb; b++) {
				if (set[XLO]) {
					G->data[nb*(jno2 + iwo2)+b] = G->data[nb*(jno2k + iwo2)+b];
					G->data[nb*(jso2 + iwo2)+b] = G->data[nb*(jso2k + iwo2)+b];
					G->header->BC[XLO] = GMT_BC_IS_NATURAL;
				}
				if (set[XHI]) {
					G->data[nb*(jno2 + ieo2)+b] = G->data[nb*(jno2k + ieo2)+b];
					G->data[nb*(jso2 + ieo2)+b] = G->data[nb*(jso2k + ieo2)+b];
					G->header->BC[XHI] = GMT_BC_IS_NATURAL;
				}
			}

			/* DONE with X not periodic, Y periodic case.  Fully loaded.  */
			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}

			return (GMT_NOERROR);
		}
		else {	/* Here begins the X not periodic, Y not periodic case  */

			/* First, set corner points.  Need not merely Laplacian(f) = 0
				but explicitly that d2f/dx2 = 0 and d2f/dy2 = 0.
				Also set d2f/dxdy = 0.  Then can set remaining points.  */

			for (b = 0; b < nb; b++) {
			/* d2/dx2 */	if (set[XLO]) G->data[nb*(jn + iwo1)+b]   = (unsigned char)irint (2.0 * G->data[nb*(jn + iw)+b] - G->data[nb*(jn + iwi1)+b]);
			/* d2/dy2 */	if (set[YHI]) G->data[nb*(jno1 + iw)+b]   = (unsigned char)irint (2.0 * G->data[nb*(jn + iw)+b] - G->data[nb*(jni1 + iw)+b]);
			/* d2/dxdy */	if (set[XLO] && set[YHI]) G->data[nb*(jno1 + iwo1)+b] = (unsigned char)irint (-(G->data[nb*(jno1 + iwi1)+b] - G->data[nb*(jni1 + iwi1)+b] + G->data[nb*(jni1 + iwo1)+b]));

			/* d2/dx2 */	if (set[XHI]) G->data[nb*(jn + ieo1)+b]   = (unsigned char)irint (2.0 * G->data[nb*(jn + ie)+b] - G->data[nb*(jn + iei1)+b]);
			/* d2/dy2 */	if (set[YHI]) G->data[nb*(jno1 + ie)+b]   = (unsigned char)irint (2.0 * G->data[nb*(jn + ie)+b] - G->data[nb*(jni1 + ie)+b]);
			/* d2/dxdy */	if (set[XHI] && set[YHI]) G->data[nb*(jno1 + ieo1)+b] = (unsigned char)irint (-(G->data[nb*(jno1 + iei1)+b] - G->data[nb*(jni1 + iei1)+b] + G->data[nb*(jni1 + ieo1)+b]));

			/* d2/dx2 */	if (set[XLO]) G->data[nb*(js + iwo1)+b]   = (unsigned char)irint (2.0 * G->data[nb*(js + iw)+b] - G->data[nb*(js + iwi1)+b]);
			/* d2/dy2 */	if (set[YLO]) G->data[nb*(jso1 + iw)+b]   = (unsigned char)irint (2.0 * G->data[nb*(js + iw)+b] - G->data[nb*(jsi1 + iw)+b]);
			/* d2/dxdy */	if (set[XLO] && set[YLO]) G->data[nb*(jso1 + iwo1)+b] = (unsigned char)irint (-(G->data[nb*(jso1 + iwi1)+b] - G->data[nb*(jsi1 + iwi1)+b] + G->data[nb*(jsi1 + iwo1)+b]));

			/* d2/dx2 */	if (set[XHI]) G->data[nb*(js + ieo1)+b]   = (unsigned char)irint (2.0 * G->data[nb*(js + ie)+b] - G->data[nb*(js + iei1)+b]);
			/* d2/dy2 */	if (set[YLO]) G->data[nb*(jso1 + ie)+b]   = (unsigned char)irint (2.0 * G->data[nb*(js + ie)+b] - G->data[nb*(jsi1 + ie)+b]);
			/* d2/dxdy */	if (set[XHI] && set[YLO]) G->data[nb*(jso1 + ieo1)+b] = (unsigned char)irint (-(G->data[nb*(jso1 + iei1)+b] - G->data[nb*(jsi1 + iei1)+b] + G->data[nb*(jsi1 + ieo1)+b]));
			}

			/* Now set Laplacian = 0 on interior edge points, skipping corners:  */
			for (i = iwi1; i <= iei1; i++) {
				for (b = 0; b < nb; b++) {
					if (set[YHI]) G->data[nb*(jno1 + i)+b] = (unsigned char)irint (4.0 * G->data[nb*(jn + i)+b]) - (G->data[nb*(jn + i - 1)+b] + G->data[nb*(jn + i + 1)+b] + G->data[nb*(jni1 + i)+b]);
					if (set[YLO]) G->data[nb*(jso1 + i)+b] = (unsigned char)irint (4.0 * G->data[nb*(js + i)+b]) - (G->data[nb*(js + i - 1)+b] + G->data[nb*(js + i + 1)+b] + G->data[nb*(jsi1 + i)+b]);
				}
			}
			for (jmx = jni1; jmx <= jsi1; jmx += mx) {
				for (b = 0; b < nb; b++) {
					if (set[XLO]) G->data[nb*(iwo1 + jmx)+b] = (unsigned char)irint (4.0 * G->data[nb*(iw + jmx)+b]) - (G->data[nb*(iw + jmx + mx)+b] + G->data[nb*(iw + jmx - mx)+b] + G->data[nb*(iwi1 + jmx)+b]);
					if (set[XHI]) G->data[nb*(ieo1 + jmx)+b] = (unsigned char)irint (4.0 * G->data[nb*(ie + jmx)+b]) - (G->data[nb*(ie + jmx + mx)+b] + G->data[nb*(ie + jmx - mx)+b] + G->data[nb*(iei1 + jmx)+b]);
				}
			}

			/* Now set d[Laplacian]/dn = 0 on all edge pts, including
				corners, since the points needed in this are now set.  */
			for (i = iw; i <= ie; i++) {
				for (b = 0; b < nb; b++) {
					if (set[YHI]) G->data[nb*(jno2 + i)+b] = (unsigned char)irint (G->data[nb*(jni1 + i)+b] + (5.0 * (G->data[nb*(jno1 + i)+b] - G->data[nb*(jn + i)+b]))
						+ (G->data[nb*(jn + i - 1)+b] - G->data[nb*(jno1 + i - 1)+b]) + (G->data[nb*(jn + i + 1)+b] - G->data[nb*(jno1 + i + 1)+b]));
					if (set[YLO]) G->data[nb*(jso2 + i)+b] = (unsigned char)irint (G->data[nb*(jsi1 + i)+b] + (5.0 * (G->data[nb*(jso1 + i)+b] - G->data[nb*(js + i)+b]))
						+ (G->data[nb*(js + i - 1)+b] - G->data[nb*(jso1 + i - 1)+b]) + (G->data[nb*(js + i + 1)+b] - G->data[nb*(jso1 + i + 1)+b]));
				}
			}
			for (jmx = jn; jmx <= js; jmx += mx) {
				for (b = 0; b < nb; b++) {
					if (set[XLO]) G->data[nb*(iwo2 + jmx)+b] = (unsigned char)irint (G->data[nb*(iwi1 + jmx)+b] + (5.0 * (G->data[nb*(iwo1 + jmx)+b] - G->data[nb*(iw + jmx)+b]))
						+ (G->data[nb*(iw + jmx - mx)+b] - G->data[nb*(iwo1 + jmx - mx)+b]) + (G->data[nb*(iw + jmx + mx)+b] - G->data[nb*(iwo1 + jmx + mx)+b]));
					if (set[XHI]) G->data[nb*(ieo2 + jmx)+b] = (unsigned char)irint (G->data[nb*(iei1 + jmx)+b] + (5.0 * (G->data[nb*(ieo1 + jmx)+b] - G->data[nb*(ie + jmx)+b]))
						+ (G->data[nb*(ie + jmx - mx)+b] - G->data[nb*(ieo1 + jmx - mx)+b]) + (G->data[nb*(ie + jmx + mx)+b] - G->data[nb*(ieo1 + jmx + mx)+b]));
				}
			}
			/* DONE with X not periodic, Y not periodic case.  Loaded all but three cornermost points at each corner.  */

			for (i = 0; i < 4; i++) if (set[i]) {
				G->header->BC[i] = GMT_BC_IS_NATURAL;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[i], kind[G->header->BC[i]]);
			}
			return (GMT_NOERROR);
		}
		/* DONE with all X not periodic cases  */
	}
	else {	/* X is periodic.  Load x cols first, then do Y cases.  */
		if (set[XLO]) G->header->BC[XLO] = GMT_BC_IS_PERIODIC;
		if (set[XHI]) G->header->BC[XHI] = GMT_BC_IS_PERIODIC;

		for (jmx = jn; jmx <= js; jmx += mx) {
			for (b = 0; b < nb; b++) {
				if (set[XLO]) {
					G->data[nb*(iwo1 + jmx)+b] = G->data[nb*(iwo1k + jmx)+b];
					G->data[nb*(iwo2 + jmx)+b] = G->data[nb*(iwo2k + jmx)+b];
				}
				if (set[XHI]) {
					G->data[nb*(ieo1 + jmx)+b] = G->data[nb*(ieo1k + jmx)+b];
					G->data[nb*(ieo2 + jmx)+b] = G->data[nb*(ieo2k + jmx)+b];
				}
			}
		}

		if (G->header->nyp > 0) {	/* Y is periodic.  copy all, including boundary cols:  */
			for (i = iwo2; i <= ieo2; i++) {
				for (b = 0; b < nb; b++) {
					if (set[YHI]) {
						G->data[nb*(jno1 + i)+b] = G->data[nb*(jno1k + i)+b];
						G->data[nb*(jno2 + i)+b] = G->data[nb*(jno2k + i)+b];
					}
					if (set[YLO]) {
						G->data[nb*(jso1 + i)+b] = G->data[nb*(jso1k + i)+b];
						G->data[nb*(jso2 + i)+b] = G->data[nb*(jso2k + i)+b];
					}
				}
			}
			/* DONE with X and Y both periodic.  Fully loaded.  */

			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_PERIODIC;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}
			return (GMT_NOERROR);
		}

		/* Do north (top) boundary:  */

		if (G->header->gn) {	/* Y is at north pole.  Phase-shift all, incl. bndry cols. */
			if (G->header->registration == GMT_PIXEL_REG) {
				j1p = jn;	/* constraint for jno1  */
				j2p = jni1;	/* constraint for jno2  */
			}
			else {
				j1p = jni1;		/* constraint for jno1  */
				j2p = jni1 + mx;	/* constraint for jno2  */
			}
			for (i = iwo2; set[YHI] && i <= ieo2; i++) {
				i180 = G->header->pad[XLO] + ((i + nxp2)%G->header->nxp);
				for (b = 0; b < nb; b++) {
					G->data[nb*(jno1 + i)+b] = G->data[nb*(j1p + i180)+b];
					G->data[nb*(jno2 + i)+b] = G->data[nb*(j2p + i180)+b];
				}
			}
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_POLE;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}
		}
		else {
			/* Y needs natural conditions.  x bndry cols periodic.
				First do Laplacian.  Start/end loop 1 col outside,
				then use periodicity to set 2nd col outside.  */

			for (i = iwo1; set[YHI] && i <= ieo1; i++) {
				for (b = 0; b < nb; b++) {
					G->data[nb*(jno1 + i)+b] = (unsigned char)irint ((4.0 * G->data[nb*(jn + i)+b]) - (G->data[nb*(jn + i - 1)+b] + G->data[nb*(jn + i + 1)+b] + G->data[nb*(jni1 + i)+b]));
				}
			}
			for (b = 0; b < nb; b++) {
				if (set[XLO] && set[YHI]) G->data[nb*(jno1 + iwo2)+b] = G->data[nb*(jno1 + iwo2 + G->header->nxp)+b];
				if (set[XHI] && set[YHI]) G->data[nb*(jno1 + ieo2)+b] = G->data[nb*(jno1 + ieo2 - G->header->nxp)+b];
			}

			/* Now set d[Laplacian]/dn = 0, start/end loop 1 col out,
				use periodicity to set 2nd out col after loop.  */

			for (i = iwo1; set[YHI] && i <= ieo1; i++) {
				for (b = 0; b < nb; b++) {
					G->data[nb*(jno2 + i)+b] = (unsigned char)irint (G->data[nb*(jni1 + i)+b] + (5.0 * (G->data[nb*(jno1 + i)+b] - G->data[nb*(jn + i)+b]))
						+ (G->data[nb*(jn + i - 1)+b] - G->data[nb*(jno1 + i - 1)+b]) + (G->data[nb*(jn + i + 1)+b] - G->data[nb*(jno1 + i + 1)+b]));
				}
			}
			for (b = 0; b < nb; b++) {
				if (set[XLO] && set[YHI]) G->data[nb*(jno2 + iwo2)+b] = G->data[nb*(jno2 + iwo2 + G->header->nxp)+b];
				if (set[XHI] && set[YHI]) G->data[nb*(jno2 + ieo2)+b] = G->data[nb*(jno2 + ieo2 - G->header->nxp)+b];
			}

			/* End of X is periodic, north (top) is Natural.  */
			if (set[YHI]) {
				G->header->BC[YHI] = GMT_BC_IS_NATURAL;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YHI], kind[G->header->BC[YHI]]);
			}
		}

		/* Done with north (top) BC in X is periodic case.  Do south (bottom)  */

		if (G->header->gs) {	/* Y is at south pole.  Phase-shift all, incl. bndry cols. */
			if (G->header->registration == GMT_PIXEL_REG) {
				j1p = js;	/* constraint for jso1  */
				j2p = jsi1;	/* constraint for jso2  */
			}
			else {
				j1p = jsi1;		/* constraint for jso1  */
				j2p = jsi1 - mx;	/* constraint for jso2  */
			}
			for (i = iwo2; set[YLO] && i <= ieo2; i++) {
				i180 = G->header->pad[XLO] + ((i + nxp2)%G->header->nxp);
				for (b = 0; b < nb; b++) {
					G->data[nb*(jso1 + i)+b] = G->data[nb*(j1p + i180)+b];
					G->data[nb*(jso2 + i)+b] = G->data[nb*(j2p + i180)+b];
				}
			}
			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_POLE;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
		}
		else {
			/* Y needs natural conditions.  x bndry cols periodic.
				First do Laplacian.  Start/end loop 1 col outside,
				then use periodicity to set 2nd col outside.  */

			for (i = iwo1; set[YLO] && i <= ieo1; i++) {
				for (b = 0; b < nb; b++) {
					G->data[nb*(jso1 + i)+b] = (unsigned char)irint ((4.0 * G->data[nb*(js + i)+b]) - (G->data[nb*(js + i - 1)+b] + G->data[nb*(js + i + 1)+b] + G->data[nb*(jsi1 + i)+b]));
				}
			}
			for (b = 0; b < nb; b++) {
				if (set[XLO] && set[YLO]) G->data[nb*(jso1 + iwo2)+b] = G->data[nb*(jso1 + iwo2 + G->header->nxp)+b];
				if (set[XHI] && set[YHI]) G->data[nb*(jso1 + ieo2)+b] = G->data[nb*(jso1 + ieo2 - G->header->nxp)+b];
			}


			/* Now set d[Laplacian]/dn = 0, start/end loop 1 col out,
				use periodicity to set 2nd out col after loop.  */

			for (i = iwo1; set[YLO] && i <= ieo1; i++) {
				for (b = 0; b < nb; b++) {
					G->data[nb*(jso2 + i)+b] = (unsigned char)irint (G->data[nb*(jsi1 + i)+b] + (5.0 * (G->data[nb*(jso1 + i)+b] - G->data[nb*(js + i)+b]))
						+ (G->data[nb*(js + i - 1)+b] - G->data[nb*(jso1 + i - 1)+b]) + (G->data[nb*(js + i + 1)+b] - G->data[nb*(jso1 + i + 1)+b]));
				}
			}
			for (b = 0; b < nb; b++) {
				if (set[XLO] && set[YLO]) G->data[nb*(jso2 + iwo2)+b] = G->data[nb*(jso2 + iwo2 + G->header->nxp)+b];
				if (set[XHI] && set[YHI]) G->data[nb*(jso2 + ieo2)+b] = G->data[nb*(jso2 + ieo2 - G->header->nxp)+b];
			}

			/* End of X is periodic, south (bottom) is Natural.  */
			if (set[YLO]) {
				G->header->BC[YLO] = GMT_BC_IS_NATURAL;
				GMT_report (C, GMT_MSG_VERBOSE, "Set boundary condition for %s edge: %s\n", edge[YLO], kind[G->header->BC[YLO]]);
			}
		}

		/* Done with X is periodic cases.  */

		return (GMT_NOERROR);
	}
}

GMT_LONG GMT_y_out_of_bounds (struct GMT_CTRL *C, GMT_LONG *j, struct GRD_HEADER *h, GMT_LONG *wrap_180) {
	/* Adjusts the j (y-index) value if we are dealing with some sort of periodic boundary
	* condition.  If a north or south pole condition we must "go over the pole" and access
	* the longitude 180 degrees away - this is achieved by passing the wrap_180 flag; the
	* shifting of longitude is then deferred to GMT_x_out_of_bounds.
	* If no periodicities are present then nothing happens here.  If we end up being
	* out of bounds we return TRUE (and main program can take action like continue);
	* otherwise we return FALSE.
	*/

	if ((*j) < 0) {	/* Depending on BC's we wrap around or we are above the top of the domain */
		if (h->gn) {	/* N Polar condition - adjust j and set wrap flag */
			(*j) = GMT_abs (*j) - h->registration;
			(*wrap_180) = TRUE;	/* Go "over the pole" */
		}
		else if (h->nyp) {	/* Periodic in y */
			(*j) += h->nyp;
			(*wrap_180) = FALSE;
		}
		else
			return (TRUE);	/* We are outside the range */
	}
	else if ((*j) >= h->ny) {	/* Depending on BC's we wrap around or we are below the bottom of the domain */
		if (h->gs) {	/* S Polar condition - adjust j and set wrap flag */
			(*j) += h->registration - 2;
			(*wrap_180) = TRUE;	/* Go "over the pole" */
		}
		else if (h->nyp) {	/* Periodic in y */
			(*j) -= h->nyp;
			(*wrap_180) = FALSE;
		}
		else
			return (TRUE);	/* We are outside the range */
	}
	else
		(*wrap_180) = FALSE;

	return (FALSE);	/* OK, we are inside grid now for sure */
}

GMT_LONG GMT_x_out_of_bounds (struct GMT_CTRL *C, GMT_LONG *i, struct GRD_HEADER *h, GMT_LONG wrap_180) {
	/* Adjusts the i (x-index) value if we are dealing with some sort of periodic boundary
	* condition.  If a north or south pole condition we must "go over the pole" and access
	* the longitude 180 degrees away - this is achieved by examining the wrap_180 flag and take action.
	* If no periodicities are present and we end up being out of bounds we return TRUE (and
	* main program can take action like continue); otherwise we return FALSE.
	*/

	/* Depending on BC's we wrap around or leave as is. */

	if ((*i) < 0) {	/* Potentially outside to the left of the domain */
		if (h->nxp)	/* Periodic in x - always inside grid */
			(*i) += h->nxp;
		else	/* Sorry, you're outside */
			return (TRUE);
	}
	else if ((*i) >= h->nx) {	/* Potentially outside to the right of the domain */
		if (h->nxp)	/* Periodic in x -always inside grid */
			(*i) -= h->nxp;
		else	/* Sorry, you're outside */
			return (TRUE);
	}

	if (wrap_180) (*i) = ((*i) + (h->nxp / 2)) % h->nxp;	/* Must move 180 degrees */

	return (FALSE);	/* OK, we are inside grid now for sure */
}

void GMT_set_xy_domain (struct GMT_CTRL *C, double wesn_extended[], struct GRD_HEADER *h)
{
	double off;
	/* Sets the domain boundaries to be used to determine if x,y coordinates
	 * are outside the domain of a grid.  If gridline-registered then the
	 * domain is extended by 0.5 the grid interval.  Note that points with
	 * x == x_max and y == y_max are considered inside.
	 */

	off = 0.5 * (1 - h->registration);
	if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && GMT_360_RANGE (h->wesn[XHI], h->wesn[XLO]))	/* Global longitude range */
		wesn_extended[XLO] = h->wesn[XLO], wesn_extended[XHI] = h->wesn[XHI];
	else
		wesn_extended[XLO] = h->wesn[XLO] - off * h->inc[GMT_X], wesn_extended[XHI] = h->wesn[XHI] + off * h->inc[GMT_X];
	/* Latitudes can be extended provided we are not at the poles */
	wesn_extended[YLO] = h->wesn[YLO] - off * h->inc[GMT_Y], wesn_extended[YHI] = h->wesn[YHI] + off * h->inc[GMT_Y];
	if (C->current.io.col_type[GMT_IN][GMT_Y] == GMT_IS_LAT) {
		if (wesn_extended[YLO] < -90.0) wesn_extended[YLO] = -90.0;
		if (wesn_extended[YHI] > +90.0) wesn_extended[YHI] = +90.0;
	}
}

GMT_LONG GMT_x_is_outside (struct GMT_CTRL *C, double *x, double left, double right)
{
	/* Determines if this x is inside the effective x-domain.  This is normally
	 * west to east, but when gridding is concerned it can be extended by +-0.5 * dx
	 * for gridline-registered grids.  Also, if x is longitude we must check for
	 * wrap-arounds by 360 degrees, and x may be modified accordingly.
	 */
	if (GMT_is_dnan (*x)) return (TRUE);
	if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON) {	/* Periodic longitude test */
		while ((*x) > left) (*x) -= 360.0;	/* Make sure we start west or west */
		while ((*x) < left) (*x) += 360.0;	/* See if we are outside east */
		return (((*x) > right) ? TRUE : FALSE);
	}
	else	/* Cartesian test */
		return (((*x) < left || (*x) > right) ? TRUE : FALSE);
}

GMT_LONG GMT_getscale (struct GMT_CTRL *C, char *text, struct GMT_MAP_SCALE *ms)
{
	/* Pass text as &argv[i][2] */

	GMT_LONG j = 0, i, n_slash, error = 0, k = 0, options;
	char txt_cpy[GMT_BUFSIZ], txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_sx[GMT_TEXT_LEN256], txt_sy[GMT_TEXT_LEN256], txt_len[GMT_TEXT_LEN256];

	if (!text) { GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getscale\n"); GMT_exit (EXIT_FAILURE); }

	GMT_memset (ms, 1, struct GMT_MAP_SCALE);
	ms->measure = 'k';	/* Default distance unit is km */
	ms->justify = 't';
	GMT_rgb_copy (ms->fill.rgb, C->session.no_rgb);

	/* First deal with possible prefixes f and x (i.e., f, x, xf, fx) */
	if (text[j] == 'f') ms->fancy = TRUE, j++;
	if (text[j] == 'x') ms->gave_xy = TRUE, j++;
	if (text[j] == 'f') ms->fancy = TRUE, j++;	/* in case we got xf instead of fx */

	/* Determine if we have the optional longitude component specified by counting slashes.
	 * We stop counting if we reach a + sign since the fill or label might have a slash in them */

	for (n_slash = 0, i = j; text[i] && text[i] != '+'; i++) if (text[i] == '/') n_slash++;
	options = (text[i] == '+') ? i : -1;	/* -1, or starting point of first option */
	if (options > 0) {	/* Have optional args, make a copy and truncate text */
		strcpy (txt_cpy, &text[options]);
		text[options] = '\0';
		for (i = 0; txt_cpy[i]; i++) {	/* Unless +fjlpu, change other + to ascii 1 to bypass strtok trouble later */
			if (txt_cpy[i] == '+' && !strchr ("fjlpu", (int)txt_cpy[i+1])) txt_cpy[i] = 1;
		}
	}

	if (n_slash == 4) {		/* -L[f][x]<x0>/<y0>/<lon>/<lat>/<length>[m|n|k][+l<label>][+j<just>][+p<pen>][+f<fill>][+u] */
		k = sscanf (&text[j], "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_sx, txt_sy, txt_len);
	}
	else if (n_slash == 3) {	/* -L[f][x]<x0>/<y0>/<lat>/<length>[m|n|k][+l<label>][+j<just>][+p<pen>][+f<fill>][+u] */
		k = sscanf (&text[j], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_sy, txt_len);
	}
	else	/* Wrong number of slashes */
		error++;
	i = strlen (txt_len) - 1;
	if (isalpha ((int)txt_len[i])) {	/* Letter at end of distance value */
		if (strchr (GMT_LEN_UNITS2, (int)txt_len[i])) {	/* Gave a valid distance unit */
			ms->measure = txt_len[i];
			txt_len[i] = '\0';
		}
		else {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Valid distance units are %s\n", GMT_LEN_UNITS2_DISPLAY);
			error++;
		}
	}
	ms->length = atof (txt_len);

	if (ms->gave_xy) {	/* Convert user's x/y position to inches */
		ms->x0 = GMT_to_inch (C, txt_a);
		ms->y0 = GMT_to_inch (C, txt_b);
	}
	else {	/* Read geographical coordinates */
		error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &ms->x0), txt_a);
		error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &ms->y0), txt_b);
		if (fabs (ms->y0) > 90.0) {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Position latitude is out of range\n");
			error++;
		}
		if (fabs (ms->x0) > 360.0) {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Position longitude is out of range\n");
			error++;
		}
	}
	error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_sy, GMT_IS_LAT, &ms->scale_lat), txt_sy);
	if (k == 5)	/* Must also decode longitude of scale */
		error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_sx, GMT_IS_LON, &ms->scale_lon), txt_sx);
	else		/* Default to central meridian */
		ms->scale_lon = C->current.proj.central_meridian;
	if (fabs (ms->scale_lat) > 90.0) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Scale latitude is out of range\n");
		error++;
	}
	if (fabs (ms->scale_lon) > 360.0) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Scale longitude is out of range\n");
		error++;
	}
	if (ms->length <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Length must be positive\n");
		error++;
	}
	if (fabs (ms->scale_lat) > 90.0) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Defining latitude is out of range\n");
		error++;
	}
	if (options > 0) {	/* Gave +?<args> which now must be processed */
		char p[GMT_BUFSIZ];
		GMT_LONG pos = 0, bad = 0;
		while ((GMT_strtok (C, txt_cpy, "+", &pos, p))) {
			switch (p[0]) {
				case 'f':	/* Fill specification */
					if (GMT_getfill (C, &p[1], &ms->fill)) bad++;
					ms->boxfill = TRUE;
					break;

				case 'j':	/* Label justification */
					ms->justify = p[1];
					if (!(ms->justify == 'l' || ms->justify == 'r' || ms->justify == 't' || ms->justify == 'b')) bad++;
					break;

				case 'p':	/* Pen specification */
					if (GMT_getpen (C, &p[1], &ms->pen)) bad++;
					ms->boxdraw = TRUE;
					break;

				case 'l':	/* Label specification */
					if (p[1]) strcpy (ms->label, &p[1]);
					ms->do_label = TRUE;
					for (i = 0; ms->label[i]; i++) if (ms->label[i] == 1) ms->label[i] = '+';	/* Change back ASCII 1 to + */
					break;

				case 'u':	/* Add units to annotations */
					ms->unit = TRUE;
					break;

				default:
					bad++;
					break;
			}
		}
		error += bad;
		text[options] = '+';	/* Restore original string */
	}

	if (error) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  Correct syntax\n");
		GMT_report (C, GMT_MSG_FATAL, "\t-L[f][x]<x0>/<y0>/[<lon>/]<lat>/<length>[%s][+l<label>][+j<just>][+p<pen>][+f<fill>][+u]\n", GMT_LEN_UNITS2_DISPLAY);
		GMT_report (C, GMT_MSG_FATAL, "\t  Append length distance unit from %s [k]\n", GMT_LEN_UNITS2_DISPLAY);
		GMT_report (C, GMT_MSG_FATAL, "\t  Justification can be l, r, b, or t [Default]\n");
	}
	ms->plot = TRUE;
	return (error);
}

GMT_LONG GMT_getrose (struct GMT_CTRL *C, char *text, struct GMT_MAP_ROSE *ms)
{
	GMT_LONG j = 0, i, error = 0, colon, plus, slash, k, pos, order[4] = {3,1,0,2};
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256], txt_d[GMT_TEXT_LEN256], tmpstring[GMT_TEXT_LEN256], p[GMT_TEXT_LEN256];

	/* SYNTAX is -T[f|m][x]<lon0>/<lat0>/<size>[/<info>][:label:][+<aint>/<fint>/<gint>[/<aint>/<fint>/<gint>]], where <info> is
	 * 1)  -Tf: <info> is <kind> = 1,2,3 which is the level of directions [1].
	 * 2)  -Tm: <info> is <dec>/<dlabel>, where <Dec> is magnetic declination and dlabel its label [no mag].
	 * If -Tm, optionally set annotation interval with +
	 */

	if (!text) { GMT_report (C, GMT_MSG_FATAL, "No argument given to GMT_getrose\n"); GMT_exit (EXIT_FAILURE); }

	ms->fancy = ms->gave_xy = FALSE;
	ms->size = 0.0;
	ms->a_int[0] = 10.0;	ms->f_int[0] = 5.0;	ms->g_int[0] = 1.0;
	ms->a_int[1] = 30.0;	ms->f_int[1] = 5.0;	ms->g_int[1] = 1.0;
	strcpy (ms->label[0], "S");
	strcpy (ms->label[1], "E");
	strcpy (ms->label[2], "N");
	strcpy (ms->label[3], "W");

	/* First deal with possible prefixes f and x (i.e., f|m, x, xf|m, f|mx) */
	if (text[j] == 'f') ms->fancy = TRUE, j++;
	if (text[j] == 'm') ms->fancy = 2, j++;
	if (text[j] == 'x') ms->gave_xy = TRUE, j++;
	if (text[j] == 'f') ms->fancy = TRUE, j++;	/* in case we got xf instead of fx */
	if (text[j] == 'm') ms->fancy = 2, j++;		/* in case we got xm instead of mx */

	/* Determine if we have the optional label components specified */

	for (i = j, slash = 0; text[i] && slash < 2; i++) if (text[i] == '/') slash++;	/* Move i until the 2nd slash is reached */
	for (k = strlen(text) - 1, colon = 0; text[k] && k > i && colon < 2; k--) if (text[k] == ':') colon++;	/* Move k to starting colon of :label: */
	if (colon == 2 && k > i)
		colon = k + 2;	/* Beginning of label */
	else
		colon = 0;	/* No labels given */

	for (plus = -1, i = slash; text[i] && plus < 0; i++) if (text[i] == '+') plus = i+1;	/* Find location of + */
	if (plus > 0) {		/* Get annotation interval(s) */
		k = sscanf (&text[plus], "%lf/%lf/%lf/%lf/%lf/%lf", &ms->a_int[1], &ms->f_int[1], &ms->g_int[1], &ms->a_int[0], &ms->f_int[0], &ms->g_int[0]);
		if (k < 1) {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option:  Give annotation interval(s)\n");
			error++;
		}
		if (k == 3) ms->a_int[0] = ms->a_int[1], ms->f_int[0] = ms->f_int[1], ms->g_int[0] = ms->g_int[1];
		text[plus-1] = '\0';	/* Break string so sscanf wont get confused later */
	}
	if (colon > 0) {	/* Get labels in string :w,e,s,n: */
		for (k = colon; text[k] && text[k] != ':'; k++);	/* Look for terminating colon */
		if (text[k] != ':') { /* Ran out, missing terminating colon */
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option: Labels must be given in format :w,e,s,n:\n");
			error++;
			return (error);
		}
		strncpy (tmpstring, &text[colon], (size_t)(k-colon));
		tmpstring[k-colon] = '\0';
		k = pos = 0;
		while (k < 4 && (GMT_strtok (C, tmpstring, ",", &pos, p))) {	/* Get the four labels */
			if (strcmp (p, "-")) strcpy (ms->label[order[k]], p);
			k++;
		}
		if (k == 0)	/* No labels wanted */
			ms->label[0][0] = ms->label[1][0] = ms->label[2][0] = ms->label[3][0] = '\0';
		else if (k != 4) {	/* Ran out of labels */
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option: Labels must be given in format :w,e,s,n:\n");
			error++;
		}
		text[colon-1] = '\0';	/* Break string so sscanf wont get confused later */
	}

	/* -L[f][x]<x0>/<y0>/<size>[/<kind>][:label:] OR -L[m][x]<x0>/<y0>/<size>[/<dec>/<declabel>][:label:][+gint[/mint]] */
	if (ms->fancy == 2) {	/* Magnetic rose */
		k = sscanf (&text[j], "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]", txt_a, txt_b, txt_c, txt_d, ms->dlabel);
		if (! (k == 3 || k == 5)) {	/* Wrong number of parameters */
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option:  Correct syntax\n");
			GMT_report (C, GMT_MSG_FATAL, "\t-T[f|m][x]<x0>/<y0>/<size>[/<info>][:wesnlabels:][+<gint>[/<mint>]]\n");
			error++;
		}
		if (k == 3) {	/* No magnetic north directions */
			ms->kind = 1;
			ms->declination = 0.0;
			ms->dlabel[0] = '\0';
		}
		else {
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_d, GMT_IS_LON, &ms->declination), txt_d);
			ms->kind = 2;
		}
	}
	else {
		k = sscanf (&text[j], "%[^/]/%[^/]/%[^/]/%" GMT_LL "d", txt_a, txt_b, txt_c, &ms->kind);
		if (k == 3) ms->kind = 1;
		if (k < 3 || k > 4) {	/* Wrong number of parameters */
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option:  Correct syntax\n");
			GMT_report (C, GMT_MSG_FATAL, "\t-T[f|m][x]<x0>/<y0>/<size>[/<info>][:wesnlabels:][+<gint>[/<mint>]]\n");
			error++;
		}
	}
	if (colon > 0) text[colon-1] = ':';	/* Put it back */
	if (plus > 0) text[plus-1] = '+';	/* Put it back */
	if (ms->gave_xy) {	/* Convert user's x/y to inches */
		ms->x0 = GMT_to_inch (C, txt_a);
		ms->y0 = GMT_to_inch (C, txt_b);
	}
	else {	/* Read geographical coordinates */
		error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &ms->x0), txt_a);
		error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &ms->y0), txt_b);
		if (fabs (ms->y0) > 90.0) {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option:  Position latitude is out of range\n");
			error++;
		}
		if (fabs (ms->x0) > 360.0) {
			GMT_report (C, GMT_MSG_FATAL, "syntax error -T option:  Position longitude is out of range\n");
			error++;
		}
	}
	ms->size = GMT_to_inch (C, txt_c);
	if (ms->size <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -T option:  Size must be positive\n");
		error++;
	}
	if (ms->kind < 1 || ms->kind > 3) {
		GMT_report (C, GMT_MSG_FATAL, "syntax error -L option:  <kind> must be 1, 2, or 3\n");
		error++;
	}

	ms->plot = TRUE;
	return (error);
}

GMT_LONG GMT_minmaxinc_verify (struct GMT_CTRL *C, double min, double max, double inc, double slop)
{
	double range;

	/* Check for how compatible inc is with the range max - min.
	   We will tolerate a fractional sloppiness <= slop.  The
	   return values are:
	   0 : Everything is ok
	   1 : range is not a whole multiple of inc (within assigned slop)
	   2 : the range (max - min) is < 0
	   3 : inc is <= 0
	*/

	if (inc <= 0.0) return (3);

	range = max - min;
	if (range < 0.0) return (2);

	range = (fmod (range / inc, 1.0));
	if (range > slop && range < (1.0 - slop)) return (1);
	return 0;
}

void GMT_str_tolower (char *value)
{
	/* Convert entire string to lower case */
	int i, c;
	for (i = 0; value[i]; i++) {
		c = (int)value[i];
		value[i] = (char) tolower (c);
	}
}

void GMT_str_toupper (char *value)
{
	/* Convert entire string to upper case */
	int i, c;
	for (i = 0; value[i]; i++) {
		c = (int)value[i];
		value[i] = (char) toupper (c);
	}
}

void GMT_str_setcase (struct GMT_CTRL *C, char *value, GMT_LONG mode)
{
	if (mode == 0) return;	/* Do nothing */
	if (mode == -1)
		GMT_str_tolower (value);
	else if (mode == +1)
		GMT_str_toupper (value);
	else
		GMT_report (C, GMT_MSG_FATAL, "Error: Bad mode in GMT_str_setcase (%ld)\n", mode);
}

char *GMT_chop_ext (struct GMT_CTRL *C, char *string)
{
	/* Chops off the extension (the .xxx ) in string and returns it, including the leading '.' */
	GMT_LONG i, n, pos_ext = 0;
	if (!string) return (NULL);	/* NULL pointer */
	if ((n = (GMT_LONG)strlen (string)) == 0) return (NULL);	/* Empty string */

	for (i = n - 1; i > 0; i--) {
		if (string[i] == '.') {	/* Beginning of file extension */
			pos_ext = i;
			break;
		}
	}
	if (pos_ext) {
		string[pos_ext] = '\0';		/* Remove the extension */
		return (strdup(&string[pos_ext]));
	}
	else
		return (NULL);
}

void GMT_chop (struct GMT_CTRL *C, char *string)
{
	/* Chops off any CR or LF at end of string and ensures it is null-terminated */
	GMT_LONG i, n;
	if (!string) return;	/* NULL pointer */
	if ((n = (GMT_LONG)strlen (string)) == 0) return;	/* Empty string */
	for (i = n - 1; i >= 0 && (string[i] == '\n' || string[i] == '\r'); i--) string[i] = '\0';	/* Overwrite CR or LF with terminate string */
}

GMT_LONG GMT_strlcmp (char *str1, char *str2)
{
	/* Compares str1 with str2 but only until str1 reaches the
	 * null-terminator character while case is ignored.
	 * When the strings match until that point, the routine returns the
	 * length of str1, otherwise it returns 0.
	 */
	GMT_LONG i = 0;
	while (str1[i] && tolower((unsigned char) str1[i]) == tolower((unsigned char) str2[i])) ++i;
	if (str1[i]) return 0;
	return i;
}

GMT_LONG GMT_strrcmp (char *str1, char *str2)
{
	/* Compares str1 with str2 starting at the right of each string
	 * until the front of str1 is reached while case is ignored.
	 * When the strings match until that point, the routine returns the
	 * length of str1, otherwise it returns 0.
	 */
	GMT_LONG i, j, l;
	l = i = strlen (str1), j = strlen (str2);
	if (j < i) return 0;
	--i, --j;
	while (i >= 0 && tolower((unsigned char) str1[i]) == tolower((unsigned char) str2[j])) --i, --j;
	if (i >= 0) return 0;
	return l;
}

GMT_LONG GMT_strtok (struct GMT_CTRL *C, const char *string, const char *sep, GMT_LONG *pos, char *token)
{
	/* Reentrant replacement for strtok that uses no static variables.
	 * Breaks string into tokens separated by one of more separator
	 * characters (in sep).  Set *pos to 0 before first call.  Unlike
	 * strtok, always pass the original string as first argument.
	 * Returns 1 if it finds a token and 0 if no more tokens left.
	 * pos is updated and token is returned.  char *token must point
	 * to memory of length >= strlen (string).
	 * string is not changed by GMT_strtok.
	 */

	GMT_LONG i, j, string_len;

	string_len = strlen (string);

	/* Wind up *pos to first non-separating character: */
	while (string[*pos] && strchr (sep, (int)string[*pos])) (*pos)++;

	token[0] = 0;	/* Initialize token to NULL in case we are at end */

	if (*pos >= string_len || string_len == 0) return 0;	/* Got NULL string or no more string left to search */

	/* Search for next non-separating character */
	i = *pos; j = 0;
	while (string[i] && !strchr (sep, (int)string[i])) token[j++] = string[i++];
	token[j] = 0;	/* Add terminating \0 */

	/* Wind up *pos to next non-separating character */
	while (string[i] && strchr (sep, (int)string[i])) i++;
	*pos = i;

	return 1;
}

GMT_LONG GMT_strtok1 (const char *string, const char sep, GMT_LONG *pos, char *token)
{
	/* strtok-like function that retrieves tokens separate by a single character sep.
	 * Unlike strtok, a token returned may be NULL (if two sep are found in sequence).
	 * Breaks string into tokens separated by the character seg.  Set *pos to 0
	 * before first call.  Unlike strtok, always pass the original string as first argument.
	 * Returns 1 if it finds a token and 0 if no more tokens left.
	 * pos is updated and token is returned.  char *token must point
	 * to memory of length >= strlen (string).
	 * string is not changed by GMT_strtok1.
	 */

	GMT_LONG i, j, string_len;

	string_len = strlen (string);

	token[0] = 0;	/* Initialize token to NULL in case we are at end */

	if (*pos >= string_len || string_len == 0) return 0;	/* Got NULL string or no more string left to search */

	/* Search for next non-separating character */
	i = *pos; j = 0;
	while (string[i] && string[i] != sep) token[j++] = string[i++];
	token[j] = 0;	/* Add terminating \0 */

	/* Increase *pos to next non-separating character */
	if (string[i] == sep) i++;
	*pos = i;

	return 1;
}

GMT_LONG GMT_getmodopt (struct GMT_CTRL *C, const char *string, const char *sep, GMT_LONG *pos, char *token)
{
	/* Breaks string into tokens separated by one of more modifier separator
	 * characters (in sep) following a +.  Set *pos to 0 before first call.
	 * Returns 1 if it finds a token and 0 if no more tokens left.
	 * pos is updated and token is returned.  char *token must point
	 * to memory of length >= strlen (string).
	 * string is not changed by GMT_getmodopt.
	 */

	GMT_LONG i, j, string_len, done = FALSE;

	string_len = strlen (string);
	token[0] = 0;	/* Initialize token to NULL in case we are at end */

	while (!done) {
		/* Wind up *pos to first + */
		while (string[*pos] && string[*pos] != '+') (*pos)++;

		if (*pos >= string_len || string_len == 0) return 0;	/* Got NULL string or no more string left to search */

		(*pos)++;	/* Go and examine if next char is one of requested modifier options */
		done = (strchr (sep, (int)string[*pos]) != NULL);	/* TRUE if this is our guy */
	}

	/* Search for next +sep occurrence */
	i = *pos; j = 0;
	while (string[i] && !(string[i] == '+' && strchr (sep, (int)string[i+1]))) token[j++] = string[i++];
	token[j] = 0;	/* Add terminating \0 */

	*pos = i;

	return 1;
}

double GMT_get_map_interval (struct GMT_CTRL *C, struct GMT_PLOT_AXIS_ITEM *T) {

	switch (T->unit) {
		case 'd':	/* arc Degrees */
			return (T->interval);
			break;
		case 'm':	/* arc Minutes */
			return (T->interval * GMT_MIN2DEG);
			break;
#ifdef GMT_COMPAT
		case 'c':	/* arc Seconds [deprecated] */
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Second interval unit c is deprecated; use s instead\n");
#endif
		case 's':	/* arc Seconds */
			return (T->interval * GMT_SEC2DEG);
			break;
		default:
			return (T->interval);
			break;
	}
}

GMT_LONG GMT_just_decode (struct GMT_CTRL *C, char *key, GMT_LONG def)
{
	/* Converts justification info (key) like BL (bottom left) to justification indices
	 * def = default value.
	 * When def % 4 = 0, horizontal position must be specified
	 * When def / 4 = 3, vertical position must be specified
	 */
	GMT_LONG i, j, k;

	if (isdigit ((int)key[0])) return (atoi(key));

	i = def % 4;
	j = def / 4;
	for (k = 0; k < (GMT_LONG)strlen (key); k++) {
		switch (key[k]) {
			case 'b':	/* Bottom baseline */
			case 'B':
				j = 0;
				break;
			case 'm':	/* Middle baseline */
			case 'M':
				j = 1;
				break;
			case 't':	/* Top baseline */
			case 'T':
				j = 2;
				break;
			case 'l':	/* Left Justified */
			case 'L':
				i = 1;
				break;
			case 'c':	/* Center Justified */
			case 'C':
				i = 2;
				break;
			case 'r':	/* Right Justified */
			case 'R':
				i = 3;
				break;
			default:
				return (-99);
		}
	}

	if (i == 0) {
		GMT_report (C, GMT_MSG_FATAL, "Horizontal text justification not set, defaults to L(eft)\n");
		i = 1;
	}
	if (j == 3) {
		GMT_report (C, GMT_MSG_FATAL, "Vertical text justification not set, defaults to B(ottom)\n");
		j = 0;
	}

	return (j * 4 + i);
}

void GMT_smart_justify (struct GMT_CTRL *C, GMT_LONG just, double angle, double dx, double dy, double *x_shift, double *y_shift)
{
	double s, c, xx, yy;
	sincosd (angle, &s, &c);
	xx = (2 - (just%4)) * dx;	/* Smart shift in x */
	yy = (1 - (just/4)) * dy;	/* Smart shift in x */
	*x_shift += c * xx - s * yy;	/* Must account for angle of label */
	*y_shift += s * xx + c * yy;
}

GMT_LONG GMT_verify_expectations (struct GMT_CTRL *C, GMT_LONG wanted, GMT_LONG got, char *item)
{	/* Compare what we wanted with what we got and see if it is OK */
	GMT_LONG error = 0;

	if (wanted == GMT_IS_UNKNOWN) {	/* No expectations set */
		switch (got) {
			case GMT_IS_ABSTIME:	/* Found a T in the string - ABSTIME ? */
				GMT_report (C, GMT_MSG_FATAL, "Error: %s appears to be an Absolute Time String: ", item);
				if (GMT_is_geographic (C, GMT_IN))
					GMT_report (C, GMT_MSG_FATAL, "This is not allowed for a map projection\n");
				else
					GMT_report (C, GMT_MSG_FATAL, "You must specify time data type with option -f.\n");
				error++;
				break;

			case GMT_IS_GEO:	/* Found a : in the string - GEO ? */
				GMT_report (C, GMT_MSG_FATAL, "Warning: %s appears to be a Geographical Location String: ", item);
				if (C->current.proj.projection == GMT_LINEAR)
					GMT_report (C, GMT_MSG_FATAL, "You should append d to the -Jx or -JX projection for geographical data.\n");
				else
					GMT_report (C, GMT_MSG_FATAL, "You should specify geographical data type with option -f.\n");
				GMT_report (C, GMT_MSG_FATAL, "Will proceed assuming geographical input data.\n");
				break;

			case GMT_IS_LON:	/* Found a : in the string and then W or E - LON ? */
				GMT_report (C, GMT_MSG_FATAL, "Warning: %s appears to be a Geographical Longitude String: ", item);
				if (C->current.proj.projection == GMT_LINEAR)
					GMT_report (C, GMT_MSG_FATAL, "You should append d to the -Jx or -JX projection for geographical data.\n");
				else
					GMT_report (C, GMT_MSG_FATAL, "You should specify geographical data type with option -f.\n");
				GMT_report (C, GMT_MSG_FATAL, "Will proceed assuming geographical input data.\n");
				break;

			case GMT_IS_LAT:	/* Found a : in the string and then S or N - LAT ? */
				GMT_report (C, GMT_MSG_FATAL, "Warning: %s appears to be a Geographical Latitude String: ", item);
				if (C->current.proj.projection == GMT_LINEAR)
					GMT_report (C, GMT_MSG_FATAL, "You should append d to the -Jx or -JX projection for geographical data.\n");
				else
					GMT_report (C, GMT_MSG_FATAL, "You should specify geographical data type with option -f.\n");
				GMT_report (C, GMT_MSG_FATAL, "Will proceed assuming geographical input data.\n");
				break;

			case GMT_IS_FLOAT:
				break;
			default:
				break;
		}
	}
	else {
		switch (got) {
			case GMT_IS_NAN:
				GMT_report (C, GMT_MSG_FATAL, "Error: Could not decode %s, return NaN.\n", item);
				error++;
				break;

			case GMT_IS_LAT:
				if (wanted == GMT_IS_LON) {
					GMT_report (C, GMT_MSG_FATAL, "Error: Expected longitude, but %s is a latitude!\n", item);
					error++;
				}
				break;

			case GMT_IS_LON:
				if (wanted == GMT_IS_LAT) {
					GMT_report (C, GMT_MSG_FATAL, "Error: Expected latitude, but %s is a longitude!\n", item);
					error++;
				}
				break;
			default:
				break;
		}
	}
	return (error);
}

void GMT_list_custom_symbols (struct GMT_CTRL *C)
{
	/* Opens up C->init.custom_symbols.lis and dislays the list of custom symbols */

	FILE *fp = NULL;
	char list[GMT_TEXT_LEN256], buffer[GMT_BUFSIZ];

	/* Open the list in $C->session.SHAREDIR */

	GMT_getsharepath (C, "conf", "gmt_custom_symbols", ".conf", list);
	if ((fp = fopen (list, "r")) == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Cannot open file %s\n", list);
		return;
	}

	GMT_message (C, "\t     Available custom symbols (See Appendix N):\n");
	GMT_message (C, "\t     ---------------------------------------------------------\n");
	while (fgets (buffer, GMT_BUFSIZ, fp)) if (!(buffer[0] == '#' || buffer[0] == 0)) GMT_message (C, "\t     %s", buffer);
	fclose (fp);
	GMT_message (C, "\t     ---------------------------------------------------------\n");
}

void GMT_rotate2D (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, double x0, double y0, double angle, double xp[], double yp[])
{	/* Cartesian rotation of x,y in the plane by angle followed by translation by (x0, y0) */
	GMT_LONG i;
	double s, c;

	sincosd (angle, &s, &c);
	for (i = 0; i < n; i++) {	/* Coordinate transformation: Rotate and add new (x0, y0) offset */
		xp[i] = x0 + x[i] * c - y[i] * s;
		yp[i] = y0 + x[i] * s + y[i] * c;
	}
}

GMT_LONG GMT_get_arc (struct GMT_CTRL *C, double x0, double y0, double r, double dir1, double dir2, double **x, double **y)
{
	/* Create an array with a circular arc. r in inches, angles in degrees */

	GMT_LONG i, n;
	double da, s, c, *xx = NULL, *yy = NULL;

	n = irint (D2R * fabs (dir2 - dir1) * r / C->current.setting.map_line_step);
	if (n < 2) n = 2;	/* To prevent division by 0 below */
	xx = GMT_memory (C, NULL, n, double);
	yy = GMT_memory (C, NULL, n, double);
	da = (dir2 - dir1) / (n - 1);
	for (i = 0; i < n; i++) {
		sincosd (dir1 + i * da, &s, &c);
		xx[i] = x0 + r * c;
		yy[i] = y0 + r * s;
	}
	*x = xx;
	*y = yy;

	return (n);
}

/* Here lies GMT Crossover core functions that previously was in X2SYS only */

/* GMT_ysort must be an int since it is passed to qsort! */
#ifndef HAVE_QSORT_R
int GMT_ysort (const void *p1, const void *p2)			/* Must use qsort and thus rely on a global variable */
/* The global double pointer GMT_x2sys_Y must be set to point to the relevant y-array
 * before this call!!! */
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(WIN32)
/* Wonderful news: BSD and GLIBC has different argument order in qsort_r */
int GMT_ysort (void *data, const void *p1, const void *p2)	/* Can use qsort_r and pass the extra (first) argument */
#else
int GMT_ysort (const void *p1, const void *p2, void *data)	/* Can use qsort_r and pass the extra (last) argument */
#endif
{
	struct GMT_XSEGMENT *a = (struct GMT_XSEGMENT *)p1, *b = (struct GMT_XSEGMENT *)p2;
#ifdef HAVE_QSORT_R
	double *GMT_x2sys_Y = data;
#endif

	if (GMT_x2sys_Y[a->start] < GMT_x2sys_Y[b->start]) return -1;
	if (GMT_x2sys_Y[a->start] > GMT_x2sys_Y[b->start]) return  1;

	/* Here they have the same low y-value, now sort on other y value */

	if (GMT_x2sys_Y[a->stop] < GMT_x2sys_Y[b->stop]) return -1;
	if (GMT_x2sys_Y[a->stop] > GMT_x2sys_Y[b->stop]) return  1;

	/* Identical */

	return (0);
}

GMT_LONG GMT_init_track (struct GMT_CTRL *C, double y[], GMT_LONG n, struct GMT_XSEGMENT **S)
{
	/* GMT_init_track accepts the y components of an x-y track of length n and returns an array of
	 * line segments that have been sorted on the minimum y-coordinate
	 */

	GMT_LONG a, b, nl = n - 1;
	struct GMT_XSEGMENT *L = NULL;

	if (nl <= 0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT: ERROR in GMT_init_track; nl = %ld\n", nl);
		GMT_exit (EXIT_FAILURE);
	}

	L = GMT_memory (C, NULL, nl, struct GMT_XSEGMENT);

	for (a = 0, b = 1; b < n; a++, b++) {
		if (y[b] < y[a]) {
			L[a].start = b;
			L[a].stop = a;
		}
		else {
			L[a].start = a;
			L[a].stop = b;
		}
	}

	/* Sort on minimum y-coordinate, if tie then on 2nd coordinate */

#ifndef HAVE_QSORT_R
	GMT_x2sys_Y = y;	/* Sort routine needs this global variable pointer */
	qsort (L, (size_t)nl, sizeof (struct GMT_XSEGMENT), GMT_ysort);
	GMT_x2sys_Y = NULL;	/* Set to NULL to indicate not in use */
#elif defined(__APPLE__) || defined(__FreeBSD__)
	/* Wonderful news: BSD and GLIBC has different argument order in qsort_r */
	qsort_r (L, (size_t)nl, sizeof (struct GMT_XSEGMENT), y, GMT_ysort);
#else
	qsort_r (L, (size_t)nl, sizeof (struct GMT_XSEGMENT), GMT_ysort, y);
#endif

	*S = L;

	return (GMT_NOERROR);
}

void GMT_x_alloc (struct GMT_CTRL *C, struct GMT_XOVER *X, GMT_LONG nx_alloc) {
	if (nx_alloc < 0) {	/* Initial allocation */
		nx_alloc = -nx_alloc;
		X->x = GMT_memory (C, NULL,nx_alloc, double);
		X->y = GMT_memory (C, NULL, nx_alloc, double);
		X->xnode[0] = GMT_memory (C, NULL, nx_alloc, double);
		X->xnode[1] = GMT_memory (C, NULL, nx_alloc, double);
	}
	else {	/* Increment */
		X->x = GMT_memory (C, X->x, nx_alloc, double);
		X->y = GMT_memory (C, X->y, nx_alloc, double);
		X->xnode[0] = GMT_memory (C, X->xnode[0], nx_alloc, double);
		X->xnode[1] = GMT_memory (C, X->xnode[1], nx_alloc, double);
	}
}

GMT_LONG GMT_crossover (struct GMT_CTRL *C, double xa[], double ya[], GMT_LONG *sa0, struct GMT_XSEGMENT A[], GMT_LONG na, double xb[], double yb[], GMT_LONG *sb0, struct GMT_XSEGMENT B[], GMT_LONG nb, GMT_LONG internal, struct GMT_XOVER *X)
{
	GMT_LONG this_a, this_b, n_seg_a, new_a, new_b, new_a_time = FALSE, xa_OK = FALSE, xb_OK = FALSE, n_seg_b;
	GMT_LONG nx, xa_start = 0, xa_stop = 0, xb_start = 0, xb_stop = 0, ta_start = 0, ta_stop = 0, tb_start, tb_stop;
	GMT_LONG *sa = NULL, *sb = NULL, nx_alloc;
	double del_xa, del_xb, del_ya, del_yb, i_del_xa, i_del_xb, i_del_ya, i_del_yb, slp_a, slp_b, xc, yc, tx_a, tx_b;

	if (na < 2 || nb < 2) return (0);	/* Need at least 2 points to make a segment */

	this_a = this_b = nx = 0;
	new_a = new_b = TRUE;
	nx_alloc = GMT_SMALL_CHUNK;

	n_seg_a = na - 1;
	n_seg_b = nb - 1;

	/* Assign pointers to segment info given, or initialize zero arrays if not given */
	sa = (sa0) ? sa0 : GMT_memory (C, NULL, na, GMT_LONG);
	sb = (sb0) ? sb0 : GMT_memory (C, NULL, nb, GMT_LONG);

	GMT_x_alloc (C, X, -nx_alloc);

	while (this_a < n_seg_a && yb[B[this_b].start] > ya[A[this_a].stop]) this_a++;	/* Go to first possible A segment */

	while (this_a < n_seg_a) {

		/* First check for internal neighboring segments which cannot cross */

		if (internal && (this_a == this_b || (A[this_a].stop == B[this_b].start || A[this_a].start == B[this_b].stop) || (A[this_a].start == B[this_b].start || A[this_a].stop == B[this_b].stop))) {	/* Neighboring segments cannot cross */
			this_b++;
			new_b = TRUE;
		}
		else if (yb[B[this_b].start] > ya[A[this_a].stop]) {	/* Reset this_b and go to next A */
			this_b = n_seg_b;
		}
		else if (yb[B[this_b].stop] < ya[A[this_a].start]) {	/* Must advance B in y-direction */
			this_b++;
			new_b = TRUE;
		}
		else if (sb[B[this_b].stop] != sb[B[this_b].start]) {	/* Must advance B in y-direction since this segment crosses multiseg boundary*/
			this_b++;
			new_b = TRUE;
		}
		else {	/* Current A and B segments overlap in y-range */

			if (new_a) {	/* Must sort this A new segment in x */
				if (xa[A[this_a].stop] < xa[A[this_a].start]) {
					xa_start = A[this_a].stop;
					xa_stop  = A[this_a].start;
				}
				else {
					xa_start = A[this_a].start;
					xa_stop  = A[this_a].stop;
				}
				new_a = FALSE;
				new_a_time = TRUE;
				xa_OK = (sa[xa_start] == sa[xa_stop]);	/* FALSE if we cross between multiple segments */
			}

			if (new_b) {	/* Must sort this new B segment in x */
				if (xb[B[this_b].stop] < xb[B[this_b].start]) {
					xb_start = B[this_b].stop;
					xb_stop  = B[this_b].start;
				}
				else {
					xb_start = B[this_b].start;
					xb_stop  = B[this_b].stop;
				}
				new_b = FALSE;
				xb_OK = (sb[xb_start] == sb[xb_stop]);	/* FALSE if we cross between multiple segments */
			}

			/* OK, first check for any overlap in x range */

			if (xa_OK && xb_OK && !((xa[xa_stop] < xb[xb_start]) || (xa[xa_start] > xb[xb_stop]))) {

				/* We have segment overlap in x.  Now check if the segments cross  */

				del_xa = xa[xa_stop] - xa[xa_start];
				del_xb = xb[xb_stop] - xb[xb_start];
				del_ya = ya[xa_stop] - ya[xa_start];
				del_yb = yb[xb_stop] - yb[xb_start];

				if (del_xa == 0.0) {	/* Vertical A segment: Special case */
					if (del_xb == 0.0) {	/* Two vertical segments with some overlap */
						double y4[4];
						/* Assign as crossover the middle of the overlapping segments */
						X->x[nx] = xa[xa_start];
						y4[0] = ya[xa_start];	y4[1] = ya[xa_stop];	y4[2] = yb[xb_start];	y4[3] = yb[xb_stop];
						GMT_sort_array (C, y4, 4, GMTAPI_DOUBLE);
						if (y4[1] != y4[2]) {
							X->y[nx] = 0.5 * (y4[1] + y4[2]);
							X->xnode[0][nx] = 0.5 * (xa_start + xa_stop);
							X->xnode[1][nx] = 0.5 * (xb_start + xb_stop);
							nx++;
						}
					}
					else {
						i_del_xb = 1.0 / del_xb;
						yc = yb[xb_start] + (xa[xa_start] - xb[xb_start]) * del_yb * i_del_xb;
						if (!(yc < ya[A[this_a].start] || yc > ya[A[this_a].stop])) {	/* Did cross within the segment extents */
							/* Only accept xover if occurring before segment end (in time) */

							if (xb_start < xb_stop) {
								tb_start = xb_start;	/* B Node first in time */
								tb_stop = xb_stop;	/* B Node last in time */
							}
							else {
								tb_start = xb_stop;	/* B Node first in time */
								tb_stop = xb_start;	/* B Node last in time */
							}
							if (new_a_time) {
								if (xa_start < xa_stop) {
									ta_start = xa_start;	/* A Node first in time */
									ta_stop = xa_stop;	/* A Node last in time */
								}
								else {
									ta_start = xa_stop;	/* A Node first in time */
									ta_stop = xa_start;	/* A Node last in time */
								}
								new_a_time = FALSE;
							}

							tx_a = ta_start + fabs ((yc - ya[ta_start]) / del_ya);
							tx_b = tb_start + fabs (xa[xa_start] - xb[tb_start]) * i_del_xb;
							if (tx_a < ta_stop && tx_b < tb_stop) {
								X->x[nx] = xa[xa_start];
								X->y[nx] = yc;
								X->xnode[0][nx] = tx_a;
								X->xnode[1][nx] = tx_b;
								nx++;
							}
						}
					}
				}
				else if (del_xb == 0.0) {	/* Vertical B segment: Special case */

					i_del_xa = 1.0 / del_xa;
					yc = ya[xa_start] + (xb[xb_start] - xa[xa_start]) * del_ya * i_del_xa;
					if (!(yc < yb[B[this_b].start] || yc > yb[B[this_b].stop])) {	/* Did cross within the segment extents */
						/* Only accept xover if occurring before segment end (in time) */

						if (xb_start < xb_stop) {
							tb_start = xb_start;	/* B Node first in time */
							tb_stop = xb_stop;	/* B Node last in time */
						}
						else {
							tb_start = xb_stop;	/* B Node first in time */
							tb_stop = xb_start;	/* B Node last in time */
						}
						if (new_a_time) {
							if (xa_start < xa_stop) {
								ta_start = xa_start;	/* A Node first in time */
								ta_stop = xa_stop;	/* A Node last in time */
							}
							else {
								ta_start = xa_stop;	/* A Node first in time */
								ta_stop = xa_start;	/* A Node last in time */
							}
							new_a_time = FALSE;
						}

						tx_a = ta_start + fabs (xb[xb_start] - xa[ta_start]) * i_del_xa;
						tx_b = tb_start + fabs ((yc - yb[tb_start]) / del_yb);
						if (tx_a < ta_stop && tx_b < tb_stop) {
							X->x[nx] = xb[xb_start];
							X->y[nx] = yc;
							X->xnode[0][nx] = tx_a;
							X->xnode[1][nx] = tx_b;
							nx++;
						}
					}
				}
				else if (del_ya == 0.0) {	/* Horizontal A segment: Special case */

					if (del_yb == 0.0) {	/* Two horizontal segments with some overlap */
						double x4[4];
						/* Assign as crossover the middle of the overlapping segments */
						X->y[nx] = ya[xa_start];
						x4[0] = xa[xa_start];	x4[1] = xa[xa_stop];	x4[2] = xb[xb_start];	x4[3] = xb[xb_stop];
						GMT_sort_array (C, x4, 4, GMTAPI_DOUBLE);
						if (x4[1] != x4[2]) {
							X->x[nx] = 0.5 * (x4[1] + x4[2]);
							X->xnode[0][nx] = 0.5 * (xa_start + xa_stop);
							X->xnode[1][nx] = 0.5 * (xb_start + xb_stop);
							nx++;
						}
					}
					else {
						i_del_yb = 1.0 / del_yb;
						xc = xb[xb_start] + (ya[xa_start] - yb[xb_start]) * del_xb * i_del_yb;
						if (!(xc < xa[xa_start] || xc > xa[xa_stop])) {	/* Did cross within the segment extents */

							/* Only accept xover if occurring before segment end (in time) */

							if (xb_start < xb_stop) {
								tb_start = xb_start;	/* B Node first in time */
								tb_stop = xb_stop;	/* B Node last in time */
							}
							else {
								tb_start = xb_stop;	/* B Node first in time */
								tb_stop = xb_start;	/* B Node last in time */
							}
							if (new_a_time) {
								if (xa_start < xa_stop) {
									ta_start = xa_start;	/* A Node first in time */
									ta_stop = xa_stop;	/* A Node last in time */
								}
								else {
									ta_start = xa_stop;	/* A Node first in time */
									ta_stop = xa_start;	/* A Node last in time */
								}
								new_a_time = FALSE;
							}

							tx_a = ta_start + fabs (xc - xa[ta_start]) / del_xa;
							tx_b = tb_start + fabs ((ya[xa_start] - yb[tb_start]) * i_del_yb);
							if (tx_a < ta_stop && tx_b < tb_stop) {
								X->y[nx] = ya[xa_start];
								X->x[nx] = xc;
								X->xnode[0][nx] = tx_a;
								X->xnode[1][nx] = tx_b;
								nx++;
							}
						}
					}
				}
				else if (del_yb == 0.0) {	/* Horizontal B segment: Special case */

					i_del_ya = 1.0 / del_ya;
					xc = xa[xa_start] + (yb[xb_start] - ya[xa_start]) * del_xa * i_del_ya;
					if (!(xc < xb[xb_start] || xc > xb[xb_stop])) {	/* Did cross within the segment extents */

						/* Only accept xover if occurring before segment end (in time) */

						if (xb_start < xb_stop) {
							tb_start = xb_start;	/* B Node first in time */
							tb_stop = xb_stop;	/* B Node last in time */
						}
						else {
							tb_start = xb_stop;	/* B Node first in time */
							tb_stop = xb_start;	/* B Node last in time */
						}
						if (new_a_time) {
							if (xa_start < xa_stop) {
								ta_start = xa_start;	/* A Node first in time */
								ta_stop = xa_stop;	/* A Node last in time */
							}
							else {
								ta_start = xa_stop;	/* A Node first in time */
								ta_stop = xa_start;	/* A Node last in time */
							}
							new_a_time = FALSE;
						}

						tx_a = ta_start + fabs ((yb[xb_start] - ya[ta_start]) * i_del_ya);
						tx_b = tb_start + fabs (xc - xb[tb_start]) / del_xb;
						if (tx_a < ta_stop && tx_b < tb_stop) {
							X->y[nx] = yb[xb_start];
							X->x[nx] = xc;
							X->xnode[0][nx] = tx_a;
							X->xnode[1][nx] = tx_b;
							nx++;
						}
					}
				}
				else {	/* General case */

					i_del_xa = 1.0 / del_xa;
					i_del_xb = 1.0 / del_xb;
					slp_a = del_ya * i_del_xa;
					slp_b = del_yb * i_del_xb;
					if (slp_a == slp_b) {	/* Segments are parallel */
						double x4[4];
						/* Assign as possible crossover the middle of the overlapping segments */
						x4[0] = xa[xa_start];	x4[1] = xa[xa_stop];	x4[2] = xb[xb_start];	x4[3] = xb[xb_stop];
						GMT_sort_array (C, x4, 4, GMTAPI_DOUBLE);
						if (x4[1] != x4[2]) {
							xc = 0.5 * (x4[1] + x4[2]);
							yc = slp_a * (xc - xa[xa_start]) + ya[xa_start];
							if ((slp_b * (xc - xb[xb_start]) + yb[xb_start]) == yc) {
								X->y[nx] = yc;
								X->x[nx] = xc;
								X->xnode[0][nx] = 0.5 * (xa_start + xa_stop);
								X->xnode[1][nx] = 0.5 * (xb_start + xb_stop);
								nx++;
							}
						}
					}
					else {	/* Segments are not parallel */
						xc = (yb[xb_start] - ya[xa_start] + slp_a * xa[xa_start] - slp_b * xb[xb_start]) / (slp_a - slp_b);
						if (!(xc < xa[xa_start] || xc > xa[xa_stop] || xc < xb[xb_start] || xc > xb[xb_stop])) {	/* Did cross within the segment extents */

							/* Only accept xover if occurring before segment end (in time) */

							if (xb_start < xb_stop) {
								tb_start = xb_start;	/* B Node first in time */
								tb_stop = xb_stop;	/* B Node last in time */
							}
							else {
								tb_start = xb_stop;	/* B Node first in time */
								tb_stop = xb_start;	/* B Node last in time */
							}
							if (new_a_time) {
								if (xa_start < xa_stop) {
									ta_start = xa_start;	/* A Node first in time */
									ta_stop = xa_stop;	/* A Node last in time */
								}
								else {
									ta_start = xa_stop;	/* A Node first in time */
									ta_stop = xa_start;	/* A Node last in time */
								}
								new_a_time = FALSE;
							}

							tx_a = ta_start + fabs (xc - xa[ta_start]) * i_del_xa;
							tx_b = tb_start + fabs (xc - xb[tb_start]) * i_del_xb;
							if (tx_a < ta_stop && tx_b < tb_stop) {
								X->x[nx] = xc;
								X->y[nx] = ya[xa_start] + (xc - xa[xa_start]) * slp_a;
								X->xnode[0][nx] = tx_a;
								X->xnode[1][nx] = tx_b;
								nx++;
							}
						}
					}
				}

				if (nx == nx_alloc) {
					nx_alloc <<= 1;
					GMT_x_alloc (C, X, nx_alloc);
				}
			} /* End x-overlap */

			this_b++;
			new_b = TRUE;

		} /* End y-overlap */

		if (this_b == n_seg_b) {
			this_a++;
			this_b = (internal) ? this_a : 0;
			new_a = new_b = TRUE;
		}

	} /* End while loop */

	if (!sa0) GMT_free (C, sa);
	if (!sb0) GMT_free (C, sb);

	if (nx == 0) GMT_x_free (C, X);	/* Nothing to write home about... */

	return (nx);
}

void GMT_x_free (struct GMT_CTRL *C, struct GMT_XOVER *X) {
	GMT_free (C, X->x);
	GMT_free (C, X->y);
	GMT_free (C, X->xnode[0]);
	GMT_free (C, X->xnode[1]);
}

GMT_LONG GMT_linear_array (struct GMT_CTRL *C, double min, double max, double delta, double phase, double **array)
{
	/* Create an array of values between min and max, with steps delta and given phase.
	   Example: min = 0, max = 9, delta = 2, phase = 1
	   Result: 1, 3, 5, 7, 9
	*/

	GMT_LONG first, last, i, n;
	double *val = NULL;

	if (delta <= 0.0) return (0);

	/* Undo the phase and scale by 1/delta */
	min = (min - phase) / delta;
	max = (max - phase) / delta;

	/* Look for first value */
	first = (GMT_LONG) floor (min);
	while (min - first > GMT_SMALL) first++;

	/* Look for last value */
	last = (GMT_LONG) ceil (max);
	while (last - max > GMT_SMALL) last--;

	n = last - first + 1;
	if (n <= 0) return (0);

	/* Create an array of n equally spaced elements */
	val = GMT_memory (C, NULL, n, double);
	for (i = 0; i < n; i++) val[i] = phase + (first + i) * delta;	/* Rescale to original values */

	*array = val;

	return (n);
}

GMT_LONG GMT_log_array (struct GMT_CTRL *C, double min, double max, double delta, double **array)
{
	GMT_LONG first, last, i, n, nticks;
	double *val = NULL, tvals[10];

	/* Because min and max may be tiny values (e.g., 10^-20) we must do all calculations on the log10 (value) */

	if (GMT_IS_ZERO (delta)) return (0);
	min = d_log10 (C, min);
	max = d_log10 (C, max);
	first = (GMT_LONG) floor (min);
	last = (GMT_LONG) ceil (max);

	if (delta < 0) {	/* Coarser than every magnitude */
		n = GMT_linear_array (C, min, max, fabs (delta), 0.0, array);
		for (i = 0; i < n; i++) (*array)[i] = pow (10.0, (*array)[i]);
		return (n);
	}

	tvals[0] = 0.0;	/* Common to all */
	switch (irint (fabs (delta))) {
		case 2:	/* Annotate 1, 2, 5, 10 */
			tvals[1] = d_log10 (C, 2.0);
			tvals[2] = d_log10 (C, 5.0);
			tvals[3] = 1.0;
			nticks = 3;
			break;
		case 3:	/* Annotate 1, 2, ..., 8, 9, 10 */
			nticks = 9;
			for (i = 1; i <= nticks; i++) tvals[i] = d_log10 (C, (double)(i + 1));
			break;
		default:	/* Annotate just 1 and 10 */
			nticks = 1;
			tvals[1] = 1.0;
	}

	/* Assign memory to array (may be a bit too much) */
	n = (last - first + 1) * nticks + 1;
	val = GMT_memory (C, NULL, n, double);

	/* Find the first logarithm larger than min */
	i = 0;
	val[0] = (double) first;
	while (min - val[0] > GMT_SMALL && i < nticks) {
		i++;
		val[0] = first + tvals[i];
	}

	/* Find the first logarithm larger than max */
	n = 0;
	while (val[n] - max < GMT_SMALL) {
		if (i >= nticks) {
			i -= nticks;
			first++;
		}
		i++; n++;
		val[n] = first + tvals[i];
	}

	/* val[n] is too large, so we use only the first n, until val[n-1] */
	val = GMT_memory (C, val, n, double);

	/* Convert from log10 to values */
	for (i = 0; i < n; i++) val[i] = pow (10.0, val[i]);

	*array = val;

	return (n);
}

GMT_LONG GMT_pow_array (struct GMT_CTRL *C, double min, double max, double delta, GMT_LONG x_or_y, double **array)
{
	GMT_LONG i, n;
	double *val = NULL, v0, v1;

	if (delta <= 0.0) return (0);

	if (C->current.map.frame.axis[x_or_y].type != GMT_POW) return (GMT_linear_array (C, min, max, delta, 0.0, array));

	if (x_or_y == 0) { /* x-axis */
		C->current.proj.fwd_x (C, min, &v0);
		C->current.proj.fwd_x (C, max, &v1);
		n = GMT_linear_array (C, v0, v1, delta, 0.0, &val);
		for (i = 0; i < n; i++) C->current.proj.inv_x (C, &val[i], val[i]);
	}
	else {	/* y-axis */
		C->current.proj.fwd_y (C, min, &v0);
		C->current.proj.fwd_y (C, max, &v1);
		n = GMT_linear_array (C, v0, v1, delta, 0.0, &val);
		for (i = 0; i < n; i++) C->current.proj.inv_y (C, &val[i], val[i]);
	}

	*array = val;

	return (n);
}

GMT_LONG GMT_time_array (struct GMT_CTRL *C, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double **array)
{	/* When T->active is TRUE we must return interval start/stop even if outside min/max range */
	GMT_LONG n_alloc = GMT_SMALL_CHUNK, n = 0, interval;
	struct GMT_MOMENT_INTERVAL I;
	double *val = NULL;

	if (!T->active) return (0);
	val = GMT_memory (C, NULL, n_alloc, double);
	I.unit = T->unit;
	I.step = (GMT_LONG)T->interval;
	interval = (T->type == 'i' || T->type == 'I');	/* Only for i/I axis items */
	GMT_moment_interval (C, &I, min, TRUE);	/* First time we pass TRUE for initialization */
	while (I.dt[0] <= max) {		/* As long as we are not gone way past the end time */
		if (I.dt[0] >= min || interval) val[n++] = I.dt[0];		/* Was inside region */
		GMT_moment_interval (C, &I, 0.0, FALSE);			/* Advance to next interval */
		if (n == n_alloc) {					/* Allocate more space */
			n_alloc <<= 1;
			val = GMT_memory (C, val, n_alloc, double);
		}
	}
	if (interval) val[n++] = I.dt[0];	/* Must get end of interval too */
	val = GMT_memory (C, val, n, double);

	*array = val;

	return (n);
}

GMT_LONG gmt_load_custom_annot (struct GMT_CTRL *C, struct GMT_PLOT_AXIS *A, char item, double **xx, char ***labels)
{
	/* Reads a file with one or more records of the form
	 * value	types	[label]
	 * where value is the coordinate of the tickmark, types is a combination
	 * of a|i (annot or interval annot), f (tick), or g (gridline).
	 * The a|i will take a label string (or sentence).
	 * The item argument specifies which type to consider [a|i,f,g].  We return
	 * an array with coordinates and labels, and set interval to TRUE if applicable.
	 */
	GMT_LONG k = 0, nc, found, n_alloc = GMT_SMALL_CHUNK, n_annot = 0, n_int = 0, text, error = 0;
	double *x = NULL;
	char **L = NULL, line[GMT_BUFSIZ], str[GMT_TEXT_LEN64], type[8], txt[GMT_BUFSIZ];
	FILE *fp = GMT_fopen (C, A->file_custom, "r");

	text = ((item == 'a' || item == 'i') && labels);
	x = GMT_memory (C, NULL, n_alloc, double);
	if (text) L = GMT_memory (C, NULL, n_alloc, char *);
	while (GMT_fgets (C, line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#') continue;
		nc = sscanf (line, "%s %s %[^\n]", str, type, txt);
		found = ((item == 'a' && (strchr (type, 'a') || strchr (type, 'i'))) || (strchr (type, item) != NULL));
		if (!found) continue;	/* Not the type we were requesting */
		if (strchr (type, 'i')) n_int++;
		if (strchr (type, 'a')) n_annot++;
		error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][A->id], GMT_scanf (C, str, C->current.io.col_type[GMT_IN][A->id], &x[k]), str);
		if (text && nc == 3) L[k] = strdup (txt);
		k++;
		if (k == n_alloc) {
			n_alloc <<= 2;
			x = GMT_memory (C, x, n_alloc, double);
			if (text) L = GMT_memory (C, L, n_alloc, char *);
		}
	}
	GMT_fclose (C, fp);
	if (k == 0) {	/* No such items */
		GMT_free (C, x);
		if (text) GMT_free (C, L);
		return (0);
	}
	if (k < n_alloc) {
		x = GMT_memory (C, x, k, double);
		if (text) L = GMT_memory (C, L, k, char *);
	}
	*xx = x;
	if (text) *labels = L;
	return (k);
}

GMT_LONG GMT_coordinate_array (struct GMT_CTRL *C, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double **array, char ***labels)
{
	GMT_LONG n;

	if (!T->active) return (0);	/* Nothing to do */

	if (C->current.map.frame.axis[T->parent].file_custom) {	/* Want custom intervals */
		n = gmt_load_custom_annot (C, &C->current.map.frame.axis[T->parent], tolower((unsigned char) T->type), array, labels);
		return (n);
	}

	switch (C->current.proj.xyz_projection[T->parent]) {
		case GMT_LINEAR:
			n = GMT_linear_array (C, min, max, GMT_get_map_interval (C, T), C->current.map.frame.axis[T->parent].phase, array);
			break;
		case GMT_LOG10:
			n = GMT_log_array (C, min, max, GMT_get_map_interval (C, T), array);
			break;
		case GMT_POW:
			n = GMT_pow_array (C, min, max, GMT_get_map_interval (C, T), T->parent, array);
			break;
		case GMT_TIME:
			n = GMT_time_array (C, min, max, T, array);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Error: Invalid projection type (%ld) passed to GMT_coordinate_array!\n", C->current.proj.xyz_projection[T->parent]);
			GMT_exit (EXIT_FAILURE);
			break;
	}
	return (n);
}

GMT_LONG GMT_annot_pos (struct GMT_CTRL *C, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double coord[], double *pos)
{
	/* Calculates the location of the next annotation in user units.  This is
	 * trivial for tick annotations but can be tricky for interval annotations
	 * since the annotation location is not necessarily centered on the interval.
	 * For instance, if our interval is 3 months we do not want "January" centered
	 * on that quarter.  If the position is outside our range we return TRUE
	 */
	double range, start, stop;

	if (T->special) {
		range = 0.5 * (coord[1] - coord[0]);	/* Half width of interval in internal representation */
		start = MAX (min, coord[0]);			/* Start of interval, but not less that start of axis */
		stop  = MIN (max, coord[1]);			/* Stop of interval,  but not beyond end of axis */
		if ((stop - start) < (C->current.setting.time_interval_fraction * range)) return (TRUE);		/* Sorry, fraction not large enough to annotate */
		*pos = 0.5 * (start + stop);				/* Set half-way point */
		if (((*pos) - GMT_CONV_LIMIT) < min || ((*pos) + GMT_CONV_LIMIT) > max) return (TRUE);	/* Outside axis range */
	}
	else if (T->type == 'i' || T->type == 'I') {
		if (GMT_uneven_interval (T->unit) || T->interval != 1.0) {	/* Must find next month to get month centered correctly */
			struct GMT_MOMENT_INTERVAL Inext;
			Inext.unit = T->unit;		/* Initialize MOMENT_INTERVAL structure members */
			Inext.step = 1;
			GMT_moment_interval (C, &Inext, coord[0], TRUE);	/* Get this one interval only */
			range = 0.5 * (Inext.dt[1] - Inext.dt[0]);	/* Half width of interval in internal representation */
			start = MAX (min, Inext.dt[0]);			/* Start of interval, but not less that start of axis */
			stop  = MIN (max, Inext.dt[1]);			/* Stop of interval,  but not beyond end of axis */
		}
		else {
			range = 0.5 * (coord[1] - coord[0]);	/* Half width of interval in internal representation */
			start = MAX (min, coord[0]);			/* Start of interval, but not less that start of axis */
			stop  = MIN (max, coord[1]);			/* Stop of interval,  but not beyond end of axis */
		}
		if ((stop - start) < (C->current.setting.time_interval_fraction * range)) return (TRUE);		/* Sorry, fraction not large enough to annotate */
		*pos = 0.5 * (start + stop);				/* Set half-way point */
		if (((*pos) - GMT_CONV_LIMIT) < min || ((*pos) + GMT_CONV_LIMIT) > max) return (TRUE);	/* Outside axis range */
	}
	else if (coord[0] < (min - GMT_CONV_LIMIT) || coord[0] > (max + GMT_CONV_LIMIT))		/* Outside axis range */
		return (TRUE);
	else
		*pos = coord[0];

	return (FALSE);
}

GMT_LONG GMT_get_coordinate_label (struct GMT_CTRL *C, char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct GMT_PLOT_AXIS_ITEM *T, double coord)
{
	/* Returns the formatted annotation string for the non-geographic axes */

	switch (C->current.map.frame.axis[T->parent].type) {
		case GMT_LINEAR:
#if 0
			GMT_near_zero_roundoff_fixer_upper (&coord, T->parent);	/* Try to adjust those ~0 "gcc -O" values to exact 0 */
#endif
			sprintf (string, format, coord);
			break;
		case GMT_LOG10:
			sprintf (string, "%ld", (GMT_LONG)irint (d_log10 (C, coord)));
			break;
		case GMT_POW:
			if (C->current.proj.xyz_projection[T->parent] == GMT_POW)
				sprintf (string, format, coord);
			else
				sprintf (string, "10@+%ld@+", (GMT_LONG)irint (d_log10 (C, coord)));
			break;
		case GMT_TIME:
			GMT_get_time_label (C, string, P, T, coord);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Error: Wrong type (%ld) passed to GMT_get_coordinate_label!\n", C->current.map.frame.axis[T->parent].type);
			GMT_exit (EXIT_FAILURE);
			break;
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_polar_adjust (struct GMT_CTRL *C, GMT_LONG side, double angle, double x, double y)
{
	GMT_LONG justify, left, right, top, bottom, low;
	double x0, y0;

	/* GMT_geo_to_xy (C->current.proj.central_meridian, C->current.proj.pole, &x0, &y0); */

	x0 = C->current.proj.c_x0;
	y0 = C->current.proj.c_y0;
	if (C->current.proj.north_pole) {
		low = 0;
		left = 7;
		right = 5;
	}
	else {
		low = 2;
		left = 5;
		right = 7;
	}
	if ((y - y0 - GMT_SMALL) > 0.0) { /* i.e., y > y0 */
		top = 2;
		bottom = 10;
	}
	else {
		top = 10;
		bottom = 2;
	}
	if (C->current.proj.projection == GMT_POLAR && C->current.proj.got_azimuths) l_swap (left, right);	/* Because with azimuths we get confused... */
	if (C->current.proj.projection == GMT_POLAR && C->current.proj.got_elevations) {
		l_swap (top, bottom);	/* Because with elevations we get confused... */
		l_swap (left, right);
		low = 2 - low;
	}
	if (side%2) {	/* W and E border */
		if ((y - y0 + GMT_SMALL) > 0.0)
			justify = (side == 1) ? left : right;
		else
			justify = (side == 1) ? right : left;
	}
	else {
		if (C->current.map.frame.horizontal) {
			if (side == low)
				justify = (GMT_IS_ZERO (angle - 180.0)) ? bottom : top;
			else
				justify = (GMT_IS_ZERO (angle)) ? top : bottom;
			if (C->current.proj.got_elevations && (GMT_IS_ZERO (angle - 180.0) || GMT_IS_ZERO (angle))) justify = (justify + 8) % 16;
		}
		else {
			if (x >= x0)
				justify = (side == 2) ? left : right;
			else
				justify = (side == 2) ? right : left;
		}
	}
	return (justify);
}

GMT_LONG gmt_get_label_parameters (struct GMT_CTRL *C, GMT_LONG side, double line_angle, GMT_LONG type, double *text_angle, GMT_LONG *justify)
{
	GMT_LONG ok;

	*text_angle = line_angle;
#ifdef OPT_WORKS_BADLY
	if (*text_angle < -90.0) *text_angle += 360.0;
	if (C->current.map.frame.horizontal && !(side%2)) *text_angle += 90.0;
	if (*text_angle > 270.0 ) *text_angle -= 360.0;
	else if (*text_angle > 90.0) *text_angle -= 180.0;
#else
	if ( (*text_angle + 90.0) < GMT_CONV_LIMIT) *text_angle += 360.0;
	if (C->current.map.frame.horizontal && !(side%2)) *text_angle += 90.0;
	if ( (*text_angle - 270.0) > GMT_CONV_LIMIT ) *text_angle -= 360.0;
	else if ( (*text_angle - 90.0) > GMT_CONV_LIMIT) *text_angle -= 180.0;
#endif

	if (type == 0 && C->current.setting.map_annot_oblique & 2) *text_angle = 0.0;	/* Force horizontal lon annotation */
	if (type == 1 && C->current.setting.map_annot_oblique & 4) *text_angle = 0.0;	/* Force horizontal lat annotation */

	switch (side) {
		case 0:		/* S */
			if (C->current.map.frame.horizontal)
				*justify = (C->current.proj.got_elevations) ? 2 : 10;
			else
				*justify = ((*text_angle) < 0.0) ? 5 : 7;
			break;
		case 1:		/* E */
			if (type == 1 && C->current.setting.map_annot_oblique & 32) {
				*text_angle = 90.0;	/* Force parallel lat annotation */
				*justify = 10;
			}
			else
				*justify = 5;
			break;
		case 2:		/* N */
			if (C->current.map.frame.horizontal)
				*justify = (C->current.proj.got_elevations) ? 10 : 2;
			else
				*justify = ((*text_angle) < 0.0) ? 7 : 5;
			break;
		default:	/* W */
			if (type == 1 && C->current.setting.map_annot_oblique & 32) {
				*text_angle = 90.0;	/* Force parallel lat annotation */
				*justify = 2;
			}
			else
				*justify = 7;
			break;
	}

	if (C->current.map.frame.horizontal) return (TRUE);

	switch (side) {
		case 0:		/* S */
		case 2:		/* N */
			ok = (fabs ((*text_angle)) >= C->current.setting.map_annot_min_angle);
			break;
		default:	/* E or W */
			ok = (fabs ((*text_angle)) <= (90.0 - C->current.setting.map_annot_min_angle));
			break;
	}
	return (ok);
}

GMT_LONG gmt_gnomonic_adjust (struct GMT_CTRL *C, double angle, double x, double y)
{
	/* Called when GNOMONIC and global region.  angle has been fixed to the +- 90 range */
	/* This is a kludge until we rewrite the entire justification stuff */
	GMT_LONG inside;
	double xp, yp;

	/* Create a point a small step away from (x,y) along the angle baseline
	 * If it is inside the circle the we want right-justify, else left-justify. */
	sincosd (angle, &yp, &xp);
	xp = xp * C->current.setting.map_line_step + x;
	yp = yp * C->current.setting.map_line_step + y;
	inside = (hypot (xp - C->current.proj.r, yp - C->current.proj.r) < C->current.proj.r);
	return ((inside) ? 7 : 5);
}

GMT_LONG GMT_prepare_label (struct GMT_CTRL *C, double angle, GMT_LONG side, double x, double y, GMT_LONG type, double *line_angle, double *text_angle, GMT_LONG *justify)
{
	GMT_LONG set_angle;

	if (!C->current.proj.edge[side]) return -1;		/* Side doesn't exist */
	if (C->current.map.frame.side[side] < 2) return -1;	/* Don't want labels here */

	if (C->current.map.frame.check_side == TRUE && type != side%2) return -1;

	if (C->current.setting.map_annot_oblique & 16 && !(side%2)) angle = -90.0;	/* gmt_get_label_parameters will make this 0 */

	if (angle < 0.0) angle += 360.0;

	set_angle = ((!C->common.R.oblique && !(GMT_IS_AZIMUTHAL(C) || GMT_IS_CONICAL(C))) || C->common.R.oblique);
	if (!C->common.R.oblique && (C->current.proj.projection == GMT_GENPER || C->current.proj.projection == GMT_GNOMONIC || C->current.proj.projection == GMT_POLYCONIC)) set_angle = TRUE;
	if (set_angle) {
		if (side == 0 && angle < 180.0) angle -= 180.0;
		if (side == 1 && (angle > 90.0 && angle < 270.0)) angle -= 180.0;
		if (side == 2 && angle > 180.0) angle -= 180.0;
		if (side == 3 && (angle < 90.0 || angle > 270.0)) angle -= 180.0;
	}

	if (!gmt_get_label_parameters (C, side, angle, type, text_angle, justify)) return -1;
	*line_angle = angle;
	if (C->current.setting.map_annot_oblique & 16) *line_angle = (side - 1) * 90.0;

	if (!set_angle) *justify = gmt_polar_adjust (C, side, angle, x, y);
	if (set_angle && !C->common.R.oblique && C->current.proj.projection == GMT_GNOMONIC) {
		/* Fix until we write something that works for everything.  This is a global gnomonic map
		 * so it is easy to fix the angles.  We get correct justify and make sure
		 * the line_angle points away from the boundary */

		angle = fmod (2.0 * angle, 360.0) / 2.0;	/* 0-180 range */
		if (angle > 90.0) angle -= 180.0;
		*justify = gmt_gnomonic_adjust (C, angle, x, y);
		if (*justify == 7) *line_angle += 180.0;
	}

	return 0;
}

void GMT_get_annot_label (struct GMT_CTRL *C, double val, char *label, GMT_LONG do_minutes, GMT_LONG do_seconds, GMT_LONG lonlat, GMT_LONG worldmap)
/* val:		Degree value of annotation */
/* label:	String to hold the final annotation */
/* do_minutes:	TRUE if degree and minutes are desired, FALSE for just integer degrees */
/* do_seconds:	TRUE if degree, minutes, and seconds are desired */
/* lonlat:	0 = longitudes, 1 = latitudes, 2 non-geographical data passed */
/* worldmap:	T/F, whatever C->current.map.is_world is */
{
	GMT_LONG k, n_items, sign, d, m, s, m_sec, level, type, h_pos = 0, zero_fix = FALSE;
	char hemi[3], format[GMT_TEXT_LEN64];

	/* Must override do_minutes and/or do_seconds if format uses decimal notation for that item */

	if (C->current.plot.calclock.geo.order[1] == -1) do_minutes = FALSE;
	if (C->current.plot.calclock.geo.order[2] == -1) do_seconds = FALSE;
	for (k = n_items = 0; k < 3; k++) if (C->current.plot.calclock.geo.order[k] >= 0) n_items++;	/* How many of d, m, and s are requested as integers */

	if (lonlat == 0) {	/* Fix longitudes range first */
		GMT_lon_range_adjust (C->current.plot.calclock.geo.range, &val);
	}

	if (lonlat < 2) {	/* i.e., for geographical data */
		if (GMT_IS_ZERO (val - 360.0) && !worldmap) val = 0.0;
		if (GMT_IS_ZERO (val - 360.0) && worldmap && C->current.proj.projection == GMT_OBLIQUE_MERC) val = 0.0;
	}

	GMT_memset (hemi, 3, char);
	if (C->current.plot.calclock.geo.wesn) {
		if (C->current.plot.calclock.geo.wesn == 2) hemi[h_pos++] = ' ';
		if (lonlat == 0) {
			switch (C->current.plot.calclock.geo.range) {
				case GMT_IS_0_TO_P360_RANGE:
				case GMT_IS_0_TO_P360:
					hemi[h_pos] = (GMT_IS_ZERO (val)) ? 0 : 'E';
					break;
				case GMT_IS_M360_TO_0_RANGE:
				case GMT_IS_M360_TO_0:
					hemi[h_pos] = (GMT_IS_ZERO (val)) ? 0 : 'W';
					break;
				default:
					hemi[h_pos] = (GMT_IS_ZERO (val) || GMT_IS_ZERO (val - 180.0) || GMT_IS_ZERO (val + 180.0)) ? 0 : ((val < 0.0) ? 'W' : 'E');
					break;
			}
		}
		else
			hemi[h_pos] = (GMT_IS_ZERO (val)) ? 0 : ((val < 0.0) ? 'S' : 'N');
		val = fabs (val);
		if (hemi[h_pos] == 0) hemi[0] = 0;	/* No space if no hemisphere letter */
	}
	if (C->current.plot.calclock.geo.no_sign) val = fabs (val);
	sign = (val < 0.0) ? -1 : 1;

	level = do_minutes + do_seconds;		/* 0, 1, or 2 */
	type = (C->current.plot.calclock.geo.n_sec_decimals > 0) ? 1 : 0;

	if (C->current.plot.r_theta_annot && lonlat) {	/* Special check for the r in r-theta (set in )*/
		sprintf (format, "%s", C->current.setting.format_float_out);
		sprintf (label, format, val);
	}
	else if (C->current.plot.calclock.geo.decimal)
		sprintf (label, C->current.plot.calclock.geo.x_format, val, hemi);
	else {
		(void) GMT_geo_to_dms (val, n_items, C->current.plot.calclock.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
		if (d == 0 && sign == -1) {	/* Must write out -0 degrees, do so by writing -1 and change 1 to 0 */
			d = -1;
			zero_fix = TRUE;
		}
		switch (2*level+type) {
			case 0:
				sprintf (label, C->current.plot.format[level][type], d, hemi);
				break;
			case 1:
				sprintf (label, C->current.plot.format[level][type], d, m_sec, hemi);
				break;
			case 2:
				sprintf (label, C->current.plot.format[level][type], d, m, hemi);
				break;
			case 3:
				sprintf (label, C->current.plot.format[level][type], d, m, m_sec, hemi);
				break;
			case 4:
				sprintf (label, C->current.plot.format[level][type], d, m, s, hemi);
				break;
			case 5:
				sprintf (label, C->current.plot.format[level][type], d, m, s, m_sec, hemi);
				break;
		}
		if (zero_fix) label[1] = '0';	/* Undo the fix above */
	}

	return;
}

double gmt_get_angle (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2)
{
	double x1, y1, x2, y2, dx, dy, angle, direction;

	GMT_geo_to_xy (C, lon1, lat1, &x1, &y1);
	GMT_geo_to_xy (C, lon2, lat2, &x2, &y2);
	dx = x2 - x1;
	dy = y2 - y1;
	if (GMT_IS_ZERO (dy) && GMT_IS_ZERO (dx)) {	/* Special case that only(?) occurs at N or S pole or r=0 for GMT_POLAR */
		if (fabs (fmod (lon1 - C->common.R.wesn[XLO] + 360.0, 360.0)) > fabs (fmod (lon1 - C->common.R.wesn[XHI] + 360.0, 360.0))) {	/* East */
			GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO], &x1, &y1);
			GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &x2, &y2);
			C->current.map.corner = 1;
		}
		else {
			GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &x1, &y1);
			GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YHI], &x2, &y2);
			C->current.map.corner = 3;
		}
		angle = d_atan2d (y2-y1, x2-x1) - 90.0;
		if (C->current.proj.got_azimuths) angle += 180.0;
	}
	else
		angle = d_atan2d (dy, dx);

	if (GMT_abs (C->current.map.prev_x_status) == 2 && GMT_abs (C->current.map.prev_y_status) == 2)	/* Last point outside */
		direction = angle + 180.0;
	else if (C->current.map.prev_x_status == 0 && C->current.map.prev_y_status == 0)		/* Last point inside */
		direction = angle;
	else {
		if (GMT_abs (C->current.map.this_x_status) == 2 && GMT_abs (C->current.map.this_y_status) == 2)	/* This point outside */
			direction = angle;
		else if (C->current.map.this_x_status == 0 && C->current.map.this_y_status == 0)		/* This point inside */
			direction = angle + 180.0;
		else {	/* Special case of corners and sides only */
			if (C->current.map.prev_x_status == C->current.map.this_x_status)
				direction = (C->current.map.prev_y_status == 0) ? angle : angle + 180.0;
			else if (C->current.map.prev_y_status == C->current.map.this_y_status)
				direction = (C->current.map.prev_x_status == 0) ? angle : angle + 180.0;
			else
				direction = angle;

		}
	}

	if (direction < 0.0) direction += 360.0;
	if (direction >= 360.0) direction -= 360.0;
	return (direction);
}

double GMT_get_annot_offset (struct GMT_CTRL *C, GMT_LONG *flip, GMT_LONG level)
{
	/* Return offset in inches for text annotation.  If annotation
	 * is to be placed 'inside' the map, set flip to TRUE */

	double a = C->current.setting.map_annot_offset[level];
	if (a >= 0.0) {	/* Outside annotation */
		double dist = C->current.setting.map_tick_length[0];	/* Length of tickmark (could be negative) */
		/* For fancy frame we must consider that the frame width might exceed the ticklength */
		if (C->current.setting.map_frame_type & GMT_IS_FANCY && C->current.setting.map_frame_width > dist) dist = C->current.setting.map_frame_width;
		if (dist > 0.0) a += dist;
		*flip = FALSE;
	}
	else {		/* Inside annotation */
		if (C->current.setting.map_tick_length[0] < 0.0) a += C->current.setting.map_tick_length[0];
		*flip = TRUE;
	}

	return (a);
}

GMT_LONG GMT_flip_justify (struct GMT_CTRL *C, GMT_LONG justify)
{
	/* Return the opposite justification */

	GMT_LONG j;

	switch (justify) {
		case 2:
			j = 10;
			break;
		case 5:
			j = 7;
			break;
		case 7:
			j = 5;
			break;
		case 10:
			j = 2;
			break;
		default:
			j = justify;
			GMT_report (C, GMT_MSG_FATAL, "GMT_flip_justify called with incorrect argument (%ld)\n", j);
			break;
	}

	return (j);
}

GMT_LONG GMT_init_custom_symbol (struct GMT_CTRL *C, char *name, struct GMT_CUSTOM_SYMBOL **S) {
	GMT_LONG k, nc = 0, last, error = 0, do_fill, do_pen, first = TRUE;
	char file[GMT_BUFSIZ], buffer[GMT_BUFSIZ], col[8][GMT_TEXT_LEN64], var[8], OP[8], constant[GMT_TEXT_LEN64];
	char *fill_p = NULL, *pen_p = NULL;
	FILE *fp = NULL;
	struct GMT_CUSTOM_SYMBOL *head = NULL;
	struct GMT_CUSTOM_SYMBOL_ITEM *s = NULL, *previous = NULL;
#ifdef PS_MACRO
	struct GMT_STAT buf;
#endif

	/* Parse the *.def files.  Note: PS_MACRO is off and will be worked on later.  For now the
	 * extended macro language works well and can handle most situations. PW 10/11/10 */

	GMT_getsharepath (C, "custom", name, ".def", file);
#ifdef PS_MACRO
	if (GMT_STAT (file, &buf)) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Could not find custom symbol %s\n", name);
		GMT_exit (EXIT_FAILURE);
	}
#endif
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Could not find custom symbol %s\n", name);
		GMT_exit (EXIT_FAILURE);
	}

	head = GMT_memory (C, NULL, 1, struct GMT_CUSTOM_SYMBOL);
	strcpy (head->name, name);
	while (fgets (buffer, GMT_BUFSIZ, fp)) {
#ifdef PS_MACRO
		if (head->PS) {	/* Working on a PS symbol, just append the text as is */
			strcat (head->PS_macro, buffer);
			continue;
		}
#endif
		if (buffer[0] == '#' || buffer[0] == '\n' || buffer[0] == '\r') continue;	/* Skip comments or blank lines */
		if (buffer[0] == 'N' && buffer[1] == ':') {	/* Got extra parameter specs. This is # of data columns expected beyond the x,y stuff */
			head->n_required = atoi (&buffer[2]);
			continue;
		}
#ifdef PS_MACRO
		if (buffer[0] == '/') {	/* This is the initial definition of a PSL symbol */
			head->PS = TRUE;
			head->PS_macro = GMT_memory (C, NULL, (GMT_LONG)buf.st_size, char);
			continue;
		}
#endif
		s = GMT_memory (C, NULL, 1, struct GMT_CUSTOM_SYMBOL_ITEM);
		if (first) head->first = s;
		first = FALSE;

		if (strstr (buffer, "if $")) {	/* Parse a logical if-test or elseif here */
			if (strstr (buffer, "} elseif $")) {	/* Actually, it is an elseif-branch [skip { elseif]; nc -=3 means we count the cols only */
				nc = sscanf (buffer, "%*s %*s %s %s %s %*s %s %s %s %s %s %s %s %s", var, OP, constant, col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7]) - 3;
				s->conditional = 8;	/* elseif test */
			}
			else {	/* Starting if-branch [skip if] */
				nc = sscanf (buffer, "%*s %s %s %s %*s %s %s %s %s %s %s %s %s", var, OP, constant, col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7]) - 3;
				s->conditional = (col[nc-1][0] == '{') ? 2 : 1;		/* If block (2) or single command (1) */
			}
			s->var = atoi (&var[1]);				/* Get the i in $i, i.e., if $1 then return 1 since $0 is used for symbol scale */
			for (k = 0; constant[k]; k++) if (constant[k] == ':') constant[k] = ' ';	/* Remove any colon in case constant has two range values */
			sscanf (constant, "%lf %lf", &s->const_val[0], &s->const_val[1]);	/* Get one [or two] constants */
			k = 0;
			if (OP[0] == '!') s->negate = TRUE, k = 1;	/* Reverse any test that follows */
			s->operator = OP[k];	/* Use a 1-char code for operator (<, =, >, etc.); modify below where ambiguous */
			switch (OP[k]) {
				case '<':	/* Less than [Default] */
				 	if (OP[k+1] == '=')	/* Let <= be called operator L */
						s->operator = 'L';
					else if (OP[k+1] == '>')	/* Let <> be called operator i (exclusive in-range not including the boundaries) */
						s->operator = 'i';
					else if (OP[k+1] == ']')	/* Let <] be called operator r (in-range but only include right boundary) */
						s->operator = 'r';
					break;
				case '[':	/* Inclusive range */
				 	if (OP[k+1] == ']')	/* Let [] be called operator I (inclusive range) */
						s->operator = 'I';
					else if (OP[k+1] == '>')	/* Let [> be called operator l (in range but only include left boundary) */
						s->operator = 'l';
					break;
				case '>':	/* Greater than [Default] */
				 	if (OP[k+1] == '=') s->operator = 'G';	/* Let >= be called operator G */
					break;
				case '=':	/* Equal [Default] */
					if (GMT_is_dnan (s->const_val[0])) s->operator = 'E';	/* Let var == NaN be called operator E (equals NaN) */
					break;
			}
		}
		else if (strstr (buffer, "} else {"))	/* End of if-block and start of else-block */
			s->conditional = 6;
		else if (strchr (buffer, '}'))		/* End of if-block */
			s->conditional = 4;
		else					/* Regular command, no conditional test prepended */
			nc = sscanf (buffer, "%s %s %s %s %s %s %s %s", col[0], col[1], col[2], col[3], col[4], col[5], col[6], col[7]);

		last = nc - 1;	/* Index of last col argument */

		/* Check if command has any trailing -Gfill -Wpen specifiers */

		do_fill = do_pen = FALSE;
		if (col[last][0] == '-' && col[last][1] == 'G') fill_p = &col[last][2], do_fill = TRUE, last--;
		if (col[last][0] == '-' && col[last][1] == 'W') pen_p = &col[last][2], do_pen = TRUE, last--;
		if (col[last][0] == '-' && col[last][1] == 'G') fill_p = &col[last][2], do_fill = TRUE, last--;	/* Check again for -G since perhaps -G -W was given */
		if (last < 0) error++;

		s->action = (GMT_LONG)col[last][0];
		if (s->conditional == 4) s->action = '}';	/* The {, }, E, and F are dummy actions */
		if (s->conditional == 6) s->action = 'E';
		if (s->conditional == 8) s->action = 'F';
		if (last >= 2 && !s->conditional) {	/* OK to parse x,y coordinates */
			s->x = atof (col[GMT_X]);
			s->y = atof (col[GMT_Y]);
		}

		switch (s->action) {

			/* M, D, S, and A allows for arbitrary lines or polygons to be designed - these may be painted or filled with pattern */

			case 'M':		/* Set a new anchor point */
				if (last != 2) error++;
				break;

			case 'D':		/* Draw to next point */
				if (last != 2) error++;
				break;

			case 'S':		/* Stroke current path as line, not polygon */
				if (last != 0) error++;
				break;

			case 'A':		/* Draw arc of a circle */
				if (last != 5) error++;
				s->p[0] = atof (col[2]);
				s->p[1] = atof (col[3]);
				s->p[2] = atof (col[4]);
				break;

			case 'R':		/* Rotate coordinate system about (0,0) */
				if (last != 1) error++;
				if (col[0][0] == '$') {	/* Got a variable as angle */
					s->var = atoi (&col[0][1]);
					s->action = GMT_SYMBOL_VARROTATE;	/* Mark as a different rotate action */
				}
				else	/* Got a fixed angle */
					s->p[0] = atof (col[0]);
				break;

			case 'T':		/* Texture changes only (modify pen, fill settings) */
				if (last != 0) error++;
				break;

			case '{':		/* Start of block */
			case '}':		/* End of block */
			case 'E':		/* End of if-block and start of else block */
			case 'F':		/* End of if-block and start of another if block */
				break;	/* Nothing to do but move on */

			/* These are standard psxy-type symbols */

			case 'a':		/* Draw star symbol */
			case 'c':		/* Draw complete circle */
			case 'd':		/* Draw diamond symbol */
			case 'g':		/* Draw octagon symbol */
			case 'h':		/* Draw hexagon symbol */
			case 'i':		/* Draw inverted triangle symbol */
			case 'n':		/* Draw pentagon symbol */
			case 'p':		/* Draw solid dot */
			case 's':		/* Draw square symbol */
			case 't':		/* Draw triangle symbol */
			case 'x':		/* Draw cross symbol */
			case 'y':		/* Draw vertical dash symbol */
			case '+':		/* Draw plus symbol */
			case '-':		/* Draw horizontal dash symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				break;

			case 'l':		/* Draw letter/text symbol */
				if (last != 4) error++;
				s->p[0] = atof (col[2]);
				s->string = GMT_memory (C, NULL, strlen (col[3]) + 1, char);
				strcpy (s->string, col[3]);
				break;

			case 'r':		/* Draw rect symbol */
				if (last != 4) error++;
				s->p[0] = atof (col[2]);
				s->p[1] = atof (col[3]);
				break;

			case 'e':		/* Draw ellipse symbol */
			case 'j':		/* Draw rotated rect symbol */
			case 'w':		/* Draw wedge (pie) symbol */
				if (last != 5) error++;
				s->p[0] = atof (col[2]);	/* Leave direction in degrees */
				s->p[1] = atof (col[3]);
				s->p[2] = atof (col[4]);
				break;

			default:
				error++;
				break;
		}

		if (error) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Failed to parse symbol commands in file %s\n", file);
			GMT_report (C, GMT_MSG_FATAL, "Error: Offending line: %s\n", buffer);
			GMT_exit (EXIT_FAILURE);
		}

		if (do_fill) {	/* Update current fill */
			s->fill = GMT_memory (C, NULL, 1, struct GMT_FILL);
			if (fill_p[0] == '-')	/* Do not want to fill this polygon */
				s->fill->rgb[0] = -1;
			else if (GMT_getfill (C, fill_p, s->fill)) {
				GMT_fill_syntax (C, 'G', " ");
				GMT_exit (EXIT_FAILURE);
			}
		}
		else
			s->fill = NULL;
		if (do_pen) {	/* Update current pen */
			s->pen = GMT_memory (C, NULL, 1, struct GMT_PEN);
			if (pen_p[0] == '-')	/* Do not want to draw outline */
				s->pen->rgb[0] = -1;
			else if (GMT_getpen (C, pen_p, s->pen)) {
				GMT_pen_syntax (C, 'W', " ");
				GMT_exit (EXIT_FAILURE);
			}
		}
		else
			s->pen = NULL;

		GMT_report (C, GMT_MSG_DEBUG, "Code %c Conditional = %ld, OP = %c negate = %ld var = %ld do_pen = %ld do_fill = %ld\n", \
			(int)s->action, s->conditional, (int)s->operator, s->negate, s->var, do_pen, do_fill);
		if (previous) previous->next = s;
		previous = s;
	}
	fclose (fp);
	*S = head;
	return (GMT_NOERROR);
}

struct GMT_CUSTOM_SYMBOL * GMT_get_custom_symbol (struct GMT_CTRL *C, char *name) {
	GMT_LONG i, found = -1;

	/* First see if we already have loaded this symbol */

	for (i = 0; found == -1 && i < C->init.n_custom_symbols; i++) if (!strcmp (name, C->init.custom_symbol[i]->name)) found = i;

	if (found >= 0) return (C->init.custom_symbol[found]);	/* Return a previously loaded symbol */

	/* Must load new symbol */

	C->init.custom_symbol = GMT_memory (C, C->init.custom_symbol, C->init.n_custom_symbols+1, struct GMT_CUSTOM_SYMBOL *);
	GMT_init_custom_symbol (C, name, &(C->init.custom_symbol[C->init.n_custom_symbols]));

	return (C->init.custom_symbol[C->init.n_custom_symbols++]);
}

void GMT_free_custom_symbols (struct GMT_CTRL *C) {	/* Free the allocated list of custom symbols */
	GMT_LONG i;
	struct GMT_CUSTOM_SYMBOL_ITEM *s = NULL, *current = NULL;

	if (C->init.n_custom_symbols > 0) {
		for (i = 0; i < C->init.n_custom_symbols; i++) {
			s = C->init.custom_symbol[i]->first;
			while (s) {
				current = s;
				s = s->next;
				GMT_free (C, current->fill);
				GMT_free (C, current->pen);
				GMT_free (C, current->string);
				GMT_free (C, current);
			}
			GMT_free (C, C->init.custom_symbol[i]->PS_macro);
			GMT_free (C, C->init.custom_symbol[i]);
		}
		GMT_free (C, C->init.custom_symbol);
		C->init.n_custom_symbols = 0;
	}
}

GMT_LONG GMT_polygon_is_open (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n)
{	/* Returns TRUE if the first and last point is not identical */
	if (n < 2) return FALSE;	/*	A point is closed */
	if (!GMT_IS_ZERO (x[0] - x[n-1])) return TRUE;	/* x difference exceeds threshold */
	if (!GMT_IS_ZERO (y[0] - y[n-1])) return TRUE;	/* y difference exceeds threshold */
	/* Here, first and last are ~identical - to be safe we enforce exact closure */
	x[n-1] = x[0];	y[n-1] = y[0];
	return FALSE;
}

double GMT_polygon_area (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n)
{
	GMT_LONG i, last;
	double area, xold, yold;

	/* Sign will be +ve if polygon is CW, negative if CCW */

	last = (GMT_polygon_is_open (C, x, y, n)) ? n : n - 1;	/* Skip repeating vertex */

	area = yold = 0.0;
	xold = x[last-1];
	yold = y[last-1];

	for (i = 0; i < last; i++) {
		area += (xold - x[i]) * (yold + y[i]);
		xold = x[i];
		yold = y[i];
	}
	return (0.5 * area);
}

GMT_LONG GMT_polygon_centroid (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double *Cx, double *Cy)
{	/* Compute polygon centroid location */
	GMT_LONG i, last;
	double A, d, xold, yold;

	A = GMT_polygon_area (C, x, y, n);
	last = (GMT_polygon_is_open (C, x, y, n)) ? n : n - 1;	/* Skip repeating vertex */
	*Cx = *Cy = 0.0;
	xold = x[last-1];	yold = y[last-1];
	for (i = 0; i < last; i++) {
		d = (xold * y[i] - x[i] * yold);
		*Cx += (x[i] + xold) * d;
		*Cy += (y[i] + yold) * d;
		xold = x[i];	yold = y[i];
	}
	*Cx /= (6.0 * A);
	*Cy /= (6.0 * A);
	return ((A < 0.0) ? -1 : +1);	/* -1 means CCW, +1 means CW */
}

GMT_LONG * GMT_prep_nodesearch (struct GMT_CTRL *GMT, struct GMT_GRID *G, double radius, GMT_LONG mode, GMT_LONG *d_row, GMT_LONG *actual_max_d_col)
{
	/* When we search all nodes within a radius R on a grid, we first rule out nodes that are
	 * outside the circumscribing rectangle.  However, for geographic data the circle becomes
	 * elliptical in lon/lat space due to the cos(lat) shrinking of the length of delta_lon.
	 * Thus, while the +- width of nodes in y is fixed (d_row), the +-width of nodes in x
	 * is a function of latitude.  This is the array d_col (which is constant for Cartesian data).
	 * We expect GMT_init_distaz has been called so the GMT_distance function returns distances
	 * in the same units as the radius.  We also return the widest value in the d_col array via
	 * the actual_max_d_col value.
	 */
	GMT_LONG max_d_col, row, *d_col = GMT_memory (GMT, NULL, G->header->ny, GMT_LONG);
	double dist_x, dist_y, lon, lat;
	
	lon = G->header->wesn[XLO] + G->header->inc[GMT_X];

	dist_y = GMT_distance (GMT, G->header->wesn[XLO], G->header->wesn[YLO], G->header->wesn[XLO], G->header->wesn[YLO] + G->header->inc[GMT_Y]);
	if (mode) {	/* Input data is geographical, so circle widens with latitude due to cos(lat) effect */
		max_d_col = (GMT_LONG) (ceil (G->header->nx / 2.0) + 0.1);	/* Upper limit on +- halfwidth */
		*actual_max_d_col = 0;
		for (row = 0; row < G->header->ny; row++) {
			lat = GMT_grd_row_to_y (GMT, row, G->header);
			/* Determine longitudinal width of one grid ell at this latitude */
			dist_x = GMT_distance (GMT, G->header->wesn[XLO], lat, lon, lat);
			d_col[row] = (fabs (lat) == 90.0) ? max_d_col : (GMT_LONG)(ceil (radius / dist_x) + 0.1);
			if (d_col[row] > max_d_col) d_col[row] = max_d_col;	/* Safety valve */
			if (d_col[row] > (*actual_max_d_col)) *actual_max_d_col = d_col[row];
		}
	}
	else {	/* Plain Cartesian data with rectangular box */
		dist_x = GMT_distance (GMT, G->header->wesn[XLO], G->header->wesn[YLO], lon, G->header->wesn[YLO]);
		*actual_max_d_col = max_d_col = (GMT_LONG) (ceil (radius / dist_x) + 0.1);
		for (row = 0; row < G->header->ny; row++) d_col[row] = max_d_col;
	}
	*d_row = (GMT_LONG) (ceil (radius / dist_y) + 0.1);	/* The constant half-width of nodes in y-direction */
	GMT_report (GMT, GMT_MSG_VERBOSE, "Max node-search half-widths are: half_x = %ld, half_y = %ld\n", *d_row, *actual_max_d_col);
	return (d_col);		/* The (possibly variable) half-width of nodes in x-direction as function of y */
}

/* THese three functions are used by grdmath and gmtmath only */

GMT_LONG gmt_load_macros (struct GMT_CTRL *GMT, char *mtype, struct MATH_MACRO **M)
{
	/* Load in any gmt/grdmath macros.  These records are of the format
	 * MACRO = ARG1 ARG2 ... ARGN : comments on what they do */

	GMT_LONG n = 0, k = 0, n_alloc = 0, pos = 0;
	char line[GMT_BUFSIZ], name[GMT_TEXT_LEN64], item[GMT_TEXT_LEN64], args[GMT_BUFSIZ];
	struct MATH_MACRO *macro = NULL;
	FILE *fp = NULL;

	if (!GMT_getuserpath (GMT, mtype, line)) return (0);

	if ((fp = fopen (line, "r")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Unable to open %s macro file\n", line);
		return -1;
	}

	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#') continue;
		GMT_chop (GMT, line);
		sscanf (line, "%s = %[^:]", name, args);
		if (n == n_alloc) macro = GMT_memory (GMT, macro, n_alloc += GMT_TINY_CHUNK, struct MATH_MACRO);
		macro[n].name = strdup (name);
		pos = 0;
		while (GMT_strtok (GMT, args, " \t", &pos, item)) macro[n].n_arg++;		/* Count the arguments */
		macro[n].arg = GMT_memory (GMT, macro[n].arg, macro[n].n_arg, char *);	/* Allocate pointers for args */
		pos = k = 0;
		while (GMT_strtok (GMT, args, " \t", &pos, item)) macro[n].arg[k++] = strdup (item);	/* Assign arguments */
		n++;
	}
	fclose (fp);
	if (n < n_alloc) macro = GMT_memory (GMT, macro, n, struct MATH_MACRO);

	*M = macro;
	return (n);
}

GMT_LONG gmt_find_macro (char *arg, GMT_LONG n_macros, struct MATH_MACRO *M)
{
	/* See if the arg matches the name of a macro; return its ID or -1 */

	GMT_LONG n;

	if (n_macros == 0 || !M) return (GMTAPI_NOTSET);

	for (n = 0; n < n_macros; n++) if (!strcmp (arg, M[n].name)) return (n);

	return (GMTAPI_NOTSET);
}

void gmt_free_macros (struct GMT_CTRL *GMT, GMT_LONG n_macros, struct MATH_MACRO **M)
{
	/* Free space allocated for macros */

	GMT_LONG n, k;

	if (n_macros == 0 || !(*M)) return;

	for (n = 0; n < n_macros; n++) {
		free ((*M)[n].name);
		for (k = 0; k < (*M)[n].n_arg; k++) free ((*M)[n].arg[k]);	/* Free arguments */
		GMT_free (GMT, (*M)[n].arg);	/* Free argument list */
	}
	GMT_free (GMT, (*M));
}

#ifdef DEBUG
/*
 * To monitor memory usage in GMT and to make sure things are freed properly.
 * Only included in the compilation if -DDEBUG is passed to the compiler.
 * Some programs use the GMT_memtrack_on|off since they do lots of memory
 * allocation and this bogs down the tracking machinery so the program will
 * run very very slowly.
 *
 * Paul Wessel, Feb 2008.
*/

void GMT_memtrack_on (struct GMT_CTRL *C, struct MEMORY_TRACKER *M)
{	/* Turns memory tracking ON */
	M->active = TRUE;
}

void GMT_memtrack_off (struct GMT_CTRL *C, struct MEMORY_TRACKER *M)
{	/* Turns memory tracking OFF */
	M->active = FALSE;
}

double GMT_memtrack_mem (struct GMT_CTRL *C, GMT_LONG mem, GMT_LONG *unit)
{	/* Report the memory in the chosen unit */
	GMT_LONG k = 0;
	double val = mem / 1024.0;	/* Kb */
	if (val > 1024.0) {val /= 1024.0; k++;}	/* Now in Mb */
	if (val > 1024.0) {val /= 1024.0; k++;}	/* Now in Gb */
	*unit = k;
	return (val);
}

#ifndef NEW_DEBUG
void GMT_memtrack_init (struct GMT_CTRL *C, struct MEMORY_TRACKER **M) {	/* Called in GMT_begin() */
	/* Create the memory tracker structure and initialize it */
	struct MEMORY_TRACKER *P = NULL;
	char *c = NULL;
	P = calloc (1, sizeof (struct MEMORY_TRACKER));
	P->n_alloc = GMT_CHUNK;
	P->item = calloc ((size_t)P->n_alloc, sizeof(struct MEMORY_ITEM));
	P->active = ((c = getenv ("GMT_MEM")) == CNULL);
	*M = P;
}

void gmt_memtrack_alloc (struct GMT_CTRL *C, struct MEMORY_TRACKER *M)
{	/* Increase available memory for memory tracking */
	M->n_alloc += GMT_CHUNK;
	M->item = realloc (M->item, (size_t)(M->n_alloc * sizeof(struct MEMORY_ITEM)));
}

void gmt_memtrack_add (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr, void *prev_ptr, GMT_LONG size) {
	/* Called from GMT_memory to update current list of memory allocated */
	GMT_LONG entry, old;
	void *use = NULL;

	if (!M) return;		/* Not initialized */
	if (!M->active) return;	/* Not activated */
	use = (prev_ptr) ? prev_ptr : ptr;
	entry = gmt_memtrack_find (C, M, use);
	if (entry == -1) {	/* Not found, must insert new entry at end of list */
		entry = M->n_ptr;	/* Position of this new entry as last item */
		if (entry == M->n_alloc) gmt_memtrack_alloc (C, M);	/* Must update our memory arrays */
		M->item[entry].ptr = ptr;	/* Store the pointer */
		strncpy (M->item[entry].name, name, (size_t)(MEM_TXT_LEN-1));
		M->item[entry].name[MEM_TXT_LEN-1] = 0;
		M->item[entry].line = line;
		old = 0;
		M->n_ptr++;
		M->n_allocated++;
	}
	else {	/* Found existing pointer, get its previous size */
		old = M->item[entry].size;
		M->item[entry].ptr = ptr;	/* Since realloc could have changed it */
		M->n_reallocated++;
	}
	M->current += (size - old);	/* Revised memory tally */
	if (M->current < 0) {
		GMT_report (C, GMT_MSG_FATAL, "Memory tracker reports < 0 bytes allocated!\n");
	}
	M->item[entry].size = size;
	if (M->current > M->maximum) M->maximum = M->current;	/* Update total allocation */
	if (size > M->largest) M->largest = size;		/* Update largest single item */
}

void gmt_memtrack_sub (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr) {
	/* Called from GMT_free to remove memory pointer */
	GMT_LONG entry;

	if (!M) return;		/* Not initialized */
	if (!M->active) return;	/* Not activated */
	entry = gmt_memtrack_find (C, M, ptr);
	if (entry == -1) {	/* Error, trying to free something not allocated by GMT_memory */
		GMT_report (C, GMT_MSG_FATAL, "Wrongly tries to free item in %s, line %ld\n", name, line);
		return;
	}
	M->current -= M->item[entry].size;	/* "Free" the memory */
	entry++;
	while (entry < M->n_ptr) {		/* For the rest of the array we shuffle one down */
		M->item[entry-1] = M->item[entry];
		entry++;
	}
	M->n_ptr--;
	M->n_freed++;
	if (M->current < 0) GMT_report (C, GMT_MSG_FATAL, "Memory tracker reports < 0 bytes allocated!\n");
}

GMT_LONG gmt_memtrack_find (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, void *ptr) {
	/* Brute force linear search for now - if useful we'll do linked lists or something */
	GMT_LONG i = 0, go = TRUE;
	while (go && i < M->n_ptr) {	/* Loop over array of known pointers */
		if (ptr == M->item[i].ptr)
			go = FALSE;
		else
			i++;
	}
	return (go ? -1 : i);
}

void GMT_memtrack_report (struct GMT_CTRL *C, struct MEMORY_TRACKER *M) {	/* Called at end of GMT_end() */
	GMT_LONG k, u, excess, report;
	char *unit[3] = {"kb", "Mb", "Gb"};
	double tot;

	if (!M) return;		/* Not initialized */
	if (!M->active) return;	/* Not activated */
	report = (M->current > 0);
	if (report) {
		tot = GMT_memtrack_mem (C, M->maximum, &u);
		GMT_report (C, GMT_MSG_FATAL, "Max total memory allocated was %.3f %s [%ld bytes]\n", tot, unit[u], M->maximum);
		tot = GMT_memtrack_mem (C, M->largest, &u);
		GMT_report (C, GMT_MSG_FATAL, "Single largest allocation was %.3f %s [%ld bytes]\n", tot, unit[u], M->largest);
		tot = GMT_memtrack_mem (C, M->current, &u);
		if (M->current) GMT_report (C, GMT_MSG_FATAL, "MEMORY NOT FREED: %.3f %s [%ld bytes]\n", tot, unit[u], M->current);
		GMT_report (C, GMT_MSG_FATAL, "Items allocated: %ld reallocated: %ld Freed: %ld\n", M->n_allocated, M->n_reallocated, M->n_freed);
		excess = M->n_allocated - M->n_freed;
		if (excess) GMT_report (C, GMT_MSG_FATAL, "Items not properly freed: %ld\n", excess);
		for (k = 0; k < M->n_ptr; k++) {
			tot = GMT_memtrack_mem (C, M->item[k].size, &u);
			GMT_report (C, GMT_MSG_FATAL, "Memory not freed first allocated in %s, line %ld is %.3f %s [%ld bytes]\n", M->item[k].name, M->item[k].line, tot, unit[u], M->item[k].size);
		}
	}

	free (M->item);
	free (M);
}
#else
/* Binary tree manipulation, modified after Sedgewick's Algorithms in C */
/* 2001-07-03 PW: Not working it seems - left here in case we have time to fix later */

void GMT_memtrack_init (struct GMT_CTRL *C, struct MEMORY_TRACKER **M) {	/* Called in GMT_begin() */
	struct MEMORY_TRACKER *P = NULL;
	char *c = NULL;
	P = calloc (1, sizeof (struct MEMORY_TRACKER));
	P->active = ((c = getenv ("GMT_MEM")) == CNULL);
	P->search = TRUE;
	P->list_tail = calloc (1, sizeof *P->list_tail);
	P->list_tail->l = P->list_tail;	P->list_tail->r = P->list_tail;
	P->list_head = calloc (1, sizeof *P->list_head);
	P->list_head->r = P->list_tail;
	P->list_head->l = NULL;
	*M = P;
}

struct MEMORY_ITEM * gmt_treeinsert (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, void *addr)
{
	struct MEMORY_ITEM *p = NULL, *x = NULL;
	p = M->list_head;	x = M->list_head->r;
	while (x != M->list_tail) {
		p = x;
		x = (addr < x->ptr) ? x->l : x->r;
	}
	x = calloc (1, sizeof (struct MEMORY_ITEM));
	x->ptr = addr;
	x->l = M->list_tail;	x->r = M->list_tail;
	if (x->ptr < p->ptr) p->l = x; else p->r = x;
	return (x);
}

struct MEMORY_ITEM * gmt_memtrack_find (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, void *addr)
{
	struct MEMORY_ITEM *x = NULL;
	M->list_tail->ptr = addr;
	x = M->list_head->r;
	while (x->ptr && addr != x->ptr) {
		x = (addr < x->ptr) ? x->l : x->r;
	}
	M->list_tail->ptr = NULL;
	return ((x->ptr) ? x : NULL);
}

void gmt_memtrack_add (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr, void *prev_ptr, GMT_LONG size) {
	/* Called from GMT_memory to update current list of memory allocated */
	GMT_LONG old;
	void *use = NULL;
	struct MEMORY_ITEM *entry = NULL;

	if (!M) return;		/* Not initialized */
	if (!M->active) return;	/* Not activated */
	use = (prev_ptr) ? prev_ptr : ptr;
	entry = (M->search) ? gmt_memtrack_find (C, M, use) : NULL;
	if (!entry) {	/* Not found, must insert new entry at end */
		entry = gmt_treeinsert (C, M, use);
		entry->line = line;
		entry->name = strdup (name);
		old = 0;
		M->n_ptr++;
		M->n_allocated++;
	}
	else {	/* Found existing pointer, get its previous size */
		old = entry->size;
		entry->ptr = ptr;	/* Since realloc could have changed it */
		M->n_reallocated++;
	}
	M->current += (size - old);
	if (M->current < 0) GMT_report (C, GMT_MSG_FATAL, "Memory tracker reports < 0 bytes allocated!\n");
	entry->size = size;
	if (M->current > M->maximum) M->maximum = M->current;	/* Update total allocation */
	if (size > M->largest) M->largest = size;		/* Update largest single item */
}

void gmt_treedelete (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, void *addr) {
	struct MEMORY_ITEM *c = NULL, *p = NULL, *t = NULL, *x = NULL;
	M->list_tail->ptr = addr;
	p = M->list_head;	x = M->list_head->r;
	while (addr != x->ptr) {
		p = x;
		x = (addr < x->ptr) ? x->l : x->r;
	}
	t = x;
	if (t->r == M->list_tail) x = x->l;
	else if (t->r->l == M->list_tail) {
		x = x->r;	x->l = t->l;
	}
	else {
		c = x->r;
		while (c->l->l != M->list_tail) c = c->l;
		x = c->l;	c->l = x->r;
		x->l = t->l;	x->r = t->r;
	}
	if (t->name) free (t->name);
	free (t);
	if (addr < p->ptr) p->l = x; else p->r = x;
	M->list_tail->ptr = NULL;
}

void gmt_memtrack_sub (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr) {
	/* Called from GMT_free to remove memory pointer */
	struct MEMORY_ITEM *entry = NULL;

	if (!M) return;		/* Not initialized */
	if (!M->active) return;	/* Not activated */
	entry = gmt_memtrack_find (C, M, ptr);
	if (!entry) {	/* Error, trying to free something not allocated by GMT_memory */
		GMT_report (C, GMT_MSG_FATAL, "Wrongly tries to free item in %s, line %ld\n", name, line);
		return;
	}
	M->current -= entry->size;	/* "Free" the memory */
	gmt_treedelete (C, M, entry->ptr);
	M->n_ptr--;
	M->n_freed++;
	if (M->current < 0) GMT_report (C, GMT_MSG_FATAL, "Memory tracker reports < 0 bytes allocated!\n");
}

void gmt_treereport (struct GMT_CTRL *C, struct MEMORY_ITEM *x) {
	GMT_LONG u;
	char *unit[3] = {"kb", "Mb", "Gb"};
	double tot = GMT_memtrack_mem (C, x->size, &u);
	GMT_report (C, GMT_MSG_FATAL, "Memory not freed first allocated in %s, line %ld is %.3f %s [%ld bytes]\n", x->name, x->line, tot, unit[u], x->size);
}

void gmt_treeprint (struct GMT_CTRL *C, struct MEMORY_TRACKER *M, struct MEMORY_ITEM *x)
{
	if (!x) return;
	if (x != M->list_tail) {
		gmt_treeprint (C, M, x->l);
		gmt_treereport (C, x);
		gmt_treeprint (C, M, x->r);
	}
}

void GMT_memtrack_report (struct GMT_CTRL *C, struct MEMORY_TRACKER *M) {	/* Called at end of GMT_end() */
	GMT_LONG u, excess, report;
	double tot;
	char *unit[3] = {"kb", "Mb", "Gb"};

	if (!M) return;		/* Not initialized */
	if (!M->active) return;	/* Not activated */
	report = (M->current > 0);
	if (report) {
		tot = GMT_memtrack_mem (C, M->maximum, &u);
		GMT_report (C, GMT_MSG_FATAL, "Max total memory allocated was %.3f %s [%ld bytes]\n", tot, unit[u], M->maximum);
		tot = GMT_memtrack_mem (C, M->largest, &u);
		GMT_report (C, GMT_MSG_FATAL, "Single largest allocation was %.3f %s [%ld bytes]\n", tot, unit[u], M->largest);
		tot = GMT_memtrack_mem (C, M->current, &u);
		if (M->current) GMT_report (C, GMT_MSG_FATAL, "MEMORY NOT FREED: %.3f %s [%ld bytes]\n", tot, unit[u], M->current);
		GMT_report (C, GMT_MSG_FATAL, "Items allocated: %ld reallocated: %ld Freed: %ld\n", M->n_allocated, M->n_reallocated, M->n_freed);
		excess = M->n_allocated - M->n_freed;
		if (excess) GMT_report (C, GMT_MSG_FATAL, "Items not properly freed: %ld\n", excess);
		gmt_treeprint (C, M, M->list_head->r);
	}

	free (M->list_head);
	free (M->list_tail);
	free (M);
}

#endif

#endif

void gmt_init_rot_matrix (double R[3][3], double E[])
{	/* This starts setting up the matrix without knowing the angle of rotation
	 * Call set_rot_angle with R, and omega to complete the matrix
	 * P	Cartesian 3-D vector of rotation pole
	 * R	the rotation matrix without terms depending on omega
	 * See Cox and Hart [1985] Box ?? for details.
	 */

	R[0][0] = E[0] * E[0];
	R[0][1] = E[0] * E[1];
	R[0][2] = E[0] * E[2];

	R[1][0] = E[0] * E[1];
	R[1][1] = E[1] * E[1];
	R[1][2] = E[1] * E[2];

	R[2][0] = E[0] * E[2];
	R[2][1] = E[1] * E[2];
	R[2][2] = E[2] * E[2];
}

void gmt_load_rot_matrix (double w, double R[3][3], double E[])
{	/* Sets R using R(no_omega) and the given rotation angle w in radians */
	double sin_w, cos_w, c, E_x, E_y, E_z;

	sincos (w, &sin_w, &cos_w);
	c = 1.0 - cos_w;

	E_x = E[0] * sin_w;
	E_y = E[1] * sin_w;
	E_z = E[2] * sin_w;

	R[0][0] = R[0][0] * c + cos_w;
	R[0][1] = R[0][1] * c - E_z;
	R[0][2] = R[0][2] * c + E_y;

	R[1][0] = R[1][0] * c + E_z;
	R[1][1] = R[1][1] * c + cos_w;
	R[1][2] = R[1][2] * c - E_x;

	R[2][0] = R[2][0] * c - E_y;
	R[2][1] = R[2][1] * c + E_x;
	R[2][2] = R[2][2] * c + cos_w;
}

void gmt_matrix_vect_mult (double a[3][3], double b[3], double c[3])
{	/* c = A * b */
	int i, j;

	for (i = 0; i < 3; i++) for (j = 0, c[i] = 0.0; j < 3; j++) c[i] += a[i][j] * b[j];
}

#define SEG_DIST 2
#define SEG_AZIM 3

struct GMT_DATASET * gmt_resample_data_spherical (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double along_ds, GMT_LONG mode, GMT_LONG ex_cols, GMT_LONG smode)
{
	/* Din is a data set with at least two columns (x,y or lon/lat);
	 * it can contain any number of tables and segments with lines.
	 * along_ds is the resampling interval along the traces in Din.
	 * It is in the current distance units (set via GMT_init_distaz).
	 * mode is either 0 (just lon,lat), 1 (lon,lat,dist) or 2 (lon,lat,dist,azim)
	 * ex_cols makes space for this many extra empty data columns [0]
	 * smode sets the sampling mode for GMT_fix_up_path
	 * Dout is the new data set with all the resampled lines;
	 */

	int ndig;
	GMT_LONG tbl, seg, row, col, n_cols, resample, seg_no;
	char buffer[GMT_BUFSIZ], ID[GMT_BUFSIZ];
	double along_dist, azimuth, dist_inc, along_ds_degree;
	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *Tin = NULL, *Tout = NULL;

	resample = (!GMT_IS_ZERO(along_ds));
	n_cols = 2 + mode + ex_cols;
	along_ds_degree = (along_ds / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.DIST_M_PR_DEG;
	D = GMT_alloc_dataset (GMT, Din, n_cols, 0, GMT_ALLOC_NORMAL);	/* Same table length as Din, but with up to 4 columns (lon, lat, dist, az) */
	ndig = irint (floor (log10 ((double)Din->n_segments))) + 1;	/* Determine how many decimals are needed for largest segment id */

	for (tbl = seg_no = 0; tbl < Din->n_tables; tbl++) {
		Tin  = Din->table[tbl];
		Tout = D->table[tbl];
		for (seg = Tout->n_records = 0; seg < Tin->n_segments; seg++, seg_no++) {	/* For each segment to resample */
			GMT_memcpy (Tout->segment[seg]->coord[GMT_X], Tin->segment[seg]->coord[GMT_X], Tin->segment[seg]->n_rows, double);	/* Duplicate longitudes */
			GMT_memcpy (Tout->segment[seg]->coord[GMT_Y], Tin->segment[seg]->coord[GMT_Y], Tin->segment[seg]->n_rows, double);	/* Duplicate latitudes */
			/* Resample lines along great circles (or parallels/meridians as per -A) */
			if (resample) {	/* Resample lon/lat path and also reallocate more space for all other columns */
				Tout->segment[seg]->n_rows = GMT_fix_up_path (GMT, &Tout->segment[seg]->coord[GMT_X], &Tout->segment[seg]->coord[GMT_Y], Tout->segment[seg]->n_rows, along_ds_degree, smode);
				for (col = 2; col < n_cols; col++)	/* Also realloc the other columns */
					Tout->segment[seg]->coord[col] = GMT_memory (GMT, Tout->segment[seg]->coord[col], Tout->segment[seg]->n_rows, double);
			}
			Tout->n_records += Tout->segment[seg]->n_rows;	/* Update record count */
			if (mode == 0) continue;	/* No dist/az needed */
			for (row = 0, along_dist = 0.0; row < Tout->segment[seg]->n_rows; row++) {	/* Process each point along resampled FZ trace */
				dist_inc = (row) ? GMT_distance (GMT, Tout->segment[seg]->coord[GMT_X][row], Tout->segment[seg]->coord[GMT_Y][row], Tout->segment[seg]->coord[GMT_X][row-1], Tout->segment[seg]->coord[GMT_Y][row-1]) : 0.0;
				along_dist += dist_inc;
				Tout->segment[seg]->coord[SEG_DIST][row] = along_dist;
				if (mode == 1) continue;	/* No az needed */
				if (row)
					azimuth = GMT_az_backaz (GMT, Tout->segment[seg]->coord[GMT_X][row-1], Tout->segment[seg]->coord[GMT_Y][row-1], Tout->segment[seg]->coord[GMT_X][row], Tout->segment[seg]->coord[GMT_Y][row], FALSE);
				else	/* Special deal for first point */
					azimuth = GMT_az_backaz (GMT, Tout->segment[seg]->coord[GMT_X][0], Tout->segment[seg]->coord[GMT_Y][0], Tout->segment[seg]->coord[GMT_X][1], Tout->segment[seg]->coord[GMT_Y][1], FALSE);
				Tout->segment[seg]->coord[SEG_AZIM][row] = azimuth;
			}
			ID[0] = 0;
			if (Tout->segment[seg]->label) strcpy (ID, Tout->segment[seg]->label);	/* Look for label in header */
			else if (Tout->segment[seg]->header) GMT_parse_segment_item (GMT, Tout->segment[seg]->header, "-L", ID);	/* Look for label in header */
			if (!ID[0]) sprintf (ID, "%*.*ld", ndig, ndig, seg_no);	/* Must assign a label from running numbers */
			if (!Tout->segment[seg]->label) Tout->segment[seg]->label = strdup (ID);
			if (Tout->segment[seg]->header) free (Tout->segment[seg]->header);
			sprintf (buffer, "Segment label -L%s", ID);
			Tout->segment[seg]->header = strdup (buffer);
		}
	}
	return (D);
}

struct GMT_DATASET * gmt_resample_data_cartesian (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double along_ds, GMT_LONG mode, GMT_LONG ex_cols, GMT_LONG smode)
{
	/* Din is a data set with at least two columns (x,y or lon/lat);
	 * it can contain any number of tables and segments with lines.
	 * along_ds is the resampling interval along the traces in Din.
	 * It is in user distance units; it is assumed that x and y have the same units!
	 * mode is either 0 (just x,y), 1 (x,y,dist) or 2 (x,y,dist,azim)
	 * ex_cols makes space for this many extra empty data columns [0]
	 * smode sets the sampling mode for GMT_fix_up_path
	 * Dout is the new data set with all the resampled lines;
	 */

	int ndig;
	GMT_LONG tbl, seg, row, col, n_cols, resample, seg_no;
	char buffer[GMT_BUFSIZ], ID[GMT_BUFSIZ];
	double along_dist, azimuth, dist_inc;
	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *Tin = NULL, *Tout = NULL;

	resample = (!GMT_IS_ZERO(along_ds));
	n_cols = 2 + mode + ex_cols;
	D = GMT_alloc_dataset (GMT, Din, n_cols, 0, GMT_ALLOC_NORMAL);	/* Same table length as Din, but with up to 4 columns (lon, lat, dist, az) */
	ndig = irint (floor (log10 ((double)Din->n_segments))) + 1;	/* Determine how many decimals are needed for largest segment id */

	for (tbl = seg_no = 0; tbl < Din->n_tables; tbl++) {
		Tin  = Din->table[tbl];
		Tout = D->table[tbl];
		for (seg = Tout->n_records = 0; seg < Tin->n_segments; seg++, seg_no++) {	/* For each segment to resample */
			GMT_memcpy (Tout->segment[seg]->coord[GMT_X], Tin->segment[seg]->coord[GMT_X], Tin->segment[seg]->n_rows, double);	/* Duplicate x */
			GMT_memcpy (Tout->segment[seg]->coord[GMT_Y], Tin->segment[seg]->coord[GMT_Y], Tin->segment[seg]->n_rows, double);	/* Duplicate y */
			/* Resample lines along straight lines (or along x/y as per -A) */
			if (resample) {	/* Resample x/y path and also reallocate more space for all other columns */
				Tout->segment[seg]->n_rows = GMT_fix_up_path_cartesian (GMT, &Tout->segment[seg]->coord[GMT_X], &Tout->segment[seg]->coord[GMT_Y], Tout->segment[seg]->n_rows, along_ds, smode);
				for (col = 2; col < n_cols; col++)	/* Also realloc the other columns */
					Tout->segment[seg]->coord[col] = GMT_memory (GMT, Tout->segment[seg]->coord[col], Tout->segment[seg]->n_rows, double);
			}
			Tout->n_records += Tout->segment[seg]->n_rows;	/* Update record count */
			if (mode == 0) continue;	/* No dist/az needed */
			for (row = 0, along_dist = 0.0; row < Tout->segment[seg]->n_rows; row++) {	/* Process each point along resampled FZ trace */
				dist_inc = (row) ? GMT_distance (GMT, Tout->segment[seg]->coord[GMT_X][row], Tout->segment[seg]->coord[GMT_Y][row], Tout->segment[seg]->coord[GMT_X][row-1], Tout->segment[seg]->coord[GMT_Y][row-1]) : 0.0;
				along_dist += dist_inc;
				Tout->segment[seg]->coord[SEG_DIST][row] = along_dist;
				if (mode == 1) continue;	/* No az needed */
				if (row)
					azimuth = GMT_az_backaz (GMT, Tout->segment[seg]->coord[GMT_X][row-1], Tout->segment[seg]->coord[GMT_Y][row-1], Tout->segment[seg]->coord[GMT_X][row], Tout->segment[seg]->coord[GMT_Y][row], FALSE);
				else	/* Special deal for first point */
					azimuth = GMT_az_backaz (GMT, Tout->segment[seg]->coord[GMT_X][0], Tout->segment[seg]->coord[GMT_Y][0], Tout->segment[seg]->coord[GMT_X][1], Tout->segment[seg]->coord[GMT_Y][1], FALSE);
				Tout->segment[seg]->coord[SEG_AZIM][row] = azimuth;
			}
			ID[0] = 0;
			if (Tout->segment[seg]->label) strcpy (ID, Tout->segment[seg]->label);	/* Look for label in header */
			else if (Tout->segment[seg]->header) GMT_parse_segment_item (GMT, Tout->segment[seg]->header, "-L", ID);	/* Look for label in header */
			if (!ID[0]) sprintf (ID, "%*.*ld", ndig, ndig, seg_no);	/* Must assign a label from running numbers */
			if (!Tout->segment[seg]->label) Tout->segment[seg]->label = strdup (ID);
			if (Tout->segment[seg]->header) free (Tout->segment[seg]->header);
			sprintf (buffer, "Segment label -L%s", ID);
			Tout->segment[seg]->header = strdup (buffer);
		}
	}
	return (D);

}

struct GMT_DATASET * GMT_resample_data (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double along_ds, GMT_LONG mode, GMT_LONG ex_cols, GMT_LONG smode)
{
	struct GMT_DATASET *D = NULL;
	if (GMT_is_geographic (GMT, GMT_IN))
		D = gmt_resample_data_spherical (GMT, Din, along_ds, mode, ex_cols, smode);
	else
		D = gmt_resample_data_cartesian (GMT, Din, along_ds, mode, ex_cols, smode);
	return (D);
}

struct GMT_DATASET * gmt_crosstracks_spherical (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double cross_length, double across_ds, GMT_LONG n_cols)
{
	/* Din is a data set with at least two columns (lon/lat);
	 * it can contain any number of tables and segments.
	 * cross_length is the desired length of cross-profiles, in meters.
	 * across_ds is the sampling interval to use along the cross-profiles.
	 * n_cols sets how many data columns beyond x,y,d should be allocated.
	 * Dout is the new data set with all the crossing profiles; it will
	 * have 4 + n_cols columns, where the first 4 are x,y,d,az.
	 */

	int ndig, sdig;

	GMT_LONG tbl, row, k, seg, left, right, ii, np_cross, seg_no, dim[4] = {0, 0, 0, 0};
	GMT_LONG n_x_seg = 0, n_x_seg_alloc = 0, n_half_cross, n_tot_cols;

	char buffer[GMT_BUFSIZ], seg_name[GMT_BUFSIZ], ID[GMT_BUFSIZ];

	double dist_inc, cross_half_width, d_shift, orientation, sign, az_cross, x, y;
	double dist_across_seg, angle_radians, across_ds_radians;
	double Rot[3][3], Rot0[3][3], E[3], P[3], L[3], R[3], T[3], X[3];

	struct GMT_DATASET *Xout = NULL;
	struct GMT_TABLE *Tin = NULL, *Tout = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;

	if (Din->n_columns < 2) {	/* Trouble */
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Dataset does not have at least 2 columns with coordinates\n");
		return (NULL);
	}

	/* Get resampling step size and zone width in degrees */

	cross_length *= 0.5;	/* Now half-length in user's units */
	cross_half_width = cross_length / GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Now in meters */
	n_half_cross = irint (cross_length / across_ds);	/* Half-width of points in a cross profile */
	across_ds_radians = D2R * (cross_half_width / GMT->current.proj.DIST_M_PR_DEG) / n_half_cross;	/* Angular change from point to point */
	np_cross = 2 * n_half_cross + 1;			/* Total cross-profile length */
	n_tot_cols = 4 + n_cols;	/* Total number of columns in the resulting data set */
	dim[0] = Din->n_tables;	dim[2] = n_tot_cols;	dim[3] = np_cross;
	if ((Xout = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, dim)) == NULL) return (NULL);	/* An empty dataset of n_tot_cols columns and np_cross rows */
	sdig = irint (floor (log10 ((double)Din->n_segments))) + 1;	/* Determine how many decimals are needed for largest segment id */

	for (tbl = seg_no = 0; tbl < Din->n_tables; tbl++) {	/* Process all tables */
		Tin  = Din->table[tbl];
		Tout = Xout->table[tbl];
		for (seg = 0; seg < Tin->n_segments; seg++, seg_no++) {	/* Process all segments */

			ndig = irint (floor (log10 ((double)Tin->segment[seg]->n_rows))) + 1;	/* Determine how many decimals are needed for largest id */

			GMT_report (GMT, GMT_MSG_NORMAL, "Process Segment %s [segment %ld] which has %ld crossing profiles\n", Tin->segment[seg]->label, seg, Tin->segment[seg]->n_rows);

			/* Resample control point track along great circle paths using specified sampling interval */

			for (row = 0; row < Tin->segment[seg]->n_rows; row++) {	/* Process each point along segment */
				/* Compute segment line orientation (-90/90) from azimuths */
				orientation = 0.5 * fmod (2.0 * Tin->segment[seg]->coord[SEG_AZIM][row], 360.0);
				if (orientation > 90.0) orientation -= 180.0;
				GMT_report (GMT, GMT_MSG_NORMAL, "Working on cross profile %ld [local line orientation = %06.1f]\n", row, orientation);

				x = Tin->segment[seg]->coord[GMT_X][row];	y = Tin->segment[seg]->coord[GMT_Y][row];	/* Reset since now we want lon/lat regardless of grid format */
				GMT_geo_to_cart (GMT, y, x, P, TRUE);		/* 3-D vector of current point P */
				left = (row) ? row - 1 : row;			/* Left point (if there is one) */
				x = Tin->segment[seg]->coord[GMT_X][left];	y = Tin->segment[seg]->coord[GMT_Y][left];
				GMT_geo_to_cart (GMT, y, x, L, TRUE);		/* 3-D vector of left point L */
				right = (row < (Tin->segment[seg]->n_rows-1)) ? row + 1 : row;	/* Right point (if there is one) */
				x = Tin->segment[seg]->coord[GMT_X][right];	y = Tin->segment[seg]->coord[GMT_Y][right];
				GMT_geo_to_cart (GMT, y, x, R, TRUE);		/* 3-D vector of right point R */
				GMT_cross3v (GMT, L, R, T);			/* Get pole T of plane trough L and R (and center of Earth) */
				GMT_normalize3v (GMT, T);			/* Make sure T has unit length */
				GMT_cross3v (GMT, T, P, E);			/* Get pole E to plane trough P normal to L,R (hence going trough P) */
				GMT_normalize3v (GMT, E);			/* Make sure E has unit length */
				gmt_init_rot_matrix (Rot0, E);			/* Get partial rotation matrix since no actual angle is applied yet */
				az_cross = fmod (Tin->segment[seg]->coord[SEG_AZIM][row] + 270.0, 360.0);	/* Azimuth of cross-profile in 0-360 range */
				sign = (az_cross >= 315.0 || az_cross < 135.0) ? -1.0 : 1.0;	/* We want profiles to be either ~E-W or ~S-N */
				dist_across_seg = 0.0;
				S = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);
				GMT_alloc_segment (GMT, S, np_cross, n_tot_cols, TRUE);
				for (k = -n_half_cross, ii = 0; k <= n_half_cross; k++, ii++) {	/* For each point along normal to FZ */
					angle_radians = sign * k * across_ds_radians;		/* The required rotation for this point relative to FZ origin */
					GMT_memcpy (Rot, Rot0, 9, double);			/* Get a copy of the "0-angle" rotation matrix */
					gmt_load_rot_matrix (angle_radians, Rot, E);		/* Build the actual rotation matrix for this angle */
					gmt_matrix_vect_mult (Rot, P, X);				/* Rotate the current FZ point along the normal */
					GMT_cart_to_geo (GMT, &S->coord[GMT_Y][ii], &S->coord[GMT_X][ii], X, TRUE);	/* Get lon/lat of this point along crossing profile */
					dist_inc = (ii) ? GMT_distance (GMT, S->coord[GMT_X][ii], S->coord[GMT_Y][ii], S->coord[GMT_X][ii-1], S->coord[GMT_Y][ii-1]) : 0.0;
					dist_across_seg += dist_inc;
					S->coord[SEG_DIST][ii] = dist_across_seg;	/* Store distances across the profile */
					if (ii) S->coord[SEG_AZIM][ii] = GMT_az_backaz (GMT, S->coord[GMT_X][ii-1], S->coord[GMT_Y][ii-1], S->coord[GMT_X][ii], S->coord[GMT_Y][ii], FALSE);
				}
				S->coord[SEG_AZIM][0] = GMT_az_backaz (GMT, S->coord[GMT_X][0], S->coord[GMT_Y][0], S->coord[GMT_X][1], S->coord[GMT_Y][1], FALSE);	/* Special deal for first point */

				/* Reset distance origin for cross profile */

				d_shift = S->coord[SEG_DIST][n_half_cross];	/* d_shift is here the distance at the center point (i.e., where crossing the guide FZ) */
				for (ii = 0; ii < np_cross; ii++) S->coord[SEG_DIST][ii] -= d_shift;	/* We reset the origin for distances to where this profile crosses the trial FZ */

				orientation = 0.5 * fmod (2.0 * (Tin->segment[seg]->coord[SEG_AZIM][row]+90.0), 360.0);	/* Orientation of cross-profile at zero distance */
				if (orientation > 90.0) orientation -= 180.0;
				ID[0] = seg_name[0] = 0;
				if (Tin->segment[seg]->label) {	/* Use old segment label and append crossprofile number */
					sprintf (ID, "%s-%*.*ld", Tin->segment[seg]->label, ndig, ndig, row);
				}
				else if (Tin->segment[seg]->header) {	/* Look for label in header */
					GMT_parse_segment_item (GMT, Tin->segment[seg]->header, "-L", seg_name);
					if (seg_name[0]) sprintf (ID, "%s-%*.*ld", seg_name, ndig, ndig, row);
				}
				if (!ID[0]) {	/* Must assign a label from running numbers */
					if (Tin->n_segments == 1 && Din->n_tables == 1)	/* Single track, just list cross-profile no */
						sprintf (ID, "%*.*ld", ndig, ndig, row);
					else	/* Segment number and cross-profile no */
						sprintf (ID, "%*.*ld-%*.*ld", sdig, sdig, seg_no, ndig, ndig, row);
				}
				S->label = strdup (ID);
				sprintf (buffer, "Cross profile number -L%s at %8.3f/%07.3f az=%05.1f",
					ID, Tin->segment[seg]->coord[GMT_X][row], Tin->segment[seg]->coord[GMT_Y][row], orientation);
				S->header = strdup (buffer);

				if (n_x_seg == n_x_seg_alloc) Tout->segment = GMT_memory (GMT, Tout->segment, (n_x_seg_alloc += GMT_SMALL_CHUNK), struct GMT_LINE_SEGMENT *);
				Tout->segment[n_x_seg++] = S;
				Tout->n_segments++;	Xout->n_segments++;
				Tout->n_records += np_cross;	Xout->n_records += np_cross;
			}
		}
	}

	return (Xout);
}

struct GMT_DATASET * gmt_crosstracks_cartesian (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double cross_length, double across_ds, GMT_LONG n_cols)
{
	/* Din is a data set with at least two columns (x,y);
	 * it can contain any number of tables and segments.
	 * cross_length is the desired length of cross-profiles, in Cartesian units.
	 * across_ds is the sampling interval to use along the cross-profiles.
	 * n_cols sets how many data columns beyond x,y,d should be allocated.
	 * Dout is the new data set with all the crossing profiles;
	*/

	int ndig, sdig;

	GMT_LONG tbl, row, k, seg, ii, np_cross, seg_no, dim[4] = {0, 0, 0, 0};
	GMT_LONG n_x_seg = 0, n_x_seg_alloc = 0, n_half_cross, n_tot_cols;

	char buffer[GMT_BUFSIZ], seg_name[GMT_BUFSIZ], ID[GMT_BUFSIZ];

	double dist_across_seg, orientation, sign, az_cross, x, y, sa, ca;

	struct GMT_DATASET *Xout = NULL;
	struct GMT_TABLE *Tin = NULL, *Tout = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;

	if (Din->n_columns < 2) {	/* Trouble */
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Dataset does not have at least 2 columns with coordinates\n");
		return (NULL);
	}

	/* Get resampling step size and zone width in degrees */

	cross_length *= 0.5;					/* Now half-length in user's units */
	n_half_cross = irint (cross_length / across_ds);	/* Half-width of points in a cross profile */
	np_cross = 2 * n_half_cross + 1;			/* Total cross-profile length */
	across_ds = cross_length / n_half_cross;		/* Exact increment (recalculated in case of roundoff) */
	n_tot_cols = 4 + n_cols;				/* Total number of columns in the resulting data set */
	dim[0] = Din->n_tables;	dim[2] = n_tot_cols;	dim[3] = np_cross;
	if ((Xout = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, dim)) == NULL) return (NULL);	/* An empty dataset of n_tot_cols columns and np_cross rows */
	sdig = irint (floor (log10 ((double)Din->n_segments))) + 1;	/* Determine how many decimals are needed for largest segment id */

	for (tbl = seg_no = 0; tbl < Din->n_tables; tbl++) {	/* Process all tables */
		Tin  = Din->table[tbl];
		Tout = Xout->table[tbl];
		for (seg = 0; seg < Tin->n_segments; seg++, seg_no++) {	/* Process all segments */

			ndig = irint (floor (log10 ((double)Tin->segment[seg]->n_rows))) + 1;	/* Determine how many decimals are needed for largest id */

			GMT_report (GMT, GMT_MSG_NORMAL, "Process Segment %s [segment %ld] which has %ld crossing profiles\n", Tin->segment[seg]->label, seg, Tin->segment[seg]->n_rows);

			for (row = 0; row < Tin->segment[seg]->n_rows; row++) {	/* Process each point along segment */
				/* Compute segment line orientation (-90/90) from azimuths */
				orientation = 0.5 * fmod (2.0 * Tin->segment[seg]->coord[SEG_AZIM][row], 360.0);
				if (orientation > 90.0) orientation -= 180.0;
				GMT_report (GMT, GMT_MSG_NORMAL, "Working on cross profile %ld [local line orientation = %06.1f]\n", row, orientation);

				x = Tin->segment[seg]->coord[GMT_X][row];	y = Tin->segment[seg]->coord[GMT_Y][row];	/* Reset since now we want lon/lat regardless of grid format */
				az_cross = fmod (Tin->segment[seg]->coord[SEG_AZIM][row] + 270.0, 360.0);	/* Azimuth of cross-profile in 0-360 range */
				sign = (az_cross >= 315.0 || az_cross < 135.0) ? -1.0 : 1.0;	/* We want profiles to be either ~E-W or ~S-N */
				S = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);
				GMT_alloc_segment (GMT, S, np_cross, n_tot_cols, TRUE);
				sincosd (90.0 - az_cross, &sa, &ca);	/* Trig on the direction */
				for (k = -n_half_cross, ii = 0; k <= n_half_cross; k++, ii++) {	/* For each point along normal to FZ */
					dist_across_seg = sign * k * across_ds;		/* The current distance along this profile */
					S->coord[GMT_X][ii] = x + dist_across_seg * ca;
					S->coord[GMT_Y][ii] = y + dist_across_seg * sa;
					S->coord[SEG_DIST][ii] = dist_across_seg;	/* Store distances across the profile */
					if (ii) S->coord[SEG_AZIM][ii] = GMT_az_backaz (GMT, S->coord[GMT_X][ii-1], S->coord[GMT_Y][ii-1], S->coord[GMT_X][ii], S->coord[GMT_Y][ii], FALSE);
				}
				S->coord[SEG_AZIM][0] = GMT_az_backaz (GMT, S->coord[GMT_X][0], S->coord[GMT_Y][0], S->coord[GMT_X][1], S->coord[GMT_Y][1], FALSE);	/* Special deal for first point */

				orientation = 0.5 * fmod (2.0 * (Tin->segment[seg]->coord[SEG_AZIM][row]+90.0), 360.0);	/* Orientation of cross-profile at zero distance */
				if (orientation > 90.0) orientation -= 180.0;
				ID[0] = seg_name[0] = 0;
				if (Tin->segment[seg]->label) {	/* Use old segment label and append crossprofile number */
					sprintf (ID, "%s-%*.*ld", Tin->segment[seg]->label, ndig, ndig, row);
				}
				else if (Tin->segment[seg]->header) {	/* Look for label in header */
					GMT_parse_segment_item (GMT, Tin->segment[seg]->header, "-L", seg_name);
					if (seg_name[0]) sprintf (ID, "%s-%*.*ld", seg_name, ndig, ndig, row);
				}
				if (!ID[0]) {	/* Must assign a label from running numbers */
					if (Tin->n_segments == 1 && Din->n_tables == 1)	/* Single track, just list cross-profile no */
						sprintf (ID, "%*.*ld", ndig, ndig, row);
					else	/* Segment number and cross-profile no */
						sprintf (ID, "%*.*ld-%*.*ld", sdig, sdig, seg_no, ndig, ndig, row);
				}
				S->label = strdup (ID);
				sprintf (buffer, "Cross profile number -L%s at %g/%g az=%05.1f",
					ID, Tin->segment[seg]->coord[GMT_X][row], Tin->segment[seg]->coord[GMT_Y][row], orientation);
				S->header = strdup (buffer);

				if (n_x_seg == n_x_seg_alloc) Tout->segment = GMT_memory (GMT, Tout->segment, (n_x_seg_alloc += GMT_SMALL_CHUNK), struct GMT_LINE_SEGMENT *);
				Tout->segment[n_x_seg++] = S;
				Tout->n_segments++;	Xout->n_segments++;
				Tout->n_records += np_cross;	Xout->n_records += np_cross;
			}
		}
	}

	return (Xout);
}

struct GMT_DATASET * GMT_crosstracks (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double cross_length, double across_ds, GMT_LONG n_cols)
{	/* Call either the spherical or Cartesian version */
	struct GMT_DATASET *D = NULL;
	if (GMT_is_geographic (GMT, GMT_IN))
		D = gmt_crosstracks_spherical (GMT, Din, cross_length, across_ds, n_cols);
	else
		D = gmt_crosstracks_cartesian (GMT, Din, cross_length, across_ds, n_cols);
	return (D);
}

GMT_LONG gmt_straddle_dateline (double x0, double x1) {
	if (fabs (x0 - x1) > 90.0) return (FALSE);	/* Probably Greenwhich crossing with 0/360 discontinuity */
	if ((x0 < 180.0 && x1 > 180.0) || (x0 > 180.0 && x1 < 180.0)) return (TRUE);	/* Crossed Dateline */
	return (FALSE);
}

GMT_LONG GMT_crossing_dateline (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S)
{	/* Return TRUE if this line or polygon feature contains points on either side of the Dateline */
	GMT_LONG k, east = FALSE, west = FALSE, cross = FALSE;
	for (k = 0; !cross && k < S->n_rows; k++) {
		if ((S->coord[GMT_X][k] > 180.0 && S->coord[GMT_X][k] < 270.0) || (S->coord[GMT_X][k] > -180.0 && S->coord[GMT_X][k] < -90.0)) west = TRUE;
		if ((S->coord[GMT_X][k] > 90.0 && S->coord[GMT_X][k] < 180.0) || (S->coord[GMT_X][k] > -270.0 && S->coord[GMT_X][k] < -180.0)) east = TRUE;
		if (east && west) cross = TRUE;
	}
	return (cross);
}

GMT_LONG GMT_split_line_at_dateline (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, struct GMT_LINE_SEGMENT ***Lout)
{	/* Create two or more feature segments by splitting them across the Dateline.
	 * GMT_split_line_at_dateline should ONLY be called when we KNOW we must split. */
	GMT_LONG k, row, col, seg, n_split, start, length, *pos = GMT_memory (C, NULL, S->n_rows, GMT_LONG);
	char label[GMT_BUFSIZ], *txt = NULL, *feature = "Line";
	double r;
	struct GMT_LINE_SEGMENT **L = NULL, *Sx = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
	
	for (k = 0; k < S->n_rows; k++) GMT_lon_range_adjust (GMT_IS_0_TO_P360_RANGE, &S->coord[GMT_X][k]);	/* First enforce 0 <= lon < 360 so we dont have to check again */
	GMT_alloc_segment (C, Sx, 2*S->n_rows, S->n_columns, TRUE);	/* Temp segment with twice the number of points as we will add crossings*/
	
	for (k = row = n_split = 0; k < S->n_rows; k++) {	/* Hunt for crossings */
		if (k && gmt_straddle_dateline (S->coord[GMT_X][k-1], S->coord[GMT_X][k])) {	/* Crossed Dateline */
			r = (180.0 - S->coord[GMT_X][k-1]) / (S->coord[GMT_X][k] - S->coord[GMT_X][k-1]);	/* Fractional distance from k-1'th point to 180 crossing */
			Sx->coord[GMT_X][row] = 180.0;	/* Exact longitude is known */
			for (col = 1; col < S->n_columns; col++) Sx->coord[col][row] = S->coord[col][k-1] + r * (S->coord[col][k] - S->coord[col][k-1]);	/* Linear interpolation for other fields */
			pos[n_split++] = row++;		/* Keep track of first point (the crossing) in new section */
		}
		for (col = 0; col < S->n_columns; col++) Sx->coord[col][row] = S->coord[col][k];	/* Append the current point */
		row++;
	}
	Sx->n_rows = row;	/* Number of points in extended feature with explicit crossings */
	if (n_split == 0) {	/* No crossings, should not have been called in the first place */
		GMT_report (C, GMT_MSG_NORMAL, "GMT_split_line_at_dateline called but no straddling detected (bug?)\n");
		GMT_free_segment (C, Sx);
		GMT_free (C, pos);
		return 0;
	}
	pos[n_split] = Sx->n_rows - 1;
	n_split++;	/* Now means number of segments */
	L = GMT_memory (C, NULL, n_split, struct GMT_LINE_SEGMENT *);	/* Number of output segments needed are allocated here */
	txt = (S->label) ? S->label : feature;	/* What to label the features */
	start = 0;
	for (seg = 0; seg < n_split; seg++) {	/* Populate the output segment coordinate arrays */
		L[seg] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);		/* Allocate space for one segment */
		length = pos[seg] - start + 1;	/* Length of new segment */
		GMT_alloc_segment (C, L[seg], length, S->n_columns, TRUE);		/* Allocate array space for coordinates */
		for (col = 0; col < S->n_columns; col++) GMT_memcpy (L[seg]->coord[col], &(Sx->coord[col][start]), length, double);	/* Copy coordinates */
		L[seg]->range = (L[seg]->coord[GMT_X][length/2] > 180.0) ? GMT_IS_M180_TO_P180 : GMT_IS_M180_TO_P180_RANGE;	/* Formatting ID to enable special -180 and +180 formatting on outout */
		/* Modify label to part number */
		sprintf (label, "%s part %ld", txt, seg);
		L[seg]->label = strdup (label);
		if (S->header) L[seg]->header = strdup (S->header);
		if (S->ogr) GMT_duplicate_ogr_seg (C, L[seg], S);
		start = pos[seg];
	}
	GMT_free_segment (C, Sx);
	GMT_free (C, pos);
	
	*Lout = L;		/* Pass pointer to the array of segments */
	
	return (n_split);	/* Return how many segments was made */
}

GMT_LONG GMT_detrend (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double increment, double *intercept, double *slope, GMT_LONG mode)
{	/* Deals with linear trend in a dataset, depending on mode:
	 * -1: Determine trend, and remove it from x,y. Return slope and intercept
	 * 0 : Just determine trend. Return slope and intercept
	 * +1: Restore trend in x,y based on given slope/intercept.
	 * (x,y) is the data.  If x == NULL then data is equidistant with increment as the spacing.
	 */
	GMT_LONG i, equidistant;
	double xx;
	
	equidistant = (x == NULL);	/* If there are no x-values we assume dx is passed via intercept */
	if (mode < 1) {	/* Must determine trend */
		GMT_LONG m;
		double sum_x = 0.0, sum_xx = 0.0, sum_y = 0.0, sum_xy = 0.0;
		for (i = m = 0; i < n; i++) {
			if (GMT_is_dnan (y[i])) continue;
			xx = (equidistant) ? increment*i : x[i];
			sum_x  += xx;
			sum_xx += xx*xx;
			sum_y  += y[i];
			sum_xy += xx*y[i];
			m++;
		}
		if (m > 1) {	/* Got enough points to compute the trend */
			*intercept = (sum_y*sum_xx - sum_x*sum_xy) / (m*sum_xx - sum_x*sum_x);
			*slope = (m*sum_xy - sum_x*sum_y) / (m*sum_xx - sum_x*sum_x);
		}
		else {
			GMT_report (C, GMT_MSG_NORMAL, "GMT_detrend called with less than 2 points, return NaNs\n");
			*intercept = (m) ? sum_y : C->session.d_NaN;	/* Value of single y-point or NaN */
			*slope = C->session.d_NaN;
		}
	}
	
	if (mode) {	/* Either remove or restore trend from/to the data */
		if (GMT_is_dnan (*slope)) {
			GMT_report (C, GMT_MSG_NORMAL, "GMT_detrend called with slope = NaN - skipped\n");
			return (-1);
		}
		if (GMT_is_dnan (*intercept)) {
			GMT_report (C, GMT_MSG_NORMAL, "GMT_detrend called with intercept = NaN - skipped\n");
			return (-1);
		}
		for (i = 0; i < n; i++) {
			xx = (equidistant) ? increment*i : x[i];
			y[i] += (mode * (*intercept + (*slope) * xx));
		}
	}
	return (GMT_NOERROR);
}

char *GMT_putusername (struct GMT_CTRL *C)
{
	static char *unknown = "unknown";
#ifdef HAVE_GETPWUID
#include <pwd.h>
	struct passwd *pw = NULL;
	pw = getpwuid (getuid ());
	if (pw) return (pw->pw_name);
#endif
	return (unknown);
}
