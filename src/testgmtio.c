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
 * Demonstrate how to use the table i/o machinery on a record-by-record
 * basis, including the various checks for gaps, headers, etc.
 *
 * Version:	5
 * Created:	19-Mar-2010
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_OPTIONS "->Vghi"

int main (int argc, char *argv[]) {
	int n_fields = 0, mode = 0,  error = 0;
	struct GMT_RECORD *In = NULL;
	struct GMT_OPTION *options = NULL;	/* Linked list of options */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct GMT_CTRL *GMT = NULL;

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);
	GMT = API->GMT;

	/* 2. Convert command line arguments to local linked option list */
	if ((options = GMT_Create_Options (API, argc-1, (argv+1))) == NULL) exit (EXIT_FAILURE);

	/* 3. Parse the common GMT options (e.g., -h -V) */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) exit (EXIT_FAILURE);

	/* 4. Initializing data input via stdin */

	if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) exit (EXIT_FAILURE);
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) exit (EXIT_FAILURE);	/* Establishes data input */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) exit (EXIT_FAILURE);	/* Establishes data output */

	/* 5. Read individual records until end of data set */
	/*    The GMT_READ_FILEBREAK in GMT_Get_Record means we will return a special EOF marker at the end of each
	 *    data table when there are more tables to process.  The end of the last file yields the actual EOF.
	 *    This lets us do special processing after a file has been fully read. */

	/* Initialize the i/o for doing record-by-record reading/writing */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON)) != GMT_NOERROR) exit (error);				/* Enables data input and sets access mode */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON)) != GMT_NOERROR) exit (error);				/* Enables data output and sets access mode */

	do {	/* Keep returning records until we reach EOF */
		mode = GMT_WRITE_DATA;	/* Normally we treat data as double precision values */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA | GMT_READ_FILEBREAK, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {	/* This check kicks in if the data has bad formatting, text etc */
				GMT_Report (API, GMT_MSG_WARNING, "Error found in record %" PRIu64 "\n", GMT->current.io.rec_no);
				API->print_func (stdout, "E: ");
				mode = GMT_WRITE_TEXT;	/* Switch to text so we can see the bad record as is */
			}
			if (gmt_M_rec_is_file_break (GMT)) {	/* End of a file but not end of all files */
				GMT_Report (API, GMT_MSG_WARNING, "End of intermediate data file after record %" PRIu64 "\n", GMT->current.io.rec_no);
				API->print_func (stdout, "B: --- End of File except last one ---\n");
				continue;	/* Since no actual data record was returned, just the intermediate "EOF" signal */
			}
			if (gmt_M_rec_is_table_header (GMT)) {	/* Found a table header */
				GMT_Report (API, GMT_MSG_WARNING, "Table header found in record %" PRIu64 "\n", GMT->current.io.rec_no);
				API->print_func (stdout, "H: ");
				mode = GMT_WRITE_TABLE_HEADER;	/* Special flag to report the table header */
			}
			if (gmt_M_rec_is_segment_header (GMT)) {	/* Found segment header */
				GMT_Report (API, GMT_MSG_WARNING, "Segment header found in record %" PRIu64 "\n", GMT->current.io.rec_no);
				API->print_func (stdout, "S: ");
				mode = GMT_WRITE_SEGMENT_HEADER;	/* Special flag to report the segment header */
			}
			if (gmt_M_rec_is_nan (GMT)) {	/* Found NaN record */
				GMT_Report (API, GMT_MSG_WARNING, "NaN data found in record %" PRIu64 "\n", GMT->current.io.rec_no);
				API->print_func (stdout, "N: ");
				mode = GMT_WRITE_TEXT;	/* Switch to text so we can see the nan record as is */
			}
			if (gmt_M_rec_is_gap (GMT)) {	/* Found a gap */
				GMT_Report (API, GMT_MSG_WARNING, "A gap found in record %" PRIu64 "\n", GMT->current.io.rec_no);
				API->print_func (stdout, "G: ");
			}
			assert (false);						/* Should never get here */
		}
		if (gmt_M_rec_is_data (GMT)) {	/* Found a data record */
			if ((error = GMT_Set_Columns (API, GMT_OUT, n_fields, gmt_M_colmode (In->text))) != GMT_NOERROR) exit (EXIT_FAILURE);
			GMT_Report (API, GMT_MSG_WARNING, "Data found in record %" PRIu64 "\n", GMT->current.io.rec_no);
			API->print_func (stdout, "D: ");
		}
		GMT_Put_Record (API, mode, In);
	} while (true);

	API->print_func (stdout, "B: --- End of All Files ---\n");
	if ((error = GMT_End_IO (API, GMT_IN,  0)) != GMT_NOERROR) exit (error);				/* Disables further data input */
	if ((error = GMT_End_IO (API, GMT_OUT, 0)) != GMT_NOERROR) exit (error);				/* Disables further data output */

	/* 5. Destroy local linked option list */
	if (GMT_Destroy_Options (API, &options)) exit (EXIT_FAILURE);

	/* 6. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_NOERROR);		/* Return the status from this program */
}
