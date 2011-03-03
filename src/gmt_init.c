/*--------------------------------------------------------------------
 *	$Id: gmt_init.c,v 1.460 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------
 *
 * gmt_init.c contains code which is used by all GMT programs
 *
 * Author:	Paul Wessel
 * Date:	15-FEB-2000
 * Version:	4
 *
 *
 * The PUBLIC functions are:
 *
 *	GMT_explain_option		Prints explanations for the common options
 *	GMT_parse_common_options	Interprets -B -H -J -K -O -P -R -U -V -X -Y -: -c
 *	GMT_getdefaults			Initializes the GMT global parameters
 *	GMT_putdefaults			Dumps the GMT global parameters
 *	GMT_free_plot_array		Free plot memory
 *	GMT_key_lookup			Linear Key - id lookup function
 *	GMT_hash_init			Initializes a hash
 *	GMT_hash_lookup			Key - id lookup using hashing
 *	GMT_hash			Key - id lookup using hashing
 *	GMT_begin			Gets history and init parameters
 *	GMT_end				Cleans up and exits
 *	GMT_history			Read and update the .gmtcommands4 file
 *	GMT_putpen			Encode pen argument into textstring
 *	GMT_parse_J_option		Scans the -Jstring to set projection
 *	GMT_parse_R_option		Scans the -Rstring and returns map boundaries
 *
 * The INTERNAL functions are:
 *
 *	GMT_loaddefaults		Reads the GMT global parameters from .gmtdefaults4
 *	GMT_savedefaults		Writes the GMT global parameters to .gmtdefaults4
 *	GMT_parse_?_option		Decode the -B, -H, -U, -: options
 *	GMT_setparameter		Sets a default value given keyword,value-pair
 *	GMT_setshorthand		Reads and initializes the suffix shorthands
 *	GMT_get_ellipsoid		Returns ellipsoid id based on name
 *	GMT_prepare_3D			Initialize 3-D parameters
 *	GMT_scanf_epoch			Get user time origin from user epoch string
 *	GMT_init_time_system_structure  Does what it says
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_globals.h"

#ifdef CYGWIN_LOCALE_TROUBLE	/* Temporary Cygwin locale fix for Joaquim while we monitor the issue */
#include <locale.h>
#endif

/*--------------------------------------------------------------------*/
/* Load parameters from include files */
/*--------------------------------------------------------------------*/

#include "gmt_keycases.h"		/* Get all the default case values */

char *GMT_unique_option[GMT_N_UNIQUE] = {	/* The common GMT commandline options */
#include "gmt_unique.h"
};

char *GMT_keywords[GMT_N_KEYS] = {		/* Names of all parameters in .gmtdefaults4 */
#include "gmt_keywords.h"
};

char *GMT_media_name[GMT_N_MEDIA] = {		/* Names of all paper formats */
#include "gmt_media_name.h"
};

struct GMT_MEDIA {	/* Holds information about paper sizes in points */
	double width;		/* Width in points */
	double height;		/* Height in points */
};
struct GMT_MEDIA GMT_media[GMT_N_MEDIA] = {			/* Sizes in points of all paper formats */
#include "gmt_media_size.h"
};

/* For Custom paper types */

char **GMT_user_media_name = (char **)NULL;
struct GMT_MEDIA *GMT_user_media = (struct GMT_MEDIA *)NULL;
GMT_LONG GMT_n_user_media = 0;

#define USER_MEDIA_OFFSET 1000

char *GMT_color_name[GMT_N_COLOR_NAMES] = {	/* Names of X11 colors */
#include "gmt_colornames.h"
};

char *GMT_weekdays[7] = {	/* Days of the week in English */
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

char *GMT_degree_choice[4] = {  /* Users choice for degree symbol */
	"none",
	"ring",
	"degree",
	"colon",
};

char *GMT_just_string[12] = {	/* Strings to specify justification */
	"", "BL", "BC", "BR", "", "ML", "MC", "MR", "", "TL", "TC", "TR"
};

EXTERN_MSC void GMT_grdio_init (void);	/* Defined in gmt_customio.c and only used here */

GMT_LONG GMT_load_user_media (void);
GMT_LONG true_false_or_error (char *value, GMT_LONG *answer);
GMT_LONG GMT_history (int argc, char **argv);
void GMT_prepare_3D (void);
void GMT_free_plot_array(void);
char *GMT_putpen (struct GMT_PEN *pen);
GMT_LONG GMT_get_time_language (char *name);
GMT_LONG GMT_scanf_epoch (char *s, GMT_cal_rd *day, double *t0);
void GMT_backwards_compatibility ();
GMT_LONG GMT_strip_colonitem (const char *in, const char *pattern, char *item, char *out);
void GMT_strip_wesnz (const char *in, GMT_LONG side[], GMT_LONG *draw_box, char *out);
GMT_LONG GMT_split_info (const char *in, char *info[]);
GMT_LONG GMT_decode_tinfo (char *in, struct GMT_PLOT_AXIS *A);
GMT_LONG GMT_set_titem (struct GMT_PLOT_AXIS *A, double val, double phase, char flag, char unit);
static GMT_LONG load_encoding (struct gmt_encoding *);
void GMT_verify_encodings ();
GMT_LONG GMT_key_lookup (char *name, char **list, GMT_LONG n);
void GMT_PS_init (void);
void *New_GMT_Ctrl ();
void Free_GMT_Ctrl (struct GMT_CTRL *C);
GMT_LONG GMT_project_type (char *args, GMT_LONG *pos, GMT_LONG *width_given);
GMT_LONG GMT_scale_or_width (char *scale_or_width, double *value);
GMT_LONG GMT_parse_B_option (char *in);
GMT_LONG GMT_parse_H_option (char *item);
GMT_LONG GMT_parse_U_option (char *item);
GMT_LONG GMT_parse_t_option (char *item);
GMT_LONG GMT_parse_g_option (char *item);
GMT_LONG GMT_loaddefaults (char *file);
GMT_LONG GMT_savedefaults (char *file);
void GMT_setshorthand (void);
void GMT_freeshorthand (void);
void GMT_set_inside_border (void);
double GMT_neg_col_dist (GMT_LONG col);
double GMT_pos_col_dist (GMT_LONG col);
double GMT_abs_col_dist (GMT_LONG col);
double GMT_neg_col_map_dist (GMT_LONG col);
double GMT_pos_col_map_dist (GMT_LONG col);
double GMT_abs_col_map_dist (GMT_LONG col);
double GMT_xy_map_dist (GMT_LONG col);
double GMT_xy_deg_dist (GMT_LONG col);
double GMT_xy_true_dist (GMT_LONG col);
double GMT_xy_cart_dist (GMT_LONG col);
void GMT_file_lock (int fd, struct flock *lock);
void GMT_file_unlock (int fd, struct flock *lock);
void GMT_put_colorname (FILE *fp, char *string, int *rgb);

/* Local variables to gmt_init.c */

struct GMT_HASH keys_hashnode[GMT_N_KEYS];
GMT_LONG GMT_x_abs = FALSE, GMT_y_abs = FALSE;
GMT_LONG GMT_got_frame_rgb;
struct GMT_BACKWARD {	/* Used to ensure backwards compatibility */
	GMT_LONG got_old_plot_format;		/* TRUE if DEGREE_FORMAT was decoded */
	GMT_LONG got_old_degree_symbol;		/* TRUE if DEGREE_FORMAT was decoded */
	GMT_LONG got_new_plot_format;		/* TRUE if PLOT_DEGREE_FORMAT was decoded */
	GMT_LONG got_new_degree_symbol;		/* TRUE if DEGREE_SYMBOL was decoded */
	GMT_LONG got_old_want_euro;		/* TRUE if WANT_EURO_FONTS was decoded */
	GMT_LONG got_new_char_encoding;		/* TRUE if CHAR_ENCODING was decoded */
} GMT_backward;

GMT_LONG GMT_force_resize = FALSE, GMT_annot_special = FALSE;
double save_annot_size[2], save_label_size, save_header_size;
double save_annot_offset[2], save_label_offset, save_header_offset, save_tick_length, save_frame_width;
GMT_LONG GMT_primary;
char month_names[12][16], *months[12];

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void GMT_explain_option (char option)
{

	/* The function print to stderr a short explanation for the option indicated by
	 * the variable <option>.  Only the common parameter options are covered
	 */

	char u, *GMT_choice[2] = {"OFF", "ON"};
	double s;

	u = GMT_unit_names[gmtdefs.measure_unit][0];
	s = GMT_u2u[GMT_INCH][gmtdefs.measure_unit];	/* Convert from internal inch to users unit */

	switch (option) {

		case 'B':	/* Tickmark option */

			fprintf (stderr, "\t-B specifies Basemap frame info.  <tickinfo> is a textstring made up of one or\n");
			fprintf (stderr, "\t   more substrings of the form [t]<stride>[+-<phase>][<unit>], where the (optional) [t] is the\n");
			fprintf (stderr, "\t   axis item type, <stride> is the spacing between ticks or annotations, the (optional)\n");
			fprintf (stderr, "\t   <phase> specifies phase-shifted annotations by that amount, and the (optional)\n");
			fprintf (stderr, "\t   <unit> specifies the <stride> unit [Default is unit implied in -R]. There can be\n");
			fprintf (stderr, "\t   no spaces between the substrings - just append to make one very long string.\n");
			fprintf (stderr, "\t   -B[p] means (p)rimary annotations; use -Bs to specify (s)econdary annotations.\n");
			fprintf (stderr, "\t   Three axis item types exist:\n");
			fprintf (stderr, "\t     a: tick annotation stride.\n");
			fprintf (stderr, "\t     f: frame tick stride.\n");
			fprintf (stderr, "\t     g: grid line stride.\n");
			fprintf (stderr, "\t   The optional [<unit>] modifies the <stride> value accordingly.  For maps, you may use\n");
			fprintf (stderr, "\t     m: arc minutes [Default unit is degree].\n");
			fprintf (stderr, "\t     c: arc seconds.\n");
			fprintf (stderr, "\t   For time axes, several units are recognized:\n");
			fprintf (stderr, "\t     Y: year - plot using all 4 digits.\n");
			fprintf (stderr, "\t     y: year - plot only last 2 digits.\n");
			fprintf (stderr, "\t     O: month - format annotation according to PLOT_DATE_FORMAT.\n");
			fprintf (stderr, "\t     o: month - plot as 2-digit integer (1-12).\n");
			fprintf (stderr, "\t     U: ISO week - format annotation according to PLOT_DATE_FORMAT.\n");
			fprintf (stderr, "\t     u: ISO week - plot as 2-digit integer (1-53).\n");
			fprintf (stderr, "\t     r: Gregorian week - 7-day stride from chosen start of week (%s).\n", GMT_weekdays[gmtdefs.time_week_start]);
			fprintf (stderr, "\t     K: ISO weekday - format annotation according to PLOT_DATE_FORMAT.\n");
			fprintf (stderr, "\t     k: weekday - plot name of weekdays in selected language [%s].\n", gmtdefs.time_language);
			fprintf (stderr, "\t     D: day  - format annotation according to PLOT_DATE_FORMAT, which also determines whether\n");
			fprintf (stderr, "\t               we should plot day of month (1-31) or day of year (1-366).\n");
			fprintf (stderr, "\t     d: day - plot as 2- (day of month) or 3- (day of year) integer.\n");
			fprintf (stderr, "\t     R: Same as d but annotates from start of Gregorian week.\n");
			fprintf (stderr, "\t     H: hour - format annotation according to PLOT_CLOCK_FORMAT.\n");
			fprintf (stderr, "\t     h: hour - plot as 2-digit integer (0-23).\n");
			fprintf (stderr, "\t     M: minute - format annotation according to PLOT_CLOCK_FORMAT.\n");
			fprintf (stderr, "\t     m: minute - plot as 2-digit integer (0-59).\n");
			fprintf (stderr, "\t     C: second - format annotation according to PLOT_CLOCK_FORMAT.\n");
			fprintf (stderr, "\t     c: second - plot as 2-digit integer (0-59; 60-61 if leap seconds are enabled).\n");
			fprintf (stderr, "\t   Specify an axis label by surrounding it with colons (e.g., :\"my x label\":).\n");
			fprintf (stderr, "\t   To prepend a prefix to each annotation (e.g., $ 10, $ 20 ...) add a prefix that begins\n");
			fprintf (stderr, "\t     with the equal-sign (=); the rest is used as annotation prefix (e.g. :=\'$\':). If the prefix has\n");
			fprintf (stderr, "\t     a leading hyphen (-) there will be no space between prefix and annotation (e.g., :=-\'$\':).\n");
			fprintf (stderr, "\t   To append a unit to each annotation (e.g., 5 km, 10 km ...) add a label that begins\n");
			fprintf (stderr, "\t     with a comma; the rest is used as unit annotation (e.g. :\",km\":). If the unit has\n");
			fprintf (stderr, "\t     a leading hyphen (-) there will be no space between unit and annotation (e.g., :,-%%:).\n");
			fprintf (stderr, "\t   For separate x and y [and z if -Jz is used] tickinfo, separate the strings with slashes [/].\n");
			fprintf (stderr, "\t   Specify an plot title by adding a label whose first character is a period; the rest\n");
			fprintf (stderr, "\t     of the label is used as the title (e.g. :\".My Plot Title\":).\n");
			fprintf (stderr, "\t   Append any combination of W, E, S, N, Z to annotate those axes only [Default is WESNZ (all)].\n");
			fprintf (stderr, "\t     Use lower case w, e, s, n, z to draw & tick but not to annotate those axes.\n");
			fprintf (stderr, "\t     Z+ will also draw a 3-D box .\n");
			fprintf (stderr, "\t   Log10 axis: Append l to annotate log10 (x) or p for 10^(log10(x)) [Default annotates x].\n");
			fprintf (stderr, "\t   Power axis: append p to annotate x at equidistant pow increments [Default is nonlinear].\n");
			fprintf (stderr, "\t   See psbasemap man pages for more details and examples of all settings.\n");
			break;

		case 'b':	/* Condensed tickmark option */

			fprintf (stderr, "\t-B Boundary annotation, give -B[p|s]<xinfo>[/<yinfo>[/<zinfo>]][.:\"title\":][wesnzWESNZ+]\n");
			fprintf (stderr, "\t   <?info> is 1-3 substring(s) of form [<type>]<stride>[<unit>][l|p][:\"label\":][:,[-]\"unit\":]\n");
			fprintf (stderr, "\t   See psbasemap man pages for more details and examples of all settings.\n");
			break;

		case 'H':	/* Header */

			fprintf (stderr, "\t-H[i][n_rec] means input/output file has %ld Header record(s) [%s]\n", gmtdefs.n_header_recs, GMT_choice[gmtdefs.io_header[GMT_IN]]);
			fprintf (stderr, "\t   Optionally, append i for input only and/or number of header records\n");
			break;

		case 'J':	/* Map projection option */

			fprintf (stderr, "\t-J Selects the map proJection system. The projection type is identified by a 1- or\n");
			fprintf (stderr, "\t   2-character ID (e.g. 'm' or 'kf') or by an abbreviation followed by a slash\n");
			fprintf (stderr, "\t   (e.g. 'cyl_stere/'). When using a lower-case ID <scale> can be given either as 1:<xxxx>\n");
			fprintf (stderr, "\t   or in %s/degree along the standard parallel. Alternatively, when the projection ID is\n", GMT_unit_names[gmtdefs.measure_unit]);
			fprintf (stderr, "\t   Capitalized, <scale|width> denotes the width of the plot in %s\n", GMT_unit_names[gmtdefs.measure_unit]);
			fprintf (stderr, "\t   Append h for map height, + for max map dimension, and - for min map dimension.\n");
			fprintf (stderr, "\t   When the central meridian (lon0) is optional and omitted, the center of the\n");
			fprintf (stderr, "\t   longitude range specified by -R is used. The default standard parallel is the equator\n");
			fprintf (stderr, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is given.\n");

			fprintf (stderr, "\t   -Ja|A<lon0>/<lat0>[/<horizon>]/<scale|width> (Lambert Azimuthal Equal Area)\n");
			fprintf (stderr, "\t     lon0/lat0 is the center of the projection.\n");
			fprintf (stderr, "\t     horizon is max distance from center of the projection (<= 180, default 90).\n");
			fprintf (stderr, "\t     Scale can also be given as <radius>/<lat>, where <radius> is the distance\n");
			fprintf (stderr, "\t     in %s to the oblique parallel <lat>.\n", GMT_unit_names[gmtdefs.measure_unit]);

			fprintf (stderr, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scale|width> (Albers Equal-Area Conic)\n");
			fprintf (stderr, "\t     Give origin, 2 standard parallels, and true scale\n");

			fprintf (stderr, "\t   -Jc|C<lon0>/<lat0><scale|width> (Cassini)\n\t     Give central point and scale\n");

			fprintf (stderr, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<scale|width> (Cylindrical Stereographic)\n");
			fprintf (stderr, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			fprintf (stderr, "\t     <lat0> = 66.159467 (Miller's modified Gall), 55 (Kamenetskiy's First),\n");
			fprintf (stderr, "\t     45 (Gall Stereographic), 30 (Bolshoi Sovietskii Atlas Mira), 0 (Braun)\n");

			fprintf (stderr, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scale|width> (Equidistant Conic)\n");
			fprintf (stderr, "\t     Give origin, 2 standard parallels, and true scale\n");

			fprintf (stderr, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scale|width> (Azimuthal Equidistant)\n");
			fprintf (stderr, "\t     lon0/lat0 is the center of the projection.\n");
			fprintf (stderr, "\t     horizon is max distance from center of the projection (<= 180, default 180).\n");
			fprintf (stderr, "\t     Scale can also be given as <radius>/<lat>, where <radius> is the distance\n");
			fprintf (stderr, "\t     in %s to the oblique parallel <lat>. \n", GMT_unit_names[gmtdefs.measure_unit]);

			fprintf (stderr, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scale|width> (Gnomonic)\n");
			fprintf (stderr, "\t     lon0/lat0 is the center of the projection.\n");
			fprintf (stderr, "\t     horizon is max distance from center of the projection (< 90, default 60).\n");
			fprintf (stderr, "\t     Scale can also be given as <radius>/<lat>, where <radius> is distance\n");
			fprintf (stderr, "\t     in %s to the oblique parallel <lat>. \n", GMT_unit_names[gmtdefs.measure_unit]);

			fprintf (stderr, "\t   -Jg|G<lon0>/<lat0>/<scale|width> (Orthographic)\n");
			fprintf (stderr, "\t     lon0/lat0 is the center of the projection.\n");
			fprintf (stderr, "\t     Scale can also be given as <radius>/<lat>, where <radius> is distance\n");
			fprintf (stderr, "\t     in %s to the oblique parallel <lat>. \n", GMT_unit_names[gmtdefs.measure_unit]);

			fprintf (stderr, "\t   -Jg|G<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale|width> (General Perspective)\n");
			fprintf (stderr, "\t     lon0/lat0 is the center of the projection.\n");
			fprintf (stderr, "\t     Altitude is the height (in km) of the viewpoint above local sea level\n");
			fprintf (stderr, "\t        - if altitude less than 10 then it is the distance \n");
			fprintf (stderr, "\t        from center of earth to viewpoint in earth radii\n");
			fprintf (stderr, "\t        - if altitude has a suffix of 'r' then it is the radius \n");
			fprintf (stderr, "\t        from the center of earth in kilometers\n");
			fprintf (stderr, "\t     Azimuth is azimuth east of North of view\n");
			fprintf (stderr, "\t     Tilt is the upward tilt of the plane of projection\n");
			fprintf (stderr, "\t       if tilt < 0 then viewpoint is centered on the horizon\n");
			fprintf (stderr, "\t     Twist is the CW twist of the viewpoint in degree\n");
			fprintf (stderr, "\t     Width is width of the viewpoint in degree\n");
			fprintf (stderr, "\t     Height is the height of the viewpoint in degrees\n");
			fprintf (stderr, "\t     Scale can also be given as <radius>/<lat>, where <radius> is distance\n");
			fprintf (stderr, "\t     in %s to the oblique parallel <lat>. \n", GMT_unit_names[gmtdefs.measure_unit]);

			fprintf (stderr, "\t   -Jh|H[<lon0>/]<scale|width> (Hammer-Aitoff)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Ji|I[<lon0>/]<scale|width> (Sinusoidal)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Jj|J[<lon0>/]<scale|width> (Miller)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Jkf|Kf[<lon0>/]<scale|width> (Eckert IV)\n\t     Give central meridian (opt) and scale\n");
			fprintf (stderr, "\t   -Jk|K[s][<lon0>/]<scale|width> (Eckert VI)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scale|width> (Lambert Conformal Conic)\n");
			fprintf (stderr, "\t     Give origin, 2 standard parallels, and true scale\n");

			fprintf (stderr, "\t   -Jm|M[<lon0>/[<lat0>/]]<scale|width> (Mercator).\n");
			fprintf (stderr, "\t     Give central meridian (opt), true scale parallel (opt), and scale\n");

			fprintf (stderr, "\t   -Jn|N[<lon0>/]<scale|width> (Robinson projection)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Jo|O<parameters> (Oblique Mercator).  Specify one of three definitions:\n");
			fprintf (stderr, "\t     -Jo|O[a]<lon0>/<lat0>/<azimuth>/<scale|width>\n");
			fprintf (stderr, "\t       Give origin, azimuth of oblique equator, and scale at oblique equator\n");
			fprintf (stderr, "\t     -Jo|O[b]<lon0>/<lat0>/<lon1>/<lat1>/<scale|width>\n");
			fprintf (stderr, "\t       Give origin, second point on oblique equator, and scale at oblique equator\n");
			fprintf (stderr, "\t     -Jo|Oc<lon0>/<lat0>/<lonp>/<latp>/<scale|width>\n");
			fprintf (stderr, "\t       Give origin, pole of projection, and scale at oblique equator\n");
			fprintf (stderr, "\t       Specify region in oblique degrees OR use -R<>r\n");

			fprintf (stderr, "\t   -Jp|P[a]<scale|width>[/<base>][r|z] (Polar (theta,radius))\n");
			fprintf (stderr, "\t     Linear scaling for polar coordinates.\n");
			fprintf (stderr, "\t     Optionally append 'a' to -Jp or -JP to use azimuths (CW from North) instead of directions (CCW from East) [default].\n");
			fprintf (stderr, "\t     Give scale in %s/units, and append theta value for angular offset (base) [0]\n", GMT_unit_names[gmtdefs.measure_unit]);
			fprintf (stderr, "\t     Append r to reverse radial direction (s/n must be in 0-90 range) or z to annotate depths rather than radius [Default]\n");

			fprintf (stderr, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scale|width> ((American) Polyconic)\n");
			fprintf (stderr, "\t     Give central meridian (opt), reference parallel (opt, default = equator), and scale\n");

			fprintf (stderr, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scale|width> (Equidistant Cylindrical)\n");
			fprintf (stderr, "\t     Give central meridian (opt), standard parallel (opt), and scale\n");
			fprintf (stderr, "\t     <lat0> = 61.7 (Min. linear distortion), 50.5 (R. Miller equirectangular),\n");
			fprintf (stderr, "\t     45 (Gall isographic), 43.5 (Min. continental distortion), 42 (Grafarend & Niermann),\n");
			fprintf (stderr, "\t     37.5 (Min. overall distortion), 0 (Plate Carree, default)\n");

			fprintf (stderr, "\t   -Jr|R[<lon0>/]<scale|width> (Winkel Tripel)\n\t     Give central meridian and scale\n");

			fprintf (stderr, "\t   -Js|S<lon0>/<lat0>[/<horizon>]/<scale|width> (Stereographic)\n");
			fprintf (stderr, "\t     lon0/lat0 is the center or the projection.\n");
			fprintf (stderr, "\t     horizon is max distance from center of the projection (< 180, default 90).\n");
			fprintf (stderr, "\t     Scale is either <1:xxxx> (true at pole) or <slat>/<1:xxxx> (true at <slat>)\n");
			fprintf (stderr, "\t     or <radius>/<lat> (distance in %s to the [oblique] parallel <lat>.\n", GMT_unit_names[gmtdefs.measure_unit]);

			fprintf (stderr, "\t   -Jt|T<lon0>/[<lat0>/]<scale|width> (Transverse Mercator).\n\t         Give central meridian and scale\n");
			fprintf (stderr, "\t     Optionally, also give the central parallel (default = equator)\n");

			fprintf (stderr, "\t   -Ju|U<zone>/<scale|width> (UTM)\n");
			fprintf (stderr, "\t     Give zone (A,B,Y,Z, or 1-60 (negative for S hemisphere) or append C-X) and scale\n");

			fprintf (stderr, "\t   -Jv|V[<lon0>/]<scale|width> (van der Grinten)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Jw|W[<lon0>/]<scale|width> (Mollweide)\n\t     Give central meridian (opt) and scale\n");

			fprintf (stderr, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scale|width> (Cylindrical Equal-area)\n");
			fprintf (stderr, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			fprintf (stderr, "\t     <lat0> = 50 (Balthasart), 45 (Gall-Peters), 37.5 (Hobo-Dyer), 37.4 (Trystan Edwards),\n");
			fprintf (stderr, "\t              37.0666 (Caster), 30 (Behrmann), 0 (Lambert, default)\n");

			fprintf (stderr, "\t   -Jx|X<x-scale|width>[/<y-scale|height>] (Linear, log, power scaling)\n");
			fprintf (stderr, "\t     Scale in %s/units (or 1:xxxx). Optionally, append to <x-scale> and/or <y-scale>:\n", GMT_unit_names[gmtdefs.measure_unit]);
			fprintf (stderr, "\t       d         Geographic coordinate (in degrees)\n");
			fprintf (stderr, "\t       l         Log10 projection\n");
			fprintf (stderr, "\t       p<power>  x^power projection\n");
			fprintf (stderr, "\t       t         Calendar time projection using relative time coordinates\n");
			fprintf (stderr, "\t       T         Calendar time projection using absolute time coordinates\n");
			fprintf (stderr, "\t     Use / to specify separate x/y scaling (e.g., -Jx0.5/0.3.).  Not allowed with 1:xxxxx.\n");
			fprintf (stderr, "\t     If -JX is used then give axes lengths rather than scales.\n");
			break;

		case 'j':	/* Condensed version of J */

			fprintf (stderr, "\t-J Selects map proJection. (<scale> in %s/degree, <width> in %s)\n", GMT_unit_names[gmtdefs.measure_unit], GMT_unit_names[gmtdefs.measure_unit]);
			fprintf (stderr, "\t   Append h for map height, + for max map dimension, and - for min map dimension.\n");
			fprintf (stderr, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is given.\n\n");

			fprintf (stderr, "\t   -Ja|A<lon0>/<lat0>[/<horizon>]/<scale (or radius/lat)|width> (Lambert Azimuthal Equal Area)\n");

			fprintf (stderr, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scale|width> (Albers Equal-Area Conic)\n");

			fprintf (stderr, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<lat1>/<lat2>/<scale|width> (Cylindrical Stereographic)\n");

			fprintf (stderr, "\t   -Jc|C<lon0>/<lat0><scale|width> (Cassini)\n");

			fprintf (stderr, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scale|width> (Equidistant Conic)\n");

			fprintf (stderr, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scale (or radius/lat)|width>  (Azimuthal Equidistant)\n");

			fprintf (stderr, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scale (or radius/lat)|width>  (Gnomonic)\n");

			fprintf (stderr, "\t   -Jg|G<lon0>/<lat0>/<scale (or radius/lat)|width>  (Orthographic)\n");

			fprintf (stderr, "\t   -Jg|G[<lon0>/]<lat0>[/<horizon>|/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>]/<scale|width> (General Perspective)\n");

			fprintf (stderr, "\t   -Jh|H[<lon0>/]<scale|width> (Hammer-Aitoff)\n");

			fprintf (stderr, "\t   -Ji|I[<lon0>/]<scale|width> (Sinusoidal)\n");

			fprintf (stderr, "\t   -Jj|J[<lon0>/]<scale|width> (Miller)\n");

			fprintf (stderr, "\t   -Jkf|Kf[<lon0>/]<scale|width> (Eckert IV)\n");

			fprintf (stderr, "\t   -Jks|Ks[<lon0>/]<scale|width> (Eckert VI)\n");

			fprintf (stderr, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scale|width> (Lambert Conformal Conic)\n");

			fprintf (stderr, "\t   -Jm|M[<lon0>/[<lat0>/]]<scale|width> (Mercator).\n");

			fprintf (stderr, "\t   -Jn|N[<lon0>/]<scale|width> (Robinson projection)\n");

			fprintf (stderr, "\t   -Jo|O (Oblique Mercator).  Specify one of three definitions:\n");
			fprintf (stderr, "\t      -Jo|O[a]<lon0>/<lat0>/<azimuth>/<scale|width>\n");
			fprintf (stderr, "\t      -Jo|O[b]<lon0>/<lat0>/<lon1>/<lat1>/<scale|width>\n");
			fprintf (stderr, "\t      -Jo|Oc<lon0>/<lat0>/<lonp>/<latp>/<scale|width>\n");

			fprintf (stderr, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scale|width> ((American) Polyconic)\n");

			fprintf (stderr, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scale|width> (Equidistant Cylindrical)\n");

			fprintf (stderr, "\t   -Jr|R[<lon0>/]<scale|width> (Winkel Tripel)\n");

			fprintf (stderr, "\t   -Js|S<lon0>/<lat0>/[<horizon>/]<scale (or slat/scale or radius/lat)|width> (Stereographic)\n");

			fprintf (stderr, "\t   -Jt|T<lon0>/[<lat0>/]<scale|width> (Transverse Mercator).\n");

			fprintf (stderr, "\t   -Ju|U<zone>/<scale|width> (UTM)\n");

			fprintf (stderr, "\t   -Jv|V<lon0>/<scale|width> (van der Grinten)\n");

			fprintf (stderr, "\t   -Jw|W<lon0>/<scale|width> (Mollweide)\n");

			fprintf (stderr, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scale|width> (Cylindrical Equal-area)\n");

			fprintf (stderr, "\t   -Jp|P[a]<scale|width>[/<origin>][r|z] (Polar [azimuth] (theta,radius))\n");

			fprintf (stderr, "\t   -Jx|X<x-scale|width>[d|l|p<power>|t|T][/<y-scale|height>[d|l|p<power>|t|T]] (Linear, log, and power projections)\n");
			fprintf (stderr, "\t   (See psbasemap for more details on projection syntax)\n");
			break;

		case 'K':	/* Append-more-PostScript-later */

			fprintf (stderr, "\t-K means allow for more plot code to be appended later [%s].\n", GMT_choice[!GMT_ps.last_page]);
			break;

		case 'm':	/* Multisegment option */

			fprintf (stderr, "\t-m Input/output file(s) contain multiple segments separated by a record\n");
			fprintf (stderr, "\t   whose first character is <flag> [%c]\n", GMT_io.EOF_flag[GMT_IN]);
			fprintf (stderr, "\t   Use -mi or -mo to give different settings for input and output [Same]\n");
			break;

		case 'O':	/* Overlay plot */

			fprintf (stderr, "\t-O means Overlay plot mode [%s].\n", GMT_choice[GMT_ps.overlay]);
			break;

		case 'P':	/* Portrait or landscape */

			fprintf (stderr, "\t-P means Portrait page orientation [%s].\n", GMT_choice[GMT_ps.portrait]);
			break;

		case 'R':	/* Region option */

			fprintf (stderr, "\t-R specifies the min/max coordinates of data region in user units.\n");
			fprintf (stderr, "\t   Use dd:mm[:ss] format for regions given in degrees and minutes [and seconds].\n");
			fprintf (stderr, "\t   Use [yyy[-mm[-dd]]]T[hh[:mm[:ss[.xxx]]]] format for time axes.\n");
			fprintf (stderr, "\t   Append r if -R specifies the longitudes/latitudes of the lower left\n");
			fprintf (stderr, "\t   and upper right corners of a rectangular area.\n");
			fprintf (stderr, "\t   -Rg -Rd are accepted shorthands for -R0/360/-90/90 -R-180/180/-90/90\n");
			fprintf (stderr, "\t   Alternatively, give a gridfile and use its limits (and increments if applicable).\n");
			break;

		case 'r':	/* Region option for 3-D */

			fprintf (stderr, "\t-R specifies the xyz min/max coordinates of the plot window in user units.\n");
			fprintf (stderr, "\t   Use dd:mm[:ss] format for regions given in degrees and minutes [and seconds].\n");
			fprintf (stderr, "\t   Append r if first 4 arguments to -R specify the longitudes/latitudes\n");
			fprintf (stderr, "\t   of the lower left and upper right corners of a rectangular area\n");
			fprintf (stderr, "\t   Alternatively, give a gridfile and use its limits (and increments if applicable).\n");
			break;

		case 'U':	/* Plot time mark and [optionally] command line */

			fprintf (stderr, "\t-U to plot Unix System Time stamp [and optionally appended text].\n");
			fprintf (stderr, "\t   You may also set the reference points and position of stamp [%s/%g%c/%g%c].\n", GMT_just_string[gmtdefs.unix_time_just], gmtdefs.unix_time_pos[GMT_X] * s, u, gmtdefs.unix_time_pos[GMT_Y] * s, u);
			fprintf (stderr, "\t   Give -Uc to have the command line plotted [%s].\n", GMT_choice[gmtdefs.unix_time]);
			break;

		case 'V':	/* Verbose */

			fprintf (stderr, "\t-V Run in verbose mode [%s].\n", GMT_choice[gmtdefs.verbose]);
			break;

		case 'X':
		case 'Y':	/* Reset plot origin option */

			fprintf (stderr, "\t-X -Y to shift origin of plot to (<xshift>, <yshift>) [a%g%c,a%g%c].\n", gmtdefs.x_origin * s, u, gmtdefs.y_origin * s, u);
			fprintf (stderr, "\t   Prepend a for absolute [Default r is relative]\n");
			fprintf (stderr, "\t   (Note that for overlays (-O), the default is [r0,r0].)\n");
			fprintf (stderr, "\t   Give c to center plot on page in x and/or y.\n");
			break;

		case 'Z':	/* Vertical scaling for 3-D plots */

			fprintf (stderr, "\t   -Jz for z component of 3-D projections.  Same syntax as -Jx.\n");
			break;

		case 'E':	/* Enhanced pseudo-perspective 3-D plot settings */

			fprintf (stderr, "\t-E set azimuth and elevation of viewpoint for 3-D pseudo perspective view [180/90].\n");
			fprintf (stderr, "\t   Optionally, append +w<lon/lat[/z] to specify a fixed point and +vx/y for its justification.\n");
			fprintf (stderr, "\t   Just append + by itself to select default values [region center and page center]\n");
			break;

		case 'c':	/* Set number of plot copies option */

			fprintf (stderr, "\t-c specifies the number of copies [%ld].\n", gmtdefs.n_copies);
			break;

		case 'i':	/* -b binary option with input only */

			fprintf (stderr, "\t-bi for binary input.  Append s for single precision [Default is double]\n");
			break;

		case 'n':	/* -bi addendum when input format is unknown */

			fprintf (stderr, "\t    Append <n> for the number of columns in binary file(s).\n");
			break;

		case 'o':	/* -b binary option with output only */

			fprintf (stderr, "\t-bo for binary output. Append s for single precision [Default is double]\n");
			break;

		case ':':	/* lon/lat or lat/lon */

			fprintf (stderr, "\t-: Expect lat/lon input/output rather than lon/lat [%s/%s].\n", GMT_choice[gmtdefs.xy_toggle[GMT_IN]], GMT_choice[gmtdefs.xy_toggle[GMT_OUT]]);
			break;

		case 'f':	/* -f option to tell GMT which columns are time (and optionally geographical) */

			fprintf (stderr, "\t-f Special formatting of input/output columns (e.g., time or geographical)\n");
			fprintf (stderr, "\t   Specify i(nput) or o(utput) [Default is both input and output]\n");
			fprintf (stderr, "\t   Give one or more columns (or column ranges) separated by commas.\n");
			fprintf (stderr, "\t   Append T (Calendar format), t (time relative to TIME_EPOCH), f (plain floating point)\n");
			fprintf (stderr, "\t   x (longitude), y (latitude) to each col/range item.\n");
			fprintf (stderr, "\t   -f[i|o]g means -f[i|o]0x,1y (geographic coordinates).\n");
			break;

		case 'g':	/* -g option to tell GMT to identify data gaps based on point separation */
			fprintf (stderr, "\t-g Use point separation to identify data gaps.\n");
			fprintf (stderr, "\t   Append x|X or y|Y to flag gaps in x or y coordinates, respectively, and d|D for distance gaps.\n");
			fprintf (stderr, "\t   Upper case means we first project the points.  Append <gap> and optionally a unit.\n");
			fprintf (stderr, "\t   For geographic data, choose from m(e)ter [Default], (k)ilometer, (m)iles), or (n)autical miles).\n");
			fprintf (stderr, "\t   For gaps based on mapped coordinates, choose from (i)nch, (c)entimeter, (m)eter, or (p)oints [%s].\n", GMT_unit_names[gmtdefs.measure_unit]);
			fprintf (stderr, "\t   Note: For x|y with time data the unit is instead controlled by TIME_UNIT\n");
			fprintf (stderr, "\t   Repeat option to specify multiple criteria, and prepend + to indicate all critera must be met [any].\n");
			break;
	
		case '.':	/* Trailer message */

			fprintf (stderr, "\t(See gmtdefaults man page for hidden GMT default parameters)\n");
			break;

		default:
			break;
	}
}

void GMT_label_syntax (GMT_LONG indent, GMT_LONG kind)
{
	/* Contour/line specifications in *contour and psxy[z]
	 * indent is the number of spaces to indent after the TAB.
	 * kind = 0 for *contour and 1 for psxy[z]
	 */

	GMT_LONG i;
	char pad[16];

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	fprintf (stderr, "%s +a<angle> for annotations at a fixed angle, +an for line-normal, or +ap for line-parallel [Default]\n", pad);
	if (kind == 0) fprintf (stderr, "%s   For +ap, optionally append u for up-hill and d for down-hill cartographic annotations\n", pad);
	fprintf (stderr, "%s +c<dx>[/<dy>] to change the clearance between label and text box [15%%]\n", pad);
	fprintf (stderr, "%s +d turns on debug which draws helper points and lines\n", pad);
	fprintf (stderr, "%s +f followed by desired label font [Default is %ld].\n", pad, gmtdefs.annot_font[0]);
	fprintf (stderr, "%s +g[<color>] for opaque text box [Default is transparent]; optionally give color [white]\n", pad);
	fprintf (stderr, "%s +j<just> to set label justification [Default is CM]\n", pad);
	fprintf (stderr, "%s +k<color> to change color of label text [Default is black]\n", pad);
	if (kind == 1) {
		fprintf (stderr, "%s +l<label> Use this fixed text as the label (quote text if containing spaces).\n", pad);
		fprintf (stderr, "%s +L<d|D|f|h|n|N|x>[<unit>] Sets the label according to the given flag:\n", pad);
		fprintf (stderr, "%s   d Cartesian plot distance; append desired unit c, i, m, or p.\n", pad);
		fprintf (stderr, "%s   D Map distance; append desired unit d, e, k, m, or n.\n", pad);
		fprintf (stderr, "%s   f Label is text after 2nd column the <ffile.d> fixed label location file.\n", pad);
		fprintf (stderr, "%s   h Use multisegment header labels (either in -Lstring or first word).\n", pad);
		fprintf (stderr, "%s   n Use the current multisegment number (starting at 0).\n", pad);
		fprintf (stderr, "%s   N Use current file number / multisegment number (starting at 0/0).\n", pad);
		fprintf (stderr, "%s   x Like h, but scan headers in the <xfile.d> crossing lines instead.\n", pad);
	}
	fprintf (stderr, "%s +n<dx>[/<dy>] to nudge placement of label along line (+N for along x/y axis)\n", pad);
	fprintf (stderr, "%s +o to use rounded rectangular text box [Default is rectangular]\n", pad);
	fprintf (stderr, "%s +p[<pen>] draw outline of textbox  [Default is no outline]; optionally give pen [Default is default pen]\n", pad);
	fprintf (stderr, "%s +r<min_rad> places no labels where radius of curvature < <min_rad> [Default is 0].\n", pad);
	fprintf (stderr, "%s +s followed by desired font size in points [Default is 9 point].\n", pad);
	fprintf (stderr, "%s +u<unit> to append unit to labels; Start with - for no space between annotation and unit.\n", pad);
	if (kind == 0) fprintf (stderr, "%s  If no unit appended, use z-unit from grdfile. [Default is no unit]\n", pad);
	fprintf (stderr, "%s +v for placing curved text along path [Default is straight]\n", pad);
	fprintf (stderr, "%s +w to set how many (x,y) points to use for angle calculation [Default is 10]\n", pad);
	fprintf (stderr, "%s +=<prefix> to give labels a prefix; Start with - for no space between annotation and prefix.\n", pad);
}

void GMT_cont_syntax (GMT_LONG indent, GMT_LONG kind)
{
	/* Contour/line label placement specifications in *contour and psxy[z]
	 * indent is the number of spaces to indent after the TAB.
	 * kind = 0 for *contour and 1 for psxy[z]
	 */
	GMT_LONG i;
	double gap;
	char pad[16];
	char *type[2] = {"contour", "quoted line"};

	gap = 4.0 * GMT_u2u[GMT_INCH][gmtdefs.measure_unit];

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	fprintf (stderr, "%sd<dist>[c|i|m|p] or D<dist>[d|e|k|m|n].\n", pad);
	fprintf (stderr, "%s   d: Give distance between labels in specified unit [Default algorithm is d%g%c]\n", pad, gap, GMT_unit_names[gmtdefs.measure_unit][0]);
	fprintf (stderr, "%s   D: Specify distance between labels in m(e)ter [Default], (k)m, (m)ile, (n)autical mile, or (d)egree.\n", pad);
	fprintf (stderr, "%s   The first label appears at <frac>*<dist>; change by appending /<frac> [0.25].\n", pad);
	fprintf (stderr, "%sf<ffile.d> reads the file <ffile.d> and places labels at those locations that match\n", pad);
	fprintf (stderr, "%s   individual points along the %ss\n", pad, type[kind]);
	fprintf (stderr, "%sl|L<line1>[,<line2>,...] Give start and stop coordinates for straight line segments.\n", pad);
	fprintf (stderr, "%s   Labels will be placed where these lines intersect %ss.  The format of each <line> is\n", pad, type[kind]);
	fprintf (stderr, "%s   <start>/<stop>, where <start> or <stop> = <lon/lat> or a 2-character XY key that uses the\n", pad);
	fprintf (stderr, "%s   \"pstext\"-style justification format to specify a point on the map as [LCR][BMT].\n", pad);
	if (kind == 0) fprintf (stderr, "%s   In addition, you can use Z-, Z+ to mean the global min, max locations in the grid.\n", pad);
	fprintf (stderr, "%s   L: Let point pairs define great circles [Default is a straight line].\n", pad);
	fprintf (stderr, "%sn|N<n_label> specifies the number of equidistant labels per %s.\n", pad, type[kind]);
	fprintf (stderr, "%s   N: Starts labeling exactly at the start of %s [Default centers the labels].\n", pad, type[kind]);
	fprintf (stderr, "%s   N-1 places one label at start, while N+1 places one label at the end of the %s.\n", pad, type[kind]);
	fprintf (stderr, "%s   Append /<min_dist> to enforce a minimum distance between successive labels [0]\n", pad);
	fprintf (stderr, "%sx|X<xfile.d> reads the multi-segment file <xfile.d> and places labels at the intersections\n", pad);
	fprintf (stderr, "%s   between the %ss and the lines in <xfile.d>.  X: Resample the lines first.\n", pad, type[kind]);
	fprintf (stderr, "%s   For all options, append +r<radius>[unit] to specify minimum radial separation between labels [0]\n", pad);
}

void GMT_inc_syntax (char option, GMT_LONG error)
{
	if (error) fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c<xinc>[m|c|e|k|i|n|+][=][/<yinc>[m|c|e|k|i|n|+][=]]\n", option);
	fprintf (stderr, "\t   Give increment and append unit (m)inute, se(c)ond, m(e)ter, (k)ilometer, m(i)les, (n)autical miles.\n");
	fprintf (stderr, "\t   (Note: m,c,e,k,i,n only apply to geographic regions specified in degrees)\n");
	fprintf (stderr, "\t   Append = to adjust the domain to fit the increment [Default adjusts increment to fit domain].\n");
	fprintf (stderr, "\t   Alternatively, specify number of nodes by appending +. Then, the increments are calculated\n");
	fprintf (stderr, "\t   from the given domain and node-registration settings (see Appendix B for details).\n");
	fprintf (stderr, "\t   Note: If -R<grdfile> was used the increments were set as well; use -I to override.\n");
}

void GMT_fill_syntax (char option, char *string)
{
	if (string[0] == ' ') fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c<fill> %s Specify <fill> as one of:\n", option, string);
	fprintf (stderr, "\t   1) <gray> or <red>/<green>/<blue>, all in the range 0-255;\n");
	fprintf (stderr, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	fprintf (stderr, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	fprintf (stderr, "\t   4) any valid color name;\n");
	fprintf (stderr, "\t   5) P|p<dpi>/<pattern>[:F<color>B<color>], with <dpi> of pattern, <pattern> from 1-90 or a filename,\n");
	fprintf (stderr, "\t      optionally add fore/background colors (use - for transparency).\n");
}

void GMT_pen_syntax (char option, char *string)
{
	if (string[0] == ' ') fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c %s\n", option, string);
	fprintf (stderr, "\t   <pen> is a comma-separated list of optional <width>[cipm], <color>, and <texture>[cipm]\n");
	fprintf (stderr, "\t   <width> >= 0.0, or a pen name: faint, default, or {thin, thick, fat}[er|est], obese.\n");
	fprintf (stderr, "\t   <color> = (1) <gray> or <red>/<green>/<blue>, all in the range 0-255,\n");
	fprintf (stderr, "\t             (2) <c>/<m>/<y>/<k> in 0-100%% range,\n");
	fprintf (stderr, "\t             (3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1,\n");
	fprintf (stderr, "\t             (4) any valid color name.\n");
	fprintf (stderr, "\t   <texture> = (1) pattern of dashes (-) and dots (.) which will be scaled by pen width.\n");
	fprintf (stderr, "\t               (2) a for d(a)shed or o for d(o)tted lines, scaled by pen width.\n");
	fprintf (stderr, "\t               (3) <pattern>:<offset>; <pattern> holds lengths of lines and gaps separated\n");
	fprintf (stderr, "\t                   by underscores and <offset> is a phase offset.\n");
	fprintf (stderr, "\t   If no unit is appended, then dots-per-inch is assumed [current dpi = %ld].\n", gmtdefs.dpi);
}

void GMT_rgb_syntax (char option, char *string)
{
	if (string[0] == ' ') fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c<color> %s Specify <color> as one of:\n", option, string);
	fprintf (stderr, "\t   1) <gray> or <red>/<green>/<blue>, all in the range 0-255;\n");
	fprintf (stderr, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	fprintf (stderr, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	fprintf (stderr, "\t   4) any valid color name.\n");
}

void GMT_mapscale_syntax (char option, char *string)
{
	if (string[0] == ' ') fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c %s\n", option, string);
	fprintf (stderr, "\t   Use -%cx to specify Cartesian coordinates instead.  Scale is calculated at latitude <slat>;\n", option);
	fprintf (stderr, "\t   optionally give longitude <slon> [Default is central longitude].  <length> is in km [Default]\n");
	fprintf (stderr, "\t   or [nautical] miles if [n] m is appended.  -%cf draws a \"fancy\" scale [Default is plain].\n", option);
	fprintf (stderr, "\t   By default, the label is set to the distance unit and placed on top [+jt].  Use the +l<label>\n");
	fprintf (stderr, "\t   and +j<just> mechanisms to specify another label and placement (t,b,l,r).  +u sets the label as a unit.\n");
	fprintf (stderr, "\t   Append +p<pen> and/or +f<fill> to draw/paint a rectangle behind the scale [no rectangle]\n");
}

void GMT_GSHHS_syntax (char option, char *string)
{
	if (string[0] == ' ') fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c %s\n", option, string);
	fprintf (stderr, "\t   Features smaller than <min_area> (in km^2) or of levels (0-4) outside the min-max levels\n");
	fprintf (stderr, "\t   will be skipped [0/4 (4 means lake inside island inside lake)].\n");
	fprintf (stderr, "\t   Append +r to only get riverlakes from level 2, or +l to only get lakes [both].\n");
	fprintf (stderr, "\t   Append +p<percent> to exclude features whose size is < <percent>%% of the full-resolution feature [use all].\n");
}

void GMT_maprose_syntax (char option, char *string)
{
	if (string[0] == ' ') fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);
	fprintf (stderr, "\t-%c %s\n", option, string);
	fprintf (stderr, "\t   Use -%cx to specify Cartesian coordinates instead.  -Tf draws a \"fancy\" rose [Default is plain].\n", option);
	fprintf (stderr, "\t   Give rose <diameter> and optionally the west, east, south, north labels desired [W,E,S,N].\n");
	fprintf (stderr, "\t   For fancy rose, specify as <info> the kind you want: 1 draws E-W, N-S directions [Default],\n");
	fprintf (stderr, "\t   2 adds NW-SE and NE-SW, while 3 adds WNW-ESE, NNW-SSE, NNE-SSW, and ENE-WSW.\n");
	fprintf (stderr, "\t   For Magnetic compass rose, specify -%cm.  Use the optional <info> = <dec>/<dlabel> (where <dec> is\n", option);
	fprintf (stderr, "\t   the magnetic declination and <dlabel> is a label for the magnetic compass needle) to plot\n");
	fprintf (stderr, "\t   directions to both magnetic and geographic north [Default is just geographic].\n");
	fprintf (stderr, "\t   If the North label = \'*\' then a north star is plotted instead of the label.\n");
	fprintf (stderr, "\t   Append +<gints>/<mints> to override default annotation/tick interval(s) [10/5/1/30/5/1].\n");
}

void GMT_syntax (char option)
{
	/* The function print to stderr the syntax for the option indicated by
	 * the variable <option>.  Only the common parameter options are covered
	 */
	
	char *u;

	u = GMT_unit_names[gmtdefs.measure_unit];

	fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option.  Correct syntax:\n", GMT_program, option);

	switch (option) {

		case 'B':	/* Tickmark option */

			fprintf (stderr, "\t-B[p|s][a|f|g]<tick>[m][l|p][:\"label\":][:,\"unit\":][/.../...]:.\"Title\":[W|w|E|e|S|s|N|n][Z|z]\n");
			break;

		case 'H':	/* Header */

			fprintf (stderr, "\t-H[n-header-records]\n");
			break;

		case 'J':	/* Map projection option */

			switch (project_info.projection) {
				case GMT_LAMB_AZ_EQ:
					fprintf (stderr, "\t-Ja<lon0>/<lat0>[/<horizon>]/<scale> OR -JA<lon0>/<lat0>[/<horizon>]/<width>\n");
					fprintf (stderr, "\t  <horizon> is distance from center to perimeter (<= 180, default 90)\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ALBERS:
					fprintf (stderr, "\t-Jb<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JB<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CASSINI:
					fprintf (stderr, "\t-Jc<lon0>/<lat0><scale> OR -JC<lon0>/<lat0><width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree ,or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_STEREO:
					fprintf (stderr, "\t-Jcyl_stere/[<lon0>/[<lat0>/]]<scale> OR -JCyl_stere/[<lon0>/[<lat0>/]]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECONIC:
					fprintf (stderr, "\t-Jd<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JD<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_AZ_EQDIST:
					fprintf (stderr, "\t-Je<lon0>/<lat0>[/<horizon>]/<scale> OR -JE<lon0>/<lat0>[/<horizon>/<width>\n");
					fprintf (stderr, "\t  <horizon> is distance from center to perimeter (<= 180, default 180)\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GNOMONIC:
					fprintf (stderr, "\t-Jf<lon0>/<lat0>[/<horizon>]/<scale> OR -JF<lon0>/<lat0>[/<horizon>]/<width>\n");
					fprintf (stderr, "\t  <horizon> is distance from center to perimeter (< 90, default 60)\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ORTHO:
					fprintf (stderr, "\t-Jg<lon0>/<lat0>[/<horizon>]/<scale> OR -JG<lon0>/<lat0>[/<horizon>]/<width>\n");
					fprintf (stderr, "\t  <horizon> is distance from center to perimeter (<= 90, default 90)\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GENPER:
					fprintf (stderr, "\t-Jg<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale> OR\n\t-JG<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_HAMMER:
					fprintf (stderr, "\t-Jh[<lon0>/]<scale> OR -JH[<lon0>/]<width\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_SINUSOIDAL:
					fprintf (stderr, "\t-Ji[<lon0>/]<scale> OR -JI[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MILLER:
					fprintf (stderr, "\t-Jj[<lon0>/]<scale> OR -JJ[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT4:
					fprintf (stderr, "\t-Jkf[<lon0>/]<scale> OR -JKf[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT6:
					fprintf (stderr, "\t-Jk[s][<lon0>/]<scale> OR -JK[s][<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_LAMBERT:
					fprintf (stderr, "\t-Jl<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JL<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MERCATOR:
					fprintf (stderr, "\t-Jm[<lon0>/[<lat0>/]]<scale> OR -JM[<lon0>/[<lat0>/]]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ROBINSON:
					fprintf (stderr, "\t-Jn[<lon0>/]<scale> OR -JN[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_OBLIQUE_MERC:
					fprintf (stderr, "\t-Jo[a]<lon0>/<lat0>/<azimuth>/<scale> OR -JO[a]<lon0>/<lat0>/<azimuth>/<width>\n");
					fprintf (stderr, "\t-Jo[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<scale> OR -JO[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<width>\n");
					fprintf (stderr, "\t-Joc<lon0>/<lat0>/<lonp>/<latp>/<scale> OR -JOc<lon0>/<lat0>/<lonp>/<latp>/<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/oblique degree, or use <width> in %s\n", u, u);
					break;
				case GMT_WINKEL:
					fprintf (stderr, "\t-Jr[<lon0>/]<scale> OR -JR[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLYCONIC:
					fprintf (stderr, "\t-Jpoly/[<lon0>/[<lat0>/]]<scale> OR -JPoly/[<lon0>/[<lat0>/]]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQDIST:
					fprintf (stderr, "\t-Jq[<lon0>/[<lat0>/]]<scale> OR -JQ[<lon0>/[<lat0>/]]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_STEREO:
					fprintf (stderr, "\t-Js<lon0>/<lat0>[/<horizon>]/<scale> OR -JS<lon0>/<lat0>[/<horizon>]/<width>\n");
					fprintf (stderr, "\t  <horizon> is distance from center to perimeter (< 180, default 90)\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx>, <lat>/<1:xxxx>, or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_TM:
					fprintf (stderr, "\t-Jt<lon0>/[<lat0>/]<scale> OR -JT<lon0>/[<lat0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_UTM:
					fprintf (stderr, "\t-Ju<zone>/<scale> OR -JU<zone>/<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					fprintf (stderr, "\t  <zone is A, B, 1-60[w/ optional C-X except I, O], Y, Z\n");
					break;
				case GMT_VANGRINTEN:
					fprintf (stderr, "\t-Jv<lon0>/<scale> OR -JV[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MOLLWEIDE:
					fprintf (stderr, "\t-Jw[<lon0>/]<scale> OR -JW[<lon0>/]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQ:
					fprintf (stderr, "\t-Jy[<lon0>/[<lat0>/]]<scale> OR -JY[<lon0>/[<lat0>/]]<width>\n");
					fprintf (stderr, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLAR:
					fprintf (stderr, "\t-Jp[a]<scale>[/<origin>][r] OR -JP[a]<width>[/<origin>][r]\n");
					fprintf (stderr, "\t  <scale> is %s/units, or use <width> in %s\n", u, u);
					fprintf (stderr, "\t  Optionally, prepend a for azimuths, append theta as origin [0],\n");
					fprintf (stderr, "\t  or append r to reverse radial coordinates.\n");
				case GMT_LINEAR:
					fprintf (stderr, "\t-Jx<x-scale|width>[d|l|p<power>|t|T][/<y-scale|height>[d|l|p<power>|t|T]], scale in %s/units\n", u);
					fprintf (stderr, "\t-Jz<z-scale>[l|p<power>], scale in %s/units\n", u);
					fprintf (stderr, "\tUse / to specify separate x/y scaling (e.g., -Jx0.5/0.3.).  Not allowed with 1:xxxxx\n");
					fprintf (stderr, "\tUse -JX (and/or -JZ) to give axes lengths rather than scales\n");
					break;
				default:
					fprintf (stderr, "\tProjection not recognized!\n");
					break;
			}
			break;

		case 'R':	/* Region option */

			fprintf (stderr, "\t-R<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>]\n");
			fprintf (stderr, "\tAppend r if giving lower left and upper right coordinates\n");
			break;

		case 'U':	/* Set time stamp option */

			fprintf (stderr, "\t-U[<just>/<dx>/<dy>/][c|<label>], c will plot command line.\n");
			break;

		case ':':	/* lon/lat vs lat/lon i/o option  */

			fprintf (stderr, "\t-:[i|o], i for input, o for output [Default is both].\n");
			fprintf (stderr, "\t   Swap 1st and 2nd column on input and/or output.\n");
			break;

		case 'b':	/* Binary i/o option  */

			fprintf (stderr, "\t-b[i|o][s[<n>]|d[<n>]|c[<var1>/<var2>/...]], i for input, o for output [Default is both].\n");
			fprintf (stderr, "\t   Use s or d for single or double precision [Default is double precision]\n");
			fprintf (stderr, "\t   and append the number of data columns (for input only).\n");
			fprintf (stderr, "\t   Use c for netCDF file and optionally append variable names.\n");
			break;

		case 'c':	/* Set number of plot copies option */

			fprintf (stderr, "\t-c<copies>, copies is number of copies\n");
			break;

		case 'f':	/* Column information option  */

			fprintf (stderr, "\t-f[i|o]<colinfo>, i for input, o for output [Default is both].\n");
			fprintf (stderr, "\t   <colinfo> is <colno|colrange>u, where column numbers start at 0\n");
			fprintf (stderr, "\t   a range is given as <first>-<last>, e.g., 2-5., u is type:\n");
			fprintf (stderr, "\t   t: relative time, T: absolute time, f: floating point,\n");
			fprintf (stderr, "\t   x: longitude, y: latitude, g: geographic coordinate.\n");
			break;

		default:
			break;
	}
}

void GMT_default_error (char option) {
	fprintf (stderr, "%s: GMT SYNTAX ERROR:  Unrecognized option -%c\n", GMT_program, option);
}

GMT_LONG GMT_sort_options (int argc, char **argv, char *order) {
	/* GMT_sort_options reorganizes the arguments on the command line
	 * according to the order specified in the character string "order"
	 * All options starting with "-" can be reorganised.
	 * For example, when "order" is "Vf:", the all options -V, -f, and
	 * -: will come first, in that order (if they are used),
	 * followed by all other arguments in their original order.
	 * ? indicates any option; * stands for any non-option.
	 */

	GMT_LONG i, j, k, arg1 = 1;
	GMT_LONG success;
	char *p;

	for (i = 0; order[i]; i++) {
		for (j = arg1; j < argc; j++) {
			success = (argv[j][0] == '-') ? (order[i] == '?' || order[i] == argv[j][1]) : (order[i] == '*');
			if (success) {
				p = argv[j];
				for (k = j; k > arg1; k--) argv[k] = argv[k-1];
				argv[arg1] = p;
				arg1++;
			}
		}
	}
	arg1--;
	return (arg1);
}

GMT_LONG GMT_get_common_args (char *item, double *w, double *e, double *s, double *n)
{	/* Just for backwards compatibility */
	return (GMT_parse_common_options (item, w, e, s, n));
}

GMT_LONG GMT_parse_common_options (char *item, double *w, double *e, double *s, double *n)
{
	/* GMT_parse_common_options interprets the command line for the common, unique options
	 * -B, -H, -J, -K, -O, -P, -R, -U, -V, -X, -Y, -b, -c, -f, -g, -m, -:, -
	 */

	GMT_LONG i, error = 0, j_type, opt;
	char processed[256];

	memset ((void *)processed, 0, (size_t)256);	/* All set to 0 for starters */
	opt = (GMT_LONG)item[1];

	switch (opt) {
		case '\0':
			if (processed[opt]) fprintf (stderr, "%s: Warning: Option - given more than once\n", GMT_program);
			GMT->common->synopsis.active = processed[opt] = TRUE;
			GMT_give_synopsis_and_exit = TRUE;
			break;

		case 'B':
			GMT->common->B.active = TRUE;
			switch (item[2]) {	/* Check for -B[p] and -Bs */
				case 's':
					if (processed[opt] & 2) {
						fprintf (stderr, "%s: Error: Option -Bs given more than once\n", GMT_program);
						error++;
					}
					processed[opt] |= 2;
					break;
				default:
					if (processed[opt] & 1) {
						fprintf (stderr, "%s: Error: Option -B[p] given more than once\n", GMT_program);
						error++;
					}
					processed[opt] |= 1;
					break;
			}
			if (error == 0) {
				error += (i = GMT_parse_B_option (&item[2]));
				if (i) GMT_syntax ('B');
			}
			break;

		case 'H':
			GMT->common->H.active[0] = TRUE;
			if (processed[opt]) {
				fprintf (stderr, "%s: Error: Option -H given more than once\n", GMT_program);
				error++;
			}
			else {
				processed[opt] = TRUE;
				error += GMT_parse_H_option (item);
			}
			break;
		case 'J':
			GMT->common->J.active = TRUE;
			j_type = (item[2] == 'Z' || item[2] == 'z') ? 2 : 1;
			if (processed[opt] & j_type) {
				fprintf (stderr, "%s: Error: Option -J given more than once\n", GMT_program);
				error++;
			}
			else {
				processed[opt] |= j_type;
				error += (i = GMT_parse_J_option (&item[2]));
				if (i) GMT_syntax ('J');
			}
			break;
		case 'K':
			if (processed[opt]) fprintf (stderr, "%s: Warning: Option -K given more than once\n", GMT_program);
			GMT->common->K.active = processed[opt] = TRUE;
			GMT_ps.last_page = FALSE;
			break;
		case 'O':
			if (processed[opt]) fprintf (stderr, "%s: Warning: Option -O given more than once\n", GMT_program);
			GMT->common->O.active = processed[opt] = TRUE;
			GMT_ps.overlay = TRUE;
			break;
		case 'P':
			if (processed[opt]) fprintf (stderr, "%s: Warning: Option -P given more than once\n", GMT_program);
			GMT->common->P.active = processed[opt] = TRUE;
			GMT_ps.portrait = TRUE;
			break;
		case 'R':
			GMT->common->R.active = TRUE;
			if (processed[opt]) {
				fprintf (stderr, "%s: Error: Option -R given more than once\n", GMT_program);
				error++;
				break;
			}
			else {
				processed[opt] = TRUE;
				error += GMT_parse_R_option (item, w, e, s, n);
			}
			break;
		case 'U':
			GMT->common->U.active = TRUE;
			if (processed[opt]) {
				fprintf (stderr, "%s: Error: Option -U given more than once\n", GMT_program);
				error++;
				break;
			}
			else {
				processed[opt] = TRUE;
				error += GMT_parse_U_option (item);
			}
			break;
		case 'V':
			if (processed[opt]) fprintf (stderr, "%s: Warning: Option -V given more than once\n", GMT_program);
			GMT->common->V.active = processed[opt] = TRUE;
			gmtdefs.verbose = (item[2] == 'l') ? 2 : TRUE;	/* -Vl is long verbose */
			GMT_ps.verbose = TRUE;
			break;
		case 'X':
		case 'x':
			GMT->common->X.active = TRUE;
			if (processed['X']) {
				fprintf (stderr, "%s: Error: Option -%c given more than once\n", GMT_program, item[1]);
				error++;
				break;
			}
			else {
				processed['X'] = TRUE;
				i = 2;
				if (item[2] == 'r') i++;	/* Relative mode is default anyway */
				if (item[2] == 'a') i++, GMT_x_abs = TRUE;
				if (item[2] == 'c')
					project_info.x_off_supplied = 2;	/* Must center in map_setup */
				else {
					GMT_ps.x_origin = GMT_convert_units (&item[i], GMT_INCH);
					project_info.x_off_supplied = TRUE;
				}
			}
			break;
		case 'Y':
		case 'y':
			GMT->common->Y.active = TRUE;
			if (processed['Y']) {
				fprintf (stderr, "%s: Error: Option -%c given more than once\n", GMT_program, item[1]);
				error++;
				break;
			}
			else {
				processed['Y'] = TRUE;
				i = 2;
				if (item[2] == 'r') i++;	/* Relative mode is default anyway */
				if (item[2] == 'a') i++, GMT_y_abs = TRUE;
				if (item[2] == 'c')
					project_info.y_off_supplied = 2;	/* Must center in map_setup */
				else {
					GMT_ps.y_origin = GMT_convert_units (&item[i], GMT_INCH);
					project_info.y_off_supplied = TRUE;
				}
			}
			break;
		case 'c':
			GMT->common->c.active = TRUE;
			if (processed[opt]) {
				fprintf (stderr, "%s: Error: Option -c given more than once\n", GMT_program);
				error++;
				break;
			}
			else {
				processed[opt] = TRUE;
				i = atoi (&item[2]);
				if (i < 1) {
					error++;
					GMT_syntax ('c');
				}
				else
					GMT_ps.n_copies = i;
			}
			break;
		case ':':	/* Toggle lon/lat - lat/lon */
			GMT->common->t.active = TRUE;
			if (processed[opt]) {
				fprintf (stderr, "%s: Error: Option -: given more than once\n", GMT_program);
				error++;
				break;
			}
			else {
				processed[opt] = TRUE;
				error += GMT_parse_t_option (item);
			}
			break;
		case 'b':	/* Binary i/o */
			GMT->common->b.active = processed[opt] = TRUE;
			i = GMT_parse_b_option (&item[2]);
			if (i) GMT_syntax ('b');
			error += i;
			break;
		case 'f':	/* Column type specifications */
			GMT->common->f.active = TRUE;
			if (processed[opt] > 3) {
				fprintf (stderr, "%s: Error: Option -f given more than once\n", GMT_program);
				error++;
				break;
			}
			else {
				switch (item[2]) {
					case 'i':
						processed[opt]++;
						break;
					case 'o':
						processed[opt] += 2;
						break;
					default:
						processed[opt] += 3;
						break;
				}
				i = GMT_parse_f_option (&item[2]);
				if (i) GMT_syntax ('f');
				error += i;
			}
			break;
		case 'g':	/* Gap detection settings */
			i = 0;
			if (item[2]) {
				GMT->common->g.active = TRUE;
				if (GMT_parse_g_option (&item[2])) {
					fprintf (stderr, "%s: Error: Bad -g option arguments\n", GMT_program);
					i = 1;
				}
			}
			else {
				fprintf (stderr, "%s: Error: No arguments given to -g option\n", GMT_program);
				i = 1;
			}
			if (i) GMT_syntax ('g');
			error += i;
			break;
		case 'M':
		case 'm':
			GMT_parse_m_option (&item[2]);
			break;
			
		default:	/* Should never get here, but... */
			error++;
			fprintf (stderr, "GMT: Warning: bad case in GMT_parse_common_options (ignored)\n");
			break;
	}

	/* -g gap checking implies -mo if not already set */
	
	if (GMT->common->g.active) GMT_io.multi_segments[GMT_OUT] = TRUE;
	
	/* First check that -X -Y was done correctly */

	if ((project_info.x_off_supplied && project_info.y_off_supplied) && GMT_x_abs != GMT_y_abs) {
		error++;
		fprintf (stderr, "%s: GMT SYNTAX ERROR: -X -Y must both be absolute or relative\n", GMT_program);
	}
	if (GMT_x_abs && GMT_y_abs) GMT_ps.absolute = TRUE;

	return (error);
}

GMT_LONG GMT_parse_H_option (char *item) {
	GMT_LONG i, j, error = 0;

	/* Parse the -H option.  Full syntax:  -H[i][<nrecs>] */
	j = 2;
	if (item[j] == 'i') j = 3;	/* -Hi[nrecs] given */
	if (item[j]) {
		i = atoi (&item[j]);
		if (i < 0) {
			GMT_syntax ('H');
			error++;
		}
		else
			GMT_io.n_header_recs = i;
	}
	if (j == 2)	/* Both in and out may have header records */
		GMT_io.io_header[GMT_IN] = GMT_io.io_header[GMT_OUT] = (GMT_io.n_header_recs > 0);
	else		/* Only input should have header records */
		GMT_io.io_header[GMT_IN] = (GMT_io.n_header_recs > 0);
	return (error);
}

GMT_LONG GMT_parse_R_option (char *item, double *w, double *e, double *s, double *n) {
	GMT_LONG i, icol, pos, got, col_type[2], expect_to_read, error = 0;
	char text[BUFSIZ], string[BUFSIZ];
	double *p[6];

	/* Parse the -R option.  Full syntax:  -R<grdfile> or -Rg or -Rd or -R[g|d]w/e/s/n[/z0/z1][r] */

	if ((item[2] == 'g' || item[2] == 'd') && item[3] == '\0') {	/* Check -Rd|g separately in case user has files called d or g */
		if (item[2] == 'g')	/* -Rg is shorthand for -R0/360/-90/90 */
			*w = project_info.w = 0.0, *e = project_info.e = 360.0;
		else			/* -Rd is shorthand for -R-180/+180/-90/90 */
			*w = project_info.w = -180.0, *e = project_info.e = 180.0;
		*s = project_info.s = -90.0;	*n = project_info.n = +90.0;
		GMT_io.in_col_type[0] = GMT_IS_LON, GMT_io.in_col_type[1] = GMT_IS_LAT;
		project_info.degree[0] = project_info.degree[1] = TRUE;
		project_info.region_supplied = TRUE;
		return (0);
	}
	if (!GMT_access (&item[2], R_OK)) {	/* Gave a readable file, presumably a grid */		
		GMT_err_fail (GMT_read_grd_info (&item[2], &GMT_grd_info.grd), &item[2]);
		*w = project_info.w = GMT_grd_info.grd.x_min; *e = project_info.e = GMT_grd_info.grd.x_max;
		*s = project_info.s = GMT_grd_info.grd.y_min; *n =project_info.n = GMT_grd_info.grd.y_max;
		project_info.z_bottom = GMT_grd_info.grd.z_min;	project_info.z_top = GMT_grd_info.grd.z_max;
		GMT_grd_info.active = project_info.region_supplied = TRUE;
		return (0);
	}
	if (item[2] == 'g' || item[2] == 'd') {	/* Here we have a region appended to -Rd|g */
		GMT_io.in_col_type[0] = GMT_IS_LON, GMT_io.in_col_type[1] = GMT_IS_LAT;
		project_info.degree[0] = project_info.degree[1] = TRUE;
		project_info.region_supplied = TRUE;
		strcpy (string, &item[3]);
	}
	else
		strcpy (string, &item[2]);

	/* Now decode the string */
	
	p[0] = w;	p[1] = e;	p[2] = s;	p[3] = n;
	p[4] = &project_info.z_bottom;	p[5] = &project_info.z_top;
	col_type[0] = col_type[1] = 0;
	project_info.region_supplied = TRUE;
	if (string[strlen(string)-1] == 'r') {
		project_info.region = FALSE;
		string[strlen(string)-1] = '\0';	/* Remove the trailing r so GMT_scanf will work */
	}
	i = pos = 0;
	while ((GMT_strtok (string, "/", &pos, text))) {
		if (i > 5) {
			error++;
			GMT_syntax ('R');
			return (error);		/* Have to break out here to avoid segv on *p[6]  */
		}
		/* Figure out what column corresponds to a token to get in_col_type flag  */
		if (i > 3)
			icol = 2;
		else if (!project_info.region)
			icol = i%2;
		else
			icol = i/2;
		if (icol < 2 && gmtdefs.xy_toggle[GMT_IN]) icol = 1 - icol;	/* col_types were swapped */
		/* If column is either RELTIME or ABSTIME, use ARGTIME */
		if (GMT_io.in_col_type[icol] == GMT_IS_UNKNOWN) {	/* No -J or -f set, proceed with caution */
			got = GMT_scanf_arg (text, GMT_io.in_col_type[icol], p[i]);
			if (got & GMT_IS_GEO)
				GMT_io.in_col_type[icol] = got, project_info.degree[icol] = TRUE;
			else if (got & GMT_IS_RATIME)
				GMT_io.in_col_type[icol] = got, project_info.xyz_projection[icol] = GMT_TIME;
		}
		else {	/* Things are set, do or die */
			expect_to_read = (GMT_io.in_col_type[icol] & GMT_IS_RATIME) ? GMT_IS_ARGTIME : GMT_io.in_col_type[icol];
			error += GMT_verify_expectations (expect_to_read, GMT_scanf (text, expect_to_read, p[i]), text);
		}
		if (error) {
			GMT_syntax ('R');
			return (error);
		}

		i++;
	}
	if (!project_info.region) d_swap (*p[2], *p[1]);	/* So w/e/s/n makes sense */
	if (i < 4 || i > 6 || (GMT_check_region (*p[0], *p[1], *p[2], *p[3]) || (i == 6 && *p[4] >= *p[5]))) {
		error++;
		GMT_syntax ('R');
	}
	project_info.w = *p[0];	project_info.e = *p[1];	/* This will probably be reset by GMT_map_setup */
	project_info.s = *p[2];	project_info.n = *p[3];

	return (error);
}

void GMT_check_lattice (double *x_inc, double *y_inc, GMT_LONG *pixel, GMT_LONG *active)
{	/* Uses provided settings to initialize the lattice settings from
	 * the -R<grdfile> if it was given; else it does nothing.
	 */
	if (!GMT_grd_info.active) return;	/* -R<grdfile> was not used; use existing settings */
	
	/* Here, -R<grdfile> was used and we will use the settings supplied by the grid file (unless overridden) */
	if (!active || *active == FALSE) {	/* -I not set separately */
		*x_inc = GMT_grd_info.grd.x_inc;
		*y_inc = GMT_grd_info.grd.y_inc;
	}
	if (pixel) {	/* An pointer not NULL was passed that indicates grid registration */
		/* If a -F like option was set then toggle grid setting, else use grid setting */
		*pixel = (*pixel) ? !GMT_grd_info.grd.node_offset : GMT_grd_info.grd.node_offset;
	}
	if (active) *active = TRUE;	/* When 4th arg is not NULL it is set to TRUE (for Ctrl->active args) */
}

GMT_LONG GMT_parse_U_option (char *item) {
	GMT_LONG i, n = 0, n_slashes, error = 0;
	char txt_j[GMT_LONG_TEXT], txt_x[GMT_LONG_TEXT], txt_y[GMT_LONG_TEXT];

	/* Parse the -U option.  Full syntax:  -U[<just>/<dx>/<dy>/][c|<label>] */

	GMT_ps.unix_time = TRUE;
	for (i = n_slashes = 0; item[i]; i++) {
		if (item[i] == '/') n_slashes++;	/* Count slashes to detect <just>/<dx>/<dy>/ presence */
	}
	if (n_slashes >= 2) {	/* Probably gave -U<just>/<dx>/<dy>[/<string>] */
		n = sscanf (item, "%[^/]/%[^/]/%[^/]/%[^\n]", txt_j, txt_x, txt_y, GMT_ps.unix_time_label);
		if ((i = GMT_just_decode (&txt_j[2], GMT_ps.unix_time_just)) < 0) {
			/* Garbage before first slash: we simply have -U<string> */
			strcpy (GMT_ps.unix_time_label, &item[2]);
		}
		else {
			GMT_ps.unix_time_just = i;
			GMT_ps.unix_time_pos[GMT_X] = GMT_convert_units (txt_x, GMT_INCH);
			GMT_ps.unix_time_pos[GMT_Y] = GMT_convert_units (txt_y, GMT_INCH);
		}
	}
	else
		strcpy (GMT_ps.unix_time_label, &item[2]);
	if ((item[2] == '/' && n_slashes == 1) || (item[2] == '/' && n_slashes >= 2 && n < 2)) {
		error++;
		GMT_syntax ('U');
	}
	return (error);
}

GMT_LONG GMT_parse_t_option (char *item) {
	GMT_LONG error = 0;
	/* Parse the -: option.  Full syntax:  -:[i|o] */
	switch (item[2]) {
		case 'i':	/* Toggle on input data only */
			gmtdefs.xy_toggle[GMT_IN] = TRUE;
			break;
		case 'o':	/* Toggle on output data only */
			gmtdefs.xy_toggle[GMT_OUT] = TRUE;
			break;
		case '\0':	/* Toggle both input and output data */
			gmtdefs.xy_toggle[GMT_IN] = gmtdefs.xy_toggle[GMT_OUT] = TRUE;
			break;
		default:
			GMT_syntax (':');
			error++;
			break;

	}
	return (error);
}

GMT_LONG GMT_parse_g_option (char *txt)
{
	GMT_LONG i, k = 0, c;
	/* Process the GMT gap detection option for parameters */
	/* Syntax, e.g., -g[x|X|y|Y|d|D|[<col>]z][+|-]<gap>[d|e|m|n|k|i|c|p] or -ga */
	
	if ((i = GMT->common->g.n_methods) == GMT_N_GAP_METHODS) {
		fprintf (stderr, "%s: ERROR: Cannot specify more than %d gap criteria\n", GMT_program, GMT_N_GAP_METHODS);
		return (1);
	}
	
	if (txt[0] == 'a') {	/* For multiple criteria, specify that all criteria be met [default is any] */
		k++;
		GMT->common->g.match_all = TRUE;
		if (!txt[k]) return (1);	/* Just a single -ga */
	}
	switch (txt[k]) {	/* Determine method used for gap detection */
		case 'x':	/* Difference in user's x-coordinates used for test */
			GMT->common->g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common->g.col[i] = GMT_X;
			break;
		case 'X':	/* Difference in user's mapped x-coordinates used for test */
			GMT->common->g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_MAP_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			GMT->common->g.col[i] = GMT_X;
			break;
		case 'y':	/* Difference in user's y-coordinates used for test */
			GMT->common->g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common->g.col[i] = GMT_Y;
			break;
		case 'Y':	/* Difference in user's mapped y-coordinates used for test */
			GMT->common->g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_MAP_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			GMT->common->g.col[i] = GMT_Y;
			break;
		case 'd':	/* Great circle (if geographic data) or Cartesian distance used for test */
			GMT->common->g.method[i] = (GMT_io.in_col_type[GMT_X] == GMT_IS_LON && GMT_io.in_col_type[GMT_Y] == GMT_IS_LAT) ? GMT_GAP_IN_GDIST : GMT_GAP_IN_CDIST;
			GMT->common->g.col[i] = -1;
			break;
		case 'D':	/* Cartesian mapped distance used for test */
			GMT->common->g.method[i] = GMT_GAP_IN_PDIST;
			GMT->common->g.col[i] = -1;
			break;
		case 'z':	/* Difference in user's z-coordinates used for test */
			GMT->common->g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common->g.col[i] = 2;
			break;
		case '1':	/* Difference in a specified column's coordinates used for test */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
			c = k;
			while (txt[k] && isdigit ((int)txt[k])) k++;	/* Skip past until we find z */
			if (txt[k] != 'z') {
				fprintf (stderr, "%s: GMT ERROR: Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", GMT_program, txt[k]);
				return (1);
			}
			GMT->common->g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common->g.col[i] = atoi (&txt[c]);
			break;
		default:
			fprintf (stderr, "%s: GMT ERROR: Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", GMT_program, txt[0]);
			return (1);
			break;
	}
	switch (GMT->common->g.method[i]) {
		case GMT_NEGGAP_IN_COL:
			GMT->common->g.get_dist[i] = (PFD) GMT_neg_col_dist;
			break;
		case GMT_POSGAP_IN_COL:
			GMT->common->g.get_dist[i] = (PFD) GMT_pos_col_dist;
			break;
		case GMT_ABSGAP_IN_COL:
			GMT->common->g.get_dist[i] = (PFD) GMT_abs_col_dist;
			break;
		case GMT_NEGGAP_IN_MAP_COL:
			GMT->common->g.get_dist[i] = (PFD) GMT_neg_col_map_dist;
			break;
		case GMT_POSGAP_IN_MAP_COL:
			GMT->common->g.get_dist[i] = (PFD) GMT_pos_col_map_dist;
			break;
		case GMT_ABSGAP_IN_MAP_COL:
			GMT->common->g.get_dist[i] = (PFD) GMT_abs_col_map_dist;
			break;
		case GMT_GAP_IN_GDIST:
			GMT->common->g.get_dist[i] = (PFD) GMT_xy_true_dist;
			break;
		case GMT_GAP_IN_CDIST:
			GMT->common->g.get_dist[i] = (PFD) GMT_xy_cart_dist;
			break;
		case GMT_GAP_IN_PDIST:
			GMT->common->g.get_dist[i] = (PFD) GMT_xy_map_dist;
			break;
		default:
			break;	/* Already set, or will be reset below  */
	}
	k++;	/* Skip to start of gap value */
	if (txt[k] == '-' || txt[k] == '+') k++;	/* SKip sign */
	if ((GMT->common->g.gap[i] = atof (&txt[k])) == 0.0) {
		fprintf (stderr, "%s: GMT ERROR: Gap value must be non-zero\n", GMT_program);
		return (1);
	}
	if (GMT->common->g.method[i] == GMT_GAP_IN_GDIST) {	/* Convert any gap given to meters */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'd':	/* Degrees, reset pointer */
				GMT->common->g.get_dist[i] = (PFD) GMT_xy_deg_dist;
				GMT->common->g.method[i] = GMT_GAP_IN_DDIST;
				break;
			case 'k':	/* Km  */
				GMT->common->g.gap[i] *= 1000.0;
				break;
			case 'm':	/* Miles */
				GMT->common->g.gap[i] *= METERS_IN_A_MILE;
				break;
			case 'n':	/* Nautical miles */
				GMT->common->g.gap[i] *= METERS_IN_A_NAUTICAL_MILE;
				break;
			default:	/* E.g., meters or junk */
				break;
		}
	}
	else if (GMT->common->g.method[i] == GMT_GAP_IN_PDIST){	/* Cartesian plot distance stuff */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'c':	/* cm*/
				GMT->common->g.gap[i] /= 2.54;
				break;
			case 'p':	/* Points */
				GMT->common->g.gap[i] /= 72.0;
				break;
			case 'm':	/* m */
				GMT->common->g.gap[i] /= 0.0254;
				break;
			default:	/* E.g., inch or junk */
				break;
		}
	}
	if ((GMT->common->g.col[i] + 1) > GMT->common->g.n_col) GMT->common->g.n_col = GMT->common->g.col[i] + 1;	/* Needed when checking since it may otherwise not be read */
	GMT->common->g.n_methods++;
	return (0);
}

double GMT_neg_col_dist (GMT_LONG col)
{	/* Compute reverse col-separation before mapping */
	return (GMT_prev_rec[col] - GMT_curr_rec[col]);
}
	
double GMT_pos_col_dist (GMT_LONG col)
{	/* Compute forward col-separation before mapping */
	return (GMT_curr_rec[col] - GMT_prev_rec[col]);
}
	
double GMT_abs_col_dist (GMT_LONG col)
{	/* Compute absolute col-separation before mapping */
	return (fabs (GMT_curr_rec[col] - GMT_prev_rec[col]));
}
	
double GMT_neg_col_map_dist (GMT_LONG col)
{	/* Compute reverse col-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][0] - X[col][1]);
}
	
double GMT_pos_col_map_dist (GMT_LONG col)
{	/* Compute forward col-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][1] - X[col][0]);
}
	
double GMT_abs_col_map_dist (GMT_LONG col)
{	/* Compute forward col-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (fabs (X[col][1] - X[col][0]));
}
	
double GMT_xy_map_dist (GMT_LONG col)
{	/* Compute point-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (GMT_cartesian_dist (X[GMT_X][0], X[GMT_Y][0], X[GMT_X][1], X[GMT_Y][1]));
}

double GMT_xy_deg_dist (GMT_LONG col)
{
	return (GMT_great_circle_dist (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y]));
	
}

double GMT_xy_true_dist (GMT_LONG col)
{
	return (GMT_great_circle_dist_meter (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y]));
	
}

double GMT_xy_cart_dist (GMT_LONG col)
{
	return (GMT_cartesian_dist (GMT_prev_rec[GMT_X], GMT_prev_rec[GMT_Y], GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y]));
	
}

GMT_LONG GMT_loaddefaults (char *file)
{
	GMT_LONG error = 0;
	char line[BUFSIZ], keyword[GMT_LONG_TEXT], value[GMT_LONG_TEXT];
	FILE *fp = NULL;

	if ((fp = fopen (file, "r")) == NULL) return (-1);

	GMT_force_resize = FALSE;	/* "Listen" for +<size> for ANNOT_FONT */

	/* Set up hash table */

	GMT_hash_init (keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);

	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments */
		GMT_chop (line);		/* Get rid of [\r]\n */
		if (line[0] == '\0') continue;	/* Skip Blank lines */

		keyword[0] = value[0] = '\0';	/* Initialize */
		sscanf (line, "%s = %[^\n]", keyword, value);

		error += GMT_setparameter (keyword, value);
	}

	fclose (fp);
	GMT_backwards_compatibility ();
	if (!strstr (GMT_program, "gmtset")) GMT_verify_encodings ();

	GMT_free_hash (keys_hashnode, GMT_N_KEYS);	/* Done with this for now */
	if (error) fprintf (stderr, "GMT:  %ld conversion errors in file %s!\n", error, file);

	return (0);
}

void GMT_setdefaults (int argc, char **argv)
{
	GMT_LONG j, k, p, error = 0;

	/* Set up hash table */

	GMT_hash_init (keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);

	GMT_got_frame_rgb = FALSE;	/* "Listen" for changes to basemap_frame RGB */

	j = 1;
	while (j < argc) {	/* j points to parameter, k to value */
		k = j + 1;
		if (strchr (argv[j], '=')) {	/* User forgot and gave parameter=value (1 word) */
			p = 0;
			while (argv[j][p] && argv[j][p] != '=') p++;
			if (argv[j][p] == '=') {
				argv[j][p] = '\0';
				error += GMT_setparameter (argv[j], &argv[j][p+1]);
				argv[j][p] = '=';
				k = j;
			}
			else {
				error++;
				break;
			}
		}
		else if (k >= argc) {	/* Ran out of arguments */
			error++;
		}
		else if (!strcmp (argv[k], "=")) {	/* User forgot and gave parameter = value */
			k++;
			if (k >= argc) {
				error++;
				break;
			}
			error += GMT_setparameter (argv[j], argv[k]);
		}
		else {
			if (k >= argc) {	/* Ran out of arguments, error */
				error++;
				break;
			}
			error += GMT_setparameter (argv[j], argv[k]);
		}
		j = k + 1;	/* Goto next parameter */
	}

	GMT_backwards_compatibility ();

	if (GMT_got_frame_rgb) {	/* Must enforce change of frame, tick, and grid pen rgb */
		memcpy ((void *)gmtdefs.frame_pen.rgb, (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)gmtdefs.tick_pen.rgb,  (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)gmtdefs.grid_pen[0].rgb,   (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)gmtdefs.grid_pen[1].rgb, (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
	}

	GMT_free_hash (keys_hashnode, GMT_N_KEYS);	/* Done with this for now  */
	if (error) fprintf (stderr, "%s:  %ld conversion errors\n", GMT_program, error);
}

void GMT_backwards_compatibility () {
	/* Convert old GMT 3.4 DEGREE_FORMAT settings to the new PLOT_DEGREE_FORMAT string and DEGREE_SYMBOL setting */
	/* Also to automatic scaling of font sizes relative to ANNOT_FONT_SIZE if given with leading + */

	char string[GMT_LONG_TEXT];
	GMT_LONG k;

	if (GMT_backward.got_old_plot_format && GMT_backward.got_new_plot_format) {	/* Got both old and new */
		fprintf (stderr, "%s: WARNING: Both old-style DEGREE_FORMAT and PLOT_DEGREE_FORMAT present in .gmtdefaults\n", GMT_program);
		fprintf (stderr, "%s: WARNING: PLOT_DEGREE_FORMAT overrides old DEGREE_FORMAT\n", GMT_program);
	}
	else if (GMT_backward.got_old_plot_format && !GMT_backward.got_new_plot_format) {	/* Must decode old DEGREE_FORMAT */
		memset ((void *)string, 0, (size_t)GMT_LONG_TEXT);
		k = gmtdefs.degree_format % 100;
		if (k == 0 || k == 4 || k == 6 || k == 8)	/* These were 0-360 values */
			strcpy (string, "+");
		else if (k >= 12 && k <= 17)			/* These were -360 to 0 numbers */
			strcpy (string, "-");
		/* else we do nothing and get -180/+180 */

		if ((k >= 4 && k <= 7) || k == 13 || k == 16)		/* Decimal degrees using D_FORMAT */
			strcat (string, "D");
		else if (( k >= 8 && k <= 11) || k == 14 || k == 17)	/* Degrees and decimal minutes - pick 1 decimal */
			strcat (string, "ddd:mm.x");
		else
			strcat (string, "ddd:mm:ss");			/* Standard dd mm ss */
		if (k == 2 || k == 10)					/* Abs value */
			strcat (string, "A");
		else if (k == 3 || k == 6 || k == 7 || k == 11 || (k >= 15 && k <= 17))	/* Append WESN */
			strcat (string, "F");
		strcpy (gmtdefs.plot_degree_format, string);
		fprintf (stderr, "%s: WARNING: DEGREE_FORMAT decoded (%ld) but is obsolete.  Please use PLOT_DEGREE_FORMAT (%s)\n", GMT_program, gmtdefs.degree_format, gmtdefs.plot_degree_format);
	}
	if (GMT_backward.got_old_degree_symbol && GMT_backward.got_new_degree_symbol) {	/* Got both old and new */
		fprintf (stderr, "%s: WARNING: Both old-style DEGREE_FORMAT and DEGREE_SYMBOL present in .gmtdefaults\n", GMT_program);
		fprintf (stderr, "%s: WARNING: DEGREE_SYMBOL overrides old DEGREE_FORMAT\n", GMT_program);
	}
	else if (GMT_backward.got_old_degree_symbol && !GMT_backward.got_new_degree_symbol) {	/* Must decode old DEGREE_FORMAT */
		fprintf (stderr, "%s: WARNING: DEGREE_FORMAT decoded but is obsolete.  Please use DEGREE_SYMBOL\n", GMT_program);
		if (gmtdefs.degree_format >= 1000)	/* No degree symbol */
			gmtdefs.degree_symbol = gmt_squote;
		else if (gmtdefs.degree_format >= 100)	/* Large degree symbol */
			gmtdefs.degree_symbol = gmt_degree;
	}
	if (GMT_backward.got_old_want_euro && GMT_backward.got_new_char_encoding) {	/* Got both old and new */
		fprintf (stderr, "%s: WARNING: Both old-style WANT_EURO_FONT and CHAR_ENCODING present in .gmtdefaults\n", GMT_program);
		fprintf (stderr, "%s: WARNING: CHAR_ENCODING overrides old WANT_EURO_FONT\n", GMT_program);
	}
	else if (GMT_backward.got_old_want_euro && GMT_backward.got_new_char_encoding)  {	/* Must decode old WANT_EURO_FONT */
		fprintf (stderr, "%s: WARNING: WANT_EURO_FONT decoded but is obsolete.  Please use CHAR_ENCODING\n", GMT_program);
		strncpy (gmtdefs.encoding.name, "Standard+", (size_t)GMT_TEXT_LEN);
		load_encoding (&gmtdefs.encoding);
	}

	if (GMT_force_resize) {	/* Adjust fonts and offsets and ticklengths relative to ANNOT_FONT_SIZE */
		gmtdefs.annot_font_size[1] = 16.0 * gmtdefs.annot_font_size[0] / 14.0;
		gmtdefs.label_font_size = 24.0 * gmtdefs.annot_font_size[0] / 14.0;
		gmtdefs.header_font_size = 36.0 * gmtdefs.annot_font_size[0] / 14.0;
		gmtdefs.annot_offset[0] = 0.075 * gmtdefs.annot_font_size[0] / 14.0;
		gmtdefs.tick_length = 0.075 * gmtdefs.annot_font_size[0] * copysign (1.0, gmtdefs.tick_length) / 14.0;
		gmtdefs.annot_offset[1] = 0.075 * gmtdefs.annot_font_size[1] / 14.0;
		gmtdefs.label_offset = 1.5 * fabs (gmtdefs.annot_offset[0]);
		gmtdefs.header_offset = 2.5 * fabs (gmtdefs.annot_offset[0]);
		gmtdefs.frame_width = 0.05 * gmtdefs.annot_font_size[0] / 14.0;
	}
}

GMT_LONG GMT_setparameter (char *keyword, char *value)
{
	GMT_LONG i, ival, case_val, pos;
	int rgb[3];
	GMT_LONG manual, eps, error = FALSE;
	char txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT], txt_c[GMT_LONG_TEXT], lower_value[BUFSIZ];
	double dval;

	if (!value) return (TRUE);		/* value argument missing */
	strncpy (lower_value, value, (size_t)BUFSIZ);	/* Get a lower case version */
	GMT_str_tolower (lower_value);

	case_val = GMT_hash_lookup (keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		case GMTCASE_ANNOT_MIN_ANGLE:
		case GMTCASE_ANOT_MIN_ANGLE:
			dval = atof (value);
			if (dval < 0.0)
				error = TRUE;
			else
				gmtdefs.annot_min_angle = dval;
			break;
		case GMTCASE_ANNOT_MIN_SPACING:
		case GMTCASE_ANOT_MIN_SPACING:
			if (value[0] == '-')	/* Negative */
				error = TRUE;
			else
				gmtdefs.annot_min_spacing = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_ANNOT_FONT_PRIMARY:
		case GMTCASE_ANOT_FONT_PRIMARY:
		case GMTCASE_ANNOT_FONT:
		case GMTCASE_ANOT_FONT:
			ival = GMT_font_lookup (value, GMT_font, GMT_N_FONTS);
			if (ival < 0 || ival >= GMT_N_FONTS)
				error = TRUE;
			else
				gmtdefs.annot_font[0] = ival;
			break;
		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY:
		case GMTCASE_ANOT_FONT_SIZE_PRIMARY:
		case GMTCASE_ANNOT_FONT_SIZE:
		case GMTCASE_ANOT_FONT_SIZE:
			if (value[0] == '+') GMT_force_resize = TRUE;	/* Turning on autoscaling of font sizes and ticklengths */
			if (value[0] != '+' && GMT_force_resize) GMT_annot_special = TRUE;	/* gmtset tries to turn off autoscaling - must report saved values but reset this one */
			dval = GMT_convert_units (value, 10+GMT_PT);
			if (dval > 0.0)
				gmtdefs.annot_font_size[0] = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_ANNOT_OFFSET_PRIMARY:
		case GMTCASE_ANOT_OFFSET_PRIMARY:
		case GMTCASE_ANNOT_OFFSET:
		case GMTCASE_ANOT_OFFSET:
			save_annot_offset[0] = gmtdefs.annot_offset[0] = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_BASEMAP_AXES:
			strcpy (gmtdefs.basemap_axes, value);
			for (i = 0; i < 4; i++) frame_info.side[i] = 0;	/* Otherwise we cannot unset default settings */
			for (i = 0; value[i]; i++) {
				switch (value[i]) {
					case 'W':	/* Upper case: Draw axis/ticks AND annotate */
						frame_info.side[3] = 2;
						break;
					case 'w':	/* Lower case: Draw axis/ticks only */
						frame_info.side[3] = 1;
						break;
					case 'E':
						frame_info.side[1] = 2;
						break;
					case 'e':
						frame_info.side[1] = 1;
						break;
					case 'S':
						frame_info.side[0] = 2;
						break;
					case 's':
						frame_info.side[0] = 1;
						break;
					case 'N':
						frame_info.side[2] = 2;
						break;
					case 'n':
						frame_info.side[2] = 1;
						break;
					case '-':	/* None */
						break;
					default:
						error = TRUE;
						break;
				}
			}
			break;
		case GMTCASE_BASEMAP_FRAME_RGB:
			i = (value[0] == '+') ? 1 : 0;	/* Plus means propagate color to frame, tick, and grid pens as well */
			error = GMT_getrgb (&value[i], rgb);
			if (GMT_check_rgb (rgb))
				error = TRUE;
			else {
				memcpy ((void *)gmtdefs.basemap_frame_rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
				memcpy ((void *)gmtdefs.frame_pen.rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
				if (i == 1) GMT_got_frame_rgb = TRUE;
			}
			break;
		case GMTCASE_BASEMAP_TYPE:
			if (!strcmp (lower_value, "plain"))
				gmtdefs.basemap_type = GMT_IS_PLAIN;
			else if (!strcmp (lower_value, "graph"))
				gmtdefs.basemap_type = GMT_IS_GRAPH;
			else if (!strcmp (lower_value, "fancy"))
				gmtdefs.basemap_type = GMT_IS_FANCY;
			else if (!strcmp (lower_value, "fancy+"))
				gmtdefs.basemap_type = GMT_IS_ROUNDED;
			else if (!strcmp (lower_value, "inside"))
				gmtdefs.basemap_type = GMT_IS_INSIDE;
			else
				error = TRUE;
			break;
		case GMTCASE_COLOR_BACKGROUND:
			if (value[0] == '-')
				gmtdefs.background_rgb[0] = gmtdefs.background_rgb[1] = gmtdefs.background_rgb[2] = -1;
			else {
				error = GMT_getrgb (value, rgb);
				if (GMT_check_rgb (rgb))
					error = TRUE;
				else
					memcpy ((void *)gmtdefs.background_rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
			}
			break;
		case GMTCASE_COLOR_FOREGROUND:
			if (value[0] == '-')
				gmtdefs.foreground_rgb[0] = gmtdefs.foreground_rgb[1] = gmtdefs.foreground_rgb[2] = -1;
			else {
				error = GMT_getrgb (value, rgb);
				if (GMT_check_rgb (rgb))
					error = TRUE;
				else
					memcpy ((void *)gmtdefs.foreground_rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
			}
			break;
		case GMTCASE_COLOR_NAN:
			if (value[0] == '-')
				gmtdefs.nan_rgb[0] = gmtdefs.nan_rgb[1] = gmtdefs.nan_rgb[2] = -1;
			else {
				error = GMT_getrgb (value, rgb);
				if (GMT_check_rgb (rgb))
					error = TRUE;
				else
					memcpy ((void *)gmtdefs.nan_rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
			}
			break;
		case GMTCASE_COLOR_IMAGE:
			if (!strcmp (lower_value, "adobe"))
				gmtdefs.color_image = 0;
			else if (!strcmp (lower_value, "tiles"))
				gmtdefs.color_image = 1;
			else
				error = TRUE;
			break;
		case GMTCASE_COLOR_MODEL:
			if (!strcmp (lower_value, "+hsv"))
				gmtdefs.color_model = GMT_HSV;
			else if (!strcmp (lower_value, "hsv"))
				gmtdefs.color_model = GMT_READ_HSV;
			else if (!strcmp (lower_value, "+rgb"))
				gmtdefs.color_model = GMT_RGB;
			else if (!strcmp (lower_value, "rgb"))
				gmtdefs.color_model = GMT_READ_RGB;
			else if (!strcmp (lower_value, "+cmyk"))
				gmtdefs.color_model = GMT_CMYK;
			else if (!strcmp (lower_value, "cmyk"))
				gmtdefs.color_model = GMT_READ_CMYK;
			else
				error = TRUE;
			break;
		case GMTCASE_D_FORMAT:
			strcpy (gmtdefs.d_format, value);
			break;
		case GMTCASE_DEGREE_FORMAT:
			ival = atoi (value);
			if ((ival%100) >= 0 && (ival%100) <= 17)	/* Mod 100 since we may have added 100 to get big degree symbol or 1000 for no symbol */
				gmtdefs.degree_format = ival;
			else
				error = TRUE;
			GMT_backward.got_old_plot_format = TRUE;
			GMT_backward.got_old_degree_symbol = (ival >= 100);
			break;
		case GMTCASE_DOTS_PR_INCH:
			ival = atoi (value);
			if (ival > 0)
				gmtdefs.dpi = ival;
			else
				error = TRUE;
			break;
		case GMTCASE_ELLIPSOID:
			ival = GMT_get_ellipsoid (value);
			if (ival < 0)
				error = TRUE;
			else
				gmtdefs.ellipsoid = ival;
			break;
		case GMTCASE_FRAME_PEN:
			error = GMT_getpen (value, &gmtdefs.frame_pen);
			break;
		case GMTCASE_FRAME_WIDTH:
			dval = GMT_convert_units (value, GMT_INCH);
			if (dval > 0.0)
				save_frame_width = gmtdefs.frame_width = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_GLOBAL_X_SCALE:
			dval = atof (value);
			if (dval > 0.0)
				gmtdefs.global_x_scale = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_GLOBAL_Y_SCALE:
			dval = atof (value);
			if (dval > 0.0)
				gmtdefs.global_y_scale = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY:
		case GMTCASE_GRID_CROSS_SIZE:
			dval = GMT_convert_units (value, GMT_INCH);
			if (dval >= 0.0)
				gmtdefs.grid_cross_size[0] = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY:
			dval = GMT_convert_units (value, GMT_INCH);
			if (dval >= 0.0)
				gmtdefs.grid_cross_size[1] = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_GRIDFILE_FORMAT:
		case GMTCASE_GRID_FORMAT:
			strcpy (gmtdefs.gridfile_format, value);
			break;
		case GMTCASE_GRID_PEN_PRIMARY:
		case GMTCASE_GRID_PEN:
			error = GMT_getpen (value, &gmtdefs.grid_pen[0]);
			break;
		case GMTCASE_GRID_PEN_SECONDARY:
			error = GMT_getpen (value, &gmtdefs.grid_pen[1]);
			break;
		case GMTCASE_GRIDFILE_SHORTHAND:
			error = true_false_or_error (lower_value, &gmtdefs.gridfile_shorthand);
			break;
		case GMTCASE_HEADER_FONT:
			ival = GMT_font_lookup (value, GMT_font, GMT_N_FONTS);
			if (ival < 0 || ival >= GMT_N_FONTS)
				error = TRUE;
			else
				gmtdefs.header_font = ival;
			break;
		case GMTCASE_HEADER_FONT_SIZE:
			dval = GMT_convert_units (value, 10+GMT_PT);
			if (dval > 0.0)
				save_header_size = gmtdefs.header_font_size = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_HSV_MIN_SATURATION:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				gmtdefs.hsv_min_saturation = dval;
			break;
		case GMTCASE_HSV_MAX_SATURATION:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				gmtdefs.hsv_max_saturation = dval;
			break;
		case GMTCASE_HSV_MIN_VALUE:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				gmtdefs.hsv_min_value = dval;
			break;
		case GMTCASE_HSV_MAX_VALUE:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				gmtdefs.hsv_max_value = dval;
			break;
		case GMTCASE_INTERPOLANT:
			if (!strcmp (lower_value, "linear"))
				gmtdefs.interpolant = 0;
			else if (!strcmp (lower_value, "akima"))
				gmtdefs.interpolant = 1;
			else if (!strcmp (lower_value, "cubic"))
				gmtdefs.interpolant = 2;
			else if (!strcmp (lower_value, "none"))
				gmtdefs.interpolant = 3;
			else
				error = TRUE;
			break;
		case GMTCASE_IO_HEADER:
			error = true_false_or_error (lower_value, &gmtdefs.io_header[GMT_IN]);
			gmtdefs.io_header[GMT_OUT] = gmtdefs.io_header[GMT_IN];
			break;
		case GMTCASE_N_HEADER_RECS:
			ival = atoi (value);
			if (ival < 0)
				error = TRUE;
			else
				gmtdefs.n_header_recs = ival;
			break;
		case GMTCASE_LABEL_FONT:
			ival = GMT_font_lookup (value, GMT_font, GMT_N_FONTS);
			if (ival < 0 || ival >= GMT_N_FONTS)
				error = TRUE;
			else
				gmtdefs.label_font = ival;
			break;
		case GMTCASE_LABEL_FONT_SIZE:
			dval = GMT_convert_units (value, 10+GMT_PT);
			if (dval > 0.0)
				save_label_size = gmtdefs.label_font_size = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_LINE_STEP:
			if ((gmtdefs.line_step = GMT_convert_units (value, GMT_INCH)) <= 0.0) {
				gmtdefs.line_step = 0.01;
				fprintf (stderr, "%s: GMT WARNING: %s <= 0, reset to %g %s\n",
				GMT_program, keyword, gmtdefs.line_step, GMT_unit_names[GMT_INCH]);
			}
			break;
		case GMTCASE_MAP_SCALE_FACTOR:
			if (!strncmp (value, "def", (size_t)3)) /* Default scale for chosen projection */
				gmtdefs.map_scale_factor = -1.0;
			else {
				dval = atof (value);
				if (dval <= 0.0)
					error = TRUE;
				else
					gmtdefs.map_scale_factor = dval;
			}
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			dval = GMT_convert_units (value, GMT_INCH);
			if (dval <= 0.0)
				error = TRUE;
			else
				gmtdefs.map_scale_height = dval;
			break;
		case GMTCASE_MEASURE_UNIT:
			if (!strcmp (lower_value, "cm"))
				gmtdefs.measure_unit = 0;
			else if (!strcmp (lower_value, "inch"))
				gmtdefs.measure_unit = 1;
			else if (!strcmp (lower_value, "m"))
				gmtdefs.measure_unit = 2;
			else if (!strcmp (lower_value, "point"))
				gmtdefs.measure_unit = 3;
			else
				error = TRUE;
			break;
		case GMTCASE_NAN_RECORDS:
			if (!strcmp (lower_value, "pass"))
				gmtdefs.nan_is_gap = TRUE;
			else if (!strcmp (lower_value, "skip"))
				gmtdefs.nan_is_gap = FALSE;
			else
				error = TRUE;
			break;
		case GMTCASE_N_COPIES:
			ival = atoi (value);
			if (ival > 0)
				gmtdefs.n_copies = ival;
			else
				error = TRUE;
			break;
		case GMTCASE_OBLIQUE_ANNOTATION:
		case GMTCASE_OBLIQUE_ANOTATION:
			ival = atoi (value);
			if (ival >= 0 && ival < 64)
				gmtdefs.oblique_annotation = ival;
			else
				error = TRUE;
			break;
		case GMTCASE_PAGE_COLOR:
			error = GMT_getrgb (value, rgb);
			if (GMT_check_rgb (rgb))
				error = TRUE;
			else
				memcpy ((void *)gmtdefs.page_rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
			break;
		case GMTCASE_PAGE_ORIENTATION:
			if (!strcmp (lower_value, "landscape"))
				gmtdefs.portrait = FALSE;
			else if (!strcmp (lower_value, "portrait"))
				gmtdefs.portrait = TRUE;
			else
				error = TRUE;
			break;
		case GMTCASE_PAPER_MEDIA:
			manual = eps = FALSE;
			ival = strlen (lower_value) - 1;
			if (lower_value[ival] == '-') {	/* Manual Feed selected */
				lower_value[ival] = '\0';
				manual = TRUE;
			}
			else if (lower_value[ival] == '+') {	/* EPS format selected */
				lower_value[ival] = '\0';
				eps = TRUE;
			}

			i = GMT_key_lookup (lower_value, GMT_media_name, GMT_N_MEDIA);
			if (i >= 0 && i < GMT_N_MEDIA) {	/* Use the specified standard format */
				gmtdefs.media = i;
				gmtdefs.paper_width[0] = GMT_media[i].width;
				gmtdefs.paper_width[1] = GMT_media[i].height;
			}
			else if (!strncmp (lower_value, "custom_", (size_t)7)) {	/* A custom paper size in W x H points (or in inch/c if units are appended) */
				pos = 0;
				GMT_strtok (&lower_value[7], "x", &pos, txt_a);	/* Returns width and update pos */
				gmtdefs.paper_width[0] = (isdigit(txt_a[strlen(txt_a)-1])) ? atof (txt_a) : GMT_convert_units (txt_a, GMT_PT);
				GMT_strtok (&lower_value[7], "x", &pos, txt_b);	/* Returns height and update pos */
				gmtdefs.paper_width[1] = (isdigit(txt_b[strlen(txt_b)-1])) ? atof (txt_b) : GMT_convert_units (txt_b, GMT_PT);
				if (gmtdefs.paper_width[0] <= 0.0) error++;
				if (gmtdefs.paper_width[1] <= 0.0) error++;
				gmtdefs.media = -USER_MEDIA_OFFSET;
			}
			else {	/* Not one of the standards, try the user-specified formats, if any */
				if ((GMT_n_user_media = GMT_load_user_media ())) {	/* Got some */
					i = GMT_key_lookup (lower_value, GMT_user_media_name, GMT_n_user_media);
					if (i < 0 || i >= GMT_n_user_media) {	/* Not found, give error */
						error = TRUE;
					}
					else {	/* User the user-specified format */
						gmtdefs.media = i + USER_MEDIA_OFFSET;
						gmtdefs.paper_width[0] = GMT_user_media[i].width;
						gmtdefs.paper_width[1] = GMT_user_media[i].height;
					}
				}
				else {	/* Not found, give error */
					error = TRUE;
				}
			}
			if (!error) {
				if (manual) gmtdefs.paper_width[0] = -gmtdefs.paper_width[0];
				if (eps) gmtdefs.paper_width[1] = -gmtdefs.paper_width[1];
			}
			break;
		case GMTCASE_POLAR_CAP:
			if (!strcmp (lower_value, "none")) {	/* Means reset to no cap -> lat = 90, dlon = 0 */
				gmtdefs.polar_cap[0] = 90.0;
				gmtdefs.polar_cap[1] = 0.0;
			}
			else {
				i = sscanf (lower_value, "%[^/]/%s", txt_a, txt_b);
				if (i != 2) error = TRUE;
				error = GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_a, GMT_IS_LAT, &gmtdefs.polar_cap[0]), txt_a);
				error += GMT_getinc (txt_b, &gmtdefs.polar_cap[1], &dval);
			}
			break;
		case GMTCASE_PS_COLOR:
			if (!strcmp (lower_value, "rgb"))
				gmtdefs.ps_colormode = 0;
			else if (!strcmp (lower_value, "cmyk"))
				gmtdefs.ps_colormode = 1;
			else if (!strcmp (lower_value, "hsv"))
				gmtdefs.ps_colormode = 2;
			else if (!strcmp (lower_value, "gray") || !strcmp (lower_value, "grey"))
				gmtdefs.ps_colormode = 3;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_IMAGE_COMPRESS:
		case GMTCASE_PSIMAGE_COMPRESS:
			if (!strcmp (lower_value, "none"))
				gmtdefs.ps_compress = 0;
			else if (!strcmp (lower_value, "rle"))
				gmtdefs.ps_compress = 1;
			else if (!strcmp (lower_value, "lzw"))
				gmtdefs.ps_compress = 2;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_IMAGE_FORMAT:
		case GMTCASE_PSIMAGE_FORMAT:
			if (!strcmp (lower_value, "ascii"))
				gmtdefs.ps_heximage = 1;
			else if (!strcmp (lower_value, "hex"))	/* Backwards compatible */
				gmtdefs.ps_heximage = 1;
			else if (!strcmp (lower_value, "bin"))
				gmtdefs.ps_heximage = 0;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_LINE_CAP:
			if (!strcmp (lower_value, "butt"))
				gmtdefs.ps_line_cap = 0;
			else if (!strcmp (lower_value, "round"))
				gmtdefs.ps_line_cap = 1;
			else if (!strcmp (lower_value, "square"))
				gmtdefs.ps_line_cap = 2;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_LINE_JOIN:
			if (!strcmp (lower_value, "miter"))
				gmtdefs.ps_line_join = 0;
			else if (!strcmp (lower_value, "round"))
				gmtdefs.ps_line_join = 1;
			else if (!strcmp (lower_value, "bevel"))
				gmtdefs.ps_line_join = 2;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_MITER_LIMIT:
			ival = atoi (value);
			if (ival >= 0 && ival <= 180)
				gmtdefs.ps_miter_limit = ival;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_VERBOSE:
			error = true_false_or_error (lower_value, &gmtdefs.ps_verbose);
			break;
		case GMTCASE_TICK_LENGTH:
			save_tick_length = gmtdefs.tick_length = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_TICK_PEN:
			error = GMT_getpen (value, &gmtdefs.tick_pen);
			break;
		case GMTCASE_UNIX_TIME:
			error = true_false_or_error (lower_value, &gmtdefs.unix_time);
			break;
		case GMTCASE_UNIX_TIME_POS:
			i = sscanf (value, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (i == 2) {
				gmtdefs.unix_time_pos[GMT_X] = GMT_convert_units (txt_a, GMT_INCH);
				gmtdefs.unix_time_pos[GMT_Y] = GMT_convert_units (txt_b, GMT_INCH);
			}
			else if (i == 3) {	/* New style, includes justification, introduced in GMT 4.3.0 */
				gmtdefs.unix_time_just = GMT_just_decode (txt_a, 12);
				gmtdefs.unix_time_pos[GMT_X] = GMT_convert_units (txt_b, GMT_INCH);
				gmtdefs.unix_time_pos[GMT_Y] = GMT_convert_units (txt_c, GMT_INCH);
			}
			else
				error = TRUE;
			break;
		case GMTCASE_UNIX_TIME_FORMAT:
			strncpy (gmtdefs.unix_time_format, value, (size_t)GMT_LONG_TEXT);
			break;
		case GMTCASE_VECTOR_SHAPE:
			dval = atof (value);
			if (dval < -2.0 || dval > 2.0)
				error = TRUE;
			else
				gmtdefs.vector_shape = dval;
			break;
		case GMTCASE_VERBOSE:
			error = true_false_or_error (lower_value, &gmtdefs.verbose);
			break;
		case GMTCASE_WANT_EURO_FONT:
			error = true_false_or_error (lower_value, &gmtdefs.want_euro_font);
			break;
		case GMTCASE_X_AXIS_LENGTH:
			dval = GMT_convert_units (value, GMT_INCH);
			if (dval <= 0.0)
				error = TRUE;
			else
				gmtdefs.x_axis_length = dval;
			break;
		case GMTCASE_Y_AXIS_LENGTH:
			dval = GMT_convert_units (value, GMT_INCH);
			if (dval <= 0.0)
				error = TRUE;
			else
				gmtdefs.y_axis_length = dval;
			break;
		case GMTCASE_X_ORIGIN:
			gmtdefs.x_origin = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_Y_ORIGIN:
			gmtdefs.y_origin = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_XY_TOGGLE:
			if (!strcmp (lower_value, "true"))
				gmtdefs.xy_toggle[GMT_IN] = gmtdefs.xy_toggle[GMT_OUT] = TRUE;
			else if (!strcmp (lower_value, "false"))
				gmtdefs.xy_toggle[GMT_IN] = gmtdefs.xy_toggle[GMT_OUT] = FALSE;
			else if (!strcmp (lower_value, "in")) {
				gmtdefs.xy_toggle[GMT_IN] = TRUE;
				gmtdefs.xy_toggle[GMT_OUT] = FALSE;
			}
			else if (!strcmp (lower_value, "out")) {
				gmtdefs.xy_toggle[GMT_IN] = FALSE;
				gmtdefs.xy_toggle[GMT_OUT] = TRUE;
			}
			else
				error = TRUE;
			break;
		case GMTCASE_Y_AXIS_TYPE:
			if (!strcmp (lower_value, "ver_text"))
				gmtdefs.y_axis_type = 1;
			else if (!strcmp (lower_value, "hor_text"))
				gmtdefs.y_axis_type = 0;
			else
				error = TRUE;
			break;
		case GMTCASE_INPUT_CLOCK_FORMAT:
			strncpy (gmtdefs.input_clock_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_INPUT_DATE_FORMAT:
			strncpy (gmtdefs.input_date_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_OUTPUT_CLOCK_FORMAT:
			strncpy (gmtdefs.output_clock_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_OUTPUT_DATE_FORMAT:
			strncpy (gmtdefs.output_date_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_OUTPUT_DEGREE_FORMAT:
			strncpy (gmtdefs.output_degree_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_PLOT_CLOCK_FORMAT:
			strncpy (gmtdefs.plot_clock_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_PLOT_DATE_FORMAT:
			strncpy (gmtdefs.plot_date_format, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_PLOT_DEGREE_FORMAT:
			strncpy (gmtdefs.plot_degree_format, value, (size_t)GMT_TEXT_LEN);
			GMT_backward.got_new_plot_format = TRUE;
			break;
		case GMTCASE_TIME_FORMAT:
		case GMTCASE_TIME_FORMAT_PRIMARY:
			strncpy (gmtdefs.time_format[0], value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_TIME_FORMAT_SECONDARY:
			strncpy (gmtdefs.time_format[1], value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_TIME_IS_INTERVAL:
			if (value[0] == '+' || value[0] == '-') {	/* OK, gave +<n>u or -<n>u, check for unit */
				sscanf (&lower_value[1], "%" GMT_LL "d%c", &GMT_truncate_time.T.step, &GMT_truncate_time.T.unit);
				switch (GMT_truncate_time.T.unit) {
					case 'y':
					case 'o':
					case 'd':
					case 'h':
					case 'm':
					case 'c':
						GMT_truncate_time.direction = (lower_value[0] == '+') ? 0 : 1;
						break;
					default:
						error = TRUE;
						break;
				}
				if (GMT_truncate_time.T.step == 0) error = TRUE;
				gmtdefs.time_is_interval = TRUE;
			}
			else if (!strcmp (lower_value, "off"))
				gmtdefs.time_is_interval = FALSE;
			else
				error = TRUE;
			break;
		case GMTCASE_TIME_INTERVAL_FRACTION:
			gmtdefs.time_interval_fraction = atof (value);
			break;
		case GMTCASE_WANT_LEAP_SECONDS:
			error = true_false_or_error (lower_value, &gmtdefs.want_leap_seconds);
			break;
		case GMTCASE_TIME_EPOCH:
			strncpy (gmtdefs.time_system.epoch, value, (size_t)GMT_TEXT_LEN);
			break;
		case GMTCASE_TIME_UNIT:
			gmtdefs.time_system.unit = lower_value[0];
			break;
		case GMTCASE_TIME_SYSTEM:
			error = GMT_get_time_system (lower_value, &gmtdefs.time_system);
			break;
		case GMTCASE_TIME_WEEK_START:
			gmtdefs.time_week_start = GMT_key_lookup (value, GMT_weekdays, 7);
			if (gmtdefs.time_week_start < 0 || gmtdefs.time_week_start >= 7) {
				error = TRUE;
				gmtdefs.time_week_start = 0;
			}
			break;
		case GMTCASE_TIME_LANGUAGE:
			strncpy (gmtdefs.time_language, value, (size_t)GMT_TEXT_LEN);
			GMT_str_tolower (gmtdefs.time_language);
			break;
		case GMTCASE_CHAR_ENCODING:
			strncpy (gmtdefs.encoding.name, value, (size_t)GMT_TEXT_LEN);
			load_encoding (&gmtdefs.encoding);
			break;
		case GMTCASE_Y2K_OFFSET_YEAR:
			if ((gmtdefs.Y2K_offset_year = atoi (value)) < 0) error = TRUE;
			break;
		case GMTCASE_FIELD_DELIMITER:
			if (value[0] == '\0' || !strcmp (lower_value, "tab"))	/* DEFAULT */
				strncpy (gmtdefs.field_delimiter, "\t", (size_t)8);
			else if (!strcmp (lower_value, "space"))
				strncpy (gmtdefs.field_delimiter, " ", (size_t)8);
			else if (!strcmp (lower_value, "comma"))
				strncpy (gmtdefs.field_delimiter, ",", (size_t)8);
			else if (!strcmp (lower_value, "none"))
				gmtdefs.field_delimiter[0] = 0;
			else
				strncpy (gmtdefs.field_delimiter, value, (size_t)8);
			gmtdefs.field_delimiter[7] = 0;	/* Just a precaution */
			break;
		case GMTCASE_DEGREE_SYMBOL:
			if (value[0] == '\0' || !strcmp (lower_value, "ring"))	/* DEFAULT */
				gmtdefs.degree_symbol = gmt_ring;
			else if (!strcmp (lower_value, "degree"))
				gmtdefs.degree_symbol = gmt_degree;
			else if (!strcmp (lower_value, "colon"))
				gmtdefs.degree_symbol = gmt_colon;
			else if (!strcmp (lower_value, "none"))
				gmtdefs.degree_symbol = gmt_none;
			else
				error = TRUE;
			GMT_backward.got_new_degree_symbol = TRUE;
			break;
		case GMTCASE_ANNOT_FONT_SECONDARY:
		case GMTCASE_ANOT_FONT_SECONDARY:
		case GMTCASE_ANNOT_FONT2:
		case GMTCASE_ANOT_FONT2:
			ival = GMT_font_lookup (value, GMT_font, GMT_N_FONTS);
			if (ival < 0 || ival >= GMT_N_FONTS)
				error = TRUE;
			else
				gmtdefs.annot_font[1] = ival;
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY:
		case GMTCASE_ANOT_FONT_SIZE_SECONDARY:
		case GMTCASE_ANNOT_FONT2_SIZE:
		case GMTCASE_ANOT_FONT2_SIZE:
			dval = GMT_convert_units (value, 10+GMT_PT);
			if (dval > 0.0)
				save_annot_size[1] = gmtdefs.annot_font_size[1] = dval;
			else
				error = TRUE;
			break;

		case GMTCASE_ANNOT_OFFSET_SECONDARY:
		case GMTCASE_ANOT_OFFSET_SECONDARY:
		case GMTCASE_ANNOT_OFFSET2:
		case GMTCASE_ANOT_OFFSET2:
			save_annot_offset[1] = gmtdefs.annot_offset[1] = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_LABEL_OFFSET:
			save_label_offset = gmtdefs.label_offset = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_HEADER_OFFSET:
			save_header_offset = gmtdefs.header_offset = GMT_convert_units (value, GMT_INCH);
			break;
		case GMTCASE_HISTORY:
			error = true_false_or_error (lower_value, &gmtdefs.history);
			break;
		case GMTCASE_TRANSPARENCY:
			i = sscanf (value, "%" GMT_LL "d/%" GMT_LL "d", &gmtdefs.transparency[0], &gmtdefs.transparency[1]);
			if (i == 1)
				gmtdefs.transparency[1] = gmtdefs.transparency[0];
			else if (i != 2)
				error = TRUE;
			break;
		default:
			error = TRUE;
			fprintf (stderr, "%s: GMT SYNTAX ERROR in GMT_setparameter:  Unrecognized keyword %s\n",
				GMT_program, keyword);
			break;
	}

	if (error && case_val >= 0) fprintf (stderr, "%s: GMT SYNTAX ERROR:  %s given illegal value (%s)!\n", GMT_program, keyword, value);
	return (error);
}

GMT_LONG true_false_or_error (char *value, GMT_LONG *answer)
{
	/* Assigns 1 or 0 to answer if value is true or false and return FALSE.
	 * If not given true or false, return error TRUE */

	if (!strcmp (value, "true")) {	/* TRUE */
		*answer = 1;
		return (FALSE);
	}
	if (!strcmp (value, "false")) {	/* FALSE */
		*answer = 0;
		return (FALSE);
	}

	/* Got neither true or false.  Make no assignment and return TRUE for error */

	return (TRUE);
}

void GMT_putdefaults (char *this_file)	/* Dumps the GMT parameters to file or standard output */
{	/* ONLY USED BY GMTSET AND GMTDEFAULTS */
	if (this_file)	/* File name is defined: use it */
		GMT_savedefaults (this_file);
	else if (GMT_TMPDIR) {	/* Write $GMT_TMPDIR/.gmtdefaults4 */
		char *path = CNULL;
		path = (char *) GMT_memory (VNULL, (GMT_LONG)(strlen (GMT_TMPDIR) + 15), sizeof (char), "GMT");
		sprintf (path, "%s%c.gmtdefaults4", GMT_TMPDIR, DIR_DELIM);
		GMT_savedefaults (path);
		GMT_free(path);
	}
	else	/* Write .gmtdefaults4 in current directory */
		GMT_savedefaults (".gmtdefaults4");
}

GMT_LONG GMT_savedefaults (char *file)
{
	FILE *fp;
	char u, pm[2] = {'+', '-'};
	char *ft[2] = {"FALSE", "TRUE"};
	double s;

	if (file[0] == '-' && !file[1])
		fp = GMT_stdout;
	else if ((fp = fopen (file, "w")) == NULL) {
		fprintf (stderr, "%s: Error: Could not create file %s\n", file, GMT_program);
		return (-1);
	}

	u = GMT_unit_names[gmtdefs.measure_unit][0];
	s = GMT_u2u[GMT_INCH][gmtdefs.measure_unit];	/* Convert from internal inch to users unit */

	fprintf (fp, "#\n#\tGMT-SYSTEM %s Defaults file\n#\n", GMT_VERSION);
	fprintf (fp, "#-------- Plot Media Parameters -------------\n");
	GMT_put_colorname (fp, "PAGE_COLOR\t\t= ", gmtdefs.page_rgb);
	fprintf (fp, "PAGE_ORIENTATION\t= %s\n", (gmtdefs.portrait ? "portrait" : "landscape"));
	fprintf (fp, "PAPER_MEDIA\t\t= ");
	if (gmtdefs.media == -USER_MEDIA_OFFSET)
		fprintf (fp, "Custom_%gx%g", fabs(gmtdefs.paper_width[0]), fabs(gmtdefs.paper_width[1]));
	else if (gmtdefs.media >= USER_MEDIA_OFFSET)
		fprintf (fp, "%s", GMT_user_media_name[gmtdefs.media-USER_MEDIA_OFFSET]);
	else
		fprintf (fp, "%s", GMT_media_name[gmtdefs.media]);
	if (gmtdefs.paper_width[0] < 0.0)
		fprintf (fp, "-\n");
	else if (gmtdefs.paper_width[1] < 0.0)
		fprintf (fp, "+\n");
	else
		fprintf (fp, "\n");
	fprintf (fp, "#-------- Basemap Annotation Parameters ------\n");
	fprintf (fp, "ANNOT_MIN_ANGLE\t\t= %g\n", gmtdefs.annot_min_angle);
	fprintf (fp, "ANNOT_MIN_SPACING\t= %g\n", gmtdefs.annot_min_spacing);
	fprintf (fp, "ANNOT_FONT_PRIMARY\t= %s\n", GMT_font[gmtdefs.annot_font[0]].name);
	fprintf (fp, "ANNOT_FONT_SIZE_PRIMARY\t= %s%gp\n", (GMT_force_resize && !GMT_annot_special ? "+" : ""), gmtdefs.annot_font_size[0]);
	fprintf (fp, "ANNOT_OFFSET_PRIMARY\t= %g%c\n", (GMT_force_resize ? save_annot_offset[0] : gmtdefs.annot_offset[0]) * s, u);
	fprintf (fp, "ANNOT_FONT_SECONDARY\t= %s\n", GMT_font[gmtdefs.annot_font[1]].name);
	fprintf (fp, "ANNOT_FONT_SIZE_SECONDARY\t= %gp\n", (GMT_force_resize ? save_annot_size[1] : gmtdefs.annot_font_size[1]));
	fprintf (fp, "ANNOT_OFFSET_SECONDARY\t= %g%c\n", (GMT_force_resize ? save_annot_offset[1] : gmtdefs.annot_offset[1]) * s, u);
	fprintf (fp, "DEGREE_SYMBOL\t\t= %s\n", GMT_degree_choice[gmtdefs.degree_symbol - gmt_none]);
	fprintf (fp, "HEADER_FONT\t\t= %s\n", GMT_font[gmtdefs.header_font].name);
	fprintf (fp, "HEADER_FONT_SIZE\t= %gp\n", (GMT_force_resize ? save_header_size : gmtdefs.header_font_size));
	fprintf (fp, "HEADER_OFFSET\t\t= %g%c\n", (GMT_force_resize ? save_header_offset : gmtdefs.header_offset) * s, u);
	fprintf (fp, "LABEL_FONT\t\t= %s\n", GMT_font[gmtdefs.label_font].name);
	fprintf (fp, "LABEL_FONT_SIZE\t\t= %gp\n", (GMT_force_resize ? save_label_size : gmtdefs.label_font_size));
	fprintf (fp, "LABEL_OFFSET\t\t= %g%c\n", (GMT_force_resize ? save_label_offset : gmtdefs.label_offset) * s, u);
	fprintf (fp, "OBLIQUE_ANNOTATION\t= %ld\n", gmtdefs.oblique_annotation);
	fprintf (fp, "PLOT_CLOCK_FORMAT\t= %s\n", gmtdefs.plot_clock_format);
	fprintf (fp, "PLOT_DATE_FORMAT\t= %s\n", gmtdefs.plot_date_format);
	fprintf (fp, "PLOT_DEGREE_FORMAT\t= %s\n", gmtdefs.plot_degree_format);
	fprintf (fp, "Y_AXIS_TYPE\t\t= %s\n", (gmtdefs.y_axis_type == 1 ? "ver_text" : "hor_text"));
	fprintf (fp, "#-------- Basemap Layout Parameters ---------\n");
	fprintf (fp, "BASEMAP_AXES\t\t= %s\n", gmtdefs.basemap_axes);
	GMT_put_colorname (fp, "BASEMAP_FRAME_RGB\t= ", gmtdefs.basemap_frame_rgb);
	fprintf (fp, "BASEMAP_TYPE\t\t= ");
	if (gmtdefs.basemap_type == GMT_IS_PLAIN)
		fprintf (fp, "plain\n");
	else if (gmtdefs.basemap_type == GMT_IS_FANCY)
		fprintf (fp, "fancy\n");
	else if (gmtdefs.basemap_type == GMT_IS_ROUNDED)
		fprintf (fp, "fancy+\n");
	fprintf (fp, "FRAME_PEN\t\t= %s\n", GMT_putpen (&gmtdefs.frame_pen));
	fprintf (fp, "FRAME_WIDTH\t\t= %g%c\n", (GMT_force_resize ? save_frame_width : gmtdefs.frame_width) * s, u);
	fprintf (fp, "GRID_CROSS_SIZE_PRIMARY\t= %g%c\n", gmtdefs.grid_cross_size[0] * s, u);
	fprintf (fp, "GRID_PEN_PRIMARY\t= %s\n", GMT_putpen (&gmtdefs.grid_pen[0]));
	fprintf (fp, "GRID_CROSS_SIZE_SECONDARY\t= %g%c\n", gmtdefs.grid_cross_size[1] * s, u);
	fprintf (fp, "GRID_PEN_SECONDARY\t= %s\n", GMT_putpen (&gmtdefs.grid_pen[1]));
	fprintf (fp, "MAP_SCALE_HEIGHT\t= %g%c\n", gmtdefs.map_scale_height * s, u);
	fprintf (fp, "POLAR_CAP\t\t= ");
	if (GMT_IS_ZERO (gmtdefs.polar_cap[0] - 90.0))
		fprintf (fp, "none\n");
	else
		fprintf (fp, "%g/%g\n", gmtdefs.polar_cap[0], gmtdefs.polar_cap[1]);
	fprintf (fp, "TICK_LENGTH\t\t= %g%c\n", (GMT_force_resize ? save_tick_length : gmtdefs.tick_length) * s, u);
	fprintf (fp, "TICK_PEN\t\t= %s\n", GMT_putpen (&gmtdefs.tick_pen));
	fprintf (fp, "X_AXIS_LENGTH\t\t= %g%c\n", gmtdefs.x_axis_length * s, u);
	fprintf (fp, "Y_AXIS_LENGTH\t\t= %g%c\n", gmtdefs.y_axis_length * s, u);
	fprintf (fp, "X_ORIGIN\t\t= %g%c\n", gmtdefs.x_origin * s, u);
	fprintf (fp, "Y_ORIGIN\t\t= %g%c\n", gmtdefs.y_origin * s, u);
	fprintf (fp, "UNIX_TIME\t\t= %s\n", ft[gmtdefs.unix_time]);
	fprintf (fp, "UNIX_TIME_POS\t\t= %s/%g%c/%g%c\n", GMT_just_string[gmtdefs.unix_time_just], gmtdefs.unix_time_pos[GMT_X] * s, u, gmtdefs.unix_time_pos[GMT_Y] * s, u);
	fprintf (fp, "UNIX_TIME_FORMAT\t= %s\n", gmtdefs.unix_time_format);
	fprintf (fp, "#-------- Color System Parameters -----------\n");
	GMT_put_colorname (fp, "COLOR_BACKGROUND\t= ", gmtdefs.background_rgb);
	GMT_put_colorname (fp, "COLOR_FOREGROUND\t= ", gmtdefs.foreground_rgb);
	GMT_put_colorname (fp, "COLOR_NAN\t\t= ", gmtdefs.nan_rgb);
	fprintf (fp, "COLOR_IMAGE\t\t= %s\n", (gmtdefs.color_image ? "tiles" : "adobe"));
	fprintf (fp, "COLOR_MODEL\t\t= ");
	if (gmtdefs.color_model & GMT_USE_HSV)
		fprintf (fp, "+hsv\n");
	else if (gmtdefs.color_model & GMT_READ_HSV)
		fprintf (fp, "hsv\n");
	else if (gmtdefs.color_model & GMT_USE_CMYK)
		fprintf (fp, "+cmyk\n");
	else if (gmtdefs.color_model & GMT_READ_CMYK)
		fprintf (fp, "cmyk\n");
	else if (gmtdefs.color_model & GMT_USE_RGB)
		fprintf (fp, "+rgb\n");
	else
		fprintf (fp, "rgb\n");
	fprintf (fp, "HSV_MIN_SATURATION\t= %g\n", gmtdefs.hsv_min_saturation);
	fprintf (fp, "HSV_MAX_SATURATION\t= %g\n", gmtdefs.hsv_max_saturation);
	fprintf (fp, "HSV_MIN_VALUE\t\t= %g\n", gmtdefs.hsv_min_value);
	fprintf (fp, "HSV_MAX_VALUE\t\t= %g\n", gmtdefs.hsv_max_value);
	fprintf (fp, "#-------- PostScript Parameters -------------\n");
	fprintf (fp, "CHAR_ENCODING\t\t= %s\n", gmtdefs.encoding.name);
	fprintf (fp, "DOTS_PR_INCH\t\t= %ld\n", gmtdefs.dpi);
	fprintf (fp, "GLOBAL_X_SCALE\t\t= %g\n", gmtdefs.global_x_scale);
	fprintf (fp, "GLOBAL_Y_SCALE\t\t= %g\n", gmtdefs.global_y_scale);
	fprintf (fp, "N_COPIES\t\t= %ld\n", gmtdefs.n_copies);
	fprintf (fp, "PS_COLOR\t\t= ");
	if (gmtdefs.ps_colormode == 0)
		fprintf (fp, "rgb\n");
	else if (gmtdefs.ps_colormode == 1)
		fprintf (fp, "cmyk\n");
	else if (gmtdefs.ps_colormode == 2)
		fprintf (fp, "hsv\n");
	else
		fprintf (fp, "gray\n");
	fprintf (fp, "PS_IMAGE_COMPRESS\t= ");
	if (gmtdefs.ps_compress == 1)
		fprintf (fp, "rle\n");
	else if (gmtdefs.ps_compress == 2)
		fprintf (fp, "lzw\n");
	else
		fprintf (fp, "none\n");
	fprintf (fp, "PS_IMAGE_FORMAT\t\t= %s\n", (gmtdefs.ps_heximage ? "ascii" : "bin"));
	fprintf (fp, "PS_LINE_CAP\t\t= ");
	if (gmtdefs.ps_line_cap == 0)
		fprintf (fp, "butt\n");
	else if (gmtdefs.ps_line_cap == 1)
		fprintf (fp, "round\n");
	else
		fprintf (fp, "square\n");
	fprintf (fp, "PS_LINE_JOIN\t\t= ");
	if (gmtdefs.ps_line_join == 0)
		fprintf (fp, "miter\n");
	else if (gmtdefs.ps_line_join == 1)
		fprintf (fp, "round\n");
	else
		fprintf (fp, "bevel\n");
	fprintf (fp, "PS_MITER_LIMIT\t\t= %ld\n", gmtdefs.ps_miter_limit);
	fprintf (fp, "PS_VERBOSE\t\t= %s\n", ft[gmtdefs.ps_verbose]);
	if (gmtdefs.transparency[0] == gmtdefs.transparency[1])
		fprintf (fp, "TRANSPARENCY\t\t= %ld\n", gmtdefs.transparency[0]);
	else
		fprintf (fp, "TRANSPARENCY\t\t= %ld/%ld\n", gmtdefs.transparency[0], gmtdefs.transparency[1]);
	fprintf (fp, "#-------- I/O Format Parameters -------------\n");
	fprintf (fp, "D_FORMAT\t\t= %s\n", gmtdefs.d_format);
	fprintf (fp, "FIELD_DELIMITER\t\t= ");
	if (!strcmp (gmtdefs.field_delimiter, "\t"))
		fprintf (fp, "tab\n");
	else if (!strcmp (gmtdefs.field_delimiter, " "))
		fprintf (fp, "space\n");
	else if (!strcmp (gmtdefs.field_delimiter, ","))
		fprintf (fp, "comma\n");
	else if (gmtdefs.field_delimiter[0] == 0)
		fprintf (fp, "none\n");
	else
		fprintf (fp, "%s\n", gmtdefs.field_delimiter);
	fprintf (fp, "GRIDFILE_FORMAT\t\t= %s\n", gmtdefs.gridfile_format);
	fprintf (fp, "GRIDFILE_SHORTHAND\t= %s\n", ft[gmtdefs.gridfile_shorthand]);
	fprintf (fp, "INPUT_CLOCK_FORMAT\t= %s\n", gmtdefs.input_clock_format);
	fprintf (fp, "INPUT_DATE_FORMAT\t= %s\n", gmtdefs.input_date_format);
	fprintf (fp, "IO_HEADER\t\t= %s\n", ft[gmtdefs.io_header[GMT_IN]]);
	fprintf (fp, "N_HEADER_RECS\t\t= %ld\n", gmtdefs.n_header_recs);
	fprintf (fp, "NAN_RECORDS\t\t= ");
	if (gmtdefs.nan_is_gap)
		fprintf (fp, "pass\n");
	else
		fprintf (fp, "skip\n");
	fprintf (fp, "OUTPUT_CLOCK_FORMAT\t= %s\n", gmtdefs.output_clock_format);
	fprintf (fp, "OUTPUT_DATE_FORMAT\t= %s\n", gmtdefs.output_date_format);
	fprintf (fp, "OUTPUT_DEGREE_FORMAT\t= %s\n", gmtdefs.output_degree_format);
	fprintf (fp, "XY_TOGGLE\t\t= ");
	if (gmtdefs.xy_toggle[GMT_IN] && gmtdefs.xy_toggle[GMT_OUT])
		fprintf (fp, "TRUE\n");
	else if (!gmtdefs.xy_toggle[GMT_IN] && !gmtdefs.xy_toggle[GMT_OUT])
		fprintf (fp, "FALSE\n");
	else if (gmtdefs.xy_toggle[GMT_IN] && !gmtdefs.xy_toggle[GMT_OUT])
		fprintf (fp, "IN\n");
	else
		fprintf (fp, "OUT\n");
	fprintf (fp, "#-------- Projection Parameters -------------\n");
	if (gmtdefs.ellipsoid < GMT_N_ELLIPSOIDS - 1)	/* Custom ellipse */
		fprintf (fp, "ELLIPSOID\t\t= %s\n", gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].name);
	else if (GMT_IS_SPHERICAL)
		fprintf (fp, "ELLIPSOID\t\t= %f\n", gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius);
	else
		fprintf (fp, "ELLIPSOID\t\t= %f,%f\n", gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius,
			1.0/gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening);
	fprintf (fp, "MAP_SCALE_FACTOR\t= ");
	if (gmtdefs.map_scale_factor == -1.0)
		fprintf (fp, "default\n");
	else
		fprintf (fp, "%g\n", gmtdefs.map_scale_factor);
	fprintf (fp, "MEASURE_UNIT\t\t= %s\n", GMT_unit_names[gmtdefs.measure_unit]);
	fprintf (fp, "#-------- Calendar/Time Parameters ----------\n");
	fprintf (fp, "TIME_FORMAT_PRIMARY\t= %s\n", gmtdefs.time_format[0]);
	fprintf (fp, "TIME_FORMAT_SECONDARY\t= %s\n", gmtdefs.time_format[1]);
	fprintf (fp, "TIME_EPOCH\t\t= %s\n", gmtdefs.time_system.epoch);
	fprintf (fp, "TIME_IS_INTERVAL\t= ");
	if (gmtdefs.time_is_interval)
		fprintf (fp, "%c%ld%c\n", pm[GMT_truncate_time.direction], GMT_truncate_time.T.step, GMT_truncate_time.T.unit);
	else
		fprintf (fp, "OFF\n");
	fprintf (fp, "TIME_INTERVAL_FRACTION\t= %g\n", gmtdefs.time_interval_fraction);
	fprintf (fp, "TIME_LANGUAGE\t\t= %s\n", gmtdefs.time_language);
	fprintf (fp, "TIME_UNIT\t\t= %c\n", gmtdefs.time_system.unit);
	fprintf (fp, "TIME_WEEK_START\t\t= %s\n", GMT_weekdays[gmtdefs.time_week_start]);
	/*
	 * PW 4/2/03: LEAP_SECONDS is commented out for output until we actually want to implement this feature.  We still process on input to avoid error messages.
	fprintf (fp, "WANT_LEAP_SECONDS\t= %s\n", ft[gmtdefs.want_leap_seconds]);
	 */
	fprintf (fp, "Y2K_OFFSET_YEAR\t\t= %ld\n", gmtdefs.Y2K_offset_year);
	fprintf (fp, "#-------- Miscellaneous Parameters ----------\n");
	fprintf (fp, "HISTORY\t\t\t= %s\n", ft[gmtdefs.history]);
	fprintf (fp, "INTERPOLANT\t\t= ");
	if (gmtdefs.interpolant == 0)
		fprintf (fp, "linear\n");
	else if (gmtdefs.interpolant == 1)
		fprintf (fp, "akima\n");
	else if (gmtdefs.interpolant == 2)
		fprintf (fp, "cubic\n");
	else if (gmtdefs.interpolant == 3)
		fprintf (fp, "none\n");
	fprintf (fp, "LINE_STEP\t\t= %g%c\n", gmtdefs.line_step * s, u);
	fprintf (fp, "VECTOR_SHAPE\t\t= %g\n", gmtdefs.vector_shape);
	fprintf (fp, "VERBOSE\t\t\t= %s\n", ft[gmtdefs.verbose]);

	if (fp != GMT_stdout) fclose (fp);

	return (0);
}

void GMT_getdefaults (char *this_file)	/* Read user's .gmtdefaults4 file and initialize parameters */
{
	GMT_LONG i;
	char file[BUFSIZ];

	/* Set up hash table for colornames */

	GMT_hash_init (GMT_rgb_hashnode, GMT_color_name, GMT_N_COLOR_NAMES, GMT_N_COLOR_NAMES);

	/* Default is to draw AND annotate all sides */
	for (i = 0; i < 5; i++) frame_info.side[i] = 2;

	if (this_file)	/* Defaults file is specified */
		GMT_loaddefaults (this_file);
	else if (GMT_getuserpath (".gmtdefaults4", file))
		GMT_loaddefaults (file);
	else if (GMT_getuserpath (".gmtdefaults", file)) {	/* Try old GMT 3 defaults - give warning */
		fprintf (stderr, "GMT Warning: Old GMT 3 .gmtdefaults file found.  May not be fully compatible with GMT 4.\n");
		fprintf (stderr, "GMT Warning: It is recommended that you migrate your GMT 3 settings to GMT 4 settings.\n");
		GMT_loaddefaults (file);
	}
	else {		/* No .gmtdefaults[4] files in sight; Must use GMT system defaults */
		char *path;
		GMT_getdefpath (0, &path);
		GMT_loaddefaults (path);
		free ((void *)path);
	}
}

GMT_LONG GMT_getdefpath (GMT_LONG get, char **P)
{
	/* Return the full path to the chosen .gmtdefaults4 system file
	 * depending on the value of get:
	 * get = 0:	Use gmt.conf to set get to 1 or 2.
	 * get = 1:	Use the SI settings.
	 * get = 2:	Use the US settings. */

	GMT_LONG id;
	char line[BUFSIZ], *path, *suffix[2] = {"SI", "US"};
	FILE *fp;

	if (get == 0) {	/* Must use GMT system defaults via gmt.conf */

		GMT_getsharepath ("conf", "gmt", ".conf", line);
		if ((fp = fopen (line, "r")) == NULL) {
			fprintf (stderr, "GMT Fatal Error: Cannot open/find GMT configuration file %s\n", line);
			GMT_exit (EXIT_FAILURE);
		}

		while (fgets (line, BUFSIZ, fp) && (line[0] == '#' || line[0] == '\n'));	/* Scan to first real line */
		fclose (fp);
		if (!strncmp (line, "US", (size_t)2))
			id = 2;
		else if (!strncmp (line, "SI", (size_t)2))
			id = 1;
		else {
			fprintf (stderr, "GMT Fatal Error: No SI/US keyword in GMT configuration file (%s)\n", line);
			GMT_exit (EXIT_FAILURE);
		}
	}
	else
		id = get;

	id--;	/* Get 0 or 1 */
	GMT_getsharepath ("conf", "gmtdefaults_", suffix[id], line);

	path = strdup (line);

	*P = path;

	return (GMT_NOERROR);
}

void GMT_put_colorname (FILE *fp, char *string, int *rgb)
{
	/* Write the name (if available) or rr[/gg/bb] corresponding to the RGB triplet */

	GMT_LONG i;

	if (string[0]) fprintf (fp, "%s", string);
	if (rgb[0] < 0)
		fprintf (fp, "-\n");
	else if ((i = GMT_getrgb_index (rgb)) >= 0)
		fprintf (fp, "%s\n", GMT_color_name[i]);
	else if (rgb[0] == rgb[1] && rgb[1] == rgb[2])
		fprintf (fp, "%d\n", rgb[0]);
	else
		fprintf (fp, "%d/%d/%d\n", rgb[0], rgb[1], rgb[2]);
}
		

double GMT_convert_units (char *from, GMT_LONG new_format)
{
	/* Converts the input value to new units indicated by new_format
	 * If new_format >= 10, then use (new_format - 10) as default unit
	 * instead of gmtdefs.measure_unit.
	 */

	GMT_LONG c = 0, len, old_format, save_measure_unit = -1;
	GMT_LONG have_unit = FALSE;
	double value;

	/* If new_format >= 10, temporily change gmtdefs.measure_unit */

	if (new_format >= 10) {
		new_format -= 10;
		save_measure_unit = gmtdefs.measure_unit;
		gmtdefs.measure_unit = new_format;
	}

	if ((len = strlen(from))) {
		c = from[len-1];
		if ((have_unit = (isalpha ((int)c) || c == '%'))) from[len-1] = '\0';	/* Temporarily remove unit */
	}

	/* So c is either 0 (meaing default unit) or any letter (even junk like z) */

	old_format = GMT_unit_lookup (c);	/* Will warn if c is not 0, 'c', 'i', 'm', 'p' */

	if (GMT_is_invalid_number (from))
		fprintf (stderr, "%s: Warning: %s not a valid number and may not be decoded properly.\n", GMT_program, from);

	value = atof (from) * GMT_u2u[old_format][new_format];
	if (have_unit) from[len-1] = (char)c;	/* Put back what we took out temporarily */

	if (save_measure_unit >= 0) gmtdefs.measure_unit = save_measure_unit;	/* Put back default unit */

	return (value);

}

GMT_LONG GMT_unit_lookup (GMT_LONG c)
{
	GMT_LONG unit;

	if (!isalpha ((int)c)) {	/* Not a unit modifier - just return the current default unit */
		return (gmtdefs.measure_unit);
	}

	/* Now we check for the c-i-m-p units and barf otherwise */

	switch (c) {
		case 'C':	/* Centimeters */
		case 'c':
			unit = GMT_CM;
			break;
		case 'I':	/* Inches */
		case 'i':
			unit = GMT_INCH;
			break;
		case 'M':	/* Meters */
		case 'm':
			unit = GMT_M;
			break;
		case 'P':	/* Points */
		case 'p':
			unit = GMT_PT;
			break;
		default:
			unit = gmtdefs.measure_unit;
			fprintf (stderr, "%s: Warning: Length unit %c not supported - revert to default unit [%s]\n", GMT_program, (int)c, GMT_unit_names[unit]);
			break;
	}

	return (unit);
}

GMT_LONG GMT_is_invalid_number (char *t)
{
	GMT_LONG i, n;

	/* Checks if t fits the format [+|-][xxxx][.][yyyy][e|E[+|-]nn]. */

	if (!t) return (TRUE);				/* Cannot be NULL */
	i = n = 0;
	if (t[i] == '+' || t[i] == '-') i++;		/* OK to have leading sign */
	if (!t[i]) return (FALSE);			/* Let a single - | + slip through without warning as they are psxy symbols */
	while (isdigit ((int)t[i])) i++, n++;		/* OK to have numbers */
	if (t[i] == '.') {				/* Found a decimal */
		i++;	/* Go to next character */
		while (isdigit ((int)t[i])) i++, n++;	/* OK to have numbers following the decimal */
	}
	/* Here n must be > 0.  Also, we might find exponential notation */
	if (t[i] == 'e' || t[i] == 'E') {
		i++;
		if (t[i] == '+' || t[i] == '-') i++;	/* OK to have leading sign for exponent */
		while (isdigit ((int)t[i])) i++;	/* OK to have numbers for the exponent */
	}
	/* If all is well we should now have run out of characters in t and n > 0 - otherwise it is an error */
	return ((t[i] || n == 0) ? TRUE : FALSE);
}

GMT_LONG GMT_hash_lookup (char *key, struct GMT_HASH *hashnode, GMT_LONG n, GMT_LONG n_hash)
{
	GMT_LONG i;
	struct GMT_HASH *this;

	i = GMT_hash (key, n_hash);

	if (i >= n || i < 0 || !hashnode[i].next) return (-1);	/* Bad key */
	this = hashnode[i].next;
	while (this && strcmp (this->key, key)) this = this->next;

	return ((this) ? this->id : -1);
}

void GMT_hash_init (struct GMT_HASH *hashnode, char **keys, GMT_LONG n_hash, GMT_LONG n_keys)
{
	GMT_LONG i, entry;
	struct GMT_HASH *this;

	/* Set up hash table */

	for (i = 0; i < n_hash; i++) hashnode[i].next = (struct GMT_HASH *)0;
	for (i = 0; i < n_keys; i++) {
		entry = GMT_hash (keys[i], n_hash);
		this = &hashnode[entry];
		while (this->next) this = this->next;
		this->next = (struct GMT_HASH *)GMT_memory (VNULL, (GMT_LONG)1, sizeof (struct GMT_HASH), GMT_program);
		this->next->key = keys[i];
		this->next->id = i;
	}
}

GMT_LONG GMT_hash (char *v, GMT_LONG n_hash)
{
	GMT_LONG h;
	for (h = 0; *v != '\0'; v++) h = (64 * h + (*v)) % n_hash;
	while (h < 0) h += n_hash;
	return (h);
}

GMT_LONG GMT_get_ellipsoid (char *name)
{
	GMT_LONG i, n;
	FILE *fp;
	char line[BUFSIZ], path[BUFSIZ];
	double slop, pol_radius;

	/* Try to ge ellipsoid from the default list */

	for (i = 0; i < GMT_N_ELLIPSOIDS; i++) {
		if (!strcmp(name, gmtdefs.ref_ellipsoid[i].name)) return (i);
	}

	i = GMT_N_ELLIPSOIDS - 1;

	/* Try to open as file first in (1) current dir, then in (2) $GMT_SHAREDIR */

	GMT_getsharepath (CNULL, name, "", path);

	if ((fp = fopen (name, "r")) == NULL && (fp = fopen (path, "r")) == NULL) {
		/* No file of that name, so parse argument instead */
		n = sscanf (name, "%lf,%s", &gmtdefs.ref_ellipsoid[i].eq_radius, line);
		if (n < 1)
			return (-1);	/* Failed to read arguments */
		else if (n == 1)
			gmtdefs.ref_ellipsoid[i].flattening = 0.0; /* Read equatorial radius only ... spherical */
		else if (line[0] == 'b') {	/* Read semi-minor axis */
			n = sscanf (&line[2], "%lf", &pol_radius);
			gmtdefs.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / gmtdefs.ref_ellipsoid[i].eq_radius);
		}
		else if (line[0] == 'f') {	/* Read flattening */
			n = sscanf (&line[2], "%lf", &gmtdefs.ref_ellipsoid[i].flattening);
		}
		else {				/* Read inverse flattening */
			n = sscanf (line, "%lf", &gmtdefs.ref_ellipsoid[i].flattening);
			if (!GMT_IS_SPHERICAL) gmtdefs.ref_ellipsoid[i].flattening = 1.0 / gmtdefs.ref_ellipsoid[i].flattening;
		}
		if (n < 1) return (-1);
	}
	else {	/* Found file, now get parameters */
		i = GMT_N_ELLIPSOIDS - 1;
		while (fgets (line, BUFSIZ, fp) && (line[0] == '#' || line[0] == '\n'));
		fclose (fp);
		n = sscanf (line, "%s %" GMT_LL "d %lf %lf %lf", gmtdefs.ref_ellipsoid[i].name, &gmtdefs.ref_ellipsoid[i].date, &gmtdefs.ref_ellipsoid[i].eq_radius, &pol_radius, &gmtdefs.ref_ellipsoid[i].flattening);
		if (n != 5) {
			fprintf (stderr, "GMT: Error decoding user ellipsoid parameters (%s)\n", line);
			GMT_exit (EXIT_FAILURE);
		}

		if (pol_radius == 0.0) {} /* Ignore semi-minor axis */
		else if (GMT_IS_SPHERICAL) {
			/* zero flattening means we must compute flattening from the polar and equatorial radii: */

			gmtdefs.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / gmtdefs.ref_ellipsoid[i].eq_radius);
			if (gmtdefs.verbose) fprintf (stderr, "GMT: user-supplied ellipsoid has implicit flattening of %.8f\n", gmtdefs.ref_ellipsoid[i].flattening);
		}
		/* else check consistency: */
		else if ((slop = fabs(gmtdefs.ref_ellipsoid[i].flattening - 1.0 + (pol_radius/gmtdefs.ref_ellipsoid[i].eq_radius))) > 1.0e-8) {
			fprintf (stderr, "GMT Warning: Possible inconsistency in user ellipsoid parameters (%s) [off by %g]\n", line, slop);
		}
	}

	return (i);
}

GMT_LONG GMT_get_datum (char *name)
{
	GMT_LONG i;

	for (i = 0; i < GMT_N_DATUMS && strcmp (name, gmtdefs.datum[i].name); i++);
	if (i == GMT_N_DATUMS) return (-1);	/* Error */
	return (i);
}

GMT_LONG GMT_key_lookup (char *name, char **list, GMT_LONG n)
{
	GMT_LONG i;

	for (i = 0; i < n && strcmp (name, list[i]); i++);
	return (i);
}

GMT_LONG GMT_font_lookup (char *name, struct GMT_FONT *list, GMT_LONG n)
{
	GMT_LONG i;

	if (name[0] >= '0' && name[0] <= '9') return (atoi (name));
	for (i = 0; i < n && strcmp (name, list[i].name); i++);
	return (i);
}

GMT_LONG GMT_load_user_media (void) {	/* Load any user-specified media formats */
	GMT_LONG n = 0, n_alloc = 0;
	double w, h;
	char line[BUFSIZ], file[BUFSIZ], media[GMT_TEXT_LEN];
	FILE *fp = NULL;

	GMT_getsharepath ("conf", "gmt_custom_media", ".conf", file);
	if ((fp = fopen (file, "r")) == NULL) return (0);

	GMT_set_meminc (GMT_TINY_CHUNK);	/* Only allocate a small amount */
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments and blank lines */

		if (sscanf (line, "%s %lg %lg", media, &w, &h) != 3) {
			fprintf (stderr, "%s: Error decoding file %s.  Bad format? [%s]\n", GMT_program, file, line);
			GMT_exit (EXIT_FAILURE);
		}

		GMT_str_tolower (media);	/* Convert string to lower case */

		if (n == n_alloc) {
			(void)GMT_alloc_memory ((void **)&GMT_user_media, n, n_alloc, sizeof (struct GMT_MEDIA), GMT_program);
			n_alloc = GMT_alloc_memory ((void **)&GMT_user_media_name, n, n_alloc, sizeof (char *), GMT_program);
		}
		GMT_user_media_name[n] = strdup (media);
		GMT_user_media[n].width  = w;
		GMT_user_media[n].height = h;
		n++;
	}
	fclose (fp);
	
	(void)GMT_alloc_memory ((void **)&GMT_user_media, 0, n, sizeof (struct GMT_MEDIA), GMT_program);
	(void)GMT_alloc_memory ((void **)&GMT_user_media_name, 0, n, sizeof (char *), GMT_program);
	GMT_reset_meminc ();

	return (n);
}

GMT_LONG GMT_get_time_system (char *name, struct GMT_TIME_SYSTEM *time_system)
{
	/* Convert TIME_SYSTEM into TIME_EPOCH and TIME_UNIT.
	   TIME_SYSTEM can be one of the following: j2000, jd, mjd, s1985, unix, dr0001, rata
	   or any string in the form "TIME_UNIT since TIME_EPOCH", like "seconds since 1985-01-01".
	   This function only splits the strings, no validation or analysis is done. See
	   GMT_init_time_system_structure for that.
	   TIME_SYSTEM = other is completely ignored.
	*/
	char *epoch;

	if (!strcmp (name, "j2000")) {
		strcpy (time_system->epoch, "2000-01-01T12:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "jd")) {
		strcpy (time_system->epoch, "-4713-11-25T12:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "mjd")) {
		strcpy (time_system->epoch, "1858-11-17T00:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "s1985")) {
		strcpy (time_system->epoch, "1985-01-01T00:00:00");
		time_system->unit = 'c';
	}
	else if (!strcmp (name, "unix")) {
		strcpy (time_system->epoch, "1970-01-01T00:00:00");
		time_system->unit = 'c';
	}
	else if (!strcmp (name, "dr0001")) {
		strcpy (time_system->epoch, "0001-01-01T00:00:00");
		time_system->unit = 'c';
	}
	else if (!strcmp (name, "rata")) {
		strcpy (time_system->epoch, "0000-12-31T00:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "other")) {
		/* Ignore completely */
	}
	else if ((epoch = strstr (name, "since"))) {
		epoch += 6;
		strcpy (time_system->epoch, epoch);
		time_system->unit = name[0];
		if (!strncmp (name, "mon", (size_t)3)) time_system->unit = 'o';
	}
	else
		return (TRUE);
	return (FALSE);
}

GMT_LONG GMT_get_char_encoding (char *name)
{
	short int i;

	for (i = 0; i < 7 && strcmp (name, GMT_weekdays[i]); i++);
	return (i);
}

GMT_LONG GMT_get_time_language (char *name)
{
	FILE *fp;
	char file[BUFSIZ], line[BUFSIZ], full[16], abbrev[16], c[16], dwu;
	GMT_LONG i, nm = 0, nw = 0, nu = 0;

	GMT_getsharepath ("time", name, ".d", file);
	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "GMT Warning: Could not load time language %s - revert to us (English)!\n", name);
		GMT_getsharepath ("time", "us", ".d", file);
		if ((fp = fopen (file, "r")) == NULL) {
			fprintf (stderr, "GMT Error: Could not find %s!\n", file);
			GMT_exit (EXIT_FAILURE);
		}
		strcpy (gmtdefs.time_language, "us");
	}

	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		sscanf (line, "%c %" GMT_LL "d %s %s %s", &dwu, &i, full, abbrev, c);
		if (dwu == 'M') {	/* Month record */
			strncpy (GMT_time_language.month_name[0][i-1], full, (size_t)16);
			strncpy (GMT_time_language.month_name[1][i-1], abbrev, (size_t)16);
			strncpy (GMT_time_language.month_name[2][i-1], c, (size_t)16);
			nm += i;
		}
		else if (dwu == 'W') {	/* Weekday record */
			strncpy (GMT_time_language.day_name[0][i-1], full, (size_t)16);
			strncpy (GMT_time_language.day_name[1][i-1], abbrev, (size_t)16);
			strncpy (GMT_time_language.day_name[2][i-1], c, (size_t)16);
			nw += i;
		}
		else {			/* Week name record */
			strncpy (GMT_time_language.week_name[0], full, (size_t)16);
			strncpy (GMT_time_language.week_name[1], abbrev, (size_t)16);
			strncpy (GMT_time_language.week_name[2], c, (size_t)16);
			nu += i;
		}
	}
	fclose (fp);
	if (! (nm == 78 && nw == 28 && nu == 1)) {	/* Sums of 1-12, 1-7, and 1, respectively */
		fprintf (stderr, "GMT Error: Mismatch between expected and actual contents in %s!\n", file);
		GMT_exit (EXIT_FAILURE);
	}

	for (i = 0; i < 12; i++) {	/* Get upper-case abbreviated month names for i/o */
		strcpy (month_names[i], GMT_time_language.month_name[1][i]);
		GMT_str_toupper (month_names[i]);
		months[i] = month_names[i];
	}
	GMT_hash_init (GMT_month_hashnode, months, 12, 12);
	return (GMT_NOERROR);
}

void GMT_setshorthand (void) {/* Read user's .gmt_io file and initialize shorthand notation */
	GMT_LONG n = 0, n_alloc = 0;
	char file[BUFSIZ], line[BUFSIZ], a[GMT_TEXT_LEN], b[GMT_TEXT_LEN], c[GMT_TEXT_LEN], d[GMT_TEXT_LEN], e[GMT_TEXT_LEN];
	FILE *fp = NULL;

	GMT_n_file_suffix = 0;	/* By default there are no shorthands unless .gmt_io is found */
	
	if (!GMT_getuserpath (".gmt_io", file)) return;
	if ((fp = fopen (file, "r")) == NULL) return;

	GMT_set_meminc (GMT_TINY_CHUNK);	/* Only allocate a small amount */
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		if (sscanf (line, "%s %s %s %s %s", a, b, c, d, e) != 5) {
			fprintf (stderr, "%s: Error decoding file %s.  Bad format? [%s]\n", GMT_program, file, line);
			GMT_exit (EXIT_FAILURE);
		}
		
		if (n == n_alloc) {
			(void)GMT_alloc_memory ((void **)&GMT_file_id, n, n_alloc, sizeof (GMT_LONG), GMT_program);
			(void)GMT_alloc_memory ((void **)&GMT_file_suffix, n, n_alloc, sizeof (char *), GMT_program);
			n_alloc = GMT_alloc_memory3 ((void **)&GMT_file_scale, (void **)&GMT_file_offset, (void **)&GMT_file_nan, n, n_alloc, sizeof (double), GMT_program);
		}
		
		GMT_file_suffix[n] = strdup (a);
		GMT_file_id[n] = GMT_grd_format_decoder (b);
		GMT_file_scale[n] = (strcmp (c, "-")) ? atof (c) : 1.0;
		GMT_file_offset[n] = (strcmp (d, "-")) ? atof (d) : 0.0;
		GMT_file_nan[n] = (strcmp (e, "-")) ? atof (e) : GMT_d_NaN;
		n++;
	}
	fclose (fp);
	
	GMT_n_file_suffix = n;
	GMT_reset_meminc ();
	(void)GMT_alloc_memory ((void **)&GMT_file_id, 0, n, sizeof (GMT_LONG), GMT_program);
	(void)GMT_alloc_memory ((void **)&GMT_file_suffix, 0, n, sizeof (char *), GMT_program);
	(void)GMT_alloc_memory3 ((void **)&GMT_file_scale, (void **)&GMT_file_offset, (void **)&GMT_file_nan, 0, n, sizeof (double), GMT_program);
}

void GMT_freeshorthand (void) {/* Free memory used by shorthand arrays */
	GMT_LONG i;

	if (GMT_n_file_suffix == 0) return;

	for (i = 0; i < GMT_n_file_suffix; i++) free ((void *)GMT_file_suffix[i]);
	GMT_free ((void *)GMT_file_id);
	GMT_free ((void *)GMT_file_scale);
	GMT_free ((void *)GMT_file_offset);
	GMT_free ((void *)GMT_file_nan);
	GMT_free ((void *)GMT_file_suffix);
}

void GMT_begin_io ()
{	/* Do this so we can call this separately in gmtdefaults/gmtset and avoid the wrath of WIN DLL */
	GMT_stdin  = stdin;
	GMT_stdout = stdout;
}

GMT_LONG GMT_begin (int argc, char **argv)
{
	/* GMT_begin will merge the command line arguments with the arguments
	 * that were used the last time this program was called (if any).  This
	 * way one only has to give -R -J to repeat previous map projection
	 * instead of spelling out the -R and projection parameters every time.
	 * The information is stored in the first line of the file .gmtcommands4
	 * in the current directory [or optionally a provided filename] and will
	 * contain the last arguments to the common parameters like
	 * -B, -H, -J, -K, -O, -P, -R, -U, -V, -X, -Y, -c
	 * Since the meaning of -X/-Y depends on whether we have an overlay,
	 * we maintain -X -Y for absolute shifts and -x -y for relative shifts.
	 * If the argument +file is encountered then file is used in lieu of the
	 * usual .gmtdefaults4 file and this argument is chopped from argv
	 */

	GMT_LONG i, j, k, n;
	char *this;

#ifdef CYGWIN_LOCALE_TROUBLE
	setlocale (LC_CTYPE, "C.ASCII");
#endif

#ifdef DEBUG
	GMT_memtrack_init (&GMT_mem_keeper);
#endif

#ifdef __FreeBSD__
#ifdef _i386_
	/* allow divide by zero -- Inf */
	fpsetmask (fpgetmask () & ~(FP_X_DZ | FP_X_INV));
#endif
#endif
	GMT = (struct GMT_CTRL *)New_GMT_Ctrl ();	/* Allocate and initialize a new common control structure */

	/* Initialize parameters that don'd depend on .gmtdefaults */

	GMT_begin_io ();
	GMT_set_home ();

	GMT_init_fonts (&GMT_N_FONTS);
	memset ((void *)&GMT_ps, 0, sizeof (struct GMT_PS));
	this = CNULL;
	GMT_make_fnan (GMT_f_NaN);
	GMT_make_dnan (GMT_d_NaN);
	frame_info.plot = FALSE;
	project_info.projection = GMT_NO_PROJ;
	project_info.gave_map_width = 0;
	project_info.region = TRUE;
	project_info.compute_scale[0] =  project_info.compute_scale[1] = project_info.compute_scale[2] = FALSE;
	project_info.x_off_supplied = project_info.y_off_supplied = FALSE;
	project_info.region_supplied = FALSE;
	for (j = 0; j < 10; j++) project_info.pars[j] = 0.0;
	project_info.xmin = project_info.ymin = 0.0;
	project_info.z_level = DBL_MAX;	/* Will be set in map_setup */
	project_info.xyz_pos[0] = project_info.xyz_pos[1] = TRUE;
	memset ((void *)&GMT_grd_info, 0, sizeof (struct GMT_GRD_INFO));
	GMT_prepare_3D ();
	GMT_dlon = (project_info.e - project_info.w) / GMT_n_lon_nodes;
	GMT_dlat = (project_info.n - project_info.s) / GMT_n_lat_nodes;
	for (i = 0; i < 4; i++) project_info.edge[i] = TRUE;
	GMT_grdio_init ();
	for (i = strlen(argv[0]); i >= 0 && argv[0][i] != '/'; i--);
	GMT_program = &argv[0][i+1];
	frame_info.check_side = frame_info.horizontal = FALSE;
	project_info.f_horizon = 90.0;
	GMT_distance_func = (PFD) GMT_great_circle_dist_km;

	/* Set the gmtdefault parameters from the $HOME/.gmtdefaults4 (if any) */

	/* See if user specified +optional_defaults_file.  If so, assign filename to this and remove option from argv */

	for (i = j = 1; i < argc; i++) {
		GMT_chop (argv[i]);	/* Remove unlikely \n or \r due to cross-OS binaries (e.g. Cygwin using Win32 executables) */
		argv[j] = argv[i];
		if (argv[j][0] == '+' && argv[i][1] && !access (&argv[i][1], R_OK))
			this = &argv[i][1];
		else
			j++;
	}
	argc = (int)j;

	GMT_getdefaults (this);

	/* See if user specified -- defaults on the command line [Per Dave Ball's suggestion].
	 * If found, apply option and remove from argv.  These options only apply to current process. */

	GMT_hash_init (keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);
	for (i = j = 1, n = 0; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] ) {
			if ((this = strchr (argv[i], '='))) {	/* Got --PAR=VALUE */
				this[0] = '\0';
				n += GMT_setparameter (&argv[i][2], &this[1]);
			}
			else				/* Got --PAR */
				n += GMT_setparameter (&argv[i][2], "TRUE");
		}
		else
			argv[j++] = argv[i];
	}
	argc = (int)j;
	GMT_free_hash (keys_hashnode, GMT_N_KEYS);	/* Done with this for now */
	if (n) fprintf (stderr, "%s:  %ld conversion errors from command-line default override settings!\n", GMT_program, n);

	GMT_history (argc, argv);	/* Process and store command shorthands */

	GMT_PS_init ();		/* Init the PostScript-related parameters */

	GMT_init_ellipsoid ();	/* Set parameters depending on the ellipsoid */

	i = GMT_init_time_system_structure (&gmtdefs.time_system);
	if (i & 1) {
		fprintf (stderr, "%s: GMT Warning:  gmtdefaults TIME_UNIT is invalid.  Default assumed.\n", GMT_program);
		fprintf (stderr, "    Choose one only from y o d h m c\n");
		fprintf (stderr, "    Corresponding to year month day hour minute second\n");
		fprintf (stderr, "    Note year and month are simply defined (365.2425 days and 1/12 of a year)\n");
	}
	if (i & 2) {
		fprintf (stderr, "%s: GMT Warning:  TIME_EPOCH format is invalid.  Default assumed.\n", GMT_program);
		fprintf (stderr, "    A correct format has the form [-]yyyy-mm-ddThh:mm:ss[.xxx]\n");
		fprintf (stderr, "    or (using ISO weekly calendar)   yyyy-Www-dThh:mm:ss[.xxx]\n");
		fprintf (stderr, "    An example of a correct format is:  2000-01-01T12:00:00\n");
	}

	if (GMT_got_frame_rgb) {	/* Must enforce change of frame, tick, and grid pen rgb */
		memcpy ((void *)gmtdefs.frame_pen.rgb, (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)gmtdefs.tick_pen.rgb,  (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)gmtdefs.grid_pen[0].rgb,   (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)gmtdefs.grid_pen[1].rgb, (void *)gmtdefs.basemap_frame_rgb, (size_t)(3 * sizeof (int)));
	}

	GMT_set_inside_border ();
	
	GMT_io_init ();			/* Init the table i/o structure */

	GMT_get_time_language (gmtdefs.time_language);

	if (gmtdefs.gridfile_shorthand) GMT_setshorthand ();

	/* Copy various colors to GMT_BFN_COLOR structure */

	memset ((void *)(&GMT_bfn), 0, 3*sizeof (struct GMT_BFN_COLOR));
	memcpy ((void *)GMT_bfn[GMT_FGD].rgb, (void *)gmtdefs.foreground_rgb, 3 * sizeof (int));
	memcpy ((void *)GMT_bfn[GMT_BGD].rgb, (void *)gmtdefs.background_rgb, 3 * sizeof (int));
	memcpy ((void *)GMT_bfn[GMT_NAN].rgb, (void *)gmtdefs.nan_rgb, 3 * sizeof (int));
	for (k = 0; k < 3; k++) if (GMT_bfn[k].rgb[0] == -1) GMT_bfn[k].skip = TRUE;

	/* Make sure -b options are parsed first in case filenames are given
	 * before -b options on the command line.  This would only cause grief
	 * under WIN32. Also make -J come first and -R before -I, if present.
	 * We also run -f through in case -: is given.
	 * Finally, we look for -V so verbose is set prior to testing arguments */

	GMT_sort_options (argc, argv, "VJRIbf:");

	return (argc);
}

#ifdef MIRONE 
GMT_LONG GMT_short_begin (int argc, char **argv) {
	GMT_LONG i, j, n;
	char *this;

#ifdef DEBUG
	GMT_memtrack_init (&GMT_mem_keeper);
#endif

	GMT = (struct GMT_CTRL *)New_GMT_Ctrl ();	/* Allocate and initialize a new common control structure */

	/* Initialize parameters that don'd depend on .gmtdefaults */

	this = CNULL;
	frame_info.plot = FALSE;
	project_info.projection = GMT_NO_PROJ;
	project_info.gave_map_width = 0;
	project_info.region = TRUE;
	project_info.compute_scale[0] =  project_info.compute_scale[1] = project_info.compute_scale[2] = FALSE;
	project_info.x_off_supplied = project_info.y_off_supplied = FALSE;
	project_info.region_supplied = FALSE;
	for (j = 0; j < 10; j++) project_info.pars[j] = 0.0;
	project_info.xmin = project_info.ymin = 0.0;
	project_info.z_level = DBL_MAX;	/* Will be set in map_setup */
	project_info.xyz_pos[0] = project_info.xyz_pos[1] = TRUE;
	for (i = 0; i < 4; i++) project_info.edge[i] = TRUE;
	for (i = strlen(argv[0]); i >= 0 && argv[0][i] != '/'; i--);
	GMT_program = &argv[0][i+1];
	frame_info.check_side = frame_info.horizontal = FALSE;
	project_info.f_horizon = 90.0;
	GMT_distance_func = (PFD) GMT_great_circle_dist_km;

	/* Set the gmtdefault parameters from the $HOME/.gmtdefaults4 (if any) */

	/* See if user specified +optional_defaults_file.  If so, assign filename to this and remove option from argv */

	for (i = j = 1; i < argc; i++) {
		argv[j] = argv[i];
		if (argv[j][0] == '+' && argv[i][1] && !access (&argv[i][1], R_OK))
			this = &argv[i][1];
		else
			j++;
	}
	argc = (int)j;

	GMT_getdefaults (this);

	/* See if user specified -- defaults on the command line [Per Dave Ball's suggestion].
	 * If found, apply option and remove from argv.  These options only apply to current process. */

	GMT_hash_init (keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);
	for (i = j = 1, n = 0; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] ) {
			if ((this = strchr (argv[i], '='))) {	/* Got --PAR=VALUE */
				this[0] = '\0';
				n += GMT_setparameter (&argv[i][2], &this[1]);
			}
			else				/* Got --PAR */
				n += GMT_setparameter (&argv[i][2], "TRUE");
		}
		else
			argv[j++] = argv[i];
	}
	argc = (int)j;
	GMT_free_hash (keys_hashnode, GMT_N_KEYS);	/* Done with this for now */

	GMT_init_ellipsoid ();	/* Set parameters depending on the ellipsoid */

	GMT_io_init ();		/* Init the table i/o structure */

	GMT_get_time_language (gmtdefs.time_language);

	/* Make sure -b options are parsed first in case filenames are given
	 * before -b options on the command line.  This would only cause grief
	 * under WIN32. Also make -J come first and -R before -I, if present.
	 * We also run -f through in case -: is given.
	 * Finally, we look for -V so verbose is set prior to testing arguments */

	GMT_sort_options (argc, argv, "VJRIbf:");

	return (argc);
}


void GMT_end_for_mex (int argc, char **argv) {
	/* Special version to be used in MEXs. */

	GMT_LONG i, j;

	for (i = 0; i < GMT_N_UNIQUE; i++) if (GMT_oldargv[i]) GMT_free ((void *)GMT_oldargv[i]);
	GMT_free_plot_array ();
	/* Remove allocated hash structures */
	GMT_free_hash (GMT_month_hashnode, 12);
	/*GMT_free_hash (GMT_rgb_hashnode, GMT_N_COLOR_NAMES);*/
	GMT_free_custom_symbols();

	if (GMT_io.skip_if_NaN) GMT_free ((void *)GMT_io.skip_if_NaN);
	if (GMT_io.in_col_type) GMT_free ((void *)GMT_io.in_col_type);
	if (GMT_io.out_col_type) GMT_free ((void *)GMT_io.out_col_type);

	if (project_info.n_x_coeff) GMT_free ((void *)project_info.n_x_coeff);
	if (project_info.n_y_coeff) GMT_free ((void *)project_info.n_y_coeff);
	if (project_info.n_iy_coeff) GMT_free ((void *)project_info.n_iy_coeff);

	for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) if (GMT_plot_format[i][j]) GMT_free ((void *)GMT_plot_format[i][j]);

	if (GMT_n_colors) {
		for (i = 0; i < GMT_n_colors; i++) {
			if (GMT_lut[i].label) GMT_free ((void *)GMT_lut[i].label);
			if (GMT_lut[i].fill) GMT_free ((void *)GMT_lut[i].fill);
		}
		GMT_free ((void *)GMT_lut);
	}
	for (i = 0; i < 3; i++) if (GMT_bfn[i].fill) GMT_free ((void *)GMT_bfn[i].fill);

	Free_GMT_Ctrl (GMT);	/* Deallocate control structure */

#ifdef DEBUG
	GMT_memtrack_report (GMT_mem_keeper);
#endif

}
#endif		/* endif ifdef MIRONE */

void GMT_end (int argc, char **argv)
{
	/* GMT_end will clean up after us. */

	GMT_LONG i, j;

	for (i = 0; i < GMT_N_UNIQUE; i++) if (GMT_oldargv[i]) free ((void *)GMT_oldargv[i]);
	GMT_free_plot_array ();
	/* Remove allocated hash structures */
	GMT_free_hash (GMT_month_hashnode, 12);
	GMT_free_hash (GMT_rgb_hashnode, GMT_N_COLOR_NAMES);
	for (i = 0; i < GMT_N_FONTS; i++) free ((void *)GMT_font[i].name);
	GMT_free ((void *)GMT_font);
	GMT_free_custom_symbols();
#ifdef __FreeBSD__
#ifdef _i386_
	fpresetsticky (FP_X_DZ | FP_X_INV);
	fpsetmask (FP_X_DZ | FP_X_INV);
#endif
#endif

	if (GMT_io.skip_if_NaN) GMT_free ((void *)GMT_io.skip_if_NaN);
	if (GMT_io.in_col_type) GMT_free ((void *)GMT_io.in_col_type);
	if (GMT_io.out_col_type) GMT_free ((void *)GMT_io.out_col_type);

	GMT_free ((void *)GMT_SHAREDIR);
	GMT_free ((void *)GMT_HOMEDIR);
	if (GMT_USERDIR) GMT_free ((void *)GMT_USERDIR);
	if (GMT_GRIDDIR) free ((void *)GMT_GRIDDIR);
	if (GMT_IMGDIR) free ((void *)GMT_IMGDIR);
	if (GMT_DATADIR) free ((void *)GMT_DATADIR);
	if (GMT_TMPDIR) free ((void *)GMT_TMPDIR);
	if (project_info.n_x_coeff) GMT_free ((void *)project_info.n_x_coeff);
	if (project_info.n_y_coeff) GMT_free ((void *)project_info.n_y_coeff);
	if (project_info.n_iy_coeff) GMT_free ((void *)project_info.n_iy_coeff);

	for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) if (GMT_plot_format[i][j]) GMT_free ((void *)GMT_plot_format[i][j]);

	if (GMT_n_colors) {
		for (i = 0; i < GMT_n_colors; i++) {
			if (GMT_lut[i].label) GMT_free ((void *)GMT_lut[i].label);
			if (GMT_lut[i].fill) GMT_free ((void *)GMT_lut[i].fill);
		}
		GMT_free ((void *)GMT_lut);
	}
	for (i = 0; i < 3; i++) if (GMT_bfn[i].fill) GMT_free ((void *)GMT_bfn[i].fill);
	if (gmtdefs.gridfile_shorthand) GMT_freeshorthand ();

	fflush (GMT_stdout);	/* Make sure output buffer is flushed */

	Free_GMT_Ctrl (GMT);	/* Deallocate control structure */

#ifdef DEBUG
	GMT_memtrack_report (GMT_mem_keeper);
#endif
}

void GMT_set_inside_border (void)
{
	if (gmtdefs.basemap_type == GMT_IS_INSIDE) {	/* Prepare for inside map annotations/ticking */
		gmtdefs.annot_offset[0] = -fabs (gmtdefs.annot_offset[0]);
		gmtdefs.annot_offset[1] = -fabs (gmtdefs.annot_offset[1]);
		gmtdefs.label_offset = -fabs (gmtdefs.label_offset);
		gmtdefs.header_offset = -fabs (gmtdefs.header_offset);
		gmtdefs.tick_length = -fabs (gmtdefs.tick_length);
	}
}

void GMT_free_hash (struct GMT_HASH *hashnode, GMT_LONG n_items) {
	GMT_LONG i;
	struct GMT_HASH *p, *current;

	if (!hashnode) return;	/* Nothing to free */
	for (i = 0; i < n_items; i++) {
		p = hashnode[i].next;
		while (p) {
			current = p;
			p = p->next;
			GMT_free ((void *)current);
		}
	}
}

void GMT_set_home (void)
{
	char *this;

	/* Determine GMT_SHAREDIR (directory containing coast, cpt, etc. subdirectories) */

	if ((this = getenv ("GMT_SHAREDIR")) != CNULL) {	/* GMT_SHAREDIR was set */
		GMT_SHAREDIR = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 1), sizeof (char), "GMT");
		strcpy (GMT_SHAREDIR, this);
	}
	else {	/* Default is GMT_SHARE_PATH */
		GMT_SHAREDIR = (char *) GMT_memory (VNULL, (size_t)(strlen (GMT_SHARE_PATH) + 1), sizeof (char), "GMT");
		strcpy (GMT_SHAREDIR, GMT_SHARE_PATH);
	}

	/* Determine GMT_HOMEDIR (user home directory) */

	if ((this = getenv ("HOME")) != CNULL) {	/* HOME was set */
		GMT_HOMEDIR = (char *) GMT_memory (VNULL, (size_t)(strlen (this)+1), sizeof (char), "GMT");
		strcpy (GMT_HOMEDIR, this);
	}
#ifdef WIN32
	else if ((this = getenv ("HOMEPATH")) != CNULL) {	/* HOMEPATH was set */
		GMT_HOMEDIR = (char *) GMT_memory (VNULL, (size_t)(strlen (this)+1), sizeof (char), "GMT");
		strcpy (GMT_HOMEDIR, this);
	}
#endif
	else
		fprintf (stderr, "GMT Warning: Could not determine home directory!\n");

	/* Determine GMT_USERDIR (directory containing user replacements contents in GMT_SHAREDIR) */

	if ((this = getenv ("GMT_USERDIR")) != CNULL) {	/* GMT_USERDIR was set */
		GMT_USERDIR = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 1), sizeof (char), "GMT");
		strcpy (GMT_USERDIR, this);
	}
	else if (GMT_HOMEDIR) {	/* Use default path for GMT_USERDIR (~/.gmt) */
		GMT_USERDIR = (char *) GMT_memory (VNULL, (size_t)(strlen (GMT_HOMEDIR) + 6), sizeof (char), "GMT");
		sprintf (GMT_USERDIR, "%s%c%s", GMT_HOMEDIR, DIR_DELIM, ".gmt");
	}
	if (access(GMT_USERDIR,R_OK)) {
		GMT_free ((void *) GMT_USERDIR);
		GMT_USERDIR = CNULL;
	}

	/* Check if obsolete GMT_CPTDIR was specified */

	if ((this = getenv ("GMT_CPTDIR")) != CNULL) {	/* GMT_CPTDIR was set */
		fprintf (stderr, "GMT WARNING: Environment variable GMT_CPTDIR was set but is no longer used by GMT.\n");
		fprintf (stderr, "GMT WARNING: System-wide color tables are in %s/cpt.\n", GMT_SHAREDIR);
		fprintf (stderr, "GMT WARNING: Use GMT_USERDIR (%s) instead and place user-defined color tables there.\n", GMT_USERDIR);
	}

	/* Determine GMT_DATADIR, GMT_GRIDDIR, GMT_IMGDIR (data directories) */

	if ((this = getenv ("GMT_DATADIR")) != CNULL) {	/* GMT_DATADIR was set */
		if (!strchr (this, PATH_DELIM) && access(this,R_OK))	/* A single directory, but cannot be accessed */
			GMT_DATADIR = CNULL;
		else {	/* A list of directories */
			GMT_DATADIR = strdup (this);
		}
	}
	if ((this = getenv ("GMT_GRIDDIR")) != CNULL) {	/* GMT_GRIDDIR was set */
		if (access(this,R_OK))
			GMT_GRIDDIR = CNULL;
		else {
			GMT_GRIDDIR = strdup (this);
		}
	}
	if ((this = getenv ("GMT_IMGDIR")) != CNULL) {	/* GMT_IMGDIR was set */
		if (access(this,R_OK))
			GMT_IMGDIR = CNULL;
		else {
			GMT_IMGDIR = strdup (this);
		}
	}

	/* Determine GMT_TMPDIR (for isolation mode). Needs to exist use it. */

	if ((this = getenv("GMT_TMPDIR")) != CNULL) {	/* GMT_TMPDIR was set */
		if (access(this,R_OK+W_OK+X_OK)) {
			fprintf (stderr, "GMT WARNING: Environment variable GMT_TMPDIR was set to %s, but directory is not accessible.\n", this);
			fprintf (stderr, "GMT WARNING: GMT_TMPDIR needs to have mode rwx. Isolation mode switched off.\n");
			GMT_TMPDIR = CNULL;
		}
		else {
			GMT_TMPDIR = strdup (this);
		}
	}
}

GMT_LONG GMT_history (int argc, char ** argv)
{
	GMT_LONG i, j, k;
	GMT_LONG need_xy = FALSE, overlay = FALSE, found_old, found_new, done = FALSE, new_unique = FALSE;
	char line[BUFSIZ], hfile[BUFSIZ], cwd[BUFSIZ];
	char *newargv[GMT_N_UNIQUE], *new_j = CNULL, *old_j = CNULL, *not_used = NULL;
	FILE *fp;	/* For .gmtcommands4 file */
#ifdef FLOCK
	struct flock lock;
#endif

	if (!gmtdefs.history) return (GMT_NOERROR);	/* .gmtcommands4 mechanism has been disabled */

	/* Open .gmtcommands4 file and retrieve old argv (if any)
	 * This is tricky since GMT programs are often hooked together
	 * with pipes so it actually has happened that the first program
	 * is updating the .gmtcommands4 file while the second tries to
	 * read.  The result is that the second program often cannot expand
	 * the shorthand and fails with error message.  In GMT 3.1 we introduced
	 * Advisory File Locking and also update the .gmtcommands4 file as soon
	 * as possible so that commands down the pipe will see the new options
	 * already have taken effect.
	 *
	 * In GMT 4.1.5 GMT_get_history and GMT_put_history got consolidated into GMT_history
	 * in an attempt to lower the chances that in a pipe one program writes to .gmtcommands4
	 * while another is reading from it (despite the lock).
	 */

	for (i = 0; i < GMT_N_UNIQUE; i++) newargv[i] = CNULL;

	/* If current directory is writable, use it; else use the home directory */

	not_used = getcwd (cwd, (size_t)BUFSIZ);
	if (GMT_TMPDIR)			/* Isolation mode: Use GMT_TMPDIR/.gmtcommands4 */
		sprintf (hfile, "%s%c.gmtcommands4", GMT_TMPDIR, DIR_DELIM);
	else if (!access (cwd, W_OK))	/* Current directory is writable */
		sprintf (hfile, ".gmtcommands4");
	else	/* Try home directory instead */
		sprintf (hfile, "%s%c.gmtcommands4", GMT_HOMEDIR, DIR_DELIM);

	GMT_oldargc = 0;
	for (i = 0; i < GMT_N_UNIQUE; i++) GMT_oldargv[i] = CNULL;

	if ((fp = fopen (hfile, "r+")) != NULL) {

		/* When we get here the file exists */
#ifdef FLOCK
		GMT_file_lock (fileno(fp), &lock);
#endif
		/* Get the common arguments and copy them to array GMT_oldargv */
		/* PS!  GMT_oldarg? must remain global and not freed until GMT_end - otherwise argv will point to junk */

		while (!done && fgets (line, BUFSIZ, fp)) {

			if (line[0] == '#') continue;	/* Skip comments lines */
			GMT_chop (line);		/* Get rid of EOL */
			if (line[0] == '\0') continue;	/* Skip blank lines */
			if (!strncmp (line, "EOF", (size_t)3)) {	/* Logical end of .gmtcommands4 file */
				done = TRUE;
				continue;
			}
			if (line[0] != '-') continue;	/* Possibly reading old .gmtcommands4 format or junk */
			GMT_oldargv[GMT_oldargc] = strdup (line);
			if (GMT_oldargv[GMT_oldargc][1] == 'j') old_j = GMT_oldargv[GMT_oldargc];
			GMT_oldargc++;
			if (GMT_oldargc > GMT_N_UNIQUE) {
				fprintf (stderr, "GMT Fatal Error: Failed while decoding common arguments\n");
				GMT_exit (EXIT_FAILURE);
			}
		}

		/* Close the file */
#ifdef FLOCK
		GMT_file_unlock (fileno(fp), &lock);
#endif
		fclose (fp);
	}

	/* See if (1) We need abs/rel shift and (2) if we have an overlay */

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') continue;
		if (argv[i][1] == 'X' || argv[i][1] == 'Y' || argv[i][1] == 'x' || argv[i][1] == 'y') need_xy = TRUE;
		if (argv[i][1] == 'O' || argv[i][1] == 'o') overlay = TRUE;
	}

	overlay = (need_xy && overlay); /* -O means overlay only if -X or -Y has been specified */

	/* Change x/y to upper case if no overlay and change X/Y to lower case if overlay */

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') continue;
		if (overlay) {
			if (argv[i][1] == 'X') argv[i][1] = 'x';
			if (argv[i][1] == 'Y') argv[i][1] = 'y';
		}
		else {
			if (argv[i][1] == 'x') argv[i][1] = 'X';
			if (argv[i][1] == 'y') argv[i][1] = 'Y';
		}
	}

	/* Loop over argv and merge argv and GMT_oldargv */

	for (i = 1; i < argc; i++) {

		/* Skip if filename or option has modifiers */

		if (argv[i][0] != '-') continue;			/* Not an option argument */
		if (argv[i][1] != 'J' && argv[i][2] != 0) continue;	/* 1-char option with modifier */
		if (argv[i][1] == 'J' && argv[i][2] != 0 && argv[i][3] != 0) continue;	/* 2-char option with modifier */

		for (j = 0, found_old = FALSE; !found_old && j < GMT_oldargc; j++) {
			if (argv[i][1] == 'J' && argv[i][2] == '\0')	/* Given -J only, match with -j */
				found_old = (GMT_oldargv[j][1] == 'j');
			else if (argv[i][1] == 'J')			/* Given -J? */
				found_old = (GMT_oldargv[j][1] == argv[i][1] && GMT_oldargv[j][2] == argv[i][2]);
			else						/* All other options */
				found_old = (GMT_oldargv[j][1] == argv[i][1]);
		}
		j--;

		/* Skip if not found or GMT_oldargv has no modifiers */

		if (!found_old) continue;

		if (argv[i][1] != 'J' && GMT_oldargv[j][2] == 0) continue;
		if (argv[i][1] == 'J' && GMT_oldargv[j][3] == 0) continue;

		/* Here, GMT_oldargv has modifiers and argv doesn't, set pointer */

		argv[i] = GMT_oldargv[j];

		if (argv[i][1] == 'j') argv[i][1] = 'J';	/* Reset to upper case */
	}

	/* Loop over GMT_unique_option parameters to determine if common arguments changed */

	for (i = 0; i < GMT_N_UNIQUE; i++) {
		for (j = 0, found_new = FALSE; !found_new && j < argc; j++) {
			if (argv[j][0] != '-')
				continue;
			else if (GMT_unique_option[i][0] == 'J') /* Range of -J? options */
				found_new = !strncmp (&argv[j][1], GMT_unique_option[i], (size_t)2);
			else
				found_new = (argv[j][1] == GMT_unique_option[i][0]);
		}
		j--;
		if (found_new) {
			newargv[i] = argv[j];
			/* Make this the last -J? of any kind (except -JZ), identified by lower-case -j */
			if (GMT_unique_option[i][0] == 'J' && toupper((int)GMT_unique_option[i][1]) != 'Z') new_j = argv[j];
		}

		for (k = 0, found_old = FALSE; !found_old && k < GMT_oldargc; k++) {
			if (GMT_oldargv[k][0] != '-')
				continue;
			else if (GMT_unique_option[i][0] == 'J') /* Range of -J? options */
				found_old = !strncmp (&GMT_oldargv[k][1], GMT_unique_option[i], (size_t)2);
			else
				found_old = (GMT_oldargv[k][1] == GMT_unique_option[i][0]);
		}
		k--;

		if (newargv[i]) {
			if (!found_old || strcmp (newargv[i], GMT_oldargv[k])) new_unique = TRUE;
		}
		else if (found_old)
			newargv[i] = GMT_oldargv[k];
	}

	/* Finally check if default projection changed (-j) */

	if (new_j) {
		if (!old_j || strcmp (&new_j[2], &old_j[2])) new_unique = TRUE;
	}
	else if (old_j)
		new_j = old_j;

	/* Only write to .gmtcommands4 file if something changed */

	if (!new_unique) return (GMT_NOERROR);

	if (access (hfile, R_OK)) {    /* No .gmtcommands4 file in chosen directory, try to make one */
		if ((fp = fopen (hfile, "w")) == NULL) {
			fprintf (stderr, "GMT Warning: Could not create %s [permission problem?]\n", hfile);
			return (GMT_NOERROR);
		}
	}
	else if ((fp = fopen (hfile, "r+")) == NULL) {
		fprintf (stderr, "GMT Warning: Could not update %s [permission problem?]\n", hfile);
		return (GMT_NOERROR);
	}
#ifdef FLOCK
	GMT_file_lock(fileno(fp), &lock);
#endif

	fprintf (fp, "# GMT common arguments shelf\n");
	for (i = 0; i < GMT_N_UNIQUE; i++) {
		if (newargv[i]) fprintf (fp, "%s\n", newargv[i]);
	}
	if (new_j) fprintf (fp, "-j%s\n", &new_j[2]);
	fprintf (fp, "EOF\n");	/* Logical end of file marker (since old file may be longer) */

	fflush (fp);		/* To ensure all is written when lock is released */
#ifdef FLOCK
	GMT_file_unlock(fileno(fp), &lock);
#endif
	fclose (fp);

	return (GMT_NOERROR);
}

#ifdef FLOCK
void GMT_file_lock (int fd, struct flock *lock)
{
	lock->l_type = F_WRLCK;		/* Lock for [exclusive] reading/writing */
	lock->l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock->l_start = lock->l_len = 0;

	if (fcntl (fd, F_SETLKW, lock)) {	/* Will wait for file to be ready for reading */
		fprintf (stderr, "%s: Error returned by fcntl [F_WRLCK]\n", GMT_program);
		GMT_exit (EXIT_FAILURE);
	}
}

void GMT_file_unlock (int fd, struct flock *lock)
{
	lock->l_type = F_UNLCK;		/* Release lock and close file */
	lock->l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock->l_start = lock->l_len = 0;

	if (fcntl (fd, F_SETLK, lock)) {
		fprintf (stderr, "%s: Error returned by fcntl [F_UNLCK]\n", GMT_program);
		GMT_exit (EXIT_FAILURE);
	}
}
#endif

void GMT_PS_init (void) {		/* Init the PostScript-related parameters */

	/* Some of these might be modified later by -K, -O, -P, -U, -V, -X, -Y, -c.
	 * Must be called BEFORE processing common arguments, since they modify the  GMT_ps struct.
	 * But must be called AFTER processing --OPT=var, since they modify the gmtdefs struct. */

	GMT_ps.portrait = gmtdefs.portrait;		/* TRUE for portrait, FALSE for landscape */
	GMT_ps.verbose = gmtdefs.verbose;		/* TRUE to give verbose feedback from pslib routines [FALSE] */
	GMT_ps.heximage = gmtdefs.ps_heximage;		/* TRUE to write images in ASCII, FALSE in BIN [TRUE] */
	GMT_ps.absolute = FALSE;			/* TRUE if -X, -Y was absolute [FALSE] */
	GMT_ps.last_page = TRUE;			/* Result of not -K [TRUE] */
	GMT_ps.overlay = FALSE;				/* Result of -O [FALSE] */
	GMT_ps.comments = gmtdefs.ps_verbose;		/* TRUE to write comments to PS file [FALSE] */
	GMT_ps.clip = 0;				/* Used to manage multi-process clipping operations [0] */
	GMT_ps.n_copies = gmtdefs.n_copies;		/* Result of -c [gmtdefs.n_copies] */
	GMT_ps.colormode = gmtdefs.ps_colormode;	/* 0 (RGB), 1 (CMYK), 2 (HSV) */
	GMT_ps.compress = gmtdefs.ps_compress;		/* 0 (none), 1 (RLE), 2 (LZW) */
	GMT_ps.line_cap = gmtdefs.ps_line_cap;		/* 0 (butt), 1 (round), 2 (square) */
	GMT_ps.line_join = gmtdefs.ps_line_join;	/* 0 (miter), 1 (round), 2 (bevel) */
	GMT_ps.miter_limit = gmtdefs.ps_miter_limit;	/* 0-180 degrees as whole integer */
	GMT_ps.dpi = gmtdefs.dpi;			/* Plotter resolution in dots-per-inch */
	memcpy ((void *)GMT_ps.paper_width, (void *)gmtdefs.paper_width, 2 * sizeof (double));	/* Physical width and height of paper used in points */
	memcpy ((void *)GMT_ps.page_rgb, (void *)gmtdefs.page_rgb, 3*sizeof (int));		/* array with Color of page (paper) */
	GMT_ps.x_origin = gmtdefs.x_origin;		/* Result of -X [gmtdefs.x_origin] */
	GMT_ps.y_origin = gmtdefs.y_origin;		/* Result of -Y [gmtdefs.y_origin] */
	GMT_ps.x_scale = gmtdefs.global_x_scale;	/* Copy of gmtdefs.global_y_scale */
	GMT_ps.y_scale = gmtdefs.global_y_scale;	/* Copy of gmtdefs.global_y_scale */
	GMT_ps.unix_time = gmtdefs.unix_time;		/* Result of -U [gmtdefs.unix_time] */
	GMT_ps.unix_time_just = gmtdefs.unix_time_just;	/* Result of -U [gmtdefs.unit_time_justify] */
	memcpy ((void *)GMT_ps.unix_time_pos, (void *)gmtdefs.unix_time_pos, 2*sizeof (double));/* Result of -U [gmtdefs.unix_time_pos] */
	GMT_ps.encoding_name = gmtdefs.encoding.name;	/* Font encoding used */
}

/* Here is the new -B parser with all its sub-functions */

GMT_LONG GMT_strip_colonitem (const char *in, const char *pattern, char *item, char *out) {
	/* Removes the searched-for item from in, returns it in item, with the rest in out.
	 * pattern is usually ":." for title, ":," for unit, and ":" for label.
	 * ASSUMPTION: Only pass ":" after first removing titles and units
	 */

	char *s;
	GMT_LONG error = FALSE;

	if ((s = strstr (in, pattern))) {		/* OK, found what we are looking for */
		GMT_LONG i, j, k;
		k = (GMT_LONG)(s - in);			/* Start index of item */
		strncpy (out, in, (size_t)k);			/* Copy everything up to the pattern */
		i = k + strlen (pattern);		/* Now go to beginning of item */
		j = 0;
		while (in[i] && in[i] != ':') item[j++] = in[i++];	/* Copy the item... */
		item[j] = '\0';				/* ...and terminate the string */
		if (in[i] != ':') error = TRUE;		/* Error: Missing terminating colon */
		i++;					/* Skip the ending colon */
		while (in[i]) out[k++] = in[i++];	/* Copy rest to out... */
		out[k] = '\0';				/* .. and terminate */
	}
	else {	/* No item to update */
		strcpy (out, in);
	}

	if (error) {	/* Problems with decoding */
		fprintf (stderr, "%s: ERROR: Missing terminating colon in -B string %s\n", GMT_program, in);
		GMT_exit (EXIT_FAILURE);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":.")) {	/* Problems with decoding title */
		fprintf (stderr, "%s: ERROR: More than one title in  -B string %s\n", GMT_program, in);
		GMT_exit (EXIT_FAILURE);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":,")) {	/* Problems with decoding unit */
		fprintf (stderr, "%s: ERROR: More than one unit string in  -B component %s\n", GMT_program, in);
		GMT_exit (EXIT_FAILURE);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":=")) {	/* Problems with decoding prefix */
		fprintf (stderr, "%s: ERROR: More than one prefix string in  -B component %s\n", GMT_program, in);
		GMT_exit (EXIT_FAILURE);
	}
	if (strstr (out, pattern)) {	/* Problems with decoding label */
		fprintf (stderr, "%s: ERROR: More than one label string in  -B component %s\n", GMT_program, in);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

void GMT_strip_wesnz (const char *in, GMT_LONG t_side[], GMT_LONG *draw_box, char *out) {
	/* Removes the WESNZwesnz+ flags and sets the side/drawbox parameters
	 * and return the resulting stripped string
	 */

	GMT_LONG set_sides = FALSE, mute = FALSE;
	GMT_LONG i, k, set, side[5] = {0, 0, 0, 0, 0};

	for (i = k = 0; in[i]; i++) {
		if (in[i] == ':') mute = !mute;	/* Toggle so that mute is TRUE when we are within a :<stuff>: string */
		if (mute) {	/* Do not look for WEST inside a label */
			out[k++] = in[i];
			continue;
		}
		set = 0;
		switch (in[i]) {
			case 'W':	/* Draw AND Annotate */
				set++;
			case 'w':	/* Just Draw */
				side[3] = ++set;
				set_sides = TRUE;
				break;
			case 'E':	/* Draw AND Annotate */
				set++;
			case 'e':	/* Just Draw */
				if (i > 0 && (in[i-1] == '.' || isdigit ((int)in[i-1])) && (in[i+1] && (isdigit ((int)in[i+1]) || in[i+1] == '-' || in[i+1] == '+')))	/* Exponential notation */
					out[k++] = in[i];
				else {
					side[1] = ++set;
					set_sides = TRUE;
				}
				break;
			case 'S':	/* Draw AND Annotate */
				set++;
			case 's':	/* Just Draw */
				side[0] = ++set;
				set_sides = TRUE;
				break;
			case 'N':	/* Draw AND Annotate */
				set++;
			case 'n':	/* Just Draw */
				side[2] = ++set;
				set_sides = TRUE;
				break;
			case 'Z':	/* Draw AND Annotate */
				set++;
			case 'z':	/* Just Draw */
				side[4] = ++set;
				set_sides = TRUE;
				if (in[i+1] == '+') *draw_box = TRUE, i++;
				break;
			default:	/* Anything else is copy */
				out[k++] = in[i];
				break;
		}
	}
	out[k] = '\0';	/* Terminate string */

	if (set_sides) for (i = 0; i < 5; i++) t_side[i] = side[i];	/* Only changes these if WESN was provided */
}

GMT_LONG GMT_split_info (const char *in, char *info[]) {
	/* Take the -B string (minus the leading -B) and chop into 3 strings for x, y, and z */

	GMT_LONG mute = FALSE;

	GMT_LONG i, n_slash, s_pos[2];

	for (i = n_slash = 0; in[i] && n_slash < 3; i++) {
		if (in[i] == ':') mute = !mute;
		if (in[i] == '/' && !mute) {	/* Axis-separating slash, not a slash in a label */
			s_pos[n_slash++] = i;
		}
	}

	if (n_slash == 3) {
		fprintf (stderr, "%s: Error splitting -B string %s\n", GMT_program, in);
		GMT_exit (EXIT_FAILURE);
	}

	if (n_slash == 2) {	/* Got x/y/z */
		i = strlen (in);
		strncpy (info[0], in, (size_t)s_pos[0]);				info[0][s_pos[0]] = '\0';
		strncpy (info[1], &in[s_pos[0]+1], (size_t)(s_pos[1] - s_pos[0] - 1));	info[1][s_pos[1] - s_pos[0] - 1] = '\0';
		strncpy (info[2], &in[s_pos[1]+1], (size_t)(i - s_pos[1] - 1));		info[2][i - s_pos[1] - 1] = '\0';
	}
	else if (n_slash == 1) {	/* Got x/y */
		i = strlen (in);
		strncpy (info[0], in, (size_t)s_pos[0]);				info[0][s_pos[0]] = '\0';
		strncpy (info[1], &in[s_pos[0]+1], (size_t)(i - s_pos[0] - 1));		info[1][i - s_pos[0] - 1] = '\0';
		info[2][0] = '\0';			/* Zero out the z info */
	}
	else {	/* Got x with implicit copy to y */
		strcpy (info[0], in);
		strcpy (info[1], in);
		info[2][0] = '\0';			/* Zero out the z info */
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_decode_tinfo (char *in, struct GMT_PLOT_AXIS *A) {
	/* Decode the annot/tick segments of the clean -B string pieces */

	char *t, *s, flag, orig_flag = 0, unit;
	GMT_LONG error = 0;
	GMT_LONG time_interval_unit;
	double val, phase = 0.0;

	if (!in) return (GMT_NOERROR);	/* NULL pointer passed */

	t = in;
	while (t[0] && !error) {	/* As long as there are more segments to decode and no trouble so far */
		if (isdigit ((int)t[0]) || t[0] == '-' || t[0] == '+' || t[0] == '.')	/* No segment type given, set to * which means a + f */
			flag = '*';
		else {
			flag = t[0];	/* Set flag */
			if (!strchr ("afg", flag)) {	/* Illegal flag given */
				error = 1;
				continue;
			}
			t++;		/* Skip to next */
			if (!t[0]) {
				error = 2;
				continue;
			}
		}

		/* Here, t must point to a valid number.  If t[0] is not [+,-,.] followed by a digit we have an error */

		if (!(isdigit ((int)t[0]) || ((t[0] == '-' || t[0] == '+' || t[0] == '.') && strlen(t) > 1))) {
			error = 2;
			continue;
		}
		/* Decode interval, get pointer to next segment */
		if ((val = strtod (t, &s)) < 0.0 && project_info.xyz_projection[A->id] != GMT_LOG10) {	/* Interval must be >= 0 */
			error = 3;
			continue;
		}
		if (s[0] && (s[0] == '-' || s[0] == '+')) {	/* Phase shift information given */
			t = s;
			phase = strtod (t, &s);
		}
		if (s[0] && strchr ("YyOoUuKkJjDdHhMmCcrRlp", s[0])) {	/* Appended one of the allowed units, or l or p for log10/pow */
			unit = s[0];
			s++;
		}
		else if (A->type == GMT_TIME)				/* Default time system unit implied */
			unit = gmtdefs.time_system.unit;
		else
			unit = 0;	/* Not specified */

		/* else s is either 0 or points to the next segment */

		switch (unit) {	/* Determine if we have intervals or moments */
			case 'Y':
			case 'y':
			case 'O':
			case 'o':
			case 'K':
			case 'k':
			case 'J':
			case 'j':
			case 'D':
			case 'd':
			case 'R':
			case 'r':
			case 'U':
			case 'u':
				if (A->type == GMT_TIME && flag == 'a') flag = 'i';
				time_interval_unit = TRUE;
				break;
			default:
				time_interval_unit = FALSE;
				break;
		}
		orig_flag = flag;
		if (GMT_primary) {	/* Since this is primary axes items */
			if (flag == '*' && time_interval_unit) flag = '+';
		}
		else {			/* Since this is secondary axes items */
			if (flag == '*')
				flag = (time_interval_unit) ? '-' : '^';
			else
				flag = (char) toupper ((int)flag);
		}
		if (!error) GMT_set_titem (A, val, phase, flag, unit);				/* Store the findings for this segment */
		t = s;									/* Make t point to start of next segment, if any */
	}

	if (error) {
		switch (error) {
			case 1:
				fprintf (stderr, "%s: ERROR: Unrecognized axis item or unit %c in -B string component %s\n", GMT_program, orig_flag, in);
				break;
			case 2:
				fprintf (stderr, "%s: ERROR: Interval missing from -B string component %s\n", GMT_program, in);
				break;
			case 3:
				fprintf (stderr, "%s: ERROR: Negative interval in -B string component %s\n", GMT_program, in);
				break;
			default:
				break;
		}
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_set_titem (struct GMT_PLOT_AXIS *A, double val, double phase, char flag, char unit) {
	/* Load the values into the appropriate GMT_PLOT_AXIS_ITEM structure */

	GMT_LONG i, n = 1;
	struct GMT_PLOT_AXIS_ITEM *I[2];
	char item_flag[8] = {'a', 'A', 'i', 'I', 'f', 'F', 'g', 'G'}, *format;

	if (A->type == GMT_TIME) {	/* Strict check on time intervals */
		if (GMT_verify_time_step (irint (val), unit)) GMT_exit (EXIT_FAILURE);
		if ((fmod (val, 1.0) > GMT_CONV_LIMIT)) {
			fprintf (stderr, "%s: ERROR: Time step interval (%g) must be an integer\n", GMT_program, val);
			GMT_exit (EXIT_FAILURE);
		}
	}

	switch (flag) {
		case 'a':	/* Upper tick annotation */
			I[0] = &A->item[0];
			break;
		case 'A':	/* Lower tick annotation */
			I[0] = &A->item[1];
			break;
		case 'i':	/* Upper interval annotation */
			I[0] = &A->item[2];
			break;
		case 'I':	/* Lower interval annotation */
			I[0] = &A->item[3];
			break;
		case 'f':	/* Upper Frame tick interval */
			I[0] = &A->item[4];
			break;
		case 'F':	/* Lower Frame tick interval */
			I[0] = &A->item[5];
			break;
		case 'g':	/* Upper Gridline interval */
			I[0] = &A->item[6];
			break;
		case 'G':	/* Lower gridline interval */
			I[0] = &A->item[7];
			break;
		case '*':	/* Both a and f */
			I[0] = &A->item[0];
			I[1] = &A->item[4];
			n = 2;
			break;
		case '+':	/* Both i and f */
			I[0] = &A->item[2];
			I[1] = &A->item[4];
			n = 2;
			break;
		case '^':	/* Both A and F */
			I[0] = &A->item[1];
			I[1] = &A->item[5];
			n = 2;
			break;
		case '-':	/* Both I and F */
			I[0] = &A->item[3];
			I[1] = &A->item[5];
			n = 2;
			break;
		default:	/* Bad flag should never get here */
			fprintf (stderr, "%s: Bad flag passed to GMT_set_titem\n", GMT_program);
			GMT_exit (EXIT_FAILURE);
			break;
	}

	switch (unit) {
		case 'l':	/* Log10 annotation flag */
			A->type = GMT_LOG10;
			unit = 0;
			break;
		case 'p':	/* pow annotation flag */
			A->type = GMT_POW;
			unit = 0;
			break;
		default:
			break;
	}

	if (phase != 0.0) A->phase = phase;	/* phase must apply to entire axis */
	for (i = 0; i < n; i++) {
		if (I[i]->active == 1) {
			fprintf (stderr, "%s: Warning: Axis sub-item %c set more than once (typo?)\n", GMT_program, item_flag[i]);
		}
		I[i]->interval = val;
		I[i]->unit = unit;
		I[i]->type = (flag == 'I' || flag == 'i') ? 'I' : 'A';
		I[i]->flavor = 0;
		I[i]->active = n;
		I[i]->upper_case = FALSE;
		format = (GMT_primary) ? gmtdefs.time_format[0] : gmtdefs.time_format[1];
		switch (format[0]) {	/* This parameter controls which version of month/day textstrings we use for plotting */
			case 'F':	/* Full name, upper case */
				I[i]->upper_case = TRUE;
			case 'f':	/* Full name, lower case */
				I[i]->flavor = 0;
				break;
			case 'A':	/* Abbreviated name, upper case */
				I[i]->upper_case = TRUE;
			case 'a':	/* Abbreviated name, lower case */
				I[i]->flavor = 1;
				break;
			case 'C':	/* 1-char name, upper case */
				I[i]->upper_case = TRUE;
			case 'c':	/* 1-char name, lower case */
				I[i]->flavor = 2;
				break;
			default:
				break;
		}
	}
	return (GMT_NOERROR);
}

void gmt_handle_atcolon (char *txt, GMT_LONG old)
{	/* Way = 0: Replaces @:<size>: and @:: with @^<size>^ and @^^ to avoid trouble in -B:label: parsing;
	 * Way = 1: Restores it the way it was. */
	GMT_LONG pos, new;
	char *item[2] = {"@:", "@^"}, mark[2] = {':', '^'}, *s = NULL;
	
	if (!txt || !txt[0]) return;	/* Nothing to do */
	new = 1 - old;	/* The opposite of old */
	while ((s = strstr (txt, item[old]))) {	/* As long as we keep finding these */
		pos = ((GMT_LONG)s - (GMT_LONG)txt) + 1;	/* Skip past the @ character */
		if (txt[pos+1] == mark[old]) {			/* Either :: or ^^ */
			txt[pos] = txt[pos+1] = mark[new];	/* Replace @:: with @^^ or vice versa */
		}
		else {	/* Found @:<size>: or @^<size>^ */
			txt[pos] = mark[new];
			while (txt[pos] && txt[pos] != mark[old]) pos++;
			if (txt[pos] == mark[old]) txt[pos] = mark[new];
		}
	}
}

GMT_LONG GMT_parse_B_option (char *in) {
	/* GMT_parse_B_option scans an argument string and extract parameters that
	 * set the interval for tickmarks and annotations on the boundary.
	 * The string must be continuous, i.e. no whitespace must be present
	 * The string may have 1, 2,  or 3 parts, separated by a slash '/'. All
	 * info after the first slash are assigned to the y-axis.  Info after
	 * the second slash are assigned to the z-axis.  If there is no
	 * slash, x-values are copied to y-values.
	 * A substring looks like [t][value][m|c]. The [t] and [m|c] are optional
	 * ([ and ] are NOT part of the string and are just used to clarify)
	 * [t] can be any of [a](annotation int), [f](frame int), or [g](gridline int).
	 * Default is a AND f. The [m], if present indicates value is in minutes.
	 * The [c], if present indicates value is in seConds (s already used for south...).
	 * Text between : and : are labels for the respective axes. If the first
	 * character of the text is a period, then the rest of the text is used
	 * as the plot title.  If it is a comma, then the rest is used as annotation unit.
	 * For GMT_LINEAR axes: If the first characters in args are one or more of w,e,s,n
	 * only those axes will be drawn. Upper case letters means the chosen axes
	 * also will be annotated. Default is all 4 axes drawn/annotated.
	 * For logscale plots:  l will cause log10(x) to be plotted
	 *			p will cause 10 ^ log10(x) to be plotted
	 *	annot/tick/grid interval can here be either:
	 *		1.0	-> Only powers of 10 are annotated
	 *		2.0	-> powers of 10 times (1, 2, 5) are annotated
	 *		3.0	-> powers of 10 times (1,2,3,..9) are annotated
	 *
	 * Up to two -B options may be given on the command line:
	 *	-B[p] the primary specifications
	 *	-Bs   the secondary specifications
	 *
	 *	-Bs must be in addition to -B[p].
	 */

	char out1[BUFSIZ], out2[BUFSIZ], out3[BUFSIZ], *info[3];
	char one[BUFSIZ], two[BUFSIZ], three[BUFSIZ];
	struct GMT_PLOT_AXIS *A;
	GMT_LONG i, j, k;

	if (in[0] == 's') {
		GMT_primary = FALSE;
		k = 1;
	}
	else if (in[0] == 'p') {
		GMT_primary = TRUE;
		k = 1;
	}
	else {
		GMT_primary = TRUE;
		k = 0;
	}
	/* frame_info.side[] may be set already when parsing .gmtdefaults4 flags */

	info[0] = one;	info[1] = two;	info[2] = three;
	if (!frame_info.plot) {	/* First time we initialize stuff */
		for (i = 0; i < 3; i++) {
			memset ((void *)&frame_info.axis[i], 0, sizeof (struct GMT_PLOT_AXIS));
			frame_info.axis[i].id = (int)i;
			for (j = 0; j < 8; j++) {
				frame_info.axis[i].item[j].parent = i;
				frame_info.axis[i].item[j].id = j;
			}
			if (project_info.xyz_projection[i] == GMT_TIME) frame_info.axis[i].type = GMT_TIME;
		}
		frame_info.header[0] = '\0';
		frame_info.plot = TRUE;
		frame_info.draw_box = FALSE;
	}

	GMT_strip_colonitem (&in[k], ":.", frame_info.header, out1);			/* Extract header string, if any */
	GMT_enforce_rgb_triplets (frame_info.header, GMT_LONG_TEXT);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */

	GMT_strip_wesnz (out1, frame_info.side, &frame_info.draw_box, out2);		/* Decode WESNZwesnz+ flags, if any */

	GMT_split_info (out2, info);					/* Chop/copy the three axis strings */

	for (i = 0; i < 3; i++) {					/* Process each axis separately */

		if (!info[i][0]) continue;

		gmt_handle_atcolon (info[i], 0);	/* Temporarily modify text escape @: to @^ to avoid : parsing trouble */
		GMT_enforce_rgb_triplets (info[i], BUFSIZ);				/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
		GMT_strip_colonitem (info[i], ":,", frame_info.axis[i].unit, out1);	/* Pull out annotation unit, if any */
		GMT_strip_colonitem (out1, ":=", frame_info.axis[i].prefix, out2);	/* Pull out annotation prefix, if any */
		GMT_strip_colonitem (out2, ":", frame_info.axis[i].label, out3);	/* Pull out axis label, if any */
		gmt_handle_atcolon (frame_info.axis[i].label, 1);			/* Restore any @^ to @: */

		GMT_decode_tinfo (out3, &frame_info.axis[i]);				/* Decode the annotation intervals */

		/* Make sure we have ticks to match annotation stride */
		A = &frame_info.axis[i];
		if (A->item[GMT_ANNOT_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			memcpy ((void *)&A->item[GMT_TICK_UPPER], (void *)&A->item[GMT_ANNOT_UPPER], sizeof (struct GMT_PLOT_AXIS_ITEM));
		else if (A->item[GMT_INTV_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			memcpy ((void *)&A->item[GMT_TICK_UPPER], (void *)&A->item[GMT_INTV_UPPER], sizeof (struct GMT_PLOT_AXIS_ITEM));
		if (A->item[GMT_ANNOT_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			memcpy ((void *)&A->item[GMT_TICK_LOWER], (void *)&A->item[GMT_ANNOT_LOWER], sizeof (struct GMT_PLOT_AXIS_ITEM));
		else if (A->item[GMT_INTV_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			memcpy ((void *)&A->item[GMT_TICK_LOWER], (void *)&A->item[GMT_INTV_LOWER], sizeof (struct GMT_PLOT_AXIS_ITEM));
	}

	/* Check if we asked for linear projections of geographic coordinates and did not specify a unit - if so set degree symbol as unit */
	if (project_info.projection == GMT_LINEAR && gmtdefs.degree_symbol != gmt_none) {
		for (i = 0; i < 2; i++) {
			if (GMT_io.in_col_type[i] & GMT_IS_GEO && frame_info.axis[i].unit[0] == 0) {
				frame_info.axis[i].unit[0] = '-';
				frame_info.axis[i].unit[1] = (char)gmtdefs.encoding.code[gmtdefs.degree_symbol];
				frame_info.axis[i].unit[2] = '\0';
			}
		}
	}

	return (0);
}

GMT_LONG GMT_project_type (char *args, GMT_LONG *pos, GMT_LONG *width_given)
{
	/* Parse the start of the -J option to determine the projection type.
	 * If the first character of args is uppercase, width_given is set to 1.
	 * Pos returns the position of the first character of the parameters
	 * following the projections type.
	 * The return value is the projection type ID (see gmt_project.h), or
	 * GMT_NO_PROJ when unsuccessful.
	 */

	char c;

	/* Check for upper case */

	*width_given = (args[0] >= 'A' && args[0] <= 'Z');

	/* Compared the first part of the -J arguments against a number of Proj4
	   projection names (followed by a slash) or the 1- or 2-letter abbreviation
	   used prior to GMT 4.2.2. Case is ignored */

	if ((*pos = GMT_strlcmp("aea/"      , args))) return (GMT_ALBERS);
	if ((*pos = GMT_strlcmp("aeqd/"     , args))) return (GMT_AZ_EQDIST);
	if ((*pos = GMT_strlcmp("cyl_stere/", args))) return (GMT_CYL_STEREO);
	if ((*pos = GMT_strlcmp("cass/"     , args))) return (GMT_CASSINI);
	if ((*pos = GMT_strlcmp("cea/"      , args))) return (GMT_CYL_EQ);
	if ((*pos = GMT_strlcmp("eck4/"     , args))) return (GMT_ECKERT4);
	if ((*pos = GMT_strlcmp("eck6/"     , args))) return (GMT_ECKERT6);
	if ((*pos = GMT_strlcmp("eqc/"      , args))) return (GMT_CYL_EQDIST);
	if ((*pos = GMT_strlcmp("eqdc/"     , args))) return (GMT_ECONIC);
	if ((*pos = GMT_strlcmp("gnom/"     , args))) return (GMT_GNOMONIC);
	if ((*pos = GMT_strlcmp("hammer/"   , args))) return (GMT_HAMMER);
	if ((*pos = GMT_strlcmp("laea/"     , args))) return (GMT_LAMB_AZ_EQ);
	if ((*pos = GMT_strlcmp("lcc/"      , args))) return (GMT_LAMBERT);
	if ((*pos = GMT_strlcmp("merc/"     , args))) return (GMT_MERCATOR);
	if ((*pos = GMT_strlcmp("mill/"     , args))) return (GMT_MILLER);
	if ((*pos = GMT_strlcmp("moll/"     , args))) return (GMT_MOLLWEIDE);
	if ((*pos = GMT_strlcmp("nsper/"    , args))) return (GMT_GENPER);
	if ((*pos = GMT_strlcmp("omerc/"    , args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = GMT_strlcmp("omercp/"   , args))) return (GMT_OBLIQUE_MERC_POLE);
	if ((*pos = GMT_strlcmp("ortho/"    , args))) return (GMT_ORTHO);
	if ((*pos = GMT_strlcmp("polar/"    , args))) return (GMT_POLAR);
	if ((*pos = GMT_strlcmp("poly/"     , args))) return (GMT_POLYCONIC);
	if ((*pos = GMT_strlcmp("robin/"    , args))) return (GMT_ROBINSON);
	if ((*pos = GMT_strlcmp("sinu/"     , args))) return (GMT_SINUSOIDAL);
	if ((*pos = GMT_strlcmp("stere/"    , args))) return (GMT_STEREO);
	if ((*pos = GMT_strlcmp("tmerc/"    , args))) return (GMT_TM);
	if ((*pos = GMT_strlcmp("utm/"      , args))) return (GMT_UTM);
	if ((*pos = GMT_strlcmp("vandg/"    , args))) return (GMT_VANGRINTEN);
	if ((*pos = GMT_strlcmp("wintri/"   , args))) return (GMT_WINKEL);
	if ((*pos = GMT_strlcmp("xy/"       , args))) return (GMT_LINEAR);
	if ((*pos = GMT_strlcmp("z/"        , args))) return (GMT_ZAXIS);

	/* These older codes (up to GMT 4.2.1) took 2 characters */

	if ((*pos = GMT_strlcmp("kf", args))) return (GMT_ECKERT4);
	if ((*pos = GMT_strlcmp("ks", args))) return (GMT_ECKERT6);
	if ((*pos = GMT_strlcmp("oa", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = GMT_strlcmp("ob", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = GMT_strlcmp("oc", args))) return (GMT_OBLIQUE_MERC_POLE);

	/* Finally, check only the first letter (used until GMT 4.2.1) */

	*pos = 1;
	c = tolower(args[0]);
	if (c == 'a') return (GMT_LAMB_AZ_EQ);
	if (c == 'b') return (GMT_ALBERS);
	if (c == 'c') return (GMT_CASSINI);
	if (c == 'd') return (GMT_ECONIC);
	if (c == 'e') return (GMT_AZ_EQDIST);
	if (c == 'f') return (GMT_GNOMONIC);
	if (c == 'g') return (GMT_GENPER);
	if (c == 'h') return (GMT_HAMMER);
	if (c == 'i') return (GMT_SINUSOIDAL);
	if (c == 'j') return (GMT_MILLER);
	if (c == 'k') return (GMT_ECKERT6);
	if (c == 'l') return (GMT_LAMBERT);
	if (c == 'm') return (GMT_MERCATOR);
	if (c == 'n') return (GMT_ROBINSON);
	if (c == 'o') return (GMT_OBLIQUE_MERC);
	if (c == 'p') return (GMT_POLAR);
	if (c == 'q') return (GMT_CYL_EQDIST);
	if (c == 'r') return (GMT_WINKEL);
	if (c == 's') return (GMT_STEREO);
	if (c == 't') return (GMT_TM);
	if (c == 'u') return (GMT_UTM);
	if (c == 'v') return (GMT_VANGRINTEN);
	if (c == 'w') return (GMT_MOLLWEIDE);
	if (c == 'x') return (GMT_LINEAR);
	if (c == 'y') return (GMT_CYL_EQ);
	if (c == 'z') return (GMT_ZAXIS);

	/* Did not find any match. Report error */

	*pos = 0;
	return (GMT_NO_PROJ);
}

GMT_LONG GMT_parse_J_option (char *args)
{
	/* GMT_parse_J_option scans the arguments given and extracts the parameters needed
	 * for the specified map projection. These parameters are passed through the
	 * project_info structure.  The function returns TRUE if an error is encountered.
	 */

	GMT_LONG i, j, k = 9, m, n, nlen, slash, l_pos[2], p_pos[2], t_pos[2], d_pos[2], id, project;
	GMT_LONG n_slashes = 0, width_given, last_pos, error = FALSE, skip = FALSE;
	double c, az;
	double GMT_units[3] = {0.01, 0.0254, 1.0};      /* No of meters in a cm, inch, m */
	char mod, args_cp[BUFSIZ], txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT], txt_c[GMT_LONG_TEXT];
	char txt_d[GMT_LONG_TEXT], txt_e[GMT_LONG_TEXT], last_char;
	char txt_arr[11][GMT_LONG_TEXT];

	l_pos[0] = l_pos[1] = p_pos[0] = p_pos[1] = t_pos[0] = t_pos[1] = d_pos[0] = d_pos[1] = 0;

	project = GMT_project_type (args, &i, &width_given);
	if (project == GMT_NO_PROJ) return (TRUE);	/* No valid projection specified */
	args += i;

	last_pos = strlen (args) - 1;	/* Position of last character in this string */
	last_char = args[last_pos];
	if (last_pos > 0) {	/* Avoid having -JXh|v be misinterpreted */
		switch (last_char) {	/* Check for what kind of width is given (only used if upper case is given below */
			case 'h':	/* Want map HEIGHT instead */
				width_given = 2;
				break;
			case '+':	/* Want this to be the MAX dimension of map */
				width_given = 3;
				break;
			case '-':	/* Want this to be the MIN dimension of map */
				width_given = 4;
				break;
		}
	}
	if (width_given > 1) args[last_pos] = 0;	/* Temporarily chop off modifier */

	for (j = 0; args[j]; j++) if (args[j] == '/') n_slashes++;

	/* Differentiate between general perspective and orthographic projection based on number of slashes */
	if (project == GMT_GENPER || project == GMT_ORTHO) {
		if (n_slashes >= 5)
			project = GMT_GENPER;
		else
			project = GMT_ORTHO;
	}

	if (project != GMT_ZAXIS) {
		/* Check to see if scale is specified in 1:xxxx */
		for (j = strlen (args), k = -1; j > 0 && k < 0 && args[j] != '/'; j--) if (args[j] == ':') k = j + 1;
		project_info.units_pr_degree = (k == -1) ? TRUE : FALSE;
		GMT_io.out_col_type[0] = GMT_io.out_col_type[1] = GMT_IS_FLOAT;		/* This may be overridden by mapproject -I */
		if (project != GMT_LINEAR) {
			project_info.gave_map_width = width_given;
			GMT_io.in_col_type[0] = GMT_IS_LON, GMT_io.in_col_type[1] = GMT_IS_LAT;
			project_info.degree[0] = project_info.degree[1] = TRUE;
		}
	}

	project_info.unit = GMT_units[GMT_INCH];	/* No of meters in an inch */
	n = 0;	/* Initialize with no fields found */

	switch (project) {
		case GMT_LINEAR:	/* Linear x/y scaling */
			if (width_given) {
				project_info.compute_scale[0] = project_info.compute_scale[1] = TRUE;
				if (args[0] == 'v') {
					project_info.pars[0] = gmtdefs.y_axis_length;
					project_info.pars[1] = gmtdefs.x_axis_length;
					skip = TRUE;
				}
				else if (args[0] == 'h') {
					project_info.pars[0] = gmtdefs.x_axis_length;
					project_info.pars[1] = gmtdefs.y_axis_length;
					skip = TRUE;
				}
			}

			/* Default is not involving geographical coordinates */
			GMT_io.in_col_type[0] = GMT_io.in_col_type[1] = GMT_IS_UNKNOWN;
			project_info.degree[0] = project_info.degree[1] = FALSE;

			error = (n_slashes > 1);
			if (!strncmp (args, "1:", (size_t)2)) k = 1;	/* Special check for linear proj with 1:xxx scale */

			/* Find occurrences of /, l, p, t, or d */
			for (j = 0, slash = 0; args[j] && slash == 0; j++) if (args[j] == '/') slash = j;
			for (j = id = 0; args[j]; j++) {
				if (args[j] == '/') id = 1;
				if (strchr ("Ll"  , (int)args[j])) l_pos[id] = j;
				if (strchr ("Pp"  , (int)args[j])) p_pos[id] = j;
				if (strchr ("Tt"  , (int)args[j])) t_pos[id] = j;
				if (strchr ("DdGg", (int)args[j])) d_pos[id] = j;
			}

			if (k > 0) {	/* For 1:xxxxx  we cannot have /LlTtDdGg modifiers */
				if (n_slashes) error = TRUE;	/* Cannot have 1:xxx separately for x/y */
				/* if (l_pos[0] || l_pos[1] || p_pos[0] || p_pos[1] || t_pos[0] || t_pos[1] || d_pos[0] || d_pos[1]) error = TRUE; */
				if (l_pos[0] || l_pos[1] || p_pos[0] || p_pos[1]) error = TRUE;
			}

			/* Distinguish between p for points and p<power> for scaling */

			n = strlen (args);
			for (j = 0; j < 2; j++) {
				if (!p_pos[j]) continue;
				i = p_pos[j] + 1;
				if (i == n || strchr ("/LlTtDdGg", (int)args[i]))	/* This p is for points since no power is following */
					p_pos[j] = 0;
				else if (strchr("Pp", (int)args[i]))	/* The 2nd p is the p for power */
					p_pos[j]++;
			}

			/* Get x-arguments */

			strcpy (args_cp, args);	/* Since GMT_convert_units modifies the string */
			if (slash) args_cp[slash] = 0;	/* Chop off y part */
			if ((i = MAX (l_pos[0], p_pos[0])) > 0)
				args_cp[i] = 0;	/* Chop off log or power part */
			else if (t_pos[0] > 0)
				args_cp[t_pos[0]] = 0;	/* Chop off time part */
			else if (d_pos[0] > 0)	/* Chop of trailing 'd' */
				args_cp[d_pos[0]] = 0;
			if (!skip) {
				if (k >= 0)	/* Scale entered as 1:mmmmm - this implies -R is in meters */
					project_info.pars[0] = GMT_u2u[GMT_M][GMT_INCH] / atof (&args_cp[2]);
				else
					project_info.pars[0] = GMT_convert_units (args_cp, GMT_INCH);	/* x-scale */
			}
			if (l_pos[0] > 0)
				project_info.xyz_projection[0] = GMT_LOG10;
			else if (p_pos[0] > 0) {
				project_info.xyz_projection[0] = GMT_POW;
				project_info.pars[2] = atof (&args[p_pos[0]+1]);	/* pow to raise x */
			}
			else if (t_pos[0] > 0) {	/* Add option to append time_systems or epoch/unit later */
				project_info.xyz_projection[0] = GMT_TIME;
				GMT_io.in_col_type[0] = (args[t_pos[0]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME;
			}

			if (d_pos[0] > 0) GMT_io.in_col_type[0] = GMT_IS_LON, project_info.degree[0] = TRUE;

			if (slash) {	/* Separate y-scaling desired */
				strcpy (args_cp, &args[slash+1]);	/* Since GMT_convert_units modifies the string */
				if ((i = MAX (l_pos[1], p_pos[1])) > 0)
					args_cp[i-slash-1] = 0;	/* Chop off log or power part */
				else if (t_pos[1] > 0)
					args_cp[t_pos[1]-slash-1] = 0;	/* Chop off log or power part */
				else if (d_pos[1] > 0)
					args_cp[d_pos[1]-slash-1] = 0;	/* Chop of trailing 'd' part */
				if (!skip) project_info.pars[1] = GMT_convert_units (args_cp, GMT_INCH);	/* y-scale */

				if (l_pos[1] > 0)
					project_info.xyz_projection[1] = GMT_LOG10;
				else if (p_pos[1] > 0) {
					project_info.xyz_projection[1] = GMT_POW;
					project_info.pars[3] = atof (&args[p_pos[1]+1]);	/* pow to raise y */
				}
				else if (t_pos[1] > 0) {	/* Add option to append time_systems or epoch/unit later */
					project_info.xyz_projection[1] = GMT_TIME;
					GMT_io.in_col_type[1] = (args[t_pos[0]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME;
				}
				if (d_pos[1] > 0) GMT_io.in_col_type[1] = GMT_IS_LAT, project_info.degree[1] = TRUE;
			}
			else {	/* Just copy x parameters */
				project_info.xyz_projection[1] = project_info.xyz_projection[0];
				if (!skip) project_info.pars[1] = project_info.pars[0];
				project_info.pars[3] = project_info.pars[2];
				project_info.degree[1] = project_info.degree[0];
				if (GMT_io.in_col_type[0] & GMT_IS_GEO) GMT_io.in_col_type[1] = GMT_IS_LAT;
			}

			/* Not both sizes can be zero, but if one is, we will adjust to the scale of the other */
			if (project_info.pars[0] == 0.0 && project_info.pars[1] == 0.0) error = TRUE;
			break;

		case GMT_ZAXIS:	/* 3D plot */
			if (width_given) project_info.compute_scale[2] = TRUE;
			error = (n_slashes > 0);
			GMT_io.in_col_type[2] = GMT_IS_UNKNOWN;
			project_info.degree[2] = FALSE;

			/* Find occurrences of l, p, or t */
			for (j = 0; args[j]; j++) {
				if (strchr ("Ll", (int)args[j])) l_pos[0] = j;
				if (strchr ("Pp", (int)args[j])) p_pos[0] = j;
				if (strchr ("Tt", (int)args[j])) t_pos[0] = j;
			}

			/* Distinguish between p for points and p<power> for scaling */

			n = strlen (args);
			if (p_pos[0]) {
				i = p_pos[0] + 1;
				if (i == n || strchr ("LlTtDdGg", (int)args[i]))	/* This p is for points since no power is following */
					p_pos[0] = 0;
				else if (strchr ("Pp", (int)args[i]))	/* The 2nd p is the p for power */
					p_pos[0]++;
			}

			/* Get arguments */

			strcpy (args_cp, args);	/* Since GMT_convert_units modifies the string */
			if ((i = MAX (l_pos[0], p_pos[0])) > 0)
				args_cp[i] = 0;
			else if (t_pos[0] > 0)
				args_cp[t_pos[0]] = 0;
			project_info.z_pars[0] = GMT_convert_units (args_cp, GMT_INCH);	/* z-scale */

			if (l_pos[0] > 0)
				project_info.xyz_projection[2] = GMT_LOG10;
			else if (p_pos[0] > 0) {
				project_info.xyz_projection[2] = GMT_POW;
				project_info.z_pars[1] = atof (&args[p_pos[0]+1]);	/* pow to raise z */
			}
			else if (t_pos[0] > 0) {
				project_info.xyz_projection[2] = GMT_TIME;
				GMT_io.in_col_type[2] = (args[t_pos[0]] == 'T') ? GMT_IS_ABSTIME : GMT_IS_RELTIME;
			}
			if (project_info.z_pars[0] == 0.0) error = TRUE;
			project_info.JZ_set = TRUE;
			break;

		case GMT_POLAR:		/* Polar (theta,r) */
			GMT_io.in_col_type[0] = GMT_IS_LON, GMT_io.in_col_type[1] = GMT_IS_FLOAT;
			project_info.degree[0] = project_info.degree[1] = FALSE;
			if (args[0] == 'a' || args[0] == 'A') {
				project_info.got_azimuths = TRUE;	/* using azimuths instead of directions */
				i = 1;
			}
			else {
				project_info.got_azimuths = FALSE;
				i = 0;
			}
			j = strlen (args) - 1;
			if (args[j] == 'r') {	/* Gave optional r for reverse (elevations, presumably) */
				project_info.got_elevations = TRUE;
				args[j] = '\0';	/* Temporarily chop off the r */
			}
			else if (args[j] == 'z') {	/* Gave optional z for annotating depths rather than radius */
				project_info.z_down = TRUE;
				args[j] = '\0';	/* Temporarily chop off the z */
			}
			else
				project_info.got_elevations = project_info.z_down = FALSE;
			if (n_slashes == 1) {	/* Gave optional zero-base angle [0] */
				n = sscanf (args, "%[^/]/%lf", txt_a, &project_info.pars[1]);
				if (n == 2) project_info.pars[0] = GMT_convert_units (&txt_a[i], GMT_INCH);
				error = (project_info.pars[0] <= 0.0 || n != 2);
			}
			else if (n_slashes == 0) {
				project_info.pars[0] = GMT_convert_units (&args[i], GMT_INCH);
				n = (args) ? 1 : 0;
				error = (project_info.pars[0] <= 0.0 || n != 1);
			}
			else
				error = TRUE;
			if (project_info.got_elevations) args[j] = 'r';	/* Put the r back in the argument */
			if (project_info.z_down) args[j] = 'z';	/* Put the z back in the argument */
			if (project_info.got_azimuths) project_info.pars[1] = -project_info.pars[1];	/* Because azimuths go clockwise */
			break;

		/* Map projections */

		case GMT_ECKERT4:	/* Eckert IV */
		case GMT_ECKERT6:	/* Eckert VI */
		case GMT_HAMMER:	/* Hammer-Aitoff Equal-Area */
		case GMT_MILLER:	/* Miller cylindrical */
		case GMT_MOLLWEIDE:	/* Mollweide Equal-Area */
		case GMT_ROBINSON:	/* Robinson Projection */
		case GMT_SINUSOIDAL:	/* Sinusoidal Equal-Area */
		case GMT_VANGRINTEN:	/* Van der Grinten */
		case GMT_WINKEL:	/* Winkel Tripel Modified azimuthal */
			project_info.pars[0] = GMT_d_NaN;	/* Will be replaced by central meridian either below or in GMT_map_init_... */
			if (n_slashes == 0)
				n = sscanf (args, "%s", txt_b);
			else if (n_slashes == 1) {
				n = sscanf (args, "%[^/]/%s", txt_a, txt_b);
				error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
			}
			error += GMT_scale_or_width (txt_b, &project_info.pars[1]);
			error += !(n = n_slashes + 1);
			break;

		case GMT_CYL_EQ:	/* Cylindrical Equal Area */
		case GMT_CYL_EQDIST:	/* Equidistant Cylindrical */
		case GMT_CYL_STEREO:	/* Cylindrical Stereographic */
		case GMT_CASSINI:	/* Cassini */
		case GMT_MERCATOR:	/* Mercator */
		case GMT_TM:		/* Transverse Mercator */
		case GMT_POLYCONIC:	/* Polyconic */
			project_info.pars[0] = GMT_d_NaN;
			project_info.pars[1] = 0.0;
			txt_a[0] = txt_b[0] = 0;
			if (n_slashes == 0)
				n = sscanf (args, "%s", txt_c);
			else if (n_slashes == 1)
				n = sscanf (args, "%[^/]/%s", txt_a, txt_c);
			else if (n_slashes == 2)
				n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (txt_a[0]) error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
			if (txt_b[0]) error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
			error += GMT_scale_or_width (txt_c, &project_info.pars[2]);
			error += ((project == GMT_CYL_EQ || project == GMT_MERCATOR || project == GMT_TM || project == GMT_POLYCONIC)
				&& fabs(project_info.pars[1]) >= 90.0);
			error += !(n = n_slashes + 1);
			break;

		case GMT_ALBERS:	/* Albers Equal-area Conic */
		case GMT_ECONIC:	/* Equidistant Conic */
		case GMT_LAMBERT:	/* Lambert Conformal Conic */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_c, GMT_IS_LAT, &project_info.pars[2]), txt_c);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_d, GMT_IS_LAT, &project_info.pars[3]), txt_d);
			error += GMT_scale_or_width (txt_e, &project_info.pars[4]);
			error += (project_info.pars[2] == project_info.pars[3]);
			error += !(n_slashes == 4 && n == 5);
			break;

		case GMT_ORTHO:
			project_info.g_debug = 0;
			project_info.g_box = project_info.g_outside = project_info.g_longlat_set = project_info.g_radius = project_info.g_auto_twist = FALSE;
			project_info.g_sphere = TRUE; /* force spherical as default */
			project_info.pars[5] = project_info.pars[6] = project_info.pars[7] = 0.0;

		case GMT_AZ_EQDIST:	/* Azimuthal equal-distant */
		case GMT_LAMB_AZ_EQ:	/* Lambert Azimuthal Equal-Area */
		case GMT_GNOMONIC:	/* Gnomonic */
			if (project == GMT_AZ_EQDIST)	/* Initialize default horizons */
				strcpy (txt_c, "180");
			else if (project == GMT_GNOMONIC)
				strcpy (txt_c, "60");
			else
				strcpy (txt_c, "90");
			if (k >= 0) {	/* Scale entered as 1:mmmmm */
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/1:%lf", txt_a, txt_b, &project_info.pars[3]);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, &project_info.pars[3]);
				if (project_info.pars[3] != 0.0) project_info.pars[3] = 1.0 / (project_info.pars[3] * project_info.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				project_info.pars[3] = GMT_convert_units (txt_d, GMT_INCH);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					project_info.pars[3] = GMT_convert_units (txt_d, GMT_INCH);
					error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_e, GMT_IS_LAT, &project_info.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_c, GMT_IS_LAT, &project_info.pars[2]), txt_c);
			error += (project_info.pars[2] <= 0.0 || project_info.pars[2] > 180.0 || project_info.pars[3] <= 0.0 || (k >= 0 && width_given));
			error += (project == GMT_GNOMONIC && project_info.pars[2] >= 90.0);
			error += (project == GMT_ORTHO && project_info.pars[2] >= 180.0);
			break;

		case GMT_STEREO:	/* Stereographic */
			strcpy (txt_c, "90");	/* Initialize default horizon */
			if (k >= 0) {	/* Scale entered as 1:mmmmm */
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/1:%lf", txt_a, txt_b, &project_info.pars[3]);
				else if (n_slashes == 3) {	/* with true scale at specified latitude */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_e, &project_info.pars[3]);
					error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_e, GMT_IS_LAT, &project_info.pars[4]), txt_e);
					project_info.pars[5] = 1.0;	/* flag for true scale case */
				}
				else if (n_slashes == 4) {
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, txt_e, &project_info.pars[3]);
					error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_e, GMT_IS_LAT, &project_info.pars[4]), txt_e);
					project_info.pars[5] = 1.0;	/* flag for true scale case */
				}
				if (project_info.pars[3] != 0.0) project_info.pars[3] = 1.0 / (project_info.pars[3] * project_info.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				project_info.pars[3] = GMT_convert_units (txt_d, GMT_INCH);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					project_info.pars[3] = GMT_convert_units (txt_d, GMT_INCH);
					error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_e, GMT_IS_LAT, &project_info.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_c, GMT_IS_LON, &project_info.pars[2]), txt_c);
			error += (project_info.pars[2] <= 0.0 || project_info.pars[2] >= 180.0 || project_info.pars[3] <= 0.0 || (k >= 0 && width_given));
			break;

		case GMT_GENPER:	/* General perspective */

			project_info.g_debug = 0;
			project_info.g_box = project_info.g_outside = project_info.g_longlat_set = project_info.g_radius = project_info.g_auto_twist = FALSE;
			project_info.g_sphere = TRUE; /* force spherical as default */

			i = 0;
			for (j = 0 ; j < 2 ; j++) {
				if (args[j] == 'd') {         /* standard genper debugging */
					project_info.g_debug = 1;
					i++;
				} else if (args[j] == 'D') {  /* extensive genper debugging */
					project_info.g_debug = 2;
					i++;
				} else if (args[j] == 'X') {  /* extreme genper debugging */
					project_info.g_debug = 3;
					i++;
				} else if (args[j] == 's') {
					project_info.g_sphere = TRUE;
					i++;
				} else if (args[j] == 'e') {
					project_info.g_sphere = FALSE;
					i++;
				}
			}

			project_info.pars[4] = project_info.pars[5] = project_info.pars[6] = project_info.pars[7] = project_info.pars[8] = project_info.pars[9] = 0.0;

			if (project_info.g_debug > 1) {
				fprintf (stderr, "genper: arg '%s' n_slashes %ld k %ld\n", args, n_slashes, j);
				fprintf (stderr, "initial error %ld\n", error);
				fprintf (stderr, "j = %ld\n", j);
				fprintf (stderr, "width_given %ld\n", width_given);
			}

			n = sscanf(args+i, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s",
				&(txt_arr[0][0]), &(txt_arr[1][0]), &(txt_arr[2][0]), &(txt_arr[3][0]),
				&(txt_arr[4][0]), &(txt_arr[5][0]), &(txt_arr[6][0]), &(txt_arr[7][0]),
				&(txt_arr[8][0]), &(txt_arr[9][0]), &(txt_arr[10][0]));

			if (project_info.g_debug > 1) {
				for (i = 0 ; i < n ; i ++) {
					fprintf (stderr, "txt_arr[%ld] '%s'\n", i, &(txt_arr[i][0]));
				}
				fflush(NULL);
			}

			if (k >= 0) {
				/* Scale entered as 1:mmmmm */
				m = sscanf(&(txt_arr[n-1][0]),"1:%lf", &project_info.pars[2]);
				if (project_info.pars[2] != 0.0) {
					project_info.pars[2] = 1.0 / (project_info.pars[2] * project_info.unit);
				}
				error += (m == 0) ? 1 : 0;
				if (error) fprintf (stderr, "scale entered but couldn't read\n");
			} else  if (width_given) {
				project_info.pars[2] = GMT_convert_units(&(txt_arr[n-1][0]), GMT_INCH);
			} else {
				project_info.pars[2] = GMT_convert_units(&(txt_arr[n-2][0]), GMT_INCH);
				/*            project_info.pars[3] = GMT_ddmmss_to_degree(txt_i); */
				error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (&(txt_arr[n-1][0]), GMT_IS_LAT, &project_info.pars[3]), &(txt_arr[n-1][0]));
				if (error) fprintf (stderr, "error in reading last lat value\n");
			}
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf(&(txt_arr[0][0]), GMT_IS_LON, &project_info.pars[0]), &(txt_arr[0][0]));
			if (error) fprintf (stderr, "error is reading longitude '%s'\n", &(txt_arr[0][0]));
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (&(txt_arr[1][0]), GMT_IS_LAT, &project_info.pars[1]), &(txt_arr[1][0]));
			if (error) fprintf (stderr, "error reading latitude '%s'\n", &(txt_arr[1][0]));

			/* g_alt    project_info.pars[4] = atof(txt_c); */
			nlen = strlen(&(txt_arr[2][0]));
			if (txt_arr[2][nlen-1] == 'r') {
				project_info.g_radius = TRUE;
				txt_arr[2][nlen-1] = 0;
			}
			error += GMT_verify_expectations (GMT_IS_FLOAT, GMT_scanf (&(txt_arr[2][0]), GMT_IS_FLOAT, &project_info.pars[4]), &(txt_arr[2][0]));
			if (error) fprintf (stderr, "error reading altitude '%s'\n", &(txt_arr[2][0]));

			/* g_az    project_info.pars[5] = atof(txt_d); */
			nlen = strlen(&(txt_arr[3][0]));
			if (txt_arr[3][nlen-1] == 'l' || txt_arr[3][nlen-1] == 'L') {
				project_info.g_longlat_set = TRUE;
				txt_arr[3][nlen-1] = 0;
			}
			error += GMT_verify_expectations (GMT_IS_GEO, GMT_scanf (&(txt_arr[3][0]), GMT_IS_GEO, &project_info.pars[5]), &(txt_arr[3][0]));
			if (error) fprintf (stderr, "error reading azimuth '%s'\n", &(txt_arr[3][0]));

			/*g_tilt    project_info.pars[6] = atof(txt_e); */
			nlen = strlen(&(txt_arr[4][0]));
			if (txt_arr[4][nlen-1] == 'l' || txt_arr[4][nlen-1] == 'L') {
				project_info.g_longlat_set = TRUE;
				txt_arr[4][nlen-1] = 0;
			}
			error += GMT_verify_expectations (GMT_IS_GEO, GMT_scanf (&(txt_arr[4][0]), GMT_IS_GEO, &project_info.pars[6]), &(txt_arr[4][0]));
			if (error) fprintf (stderr, "error reading tilt '%s'\n", &(txt_arr[4][0]));

			if (n > 6) {
				/*g_twist   project_info.pars[7] = atof(txt_f); */
				nlen = strlen(&(txt_arr[5][0]));
				if (txt_arr[5][nlen-1] == 'n') {
					project_info.g_auto_twist = TRUE;
					txt_arr[5][nlen-1] = 0;
				}
				error += GMT_verify_expectations (GMT_IS_GEO, GMT_scanf (&(txt_arr[5][0]), GMT_IS_GEO, &project_info.pars[7]), &(txt_arr[5][0]));
				if (error) fprintf (stderr, "error reading twist '%s'\n", &(txt_arr[5][0]));

				/*g_width   project_info.pars[8] = atof(txt_f); */
				if (n > 7) {
					error += GMT_verify_expectations (GMT_IS_GEO, GMT_scanf (&(txt_arr[6][0]), GMT_IS_GEO, &project_info.pars[8]), &(txt_arr[6][0]));
					if (error) fprintf (stderr, "error reading width '%s'\n", &(txt_arr[6][0]));

					if (n > 8) {
						/*g_height  project_info.pars[9] = atof(txt_g); */
						error += GMT_verify_expectations (GMT_IS_GEO, GMT_scanf (&(txt_arr[7][0]), GMT_IS_GEO, &project_info.pars[9]), &(txt_arr[7][0]));
						if (error) fprintf (stderr, "error height '%s'\n", &(txt_arr[7][0]));
					}
				}
			}
			error += (project_info.pars[2] <= 0.0 || (k >= 0 && width_given));
			if (error) fprintf (stderr, "final error %ld\n", error);
			break;

		case GMT_OBLIQUE_MERC:		/* Oblique mercator, specifying origin and azimuth or second point */
			if (n_slashes == 3) {
				n = sscanf (args, "%[^/]/%[^/]/%lf/%s", txt_a, txt_b, &az, txt_e);
				error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
				error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
				c = 10.0;	/* compute point 10 degrees from origin along azimuth */
				project_info.pars[2] = project_info.pars[0] + atand (sind (c) * sind (az) / (cosd (project_info.pars[1]) * cosd (c) - sind (project_info.pars[1]) * sind (c) * cosd (az)));
				project_info.pars[3] = d_asind (sind (project_info.pars[1]) * cosd (c) + cosd (project_info.pars[1]) * sind (c) * cosd (az));
			}
			else if (n_slashes == 4) {
				n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
				error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
				error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_c, GMT_IS_LON, &project_info.pars[2]), txt_c);
				error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_d, GMT_IS_LAT, &project_info.pars[3]), txt_d);
			}
			error += GMT_scale_or_width (txt_e, &project_info.pars[4]);
			project_info.pars[6] = 0.0;
			error += !(n == n_slashes + 1);
			break;

		case GMT_OBLIQUE_MERC_POLE:	/* Oblique mercator, specifying orgin and pole */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_a, GMT_IS_LON, &project_info.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_b, GMT_IS_LAT, &project_info.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT_IS_LON, GMT_scanf (txt_c, GMT_IS_LON, &project_info.pars[2]), txt_c);
			error += GMT_verify_expectations (GMT_IS_LAT, GMT_scanf (txt_d, GMT_IS_LAT, &project_info.pars[3]), txt_d);
			if (project_info.pars[3] < 0.0) {	/* Flip from S hemisphere to N */
				project_info.pars[3] = -project_info.pars[3];
				project_info.pars[2] += 180.0;
				if (project_info.pars[2] >= 360.0) project_info.pars[2] -= 360.0;
			}
			error += GMT_scale_or_width (txt_e, &project_info.pars[4]);
			project_info.pars[6] = 1.0;
			error += !(n_slashes == 4 && n == 5);
			project = GMT_OBLIQUE_MERC;
			break;

		case GMT_UTM:	/* Universal Transverse Mercator */
			n = sscanf (args, "%[^/]/%s", txt_a, txt_b);
			project_info.pars[0] = atof (txt_a);
			switch (args[0]) {
				case '-':	/* Enforce Southern hemisphere convention for y */
					project_info.utm_hemisphere = -1;
					break;
				case '+':	/* Enforce Norther hemisphere convention for y */
					project_info.utm_hemisphere = +1;
					break;
				default:	/* Decide in gmt_map_setup based on -R */
					project_info.utm_hemisphere = 0;
					break;
			}
			mod = toupper ((int)txt_a[strlen(txt_a)-1]);	/* Check if UTM zone has a valid latitude modifier */
			error = 0;
			if (mod >= 'A' && mod <= 'Z') {	/* Got fully qualified UTM zone, e.g., 33N */
				project_info.utm_zoney = (GMT_LONG)mod;
				project_info.utm_hemisphere = -1;
				if (mod >= 'N') project_info.utm_hemisphere = +1;
				if (mod == 'I' || mod == 'O') error++;	/* No such zones */
			}
			project_info.pars[0] = fabs (project_info.pars[0]);
			project_info.utm_zonex = irint (project_info.pars[0]);
			error += GMT_scale_or_width (txt_b, &project_info.pars[1]);
			error += !(n_slashes == 1 && n == 2);
			error += (project_info.utm_zonex < 1 || project_info.utm_zonex > 60);	/* Zones must be 1-60 */
			break;

		default:
			error = TRUE;
			project = GMT_NO_PROJ;
			break;
	}

	if (project != GMT_ZAXIS) project_info.projection = project;
	if (width_given > 1) args[last_pos] = last_char;	/* Restore modifier */

	return (error);
}

GMT_LONG GMT_scale_or_width (char *scale_or_width, double *value) {
	/* Scans character that may contain a scale (1:xxxx or units per degree) or a width.
	   Return 1 upon error */
	GMT_LONG n;
	project_info.units_pr_degree = strncmp (scale_or_width, "1:", (size_t)2);	/* FALSE if scale given as 1:xxxx */
	if (project_info.units_pr_degree)
		*value = GMT_convert_units (scale_or_width, GMT_INCH);
	else {
		n = sscanf (scale_or_width, "1:%lf", value);
		if (n != 1 || *value < 0.0) return (1);
		*value = 1.0 / (*value * project_info.unit);
		if (project_info.gave_map_width) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -J option: Cannot specify map width with 1:xxxx format\n", GMT_program);
			return (1);
		}
	}
	return (0);
}

void GMT_free_plot_array (void) {
	if (GMT_n_alloc) {
		GMT_free ((void *)GMT_x_plot);
		GMT_free ((void *)GMT_y_plot);
		GMT_free ((void *)GMT_pen);
	}
}

void GMT_prepare_3D (void) {	/* Initialize 3-D parameters */
	project_info.z_pars[0] = project_info.z_pars[1] = 0.0;
	project_info.xyz_pos[2] = TRUE;
	project_info.zmin = project_info.zmax = 0.0;
	GMT_z_forward = (PFL) NULL;
	GMT_z_inverse = (PFL) NULL;
	memset ((void *)&z_project, 0, sizeof (struct GMT_THREE_D));
	z_project.view_azimuth = 180.0;
	z_project.view_elevation = 90.0;
	project_info.z_bottom = project_info.z_top = 0.0;
}

GMT_LONG GMT_parse_symbol_option (char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd)
{
	/* mode = 0 for 2-D (psxy) and = 1 for 3-D (psxyz) */
	GMT_LONG decode_error = 0, bset = 0, j, n, k, len, slash = 0, one, colon;
	GMT_LONG check, old_style, col_off = mode;
	char symbol_type, txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT], txt_c[GMT_LONG_TEXT], text_cp[GMT_LONG_TEXT], *c;
	static char *allowed_symbols[2] = {"-+aAbBCcDdeEfGgHhIijJmNnpqrSsTtVvwWxy", "-+aAbCcDdeEfGgHhIijJmNnoOpqrSsTtuUVvwWxy"};
	static char *bar_symbols[2] = {"bB", "-bBoOuU"};

	p->n_required = p->convert_angles = p->n_nondim = 0;
	p->user_unit = p->shrink = p->read_vector = p->base_set = p->u_set = FALSE;

	/* Col_off is the col number of first parameter after (x,y) [or (x,y,z) if mode == 1)].
	   However, if size is not given then that is requred too so col_off++ */
	
	if (!text[0]) {	/* No symbol or size given */
		p->size_x = p->size_y = 0.0;
		symbol_type = '*';
		col_off++;
	}
	else if (isdigit ((int)text[0]) || text[0] == '.') {	/* Size, but no symbol given */
		n = sscanf (text, "%[^/]/%s", txt_a, txt_b);
		p->size_x = p->given_size_x = GMT_convert_units (txt_a, GMT_INCH);
		if (n == 2)
			p->size_y = p->given_size_y = GMT_convert_units (txt_b, GMT_INCH);
		else if (n == 1)
			p->size_y = p->given_size_y = p->size_x;
		else
			decode_error = TRUE;
		symbol_type = '*';
	}
	else if (text[0] == 'l') {	/* Letter symbol is special case */
		strcpy (text_cp, text);
		if ((c = strchr (text_cp, '%'))) {	/* Gave font name or number, too */
			*c = ' ';	/* Make the % a space */
			c++;		/* Go to next character */
			p->font_no = GMT_font_lookup (c, GMT_font, GMT_N_FONTS);
			if (p->font_no >= GMT_N_FONTS) {
				fprintf (stderr, "%s: -Sl contains bad font (set to %s (0))\n", GMT_program, GMT_font[gmtdefs.annot_font[0]].name);
				p->font_no = gmtdefs.annot_font[0];
			}
		}
		if (text[1] == '/') {	/* No size given */
			symbol_type = 'l';
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			col_off++;
		}
		else {
			n = sscanf (text_cp, "%c%[^/]/%s", &symbol_type, txt_a, p->string);
			p->size_x = p->given_size_x = GMT_convert_units (txt_a, GMT_INCH);
			decode_error = (n != 3);
		}
	}
	else if (text[0] == 'k') {	/* Custom symbol spec */
		for (j = strlen (text); j > 0 && text[j] != '/'; j--);;
		if (j == 0) {	/* No slash, i.e., no symbol size given */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			n = sscanf (text, "%c%s", &symbol_type, text_cp);
			col_off++;
		}
		else {
			text[j] = ' ';
			n = sscanf (text, "%c%s %s", &symbol_type, text_cp, txt_a);
			text[j] = '/';
			p->given_size_x = p->size_x = GMT_convert_units (txt_a, GMT_INCH);
		}
	}
	else if (text[0] == 'm') {	/* mathangle gets separate treatment since m gets confused by meter in some tests */
		k = (strchr ("bfl", text[1])) ? 2 : 1;	/* Skip the modifier b, f, or l */
		n = sscanf (text, "%c", &symbol_type);
		if (text[k] == '\0') {	/* No size nor unit */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			p->equal_area = FALSE;
			col_off++;
		}
		else if (strchr ("CcIiMmPp", (int) text[k])) {	/* No size given, only unit information */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if ((p->u = GMT_get_unit (text[k])) < 0) decode_error = TRUE; else p->u_set = TRUE;
			p->equal_area = FALSE;
			col_off++;
		}
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && strchr ("CcIiMmPp", (int) text[1])) {	/* Symbol, but no size given (size assumed given on command line), only unit information */
		n = sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		if (text[1] && (p->u = GMT_get_unit (text[1])) < 0) decode_error = TRUE; else p->u_set = TRUE;
		p->equal_area = FALSE;
		col_off++;
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && (text[1] == '\n' || text[1] == '\0')) {	/* Symbol, but no size given (size assumed given on command line) */
		n = sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		p->equal_area = FALSE;
		col_off++;
	}
	else if (strchr (bar_symbols[mode], (int) text[0])) {	/* Bar, column, cube with size */

		/* Bar:		-Sb|B<size_x|y>[c|i|m|p|u][b<base>]				*/
		/* Column:	-So|O<size_x>[c|i|m|p][/<ysize>[c|i|m|p]][u][b<base>]	*/
		/* Cube:	-Su|U<size_x>[c|i|m|p|u]	*/

		for (j = 0; text[j]; j++) {
			if (text[j] == '/') slash = j;
			if (text[j] == 'b' || text[j] == 'B') bset = j;
		}
		strcpy (text_cp, text);
		if (bset) text_cp[bset] = 0;	/* Chop off the b<base> from copy */
		if ((bset && text_cp[bset-1] == 'u') || (j && text[j-1] == 'u')) p->user_unit = TRUE;
		if (slash) {	/* Separate x/y sizes */
			n = sscanf (text_cp, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);
			decode_error = (n != 3);
			if (p->user_unit) {
				p->size_x = p->given_size_x = atof (txt_a);
				p->size_y = p->given_size_y = atof (txt_b);
			}
			else {
				p->size_x = p->given_size_x = GMT_convert_units (txt_a, GMT_INCH);
				p->size_y = p->given_size_y = GMT_convert_units (txt_b, GMT_INCH);
			}
		}
		else {	/* Only a single x = y size */
			n = sscanf (text_cp, "%c%s", &symbol_type, txt_a);
			if (n == 2) {
				if (p->user_unit) {
					p->size_x = p->given_size_x = atof (txt_a);
					p->size_y = p->given_size_y = p->size_x;
				}
				else {
					p->size_x = p->given_size_x = GMT_convert_units (txt_a, GMT_INCH);
					p->size_y = p->given_size_y = p->size_x;
				}
			}
			else {
				if (p->size_x == 0.0) p->size_x = p->given_size_x;
				if (p->size_y == 0.0) p->size_y = p->given_size_y;
			}
		}
	}
	else {
		char s_upper;
		n = sscanf (text, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);
		s_upper = (char)toupper ((int)symbol_type);
		if (s_upper == 'F' || s_upper == 'V' || s_upper == 'Q' || s_upper == 'M') {	/* "Symbols" that do not take normal symbol size */
			p->size_y = p->given_size_y = 0.0;
		}
		else {
			p->size_x = p->given_size_x = GMT_convert_units (txt_a, GMT_INCH);
			if (n == 3)
				p->size_y = p->given_size_y = GMT_convert_units (txt_b, GMT_INCH);
			else if (n == 2)
				p->size_y = p->given_size_y = p->size_x;
			else
				decode_error = TRUE;
		}
		p->equal_area = FALSE;
	}

	check = TRUE;
	switch (symbol_type) {
		case '*':
			p->symbol = GMT_SYMBOL_NOT_SET;
			break;
		case '-':
			p->symbol = GMT_SYMBOL_XDASH;
			break;
		case 'A':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 'a':
			p->symbol = GMT_SYMBOL_STAR;
			break;
		case 'B':
			p->symbol = GMT_SYMBOL_BARX;
			if (bset) {
				p->base = atof (&text[bset+1]);
				p->base_set = TRUE;
			}
			break;
		case 'b':
			p->symbol = GMT_SYMBOL_BARY;
			if (bset) {
				p->base = atof (&text[bset+1]);
				p->base_set = TRUE;
			}
			break;
		case 'C':
		case 'c':
			p->symbol = GMT_SYMBOL_CIRCLE;
			break;
		case 'D':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 'd':
			p->symbol = GMT_SYMBOL_DIAMOND;
			break;
		case 'E':	/* Expect axis in km to be scaled based on -J */
			p->convert_angles = 1;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle in degrees */
			p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in km, not inches or cm etc */
			p->nondim_col[p->n_nondim++] = 4 + mode;
		case 'e':
			p->symbol = GMT_SYMBOL_ELLIPSE;
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle in degrees */
			check = FALSE;
			break;

		case 'f':	/* Fronts:   -Sf<spacing>/<size>[dir][type][:<offset>]	*/
			p->symbol = GMT_SYMBOL_FRONT;
			p->f.f_off = 0.0;
			strcpy (text_cp, text);
			if ((c = strchr (text_cp, ':'))) {	/* Gave :<offset>, set it and strip it off */
				c++;	/* Skip over the colon */
				p->f.f_off = GMT_convert_units (c, GMT_INCH);
				c--;	/* Go back to colon */
				*c = 0;	/* Effectively chops off the offset modifier */
			}
			len = strlen (text_cp) - 1;

			old_style = FALSE;
			switch (text_cp[len]) {
				case 'f':	/* Fault front */
					p->f.f_symbol = GMT_FRONT_FAULT;
					len--;
					break;
				case 't':	/* Triangle front */
					p->f.f_symbol = GMT_FRONT_TRIANGLE;
					len--;
					break;
				case 's':	/* Strike-slip front */
					p->f.f_symbol = GMT_FRONT_SLIP;
					len--;
					break;
				case 'c':	/* [half-]circle front */
					p->f.f_symbol = GMT_FRONT_CIRCLE;
					len--;
					break;
				case 'b':	/* [half-]square front */
					p->f.f_symbol = GMT_FRONT_BOX;
					len--;
					break;

				/* Old style (backward compatibility) */

				case 'L':	/* Left triangle */
					p->f.f_symbol = GMT_FRONT_TRIANGLE;
				case 'l':	/* Left ticked fault */
					p->f.f_sense = GMT_FRONT_LEFT;
					old_style = TRUE;
					break;
				case 'R':	/* Right triangle */
					p->f.f_symbol = GMT_FRONT_TRIANGLE;
				case 'r':	/* Right ticked fault */
					p->f.f_sense = GMT_FRONT_RIGHT;
					old_style = TRUE;
					break;
				default:
					p->f.f_sense = GMT_FRONT_CENTERED;
					break;
			}

			if (!old_style) {
				switch (text_cp[len]) {	/* Get sense - default is centered */
					case 'l':
						p->f.f_sense = GMT_FRONT_LEFT;
						break;
					case 'r':
						p->f.f_sense = GMT_FRONT_RIGHT;
						break;
					default:
						len++;
						p->f.f_sense = GMT_FRONT_CENTERED;
						if (p->f.f_symbol == GMT_FRONT_SLIP) {
							fprintf (stderr, "%s: Error in Option -Sf: Must specify (l)eft-lateral or (r)ight-lateral slip\n", GMT_program);
							GMT_exit (EXIT_FAILURE);
						}
						break;
				}
			}

			text_cp[len] = 0;	/* Gets rid of the [dir][type] flags, if present */

			/* Pull out and get spacing and size */

			sscanf (&text_cp[1], "%[^/]/%s", txt_a, txt_b);
			p->f.f_gap = (txt_a[0] == '-') ? atof (txt_a) : GMT_convert_units (txt_a, GMT_INCH);
			p->f.f_len = GMT_convert_units (txt_b, GMT_INCH);
			check = FALSE;
			break;
		case 'G':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 'g':
			p->symbol = GMT_SYMBOL_OCTAGON;
			break;
		case 'H':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 'h':
			p->symbol = GMT_SYMBOL_HEXAGON;
			break;
		case 'I':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 'i':
			p->symbol = GMT_SYMBOL_ITRIANGLE;
			break;
		case 'J':	/* Expect dimensions in km to be scaled based on -J */
			p->convert_angles = 1;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in km, not inches or cm etc */
			p->nondim_col[p->n_nondim++] = 4 + mode;
		case 'j':
			p->symbol = GMT_SYMBOL_ROTATERECT;
			p->n_required = 3;
			check = FALSE;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			break;
		case 'l':
			p->symbol = GMT_SYMBOL_TEXT;
			/* Look for a slash that separates size and string: */
			for (j = 1, slash = 0; text_cp[j] && !slash; j++) if (text_cp[j] == '/') slash = j;
			/* Set j to the first char in the string: */
			j = slash + 1;
			/* Copy string characters */
			k = 0;
			while (text_cp[j] && text_cp[j] != ' ' && k < 63) p->string[k++] = text_cp[j++];
			if (!k) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Sl option:  No string given\n", GMT_program);
				decode_error++;
			}
			p->string[k] = 0;
			break;
		case 'm':
			p->symbol = GMT_SYMBOL_MANGLE;
			p->n_required = k = 2;
			switch (text[1]) {	/* Check if f(irst), l(last), b(oth), or none [Default] arrow heads*/
				case 'f':	/* Input (x,y) refers to vector head (the tip), double heads */
					p->v_double_heads = 1;
					break;
				case 'l':	/* Input (x,y) refers to vector head (the tip), double heads */
					p->v_double_heads = 2;
					break;
				case 'b':	/* Input (x,y) refers to balance point of vector, double heads */
					p->v_double_heads = 3;
					break;
				default:	/* No modifier given, default to tail, single head */
					p->v_double_heads = 0;
					k = 1;
					break;
			}
			p->size_x = p->given_size_x = p->size_y = p->given_size_y = GMT_convert_units (&text[k], GMT_INCH);
			p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Angle */
			p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Angle */
			break;
		case 'N':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 'n':
			p->symbol = GMT_SYMBOL_PENTAGON;
			break;
		case 'o':	/*3-D symbol */
			p->shade3D = TRUE;
		case 'O':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_COLUMN;
			if (bset) p->base = atof (&text[bset+1]);
			if (mode == 0) {
				decode_error = TRUE;
				fprintf (stderr, "%s: GMT SYNTAX ERROR -S option:  Symbol type %c is 3-D only\n", GMT_program, symbol_type);
			}
			break;
		case 'p':
			p->symbol = GMT_SYMBOL_POINT;
			if (p->size_x == 0.0) p->size_x = GMT_POINT_SIZE;
			check = FALSE;
			break;
		case 'q':	/* Quoted lines: -Sq[d|n|l|x]<info>[:<labelinfo>] */
			p->symbol = GMT_SYMBOL_QUOTED_LINE;
			for (j = 1, colon = 0; text[j]; j++) if (text[j] == ':') colon = j;
			if (colon) {	/* Gave :<labelinfo> */
				text[colon] = 0;
				decode_error += GMT_contlabel_info ('S', &text[1], &p->G);
				decode_error += GMT_contlabel_specs (&text[colon+1], &p->G);
			}
			else
				decode_error += GMT_contlabel_info ('S', &text[1], &p->G);
			check = FALSE;
			break;
		case 'S':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 's':
			p->symbol = GMT_SYMBOL_SQUARE;
			break;
		case 'T':
			p->equal_area = TRUE;	/* To equal area of circle with same size */
		case 't':
			p->symbol = GMT_SYMBOL_TRIANGLE;
			break;
		case 'u':	/*3-D symbol */
			p->shade3D = TRUE;
		case 'U':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_CUBE;
			if (mode == 0) {
				decode_error = TRUE;
				fprintf (stderr, "%s: GMT SYNTAX ERROR -S option:  Symbol type %c is 3-D only\n", GMT_program, symbol_type);
			}
			break;
		case 'V':
			p->convert_angles = 1;
		case 'v':
			p->symbol = GMT_SYMBOL_VECTOR;
			switch (text[1]) {	/* Check if s(egment), h(ead), b(alance center), or t(ail) have been specified */
				case 'S':	/* Input (x,y) refers to vector head (the tip), double heads */
					p->v_double_heads = TRUE;
				case 's':	/* Input (x,y) refers to vector head (the tip), single head  */
					p->v_just = 3;
					one = 2;
					break;
				case 'H':	/* Input (x,y) refers to vector head (the tip), double heads */
					p->v_double_heads = TRUE;
				case 'h':	/* Input (x,y) refers to vector head (the tip), single head */
					p->v_just = 2;
					one = 2;
					p->nondim_col[p->n_nondim++] = 2 + mode;
					break;
				case 'B':	/* Input (x,y) refers to balance point of vector, double heads */
					p->v_double_heads = TRUE;
				case 'b':	/* Input (x,y) refers to balance point of vector, single head */
					p->v_just = 1;
					one = 2;
					p->nondim_col[p->n_nondim++] = 2 + mode;
					break;
				case 'T':	/* Input (x,y) refers to tail of vector, double heads */
					p->v_double_heads = TRUE;
				case 't':	/* Input (x,y) refers to tail of vector [Default], single head */
					p->v_just = 0;
					one = 2;
					p->nondim_col[p->n_nondim++] = 2 + mode;
					break;
				default:	/* No modifier given, default to tail, single head */
					p->v_just = 0;
					one = 1;
					p->nondim_col[p->n_nondim++] = 2 + mode;
					break;
			}
			for (j = one; text[j] && text[j] != 'n'; j++);
			len = strlen(text) - 1;
			if (text[j] == 'n') {	/* Normalize option used */
				k = GMT_get_unit (text[len]);
				if (k >= 0) { p->u = k; p->u_set = TRUE;}
				p->v_norm = atof (&text[j+1]);
				if (p->v_norm > 0.0) {
					p->v_shrink = 1.0 / p->v_norm;
					p->symbol = GMT_SYMBOL_VECTOR2;
				}
				text[j] = 0;	/* Chop off the shrink part */
			}
			if (text[one]) {
				/* It is possible that the user have appended a unit modifier after
				 * the <size> argument (which here are vector attributes).  We use that
				 * to set the unit, but only if the vector attributes themselves have
				 * units. (If not we would override MEASURE_UNIT without cause).
				 * So, -SV0.1i/0.2i/0.3ic will expect 4th column to have length in cm
				 * while SV0.1i/0.2i/0.3i expects data units in MEASURE_UNIT
				 */

				if (isalpha ((int)text[len]) && isalpha ((int)text[len-1])) {
					p->u = GMT_get_unit (text[len]);
					if (p->u >= 0) p->u_set = TRUE;
					text[len] = 0;
				}
				sscanf (&text[one], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				p->v_width  = GMT_convert_units (txt_a, GMT_INCH);
				p->h_length = GMT_convert_units (txt_b, GMT_INCH);
				p->h_width  = GMT_convert_units (txt_c, GMT_INCH);
			}
			if (p->symbol == GMT_SYMBOL_VECTOR2) text[j] = 'n';	/* Put back the n<shrink> part */
			p->read_vector = TRUE;
			p->n_required = 2;
			check = FALSE;
			break;
		case 'W':
			p->convert_angles = 1;
		case 'w':
			p->symbol = GMT_SYMBOL_PIE;
			p->n_required = 2;
			p->nondim_col[p->n_nondim++] = 2 + col_off;
			p->nondim_col[p->n_nondim++] = 3 + col_off;
			break;
		case 'r':
			p->symbol = GMT_SYMBOL_RECT;
			p->n_required = 2;
			check = FALSE;
			break;
		case '+':
			p->symbol = GMT_SYMBOL_PLUS;
			break;
		case 'x':
			p->symbol = GMT_SYMBOL_CROSS;
			break;
		case 'y':
			p->symbol = GMT_SYMBOL_YDASH;
			break;
		case 'z':
			p->symbol = GMT_SYMBOL_ZDASH;
			break;
		case 'k':
			p->symbol = GMT_SYMBOL_CUSTOM;
			p->custom = GMT_get_custom_symbol (text_cp);
			break;
		default:
			decode_error = TRUE;
			fprintf (stderr, "%s: GMT SYNTAX ERROR -S option:  Unrecognized symbol type %c\n", GMT_program, symbol_type);
			break;
	}
	if (p->given_size_x == 0.0 && check) {
		p->read_size = TRUE;
		p->n_required++;
		if (p->symbol == GMT_SYMBOL_COLUMN) p->n_required++;
	}
	else
		p->read_size = FALSE;
	if (!cmd && p->symbol == GMT_SYMBOL_COLUMN) {
		if (!bset) p->base = (project_info.xyz_projection[2] == GMT_LOG10) ? 1.0 : 0.0;
	}
	if (!cmd && p->symbol == GMT_SYMBOL_BARX) {
		if (!bset) p->base = (project_info.xyz_projection[GMT_X] == GMT_LOG10) ? 1.0 : 0.0;
	}
	if (!cmd && p->symbol == GMT_SYMBOL_BARY) {
		if (!bset) p->base = (project_info.xyz_projection[GMT_Y] == GMT_LOG10) ? 1.0 : 0.0;
	}
	p->size_x2 = 0.5 * p->size_x;
	if (mode) p->size_y2 = 0.5 * p->size_y;

	return (decode_error);
}

void GMT_extract_label (char *line, char *label)
{
	GMT_LONG i = 0, j, j0;
	char *p;

	label[0] = '\0';	/* Remove previous label */
	if ((p = strstr (line, " -L")) || (p = strstr (line, "	-L")))	/* Get label specified wih -L option */
		i = p + 3 - line;
	else {								/* Bypass whitespace and pick first word */
		while (line[i] && (line[i] == ' ' || line[i] == '\t')) i++;

	}
	if ((p = strchr (&line[i], '\"'))) {	/* Gave several words as label */
		for (j0 = j = i + 1; line[j] != '\"'; j++);
		if (line[j] == '\"') {	/* Found the matching quote */
			strncpy (label, &line[j0], (size_t)(j-j0));
			label[j-j0] = '\0';
		}
		else {			/* Missing the matching quote */
			sscanf (&line[i], "%s", label);
			fprintf (stderr, "%s: Warning: Label (%s) not terminated by matching quote\n", GMT_program, label);
		}
	}
	else
		sscanf (&line[i], "%s", label);
}

char *GMT_putpen (struct GMT_PEN *pen)
{
	/* GMT_putpen creates a GMT textstring equivalent of the specified pen */

	static char text[BUFSIZ];
	GMT_LONG i;

	if (pen->texture[0]) {

		if (pen->rgb[0] == 0 && pen->rgb[0] == pen->rgb[1] && pen->rgb[1] == pen->rgb[2]) /* Default black pen */
			sprintf (text, "%.5gp,,%s:%.5g", pen->width, pen->texture, pen->offset);
		else
			sprintf (text, "%.5gp,%d/%d/%d,%s:%.5g", pen->width, pen->rgb[0], pen->rgb[1], pen->rgb[2], pen->texture, pen->offset);
		for (i = 0; text[i]; i++) if (text[i] == ' ') text[i] = '_';
	}
	else {
		if (pen->rgb[0] == 0 && pen->rgb[0] == pen->rgb[1] && pen->rgb[1] == pen->rgb[2]) /* Default black pen */
			sprintf (text, "%.5gp", pen->width);
		else
			sprintf (text, "%.5gp,%d/%d/%d", pen->width, pen->rgb[0], pen->rgb[1], pen->rgb[2]);
	}

	return (text);
}

GMT_LONG GMT_check_region (double w, double e, double s, double n)
{	/* If region is given then we must have w < e and s < n */
	return ((w >= e || s >= n) && project_info.region);
}

GMT_LONG GMT_get_unit (char c)
{
	/* Converts cC, iI, mM, and pP into 0-3 */

	GMT_LONG i;
	switch ((int)c) {
		case 'C':	/* cm */
		case 'c':
			i = 0;
			break;
		case 'I':	/* inch */
		case 'i':
			i = 1;
			break;
		case 'M':	/* meter */
		case 'm':
			i = 2;
			break;
		case 'P':	/* point */
		case 'p':
			i = 3;
			break;
		default:	/* error */
			i = -1;
			break;
	}
	return (i);
}

void GMT_init_scales (GMT_LONG unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name) {
	/* unit is 0-6 and stands for m, km, miles, nautical miles, inch, cm, or point */
	/* fwd_scale is used to convert user distance units to meter */
	/* inv_scale is used to convert meters to user distance units */
	/* inch_to_unit is used to convert internal inches to users units (c, i, m, p) */
	/* unit_to_inch is used to convert users units (c, i, m, p) to internal inches */
	/* unit_name is the name of the users measure unit (cm/inch/m/point) */

	double scales[7];

	/* These scales are used if 1 to 1 scaling is chosen */

	scales[0] = 1.0;		/* m in m */
	scales[1] = 1000.0;		/* m in km */
	scales[2] = METERS_IN_A_MILE;		/* m in miles */
	scales[3] = METERS_IN_A_NAUTICAL_MILE;	/* m in nautical miles */
	scales[4] = 0.0254;		/* m in inch */
	scales[5] = 0.01;		/* m in cm */
	scales[6] = 0.0254 / 72.0;	/* m in points */

	/* These scales are used when 1:1 is not set to ensure that the
	 * output (or input with -I) is given (taken) in the units set
	 * by MEASURE_UNIT */

	switch (gmtdefs.measure_unit) {
		case GMT_CM:
			*inch_to_unit = 2.54;
			strcpy (unit_name, "cm");
			break;
		case GMT_INCH:
			*inch_to_unit = 1.0;
			strcpy (unit_name, "inch");
			break;
		case GMT_M:
			*inch_to_unit = 0.0254;
			strcpy (unit_name, "m");
			break;
		case GMT_PT:
			*inch_to_unit = 72.0;
			strcpy (unit_name, "point");
			break;
	}
	*unit_to_inch = 1.0 / (*inch_to_unit);
	*fwd_scale = 1.0 / scales[unit];
	*inv_scale = scales[unit];
}

GMT_LONG GMT_check_scalingopt (char option, char unit, char *unit_name) {
	GMT_LONG mode;

	switch (unit) {
		case '\0':
			mode = 0;
			strcpy (unit_name, "m");
			break;
		case 'k':
		case 'K':
			mode = 1;
			strcpy (unit_name, "km");
			break;
		case 'm':
		case 'M':
			mode = 2;
			strcpy (unit_name, "miles");
			break;
		case 'n':
		case 'N':
			mode = 3;
			strcpy (unit_name, "nautical miles");
			break;
		case 'i':
		case 'I':
			mode = 4;
			strcpy (unit_name, "inch");
			break;
		case 'c':
		case 'C':
			mode = 5;
			strcpy (unit_name, "cm");
			break;
		case 'p':
		case 'P':
			mode = 6;
			strcpy (unit_name, "point");
			break;
		default:
			fprintf (stderr, "%s: GMT ERROR Option -%c: Only append one of cimpkn\n", GMT_program, option);
			GMT_exit (EXIT_FAILURE);
	}

	return (mode);
}

GMT_LONG GMT_set_measure_unit (char unit) {
	/* Option to override the GMT measure unit default */

	switch (unit) {
		case 'm':
		case 'M':
			gmtdefs.measure_unit = GMT_M;
			break;
		case 'i':
		case 'I':
			gmtdefs.measure_unit = GMT_INCH;
			break;
		case 'c':
		case 'C':
			gmtdefs.measure_unit = GMT_CM;
			break;
		case 'p':
		case 'P':
			gmtdefs.measure_unit = GMT_PT;
			break;
		default:
			return (GMT_MAP_BAD_MEASURE_UNIT);
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_init_time_system_structure (struct GMT_TIME_SYSTEM *time_system) {
	/* Processes strings time_system.unit and time_system.epoch to produce a time system scale
	   (units in seconds), inverse scale, and rata die number and fraction of the epoch (days).
	   Return values: 0 = no error, 1 = unit error, 2 = epoch error, 3 = unit and epoch error.
	*/
	GMT_LONG error = GMT_NOERROR;

	/* Check the unit sanity:  */
	switch (time_system->unit) {
		case 'y':
		case 'Y':
			/* This is a kludge:  we assume all years are the same length, thinking that a user
			with decimal years doesn't care about precise time.  To do this right would
			take an entirely different scheme, not a simple unit conversion. */
			time_system->scale = GMT_YR2SEC_F;
			break;
		case 'o':
		case 'O':
			/* This is also a kludge:  we assume all months are the same length, thinking that a user
			with decimal years doesn't care about precise time.  To do this right would
			take an entirely different scheme, not a simple unit conversion. */
			time_system->scale = GMT_MON2SEC_F;
			break;
		case 'd':
		case 'D':
			time_system->scale = GMT_DAY2SEC_F;
			break;
		case 'h':
		case 'H':
			time_system->scale = GMT_HR2SEC_F;
			break;
		case 'm':
		case 'M':
			time_system->scale = GMT_MIN2SEC_F;
			break;
		case 's':	/* For backwards compatibility. Should be 'c' instead */
		case 'S':
			time_system->unit = 'c';
		case 'c':
		case 'C':
			time_system->scale = 1.0;
			break;
		default:
			error += 1;
			break;
	}

	/* Set inverse scale and store it to avoid divisions later */
	time_system->i_scale = 1.0 / time_system->scale;

	/* Now convert epoch into rata die number and fraction */
	if (GMT_scanf_epoch (time_system->epoch, &time_system->rata_die, &time_system->epoch_t0)) error += 2;

	return (error);
}

GMT_LONG GMT_scanf_epoch (char *s, GMT_cal_rd *rata_die, double *t0) {

	/* Read a string which must be in one of these forms:
		[-]yyyy-mm-dd[T| [hh:mm:ss.sss]]
		[-]yyyy-Www-d[T| [hh:mm:ss.sss]]
	   Hence, data and clock can be separated by 'T' or ' ' (space), and the clock string is optional.
	   In fact, seconds can be decimal or integer, or missing. Minutes and hour are optional too.
	   Examples: 2000-01-01, 2000-01-01T, 2000-01-01 00:00, 2000-01-01T00, 2000-01-01T00:00:00.000
	*/

	double ss = 0.0;
	GMT_LONG i, yy, mo, dd, hh = 0, mm = 0;
	GMT_cal_rd rd;
	char tt[8];

	i = 0;
	while (s[i] && s[i] == ' ') i++;
	if (!(s[i])) return (-1);
	if (strchr (&s[i], 'W') ) {	/* ISO calendar string, date with or without clock */
		if (sscanf (&s[i], "%5" GMT_LL "d-W%2" GMT_LL "d-%1" GMT_LL "d%[^0-9:-]%2" GMT_LL "d:%2" GMT_LL "d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (GMT_iso_ywd_is_bad (yy, mo, dd) ) return (-1);
		rd = GMT_rd_from_iywd (yy, mo, dd);
	}
	else {				/* Gregorian calendar string, date with or without clock */
		if (sscanf (&s[i], "%5" GMT_LL "d-%2" GMT_LL "d-%2" GMT_LL "d%[^0-9:-]%2" GMT_LL "d:%2" GMT_LL "d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (GMT_g_ymd_is_bad (yy, mo, dd) ) return (-1);
		rd = GMT_rd_from_gymd (yy, mo, dd);
	}
	if (GMT_hms_is_bad (hh, mm, ss)) return (-1);

	*rata_die = rd;								/* Rata day number of epoch */
	*t0 =  (GMT_HR2SEC_F * hh + GMT_MIN2SEC_F * mm + ss) * GMT_SEC2DAY;	/* Fractional day (0<= t0 < 1) since rata_die of epoch */
	return (0);
}


/* Load a PostScript encoding from a file, given the filename.
 * Use Brute Force and Ignorance.
 */
static GMT_LONG load_encoding (struct gmt_encoding *enc)
{
	char line[GMT_LONG_TEXT], symbol[GMT_LONG_TEXT];
	GMT_LONG code = 0, pos;
	FILE *in;

	GMT_getsharepath ("pslib", enc->name, ".ps", line);
	if ((in = fopen (line, "r")) == NULL) {
		perror (line);
		GMT_exit (EXIT_FAILURE);
	}

	while (fgets (line, (int)GMT_LONG_TEXT, in))
	{
		pos = 0;
		while ((GMT_strtok (line, " /\t\n", &pos, symbol)))
		{
			if (strcmp (symbol, "[") == 0)	/* We have found the start of the encoding array. */
			{
				code = 0;
				continue;
			}
			if (strcmp (symbol, "degree") == 0)
				enc->code[gmt_degree] = code;
			else if (strcmp (symbol, "ring") == 0)
				enc->code[gmt_ring] = code;
			else if (strcmp (symbol, "quotedbl") == 0)
				enc->code[gmt_dquote] = code;
			else if (strcmp (symbol, "quotesingle") == 0)
				enc->code[gmt_squote] = code;
			else if (strcmp (symbol, "colon") == 0)
				enc->code[gmt_colon] = code;
			code++;
		}
	}

	fclose (in);
	return (GMT_NOERROR);
}

void GMT_verify_encodings () {
	/* Check that special map-related codes are present - if not give warning */

	/* First check for degree symbol */

	if (gmtdefs.encoding.code[gmt_ring] == 32 && gmtdefs.encoding.code[gmt_degree] == 32) {	/* Neither /ring or /degree encoded */
		fprintf (stderr, "GMT Warning: Selected character encoding does not have suitable degree symbol - will use space instead\n");
	}
	else if (gmtdefs.degree_symbol == gmt_ring && gmtdefs.encoding.code[gmt_ring] == 32) {		/* want /ring but only /degree is encoded */
		fprintf (stderr, "GMT Warning: Selected character encoding does not have ring symbol - will use degree symbol instead\n");
		gmtdefs.degree_symbol = gmt_degree;
	}
	else if (gmtdefs.degree_symbol == gmt_degree && gmtdefs.encoding.code[gmt_degree] == 32) {	/* want /degree but only /ring is encoded */
		fprintf (stderr, "GMT Warning: Selected character encoding does not have degree symbol - will use ring symbol instead\n");
		gmtdefs.degree_symbol = gmt_ring;
	}

	/* Then single quote for minute symbol... */

	if (gmtdefs.degree_symbol < 2 && gmtdefs.encoding.code[gmt_squote] == 32) {
		fprintf (stderr, "GMT Warning: Selected character encoding does not have minute symbol (single quote) - will use space instead\n");
	}

	/* ... and double quote for second symbol */

	if (gmtdefs.degree_symbol < 2 && gmtdefs.encoding.code[gmt_dquote] == 32) {
		fprintf (stderr, "GMT Warning: Selected character encoding does not have second symbol (double quote) - will use space instead\n");
	}
}

GMT_LONG GMT_init_fonts (GMT_LONG *n_fonts)
{
	FILE *in = NULL;
	GMT_LONG i = 0, n_GMT_fonts, n_alloc = 0;
	char buf[BUFSIZ];
	char fullname[BUFSIZ];

	/* Loads the available fonts for this installation */

	/* First the standard 35 PostScript fonts from Adobe */

	GMT_getsharepath ("pslib", "PS_font_info", ".d", fullname);
	if ((in = fopen (fullname, "r")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: ");
		perror (fullname);
		GMT_exit (EXIT_FAILURE);
	}

	GMT_set_meminc (GMT_SMALL_CHUNK);	/* Only allocate a small amount */
	while (fgets (buf, BUFSIZ, in)) {
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
		if (i == n_alloc) n_alloc = GMT_alloc_memory ((void **)&GMT_font, i, n_alloc, sizeof (struct GMT_FONT), GMT_program);
		if (sscanf (buf, "%s %lf %*d", fullname, &GMT_font[i].height) != 2) {
			fprintf (stderr, "GMT Fatal Error: Trouble decoding font info for font %ld\n", i);
			GMT_exit (EXIT_FAILURE);
		}
		GMT_font[i].name = strdup (fullname);
		i++;
	}
	fclose (in);
	*n_fonts = n_GMT_fonts = i;

	/* Then any custom fonts */

	if (GMT_getsharepath ("pslib", "CUSTOM_font_info", ".d", fullname)) {	/* Decode Custom font file */
		if ((in = fopen (fullname, "r")) == NULL) {
			fprintf (stderr, "GMT Fatal Error: ");
			perror (fullname);
			GMT_exit (EXIT_FAILURE);
		}

		while (fgets (buf, BUFSIZ, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			if (i == n_alloc) n_alloc = GMT_alloc_memory ((void **)&GMT_font, i, n_alloc, sizeof (struct GMT_FONT), GMT_program);
			if (sscanf (buf, "%s %lf %*d", fullname, &GMT_font[i].height) != 2) {
				fprintf (stderr, "GMT Fatal Error: Trouble decoding custom font info for font %ld\n", i - n_GMT_fonts);
				GMT_exit (EXIT_FAILURE);
			}
			GMT_font[i].name = strdup (fullname);
			i++;
		}
		fclose (in);
		*n_fonts = i;
	}
	(void)GMT_alloc_memory ((void **)&GMT_font, 0, i, sizeof (struct GMT_FONT), GMT_program);
	GMT_reset_meminc ();
	return (GMT_NOERROR);
}

void *New_GMT_Ctrl () {	/* Allocate and initialize a new common control structure */
	struct GMT_CTRL *C;

	C = (struct GMT_CTRL *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_CTRL), "New_GMT_Ctrl");
	C->common = (struct GMT_COMMON *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_COMMON), "New_GMT_Ctrl");
	C->gmtdefs = (struct GMT_DEFAULTS *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_DEFAULTS), "New_GMT_Ctrl");
	C->hidden = (struct GMT_HIDDEN *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_HIDDEN), "New_GMT_Ctrl");

	/* Initialize values whose defaults are not necessarily 0/FALSE/NULL */

	/* [2]  -H[i][<nrecs>] */
	C->common->H.active[GMT_IN] = gmtdefs.io_header[GMT_IN];
	C->common->H.active[GMT_OUT] = gmtdefs.io_header[GMT_OUT];
	C->common->H.n_recs = gmtdefs.n_header_recs;
	/* [7]  -P */
	C->common->P.active = gmtdefs.portrait;
	/* [9]  -U */
	C->common->U.active = gmtdefs.unix_time;
	C->common->U.just = gmtdefs.unix_time_just;
	C->common->U.x = gmtdefs.unix_time_pos[GMT_X];
	C->common->U.y = gmtdefs.unix_time_pos[GMT_Y];
	/* [10]  -V */
	C->common->V.active = gmtdefs.verbose;
	/* [11]  -X */
	C->common->X.off = gmtdefs.x_origin;
	/* [12] -Y */
	C->common->Y.off = gmtdefs.y_origin;
	/* [13]  -c */
	C->common->c.copies = gmtdefs.n_copies;
	/* [14]  -:[i|o] */
	C->common->t.toggle[GMT_IN] = gmtdefs.xy_toggle[GMT_IN];
	C->common->t.toggle[GMT_OUT] = gmtdefs.xy_toggle[GMT_OUT];

	return ((void *)C);
}

void Free_GMT_Ctrl (struct GMT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;	/* Never was allocated */
	GMT_free ((void *)C->common);
	GMT_free ((void *)C->gmtdefs);
	GMT_free ((void *)C->hidden);
	GMT_free ((void *)C);
}

#ifdef WIN32

/* Make dummy functions so GMT will link under WIN32 */

struct passwd *getpwuid (const int uid)
{
	return ((struct passwd *)NULL);
}

int getuid (void) {
	return (0);
}

#endif

#ifdef SET_IO_MODE

/* Under non-Unix operating systems running on the PC, such as
 * Windows and OS/2, files are opened in either TEXT or BINARY
 * mode.  This difference does not exist under UNIX, but is important
 * on the PC.  Specifically, it causes a problem when a program
 * that writes/reads standard i/o wants to use binary data.
 * In those situations we must change the default (TEXT) mode of
 * the file handle to BINARY via a call to "setmode".
 *
 * Under OS/2, using the EMX compilation system, one can change
 * the mode of input/output from TEXT to BINARY using _fsetmode.
 * Suggested by Alan Cogbill, Los Alamos National Laboratory
 *
 * This can also be done under Win32 with the Microsoft VC++
 * compiler which supports ANSI-C (P. Wessel).  This may be true
 * of other Win32 compilers as well.  Until we know if _setmode
 * would work in the same way under OS/2 we choose to take two
 * different routes, hence the #ifdefs below
 */

void GMT_setmode (int i_or_o)
{
	/* Changes the stream to deal with BINARY rather than TEXT data */

	FILE *fp;

	if (GMT_io.binary[i_or_o]) {	/* User wants binary */

		fp = (i_or_o == 0) ? GMT_stdin : GMT_stdout;
		fflush (fp);	/* Should be untouched but anyway... */
#ifdef _WIN32
		setmode (fileno (fp), _O_BINARY);
#else
		_fsetmode (fp, "b");
#endif
	}
}

#endif	/* SET_IO_MODE */
