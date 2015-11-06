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
 * Testing grid i/o for complex grids.  Needs the two test grids
 * in_real.grd and in_imag.grd to be present in the current directory.
 * Also takes an optional argument =<gridformatcode>, e.g. =bf
 * for testing that particular grid format [Default uses GMT defaults]
 * On output the grids out_real.grd and out_real_after_demux.grd should
 * be identical to in_real.grd and out_imag.grd and out_imag_after_demux.grd
 * should be identical to in_imag.grd.
 *
 * Version:	5
 * Created:	22-Mar-2013
 *
 */

#include "gmt_dev.h"

int main (int argc, char *argv[])
{
	unsigned int k;
	char *format[6] = {"in_real.grd%s", "in_imag.grd%s", "out_real.grd%s", "out_imag.grd%s", "out_real_after_demux.grd%s", "out_imag_after_demux.grd%s"};
	char *name[6], *code, *def_code = "", buffer[GMT_LEN64] = {""};
	struct GMTAPI_CTRL *API = NULL;	/* GMT API control structure */
	struct GMT_GRID *G_real = NULL, *G_imag = NULL, *G_cplx = NULL;
	
	/* 0. Create file names using the formats and code given [if any] */
	code = (argc == 2) ? argv[1] : def_code;
	for (k = 0; k < 6; k++) {
		sprintf (buffer, format[k], code);
		name[k] = strdup (buffer);
	}

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. Read in real and imaginary components into one grid  */
	if ((G_cplx = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_REAL, NULL, name[0], NULL)) == NULL) exit (API->error);
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY | GMT_GRID_IS_COMPLEX_IMAG, NULL, name[1], G_cplx) == NULL) exit (API->error);

	/* 3. Write out real and imaginary components from one grid  */
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_REAL, NULL, name[2], G_cplx) != GMT_OK) exit (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_IMAG, NULL, name[3], G_cplx) != GMT_OK) exit (API->error);
	
	/* 4. Read in real components into one grid, multiplex, then demultiplex, then write out real again */
	if ((G_real = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_REAL, NULL, name[0], NULL)) == NULL) exit (API->error);
	GMT_grd_mux_demux (API->GMT, G_real->header, G_real->data, GMT_GRID_IS_INTERLEAVED);	/* First multiplex ... */
	GMT_grd_mux_demux (API->GMT, G_real->header, G_real->data, GMT_GRID_IS_SERIAL);		/* Then demultiplex .. */
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_REAL, NULL, name[4], G_real) != GMT_OK) exit (API->error);

	/* 5. Read in imag component into one grid, multiplex, then demultiplex, then write out imag again */
	if ((G_imag = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_IMAG, NULL, name[1], NULL)) == NULL) exit (API->error);
	GMT_grd_mux_demux (API->GMT, G_real->header, G_real->data, GMT_GRID_IS_INTERLEAVED);	/* First multiplex ... */
	GMT_grd_mux_demux (API->GMT, G_real->header, G_real->data, GMT_GRID_IS_SERIAL);		/* Then demultiplex .. */
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL | GMT_GRID_IS_COMPLEX_IMAG, NULL, name[5], G_imag) != GMT_OK) exit (API->error);

	for (k = 0; k < 6; k++) free ((void *)name[k]);

	/* 8. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_OK);
}
