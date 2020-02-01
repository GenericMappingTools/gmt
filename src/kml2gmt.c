/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: kml2gmt is a reformatter that takes KML files and extracts GMT tables;
 * it is the opposite of kml2gmt.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"kml2gmt"
#define THIS_MODULE_MODERN_NAME	"kml2gmt"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Extract GMT table data from Google Earth KML files"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:Vbdh" GMT_OPT("HMm")

#define POINT			0
#define LINE			1
#define POLYGON			2

struct KML2GMT_CTRL {
	struct KML2GMT_In {	/* in file */
		bool active;
		char *file;
	} In;
	struct KML2GMT_E {	/* -E */
		bool active;
	} E;
	struct KML2GMT_F {	/* -F */
		bool active;
		unsigned int mode;
		unsigned int geometry;
	} F;
	struct KML2GMT_Z {	/* -Z */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct KML2GMT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct KML2GMT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct KML2GMT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<kmlfiles>] [-E] [-Fs|l|p] [%s] [-Z] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		name, GMT_V_OPT, GMT_bo_OPT, GMT_do_OPT, GMT_ho_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<kmlfiles> is one or more Google Earth KML files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  If no file(s) is given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<kmlfiles> is one or more KML files from Google Earth or similar.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Get Z from the ExtendData property (only single <SimpleData name=\"string\"> implemented so far).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Restrict feature type; choose from (s)symbol, (l)ine, or (p)olygon.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use to only output data for the selected feature type [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Output the z-column from the KML file [Only lon,lat is output].\n");
	GMT_Option (API, "V,bo,do,h,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct KML2GMT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to kml2gmt and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'E':	/* Feature type */
		 		Ctrl->E.active = true;
 				Ctrl->Z.active = true;	/* Needs this too */
				break;
			case 'F':	/* Feature type */
		 		Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 's':
						Ctrl->F.mode = POINT;
						break;
					case 'l':
						Ctrl->F.mode = LINE;
						Ctrl->F.geometry = GMT_IS_LINE;
						break;
					case 'p':
						Ctrl->F.mode = POLYGON;
						Ctrl->F.geometry = GMT_IS_POLY;
						break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad feature type. Use s, l or p.\n");
						n_errors++;
						break;
				}
				break;
			case 'Z':
 				Ctrl->Z.active = true;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Only one file can be processed at the time\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.active && access (Ctrl->In.file, R_OK), "Cannot read file %s\n", Ctrl->In.file);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_kml2gmt (void *V_API, int mode, void *args) {
	unsigned int i, start, fmode = POINT, n_features = 0, pos;
	int error = 0, n_scan;
	size_t length;
	bool scan = true, first = true, skip, single = false, extended = false;

	char buffer[GMT_BUFSIZ] = {""}, name[GMT_BUFSIZ] = {""};
	char word[GMT_LEN128] = {""}, description[GMT_BUFSIZ] = {""};
	char *gm[3] = {"Point", "Line", "Polygon"}, *line = NULL;

	double out[3], elev;

	FILE *fp = NULL;

	struct GMT_RECORD *Out = NULL;
	struct KML2GMT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
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

	/*---------------------------- This is the kml2gmt main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input KML data\n");
	gmt_set_geographic (GMT, GMT_IN);
	gmt_set_geographic (GMT, GMT_OUT);
	gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

	GMT_Set_Columns (API, GMT_OUT, 2 + Ctrl->Z.active, GMT_COL_FIX_NO_TEXT);
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, Ctrl->F.geometry) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	/* We read the input from stdin or file via fscanf and fgets. We cannot easily
	 * switch this over to using GMT_Get_Record since the kml file may have multiple
	 * coordinate pairs/triplets on the same line.  It is also unlikely anyone really
	 * needs to call GMT_kml2gmt with a memory pointer, so this is a small sacrifice.
	 * P. Wessel, April 2013. */

	if (Ctrl->In.active) {
		if ((fp = gmt_fopen (GMT, Ctrl->In.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->In.file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Processing %s\n", Ctrl->In.file);
		sprintf (buffer, "kml2gmt: KML read from %s", Ctrl->In.file);
	}
	else {     /* Just read standard input */
		fp = stdin;
		GMT_Report (API, GMT_MSG_INFORMATION, "Reading from standard input\n");
		sprintf (buffer, "kml2gmt: KML read from standard input");
	}
	if (Ctrl->F.active)
		GMT_Report (API, GMT_MSG_INFORMATION, "Only output features with geometry: %s\n", gm[Ctrl->F.mode]);

	Out = gmt_new_record (GMT, out, buffer);

	/* Now we are ready to take on some input values */

	strcpy (GMT->current.setting.format_float_out, "%.12g");	/* Get enough decimals */

	GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, Out);	/* Write this to output */
	Out->text = NULL;
	line = gmt_M_memory (GMT, NULL, GMT_INITIAL_MEM_ROW_ALLOC, char);

	while (fgets (line, GMT_INITIAL_MEM_ROW_ALLOC, fp)) {
		if (strstr (line, "<Placemark")) {	/* New Placemark, reset name and description */
			scan = true;
			name[0] = description[0] = 0;
		}
		if (strstr (line, "</Placemark")) scan = false;
		if (!scan) continue;
		if (strstr (line, "<Point")) fmode = POINT;
		if (strstr (line, "<LineString")) fmode = LINE;
		if (strstr (line, "<Polygon")) fmode = POLYGON;
		skip = (Ctrl->F.active && fmode != Ctrl->F.mode);
		length = strlen (line);
		if (strstr (line, "<name>")) {
			for (i = 0; i < length && line[i] != '>'; i++);	/* Find end of <name> */
			start = i + 1;
			for (i = start; i < length && line[i] != '<'; i++);	/* Find start of </name> */
			line[i] = '\0';
			strncpy (name, &line[start], GMT_BUFSIZ-1);
			gmt_chop (name);
			if (first && !skip) {
				sprintf (buffer, "%s", &line[start]);
				Out->text = buffer;
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, Out);	/* Write this to output */
				first = false;
				Out->text = NULL;
			}
		}
		if (strstr (line, "<description>")) {
			for (i = 0; i < length && line[i] != '>'; i++);	/* Find end of <description> */
			start = i + 1;
			for (i = start; i < length && line[i] != '<'; i++);	/* Find start of </description> */
			line[i] = '\0';
			strncpy (description, &line[start], GMT_BUFSIZ-1);
			gmt_chop (description);
			if (first && !skip) {
				sprintf (buffer, "%s", &line[start]);
				Out->text = buffer;
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, Out);	/* Write this to output */
				Out->text = NULL;
				first = false;
			}
		}
		if (skip) continue;
		if (name[0] || description[0]) {
			GMT->current.io.segment_header[0] = 0;
			if (name[0]) { strcat (GMT->current.io.segment_header, "-L\""); strcat (GMT->current.io.segment_header, name); strcat (GMT->current.io.segment_header, "\""); }
			if (name[0] && description[0]) strcat (GMT->current.io.segment_header, " ");
			if (description[0]) { strcat (GMT->current.io.segment_header, "-D\""); strcat (GMT->current.io.segment_header, description); strcat (GMT->current.io.segment_header, "\""); }
		}

		if (Ctrl->E.active && strstr (line, "<ExtendedData>")) {
			/* https://developers.google.com/kml/documentation/kmlreference#extendeddata
			   But only a single <SimpleData name="string" is implemented here. */
			fgets (line, GMT_INITIAL_MEM_ROW_ALLOC, fp);
			if (strstr (line, "<SchemaData")) {
				fgets (line, GMT_INITIAL_MEM_ROW_ALLOC, fp);
				if (strstr (line, "<SimpleData")) {
					char *p1, *p2;
					p1 = strchr(line, '>');		p1++;           /* Find end of <SimpleData */
					p2 = strchr (&p1[0], '<');	p2[0] = '\0';   /* Find begin of </SimpleData>*/
					elev = atof(p1);
					extended = true;
				}
				else
					extended = false;
			}
		}

		if (!strstr (line, "<coordinates>")) continue;
		/* We get here when the line says coordinates */

		if (fmode == POINT && strstr (line, "</coordinates>")) {	/* Process the single point */
			if (!GMT->current.io.segment_header[0]) sprintf (GMT->current.io.segment_header, "Next Point");
		}
		else {
			if (!GMT->current.io.segment_header[0]) sprintf (GMT->current.io.segment_header, "Next feature");
		}
		GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);	/* Write segment header */

		single = (strstr (line, "</coordinates>") != NULL);	/* All on one line */

		if (fmode == POINT && single) {	/* Process the single point from current record */
			for (i = 0; i < length && line[i] != '>'; i++);		/* Find end of <coordinates> */
			sscanf (&line[i+1], "%lg,%lg,%lg", &out[GMT_X], &out[GMT_Y], &out[GMT_Z]);
			if (extended) out[GMT_Z] = elev;
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
		}
		else if (single) {	/* Process multiple points from current single record */
			for (i = 0; i < length && line[i] != '>'; i++);		/* Find end of <coordinates> */
			pos = i + 1;
			while (gmt_strtok (line, " \t", &pos, word)) {	/* Look for clusters of x,y,z separated by whitespace */
				n_scan = sscanf (word, "%lg,%lg,%lg", &out[GMT_X], &out[GMT_Y], &out[GMT_Z]);
				if (extended) out[GMT_Z] = elev;
				if (n_scan == 2 || n_scan == 3)
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
				else
					break;
			}
		}
		else {	/* Processes points from separate lines */
			while (fscanf (fp, "%lg,%lg,%lg", &out[GMT_X], &out[GMT_Y], &out[GMT_Z])) {
				if (extended) out[GMT_Z] = elev;	/* Replave Z by the extended data value */
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
		}
		n_features++;
	}
	if (fp != stdin) fclose (fp);
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	gmt_M_free (GMT, Out);
	gmt_M_free (GMT, line);

	GMT_Report (API, GMT_MSG_INFORMATION, "Found %u features with selected geometry\n", n_features);

	Return (GMT_NOERROR);
}
