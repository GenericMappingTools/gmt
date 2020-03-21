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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: grdimage will read one (or 3) grid file and image the area
 * with optional shading. Alternatively, it will read an image and project it
 * using the selected projection.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdimage"
#define THIS_MODULE_MODERN_NAME	"grdimage"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Project and plot grids or images"
#define THIS_MODULE_KEYS	"<G{+,CC(,IG(,>X},>IA,<ID@<G{+,CC(,IG(,AI),<ID"
#define THIS_MODULE_NEEDS	"Jg"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYfnptxy" GMT_OPT("Sc") GMT_ADD_x_OPT

#ifdef HAVE_GDAL
#define N_IMG_EXTENSIONS 6
static char *gdal_ext[N_IMG_EXTENSIONS] = {"tiff", "tif", "gif", "png", "jpg", "bmp"};
#endif

/* Control structure for grdimage */

struct GRDIMAGE_CTRL {
	struct GRDIMG_In {
		bool active;
		bool do_rgb;
		char *file[3];
	} In;
	struct GRDIMG_Out {
		bool active;
		char *file;
	} Out;
	struct GRDIMG_C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...][+i<dz>] */
		bool active;
		double dz;
		char *file;
	} C;
	struct GRDIMG_D {	/* -D to read image instead of grid */
		bool active;
		bool mode;	/* Use info of -R option to reference image */
	} D;
	struct GRDIMG_A {	/* -A to write a raster file or return image to API */
		bool active;
		bool return_image;
		char *file;
	} A;
	struct GRDIMG_E {	/* -Ei|<dpi> */
		bool active;
		bool device_dpi;
		unsigned int dpi;
	} E;
	struct GRDIMG_G {	/* -G<rgb>[+b|f] */
		bool active;
		double rgb[2][4];
	} G;
	struct GRDIMG_I {	/* -I[<intensfile>|<value>|<modifiers>] */
		bool active;
		bool constant;
		bool derive;
		double value;
		char *azimuth;	/* Default azimuth(s) for shading */
		char *file;
		char *method;	/* Default scaling method */
		char *ambient;	/* Default ambient offset */
	} I;
	struct GRDIMG_M {	/* -M */
		bool active;
	} M;
	struct GRDIMG_N {	/* -N */
		bool active;
	} N;
	struct GRDIMG_Q {	/* -Q */
		bool active;
	} Q;
	struct GRDIMG_W {	/* -W */
		bool active;
	} W;
};

#define GRDIMAGE_BGD	0
#define GRDIMAGE_FGD	1

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDIMAGE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDIMAGE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->G.rgb[GRDIMAGE_BGD][0] = C->G.rgb[GRDIMAGE_BGD][1] = C->G.rgb[GRDIMAGE_BGD][2] = 1.0;	/* White background pixel, black foreground defaults  */
	C->I.azimuth = strdup ("-45.0");		/* Default azimuth for shading when -I is used */
	C->I.method  = strdup ("t1");	/* Default normalization for shading when -I is used */
	C->I.ambient  = strdup ("0");	/* Default ambient light for shading when -I is used */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *C) {	/* Deallocate control structure */
	int k;
	if (!C) return;
	for (k = 0; k < 3; k++) gmt_M_str_free (C->In.file[k]);
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->I.azimuth);
	gmt_M_str_free (C->I.ambient);
	gmt_M_str_free (C->I.method);
	gmt_M_str_free (C->Out.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	if (API->external) {	/* External interface */
		GMT_Message (API, GMT_TIME_NONE, "usage: %s <grd_z>|<img>|<grd_r> <grd_g> <grd_b> %s [%s] [-A] [-C<cpt>]\n", name, GMT_J_OPT, GMT_B_OPT);
		GMT_Message (API, GMT_TIME_NONE, "\t[-D[r]] [-Ei|<dpi>] [-G<rgb>[+b|f]] [-I[<intensgrid>|<value>|<modifiers>]] %s[-M] [-N] %s%s[-Q]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	}
	else {
#ifdef HAVE_GDAL
		GMT_Message (API, GMT_TIME_NONE, "usage: %s <grd_z>|<img>|<grd_r> <grd_g> <grd_b> %s [%s] [-A<out_img>[=<driver>]] [-C<cpt>]\n",
		             name, GMT_J_OPT, GMT_B_OPT);
		GMT_Message (API, GMT_TIME_NONE, "\t[-D[r]] [-Ei|<dpi>] [-G<rgb>[+b|f]] [-I[<intensgrid>|<value>|<modifiers]] %s[-M] [-N] %s%s[-Q]\n", API->K_OPT, API->O_OPT, API->P_OPT);
#else
		GMT_Message (API, GMT_TIME_NONE, "usage: %s <grd_z>|<img>|<grd_r> <grd_g> <grd_b> %s [%s] [-C<cpt>] [-Ei[|<dpi>]]\n",
		             name, GMT_J_OPT, GMT_B_OPT);
		GMT_Message (API, GMT_TIME_NONE, "\t[-G[f|b]<rgb>] [-I[<intensgrid>|<value>|<modifiers]] %s[-M] [-N] %s%s[-Q]\n", API->K_OPT, API->O_OPT, API->P_OPT);
#endif
	}
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] %s[%s]\n\t[%s]\n\t[%s] [%s] [%s] [%s]\n\n",
	             GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_f_OPT, GMT_n_OPT, GMT_p_OPT, GMT_t_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grd_z> is data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  converted to rgb colors via the CPT.  Alternatively, give an image or three separate\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  grid files that contain the red, green, and blue components in the 0-255 range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  If the image is plain (e.g., JPG, PNG, GIF) you must also give a corresponding -R.\n");
	if (API->external)	/* External interface */
		GMT_Message (API, GMT_TIME_NONE, "\t  If -D is used then <grd_z> is instead expected to be an image.\n");

	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	if (API->external)	/* External interface */
		GMT_Message (API, GMT_TIME_NONE, "\t-A Return a GMT raster image instead of a PostScript plot.\n");
	else {
#ifdef HAVE_GDAL
		GMT_Message (API, GMT_TIME_NONE, "\t-A Save image in a raster format (.bmp, .gif, .jpg, .png, .tif) instead of PostScript.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   If filename does not have any of these extensions then append =<driver> to select\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   the desired image format. The 'driver' is the driver code name used by GDAL.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   See GDAL documentation for available drivers. Note: any vector elements are lost. \n");
#else
		GMT_Message (API, GMT_TIME_NONE, "\t-A Save image in a PPM format (give .ppm extension) instead of PostScript.\n");
#endif
	}
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert grid values to colors. Optionally, name a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign continuous colors over the data range [%s]; if so,\n", GMT_DEFAULT_CPT_NAME);
	GMT_Message (API, GMT_TIME_NONE, "\t   optionally append +i<dz> to quantize the range [the exact grid range].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Another option is to specify -C<color1>,<color2>[,<color3>,...] to build a\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   linear continuous cpt from those colors automatically.\n");
	if (API->external)	/* External interface */
		GMT_Message (API, GMT_TIME_NONE, "\t-D <img> is an image instead of a grid. Append r to equate image region to -R region.\n");
#ifdef HAVE_GDAL
	else {
		GMT_Message (API, GMT_TIME_NONE, "\t-D GMT automatically detects standard image formats. For non-standard formats you must\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   use -D to force it to be seen as an image. Append r to equate image region with -R region.\n");
	}
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set dpi for the projected grid which must be constructed [100]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   if -Jx or -Jm is not selected [Default gives same size as input grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give i to do the interpolation in PostScript at device resolution (invalid with -Q).\n");
	gmt_rgb_syntax (API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images.\n\t  ");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +b to set background or +f to set foreground color [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Apply directional illumination. Append name of intensity grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a constant intensity (i.e., change the ambient light), append a value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To derive intensities from <grd_z> instead, append +a<azim> [-45], +n<method> [t1], and +m<ambient> [0]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or use -I+d to accept the default values (see grdgradient for details).\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Force monochrome image.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not clip image at the map boundary.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Use PS Level 3 colormasking to make nodes with z = NaN transparent.\n");
	GMT_Option (API, "R");
	GMT_Option (API, "U,V,X,c,f,n,p,t,x,.");

	return (GMT_MODULE_USAGE);
}

EXTERN_MSC int gmtinit_parse_n_option (struct GMT_CTRL *GMT, char *item);
EXTERN_MSC int gmtlib_get_grdtype (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_GRID_HEADER *h);

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDIMAGE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdimage and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, ind, off, k;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	size_t n;
	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one or three is accepted) */
				Ctrl->In.active = true;
				if (n_files >= 3) break;
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else
					n_errors++;
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
				Ctrl->A.active = true;
				if (API->external) {	/* External interface only */
					if ((n = strlen (opt->arg)) > 0) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: No output argument allowed\n");
						n_errors++;
					}
					Ctrl->A.return_image = true;
				}
				else if ((n = strlen (opt->arg)) == 0) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -A: No output name provided\n");
					n_errors++;
				}
				else if (!strcmp (gmt_get_ext (opt->arg), "ppm")) {	/* Want a ppm image which we can do without GDAL */
					Ctrl->A.file = strdup (opt->arg);
				}
#ifdef HAVE_GDAL
				else {	/* Must give file and GDAL driver and this requires GDAL support */
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
#else
				else
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Your selection requires building GMT with GDAL support.\n");
#endif
				break;

			case 'C':	/* CPT */
				Ctrl->C.active = true;
				if ((c = strstr (opt->arg, "+i"))) {	/* Gave auto-interval */
					Ctrl->C.dz = atof (&c[2]);
					c[0] = '\0';	/* Temporarily chop off the modifier */
				}
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore */
				break;
#ifdef HAVE_GDAL
			case 'D':	/* Get an image via GDAL */
				if (!API->external)			/* For externals we actually still need the -D */
					GMT_Report (API, GMT_MSG_COMPAT,
					            "Option -D is deprecated; images are detected automatically\n");
				Ctrl->D.active = true;
				Ctrl->D.mode = (opt->arg[0] == 'r');
				break;
#endif
			case 'E':	/* Sets dpi */
				Ctrl->E.active = true;
				if (opt->arg[0] == 'i')	/* Interpolate image to device resolution */
					Ctrl->E.device_dpi = true;
				else if (opt->arg[0] == '\0')
					Ctrl->E.dpi = 100;	/* Default grid dpi */
				else
					Ctrl->E.dpi = atoi (opt->arg);
				break;
			case 'G':	/* -G<color>[+b|f] 1-bit fore- or background color for transparent masks (was -G[f|b]<color>) */
				Ctrl->G.active = true;
				if ((c = strstr (opt->arg, "+b"))) {	/* Background color */
					ind = GRDIMAGE_BGD;	off = GRDIMAGE_FGD;	k = 0;	c[0] = '\0';
				}
				else if ((c = strstr (opt->arg, "+f"))) {	/* Foreground color */
					ind = GRDIMAGE_FGD;	off = GRDIMAGE_BGD;	k = 0;	c[0] = '\0';
				}
				else if (gmt_M_compat_check (GMT, 5) && strchr ("bf", opt->arg[0])) {	/* No modifier given so heck if first character is f or b */
					k = 1;
					if (opt->arg[0] == 'b') ind = GRDIMAGE_BGD, off = GRDIMAGE_FGD;
					else ind = GRDIMAGE_FGD, off = GRDIMAGE_BGD;
				}
				else {	/* Modern syntax where missing modifier means +f and just color is given */
					ind = GRDIMAGE_FGD;	off = GRDIMAGE_BGD;	k = 0;
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
				Ctrl->I.active = true;
				if (!strcmp (opt->arg, "+d"))	/* Gave +d only, so derive intensities from input grid using default settings */
					Ctrl->I.derive = true;
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
				else if (!opt->arg[0] || strstr (opt->arg, "+"))	/* No argument or just +, so derive intensities from input grid using default settings */
					Ctrl->I.derive = true;	/* Need to have -I+ since otherwise, -I in mex would try to attach a grid */
				else if (!gmt_access (GMT, opt->arg, R_OK))	/* Got a file */
					Ctrl->I.file = strdup (opt->arg);
				else if (gmt_M_file_is_cache (opt->arg))	/* Got a remote file */
					Ctrl->I.file = strdup (opt->arg);
				else if (opt->arg[0] && gmt_is_float (GMT, opt->arg)) {	/* Looks like a constant value */
					Ctrl->I.value = atof (opt->arg);
					Ctrl->I.constant = true;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
				break;
			case 'M':	/* Monochrome image */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not clip at map boundary */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* PS3 colormasking */
				Ctrl->Q.active = true;
				break;
			case 'W':	/* Warn if no image */
				Ctrl->W.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	if (!GMT->common.n.active && (!Ctrl->C.active || gmt_is_cpt_master (GMT, Ctrl->C.file)))
		/* Unless user selected -n we want the default not to exceed data range on projection when we are auto-scaling a master table */
		n_errors += gmtinit_parse_n_option (GMT, "b+c");

	if (n_files == 3) Ctrl->In.do_rgb = true;
	if (Ctrl->D.active) {	/* Only OK with memory input or GDAL support */
		if (!gmt_M_file_is_memory (Ctrl->In.file[0])) {
#ifndef HAVE_GDAL
			GMT_Report (API, GMT_MSG_ERROR, "Option -D: Requires building GMT with GDAL support.\n");
			n_errors++;
#endif
		}
	}
	if (!API->external) {	/* I.e, not an External interface */
		n_errors += gmt_M_check_condition (GMT, !(n_files == 1 || n_files == 3),
		                                   "Must specify one (or three) input file(s)\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->I.constant && !Ctrl->I.file && !Ctrl->I.derive,
	                                   "Option -I: Must specify intensity file, value, or modifiers\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.derive && n_files == 3,
	                                   "Option -I: Cannot derive intensities when r,g,b grids are given as data\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.derive && Ctrl->D.active,
	                                   "Option -I: Cannot derive intensities when an image is given as data\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0,
	                                   "Option -E: dpi must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.rgb[GRDIMAGE_FGD][0] < 0 && Ctrl->G.rgb[GRDIMAGE_BGD][0] < 0,
	                                   "Option -G: Only one of fore/back-ground can be transparent for 1-bit images\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && Ctrl->Q.active,
	                                   "SOption -Q: Cannot use -M when doing colormasking\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.device_dpi && Ctrl->Q.active,
	                                   "Option -Q: Cannot use -Ei when doing colormasking\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->Q.active,
	                                   "Option -Q: Cannot use -D when doing colormasking\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.return_image && Ctrl->Out.file == NULL,
	                                   "Option -A: Must provide an output filename for image\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.file && Ctrl->Out.file,
								       "Option -A, -> options: Cannot provide two output files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active,
								       "Options -C and -D options are mutually exclusive\n");
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int clean_global_headers (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* Problem is that many global grids are reported with misleading -R or imprecise
	 * -R or non-global -R yet nx * xinc = 360, etc.  We fix these here for GMT use. */
	unsigned int flag = 0;
	double wasR[4], wasI[2];
	if (gmt_M_x_is_lon (GMT, GMT_IN)) {
		if (gmt_M_360_range (h->wesn[XLO], h->wesn[XHI]))
			flag = 0;
		else if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < GMT_CONV4_LIMIT)
			flag |= 1;	/* Sloppy, we can improve this a bit */
		else if (fabs (h->n_columns * h->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
			/* If n*xinc = 360 and previous test failed then we do not have a repeat node.  These are pixel grids */
			flag |= 2;	/* Global but flawed -R and registration */
		}
	}
	if (gmt_M_y_is_lat (GMT, GMT_IN)) {
		if (gmt_M_180_range (h->wesn[YLO], h->wesn[YHI]))
			flag |= 0;
		else if (fabs (h->wesn[YHI] - h->wesn[YLO] - 180.0) < GMT_CONV4_LIMIT)
			flag |= 4;	/* Sloppy, we can improve this a bit */
		else if (fabs (h->n_rows * h->inc[GMT_Y] - 180.0) < GMT_CONV4_LIMIT)
			flag |= 8;	/* Global but flawed -R and registration */
	}
	if (!(flag == 9 || flag == 6)) return 0;	/* Only deal with mixed cases of gridline and pixel reg in lon vs lat */
	gmt_M_memcpy (wasR, h->wesn, 4, double);
	gmt_M_memcpy (wasI, h->inc, 2, double);
	if ((flag & 1)  && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_X] = 360.0 / (h->n_columns - 1);	/* Get exact increment */
		h->wesn[XHI] = h->wesn[XLO] + 360.0;
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Detected geographic global gridline-registered grid with sloppy longitude bounds.\n");
	}
	if ((flag & 2)  && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_X] = 360.0 / h->n_columns;	/* Get exact increment */
		h->wesn[XHI] = h->wesn[XLO] + h->inc[GMT_X] * (h->n_columns - 1);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Detected geographic global pixel-registered grid posing as gridline-registered with sloppy longitude bounds.\n");
	}
	if ((flag & 4) && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_Y] = 180.0 / (h->n_rows - 1);	/* Get exact increment */
		h->wesn[YLO] = -90.0;
		h->wesn[YHI] = 90.0;
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Detected geographic global gridline-registered grid with gridline latitude bounds.\n");
	}
	if ((flag & 8) && h->registration == GMT_GRID_NODE_REG) {	/* This grid needs to be treated as a pixel grid */
		h->inc[GMT_Y] = 180.0 / h->n_rows;	/* Get exact increment */
		h->wesn[YLO] = -90.0 + 0.5 * h->inc[GMT_Y];
		h->wesn[YHI] = h->wesn[YLO] +  h->inc[GMT_Y] * (h->n_rows - 1);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Detected geographic global pixel-registered grid posing as gridline-registered with sloppy latitude bounds.\n");
	}
	if (flag) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Old region: %g/%g/%g/%g with incs %g/%g\n", wasR[XLO], wasR[XHI], wasR[YLO], wasR[YHI], wasI[GMT_X], wasI[GMT_Y]);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "New region: %g/%g/%g/%g with incs %g/%g\n", h->wesn[XLO], h->wesn[XHI], h->wesn[YLO], h->wesn[YHI], h->inc[GMT_X], h->inc[GMT_Y]);
	}
	return 1;
}

GMT_LOCAL void GMT_set_proj_limits (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *r, struct GMT_GRID_HEADER *g, bool projected, unsigned int mixed) {
	/* Sets the projected extent of the grid given the map projection
	 * The extreme x/y coordinates are returned in r, and dx/dy, and
	 * n_columns/n_rows are set accordingly.  Not that some of these may change
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
	}
}

GMT_LOCAL int set_rgb_three_grids (struct GMT_GRID *Grid_proj[], uint64_t node, double *NaN_rgb, double *rgb) {
	/* Got three grids with red, green, blue values */
	int k;
	for (k = 0; k < 3; k++) {
		if (gmt_M_is_fnan (Grid_proj[k]->data[node])) {	/* If one is NaN they are all assumed to be NaN */
			gmt_M_rgb_copy (rgb, NaN_rgb);
			return (GMT_NAN - 3);	/* Ensure no illumination will take place later for this pixel */
		}
		else {	/* Set color */
			rgb[k] = gmt_M_is255 (Grid_proj[k]->data[node]);
			if (rgb[k] < 0.0) rgb[k] = 0.0; else if (rgb[k] > 1.0) rgb[k] = 1.0;	/* Clip */
		}
	}
	return 0;	/* OK to add illumination later */
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int gmtlib_read_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);
EXTERN_MSC bool gmtlib_file_is_srtmlist (struct GMTAPI_CTRL *API, const char *file);

#define img_inc_is_one(h) (h->inc[GMT_X] == 1.0 && h->inc[GMT_Y] == 1.0)
#define img_region_is_dimension(h) (h->wesn[XLO] == 0.0 && h->wesn[YLO] == 0.0 && img_inc_is_one(h) && \
                                    urint (h->wesn[XHI]) == h->n_columns && urint (h->wesn[YHI]) == h->n_rows)
#define img_region_is_invalid(h) (h->wesn[XLO] == 0.0 && h->wesn[YLO] == 0.0 && img_inc_is_one(h) && \
                                  (h->wesn[YHI] > 90.0 || h->wesn[XHI] > 720.0))

int GMT_grdimage (void *V_API, int mode, void *args) {
	bool done, need_to_project, normal_x, normal_y, resampled = false, gray_only = false;
	bool nothing_inside = false, use_intensity_grid = false, got_data_grid = false, set_gray, rgb_from_z, rgb_cube_scan;
	bool do_indexed = false, has_content = false, mem_G[3] = {false, false, false}, mem_I = false, mem_D = false;
	unsigned int n_columns = 0, n_rows = 0, grid_registration = GMT_GRID_NODE_REG, n_grids, intensity_mode;
	unsigned int colormask_offset = 0, try, row, col, mixed = 0, *actual_row = NULL, *actual_col = NULL;
	uint64_t node_RGBA = 0;             /* uint64_t for the RGB(A) image array. */
	uint64_t node, k, kk, byte, step, dim[GMT_DIM_SIZE] = {0, 0, 3, 0};
	int index = 0, ks, error = 0, ret_val = GMT_NOERROR, ftype = GMT_NOTSET;

	char   *img_ProjectionRefPROJ4 = NULL, *way[2] = {"via GDAL", "directly"}, cmd[GMT_LEN256] = {""}, data_grd[GMT_VF_LEN] = {""};
	unsigned char *bitimage_8 = NULL, *bitimage_24 = NULL, *rgb_used = NULL, i_rgb[3];

	double  dx, dy, x_side, y_side, x0 = 0.0, y0 = 0.0, rgb[4] = {0.0, 0.0, 0.0, 0.0};
	double	img_wesn[4], img_inc[2] = {1.0, 1.0};    /* Image increments & min/max for writing images or external interfaces */
	double *NaN_rgb = NULL, red[4] = {1.0, 0.0, 0.0, 0.0}, wesn[4] = {0.0, 0.0, 0.0, 0.0};
	double *Ix = NULL, *Iy = NULL;

	struct GMT_GRID *Grid_orig[3] = {NULL, NULL, NULL}, *Grid_proj[3] = {NULL, NULL, NULL};
	struct GMT_GRID *Intens_orig = NULL, *Intens_proj = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL, *IH = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRDIMAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;        /* General PSL internal parameters */
	struct GMT_GRID_HEADER *header_work = NULL;	/* Pointer to a GMT header for the image or grid */
	struct GMT_GRID_HEADER *header_D = NULL, *header_I = NULL, *header_G[3] = {NULL, NULL, NULL};
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	double *r_table = NULL, *g_table = NULL, *b_table = NULL;
	struct GMT_IMAGE *I = NULL, *Img_proj = NULL;		/* A GMT image datatype, if GDAL is used */
	struct GMT_IMAGE *Out = NULL;       /* A GMT image datatype, if external interface is used with -A */
	struct GMT_GRID *G2 = NULL;

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

	use_intensity_grid = (Ctrl->I.active && !Ctrl->I.constant);	/* We want to use an intensity grid */
	n_grids = (Ctrl->In.do_rgb) ? 3 : 1;	/* Either reading 3 grids (r, g, b) or a z-data grid */
	if (Ctrl->A.file) {Ctrl->Out.file = Ctrl->A.file; Ctrl->A.file = NULL;}	/* Only use Out.file for writing */

	/* Read the illumination grid header right away so we can use its region to set that of an image (if requested) */
	if (use_intensity_grid) {	/* Illumination grid desired */
		if (Ctrl->I.derive) {	/* Illumination grid must be derived */
			/* If input grid is actually a list of SRTM tiles then we must read that list first since this will
			 * force all tiles to be downloaded, converted, and stitched into a single grid per -R. This must
			 * happen _before_ we auto-derive intensities via grdgradient so that there is an input data grid */
			if (gmtlib_file_is_srtmlist (API, Ctrl->In.file[0])) {	/* Must read and stitch the tiles first */
				if ((Grid_orig[0] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, GMT->common.R.wesn, Ctrl->In.file[0], NULL)) == NULL)	/* Get srtm grid data */
					Return (API->error);
				if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, Grid_orig[0], data_grd))
					Return (API->error);
				got_data_grid = true;
			}
		}
		else {	/* Illumination grid present and can be read */
			mem_I = gmt_M_file_is_memory (Ctrl->I.file);
			GMT_Report (API, GMT_MSG_INFORMATION, "Read intensity grid header from file %s\n", Ctrl->I.file);
			if ((Intens_orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->I.file, NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
		}
	}

#ifdef HAVE_GDAL
	if (!Ctrl->D.active && (ftype = gmt_raster_type (GMT, Ctrl->In.file[0])) == GMT_IS_IMAGE) {
		/* The input file is an ordinary image instead of a grid and -R may be required to use it */
		Ctrl->D.active = true;
		if (GMT->common.R.active[RSET]) Ctrl->D.mode = true;
	}

	if (!Ctrl->D.active && ftype == GMT_IS_GRID) {	/* See if input could be an image of a kind that could also be a grid and we don't yet know what it is.  Pass GMT_GRID_IS_IMAGE mode */
		if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY | GMT_GRID_IS_IMAGE, NULL, Ctrl->In.file[0], NULL)) != NULL) {
			gmtlib_read_grd_info (GMT, Ctrl->In.file[0], I->header);	/* Re-read header as grid to ensure orig_datatype is set*/
			HH = gmt_get_H_hidden (I->header);	/* Get hidden structure */
			if (HH->orig_datatype == GMT_UCHAR || HH->orig_datatype == GMT_CHAR) Ctrl->D.active = true;
			/* Guess that if the image region goes from 0 to col/rol-dimensions then we want -Dr */
			if (Ctrl->D.active && GMT->common.R.active[RSET]) {	/* Should we add -Dr or is it just a valid subset specification? */
				if (gmt_M_is_geographic (GMT, GMT_IN) && img_region_is_invalid (I->header)) Ctrl->D.mode = true;
				else if (img_region_is_dimension (I->header)) Ctrl->D.mode = true;
			}
		}
	}
#endif

	if (Ctrl->D.active) {	/* Main input is a single image and not a grid */

		if (use_intensity_grid && GMT->common.R.active[RSET]) {
            double xnoise = Intens_orig->header->inc[GMT_X]*GMT_CONV4_LIMIT, ynoise = Intens_orig->header->inc[GMT_Y]*GMT_CONV4_LIMIT;
			if (GMT->common.R.wesn[XLO] < (Intens_orig->header->wesn[XLO]-xnoise) || GMT->common.R.wesn[XHI] > (Intens_orig->header->wesn[XHI]+xnoise) ||
			    GMT->common.R.wesn[YLO] < (Intens_orig->header->wesn[YLO]-ynoise) || GMT->common.R.wesn[YHI] > (Intens_orig->header->wesn[YHI]+ynoise)) {
				GMT_Report (API, GMT_MSG_ERROR, "Requested region exceeds illumination extent\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}

		if (!Ctrl->D.mode && use_intensity_grid && !GMT->common.R.active[RSET])	/* Apply illumination to an image but no -R provided; use intensity domain */
			gmt_M_memcpy (GMT->common.R.wesn, Intens_orig->header->wesn, 4, double);

		/* Read in the the entire image that is to be mapped */
		GMT_Report (API, GMT_MSG_INFORMATION, "Allocate memory and read image file %s\n", Ctrl->In.file[0]);
		if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file[0], NULL)) == NULL) {
			Return (API->error);
		}
		mixed = clean_global_headers (GMT, I->header);
		HH = gmt_get_H_hidden (I->header);
		if ((I->header->n_bands > 1 && strncmp (I->header->mem_layout, "BRP", 3)) || strncmp (I->header->mem_layout, "BR", 2))
			GMT_Report(API, GMT_MSG_WARNING, "The image memory layout (%s) is of a wrong type. It should be BRPa.\n", I->header->mem_layout);

		if (!Ctrl->D.mode && !Ctrl->I.active && !GMT->common.R.active[RSET])	/* No -R or -I. Use image dimensions as -R */
			gmt_M_memcpy (GMT->common.R.wesn, I->header->wesn, 4, double);

		if ( (Ctrl->D.mode && GMT->common.R.active[RSET]) || (!Ctrl->D.mode && use_intensity_grid) ) {
			gmt_M_memcpy (I->header->wesn, GMT->common.R.wesn, 4, double);
			/* Get actual size of each pixel */
			dx = gmt_M_get_inc (GMT, I->header->wesn[XLO], I->header->wesn[XHI], I->header->n_columns, I->header->registration);
			dy = gmt_M_get_inc (GMT, I->header->wesn[YLO], I->header->wesn[YHI], I->header->n_rows, I->header->registration);
			I->header->inc[GMT_X] = dx;	I->header->inc[GMT_Y] = dy;
			HH->r_inc[GMT_X] = 1.0 / dx;	/* Get inverse increments to avoid divisions later */
			HH->r_inc[GMT_Y] = 1.0 / dy;
			HH->grdtype = gmtlib_get_grdtype (GMT, GMT_IN, I->header);	/* Since we now have a proper region */
			if (API->external) {	/* Cannot free and update read-only image so just juggle new and old arrays */
				Ix = I->x;	Iy = I->y;	/* Keep old arrays */
			}
			else {	/* Reset the grid x/y arrays */
				gmt_M_free (GMT, I->x);	gmt_M_free (GMT, I->y);
			}
			I->x = gmt_grd_coord (GMT, I->header, GMT_X);
			I->y = gmt_grd_coord (GMT, I->header, GMT_Y);
		}

		Ctrl->In.do_rgb = (I->header->n_bands >= 3);
		n_grids = 0;	/* Flag that we are using a GMT_IMAGE instead of a GMT_GRID */

		if (I->header->ProjRefPROJ4 != NULL)
			GMT_Report (API, GMT_MSG_INFORMATION, "Data projection (Proj4 type)\n\t%s\n", I->header->ProjRefPROJ4);

		header_work = I->header;	/* OK, that's what what we'll use to send to gmt_grd_setregion */
	}

	if (!Ctrl->D.active) {	/* Read the headers of 1 or 3 grids */
		for (k = 0; k < n_grids; k++) {
			mem_G[k] = gmt_M_file_is_memory (Ctrl->In.file[k]);
			if (got_data_grid && k == 0) continue;	/* Only true if we already read a SRTM tile bunch earlier under I.derive = true */
			GMT_Report (API, GMT_MSG_INFORMATION, "Read header from file %s\n", Ctrl->In.file[k]);
			if ((Grid_orig[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			if ((API->error = gmt_img_sanitycheck (GMT, Grid_orig[k]->header))) {	/* Used map projection on a Mercator (cartesian) grid */
				Return (API->error);
			}
		}
		if (!GMT->common.J.active) {
			if ((Grid_orig[0]->header->ProjRefWKT != NULL) || (Grid_orig[0]->header->ProjRefPROJ4 != NULL)) {
				gmt_parse_common_options (GMT, "J", 'J', "X15c");	/* No projection specified, use fake linear */
				GMT->common.J.active = true;
			}
			else if (GMT->current.setting.run_mode == GMT_CLASSIC) {
				GMT_Report (API, GMT_MSG_ERROR, "Must specify a map projection with the -J option\n");
				Return (GMT_PARSE_ERROR);
			}
		}
		if (!Ctrl->C.active)
			Ctrl->C.active = true;	/* Use default CPT (GMT_DEFAULT_CPT_NAME) and autostretch or under modern reuse current CPT */
	}

	if (n_grids) header_work = Grid_orig[0]->header;	/* OK, we are in GRID mode and this was not set previuosly. Do it now */

	if (n_grids && Ctrl->In.do_rgb) {	/* Must ensure all three grids are coregistered */
		if (!gmt_M_grd_same_region (GMT, Grid_orig[0], Grid_orig[1])) error++;
		if (!gmt_M_grd_same_region (GMT, Grid_orig[0], Grid_orig[2])) error++;
		if (!(Grid_orig[0]->header->inc[GMT_X] == Grid_orig[1]->header->inc[GMT_X] && Grid_orig[0]->header->inc[GMT_X] ==
			Grid_orig[2]->header->inc[GMT_X])) error++;
		if (!(Grid_orig[0]->header->n_columns == Grid_orig[1]->header->n_columns && Grid_orig[0]->header->n_columns == Grid_orig[2]->header->n_columns)) error++;
		if (!(Grid_orig[0]->header->n_rows == Grid_orig[1]->header->n_rows && Grid_orig[0]->header->n_rows == Grid_orig[2]->header->n_rows)) error++;
		if (!(Grid_orig[0]->header->registration == Grid_orig[1]->header->registration && Grid_orig[0]->header->registration ==
			Grid_orig[2]->header->registration)) error++;
		if (error) {
			GMT_Report (API, GMT_MSG_ERROR, "The r, g, and b grids are not coregistered\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active[RSET] && n_grids)	/* -R was not set so we use the grid domain */
		gmt_set_R_from_grd (GMT, Grid_orig[0]->header);

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	/* Determine if grid/image is to be projected */

	need_to_project = (gmt_M_is_nonlinear_graticule (GMT) || Ctrl->E.dpi > 0);
	if (need_to_project) GMT_Report (API, GMT_MSG_DEBUG, "Projected grid is non-orthogonal, nonlinear, or dpi was changed\n");

	/* Determine the wesn to be used to read the grid file; or bail if file is outside -R */

	if (!gmt_grd_setregion (GMT, header_work, wesn, need_to_project * GMT->common.n.interpolant))
		nothing_inside = true;
	else if (use_intensity_grid && !Ctrl->I.derive && !gmt_grd_setregion (GMT, Intens_orig->header, wesn, need_to_project * GMT->common.n.interpolant))
		nothing_inside = true;

	if (nothing_inside) {	/* Create a blank plot (or image) */
		GMT_Report (API, GMT_MSG_WARNING, "No grid or image inside plot domain\n");
		/* No grid to plot; just do an empty map and bail */
		/* MISSING: Action to take if -A is in effect.  Need to create an empty image and return/save it */
		if (Ctrl->A.active) {	/* Create an empty image of the right dimensions */
		}
		else {
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		Return (GMT_NOERROR);
	}

	if (Ctrl->W.active && n_grids == 1 && gmt_M_is_dnan (header_work->z_min))
		ret_val = GMT_IMAGE_NO_DATA;	/* Flag that our output image has no information*/

	/* Here the grid/image is inside the plot domain.  The same must be true of any
	 * auto-derived intensities we may create below */

	if (Ctrl->I.derive) {	/* Auto-create intensity grid from data grid using the now determined data region */
		char int_grd[GMT_VF_LEN] = {""};
		GMT_Report (API, GMT_MSG_INFORMATION, "Derive intensity grid from data grid\n");
		/* Create a virtual file to hold the intensity grid */
		if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, int_grd))
			Return (API->error);
		/* Prepare the grdgradient arguments using selected -A -N and the data region in effect */
		sprintf (cmd, "-G%s -A%s -N%s+a%s -R%.16g/%.16g/%.16g/%.16g --GMT_HISTORY=false ",
			int_grd, Ctrl->I.azimuth, Ctrl->I.method, Ctrl->I.ambient, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
		if (got_data_grid)	/* Use the virtual file we made earlier */
			strcat (cmd, data_grd);
		else
			strcat (cmd, Ctrl->In.file[0]);
		/* Call the grdgradient module */
		GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdgradient with args %s\n", cmd);
		if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd))
			Return (API->error);
		/* Obtain the data from the virtual file */
		if ((Intens_orig = GMT_Read_VirtualFile (API, int_grd)) == NULL)
			Return (API->error);
		if (got_data_grid)
			GMT_Close_VirtualFile (API, data_grd);
	}

	if (n_grids) {	/* Get grid dimensions */
		n_columns = gmt_M_get_n (GMT, wesn[XLO], wesn[XHI], Grid_orig[0]->header->inc[GMT_X], Grid_orig[0]->header->registration);
		n_rows = gmt_M_get_n (GMT, wesn[YLO], wesn[YHI], Grid_orig[0]->header->inc[GMT_Y], Grid_orig[0]->header->registration);
	}

	if (Ctrl->D.active) {	/* Trust the info from gdal to make it more stable against pixel vs grid registration troubles */
		n_columns = I->header->n_columns;
		n_rows = I->header->n_rows;
	}

	if (!Ctrl->A.active) {	/* Otherwise we are not writing any postscript */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	}

	/* Read the grid data, possibly via subset in wesn */

	for (k = 0; k < n_grids; k++) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Allocate and read data from file %s\n", Ctrl->In.file[k]);
		if (got_data_grid && k == 0) continue;	/* Only true if we already read a SRTM tile bunch earlier under I.derive = true */
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file[k], Grid_orig[k]) == NULL) {	/* Get grid data */
			Return (API->error);
		}
		mixed = clean_global_headers (GMT, Grid_orig[k]->header);
	}


	/* If given, get intensity grid or compute intensities (for a constant intensity) */

	if (use_intensity_grid) {	/* Illumination wanted */

		GMT_Report (API, GMT_MSG_INFORMATION, "Allocates memory and read intensity file\n");

		/* Remember, the illumination header was already read at the start of grdimage */
		if (!Ctrl->I.derive && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->I.file, Intens_orig) == NULL) {
			Return (API->error);	/* Failed to read the intensity grid data */
		}
		mixed = clean_global_headers (GMT, Intens_orig->header);
		if (n_grids && (Intens_orig->header->n_columns != Grid_orig[0]->header->n_columns ||
		                Intens_orig->header->n_rows != Grid_orig[0]->header->n_rows)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimensions of intensity grid do not match that of the data grid!\n");
			Return (GMT_RUNTIME_ERROR);
		}

		if (Ctrl->D.active && (I->header->n_columns != Intens_orig->header->n_columns || I->header->n_rows != Intens_orig->header->n_rows)) {
			/* Resize illumination grid to match the dimensions of the image */

			char in_string[GMT_VF_LEN] = {""}, out_string[GMT_VF_LEN] = {""};
    		/* Associate the intensity grid with an open virtual file - in_string will then hold the name of this input "file" */
    		GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, Intens_orig, in_string);
   			/* Create a virtual file to hold the resampled grid - out_string then holds the name of this output "file" */
    		GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, out_string);
			/* Create the command to do the resampling via the grdsample module */
			sprintf (cmd, "%s -G%s -I%d+/%d+ --GMT_HISTORY=false", in_string, out_string, n_columns, n_rows);
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

	if (need_to_project) {	/* Need to resample the grd file using the specified map projection */
		int nx_proj = 0, ny_proj = 0;
		double inc[2] = {0.0, 0.0};
		GMT_Report (API, GMT_MSG_INFORMATION, "project grid files\n");

		if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
			nx_proj = n_columns;
			ny_proj = n_rows;
		}
		if (Ctrl->D.active) { /* Must project the input image instead */
			if ((Img_proj = GMT_Duplicate_Data (API, GMT_IS_IMAGE, GMT_DUPLICATE_NONE, I)) == NULL) Return (API->error);	/* Just to get a header we can change */
			grid_registration = GMT_GRID_PIXEL_REG;	/* Force pixel */
			GMT_set_proj_limits (GMT, Img_proj->header, I->header, need_to_project, mixed);
			gmt_M_err_fail (GMT, gmt_project_init (GMT, Img_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			                Ctrl->In.file[0]);
			if (Ctrl->A.active) /* Need to set background color to white for raster images */
				for (k = 0; k < 3; k++) GMT->current.setting.color_patch[GMT_NAN][k] = 1.0;	/* For img GDAL write use white as bg color */
			gmt_set_grddim (GMT, Img_proj->header);	/* Recalculate projected image dimensions */
			if (GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Img_proj) == NULL)
				Return (API->error);	/* Failed to allocate memory for the projected image */
			gmt_img_project (GMT, I, Img_proj, false);	/* Now project the image onto the projected rectangle */
			if (!API->external && (GMT_Destroy_Data (API, &I) != GMT_NOERROR)) {	/* Free the original image now we have projected.  Use Img_proj from now on */
				Return (API->error);	/* Failed to free the image */
			}
		}
		for (k = 0; k < n_grids; k++) {	/* Project the 1 or 3 grids */
			if ((Grid_proj[k] = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Grid_orig[k])) == NULL)
				Return (API->error);	/* Just to get a header we can change */
			/* Determine the dimensions of the projected grid */
			GMT_set_proj_limits (GMT, Grid_proj[k]->header, Grid_orig[k]->header, need_to_project, mixed);
			if (grid_registration == GMT_GRID_NODE_REG)		/* Force pixel if a dpi was specified, else keep as is */
				grid_registration = (Ctrl->E.dpi > 0) ? GMT_GRID_PIXEL_REG : Grid_orig[k]->header->registration;
			gmt_M_err_fail (GMT, gmt_project_init (GMT, Grid_proj[k]->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			                Ctrl->In.file[k]);
			gmt_set_grddim (GMT, Grid_proj[k]->header);	/* Recalculate projected grid dimensions */
			if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid_proj[k]) == NULL)
				Return (API->error);	/* Failed to allocate memory for the projected grid */
			gmt_grd_project (GMT, Grid_orig[k], Grid_proj[k], false);	/* Now project the grid onto the projected rectangle */
			if (GMT_Destroy_Data (API, &Grid_orig[k]) != GMT_NOERROR) {	/* Free the original grid now we have projected.  Use Grid_proj from now on */
				Return (API->error);	/* Failed to free the original grid */
			}
		}
		if (use_intensity_grid) {	/* Must also project the intensity grid */
			if ((Intens_proj = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Intens_orig)) == NULL)	/* Just to get a header we can change */
				Return (API->error);
			if (n_grids)	/* Use projected grid bounds as template */
				gmt_M_memcpy (Intens_proj->header->wesn, Grid_proj[0]->header->wesn, 4, double);
			else	/* Use projected image bounds as template */
				gmt_M_memcpy (Intens_proj->header->wesn, Img_proj->header->wesn, 4, double);

			if (Ctrl->E.dpi == 0) {	/* Use input # of nodes as # of projected nodes */
				nx_proj = Intens_orig->header->n_columns;
				ny_proj = Intens_orig->header->n_rows;
			}
			gmt_M_err_fail (GMT, gmt_project_init (GMT, Intens_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			                Ctrl->I.file);
			gmt_set_grddim (GMT, Intens_proj->header);	/* Recalculate projected intensity grid dimensions */
			if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Intens_proj) == NULL)
				Return (API->error);	/* Failed to allocate memory for the projected intensity grid */
			gmt_grd_project (GMT, Intens_orig, Intens_proj, false);	/* Now project the intensity grid onto the projected rectangle */
			if (GMT_Destroy_Data (API, &Intens_orig) != GMT_NOERROR) {	/* Free the original intensity grid now we have projected.  Use Intens_proj from now on */
				Return (API->error);	/* Failed to free the original intensity grid */
			}
		}
		resampled = true;	/* Yes, we did it */
	}
	else {	/* Simply set the unused Grid_proj[i]/Intens_proj pointers to point to original unprojected Grid_orig[i]/Intens_orig objects */
		struct GMT_GRID_HEADER *tmp_header = gmt_get_header (GMT);
		for (k = 0; k < n_grids; k++) {	/* Must get a copy of the header so we can change one without affecting the other */
			gmt_copy_gridheader (GMT, tmp_header, Grid_orig[k]->header);
			if (mem_G[k]) {
				header_G[k] = gmt_get_header (GMT);
				gmt_copy_gridheader (GMT, header_G[k], Grid_orig[k]->header);
			}
			Grid_proj[k] = Grid_orig[k];
			GMT_set_proj_limits (GMT, Grid_proj[k]->header, tmp_header, need_to_project, mixed);
		}
		if (use_intensity_grid) {
			if (mem_I) {
				header_I = gmt_get_header (GMT);
				gmt_copy_gridheader (GMT, header_I, Intens_orig->header);
			}
			Intens_proj = Intens_orig;
		}
		if (n_grids) /* Dealing with 1 or 3 projected grids */
			grid_registration = Grid_orig[0]->header->registration;
		else {	/* Dealing with a projected image */
			gmt_copy_gridheader (GMT, tmp_header, I->header);
			if (mem_D) {
				header_D = gmt_get_header (GMT);
				gmt_copy_gridheader (GMT, header_D, I->header);
			}
			Img_proj = I;
			GMT_set_proj_limits (GMT, Img_proj->header, tmp_header, need_to_project, mixed);
		}
		gmt_free_header (API->GMT, &tmp_header);
	}

	/* From here, use Grid_proj or Img_proj plus optional Intens_proj in making the Cartesian image */

	if (use_intensity_grid) IH = gmt_get_H_hidden (Intens_proj->header);
	if (n_grids) { /* Dealing with 1 or 3 projected grids, only one band */
		Grid_proj[0]->header->n_bands = 1;
		header_work = Grid_proj[0]->header;     /* Later when need to refer to the header, use this copy */
	}
	if (Ctrl->D.active) /* Use a different reference header for the image */
		header_work = Img_proj->header;         /* Later when need to refer to the header, use this copy */

	n_columns = header_work->n_columns;	/* To simplify code below */
	n_rows    = header_work->n_rows;

	/* Get or calculate a color palette file */

	if (!Ctrl->In.do_rgb) {	/* Got a single grid so need to convert z to color via a CPT */
		if (Ctrl->C.active) {		/* Read a palette file */
			unsigned int zmode = gmt_cpt_default (GMT, header_work);
			if ((P = gmt_get_palette (GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, header_work->z_min, header_work->z_max, Ctrl->C.dz, zmode)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to read CPT %s.\n", Ctrl->C.file);
				Return (API->error);	/* Well, that did not go well... */
			}
			gray_only = (P && P->is_gray);	/* Flag that we are doing a grayscale image below */
		}
		else if (Ctrl->D.active) {	/* Already got an image with colors but need to set up a colormap of 256 entries */
			uint64_t cpt_len[1] = {256};
			grid_registration = GMT_GRID_PIXEL_REG;	/* Force pixel */
			/* We won't use much of the next 'P' but we still need to use some of its fields */
			if ((P = GMT_Create_Data (API, GMT_IS_PALETTE, GMT_IS_NONE, 0, cpt_len, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
			P->model = GMT_RGB;
			if (Img_proj->colormap == NULL && Img_proj->color_interp &&
			    (!strncmp (Img_proj->color_interp, "Gra", 3) || !strncmp (Img_proj->color_interp, "Red", 3) ||
				 !strncmp (Img_proj->color_interp, "Gre", 3) || !strncmp (Img_proj->color_interp, "Blu", 3) ||
			     !strncmp (Img_proj->color_interp, "Und", 3)) ) {
					/* Grayscale image, only assign r as shade */
				r_table = gmt_M_memory (GMT, NULL, 256, double);
				for (k = 0; k < 256; k++) r_table[k] = gmt_M_is255 (k);	/* Sets k/255.0 */
				gray_only = true;	/* Flag that we are doing a grayscale image below */
			}
			else if (Img_proj->colormap != NULL) {	/* Incoming image has a colormap, extract it */
				r_table = gmt_M_memory (GMT, NULL, 256, double);
				g_table = gmt_M_memory (GMT, NULL, 256, double);
				b_table = gmt_M_memory (GMT, NULL, 256, double);
				for (k = 0; k < 256; k++) {
					r_table[k] = gmt_M_is255 (I->colormap[k*4]);	/* 4 because image colormap is in RGBA format */
					g_table[k] = gmt_M_is255 (I->colormap[k*4 + 1]);
					b_table[k] = gmt_M_is255 (I->colormap[k*4 + 2]);
				}
				do_indexed = true;	/* Now it will be RGB */
				gray_only = false;	/* True technocolor, baby */
			}
		}
		else {
			GMT_Report (API, GMT_MSG_ERROR, "Indexed image without colormap. Can't proceed.\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (P && P->has_pattern) GMT_Report (API, GMT_MSG_WARNING, "Patterns in CPT will be ignored\n");

	NaN_rgb = (P) ? P->bfn[GMT_NAN].rgb : GMT->current.setting.color_patch[GMT_NAN];
	if (Ctrl->Q.active) {	/* Want colormask via NaN entries */
		if (gray_only) {
			GMT_Report (API, GMT_MSG_WARNING, "Your image is grayscale only but -Q requires 24-bit; image will be expanded to 24-bit.\n");
			gray_only = false;
			NaN_rgb = red;	/* Arbitrarily pick red as the NaN color since the entire image is gray only */
			gmt_M_memcpy (P->bfn[GMT_NAN].rgb, red, 4, double);
		}
		if (!Ctrl->A.return_image) rgb_used = gmt_M_memory (GMT, NULL, 256*256*256, unsigned char);	/* Keep track of which colors we encounter */
	}

	if (Ctrl->A.active) {	/* We desire a raster image, not a PostScript plot */
		int	id, k;
		unsigned int this_proj = GMT->current.proj.projection;
		char mem_layout[5] = {""}, *pch;
		if (Ctrl->M.active || gray_only) dim[GMT_Z] = 1;	/* Only one band */
		if (!need_to_project) {	/* Stick with original -R */
			img_wesn[XLO] = GMT->common.R.wesn[XLO];		img_wesn[XHI] = GMT->common.R.wesn[XHI];
			img_wesn[YHI] = GMT->common.R.wesn[YHI];		img_wesn[YLO] = GMT->common.R.wesn[YLO];
		}
		else {	/* Must use the projected limits, here in meters */
			img_wesn[XLO] = GMT->current.proj.rect_m[XLO];	img_wesn[XHI] = GMT->current.proj.rect_m[XHI];
			img_wesn[YHI] = GMT->current.proj.rect_m[YHI];	img_wesn[YLO] = GMT->current.proj.rect_m[YLO];
		}
		/* Determine raster image pixel sizes in the final units */
		img_inc[0] = (img_wesn[XHI] - img_wesn[XLO]) / (n_columns - !grid_registration);
		img_inc[1] = (img_wesn[YHI] - img_wesn[YLO]) / (n_rows - !grid_registration);

		if (grid_registration == GMT_GRID_NODE_REG) {	/* Adjust domain by 1/2 pixel since they are outside the domain */
			img_wesn[XLO] -= 0.5 * img_inc[0];		img_wesn[XHI] += 0.5 * img_inc[0];
			img_wesn[YLO] -= 0.5 * img_inc[1];		img_wesn[YHI] += 0.5 * img_inc[1];
		}
		if (Ctrl->Q.active) dim[GMT_Z]++;	/* Flag to remind us that we need to allocate a transparency array */
#ifdef HAVE_GDAL
		if (GMT->current.gdal_read_in.O.mem_layout[0])
			strcpy (mem_layout, GMT->current.gdal_read_in.O.mem_layout);	/* Backup current layout */
		else
#endif
			gmt_strncpy (mem_layout, "TRPa", 4);					/* Don't let it be empty (may it screw?) */
		GMT_Set_Default (API, "API_IMAGE_LAYOUT", "TRPa");			/* This is the grdimage's mem layout */

		if ((Out = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, dim, img_wesn, img_inc, 1, 0, NULL)) == NULL) {
			if (Ctrl->Q.active) gmt_M_free (GMT, rgb_used);
			Return(API->error);	/* Well, no luck with that allocation */
		}

		GMT_Set_Default (API, "API_IMAGE_LAYOUT", mem_layout);		/* Reset previous mem layout */

		HH = gmt_get_H_hidden (Out->header);
		if ((pch = strstr(Ctrl->Out.file, "+c")) != NULL) {			/* Check if we have +c<options> */
			HH->pocket = strdup(pch);
			pch[0] = '\0';
		}

		/* See if we have valid proj info the chosen projection has a valid PROJ4 setting */
		if (Out->header->ProjRefWKT != NULL)
			Out->header->ProjRefWKT = strdup (header_work->ProjRefWKT);
		else if (header_work->ProjRefPROJ4 != NULL)
			Out->header->ProjRefPROJ4 = strdup (header_work->ProjRefPROJ4);
		else if (header_work->ProjRefWKT != NULL)
			Out->header->ProjRefWKT = strdup (header_work->ProjRefWKT);
		else {
			for (k = 0, id = -1; id == -1 && k < GMT_N_PROJ4; k++)
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
		else {	/* Need 3-byte array for a 24-bit image plus possibly 3 bytes for the NaN mask color */
			if (Ctrl->Q.active) colormask_offset = 3;
			bitimage_24 = gmt_M_memory (GMT, NULL, 3 * header_work->nm + colormask_offset, unsigned char);
		}
		if (P && Ctrl->Q.active && !(Ctrl->M.active || gray_only)) {
			for (k = 0; k < 3; k++) bitimage_24[k] = gmt_M_u255 (P->bfn[GMT_NAN].rgb[k]);	/* Scale up to 0-255 range */
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate image pixel colors\n");

	/* Worry about linear projections with negative scales that may reverse the orientation of the image */
	normal_x = !(GMT->current.proj.projection_GMT == GMT_LINEAR && !GMT->current.proj.xyz_pos[0] && !resampled);
	normal_y = !(GMT->current.proj.projection_GMT == GMT_LINEAR && !GMT->current.proj.xyz_pos[1] && !resampled);

	actual_row = gmt_M_memory (GMT, NULL, n_rows, unsigned int);	/* Deal with any reversal of the y-axis due to -J */
	for (row = 0; row < n_rows; row++) actual_row[row] = (normal_y) ? row : n_rows - row - 1;
	actual_col = gmt_M_memory (GMT, NULL, n_columns, unsigned int);	/* Deal with any reversal of the x-axis due to -J */
	for (col = 0; col < n_columns; col++) actual_col[col] = (normal_x) ? col : n_columns - col - 1;

	intensity_mode = use_intensity_grid;				/* Set bit 1 */
	if (!n_grids || (IH && IH->reset_pad)) intensity_mode |= 2;	/* Add bit 2 */
	set_gray = (P && gray_only);
	rgb_from_z = (!Ctrl->D.active && !Ctrl->In.do_rgb);		/* Normal case of getting rgb from z(x,y) */
	rgb_cube_scan = (P && Ctrl->Q.active && !Ctrl->A.return_image);	/* Need to look for unique rgb for masking */
	step = (set_gray || Ctrl->M.active) ? 1 : 3;

	/* Evaluate colors at least once (try = 0), or twice if -Q is active and we need to select another unique NaN color not used in the image */
	for (try = 0, done = false; !done && try < 2; try++) {
		if (rgb_from_z && !Ctrl->Q.active) {	/* Got a single grid and need to look up color via the CPT */
			GMT_Report (API, GMT_MSG_INFORMATION, "Basic z(x,y) with optional illumination and no colormasking.\n");
//#ifdef _OPENMP
//#pragma omp parallel for private(row,byte,kk,col,node,index,rgb,k) shared(n_rows,header_work,actual_row,n_columns,GMT,P,Grid_proj,Ctrl,intensity_mode,Intens_proj,set_gray,bitimage_8,bitimage_24)
//#endif
			for (row = 0; row < n_rows; row++) {	/* March along scanlines */
				byte = colormask_offset + row * n_columns * step;
				kk = gmt_M_ijpgi (header_work, actual_row[row], 0);	/* Start pixel of this row */
				for (col = 0; col < n_columns; col++) {	/* Compute rgb for each pixel along this scanline */
					node = kk + actual_col[col];
					index = gmt_get_rgb_from_z (GMT, P, Grid_proj[0]->data[node], rgb);
					if (index != (GMT_NAN - 3) && Ctrl->I.active) {	/* Need to deal with illumination */
						if (intensity_mode & 1)	/* Intensity value comes from the grid */
							gmt_illuminate (GMT, Intens_proj->data[node], rgb);
						else	/* A constant (ambient) intensity was given via -I */
							gmt_illuminate (GMT, Ctrl->I.value, rgb);
					}
					/* Assigning bytes to colorimage */
					if (set_gray)	/* Color table only has grays, just use r since r = g = b here */
						bitimage_8[byte] = gmt_M_u255 (rgb[0]);
					else if (Ctrl->M.active)	/* Convert rgb to gray using the gmt_M_yiq transformation */
						bitimage_8[byte] = gmt_M_u255 (gmt_M_yiq (rgb));
					else {	/* Here we do r/g/b 24-bit color */
						for (k = 0; k < 3; k++) bitimage_24[byte+k] = gmt_M_u255 (rgb[k]);
					}
					byte += step;
				}
			}
			done = true;	/* Only doing the loop once here since no -Q */
		}
		else {	/* Dealing with images, three grids, and/or colormasking */
			for (row = 0, byte = colormask_offset; row < n_rows; row++) {	/* March along scanlines */
				kk = gmt_M_ijpgi (header_work, actual_row[row], 0);	/* Start pixel of this row */
				if (Ctrl->D.active && row == 0) node_RGBA = kk;		/* First time per row equals 'node', afterwards it grows alone */
				for (col = 0; col < n_columns; col++) {	/* Compute rgb for each pixel along this scanline */
					node = kk + actual_col[col];
					if (rgb_from_z) {	/* Got a single grid and need to look up color via the CPT */
						index = gmt_get_rgb_from_z (GMT, P, Grid_proj[0]->data[node], rgb);
						if (index != (GMT_NAN - 3)) has_content = true;
					}
					else if (Ctrl->D.active) {	/* Input was an image, not grid */
						if (!Ctrl->In.do_rgb) {
							k = (int)Img_proj->data[node];
							rgb[0] = r_table[k];	/* Either shade or red */
							if (do_indexed) {	/* Color via index colortable */
								rgb[1] = g_table[k];
								rgb[2] = b_table[k];
							}
						}
						else {	/* Got RGBA image, convert to rgb in 0-1 range */
							for (k = 0; k < 3; k++) rgb[k] = gmt_M_is255 (Img_proj->data[node_RGBA++]);
							if (Img_proj->header->n_bands == 4) node_RGBA++;	/* Must skip the alpha transparency byte in the image */
						}
					}
					else	/* Got three grids with red, green, blue values */
						index = set_rgb_three_grids (Grid_proj, node, NaN_rgb, rgb);

					if (Ctrl->I.active && index != (GMT_NAN - 3)) {	/* Need to deal with illumination */
						if (intensity_mode & 1) {	/* Intensity value comes from the grid */
							if (intensity_mode & 2)	/* Must recompute "node" with the gmt_M_ijp macro suitable for the image */
								node = gmt_M_ijp (Intens_proj->header, actual_row[row], actual_col[col]);
							gmt_illuminate (GMT, Intens_proj->data[node], rgb);
						}
						else	/* A constant (ambient) intensity was given via -I */
							gmt_illuminate (GMT, Ctrl->I.value, rgb);
					}

					if (set_gray)	/* Color table only has grays, just use r since r = g = b here */
						bitimage_8[byte++] = gmt_M_u255 (rgb[0]);
					else if (Ctrl->M.active)	/* Convert rgb to gray using the gmt_M_yiq transformation */
						bitimage_8[byte++] = gmt_M_u255 (gmt_M_yiq (rgb));
					else {	/* Here we do r/g/b 24-bit color */
						for (k = 0; k < 3; k++) bitimage_24[byte++] = i_rgb[k] = gmt_M_u255 (rgb[k]);
						if (rgb_cube_scan && index != (GMT_NAN - 3)) /* Keep track of all r/g/b combinations used except for NaN */
							rgb_used[(i_rgb[0]*256 + i_rgb[1])*256+i_rgb[2]] = true;
					}
				}

				if (!n_grids) node_RGBA += header_work->n_bands * (header_work->pad[XLO] + header_work->pad[XHI]);	/* Increment the node index for the image row */
			}
		}
		if (rgb_cube_scan) {	/* Check that we found an unused r/g/b value so colormasking will work as advertised */
			index = (gmt_M_u255(P->bfn[GMT_NAN].rgb[0])*256 + gmt_M_u255(P->bfn[GMT_NAN].rgb[1]))*256 + gmt_M_u255(P->bfn[GMT_NAN].rgb[2]);
			if (rgb_used[index]) {	/* This r/g/b already appears in the image as a non-NaN color; we must find a replacement NaN color */
				for (index = 0, ks = -1; ks == -1 && index < 256*256*256; index++)
					if (!rgb_used[index]) ks = index;	/* Use this entry instead */
				if (ks == -1) {	/* This is really really unlikely, meaning image uses all 256^3 colors */
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
		}
		else	/* No colormasking so we are done */
			done = true;
	}
	if (Ctrl->Q.active) gmt_M_free (GMT, rgb_used);	/* Done using the r/g/b cube */
	gmt_M_free (GMT, actual_row);
	gmt_M_free (GMT, actual_col);

	for (k = 1; k < n_grids; k++) {		/* Not done with Grid_proj[0] yet, hence we start loop at k = 1, which only will happen for the 3 grids case */
		if (need_to_project && GMT_Destroy_Data (API, &Grid_proj[k]) != GMT_NOERROR)
			GMT_Report (API, GMT_MSG_ERROR, "Failed to free Grid_proj[%d]\n", k);
	}
	if (use_intensity_grid) {	/* Also done with the intensity grid */
		if (need_to_project || !n_grids) {
			if (GMT_Destroy_Data (API, &Intens_proj) != GMT_NOERROR)
				GMT_Report (API, GMT_MSG_ERROR, "Failed to free Intens_proj\n");
		}
	}

	/* Get actual size of each pixel */
	dx = gmt_M_get_inc (GMT, header_work->wesn[XLO], header_work->wesn[XHI], header_work->n_columns, header_work->registration);
	dy = gmt_M_get_inc (GMT, header_work->wesn[YLO], header_work->wesn[YHI], header_work->n_rows, header_work->registration);

	/* Set lower left position of image on map; this depends on grid registration */

	x0 = header_work->wesn[XLO];	y0 = header_work->wesn[YLO];
	if (grid_registration == GMT_GRID_NODE_REG) {	/* Grid registration, move 1/2 pixel down/left */
		x0 -= 0.5 * dx;
		y0 -= 0.5 * dy;
	}

	/* Full rectangular dimension of the projected image in inches */
	x_side = dx * header_work->n_columns;
	y_side = dy * header_work->n_rows;

	if (P && gray_only) /* Determine if the gray image in fact is just black & white */
		for (kk = 0, P->is_bw = true; P->is_bw && kk < header_work->nm; kk++)
			if (!(bitimage_8[kk] == 0 || bitimage_8[kk] == 255)) P->is_bw = false;

	if (P && P->is_bw && !Ctrl->A.active) {	/* Can get away with a 1-bit image, but we must pack the original byte to 8 image bits */
		int nx8, shift, b_or_w, nx_pixels;
		uint64_t imsize, k8;
		unsigned char *bit = NULL;

		GMT_Report (API, GMT_MSG_INFORMATION, "Creating 1-bit B/W image\n");

		nx8 = irint (ceil (n_columns / 8.0));	/* Image width must be a multiple of 8 bits, so we round up */
		nx_pixels = nx8 * 8;	/* The row length in bits after the rounding up */
		imsize = gmt_M_get_nm (GMT, nx8, n_rows);
		bit = gmt_M_memory (GMT, NULL, imsize, unsigned char);	/* Memory to hold the 1-bit image */

		/* Reprocess the byte image.  Here there are no worries about direction of rows, cols since that was dealt with during color assignment */

		for (row = 0, k = k8 = 0; row < n_rows; row++) {	/* Process each scanline */
			shift = 0; byte = 0;
			for (col = 0; col < n_columns; col++, k++) {	/* Visit each byte in the original grayshade image */
				b_or_w = (bitimage_8[k] == 255);	/* Let white == 1, black == 0 */
				byte |= b_or_w;	/* Add in the current bit (0 or 1) */
				shift++;	/* Position us for the next bit */
				if (shift == 8) {	/* Did all 8, time to dump out another byte ["another one bytes the dust", he, he] */
					bit[k8++] = (unsigned char) byte;	/* Place the last 8 bits in output array... */
					byte = shift = 0;	/* ...and start over for next sequence of 8 nodes */
				}
				else	/* Move the bits we have so far 1 step to the left */
					byte <<= 1;
			}
			if (shift) {	/* Set the remaining bits in this bit to white; this applies to the last 1-7 nodes on each row */
				byte |= 1;
				shift++;
				while (shift < 8) {
					byte <<= 1;
					byte |= 1;
					shift++;
				}
				bit[k8++] = (unsigned char) byte;	/* Copy final byte from this row into the image */
			}
		}

		x_side = nx_pixels * dx;	/* Since the image may be 1-7 bits wider than what we need we must enlarge it a tiny amount */
		PSL_plotbitimage (PSL, x0, y0, x_side, y_side, PSL_BL, bit, nx_pixels, n_rows, Ctrl->G.rgb[GRDIMAGE_FGD], Ctrl->G.rgb[GRDIMAGE_BGD]);
		gmt_M_free (GMT, bit);	/* Done with the B/W buffer */
	}
	else if ((P && gray_only) || Ctrl->M.active) {	/* Here we have a 1-layer 8 bit grayscale image */
		if (Ctrl->A.active) {	/* Creating a raster image, not PostScript */
			GMT_Report (API, GMT_MSG_INFORMATION, "Creating 8-bit grayshade image %s\n", way[Ctrl->A.return_image]);
			if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->Out.file, Out) != GMT_NOERROR)
				Return (API->error);
		}
		else {	/* Lay down a PostScript 8-bit image */
			GMT_Report (API, GMT_MSG_INFORMATION, "Creating 8-bit grayshade image\n");
			PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_8, n_columns, n_rows, (Ctrl->E.device_dpi ? -8 : 8));
		}
	}
	else {	/* Dealing with a 24-bit color image */
		if (Ctrl->A.active) {	/* Creating a raster image, not PostScript */
			if (Ctrl->Q.active) {	/* Must initialize the transparency byte (alpha): 255 everywhere except at NaNs where it should be 0 */
				memset (Out->alpha, 255, header_work->nm);
				for (node = row = 0; row < n_rows; row++) {
					kk = gmt_M_ijpgi (header_work, row, 0);
					for (col = 0; col < n_columns; col++, node++) {
						if (gmt_M_is_fnan (Grid_proj[0]->data[kk + col])) Out->alpha[node] = 0;
					}
				}
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "Creating 24-bit color image %s\n", way[Ctrl->A.return_image]);
			if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->Out.file, Out) != GMT_NOERROR)
				Return (API->error);
		}
		else {	/* Lay down a PostScript 24-bit color image */
			GMT_Report (API, GMT_MSG_INFORMATION, "Creating 24-bit color image\n");
			PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_24, (Ctrl->Q.active ? -1 : 1) *
		    	n_columns, n_rows, (Ctrl->E.device_dpi ? -24 : 24));
		}
	}

	if (!Ctrl->A.active) {	/* Finalize PostScript plot, including basemap, etc. */
		if (!Ctrl->N.active) gmt_map_clip_off (GMT);

		gmt_map_basemap (GMT);
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);

		/* Here we are done producing the PostScript or raster image and can free things we used */

		/* Free bitimage arrays. gmt_M_free will not complain if they have not been used (NULL) */
		gmt_M_free (GMT, bitimage_8);
		gmt_M_free (GMT, bitimage_24);
	}

	if (need_to_project && n_grids && GMT_Destroy_Data (API, &Grid_proj[0]) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free Grid_proj[0]\n");
	}

	if (API->external && Ix && Iy) {	/* Restore old arrays since read-only image */
		gmt_M_free (GMT, I->x);	gmt_M_free (GMT, I->y);
		I->x = Ix;	I->y = Iy;
	}

	if (!need_to_project && !API->external) {
		for (k = 0; k < n_grids; k++) {	/* If memory grids are passed in we must restore the headers */
			if (mem_G[k]) {
				gmt_copy_gridheader (GMT, Grid_orig[k]->header, header_G[k]);
				gmt_free_header (API->GMT, &header_G[k]);
			}
		}
		if (mem_I) {
			gmt_copy_gridheader (GMT, Intens_orig->header, header_I);
			gmt_free_header (API->GMT, &header_I);
		}
		if (mem_D) {
			gmt_copy_gridheader (GMT, I->header, header_D);
			gmt_free_header (API->GMT, &header_D);
		}
	}

	if (Ctrl->D.active) {	/* Free the color tables for indexed or gray images */
		gmt_M_free (GMT, r_table);
		if (g_table) {
			gmt_M_free (GMT, g_table);
			gmt_M_free (GMT, b_table);
		}
		if (GMT_Destroy_Data (API, &Img_proj) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	if (!Ctrl->C.active && GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
		Return (API->error);
	}
	/* May return a flag that the image/PS has no data (see -Qn), or just NO_ERROR */
	ret_val = (Ctrl->W.active && !has_content) ? GMT_IMAGE_NO_DATA : GMT_NOERROR;
	Return (ret_val);
}
