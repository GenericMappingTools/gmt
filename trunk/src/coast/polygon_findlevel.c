/*
 *	$Id: polygon_findlevel.c,v 1.28 2011-04-11 21:15:32 remko Exp $
 */
#include "wvs.h"

struct BLOB {
	struct GMT3_POLY h;
	int inside[6];
	int start;
	unsigned char n_inside, reverse;
	int x0, y0;
} blob[N_POLY];

int *lon, *lat;
double *flon, *flat;
struct LONGPAIR *pp;

int main (int argc, char **argv) {
	int i, j, k, c, n_id, pos, id, id1, id2, idmax, intest, sign, max_level, n, cont_no_1, cont_no_2, n_of_this[6];
	int n_reset = 0, old, bad = 0, ix0, set, force, full = 0, eur_id = 0, parent, *id2k;
	int *IX[N_CONTINENTS][2], *IY[N_CONTINENTS][2], N[N_CONTINENTS][2];
	double size, f, x_shift;
	FILE *fp, *fp2 = NULL, *fpx, *fpr;
	struct LONGPAIR p;
	char olds[32], news[32];
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_findlevel final_polygons.b revised_final_dbase.b [-f]\n");
		fprintf (stderr, "  -f will force calculation of areas.  Otherwise we will only\n");
		fprintf (stderr, "     recalculate areas if areas.lis does not exists\n");
		exit (EXIT_FAILURE);
	}
	force = (argc == 4);
	fpx = fopen ("still_bad.lis", "w");
	
	for (i = 0; i < 6; i++) n_of_this[i] = 0;
	fprintf (stderr, "Read headers\n");
	if ((fp = fopen (argv[1], "r")) == NULL) {
		fprintf (stderr, "polygon_findlevel: Cannot open file %s\n", argv[1]);
		exit (EXIT_FAILURE);
	}
	
	/* Keep existing ids */
	
	n_id = pos = 0;
	while (pol_readheader (&blob[n_id].h, fp) == 1) {
		cont_no_1 = (blob[n_id].h.river >> 8);	/* Get continent number 1-6 (0 if not a continent) */
		if (cont_no_1 == EURASIA) eur_id = n_id;	/* blob with Eurasia */

		pos += sizeof (struct GMT3_POLY);
		blob[n_id].start = pos;
		if (pol_fread (&p, 1, fp) != 1) {
			fprintf(stderr,"polygon_findlevel:  ERROR  reading file.\n");
			exit(-1);
		}
		if (p.x < 0) {
			fprintf (stderr, "x0 is actually neg %d. Stop; fix the problem\n", n_id);
			exit (-1);
		}
		if ((blob[n_id].h.greenwich & 1) && p.x > blob[n_id].h.datelon) p.x -= M360;
		blob[n_id].x0 = p.x;	/* Pick first point on the polygon, with x0 bracketed by header w/e */
		blob[n_id].y0 = p.y;
		blob[n_id].n_inside = blob[n_id].reverse = 0;
		fseek (fp, (blob[n_id].h.n - 1) * sizeof(struct LONGPAIR), 1);
		pos += blob[n_id].h.n * sizeof(struct LONGPAIR);
		if (blob[n_id].h.n < 3) blob[n_id].h.source = -1;
		n_id++;
	}
	full = (blob[eur_id].h.n > 1000000);	/* Only the full resolution has more than 1 mill points for EURASIA polygon */
	fprintf (stderr, "\n\nFind area and direction of polygons\n");

	area_init ();

	flon = (double *) GMT_memory (CNULL, blob[eur_id].h.n, sizeof(double), "polygon_findlevel");
	flat = (double *) GMT_memory (CNULL, blob[eur_id].h.n, sizeof(double), "polygon_findlevel");
	id2k = (int *) GMT_memory (CNULL, n_id, sizeof(int), "polygon_findlevel");

	if (full) fp2 = fopen ("areas.lis", "w");

	for (id = 0; id < n_id; id++) {
		id2k[blob[id].h.id] = id;
		if (blob[id].h.source == -1) continue;	/* Marked for deletion */
		
		fseek (fp, (long)blob[id].start, 0);
		for (k = 0; k < blob[id].h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_findlevel:  ERROR  reading file.\n");
				exit(-1);
			}
			if ((blob[id].h.greenwich & 1) && p.x > blob[id].h.datelon) p.x -= M360;
			flon[k] = p.x * 1.0e-6;
			flat[k] = p.y * 1.0e-6;
		}
		size = 1.0e-6 * area_size (flon, flat, blob[id].h.n, &sign); /* in km^2 */
		sprintf (olds, "%.12g", blob[id].h.area);
		sprintf (news, "%.12g", size);
		if (full && blob[id].h.area > 0.0 && size > 0.0) {
			f = fabs ((size / blob[id].h.area) - 1.0)*1e6;	/* ppm change */
			if (f > 5.0) fprintf (stderr, "Polygon %d has changed size by > 5 ppm (%.1f) [Area: old %s vs new %s]\n", blob[id].h.id, f, olds, news);
		}
		if (full) {
			fprintf (fp2, "%d\t%.12g\t%.12g\n", blob[id].h.id, size * sign, blob[id].h.area);
			blob[id].h.area = size;
		}
		blob[id].reverse = sign + 1;
	}

	free ((void *)flon);
	free ((void *)flat);
	if (full) fclose (fp2);
	
	lon = (int *) GMT_memory (CNULL, N_LONGEST, sizeof(int), "polygon_findlevel");
	lat = (int *) GMT_memory (CNULL, N_LONGEST, sizeof(int), "polygon_findlevel");
	
	/* Scale crude polygons by 1e6 to match the data scale */
	
	crude_init_int (IX, IY, N, MILL);
	
	/* Test everything except Antarctica which has no lakes */
	
	fprintf (stderr, "Start inside testing\n\n");
	
	for (id1 = 0; id1 < n_id; id1++) {	/* For all anchor polygons */
		if (blob[id1].h.source == -1) continue;	/* Marked for deletion */
#ifdef TEST
		if (blob[id1].h.id != 0) continue;	/* Looking for 0 */
#endif
		cont_no_1 = (blob[id1].h.river >> 8);	/* Get continent nubmer 1-6 (0 if not a continent) */
		if (cont_no_1 == ANTARCTICA) continue;	/* But skip Antarctica since there are no lakes in the data set */
		
		if (id1%10 == 0) fprintf (stderr, "Polygon %d\r", id1);

		fseek (fp, (long)blob[id1].start, 0);
		for (k = 0; k < blob[id1].h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_findlevel:  ERROR  reading file.\n");
				exit(-1);
			}
			if ((blob[id1].h.greenwich & 1) && p.x > blob[id1].h.datelon) p.x -= M360;
			lon[k] = p.x;
			lat[k] = p.y;
		}
		n = blob[id1].h.n;
		/* Here lon,lat goes from -180 to +359.99999 and is continuous (no jumps), and limited by w/e */
		
		for (id2 = 0; id2 < n_id; id2++) {
			
#ifdef TEST
			if (blob[id2].h.id != 799) continue;	/* Looking for 799 */
#endif
			if (blob[id2].h.source == -1) continue;		/* Marked for deletion */
			cont_no_2 = (blob[id2].h.river >> 8);		/* Get continent number 1-6 (0 if not a continent) */
			if (cont_no_2) continue;			/* But skip continents since they cannot contain each other */
			if (blob[id1].h.id == blob[id2].h.id) continue;	/* Skip self testing */
			
			if (nothing_in_common (&blob[id1].h, &blob[id2].h, &x_shift)) continue;	/* No area in common */

			/* Must compare with polygon boundaries */
			
			ix0 = blob[id2].x0 + (x_shift * MILL);
			if (full && cont_no_1 > 0 && cont_no_1 != ANTARCTICA) {	/* Use course outlines to determine if id2 is inside/outside a continent */
				c = cont_no_1 - 1;
				intest = non_zero_winding2 (ix0, blob[id2].y0, IX[c][OUTSIDE], IY[c][OUTSIDE], N[c][OUTSIDE]);
				if (!intest) continue;	/* id2 is outside this crude continent outline */
				
				/* Now check if it is inside crude inside */
				
				intest = non_zero_winding2 (ix0, blob[id2].y0, IX[c][INSIDE], IY[c][INSIDE], N[c][INSIDE]);

				if (intest == 2) {	/* way inside, set levels */
					blob[id2].inside[blob[id2].n_inside] = blob[id1].h.id;
					blob[id2].n_inside++;
					if (blob[id2].n_inside == 6) {
						fprintf (stderr, "You're fucked again!\n");
						exit (EXIT_FAILURE);
					}
					continue;
				}
				/* If not, we are between the two crude representations we fall down to the _real_ test using actual outlines */
			}
			
			/* Here we need to perform complete inside test */
			
			if (!(lon[0] == lon[n-1] && lat[0] == lat[n-1])) {	/* Close the polygon if not already */
				lon[n] = lon[0];
				lat[n] = lat[0];
				n++;
			}
			intest = non_zero_winding2 (ix0, blob[id2].y0, lon, lat, n);
			
			if (!intest) continue;	/* Not inside */
			if (intest == 1) {	/* Should not happen - duplicate polygon? */
				set = FALSE;
				if (blob[id1].h.source == 0 && blob[id2].h.source == 0 && blob[id1].h.n == blob[id2].h.n) { /* duplicate */
					fprintf (fpx, "%d is duplicate of %d, %d removed\n", id2, blob[id1].h.id, blob[id2].h.id);
					set = TRUE;
				}
				else {
					fprintf (stderr, "Point on edge!, ids = %d and %d\n", blob[id1].h.id, blob[id2].h.id);
					fprintf (fpx, "Point on edge!, ids = %d and %d\n", blob[id1].h.id, blob[id2].h.id);
					bad++;
				}
				if (set) blob[id2].h.source = -1;
			}
			
			/* OK, here id2 is inside id1 */
			
			blob[id2].inside[blob[id2].n_inside] = blob[id1].h.id;
			blob[id2].n_inside++;
			if (blob[id2].n_inside == 6) {
				fprintf (stderr, "You're fucked again!\n");
				exit (EXIT_FAILURE);
			}
		}
	}
		
	free ((void *)lon);
	free ((void *)lat);
	crude_free_int (IX, IY, N);

	fprintf (stderr, "\nFound %d bad cases\n", bad);
	
	/* Check if polygons need to be reversed */
	
	/* Find levels and decide if polygon need to be reversed */
	
	fprintf (stderr, "Check for need to reverse polygons\n");
	
	fpr = fopen ("levels.lis", "w");
	max_level = idmax = 0;
	for (id = 0; id < n_id; id++) {
		if (blob[id].h.source == -1) continue;	/* Marked for deletion */
		old = blob[id].h.level;
		blob[id].h.level = blob[id].n_inside + 1;
		if (old > 0 && old != blob[id].h.level) {
			fprintf (stderr, "Reset polygon %d level from %d to %d\n", blob[id].h.id, old, blob[id].h.level);
			fprintf (fpr, "Reset polygon %d level from %d to %d\n", blob[id].h.id, old, blob[id].h.level);
			n_reset++;
		}
		n_of_this[blob[id].h.level]++;
		old = blob[id].h.parent;
		if (blob[id].n_inside)
		
		if (blob[id].h.level > max_level) {
			max_level = blob[id].h.level;
			idmax = id;
		}
		
		if ( (blob[id].h.level%2) && blob[id].reverse == 0)	/* Land and negative area */
			blob[id].reverse = 1;
		else if ( !(blob[id].h.level%2) && blob[id].reverse == 2)	/* Water and positive area */
			blob[id].reverse = 1;
		else
			blob[id].reverse = 0;
			
	}
	fclose (fpr);
	/* Determine parent IDs */
	
	fpr = fopen ("parents.lis", "w");
	fprintf (stderr, "Determine parenthood\n");
	for (id1 = 0; id1 < n_id; id1++) {	/* See which polygon contains this polygon */
		fprintf (stderr, "Id = %d\r", id1);
		if (blob[id1].h.source == -1) continue;		/* Marked for deletion */
		if (blob[id1].h.level == 1) {			/* Highest level cannot have a parent */
			parent = blob[id1].h.parent;
			blob[id1].h.parent = -1;
			if (parent != blob[id1].h.parent) fprintf (stderr, "Reset level 1 polygon %d parent from %d to -1\n", blob[id1].h.id, blob[id1].h.parent);
			continue;
		}
		parent = -1;	/* OK, level > 1.  We now look through the list of polygons that contain id1 */
		for (i = 0; parent == -1 && i < blob[id1].n_inside; i++) {	/* For each of the polygon that polygon id1 is contained by */
			k = id2k[blob[id1].inside[i]];	/* Get the index to this id */
			if ((blob[id1].h.level - blob[k].h.level) == 1) {
				parent = blob[id1].inside[i]; /* Found the polygon one level up */
			}
		}
		if (parent == -1) fprintf (stderr, "Error: Polygon %d has no parent!\n", blob[id1].h.id);
		if (parent != blob[id1].h.parent) fprintf (stderr, "Reset polygon %d parent from %d to %d\n", blob[id1].h.id, blob[id1].h.parent, parent);
		blob[id1].h.parent = parent;
		if (parent != -1) fprintf (fpr, "%d is parent of %d\n", blob[id1].h.parent, blob[id1].h.id);
		if ((blob[id1].h.river & 1) && blob[id1].h.parent == -1) fprintf (stderr, "River polygon %d has no parent!\n", blob[id1].h.id);
	}
	fprintf (stderr, "Id = %d\n", id1);
	fclose (fpr);
	
	fprintf (fpx, "%d polygons had their presumed level reset\n", n_reset);
	fprintf (fpx, "max_level = %d for polygon %d (%d", max_level, blob[idmax].h.id, blob[idmax].h.id);
	for (i = 0; i < blob[idmax].n_inside; i++) fprintf (fpx, "-%d", blob[idmax].inside[i]);
	fprintf (fpx, ")\n");
	for (i = 1; i <= max_level; i++) fprintf (fpx, "Level%d: %d polygons\n", i, n_of_this[i]);
	fclose (fpx);
	
	fprintf (stderr, "Write out new data base\n");

	/* Write new base */
	
	pp = (struct LONGPAIR *) GMT_memory (CNULL, N_LONGEST, sizeof(struct LONGPAIR), "polygon_findlevel");
	
	fp2 = fopen (argv[2], "w");
	for (id = i = 0; id < n_id; id++) {
		if (blob[id].h.source == -1) continue;
		pol_writeheader (&blob[id].h, fp2);
		fseek (fp, (long)blob[id].start, 0);
		if (pol_fread (pp, blob[id].h.n, fp) != blob[id].h.n) {
			fprintf(stderr,"polygon_findlevel:  ERROR  reading file.\n");
			exit(-1);
		}
		if (blob[id].reverse) {	/* Reverse polygon */
			fprintf (stderr, "Reversing polygon %d\n", blob[id].h.id);
			for (j = blob[id].h.n - 1; j >= 0; j--) pol_fwrite (&pp[j], 1, fp2);
		}
		else
			pol_fwrite (pp, blob[id].h.n, fp2);
		i++;
	}
	
	fclose (fp);
	fclose (fp2);
	
	fp = fopen ("hierarchy.lis", "w");
	for (id = 0; id < n_id; id++) {
		if (blob[id].n_inside == 0) continue;
		fprintf (fp, "%d\tLevel-%d:", blob[id].h.id, blob[id].n_inside+1);
		for (i = 0; i < blob[id].n_inside; i++) fprintf (fp, "\t%d", blob[id].inside[i]);
		fprintf (fp, "\n");
	}
	fclose (fp);
	free ((void *)pp);
	free ((void *)id2k);
	
	exit (EXIT_SUCCESS);
}	
