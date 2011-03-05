/*	$Id: gshhs.c,v 1.37 2011-03-05 21:24:28 guru Exp $
 *
 *	Copyright (c) 1996-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 * PROGRAM:	gshhs.c
 * AUTHOR:	Paul Wessel (pwessel@hawaii.edu)
 * CREATED:	JAN. 28, 1996
 * PURPOSE:	To extract ASCII data from the binary GSHHS shoreline data
 *		as described in the 1996 Wessel & Smith JGR Data Analysis Note.
 * VERSION:	1.1 (Byte flipping added)
 *		1.2 18-MAY-1999:
 *		   Explicit binary open for DOS systems
 *		   POSIX.1 compliant
 *		1.3 08-NOV-1999: Released under GNU GPL
 *		1.4 05-SEPT-2000: Made a GMT supplement; FLIP no longer needed
 *		1.5 14-SEPT-2004: Updated to deal with latest GSHHS database (1.3)
 *		1.6 02-MAY-2006: Updated to deal with latest GSHHS database (1.4)
 *		1.7 11-NOV-2006: Fixed bug in computing level (&& vs &)
 *		1.8 31-MAR-2007: Updated to deal with latest GSHHS database (1.5)
 *		1.9 27-AUG-2007: Handle line data as well as polygon data
 *		1.10 15-FEB-2008: Updated to deal with latest GSHHS database (1.6)
 *		1.12 15-JUN-2009: Now contains information on container polygon,
 *				the polygons ancestor in the full resolution, and
 *				a flag to tell if a lake is a riverlake.
 *				Updated to deal with latest GSHHS database (2.0)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: www.soest.hawaii.edu/pwessel */

#include "gshhs.h"

int main (int argc, char **argv)
{
	double w, e, s, n, area, f_area, lon, lat;
	char source, kind[2] = {'P', 'L'}, c = '>', *file = NULL;
	char *name[2] = {"polygon", "line"}, container[8], ancestor[8];
	FILE *fp = NULL;
	int k, line, max_east = 270000000, info, single, error, ID, flip;
	int  OK, level, version, greenwich, river, src, msformat = 0, first = 1;
	size_t n_read;
	struct	POINT p;
	struct GSHHS h;
        
	info = single = error = ID = 0;
	for (k = 1; k < argc; k++) {
		if (argv[k][0] == '-') {	/* Option switch */
			switch (argv[k][1]) {
				case 'L':
					info = 1;
					break;
				case 'M':
					msformat = 1;
					break;
				case 'I':
					single = 1;
					ID = atoi (&argv[k][2]);
					break;
				default:
					error++;
					break;
			}
		}
		else
			file = argv[k];
	}

	if (!file) {
		fprintf (stderr, "gshhs: No data file given!\n");
		error++;
	}
	if (argc == 1 || error) {
		fprintf (stderr, "gshhs %s - ASCII export of GSHHS %s data\n", GSHHS_PROG_VERSION, GSHHS_DATA_VERSION);
		fprintf (stderr, "usage:  gshhs gshhs_[f|h|i|l|c].b [-I<id>] [-L] [-M] > ascii.dat\n");
		fprintf (stderr, "-L will only list headers (no data output)\n");
		fprintf (stderr, "-I only output data for polygon number <id> [Default is all polygons]\n");
		fprintf (stderr, "-M will write '>' at start of each segment header [P or L]\n");
		exit (EXIT_FAILURE);
	}

	if ((fp = fopen (file, "rb")) == NULL ) {
		fprintf (stderr, "gshhs:  Could not find file %s.\n", file);
		exit (EXIT_FAILURE);
	}
		
	n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	version = (h.flag >> 8) & 255;
	flip = (version != GSHHS_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */
	
	while (n_read == 1) {
		if (flip) {
			h.id = swabi4 ((unsigned int)h.id);
			h.n  = swabi4 ((unsigned int)h.n);
			h.west  = swabi4 ((unsigned int)h.west);
			h.east  = swabi4 ((unsigned int)h.east);
			h.south = swabi4 ((unsigned int)h.south);
			h.north = swabi4 ((unsigned int)h.north);
			h.area  = swabi4 ((unsigned int)h.area);
			h.area_full  = swabi4 ((unsigned int)h.area_full);
			h.flag  = swabi4 ((unsigned int)h.flag);
			h.container  = swabi4 ((unsigned int)h.container);
			h.ancestor  = swabi4 ((unsigned int)h.ancestor);
		}
		level = h.flag & 255;				/* Level is 1-4 */
		version = (h.flag >> 8) & 255;			/* Version is 1-7 */
		if (first) fprintf (stderr, "gshhs %s - Found GSHHS version %d in file %s\n", GSHHS_PROG_VERSION, version, file);
		greenwich = (h.flag >> 16) & 1;			/* Greenwich is 0 or 1 */
		src = (h.flag >> 24) & 1;			/* Greenwich is 0 (WDBII) or 1 (WVS) */
		river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
		w = h.west  * GSHHS_SCL;			/* Convert from microdegrees to degrees */
		e = h.east  * GSHHS_SCL;
		s = h.south * GSHHS_SCL;
		n = h.north * GSHHS_SCL;
		source = (src == 1) ? 'W' : 'C';		/* Either WVS or CIA (WDBII) pedigree */
		if (river) source = tolower ((int)source);	/* Lower case c means river-lake */
		line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
		area = 0.1 * h.area;				/* Now im km^2 */
		f_area = 0.1 * h.area_full;			/* Now im km^2 */

		OK = (!single || h.id == ID);
		first = 0;
		
		if (!msformat) c = kind[line];
		if (OK) {
			if (line)
				printf ("%c %6d%8d%2d%2c%10.5f%10.5f%10.5f%10.5f\n", c, h.id, h.n, level, source, w, e, s, n);
			else {
				(h.container == -1) ? sprintf (container, "-") : sprintf (container, "%6d", h.container);
				(h.ancestor == -1) ? sprintf (ancestor, "-") : sprintf (ancestor, "%6d", h.ancestor);
				printf ("%c %6d%8d%2d%2c%13.3f%13.3f%10.5f%10.5f%10.5f%10.5f %s %s\n", c, h.id, h.n, level, source, area, f_area, w, e, s, n, container, ancestor);
			}
		}

		if (info || !OK) {	/* Skip data, only want headers */
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
		}
		else {
			for (k = 0; k < h.n; k++) {

				if (fread ((void *)&p, (size_t)sizeof(struct POINT), (size_t)1, fp) != 1) {
					fprintf (stderr, "gshhs:  Error reading file %s for %s %d, point %d.\n", argv[1], name[line], h.id, k);
					exit (EXIT_FAILURE);
				}
				if (flip) {
					p.x = swabi4 ((unsigned int)p.x);
					p.y = swabi4 ((unsigned int)p.y);
				}
				lon = p.x * GSHHS_SCL;
				if ((greenwich && p.x > max_east) || (h.west > 180000000)) lon -= 360.0;
				lat = p.y * GSHHS_SCL;
				printf ("%11.6f%11.6f\n", lon, lat);
			}
		}
		max_east = 180000000;	/* Only Eurasia needs 270 */
		n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	}
		
	fclose (fp);

	exit (EXIT_SUCCESS);
}
