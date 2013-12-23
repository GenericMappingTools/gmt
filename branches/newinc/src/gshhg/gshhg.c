/*	$Id$
 *
 *	Copyright (c) 1996-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 * PROGRAM:	gshhg.c
 * AUTHOR:	Paul Wessel (pwessel@hawaii.edu)
 * CREATED:	JAN. 28, 1996
 * PURPOSE:	To extract ASCII data from the binary GSHHG shoreline data
 *		as described in the 1996 Wessel & Smith JGR Data Analysis Note.
 * VERSION:	1.1 (Byte flipping added)
 *		1.2 18-MAY-1999:
 *		   Explicit binary open for DOS systems
 *		   POSIX.1 compliant
 *		1.3 08-NOV-1999: Released under GNU GPL
 *		1.4 05-SEPT-2000: Made a GMT supplement; FLIP no longer needed
 *		1.5 14-SEPT-2004: Updated to deal with latest GSHHG database (1.3)
 *		1.6 02-MAY-2006: Updated to deal with latest GSHHG database (1.4)
 *		1.7 11-NOV-2006: Fixed bug in computing level (&& vs &)
 *		1.8 31-MAR-2007: Updated to deal with latest GSHHG database (1.5)
 *		1.9 27-AUG-2007: Handle line data as well as polygon data
 *		1.10 15-FEB-2008: Updated to deal with latest GSHHG database (1.6)
 *		1.11 15-JUN-2009: Now contains information on container polygon,
 *				the polygons ancestor in the full resolution, and
 *				a flag to tell if a lake is a riverlake.
 *				Updated to deal with latest GSHHG database (2.0)
 *		1.12 24-MAY-2010: Deal with 2.1 format.
 *		1.13 15-JUL-2011: Now contains improved area information (2.2.0),
 *				 and revised greenwich flags (now 2-bit; see gshhg.h).
 *				 Also added -A and -G as suggested by José Luis García Pallero,
 *				 as well as -Qe|i to control river-lake output, and -N to
 *				 get a particular level.
 *		1.14 15-APR-2012:  	Data version is now 2.2.1. [no change to format]
 *		1.15 1-JAN-2013:   	Data version is now 2.2.2. [no change to format]
 *		1.16 1-JUL-2013:   	Data version is now 2.2.3. [no change to format]
 *		1-NOV-2013.   PW: Data version is now 2.2.4. [no change to format]
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.soest.hawaii.edu/pwessel */

#define THIS_MODULE_NAME	"gshhg"
#define THIS_MODULE_LIB		"gshhg"
#define THIS_MODULE_PURPOSE	"Extract data tables from binary GSHHS or WDBII data files"

#include "gmt_dev.h"
#include "gshhg.h"

#define GMT_PROG_OPTIONS "-:Vbo"

struct GSHHG_CTRL {
	struct In {	/* <file> */
		bool active;
		char *file;
	} In;
	struct Out {	/* > <file> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A */
		bool active;
		double min;	/* Cutoff area in km^2 */
	} A;
	struct L {	/* -L */
		bool active;
	} L;
	struct G {	/* -G */
		bool active;
	} G;
	struct I {	/* -I[<id>|c] */
		bool active;
		unsigned int mode;
		unsigned int id;
	} I;
	struct N {	/* -N<level> */
		bool active;
		unsigned int level;
	} N;
	struct Q {	/* -Qe|i */
		bool active;
		unsigned int mode;
	} Q;
};

void *New_gshhg_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GSHHG_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GSHHG_CTRL);

	return (C);
}

void Free_gshhg_Ctrl (struct GMT_CTRL *GMT, struct GSHHG_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->Out.file) free (C->Out.file);	
	GMT_free (GMT, C);	
}

int GMT_gshhg_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gshhg gshhs|wdb_rivers|wdb_borders_[f|h|i|l|c].b [-A<area>] [-G] [-I<id>] [-L] [-N<level>]\n\t[-Qe|i] [%s] [%s] [%s] > table\n", GMT_V_OPT, GMT_bo_OPT, GMT_o_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

        GMT_Message (API, GMT_TIME_NONE, "-A Extract polygons whose area is greater than or equal to <area> (in km^2) [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "-G Write '%%' at start of each segment header [P or L] (overwrites -M)\n");
	GMT_Message (API, GMT_TIME_NONE, "   and write 'NaN NaN' after each segment to enable import by GNU Octave or Matlab.\n");
	GMT_Message (API, GMT_TIME_NONE, "-L List header records only (no data records will be written).\n");
	GMT_Message (API, GMT_TIME_NONE, "-I Output data for polygon number <id> only.  Use -Ic to get all continent polygons\n");
	GMT_Message (API, GMT_TIME_NONE, "   [Default is all polygons].\n");
	GMT_Message (API, GMT_TIME_NONE, "-N Output features whose level matches <level> [Default outputs all levels].\n");
	GMT_Message (API, GMT_TIME_NONE, "-Q Control river-lakes: Use -Qe to exclude river-lakes, and -Qi to ONLY get river-lakes\n");
	GMT_Message (API, GMT_TIME_NONE, "   [Default outputs all polygons].\n");
	GMT_Option (API, "V,bo2,o,:,.");
	
	return (EXIT_FAILURE);
}
	
int GMT_gshhg_parse (struct GMT_CTRL *GMT, struct GSHHG_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gshhg and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int sval;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Count input files */
				n_files++;
				if (n_files == 1 && GMT_check_filearg (GMT, '<', opt->arg, GMT_IN))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			case '>':	/* Got output file */
				Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				Ctrl->A.min = atof (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = true;
				break;
			case 'L':
				Ctrl->L.active = true;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (opt->arg[0] == 'c')
					Ctrl->I.mode = 1;
				else {
					sval = atoi (opt->arg);
					n_errors += GMT_check_condition (GMT, sval < 0, "Syntax error -I: ID cannot be negative!\n");
					Ctrl->I.id = sval;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				sval = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, sval < 0, "Syntax error -N: Level cannot be negative!\n");
				Ctrl->N.level = sval;
				break;
			case 'Q':
				Ctrl->Q.active = true;
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
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->A.min < 0.0, "Syntax error -A: area cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && Ctrl->Q.mode == 3, "Syntax error -Q: Append e or i!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gshhg_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gshhg (void *V_API, int mode, void *args)
{
	unsigned int row, seg_no = 0, is_line = 0, n_seg = 0, m, level, this_id;
	int error, gmode, version, greenwich, is_river, src;
	int32_t max_east = 270000000;
	size_t n_read;
	bool must_swab, OK, first = true;

	uint64_t dim[4] = {1, 0, 0, 2};

	size_t n_alloc = 0;

	double w, e, s, n, area, f_area, scale = 10.0;

	char source, marker = 0, container[8] = {""}, ancestor[8] = {""}, header[GMT_BUFSIZ] = {""}, *name[2] = {"polygon", "line"};

	FILE *fp = NULL;

	struct POINT p;
	struct GSHHG h;
	struct GMT_DATASET *D = NULL;
	struct GMT_TEXTSET *X = NULL;
	struct GMT_DATASEGMENT **T = NULL;
	struct GMT_TEXTSEGMENT *TX = NULL;
	struct GSHHG_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gshhg_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gshhg_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gshhg_usage (API, GMT_SYNOPSIS));		/* Return the synopsis */

	/* Parse the command-line arguments */
	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg unless already set */
	Ctrl = New_gshhg_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gshhg_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gshhg main code ----------------------------*/

	if (GMT_access (GMT, Ctrl->In.file, F_OK)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot find file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}
	if ((fp = GMT_fopen (GMT, Ctrl->In.file, "rb")) == NULL ) {
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot read file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}

	GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
	if (Ctrl->G.active) {
		marker = GMT->current.setting.io_seg_marker[GMT_OUT];
		GMT->current.setting.io_seg_marker[GMT_OUT] = '%';
	}
	else
		GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on -ho explicitly */
	if (Ctrl->L.active) {	/* Want a text set of headers back */
		dim[GMT_SEG] = 1;
		dim[GMT_ROW] = n_alloc = (Ctrl->I.active) ? ((Ctrl->I.mode) ? 6 : 1) : GSHHG_MAXPOL;
		if ((X = GMT_Create_Data (API, GMT_IS_TEXTSET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, Ctrl->Out.file)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a text set for GSHHG header features.\n");
			return (API->error);
		}
	}
	else {
		dim[GMT_SEG] = n_alloc = 0;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, Ctrl->Out.file)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for GSHHG features.\n");
			return (API->error);
		}
	}
	sprintf (header, "# Data extracted from GSHHG file %s", Ctrl->In.file);
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
		n_alloc = (Ctrl->I.active) ? ((Ctrl->I.mode) ? 6 : 1) : GSHHG_MAXPOL;
		D->table[0]->segment = GMT_memory (GMT, NULL, n_alloc, struct GMT_DATASEGMENT *);
		T = D->table[0]->segment;	/* There is only one output table with one or many segments */
	}
	n_read = fread (&h, sizeof (struct GSHHG), 1U, fp);
	version = (h.flag >> 8) & 255;
	must_swab = (version != GSHHG_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */

	while (n_read == 1) {
		n_seg++;
		if (must_swab) /* Must deal with different endianness */
			bswap_GSHHG_struct (&h);

		/* OK, we want to return info for this feature */

		level = h.flag & 255;				/* Level is 1-4 */
		version = (h.flag >> 8) & 255;			/* Version is 1-7 */
		if (first) GMT_Report (API, GMT_MSG_VERBOSE, "Found GSHHG/WDBII version %d in file %s\n", version, Ctrl->In.file);
		first = false;
		greenwich = (h.flag >> 16) & 3;			/* Greenwich is 0-3 */
		src = (h.flag >> 24) & 1;			/* Source is 0 (WDBII) or 1 (WVS) */
		is_river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
		m = h.flag >> 26;				/* Magnitude for area scale */
		w = h.west  * GSHHG_SCL;			/* Convert region from microdegrees to degrees */
		e = h.east  * GSHHG_SCL;
		s = h.south * GSHHG_SCL;
		n = h.north * GSHHG_SCL;
		source = (src == 1) ? 'W' : 'C';		/* Either WVS or CIA (WDBII) pedigree */
		if (is_river) source = (char)tolower ((int)source);	/* Lower case c means river-lake */
		is_line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
		if (is_line && D->geometry == GMT_IS_POLY) D->geometry = GMT_IS_LINE;	/* Change from polygon to line geometry */
		if (version >= 9) {				/* Magnitude for area scale */
			m = h.flag >> 26;
			scale = pow (10.0, (double)m);		/* Area scale */
		}
		area = h.area / scale;				/* Now im km^2 */
		f_area = h.area_full / scale;			/* Now im km^2 */
		this_id = h.id;
		
		OK = ((!Ctrl->I.active || ((!Ctrl->I.mode && this_id == Ctrl->I.id) || (Ctrl->I.mode && this_id <= 5))) && area >= Ctrl->A.min);	/* Skip if not the one (-I) or too small (-A) */
		if (OK && Ctrl->Q.active && ((is_river && Ctrl->Q.mode == 1) || (!is_river && Ctrl->Q.mode == 2))) OK = false;	/* Skip if riverlake/not riverlake (-Q) */
		if (OK && Ctrl->N.active && Ctrl->N.level != level) OK = 0;		/* Skip if not the right level (-N) */
		if (!OK) {	/* Not what we are looking for, skip to next */
			fseek (fp, (off_t)(h.n * sizeof(struct POINT)), SEEK_CUR);
			n_read = fread (&h, sizeof (struct GSHHG), 1U, fp);	/* Get the next GSHHG header */
			continue;	/* Back to top of loop */
		}
		

		if (Ctrl->L.active) {	/* Want a text set of headers back */
			if (seg_no == n_alloc) {	/* Must add more segments to this table first */
				n_alloc <<= 2;
				TX->record = GMT_memory (GMT, TX->record, n_alloc, char *);
			}
		}
		else {
			dim[GMT_ROW] = h.n + Ctrl->G.active;	/* Number of data records to allocate for this segment/polygon*/
			if (seg_no == n_alloc) {	/* Must add more segments to this table first */
				size_t old_n_alloc = n_alloc;
				n_alloc <<= 1;
				T = GMT_memory (GMT, T, n_alloc, struct GMT_DATASEGMENT *);
				GMT_memset (&(T[old_n_alloc]), n_alloc - old_n_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
			}
		}

		/* Create the segment/polygon header record */
		if (is_line) {	/* River or border line-segment */
			sprintf (header, "%6d%8d%3d%2c%11.5f%10.5f%10.5f%10.5f", h.id, h.n, level, source, w, e, s, n);
			max_east = 180000000;	/* For line segments we always use -180/+180  */
		}
		else {		/* Island or lake polygon */
			(h.container == -1) ? sprintf (container, "-") : sprintf (container, "%6d", h.container);
			(h.ancestor == -1) ? sprintf (ancestor, "-") : sprintf (ancestor, "%6d", h.ancestor);
			sprintf (header, "%6d%8d%2d%2c %.12g %.12g%11.5f%11.5f%10.5f%10.5f %s %s", h.id, h.n, level, source, area, f_area, w, e, s, n, container, ancestor);
		}

		if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
			TX->record[seg_no] = strdup (header);
			TX->n_rows++;
			fseek (fp, (off_t)(h.n * sizeof(struct POINT)), SEEK_CUR);
		}
		else {	/* Return the data points also */
			/* Place the header in the output data structure */
			T[seg_no] = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
			T[seg_no]->header = strdup (header);
			if (h.id == 0)	/* Special longitude range for Eurasia since it crosses Greenwich and Dateline */
				T[seg_no]->range = GMT_IS_M180_TO_P270_RANGE;
			else if (h.id == 4)	/* Special longitude range for Antarctica since it crosses Greenwich and Dateline */
				T[seg_no]->range = GMT_IS_M180_TO_P180_RANGE;
			else
				T[seg_no]->range = (greenwich & 2) ? GMT_IS_0_TO_P360_RANGE : GMT_IS_M180_TO_P180_RANGE;
			/* Allocate h.n number of data records */
			GMT_alloc_segment (GMT, T[seg_no], dim[GMT_ROW], dim[GMT_COL], true);
			for (row = 0; row < h.n; row++) {
				if (fread (&p, sizeof (struct POINT), 1U, fp) != 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error reading file %s for %s %d, point %d.\n", Ctrl->In.file, name[is_line], h.id, row);
					Return (EXIT_FAILURE);
				}
				if (must_swab) /* Must deal with different endianness */
					bswap_POINT_struct (&p);
				T[seg_no]->coord[GMT_X][row] = p.x * GSHHG_SCL;
				if ((greenwich && p.x > max_east) || (h.west > 180000000)) T[seg_no]->coord[GMT_X][row] -= 360.0;
				T[seg_no]->coord[GMT_Y][row] = p.y * GSHHG_SCL;
			}
			T[seg_no]->coord[GMT_X][row] = T[seg_no]->coord[GMT_Y][row] = GMT->session.d_NaN;
			D->n_records += T[seg_no]->n_rows;
		}
		seg_no++;
		max_east = 180000000;	/* Only Eurasia (the first polygon) needs 270 */
		n_read = fread (&h, sizeof (struct GSHHG), 1U, fp);	/* Get the next GSHHG header */
	}
	GMT_fclose (GMT, fp);
	
	if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
		if (seg_no < n_alloc) {	/* Allocate to final size table */
			TX->record = GMT_memory (GMT, TX->record, seg_no, char *);
		}
		X->n_records = X->table[0]->n_records = TX->n_rows;
		if (GMT_Write_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->Out.file, X) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		if (seg_no < n_alloc) {	/* Allocate to final size table */
			D->table[0]->segment = GMT_memory (GMT, D->table[0]->segment, seg_no, struct GMT_DATASEGMENT *);
		}
		D->n_segments = D->table[0]->n_segments = seg_no;
		gmode = (is_line) ? GMT_IS_LINE : GMT_IS_POLY;
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, gmode, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_OK) {
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "%s in: %d %s out: %d\n", name[is_line], n_seg, name[is_line], seg_no);

	if (Ctrl->G.active) GMT->current.setting.io_seg_marker[GMT_OUT] = marker;

	Return (GMT_OK);
}
