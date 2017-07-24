/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: grd2rgb reads either (1) an 8, 24, or 32 bit Sun rasterfile and writes out the
 * red, green, and blue components in three separate grid files, or (2) a z grd
 * file and a CPT and compute r, g, b and write these out instead.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grd2rgb"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Write r/g/b grid files from a grid file, a raw RGB file, or SUN rasterfile"
#define THIS_MODULE_KEYS	"<G{,CC("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RVh"

struct GRD2RGB_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct G {	/* -G<nametemplate> */
		bool active;
		char *name;
	} G;
	struct L {	/* -L<layer> */
		bool active;
		char layer;
	} L;
	struct W {	/* -W<width/height>[/<n_bytes>] */
		bool active;
		unsigned int n_columns, n_rows;	/* Dimension of image */
		unsigned int size;	/* Number of bytes per pixels */
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2RGB_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRD2RGB_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->G.name = strdup ("grd2rgb_%c.nc");
	C->W.size = 3;	/* 3 bytes per pixel */
		
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRD2RGB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);	
	gmt_M_str_free (C->C.file);	
	gmt_M_str_free (C->G.name);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int loadraw (struct GMT_CTRL *GMT, char *file, struct imageinfo *header, int byte_per_pixel, int n_columns, int n_rows, unsigned char **P) {
	/* loadraw reads a raw binary grb or rgba rasterfile of depth 24, or 32 into memory */

	unsigned int k;
	uint64_t j, i;
	size_t nm;
	unsigned char *buffer = NULL;

	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, file, "rb")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open rasterfile %s!\n", file);
		return (GMT_ERROR_ON_FOPEN);
	}

	/* Lets pretend that the raw file is a sunraster file. This way the grd2rgb code
	   can be used with very little changes */
	header->depth = 24;
	header->width = n_columns;
	header->height = n_rows;
	nm = (size_t)n_columns * (size_t)n_rows * (size_t)byte_per_pixel;
	header->length = (int)nm;

	buffer = gmt_M_memory (GMT, NULL, nm, unsigned char);
	if (gmt_M_fread (buffer, 1U, nm, fp) != nm) {
		if (byte_per_pixel == 3)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Trouble reading raw 24-bit rasterfile!\n");
		if (byte_per_pixel == 4)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Trouble reading raw 32-bit rasterfile!\n");
		gmt_fclose (GMT, fp);
		gmt_M_free (GMT, buffer);
		return (GMT_IMAGE_READ_ERROR);
	}

	if (byte_per_pixel == 4) {		/* RGBA */
		for (i = 3, j = 4; j < nm; i += 3, j += 4) {
			for (k = 0; k < 3; k++) buffer[i+k] = buffer[j+k];
		}
	}

	gmt_fclose (GMT, fp);
	*P = buffer;
	return (0);
}

GMT_LOCAL int guess_width (struct GMT_CTRL *GMT, char *file, unsigned int byte_per_pixel, unsigned int *raw_nx, unsigned int *raw_ny) {
	int inc, even;
	unsigned int k = 0, j, i, l, n_pix;
	unsigned char *buffer = NULL;
	size_t img_size;
	gmt_grdfloat *work = NULL, *datac = NULL, *img_pow = NULL, pow_max = -FLT_MAX, pm;
	int rgb[3];
	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, file, "rb")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open rasterfile %s!\n", file);
		return (GMT_ERROR_ON_FOPEN);
	}

	if (fseek (fp, (off_t)0, SEEK_END)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot seek to end of file %s!\n", file);
		return (GMT_RUNTIME_ERROR);
	}
	
	img_size = ftell (fp);
	if (fseek (fp, 0, SEEK_SET)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot seek to start of file %s!\n", file);
		return (GMT_RUNTIME_ERROR);
	}

	n_pix = (unsigned int) (img_size / byte_per_pixel);

	buffer  = gmt_M_memory (GMT, NULL, img_size, unsigned char);
	datac   = gmt_M_memory (GMT, NULL, 2*n_pix, gmt_grdfloat);
	work    = gmt_M_memory (GMT, NULL, 2*n_pix, gmt_grdfloat);
	img_pow = gmt_M_memory (GMT, NULL, n_pix/2, gmt_grdfloat);
	gmt_M_memset (work, 2*n_pix, gmt_grdfloat);

	if (gmt_M_fread (buffer, 1U, img_size, fp) != img_size) {
		if (byte_per_pixel == 3)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Trouble_ reading raw 24-bit rasterfile!\n");
		if (byte_per_pixel == 4)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Trouble_ reading raw 32-bit rasterfile!\n");
		gmt_M_free (GMT, buffer);
		gmt_M_free (GMT, datac);
		gmt_M_free (GMT, work);
		gmt_M_free (GMT, img_pow);
		return (GMT_IMAGE_READ_ERROR);
	}
	gmt_fclose (GMT, fp);

	inc = (byte_per_pixel == 3) ? 3: 4;
	for (j = 0; j < img_size; j += inc) {
		for (i = 0; i < 3; i++) rgb[i] = buffer[j+i];
		/* Convert rgb to gray using the gmt_M_yiq transformation */
		datac[k] = (gmt_grdfloat) gmt_M_yiq (rgb);
		k += 2;
	}

	if (GMT_FFT_1D (GMT->parent, datac, n_pix, GMT_FFT_FWD, GMT_FFT_COMPLEX))
		return (GMT_RUNTIME_ERROR);

	/* Now compute the image's power spectrum */
	for (k = 0, j = 0; k < n_pix; k += 2, j++) {
		img_pow[j] = (datac[k]*datac[k] + datac[k+1]*datac[k+1]) / n_pix; /* I*I-conj = power */
	}

	/* I'll assume that searching on one fifth of the spectrum is enough to find the
	   line frequency. */
	for (k = 5; k < n_pix/10; k++) {
		if (img_pow[k] > pow_max) {
			pow_max = img_pow[k];	 j = k+1;
		}
	}

	/* That's the way it should be but, I don't know why, the result is transposed. Instead
	   of the number of lines I get number of columns. This is very weird and smells BUG */
	/* *raw_nx = j;		*raw_ny = lrint((gmt_grdfloat)n_pix / raw_nx);*/

	/* So be it */
	*raw_ny = j;
	if (j > 0) *raw_nx = urint((gmt_grdfloat)n_pix / (*raw_ny));

	if ((*raw_nx) * (*raw_ny) != n_pix) {
		/* Let's make another attempt to find the right n_columns * n_rows combination. The idea is that we
	   	failed by a little, so we'll look around the approximate solution by adding 1 to n_columns and
	   	subtracting 1 to n_rows. Then we revert (subtract 1 to n_columns and add 1 to n_rows). Next apply the
	   	same test with an offset of 2, and so on until the offset is 10. */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: first test based on FFT failed to guess image dimensions.\n\tI'll do now a second try\t");
		k = 1;		pm = 1;		l = 1;
		while (k < 41) {
			i = *raw_ny + urint (copysign((double)l, (double)pm));
			pm = -pm;
			j = (*raw_nx) + urint (copysign((double)l, (double)pm));
			if (i*j == n_pix) {	/* Got a good candidate */
				*raw_ny = i;	*raw_nx = j;
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "... SUCCESS (W = %d, H = %d)\n", *raw_nx, *raw_ny);
				break;
			}
			even = (k%2 == 0) ? 1 : 0;
			if (even) l++;
			k++;
		}
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "File %s has %d Lines and %d Cols\n", file, *raw_ny, *raw_nx);

	/* If both attempts failed */
	if ((*raw_nx) * (*raw_ny) != n_pix) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "FAILURE while guessing image dimensions (W = %d, H = %d)\n", *raw_nx, *raw_ny);
		return (GMT_RUNTIME_ERROR);
	}

	gmt_M_free (GMT, buffer);
	gmt_M_free (GMT, datac);
	gmt_M_free (GMT, work);
	gmt_M_free (GMT, img_pow);

	return (GMT_NOERROR);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grd2rgb <infile> [-C<cpt>] [-G<template>] [%s] [-L<layer>]\n", GMT_Id_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W<width>/<height>[/<n_bytes>]] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_r_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<infile> can be one of three different intput files:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  (1) An 8, 24, or 32-bit Sun rasterfile.  Use -I, -R, and -r to change the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      the default values of dx = dy = 1 and region 1/ncols/1/nrows.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  (2) A regular z grid file.  Use -C to provide a CPT with which\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      to convert z to r/g/b triplets. -R, -I, and -r are ignored.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  (3) A RGB or RGBA raw rasterfile. Since raw rasterfiles have no header, you have to\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      give the image dimensions via -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      However, you may take the chance of letting the program try to\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      guess the image dimensions (slow, via FFT spectrum).\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert z to rgb.  If given, we assume a z grid file is provided,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   else we will try to read a Sun rasterfile.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Give outputfile name template for the three red, green, blue grid files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The template MUST contain the format code %%c which will be replaced with r, g, and b\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is grd2rgb_%%c.nc].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify grid size(s).  Append m (or c) to <dx> and/or <dy> for minutes (or seconds).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Only output the given layer (r, g, or b) [Default output all three].\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set the size of the raw raster file. By default an RGB file (which has 3 bytes/pixel)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   is assumed. For RGBA files use n_bytes = 4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -W for guessing the image size of a RGB raw file, and -W=/=/4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   if the raw image is of the RGBA type. Notice that this might be a\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   bit slow because the guessing algorithm makes uses of FFTs.\n");
	GMT_Option (API, "r");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRD2RGB_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, pos, entry;
	bool guess = false;
	char ptr[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	
	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Use CPT */
				Ctrl->C.file = strdup (opt->arg);
				Ctrl->C.active = true;
				break;
			case 'G':	/* Output file template */
				gmt_M_str_free (Ctrl->G.name);	
				Ctrl->G.name = strdup (opt->arg);
				break;
			case 'I':	/* Get grid spacings */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* Select one layer */
				Ctrl->L.layer = opt->arg[0];
				Ctrl->L.active = true;
				break;
			case 'W':	/* Give raw size */
				Ctrl->W.active = true;
				guess = true;
				entry = pos = 0;
				while ((gmt_strtok (opt->arg, "/", &pos, ptr))) {
					if (ptr[0] != '=') {
						switch (entry) {
							case 0:
								Ctrl->W.n_columns = atoi (ptr);
								guess = false; break;
							case 1:
								Ctrl->W.n_rows = atoi (ptr); break;
							case 2:
								Ctrl->W.size = atoi (ptr); break;
						}
					}
					entry++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	if (!Ctrl->C.active) {
		n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input raster file\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] == 0.0 || GMT->common.R.inc[GMT_Y] == 0.0), "Syntax error: increments must be positive\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.size != 3 && Ctrl->W.size != 4, "Syntax error: byte_per_pixel must be either 3 or 4\n");
		if (guess) guess_width (GMT, Ctrl->In.file, Ctrl->W.size, &Ctrl->W.n_columns, &Ctrl->W.n_rows);

		n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.n_columns <= 0, "Syntax error: Witdth of raw raster file must be a positive integer. Not %d\n", Ctrl->W.n_columns);
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.n_rows <= 0, "Syntax error: Height of raw raster file must be a positive integer. Not %d\n", Ctrl->W.n_rows);
	}
	else {
		n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input z grid file\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->C.file, "Syntax error: Must specify CPT\n");
	}
	n_errors += gmt_M_check_condition (GMT, !strstr (Ctrl->G.name, "%c"), "Syntax error: output template must contain %%c\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && !strchr ("rgb", Ctrl->L.layer), "Syntax error: -L layer must be one of r, g, or b\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grd2rgb (void *V_API, int mode, void *args) {
	unsigned int channel, row, col;
	int error = 0;

	uint64_t ij, k, k3;

	char rgb[3] = {'r', 'g', 'b'}, *comp[3] = {"red", "green", "blue"};
	char buffer[GMT_BUFSIZ] = {""}, *grdfile = NULL;

	unsigned char *picture = NULL;

	double f_rgb[4];

	struct imageinfo header;
	struct GMT_GRID *Grid = NULL, *Out = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRD2RGB_CTRL *Ctrl = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL internal parameters */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	memset (&header, 0, sizeof(struct imageinfo)); /* initialize struct */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */
	
	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	PSL = GMT->PSL;	/* This module also needs PSL */

	/*---------------------------- This is the gmt2kml main code ----------------------------*/

	/* No command line files or std** to add via GMT_Init_IO */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid|raster\n");

	if (Ctrl->C.active) {	/* Apply CPT to get three r,g,b channel files */
		/* Since these GMT grids COULD be passed in via memory locations, they COULD have pads so we must use general IJ access */
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		
		for (channel = 0; channel < 3; channel++) {	/* Do the r, g, and b channels */
			if (Ctrl->L.active && Ctrl->L.layer != rgb[channel]) continue;	/* Only do one of the layers */
			GMT_Report (API, GMT_MSG_VERBOSE, "Processing the %s components\n", comp[channel]);
			if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file, NULL)) == NULL) {
				Return (API->error);
			}
			gmt_grd_init (GMT, Grid->header, options, true);

			(void)gmt_set_outgrid (GMT, Ctrl->In.file, false, Grid, &Out);	/* true if input is a read-only array; else Out == Grid */
				
			sprintf (buffer, Ctrl->G.name, rgb[channel]);
			grdfile = strdup (buffer);
			snprintf (Out->header->remark, GMT_GRID_REMARK_LEN160, "Grid of %s components in the 0-255 range", comp[channel]);
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				(void)gmt_get_rgb_from_z (GMT, P, Grid->data[ij], f_rgb);
				Out->data[ij] = (gmt_grdfloat)gmt_M_s255 (f_rgb[channel]);
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, grdfile, Out) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_M_str_free (grdfile);
		}
		if (GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {
		int n_columns, n_rows;
		if (gmt_access (GMT, Ctrl->In.file, R_OK)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open/read file %s\n", Ctrl->In.file);
			Return (GMT_FILE_NOT_FOUND);
		}

		if (!Ctrl->W.active) {
			if (gmt_getdatapath (GMT, Ctrl->In.file, buffer, R_OK) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open file %s\n", Ctrl->In.file);
				Return (GMT_FILE_NOT_FOUND);
			}
			if (PSL_loadimage (PSL, buffer, &header, &picture)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Trouble loading/converting Sun rasterfile %s\n", buffer);
				Return (GMT_IMAGE_READ_ERROR);
			}
		}
		else {
			if (loadraw (GMT, Ctrl->In.file, &header, Ctrl->W.size, Ctrl->W.n_columns, Ctrl->W.n_rows, &picture)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Trouble loading/converting RGB image!\n");
				Return (GMT_IMAGE_READ_ERROR);
			}
		}

		if (header.depth < 8) {
			GMT_Report (API, GMT_MSG_NORMAL, "Sun rasterfile must be at least 8 bits deep\n");
			PSL_free (picture);
			Return (GMT_DIM_TOO_SMALL);
		}

		if (!GMT->common.R.active[ISET]) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Assign default dx = dy = 1\n");
			GMT->common.R.inc[GMT_X] = GMT->common.R.inc[GMT_Y] = 1.0;	GMT->common.R.active[ISET] = true;
		}
		if (!GMT->common.R.active[RSET]) {	/* R not given, provide default */
			GMT->common.R.wesn[XLO] = GMT->common.R.wesn[YLO] = 0.0;
			GMT->common.R.wesn[XHI] = header.width  - 1 + GMT->common.R.registration;
			GMT->common.R.wesn[YHI] = header.height - 1 + GMT->common.R.registration;
			GMT->common.R.active[RSET] = true;
			GMT_Report (API, GMT_MSG_VERBOSE, "Assign default -R0/%g/0/%g\n", GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI]);
		}

		n_columns = (int)gmt_M_get_n (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.inc[GMT_X], GMT->common.R.registration);
		n_rows = (int)gmt_M_get_n (GMT, GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI], GMT->common.R.inc[GMT_Y], GMT->common.R.registration);
		if (Ctrl->W.active && !GMT->common.R.active[ISET]) {		/* This isn't correct because it doesn't deal with -r */
			n_columns = Ctrl->W.n_columns;
			n_rows = Ctrl->W.n_rows;
			GMT->common.R.inc[GMT_X] = gmt_M_get_inc (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], n_columns, GMT->common.R.registration);
			GMT->common.R.inc[GMT_Y] = gmt_M_get_inc (GMT, GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI], n_rows, GMT->common.R.registration);
		}
		if (header.width != n_columns) {
			GMT_Report (API, GMT_MSG_NORMAL, "Sun rasterfile width and -R -I do not match (%d versus %d)  Need -r?\n", header.width, n_columns);
			PSL_free (picture);
			Return (GMT_RUNTIME_ERROR);
		}
		if (header.height != n_rows) {
			GMT_Report (API, GMT_MSG_NORMAL, "Sun rasterfile height and -R -I do not match (%d versus %d)  Need -r?\n", header.height, n_rows);
			PSL_free (picture);
			Return (GMT_RUNTIME_ERROR);
		}

		if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
		                             GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
				PSL_free (picture);
				Return (API->error);
			}
		
		GMT_Report (API, GMT_MSG_VERBOSE, "n_columns = %d  n_rows = %d\n", Grid->header->n_columns, Grid->header->n_rows);

		/* Note: While the picture array has no pads, we must assume the output grid may have one */
		
		for (channel = 0; channel < 3; channel++) {	/* Do the r, g, and b channels */
			if (Ctrl->L.active && Ctrl->L.layer != rgb[channel]) continue;
			GMT_Report (API, GMT_MSG_VERBOSE, "Processing the %s components\n", comp[channel]);
			sprintf (buffer, Ctrl->G.name, rgb[channel]);
			grdfile = strdup (buffer);
			snprintf (Grid->header->remark, GMT_GRID_REMARK_LEN160, "Grid of %s components in the 0-255 range", comp[channel]);
			k3 = channel;
			k = 0;
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (header.depth == 8)	/* Gray ramp */
					Grid->data[ij] = (gmt_grdfloat)picture[k++];
				else {				/* 24-bit image */
					Grid->data[ij] = (gmt_grdfloat)picture[k3];
					k3 += 3;
				}
			}
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
				gmt_M_str_free (grdfile);
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, grdfile, Grid) != GMT_NOERROR) {
				gmt_M_str_free (grdfile);
				Return (API->error);
			}
			gmt_M_str_free (grdfile);
		}
		if (Ctrl->W.active)
			gmt_M_free (GMT, picture);
		else
			PSL_free (picture);
		if (GMT_Destroy_Data (API, &Grid) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	Return (GMT_NOERROR);
}
