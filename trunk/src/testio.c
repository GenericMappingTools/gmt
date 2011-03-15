/*--------------------------------------------------------------------
 *	$Id: testio.c,v 1.2 2011-03-15 02:06:37 guru Exp $
 *
 *	Copyright (c) 1991-$year by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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

#include "gmt.h"

EXTERN_MSC GMT_LONG GMT_mapproject_cmd (struct GMTAPI_CTRL *API, GMT_LONG argc, char *argv[]);

int main (int argc, char *argv[]) {

	GMT_LONG status = 0, in_ID, out_ID, row, col, ij;
	struct GMTAPI_CTRL *API = NULL;			/* GMT API control structure */
	float x[4] = {1.0, 2.0, 3.0, 4.0}, y[4] = {0.5, 1.5, 2.5, 3.5}, z[4] = {3.3f, 8.1f, 2.9f, 4.4f};
	char i_string[GMTAPI_STRLEN], o_string[GMTAPI_STRLEN], buffer[BUFSIZ];
	struct GMT_VECTOR *Vi = NULL, *Vo = NULL;
	struct GMT_GRID *G = NULL;
	
	/* 1. Initializing new GMT session */
	if (GMT_Create_Session (&API, "TEST", GMTAPI_GMT)) exit (EXIT_FAILURE);

	Vi = GMT_create_vector (API->GMT, 3);
	Vi->data[0] = (void *)x;	Vi->data[1] = (void *)y;	Vi->data[2] = (void *)z;
	Vo = GMT_create_vector (API->GMT, 3);
	Vi->type[0] = Vi->type[1] = Vi->type[2] = GMTAPI_FLOAT;
	Vi->n_rows = 4;
	Vi->n_columns = 3;

	if (GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_READONLY + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, (void **)&Vi, NULL, (void *)Vi, &in_ID)) exit (EXIT_FAILURE);

	Vo->type[0] = Vo->type[1] = Vo->type[2] = GMTAPI_DOUBLE;
	Vo->alloc_mode = GMT_REFERENCE;	/* To tell mapproject to allocate as needed */
	if (GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_COPY + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_OUT, (void **)&Vo, NULL, (void *)Vo, &out_ID)) exit (EXIT_FAILURE);

	/* 4. Create command options for GMT_mapproject */

	GMT_Encode_ID (API, i_string, in_ID);	/* Make filename with embedded object ID */
	GMT_Encode_ID (API, o_string, out_ID);	/* Make filename with embedded object ID */
	sprintf (buffer, "-<%s -R0/5/0/5 -Jm1 -Fk -bi3 ->%s", i_string, o_string);
	
	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_mapproject_cmd (API, 0, (void *)buffer);

	/* 6. Create command options for GMT_xyz2grd */

	if (GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_READONLY + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, (void **)&Vi, NULL, (void *)Vi, &in_ID)) exit (EXIT_FAILURE);
	if (GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_OUT, (void **)&G, NULL, (void *)G, &out_ID)) exit (EXIT_FAILURE);
	GMT_Encode_ID (API, i_string, in_ID);	/* Make filename with embedded object ID */
	GMT_Encode_ID (API, o_string, out_ID);	/* Make filename with embedded object ID */
	sprintf (buffer, "-<%s -R0/3/0/3 -I1 -G%s", i_string, o_string);
	
	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_xyz2grd_cmd (API, 0, (void *)buffer);

	/* Now print out the results locally */
	
	for (row = 0; row < Vo->n_rows; row++) {
		for (col = 0; col < Vo->n_columns; col++) printf ("%g\t", ((double *)Vo->data[col])[row]);
		printf ("\n");
	}
	GMT_free_vector (API->GMT, &Vo, TRUE);
	
	printf ("nx,ny = %d %d\n", G->header->nx, G->header->ny);
	GMT_grd_loop (G, row, col, ij) if (!GMT_is_fnan (G->data[ij])) printf ("%g\n", G->data[ij]);
	
	GMT_free_grid (API->GMT, &G, TRUE);

	/* 6. Create command options for GMT_gmtselect */

	Vo = GMT_create_vector (API->GMT, 3);
	Vo->alloc_mode = GMT_REFERENCE;	/* To tell gmtselect to allocate as needed */
	if (GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_READONLY + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, (void **)&Vi, NULL, (void *)Vi, &in_ID)) exit (EXIT_FAILURE);
	if (GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_COPY + GMT_VIA_VECTOR, GMT_IS_POINT, GMT_OUT, (void **)&Vo, NULL, (void *)Vo, &out_ID)) exit (EXIT_FAILURE);
	GMT_Encode_ID (API, i_string, in_ID);	/* Make filename with embedded object ID */
	GMT_Encode_ID (API, o_string, out_ID);	/* Make filename with embedded object ID */
	sprintf (buffer, "-<%s -R0/3/0/3 ->%s", i_string, o_string);
	
	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	fprintf (stderr, "\ngmtselect output\n");
	status = GMT_gmtselect_cmd (API, 0, (void *)buffer);
	GMT_free_vector (API->GMT, &Vi, FALSE);
	for (row = 0; row < Vo->n_rows; row++) {
		for (col = 0; col < Vo->n_columns; col++) printf ("%g\t", ((double *)Vo->data[col])[row]);
		printf ("\n");
	}
	GMT_free_vector (API->GMT, &Vo, TRUE);
	GMT_free_vector (API->GMT, &Vi, FALSE);

	/* 8. Destroy GMT session */
	if (GMT_Destroy_Session (&API)) exit (EXIT_FAILURE);

	exit (GMT_OK);		/* Return the status from this program */
}
