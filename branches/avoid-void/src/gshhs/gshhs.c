/*	$Id$
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
 *		1.11 15-JUN-2009: Now contains information on container polygon,
 *				the polygons ancestor in the full resolution, and
 *				a flag to tell if a lake is a riverlake.
 *				Updated to deal with latest GSHHS database (2.0)
 *		1.12 24-MAY-2010: Deal with 2.1 format.
 *		1.13 15-JUL-2011: Now contains improved area information (2.2.0),
 *				 and revised greenwhich flags (now 2-bit; see gshhs.h).
 *				 Also added -A and -G as suggested by José Luis García Pallero,
 *				 as well as -Qe|i to control river-lake output, and -N to
 *				 get a particular level.
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
	struct A {	/* -A */
		GMT_LONG active;
		double min;	/* Cutoff area in km^2 */
	} A;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct G {	/* -G */
		GMT_LONG active;
	} G;
	struct I {	/* -I[<id>|c] */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG id;
	} I;
	struct N {	/* -N<level> */
		GMT_LONG active;
		GMT_LONG level;
	} N;
	struct Q {	/* -Qe|i */
		GMT_LONG active;
		GMT_LONG mode;
	} Q;
};

void *New_gshhs_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GSHHS_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GSHHS_CTRL);

	return (C);
}

void Free_gshhs_Ctrl (struct GMT_CTRL *GMT, struct GSHHS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->Out.file) free (C->Out.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gshhs_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;
	
	GMT_message (GMT, "gshhs %s [API] - Extract data tables from binary GSHHS or WDBII %s data files\n", GSHHS_PROG_VERSION, GSHHS_DATA_VERSION);
	GMT_message (GMT, "usage: gshhs gshhs|wdb_rivers|wdb_borders_[f|h|i|l|c].b [-A<area>] [-G] [-I<id>] [-L] [-N<level>] [-Qe|i] [%s] [%s] [%s] > table\n", GMT_V_OPT, GMT_bo_OPT, GMT_o_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

        GMT_message (GMT, "-A Extract polygons whose area is greater than or equal to <area> (in km^2) [all].\n");
	GMT_message (GMT, "-G Write '%%' at start of each segment header [P or L] (overwrites -M)\n");
	GMT_message (GMT, "   and write 'NaN NaN' after each segment to enable import by GNU Octave or Matlab.\n");
	GMT_message (GMT, "-L List header records only (no data records will be written).\n");
	GMT_message (GMT, "-I Output data for polygon number <id> only.  Use -Ic to get all continent polygons\n");
	GMT_message (GMT, "   [Default is all polygons].\n");
	GMT_message (GMT, "-N Output features whose level matches <level> [Default outputs all levels].\n");
	GMT_message (GMT, "-Q Control river-lakes: Use -Qe to exclude river-lakes, and -Qi to ONLY get river-lakes\n");
	GMT_message (GMT, "   [Default outputs all polygons].\n");
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

			case 'A':
				Ctrl->A.active = TRUE;
				Ctrl->A.min = atof (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (opt->arg[0] == 'c')
					Ctrl->I.mode = 1;
				else
					Ctrl->I.id = atoi (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				Ctrl->N.level = atoi (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'e')
					Ctrl->Q.mode = 1;
				else if (opt->arg[0] == 'i')
					Ctrl->Q.mode = 2;
				else
					Ctrl->Q.mode = 3;	/* Flag the error */
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: No data file specified!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.level < 0, "Syntax error -N: Level cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->A.min < 0.0, "Syntax error -A: area cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->I.id < 0, "Syntax error -I: ID cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && Ctrl->Q.mode == 3, "Syntax error -Q: Append e or i!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#ifdef DEBUG
#define Return(code) {Free_gshhs_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); GMT_memtrack_on (GMT, GMT_mem_keeper); bailout (code);}
#else
#define Return(code) {Free_gshhs_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}
#endif

GMT_LONG GMT_gshhs (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG k, seg_no = 0, is_line = 0, n_alloc = 0, n_seg = 0, max_east = 270000000;
	GMT_LONG error, n_read, m, gmode, level, version, greenwich, is_river, src;
	GMT_LONG must_swab, dim[4] = {1, 0, 2, 0}, OK, first = TRUE;

	double w, e, s, n, area, f_area, scale = 10.0;
	
	char source, marker = 0, container[8], ancestor[8], header[GMT_BUFSIZ], *name[2] = {"polygon", "line"};
	
	FILE *fp = NULL;
	
	struct POINT p;
	struct GSHHS h;
 	struct GMT_DATASET *D = NULL;
 	struct GMT_TEXTSET *X = NULL;
 	struct GMT_LINE_SEGMENT **T = NULL;
 	struct GMT_TEXT_SEGMENT *TX = NULL;
	struct GSHHS_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
      
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gshhs_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gshhs_usage (API, GMTAPI_SYNOPSIS));		/* Return the synopsis */

	/* Parse the command-line arguments */

#ifdef DEBUG
	GMT_memtrack_off (GMT, GMT_mem_keeper);
#endif
	GMT = GMT_begin_module (API, "GMT_gshhs", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vbfo:", "m", options)) Return (API->error);
	if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg unless already set */
	Ctrl = New_gshhs_Ctrl (GMT);	/* Allocate and initialize a new control structure */
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

	GMT->current.io.multi_segments[GMT_OUT] = TRUE;	/* Turn on -mo explicitly */
	if (Ctrl->G.active) {
		marker = GMT->current.setting.io_seg_marker[GMT_OUT];
		GMT->current.setting.io_seg_marker[GMT_OUT] = '%';
	}
	else
		GMT->current.io.io_header[GMT_OUT] = TRUE;	/* Turn on -ho explicitly */
	if (Ctrl->L.active) {	/* Want a text set of headers back */
		dim[1] = 1;
		dim[2] = n_alloc = (Ctrl->I.active) ? ((Ctrl->I.mode) ? 6 : 1) : GSHHS_MAXPOL;
		if ((X = GMT_Create_Data (API, GMT_IS_TEXTSET, dim)) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a text set for GSHHS header features.\n");
			return (API->error);
		}
	}
	else {
		dim[1] = n_alloc = 0;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a data set for GSHHS features.\n");
			return (API->error);
		}
	}
	sprintf (header, "# Data extracted from GSHHS file %s", Ctrl->In.file);
	if (Ctrl->L.active) {	/* Want a text set of headers back */
		X->table[0]->header = GMT_memory (GMT, NULL, 1, char *);
		X->table[0]->header[0] = strdup (header);
		X->table[0]->n_headers = 1;
		TX = X->table[0]->segment[0];	/* There is only one output table with one segment */
	}
	else {
		D->table[0]->header = GMT_memory (GMT, NULL, 1, char *);
		D->table[0]->header[0] = strdup (header);
		D->table[0]->n_headers = 1;
		n_alloc = (Ctrl->I.active) ? ((Ctrl->I.mode) ? 6 : 1) : GSHHS_MAXPOL;
		D->table[0]->segment = GMT_memory (GMT, T, n_alloc, struct GMT_LINE_SEGMENT *);
		T = D->table[0]->segment;	/* There is only one output table with one or many segments */
	}
	n_read = fread (&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
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
		/* OK, we want to return info for this feature */
		
		level = h.flag & 255;				/* Level is 1-4 */
		version = (h.flag >> 8) & 255;			/* Version is 1-7 */
		if (first) GMT_report (GMT, GMT_MSG_NORMAL, "Found GSHHS/WDBII version %ld in file %s\n", version, Ctrl->In.file);
		first = FALSE;
		greenwich = (h.flag >> 16) & 3;			/* Greenwich is 0-3 */
		src = (h.flag >> 24) & 1;			/* Source is 0 (WDBII) or 1 (WVS) */
		is_river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
		m = h.flag >> 26;				/* Magnitude for area scale */
		w = h.west  * GSHHS_SCL;			/* Convert region from microdegrees to degrees */
		e = h.east  * GSHHS_SCL;
		s = h.south * GSHHS_SCL;
		n = h.north * GSHHS_SCL;
		source = (src == 1) ? 'W' : 'C';		/* Either WVS or CIA (WDBII) pedigree */
		if (is_river) source = (char)tolower ((int)source);	/* Lower case c means river-lake */
		is_line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
		if (version >= 9) {				/* Magnitude for area scale */
			m = h.flag >> 26;
			scale = pow (10.0, (double)m);		/* Area scale */
		}
		area = h.area / scale;				/* Now im km^2 */
		f_area = h.area_full / scale;			/* Now im km^2 */
		
		OK = ((!Ctrl->I.active || ((!Ctrl->I.mode && h.id == Ctrl->I.id) || (Ctrl->I.mode && h.id <= 5))) && area >= Ctrl->A.min);	/* Skip if not the one (-I) or too small (-A) */
		if (OK && Ctrl->Q.active && ((is_river && Ctrl->Q.mode == 1) || (!is_river && Ctrl->Q.mode == 2))) OK = FALSE;	/* Skip if riverlake/not riverlake (-Q) */
		if (OK && Ctrl->N.active && Ctrl->N.level != level) OK = 0;		/* Skip if not the right level (-N) */
		if (!OK) {	/* Not what we are looking for, skip to next */
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
			n_read = fread (&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);	/* Get the next GSHHS header */
			continue;	/* Back to top of loop */
		}
		

		if (Ctrl->L.active) {	/* Want a text set of headers back */
			if (seg_no == n_alloc) {	/* Must add more segments to this table first */
				n_alloc <<= 2;
				TX->record = GMT_memory (GMT, TX->record, n_alloc, char *);
			}
		}
		else {
			dim[3] = h.n + Ctrl->G.active;	/* Number of data records to allocate for this segment/polygon*/
			if (seg_no == n_alloc) {	/* Must add more segments to this table first */
				n_alloc <<= 2;
				T = GMT_memory (GMT, T, n_alloc, struct GMT_LINE_SEGMENT *);
			}
		}

		/* Create the segment/polygon header record */
		if (is_line) {	/* River or border line-segment */
			sprintf (header, "%6d%8d%3ld%2c%11.5f%10.5f%10.5f%10.5f", h.id, h.n, level, source, w, e, s, n);
			max_east = 180000000;	/* For line segments we always use -180/+180  */
		}
		else {		/* Island or lake polygon */
			(h.container == -1) ? sprintf (container, "-") : sprintf (container, "%6d", h.container);
			(h.ancestor == -1) ? sprintf (ancestor, "-") : sprintf (ancestor, "%6d", h.ancestor);
			sprintf (header, "%6d%8d%2ld%2c %.12g %.12g%11.5f%11.5f%10.5f%10.5f %s %s", h.id, h.n, level, source, area, f_area, w, e, s, n, container, ancestor);
		}

		if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
			TX->record[seg_no] = strdup (header);
			TX->n_rows++;
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
		}
		else {	/* Return the data points also */
			/* Place the header in the output data structure */
			T[seg_no] = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);
			T[seg_no]->header = strdup (header);
			if (h.id == 0)	/* Special longitude range for Eurasia since it crosses Greenwich and Dateline */
				T[seg_no]->range = GMT_IS_M180_TO_P270_RANGE;
			else if (h.id == 4)	/* Special longitude range for Antarctica since it crosses Greenwich and Dateline */
				T[seg_no]->range = GMT_IS_M180_TO_P180_RANGE;
			else
				T[seg_no]->range = (greenwich & 2) ? GMT_IS_0_TO_P360_RANGE : GMT_IS_M180_TO_P180_RANGE;
			/* Allocate h.n number of data records */
			GMT_alloc_segment (GMT, T[seg_no], dim[3], dim[2], TRUE);
			for (k = 0; k < h.n; k++) {
				if (fread (&p, (size_t)sizeof(struct POINT), (size_t)1, fp) != 1) {
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
			T[seg_no]->coord[GMT_X][k] = T[seg_no]->coord[GMT_Y][k] = GMT->session.d_NaN;
			D->n_records += T[seg_no]->n_rows;
		}
		seg_no++;
		max_east = 180000000;	/* Only Eurasia (the first polygon) needs 270 */
		n_read = fread(&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);	/* Get the next GSHHS header */
	}
	GMT_fclose (GMT, fp);
	
	if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
		if (seg_no < n_alloc) {	/* Allocate to final size table */
			TX->record = GMT_memory (GMT, TX->record, seg_no, char *);
		}
		X->n_records = X->table[0]->n_records = TX->n_rows;
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_BY_SET)) Return (API->error);	/* Enables data output and sets access mode */
		if (GMT_Put_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_TEXT, NULL, 0, Ctrl->Out.file, X)) Return (API->error);
	}
	else {
		if (seg_no < n_alloc) {	/* Allocate to final size table */
			D->table[0]->segment = GMT_memory (GMT, D->table[0]->segment, seg_no, struct GMT_LINE_SEGMENT *);
		}
		D->n_segments = D->table[0]->n_segments = seg_no;
		gmode = (is_line) ? GMT_IS_LINE : GMT_IS_POLY;
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET)) Return (API->error);	/* Enables data output and sets access mode */
		if (GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, gmode, NULL, 0, Ctrl->Out.file, D)) Return (API->error);
	}
  	if (GMT_End_IO (API, GMT_OUT, 0)) Return (API->error);				/* Disables further data output */

	GMT_report (GMT, GMT_MSG_NORMAL, "%s in: %ld %s out: %ld\n", name[is_line], n_seg, name[is_line], seg_no);

	if (Ctrl->G.active) GMT->current.setting.io_seg_marker[GMT_OUT] = marker;

	Return (GMT_OK);
}
