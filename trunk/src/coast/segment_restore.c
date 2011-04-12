/*
 *	$Id: segment_restore.c,v 1.2 2011-04-12 13:06:43 remko Exp $
 */
/* segment_restore <asciifiles> > <segments.b>
 *
 * Restores integer ASCII segments to binary form
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp;
	struct RAWSEG_HEADER hin;
	struct LONGPAIR *p;
	int	id, i, j, level = 1, n_alloc = 100000;
	double x, y;
	char line[512];

	if (argc == 1) {
		fprintf(stderr,"usage: segment_restore [-L<level>] asciifiles > polygons.b\n");
		exit(-1);
	}

	p = (struct LONGPAIR *) GMT_memory (CNULL, n_alloc, sizeof (struct LONGPAIR), "segment_restore");
	id = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'L') {
				level = atoi (&argv[i][2]);
				fprintf (stderr, "segment_restore: level set to %d\n", level);
			}
			continue;
		}
		fprintf (stderr, "segment_restore: Read file %s...", argv[i]);
		fp = fopen(argv[i], "r");
		j = 0;
		while (fgets (line, 512, fp)) {
			sscanf (line, "%lf %lf", &x, &y);
			p[j].x = (int) rint (1.0e6 * x);
			p[j].y = (int) rint (1.0e6 * y);
			j++;
			if (j == n_alloc) {
				n_alloc += 50000;
				p = (struct LONGPAIR *) GMT_memory ((char *)p, n_alloc, sizeof (struct LONGPAIR), "segment_restore");
			}
		}
		hin.n = j;
		hin.rank = level;
		fwrite((void *)&hin, sizeof(struct RAWSEG_HEADER), 1, stdout);
		if ((fwrite((void *)p, sizeof(struct LONGPAIR), (size_t) hin.n, stdout)) != hin.n) {
			fprintf(stderr,"segment_restore: Error writing file.\n");
			exit(-1);
		}
		fclose (fp);
		id++;
		fprintf (stderr, "%d points stored\n", hin.n);
	}
	fprintf (stderr, "segment_restore: Written %d binary segments\n", id);
	
	exit(0);
}
