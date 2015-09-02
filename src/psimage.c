/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: psimage reads a 1, 8, 24, or 32 bit Sun rasterfile and plots it on the page
 * Other raster formats are supported if GraphicsMagick's/ImageMagick's convert is found in the
 * system path.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"psimage"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Place images or EPS files on maps"
#define THIS_MODULE_KEYS	"<II,>XO,RG!"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcptxy"

struct PSIMAGE_CTRL {
	struct PSIMG_In {
		bool active;
		char *file;
	} In;
	struct PSIMG_D {	/* -D[g|j|n|x]<refpoint>+w[-]<width>[/<height>][+j<justify>][+n<nx>[/<ny>]][+o<off[GMT_X]>[/<dy>]][+r<dpi>] */
		bool active;
		struct GMT_REFPOINT *refpoint;
		double off[2];
		int justify;
		bool interpolate;
		double dim[2];
		double dpi;
		unsigned int nx, ny;
	} D;
	struct PSIMG_F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		struct GMT_MAP_PANEL *panel;
	} F;
	struct PSIMG_G {	/* -G[f|b|t]<rgb> */
		bool active;
		double f_rgb[4];
		double b_rgb[4];
		double t_rgb[4];
	} G;
	struct PSIMG_I {	/* -I */
		bool active;
	} I;
	struct PSIMG_M {	/* -M */
		bool active;
	} M;
};

void *New_psimage_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSIMAGE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSIMAGE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->G.f_rgb[0] = C->G.b_rgb[0] = C->G.t_rgb[0] = -2;
	C->D.nx = C->D.ny = 1;
	return (C);
}

void Free_psimage_Ctrl (struct GMT_CTRL *GMT, struct PSIMAGE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	GMT_free_refpoint (GMT, &C->D.refpoint);
	if (C->F.panel) GMT_free (GMT, C->F.panel);
	GMT_free (GMT, C);
}

int GMT_psimage_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the psimage synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psimage <imagefile> [%s] [-D%s+w[-]<width>[/<height>][+n<nx>[/<ny>]]%s+r<dpi>]\n", GMT_B_OPT, GMT_XYANCHOR, GMT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G[b|f|t]<color>] [-I] [%s] [%s] [-K] [-M] [-O]\n", GMT_J_OPT, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-P] [%s] [%s]\n", GMT_Rgeoz_OPT, GMT_U_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<imagefile> is an EPS or image file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B");
	GMT_refpoint_syntax (API->GMT, 'D', "Specify reference point for the image", GMT_ANCHOR_IMAGE, 3);
	GMT_Message (API, GMT_TIME_NONE, "\t   Set width (and height) of image with +w<width>/[/<height>].  If <height> = 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then the original aspect ratio is maintained.  If <width> < 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then we use absolute value as width and interpolate image in PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set image dpi (dots per inch) with +r.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +n to replicate image <nx> by <ny> times [Default is no replication].\n");
	GMT_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the image", 1);
	GMT_Message (API, GMT_TIME_NONE, "\t-Gb and -Gf (1-bit images only) sets the background and foreground color,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   respectively. Set <color> = - for transparency [Default is black and white].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Gt (not for 1-bit images) indicate which color to be made transparent\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default no transparency].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Invert 1-bit images (does not affect 8 or 24-bit images).\n");
	GMT_Option (API, "J-Z,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Force color -> monochrome image using GMT_YIQ-transformation.\n");
	GMT_Option (API, "O,P,R,U,V,X,c,p");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Requires -R and -J for proper functioning).\n");
	GMT_Option (API, "t,.");

	return (EXIT_FAILURE);
}

int GMT_psimage_parse (struct GMT_CTRL *GMT, struct PSIMAGE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psimage and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int n;
	char string[GMT_LEN256] = {""}, letter, *p = NULL;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[4] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ == 0 && GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_IMAGE))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Image placement (old syntax) */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: -C option is deprecated, use -Dx instead.\n");
				n = sscanf (opt->arg, "%[^/]/%[^/]/%2s", txt_a, txt_b, txt_c);
				sprintf (string, "x%s/%s", txt_a, txt_b);
				if (n == 3) {
					strcat (string, "+j");
					strcat (string, txt_c);
				}
			case 'D':
				Ctrl->D.active = true;
				p = (string[0]) ? string : opt->arg;	/* If -C was used the string is set */
				if ((Ctrl->D.refpoint = GMT_get_refpoint (GMT, p)) == NULL) n_errors++;	/* Failed basic parsing */
				else {	/* args are now [+j<justify>][+o<off[GMT_X]>[/<dy>]][+n<nx>[/<ny>]][+r<dpi>] */
					if (GMT_validate_modifiers (GMT, Ctrl->D.refpoint->args, 'D', "jnorw")) n_errors++;
					/* Required modifier +w OR +r */
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'w', string)) {
						if ((n = GMT_get_pair (GMT, string, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) < 0) n_errors++;
						if (Ctrl->D.dim[GMT_X] < 0.0) {	/* Negative width means PS interpolation */
							Ctrl->D.dim[GMT_X] = -Ctrl->D.dim[GMT_X];
							Ctrl->D.interpolate = true;
						}
					}
					else if (GMT_get_modifier (Ctrl->D.refpoint->args, 'r', string))
						Ctrl->D.dpi = atof (string);
					/* Optional modifiers +j, +n, +o */
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'j', string))
						Ctrl->D.justify = GMT_just_decode (GMT, string, PSL_NO_DEF);
					else	/* With -Dj or -DJ, set default to reference (mirrored) justify point, else BL */
						Ctrl->D.justify = GMT_just_default (GMT, Ctrl->D.refpoint, PSL_BL);
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'n', string)) {
						n = sscanf (string, "%d/%d", &Ctrl->D.nx, &Ctrl->D.ny);
						if (n == 1) Ctrl->D.ny = Ctrl->D.nx;
					}
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
						if ((n = GMT_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
					}
				}
				break;
			case 'E':	/* Specify image dpi */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: The -E option is deprecated but is accepted.\n");
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "For the current -D syntax you should use -D modifier +r instead.\n");
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Note you cannot mix new-style modifiers (+r) with the old-style -C option.\n");
				Ctrl->D.dpi = atof (opt->arg);
				break;
			case 'F':	/* Specify frame pen */
				Ctrl->F.active = true;
				if (GMT_compat_check (GMT, 5) && opt->arg[0] != '+') /* Warn but process old -F<pen> */
					sprintf (string, "+c0+p%s", opt->arg);
				else
					strcpy (string, opt->arg);
				if (GMT_getpanel (GMT, opt->option, string, &(Ctrl->F.panel))) {
					GMT_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the image", 1);
					n_errors++;
				}
				break;
			case 'G':	/* Background/foreground color for 1-bit images */
				Ctrl->G.active = true;
				letter = (GMT_colorname2index (GMT, opt->arg) >= 0) ? 'x' : opt->arg[0];	/* If we have -G<colorname>, the x is used to bypass the case f|b|t switching below */
				switch (letter) {
					case 'f':
						/* Set color for foreground pixels */
						if (opt->arg[1] == '-' && opt->arg[2] == '\0')
							Ctrl->G.f_rgb[0] = -1;
						else if (GMT_getrgb (GMT, &opt->arg[1], Ctrl->G.f_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						break;
					case 'b':
						/* Set color for background pixels */
						if (opt->arg[1] == '-' && opt->arg[2] == '\0')
							Ctrl->G.b_rgb[0] = -1;
						else if (GMT_getrgb (GMT, &opt->arg[1], Ctrl->G.b_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						break;
					case 't':
						/* Set transparent color */
						if (GMT_getrgb (GMT, &opt->arg[1], Ctrl->G.t_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						break;
					default:	/* Gave either -G<r/g/b>, -G-, or -G<colorname>; all treated as -Gf */
						if (opt->arg[0] == '-' && opt->arg[1] == '\0')
							Ctrl->G.f_rgb[0] = -1;
						else if (GMT_getrgb (GMT, opt->arg, Ctrl->G.f_rgb)) {
							GMT_rgb_syntax (GMT, 'G', " ");
							n_errors++;
						}
						break;
				}
				break;
			case 'I':	/* Invert 1-bit images */
				Ctrl->I.active = true;
				break;
			case 'M':	/* Monochrome image */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Replicate image */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: -N option is deprecated; use -D modifier +n instead.\n");
				n = sscanf (opt->arg, "%d/%d", &Ctrl->D.nx, &Ctrl->D.ny);
				if (n == 1) Ctrl->D.ny = Ctrl->D.nx;
				n_errors += GMT_check_condition (GMT, n < 1, "Syntax error -N option: Must give values for replication\n");
				break;
			case 'W':	/* Image width */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: -W option is deprecated; use -D modifier +w instead.\n");
				if ((n = GMT_get_pair (GMT, opt->arg, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) < 0) n_errors++;
				if (Ctrl->D.dim[GMT_X] < 0.0) {
					Ctrl->D.dim[GMT_X] = -Ctrl->D.dim[GMT_X];
					Ctrl->D.interpolate = true;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* If not done previously, set foreground to black, background to white */

	if (Ctrl->G.f_rgb[0] == -2) { Ctrl->G.f_rgb[0] = Ctrl->G.f_rgb[1] = Ctrl->G.f_rgb[2] = 0.0; }
	if (Ctrl->G.b_rgb[0] == -2) { Ctrl->G.b_rgb[0] = Ctrl->G.b_rgb[1] = Ctrl->G.b_rgb[2] = 1.0; }

	if (!Ctrl->D.active) {	/* Old syntax without reference point implies -Dx0/0 */
		Ctrl->D.refpoint = GMT_get_refpoint (GMT, "x0/0");	/* Default if no -D given */
		Ctrl->D.active = true;
	}
	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single input raster or EPS file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.dim[GMT_X] <= 0.0 && Ctrl->D.dpi <= 0.0, "Syntax error -D option: Must specify image width (+w) or dpi (+e)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.nx < 1 || Ctrl->D.ny < 1,
			"Syntax error -D option: Must specify positive values for replication with +n\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.f_rgb[0] < 0 && Ctrl->G.b_rgb[0] < 0,
			"Syntax error -G option: Only one of fore/back-ground can be transparent for 1-bit images\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int file_is_known (struct GMT_CTRL *GMT, char *file)
{	/* Returns 1 if it is an EPS file, 2 if a Sun rasterfile; 0 for any other file.
       Returns -1 on read error */
	FILE *fp = NULL;
	unsigned char c[4], magic_ras[4] = {0x59, 0xa6, 0x6a, 0x95}, magic_ps[4] = {'%', '!', 'P', 'S'};
	int j;

	if (GMT_File_Is_Memory (file)) return (0);	/* Special passing of image */
	j = (int)strlen(file) - 1;
	while (j && file[j] && file[j] != '+') j--;	/* See if we have a band request */
	if (j && file[j+1] == 'b') file[j] = '\0';			/* Temporarily strip the band request string so that the opening test doesn't fail */

	if ((fp = GMT_fopen (GMT, file, "rb")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open file %s\n", file);
		return (-1);
	}
	if (GMT_fread (c, 1U, 4U, fp) != 4U) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not read 4 bytes from file %s\n", file);
		return (-1);
	}
	GMT_fclose (GMT, fp);
	if (j) file[j] = '+';			/* Reset the band request string */
	/* Note: cannot use GMT_same_rgb here, because that requires doubles */
	if (c[0] == magic_ps[0] && c[1] == magic_ps[1] && c[2] == magic_ps[2] && c[3] == magic_ps[3]) return(1);
	if (c[0] == magic_ras[0] && c[1] == magic_ras[1] && c[2] == magic_ras[2] && c[3] == magic_ras[3]) return(2);
	return (0);	/* Neither */
}

#define Return(code) {GMT_free (GMT, table); return (code);}

int find_unique_color (struct GMT_CTRL *GMT, unsigned char *rgba, size_t n, int *r, int *g, int *b)
{
	size_t i, j;
	int idx;
	bool trans = false;
	unsigned char *table = NULL;

	table = GMT_memory (GMT, NULL, 256*256*256, unsigned char);	/* Get table all initialized to zero */

	/* Check off all the non-transparent colors, store the transparent one */
	*r = *g = *b = 0;
	for (i = j = 0; i < n; i++, j+=4) {
		if (rgba[j+3] == 0)
			*r = rgba[j], *g = rgba[j+1], *b = rgba[j+2], trans = true;
		else {
			idx = 256 * (256*(int)rgba[j] + (int)rgba[j+1]) + (int)rgba[j+2];
			table[idx] = true;
		}
	}

	/* Was there a transparent color? */
	if (!trans) Return (0);

	/* Was the transparent color unique? */
	idx = 256 * (256*(*r) + (*g)) + (*b);
	if (!table[idx]) Return (1);

	/* Find a unique color */
	idx = 0;
	for (*r = 0; *r < 256; (*r)++) {
		for (*g = 0; *g < 256; (*g)++) {
			for (*b = 0; *b < 256; (*b)++, idx++) {
				if (!table[idx]) Return (2);
			}
		}
	}
	Return (3);
}
#undef Return

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psimage_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psimage (void *V_API, int mode, void *args)
{
	int i, j, n, PS_interpolate = 1, PS_transparent = 1, known = 0, error = 0;
	unsigned int row, col;
	bool free_GMT = false, did_gray = false;

	double x, y, wesn[4];

	unsigned char *picture = NULL, *buffer = NULL;

	char path[GMT_BUFSIZ] = {""}, *format[2] = {"EPS", "Sun raster"};

	struct imageinfo header;

	struct PSIMAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
#ifdef HAVE_GDAL
	int k, r,g,b, has_trans = 0;
	unsigned char colormap[4*256];
	struct GMT_IMAGE *I = NULL;		/* A GMT image datatype, if GDAL is used */
#endif
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psimage_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psimage_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psimage_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_psimage_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psimage_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the psimage main code ----------------------------*/

	PS_interpolate = (Ctrl->D.interpolate) ? -1 : +1;

	known = file_is_known (GMT, Ctrl->In.file);	/* Determine if this is an EPS file, Sun rasterfile, or other */
	if (known < 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open/read file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}

	memset (&header, 0, sizeof(struct imageinfo)); /* initialize struct */
	if (known) {	/* Read an EPS or Sun raster file */
		GMT_Report (API, GMT_MSG_VERBOSE, "Processing input EPS or Sun rasterfile\n");
		if (GMT_getdatapath (GMT, Ctrl->In.file, path, R_OK) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open file %s.\n", Ctrl->In.file);
			Return (EXIT_FAILURE);
		}
		if (PSL_loadimage (PSL, path, &header, &picture)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Trouble loading %s file %s!\n", format[known-1], path);
			Return (EXIT_FAILURE);
		}
	}
#ifdef HAVE_GDAL
	else  {	/* Read a raster image */
		GMT_Report (API, GMT_MSG_VERBOSE, "Processing input raster via GDAL\n");
		GMT_set_pad (GMT, 0U);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
		if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}
		GMT_set_pad (GMT, API->pad);	/* Reset to GMT default */

		/* Handle transparent images */
		if (I->ColorMap != NULL) {	/* Image has a color map */
			/* Convert colormap from integer to unsigned char and count colors */
			for (n = 0; n < 4 * 256 && I->ColorMap[n] >= 0; n++) colormap[n] = (unsigned char)I->ColorMap[n];
			n /= 4;
			if (!Ctrl->G.active) has_trans = find_unique_color (GMT, colormap, n, &r, &g, &b);

			/* Expand 8-bit indexed image to 24-bit image */
			I->data = GMT_memory (GMT, I->data, 3 * I->header->nm, unsigned char);
			n = (int)(3 * I->header->nm - 1);
			for (j = (int)I->header->nm - 1; j >= 0; j--) {
				k = 4 * I->data[j] + 3;
				if (has_trans && colormap[k] == 0)
					I->data[n--] = b, I->data[n--] = g, I->data[n--] = r;
				else
					I->data[n--] = colormap[--k], I->data[n--] = colormap[--k], I->data[n--] = colormap[--k];
			}
			I->header->n_bands = 3;
		}
		else if (I->header->n_bands == 4) { /* RGBA image, with a color map */
			uint64_t n4, j4;
			if (!Ctrl->G.active) has_trans = find_unique_color (GMT, I->data, (int)I->header->nm, &r, &g, &b);
			for (j4 = n4 = 0; j4 < 4 * I->header->nm; j4++) { /* Reduce image from 32- to 24-bit */
				if (has_trans && I->data[j4+3] == 0)
					I->data[n4++] = r, I->data[n4++] = g, I->data[n4++] = b, j4 += 3;
				else
					I->data[n4++] = I->data[j4++], I->data[n4++] = I->data[j4++], I->data[n4++] = I->data[j4++];
			}
			I->header->n_bands = 3;
		}

		/* If a transparent color was found, we replace it with a unique one */
		if (has_trans) {
			Ctrl->G.t_rgb[0] = r / 255.;
			Ctrl->G.t_rgb[1] = g / 255.;
			Ctrl->G.t_rgb[2] = b / 255.;
		}

		picture = (unsigned char *)I->data;
		header.width = I->header->nx;
		header.height = I->header->ny;
		header.depth = (int)I->header->n_bands * 8;
	}
#else
	else {	/* Without GDAL we can only read EPS and Sun raster */
		GMT_Report (API, GMT_MSG_NORMAL, "Unsupported file format for file %s!\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}
#endif

	if (Ctrl->M.active && header.depth == 24) {	/* Downgrade to grayshade image */
		did_gray = true;
		n = 3 * header.width * header.height;
		buffer = psl_gray_encode (PSL, &n, picture);
		header.depth = 8;
		if (known) PSL_free (picture); /* EPS or Sun raster file */
#ifdef HAVE_GDAL
		else {	/* Got it via GMT_Read_Data */
			if (GMT_Destroy_Data (API, &I) != GMT_OK) {
				Return (API->error);
			}
		}
#endif
		picture = buffer;
	}

	/* Add transparent color at beginning, if requested */
	if (Ctrl->G.t_rgb[0] < 0)
		PS_transparent = 1;
	else if (header.depth >= 8) {
		PS_transparent = -1;
		j = header.depth / 8;
		n = j * (header.width * header.height + 1);
		buffer = GMT_memory (GMT, NULL, n, unsigned char);
		for (i = 0; i < j; i++) buffer[i] = (unsigned char)rint(255 * Ctrl->G.t_rgb[i]);
		GMT_memcpy (&(buffer[j]), picture, n - j, unsigned char);
#ifdef HAVE_GDAL
		if (GMT_Destroy_Data (API, &I) != GMT_OK) {	/* If I is NULL then nothing is done */
			Return (API->error);
		}
#else
		PSL_free (picture);
#endif
		picture = buffer;
		free_GMT = true;
	}
	else
		GMT_Report (API, GMT_MSG_NORMAL, "Can only do transparent color for 8- or 24-bit images. -Gt ignored\n");

	if (Ctrl->D.dpi > 0.0) Ctrl->D.dim[GMT_X] = (double) header.width / Ctrl->D.dpi;
	if (Ctrl->D.dim[GMT_Y] == 0.0) Ctrl->D.dim[GMT_Y] = header.height * Ctrl->D.dim[GMT_X] / header.width;

	/* The following is needed to have psimage work correctly in perspective */

	GMT_memset (wesn, 4, double);
	if (!(GMT->common.R.active && GMT->common.J.active)) {	/* When no projection specified, use fake linear projection */
		GMT->common.R.active = true;
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', "X1i");
		GMT_adjust_refpoint (GMT, Ctrl->D.refpoint, Ctrl->D.dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust refpoint to BL corner */
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->D.nx * Ctrl->D.dim[GMT_X];
		wesn[YHI] = Ctrl->D.refpoint->y + Ctrl->D.ny * Ctrl->D.dim[GMT_Y];
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
		GMT_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */
		GMT_adjust_refpoint (GMT, Ctrl->D.refpoint, Ctrl->D.dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust refpoint to BL corner */
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT_plotcanvas (GMT);	/* Fill canvas if requested */
		GMT_map_basemap (GMT);	/* Draw basemap if requested */
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', "X1i");
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->D.nx * Ctrl->D.dim[GMT_X];
		wesn[YHI] = Ctrl->D.refpoint->y + Ctrl->D.ny * Ctrl->D.dim[GMT_Y];
		GMT->common.R.active = GMT->common.J.active = true;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
	}

 	if (Ctrl->F.active) {	/* Draw frame, fill only */
		Ctrl->F.panel->width = Ctrl->D.nx * Ctrl->D.dim[GMT_X];	Ctrl->F.panel->height = Ctrl->D.ny * Ctrl->D.dim[GMT_Y];
		GMT_draw_map_panel (GMT, 0.5 * Ctrl->F.panel->width, 0.5 * Ctrl->F.panel->height, 1U, Ctrl->F.panel);
 	}

	for (row = 0; row < Ctrl->D.ny; row++) {
		y = Ctrl->D.refpoint->y + row * Ctrl->D.dim[GMT_Y];
		if (Ctrl->D.ny > 1) GMT_Report (API, GMT_MSG_VERBOSE, "Replicating image %d times for row %d\n", Ctrl->D.nx, row);
		for (col = 0; col < Ctrl->D.nx; col++) {
			x = Ctrl->D.refpoint->x + col * Ctrl->D.dim[GMT_X];
			if (header.depth == 0)
				PSL_plotepsimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture, header.length,
						header.width, header.height, header.xorigin, header.yorigin);
			else if (header.depth == 1) {
				/* Invert is opposite from what is expected. This is to match the behaviour of -Gp */
				if (Ctrl->I.active)
					PSL_plotbitimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture,
							header.width, header.height, Ctrl->G.f_rgb, Ctrl->G.b_rgb);
				else
					PSL_plotbitimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture,
							header.width, header.height, Ctrl->G.b_rgb, Ctrl->G.f_rgb);
			}
			else
				 PSL_plotcolorimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture,
						 PS_transparent * header.width, header.height, PS_interpolate * header.depth);
		}
	}
 	if (Ctrl->F.active)	/* Draw frame outlines */
		GMT_draw_map_panel (GMT, 0.5 * Ctrl->F.panel->width, 0.5 * Ctrl->F.panel->height, 2U, Ctrl->F.panel);

	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

#ifdef HAVE_GDAL
	if (I && GMT_Destroy_Data (API, &I) != GMT_OK) {
		Return (API->error);	/* If I is NULL then nothing is done */
	}
#endif
	if (free_GMT) {
		GMT_free (GMT, picture);
	}
	else if (known || did_gray)
		PSL_free (picture);

	Return (GMT_OK);
}
