/*
 *	$Id$
 */
/* segment_clean.c <raw_wvs_segment_file.b>  <clean_wvs_segment_file.b>
 *
 * Verifies that segs are in -90 to 90, 0-360, changes RAWSEG_HEADERs to
 * SEG_HEADERs, and loads first and last points into seg headers.
 *
 * Checks for redundant points, spikes, and crossing lines.  Also dumps some
 * statistics, and a seek table.
 * 
 * Walter Smith, 2 February, 1994.
 */

#include "wvs.h"

#define	MAX_STR_LEN 8192	/* I ran some stats and found 6137 as the max  */
#define	MAX_N_STR 262144	/* I ran some stats and found 210762 actually  */
extern int new_stringcheck (struct LONGPAIR p[], int *n, double x[], double y[], int id, int verbose);

int main (int argc, char **argv)
{
	FILE	*fp, *fp2;
	struct RAWSEG_HEADER hin, hout;
	struct LONGPAIR p[MAX_STR_LEN], ptemp;
	double	xx[MAX_STR_LEN], yy[MAX_STR_LEN];
	int	id, shortest, longest, avg, minrank, maxrank, lonerr, laterr, j, zero = 0;
	int	dx, dy, x_max, y_max, n_closed = 0, n_not_on_edge = 0, n_zero = 0;
	double sum_dx = 0.0, sum_dy = 0.0, sum_dx2 = 0.0, sum_dy2 = 0.0;
	double	ds, maxds, xscale, meanlat, avgds;
	int	maxid = 0, nbigbad = 0, nsmallbad = 0;
	int	ncross, ncleaned = 0, verbose = 0;



	if (argc < 3 || argc > 4 || (fp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr,"usage: segment_clean raw_wvs_segment_file  clean_wvs_segment_file [-v]\n");
		exit(-1);
	}

	fp2 = fopen (argv[2], "w");
	if (argc == 4) verbose = 1;
	
	id = 0;
	longest = avg = 0;
	shortest = 1000000;
	minrank = maxrank = 1;
	lonerr = laterr = 0;
	x_max = y_max = 0;
	maxds = avgds = 0.0;
	while ((fread((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp)) == 1) {
		if (hin.rank > maxrank) maxrank = hin.rank;
		if (hin.rank < minrank) minrank = hin.rank;
		if (hin.n <= 0) {
			fprintf(stderr,"segment_clean: There are non-positive string numbers.\n");
		}
		for (j = 0; j < hin.n; j++) {
			if ((fread((char *)&ptemp, sizeof(struct LONGPAIR), 1, fp)) != 1) {
				fprintf(stderr,"segment_clean: Error reading file.\n");
				exit(-1);
			}
			if (ptemp.x < 0 || ptemp.x >= 360000000) {
				if (ptemp.x == 360000000) 
					ptemp.x = 0;
				else {
					lonerr++;
					fprintf(stderr,"segment_clean: Longitude = %d\n", ptemp.x);
				}
			}
			if (ptemp.y < -90000000 || ptemp.y > 90000000) laterr++;
			
			p[j] = ptemp;
		}

 		for (j = 0; j < hin.n; j++) if (p[j].x == 0 && p[j].y == 0) zero++;
 		if (hin.n == 1 && p[0].x == 0 && p[0].y == 0) n_zero++;
 		
 		for (j = 1; j < hin.n; j++) {	/* Do stats on point separation */
			dx = abs(p[j].x - p[j-1].x);
			if (dx > 180000000) dx = 360000000 - dx;
			dy = abs (p[j].y - p[j-1].y);
			meanlat = 0.5e-06 * (p[j].y + p[j-1].y);
			xscale = cosd(meanlat);
			ds = 111.1949e-06 * hypot(dx*xscale, (double)dy);
			avgds += ds;
			if (maxds < ds) {
				maxds = ds;
				maxid = id;
			}
			sum_dx += dx;	sum_dx2 += dx * dx;
			sum_dy += dy;	sum_dy2 += dy * dy;
			if (dx > x_max) x_max = dx;
			if (dy > y_max) y_max = dy;
		}

		hout.n = hin.n;

		if ((ncross = new_stringcheck(p, &hout.n, xx, yy, id, verbose)) ) {
			if (hout.n > 2) {
				nbigbad++;
				fprintf(stderr, "id = %d has %d crossovers.\n", id, ncross);
				/* for (j = 0; j < hout.n; j++) printf("%10.6lf\t%10.6lf\n", 1.0e-06*p[j].x, 1.0e-06*p[j].y); */
			}
			else
				nsmallbad++;

		}
		else {
			hout.rank = hin.rank;
			if (hout.n > longest) longest = hout.n;
			if (hout.n < shortest) shortest = hout.n;
			avg += hout.n;
			if (p[0].x == p[hout.n-1].x && p[0].y == p[hout.n-1].y)
				n_closed++;
			else
				n_not_on_edge += !((p[0].x%1000000 == 0 || p[0].y%1000000 == 0) && (p[hout.n-1].x%1000000 == 0 || p[hout.n-1].y%1000000 == 0));
			/* Need to write out the segment and header here */
		}
		
		if (hout.n > 1) {
			if ((j=fwrite ((char *)&hout, sizeof(struct RAWSEG_HEADER), 1, fp2)) != 1) {
				fprintf(stderr,"segment_clean: Error writing header.\n");
				exit(-1);
			}
			if ((j=fwrite((char *)p, sizeof(struct LONGPAIR), hout.n, fp2)) != hout.n) {
				fprintf(stderr,"segment_clean: Error writing data.\n");
				exit(-1);
			}
		}
		else
			nsmallbad++;
				
		id++;
		if (hout.n != hin.n)
			ncleaned++;

		if (id%100 == 0)fprintf(stderr,"segment_clean: Finished %6.6d strings with %6.6d too short, %6.6d crossing, %6.6d cleaned so far\r", id, nsmallbad, nbigbad, ncleaned);

	}
	
	fclose (fp2);
	fprintf(stderr,"\nsegment_clean: There are %d strings in the file.\n", id);
	fprintf(stderr,"segment_clean: Min and Max # points in a string are %d %d\n", shortest, longest);
	fprintf(stderr,"segment_clean: Min and Max ranks found are %d %d\n", minrank, maxrank);
	fprintf(stderr,"segment_clean: There are %d longitudes out of valid range.\n", lonerr);
	fprintf(stderr,"segment_clean: There are %d latitudes out of valid range.\n", laterr);
	fprintf(stderr,"segment_clean: There are a total of %d points stored in the file.\n", avg);
	fprintf(stderr,"segment_clean: The average string length is %d.\n", irint((double)avg/(double)id));
	fprintf(stderr,"segment_clean: %d strings represents closed polygons\n", n_closed);
	fprintf(stderr,"segment_clean: %d strings did not start and end on a grid line\n", n_not_on_edge);
	fprintf(stderr,"segment_clean: %d strings shrank to <2 points after cleaning\n", nsmallbad);
	fprintf(stderr,"segment_clean: %d strings have crossings\n", nbigbad);
	fprintf(stderr,"segment_clean: The coordinate (0,0) occured %d times\n", zero);
	fprintf(stderr,"segment_clean: %d strings of length 1 had (0,0) as only point\n", n_zero);
	fprintf(stderr,"segment_clean: Max point separation withing string %g for segment # %d\n", maxds, maxid);

	fclose(fp);

	/* For testing, dump the string with the largest ds: */
	
	exit (0); 
	
	fp = fopen(argv[1], "r");
	id = 0;
	while (id <= maxid && (fread((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp)) == 1) {
		if (id == maxid) {
			for (j = 0; j < hin.n; j++) {
				if ((fread((char *)&ptemp, sizeof(struct LONGPAIR), 1, fp)) != 1) {
					fprintf(stderr,"segment_clean: Error reading file.\n");
					exit(-1);
				}
				printf("%10.6f\t%10.6f\n", 1.0e-06*ptemp.x, 1.0e-06*ptemp.y);
			}
		}
		else {
			fseek(fp, hin.n*sizeof(struct LONGPAIR), 1);
		}
		id++;
	}
	fclose(fp);
  
	exit(0);
}
