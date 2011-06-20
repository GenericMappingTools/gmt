/*	$Id: gshhs_func.c,v 1.8 2011-06-20 22:15:09 guru Exp $
 *
 *	Copyright (c) 1996-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

#include "gmt_gshhs.h"
#include "gshhs.h"

struct GSHHS_CTRL {
	struct In {	/* <file> */
		GMT_LONG active;
		char *file;
	} In;
	struct Out {	/* > <file> */
		GMT_LONG active;
		char *file;
	} Out;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct I {	/* -I[<id>] */
		GMT_LONG active;
		GMT_LONG id;
	} I;
};

void *New_gshhs_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GSHHS_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GSHHS_CTRL);

	return ((void *)C);
}

void Free_gshhs_Ctrl (struct GMT_CTRL *GMT, struct GSHHS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->Out.file) free ((void *)C->Out.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gshhs_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;
	
	GMT_message (GMT, "gshhs %s [API] - ASCII export of GSHHS %s data\n", GSHHS_PROG_VERSION, GSHHS_DATA_VERSION);
	GMT_message (GMT, "usage: gshhs gshhs_[f|h|i|l|c].b [-I<id>] [-L] [%s] [%s] [%s] > table\n", GMT_V_OPT, GMT_bo_OPT, GMT_o_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "-L Only list header records (no data records will be written)\n");
	GMT_message (GMT, "-I Only output data for polygon number <id> [Default is all polygons]\n");
	GMT_explain_options (GMT, "VD2o:.");
	
	return (EXIT_FAILURE);
}
	
GMT_LONG GMT_gshhs_parse (struct GMTAPI_CTRL *C, struct GSHHS_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gshhs and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Count input files */
				n_files++;
				if (n_files == 1) Ctrl->In.file = strdup (opt->arg);
				break;

			case '>':	/* Got output file */
				Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'L':
				Ctrl->L.active = TRUE;
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				Ctrl->I.id = atoi (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: No data file specified!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_gshhs_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_gshhs (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG k, seg_no = 0, is_line = 0, n_alloc = 0, n_seg = 0, max_east = 270000000, error, ID, n_read;
	GMT_LONG mode, level, version, greenwich, is_river, src, must_swab, dim[4] = {1, 0, 2, 0}, first = TRUE;

	double w, e, s, n, area, f_area;
	
	char source, container[8], ancestor[8], header[GMT_BUFSIZ], *name[2] = {"polygon", "line"};
	
	FILE *fp = NULL;
	
	struct POINT p;
	struct GSHHS h;
 	struct GMT_DATASET *D = NULL;
 	struct GMT_LINE_SEGMENT **T = NULL;
	struct GSHHS_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
      
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_gshhs_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_gshhs_usage (API, GMTAPI_SYNOPSIS));		/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gshhs", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbo:", "m", options))) Return (error);
	Ctrl = (struct GSHHS_CTRL *) New_gshhs_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gshhs_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gshhs main code ----------------------------*/

	if (GMT_access (GMT, Ctrl->In.file, F_OK)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Cannot find file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}
	if ((fp = GMT_fopen (GMT, Ctrl->In.file, "rb")) == NULL ) {
		GMT_report (GMT, GMT_MSG_FATAL, "Cannot read file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}

	dim[1] = n_alloc = (Ctrl->I.active) ? 1 : GMT_CHUNK;
	if ((error = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, dim, (void **)&D, -1, &ID))) {
		GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a data set for GSHHS features.\n");
		return (GMT_RUNTIME_ERROR);
	}
	sprintf (header, "# Data extracted from GSHHS file %s", Ctrl->In.file);
	D->table[0]->header = GMT_memory (GMT, NULL, 1, char *);
	D->table[0]->header[0] = strdup (header);
	T = D->table[0]->segment;	/* There is only one output table with one or many segments */
	n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	version = (h.flag >> 8) & 255;
	must_swab = (version != GSHHS_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */
	
	while (n_read == 1) {
		n_seg++;
		if (must_swab) {	/* Must deal with different endianness */
			h.id 		= swabi4 ((unsigned int)h.id);
			h.n 		= swabi4 ((unsigned int)h.n);
			h.west		= swabi4 ((unsigned int)h.west);
			h.east		= swabi4 ((unsigned int)h.east);
			h.south		= swabi4 ((unsigned int)h.south);
			h.north		= swabi4 ((unsigned int)h.north);
			h.area		= swabi4 ((unsigned int)h.area);
			h.area_full	= swabi4 ((unsigned int)h.area_full);
			h.flag		= swabi4 ((unsigned int)h.flag);
			h.container	= swabi4 ((unsigned int)h.container);
			h.ancestor	= swabi4 ((unsigned int)h.ancestor);
		}
		if (Ctrl->I.active && h.id != Ctrl->I.id) {	/* Not what we are looking for, skip to next */
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
			n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);	/* Get the next GSHHS header */
			continue;	/* Back to top of loop */
		}
		
		/* OK, we want to return info for this feature */
		
		level = h.flag & 255;				/* Level is 1-4 */
		version = (h.flag >> 8) & 255;			/* Version is 1-7 */
		if (first) GMT_report (GMT, GMT_MSG_NORMAL, "Found GSHHS version %ld in file %s\n", version, Ctrl->In.file);
		greenwich = (h.flag >> 16) & 1;			/* Greenwich is 0 or 1 */
		src = (h.flag >> 24) & 1;			/* Source is 0 (WDBII) or 1 (WVS) */
		is_river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
		w = h.west  * GSHHS_SCL;			/* Convert region from microdegrees to degrees */
		e = h.east  * GSHHS_SCL;
		s = h.south * GSHHS_SCL;
		n = h.north * GSHHS_SCL;
		source = (src == 1) ? 'W' : 'C';		/* Either WVS or CIA (WDBII) pedigree */
		if (is_river) source = (char)tolower ((int)source);	/* Lower case c means river-lake */
		is_line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
		area = 0.1 * h.area;				/* Polygon area im km^2 */
		f_area = 0.1 * h.area_full;			/* Comparable area for full-resolution feature im km^2 */
		first = FALSE;

		dim[3] = h.n;			/* Number of data records to allocate for this segment/polygon */
		if (seg_no == n_alloc) {	/* Must add more segments to this table first */
			n_alloc <<= 2;
			T = GMT_memory (GMT, T, n_alloc, struct GMT_LINE_SEGMENT *);
		}

		/* Create the segment/polygon header record */
		if (is_line) {	/* River or border line-segment */
			sprintf (header, "%6d%8d%2ld%2c%10.5f%10.5f%10.5f%10.5f\n", h.id, h.n, level, source, w, e, s, n);
			max_east = 180000000;	/* For line segments we always use -180/+180  */
		}
		else {		/* Island or lake polygon */
			(h.container == -1) ? sprintf (container, "-") : sprintf (container, "%6d", h.container);
			(h.ancestor == -1) ? sprintf (ancestor, "-") : sprintf (ancestor, "%6d", h.ancestor);
			sprintf (header, "%6d%8d%2ld%2c%13.3f%13.3f%10.5f%10.5f%10.5f%10.5f %s %s\n", h.id, h.n, level, source, area, f_area, w, e, s, n, container, ancestor);
		}
		/* Place the header in the output data structure */
		T[seg_no]->header = strdup (header);

		if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
		}
		else {	/* Return the data points also */
			/* Allocate h.n number of data records */
			GMT_alloc_segment (GMT, T[seg_no], dim[3], dim[2], TRUE);
			for (k = 0; k < h.n; k++) {
				if (fread ((void *)&p, (size_t)sizeof(struct POINT), (size_t)1, fp) != 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error reading file %s for %s %d, point %ld.\n", Ctrl->In.file, name[is_line], h.id, k);
					Return (EXIT_FAILURE);
				}
				if (must_swab) {	/* Must deal with different endianness */
					p.x = swabi4 ((unsigned int)p.x);
					p.y = swabi4 ((unsigned int)p.y);
				}
				T[seg_no]->coord[GMT_X][k] = p.x * GSHHS_SCL;
				if ((greenwich && p.x > max_east) || (h.west > 180000000)) T[seg_no]->coord[GMT_X][k] -= 360.0;
				T[seg_no]->coord[GMT_Y][k] = p.y * GSHHS_SCL;
			}
		}
		seg_no++;
		max_east = 180000000;	/* Only Eurasia (the first polygon) needs 270 */
		n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);	/* Get the next GSHHS header */
	}
	if (seg_no < n_alloc) {	/* Allocate to final size table */
		T = GMT_memory (GMT, T, seg_no, struct GMT_LINE_SEGMENT *);
	}
		
	GMT_fclose (GMT, fp);

	mode = (is_line) ? GMT_IS_LINE : GMT_IS_POLY;
	if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, mode, NULL, 0, (void **)&Ctrl->Out.file, (void *)D))) Return (error);
	
	GMT_report (GMT, GMT_MSG_NORMAL, "%s in: %ld %s out: %ld\n", name[is_line], n_seg, name[is_line], seg_no);

	Return (GMT_OK);
}
