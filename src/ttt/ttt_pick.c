/*	ttt_pick.c
 *
 * ttt_pick reads a xyfile, opens the 2d gridded traveltime file, 
 * and samples the dataset at the xy positions with a bilinear interpolation.
 * If the nearest node is NaN it will search for the nearest non-NaN node.
 * This new data is added to the input as an extra column and printed
 * to standard output.  In order to evaluate derivatives along the edges
 * of the surface, I assume natural bi-cubic spline conditions, i.e.
 * both the second and third normal derivatives are zero, and that the
 * dxdy derivative in the corners are zero, too.  Proper map projection
 * is assumed to have been done.
 *
 * PROGRAM:	ttt.c
 * PURPOSE:	Sample ttt grids at given coordinates
 * AUTHOR:	Paul Wessel, GEOWARE
 * DATE:	June 16 1993
 * UPDATED:	August 1, 2024
 * VERSION:	4.0
 */

#include "ttt.h"

#define		SEARCH_RADIUS		1.12415		/* Default 125 km search radius (in spherical degrees) for moving a station off land or to deep-enough water */
#define		DEGREE_2_KM	(0.001 * DEGREE_TO_METER)

#include "ttt_subs.c"	/* Include common functions shared with ttt */

int main (int argc, char **argv)
{
	TTT_LONG i, i_0, j_0, a, b, c, d, k, ix, iy, row_width, col_height, n_read;
	TTT_LONG i1, i2, j, n_header_recs = 1, swabbing, n_points = 0, rec = 0;
	
	BOOLEAN error = FALSE, give_dist = FALSE, flip = FALSE, header = FALSE;
	BOOLEAN search = FALSE, verbose = FALSE, quick = FALSE;
	
	float *f = NULL, *z = NULL, TTT_NaN;

	double value, xy[2], i_xinc, i_yinc, x, y, cx, cy, cxy, dx, dy, shortest_dist;
	double search_depth = 0.0, z_value = 0.0, hours_to_unit = 1.0;
	
	char txt[2][32], stuff[BUFSIZ], line[BUFSIZ], format1[BUFSIZ], format2[BUFSIZ];
	char *grdfile = NULL, *d_format = "%lg", zfile[BUFSIZ];
	
	FILE *fp = NULL;
	
	struct GRD_HEADER grd, Z;
	
	TTT_make_NaN (TTT_NaN);

 	for (i = 1; !error && i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			
				case '\0':	/* Give short synopsis */

					quick = error = TRUE;
					break;

				/* Common parameters */
			
				case 'H':
					header = TRUE;
					if (argv[i][2]) n_header_recs = atoi (&argv[i][2]);
					break;
				case 'I':
					fprintf (stderr, "ttt_pick: -I is obsolete; format determined from file extension.\n");
					break;
				case 'V':
					verbose = TRUE;
					break;
				case 'h':	/* Want hours on output */
					hours_to_unit = 1.0;
					break;
				case 'm':	/* Want minutes on output */
					hours_to_unit = 60.0;
					break;
				case 's':	/* Want secods on output */
					hours_to_unit = 3600.0;
					break;
				case ':':
					flip = TRUE;
					break;

				/* Supplemental parameters */
				
				case 'G':
					grdfile = &argv[i][2];
					break;

				case 'D':
					give_dist = TRUE;
					break;

				case 'Z':	/* Sample station at sufficient water depth */
					search = TRUE;
					sscanf (&argv[i][2], "%[^,],%s", stuff, zfile);
					search_depth = atof (stuff);
					break;

				default:
					error = TRUE;
					break;
			}
		}
		else if ((fp = fopen (argv[i], "r")) == NULL) {
			fprintf (stderr, "ttt_pick: Cannot open file %s\n", argv[i]);
			exit (EXIT_FAILURE);
		}
	}
	
	if (argc == 1 || error) {
		fprintf (stderr,"ttt_pick %s - Sampling of a 2-D gridded travel time file at specified locations\n\n", TTT_VERSION);
		fprintf (stderr, "usage: ttt_pick <xyfile> -G<ttt_file> [-D] [-H[<n>]] [-V] [-Z<depth>,<depth.i2>] [-h|m|s] [-:]\n");
		if (quick) exit (EXIT_FAILURE);

		fprintf (stderr, "	<xyfile> is a multicolumn ASCII file with (lon,lat) in the first two columns\n");
		fprintf (stderr, "	-G <ttt_file> is the name of the 2-D grid produced by ttt.\n");
		fprintf (stderr, "\n\tOPTIONS:\n");
		fprintf (stderr, "	-D will report the distance to the nearest non-NaN node in last column [no report]\n");
		fprintf (stderr, "	-H means <xyfile> has one or more (append <n>) header %cecords\n", 'r');
		fprintf (stderr, "	-I Travel times in grdfile were stored as 2 byte ints with units of 10 sec [Default is 4-byte float in hours]\n");
		fprintf (stderr, "	-V for verbal feedback; will report progress\n");
		fprintf (stderr, "	-Z Sample travel time at point nearest to station where water depth is <depth> m or deeper.\n");
		fprintf (stderr, "	   Give <depth.grd> is the name of the bathymetry grid used to calculate the travel times.\n");
		fprintf (stderr, "	   Note <depth> is negative below the sea surface [Default is no depth search]\n");
		fprintf (stderr, "	-h|m|s reports travel times in hours, minutes, or seconds [hours]\n");
		fprintf (stderr, "	-: <xyfile> contains lat,lon instead of lon,lat\n");
		exit (EXIT_FAILURE);
	}
	
	if (!grdfile) {
		fprintf (stderr, "ttt_pick: Must specify a travel time grid file\n");
		exit (EXIT_FAILURE);
	}
	
	if (fp == NULL) fp = stdin;

	swabbing = ttt_read_header (grdfile, &grd, 0.0, 0.0, 0.0, 0.0);
	
	row_width = grd.nx + 2;		col_height = grd.ny + 2;
	
	ttt_read_grid (grdfile, &grd, &f, 0.0, 0.0, 0.0, 0.0, 1, swabbing);

	i_xinc = 1.0 / grd.inc[TTT_X];
	i_yinc = 1.0 / grd.inc[TTT_Y];
	
	/* Set natural cubic spline BC's */
	
	for (i1 = 0, i2 = (col_height-1)*row_width; i1 < row_width; i1++, i2++) {
		f[i1] = (float)(2.0 * f[i1+row_width] - f[i1+2*row_width]);
		f[i2] = (float)(2.0 * f[i2-row_width] - f[i2-2*row_width]);
	}
	for (j = 1, i1 = row_width - 2; j < col_height-1; j++) {
		i1 = j * row_width;
		f[i1] = (float)(2.0 * f[i1+1] - f[i1+2]);
		i1 += row_width - 2;
		f[i1] = (float)(2.0 * f[i1-1] - f[i1-2]);
	}
	
	if (search) {
		swabbing = ttt_read_header (zfile, &Z, 0.0, 0.0, 0.0, 0.0);
		if (! (grd.nx == Z.nx && grd.ny == Z.ny)) {
			fprintf (stderr, "ttt_pick: Travel time grid and depth grid not of same size\n");
			exit (EXIT_FAILURE);
		}
		ttt_read_grid (zfile, &Z, &z, 0.0, 0.0, 0.0, 0.0, 1, swabbing);
	}
	
	if (search) {	/* Report both depth and shortest distance */
		sprintf (format1, "%s\t%s\t%%s\t%s\t%s\t%s\n", d_format, d_format, d_format, d_format, d_format);
		sprintf (format2, "%s\t%s\t%s\t%s\t%s\n", d_format, d_format, d_format, d_format, d_format);
	}
	else if (give_dist) {
		sprintf (format1, "%s\t%s\t%%s\t%s\t%s\n", d_format, d_format, d_format, d_format);
		sprintf (format2, "%s\t%s\t%s\t%s\n", d_format, d_format, d_format, d_format);
	}
	else {
		sprintf (format1, "%s\t%s\t%%s\t%s\n", d_format, d_format, d_format);
		sprintf (format2, "%s\t%s\t%s\n", d_format, d_format, d_format);
	}
	
	if (header) {	/* First echo headers, if any */
		for (i = 0; i < n_header_recs - 1; i++) {
			fgets (line, BUFSIZ, fp);
			printf ("%s", line);
		}
		fgets (line, BUFSIZ, fp);
		line[strlen(line)-1] = 0;
		printf ("%s\tsample\n", line);
	}

	ix = (flip) ? 1 : 0;	iy = 1 - ix;		/* Set up which columns have x and y */
	while (fgets (line, BUFSIZ, fp)) {
		rec++;
		if (line[0] == '>' || line[0] == '#' || line[0] == '\0') continue;

		n_read = sscanf (line, "%s %s %[^\n]", txt[ix], txt[iy], stuff);
		if (n_read < 2) {
			fprintf (stderr, "ttt_pick: Error reading coordinates from record # %" TTT_LL "d (skipped)\n", rec);
			continue;
		}
		xy[0] = txt2fl (txt[0], 'W');
		xy[1] = txt2fl (txt[1], 'S');

		/* Check that we are inside the grd area */
		
		if (xy[1] < grd.wesn[YLO] || xy[1] > grd.wesn[YHI]) continue;
		xy[0] -= 360.0;
		while (xy[0] < grd.wesn[XLO]) xy[0] += 360.0;
		if (xy[0] > grd.wesn[XHI]) continue;
		
		/* Get nearest node */
		
		i_0 = ttt_x_to_i (xy[0], grd.wesn[XLO], grd.inc[TTT_X], grd.xy_off, grd.nx);
		j_0 = ttt_y_to_j (xy[1], grd.wesn[YLO], grd.inc[TTT_Y], grd.xy_off, grd.ny);
		k = IJ(i_0,j_0,row_width,1);
		
		if (search || TTT_is_fnan (f[k])) {	/* Must search for nearest non-NaN node */
			TTT_LONG i, j, ij, imin, imax, jmin, jmax;
			double xx, yy, d, s_radius = MAX (SEARCH_RADIUS, 2.0 * grd.inc[TTT_X]);
                
			dx = grd.inc[TTT_X] * cos (D2R * xy[1]);
			imin = MAX (0, i_0 - (TTT_LONG)ceil (s_radius / dx));
			imax = MIN (grd.nx - 1, i_0 + (TTT_LONG)ceil (s_radius / dx));
			jmin = MAX (0, j_0 - (TTT_LONG)ceil (s_radius / grd.inc[TTT_Y]));
			jmax = MIN (grd.ny - 1, j_0 + (TTT_LONG)ceil (s_radius / grd.inc[TTT_Y]));
                
			shortest_dist = 180.0;
			xx = yy = 0.0;
			for (j = jmin; j <= jmax; j++) {
				y = ttt_j_to_y (j, grd.wesn[YLO], grd.wesn[YHI], grd.inc[TTT_Y], grd.xy_off, grd.ny);
				for (i = imin; i <= imax; i++) {
					ij = IJ(i,j,row_width,1);
					if (TTT_is_fnan (f[ij])) continue;
					if (search && (TTT_is_fnan (z[ij])|| z[ij] >= search_depth)) continue;	/* Too shallow */
					x = ttt_i_to_x (i, grd.wesn[XLO], grd.wesn[XHI], grd.inc[TTT_X], grd.xy_off, grd.nx);
					d = ttt_great_circle_dist (xy[0], xy[1], x, y);
					if (d < shortest_dist) {
						k = ij;
						shortest_dist = d;
						xx = x;
						yy = y;
					}
				}
			}
                
			if (shortest_dist == 180.0) {           
				fprintf (stderr, "\nttt_pick: Station location more than %g degrees from the ocean - check!\n", s_radius);
				fprintf (stderr, "[%s]\n", line);
				value = z_value = TTT_NaN;
			}
			else {
				xy[0] = xx;
				xy[1] = yy;
				value = f[k];
				if (search) z_value = z[k];
			}
			shortest_dist *= DEGREE_2_KM;
		}
		else {	/* May attempt to do bilinear interpolation if 4 nodes exist */
			shortest_dist = 0.0;
			x = ttt_i_to_x (i_0, grd.wesn[XLO], grd.wesn[XHI], grd.inc[TTT_X], grd.xy_off, grd.nx);
			y = ttt_j_to_y (j_0, grd.wesn[YLO], grd.wesn[YHI], grd.inc[TTT_Y], grd.xy_off, grd.ny);
			dx = xy[0] - x;
			dy = xy[1] - y;
		
			if (dx >= 0.0) {
				if (dy >= 0.0) {
					a = k;	b = k + 1;	c = k + 1 - row_width;	d = k - row_width;
				}
				else {
					a = k + row_width;	b = k + row_width + 1;	c = k + 1;	d = k;
					dy += grd.inc[TTT_Y];
				}
			}
			else {
				dx += grd.inc[TTT_X];
				if (dy >= 0.0) {
					a = k - 1;	b = k;	c = k - row_width;	d = k - row_width - 1;
				}
				else {
					a = k - 1 + row_width;	b = k + row_width;	c = k;	d = k - 1;
					dy += grd.inc[TTT_Y];
				}
			}
			if (TTT_is_fnan (f[a]) || TTT_is_fnan (f[b]) || TTT_is_fnan (f[c]) || TTT_is_fnan (f[d])) {	/* Shoot, at least one NaN; pick nearest node and report distance to actual */
				shortest_dist = DEGREE_2_KM * hypot (dx * cos (D2R * xy[1]), dy);
				value = f[k];
			}
			else {	/* Here we can interpolate so we assume shortest_dist = 0.0 */
				cx = (f[b] - f[a]) * i_xinc;
				cy = (f[d] - f[a]) * i_yinc;
				cxy = (f[c] - f[b] + f[a] - f[d]) * i_xinc * i_yinc;
				
				value = cx * dx + cy * dy + cxy * dx * dy + f[a];
			}
			if (search) {
				if (TTT_is_fnan (z[a]) || TTT_is_fnan (z[b]) || TTT_is_fnan (z[c]) || TTT_is_fnan (z[d]))	/* Shoot, at least one NaN; pick nearest node */
					z_value = f[k];
				else {	/* Here we can interpolate so we assume shortest_dist = 0.0 */
					cx = (z[b] - z[a]) * i_xinc;
					cy = (z[d] - z[a]) * i_yinc;
					cxy = (z[c] - z[b] + z[a] - z[d]) * i_xinc * i_yinc;
				
					z_value = cx * dx + cy * dy + cxy * dx * dy + z[a];
				}
			}
			
		}
		value *= hours_to_unit;
		if (n_read == 2) {	/* Note output location is the adjusted location that falls on the grid nodes */
			if (search)
				printf (format2, xy[0], xy[1], value, z_value, shortest_dist);
			else if (give_dist)
				printf (format2, xy[0], xy[1], value, shortest_dist);
			else
				printf (format2, xy[0], xy[1], value);
			}
		else {
			if (search)
				printf (format1, xy[0], xy[1], stuff, value, z_value, shortest_dist);
			else if (give_dist)
				printf (format1, xy[0], xy[1], stuff, value, shortest_dist);
			else
				printf (format1, xy[0], xy[1], stuff, value);
		}
		n_points++;
	}
	fclose (fp);
	
	if (verbose) fprintf (stderr, "ttt_pick: Sampled %" TTT_LL "d points from grid %s (%d x %d)\n",
		n_points, grdfile, grd.nx, grd.ny);
	
	free ((void *)f);
	if (search) free ((void *)z);
	
	exit (EXIT_SUCCESS);
}
