/*
 *	$Id: polygon_setnodes.c,v 1.8 2009-06-05 00:25:12 guru Exp $
 */
/* polygon_setnodes is run on the final polygon file when all polygons
 * have had their level determined.  This program will determine
 * the level of each node on a bin x bin global grid, i.e., the level of
 * the innermost polygon that contains the node.  This is needed since
 * many blocks will be completely outside all polygons (mid ocean) or
 * completely inside a large enough polygon that no crossings with the
 * block borders occur (mid continent, middle of big lake etc).  We
 * store the node levels in a grdfile for convenience.
 */
 
#include "wvs.h"


struct BLOB {
	struct GMT3_POLY h;
	int start;
} blob[N_POLY];

int *lon, *lat;
int ANTARCTICA;	/* Current id of Antarctica in sorted list */
int non_zero_winding2 (int xp, int yp, int *x, int *y, int n_path);

struct GRD_HEADER grdh;
float *grd;
int main (int argc, char **argv)
{
	int i, j, k, n_id, pos, n_nodes, id, intest, n, nx_minus_1, ix0, off, iy0, slow = 0;
	int iblon, iblat, ij, i0, i1, j0, j1, ii, full;
	double west, east, blon, blat, x0, y0, w, iw, slon, clon, r0;
	FILE *fp;
	struct LONGPAIR p;
	
	if (argc != 4) {
		fprintf (stderr, "usage: polygon_setnodes final_dbase.b bin_width nodegrdfile\n");
		exit (EXIT_FAILURE);
	}

	argc = GMT_begin (argc, argv);
	
	/* Grdfile will be grid registered so lon = 0 and 360 will be separate nodes.
	   We must make sure we update both lons */

	w = atof (argv[2]);
	iw = 1.0 / w;
	GMT_grd_init (&grdh, argc, argv, FALSE);
	grdh.nx = irint (360.0 * iw) + 1;
	grdh.ny = irint (180.0 * iw) + 1;
	n_nodes = grdh.nx * grdh.ny;
	grd = (float *) GMT_memory (VNULL, (size_t)n_nodes, sizeof(float), "polygon_setnodes");
        grdh.x_min = 0.0;
        grdh.x_max = 360.0;
        grdh.y_min = -90.0;
        grdh.y_max = 90.0;
        grdh.x_inc = grdh.y_inc = w;
        grdh.node_offset = 0;
        grdh.z_scale_factor = 1.0;
        grdh.z_add_offset = 0.0;
        strcpy (grdh.x_units, "Longitude");
        strcpy (grdh.y_units, "Latitude");
        strcpy (grdh.z_units, "Polygon Level");
        sprintf (grdh.title, "Polygon Levels in file %s", argv[1]);
        
        nx_minus_1 = grdh.nx - 1;
        
	fprintf (stderr, "Read headers\n");
	fp = fopen (argv[1], "r");
	
	/* Reset ids as we go along */
	
	n_id = pos = 0;
	while (pol_readheader (&blob[n_id].h, fp) == 1) {
		if (fabs (blob[n_id].h.east - blob[n_id].h.west) == 360.0) ANTARCTICA = n_id;
		pos += sizeof (struct GMT3_POLY);
		blob[n_id].start = pos;
		if (pol_fread (&p, 1, fp) != 1) {
			fprintf(stderr,"polygon_setnodes:  ERROR  reading file.\n");
			exit (EXIT_FAILURE);
		}
		fseek (fp, (long) (blob[n_id].h.n - 1) * sizeof(struct LONGPAIR), 1);
		pos += blob[n_id].h.n * sizeof(struct LONGPAIR);
		blob[n_id].h.id = n_id;
		n_id++;
	}

	lon = (int *) GMT_memory (VNULL, N_LONGEST, sizeof(int), "polygon_setnodes");
	lat = (int *) GMT_memory (VNULL, N_LONGEST, sizeof(int), "polygon_setnodes");
	full = (blob[0].h.n > 1000000);	/* TRUE for full resolution */
	full = FALSE;
	fprintf (stderr, "Start inside testing (Antarctica == %d)\n\n", ANTARCTICA);
	
	for (id = 0; id < n_id; id++) {	/* For all anchor polygons */
	
		slow = (w <= 2.0 && id < 3);
		
		if (full && id == 2) {	/* deallocate some space */
			lon = (int *) GMT_memory ((void *)lon, blob[id].h.n+5, sizeof(int), "polygon_setnodes");
			lat = (int *) GMT_memory ((void *)lat, blob[id].h.n+5, sizeof(int), "polygon_setnodes");
		}
		
		if (full && id == 3) fprintf (stderr, "\n");
		
		if (!slow) fprintf (stderr, "P = %d\r", id);

		fseek (fp, (long)blob[id].start, 0);
		for (k = 0; k < blob[id].h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_setnodes:  ERROR  reading file.\n");
				exit (EXIT_FAILURE);
			}
			if (blob[id].h.greenwich && p.x > blob[id].h.datelon) p.x -= M360;
			lon[k] = p.x;
			lat[k] = p.y;
		}
		n = blob[id].h.n;
		
		west = blob[id].h.west;	east = blob[id].h.east;
		if (blob[id].h.greenwich) {
			west += 360.0;
			east += 360.0;
		}
		
		if (id == ANTARCTICA) {	/* Antarctica : Switch to polar coordinates r, theta */
			for (i = 0; i < n; i++) {
				sincosd (1e-6 * lon[i], &slon, &clon);
				r0 = 90.0 + (1e-6 * lat[i]);
				x0 = r0 * clon;
				y0 = r0 * slon;
				lon[i] = irint (x0 * 1e6);
				lat[i] = irint (y0 * 1e6);
			}
		}

		j0 = ceil ((90.0 - blob[id].h.north) * iw);
		j1 = floor ((90.0 - blob[id].h.south) * iw);
		i0 = ceil (blob[id].h.west * iw);
		i1 = floor (blob[id].h.east * iw);
		for (j = j0; j <= j1; j++) {
			blat = 90.0 - j * grdh.y_inc;;
			iblat = blat * MILL;
			for (i = i0; i <= i1; i++) {
				ii = (i < 0) ? i + nx_minus_1 : i;
				ij = j * grdh.nx + ii;
			
				blon = i * grdh.x_inc;
				iblon = blon * MILL;
			
				/* First perform simple tests based on min/max coordinates */
			
				if ( (blat > blob[id].h.north) || blat < blob[id].h.south) continue;
			
				off = 0;
				ix0 = blon;
				while (ix0 < west) {
					ix0 += 360;
					off += M360;
				}
				if (ix0 > east) continue;
			
				/* Must compare with polygon boundaries */
			
				if (full && id == 0) {	/* Eurasia, first do quick test */
					intest = non_zero_winding2 (iblon + off, iblat, ieur_o[0], ieur_o[1], N_EUR_O);
					if (!intest) continue;
					
					/* So point is inside crude outside. Now check if it is inside crude inside */
				
					intest = non_zero_winding2 (iblon + off, iblat, ieur_i[0], ieur_i[1], N_EUR_I);

					if (!intest) intest = non_zero_winding2 (iblon + off, iblat, iafr_i[0], iafr_i[1], N_AFR_I);

					if (intest == 2) {	/* way inside, set levels */
						if (grd[ij] < blob[id].h.level) grd[ij] = blob[id].h.level;
						if (i == 0) grd[ij+nx_minus_1] = grd[ij];
						if (i == nx_minus_1) grd[ij-nx_minus_1] = grd[ij];
						continue;
					}
					/* If not, we fall down to the _real_ test */
				}
				else if (full && id == 1) {	/* Americas, first do quick test */
					intest = non_zero_winding2 (iblon + off, iblat, iam_o[0], iam_o[1], N_AM_O);
					if (!intest) continue;
				
					/* So point is inside crude outside. Now check if it is inside crude inside */
				
					intest = non_zero_winding2 (iblon + off, iblat, isam_i[0], isam_i[1], N_SAM_I);

					if (!intest) intest = non_zero_winding2 (iblon + off, iblat, inam_i[0], inam_i[1], N_NAM_I);

					if (intest == 2) {	/* way inside, set levels */
						if (grd[ij] < blob[id].h.level) grd[ij] = blob[id].h.level;
						if (i == 0) grd[ij+nx_minus_1] = grd[ij];
						if (i == nx_minus_1) grd[ij-nx_minus_1] = grd[ij];
						continue;
					}
					/* If not, we fall down to the _real_ test */
				}
				else if (full && id == 3) {	/* Austraila, first do quick test */
					intest = non_zero_winding2 (iblon + off, iblat, iaus_o[0], iaus_o[1], N_AUS_O);
					if (!intest) continue;
				
					/* So point is inside crude outside. Now check if it is inside crude inside */
				
					intest = non_zero_winding2 (iblon + off, iblat, iaus_i[0], iaus_i[1], N_AUS_I);

					if (intest == 2) {	/* way inside, set levels */
						if (grd[ij] < blob[id].h.level) grd[ij] = blob[id].h.level;
						if (i == 0) grd[ij+nx_minus_1] = grd[ij];
						if (i == nx_minus_1) grd[ij-nx_minus_1] = grd[ij];
						continue;
					}
					/* If not, we fall down to the _real_ test */
				}
			
				/* Here we need to perform complete inside test */
			
				if (!(lon[0] == lon[n-1] && lat[0] == lat[n-1])) {
					lon[n] = lon[0];
					lat[n] = lat[0];
					n++;
				}

				if (id == ANTARCTICA) {
					sincosd (blon, &slon, &clon);
					r0 = 90.0 + blat;
					x0 = r0 * clon;
					y0 = r0 * slon;
					ix0 = irint (x0 * MILL);
					iy0 = irint (y0 * MILL);
				}
				else {
					x0 = blon;
					if (id == 0 && x0 > blob[id].h.east)
						x0 -= 360.0;
					else if (id > 0 && blob[id].h.greenwich && x0 > 180.0)
						x0 -= 360.0;
					else if (!blob[id].h.greenwich && x0 < 0.0)
						x0 += 360.0;
					ix0 = x0 * MILL;
					iy0 = iblat;
				}
			
				intest = non_zero_winding2 (ix0, iy0, lon, lat, n);
			
				if (!intest) {	

					if (slow) fprintf (stderr, "P = %6d O = %3d/%3d\r", id, (int)blon, (int)blat);
					continue;
				}
			
				if (full && slow) fprintf (stderr, "P = %6d I = %3d/%3d\r", id, (int)blon, (int)blat);

				/* Inside or on edge is inside here */
			
				if (irint ((double)grd[ij]) < blob[id].h.level) {
					grd[ij] = (double)blob[id].h.level;
					if (i == 0) grd[ij+nx_minus_1] = grd[ij];
					if (i == nx_minus_1) grd[ij-nx_minus_1] = grd[ij];
				}
				
			}
		}
	}
	
	fprintf (stderr, "\n");
	
	fclose (fp);
	free ((void *)lon);
	free ((void *)lat);

	GMT_err_fail (GMT_write_grd (argv[3], &grdh, grd, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE), argv[3]);
	
	exit (EXIT_SUCCESS);
}	

int non_zero_winding2 (int xp, int yp, int *x, int *y, int n_path)
{
	/* Based on GMT_non_zero_winding, but uses integer arguments */
	
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
	
	int	i, j, k, jend, crossing_count, above;
	double	y_sect;

	above = FALSE;
	crossing_count = 0;

	/* First make sure first point in path is not a special case:  */
	j = jend = n_path - 1;
	if (x[j] == xp) {
		/* Trouble already.  We might get lucky:  */
		if (y[j] == yp) return(1);

		/* Go backward down the polygon until x[i] != xp:  */
		if (y[j] > yp) above = TRUE;
		i = j - 1;
		while (x[i] == xp && i > 0) {
			if (y[i] == yp) return (1);
			if (!(above) && y[i] > yp) above = TRUE;
			i--;
		}

		/* Now if i == 0 polygon is degenerate line x=xp;
		   since we know xp,yp is inside bounding box,
		   it must be on edge:  */
		if (i == 0) return(1);

		/* Now we want to mark this as the end, for later:  */
		jend = i;

		/* Now if (j-i)>1 there are some segments the point could be exactly on:  */
		for (k = i+1; k < j; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}


		/* Now we have arrived where i is > 0 and < n_path-1, and x[i] != xp.
			We have been using j = n_path-1.  Now we need to move j forward 
			from the origin:  */
		j = 1;
		while (x[j] == xp) {
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}

		/* Now at the worst, j == jstop, and we have a polygon with only 1 vertex
			not at x = xp.  But now it doesn't matter, that would end us at
			the main while below.  Again, if j>=2 there are some segments to check:  */
		for (k = 0; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
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
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* Again, if j==jend, (i.e., 0) then we have a polygon with only 1 vertex
			not on xp and we will branch out below.  */

		/* if ((j-i)>2) the point could be on intermediate segments:  */
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
		/* if ((j-i)>2) the point could be on intermediate segments:  */
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

		/* That's it for this piece.  Advance i:  */

		i = j;
	}

	/* End of MAIN WHILE.  Get here when we have gone all around without landing on edge.  */

	if (crossing_count)
		return(2);
	else
		return(0);
}
