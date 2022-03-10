/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: grdimage will read a grid or image and plot the area
 * with optional shading using the selected projection.  Optionally, it
 * may return the raw raster image.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdimage"
#define THIS_MODULE_MODERN_NAME	"grdimage"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Project and plot grids or images"
#define THIS_MODULE_KEYS	"<G{+,CC(,IG(,>X},>IA,<ID"
#define THIS_MODULE_NEEDS	"Jg"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYfnptxy" GMT_OPT("Sc") GMT_ADD_x_OPT

/* These are images that GDAL knows how to read for us. */
#define N_IMG_EXTENSIONS 6
static char *gdal_ext[N_IMG_EXTENSIONS] = {"tiff", "tif", "gif", "png", "jpg", "bmp"};

#define GRDIMAGE_NAN_INDEX	(GMT_NAN - 3)

/* Control structure for grdimage */

struct GRDIMAGE_CTRL {
	struct GRDIMAGE_In {	/* Input grid or image */
		bool active;
		char *file;
	} In;
	struct GRDIMAGE_Out {	/* Output image under -A */
		bool active;
		char *file;
	} Out;
	struct GRDIMAGE_C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...][+i<dz>] */
		bool active;
		double dz;	/* Rounding for min/max determined from data */
		char *file;
	} C;
	struct GRDIMAGE_D {	/* -D to read image instead of grid */
		bool active;
		bool mode;	/* Use info of -R option to reference image */
	} D;
	struct GRDIMAGE_A {	/* -A to write a raster file or return image to API */
		bool active;
		bool return_image;
		unsigned int way;
		char *file;
	} A;
	struct GRDIMAGE_E {	/* -Ei|<dpi> */
		bool active;
		bool device_dpi;
		unsigned int dpi;
	} E;
	struct GRDIMAGE_G {	/* -G<rgb>[+b|f] */
		bool active;
		double rgb[2][4];
	} G;
	struct GRDIMAGE_I {	/* -I[<intens_or_zfile>|<value>|<modifiers>] */
		bool active;
		bool constant;
		bool derive;
		double value;
		char *azimuth;	/* Default azimuth(s) for shading */
		char *file;
		char *method;	/* Default scaling method */
		char *ambient;	/* Default ambient offset */
	} I;
	struct GRDIMAGE_M {	/* -M */
		bool active;
	} M;
	struct GRDIMAGE_N {	/* -N */
		bool active;
	} N;
	struct GRDIMAGE_Q {	/* -Q[r/g/b][+z<value>] */
		bool active;
		bool transp_color;	/* true if a color was given */
		bool z_given;	/* true if a z-value was given */
		double rgb[4];	/* Pixel value for transparency in images */
		double value;	/* If +z is used this z-value will give us the r/g/b via CPT */
	} Q;
	struct GRDIMAGE_W {	/* -W */
		bool active;
	} W;
};

struct GRDIMAGE_CONF {
	/* Configuration structure for things to pass around to sub-functions */
	unsigned int colormask_offset;	/* Either 0 or 3 depending on -Q */
	bool int_mode;				/* true if we are to apply illumination */
	unsigned int *actual_row;	/* Array with actual rows as a function of pseudo row */
	unsigned int *actual_col;	/* Array of actual columns as a function of pseudo col */
	int64_t n_columns, n_rows;		/* Signed dimensions for use in OpenMP loops (if implemented) */
	uint64_t nm;				/* Number of pixels in the image */
	struct GMT_PALETTE *P;		/* Pointer to the active palette [NULL if image] */
	struct GMT_GRID *Grid;		/* Pointer to the active grid [NULL if image] */
	struct GMT_GRID *Intens;	/* Pointer to the active intensity grid [NULL if no intensity] */
	struct GMT_IMAGE *Image;	/* Pointer to the active image [NUYLL if grid] */
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDIMAGE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDIMAGE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->G.rgb[GMT_BGD][0] = C->G.rgb[GMT_BGD][1] = C->G.rgb[GMT_BGD][2] = 1.0;	/* White background pixel, black foreground defaults  */
	C->I.azimuth = strdup ("-45.0");	/* Default azimuth for shading when -I+d is used */
	C->I.method  = strdup ("t1");		/* Default normalization for shading when -I+d is used */
	C->I.ambient = strdup ("0");		/* Default ambient light for shading when -I+d is used */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->I.azimuth);
	gmt_M_str_free (C->I.ambient);
	gmt_M_str_free (C->I.method);
	gmt_M_str_free (C->Out.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *A = " [-A<out_img>[=<driver>]]";
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	const char *extra[2] = {A, " [-A]"};
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s %s%s [%s] [-C<cpt>] [-D[r]] [-Ei|<dpi>] "
		"[-G<rgb>[+b|f]] [-I[<intensgrid>|<value>|<modifiers>]] %s[-M] [-N] %s%s[-Q[<color>][+z<value>]] "
		"[%s] [%s] [%s] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_INGRID, GMT_J_OPT, extra[API->external], GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_Rgeo_OPT, GMT_U_OPT,
		GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_f_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of a grid or image to plot");
	GMT_Usage (API, -2, "Note: Grid z-values are in user units and will be "
		"converted to rgb colors via the CPT.  Alternatively, give a raster image. "
		"If the image is plain (e.g., JPG, PNG, GIF) you must also give a corresponding -R.");
	if (API->external)	/* External interface */
		GMT_Usage (API, -2, "If -D is used then <grid> is instead expected to be an image.");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	if (API->external)	/* External interface */
		GMT_Usage (API, 1, "\n-A Return a GMT raster image instead of a PostScript plot.");
	else {
		GMT_Usage (API, 1, "\n-A<out_img>[=<driver>]");
		GMT_Usage (API, -2, "Save image in a raster format (.bmp, .gif, .jpg, .png, .tif) instead of PostScript. "
			"If filename does not have any of these extensions then append =<driver> to select "
			"the desired image format. The 'driver' is the driver code name used by GDAL. "
			"See GDAL documentation for available drivers. Note: any vector elements are lost.");
	}
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C<cpt>");
	GMT_Usage (API, -2, "Color palette file to convert grid values to colors. Optionally, name a master cpt "
		"to automatically assign continuous colors over the data range [%s]; if so, "
		"optionally append +i<dz> to quantize the range [the exact grid range]. "
		"Another option is to specify -C<color1>,<color2>[,<color3>,...] to build a "
		"linear continuous cpt from those colors automatically.", API->GMT->current.setting.cpt);
	GMT_Usage (API, 1, "\n-D[r]");
	if (API->external)	/* External interface */
		GMT_Usage (API, -2, "Input is an image instead of a grid. Append r to equate image region to -R region.");
	else {
		GMT_Usage (API, -2, "GMT automatically detects standard image formats. For non-standard formats you must "
			"use -D to force it to be seen as an image. Append r to equate image region with -R region.");
	}
	GMT_Usage (API, 1, "\n-Ei|<dpi>");
	GMT_Usage (API, -2, "Set dpi for the projected grid which must be constructed [100] "
		"if -J implies a nonlinear graticule [Default gives same size as input grid]. "
		"Alternatively, append i to do the interpolation in PostScript at device resolution (invalid with -Q).");
	gmt_rgb_syntax (API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images. ");
	GMT_Usage (API, 3, "+b Set background color.");
	GMT_Usage (API, 3, "+f Set foreground color [Default].");
	GMT_Usage (API, 1, "\n-I[<intensgrid>|<value>|<modifiers>]");
	GMT_Usage (API, -2, "Apply directional illumination. Append name of an intensity grid, or "
		"for a constant intensity (i.e., change the ambient light), just give a scalar. "
		"To derive intensities from <grid> instead, append desired modifiers:");
	GMT_Usage (API, 3, "+a Specify <azim> of illumination [-45].");
	GMT_Usage (API, 3, "+n Set the <method> and <scale> to use [t1].");
	GMT_Usage (API, 3, "+m Set <ambient> light to add [0].");
	GMT_Usage (API, -2, "Alternatively, use -I+d to accept the default values (see grdgradient for more details). "
		"To derive intensities from another grid than <grid>, give the alternative data grid with suitable modifiers.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-M Force a monochrome (gray-scale) image.");
	GMT_Usage (API, 1, "\n-N Do Not clip image at the map boundary.");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-Q[<color>]");
	GMT_Usage (API, -2, "Use color-masking to make grid nodes with z = NaN or black image pixels transparent. "
		"Append an alternate <color> to change the transparent pixel for images [black], or alternatively");
	GMT_Usage (API, 3, "+z Specify <value> to set transparent pixel color via CPT lookup.");
	GMT_Option (API, "R");
	GMT_Option (API, "U,V,X,c,f,n,p,t,x,.");

	return (GMT_MODULE_USAGE);
}

/* A few non-exported library functions we need here only */

EXTERN_MSC int gmtinit_parse_n_option (struct GMT_CTRL *GMT, char *item);
EXTERN_MSC int gmtlib_get_grdtype (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_GRID_HEADER *h);
EXTERN_MSC int gmtlib_read_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);

static int parse (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdimage and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, ind, off, k;
	char *c = NULL, *file[3] = {NULL, NULL, NULL};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	size_t n;
	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one or three is accepted) */
				Ctrl->In.active = true;
				if (n_files >= 3) {n_errors++; continue; }
				file[n_files] = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(file[n_files]))) n_errors++;
				n_files++;
				break;
			case '>':	/* Output file (probably for -A via external interface) */
				Ctrl->Out.active = true;
				if (Ctrl->Out.file == NULL)
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Get image file name plus driver name to write via GDAL */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				if (API->external) {	/* External interface only */
					if ((n = strlen (opt->arg)) > 0) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: No output argument allowed\n");
						n_errors++;
					}
					Ctrl->A.return_image = true;
					Ctrl->A.way = 1;	/* Building image directly, use TRPa layout, no call to GDAL */
				}
				else if ((n = strlen (opt->arg)) == 0) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -A: No output name provided\n");
					n_errors++;
				}
				else if (gmt_M_file_is_memory (opt->arg)) {
					Ctrl->A.file = strdup (opt->arg);
					Ctrl->A.way = 1;	/* Building image directly, use TRPa layout, no call to GDAL */
				}
				else if ((c = gmt_get_ext (opt->arg)) && !strcmp (c, "ppm")) {	/* Want a ppm image which we can do without GDAL */
					Ctrl->A.file = strdup (opt->arg);
					Ctrl->A.way = 1;	/* Building image directly, use TRP layout, no call to GDAL, writing a PPM file */
				}
				else {	/* Must give file and GDAL driver */
					Ctrl->A.file = strdup (opt->arg);
					while (Ctrl->A.file[n] != '=' && n > 0) n--;
					if (n == 0) {	/* Gave no driver, see if we requested one of the standard image formats */
						n = strlen (Ctrl->A.file) - 1;
						while (n && Ctrl->A.file[n] != '.') n--;
						if (n == 0) {	/* Gave no image extension either... */
							GMT_Report (API, GMT_MSG_ERROR, "Option -A: Missing image extension or =<driver> name.\n");
							n_errors++;
						}
						else {	/* Check if we got a recognized extension */
							unsigned int k, found = 0;
							n++;	/* Start of extension */
							for (k = 0; !found && k < N_IMG_EXTENSIONS; k++) {
								if (!strcmp (&(Ctrl->A.file[n]), gdal_ext[k]))
									found = 1;
							}
							if (found == 0) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -A: Missing the required =<driver> name.\n");
								n_errors++;
							}
							else
								GMT_Report (API, GMT_MSG_DEBUG, "grdimage: Auto-recognized GDAL driver from filename extension.\n");
						}
					}
				}
				break;

			case 'C':	/* CPT */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				gmt_cpt_interval_modifier (GMT, &(Ctrl->C.file), &(Ctrl->C.dz));
				break;
			case 'D':	/* Get an image via GDAL */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				if (!API->external)			/* For externals we actually still need the -D */
					GMT_Report (API, GMT_MSG_COMPAT,
					            "Option -D is deprecated; images are detected automatically\n");
				Ctrl->D.active = true;
				Ctrl->D.mode = (opt->arg[0] == 'r');
				break;
			case 'E':	/* Sets dpi */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				if (opt->arg[0] == 'i')	/* Interpolate image to device resolution */
					Ctrl->E.device_dpi = true;
				else if (opt->arg[0] == '\0')
					Ctrl->E.dpi = 100;	/* Default grid dpi */
				else
					Ctrl->E.dpi = atoi (opt->arg);
				break;
			case 'G':	/* -G<color>[+b|f] 1-bit fore- or background color for transparent masks (was -G[f|b]<color>) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if ((c = strstr (opt->arg, "+b"))) {	/* Background color */
					ind = GMT_BGD;	off = GMT_FGD;	k = 0;	c[0] = '\0';
				}
				else if ((c = strstr (opt->arg, "+f"))) {	/* Foreground color */
					ind = GMT_FGD;	off = GMT_BGD;	k = 0;	c[0] = '\0';
				}
				else if (gmt_M_compat_check (GMT, 5) && strchr ("bf", opt->arg[0])) {	/* No modifier given so heck if first character is f or b */
					k = 1;
					if (opt->arg[0] == 'b') ind = GMT_BGD, off = GMT_FGD;
					else ind = GMT_FGD, off = GMT_BGD;
				}
				else {	/* Modern syntax where missing modifier means +f and just color is given */
					ind = GMT_FGD;	off = GMT_BGD;	k = 0;
				}
				if (opt->arg[k] && gmt_getrgb (GMT, &opt->arg[k], Ctrl->G.rgb[ind])) {
					gmt_rgb_syntax (GMT, 'G', " ");
					n_errors++;
				}
				else
					Ctrl->G.rgb[off][0] = -1;
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'I':	/* Use intensity from grid or constant or auto-compute it */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if ((c = strstr (opt->arg, "+d"))) {	/* Gave +d, so derive intensities from the input grid using default settings */
					Ctrl->I.derive = true;
					c[0] = '\0';	/* Chop off modifier */
				}
				else if ((c = gmt_first_modifier (GMT, opt->arg, "amn"))) {	/* Want to control how grdgradient is run */
					unsigned int pos = 0;
					char p[GMT_BUFSIZ] = {""};
					Ctrl->I.derive = true;
					while (gmt_getmodopt (GMT, 'I', c, "amn", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'a': gmt_M_str_free (Ctrl->I.azimuth); Ctrl->I.azimuth  = strdup (&p[1]); break;
							case 'm': gmt_M_str_free (Ctrl->I.ambient);  Ctrl->I.ambient = strdup (&p[1]); break;
							case 'n': gmt_M_str_free (Ctrl->I.method);  Ctrl->I.method   = strdup (&p[1]); break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';	/* Chop off all modifiers so range can be determined */
				}
				else if (!opt->arg[0] || !strcmp (opt->arg, "+")) {	/* Gave deprecated options -I or -I+ to derive intensities from the input grid using default settings */
					Ctrl->I.derive = true;
					if (opt->arg[0]) opt->arg[0] = '\0';	/* Remove the single + */
				}
				if (opt->arg[0]) {	/* Gave an argument in addition to or instead of a modifier */
					/* If given a file then it is either a intensity grid to use as is or, if I.derive is true, an alternate grid form which to derive intensities */
					if (!gmt_access (GMT, opt->arg, R_OK))	/* Got a physical grid */
						Ctrl->I.file = strdup (opt->arg);
					else if (gmt_M_file_is_remote (opt->arg))	/* Got a remote grid */
						Ctrl->I.file = strdup (opt->arg);
					else if (opt->arg[0] && gmt_is_float (GMT, opt->arg)) {	/* Looks like a constant value */
						Ctrl->I.value = atof (opt->arg);
						Ctrl->I.constant = true;
					}
				}
				else if (!Ctrl->I.derive) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
				if (c) c[0] = '+';	/* Restore the plus */
				break;
			case 'M':	/* Monochrome image */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not clip at map boundary */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				break;
			case 'Q':	/* PS3 colormasking */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				if ((c = strstr (opt->arg, "+z"))) {	/* Gave a z-value */
					if (c[2]) {
						Ctrl->Q.value = atof (&c[2]);
						Ctrl->Q.z_given = true;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Q: The +z modifier requires a valid z-value\n");
						n_errors++;
					}
					c[0] = '\0';	/* Chop off modifier */
				}
				if (opt->arg[0]) {	/* Change input image transparency pixel color */
					if (gmt_getrgb (GMT, opt->arg, Ctrl->Q.rgb)) {	/* Change input image transparency pixel color */
						gmt_rgb_syntax (GMT, 'Q', " ");
						n_errors++;
					}
					else
						Ctrl->Q.transp_color = true;
				}
				if (c) c[0] = '+';	/* Restore the modifier */
				break;
			case 'W':	/* Warn if no image, usually when called from grd2kml */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (n_files == 3) {	/* Old-style, deprecated way of plotting images via red, green, blue grids*/
		/* We will combine these three grids into an image instead */
		char output[GMT_VF_LEN] = {""}, cmd[GMT_LEN512] = {""};
		GMT_Report (API, GMT_MSG_COMPAT, "Passing three grids instead of an image is deprecated.  Please consider using an image instead.\n");
		GMT_Open_VirtualFile (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_OUT|GMT_IS_REFERENCE, NULL, output);
		for (k = 0; k < 3; k++)
			gmt_filename_set (file[k]);	/* Replace any spaces with ASCII 29 */
		sprintf (cmd, "%s %s %s -C -N -G%s", file[0], file[1], file[2], output);
		if (GMT_Call_Module (API, "grdmix", GMT_MODULE_CMD, cmd)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to combine %s/%s/%s into an image - aborting.\n", file[0], file[1], file[2]);
			n_errors++;
		}
		Ctrl->In.file = strdup (output);
	}
	else if (n_files == 1)	/* Got a single grid or image */
		Ctrl->In.file = strdup (file[0]);
	for (k = 0; k < 3; k++)
		gmt_M_str_free (file[k]);

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	if (!GMT->common.n.active && (!Ctrl->C.active || gmt_is_cpt_master (GMT, Ctrl->C.file)))
		/* Unless user selected -n we want the default not to exceed data range on projection when we are auto-scaling a master table */
		n_errors += gmtinit_parse_n_option (GMT, "c+c");

	if (!API->external) {	/* I.e, not an External interface */
		n_errors += gmt_M_check_condition (GMT, !(n_files == 1 || n_files == 3),
		                                   "Must specify one (or three [deprecated]) input file(s)\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->I.constant && !Ctrl->I.file && !Ctrl->I.derive,
	                                   "Option -I: Must specify intensity file, value, or modifiers\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.derive && n_files == 3,
	                                   "Option -I: Cannot derive intensities when r,g,b grids are given as data\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.derive && Ctrl->D.active,
	                                   "Option -I: Cannot derive intensities when an image is given as data\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0,
	                                   "Option -E: dpi must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.rgb[GMT_FGD][0] < 0 && Ctrl->G.rgb[GMT_BGD][0] < 0,
	                                   "Option -G: Only one of fore/back-ground can be transparent for 1-bit images\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && Ctrl->Q.active,
	                                   "SOption -Q: Cannot use -M when doing colormasking\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.device_dpi && Ctrl->Q.active,
	                                   "Option -Q: Cannot use -Ei when doing colormasking\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.return_image && Ctrl->Out.file == NULL,
	                                   "Option -A: Must provide an output filename for image\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.file && Ctrl->Out.file,
								       "Option -A, -> options: Cannot provide two output files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active,
								       "Options -C and -D options are mutually exclusive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.transp_color && Ctrl->Q.z_given,
								       "Option Q: Cannot both specify a r/g/b and a z-value\n");
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int grdimage_clean_global_headers (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* Problem is that many global grids are reported with misleading -R or imprecise
	 * -R or non-global -R yet nx * xinc = 360, etc.  We fix these here for GMT use. */
	unsigned int flag = 0;
	double wasR[4], wasI[2];
	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* We know x is geographic longitude */
		if (gmt_M_360_range (h->wesn[XLO], h->wesn[XHI]))	/* Exact 360 range is great */
			flag = 0;
		else if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < GMT_CONV4_LIMIT)	/* Pretty close to 360, shame on manufacturer of this grid */
			flag |= 1;	/* Sloppy, we can improve this a bit */
		else if (fabs (h->n_columns * h->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
			/* If n*xinc = 360 and previous test failed then we do not have a repeat node.  These are therefore pixel grids */
			flag |= 2;	/* Global but flawed -R and registration */
		}
	}
	if (gmt_M_y_is_lat (GMT, GMT_IN)) {	/* We know y is geographic latitude */
		if (gmt_M_180_range (h->wesn[YLO], h->wesn[YHI]))	/* Exact 180 range is great */
			flag |= 0;
		else if (fabs (h->wesn[YHI] - h->wesn[YLO] - 180.0) < GMT_CONV4_LIMIT)	/* Pretty close to 180, shame on manufacturer of this grid */
			flag |= 4;	/* Sloppy, we can improve this a bit */
		else if (fabs (h->n_rows * h->inc[GMT_Y] - 180.0) < GMT_CONV4_LIMIT)
			flag |= 8;	/* Global but flawed -R and registration */
	}
	if (!(flag == 9 || flag == 6)) return 0;	/* Only deal with mixed cases of gridline and pixel reg in lon vs lat */
	gmt_M_memcpy (wasR, h->wesn, 4, double);
	gmt_M_memcpy (wasI, h->inc, 2, double);
	if ((flag & 1) && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_X] = 360.0 / (h->n_columns - 1);	/* Get exact increment */
		h->wesn[XHI] = h->wesn[XLO] + 360.0;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected geographic global gridline-registered grid with sloppy longitude bounds.\n");
	}
	if ((flag & 2) && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_X] = 360.0 / h->n_columns;	/* Get exact increment */
		h->wesn[XHI] = h->wesn[XLO] + h->inc[GMT_X] * (h->n_columns - 1);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected geographic global pixel-registered grid posing as gridline-registered with sloppy longitude bounds.\n");
	}
	if ((flag & 4) && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_Y] = 180.0 / (h->n_rows - 1);	/* Get exact increment */
		h->wesn[YLO] = -90.0;
		h->wesn[YHI] = 90.0;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected geographic global gridline-registered grid with gridline latitude bounds.\n");
	}
	if ((flag & 8) && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_Y] = 180.0 / h->n_rows;	/* Get exact increment */
		h->wesn[YLO] = -90.0 + 0.5 * h->inc[GMT_Y];
		h->wesn[YHI] = h->wesn[YLO] +  h->inc[GMT_Y] * (h->n_rows - 1);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected geographic global pixel-registered grid posing as gridline-registered with sloppy latitude bounds.\n");
	}
	if (flag) {	/* Report the before and after regions and increments */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Old region: %g/%g/%g/%g with incs %g/%g\n", wasR[XLO], wasR[XHI], wasR[YLO], wasR[YHI], wasI[GMT_X], wasI[GMT_Y]);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "New region: %g/%g/%g/%g with incs %g/%g\n", h->wesn[XLO], h->wesn[XHI], h->wesn[YLO], h->wesn[YHI], h->inc[GMT_X], h->inc[GMT_Y]);
	}
	return 1;
}

GMT_LOCAL void grdimage_set_proj_limits (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *r, struct GMT_GRID_HEADER *g, bool projected, unsigned int mixed) {
	/* Sets the projected extent of the grid given the map projection.
	 * The extreme x/y coordinates are returned in r, with inc and
	 * n_columns/n_rows set accordingly.  Note that some of these may change
	 * if gmt_project_init is called at a later stage */

	unsigned int i, k;
	bool all_lats = false, all_lons = false;
	double x, y;
	gmt_M_unused (mixed);

	r->n_columns = g->n_columns;	r->n_rows = g->n_rows;
	r->registration = g->registration;
	r->n_bands = g->n_bands;

	/* By default, use entire plot region */

	gmt_M_memcpy (r->wesn, GMT->current.proj.rect, 4, double);

	if (GMT->current.proj.projection_GMT == GMT_GENPER && GMT->current.proj.g_width != 0.0) return;
	if (gmt_M_is_azimuthal (GMT) && !GMT->current.proj.polar) return;

	/* This fails when -R is not the entire grid region and projected is false. */
	if (projected && gmt_M_is_geographic (GMT, GMT_IN)) {
		all_lats = gmt_grd_is_polar (GMT, g);
		all_lons = gmt_grd_is_global (GMT, g);
		if (all_lons && all_lats) return;	/* Whole globe */
	}
	/* Must search for extent along perimeter */

	r->wesn[XLO] = r->wesn[YLO] = +DBL_MAX;
	r->wesn[XHI] = r->wesn[YHI] = -DBL_MAX;
	k = (g->registration == GMT_GRID_NODE_REG) ? 1 : 0;

	for (i = 0; i < g->n_columns - k; i++) {	/* South and north sides */
		gmt_geo_to_xy (GMT, g->wesn[XLO] + i * g->inc[GMT_X], g->wesn[YLO], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
		gmt_geo_to_xy (GMT, g->wesn[XHI] - i * g->inc[GMT_X], g->wesn[YHI], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
	}
	for (i = 0; i < g->n_rows - k; i++) {	/* East and west sides */
		gmt_geo_to_xy (GMT, g->wesn[XLO], g->wesn[YHI] - i * g->inc[GMT_Y], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
		gmt_geo_to_xy (GMT, g->wesn[XHI], g->wesn[YLO] + i * g->inc[GMT_Y], &x, &y);
		r->wesn[XLO] = MIN (r->wesn[XLO], x), r->wesn[XHI] = MAX (r->wesn[XHI], x);
		r->wesn[YLO] = MIN (r->wesn[YLO], y), r->wesn[YHI] = MAX (r->wesn[YHI], y);
	}
	if (projected) {
		if (all_lons) {	/* Full 360, use min/max for x */
			r->wesn[XLO] = GMT->current.proj.rect[XLO];	r->wesn[XHI] = GMT->current.proj.rect[XHI];
		}
		if (all_lats) {	/* Full -90/+90, use min/max for y */
			r->wesn[YLO] = GMT->current.proj.rect[YLO];	r->wesn[YHI] = GMT->current.proj.rect[YHI];
		}
		if (GMT->current.map.is_world && gmt_M_is_periodic (GMT)) {	/* Worry about grids crossing a periodic boundary as the search above may fail */
			if (g->wesn[XLO] < (GMT->current.proj.central_meridian+180) && g->wesn[XHI] > (GMT->current.proj.central_meridian+180))
				r->wesn[XLO] = GMT->current.proj.rect[XLO],	r->wesn[XHI] = GMT->current.proj.rect[XHI];
			else if (g->wesn[XLO] < (GMT->current.proj.central_meridian-180) && g->wesn[XHI] > (GMT->current.proj.central_meridian-180))
				r->wesn[XLO] = GMT->current.proj.rect[XLO],	r->wesn[XHI] = GMT->current.proj.rect[XHI];
		}
	}
	else if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Extra check for non-projected longitudes that wrap */
		double x1;
		gmt_geo_to_xy (GMT, g->wesn[XHI]-g->inc[GMT_X], g->wesn[YHI], &x1, &y);
		gmt_geo_to_xy (GMT, g->wesn[XHI], g->wesn[YHI], &x, &y);
		if (x < x1) /* Wrapped around because end of pixel is outside east; use plot width instead */
			r->wesn[XHI] = r->wesn[XLO] + GMT->current.proj.rect[XHI];
	}
}

GMT_LOCAL void grdimage_blackwhite_PS_image (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, unsigned char *image, unsigned int n_columns, unsigned int n_rows, double x_LL, double y_LL, double dx, double dy) {
	/* This function takes an 8-bit grayshade image that we know is only white (255) or black (0) and converts it
	 * to a 1-bit black/white image suitable for PSL to plot using PostScripts image operator for 1-bit images.
	 * Because all projections and scalings have already taken place, this is a simple scanline operation. */
	unsigned char *bitimage = NULL;
	unsigned int row, col, nx8, shift, b_or_w, nx_pixels;
	uint64_t imsize, k, k8, byte;
	double x_side, y_side = n_rows * dy;

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Converting 8-bit image to 1-bit B/W image then plotting it\n");

	nx8 = irint (ceil (n_columns / 8.0));	/* Image width must be a multiple of 8 bits, so we round up */
	nx_pixels = nx8 * 8;	/* The row length in bits after the rounding up */
	imsize = gmt_M_get_nm (GMT, nx8, n_rows);
	bitimage = gmt_M_memory (GMT, NULL, imsize, unsigned char);	/* Memory to hold the 1-bit image */

	/* Reprocess the byte image.  Here there are no worries about direction of rows, cols since that was dealt with during color assignment */

	for (row = 0, k = k8 = 0; row < n_rows; row++) {	/* Process each scan line */
		shift = 0; byte = 0;
		for (col = 0; col < n_columns; col++, k++) {	/* Visit each byte in the original gray shade image */
			b_or_w = (image[k] == 255);	/* Let white == 1, black == 0 */
			byte |= b_or_w;	/* Add in the current bit (0 or 1) */
			shift++;	/* Position us for the next bit */
			if (shift == 8) {	/* Did all 8, time to dump out another byte ["another one bytes the dust", he, he,...] */
				bitimage[k8++] = (unsigned char) byte;	/* Place the last 8 bits in output array... */
				byte = shift = 0;	/* ...and start over for next sequence of 8 bit nodes */
			}
			else	/* Move the bits we have so far 1 step to the left */
				byte <<= 1;
		}
		if (shift) {	/* Set the remaining bits in this bit to white; this can apply to the last 1-7 nodes on each row */
			byte |= 1;
			shift++;
			while (shift < 8) {
				byte <<= 1;
				byte |= 1;
				shift++;
			}
			bitimage[k8++] = (unsigned char) byte;	/* Copy final byte from this row into the image */
		}
	}

	x_side = nx_pixels * dx;	/* Since the image may be 1-7 bits wider than what we need we may enlarge it by a tiny amount */
	PSL_plotbitimage (GMT->PSL, x_LL, y_LL, x_side, y_side, PSL_BL, bitimage, nx_pixels, n_rows, Ctrl->G.rgb[GMT_FGD], Ctrl->G.rgb[GMT_BGD]);
	gmt_M_free (GMT, bitimage);	/* Done with the B/W image array */
}

/* Here lies specific loop functions over grids and optional intensities etc.  Because the two data sets (when intensities are involved)
 * may very well have different padding, we must compute two node counters in some cases. When that is the case we always set these:
 * H_s:    Pointer to the header of the SOURCE grid or image
 * H_i:    Pointer to the header of the INTENSITY grid
 * kk_s:   Start index for current row of the SOURCE grid or image
 * kk_i:   Start index for current row of the intensity grid (sometimes not needed when we compute node_i directly)
 * node_s: The current node of the SOURCE grid or image
 * node_i: THe current node of the INTENSITY grid
 *
 * In the cases without intensity we simply use H_s, kk_s, and node_s for consistency.
 */

GMT_LOCAL void grdimage_grd_gray_no_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) grid, 2) grayscale, 3) no intensity */
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, node_s;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	gmt_M_unused (Ctrl);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> gray image with no illumination.\n");
#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,scol,node_s,rgb) shared(GMT,Conf,H_s,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this data row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			(void)gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			image[byte++] = gmt_M_u255 (rgb[0]);	/* Color table only has grays, just use r since r = g = b here */
		}
	}
}

GMT_LOCAL void grdimage_grd_gray_with_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) grid, 2) grayscale, 3) with intensity */
	int index;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, kk_i, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */
	bool different = (Conf->int_mode && !gmt_M_grd_same_region (GMT,Conf->Grid,Conf->Intens));	/* True of they differ in padding */

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> gray image with illumination.\n");
#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,kk_i,scol,node_s,node_i,index,rgb) shared(GMT,Conf,Ctrl,different,H_s,H_i,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start node of this row in the data grid */
		kk_i = (different) ? gmt_M_ijpgi (H_i, Conf->actual_row[srow], 0) : kk_s;	/* Start node of same row in the intensity grid */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			index = gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			if (index != GRDIMAGE_NAN_INDEX) {	/* Add in the effect of illumination */
				if (Conf->int_mode) {	/* Intensity value comes from a co-registered grid */
					node_i = kk_i + Conf->actual_col[scol];	/* Current intensity node */
					gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);
				}
				else	/* A constant (ambient) intensity was given via -I */
					gmt_illuminate (GMT, Ctrl->I.value, rgb);
			}
			image[byte++] = gmt_M_u255 (rgb[0]);	/* Color table only has grays, just use r since r = g = b here */
		}
	}
}

GMT_LOCAL void grdimage_grd_c2s_no_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) grid, 2) color -> gray via YIQ, 3) no intensity */
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, node_s;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	gmt_M_unused (Ctrl);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> color image with no illumination.\n");
#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,scol,node_s,rgb) shared(GMT,Conf,H_s,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this row in the data grid */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			(void)gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			image[byte++] = gmt_M_u255 (gmt_M_yiq (rgb));
		}
	}
}

GMT_LOCAL void grdimage_grd_c2s_with_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) grid, 2) color -> gray via YIQ, 3) no intensity */
	int index;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, kk_i, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */
	bool different = (Conf->int_mode && !gmt_M_grd_same_region (GMT,Conf->Grid,Conf->Intens));	/* True of they differ in padding */

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> color image with no illumination.\n");
#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,kk_i,scol,node_s,node_i,index,rgb) shared(GMT,Conf,Ctrl,different,H_s,H_i,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this row */
		kk_i = (different) ? gmt_M_ijpgi (H_i, Conf->actual_row[srow], 0) : kk_s;	/* Start pixel of this row in the intensity grid */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current data grid node */
			index = gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			if (index != GRDIMAGE_NAN_INDEX) {	/* Deal with illumination */
				if (Conf->int_mode) {	/* Intensity value comes from the grid */
					node_i = kk_i + Conf->actual_col[scol];	/* Current intensity grid node */
					gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);
				}
				else	/* A constant (ambient) intensity was given via -I */
					gmt_illuminate (GMT, Ctrl->I.value, rgb);
			}
			image[byte++] = gmt_M_u255 (gmt_M_yiq (rgb));
		}
	}
}

GMT_LOCAL void grdimage_grd_color_no_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) grid, 2) color, 3) no intensity */
	int k;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, node_s;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	gmt_M_unused (Ctrl);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> color image with no illumination.\n");
#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,k,scol,node_s,rgb) shared(GMT,Conf,H_s,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)Conf->colormask_offset + 3 * srow * Conf->n_columns;	/* Start of output color image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			(void)gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			for (k = 0; k < 3; k++) image[byte++] = gmt_M_u255 (rgb[k]);
		}
	}
}

GMT_LOCAL void grdimage_grd_color_no_intensity_CM (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image, unsigned char *rgb_used) {
	/* Function that fills out the image in the special case of 1) grid, 2) color, 3) no intensity */
	unsigned char i_rgb[3];
	int k, index;
	int64_t srow, scol;
	uint64_t byte, kk_s, node_s;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	gmt_M_unused (Ctrl);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> color image with no illumination.\n");
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)Conf->colormask_offset + 3 * srow * Conf->n_columns;	/* Start of output color image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			index = gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			for (k = 0; k < 3; k++) image[byte++] = i_rgb[k] = gmt_M_u255 (rgb[k]);
			if (index != GRDIMAGE_NAN_INDEX) {	/* Deal with illumination */
				index = (i_rgb[0]*256 + i_rgb[1])*256 + i_rgb[2];	/* The index into the cube for the selected NaN color */
				rgb_used[index] = true;
			}
		}
	}
}

GMT_LOCAL void grdimage_grd_color_with_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) grid, 2) color, 3) no intensity */
	int index, k;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, kk_i, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */
	bool different = (Conf->int_mode && !gmt_M_grd_same_region (GMT,Conf->Grid,Conf->Intens));	/* True of they differ in padding */

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> color image with no illumination.\n");
#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,kk_i,k,scol,node_s,node_i,index,rgb) shared(GMT,Conf,Ctrl,different,H_s,H_i,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)Conf->colormask_offset + 3 * srow * Conf->n_columns;	/* Start of output color image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this data row */
		kk_i = (different) ? gmt_M_ijpgi (H_i, Conf->actual_row[srow], 0) : kk_s;	/* Start pixel of this row in the intensity grid */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			index = gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			if (index != GRDIMAGE_NAN_INDEX) {	/* Deal with illumination */
				if (Conf->int_mode) {	/* Intensity value comes from the grid */
					node_i = kk_i + Conf->actual_col[scol];	/* Current intensity node */
					gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);
				}
				else	/* A constant (ambient) intensity was given via -I */
					gmt_illuminate (GMT, Ctrl->I.value, rgb);
			}
			for (k = 0; k < 3; k++) image[byte++] = gmt_M_u255 (rgb[k]);
		}
	}
}

GMT_LOCAL void grdimage_grd_color_with_intensity_CM (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image, unsigned char *rgb_used) {
	/* Function that fills out the image in the special case of 1) grid, 2) color, 3) no intensity */
	unsigned char i_rgb[3], n_rgb[3];
	int index, k;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, kk_i, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Grid->header;	/* Pointer to the active data header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */
	bool different = (Conf->int_mode && !gmt_M_grd_same_region (GMT,Conf->Grid,Conf->Intens));

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Basic z(x,y) -> color image with no illumination.\n");
	/* Determine NaN rgb 0-255 triple ones */
	(void)gmt_get_rgb_from_z (GMT, Conf->P, GMT->session.d_NaN, rgb);
	for (k = 0; k < 3; k++) n_rgb[k] = gmt_M_u255 (rgb[k]);

	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines */
		byte = (uint64_t)Conf->colormask_offset + 3 * srow * Conf->n_columns;	/* Start of output color image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this data row */
		kk_i = (different) ? gmt_M_ijpgi (H_i, Conf->actual_row[srow], 0) : kk_s;	/* Start pixel of this row in the intensity grid */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Current grid node */
			index = gmt_get_rgb_from_z (GMT, Conf->P, Conf->Grid->data[node_s], rgb);
			if (index == GRDIMAGE_NAN_INDEX) {	 /* Nan color */
				for (k = 0; k < 3; k++) image[byte++] = n_rgb[k];
			}
			else {	/* Deal with illumination for non-NaN nodes */
				if (Conf->int_mode) {	/* Intensity value comes from the grid */
					node_i = kk_i + Conf->actual_col[scol];	/* Current intensity node */
					gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);
				}
				else	/* A constant (ambient) intensity was given via -I */
					gmt_illuminate (GMT, Ctrl->I.value, rgb);
				for (k = 0; k < 3; k++) i_rgb[k] = image[byte++] = gmt_M_u255 (rgb[k]);
				index = (i_rgb[0]*256 + i_rgb[1])*256 + i_rgb[2];	/* The index into the cube for the selected NaN color */
				rgb_used[index] = true;
			}
		}
	}
}

GMT_LOCAL void grdimage_img_set_transparency (struct GMT_CTRL *GMT, unsigned char pix4, double *rgb) {
	/* JL: Here we assume background color is white, hence t * 1.
	   But what would it take to have a user selected background color? */
	double o, t;		/* o - opacity, t = transparency */
	o = pix4 / 255.0;	t = 1 - o;
	rgb[0] = o * rgb[0] + t * GMT->current.map.frame.fill[GMT_Z].rgb[0];
	rgb[1] = o * rgb[1] + t * GMT->current.map.frame.fill[GMT_Z].rgb[1];
	rgb[2] = o * rgb[2] + t * GMT->current.map.frame.fill[GMT_Z].rgb[2];
}

GMT_LOCAL void grdimage_img_gray_with_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) image, 2) gray, 3) with intensity */
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Image->header;	/* Pointer to the active image header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */

#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,scol,node_s,node_i,rgb) shared(GMT,Conf,Ctrl,H_s,H_i,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines in the output bitimage */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this image row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Start of current pixel node */
			rgb[0] = rgb[1] = rgb[2] = gmt_M_is255 (Conf->Image->data[node_s++]);
			if (Conf->int_mode) {	/* Intensity value comes from the grid, so update node */
				node_i = gmt_M_ijp (H_i, Conf->actual_row[srow], Conf->actual_col[scol]);
				gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);	/* Apply illumination to this color */
			}
			else	/* A constant (ambient) intensity was given via -I */
				gmt_illuminate (GMT, Ctrl->I.value, rgb);	/* Apply constant illumination to this color */
			image[byte++] = gmt_M_u255 (rgb[0]);
		}
	}
}

GMT_LOCAL void grdimage_img_gray_no_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) image, 2) gray, 3) no intensity */
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t byte, kk_s, node_s;
	struct GMT_GRID_HEADER *H_s = Conf->Image->header;	/* Pointer to the active data header */
	gmt_M_unused (GMT);
	gmt_M_unused (Ctrl);

#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,scol,node_s) shared(GMT,Conf,Ctrl,H_s,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines in the output bitimage */
		byte = (uint64_t)srow * Conf->n_columns;
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this image row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol];	/* Start of current pixel node */
			image[byte++] = Conf->Image->data[node_s];
		}
	}
}

GMT_LOCAL void grdimage_img_c2s_with_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) image, 2) color -> gray via YIQ, 3) with intensity */
	bool transparency = (Conf->Image->header->n_bands == 4);
	int64_t srow, scol, k;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t n_bands = Conf->Image->header->n_bands;
	uint64_t byte, kk_s, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Image->header;	/* Pointer to the active image header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */

#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,k,scol,node_s,node_i,rgb) shared(GMT,Conf,Ctrl,H_s,H_i,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines in the output bitimage */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this image row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol] * n_bands;	/* Start of current pixel node */
			for (k = 0; k < 3; k++) rgb[k] = gmt_M_is255 (Conf->Image->data[node_s++]);
			if (transparency && Conf->Image->data[node_s] < 255)	/* Dealing with an image with transparency values less than 255 */
				grdimage_img_set_transparency (GMT, Conf->Image->data[node_s], rgb);
			if (Conf->int_mode) {	/* Intensity value comes from the grid, so update node */
				node_i = gmt_M_ijp (H_i, Conf->actual_row[srow], Conf->actual_col[scol]);
				gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);	/* Apply illumination to this color */
			}
			else	/* A constant (ambient) intensity was given via -I */
				gmt_illuminate (GMT, Ctrl->I.value, rgb);	/* Apply constant illumination to this color */
			image[byte++] = gmt_M_u255 (gmt_M_yiq (rgb));
		}
	}
}

GMT_LOCAL void grdimage_img_c2s_no_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) image, 2) color -> gray via YIQ, 3) with intensity */
	bool transparency = (Conf->Image->header->n_bands == 4);
	int k;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t n_bands = Conf->Image->header->n_bands;
	uint64_t byte, kk_s, node_s;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Image->header;	/* Pointer to the active data header */
	gmt_M_unused (Ctrl);

#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,k,scol,node_s,rgb) shared(GMT,Conf,Ctrl,H_s,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines in the output bitimage */
		byte = (uint64_t)srow * Conf->n_columns;	/* Start of output gray image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol] * n_bands;	/* Start of current pixel node */
			for (k = 0; k < 3; k++) rgb[k] = gmt_M_is255 (Conf->Image->data[node_s++]);
			if (transparency && Conf->Image->data[node_s] < 255)	/* Dealing with an image with transparency values less than 255 */
				grdimage_img_set_transparency (GMT, Conf->Image->data[node_s], rgb);
			image[byte++] = gmt_M_u255 (gmt_M_yiq (rgb));
		}
	}
}

GMT_LOCAL void grdimage_img_color_no_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) image, 2) color, 3) with intensity */
	bool transparency = (Conf->Image->header->n_bands == 4);
	int k;	/* Due to OPENMP on Windows requiring signed int loop variables */
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t n_bands = Conf->Image->header->n_bands;
	uint64_t byte, kk_s, node_s;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Image->header;	/* Pointer to the active data header */
	gmt_M_unused (Ctrl);

#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,k,scol,node_s,rgb) shared(GMT,Conf,Ctrl,H_s,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines in the output bitimage */
		byte = (uint64_t)Conf->colormask_offset + 3 * srow * Conf->n_columns;	/* Start of output color image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol] * n_bands;	/* Start of current pixel node */
			for (k = 0; k < 3; k++) rgb[k] = gmt_M_is255 (Conf->Image->data[node_s++]);
			if (transparency && Conf->Image->data[node_s] < 255)	/* Dealing with an image with transparency values less than 255 */
				grdimage_img_set_transparency (GMT, Conf->Image->data[node_s], rgb);
			for (k = 0; k < 3; k++) image[byte++] = gmt_M_u255 (rgb[k]);	/* Scale up to integer 0-255 range */
		}
	}
}

GMT_LOCAL void grdimage_img_color_with_intensity (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GRDIMAGE_CONF *Conf, unsigned char *image) {
	/* Function that fills out the image in the special case of 1) image, 2) color, 3) with intensity */
	bool transparency = (Conf->Image->header->n_bands == 4);
	int k;
	int64_t srow, scol;	/* Due to OPENMP on Windows requiring signed int loop variables */
	uint64_t n_bands = Conf->Image->header->n_bands;
	uint64_t byte, kk_s, node_s, node_i;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_GRID_HEADER *H_s = Conf->Image->header;	/* Pointer to the active image header */
	struct GMT_GRID_HEADER *H_i = (Conf->int_mode) ? Conf->Intens->header : NULL;	/* Pointer to the active intensity header */

#ifdef _OPENMP
#pragma omp parallel for private(srow,byte,kk_s,k,scol,node_s,node_i,rgb) shared(GMT,Conf,Ctrl,H_s,H_i,image)
#endif
	for (srow = 0; srow < Conf->n_rows; srow++) {	/* March along scanlines in the output bitimage */
		byte = (uint64_t)Conf->colormask_offset + 3 * srow * Conf->n_columns;	/* Start of output color image row */
		kk_s = gmt_M_ijpgi (H_s, Conf->actual_row[srow], 0);	/* Start pixel of this image row */
		for (scol = 0; scol < Conf->n_columns; scol++) {	/* Compute rgb for each pixel along this scanline */
			node_s = kk_s + Conf->actual_col[scol] * n_bands;	/* Start of current input pixel node */
			for (k = 0; k < 3; k++) rgb[k] = gmt_M_is255 (Conf->Image->data[node_s++]);
			if (transparency && Conf->Image->data[node_s] < 255)	/* Dealing with an image with transparency values less than 255 */
				grdimage_img_set_transparency (GMT, Conf->Image->data[node_s], rgb);
			if (Conf->int_mode) {	/* Intensity value comes from the grid, so update node */
				node_i = gmt_M_ijp (H_i, Conf->actual_row[srow], Conf->actual_col[scol]);
				gmt_illuminate (GMT, Conf->Intens->data[node_i], rgb);	/* Apply illumination to this color */
			}
			else	/* A constant (ambient) intensity was given via -I */
				gmt_illuminate (GMT, Ctrl->I.value, rgb);	/* Apply constant illumination to this color */
			for (k = 0; k < 3; k++) image[byte++] = gmt_M_u255 (rgb[k]);	/* Scale up to integer 0-255 range */
		}
	}
}

GMT_LOCAL bool grdimage_adjust_R_consideration (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* As per https://github.com/GenericMappingTools/gmt/issues/4440, when the user wants
	 * to plot a pixel-registered global grid using a lon-lat scaling with periodic boundaries
	 * and a central meridian that is not a multiple of the grid increment, we must actually
	 * adjust the plot domain -R to be such a multiple.  That, or require projection. */

	double delta;

	if (!gmt_M_is_geographic (GMT, GMT_IN)) return false;	/* No geographic */
	if (h->registration == GMT_GRID_NODE_REG) return false;	/* gridline-registration has the repeated column needed */
	if (gmt_M_is_nonlinear_graticule (GMT)) return true;	/* Always have to project when given most projections except -JQ */
	if (!gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) return false;	/* No repeating columns would be visible */
	delta = remainder (h->wesn[XLO] - GMT->common.R.wesn[XLO], h->inc[GMT_X]);
	if (gmt_M_is_zero (delta)) return false;	/* No need to project if it is lining up */
	/* Here we need to adjust plot region */
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your grid is pixel-registered, the projection is simply longlat, and plot region is a full 360 degrees in longitude.\n");
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Due to lack of repeated boundary nodes, -Rw/e must be given in multiples of the grid increment (%g)\n", h->inc[GMT_X]);
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Current region (-R) setting in longitude is %g to %g\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	GMT->common.R.wesn[XLO] += delta;
	GMT->common.R.wesn[XHI] += delta;
	if (GMT->common.R.wesn[XHI] <= 0.0) {
		GMT->common.R.wesn[XLO] += 360.0;
		GMT->common.R.wesn[XHI] += 360.0;
	}
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Adjusted region (-R) setting in longitude is %g to %g\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	return false;
}


void grdimage_reset_grd_minmax (struct GMT_CTRL *GMT, struct GMT_GRID *G, double *zmin, double *zmax) {
	/* grdimage via gmt_grd_setregion may extend the grid outward to ensure we have enough nodes to
	 * fill the map.  However, some of these nodes are actually outside the w/e/s/n requested. Here,
	 * we check for simple cases where we can shrink the w/e/s/n back temporarily to recompute the grid
	 * zmin/zmax which are often used for scaling of a CPT.  This can be done in these situations:
	 * 	1. Not an oblique projection
	 * 	2. -R was set and is not 360 in longitude
	 * 	3. G->header->wesn is not 360 in longitude
	 */
	unsigned int pad[4], k, n_pad = 0;
	double old_wesn[4], new_wesn[4], old_z_min, old_z_max;
	if (GMT->common.R.oblique) return;	/* Do nothing for oblique maps */
	if (!GMT->common.R.active[RSET]) return;	/* Do nothing if -R was not set */
	if (gmt_M_360_range (G->header->wesn[XLO], G->header->wesn[XHI])) return;	/* Do nothing for 360 grids */
	if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) return;	/* Do nothing for global maps */
	/* OK, may try to do a bit of work */
	gmt_M_memcpy (old_wesn, G->header->wesn, 4, double);	/* Save a copy of what we have */
	gmt_M_memcpy (new_wesn, G->header->wesn, 4, double);	/* Save a copy of what we have */
	old_z_min = G->header->z_min;	old_z_max = G->header->z_max;
	pad[XLO] = (G->header->wesn[XLO] < GMT->common.R.wesn[XLO]) ? irint (floor ((GMT->common.R.wesn[XLO] - G->header->wesn[XLO] + GMT_CONV12_LIMIT) / G->header->inc[GMT_X])) : 0;
	pad[XHI] = (G->header->wesn[XHI] > GMT->common.R.wesn[XHI]) ? irint (floor ((G->header->wesn[XHI] - GMT->common.R.wesn[XHI] + GMT_CONV12_LIMIT) / G->header->inc[GMT_X])) : 0;
	pad[YLO] = (G->header->wesn[YLO] < GMT->common.R.wesn[YLO]) ? irint (floor ((GMT->common.R.wesn[YLO] - G->header->wesn[YLO] + GMT_CONV12_LIMIT) / G->header->inc[GMT_Y])) : 0;
	pad[YHI] = (G->header->wesn[YHI] > GMT->common.R.wesn[YHI]) ? irint (floor ((G->header->wesn[YHI] - GMT->common.R.wesn[YHI] + GMT_CONV12_LIMIT) / G->header->inc[GMT_Y])) : 0;
	for (k = 0; k < 4; k++) if (pad[k]) {
		new_wesn[k] = GMT->common.R.wesn[k];	/* Snap back to -R */
		n_pad++;	/* Number of nonzero pads */
	}
	if (n_pad == 0) return;	/* No change */
	gmt_M_memcpy (G->header->wesn, new_wesn, 4, double);	/* Temporarily update the header */
	for (k = 0; k < 4; k++) G->header->pad[k] += pad[k];	/* Temporarily change the pad */
	gmt_set_grddim (GMT, G->header);	/* Change header items */
	gmt_grd_zminmax (GMT, G->header, G->data);		/* Recompute the min/max */
	*zmin = G->header->z_min;	*zmax = G->header->z_max;	/* These are then passed out */
	gmt_M_memcpy (G->header->wesn, old_wesn, 4, double);	/* Reset the header */
	for (k = 0; k < 4; k++) G->header->pad[k] -= pad[k];	/* Reset the pad */
	gmt_set_grddim (GMT, G->header);	/* Reset header items */
	G->header->z_min = old_z_min;	G->header->z_max = old_z_max;	/* Restore original min/max */
}

EXTERN_MSC int gmtlib_ind2rgb (struct GMT_CTRL *GMT, struct GMT_IMAGE **I_in);

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_M_free (GMT, Conf); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* Two macros used to detect if an image has no meta-data for region and increments */
#define img_inc_is_one(h) (h->inc[GMT_X] == 1.0 && h->inc[GMT_Y] == 1.0)
#define img_region_is_dimension(h) (h->wesn[XLO] == 0.0 && h->wesn[YLO] == 0.0 && img_inc_is_one(h) && \
                                    urint (h->wesn[XHI]) == h->n_columns && urint (h->wesn[YHI]) == h->n_rows)
#define img_region_is_invalid(h) (h->wesn[XLO] == 0.0 && h->wesn[YLO] == 0.0 && img_inc_is_one(h) && \
                                  (h->wesn[YHI] > 90.0 || h->wesn[XHI] > 720.0))

EXTERN_MSC int GMT_grdimage (void *V_API, int mode, void *args) {
	bool done, need_to_project, normal_x, normal_y, resampled = false, gray_only = false;
	bool nothing_inside = false, use_intensity_grid = false, got_data_tiles = false, rgb_cube_scan;
	bool has_content, mem_G = false, mem_I = false, mem_D = false, got_z_grid = true;
	unsigned int grid_registration = GMT_GRID_NODE_REG, try, row, col, mixed = 0, pad_mode = 0;
	uint64_t node, k, kk, dim[GMT_DIM_SIZE] = {0, 0, 3, 0};
	int error = 0, ret_val = GMT_NOERROR, ftype = GMT_NOTSET;

	char *img_ProjectionRefPROJ4 = NULL, *way[2] = {"via GDAL", "directly"}, cmd[GMT_LEN256] = {""}, data_grd[GMT_VF_LEN] = {""}, *e = NULL;
	unsigned char *bitimage_8 = NULL, *bitimage_24 = NULL, *rgb_used = NULL;

	double dx, dy, x_side, y_side, x0 = 0.0, y0 = 0.0;
	double img_wesn[4], img_inc[2] = {1.0, 1.0};    /* Image increments & min/max for writing images or external interfaces */
	double *NaN_rgb = NULL, red[4] = {1.0, 0.0, 0.0, 0.0}, black[4] = {0.0, 0.0, 0.0, 0.0}, wesn[4] = {0.0, 0.0, 0.0, 0.0};
	double *Ix = NULL, *Iy = NULL;	/* Pointers to hold on to read-only x/y image arrays */

	struct GMT_GRID *Grid_orig = NULL, *Grid_proj = NULL;
	struct GMT_GRID *Intens_orig = NULL, *Intens_proj = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRDIMAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;        /* General PSL internal parameters */
	struct GMT_GRID_HEADER *header_work = NULL;	/* Pointer to a GMT header for the image or grid */
	struct GMT_GRID_HEADER *header_D = NULL, *header_I = NULL, *header_G = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct GMT_IMAGE *I = NULL, *Img_proj = NULL;	/* A GMT image datatype */
	struct GMT_IMAGE *Out = NULL;	/* A GMT image datatype, if external interface is used with -A */
	struct GMT_GRID *G2 = NULL;
	struct GRDIMAGE_CONF *Conf = NULL;

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

	/*---------------------------- This is the grdimage main code ----------------------------*/

	if ((Conf = gmt_M_memory (GMT, NULL, 1, struct GRDIMAGE_CONF)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to allocate Conf structure\n");
		Return (GMT_MEMORY_ERROR);
	}

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	use_intensity_grid = (Ctrl->I.active && !Ctrl->I.constant);	/* We want to use an intensity grid */
	if (Ctrl->A.file) {
		Ctrl->Out.file = Ctrl->A.file; Ctrl->A.file = NULL;	/* Only use Out.file for writing */
		if ((e = gmt_get_ext (Ctrl->Out.file)) && strcmp (e, "ppm")) {	/* Turn off the automatic creation of aux files by GDAL */
#ifdef WIN32
			if (_putenv ("GDAL_PAM_ENABLED=NO"))
#else
			if (setenv ("GDAL_PAM_ENABLED", "NO", 0))
#endif
				GMT_Report (API, GMT_MSG_WARNING, "Unable to set GDAL_PAM_ENABLED to prevent writing of auxiliary files\n");
		}
	}

	/* Determine if grid/image is to be projected */
	need_to_project = (gmt_M_is_nonlinear_graticule (GMT) || Ctrl->E.dpi > 0);
	if (need_to_project)
		GMT_Report (API, GMT_MSG_DEBUG, "Projected grid is non-orthogonal, nonlinear, or dpi was changed\n");
	else  if (Ctrl->D.active)			/* If not projecting no need for a pad */
		gmt_set_pad(GMT, 0);
	pad_mode = (need_to_project) ? GMT_GRID_NEEDS_PAD2 : 0;
	/* Read the illumination grid header right away so we can use its region to set that of an image (if requested) */
	if (use_intensity_grid) {	/* Illumination grid desired */
		if (Ctrl->I.derive) {	/* Illumination grid must be derived */
			/* If input grid is actually a list of data tiles then we must read that list first since this will
			 * force all tiles to be downloaded, converted, and stitched into a single grid per -R. This must
			 * happen _before_ we auto-derive intensities via grdgradient so that there is an input data grid */

			if (Ctrl->I.file == NULL && gmt_file_is_tiled_list (API, Ctrl->In.file, NULL, NULL, NULL)) {	/* Must read and stitch the data tiles first */
				if ((Grid_orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA|pad_mode, API->tile_wesn, Ctrl->In.file, NULL)) == NULL) {	/* Get stitched grid */
					Return (API->error);
				}
				if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, Grid_orig, data_grd)) {
					Return (API->error);
				}
				got_data_tiles = true;
			}
		}
		else {	/* Illumination grid present and can be read */
			mem_I = gmt_M_file_is_memory (Ctrl->I.file);	/* Remember if the intensity grid was passed as a memory grid */
			GMT_Report (API, GMT_MSG_INFORMATION, "Read intensity grid header from file %s\n", Ctrl->I.file);
			if ((Intens_orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|pad_mode, NULL, Ctrl->I.file, NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
		}
	}

	if (!Ctrl->D.active && (ftype = gmt_raster_type (GMT, Ctrl->In.file, false)) == GMT_IS_IMAGE) {
		/* The input file is an ordinary image instead of a grid and -R may be required to use it */
		Ctrl->D.active = true;
		if (GMT->common.R.active[RSET] && !strstr (Ctrl->In.file, "earth_")) Ctrl->D.mode = true;
	}

	if (!Ctrl->D.active && ftype == GMT_IS_GRID) {	/* See if input could be an image of a kind that could also be a grid and we don't yet know what it is.  Pass GMT_GRID_IS_IMAGE mode */
		if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY | GMT_GRID_IS_IMAGE|pad_mode, NULL, Ctrl->In.file, NULL)) != NULL) {
			gmtlib_read_grd_info (GMT, Ctrl->In.file, I->header);	/* Re-read header as grid to ensure orig_datatype is set */
			HH = gmt_get_H_hidden (I->header);	/* Get hidden structure */
			if (HH->orig_datatype == GMT_UCHAR || HH->orig_datatype == GMT_CHAR) Ctrl->D.active = true;
			/* Guess that if the image region goes from 0 to col/rol-dimensions then there is no region metadata and we want to set -Dr */
			if (Ctrl->D.active && GMT->common.R.active[RSET]) {	/* Should we add -Dr or is it just a valid subset specification? */
				if (gmt_M_is_geographic (GMT, GMT_IN) && img_region_is_invalid (I->header)) Ctrl->D.mode = true;
				else if (img_region_is_dimension (I->header)) Ctrl->D.mode = true;
			}
		}
	}

	gmt_detect_oblique_region (GMT, Ctrl->In.file);	/* Ensure a proper and smaller -R for oblique projections */

	if (Ctrl->D.active) {	/* Main input is a single image and not a grid */
		bool R_save = GMT->common.R.active[RSET];
		double *I_wesn = (API->got_remote_wesn) ? API->tile_wesn : NULL;
		if (I && !need_to_project) {
			gmt_M_memcpy (wesn, GMT->common.R.active[RSET] ? GMT->common.R.wesn : I->header->wesn, 4, double);
			need_to_project = (gmt_whole_earth (GMT, I->header->wesn, wesn));	/* Must project global images even if not needed for grids */
		}
		pad_mode = (need_to_project) ? GMT_GRID_NEEDS_PAD2 : 0;
		if (Ctrl->I.derive) {	/* Cannot auto-derive intensities from an image */
			GMT_Report (API, GMT_MSG_WARNING, "Cannot derive intensities from an input image file; -I ignored\n");
			Ctrl->I.derive = use_intensity_grid = false;
		}
		if (use_intensity_grid && GMT->common.R.active[RSET]) {
			/* Make sure the region of the intensity grid and -R are in agreement within a noise threshold */
			double xnoise = Intens_orig->header->inc[GMT_X]*GMT_CONV4_LIMIT, ynoise = Intens_orig->header->inc[GMT_Y]*GMT_CONV4_LIMIT;
			if (GMT->common.R.wesn[XLO] < (Intens_orig->header->wesn[XLO]-xnoise) || GMT->common.R.wesn[XHI] > (Intens_orig->header->wesn[XHI]+xnoise) ||
			    GMT->common.R.wesn[YLO] < (Intens_orig->header->wesn[YLO]-ynoise) || GMT->common.R.wesn[YHI] > (Intens_orig->header->wesn[YHI]+ynoise)) {
				GMT_Report (API, GMT_MSG_ERROR, "Requested region exceeds illumination extent\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}

		if (!Ctrl->D.mode && use_intensity_grid && !GMT->common.R.active[RSET])	/* Apply illumination to an image but no -R provided; use intensity domain as -R region */
			gmt_M_memcpy (GMT->common.R.wesn, Intens_orig->header->wesn, 4, double);

		if (Ctrl->D.mode && GMT->common.R.active[RSET]) {
			/* Need to assign given -R as the image's -R, so cannot pass -R in when reading */
			GMT->common.R.active[RSET] = false;	/* Temporarily turn off -R if given */
            I_wesn = NULL;
		}
		/* Read in the the entire image that is to be mapped */
		GMT_Report (API, GMT_MSG_INFORMATION, "Allocate memory and read image file %s\n", Ctrl->In.file);
		if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA | GMT_IMAGE_NO_INDEX | pad_mode, I_wesn, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}
		GMT->common.R.active[RSET] = R_save;	/* Restore -R if it was set */
		grid_registration = I->header->registration;	/* This is presumably pixel registration since it is an image */
		if (grid_registration != GMT_GRID_PIXEL_REG)
			GMT_Report(API, GMT_MSG_INFORMATION, "Your image has gridline registration yet all images ought to be pixel registered.\n");
		mixed = grdimage_clean_global_headers (GMT, I->header);	/* Possibly adjust flawed global -R settings in the image header */
		HH = gmt_get_H_hidden (I->header);
		if ((I->header->n_bands > 1 && strncmp (I->header->mem_layout, "BRP", 3)) || strncmp (I->header->mem_layout, "BR", 2))
			GMT_Report(API, GMT_MSG_INFORMATION, "The image memory layout (%s) may be of the wrong type. It should be BRPa.\n", I->header->mem_layout);

		if (!Ctrl->D.mode && !Ctrl->I.active && !GMT->common.R.active[RSET])	/* No -R or -I were set. Use image dimensions as -R */
			gmt_M_memcpy (GMT->common.R.wesn, I->header->wesn, 4, double);

		if ( (Ctrl->D.mode && GMT->common.R.active[RSET]) || (!Ctrl->D.mode && use_intensity_grid) ) {	/* In either case the -R was set above or on command line */
			gmt_M_memcpy (I->header->wesn, GMT->common.R.wesn, 4, double);
			/* Get actual size of each pixel in user units */
			dx = gmt_M_get_inc (GMT, I->header->wesn[XLO], I->header->wesn[XHI], I->header->n_columns, I->header->registration);
			dy = gmt_M_get_inc (GMT, I->header->wesn[YLO], I->header->wesn[YHI], I->header->n_rows,    I->header->registration);
			I->header->inc[GMT_X] = dx;	I->header->inc[GMT_Y] = dy;
			HH->r_inc[GMT_X] = 1.0 / dx;	/* Get inverse increments to avoid divisions later */
			HH->r_inc[GMT_Y] = 1.0 / dy;
			HH->grdtype = gmtlib_get_grdtype (GMT, GMT_IN, I->header);	/* Since we now have a proper region we can determine geo/cart type */
			if (API->external) {	/* Cannot free and update read-only image so just juggle new and old arrays */
				Ix = I->x;	Iy = I->y;	/* Keep old arrays */
			}
			else {	/* Reset the grid x/y arrays */
				gmt_M_free (GMT, I->x);	gmt_M_free (GMT, I->y);
			}
			/* Assign image coordinate arrays */
			I->x = gmt_grd_coord (GMT, I->header, GMT_X);
			I->y = gmt_grd_coord (GMT, I->header, GMT_Y);
		}

		if (!Ctrl->A.active && gmtlib_ind2rgb(GMT, &I)) {
			GMT_Report (API, GMT_MSG_ERROR, "Error converting from indexed to RGB\n");
			Return (API->error);
		}

		gray_only  = (I->header->n_bands == 1 && I->n_indexed_colors == 0);	/* Got a grayscale image */
		got_z_grid = false;		/* Flag that we are using a GMT_IMAGE instead of a GMT_GRID as main input */

		if (I->header->ProjRefPROJ4 != NULL)	/* We are not using this information yet, but report it under -V */
			GMT_Report (API, GMT_MSG_INFORMATION, "Data projection (Proj4 type)\n\t%s\n", I->header->ProjRefPROJ4);

		header_work = I->header;	/* OK, that's what what we'll use as header region to send to gmt_grd_setregion */
	}

	if (!Ctrl->D.active) {	/* Read the header of the input grid */
		mem_G = gmt_M_file_is_memory (Ctrl->In.file);	/* Remember if we got the grid via a memory file or not */
		if (!got_data_tiles) {	/* Only avoid this step if we already read a data tile bunch earlier under the I.derive == true section above */
			GMT_Report (API, GMT_MSG_INFORMATION, "Read header from file %s\n", Ctrl->In.file);
			if ((Grid_orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|pad_mode, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			if ((API->error = gmt_img_sanitycheck (GMT, Grid_orig->header))) {	/* Used map projection on a Mercator (i.e., already a Cartesian) grid */
				Return (API->error);
			}
			if (!GMT->common.J.active) {	/* No map projection was supplied, set default */
				if ((Grid_orig->header->ProjRefWKT != NULL) || (Grid_orig->header->ProjRefPROJ4 != NULL)) {
					static char *Jarg[2] = {"X15c", "Q15c+"};	/* The two default projections for Cartesian and Geographic */
					unsigned int kind = gmt_M_is_geographic (GMT, GMT_IN);
					gmt_parse_common_options (GMT, "J", 'J', Jarg[kind]);	/* No projection specified, use linear or equidistant */
					GMT->common.J.active = true;	/* Now parsed */
				}
				else if (GMT->current.setting.run_mode == GMT_CLASSIC) {
					GMT_Report (API, GMT_MSG_ERROR, "Must specify a map projection with the -J option\n");
					Return (GMT_PARSE_ERROR);
				}
			}
		}
		if (!Ctrl->C.active)	/* Set no specific CPT so we turn -C on to use current or default CPT */
			Ctrl->C.active = true;	/* Use default CPT (GMT->current.setting.cpt) and autostretch or under modern reuse current CPT */
	}

	if (got_z_grid) header_work = Grid_orig->header;	/* OK, we are in GRID mode and this was not set previously. Do it now. */

	/* Determine what wesn to pass to gmt_map_setup */

	if (!GMT->common.R.active[RSET] && got_z_grid)	/* -R was not set so we use the grid domain */
		gmt_set_R_from_grd (GMT, Grid_orig->header);

	if (Ctrl->E.dpi == 0) grdimage_adjust_R_consideration (GMT, header_work);	/* Special check for global pixel-registered plots */

	/* Initialize the projection for the selected -R -J */
	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	/* Determine the wesn to be used to read the grid file; or bail if file is outside -R */

	if (!gmt_grd_setregion (GMT, header_work, wesn, need_to_project * GMT->common.n.interpolant))
		nothing_inside = true;	/* Means we can return an empty plot, possibly with a basemap */
	else if (use_intensity_grid && !Ctrl->I.derive && !gmt_grd_setregion (GMT, Intens_orig->header, wesn, need_to_project * GMT->common.n.interpolant))
		nothing_inside = true;

	if (nothing_inside) {	/* Create a blank plot (or image) */
		GMT_Report (API, GMT_MSG_WARNING, "No grid or image inside plot domain\n");
		/* No grid to plot; just do an empty map and bail */
		/* MISSING: Action to take if -A is in effect.  Need to create an empty image and return/save it */
		if (Ctrl->A.active) {	/* Create an empty image of the right dimensions */
			/* Not implemented yet */
			GMT_Report (API, GMT_MSG_WARNING, "No image returned\n");
		}
		else {	/* Plot that blank canvas */
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_AFTER, GMT_BASEMAP_ANNOT_AFTER);
			GMT->current.map.frame.order = GMT_BASEMAP_AFTER;	/* Move to last order since only calling gmt_map_basemap once */
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		if (Ctrl->W.active) ret_val = GMT_IMAGE_NO_DATA;	/* Flag that output image has no data - needed by grd2kml only so far */
		Return (ret_val);
	}

	/* Here the grid/image is inside the plot domain.  The same must be true of any
	 * auto-derived intensities we may create below */

	if (Ctrl->I.derive) {	/* Auto-create intensity grid from data grid using the now determined data region */
		bool got_int4_grid = false;
		char int_grd[GMT_VF_LEN] = {""}, int4_grd[GMT_VF_LEN] = {""};
		double *region = (got_data_tiles) ? header_work->wesn : wesn;	/* Region to pass to grdgradient */
		struct GMT_GRID *I_data = NULL;

		GMT_Report (API, GMT_MSG_INFORMATION, "Derive intensity grid from data grid %s\n", (Ctrl->I.file) ? Ctrl->I.file : data_grd);
		/* Create a virtual file to hold the intensity grid */
		if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT|GMT_IS_REFERENCE, NULL, int_grd))
			Return (API->error);
		if (Ctrl->I.file) {	/* Gave a file to derive from. In case it is a tiled grid we read it in here */
			if ((I_data = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA|pad_mode, wesn, Ctrl->I.file, NULL)) == NULL)	/* Get grid data header*/
				Return (API->error);
			/* If dimensions don't match the data grid we must resample this secondary z-grid */
			if (Grid_orig && (I_data->header->n_columns != Grid_orig->header->n_columns || I_data->header->n_rows != Grid_orig->header->n_rows)) {
				char int_z_grd[GMT_VF_LEN] = {""}, *res = "gp";
				if (I_data->header->wesn[XLO] > region[XLO] || I_data->header->wesn[XHI] < region[XHI] || I_data->header->wesn[YLO] > region[YLO] || I_data->header->wesn[YHI] < region[YHI]) {
					GMT_Report (API, GMT_MSG_ERROR, "Your secondary data grid given via -I does not cover the same area as the primary grid - aborting\n");
					Return (GMT_GRDIO_DOMAIN_VIOLATION);
				}
				if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT|GMT_IS_REFERENCE, NULL, int_z_grd))
					Return (API->error);
				sprintf (cmd, "%s -R%.16g/%.16g/%.16g/%.16g -I%.16g/%.16g -r%c -G%s --GMT_HISTORY=readonly ",
					Ctrl->I.file, region[XLO], region[XHI], region[YLO], region[YHI], Grid_orig->header->inc[GMT_X], Grid_orig->header->inc[GMT_Y], res[Grid_orig->header->registration], int_z_grd);
				/* Call the grdsample module */
				GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdsample with args %s\n", cmd);
				if (GMT_Call_Module (API, "grdsample", GMT_MODULE_CMD, cmd))
					Return (API->error);
				/* Destroy the header we read so we can get the revised on from grdsample */
				if (GMT_Destroy_Data (API, &I_data) != GMT_NOERROR)
					Return (API->error);
				/* Obtain the resampled data from the virtual file */
				if ((I_data = GMT_Read_VirtualFile (API, int_z_grd)) == NULL)
					Return (API->error);
			}
			else if ((I_data = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|pad_mode, wesn, Ctrl->I.file, I_data)) == NULL)	/* Get grid data */
				Return (API->error);
			if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, I_data, int4_grd))
				Return (API->error);
			got_int4_grid = true;
		}
		/* Prepare the grdgradient arguments using selected -A -N and the data region in effect.  If we read in a tiled grid
		 * then it was made with 0/360 so we must use that region in grdgradient.  For non-tiles, we read the actual grid
		 * AFTER calling gmt_mapsetup which changes the region.  The tiled region remains unchanged because it is read in
		 * all at once as it is being assembled.  This fixes https://github.com/GenericMappingTools/gmt/issues/3694 */
		sprintf (cmd, "-G%s -A%s -N%s+a%s -R%.16g/%.16g/%.16g/%.16g --GMT_HISTORY=readonly ",
			int_grd, Ctrl->I.azimuth, Ctrl->I.method, Ctrl->I.ambient, region[XLO], region[XHI], region[YLO], region[YHI]);
		if (got_data_tiles)	/* Use the virtual file we made earlier when first getting a tiled grid */
			strcat (cmd, data_grd);
		else if (got_int4_grid)	/* Use the virtual file just assigned a few lines above this call */
			strcat (cmd, int4_grd);
		else {	/* Default is to use the data file; we quote it in case there are spaces in the filename */
			gmt_filename_set (Ctrl->In.file);	/* Replace any spaces with ASCII 29 */
			strcat (cmd, Ctrl->In.file);
			gmt_filename_get (Ctrl->In.file);	/* Replace any ASCII 29 with spaces */
		}
		/* Call the grdgradient module */
		GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdgradient with args %s\n", cmd);
		if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd))
			Return (API->error);
		/* Obtain the data from the virtual file */
		if ((Intens_orig = GMT_Read_VirtualFile (API, int_grd)) == NULL)
			Return (API->error);
		if (got_data_tiles)
			GMT_Close_VirtualFile (API, data_grd);
		else if (got_int4_grid)	/* Use the virtual file we made earlier */
			GMT_Close_VirtualFile (API, int4_grd);
	}

	if (got_z_grid) {	/* Get grid dimensions */
		double *region = (gmt_file_is_tiled_list (API, Ctrl->In.file, NULL, NULL, NULL)) ? API->tile_wesn : wesn;	/* Make sure we get correct dimensions if tiled grids are used */
		Conf->n_columns = gmt_M_get_n (GMT, region[XLO], region[XHI], Grid_orig->header->inc[GMT_X], Grid_orig->header->registration);
		Conf->n_rows    = gmt_M_get_n (GMT, region[YLO], region[YHI], Grid_orig->header->inc[GMT_Y], Grid_orig->header->registration);
	}
	else {	/* Trust the image info from GDAL to make it more stable against pixel vs grid registration troubles */
		Conf->n_columns = I->header->n_columns;
		Conf->n_rows    = I->header->n_rows;
	}

	if (!Ctrl->A.active) {	/* Initialize the PostScript plot */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_set_basemap_orders (GMT, Ctrl->N.active ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_AFTER, GMT_BASEMAP_ANNOT_BEFORE);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		gmt_map_basemap (GMT);
		if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	}

	/* Read the grid data, possibly via a subset specified in wesn */

	if (got_z_grid && !got_data_tiles) {	/* Only skip this step if we already read a tiled grid earlier under the (I.derive == true) section */
		GMT_Report (API, GMT_MSG_INFORMATION, "Allocate and read data from file %s\n", Ctrl->In.file);
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|pad_mode, wesn, Ctrl->In.file, Grid_orig) == NULL) {	/* Get grid data */
			Return (API->error);
		}
		mixed = grdimage_clean_global_headers (GMT, Grid_orig->header);	/* Possibly clean up near-global headers */
	}

	/* If given, get intensity grid or compute intensities (for a constant intensity) */

	if (use_intensity_grid) {	/* Illumination wanted */
		double *region = (gmt_file_is_tiled_list (API, Ctrl->In.file, NULL, NULL, NULL)) ? API->tile_wesn : wesn;	/* Subset to pass to GMT_Read_Data if data set is tiled */
		GMT_Report (API, GMT_MSG_INFORMATION, "Allocates memory and read intensity file\n");

		/* Remember, the illumination header was already read at the start of grdimage */
		if (!Ctrl->I.derive && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY|pad_mode, region, Ctrl->I.file, Intens_orig) == NULL) {
			Return (API->error);	/* Failed to read the intensity grid data */
		}
		mixed = grdimage_clean_global_headers (GMT, Intens_orig->header);	/* Possibly clean up near-global headers */
		if (got_z_grid && (Intens_orig->header->n_columns != Grid_orig->header->n_columns ||
		                Intens_orig->header->n_rows != Grid_orig->header->n_rows)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimensions of intensity grid do not match that of the data grid!\n");
			Return (GMT_RUNTIME_ERROR);
		}

		if (Ctrl->D.active && (I->header->n_columns != Intens_orig->header->n_columns || I->header->n_rows != Intens_orig->header->n_rows)) {
			/* Resize illumination grid to match the dimensions of the image via a call to grdsample */

			char in_string[GMT_VF_LEN] = {""}, out_string[GMT_VF_LEN] = {""};
    		/* Associate the intensity grid with an open virtual file - in_string will then hold the name of this input "file" */
    		GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, Intens_orig, in_string);
   			/* Create a virtual file to hold the resampled grid - out_string then holds the name of this output "file" */
    		GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT|GMT_IS_REFERENCE, NULL, out_string);
			/* Create the command to do the resampling via the grdsample module */
			sprintf (cmd, "%s -G%s -I%d+/%d+ --GMT_HISTORY=readonly", in_string, out_string, (int)Conf->n_columns, (int)Conf->n_rows);
			GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdsample with args %s\n", cmd);
			if (GMT_Call_Module (GMT->parent, "grdsample", GMT_MODULE_CMD, cmd) != GMT_NOERROR)	/* Do the resampling */
				return (API->error);
   			/* Obtain the resmapled intensity grid from the virtual file */
    		G2 = GMT_Read_VirtualFile (API, out_string);
			if (GMT_Destroy_Data (API, &Intens_orig) != GMT_NOERROR) {	/* We can now delete the original intensity grid ... */
				Return (API->error);
			}
			Intens_orig = G2;	/* ...and point to the resampled intensity grid */
		}
	}

	if (got_z_grid && (gmt_whole_earth (GMT, Grid_orig->header->wesn, wesn) == 1))
		need_to_project = true;	/* This can only happen if reading a remote global geographic grid and the central meridian differs from that of the projection */
	else if (Ctrl->D.active && (gmt_whole_earth (GMT, I->header->wesn, wesn) == 1))
		need_to_project = true;	/* This can only happen if reading a remote global geographic image and the central meridian differs from that of the projection */

	/* Get or calculate a color palette file */

	has_content = (got_z_grid) ? false : true;	/* Images always have content but grids may be all NaN */
	if (got_z_grid) {	/* Got a single grid so need to convert z to color via a CPT */
		if (Ctrl->C.active) {	/* Read a palette file */
			double zmin = Grid_orig->header->z_min, zmax = Grid_orig->header->z_max;
			char *cpt = gmt_cpt_default (API, Ctrl->C.file, Ctrl->In.file, Grid_orig->header);
			grdimage_reset_grd_minmax (GMT, Grid_orig, &zmin, &zmax);
			if ((P = gmt_get_palette (GMT, cpt, GMT_CPT_OPTIONAL, zmin, zmax, Ctrl->C.dz)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to read CPT %s.\n", Ctrl->C.file);
				gmt_free_header (API->GMT, &header_G);
				gmt_free_header (API->GMT, &header_I);
				Return (API->error);	/* Well, that did not go so well... */
			}
			if (cpt) gmt_M_str_free (cpt);
			gray_only = (P && P->is_gray);	/* Flag that we are doing a gray scale image below */
			Conf->P = P;
			if (P && P->has_pattern) GMT_Report (API, GMT_MSG_WARNING, "Patterns in CPTs will be ignored\n");
			if (Ctrl->Q.z_given) {	/* Obtain the transparent color based on P and value */
				(void)gmt_get_rgb_from_z (GMT, Conf->P, Ctrl->Q.value, Ctrl->Q.rgb);
				Ctrl->Q.transp_color = true;
			}
		}
		if (Ctrl->W.active) {	/* Check if there are just NaNs in the grid */
			for (node = 0; !has_content && node < Grid_orig->header->size; node++)
				if (!gmt_M_is_dnan (Grid_orig->data[node])) has_content = true;
		}
	}

	if (need_to_project) {	/* Need to resample the grid or image [and intensity grid] using the specified map projection */
		int nx_proj = 0, ny_proj = 0;
		double inc[2] = {0.0, 0.0};

		if (got_z_grid && P && P->categorical && (GMT->common.n.interpolant != BCR_NEARNEIGHBOR || GMT->common.n.antialias)) {
			GMT_Report (API, GMT_MSG_WARNING, "Your CPT is categorical. Enabling -nn+a to avoid interpolation across categories.\n");
			GMT->common.n.interpolant = BCR_NEARNEIGHBOR;
			GMT->common.n.antialias = false;
			GMT->common.n.threshold = 0.0;
			HH = gmt_get_H_hidden (Grid_orig->header);
			HH->bcr_threshold = GMT->common.n.threshold;
			HH->bcr_interpolant = GMT->common.n.interpolant;
			HH->bcr_n = 1;
		}
		if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
			nx_proj = Conf->n_columns;
			ny_proj = Conf->n_rows;
		}
		if (Ctrl->D.active) { /* Must project the input image instead */
			GMT_Report (API, GMT_MSG_INFORMATION, "Project the input image\n");
			if ((Img_proj = GMT_Duplicate_Data (API, GMT_IS_IMAGE, GMT_DUPLICATE_NONE, I)) == NULL) Return (API->error);	/* Just to get a header we can change */
			grid_registration = GMT_GRID_PIXEL_REG;	/* Force pixel */
			grdimage_set_proj_limits (GMT, Img_proj->header, I->header, need_to_project, mixed);
			if (gmt_M_err_fail (GMT, gmt_project_init (GMT, Img_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			                Ctrl->In.file)) Return (GMT_PROJECTION_ERROR);
			if (Ctrl->A.active) /* Need to set background color to white for raster images */
				for (k = 0; k < 3; k++) GMT->current.setting.color_patch[GMT_NAN][k] = 1.0;	/* For img GDAL write use white as bg color */
			gmt_set_grddim (GMT, Img_proj->header);	/* Recalculate projected image dimensions */
			if (GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Img_proj) == NULL)
				Return (API->error);	/* Failed to allocate memory for the projected image */
			if (gmt_img_project (GMT, I, Img_proj, false)) Return (GMT_RUNTIME_ERROR);	/* Now project the image onto the projected rectangle */
			if (!API->external && (GMT_Destroy_Data (API, &I) != GMT_NOERROR)) {	/* Free the original image now we have projected.  Use Img_proj from now on */
				Return (API->error);	/* Failed to free the image */
			}
		}
		else {	/* Project the grid */
			GMT_Report (API, GMT_MSG_INFORMATION, "Project the input grid\n");
			if ((Grid_proj = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Grid_orig)) == NULL)
				Return (API->error);	/* Just to get a header we can change */
			/* Determine the dimensions of the projected grid */
			grdimage_set_proj_limits (GMT, Grid_proj->header, Grid_orig->header, need_to_project, mixed);
			if (grid_registration == GMT_GRID_NODE_REG)		/* Force pixel if a dpi was specified, else keep as is */
				grid_registration = (Ctrl->E.dpi > 0) ? GMT_GRID_PIXEL_REG : Grid_orig->header->registration;
			if (gmt_M_err_fail (GMT, gmt_project_init (GMT, Grid_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			                Ctrl->In.file)) Return (GMT_PROJECTION_ERROR);
			gmt_set_grddim (GMT, Grid_proj->header);	/* Recalculate projected grid dimensions */
			if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid_proj) == NULL)
				Return (API->error);	/* Failed to allocate memory for the projected grid */
			if (gmt_grd_project (GMT, Grid_orig, Grid_proj, false)) Return (GMT_RUNTIME_ERROR);	/* Now project the grid onto the projected rectangle */
			if (GMT_Destroy_Data (API, &Grid_orig) != GMT_NOERROR) {	/* Free the original grid now we have projected.  Use Grid_proj from now on */
				Return (API->error);	/* Failed to free the original grid */
			}
		}
		if (use_intensity_grid) {	/* Must also project the intensity grid */
			GMT_Report (API, GMT_MSG_INFORMATION, "Project the intensity grid\n");
			if ((Intens_proj = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Intens_orig)) == NULL)	/* Just to get a header we can change */
				Return (API->error);
			if (got_z_grid)	/* Use projected grid bounds as template */
				gmt_M_memcpy (Intens_proj->header->wesn, Grid_proj->header->wesn, 4, double);
			else	/* Use projected image bounds as template */
				gmt_M_memcpy (Intens_proj->header->wesn, Img_proj->header->wesn, 4, double);

			if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
				nx_proj = Intens_orig->header->n_columns;
				ny_proj = Intens_orig->header->n_rows;
			}
			if (gmt_M_err_fail (GMT, gmt_project_init (GMT, Intens_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			                Ctrl->I.file)) Return (GMT_PROJECTION_ERROR);
			gmt_set_grddim (GMT, Intens_proj->header);	/* Recalculate projected intensity grid dimensions */
			if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Intens_proj) == NULL)
				Return (API->error);	/* Failed to allocate memory for the projected intensity grid */
			if (gmt_grd_project (GMT, Intens_orig, Intens_proj, false)) Return (GMT_RUNTIME_ERROR);	/* Now project the intensity grid onto the projected rectangle */
			if (GMT_Destroy_Data (API, &Intens_orig) != GMT_NOERROR) {	/* Free the original intensity grid now we have projected.  Use Intens_proj from now on */
				Return (API->error);	/* Failed to free the original intensity grid */
			}
		}
		resampled = true;	/* Yes, we did it */
	}
	else {	/* Simply set the unused Grid_proj/Intens_proj pointers to point to the original unprojected Grid_orig/Intens_orig objects */
		struct GMT_GRID_HEADER *tmp_header = gmt_get_header (GMT);
		if (got_z_grid) {	/* Must get a copy of the header so we can change one without affecting the other */
			gmt_copy_gridheader (GMT, tmp_header, Grid_orig->header);
			if (mem_G) {	/* Need a copy of the original headers so we can restore when done */
				header_G = gmt_get_header (GMT);
				gmt_copy_gridheader (GMT, header_G, Grid_orig->header);
			}
			Grid_proj = Grid_orig;
			grdimage_set_proj_limits (GMT, Grid_proj->header, tmp_header, need_to_project, mixed);
		}
		if (use_intensity_grid) {
			if (mem_I) {	/* Need a copy of the original headers so we can restore when done */
				header_I = gmt_get_header (GMT);
				gmt_copy_gridheader (GMT, header_I, Intens_orig->header);
			}
			Intens_proj = Intens_orig;
		}
		if (got_z_grid) /* Dealing with a projected grid, ensure registration is set */
			grid_registration = Grid_orig->header->registration;
		else {	/* Dealing with a projected image */
			gmt_copy_gridheader (GMT, tmp_header, I->header);
			if (mem_D) {	/* Need a copy of the original headers so we can restore when done */
				header_D = gmt_get_header (GMT);
				gmt_copy_gridheader (GMT, header_D, I->header);
			}
			Img_proj = I;
			grdimage_set_proj_limits (GMT, Img_proj->header, tmp_header, need_to_project, mixed);
		}
		gmt_free_header (API->GMT, &tmp_header);
	}

	/* From here, use Grid_proj or Img_proj plus optionally Intens_proj in making the (now) Cartesian rectangular image */

	if (got_z_grid) { /* Dealing with a projected grid, so we only have one band [z]*/
		Grid_proj->header->n_bands = 1;
		header_work = Grid_proj->header;	/* Later when need to refer to the header, use this copy */
	}
	else /* Use a different reference header for the image */
		header_work = Img_proj->header;		/* Later when need to refer to the header, use this copy */

	/* Assign the Conf structure members */
	Conf->Grid      = Grid_proj;
	Conf->Image     = Img_proj;
	Conf->Intens    = Intens_proj;
	Conf->n_columns = header_work->n_columns;
	Conf->n_rows    = header_work->n_rows;
	Conf->int_mode  = use_intensity_grid;
	Conf->nm        = header_work->nm;

	NaN_rgb = (P) ? P->bfn[GMT_NAN].rgb : GMT->current.setting.color_patch[GMT_NAN];	/* Determine which color represents a NaN grid node */
	if (got_z_grid && Ctrl->Q.active) {	/* Want colormasking via the grid's NaN entries */
		if (gray_only) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Your image is gray scale only but -Q requires building a 24-bit image; your image will be expanded to 24-bit.\n");
			gray_only = false;	/* Since we cannot do 8-bit and colormasking */
			NaN_rgb = (Ctrl->A.active) ? black : red;	/* Arbitrarily pick red for PS as the NaN color since the entire image is gray only, or black if returning an image directly */
			if (P) gmt_M_memcpy (P->bfn[GMT_NAN].rgb, NaN_rgb, 4, double);	/* Update the NaN color entry in the CPT in memory */
		}
		if (!Ctrl->A.return_image) rgb_used = gmt_M_memory (GMT, NULL, 256*256*256, unsigned char);	/* Keep track of which colors we encounter so we can find an unused unique one later */
	}

	if (Ctrl->A.active) {	/* We desire a raster image to be returned, not a PostScript plot */
		int	id, k;
		unsigned int this_proj = GMT->current.proj.projection;
		char mem_layout[5] = {""}, *pch = NULL;
		if (Ctrl->M.active || gray_only) dim[GMT_Z] = 1;	/* Only one band requested */
		if (!need_to_project) {	/* Stick with original -R */
			img_wesn[XLO] = GMT->common.R.wesn[XLO];		img_wesn[XHI] = GMT->common.R.wesn[XHI];
			img_wesn[YHI] = GMT->common.R.wesn[YHI];		img_wesn[YLO] = GMT->common.R.wesn[YLO];
		}
		else {	/* Must use the projected limits, here in meters */
			img_wesn[XLO] = GMT->current.proj.rect_m[XLO];	img_wesn[XHI] = GMT->current.proj.rect_m[XHI];
			img_wesn[YHI] = GMT->current.proj.rect_m[YHI];	img_wesn[YLO] = GMT->current.proj.rect_m[YLO];
		}
		/* Determine raster image pixel sizes in the final units */
		img_inc[0] = (img_wesn[XHI] - img_wesn[XLO]) / (Conf->n_columns - !grid_registration);
		img_inc[1] = (img_wesn[YHI] - img_wesn[YLO]) / (Conf->n_rows - !grid_registration);

		if (grid_registration == GMT_GRID_NODE_REG) {	/* Adjust domain by 1/2 pixel since SW pixel is outside the domain */
			img_wesn[XLO] -= 0.5 * img_inc[0];		img_wesn[XHI] += 0.5 * img_inc[0];
			img_wesn[YLO] -= 0.5 * img_inc[1];		img_wesn[YHI] += 0.5 * img_inc[1];
		}
		if (Ctrl->Q.active) dim[GMT_Z]++;	/* Flag to remind us that we need to allocate a transparency array in GMT_Create_Data */
		if (GMT->current.gdal_read_in.O.mem_layout[0])
			strcpy (mem_layout, GMT->current.gdal_read_in.O.mem_layout);	/* Backup current layout */
		else
			gmt_strncpy (mem_layout, "TRPa", 4);					/* Don't let it be empty (maybe it will screw?) */
		GMT_Set_Default (API, "API_IMAGE_LAYOUT", "TRPa");			/* Set grdimage's default image memory layout */

		if ((Out = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, dim, img_wesn, img_inc, 1, 0, NULL)) == NULL) {	/* Yikes, must bail */
			if (rgb_used) gmt_M_free (GMT, rgb_used);
			gmt_free_header (API->GMT, &header_G);
			gmt_free_header (API->GMT, &header_I);
			Return (API->error);	/* Well, no luck with that allocation */
		}

		GMT_Set_Default (API, "API_IMAGE_LAYOUT", mem_layout);		/* Reset to previous memory layout */

		HH = gmt_get_H_hidden (Out->header);
		if ((pch = strstr (Ctrl->Out.file, "+c")) != NULL) {		/* Check if we have +c<options> for GDAL */
			HH->pocket = strdup (pch);
			pch[0] = '\0';
		}

		/* See if we have valid proj info or the chosen projection has a valid PROJ4 setting */
		if (header_work->ProjRefWKT != NULL)
			Out->header->ProjRefWKT = strdup (header_work->ProjRefWKT);
		else if (header_work->ProjRefPROJ4 != NULL)
			Out->header->ProjRefPROJ4 = strdup (header_work->ProjRefPROJ4);
		else if (header_work->ProjRefEPSG)
			Out->header->ProjRefEPSG = header_work->ProjRefEPSG;
		else {
			for (k = 0, id = GMT_NOTSET; id == GMT_NOTSET && k < GMT_N_PROJ4; k++)
				if (GMT->current.proj.proj4[k].id == this_proj) id = k;
			if (id >= 0) 			/* Valid projection for creating world file info */
				img_ProjectionRefPROJ4 = gmt_export2proj4(GMT);	/* Get the proj4 string */
			Out->header->ProjRefPROJ4 = img_ProjectionRefPROJ4;
		}

		if (Ctrl->M.active || gray_only)	/* Only need a byte-array to hold this image */
			bitimage_8  = Out->data;
		else	/* Need 3-byte array for a 24-bit image */
			bitimage_24 = Out->data;
	}
	else {	/* Produce a PostScript image layer */
		if (Ctrl->M.active || gray_only)	/* Only need a byte-array to hold this image */
			bitimage_8 = gmt_M_memory (GMT, NULL, header_work->nm, unsigned char);
		else {	/* Need 3-byte array for a 24-bit image, plus possibly 3 bytes for the NaN mask color */
			if (Ctrl->Q.active) Conf->colormask_offset = 3;
			bitimage_24 = gmt_M_memory (GMT, NULL, 3 * header_work->nm + Conf->colormask_offset, unsigned char);
			if (Ctrl->Q.transp_color)
				for (k = 0; k < 3; k++) bitimage_24[k] = gmt_M_u255 (Ctrl->Q.rgb[k]);	/* Scale the specific rgb up to 0-255 range */
			else if (P)	/* Use the CPT NaN color */
				for (k = 0; k < 3; k++) bitimage_24[k] = gmt_M_u255 (P->bfn[GMT_NAN].rgb[k]);	/* Scale the NaN rgb up to 0-255 range */
			/* else we default to 0 0 0 of course */
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate image pixel colors\n");

	/* Worry about linear projections with negative scales that may reverse the orientation of the final image */
	normal_x = !(GMT->current.proj.projection_GMT == GMT_LINEAR && !GMT->current.proj.xyz_pos[0] && !resampled);
	normal_y = !(GMT->current.proj.projection_GMT == GMT_LINEAR && !GMT->current.proj.xyz_pos[1] && !resampled);

	Conf->actual_row = gmt_M_memory (GMT, NULL, Conf->n_rows, unsigned int);	/* Deal with any reversal of the y-axis due to -J */
	for (row = 0; row < (unsigned int)Conf->n_rows; row++) Conf->actual_row[row] = (normal_y) ? row : Conf->n_rows - row - 1;
	Conf->actual_col = gmt_M_memory (GMT, NULL, Conf->n_columns, unsigned int);	/* Deal with any reversal of the x-axis due to -J */
	for (col = 0; col < (unsigned int)Conf->n_columns; col++) Conf->actual_col[col] = (normal_x) ? col : Conf->n_columns - col - 1;

	rgb_cube_scan = (P && Ctrl->Q.active && !Ctrl->A.active);	/* Need to look for unique rgb for PostScript masking */

	/* Evaluate colors at least once (try = 0), but may do twice if -Q is active and we need to select another unique NaN color not used in the image */
	for (try = 0, done = false; !done && try < 2; try++) {
		if (got_z_grid) {	/* Dealing with Grids and CPT lookup */
			if (Ctrl->I.active) {	/* With intensity */
				if (gray_only)	/* Grid, grayscale, with intensity, with intensity */
					grdimage_grd_gray_with_intensity (GMT, Ctrl, Conf, bitimage_8);
				else if (Ctrl->M.active) 	/* Grid, color converted to gray, with intensity */
					grdimage_grd_c2s_with_intensity (GMT, Ctrl, Conf, bitimage_8);
				else if (Ctrl->Q.active) /* Must deal with colormasking */
					grdimage_grd_color_with_intensity_CM (GMT, Ctrl, Conf, bitimage_24, rgb_used);
				else 	/* Grid, color, with intensity, no colormasking */
					grdimage_grd_color_with_intensity (GMT, Ctrl, Conf, bitimage_24);
			}
			else {	/* No intensity */
				if (gray_only)	/* Grid, grayscale, no intensity */
					grdimage_grd_gray_no_intensity (GMT, Ctrl, Conf, bitimage_8);
				else if (Ctrl->M.active) 	/* Grid, color converted to gray, with intensity */
					grdimage_grd_c2s_no_intensity (GMT, Ctrl, Conf, bitimage_8);
				else if (Ctrl->Q.active) /* Must deal with colormasking */
					grdimage_grd_color_no_intensity_CM (GMT, Ctrl, Conf, bitimage_24, rgb_used);
				else	/* Grid, color, no intensity */
					grdimage_grd_color_no_intensity (GMT, Ctrl, Conf, bitimage_24);
			}
		}
		else {	/* Dealing with an image, so no CPT lookup nor colormasking */
			if (Ctrl->I.active) {	/* With intensity */
				if (gray_only)	/* Image, grayscale, with intensity, with intensity */
					grdimage_img_gray_with_intensity (GMT, Ctrl, Conf, bitimage_8);
				else if (Ctrl->M.active) 	/* Image, color converted to gray, with intensity */
					grdimage_img_c2s_with_intensity (GMT, Ctrl, Conf, bitimage_8);
				else 	/* Grid, color, with intensity */
					grdimage_img_color_with_intensity (GMT, Ctrl, Conf, bitimage_24);
			}
			else {	/* No intensity */
				if (gray_only)	/* Image, grayscale, no intensity */
					grdimage_img_gray_no_intensity (GMT, Ctrl, Conf, bitimage_8);
				else if (Ctrl->M.active) 	/* Image, color converted to gray, with intensity */
					grdimage_img_c2s_no_intensity (GMT, Ctrl, Conf, bitimage_8);
				else	/* Image, color, no intensity */
					grdimage_img_color_no_intensity (GMT, Ctrl, Conf, bitimage_24);
			}
		}
		if (rgb_cube_scan) {	/* Fill in the RGB cube use */
			int index = 0, ks;
			/* Check that we found an unused r/g/b value so that colormasking will work as advertised */
			index = (gmt_M_u255(P->bfn[GMT_NAN].rgb[0])*256 + gmt_M_u255(P->bfn[GMT_NAN].rgb[1]))*256 + gmt_M_u255(P->bfn[GMT_NAN].rgb[2]);	/* The index into the cube for the selected NaN color */
			if (rgb_used[index]) {	/* This r/g/b already appears in the image as a non-NaN color; we must find a replacement NaN color */
				for (index = 0, ks = GMT_NOTSET; ks == GMT_NOTSET && index < 256*256*256; index++)	/* Examine the entire cube */
					if (!rgb_used[index]) ks = index;	/* Use this unused entry instead */
				if (ks == GMT_NOTSET) {	/* This is really really unlikely, meaning image uses all 256^3 colors */
					GMT_Report (API, GMT_MSG_WARNING, "Colormasking will fail as there is no unused color that can represent transparency\n");
					done = true;
				}
				else {	/* Pick the first unused color (i.e., ks) and let it play the role of the NaN color for transparency */
					bitimage_24[0] = (unsigned char)(ks >> 16);
					bitimage_24[1] = (unsigned char)((ks >> 8) & 255);
					bitimage_24[2] = (unsigned char)(ks & 255);
					GMT_Report (API, GMT_MSG_INFORMATION, "Transparency color reset from %s to color %d/%d/%d\n",
						gmt_putrgb (GMT, P->bfn[GMT_NAN].rgb), (int)bitimage_24[0], (int)bitimage_24[1], (int)bitimage_24[2]);
					for (k = 0; k < 3; k++) P->bfn[GMT_NAN].rgb[k] = gmt_M_is255 (bitimage_24[k]);	/* Set new NaN color */
				}
			}
			else
				done = true;	/* Original NaN color was never used by other nodes */
		}
		else
			done = true;	/* Only doing the loop once here since no -Q */
	}

	if (rgb_used) gmt_M_free (GMT, rgb_used);	/* Done using the r/g/b cube */
	gmt_M_free (GMT, Conf->actual_row);
	gmt_M_free (GMT, Conf->actual_col);

	if (use_intensity_grid) {	/* Also done with the intensity grid */
		if (need_to_project || !got_z_grid) {
			if (GMT_Destroy_Data (API, &Intens_proj) != GMT_NOERROR)
				GMT_Report (API, GMT_MSG_ERROR, "Failed to free Intens_proj\n");
		}
	}

	/* Get actual plot size of each pixel */
	dx = gmt_M_get_inc (GMT, header_work->wesn[XLO], header_work->wesn[XHI], Conf->n_columns, header_work->registration);
	dy = gmt_M_get_inc (GMT, header_work->wesn[YLO], header_work->wesn[YHI], Conf->n_rows,    header_work->registration);

	/* Set lower left position of image on map; this depends on grid or image registration */

	x0 = header_work->wesn[XLO];	y0 = header_work->wesn[YLO];
	if (grid_registration == GMT_GRID_NODE_REG) {	/* Grid registration, move 1/2 pixel down/left */
		x0 -= 0.5 * dx;
		y0 -= 0.5 * dy;
	}

	/* Full rectangular dimension of the projected image in inches */
	x_side = dx * Conf->n_columns;
	y_side = dy * Conf->n_rows;

	if (P && gray_only && !Ctrl->A.active) /* Determine if the gray image in fact is just black & white since the PostScript can then simplify */
		for (kk = 0, P->is_bw = true; P->is_bw && kk < header_work->nm; kk++)
			if (!(bitimage_8[kk] == 0 || bitimage_8[kk] == 255)) P->is_bw = false;

	if (P && P->is_bw && !Ctrl->A.active)	/* Can get away with a 1-bit image, but we must pack the original byte to 8 image bits, unless we are returning the image (8 or 24 bit only allowed) */
		grdimage_blackwhite_PS_image (GMT, Ctrl, bitimage_8, Conf->n_columns, Conf->n_rows, x0, y0, dx, dy);
	else if (gray_only || Ctrl->M.active) {	/* Here we have a 1-layer 8 bit grayshade image */
		if (Ctrl->A.active) {	/* Creating a raster image, not PostScript */
			GMT_Report (API, GMT_MSG_INFORMATION, "Writing 8-bit grayshade image %s\n", way[Ctrl->A.way]);
			if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->Out.file, Out) != GMT_NOERROR)
				Return (API->error);
		}
		else {	/* Lay down a PostScript 8-bit image */
			GMT_Report (API, GMT_MSG_INFORMATION, "Plotting 8-bit grayshade image\n");
			PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_8, Conf->n_columns, Conf->n_rows, (Ctrl->E.device_dpi ? -8 : 8));
		}
	}
	else {	/* Dealing with a 24-bit color image */
		if (Ctrl->A.active) {	/* Creating a raster image, not PostScript */
			if (Ctrl->Q.active) {	/* Must initialize the transparency byte (alpha): 255 everywhere except at NaNs where it should be 0 */
				memset (Out->alpha, 255, header_work->nm);
				for (node = row = 0; row < (unsigned int)Conf->n_rows; row++) {
					kk = gmt_M_ijpgi (header_work, row, 0);
					for (col = 0; col < (unsigned int)Conf->n_columns; col++, node++) {
						if (gmt_M_is_fnan (Grid_proj->data[kk + col])) Out->alpha[node] = 0;
					}
				}
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "Writing 24-bit color image %s\n", way[Ctrl->A.way]);
			if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->Out.file, Out) != GMT_NOERROR)
				Return (API->error);
		}
		else {	/* Lay down a PostScript 24-bit color image */
			GMT_Report (API, GMT_MSG_INFORMATION, "Plotting 24-bit color image\n");
			PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_24, (Ctrl->Q.active ? -1 : 1) *
		    	Conf->n_columns, Conf->n_rows, (Ctrl->E.device_dpi ? -24 : 24));
		}
	}

	if (!Ctrl->A.active) {	/* Finalize PostScript plot, possibly including basemap */
		if (!Ctrl->N.active) gmt_map_clip_off (GMT);

		gmt_map_basemap (GMT);
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);

		/* Here we are done producing the PostScript or raster image and can free the image arrays we used */

		/* Free both bitimage arrays. gmt_M_free will not complain if they have not been used (NULL) */
		gmt_M_free (GMT, bitimage_8);
		gmt_M_free (GMT, bitimage_24);
	}

	if (need_to_project && got_z_grid && GMT_Destroy_Data (API, &Grid_proj) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free Grid_proj\n");
	}

	if (API->external && Ix && Iy) {	/* Restore old arrays since we got a read-only image */
		gmt_M_free (GMT, I->x);	gmt_M_free (GMT, I->y);
		I->x = Ix;	I->y = Iy;
	}

	if (got_z_grid) {	/* If memory grid was passed in we must restore the header */
		if (mem_G && Grid_orig && header_G) {
			if (GMT->parent->object[0]->alloc_mode != GMT_ALLOC_EXTERNALLY)	/* Because it would call gmt_M_str_free(to->ProjRefWKT) */
				gmt_copy_gridheader (GMT, Grid_orig->header, header_G);
			gmt_free_header (API->GMT, &header_G);
		}
	}
	if (mem_I && Intens_orig && header_I) {	/* If memory grid was passed in we must restore the header */
		gmt_copy_gridheader (GMT, Intens_orig->header, header_I);
		gmt_free_header (API->GMT, &header_I);
	}
	if (mem_D && I && header_D) {	/* If memory image was passed in we must restore the header */
		gmt_copy_gridheader (GMT, I->header, header_D);
		gmt_free_header (API->GMT, &header_D);
	}

	if (Ctrl->D.active) {	/* Free the projected image */
		if (GMT_Destroy_Data (API, &Img_proj) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	if (Ctrl->C.active && GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
		Return (API->error);
	}

	/* May return a flag that the image/PS had no data (see -W), or just NO_ERROR */
	ret_val = (Ctrl->W.active && !has_content) ? GMT_IMAGE_NO_DATA : GMT_NOERROR;
	Return (ret_val);
}
