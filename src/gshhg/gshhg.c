/*
 *	Copyright (c) 1996-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 * PROGRAM:	gshhg.c
 * AUTHOR:	Paul Wessel (pwessel@hawaii.edu)
 * CREATED:	JAN. 28, 1996
 * DATE:	JAN 1, 2015
 * PURPOSE:	To extract ASCII data from the binary GSHHG geography database
 *		as described in the 1996 Wessel & Smith JGR Data Analysis Note.
 * VERSION:	1-JAN-2015.  For use with GSHHG version 2.3.4
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
 *	Contact info: www.generic-mapping-tools.org */

#include "gmt_dev.h"
#include "gmt_gshhg.h"

#define THIS_MODULE_CLASSIC_NAME	"gshhg"
#define THIS_MODULE_MODERN_NAME	"gshhg"
#define THIS_MODULE_LIB		"gshhg"
#define THIS_MODULE_PURPOSE	"Extract data tables from binary GSHHG or WDBII data files"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:Vbdo"

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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GSHHG_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GSHHG_CTRL);

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GSHHG_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->Out.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s gshhs|wdb_rivers|wdb_borders_[f|h|i|l|c].b [-A<area>] [-G] [-I<id>] [-L] [-N<level>]\n\t[-Qe|i] [%s] [%s] [%s] [%s] [%s]\n\n",
		name, GMT_V_OPT, GMT_bo_OPT, GMT_do_OPT, GMT_o_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tgshhs|wdb_rivers|wdb_borders_[f|h|i|l|c].b is a GSHHG polygon or line file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
        GMT_Message (API, GMT_TIME_NONE, "\t-A Extract polygons whose area is greater than or equal to <area> (in km^2) [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Write '%%' at start of each segment header [P or L] (overwrites -M)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and write 'NaN NaN' after each segment to enable import by MATLAB or GNU Octave.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L List header records only (no data records will be written).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Output data for polygon number <id> only.  Use -Ic to get all continent polygons\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is all polygons].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Output features whose level matches <level> [Default outputs all levels].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Control river-lakes: Use -Qe to exclude river-lakes, and -Qi to ONLY get river-lakes\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default outputs all polygons].\n");
	GMT_Option (API, "V,bo2,do,o,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GSHHG_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (n_files == 1 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET))
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
					n_errors += gmt_M_check_condition (GMT, sval < 0, "Option -I: ID cannot be negative!\n");
					Ctrl->I.id = sval;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				sval = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, sval < 0, "Option -N: Level cannot be negative!\n");
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
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "No data file specified!\n");
	n_errors += gmt_M_check_condition (GMT, n_files == 1 && strstr (Ctrl->In.file, ".nc"), "gshhs does not read GMT netCDF coastline files!  See man page for binary files.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->A.min < 0.0, "Option -A: area cannot be negative!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->Q.mode == 3, "Option -Q: Append e or i!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gshhg (void *V_API, int mode, void *args) {
	unsigned int row, seg_no = 0, is_line = 0, n_seg = 0, m, level, this_id;
	int error, gmode, version, greenwich, is_river, src;
	int32_t max_east = 270000000;
	size_t n_read;
	bool OK, first = true;
#ifdef WORDS_BIGENDIAN
	static const bool must_swab = false;
#else
	static const bool must_swab = true;
#endif

	uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 2};

	size_t n_alloc = 0;

	double w, e, s, n, area, f_area, scale = 10.0;

	char source, marker = 0, header[GMT_BUFSIZ] = {""}, *name[2] = {"polygon", "line"};
	char west[GMT_LEN64] = {""}, east[GMT_LEN64] = {""}, south[GMT_LEN64] = {""}, north[GMT_LEN64] = {""};

	FILE *fp = NULL;

	struct GSHHG_POINT p;
	struct GSHHG_HEADER h;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT **T = NULL;
	struct GSHHG_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */
	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gshhg main code ----------------------------*/

	if (gmt_access (GMT, Ctrl->In.file, F_OK)) {
		GMT_Report (API, GMT_MSG_ERROR, "Cannot find file %s\n", Ctrl->In.file);
		Return (GMT_FILE_NOT_FOUND);
	}
	if ((fp = gmt_fopen (GMT, Ctrl->In.file, "rb")) == NULL ) {
		GMT_Report (API, GMT_MSG_ERROR, "Cannot read file %s\n", Ctrl->In.file);
		Return (GMT_ERROR_ON_FOPEN);
	}

	gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
	gmt_set_geographic (GMT, GMT_IN);
	gmt_set_geographic (GMT, GMT_OUT);
	if (Ctrl->G.active) {
		marker = GMT->current.setting.io_seg_marker[GMT_OUT];
		GMT->current.setting.io_seg_marker[GMT_OUT] = '%';
	}
	else
		GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on -ho explicitly */
	if (Ctrl->L.active) {	/* Want a text set of headers back */
		dim[GMT_SEG] = 1;	dim[GMT_COL] = 0;
		dim[GMT_ROW] = n_alloc = (Ctrl->I.active) ? ((Ctrl->I.mode) ? 6 : 1) : GSHHG_MAXPOL;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET|GMT_WITH_STRINGS, GMT_IS_TEXT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for GSHHG header features.\n");
			gmt_fclose (GMT, fp);
			Return (API->error);
		}
		T = &D->table[0]->segment[0];	/* There is only one output table with one segment */
	}
	else {
		dim[GMT_SEG] = 0;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for GSHHG features.\n");
			gmt_fclose (GMT, fp);
			Return (API->error);
		}
		n_alloc = (Ctrl->I.active) ? ((Ctrl->I.mode) ? 6 : 1) : GSHHG_MAXPOL;
		D->table[0]->segment = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_DATASEGMENT *);
		T = D->table[0]->segment;	/* There is only one output table with one or many segments */
	}
	sprintf (header, "# Data extracted from GSHHG file %s", Ctrl->In.file);
	D->table[0]->header = gmt_M_memory (GMT, NULL, 1, char *);
	D->table[0]->header[0] = strdup (header);
	D->table[0]->n_headers = 1;

	n_read = fread (&h, sizeof (struct GSHHG_HEADER), 1U, fp);

	while (n_read == 1) {
		n_seg++;
		if (must_swab) /* Must deal with different endianness */
			bswap_GSHHG_struct (&h);

		/* OK, we want to return info for this feature */

		level = h.flag & 255;				/* Level is 1-4 [5-6 for Antarctica] */
		version = (h.flag >> 8) & 255;			/* Version is 1-255 */
		if (first) GMT_Report (API, GMT_MSG_INFORMATION, "Found GSHHG/WDBII version %d in file %s\n", version, Ctrl->In.file);
		first = false;
		greenwich = (h.flag >> 16) & 3;			/* Greenwich is 0-3 */
		src = (h.flag >> 24) & 1;			/* Source is 0 (WDBII) or 1 (WVS) */
		is_river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
		w = h.west  * GSHHG_SCL;			/* Convert region from microdegrees to degrees */
		e = h.east  * GSHHG_SCL;
		s = h.south * GSHHG_SCL;
		n = h.north * GSHHG_SCL;
		source = (level > 4) ? 'A' : ((src == 1) ? 'W' : 'C');		/* Either Antarctica, WVS or CIA (WDBII) pedigree */
		if (is_river) source = (char)tolower ((int)source);	/* Lower case c means river-lake */
		is_line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
		if (version >= 9) {				/* Magnitude for area scale */
			m = h.flag >> 26;
			scale = pow (10.0, (double)m);		/* Area scale */
		}
		area = h.area / scale;				/* Now in km^2 */
		f_area = h.area_full / scale;			/* Now in km^2 */
		this_id = h.id;

		OK = ((!Ctrl->I.active || ((!Ctrl->I.mode && this_id == Ctrl->I.id) || (Ctrl->I.mode && this_id <= 5))) && area >= Ctrl->A.min);	/* Skip if not the one (-I) or too small (-A) */
		if (OK && Ctrl->Q.active && ((is_river && Ctrl->Q.mode == 1) || (!is_river && Ctrl->Q.mode == 2))) OK = false;	/* Skip if riverlake/not riverlake (-Q) */
		if (OK && Ctrl->N.active && Ctrl->N.level != level) OK = 0;		/* Skip if not the right level (-N) */
		if (!OK) {	/* Not what we are looking for, skip to next */
			fseek (fp, (off_t)(h.n * sizeof(struct GSHHG_POINT)), SEEK_CUR);
			n_read = fread (&h, sizeof (struct GSHHG_HEADER), 1U, fp);	/* Get the next GSHHG header */
			continue;	/* Back to top of loop */
		}


		if (Ctrl->L.active) {	/* Want a text set of headers back */
			if (seg_no == n_alloc) {	/* Must add more segments to this table first */
				n_alloc <<= 2;
				T[0]->text = gmt_M_memory (GMT, T[0]->text, n_alloc, char *);
			}
		}
		else {
			if (is_line && D->geometry == GMT_IS_POLY) D->geometry = GMT_IS_LINE;	/* Change from polygon to line geometry */
			dim[GMT_ROW] = h.n + Ctrl->G.active;	/* Number of data records to allocate for this segment/polygon*/
			if (seg_no == n_alloc) {	/* Must add more segments to this table first */
				size_t old_n_alloc = n_alloc;
				n_alloc <<= 1;
				T = gmt_M_memory (GMT, T, n_alloc, struct GMT_DATASEGMENT *);
				gmt_M_memset (&(T[old_n_alloc]), n_alloc - old_n_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
			}
		}

		/* Format w/e/s/n for header according to users format choice */
		gmt_ascii_format_col (GMT, west,  w, GMT_OUT, GMT_X);
		gmt_ascii_format_col (GMT, east,  e, GMT_OUT, GMT_X);
		gmt_ascii_format_col (GMT, south, s, GMT_OUT, GMT_Y);
		gmt_ascii_format_col (GMT, north, n, GMT_OUT, GMT_Y);

		/* Create the segment/polygon header record */
		if (is_line) {	/* River or border line-segment */
			sprintf (header, "%6d%8d%3d%2c %s %s %s %s", h.id, h.n, level, source, west, east, south, north);
			max_east = 180000000;	/* For line segments we always use -180/+180  */
		}
		else		/* Island or lake polygon */
			sprintf (header, "%6d%8d%2d%2c %.12g %.12g %s %s %s %s %6d %6d", h.id, h.n, level, source, area, f_area, west, east, south, north, h.container, h.ancestor);

		if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
			T[0]->text[seg_no] = strdup (header);
			T[0]->n_rows++;
			fseek (fp, (off_t)(h.n * sizeof(struct GSHHG_POINT)), SEEK_CUR);
		}
		else {	/* Return the data points also */
			struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
			/* Place the header in the output data structure */
			T[seg_no] = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, dim[GMT_ROW], dim[GMT_COL], header, NULL);
			SH = gmt_get_DS_hidden (T[seg_no]);
			if (h.id == 0)	/* Special longitude range for Eurasia since it crosses Greenwich and Dateline */
				SH->range = GMT_IS_M180_TO_P270_RANGE;
			else if (h.id == 4)	/* Special longitude range for Antarctica since it crosses Greenwich and Dateline */
				SH->range = GMT_IS_M180_TO_P180_RANGE;
			else
				SH->range = (greenwich & 2) ? GMT_IS_0_TO_P360_RANGE : GMT_IS_M180_TO_P180_RANGE;
			for (row = 0; row < h.n; row++) {
				if (fread (&p, sizeof (struct GSHHG_POINT), 1U, fp) != 1) {
					GMT_Report (API, GMT_MSG_ERROR, "Failure while reading file %s for %s %d, point %d.\n", Ctrl->In.file, name[is_line], h.id, row);
					gmt_fclose (GMT, fp);
					Return (GMT_DATA_READ_ERROR);
				}
				if (must_swab) /* Must deal with different endianness */
					bswap_POINT_struct (&p);
				T[seg_no]->data[GMT_X][row] = p.x * GSHHG_SCL;
				if ((greenwich && p.x > max_east) || (h.west > 180000000)) T[seg_no]->data[GMT_X][row] -= 360.0;
				T[seg_no]->data[GMT_Y][row] = p.y * GSHHG_SCL;
			}
			T[seg_no]->n_rows = h.n;
			if (Ctrl->G.active) T[seg_no]->data[GMT_X][row] = T[seg_no]->data[GMT_Y][row] = GMT->session.d_NaN;
			D->n_records += T[seg_no]->n_rows;
		}
		seg_no++;
		max_east = 180000000;	/* Only Eurasia (the first polygon) needs 270 */
		n_read = fread (&h, sizeof (struct GSHHG_HEADER), 1U, fp);	/* Get the next GSHHG header */
	}
	gmt_fclose (GMT, fp);

	if (Ctrl->L.active) {	/* Skip data, only wanted the headers */
		if (seg_no < n_alloc) {	/* Allocate to final size table */
			T[0]->text = gmt_M_memory (GMT, T[0]->text, seg_no, char *);
		}
		D->n_records = D->table[0]->n_records = T[0]->n_rows;
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {
		if (seg_no < n_alloc) {	/* Allocate to final size table */
			D->table[0]->segment = gmt_M_memory (GMT, D->table[0]->segment, seg_no, struct GMT_DATASEGMENT *);
		}
		D->n_segments = D->table[0]->n_segments = seg_no;
		gmode = (is_line) ? GMT_IS_LINE : GMT_IS_POLY;
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, gmode, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "%s in: %d %s out: %d\n", name[is_line], n_seg, name[is_line], seg_no);

	if (Ctrl->G.active) GMT->current.setting.io_seg_marker[GMT_OUT] = marker;

	Return (GMT_NOERROR);
}
