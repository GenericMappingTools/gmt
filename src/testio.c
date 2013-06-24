/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-$year by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Demonstration program for how GMT5 may handle i/o and passing data
 * from one GMT_* module to another.
 *
 * Currently shows how "grdcut t.nc -R2/4/2/4 -Gnew.nc -V" is obtained
 * but calling GMT_grdcut from this main.
 *
 * Version:	5
 * Created:	13-Nov-2009
 *
 */

#include "gmt_dev.h"

int main (int argc, char *argv[]) {

	int status = 0, xrow, in_ID, out_ID;
	uint64_t row, col, ij;
	struct GMTAPI_CTRL *API = NULL;			/* GMT API control structure */
	float x[4] = {1.0f, 2.0f, 3.0f, 4.0f}, y[4] = {0.5f, 1.5f, 2.5f, 3.5f}, z[4] = {3.3f, 8.1f, 2.9f, 4.4f};
	char i_string[GMT_STR16], o_string[GMT_STR16], buffer[GMT_BUFSIZ];
	struct GMT_VECTOR *Vi = NULL, *Vo = NULL;
	struct GMT_GRID *G = NULL;
	
	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", 2U, 0U, NULL)) == NULL) exit (EXIT_FAILURE);

	Vi = GMT_create_vector (API->GMT, 3);
	Vi->type[0] = Vi->type[1] = Vi->type[2] = GMT_FLOAT;
	Vi->n_rows = 4;
	Vi->data[0].f4 = x;	Vi->data[1].f4 = y;	Vi->data[2].f4 = z;
	Vo = GMT_create_vector (API->GMT, 3);

	if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_REFERENCE + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, NULL, Vi)) == GMT_NOTSET) exit (EXIT_FAILURE);

	Vo->type[0] = Vo->type[1] = Vo->type[2] = GMT_DOUBLE;
	if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_DUPLICATE + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_OUT, NULL, NULL)) == GMT_NOTSET) exit (EXIT_FAILURE);

	/* 4. Create command options for GMT_mapproject */

	if (GMT_Encode_ID (API, i_string, in_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if (GMT_Encode_ID (API, o_string, out_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (buffer, "-<%s -R0/5/0/5 -Jm1 -Fk -bi3 ->%s", i_string, o_string);
	
	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, "mapproject", GMT_MODULE_CMD, buffer);
	if (status) {
		GMT_Report (API, GMT_MSG_NORMAL, "GMT_mapproject returned error %d\n", status);
		exit (EXIT_FAILURE);
	}
	if ((Vo = GMT_Retrieve_Data (API, out_ID)) == NULL) exit (EXIT_FAILURE);

	/* 6. Create command options for GMT_xyz2grd */

	if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_REFERENCE + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, NULL, Vi)) == GMT_NOTSET) exit (EXIT_FAILURE);
	if ((out_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMT_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, i_string, in_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if (GMT_Encode_ID (API, o_string, out_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (buffer, "-<%s -R0/3/0/3 -I1 -G%s", i_string, o_string);
	
	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, "xyz2grd", GMT_MODULE_CMD, buffer);
	if (status) {
		GMT_Report (API, GMT_MSG_NORMAL, "GMT_xyz2grd returned error %d\n", status);
		exit (EXIT_FAILURE);
	}
	if ((G = GMT_Retrieve_Data (API, out_ID)) == NULL) exit (EXIT_FAILURE);

	/* Now print out the results locally */
	
	for (row = 0; row < Vo->n_rows; row++) {
		for (col = 0; col < Vo->n_columns; col++) printf ("%g\t", Vo->data[col].f8[row]);
		printf ("\n");
	}
	GMT_free_vector (API->GMT, &Vo, true);
	
	printf ("nx,ny = %d %d\n", G->header->nx, G->header->ny);
	GMT_grd_loop (API->GMT, G, xrow, col, ij) if (!GMT_is_fnan (G->data[ij])) printf ("%g\n", G->data[ij]);
	
	if (GMT_Destroy_Data (API, &G) != GMT_OK) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free G\n");
	}

	/* 6. Create command options for GMT_gmtselect */

	Vo = GMT_create_vector (API->GMT, 3);
	if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_REFERENCE + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, NULL, Vi)) == GMT_NOTSET) exit (EXIT_FAILURE);
	if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_DUPLICATE + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_OUT, NULL, NULL)) == GMT_NOTSET) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, i_string, in_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if (GMT_Encode_ID (API, o_string, out_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (buffer, "-<%s -R0/3/0/3 ->%s", i_string, o_string);
	
	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	GMT_Message (API, GMT_TIME_NONE, "\ngmtselect output\n");
	status = GMT_Call_Module (API, "gmtselect", GMT_MODULE_CMD, buffer);
	if (status) {
		GMT_Report (API, GMT_MSG_NORMAL, "GMT_gmtselect returned error %d\n", status);
		exit (EXIT_FAILURE);
	}
	if ((Vo = GMT_Retrieve_Data (API, out_ID)) == NULL) exit (EXIT_FAILURE);
	GMT_free_vector (API->GMT, &Vi, false);
	for (row = 0; row < Vo->n_rows; row++) {
		for (col = 0; col < Vo->n_columns; col++) printf ("%g\t", Vo->data[col].f8[row]);
		printf ("\n");
	}
	GMT_free_vector (API->GMT, &Vo, true);
	GMT_free_vector (API->GMT, &Vi, false);

	/* 8. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_OK);		/* Return the status from this program */
}
