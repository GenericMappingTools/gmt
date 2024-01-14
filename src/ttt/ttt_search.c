/*	ttt_search.c
 *
 * ttt_search is used to determine how much a point must be moved to
 * place it at a specified water depth.  Also gives an estimate
 * of the likely max delay in propagation from original point
 * to the relocated point.  If the original point is actually on
 * land then we first move it to the shore.
 *
 * AUTHOR:	Paul Wessel, GEOWARE
 * UPDATED:	January 1, 2024
 */

#include "ttt.h"

#define		RADIUS		5.0     /* Search radius in degrees */
#define		DEGREE_2_KM	111.31709
#define		THRESHOLD	0.1
#define		TTT_CONV_LIMIT	1.0e-8

int ttt_read_header (char *file, struct GRD_HEADER *h, double w, double e, double s, double n);
void ttt_read_ttt (char *file, struct GRD_HEADER *h, float *s, BOOLEAN read_short, int swabbing);
double ttt_great_circle_dist(double lon1, double lat1, double lon2, double lat2);
void *ttt_memory (void *prev_addr, size_t n, size_t size, char *progname);
double txt2fl (char *t, char neg);

int main (int argc, char **argv)
{
	int i, i_0, j_0, k, k_0, ix, iy, row_width, col_height, n_read, n_points = 0, rec = 0;
	int i1, i2, j, n_header_recs = 1, swabbing, ij, imin, imax, jmin, jmax;
 	
	BOOLEAN error = FALSE, flip = FALSE, header = FALSE, verbose = FALSE;
	BOOLEAN read_short = FALSE, have_depth = FALSE, have_ttt = FALSE;
	
	float *ttt, *z, *f, TTT_NaN;

	double value, xy[2], i_xinc, i_yinc, x, y, shortest_dist, hours_to_unit = 1.0;
	double search_depth = 0.0, z_value, xx, yy, dx, dy, d, x0, y0, z0, x1, y1, z1, NaN_dist, delay;
	
	char *grdfile = NULL, txt[2][32], stuff[BUFSIZ], line[BUFSIZ], zfile[BUFSIZ];
	
	FILE *fp = NULL;
	
	struct GRD_HEADER grd, Z;
	
	TTT_make_NaN (TTT_NaN);
	grdfile = CNULL;

 	for (i = 1; !error && i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			
				/* Common parameters */
			
				case 'H':
					header = TRUE;
					if (argv[i][2]) n_header_recs = atoi (&argv[i][2]);
					break;
				case 'I':	/* Read 2-byte int (in decaseconds) file instead of 4-byte floats (in hours) */

					read_short = TRUE;
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
					have_ttt = TRUE;
					break;

				case 'Z':	/* Sample station at sufficient water depth */
					have_depth = TRUE;
					sscanf (&argv[i][2], "%lf,%s", &search_depth, zfile);
					break;

				default:
					error = TRUE;
					break;
			}
		}
		else if ((fp = fopen (argv[i], "r")) == NULL) {
			fprintf (stderr, "ttt_search: Cannot open file %s\n", argv[i]);
			exit (EXIT_FAILURE);
		}
	}
	
	if (argc == 1 || error) {
		fprintf (stderr,"ttt_search %s - Sampling of a 2-D gridded travel time file at specified locations\n\n", TTT_VERSION);
		fprintf (stderr, "usage: ttt_search <xyfile> [-Z<depth>,<depth.i2>] [-G<grdfile>]  [-H[<n>]] [-I] [-V] [-h|m|s] [-:]\n");

		fprintf (stderr, "	<xyfile> is a multicolumn ASCII file with (lon,lat) in the first two columns\n");
		fprintf (stderr, "	-Z Determine point nearest to origin where water depth is <depth> m or deeper.\n");
		fprintf (stderr, "	   Give <depth.grd> is the name of the bathymetry grid used to calculate the travel times\n");
		fprintf (stderr, "	   Note <depth> is negative below the sea surface.  Use this option to see how far\n");
		fprintf (stderr, "	   an epicenter must move to be used in ttt.  If -G is used then we also determine delay\n");
		fprintf (stderr, "\n\tOPTIONS:\n");
		fprintf (stderr, "	-G <grdfile> is the name of the 2-D binary data set.  We will then report\n");
		fprintf (stderr, "	   the travel time delay involved when moving the point (see -Z)\n");
		fprintf (stderr, "	-H means <xyfile> has one or more (append <n>) header record\n");
		fprintf (stderr, "	-I Travel times in grdfile were stored as 2 byte ints with units of 10 sec [Default is 4-byte float in hours]\n");
		fprintf (stderr, "	-V for verbose; will report progress\n");
		fprintf (stderr, "	-h|m|s reports travel times in hours, minutes, or seconds [hours]\n");
		fprintf (stderr, "	-: <xyfile> contains lat,lon instead of lon,lat\n");
		exit (EXIT_FAILURE);
	}
	
	if ((have_ttt + have_depth) == 0) {
		fprintf (stderr, "ttt_search: Must specify at least one grid (ttt -G or depth -Z)\n");
		exit (EXIT_FAILURE);
	}
	
	if (fp == NULL) fp = stdin;

	if (have_ttt) {
		swabbing = ttt_read_header (grdfile, &grd, 0.0, 0.0, 0.0, 0.0);
	
		row_width = grd.nx + 2;
		col_height = grd.ny + 2;
	
		ttt = (float *) ttt_memory (VNULL, row_width * col_height, sizeof (float), "ttt_search");

		ttt_read_ttt (grdfile, &grd, ttt, read_short, swabbing);

		i_xinc = 1.0 / grd.x_inc;
		i_yinc = 1.0 / grd.y_inc;
	
		/* Set natural cubic spline BC's */
	
		for (i1 = 0, i2 = (col_height-1)*row_width; i1 < row_width; i1++, i2++) {
			ttt[i1] = (float)(2.0 * ttt[i1+row_width] - ttt[i1+2*row_width]);
			ttt[i2] = (float)(2.0 * ttt[i2-row_width] - ttt[i2-2*row_width]);
		}
		for (j = 1, i1 = row_width - 2; j < col_height-1; j++) {
			i1 = j * row_width;
			ttt[i1] = (float)(2.0 * ttt[i1+1] - ttt[i1+2]);
			i1 += row_width - 2;
			ttt[i1] = (float)(2.0 * ttt[i1-1] - ttt[i1-2]);
		}
	}
	if (have_depth) {
		swabbing = ttt_read_header (zfile, &Z, 0.0, 0.0, 0.0, 0.0);
		if (have_ttt && ! (grd.nx == Z.nx && grd.ny == Z.ny)) {
			fprintf (stderr, "ttt_search: Travel time grid and depth grid not of same size\n");
			exit (EXIT_FAILURE);
		}
		row_width = Z.nx + 2;
		col_height = Z.ny + 2;
	
		z = (float *) ttt_memory (VNULL, row_width * col_height, sizeof (float), "ttt_search");
		ttt_read_ttt (zfile, &Z, z, 2, swabbing);
		i_xinc = 1.0 / Z.x_inc;
		i_yinc = 1.0 / Z.y_inc;
	
		/* Set natural cubic spline BC's */
	
		for (i1 = 0, i2 = (col_height-1)*row_width; i1 < row_width; i1++, i2++) {
			z[i1] = (float)(2.0 * z[i1+row_width] - z[i1+2*row_width]);
			z[i2] = (float)(2.0 * z[i2-row_width] - z[i2-2*row_width]);
		}
		for (j = 1, i1 = row_width - 2; j < col_height-1; j++) {
			i1 = j * row_width;
			z[i1] = (float)(2.0 * z[i1+1] - z[i1+2]);
			i1 += row_width - 2;
			z[i1] = (float)(2.0 * z[i1-1] - z[i1-2]);
		}
	}
	if (!have_ttt) memcpy ((void *)&grd, (void *)&Z, sizeof (struct GRD_HEADER));
	
	if (header) {	/* First echo headers, if any */
		for (i = 0; i < n_header_recs - 1; i++) {
			fgets (line, BUFSIZ, fp);
			printf ("%s", line);
		}
		fgets (line, BUFSIZ, fp);
		line[strlen(line)-1] = 0;
		printf ("%s\tsample\n", line);
	}
	printf ("x0\ty0\tz0\tx1\ty1\tz1\txf\tyf\tzf\td0\td1\tdt\n");

	ix = (flip) ? 1 : 0;	iy = 1 - ix;		/* Set up which columns have x and y */
	while (fgets (line, BUFSIZ, fp)) {
		rec++;
		if (line[0] == '>' || line[0] == '#' || line[0] == '\0') continue;

		n_read = sscanf (line, "%s %s %[^\n]", txt[ix], txt[iy], stuff);
		if (n_read < 2) {
			fprintf (stderr, "ttt_search: Error reading coordinates from record # %d (skipped)\n", rec);
			continue;
		}
		xy[0] = txt2fl (txt[0], 'W');
		xy[1] = txt2fl (txt[1], 'S');

		/* Check that we are inside the grd area */
		
		if (xy[1] < grd.y_min || xy[1] > grd.y_max) continue;
		xy[0] -= 360.0;
		while (xy[0] < grd.x_min) xy[0] += 360.0;
		if (xy[0] > grd.x_max) continue;
		
		/* Get nearest node */
		
		i_0 = ttt_x_to_i (xy[0], grd.x_min, grd.x_inc, grd.xy_off, grd.nx);
		j_0 = ttt_y_to_j (xy[1], grd.y_min, grd.y_inc, grd.xy_off, grd.ny);
		k_0 = IJ(i_0,j_0,row_width,1);
		x0 = xy[0];	y0 = xy[1];	z0 = z[k_0];	/* Original coordinates */
		
		NaN_dist = 0.0;
		f = (have_ttt) ? ttt : z;
		if (TTT_is_fnan (f[k_0]) || z[k_0] >= 0.0) {	/* Must search for nearest non-NaN or ocean node (in either depth or ttt) */
			dx = grd.x_inc * cos (D2R * xy[1]);
			imin = MAX (0, i_0 - (int)ceil (RADIUS / dx));
			imax = MIN (grd.nx - 1, i_0 + (int)ceil (RADIUS / dx));
			jmin = MAX (0, j_0 - (int)ceil (RADIUS / grd.y_inc));
			jmax = MIN (grd.ny - 1, j_0 + (int)ceil (RADIUS / grd.y_inc));
                
			NaN_dist = 180.0;
			xx = yy = 0.0;
			for (j = jmin; j <= jmax; j++) {
				y = ttt_j_to_y (j, grd.y_min, grd.y_max, grd.y_inc, grd.xy_off, grd.ny);
				for (i = imin; i <= imax; i++) {
					ij = IJ(i,j,row_width,1);
					if (TTT_is_fnan (f[ij]) || z[ij] >= 0.0) continue;
					x = ttt_i_to_x (i, grd.x_min, grd.x_max, grd.x_inc, grd.xy_off, grd.nx);
					d = ttt_great_circle_dist (xy[0], xy[1], x, y);
					if (d < NaN_dist) {
						k_0 = ij;
						NaN_dist = d;
						xx = x;
						yy = y;
					}
				}
			}
                
			if (NaN_dist == 180.0) {           
				fprintf (stderr, "\nttt_search: Station location more than %g degrees from the ocean - check!\n", RADIUS);
				fprintf (stderr, "[%s]\n", line);
				continue;
			}
			xy[0] = xx;
			xy[1] = yy;
			NaN_dist *= DEGREE_2_KM;
		}
		/* Reset nearest node parameters */
		
		i_0 = ttt_x_to_i (xy[0], grd.x_min, grd.x_inc, grd.xy_off, grd.nx);
		j_0 = ttt_y_to_j (xy[1], grd.y_min, grd.y_inc, grd.xy_off, grd.ny);
		x1 = xy[0];	y1 = xy[1];	z1 = z[k_0];		/* Nearnest node over water */
	
		/* Now xy is at the nearest water node to the epicenter */
		
		/* Must search for nearest node at specified minimum water depth */
               
		dx = grd.x_inc * cos (D2R * xy[1]);
		imin = MAX (0, i_0 - (int)ceil (RADIUS / dx));
		imax = MIN (grd.nx - 1, i_0 + (int)ceil (RADIUS / dx));
		jmin = MAX (0, j_0 - (int)ceil (RADIUS / grd.y_inc));
		jmax = MIN (grd.ny - 1, j_0 + (int)ceil (RADIUS / grd.y_inc));
                
		shortest_dist = 180.0;
		xx = yy = 0.0;
		k = k_0;
		for (j = jmin; j <= jmax; j++) {
			y = ttt_j_to_y (j, grd.y_min, grd.y_max, grd.y_inc, grd.xy_off, grd.ny);
			for (i = imin; i <= imax; i++) {
				ij = IJ(i,j,row_width,1);
				if (have_ttt && TTT_is_fnan (ttt[ij])) continue;
				x = ttt_i_to_x (i, grd.x_min, grd.x_max, grd.x_inc, grd.xy_off, grd.nx);
				d = ttt_great_circle_dist (xy[0], xy[1], x, y);
				if (TTT_is_fnan (z[ij])|| z[ij] >= search_depth) continue;	/* Too shallow */
				if (d < shortest_dist) {
					k = ij;
					shortest_dist = d;
					xx = x;
					yy = y;
				}
			}
		}
               
		if (shortest_dist == 180.0) {           
			fprintf (stderr, "\nttt_search: Station location more than %g degrees from the ocean - check!\n", RADIUS);
			fprintf (stderr, "[%s]\n", line);
			continue;
		}
		xy[0] = xx;
		xy[1] = yy;
		shortest_dist *= DEGREE_2_KM;
		
		/* Estimate delay */
		
		delay = (have_ttt) ? hours_to_unit * (ttt[k] - ttt[k_0]) : TTT_NaN;
		
		printf ("%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t\%g\t%g\t%g\n", x0, y0, z0, x1, y1, z1, xy[0], xy[1], z[k], NaN_dist, shortest_dist, delay);
		
		n_points++;
	}
	fclose(fp);
	
	if (verbose) fprintf (stderr, "ttt_search: Sampled %d points from grid %s (%d x %d)\n",
		n_points, grdfile, grd.nx, grd.ny);
	
	if (have_ttt) free ((void *)ttt);
	if (have_depth) free ((void *)z);
	
	exit (EXIT_SUCCESS);
}

/* Reads ttt from float (GMT format # 1 (bf)) */

void ttt_read_ttt (char *file, struct GRD_HEADER *h, float *s, BOOLEAN read_short, int swabbing)
{
	int i, j, k, row_width;
	unsigned int *i_ptr;
	short int *is;
	float TTT_NaN, scale;
	FILE *fp;

	if ((fp = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "ttt: Error creating file %s\n", file);
		exit (17);
	}

	if (fseek (fp, (long)(3*sizeof (int) + sizeof (struct GRD_HEADER) - ((long)&h->x_min - (long)&h->nx)), SEEK_SET)) {
		fprintf (stderr, "ttt: Error seeking past header from file %s\n", file);
		exit (18);
	}
	h->xy_off = 0.5 * h->node_offset;

	row_width = h->nx + 2;

	scale = (read_short == 2) ? 1.0 : (1.0 / 360.0);	/* Depth in meters or tt in decaseconds (-> hours) */
	if (read_short) {	/* Time in deca-seconds (or depths) */
		TTT_make_NaN (TTT_NaN);
	 	is = (short int *) ttt_memory (VNULL, h->nx, sizeof (short int), "ttt_search");
		for (j = 0; j < h->ny; j++) {
			k = (j + 1) * row_width + 1;
			if (fread ((void *)is, sizeof (short int), h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error reading file %s\n", file);
				exit (18);
			}
			for (i = 0; i < h->nx; i++, k++) {
				if (swabbing) is[i] = TTT_swab2 (is[i]);
				s[k] = (is[i] == SHRT_MAX || is[i] == SHRT_MIN) ? TTT_NaN : (float)(is[i] * scale);
			}
		}
		free ((void *)is);
	}
	else {	/* Time in hours */
		for (j = 0; j < h->ny; j++) {
			k = (j + 1) * row_width + 1;
			if (fread ((void *)&s[k], sizeof (float), h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error reading file %s\n", file);
				exit (18);
			}
			if (swabbing) {
				i_ptr = (unsigned int *)&s[k];
				*i_ptr = TTT_swab4 (*i_ptr);
			}
		}
	}
	fclose (fp);
}

#include "ttt_subs.c"
