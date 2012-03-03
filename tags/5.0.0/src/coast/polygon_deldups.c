/*
 *	$Id$
 */
#include "wvs.h"

#define NDEL		763

struct BLOB {
	struct GMT3_POLY h;
	int start;
	unsigned char reverse;
	int x0, y0;
} blob[N_POLY];

struct Q {
	int dup, orig;
} q[NDEL];

int *lon, *lat;
struct LONGPAIR *pp;
int non_zero_winding2 (int xp, int yp, int *x, int *y, int n_path);

int main (int argc, char **argv) {
	int i, k, n_id, pos, i1, id, id1, id2, intest, n;
	int ix0, off;
	double x0, west1, west2, east1, east2;
	FILE *fp, *fp2, *fpx;
	struct LONGPAIR p;
	char line[80];
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_deldups final_x_polygons.b final_dbase.b\n");
		exit (-1);
	}

	fpx = fopen ("to_remove.lis", "r");
	for (i = 0; i < NDEL; i++) {
		fgets (line, 512,fpx);
		sscanf (line, "%d %d", &q[i].dup, &q[i].orig);
	}
	fclose (fpx);
	
	fprintf (stderr, "Read headers\n");
	fp = fopen (argv[1], "r");
	
	/* Reset ids as we go along */
	
	n_id = pos = 0;
	while (pol_readheader (&blob[n_id].h, fp) == 1) {
		pos += sizeof (struct GMT3_POLY);
		blob[n_id].start = pos;
		if (pol_fread (&p, 1, fp) != 1) {
			fprintf(stderr,"polygon_deldups: Error reading file.\n");
			exit(-1);
		}
		blob[n_id].x0 = p.x;	/* Pick any point on the polygon */
		blob[n_id].y0 = p.y;
		blob[n_id].reverse = FALSE;
		fseek (fp, (blob[n_id].h.n - 1) * sizeof(struct LONGPAIR), 1);
		pos += blob[n_id].h.n * sizeof(struct LONGPAIR);
		blob[n_id].h.id = n_id;
		n_id++;
	}
	
	
	lon = (int *) GMT_memory (CNULL, N_LONGEST, sizeof(int), "polygon_deldups");
	lat = (int *) GMT_memory (CNULL, N_LONGEST, sizeof(int), "polygon_deldups");
	
	fprintf (stderr, "Start inside testing\n\n");
	
	for (i1 = 0; i1 < NDEL; i1++) {	/* For all anchor polygons */
	
		id1 = q[i1].dup;
		fprintf (stderr, "Polygon %d\n", id1);

		fseek (fp, (long)blob[id1].start, 0);
		for (k = 0; k < blob[id1].h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_deldups: Error reading file.\n");
				exit(-1);
			}
			if ((blob[id1].h.greenwich & 1) && p.x > blob[id1].h.datelon) p.x -= M360;
			lon[k] = p.x;
			lat[k] = p.y;
		}
		n = blob[id1].h.n;
		
		west1 = blob[id1].h.west;	east1 = blob[id1].h.east;
		if (blob[id1].h.greenwich & 1) {
			west1 += 360.0;
			east1 += 360.0;
		}
		
		for (id2 = 500; id2 < n_id; id2++) {
			
			if (id1 == id2) continue;
			if (blob[id2].h.source == -1) continue;	/* Marked for deletion */
			
			if (q[i1].orig == id2) continue;
			
			if (blob[id2].h.level < blob[id1].h.level) continue;
			
			/* First perform simple tests based on min/max coordinates */
			
			if ( (blob[id2].h.south > blob[id1].h.north) || (blob[id2].h.north < blob[id1].h.south)) continue;
			
			west2 = blob[id2].h.west;	east2 = blob[id2].h.east;
			off = 0.0;
			if (blob[id2].h.greenwich & 1) {
				west2 += 360.0;
				east2 += 360.0;
				off = M360;
			}
			while (east2 < west1) {
				east2 += 360.0;
				west2 += 360.0;
				off += M360;
			}
			if ((west2 > east1) || (east2 < west1)) continue;
			
			/* Must compare with polygon boundaries */
			
			/* Here we need to perform complete inside test */
			
			x0 = blob[id2].x0 * 1.0e-6;
			if (id1 == 0 && x0 > blob[id1].h.east)
				x0 -= 360.0;
			else if (id1 > 0 && (blob[id1].h.greenwich & 1) && x0 > 180.0)
				x0 -= 360.0;
			else if (!(blob[id1].h.greenwich & 1) && x0 < 0.0)
				x0 += 360.0;
			
			if (!(lon[0] == lon[n-1] && lat[0] == lat[n-1])) {
				lon[n] = lon[0];
				lat[n] = lat[0];
				n++;
			}
			ix0 = x0 * MILL;
			intest = non_zero_winding2 (ix0, blob[id2].y0, lon, lat, n);
			
			if (!intest) continue;
			
			if (intest == 1) fprintf (stderr, "%d is on border of %d\n", id2, id1);
			
			blob[id2].h.level--;
		}
		
		if (blob[q[i1].orig].h.level > blob[q[i1].dup].h.level) {
			blob[q[i1].orig].h.level--;
			blob[q[i1].orig].reverse = TRUE;
			
		}
		blob[q[i1].dup].h.source = -1;
	}
	
	free (lon);
	free (lat);

	fprintf (stderr, "Write out new data base\n");

	/* Write new base */
	
	pp = (struct LONGPAIR *) GMT_memory (CNULL, 1450000, sizeof(struct LONGPAIR), "polygon_deldups");
	
	fp2 = fopen (argv[2], "w");
	for (id = i = 0; id < n_id; id++) {
		if (blob[id].h.source == -1) continue;
		blob[id].h.id = i;
		pol_writeheader (&blob[id].h, fp2);
		fseek (fp, (long)blob[id].start, 0);
		if (pol_fread (pp, blob[id].h.n, fp) != blob[id].h.n) {
			fprintf(stderr,"polygon_deldups: Error reading file.\n");
			exit(-1);
		}
		if (blob[id].reverse) {	/* Reverse polygon */
			for (i = blob[id].h.n - 1; i >= 0; i--) pol_fwrite (&pp[i], 1, fp2);
		}
		else
			pol_fwrite (pp, blob[id].h.n, fp2);
		i++;
	}
	
	fclose (fp);
	fclose (fp2);
	
	free (pp);
	
	exit (0);
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
