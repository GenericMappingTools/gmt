/*
 *	$Id$
 */
/* 
 *	segment_dump makes a multisegment ascii-file of entire dbase
 */

#include "wvs.h"

#define M360		360000000

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k, level = 0, id;
	struct	LONGPAIR p;
	struct RAWSEG_HEADER h;
        
	if (argc != 3) {
		fprintf(stderr,"usage: segment_dump polygons.b level\n");
		exit(-1);
	}
	level = atoi (argv[2]);
	
	fp_in = fopen(argv[1], "r");
	id = 0;
	while (fread((char *)&h, sizeof (struct RAWSEG_HEADER), 1, fp_in) == 1) {
		if (level > 0 && h.rank != level) {
			fseek (fp_in, h.n * sizeof(struct LONGPAIR), 1);
			id++;
			continue;
		}
		
		printf ("> Segment %d N = %d\n", id, h.n);
		for (k = 0; k < h.n; k++) {
			if (fread((char *)&p, sizeof(struct LONGPAIR), 1, fp_in) != 1) {
				fprintf(stderr,"segment_dump: Error reading file.\n");
				exit(-1);
			}
			printf ("%g\t%g\n", 1.0e-6 * p.x, 1.0e-6 * p.y);
		}
		id++;
	}
		
	fclose(fp_in);

	exit (0);
}
