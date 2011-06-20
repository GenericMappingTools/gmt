/*
 *	$Id: poly_misc_subs.c,v 1.8 2011-06-20 02:02:39 guru Exp $
 *
 * Contains misc functions used by polygon* executables
 */
 
#define COASTLIB 1
#include "wvs.h"

void crude_init (double *X[N_CONTINENTS][2], double *Y[N_CONTINENTS][2], int N[N_CONTINENTS][2])
{
	/* Reads the five crude continent outlines into X,Y */
	int c, i, t;
	char *dir, line[GMT_BUFSIZ], *cont[N_CONTINENTS] = {"eur", "afr", "nam", "sam", "ant", "aus"}, *type[2] = {"out", "in"};
	double *x, *y;
	FILE *fp;
	
	if ((dir = getenv("GMTHOME")) == CNULL) {	/* GMTHOME was not set */
		fprintf (stderr, "coast: Please set GMTHOME first\n");
		exit (EXIT_FAILURE);
	}
#ifdef WIN32
	DOS_path_fix (dir);
#endif
	for (c = 0; c < N_CONTINENTS; c++) {	/* For each continent */
		for (t = 0; t < 2; t++) {	/* For both outside and inside polygons */
			sprintf (line, "%s/src/coast/crude_%s_%s.txt", dir, cont[c], type[t]);
			if ((fp = fopen (line, "r")) == NULL) {
				fprintf (stderr, "coast: Cannot open %s\n", line);
				exit (EXIT_FAILURE);
			}
			x = (double *) GMT_memory (NULL, 100, sizeof (double), "coast");
			y = (double *) GMT_memory (NULL, 100, sizeof (double), "coast");
			i = 0;
			while (fgets (line, GMT_BUFSIZ, fp)) {
				if (line[0] == '#') continue;
				sscanf (line, "%lf %lf", &x[i], &y[i]);
				i++;
			}
			fclose (fp);
			x = (double *) GMT_memory ((void *)x, i, sizeof (double), "coast");
			y = (double *) GMT_memory ((void *)y, i, sizeof (double), "coast");
			N[c][t] = i;
			X[c][t] = x;
			Y[c][t] = y;
		}
	}
}

void crude_free (double *X[N_CONTINENTS][2], double *Y[N_CONTINENTS][2], int N[N_CONTINENTS][2])
{
	int c, t;
	for (c = 0; c < N_CONTINENTS; c++) {	/* For each continent */
		for (t = 0; t < 2; t++) {	/* For both outside and inside polygons */
			GMT_free ((void *)X[c][t]);
			GMT_free ((void *)Y[c][t]);
		}
	}
}

void crude_init_int (int *IX[N_CONTINENTS][2], int *IY[N_CONTINENTS][2], int N[N_CONTINENTS][2], int scale)
{
	/* Reads the five crude continent outlines into X,Y integer arrays */
	int c, i, t, *ix, *iy;
	double *X[N_CONTINENTS][2], *Y[N_CONTINENTS][2];
	
	crude_init (X, Y, N);
	for (c = 0; c < N_CONTINENTS; c++) {	/* For each continent */
		for (t = 0; t < 2; t++) {	/* For both outside and inside polygons */
			ix = (int *) GMT_memory (NULL, N[c][t], sizeof (int), "coast");
			iy = (int *) GMT_memory (NULL, N[c][t], sizeof (int), "coast");
			for (i = 0; i < N[c][t]; i++) {
				ix[i] = irint (X[c][t][i]) * scale;
				iy[i] = irint (Y[c][t][i]) * scale;
			}
			IX[c][t] = ix;
			IY[c][t] = iy;
		}
	}
	crude_free (X, Y, N);
}

void crude_free_int (int *IX[N_CONTINENTS][2], int *IY[N_CONTINENTS][2], int N[N_CONTINENTS][2])
{
	int c, t;
	for (c = 0; c < 5; c++) {	/* For each continent */
		for (t = 0; t < 2; t++) {	/* For both outside and inside polygons */
			GMT_free ((void *)IX[c][t]);
			GMT_free ((void *)IY[c][t]);
		}
	}
}

void area_init ()
{	/* Initializes GMT projection parameters to the -JA settings */
	GMT->current.setting.proj_ellipsoid = GMT_N_ELLIPSOIDS-1;
	GMT->current.proj.projection = GMT_LAMB_AZ_EQ;
	GMT->current.proj.unit = GMT_M;
	GMT->current.proj.pars[3] = 39.3700787401574814;
	GMT->current.proj.rotated = FALSE;
	GMT->current.setting.map_line_step = 1.0e7;	/* To avoid nlon/nlat being huge */
	GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;
	GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
}

double area_size (double x[], double y[], int n, int *sign)
{
	int i;
	double west, east, south, north, lon, lat, xx, yy, size, ix, iy;
	double area (double x[], double y[], int n);
	
	west = south = 1.0e100;
	east = north = -1.0e100;
	lon = lat = 0.0;
	
	/* Estimate 'average' position */
	
	for (i = 0; i < n; i++) {
		lon += x[i];
		lat += y[i];
		west = MIN (west, x[i]);
		east = MAX (east, x[i]);
		south = MIN (south, y[i]);
		north = MAX (north, y[i]);
	}
	lon /= n;
	lat /= n;
		
	GMT->current.proj.pars[0] = lon;
	GMT->current.proj.pars[1] = lat;
	GMT_err_fail (GMT_map_setup (west, east, south, north), "");
	
	ix = 1.0 / GMT->current.proj.scale[GMT_X];
	iy = 1.0 / GMT->current.proj.scale[GMT_Y];
	
	for (i = 0; i < n; i++) {
		GMT_geo_to_xy (x[i], y[i], &xx, &yy);
		x[i] = (xx - GMT->current.proj.origin[GMT_X]) * ix;
		y[i] = (yy - GMT->current.proj.origin[GMT_Y]) * iy;
	}
	
	size = area (x, y, n);
	*sign = (size < 0.0) ? -1 : 1;
	return (fabs (size));
}

double area (double x[], double y[], int n)
{
	int i;
	double area, xold, yold;
	
	/* Sign will be +ve if polygon is CW, negative if CCW */
	
	area = yold = 0.0;
	xold = x[n-1];
	yold = y[n-1];
	
	for (i = 0; i < n; i++) {
		area += (xold - x[i]) * (yold + y[i]);
		xold = x[i];
		yold = y[i];
	}
	return (0.5 * area);
}

int non_zero_winding2 (int xp, int yp, int *x, int *y, int n_path)
{
	/* Routine returns (2) if (xp,yp) is inside the
	   polygon x[n_path], y[n_path], (0) if outside,
	   and (1) if exactly on the path edge.
	   Uses non-zero winding rule in Adobe PostScript
	   Language reference manual, section 4.6 on Painting.
	   Path should have been closed first, so that
	   x[n_path-1] = x[0], and y[n_path-1] = y[0].

	   This is version 2, trying to kill a bug
	   in above routine: If point is on X edge,
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
	
	int	i, j, k, jend, crossing_count, above;
	double	y_sect;

	above = FALSE;
	crossing_count = 0;

	/* First make sure first point in path is not a special case */
	j = jend = n_path - 1;
	if (x[j] == xp) {
		/* Trouble already.  We might get lucky */
		if (y[j] == yp) return(1);

		/* Go backward down the polygon until x[i] != xp */
		if (y[j] > yp) above = TRUE;
		i = j - 1;
		while (x[i] == xp && i > 0) {
			if (y[i] == yp) return (1);
			if (!(above) && y[i] > yp) above = TRUE;
			i--;
		}

		/* Now if i == 0 polygon is degenerate line x=xp;
		   since we know xp,yp is inside bounding box,
		   it must be on edge */
		if (i == 0) return(1);

		/* Now we want to mark this as the end, for later */
		jend = i;

		/* Now if (j-i)>1 there are some segments the point could be exactly on */
		for (k = i+1; k < j; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}


		/* Now we have arrived where i is > 0 and < n_path-1, and x[i] != xp.
			We have been using j = n_path-1.  Now we need to move j forward 
			from the origin */
		j = 1;
		while (x[j] == xp) {
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}

		/* Now at the worst, j == jstop, and we have a polygon with only 1 vertex
			not at x = xp.  But now it doesn't matter, that would end us at
			the main while below.  Again, if j>=2 there are some segments to check */
		for (k = 0; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}


		/* Finally, we have found an i and j with points != xp.  If (above) we may have crossed the ray */
		if (above && x[i] < xp && x[j] > xp) 
			crossing_count++;
		else if (above && x[i] > xp && x[j] < xp) 
			crossing_count--;

		/* End nightmare scenario for x[0] == xp.  */
	}

	else {
		/* Get here when x[0] != xp */
		i = 0;
		j = 1;
		while (x[j] == xp && j < jend) {
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* Again, if j==jend, (i.e., 0) then we have a polygon with only 1 vertex
			not on xp and we will branch out below.  */

		/* if ((j-i)>2) the point could be on intermediate segments */
		for (k = i+1; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}

		/* Now we have x[i] != xp and x[j] != xp.
			If (above) and x[i] and x[j] on opposite sides, we are certain to have crossed the ray.
			If not (above) and (j-i)>1, then we have not crossed it.
			If not (above) and j-i == 1, then we have to check the intersection point.  */

		if (x[i] < xp && x[j] > xp) {
			if (above) 
				crossing_count++;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( ((double)(xp - x[i])) / ((double)(x[j] - x[i])) );
				if (rint (y_sect) == yp) return (1);
				if (y_sect > yp) crossing_count++;
			}
		}
		if (x[i] > xp && x[j] < xp) {
			if (above) 
				crossing_count--;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( ((double)(xp - x[i])) / ((double)(x[j] - x[i])) );
				if (rint (y_sect) == yp) return (1);
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
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* if ((j-i)>2) the point could be on intermediate segments */
		for (k = i+1; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}

		/* Now we have x[i] != xp and x[j] != xp.
			If (above) and x[i] and x[j] on opposite sides, we are certain to have crossed the ray.
			If not (above) and (j-i)>1, then we have not crossed it.
			If not (above) and j-i == 1, then we have to check the intersection point.  */

		if (x[i] < xp && x[j] > xp) {
			if (above) 
				crossing_count++;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( ((double)(xp - x[i])) / ((double)(x[j] - x[i])) );
				if (rint (y_sect) == yp) return (1);
				if (y_sect > yp) crossing_count++;
			}
		}
		if (x[i] > xp && x[j] < xp) {
			if (above) 
				crossing_count--;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( ((double)(xp - x[i])) / ((double)(x[j] - x[i])) );
				if (rint (y_sect) == yp) return (1);
				if (y_sect > yp) crossing_count--;
			}
		}

		/* That's it for this piece.  Advance i */

		i = j;
	}

	/* End of MAIN WHILE.  Get here when we have gone all around without landing on edge.  */

	if (crossing_count)
		return(2);
	else
		return(0);
}

int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY *hj, double *shift)
{	/* Returns TRUE of the two rectangular areas do not overlap.
	 * Also sets shift to -360,0,+360 as the amount to adjust longitudes */
	double w, e;

	if (hi->north < hj->south || hi->south > hj->north) return (TRUE);

	w = hj->west - 360.0;	e = hj->east - 360.0;
	*shift = -360.0;
	while (e < hi->west) e += 360.0, w += 360.0, (*shift) += 360.0;
	if (w > hi->east) return (TRUE);
	return (FALSE);
}

void xy2rtheta (double *lon, double *lat)
{	/* Just convert lon lat to a polar coordinate system */
	double slon, clon, r0;
	sincosd (*lon, &slon, &clon);
	r0 = 90.0 + (*lat);
	*lon = clon * r0;
	*lat = slon * r0;
}

void xy2rtheta_int (int *ilon, int *ilat)
{	/* Just convert lon lat to a polar coordinate system */
	double lon, lat;
	lon = (*ilon) * 1e-6;
	lat = (*ilat) * 1e-6;
	xy2rtheta (&lon, &lat);
	*ilon = irint (lon * MILL);
	*ilat = irint (lat * MILL);
}

void rtheta2xy (double *lon, double *lat)
{	/* Reverse the r-theta projection */
	double r, theta;
	r = hypot (*lon, *lat);
	theta = atan2d (*lat, *lon);
	if (theta < 0.0) theta += 360.0;
	*lon = theta;
	*lat = r - 90.0;
}
