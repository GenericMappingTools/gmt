/*
 *	$Id: wvs_segment_dump.c,v 1.3 2011-04-12 13:06:42 remko Exp $
 */
/* wvs_segment_dump <raw_wvs_segment_file.b> <prefix>
 *
 * Dumps segments in integer ASCII.
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp, *fp2;
	struct RAWSEG_HEADER hin;
	struct LONGPAIR ptemp;
	int	id, j;
	char prefix[100], file[512];
	strcpy (prefix, "piece");

	if (argc == 1) {
		fprintf(stderr,"usage: wvs_segment_dump raw_wvs_segment_file prefix\n");
		exit(-1);
	}

	fp = fopen(argv[1], "r");
	if (argc == 3) strcpy (prefix, argv[2]);
	id = 0;
	while (fread((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp) == 1) {
		sprintf(file, "%s.%d", prefix, id);
		fp2 = fopen (file, "w");
		for (j = 0; j < hin.n; j++) {
			if ((fread((char *)&ptemp, sizeof(struct LONGPAIR), 1, fp)) != 1) {
				fprintf(stderr,"wvs_segment_dump: Error reading file.\n");
				exit(-1);
			}
			if (ptemp.x == 360000000) ptemp.x = 0;
			fprintf(fp2, "%d\t%d\n", ptemp.x, ptemp.y);
		}
		fclose (fp2);
		id++;
	}

	fclose(fp);

	exit(0);
}
