/* $Id: x_edit.c,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
 *
 * X E D I T  will allow you to convert crossover tables ascii <--> binary
 *
 * Author:	Paul Wessel
 * Date:	18-FEB-1989
 * Revised:	06-MAR-2000	POSIX
 *
 */
#include "gmt.h"
#include "x_system.h"

struct CORR leg[3000];

main (int argc, char **argv)
{
	FILE *fp = NULL, *fpo = NULL;
	int i, nlegs, ascii = FALSE, error = FALSE;
	size_t legsize = sizeof (struct CORR);
	char line[BUFSIZ], *name = NULL;

	for (i = 1; i < argc; i++) {
		switch (argv[i][1]) {
			case 'X':
				fp = fopen (&argv[i][2], "rb");
				break;
			case 'A':
				fp = fopen (&argv[i][2], "r");
				ascii = TRUE;
				break;
			case 'O':
				name = &argv[i][2];
				break;
			default:
				error = TRUE;
				break;
		}
	}

	if (name) {
		fpo = (ascii) ? fopen (name, "w") : fopen (name, "wb");
	}
	else {
		fpo = stdout;
		if (ascii) {
#ifdef _WIN32
			setmode (fileno (fpo), _O_BINARY);
#elif WIN32
			_fsetmode (fpo, "b");
#endif
		}
	}
	
	if (fp == NULL) {
		fp = stdin;
		if (!ascii) {
#ifdef _WIN32
			setmode (fileno (fp), _O_BINARY);
#elif WIN32
			_fsetmode (fp, "b");
#endif
		}
	}
	
	if (argc == 1 || error) {
		fprintf (stderr, "x_edit - convert between binary and ASCCI xocer correction tables\n");
		fprintf (stderr, "usage: x_edit [-Xbinfile -Aasciifile -Ooutfile]\n");
		fprintf (stderr, "	-A give ASCII filename, convert to binary (-O)\n");
		fprintf (stderr, "	-O name of output file\n");
		fprintf (stderr, "	-X give binary filename, convert to ASCII (-O)\n");
		exit (EXIT_FAILURE);
	}

	if (ascii) {
		i = 0;
		while (fgets (line, BUFSIZ, fp)) {
			sscanf (line, "%s %hd %f %f %f %f %f %f",
			leg[i].name,
			&leg[i].year,
			&leg[i].dc_shift_gmt[0],
			&leg[i].dc_shift_gmt[1],
			&leg[i].dc_shift_gmt[2],
			&leg[i].drift_rate_gmt[0],
			&leg[i].drift_rate_gmt[1],
			&leg[i].drift_rate_gmt[2]);
			i++;
		}
		if (fp != stdin) fclose (fp);
		nlegs = i;
		for (i = 0; i < nlegs; i++)
                    fwrite ((void *)&leg[i], legsize, 1, fpo);
            fclose(fpo);
	}
	else {
		i = 0;
		while (fread ((void *)&leg[i], legsize, 1, fp) == 1) i++;
		nlegs = i;
		fclose (fp);
		for (i = 0; i < nlegs; i++)
			fprintf (fpo, "%s\t%d\t%g\t%g\t%g\t%g\t%g\t%g\n",
			leg[i].name,
			leg[i].year,
			leg[i].dc_shift_gmt[0],
			leg[i].dc_shift_gmt[1],
			leg[i].dc_shift_gmt[2],
			leg[i].drift_rate_gmt[0],
			leg[i].drift_rate_gmt[1],
			leg[i].drift_rate_gmt[2]);
		if (fpo != stdout) fclose (fpo);
	}
	exit (EXIT_SUCCESS);
}
