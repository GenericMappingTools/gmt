/*
 *	$Id: wvs_segment_restore2.c,v 1.3 2011-04-12 13:06:42 remko Exp $
 */
/* wvs_segment_restore2 <asciifiles> > <segments.b>
 *
 * Restores integer ASCII segments to binary form
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp;
	struct RAWSEG_HEADER hin;
	struct LONGPAIR *p;
	int	id, i, j, n_alloc = 100000;
	double x, y;
	char line[512];

	if (argc == 1) {
		fprintf(stderr,"usage: wvs_segment_restore2 asciifiles > raw_wvs_segment_file\n");
		exit(-1);
	}

	p = (struct LONGPAIR *) GMT_memory (CNULL, n_alloc, sizeof (struct LONGPAIR), "wvs_segment_restore2");
	id = 0;
	for (i = 1; i < argc; i++) {
		fprintf (stderr, "wvs_segment_restore2: Read file %s...", argv[i]);
		fp = fopen(argv[i], "r");
		j = 0;
		while (fgets (line, 512, fp)) {
			sscanf (line, "%lf %lf", &x, &y);
			p[j].x = (int) rint (1.0e6 * x);
			p[j].y = (int) rint (1.0e6 * y);
			j++;
			if (j == n_alloc) {
				n_alloc += 50000;
				p = (struct LONGPAIR *) GMT_memory ((char *)p, n_alloc, sizeof (struct LONGPAIR), "wvs_segment_restore2");
			}
		}
		hin.n = j;
		hin.rank = 1;
		fwrite((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, stdout);
		if ((fwrite((char *)p, sizeof(struct LONGPAIR), hin.n, stdout)) != hin.n) {
			fprintf(stderr,"wvs_segment_restore2: Error writing file.\n");
			exit(-1);
		}
		fclose (fp);
		id++;
		fprintf (stderr, "%d points stored\n", hin.n);
	}
	fprintf (stderr, "wvs_segment_restore2: Written %d binary segments\n", id);
	
	exit(0);
}
