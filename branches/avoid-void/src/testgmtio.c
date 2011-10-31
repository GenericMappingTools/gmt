/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Demonstrate how to use the table i/o machinery on a record-by-record
 * basis, including the various checks for gaps, headers, etc.
 *
 * Version:	5
 * Created:	19-Mar-2010
 *
 */

#include "gmt.h"

int main (int argc, char *argv[]) {
	GMT_LONG n_fields = 0, mode = 0,  error = 0;
	double *in = NULL;
	struct GMT_OPTION *options = NULL;	/* Linked list of options */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct GMT_CTRL *GMT = NULL;

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMTAPI_GMT)) == NULL) exit (EXIT_FAILURE);
	GMT = API->GMT;

	/* 2. Convert command line arguments to local linked option list */
	if ((options = GMT_Create_Options (API, (GMT_LONG)(argc-1), (argv+1))) == NULL) exit (EXIT_FAILURE);

	/* 3. Parse the common GMT options (e.g., -h -V) */
	if (GMT_Parse_Common (API, "-VJRbf:", "BKOPUXYcghp", options)) exit (EXIT_FAILURE);

	/* 4. Initializing data input via stdin */
	
	if ((error = GMT_set_cols (GMT, GMT_IN, 0))) exit (EXIT_FAILURE);
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options)) exit (EXIT_FAILURE);	/* Establishes data input */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options)) exit (EXIT_FAILURE);	/* Establishes data output */
	
	/* 5. Read individual records until end of data set */
	/*    The GMT_FILE_BREAK in GMT_Get_Record means we will return a special EOF marker at the end of each
	 *    data table when there are more tables to process.  The end of the last file yields the actual EOF.
	 *    This lets us do special processing after a file has been fully read. */

	/* Initialize the i/o for doing record-by-record reading/writing */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) exit (error);				/* Enables data input and sets access mode */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) exit (error);				/* Enables data output and sets access mode */
	
	while ((in = GMT_Get_Record (API, GMT_READ_DOUBLE | GMT_FILE_BREAK, &n_fields))) {	/* Keep returning records until we reach EOF */
		mode = GMT_WRITE_DOUBLE;	/* Normally we treat data as double precision values */
		if (GMT_REC_IS_ERROR (GMT)) {	/* This check kicks in if the data has bad formatting, text etc */
			GMT_report (GMT, GMT_MSG_NORMAL, "Error found in record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "E: ");
			mode = GMT_WRITE_TEXT;	/* Switch to text so we can see the bad record as is */
			in = NULL;
		}
		if (GMT_REC_IS_FILE_BREAK (GMT)) {	/* End of a file but not end of all files */
			GMT_report (GMT, GMT_MSG_NORMAL, "End of intermediate data file after record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "B: --- End of File except last one ---\n");
			continue;	/* Since no actual data record was returned, just the intermediate "EOF" signal */
		}
		if (GMT_REC_IS_TBL_HEADER (GMT)) {	/* Found a table header */
			GMT_report (GMT, GMT_MSG_NORMAL, "Table header found in record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "H: ");
			mode = GMT_WRITE_TBLHEADER;	/* Special flag to report the table header */
		}
		if (GMT_REC_IS_SEG_HEADER (GMT)) {	/* Found segment header */
			GMT_report (GMT, GMT_MSG_NORMAL, "Segment header found in record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "S: ");
			mode = GMT_WRITE_SEGHEADER;	/* Special flag to report the segment header */
		}
		if (GMT_REC_IS_NAN (GMT)) {	/* Found NaN record */
			GMT_report (GMT, GMT_MSG_NORMAL, "NaN data found in record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "N: ");
			mode = GMT_WRITE_TEXT;	/* Switch to text so we can see the nan record as is */
			in = NULL;
		}
		if (GMT_REC_IS_GAP (GMT)) {	/* Found a gap */
			GMT_report (GMT, GMT_MSG_NORMAL, "A gap found in record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "G: ");
		}
		if (GMT_REC_IS_DATA (GMT)) {	/* Found a data record */
			if ((error = GMT_set_cols (GMT, GMT_IN, n_fields))) exit (EXIT_FAILURE);
			if ((error = GMT_set_cols (GMT, GMT_OUT, n_fields))) exit (EXIT_FAILURE);
			GMT_report (GMT, GMT_MSG_NORMAL, "Data found in record %ld\n", GMT->current.io.rec_no);
			fprintf (stdout, "D: ");
		}
		GMT_Put_Record (API, mode, in);
	}
	fprintf (stdout, "B: --- End of All Files ---\n");
	if ((error = GMT_End_IO (API, GMT_IN,  0))) exit (error);				/* Disables further data input */
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) exit (error);				/* Disables further data output */
	
	/* 5. Destroy local linked option list */
	if (GMT_Destroy_Options (API, &options)) exit (EXIT_FAILURE);

	/* 6. Destroy GMT session */
	if (GMT_Destroy_Session (&API)) exit (EXIT_FAILURE);

	exit (GMT_OK);		/* Return the status from this program */
}
