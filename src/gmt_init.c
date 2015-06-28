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
 *--------------------------------------------------------------------
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 */

/*!
 * \file gmt_init.c
 * \brief gmt_init.c contains code which is used by all GMT programs
 *
 * The PUBLIC functions are:
 *
 *	GMT_explain_options		Prints explanations for the common options\n
 *	GMT_parse_common_options	Interprets common options, such as -B, -R, --\n
 *	GMT_getdefaults			Initializes the GMT global parameters\n
 *	GMT_putdefaults			Dumps the GMT global parameters\n
 *	GMT_hash_init			Initializes a hash\n
 *	GMT_hash_lookup			Key - id lookup using hashing\n
 *	GMT_hash			Key - id lookup using hashing\n
 *	GMT_begin			Gets history and init parameters\n
 *	GMT_end				Cleans up and returns\n
 *	gmt_history			Read and update the gmt.history file\n
 *	GMT_putcolor			Encode color argument into textstring\n
 *	GMT_putrgb			Encode color argument into r/g/b textstring\n
 *	GMT_puthsv			Encode color argument into h-s-v textstring\n
 *	GMT_putcmyk			Encode color argument into c/m/y/k textstring
 *
 * The INTERNAL functions are:
 *
 *	GMT_loaddefaults		Reads the GMT global parameters from gmt.conf\n
 *	GMT_savedefaults		Writes the GMT global parameters to gmt.conf\n
 *	GMT_parse_?_option		Decode the one of the common options\n
 *	gmt_setparameter		Sets a default value given keyword,value-pair\n
 *	gmt_setshorthand		Reads and initializes the suffix shorthands\n
 *	GMT_get_ellipsoid		Returns ellipsoid id based on name\n
 *	gmt_scanf_epoch			Get user time origin from user epoch string\n
 *	GMT_init_time_system_structure  Does what it says\n
 */

#include "gmt_dev.h"
#include <stdarg.h>
#include "gmt_internals.h"
#include "common_runpath.h"

#ifdef GMT_MATLAB
#	include <mex.h>
#endif

#define USER_MEDIA_OFFSET 1000

#define GMT_def(case_val) * GMT->session.u2u[GMT_INCH][GMT_unit_lookup(GMT, GMT->current.setting.given_unit[case_val], GMT->current.setting.proj_length_unit)], GMT->current.setting.given_unit[case_val]

#define Return { GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Found no history for option -%s\n", str); return (-1); }

#define GMT_more_than_once(GMT,active) (GMT_check_condition (GMT, active, "Warning: Option -%c given more than once\n", option))

#define GMT_COMPAT_INFO "Please see " GMT_TRAC_WIKI "doc/" GMT_PACKAGE_VERSION "/GMT_Docs.html#new-features-in-gmt-5 for more information.\n"

#define GMT_COMPAT_WARN GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Parameter %s is deprecated.\n" GMT_COMPAT_INFO, GMT_keywords[case_val])
#define GMT_COMPAT_CHANGE(new_P) GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Parameter %s is deprecated. Use %s instead.\n" GMT_COMPAT_INFO, GMT_keywords[case_val], new_P)
#define GMT_COMPAT_OPT(new_P) if (strchr (list, option)) { GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -%c is deprecated. Use -%c instead.\n" GMT_COMPAT_INFO, option, new_P); option = new_P; }

extern int gmt_geo_C_format (struct GMT_CTRL *GMT);
extern void GMT_grdio_init (struct GMT_CTRL *GMT);	/* Defined in gmt_customio.c and only used here */
unsigned int gmt_setparameter (struct GMT_CTRL *GMT, char *keyword, char *value);

/*--------------------------------------------------------------------*/
/* Load private fixed array parameters from include files */
/*--------------------------------------------------------------------*/

#include "gmt_keycases.h"				/* Get all the default case values */
static char *GMT_keywords[GMT_N_KEYS] = {		/* Names of all parameters in gmt.conf */
#include "gmt_keywords.h"
};

static char *GMT_unique_option[GMT_N_UNIQUE] = {	/* The common GMT command-line options [ just the subset that accepts arguments (e.g., -O is not listed) ] */
#include "gmt_unique.h"
};

static char *GMT_media_name[GMT_N_MEDIA] = {		/* Names of all recognized paper formats */
#include "gmt_media_name.h"
};
static struct GMT_MEDIA GMT_media[GMT_N_MEDIA] = {	/* Sizes in points of all paper formats */
#include "gmt_media_size.h"
};

static char *GMT_color_name[GMT_N_COLOR_NAMES] = {	/* Names of all the X11 colors */
#include "gmt_colornames.h"
};

static char *GMT_weekdays[7] = {	/* Days of the week in English [Default] */
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static char *GMT_just_string[12] = {	/* Strings to specify justification */
	"", "BL", "BC", "BR", "", "ML", "MC", "MR", "", "TL", "TC", "TR"
};

/* Local variables to gmt_init.c */

static struct GMT_HASH keys_hashnode[GMT_N_KEYS];

/*! whether to ignore/read/write history file gmt.history */
enum history_mode {
	/*! 0 */	k_history_off = 0,
	/*! 1 */	k_history_read,
	/*! 2 */	k_history_write
};

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

int gmt_get_uservalue (struct GMT_CTRL *GMT, char *txt, int type, double *value, char *err_msg)
{	/* Use to get a single data value of given type and exit if error, and return EXIT_FAILURE */
	int kind;
	if ((kind = GMT_scanf (GMT, txt, type, value)) == GMT_IS_NAN) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error %s: %s\n", err_msg, txt);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	return 0;
}

/*!
	\brief Print to stderr a short explanation for each of the options listed by the variable <options>
	\param GMT ...
	\param options ...

 * The function print to stderr a short explanation for each of the options listed by
 * the variable <options>. Only the common parameter options are covered.
 * Note: The cases below do not directly correspond to the common option letters,
 * although in some cases they do (e.g., case 'B' explains -B). For instance, to
 * display the help for the -r (registration setting for grid) option we use case F.
 * Part of this is historic and part is multiple flavor of output for same option.
 * However, GMT_explain_options is not called directly but via GMT_Option which
 * do accept a list of comma-separated options and there are the normal GMT common
 * option letters, sometimes with modifiers, and it translate between those and the
 * crazy cases below.\n
 * Remaining cases for additional options: A,H,L,M,N,T,W,e,m,q,u,v,w
 */
void GMT_explain_options (struct GMT_CTRL *GMT, char *options) {

	char u, *GMT_choice[2] = {"OFF", "ON"}, *V_code = "qncvld";
	double s;
	unsigned int k;

	if (!options) return;
	if (GMT->common.synopsis.extended) return;	/* Only want to list module-specific options, i.e gave + instead of - */
	u = GMT->session.unit_name[GMT->current.setting.proj_length_unit][0];
	s = GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];	/* Convert from internal inch to users unit */

	for (k = 0; k < strlen (options); k++) {

		switch (options[k]) {

		case 'B':	/* Tickmark option */

			GMT_message (GMT, "\t-B Specify both (1) basemap frame settings and (2) axes parameters.\n");
			GMT_message (GMT, "\t   Frame settings are modified via an optional single invocation of\n");
			GMT_message (GMT, "\t     -B[<axes>][+b][+g<fill>][+n][+o<lon>/<lat>][+t<title>]\n");
			GMT_message (GMT, "\t   Axes parameters are specified via one or more invocations of\n");
			GMT_message (GMT, "\t     -B[p|s][x|y|z]<info>\n\n");
			GMT_message (GMT, "\t   1. Frame settings control which axes to plot, frame fill, title, and type of gridlines:\n");
			GMT_message (GMT, "\t     <axes> is a combination of W,E,S,N,Z and plots those axes only [Default is WESNZ (all)].\n");
			GMT_message (GMT, "\t     Use lower case w,e,s,n,z just to draw and tick (but not annotate) those axes.\n");
			GMT_message (GMT, "\t     For 3-D plots the Z|z[<corners>][+b] controls the vertical axis.  The <corners> specifies\n");
			GMT_message (GMT, "\t     at which corner(s) to erect the axis via a combination of 1,2,3,4; 1 means lower left corner,\n");
			GMT_message (GMT, "\t     2 is lower right, etc., in a counter-clockwise order. [Default automatically selects one axis].\n");
			GMT_message (GMT, "\t     The optional +b will erect a 3-D frame box to outline the 3-D domain [no frame box]. The +b\n");
			GMT_message (GMT, "\t     is also required for x-z or y-z gridlines to be plotted (if such gridlines are selected below).\n");
			GMT_message (GMT, "\t     Append +g<fill> to paint the inside of the map region before further plotting [no fill].\n");
			GMT_message (GMT, "\t     Append +n to have no frame and annotations whatsoever [Default is controlled by WESNZ/wesnz].\n");
			GMT_message (GMT, "\t     Append +o<plon>/<plat> to draw oblique gridlines about this pole [regular gridlines].\n");
			GMT_message (GMT, "\t     Note: the +o modifier is ignored unless gridlines are specified via the axes parameters (below).\n");
			GMT_message (GMT, "\t     Append +t<title> to place a title over the map frame [no title].\n");
			GMT_message (GMT, "\t   2. Axes settings control the annotation, tick, and grid intervals and labels.\n");
			GMT_message (GMT, "\t     The full axes specification is\n");
			GMT_message (GMT, "\t       -B[p|s][x|y|z]<intervals>[+l<label>][+p<prefix>][+u<unit>]\n");
			GMT_message (GMT, "\t     Alternatively, you may break this syntax into two separate -B options:\n");
			GMT_message (GMT, "\t       -B[p|s][x|y|z][+l<label>][+p<prefix>][+u<unit>]\n");
			GMT_message (GMT, "\t       -B[p|s][x|y|z]<intervals>\n");
			GMT_message (GMT, "\t     There are two levels of annotations: Primary and secondary (most situations only require primary).\n");
			GMT_message (GMT, "\t     The -B[p] selects (p)rimary annotations while -Bs specifies (s)econdary annotations.\n");
			GMT_message (GMT, "\t     The [x|y|z] selects which axes the settings apply to.  If none are given we default to xy.\n");
			GMT_message (GMT, "\t     To specify different settings for different axes you must repeat the -B axes option for\n");
			GMT_message (GMT, "\t     each dimension., i.e., provide separate -B[p|s]x, -B[p|s]y, and -B[p|s]z settings.\n");
			GMT_message (GMT, "\t     To prepend a prefix to each annotation (e.g., $ 10, $ 20 ...), add +p<prefix>.\n");
			GMT_message (GMT, "\t     To append a unit to each annotation (e.g., 5 km, 10 km ...), add +u<unit>.\n");
			GMT_message (GMT, "\t     To label an axis, add +l<label>.  Use quotes if <label>, <prefix> or <unit> have spaces.\n");
			GMT_message (GMT, "\t     Geographic map annotations will automatically have degree, minute, seconds units.\n");
			GMT_message (GMT, "\t     The <intervals> setting controls the annotation spacing and is a textstring made up of one or\n");
			GMT_message (GMT, "\t     more substrings of the form [a|f|g][<stride>[+-<phase>][<unit>]], where the (optional) a\n");
			GMT_message (GMT, "\t     indicates annotation and major tick interval, f minor tick interval, and g grid interval.\n");
			GMT_message (GMT, "\t     Here, <stride> is the spacing between ticks or annotations, the (optional)\n");
			GMT_message (GMT, "\t     <phase> specifies phase-shifted annotations/ticks by that amount, and the (optional)\n");
			GMT_message (GMT, "\t     <unit> specifies the <stride> unit [Default is the unit implied in -R]. There can be\n");
			GMT_message (GMT, "\t     no spaces between the substrings; just append items to make one very long string.\n");
			GMT_message (GMT, "\t     For custom annotations or intervals, let <intervals> be c<intfile>; see man page for details.\n");
			GMT_message (GMT, "\t     The optional <unit> modifies the <stride> value accordingly.  For geographic maps you may use\n");
			GMT_message (GMT, "\t       d: arc degree [Default].\n");
			GMT_message (GMT, "\t       m: arc minute.\n");
			GMT_message (GMT, "\t       s: arc second.\n");
			GMT_message (GMT, "\t     For time axes, several units are recognized:\n");
			GMT_message (GMT, "\t       Y: year - plot using all 4 digits.\n");
			GMT_message (GMT, "\t       y: year - plot only last 2 digits.\n");
			GMT_message (GMT, "\t       O: month - format annotation according to FORMAT_DATE_MAP.\n");
			GMT_message (GMT, "\t       o: month - plot as 2-digit integer (1-12).\n");
			GMT_message (GMT, "\t       U: ISO week - format annotation according to FORMAT_DATE_MAP.\n");
			GMT_message (GMT, "\t       u: ISO week - plot as 2-digit integer (1-53).\n");
			GMT_message (GMT, "\t       r: Gregorian week - 7-day stride from chosen start of week (%s).\n",
			             GMT_weekdays[GMT->current.setting.time_week_start]);
			GMT_message (GMT, "\t       K: ISO weekday - format annotation according to FORMAT_DATE_MAP.\n");
			GMT_message (GMT, "\t       k: weekday - plot name of weekdays in selected language [%s].\n", GMT->current.setting.language);
			GMT_message (GMT, "\t       D: day - format annotation according to FORMAT_DATE_MAP, which also determines whether\n");
			GMT_message (GMT, "\t                we should plot day of month (1-31) or day of year (1-366).\n");
			GMT_message (GMT, "\t       d: day - plot as 2- (day of month) or 3- (day of year) integer.\n");
			GMT_message (GMT, "\t       R: Same as d but annotates from start of Gregorian week.\n");
			GMT_message (GMT, "\t       H: hour - format annotation according to FORMAT_CLOCK_MAP.\n");
			GMT_message (GMT, "\t       h: hour - plot as 2-digit integer (0-23).\n");
			GMT_message (GMT, "\t       M: minute - format annotation according to FORMAT_CLOCK_MAP.\n");
			GMT_message (GMT, "\t       m: minute - plot as 2-digit integer (0-59).\n");
			GMT_message (GMT, "\t       S: second - format annotation according to FORMAT_CLOCK_MAP.\n");
			GMT_message (GMT, "\t       s: second - plot as 2-digit integer (0-59; 60-61 if leap seconds are enabled).\n");
			GMT_message (GMT, "\t     Cartesian axes takes no units.\n");
			GMT_message (GMT, "\t     When <stride> is omitted, a reasonable value will be determined automatically, e.g., -Bafg.\n");
			GMT_message (GMT, "\t     Log10 axis: Append l to annotate log10 (value) or p for 10^(log10(value)) [Default annotates value].\n");
			GMT_message (GMT, "\t     Power axis: Append p to annotate value at equidistant pow increments [Default is nonlinear].\n");
			GMT_message (GMT, "\t     See psbasemap man pages for more details and examples of all settings.\n");
			break;

		case 'b':	/* Condensed tickmark option */

			GMT_message (GMT, "\t-B Specify both (1) basemap frame settings and (2) axes parameters.\n");
			GMT_message (GMT, "\t   (1) Frame settings are modified via an optional single invocation of\n");
			GMT_message (GMT, "\t     -B[<axes>][+g<fill>][+n][+o<lon>/<lat>][+t<title>]\n");
			GMT_message (GMT, "\t   (2) Axes parameters are specified via one or more invocations of\n");
			GMT_message (GMT, "\t       -B[p|s][x|y|z]<intervals>[+l<label>][+p<prefix>][+u<unit>]\n");
			GMT_message (GMT, "\t   <intervals> is composed of concatenated [<type>]<stride>[<unit>][l|p] sub-strings\n");
			GMT_message (GMT, "\t   See psbasemap man page for more details and examples of all settings.\n");
			break;

		case 'J':	/* Map projection option */

			GMT_message (GMT, "\t-J Select the map proJection. The projection type is identified by a 1- or\n");
			GMT_message (GMT, "\t   2-character ID (e.g. 'm' or 'kf') or by an abbreviation followed by a slash\n");
			GMT_message (GMT, "\t   (e.g. 'cyl_stere/'). When using a lower-case ID <scale> can be given either as 1:<xxxx>\n");
			GMT_message (GMT, "\t   or in %s/degree along the standard parallel. Alternatively, when the projection ID is\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t   Capitalized, <scale>|<width> denotes the width of the plot in %s\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t   Append h for map height, + for max map dimension, and - for min map dimension.\n");
			GMT_message (GMT, "\t   When the central meridian (lon0) is optional and omitted, the center of the\n");
			GMT_message (GMT, "\t   longitude range specified by -R is used. The default standard parallel is the equator\n");
			GMT_message (GMT, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is given.\n");

			GMT_message (GMT, "\t   -Ja|A<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Lambert Azimuthal Equal Area)\n");
			GMT_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (GMT, "\t     <horizon> is max distance from center of the projection (<= 180, default 90).\n");
			GMT_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is the distance\n");
			GMT_message (GMT, "\t     in %s to the oblique parallel <lat>.\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			GMT_message (GMT, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Albers Equal-Area Conic)\n");
			GMT_message (GMT, "\t     Give origin, 2 standard parallels, and true scale\n");

			GMT_message (GMT, "\t   -Jc|C<lon0>/<lat0><scale>|<width> (Cassini)\n\t     Give central point and scale\n");

			GMT_message (GMT, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<scale>|<width> (Cylindrical Stereographic)\n");
			GMT_message (GMT, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			GMT_message (GMT, "\t     <lat0> = 66.159467 (Miller's modified Gall), 55 (Kamenetskiy's First),\n");
			GMT_message (GMT, "\t     45 (Gall Stereographic), 30 (Bolshoi Sovietskii Atlas Mira), 0 (Braun)\n");

			GMT_message (GMT, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Equidistant Conic)\n");
			GMT_message (GMT, "\t     Give origin, 2 standard parallels, and true scale\n");

			GMT_message (GMT, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Azimuthal Equidistant)\n");
			GMT_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (GMT, "\t     <horizon> is max distance from center of the projection (<= 180, default 180).\n");
			GMT_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is the distance\n");
			GMT_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			GMT_message (GMT, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Gnomonic)\n");
			GMT_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (GMT, "\t     <horizon> is max distance from center of the projection (< 90, default 60).\n");
			GMT_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			GMT_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			GMT_message (GMT, "\t   -Jg|G<lon0>/<lat0>/<scale>|<width> (Orthographic)\n");
			GMT_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			GMT_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			GMT_message (GMT, "\t   -Jg|G<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale>|<width> (General Perspective)\n");
			GMT_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (GMT, "\t     <altitude> is the height (in km) of the viewpoint above local sea level\n");
			GMT_message (GMT, "\t        - if <altitude> less than 10 then it is the distance \n");
			GMT_message (GMT, "\t        from center of earth to viewpoint in earth radii\n");
			GMT_message (GMT, "\t        - if <altitude> has a suffix of 'r' then it is the radius \n");
			GMT_message (GMT, "\t        from the center of earth in kilometers\n");
			GMT_message (GMT, "\t     <azimuth> is azimuth east of North of view\n");
			GMT_message (GMT, "\t     <tilt> is the upward tilt of the plane of projection\n");
			GMT_message (GMT, "\t       if <tilt> < 0 then viewpoint is centered on the horizon\n");
			GMT_message (GMT, "\t     <twist> is the CW twist of the viewpoint in degree\n");
			GMT_message (GMT, "\t     <width> is width of the viewpoint in degree\n");
			GMT_message (GMT, "\t     <height> is the height of the viewpoint in degrees\n");
			GMT_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			GMT_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			GMT_message (GMT, "\t   -Jh|H[<lon0>/]<scale>|<width> (Hammer-Aitoff)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Ji|I[<lon0>/]<scale>|<width> (Sinusoidal)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Jj|J[<lon0>/]<scale>|<width> (Miller)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Jkf|Kf[<lon0>/]<scale>|<width> (Eckert IV)\n\t     Give central meridian (opt) and scale\n");
			GMT_message (GMT, "\t   -Jk|K[s][<lon0>/]<scale>|<width> (Eckert VI)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Lambert Conformal Conic)\n");
			GMT_message (GMT, "\t     Give origin, 2 standard parallels, and true scale\n");

			GMT_message (GMT, "\t   -Jm|M[<lon0>/[<lat0>/]]<scale>|<width> (Mercator).\n");
			GMT_message (GMT, "\t     Give central meridian (opt), true scale parallel (opt), and scale\n");

			GMT_message (GMT, "\t   -Jn|N[<lon0>/]<scale>|<width> (Robinson projection)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Jo|O<parameters> (Oblique Mercator).  Specify one of three definitions:\n");
			GMT_message (GMT, "\t     -Jo|O[a]<lon0>/<lat0>/<azimuth>/<scale>|<width>\n");
			GMT_message (GMT, "\t       Give origin, azimuth of oblique equator, and scale at oblique equator\n");
			GMT_message (GMT, "\t     -Jo|O[b]<lon0>/<lat0>/<lon1>/<lat1>/<scale>|<width>\n");
			GMT_message (GMT, "\t       Give origin, second point on oblique equator, and scale at oblique equator\n");
			GMT_message (GMT, "\t     -Jo|Oc<lon0>/<lat0>/<lonp>/<latp>/<scale>|<width>\n");
			GMT_message (GMT, "\t       Give origin, pole of projection, and scale at oblique equator\n");
			GMT_message (GMT, "\t       Specify region in oblique degrees OR use -R<>r\n");

			GMT_message (GMT, "\t   -Jp|P[a]<scale>|<width>[/<base>][r|z] (Polar (theta,radius))\n");
			GMT_message (GMT, "\t     Linear scaling for polar coordinates.\n");
			GMT_message (GMT, "\t     Optionally append 'a' to -Jp or -JP to use azimuths (CW from North) instead of directions (CCW from East) [default].\n");
			GMT_message (GMT, "\t     Give scale in %s/units, and append theta value for angular offset (base) [0]\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t     Append r to reverse radial direction (s/n must be in 0-90 range) or z to annotate depths rather than radius [Default]\n");

			GMT_message (GMT, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scale>|<width> ((American) Polyconic)\n");
			GMT_message (GMT, "\t     Give central meridian (opt), reference parallel (opt, default = equator), and scale\n");

			GMT_message (GMT, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scale>|<width> (Equidistant Cylindrical)\n");
			GMT_message (GMT, "\t     Give central meridian (opt), standard parallel (opt), and scale\n");
			GMT_message (GMT, "\t     <lat0> = 61.7 (Min. linear distortion), 50.5 (R. Miller equirectangular),\n");
			GMT_message (GMT, "\t     45 (Gall isographic), 43.5 (Min. continental distortion), 42 (Grafarend & Niermann),\n");
			GMT_message (GMT, "\t     37.5 (Min. overall distortion), 0 (Plate Carree, default)\n");

			GMT_message (GMT, "\t   -Jr|R[<lon0>/]<scale>|<width> (Winkel Tripel)\n\t     Give central meridian and scale\n");

			GMT_message (GMT, "\t   -Js|S<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Stereographic)\n");
			GMT_message (GMT, "\t     <lon0>/<lat0> is the center or the projection.\n");
			GMT_message (GMT, "\t     <horizon> is max distance from center of the projection (< 180, default 90).\n");
			GMT_message (GMT, "\t     <scale> is either <1:xxxx> (true at pole) or <slat>/<1:xxxx> (true at <slat>)\n");
			GMT_message (GMT, "\t     or <radius>/<lat> (distance in %s to the [oblique] parallel <lat>.\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			GMT_message (GMT, "\t   -Jt|T<lon0>/[<lat0>/]<scale>|<width> (Transverse Mercator).\n\t         Give central meridian and scale\n");
			GMT_message (GMT, "\t     Optionally, also give the central parallel (default = equator)\n");

			GMT_message (GMT, "\t   -Ju|U<zone>/<scale>|<width> (UTM)\n");
			GMT_message (GMT, "\t     Give zone (A,B,Y,Z, or 1-60 (negative for S hemisphere) or append GMT-X) and scale\n");
			GMT_message (GMT, "\t     Or give -Ju|U<scale>|<width> to have the UTM zone determined from the region.\n");

			GMT_message (GMT, "\t   -Jv|V[<lon0>/]<scale>|<width> (van der Grinten)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Jw|W[<lon0>/]<scale>|<width> (Mollweide)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (GMT, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scale>|<width> (Cylindrical Equal-area)\n");
			GMT_message (GMT, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			GMT_message (GMT, "\t     <lat0> = 50 (Balthasart), 45 (Gall-Peters), 37.5 (Hobo-Dyer), 37.4 (Trystan Edwards),\n");
			GMT_message (GMT, "\t              37.0666 (Caster), 30 (Behrmann), 0 (Lambert, default)\n");

			GMT_message (GMT, "\t   -Jx|X<x-scale|<width>[/<y-scale|height>] (Linear, log, power scaling)\n");
			GMT_message (GMT, "\t     <scale> in %s/units (or 1:xxxx). Optionally, append to <x-scale> and/or <y-scale>:\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t       d         Geographic coordinate (in degrees)\n");
			GMT_message (GMT, "\t       l         Log10 projection\n");
			GMT_message (GMT, "\t       p<power>  x^power projection\n");
			GMT_message (GMT, "\t       t         Calendar time projection using relative time coordinates\n");
			GMT_message (GMT, "\t       T         Calendar time projection using absolute time coordinates\n");
			GMT_message (GMT, "\t     Use / to specify separate x/y scaling (e.g., -Jx0.5c/0.3c).  If 1:xxxxx is used it implies -R is in meters.\n");
			GMT_message (GMT, "\t     If -JX is used then give axes lengths rather than scales.\n");
			break;

		case 'j':	/* Condensed version of J */

			GMT_message (GMT, "\t-J Select map proJection. (<scale> in %s/degree, <width> in %s)\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit],
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t   Append h for map height, or +|- for max|min map dimension.\n");
			GMT_message (GMT, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is set.\n\n");

			GMT_message (GMT, "\t   -Ja|A<lon0>/<lat0>[/<hor>]/<scl (or <radius>/<lat>)|<width> (Lambert Azimuthal EA)\n");

			GMT_message (GMT, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Albers Conic EA)\n");

			GMT_message (GMT, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<lat1>/<lat2>/<scl>|<width> (Cylindrical Stereographic)\n");

			GMT_message (GMT, "\t   -Jc|C<lon0>/<lat0><scl>|<width> (Cassini)\n");

			GMT_message (GMT, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Equidistant Conic)\n");

			GMT_message (GMT, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scl (or <radius>/<lat>)|<width>  (Azimuthal Equidistant)\n");

			GMT_message (GMT, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scl (or <radius>/<lat>)|<width>  (Gnomonic)\n");

			GMT_message (GMT, "\t   -Jg|G<lon0>/<lat0>/<scl (or <radius>/<lat>)|<width>  (Orthographic)\n");

			GMT_message (GMT, "\t   -Jg|G[<lon0>/]<lat0>[/<horizon>|/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>]/<scl>|<width> (General Perspective)\n");

			GMT_message (GMT, "\t   -Jh|H[<lon0>/]<scl>|<width> (Hammer-Aitoff)\n");

			GMT_message (GMT, "\t   -Ji|I[<lon0>/]<scl>|<width> (Sinusoidal)\n");

			GMT_message (GMT, "\t   -Jj|J[<lon0>/]<scl>|<width> (Miller)\n");

			GMT_message (GMT, "\t   -Jkf|Kf[<lon0>/]<scl>|<width> (Eckert IV)\n");

			GMT_message (GMT, "\t   -Jks|Ks[<lon0>/]<scl>|<width> (Eckert VI)\n");

			GMT_message (GMT, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Lambert Conformal Conic)\n");

			GMT_message (GMT, "\t   -Jm|M[<lon0>/[<lat0>/]]<scl>|<width> (Mercator)\n");

			GMT_message (GMT, "\t   -Jn|N[<lon0>/]<scl>|<width> (Robinson projection)\n");

			GMT_message (GMT, "\t   -Jo|O (Oblique Mercator).  Specify one of three definitions:\n");
			GMT_message (GMT, "\t      -Jo|O[a]<lon0>/<lat0>/<azimuth>/<scl>|<width>\n");
			GMT_message (GMT, "\t      -Jo|O[b]<lon0>/<lat0>/<lon1>/<lat1>/<scl>|<width>\n");
			GMT_message (GMT, "\t      -Jo|Oc<lon0>/<lat0>/<lonp>/<latp>/<scl>|<width>\n");

			GMT_message (GMT, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scl>|<width> ((American) Polyconic)\n");

			GMT_message (GMT, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scl>|<width> (Equidistant Cylindrical)\n");

			GMT_message (GMT, "\t   -Jr|R[<lon0>/]<scl>|<width> (Winkel Tripel)\n");

			GMT_message (GMT, "\t   -Js|S<lon0>/<lat0>/[<horizon>/]<scl> (or <slat>/<scl> or <radius>/<lat>)|<width> (Stereographic)\n");

			GMT_message (GMT, "\t   -Jt|T<lon0>/[<lat0>/]<scl>|<width> (Transverse Mercator)\n");

			GMT_message (GMT, "\t   -Ju|U[<zone>/]<scl>|<width> (UTM)\n");

			GMT_message (GMT, "\t   -Jv|V<lon0>/<scl>|<width> (van der Grinten)\n");

			GMT_message (GMT, "\t   -Jw|W<lon0>/<scl>|<width> (Mollweide)\n");

			GMT_message (GMT, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scl>|<width> (Cylindrical Equal-area)\n");

			GMT_message (GMT, "\t   -Jp|P[a]<scl>|<width>[/<origin>][r|z] (Polar [azimuth] (theta,radius))\n");

			GMT_message (GMT, "\t   -Jx|X<x-scl>|<width>[d|l|p<power>|t|T][/<y-scl>|<height>[d|l|p<power>|t|T]] (Linear, log, and power projections)\n");
			GMT_message (GMT, "\t   (See psbasemap for more details on projection syntax)\n");
			break;

		case 'I':	/* Near-common option for grid increments */
			GMT_inc_syntax (GMT, 'I', false);
			break;

		case 'K':	/* Append-more-PostScript-later */

			GMT_message (GMT, "\t-K Allow for more plot code to be appended later.\n");
			break;

		case 'O':	/* Overlay plot */

			GMT_message (GMT, "\t-O Set Overlay plot mode, i.e., append to an existing plot.\n");
			break;

		case 'P':	/* Portrait or landscape */

			GMT_message (GMT, "\t-P Set Portrait page orientation [%s].\n", GMT_choice[GMT->current.setting.ps_orientation]);
			break;

		case 'S':	/* CarteSian Region option */

			GMT_message (GMT, "\t-R Specify the xmin/xmax/ymin/ymax coordinates of data region in user units.\n");
			GMT_message (GMT, "\t   Use [yyy[-mm[-dd]]]T[hh[:mm[:ss[.xxx]]]] format for time coordinates.\n");
			GMT_message (GMT, "\t   Or, give a gridfile to use its region (and increments, registration if applicable).\n");
			break;

		case 'G':	/* Geographic Region option */

			GMT_message (GMT, "\t-R Specify the west/east/south/north coordinates of map region.\n");
			GMT_message (GMT, "\t   Use decimal degrees or ddd[:mm[:ss]] degrees [ and minutes [and seconds]].\n");
			GMT_message (GMT, "\t   Use -R<unit>... for regions given in projected coordinates.\n");
			GMT_message (GMT, "\t   Append r if -R specifies the coordinates of the lower left and\n");
			GMT_message (GMT, "\t   upper right corners of a rectangular map area.\n");
			GMT_message (GMT, "\t   -Rg and -Rd are shorthands for -R0/360/-90/90 and -R-180/180/-90/90.\n");
			GMT_message (GMT, "\t   Or, give a gridfile to use its limits (and increments if applicable).\n");
			break;

		case 'R':	/* Generic [Default] Region option */

			GMT_message (GMT, "\t-R Specify the min/max coordinates of data region in user units.\n");
			GMT_message (GMT, "\t   Use dd:mm[:ss] for regions given in degrees, minutes [and seconds].\n");
			GMT_message (GMT, "\t   Use -R<unit>... for regions given in projected coordinates.\n");
			GMT_message (GMT, "\t   Use [yyy[-mm[-dd]]]T[hh[:mm[:ss[.xxx]]]] format for time axes.\n");
			GMT_message (GMT, "\t   Append r if -R specifies the coordinates of the lower left and\n");
			GMT_message (GMT, "\t   upper right corners of a rectangular area.\n");
			GMT_message (GMT, "\t   -Rg and -Rd are shorthands for -R0/360/-90/90 and -R-180/180/-90/90.\n");
			GMT_message (GMT, "\t   Or use -R<code><x0>/<y0>/<nx>/<ny> for origin and grid dimensions, where\n");
			GMT_message (GMT, "\t     <code> is a 2-char combo from [T|M|B][L|C|R] (top/middle/bottom/left/center/right)\n");
			GMT_message (GMT, "\t     and grid spacing must be specified via -I<dx>[/<dy>] (also see -r).\n");
			GMT_message (GMT, "\t   Or, give a gridfile to use its limits (and increments if applicable).\n");
			break;

		case 'z':	/* Region addition for 3-D */

			GMT_message (GMT, "\t   Append /zmin/zmax coordinates for the vertical domain limits.\n");
			break;

		case 'r':	/* Region option for 3-D */

			GMT_message (GMT, "\t-R Specify the xyz min/max coordinates of the plot window in user units.\n");
			GMT_message (GMT, "\t   Use dd:mm[:ss] for regions given in degrees, minutes [and seconds].\n");
			GMT_message (GMT, "\t   Append r if first 4 arguments to -R specify the longitudes/latitudes\n");
			GMT_message (GMT, "\t   of the lower left and upper right corners of a rectangular area.\n");
			GMT_message (GMT, "\t   Or, give a gridfile to use its limits (and increments if applicable).\n");
			break;

		case 'U':	/* Plot time mark and [optionally] command line */

			GMT_message (GMT, "\t-U Plot Unix System Time stamp [and optionally appended text].\n");
			GMT_message (GMT, "\t   You may also set the reference points and position of stamp\n");
			GMT_message (GMT, "\t   [%s/%g%c/%g%c].  Give -Uc to have the command line plotted [%s].\n",
			             GMT_just_string[GMT->current.setting.map_logo_justify], GMT->current.setting.map_logo_pos[GMT_X] * s, u,
			             GMT->current.setting.map_logo_pos[GMT_Y] * s, u, GMT_choice[GMT->current.setting.map_logo]);
			break;

		case 'V':	/* Verbose */

			GMT_message (GMT, "\t-V Change the verbosity level (currently %c).\n", V_code[GMT->current.setting.verbose]);
			GMT_message (GMT, "\t   Choose among 6 levels; each level adds more messages:\n");
			GMT_message (GMT, "\t     q - Quiet, not even fatal error messages.\n");
			GMT_message (GMT, "\t     n - Normal verbosity: only error messages.\n");
			GMT_message (GMT, "\t     c - Also produce compatibility warnings [Default when no -V is used].\n");
			GMT_message (GMT, "\t     v - Verbose progress messages [Default when -V is used].\n");
			GMT_message (GMT, "\t     l - Long verbose progress messages.\n");
			GMT_message (GMT, "\t     d - Debugging messages.\n");
			break;

		case 'X':
		case 'Y':	/* Reset plot origin option */

			GMT_message (GMT, "\t-X -Y Shift origin of plot to (<xshift>, <yshift>).\n");
			GMT_message (GMT, "\t   Prepend r for shift relative to current point (default), prepend a for temporary\n");
			GMT_message (GMT, "\t   adjustment of origin, prepend f to position relative to lower left corner of page,\n");
			GMT_message (GMT, "\t   prepend c for offset of center of plot to center of page.\n");
			GMT_message (GMT, "\t   For overlays (-O), the default setting is [r0], otherwise [f%g%c].\n",
			             GMT->current.setting.map_origin[GMT_Y] * s, u);
			break;

		case 'x':	/* Just linear -Jx|X allowed for this program */

			GMT_message (GMT, "\t-Jx|X for linear projection.  Scale in %s/units (or width in %s).\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit],
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t    Use / to specify separate x/y scaling.\n");
			GMT_message (GMT, "\t    If -JX is used then give axes lengths in %s rather than scales.\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			break;

		case 'y':	/* Number of threads (reassigned from -x in GMT_Option) */
			GMT_message (GMT, "\t-x Choose the number of processors used in multi-threading.\n");
			GMT_message (GMT, "\t   -x+a Use all available processors.\n");
			GMT_message (GMT, "\t   -xn  Use n processors (not more than max available off course).\n");
			GMT_message (GMT, "\t   -x-n Use (all - n) processors.\n");
			break;

		case 'Z':	/* Vertical scaling for 3-D plots */

			GMT_message (GMT, "\t   -JZ|z For z component of 3-D projections.  Same syntax as -JX|x, i.e.,\n");
			GMT_message (GMT, "\t   -Jz|Z<z-scl>|<height>[d|l|p<power>|t|T] (Linear, log, and power projections)\n");
			break;

		case 'a':	/* -a option for aspatial field substitution into data columns */

			GMT_message (GMT, "\t-a Give one or more comma-separated <col>=<name> associations.\n");
			break;

		case 'C':	/* -b binary option with input only */

			GMT_message (GMT, "\t-bi For binary input; [<n>]<type>[w][+L|B]; <type> = c|u|h|H|i|I|l|L|f|D.\n");
			break;

		case '0':	/* -bi/-bo addendum when input format is unknown */

			GMT_message (GMT, "\t    Prepend <n> for the number of columns for each <type>.\n");
			break;

		case '1':	/* -bi/-bo addendum when input format is unknown */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':

			GMT_message (GMT, "\t    Prepend <n> for the number of columns for each <type> in binary file(s) [%c].\n", options[k]);
			break;

		case 'D':	/* -b binary option with output only */

			GMT_message (GMT, "\t-bo For binary output; append <type>[w][+L|B]; <type> = c|u|h|H|i|I|l|L|f|D..\n");
			break;

		case 'c':	/* -c option to set number of plot copies option */

			GMT_message (GMT, "\t-c Specify the number of copies [%d].\n", GMT->PSL->init.copies);
			break;

		case 'd':	/* -d option to tell GMT the relationship between NaN and a nan-proxy for input/output */

			GMT_message (GMT, "\t-d On input, replace <nodata> with NaN; on output do the reverse.\n");
			break;

		case 'k':	/* -di option to tell GMT the relationship between NaN and a nan-proxy for input */

			GMT_message (GMT, "\t-di Replace any <nodata> in input data with NaN.\n");
			break;

		case 'l':	/* -do option to tell GMT the relationship between NaN and a nan-proxy for output */

			GMT_message (GMT, "\t-do Replace any NaNs in output data with <nodata>.\n");
			break;

		case 'f':	/* -f option to tell GMT which columns are time (and optionally geographical) */

			GMT_message (GMT, "\t-f Special formatting of input/output columns (time or geographical).\n");
			GMT_message (GMT, "\t   Specify i(nput) or o(utput) [Default is both input and output].\n");
			GMT_message (GMT, "\t   Give one or more columns (or column ranges) separated by commas.\n");
			GMT_message (GMT, "\t   Append T (Calendar format), t (time relative to TIME_EPOCH),\n");
			GMT_message (GMT, "\t   f (floating point), x (longitude), y (latitude) to each item.\n");
			GMT_message (GMT, "\t   -f[i|o]g means -f[i|o]0x,1y (geographic coordinates).\n");
			GMT_message (GMT, "\t   -fp[<unit>] means input is projected coordinates.\n");
			break;

		case 'g':	/* -g option to tell GMT to identify data gaps based on point separation */

			GMT_message (GMT, "\t-g Use data point separations to determine data gaps.\n");
			GMT_message (GMT, "\t   Append x|X or y|Y to flag data gaps in x or y coordinates,\n");
			GMT_message (GMT, "\t   respectively, and d|D for distance gaps.\n");
			GMT_message (GMT, "\t   Upper case means we first project the points.  Append <gap>[unit].\n");
			GMT_message (GMT, "\t   Geographic data: choose from %s [Default is meter (%c)].\n", GMT_LEN_UNITS2_DISPLAY, GMT_MAP_DIST_UNIT);
			GMT_message (GMT, "\t   For gaps based on mapped coordinates, choose from %s [%s].\n",
			             GMT_DIM_UNITS_DISPLAY, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			GMT_message (GMT, "\t   Note: For x|y with time data the unit is controlled by TIME_UNIT.\n");
			GMT_message (GMT, "\t   Repeat option to specify multiple criteria, and prepend +\n");
			GMT_message (GMT, "\t   to indicate that all the critera must be met [any].\n");
			break;

		case 'h':	/* Header */

			GMT_message (GMT, "\t-h[i][<n>][+c][+d][+r<remark>][+t<title>] Input/output file has [%d] Header record(s) [%s]\n",
			             GMT->current.setting.io_n_header_items, GMT_choice[GMT->current.setting.io_header[GMT_IN]]);
			GMT_message (GMT, "\t   Optionally, append i for input only and/or number of header records [0].\n");
			GMT_message (GMT, "\t     -hi turns off the writing of all headers on output.\n");
			GMT_message (GMT, "\t   Append +c to add header record with column information [none].\n");
			GMT_message (GMT, "\t   Append +d to delete headers before adding new ones [Default will append headers].\n");
			GMT_message (GMT, "\t   Append +r to add a <remark> comment to the output [none].\n");
			GMT_message (GMT, "\t   Append +t to add a <title> comment to the output [none].\n");
			GMT_message (GMT, "\t     (these strings may contain \\n to indicate line-breaks)\n");
			GMT_message (GMT, "\t   For binary files, <n> is considered to mean number of bytes.\n");
			break;

		case 'i':	/* -i option for input column order */

			GMT_message (GMT, "\t-i Sets alternate input column order [Default reads all columns in order].\n");
			break;

		case 'n':	/* -n option for grid resampling parameters in BCR */

			GMT_message (GMT, "\t-n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>] Determine the grid interpolation mode.\n");
			GMT_message (GMT, "\t   (b = B-spline, c = bicubic, l = bilinear, n = nearest-neighbor) [Default: bicubic].\n");
			GMT_message (GMT, "\t   Append +a switch off antialiasing (except for l) [Default: on].\n");
			GMT_message (GMT, "\t   Append +b<BC> to change boundary conditions.  <BC> can be either:\n");
			GMT_message (GMT, "\t   g for geographic boundary conditions, or one or both of\n");
			GMT_message (GMT, "\t   x for periodic boundary conditions on x,\n");
			GMT_message (GMT, "\t   y for periodic boundary conditions on y.\n");
			GMT_message (GMT, "\t   [Default: Natural conditions, unless grid is known to be geographic].\n");
			GMT_message (GMT, "\t   Append +c to clip interpolated grid to input z-min/max [Default may exceed limits].\n");
			GMT_message (GMT, "\t   Append +t<threshold> to change the minimum weight in vicinity of NaNs. A threshold of\n");
			GMT_message (GMT, "\t   1.0 requires all nodes involved in interpolation to be non-NaN; 0.5 will interpolate\n");
			GMT_message (GMT, "\t   about half way from a non-NaN to a NaN node [Default: 0.5].\n");
			break;

		case 'o':	/* -o option for output column order */

			GMT_message (GMT, "\t-o Set alternate output column order [Default writes all columns in order].\n");
			break;

		case 'p':	/* Enhanced pseudo-perspective 3-D plot settings */
		case 'E':	/* GMT4: For backward compatibility */
			if (GMT_compat_check (GMT, 4) || options[k] == 'p') {
				GMT_message (GMT, "\t-%c Select a 3-D pseudo perspective view.  Append the\n", options[k]);
				GMT_message (GMT, "\t   azimuth and elevation of the viewpoint [180/90].\n");
				GMT_message (GMT, "\t   When used with -Jz|Z, optionally add zlevel for frame, etc [bottom of z-axis]\n");
				GMT_message (GMT, "\t   Optionally, append +w<lon/lat[/z] to specify a fixed point\n");
				GMT_message (GMT, "\t   and +vx/y for its justification.  Just append + by itself\n");
				GMT_message (GMT, "\t   to select default values [region center and page center].\n");
			}
			break;

		case 's':	/* Output control for records where z are NaN */

			GMT_message (GMT, "\t-s Suppress output for records whose z-value (col = 2) equals NaN\n");
			GMT_message (GMT, "\t   [Default prints all records].\n");
			GMT_message (GMT, "\t   Append <cols> to examine other column(s) instead [2].\n");
			GMT_message (GMT, "\t   Append a to suppress records where any column equals NaN.\n");
			GMT_message (GMT, "\t   Append r to reverse the suppression (only output z = NaN records).\n");
			break;

		case 'F':	/* -r Pixel registration option  */

			GMT_message (GMT, "\t-r Set pixel registration [Default is grid registration].\n");
			break;

		case 't':	/* -t layer transparency option  */

			GMT_message (GMT, "\t-t Set the layer PDF transparency from 0-100 [Default is 0; opaque].\n");
			break;

		case ':':	/* lon/lat [x/y] or lat/lon [y/x] */

			GMT_message (GMT, "\t-: Swap 1st and 2nd column on input and/or output [%s/%s].\n",
			             GMT_choice[GMT->current.setting.io_lonlat_toggle[GMT_IN]], GMT_choice[GMT->current.setting.io_lonlat_toggle[GMT_OUT]]);
			break;

		case '.':	/* Trailer message */

			GMT_message (GMT, "\t-^ (or -) Print short synopsis message.\n");
			GMT_message (GMT, "\t-+ (or +) Print longer synopsis message.\n");
			GMT_message (GMT, "\t-? (or no arguments) Print this usage message\n");
			GMT_message (GMT, "\t(See gmt.conf man page for GMT default parameters).\n");
			break;

		case '<':	/* Table input */

			GMT_message (GMT, "\t<table> is one or more data files (in ASCII, binary, netCDF).\n");
			GMT_message (GMT, "\t   If no files are given, standard input is read.\n");
			break;

		default:
			break;
		}
	}
}

/*! GSHHG subset specification */
/*!
	\param GMT ...
	\param option ...
*/
void GMT_GSHHG_syntax (struct GMT_CTRL *GMT, char option) {
 	GMT_message (GMT, "\t-%c Place limits on coastline features from the GSHHG data base.\n", option);
	GMT_message (GMT, "\t   Features smaller than <min_area> (in km^2) or of levels (0-4) outside the min-max levels\n");
	GMT_message (GMT, "\t   will be skipped [0/4 (4 means lake inside island inside lake)].\n");
	GMT_message (GMT, "\t   Append +as to skip Antarctica (all data south of %dS) [use all].\n", abs(GSHHS_ANTARCTICA_LIMIT));
	GMT_message (GMT, "\t   Append +aS to skip anything BUT Antarctica (all data north of %dS) [use all].\n", abs(GSHHS_ANTARCTICA_LIMIT));
	GMT_message (GMT, "\t   Append +ag to use shelf ice grounding line for Antarctica coastline.\n");
	GMT_message (GMT, "\t   Append +ai to use ice/water front for Antarctica coastline [Default].\n");
	GMT_message (GMT, "\t   Append +r to only get riverlakes from level 2, or +l to only get lakes [both].\n");
	GMT_message (GMT, "\t   Append +p<percent> to exclude features whose size is < <percent>%% of the full-resolution feature [use all].\n");
}

/*! Contour/line specifications in *contour and psxy[z] */
/*!
	\param GMT ...
	\param indent The number of spaces to indent after the TAB
	\param kind  kind = 0 for *contour and 1 for psxy[z]
*/
void GMT_label_syntax (struct GMT_CTRL *GMT, unsigned int indent, unsigned int kind) {
	unsigned int i;
	char pad[16], *type[2] = {"Contour", "Line"};

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	GMT_message (GMT, "%s +a<angle> will place all annotations at a fixed angle.\n", pad);
	GMT_message (GMT, "%s Or, specify +an (line-normal) or +ap (line-parallel) [Default].\n", pad);
	if (kind == 0) {
		GMT_message (GMT, "%s   For +ap, you may optionally append u for up-hill\n", pad);
		GMT_message (GMT, "%s   and d for down-hill cartographic annotations.\n", pad);
	}
	GMT_message (GMT, "%s +c<dx>[/<dy>] sets clearance between label and text box [15%%].\n", pad);
	GMT_message (GMT, "%s +d turns on debug which draws helper points and lines.\n", pad);
	GMT_message (GMT, "%s +e delays the plotting of the text as text clipping is set instead.\n", pad);
	GMT_message (GMT, "%s +f sets specified label font [Default is %s].\n", pad, GMT_putfont (GMT, GMT->current.setting.font_annot[0]));
	GMT_message (GMT, "%s +g[<color>] paints text box [transparent]; append color [white].\n", pad);
	GMT_message (GMT, "%s +j<just> sets label justification [Default is CM].\n", pad);
	if (kind == 1) {
		GMT_message (GMT, "%s +l<text> Use text as label (quote text if containing spaces).\n", pad);
		GMT_message (GMT, "%s +L<d|D|f|h|n|N|x>[<unit>] Sets label according to given flag:\n", pad);
		GMT_message (GMT, "%s   d Cartesian plot distance; append a desired unit from %s.\n", pad, GMT_DIM_UNITS_DISPLAY);
		GMT_message (GMT, "%s   D Map distance; append a desired unit from %s.\n", pad, GMT_LEN_UNITS_DISPLAY);
		GMT_message (GMT, "%s   f Label is last column of given label location file.\n", pad);
		GMT_message (GMT, "%s   h Use segment header labels (via -Lstring).\n", pad);
		GMT_message (GMT, "%s   n Use the current segment number (starting at 0).\n", pad);
		GMT_message (GMT, "%s   N Use current file number / segment number (starting at 0/0).\n", pad);
		GMT_message (GMT, "%s   x Like h, but us headers in file with crossing lines instead.\n", pad);
	}
	GMT_message (GMT, "%s +n<dx>[/<dy>] to nudge label along line (+N for along x/y axis); ignored with +v.\n", pad);
	GMT_message (GMT, "%s +o to use rounded rectangular text box [Default is rectangular].\n", pad);
	GMT_message (GMT, "%s +p[<pen>] draw outline of textbox  [Default is no outline].\n", pad);
	GMT_message (GMT, "%s   Optionally append a pen [Default is default pen].\n", pad);
	GMT_message (GMT, "%s +r<rmin> skips labels where radius of curvature < <rmin> [0].\n", pad);
	GMT_message (GMT, "%s +t[<file>] saves (x y label) to <file> [%s_labels.txt].\n", pad, type[kind%2]);
	GMT_message (GMT, "%s   use +T to save (x y angle label) instead\n", pad);
	GMT_message (GMT, "%s +u<unit> to append unit to all labels.\n", pad);
	if (kind == 0) GMT_message (GMT, "%s  If z is appended we use the z-unit from the grdfile [no unit].\n", pad);
	GMT_message (GMT, "%s +v for placing curved text along path [Default is straight].\n", pad);
	GMT_message (GMT, "%s +w sets how many (x,y) points to use for angle calculation [auto].\n", pad);
	if (kind == 1) {
		GMT_message (GMT, "%s +x[first,last] adds <first> and <last> to these two labels [,'].\n", pad);
		GMT_message (GMT, "%s   This modifier is only allowed if -SqN2 is used.\n", pad);
	}
	GMT_message (GMT, "%s +=<prefix> to give all labels a prefix.\n", pad);
}

/*! Contour/line label placement specifications in *contour and psxy[z] */
/*!
	\param GMT ...
	\param indent The number of spaces to indent after the TAB
	\param kind  kind = 0 for *contour and 1 for psxy[z]
*/
void GMT_cont_syntax (struct GMT_CTRL *GMT, unsigned int indent, unsigned int kind) {
	unsigned int i;
	double gap;
	char pad[16];
	char *type[2] = {"contour", "quoted line"};

	gap = 4.0 * GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	GMT_message (GMT, "%sd<dist>[%s] or D<dist>[%s]  [Default is d%g%c].\n",
	             pad, GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY, gap, GMT->session.unit_name[GMT->current.setting.proj_length_unit][0]);
	GMT_message (GMT, "%s   d: Give distance between labels with specified map unit in %s.\n", pad, GMT_DIM_UNITS_DISPLAY);
	GMT_message (GMT, "%s   D: Specify geographic distance between labels in %s,\n", pad, GMT_LEN_UNITS_DISPLAY);
	GMT_message (GMT, "%s   The first label appears at <frac>*<dist>; change by appending /<frac> [0.25].\n", pad);
	GMT_message (GMT, "%sf<file.d> reads file <file.d> and places labels at locations\n", pad);
	GMT_message (GMT, "%s   that match individual points along the %ss.\n", pad, type[kind]);
	GMT_message (GMT, "%sl|L<line1>[,<line2>,...] Give start and stop coordinates for\n", pad);
	GMT_message (GMT, "%s   straight line segments.  Labels will be placed where these\n", pad);
	GMT_message (GMT, "%s   lines intersect %ss.  The format of each <line>\n", pad, type[kind]);
	GMT_message (GMT, "%s   is <start>/<stop>, where <start> or <stop> = <lon/lat> or a\n", pad);
	GMT_message (GMT, "%s   2-character XY key that uses the \"pstext\"-style justification\n", pad);
	GMT_message (GMT, "%s   format to specify a point on the map as [LCR][BMT].\n", pad);
	if (kind == 0) {
		GMT_message (GMT, "%s   In addition, you can use Z-, Z+ to mean the global\n", pad);
		GMT_message (GMT, "%s   minimum and maximum locations in the grid.\n", pad);
	}
	GMT_message (GMT, "%s   L Let point pairs define great circles [Straight lines].\n", pad);
	GMT_message (GMT, "%sn|N<n_label> sets number of equidistant labels per %s.\n", pad, type[kind]);
	GMT_message (GMT, "%s   N: Starts labeling exactly at the start of %s\n", pad, type[kind]);
	GMT_message (GMT, "%s     [Default centers the labels on the %s].\n", pad, type[kind]);
	GMT_message (GMT, "%s   N-1 places a single label at start of the %s, while\n", pad, type[kind]);
	GMT_message (GMT, "%s   N+1 places a single label at the end of the %s.\n", pad, type[kind]);
	GMT_message (GMT, "%s   Append /<min_dist> to enforce a minimum spacing between\n", pad);
	GMT_message (GMT, "%s   consecutive labels [0]\n", pad);
	if (kind == 1) {
		GMT_message (GMT, "%ss|S<n_label> sets number of equidistant labels per segmentized %s.\n", pad, type[kind]);
		GMT_message (GMT, "%s   Same as n|N but splits input lines to series of 2-point segments first.\n", pad);
	}
	GMT_message (GMT, "%sx|X<xfile.d> reads the multi-segment file <xfile.d> and places\n", pad);
	GMT_message (GMT, "%s   labels at intersections between %ss and lines in\n", pad, type[kind]);
	GMT_message (GMT, "%s   <xfile.d>.  Use X to resample the lines first.\n", pad);
	GMT_message (GMT, "%s   For all options, append +r<radius>[unit] to specify minimum\n", pad);
	GMT_message (GMT, "%s   radial separation between labels [0]\n", pad);
}

/*! Widely used in most programs that need grid increments to be set */
/*!
	\param GMT ...
	\param option ...
	\param error ...
*/
void GMT_inc_syntax (struct GMT_CTRL *GMT, char option, bool error)
{
	if (error) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c Specify increment(s) and optionally append units or flags.\n", option);
	GMT_message (GMT, "\t   Full syntax is <xinc>[%s|+][=][/<yinc>[%s|+][=]]\n", GMT_LEN_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY);
	GMT_message (GMT, "\t   For geographic regions in degrees you can optionally append units from this list:\n");
	GMT_message (GMT, "\t   (d)egree [Default], (m)inute, (s)econd, m(e)ter, (f)oot, (k)ilometer, (M)ile, (n)autical mile, s(u)rvey foot.\n");
	GMT_message (GMT, "\t   Append = to adjust the region to fit increments [Adjust increment to fit domain].\n");
	GMT_message (GMT, "\t   Alternatively, specify number of nodes by appending +. Then, the increments\n");
	GMT_message (GMT, "\t   are calculated from the given domain and node-registration settings\n");
	GMT_message (GMT, "\t   (see Appendix B for details).  Note: If -R<grdfile> was used then\n");
	GMT_message (GMT, "\t   both -R and -I have been set; use -I to override those values.\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_fill_syntax (struct GMT_CTRL *GMT, char option, char *string)
{
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c<fill> %s Specify <fill> as one of:\n", option, string);
	GMT_message (GMT, "\t   1) <gray> or <red>/<green>/<blue>, all in the range 0-255;\n");
	GMT_message (GMT, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	GMT_message (GMT, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	GMT_message (GMT, "\t   4) any valid color name;\n");
	GMT_message (GMT, "\t   5) P|p<dpi>/<pattern>[:F<color>B<color>], with <dpi> of the pattern.\n");
	GMT_message (GMT, "\t      Give <pattern> number from 1-90 or a filename, optionally add\n");
	GMT_message (GMT, "\t      replacement fore- or background colors (set - for transparency).\n");
	GMT_message (GMT, "\t   For PDF fill transparency, append @<transparency> in the range 0-100 [0 = opaque].\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_pen_syntax (struct GMT_CTRL *GMT, char option, char *string)
{
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c ", option);
	GMT_message (GMT, string, GMT_putpen (GMT, GMT->current.setting.map_default_pen));
	GMT_message (GMT, "\n\t   <pen> is a comma-separated list of three optional items in the order:\n");
	GMT_message (GMT, "\t       <width>[%s], <color>, and <style>[%s].\n", GMT_DIM_UNITS, GMT_DIM_UNITS);
	GMT_message (GMT, "\t   <width> >= 0.0 sets pen width (default units are points); alternatively a pen\n");
	GMT_message (GMT, "\t       name: Choose among faint, default, or [thin|thick|fat][er|est], or obese.\n");
	GMT_message (GMT, "\t   <color> = (1) <gray> or <red>/<green>/<blue>, all in range 0-255,\n");
	GMT_message (GMT, "\t             (2) <c>/<m>/<y>/<k> in 0-100%% range,\n");
	GMT_message (GMT, "\t             (3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1,\n");
	GMT_message (GMT, "\t             (4) any valid color name.\n");
	GMT_message (GMT, "\t   <style> = (1) pattern of dashes (-) and dots (.), scaled by <width>.\n");
	GMT_message (GMT, "\t             (2) \"dashed\", \"dotted\", or \"solid\".\n");
	GMT_message (GMT, "\t             (3) <pattern>:<offset>; <pattern> holds lengths (default unit points)\n");
	GMT_message (GMT, "\t                 of any number of lines and gaps separated by underscores.\n");
	GMT_message (GMT, "\t                 <offset> shifts elements from start of the line [0].\n");
	GMT_message (GMT, "\t   For PDF stroke transparency, append @<transparency> in the range 0-100%% [0 = opaque].\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_rgb_syntax (struct GMT_CTRL *GMT, char option, char *string)
{
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c<color> %s Specify <color> as one of:\n", option, string);
	GMT_message (GMT, "\t   1) <gray> or <red>/<green>/<blue>, all in range 0-255;\n");
	GMT_message (GMT, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	GMT_message (GMT, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	GMT_message (GMT, "\t   4) any valid color name.\n");
	GMT_message (GMT, "\t   For PDF fill transparency, append @<transparency> in the range 0-100%% [0 = opaque].\n");
}


void GMT_refpoint_syntax (struct GMT_CTRL *GMT, char option, char *string, unsigned int kind, unsigned int part)
{	/* For -Dgjnx */
	char *type[GMT_ANCHOR_NTYPES] = {"logo", "image", "legend", "color-bar", "insert", "map scale", "map rose"}, *tab[2] = {"", "     "};
	unsigned int shift = (part & 4) ? 1 : 0;
	if (part & 1) {	/* Here string is message, or NULL */
		if (string) GMT_message (GMT, "\t-%c %s\n", option, string);
		GMT_message (GMT, "\t   %sPositioning is specified via one of four coordinate systems:\n", tab[shift]);
		GMT_message (GMT, "\t   %s  Use -Dg to specify <refpoint> with map coordinates.\n", tab[shift]);
		GMT_message (GMT, "\t   %s  Use -Dj to specify <refpoint> with 2-char justification code (LB, CM, etc).\n", tab[shift]);
		GMT_message (GMT, "\t   %s  Use -Dn to specify <refpoint> with normalized coordinates in 0-1 range.\n", tab[shift]);
		GMT_message (GMT, "\t   %s  Use -Dx to specify <refpoint> with plot coordinates.\n", tab[shift]);
		GMT_message (GMT, "\t   %sAll except -Dx require the -R and -J options to be set.\n", tab[shift]);
	}
	/* May need to place other things in the middle */
	if (part & 2) {	/* Here string is justification unless part == 3 */
		char *just = (part == 3) ? "LB" : string;
		GMT_message (GMT, "\t   %sAppend 2-char +j<justify> code to associate that point on the %s with <refpoint> [%s].\n", tab[shift], type[kind], just);
		GMT_message (GMT, "\t   %sNote for -Dj: If +j<justify> is not given then it inherits the inverse of <refpoint>.\n", tab[shift]);
		GMT_message (GMT, "\t   %sOptionally, append +o<dx>[/<dy>] to offset %s from refpoint in direction implied by <justify> [0/0].\n", tab[shift], type[kind]);
	}
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_mapinsert_syntax (struct GMT_CTRL *GMT, char option, char *string)
{	/* Only called in psbasemap.c for now */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c %s\n", option, string);
	GMT_message (GMT, "\t     Specify the map insert region using one of three specifications:\n");
	GMT_message (GMT, "\t     a) Give <west>/<east>/<south>/<north> of geographic rectangle bounded by meridians and parallels.\n");
	GMT_message (GMT, "\t        Append r if coordinates are the lower left and upper right corners of a rectangular area.\n");
	GMT_message (GMT, "\t     b) Give <u><xmin>/<xmax>/<ymin>/<ymax> of bounding rectangle in projected coordinates.\n");
	GMT_message (GMT, "\t     c) Set reference point and dimensions of the insert:\n");
	GMT_refpoint_syntax (GMT, 'D', NULL, GMT_ANCHOR_INSERT, 5);
	GMT_message (GMT, "\t        Append width[<u>]/height[u] of bounding rectangle (<u> is unit).\n");
	GMT_refpoint_syntax (GMT, 'D', "CM", GMT_ANCHOR_INSERT, 6);
	GMT_message (GMT, "\t   Set panel attributes separately via -F option:\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_mapscale_syntax (struct GMT_CTRL *GMT, char option, char *string)
{	/* Used in psbasemap and pscoast */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c %s\n", option, string);
	GMT_message (GMT, "\t   Set the reference point and length of the scale:\n");
	GMT_refpoint_syntax (GMT, 'L', NULL, GMT_ANCHOR_MAPSCALE, 5);
	GMT_refpoint_syntax (GMT, 'L', "CT", GMT_ANCHOR_MAPSCALE, 2);
	GMT_message (GMT, "\t   Use +c<slat> (with central longitude) or +c<slon>/<slat> to specify scale origin.\n");
	GMT_message (GMT, "\t   Set scale length with +w<length> and append a unit from %s [km].  Use -%cf to draw a \"fancy\" scale [Default is plain].\n", GMT_LEN_UNITS2_DISPLAY, option);
	GMT_message (GMT, "\t   Several modifiers are optional:\n");
	GMT_message (GMT, "\t   Add +f to draw a \"fancy\" scale [Default is plain].\n");
	GMT_message (GMT, "\t   By default, the scale label equals the distance unit name and is placed on top [+at].  Use the +l<label>\n");
	GMT_message (GMT, "\t   and +a<align> mechanisms to specify another label and placement (t,b,l,r).  For the fancy scale,\n");
	GMT_message (GMT, "\t   +u appends units to annotations while for plain scale it uses unit abbreviation instead of name as label.\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_maprose_syntax (struct GMT_CTRL *GMT, char option, char *string)
{	/* Used in psbasemap and pscoast */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c %s\n", option, string);
	GMT_message (GMT, "\t   Choose between a directional (-Td) or magnetic (-Tm) rose.\n");
	GMT_message (GMT, "\t   Both share most modifers for locating and sizing the rose.\n");
	GMT_message (GMT, "\t   First set the reference point (center of rose) and size of the rose:\n");
	GMT_refpoint_syntax (GMT, 'T', NULL, GMT_ANCHOR_MAPROSE, 5);
	GMT_refpoint_syntax (GMT, 'T', "CM", GMT_ANCHOR_MAPROSE, 2);
	GMT_message (GMT, "\t   Set the diameter of the rose with modifier +w<width>.\n");
	GMT_message (GMT, "\t   Several modifiers are optional:\n");
	GMT_message (GMT, "\t   Add labels with +l, which places the letters W, E, S, N at the cardinal points.\n");
	GMT_message (GMT, "\t     Optionally, append comma-separated west, east, south, north labels instead.\n");
	GMT_message (GMT, "\t   Append +i<pint>[/<sint>] to override default primary and secondary annotation/tick interval(s) [30/5/1].\n");
	GMT_message (GMT, "\t   Directional rose: Add +f to draws a \"fancy\" rose [Default is plain].\n");
	GMT_message (GMT, "\t     Optionally, add <kind> of fancy rose: 1 draws E-W, N-S directions [Default],\n");
	GMT_message (GMT, "\t     2 adds NW-SE and NE-SW, while 3 adds WNW-ESE, NNW-SSE, NNE-SSW, and ENE-WSW directions.\n");
	GMT_message (GMT, "\t   Magnetic compass rose:  Optional add +d<dec>[/<dlabel>], where <dec> is the\n");
	GMT_message (GMT, "\t     magnetic declination and <dlabel> is an optional label for the magnetic compass needle.\n");
	GMT_message (GMT, "\t     If +d does not include <dlabel> we default to \"delta = <declination>\".\n");
	GMT_message (GMT, "\t   If the North label = \'*\' then a north star is plotted instead of the label.\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_mappanel_syntax (struct GMT_CTRL *GMT, char option, char *string, unsigned int kind)
{	/* Called by gmtlogo, psimage, pslegend, psscale */
	char *type[4] = {"logo", "image", "legend", "scale"};
	assert (kind < 4);
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c %s\n", option, string);
	GMT_message (GMT, "\t   Without further options: draw border around the %s panel (using MAP_FRAME_PEN)\n", type[kind]);
	GMT_message (GMT, "\t      [Default is no border].\n");
	GMT_message (GMT, "\t   Append +c<clearance> where <clearance> is <gap>, <xgap/ygap>, or <lgap/rgap/bgap/tgap> [%gp].\n", GMT_FRAME_CLEARANCE);
	GMT_message (GMT, "\t     Note: For a map insert the default clearance is zero.\n");
#ifdef DEBUG
	GMT_message (GMT, "\t   Append +d to draw guide lines for debugging.\n");
#endif
	GMT_message (GMT, "\t   Append +g<fill> to set the fill for the %s panel [Default is no fill].\n", type[kind]);
	GMT_message (GMT, "\t   Append +i[[<gap>/]<pen>] to add a secondary inner frame boundary [Default gap is %gp].\n", GMT_FRAME_GAP);
	GMT_message (GMT, "\t   Append +p[<pen>] to draw the border and optionally change the border pen [%s].\n",
		GMT_putpen (GMT, GMT->current.setting.map_frame_pen));
	GMT_message (GMT, "\t   Append +r[<radius>] to plot rounded rectangles instead [Default radius is %gp].\n", GMT_FRAME_RADIUS);
	GMT_message (GMT, "\t   Append +s[<dx>/<dy>/]<fill> to plot a shadow behind the %s panel [Default offset is %gp/%g].\n", type[kind], GMT_FRAME_CLEARANCE, -GMT_FRAME_CLEARANCE);
}
/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void GMT_dist_syntax (struct GMT_CTRL *GMT, char option, char *string)
{	/* Used by many modules */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (GMT, "\t-%c %s\n", option, string);
	GMT_message (GMT, "\t   Append e (meter), f (foot), k (km), M (mile), n (nautical mile), u (survey foot)\n");
	GMT_message (GMT, "\t   d (arc degree), m (arc minute), or s (arc second) [%c].\n", GMT_MAP_DIST_UNIT);
	GMT_message (GMT, "\t   Prepend - for (fast) flat Earth or + for (slow) geodesic calculations.\n");
	GMT_message (GMT, "\t   [Default is spherical great-circle calculations].\n");
}

/*! Use mode to control which options are displayed */
void GMT_vector_syntax (struct GMT_CTRL *GMT, unsigned int mode)
{
	/* Mode is composed of bit-flags to control which lines are printed.
	 * Items without if-test are common to all vectors.
	 * 1	= Accepts +j (not mathangle)
	 * 2	= Accepts +s (not mathangle)
	 * 4	= Accepts +p (not mathangle)
	 * 8	= Accepts +g (not mathangle)
	 * 16	= Accepts +z (not mathangle, geovector)
	 */
	GMT_message (GMT, "\t   Append length of vector head, with optional modifiers:\n");
	GMT_message (GMT, "\t   [Left and right are defined by looking from start to end of vector]\n");
	GMT_message (GMT, "\t     +a<angle> to set angle of the vector head apex [30]\n");
	GMT_message (GMT, "\t     +b to place a vector head at the beginning of the vector [none].\n");
	GMT_message (GMT, "\t       Append t for terminal, c for circle, or a for arrow [Default].\n");
	GMT_message (GMT, "\t       For arrow, append l|r to only draw left or right side of this head [both sides].\n");
	GMT_message (GMT, "\t     +e to place a vector head at the end of the vector [none].\n");
	GMT_message (GMT, "\t       Append t for terminal, c for circle, or a for arrow [Default].\n");
	GMT_message (GMT, "\t       For arrow, append l|r to only draw left or right side of this head [both sides].\n");
	if (mode & 8) GMT_message (GMT, "\t     +g<fill> to set head fill or use - to turn off fill [default fill].\n");
	if (mode & 1) GMT_message (GMT, "\t     +j<just> to justify vector at (b)eginning [default], (e)nd, or (c)enter.\n");
	GMT_message (GMT, "\t     +l to only draw left side of all specified vector heads [both sides].\n");
	GMT_message (GMT, "\t     +n<norm> to shrink attributes if vector length < <norm> [none].\n");
	GMT_message (GMT, "\t     +o[<plon/plat>] sets pole [north pole] for great or small circles; only give length via input.\n");
	if (mode & 4) GMT_message (GMT, "\t     +p[-][<pen>] to set pen attributes, prepend - to turn off head outlines [default pen and outline].\n");
	GMT_message (GMT, "\t     +q if start and stop opening angle is given instead of (azimuth,length) on input.\n");
	GMT_message (GMT, "\t     +r to only draw right side of all specified vector heads [both sides].\n");
	if (mode & 2) GMT_message (GMT, "\t     +s if (x,y) coordinates of tip is given instead of (azimuth,length) on input.\n");
	if (mode & 16) GMT_message (GMT, "\t     +z if (dx,dy) vector components are given instead of (azimuth,length) on input.\n");
	if (mode & 16) GMT_message (GMT, "\t       Append <scale>[unit] to convert components to length in given unit.\n");
}

/*! For programs that can read *.img grids */
void GMT_img_syntax (struct GMT_CTRL *GMT)
{
	GMT_message (GMT, "\t      Give filename and append comma-separated scale, mode, and optionally max latitude.\n");
	GMT_message (GMT, "\t      The scale (typically 0.1 or 1) is used to multiply after read; give mode as follows:\n");
	GMT_message (GMT, "\t        0 = img file with no constraint code, interpolate to get data at track.\n");
	GMT_message (GMT, "\t        1 = img file with constraints coded, interpolate to get data at track.\n");
	GMT_message (GMT, "\t        2 = img file with constraints coded, gets data only at constrained points, NaN elsewhere.\n");
	GMT_message (GMT, "\t        3 = img file with constraints coded, gets 1 at constraints, 0 elsewhere.\n");
	GMT_message (GMT, "\t        For mode 2|3 you may want to consider the -n+t<threshold> setting.\n");
}

/*! . */
void GMT_syntax (struct GMT_CTRL *GMT, char option)
{
	/* The function print to stderr the syntax for the option indicated by
	 * the variable <option>.  Only the common parameter options are covered
	 */

	char *u = GMT->session.unit_name[GMT->current.setting.proj_length_unit];

	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);

	switch (option) {

		case 'B':	/* Tickmark option */
			GMT_message (GMT, "\t-B[p|s][x|y|z]<intervals>[+l<label>][+p<prefix>][+u<unit>] -B[<axes>][+b][+g<fill>][+o<lon>/<lat>][+t<title>] OR\n");
			GMT_message (GMT, "\t-B[p|s][x|y|z][a|f|g]<tick>[m][l|p] -B[p|s][x|y|z][+l<label>][+p<prefix>][+u<unit>] -B[<axes>][+b][+g<fill>][+o<lon>/<lat>][+t<title>]\n");
			break;

		case 'J':	/* Map projection option */
			switch (GMT->current.proj.projection) {
				case GMT_LAMB_AZ_EQ:
					GMT_message (GMT, "\t-Ja<lon0>/<lat0>[/<horizon>]/<scale> OR -JA<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (GMT, "\t  <horizon> is distance from center to perimeter (<= 180, default 90)\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ALBERS:
					GMT_message (GMT, "\t-Jb<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JB<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CASSINI:
					GMT_message (GMT, "\t-Jc<lon0>/<lat0><scale> OR -JC<lon0>/<lat0><width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree ,or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_STEREO:
					GMT_message (GMT, "\t-Jcyl_stere/[<lon0>/[<lat0>/]]<scale> OR -JCyl_stere/[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECONIC:
					GMT_message (GMT, "\t-Jd<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JD<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_AZ_EQDIST:
					GMT_message (GMT, "\t-Je<lon0>/<lat0>[/<horizon>]/<scale> OR -JE<lon0>/<lat0>[/<horizon>/<width>\n");
					GMT_message (GMT, "\t  <horizon> is distance from center to perimeter (<= 180, default 180)\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GNOMONIC:
					GMT_message (GMT, "\t-Jf<lon0>/<lat0>[/<horizon>]/<scale> OR -JF<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (GMT, "\t  <horizon> is distance from center to perimeter (< 90, default 60)\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ORTHO:
					GMT_message (GMT, "\t-Jg<lon0>/<lat0>[/<horizon>]/<scale> OR -JG<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (GMT, "\t  <horizon> is distance from center to perimeter (<= 90, default 90)\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GENPER:
					GMT_message (GMT, "\t-Jg<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale> OR\n\t-JG<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_HAMMER:
					GMT_message (GMT, "\t-Jh[<lon0>/]<scale> OR -JH[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_SINUSOIDAL:
					GMT_message (GMT, "\t-Ji[<lon0>/]<scale> OR -JI[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MILLER:
					GMT_message (GMT, "\t-Jj[<lon0>/]<scale> OR -JJ[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT4:
					GMT_message (GMT, "\t-Jkf[<lon0>/]<scale> OR -JKf[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT6:
					GMT_message (GMT, "\t-Jk[s][<lon0>/]<scale> OR -JK[s][<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_LAMBERT:
					GMT_message (GMT, "\t-Jl<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JL<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MERCATOR:
					GMT_message (GMT, "\t-Jm[<lon0>/[<lat0>/]]<scale> OR -JM[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ROBINSON:
					GMT_message (GMT, "\t-Jn[<lon0>/]<scale> OR -JN[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_OBLIQUE_MERC:
					GMT_message (GMT, "\t-Jo[a]<lon0>/<lat0>/<azimuth>/<scale> OR -JO[a]<lon0>/<lat0>/<azimuth>/<width>\n");
					GMT_message (GMT, "\t-Jo[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<scale> OR -JO[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<width>\n");
					GMT_message (GMT, "\t-Joc<lon0>/<lat0>/<lonp>/<latp>/<scale> OR -JOc<lon0>/<lat0>/<lonp>/<latp>/<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/oblique degree, or use <width> in %s\n", u, u);
					break;
				case GMT_WINKEL:
					GMT_message (GMT, "\t-Jr[<lon0>/]<scale> OR -JR[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLYCONIC:
					GMT_message (GMT, "\t-Jpoly/[<lon0>/[<lat0>/]]<scale> OR -JPoly/[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQDIST:
					GMT_message (GMT, "\t-Jq[<lon0>/[<lat0>/]]<scale> OR -JQ[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_STEREO:
					GMT_message (GMT, "\t-Js<lon0>/<lat0>[/<horizon>]/<scale> OR -JS<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (GMT, "\t  <horizon> is distance from center to perimeter (< 180, default 90)\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx>, <lat>/<1:xxxx>, or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_TM:
					GMT_message (GMT, "\t-Jt<lon0>/[<lat0>/]<scale> OR -JT<lon0>/[<lat0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_UTM:
					GMT_message (GMT, "\t-Ju<zone>/<scale> OR -JU<zone>/<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					GMT_message (GMT, "\t  <zone is A, B, 1-60[w/ optional C-X except I, O], Y, Z\n");
					break;
				case GMT_VANGRINTEN:
					GMT_message (GMT, "\t-Jv<lon0>/<scale> OR -JV[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MOLLWEIDE:
					GMT_message (GMT, "\t-Jw[<lon0>/]<scale> OR -JW[<lon0>/]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQ:
					GMT_message (GMT, "\t-Jy[<lon0>/[<lat0>/]]<scale> OR -JY[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLAR:
					GMT_message (GMT, "\t-Jp[a]<scale>[/<origin>][r] OR -JP[a]<width>[/<origin>][r]\n");
					GMT_message (GMT, "\t  <scale> is %s/units, or use <width> in %s\n", u, u);
					GMT_message (GMT, "\t  Optionally, prepend a for azimuths, append theta as origin [0],\n");
					GMT_message (GMT, "\t  or append r to reverse radial coordinates.\n");
				case GMT_LINEAR:
					GMT_message (GMT, "\t-Jx<x-scale>|<width>[d|l|p<power>|t|T][/<y-scale>|<height>[d|l|p<power>|t|T]], scale in %s/units\n", u);
					GMT_message (GMT, "\t-Jz<z-scale>[l|p<power>], scale in %s/units\n", u);
					GMT_message (GMT, "\tUse / to specify separate x/y scaling (e.g., -Jx0.5/0.3.).  Not allowed with 1:xxxxx\n");
					GMT_message (GMT, "\tUse -JX (and/or -JZ) to give axes lengths rather than scales\n");
					break;
				default:
					GMT_message (GMT, "\tProjection not recognized!\n");
					break;
			}
			break;

		case 'K':
			GMT_message (GMT, "\t-%c More PS matter will follow\n", option);
			break;

		case 'O':
			GMT_message (GMT, "\t-%c This is a PS overlay\n", option);
			break;

		case 'P':
			GMT_message (GMT, "\t-%c Turn on portrait mode\n", option);
			break;

		case 'R':	/* Region option */
			GMT_message (GMT, "\t-R<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>]\n");
			GMT_message (GMT, "\t  Append r if giving lower left and upper right coordinates\n");
			GMT_message (GMT, "\t-Rg or -Rd for global domain\n");
			GMT_message (GMT, "\t-R<grdfile> to take the domain from a grid file\n");
			break;

		case 'U':	/* Set time stamp option */
			GMT_message (GMT, "\t%s\n", GMT_U_OPT);
			GMT_message (GMT, "\t   Plot the time stamp and optional command line or text.\n");
			break;

		case 'X':
			GMT_message (GMT, "\t%s\n", GMT_X_OPT);
			GMT_message (GMT, "\tPrepend a for temporary adjustment, c for center of page reference,\n");
			GMT_message (GMT, "\tf for lower left corner of page reference, r (or none) for relative to\n");
			GMT_message (GMT, "\tcurrent position; u is unit (c, i, p).\n");
			break;

		case 'Y':
			GMT_message (GMT, "\t%s\n", GMT_Y_OPT);
			GMT_message (GMT, "\tPrepend a for temporary adjustment, c for center of page reference,\n");
			GMT_message (GMT, "\tf for lower left corner of page reference, r (or none) for relative to\n");
			GMT_message (GMT, "\tcurrent position; u is unit (c, i, p).\n");
			break;

		case 'Z':
			if (GMT_compat_check (GMT, 4)) {
				GMT_message (GMT, "\t-Z<zlevel> set zlevel of basemap\n");
			}
			break;

		case 'a':	/* -a option for aspatial field substitution into data columns */

			GMT_message (GMT, "\t%s\n", GMT_a_OPT);
			GMT_message (GMT, "\t  Specify the aspatial field information.\n");
			break;

		case 'b':	/* Binary i/o option  */
			GMT_message (GMT, "\t%s\n", GMT_b_OPT);
			GMT_message (GMT, "\t   Binary data, add i for input, o for output [Default is both].\n");
			GMT_message (GMT, "\t   Here, t is c|u|h|H|i|I|l|L|f|d [Default is d (double)].\n");
			GMT_message (GMT, "\t   Prepend the number of data columns.\n");
			GMT_message (GMT, "\t   Append w to byte swap an item; append +L|B to fix endianness of file.\n");
			break;

		case 'c':	/* Set number of plot copies option */
			GMT_message (GMT, "\t%s\n", GMT_c_OPT);
			GMT_message (GMT, "\t   Set the number of PS copies\n");
			break;

		case 'f':	/* Column information option  */
			GMT_message (GMT, "\t%s\n", GMT_f_OPT);
			GMT_message (GMT, "\t   Column information, add i for input, o for output [Default is both].\n");
			GMT_message (GMT, "\t   <colinfo> is <colno>|<colrange>u, where column numbers start at 0\n");
			GMT_message (GMT, "\t   a range is given as <first>-<last>, e.g., 2-5., u is type:\n");
			GMT_message (GMT, "\t   t: relative time, T: absolute time, f: floating point,\n");
			GMT_message (GMT, "\t   x: longitude, y: latitude, g: geographic coordinate.\n");
			break;

		case 'g':
			GMT_message (GMT, "\t%s\n", GMT_g_OPT);
			GMT_message (GMT, "\t   (Consult manual)\n");
			break;

		case 'h':	/* Header */

			GMT_message (GMT, "\t%s\n", GMT_h_OPT);
			GMT_message (GMT, "\t   Specify if Input/output file has header record(s)\n");
			GMT_message (GMT, "\t   Optionally, append i for input only and/or number of header records [0].\n");
			GMT_message (GMT, "\t     -hi turns off the writing of all headers on output.\n");
			GMT_message (GMT, "\t   Append +c to add header record with column information [none].\n");
			GMT_message (GMT, "\t   Append +d to delete headers before adding new ones [Default will append headers].\n");
			GMT_message (GMT, "\t   Append +r to add a <remark> comment to the output [none].\n");
			GMT_message (GMT, "\t   Append +t to add a <title> comment to the output [none].\n");
			GMT_message (GMT, "\t     (these strings may contain \\n to indicate line-breaks)\n");
			GMT_message (GMT, "\t   For binary files, <n> is considered to mean number of bytes.\n");
			break;

		case 'i':	/* -i option for input column order */

			GMT_message (GMT, "\t%s\n", GMT_i_OPT);
			GMT_message (GMT, "\t   Sets alternate input column order and/or scaling [Default reads all columns in order].\n");
			break;

		case 'n':	/* -n option for grid resampling parameters in BCR */

			GMT_message (GMT, "\t%s\n", GMT_n_OPT);
			GMT_message (GMT, "\t   Determine the grid interpolation mode.\n");
			GMT_message (GMT, "\t   (b = B-spline, c = bicubic, l = bilinear, n = nearest-neighbor) [Default: bicubic].\n");
			GMT_message (GMT, "\t   Append +a switch off antialiasing (except for l) [Default: on].\n");
			GMT_message (GMT, "\t   Append +b<BC> to change boundary conditions.  <BC> can be either:\n");
			GMT_message (GMT, "\t   g for geographic boundary conditions, or one or both of\n");
			GMT_message (GMT, "\t   x for periodic boundary conditions on x,\n");
			GMT_message (GMT, "\t   y for periodic boundary conditions on y.\n");
			GMT_message (GMT, "\t   [Default: Natural conditions, unless grid is known to be geographic].\n");
			GMT_message (GMT, "\t   Append +c to clip interpolated grid to input z-min/max [Default may exceed limits].\n");
			GMT_message (GMT, "\t   Append +t<threshold> to change the minimum weight in vicinity of NaNs. A threshold of\n");
			GMT_message (GMT, "\t   1.0 requires all nodes involved in interpolation to be non-NaN; 0.5 will interpolate\n");
			GMT_message (GMT, "\t   about half way from a non-NaN to a NaN node [Default: 0.5].\n");
			break;

		case 'o':	/* -o option for output column order */

			GMT_message (GMT, "\t%s\n", GMT_o_OPT);
			GMT_message (GMT, "\t   Sets alternate output column order and/or scaling [Default writes all columns in order].\n");
			break;

		case 'p':
			GMT_message (GMT, "\t%s\n", GMT_p_OPT);
			GMT_message (GMT, "\t   Azimuth and elevation (and zlevel) of the viewpoint [180/90/bottom z-axis].\n");
			GMT_message (GMT, "\t   Append +w and +v to set coordinates to a fixed viewpoint\n");
			break;

		case 's':	/* Skip records with NaN as z */
			GMT_message (GMT, "\t%s\n", GMT_s_OPT);
			GMT_message (GMT, "\t   Skip records whose <col> [2] output is NaN.\n");
			GMT_message (GMT, "\t   a skips if ANY columns is NaN, while r reverses the action.\n");
			break;

		case 't':	/* -t layer transparency option  */
			GMT_message (GMT, "\t%s\n", GMT_t_OPT);
			GMT_message (GMT, "\t   Set the layer PDF transparency from 0-100 [Default is 0; opaque].\n");
			break;

		case 'x':	/* Number of threads */
			GMT_message (GMT, "\t%s\n", GMT_x_OPT);
			GMT_message (GMT, "\t   Control the number of processors used in multi-threading.\n");
			GMT_message (GMT, "\t   -x+a Use all available processors.\n");
			GMT_message (GMT, "\t   -xn Use n processors (not more than max available off course).\n");
			GMT_message (GMT, "\t   -x-n Use (all - n) processors.\n");
			break;

		case ':':	/* lon/lat vs lat/lon i/o option  */
			GMT_message (GMT, "\t%s\n", GMT_colon_OPT);
			GMT_message (GMT, "\t   Interpret first two columns, add i for input, o for output [Default is both].\n");
			GMT_message (GMT, "\t   Swap 1st and 2nd column on input and/or output.\n");
			break;

		case '-':	/* --PAR=value  */
			GMT_message (GMT, "\t--<PARAMETER>=<value>.\n");
			GMT_message (GMT, "\t   See gmt.conf for list of parameters.\n");
			break;

		default:
			break;
	}
}

int GMT_default_error (struct GMT_CTRL *GMT, char option)
{
	/* GMT_default_error ignores all the common options that have already been processed and returns
	 * true if the option is not an already processed common option.
	 */

	int error = 0;

	switch (option) {

		case '-': break;	/* Skip indiscriminently */
		case '>': break;	/* Skip indiscriminently since dealt with internally */
		case 'B': error += (GMT->common.B.active[0] == false && GMT->common.B.active[1] == false); break;
		case 'J': error += GMT->common.J.active == false; break;
		case 'K': error += GMT->common.K.active == false; break;
		case 'O': error += GMT->common.O.active == false; break;
		case 'P': error += GMT->common.P.active == false; break;
		case 'R': error += GMT->common.R.active == false; break;
		case 'U': error += GMT->common.U.active == false; break;
		case 'V': error += GMT->common.V.active == false; break;
		case 'X': error += GMT->common.X.active == false; break;
		case 'Y': error += GMT->common.Y.active == false; break;
		case 'a': error += GMT->common.a.active == false; break;
		case 'b': error += (GMT->common.b.active[GMT_IN] == false && GMT->common.b.active[GMT_OUT] == false); break;
		case 'c': error += GMT->common.c.active == false; break;
		case 'd': error += (GMT->common.d.active[GMT_IN] == false && GMT->common.d.active[GMT_OUT] == false); break;
		case 'f': error += (GMT->common.f.active[GMT_IN] == false &&  GMT->common.f.active[GMT_OUT] == false); break;
		case 'g': error += GMT->common.g.active == false; break;
		case 'H':
			if (GMT_compat_check (GMT, 4)) {
				error += GMT->common.h.active == false;
			}
			else
				error++;
			break;
		case 'h': error += GMT->common.h.active == false; break;
		case 'i': error += GMT->common.i.active == false; break;
		case 'n': error += GMT->common.n.active == false; break;
		case 'o': error += GMT->common.o.active == false; break;
		case 'Z':
			if (!GMT_compat_check (GMT, 4)) error++;
			break;
		case 'E':
			if (GMT_compat_check (GMT, 4))
				error += GMT->common.p.active == false;
			else
				error++;
			break;
		case 'p': error += GMT->common.p.active == false; break;
		case 'm': if (!GMT_compat_check (GMT, 4)) error++; break;
		case 'S': if (!GMT_compat_check (GMT, 4)) error++; break;
		case 'F': if (!GMT_compat_check (GMT, 4)) error++; break;
		case 'r': error += GMT->common.r.active == false; break;
		case 's': error += GMT->common.s.active == false; break;
		case 't': error += GMT->common.t.active == false; break;
#ifdef HAVE_GLIB_GTHREAD
		case 'x': error += GMT->common.x.active == false; break;
#endif
		case ':': error += GMT->common.colon.active == false; break;

		default:
			/* Not a processed common options */
			error++;
			break;
	}

	if (error) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unrecognized option -%c\n", option);
	return (error);
}

/*! . */
int gmt_parse_h_option (struct GMT_CTRL *GMT, char *item) {
	int i, k = 1, error = 0, col = -1;
	unsigned int pos = 0;
	char p[GMT_BUFSIZ] = {""}, *c = NULL;

	/* Parse the -h option.  Full syntax: -h[i|o][<nrecs>][+c][+d][+r<remark>][+t<title>] */

	/* Note: This forces the io to skip the first <nrecs> records, regardless of what they are.
	 * In addition, any record starting with # will be considered a comment.
	 * For output (-ho) no <nrecs> is allowed since either (a) we output the same number of
	 * input records we found or (b) the program writes a specific number of records built from scratch.
	 * Use +c to add a header identifying the various columns + [colno].
	 * Use +d to have a program delete existing headers in the input [Default appends].
	 * Use +r<remark> to add a specified header remark to the output file.
	 * Use +t<title> to add a specified header title to the output file.
	 */
	if (!item || !item[0]) {	/* Just -h: Nothing further to parse; just set defaults */
		GMT->current.setting.io_header[GMT_IN] = GMT->current.setting.io_header[GMT_OUT] = true;
		return (GMT_NOERROR);
	}
	if (item[0] == 'i')	/* Apply to input only */
		col = GMT_IN;
	else if (item[0] == 'o')	/* Apply to output only */
		col = GMT_OUT;
	else			/* Apply to both input and output columns */
		k = 0;
	if (item[k]) {	/* Specified how many records for input */
		if (col == GMT_OUT) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Can only set the number of input header records; %s ignored\n", &item[k]);
		}
		else {
			i = atoi (&item[k]);
			if (i < 0)
				error++;
			else
				GMT->current.setting.io_n_header_items = i;
		}
	}

	if (col == GMT_IN) {		/* Only input should have header records, set to true unless we gave -h[i]0 */
		GMT->current.setting.io_header[GMT_IN] = true;
		GMT->current.setting.io_header[GMT_OUT] = false;
	}
	else if (col == GMT_OUT) {	/* Only output should have header records */
		GMT->current.setting.io_header[GMT_OUT] = true;
		GMT->current.setting.io_header[GMT_IN] = false;
	}
	else {	/* Both in and out may have header records */
		GMT->current.setting.io_header[GMT_IN] = true;
		GMT->current.setting.io_header[GMT_OUT] = true;
	}

	if ((c = strchr (item, '+'))) {	/* Found modifiers */
		while ((GMT_strtok (c, "+", &pos, p))) {
			switch (p[0]) {
				case 'd':	/* Delete existing headers */
					GMT->common.h.mode = GMT_COMMENT_IS_RESET;
					break;
				case 'c':	/* Add column names record */
					GMT->common.h.add_colnames = true;
					break;
				case 'r':	/* Add specific text remark */
					if (GMT->common.h.remark) free (GMT->common.h.remark);
					GMT->common.h.remark = strdup (&p[1]);
					break;
				case 't':	/* Add specific text title */
					if (GMT->common.h.title) free (GMT->common.h.title);
					GMT->common.h.title = strdup (&p[1]);
					break;
				default:	/* Bad modifier */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unrecognized modifier +%c.\n", p[0]);
					error++;
					break;
			}
		}

	}
	if ((c = strstr (item, "+t"))) *c = '\0';	/* Truncate the -h...+t<txt> option to avoid duplicate title output in command */
	return (error);
}

/*! If region is given then we must have w < e and s < n */
bool GMT_check_region (struct GMT_CTRL *GMT, double wesn[]) {
	GMT_UNUSED(GMT);
	return ((wesn[XLO] >= wesn[XHI] || wesn[YLO] >= wesn[YHI]));
}

/*! . */
int GMT_rectR_to_geoR (struct GMT_CTRL *GMT, char unit, double rect[], double out_wesn[], bool get_R)
{
	/* If user gives -Re|f|k|M|n<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>][r] then we must
	 * call GMT_mapproject to convert this to geographic degrees.
	 * get_R is true when this is done to obtain the -R setting.  */

	int object_ID, proj_class;
	uint64_t dim[4] = {1, 1, 2, 2};	/* Just a single data table with one segment with two 2-column records */
	bool was_R, was_J;
	double wesn[4];
	char buffer[GMT_BUFSIZ] = {""}, in_string[GMT_STR16] = {""}, out_string[GMT_STR16] = {""};
	struct GMT_DATASET *In = NULL, *Out = NULL;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Call GMT_rectR_to_geoR to convert projected -R to geo -R\n");
	if (GMT_is_dnan (GMT->current.proj.lon0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Central meridian is not known; cannot convert -R<unit>... to geographic corners\n");
		return (GMT_MAP_NO_PROJECTION);
	}
	if (GMT_is_dnan (GMT->current.proj.lat0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Projection standard latitude is not known; cannot convert -R<unit>... to geographic corners\n");
		return (GMT_MAP_NO_PROJECTION);
	}
	/* Create dataset to hold the rect coordinates */
	if ((In = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (GMT_MEMORY_ERROR);

	In->table[0]->segment[0]->coord[GMT_X][0] = rect[XLO];
	In->table[0]->segment[0]->coord[GMT_Y][0] = rect[YLO];
	In->table[0]->segment[0]->coord[GMT_X][1] = rect[XHI];
	In->table[0]->segment[0]->coord[GMT_Y][1] = rect[YHI];

	/* Set up machinery to call mapproject */

	/* Register In as input source via ref (this just returns the ID associated with In sinc already registered by GMT_Create_Data) */
	if ((object_ID = GMT_Register_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_REFERENCE, GMT_IS_POINT, GMT_IN, NULL, In)) == GMT_NOTSET) {
		return (GMT->parent->error);
	}
	if (GMT_Encode_ID (GMT->parent, in_string, object_ID) != GMT_OK) {	/* Make filename with embedded object ID */
		return (GMT->parent->error);
	}
	if ((object_ID = GMT_Register_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_DUPLICATE, GMT_IS_POINT, GMT_OUT, NULL, NULL)) == GMT_NOTSET) {
		return (GMT->parent->error);
	}
	if (GMT_Encode_ID (GMT->parent, out_string, object_ID)) {
		return (GMT->parent->error);	/* Make filename with embedded object ID */
	}
	was_R = GMT->common.R.active;	was_J = GMT->common.J.active;
	GMT->common.R.active = GMT->common.J.active = false;	/* To allow new entries */

	/* Determine suitable -R setting for this projection */

	/* Default w/e/s/n is small patch centered on projection center - this may change below */
	wesn[XLO] = GMT->current.proj.lon0 - 1.0;		wesn[XHI] = GMT->current.proj.lon0 + 1.0;
	wesn[YLO] = MAX (GMT->current.proj.lat0 -1.0, -90.0);	wesn[YHI] = MIN (GMT->current.proj.lat0 + 1.0, 90.0);

	proj_class = GMT->current.proj.projection / 100;	/* 1-4 for valid projections */
	if (GMT->current.proj.projection == GMT_AZ_EQDIST) proj_class = 4;	/* Make -JE use global region */
	switch (proj_class) {
		case 1:	/* Cylindrical: pick small equatorial patch centered on central meridian */
			if (GMT->current.proj.projection == GMT_UTM && GMT_UTMzone_to_wesn (GMT, GMT->current.proj.utm_zonex, GMT->current.proj.utm_zoney, GMT->current.proj.utm_hemisphere, wesn))
			{
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: UTM projection insufficiently specified to auto-determine geographic region\n");
				return (GMT_MAP_NO_PROJECTION);
			}
			else {
				wesn[YLO] = -1.0;	wesn[YHI] = 1.0;
			}
			break;
		case 2: /* Conical: Use default patch */
			break;
		case 3: /* Azimuthal: Use default patch, or hemisphere for polar projections */
			wesn[XLO] = GMT->current.proj.lon0 - 180.0;	wesn[XHI] = GMT->current.proj.lon0 + 180.0;
			if (doubleAlmostEqualZero (GMT->current.proj.lat0, 90.0)) {
				wesn[YLO] = 0.0;	wesn[YHI] = 90.0;
			}
			else if (doubleAlmostEqualZero (GMT->current.proj.lat0, -90.0)) {
				wesn[YLO] = -90.0;	wesn[YHI] = 0.0;
			}
			break;
		case 4: /* Global: Give global region */
			wesn[XLO] = 0.0;	wesn[XHI] = 360.0;	wesn[YLO] = -90.0;	wesn[YHI] = 90.0;
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: No map projection specified to auto-determine geographic region\n");
			break;
	}
	sprintf (buffer, "-R%g/%g/%g/%g -J%s -I -F%c -C -bi2d -bo2d -<%s ->%s",
	         wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], GMT->common.J.string, unit, in_string, out_string);
	if (get_R) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Obtaining geographic corner coordinates via mapproject %s\n", buffer);
	if (GMT_Call_Module (GMT->parent, "mapproject", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Get the corners in degrees via mapproject */
		return (GMT->parent->error);
	}
	GMT->common.R.active = was_R;	GMT->common.J.active = was_J;
	if ((Out = GMT_Retrieve_Data (GMT->parent, object_ID)) == NULL) {
		return (GMT->parent->error);
	}
	out_wesn[XLO] = Out->table[0]->segment[0]->coord[GMT_X][0];
	out_wesn[YLO] = Out->table[0]->segment[0]->coord[GMT_Y][0];
	out_wesn[XHI] = Out->table[0]->segment[0]->coord[GMT_X][1];
	out_wesn[YHI] = Out->table[0]->segment[0]->coord[GMT_Y][1];

	if (get_R) GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
	                       "Region selection -R%s is replaced by the equivalent geographic region -R%.12g/%.12g/%.12g/%.12gr\n",
	                       GMT->common.R.string, out_wesn[XLO], out_wesn[YLO], out_wesn[XHI], out_wesn[YHI]);

	if (GMT_Destroy_Data (GMT->parent, &In) != GMT_OK) {
		return (GMT->parent->error);
	}
	if (GMT_Destroy_Data (GMT->parent, &Out) != GMT_OK) {
		return (GMT->parent->error);
	}

	return (GMT_NOERROR);
}

/*! . */
int gmt_parse_R_option (struct GMT_CTRL *GMT, char *item) {
	unsigned int i, icol, pos, error = 0, n_slash = 0;
	int got, col_type[2], expect_to_read;
	size_t length;
	bool inv_project = false, scale_coord = false;
	char text[GMT_BUFSIZ] = {""}, string[GMT_BUFSIZ] = {""}, r_unit = 0;
	double p[6];

	if (!item || !item[0]) return (GMT_PARSE_ERROR);	/* Got nothing */

	/* Parse the -R option.  Full syntax: -R<grdfile> or -Rg or -Rd or -R[L|C|R][B|M|T]<x0>/<y0>/<nx>/<ny> or -R[g|d]w/e/s/n[/z0/z1][r] */
	length = strlen (item) - 1;
	for (i = 0; i < length; i++) if (item[i] == '/') n_slash++;

	strncpy (GMT->common.R.string, item, GMT_LEN256);	/* Verbatim copy */

	if (strchr ("LCRlcr", item[0]) && strchr ("TMBtmb", item[1])) {	/* Extended -R option using coordinate codes and grid increments */
		char X[2][GMT_LEN64] = {"", ""}, code[3] = {""};
		double xdim, ydim, orig[2];
		int nx, ny, just;
		GMT_memcpy (code, item, 2, char);
		if ((just = GMT_just_decode (GMT, code, PSL_NO_DEF)) == -99) {	/* Since justify not in correct format */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -R: Unrecognized justification code %s\n", code);
			return (GMT_PARSE_ERROR);
		}
		if (sscanf (&item[2], "%[^/]/%[^/]/%d/%d", X[0], X[1], &nx, &ny) != 4) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -R%s<lon0>/<lat0>/<nx>/<ny>: Did not get 4 items\n", code);
			return (GMT_PARSE_ERROR);
		}
		for (icol = GMT_X; icol <= GMT_Y; icol++) {
			if (GMT->current.io.col_type[GMT_IN][icol] == GMT_IS_UNKNOWN) {	/* No -J or -f set, proceed with caution */
				got = GMT_scanf_arg (GMT, X[icol], GMT->current.io.col_type[GMT_IN][icol], &orig[icol]);
				if (got & GMT_IS_GEO)
					GMT->current.io.col_type[GMT_IN][icol] = got;
				else if (got & GMT_IS_RATIME)
					GMT->current.io.col_type[GMT_IN][icol] = got, GMT->current.proj.xyz_projection[icol] = GMT_TIME;
			}
			else {	/* Things are set, do or die */
				expect_to_read = (GMT->current.io.col_type[GMT_IN][icol] & GMT_IS_RATIME) ? GMT_IS_ARGTIME : GMT->current.io.col_type[GMT_IN][icol];
				error += GMT_verify_expectations (GMT, expect_to_read, GMT_scanf (GMT, X[icol], expect_to_read, &orig[icol]), X[icol]);
			}
		}
		if (error) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -R%s<lon0>/<lat0>/<nx>/<ny>: Could not parse coordinate pair\n", code);
			return (GMT_PARSE_ERROR);
		}
		if (nx <= 0 || ny <= 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -R%s<lon0>/<lat0>/<nx>/<ny>: Must have positive dimensions\n", code);
			return (GMT_PARSE_ERROR);
		}
		/* Finally set up -R */
		if (!GMT->common.r.active) nx--, ny--;	/* Needed to get correct dimensions */
		xdim = nx * GMT->common.API_I.inc[GMT_X];
		ydim = ny * GMT->common.API_I.inc[GMT_Y];
		GMT->common.R.wesn[XLO] = orig[GMT_X] - 0.5 * ((just%4)-1) * xdim;
		GMT->common.R.wesn[YLO] = orig[GMT_Y] - 0.5 * (just/4) * ydim;
		GMT->common.R.wesn[XHI] = GMT->common.R.wesn[XLO] + xdim;
		GMT->common.R.wesn[YHI] = GMT->common.R.wesn[YLO] + ydim;
		return (GMT_NOERROR);
	}
	if ((item[0] == 'g' || item[0] == 'd') && item[1] == '\0') {	/* Check -Rd|g separately in case user has files called d or g */
		if (item[0] == 'g') {	/* -Rg is shorthand for -R0/360/-90/90 */
			GMT->common.R.wesn[XLO] = 0.0, GMT->common.R.wesn[XHI] = 360.0;
			GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
		}
		else {			/* -Rd is shorthand for -R-180/180/-90/90 */
			GMT->common.R.wesn[XLO] = -180.0, GMT->common.R.wesn[XHI] = 180.0;
			GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
		}
		GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = +90.0;
		GMT_set_geographic (GMT, GMT_IN);
		return (GMT_NOERROR);
	}
	if (!GMT_access (GMT, item, R_OK)) {	/* Gave a readable file, presumably a grid */
		struct GMT_GRID *G = NULL;
		if ((G = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, item, NULL)) == NULL) {	/* Read header */
			return (GMT->parent->error);
		}
		GMT_memcpy (&(GMT->current.io.grd_info.grd), G->header, 1, struct GMT_GRID_HEADER);
		if (GMT_Destroy_Data (GMT->parent, &G) != GMT_OK) {
			return (GMT->parent->error);
		}
		if ((GMT->current.proj.projection == GMT_UTM || GMT->current.proj.projection == GMT_TM || GMT->current.proj.projection == GMT_STEREO)) {	/* Perhaps we got an [U]TM or stereographic grid? */
			if (fabs (GMT->current.io.grd_info.grd.wesn[XLO]) > 360.0 || fabs (GMT->current.io.grd_info.grd.wesn[XHI]) > 360.0 \
			  || fabs (GMT->current.io.grd_info.grd.wesn[YLO]) > 90.0 || fabs (GMT->current.io.grd_info.grd.wesn[YHI]) > 90.0) {	/* Yes we probably did, but cannot be sure */
				inv_project = true;
				r_unit = 'e';	/* Must specify the "missing" leading e for meter */
				sprintf (string, "%.16g/%.16g/%.16g/%.16g", GMT->current.io.grd_info.grd.wesn[XLO], GMT->current.io.grd_info.grd.wesn[XHI], GMT->current.io.grd_info.grd.wesn[YLO], GMT->current.io.grd_info.grd.wesn[YHI]);
			}
		}
		if (!inv_project) {	/* Got grid with degrees */
			GMT_memcpy (GMT->common.R.wesn, GMT->current.io.grd_info.grd.wesn, 4, double);
#if 0
			/* Next bit removed because of issue #592. Should not change the boundaries of the grid */
			if (GMT->current.io.grd_info.grd.registration == GMT_GRID_NODE_REG && doubleAlmostEqualZero (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO] + GMT->current.io.grd_info.grd.inc[GMT_X], 360.0)) {
				/* Geographic grid with gridline registration that does not contain the repeating column, but is still 360 range */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG,
				            "-R<file> with gridline registration and non-repeating column detected; return full 360 degree range for -R\n");
				if (GMT_IS_ZERO (GMT->common.R.wesn[XLO]) || doubleAlmostEqualZero (GMT->common.R.wesn[XLO], -180.0))
					GMT->common.R.wesn[XHI] = GMT->common.R.wesn[XLO] + 360.0;
				else
					GMT->common.R.wesn[XLO] = GMT->common.R.wesn[XHI] - 360.0;
			}
#endif
			GMT->common.R.wesn[ZLO] = GMT->current.io.grd_info.grd.z_min;	GMT->common.R.wesn[ZHI] = GMT->current.io.grd_info.grd.z_max;
			GMT->current.io.grd_info.active = true;
			return (GMT_NOERROR);
		}
	}
	else if ((item[0] == 'g' || item[0] == 'd') && n_slash == 3) {	/* Here we have a region appended to -Rd|g */
		GMT_set_geographic (GMT, GMT_IN);
		strncpy (string, &item[1], GMT_BUFSIZ);
		GMT->current.io.geo.range = (item[0] == 'g') ? GMT_IS_0_TO_P360_RANGE : GMT_IS_M180_TO_P180_RANGE;
	}
	else if (strchr (GMT_LEN_UNITS2, item[0])) {	/* Specified min/max in projected distance units */
		strncpy (string, &item[1], GMT_BUFSIZ);
		r_unit = item[0];	/* The leading unit */
		if (GMT_IS_LINEAR (GMT))	/* Just scale up the values */
			scale_coord = true;
		else
			inv_project = true;
	}
	else if (item[length] != 'r' && (GMT->current.proj.projection == GMT_UTM || GMT->current.proj.projection == GMT_TM ||
	         GMT->current.proj.projection == GMT_STEREO)) {	/* Just _might_ be getting -R in meters, better check */
		double rect[4];
		strncpy (string, item, GMT_BUFSIZ);
		sscanf (string, "%lg/%lg/%lg/%lg", &rect[XLO], &rect[XHI], &rect[YLO], &rect[YHI]);
		if (fabs (rect[XLO]) > 360.0 || fabs (rect[XHI]) > 360.0 || fabs (rect[YLO]) > 90.0 || fabs (rect[YHI]) > 90.0) {	/* Oh, yeah... */
			inv_project = true;
			r_unit = 'e';	/* Must specify the "missing" leading e for meter */
		}
	}
	else	/* Plain old -Rw/e/s/n */
		strncpy (string, item, GMT_BUFSIZ);

	/* Now decode the string */

	length = strlen (string) - 1;
	col_type[0] = col_type[1] = 0;
	if (string[length] == 'r') {
		GMT->common.R.oblique = true;
		string[strlen(string)-1] = '\0';	/* Remove the trailing r so GMT_scanf will work */
	}
	else
		GMT->common.R.oblique = false;
	i = pos = 0;
	GMT_memset (p, 6, double);
	while ((GMT_strtok (string, "/", &pos, text))) {
		if (i > 5) {
			error++;
			return (error);		/* Have to break out here to avoid segv on p[6]  */
		}
		/* Figure out what column corresponds to a token to get col_type[GMT_IN] flag  */
		if (i > 3)
			icol = 2;
		else if (GMT->common.R.oblique)
			icol = i%2;
		else
			icol = i/2;
		if (icol < 2 && GMT->current.setting.io_lonlat_toggle[GMT_IN]) icol = 1 - icol;	/* col_types were swapped */
		/* If column is either RELTIME or ABSTIME, use ARGTIME; if inv_project then just read floats via atof */
		if (inv_project)	/* input is distance units */
			p[i] = atof (text);
		else if (GMT->current.io.col_type[GMT_IN][icol] == GMT_IS_UNKNOWN) {	/* No -J or -f set, proceed with caution */
			got = GMT_scanf_arg (GMT, text, GMT->current.io.col_type[GMT_IN][icol], &p[i]);
			if (got & GMT_IS_GEO)
				GMT->current.io.col_type[GMT_IN][icol] = got;
			else if (got & GMT_IS_RATIME)
				GMT->current.io.col_type[GMT_IN][icol] = got, GMT->current.proj.xyz_projection[icol] = GMT_TIME;
		}
		else {	/* Things are set, do or die */
			expect_to_read = (GMT->current.io.col_type[GMT_IN][icol] & GMT_IS_RATIME) ? GMT_IS_ARGTIME : GMT->current.io.col_type[GMT_IN][icol];
			error += GMT_verify_expectations (GMT, expect_to_read, GMT_scanf (GMT, text, expect_to_read, &p[i]), text);
		}
		if (error) return (error);

		i++;
	}
	if (GMT->common.R.oblique) {
		double_swap (p[2], p[1]);	/* So w/e/s/n makes sense */
		GMT_memcpy (GMT->common.R.wesn_orig, p, 4, double);	/* Save these in case they get enlarged by oblique projections */
	}
	if (inv_project) {	/* Convert rectangular distances to geographic corner coordinates */
		double wesn[4];
		GMT->common.R.oblique = false;
		error += GMT_rectR_to_geoR (GMT, r_unit, p, wesn, true);
		GMT_memcpy (p, wesn, 4, double);
		GMT->common.R.oblique = true;
	}
	else if (scale_coord) {	/* Just scale x/y coordinates to meters according to given unit */
		double fwd_scale, inv_scale = 0.0, inch_to_unit, unit_to_inch;
		int k_unit;
		k_unit = GMT_get_unit_number (GMT, item[0]);
		GMT_init_scales (GMT, k_unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, NULL);
		for (pos = 0; pos < 4; pos++) p[pos] *= inv_scale;
	}

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Arrange so geographic region always has w < e */
		double w = p[0], e = p[1];
		if (p[0] <= -360.0 || p[1] > 360.0) {	/* Arrange so geographic region always has |w,e| <= 360 */
			double shift = (p[0] <= -360.0) ? 360.0 : -360.0;
			p[0] += shift;	p[1] += shift;
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "Warning -R: Given west and east values [%g %g] were adjusted so not exceed multiples of 360 [%g %g]\n", w, e, p[0], p[1]);
		}
		else if (p[0] > p[1] && GMT->common.R.oblique && !GMT->common.J.active) {	/* Used -Rw/s/e/nr for non mapping */
			if (GMT->current.io.geo.range == GMT_IS_M180_TO_P180_RANGE) p[0] -= 360.0; else p[1] += 360.0;
		}
#if 0	/* This causes too much trouble: Better to annoy the person wishing this to work vs annoy all those who made an honest error.  We cannot be mind-readers here so we insist on e > w */
		else if (p[0] > p[1]) {	/* Arrange so geographic region always has w < e */
			if (GMT->current.io.geo.range == GMT_IS_M180_TO_P180_RANGE) p[0] -= 360.0; else p[1] += 360.0;
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "Warning -R: Given west and east values [%g %g] were adjusted so west < east [%g %g]\n", w, e, p[0], p[1]);
		}
#endif
	}
	if (i < 4 || i > 6 || ((!GMT->common.R.oblique && GMT_check_region (GMT, p)) || (i == 6 && p[4] >= p[5]))) error++;
	GMT_memcpy (GMT->common.R.wesn, p, 6, double);	/* This will probably be reset by GMT_map_setup */
	error += GMT_check_condition (GMT, i == 6 && !GMT->current.proj.JZ_set, "Error: -R with six parameters requires -Jz|Z\n");

	return (error);
}

/*! . */
int gmt_parse_XY_option (struct GMT_CTRL *GMT, int axis, char *text)
{
	int i = 0;
	if (!text || !text[0]) {	/* Default is -Xr0 */
		GMT->current.ps.origin[axis] = 'r';
		GMT->current.setting.map_origin[axis] = 0.0;
		return (GMT_NOERROR);
	}
	switch (text[0]) {
		case 'r': case 'a': case 'f': case 'c':
			GMT->current.ps.origin[axis] = text[0]; i++; break;
		default:
			GMT->current.ps.origin[axis] = 'r'; break;
	}
	if (text[i])
		GMT->current.setting.map_origin[axis] = GMT_to_inch (GMT, &text[i]);
	else	/* Allow use of -Xc or -Xf meaning -Xc0 or -Xf0 */
		GMT->current.setting.map_origin[axis] = 0.0;
	return (GMT_NOERROR);
}

/*! . */
int gmt_parse_a_option (struct GMT_CTRL *GMT, char *arg)
{	/* -a<col>=<name>[:<type>][,<col>...][+g|G<geometry>] */
	unsigned int pos = 0;
	int col, a_col = GMT_Z;
	char p[GMT_BUFSIZ] = {""}, name[GMT_BUFSIZ] = {""}, A[64] = {""}, *s = NULL, *c = NULL;
	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -a requires an argument */
	if ((s = strstr (arg, "+g")) || (s = strstr (arg, "+G"))) {	/* Also got +g|G<geometry> */
		GMT->common.a.geometry = gmt_ogr_get_geometry (s+2);
		if (s[1] == 'G') GMT->common.a.clip = true;	/* Clip features at Dateline */
		s[0] = '\0';	/* Temporarily truncate off the geometry */
		GMT->common.a.output = true;	/* We are producing, not reading an OGR/GMT file */
		if (GMT->current.setting.io_seg_marker[GMT_OUT] != '>') {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "Warning -a: OGR/GMT requires > as output segment marker; your selection of %c will be overruled by >\n",
			            GMT->current.setting.io_seg_marker[GMT_OUT]);
			GMT->current.setting.io_seg_marker[GMT_OUT] = '>';
		}
	}
	else if (GMT->current.setting.io_seg_marker[GMT_IN] != '>') {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
		            "Warning -a: OGR/GMT requires < as input segment marker; your selection of %c will be overruled by >\n",
		            GMT->current.setting.io_seg_marker[GMT_IN]);
		GMT->current.setting.io_seg_marker[GMT_IN] = '>';
	}
	while ((GMT_strtok (arg, ",", &pos, p))) {	/* Another col=name argument */
		if ((c = strchr (p, ':'))) {	/* Also got :<type> */
			GMT->common.a.type[GMT->common.a.n_aspatial] = gmt_ogr_get_type (c+1);
			c[0] = '\0';	/* Truncate off the type */
		}
		else
			GMT->common.a.type[GMT->common.a.n_aspatial] = GMT_DOUBLE;
		if (strchr (p, '=')) {	/* Got col=name */
			if (sscanf (p, "%[^=]=%s", A, name) != 2) return (GMT_PARSE_ERROR);	/* Did not get two items */
		}
		else {	/* Auto-fill col as the next col starting at GMT_Z */
			sprintf (A, "%d", a_col++);
			strcpy (name, p);
		}
		switch (A[0]) {	/* Watch for different multisegment header cases */
			case 'D': col = GMT_IS_D; break;	/* Distance flag */
			case 'G': col = GMT_IS_G; break;	/* Color flag */
			case 'I': col = GMT_IS_I; break;	/* ID flag */
			case 'L': col = GMT_IS_L; break;	/* Label flag */
			case 'T': col = GMT_IS_T; break;	/* Text flag */
			case 'W': col = GMT_IS_W; break;	/* Pen flag */
			case 'Z': col = GMT_IS_Z; break;	/* Value flag */
			default:
				col = atoi (A);
				if (col < GMT_Z)
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -a: Columns 0 and 1 are reserved for lon and lat.\n");
				if (col < GMT_Z || col >= GMT_MAX_COLUMNS) return (GMT_PARSE_ERROR);		/* Col value out of whack */
				break;
		}
		GMT->common.a.col[GMT->common.a.n_aspatial] = col;
		if (col < 0 && col != GMT_IS_Z) GMT->common.a.type[GMT->common.a.n_aspatial] = GMT_TEXT;
		if (GMT->common.a.name[GMT->common.a.n_aspatial]) free (GMT->common.a.name[GMT->common.a.n_aspatial]);	/* Free any previous names */
		GMT->common.a.name[GMT->common.a.n_aspatial] = strdup (name);
		GMT->common.a.n_aspatial++;
		if (GMT->common.a.n_aspatial == MAX_ASPATIAL) return (GMT_PARSE_ERROR);	/* Too many items */
	}
	if (s) s[0] = '+';	/* Restore the geometry part */
	/* -a implies -fg */
	GMT_set_geographic (GMT, GMT_IN);
	GMT_set_geographic (GMT, GMT_OUT);
	return (GMT_NOERROR);
}

/*! . */
int gmt_parse_b_option (struct GMT_CTRL *GMT, char *text)
{
	/* GMT5 Syntax:	-b[i][cvar1/var2/...] or -b[i|o]<n><type>[,<n><type>]...
	 * GMT4 Syntax:	-b[i][o][s|S][d|D][<n>][c[<var1>/<var2>/...]]
	 * -b with no args means both in and out have double precision binary i/o, with #columns determined by module
	 * -bi or -bo means the same for that direction only.
	 * -bif or -bof or any other letter means that type instead of double.
	 * Note to developers: It is allowed NOT to specify anything (e.g., -bi) or not specify how many columns (e.g., -bif).
	 * If those are not set then there are two opportunities in the modules to correct this:
	 *   1) GMT_io_banner is called from GMT_Init_IO and if things are not set we default to the default data type [double].
	 *   2) GMT_set_cols sets the actual columns needed for in or out and is also use to set un-initialized data read pointers.
	 */

	unsigned int i, col = 0, id = GMT_IN, swap_flag;
	int k, ncol = 0;
	bool endian_swab = false, swab = false, error = false, i_or_o = false, set = false, v4_parse = false;
	char *p = NULL, c;

	if (!text) return (GMT_PARSE_ERROR);	/* -B requires an argument even if it is blank */
	/* First determine if there is an endian modifer supplied */
	if ((p = strchr (text, '+'))) {	/* Yes */
		*p = '\0';	/* Temporarily chop off the modifier */
		switch (p[1]) {
			case 'B':
			case 'L':
				swab = (p[1] != GMT_ENDIAN);
				break;	/* Must swap */
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -b: Bad endian modifier +%c\n", (int)p[1]);
				return (EXIT_FAILURE);
				break;
		}
		if (strchr (text, 'w')) {	/* Cannot do individual swap when endian has been indicated */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -b: Cannot use both w and endian modifiers\n");
			return (EXIT_FAILURE);
		}
		endian_swab = true;
	}

	/* Now deal with [i|o] modifier Note: If there is no i|o then id is GMT_IN below */
	if (text[0] == 'i') { id = GMT_IN; i_or_o = true; }
	if (text[0] == 'o') { id = GMT_OUT; i_or_o = true; }
	GMT->common.b.active[id] = true;
	GMT->common.b.type[id] = 'd';	/* Set default to double */
	GMT->common.b.swab[id] = k_swap_none;	/* No byte swapping */

	/* Because under GMT_COMPAT c means either netCDF or signed char we deal with netCDF up front */

	k = (i_or_o) ? 1 : 0;
	if (GMT_compat_check (GMT, 4)) {	/* GMT4 */
		if (text[k] == 'c' && !text[k+1]) {	/* Ambiguous case "-bic" which MAY mean netCDF */
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Syntax warning: -b[i]c now applies to character tables, not to netCDF\n");
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Syntax warning: If input file is netCDF, just leave out -b[i]c\n");
			GMT->common.b.type[id] = 'c';
			v4_parse = true;	/* Yes, we parsed a GMT4-compatible option */
		}
		else if (text[k] == 'c' && text[k+1] != ',') {	/* netCDF with list of variables */
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Syntax warning: -b[i]c<varlist> is deprecated. Use <file>?<varlist> instead.\n");
			GMT->common.b.active[id] = false;	/* Binary is 'false' if netCDF is to be read */
			strncpy (GMT->common.b.varnames, &text[k+1], GMT_BUFSIZ);	/* Copy the list of netCDF variable names */
			v4_parse = true;	/* Yes, we parsed a GMT4-compatible option */
		}
	}
	if (!v4_parse && text[k] && strchr ("cuhHiIfd" GMT_OPT ("sSD"), text[k]) && (!text[k+1] || (text[k+1] == 'w' && !text[k+2] ))) {	/* Just save the type for the entire record */
		GMT->common.b.type[id] = text[k];			/* Set the default column type to the first (and possibly only data type) listed */
		if (GMT_compat_check (GMT, 4)) {	/* GMT4: Must switch s,S,D to f, f(with swab), and d (with swab) */
			if (GMT->common.b.type[id] == 's') GMT->common.b.type[id] = 'f';
			if (GMT->common.b.type[id] == 'S') { GMT->common.b.type[id] = 'f'; GMT->common.b.swab[id] = (id == GMT_IN) ? k_swap_in : k_swap_out;	}
			if (GMT->common.b.type[id] == 'D') { GMT->common.b.type[id] = 'd'; GMT->common.b.swab[id] = (id == GMT_IN) ? k_swap_in : k_swap_out;	}
		}
		if (text[k+1] == 'w') GMT->common.b.swab[id] = (id == GMT_IN) ? k_swap_in : k_swap_out;	/* Default swab */
	}
	if (!v4_parse) {	/* Meaning we did not hit netcdf-like options above */
		for (i = k; text[i]; i++) {
			c = text[i];
			switch (c) {
				case 's': case 'S': case 'D':	/* Obsolete GMT 4 syntax with single and double precision w/wo swapping */
					if (GMT_compat_check (GMT, 4)) {
						GMT_Report (GMT->parent, GMT_MSG_COMPAT,
						            "Warning: -b option with type s, S, or D are deprecated; Use <n>f or <n>d, with w to indicate swab\n");
						if (c == 'S' || c == 'D') swab = true;
						if (c == 'S' || c == 's') c = 'f';
						else if (c == 'D') c = 'd';
						if (ncol == 0) ncol = 1;	/* In order to make -bs work as before */
					}
					else {
						error = true;
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Malformed -b argument [%s]\n", text);
						GMT_syntax (GMT, 'b');
						break;
					} /* Deliberate fall-through to these cases here */
				case 'c': case 'u': /* int8_t, uint8_t */
				case 'h': case 'H': /* int16_t, uint16_t */
				case 'i': case 'I': /* int32_t, uint32_t */
				case 'l': case 'L': /* int64_t, uint64_t */
				case 'f': case 'd': /* float, double */
					if (GMT_compat_check (GMT, 4) && c == 'd' && ncol == 0) {
						ncol = 1;	/* In order to make -bd work as before */
						GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: -b[o]d is deprecated; Use <n>d to indicate how many columns\n");
					}
					if (text[i+1] == 'w')	/* Want to swab the input or output first */
						swab = true;
					set = true;	/* Meaning we have set the data type */
					if (ncol == 0) {	/* Just specifying type, no columns yet */
						GMT->common.b.type[id] = c;	/* Set default to double */
					}
					swap_flag = (swab) ? id + 1 : 0;	/* 0 for no swap, 1 if swap input, 2 if swap output */
					for (k = 0; k < ncol; k++, col++) {	/* Assign io function pointer and data type for each column */
						GMT->current.io.fmt[id][col].io = GMT_get_io_ptr (GMT, id, swap_flag, c);
						GMT->current.io.fmt[id][col].type = GMT_get_io_type (GMT, c);
						if (!i_or_o) {	/* Must also set output */
							GMT->current.io.fmt[GMT_OUT][col].io = GMT_get_io_ptr (GMT, GMT_OUT, swap_flag, c);
							GMT->current.io.fmt[GMT_OUT][col].type = GMT_get_io_type (GMT, c);
						}
					}
					ncol = 0;	/* Must parse a new number for each item */
					break;
				case 'x':	/* Binary skip before/after column */
					if (col == 0)	/* Must skip BEFORE reading first data column (flag this as a negative skip) */
						GMT->current.io.fmt[id][col].skip = -ncol;	/* Number of bytes to skip */
					else	/* Skip after reading previous column (hence col-1) */
						GMT->current.io.fmt[id][col-1].skip = ncol;	/* Number of bytes to skip */
					if (!i_or_o) GMT->current.io.fmt[GMT_OUT][col-1].skip = GMT->current.io.fmt[id][col-1].skip;
					break;
				case '0':	/* Number of columns */
				case '1': case '2': case '3':
				case '4': case '5': case '6':
				case '7': case '8': case '9':
					ncol = atoi (&text[i]);
					if (ncol < 1) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -b: Column count must be > 0\n");
						return (EXIT_FAILURE);
					}
					while (text[++i] && isdigit ((int)text[i]))
						; --i; /* Wind past the digits */
					break;
				case ',':
					break;	/* Comma between sequences are optional and just ignored */
				case 'w':		/* Turn off the swap on a per-item basis unless it was set via +L|B */
					if (!endian_swab)
						swab = false;
					break;
				default:
					error = true;
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Malformed -b argument [%s]\n", text);
					GMT_syntax (GMT, 'b');
					break;
			}
		}
	}
	if (col == 0)
		col = ncol; /* Maybe we got a column count */
	GMT->common.b.ncol[id] = col;
	if (col && !set) {
		for (col = 0; col < GMT->common.b.ncol[id]; col++) {
			/* Default binary type is double */
			GMT->current.io.fmt[id][col].io   = GMT_get_io_ptr (GMT, id, swab, 'd');
			GMT->current.io.fmt[id][col].type = GMT_get_io_type (GMT, 'd');
			if (!i_or_o) {	/* Must also set output */
				GMT->current.io.fmt[GMT_OUT][col].io   = GMT_get_io_ptr (GMT, GMT_OUT, swab, 'd');
				GMT->current.io.fmt[GMT_OUT][col].type = GMT_get_io_type (GMT, 'd');
			}
		}
	}

	if (!i_or_o) {	/* Specified neither i or o so let settings apply to both */
		GMT->common.b.active[GMT_OUT] = GMT->common.b.active[GMT_IN];
		GMT->common.b.ncol[GMT_OUT] = GMT->common.b.ncol[GMT_IN];
		GMT->common.b.type[GMT_OUT] = GMT->common.b.type[GMT_IN];
		if (GMT->common.b.swab[GMT_IN] == k_swap_in) GMT->common.b.swab[GMT_OUT] = k_swap_out;
	}

	GMT_set_bin_io (GMT);	/* Make sure we point to binary i/o functions after processing -b option */

	if (p) *p = '+';	/* Restore the + sign we gobbled up earlier */
	return (error);
}

/*! . */
int gmt_parse_c_option (struct GMT_CTRL *GMT, char *arg)
{
	int i, error = 0;
	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -c requires an argument */

	i = atoi (arg);
	if (i < 1)
		error++;
	else
		GMT->PSL->init.copies = i;
	return (error);
}

/*! . */
int64_t gmt_parse_range (struct GMT_CTRL *GMT, char *p, int64_t *start, int64_t *stop)
{	/* Parses p looking for range or columns or individual columns.
	 * If neither then we just increment both start and stop. */
	int64_t inc = 1;
	int got;
	char *c = NULL;
	if ((c = strchr (p, '-'))) {	/* Range of columns given. e.g., 7-9 */
		got = sscanf (p, "%" PRIu64 "-%" PRIu64, start, stop);
		if (got != 2) inc = 0L;	/* Error flag */
	}
	else if ((c = strchr (p, ':'))) {	/* Range of columns given. e.g., 7:9 or 1:2:5 */
		got = sscanf (p, "%" PRIu64 ":%" PRIu64 ":%" PRIu64, start, &inc, stop);
		if (got == 2) { *stop = inc; inc = 1L;}	/* Just got start:stop with implicit inc = 1 */
		else if (got != 3 || inc < 1) inc = 0L;	/* Error flag */
	}
	else if (isdigit ((int)p[0]))	/* Just a single column, e.g., 3 */
		*start = *stop = atol (p);
	else				/* Just assume it goes column by column */
		(*start)++, (*stop)++;
	if ((*stop) < (*start)) inc = 0L;	/* Not good */
	if (inc == 0)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad range [%s]: col, start-stop, start:stop, or start:step:stop must yield monotonically increasing positive selections\n", p);
	return (inc);	/* Either > 0 or 0 for error */
}

/*! Routine will decode the -f[i|o]<col>|<colrange>[t|T|g],... arguments */
int gmt_parse_f_option (struct GMT_CTRL *GMT, char *arg) {

	char copy[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""};
	unsigned int k = 1, ic, pos = 0, code, *col = NULL;
	size_t len;
	int64_t i, start = -1, stop = -1, inc;
	enum GMT_enum_units unit = GMT_IS_METER;
	bool both_i_and_o = false;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -f requires an argument */

	if (arg[0] == 'i')	/* Apply to input columns only */
		col = GMT->current.io.col_type[GMT_IN];
	else if (arg[0] == 'o')	/* Apply to output columns only */
		col = GMT->current.io.col_type[GMT_OUT];
	else {			/* Apply to both input and output columns */
		both_i_and_o = true;
		k = 0;
	}

	strncpy (copy, &arg[k], GMT_BUFSIZ);	/* arg should NOT have a leading i|o part */

	if (copy[0] == 'g' || copy[0] == 'p') {	/* Got -f[i|o]g which is shorthand for -f[i|o]0x,1y, or -fp[<unit>] (see below) */
		if (both_i_and_o) {
			GMT_set_geographic (GMT, GMT_IN);
			GMT_set_geographic (GMT, GMT_OUT);
		}
		else {
			col[GMT_X] = GMT_IS_LON;
			col[GMT_Y] = GMT_IS_LAT;
		}
		pos = 1;
		start = stop = 1;
	}
	if (copy[0] == 'p') {	/* Got -f[i|o]p[<unit>] for projected floating point map coordinates (e.g., UTM meters) */
		if (copy[1] && strchr (GMT_LEN_UNITS2, copy[1])) {	/* Given a unit via -fp<unit>*/
			if ((unit = GMT_get_unit_number (GMT, copy[1])) == GMT_IS_NOUNIT) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Malformed -f argument [%s] - bad projected unit\n", arg);
				return 1;
			}
			pos++;
		}
		GMT->current.proj.inv_coordinates = true;
		GMT->current.proj.inv_coord_unit = unit;
	}

	while ((GMT_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((inc = gmt_parse_range (GMT, p, &start, &stop)) == 0) return (GMT_PARSE_ERROR);
		len = strlen (p);	/* Length of the string p */
		ic = (int) p[len-1];	/* Last char in p is the potential code T, t, x, y, or f. */
		switch (ic) {
			case 'T':	/* Absolute calendar time */
				code = GMT_IS_ABSTIME;
				break;
			case 't':	/* Relative time (units since epoch) */
				code = GMT_IS_RELTIME;
				break;
			case 'x':	/* Longitude coordinates */
				code = GMT_IS_LON;
				break;
			case 'y':	/* Latitude coordinates */
				code = GMT_IS_LAT;
				break;
			case 'f':	/* Plain floating point coordinates */
				code = GMT_IS_FLOAT;
				break;
			default:	/* No suffix, consider it an error */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Malformed -f argument [%s]\n", arg);
				return 1;
				break;
		}

		/* Now set the code for these columns */

		if (both_i_and_o)
			for (i = start; i <= stop; i += inc) GMT->current.io.col_type[GMT_IN][i] = GMT->current.io.col_type[GMT_OUT][i] = code;
		else
			for (i = start; i <= stop; i += inc) col[i] = code;
	}
	return (GMT_NOERROR);
}

/*! . */
int gmt_compare_cols (const void *point_1, const void *point_2)
{
	/* Sorts cols into ascending order  */
	if (((struct GMT_COL_INFO *)point_1)->col < ((struct GMT_COL_INFO *)point_2)->col) return (-1);
	if (((struct GMT_COL_INFO *)point_1)->col > ((struct GMT_COL_INFO *)point_2)->col) return (+1);
	return (0);
}

/*! . */
unsigned int gmt_parse_d_option (struct GMT_CTRL *GMT, char *arg)
{
	unsigned int dir, first, last;
	char *c = NULL;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -d requires an argument */
	if (arg[0] == 'i') {
		first = last = GMT_IN;
		c = &arg[1];
	}
	else if (arg[0] == 'o') {
		first = last = GMT_OUT;
		c = &arg[1];
	}
	else {
		first = GMT_IN;	last = GMT_OUT;
		c = arg;
	}

	for (dir = first; dir <= last; dir++) {
		GMT->common.d.active[dir] = true;
		GMT->common.d.nan_proxy[dir] = atof (c);
		GMT->common.d.is_zero[dir] = doubleAlmostEqualZero (0.0, GMT->common.d.nan_proxy[dir]);
	}
	return (GMT_NOERROR);
}

/*! Routine will decode the -i<col>|<colrange>[l][s<scale>][o<offset>],... arguments */
int gmt_parse_i_option (struct GMT_CTRL *GMT, char *arg) {

	char copy[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, *c = NULL;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	unsigned int k = 0, pos = 0;
	int64_t i, start = -1, stop = -1, inc;
	int convert;
	double scale, offset;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -i requires an argument */

	strncpy (copy, arg, GMT_BUFSIZ);
	if ((c = strstr (copy, "+n"))) c[0] = '\0';	/* Chop off modifier since processed earlier */
	for (i = 0; i < GMT_MAX_COLUMNS; i++) GMT->current.io.col_skip[i] = true;	/* Initially, no input column is requested */

	while ((GMT_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process it */
		convert = 0, scale = 1.0, offset = 0.0;

		if ((c = strchr (p, 'o'))) {	/* Look for offset */
			c[0] = '\0';	/* Wipe out the 'o' so that next scan terminates there */
			convert |= 1;
			offset = atof (&c[1]);
		}
		if ((c = strchr (p, 's'))) {	/* Look for scale factor */
			c[0] = '\0';	/* Wipe out the 's' so that next scan terminates there */
			if (GMT_compat_check (GMT, 4)) {	/* GMT4 */
				i = (int)strlen (p) - 1;
				convert = (p[i] == 'l') ? 2 : 1;
				i = sscanf (&c[1], "%[^/]/%[^l]", txt_a, txt_b);
				if (i == 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-i...s contains bad scale info\n");
				scale = atof (txt_a);
				if (i == 2) offset = atof (txt_b);
			} else {
				convert |= 1;
				scale = atof (&c[1]);
			}
		}
		if ((c = strchr (p, 'l'))) {	/* Look for log indicator */
			c[0] = '\0';	/* Wipe out the 's' so that next scan terminates there */
			convert = 2;
		}
		if ((inc = gmt_parse_range (GMT, p, &start, &stop)) == 0) return (GMT_PARSE_ERROR);

		/* Now set the code for these columns */

		for (i = start; i <= stop; i += inc, k++) {
			GMT->current.io.col_skip[i] = false;	/* Do not skip these */
			GMT->current.io.col[GMT_IN][k].col = (unsigned int)i;	/* Requested order of columns */
			GMT->current.io.col[GMT_IN][k].order = k;		/* Requested order of columns */
			GMT->current.io.col[GMT_IN][k].convert = convert;
			GMT->current.io.col[GMT_IN][k].scale = scale;
			GMT->current.io.col[GMT_IN][k].offset = offset;
		}
	}
	qsort (GMT->current.io.col[GMT_IN], k, sizeof (struct GMT_COL_INFO), gmt_compare_cols);
	GMT->common.i.n_cols = k;
	return (GMT_NOERROR);
}

/*! Routine will decode the -o<col>|<colrange>,... arguments */
int gmt_parse_o_option (struct GMT_CTRL *GMT, char *arg) {

	char copy[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, *c = NULL;
	unsigned int pos = 0;
	uint64_t k = 0;
	int64_t i, start = -1, stop = -1, inc;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -o requires an argument */

	strncpy (copy, arg, GMT_BUFSIZ);
	if ((c = strstr (copy, "+n"))) c[0] = '\0';	/* Chop off modifier */

	while ((GMT_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((inc = gmt_parse_range (GMT, p, &start, &stop)) == 0) return (GMT_PARSE_ERROR);

		/* Now set the code for these columns */

		for (i = start; i <= stop; i += inc, k++) {
			GMT->current.io.col[GMT_OUT][k].col = (unsigned int)i;	/* Requested order of columns */
			GMT->current.io.col[GMT_OUT][k].order = (unsigned int)k;		/* Requested order of columns */
		}
	}
	GMT->common.o.n_cols = k;
	if (GMT->common.b.active[GMT_OUT]) GMT->common.b.ncol[GMT_OUT] = GMT->common.b.ncol[GMT_IN];	/* Since -o machinery will march through */
	return (GMT_NOERROR);
}

/*! parse any --PARAM[=value] arguments */
int GMT_parse_dash_option (struct GMT_CTRL *GMT, char *text) {
	int n;
	char *this_c = NULL, message[GMT_LEN128] = {""};
	if (!text)
		return (GMT_NOERROR);

	/* print version and exit */
	if (strcmp (text, "version") == 0) {
		sprintf (message, "%s\n", GMT_PACKAGE_VERSION_WITH_SVN_REVISION);
		GMT->parent->print_func (stdout, message);
		/* cannot call GMT_Free_Options() from here, so we are leaking on exit.
		 * struct GMTAPI_CTRL *G = GMT->parent;
		 * if (GMT_Destroy_Session (G))
		 *   exit (EXIT_FAILURE); */
		exit (EXIT_SUCCESS);
	}

	/* print GMT folders and exit */
	if (strcmp (text, "show-datadir") == 0) {
		sprintf (message, "%s\n", GMT->session.SHAREDIR);
		GMT->parent->print_func (stdout, message);
		/* leaking on exit same as above. */
		exit (EXIT_SUCCESS);
	}

	if ((this_c = strchr (text, '='))) {
		/* Got --PAR=VALUE */
		this_c[0] = '\0';	/* Temporarily remove the '=' character */
		n = gmt_setparameter (GMT, text, &this_c[1]);
		this_c[0] = '=';	/* Put it back were it was */
	}
	else
		/* Got --PAR */
		n = gmt_setparameter (GMT, text, "true");
	return (n);
}

int count_xy_terms (char *txt, int64_t *xstart, int64_t *xstop, int64_t *ystart, int64_t *ystop)
{	/* Process things like xxxxyy, x4y2, etc and find the number of x and y items.
	 * We return the start=stop= number of x and ystart=ystop = number of y. */
	
	unsigned int n[2] = {0, 0};
	size_t len = strlen (txt), k = 0;
	while (k < len) {
		switch (txt[k]) {
			case 'x':
				if (isdigit (txt[k+1])) {	/* Gave things like x3y */
					n[GMT_X] += atoi (&txt[k+1]);
					k++;
					while (txt[k] && isdigit (txt[k])) k++;	/* Wind pass the number */
				}
				else {	/* Just one x */
					n[GMT_X]++;
					k++;
				}
				break;
			case 'y':
				if (isdigit (txt[k+1])) {	/* Gave things like y3x */
					n[GMT_Y] += atoi (&txt[k+1]);
					k++;
					while (txt[k] && isdigit (txt[k])) k++;	/* Wind pass the number */
				}
				else {	/* Just one y */
					n[GMT_Y]++;
					k++;
				}
				break;
			default:	/* Bad args */
				return -1;
				break;
		}
	}
	*xstart = *xstop = n[GMT_X];	/* Just a single x combo */
	*ystart = *ystop = n[GMT_Y];	/* Just a particular y combo */
	return 0;
}
#if 0
/*! . */
int GMT_parse_model (struct GMT_CTRL *GMT, char option, char *in_arg, unsigned int dim, struct GMT_MODEL *M)
{
	/* Parse -N[p|P|f|F|c|C|s|S|x|X|y|Y][x|y]<list-of-terms>[,...][+l<lengths>][+o<origins>][+r] for trend1d, trend2d, grdtrend.
	 * p means polynomial.
	 * c means cosine.  For 2-D you may optionaly add x|y to only add basis for that dimension [both]
	 * s means sine.  Optionally append x|y [both]
	 * f means both cosine and sine.    Optionally append x|y [both]
	 * list-of-terms is either a single order (e.g., 2) or a range (e.g., 0-3)
	 * Give one or more lists separated by commas.
	 * In 1-D, we add the basis x^p, cos(2*pi*p/X), and/or sin(2*pi*p/X) depending on selection.
	 * In 2-D, for polynomial the order means all p products of x^m*y^n where m+n == p
	 *   To only have some of these terms you must instead specify which ones you want,
	 *   e.g., xxxy (or x3y) and yyyx (or y3x) for just those two.
	 *   For Fourier we add these 4 terms per order:
	 *   cos(2*pi*p*x/X), sin(2*pi*p*x/X), cos(2*pi*p*y/Y), sin(2*pi*p*y/Y)
	 *   To only add basis in x or y you must apped x|y after the c|s|f.
	 * dim is either 1 (1-D) or 2 (for 2-D, grdtrend).
 	 * Indicate robust fit by appending +r
	 */
	
	unsigned int pos = 0, n_model = 0, part, n_parts, k, j;
	int64_t order, xstart, xstop, ystart, ystop, step, n_order;
	size_t end;
	bool got_intercept;
	enum GMT_enum_model kind[2];
	char p[GMT_BUFSIZ] = {""}, type, *this_range = NULL, *arg = NULL, *name = "pcs";
	
	if (!in_arg || !in_arg[0]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: No arguments given!\n", option);
		return -1;	/* No arg given */
	}
	if ((strchr (in_arg, 'x') || strchr (in_arg, 'y')) && dim == 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Cannot use x|y for 1-D basis\n", option);
		return -1;
	}
	/* Deal with backwards compatibilities: -N[f]<nmodel>[r] for 1-D and -N<nmodel>[r] for 2-D */
	arg = strdup (in_arg);
	end = strlen (arg) - 1;
	if ((isdigit (arg[0]) || (dim == 1 && end > 1 && arg[0] == 'f' && isdigit (arg[2]))) && ((arg[end] == 'r' && arg[end-1] != '+') || isdigit (arg[end])))
		/* Old GMT4-like syntax. If compatibility allows it we rewrite using new syntax so we only have one parser below */
		if (GMT_compat_check (GMT, 5)) {	/* Allow old-style syntax */
			char new[GMT_BUFSIZ] = {""};
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: -%c%s is deprecated; see usage for new syntax\n", option, arg);
			if (arg[0] != 'f') new[0] = 'p';	/* So we start with f or p */			
			if (arg[end] == 'r') {
				arg[end] = '\0';	/* Chop off the r */
				strcat (new, arg);
				strcat (new, "+r");	/* Add robust flag */
			}
			else
				strcat (new, arg);
			strcpy (arg, new);	/* Place revised args */
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Old-style arguments given and chosen compatibility mode does not allow it\n", option);
			free (arg);
			return -1;
		}
	}
	if ((c = strchr (arg, '+'))) {	/* Gave one or more modifiers */
		pos = 0;
		while ((GMT_strtok (c, "+", &pos, p))) {
			switch (c[0]) {
				case 'o':	/* Origin of axes */
					if ((k = GMT_Get_Value (GMT->parent, &c[1], M->origin)) < 1) {
						GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Error -%c: Unable to parse the +o arguments (%s)\n", option, &c[1]);
						return -1;
					}
					break;
				case 'r':
					M->robust = true;
					break;
				case 'x':
					if ((k = GMT_Get_Value (GMT->parent, &c[1], M->period)) < 1) {
						GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Error -%c: Unable to parse the +x argument (%s)\n", option, &c[1]);
						return -1;
					}
					break;
				case 'y':
					if ((k = GMT_Get_Value (GMT->parent, &c[1], &M->period[GMT_Y])) < 1) {
						GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Error -%c: Unable to parse the +y argument (%s)\n", option, &c[1]);
						return -1;
					}
					break;
				default:
					GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Error -%c: Unrecognized modifier +%s\n", option, c);
					return -1;
					break;
			}
		}
		c[0] = '\0';	/* Chop off modifiers in arg before processing settings */
	}
	pos = 0;	/* Reset position since now working on arg */
	while ((GMT_strtok (arg, ",", &pos, p))) {
		/* Here, p will be one instance of [p|f|c|s][x|y]<list-of-terms> */
		if (!strchr ("cfsp", p[0])) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad basis function type (%c)\n", option, p[0]);
			return -1;
		}
		this_range = &p[1];
		n_parts = 1;	/* Normally just one basis function at the time but f implies both c and s */
		special = false;
		switch (p[0]) {	/* What kind of basis function? */
			case 'p': kind = GMT_POLYNOMIAL; break;
			case 'c': kind = GMT_COSINE; break;
			case 's': kind = GMT_SINE; break;
			case 'f': kind = GMT_FOURIER; break;
				
		}
		if (p[1] == 'x' || p[1] == 'y')	{	/* Single building block and not all items of given order */
			special = true;
			count_xy_terms (&p[1], &xstart, &xstop, &ystart, &ystop);
		else if ((step = gmt_parse_range (GMT, this_range, &xstart, &xstop)) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad basis function order (%s)\n", option, this_range);
			return -1;
		}
		
		if (kind != GMT_POLYNOMIAL && xstart == 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Cosine|Sine cannot start with order 0.  Use p0 to add a constant\n", option);
			return -1;
		}
		/* Here we have range and kind */
		
		/* For the Fourier components we need to distinguish bewteen things like cos(x)*sin(y), sin(x)*cos(y), cos(x), etc.  We use these 8 type flags:
		   0 = C- cos (x)
		   1 = -C cos (y)
		   2 = S- sin (x)
		   3 = -S sin (y)
		   4 = CC cos (x)*cos(y)
		   5 = CS cos (x)*sin(y)
		   6 = SC sin (x)*cos(y)
		   7 = SS sin (x)*sin(y)
		 */
		for (order = xstart; order <= xstop; order++) {
			/* For each order given in the range, or just this particular order */
			switch (kind) {
				case GMT_POLYNOMIAL:	/* Add one or more polynomial basis */
					if (!special) {
						ystart = 0;
						ystop = (dim == 1) ? 0 : order;
					}
					for (k = ystart; k <= ystop; k++) {
						M->term[n_model].kind = GMT_POLYNOMIAL;
						M->term[n_model].order[GMT_X] = (unsigned int)(order - k);
						M->term[n_model].order[GMT_Y] = (unsigned int)k;
						n_model++;
						if (n_model == GMT_N_MAX_MODEL) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
							return -1;
						}
					}
					break;
				case GMT_FOURIER:	/* Add a Fourier basis (2 or 4 parts) */
					if (!special) {
						ystart = ystop = order;
					}
					for (i = 0; i < 2; i) {	/* Loop over cosine and sine in x */
						for (k = 0; k < 2; k++) {	/* Loop over cosine and sine in y */
							M->term[n_model].kind = GMT_FOURIER;
							M->term[n_model].type = 4 + 2*i + k;	/* CC, CS, SC, SS */
							M->term[n_model].order[GMT_X] = (unsigned int)order;
							M->term[n_model].order[GMT_Y] = (unsigned int)ystart;
							n_model++;
							if (n_model == GMT_N_MAX_MODEL) {
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
								return -1;
							}
						}
					}
					break;
				case GMT_COSINE:	/* Add a Cosine basis (1 or 2 parts) */
					if (!special) {
						ystart = ystop = order;
					}
					/* cosine in x? */
					if (order) {
						M->term[n_model].kind = GMT_COSINE;
						M->term[n_model].type = 0;	/* C- */
						M->term[n_model].order[GMT_X] = (unsigned int)order;
						n_model++;
						if (n_model == GMT_N_MAX_MODEL) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
							return -1;
						}
					}
					if (ystart) {
						M->term[n_model].kind = GMT_COSINE;
						M->term[n_model].type = 1;	/* -C */
						M->term[n_model].order[GMT_Y] = (unsigned int)ystart;
						n_model++;
						if (n_model == GMT_N_MAX_MODEL) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
							return -1;
						}
					}
					break;
				case GMT_SINE:	/* Add a Sine basis (1 or 2 parts) */
					if (!special) {
						ystart = ystop = order;
					}
					/* sine in x? */
					if (order) {
						M->term[n_model].kind = GMT_SINE;
						M->term[n_model].type = 2;	/* S- */
						M->term[n_model].order[GMT_X] = (unsigned int)order;
						n_model++;
						if (n_model == GMT_N_MAX_MODEL) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
							return -1;
						}
					}
					if (ystart) {
						M->term[n_model].kind = GMT_SINE;
						M->term[n_model].type = 3;	/* -S */
						M->term[n_model].order[GMT_Y] = (unsigned int)ystart;
						n_model++;
						if (n_model == GMT_N_MAX_MODEL) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
							return -1;
						}
					}
					break;
			}
		}
	}
	free (arg);
	/* Make sure there are no duplicates */
	
	for (k = 0; k < n_model; k++) {
		for (j = k+1; j < n_model; j++) {
			if (M->term[k].kind == M->term[j].kind && M->term[k].order[GMT_X] == M->term[j].order[GMT_X] && M->term[k].order[GMT_Y] == M->term[j].order[GMT_Y] && M->term[k].type == M->term[j].type) {
				if (dim == 1)
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Basis %c%u occurs more than once!\n", option, name[M->term[k].kind], M->term[k].order[GMT_X]);
				else
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Basis %cx%uy%u occurs more than once!\n", option, name[M->term[k].kind], M->term[k].order[GMT_X], M->term[k].order[GMT_Y]);
				return -1;
			}
		}
		if (M->term[k].order == 0) got_intercept = true;
	}
	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		if (!got_intercept) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning -%c: No intercept term (p0) given\n", option);
		fprintf (stderr, "Fit %u terms.  Use a robust model?: %s\n", n_model, (*robust) ? "Yes" : "No");
		for (k = 0; k < n_model; k++)
			fprintf (stderr, "Model basis %d is of type %c and order %u\n", k, name[M->term[k].kind], M->term[k].order);
	}
	return (0);
}
#endif

/*! . */
void GMT_check_lattice (struct GMT_CTRL *GMT, double *inc, unsigned int *registration, bool *active)
{	/* Uses provided settings to initialize the lattice settings from
	 * the -R<grdfile> if it was given; else it does nothing.
	 */
	if (!GMT->current.io.grd_info.active) return;	/* -R<grdfile> was not used; use existing settings */

	/* Here, -R<grdfile> was used and we will use the settings supplied by the grid file (unless overridden) */
	if (!active || *active == false) {	/* -I not set separately */
		GMT_memcpy (inc, GMT->current.io.grd_info.grd.inc, 2, double);
		inc[GMT_Y] = GMT->current.io.grd_info.grd.inc[GMT_Y];
	}
	if (registration) {	/* An pointer not NULL was passed that indicates grid registration */
		/* If a -r like option was set then toggle grid setting, else use grid setting */
		*registration = (*registration) ? !GMT->current.io.grd_info.grd.registration : GMT->current.io.grd_info.grd.registration;
	}
	if (active) *active = true;	/* When 4th arg is not NULL it is set to true (for Ctrl->active args) */
}

/*! . */
int GMT_check_binary_io (struct GMT_CTRL *GMT, uint64_t n_req) {
	int n_errors = 0;

	/* Check the binary options that are used with most GMT programs.
	 * GMT is the pointer to the GMT structure.
	 * n_req is the number of required columns. If 0 then it relies on
	 *    GMT->common.b.ncol[GMT_IN] to be non-zero.
	 * Return value is the number of errors that where found.
	 */

	if (!GMT->common.b.active[GMT_IN]) return (GMT_NOERROR);	/* Let machinery figure out input cols for ascii */

	/* These are specific tests for binary input */

	if (GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_req;
	if (GMT->common.b.ncol[GMT_IN] == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Must specify number of columns in binary input data (-bi)\n");
		n_errors++;
	}
	else if (n_req > GMT->common.b.ncol[GMT_IN]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL,
		            "Syntax error: Binary input data (-bi) provides %d but must have at least %d columns\n",
		            GMT->common.b.ncol[GMT_IN], n_req);
		n_errors++;
	}
	if (GMT->common.b.ncol[GMT_IN] < GMT->common.i.n_cols) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL,
		            "Syntax error: Binary input data (-bi) provides %d but column selection (-i) asks for %d columns\n",
		            GMT->common.b.ncol[GMT_IN], GMT->common.i.n_cols);
		n_errors++;
	}
	if (GMT->common.b.active[GMT_OUT] && GMT->common.b.ncol[GMT_OUT] && (GMT->common.b.ncol[GMT_OUT] < GMT->common.o.n_cols)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL,
		            "Syntax error: Binary output data (-bo) provides %d but column selection (-o) asks for %d columns\n",
		            GMT->common.b.ncol[GMT_OUT], GMT->common.o.n_cols);
		n_errors++;
	}

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Provides %d, expects %d-column binary data\n", GMT->common.b.ncol[GMT_IN], n_req);

	return (n_errors);
}

/*! Parse the -U option.  Full syntax: -U[<just>/<dx>/<dy>/][c|<label>] */
int gmt_parse_U_option (struct GMT_CTRL *GMT, char *item) {
	int i, just, n = 0, n_slashes, error = 0;
	char txt_j[GMT_LEN256] = {""}, txt_x[GMT_LEN256] = {""}, txt_y[GMT_LEN256] = {""};

	GMT->current.setting.map_logo = true;
	if (!item || !item[0]) return (GMT_NOERROR);	/* Just basic -U with no args */

	for (i = n_slashes = 0; item[i]; i++) {
		if (item[i] == '/') n_slashes++;	/* Count slashes to detect [<just>]/<dx>/<dy>/ presence */
	}
	if (n_slashes >= 2) {	/* Probably gave -U[<just>]/<dx>/<dy>[/<string>] */
		if (item[0] == '/') { /* No justification given */
			n = sscanf (&item[1], "%[^/]/%[^/]/%[^\n]", txt_x, txt_y, GMT->current.ps.map_logo_label);
			just = 1;	/* Default justification is LL */
		}
		else {
			n = sscanf (item, "%[^/]/%[^/]/%[^/]/%[^\n]", txt_j, txt_x, txt_y, GMT->current.ps.map_logo_label);
			just = GMT_just_decode (GMT, txt_j, GMT->current.setting.map_logo_justify);
		}
		if (just < 0) {
			/* Garbage before first slash: we simply have -U<string> */
			strncpy (GMT->current.ps.map_logo_label, item, GMT_BUFSIZ);
		}
		else {
			GMT->current.setting.map_logo_justify = just;
			GMT->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (GMT, txt_x);
			GMT->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (GMT, txt_y);
		}
	}
	else
		strncpy (GMT->current.ps.map_logo_label, item, GMT_BUFSIZ);
	if ((item[0] == '/' && n_slashes == 1) || (item[0] == '/' && n_slashes >= 2 && n < 2)) error++;
	return (error);
}

#ifdef HAVE_GLIB_GTHREAD
/*! -x+a|[-]n */
int gmt_parse_x_option (struct GMT_CTRL *GMT, char *arg) {
	char *s = NULL;

	if (!arg || !arg[0]) return (GMT_NOERROR);      /* For the time being we ignore this, but in future it may mean -x1 */
	if ((s = strstr (arg, "+a")))                     /* Use all processors */
		GMT->common.x.n_threads = GMT_get_num_processors();
	else
		GMT->common.x.n_threads = atoi(arg);

	if (GMT->common.x.n_threads == 0)
		GMT->common.x.n_threads = 1;
	else if (GMT->common.x.n_threads < 0)
		GMT->common.x.n_threads = MAX(GMT_get_num_processors() - GMT->common.x.n_threads, 1);		/* Max -n but at least one */

	return (GMT_NOERROR);
}
#endif

/*! . */
int gmt_parse_colon_option (struct GMT_CTRL *GMT, char *item) {
	int error = 0, way, off = 0;
	bool ok[2] = {false, false};
	static char *mode[4] = {"i", "o", "", ""}, *dir[2] = {"input", "output"};
	char kase = (item) ? item[0] : '\0';
	/* Parse the -: option.  Full syntax: -:[i|o].
	 * We know that if -f was given it has already been parsed due to the parsing order imposed.
	 * Must check that -: does not conflict with -f */

	switch (kase) {
		case 'i':	/* Toggle on input data only */
			ok[GMT_IN] = true;
			break;
		case 'o':	/* Toggle on output data only */
			ok[GMT_OUT] = true;
			break;
		case '\0':	/* Toggle both input and output data */
			ok[GMT_IN] = ok[GMT_OUT] = true;
			off = 2;
			break;
		default:
			error++;	/* Error */
			break;
	}
	for (way = 0; !error && way < 2; way++) if (ok[way]) {
		if (GMT->current.io.col_type[way][GMT_X] == GMT_IS_UNKNOWN && GMT->current.io.col_type[way][GMT_Y] == GMT_IS_UNKNOWN)	/* Dont know what x/y is yet */
			GMT->current.setting.io_lonlat_toggle[way] = true;
		else if (GMT->current.io.col_type[way][GMT_X] == GMT_IS_FLOAT && GMT->current.io.col_type[way][GMT_Y] == GMT_IS_FLOAT)	/* Cartesian x/y vs y/x cannot be identified */
			GMT->current.setting.io_lonlat_toggle[way] = true;
		else if (GMT_is_geographic (GMT, way))	/* Lon/lat becomes lat/lon */
			GMT->current.setting.io_lonlat_toggle[way] = true;
		else if (GMT->current.io.col_type[way][GMT_X] == GMT_IS_LAT && GMT->current.io.col_type[way][GMT_Y] == GMT_IS_LON)	/* Already lat/lon! */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: -:%s given but %s order already set by -f; -:%s ignored.\n", mode[way+off], dir[way], mode[way+off]);
#if 0
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: -:%s given but %s first two columns do not hold x/y or lon/lat\n", mode[way+off], dir[way]);
			error++;
		}
#endif
	}
	if (error) GMT->current.setting.io_lonlat_toggle[GMT_IN] = GMT->current.setting.io_lonlat_toggle[GMT_OUT] = false;	/* Leave in case we had errors */
	return (error);
}

/*! Compute reverse col-separation before mapping */
double gmt_neg_col_dist (struct GMT_CTRL *GMT, uint64_t col) {
	return (GMT->current.io.prev_rec[col] - GMT->current.io.curr_rec[col]);
}

/*! Compute forward col-separation before mapping */
double gmt_pos_col_dist (struct GMT_CTRL *GMT, uint64_t col) {
	return (GMT->current.io.curr_rec[col] - GMT->current.io.prev_rec[col]);
}

/*! Compute absolute col-separation before mapping */
double gmt_abs_col_dist (struct GMT_CTRL *GMT, uint64_t col) {
	return (fabs (GMT->current.io.curr_rec[col] - GMT->current.io.prev_rec[col]));
}

/*! Compute reverse col-separation after mapping */
double gmt_neg_col_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	double X[2][2];
	GMT_geo_to_xy (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT, GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][0] - X[col][1]);
}

/*! Compute forward col-separation after mapping */
double gmt_pos_col_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	double X[2][2];
	GMT_geo_to_xy (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT, GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][1] - X[col][0]);
}

/*! Compute forward col-separation after mapping */
double gmt_abs_col_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	double X[2][2];
	GMT_geo_to_xy (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (GMT, GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (fabs (X[col][1] - X[col][0]));
}

/*! Compute point-separation after mapping */
double gmt_xy_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	GMT_UNUSED(col);
	return (GMT_cartesian_dist_proj (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
double gmt_xy_deg_dist (struct GMT_CTRL *GMT, uint64_t col) {
	GMT_UNUSED(col);
	return (GMT_great_circle_dist_degree (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
double gmt_xy_true_dist (struct GMT_CTRL *GMT, uint64_t col) {
	GMT_UNUSED(col);
	return (GMT_great_circle_dist_meter (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
double gmt_xy_cart_dist (struct GMT_CTRL *GMT, uint64_t col) {
	GMT_UNUSED(col);
	return (GMT_cartesian_dist (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
int gmt_parse_g_option (struct GMT_CTRL *GMT, char *txt) {
	int i, k = 0, c;
	/* Process the GMT gap detection option for parameters */
	/* Syntax, e.g., -g[x|X|y|Y|d|D|[<col>]z][+|-]<gap>[d|m|s|e|f|k|M|n|c|i|p] or -ga */

	if (!txt && !txt[0]) return (GMT_PARSE_ERROR);	/* -g requires an argument */
	if ((i = GMT->common.g.n_methods) == GMT_N_GAP_METHODS) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot specify more than %d gap criteria\n", GMT_N_GAP_METHODS);
		return (1);
	}

	GMT_set_segmentheader (GMT, GMT_OUT, true);	/* -g gap checking implies -mo if not already set */

	if (txt[0] == 'a') {	/* For multiple criteria, specify that all criteria be met [default is any] */
		k++;
		GMT->common.g.match_all = true;
		if (!txt[k]) return (1);	/* Just a single -ga */
	}
	switch (txt[k]) {	/* Determine method used for gap detection */
		case 'x':	/* Difference in user's x-coordinates used for test */
			GMT->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = GMT_X;
			break;
		case 'X':	/* Difference in user's mapped x-coordinates used for test */
			GMT->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_MAP_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			GMT->common.g.col[i] = GMT_X;
			break;
		case 'y':	/* Difference in user's y-coordinates used for test */
			GMT->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = GMT_Y;
			break;
		case 'Y':	/* Difference in user's mapped y-coordinates used for test */
			GMT->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_MAP_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			GMT->common.g.col[i] = GMT_Y;
			break;
		case 'd':	/* Great circle (if geographic data) or Cartesian distance used for test */
			GMT->common.g.method[i] = (GMT_is_geographic (GMT, GMT_IN)) ? GMT_GAP_IN_GDIST : GMT_GAP_IN_CDIST;
			GMT->common.g.col[i] = -1;
			break;
		case 'D':	/* Cartesian mapped distance used for test */
			GMT->common.g.method[i] = GMT_GAP_IN_PDIST;
			GMT->common.g.col[i] = -1;
			break;
		case 'z':	/* Difference in user's z-coordinates used for test */
			GMT->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = 2;
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
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", txt[k]);
				return (1);
			}
			GMT->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = atoi (&txt[c]);
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", txt[0]);
			return (1);
			break;
	}
	switch (GMT->common.g.method[i]) {
		case GMT_NEGGAP_IN_COL:
			GMT->common.g.get_dist[i] = &gmt_neg_col_dist;
			break;
		case GMT_POSGAP_IN_COL:
			GMT->common.g.get_dist[i] = &gmt_pos_col_dist;
			break;
		case GMT_ABSGAP_IN_COL:
			GMT->common.g.get_dist[i] = &gmt_abs_col_dist;
			break;
		case GMT_NEGGAP_IN_MAP_COL:
			GMT->common.g.get_dist[i] = &gmt_neg_col_map_dist;
			break;
		case GMT_POSGAP_IN_MAP_COL:
			GMT->common.g.get_dist[i] = &gmt_pos_col_map_dist;
			break;
		case GMT_ABSGAP_IN_MAP_COL:
			GMT->common.g.get_dist[i] = &gmt_abs_col_map_dist;
			break;
		case GMT_GAP_IN_GDIST:
			GMT->common.g.get_dist[i] = &gmt_xy_true_dist;
			break;
		case GMT_GAP_IN_CDIST:
			GMT->common.g.get_dist[i] = &gmt_xy_cart_dist;
			break;
		case GMT_GAP_IN_PDIST:
			GMT->common.g.get_dist[i] = &gmt_xy_map_dist;
			break;
		default:
			break;	/* Already set, or will be reset below  */
	}
	k++;	/* Skip to start of gap value */
	if (txt[k] == '-' || txt[k] == '+') k++;	/* Skip sign */
	if ((GMT->common.g.gap[i] = atof (&txt[k])) == 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Gap value must be non-zero\n");
		return (1);
	}
	if (GMT->common.g.method[i] == GMT_GAP_IN_GDIST) {	/* Convert any gap given to meters */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'd':	/* Arc degrees, reset pointer */
				GMT->common.g.get_dist[i] = &gmt_xy_deg_dist;
				GMT->common.g.method[i] = GMT_GAP_IN_DDIST;
				break;
			case 'm':	/* Arc minutes, reset pointer */
				GMT->common.g.get_dist[i] = &gmt_xy_deg_dist;
				GMT->common.g.method[i] = GMT_GAP_IN_DDIST;
				GMT->common.g.gap[i] *= GMT_MIN2DEG;
			case 's':	/* Arc seconds, reset pointer */
				GMT->common.g.get_dist[i] = &gmt_xy_deg_dist;
				GMT->common.g.method[i] = GMT_GAP_IN_DDIST;
				GMT->common.g.gap[i] *= GMT_SEC2DEG;
			case 'f':	/* Feet  */
				GMT->common.g.gap[i] *= METERS_IN_A_FOOT;
				break;
			case 'k':	/* Km  */
				GMT->common.g.gap[i] *= 1000.0;
				break;
			case 'M':	/* Miles */
				GMT->common.g.gap[i] *= METERS_IN_A_MILE;
				break;
			case 'n':	/* Nautical miles */
				GMT->common.g.gap[i] *= METERS_IN_A_NAUTICAL_MILE;
				break;
			case 'u':	/* Survey feet  */
				GMT->common.g.gap[i] *= METERS_IN_A_SURVEY_FOOT;
				break;
			default:	/* E.g., meters or junk */
				break;
		}
	}
	else if (GMT->common.g.method[i] == GMT_GAP_IN_PDIST){	/* Cartesian plot distance stuff */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'c':	/* cm */
				GMT->common.g.gap[i] /= 2.54;
				break;
			case 'p':	/* Points */
				GMT->common.g.gap[i] /= 72.0;
				break;
			default:	/* E.g., inch or junk */
				break;
		}
	}
	if ((GMT->common.g.col[i] + 1) > GMT->common.g.n_col) GMT->common.g.n_col = GMT->common.g.col[i] + 1;	/* Needed when checking since it may otherwise not be read */
	GMT->common.g.n_methods++;
	return (GMT_NOERROR);
}

/*! Parse the -n option for 2-D grid resampling parameters -n[b|c|l|n][+a][+t<BC>][+<threshold>] */
int gmt_parse_n_option (struct GMT_CTRL *GMT, char *item)
{
	unsigned int pos = 0, j, k = 1;
	char p[GMT_LEN256] = {""};

	switch (item[0]) {
		case '+':	/* Means no mode was specified so we get the default */
			GMT->common.n.interpolant = BCR_BICUBIC; k = 0; break;
		case 'n':
			GMT->common.n.interpolant = BCR_NEARNEIGHBOR; break;
		case 'l':
			GMT->common.n.interpolant = BCR_BILINEAR; break;
		case 'b':
			GMT->common.n.interpolant = BCR_BSPLINE; break;
		case 'c':
			GMT->common.n.interpolant = BCR_BICUBIC; break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Use %s to set 2-D grid interpolation mode.\n", GMT_n_OPT);
			return (1);
			break;
	}

	/* Now look for +modifiers */

	while ((GMT_strtok (&item[k], "+", &pos, p))) {
		switch (p[0]) {
			case 'a':	/* Turn off antialias */
				GMT->common.n.antialias = false;
				break;
			case 'b':	/* Set BCs */
				GMT->common.n.bc_set = true;
				strncpy (GMT->common.n.BC, &p[1], 4U);
				for (j = 0; j < strlen (GMT->common.n.BC); j++) {
					switch (GMT->common.n.BC[j]) {
						case 'g': case 'x': case 'y': break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -n: +b<BC> requires <BC> to be g or p[x|y], n[x|y]\n");
							break;
					}
				}
				break;
			case 'c':	/* Turn on min/max clipping */
				GMT->common.n.truncate = true;
				break;
			case 't':	/* Set interpolation threshold */
				GMT->common.n.threshold = atof (&p[1]);
				if (GMT->common.n.threshold < 0.0 || GMT->common.n.threshold > 1.0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -n: Interpolation threshold must be in [0,1] range\n");
					return (1);
				}
				break;
			default:	/* Bad modifier */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Use %s to set 2-D grid interpolation mode.\n", GMT_n_OPT);
				return (1);
				break;
		}
	}
	return (GMT_NOERROR);
}

/*! . */
int gmt_parse_p_option (struct GMT_CTRL *GMT, char *item)
{
	unsigned int k, l = 0, s, pos = 0, error = 0;
	double az, el, z;
	char p[GMT_LEN256] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};

	if (!GMT->common.J.active) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning -p option works best in consort with -J (and -R or a grid)\n");
	switch (item[0]) {
		case 'x':
			GMT->current.proj.z_project.view_plane = GMT_X + GMT_ZW;
			l++;
			break;
		case 'y':
			GMT->current.proj.z_project.view_plane = GMT_Y + GMT_ZW;
			l++;
			break;
		case 'z':
			GMT->current.proj.z_project.view_plane = GMT_Z + GMT_ZW;
			l++;
			break;
		default:
			GMT->current.proj.z_project.view_plane = GMT_Z + GMT_ZW;
			break;
	}
	if ((k = sscanf (&item[l], "%lf/%lf/%lf", &az, &el, &z)) < 2) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -p (%s): Syntax is %s\n", item, GMT_p_OPT);
		return 1;
	}
	if (el <= 0.0 || el > 90.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -p option: Elevation must be in 0-90 range\n");
		return 1;
	}
	GMT->current.proj.z_project.view_azimuth = az;
	GMT->current.proj.z_project.view_elevation = el;
	if (k == 3) GMT->current.proj.z_level = z;

	for (s = 0; item[s] && item[s] != '/'; s++);	/* Look for position of slash / */
	for (k = 0; item[k] && item[k] != '+'; k++);	/* Look for +<options> strings */
	if (!item[k] || k < s) return 0;		/* No + before the slash, so we are done here */

	/* Decode +separated substrings */

	GMT->current.proj.z_project.fixed = true;
	k++;
	if (!item[k]) return 0;	/* No specific settings given, we will apply default values in 3D init */
	while ((GMT_strtok (&item[k], "+", &pos, p))) {
		switch (p[0]) {
			case 'v':	/* Specify fixed view point in 2-D projected coordinates */
				if (sscanf (&p[1], "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -p (%s): Syntax is -p<az>/<el>[/<z>][+wlon0/lat0[/z0]][+vx0[%s]/y0[%s]]\n", p, GMT_DIM_UNITS, GMT_DIM_UNITS);
					return 1;
				}
				GMT->current.proj.z_project.view_x = GMT_to_inch (GMT, txt_a);
				GMT->current.proj.z_project.view_y = GMT_to_inch (GMT, txt_b);
				GMT->current.proj.z_project.view_given = true;
				break;
			case 'w':	/* Specify fixed World point in user's coordinates */
				if (sscanf (&p[1], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c) < 2) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -p: (%s)  Syntax is -p<az>/<el>[/<z>][+wlon0/lat0[/z0]][+vx0[%s]/y0[%s]]\n", p, GMT_DIM_UNITS, GMT_DIM_UNITS);
					return 1;
				}
				error += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &GMT->current.proj.z_project.world_x), txt_a);
				error += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &GMT->current.proj.z_project.world_y), txt_b);
				if (k == 3) error += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf (GMT, txt_c, GMT->current.io.col_type[GMT_IN][GMT_Z], &GMT->current.proj.z_project.world_z), txt_c);
				GMT->current.proj.z_project.world_given = true;
				break;
			default:	/* If followed by an integer we assume this might be an exponential notation picked up by mistake */
				if (!isdigit ((int)p[0])) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning -p: Unrecognized modifier %s (ignored)\n", p);
				break;
		}
	}
	return (error);
}

/*! . */
bool gmt_parse_s_option (struct GMT_CTRL *GMT, char *item) {
	unsigned int error = 0, n, pos = 0;
	int64_t i, start = -1, stop = -1, inc;
	char p[GMT_BUFSIZ] = {""}, tmp[GMT_MAX_COLUMNS] = {""};
	/* Parse the -s option.  Full syntax: -s[<cols>][r|a] */

	GMT_memset (GMT->current.io.io_nan_col, GMT_MAX_COLUMNS, int);
	GMT->current.io.io_nan_col[0] = GMT_Z;	/* The default is to examine the z-column */
	GMT->current.io.io_nan_ncols = 1;		/* Default is that single z column */
	GMT->current.setting.io_nan_mode = GMT_IO_NAN_SKIP;	/* Plain -s */
	if (!item || !item[0]) return (false);	/* Nothing more to do */
	n = (int)strlen (item);
	if (item[n-1] == 'a') GMT->current.setting.io_nan_mode = GMT_IO_NAN_ONE, n--;		/* Set -sa */
	else if (item[n-1] == 'r') GMT->current.setting.io_nan_mode = GMT_IO_NAN_KEEP, n--;	/* Set -sr */
	if (n == 0) return (false);		/* No column arguments to process */
	/* Here we have user-supplied column information */
	for (i = 0; i < GMT_MAX_COLUMNS; i++) tmp[i] = -1;
	while (!error && (GMT_strtok (item, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((inc = gmt_parse_range (GMT, p, &start, &stop)) == 0) return (true);

		/* Now set the code for these columns */
		for (i = start; i <= stop; i += inc) tmp[i] = true;
	}
	/* Count and set array of NaN-columns */
	for (i = n = 0; i < GMT_MAX_COLUMNS; i++) if (tmp[i] != -1) GMT->current.io.io_nan_col[n++] = (unsigned int)i;
	if (error || n == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -s option: Unable to decode columns from %s\n", item);
		return true;
	}
	GMT->current.io.io_nan_ncols = n;

	return (false);
}

/*! . */
int GMT_get_V (char arg) {
	int mode = GMT_MSG_QUIET;
	switch (arg) {
		case 'q': case '0': mode = GMT_MSG_QUIET;   break;
		case 'n':           mode = GMT_MSG_NORMAL;  break;
		case 't':           mode = GMT_MSG_TICTOC;  break;
		case 'c': case '1': mode = GMT_MSG_COMPAT;  break;
		case 'v': case '2': case '\0': mode = GMT_MSG_VERBOSE; break;
		case 'l': case '3': mode = GMT_MSG_LONG_VERBOSE; break;
		case 'd': case '4': mode = GMT_MSG_DEBUG;   break;
		default: mode = -1;
	}
	return mode;
}

/*! . */
int gmt_parse_V_option (struct GMT_CTRL *GMT, char arg) {
	int mode = GMT_get_V (arg);
	if (mode < 0) return true;	/* Error in parsing */
	GMT->current.setting.verbose = (unsigned int)mode;
	return false;
}

/*! Check that special map-related codes are present - if not give warning */
void gmt_verify_encodings (struct GMT_CTRL *GMT) {

	/* First check for degree symbol */

	if (GMT->current.setting.ps_encoding.code[gmt_ring] == 32 && GMT->current.setting.ps_encoding.code[gmt_degree] == 32) {	/* Neither /ring or /degree encoded */
		GMT_message (GMT, "Warning: Selected character encoding does not have suitable degree symbol - will use space instead\n");
	}
	else if (GMT->current.setting.map_degree_symbol == gmt_ring && GMT->current.setting.ps_encoding.code[gmt_ring] == 32) {		/* want /ring but only /degree is encoded */
		GMT_message (GMT, "Warning: Selected character encoding does not have ring symbol - will use degree symbol instead\n");
		GMT->current.setting.map_degree_symbol = gmt_degree;
	}
	else if (GMT->current.setting.map_degree_symbol == gmt_degree && GMT->current.setting.ps_encoding.code[gmt_degree] == 32) {	/* want /degree but only /ring is encoded */
		GMT_message (GMT, "Warning: Selected character encoding does not have degree symbol - will use ring symbol instead\n");
		GMT->current.setting.map_degree_symbol = gmt_ring;
	}

	/* Then single quote for minute symbol... */

	if (GMT->current.setting.map_degree_symbol < 2 && GMT->current.setting.ps_encoding.code[gmt_squote] == 32) {
		GMT_message (GMT, "Warning: Selected character encoding does not have minute symbol (single quote) - will use space instead\n");
	}

	/* ... and double quote for second symbol */

	if (GMT->current.setting.map_degree_symbol < 2 && GMT->current.setting.ps_encoding.code[gmt_dquote] == 32) {
		GMT_message (GMT, "Warning: Selected character encoding does not have second symbol (double quote) - will use space instead\n");
	}
}

/*! . */
int GMT_loaddefaults (struct GMT_CTRL *GMT, char *file)
{
	static int gmt_version_major = GMT_PACKAGE_VERSION_MAJOR;
	unsigned int error = 0, rec = 0;
	char line[GMT_BUFSIZ] = {""}, keyword[GMT_LEN256] = {""}, value[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (file, "r")) == NULL) return (-1);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Reading GMT Default parameters from file: %s\n", file);
	while (fgets (line, GMT_BUFSIZ, fp)) {
		rec++;
		GMT_chop (line); /* Get rid of [\r]\n */
		if (rec == 1 && (line[0] == 'S' || line[0] == 'U'))	{	/* An old GMT4 gmt.conf got in the way */
			fclose (fp);
			return (GMT_NOERROR);
		}

		if (rec != 2) { /* Nothing */ }
		else if (strlen (line) < 7 || strtol (&line[6], NULL, 10) != gmt_version_major )
			GMT_message (GMT, "Warning: Your gmt.conf file (%s) may not be GMT %d compatible\n", file, gmt_version_major);
		else if (!strncmp (&line[6], "5.0.0", 5))
			GMT_message (GMT, "Warning: Your gmt.conf file (%s) is of version 5.0.0 and may need to be updated. Use \"gmtset -G%s\"\n", file, file);
		if (line[0] == '#') continue;	/* Skip comments */
		if (line[0] == '\0') continue;	/* Skip Blank lines */

		keyword[0] = value[0] = '\0';	/* Initialize */
		sscanf (line, "%s = %[^\n]", keyword, value);

		error += gmt_setparameter (GMT, keyword, value);
	}

	fclose (fp);
	gmt_verify_encodings (GMT);

	if (error) GMT_message (GMT, "Warning: %d GMT Defaults conversion errors in file %s!\n", error, file);

	return (GMT_NOERROR);
}

/*! . */
unsigned int GMT_setdefaults (struct GMT_CTRL *GMT, struct GMT_OPTION *options)
{
	unsigned int p, n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	char *param = NULL;

	for (opt = options; opt; opt = opt->next) {
		if (!(opt->option == '<' || opt->option == '#') || !opt->arg) continue;		/* Skip other and empty options */
		if (!strcmp (opt->arg, "=")) continue;			/* User forgot and gave parameter = value (3 words) */
		if (opt->arg[0] != '=' && strchr (opt->arg, '=')) {	/* User forgot and gave parameter=value (1 word) */
			p = 0;
			while (opt->arg[p] && opt->arg[p] != '=') p++;
			opt->arg[p] = '\0';	/* Temporarily remove the equal sign */
			n_errors += gmt_setparameter (GMT, opt->arg, &opt->arg[p+1]);
			opt->arg[p] = '=';	/* Restore the equal sign */
		}
		else if (!param)			/* Keep parameter name */
			param = opt->arg;
		else {					/* This must be value */
			n_errors += gmt_setparameter (GMT, param, opt->arg);
			param = NULL;	/* Get ready for next parameter */
		}
	}

	if (param != NULL)	/* param should be NULL unless no value were added */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Last GMT Defaults parameter from command options had no value\n");

	if (n_errors) GMT_Report (GMT->parent, GMT_MSG_NORMAL, " %d GMT Defaults conversion errors from command options\n", n_errors);
	return (n_errors);
}

/*! . */
bool gmt_true_false_or_error (char *value, bool *answer)
{
	/* Assigns false or true to answer, depending on whether value is false or true.
	 * answer = false, when value is "f", "false" or "0"
	 * answer = true, when value is "t", "true" or "1"
	 * In either case, the function returns false as exit code.
	 * When value is something else, answer is not altered and true is return as error.
	 */

	if (!strcmp (value, "true") || !strcmp (value, "t") || !strcmp (value, "1")) {	/* true */
		*answer = true;
		return (false);
	}
	if (!strcmp (value, "false") || !strcmp (value, "f") || !strcmp (value, "0")) {	/* false */
		*answer = false;
		return (false);
	}

	/* Got neither true or false.  Make no assignment and return true for error */

	return (true);
}

/*! . */
int gmt_get_language (struct GMT_CTRL *GMT)
{
	FILE *fp = NULL;
	char file[GMT_BUFSIZ] = {""}, line[GMT_BUFSIZ] = {""}, full[16] = {""}, abbrev[16] = {""}, c[16] = {""}, dwu;
	char *months[12];

	int i, nm = 0, nw = 0, nu = 0, nc = 0;

	GMT_memset (months, 12, char *);

	sprintf (line, "gmt_%s", GMT->current.setting.language);
	GMT_getsharepath (GMT, "localization", line, ".locale", file, R_OK);
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Could not load language %s - revert to us (English)!\n", GMT->current.setting.language);
		GMT_getsharepath (GMT, "localization", "gmt_us", ".locale", file, R_OK);
		if ((fp = fopen (file, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not find %s!\n", file);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		strcpy (GMT->current.setting.language, "us");
	}

	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		sscanf (line, "%c %d %s %s %s", &dwu, &i, full, abbrev, c);
		if (dwu == 'M') {	/* Month record */
			strncpy (GMT->current.language.month_name[0][i-1], full, 16U);
			strncpy (GMT->current.language.month_name[1][i-1], abbrev, 16U);
			strncpy (GMT->current.language.month_name[2][i-1], c, 16U);
			GMT_str_toupper(abbrev);
			strncpy (GMT->current.language.month_name[3][i-1], abbrev, 16U);
			nm += i;
		}
		else if (dwu == 'W') {	/* Weekday record */
			strncpy (GMT->current.language.day_name[0][i-1], full, 16U);
			strncpy (GMT->current.language.day_name[1][i-1], abbrev, 16U);
			strncpy (GMT->current.language.day_name[2][i-1], c, 16U);
			nw += i;
		}
		else if (dwu == 'U') {			/* Week name record */
			strncpy (GMT->current.language.week_name[0], full, 16U);
			strncpy (GMT->current.language.week_name[1], abbrev, 16U);
			strncpy (GMT->current.language.week_name[2], c, 16U);
			nu += i;
		}
		else {	/* Compass name record */
			strncpy (GMT->current.language.cardinal_name[0][i-1], full, 16U);
			strncpy (GMT->current.language.cardinal_name[1][i-1], abbrev, 16U);
			strncpy (GMT->current.language.cardinal_name[2][i-1], c, 16U);
			nc += i;
		}
	}
	fclose (fp);
	if (! (nm == 78 && nw == 28 && nu == 1 && nc == 10)) {	/* Sums of 1-12, 1-7, 1, and 1-4, respectively */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Mismatch between expected and actual contents in %s!\n", file);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	return (GMT_NOERROR);
}

/*! . */
unsigned int gmt_key_lookup (char *name, char **list, unsigned int n) {
	unsigned int i;

	for (i = 0; i < n && strcmp (name, list[i]); i++);
	return (i);
}

/*! . */
void gmt_free_user_media (struct GMT_CTRL *GMT) {	/* Free any user-specified media formats */
	unsigned int i;

	if (GMT->session.n_user_media == 0) return;	/* Nothing to free */

	for (i = 0; i < GMT->session.n_user_media; i++) {
		free (GMT->session.user_media_name[i]);
		GMT->session.user_media_name[i] = NULL;
	}
	GMT_free (GMT, GMT->session.user_media_name);
	GMT_free (GMT, GMT->session.user_media);
	GMT->session.n_user_media = 0;
}

/*! . */
unsigned int gmt_load_user_media (struct GMT_CTRL *GMT) {	/* Load any user-specified media formats */
	size_t n_alloc = 0;
	unsigned int n = 0;
	double w, h;
	char line[GMT_BUFSIZ] = {""}, file[GMT_BUFSIZ] = {""}, media[GMT_LEN64] = {""};
	FILE *fp = NULL;

	GMT_getsharepath (GMT, "conf", "gmt_custom_media", ".conf", file, R_OK);
	if ((fp = fopen (file, "r")) == NULL) return (0);

	gmt_free_user_media (GMT);	/* Free any previously allocated user-specified media formats */
	GMT_set_meminc (GMT, GMT_TINY_CHUNK);	/* Only allocate a small amount */
	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments and blank lines */

		if (sscanf (line, "%s %lg %lg", media, &w, &h) != 3) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding file %s.  Bad format? [%s]\n", file, line);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}

		GMT_str_tolower (media);	/* Convert string to lower case */

		if (n == n_alloc) {
			size_t k = n_alloc;	/* So we don't update n_alloc in the first GMT_malloc call */
			GMT->session.user_media = GMT_malloc (GMT, GMT->session.user_media, n, &k, struct GMT_MEDIA);
			GMT->session.user_media_name = GMT_malloc (GMT, GMT->session.user_media_name, n, &n_alloc, char *);
		}
		GMT->session.user_media_name[n] = strdup (media);
		GMT->session.user_media[n].width  = w;
		GMT->session.user_media[n].height = h;
		n++;
	}
	fclose (fp);

	n_alloc = n;
	GMT->session.user_media = GMT_malloc (GMT, GMT->session.user_media, 0, &n_alloc, struct GMT_MEDIA);
	GMT->session.user_media_name = GMT_malloc (GMT, GMT->session.user_media_name, 0, &n_alloc, char *);
	GMT_reset_meminc (GMT);

	GMT->session.n_user_media = n;

	return (n);
}

/*! Load a PostScript encoding from a file, given the filename.
 * Use Brute Force and Ignorance.
 */
int gmt_load_encoding (struct GMT_CTRL *GMT)
{
	char line[GMT_LEN256] = {""}, symbol[GMT_LEN256] = {""};
	unsigned int code = 0, pos;
	FILE *in = NULL;
	struct GMT_ENCODING *enc = &GMT->current.setting.ps_encoding;

	GMT_getsharepath (GMT, "pslib", enc->name, ".ps", line, R_OK);
	if ((in = fopen (line, "r")) == NULL) {
		perror (line);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}

	while (fgets (line, GMT_LEN256, in))
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

/*! . */
int gmt4_decode_wesnz (struct GMT_CTRL *GMT, const char *in, unsigned int side[], bool *draw_box, int part) {
	/* Scans the WESNZwesnz+ flags at the end of string "in" and sets the side/drawbox parameters
	 * and returns the length of the remaining string.  Assumes any +g<fill> has been removed from in.
	 */

	int i, k;
	bool go = true;

	GMT->current.map.frame.set_frame[part]++;
	if (GMT->current.map.frame.set_frame[0] > 1 || GMT->current.map.frame.set_frame[1] > 1) {
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error -B: <WESN-framesettings> given more than once!\n");
		return (1);
	}
	i = (int)strlen (in);
	if (i == 0) return (0);

	for (k = 0, i--; go && i >= 0 && strchr ("WESNZwesnz+", in[i]); i--) {
		if (k == 0 && part == 0) {	/* Wipe out default values when the first flag is found */
			for (k = 0; k < 5; k++) side[k] = 0;
			*draw_box = false;
		}
		if (in[i] == 's') {	/* Since s can mean both "draw south axis" and "seconds", check futher */
			if (side[S_SIDE]) go = false;	/* If S was set already then s probably means seconds */
			else if (i && in[i-1] == ',') go = true;	/* Special case of ,s to indicate south, e.g. -B30,s */
			else if (i && (in[i-1] == '.' || isdigit ((int)in[i-1]))) go = false;	/* Probably seconds, e.g. -B30s */
			if (!go) { i++; continue; }	/* Break out of loop */
		}
		switch (in[i]) {
			/* Draw AND Annotate */
			case 'W': side[W_SIDE] |= 3; break;
			case 'E': side[E_SIDE] |= 3; break;
			case 'S': side[S_SIDE] |= 3; break;
			case 'N': side[N_SIDE] |= 3; break;
			case 'Z': side[Z_SIDE] |= 3; break;
			/* Just Draw */
			case 'w': side[W_SIDE] |= 1; break;
			case 'e': side[E_SIDE] |= 1; break;
			case 's': side[S_SIDE] |= 1; break;
			case 'n': side[N_SIDE] |= 1; break;
			case 'z': side[Z_SIDE] |= 1; break;
			/* Draw 3-D box */
			case '+': *draw_box = true; break;
		}
	}
	if (i >= 0 && in[i] == ',') i--;	/* Special case for -BCcustomfile,WESNwesn to avoid the filename being parsed for WESN */

	return (i+1);	/* Return remaining string length */
}

/*! Scans the WESNZ[1234]wesnz[1234] flags and sets the side/drawbox parameters
 * and returns the length of the remaining string.
 */
int gmt5_decode_wesnz (struct GMT_CTRL *GMT, const char *in, bool check) {

	unsigned int k, error = 0, f_side[5] = {0, 0, 0, 0, 0}, z_axis[4] = {0, 0, 0, 0};
	bool s_given = false;
	if (check) {	/* true if coming via -B, false if parsing gmt.conf */
		GMT->current.map.frame.set_frame[0]++, GMT->current.map.frame.set_frame[1]++;
		if (GMT->current.map.frame.set_frame[0] > 1 || GMT->current.map.frame.set_frame[1] > 1) {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error -B: <WESN-framesettings> given more than once!\n");
			return (1);
		}
	}
	for (k = 0; in[k]; k++) {
		switch (in[k]) {
			/* Draw AND Annotate */
			case 'W': f_side[W_SIDE] |= 3; s_given = true; break;
			case 'E': f_side[E_SIDE] |= 3; s_given = true; break;
			case 'S': f_side[S_SIDE] |= 3; s_given = true; break;
			case 'N': f_side[N_SIDE] |= 3; s_given = true; break;
			case 'Z': f_side[Z_SIDE] |= 3; s_given = true; break;
			/* Just Draw */
			case 'w': f_side[W_SIDE] |= 1; s_given = true; break;
			case 'e': f_side[E_SIDE] |= 1; s_given = true; break;
			case 's': f_side[S_SIDE] |= 1; s_given = true; break;
			case 'n': f_side[N_SIDE] |= 1; s_given = true; break;
			case 'z': f_side[Z_SIDE] |= 1; s_given = true; break;
			/* Draw 3-D box */
			case '+':
				if (in[k+1] == 'b')	/* Got +b appended to MAP_FRAME_AXES, possibly */
					GMT->current.map.frame.draw_box = true;
				else if (in[k+1] == 'n')	/* Got +n appended to MAP_FRAME_AXES, means no frame nor annotations desired */
					GMT->current.map.frame.no_frame = true;
				else if (GMT_compat_check (GMT, 4)) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Modifier + in MAP_FRAME_AXES is deprecated; use +b instead.\n");
					GMT->current.map.frame.draw_box = true;
				}
				else {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error: Modifier + in MAP_FRAME_AXES not recognized.\n");
					error++;
				}
				break;
			case '1': if (f_side[Z_SIDE]) z_axis[0] = 1; else error++; break;
			case '2': if (f_side[Z_SIDE]) z_axis[1] = 1; else error++; break;
			case '3': if (f_side[Z_SIDE]) z_axis[2] = 1; else error++; break;
			case '4': if (f_side[Z_SIDE]) z_axis[3] = 1; else error++; break;
			default:
				error++;
		}
	}
	if (s_given) {
		GMT_memcpy (GMT->current.map.frame.side, f_side, 5, unsigned int);	/* Overwrite the GMT defaults */
		GMT->current.map.frame.no_frame = false;
	}
	if (GMT->current.map.frame.no_frame) GMT_memset (GMT->current.map.frame.side, 5, unsigned int);	/* Set all to nothing */
	if (z_axis[0] || z_axis[1] || z_axis[2] || z_axis[3]) GMT_memcpy (GMT->current.map.frame.z_axis, z_axis, 4, unsigned int);	/* Overwrite the GMT defaults */
	return (error);
}

void gmt_reset_colformats (struct GMT_CTRL *GMT)
{
	unsigned int i;
	for (i = 0; i < GMT_MAX_COLUMNS; i++) if (GMT->current.io.o_format[i]) {
		free (GMT->current.io.o_format[i]);
		GMT->current.io.o_format[i] = NULL;
	}
}

/*! . */
void gmt_parse_format_float_out (struct GMT_CTRL *GMT, char *value) {

	unsigned int pos = 0, col = 0, k;
	char fmt[GMT_LEN64] = {""};
	strncpy (GMT->current.setting.format_float_out_orig, value, GMT_LEN256);
	if (strchr (value, ',')) {
		unsigned int start = 0, stop = 0, error = 0;
		char *p = NULL;
		/* Look for multiple comma-separated format statements of type [<cols>:]<format>.
		 * Last format also becomes the default for unspecified columns */
		gmt_reset_colformats (GMT);	/* Wipe previous settings */
		while ((GMT_strtok (value, ",", &pos, fmt))) {
			if ((p = strchr (fmt, ':'))) {	/* Must decode which columns */
				if (strchr (fmt, '-'))	/* Range of columns given. e.g., 7-9 */
					sscanf (fmt, "%d-%d", &start, &stop);
				else if (isdigit ((int)fmt[0]))	/* Just a single column, e.g., 3 */
					start = stop = atoi (fmt);
				else				/* Something bad */
					error++;
				p++;	/* Move to format */
				for (k = start; k <= stop; k++)
					GMT->current.io.o_format[k] = strdup (p);
				if (stop > col) col = stop;	/* Retain last column set */
			}
		}
		strncpy (GMT->current.setting.format_float_out, GMT->current.io.o_format[col], GMT_LEN64);
	}
	else if (strchr (value, ' ')) {
		/* Look for N space-separated format statements of type <format1> <format2> <format3> ...
		 * and let these apply to the first N output columns.
		 * Last format also becomes the default for unspecified columns. */
		gmt_reset_colformats (GMT);	/* Wipe previous settings */
		k = 0;
		while ((GMT_strtok (value, " ", &pos, fmt)))
			GMT->current.io.o_format[k++] = strdup (fmt);
		strncpy (GMT->current.setting.format_float_out, GMT->current.io.o_format[k-1], GMT_LEN64);
	}
	else {	/* No columns, set the default format */
		gmt_reset_colformats (GMT);	/* Wipe previous settings */
		strncpy (GMT->current.setting.format_float_out, value, GMT_LEN64);
	}
}

/*! . */
bool gmt_badvalreport (struct GMT_CTRL *GMT, char *keyword) {
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized keyword %s. You may have been using a deprecated GMT3 or GMT4 keyword.\nChange keyword or use with GMT_COMPATIBILITY=4. " GMT_COMPAT_INFO, keyword);
	return (true);
}

/*! . */
unsigned int gmt_setparameter (struct GMT_CTRL *GMT, char *keyword, char *value)
{
	unsigned int pos;
	size_t len;
	int i, ival, case_val, manual;
	bool error = false, tf_answer;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, lower_value[GMT_BUFSIZ] = {""};

	double dval;

	if (!value) return (1);		/* value argument missing */
	strncpy (lower_value, value, GMT_BUFSIZ);	/* Get a lower case version */
	GMT_str_tolower (lower_value);
	len = strlen (value);

	case_val = GMT_hash_lookup (GMT, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		/* FORMAT GROUP */
		case GMTCASE_INPUT_CLOCK_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_CLOCK_IN");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_CLOCK_IN:
			strncpy (GMT->current.setting.format_clock_in, value, GMT_LEN64);
			gmt_clock_C_format (GMT, GMT->current.setting.format_clock_in, &GMT->current.io.clock_input, 0);
			break;
		case GMTCASE_INPUT_DATE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_DATE_IN");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_DATE_IN:
			strncpy (GMT->current.setting.format_date_in, value, GMT_LEN64);
			gmt_date_C_format (GMT, GMT->current.setting.format_date_in, &GMT->current.io.date_input, 0);
			break;
		case GMTCASE_OUTPUT_CLOCK_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_CLOCK_OUT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_CLOCK_OUT:
			strncpy (GMT->current.setting.format_clock_out, value, GMT_LEN64);
			gmt_clock_C_format (GMT, GMT->current.setting.format_clock_out, &GMT->current.io.clock_output, 1);
			break;
		case GMTCASE_OUTPUT_DATE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_DATE_OUT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_DATE_OUT:
			strncpy (GMT->current.setting.format_date_out, value, GMT_LEN64);
			gmt_date_C_format (GMT, GMT->current.setting.format_date_out, &GMT->current.io.date_output, 1);
			break;
		case GMTCASE_OUTPUT_DEGREE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_GEO_OUT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_GEO_OUT:
			strncpy (GMT->current.setting.format_geo_out, value, GMT_LEN64);
			gmt_geo_C_format (GMT);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of GMT_begin */
			break;
		case GMTCASE_PLOT_CLOCK_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_CLOCK_MAP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_CLOCK_MAP:
			strncpy (GMT->current.setting.format_clock_map, value, GMT_LEN64);
			gmt_clock_C_format (GMT, GMT->current.setting.format_clock_map, &GMT->current.plot.calclock.clock, 2);
			break;
		case GMTCASE_PLOT_DATE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_DATE_MAP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_DATE_MAP:
			strncpy (GMT->current.setting.format_date_map, value, GMT_LEN64);
			gmt_date_C_format (GMT, GMT->current.setting.format_date_map, &GMT->current.plot.calclock.date, 2);
			break;
		case GMTCASE_PLOT_DEGREE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_GEO_MAP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_GEO_MAP:
			strncpy (GMT->current.setting.format_geo_map, value, GMT_LEN64);
			gmt_plot_C_format (GMT);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of GMT_begin */
			break;
		case GMTCASE_TIME_FORMAT_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_TIME_PRIMARY_MAP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_TIME_MAP:
			strncpy (GMT->current.setting.format_time[1], value, GMT_LEN64);
		case GMTCASE_FORMAT_TIME_PRIMARY_MAP:
			strncpy (GMT->current.setting.format_time[0], value, GMT_LEN64);
			break;
		case GMTCASE_TIME_FORMAT_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_TIME_SECONDARY_MAP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_TIME_SECONDARY_MAP:
			strncpy (GMT->current.setting.format_time[1], value, GMT_LEN64);
			break;
		case GMTCASE_D_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_FLOAT_OUT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_FLOAT_OUT:
			gmt_parse_format_float_out (GMT, value);
			break;
		case GMTCASE_FORMAT_FLOAT_MAP:
			strncpy (GMT->current.setting.format_float_map, value, GMT_LEN64);
			break;
		case GMTCASE_UNIX_TIME_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FORMAT_TIME_STAMP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FORMAT_TIME_STAMP:
			strncpy (GMT->current.setting.format_time_stamp, value, GMT_LEN256);
			break;

		/* FONT GROUP */

		case GMTCASE_FONT:	/* Special to set all fonts */
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_annot[0])) error = true;
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_annot[1])) error = true;
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_title)) error = true;
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_label)) error = true;
			/* if (GMT_getfont (GMT, value, &GMT->current.setting.font_logo)) error = true; */
			break;
		case GMTCASE_ANNOT_FONT_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_ANNOT_PRIMARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FONT_ANNOT:
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_annot[1])) error = true;
		case GMTCASE_FONT_ANNOT_PRIMARY:
			if (value[0] == '+') {
				/* When + is prepended, scale fonts, offsets and ticklengths relative to FONT_ANNOT_PRIMARY (except LOGO font) */
				double scale;
				scale = GMT->current.setting.font_annot[0].size;
				if (GMT_getfont (GMT, &value[1], &GMT->current.setting.font_annot[0])) error = true;
				scale = GMT->current.setting.font_annot[0].size / scale;
				GMT->current.setting.font_annot[1].size *= scale;
				GMT->current.setting.font_label.size *= scale;
				GMT->current.setting.font_title.size *= scale;
				GMT->current.setting.map_annot_offset[0] *= scale;
				GMT->current.setting.map_annot_offset[1] *= scale;
				GMT->current.setting.map_label_offset *= scale;
				GMT->current.setting.map_title_offset *= scale;
				GMT->current.setting.map_frame_width *= scale;
				GMT->current.setting.map_tick_length[0] *= scale;
				GMT->current.setting.map_tick_length[1] *= scale;
				GMT->current.setting.map_tick_length[2] *= scale;
				GMT->current.setting.map_tick_length[3] *= scale;
			}
			else
				if (GMT_getfont (GMT, value, &GMT->current.setting.font_annot[0])) error = true;
			break;
		case GMTCASE_ANNOT_FONT_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_ANNOT_SECONDARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FONT_ANNOT_SECONDARY:
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_annot[1])) error = true;
			break;
		case GMTCASE_HEADER_FONT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_TITLE");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FONT_TITLE:
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_title)) error = true;
			break;
		case GMTCASE_LABEL_FONT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_LABEL");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_FONT_LABEL:
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_label)) error = true;
			break;
		case GMTCASE_FONT_LOGO:
			if (GMT_getfont (GMT, value, &GMT->current.setting.font_logo)) error = true;
			break;

		/* FONT GROUP ... obsolete options */

		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_ANNOT_PRIMARY");
				dval = GMT_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0)
					GMT->current.setting.font_annot[0].size = dval;
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_ANNOT_SECONDARY");
				dval = GMT_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0)
					GMT->current.setting.font_annot[1].size = dval;
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_HEADER_FONT_SIZE:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_TITLE");
				dval = GMT_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0)
					GMT->current.setting.font_title.size = dval;
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_LABEL_FONT_SIZE:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("FONT_LABEL");
				dval = GMT_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0)
					GMT->current.setting.font_label.size = dval;
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;

		/* MAP GROUP */

		case GMTCASE_ANNOT_OFFSET_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ANNOT_OFFSET_PRIMARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ANNOT_OFFSET:
			GMT->current.setting.map_annot_offset[1] = GMT_to_inch (GMT, value);
		case GMTCASE_MAP_ANNOT_OFFSET_PRIMARY:
			GMT->current.setting.map_annot_offset[0] = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_ANNOT_OFFSET_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ANNOT_OFFSET_SECONDARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ANNOT_OFFSET_SECONDARY:
			GMT->current.setting.map_annot_offset[1] = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_OBLIQUE_ANNOTATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ANNOT_OBLIQUE");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ANNOT_OBLIQUE:
			ival = atoi (value);
			if (ival >= 0 && ival < 64)
				GMT->current.setting.map_annot_oblique = ival;
			else
				error = true;
			break;
		case GMTCASE_ANNOT_MIN_ANGLE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ANNOT_MIN_ANGLE");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ANNOT_MIN_ANGLE:
			dval = atof (value);
			if (dval < 0.0)
				error = true;
			else
				GMT->current.setting.map_annot_min_angle = dval;
			break;
		case GMTCASE_ANNOT_MIN_SPACING:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ANNOT_MIN_SPACING");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ANNOT_MIN_SPACING:
			if (value[0] == '-')	/* Negative */
				error = true;
			else
				GMT->current.setting.map_annot_min_spacing = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_Y_AXIS_TYPE:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ANNOT_ORTHO");
				if (!strcmp (lower_value, "ver_text"))
					strncpy (GMT->current.setting.map_annot_ortho, "", 5U);
				else if (!strcmp (lower_value, "hor_text"))
					strncpy (GMT->current.setting.map_annot_ortho, "we", 5U);
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_MAP_ANNOT_ORTHO:
			strncpy (GMT->current.setting.map_annot_ortho, lower_value, 5U);
			break;
		case GMTCASE_DEGREE_SYMBOL:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_DEGREE_SYMBOL");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_DEGREE_SYMBOL:
			if (value[0] == '\0' || !strcmp (lower_value, "ring"))	/* Default */
				GMT->current.setting.map_degree_symbol = gmt_ring;
			else if (!strcmp (lower_value, "degree"))
				GMT->current.setting.map_degree_symbol = gmt_degree;
			else if (!strcmp (lower_value, "colon"))
				GMT->current.setting.map_degree_symbol = gmt_colon;
			else if (!strcmp (lower_value, "none"))
				GMT->current.setting.map_degree_symbol = gmt_none;
			else
				error = true;
			break;
		case GMTCASE_BASEMAP_AXES:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_FRAME_AXES");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_FRAME_AXES:
			strncpy (GMT->current.setting.map_frame_axes, value, 5U);
			for (i = 0; i < 5; i++) GMT->current.map.frame.side[i] = 0;	/* Unset default settings */
			GMT->current.map.frame.draw_box = false;
			error += gmt5_decode_wesnz (GMT, value, false);
			break;

		case GMTCASE_BASEMAP_FRAME_RGB:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_DEFAULT_PEN");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_DEFAULT_PEN:
			i = (value[0] == '+') ? 1 : 0;	/* If plus is added, copy color to MAP_*_PEN settings */
			error = GMT_getpen (GMT, &value[i], &GMT->current.setting.map_default_pen);
			if (i == 1) {
				GMT_rgb_copy (&GMT->current.setting.map_grid_pen[0].rgb, &GMT->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&GMT->current.setting.map_grid_pen[1].rgb, &GMT->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&GMT->current.setting.map_frame_pen.rgb  , &GMT->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&GMT->current.setting.map_tick_pen[0].rgb, &GMT->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&GMT->current.setting.map_tick_pen[1].rgb, &GMT->current.setting.map_default_pen.rgb);
			}
			break;
		case GMTCASE_FRAME_PEN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_FRAME_PEN");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_FRAME_PEN:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_frame_pen);
			break;
		case GMTCASE_BASEMAP_TYPE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_FRAME_TYPE");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_FRAME_TYPE:
			if (!strcmp (lower_value, "plain"))
				GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
			else if (!strcmp (lower_value, "graph"))
				GMT->current.setting.map_frame_type = GMT_IS_GRAPH;
			else if (!strcmp (lower_value, "fancy"))
				GMT->current.setting.map_frame_type = GMT_IS_FANCY;
			else if (!strcmp (lower_value, "fancy+"))
				GMT->current.setting.map_frame_type = GMT_IS_ROUNDED;
			else if (!strcmp (lower_value, "inside"))
				GMT->current.setting.map_frame_type = GMT_IS_INSIDE;
			else
				error = true;
			break;
		case GMTCASE_FRAME_WIDTH:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_FRAME_WIDTH");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_FRAME_WIDTH:
			dval = GMT_to_inch (GMT, value);
			if (dval > 0.0)
				GMT->current.setting.map_frame_width = dval;
			else
				error = true;
			break;
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_GRID_CROSS_SIZE_PRIMARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_GRID_CROSS_SIZE:
			dval = GMT_to_inch (GMT, value);
			if (dval >= 0.0)
				GMT->current.setting.map_grid_cross_size[0] = GMT->current.setting.map_grid_cross_size[1] = dval;
			else
				error = true;
			break;
		case GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY:
			dval = GMT_to_inch (GMT, value);
			if (dval >= 0.0)
				GMT->current.setting.map_grid_cross_size[0] = dval;
			else
				error = true;
			break;
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_GRID_CROSS_SIZE_SECONDARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY:
			dval = GMT_to_inch (GMT, value);
			if (dval >= 0.0)
				GMT->current.setting.map_grid_cross_size[1] = dval;
			else
				error = true;
			break;
		case GMTCASE_GRID_PEN_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_GRID_PEN_PRIMARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_GRID_PEN:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_grid_pen[1]);
		case GMTCASE_MAP_GRID_PEN_PRIMARY:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_grid_pen[0]);
			break;
		case GMTCASE_GRID_PEN_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_GRID_PEN_SECONDARY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_GRID_PEN_SECONDARY:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_grid_pen[1]);
			break;
		case GMTCASE_LABEL_OFFSET:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_LABEL_OFFSET");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_LABEL_OFFSET:
			GMT->current.setting.map_label_offset = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_LINE_STEP:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_LINE_STEP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_LINE_STEP:
			if ((GMT->current.setting.map_line_step = GMT_to_inch (GMT, value)) <= 0.0) {
				GMT->current.setting.map_line_step = 0.01;
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %s <= 0, reset to %g %s\n", keyword, GMT->current.setting.map_line_step, GMT->session.unit_name[GMT_INCH]);
			}
			break;
		case GMTCASE_UNIX_TIME:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_LOGO");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_LOGO:
			error = gmt_true_false_or_error (lower_value, &GMT->current.setting.map_logo);
			break;
		case GMTCASE_UNIX_TIME_POS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_LOGO_POS");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_LOGO_POS:
			i = sscanf (value, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (i == 2) {
				GMT->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (GMT, txt_a);
				GMT->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (GMT, txt_b);
			}
			else if (i == 3) {	/* New style, includes justification, introduced in GMT 4.3.0 */
				GMT->current.setting.map_logo_justify = GMT_just_decode (GMT, txt_a, PSL_NO_DEF);
				GMT->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (GMT, txt_b);
				GMT->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (GMT, txt_c);
			}
			else
				error = true;
			break;
		case GMTCASE_X_ORIGIN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ORIGIN_X");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ORIGIN_X:
			GMT->current.setting.map_origin[GMT_X] = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_Y_ORIGIN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_ORIGIN_Y");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_ORIGIN_Y:
			GMT->current.setting.map_origin[GMT_Y] = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_POLAR_CAP:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_POLAR_CAP");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_POLAR_CAP:
			if (!strcmp (lower_value, "none")) {	/* Means reset to no cap -> lat = 90, dlon = 0 */
				GMT->current.setting.map_polar_cap[0] = 90.0;
				GMT->current.setting.map_polar_cap[1] = 0.0;
			}
			else {
				double inc[2];
				i = sscanf (lower_value, "%[^/]/%s", txt_a, txt_b);
				if (i != 2) error = true;
				error = GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_a, GMT_IS_LAT, &GMT->current.setting.map_polar_cap[0]), txt_a);
				if (GMT_getinc (GMT, txt_b, inc)) error = true;
				GMT->current.setting.map_polar_cap[1] = inc[GMT_X];
			}
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			dval = GMT_to_inch (GMT, value);
			if (dval <= 0.0)
				error = true;
			else
				GMT->current.setting.map_scale_height = dval;
			break;
		case GMTCASE_TICK_LENGTH:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_CHANGE ("MAP_TICK_LENGTH_PRIMARY and MAP_TICK_LENGTH_SECONDARY");
				GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = GMT_to_inch (GMT, value);
				GMT->current.setting.map_tick_length[GMT_TICK_UPPER]  = 0.50 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
				GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = 3.00 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
				GMT->current.setting.map_tick_length[GMT_TICK_LOWER]  = 0.75 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_MAP_TICK_LENGTH:
			i = sscanf (value, "%[^/]/%s", txt_a, txt_b);
			GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = GMT_to_inch (GMT, txt_a);
			GMT->current.setting.map_tick_length[GMT_TICK_LOWER]  = (i > 1) ? GMT_to_inch (GMT, txt_b) : 0.25 * GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER];
		case GMTCASE_MAP_TICK_LENGTH_PRIMARY:
			i = sscanf (value, "%[^/]/%s", txt_a, txt_b);
			GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = GMT_to_inch (GMT, txt_a);
			GMT->current.setting.map_tick_length[GMT_TICK_UPPER]  = (i > 1) ? GMT_to_inch (GMT, txt_b) : 0.50 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
			break;
		case GMTCASE_MAP_TICK_LENGTH_SECONDARY:
			i = sscanf (value, "%[^/]/%s", txt_a, txt_b);
			GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = GMT_to_inch (GMT, txt_a);
			GMT->current.setting.map_tick_length[GMT_TICK_LOWER]  = (i > 1) ? GMT_to_inch (GMT, txt_b) : 0.25 * GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER];
			break;
		case GMTCASE_TICK_PEN:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_CHANGE ("MAP_TICK_PEN_PRIMARY and MAP_TICK_PEN_SECONDARY");
				error = GMT_getpen (GMT, value, &GMT->current.setting.map_tick_pen[0]);
				error = GMT_getpen (GMT, value, &GMT->current.setting.map_tick_pen[1]);
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_MAP_TICK_PEN:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_tick_pen[1]);
		case GMTCASE_MAP_TICK_PEN_PRIMARY:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_tick_pen[0]);
			break;
		case GMTCASE_MAP_TICK_PEN_SECONDARY:
			error = GMT_getpen (GMT, value, &GMT->current.setting.map_tick_pen[1]);
			break;
		case GMTCASE_HEADER_OFFSET:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_TITLE_OFFSET");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_TITLE_OFFSET:
			GMT->current.setting.map_title_offset = GMT_to_inch (GMT, value);
			break;
		case GMTCASE_VECTOR_SHAPE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("MAP_VECTOR_SHAPE");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_MAP_VECTOR_SHAPE:
			dval = atof (value);
			if (dval < -2.0 || dval > 2.0)
				error = true;
			else
				GMT->current.setting.map_vector_shape = dval;
			break;

		/* COLOR GROUP */

		case GMTCASE_COLOR_BACKGROUND:
			error = GMT_getrgb (GMT, value, GMT->current.setting.color_patch[GMT_BGD]);
			break;
		case GMTCASE_COLOR_FOREGROUND:
			error = GMT_getrgb (GMT, value, GMT->current.setting.color_patch[GMT_FGD]);
			break;
		case GMTCASE_COLOR_MODEL:
			if (!strcmp (lower_value, "none"))
				GMT->current.setting.color_model = GMT_RGB;
			else if (!strcmp (lower_value, "rgb"))
				GMT->current.setting.color_model = GMT_RGB + GMT_COLORINT;
			else if (!strcmp (lower_value, "cmyk"))
				GMT->current.setting.color_model = GMT_CMYK + GMT_COLORINT;
			else if (!strcmp (lower_value, "hsv"))
				GMT->current.setting.color_model = GMT_HSV + GMT_COLORINT;
			else if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				if (!strcmp (lower_value, "+rgb")) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "warning: COLOR_MODEL = %s is deprecated, use COLOR_MODEL = %s instead\n" GMT_COMPAT_INFO, value, &lower_value[1]);
					GMT->current.setting.color_model = GMT_RGB + GMT_COLORINT;
				}
				else if (!strcmp (lower_value, "+cmyk")) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "warning: COLOR_MODEL = %s is deprecated, use COLOR_MODEL = %s instead\n" GMT_COMPAT_INFO, value, &lower_value[1]);
					GMT->current.setting.color_model = GMT_CMYK + GMT_COLORINT;
				}
				else if (!strcmp (lower_value, "+hsv")) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "warning: COLOR_MODEL = %s is deprecated, use COLOR_MODEL = %s instead\n" GMT_COMPAT_INFO, value, &lower_value[1]);
					GMT->current.setting.color_model = GMT_HSV + GMT_COLORINT;
				}
				else
					error = true;
			}
			else
				error = true;
			break;
		case GMTCASE_COLOR_NAN:
			error = GMT_getrgb (GMT, value, GMT->current.setting.color_patch[GMT_NAN]);
			break;
		case GMTCASE_HSV_MIN_SATURATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("COLOR_HSV_MIN_S");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_COLOR_HSV_MIN_S:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_min_s = dval;
			break;
		case GMTCASE_HSV_MAX_SATURATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("COLOR_HSV_MAX_S");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_COLOR_HSV_MAX_S:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_max_s = dval;
			break;
		case GMTCASE_HSV_MIN_VALUE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("COLOR_HSV_MIN_V");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_COLOR_HSV_MIN_V:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_min_v = dval;
			break;
		case GMTCASE_HSV_MAX_VALUE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("COLOR_HSV_MAX_V");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_COLOR_HSV_MAX_V:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_max_v = dval;
			break;

		/* PS GROUP */

		case GMTCASE_CHAR_ENCODING:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("PS_CHAR_ENCODING");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_CHAR_ENCODING:
			strncpy (GMT->current.setting.ps_encoding.name, value, GMT_LEN64);
			gmt_load_encoding (GMT);
			break;
		case GMTCASE_PS_COLOR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: Warn then fall through to other case */
				GMT_COMPAT_CHANGE ("PS_COLOR");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_COLOR_MODEL:
			if (!strcmp (lower_value, "rgb"))
				GMT->current.setting.ps_color_mode = PSL_RGB;
			else if (!strcmp (lower_value, "cmyk"))
				GMT->current.setting.ps_color_mode = PSL_CMYK;
			else if (!strcmp (lower_value, "hsv"))
				GMT->current.setting.ps_color_mode = PSL_HSV;
			else if (!strcmp (lower_value, "gray") || !strcmp (lower_value, "grey"))
				GMT->current.setting.ps_color_mode = PSL_GRAY;
			else
				error = true;
			break;
		case GMTCASE_N_COPIES:
		case GMTCASE_PS_COPIES:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				ival = atoi (value);
				if (ival > 0)
					GMT->current.setting.ps_copies = ival;
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_DOTS_PR_INCH:
		case GMTCASE_PS_DPI:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_EPS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else	/* Not recognized so give error message */
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_IMAGE_COMPRESS:
			if (!GMT->PSL) return (0);	/* Not using PSL in this session */
			if (!strcmp (lower_value, "none"))
				GMT->PSL->internal.compress = PSL_NONE;
			else if (!strcmp (lower_value, "rle"))
				GMT->PSL->internal.compress = PSL_RLE;
			else if (!strcmp (lower_value, "lzw"))
				GMT->PSL->internal.compress = PSL_LZW;
			else if (!strncmp (lower_value, "deflate", 7)) {
#ifdef HAVE_ZLIB
				GMT->PSL->internal.compress = PSL_DEFLATE;
				if ((sscanf (value + 7, " , %u", &GMT->PSL->internal.deflate_level) != 1)
						|| GMT->PSL->internal.deflate_level > 9)
					/* Compression level out of range or not provided, using default */
					GMT->PSL->internal.deflate_level = 0;
#else
				/* Silently fall back to LZW compression when ZLIB not available */
				GMT->PSL->internal.compress = PSL_LZW;
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "warning: PS_IMAGE_COMPRESS = DEFLATE not available, falling back to LZW.\n");
#endif
			}
			else
				error = true;
			break;
		case GMTCASE_PS_LINE_CAP:
			if (!GMT->PSL) return (0);	/* Not using PSL in this session */
			if (!strcmp (lower_value, "butt"))
				GMT->PSL->internal.line_cap = PSL_BUTT_CAP;
			else if (!strcmp (lower_value, "round"))
				GMT->PSL->internal.line_cap = PSL_ROUND_CAP;
			else if (!strcmp (lower_value, "square"))
				GMT->PSL->internal.line_cap = PSL_SQUARE_CAP;
			else
				error = true;
			break;
		case GMTCASE_PS_LINE_JOIN:
			if (!GMT->PSL) return (0);	/* Not using PSL in this session */
			if (!strcmp (lower_value, "miter"))
				GMT->PSL->internal.line_join = PSL_MITER_JOIN;
			else if (!strcmp (lower_value, "round"))
				GMT->PSL->internal.line_join = PSL_ROUND_JOIN;
			else if (!strcmp (lower_value, "bevel"))
				GMT->PSL->internal.line_join = PSL_BEVEL_JOIN;
			else
				error = true;
			break;
		case GMTCASE_PS_MITER_LIMIT:
			if (!GMT->PSL) return (0);	/* Not using PSL in this session */
			ival = atoi (value);
			if (ival >= 0 && ival <= 180)
				GMT->PSL->internal.miter_limit = ival;
			else
				error = true;
			break;
		case GMTCASE_PAGE_COLOR:
		if (GMT_compat_check (GMT, 4))	/* GMT4: */
			GMT_COMPAT_CHANGE ("PS_PAGE_COLOR");
		else {	/* Not recognized so give error message */
			error = gmt_badvalreport (GMT, keyword);
			break;
		}
		case GMTCASE_PS_PAGE_COLOR:
			error = GMT_getrgb (GMT, value, GMT->current.setting.ps_page_rgb);
			break;
		case GMTCASE_PAGE_ORIENTATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PS_PAGE_ORIENTATION");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_PAGE_ORIENTATION:
			if (!strcmp (lower_value, "landscape"))
				GMT->current.setting.ps_orientation = PSL_LANDSCAPE;
			else if (!strcmp (lower_value, "portrait"))
				GMT->current.setting.ps_orientation = PSL_PORTRAIT;
			else
				error = true;
			break;
		case GMTCASE_PAPER_MEDIA:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PS_MEDIA");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_MEDIA:
			manual = false;
			len--;
			if (lower_value[len] == '-') {	/* Manual Feed selected */
				lower_value[len] = '\0';
				manual = true;
			}
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				if (lower_value[len] == '+') {	/* EPS format selected */
					lower_value[len] = '\0';
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Production of EPS format is no longer supported, remove + after paper size\n");
				}
			}
			if ((i = gmt_key_lookup (lower_value, GMT_media_name, GMT_N_MEDIA)) < GMT_N_MEDIA) {
				/* Use the specified standard format */
				GMT->current.setting.ps_media = i;
				GMT->current.setting.ps_page_size[0] = GMT_media[i].width;
				GMT->current.setting.ps_page_size[1] = GMT_media[i].height;
			}
			else if (gmt_load_user_media (GMT) &&
				(pos = gmt_key_lookup (lower_value, GMT->session.user_media_name, GMT->session.n_user_media)) < GMT->session.n_user_media) {
				/* Use a user-specified format */
					GMT->current.setting.ps_media = pos + USER_MEDIA_OFFSET;
					GMT->current.setting.ps_page_size[0] = GMT->session.user_media[pos].width;
					GMT->current.setting.ps_page_size[1] = GMT->session.user_media[pos].height;
				}
			else {
				/* A custom paper size in W x H points (or in inch/c if units are appended) */
				if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
					pos = (strncmp (lower_value, "custom_", 7U) ? 0 : 7);
				}
				else
					pos = 0;
				GMT_strtok (lower_value, "x", &pos, txt_a);	/* Returns width and update pos */
				GMT->current.setting.ps_page_size[0] = GMT_convert_units (GMT, txt_a, GMT_PT, GMT_PT);
				GMT_strtok (lower_value, "x", &pos, txt_b);	/* Returns height and update pos */
				GMT->current.setting.ps_page_size[1] = GMT_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
				if (GMT->current.setting.ps_page_size[0] <= 0.0) error++;
				if (GMT->current.setting.ps_page_size[1] <= 0.0) error++;
				GMT->current.setting.ps_media = -USER_MEDIA_OFFSET;
			}
			if (!error && manual) GMT->current.setting.ps_page_size[0] = -GMT->current.setting.ps_page_size[0];
			break;
		case GMTCASE_GLOBAL_X_SCALE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PS_SCALE_X");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_SCALE_X:
			dval = atof (value);
			if (dval > 0.0)
				GMT->current.setting.ps_magnify[GMT_X] = dval;
			else
				error = true;
			break;
		case GMTCASE_GLOBAL_Y_SCALE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PS_SCALE_Y");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_SCALE_Y:
			dval = atof (value);
			if (dval > 0.0)
				GMT->current.setting.ps_magnify[GMT_Y] = dval;
			else
				error = true;
			break;
		case GMTCASE_TRANSPARENCY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Transparency is now part of pen and fill specifications.  TRANSPARENCY is ignored\n");
			else
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_TRANSPARENCY:
			strncpy (GMT->current.setting.ps_transpmode, value, 15U);
			break;
		case GMTCASE_PS_VERBOSE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PS_COMMENTS");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_PS_COMMENTS:
			if (!GMT->PSL) return (0);	/* Not using PSL in this session */
			error = gmt_true_false_or_error (lower_value, &tf_answer);
			GMT->PSL->internal.comments = (tf_answer) ? 1 : 0;
			break;

		/* IO GROUP */

		case GMTCASE_FIELD_DELIMITER:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("IO_COL_SEPARATOR");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */

		case GMTCASE_IO_COL_SEPARATOR:
			if (value[0] == '\0' || !strcmp (lower_value, "tab"))	/* DEFAULT */
				strncpy (GMT->current.setting.io_col_separator, "\t", 8U);
			else if (!strcmp (lower_value, "space"))
				strncpy (GMT->current.setting.io_col_separator, " ", 8U);
			else if (!strcmp (lower_value, "comma"))
				strncpy (GMT->current.setting.io_col_separator, ",", 8U);
			else if (!strcmp (lower_value, "none"))
				GMT->current.setting.io_col_separator[0] = 0;
			else
				strncpy (GMT->current.setting.io_col_separator, value, 8U);
			GMT->current.setting.io_col_separator[7] = 0;	/* Just a precaution */
			break;
		case GMTCASE_GRIDFILE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("IO_GRIDFILE_FORMAT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_GRIDFILE_FORMAT:
			strncpy (GMT->current.setting.io_gridfile_format, value, GMT_LEN64);
			break;
		case GMTCASE_GRIDFILE_SHORTHAND:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("IO_GRIDFILE_SHORTHAND");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_GRIDFILE_SHORTHAND:
			error = gmt_true_false_or_error (lower_value, &GMT->current.setting.io_gridfile_shorthand);
			break;
		case GMTCASE_IO_HEADER:
			error = gmt_true_false_or_error (lower_value, &GMT->current.setting.io_header[GMT_IN]);
			GMT->current.setting.io_header[GMT_OUT] = GMT->current.setting.io_header[GMT_IN];
			break;
		case GMTCASE_N_HEADER_RECS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("IO_N_HEADER_ITEMS");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_N_HEADER_RECS:
			ival = atoi (value);
			if (ival < 0)
				error = true;
			else
				GMT->current.setting.io_n_header_items = ival;
			break;
		case GMTCASE_NAN_RECORDS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("IO_NAN_RECORDS");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_NAN_RECORDS:
			if (!strcmp (lower_value, "pass"))
				GMT->current.setting.io_nan_records = true;
			else if (!strcmp (lower_value, "skip"))
				GMT->current.setting.io_nan_records = false;
			else
				error = true;
			break;
		case GMTCASE_IO_NC4_CHUNK_SIZE:
				if (*lower_value == 'a') /* auto */
				GMT->current.setting.io_nc4_chunksize[0] = k_netcdf_io_chunked_auto;
			else if (*lower_value == 'c') /* classic */
				GMT->current.setting.io_nc4_chunksize[0] = k_netcdf_io_classic;
			else if ((i = sscanf (value, "%" SCNuS " , %" SCNuS, /* chunk size: lat,lon */
						&GMT->current.setting.io_nc4_chunksize[0],
						&GMT->current.setting.io_nc4_chunksize[1])) > 0) {
				if (i == 1) /* use chunk size of lat for long as well */
					GMT->current.setting.io_nc4_chunksize[1] = GMT->current.setting.io_nc4_chunksize[0];
				if (GMT->current.setting.io_nc4_chunksize[0] <= k_netcdf_io_chunked_auto ||
						GMT->current.setting.io_nc4_chunksize[1] <= k_netcdf_io_chunked_auto)
					/* chunk size too small */
					error = true;
			}
			else
				error = true;
			break;
		case GMTCASE_IO_NC4_DEFLATION_LEVEL:
			if (!strcmp (lower_value, "false"))
				ival = 0;
			else
				ival = atoi (value);
			if (ival >= 0 && ival <= 9)
				GMT->current.setting.io_nc4_deflation_level = ival;
			else
				error = true;
			break;
		case GMTCASE_XY_TOGGLE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("IO_LONLAT_TOGGLE");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_LONLAT_TOGGLE:
			if (!gmt_true_false_or_error (lower_value, &GMT->current.setting.io_lonlat_toggle[GMT_IN]))
				/* We got false/f/0 or true/t/1. Set outgoing setting to the same as the ingoing. */
				GMT->current.setting.io_lonlat_toggle[GMT_OUT] = GMT->current.setting.io_lonlat_toggle[GMT_IN];
			else if (!strcmp (lower_value, "in")) {
				GMT->current.setting.io_lonlat_toggle[GMT_IN] = true;
				GMT->current.setting.io_lonlat_toggle[GMT_OUT] = false;
			}
			else if (!strcmp (lower_value, "out")) {
				GMT->current.setting.io_lonlat_toggle[GMT_IN] = false;
				GMT->current.setting.io_lonlat_toggle[GMT_OUT] = true;
			}
			else
				error = true;
			break;

		case GMTCASE_IO_SEGMENT_BINARY:
			if (!strcmp (lower_value, "off"))
				GMT->current.setting.n_bin_header_cols = 0;	/* 0 means do not consider nans to mean segment header */
			else {	/* Read the minimum columns a binary record must have to be examined for segment headers */
				ival = atoi (value);
				if (ival < 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding IO_SEGMENT_BINARY: Cannot be negative.\n");
					error = true;
				}
				else
					GMT->current.setting.n_bin_header_cols = (uint64_t)ival;	/* Only do it for files with at least this many cols */
			}
			break;

		case GMTCASE_IO_SEGMENT_MARKER:
			if (len == 0)	/* Blank gives default */
				GMT->current.setting.io_seg_marker[GMT_OUT] = GMT->current.setting.io_seg_marker[GMT_IN] = '>';
			else {
				int dir, k;
				char txt[2][GMT_LEN256];
				if (strchr (value, ',')) {	/* Got separate markers for input,output */
					sscanf (value, "%[^,],%s", txt[GMT_IN], txt[GMT_OUT]);
				}
				else {	/* Just duplicate */
					strncpy (txt[GMT_IN], value, GMT_LEN256);	strncpy (txt[GMT_OUT], value, GMT_LEN256);
				}
				for (dir = 0; dir < 2; dir++) {
					switch (txt[dir][0]) {
						case 'B':
							GMT->current.setting.io_blankline[dir] = true;
							break;
						case 'N':
							GMT->current.setting.io_nanline[dir] = true;
							break;
						default:
							k = (txt[dir][0] == '\\') ? 1 : 0;
							GMT->current.setting.io_seg_marker[dir] = txt[dir][k];
							break;
					}
				}
			}
			break;

		/* PROJ GROUP */

		case GMTCASE_PROJ_AUX_LATITUDE:
			if (!strncmp (lower_value, "none", 4U)) /* Use lat as is */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_NONE;
			else if (!strncmp (lower_value, "authalic", 8U)) /* Authalic latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2A;
			else if (!strncmp (lower_value, "conformal", 9U)) /* Conformal latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2C;
			else if (!strncmp (lower_value, "geocentric", 9U)) /* Geocentric latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2O;
			else if (!strncmp (lower_value, "meridional", 10U)) /* Meridional latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2M;
			else if (!strncmp (lower_value, "parametric", 10U)) /* Parametric latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2P;
			else
				error = true;
			GMT_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
			break;

		case GMTCASE_ELLIPSOID:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PROJ_ELLIPSOID");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_ELLIPSOID:
			ival = GMT_get_ellipsoid (GMT, value);
			if (ival < 0)
				error = true;
			else
				GMT->current.setting.proj_ellipsoid = ival;
			GMT_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
			break;
		case GMTCASE_PROJ_DATUM:	/* Not implemented yet */
			break;
		case GMTCASE_PROJ_GEODESIC:
			if (!strncmp (lower_value, "vincenty", 8U)) /* Same as exact*/
				GMT->current.setting.proj_geodesic = GMT_GEODESIC_VINCENTY;
			else if (!strncmp (lower_value, "andoyer", 7U)) /* Andoyer approximation */
				GMT->current.setting.proj_geodesic = GMT_GEODESIC_ANDOYER;
			else if (!strncmp (lower_value, "rudoe", 5U)) /* Volumetric radius R_3 */
				GMT->current.setting.proj_geodesic = GMT_GEODESIC_RUDOE;
			else
				error = true;
			GMT_init_geodesic (GMT);	/* Set function pointer depending on the geodesic selected */
			break;

		case GMTCASE_MEASURE_UNIT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PROJ_LENGTH_UNIT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_LENGTH_UNIT:
			switch (lower_value[0]) {
				case 'c': GMT->current.setting.proj_length_unit = GMT_CM; break;
				case 'i': GMT->current.setting.proj_length_unit = GMT_INCH; break;
				case 'p': GMT->current.setting.proj_length_unit = GMT_PT; break;
				default: error = true;
			}
			break;
		case GMTCASE_PROJ_MEAN_RADIUS:
			if (!strncmp (lower_value, "mean", 4U)) /* Mean radius R_1 */
				GMT->current.setting.proj_mean_radius = GMT_RADIUS_MEAN;
			else if (!strncmp (lower_value, "authalic", 8U)) /* Authalic radius R_2 */
				GMT->current.setting.proj_mean_radius = GMT_RADIUS_AUTHALIC;
			else if (!strncmp (lower_value, "volumetric", 10U)) /* Volumetric radius R_3 */
				GMT->current.setting.proj_mean_radius = GMT_RADIUS_VOLUMETRIC;
			else if (!strncmp (lower_value, "meridional", 10U)) /* Meridional radius */
				GMT->current.setting.proj_mean_radius = GMT_RADIUS_MERIDIONAL;
			else if (!strncmp (lower_value, "quadratic", 9U)) /* Quadratic radius */
				GMT->current.setting.proj_mean_radius = GMT_RADIUS_QUADRATIC;
			else
				error = true;
			GMT_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
			break;

		case GMTCASE_MAP_SCALE_FACTOR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("PROJ_SCALE_FACTOR");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_SCALE_FACTOR:
			if (!strncmp (value, "def", 3U)) /* Default scale for chosen projection */
				GMT->current.setting.proj_scale_factor = -1.0;
			else {
				dval = atof (value);
				if (dval <= 0.0)
					error = true;
				else
					GMT->current.setting.proj_scale_factor = dval;
			}
			break;

		/* GMT GROUP */

		case GMTCASE_GMT_COMPATIBILITY:
			ival = (int)atof (value);
			if (ival < 4) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_COMPATIBILITY: Expects values from 4 to %d; reset to 4.\n", GMT_MAJOR_VERSION);
				GMT->current.setting.compatibility = 4;
			}
			else if (ival > GMT_MAJOR_VERSION) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_COMPATIBILITY: Expects values from 4 to %d; reset to %d.\n", GMT_MAJOR_VERSION, GMT_MAJOR_VERSION);
				GMT->current.setting.compatibility = GMT_MAJOR_VERSION;
			}
			else
				GMT->current.setting.compatibility = ival;
			break;

		case GMTCASE_GMT_CUSTOM_LIBS:
			if (*value) {
				if (GMT->session.CUSTOM_LIBS) {
					if ((strcmp (GMT->session.CUSTOM_LIBS, value) == 0))
						break; /* stop here if string in place is equal */
					free (GMT->session.CUSTOM_LIBS);
				}
				/* Set Extension shared libraries */
				GMT->session.CUSTOM_LIBS = strdup (value);
			}
			break;

		case GMTCASE_GMT_EXTRAPOLATE_VAL:
			if (!strcmp (lower_value, "nan"))
				GMT->current.setting.extrapolate_val[0] = GMT_EXTRAPOLATE_NONE;
			else if (!strcmp (lower_value, "extrap"))
				GMT->current.setting.extrapolate_val[0] = GMT_EXTRAPOLATE_SPLINE;
			else if (!strncmp (lower_value, "extrapval",9)) {
				GMT->current.setting.extrapolate_val[0] = GMT_EXTRAPOLATE_CONSTANT;
				GMT->current.setting.extrapolate_val[1] = atof (&lower_value[10]);
				if (lower_value[9] != ',') {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding GMT_EXTRAPOLATE_VAL for 'val' value. Comma out of place.\n");
					error = true;
				}
			}
			else
				error = true;
			if (error) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_EXTRAPOLATE_VAL: resetting to 'extrapolated is NaN' to avoid later crash.\n");
				GMT->current.setting.extrapolate_val[0] = GMT_EXTRAPOLATE_NONE;
			}
			break;

		case GMTCASE_GMT_FFT:
			if (!strncmp (lower_value, "auto", 4))
				GMT->current.setting.fft = k_fft_auto;
			else if (!strncmp (lower_value, "fftw", 4)) { /* complete name: fftw */
				GMT->current.setting.fft = k_fft_fftw;
#ifdef HAVE_FFTW3F
				/* FFTW planner flags supported by the planner routines in FFTW
				 * FFTW_ESTIMATE:   pick a (probably sub-optimal) plan quickly
				 * FFTW_MEASURE:    find optimal plan by computing several FFTs and measuring their execution time
				 * FFTW_PATIENT:    like FFTW_MEASURE, but considers a wider range of algorithms
				 * FFTW_EXHAUSTIVE: like FFTW_PATIENT, but considers an even wider range of algorithms */
				GMT->current.setting.fftw_plan = FFTW_ESTIMATE; /* default planner flag */
				{
					char *c;
					if ((c = strchr (lower_value, ',')) != NULL) { /* Parse FFTW planner flags */
						c += strspn(c, ", \t"); /* advance past ',' and whitespace */
						if (!strncmp (c, "m", 1)) /* complete: measure */
							GMT->current.setting.fftw_plan = FFTW_MEASURE;
						else if (!strncmp (c, "p", 1)) /* complete: patient */
							GMT->current.setting.fftw_plan = FFTW_PATIENT;
						else if (!strncmp (c, "ex", 2)) /* complete: exhaustive */
							GMT->current.setting.fftw_plan = FFTW_EXHAUSTIVE;
					}
				}
#endif /* HAVE_FFTW3F */
			}
			else if (!strncmp (lower_value, "ac", 2))   /* complete name: accelerate */
				GMT->current.setting.fft = k_fft_accelerate;
			else if (!strncmp (lower_value, "kiss", 4)) /* complete name: kissfft */
				GMT->current.setting.fft = k_fft_kiss;
			else if (!strcmp (lower_value, "brenner"))
				GMT->current.setting.fft = k_fft_brenner;
			else
				error = true;
			break;
		case GMTCASE_HISTORY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("GMT_HISTORY");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_HISTORY:
			if      (strspn (lower_value, "1t"))
				GMT->current.setting.history = (k_history_read | k_history_write);
			else if (strchr (lower_value, 'r'))
				GMT->current.setting.history = k_history_read;
			else if (strspn (lower_value, "0f"))
				GMT->current.setting.history = k_history_off;
			else
				error = true;
			break;
		case GMTCASE_INTERPOLANT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("GMT_INTERPOLANT");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_INTERPOLANT:
			if (!strcmp (lower_value, "linear"))
				GMT->current.setting.interpolant = GMT_SPLINE_LINEAR;
			else if (!strcmp (lower_value, "akima"))
				GMT->current.setting.interpolant = GMT_SPLINE_AKIMA;
			else if (!strcmp (lower_value, "cubic"))
				GMT->current.setting.interpolant = GMT_SPLINE_CUBIC;
			else if (!strcmp (lower_value, "none"))
				GMT->current.setting.interpolant = GMT_SPLINE_NONE;
			else
				error = true;
			break;
		case GMTCASE_GMT_TRIANGULATE:
			if (!strcmp (lower_value, "watson"))
				GMT->current.setting.triangulate = GMT_TRIANGLE_WATSON;
			else if (!strcmp (lower_value, "shewchuk"))
				GMT->current.setting.triangulate = GMT_TRIANGLE_SHEWCHUK;
			else
				error = true;
			break;
		case GMTCASE_VERBOSE:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_CHANGE ("GMT_VERBOSE");
				ival = atoi (value) + 2;
				if (ival >= GMT_MSG_QUIET && ival <= GMT_MSG_DEBUG)
					GMT->current.setting.verbose = ival;
				else
					error = true;
			}
			else
				error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_GMT_VERBOSE:
			error = gmt_parse_V_option (GMT, lower_value[0]);
			break;

		/* DIR GROUP */

		case GMTCASE_DIR_DATA:
			if (*value) {
				if (GMT->session.DATADIR) {
					if ((strcmp (GMT->session.DATADIR, value) == 0))
						break; /* stop here if string in place is equal */
					free (GMT->session.DATADIR);
				}
				/* Set session DATADIR dir */
				GMT->session.DATADIR = strdup (value);
			}
			break;
		case GMTCASE_DIR_DCW:
			if (*value) {
				if (GMT->session.DCWDIR) {
					if ((strcmp (GMT->session.DCWDIR, value) == 0))
						break; /* stop here if string in place is equal */
					free (GMT->session.DCWDIR);
				}
				/* Set session DCW dir */
				GMT->session.DCWDIR = strdup (value);
			}
			break;
		case GMTCASE_DIR_GSHHG:
			if (*value) {
				if (GMT->session.GSHHGDIR) {
					if ((strcmp (GMT->session.GSHHGDIR, value) == 0))
						break; /* stop here if string in place is equal */
					free (GMT->session.GSHHGDIR);
				}
				/* Set session GSHHG dir */
				GMT->session.GSHHGDIR = strdup (value);
			}
			break;

		/* TIME GROUP */

		case GMTCASE_TIME_EPOCH:
			strncpy (GMT->current.setting.time_system.epoch, value, GMT_LEN64);
			(void) GMT_init_time_system_structure (GMT, &GMT->current.setting.time_system);
			break;
		case GMTCASE_TIME_IS_INTERVAL:
			if (value[0] == '+' || value[0] == '-') {	/* OK, gave +<n>u or -<n>u, check for unit */
				sscanf (&lower_value[1], "%d%c", &GMT->current.time.truncate.T.step, &GMT->current.time.truncate.T.unit);
				switch (GMT->current.time.truncate.T.unit) {
					case 'y': case 'o': case 'd': case 'h': case 'm': case 'c':
						GMT->current.time.truncate.direction = (lower_value[0] == '+') ? 0 : 1;
						break;
					default:
						error = true;
						break;
				}
				if (GMT->current.time.truncate.T.step == 0) error = true;
				GMT->current.setting.time_is_interval = true;
			}
			else if (!strcmp (lower_value, "off"))
				GMT->current.setting.time_is_interval = false;
			else
				error = true;
			break;
		case GMTCASE_TIME_INTERVAL_FRACTION:
			GMT->current.setting.time_interval_fraction = atof (value);
			break;
		case GMTCASE_GMT_LANGUAGE:
		case GMTCASE_TIME_LANGUAGE:
			strncpy (GMT->current.setting.language, lower_value, GMT_LEN64);
			gmt_get_language (GMT);	/* Load in names and abbreviations in chosen language */
			break;
		case GMTCASE_WANT_LEAP_SECONDS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("TIME_LEAP_SECONDS");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_TIME_LEAP_SECONDS:
			error = gmt_true_false_or_error (lower_value, &GMT->current.setting.time_leap_seconds);
			break;
		case GMTCASE_TIME_REPORT:
			if (!strncmp (lower_value, "none", 4U))
				GMT->current.setting.timer_mode = GMT_NO_TIMER;
			else if (!strncmp (lower_value, "clock", 5U))
				GMT->current.setting.timer_mode = GMT_ABS_TIMER;
			else if (!strncmp (lower_value, "elapsed", 7U))
				GMT->current.setting.timer_mode = GMT_ELAPSED_TIMER;
			else
				error = true;
			break;
		case GMTCASE_TIME_UNIT:
			GMT->current.setting.time_system.unit = lower_value[0];
			(void) GMT_init_time_system_structure (GMT, &GMT->current.setting.time_system);
			break;
		case GMTCASE_TIME_SYSTEM:
			error = GMT_get_time_system (GMT, lower_value, &GMT->current.setting.time_system);
			(void) GMT_init_time_system_structure (GMT, &GMT->current.setting.time_system);
			break;
		case GMTCASE_TIME_WEEK_START:
			ival = gmt_key_lookup (value, GMT_weekdays, 7);
			if (ival < 0 || ival >= 7) {
				error = true;
				GMT->current.setting.time_week_start = 0;
			}
			else
				GMT->current.setting.time_week_start = ival;
			break;
		case GMTCASE_Y2K_OFFSET_YEAR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_CHANGE ("TIME_Y2K_OFFSET_YEAR");
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_TIME_Y2K_OFFSET_YEAR:
			if ((ival = atoi (value)) < 0) error = true;
			else GMT->current.setting.time_Y2K_offset_year = ival;
			/* Set the Y2K conversion parameters */
			GMT->current.time.Y2K_fix.y2_cutoff = GMT->current.setting.time_Y2K_offset_year % 100;
			GMT->current.time.Y2K_fix.y100 = GMT->current.setting.time_Y2K_offset_year - GMT->current.time.Y2K_fix.y2_cutoff;
			GMT->current.time.Y2K_fix.y200 = GMT->current.time.Y2K_fix.y100 + 100;
			break;

		/* Obsolete */

		case GMTCASE_PS_IMAGE_FORMAT:
			/* Setting ignored, now always ASCII85 encoding */
		case GMTCASE_X_AXIS_LENGTH:
		case GMTCASE_Y_AXIS_LENGTH:
			/* Setting ignored: x- and/or y scale are required inputs on -J option */
		case GMTCASE_COLOR_IMAGE:
			GMT_COMPAT_WARN;
			/* Setting ignored, now always adobe image */
			if (!GMT_compat_check (GMT, 4))	error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_DIR_TMP:
		case GMTCASE_DIR_USER:
			/* Setting ignored, were active previously in GMT5 but no longer */
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Warning: Parameter %s (previously introduced in GMT5) is deprecated.\n" GMT_COMPAT_INFO, GMT_keywords[case_val]);
			break;

		default:
			error = true;
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized keyword %s.\n", keyword);
			break;
	}

	/* Store possible unit.  For most cases these are irrelevant as no unit is expected */
	if (len) GMT->current.setting.given_unit[case_val] = value[len-1];

	if (error && case_val >= 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: %s given illegal value (%s)!\n", keyword, value);
	return ((error) ? 1 : 0);
}

/*! . */
char *GMT_putparameter (struct GMT_CTRL *GMT, char *keyword)
{	/* value must hold at least GMT_BUFSIZ chars */
	static char value[GMT_LEN256] = {""}, txt[8];
	int case_val;
	bool error = false;
	char pm[2] = {'+', '-'}, *ft[2] = {"false", "true"};

	GMT_memset (value, GMT_LEN256, char);
	if (!keyword) return (value);		/* keyword argument missing */

	case_val = GMT_hash_lookup (GMT, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		/* FORMAT GROUP */
		case GMTCASE_INPUT_CLOCK_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_CLOCK_IN:
			strncpy (value, GMT->current.setting.format_clock_in,  GMT_LEN256);
			break;
		case GMTCASE_INPUT_DATE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_DATE_IN:
			strncpy (value, GMT->current.setting.format_date_in, GMT_LEN256);
			break;
		case GMTCASE_OUTPUT_CLOCK_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_CLOCK_OUT:
			strncpy (value, GMT->current.setting.format_clock_out, GMT_LEN256);
			break;
		case GMTCASE_OUTPUT_DATE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_DATE_OUT:
			strncpy (value, GMT->current.setting.format_date_out, GMT_LEN256);
			break;
		case GMTCASE_OUTPUT_DEGREE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_GEO_OUT:
			strncpy (value, GMT->current.setting.format_geo_out, GMT_LEN256);
			break;
		case GMTCASE_PLOT_CLOCK_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_CLOCK_MAP:
			strncpy (value, GMT->current.setting.format_clock_map, GMT_LEN256);
			break;
		case GMTCASE_PLOT_DATE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_DATE_MAP:
			strncpy (value, GMT->current.setting.format_date_map, GMT_LEN256);
			break;
		case GMTCASE_PLOT_DEGREE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_GEO_MAP:
			strncpy (value, GMT->current.setting.format_geo_map, GMT_LEN256);
			break;
		case GMTCASE_TIME_FORMAT_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_TIME_PRIMARY_MAP:
			strncpy (value, GMT->current.setting.format_time[0], GMT_LEN256);
			break;
		case GMTCASE_TIME_FORMAT_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_TIME_SECONDARY_MAP:
			strncpy (value, GMT->current.setting.format_time[1], GMT_LEN256);
			break;
		case GMTCASE_D_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_FLOAT_OUT:
			strncpy (value, GMT->current.setting.format_float_out_orig, GMT_LEN256);
			break;
		case GMTCASE_FORMAT_FLOAT_MAP:
			strncpy (value, GMT->current.setting.format_float_map, GMT_LEN256);
			break;
		case GMTCASE_UNIX_TIME_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_TIME_STAMP:
			strncpy (value, GMT->current.setting.format_time_stamp, GMT_LEN256);
			break;

		/* FONT GROUP */

		case GMTCASE_ANNOT_FONT_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_ANNOT_PRIMARY:
			strncpy (value, GMT_putfont (GMT, GMT->current.setting.font_annot[0]), GMT_LEN256);
			break;
		case GMTCASE_ANNOT_FONT_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_ANNOT_SECONDARY:
			strncpy (value, GMT_putfont (GMT, GMT->current.setting.font_annot[1]), GMT_LEN256);
			break;
		case GMTCASE_HEADER_FONT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_TITLE:
			strncpy (value, GMT_putfont (GMT, GMT->current.setting.font_title), GMT_LEN256);
			break;
		case GMTCASE_LABEL_FONT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_LABEL:
			strncpy (value, GMT_putfont (GMT, GMT->current.setting.font_label), GMT_LEN256);
			break;

		case GMTCASE_FONT_LOGO:
			strncpy (value, GMT_putfont (GMT, GMT->current.setting.font_logo), GMT_LEN256);
			break;

		/* FONT GROUP ... obsolete options */

		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				sprintf (value, "%g", GMT->current.setting.font_annot[0].size);
			}
			else
				error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				sprintf (value, "%g", GMT->current.setting.font_annot[1].size);
			}
			else
				error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_HEADER_FONT_SIZE:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				sprintf (value, "%g", GMT->current.setting.font_title.size);
			}
			else
				error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_LABEL_FONT_SIZE:
			if (GMT_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				sprintf (value, "%g", GMT->current.setting.font_label.size);
			}
			else
				error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;

		/* MAP GROUP */

		case GMTCASE_ANNOT_OFFSET_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_OFFSET_PRIMARY:
			sprintf (value, "%g%c", GMT->current.setting.map_annot_offset[0] GMT_def(GMTCASE_MAP_ANNOT_OFFSET_PRIMARY));
			break;
		case GMTCASE_ANNOT_OFFSET_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_OFFSET_SECONDARY:
			sprintf (value, "%g%c", GMT->current.setting.map_annot_offset[1] GMT_def(GMTCASE_MAP_ANNOT_OFFSET_SECONDARY));
			break;
		case GMTCASE_OBLIQUE_ANNOTATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_OBLIQUE:
			sprintf (value, "%d", GMT->current.setting.map_annot_oblique);
			break;
		case GMTCASE_ANNOT_MIN_ANGLE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_MIN_ANGLE:
			sprintf (value, "%g", GMT->current.setting.map_annot_min_angle);
			break;
		case GMTCASE_ANNOT_MIN_SPACING:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_MIN_SPACING:
			sprintf (value, "%g%c", GMT->current.setting.map_annot_min_spacing GMT_def(GMTCASE_MAP_ANNOT_MIN_SPACING));
			break;
		case GMTCASE_Y_AXIS_TYPE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_ORTHO:
			strncpy (value, GMT->current.setting.map_annot_ortho, GMT_LEN256);
			break;
		case GMTCASE_DEGREE_SYMBOL:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_DEGREE_SYMBOL:
			switch (GMT->current.setting.map_degree_symbol) {
				case gmt_ring:		strcpy (value, "ring");		break;
				case gmt_degree:	strcpy (value, "degree");	break;
				case gmt_colon:		strcpy (value, "colon");	break;
				case gmt_none:		strcpy (value, "none");		break;
				default: strcpy (value, "undefined");
			}
			break;
		case GMTCASE_BASEMAP_AXES:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_AXES:
			strncpy (value, GMT->current.setting.map_frame_axes, GMT_LEN256);
			break;
		case GMTCASE_BASEMAP_FRAME_RGB:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_DEFAULT_PEN:
			sprintf (value, "%s", GMT_putpen (GMT, GMT->current.setting.map_default_pen));
			break;
		case GMTCASE_FRAME_PEN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_PEN:
			sprintf (value, "%s", GMT_putpen (GMT, GMT->current.setting.map_frame_pen));
			break;
		case GMTCASE_BASEMAP_TYPE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_TYPE:
			if (GMT->current.setting.map_frame_type == GMT_IS_PLAIN)
				strcpy (value, "plain");
			else if (GMT->current.setting.map_frame_type == GMT_IS_GRAPH)
				strcpy (value, "graph");
			else if (GMT->current.setting.map_frame_type == GMT_IS_FANCY)
				strcpy (value, "fancy");
			else if (GMT->current.setting.map_frame_type == GMT_IS_ROUNDED)
				strcpy (value, "fancy+");
			else if (GMT->current.setting.map_frame_type == GMT_IS_INSIDE)
				strcpy (value, "inside");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_FRAME_WIDTH:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_WIDTH:
			sprintf (value, "%g%c", GMT->current.setting.map_frame_width GMT_def(GMTCASE_MAP_FRAME_WIDTH));
			break;
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY:
			sprintf (value, "%g%c", GMT->current.setting.map_grid_cross_size[0] GMT_def(GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY));
			break;
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY:
			sprintf (value, "%g%c", GMT->current.setting.map_grid_cross_size[1] GMT_def(GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY));
			break;
		case GMTCASE_GRID_PEN_PRIMARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_PEN_PRIMARY:
			sprintf (value, "%s", GMT_putpen (GMT, GMT->current.setting.map_grid_pen[0]));
			break;
		case GMTCASE_GRID_PEN_SECONDARY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_PEN_SECONDARY:
			sprintf (value, "%s", GMT_putpen (GMT, GMT->current.setting.map_grid_pen[1]));
			break;
		case GMTCASE_LABEL_OFFSET:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LABEL_OFFSET:
			sprintf (value, "%g%c", GMT->current.setting.map_label_offset GMT_def(GMTCASE_MAP_LABEL_OFFSET));
			break;
		case GMTCASE_LINE_STEP:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LINE_STEP:
			sprintf (value, "%g%c", GMT->current.setting.map_line_step GMT_def(GMTCASE_MAP_LINE_STEP));
			break;
		case GMTCASE_UNIX_TIME:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LOGO:
			sprintf (value, "%s", ft[GMT->current.setting.map_logo]);
			break;
		case GMTCASE_UNIX_TIME_POS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LOGO_POS:
			sprintf (value, "%s/%g%c/%g%c", GMT_just_string[GMT->current.setting.map_logo_justify],
			GMT->current.setting.map_logo_pos[GMT_X] GMT_def(GMTCASE_MAP_LOGO_POS),
			GMT->current.setting.map_logo_pos[GMT_Y] GMT_def(GMTCASE_MAP_LOGO_POS));
			break;
		case GMTCASE_X_ORIGIN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ORIGIN_X:
			sprintf (value, "%g%c", GMT->current.setting.map_origin[GMT_X] GMT_def(GMTCASE_MAP_ORIGIN_X));
			break;
		case GMTCASE_Y_ORIGIN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ORIGIN_Y:
			sprintf (value, "%g%c", GMT->current.setting.map_origin[GMT_Y] GMT_def(GMTCASE_MAP_ORIGIN_Y));
			break;
		case GMTCASE_POLAR_CAP:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_POLAR_CAP:
			if (doubleAlmostEqual (GMT->current.setting.map_polar_cap[0], 90.0))
				sprintf (value, "none");
			else
				sprintf (value, "%g/%g", GMT->current.setting.map_polar_cap[0], GMT->current.setting.map_polar_cap[1]);
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			sprintf (value, "%g%c", GMT->current.setting.map_scale_height GMT_def(GMTCASE_MAP_SCALE_HEIGHT));
			break;
		case GMTCASE_MAP_TICK_LENGTH:
		case GMTCASE_TICK_LENGTH:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_TICK_LENGTH_PRIMARY:
			sprintf (value, "%g%c/%g%c",
			GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] GMT_def(GMTCASE_MAP_TICK_LENGTH_PRIMARY),
			GMT->current.setting.map_tick_length[GMT_TICK_UPPER] GMT_def(GMTCASE_MAP_TICK_LENGTH_PRIMARY));
			break;
		case GMTCASE_MAP_TICK_LENGTH_SECONDARY:
			sprintf (value, "%g%c/%g%c",
			GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] GMT_def(GMTCASE_MAP_TICK_LENGTH_SECONDARY),
			GMT->current.setting.map_tick_length[GMT_TICK_LOWER] GMT_def(GMTCASE_MAP_TICK_LENGTH_SECONDARY));
			break;
		case GMTCASE_MAP_TICK_PEN:
		case GMTCASE_TICK_PEN:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_TICK_PEN_PRIMARY:
			sprintf (value, "%s", GMT_putpen (GMT, GMT->current.setting.map_tick_pen[0]));
			break;
		case GMTCASE_MAP_TICK_PEN_SECONDARY:
			sprintf (value, "%s", GMT_putpen (GMT, GMT->current.setting.map_tick_pen[1]));
			break;
		case GMTCASE_HEADER_OFFSET:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_TITLE_OFFSET:
			sprintf (value, "%g%c", GMT->current.setting.map_title_offset GMT_def(GMTCASE_MAP_TITLE_OFFSET));
			break;
		case GMTCASE_VECTOR_SHAPE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_VECTOR_SHAPE:
			sprintf (value, "%g", GMT->current.setting.map_vector_shape);
			break;

		/* COLOR GROUP */

		case GMTCASE_COLOR_BACKGROUND:
			sprintf (value, "%s", GMT_putcolor (GMT, GMT->current.setting.color_patch[GMT_BGD]));
			break;
		case GMTCASE_COLOR_FOREGROUND:
			sprintf (value, "%s", GMT_putcolor (GMT, GMT->current.setting.color_patch[GMT_FGD]));
			break;
		case GMTCASE_COLOR_MODEL:
			if (GMT->current.setting.color_model == GMT_RGB)
				strcpy (value, "none");
			else if (GMT->current.setting.color_model == (GMT_RGB + GMT_COLORINT))
				strcpy (value, "rgb");
			else if (GMT->current.setting.color_model == (GMT_CMYK + GMT_COLORINT))
				strcpy (value, "cmyk");
			else if (GMT->current.setting.color_model == (GMT_HSV + GMT_COLORINT))
				strcpy (value, "hsv");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_COLOR_NAN:
			sprintf (value, "%s", GMT_putcolor (GMT, GMT->current.setting.color_patch[GMT_NAN]));
			break;
		case GMTCASE_HSV_MIN_SATURATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MIN_S:
			sprintf (value, "%g", GMT->current.setting.color_hsv_min_s);
			break;
		case GMTCASE_HSV_MAX_SATURATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MAX_S:
			sprintf (value, "%g", GMT->current.setting.color_hsv_max_s);
			break;
		case GMTCASE_HSV_MIN_VALUE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MIN_V:
			sprintf (value, "%g", GMT->current.setting.color_hsv_min_v);
			break;
		case GMTCASE_HSV_MAX_VALUE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MAX_V:
			sprintf (value, "%g", GMT->current.setting.color_hsv_max_v);
			break;

		/* PS GROUP */

		case GMTCASE_CHAR_ENCODING:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_CHAR_ENCODING:
			strncpy (value, GMT->current.setting.ps_encoding.name, GMT_LEN256);
			break;
		case GMTCASE_PS_COLOR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_COLOR_MODEL:
			if (GMT->current.setting.ps_color_mode == PSL_RGB)
				strcpy (value, "rgb");
			else if (GMT->current.setting.ps_color_mode == PSL_CMYK)
				strcpy (value, "cmyk");
			else if (GMT->current.setting.ps_color_mode == PSL_HSV)
				strcpy (value, "hsv");
			else if (GMT->current.setting.ps_color_mode == PSL_GRAY)
				strcpy (value, "gray");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_N_COPIES:
		case GMTCASE_PS_COPIES:
			if (GMT_compat_check (GMT, 4)) {
				GMT_COMPAT_WARN;
				sprintf (value, "%d", GMT->current.setting.ps_copies);
			}
			else
				error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_DOTS_PR_INCH:
		case GMTCASE_PS_DPI: GMT_COMPAT_WARN;
			if (!GMT_compat_check (GMT, 4)) error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_PS_EPS: GMT_COMPAT_WARN;
			if (!GMT_compat_check (GMT, 4)) error = gmt_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_PS_IMAGE_COMPRESS:
			if (!GMT->PSL) return (NULL);	/* Not using PSL in this session */
			if (GMT->PSL->internal.compress == PSL_NONE)
				strcpy (value, "none");
			else if (GMT->PSL->internal.compress == PSL_RLE)
				strcpy (value, "rle");
			else if (GMT->PSL->internal.compress == PSL_LZW)
				strcpy (value, "lzw");
			else if (GMT->PSL->internal.compress == PSL_DEFLATE) {
				if (GMT->PSL->internal.deflate_level != 0)
					sprintf (value, "deflate,%u", GMT->PSL->internal.deflate_level);
				else
					strcpy (value, "deflate");
			}
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PS_LINE_CAP:
			if (!GMT->PSL) return (NULL);	/* Not using PSL in this session */
			if (GMT->PSL->internal.line_cap == PSL_BUTT_CAP)
				strcpy (value, "butt");
			else if (GMT->PSL->internal.line_cap == PSL_ROUND_CAP)
				strcpy (value, "round");
			else if (GMT->PSL->internal.line_cap == PSL_SQUARE_CAP)
				strcpy (value, "square");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PS_LINE_JOIN:
			if (!GMT->PSL) return (NULL);	/* Not using PSL in this session */
			if (GMT->PSL->internal.line_join == PSL_MITER_JOIN)
				strcpy (value, "miter");
			else if (GMT->PSL->internal.line_join == PSL_ROUND_JOIN)
				strcpy (value, "round");
			else if (GMT->PSL->internal.line_join == PSL_BEVEL_JOIN)
				strcpy (value, "bevel");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PS_MITER_LIMIT:
			if (!GMT->PSL) return (NULL);	/* Not using PSL in this session */
			sprintf (value, "%d", GMT->PSL->internal.miter_limit);
			break;
		case GMTCASE_PAGE_COLOR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_PAGE_COLOR:
			sprintf (value, "%s", GMT_putcolor (GMT, GMT->current.setting.ps_page_rgb));
			break;
		case GMTCASE_PAGE_ORIENTATION:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_PAGE_ORIENTATION:
			if (GMT->current.setting.ps_orientation == PSL_LANDSCAPE)
				strcpy (value, "landscape");
			else if (GMT->current.setting.ps_orientation == PSL_PORTRAIT)
				strcpy (value, "portrait");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PAPER_MEDIA:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_MEDIA:
			if (GMT->current.setting.ps_media == -USER_MEDIA_OFFSET)
				sprintf (value, "%gx%g", fabs (GMT->current.setting.ps_page_size[0]), fabs (GMT->current.setting.ps_page_size[1]));
			else if (GMT->current.setting.ps_media >= USER_MEDIA_OFFSET)
				sprintf (value, "%s", GMT->session.user_media_name[GMT->current.setting.ps_media-USER_MEDIA_OFFSET]);
			else
				sprintf (value, "%s", GMT_media_name[GMT->current.setting.ps_media]);
			if (GMT->current.setting.ps_page_size[0] < 0.0)
				strcat (value, "-");
			else if (GMT->current.setting.ps_page_size[1] < 0.0)
				strcat (value, "+");
			break;
		case GMTCASE_GLOBAL_X_SCALE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_SCALE_X:
			sprintf (value, "%g", GMT->current.setting.ps_magnify[GMT_X]);
			break;
		case GMTCASE_GLOBAL_Y_SCALE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_SCALE_Y:
			sprintf (value, "%g", GMT->current.setting.ps_magnify[GMT_Y]);
			break;
		case GMTCASE_TRANSPARENCY:
			if (GMT_compat_check (GMT, 4))
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Transparency is now part of pen and fill specifications.  TRANSPARENCY is ignored\n");
			else
				error = gmt_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_TRANSPARENCY:
			strncpy (value, GMT->current.setting.ps_transpmode, 15U);
			break;
		case GMTCASE_PS_VERBOSE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_COMMENTS:
			if (!GMT->PSL) return (NULL);	/* Not using PSL in this session */
			sprintf (value, "%s", ft[GMT->PSL->internal.comments]);
			break;

		/* IO GROUP */

		case GMTCASE_FIELD_DELIMITER:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_COL_SEPARATOR:
			if (GMT->current.setting.io_col_separator[0] == '\t')	/* DEFAULT */
				strcpy (value, "tab");
			else if (GMT->current.setting.io_col_separator[0] == ' ')
				strcpy (value, "space");
			else if (GMT->current.setting.io_col_separator[0] == ',')
				strcpy (value, "comma");
			else if (!GMT->current.setting.io_col_separator[0])
				strcpy (value, "none");
			else
				strncpy (value, GMT->current.setting.io_col_separator, GMT_LEN256);
			break;
		case GMTCASE_GRIDFILE_FORMAT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_GRIDFILE_FORMAT:
			strncpy (value, GMT->current.setting.io_gridfile_format, GMT_LEN256);
			break;
		case GMTCASE_GRIDFILE_SHORTHAND:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_GRIDFILE_SHORTHAND:
			sprintf (value, "%s", ft[GMT->current.setting.io_gridfile_shorthand]);
			break;
		case GMTCASE_IO_HEADER:
			sprintf (value, "%s", ft[GMT->current.setting.io_header[GMT_IN]]);
			break;
		case GMTCASE_N_HEADER_RECS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_N_HEADER_RECS:
			sprintf (value, "%d", GMT->current.setting.io_n_header_items);
			break;
		case GMTCASE_NAN_RECORDS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_NAN_RECORDS:
			if (GMT->current.setting.io_nan_records)
				strcpy (value, "pass");
			else
				strcpy (value, "skip");
			break;
		case GMTCASE_IO_NC4_CHUNK_SIZE:
			if (GMT->current.setting.io_nc4_chunksize[0] == k_netcdf_io_chunked_auto)
				strcpy (value, "auto");
			else if (GMT->current.setting.io_nc4_chunksize[0] == k_netcdf_io_classic)
				strcpy (value, "classic");
			else
				sprintf (value, "%" PRIuS ",%" PRIuS, /* chunk size: lat,lon */
						 GMT->current.setting.io_nc4_chunksize[0],
						 GMT->current.setting.io_nc4_chunksize[1]);
			break;
		case GMTCASE_IO_NC4_DEFLATION_LEVEL:
			sprintf (value, "%u", GMT->current.setting.io_nc4_deflation_level);
			break;
		case GMTCASE_XY_TOGGLE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_LONLAT_TOGGLE:
			if (GMT->current.setting.io_lonlat_toggle[GMT_IN] && GMT->current.setting.io_lonlat_toggle[GMT_OUT])
				strcpy (value, "true");
			else if (!GMT->current.setting.io_lonlat_toggle[GMT_IN] && !GMT->current.setting.io_lonlat_toggle[GMT_OUT])
				strcpy (value, "false");
			else if (GMT->current.setting.io_lonlat_toggle[GMT_IN] && !GMT->current.setting.io_lonlat_toggle[GMT_OUT])
				strcpy (value, "in");
			else
				strcpy (value, "out");
			break;

		case GMTCASE_IO_SEGMENT_BINARY:
			if (GMT->current.setting.n_bin_header_cols == 0)
				strcpy (value, "off");
			else
				sprintf (value, "%" PRIu64, GMT->current.setting.n_bin_header_cols);
			break;

		case GMTCASE_IO_SEGMENT_MARKER:
			value[0] = '\0';
			if (GMT->current.setting.io_seg_marker[GMT_OUT] != GMT->current.setting.io_seg_marker[GMT_IN]) {
				if ((GMT->current.setting.io_seg_marker[GMT_IN] == 'N' && !GMT->current.setting.io_nanline[GMT_IN]) || (GMT->current.setting.io_seg_marker[GMT_IN] == 'B' && !GMT->current.setting.io_blankline[GMT_IN])) value[0] = '\\';
				sprintf (txt, "%c,", GMT->current.setting.io_seg_marker[GMT_IN]);	strcat (value, txt);
				if ((GMT->current.setting.io_seg_marker[GMT_IN] == 'N' && !GMT->current.setting.io_nanline[GMT_IN]) || (GMT->current.setting.io_seg_marker[GMT_IN] == 'B' && !GMT->current.setting.io_blankline[GMT_IN])) strcat (value, "\\");
				sprintf (txt, "%c", GMT->current.setting.io_seg_marker[GMT_OUT]);	strcat (value, txt);
			}
			else {
				if ((GMT->current.setting.io_seg_marker[GMT_IN] == 'N' && !GMT->current.setting.io_nanline[GMT_IN]) || (GMT->current.setting.io_seg_marker[GMT_IN] == 'B' && !GMT->current.setting.io_blankline[GMT_IN])) value[0] = '\\';
				sprintf (txt, "%c", GMT->current.setting.io_seg_marker[GMT_IN]);	strcat (value, txt);
			}
			break;

		/* PROJ GROUP */

		case GMTCASE_PROJ_AUX_LATITUDE:
			switch (GMT->current.setting.proj_aux_latitude) {
				case GMT_LATSWAP_NONE:
					strcpy (value, "none");
					break;
				case GMT_LATSWAP_G2A:
					strcpy (value, "authalic");
					break;
				case GMT_LATSWAP_G2C:
					strcpy (value, "conformal");
					break;
				case GMT_LATSWAP_G2M:
					strcpy (value, "meridional");
					break;
				case GMT_LATSWAP_G2O:
					strcpy (value, "geocentric");
					break;
				case GMT_LATSWAP_G2P:
					strcpy (value, "parametric");
					break;
				default:
					strcpy (value, "undefined");
			}
			break;

		case GMTCASE_ELLIPSOID:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_ELLIPSOID:
			if (GMT->current.setting.proj_ellipsoid < GMT_N_ELLIPSOIDS - 1)	/* Custom ellipse */
				sprintf (value, "%s", GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].name);
			else if (GMT_IS_SPHERICAL (GMT))
				sprintf (value, "%f", GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius);
			else
				sprintf (value, "%f,%f", GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius,
					1.0/GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening);
			break;
		case GMTCASE_PROJ_DATUM:	/* Not implemented yet */
			break;
		case GMTCASE_PROJ_GEODESIC:
			switch (GMT->current.setting.proj_geodesic) {
				case GMT_GEODESIC_VINCENTY:
					strcpy (value, "Vincenty");
					break;
				case GMT_GEODESIC_ANDOYER:
					strcpy (value, "Andoyer");
					break;
				case GMT_GEODESIC_RUDOE:
					strcpy (value, "Rudoe");
					break;
				default:
					strcpy (value, "undefined");
			}
			break;

		case GMTCASE_MEASURE_UNIT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_LENGTH_UNIT:
			sprintf (value, "%s", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			break;
		case GMTCASE_PROJ_MEAN_RADIUS:
			switch (GMT->current.setting.proj_mean_radius) {
				case GMT_RADIUS_MEAN:
					strcpy (value, "mean");
					break;
				case GMT_RADIUS_AUTHALIC:
					strcpy (value, "authalic");
					break;
				case GMT_RADIUS_VOLUMETRIC:
					strcpy (value, "volumetric");
					break;
				case GMT_RADIUS_MERIDIONAL:
					strcpy (value, "meridional");
					break;
				case GMT_RADIUS_QUADRATIC:
					strcpy (value, "quadratic");
					break;
				default:
					strcpy (value, "undefined");
			}
			break;
		case GMTCASE_MAP_SCALE_FACTOR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_SCALE_FACTOR:
			if (doubleAlmostEqual (GMT->current.setting.proj_scale_factor, -1.0)) /* Default scale for chosen projection */
				strcpy (value, "default"); /* Default scale for chosen projection */
			else
				sprintf (value, "%g", GMT->current.setting.proj_scale_factor);
			break;

		/* GMT GROUP */

		case GMTCASE_GMT_COMPATIBILITY:
			sprintf (value, "%u", GMT->current.setting.compatibility);
			break;

		case GMTCASE_GMT_CUSTOM_LIBS:
			strncpy (value, (GMT->session.CUSTOM_LIBS) ? GMT->session.CUSTOM_LIBS : "", GMT_LEN256);
			break;
		case GMTCASE_GMT_EXTRAPOLATE_VAL:
			if (GMT->current.setting.extrapolate_val[0] == GMT_EXTRAPOLATE_NONE)
				strcpy (value, "NaN");
			else if (GMT->current.setting.extrapolate_val[0] == GMT_EXTRAPOLATE_SPLINE)
				strcpy (value, "extrap");
			else if (GMT->current.setting.extrapolate_val[0] == GMT_EXTRAPOLATE_CONSTANT)
				sprintf (value, "extrapval,%g", GMT->current.setting.extrapolate_val[1]);
			break;
		case GMTCASE_GMT_FFT:
			switch (GMT->current.setting.fft) {
				case k_fft_auto:
					strcpy (value, "auto");
					break;
				case k_fft_kiss:
					strcpy (value, "kissfft");
					break;
				case k_fft_brenner:
					strcpy (value, "brenner");
					break;
				case k_fft_fftw:
					strcpy (value, "fftw");
#ifdef HAVE_FFTW3F
					switch (GMT->current.setting.fftw_plan) {
						case FFTW_MEASURE:
							strcat (value, ",measure");
							break;
						case FFTW_PATIENT:
							strcat (value, ",patient");
							break;
						case FFTW_EXHAUSTIVE:
							strcat (value, ",exhaustive");
							break;
						default:
							strcat (value, ",estimate");
					}
#endif /* HAVE_FFTW3F */
					break;
				case k_fft_accelerate:
					strcpy (value, "accelerate");
					break;
				default:
					strcpy (value, "undefined");
			}
			break;
		case GMTCASE_HISTORY:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_HISTORY:
			if (GMT->current.setting.history & k_history_write)
				sprintf (value, "true");
			else if (GMT->current.setting.history & k_history_read)
				sprintf (value, "readonly");
			else
				sprintf (value, "false");
			break;
		case GMTCASE_INTERPOLANT:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_INTERPOLANT:
			if (GMT->current.setting.interpolant == GMT_SPLINE_LINEAR)
				strcpy (value, "linear");
			else if (GMT->current.setting.interpolant == GMT_SPLINE_AKIMA)
				strcpy (value, "akima");
			else if (GMT->current.setting.interpolant == GMT_SPLINE_CUBIC)
				strcpy (value, "cubic");
			else if (GMT->current.setting.interpolant == GMT_SPLINE_NONE)
				strcpy (value, "none");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_GMT_TRIANGULATE:
			if (GMT->current.setting.triangulate == GMT_TRIANGLE_WATSON)
				strcpy (value, "Watson");
			else if (GMT->current.setting.triangulate == GMT_TRIANGLE_SHEWCHUK)
				strcpy (value, "Shewchuk");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_VERBOSE:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_VERBOSE:
			switch (GMT->current.setting.verbose) {
				case GMT_MSG_QUIET:		strcpy (value, "quiet");	break;
				case GMT_MSG_NORMAL:		strcpy (value, "normal");	break;
				case GMT_MSG_VERBOSE:	strcpy (value, "verbose");	break;
				case GMT_MSG_LONG_VERBOSE:	strcpy (value, "long_verbose");	break;
				case GMT_MSG_DEBUG:		strcpy (value, "debug");	break;
				default:				strcpy (value, "compat");	break;
			}
			break;

		/* DIR GROUP */

		case GMTCASE_DIR_DATA:
			/* Force update of session.DATADIR before copying the string */
			strncpy (value, (GMT->session.DATADIR) ? GMT->session.DATADIR : "", GMT_LEN256);
			break;
		case GMTCASE_DIR_DCW:
			/* Force update of session.DCWDIR before copying the string */
			strncpy (value, (GMT->session.DCWDIR) ? GMT->session.DCWDIR : "", GMT_LEN256);
			break;
		case GMTCASE_DIR_GSHHG:
			/* Force update of session.GSHHGDIR before copying the string */
			GMT_shore_adjust_res (GMT, 'c');
			strncpy (value, (GMT->session.GSHHGDIR) ? GMT->session.GSHHGDIR : "", GMT_LEN256);
			break;

		/* TIME GROUP */

		case GMTCASE_TIME_EPOCH:
			strncpy (value, GMT->current.setting.time_system.epoch, GMT_LEN64);
			break;
		case GMTCASE_TIME_IS_INTERVAL:
			if (GMT->current.setting.time_is_interval)
				sprintf (value, "%c%d%c", pm[GMT->current.time.truncate.direction], GMT->current.time.truncate.T.step, GMT->current.time.truncate.T.unit);
			else
				sprintf (value, "off");
			break;
		case GMTCASE_TIME_INTERVAL_FRACTION:
			sprintf (value, "%g", GMT->current.setting.time_interval_fraction);
			break;
		case GMTCASE_GMT_LANGUAGE:
		case GMTCASE_TIME_LANGUAGE:
			strncpy (value, GMT->current.setting.language, GMT_LEN64);
			break;
		case GMTCASE_WANT_LEAP_SECONDS:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_TIME_LEAP_SECONDS:
			sprintf (value, "%s", ft[GMT->current.setting.time_leap_seconds]);
			break;
		case GMTCASE_TIME_REPORT:
			if (GMT->current.setting.timer_mode == GMT_NO_TIMER)
				strcpy (value, "none");
			else if (GMT->current.setting.timer_mode == GMT_ABS_TIMER)
				strcpy (value, "clock");
			else if (GMT->current.setting.timer_mode == GMT_ELAPSED_TIMER)
				strcpy (value, "elapsed");
			break;
		case GMTCASE_TIME_UNIT:
			value[0] = GMT->current.setting.time_system.unit;
			break;
		case GMTCASE_TIME_WEEK_START:
			sprintf (value, "%s", GMT_weekdays[GMT->current.setting.time_week_start]);
			break;
		case GMTCASE_Y2K_OFFSET_YEAR:
			if (GMT_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmt_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_TIME_Y2K_OFFSET_YEAR:
			sprintf (value, "%d", GMT->current.setting.time_Y2K_offset_year);
			break;

		default:
			error = true; /* keyword not known */
			break;
	}
	if (error)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized keyword %s\n", keyword);
	return (value);
}

/*! . */
int GMT_pickdefaults (struct GMT_CTRL *GMT, bool lines, struct GMT_OPTION *options)
{
	int error = GMT_OK, n = 0;
	struct GMT_OPTION *opt = NULL;
	char *param;

	for (opt = options; opt; opt = opt->next) {
		if (!(opt->option == '<' || opt->option == '#') || !opt->arg)
			continue;		/* Skip other and empty options */
		if (!lines && n)
			GMT->parent->print_func (GMT->session.std[GMT_OUT], " ");	/* Separate by spaces */
		param = GMT_putparameter (GMT, opt->arg);
		if (*param == '\0') {
			/* if keyword unknown */
			error = GMT_OPTION_NOT_FOUND;
			break;
		}
		GMT->parent->print_func (GMT->session.std[GMT_OUT], param);
		if (lines)
			GMT->parent->print_func (GMT->session.std[GMT_OUT], "\n");		/* Separate lines */
		n++;
	}
	if (!lines && n)
		GMT->parent->print_func (GMT->session.std[GMT_OUT], "\n");		/* Single lines */
	return error;
}

/*! . */
int GMT_savedefaults (struct GMT_CTRL *GMT, char *file)
{
	unsigned int error = 0, rec = 0;
	char line[GMT_BUFSIZ] = {""}, keyword[GMT_LEN256] = {""}, string[GMT_LEN256] = {""};
	FILE *fpi = NULL, *fpo = NULL;

	if (file[0] == '-' && !file[1])
		fpo = GMT->session.std[GMT_OUT];
	else if ((fpo = fopen (file, "w")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not create file %s\n", file);
		return (-1);
	}

	/* Find the global gmt.conf file */

	sprintf (line, "%s/conf/gmt.conf", GMT->session.SHAREDIR);
	if (access (line, R_OK)) {
		/* Not found in SHAREDIR, try USERDIR instead */
		if (GMT_getuserpath (GMT, "conf/gmt.conf", line) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not find system defaults file - Aborting.\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
	}
	if ((fpi = fopen (line, "r")) == NULL) return (-1);

	while (fgets (line, GMT_BUFSIZ, fpi)) {
		rec++;
		GMT_chop (line);	/* Get rid of [\r]\n */
		if (rec == 2) {	/* Copy version from gmt.conf */
			sscanf (line, "# GMT %s", string);
			fprintf (fpo, "# GMT %s Defaults file\n", string);
			continue;
		}
		if (line[0] == '#') { /* Copy comments */
			fprintf (fpo, "%s\n", line);
			continue;
		}
		if (line[0] == '\0') continue;	/* Skip Blank lines */

		/* Read the keyword and get the value from memory */
		keyword[0] = '\0';
		sscanf (line, "%s", keyword);

		/* Write things out (with possible tabs, spaces) */
		sscanf (line, "%[^=]", string);
		fprintf (fpo, "%s= %s\n", string, GMT_putparameter (GMT, keyword));
	}

	fclose (fpi);
	if (fpo != GMT->session.std[GMT_OUT]) fclose (fpo);

	if (error) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: %d GMT Defaults conversion errors while writing gmt.conf\n", error);

	return (0);
}

/*! Dumps the GMT parameters to file or standard output */
void GMT_putdefaults (struct GMT_CTRL *GMT, char *this_file) {
	/* ONLY USED BY GMTSET AND GMTDEFAULTS */
	if (this_file)	/* File name is defined: use it */
		GMT_savedefaults (GMT, this_file);
	else if (GMT->session.TMPDIR) {	/* Write GMT->session.TMPDIR/gmt.conf */
		char *path = NULL;
		path = GMT_memory (GMT, NULL, strlen (GMT->session.TMPDIR) + 10, char);
		sprintf (path, "%s/gmt.conf", GMT->session.TMPDIR);
		GMT_savedefaults (GMT, path);
		GMT_free (GMT, path);
	}
	else	/* Write gmt.conf in current directory */
		GMT_savedefaults (GMT, "gmt.conf");
}

/*! Read user's gmt.conf file and initialize parameters */
void GMT_getdefaults (struct GMT_CTRL *GMT, char *this_file) {
	char file[GMT_BUFSIZ];

	if (this_file)	/* Defaults file is specified */
		GMT_loaddefaults (GMT, this_file);
	else if (GMT_getuserpath (GMT, "gmt.conf", file))
		GMT_loaddefaults (GMT, file);
}

/*! . */
void gmt_append_trans (char *text, double transparency) {
	char trans[GMT_LEN64] = {""};
	if (!GMT_IS_ZERO (transparency) && text[0] != '-') {	/* Append nonzero transparency */
		sprintf (trans, "@%ld", lrint (100.0 * transparency));
		strcat (text, trans);
	}
}

/*! Creates the name (if equivalent) or the string r[/g/b] corresponding to the RGB triplet or a pattern.
 * Example: GMT_putfill (GMT, fill) may produce "white" or "1/2/3" or "p300/7"
 */
char *GMT_putfill (struct GMT_CTRL *GMT, struct GMT_FILL *F) {

	static char text[GMT_LEN256] = {""};
	int i;

	if (F->use_pattern) {
		if (F->pattern_no)
			sprintf (text, "p%d/%d", F->dpi, F->pattern_no);
		else
			sprintf (text, "p%d/%s", F->dpi, F->pattern);
	}
	else if (F->rgb[0] < -0.5)
		sprintf (text, "-");
	else if ((i = GMT_getrgb_index (GMT, F->rgb)) >= 0)
		sprintf (text, "%s", GMT_color_name[i]);
	else if (GMT_is_gray (F->rgb))
		sprintf (text, "%.5g", GMT_q(GMT_s255(F->rgb[0])));
	else
		sprintf (text, "%.5g/%.5g/%.5g", GMT_t255(F->rgb));
	gmt_append_trans (text, F->rgb[3]);
	return (text);
}

/*! Creates the name (if equivalent) or the string r[/g/b] corresponding to the RGB triplet.
 * Example: GMT_putcolor (GMT, rgb) may produce "white" or "1/2/3"
 */
char *GMT_putcolor (struct GMT_CTRL *GMT, double *rgb) {

	static char text[GMT_LEN256] = {""};
	int i;

	if (rgb[0] < -0.5)
		sprintf (text, "-");
	else if ((i = GMT_getrgb_index (GMT, rgb)) >= 0)
		sprintf (text, "%s", GMT_color_name[i]);
	else if (GMT_is_gray(rgb))
		sprintf (text, "%.5g", GMT_q(GMT_s255(rgb[0])));
	else
		sprintf (text, "%.5g/%.5g/%.5g", GMT_t255(rgb));
	gmt_append_trans (text, rgb[3]);
	return (text);
}

/*! Creates t the string r/g/b corresponding to the RGB triplet */
char *GMT_putrgb (struct GMT_CTRL *GMT, double *rgb) {

	static char text[GMT_LEN256] = {""};
	GMT_UNUSED(GMT);

	if (rgb[0] < -0.5)
		sprintf (text, "-");
	else
		sprintf (text, "%.5g/%.5g/%.5g", GMT_t255(rgb));
	gmt_append_trans (text, rgb[3]);
	return (text);
}

/*! Creates the string c/m/y/k corresponding to the CMYK quadruplet */
char *GMT_putcmyk (struct GMT_CTRL *GMT, double *cmyk) {

	static char text[GMT_LEN256] = {""};
	GMT_UNUSED(GMT);

	if (cmyk[0] < -0.5)
		sprintf (text, "-");
	else
		sprintf (text, "%.5g/%.5g/%.5g/%.5g", GMT_q(cmyk[0]), GMT_q(cmyk[1]), GMT_q(cmyk[2]), GMT_q(cmyk[3]));
	gmt_append_trans (text, cmyk[4]);
	return (text);
}

/*! Creates the string h/s/v corresponding to the HSV triplet */
char *GMT_puthsv (struct GMT_CTRL *GMT, double *hsv) {

	static char text[GMT_LEN256] = {""};
	GMT_UNUSED(GMT);

	if (hsv[0] < -0.5)
		sprintf (text, "-");
	else
		sprintf (text, "%.5g-%.5g-%.5g", GMT_q(hsv[0]), GMT_q(hsv[1]), GMT_q(hsv[2]));
	gmt_append_trans (text, hsv[3]);
	return (text);
}

/*! Checks if t fits the format [+|-][xxxx][.][yyyy][e|E[+|-]nn]. */
bool GMT_is_valid_number (char *t)
{
	int i, n;


	if (!t) return (true);				/* Cannot be NULL */
	i = n = 0;
	if (t[i] == '+' || t[i] == '-') i++;		/* OK to have leading sign */
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
	return ((t[i] || n == 0) ? false : true);
}

/*! . */
double GMT_convert_units (struct GMT_CTRL *GMT, char *string, unsigned int default_unit, unsigned int target_unit)
{
	/* Converts the input string "value" to a float in units indicated by target_unit
	 * If value does not contain a unit (''c', 'i', or p') then the units indicated
	 * by default_unit will be used.
	 * Both target_unit and default_unit are either GMT_PT, GMT_CM, GMT_INCH or GMT_M.
	 */

	int c = 0, len, given_unit;
	bool have_unit = false;
	double value;

	if ((len = (int)strlen(string))) {
		c = string[len-1];
		if ((have_unit = isalpha ((int)c))) string[len-1] = '\0';	/* Temporarily remove unit */
	}

	/* So c is either 0 (meaning default unit) or any letter (even junk like z) */

	given_unit = GMT_unit_lookup (GMT, c, default_unit);	/* Will warn if c is not 0, 'c', 'i', 'p' */

	if (!GMT_is_valid_number (string))
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %s not a valid number and may not be decoded properly.\n", string);

	value = atof (string) * GMT->session.u2u[given_unit][target_unit];
	if (have_unit) string[len-1] = (char)GMT->session.unit_name[given_unit][0];	/* Put back the (implied) given unit */

	return (value);
}

/*! . */
unsigned int GMT_unit_lookup (struct GMT_CTRL *GMT, int c, unsigned int unit)
{
	if (!isalpha ((int)c))	/* Not a unit modifier - just return the current default unit */
		return (unit);

	/* Now we check for the c-i-p units and barf otherwise */

	switch (c) {
		case 'c': case 'C':	/* Centimeters */
			unit = GMT_CM;
			break;
		case 'i': case 'I':	/* Inches */
			unit = GMT_INCH;
			break;
		case 'p': case 'P':	/* Points */
			unit = GMT_PT;
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Length unit %c not supported - revert to default unit [%s]\n", (int)c, GMT->session.unit_name[unit]);
			break;
	}

	return (unit);
}

/*! . */
int GMT_hash (struct GMT_CTRL *GMT, char *v, unsigned int n_hash) {
	int h;
	GMT_UNUSED(GMT);
	assert (v!=NULL); /* We are in trouble if we get a NULL pointer here */
	for (h = 0; *v != '\0'; v++) h = (64 * h + (*v)) % n_hash;
	while (h < 0) h += n_hash;
	return (h);
}

/*! . */
int GMT_hash_lookup (struct GMT_CTRL *GMT, char *key, struct GMT_HASH *hashnode, unsigned int n, unsigned int n_hash)
{
	int i;
	unsigned int ui, k;

	i = GMT_hash (GMT, key, n_hash);			/* Get initial hash key */

	if (i < 0 || (ui = i) >= n) return (-1);	/* Bad key */
	if (hashnode[ui].n_id == 0) return (-1);	/* No entry for this hash value */
	/* Must search among the entries with identical hash value ui, starting at item k = 0 */
	k = 0;
	while (k < hashnode[ui].n_id && strcmp (hashnode[ui].key[k], key)) k++;
	if (k == hashnode[ui].n_id) return (-1);	/* Bad key; no match found */
	return (hashnode[ui].id[k]);			/* Return array index that goes with this key */
}

/*! Set up hash table */
int GMT_hash_init (struct GMT_CTRL *GMT, struct GMT_HASH *hashnode, char **keys, unsigned int n_hash, unsigned int n_keys)
{
	unsigned int i, next;
	int entry;

	GMT_memset (hashnode, n_hash, struct GMT_HASH);	/* Start with NULL everywhere */
	for (i = 0; i < n_keys; i++) {
		entry = GMT_hash (GMT, keys[i], n_hash);
		next = hashnode[entry].n_id;
		if (next == GMT_HASH_MAXDEPTH) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s makes hash-depth exceed hard-wired limit of %d - increment GMT_HASH_MAXDEPTH in gmt_hash.h and recompile GMT\n", keys[i], GMT_HASH_MAXDEPTH);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		hashnode[entry].key[next] = keys[i];
		hashnode[entry].id[next]  = i;
		hashnode[entry].n_id++;
	}
	return GMT_OK;
}

/*! Return ID of requested ellipsoid, or -1 if not found */
int GMT_get_ellipsoid (struct GMT_CTRL *GMT, char *name)
{
	int i, n;
	char line[GMT_BUFSIZ], ename[GMT_LEN64];
	double pol_radius;

	/* Try to get ellipsoid from the default list; use case-insensitive checking */

	strcpy (ename, name);		/* Make a copy of name */
	GMT_str_tolower (ename);	/* Convert to lower case */
	for (i = 0; i < GMT_N_ELLIPSOIDS; i++) {
		strcpy (line, GMT->current.setting.ref_ellipsoid[i].name);
		GMT_str_tolower (line);	/* Convert to lower case */
		if (!strcmp (ename, line)) return (i);
	}

	i = GMT_N_ELLIPSOIDS - 1;	/* Place any custom ellipsoid in this position in array */

	/* Read ellipsoid information as <a>,<finv> */
	n = sscanf (name, "%lf,%s", &GMT->current.setting.ref_ellipsoid[i].eq_radius, line);
	if (n < 1) {}	/* Failed to read arguments */
	else if (n == 1)
		GMT->current.setting.ref_ellipsoid[i].flattening = 0.0; /* Read equatorial radius only ... spherical */
	else if (line[0] == 'b') {	/* Read semi-minor axis */
		n = sscanf (&line[2], "%lf", &pol_radius);
		GMT->current.setting.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / GMT->current.setting.ref_ellipsoid[i].eq_radius);
	}
	else if (line[0] == 'f') {	/* Read flattening */
		n = sscanf (&line[2], "%lf", &GMT->current.setting.ref_ellipsoid[i].flattening);
	}
	else {				/* Read inverse flattening */
		n = sscanf (line, "%lf", &GMT->current.setting.ref_ellipsoid[i].flattening);
		if (!GMT_IS_SPHERICAL (GMT)) GMT->current.setting.ref_ellipsoid[i].flattening = 1.0 / GMT->current.setting.ref_ellipsoid[i].flattening;
	}
	if (n == 1) return (i);

	if (GMT_compat_check (GMT, 4)) {
		FILE *fp = NULL;
		char path[GMT_BUFSIZ];
		double slop;
		/* Try to open as file first in (1) current dir, then in (2) $GMT->session.SHAREDIR */

		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Assigning PROJ_ELLIPSOID a file name is deprecated, use <a>,<inv_f> instead");
		GMT_getsharepath (GMT, NULL, name, "", path, R_OK);

		if ((fp = fopen (name, "r")) != NULL || (fp = fopen (path, "r")) != NULL) {
			/* Found file, now get parameters */
			while (fgets (line, GMT_BUFSIZ, fp) && (line[0] == '#' || line[0] == '\n'));
			fclose (fp);
			n = sscanf (line, "%s %d %lf %lf %lf", GMT->current.setting.ref_ellipsoid[i].name,
				&GMT->current.setting.ref_ellipsoid[i].date, &GMT->current.setting.ref_ellipsoid[i].eq_radius,
				&pol_radius, &GMT->current.setting.ref_ellipsoid[i].flattening);
			if (n != 5) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding user ellipsoid parameters (%s)\n", line);
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}

			if (pol_radius == 0.0) {} /* Ignore semi-minor axis */
			else if (GMT_IS_SPHERICAL (GMT)) {
				/* zero flattening means we must compute flattening from the polar and equatorial radii: */

				GMT->current.setting.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / GMT->current.setting.ref_ellipsoid[i].eq_radius);
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "user-supplied ellipsoid has implicit flattening of %.8f\n", GMT->current.setting.ref_ellipsoid[i].flattening);
			}
			/* else check consistency: */
			else if ((slop = fabs (GMT->current.setting.ref_ellipsoid[i].flattening - 1.0 + (pol_radius/GMT->current.setting.ref_ellipsoid[i].eq_radius))) > 1.0e-8) {
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Possible inconsistency in user ellipsoid parameters (%s) [off by %g]\n", line, slop);
			}
			return (i);
		}
	}

	return (-1);
}

/*! Return ID of requested datum, or -1 if not found */
int GMT_get_datum (struct GMT_CTRL *GMT, char *name)
{
	int i;
	char dname[GMT_LEN64], current_name[GMT_LEN64];

	if (!name[0]) return (-1);	/* Nothing given */
	strcpy (dname, name);		/* Make a copy of desired datum name */
	GMT_str_tolower (dname);	/* Convert it to lower case */
	for (i = 0; i < GMT_N_DATUMS; i++) {
		strcpy (current_name, GMT->current.setting.proj_datum[i].name);		/* Make a copy of this datum name */
		GMT_str_tolower (current_name);	/* Convert it to lower case */
		if (!strcmp (dname, current_name)) return (i);	/* Found a match */
	}
	return (-1);	/* Not found */
}

/*! . */
bool GMT_get_time_system (struct GMT_CTRL *GMT, char *name, struct GMT_TIME_SYSTEM *time_system) {
	/* Convert TIME_SYSTEM into TIME_EPOCH and TIME_UNIT.
	   TIME_SYSTEM can be one of the following: j2000, jd, mjd, s1985, unix, dr0001, rata
	   or any string in the form "TIME_UNIT since TIME_EPOCH", like "seconds since 1985-01-01".
	   This function only splits the strings, no validation or analysis is done.
	   See GMT_init_time_system_structure for that.
	   TIME_SYSTEM = other is completely ignored.
	*/
	char *epoch = NULL;
	GMT_UNUSED(GMT);

	if (!strcmp (name, "j2000")) {
		strcpy (time_system->epoch, "2000-01-01T12:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "jd")) {
		strcpy (time_system->epoch, "-4713-11-24T12:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "mjd")) {
		strcpy (time_system->epoch, "1858-11-17T00:00:00");
		time_system->unit = 'd';
	}
	else if (!strcmp (name, "s1985")) {
		strcpy (time_system->epoch, "1985-01-01T00:00:00");
		time_system->unit = 's';
	}
	else if (!strcmp (name, "unix")) {
		strcpy (time_system->epoch, "1970-01-01T00:00:00");
		time_system->unit = 's';
	}
	else if (!strcmp (name, "dr0001")) {
		strcpy (time_system->epoch, "0001-01-01T00:00:00");
		time_system->unit = 's';
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
		strncpy (time_system->epoch, epoch, GMT_LEN64);
		time_system->unit = name[0];
		if (!strncmp (name, "mon", 3U)) time_system->unit = 'o';
	}
	else
		return (true);
	return (false);
}

/*! . */
int GMT_get_char_encoding (struct GMT_CTRL *GMT, char *name) {
	int i;
	GMT_UNUSED(GMT);

	for (i = 0; i < 7 && strcmp (name, GMT_weekdays[i]); i++);
	return (i);
}

/*! Read user's gmt.io file and initialize shorthand notation */
int gmt_setshorthand (struct GMT_CTRL *GMT) {
	unsigned int id, n = 0;
	size_t n_alloc = 0;
	char file[GMT_BUFSIZ] = {""}, line[GMT_BUFSIZ] = {""}, a[GMT_LEN64] = {""}, b[GMT_LEN64] = {""};
	char c[GMT_LEN64] = {""}, d[GMT_LEN64] = {""}, e[GMT_LEN64] = {""};
	FILE *fp = NULL;

	GMT->session.n_shorthands = 0; /* By default there are no shorthands unless gmt.io is found */

	if (!GMT_getuserpath (GMT, "gmt.io", file)) {
		if (!GMT_getsharepath (GMT, "", "gmt.io", "", file, R_OK)) {	/* try non-hidden file in ~/.gmt */
			if (GMT_compat_check (GMT, 4)) {	/* Look for obsolete .gmt_io files */
				if (!GMT_getuserpath (GMT, ".gmt_io", file)) {
					if (!GMT_getsharepath (GMT, "", "gmt_io", "", file, R_OK))	/* try non-hidden file in ~/.gmt */
						return GMT_OK;
				}
			}
			else
				return GMT_OK;
		}
	}
	if ((fp = fopen (file, "r")) == NULL)
		return GMT_OK;

	GMT_set_meminc (GMT, GMT_TINY_CHUNK); /* Only allocate a small amount */
	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf (line, "%s %s %s %s %s", a, b, c, d, e) != 5) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding file %s.  Bad format? [%s]\n", file, line);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}

		if (n == n_alloc)
			GMT->session.shorthand = GMT_malloc (GMT, GMT->session.shorthand, n, &n_alloc, struct GMT_SHORTHAND);

		GMT->session.shorthand[n].suffix = strdup (a);
		if (GMT_grd_format_decoder (GMT, b, &id) != GMT_NOERROR) {
			/* no valid type id */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unknown shorthand format [%s]\n", file, b);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		snprintf (line, GMT_BUFSIZ, "%s/%s/%s/%s", b, c, d, e); /* ff/scale/offset/invalid */
		GMT->session.shorthand[n].format = strdup (line);
		++n;
	}
	fclose (fp);

	n_alloc = GMT->session.n_shorthands = n;
	GMT_reset_meminc (GMT);
	GMT->session.shorthand = GMT_malloc (GMT, GMT->session.shorthand, 0, &n_alloc, struct GMT_SHORTHAND);
	return GMT_OK;
}

/*! . */
void gmt_freeshorthand (struct GMT_CTRL *GMT) {/* Free memory used by shorthand arrays */
	unsigned int i;

	if (GMT->session.n_shorthands == 0)
		return;

	for (i = 0; i < GMT->session.n_shorthands; ++i) {
		free (GMT->session.shorthand[i].suffix);
		free (GMT->session.shorthand[i].format);
	}
	GMT_free (GMT, GMT->session.shorthand);
}

#if defined (WIN32) /* Use Windows API */
#include <Windows.h>
EXTERN_MSC char *dlerror (void);

/*! . */
bool gmt_file_lock (struct GMT_CTRL *GMT, int fd) {
	OVERLAPPED over = { 0 };
	HANDLE hand = (HANDLE)_get_osfhandle(fd);
	if (!LockFileEx(hand, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &over)) /* Will block until exclusive lock is acquired */
	{
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: exclusive lock could not be acquired (%s)\n", dlerror());
		return false;
	}
	return true;
}

/*! . */
bool gmt_file_unlock (struct GMT_CTRL *GMT, int fd) {
	HANDLE hand = (HANDLE)_get_osfhandle(fd);
	if (!UnlockFile(hand, 0, 0, 0, 1))
	{
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: failed to release lock (%s)\n", dlerror());
		return false;
	}
	return true;
}

#elif defined (HAVE_FCNTL_H_) /* Use POSIX fcntl */
/*! . */
bool gmt_file_lock (struct GMT_CTRL *GMT, int fd)
{
	int status;
	struct flock lock;
	lock.l_type = F_WRLCK;		/* Lock for exclusive reading/writing */
	lock.l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock.l_start = lock.l_len = 0;

	if ((status = fcntl (fd, F_SETLKW, &lock))) /* Will block until exclusive lock is acquired */
	{
		int errsv = status; /* make copy of status */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: exclusive lock could not be acquired (%s)\n", strerror(errsv));
		return false;
	}
	return true;
}

/*! . */
bool gmt_file_unlock (struct GMT_CTRL *GMT, int fd)
{
	int status;
	struct flock lock;
	lock.l_type = F_UNLCK;		/* Release lock and close file */
	lock.l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock.l_start = lock.l_len = 0;

	if ((status = fcntl (fd, F_SETLK, &lock))) /* Release lock */
	{
		int errsv = status; /* make copy of status */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: failed to release lock (%s)\n", strerror(errsv));
		return false;
	}
	return true;
}

#else /* Not Windows and fcntl not available */
/*! . */
bool gmt_file_lock (struct GMT_CTRL *GMT, int fd) {
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: file locking not supported.\n");
	return false;
}

/*! . */
bool gmt_file_unlock (struct GMT_CTRL *GMT, int fd) {
	return false;
}
#endif

/*! . */
int gmt_get_history (struct GMT_CTRL *GMT)
{
	int id;
	size_t len = strlen ("BEGIN GMT " GMT_PACKAGE_VERSION);
	bool done = false, process = false;
	char line[GMT_BUFSIZ] = {""}, hfile[GMT_BUFSIZ] = {""}, cwd[GMT_BUFSIZ] = {""};
	char option[GMT_LEN64] = {""}, value[GMT_BUFSIZ] = {""};
	FILE *fp = NULL; /* For gmt.history file */
	static struct GMT_HASH unique_hashnode[GMT_N_UNIQUE];

	if (!(GMT->current.setting.history & k_history_read))
		return (GMT_NOERROR); /* gmt.history mechanism has been disabled */

	/* This is called once per GMT Session by GMT_Create_Session via GMT_begin and before any GMT_* module is called.
	 * It loads in the known shorthands found in the gmt.history file
	 */

	/* If current directory is writable, use it; else use the home directory */

	if (getcwd (cwd, GMT_BUFSIZ) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: Unable to determine current working directory.\n");
	}
	if (GMT->session.TMPDIR)			/* Isolation mode: Use GMT->session.TMPDIR/gmt.history */
		sprintf (hfile, "%s/gmt.history", GMT->session.TMPDIR);
	else if (!access (cwd, W_OK))		/* Current directory is writable */
		sprintf (hfile, "gmt.history");
	else	/* Try home directory instead */
		sprintf (hfile, "%s/gmt.history", GMT->session.HOMEDIR);

	if ((fp = fopen (hfile, "r+")) == NULL) /* In order to place an exclusive lock, fp must be open for writing */
		return (GMT_NOERROR);	/* OK to be unsuccessful in opening this file */

	GMT_hash_init (GMT, unique_hashnode, GMT_unique_option, GMT_N_UNIQUE, GMT_N_UNIQUE);

	/* When we get here the file exists */
	gmt_file_lock (GMT, fileno(fp));
	/* Format of GMT 5 gmt.history is as follow:
	 * BEGIN GMT <version>		This is the start of parsable section
	 * OPT ARG
	 * where OPT is a 1- or 2-char code, e.g., R, X, JM, JQ, Js.  ARG is the argument.
	 * Exception: if OPT = J then ARG is just the first character of the argument  (e.g., M).
	 * File ends when we find
	 * END				This is the end of parsable section
	 */

	while (!done && fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments lines */
		GMT_chop (line);		/* Remove linefeed,CR */
		if (line[0] == '\0') continue;	/* Skip blank lines */
		if (!strncmp (line, "BEGIN GMT " GMT_PACKAGE_VERSION, len))
			process = true;	/* OK to parse gmt.history file compatible with this GMT version */
		else if (!strncmp (line, "END", 3U)) {		/* Logical end of gmt.history file */
			done = true;
			process = false;
		}
		if (!process) continue;		/* Not inside the good stuff yet */
		if (sscanf (line, "%s %[^\n]", option, value) != 2) continue;	/* Quietly skip malformed lines */
		if (!value[0]) continue;	/* No argument found */
		if (option[0] == 'C') {	/* Read clip level */
			GMT->current.ps.clip_level = atoi (value);
			continue;
		}
		else if (option[0] == 'L') {	/* Read PS layer */
			GMT->current.ps.layer = atoi (value);
			continue;
		}
		if ((id = GMT_hash_lookup (GMT, option, unique_hashnode, GMT_N_UNIQUE, GMT_N_UNIQUE)) < 0) continue;	/* Quietly skip malformed lines */
		if (GMT->init.history[id])
			free (GMT->init.history[id]);
		GMT->init.history[id] = strdup (value);
	}

	/* Close the file */
	gmt_file_unlock (GMT, fileno(fp));
	fclose (fp);

	return (GMT_NOERROR);
}

/*! . */
int gmt_put_history (struct GMT_CTRL *GMT)
{
	int id;
	bool empty;
	char hfile[GMT_BUFSIZ] = {""}, cwd[GMT_BUFSIZ] = {""};
	FILE *fp = NULL; /* For gmt.history file */

	if (!(GMT->current.setting.history & k_history_write))
		return (GMT_NOERROR); /* gmt.history mechanism has been disabled */

	/* This is called once per GMT Session by GMT_end via GMT_Destroy_Session.
	 * It writes out the known shorthands to the gmt.history file
	 */

	/* Do we even need to write? If empty, simply skip */
	for (id = 0, empty = true; id < GMT_N_UNIQUE && empty; id++) {
		if (GMT->init.history[id]) empty = false;	/* Have something to write */
	}
	if (empty) return (GMT_NOERROR);

	/* If current directory is writable, use it; else use the home directory */

	if (getcwd (cwd, GMT_BUFSIZ) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: Unable to determine current working directory.\n");
	}
	if (GMT->session.TMPDIR)			/* Isolation mode: Use GMT->session.TMPDIR/gmt.history */
		sprintf (hfile, "%s/gmt.history", GMT->session.TMPDIR);
	else if (!access (cwd, W_OK))	/* Current directory is writable */
		sprintf (hfile, "gmt.history");
	else	/* Try home directory instead */
		sprintf (hfile, "%s/gmt.history", GMT->session.HOMEDIR);

	if ((fp = fopen (hfile, "w")) == NULL) return (-1);	/* Not OK to be unsuccessful in creating this file */

	/* When we get here the file is open */
	if (!gmt_file_lock (GMT, fileno(fp)))
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: %s is not locked for exclusive access. Multiple gmt processes running at once could corrupt history file.\n", hfile);

	fprintf (fp, "# GMT 5 Session common arguments shelf\n");
	fprintf (fp, "BEGIN GMT " GMT_PACKAGE_VERSION "\n");
	for (id = 0; id < GMT_N_UNIQUE; id++) {
		if (!GMT->init.history[id]) continue;	/* Not specified */
		fprintf (fp, "%s\t%s\n", GMT_unique_option[id], GMT->init.history[id]);
	}
	if (GMT->current.ps.clip_level) fprintf (fp, "C\t%d\n", GMT->current.ps.clip_level); /* Write clip level */
	if (GMT->current.ps.layer) fprintf (fp, "L\t%d\n", GMT->current.ps.layer); /* Write PS layer, if non-zero */
	fprintf (fp, "END\n");

	/* Close the file */
	gmt_file_unlock (GMT, fileno(fp));
	fclose (fp);

	return (GMT_NOERROR);
}

/*! . */
void Free_GMT_Ctrl (struct GMT_CTRL *GMT) {	/* Deallocate control structure */
	if (!GMT) return;	/* Never was allocated */
	free (GMT);
}

/*! . */
void GMT_end (struct GMT_CTRL *GMT)
{
	/* GMT_end will clean up after us. */

	unsigned int i;

	gmt_put_history (GMT);

	/* Remove font structures */
	for (i = 0; i < GMT->session.n_fonts; i++) {
		free (GMT->session.font[i].name);
		GMT->session.font[i].name = NULL;
	}
	GMT_free (GMT, GMT->session.font);
#ifdef __FreeBSD__
#ifdef _i386_
	fpresetsticky (FP_X_DZ | FP_X_INV);
	fpsetmask (FP_X_DZ | FP_X_INV);
#endif
#endif

	if (GMT->init.runtime_bindir) {free (GMT->init.runtime_bindir); GMT->init.runtime_bindir = NULL;}
	if (GMT->init.runtime_libdir) {free (GMT->init.runtime_libdir); GMT->init.runtime_libdir = NULL;}
	if (GMT->init.runtime_plugindir) {free (GMT->init.runtime_plugindir); GMT->init.runtime_plugindir = NULL;}
	free (GMT->session.SHAREDIR); GMT->session.SHAREDIR = NULL;
	free (GMT->session.HOMEDIR); GMT->session.HOMEDIR = NULL;
	if (GMT->session.DATADIR) {free (GMT->session.DATADIR); GMT->session.DATADIR = NULL;}
	if (GMT->session.DCWDIR) {free (GMT->session.DCWDIR); GMT->session.DCWDIR = NULL;}
	if (GMT->session.GSHHGDIR) {free (GMT->session.GSHHGDIR); GMT->session.GSHHGDIR = NULL;}
	if (GMT->session.USERDIR) {free (GMT->session.USERDIR);  GMT->session.USERDIR = NULL;}
	if (GMT->session.TMPDIR) {free (GMT->session.TMPDIR); GMT->session.TMPDIR = NULL;}
	if (GMT->session.CUSTOM_LIBS) {free (GMT->session.CUSTOM_LIBS); GMT->session.CUSTOM_LIBS = NULL;}
	for (i = 0; i < GMT_N_PROJ4; i++) {
		free (GMT->current.proj.proj4[i].name);
		GMT->current.proj.proj4[i].name = NULL;
	}
	GMT_free (GMT, GMT->current.proj.proj4);
	for (i = 0; i < GMT_N_UNIQUE; i++) if (GMT->init.history[i]) {
		free (GMT->init.history[i]);
		GMT->init.history[i] = NULL;
	}
	gmt_reset_colformats (GMT);	/* Wipe settings */
	for (i = 0; i < GMT->common.a.n_aspatial; i++) if (GMT->common.a.name[i]) {
		free (GMT->common.a.name[i]);
		GMT->common.a.name[i] = NULL;
	}
	if (GMT->common.h.title)    {free (GMT->common.h.title);    GMT->common.h.title    = NULL;}
	if (GMT->common.h.remark)   {free (GMT->common.h.remark);   GMT->common.h.remark   = NULL;}
	if (GMT->common.h.colnames) {free (GMT->common.h.colnames); GMT->common.h.colnames = NULL;}

	if (GMT->current.setting.io_gridfile_shorthand) gmt_freeshorthand (GMT);

	fflush (GMT->session.std[GMT_OUT]);	/* Make sure output buffer is flushed */

	GMT_free_ogr (GMT, &(GMT->current.io.OGR), 1);	/* Free up the GMT/OGR structure, if used */
	GMT_free_tmp_arrays (GMT);			/* Free emp memory for vector io or processing */
	gmt_free_user_media (GMT);
	/* Terminate PSL machinery (if used) */
	PSL_endsession (GMT->PSL);
#ifdef MEMDEBUG
	GMT_memtrack_report (GMT);
	free (GMT->hidden.mem_keeper);
#endif

	Free_GMT_Ctrl (GMT);	/* Deallocate control structure */
}

/*! . */
struct GMT_CTRL *GMT_begin_module (struct GMTAPI_CTRL *API, const char *lib_name, const char *mod_name, struct GMT_CTRL **Ccopy)
{	/* All GMT modules (i.e. GMT_psxy, GMT_blockmean, ...) must call GMT_begin_module
	 * as their first call and call GMT_end_module as their last call.  This
	 * allows us to capture the GMT control structure so we can reset all
	 * parameters to what they were before exiting the module. Note:
	 * 1. Session items that remain unchanged are not replicated if allocated separately.
	 * 2. Items that may grow through session are not replicated if allocated separately.
	 */

	unsigned int i;
	struct GMT_CTRL *GMT = API->GMT, *Csave = NULL;

	Csave = calloc (1U, sizeof (struct GMT_CTRL));

	GMT_free_tmp_arrays (GMT);			/* Free temp memory for vector io or processing */

	/* First memcpy over everything; this will include pointer addresses we will have to fix below */

	GMT_memcpy (Csave, GMT, 1, struct GMT_CTRL);

	/* Increment level uint64_t */
	GMT->hidden.func_level++;		/* This lets us know how deeply we are nested when a GMT module is called */

	/* Now fix things that were allocated separately from the main GMT structure.  These are usually text strings
	 * that were allocated via strdup since the structure only have a pointer allocated. */

	/* GMT_INIT */
	if (GMT->session.n_user_media) {
		Csave->session.n_user_media = GMT->session.n_user_media;
		Csave->session.user_media = GMT_memory (GMT, NULL, GMT->session.n_user_media, struct GMT_MEDIA);
		Csave->session.user_media_name = GMT_memory (GMT, NULL, GMT->session.n_user_media, char *);
		for (i = 0; i < GMT->session.n_user_media; i++) Csave->session.user_media_name[i] = strdup (GMT->session.user_media_name[i]);
	}

	/* GMT_PLOT */
	if (GMT->current.plot.n_alloc) {
		Csave->current.plot.n_alloc = GMT->current.plot.n_alloc;
		Csave->current.plot.x = GMT_memory (GMT, NULL, GMT->current.plot.n_alloc, double);
		Csave->current.plot.y = GMT_memory (GMT, NULL, GMT->current.plot.n_alloc, double);
		Csave->current.plot.pen = GMT_memory (GMT, NULL, GMT->current.plot.n_alloc, unsigned int);
		GMT_memcpy (Csave->current.plot.x, GMT->current.plot.x, GMT->current.plot.n_alloc, double);
		GMT_memcpy (Csave->current.plot.y, GMT->current.plot.y, GMT->current.plot.n_alloc, double);
		GMT_memcpy (Csave->current.plot.pen, GMT->current.plot.pen, GMT->current.plot.n_alloc, unsigned int);
	}

	/* GMT_IO */
	Csave->current.io.OGR = GMT_duplicate_ogr (GMT, GMT->current.io.OGR);	/* Duplicate OGR struct, if set */
	GMT_free_ogr (GMT, &(GMT->current.io.OGR), 1);		/* Free up the GMT/OGR structure, if used */

	GMT_memset (Csave->current.io.o_format, GMT_MAX_COLUMNS, char *);
	for (i = 0; i < GMT_MAX_COLUMNS; i++)
		if (GMT->current.io.o_format[i]) Csave->current.io.o_format[i] = strdup (GMT->current.io.o_format[i]);

	/* GMT_COMMON */
	if (GMT->common.U.label) Csave->common.U.label = strdup (GMT->common.U.label);
	for (i = 0; i < GMT->common.a.n_aspatial; i++)
		if (GMT->common.a.name[i]) Csave->common.a.name[i] = strdup (GMT->common.a.name[i]);
	if (GMT->common.h.title) Csave->common.h.title = strdup (GMT->common.h.title);
	if (GMT->common.h.remark) Csave->common.h.remark = strdup (GMT->common.h.remark);
	if (GMT->common.h.colnames) Csave->common.h.colnames = strdup (GMT->common.h.colnames);

	/* Reset all the common.?.active settings to false */

	GMT->common.B.active[0] = GMT->common.B.active[1] = GMT->common.K.active = GMT->common.O.active = false;
	GMT->common.P.active = GMT->common.U.active = GMT->common.V.active = false;
	GMT->common.X.active = GMT->common.Y.active = false;
	GMT->common.R.active = GMT->common.J.active = false;
	GMT->common.a.active = GMT->common.b.active[GMT_IN] = GMT->common.b.active[GMT_OUT] = GMT->common.c.active = false;
	GMT->common.f.active[GMT_IN] = GMT->common.f.active[GMT_OUT] = GMT->common.g.active = GMT->common.h.active = false;
	GMT->common.p.active = GMT->common.s.active = GMT->common.t.active = GMT->common.colon.active = false;
	GMT_memset (GMT->common.b.ncol, 2, int);

	*Ccopy = Csave; /* Pass back out for safe-keeping by the module until GMT_end_module is called */

	GMT->init.module_name = mod_name;
	GMT->init.module_lib  = lib_name;

	return (GMT);
}

/*! . */
void gmt_free_plot_array (struct GMT_CTRL *GMT) {
	if (GMT->current.plot.n_alloc) {
		GMT_free (GMT, GMT->current.plot.x);
		GMT_free (GMT, GMT->current.plot.y);
		GMT_free (GMT, GMT->current.plot.pen);
	}
	GMT->current.plot.n = GMT->current.plot.n_alloc = 0;
}

/*! . */
void GMT_end_module (struct GMT_CTRL *GMT, struct GMT_CTRL *Ccopy) {
	unsigned int i;
	unsigned int V_level = GMT->current.setting.verbose;	/* Keep copy of currently selected level */

	if (GMT->current.proj.n_geodesic_approx) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Warning: Of % " PRIu64 " geodesic calls, % " PRIu64 " exceeded the iteration limit of 50.\n", GMT->current.proj.n_geodesic_calls, GMT->current.proj.n_geodesic_approx);
	}

	GMT_Garbage_Collection (GMT->parent, GMT->hidden.func_level);	/* Free up all registered memory for this module level */

	/* At the end of the module we restore all GMT settings as we found them (in Ccopy) */

	/* GMT_INIT */

	/* We treat the history explicitly since we accumulate the history regardless of nested level */

	for (i = 0; i < GMT_N_UNIQUE; i++)
		Ccopy->init.history[i] = GMT->init.history[i];

	/* GMT_CURRENT */

	Ccopy->current.ps.clip_level = GMT->current.ps.clip_level;
	Ccopy->current.ps.layer = GMT->current.ps.layer;

	/* GMT_COMMON */

	if (Ccopy->common.U.label && Ccopy->common.U.label != GMT->common.U.label) free (Ccopy->common.U.label);
	Ccopy->common.U.label = GMT->common.U.label;
	for (i = 0; i < GMT->common.a.n_aspatial; i++) if (GMT->common.a.name[i]) {
		free (GMT->common.a.name[i]);
		GMT->common.a.name[i] = NULL;
	}
	if (GMT->common.h.title)    free (GMT->common.h.title),    GMT->common.h.title    = NULL;
	if (GMT->common.h.remark)   free (GMT->common.h.remark),   GMT->common.h.remark   = NULL;
	if (GMT->common.h.colnames) free (GMT->common.h.colnames), GMT->common.h.colnames = NULL;

	/* GMT_PLOT */

	gmt_free_plot_array (GMT);	/* Free plot arrays and reset n_alloc, n */
	GMT_free_custom_symbols (GMT);	/* Free linked list of custom psxy[z] symbols, if any */
	gmt_free_user_media (GMT);	/* Free user-specified media formats */

	/* GMT_IO */

	GMT_free_ogr (GMT, &(GMT->current.io.OGR), 1);	/* Free up the GMT/OGR structure, if used */
	GMT_free_tmp_arrays (GMT);			/* Free emp memory for vector io or processing */
	gmt_reset_colformats (GMT);			/* Wipe previous settings */

	GMT_fft_cleanup (GMT); /* Clean FFT resources */

	/* Overwrite GMT with what we saved in GMT_begin_module */
	GMT_memcpy (GMT, Ccopy, 1, struct GMT_CTRL);	/* Overwrite struct with things from Ccopy */
	GMT->current.setting.verbose = V_level;	/* Pass the currently selected level back up */

	/* Now fix things that were allocated separately */
	if (Ccopy->session.n_user_media) {
		GMT->session.n_user_media = Ccopy->session.n_user_media;
		GMT->session.user_media = GMT_memory (GMT, NULL, Ccopy->session.n_user_media, struct GMT_MEDIA);
		GMT->session.user_media_name = GMT_memory (GMT, NULL, Ccopy->session.n_user_media, char *);
		for (i = 0; i < Ccopy->session.n_user_media; i++) GMT->session.user_media_name[i] = strdup (Ccopy->session.user_media_name[i]);
	}

	gmt_free_user_media (Ccopy);		/* Free user-specified media formats */

	free (Ccopy);	/* Good riddance */
}

/*! . */
int GMT_set_env (struct GMT_CTRL *GMT) {
	char *this_c = NULL, path[PATH_MAX+1];

#ifdef SUPPORT_EXEC_IN_BINARY_DIR
	/* If SUPPORT_EXEC_IN_BINARY_DIR is defined we try to set the share dir to
	 * ${GMT_SOURCE_DIR}/share and the user dir to ${GMT_BINARY_DIR}/share in
	 * order to simplify debugging and running in GMT_BINARY_DIR, e.g., when
	 * debugging with Xcode or Visual Studio. This saves us from setting the
	 * env variables GMT_SHAREDIR and GMT_USERDIR and we do not have to install
	 * src/share in its destination dir. */

	/* Only true, when we are running in a subdir of GMT_BINARY_DIR_SRC_DEBUG: */
	bool running_in_bindir_src = !strncmp (GMT->init.runtime_bindir, GMT_BINARY_DIR_SRC_DEBUG, strlen(GMT_BINARY_DIR_SRC_DEBUG));
#endif

	/* Determine GMT->session.SHAREDIR (directory containing coast, cpt, etc. subdirectories) */

	/* Note: GMT_set_env cannot use GMT_Report because the verbose level is not yet set */

	if ((this_c = getenv ("GMT5_SHAREDIR")) != NULL
			&& GMT_verify_sharedir_version (this_c) )
		/* GMT5_SHAREDIR was set */
		GMT->session.SHAREDIR = strdup (this_c);
	else if ((this_c = getenv ("GMT_SHAREDIR")) != NULL
			&& GMT_verify_sharedir_version (this_c) )
		/* GMT_SHAREDIR was set */
		GMT->session.SHAREDIR = strdup (this_c);
#ifdef SUPPORT_EXEC_IN_BINARY_DIR
	else if ( running_in_bindir_src && GMT_verify_sharedir_version (GMT_SHARE_DIR_DEBUG) )
		/* Use ${GMT_SOURCE_DIR}/share to simplify debugging and running in GMT_BINARY_DIR */
		GMT->session.SHAREDIR = strdup (GMT_SHARE_DIR_DEBUG);
#endif
	else if ( GMT_verify_sharedir_version (GMT_SHARE_DIR) )
		/* Found in hardcoded GMT_SHARE_DIR */
		GMT->session.SHAREDIR = strdup (GMT_SHARE_DIR);
	else {
		/* SHAREDIR still not found, make a smart guess based on runpath: */
		if ( GMT_guess_sharedir (path, GMT->init.runtime_bindir) )
			GMT->session.SHAREDIR = strdup (path);
		else {
			/* Still not found */
			fprintf (stderr, "Error: Could not locate share directory that matches the current GMT version %s.\n", GMT_PACKAGE_VERSION_WITH_SVN_REVISION);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
	}
	DOS_path_fix (GMT->session.SHAREDIR);

	/* Determine HOMEDIR (user home directory) */

	if ((this_c = getenv ("HOME")) != NULL)
		/* HOME was set */
		GMT->session.HOMEDIR = strdup (this_c);
#ifdef WIN32
	else if ((this_c = getenv ("HOMEPATH")) != NULL)
		/* HOMEPATH was set */
		GMT->session.HOMEDIR = strdup (this_c);
#endif
	else {
		/* If HOME not set: use root directory instead (http://gmt.soest.hawaii.edu/issues/710) */
		GMT->session.HOMEDIR = strdup ("/"); /* Note: Windows will use the current drive if drive letter unspecified. */
#ifdef DEBUG
		fprintf (stderr, "Warning: HOME environment not set. Using root directory instead.\n");
#endif
	}
	DOS_path_fix (GMT->session.HOMEDIR);

	/* Determine GMT_USERDIR (directory containing user replacements contents in GMT_SHAREDIR) */

	if ((this_c = getenv ("GMT_USERDIR")) != NULL)
		/* GMT_USERDIR was set */
		GMT->session.USERDIR = strdup (this_c);
#ifdef SUPPORT_EXEC_IN_BINARY_DIR
	else if ( running_in_bindir_src && access (GMT_USER_DIR_DEBUG, R_OK|X_OK) == 0 )
		/* Use ${GMT_BINARY_DIR}/share to simplify debugging and running in GMT_BINARY_DIR */
		GMT->session.USERDIR = strdup (GMT_USER_DIR_DEBUG);
#endif
	else {
		/* Use default path for GMT_USERDIR (~/.gmt) */
		sprintf (path, "%s/%s", GMT->session.HOMEDIR, ".gmt");
		GMT->session.USERDIR = strdup (path);
	}
	DOS_path_fix (GMT->session.USERDIR);
	if (GMT->session.USERDIR != NULL && access (GMT->session.USERDIR, R_OK)) {
		/* If we cannot access this dir then we won't use it */
		free (GMT->session.USERDIR);
		GMT->session.USERDIR = NULL;
	}

	if (GMT_compat_check (GMT, 4)) {
		/* Check if obsolete GMT_CPTDIR was specified */

		if ((this_c = getenv ("GMT_CPTDIR")) != NULL) {
			/* GMT_CPTDIR was set */
			fprintf (stderr, "Warning: Environment variable GMT_CPTDIR was set but is no longer used by GMT.\n");
			fprintf (stderr, "Warning: System-wide color tables are in %s/cpt.\n", GMT->session.SHAREDIR);
			fprintf (stderr, "Warning: Use GMT_USERDIR (%s) instead and place user-defined color tables there.\n", GMT->session.USERDIR);
		}
	}

	/* Determine GMT_DATADIR (data directories) */

	if ((this_c = getenv ("GMT_DATADIR")) != NULL) {
		/* GMT_DATADIR was set */
		if (strchr (this_c, PATH_SEPARATOR) || access (this_c, R_OK) == 0) {
			/* A list of directories or a single directory that is accessible */
			GMT->session.DATADIR = strdup (this_c);
			DOS_path_fix (GMT->session.DATADIR);
		}
	}

	/* Determine GMT_TMPDIR (for isolation mode). Needs to exist use it. */

	if ((this_c = getenv ("GMT_TMPDIR")) != NULL) {
		/* GMT_TMPDIR was set */
		if (access (this_c, R_OK|W_OK|X_OK)) {
			fprintf (stderr, "Warning: Environment variable GMT_TMPDIR was set to %s, but directory is not accessible.\n", this_c);
			fprintf (stderr, "Warning: GMT_TMPDIR needs to have mode rwx. Isolation mode switched off.\n");
			GMT->session.TMPDIR = NULL;
		}
		else
			GMT->session.TMPDIR = strdup (this_c);
		DOS_path_fix (GMT->session.TMPDIR);
	}
	return GMT_OK;
}

/*! . */
int GMT_Complete_Options (struct GMT_CTRL *GMT, struct GMT_OPTION *options)
{
	/* Go through the given arguments and look for shorthands,
	 * i.e., -B, -J, -R, -X, -x, -Y, -c, -p. given without arguments.
	 * If found, see if we have a matching command line history and then
	 * update that entry in the option list.
	 * Finally, keep the option arguments in the history list.
	 * However, when func_level > 1, do update the entry, but do not
	 * remember it in history. Note, there are two special cases here:
	 * -J is special since we also need to deal with the sub-species
	 *    like -JM, -JX, etc.  So these all have separate entries.
	 * -B is special because the option is repeatable for different
	 *    aspects of the basemap.  We concatenate all of them to store
	 *    in the history file and use RS = ASCII 30 as separator.
	 *    Also, a single -B in the options may expand to several
	 *    separate -B<args> so the linked options list may grow.
	 */
	int id = 0, k, n_B = 0, B_id;
	unsigned int pos = 0, B_replace = 1;
	bool update, remember;
	struct GMT_OPTION *opt = NULL, *opt2 = NULL, *B_next = NULL;
	char str[3] = {""}, B_string[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, B_delim[2] = {30, 0};	/* Use ASCII 30 RS Record Separator between -B strings */

	remember = (GMT->hidden.func_level == 1);   /* Only update the history for top level function */

	for (opt = options; opt; opt = opt->next) {
		if (opt->option == 'B') {	/* Do some initial counting of how many -B options and determine if there is just one with no args */
			if (n_B > 0 || opt->arg[0]) B_replace = 0;
			n_B++;
		}
	}
	for (k = 0, B_id = -1; k < GMT_N_UNIQUE && B_id == -1; k++)
		if (!strcmp (GMT_unique_option[k], "B")) B_id = k;	/* B_id === 0 but just in case this changes we do this search anyway */

	for (opt = options; opt; opt = opt->next) {
		if (!strchr (GMT_SHORTHAND_OPTIONS, opt->option)) continue;	/* Not one of the shorthand options */
		update = false;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "History: Process -%c%s.\n", opt->option, opt->arg);

		str[0] = opt->option; str[1] = str[2] = '\0';
		if (opt->option == 'J') {               /* -J is special since it can be -J or -J<code> */
			/* Always look up "J" first. It comes before "J?" and tells what the last -J was */
			for (k = 0, id = -1; k < GMT_N_UNIQUE && id == -1; k++)
				if (!strcmp (GMT_unique_option[k], str)) id = k;
			if (id < 0) Return;
			if (opt->arg && opt->arg[0]) {      /* Gave -J<code>[<args>] so we either use or update history and continue */
				str[1] = opt->arg[0];
				/* Remember this last -J<code> for later use as -J, but do not remember it when -Jz|Z */
				if (str[1] != 'Z' && str[1] != 'z' && remember) {
					if (GMT->init.history[id]) free (GMT->init.history[id]);
					GMT->init.history[id] = strdup (&str[1]);
				}
				if (opt->arg[1]) update = true; /* Gave -J<code><args> so we want to update history and continue */
			}
			else {
				if (!GMT->init.history[id]) Return;
				str[1] = GMT->init.history[id][0];
			}
			/* Continue looking for -J<code> */
			for (k = id + 1, id = -1; k < GMT_N_UNIQUE && id == -1; k++)
				if (!strcmp (GMT_unique_option[k], str)) id = k;
			if (id < 0) Return;
		}
		else if (opt->option == 'B') {          /* -B is also special since there may be many of these, or just -B */
			if (B_replace) {                    /* Only -B is given and we want to use the history */
				if (B_replace == 2) continue;   /* Already done this */
				if (!GMT->init.history[B_id]) Return;
				opt2 = opt;                     /* Since we dont want to change the opt loop avove */
				B_next = opt->next;             /* Pointer to option following the -B option */
				if (opt2->arg) free (opt2->arg);/* Free previous pointer to arg */
				GMT_strtok (GMT->init.history[B_id], B_delim, &pos, p);	/* Get the first argument */
				opt2->arg = strdup (p);         /* Update arg */
				while (GMT_strtok (GMT->init.history[B_id], B_delim, &pos, p)) {	/* Parse any additional |<component> statements */
					opt2->next = GMT_Make_Option (GMT->parent, 'B', p);	/* Create new struct */
					opt2->next->previous = opt2;
					opt2 = opt2->next;
				}
				opt2->next = B_next;            /* Hook back onto main option list */
				B_replace = 2;                  /* Flag to let us know we are done with -B */
			}
			else {	/* One of possibly several -B<arg> options; concatenate and separate by RS */
				if (B_string[0]) strcat (B_string, B_delim);	/* Add RS separator between args */
				strcat (B_string, opt->arg);
			}
		}
		else {	/* Gave -R[<args>], -V[<args>] etc., so we either use or update the history and continue */
			for (k = 0, id = -1; k < GMT_N_UNIQUE && id == -1; k++)
				if (!strcmp (GMT_unique_option[k], str)) id = k;	/* Find entry in history array */
			if (id < 0) Return;                 /* Error: user gave shorthand option but there is no record in the history */
			if (opt->arg && opt->arg[0]) update = true;	/* Gave -R<args>, -V<args> etc. so we we want to update history and continue */
		}
		if (opt->option != 'B') {               /* Do -B separately again after the loop so skip it here */
			if (update) {                       /* Gave -J<code><args>, -R<args>, -V<args> etc. so we update history and continue */
				if (remember) {
					if (GMT->init.history[id]) free (GMT->init.history[id]);
					GMT->init.history[id] = strdup (opt->arg);
				}
			}
			else {	/* Gave -J<code>, -R, -J etc. so we complete the option and continue */
				if (!GMT->init.history[id]) Return;
				if (opt->arg) free (opt->arg);   /* Free previous pointer to arg */
				opt->arg = strdup (GMT->init.history[id]);
			}
		}
	}

	if (B_string[0]) {	/* Got a concatenated string with one or more individual -B args, now separated by the RS character (ascii 30) */
		if (GMT->init.history[B_id]) free (GMT->init.history[B_id]);
		GMT->init.history[B_id] = strdup (B_string);
	}

	return (GMT_NOERROR);
}

/* Here is the new -B parser with all its sub-functions */

#ifdef WIN32
/*! . */
void gmt_handle_dosfile (struct GMT_CTRL *GMT, char *in, int this_mark)
{
	/* Because (1) we use colons to indicate start/stop of text labels and
	 * (2) under Windows, a colon can be part of a path (e.g., C:\dir\file)
	 * we need to temporarily replace <drive>:\ with <drive>;\ so that this
	 * path colon does not interfere with the rest of the parsing.  Once the
	 * colon items have been parsed, we replace the ; back to : */
	int i, len, other = 1 - this_mark;
	char mark[2] = {':', ';'};

	if (!in)
		return;	/* Nothing to work on */
	if ((len = (int)strlen (in)) < 2)
		return;	/* Nothing to work on */
	len -= 2; /* Since this use of : cannot be at the end anyway and we need to check the next two characters */
	for (i = 1; i < len; ++i) {
		/* Start at position 1 since we need the position before.
		 * Look for "X:/<nocolon>" pattern, with X = A-Z */
		if (in[i] == mark[this_mark] && (in[i-1] >= 'A' && in[i-1] <= 'Z')
				&& (in[i+1] == '/' || in[i+1] == '\\') && (in[i+2] != mark[this_mark]))
			in[i] = mark[other];
	}
}
#endif

/*! . */
int gmt_strip_colonitem (struct GMT_CTRL *GMT, int axis, const char *in, const char *pattern, char *item, char *out) {
	/* Removes the searched-for item from in, returns it in item, with the rest in out.
	 * pattern is usually ":." for title, ":," for unit, and ":" for label.
	 * ASSUMPTION: Only pass ":" after first removing titles and units
	 */

	char *s = NULL, *str = "xyz";
	bool error = false;

	if ((s = strstr (in, pattern))) {		/* OK, found what we are looking for */
		size_t i, j, k;
		k = (size_t)(s - in);			/* Start index of item */
		strncpy (out, in, k);			/* Copy everything up to the pattern */
		i = k + strlen (pattern);		/* Now go to beginning of item */
		j = 0;
		while (in[i] && in[i] != ':') item[j++] = in[i++];	/* Copy the item... */
		item[j] = '\0';				/* ...and terminate the string */
		if (in[i] != ':') error = true;		/* Error: Missing terminating colon */
		i++;					/* Skip the ending colon */
		while (in[i]) out[k++] = in[i++];	/* Copy rest to out... */
		out[k] = '\0';				/* .. and terminate */
	}
	else	/* No item to update */
		strcpy (out, in);

	if (error) {	/* Problems with decoding */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: Missing terminating colon in -B string %c-component %s\n", str[axis], in);
		return (1);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":.")) {	/* Problems with decoding title */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: More than one title in -B string %c-component %s\n", str[axis], in);
		return (1);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":,")) {	/* Problems with decoding unit */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: More than one unit string in -B %c-component %s\n", str[axis], in);
		return (1);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":=")) {	/* Problems with decoding prefix */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: More than one prefix string in  -B component %s\n", in);
		return (1);
	}
	if (strstr (out, pattern)) {	/* Problems with decoding label */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: More than one label string in  -B component %s\n", in);
		return (1);
	}
#ifdef _WIN32
	gmt_handle_dosfile (GMT, item, 1);	/* Undo any DOS files like X;/ back to X:/ */
#endif
	return (GMT_NOERROR);
}

/*! . */
void gmt_handle_atcolon (struct GMT_CTRL *GMT, char *txt, int old_p)
{	/* Way = 0: Replaces @:<size>: and @:: with @^<size>^ and @^^ to avoid trouble in -B:label: parsing;
	 * Way = 1: Restores it the way it was. */
	int new_p;
	char *item[2] = {"@:", "@^"}, mark[2] = {':', '^'}, *s = NULL;
	GMT_UNUSED(GMT);

	if (!txt || !txt[0]) return;	/* Nothing to do */
	new_p = 1 - old_p;	/* The opposite of old */
	while ((s = strstr (txt, item[old_p]))) {	/* As long as we keep finding these */
		ptrdiff_t pos = ((size_t)s - (size_t)txt) + 1; /* Skip past the @ character */
		if (txt[pos+1] == mark[old_p]) {			/* Either :: or ^^ */
			txt[pos] = txt[pos+1] = mark[new_p];	/* Replace @:: with @^^ or vice versa */
		}
		else {	/* Found @:<size>: or @^<size>^ */
			txt[pos] = mark[new_p];
			while (txt[pos] && txt[pos] != mark[old_p]) pos++;
			if (txt[pos] == mark[old_p]) txt[pos] = mark[new_p];
		}
	}
}

/*! Take the -B string (minus the leading -B) and chop into 3 strings for x, y, and z */
int gmt_split_info_strings (struct GMT_CTRL *GMT, const char *in, char *x_info, char *y_info, char *z_info) {

	bool mute = false;
	size_t i, n_slash, s_pos[2];

	x_info[0] = y_info[0] = z_info[0] = '\0';

	for (i = n_slash = 0; in[i] && n_slash < 3; i++) {
		if (in[i] == ':') mute = !mute;
		if (in[i] == '/' && !mute) s_pos[n_slash++] = i;	/* Axis-separating slash, not a slash in a label */
	}
	if (n_slash) GMT->current.map.frame.slash = true;

	if (n_slash == 3) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error splitting -B string %s\n", in);
		return (1);
	}

	if (n_slash == 2) {	/* Got x/y/z */
		i = strlen (in);
		strncpy (x_info, in, s_pos[0]);					x_info[s_pos[0]] = '\0';
		strncpy (y_info, &in[s_pos[0]+1], s_pos[1] - s_pos[0] - 1);	y_info[s_pos[1] - s_pos[0] - 1] = '\0';
		strncpy (z_info, &in[s_pos[1]+1], i - s_pos[1] - 1);		z_info[i - s_pos[1] - 1] = '\0';
	}
	else if (n_slash == 1) {	/* Got x/y */
		i = strlen (in);
		strncpy (x_info, in, s_pos[0]);					x_info[s_pos[0]] = '\0';
		strncpy (y_info, &in[s_pos[0]+1], i - s_pos[0] - 1);		y_info[i - s_pos[0] - 1] = '\0';
	}
	else {	/* Got x with implicit copy to y */
		strcpy (x_info, in);
		strcpy (y_info, in);
	}
	return (GMT_NOERROR);
}

/*! . */
int gmt_init_custom_annot (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS *A, int *n_int)
{
	/* Reads a file with one or more records of the form
	 * value	types	[label]
	 * where value is the coordinate of the tickmark, types is a combination
	 * of a|i (annot or interval annot), f (tick), or g (gridline).
	 * The a|i will take a label string (or sentence).
	 * The item argument specifies which type to consider [a|i,f,g].  We return
	 * an array with coordinates and labels, and set interval to true if applicable.
	 */
	int k, n_errors = 0;
	char line[GMT_BUFSIZ] = {""}, type[8] = {""};
	FILE *fp = GMT_fopen (GMT, A->file_custom, "r");

	GMT_memset (n_int, 4, int);
	while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		sscanf (line, "%*s %s", type);
		k = 0;
		for (k = 0; type[k]; k++) {
			switch (type[k]) {
				case 'a':	/* Regular annotation */
					n_int[0]++;
					break;
				case 'i':	/* Interval annotation */
					n_int[1]++;
					break;
				case 'f':	/* Tick placement */
					n_int[2]++;
					break;
				case 'g':	/* Gridline placement */
					n_int[3]++;
					break;
				default:
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unrecognixed type (%c) in custom file %s.\n", type[k], A->file_custom);
					n_errors++;
					break;
			}
		}
	}
	GMT_fclose (GMT, fp);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Processed custom annotations via %s for axis %d.\n", A->file_custom, A->id);
	if (n_int[0] && n_int[1]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot mix interval and regular annotations in custom file %s.\n", A->file_custom);
		n_errors++;
	}
	return (n_errors);
}

/*! Load the values into the appropriate GMT_PLOT_AXIS_ITEM structure */
int gmt_set_titem (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS *A, char *in, char flag, char axis, int custom) {

	struct GMT_PLOT_AXIS_ITEM *I = NULL;
	char *format = NULL, *t = NULL, *s = NULL, unit = 0;
	double phase = 0.0, val = 0.0;

	t = in;

	/* Here, t must point to a valid number.  If t[0] is not [+,-,.] followed by a digit we have an error */

	/* Decode interval, get pointer to next segment */
	if ((val = strtod (t, &s)) < 0.0 && GMT->current.proj.xyz_projection[A->id] != GMT_LOG10) {	/* Interval must be >= 0 */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: Negative interval in -B option (%c-component, %c-info): %s\n", axis, flag, in);
		return (3);
	}
	if (s[0] && (s[0] == '-' || s[0] == '+')) {	/* Phase shift information given */
		t = s;
		phase = strtod (t, &s);
	}
	if (val == 0.0 && t[0] && t == s) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: Bad interval in -B option (%c-component, %c-info): %s gave interval = 0\n", axis, flag, in);
		return (3);
	}

	/* Appended one of the allowed units, or l or p for log10/pow */
	if (s[0] && strchr ("YyOoUuKkJjDdHhMmSsCcrRlp", s[0]))
		unit = s[0];
	else if (A->type == GMT_TIME)				/* Default time system unit implied */
		unit = GMT->current.setting.time_system.unit;
	else
		unit = 0;	/* Not specified */

	if (!GMT->current.map.frame.primary) flag = (char) toupper ((int)flag);

	if (A->type == GMT_TIME) {	/* Strict check on time intervals */
		if (GMT_verify_time_step (GMT, irint (val), unit)) {
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		if ((fmod (val, 1.0) > GMT_CONV8_LIMIT)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: Time step interval (%g) must be an integer\n", val);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
	}

	switch (unit) {	/* Determine if we have intervals or moments */
		case 'Y':	case 'y':
		case 'O':	case 'o':
		case 'K':	case 'k':
		case 'J':	case 'j':
		case 'D':	case 'd':
		case 'R':	case 'r':
		case 'U':	case 'u':
			if (A->type == GMT_TIME && flag == 'a') flag = 'i';
			if (A->type == GMT_TIME && flag == 'A') flag = 'I';
			break;
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

	switch (flag) {
		case 'a': case 'i':	/* Upper annotation / major tick annotation */
			I = &A->item[GMT_ANNOT_UPPER];
			break;
		case 'A': case 'I':	/* Lower annotation / major tick annotation */
			I = &A->item[GMT_ANNOT_LOWER];
			break;
		case 'f':	/* Upper minor tick interval */
			I = &A->item[GMT_TICK_UPPER];
			break;
		case 'F':	/* Lower minor tick interval */
			I = &A->item[GMT_TICK_LOWER];
			break;
		case 'g':	/* Upper gridline interval */
			I = &A->item[GMT_GRID_UPPER];
			break;
		case 'G':	/* Lower gridline interval */
			I = &A->item[GMT_GRID_LOWER];
			break;
		default:	/* Bad flag should never get here */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad flag (%c) passed to gmt_set_titem\n", flag);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			break;
	}

	if (phase != 0.0) A->phase = phase;	/* phase must apply to entire axis */
	if (I->active) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Axis sub-item %c set more than once (typo?)\n", flag);
		return (GMT_NOERROR);
	}
	if (unit == 'c' || unit == 'C') {
		if (GMT_compat_check (GMT, 4)) {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Unit c (arcseconds) is deprecated; use s instead.\n");
			unit = 's';
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error: Unit %c not recognized.\n", unit);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
	}
	I->type = flag;
	I->unit = unit;
	I->interval = val;
	I->flavor = 0;
	I->active = true;
	if (!custom && in[0] && val == 0.0) I->active = false;
	I->upper_case = false;
	format = (GMT->current.map.frame.primary) ? GMT->current.setting.format_time[0] : GMT->current.setting.format_time[1];
	switch (format[0]) {	/* This parameter controls which version of month/day textstrings we use for plotting */
		case 'F':	/* Full name, upper case */
			I->upper_case = true;
		case 'f':	/* Full name, lower case */
			I->flavor = 0;
			break;
		case 'A':	/* Abbreviated name, upper case */
			I->upper_case = true;
		case 'a':	/* Abbreviated name, lower case */
			I->flavor = 1;
			break;
		case 'C':	/* 1-char name, upper case */
			I->upper_case = true;
		case 'c':	/* 1-char name, lower case */
			I->flavor = 2;
			break;
		default:
			break;
	}

	GMT->current.map.frame.draw = true;

	return (GMT_NOERROR);
}

/*! Decode the annot/tick segments of the clean -B string pieces */
int gmt_decode_tinfo (struct GMT_CTRL *GMT, int axis, char flag, char *in, struct GMT_PLOT_AXIS *A) {

	char *str = "xyz";

	if (!in) return (GMT_NOERROR);	/* NULL pointer passed */

	if (flag == 'c') {	/* Custom annotation arrangement */
		int k, n_int[4];
		char *list = "aifg";
		if (!(GMT_access (GMT, &in[1], R_OK))) {
			if (A->file_custom) free (A->file_custom);
			A->file_custom = strdup (&in[1]);
			A->special = GMT_CUSTOM;
			if (gmt_init_custom_annot (GMT, A, n_int)) return (-1);	/* See what ticks, anots, gridlines etc are requested */
			for (k = 0; k < 4; k++) {
				if (n_int[k] == 0) continue;
				flag = list[k];
				if (!GMT->current.map.frame.primary) flag = (char)toupper ((int)flag);
				gmt_set_titem (GMT, A, "0", flag, str[axis], true);	/* Store the findings for this segment */
			}
			if (n_int[1]) A->item[GMT_ANNOT_UPPER+!GMT->current.map.frame.primary].special = true;
			GMT->current.map.frame.draw = true;
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: Cannot access custom file in -B string %c-component %s\n", str[axis], in);
	}
	else
		gmt_set_titem (GMT, A, in, flag, str[axis], false);

	return (GMT_NOERROR);
}

/*! . */
int gmt4_parse_B_option (struct GMT_CTRL *GMT, char *in) {
	/* gmt4_parse_B_option scans an argument string and extract parameters that
	 * set the interval for tickmarks and annotations on the boundary.
	 * The string must be continuous, i.e. no whitespace must be present
	 * The string may have 1, 2,  or 3 parts, separated by a slash '/'. All
	 * info after the first slash are assigned to the y-axis.  Info after
	 * the second slash are assigned to the z-axis.  If there is no
	 * slash, x-values are copied to y-values.
	 * A substring looks like [t][value][m|s]. The [t] and [m|s] are optional
	 * ([ and ] are NOT part of the string and are just used to clarify)
	 * [t] can be any of [a](annotation int), [f](frame int), or [g](gridline int).
	 * Default is a AND f. The [m], if present indicates value is in minutes.
	 * The [s], if present indicates value is in seconds (s also is used for south...).
	 * Text between : and : are labels for the respective axes. If the first
	 * character of the text is a period, then the rest of the text is used
	 * as the plot title.  If it is a comma, then the rest is used as annotation unit.
	 * For GMT_LINEAR axes: If the first characters in args are one or more of w,e,s,n
	 * only those axes will be drawn. Upper case letters means the chosen axes
	 * also will be annotated. Default is all 4 axes drawn/annotated.
	 * For logscale plots: l will cause log10(x) to be plotted
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

	char out1[GMT_BUFSIZ] = "", out2[GMT_BUFSIZ] = "", out3[GMT_BUFSIZ] = "", info[3][GMT_BUFSIZ] = {""};
	struct GMT_PLOT_AXIS *A = NULL;
	int i, j, k, ignore, g = 0, o = 0, part = 0, error = 0;

	if (!in || !in[0]) return (GMT_PARSE_ERROR);	/* -B requires an argument */

	switch (in[0]) {
		case 's':
			GMT->current.map.frame.primary = false; k = part = 1; break;
		case 'p':
			GMT->current.map.frame.primary = true; k = 1; break;
		default:
			GMT->current.map.frame.primary = true; k = 0; break;
	}
	i = (GMT->current.map.frame.primary) ? 0 : 1;
	strncpy (GMT->common.B.string[i], in, GMT_LEN256);	/* Keep a copy of the actual option(s) */

	/* GMT->current.map.frame.side[] may be set already when parsing gmt.conf flags */

	if (!GMT->current.map.frame.init) {	/* First time we initialize stuff */
		for (i = 0; i < 3; i++) {
			GMT_memset (&GMT->current.map.frame.axis[i], 1, struct GMT_PLOT_AXIS);
			GMT->current.map.frame.axis[i].id = i;
			for (j = 0; j < 6; j++) GMT->current.map.frame.axis[i].item[j].parent = i;
			if (GMT->current.proj.xyz_projection[i] == GMT_TIME) GMT->current.map.frame.axis[i].type = GMT_TIME;
		}
		GMT->current.map.frame.header[0] = '\0';
		GMT->current.map.frame.init = true;
		GMT->current.map.frame.draw = false;
		GMT->current.map.frame.set_frame[0] = GMT->current.map.frame.set_frame[1] = 0;
	}

#ifdef _WIN32
	gmt_handle_dosfile (GMT, in, 0);	/* Temporarily replace DOS files like X:/ with X;/ to avoid colon trouble */
#endif

	for (i = (int)strlen(in) - 1, ignore = false; !GMT->current.map.frame.paint && !error && i >= 0; i--) {	/** Look for +g<fill */
		if (in[i] == ':') ignore = !ignore;
		if (ignore) continue;	/* Not look inside text items */
		if (in[i] == '+' && in[i+1] == 'o') {	/* Found +o<plon>/<plat> */
			double lon, lat;
			char A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""};
			if (GMT->current.proj.projection == GMT_OBLIQUE_MERC) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Cannot specify oblique gridlines for the oblique Mercator projection\n");
				error++;
			}
			GMT->current.map.frame.obl_grid = true;
			if (sscanf (&in[i+2], "%[^/]/%[^+]", A, B) != 2) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Did not find the expected format +o<plon>/<plat>\n");
				error++;
			}
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, A, GMT_IS_LON, &lon), A);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, B, GMT_IS_LAT, &lat), B);
			if (GMT->current.proj.projection != GMT_OBLIQUE_MERC) gmt_set_oblique_pole_and_origin (GMT, lon, lat, 0.0, 0.0);
			o = i;
			in[o] = '\0';	/* Chop off +o for now */
		}
		if (in[i] == '+' && in[i+1] == 'g') {	/* Found +g<fill> */
			strcpy (out1, &in[i+2]);	/* Make a copy of the fill argument */
#ifdef _WIN32
			gmt_handle_dosfile (GMT, out1, 1);	/* Undo any DOS files like X;/ back to X:/ */
#endif
			if (GMT_getfill (GMT, out1, &GMT->current.map.frame.fill)) error++;
			if (!error) {
				GMT->current.map.frame.paint = true;
				g = i;
				in[g] = '\0';	/* Chop off +g for now */
			}
		}
	}
	/* Note that gmt_strip_colonitem calls gmt_handle_dosfile so that the item return has been processed for DOS path restoration */
	error += gmt_strip_colonitem (GMT, 0, &in[k], ":.", GMT->current.map.frame.header, out1);	/* Extract header string, if any */
	GMT_enforce_rgb_triplets (GMT, GMT->current.map.frame.header, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */

	i = gmt4_decode_wesnz (GMT, out1, GMT->current.map.frame.side, &GMT->current.map.frame.draw_box, part);		/* Decode WESNZwesnz+ flags, if any */
	out1[i] = '\0';	/* Strip the WESNZwesnz+ flags off */

	gmt_split_info_strings (GMT, out1, info[0], info[1], info[2]);	/* Chop/copy the three axis strings */

	for (i = 0; i < 3; i++) {	/* Process each axis separately */

		if (!info[i][0]) continue;	 /* Skip empty format string */
		if (info[i][0] == '0' && !info[i][1]) {	 /* Skip format '0' */
			GMT->current.map.frame.draw = true;
			continue;
		}

		gmt_handle_atcolon (GMT, info[i], 0);	/* Temporarily modify text escape @: to @^ to avoid : parsing trouble */
		GMT_enforce_rgb_triplets (GMT, info[i], GMT_BUFSIZ);				/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
		error += gmt_strip_colonitem (GMT, i, info[i], ":,", GMT->current.map.frame.axis[i].unit, out1);	/* Pull out annotation unit, if any */
		error += gmt_strip_colonitem (GMT, i, out1, ":=", GMT->current.map.frame.axis[i].prefix, out2);	/* Pull out annotation prefix, if any */
		error += gmt_strip_colonitem (GMT, i, out2, ":", GMT->current.map.frame.axis[i].label, out3);	/* Pull out axis label, if any */
		gmt_handle_atcolon (GMT, GMT->current.map.frame.axis[i].label, 1);	/* Restore any @^ to @: */
		gmt_handle_atcolon (GMT, GMT->current.map.frame.axis[i].prefix, 1);	/* Restore any @^ to @: */
		gmt_handle_atcolon (GMT, GMT->current.map.frame.axis[i].unit, 1);	/* Restore any @^ to @: */

		if (GMT->current.map.frame.axis[i].prefix[0]) {	/* Deal with space/no space before prefix */
			char workspace[GMT_LEN64] = {""};
			if (GMT->current.map.frame.axis[i].prefix[0] == '-') /* Dont want a space */
				strcpy (workspace, &GMT->current.map.frame.axis[i].prefix[1]);
			else {	/* Want a space */
				workspace[0] = ' ';	/* The leading space */
				strcpy (&workspace[1], GMT->current.map.frame.axis[i].prefix);
			}
			GMT_memcpy (GMT->current.map.frame.axis[i].prefix, workspace, GMT_LEN64, char);
		}
		if (GMT->current.map.frame.axis[i].unit[0]) {	/* Deal with space/no space before unit */
			char workspace[GMT_LEN64] = {""};
			if (GMT->current.map.frame.axis[i].unit[0] == '-') /* Dont want a space */
				strcpy (workspace, &GMT->current.map.frame.axis[i].unit[1]);
			else {	/* Want a space */
				workspace[0] = ' ';	/* The leading space */
				strcpy (&workspace[1], GMT->current.map.frame.axis[i].unit);
			}
			GMT_memcpy (GMT->current.map.frame.axis[i].unit, workspace, GMT_LEN64, char);
		}

		if (out3[0] == '\0') continue;	/* No intervals */
		GMT->current.map.frame.set = true;	/* Got here so we are setting intervals */

		/* Parse the annotation/tick info string */
		if (out3[0] == 'c')
			error += gmt_decode_tinfo (GMT, i, 'c', out3, &GMT->current.map.frame.axis[i]);
		else {	/* Parse from back for 'a', 'f', 'g' chunks */
			for (k = (int)strlen (out3) - 1; k >= 0; k--) {
				if (out3[k] == 'a' || out3[k] == 'f' || out3[k] == 'g') {
					error += gmt_decode_tinfo (GMT, i, out3[k], &out3[k+1], &GMT->current.map.frame.axis[i]);
					out3[k] = '\0';	/* Replace with terminator */
				}
				else if (k == 0)	/* If no [a|f|g] then 'a' */
					error += gmt_decode_tinfo (GMT, i, 'a', out3, &GMT->current.map.frame.axis[i]);
			}
		}

		/* Make sure we have ticks to match annotation stride */
		A = &GMT->current.map.frame.axis[i];
		if (A->item[GMT_ANNOT_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			GMT_memcpy (&A->item[GMT_TICK_UPPER], &A->item[GMT_ANNOT_UPPER], 1, struct GMT_PLOT_AXIS_ITEM);
		if (A->item[GMT_ANNOT_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			GMT_memcpy (&A->item[GMT_TICK_LOWER], &A->item[GMT_ANNOT_LOWER], 1, struct GMT_PLOT_AXIS_ITEM);
		/* Note that item[].type will say 'a', 'A', 'i' or 'I' in these cases, so we know when minor ticks were not set */
	}

	/* Check if we asked for linear projections of geographic coordinates and did not specify a unit - if so set degree symbol as unit */
	if (GMT->current.proj.projection == GMT_LINEAR && GMT->current.setting.map_degree_symbol != gmt_none) {
		for (i = 0; i < 2; i++) {
			if (GMT->current.io.col_type[GMT_IN][i] & GMT_IS_GEO && GMT->current.map.frame.axis[i].unit[0] == 0) {
				GMT->current.map.frame.axis[i].unit[0] = '-';
				GMT->current.map.frame.axis[i].unit[1] = (char)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol];
				GMT->current.map.frame.axis[i].unit[2] = '\0';
			}
		}
	}
	if (g) in[g] = '+';	/* Restore + */

	return (error);
}

/* New GMT5 functions for parsing new -B syntax */

/*! . */
void gmt5_handle_plussign (struct GMT_CTRL *GMT, char *in, char *mods, unsigned way) {
	/* Way = 0: replace any +<letter> with <letter> NOT in <mods> with ASCII 1<letter>
	 * Way = 1: Replace ASCII 1 with + */
	GMT_UNUSED(GMT);
	if (in == NULL || in[0] == '\0') return;	/* No string to check */
	if (way == 0) {	/* Replace any +<letter> with <letter> NOT in <mods> with ASCII 1<letter> */
		char *c = in;
		for ( ;; ) { /* Replace super-script escape sequence @+ with @1 */
			c = strstr (c, "@+");
			if (c == NULL) break;
			++c;
			*c = 1;
		}
		c = in;
		for ( ;; ) { /* Now look for +<letter> */
			c = strchr (c, '+');	/* Find next '+' */
			if (c == NULL) break;	/* No more + found */
			if (!strchr (mods, c[1])) /* Not one of the +<mods> cases so we can replace the + by 1 */
				*c = 1;
			++c;
		}
	}
	else /* way != 0: Replace single ASCII 1 with + */
		GMT_strrepc (in, 1, '+');
}

/*! . */
void gmt5_handle_plussign_orig (struct GMT_CTRL *GMT, char *in, unsigned way)
{	/* Way = 0: replace ++ with ASCII 1, Way = 1: Replace ASCII 1 with + */
	GMT_UNUSED(GMT);
	if (in == NULL || in[0] == '\0') return;	/* No string to check */
	if (way == 0) {	/* Replace pairs of ++ with a single ASCII 1 */
		char *c = in;
		for ( ;; ) { /* Replace ++ with 1 */
			c = strstr (c, "++");
			if (c == NULL) break;
			*c = 1;
			GMT_strlshift (++c, 1);
		}
		c = in;
		for ( ;; ) { /* Replace @+ with @1 */
			c = strstr (c, "@+");
			if (c == NULL) break;
			++c;
			*c = 1;
		}
	}
	else /* way != 0: Replace single ASCII 1 with + */
		GMT_strrepc (in, 1, '+');
}

/*! . */
int gmt5_parse_B_frame_setting (struct GMT_CTRL *GMT, char *in) {
	unsigned int pos = 0, k, error = 0, is_frame = 0;
	char p[GMT_BUFSIZ] = {""}, text[GMT_BUFSIZ] = {""}, *mod = NULL;
	double pole[2];

	/* Parsing of -B<framesettings> */

	/* First determine that the given -B<in> string is indeed the framesetting option.  If not return -1 */

	if (strchr ("pxyz", in[0])) return (-1);	/* -B[p[xyz] is definitively not the frame settings (-Bs is tricker; see below) */
	if (strstr (in, "+b")) is_frame++;	/* Found a +b so likely frame */
	if (strstr (in, "+g")) is_frame++;	/* Found a +g so likely frame */
	if (strstr (in, "+n")) is_frame++;	/* Found a +n so likely frame */
	if (strstr (in, "+o")) is_frame++;	/* Found a +o so likely frame */
	if (strstr (in, "+t")) is_frame++;	/* Found a +t so likely frame */
	if (strchr ("WESNZwenz", in[0])) is_frame++;	/* Found one of the side specifiers so likely frame (left s off since -Bs could trick it) */
	if (in[0] == 's' && (in[1] == 0 || strchr ("WESNZwenz", in[1]) != NULL)) is_frame++;	/* Found -Bs (just draw south axis) or -Bs<another axis flag> */
	if (is_frame == 0) return (-1);		/* No, nothing matched */

	/* OK, here we are pretty sure this is a frame -B statement */

	strcpy (text, in);
	gmt5_handle_plussign (GMT, text, "bgnot", 0);	/* Temporarily change double plus-signs to double ASCII 1 to avoid +<modifier> angst */
	GMT->current.map.frame.header[0] = '\0';

	if ((mod = strchr (text, '+'))) {	/* Find start of modifiers, if any */
		while ((GMT_strtok (mod, "+", &pos, p))) {	/* Parse any +<modifier> statements */
			switch (p[0]) {
				case 'b':	/* Activate 3-D box and x-z, y-z gridlines (if selected) */
					GMT->current.map.frame.draw_box = true;
					break;
				case 'g':	/* Paint the basemap interior */
					if (p[1] == 0 || GMT_getfill (GMT, &p[1], &GMT->current.map.frame.fill)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +g<fill> modifier %c\n", &p[1]);
						error++;
					}
					GMT->current.map.frame.paint = true;
					break;
				case 'n':	/* Turn off frame entirely; this is also done in gmt5_decode_wesnz */
					GMT->current.map.frame.no_frame = true;
					break;
				case 'o':	/* Specify pole for oblique gridlines */
					if (GMT->current.proj.projection == GMT_OBLIQUE_MERC) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Cannot specify oblique gridlines for the oblique Mercator projection\n");
						error++;
					}
					else if (!p[1] || (k = GMT_Get_Value (GMT->parent, &p[1], pole)) != 2) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +o[<plon>/<plat>] modifier %c\n", &p[1]);
						error++;
					}
					else {	/* Successful parsing of pole */
						GMT->current.map.frame.obl_grid = true;
						gmt_set_oblique_pole_and_origin (GMT, pole[GMT_X], pole[GMT_Y], 0.0, 0.0);
					}
					break;
				case 't':
					if (p[1]) {	/* Actual title was appended */
						strcpy (GMT->current.map.frame.header, &p[1]);
						gmt5_handle_plussign (GMT, GMT->current.map.frame.header, NULL, 1);	/* Recover any non-modifier plus signs */
						GMT_enforce_rgb_triplets (GMT, GMT->current.map.frame.header, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
					}
					break;
				default:
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Unrecognized frame modifier %s\n", p);
					error++;
					break;
			}
		}
		*mod = '\0';	/* Separate the modifiers from the frame selectors */
	}

	/* Now parse the frame choices, if any */
	error += gmt5_decode_wesnz (GMT, text, true);

	return (error);
}

/*! . */
int gmt5_parse_B_option (struct GMT_CTRL *GMT, char *in) {
	/* GMT5 clean version based on new syntax:
	 * Frame settings:
	 *	-B[WESNwesnz|Z[1234]][+b][+g<fill>][+o<lon/lat>][+t<title>]
	 *    		+b enables 3-D box and x-z, y-z gridlines.
	 *    		+g<fill> as plot interior fill [none].
	 *    		+t<title> as plot title [none].
	 *    		of one or more of w,e,s,n,z then only those axes will be drawn.
	 *		Upper case letters means the chosen axes also will be annotated.
	 *		Default is determined by MAP_FRAME_AXES setting [WESN].
	 * Axis settings:
	 * 	-B[p|s][x|y|z]<info>
	 *   where <info> is of the format
	 * 	<intervals>[+l<label>][+p<prefix>][+u<unit>]
	 *   and each <intervals> is a concatenation of one or more [t][value][<unit>]
	 *    		+l<label> as labels for the respective axes [none].
	 *    		+u<unit> as as annotation suffix unit [none].
	 *    		+p<prefix> as as annotation prefix unit [none].
	 *
	 * The [t] and [<unit] are optional ([ and ] are NOT part of the string and are
	 * just used to clarify). [t] can be any of [a](annotation int), [f](frame int),
	 * or [g](gridline int).  Default is a AND f.
	 * At the top level, these modifiers are recognized once [repeats are ignored]:
	 * For each axes, these modifies are recognized:
	 * For logscale plots: l will cause log10(x) to be plotted
	 *			p will cause 10 ^ log10(x) to be plotted
	 *	annot/tick/grid interval can here be either:
	 *		1.0	-> Only powers of 10 are annotated
	 *		2.0	-> powers of 10 times (1, 2, 5) are annotated
	 *		3.0	-> powers of 10 times (1,2,3,..9) are annotated
	 *
	 *	-Bs must be in addition to -B[p].
	 */

	char string[GMT_BUFSIZ] = {""}, orig_string[GMT_BUFSIZ] = {""}, text[GMT_BUFSIZ] = {""}, *mod = NULL;
	struct GMT_PLOT_AXIS *A = NULL;
	unsigned int no;
	int k, error = 0;
	bool side[3] = {false, false, false};

	if (!in || !in[0]) return (GMT_PARSE_ERROR);	/* -B requires an argument */

	if (!GMT->current.map.frame.init) {	/* First time we initialize stuff */
		for (no = 0; no < 3; no++) {
			GMT_memset (&GMT->current.map.frame.axis[no], 1, struct GMT_PLOT_AXIS);
			GMT->current.map.frame.axis[no].id = no;
			for (k = 0; k < 6; k++) GMT->current.map.frame.axis[no].item[k].parent = no;
			if (GMT->current.proj.xyz_projection[no] == GMT_TIME) GMT->current.map.frame.axis[no].type = GMT_TIME;
		}
		GMT->common.B.string[0][0] = GMT->common.B.string[1][0] = '\0';
		GMT->current.map.frame.init = true;
		GMT->current.map.frame.draw = false;
		GMT->current.map.frame.set_frame[0] = GMT->current.map.frame.set_frame[1] = 0;
	}

	if ((error = gmt5_parse_B_frame_setting (GMT, in)) >= 0) return (error);	/* Parsed the -B frame settings separately */
	error = 0;	/* Reset since otherwise it is -1 */

	/* Below here are the axis settings only -B[p|s][x|y|z] */
	switch (in[0]) {
		case 's': GMT->current.map.frame.primary = false; k = 1; break;
		case 'p': GMT->current.map.frame.primary = true;  k = 1; break;
		default:  GMT->current.map.frame.primary = true;  k = 0; break;
	}
	no = (GMT->current.map.frame.primary) ? 0 : 1;
	if (GMT->common.B.string[no][0]) {	/* Append this option */
		strcat (GMT->common.B.string[no], " ");
		strcat (GMT->common.B.string[no], in);
	}
	else
		strncpy (GMT->common.B.string[no], in, GMT_LEN256);	/* Keep a copy of the actual option(s) */

	/* Set which axes this option applies to */
	while (in[k] && strchr ("xyz", in[k])) {	/* As long as there are leading x,y,z axes specifiers */
		switch (in[k]) {	/* We specified a named axis */
			case 'x': side[GMT_X] = true; break;
			case 'y': side[GMT_Y] = true; break;
			case 'z': side[GMT_Z] = true; break;
		}
		k++;
	}
	if (!(side[GMT_X] || side[GMT_Y] || side[GMT_Z])) side[GMT_X] = side[GMT_Y] = true;	/* If no axis were named we default to both x and y */

	strcpy (text, &in[k]);			/* Make a copy of the input, starting after the leading -B[p|s][xyz] indicators */
	gmt5_handle_plussign (GMT, text, "lpu", 0);	/* Temporarily change any +<letter> except +l, +p, +u to ASCII 1 to avoid interference with +modifiers */
	k = 0;					/* Start at beginning of text and look for first occurrence of +l, +p, or +s */
	while (text[k] && !(text[k] == '+' && strchr ("lpu", text[k+1]))) k++;
	GMT_memset (orig_string, GMT_BUFSIZ, char);
	strncpy (orig_string, text, k);		/* orig_string now has the interval information */
	gmt5_handle_plussign (GMT, orig_string, NULL, 1);	/* Recover any non-modifier plus signs */
	if (text[k]) mod = &text[k];		/* mod points to the start of the modifier information in text*/
	for (no = 0; no < 3; no++) {		/* Process each axis separately */
		if (!side[no]) continue;	/* Except we did not specify this axis */
		if (!text[0]) continue;	 	/* Skip any empty format string */
		if (text[0] == '0' && !text[1]) {	 /* Understand format '0' to mean "no annotation, ticks, or gridlines" */
			GMT->current.map.frame.draw = true;	/* But we do wish to draw the frame */
			continue;
		}

		if (mod) {	/* Process the given axis modifiers */
			unsigned int pos = 0;
			char p[GMT_BUFSIZ];
			while ((GMT_strtok (mod, "+", &pos, p))) {	/* Parse any +<modifier> statements */
				switch (p[0]) {
					case 'l':	/* Axis label */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No axis label given after +l\n");
							error++;
						}
						else {
							strcpy (GMT->current.map.frame.axis[no].label, &p[1]);
							gmt5_handle_plussign (GMT, GMT->current.map.frame.axis[no].label, NULL, 1);	/* Recover any non-modifier plus signs */
							GMT_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].label, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
						}
						break;
					case 'p':	/* Annotation prefix */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No annotation prefix given after +p\n");
							error++;
						}
						else {
							strcpy (GMT->current.map.frame.axis[no].prefix, &p[1]);
							gmt5_handle_plussign (GMT, GMT->current.map.frame.axis[no].prefix, NULL, 1);	/* Recover any non-modifier plus signs */
							GMT_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].prefix, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
						}
						break;
					case 'u':	/* Annotation unit */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No annotation suffix given after +u\n");
							error++;
						}
						else {
							strcpy (GMT->current.map.frame.axis[no].unit, &p[1]);
							gmt5_handle_plussign (GMT, GMT->current.map.frame.axis[no].unit, NULL, 1);	/* Recover any non-modifier plus signs */
							GMT_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].unit, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
						}
						break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Unrecognized axis modifier %s\n", p);
						error++;
						break;
				}
			}
		}

		/* Now parse the annotation/tick info string */

		if (orig_string[0] == '\0') continue;	/* Got nothing */
		GMT->current.map.frame.set = true;	/* Got here so we are setting intervals */

		GMT_memset (string, GMT_BUFSIZ, char);
		strcpy (string, orig_string);	/* Make a copy of string as it gets messed with below */
		if (string[0] == 'c')		/* Special custom annotation information given via file */
			error += gmt_decode_tinfo (GMT, no, 'c', string, &GMT->current.map.frame.axis[no]);
		else {				/* Parse from back of string for 'a', 'f', 'g' chunks */
			for (k = (int)strlen (string) - 1; k >= 0; k--) {
				if (string[k] == 'a' || string[k] == 'f' || string[k] == 'g') {
					error += gmt_decode_tinfo (GMT, no, string[k], &string[k+1], &GMT->current.map.frame.axis[no]);
					string[k] = '\0';	/* Done with this chunk; replace with terminator */
				}
				else if (k == 0)		/* If no [a|f|g] given then it is implicitly 'a' */
					error += gmt_decode_tinfo (GMT, no, 'a', string, &GMT->current.map.frame.axis[no]);
			}
		}

		/* Make sure we have ticks to match specified annotation stride */
		A = &GMT->current.map.frame.axis[no];
		if (A->item[GMT_ANNOT_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			GMT_memcpy (&A->item[GMT_TICK_UPPER], &A->item[GMT_ANNOT_UPPER], 1, struct GMT_PLOT_AXIS_ITEM);
		if (A->item[GMT_ANNOT_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			GMT_memcpy (&A->item[GMT_TICK_LOWER], &A->item[GMT_ANNOT_LOWER], 1, struct GMT_PLOT_AXIS_ITEM);
		/* Note that item[].type will say 'a', 'A', 'i' or 'I' in these cases, so we know when minor ticks were not set */
	}

	/* Check if we asked for linear projections of geographic coordinates and did not specify a unit (suffix) - if so set degree symbol as unit */
	if (GMT->current.proj.projection == GMT_LINEAR && GMT->current.setting.map_degree_symbol != gmt_none) {
		for (no = 0; no < 2; no++) {
			if (GMT->current.io.col_type[GMT_IN][no] & GMT_IS_GEO && GMT->current.map.frame.axis[no].unit[0] == 0) {
				GMT->current.map.frame.axis[no].unit[0] = '-';
				GMT->current.map.frame.axis[no].unit[1] = (char)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol];
				GMT->current.map.frame.axis[no].unit[2] = '\0';
			}
		}
	}

	return (error);
}

/*! . */
int gmt_parse_B_option (struct GMT_CTRL *GMT, char *in) {
	int error = 0;
	if (GMT->common.B.mode == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: Calling gmt_parse_B_option before gmt_check_b_options somehow\n");
		error = 1;
	}
	else if (GMT->common.B.mode == -1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Mixing of GMT 4 and 5 level syntax is not possible\n");
		error = 1;
	}
	else if (GMT->common.B.mode == 4) {	/* Got GMT 4 syntax */
		if (GMT_compat_check (GMT, 4))
			error = gmt4_parse_B_option (GMT, in);
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Cannot use GMT 4 syntax except in compatibility mode\n");
			error = 1;
		}
	}
	else	/* Clean GMT5 syntax */
		error = gmt5_parse_B_option (GMT, in);

	return (error);
}

/*! . */
int gmt_project_type (char *args, int *pos, bool *width_given) {
	/* Parse the start of the -J option to determine the projection type.
	 * If the first character of args is uppercase, width_given is set to 1.
	 * Pos returns the position of the first character of the parameters
	 * following the projections type.
	 * The return value is the projection type ID (see gmt_project.h), or
	 * GMT_NO_PROJ when unsuccessful.
	 */

	unsigned char t;

	/* Check for upper case */

	*width_given = (args[0] >= 'A' && args[0] <= 'Z');

	/* Compared the first part of the -J arguments against a number of Proj4
	   projection names (followed by a slash) or the 1- or 2-letter abbreviation
	   used prior to GMT 4.2.2. Case is ignored */

	if ((*pos = (int)GMT_strlcmp ("aea/"      , args))) return (GMT_ALBERS);
	if ((*pos = (int)GMT_strlcmp ("aeqd/"     , args))) return (GMT_AZ_EQDIST);
	if ((*pos = (int)GMT_strlcmp ("cyl_stere/", args))) return (GMT_CYL_STEREO);
	if ((*pos = (int)GMT_strlcmp ("cass/"     , args))) return (GMT_CASSINI);
	if ((*pos = (int)GMT_strlcmp ("cea/"      , args))) return (GMT_CYL_EQ);
	if ((*pos = (int)GMT_strlcmp ("eck4/"     , args))) return (GMT_ECKERT4);
	if ((*pos = (int)GMT_strlcmp ("eck6/"     , args))) return (GMT_ECKERT6);
	if ((*pos = (int)GMT_strlcmp ("eqc/"      , args))) return (GMT_CYL_EQDIST);
	if ((*pos = (int)GMT_strlcmp ("eqdc/"     , args))) return (GMT_ECONIC);
	if ((*pos = (int)GMT_strlcmp ("gnom/"     , args))) return (GMT_GNOMONIC);
	if ((*pos = (int)GMT_strlcmp ("hammer/"   , args))) return (GMT_HAMMER);
	if ((*pos = (int)GMT_strlcmp ("laea/"     , args))) return (GMT_LAMB_AZ_EQ);
	if ((*pos = (int)GMT_strlcmp ("lcc/"      , args))) return (GMT_LAMBERT);
	if ((*pos = (int)GMT_strlcmp ("merc/"     , args))) return (GMT_MERCATOR);
	if ((*pos = (int)GMT_strlcmp ("mill/"     , args))) return (GMT_MILLER);
	if ((*pos = (int)GMT_strlcmp ("moll/"     , args))) return (GMT_MOLLWEIDE);
	if ((*pos = (int)GMT_strlcmp ("nsper/"    , args))) return (GMT_GENPER);
	if ((*pos = (int)GMT_strlcmp ("omerc/"    , args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = (int)GMT_strlcmp ("omercp/"   , args))) return (GMT_OBLIQUE_MERC_POLE);
	if ((*pos = (int)GMT_strlcmp ("ortho/"    , args))) return (GMT_ORTHO);
	if ((*pos = (int)GMT_strlcmp ("polar/"    , args))) return (GMT_POLAR);
	if ((*pos = (int)GMT_strlcmp ("poly/"     , args))) return (GMT_POLYCONIC);
	if ((*pos = (int)GMT_strlcmp ("robin/"    , args))) return (GMT_ROBINSON);
	if ((*pos = (int)GMT_strlcmp ("sinu/"     , args))) return (GMT_SINUSOIDAL);
	if ((*pos = (int)GMT_strlcmp ("stere/"    , args))) return (GMT_STEREO);
	if ((*pos = (int)GMT_strlcmp ("tmerc/"    , args))) return (GMT_TM);
	if ((*pos = (int)GMT_strlcmp ("utm/"      , args))) return (GMT_UTM);
	if ((*pos = (int)GMT_strlcmp ("vandg/"    , args))) return (GMT_VANGRINTEN);
	if ((*pos = (int)GMT_strlcmp ("wintri/"   , args))) return (GMT_WINKEL);
	if ((*pos = (int)GMT_strlcmp ("xy/"       , args))) return (GMT_LINEAR);
	if ((*pos = (int)GMT_strlcmp ("z/"        , args))) return (GMT_ZAXIS);

	/* These older codes (up to GMT 4.2.1) took 2 characters */

	if ((*pos = (int)GMT_strlcmp ("kf", args))) return (GMT_ECKERT4);
	if ((*pos = (int)GMT_strlcmp ("ks", args))) return (GMT_ECKERT6);
	if ((*pos = (int)GMT_strlcmp ("oa", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = (int)GMT_strlcmp ("ob", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = (int)GMT_strlcmp ("oc", args))) return (GMT_OBLIQUE_MERC_POLE);

	/* Finally, check only the first letter (used until GMT 4.2.1) */

	*pos = 1;
	t = (unsigned char) tolower((unsigned char) args[0]);
	if (t == 'a') return (GMT_LAMB_AZ_EQ);
	if (t == 'b') return (GMT_ALBERS);
	if (t == 'c') return (GMT_CASSINI);
	if (t == 'd') return (GMT_ECONIC);
	if (t == 'e') return (GMT_AZ_EQDIST);
	if (t == 'f') return (GMT_GNOMONIC);
	if (t == 'g') return (GMT_GENPER);
	if (t == 'h') return (GMT_HAMMER);
	if (t == 'i') return (GMT_SINUSOIDAL);
	if (t == 'j') return (GMT_MILLER);
	if (t == 'k') return (GMT_ECKERT6);
	if (t == 'l') return (GMT_LAMBERT);
	if (t == 'm') return (GMT_MERCATOR);
	if (t == 'n') return (GMT_ROBINSON);
	if (t == 'o') return (GMT_OBLIQUE_MERC);
	if (t == 'p') return (GMT_POLAR);
	if (t == 'q') return (GMT_CYL_EQDIST);
	if (t == 'r') return (GMT_WINKEL);
	if (t == 's') return (GMT_STEREO);
	if (t == 't') return (GMT_TM);
	if (t == 'u') return (GMT_UTM);
	if (t == 'v') return (GMT_VANGRINTEN);
	if (t == 'w') return (GMT_MOLLWEIDE);
	if (t == 'x') return (GMT_LINEAR);
	if (t == 'y') return (GMT_CYL_EQ);
	if (t == 'z') return (GMT_ZAXIS);

	/* Did not find any match. Report error */

	*pos = 0;
	return (GMT_NO_PROJ);
}

/*! . */
int gmt_scale_or_width (struct GMT_CTRL *GMT, char *scale_or_width, double *value) {
	/* Scans character that may contain a scale (1:xxxx or units per degree) or a width.
	   Return 1 upon error. Here we want to make an exception for users giving a scale
	   of 1 in grdproject and mapproject as it is most likely meant to be 1:1. */
	int n, answer;
	answer = strncmp (scale_or_width, "1:", 2U);	/* 0 if scale given as 1:xxxx */
	GMT->current.proj.units_pr_degree = (answer != 0);
	if (GMT->current.proj.units_pr_degree) {	/* Check if we got "1" and this is grd|map-project */
		size_t k = strlen (scale_or_width);
		if (k == 1 && scale_or_width[0] == '1' && GMT_is_grdmapproject (GMT)) {	/* OK, pretend we got 1:1 */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning -J option: Your scale of 1 was interpreted to mean 1:1 since no plotting is involved.\n");
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "If a scale of 1 was intended, please append a unit from %s.\n", GMT_DIM_UNITS_DISPLAY);
			gmt_scale_or_width (GMT, strcat(scale_or_width,":1"), value);
			return (GMT_NOERROR);
		}

		*value = GMT_to_inch (GMT, scale_or_width);
	}
	else {
		n = sscanf (scale_or_width, "1:%lf", value);
		if (n != 1 || *value < 0.0) return (1);
		*value = 1.0 / (*value * GMT->current.proj.unit);
		if (GMT->current.proj.gave_map_width) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -J option: Cannot specify map width with 1:xxxx format\n");
			return (1);
		}
	}
	return (GMT_NOERROR);
}

/*! . */
bool gmt_parse_J_option (struct GMT_CTRL *GMT, char *args) {
	/* gmt_parse_J_option scans the arguments given and extracts the parameters needed
	 * for the specified map projection. These parameters are passed through the
	 * GMT->current.proj structure.  The function returns true if an error is encountered.
	 */

	int i, j, k, m, n, nlen, slash, l_pos[3], p_pos[3], t_pos[3], d_pos[3], id, project;
	int n_slashes = 0, last_pos, error = 0;
	unsigned int mod_flag = 0;
	bool width_given = false;
	double c, az, GMT_units[3] = {0.01, 0.0254, 1.0};      /* No of meters in a cm, inch, m */
	char mod, args_cp[GMT_BUFSIZ] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	char txt_d[GMT_LEN256] = {""}, txt_e[GMT_LEN256] = {""}, last_char;
	char txt_arr[11][GMT_LEN256];

	GMT_memset (l_pos, 3, int);	GMT_memset (p_pos, 3, int);
	GMT_memset (t_pos, 3, int);	GMT_memset (d_pos, 3, int);
	GMT->current.proj.lon0 = GMT->current.proj.lat0 = GMT->session.d_NaN;	/* Projection center, to be set via -J */

	strncpy (GMT->common.J.string, args, GMT_LEN256);	/* Verbatim copy */
	project = gmt_project_type (args, &i, &width_given);
	if (project == GMT_NO_PROJ) return (true);	/* No valid projection specified */
	args += i;

	last_pos = (int)strlen (args) - 1;	/* Position of last character in this string */
	last_char = args[last_pos];
	if (width_given) mod_flag = 1;
	if (last_pos > 0) {	/* Avoid having -JXh|v be misinterpreted */
		switch (last_char) {	/* Check for what kind of width is given (only used if upper case is given below */
			case 'h':	/* Want map HEIGHT instead */
				mod_flag = 2;
				break;
			case '+':	/* Want this to be the MAX dimension of map */
				mod_flag = 3;
				break;
			case '-':	/* Want this to be the MIN dimension of map */
				mod_flag = 4;
				break;
		}
	}
	if (mod_flag > 1) args[last_pos] = 0;	/* Temporarily chop off modifier */

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
		for (j = (int)strlen (args), k = -1; j > 0 && k < 0 && args[j] != '/'; j--) if (args[j] == ':') k = j + 1;
		GMT->current.proj.units_pr_degree = (k == -1) ? true : false;
		GMT_set_cartesian (GMT, GMT_OUT);	/* This may be overridden by mapproject -I */
		if (project != GMT_LINEAR) {
			GMT->current.proj.gave_map_width = mod_flag;
			GMT_set_geographic (GMT, GMT_IN);
		}
	}

	GMT->current.proj.unit = GMT_units[GMT_INCH];	/* No of meters in an inch */
	n = 0;	/* Initialize with no fields found */
	switch (project) {
		case GMT_LINEAR:	/* Linear x/y scaling */
			GMT_set_cartesian (GMT, GMT_IN);	/* This will be overridden below if -Jx or -Jp, for instance */
			GMT->current.proj.compute_scale[GMT_X] = GMT->current.proj.compute_scale[GMT_Y] = width_given;

			/* Default is not involving geographical coordinates */
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_UNKNOWN;

			error += (n_slashes > 1) ? 1 : 0;

			/* Find occurrences of /, l, p, t, or d */
			for (j = 0, slash = 0; args[j] && slash == 0; j++) if (args[j] == '/') slash = j;
			for (j = id = 0; args[j]; j++) {
				if (args[j] == '/') id = 1;
				if (strchr ("Ll"  , (int)args[j])) l_pos[id] = j;
				if (strchr ("Pp"  , (int)args[j])) p_pos[id] = j;
				if (strchr ("Tt"  , (int)args[j])) t_pos[id] = j;
				if (strchr ("DdGg", (int)args[j])) d_pos[id] = j;
			}

			/* Distinguish between p for points and p<power> for scaling */

			n = (int)strlen (args);
			for (j = 0; j < 2; j++) {
				if (!p_pos[j]) continue;
				i = p_pos[j] + 1;
				if (i == n || strchr ("/LlTtDdGg", (int)args[i]))	/* This p is for points since no power is following */
					p_pos[j] = 0;
				else if (strchr("Pp", (int)args[i]))	/* The 2nd p is the p for power */
					p_pos[j]++;
			}

			/* Get x-arguments */

			strncpy (args_cp, args, GMT_BUFSIZ);	/* Since GMT_to_inch modifies the string */
			if (slash) args_cp[slash] = 0;	/* Chop off y part */
			k = (!strncmp (args_cp, "1:", 2U)) ? 1 : -1;	/* Special check for linear proj with 1:xxx scale */
			if (k > 0) {	/* For 1:xxxxx  we cannot have /LlTtDdGg modifiers */
				if (l_pos[GMT_X] || p_pos[GMT_X] || t_pos[GMT_X] || d_pos[GMT_X]) error++;
			}

			if ((i = MAX (l_pos[GMT_X], p_pos[GMT_X])) > 0)
				args_cp[i] = 0;	/* Chop off log or power part */
			else if (t_pos[GMT_X] > 0)
				args_cp[t_pos[GMT_X]] = 0;	/* Chop off time part */
			else if (d_pos[GMT_X] > 0)	/* Chop of trailing 'd' */
				args_cp[d_pos[GMT_X]] = 0;
			if (k > 0)	/* Scale entered as 1:mmmmm - this implies -R is in meters */
				GMT->current.proj.pars[0] = GMT->session.u2u[GMT_M][GMT_INCH] / atof (&args_cp[2]);
			else
				GMT->current.proj.pars[0] = GMT_to_inch (GMT, args_cp);	/* x-scale */
			if (l_pos[GMT_X] > 0)
				GMT->current.proj.xyz_projection[GMT_X] = GMT_LOG10;
			else if (p_pos[GMT_X] > 0) {
				GMT->current.proj.xyz_projection[GMT_X] = GMT_POW;
				GMT->current.proj.pars[2] = atof (&args[p_pos[GMT_X]+1]);	/* pow to raise x */
			}
			else if (t_pos[GMT_X] > 0) {	/* Add option to append time_systems or epoch/unit later */
				GMT->current.proj.xyz_projection[GMT_X] = GMT_TIME;
				GMT->current.io.col_type[GMT_IN][GMT_X] = (args[t_pos[GMT_X]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME;
			}

			if (d_pos[GMT_X] > 0) GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;

			if (slash) {	/* Separate y-scaling desired */
				strncpy (args_cp, &args[slash+1], GMT_BUFSIZ);	/* Since GMT_to_inch modifies the string */
				k = (!strncmp (args_cp, "1:", 2U)) ? 1 : -1;	/* Special check for linear proj with separate 1:xxx scale for y-axis */
				if (k > 0) {	/* For 1:xxxxx  we cannot have /LlTtDdGg modifiers */
					if (l_pos[GMT_Y] || p_pos[GMT_Y] || t_pos[GMT_Y] || d_pos[GMT_Y]) error++;
				}
				if ((i = MAX (l_pos[GMT_Y], p_pos[GMT_Y])) > 0)
					args_cp[i-slash-1] = 0;	/* Chop off log or power part */
				else if (t_pos[GMT_Y] > 0)
					args_cp[t_pos[GMT_Y]-slash-1] = 0;	/* Chop off log or power part */
				else if (d_pos[GMT_Y] > 0)
					args_cp[d_pos[GMT_Y]-slash-1] = 0;	/* Chop of trailing 'd' part */
				if (k > 0)	/* Scale entered as 1:mmmmm - this implies -R is in meters */
					GMT->current.proj.pars[1] = GMT->session.u2u[GMT_M][GMT_INCH] / atof (&args_cp[2]);
				else
					GMT->current.proj.pars[1] = GMT_to_inch (GMT, args_cp);	/* y-scale */

				if (l_pos[GMT_Y] > 0)
					GMT->current.proj.xyz_projection[GMT_Y] = GMT_LOG10;
				else if (p_pos[GMT_Y] > 0) {
					GMT->current.proj.xyz_projection[GMT_Y] = GMT_POW;
					GMT->current.proj.pars[3] = atof (&args[p_pos[GMT_Y]+1]);	/* pow to raise y */
				}
				else if (t_pos[GMT_Y] > 0) {	/* Add option to append time_systems or epoch/unit later */
					GMT->current.proj.xyz_projection[GMT_Y] = GMT_TIME;
					GMT->current.io.col_type[GMT_IN][GMT_Y] = (args[t_pos[GMT_Y]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME;
				}
				if (d_pos[GMT_Y] > 0) GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
			}
			else {	/* Just copy x parameters */
				GMT->current.proj.xyz_projection[GMT_Y] = GMT->current.proj.xyz_projection[GMT_X];
				GMT->current.proj.pars[1] = GMT->current.proj.pars[0];
				GMT->current.proj.pars[3] = GMT->current.proj.pars[2];
				/* Assume -JX<width>[unit]d means a linear geographic plot so x = lon and y = lat */
				if (GMT->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_LON) GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
			}

			/* Not both sizes can be zero, but if one is, we will adjust to the scale of the other */
			if (GMT->current.proj.pars[GMT_X] == 0.0 && GMT->current.proj.pars[GMT_Y] == 0.0) error++;
			break;

		case GMT_ZAXIS:	/* 3D plot */
			GMT->current.proj.compute_scale[GMT_Z] = width_given;
			error += (n_slashes > 0) ? 1 : 0;
			GMT->current.io.col_type[GMT_IN][GMT_Z] = GMT_IS_UNKNOWN;

			/* Find occurrences of l, p, or t */
			for (j = 0; args[j]; j++) {
				if (strchr ("Ll", (int)args[j])) l_pos[GMT_Z] = j;
				if (strchr ("Pp", (int)args[j])) p_pos[GMT_Z] = j;
				if (strchr ("Tt", (int)args[j])) t_pos[GMT_Z] = j;
			}

			/* Distinguish between p for points and p<power> for scaling */

			n = (int)strlen (args);
			if (p_pos[GMT_Z]) {
				i = p_pos[GMT_Z] + 1;
				if (i == n || strchr ("LlTtDdGg", (int)args[i]))	/* This p is for points since no power is following */
					p_pos[GMT_Z] = 0;
				else if (strchr ("Pp", (int)args[i]))	/* The 2nd p is the p for power */
					p_pos[GMT_Z]++;
			}

			/* Get arguments */

			strncpy (args_cp, args, GMT_BUFSIZ);	/* Since GMT_to_inch modifies the string */
			if ((i = MAX (l_pos[GMT_Z], p_pos[GMT_Z])) > 0)
				args_cp[i] = 0;
			else if (t_pos[GMT_Z] > 0)
				args_cp[t_pos[GMT_Z]] = 0;
			GMT->current.proj.z_pars[0] = GMT_to_inch (GMT, args_cp);	/* z-scale */

			if (l_pos[GMT_Z] > 0)
				GMT->current.proj.xyz_projection[GMT_Z] = GMT_LOG10;
			else if (p_pos[GMT_Z] > 0) {
				GMT->current.proj.xyz_projection[GMT_Z] = GMT_POW;
				GMT->current.proj.z_pars[1] = atof (&args[p_pos[GMT_Z]+1]);	/* pow to raise z */
			}
			else if (t_pos[GMT_Z] > 0) {
				GMT->current.proj.xyz_projection[GMT_Z] = GMT_TIME;
				GMT->current.io.col_type[GMT_IN][GMT_Z] = (args[t_pos[GMT_Z]] == 'T') ? GMT_IS_ABSTIME : GMT_IS_RELTIME;
			}
			if (GMT->current.proj.z_pars[0] == 0.0) error++;
			GMT->current.proj.JZ_set = true;
			break;

		case GMT_POLAR:		/* Polar (theta,r) */
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON, GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_FLOAT;
			if (args[0] == 'a' || args[0] == 'A') {
				GMT->current.proj.got_azimuths = true;	/* using azimuths instead of directions */
				i = 1;
			}
			else {
				GMT->current.proj.got_azimuths = false;
				i = 0;
			}
			j = (int)strlen (args) - 1;
			if (args[j] == 'r') {	/* Gave optional r for reverse (elevations, presumably) */
				GMT->current.proj.got_elevations = true;
				args[j] = '\0';	/* Temporarily chop off the r */
			}
			else if (args[j] == 'z') {	/* Gave optional z for annotating depths rather than radius */
				GMT->current.proj.z_down = true;
				args[j] = '\0';	/* Temporarily chop off the z */
			}
			else
				GMT->current.proj.got_elevations = GMT->current.proj.z_down = false;
			if (n_slashes == 1) {	/* Gave optional zero-base angle [0] */
				n = sscanf (args, "%[^/]/%lf", txt_a, &GMT->current.proj.pars[1]);
				if (n == 2) GMT->current.proj.pars[0] = GMT_to_inch (GMT, &txt_a[i]);
				error += (GMT->current.proj.pars[0] <= 0.0 || n != 2) ? 1 : 0;
			}
			else if (n_slashes == 0) {
				GMT->current.proj.pars[0] = GMT_to_inch (GMT, &args[i]);
				n = (args) ? 1 : 0;
				error += (GMT->current.proj.pars[0] <= 0.0 || n != 1) ? 1 : 0;
			}
			else
				error++;
			if (GMT->current.proj.got_elevations) args[j] = 'r';	/* Put the r back in the argument */
			if (GMT->current.proj.z_down) args[j] = 'z';	/* Put the z back in the argument */
			if (GMT->current.proj.got_azimuths) GMT->current.proj.pars[1] = -GMT->current.proj.pars[1];	/* Because azimuths go clockwise */
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
			GMT->current.proj.pars[0] = GMT->session.d_NaN;	/* Will be replaced by central meridian either below or in GMT_map_init_... */
			if (n_slashes == 0)
				n = sscanf (args, "%s", txt_b);
			else if (n_slashes == 1) {
				n = sscanf (args, "%[^/]/%s", txt_a, txt_b);
				error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = 0.0;
			}
			error += gmt_scale_or_width (GMT, txt_b, &GMT->current.proj.pars[1]);
			error += !(n == n_slashes + 1);
			break;

		case GMT_CYL_EQ:	/* Cylindrical Equal Area */
		case GMT_CYL_EQDIST:	/* Equidistant Cylindrical */
		case GMT_CYL_STEREO:	/* Cylindrical Stereographic */
		case GMT_CASSINI:	/* Cassini */
		case GMT_MERCATOR:	/* Mercator */
		case GMT_TM:		/* Transverse Mercator */
		case GMT_POLYCONIC:	/* Polyconic */
			GMT->current.proj.pars[0] = GMT->session.d_NaN;
			GMT->current.proj.pars[1] = 0.0;
			txt_a[0] = txt_b[0] = 0;
			if (n_slashes == 0)
				n = sscanf (args, "%s", txt_c);
			else if (n_slashes == 1)
				n = sscanf (args, "%[^/]/%s", txt_a, txt_c);
			else if (n_slashes == 2)
				n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (txt_a[0]) {
				error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				GMT->current.proj.lon0 = GMT->current.proj.pars[0];
			}
			if (txt_b[0]) {
				error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
				GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			}
			error += gmt_scale_or_width (GMT, txt_c, &GMT->current.proj.pars[2]);
			error += ((project == GMT_CYL_EQ || project == GMT_MERCATOR || project == GMT_POLYCONIC)
				&& fabs (GMT->current.proj.pars[1]) >= 90.0);
			error += !(n == n_slashes + 1);
			break;

		case GMT_ALBERS:	/* Albers Equal-area Conic */
		case GMT_ECONIC:	/* Equidistant Conic */
		case GMT_LAMBERT:	/* Lambert Conformal Conic */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_c, GMT_IS_LAT, &GMT->current.proj.pars[2]), txt_c);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_d, GMT_IS_LAT, &GMT->current.proj.pars[3]), txt_d);
			error += gmt_scale_or_width (GMT, txt_e, &GMT->current.proj.pars[4]);
			error += !(n_slashes == 4 && n == 5);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_ORTHO:
			GMT->current.proj.g_debug = 0;
			GMT->current.proj.g_box = GMT->current.proj.g_outside = GMT->current.proj.g_longlat_set = GMT->current.proj.g_radius = GMT->current.proj.g_auto_twist = false;
			GMT->current.proj.g_sphere = true; /* force spherical as default */
			GMT->current.proj.pars[5] = GMT->current.proj.pars[6] = GMT->current.proj.pars[7] = 0.0;

		case GMT_AZ_EQDIST:	/* Azimuthal equal-distant */
		case GMT_LAMB_AZ_EQ:	/* Lambert Azimuthal Equal-Area */
		case GMT_GNOMONIC:	/* Gnomonic */
			/* -Ja|A or e|e or g|G <lon0>/<lat0>[/<horizon>]/<scale>|<width> */

			if (project == GMT_AZ_EQDIST)	/* Initialize default horizons */
				strcpy (txt_c, "180");
			else if (project == GMT_GNOMONIC)
				strcpy (txt_c, "60");
			else
				strcpy (txt_c, "90");
			if (k >= 0) {	/* Scale entered as 1:mmmmm */
				if (n_slashes == 2)	/* Got lon0/lat0/1:mmmm */
					n = sscanf (args, "%[^/]/%[^/]/1:%lf", txt_a, txt_b, &GMT->current.proj.pars[3]);
				else if (n_slashes == 3)	/* Got lon0/lat0/lath/1:mmmm */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, &GMT->current.proj.pars[3]);
				if (GMT->current.proj.pars[3] != 0.0) GMT->current.proj.pars[3] = 1.0 / (GMT->current.proj.pars[3] * GMT->current.proj.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)	/* Got lon0/lat0/width */
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)	/* Got lon0/lat0/lath/width */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				GMT->current.proj.pars[3] = GMT_to_inch (GMT, txt_d);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)	/* Got lon0/lat0/radius/lat */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)	/* Got lon0/lat0/lat_h/radius/lat */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					GMT->current.proj.pars[3] = GMT_to_inch (GMT, txt_d);
					if (gmt_get_uservalue (GMT, txt_e, GMT->current.io.col_type[GMT_IN][GMT_Y], &c, "oblique latitude")) return 1;
					if (c <= -90.0 || c >= 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_c, GMT_IS_LAT, &GMT->current.proj.pars[2]), txt_c);
			error += (GMT->current.proj.pars[2] <= 0.0 || GMT->current.proj.pars[2] > 180.0 || GMT->current.proj.pars[3] <= 0.0 || (k >= 0 && width_given));
			error += (project == GMT_GNOMONIC && GMT->current.proj.pars[2] >= 90.0);
			error += (project == GMT_ORTHO && GMT->current.proj.pars[2] >= 180.0);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_STEREO:	/* Stereographic */
			strcpy (txt_c, "90");	/* Initialize default horizon */
			if (k >= 0) {	/* Scale entered as 1:mmmmm */
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/1:%lf", txt_a, txt_b, &GMT->current.proj.pars[3]);
				else if (n_slashes == 3) {	/* with true scale at specified latitude */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_e, &GMT->current.proj.pars[3]);
					if (gmt_get_uservalue (GMT, txt_e, GMT->current.io.col_type[GMT_IN][GMT_Y], &c, "oblique latitude")) return 1;
					if (c <= -90.0 || c >= 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
					GMT->current.proj.pars[5] = 1.0;	/* flag for true scale case */
				}
				else if (n_slashes == 4) {
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, txt_e, &GMT->current.proj.pars[3]);
					if (gmt_get_uservalue (GMT, txt_e, GMT->current.io.col_type[GMT_IN][GMT_Y], &c, "oblique latitude")) return 1;
					if (c <= -90.0 || c >= 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
					GMT->current.proj.pars[5] = 1.0;	/* flag for true scale case */
				}
				if (GMT->current.proj.pars[3] != 0.0) GMT->current.proj.pars[3] = 1.0 / (GMT->current.proj.pars[3] * GMT->current.proj.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				GMT->current.proj.pars[3] = GMT_to_inch (GMT, txt_d);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					GMT->current.proj.pars[3] = GMT_to_inch (GMT, txt_d);
					if (gmt_get_uservalue (GMT, txt_e, GMT->current.io.col_type[GMT_IN][GMT_Y], &c, "oblique latitude")) return 1;
					if (c <= -90.0 || c >= 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);
			error += (GMT->current.proj.pars[2] <= 0.0 || GMT->current.proj.pars[2] >= 180.0 || GMT->current.proj.pars[3] <= 0.0 || (k >= 0 && width_given));
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_GENPER:	/* General perspective */

			GMT->current.proj.g_debug = 0;
			GMT->current.proj.g_box = GMT->current.proj.g_outside = GMT->current.proj.g_longlat_set = GMT->current.proj.g_radius = GMT->current.proj.g_auto_twist = false;
			GMT->current.proj.g_sphere = true; /* force spherical as default */

			i = 0;
			for (j = 0 ; j < 2 ; j++) {
				if (args[j] == 'd') {         /* standard genper debugging */
					GMT->current.proj.g_debug = 1;
					i++;
				} else if (args[j] == 'D') {  /* extensive genper debugging */
					GMT->current.proj.g_debug = 2;
					i++;
				} else if (args[j] == 'X') {  /* extreme genper debugging */
					GMT->current.proj.g_debug = 3;
					i++;
				} else if (args[j] == 's') {
					GMT->current.proj.g_sphere = true;
					i++;
				} else if (args[j] == 'e') {
					GMT->current.proj.g_sphere = false;
					i++;
				}
			}

			GMT->current.proj.pars[4] = GMT->current.proj.pars[5] = GMT->current.proj.pars[6] = GMT->current.proj.pars[7] = GMT->current.proj.pars[8] = GMT->current.proj.pars[9] = 0.0;

			if (GMT->current.proj.g_debug > 1) {
				GMT_message (GMT, "genper: arg '%s' n_slashes %d k %d\n", args, n_slashes, j);
				GMT_message (GMT, "initial error %d\n", error);
				GMT_message (GMT, "j = %d\n", j);
				GMT_message (GMT, "width_given %d\n", width_given);
			}

			n = sscanf(args+i, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s",
				&(txt_arr[0][0]), &(txt_arr[1][0]), &(txt_arr[2][0]), &(txt_arr[3][0]),
				&(txt_arr[4][0]), &(txt_arr[5][0]), &(txt_arr[6][0]), &(txt_arr[7][0]),
				&(txt_arr[8][0]), &(txt_arr[9][0]), &(txt_arr[10][0]));

			if (GMT->current.proj.g_debug > 1) {
				for (i = 0 ; i < n ; i ++) {
					GMT_message (GMT, "txt_arr[%d] '%s'\n", i, &(txt_arr[i][0]));
				}
				fflush (NULL);
			}

			if (k >= 0) {
				/* Scale entered as 1:mmmmm */
				m = sscanf(&(txt_arr[n-1][0]),"1:%lf", &GMT->current.proj.pars[2]);
				if (GMT->current.proj.pars[2] != 0.0) {
					GMT->current.proj.pars[2] = 1.0 / (GMT->current.proj.pars[2] * GMT->current.proj.unit);
				}
				error += (m == 0) ? 1 : 0;
				if (error) GMT_message (GMT, "scale entered but couldn't read\n");
			} else  if (width_given) {
				GMT->current.proj.pars[2] = GMT_to_inch (GMT, &(txt_arr[n-1][0]));
			} else {
				GMT->current.proj.pars[2] = GMT_to_inch (GMT, &(txt_arr[n-2][0]));
				/*            GMT->current.proj.pars[3] = GMT_ddmmss_to_degree(txt_i); */
				if (gmt_get_uservalue (GMT, &(txt_arr[n-1][0]), GMT->current.io.col_type[GMT_IN][GMT_Y], &c, "oblique latitude")) return 1;
				if (c <= -90.0 || c >= 90.0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
					error++;
				}
				else
					error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, &(txt_arr[n-1][0]), GMT_IS_LAT, &GMT->current.proj.pars[3]), &(txt_arr[n-1][0]));
				if (error) GMT_message (GMT, "error in reading last lat value\n");
			}
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, &(txt_arr[0][0]), GMT_IS_LON, &GMT->current.proj.pars[0]), &(txt_arr[0][0]));
			if (error) GMT_message (GMT, "error is reading longitude '%s'\n", &(txt_arr[0][0]));
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, &(txt_arr[1][0]), GMT_IS_LAT, &GMT->current.proj.pars[1]), &(txt_arr[1][0]));
			if (error) GMT_message (GMT, "error reading latitude '%s'\n", &(txt_arr[1][0]));

			/* g_alt    GMT->current.proj.pars[4] = atof(txt_c); */
			nlen = (int)strlen(&(txt_arr[2][0]));
			if (txt_arr[2][nlen-1] == 'r') {
				GMT->current.proj.g_radius = true;
				txt_arr[2][nlen-1] = 0;
			}
			error += GMT_verify_expectations (GMT, GMT_IS_FLOAT, GMT_scanf (GMT, &(txt_arr[2][0]), GMT_IS_FLOAT, &GMT->current.proj.pars[4]), &(txt_arr[2][0]));
			if (error) GMT_message (GMT, "error reading altitude '%s'\n", &(txt_arr[2][0]));

			/* g_az    GMT->current.proj.pars[5] = atof(txt_d); */
			nlen = (int)strlen(&(txt_arr[3][0]));
			if (txt_arr[3][nlen-1] == 'l' || txt_arr[3][nlen-1] == 'L') {
				GMT->current.proj.g_longlat_set = true;
				txt_arr[3][nlen-1] = 0;
			}
			error += GMT_verify_expectations (GMT, GMT_IS_GEO, GMT_scanf (GMT, &(txt_arr[3][0]), GMT_IS_GEO, &GMT->current.proj.pars[5]), &(txt_arr[3][0]));
			if (error) GMT_message (GMT, "error reading azimuth '%s'\n", &(txt_arr[3][0]));

			/* g_tilt    GMT->current.proj.pars[6] = atof(txt_e); */
			nlen = (int)strlen(&(txt_arr[4][0]));
			if (txt_arr[4][nlen-1] == 'l' || txt_arr[4][nlen-1] == 'L') {
				GMT->current.proj.g_longlat_set = true;
				txt_arr[4][nlen-1] = 0;
			}
			error += GMT_verify_expectations (GMT, GMT_IS_GEO, GMT_scanf (GMT, &(txt_arr[4][0]), GMT_IS_GEO, &GMT->current.proj.pars[6]), &(txt_arr[4][0]));
			if (error) GMT_message (GMT, "error reading tilt '%s'\n", &(txt_arr[4][0]));

			if (n > 6) {
				/*g_twist   GMT->current.proj.pars[7] = atof(txt_f); */
				nlen = (int)strlen(&(txt_arr[5][0]));
				if (txt_arr[5][nlen-1] == 'n') {
					GMT->current.proj.g_auto_twist = true;
					txt_arr[5][nlen-1] = 0;
				}
				error += GMT_verify_expectations (GMT, GMT_IS_GEO, GMT_scanf (GMT, &(txt_arr[5][0]), GMT_IS_GEO, &GMT->current.proj.pars[7]), &(txt_arr[5][0]));
				if (error) GMT_message (GMT, "error reading twist '%s'\n", &(txt_arr[5][0]));

				/*g_width   GMT->current.proj.pars[8] = atof(txt_f); */
				if (n > 7) {
					error += GMT_verify_expectations (GMT, GMT_IS_GEO, GMT_scanf (GMT, &(txt_arr[6][0]), GMT_IS_GEO, &GMT->current.proj.pars[8]), &(txt_arr[6][0]));
					if (error) GMT_message (GMT, "error reading width '%s'\n", &(txt_arr[6][0]));

					if (n > 8) {
						/* g_height  GMT->current.proj.pars[9] = atof(txt_g); */
						error += GMT_verify_expectations (GMT, GMT_IS_GEO, GMT_scanf (GMT, &(txt_arr[7][0]), GMT_IS_GEO, &GMT->current.proj.pars[9]), &(txt_arr[7][0]));
						if (error) GMT_message (GMT, "error height '%s'\n", &(txt_arr[7][0]));
					}
				}
			}
			error += (GMT->current.proj.pars[2] <= 0.0 || (k >= 0 && width_given));
			if (error) GMT_message (GMT, "final error %d\n", error);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_OBLIQUE_MERC:		/* Oblique mercator, specifying origin and azimuth or second point */
			if (n_slashes == 3) {
				n = sscanf (args, "%[^/]/%[^/]/%lf/%s", txt_a, txt_b, &az, txt_e);
				error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
				c = 10.0;	/* compute point 10 degrees from origin along azimuth */
				GMT->current.proj.pars[2] = GMT->current.proj.pars[0] + atand (sind (c) * sind (az) / (cosd (GMT->current.proj.pars[1]) * cosd (c) - sind (GMT->current.proj.pars[1]) * sind (c) * cosd (az)));
				GMT->current.proj.pars[3] = d_asind (sind (GMT->current.proj.pars[1]) * cosd (c) + cosd (GMT->current.proj.pars[1]) * sind (c) * cosd (az));
			}
			else if (n_slashes == 4) {
				n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
				error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);
				error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_d, GMT_IS_LAT, &GMT->current.proj.pars[3]), txt_d);
			}
			error += gmt_scale_or_width (GMT, txt_e, &GMT->current.proj.pars[4]);
			GMT->current.proj.pars[6] = 0.0;
			error += !(n == n_slashes + 1);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_OBLIQUE_MERC_POLE:	/* Oblique mercator, specifying orgin and pole */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);
			error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_d, GMT_IS_LAT, &GMT->current.proj.pars[3]), txt_d);
			if (GMT->current.proj.pars[3] < 0.0) {	/* Flip from S hemisphere to N */
				GMT->current.proj.pars[3] = -GMT->current.proj.pars[3];
				GMT->current.proj.pars[2] += 180.0;
				if (GMT->current.proj.pars[2] >= 360.0) GMT->current.proj.pars[2] -= 360.0;
			}
			error += gmt_scale_or_width (GMT, txt_e, &GMT->current.proj.pars[4]);
			GMT->current.proj.pars[6] = 1.0;
			error += !(n_slashes == 4 && n == 5);
			project = GMT_OBLIQUE_MERC;
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_UTM:	/* Universal Transverse Mercator */
			if (!strchr (args, '/')) {	/* No UTM zone given, must obtain from -R */
				GMT->current.proj.pars[0] = -1.0;	/* Flag we need zone to be set later */
				error += gmt_scale_or_width (GMT, args, &GMT->current.proj.pars[1]);
			}
			else {
				n = sscanf (args, "%[^/]/%s", txt_a, txt_b);
				GMT->current.proj.pars[0] = atof (txt_a);
				switch (args[0]) {
					case '-':	/* Enforce Southern hemisphere convention for y */
						GMT->current.proj.utm_hemisphere = -1;
						break;
					case '+':	/* Enforce Norther hemisphere convention for y */
						GMT->current.proj.utm_hemisphere = +1;
						break;
					default:	/* Decide in gmt_map_setup based on -R */
						GMT->current.proj.utm_hemisphere = 0;
						break;
				}
				mod = (char)toupper ((int)txt_a[strlen(txt_a)-1]);	/* Check if UTM zone has a valid latitude modifier */
				error = 0;
				if (mod >= 'A' && mod <= 'Z') {	/* Got fully qualified UTM zone, e.g., 33N */
					GMT->current.proj.utm_zoney = mod;
					GMT->current.proj.utm_hemisphere = -1;
					if (mod >= 'N') GMT->current.proj.utm_hemisphere = +1;
					if (mod == 'I' || mod == 'O') error++;	/* No such zones */
				}
				GMT->current.proj.pars[0] = fabs (GMT->current.proj.pars[0]);
				GMT->current.proj.lat0 = 0.0;
				k = irint (GMT->current.proj.pars[0]);
				GMT->current.proj.lon0 = -180.0 + k * 6.0 - 3.0;

				error += (k < 1 || k > 60);	/* Zones must be 1-60 */
				GMT->current.proj.utm_zonex = k;
				error += gmt_scale_or_width (GMT, txt_b, &GMT->current.proj.pars[1]);
				error += !(n_slashes == 1 && n == 2);
			}
			break;

		default:
			error = true;
			project = GMT_NO_PROJ;
			break;
	}

	if (project != GMT_ZAXIS) GMT->current.proj.projection = project;
	if (mod_flag > 1) args[last_pos] = last_char;	/* Restore modifier */

	return (error > 0);
}

/*! Converts c, i, and p into 0,1,3 */
int gmt_get_unit (struct GMT_CTRL *GMT, char c) {

	int i;
	switch ((int)c) {
		case 'c':	/* cm */
			i = GMT_CM;
			break;
		case 'i':	/* inch */
			i = GMT_INCH;
			break;
		case 'm':	/* meter */
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Specifying a plot distance unit in meters is deprecated; use c, i, or p.\n");
				i = GMT_M;
			}
			else	/* error */
				i = -1;
			break;
		case 'p':	/* point */
			i = GMT_PT;
			break;
		default:	/* error */
			i = -1;
			break;
	}
	return (i);
}

/*! Update vector head length and width parameters based on size_z and v_angle, and deal with pen/fill settings */
int GMT_init_vector_param (struct GMT_CTRL *GMT, struct GMT_SYMBOL *S, bool set, bool outline, struct GMT_PEN *pen, bool do_fill, struct GMT_FILL *fill) {
	bool no_outline = false, no_fill = false;
	if (set) {	/* Determine proper settings for head fill or outline */
		if (outline && (S->v.status & GMT_VEC_OUTLINE2) == 0) S->v.pen = *pen;	/* If no +p<pen> but -W<pen> was used, use same pen for vector heads */
		else if (!outline && S->v.status & GMT_VEC_OUTLINE2) *pen = S->v.pen;	/* If no -W<pen> was set but +p<pen> given, use same pen for vector tails */
		else if (!outline && (S->v.status & GMT_VEC_OUTLINE2) == 0) no_outline = true;
		if (do_fill && (S->v.status & GMT_VEC_FILL2) == 0) S->v.fill = *fill;	/* If no +g<fill> but -G<fill> was used, use same fill for vector heads */
		else if (!do_fill && S->v.status & GMT_VEC_FILL2) no_fill = false;		/* If no -G<fill> was set but +g<fill> given, we do nothing here */
		else if (!do_fill && (S->v.status & GMT_VEC_FILL2) == 0) no_fill = true;	/* Neither -G<fill> nor +g<fill> were set */
		if (no_outline && no_fill && (S->v.status & GMT_VEC_HEADS)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot draw vector heads without specifying at least one of head outline or head fill.\n");
			return 1;
		}
	}
	if (GMT_IS_ZERO (S->size_x)) return 0;	/* Not set yet */
	S->v.h_length = (float)S->size_x;
	S->v.h_width = (float)(2.0 * S->v.h_length * tand (0.5 * S->v.v_angle));
	return 0;
}

/*! Parser for -Sv|V, -S=, and -Sm */
int GMT_parse_vector (struct GMT_CTRL *GMT, char symbol, char *text, struct GMT_SYMBOL *S) {

	unsigned int pos = 0, k, error = 0;
	size_t len;
	bool p_opt = false, g_opt = false;
	int j;
	char p[GMT_BUFSIZ];
	double pole[2];

	S->v.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &S->v.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	S->v.status = 0;	/* Start with no flags turned on */
	S->v.v_angle = 30.0f;	S->v.v_norm = -1.0f;	S->v.v_stem = 0.1f;
	S->v.v_kind[0] = S->v.v_kind[1] = GMT_VEC_ARROW;
	for (k = 0; text[k] && text[k] != '+'; k++);	/* Either find the first plus or run out or chars */
	strncpy (p, text, k); p[k] = 0;

	while ((GMT_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
		switch (p[0]) {
			case 'a': S->v.v_angle = (float)atof (&p[1]);	break;	/* Vector head opening angle [30] */
			case 'b':	/* Vector head at beginning point */
				S->v.status |= GMT_VEC_BEGIN;
				switch (p[1]) {
					case 'a': S->v.v_kind[0] = GMT_VEC_ARROW;	/* Explicitly selected arrow head, must check for other modifiers */
		  	  			if (p[2] == 'l') S->v.status |= GMT_VEC_BEGIN_L;	/* Only left  half of head requested */
		  	  			else if (p[2] == 'r') S->v.status |= GMT_VEC_BEGIN_R;	/* Only right half of head requested */
						break;
					case 'c': S->v.v_kind[0] = GMT_VEC_CIRCLE;	break;
					case 't': S->v.v_kind[0] = GMT_VEC_TERMINAL;	break;
			  	 	case 'l': S->v.v_kind[0] = GMT_VEC_ARROW;	S->v.status |= GMT_VEC_BEGIN_L;	break;	/* Only left  half of head requested */
			  	  	case 'r': S->v.v_kind[0] = GMT_VEC_ARROW;	S->v.status |= GMT_VEC_BEGIN_R;	break;	/* Only right half of head requested */
					default:  S->v.v_kind[0] = GMT_VEC_ARROW;	break;
				}
				break;
			case 'e':	/* Vector head at end point */
				S->v.status |= GMT_VEC_END;
				switch (p[1]) {
					case 'a': S->v.v_kind[1] = GMT_VEC_ARROW;	/* Explicitly selected arrow head, must check for other modifiers */
		  	  			if (p[2] == 'l') S->v.status |= GMT_VEC_END_L;		/* Only left  half of head requested */
		  	  			else if (p[2] == 'r') S->v.status |= GMT_VEC_END_R;	/* Only right half of head requested */
						break;
					case 'c': S->v.v_kind[1] = GMT_VEC_CIRCLE;	break;
					case 't': S->v.v_kind[1] = GMT_VEC_TERMINAL;	break;
			  	 	case 'l': S->v.v_kind[1] = GMT_VEC_ARROW;	S->v.status |= GMT_VEC_END_L;	break;	/* Only left  half of head requested */
			  	  	case 'r': S->v.v_kind[1] = GMT_VEC_ARROW;	S->v.status |= GMT_VEC_END_R;	break;	/* Only right half of head requested */
					default:  S->v.v_kind[1] = GMT_VEC_ARROW;	break;
				}
				break;
			case 'l': S->v.status |= (GMT_VEC_BEGIN_L + GMT_VEC_END_L);	break;	/* Obsolete modifier for left halfs at active heads */
			case 'q': S->v.status |= GMT_VEC_ANGLES;	break;	/* Expect start,stop angle rather than length in input */
			case 'r': S->v.status |= (GMT_VEC_BEGIN_R + GMT_VEC_END_R);	break;	/* Obsolete modifier for right halfs at active heads */
			case 's': S->v.status |= GMT_VEC_JUST_S;	break;	/* Input (angle,length) are vector end point (x,y) instead */
			case 'j':	/* Vector justification */
				if (symbol == 'm') {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-Sm does not accept +j<just> modifiers.\n");
					error++;
				}
				else {
					switch (p[1]) {
						case 'b': S->v.status |= GMT_VEC_JUST_B;	break;	/* Input (x,y) refers to vector beginning point */
						case 'c': S->v.status |= GMT_VEC_JUST_C;	break;	/* Input (x,y) refers to vector center point */
						case 'e': S->v.status |= GMT_VEC_JUST_E;	break;	/* Input (x,y) refers to vector end point */
						default:  /* Bad justifier code */
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +j<just> modifier %c\n", p[1]);
							error++;
							break;
					}
				}
				break;
			case 'n':	/* Vector shrinking head */
				len = strlen (p);
				j = (symbol == 'v' || symbol == 'V') ? gmt_get_unit (GMT, p[len]) : -1;	/* Only -Sv|V takes unit */
				if (j >= 0) { S->u = j; S->u_set = true; }
				S->v.v_norm = (float)atof (&p[1]);
				if (symbol == '=') S->v.v_norm /= (float)GMT->current.proj.DIST_KM_PR_DEG;	/* Since norm distance is in km and we compute spherical degrees later */
				break;
			case 'g':	/* Vector head fill +g[-|<fill>]*/
				g_opt = true;	/* Marks that +g was used */
				if (p[1] == '-') break; /* Do NOT turn on fill */
				S->v.status |= GMT_VEC_FILL;
				if (p[1]) {
					if (GMT_getfill (GMT, &p[1], &S->v.fill)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +g<fill> modifier %c\n", &p[1]);
						error++;
					}
					S->v.status |= GMT_VEC_FILL2;
				}
				break;
			case 'o':	/* Sets oblique pole for small or great circles */
				S->v.status |= GMT_VEC_POLE;
				if (!p[1]) {	/* Gave no pole, use North pole */
					S->v.pole[GMT_X] = 0.0f;	S->v.pole[GMT_Y] = 90.0f;
				}
				else if ((j = GMT_Get_Value (GMT->parent, &p[1], pole)) != 2) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +o[<plon>/<plat>] modifier %c\n", &p[1]);
					error++;
				}
				else {	/* Successful parsing of pole */
					S->v.pole[GMT_X] = (float)pole[GMT_X];	S->v.pole[GMT_Y] = (float)pole[GMT_Y];}
				break;
			case 'p':	/* Vector pen and head outline +p[-][<pen>] */
				p_opt = true;	/* Marks that +p was used */
				if (p[1] == '-')	/* Do NOT turn on outlines */
					j = 2;
				else {
					j = 1;
					S->v.status |= GMT_VEC_OUTLINE;
				}
				if (p[j]) {	/* Change default vector pen */
					if (GMT_getpen (GMT, &p[j], &S->v.pen)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +p<pen> modifier %c\n", &p[1]);
						error++;
					}
					S->v.status |= GMT_VEC_OUTLINE2;	/* Flag that a pen specification was given */
				}
				break;
			case 'z':	/* Input (angle,length) are vector components (dx,dy) instead */
				S->v.status |= GMT_VEC_COMPONENTS;
				S->v.comp_scale = (float)GMT_convert_units (GMT, &p[1], GMT->current.setting.proj_length_unit, GMT_INCH);
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad modifier +%c\n", p[0]);
				error++;
				break;
		}
	}
	if (!g_opt) S->v.status |= GMT_VEC_FILL;	/* Default is to fill vector head with current fill unless (a) no fill given or (b) turned off with +g- */
	if (!p_opt) S->v.status |= GMT_VEC_OUTLINE;	/* Default is to draw vector head outline with current pen unless explicitly turned off with +p- */

	/* Set head parameters */
	GMT_init_vector_param (GMT, S, false, false, NULL, false, NULL);

	return (error);
}

/*! . */
int GMT_parse_front (struct GMT_CTRL *GMT, char *text, struct GMT_SYMBOL *S)
{
	/* Parser for -Sf<tickgap>[/<ticklen>][+l|+r][+<type>][+o<offset>][+<pen>]
	 * <tickgap> is required and is either a distance in some unit (append c|i|p)
	 * or it starts with - and gives the number of desired ticks instead.
	 * <ticklen> defaults to 15% of <tickgap> but is required if the number
	 * of ticks are specified. */

	unsigned int pos = 0, k, error = 0;
	int mods, n;
	char p[GMT_BUFSIZ] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};

	for (k = 0; text[k] && text[k] != '+'; k++);	/* Either find the first plus or run out or chars */
	strncpy (p, text, k); p[k] = 0;
	mods = (text[k] == '+');
	if (mods) text[k] = 0;		/* Temporarily chop off the modifiers */
	n = sscanf (&text[1], "%[^/]/%s", txt_a, txt_b);
	if (mods) text[k] = '+';	/* Restore the modifiers */
	if (txt_a[0] == '-' && n == 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error option -Sf: Must specify <ticklen> when specifying the number of ticks\n");
		error++;
	}
	S->f.f_gap = (txt_a[0] == '-') ? atof (txt_a) : GMT_to_inch (GMT, txt_a);
	S->f.f_len = (n == 1) ? 0.15 * S->f.f_gap : GMT_to_inch (GMT, txt_b);

	S->f.f_symbol = GMT_FRONT_FAULT;	/* Default is the fault symbol */
	S->f.f_sense = GMT_FRONT_CENTERED;	/* Default is centered symbols unless +l or +r is found */
	S->f.f_pen = 0;				/* Draw outline with pen set via -W, i.e., same as frontline */
	while ((GMT_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
		switch (p[0]) {
			case 'b':	S->f.f_symbol = GMT_FRONT_BOX;		break;	/* [half-]square front */
			case 'c':	S->f.f_symbol = GMT_FRONT_CIRCLE;	break;	/* [half-]circle front */
			case 'f':	S->f.f_symbol = GMT_FRONT_FAULT;	break;	/* Fault front */
			case 'l':	S->f.f_sense  = GMT_FRONT_LEFT;		break;	/* Symbols to the left */
			case 'r':	S->f.f_sense  = GMT_FRONT_RIGHT;	break;	/* Symbols to the right */
			case 's':	S->f.f_symbol = GMT_FRONT_SLIP;		break;	/* Strike-slip front */
			case 't':	S->f.f_symbol = GMT_FRONT_TRIANGLE;	break;	/* Triangle front */
			case 'o':	S->f.f_off = GMT_to_inch (GMT, &p[1]);	break;	/* Symbol offset along line */
			case 'p':	if (p[1]) {	/* Get alternate pen for front-symbol outline [-W] */
						if (GMT_getpen (GMT, &p[1], &S->f.pen)) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +p<pen> modifier %c\n", &p[1]);
							error++;
						}
						else
							S->f.f_pen = +1;
					}
					else	/* Turn off outline */
						S->f.f_pen = -1;
					break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error option -Sf: Bad modifier +%c\n", p[0]);
				error++;	break;
		}
	}

	return (error);
}

/*! Parse the arguments given to -Sl.  The allowed syntax is:
 * -Sl<size>[unit]+t<text>[+f<font<][+j<justify>] */
int gmt_parse_text (struct GMT_CTRL *GMT, char *text, struct GMT_SYMBOL *S) {

	unsigned int pos = 0, k, j, slash, error = 0;
	if ((!strstr (text, "+t") && strchr (text, '/')) || strchr (text, '%')) {	/* GMT4 syntax */
		char *c = NULL;
		if (GMT_compat_check (GMT, 4)) {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning in Option -Sl: Sl<size>/<string>[%<font>] is deprecated syntax\n");
			if ((c = strchr (text, '%'))) {	/* Gave font name or number, too */
				*c = 0;	/* Chop off the %font info */
				c++;		/* Go to next character */
				if (GMT_getfont (GMT, c, &S->font)) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-Sl contains bad font (set to %s)\n", GMT_putfont (GMT, S->font));
			}
			/* Look for a slash that separates size and string: */
			for (j = 1, slash = 0; text[j] && !slash; j++) if (text[j] == '/') slash = j;
			/* Set j to the first char in the string: */
			j = slash + 1;
			/* Copy string characters */
			k = 0;
			while (text[j] && text[j] != ' ' && k < (GMT_LEN256-1)) S->string[k++] = text[j++];
			S->string[k] = '\0';
			if (!k) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -Sl option: No string given\n");
				error++;
			}
		}
		else {	/* Not accept it unless under compatibility mode 4 */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -Sl option: Usage is -Sl[<size>]+t<string>[+f<font>][+j<justify]\n");
			error++;
		}
	}
	else {	/* GMT5 syntax */
		char p[GMT_BUFSIZ];
		for (k = 0; text[k] && text[k] != '+'; k++);	/* Either find the first plus or run out or chars; should at least find +t */
		if (!text[k]) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -Sl option: No string information given\n");
			return (1);
		}
		while ((GMT_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
			switch (p[0]) {
				case 'f':	/* Change font */
					if (GMT_getfont (GMT, &p[1], &S->font))
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-Sl contains bad +<font> modifier (set to %s)\n", GMT_putfont (GMT, S->font));
					break;
				case 'j':	S->justify = GMT_just_decode (GMT, &p[1], PSL_NO_DEF);	break;	/* text justification */
				case 't':	strncpy (S->string, &p[1], GMT_LEN256);	break;	/* Get the symbol text */
				default:
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error option -Sl: Bad modifier +%c\n", p[0]);
					error++;
					break;
			}
		}
	}

	return (error);
}

#define GMT_VECTOR_CODES "mMvV="	/* The vector symbol codes */

/*! . */
int GMT_parse_symbol_option (struct GMT_CTRL *GMT, char *text, struct GMT_SYMBOL *p, unsigned int mode, bool cmd)
{
	/* mode = 0 for 2-D (psxy) and = 1 for 3-D (psxyz); cmd = true when called to process command line options */
	int decode_error = 0, bset = 0, j, n, k, slash = 0, colon, col_off = mode, len;
	bool check = true, degenerate = false;
	unsigned int ju;
	char symbol_type, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, text_cp[GMT_LEN256] = {""}, diameter[GMT_LEN32] = {""}, *c = NULL;
	static char *allowed_symbols[2] = {"=-+AaBbCcDdEefGgHhIiJjMmNnpqRrSsTtVvWwxy", "=-+AabCcDdEefGgHhIiJjMmNnOopqRrSsTtUuVvWwxy"};
	static char *bar_symbols[2] = {"Bb", "-BbOoUu"};
	if (cmd) {
		p->base = GMT->session.d_NaN;
		p->u = GMT->current.setting.proj_length_unit;
	}
	else {
		p->read_size = p->read_size_cmd;
	}
	p->n_required = p->convert_angles = p->n_nondim = p->base_set = 0;
	p->user_unit[GMT_X] = p->user_unit[GMT_Y] = p->u_set = false;
	p->font = GMT->current.setting.font_annot[0];
	if (p->read_size)  p->given_size_x = p->given_size_y = p->size_x = p->size_y = 0.0;

	/* col_off is the col number of first parameter after (x,y) [or (x,y,z) if mode == 1)].
	   However, if size is not given then that is requred too so col_off++ */

	if (!strncmp (text, "E-", 2U) || !strncmp (text, "J-", 2U)) {	/* Special degenerate geographic ellipse and rectangle symbols, remove the - to avoid parsing issues */
		degenerate = true;
		if (text[2]) strcpy (diameter, &text[2]);	/* Gave circle diameter on command line */
		text[1] = 0;
	}
	if (!text[0]) {	/* No symbol or size given */
		p->size_x = p->size_y = 0.0;
		symbol_type = '*';
		col_off++;
		if (cmd) p->read_size_cmd = true;
		if (cmd) p->read_symbol_cmd = true;
	}
	else if (isdigit ((int)text[0]) || text[0] == '.') {	/* Size, but no symbol given */
		n = sscanf (text, "%[^/]/%s", txt_a, txt_b);
		p->size_x = p->given_size_x = GMT_to_inch (GMT, txt_a);
		if (n == 2)
			p->size_y = p->given_size_y = GMT_to_inch (GMT, txt_b);
		else if (n == 1)
			p->size_y = p->given_size_y = p->size_x;
		else
			decode_error = true;
		symbol_type = '*';
		if (cmd) p->read_symbol_cmd = true;
	}
	else if (text[0] == 'l') {	/* Letter symbol is special case */
		strncpy (text_cp, text, GMT_LEN256);	/* Copy for processing later */
		symbol_type = 'l';
		if (!text[1]) {	/* No size or text given */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if (cmd) p->read_size_cmd = true;
			col_off++;
		}
		else if (text[1] == '+' || (text[1] == '/' && GMT_compat_check (GMT, 4))) {	/* No size given */
			/* Any deprecate message comes below so no need here */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			col_off++;
		}
		else {
			n = sscanf (&text_cp[1], "%[^+]+%*s", txt_a);
			p->size_x = p->given_size_x = GMT_to_inch (GMT, txt_a);
			decode_error = (n != 1);
		}
	}
	else if (text[0] == 'k') {	/* Custom symbol spec */
		for (j = (int)strlen (text); j > 0 && text[j] != '/'; --j);
		if (j == 0) {	/* No slash, i.e., no symbol size given */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
#if 0
			/* Removed because it produced erroneous result in example 20 */
			if (p->size_x == 0.0)		/* It may still come out as zero from the above line */
				p->size_x = p->given_size_x = GMT_to_inch (GMT, "1");
#endif
			n = sscanf (text, "%c%s", &symbol_type, text_cp);
			col_off++;
		}
		else {
			text[j] = ' ';
			n = sscanf (text, "%c%s %s", &symbol_type, text_cp, txt_a);
			text[j] = '/';
			p->size_x = p->given_size_x = GMT_to_inch (GMT, txt_a);
		}
	}
	else if (strchr (GMT_VECTOR_CODES, text[0])) {	/* Vectors gets separate treatment because of optional modifiers [+j<just>+b+e+s+l+r+a<angle>+n<norm>] */
		char arg[GMT_LEN64] = {""};
		n = sscanf (text, "%c%[^+]", &symbol_type, arg);	/* arg should be symbols size with no +<modifiers> at the end */
		if (n == 1) strncpy (arg, &text[1], GMT_LEN64);	/* No modifiers present, set arg to text following symbol code */
		k = 1;
		if (GMT_compat_check (GMT, 4)) {
			int one;
			char txt_c[GMT_LEN256] = {""};
			p->v.parsed_v4 = false;
			if (strchr (text, '/') && !strchr (text, '+')) {	/* Gave old-style arrow dimensions; cannot exactly reproduce GMT 4 arrows since those were polygons */
				p->v.status |= GMT_VEC_END;		/* Default is head at end */
				p->size_y = p->given_size_y = 0.0;
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: <size> = <vectorwidth/headlength/headwidth> is deprecated; see -S%c syntax.\n", text[0]);
				one = (strchr ("bhstBHST", text[1])) ? 2 : 1;
				sscanf (&text[one], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				p->v.v_width  = (float)GMT_to_inch (GMT, txt_a);
				p->v.h_length = (float)GMT_to_inch (GMT, txt_b);
				p->v.h_width  = (float)(GMT_to_inch (GMT, txt_c) * 2.0);
				p->v.v_angle = (float)atand ((0.5 * p->v.h_width / p->v.h_length) * 2.0);
				p->v.parsed_v4 = true;
				p->size_x = p->given_size_x = p->v.h_length;
			}
			else if (strchr ("vV", symbol_type) && text[1] && strchr ("bhstBHST", text[1])) {	/* Old style */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: bhstBHST vector modifiers is deprecated; see -S%c syntax.\n", text[0]);
				p->v.status |= GMT_VEC_END;		/* Default is head at end */
				k = 2;
				strncpy (arg, &text[2], GMT_LEN64);
			}
		}
		if (text[k] && strchr (GMT_DIM_UNITS, (int) text[k])) {	/* No size given, only unit information */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if ((j = gmt_get_unit (GMT, text[k])) < 0) decode_error = true; else { p->u = j; p->u_set = true;}
			col_off++;
			if (cmd) p->read_size_cmd = true;
		}
		else if (!text[k] || text[k] == '+') {	/* No size nor unit, just possible attributes */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			col_off++;
			if (cmd) p->read_size_cmd = true;
		}
		else if (!p->v.parsed_v4) {	/* Need to get size */
			if (cmd) p->read_size_cmd = false;
			p->size_x = p->given_size_x = GMT_to_inch (GMT, arg), check = false;
		}
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && strchr (GMT_DIM_UNITS, (int) text[1])) {	/* Symbol, but no size given (size assumed given on command line), only unit information */
		n = sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		j = 0;
		if (text[1]) {	/* Gave unit information */
			if ((j = gmt_get_unit (GMT, text[1])) < 0)
				decode_error = true;
			else {
				p->u = j; p->u_set = true;
			}
		}
		col_off++;
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && (text[1] == '\n' || !text[1])) {	/* Symbol, but no size given (size assumed given on command line) */
		n = sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		col_off++;
	}
	else if (strchr (bar_symbols[mode], (int) text[0])) {	/* Bar, column, cube with size */

		/* Bar:		-Sb|B[<size_x|size_y>[c|i|p|u]][b[<base>]]				*/
		/* Column:	-So|O[<size_x>[c|i|p|u][/<ysize>[c|i|p|u]]][b[<base>]]	*/
		/* Cube:	-Su|U[<size_x>[c|i|p|u]]	*/

		for (j = 1; text[j]; j++) {	/* Look at chars following the symbol code */
			if (text[j] == '/') slash = j;
			if (text[j] == 'b') bset = j;
		}
		strncpy (text_cp, text, GMT_LEN256);
		if (bset) text_cp[bset] = 0;	/* Chop off the b<base> from copy to avoid confusion when parsing.  <base> is always in user units */
		if (slash) {	/* Separate x/y sizes */
			n = sscanf (text_cp, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);
			decode_error = (n != 3);
			if ((len = (int)strlen (txt_a)) && txt_a[len-1] == 'u') {
				p->user_unit[GMT_X] = true;	/* Specified xwidth in user units */
				txt_a[len-1] = '\0';	/* Chop off the 'u' */
			}
			if ((len = (int)strlen (txt_b)) && txt_b[len-1] == 'u') {
				p->user_unit[GMT_Y] = true;	/* Specified ywidth in user units */
				txt_b[len-1] = '\0';	/* Chop off the 'u' */
			}
			if (p->user_unit[GMT_X]) {
				if (gmt_get_uservalue (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &p->given_size_x, "-Sb|B|o|O|u|u x-size value")) return EXIT_FAILURE;
				p->size_x = p->given_size_x;
			}
			else
				p->size_x = p->given_size_x = GMT_to_inch (GMT, txt_a);
			if (p->user_unit[GMT_Y]) {
				if (gmt_get_uservalue (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &p->given_size_y, "-Sb|B|o|O|u|u y-size value")) return EXIT_FAILURE;
				p->size_y = p->given_size_y;
			}
			else
				p->size_y = p->given_size_y = GMT_to_inch (GMT, txt_b);
		}
		else {	/* Only a single x = y size */
			n = sscanf (text_cp, "%c%s", &symbol_type, txt_a);
			if ((len = (int)strlen (txt_a)) && txt_a[len-1] == 'u') {
				p->user_unit[GMT_X] = p->user_unit[GMT_Y] = true;	/* Specified xwidth [=ywidth] in user units */
				txt_a[len-1] = '\0';	/* Chop off the 'u' */
			}
			if (n == 2) {	/* Gave size */
				if (p->user_unit[GMT_X]) {
					if (gmt_get_uservalue (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &p->given_size_x, "-Sb|B|o|O|u|u x-size value")) return EXIT_FAILURE;
					p->size_x = p->size_y = p->given_size_y = p->given_size_x;
				}
				else
					p->size_x = p->size_y = p->given_size_x = p->given_size_y = GMT_to_inch (GMT, txt_a);
			}
			else {
				if (p->size_x == 0.0) p->size_x = p->given_size_x;
				if (p->size_y == 0.0) p->size_y = p->given_size_y;
			}
		}
	}
	else {	/* Everything else */
		char s_upper;
		n = sscanf (text, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);
		s_upper = (char)toupper ((int)symbol_type);
		if (s_upper == 'F' || s_upper == 'V' || s_upper == 'Q' || s_upper == 'M') {	/* "Symbols" that do not take normal symbol size */
			p->size_y = p->given_size_y = 0.0;
		}
		else {
			p->size_x = p->given_size_x = GMT_to_inch (GMT, txt_a);
			if (n == 3)
				p->size_y = p->given_size_y = GMT_to_inch (GMT, txt_b);
			else if (n == 2)
				p->size_y = p->given_size_y = p->size_x;
			else
				decode_error = true;
		}
	}

	switch (symbol_type) {
		case '*':
			p->symbol = GMT_SYMBOL_NOT_SET;
			break;
		case '-':
			p->symbol = GMT_SYMBOL_XDASH;
			break;
		case 'A':
			p->size_x *= 1.67289326141;	/* To equal area of circle with same diameter */
		case 'a':
			p->symbol = GMT_SYMBOL_STAR;
			break;
		case 'B':
			p->symbol = GMT_SYMBOL_BARX;
			if (bset) {
				if (text[bset+1] == '\0') {	/* Read it from data file */
					p->base_set = 2;
					p->n_required = 1;
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* base in user units */
				}
				else {
					if (gmt_get_uservalue (GMT, &text[bset+1], GMT->current.io.col_type[GMT_IN][GMT_X], &p->base, "-SB base value")) return EXIT_FAILURE;
					p->base_set = 1;
				}
			}
			break;
		case 'b':
			p->symbol = GMT_SYMBOL_BARY;
			if (bset) {
				if (p->user_unit[GMT_Y]) text_cp[strlen(text_cp)-1] = '\0';	/* Chop off u */
				if (text_cp[bset+1] == '\0') {	/* Read it from data file */
					p->base_set = 2;
					p->n_required = 1;
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* base in user units */
				}
				else {
					if (gmt_get_uservalue (GMT, &text_cp[bset+1], GMT->current.io.col_type[GMT_IN][GMT_Y], &p->base, "-Sb base value")) return EXIT_FAILURE;
					p->base_set = 1;
				}
			}
			break;
		case 'C':
		case 'c':
			p->symbol = GMT_SYMBOL_CIRCLE;
			break;
		case 'D':
			p->size_x *= 1.25331413732;	/* To equal area of circle with same diameter */
		case 'd':
			p->symbol = GMT_SYMBOL_DIAMOND;
			break;
		case 'E':	/* Expect axis in km to be scaled based on -J */
			p->symbol = GMT_SYMBOL_ELLIPSE;
			p->convert_angles = 1;
			if (degenerate) {	/* Degenerate ellipse = circle */
				if (diameter[0]) {	/* Gave a fixed diameter as symbol size */
					p->size_x = p->size_y = atof (diameter);	/* In km */
				}
				else {	/* Must read from data file */
					p->n_required = 1;	/* Only expect diameter */
					p->nondim_col[p->n_nondim++] = 2 + mode;	/* Since diameter is in km, not inches or cm etc */
				}
			}
			else {
				p->n_required = 3;
				p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
				p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in km, not inches or cm etc */
				p->nondim_col[p->n_nondim++] = 4 + mode;
			}
			check = false;
			break;
		case 'e':
			p->symbol = GMT_SYMBOL_ELLIPSE;
			/* Expect angle in degrees, then major and major axes in plot units */
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			check = false;
			break;

		case 'f':	/* Fronts: -Sf<spacing>[/<size>][+r+l][+f+t+s+c+b][+o<offset>][+<pen>]	[WAS: -Sf<spacing>/<size>[dir][type][:<offset>]	*/
			p->symbol = GMT_SYMBOL_FRONT;
			p->f.f_off = 0.0;	p->f.f_symbol = GMT_FRONT_FAULT;	p->f.f_sense = GMT_FRONT_CENTERED;
			check = false;
			if (!text[1]) {	/* No args given, must parse segment header later */
				p->fq_parse = true;	/* This will be set to false once at least one header has been parsed */
				break;
			}
			strncpy (text_cp, text, GMT_LEN256);
			if (GMT_compat_check (GMT, 4)) {
				len = (int)strlen (text_cp) - 1;
				if (strchr (text_cp, ':') || (!strchr (text_cp, '+') && len > 0 && strchr ("bcflrst", text_cp[len]))) {	/* Old style */
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning in Option -Sf: Sf<spacing>/<size>[dir][type][:<offset>] is deprecated syntax\n");
					if ((c = strchr (text_cp, ':'))) {	/* Gave :<offset>, set it and strip it off */
						c++;	/* Skip over the colon */
						p->f.f_off = GMT_to_inch (GMT, c);
						c--;	/* Go back to colon */
						*c = 0;	/* Effectively chops off the offset modifier */
					}
					len = (int)strlen (text_cp) - 1;

					switch (text_cp[len]) {
						case 'f':	/* Fault front */
							p->f.f_symbol = GMT_FRONT_FAULT;	len--;	break;
						case 't':	/* Triangle front */
							p->f.f_symbol = GMT_FRONT_TRIANGLE;	len--;	break;
						case 's':	/* Strike-slip front */
							p->f.f_symbol = GMT_FRONT_SLIP;		len--;	break;
						case 'c':	/* [half-]circle front */
							p->f.f_symbol = GMT_FRONT_CIRCLE;	len--;	break;
						case 'b':	/* [half-]square front */
							p->f.f_symbol = GMT_FRONT_BOX;		len--;	break;
						default:
							p->f.f_sense = GMT_FRONT_CENTERED;	break;
					}

					switch (text_cp[len]) {	/* Get sense - default is centered */
						case 'l':
							p->f.f_sense = GMT_FRONT_LEFT;			break;
						case 'r':
							p->f.f_sense = GMT_FRONT_RIGHT;			break;
						default:
							p->f.f_sense = GMT_FRONT_CENTERED;	len++;	break;
					}

					text_cp[len] = 0;	/* Gets rid of the [dir][type] flags, if present */

					/* Pull out and get spacing and size */

					sscanf (&text_cp[1], "%[^/]/%s", txt_a, txt_b);
					p->f.f_gap = (txt_a[0] == '-') ? atof (txt_a) : GMT_to_inch (GMT, txt_a);
					p->f.f_len = GMT_to_inch (GMT, txt_b);
				}
				else
					GMT_parse_front (GMT, text_cp, p);	/* Parse new -Sf syntax */
			}
			else
				GMT_parse_front (GMT, text_cp, p);	/* Parse new -Sf syntax */
			if (p->f.f_sense == GMT_FRONT_CENTERED && p->f.f_symbol == GMT_FRONT_SLIP) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in Option -Sf: Must specify (l)eft-lateral or (r)ight-lateral slip\n");
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			if (GMT_IS_ZERO (p->f.f_gap) || GMT_IS_ZERO (p->f.f_len)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in Option -Sf: Neither <gap> nor <ticklength> can be zero!\n");
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			if (p->f.f_gap < 0.0) {	/* Gave -# of ticks desired */
				k = irint (fabs (p->f.f_gap));
				if (k == 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in Option -Sf: Number of front ticks cannot be zero!\n");
					GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
				}
				if (!GMT_IS_ZERO (p->f.f_off)) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in Option -Sf: +<offset> cannot be used when number of ticks is specified!\n");
					GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
				}
			}
			p->fq_parse = false;	/* No need to parse more later */
			break;
		case 'G':
			p->size_x *= 1.05390736526;	/* To equal area of circle with same diameter */
		case 'g':
			p->symbol = GMT_SYMBOL_OCTAGON;
			break;
		case 'H':
			p->size_x *= 1.09963611079;	/* To equal area of circle with same diameter */
		case 'h':
			p->symbol = GMT_SYMBOL_HEXAGON;
			break;
		case 'I':
			p->size_x *= 1.55512030156;	/* To equal area of circle with same diameter */
		case 'i':
			p->symbol = GMT_SYMBOL_INVTRIANGLE;
			break;
		case 'J':	/* Expect dimensions in km to be scaled based on -J */
			p->symbol = GMT_SYMBOL_ROTRECT;
			p->convert_angles = 1;
			if (degenerate) {	/* Degenerate rectangle = square with zero angle */
				if (diameter[0]) {	/* Gave a fixed diameter as symbol size */
					p->size_x = p->size_y = atof (diameter);	/* In km */
				}
				else {	/* Must read diameter from data file */
					p->n_required = 1;	/* Only expect diameter */
					p->nondim_col[p->n_nondim++] = 2 + mode;	/* Since diameter is in km, not inches or cm etc */
				}
			}
			else {	/* Get all three from file */
				p->n_required = 3;
				p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
				p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in km, not inches or cm etc */
				p->nondim_col[p->n_nondim++] = 4 + mode;
			}
			check = false;
			break;
		case 'j':
			p->symbol = GMT_SYMBOL_ROTRECT;
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			check = false;
			break;
		case 'l':
			p->symbol = GMT_SYMBOL_TEXT;
			if (gmt_parse_text (GMT, text_cp, p)) decode_error++;
			break;
		case 'M':
		case 'm':
			p->symbol = GMT_SYMBOL_MARC;
			p->n_required = 3;	/* Need radius, angle1 and angle2 */
			if (GMT_parse_vector (GMT, symbol_type, text, p)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S%c option\n", symbol_type);
				decode_error++;
			}
			if (symbol_type == 'M') p->v.status |= GMT_VEC_MARC90;	/* Flag means we will plot right angle symbol if angles extend 90 exactly */
			p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Angle */
			p->nondim_col[p->n_nondim++] = 4 + col_off;	/* Angle */
			break;
		case 'N':
			p->size_x *= 1.14948092619;	/* To equal area of circle with same diameter */
		case 'n':
			p->symbol = GMT_SYMBOL_PENTAGON;
			break;
		case 'o':	/*3-D symbol */
			p->shade3D = true;
		case 'O':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_COLUMN;
			if (bset) {
				if (text[bset+1] == '\0') {	/* Read it from data file */
					p->base_set = 2;
					p->n_required = 1;
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* base in user units */
				}
				else {
					if (gmt_get_uservalue (GMT, &text[bset+1], GMT->current.io.col_type[GMT_IN][GMT_Z], &p->base, "-So|O base value")) return EXIT_FAILURE;
					p->base_set = 1;
				}
			}
			if (mode == 0) {
				decode_error = true;
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S option: Symbol type %c is 3-D only\n", symbol_type);
			}
			break;
		case 'P':
		case 'p':
			p->symbol = GMT_SYMBOL_DOT;
			if (p->size_x == 0.0 && !p->read_size) {	/* User forgot to set size */
				p->size_x = GMT_DOT_SIZE;
				check = false;
			}
			break;
		case 'q':	/* Quoted lines: -Sq[d|n|l|x]<info>[:<labelinfo>] */
			p->symbol = GMT_SYMBOL_QUOTED_LINE;
			check = false;
			if (!text[1]) {	/* No args given, must parse segment header later */
				p->fq_parse = true;	/* This will be set to false once at least one header has been parsed */
				break;
			}
			for (j = 1, colon = 0; text[j]; j++) if (text[j] == ':') colon = j;
			if (colon) {	/* Gave :<labelinfo> */
				text[colon] = 0;
				decode_error += GMT_contlabel_info (GMT, 'S', &text[1], &p->G);
				decode_error += GMT_contlabel_specs (GMT, &text[colon+1], &p->G);
			}
			else
				decode_error += GMT_contlabel_info (GMT, 'S', &text[1], &p->G);
			p->fq_parse = false;	/* No need to parse more later */
			break;
		case 'r':
			p->symbol = GMT_SYMBOL_RECT;
			p->n_required = 2;
			check = false;
			break;
		case 'R':
			p->symbol = GMT_SYMBOL_RNDRECT;
			p->n_required = 3;
			check = false;
			break;
		case 'S':
			p->size_x *= 1.25331413732;	/* To equal area of circle with same diameter */
		case 's':
			p->symbol = GMT_SYMBOL_SQUARE;
			break;
		case 'T':
			p->size_x *= 1.55512030156;	/* To equal area of circle with same diameter */
		case 't':
			p->symbol = GMT_SYMBOL_TRIANGLE;
			break;
		case 'u':	/*3-D symbol */
			p->shade3D = true;
		case 'U':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_CUBE;
			if (mode == 0) {
				decode_error = true;
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S option: Symbol type %c is 3-D only\n", symbol_type);
			}
			break;
		case 'V':
			p->convert_angles = 1;
		case 'v':
			p->symbol = GMT_SYMBOL_VECTOR;
			if (!GMT_compat_check (GMT, 4) || (strchr (text, '+') || !p->v.parsed_v4)) {	/* Check if new syntax before decoding */
				if (GMT_parse_vector (GMT, symbol_type, text, p)) {	/* Error decoding new vector syntax */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S%c option\n", symbol_type);
					decode_error++;
				}
				if (!(p->v.status & GMT_VEC_JUST_S)) p->nondim_col[p->n_nondim++] = 2 + col_off;
			}
			else {	/* Parse old-style vector specs */
				int one = 2;
				switch (text[1]) {	/* Check if s(egment), h(ead), b(alance center), or t(ail) have been specified */
					case 'S':	/* Input (x,y) refers to vector head (the tip), double heads */
						p->v.status |= GMT_VEC_BEGIN;
					case 's':	/* Input (x,y) refers to vector head (the tip), head  at end */
						p->v.status |= (GMT_VEC_JUST_S + GMT_VEC_END);
						break;
					case 'H':	/* Input (x,y) refers to vector head (the tip), double heads */
						p->v.status |= GMT_VEC_BEGIN;
					case 'h':	/* Input (x,y) refers to vector head (the tip), single head */
						p->v.status |= (GMT_VEC_JUST_E + GMT_VEC_END);
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
					case 'B':	/* Input (x,y) refers to balance point of vector, double heads */
						p->v.status |= GMT_VEC_BEGIN;
					case 'b':	/* Input (x,y) refers to balance point of vector, single head */
						p->v.status |= (GMT_VEC_JUST_C + GMT_VEC_END);
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
					case 'T':	/* Input (x,y) refers to tail of vector, double heads */
						p->v.status |= GMT_VEC_BEGIN;
					case 't':	/* Input (x,y) refers to tail of vector [Default], single head */
						p->v.status |= (GMT_VEC_JUST_B + GMT_VEC_END);
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
					default:	/* No modifier given, default to tail, single head */
						p->v.status |= (GMT_VEC_JUST_B + GMT_VEC_END);
						one = 1;
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
				}
				for (j = one; text[j] && text[j] != 'n'; j++);
				len = (int)strlen(text) - 1;
				if (text[j] == 'n') {	/* Normalize option used */
					k = gmt_get_unit (GMT, text[len]);
					if (k >= 0) { p->u = k; p->u_set = true; }
					p->v.v_norm = (float)atof (&text[j+1]);
					text[j] = 0;	/* Chop off the shrink part */
				}
				if (text[one]) {
					char txt_c[GMT_LEN256] = {""};
					/* It is possible that the user have appended a unit modifier after
					 * the <size> argument (which here are vector attributes).  We use that
					 * to set the unit, but only if the vector attributes themselves have
					 * units. (If not we would override MEASURE_UNIT without cause).
					 * So, -SV0.1i/0.2i/0.3ic will expect 4th column to have length in cm
					 * while SV0.1i/0.2i/0.3i expects data units in MEASURE_UNIT
					 */

					if (isalpha ((int)text[len]) && isalpha ((int)text[len-1])) {
						k = gmt_get_unit (GMT, text[len]);
						if (k >= 0) { p->u = k; p->u_set = true;}
						text[len] = 0;
					}
					if (!p->v.parsed_v4) {
						sscanf (&text[one], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
						p->v.v_width  = (float)GMT_to_inch (GMT, txt_a);
						p->v.h_length = (float)GMT_to_inch (GMT, txt_b);
						p->v.h_width  = (float)(GMT_to_inch (GMT, txt_c) * 2.0);
						p->v.v_angle = (float)(atand (0.5 * p->v.h_width / p->v.h_length) * 2.0);
					}
				}
				if (p->v.v_norm >= 0.0) text[j] = 'n';	/* Put back the n<shrink> part */
			}
			p->n_required = 2;
			break;
		case 'W':
			p->convert_angles = 1;
		case 'w':
			p->symbol = GMT_SYMBOL_WEDGE;
			p->n_required = 2;
			p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Angle */
			p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Angle */
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
			p->custom = GMT_get_custom_symbol (GMT, text_cp);
			p->n_required = p->custom->n_required;
			for (ju = p->n_nondim = 0; ju < p->n_required; ju++) {	/* Flag input columns that are NOT lengths */
				if (p->custom->type[ju] != GMT_IS_DIMENSION) p->nondim_col[p->n_nondim++] = 2 + col_off + ju;
			}
			break;
		case '=':
			p->symbol = GMT_SYMBOL_GEOVECTOR;
			p->convert_angles = 1;
			p->n_required = 2;
			if (GMT_parse_vector (GMT, symbol_type, text, p)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S= option\n");
				decode_error++;
			}
			if (p->v.status & GMT_VEC_POLE) {	/* Small circle vector */
				if (p->v.status & GMT_VEC_ANGLES) {
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Start angle */
					p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Stop angle */
				}
				else {
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Arc length */
					p->n_required = 1;
				}
			}
			else {	/* Great circle vector */
				p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Angle [or longitude] */
				p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Arc length [or latitude] */
			}
			break;
		default:
			decode_error = true;
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S option: Unrecognized symbol type %c\n", symbol_type);
			break;
	}
	if (p->n_nondim > GMT_MAX_SYMBOL_COLS) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal Error.  Must change GMT_MAX_SYMBOL_COLS\n");
	}
	if (p->given_size_x == 0.0 && check) {
		p->read_size = true;
		p->n_required++;
		if (p->symbol == GMT_SYMBOL_COLUMN) p->n_required++;
	}
	else
		p->read_size = false;
	if (bset || cmd) { /* Since we may not know if we have logarithmic projection at this point, skip the next checks. */ }
	else if (p->symbol == GMT_SYMBOL_BARX)
		p->base = (GMT->current.proj.xyz_projection[GMT_X] == GMT_LOG10) ? 1.0 : 0.0;
	else if (p->symbol == GMT_SYMBOL_BARY)
		p->base = (GMT->current.proj.xyz_projection[GMT_Y] == GMT_LOG10) ? 1.0 : 0.0;
	else if (p->symbol == GMT_SYMBOL_COLUMN)
		p->base = (GMT->current.proj.xyz_projection[GMT_Z] == GMT_LOG10) ? 1.0 : 0.0;

	return (decode_error);
}

/*! Loads the m_per_unit array with the scaling factors that converts various units to meters.
 * Also sets all the names for the units.
 * See gmt_project.h for enums that can be used as array indices) */
void gmt_init_unit_conversion (struct GMT_CTRL *GMT) {

	GMT->current.proj.m_per_unit[GMT_IS_METER]		= 1.0;				/* m in m */
	GMT->current.proj.m_per_unit[GMT_IS_KM]			= METERS_IN_A_KM;		/* m in km */
	GMT->current.proj.m_per_unit[GMT_IS_MILE]		= METERS_IN_A_MILE;		/* m in miles */
	GMT->current.proj.m_per_unit[GMT_IS_NAUTICAL_MILE]	= METERS_IN_A_NAUTICAL_MILE;	/* m in nautical mile */
	GMT->current.proj.m_per_unit[GMT_IS_INCH]		= 0.0254;			/* m in inch */
	GMT->current.proj.m_per_unit[GMT_IS_CM]			= 0.01;				/* m in cm */
	GMT->current.proj.m_per_unit[GMT_IS_PT]			= 0.0254 / 72.0;		/* m in point */
	GMT->current.proj.m_per_unit[GMT_IS_FOOT]		= METERS_IN_A_FOOT;		/* m in foot */
	GMT->current.proj.m_per_unit[GMT_IS_SURVEY_FOOT]	= METERS_IN_A_SURVEY_FOOT;	/* m in US Survey foot */

	strcpy (GMT->current.proj.unit_name[GMT_IS_METER],		"m");
	strcpy (GMT->current.proj.unit_name[GMT_IS_KM],		 	"km");
	strcpy (GMT->current.proj.unit_name[GMT_IS_MILE],		"mile");
	strcpy (GMT->current.proj.unit_name[GMT_IS_NAUTICAL_MILE], 	"nautical mile");
	strcpy (GMT->current.proj.unit_name[GMT_IS_INCH],		"inch");
	strcpy (GMT->current.proj.unit_name[GMT_IS_CM],		 	"cm");
	strcpy (GMT->current.proj.unit_name[GMT_IS_PT],		 	"point");
	strcpy (GMT->current.proj.unit_name[GMT_IS_FOOT],		"foot");
	strcpy (GMT->current.proj.unit_name[GMT_IS_SURVEY_FOOT],	"survey foot");
}

/*! . */
int GMT_init_scales (struct GMT_CTRL *GMT, unsigned int unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name) {
	/* unit is 0-8 (see gmt_project.h for enums) and stands for m, km, mile, nautical mile, inch, cm, point, foot, or (US) survey foot */
	/* fwd_scale is used to convert user distance units to meter */
	/* inv_scale is used to convert meters to user distance units */
	/* inch_to_unit is used to convert internal inches to users units (c, i, p) */
	/* unit_to_inch is used to convert users units (c, i, p) to internal inches */
	/* unit_name (unless NULL) is set to the name of the user's map measure unit (cm/inch/point) */

	if (unit >= GMT_N_UNITS) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT ERROR: Unit id must be 0-%d\n", GMT_N_UNITS-1);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}

	/* These scales are used when 1:1 is not set to ensure that the
	 * output (or input with -I) is given (taken) in the units set
	 * by PROJ_LENGTH_UNIT */

	switch (GMT->current.setting.proj_length_unit) {
		case GMT_CM:
			*inch_to_unit = 2.54;
			if (unit_name) strcpy (unit_name, "cm");
			break;
		case GMT_INCH:
			*inch_to_unit = 1.0;
			if (unit_name) strcpy (unit_name, "inch");
			break;
		case GMT_PT:
			*inch_to_unit = 72.0;
			if (unit_name) strcpy (unit_name, "point");
			break;
		case GMT_M:
			if (GMT_compat_check (GMT, 4)) {
				*inch_to_unit = 0.0254;
				if (unit_name) strcpy (unit_name, "m");
			}
			break;
	}
	*unit_to_inch = 1.0 / (*inch_to_unit);
	*fwd_scale = 1.0 / GMT->current.proj.m_per_unit[unit];
	*inv_scale = GMT->current.proj.m_per_unit[unit];
	return GMT_OK;
}

/*! Converts character unit (e.g., 'k') to unit number (e.g., GMT_IS_KM) */
enum GMT_enum_units GMT_get_unit_number (struct GMT_CTRL *GMT, char unit) {
	enum GMT_enum_units mode;
	GMT_UNUSED(GMT);

	switch (unit) {
		case '\0':
		case 'e':
			mode = GMT_IS_METER;
			break;
		case 'k':
			mode = GMT_IS_KM;
			break;
		case 'M':
			mode = GMT_IS_MILE;
			break;
		case 'n':
			mode = GMT_IS_NAUTICAL_MILE;
			break;
		case 'i':
			mode = GMT_IS_INCH;
			break;
		case 'c':
			mode = GMT_IS_CM;
			break;
		case 'p':
			mode = GMT_IS_PT;
			break;
		case 'f':
			mode = GMT_IS_FOOT;
			break;
		case 'u':
			mode = GMT_IS_SURVEY_FOOT;
			break;
		default:
			mode = GMT_IS_NOUNIT;
	}

	return (mode);
}

/*! . */
unsigned int GMT_check_scalingopt (struct GMT_CTRL *GMT, char option, char unit, char *unit_name) {
	int smode;
	unsigned int mode;

	if ((smode = GMT_get_unit_number (GMT, unit)) == GMT_IS_NOUNIT) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT ERROR Option -%c: Only append one of %s|%s\n",
		            option, GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS2_DISPLAY);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	mode = (unsigned int)smode;
	switch (mode) {
		case GMT_IS_METER:		strcpy (unit_name, "m");		break;
		case GMT_IS_KM:			strcpy (unit_name, "km");		break;
		case GMT_IS_MILE:		strcpy (unit_name, "mile");		break;
		case GMT_IS_NAUTICAL_MILE:	strcpy (unit_name, "nautical mile");	break;
		case GMT_IS_INCH:		strcpy (unit_name, "inch");		break;
		case GMT_IS_CM:			strcpy (unit_name, "cm");		break;
		case GMT_IS_PT:			strcpy (unit_name, "point");		break;
		case GMT_IS_FOOT:		strcpy (unit_name, "foot");		break;
		case GMT_IS_SURVEY_FOOT:	strcpy (unit_name, "survey foot");	break;
	}

	return (mode);
}

/*! Option to override the GMT measure unit default */
int GMT_set_measure_unit (struct GMT_CTRL *GMT, char unit) {
	int k;

	if ((k = gmt_get_unit (GMT, unit)) < 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Bad plot measure selected (%c); use c, i, or p.\n", unit);
		return (GMT_MAP_BAD_MEASURE_UNIT);
	}
	GMT->current.setting.proj_length_unit = k;
	return (GMT_NOERROR);
}

/*! Use to parse various -S -Q options when backwardsness has been enabled */
int backwards_SQ_parsing (struct GMT_CTRL *GMT, char option, char *item) {
	int j;

	GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -%c[-]<mode>[/<threshold>] is deprecated. Use -n<mode>[+a][+t<threshold>] instead.\n", (int)option);

	for (j = 0; j < 3 && item[j]; j++) {
		switch (item[j]) {
			case '-':
				GMT->common.n.antialias = false; break;
			case 'n':
				GMT->common.n.interpolant = BCR_NEARNEIGHBOR; break;
			case 'l':
				GMT->common.n.interpolant = BCR_BILINEAR; break;
			case 'b':
				GMT->common.n.interpolant = BCR_BSPLINE; break;
			case 'c':
				GMT->common.n.interpolant = BCR_BICUBIC; break;
			case '/':
				GMT->common.n.threshold = atof (&item[j+1]);
				if (GMT->common.n.threshold < 0.0 || GMT->common.n.threshold > 1.0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Interpolation threshold must be in [0,1] range\n");
					return (1);
				}
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Specify -%c[-]b|c|l|n[/threshold] to set grid interpolation mode.\n", option);
				return (1);
				break;
		}
	}
	return (GMT_NOERROR);
}

/*! GMT_parse_common_options interprets the command line for the common, unique options
 * -B, -J, -K, -O, -P, -R, -U, -V, -X, -Y, -b, -c, -f, -g, -h, -i, -n, -o, -p, -r, -s, -t, -:, -- and -^.
 * The list passes all of these that we should consider.
 * The API will also consider -I for grid increments.
 */
int GMT_parse_common_options (struct GMT_CTRL *GMT, char *list, char option, char *item) {

	int error = 0, i = 0;	/* The i and i+= GMT_more_than_once are there to avoid compiler warnings... */

	if (!list || !strchr (list, option)) return (0);	/* Not a common option we accept */

	if (GMT_compat_check (GMT, 4)) {
		/* Translate some GMT4 options */
		switch (option) {
			case 'E': GMT_COMPAT_OPT ('p'); break;
			case 'F': GMT_COMPAT_OPT ('r'); break;
			case 'H': GMT_COMPAT_OPT ('h'); break;
		}
	}

	switch (option) {	/* Handle parsing of this option, if allowed here */

		case 'B':
			switch (item[0]) {	/* Check for -B[p] and -Bs */
				case 's': GMT->common.B.active[1] = true; break;
				default:  GMT->common.B.active[0] = true; break;
			}
			if (!error) error = gmt_parse_B_option (GMT, item);
			break;

		case 'I':
			if (GMT->hidden.func_level > 0) return (0);	/* Just skip if we are inside a GMT module. -I is an API common option only */
			if (GMT_getinc (GMT, item, GMT->common.API_I.inc)) {
				GMT_inc_syntax (GMT, 'I', 1);
				error++;
			}
			GMT->common.API_I.active = true;
			break;

		case 'J':
			if (item && (item[0] == 'Z' || item[0] == 'z')) {	/* -JZ or -Jz */
				error += (GMT_check_condition (GMT, GMT->common.J.zactive, "Warning: Option -JZ|z given more than once\n") || gmt_parse_J_option (GMT, item));
				GMT->common.J.zactive = true;
			}
			else {	/* Horizontal map projection */
				error += (GMT_check_condition (GMT, GMT->common.J.active, "Warning: Option -J given more than once\n") || gmt_parse_J_option (GMT, item));
				GMT->common.J.active = true;
			}
			break;

		case 'K':
			i += GMT_more_than_once (GMT, GMT->common.K.active);
			GMT->common.K.active = true;
			break;

		case 'O':
			i += GMT_more_than_once (GMT, GMT->common.O.active);
			GMT->common.O.active = true;
			break;

		case 'P':
			i += GMT_more_than_once (GMT, GMT->common.P.active);
			GMT->common.P.active = true;
			break;

		case 'Q':
		case 'S':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -%c is deprecated. Use -n instead.\n" GMT_COMPAT_INFO, option);
				error += backwards_SQ_parsing (GMT, option, item);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
				return (1);
			}
			break;

		case 'R':
			error += (GMT_more_than_once (GMT, GMT->common.R.active) || gmt_parse_R_option (GMT, item));
			GMT->common.R.active = true;
			break;

		case 'U':
			error += (GMT_more_than_once (GMT, GMT->common.U.active) || gmt_parse_U_option (GMT, item));
			GMT->common.U.active = true;
			break;

		case 'V':
			i += GMT_more_than_once (GMT, GMT->common.V.active);
			GMT->common.V.active = true;
			if (item && item[0]) {	/* Specified a verbosity level */
				if (gmt_parse_V_option (GMT, item[0])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unknown argument to -V option, -V%c\n", item[0]);
					error++;
				}
			}
			else
				GMT->current.setting.verbose = GMT_MSG_VERBOSE;
			break;

		case 'X':
			error += (GMT_more_than_once (GMT, GMT->common.X.active) || gmt_parse_XY_option (GMT, GMT_X, item));
			GMT->common.X.active = true;
			break;

		case 'Y':
			error += (GMT_more_than_once (GMT, GMT->common.Y.active) || gmt_parse_XY_option (GMT, GMT_Y, item));
			GMT->common.Y.active = true;
			break;

		case 'Z':	/* GMT4 Backwards compatibility */
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -Z[<zlevel>] is deprecated. Use -p<azim>/<elev>[/<zlevel>] instead.\n" GMT_COMPAT_INFO);
				if (item && item[0]) {
					if (gmt_get_uservalue (GMT, item, GMT->current.io.col_type[GMT_IN][GMT_Z], &GMT->current.proj.z_level, "-Z zlevel value")) return 1;
				}
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
				return (1);
			}
			break;

		case 'a':
			error += (GMT_more_than_once (GMT, GMT->common.a.active) || gmt_parse_a_option (GMT, item));
			GMT->common.a.active = true;
			break;

		case 'b':
			switch (item[0]) {
				case 'i':
					error += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN], "Warning Option -bi given more than once\n");
					GMT->common.b.active[GMT_IN] = true;
					break;
				case 'o':
					error += GMT_check_condition (GMT, GMT->common.b.active[GMT_OUT], "Warning Option -bo given more than once\n");
					GMT->common.b.active[GMT_OUT] = true;
					break;
				default:
					error += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] + GMT->common.b.active[GMT_OUT], "Warning Option -b given more than once\n");
					GMT->common.b.active[GMT_IN] = GMT->common.b.active[GMT_OUT] = true;
					break;
			}
			error += gmt_parse_b_option (GMT, item);
			break;

		case 'c':
			error += (GMT_more_than_once (GMT, GMT->common.c.active) || gmt_parse_c_option (GMT, item));
			GMT->common.c.active = true;
			break;

		case 'd':
			switch (item[0]) {
				case 'i':
					error += GMT_check_condition (GMT, GMT->common.d.active[GMT_IN], "Warning Option -di given more than once\n");
					break;
				case 'o':
					error += GMT_check_condition (GMT, GMT->common.d.active[GMT_OUT], "Warning Option -do given more than once\n");
					break;
				default:
					error += GMT_check_condition (GMT, GMT->common.d.active[GMT_IN] + GMT->common.d.active[GMT_OUT], "Warning Option -d given more than once\n");
					break;
			}
			error += gmt_parse_d_option (GMT, item);
			break;

		case 'f':
			switch (item[0]) {
				case 'i':
					//error += GMT_check_condition (GMT, GMT->common.f.active[GMT_IN], "Warning Option -fi given more than once\n");
					GMT->common.f.active[GMT_IN] = true;
					break;
				case 'o':
					//error += GMT_check_condition (GMT, GMT->common.f.active[GMT_OUT], "Warning Option -fo given more than once\n");
					GMT->common.f.active[GMT_OUT] = true;
					break;
				default:
					//error += GMT_check_condition (GMT, GMT->common.f.active[GMT_IN] | GMT->common.f.active[GMT_OUT], "Warning Option -f given more than once\n");
					GMT->common.f.active[GMT_IN] = GMT->common.f.active[GMT_OUT] = true;
					break;
			}
			error += gmt_parse_f_option (GMT, item);
			break;

		case 'g':
			error += gmt_parse_g_option (GMT, item);
			GMT->common.g.active = true;
			break;

		case 'h':
			error += (GMT_more_than_once (GMT, GMT->common.h.active) || gmt_parse_h_option (GMT, item));
			GMT->common.h.active = true;
			break;

		case 'i':
			error += (GMT_more_than_once (GMT, GMT->common.i.active) || gmt_parse_i_option (GMT, item));
			GMT->common.i.active = true;
			break;

		case 'M':	/* Backwards compatibility */
		case 'm':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -%c is deprecated. Segment headers are automatically identified.\n", option);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
				return (1);
			}
			break;

		case 'n':
			error += (GMT_more_than_once (GMT, GMT->common.n.active) || gmt_parse_n_option (GMT, item));
			GMT->common.n.active = true;
			break;

		case 'o':
			error += (GMT_more_than_once (GMT, GMT->common.o.active) || gmt_parse_o_option (GMT, item));
			GMT->common.o.active = true;
			break;

		case 'p':
			error += (GMT_more_than_once (GMT, GMT->common.p.active) || gmt_parse_p_option (GMT, item));
			GMT->common.p.active = true;
			break;

		case 'r':
			if (GMT->current.io.grd_info.active) GMT->common.r.active = false;	/* OK to override registration given via -Rfile */
			error += GMT_more_than_once (GMT, GMT->common.r.active);
			GMT->common.r.active = true;
			GMT->common.r.registration = GMT_GRID_PIXEL_REG;
			break;

		case 's':
			error += (GMT_more_than_once (GMT, GMT->common.s.active) || gmt_parse_s_option (GMT, item));
			GMT->common.s.active = true;
			break;

		case 't':
			error += GMT_more_than_once (GMT, GMT->common.t.active);
			if (item[0]) {
				GMT->common.t.active = true;
				GMT->common.t.value = atof (item);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -t was not given any value (please add transparency in 0-100%% range)!\n");
				error++;
			}
			break;

#ifdef HAVE_GLIB_GTHREAD
		case 'x':
			error += (GMT_more_than_once (GMT, GMT->common.x.active) || gmt_parse_x_option (GMT, item));
			GMT->common.x.active = true;
			break;
#endif

		case ':':
			error += (GMT_more_than_once (GMT, GMT->common.colon.active) || gmt_parse_colon_option (GMT, item));
			GMT->common.colon.active = true;
			break;

		case '^':
			if (GMT->common.synopsis.active) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Option - given more than once\n");
			GMT->common.synopsis.active = true;
			break;

		case '-':
			error += GMT_parse_dash_option (GMT, item);
			break;

		case '>':	/* Registered output file; nothing to do here */
			break;

		default:	/* Here we end up if an unrecognized option is passed (should not happen, though) */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
			return (1);
			break;
	}

	/* On error, give syntax message */

	if (error) GMT_syntax (GMT, option);

	return (error);
}

/*! . */
int gmt_scanf_epoch (struct GMT_CTRL *GMT, char *s, int64_t *rata_die, double *t0) {

	/* Read a string which must be in one of these forms:
		[-]yyyy-mm-dd[T| [hh:mm:ss.sss]]
		[-]yyyy-Www-d[T| [hh:mm:ss.sss]]
	   Hence, data and clock can be separated by 'T' or ' ' (space), and the clock string is optional.
	   In fact, seconds can be decimal or integer, or missing. Minutes and hour are optional too.
	   Examples: 2000-01-01, 2000-01-01T, 2000-01-01 00:00, 2000-01-01T00, 2000-01-01T00:00:00.000
	*/

	double ss = 0.0;
	int i, yy, mo, dd, hh = 0, mm = 0;
	int64_t rd;
	char tt[8];

	i = 0;
	while (s[i] && s[i] == ' ') i++;
	if (!(s[i])) return (-1);
	if (strchr (&s[i], 'W') ) {	/* ISO calendar string, date with or without clock */
		if (sscanf (&s[i], "%5d-W%2d-%1d%[^0-9:-]%2d:%2d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (GMT_iso_ywd_is_bad (yy, mo, dd) ) return (-1);
		rd = GMT_rd_from_iywd (GMT, yy, mo, dd);
	}
	else {				/* Gregorian calendar string, date with or without clock */
		if (sscanf (&s[i], "%5d-%2d-%2d%[^0-9:-]%2d:%2d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (GMT_g_ymd_is_bad (yy, mo, dd) ) return (-1);
		rd = GMT_rd_from_gymd (GMT, yy, mo, dd);
	}
	if (GMT_hms_is_bad (hh, mm, ss)) return (-1);

	*rata_die = rd;								/* Rata day number of epoch */
	*t0 =  (GMT_HR2SEC_F * hh + GMT_MIN2SEC_F * mm + ss) * GMT_SEC2DAY;	/* Fractional day (0<= t0 < 1) since rata_die of epoch */
	return (GMT_NOERROR);
}

/*! . */
int GMT_init_time_system_structure (struct GMT_CTRL *GMT, struct GMT_TIME_SYSTEM *time_system) {
	/* Processes strings time_system.unit and time_system.epoch to produce a time system scale
	   (units in seconds), inverse scale, and rata die number and fraction of the epoch (days).
	   Return values: 0 = no error, 1 = unit error, 2 = epoch error, 3 = unit and epoch error.
	*/
	int error = GMT_NOERROR;

	/* Check the unit sanity */
	switch (time_system->unit) {
		case 'y':
		case 'Y':
			/* This is a kludge: we assume all years are the same length, thinking that a user
			with decimal years doesn't care about precise time.  To do this right would
			take an entirely different scheme, not a simple unit conversion. */
			time_system->scale = GMT_YR2SEC_F;
			break;
		case 'o':
		case 'O':
			/* This is also a kludge: we assume all months are the same length, thinking that a user
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
		case 's':
		case 'S':
			time_system->scale = 1.0;
			break;
		case 'c':
		case 'C':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Unit c (seconds) is deprecated; use s instead.\n");
				time_system->scale = 1.0;
			}
			else
				error ++;
			break;
		default:
			error ++;
			break;
	}

	/* Set inverse scale and store it to avoid divisions later */
	time_system->i_scale = 1.0 / time_system->scale;

	/* Now convert epoch into rata die number and fraction */
	if (gmt_scanf_epoch (GMT, time_system->epoch, &time_system->rata_die, &time_system->epoch_t0)) error += 2;

	if (error & 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: TIME_UNIT is invalid.  Default assumed.\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Choose one only from y o d h m s\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Corresponding to year month day hour minute second\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Note year and month are simply defined (365.2425 days and 1/12 of a year)\n");
	}
	if (error & 2) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: TIME_EPOCH format is invalid.  Default assumed.\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "    A correct format has the form [-]yyyy-mm-ddThh:mm:ss[.xxx]\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "    or (using ISO weekly calendar)   yyyy-Www-dThh:mm:ss[.xxx]\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "    An example of a correct format is:  2000-01-01T12:00:00\n");
	}
	return (error);
}

/*! Changes the 4 GMT default pad values to given isotropic pad */
void GMT_set_pad (struct GMT_CTRL *GMT, unsigned int pad) {
	GMT->current.io.pad[XLO] = GMT->current.io.pad[XHI] = GMT->current.io.pad[YLO] = GMT->current.io.pad[YHI] = pad;
}

/*! . */
int GMT_init_fonts (struct GMT_CTRL *GMT) {
	unsigned int i = 0, n_GMT_fonts;
	size_t n_alloc = 0;
	char buf[GMT_BUFSIZ] = {""}, fullname[GMT_BUFSIZ] = {""};
	FILE *in = NULL;

	/* Loads the available fonts for this installation */

	/* First the standard 35 PostScript fonts from Adobe */

	GMT_getsharepath (GMT, "pslib", "PS_font_info", ".d", fullname, R_OK);
	if ((in = fopen (fullname, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot open %s\n", fullname);
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}

	GMT_set_meminc (GMT, GMT_SMALL_CHUNK);	/* Only allocate a small amount */
	while (fgets (buf, GMT_BUFSIZ, in)) {
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
		if (i == n_alloc) GMT->session.font = GMT_malloc (GMT, GMT->session.font, i, &n_alloc, struct GMT_FONTSPEC);
		if (sscanf (buf, "%s %lf %*d", fullname, &GMT->session.font[i].height) != 2) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Trouble decoding font info for font %d\n", i);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		GMT->session.font[i++].name = strdup (fullname);
	}
	fclose (in);
	GMT->session.n_fonts = n_GMT_fonts = i;

	/* Then any custom fonts */

	if (GMT_getsharepath (GMT, "pslib", "CUSTOM_font_info", ".d", fullname, R_OK)) {	/* Decode Custom font file */
		if ((in = fopen (fullname, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot open %s\n", fullname);
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}

		while (fgets (buf, GMT_BUFSIZ, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			if (i == n_alloc) GMT->session.font = GMT_malloc (GMT, GMT->session.font, i, &n_alloc, struct GMT_FONTSPEC);
			if (sscanf (buf, "%s %lf %*d", fullname, &GMT->session.font[i].height) != 2) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Trouble decoding custom font info for font %d\n", i - n_GMT_fonts);
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			GMT->session.font[i++].name = strdup (fullname);
		}
		fclose (in);
		GMT->session.n_fonts = i;
	}
	n_alloc = i;
	GMT->session.font = GMT_malloc (GMT, GMT->session.font, 0, &n_alloc, struct GMT_FONTSPEC);
	GMT_reset_meminc (GMT);
	return (GMT_NOERROR);
}

/*! . */
struct GMT_CTRL *New_GMT_Ctrl (struct GMTAPI_CTRL *API, char *session, unsigned int pad) {	/* Allocate and initialize a new common control structure */
	int i;
	char path[PATH_MAX+1];
	char *unit_name[4] = {"cm", "inch", "m", "point"};
	double u2u[4][4] = {	/* Local initialization of unit conversion factors */
		{   1.00,    1.0/2.54,    0.01,         72.0/2.54 },
		{   2.54,    1.0,         0.0254,       72.0 },
		{ 100.00,    100.0/2.54,  1.0,          72.0/0.0254 },
		{ 2.54/72.0, 1.0/72.0,    0.0254/72.0,  1.0 }
	};
	struct GMT_PROJ4 GMT_proj4[GMT_N_PROJ4] = {
		{ "aea"      , GMT_ALBERS },
		{ "aeqd"     , GMT_AZ_EQDIST },
		{ "cyl_stere", GMT_CYL_STEREO },
		{ "cass"     , GMT_CASSINI },
		{ "cea"      , GMT_CYL_EQ },
		{ "eck4"     , GMT_ECKERT4 },
		{ "eck6"     , GMT_ECKERT6 },
		{ "eqc"      , GMT_CYL_EQDIST },
		{ "eqdc"     , GMT_ECONIC },
		{ "gnom"     , GMT_GNOMONIC },
		{ "hammer"   , GMT_HAMMER },
		{ "laea"     , GMT_LAMB_AZ_EQ },
		{ "lcc"      , GMT_LAMBERT },
		{ "merc"     , GMT_MERCATOR },
		{ "mill"     , GMT_MILLER },
		{ "moll"     , GMT_MOLLWEIDE },
		{ "nsper"    , GMT_GENPER },
		{ "omerc"    , GMT_OBLIQUE_MERC },
		{ "omercp"   , GMT_OBLIQUE_MERC_POLE },
		{ "ortho"    , GMT_ORTHO },
		{ "polar"    , GMT_POLAR },
		{ "poly"     , GMT_POLYCONIC },
		{ "robin"    , GMT_ROBINSON },
		{ "sinu"     , GMT_SINUSOIDAL },
		{ "stere"    , GMT_STEREO },
		{ "tmerc"    , GMT_TM },
		{ "utm"      , GMT_UTM },
		{ "vandg"    , GMT_VANGRINTEN },
		{ "wintri"   , GMT_WINKEL },
		{ "xy"       , GMT_LINEAR },
		{ "z"        , GMT_ZAXIS }
	};
	struct GMT_CTRL *GMT = NULL;
	struct ELLIPSOID ref_ellipsoid[GMT_N_ELLIPSOIDS] = {   /* This constant is created by GNUmakefile - do not edit */
	#include "gmt_ellipsoids.h"	/* This include file is created by GNUmakefile - do not edit */
	};
	struct DATUM datum[GMT_N_DATUMS] = {     /* This constant is created by GNUmakefile - do not edit */
	#include "gmt_datums.h"		/* This include file is created by GNUmakefile - do not edit */
	};
	GMT_UNUSED(session);

	/* Alloc using calloc since GMT_memory may use resources not yet initialized */
	GMT = calloc (1U, sizeof (struct GMT_CTRL));
	GMT_memcpy (GMT->current.setting.ref_ellipsoid, ref_ellipsoid, 1, ref_ellipsoid);
	GMT_memcpy (GMT->current.setting.proj_datum, datum, 1, datum);

	/* Assign the daddy */
	GMT->parent = API;
	
	/* Assign the three std* pointers */

	GMT->session.std[GMT_IN]  = stdin;
	GMT->session.std[GMT_OUT] = stdout;
	GMT->session.std[GMT_ERR] = stderr;

	/* Set default verbosity level */
	GMT->current.setting.verbose = GMT_MSG_COMPAT;

#ifdef MEMDEBUG
	GMT_memtrack_init (GMT);	/* Helps us determine memory leaks */
	GMT->session.min_meminc = GMT_MIN_MEMINC;
	GMT->session.max_meminc = GMT_MAX_MEMINC;
#endif

	/* We don't know the module or library names yet */
	GMT->init.module_name = GMT->init.module_lib = NULL;

	/* Set runtime bindir */
	GMT_runtime_bindir (path, session);
	GMT->init.runtime_bindir = strdup (path);

	/* Set runtime libdir */
#if defined(__CYGWIN__)
	/* Since no dladdr under Cygwin we must assume lib dir parallels bin dir */
	if (strlen (path) > 4 && !strncmp (&path[strlen(path)-4], "/bin", 4U))
		strncpy (&path[strlen(path)-3], "lib", 3U);
#else
	GMT_runtime_libdir (path);
#endif
	GMT->init.runtime_libdir = strdup (path);

	GMT_set_env (GMT);	/* Get GMT_SHAREDIR and other environment path parameters */

	GMT_init_fonts (GMT);	/* Load in available font names */

	/* Initialize values whose defaults are not necessarily 0/false/NULL */

	/* MAP settings */

	GMT_init_distaz (GMT, GMT_MAP_DIST_UNIT, GMT_GREATCIRCLE, GMT_MAP_DIST);	/* Default spherical distance calculations in m */

	GMT->current.map.n_lon_nodes = 360;
	GMT->current.map.n_lat_nodes = 180;
	GMT->current.map.frame.check_side = false;
	GMT->current.map.frame.horizontal = 0;
	GMT->current.map.dlon = (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) / GMT->current.map.n_lon_nodes;
	GMT->current.map.dlat = (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) / GMT->current.map.n_lat_nodes;

	/* PLOT settings */

	GMT->current.plot.mode_3D = 3;	/* Draw both fore and/or back 3-D box lines [1 + 2] */

	/* PROJ settings */

	GMT->current.proj.projection = GMT_NO_PROJ;
	/* We need some defaults here for the cases where we do not actually set these with GMT_map_setup */
	GMT->current.proj.fwd_x = GMT->current.proj.fwd_y = GMT->current.proj.fwd_z = &GMT_translin;
	GMT->current.proj.inv_x = GMT->current.proj.inv_y = GMT->current.proj.inv_z = &GMT_itranslin;
	/* z_level will be updated in GMT_init_three_D, but if it doesn't, it does not matter,
	 * because by default, z_scale = 0.0 */
	GMT->current.proj.z_level = DBL_MAX;
	GMT->current.proj.xyz_pos[GMT_X] = GMT->current.proj.xyz_pos[GMT_Y] = GMT->current.proj.xyz_pos[GMT_Z] = true;
	GMT->current.proj.z_project.view_azimuth = 180.0;
	GMT->current.proj.z_project.view_elevation = 90.0;
	GMT->current.proj.z_project.plane = -1;	/* Initialize no perspective projection */
	GMT->current.proj.z_project.level = 0.0;
	for (i = 0; i < 4; i++) GMT->current.proj.edge[i] = true;
	GMT_grdio_init (GMT);
	GMT_set_pad (GMT, pad); /* Sets default number of rows/cols for boundary padding in this session */
	GMT->current.proj.f_horizon = 90.0;
	GMT->current.proj.proj4 = GMT_memory (GMT, NULL, GMT_N_PROJ4, struct GMT_PROJ4);
	for (i = 0; i < GMT_N_PROJ4; i++) {	/* Load up proj4 structure once and for all */
		GMT->current.proj.proj4[i].name = strdup (GMT_proj4[i].name);
		GMT->current.proj.proj4[i].id = GMT_proj4[i].id;
	}
	/* TIME_SYSTEM settings */
	strcpy (GMT->current.setting.time_system.epoch, "2000-01-01T12:00:00");
	GMT->current.setting.time_system.unit = 'd';

	/* INIT settings */

	GMT_memcpy (GMT->session.u2u, u2u, 1, u2u);
	for (i = 0; i < 4; i++) strncpy (GMT->session.unit_name[i], unit_name[i], 8U);
	GMT_make_fnan (GMT->session.f_NaN);
	GMT_make_dnan (GMT->session.d_NaN);
	for (i = 0; i < 3; i++) GMT->session.no_rgb[i] = -1.0;

	return (GMT);
}

/*! Gets the rata die of today */
void gmt_set_today (struct GMT_CTRL *GMT) {
	time_t right_now = time (NULL);			/* Unix time right now */
	struct tm *moment = gmtime (&right_now);	/* Convert time to a TM structure */
	/* Calculate rata die from yy, mm, and dd */
	/* tm_mon is 0-11, so add 1 for 1-12 range, tm_year is years since 1900, so add 1900, but tm_mday is 1-31 so use as is */
	GMT->current.time.today_rata_die = GMT_rd_from_gymd (GMT, 1900 + moment->tm_year, moment->tm_mon + 1, moment->tm_mday);
}

/*! . */
struct GMT_CTRL *GMT_begin (struct GMTAPI_CTRL *API, char *session, unsigned int pad) {
	/* GMT_begin is called once by GMT_Create_Session and does basic
	 * one-time initialization of GMT before the GMT modules take over.
	 * It will load in the gmt.conf settings from the share dir and
	 * reset them with the user's gmt.conf settings (if any).
	 * It then does final processing of defaults so that all internal
	 * GMT parameters are properly initialized and ready to go. This
	 * means it is possible to write a functioning GMT application that
	 * does not require the use of any GMT modules.  However,
	 * most GMT applications will call various GMT modules and these
	 * may need to process additional --PAR=value arguments. This will
	 * require renewed processing of defaults and takes place in GMT_begin_module
	 * which is called at the start of all GMT modules.  This basically
	 * performs a save/restore operation so that when the GMT module
	 * returns the GMT structure is restored to its original values.
	 *
	 * Note: We do not call GMT_exit here since API is not given and
	 * API->do_not_exit have not been modified by external API yet.
	 */

	char path[GMT_LEN256] = {""};
	struct GMT_CTRL *GMT = NULL;

#ifdef __FreeBSD__
#ifdef _i386_
	/* allow divide by zero -- Inf */
	fpsetmask (fpgetmask () & ~(FP_X_DZ | FP_X_INV));
#endif
#endif

#ifdef WIN32
	/* Set all I/O to binary mode */
	if ( _setmode(_fileno(stdin), _O_BINARY) == -1 ) {
		GMT_Message (API, GMT_TIME_NONE, "Could not set binary mode for stdin.\n");
		return NULL;
	}
	if ( _setmode(_fileno(stdout), _O_BINARY) == -1 ) {
		GMT_Message (API, GMT_TIME_NONE, "Could not set binary mode for stdout.\n");
		return NULL;
	}
	if ( _setmode(_fileno(stderr), _O_BINARY) == -1 ) {
		GMT_Message (API, GMT_TIME_NONE, "Could not set binary mode for stderr.\n");
		return NULL;
	}
	if ( _set_fmode(_O_BINARY) != 0 ) {
		GMT_Message (API, GMT_TIME_NONE, "Could not set binary mode for file I/O.\n");
		return NULL;
	}
#endif

	GMT = New_GMT_Ctrl (API, session, pad);	/* Allocate and initialize a new common control structure */
	API->GMT = GMT;

	GMT->PSL = New_PSL_Ctrl ("GMT5");		/* Allocate a PSL control structure */
	if (!GMT->PSL) {
		GMT_Message (API, GMT_TIME_NONE, "Error: Could not initialize PSL - Aborting.\n");
		Free_GMT_Ctrl (GMT);	/* Deallocate control structure */
		return NULL;
	}
	GMT->PSL->init.unit = PSL_INCH;					/* We use inches internally in PSL */
	PSL_beginsession (GMT->PSL, 0, GMT->session.SHAREDIR, GMT->session.USERDIR);	/* Initializes the session and sets a few defaults */
	/* Reset session defaults to the chosen GMT settings; these are fixed for the entire PSL session */
	PSL_setdefaults (GMT->PSL, GMT->current.setting.ps_magnify, GMT->current.setting.ps_page_rgb, GMT->current.setting.ps_encoding.name);

	GMT_io_init (GMT);		/* Init the table i/o structure before parsing GMT defaults */

	gmt_init_unit_conversion (GMT);	/* Set conversion factors from various units to meters */

	GMT_hash_init (GMT, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);	/* Initialize hash table for GMT defaults */

	/* Set up hash table for colornames (used to convert <colorname> to <r/g/b>) */

	GMT_hash_init (GMT, GMT->session.rgb_hashnode, GMT_color_name, GMT_N_COLOR_NAMES, GMT_N_COLOR_NAMES);

	/* Initialize the standard GMT system default settings from the system file */

	sprintf (path, "%s/conf/gmt.conf", GMT->session.SHAREDIR);
	if (access (path, R_OK)) {
		/* Not found in SHAREDIR, try USERDIR instead */
		if (GMT_getuserpath (GMT, "conf/gmt.conf", path) == NULL) {
			GMT_Message (API, GMT_TIME_NONE, "Error: Could not find system defaults file %s - Aborting.\n", path);
			Free_GMT_Ctrl (GMT);	/* Deallocate control structure */
			return NULL;
		}
	}
	GMT_loaddefaults (GMT, path);	/* Load GMT system default settings [and PSL settings if selected] */
	GMT_getdefaults (GMT, NULL);	/* Override using local GMT default settings (if any) [and PSL if selected] */

	/* There is no longer a -m option in GMT 5 so multi segments are now always true.
	   However, in GMT_COMPAT mode the -mi and -mo options WILL turn off multi in the other direction. */
	GMT_set_segmentheader (GMT, GMT_IN, true);
	GMT_set_segmentheader (GMT, GMT_OUT, false);	/* Will be turned true when either of two situation arises: */
	/* 1. We read a multisegment header
	   2. The -g option is set which will create gaps and thus multiple segments
	 */

	/* Initialize the output and plot format machinery for ddd:mm:ss[.xxx] strings from the default format strings.
	 * While this is also done in the default parameter loop it is possible that when a decimal plain format has been selected
	 * the format_float_out string has not yet been processed.  We clear that up by processing again here. */

	gmt_geo_C_format (GMT);
	gmt_plot_C_format (GMT);

	/* Set default for -n parameters */
	GMT->common.n.antialias = true; GMT->common.n.interpolant = BCR_BICUBIC; GMT->common.n.threshold = 0.5;

	gmt_get_history (GMT);	/* Process and store command shorthands passed to the application */

	if (GMT->current.setting.io_gridfile_shorthand) gmt_setshorthand (GMT);	/* Load the short hand mechanism from gmt.io */

	GMT_fft_initialization (GMT);	/* Determine which FFT algos are available and set pointers */

	gmt_set_today (GMT);	/* Determine today's rata die value */

	return (GMT);
}

/*! . */
bool GMT_check_filearg (struct GMT_CTRL *GMT, char option, char *file, unsigned int direction, unsigned int family)
{	/* Return true if a file arg was given and, if direction is GMT_IN, check that the file
	 * exists and is readable. Otherwise we return false. */
	unsigned int k = 0;
	bool not_url = true;
	char message[GMT_LEN16] = {""};
	if (option == GMT_OPT_INFILE)
		sprintf (message, "for input file");
	else if (option == GMT_OPT_OUTFILE)
		sprintf (message, "for output file");
	else
		sprintf (message, "option -%c", option);

	if (!file || file[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error %s: No filename provided\n", message);
		return false;	/* No file given */
	}
	if (direction == GMT_OUT) return true;		/* Cannot check any further */
	if (file[0] == '=') k = 1;	/* Gave a list of files with =<filelist> mechanism in x2sys */
	if (family == GMT_IS_GRID || family == GMT_IS_IMAGE)	/* Only grid and images can be URLs so far */
		not_url = !GMT_check_url_name (&file[k]);
	if (GMT_access (GMT, &file[k], F_OK) && not_url) {	/* Cannot find the file anywhere GMT looks */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error %s: No such file (%s)\n", message, &file[k]);
		return false;	/* Could not find this file */
	}
	if (GMT_access (GMT, &file[k], R_OK) && not_url) {	/* Cannot read this file (permissions?) */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error %s: Cannot read file (%s) - check permissions\n", message, &file[k]);
		return false;	/* Could not find this file */
	}
	return true;	/* Seems OK */
}

#ifdef SET_IO_MODE

/* Under non-Unix operating systems running on the PC, such as
 * Windows, files are opened in either TEXT or BINARY mode.
 * This difference does not exist under UNIX, but is important
 * on the PC.  Specifically, it causes a problem when a program
 * that writes/reads standard i/o wants to use binary data.
 * In those situations we must change the default (TEXT) mode of
 * the file handle to BINARY via a call to "setmode".
 *
 * This can also be done under Win32 with the Microsoft VC++
 * compiler which supports ANSI-C (P. Wessel).  This may be true
 * of other Win32 compilers as well.
 */

/*! Changes the stream to deal with BINARY rather than TEXT data */
void GMT_setmode (struct GMT_CTRL *GMT, int direction) {

	FILE *fp = NULL;
	static const char *IO_direction[2] = {"Input", "Output"};

	if (GMT->common.b.active[direction]) {	/* User wants native binary i/o */

		fp = (direction == 0) ? GMT->session.std[GMT_IN] : GMT->session.std[GMT_OUT];
		fflush (fp);	/* Should be untouched but anyway... */
#ifdef WIN32
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Set binary mode for %s\n", IO_direction[direction]);
		setmode (fileno (fp), _O_BINARY);
#else
		_fsetmode (fp, "b");
#endif
	}
}

#endif	/* SET_IO_MODE */

/*! . */
int GMT_message (struct GMT_CTRL *GMT, char *format, ...) {
	char line[GMT_BUFSIZ];
	va_list args;
	va_start (args, format);
	vsnprintf (line, GMT_BUFSIZ, format, args);
	GMT->parent->print_func (GMT->session.std[GMT_ERR], line);
	va_end (args);
	return (0);
}

/*! . */
int GMT_report_func (struct GMT_CTRL *GMT, unsigned int level, const char *source_line, const char *format, ...) {
	char message[GMT_BUFSIZ];
	size_t source_info_len;
	va_list args;
	if (level > GMT->current.setting.verbose)
		return 0;
	snprintf (message, GMT_BUFSIZ, "%s (%s): ",
			GMT->init.module_name, source_line);
	source_info_len = strlen (message);
	va_start (args, format);
	/* append format to the message: */
	vsnprintf (message + source_info_len, GMT_BUFSIZ - source_info_len, format, args);
	va_end (args);
	GMT->parent->print_func (GMT->session.std[GMT_ERR], message);
	return 1;
}

/*! Return the number of CPU cores */
int GMT_get_num_processors() {
	static int n_cpu = 0;

	if (n_cpu > 0)
		/* we already know the answer. do not query again. */
		return n_cpu;

#if defined WIN32
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo ( &sysinfo );
		n_cpu = sysinfo.dwNumberOfProcessors;
	}
#elif defined HAVE_SC_NPROCESSORS_ONLN
	n_cpu = (int)sysconf (_SC_NPROCESSORS_ONLN);
#elif defined HAVE_SC_NPROC_ONLN
	n_cpu = (int)sysconf (_SC_NPROC_ONLN);
#elif defined HAVE_SYSCTL_HW_NCPU
	{
		size_t size = sizeof(n_cpu);
		int mib[] = { CTL_HW, HW_NCPU };
		sysctl(mib, 2, &n_cpu, &size, NULL, 0);
	}
#endif
	if (n_cpu < 1)
		n_cpu = 1; /* fallback */
	return n_cpu;
}
