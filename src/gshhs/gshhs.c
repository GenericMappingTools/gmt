/*	$Id: gshhs.c,v 1.6 2005-03-04 00:48:31 pwessel Exp $
 *
 * PROGRAM:	gshhs.c
 * AUTHOR:	Paul Wessel (pwessel@hawaii.edu)
 * CREATED:	JAN. 28, 1996
 * PURPOSE:	To extract ASCII data from binary shoreline data
 *		as described in the 1996 Wessel & Smith JGR Data Analysis Note.
 * VERSION:	1.1 (Byte flipping added)
 *		1.2 18-MAY-1999:
 *		   Explicit binary open for DOS systems
 *		   POSIX.1 compliant
 *		1.3 08-NOV-1999: Released under GNU GPL
 *		1.4 05-SEPT-2000: Made a GMT supplement; FLIP no longer needed
 *		1.5 14-SEPT-2004: Updated to deal with latest GSHHS database (1.3)
 *
 *	Copyright (c) 1996-2004 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
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
	double w, e, s, n, area, lon, lat;
	char source;
	FILE	*fp;
	int	k, max_east = 270000000, info, n_read, flip;
	struct	POINT p;
	struct GSHHS h;
        
	if (argc < 2 || argc > 3) {
		fprintf (stderr, "gshhs v. 1.5 ASCII export tool\n");
		fprintf (stderr, "usage:  gshhs gshhs_[f|h|i|l|c].b [-L] > ascii.dat\n");
		fprintf (stderr, "-L will only list headers (no data output)\n");
		exit (EXIT_FAILURE);
	}

	info = (argc == 3);
	if ((fp = fopen (argv[1], "rb")) == NULL ) {
		fprintf (stderr, "gshhs:  Could not find file %s.\n", argv[1]);
		exit (EXIT_FAILURE);
	}
		
	n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	flip = (! (h.level > 0 && h.level < 5));	/* Take as sign that byte-swabbing is needed */
	
	while (n_read == 1) {
		if (flip) {
			h.id = swabi4 ((unsigned int)h.id);
			h.n  = swabi4 ((unsigned int)h.n);
			h.level = swabi4 ((unsigned int)h.level);
			h.west  = swabi4 ((unsigned int)h.west);
			h.east  = swabi4 ((unsigned int)h.east);
			h.south = swabi4 ((unsigned int)h.south);
			h.north = swabi4 ((unsigned int)h.north);
			h.area  = swabi4 ((unsigned int)h.area);
			h.version  = swabi4 ((unsigned int)h.version);
			h.greenwich = swabi2 ((unsigned int)h.greenwich);
			h.source = swabi2 ((unsigned int)h.source);
		}
		w = h.west  * 1.0e-6;	/* Convert from microdegrees to degrees */
		e = h.east  * 1.0e-6;
		s = h.south * 1.0e-6;
		n = h.north * 1.0e-6;
		source = (h.source == 1) ? 'W' : 'C';	/* Either WVS or CIA (WDBII) pedigree */
		area = 0.1 * h.area;			/* Now im km^2 */

		printf ("P %6d%8d%2d%2c%13.3f%10.5f%10.5f%10.5f%10.5f\n", h.id, h.n, h.level, source, area, w, e, s, n);

		if (info) {	/* Skip data, only want headers */
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
		}
		else {
			for (k = 0; k < h.n; k++) {

				if (fread ((void *)&p, (size_t)sizeof(struct POINT), (size_t)1, fp) != 1) {
					fprintf (stderr, "gshhs:  Error reading file %s for polygon %d, point %d.\n", argv[1], h.id, k);
					exit (EXIT_FAILURE);
				}
				if (flip) {
					p.x = swabi4 ((unsigned int)p.x);
					p.y = swabi4 ((unsigned int)p.y);
				}
				lon = (h.greenwich && p.x > max_east) ? p.x * 1.0e-6 - 360.0 : p.x * 1.0e-6;
				lat = p.y * 1.0e-6;
				printf ("%10.5f%9.5f\n", lon, lat);
			}
		}
		max_east = 180000000;	/* Only Eurasiafrica needs 270 */
		n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	}
		
	fclose (fp);

	exit (EXIT_SUCCESS);
}
