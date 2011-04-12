/*
 *	$Id: segment_final_dump.c,v 1.4 2011-04-12 13:06:42 remko Exp $
 */
/* segment_final_dump <wvs_polygon_file.b> wvs_polygons.b
 *
 */

#include "wvs.h"

struct GMT3_POLY h3;

struct LONGPAIR p[N_LONGEST];

int main (int argc, char **argv)
{
	FILE	*fp, *fp2;
	struct RAWSEG_HEADER hin;
	int	id, i, greenwich, iw, ie, is, in, np = 0, ranks[100];

	if (argc != 3) {
		fprintf(stderr,"usage: segment_final_dump wvs_clean_poly.b wvs_polygons.b\n");
		exit(-1);
	}

	memset ((char *)ranks, 0, 100*sizeof(int));
	
	fp = fopen(argv[1], "r");
	fp2 = fopen(argv[2], "w");
	id = 0;
	while (fread((void *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp) == 1) {
		if ((fread((void *)p, sizeof(struct LONGPAIR), (size_t) hin.n, fp)) != hin.n) {
			fprintf(stderr,"segment_final_dump: Error reading file.\n");
			exit(-1);
		}
		
if (id == 1160)
	id = id;
		h3.id = id;
		h3.n = hin.n;
		h3.datelon = (h3.n > 1400000) ? 270000000 : 180000000;
		/* h3.level = -1; */
		h3.level = hin.rank;
		ranks[hin.rank]++;
		h3.source = 0;
		h3.area = 0.0;
		
		for (i = 1, greenwich = FALSE; !greenwich && i < h3.n; i++) if (abs (p[i].x - p[i-1].x) > 180000000)
			greenwich = TRUE;
			
		h3.greenwich = greenwich;
		
		if (greenwich) for (i = 0; i < h3.n; i++) if (p[i].x > h3.datelon) p[i].x -= 360000000;
		
		iw = ie = p[0].x;
		is = in = p[0].y;
		for (i = 1; i < h3.n; i++) {
			if (p[i].x < iw) iw = p[i].x;
			if (p[i].x > ie) ie = p[i].x;
			if (p[i].y < is) is = p[i].y;
			if (p[i].y > in) in = p[i].y;
		}
		h3.west = 1.0e-6 * iw;
		h3.east = 1.0e-6 * ie;
		h3.south = 1.0e-6 * is;
		h3.north = 1.0e-6 * in;
			
		if (greenwich && h3.north < -60.0 && fabs (h3.west - h3.east) > 350.0) {	/* Antarctica */
			h3.west = -180.0;
			h3.east = 180.0;
		}
		
		if (greenwich) for (i = 0; i < h3.n; i++) if (p[i].x < 0) p[i].x += 360000000;

		if (pol_writeheader (&h3, fp2) != 1) {
			fprintf (stderr, "segment_final_dump: Failed writing segment header\n");
			exit (-1);
		}
		
		if (pol_fwrite (p, h3.n, fp2) != h3.n) {
			fprintf (stderr, "segment_final_dump: Failed writing segment\n");
			exit (-1);
		}
		id++;
		np += h3.n;
	}

	fclose(fp2);
	fclose(fp);

	fprintf (stderr, "segment_final_dump: Wrote %d polygons (%d points)\n", id, np);

	for (i = 0; i < 100; i++) if (ranks[i]) fprintf (stderr, "rank %2.2d: %d segments\n", i, ranks[i]);
	
	exit(0);
}
