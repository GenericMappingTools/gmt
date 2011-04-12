/*
 *	$Id: polygon_findlevel2.c,v 1.10 2011-04-12 13:06:42 remko Exp $
 */
#include "wvs.h"

struct BLOB {
	struct GMT3_POLY h;
	int inside[10];
	int start;
	unsigned char n_inside, reverse;
	int x0, y0;
} blob[N_POLY];

int *lon, *lat;
double *flon, *flat;
struct LONGPAIR *pp;

int debug = 1;

int main (int argc, char **argv) {
	int i, j, k, n_id, pos, id, id1, id2, idmax, intest, sign, max_level, n = 0, n_of_this[10];
	int n_reset = 0, old, bad = 0, ix0, off, set, n_yikes = 0, n_id1, not_read;
	double x0, west1, west2, east1, east2, size, val;
	FILE *fp, *fp2, *fp3, *fpx, *fp_in;
	struct LONGPAIR p;
	char line[80];
	
	if (argc == 1) {
		fprintf (stderr, "usage: polygon_findlevel2 complete_polygons.b final_cia2_x_polygons.b final_cia2.b\n");
		exit (-1);
	}

	fpx = fopen ("still_bad.lis", "w");
	
	for (i = 0; i < 10; i++) n_of_this[i] = 0;
	fprintf (stderr, "Read headers\n");
	fp = fopen (argv[1], "r");
	fp2 = fopen (argv[2], "r");
	
	/* Reset ids as we go along */
	
	n_id = pos = 0;
	while (pol_readheader (&blob[n_id].h, fp) == 1) {
		pos += sizeof (struct GMT3_POLY);
		blob[n_id].start = pos;
		if (pol_fread (&p, 1, fp) != 1) {
			fprintf(stderr,"polygon_findlevel2: Error reading file.\n");
			exit(-1);
		}
		blob[n_id].x0 = p.x;	/* Pick any point on the polygon */
		blob[n_id].y0 = p.y;
		blob[n_id].n_inside = 0;
		blob[n_id].reverse = 0;
		fseek (fp, (blob[n_id].h.n - 1) * sizeof(struct LONGPAIR), 1);
		pos += blob[n_id].h.n * sizeof(struct LONGPAIR);
		blob[n_id].h.id = n_id;
		n_id++;
	}
	n_id1 = n_id;
	
	pos = 0;
	while (pol_readheader (&blob[n_id].h, fp2) == 1) {
		pos += sizeof (struct GMT3_POLY);
		blob[n_id].start = pos;
		if (pol_fread (&p, 1, fp2) != 1) {
			fprintf(stderr,"polygon_findlevel2: Error reading file.\n");
			exit(-1);
		}
		blob[n_id].x0 = p.x;	/* Pick any point on the polygon */
		blob[n_id].y0 = p.y;
		blob[n_id].n_inside = 0;
		blob[n_id].reverse = 0;
		fseek (fp2, (blob[n_id].h.n - 1) * sizeof(struct LONGPAIR), 1);
		pos += blob[n_id].h.n * sizeof(struct LONGPAIR);
		blob[n_id].h.id = n_id;
		n_id++;
	}
	
	if ((fp3 = fopen ("areas.lis2", "r")) != NULL) {	/* Read areas etc */
		for (i = n_id1; i < n_id; i++) {
			fgets (line, 80, fp3);
			sscanf (line, "%d %lf", &j, &size);
			sign = (size < 0.0) ? -1 : 1;
			blob[i].reverse = sign + 1;
			blob[i].h.area = fabs(size);
			if (i != j) {
				fprintf (stderr, "Error reading areas\n");
				exit (-1);
			}
		}
	}
	else {

		fprintf (stderr, "\n\nFind area and direction of polygons\n");

		area_init ();

		flon = (double *) GMT_memory (CNULL, blob[0].h.n, sizeof(double), "polygon_findlevel2");
		flat = (double *) GMT_memory (CNULL, blob[0].h.n, sizeof(double), "polygon_findlevel2");

		fp3 = fopen ("areas.lis2", "w");
	
		for (id = n_id1; id < n_id; id++) {
		
			fseek (fp2, (long)blob[id].start, 0);
			for (k = 0; k < blob[id].h.n; k++) {
				if (pol_fread (&p, 1, fp2) != 1) {
					fprintf(stderr,"polygon_findlevel2: Error reading file.\n");
					exit(-1);
				}
				if ((blob[id].h.greenwich & 1) && p.x > blob[id].h.datelon) p.x -= M360;
				flon[k] = p.x * 1.0e-6;
				flat[k] = p.y * 1.0e-6;
			}
			n = blob[id].h.n;
			if (!(flon[blob[id].h.n-1] == flon[0] && flat[blob[id].h.n-1] == flat[0])) { /* close */
				flon[blob[id].h.n] = flon[0];
				flat[blob[id].h.n] = flat[0];
				n++;
			}
				
			size = 1.0e-6 * area_size (flon, flat, n, &sign); /* in km^2 */
			blob[id].h.area = size;
			blob[id].reverse = sign + 1;
			fprintf (fp3, "%d\t%g\n", id, size * sign);
		}

	
		free ((char *)flon);
		free ((char *)flat);
	}
	fclose (fp3);
	
	lon = (int *) GMT_memory (CNULL, N_LONGEST, sizeof(int), "polygon_findlevel2");
	lat = (int *) GMT_memory (CNULL, N_LONGEST, sizeof(int), "polygon_findlevel2");
	
	/* Test everything except Antarctica ( == 34) which has no lakes */
	
	fprintf (stderr, "Start inside testing\n\n");
	
	for (id1 = 0; id1 < n_id; id1++) {	/* For every single polygon (old as well as new) */
	
		if (id1 == 34) continue;	/* But skip Antarctica */
		
		if (id1 == 2) {	/* deallocate some space */
			lon = (int *) GMT_memory ((char *)lon, blob[id1].h.n+5, sizeof(int), "polygon_findlevel2");
			lat = (int *) GMT_memory ((char *)lat, blob[id1].h.n+5, sizeof(int), "polygon_findlevel2");
		}
		
		fprintf (stderr, "Polygon %d\n", id1);

		not_read = TRUE;
		
		west1 = blob[id1].h.west;	east1 = blob[id1].h.east;
		if (blob[id1].h.greenwich & 1) {
			west1 += 360.0;
			east1 += 360.0;
		}
		
		for (id2 = n_id1; id2 < n_id; id2++) {	/* Must check the latest polygons only */
			
			if (id1 == id2) continue;
			
			if (blob[id2].h.source == -1) continue;	/* Marked for deletion */
			
			/* if (blob[id2].h.level > 0) continue; */	/* Already know what this is */
			
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
			
			if (id1 == 0) {	/* Eurasia, first do quick test */
				intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, ieur_o[0], ieur_o[1], N_EUR_O);
				if (!intest) continue;
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and eur_o, must do full test\n", id2);
				
				/* So point is inside crude outside. Now check if it is inside crude inside */
				
				intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, ieur_i[0], ieur_i[1], N_EUR_I);
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and eur_i, must do full test\n", id2);

				if (!intest) intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, iafr_i[0], iafr_i[1], N_AFR_I);
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and afr_i, must do full test\n", id2);

				if (intest == 2) {	/* way inside, set levels */
					blob[id2].inside[blob[id2].n_inside] = id1;
					blob[id2].n_inside++;
					if (blob[id2].n_inside == 10) {
						fprintf (stderr, "You're fucked again!\n");
						exit (-1);
					}
					continue;
				}
				/* If not, we fall down to the _real_ test */
			}
			else if (id1 == 1) {	/* Americas, first do quick test */
				intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, iam_o[0], iam_o[1], N_AM_O);
				if (!intest) continue;
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and am_o, do full test\n", id2);

				
				/* So point is inside crude outside. Now check if it is inside crude inside */
				
				intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, isam_i[0], isam_i[1], N_SAM_I);
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and sam_i, do full test\n", id2);

				if (!intest) intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, inam_i[0], inam_i[1], N_NAM_I);
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and nam_i, do full test\n", id2);

				if (intest == 2) {	/* way inside, set levels */
					blob[id2].inside[blob[id2].n_inside] = id1;
					blob[id2].n_inside++;
					if (blob[id2].n_inside == 10) {
						fprintf (stderr, "You're fucked again!\n");
						exit (-1);
					}
					continue;
				}
				/* If not, we fall down to the _real_ test */
			}
			else if (id1 == 3) {	/* Austraila, first do quick test */
				intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, iaus_o[0], iaus_o[1], N_AUS_O);
				if (!intest) continue;
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and aus_o, do full test\n", id2);
				
				/* So point is inside crude outside. Now check if it is inside crude inside */
				
				intest = non_zero_winding2 (blob[id2].x0 + off, blob[id2].y0, iaus_i[0], iaus_i[1], N_AUS_I);
				if (intest == 1) fprintf (stderr, "Point on edge!, ids = %d and aus_i, do full test\n", id2);

				if (intest == 2) {	/* way inside, set levels */
					blob[id2].inside[blob[id2].n_inside] = id1;
					blob[id2].n_inside++;
					if (blob[id2].n_inside == 10) {
						fprintf (stderr, "You're fucked again!\n");
						exit (-1);
					}
					continue;
				}
				/* If not, we fall down to the _real_ test */
			}
			
			/* Here we need to perform complete inside test */
			
			if (not_read) {
				fp_in = (id1 < n_id1) ? fp : fp2;
				fseek (fp_in, (long)blob[id1].start, 0);
				for (k = 0; k < blob[id1].h.n; k++) {
					if (pol_fread (&p, 1, fp_in) != 1) {
						fprintf(stderr,"polygon_findlevel2: Error reading file.\n");
						exit(-1);
					}
					if ((blob[id1].h.greenwich & 1) && p.x > blob[id1].h.datelon) p.x -= M360;
					lon[k] = p.x;
					lat[k] = p.y;
				}
				n = blob[id1].h.n;
				not_read = FALSE;
			}
		
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
			if (intest == 1) {
				set = FALSE;
				if (blob[id1].h.source == 0 && blob[id2].h.source == 0 && blob[id1].h.n == blob[id2].h.n) { /* duplicate */
					fprintf (fpx, "%d is duplicate of %d, %d removed\n", id2, id1, id2);
					set = TRUE;
				}
				else {
					fprintf (stderr, "Point on edge!, ids = %d and %d\n", id1, id2);
					fprintf (fpx, "Point on edge!, ids = %d and %d\n", id1, id2);
					bad++;
				}
				if (set) blob[id2].h.source = -1;
			}
			else if (blob[id2].h.source == 0){	/* CIA inside another polygon, check if bad duplicate */
				if ((val = blob[id2].h.area / blob[id1].h.area) > 0.95) { 
					fprintf (fpx, "%d is inside %d but has %.1f %% of area. %d deleted\n", id2, id1, val * 100.0, id2);
					blob[id2].h.source = -1;
					fflush (fpx);
					bad++;
				}
			}
			
			blob[id2].inside[blob[id2].n_inside] = id1;
			blob[id2].n_inside++;
			if (blob[id2].n_inside == 10) {
				fprintf (stderr, "You're fucked again!\n");
				exit (-1);
			}
		}
	}
	
	free ((char *)lon);
	free ((char *)lat);

	fclose (fp);

	fprintf (stderr, "\nFound %d bad cases\n", bad);
	
	/* Check if polygons need to be reversed */
	
	/* Find levels and decide if polygon need to be reversed */
	
	fprintf (stderr, "Check for need to reverse polygons\n");
	
	max_level = idmax = 0;
	for (id = 0; id < n_id; id++) {
		if (blob[id].h.source == -1) continue;	/* Marked for deletion */
		if (blob[id].h.level > 0) continue;	/* Already set */
		old = blob[id].h.level;
		blob[id].h.level = blob[id].n_inside + 1;
		if (blob[id].h.level == 1) {	/* New CIA polygons must be lakes or higher! */
			n_yikes++;
			blob[id].h.source = -1;	/* Mark for deletion */
			continue;
		}
		if (old > 0 && old != blob[id].h.level) n_reset++;
		n_of_this[blob[id].h.level]++;
		
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
	
	fprintf (fpx, "%d polygons had their presumed level reset\n", n_reset);
	fprintf (fpx, "%d polygons had level = 1 and was removed\n", n_yikes);
	fprintf (fpx, "max_level = %d for polygon %d (%d", max_level, idmax, idmax);
	for (i = 0; i < blob[idmax].n_inside; i++) fprintf (fpx, "-%d", blob[idmax].inside[i]);
	fprintf (fpx, ")\n");
	for (i = 1; i <= max_level; i++) fprintf (fpx, "Level%d: %d polygons\n", i, n_of_this[i]);
	fclose (fpx);
	
	fprintf (stderr, "Write out new data base\n");

	/* Write new base */
	
	pp = (struct LONGPAIR *) GMT_memory (CNULL, N_LONGEST, sizeof(struct LONGPAIR), "polygon_findlevel2");
	
	fp3 = fopen (argv[3], "w");
	for (id = i = n_id1; id < n_id; id++) {
		if (blob[id].h.source == -1) continue;
		blob[id].h.id = i;
		pol_writeheader (&blob[id].h, fp3);
		fseek (fp2, (long)blob[id].start, 0);
		if (pol_fread (pp, blob[id].h.n, fp2) != blob[id].h.n) {
			fprintf(stderr,"polygon_findlevel2: Error reading file.\n");
			exit(-1);
		}
		if (blob[id].reverse) {	/* Reverse polygon */
			for (i = blob[id].h.n - 1; i >= 0; i--) pol_fwrite (&pp[i], 1, fp3);
		}
		else
			pol_fwrite (pp, blob[id].h.n, fp3);
		i++;
	}
	fprintf (stderr, "%d new polygons passed the tests\n", i);
	
	fclose (fp2);
	fclose (fp3);
	
	free ((char *)pp);
	
	exit (0);
}	
