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
 * Brief synopsis: psimage reads an EPS file or a a 1, 8, 24, or 32 bit image and plots it on the page
 * Images are only supported if GMT was built with GDAL support.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psimage"
#define THIS_MODULE_MODERN_NAME	"image"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot raster or EPS images"
#define THIS_MODULE_KEYS	"<I{,>X}"
#define THIS_MODULE_NEEDS	"jr"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYptxy" GMT_OPT("c")

struct PSIMAGE_CTRL {
	struct PSIMG_In {
		bool active;
		char *file;
	} In;
	struct PSIMG_D {	/* -D[g|j|n|x]<refpoint>+w[-]<width>[/<height>][+j<justify>][+n<n_columns>[/<n_rows>]][+o<dx>[/<dy>]][+r<dpi>] */
		bool active;
		struct GMT_REFPOINT *refpoint;
		double off[2];
		int justify;
		bool interpolate;
		double dim[2];
		double dpi;
		unsigned int n_columns, n_rows;
	} D;
	struct PSIMG_F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		struct GMT_MAP_PANEL *panel;
	} F;
	struct PSIMG_G {	/* -G<rgb>[+b|+f|+t] */
		bool active;
		double rgb[3][4];
	} G;
	struct PSIMG_I {	/* -I */
		bool active;
	} I;
	struct PSIMG_M {	/* -M */
		bool active;
	} M;
};

#define PSIMG_BGD	0
#define PSIMG_FGD	1
#define PSIMG_TRA	2

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSIMAGE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSIMAGE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->G.rgb[PSIMG_FGD][0] = C->G.rgb[PSIMG_BGD][0] = C->G.rgb[PSIMG_TRA][0] = -2;	/* All turned off */
	C->D.n_columns = C->D.n_rows = 1;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSIMAGE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_free_refpoint (GMT, &C->D.refpoint);
	gmt_M_free (GMT, C->F.panel);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psimage synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <imagefile> [%s] [-D%s+w[-]<width>[/<height>][+n<n_columns>[/<n_rows>]]%s+r<dpi>]\n", name, GMT_B_OPT, GMT_XYANCHOR, GMT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G[<color>][+b|+f|+t]] [-I] [%s] %s[-M] %s\n", GMT_J_OPT, API->K_OPT, API->O_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s[%s] [%s]\n", API->P_OPT, GMT_Rgeoz_OPT, GMT_U_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t%s[%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<imagefile> is an EPS or raster image file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "B");
	gmt_refpoint_syntax (API->GMT, "D", "Specify reference point for the image", GMT_ANCHOR_IMAGE, 3);
	GMT_Message (API, GMT_TIME_NONE, "\t   Set width (and height) of image with +w<width>/[/<height>].  If <height> = 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then the original aspect ratio is maintained.  If <width> < 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then we use absolute value as width and interpolate image in PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set image dpi (dots per inch) with +r.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +n to replicate image <n_columns> by <n_rows> times [Default is no replication].\n");
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the image", 1);
	GMT_Message (API, GMT_TIME_NONE, "\t-G Change some pixels to given <color> depending on selected modifier (repeatable).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +b for replacing background and +f for foreground color (1-bit images only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no color was given we make those pixels transparent [Default is black and white].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +t to indicate the given <color> should be made transparent [no transparency].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Invert 1-bit images (does not affect 8 or 24-bit images).\n");
	GMT_Option (API, "J-Z,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Force color -> monochrome image using YIQ-transformation.\n");
	GMT_Option (API, "O,P,R,U,V,X,c,p");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   (Requires -R and -J for proper functioning).\n");
	GMT_Option (API, "t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSIMAGE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psimage and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, ind = PSIMG_FGD, k = 0;
	int n;
	bool p_fail = false;
	char string[GMT_LEN256] = {""}, *p = NULL;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[4] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ == 0 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_IMAGE))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Image placement (old syntax) */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-C option is deprecated, use -Dx instead.\n");
				n = sscanf (opt->arg, "%[^/]/%[^/]/%2s", txt_a, txt_b, txt_c);
				sprintf (string, "x%s/%s", txt_a, txt_b);
				if (n == 3) {
					strcat (string, "+j");
					strcat (string, txt_c);
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				p = (string[0]) ? string : opt->arg;	/* If -C was used the string is set */
				if ((Ctrl->D.refpoint = gmt_get_refpoint (GMT, p, 'D')) == NULL) {	/* Failed basic parsing */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -D: Basic parsing of reference point in %s failed\n", opt->arg);
					p_fail = true;
					n_errors++;
				}
				else {	/* args are now [+j<justify>][+o<dx>[/<dy>]][+n<n_columns>[/<n_rows>]][+r<dpi>] */
					if (gmt_validate_modifiers (GMT, Ctrl->D.refpoint->args, 'D', "jnorw")) n_errors++;
					/* Required modifier +w OR +r */
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'w', string)) {
						if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) < 0) n_errors++;
						if (Ctrl->D.dim[GMT_X] < 0.0) {	/* Negative width means PS interpolation */
							Ctrl->D.dim[GMT_X] = -Ctrl->D.dim[GMT_X];
							Ctrl->D.interpolate = true;
						}
					}
					else if (gmt_get_modifier (Ctrl->D.refpoint->args, 'r', string))
						Ctrl->D.dpi = atof (string);
					/* Optional modifiers +j, +n, +o */
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'j', string))
						Ctrl->D.justify = gmt_just_decode (GMT, string, PSL_NO_DEF);
					else	/* With -Dj or -DJ, set default to reference (mirrored) justify point, else BL */
						Ctrl->D.justify = gmt_M_just_default (GMT, Ctrl->D.refpoint, PSL_BL);
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'n', string)) {
						n = sscanf (string, "%d/%d", &Ctrl->D.n_columns, &Ctrl->D.n_rows);
						if (n == 1) Ctrl->D.n_rows = Ctrl->D.n_columns;
					}
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
						if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
					}
				}
				break;
			case 'E':	/* Specify image dpi */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "The -E option is deprecated but is accepted.\n");
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "For the current -D syntax you should use -D modifier +r instead.\n");
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Note you cannot mix new-style modifiers (+r) with the old-style -C option.\n");
				Ctrl->D.dpi = atof (opt->arg);
				break;
			case 'F':	/* Specify frame pen */
				Ctrl->F.active = true;
				if (gmt_M_compat_check (GMT, 5) && opt->arg[0] != '+') /* Warn but process old -F<pen> */
					sprintf (string, "+c0+p%s", opt->arg);
				else
					strncpy (string, opt->arg, GMT_LEN256-1);
				if (gmt_getpanel (GMT, opt->option, string, &(Ctrl->F.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the image", 1);
					n_errors++;
				}
				break;
			case 'G':	/* Background/foreground color for 1-bit images */
				Ctrl->G.active = true;
				if ((p = strstr (opt->arg, "+b")))	/* Background color (or transparency) selected */
					ind = PSIMG_BGD, k = 0, p[0] = '\0';
				else if ((p = strstr (opt->arg, "+f")))	/* Foreground color (or transparency) selected */
					ind = PSIMG_FGD, k = 0, p[0] = '\0';
				else if ((p = strstr (opt->arg, "+t"))) {	/* Transparency color specified */
					ind = PSIMG_TRA;	k = 0; p[0] = '\0';
					if (opt->arg[0] == '\0') {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -G: Must specify a color when +t is used\n");
						n_errors++;
					}
				}
				else if (gmt_M_compat_check (GMT, 5)) {	/* Must check old-style arguments */
					if (opt->arg[0] == '-' || gmt_colorname2index (GMT, opt->arg) >= 0 || !gmt_getrgb (GMT, opt->arg, Ctrl->G.rgb[PSIMG_FGD]))	/* Foreground color selected as default */
						ind = PSIMG_FGD, k = 0;
					else if (opt->arg[0] == 'f')	/* Foreground color selected */
						ind = PSIMG_FGD, k = 1;
					else if (opt->arg[0] == 'b')	/* Background color (or transparency) selected */
						ind = PSIMG_BGD, k = 1;
					else if (opt->arg[0] == 't')	/* Transparency color specified */
						ind = PSIMG_TRA, k = 1;
				}
				else	/* Just -G[<color>] for foreground */
					ind = PSIMG_FGD, k = 0;
				if (opt->arg[k] == '\0')	/* No color given means set transparency */
					Ctrl->G.rgb[ind][0] = -1;
				else if (opt->arg[k] == '-') {	/* - means set transparency but only in GMT 5 and earlier */
					if (gmt_M_compat_check (GMT, 5)) {	/* - means set transparency in GMT 5 and earlier */
						Ctrl->G.rgb[ind][0] = -1;
						GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-G with color - for transparency is deprecated; give no <color> instead.\n");
					}
					else {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -G: - is not a color\n");
						n_errors++;
					}
				}
				else if (gmt_getrgb (GMT, &opt->arg[k], Ctrl->G.rgb[ind])) {
					gmt_rgb_syntax (GMT, 'G', " ");
					n_errors++;
				}
				if (p) p[0] = '+';	/* Restore modifier */
				break;
			case 'I':	/* Invert 1-bit images */
				Ctrl->I.active = true;
				break;
			case 'M':	/* Monochrome image */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Replicate image */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-N option is deprecated; use -D modifier +n instead.\n");
				n = sscanf (opt->arg, "%d/%d", &Ctrl->D.n_columns, &Ctrl->D.n_rows);
				if (n == 1) Ctrl->D.n_rows = Ctrl->D.n_columns;
				n_errors += gmt_M_check_condition (GMT, n < 1, "Syntax error -N option: Must give values for replication\n");
				break;
			case 'W':	/* Image width */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-W option is deprecated; use -D modifier +w instead.\n");
				if ((n = gmt_get_pair (GMT, opt->arg, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) < 0) n_errors++;
				if (Ctrl->D.dim[GMT_X] < 0.0) {
					Ctrl->D.dim[GMT_X] = -Ctrl->D.dim[GMT_X];
					Ctrl->D.interpolate = true;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->D.active) {	/* Old syntax without reference point implies -Dx0/0 */
		Ctrl->D.refpoint = gmt_get_refpoint (GMT, "x0/0", 'D');	/* Default if no -D given */
		Ctrl->D.active = true;
	}
	/* Check that the options selected are mutually consistent */

	if (Ctrl->D.refpoint && Ctrl->D.refpoint->mode != GMT_REFPOINT_PLOT) {	/* Anything other than -Dx need -R -J; other cases don't */
		static char *kind = "gjJnx";	/* The five types of refpoint specifications */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: -D%c requires the -R option\n", kind[Ctrl->D.refpoint->mode]);
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: -D%c requires the -J option\n", kind[Ctrl->D.refpoint->mode]);
	}
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single input raster or EPS file\n");
	n_errors += gmt_M_check_condition (GMT, !p_fail && Ctrl->D.dim[GMT_X] <= 0.0 && Ctrl->D.dpi <= 0.0, "Syntax error -D option: Must specify image width (+w) or dpi (+r)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.n_columns < 1 || Ctrl->D.n_rows < 1,
			"Syntax error -D option: Must specify positive values for replication with +n\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.rgb[PSIMG_FGD][0] == -1 && Ctrl->G.rgb[PSIMG_BGD][0] == -1,
			"Syntax error -G option: Only one of fore/back-ground can be transparent for 1-bit images\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int file_is_eps (struct GMT_CTRL *GMT, char **file) {	/* Returns 1 if it is an EPS file; 0 for any other file.
       Returns -1 on read error */
	FILE *fp = NULL;
	unsigned char c[4], magic_ps[4] = {'%', '!', 'P', 'S'};
	char *F = *file;

	if (F == NULL || F[0] == '\0') return GMT_NOTSET;	/* Nothing given */
	if (gmt_M_file_is_memory (F)) return (0);	/* Special passing of image */
	if (gmt_M_file_is_cache (F)) {	/* Must download, then modify the name */
		char *tmp = strdup (&F[1]);
		(void)gmt_download_file_if_not_found (GMT, F, 0);
		gmt_M_str_free (*file);
		*file = F = tmp;
	}
	if (strstr (F, "=gd")) return (0);		/* Passing an image via GDAL cannot be EPS */
	if (strstr (F, "+b"))  return (0);		/* Band request of an image cannot be EPS */

	if ((fp = gmt_fopen (GMT, F, "rb")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open file %s\n", F);
		return (GMT_NOTSET);
	}
	if (gmt_M_fread (c, 1U, 4U, fp) != 4U) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not read 4 bytes from file %s\n", F);
		gmt_fclose (GMT, fp);
		return (GMT_NOTSET);
	}
	gmt_fclose (GMT, fp);
	/* Note: cannot use gmt_M_same_rgb here, because that requires doubles */
	if (c[0] == magic_ps[0] && c[1] == magic_ps[1] && c[2] == magic_ps[2] && c[3] == magic_ps[3]) return(1);
	return (0);
}

#define Return(code) {gmt_M_free (GMT, table); return (code);}

#ifdef HAVE_GDAL
GMT_LOCAL int find_unique_color (struct GMT_CTRL *GMT, unsigned char *rgba, size_t n, int *r, int *g, int *b) {
	size_t i, j;
	int idx;
	bool trans = false;
	unsigned char *table = NULL;

	table = gmt_M_memory (GMT, NULL, 256*256*256, unsigned char);	/* Get table all initialized to zero */

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
#endif

#undef Return

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}
EXTERN_MSC unsigned char *psl_gray_encode (struct PSL_CTRL *PSL, size_t *nbytes, unsigned char *input);

int GMT_image (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: image\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psimage (V_API, mode, args);
}

int GMT_psimage (void *V_API, int mode, void *args) {
	int i, j, PS_interpolate = 1, PS_transparent = 1, is_eps = 0, error = 0, is_gdal = 0;
	unsigned int row, col;
	size_t n;
	bool free_GMT = false, did_gray = false;

	double x, y, wesn[4];

	unsigned char *picture = NULL, *buffer = NULL;

	char path[PATH_MAX] = {""}, *file = NULL, *c = NULL;

	struct imageinfo header;

	struct PSIMAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
#ifdef HAVE_GDAL
	int k, r = 0, g = 0, b = 0, has_trans = 0;
	unsigned char colormap[4*256];
	struct GMT_IMAGE *I = NULL;		/* A GMT image datatype, if GDAL is used */
#endif
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psimage main code ----------------------------*/

	PS_interpolate = (Ctrl->D.interpolate) ? -1 : +1;

	file = strdup (Ctrl->In.file);
	if ((c = strstr (file, "=gd"))) {
		is_gdal = true;
		c[0] = '\0';	/* Chop off unnecessary =gd mandate */
	}
	is_eps = file_is_eps (GMT, &file);	/* Determine if this is an EPS file or other */
	if (is_eps < 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open/read file %s\n", file);
		gmt_M_str_free (file);
		Return (GMT_RUNTIME_ERROR);
	}

	memset (&header, 0, sizeof(struct imageinfo)); /* initialize struct */
	if (is_eps) {	/* Read an EPS file */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input EPS file\n");
		if (gmt_getdatapath (GMT, file, path, R_OK) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open file %s.\n", file);
			gmt_M_str_free (file);
			Return (GMT_FILE_NOT_FOUND);
		}
		if (PSL_loadeps (PSL, path, &header, &picture)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Trouble loading EPS file %s!\n", path);
			gmt_M_str_free (file);
			Return (GMT_IMAGE_READ_ERROR);
		}
	}
#ifdef HAVE_GDAL
	else  {	/* Read a raster image */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input raster via GDAL\n");
		if (is_gdal) {	/* Need full name since there may be band requests */
			gmt_M_str_free (file);
			file = strdup (Ctrl->In.file);
		}
		gmt_set_pad (GMT, 0U);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
		if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, NULL)) == NULL) {
			gmt_M_str_free (file);
			Return (API->error);
		}
		gmt_set_pad (GMT, API->pad);	/* Reset to GMT default */

		/* Handle transparent images */
		if (I->colormap != NULL) {	/* Image has a color map */
			/* Convert colormap from integer to unsigned char and count colors */
			for (n = 0; n < (size_t)(4 * I->n_indexed_colors) && I->colormap[n] >= 0; n++) colormap[n] = (unsigned char)I->colormap[n];
			n /= 4;
			if (n == 2 && Ctrl->G.active) {	/* Replace back or fore-ground color with color given in -G, or catch selection for transparency */
				if (Ctrl->G.rgb[PSIMG_TRA][0] != -2) {
					GMT_Report (API, GMT_MSG_VERBOSE, "Your -G<color>+t is ignored for 1-bit images; see +b/+f modifiers instead\n");
				}
				for (unsigned int k = PSIMG_BGD; k <= PSIMG_FGD; k++) {
					if (Ctrl->G.rgb[k][0] == -1) {	/* Want this color to be transparent */
						has_trans = 1; r = colormap[4*k]; g = colormap[1+4*k]; b = colormap[2+4*k];
					}
					else if (Ctrl->G.rgb[k][0] >= 0)	/* If we changed this color, update it, else use what was given in the colormap */
						for (n = 0; n < 3; n++) colormap[n+4*k] = gmt_M_u255(Ctrl->G.rgb[k][n]);	/* Do not override the A entry, just R/G/B */
				}
			}
			if (!Ctrl->G.active) has_trans = find_unique_color (GMT, colormap, n, &r, &g, &b);

			/* Expand 8-bit indexed image to 24-bit image */
			I->data = gmt_M_memory (GMT, I->data, 3 * I->header->nm, unsigned char);
			n = 3 * I->header->nm - 1;
			for (j = (int)I->header->nm - 1; j >= 0; j--) {
				k = 4 * I->data[j] + 3;
				if (has_trans && colormap[k] == 0)	/* Found a transparent pixel, set its color to the transparent color found/selected earlier */
					I->data[n--] = (unsigned char)b, I->data[n--] = (unsigned char)g, I->data[n--] = (unsigned char)r;
				else
					I->data[n--] = colormap[--k], I->data[n--] = colormap[--k], I->data[n--] = colormap[--k];
			}
			I->header->n_bands = 3;
		}
		else if (I->header->n_bands == 4) { /* RGBA image, with a color map */
			uint64_t n4, j4;
			if (!Ctrl->G.active) has_trans = find_unique_color (GMT, I->data, I->header->nm, &r, &g, &b);
			for (j4 = n4 = 0; j4 < 4 * I->header->nm; j4++) { /* Reduce image from 32- to 24-bit */
				if (has_trans && I->data[j4+3] == 0)
					I->data[n4++] = (unsigned char)r, I->data[n4++] = (unsigned char)g, I->data[n4++] = (unsigned char)b, j4 += 3;
				else
					I->data[n4++] = I->data[j4++], I->data[n4++] = I->data[j4++], I->data[n4++] = I->data[j4++];
			}
			I->header->n_bands = 3;
		}

		/* If a transparent color was found, we replace it with a unique one */
		if (has_trans) {
			Ctrl->G.rgb[PSIMG_TRA][0] = r / 255.;
			Ctrl->G.rgb[PSIMG_TRA][1] = g / 255.;
			Ctrl->G.rgb[PSIMG_TRA][2] = b / 255.;
		}

		picture = (unsigned char *)I->data;
		header.width = I->header->n_columns;
		header.height = I->header->n_rows;
		header.depth = (int)I->header->n_bands * 8;
	}
#else
	else {	/* Without GDAL we can only read EPS files */
		GMT_Report (API, GMT_MSG_NORMAL, "Unsupported file format for file %s!\n", file);
		gmt_M_str_free (file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif

	if (Ctrl->M.active && header.depth == 24) {	/* Downgrade to grayshade image */
		did_gray = true;
		n = 3 * header.width * header.height;
		buffer = psl_gray_encode (PSL, &n, picture);
		header.depth = 8;
		if (is_eps) PSL_free (picture); /* EPS ile */
#ifdef HAVE_GDAL
		else {	/* Got it via GMT_Read_Data */
			if (GMT_Destroy_Data (API, &I) != GMT_NOERROR) {
				gmt_M_str_free (file);
				Return (API->error);
			}
		}
#endif
		picture = buffer;
	}

	/* Add transparent color at beginning, if requested */
	if (Ctrl->G.rgb[PSIMG_TRA][0] < 0)
		PS_transparent = 1;
	else if (header.depth >= 8) {
		PS_transparent = -1;
		j = header.depth / 8;
		n = j * (header.width * header.height + 1);
		buffer = gmt_M_memory (GMT, NULL, n, unsigned char);
		for (i = 0; i < j; i++) buffer[i] = (unsigned char)rint(255 * Ctrl->G.rgb[PSIMG_TRA][i]);
		gmt_M_memcpy (&(buffer[j]), picture, n - j, unsigned char);
#ifdef HAVE_GDAL
		if (GMT_Destroy_Data (API, &I) != GMT_NOERROR) {	/* If I is NULL then nothing is done */
			gmt_M_str_free (file);
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

	gmt_M_memset (wesn, 4, double);
	if (!(GMT->common.R.active[RSET] && GMT->common.J.active)) {	/* When no projection specified, use fake linear projection */
		GMT->common.R.active[RSET] = true;
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', "X1i");
		gmt_adjust_refpoint (GMT, Ctrl->D.refpoint, Ctrl->D.dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust refpoint to BL corner */
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->D.n_columns * Ctrl->D.dim[GMT_X];
		wesn[YHI] = Ctrl->D.refpoint->y + Ctrl->D.n_rows * Ctrl->D.dim[GMT_Y];
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), "")) {
			if (free_GMT)
				gmt_M_free (GMT, picture);
			else if (is_eps || did_gray)
				PSL_free (picture);
			gmt_M_str_free (file);
			Return (GMT_PROJECTION_ERROR);
		}
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) {
			if (free_GMT)
				gmt_M_free (GMT, picture);
			else if (is_eps || did_gray)
				PSL_free (picture);
			gmt_M_str_free (file);
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) {
			if (free_GMT)
				gmt_M_free (GMT, picture);
			else if (is_eps || did_gray)
				PSL_free (picture);
			gmt_M_str_free (file);
			Return (GMT_PROJECTION_ERROR);
		}
		gmt_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */
		gmt_adjust_refpoint (GMT, Ctrl->D.refpoint, Ctrl->D.dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust refpoint to BL corner */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) {
			if (free_GMT)
				gmt_M_free (GMT, picture);
			else if (is_eps || did_gray)
				PSL_free (picture);
			gmt_M_str_free (file);
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		gmt_map_basemap (GMT);	/* Draw basemap if requested */
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', "X1i");
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->D.n_columns * Ctrl->D.dim[GMT_X];
		wesn[YHI] = Ctrl->D.refpoint->y + Ctrl->D.n_rows * Ctrl->D.dim[GMT_Y];
		GMT->common.R.active[RSET] = GMT->common.J.active = true;
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), "")) {
			if (free_GMT)
				gmt_M_free (GMT, picture);
			else if (is_eps || did_gray)
				PSL_free (picture);
			gmt_M_str_free (file);
			Return (GMT_PROJECTION_ERROR);
		}
	}

 	if (Ctrl->F.active) {	/* Draw frame, fill only */
		Ctrl->F.panel->width = Ctrl->D.n_columns * Ctrl->D.dim[GMT_X];	Ctrl->F.panel->height = Ctrl->D.n_rows * Ctrl->D.dim[GMT_Y];
		gmt_draw_map_panel (GMT, Ctrl->D.refpoint->x + 0.5 * Ctrl->F.panel->width, Ctrl->D.refpoint->y + 0.5 * Ctrl->F.panel->height, 1U, Ctrl->F.panel);
 	}

	for (row = 0; row < Ctrl->D.n_rows; row++) {
		y = Ctrl->D.refpoint->y + row * Ctrl->D.dim[GMT_Y];
		if (Ctrl->D.n_rows > 1) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Replicating image %d times for row %d\n", Ctrl->D.n_columns, row);
		for (col = 0; col < Ctrl->D.n_columns; col++) {
			x = Ctrl->D.refpoint->x + col * Ctrl->D.dim[GMT_X];
			if (header.depth == 0)
				PSL_plotepsimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture, &header);
			else if (header.depth == 1) {
				/* Invert is opposite from what is expected. This is to match the behaviour of -Gp */
				if (Ctrl->I.active)
					PSL_plotbitimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture,
							header.width, header.height, Ctrl->G.rgb[PSIMG_FGD], Ctrl->G.rgb[PSIMG_BGD]);
				else
					PSL_plotbitimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture,
							header.width, header.height, Ctrl->G.rgb[PSIMG_BGD], Ctrl->G.rgb[PSIMG_FGD]);
			}
			else
				 PSL_plotcolorimage (PSL, x, y, Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y], PSL_BL, picture,
						 PS_transparent * header.width, header.height, PS_interpolate * header.depth);
		}
	}
 	if (Ctrl->F.active)	/* Draw frame outlines */
		gmt_draw_map_panel (GMT, Ctrl->D.refpoint->x + 0.5 * Ctrl->F.panel->width, Ctrl->D.refpoint->y + 0.5 * Ctrl->F.panel->height, 2U, Ctrl->F.panel);

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);
	gmt_M_str_free (file);

#ifdef HAVE_GDAL
	if (I && GMT_Destroy_Data (API, &I) != GMT_NOERROR) {
		Return (API->error);	/* If I is NULL then nothing is done */
	}
#endif
	if (free_GMT) {
		gmt_M_free (GMT, picture);
	}
	else if (is_eps || did_gray)
		PSL_free (picture);

	Return (GMT_NOERROR);
}
