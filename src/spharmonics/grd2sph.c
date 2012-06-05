/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * grd2sph.c reads a grid file and outputs spherical harmonic coefficients.
 *
 * Author:	Paul Wessel
 * Date:	1-JUN-2006
 * Version:	4.x
 */

#include "gmt.h"

struct GRD2SPH_CTRL {
	struct D {	/* -D<degree> */
		bool active;
		int max_degree;
	} D;
	struct N {	/* -Ng|m|s */
		bool active;
		char mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
};

int main (int argc, char **argv)
{
	bool error = false;

	int i, d, o, nm, f_arg = -1, n_files = 0;

	float *grd;

	double out[4];

	struct GRD_HEADER header;
	struct GRD2SPH_CTRL *Ctrl;

	void *New_grd2sph_Ctrl (), Free_grd2sph_Ctrl (struct GRD2SPH_CTRL *C);

	argc = GMT_begin (argc, argv);

	Ctrl = (struct GRD2SPH_CTRL *)New_grd2sph_Ctrl ();	/* Allocate and initialize a new control structure */

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				/* Common parameters */

				case 'b':
				case 'H':
				case 'V':
				case '\0':
					error += GMT_parse_common_options (argv[i], NULL, NULL, NULL, NULL);
					break;

				/* Supplemental options */

				case 'D':
					Ctrl->D.active = true;
					if (argv[i][2]) Ctrl->D.max_degree = atoi (&argv[i][2]);
					break;
				case 'N':
					Ctrl->N.active = true;
					Ctrl->N.mode = argv[i][2];
					break;
				case 'Q':
					Ctrl->Q.active = true;
					break;

				default:
					error = true;
					GMT_default_error (argv[i][1]);
					break;
			}
		}
		else {
			n_files++;
			f_arg = i;
		}
	}

	if (argc == 1 || GMT->common.synopsis.active) {
		fprintf (stderr, "grd2sph %s - Convert a global grid file to a table of spherical harmonic coefficients\n\n", GMT_VERSION);
		fprintf( stderr, "usage: grd2sph <grdfile> -D<degree> [-N<norm>] [-Q] [-V] [%s] [%s] > coeff_file\n\n", GMT_bo_OPT, GMT_h_OPT);

		if (GMT->common.synopsis.active) exit (EXIT_FAILURE);

		fprintf (stderr, "\n\t<grdfile> is the global grid file to convert\n");
		fprintf (stderr, "\t-D The highest degree term to include in the conversion\n");
		fprintf (stderr, "\n\tOPTIONS:\n");
		fprintf (stderr, "\t-H Write one ASCII header record [Default is no header]\n");
		fprintf (stderr, "\t-N Normalization of coefficients.  Choose among\n");
		fprintf (stderr, "\t   m: Mathematical normalization - inner products summed over surface equal 1 [Default]\n");
		fprintf (stderr, "\t   g: Geodesy normalization - inner products summed over surface equal 4pi\n");
		fprintf (stderr, "\t   s: Schmidt normalization - as used in geomagnetism\n");
		fprintf (stderr, "\t-Q Use phase convention from physics, i.e., include (-1)^m factor\n");
		GMT_explain_options ("VD4o.");
		exit (EXIT_FAILURE);
	}

	if (n_files == 0) {
		fprintf (stderr, "%s: Syntax error: Must specify at least one input grid\n", GMT->init.module_name);
		error++;
	}
	if (n_files > 1) {
		fprintf (stderr, "%s: Syntax error: Can only handle one input grid\n", GMT->init.module_name);
		error++;
	}
	if (Ctrl->D.max_degree <= 0) {
		fprintf (stderr, "%s: Syntax error: -D maximum degree must be positive\n", GMT->init.module_name);
		error++;
	}
	if (!(Ctrl->N.mode == 'm' || Ctrl->N.mode == 'g' || Ctrl->N.mode == 's')) {
		fprintf (stderr, "%s: Syntax error: -N Normalization must be one of m, g, or s\n", GMT->init.module_name);
		error++;
	}
	if (GMT->current.io.info.binary[GMT_OUT] && GMT->current.io.info.io_header[GMT_OUT]) {
		fprintf (stderr, "%s: Syntax error: Binary output data cannot have header -H\n", GMT->init.module_name);
		error++;
	}

	if (error) exit (EXIT_FAILURE);

#ifdef SET_IO_MODE
	GMT_setmode (GMT, GMT_OUT);
#endif

	GMT_grd_init (&header, 0, NULL, false);
	GMT_err_fail (GMT_read_grd_info (argv[f_arg], &header), argv[f_arg]);

	if (!(GMT_grd_is_global (&header) && GMT_180_RANGE (header.y_min, header.y_max))) {
		fprintf (stderr, "%s: File %s is not a global grid (it has -R%g/%g/%g/%g)\n", GMT->init.module_name, argv[f_arg], header.x_min, header.x_max, header.y_min, header.y_max);
		exit (EXIT_FAILURE);
	}

	if (GMT->current.setting.verbose) fprintf (stderr, "%s: Working on file %s\n", GMT->init.module_name, argv[f_arg]);

	nm = header.nx * header.ny;

	grd = GMT_memory (GMT, NULL, nm, float);

	GMT_err_fail (GMT_read_grd (argv[f_arg], &header, grd, 0.0, 0.0, 0.0, 0.0, GMT->current.io.pad, false), argv[f_arg]);
	/* Do conversion to spherical harmonic coefficients */

	/* Write out coefficients in the form degree order cos sin */

	if (GMT->current.io.info.io_header[GMT_OUT]) printf ("#degree\torder\tcos\tsin\n");
	out[2] = out[3] = 0.0;
	for (d = 0; d <= Ctrl->D.max_degree; d++) {
		out[0] = (double)d;
		for (o = 0; o <= d; o++) {
			out[1] = (double)o;
			/* out[2] = cos_do;
			out[3] = sin_do; */
			GMT->current.io.output (GMT, GMT->session.stdout, 4, out);
		}

	}

	GMT_free (grd);

	if (GMT->current.setting.verbose) fprintf (stderr, "%s: Completed\n", GMT->init.module_name);

	Free_grd2sph_Ctrl (Ctrl);	/* Deallocate control structure */

	GMT_end (argc, argv);

	exit (EXIT_SUCCESS);
}

void *New_grd2sph_Ctrl () {	/* Allocate and initialize a new control structure */
	struct GRD2SPH_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRD2SPH_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->N.mode = 'm';
		
	return (C);
}

void Free_grd2sph_Ctrl (struct GRD2SPH_CTRL *C) {	/* Deallocate control structure */
	GMT_free (C);	
}
