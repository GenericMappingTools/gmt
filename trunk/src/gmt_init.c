/*--------------------------------------------------------------------
 *	$Id: gmt_init.c,v 1.549 2011-06-21 23:29:39 remko Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Date:	1-JAN-2010
 * Version:	5
 *
 *
 * The PUBLIC functions are:
 *
 *	GMT_explain_options		Prints explanations for the common options
 *	GMT_parse_common_options	Interprets common options, such as -B, -R, --
 *	GMT_getdefaults			Initializes the GMT global parameters
 *	GMT_putdefaults			Dumps the GMT global parameters
 *	GMT_hash_init			Initializes a hash
 *	GMT_hash_lookup			Key - id lookup using hashing
 *	GMT_hash			Key - id lookup using hashing
 *	GMT_begin			Gets history and init parameters
 *	GMT_end				Cleans up and returns
 *	gmt_history			Read and update the .gmtcommands file
 *	GMT_putcolor			Encode color argument into textstring
 *	GMT_putrgb			Encode color argument into r/g/b textstring
 *	GMT_puthsv			Encode color argument into h-s-v textstring
 *	GMT_putcmyk			Encode color argument into c/m/y/k textstring
 *
 * The INTERNAL functions are:
 *
 *	GMT_loaddefaults		Reads the GMT global parameters from gmt.conf
 *	GMT_savedefaults		Writes the GMT global parameters to gmt.conf
 *	GMT_parse_?_option		Decode the one of the common options
 *	GMT_setparameter		Sets a default value given keyword,value-pair
 *	gmt_setshorthand		Reads and initializes the suffix shorthands
 *	GMT_get_ellipsoid		Returns ellipsoid id based on name
 *	gmt_scanf_epoch			Get user time origin from user epoch string
 *	GMT_init_time_system_structure  Does what it says
 */

#include "pslib.h"
#include "gmt.h"
#include "gmt_internals.h"

EXTERN_MSC GMT_LONG gmt_geo_C_format (struct GMT_CTRL *C);
EXTERN_MSC void GMT_grdio_init (struct GMT_CTRL *C);	/* Defined in gmt_customio.c and only used here */

#ifdef DEBUG
/* This is used to help is find memory leaks */
struct MEMORY_TRACKER *GMT_mem_keeper;
#endif
/*--------------------------------------------------------------------*/
/* Load private fixed array parameters from include files */
/*--------------------------------------------------------------------*/

#include "gmt_keycases.h"				/* Get all the default case values */
static char *GMT_keywords[GMT_N_KEYS] = {		/* Names of all parameters in gmt.conf */
#include "gmt_keywords.h"
};

static char *GMT_unique_option[GMT_N_UNIQUE] = {	/* The common GMT command-line options */
#include "gmt_unique.h"
};

static char *GMT_media_name[GMT_N_MEDIA] = {		/* Names of all recognized paper formats */
#include "gmt_media_name.h"
};
static struct GMT_MEDIA GMT_media[GMT_N_MEDIA] = {	/* Sizes in points of all paper formats */
#include "gmt_media_size.h"
};

#define USER_MEDIA_OFFSET 1000

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

#define GMT_def(case_val) * C->session.u2u[GMT_INCH][GMT_unit_lookup(C, C->current.setting.given_unit[case_val], C->current.setting.proj_length_unit)], C->current.setting.given_unit[case_val]

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void GMT_explain_options (struct GMT_CTRL *C, char *options)
{
	/* The function print to stderr a short explanation for each of the options listed by
	 * the variable <options>.  Only the common parameter options are covered
	 */

	char u, *GMT_choice[2] = {"OFF", "ON"};
	double s;
	GMT_LONG k;

	if (!options) return;
	
	u = C->session.unit_name[C->current.setting.proj_length_unit][0];
	s = C->session.u2u[GMT_INCH][C->current.setting.proj_length_unit];	/* Convert from internal inch to users unit */

	for (k = 0; k < strlen (options); k++) {

		switch (options[k]) {

		case 'B':	/* Tickmark option */

			GMT_message (C, "\t-B Specify basemap frame info.  <tickinfo> is a textstring made up of one or\n");
			GMT_message (C, "\t   more substrings of the form [a|f|g]<stride>[+-<phase>][<unit>], where the (optional) a");
			GMT_message (C, "\t   indicates annotation and major tick interval, f minor tick interval and g grid interval");
			GMT_message (C, "\t   <stride> is the spacing between ticks or annotations, the (optional)\n");
			GMT_message (C, "\t   <phase> specifies phase-shifted annotations by that amount, and the (optional)\n");
			GMT_message (C, "\t   <unit> specifies the <stride> unit [Default is unit implied in -R]. There can be\n");
			GMT_message (C, "\t   no spaces between the substrings - just append to make one very long string.\n");
			GMT_message (C, "\t   -B[p] means (p)rimary annotations; use -Bs to specify (s)econdary annotations.\n");
			GMT_message (C, "\t   For custom labels or intervals, let <tickinfo> be c<intfile>,; see man page for details.\n");
			GMT_message (C, "\t   The optional <unit> modifies the <stride> value accordingly.  For maps, you may use\n");
			GMT_message (C, "\t     d: arc degree [Default].\n");
			GMT_message (C, "\t     m: arc minutes.\n");
			GMT_message (C, "\t     s: arc seconds.\n");
			GMT_message (C, "\t   For time axes, several units are recognized:\n");
			GMT_message (C, "\t     Y: year - plot using all 4 digits.\n");
			GMT_message (C, "\t     y: year - plot only last 2 digits.\n");
			GMT_message (C, "\t     O: month - format annotation according to format_date_map.\n");
			GMT_message (C, "\t     o: month - plot as 2-digit integer (1-12).\n");
			GMT_message (C, "\t     U: ISO week - format annotation according to format_date_map.\n");
			GMT_message (C, "\t     u: ISO week - plot as 2-digit integer (1-53).\n");
			GMT_message (C, "\t     r: Gregorian week - 7-day stride from chosen start of week (%s).\n", GMT_weekdays[C->current.setting.time_week_start]);
			GMT_message (C, "\t     K: ISO weekday - format annotation according to format_date_map.\n");
			GMT_message (C, "\t     k: weekday - plot name of weekdays in selected language [%s].\n", C->current.setting.time_language);
			GMT_message (C, "\t     D: day  - format annotation according to format_date_map, which also determines whether\n");
			GMT_message (C, "\t               we should plot day of month (1-31) or day of year (1-366).\n");
			GMT_message (C, "\t     d: day - plot as 2- (day of month) or 3- (day of year) integer.\n");
			GMT_message (C, "\t     R: Same as d but annotates from start of Gregorian week.\n");
			GMT_message (C, "\t     H: hour - format annotation according to format_clock_map.\n");
			GMT_message (C, "\t     h: hour - plot as 2-digit integer (0-23).\n");
			GMT_message (C, "\t     M: minute - format annotation according to format_clock_map.\n");
			GMT_message (C, "\t     m: minute - plot as 2-digit integer (0-59).\n");
			GMT_message (C, "\t     S: second - format annotation according to format_clock_map.\n");
			GMT_message (C, "\t     s: second - plot as 2-digit integer (0-59; 60-61 if leap seconds are enabled).\n");
			GMT_message (C, "\t   Specify an axis label by surrounding it with colons (e.g., :\"my x label\":).\n");
			GMT_message (C, "\t   To prepend a prefix to each annotation (e.g., $ 10, $ 20 ...) add a prefix that begins\n");
			GMT_message (C, "\t     with the equal-sign (=); the rest is used as annotation prefix (e.g. :=\'$\':). If the prefix has\n");
			GMT_message (C, "\t     a leading hyphen (-) there will be no space between prefix and annotation (e.g., :=-\'$\':).\n");
			GMT_message (C, "\t   To append a unit to each annotation (e.g., 5 km, 10 km ...) add a label that begins\n");
			GMT_message (C, "\t     with a comma; the rest is used as unit annotation (e.g. :\",km\":). If the unit has\n");
			GMT_message (C, "\t     a leading hyphen (-) there will be no space between unit and annotation (e.g., :,-%%:).\n");
			GMT_message (C, "\t   For separate x and y [and z if -Jz is used] tickinfo, separate the strings with slashes [/].\n");
			GMT_message (C, "\t   Specify an plot title by adding a label whose first character is a period; the rest\n");
			GMT_message (C, "\t     of the label is used as the title (e.g. :\".My Plot Title\":).\n");
			GMT_message (C, "\t   Append any combination of W, E, S, N, Z to annotate those axes only [Default is WESNZ (all)].\n");
			GMT_message (C, "\t     Use lower case w, e, s, n, z to draw & tick but not to annotate those axes.\n");
			GMT_message (C, "\t     Z+ will also draw a 3-D box.\n");
			GMT_message (C, "\t   Append +g<fill> to pain the inside of the map region before plotting [no fill].\n");
			GMT_message (C, "\t   Log10 axis: Append l to annotate log10 (x) or p for 10^(log10(x)) [Default annotates x].\n");
			GMT_message (C, "\t   Power axis: append p to annotate x at equidistant pow increments [Default is nonlinear].\n");
			GMT_message (C, "\t   See psbasemap man pages for more details and examples of all settings.\n");
			break;

		case 'b':	/* Condensed tickmark option */

			GMT_message (C, "\t-B Basemap boundary annotation attributes.\n");
			GMT_message (C, "\t   Specify -B[p|s]<xinfo>[/<yinfo>[/<zinfo>]][.:\"title\":][wesnzWESNZ+][+g<fill>]\n");
			GMT_message (C, "\t   <?info> is [<type>]<stride>[<unit>][l|p][:\"label\":][:,[-]\"unit\":]\n");
			GMT_message (C, "\t   See psbasemap man page for more details and examples of all settings.\n");
			break;

		case 'J':	/* Map projection option */

			GMT_message (C, "\t-J Select the map proJection. The projection type is identified by a 1- or\n");
			GMT_message (C, "\t   2-character ID (e.g. 'm' or 'kf') or by an abbreviation followed by a slash\n");
			GMT_message (C, "\t   (e.g. 'cyl_stere/'). When using a lower-case ID <scale> can be given either as 1:<xxxx>\n");
			GMT_message (C, "\t   or in %s/degree along the standard parallel. Alternatively, when the projection ID is\n", C->session.unit_name[C->current.setting.proj_length_unit]);
			GMT_message (C, "\t   Capitalized, <scale>|<width> denotes the width of the plot in %s\n", C->session.unit_name[C->current.setting.proj_length_unit]);
			GMT_message (C, "\t   Append h for map height, + for max map dimension, and - for min map dimension.\n");
			GMT_message (C, "\t   When the central meridian (lon0) is optional and omitted, the center of the\n");
			GMT_message (C, "\t   longitude range specified by -R is used. The default standard parallel is the equator\n");
			GMT_message (C, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is given.\n");

			GMT_message (C, "\t   -Ja|A<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Lambert Azimuthal Equal Area)\n");
			GMT_message (C, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (C, "\t     <horizon> is max distance from center of the projection (<= 180, default 90).\n");
			GMT_message (C, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is the distance\n");
			GMT_message (C, "\t     in %s to the oblique parallel <lat>.\n", C->session.unit_name[C->current.setting.proj_length_unit]);

			GMT_message (C, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Albers Equal-Area Conic)\n");
			GMT_message (C, "\t     Give origin, 2 standard parallels, and true scale\n");

			GMT_message (C, "\t   -Jc|C<lon0>/<lat0><scale>|<width> (Cassini)\n\t     Give central point and scale\n");

			GMT_message (C, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<scale>|<width> (Cylindrical Stereographic)\n");
			GMT_message (C, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			GMT_message (C, "\t     <lat0> = 66.159467 (Miller's modified Gall), 55 (Kamenetskiy's First),\n");
			GMT_message (C, "\t     45 (Gall Stereographic), 30 (Bolshoi Sovietskii Atlas Mira), 0 (Braun)\n");

			GMT_message (C, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Equidistant Conic)\n");
			GMT_message (C, "\t     Give origin, 2 standard parallels, and true scale\n");

			GMT_message (C, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Azimuthal Equidistant)\n");
			GMT_message (C, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (C, "\t     <horizon> is max distance from center of the projection (<= 180, default 180).\n");
			GMT_message (C, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is the distance\n");
			GMT_message (C, "\t     in %s to the oblique parallel <lat>. \n", C->session.unit_name[C->current.setting.proj_length_unit]);

			GMT_message (C, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Gnomonic)\n");
			GMT_message (C, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (C, "\t     <horizon> is max distance from center of the projection (< 90, default 60).\n");
			GMT_message (C, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			GMT_message (C, "\t     in %s to the oblique parallel <lat>. \n", C->session.unit_name[C->current.setting.proj_length_unit]);

			GMT_message (C, "\t   -Jg|G<lon0>/<lat0>/<scale>|<width> (Orthographic)\n");
			GMT_message (C, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (C, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			GMT_message (C, "\t     in %s to the oblique parallel <lat>. \n", C->session.unit_name[C->current.setting.proj_length_unit]);

			GMT_message (C, "\t   -Jg|G<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale>|<width> (General Perspective)\n");
			GMT_message (C, "\t     <lon0>/<lat0> is the center of the projection.\n");
			GMT_message (C, "\t     <altitude> is the height (in km) of the viewpoint above local sea level\n");
			GMT_message (C, "\t        - if <altitude> less than 10 then it is the distance \n");
			GMT_message (C, "\t        from center of earth to viewpoint in earth radii\n");
			GMT_message (C, "\t        - if <altitude> has a suffix of 'r' then it is the radius \n");
			GMT_message (C, "\t        from the center of earth in kilometers\n");
			GMT_message (C, "\t     <azimuth> is azimuth east of North of view\n");
			GMT_message (C, "\t     <tilt> is the upward tilt of the plane of projection\n");
			GMT_message (C, "\t       if <tilt> < 0 then viewpoint is centered on the horizon\n");
			GMT_message (C, "\t     <twist> is the CW twist of the viewpoint in degree\n");
			GMT_message (C, "\t     <width> is width of the viewpoint in degree\n");
			GMT_message (C, "\t     <height> is the height of the viewpoint in degrees\n");
			GMT_message (C, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			GMT_message (C, "\t     in %s to the oblique parallel <lat>. \n", C->session.unit_name[C->current.setting.proj_length_unit]);

			GMT_message (C, "\t   -Jh|H[<lon0>/]<scale>|<width> (Hammer-Aitoff)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Ji|I[<lon0>/]<scale>|<width> (Sinusoidal)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Jj|J[<lon0>/]<scale>|<width> (Miller)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Jkf|Kf[<lon0>/]<scale>|<width> (Eckert IV)\n\t     Give central meridian (opt) and scale\n");
			GMT_message (C, "\t   -Jk|K[s][<lon0>/]<scale>|<width> (Eckert VI)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Lambert Conformal Conic)\n");
			GMT_message (C, "\t     Give origin, 2 standard parallels, and true scale\n");

			GMT_message (C, "\t   -Jm|M[<lon0>/[<lat0>/]]<scale>|<width> (Mercator).\n");
			GMT_message (C, "\t     Give central meridian (opt), true scale parallel (opt), and scale\n");

			GMT_message (C, "\t   -Jn|N[<lon0>/]<scale>|<width> (Robinson projection)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Jo|O<parameters> (Oblique Mercator).  Specify one of three definitions:\n");
			GMT_message (C, "\t     -Jo|O[a]<lon0>/<lat0>/<azimuth>/<scale>|<width>\n");
			GMT_message (C, "\t       Give origin, azimuth of oblique equator, and scale at oblique equator\n");
			GMT_message (C, "\t     -Jo|O[b]<lon0>/<lat0>/<lon1>/<lat1>/<scale>|<width>\n");
			GMT_message (C, "\t       Give origin, second point on oblique equator, and scale at oblique equator\n");
			GMT_message (C, "\t     -Jo|Oc<lon0>/<lat0>/<lonp>/<latp>/<scale>|<width>\n");
			GMT_message (C, "\t       Give origin, pole of projection, and scale at oblique equator\n");
			GMT_message (C, "\t       Specify region in oblique degrees OR use -R<>r\n");

			GMT_message (C, "\t   -Jp|P[a]<scale>|<width>[/<base>][r|z] (Polar (theta,radius))\n");
			GMT_message (C, "\t     Linear scaling for polar coordinates.\n");
			GMT_message (C, "\t     Optionally append 'a' to -Jp or -JP to use azimuths (CW from North) instead of directions (CCW from East) [default].\n");
			GMT_message (C, "\t     Give scale in %s/units, and append theta value for angular offset (base) [0]\n", C->session.unit_name[C->current.setting.proj_length_unit]);
			GMT_message (C, "\t     Append r to reverse radial direction (s/n must be in 0-90 range) or z to annotate depths rather than radius [Default]\n");

			GMT_message (C, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scale>|<width> ((American) Polyconic)\n");
			GMT_message (C, "\t     Give central meridian (opt), reference parallel (opt, default = equator), and scale\n");

			GMT_message (C, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scale>|<width> (Equidistant Cylindrical)\n");
			GMT_message (C, "\t     Give central meridian (opt), standard parallel (opt), and scale\n");
			GMT_message (C, "\t     <lat0> = 61.7 (Min. linear distortion), 50.5 (R. Miller equirectangular),\n");
			GMT_message (C, "\t     45 (Gall isographic), 43.5 (Min. continental distortion), 42 (Grafarend & Niermann),\n");
			GMT_message (C, "\t     37.5 (Min. overall distortion), 0 (Plate Carree, default)\n");

			GMT_message (C, "\t   -Jr|R[<lon0>/]<scale>|<width> (Winkel Tripel)\n\t     Give central meridian and scale\n");

			GMT_message (C, "\t   -Js|S<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Stereographic)\n");
			GMT_message (C, "\t     <lon0>/<lat0> is the center or the projection.\n");
			GMT_message (C, "\t     <horizon> is max distance from center of the projection (< 180, default 90).\n");
			GMT_message (C, "\t     <scale> is either <1:xxxx> (true at pole) or <slat>/<1:xxxx> (true at <slat>)\n");
			GMT_message (C, "\t     or <radius>/<lat> (distance in %s to the [oblique] parallel <lat>.\n", C->session.unit_name[C->current.setting.proj_length_unit]);

			GMT_message (C, "\t   -Jt|T<lon0>/[<lat0>/]<scale>|<width> (Transverse Mercator).\n\t         Give central meridian and scale\n");
			GMT_message (C, "\t     Optionally, also give the central parallel (default = equator)\n");

			GMT_message (C, "\t   -Ju|U<zone>/<scale>|<width> (UTM)\n");
			GMT_message (C, "\t     Give zone (A,B,Y,Z, or 1-60 (negative for S hemisphere) or append C-X) and scale\n");

			GMT_message (C, "\t   -Jv|V[<lon0>/]<scale>|<width> (van der Grinten)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Jw|W[<lon0>/]<scale>|<width> (Mollweide)\n\t     Give central meridian (opt) and scale\n");

			GMT_message (C, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scale>|<width> (Cylindrical Equal-area)\n");
			GMT_message (C, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			GMT_message (C, "\t     <lat0> = 50 (Balthasart), 45 (Gall-Peters), 37.5 (Hobo-Dyer), 37.4 (Trystan Edwards),\n");
			GMT_message (C, "\t              37.0666 (Caster), 30 (Behrmann), 0 (Lambert, default)\n");

			GMT_message (C, "\t   -Jx|X<x-scale|<width>[/<y-scale|height>] (Linear, log, power scaling)\n");
			GMT_message (C, "\t     <scale> in %s/units (or 1:xxxx). Optionally, append to <x-scale> and/or <y-scale>:\n", C->session.unit_name[C->current.setting.proj_length_unit]);
			GMT_message (C, "\t       d         Geographic coordinate (in degrees)\n");
			GMT_message (C, "\t       l         Log10 projection\n");
			GMT_message (C, "\t       p<power>  x^power projection\n");
			GMT_message (C, "\t       t         Calendar time projection using relative time coordinates\n");
			GMT_message (C, "\t       T         Calendar time projection using absolute time coordinates\n");
			GMT_message (C, "\t     Use / to specify separate x/y scaling (e.g., -Jx0.5c/0.3c).  Not allowed with 1:xxxxx.\n");
			GMT_message (C, "\t     If -JX is used then give axes lengths rather than scales.\n");
			break;

		case 'j':	/* Condensed version of J */

			GMT_message (C, "\t-J Select map proJection. (<scale> in %s/degree, <width> in %s)\n", C->session.unit_name[C->current.setting.proj_length_unit], C->session.unit_name[C->current.setting.proj_length_unit]);
			GMT_message (C, "\t   Append h for map height, or +|- for max|min map dimension.\n");
			GMT_message (C, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is set.\n\n");

			GMT_message (C, "\t   -Ja|A<lon0>/<lat0>[/<hor>]/<scl (or <radius>/<lat>)|<width> (Lambert Azimuthal EA)\n");

			GMT_message (C, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Albers Conic EA)\n");

			GMT_message (C, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<lat1>/<lat2>/<scl>|<width> (Cylindrical Stereographic)\n");

			GMT_message (C, "\t   -Jc|C<lon0>/<lat0><scl>|<width> (Cassini)\n");

			GMT_message (C, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Equidistant Conic)\n");

			GMT_message (C, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scl (or <radius>/<lat>)|<width>  (Azimuthal Equidistant)\n");

			GMT_message (C, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scl (or <radius>/<lat>)|<width>  (Gnomonic)\n");

			GMT_message (C, "\t   -Jg|G<lon0>/<lat0>/<scl (or <radius>/<lat>)|<width>  (Orthographic)\n");

			GMT_message (C, "\t   -Jg|G[<lon0>/]<lat0>[/<horizon>|/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>]/<scl>|<width> (General Perspective)\n");

			GMT_message (C, "\t   -Jh|H[<lon0>/]<scl>|<width> (Hammer-Aitoff)\n");

			GMT_message (C, "\t   -Ji|I[<lon0>/]<scl>|<width> (Sinusoidal)\n");

			GMT_message (C, "\t   -Jj|J[<lon0>/]<scl>|<width> (Miller)\n");

			GMT_message (C, "\t   -Jkf|Kf[<lon0>/]<scl>|<width> (Eckert IV)\n");

			GMT_message (C, "\t   -Jks|Ks[<lon0>/]<scl>|<width> (Eckert VI)\n");

			GMT_message (C, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Lambert Conformal Conic)\n");

			GMT_message (C, "\t   -Jm|M[<lon0>/[<lat0>/]]<scl>|<width> (Mercator)\n");

			GMT_message (C, "\t   -Jn|N[<lon0>/]<scl>|<width> (Robinson projection)\n");

			GMT_message (C, "\t   -Jo|O (Oblique Mercator).  Specify one of three definitions:\n");
			GMT_message (C, "\t      -Jo|O[a]<lon0>/<lat0>/<azimuth>/<scl>|<width>\n");
			GMT_message (C, "\t      -Jo|O[b]<lon0>/<lat0>/<lon1>/<lat1>/<scl>|<width>\n");
			GMT_message (C, "\t      -Jo|Oc<lon0>/<lat0>/<lonp>/<latp>/<scl>|<width>\n");

			GMT_message (C, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scl>|<width> ((American) Polyconic)\n");

			GMT_message (C, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scl>|<width> (Equidistant Cylindrical)\n");

			GMT_message (C, "\t   -Jr|R[<lon0>/]<scl>|<width> (Winkel Tripel)\n");

			GMT_message (C, "\t   -Js|S<lon0>/<lat0>/[<horizon>/]<scl> (or <slat>/<scl> or <radius>/<lat>)|<width> (Stereographic)\n");

			GMT_message (C, "\t   -Jt|T<lon0>/[<lat0>/]<scl>|<width> (Transverse Mercator)\n");

			GMT_message (C, "\t   -Ju|U<zone>/<scl>|<width> (UTM)\n");

			GMT_message (C, "\t   -Jv|V<lon0>/<scl>|<width> (van der Grinten)\n");

			GMT_message (C, "\t   -Jw|W<lon0>/<scl>|<width> (Mollweide)\n");

			GMT_message (C, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scl>|<width> (Cylindrical Equal-area)\n");

			GMT_message (C, "\t   -Jp|P[a]<scl>|<width>[/<origin>][r|z] (Polar [azimuth] (theta,radius))\n");

			GMT_message (C, "\t   -Jx|X<x-scl>|<width>[d|l|p<power>|t|T][/<y-scl>|<height>[d|l|p<power>|t|T]] (Linear, log, and power projections)\n");
			GMT_message (C, "\t   (See psbasemap for more details on projection syntax)\n");
			break;

		case 'K':	/* Append-more-PostScript-later */

			GMT_message (C, "\t-K Allow for more plot code to be appended later.\n");
			break;

		case 'O':	/* Overlay plot */

			GMT_message (C, "\t-O Set Overlay plot mode, i.e., append to an existing plot.\n");
			break;

		case 'P':	/* Portrait or landscape */

			GMT_message (C, "\t-P Set Portrait page orientation [%s].\n", GMT_choice[C->current.setting.ps_orientation]);
			break;

		case 'R':	/* Region option */

			GMT_message (C, "\t-R Specify the min/max coordinates of data region in user units.\n");
			GMT_message (C, "\t   Use dd:mm[:ss] for regions given in degrees, minutes [and seconds].\n");
			GMT_message (C, "\t   Use [yyy[-mm[-dd]]]T[hh[:mm[:ss[.xxx]]]] format for time axes.\n");
			GMT_message (C, "\t   Append r if -R specifies the longitudes/latitudes of the lower left\n");
			GMT_message (C, "\t   and upper right corners of a rectangular area.\n");
			GMT_message (C, "\t   -Rg and -Rd are shorthands for -R0/360/-90/90 and -R-180/180/-90/90.\n");
			GMT_message (C, "\t   Or, give a gridfile to use its limits (and increments if applicable).\n");
			break;

		case 'r':	/* Region option for 3-D */

			GMT_message (C, "\t-R Specify the xyz min/max coordinates of the plot window in user units.\n");
			GMT_message (C, "\t   Use dd:mm[:ss] for regions given in degrees, minutes [and seconds].\n");
			GMT_message (C, "\t   Append r if first 4 arguments to -R specify the longitudes/latitudes\n");
			GMT_message (C, "\t   of the lower left and upper right corners of a rectangular area.\n");
			GMT_message (C, "\t   Or, give a gridfile to use its limits (and increments if applicable).\n");
			break;

		case 'U':	/* Plot time mark and [optionally] command line */

			GMT_message (C, "\t-U Plot Unix System Time stamp [and optionally appended text].\n");
			GMT_message (C, "\t   You may also set the reference points and position of stamp\n");
			GMT_message (C, "\t   [%s/%g%c/%g%c].  Give -Uc to have the command line plotted [%s].\n",
			GMT_just_string[C->current.setting.map_logo_justify], C->current.setting.map_logo_pos[GMT_X] * s, u, C->current.setting.map_logo_pos[GMT_Y] * s, u, GMT_choice[C->current.setting.map_logo]);
			break;

		case 'V':	/* Verbose */

			GMT_message (C, "\t-V Change the verbosity level (currently %ld).\n", C->current.setting.verbose);
			GMT_message (C, "\t   Choose among 5 levels; each level adds more messages:\n");
			GMT_message (C, "\t     0 - Complete silence, not even fatal error messages.\n");
			GMT_message (C, "\t     1 - Fatal error messages [Default when no -V is used].\n");
			GMT_message (C, "\t     2 - Warnings and progress messages [Default when -V is used].\n");
			GMT_message (C, "\t     3 - Detailed progress messages.\n");
			GMT_message (C, "\t     4 - Debugging messages.\n");
			break;

		case 'X':
		case 'Y':	/* Reset plot origin option */

			GMT_message (C, "\t-X -Y Shift origin of plot to (<xshift>, <yshift>).\n");
			GMT_message (C, "\t   Prepend r for shift relative to current point (default), prepend a for temporary\n");
			GMT_message (C, "\t   adjustment of origin, prepend f to position relative to lower left corner of page,\n");
			GMT_message (C, "\t   prepend c for offset of center of plot to center of page.\n");
			GMT_message (C, "\t   For overlays (-O), the default setting is [r0], otherwise [f%g%c].\n", C->current.setting.map_origin[GMT_Y] * s, u);
			break;

		case 'Z':	/* Vertical scaling for 3-D plots */

			GMT_message (C, "\t   -Jz For z component of 3-D projections.  Same syntax as -Jx.\n");
			break;

		case 'a':	/* -a option for aspatial field substitution into data columns */

			GMT_message (C, "\t-a Give one or more comma-separated <col>=<name> associations.\n");
			break;

		case 'C':	/* -b binary option with input only */

			GMT_message (C, "\t-bi For binary input; [<n>]<type>[w][+L|B]; <type> = c|u|h|H|i|I|l|L|f|D.\n");
			break;

		case '0':	/* -bi/-bo addendum when input format is unknown */

			GMT_message (C, "\t    Prepend <n> for the number of columns for each <type>.\n");
			break;

		case '1':	/* -bi/-bo addendum when input format is unknown */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':

			GMT_message (C, "\t    Prepend <n> for the number of columns for each <type> in binary file(s) [%c].\n", options[k]);
			break;

		case 'D':	/* -b binary option with output only */

			GMT_message (C, "\t-bo For binary output; append <type>[w][+L|B]; <type> = c|u|h|H|i|I|l|L|f|D..\n");
			break;

		case 'c':	/* -c option to set number of plot copies option */

			GMT_message (C, "\t-c Specify the number of copies [%ld].\n", C->PSL->init.copies);
			break;

		case 'f':	/* -f option to tell GMT which columns are time (and optionally geographical) */

			GMT_message (C, "\t-f Special formatting of input/output columns (time or geographical).\n");
			GMT_message (C, "\t   Specify i(nput) or o(utput) [Default is both input and output].\n");
			GMT_message (C, "\t   Give one or more columns (or column ranges) separated by commas.\n");
			GMT_message (C, "\t   Append T (Calendar format), t (time relative to TIME_EPOCH),\n");
			GMT_message (C, "\t   f (floating point), x (longitude), y (latitude) to each item.\n");
			GMT_message (C, "\t   -f[i|o]g means -f[i|o]0x,1y (geographic coordinates).\n");
			break;

		case 'g':	/* -g option to tell GMT to identify data gaps based on point separation */

			GMT_message (C, "\t-g Use data point separations to determine data gaps.\n");
			GMT_message (C, "\t   Append x|X or y|Y to flag data gaps in x or y coordinates,\n");
			GMT_message (C, "\t   respectively, and d|D for distance gaps.\n");
			GMT_message (C, "\t   Upper case means we first project the points.  Append <gap>[unit].\n");
			GMT_message (C, "\t   Geographic data: choose from %s [Default is meter (e)].\n", GMT_LEN_UNITS2_DISPLAY);
			GMT_message (C, "\t   For gaps based on mapped coordinates, choose from %s [%s].\n", GMT_DIM_UNITS_DISPLAY, C->session.unit_name[C->current.setting.proj_length_unit]);
			GMT_message (C, "\t   Note: For x|y with time data the unit is controlled by TIME_UNIT.\n");
			GMT_message (C, "\t   Repeat option to specify multiple criteria, and prepend +\n");
			GMT_message (C, "\t   to indicate that all the critera must be met [any].\n");
			break;

		case 'h':	/* Header */

			GMT_message (C, "\t-h[i][<n>] Input/output file has [%ld] Header record(s) [%s]\n", C->current.setting.io_n_header_items, GMT_choice[C->current.setting.io_header[GMT_IN]]);
			GMT_message (C, "\t   Optionally, append i for input only and/or number of header records.\n");
			GMT_message (C, "\t   For binary files, <n> is considered to mean number of bytes.\n");
			break;

		case 'i':	/* -i option for input column order */

			GMT_message (C, "\t-i Sets alternate input column order [Default reads all columns in order].\n");
			break;

		case 'n':	/* -n option for grid resampling parameters in BCR */

			GMT_message (C, "\t-n[b|c|l|n][+a][+b<BC>][+t<threshold>] Determine the grid interpolation mode.\n");
			GMT_message (C, "\t   (b = B-spline, c = bicubic, l = bilinear, n = nearest-neighbor) [Default: bicubic].\n");
			GMT_message (C, "\t   Append +a switch off antialiasing (except for l) [Default: on].\n");
			GMT_message (C, "\t   Append +b<BC> to change boundary conditions.  <BC> can be either:\n");
			GMT_message (C, "\t   g for geographic boundary conditions, or one or both of\n");
			GMT_message (C, "\t   x for periodic boundary conditions on x,\n");
			GMT_message (C, "\t   y for periodic boundary conditions on y.\n");
			GMT_message (C, "\t   [Default: Natural conditions, unless grid is known to be geographic].\n");
			GMT_message (C, "\t   Append +t<threshold> to change the minimum weight in vicinity of NaNs. A threshold of\n");
			GMT_message (C, "\t   1.0 requires all nodes involved in interpolation to be non-NaN; 0.5 will interpolate\n");
			GMT_message (C, "\t   about half way from a non-NaN to a NaN node [Default: 0.5].\n");

		case 'o':	/* -o option for output column order */

			GMT_message (C, "\t-o Set alternate output column order [Default writes all columns in order].\n");
			break;

		case 'p':	/* Enhanced pseudo-perspective 3-D plot settings */
#ifdef GMT_COMPAT
		case 'E':	/* For backward compatibility */
#endif
			GMT_message (C, "\t-%c Select a 3-D pseudo perspective view.  Append the\n", options[k]);
			GMT_message (C, "\t   azimuth and elevation of the viewpoint [180/90].\n");
			GMT_message (C, "\t   When used with -Jz|Z, optionally add zlevel for frame, etc [bottom of z-axis]\n");
			GMT_message (C, "\t   Optionally, append +w<lon/lat[/z] to specify a fixed point\n");
			GMT_message (C, "\t   and +vx/y for its justification.  Just append + by itself\n");
			GMT_message (C, "\t   to select default values [region center and page center].\n");
			break;

		case 's':	/* Output control for records where z are NaN */

			GMT_message (C, "\t-s Suppress output for records whose z-value (col = 2) equals NaN\n");
			GMT_message (C, "\t   [Default prints all records].\n");
			GMT_message (C, "\t   Append <cols> to examine other column(s) instead [2].\n");
			GMT_message (C, "\t   Append a to suppress records where any column equals NaN.\n");
			GMT_message (C, "\t   Append r to reverse the suppression (only output z = NaN records).\n");
			break;

		case 'F':	/* -r Pixel registration option  */

			GMT_message (C, "\t-r Set pixel registration [Default is grid registration].\n");
			break;

		case 't':	/* -t layer transparency option  */

			GMT_message (C, "\t-t Set the overlay PDF transparency from 0-100 [Default is 0; opaque].\n");
			break;

		case ':':	/* lon/lat [x/y] or lat/lon [y/x] */

			GMT_message (C, "\t-: Swap 1st and 2nd column on input and/or output [%s/%s].\n", GMT_choice[C->current.setting.io_lonlat_toggle[GMT_IN]], GMT_choice[C->current.setting.io_lonlat_toggle[GMT_OUT]]);
			break;

		case '.':	/* Trailer message */

			GMT_message (C, "\t-^ Print short synopsis message.\n");
			GMT_message (C, "\t-? Print this usage message\n");
			GMT_message (C, "\t(See gmt.conf man page for GMT default parameters).\n");
			break;

		case '<':	/* Table input */

			GMT_message (C, "\t<table> is one or more data files (in ASCII, binary, netCDF).\n");
			GMT_message (C, "\t   If no files are given, standard input is read.\n");
			break;

		default:
			break;
		}
	}
}

void GMT_label_syntax (struct GMT_CTRL *C, GMT_LONG indent, GMT_LONG kind)
{
	/* Contour/line specifications in *contour and psxy[z]
	 * indent is the number of spaces to indent after the TAB.
	 * kind = 0 for *contour and 1 for psxy[z]
	 */

	GMT_LONG i;
	char pad[16], *type[2] = {"Contour", "Line"};

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	GMT_message (C, "%s +a<angle> will place all annotations at a fixed angle.\n", pad);
	GMT_message (C, "%s Or, specify +an (line-normal) or +ap (line-parallel) [Default].\n", pad);
	if (kind == 0) {
		GMT_message (C, "%s   For +ap, you may optionally append u for up-hill\n", pad);
		GMT_message (C, "%s   and d for down-hill cartographic annotations.\n", pad);
	}
	GMT_message (C, "%s +c<dx>[/<dy>] sets clearance between label and text box [15%%].\n", pad);
	GMT_message (C, "%s +d turns on debug which draws helper points and lines.\n", pad);
	GMT_message (C, "%s +e delays the plotting of the text as text clipping is set instead.\n", pad);
	GMT_message (C, "%s +f sets specified label font [Default is %s].\n", pad, GMT_putfont (C, C->current.setting.font_annot[0]));
	GMT_message (C, "%s +g[<color>] paints text box [transparent]; append color [white].\n", pad);
	GMT_message (C, "%s +j<just> sets label justification [Default is CM].\n", pad);
	if (kind == 1) {
		GMT_message (C, "%s +l<text> Use text as label (quote text if containing spaces).\n", pad);
		GMT_message (C, "%s +L<d|D|f|h|n|N|x>[<unit>] Sets label according to given flag:\n", pad);
		GMT_message (C, "%s   d Cartesian plot distance; append a desired unit from %s.\n", pad, GMT_DIM_UNITS_DISPLAY);
		GMT_message (C, "%s   D Map distance; append a desired unit from %s.\n", pad, GMT_LEN_UNITS_DISPLAY);
		GMT_message (C, "%s   f Label is last column of given label location file.\n", pad);
		GMT_message (C, "%s   h Use segment header labels (via -Lstring).\n", pad);
		GMT_message (C, "%s   n Use the current segment number (starting at 0).\n", pad);
		GMT_message (C, "%s   N Use current file number / segment number (starting at 0/0).\n", pad);
		GMT_message (C, "%s   x Like h, but us headers in file with crossing lines instead.\n", pad);
	}
	GMT_message (C, "%s +n<dx>[/<dy>] to nudge label along line (+N for along x/y axis).\n", pad);
	GMT_message (C, "%s +o to use rounded rectangular text box [Default is rectangular].\n", pad);
	GMT_message (C, "%s +p[<pen>] draw outline of textbox  [Default is no outline].\n", pad);
	GMT_message (C, "%s   Optionally append a pen [Default is default pen].\n", pad);
	GMT_message (C, "%s +r<rmin> skips labels where radius of curvature < <rmin> [0].\n", pad);
	GMT_message (C, "%s +t[<file>] saves (x y label) to <file> [%s_labels.txt].\n", pad, type[kind]);
	GMT_message (C, "%s   use +T to save (x y angle label) instead\n", pad);
	GMT_message (C, "%s +u<unit> to append unit to all labels; Start unit with - for\n", pad);
	GMT_message (C, "%s   no space between annotation and unit.\n", pad);
	if (kind == 0) GMT_message (C, "%s  If no unit appended, use z-unit from grdfile [no unit].\n", pad);
	GMT_message (C, "%s +v for placing curved text along path [Default is straight].\n", pad);
	GMT_message (C, "%s +w sets how many (x,y) points to use for angle calculation [10].\n", pad);
	GMT_message (C, "%s +=<prefix> to give all labels a prefix; Start unit with - for\n", pad);
	GMT_message (C, "%s   no space between annotation and prefix.\n", pad);
}

void GMT_cont_syntax (struct GMT_CTRL *C, GMT_LONG indent, GMT_LONG kind)
{
	/* Contour/line label placement specifications in *contour and psxy[z]
	 * indent is the number of spaces to indent after the TAB.
	 * kind = 0 for *contour and 1 for psxy[z]
	 */
	GMT_LONG i;
	double gap;
	char pad[16];
	char *type[2] = {"contour", "quoted line"};

	gap = 4.0 * C->session.u2u[GMT_INCH][C->current.setting.proj_length_unit];

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	GMT_message (C, "%sd<dist>[%s] or D<dist>[%s]  [Default is d%g%c].\n", pad, GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY, gap, C->session.unit_name[C->current.setting.proj_length_unit][0]);
	GMT_message (C, "%s   d: Give distance between labels with specified map unit in %s.\n", pad, GMT_DIM_UNITS_DISPLAY);
	GMT_message (C, "%s   D: Specify geographic distance between labels in %s,\n", pad, GMT_LEN_UNITS_DISPLAY);
	GMT_message (C, "%s   The first label appears at <frac>*<dist>; change by appending /<frac> [0.25].\n", pad);
	GMT_message (C, "%sf<ffile.d> reads file <ffile.d> and places labels at locations\n", pad);
	GMT_message (C, "%s   that match individual points along the %ss.\n", pad, type[kind]);
	GMT_message (C, "%sl|L<line1>[,<line2>,...] Give start and stop coordinates for\n", pad);
	GMT_message (C, "%s   straight line segments.  Labels will be placed where these\n", pad);
	GMT_message (C, "%s   lines intersect %ss.  The format of each <line>\n", pad, type[kind]);
	GMT_message (C, "%s   is <start>/<stop>, where <start> or <stop> = <lon/lat> or a\n", pad);
	GMT_message (C, "%s   2-character XY key that uses the \"pstext\"-style justification\n", pad);
	GMT_message (C, "%s   format to specify a point on the map as [LCR][BMT].\n", pad);
	if (kind == 0) {
		GMT_message (C, "%s   In addition, you can use Z-, Z+ to mean the global\n", pad);
		GMT_message (C, "%s   minimum and maximum locations in the grid.\n", pad);
	}
	GMT_message (C, "%s   L: Let point pairs define great circles [Straight lines].\n", pad);
	GMT_message (C, "%sn|N<n_label> sets number of equidistant labels per %s.\n", pad, type[kind]);
	GMT_message (C, "%s   N: Starts labeling exactly at the start of %s\n", pad, type[kind]);
	GMT_message (C, "%s     [Default centers the labels on the %s].\n", pad, type[kind]);
	GMT_message (C, "%s   N-1 places a single label at start of the %s, while\n", pad, type[kind]);
	GMT_message (C, "%s   N+1 places a single label at the end of the %s.\n", pad, type[kind]);
	GMT_message (C, "%s   Append /<min_dist> to enforce a minimum spacing between\n", pad);
	GMT_message (C, "%s   consecutive labels [0]\n", pad);
	GMT_message (C, "%sx|X<xfile.d> reads the multi-segment file <xfile.d> and places\n", pad);
	GMT_message (C, "%s   labels at intersections between %ss and lines in\n", pad, type[kind]);
	GMT_message (C, "%s   <xfile.d>.  Use X to resample the lines first.\n", pad);
	GMT_message (C, "%s   For all options, append +r<radius>[unit] to specify minimum\n", pad);
	GMT_message (C, "%s   radial separation between labels [0]\n", pad);
}

void GMT_inc_syntax (struct GMT_CTRL *C, char option, GMT_LONG error)
{
	if (error) GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c Specify increment(s) and optionally append units or flags.\n", option);
	GMT_message (C, "\t   Full syntax is <xinc>[%s|+][=][/<yinc>[%s|+][=]]\n", GMT_LEN_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY);
	GMT_message (C, "\t   For geographic regions in degrees you can optionally append units\n");
	GMT_message (C, "\t   among (f)eet, (m)inute, (s)econd, m(e)ter, (k)ilometer, (M)iles, (n)autical miles.\n");
	GMT_message (C, "\t   Append = to adjust the region to fit increments [Adjust increment to fit domain].\n");
	GMT_message (C, "\t   Alternatively, specify number of nodes by appending +. Then, the\n");
	GMT_message (C, "\t   increments are calculated from the given domain and node-registration settings\n");
	GMT_message (C, "\t   (see Appendix B for details).  Note: If -R<grdfile> was used then\n");
	GMT_message (C, "\t   bot -R and -I have been set; use -I to override those values.\n");
}

void GMT_fill_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c<fill> %s Specify <fill> as one of:\n", option, string);
	GMT_message (C, "\t   1) <gray> or <red>/<green>/<blue>, all in the range 0-255;\n");
	GMT_message (C, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	GMT_message (C, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	GMT_message (C, "\t   4) any valid color name;\n");
	GMT_message (C, "\t   5) P|p<dpi>/<pattern>[:F<color>B<color>], with <dpi> of the pattern.\n");
	GMT_message (C, "\t      Give <pattern> number from 1-90 or a filename, optionally add\n");
	GMT_message (C, "\t      replacement fore- or background colors (set - for transparency).\n");
	GMT_message (C, "\t   For PDF fill transparency, append @<transparency> in the range 0-100 [0 = opaque].\n");
}

void GMT_pen_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c ", option);
	GMT_message (C, string, GMT_putpen (C, C->current.setting.map_default_pen));
	GMT_message (C, "\n\t   <pen> is a comma-separated list of three optional items in the order:\n");
	GMT_message (C, "\t       <width>[%s], <color>, and <style>[%s].\n", GMT_DIM_UNITS, GMT_DIM_UNITS);
	GMT_message (C, "\t   <width> >= 0.0 sets pen width (default units are points); alternatively a pen\n");
	GMT_message (C, "\t       name: Choose among faint, default, or [thin|thick|fat][er|est], or obese.\n");
	GMT_message (C, "\t   <color> = (1) <gray> or <red>/<green>/<blue>, all in range 0-255,\n");
	GMT_message (C, "\t             (2) <c>/<m>/<y>/<k> in 0-100%% range,\n");
	GMT_message (C, "\t             (3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1,\n");
	GMT_message (C, "\t             (4) any valid color name.\n");
	GMT_message (C, "\t   <style> = (1) pattern of dashes (-) and dots (.), scaled by <width>.\n");
	GMT_message (C, "\t             (2) a for dashed, o for dotted lines, scaled by <width>.\n");
	GMT_message (C, "\t             (3) <pattern>:<offset>; <pattern> holds lengths (default unit points)\n");
	GMT_message (C, "\t                 of any number of lines and gaps separated by underscores.\n");
	GMT_message (C, "\t                 <offset> shifts elements from start of the line [0].\n");
	GMT_message (C, "\t   For PDF stroke transparency, append @<transparency> in the range 0-100 [0 = opaque].\n");
}

void GMT_rgb_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c<color> %s Specify <color> as one of:\n", option, string);
	GMT_message (C, "\t   1) <gray> or <red>/<green>/<blue>, all in range 0-255;\n");
	GMT_message (C, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	GMT_message (C, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	GMT_message (C, "\t   4) any valid color name.\n");
	GMT_message (C, "\t   For PDF fill transparency, append @<transparency> in the range 0-100 [0 = opaque].\n");
}

void GMT_mapscale_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c %s\n", option, string);
	GMT_message (C, "\t   Use -%cx to specify Cartesian coordinates instead.  Scale is calculated at latitude <slat>;\n", option);
	GMT_message (C, "\t   optionally give longitude <slon> [Default is central longitude].  Give scale <length> and\n");
	GMT_message (C, "\t   append unit from %s [km].  -%cf draws a \"fancy\" scale [Default is plain].\n", GMT_LEN_UNITS2_DISPLAY, option);
	GMT_message (C, "\t   By default, the label is set to the distance unit and placed on top [+jt].  Use the +l<label>\n");
	GMT_message (C, "\t   and +j<just> mechanisms to specify another label and placement (t,b,l,r).  +u sets the label as a unit.\n");
	GMT_message (C, "\t   Append +p<pen> and/or +f<fill> to draw/paint a rectangle behind the scale [no rectangle]\n");
}

void GMT_GSHHS_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c %s\n", option, string);
	GMT_message (C, "\t   Features smaller than <min_area> (in km^2) or of levels (0-4) outside the min-max levels\n");
	GMT_message (C, "\t   will be skipped [0/4 (4 means lake inside island inside lake)].\n");
	GMT_message (C, "\t   Append +r to only get riverlakes from level 2, or +l to only get lakes [both].\n");
	GMT_message (C, "\t   Append +p<percent> to exclude features whose size is < <percent>%% of the full-resolution feature [use all].\n");
}

void GMT_maprose_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c %s\n", option, string);
	GMT_message (C, "\t   Use -%cx to specify Cartesian coordinates instead.  -Tf draws a \"fancy\" rose [Default is plain].\n", option);
	GMT_message (C, "\t   Give rose <diameter> and optionally the west, east, south, north labels desired [W,E,S,N].\n");
	GMT_message (C, "\t   For fancy rose, specify as <info> the kind you want: 1 draws E-W, N-S directions [Default],\n");
	GMT_message (C, "\t   2 adds NW-SE and NE-SW, while 3 adds WNW-ESE, NNW-SSE, NNE-SSW, and ENE-WSW.\n");
	GMT_message (C, "\t   For Magnetic compass rose, specify -%cm.  Use the optional <info> = <dec>/<dlabel> (where <dec> is\n", option);
	GMT_message (C, "\t   the magnetic declination and <dlabel> is a label for the magnetic compass needle) to plot\n");
	GMT_message (C, "\t   directions to both magnetic and geographic north [Default is just geographic].\n");
	GMT_message (C, "\t   If the North label = \'*\' then a north star is plotted instead of the label.\n");
	GMT_message (C, "\t   Append +<gints>/<mints> to override default annotation/tick interval(s) [10/5/1/30/5/1].\n");
}

void GMT_dist_syntax (struct GMT_CTRL *C, char option, char *string)
{
	if (string[0] == ' ') GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);
	GMT_message (C, "\t-%c %s\n", option, string);
	GMT_message (C, "\t   Append e (meter), f (feet), k (km), M (mile), n (nautical mile),\n");
	GMT_message (C, "\t   d (arc degree), m (arc minute), or s (arc second) [e].\n");
	GMT_message (C, "\t   Prepend - for (fast) flat Earth or + for (slow) geodesic calculations.\n");
	GMT_message (C, "\t   [Default is spherical great-circle calculations].\n");
}

void GMT_syntax (struct GMT_CTRL *C, char option)
{
	/* The function print to stderr the syntax for the option indicated by
	 * the variable <option>.  Only the common parameter options are covered
	 */

	char *u = C->session.unit_name[C->current.setting.proj_length_unit];

	GMT_report (C, GMT_MSG_FATAL, "Syntax error -%c option.  Correct syntax:\n", option);

	switch (option) {

		case 'B':	/* Tickmark option */
			GMT_message (C, "\t-B[p|s][a|f|g]<tick>[m][l|p][:\"label\":][:,\"unit\":][/.../...]:.\"Title\":[W|w|E|e|S|s|N|n][Z|z]\n");
			break;

		case 'J':	/* Map projection option */
			switch (C->current.proj.projection) {
				case GMT_LAMB_AZ_EQ:
					GMT_message (C, "\t-Ja<lon0>/<lat0>[/<horizon>]/<scale> OR -JA<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (C, "\t  <horizon> is distance from center to perimeter (<= 180, default 90)\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ALBERS:
					GMT_message (C, "\t-Jb<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JB<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CASSINI:
					GMT_message (C, "\t-Jc<lon0>/<lat0><scale> OR -JC<lon0>/<lat0><width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree ,or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_STEREO:
					GMT_message (C, "\t-Jcyl_stere/[<lon0>/[<lat0>/]]<scale> OR -JCyl_stere/[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECONIC:
					GMT_message (C, "\t-Jd<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JD<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_AZ_EQDIST:
					GMT_message (C, "\t-Je<lon0>/<lat0>[/<horizon>]/<scale> OR -JE<lon0>/<lat0>[/<horizon>/<width>\n");
					GMT_message (C, "\t  <horizon> is distance from center to perimeter (<= 180, default 180)\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GNOMONIC:
					GMT_message (C, "\t-Jf<lon0>/<lat0>[/<horizon>]/<scale> OR -JF<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (C, "\t  <horizon> is distance from center to perimeter (< 90, default 60)\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ORTHO:
					GMT_message (C, "\t-Jg<lon0>/<lat0>[/<horizon>]/<scale> OR -JG<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (C, "\t  <horizon> is distance from center to perimeter (<= 90, default 90)\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GENPER:
					GMT_message (C, "\t-Jg<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale> OR\n\t-JG<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_HAMMER:
					GMT_message (C, "\t-Jh[<lon0>/]<scale> OR -JH[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_SINUSOIDAL:
					GMT_message (C, "\t-Ji[<lon0>/]<scale> OR -JI[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MILLER:
					GMT_message (C, "\t-Jj[<lon0>/]<scale> OR -JJ[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT4:
					GMT_message (C, "\t-Jkf[<lon0>/]<scale> OR -JKf[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT6:
					GMT_message (C, "\t-Jk[s][<lon0>/]<scale> OR -JK[s][<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_LAMBERT:
					GMT_message (C, "\t-Jl<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JL<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MERCATOR:
					GMT_message (C, "\t-Jm[<lon0>/[<lat0>/]]<scale> OR -JM[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ROBINSON:
					GMT_message (C, "\t-Jn[<lon0>/]<scale> OR -JN[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_OBLIQUE_MERC:
					GMT_message (C, "\t-Jo[a]<lon0>/<lat0>/<azimuth>/<scale> OR -JO[a]<lon0>/<lat0>/<azimuth>/<width>\n");
					GMT_message (C, "\t-Jo[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<scale> OR -JO[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<width>\n");
					GMT_message (C, "\t-Joc<lon0>/<lat0>/<lonp>/<latp>/<scale> OR -JOc<lon0>/<lat0>/<lonp>/<latp>/<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/oblique degree, or use <width> in %s\n", u, u);
					break;
				case GMT_WINKEL:
					GMT_message (C, "\t-Jr[<lon0>/]<scale> OR -JR[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLYCONIC:
					GMT_message (C, "\t-Jpoly/[<lon0>/[<lat0>/]]<scale> OR -JPoly/[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQDIST:
					GMT_message (C, "\t-Jq[<lon0>/[<lat0>/]]<scale> OR -JQ[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_STEREO:
					GMT_message (C, "\t-Js<lon0>/<lat0>[/<horizon>]/<scale> OR -JS<lon0>/<lat0>[/<horizon>]/<width>\n");
					GMT_message (C, "\t  <horizon> is distance from center to perimeter (< 180, default 90)\n");
					GMT_message (C, "\t  <scale> is <1:xxxx>, <lat>/<1:xxxx>, or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_TM:
					GMT_message (C, "\t-Jt<lon0>/[<lat0>/]<scale> OR -JT<lon0>/[<lat0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_UTM:
					GMT_message (C, "\t-Ju<zone>/<scale> OR -JU<zone>/<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					GMT_message (C, "\t  <zone is A, B, 1-60[w/ optional C-X except I, O], Y, Z\n");
					break;
				case GMT_VANGRINTEN:
					GMT_message (C, "\t-Jv<lon0>/<scale> OR -JV[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MOLLWEIDE:
					GMT_message (C, "\t-Jw[<lon0>/]<scale> OR -JW[<lon0>/]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQ:
					GMT_message (C, "\t-Jy[<lon0>/[<lat0>/]]<scale> OR -JY[<lon0>/[<lat0>/]]<width>\n");
					GMT_message (C, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLAR:
					GMT_message (C, "\t-Jp[a]<scale>[/<origin>][r] OR -JP[a]<width>[/<origin>][r]\n");
					GMT_message (C, "\t  <scale> is %s/units, or use <width> in %s\n", u, u);
					GMT_message (C, "\t  Optionally, prepend a for azimuths, append theta as origin [0],\n");
					GMT_message (C, "\t  or append r to reverse radial coordinates.\n");
				case GMT_LINEAR:
					GMT_message (C, "\t-Jx<x-scale>|<width>[d|l|p<power>|t|T][/<y-scale>|<height>[d|l|p<power>|t|T]], scale in %s/units\n", u);
					GMT_message (C, "\t-Jz<z-scale>[l|p<power>], scale in %s/units\n", u);
					GMT_message (C, "\tUse / to specify separate x/y scaling (e.g., -Jx0.5/0.3.).  Not allowed with 1:xxxxx\n");
					GMT_message (C, "\tUse -JX (and/or -JZ) to give axes lengths rather than scales\n");
					break;
				default:
					GMT_message (C, "\tProjection not recognized!\n");
					break;
			}
			break;

		case 'K':
			GMT_message (C, "\t-%c More PS matter will follow\n", option);
			break;

		case 'O':
			GMT_message (C, "\t-%c This is a PS overlay\n", option);
			break;

		case 'P':
			GMT_message (C, "\t-%c Turn on portrait mode\n", option);
			break;

		case 'R':	/* Region option */
			GMT_message (C, "\t-R<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>]\n");
			GMT_message (C, "\tAppend r if giving lower left and upper right coordinates\n");
			break;

		case 'U':	/* Set time stamp option */
			GMT_message (C, "\t-U[<just>/<dx>/<dy>/][c|<label>], c will plot command line.\n");
			break;

		case 'X':
		case 'Y':
			GMT_message (C, "\t-%c[a|c|f|r]<shift>[u]\n", option);
			GMT_message (C, "\tPrepend a for temporaty adjustment, c for center of page reference,\n");
			GMT_message (C, "\tf for lower left corner of page reference, r (or none) for relative to\n");
			GMT_message (C, "\tcurrent position; u is unit (c, i, p).\n");
			break;

#ifdef GMT_COMPAT
		case 'Z':
			GMT_message (C, "\t-Z<zlevel> set zlevel of basemap\n");
			break;
#endif

		case 'b':	/* Binary i/o option  */
			GMT_message (C, "\t-b[i|o][<n>][<t>][w][+L|B], i for input, o for output [Default is both].\n");
			GMT_message (C, "\t   Here, t is c|u|h|H|i|I|l|L|f|d [Default is d (double)].\n");
			GMT_message (C, "\t   Prepend the number of data columns (for input only).\n");
			GMT_message (C, "\t   Append w to byte swap an item; append +L|B to fix endianness of file.\n");
			break;

		case 'c':	/* Set number of plot copies option */
			GMT_message (C, "\t-c<copies>, copies is number of copies\n");
			break;

		case 'f':	/* Column information option  */
			GMT_message (C, "\t-f[i|o]<colinfo>, i for input, o for output [Default is both].\n");
			GMT_message (C, "\t   <colinfo> is <colno>|<colrange>u, where column numbers start at 0\n");
			GMT_message (C, "\t   a range is given as <first>-<last>, e.g., 2-5., u is type:\n");
			GMT_message (C, "\t   t: relative time, T: absolute time, f: floating point,\n");
			GMT_message (C, "\t   x: longitude, y: latitude, g: geographic coordinate.\n");
			break;

		case 'g':
			GMT_message (C, "\t%s\n", GMT_g_OPT);
			GMT_message (C, "\t   (Consult manual)\n");
			break;

		case 'h':	/* Header */
			GMT_message (C, "\t-h[n_items]\n");
			break;

		case 'p':
			GMT_message (C, "\t%s\n", GMT_p_OPT);
			GMT_message (C, "\t   Azimuth and elevation (and zlevel) of the viewpoint [180/90/bottom z-axis].\n");
			GMT_message (C, "\t   Append +w and +v to set coordinates to a fixed viewpoint\n");
			break;

		case 's':	/* SKip records with NaN as z */
			GMT_message (C, "\t-s[<col>][a][r] to skip records whose <col> [2] output is NaN.\n");
			GMT_message (C, "\t   a skips if ANY columns is NaN, while r reverses the action.\n");
			break;

		case ':':	/* lon/lat vs lat/lon i/o option  */
			GMT_message (C, "\t-:[i|o], i for input, o for output [Default is both].\n");
			GMT_message (C, "\t   Swap 1st and 2nd column on input and/or output.\n");
			break;

		default:
			break;
	}
}

GMT_LONG GMT_default_error (struct GMT_CTRL *C, char option)
{
	/* GMT_default_error ignores all the common options that have already been processed and returns
	 * TRUE if the option is not an already processed common option.
	 */

	GMT_LONG error = 0;

	switch (option) {

		case '-': break;	/* Skip indiscriminently */
		case '>': break;	/* Skip indiscriminently since dealt with internally */
		case 'B': error += C->common.B.active[0] + C->common.B.active[1] == 0; break;
		case 'J': error += C->common.J.active == 0; break;
		case 'K': error += C->common.K.active == 0; break;
		case 'O': error += C->common.O.active == 0; break;
		case 'P': error += C->common.P.active == 0; break;
		case 'R': error += C->common.R.active == 0; break;
		case 'U': error += C->common.U.active == 0; break;
		case 'V': error += C->common.V.active == 0; break;
		case 'x':
		case 'X': error += C->common.X.active == 0; break;
		case 'y':
		case 'Y': error += C->common.Y.active == 0; break;
		case 'a': error += C->common.a.active == 0; break;
		case 'b': error += C->common.b.active[GMT_IN] + C->common.b.active[GMT_OUT] == 0; break;
		case 'c': error += C->common.c.active == 0; break;
		case 'f': error += C->common.f.active[GMT_IN] + C->common.f.active[GMT_OUT] == 0; break;
		case 'g': error += C->common.g.active == 0; break;
#ifdef GMT_COMPAT
		case 'H':
#endif
		case 'h': error += C->common.h.active == 0; break;
		case 'i': error += C->common.i.active == 0; break;
		case 'n': error += C->common.n.active == 0; break;
		case 'o': error += C->common.o.active == 0; break;
#ifdef GMT_COMPAT
		case 'Z': break;
		case 'E':
#endif
		case 'p': error += C->common.p.active == 0; break;
#ifdef GMT_COMPAT
		case 'm': break;
		case 'S':
		case 'F':
#endif
		case 'r': error += C->common.r.active == 0; break;
		case 's': error += C->common.s.active == 0; break;
		case 't': error += C->common.t.active == 0; break;
		case ':': error += C->common.colon.active == 0; break;

		default:
			/* Not a processed common options */
			GMT_report (C, GMT_MSG_FATAL, "Error: Unrecognized option -%c\n", option);
			error++;
			break;
	}

	return (error);
}

GMT_LONG gmt_parse_h_option (struct GMT_CTRL *C, char *item) {
	GMT_LONG i, j, k = 0, error = 0;

	/* Parse the -h option.  Full syntax: -h[i][o][<nrecs>] */

	if (!item || !item[0]) {	/* Nothing further to parse; just set defaults */
		C->current.io.io_header[GMT_IN] = C->current.io.io_header[GMT_OUT] = TRUE;
		C->current.io.io_n_header_items = 1;
		return (GMT_NOERROR);
	}
	j = 0;
	if (item[j] == 'i') k = j++;	/* -hi[nrecs] given */
	if (item[j]) {
		i = atoi (&item[j]);
		if (i < 0)
			error++;
		else
			C->current.io.io_n_header_items = i;
	}
	else C->current.io.io_n_header_items = 1;

	if (j == 0)	/* Both in and out may have header records */
		C->current.io.io_header[GMT_IN] = C->current.io.io_header[GMT_OUT] = (C->current.io.io_n_header_items > 0);
	else if (item[k] == 'i')		/* Only input should have header records */
		C->current.io.io_header[GMT_IN] = (C->current.io.io_n_header_items > 0);
	else		/* Only output should have header records */
		C->current.io.io_header[GMT_OUT] = (C->current.io.io_n_header_items > 0);
	return (error);
}

GMT_LONG GMT_check_region (struct GMT_CTRL *C, double wesn[])
{	/* If region is given then we must have w < e and s < n */
	return ((wesn[XLO] >= wesn[XHI] || wesn[YLO] >= wesn[YHI]) && !C->common.R.oblique);
}

GMT_LONG gmt_parse_R_option (struct GMT_CTRL *C, char *item) {
	GMT_LONG i, icol, pos, got, col_type[2], expect_to_read, error = 0;
	char text[GMT_BUFSIZ], string[GMT_BUFSIZ];
	double p[6];

	/* Parse the -R option.  Full syntax: -R<grdfile> or -Rg or -Rd or -R[g|d]w/e/s/n[/z0/z1][r] */

	strcpy (C->common.R.string, item);	/* Verbatim copy */
	if ((item[0] == 'g' || item[0] == 'd') && item[1] == '\0') {	/* Check -Rd|g separately in case user has files called d or g */
		if (item[0] == 'g')	/* -Rg is shorthand for -R0/360/-90/90 */
			C->common.R.wesn[XLO] = 0.0, C->common.R.wesn[XHI] = 360.0;
		else			/* -Rd is shorthand for -R-180/+180/-90/90 */
			C->common.R.wesn[XLO] = -180.0, C->common.R.wesn[XHI] = 180.0;
		C->common.R.wesn[YLO] = -90.0;	C->common.R.wesn[YHI] = +90.0;
		C->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON, C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
		return (GMT_NOERROR);
	}
	if (!GMT_access (C, item, R_OK)) {	/* Gave a readable file, presumably a grid */
		struct GMT_GRID *G = NULL;
		if ((error = GMT_Begin_IO (C->parent, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (C->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&item, (void **)&G)) return (GMT_DATA_READ_ERROR);
		if ((error = GMT_End_IO (C->parent, GMT_IN, 0))) return (error);	/* Disables further data input */
		GMT_memcpy (&(C->current.io.grd_info.grd), G->header, 1, struct GRD_HEADER);
		GMT_Destroy_Data (C->parent, GMT_ALLOCATED, (void **)&G);
		GMT_memcpy (C->common.R.wesn, C->current.io.grd_info.grd.wesn, 4, double);
		C->common.R.wesn[ZLO] = C->current.io.grd_info.grd.z_min;	C->common.R.wesn[ZHI] = C->current.io.grd_info.grd.z_max;
		C->current.io.grd_info.active = TRUE;
		return (GMT_NOERROR);
	}
	if (item[0] == 'g' || item[0] == 'd') {	/* Here we have a region appended to -Rd|g */
		C->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON, C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
		strcpy (string, &item[1]);
	}
	else
		strcpy (string, item);

	/* Now decode the string */

	col_type[0] = col_type[1] = 0;
	if (string[strlen(string)-1] == 'r') {
		C->common.R.oblique = TRUE;
		string[strlen(string)-1] = '\0';	/* Remove the trailing r so GMT_scanf will work */
	}
	else
		C->common.R.oblique = FALSE;
	i = pos = 0;
	GMT_memset (p, 6, double);
	while ((GMT_strtok (C, string, "/", &pos, text))) {
		if (i > 5) {
			error++;
			return (error);		/* Have to break out here to avoid segv on p[6]  */
		}
		/* Figure out what column corresponds to a token to get col_type[GMT_IN] flag  */
		if (i > 3)
			icol = 2;
		else if (C->common.R.oblique)
			icol = i%2;
		else
			icol = i/2;
		if (icol < 2 && C->current.setting.io_lonlat_toggle[GMT_IN]) icol = 1 - icol;	/* col_types were swapped */
		/* If column is either RELTIME or ABSTIME, use ARGTIME */
		if (C->current.io.col_type[GMT_IN][icol] == GMT_IS_UNKNOWN) {	/* No -J or -f set, proceed with caution */
			got = GMT_scanf_arg (C, text, C->current.io.col_type[GMT_IN][icol], &p[i]);
			if (got & GMT_IS_GEO)
				C->current.io.col_type[GMT_IN][icol] = got;
			else if (got & GMT_IS_RATIME)
				C->current.io.col_type[GMT_IN][icol] = got, C->current.proj.xyz_projection[icol] = GMT_TIME;
		}
		else {	/* Things are set, do or die */
			expect_to_read = (C->current.io.col_type[GMT_IN][icol] & GMT_IS_RATIME) ? GMT_IS_ARGTIME : C->current.io.col_type[GMT_IN][icol];
			error += GMT_verify_expectations (C, expect_to_read, GMT_scanf (C, text, expect_to_read, &p[i]), text);
		}
		if (error) return (error);

		i++;
	}
	if (C->common.R.oblique) d_swap (p[2], p[1]);	/* So w/e/s/n makes sense */
	if (i < 4 || i > 6 || (GMT_check_region (C, p) || (i == 6 && p[4] >= p[5]))) error++;
	GMT_memcpy (C->common.R.wesn, p, 6, double);	/* This will probably be reset by GMT_map_setup */
	error += GMT_check_condition (C, i == 6 && !C->current.proj.JZ_set, "Error: -R with six parameters requires -Jz|Z\n");

	return (error);
}

GMT_LONG gmt_parse_XY_option (struct GMT_CTRL *C, GMT_LONG axis, char *text)
{
	GMT_LONG i = 0;
	if (!text || !text[0]) {	/* Default is -Xr0 */
		C->current.ps.origin[axis] = 'r';
		C->current.setting.map_origin[axis] = 0.0;
		return (GMT_NOERROR);
	}
	switch (text[0]) {
		case 'r': case 'a': case 'f': case 'c':
			C->current.ps.origin[axis] = text[0]; i++; break;
		default:
			C->current.ps.origin[axis] = 'r'; break;
	}
	if (text[i])
		C->current.setting.map_origin[axis] = GMT_to_inch (C, &text[i]);
	else	/* Allow use of -Xc or -Xf meaning -Xc0 or -Xf0 */
		C->current.setting.map_origin[axis] = 0.0;
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_a_option (struct GMT_CTRL *C, char *arg)
{	/* -a<col>=<name>[:<type>][,<col>...][+g|G<geometry>] */
	GMT_LONG col, pos = 0;
	char p[GMT_BUFSIZ], name[GMT_BUFSIZ], A[64], *s = NULL, *c = NULL;
	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -a requires an argument */
	if ((s = strstr (arg, "+g")) || (s = strstr (arg, "+G"))) {	/* Also got +g|G<geometry> */
		C->common.a.geometry = gmt_ogr_get_geometry ((char *)(s+2));
		if (s[1] == 'G') C->common.a.clip = TRUE;	/* Clip features at Dateline */
		s[0] = '\0';	/* Temporarily truncate off the geometry */
		C->common.a.output = TRUE;	/* We are producing, not reading an OGR/GMT file */
		if (C->current.setting.io_seg_marker[GMT_OUT] != '>') {
			GMT_report (C, GMT_MSG_NORMAL, "Warning -a: OGR/GMT requires > as output segment marker; your selection of %c will be overruled by >\n", C->current.setting.io_seg_marker[GMT_OUT]);
			C->current.setting.io_seg_marker[GMT_OUT] = '>';
		}
	}
	else if (C->current.setting.io_seg_marker[GMT_IN] != '>') {
		GMT_report (C, GMT_MSG_NORMAL, "Warning -a: OGR/GMT requires < as input segment marker; your selection of %c will be overruled by >\n", C->current.setting.io_seg_marker[GMT_IN]);
		C->current.setting.io_seg_marker[GMT_IN] = '>';
	}
	while ((GMT_strtok (C, arg, ",", &pos, p))) {	/* Another col=name argument */
		if ((c = strchr (p, ':'))) {	/* Also got :<type> */
			C->common.a.type[C->common.a.n_aspatial] = gmt_ogr_get_type ((char *)(c+1));
			c[0] = '\0';	/* Truncate off the type */
		}
		else
			C->common.a.type[C->common.a.n_aspatial] = GMTAPI_DOUBLE;
		if (sscanf (p, "%[^=]=%s", A, name) != 2) return (GMT_PARSE_ERROR);	/* Did not get two items */
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
				if (col < GMT_Z || col >= GMT_MAX_COLUMNS) return (GMT_PARSE_ERROR);		/* Col value out of whack */
				break;
		}
		C->common.a.col[C->common.a.n_aspatial] = col;
		if (col < 0 && col != GMT_IS_Z) C->common.a.type[C->common.a.n_aspatial] = GMTAPI_TEXT;
		if (C->common.a.name[C->common.a.n_aspatial]) free ((void *)C->common.a.name[C->common.a.n_aspatial]);	/* Free any previous names */
		C->common.a.name[C->common.a.n_aspatial] = strdup (name);
		C->common.a.n_aspatial++;
		if (C->common.a.n_aspatial == MAX_ASPATIAL) return (GMT_PARSE_ERROR);	/* Too many items */
	}
	if (s) s[0] = '+';	/* Restore the geometry part */
	/* -a implies -fg */
	C->current.io.col_type[GMT_IN][GMT_X] = C->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
	C->current.io.col_type[GMT_IN][GMT_Y] = C->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_b_option (struct GMT_CTRL *C, char *text)
{
	/* Syntax:	-b[i][cvar1/var2/...] or -b[i|o]<n><type>[,<n><type>]... */

	GMT_LONG i, col = 0, k, ncol = 0, id = GMT_IN, i_or_o = FALSE, set = FALSE;
	GMT_LONG endian_swab = FALSE, swab = FALSE, error = FALSE;
	char *p = NULL, c;

	if (!text || !text[0]) return (GMT_PARSE_ERROR);	/* -b requires at least one arg, e.g, -bo */

	/* First determine if there is an endian modifer supplied */
	if ((p = strchr (text, '+'))) {	/* Yes */
		*p = '\0';	/* Temporarily chop off the modifier */
		switch (p[1]) {
			case 'B': case 'L': swab = (p[1] != GMT_ENDIAN); break;	/* Must swap */
			default:
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -b: Bad endian modifier +%c\n", (int)p[1]);
				return (EXIT_FAILURE);
				break;
		}
		if (strchr (text, 'w')) {	/* Cannot do individual swap when endian has been indicated */
			GMT_report (C, GMT_MSG_FATAL, "Syntax error -b: Cannot use both w and endian modifiers\n");
			return (EXIT_FAILURE);
		}
		endian_swab = TRUE;
	}
	
	/* Now deal with [i|o] modifier */
	if (text[0] == 'i') { id = GMT_IN; i_or_o = TRUE; }
	if (text[0] == 'o') { id = GMT_OUT; i_or_o = TRUE; }
	C->common.b.active[id] = TRUE;
	C->common.b.type[id] = 'd';	/* Set default to double */
	
	/* Because under GMT_COMPAT c means either netCDF or signed char we deal with netCDF up front */
	
	k = i_or_o;
#ifdef GMT_COMPAT
	if (text[k] == 'c' && !text[k+1]) {	/* Ambiguous case "-bic" which MAY mean netCDF */
		GMT_report (C, GMT_MSG_COMPAT, "Syntax warning: -b[i]c now applies to character tables, not to netCDF\n");
		GMT_report (C, GMT_MSG_COMPAT, "Syntax warning: If input is netCDF, just leave out -b[i]c\n");
		C->common.b.type[id] = 'c';
	}
	else if (text[k] == 'c' && text[k+1] != ',') {	/* netCDF */
		GMT_report (C, GMT_MSG_COMPAT, "Syntax warning: -b[i]c<varlist> is deprecated. Use <file>?<varlist> instead.\n");
		C->common.b.active[id] = FALSE;
		strcpy (C->common.b.varnames, &text[k+1]);
	}
	else
#endif
	if (text[k] && strchr ("cuhHiIfd" GMT_OPT ("sSD"), text[k]) && (!text[k+1] || (text[k+1] == 'w' && !text[k+2] ))) {	/* Just save the type for the entire record */
		C->common.b.type[id] = text[k];			/* Default column type */
		C->common.b.swab[id] = (text[k+1] == 'w');	/* Default swab */
	}
	else {
		for (i = k; text[i]; i++) {
			c = text[i];
			switch (c) {
#ifdef GMT_COMPAT
				case 's': case 'S': case 'D':	/* GMT 4 syntax with single and double precision w/wo swapping */
					if (c == 'S' || c == 'D') swab = TRUE;
					if (c == 'S' || c == 's') c = 'f';
					if (c == 'D') c = 'd';
					if (ncol == 0) ncol = 1;	/* in order to make -bs work as before */
#endif
				case 'l': case 'L':	/* 8-byte long integers */
					if (sizeof (GMT_LONG) == 4) {
						GMT_report (C, GMT_MSG_FATAL, "Syntax error -b: Cannot specify type %c in 32-bit mode\n", (int)c);
						return (EXIT_FAILURE);
					}
				case 'c': case 'u': case 'h': case 'H': case 'i': case 'I': case 'f': case 'd':
					if (text[i+1] == 'w') swab = TRUE;
					set = TRUE;
					if (ncol == 0) {
						GMT_report (C, GMT_MSG_FATAL, "Syntax error -b: Column count must be specified\n");
						return (EXIT_FAILURE);
					}
					for (k = 0; k < ncol; k++, col++) {	/* Assign io function pointer and data type for each column */
						C->current.io.fmt[id][col].io = GMT_get_io_ptr (C, id, swab, c);
						C->current.io.fmt[id][col].type = GMT_get_io_type (C, c);
					}
					ncol = 0;	/* Must parse a new number for each item */
					break;
				case 'x':	/* Binary skip before/after column */
					if (col == 0)	/* Must skip BEFORE reading first data column (flag this as a negative skip) */
						C->current.io.fmt[id][col].skip = -ncol;	/* Number of bytes to skip */
					else	/* Skip after reading previous column (hence col-1) */
						C->current.io.fmt[id][col-1].skip = ncol;	/* Number of bytes to skip */
					break;
				case '0':	/* Number of columns */
				case '1': case '2': case '3':
				case '4': case '5': case '6':
				case '7': case '8': case '9':
					ncol = atoi (&text[i]);
					if (ncol < 1) {
						GMT_report (C, GMT_MSG_FATAL, "Syntax error -b: Column count must be > 0\n");
						return (EXIT_FAILURE);
					}
					while (text[i] && isdigit ((int)text[i])) i++;	i--;	/* Wind past the digits */
					break;
				case ',': break;	/* Comma between sequences are optional and just ignored */
				case 'w':		/* Turn off the swap unless it set via +L|B */
					if (!endian_swab) swab = FALSE;	break;
				default:
					error = TRUE;
					GMT_report (C, GMT_MSG_FATAL, "Error: Malformed -b argument [%s]\n", text);
					GMT_syntax (C, 'b');
					break;
			}
		}
	}
	if (col == 0) col = ncol;	/* Maybe we got a column count */
	C->common.b.ncol[id] = col;
	if (col && !set) for (col = 0; col < C->common.b.ncol[id]; col++) {	/* Default binary type is double */
		C->current.io.fmt[id][col].io   = GMT_get_io_ptr (C, id, swab, 'd');
		C->current.io.fmt[id][col].type = GMT_get_io_type (C, 'd');
	}
	
	if (!i_or_o) {	/* Specified neither i or o so let settings apply to both */
		C->common.b.active[GMT_OUT] = C->common.b.active[GMT_IN];
		C->common.b.ncol[GMT_OUT] = C->common.b.ncol[GMT_IN];
		C->common.b.type[GMT_OUT] = C->common.b.type[GMT_IN];
		GMT_memcpy (C->current.io.fmt[GMT_OUT], C->current.io.fmt[GMT_OUT], C->common.b.ncol[GMT_IN], struct GMT_COL_TYPE);
	}

	GMT_set_bin_input (C);	/* Make sure we point to binary i/o functions after processing -b option */

	if (p) *p = '+';	/* Restore the + sign */
	return (error);
}

GMT_LONG gmt_parse_c_option (struct GMT_CTRL *C, char *arg)
{
	GMT_LONG i, error = 0;
	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -c requires an argument */

	i = atoi (arg);
	if (i < 1)
		error++;
	else
		C->PSL->init.copies = i;
	return (error);
}

GMT_LONG gmt_parse_f_option (struct GMT_CTRL *C, char *arg)
{
	/* Routine will decode the -f[i|o]<col>|<colrange>[t|T|g],... arguments */

	char copy[GMT_BUFSIZ], p[GMT_BUFSIZ], *c = NULL;
	GMT_LONG i, k = 1, start = -1, stop = -1, ic, pos = 0, code, *col = NULL;
	GMT_LONG both_i_and_o = FALSE;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -f requires an argument */

	if (arg[0] == 'i')	/* Apply to input columns only */
		col = C->current.io.col_type[GMT_IN];
	else if (arg[0] == 'o')	/* Apply to output columns only */
		col = C->current.io.col_type[GMT_OUT];
	else {			/* Apply to both input and output columns */
		both_i_and_o = TRUE;
		k = 0;
	}

	GMT_memset (copy, GMT_BUFSIZ, char);	/* Clean the copy */
	strncpy (copy, &arg[k], (size_t)GMT_BUFSIZ);	/* arg should NOT have a leading i|o part */

	if (copy[0] == 'g') {	/* Got -f[i|o]g which is shorthand for -f[i|o]0x,1y */
		if (both_i_and_o) {
			C->current.io.col_type[GMT_IN][GMT_X] = C->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
			C->current.io.col_type[GMT_IN][GMT_Y] = C->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
		}
		else {
			col[GMT_X] = GMT_IS_LON;
			col[GMT_Y] = GMT_IS_LAT;
		}
		return (GMT_NOERROR);
	}

	while ((GMT_strtok (C, copy, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((c = strchr (p, '-')))	/* Range of columns given. e.g., 7-9T */
			sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
		else if (isdigit ((int)p[0]))	/* Just a single column, e.g., 3t */
			start = stop = atoi (p);
		else				/* Just assume it goes column by column */
			start++, stop++;

		ic = (int) p[strlen(p)-1];	/* Last char in p is the potential code T, t, or g */
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
				GMT_report (C, GMT_MSG_FATAL, "Error: Malformed -f argument [%s]\n", arg);
				return 1;
				break;
		}

		/* Now set the code for these columns */

		if (both_i_and_o)
			for (i = start; i <= stop; i++) C->current.io.col_type[GMT_IN][i] = C->current.io.col_type[GMT_OUT][i] = code;
		else
			for (i = start; i <= stop; i++) col[i] = code;
	}
	return (GMT_NOERROR);
}

int gmt_compare_cols (const void *point_1, const void *point_2)
{
	/* Sorts cols into ascending order  */
	if (((struct GMT_COL_INFO *)point_1)->col < ((struct GMT_COL_INFO *)point_2)->col) return (-1);
	if (((struct GMT_COL_INFO *)point_1)->col > ((struct GMT_COL_INFO *)point_2)->col) return (+1);
	return (0);
}

GMT_LONG gmt_parse_i_option (struct GMT_CTRL *C, char *arg)
{
	/* Routine will decode the -i<col>|<colrange>[l][s<scale>][o<offset>],... arguments */

	char copy[GMT_BUFSIZ], p[GMT_BUFSIZ], *c = NULL;
#ifdef GMT_COMPAT
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256];
#endif
	GMT_LONG i, k = 0, start = -1, stop = -1, pos = 0, convert;
	double scale, offset;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -i requires an argument */

	GMT_memset (copy, GMT_BUFSIZ, char);	/* Get a clean copy */
	strncpy (copy, arg, (size_t)GMT_BUFSIZ);
	for (i = 0; i < GMT_BUFSIZ; i++) C->current.io.col_skip[i] = TRUE;	/* Initially, no input column is requested */

	while ((GMT_strtok (C, copy, ",", &pos, p))) {	/* While it is not empty, process it */
		convert = 0, scale = 1.0, offset = 0.0;

		if ((c = strchr (p, 'o'))) {	/* Look for offset */
			c[0] = '\0';	/* Wipe out the 'o' so that next scan terminates there */
			convert |= 1;
			offset = atof (&c[1]);
		}
		if ((c = strchr (p, 's'))) {	/* Look for scale factor */
			c[0] = '\0';	/* Wipe out the 's' so that next scan terminates there */
#ifdef GMT_COMPAT
			i = strlen (p) - 1;
			convert = (p[i] == 'l') ? 2 : 1;
			i = sscanf (&c[1], "%[^/]/%[^l]", txt_a, txt_b);
			if (i == 0) GMT_report (C, GMT_MSG_FATAL, "-i...s contains bad scale info\n");
			scale = atof (txt_a);
			if (i == 2) offset = atof (txt_b);
#else
			convert |= 1;
			scale = atof (&c[1]);
#endif
		}
		if ((c = strchr (p, 'l'))) {	/* Look for log indicator */
			c[0] = '\0';	/* Wipe out the 's' so that next scan terminates there */
			convert = 2;
		}

		if ((c = strchr (p, '-')))	/* Range of columns given. e.g., 7-9 */
			sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
		else if (isdigit ((int)p[0]))	/* Just a single column, e.g., 3 */
			start = stop = atoi (p);
		else				/* Just assume it goes column by column */
			start++, stop++;

		/* Now set the code for these columns */

		for (i = start; i <= stop; i++, k++) {
			C->current.io.col_skip[i] = FALSE;	/* Do not skip these */
			C->current.io.col[GMT_IN][k].col = i;		/* Requested order of columns */
			C->current.io.col[GMT_IN][k].order = k;		/* Requested order of columns */
			C->current.io.col[GMT_IN][k].convert = convert;
			C->current.io.col[GMT_IN][k].scale = scale;
			C->current.io.col[GMT_IN][k].offset = offset;
		}
	}
	qsort ((void *)C->current.io.col[GMT_IN], (size_t)k, sizeof (struct GMT_COL_INFO), gmt_compare_cols);
	C->common.i.n_cols = k;
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_o_option (struct GMT_CTRL *C, char *arg)
{
	/* Routine will decode the -o<col>|<colrange>,... arguments */

	char copy[GMT_BUFSIZ], p[GMT_BUFSIZ], *c = NULL;
	GMT_LONG i, k = 0, start = -1, stop = -1, pos = 0;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -o requires an argument */

	GMT_memset (copy, GMT_BUFSIZ, char);	/* Get a clean copy */
	strncpy (copy, arg, (size_t)GMT_BUFSIZ);

	while ((GMT_strtok (C, copy, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((c = strchr (p, '-')))	/* Range of columns given. e.g., 7-9 */
			sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
		else if (isdigit ((int)p[0]))	/* Just a single column, e.g., 3 */
			start = stop = atoi (p);
		else				/* Just assume it goes column by column */
			start++, stop++;

		/* Now set the code for these columns */

		for (i = start; i <= stop; i++, k++) {
			C->current.io.col[GMT_OUT][k].col = i;		/* Requested order of columns */
			C->current.io.col[GMT_OUT][k].order = k;	/* Requested order of columns */
		}
	}
	C->common.o.n_cols = k;
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_dash_option (struct GMT_CTRL *C, char *text)
{	/* parse any --PARAM[=value] arguments */
	GMT_LONG n;
	char *this = NULL;
	if (!text) return (GMT_NOERROR);
	if ((this = strchr (text, '='))) {	/* Got --PAR=VALUE */
		this[0] = '\0';	/* Temporarily remove the '=' character */
		n = GMT_setparameter (C, text, &this[1]);
		this[0] = '=';	/* Put it back were it was */
	}
	else				/* Got --PAR */
		n = GMT_setparameter (C, text, "true");
	return (n);
}

void GMT_check_lattice (struct GMT_CTRL *C, double *inc, GMT_LONG *pixel, GMT_LONG *active)
{	/* Uses provided settings to initialize the lattice settings from
	 * the -R<grdfile> if it was given; else it does nothing.
	 */
	if (!C->current.io.grd_info.active) return;	/* -R<grdfile> was not used; use existing settings */

	/* Here, -R<grdfile> was used and we will use the settings supplied by the grid file (unless overridden) */
	if (!active || *active == FALSE) {	/* -I not set separately */
		GMT_memcpy (inc, C->current.io.grd_info.grd.inc, 2, double);
		inc[GMT_Y] = C->current.io.grd_info.grd.inc[GMT_Y];
	}
	if (pixel) {	/* An pointer not NULL was passed that indicates grid registration */
		/* If a -F like option was set then toggle grid setting, else use grid setting */
		*pixel = (*pixel) ? !C->current.io.grd_info.grd.registration : C->current.io.grd_info.grd.registration;
	}
	if (active) *active = TRUE;	/* When 4th arg is not NULL it is set to TRUE (for Ctrl->active args) */
}

GMT_LONG GMT_check_binary_io (struct GMT_CTRL *C, GMT_LONG n_req) {
	GMT_LONG n_errors = 0;

	/* Check the binary options that are used with most GMT programs.
	 * C is the pointer to the GMT structure.
	 * n_req is the number of required columns. If 0 then it relies on
	 *    C->common.b.ncol[GMT_IN] to be non-zero.
	 * Return value is the number of errors that where found.
	 */

	if (!C->common.b.active[GMT_IN]) return (GMT_NOERROR);	/* Let machinery figure out input cols for ascii */

	/* These are specific tests for binary input */

	if (C->common.b.ncol[GMT_IN] == 0) C->common.b.ncol[GMT_IN] = n_req;
	if (C->common.b.ncol[GMT_IN] == 0) {
		GMT_report (C, GMT_MSG_FATAL, "Syntax error: Must specify number of columns in binary input data (-bi)\n");
		n_errors++;
	}
	else if (n_req > C->common.b.ncol[GMT_IN]) {
		GMT_report (C, GMT_MSG_FATAL, "Syntax error: Binary input data (-bi) provides %ld but must have at least %ld columns\n", C->common.b.ncol[GMT_IN], n_req);
		n_errors++;
	}

	GMT_report (C, GMT_MSG_NORMAL, "Provides %ld, expects %ld-column binary data\n", C->common.b.ncol[GMT_IN], n_req);

	return (n_errors);
}

GMT_LONG gmt_parse_U_option (struct GMT_CTRL *C, char *item) {
	GMT_LONG i, just, n = 0, n_slashes, error = 0;
	char txt_j[GMT_TEXT_LEN256], txt_x[GMT_TEXT_LEN256], txt_y[GMT_TEXT_LEN256];

	/* Parse the -U option.  Full syntax: -U[<just>/<dx>/<dy>/][c|<label>] */

	C->current.setting.map_logo = TRUE;
	if (!item || !item[0]) return (GMT_NOERROR);	/* Just basic -U with no args */

	for (i = n_slashes = 0; item[i]; i++) {
		if (item[i] == '/') n_slashes++;	/* Count slashes to detect [<just>]/<dx>/<dy>/ presence */
	}
	if (n_slashes >= 2) {	/* Probably gave -U[<just>]/<dx>/<dy>[/<string>] */
		if (item[0] == '/') { /* No justification given */
			n = sscanf (&item[1], "%[^/]/%[^/]/%[^\n]", txt_x, txt_y, C->current.ps.map_logo_label);
			just = 1;	/* Default justification is LL */
		}
		else {
			n = sscanf (item, "%[^/]/%[^/]/%[^/]/%[^\n]", txt_j, txt_x, txt_y, C->current.ps.map_logo_label);
			just = GMT_just_decode (C, txt_j, C->current.setting.map_logo_justify);
		}
		if (just < 0) {
			/* Garbage before first slash: we simply have -U<string> */
			strcpy (C->current.ps.map_logo_label, item);
		}
		else {
			C->current.setting.map_logo_justify = just;
			C->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (C, txt_x);
			C->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (C, txt_y);
		}
	}
	else
		strcpy (C->current.ps.map_logo_label, item);
	if ((item[0] == '/' && n_slashes == 1) || (item[0] == '/' && n_slashes >= 2 && n < 2)) error++;
	return (error);
}

GMT_LONG gmt_parse_colon_option (struct GMT_CTRL *C, char *item) {
	GMT_LONG error = 0, way, off = 0, ok[2] = {FALSE, FALSE};
	static char *mode[4] = {"i", "o", "", ""}, *dir[2] = {"input", "output"};
	char kase = (item) ? item[0] : '\0';
	/* Parse the -: option.  Full syntax: -:[i|o].
	 * We know that if -f was given it has already been parsed due to the parsing order imposed.
	 * Must check that -: does not conflict with -f */
	
	switch (kase) {
		case 'i':	/* Toggle on input data only */
			ok[GMT_IN] = TRUE;
			break;
		case 'o':	/* Toggle on output data only */
			ok[GMT_OUT] = TRUE;
			break;
		case '\0':	/* Toggle both input and output data */
			ok[GMT_IN] = ok[GMT_OUT] = TRUE;
			off = 2;
			break;
		default:
			error++;	/* Error */
			break;
	}
	for (way = 0; !error && way < 2; way++) if (ok[way]) {
		if (C->current.io.col_type[way][GMT_X] == GMT_IS_UNKNOWN && C->current.io.col_type[way][GMT_Y] == GMT_IS_UNKNOWN)	/* Dont know what x/y is yet */
			C->current.setting.io_lonlat_toggle[way] = TRUE;
		else if (C->current.io.col_type[way][GMT_X] == GMT_IS_FLOAT && C->current.io.col_type[way][GMT_Y] == GMT_IS_FLOAT)	/* Cartesian x/y vs y/x cannot be identified */
			C->current.setting.io_lonlat_toggle[way] = TRUE;
		else if (C->current.io.col_type[way][GMT_X] == GMT_IS_LON && C->current.io.col_type[way][GMT_Y] == GMT_IS_LAT)	/* Lon/lat becomes lat/lon */
			C->current.setting.io_lonlat_toggle[way] = TRUE;
		else if (C->current.io.col_type[way][GMT_X] == GMT_IS_LAT && C->current.io.col_type[way][GMT_Y] == GMT_IS_LON)	/* Already lat/lon! */
			GMT_report (C, GMT_MSG_FATAL, "Warning: -:%s given but %s order already set by -f; -:%s ignored.\n", mode[way+off], dir[way], mode[way+off]);
		else {
			GMT_report (C, GMT_MSG_FATAL, "Error: -:%s given but %s first two columns do not hold x/y or lon/lat\n", mode[way+off], dir[way]);
			error++;
		}
	}
	if (error) C->current.setting.io_lonlat_toggle[GMT_IN] = C->current.setting.io_lonlat_toggle[GMT_OUT] = FALSE;	/* Leave in case we had errors */
	return (error);
}

double gmt_neg_col_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute reverse col-separation before mapping */
	return (C->current.io.prev_rec[col] - C->current.io.curr_rec[col]);
}

double gmt_pos_col_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute forward col-separation before mapping */
	return (C->current.io.curr_rec[col] - C->current.io.prev_rec[col]);
}

double gmt_abs_col_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute absolute col-separation before mapping */
	return (fabs (C->current.io.curr_rec[col] - C->current.io.prev_rec[col]));
}

double gmt_neg_col_map_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute reverse col-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (C, C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][0] - X[col][1]);
}

double gmt_pos_col_map_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute forward col-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (C, C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][1] - X[col][0]);
}

double gmt_abs_col_map_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute forward col-separation after mapping */
	double X[2][2];
	GMT_geo_to_xy (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	GMT_geo_to_xy (C, C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (fabs (X[col][1] - X[col][0]));
}

double gmt_xy_map_dist (struct GMT_CTRL *C, GMT_LONG col)
{	/* Compute point-separation after mapping */
	double GMT_cartesian_dist_proj (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
	return (GMT_cartesian_dist_proj (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]));
}

double gmt_xy_deg_dist (struct GMT_CTRL *C, GMT_LONG col)
{
	double GMT_great_circle_dist_degree (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
	return (GMT_great_circle_dist_degree (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]));
}

double gmt_xy_true_dist (struct GMT_CTRL *C, GMT_LONG col)
{
	double GMT_great_circle_dist_meter (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
	return (GMT_great_circle_dist_meter (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]));
}

double gmt_xy_cart_dist (struct GMT_CTRL *C, GMT_LONG col)
{
	double GMT_cartesian_dist (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);
	return (GMT_cartesian_dist (C, C->current.io.prev_rec[GMT_X], C->current.io.prev_rec[GMT_Y], C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]));
}

GMT_LONG gmt_parse_g_option (struct GMT_CTRL *C, char *txt)
{
	GMT_LONG i, k = 0, c;
	/* Process the GMT gap detection option for parameters */
	/* Syntax, e.g., -g[x|X|y|Y|d|D|[<col>]z][+|-]<gap>[d|m|s|e|f|k|M|n|c|i|p] or -ga */

	if (!txt && !txt[0]) return (GMT_PARSE_ERROR);	/* -g requires an argument */
	if ((i = C->common.g.n_methods) == GMT_N_GAP_METHODS) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Cannot specify more than %d gap criteria\n", GMT_N_GAP_METHODS);
		return (1);
	}

	/* -g gap checking implies -mo if not already set */

	C->current.io.multi_segments[GMT_OUT] = TRUE;

	if (txt[0] == 'a') {	/* For multiple criteria, specify that all criteria be met [default is any] */
		k++;
		C->common.g.match_all = TRUE;
		if (!txt[k]) return (1);	/* Just a single -ga */
	}
	switch (txt[k]) {	/* Determine method used for gap detection */
		case 'x':	/* Difference in user's x-coordinates used for test */
			C->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			C->common.g.col[i] = GMT_X;
			break;
		case 'X':	/* Difference in user's mapped x-coordinates used for test */
			C->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_MAP_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			C->common.g.col[i] = GMT_X;
			break;
		case 'y':	/* Difference in user's y-coordinates used for test */
			C->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			C->common.g.col[i] = GMT_Y;
			break;
		case 'Y':	/* Difference in user's mapped y-coordinates used for test */
			C->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_MAP_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			C->common.g.col[i] = GMT_Y;
			break;
		case 'd':	/* Great circle (if geographic data) or Cartesian distance used for test */
			C->common.g.method[i] = (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && C->current.io.col_type[GMT_IN][GMT_Y] == GMT_IS_LAT) ? GMT_GAP_IN_GDIST : GMT_GAP_IN_CDIST;
			C->common.g.col[i] = -1;
			break;
		case 'D':	/* Cartesian mapped distance used for test */
			C->common.g.method[i] = GMT_GAP_IN_PDIST;
			C->common.g.col[i] = -1;
			break;
		case 'z':	/* Difference in user's z-coordinates used for test */
			C->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			C->common.g.col[i] = 2;
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
				GMT_report (C, GMT_MSG_FATAL, "Error: Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", txt[k]);
				return (1);
			}
			C->common.g.method[i] = (txt[k+1] == '-') ? GMT_NEGGAP_IN_COL : ((txt[k+1] == '+') ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			C->common.g.col[i] = atoi (&txt[c]);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Error: Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", txt[0]);
			return (1);
			break;
	}
	switch (C->common.g.method[i]) {
		case GMT_NEGGAP_IN_COL:
			C->common.g.get_dist[i] = (PFD) gmt_neg_col_dist;
			break;
		case GMT_POSGAP_IN_COL:
			C->common.g.get_dist[i] = (PFD) gmt_pos_col_dist;
			break;
		case GMT_ABSGAP_IN_COL:
			C->common.g.get_dist[i] = (PFD) gmt_abs_col_dist;
			break;
		case GMT_NEGGAP_IN_MAP_COL:
			C->common.g.get_dist[i] = (PFD) gmt_neg_col_map_dist;
			break;
		case GMT_POSGAP_IN_MAP_COL:
			C->common.g.get_dist[i] = (PFD) gmt_pos_col_map_dist;
			break;
		case GMT_ABSGAP_IN_MAP_COL:
			C->common.g.get_dist[i] = (PFD) gmt_abs_col_map_dist;
			break;
		case GMT_GAP_IN_GDIST:
			C->common.g.get_dist[i] = (PFD) gmt_xy_true_dist;
			break;
		case GMT_GAP_IN_CDIST:
			C->common.g.get_dist[i] = (PFD) gmt_xy_cart_dist;
			break;
		case GMT_GAP_IN_PDIST:
			C->common.g.get_dist[i] = (PFD) gmt_xy_map_dist;
			break;
		default:
			break;	/* Already set, or will be reset below  */
	}
	k++;	/* Skip to start of gap value */
	if (txt[k] == '-' || txt[k] == '+') k++;	/* Skip sign */
	if ((C->common.g.gap[i] = atof (&txt[k])) == 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Gap value must be non-zero\n");
		return (1);
	}
	if (C->common.g.method[i] == GMT_GAP_IN_GDIST) {	/* Convert any gap given to meters */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'd':	/* Arc degrees, reset pointer */
				C->common.g.get_dist[i] = (PFD) gmt_xy_deg_dist;
				C->common.g.method[i] = GMT_GAP_IN_DDIST;
				break;
			case 'm':	/* Arc minutes, reset pointer */
				C->common.g.get_dist[i] = (PFD) gmt_xy_deg_dist;
				C->common.g.method[i] = GMT_GAP_IN_DDIST;
				C->common.g.gap[i] *= GMT_MIN2DEG;
			case 's':	/* Arc seconds, reset pointer */
				C->common.g.get_dist[i] = (PFD) gmt_xy_deg_dist;
				C->common.g.method[i] = GMT_GAP_IN_DDIST;
				C->common.g.gap[i] *= GMT_SEC2DEG;
			case 'f':	/* Feet  */
				C->common.g.gap[i] *= METERS_IN_A_FOOT;
				break;
			case 'k':	/* Km  */
				C->common.g.gap[i] *= 1000.0;
				break;
			case 'M':	/* Miles */
				C->common.g.gap[i] *= METERS_IN_A_MILE;
				break;
			case 'n':	/* Nautical miles */
				C->common.g.gap[i] *= METERS_IN_A_NAUTICAL_MILE;
				break;
			default:	/* E.g., meters or junk */
				break;
		}
	}
	else if (C->common.g.method[i] == GMT_GAP_IN_PDIST){	/* Cartesian plot distance stuff */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'c':	/* cm */
				C->common.g.gap[i] /= 2.54;
				break;
			case 'p':	/* Points */
				C->common.g.gap[i] /= 72.0;
				break;
			default:	/* E.g., inch or junk */
				break;
		}
	}
	if ((C->common.g.col[i] + 1) > C->common.g.n_col) C->common.g.n_col = C->common.g.col[i] + 1;	/* Needed when checking since it may otherwise not be read */
	C->common.g.n_methods++;
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_n_option (struct GMT_CTRL *C, char *item)
{	/* Parse the -n option for 2-D grid resampling parameters -n[b|c|l|n][+a][+t<BC>][+<threshold>] */
	GMT_LONG pos = 0, j, k = 1;
	char p[GMT_TEXT_LEN256];

	switch (item[0]) {
		case '+':	/* Means no mode was specified so we get the default */
			C->common.n.interpolant = BCR_BICUBIC; k = 0; break;
		case 'n':
			C->common.n.interpolant = BCR_NEARNEIGHBOR; break;
		case 'l':
			C->common.n.interpolant = BCR_BILINEAR; break;
		case 'b':
			C->common.n.interpolant = BCR_BSPLINE; break;
		case 'c':
			C->common.n.interpolant = BCR_BICUBIC; break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Error: Use %s to set 2-D grid interpolation mode.\n", GMT_n_OPT);
			return (1);
			break;
	}

	/* Now look for +modifiers */

	while ((GMT_strtok (C, &item[k], "+", &pos, p))) {
		switch (p[0]) {
			case 'a':	/* Turn off antialias */
				C->common.n.antialias = FALSE;
				break;
			case 'b':	/* Set BCs */
				C->common.n.bc_set = TRUE;
				strncpy (C->common.n.BC, &p[1], (size_t)4);
				for (j = 0; j < strlen (C->common.n.BC); j++) {
					switch (C->common.n.BC[j]) {
						case 'g': case 'x': case 'y': break;
						default:
							GMT_report (C, GMT_MSG_FATAL, "Error -n: +b<BC> requires <BC> to be g or p[x|y], n[x|y]\n");
							break;
					}
				}
				break;
			case 't':	/* Set interpolation threshold */
				C->common.n.threshold = atof (&p[1]);
				if (C->common.n.threshold < 0.0 || C->common.n.threshold > 1.0) {
					GMT_report (C, GMT_MSG_FATAL, "Error -n: Interpolation threshold must be in [0,1] range\n");
					return (1);
				}
				break;
			default:	/* Bad modifier */
				GMT_report (C, GMT_MSG_FATAL, "Error: Use %s to set 2-D grid interpolation mode.\n", GMT_n_OPT);
				return (1);
				break;
		}
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_p_option (struct GMT_CTRL *C, char *item)
{
	GMT_LONG k, l = 0, s, pos = 0, error = 0;
	double az, el, z;
	char p[GMT_TEXT_LEN256], txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256];

	if (!C->common.J.active) GMT_report (C, GMT_MSG_FATAL, "Warning -p option works best in consort with -J (and -R or a grid)\n");
	switch (item[0]) {
		case 'x':
			C->current.proj.z_project.view_plane = GMT_X + GMT_ZW;
			l++;
			break;
		case 'y':
			C->current.proj.z_project.view_plane = GMT_Y + GMT_ZW;
			l++;
			break;
		case 'z':
			C->current.proj.z_project.view_plane = GMT_Z + GMT_ZW;
			l++;
			break;
		default:
			C->current.proj.z_project.view_plane = GMT_Z + GMT_ZW;
			break;
	}
	if ((k = sscanf (&item[l], "%lf/%lf/%lf", &az, &el, &z)) < 2) {
		GMT_report (C, GMT_MSG_FATAL, "Error in -p (%s): Syntax is %s\n", item, GMT_p_OPT);
		return 1;
	}
	if (el <= 0.0 || el > 90.0) {
		GMT_report (C, GMT_MSG_FATAL, "Syntax error -p option: Elevation must be in 0-90 range\n");
		return 1;
	}
	C->current.proj.z_project.view_azimuth = az;
	C->current.proj.z_project.view_elevation = el;
	if (k == 3) C->current.proj.z_level = z;

	for (s = 0; item[s] && item[s] != '/'; s++);	/* Look for position of slash / */
	for (k = 0; item[k] && item[k] != '+'; k++);	/* Look for +<options> strings */
	if (!item[k] || k < s) return 0;		/* No + before the slash, so we are done here */

	/* Decode +separated substrings */

	C->current.proj.z_project.fixed = TRUE;
	k++;
	if (!item[k]) return 0;	/* No specific settings given, we will apply default values in 3D init */
	while ((GMT_strtok (C, &item[k], "+", &pos, p))) {
		switch (p[0]) {
			case 'v':	/* Specify fixed view point in 2-D projected coordinates */
				if (sscanf (&p[1], "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_report (C, GMT_MSG_FATAL, "Error in -p (%s): Syntax is -p<az>/<el>[/<z>][+wlon0/lat0[/z0]][+vx0[%s]/y0[%s]]\n", p, GMT_DIM_UNITS, GMT_DIM_UNITS);
					return 1;
				}
				C->current.proj.z_project.view_x = GMT_to_inch (C, txt_a);
				C->current.proj.z_project.view_y = GMT_to_inch (C, txt_b);
				C->current.proj.z_project.view_given = TRUE;
				break;
			case 'w':	/* Specify fixed World point in user's coordinates */
				if (sscanf (&p[1], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c) < 2) {
					GMT_report (C, GMT_MSG_FATAL, "Error in -p: (%s)  Syntax is -p<az>/<el>[/<z>][+wlon0/lat0[/z0]][+vx0[%s]/y0[%s]]\n", p, GMT_DIM_UNITS, GMT_DIM_UNITS);
					return 1;
				}
				error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_X], GMT_scanf (C, txt_a, C->current.io.col_type[GMT_IN][GMT_X], &C->current.proj.z_project.world_x), txt_a);
				error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf (C, txt_b, C->current.io.col_type[GMT_IN][GMT_Y], &C->current.proj.z_project.world_y), txt_b);
				if (k == 3) error += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf (C, txt_c, C->current.io.col_type[GMT_IN][GMT_Z], &C->current.proj.z_project.world_z), txt_c);
				C->current.proj.z_project.world_given = TRUE;
				break;
			default:	/* If followed by an integer we assume this might be an exponential notation picked up by mistake */
				if (!isdigit ((int)p[0])) GMT_report (C, GMT_MSG_FATAL, "Warning -p: Unrecognized modifier %s (ignored)\n", p);
				break;
		}
	}
	return (error);
}

GMT_LONG gmt_parse_s_option (struct GMT_CTRL *C, char *item) {
	GMT_LONG error = 0, i, n, start = 0, stop = 0, pos = 0, tmp[GMT_MAX_COLUMNS];
	char p[GMT_BUFSIZ], *c = NULL;
	/* Parse the -s option.  Full syntax: -s[<cols>][r|a] */

	GMT_memset (C->current.io.io_nan_col, GMT_MAX_COLUMNS, GMT_LONG);
	C->current.io.io_nan_col[0] = GMT_Z;	/* The default is to examine the z-column */
	C->current.io.io_nan_ncols = 1;		/* Default is that single z column */
	C->current.setting.io_nan_mode = 1;	/* Plain -s */
	if (!item || !item[0]) return (FALSE);	/* Nothing more to do */
	n = strlen (item);
	if (item[n-1] == 'a') C->current.setting.io_nan_mode = 3, n--;		/* Set -sa */
	else if (item[n-1] == 'r') C->current.setting.io_nan_mode = 2, n--;	/* Set -sr */
	if (n == 0) return (FALSE);		/* No column arguments to process */
	/* Here we have user-supplied column information */
	for (i = 0; i < GMT_MAX_COLUMNS; i++) tmp[i] = -1;
	while (!error && (GMT_strtok (C, item, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((c = strchr (p, '-')))	/* Range of columns given. e.g., 7-9 */
			sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
		else if (isdigit ((int)p[0]))	/* Just a single column, e.g., 3t */
			start = stop = atoi (p);
		else				/* Unable to decode */
			error++;

		/* Now set the code for these columns */

		for (i = start; i <= stop; i++) tmp[i] = TRUE;
	}
	/* Count and set array of NaN-columns */
	for (i = n = 0; i < GMT_MAX_COLUMNS; i++) if (tmp[i] != -1) C->current.io.io_nan_col[n++] = i;
	if (error || n == 0) {
		GMT_report (C, GMT_MSG_FATAL, "Syntax error -s option: Unable to decode columns from %s\n", item);
		return TRUE;
	}
	C->current.io.io_nan_ncols = n;

	return (FALSE);
}

void gmt_verify_encodings (struct GMT_CTRL *C) {
	/* Check that special map-related codes are present - if not give warning */

	/* First check for degree symbol */

	if (C->current.setting.ps_encoding.code[gmt_ring] == 32 && C->current.setting.ps_encoding.code[gmt_degree] == 32) {	/* Neither /ring or /degree encoded */
		GMT_message (C, "Warning: Selected character encoding does not have suitable degree symbol - will use space instead\n");
	}
	else if (C->current.setting.map_degree_symbol == gmt_ring && C->current.setting.ps_encoding.code[gmt_ring] == 32) {		/* want /ring but only /degree is encoded */
		GMT_message (C, "Warning: Selected character encoding does not have ring symbol - will use degree symbol instead\n");
		C->current.setting.map_degree_symbol = gmt_degree;
	}
	else if (C->current.setting.map_degree_symbol == gmt_degree && C->current.setting.ps_encoding.code[gmt_degree] == 32) {	/* want /degree but only /ring is encoded */
		GMT_message (C, "Warning: Selected character encoding does not have degree symbol - will use ring symbol instead\n");
		C->current.setting.map_degree_symbol = gmt_ring;
	}

	/* Then single quote for minute symbol... */

	if (C->current.setting.map_degree_symbol < 2 && C->current.setting.ps_encoding.code[gmt_squote] == 32) {
		GMT_message (C, "Warning: Selected character encoding does not have minute symbol (single quote) - will use space instead\n");
	}

	/* ... and double quote for second symbol */

	if (C->current.setting.map_degree_symbol < 2 && C->current.setting.ps_encoding.code[gmt_dquote] == 32) {
		GMT_message (C, "Warning: Selected character encoding does not have second symbol (double quote) - will use space instead\n");
	}
}

void gmt_free_hash (struct GMT_CTRL *C, struct GMT_HASH *hashnode, GMT_LONG n_items) {
	GMT_LONG i;
	struct GMT_HASH *p = NULL, *current = NULL;

	/* Erase all the linked nodes from each array position. */
	if (!hashnode) return;	/* Nothing to free */
	for (i = 0; i < n_items; i++) {
		p = hashnode[i].next;
		while (p) {
			current = p;
			p = p->next;
			GMT_free (C, current);
		}
	}
}

GMT_LONG GMT_loaddefaults (struct GMT_CTRL *C, char *file)
{
	GMT_LONG error = 0, rec = 0;
	char line[GMT_BUFSIZ], keyword[GMT_TEXT_LEN256], value[GMT_TEXT_LEN256];
	FILE *fp = NULL;

	if ((fp = fopen (file, "r")) == NULL) return (-1);

	/* Set up hash table */

	GMT_hash_init (C, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);

	while (fgets (line, GMT_BUFSIZ, fp)) {
		rec++;
		GMT_chop (C, line);	/* Get rid of [\r]\n */
		if (rec == 1 && (strlen (line) < 7 || line[6] != '5')) {
			GMT_message (C, "Warning: Your gmt.conf file may not be GMT 5 compatible\n");
		}
		if (line[0] == '#') continue;	/* Skip comments */
		if (line[0] == '\0') continue;	/* Skip Blank lines */

		keyword[0] = value[0] = '\0';	/* Initialize */
		sscanf (line, "%s = %[^\n]", keyword, value);

		error += GMT_setparameter (C, keyword, value);
	}

	fclose (fp);
	gmt_verify_encodings (C);

	gmt_free_hash (C, keys_hashnode, GMT_N_KEYS);	/* Done with this for now */
	if (error) GMT_message (C, "Error: %ld conversion errors in file %s!\n", error, file);

	return (GMT_NOERROR);
}

void GMT_setdefaults (struct GMT_CTRL *C, struct GMT_OPTION *options)
{
	GMT_LONG p, n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	char *param = CNULL;

	/* Set up hash table */

	GMT_hash_init (C, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);

	for (opt = options; opt; opt = opt->next) {
		if (!(opt->option == '<' || opt->option == '#') || !opt->arg) continue;		/* Skip other and empty options */
		if (!strcmp (opt->arg, "=")) continue;			/* User forgot and gave parameter = value (3 words) */
		if (opt->arg[0] != '=' && strchr (opt->arg, '=')) {	/* User forgot and gave parameter=value (1 word) */
			p = 0;
			while (opt->arg[p] && opt->arg[p] != '=') p++;
			opt->arg[p] = '\0';	/* Temporarily remove the equal sign */
			n_errors += GMT_setparameter (C, opt->arg, &opt->arg[p+1]);
			opt->arg[p] = '=';	/* Restore the equal sign */
		}
		else if (!param)			/* Keep parameter name */
			param = opt->arg;
		else {					/* This must be value */
			n_errors += GMT_setparameter (C, param, opt->arg);
			param = CNULL;
		}
	}

	n_errors += (param != CNULL);	/* param should be NULL */

	gmt_free_hash (C, keys_hashnode, GMT_N_KEYS);	/* Done with this for now  */
	if (n_errors) GMT_report (C, GMT_MSG_FATAL, " %ld conversion errors\n", n_errors);
}

void GMT_pickdefaults (struct GMT_CTRL *C, GMT_LONG lines, struct GMT_OPTION *options)
{
	GMT_LONG n = 0;
	struct GMT_OPTION *opt = NULL;

	/* Set up hash table */

	GMT_hash_init (C, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);

	for (opt = options; opt; opt = opt->next) {
		if (!(opt->option == '<' || opt->option == '#') || !opt->arg) continue;		/* Skip other and empty options */
		if (!lines && n) fprintf (C->session.std[GMT_OUT], " ");	/* Separate by spaces */
		fprintf (C->session.std[GMT_OUT], "%s", GMT_putparameter (C, opt->arg));		
		if (lines) fprintf (C->session.std[GMT_OUT], "\n");	/* Separate lines */
		n++;
	}
	if (!lines && n) fprintf (C->session.std[GMT_OUT], "\n");	/* Single lines */

	gmt_free_hash (C, keys_hashnode, GMT_N_KEYS);	/* Done with this for now  */
}

#ifdef GMT_COMPAT
#define GMT_COMPAT_WARN GMT_report (C, GMT_MSG_COMPAT, "Warning: parameter %s is deprecated.\n", GMT_keywords[case_val])
#define GMT_COMPAT_CHANGE(new) GMT_report (C, GMT_MSG_COMPAT, "Warning: parameter %s is deprecated. Use %s instead.\n", GMT_keywords[case_val], new)
#endif

GMT_LONG gmt_true_false_or_error (char *value, GMT_LONG *answer)
{
	/* Assigns 0 or 1 to answer, depending on whether value is false or true.
	 * answer = 0, when value is "f", "false" or "0"
	 * answer = 1, when value is "t", "true" or "1"
	 * In either case, the function returns FALSE as exit code.
	 * When value is something else, answer is not altered and TRUE is return as error.
	 */

	if (!strcmp (value, "true") || !strcmp (value, "t") || !strcmp (value, "1")) {	/* TRUE */
		*answer = 1;
		return (FALSE);
	}
	if (!strcmp (value, "false") || !strcmp (value, "f") || !strcmp (value, "0")) {	/* FALSE */
		*answer = 0;
		return (FALSE);
	}

	/* Got neither true or false.  Make no assignment and return TRUE for error */

	return (TRUE);
}

GMT_LONG gmt_get_time_language (struct GMT_CTRL *C)
{
	FILE *fp = NULL;
	char file[GMT_BUFSIZ], line[GMT_BUFSIZ], full[16], abbrev[16], c[16], dwu;
	char *months[12];

	GMT_LONG i, nm = 0, nw = 0, nu = 0;

	GMT_memset (months, 12, char *);

	GMT_getsharepath (C, "time", C->current.setting.time_language, ".d", file);
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "Warning: Could not load time language %s - revert to us (English)!\n", C->current.setting.time_language);
		GMT_getsharepath (C, "time", "us", ".d", file);
		if ((fp = fopen (file, "r")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Could not find %s!\n", file);
			GMT_exit (EXIT_FAILURE);
		}
		strcpy (C->current.setting.time_language, "us");
	}

	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		sscanf (line, "%c %" GMT_LL "d %s %s %s", &dwu, &i, full, abbrev, c);
		if (dwu == 'M') {	/* Month record */
			strncpy (C->current.time.language.month_name[0][i-1], full, (size_t)16);
			strncpy (C->current.time.language.month_name[1][i-1], abbrev, (size_t)16);
			strncpy (C->current.time.language.month_name[2][i-1], c, (size_t)16);
			GMT_str_toupper(abbrev);
			strncpy (C->current.time.language.month_name[3][i-1], abbrev, (size_t)16);
			nm += i;
		}
		else if (dwu == 'W') {	/* Weekday record */
			strncpy (C->current.time.language.day_name[0][i-1], full, (size_t)16);
			strncpy (C->current.time.language.day_name[1][i-1], abbrev, (size_t)16);
			strncpy (C->current.time.language.day_name[2][i-1], c, (size_t)16);
			nw += i;
		}
		else {			/* Week name record */
			strncpy (C->current.time.language.week_name[0], full, (size_t)16);
			strncpy (C->current.time.language.week_name[1], abbrev, (size_t)16);
			strncpy (C->current.time.language.week_name[2], c, (size_t)16);
			nu += i;
		}
	}
	fclose (fp);
	if (! (nm == 78 && nw == 28 && nu == 1)) {	/* Sums of 1-12, 1-7, and 1, respectively */
		GMT_report (C, GMT_MSG_FATAL, "Error: Mismatch between expected and actual contents in %s!\n", file);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_key_lookup (char *name, char **list, GMT_LONG n)
{
	GMT_LONG i;

	for (i = 0; i < n && strcmp (name, list[i]); i++);
	return (i);
}

void gmt_free_user_media (struct GMT_CTRL *C) {	/* Free any user-specified media formats */
	GMT_LONG i;

	if (C->session.n_user_media == 0) return;	/* Nothing to free */
	
	for (i = 0; i < C->session.n_user_media; i++) free ((void *)C->session.user_media_name[i]);
	GMT_free (C, C->session.user_media_name);
	GMT_free (C, C->session.user_media);
	C->session.n_user_media = 0;
}

GMT_LONG gmt_load_user_media (struct GMT_CTRL *C) {	/* Load any user-specified media formats */
	GMT_LONG n = 0, n_alloc = 0;
	double w, h;
	char line[GMT_BUFSIZ], file[GMT_BUFSIZ], media[GMT_TEXT_LEN64];
	FILE *fp = NULL;

	GMT_getsharepath (C, "conf", "gmt_custom_media", ".conf", file);
	if ((fp = fopen (file, "r")) == NULL) return (0);

	gmt_free_user_media (C);	/* Free any previously allocated user-specified media formats */
	GMT_set_meminc (C, GMT_TINY_CHUNK);	/* Only allocate a small amount */
	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments and blank lines */

		if (sscanf (line, "%s %lg %lg", media, &w, &h) != 3) {
			GMT_report (C, GMT_MSG_FATAL, "Error decoding file %s.  Bad format? [%s]\n", file, line);
			GMT_exit (EXIT_FAILURE);
		}

		GMT_str_tolower (media);	/* Convert string to lower case */

		if (n == n_alloc) {
			(void)GMT_malloc (C, C->session.user_media, n, n_alloc, struct GMT_MEDIA);
			n_alloc = GMT_malloc (C, C->session.user_media_name, n, n_alloc, char *);
		}
		C->session.user_media_name[n] = strdup (media);
		C->session.user_media[n].width  = w;
		C->session.user_media[n].height = h;
		n++;
	}
	fclose (fp);

	(void)GMT_malloc (C, C->session.user_media, 0, n, struct GMT_MEDIA);
	(void)GMT_malloc (C, C->session.user_media_name, 0, n, char *);
	GMT_reset_meminc (C);

	C->session.n_user_media = n;

	return (n);
}

/* Load a PostScript encoding from a file, given the filename.
 * Use Brute Force and Ignorance.
 */
GMT_LONG gmt_load_encoding (struct GMT_CTRL *C)
{
	char line[GMT_TEXT_LEN256], symbol[GMT_TEXT_LEN256];
	GMT_LONG code = 0, pos;
	FILE *in = NULL;
	struct gmt_encoding *enc = &C->current.setting.ps_encoding;

	GMT_getsharepath (C, "pslib", enc->name, ".ps", line);
	if ((in = fopen (line, "r")) == NULL) {
		perror (line);
		GMT_exit (EXIT_FAILURE);
	}

	while (fgets (line, GMT_TEXT_LEN256, in))
	{
		pos = 0;
		while ((GMT_strtok (C, line, " /\t\n", &pos, symbol)))
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

GMT_LONG gmt_decode_wesnz (struct GMT_CTRL *C, const char *in, GMT_LONG side[], GMT_LONG *draw_box) {
	/* Scans the WESNZwesnz+ flags at the end of string "in" and sets the side/drawbox parameters
	 * and returns the length of the remaining string.
	 */

	GMT_LONG i, k, go = TRUE;

	i = strlen (in);
	if (i == 0) return (0);
	
	for (k = 0, i--; go && i >= 0 && strchr ("WESNZwesnz+", in[i]); i--) {
		if (k == 0) {	/* Wipe out default values when the first flag is found */
			for (k = 0; k < 5; k++) side[k] = 0;
			*draw_box = FALSE;
		}
		if (in[i] == 's') {	/* Since s can mean both "draw south axis" and "seconds", check futher */
			if (side[S_SIDE]) go = FALSE;	/* If S was set already then s probably means seconds */
			else if (i && in[i-1] == ',') go = TRUE;	/* Special case of ,s to indicate south, e.g. -B30,s */
			else if (i && (in[i-1] == '.' || isdigit ((int)in[i-1]))) go = FALSE;	/* Probably seconds, e.g. -B30s */
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
			case '+': *draw_box = TRUE; break;
		}
	}
	if (in[i] == ',') i--;	/* Special case for -BCcustomfile,WESNwesn to avoid the filename being parsed for WESN */
	
	return (i+1);	/* Return remaining string length */
}

void gmt_parse_format_float_out (struct GMT_CTRL *C, char *value)
{
	GMT_LONG pos = 0, col = 0, start = 0, stop = 0, k, error = 0;
	char fmt[GMT_TEXT_LEN64], *p = NULL;
	/* Look for multiple comma-separated format statements of type [<cols>:]<format> */
	while ((GMT_strtok (C, value, ",", &pos, fmt))) {
		if ((p = strchr (fmt, ':'))) {	/* Must decode which columns */
			if (strchr (fmt, '-'))	/* Range of columns given. e.g., 7-9 */
				sscanf (fmt, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
			else if (isdigit ((int)fmt[0]))	/* Just a single column, e.g., 3 */
				start = stop = atoi (fmt);
			else				/* Something bad */
				error++;
			p++;	/* Move to format */
			for (k = start; k <= stop; k++, col++) {
				if (C->current.io.o_format[k]) free ((void *)C->current.io.o_format[k]);
				C->current.io.o_format[k] = strdup (p);
			}
		}
		else {	/* No columns, set the default format */
			/* Last format without cols also becomes the default for unspecified columns */
			strcpy (C->current.setting.format_float_out, fmt);
		}
	}
}

GMT_LONG GMT_setparameter (struct GMT_CTRL *C, char *keyword, char *value)
{
	GMT_LONG i, ival, case_val, pos, manual, error = FALSE;
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256], lower_value[GMT_BUFSIZ];
	double dval;

	if (!value) return (TRUE);		/* value argument missing */
	strncpy (lower_value, value, (size_t)GMT_BUFSIZ);	/* Get a lower case version */
	GMT_str_tolower (lower_value);

	case_val = GMT_hash_lookup (C, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		/* FORMAT GROUP */
#ifdef GMT_COMPAT
		case GMTCASE_INPUT_CLOCK_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_CLOCK_IN");
#endif
		case GMTCASE_FORMAT_CLOCK_IN:
			strncpy (C->current.setting.format_clock_in, value, (size_t)GMT_TEXT_LEN64);
			gmt_clock_C_format (C, C->current.setting.format_clock_in, &C->current.io.clock_input, 0);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_INPUT_DATE_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_DATE_IN");
#endif
		case GMTCASE_FORMAT_DATE_IN:
			strncpy (C->current.setting.format_date_in, value, (size_t)GMT_TEXT_LEN64);
			gmt_date_C_format (C, C->current.setting.format_date_in, &C->current.io.date_input, 0);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OUTPUT_CLOCK_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_CLOCK_OUT");
#endif
		case GMTCASE_FORMAT_CLOCK_OUT:
			strncpy (C->current.setting.format_clock_out, value, (size_t)GMT_TEXT_LEN64);
			gmt_clock_C_format (C, C->current.setting.format_clock_out, &C->current.io.clock_output, 1);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OUTPUT_DATE_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_DATE_OUT");
#endif
		case GMTCASE_FORMAT_DATE_OUT:
			strncpy (C->current.setting.format_date_out, value, (size_t)GMT_TEXT_LEN64);
			gmt_date_C_format (C, C->current.setting.format_date_out, &C->current.io.date_output, 1);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OUTPUT_DEGREE_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_GEO_OUT");
#endif
		case GMTCASE_FORMAT_GEO_OUT:
			strncpy (C->current.setting.format_geo_out, value, (size_t)GMT_TEXT_LEN64);
			gmt_geo_C_format (C);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of GMT_begin */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PLOT_CLOCK_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_CLOCK_MAP");
#endif
		case GMTCASE_FORMAT_CLOCK_MAP:
			strncpy (C->current.setting.format_clock_map, value, (size_t)GMT_TEXT_LEN64);
			gmt_clock_C_format (C, C->current.setting.format_clock_map, &C->current.plot.calclock.clock, 2);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PLOT_DATE_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_DATE_MAP");
#endif
		case GMTCASE_FORMAT_DATE_MAP:
			strncpy (C->current.setting.format_date_map, value, (size_t)GMT_TEXT_LEN64);
			gmt_date_C_format (C, C->current.setting.format_date_map, &C->current.plot.calclock.date, 2);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PLOT_DEGREE_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_GEO_MAP");
#endif
		case GMTCASE_FORMAT_GEO_MAP:
			strncpy (C->current.setting.format_geo_map, value, (size_t)GMT_TEXT_LEN64);
			gmt_plot_C_format (C);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of GMT_begin */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TIME_FORMAT_PRIMARY: GMT_COMPAT_CHANGE ("FORMAT_TIME_PRIMARY_MAP");
#endif
		case GMTCASE_FORMAT_TIME_PRIMARY_MAP:
			strncpy (C->current.setting.format_time[0], value, (size_t)GMT_TEXT_LEN64);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TIME_FORMAT_SECONDARY: GMT_COMPAT_CHANGE ("FORMAT_TIME_SECONDARY_MAP");
#endif
		case GMTCASE_FORMAT_TIME_SECONDARY_MAP:
			strncpy (C->current.setting.format_time[1], value, (size_t)GMT_TEXT_LEN64);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_D_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_FLOAT_OUT");
#endif
		case GMTCASE_FORMAT_FLOAT_OUT:
			gmt_parse_format_float_out (C, value);
			break;
		case GMTCASE_FORMAT_FLOAT_MAP:
			strcpy (C->current.setting.format_float_map, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_UNIX_TIME_FORMAT: GMT_COMPAT_CHANGE ("FORMAT_TIME_LOGO");
#endif
		case GMTCASE_FORMAT_TIME_LOGO:
			strncpy (C->current.setting.format_time_logo, value, (size_t)GMT_TEXT_LEN256);
			break;

		/* FONT GROUP */

		case GMTCASE_FONT:	/* Special to set all fonts */
			if (GMT_getfont (C, value, &C->current.setting.font_annot[0])) error = TRUE;
			if (GMT_getfont (C, value, &C->current.setting.font_annot[1])) error = TRUE;
			if (GMT_getfont (C, value, &C->current.setting.font_title)) error = TRUE;
			if (GMT_getfont (C, value, &C->current.setting.font_label)) error = TRUE;
			/* if (GMT_getfont (C, value, &C->current.setting.font_logo)) error = TRUE; */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_FONT_PRIMARY: GMT_COMPAT_CHANGE ("FONT_ANNOT_PRIMARY");
#endif
		case GMTCASE_FONT_ANNOT_PRIMARY:
			if (value[0] == '+') {
				/* When + is prepended, scale fonts, offsets and ticklengths relative to FONT_ANNOT_PRIMARY (except LOGO font) */
				double scale;
				scale = C->current.setting.font_annot[0].size;
				if (GMT_getfont (C, &value[1], &C->current.setting.font_annot[0])) error = TRUE;
				scale = C->current.setting.font_annot[0].size / scale;
				C->current.setting.font_annot[1].size *= scale;
				C->current.setting.font_label.size *= scale;
				C->current.setting.font_title.size *= scale;
				C->current.setting.map_annot_offset[0] *= scale;
				C->current.setting.map_annot_offset[1] *= scale;
				C->current.setting.map_label_offset *= scale;
				C->current.setting.map_title_offset *= scale;
				C->current.setting.map_frame_width *= scale;
				C->current.setting.map_tick_length *= scale;
			}
			else
				if (GMT_getfont (C, value, &C->current.setting.font_annot[0])) error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_FONT_SECONDARY: GMT_COMPAT_CHANGE ("FONT_ANNOT_SECONDARY");
#endif
		case GMTCASE_FONT_ANNOT_SECONDARY:
			if (GMT_getfont (C, value, &C->current.setting.font_annot[1])) error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HEADER_FONT: GMT_COMPAT_CHANGE ("FONT_TITLE");
#endif
		case GMTCASE_FONT_TITLE:
			if (GMT_getfont (C, value, &C->current.setting.font_title)) error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_LABEL_FONT: GMT_COMPAT_CHANGE ("FONT_LABEL");
#endif
		case GMTCASE_FONT_LABEL:
			if (GMT_getfont (C, value, &C->current.setting.font_label)) error = TRUE;
			break;
		case GMTCASE_FONT_LOGO:
			if (GMT_getfont (C, value, &C->current.setting.font_logo)) error = TRUE;
			break;

#ifdef GMT_COMPAT
		/* FONT GROUP ... obsolete options */

		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY: GMT_COMPAT_CHANGE ("FONT_ANNOT_PRIMARY");
			dval = GMT_convert_units (C, value, GMT_PT, GMT_PT);
			if (dval > 0.0)
				C->current.setting.font_annot[0].size = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY: GMT_COMPAT_CHANGE ("FONT_ANNOT_SECONDARY");
			dval = GMT_convert_units (C, value, GMT_PT, GMT_PT);
			if (dval > 0.0)
				C->current.setting.font_annot[1].size = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_HEADER_FONT_SIZE: GMT_COMPAT_CHANGE ("FONT_TITLE");
			dval = GMT_convert_units (C, value, GMT_PT, GMT_PT);
			if (dval > 0.0)
				C->current.setting.font_title.size = dval;
			else
				error = TRUE;
			break;
		case GMTCASE_LABEL_FONT_SIZE: GMT_COMPAT_CHANGE ("FONT_LABEL");
			dval = GMT_convert_units (C, value, GMT_PT, GMT_PT);
			if (dval > 0.0)
				C->current.setting.font_label.size = dval;
			else
				error = TRUE;
			break;
#endif

		/* MAP GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_OFFSET_PRIMARY: GMT_COMPAT_CHANGE ("MAP_ANNOT_OFFSET_PRIMARY");
#endif
		case GMTCASE_MAP_ANNOT_OFFSET_PRIMARY:
			C->current.setting.map_annot_offset[0] = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_OFFSET_SECONDARY: GMT_COMPAT_CHANGE ("MAP_ANNOT_OFFSET_SECONDARY");
#endif
		case GMTCASE_MAP_ANNOT_OFFSET_SECONDARY:
			C->current.setting.map_annot_offset[1] = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OBLIQUE_ANNOTATION: GMT_COMPAT_CHANGE ("MAP_ANNOT_OBLIQUE");
#endif
		case GMTCASE_MAP_ANNOT_OBLIQUE:
			ival = atoi (value);
			if (ival >= 0 && ival < 64)
				C->current.setting.map_annot_oblique = ival;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_MIN_ANGLE: GMT_COMPAT_CHANGE ("MAP_ANNOT_MIN_ANGLE");
#endif
		case GMTCASE_MAP_ANNOT_MIN_ANGLE:
			dval = atof (value);
			if (dval < 0.0)
				error = TRUE;
			else
				C->current.setting.map_annot_min_angle = dval;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_MIN_SPACING: GMT_COMPAT_CHANGE ("MAP_ANNOT_MIN_SPACING");
#endif
		case GMTCASE_MAP_ANNOT_MIN_SPACING:
			if (value[0] == '-')	/* Negative */
				error = TRUE;
			else
				C->current.setting.map_annot_min_spacing = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_Y_AXIS_TYPE: GMT_COMPAT_CHANGE ("MAP_ANNOT_ORTHO");
			if (!strcmp (lower_value, "ver_text"))
				strcpy (C->current.setting.map_annot_ortho, "");
			else if (!strcmp (lower_value, "hor_text"))
				strcpy (C->current.setting.map_annot_ortho, "we");
			else
				error = TRUE;
			break;
#endif
		case GMTCASE_MAP_ANNOT_ORTHO:
			strcpy (C->current.setting.map_annot_ortho, lower_value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_DEGREE_SYMBOL: GMT_COMPAT_CHANGE ("MAP_DEGREE_SYMBOL");
#endif
		case GMTCASE_MAP_DEGREE_SYMBOL:
			if (value[0] == '\0' || !strcmp (lower_value, "ring"))	/* Default */
				C->current.setting.map_degree_symbol = gmt_ring;
			else if (!strcmp (lower_value, "degree"))
				C->current.setting.map_degree_symbol = gmt_degree;
			else if (!strcmp (lower_value, "colon"))
				C->current.setting.map_degree_symbol = gmt_colon;
			else if (!strcmp (lower_value, "none"))
				C->current.setting.map_degree_symbol = gmt_none;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_BASEMAP_AXES: GMT_COMPAT_CHANGE ("MAP_FRAME_AXES");
#endif
		case GMTCASE_MAP_FRAME_AXES:
			strcpy (C->current.setting.map_frame_axes, value);
			for (i = 0; i < 5; i++) C->current.map.frame.side[i] = 0;	/* Unset default settings */
			C->current.map.frame.draw_box = FALSE;
			error += gmt_decode_wesnz (C, value, C->current.map.frame.side, &C->current.map.frame.draw_box);
			break;

#ifdef GMT_COMPAT
		case GMTCASE_BASEMAP_FRAME_RGB: GMT_COMPAT_CHANGE ("MAP_DEFAULT_PEN");
#endif
		case GMTCASE_MAP_DEFAULT_PEN:
			i = (value[0] == '+') ? 1 : 0;	/* If plus is added, copy color to MAP_*_PEN settings */
			error = GMT_getpen (C, &value[i], &C->current.setting.map_default_pen);
			if (i == 1) {
				GMT_rgb_copy (&C->current.setting.map_grid_pen[0].rgb, &C->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&C->current.setting.map_grid_pen[1].rgb, &C->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&C->current.setting.map_frame_pen.rgb  , &C->current.setting.map_default_pen.rgb);
				GMT_rgb_copy (&C->current.setting.map_tick_pen.rgb   , &C->current.setting.map_default_pen.rgb);
			}
			break;
#ifdef GMT_COMPAT
		case GMTCASE_FRAME_PEN: GMT_COMPAT_CHANGE ("MAP_FRAME_PEN");
#endif
		case GMTCASE_MAP_FRAME_PEN:
			error = GMT_getpen (C, value, &C->current.setting.map_frame_pen);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_BASEMAP_TYPE: GMT_COMPAT_CHANGE ("MAP_FRAME_TYPE");
#endif
		case GMTCASE_MAP_FRAME_TYPE:
			if (!strcmp (lower_value, "plain"))
				C->current.setting.map_frame_type = GMT_IS_PLAIN;
			else if (!strcmp (lower_value, "graph"))
				C->current.setting.map_frame_type = GMT_IS_GRAPH;
			else if (!strcmp (lower_value, "fancy"))
				C->current.setting.map_frame_type = GMT_IS_FANCY;
			else if (!strcmp (lower_value, "fancy+"))
				C->current.setting.map_frame_type = GMT_IS_ROUNDED;
			else if (!strcmp (lower_value, "inside"))
				C->current.setting.map_frame_type = GMT_IS_INSIDE;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_FRAME_WIDTH: GMT_COMPAT_CHANGE ("MAP_FRAME_WIDTH");
#endif
		case GMTCASE_MAP_FRAME_WIDTH:
			dval = GMT_to_inch (C, value);
			if (dval > 0.0)
				C->current.setting.map_frame_width = dval;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY: GMT_COMPAT_CHANGE ("MAP_GRID_CROSS_SIZE_PRIMARY");
#endif
		case GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY:
			dval = GMT_to_inch (C, value);
			if (dval >= 0.0)
				C->current.setting.map_grid_cross_size[0] = dval;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY: GMT_COMPAT_CHANGE ("MAP_GRID_CROSS_SIZE_SECONDARY");
#endif
		case GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY:
			dval = GMT_to_inch (C, value);
			if (dval >= 0.0)
				C->current.setting.map_grid_cross_size[1] = dval;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_PEN_PRIMARY: GMT_COMPAT_CHANGE ("MAP_GRID_PEN_PRIMARY");
#endif
		case GMTCASE_MAP_GRID_PEN_PRIMARY:
			error = GMT_getpen (C, value, &C->current.setting.map_grid_pen[0]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_PEN_SECONDARY: GMT_COMPAT_CHANGE ("MAP_GRID_PEN_SECONDARY");
#endif
		case GMTCASE_MAP_GRID_PEN_SECONDARY:
			error = GMT_getpen (C, value, &C->current.setting.map_grid_pen[1]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_LABEL_OFFSET: GMT_COMPAT_CHANGE ("MAP_LABEL_OFFSET");
#endif
		case GMTCASE_MAP_LABEL_OFFSET:
			C->current.setting.map_label_offset = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_LINE_STEP: GMT_COMPAT_CHANGE ("MAP_LINE_STEP");
#endif
		case GMTCASE_MAP_LINE_STEP:
			if ((C->current.setting.map_line_step = GMT_to_inch (C, value)) <= 0.0) {
				C->current.setting.map_line_step = 0.01;
				GMT_report (C, GMT_MSG_FATAL, "Warning: %s <= 0, reset to %g %s\n", keyword, C->current.setting.map_line_step, C->session.unit_name[GMT_INCH]);
			}
			break;
#ifdef GMT_COMPAT
		case GMTCASE_UNIX_TIME: GMT_COMPAT_CHANGE ("MAP_LOGO");
#endif
		case GMTCASE_MAP_LOGO:
			error = gmt_true_false_or_error (lower_value, &C->current.setting.map_logo);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_UNIX_TIME_POS: GMT_COMPAT_CHANGE ("MAP_LOGO_POS");
#endif
		case GMTCASE_MAP_LOGO_POS:
			i = sscanf (value, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (i == 2) {
				C->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (C, txt_a);
				C->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (C, txt_b);
			}
			else if (i == 3) {	/* New style, includes justification, introduced in GMT 4.3.0 */
				C->current.setting.map_logo_justify = GMT_just_decode (C, txt_a, 12);
				C->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (C, txt_b);
				C->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (C, txt_c);
			}
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_X_ORIGIN: GMT_COMPAT_CHANGE ("MAP_ORIGIN_X");
#endif
		case GMTCASE_MAP_ORIGIN_X:
			C->current.setting.map_origin[GMT_X] = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_Y_ORIGIN: GMT_COMPAT_CHANGE ("MAP_ORIGIN_Y");
#endif
		case GMTCASE_MAP_ORIGIN_Y:
			C->current.setting.map_origin[GMT_Y] = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_POLAR_CAP: GMT_COMPAT_CHANGE ("MAP_POLAT_CAP");
#endif
		case GMTCASE_MAP_POLAR_CAP:
			if (!strcmp (lower_value, "none")) {	/* Means reset to no cap -> lat = 90, dlon = 0 */
				C->current.setting.map_polar_cap[0] = 90.0;
				C->current.setting.map_polar_cap[1] = 0.0;
			}
			else {
				double inc[2];
				i = sscanf (lower_value, "%[^/]/%s", txt_a, txt_b);
				if (i != 2) error = TRUE;
				error = GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_a, GMT_IS_LAT, &C->current.setting.map_polar_cap[0]), txt_a);
				error += GMT_getinc (C, txt_b, inc);
				C->current.setting.map_polar_cap[1] = inc[GMT_X];
			}
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			dval = GMT_to_inch (C, value);
			if (dval <= 0.0)
				error = TRUE;
			else
				C->current.setting.map_scale_height = dval;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TICK_LENGTH: GMT_COMPAT_CHANGE ("MAP_TICK_LENGTH");
#endif
		case GMTCASE_MAP_TICK_LENGTH:
			C->current.setting.map_tick_length = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TICK_PEN: GMT_COMPAT_CHANGE ("MAP_TICK_PEN");
#endif
		case GMTCASE_MAP_TICK_PEN:
			error = GMT_getpen (C, value, &C->current.setting.map_tick_pen);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HEADER_OFFSET: GMT_COMPAT_CHANGE ("MAP_TITLE_OFFSET");
#endif
		case GMTCASE_MAP_TITLE_OFFSET:
			C->current.setting.map_title_offset = GMT_to_inch (C, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_VECTOR_SHAPE: GMT_COMPAT_CHANGE ("MAP_VECTOR_SHAPE");
#endif
		case GMTCASE_MAP_VECTOR_SHAPE:
			dval = atof (value);
			if (dval < -2.0 || dval > 2.0)
				error = TRUE;
			else
				C->current.setting.map_vector_shape = dval;
			break;

		/* COLOR GROUP */

		case GMTCASE_COLOR_BACKGROUND:
			error = GMT_getrgb (C, value, C->current.setting.color_patch[GMT_BGD]);
			break;
		case GMTCASE_COLOR_FOREGROUND:
			error = GMT_getrgb (C, value, C->current.setting.color_patch[GMT_FGD]);
			break;
		case GMTCASE_COLOR_MODEL:
			if (!strcmp (lower_value, "none"))
				C->current.setting.color_model = GMT_RGB;
			else if (!strcmp (lower_value, "rgb"))
				C->current.setting.color_model = GMT_RGB + GMT_COLORINT;
			else if (!strcmp (lower_value, "cmyk"))
				C->current.setting.color_model = GMT_CMYK + GMT_COLORINT;
			else if (!strcmp (lower_value, "hsv"))
				C->current.setting.color_model = GMT_HSV + GMT_COLORINT;
#ifdef GMT_COMPAT
			else if (!strcmp (lower_value, "+rgb")) {
				GMT_report (C, GMT_MSG_COMPAT, "warning: COLOR_MODEL = %s is deprecated, use COLOR_MODEL = %s instead\n", value, &lower_value[1]);
				C->current.setting.color_model = GMT_RGB + GMT_COLORINT;
			}
			else if (!strcmp (lower_value, "+cmyk")) {
				GMT_report (C, GMT_MSG_COMPAT, "warning: COLOR_MODEL = %s is deprecated, use COLOR_MODEL = %s instead\n", value, &lower_value[1]);
				C->current.setting.color_model = GMT_CMYK + GMT_COLORINT;
			}
			else if (!strcmp (lower_value, "+hsv")) {
				GMT_report (C, GMT_MSG_COMPAT, "warning: COLOR_MODEL = %s is deprecated, use COLOR_MODEL = %s instead\n", value, &lower_value[1]);
				C->current.setting.color_model = GMT_HSV + GMT_COLORINT;
			}
#endif
			else
				error = TRUE;
			break;
		case GMTCASE_COLOR_NAN:
			error = GMT_getrgb (C, value, C->current.setting.color_patch[GMT_NAN]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MIN_SATURATION: GMT_COMPAT_CHANGE ("COLOR_HSV_MIN_S");
#endif
		case GMTCASE_COLOR_HSV_MIN_S:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				C->current.setting.color_hsv_min_s = dval;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MAX_SATURATION: GMT_COMPAT_CHANGE ("COLOR_HSV_MAX_S");
#endif
		case GMTCASE_COLOR_HSV_MAX_S:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				C->current.setting.color_hsv_max_s = dval;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MIN_VALUE: GMT_COMPAT_CHANGE ("COLOR_HSV_MIN_V");
#endif
		case GMTCASE_COLOR_HSV_MIN_V:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				C->current.setting.color_hsv_min_v = dval;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MAX_VALUE: GMT_COMPAT_CHANGE ("COLOR_HSV_MAX_V");
#endif
		case GMTCASE_COLOR_HSV_MAX_V:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = TRUE;
			else
				C->current.setting.color_hsv_max_v = dval;
			break;

		/* PS GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_CHAR_ENCODING: GMT_COMPAT_CHANGE ("PS_CHAR_ENCODING");
#endif
		case GMTCASE_PS_CHAR_ENCODING:
			strncpy (C->current.setting.ps_encoding.name, value, (size_t)GMT_TEXT_LEN64);
			gmt_load_encoding (C);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PS_COLOR: GMT_COMPAT_CHANGE ("PS_COLOR");
#endif
		case GMTCASE_PS_COLOR_MODEL:
			if (!strcmp (lower_value, "rgb"))
				C->current.setting.ps_color_mode = PSL_RGB;
			else if (!strcmp (lower_value, "cmyk"))
				C->current.setting.ps_color_mode = PSL_CMYK;
			else if (!strcmp (lower_value, "hsv"))
				C->current.setting.ps_color_mode = PSL_HSV;
			else if (!strcmp (lower_value, "gray") || !strcmp (lower_value, "grey"))
				C->current.setting.ps_color_mode = PSL_GRAY;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_N_COPIES:
		case GMTCASE_PS_COPIES: GMT_COMPAT_WARN;
			ival = atoi (value);
			if (ival > 0)
				C->current.setting.ps_copies = ival;
			else
				error = TRUE;
			break;
		case GMTCASE_DOTS_PR_INCH:
		case GMTCASE_PS_DPI: GMT_COMPAT_WARN;
			break;
		case GMTCASE_PS_EPS: GMT_COMPAT_WARN;
			break;
#endif
		case GMTCASE_PS_IMAGE_COMPRESS:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			if (!strcmp (lower_value, "none"))
				C->PSL->internal.compress = PSL_NONE;
			else if (!strcmp (lower_value, "rle"))
				C->PSL->internal.compress = PSL_RLE;
			else if (!strcmp (lower_value, "lzw"))
				C->PSL->internal.compress = PSL_LZW;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_LINE_CAP:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			if (!strcmp (lower_value, "butt"))
				C->PSL->internal.line_cap = PSL_BUTT_CAP;
			else if (!strcmp (lower_value, "round"))
				C->PSL->internal.line_cap = PSL_ROUND_CAP;
			else if (!strcmp (lower_value, "square"))
				C->PSL->internal.line_cap = PSL_SQUARE_CAP;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_LINE_JOIN:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			if (!strcmp (lower_value, "miter"))
				C->PSL->internal.line_join = PSL_MITER_JOIN;
			else if (!strcmp (lower_value, "round"))
				C->PSL->internal.line_join = PSL_ROUND_JOIN;
			else if (!strcmp (lower_value, "bevel"))
				C->PSL->internal.line_join = PSL_BEVEL_JOIN;
			else
				error = TRUE;
			break;
		case GMTCASE_PS_MITER_LIMIT:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			ival = atoi (value);
			if (ival >= 0 && ival <= 180)
				C->PSL->internal.miter_limit = ival;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PAGE_COLOR: GMT_COMPAT_CHANGE ("PS_PAGE_COLOR");
#endif
		case GMTCASE_PS_PAGE_COLOR:
			error = GMT_getrgb (C, value, C->current.setting.ps_page_rgb);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PAGE_ORIENTATION: GMT_COMPAT_CHANGE ("PS_PAGE_ORIENTATION");
#endif
		case GMTCASE_PS_PAGE_ORIENTATION:
			if (!strcmp (lower_value, "landscape"))
				C->current.setting.ps_orientation = PSL_LANDSCAPE;
			else if (!strcmp (lower_value, "portrait"))
				C->current.setting.ps_orientation = PSL_PORTRAIT;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PAPER_MEDIA: GMT_COMPAT_CHANGE ("PS_MEDIA");
#endif
		case GMTCASE_PS_MEDIA:
			manual = FALSE;
			ival = strlen (lower_value) - 1;
			if (lower_value[ival] == '-') {	/* Manual Feed selected */
				lower_value[ival] = '\0';
				manual = TRUE;
			}
#ifdef GMT_COMPAT
			else if (lower_value[ival] == '+') {	/* EPS format selected */
				lower_value[ival] = '\0';
				GMT_report (C, GMT_MSG_COMPAT, "Production of EPS format is no longer supported, remove + after paper size\n");
			}
#endif
			if ((i = gmt_key_lookup (lower_value, GMT_media_name, GMT_N_MEDIA)) < GMT_N_MEDIA) {
				/* Use the specified standard format */
				C->current.setting.ps_media = i;
				C->current.setting.ps_page_size[0] = GMT_media[i].width;
				C->current.setting.ps_page_size[1] = GMT_media[i].height;
			}
			else if (gmt_load_user_media (C) &&
				(i = gmt_key_lookup (lower_value, C->session.user_media_name, C->session.n_user_media)) < C->session.n_user_media) {
				/* Use a user-specified format */
					C->current.setting.ps_media = i + USER_MEDIA_OFFSET;
					C->current.setting.ps_page_size[0] = C->session.user_media[i].width;
					C->current.setting.ps_page_size[1] = C->session.user_media[i].height;
				}
			else {
				/* A custom paper size in W x H points (or in inch/c if units are appended) */
#ifdef GMT_COMPAT
				pos = (strncmp (lower_value, "custom_", (size_t)7) ? 0 : 7);
#else
				pos = 0;
#endif
				GMT_strtok (C, lower_value, "x", &pos, txt_a);	/* Returns width and update pos */
				C->current.setting.ps_page_size[0] = GMT_convert_units (C, txt_a, GMT_PT, GMT_PT);
				GMT_strtok (C, lower_value, "x", &pos, txt_b);	/* Returns height and update pos */
				C->current.setting.ps_page_size[1] = GMT_convert_units (C, txt_b, GMT_PT, GMT_PT);
				if (C->current.setting.ps_page_size[0] <= 0.0) error++;
				if (C->current.setting.ps_page_size[1] <= 0.0) error++;
				C->current.setting.ps_media = -USER_MEDIA_OFFSET;
			}
			if (!error && manual) C->current.setting.ps_page_size[0] = -C->current.setting.ps_page_size[0];
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GLOBAL_X_SCALE: GMT_COMPAT_CHANGE ("PS_SCALE_X");
#endif
		case GMTCASE_PS_SCALE_X:
			dval = atof (value);
			if (dval > 0.0)
				C->current.setting.ps_magnify[GMT_X] = dval;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GLOBAL_Y_SCALE: GMT_COMPAT_CHANGE ("PS_SCALE_X");
#endif
		case GMTCASE_PS_SCALE_Y:
			dval = atof (value);
			if (dval > 0.0)
				C->current.setting.ps_magnify[GMT_Y] = dval;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TRANSPARENCY:
			GMT_report (C, GMT_MSG_FATAL, "Transparency is now part of pen and fill specifications.  TRANSPARENCY is ignored\n");
			break;
#endif
		case GMTCASE_PS_TRANSPARENCY:
			strncpy (C->current.setting.ps_transpmode, value, 15);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PS_VERBOSE: GMT_COMPAT_CHANGE ("PS_VERBOSE");
#endif
		case GMTCASE_PS_COMMENTS:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			error = gmt_true_false_or_error (lower_value, &C->PSL->internal.comments);
			break;

		/* IO GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_FIELD_DELIMITER: GMT_COMPAT_CHANGE ("IO_COL_SEPARATOR");
#endif
		case GMTCASE_IO_COL_SEPARATOR:
			if (value[0] == '\0' || !strcmp (lower_value, "tab"))	/* DEFAULT */
				strncpy (C->current.setting.io_col_separator, "\t", (size_t)8);
			else if (!strcmp (lower_value, "space"))
				strncpy (C->current.setting.io_col_separator, " ", (size_t)8);
			else if (!strcmp (lower_value, "comma"))
				strncpy (C->current.setting.io_col_separator, ",", (size_t)8);
			else if (!strcmp (lower_value, "none"))
				C->current.setting.io_col_separator[0] = 0;
			else
				strncpy (C->current.setting.io_col_separator, value, (size_t)8);
			C->current.setting.io_col_separator[7] = 0;	/* Just a precaution */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRIDFILE_FORMAT: GMT_COMPAT_CHANGE ("IO_GRIDFILE_FORMAT");
#endif
		case GMTCASE_IO_GRIDFILE_FORMAT:
			strcpy (C->current.setting.io_gridfile_format, value);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRIDFILE_SHORTHAND: GMT_COMPAT_CHANGE ("IO_GRIDFILE_SHORTHAND");
#endif
		case GMTCASE_IO_GRIDFILE_SHORTHAND:
			error = gmt_true_false_or_error (lower_value, &C->current.setting.io_gridfile_shorthand);
			break;
		case GMTCASE_IO_HEADER:
			error = gmt_true_false_or_error (lower_value, &C->current.setting.io_header[GMT_IN]);
			C->current.setting.io_header[GMT_OUT] = C->current.setting.io_header[GMT_IN];
			break;
#ifdef GMT_COMPAT
		case GMTCASE_N_HEADER_RECS: GMT_COMPAT_CHANGE ("IO_N_HEADER_ITEMS");
#endif
		case GMTCASE_IO_N_HEADER_RECS:
			ival = atoi (value);
			if (ival < 0)
				error = TRUE;
			else
				C->current.setting.io_n_header_items = ival;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_NAN_RECORDS: GMT_COMPAT_CHANGE ("IO_NAN_RECORDS");
#endif
		case GMTCASE_IO_NAN_RECORDS:
			if (!strcmp (lower_value, "pass"))
				C->current.setting.io_nan_records = TRUE;
			else if (!strcmp (lower_value, "skip"))
				C->current.setting.io_nan_records = FALSE;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_XY_TOGGLE: GMT_COMPAT_CHANGE ("IO_LONLAT_TOGGLE");
#endif
		case GMTCASE_IO_LONLAT_TOGGLE:
			if (!gmt_true_false_or_error (lower_value, &C->current.setting.io_lonlat_toggle[GMT_IN]))
				/* We got false/f/0 or true/t/1. Set outgoing setting to the same as the ingoing. */
				C->current.setting.io_lonlat_toggle[GMT_OUT] = C->current.setting.io_lonlat_toggle[GMT_IN];
			else if (!strcmp (lower_value, "in")) {
				C->current.setting.io_lonlat_toggle[GMT_IN] = TRUE;
				C->current.setting.io_lonlat_toggle[GMT_OUT] = FALSE;
			}
			else if (!strcmp (lower_value, "out")) {
				C->current.setting.io_lonlat_toggle[GMT_IN] = FALSE;
				C->current.setting.io_lonlat_toggle[GMT_OUT] = TRUE;
			}
			else
				error = TRUE;
			break;

		case GMTCASE_IO_SEGMENT_MARKER:
			if (strlen (value) == 0)
				C->current.setting.io_seg_marker[GMT_OUT] = C->current.setting.io_seg_marker[GMT_IN] = '>';
			else if (strlen (value) == 1)
				C->current.setting.io_seg_marker[GMT_OUT] = C->current.setting.io_seg_marker[GMT_IN] = value[0];
			else if (strlen (value) == 2) {
				C->current.setting.io_seg_marker[GMT_IN]  = value[0];
				C->current.setting.io_seg_marker[GMT_OUT] = value[1];
			}
			else if (strlen (value) == 3 && value[1] == ',') {
				C->current.setting.io_seg_marker[GMT_IN]  = value[0];
				C->current.setting.io_seg_marker[GMT_OUT] = value[2];
			}
			else
				error = TRUE;
			break;

		/* PROJ GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_ELLIPSOID: GMT_COMPAT_CHANGE ("PROJ_ELLIPSOID");
#endif
		case GMTCASE_PROJ_ELLIPSOID:
			ival = GMT_get_ellipsoid (C, value);
			if (ival < 0)
				error = TRUE;
			else
				C->current.setting.proj_ellipsoid = ival;
			GMT_init_ellipsoid (C);	/* Set parameters depending on the ellipsoid */
			break;
		case GMTCASE_PROJ_DATUM:	/* Not implemented yet */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_MEASURE_UNIT: GMT_COMPAT_CHANGE ("PROJ_LENGTH_UNIT");
#endif
		case GMTCASE_PROJ_LENGTH_UNIT:
			if (lower_value[0] == 'c')
				C->current.setting.proj_length_unit = GMT_CM;
			else if (lower_value[0] == 'i')
				C->current.setting.proj_length_unit = GMT_INCH;
			else if (lower_value[0] == 'p')
				C->current.setting.proj_length_unit = GMT_PT;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_MAP_SCALE_FACTOR: GMT_COMPAT_CHANGE ("PROJ_SCALE_FACTOR");
#endif
		case GMTCASE_PROJ_SCALE_FACTOR:
			if (!strncmp (value, "def", (size_t)3)) /* Default scale for chosen projection */
				C->current.setting.proj_scale_factor = -1.0;
			else {
				dval = atof (value);
				if (dval <= 0.0)
					error = TRUE;
				else
					C->current.setting.proj_scale_factor = dval;
			}
			break;

		/* GMT GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_HISTORY: GMT_COMPAT_CHANGE ("GMT_HISTORY");
#endif
		case GMTCASE_GMT_HISTORY:
			error = gmt_true_false_or_error (lower_value, &C->current.setting.history);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_INTERPOLANT: GMT_COMPAT_CHANGE ("GMT_INTERPOLANT");
#endif
		case GMTCASE_GMT_INTERPOLANT:
			if (!strcmp (lower_value, "linear"))
				C->current.setting.interpolant = 0;
			else if (!strcmp (lower_value, "akima"))
				C->current.setting.interpolant = 1;
			else if (!strcmp (lower_value, "cubic"))
				C->current.setting.interpolant = 2;
			else if (!strcmp (lower_value, "none"))
				C->current.setting.interpolant = 3;
			else
				error = TRUE;
			break;
#ifdef GMT_COMPAT
		case GMTCASE_VERBOSE: GMT_COMPAT_CHANGE ("GMT_VERBOSE");
			ival = atoi (value) + 1;
			if (ival >= GMT_MSG_SILENCE && ival <= GMT_MSG_DEBUG)
				C->current.setting.verbose = ival;
			else
				error = TRUE;
			break;
#endif
		case GMTCASE_GMT_VERBOSE:
			ival = atoi (value);
			if (ival >= GMT_MSG_SILENCE && ival <= GMT_MSG_DEBUG)
				C->current.setting.verbose = ival;
			else
				error = TRUE;
			break;

		/* DIR GROUP */

		case GMTCASE_DIR_TMP:
			if (value[0]) {	/* Replace the session temp dir from the environment, if any */
				if (C->session.TMPDIR) free ((void *)C->session.TMPDIR);
				C->session.TMPDIR = strdup (value);
			}
			break;
		case GMTCASE_DIR_USER:
			if (value[0]) {	/* Replace the session user dir from the environment, if any */
				if (C->session.USERDIR) free ((void *)C->session.USERDIR);
				C->session.USERDIR = strdup (value);
			}
			break;

		/* TIME GROUP */

		case GMTCASE_TIME_EPOCH:
			strncpy (C->current.setting.time_system.epoch, value, (size_t)GMT_TEXT_LEN64);
			(void) GMT_init_time_system_structure (C, &C->current.setting.time_system);
			break;
		case GMTCASE_TIME_IS_INTERVAL:
			if (value[0] == '+' || value[0] == '-') {	/* OK, gave +<n>u or -<n>u, check for unit */
				sscanf (&lower_value[1], "%" GMT_LL "d%c", &C->current.time.truncate.T.step, &C->current.time.truncate.T.unit);
				switch (C->current.time.truncate.T.unit) {
					case 'y':
					case 'o':
					case 'd':
					case 'h':
					case 'm':
					case 'c':
						C->current.time.truncate.direction = (lower_value[0] == '+') ? 0 : 1;
						break;
					default:
						error = TRUE;
						break;
				}
				if (C->current.time.truncate.T.step == 0) error = TRUE;
				C->current.setting.time_is_interval = TRUE;
			}
			else if (!strcmp (lower_value, "off"))
				C->current.setting.time_is_interval = FALSE;
			else
				error = TRUE;
			break;
		case GMTCASE_TIME_INTERVAL_FRACTION:
			C->current.setting.time_interval_fraction = atof (value);
			break;
		case GMTCASE_TIME_LANGUAGE:
			strncpy (C->current.setting.time_language, lower_value, (size_t)GMT_TEXT_LEN64);
			gmt_get_time_language (C);	/* Load in names and abbreviations of time units in chosen language */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_WANT_LEAP_SECONDS: GMT_COMPAT_CHANGE ("TIME_LEAP_SECONDS");
#endif
		case GMTCASE_TIME_LEAP_SECONDS:
			error = gmt_true_false_or_error (lower_value, &C->current.setting.time_leap_seconds);
			break;
		case GMTCASE_TIME_UNIT:
			C->current.setting.time_system.unit = lower_value[0];
			(void) GMT_init_time_system_structure (C, &C->current.setting.time_system);
			break;
		case GMTCASE_TIME_SYSTEM:
			error = GMT_get_time_system (C, lower_value, &C->current.setting.time_system);
			(void) GMT_init_time_system_structure (C, &C->current.setting.time_system);
			break;
		case GMTCASE_TIME_WEEK_START:
			C->current.setting.time_week_start = gmt_key_lookup (value, GMT_weekdays, 7);
			if (C->current.setting.time_week_start < 0 || C->current.setting.time_week_start >= 7) {
				error = TRUE;
				C->current.setting.time_week_start = 0;
			}
			break;
#ifdef GMT_COMPAT
		case GMTCASE_Y2K_OFFSET_YEAR: GMT_COMPAT_CHANGE ("TIME_Y2K_OFFSET_YEAR");
#endif
		case GMTCASE_TIME_Y2K_OFFSET_YEAR:
			if ((C->current.setting.time_Y2K_offset_year = atoi (value)) < 0) error = TRUE;
			/* Set the Y2K conversion parameters */
			C->current.time.Y2K_fix.y2_cutoff = GMT_abs (C->current.setting.time_Y2K_offset_year) % 100;
			C->current.time.Y2K_fix.y100 = C->current.setting.time_Y2K_offset_year - C->current.time.Y2K_fix.y2_cutoff;
			C->current.time.Y2K_fix.y200 = C->current.time.Y2K_fix.y100 + 100;
			break;

#ifdef GMT_COMPAT
		/* Obsolete */

		case GMTCASE_PS_IMAGE_FORMAT:
			/* Setting ignored, now always ASCII85 encoding */
		case GMTCASE_X_AXIS_LENGTH:
		case GMTCASE_Y_AXIS_LENGTH:
			/* Setting ignored: x- and/or y scale are required inputs on -J option */
		case GMTCASE_COLOR_IMAGE:
			GMT_COMPAT_WARN;
			/* Setting ignored, now always adobe image */
			break;
#endif

		default:
			error = TRUE;
			GMT_report (C, GMT_MSG_FATAL, "Syntax error in GMT_setparameter: Unrecognized keyword %s\n", keyword);
			break;
	}

	if ((i = strlen(value))) C->current.setting.given_unit[case_val] = value[i-1];

	if (error && case_val >= 0) GMT_report (C, GMT_MSG_FATAL, "Syntax error: %s given illegal value (%s)!\n", keyword, value);
	return (error);
}

char *GMT_putparameter (struct GMT_CTRL *C, char *keyword)
{	/* value must hold at least GMT_BUFSIZ chars */
	static char value[GMT_TEXT_LEN256];
	GMT_LONG case_val, error = FALSE;
	char pm[2] = {'+', '-'}, *ft[2] = {"false", "true"};
	
	GMT_memset (value, GMT_TEXT_LEN256, char);
	if (!keyword) return (value);		/* keyword argument missing */

	case_val = GMT_hash_lookup (C, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		/* FORMAT GROUP */
#ifdef GMT_COMPAT
		case GMTCASE_INPUT_CLOCK_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_CLOCK_IN:
			strcpy (value, C->current.setting.format_clock_in);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_INPUT_DATE_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_DATE_IN:
			strcpy (value, C->current.setting.format_date_in);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OUTPUT_CLOCK_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_CLOCK_OUT:
			strcpy (value, C->current.setting.format_clock_out);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OUTPUT_DATE_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_DATE_OUT:
			strcpy (value, C->current.setting.format_date_out);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OUTPUT_DEGREE_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_GEO_OUT:
			strcpy (value, C->current.setting.format_geo_out);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PLOT_CLOCK_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_CLOCK_MAP:
			strcpy (value, C->current.setting.format_clock_map);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PLOT_DATE_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_DATE_MAP:
			strcpy (value, C->current.setting.format_date_map);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PLOT_DEGREE_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_GEO_MAP:
			strcpy (value, C->current.setting.format_geo_map);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TIME_FORMAT_PRIMARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_TIME_PRIMARY_MAP:
			strcpy (value, C->current.setting.format_time[0]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TIME_FORMAT_SECONDARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_TIME_SECONDARY_MAP:
			strcpy (value, C->current.setting.format_time[1]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_D_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_FLOAT_OUT:
			strcpy (value, C->current.setting.format_float_out);
			break;
		case GMTCASE_FORMAT_FLOAT_MAP:
			strcpy (value, C->current.setting.format_float_map);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_UNIX_TIME_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FORMAT_TIME_LOGO:
			strcpy (value, C->current.setting.format_time_logo);
			break;

		/* FONT GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_FONT_PRIMARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FONT_ANNOT_PRIMARY:
			strcpy (value, GMT_putfont (C, C->current.setting.font_annot[0]));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_FONT_SECONDARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FONT_ANNOT_SECONDARY:
			strcpy (value, GMT_putfont (C, C->current.setting.font_annot[1]));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HEADER_FONT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FONT_TITLE:
			strcpy (value, GMT_putfont (C, C->current.setting.font_title));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_LABEL_FONT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_FONT_LABEL:
			strcpy (value, GMT_putfont (C, C->current.setting.font_label));
			break;

		case GMTCASE_FONT_LOGO:
			strcpy (value, GMT_putfont (C, C->current.setting.font_logo));
			break;

#ifdef GMT_COMPAT
		/* FONT GROUP ... obsolete options */

		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY: GMT_COMPAT_WARN;
			sprintf (value, "%g", C->current.setting.font_annot[0].size);
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY: GMT_COMPAT_WARN;
			sprintf (value, "%g", C->current.setting.font_annot[1].size);
			break;
		case GMTCASE_HEADER_FONT_SIZE: GMT_COMPAT_WARN;
			sprintf (value, "%g", C->current.setting.font_title.size);
			break;
		case GMTCASE_LABEL_FONT_SIZE: GMT_COMPAT_WARN;
			sprintf (value, "%g", C->current.setting.font_label.size);
			break;
#endif

		/* MAP GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_OFFSET_PRIMARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ANNOT_OFFSET_PRIMARY:
			sprintf (value, "%g%c", C->current.setting.map_annot_offset[0] GMT_def(GMTCASE_MAP_ANNOT_OFFSET_PRIMARY));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_OFFSET_SECONDARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ANNOT_OFFSET_SECONDARY:
			sprintf (value, "%g%c", C->current.setting.map_annot_offset[1] GMT_def(GMTCASE_MAP_ANNOT_OFFSET_SECONDARY));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_OBLIQUE_ANNOTATION: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ANNOT_OBLIQUE:
			sprintf (value, "%ld", C->current.setting.map_annot_oblique);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_MIN_ANGLE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ANNOT_MIN_ANGLE:
			sprintf (value, "%g", C->current.setting.map_annot_min_angle);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_ANNOT_MIN_SPACING: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ANNOT_MIN_SPACING:
			sprintf (value, "%g%c", C->current.setting.map_annot_min_spacing GMT_def(GMTCASE_MAP_ANNOT_MIN_SPACING));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_Y_AXIS_TYPE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ANNOT_ORTHO:
			strcpy (value, C->current.setting.map_annot_ortho);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_DEGREE_SYMBOL: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_DEGREE_SYMBOL:
			if (C->current.setting.map_degree_symbol == gmt_ring)	/* Default */
				strcpy (value, "ring");
			else if (C->current.setting.map_degree_symbol == gmt_degree)
				strcpy (value, "degree");
			else if (C->current.setting.map_degree_symbol == gmt_colon)
				strcpy (value, "colon");
			else if (C->current.setting.map_degree_symbol == gmt_none)
				strcpy (value, "none");
			else
				strcpy (value, "undefined");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_BASEMAP_AXES: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_FRAME_AXES:
			strcpy (value, C->current.setting.map_frame_axes);
			break;

#ifdef GMT_COMPAT
		case GMTCASE_BASEMAP_FRAME_RGB: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_DEFAULT_PEN:
			sprintf (value, "%s", GMT_putpen (C, C->current.setting.map_default_pen));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_FRAME_PEN: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_FRAME_PEN:
			sprintf (value, "%s", GMT_putpen (C, C->current.setting.map_frame_pen));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_BASEMAP_TYPE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_FRAME_TYPE:
			if (C->current.setting.map_frame_type == GMT_IS_PLAIN)
				strcpy (value, "plain");
			else if (C->current.setting.map_frame_type == GMT_IS_GRAPH)
				strcpy (value, "graph");
			else if (C->current.setting.map_frame_type == GMT_IS_FANCY)
				strcpy (value, "fancy");
			else if (C->current.setting.map_frame_type == GMT_IS_ROUNDED)
				strcpy (value, "fancy+");
			else if (C->current.setting.map_frame_type == GMT_IS_INSIDE)
				strcpy (value, "inside");
			else
				strcpy (value, "undefined");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_FRAME_WIDTH: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_FRAME_WIDTH:
			sprintf (value, "%g%c", C->current.setting.map_frame_width GMT_def(GMTCASE_MAP_FRAME_WIDTH));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY:
			sprintf (value, "%g%c", C->current.setting.map_grid_cross_size[0] GMT_def(GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY:
			sprintf (value, "%g%c", C->current.setting.map_grid_cross_size[1] GMT_def(GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_PEN_PRIMARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_GRID_PEN_PRIMARY:
			sprintf (value, "%s", GMT_putpen (C, C->current.setting.map_grid_pen[0]));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRID_PEN_SECONDARY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_GRID_PEN_SECONDARY:
			sprintf (value, "%s", GMT_putpen (C, C->current.setting.map_grid_pen[1]));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_LABEL_OFFSET: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_LABEL_OFFSET:
			sprintf (value, "%g%c", C->current.setting.map_label_offset GMT_def(GMTCASE_MAP_LABEL_OFFSET));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_LINE_STEP: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_LINE_STEP:
			sprintf (value, "%g%c", C->current.setting.map_line_step GMT_def(GMTCASE_MAP_LINE_STEP));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_UNIX_TIME: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_LOGO:
			sprintf (value, "%s", ft[C->current.setting.map_logo]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_UNIX_TIME_POS: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_LOGO_POS:
			sprintf (value, "%s/%g%c/%g%c", GMT_just_string[C->current.setting.map_logo_justify],
			C->current.setting.map_logo_pos[GMT_X] GMT_def(GMTCASE_MAP_LOGO_POS),
			C->current.setting.map_logo_pos[GMT_Y] GMT_def(GMTCASE_MAP_LOGO_POS));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_X_ORIGIN: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ORIGIN_X:
			sprintf (value, "%g%c", C->current.setting.map_origin[GMT_X] GMT_def(GMTCASE_MAP_ORIGIN_X));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_Y_ORIGIN: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_ORIGIN_Y:
			sprintf (value, "%g%c", C->current.setting.map_origin[GMT_Y] GMT_def(GMTCASE_MAP_ORIGIN_Y));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_POLAR_CAP: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_POLAR_CAP:
			if (GMT_IS_ZERO (C->current.setting.map_polar_cap[0] - 90.0))
				sprintf (value, "none");
			else
				sprintf (value, "%g/%g", C->current.setting.map_polar_cap[0], C->current.setting.map_polar_cap[1]);
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			sprintf (value, "%g%c", C->current.setting.map_scale_height GMT_def(GMTCASE_MAP_SCALE_HEIGHT));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TICK_LENGTH: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_TICK_LENGTH:
			sprintf (value, "%g%c", C->current.setting.map_tick_length GMT_def(GMTCASE_MAP_TICK_LENGTH));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TICK_PEN: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_TICK_PEN:
			sprintf (value, "%s", GMT_putpen (C, C->current.setting.map_tick_pen));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HEADER_OFFSET: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_TITLE_OFFSET:
			sprintf (value, "%g%c", C->current.setting.map_title_offset GMT_def(GMTCASE_MAP_TITLE_OFFSET));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_VECTOR_SHAPE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_MAP_VECTOR_SHAPE:
			sprintf (value, "%g", C->current.setting.map_vector_shape);
			break;

		/* COLOR GROUP */

		case GMTCASE_COLOR_BACKGROUND:
			sprintf (value, "%s", GMT_putcolor (C, C->current.setting.color_patch[GMT_BGD]));
			break;
		case GMTCASE_COLOR_FOREGROUND:
			sprintf (value, "%s", GMT_putcolor (C, C->current.setting.color_patch[GMT_FGD]));
			break;
		case GMTCASE_COLOR_MODEL:
			if (C->current.setting.color_model == GMT_RGB)
				strcpy (value, "none");
			else if (C->current.setting.color_model == (GMT_RGB + GMT_COLORINT))
				strcpy (value, "rgb");
			else if (C->current.setting.color_model == (GMT_CMYK + GMT_COLORINT))
				strcpy (value, "cmyk");
			else if (C->current.setting.color_model == (GMT_HSV + GMT_COLORINT))
				strcpy (value, "hsv");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_COLOR_NAN:
			sprintf (value, "%s", GMT_putcolor (C, C->current.setting.color_patch[GMT_NAN]));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MIN_SATURATION: GMT_COMPAT_WARN;
#endif
		case GMTCASE_COLOR_HSV_MIN_S:
			sprintf (value, "%g", C->current.setting.color_hsv_min_s);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MAX_SATURATION: GMT_COMPAT_WARN;
#endif
		case GMTCASE_COLOR_HSV_MAX_S:
			sprintf (value, "%g", C->current.setting.color_hsv_max_s);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MIN_VALUE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_COLOR_HSV_MIN_V:
			sprintf (value, "%g", C->current.setting.color_hsv_min_v);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_HSV_MAX_VALUE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_COLOR_HSV_MAX_V:
			sprintf (value, "%g", C->current.setting.color_hsv_max_v);
			break;

		/* PS GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_CHAR_ENCODING: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_CHAR_ENCODING:
			strcpy (value, C->current.setting.ps_encoding.name);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PS_COLOR: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_COLOR_MODEL:
			if (C->current.setting.ps_color_mode == PSL_RGB)
				strcpy (value, "rgb");
			else if (C->current.setting.ps_color_mode == PSL_CMYK)
				strcpy (value, "cmyk");
			else if (C->current.setting.ps_color_mode == PSL_HSV)
				strcpy (value, "hsv");
			else if (C->current.setting.ps_color_mode == PSL_GRAY)
				strcpy (value, "gray");
			else
				strcpy (value, "undefined");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_N_COPIES:
		case GMTCASE_PS_COPIES: GMT_COMPAT_WARN;
			sprintf (value, "%ld", C->current.setting.ps_copies);
			break;
		case GMTCASE_DOTS_PR_INCH:
		case GMTCASE_PS_DPI: GMT_COMPAT_WARN;
			break;
		case GMTCASE_PS_EPS: GMT_COMPAT_WARN;
			break;
#endif
		case GMTCASE_PS_IMAGE_COMPRESS:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			if (C->PSL->internal.compress == PSL_NONE)
				strcpy (value, "none");
			else if (C->PSL->internal.compress == PSL_RLE)
				strcpy (value, "rle");
			else if (C->PSL->internal.compress == PSL_LZW)
				strcpy (value, "lzw");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PS_LINE_CAP:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			if (C->PSL->internal.line_cap == PSL_BUTT_CAP)
				strcpy (value, "butt");
			else if (C->PSL->internal.line_cap == PSL_ROUND_CAP)
				strcpy (value, "round");
			else if (C->PSL->internal.line_cap == PSL_SQUARE_CAP)
				strcpy (value, "square");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PS_LINE_JOIN:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			if (C->PSL->internal.line_join == PSL_MITER_JOIN)
				strcpy (value, "miter");
			else if (C->PSL->internal.line_join == PSL_ROUND_JOIN)
				strcpy (value, "round");
			else if (C->PSL->internal.line_join == PSL_BEVEL_JOIN)
				strcpy (value, "bevel");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PS_MITER_LIMIT:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			sprintf (value, "%ld", C->PSL->internal.miter_limit);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PAGE_COLOR: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_PAGE_COLOR:
			sprintf (value, "%s", GMT_putcolor (C, C->current.setting.ps_page_rgb));
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PAGE_ORIENTATION: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_PAGE_ORIENTATION:
			if (C->current.setting.ps_orientation == PSL_LANDSCAPE)
				strcpy (value, "landscape");
			else if (C->current.setting.ps_orientation == PSL_PORTRAIT)
				strcpy (value, "portrait");
			else
				strcpy (value, "undefined");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PAPER_MEDIA: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_MEDIA:
			if (C->current.setting.ps_media == -USER_MEDIA_OFFSET)
				sprintf (value, "%gx%g", fabs (C->current.setting.ps_page_size[0]), fabs (C->current.setting.ps_page_size[1]));
			else if (C->current.setting.ps_media >= USER_MEDIA_OFFSET)
				sprintf (value, "%s", C->session.user_media_name[C->current.setting.ps_media-USER_MEDIA_OFFSET]);
			else
				sprintf (value, "%s", GMT_media_name[C->current.setting.ps_media]);
			if (C->current.setting.ps_page_size[0] < 0.0)
				strcat (value, "-");
			else if (C->current.setting.ps_page_size[1] < 0.0)
				strcat (value, "+");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GLOBAL_X_SCALE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_SCALE_X:
			sprintf (value, "%g", C->current.setting.ps_magnify[GMT_X]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GLOBAL_Y_SCALE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_SCALE_Y:
			sprintf (value, "%g", C->current.setting.ps_magnify[GMT_Y]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_TRANSPARENCY:
			GMT_report (C, GMT_MSG_FATAL, "Transparency is now part of pen and fill specifications.  TRANSPARENCY is ignored\n");
			break;
#endif
		case GMTCASE_PS_TRANSPARENCY:
			strncpy (value, C->current.setting.ps_transpmode, 15);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_PS_VERBOSE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PS_COMMENTS:
			if (!C->PSL) return (GMT_NOERROR);	/* Not using PSL in this session */
			sprintf (value, "%s", ft[C->PSL->internal.comments]);
			break;

		/* IO GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_FIELD_DELIMITER: GMT_COMPAT_WARN;
#endif
		case GMTCASE_IO_COL_SEPARATOR:
			if (C->current.setting.io_col_separator[0] == '\t')	/* DEFAULT */
				strcpy (value, "tab");
			else if (C->current.setting.io_col_separator[0] == ' ')
				strcpy (value, "space");
			else if (C->current.setting.io_col_separator[0] == ',')
				strcpy (value, "comma");
			else if (!C->current.setting.io_col_separator[0])
				strcpy (value, "none");
			else
				strcpy (value, C->current.setting.io_col_separator);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRIDFILE_FORMAT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_IO_GRIDFILE_FORMAT:
			strcpy (value, C->current.setting.io_gridfile_format);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_GRIDFILE_SHORTHAND: GMT_COMPAT_WARN;
#endif
		case GMTCASE_IO_GRIDFILE_SHORTHAND:
			sprintf (value, "%s", ft[C->current.setting.io_gridfile_shorthand]);
			break;
		case GMTCASE_IO_HEADER:
			sprintf (value, "%s", ft[C->current.setting.io_header[GMT_IN]]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_N_HEADER_RECS: GMT_COMPAT_WARN;
#endif
		case GMTCASE_IO_N_HEADER_RECS:
			sprintf (value, "%ld", C->current.setting.io_n_header_items);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_NAN_RECORDS: GMT_COMPAT_WARN;
#endif
		case GMTCASE_IO_NAN_RECORDS:
			if (C->current.setting.io_nan_records)
				strcpy (value, "pass");
			else 
				strcpy (value, "skip");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_XY_TOGGLE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_IO_LONLAT_TOGGLE:
			if (C->current.setting.io_lonlat_toggle[GMT_IN] && C->current.setting.io_lonlat_toggle[GMT_OUT])
				strcpy (value, "true");
			else if (!C->current.setting.io_lonlat_toggle[GMT_IN] && !C->current.setting.io_lonlat_toggle[GMT_OUT])
				strcpy (value, "false");
			else if (C->current.setting.io_lonlat_toggle[GMT_IN] && !C->current.setting.io_lonlat_toggle[GMT_OUT])
				strcpy (value, "in");
			else
				strcpy (value, "out");
			break;

		case GMTCASE_IO_SEGMENT_MARKER:
			if (C->current.setting.io_seg_marker[GMT_OUT] != C->current.setting.io_seg_marker[GMT_IN])
				sprintf (value, "%c,%c", C->current.setting.io_seg_marker[GMT_IN], C->current.setting.io_seg_marker[GMT_OUT]);
			else
				sprintf (value, "%c", C->current.setting.io_seg_marker[GMT_IN]);
			break;

		/* PROJ GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_ELLIPSOID: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PROJ_ELLIPSOID:
			if (C->current.setting.proj_ellipsoid < GMT_N_ELLIPSOIDS - 1)	/* Custom ellipse */
				sprintf (value, "%s", C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].name);
			else if (GMT_IS_SPHERICAL (C))
				sprintf (value, "%f", C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius);
			else
				sprintf (value, "%f,%f", C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius,
					1.0/C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening);
			break;
		case GMTCASE_PROJ_DATUM:	/* Not implemented yet */
			break;
#ifdef GMT_COMPAT
		case GMTCASE_MEASURE_UNIT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PROJ_LENGTH_UNIT:
			sprintf (value, "%s", C->session.unit_name[C->current.setting.proj_length_unit]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_MAP_SCALE_FACTOR: GMT_COMPAT_WARN;
#endif
		case GMTCASE_PROJ_SCALE_FACTOR:
			if (GMT_IS_ZERO (C->current.setting.proj_scale_factor + 1.0)) /* Default scale for chosen projection */
				strcpy (value, "default"); /* Default scale for chosen projection */
			else
				sprintf (value, "%g", C->current.setting.proj_scale_factor);
			break;

		/* GMT GROUP */

#ifdef GMT_COMPAT
		case GMTCASE_HISTORY: GMT_COMPAT_WARN;
#endif
		case GMTCASE_GMT_HISTORY:
			sprintf (value, "%s", ft[C->current.setting.history]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_INTERPOLANT: GMT_COMPAT_WARN;
#endif
		case GMTCASE_GMT_INTERPOLANT:
			if (C->current.setting.interpolant == 0)
				strcpy (value, "linear");
			else if (C->current.setting.interpolant == 1)
				strcpy (value, "akima");
			else if (C->current.setting.interpolant == 2)
				strcpy (value, "cubic");
			else if (C->current.setting.interpolant == 3)
				strcpy (value, "none");
			else
				strcpy (value, "undefined");
			break;
#ifdef GMT_COMPAT
		case GMTCASE_VERBOSE: GMT_COMPAT_WARN;
#endif
		case GMTCASE_GMT_VERBOSE:
			sprintf (value, "%ld", C->current.setting.verbose);
			break;

		/* DIR GROUP */

		case GMTCASE_DIR_TMP:
			strcpy (value, (C->session.TMPDIR) ? C->session.TMPDIR : "");
			break;
		case GMTCASE_DIR_USER:
			strcpy (value, (C->session.USERDIR) ? C->session.USERDIR : "");
			break;

		/* TIME GROUP */

		case GMTCASE_TIME_EPOCH:
			strncpy (value, C->current.setting.time_system.epoch, (size_t)GMT_TEXT_LEN64);
			break;
		case GMTCASE_TIME_IS_INTERVAL:
			if (C->current.setting.time_is_interval)
				sprintf (value, "%c%ld%c", pm[C->current.time.truncate.direction], C->current.time.truncate.T.step, C->current.time.truncate.T.unit);
			else
				sprintf (value, "off");
			break;
		case GMTCASE_TIME_INTERVAL_FRACTION:
			sprintf (value, "%g", C->current.setting.time_interval_fraction);
			break;
		case GMTCASE_TIME_LANGUAGE:
			strncpy (value, C->current.setting.time_language, (size_t)GMT_TEXT_LEN64);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_WANT_LEAP_SECONDS: GMT_COMPAT_WARN;
#endif
		case GMTCASE_TIME_LEAP_SECONDS:
			sprintf (value, "%s", ft[C->current.setting.time_leap_seconds]);
			break;
		case GMTCASE_TIME_UNIT:
			value[0] = C->current.setting.time_system.unit;
			break;
		case GMTCASE_TIME_WEEK_START:
			sprintf (value, "%s", GMT_weekdays[C->current.setting.time_week_start]);
			break;
#ifdef GMT_COMPAT
		case GMTCASE_Y2K_OFFSET_YEAR: GMT_COMPAT_WARN;
#endif
		case GMTCASE_TIME_Y2K_OFFSET_YEAR:
			sprintf (value, "%ld", C->current.setting.time_Y2K_offset_year);
			break;

		default:
			GMT_report (C, GMT_MSG_FATAL, "Syntax error in GMT_putparameter: Unrecognized keyword %s\n", keyword);
			error = TRUE;
			break;
	}
	return (value);
}

GMT_LONG GMT_savedefaults (struct GMT_CTRL *C, char *file)
{
	GMT_LONG error = 0, rec = 0;
	char line[GMT_BUFSIZ], keyword[GMT_TEXT_LEN256], string[GMT_TEXT_LEN256];
	FILE *fpi = NULL, *fpo = NULL;

	if (file[0] == '-' && !file[1])
		fpo = C->session.std[GMT_OUT];
	else if ((fpo = fopen (file, "w")) == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Could not create file %s\n", file);
		return (-1);
	}

	/* Find the global gmt.conf file */

	sprintf (line, "%s/conf/gmt.conf", C->session.SHAREDIR);
	if (access (line, R_OK)) {
		GMT_report (C, -GMT_MSG_FATAL, "Error: Could not find system defaults file - Aborting.\n");
		return (0);
	}
	if ((fpi = fopen (line, "r")) == NULL) return (-1);

	/* Set up hash table */

	GMT_hash_init (C, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);

	while (fgets (line, GMT_BUFSIZ, fpi)) {
		rec++;
		GMT_chop (C, line);	/* Get rid of [\r]\n */
		if (rec == 1) {	/* Copy version from gmt.conf */
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
		fprintf (fpo, "%s= %s\n", string, GMT_putparameter (C, keyword));
	}

	fclose (fpi);
	if (fpo != C->session.std[GMT_OUT]) fclose (fpo);

	gmt_free_hash (C, keys_hashnode, GMT_N_KEYS);	/* Done with this for now */
	if (error) GMT_report (C, GMT_MSG_FATAL, "Error: %ld conversion errors while writing gmt.conf\n", error);

	return (0);
}

void GMT_putdefaults (struct GMT_CTRL *C, char *this_file)	/* Dumps the GMT parameters to file or standard output */
{	/* ONLY USED BY GMTSET AND GMTDEFAULTS */
	if (this_file)	/* File name is defined: use it */
		GMT_savedefaults (C, this_file);
	else if (C->session.TMPDIR) {	/* Write C->session.TMPDIR/gmt.conf */
		char *path = CNULL;
		path = GMT_memory (C, NULL, strlen (C->session.TMPDIR) + 10, char);
		sprintf (path, "%s/gmt.conf", C->session.TMPDIR);
		GMT_savedefaults (C, path);
		GMT_free (C, path);
	}
	else	/* Write gmt.conf in current directory */
		GMT_savedefaults (C, "gmt.conf");
}

void GMT_getdefaults (struct GMT_CTRL *C, char *this_file)	/* Read user's gmt.conf file and initialize parameters */
{
	char file[GMT_BUFSIZ];

	if (this_file)	/* Defaults file is specified */
		GMT_loaddefaults (C, this_file);
	else if (GMT_getuserpath (C, "gmt.conf", file))
		GMT_loaddefaults (C, file);
}

void gmt_append_trans (char *text, double transparency)
{
	char trans[GMT_TEXT_LEN64];
	if (!GMT_IS_ZERO (transparency) && text[0] != '-') {	/* Append nonzero transparency */
		sprintf (trans, "@%d", irint (100.0 * transparency));
		strcat (text, trans);
	}
}

char *GMT_putfill (struct GMT_CTRL *C, struct GMT_FILL *F)
{
	/* Creates the name (if equivalent) or the string r[/g/b] corresponding to the RGB triplet or a pattern.
	 * Example: GMT_putfill (C, fill) may produce "white" or "1/2/3" or "p300/7"
	 */

	static char text[GMT_TEXT_LEN256];
	GMT_LONG i;

	if (F->use_pattern) {
		if (F->pattern_no)
			sprintf (text, "p%ld/%ld", F->dpi, F->pattern_no);
		else
			sprintf (text, "p%ld/%s", F->dpi, F->pattern);
	}
	else if (F->rgb[0] < -0.5)
		sprintf (text, "-");
	else if ((i = GMT_getrgb_index (C, F->rgb)) >= 0)
		sprintf (text, "%s", GMT_color_name[i]);
	else if (GMT_is_gray (F->rgb))
		sprintf (text, "%.5g", GMT_q(GMT_s255(F->rgb[0])));
	else
		sprintf (text, "%.5g/%.5g/%.5g", GMT_t255(F->rgb));
	gmt_append_trans (text, F->rgb[3]);
	return (text);
}

char *GMT_putcolor (struct GMT_CTRL *C, double *rgb)
{
	/* Creates the name (if equivalent) or the string r[/g/b] corresponding to the RGB triplet.
	 * Example: GMT_putcolor (C, rgb) may produce "white" or "1/2/3"
	 */

	static char text[GMT_TEXT_LEN256];
	GMT_LONG i;

	if (rgb[0] < -0.5)
		sprintf (text, "-");
	else if ((i = GMT_getrgb_index (C, rgb)) >= 0)
		sprintf (text, "%s", GMT_color_name[i]);
	else if (GMT_is_gray(rgb))
		sprintf (text, "%.5g", GMT_q(GMT_s255(rgb[0])));
	else
		sprintf (text, "%.5g/%.5g/%.5g", GMT_t255(rgb));
	gmt_append_trans (text, rgb[3]);
	return (text);
}

char *GMT_putrgb (struct GMT_CTRL *C, double *rgb)
{
	/* Creates t the string r/g/b corresponding to the RGB triplet */

	static char text[GMT_TEXT_LEN256];

	if (rgb[0] < -0.5)
		sprintf (text, "-");
	else
		sprintf (text, "%.5g/%.5g/%.5g", GMT_t255(rgb));
	gmt_append_trans (text, rgb[3]);
	return (text);
}

char *GMT_putcmyk (struct GMT_CTRL *C, double *cmyk)
{
	/* Creates the string c/m/y/k corresponding to the CMYK quadruplet */

	static char text[GMT_TEXT_LEN256];

	if (cmyk[0] < -0.5)
		sprintf (text, "-");
	else
		sprintf (text, "%.5g/%.5g/%.5g/%.5g", GMT_q(cmyk[0]), GMT_q(cmyk[1]), GMT_q(cmyk[2]), GMT_q(cmyk[3]));
	gmt_append_trans (text, cmyk[4]);
	return (text);
}

char *GMT_puthsv (struct GMT_CTRL *C, double *hsv)
{
	/* Creates the string h/s/v corresponding to the HSV triplet */

	static char text[GMT_TEXT_LEN256];

	if (hsv[0] < -0.5)
		sprintf (text, "-");
	else
		sprintf (text, "%.5g-%.5g-%.5g", GMT_q(hsv[0]), GMT_q(hsv[1]), GMT_q(hsv[2]));
	gmt_append_trans (text, hsv[3]);
	return (text);
}

GMT_LONG GMT_is_valid_number (char *t)
{
	GMT_LONG i, n;

	/* Checks if t fits the format [+|-][xxxx][.][yyyy][e|E[+|-]nn]. */

	if (!t) return (TRUE);				/* Cannot be NULL */
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
	return ((t[i] || n == 0) ? FALSE : TRUE);
}

double GMT_convert_units (struct GMT_CTRL *C, char *string, GMT_LONG default_unit, GMT_LONG target_unit)
{
	/* Converts the input string "value" to a float in units indicated by target_unit
	 * If value does not contain a unit (''c', 'i', or p') then the units indicated
	 * by default_unit will be used.
	 * Both target_unit and default_unit are either GMT_PT, GMT_CM, GMT_INCH or GMT_M.
	 */

	GMT_LONG c = 0, len, given_unit, have_unit = FALSE;
	double value;

	if ((len = strlen(string))) {
		c = string[len-1];
		if ((have_unit = isalpha ((int)c))) string[len-1] = '\0';	/* Temporarily remove unit */
	}

	/* So c is either 0 (meaning default unit) or any letter (even junk like z) */

	given_unit = GMT_unit_lookup (C, c, default_unit);	/* Will warn if c is not 0, 'c', 'i', 'p' */

	if (!GMT_is_valid_number (string))
		GMT_report (C, GMT_MSG_FATAL, "Warning: %s not a valid number and may not be decoded properly.\n", string);

	value = atof (string) * C->session.u2u[given_unit][target_unit];
	if (have_unit) string[len-1] = (char)C->session.unit_name[given_unit][0];	/* Put back the (implied) given unit */

	return (value);
}

GMT_LONG GMT_unit_lookup (struct GMT_CTRL *C, GMT_LONG c, GMT_LONG unit)
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
			GMT_report (C, GMT_MSG_FATAL, "Warning: Length unit %c not supported - revert to default unit [%s]\n", (int)c, C->session.unit_name[unit]);
			break;
	}

	return (unit);
}

GMT_LONG GMT_hash (struct GMT_CTRL *C, char *v, GMT_LONG n_hash)
{
	GMT_LONG h;
	for (h = 0; *v != '\0'; v++) h = (64 * h + (*v)) % n_hash;
	while (h < 0) h += n_hash;
	return (h);
}

GMT_LONG GMT_hash_lookup (struct GMT_CTRL *C, char *key, struct GMT_HASH *hashnode, GMT_LONG n, GMT_LONG n_hash)
{
	GMT_LONG i;
	struct GMT_HASH *this = NULL;

	i = GMT_hash (C, key, n_hash);	/* Get initial hash key */

	if (i >= n || i < 0 || !hashnode[i].next) return (-1);	/* Bad key */
	this = hashnode[i].next;
	while (this && strcmp (this->key, key)) this = this->next;

	return ((this) ? this->id : -1);
}

void GMT_hash_init (struct GMT_CTRL *C, struct GMT_HASH *hashnode, char **keys, GMT_LONG n_hash, GMT_LONG n_keys)
{
	GMT_LONG i, entry;
	struct GMT_HASH *this = NULL;

	/* Set up hash table */

	for (i = 0; i < n_hash; i++) hashnode[i].next = NULL;	/* Start with NULL everywhere */
	for (i = 0; i < n_keys; i++) {
		entry = GMT_hash (C, keys[i], n_hash);
		this = &hashnode[entry];
		while (this->next) this = this->next;	/* Get to end of linked list for this hash */
		this->next = GMT_memory (C, NULL, 1, struct GMT_HASH);
		this->next->key = keys[i];
		this->next->id = i;
	}
}

GMT_LONG GMT_get_ellipsoid (struct GMT_CTRL *C, char *name)
{
	GMT_LONG i, n;
	char line[GMT_BUFSIZ];
	double pol_radius;
#ifdef GMT_COMPAT
	FILE *fp = NULL;
	char path[GMT_BUFSIZ];
	double slop;
#endif

	/* Try to get ellipsoid from the default list */

	for (i = 0; i < GMT_N_ELLIPSOIDS; i++) {
		if (!strcmp(name, C->current.setting.ref_ellipsoid[i].name)) return (i);
	}

	i = GMT_N_ELLIPSOIDS - 1;

	/* Read ellipsoid information as <a>,<finv> */
	n = sscanf (name, "%lf,%s", &C->current.setting.ref_ellipsoid[i].eq_radius, line);
	if (n < 1) {}	/* Failed to read arguments */
	else if (n == 1)
		C->current.setting.ref_ellipsoid[i].flattening = 0.0; /* Read equatorial radius only ... spherical */
	else if (line[0] == 'b') {	/* Read semi-minor axis */
		n = sscanf (&line[2], "%lf", &pol_radius);
		C->current.setting.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / C->current.setting.ref_ellipsoid[i].eq_radius);
	}
	else if (line[0] == 'f') {	/* Read flattening */
		n = sscanf (&line[2], "%lf", &C->current.setting.ref_ellipsoid[i].flattening);
	}
	else {				/* Read inverse flattening */
		n = sscanf (line, "%lf", &C->current.setting.ref_ellipsoid[i].flattening);
		if (!GMT_IS_SPHERICAL (C)) C->current.setting.ref_ellipsoid[i].flattening = 1.0 / C->current.setting.ref_ellipsoid[i].flattening;
	}
	if (n == 1) return (i);

#ifdef GMT_COMPAT
	/* Try to open as file first in (1) current dir, then in (2) $C->session.SHAREDIR */

	GMT_report (C, GMT_MSG_COMPAT, "Warning: Assigning PROJ_ELLIPSOID a file name is deprecated, use <a>,<inv_f> instead");
	GMT_getsharepath (C, CNULL, name, "", path);

	if ((fp = fopen (name, "r")) != NULL || (fp = fopen (path, "r")) != NULL) {
		/* Found file, now get parameters */
		while (fgets (line, GMT_BUFSIZ, fp) && (line[0] == '#' || line[0] == '\n'));
		fclose (fp);
		n = sscanf (line, "%s %" GMT_LL "d %lf %lf %lf", C->current.setting.ref_ellipsoid[i].name,
			&C->current.setting.ref_ellipsoid[i].date, &C->current.setting.ref_ellipsoid[i].eq_radius,
			&pol_radius, &C->current.setting.ref_ellipsoid[i].flattening);
		if (n != 5) {
			GMT_report (C, GMT_MSG_FATAL, "Error decoding user ellipsoid parameters (%s)\n", line);
			GMT_exit (EXIT_FAILURE);
		}

		if (pol_radius == 0.0) {} /* Ignore semi-minor axis */
		else if (GMT_IS_SPHERICAL (C)) {
			/* zero flattening means we must compute flattening from the polar and equatorial radii: */

			C->current.setting.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / C->current.setting.ref_ellipsoid[i].eq_radius);
			GMT_report (C, GMT_MSG_NORMAL, "user-supplied ellipsoid has implicit flattening of %.8f\n", C->current.setting.ref_ellipsoid[i].flattening);
		}
		/* else check consistency: */
		else if ((slop = fabs (C->current.setting.ref_ellipsoid[i].flattening - 1.0 + (pol_radius/C->current.setting.ref_ellipsoid[i].eq_radius))) > 1.0e-8) {
			GMT_report (C, GMT_MSG_NORMAL, "Warning: Possible inconsistency in user ellipsoid parameters (%s) [off by %g]\n", line, slop);
		}
		return (i);
	}
#endif

	return (-1);
}

GMT_LONG GMT_get_datum (struct GMT_CTRL *C, char *name)
{
	GMT_LONG i;

	if (!name[0]) return (-1);
	for (i = 0; i < GMT_N_DATUMS && strcmp (name, C->current.setting.proj_datum[i].name); i++);
	if (i == GMT_N_DATUMS) return (-1);	/* Error */
	return (i);
}

GMT_LONG GMT_get_time_system (struct GMT_CTRL *C, char *name, struct GMT_TIME_SYSTEM *time_system)
{
	/* Convert TIME_SYSTEM into TIME_EPOCH and TIME_UNIT.
	   TIME_SYSTEM can be one of the following: j2000, jd, mjd, s1985, unix, dr0001, rata
	   or any string in the form "TIME_UNIT since TIME_EPOCH", like "seconds since 1985-01-01".
	   This function only splits the strings, no validation or analysis is done.
	   See GMT_init_time_system_structure for that.
	   TIME_SYSTEM = other is completely ignored.
	*/
	char *epoch = NULL;

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
		strcpy (time_system->epoch, epoch);
		time_system->unit = name[0];
		if (!strncmp (name, "mon", (size_t)3)) time_system->unit = 'o';
	}
	else
		return (TRUE);
	return (FALSE);
}

GMT_LONG GMT_get_char_encoding (struct GMT_CTRL *C, char *name)
{
	GMT_LONG i;

	for (i = 0; i < 7 && strcmp (name, GMT_weekdays[i]); i++);
	return (i);
}

void gmt_setshorthand (struct GMT_CTRL *C) {/* Read user's .gmt_io file and initialize shorthand notation */
	GMT_LONG n = 0, n_alloc = 0;
	char file[GMT_BUFSIZ], line[GMT_BUFSIZ], a[GMT_TEXT_LEN64], b[GMT_TEXT_LEN64], c[GMT_TEXT_LEN64], d[GMT_TEXT_LEN64], e[GMT_TEXT_LEN64];
	FILE *fp = NULL;

	C->session.n_shorthands = 0;	/* By default there are no shorthands unless .gmt_io is found */

	if (!GMT_getuserpath (C, ".gmt_io", file)) return;
	if ((fp = fopen (file, "r")) == NULL) return;

	GMT_set_meminc (C, GMT_TINY_CHUNK);	/* Only allocate a small amount */
	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		if (sscanf (line, "%s %s %s %s %s", a, b, c, d, e) != 5) {
			GMT_report (C, GMT_MSG_FATAL, "Error decoding file %s.  Bad format? [%s]\n", file, line);
			GMT_exit (EXIT_FAILURE);
		}

		if (n == n_alloc) n_alloc = GMT_malloc (C, C->session.shorthand, n, n_alloc, struct GMT_SHORTHAND);

		C->session.shorthand[n].suffix = strdup (a);
		C->session.shorthand[n].id = GMT_grd_format_decoder (C, b);
		C->session.shorthand[n].scale = (strcmp (c, "-")) ? atof (c) : 1.0;
		C->session.shorthand[n].offset = (strcmp (d, "-")) ? atof (d) : 0.0;
		C->session.shorthand[n].nan = (strcmp (e, "-")) ? atof (e) : C->session.d_NaN;
		n++;
	}
	fclose (fp);

	C->session.n_shorthands = n;
	GMT_reset_meminc (C);
	(void)GMT_malloc (C, C->session.shorthand, 0, n, struct GMT_SHORTHAND);
}

void gmt_freeshorthand (struct GMT_CTRL *C) {/* Free memory used by shorthand arrays */
	GMT_LONG i;

	if (C->session.n_shorthands == 0) return;

	for (i = 0; i < C->session.n_shorthands; i++) free ((void *)C->session.shorthand[i].suffix);
	GMT_free (C, C->session.shorthand);
}

#ifdef FLOCK
void gmt_file_lock (struct GMT_CTRL *C, int fd, struct flock *lock)
{
	int status;
	lock->l_type = F_WRLCK;		/* Lock for [exclusive] reading/writing */
	lock->l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock->l_start = lock->l_len = 0;

	if ((status = fcntl (fd, F_SETLKW, lock))) {	/* Will wait for file to be ready for reading */
		GMT_report (C, GMT_MSG_DEBUG, "Error %d returned by fcntl [F_WRLCK]\n", status);
		return;
	}
}

void gmt_file_unlock (struct GMT_CTRL *C, int fd, struct flock *lock)
{
	int status;
	lock->l_type = F_UNLCK;		/* Release lock and close file */
	lock->l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock->l_start = lock->l_len = 0;

	if ((status = fcntl (fd, F_SETLK, lock))) {
		GMT_report (C, GMT_MSG_DEBUG, "Error %d returned by fcntl [F_UNLCK]\n", status);
		return;
	}
}
#endif

GMT_LONG gmt_get_history (struct GMT_CTRL *C)
{
	GMT_LONG id, done = FALSE;
	char line[GMT_BUFSIZ], hfile[GMT_BUFSIZ], cwd[GMT_BUFSIZ], *not_used = NULL;
	char option[GMT_TEXT_LEN64], value[GMT_BUFSIZ];
	FILE *fp = NULL;	/* For .gmtcommands file */
	static struct GMT_HASH unique_hashnode[GMT_N_UNIQUE];
#ifdef FLOCK
	struct flock lock;
#endif

	if (!C->current.setting.history) return (GMT_NOERROR);	/* .gmtcommands mechanism has been disabled */

	/* This is called once per GMT Session by GMT_Create_Session via GMT_begin and before any GMT_* module is called.
	 * It loads in the known shorthands found in the .gmtcommands file
	 */

	/* If current directory is writable, use it; else use the home directory */

	not_used = getcwd (cwd, (size_t)GMT_BUFSIZ);
	if (C->session.TMPDIR)			/* Isolation mode: Use C->session.TMPDIR/.gmtcommands */
		sprintf (hfile, "%s/.gmtcommands", C->session.TMPDIR);
	else if (!access (cwd, W_OK))		/* Current directory is writable */
		sprintf (hfile, ".gmtcommands");
	else	/* Try home directory instead */
		sprintf (hfile, "%s/.gmtcommands", C->session.HOMEDIR);

	if ((fp = fopen (hfile, "r")) == NULL) return (GMT_NOERROR);	/* OK to be unsuccessful in opening this file */

	GMT_hash_init (C, unique_hashnode, GMT_unique_option, GMT_N_UNIQUE, GMT_N_UNIQUE);

	/* When we get here the file exists */
#ifdef FLOCK
	gmt_file_lock (C, fileno(fp), &lock);
#endif
	/* Format of GMT 5 .gmtcommands is as follow:
	 * OPT ARG
	 * where OPT is a 1- or 2-char code, e.g., R, X, JM, JQ, Js.  ARG is the argument.
	 * Exception: if OPT = J then ARG is just the first character of the argument  (e.g., M).
	 * File ends when we find EOF
	 */

	while (!done && fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments lines */
		GMT_chop (C, line);		/* Remove linefeed,CR */
		if (line[0] == '\0') continue;	/* Skip blank lines */
		if (!strncmp (line, "EOF", (size_t)3)) {		/* Logical end of .gmtcommands file */
			done = TRUE;
			continue;
		}
		if (sscanf (line, "%s %[^\n]", option, value) != 2) continue;	/* Quietly skip malformed lines */
		if (!value[0]) continue;	/* No argument found */
		if (option[0] == 'C') {	/* Read clip level */
			C->current.ps.clip_level = atoi(value);
			continue;
		}
		if ((id = GMT_hash_lookup (C, option, unique_hashnode, GMT_N_UNIQUE, GMT_N_UNIQUE)) < 0) continue;	/* Quietly skip malformed lines */
		C->init.history[id] = strdup (value);
	}

	/* Close the file */
#ifdef FLOCK
	gmt_file_unlock (C, fileno(fp), &lock);
#endif
	fclose (fp);

	gmt_free_hash (C, unique_hashnode, GMT_N_UNIQUE);	/* Done with this for now */

	return (GMT_NOERROR);
}

GMT_LONG gmt_put_history (struct GMT_CTRL *C)
{
	GMT_LONG id;
	char hfile[GMT_BUFSIZ], cwd[GMT_BUFSIZ], *not_used = NULL;
	FILE *fp = NULL;	/* For .gmtcommands file */
#ifdef FLOCK
	struct flock lock;
#endif

	if (!C->current.setting.history) return (GMT_NOERROR);	/* .gmtcommands mechanism has been disabled */

	/* This is called once per GMT Session by GMT_end via GMT_Destroy_Session.
	 * It writes out the known shorthands to the .gmtcommands file
	 */

	/* If current directory is writable, use it; else use the home directory */

	not_used = getcwd (cwd, (size_t)GMT_BUFSIZ);
	if (C->session.TMPDIR)			/* Isolation mode: Use C->session.TMPDIR/.gmtcommands */
		sprintf (hfile, "%s/.gmtcommands", C->session.TMPDIR);
	else if (!access (cwd, W_OK))	/* Current directory is writable */
		sprintf (hfile, ".gmtcommands");
	else	/* Try home directory instead */
		sprintf (hfile, "%s/.gmtcommands", C->session.HOMEDIR);

	if ((fp = fopen (hfile, "w")) == NULL) return (-1);	/* Not OK to be unsuccessful in creating this file */

	/* When we get here the file is open */
#ifdef FLOCK
	gmt_file_lock (C, fileno(fp), &lock);
#endif

	fprintf (fp, "# GMT 5 Session common arguments shelf\n");
	for (id = 0; id < GMT_N_UNIQUE; id++) {
		if (!C->init.history[id]) continue;	/* Not specified */
		fprintf (fp, "%s\t%s\n", GMT_unique_option[id], C->init.history[id]);
	}
	if (C->current.ps.clip_level) fprintf (fp, "C\t%ld\n", C->current.ps.clip_level); /* Write clip level */
	fprintf (fp, "EOF\n");

	/* Close the file */
#ifdef FLOCK
	gmt_file_unlock (C, fileno(fp), &lock);
#endif
	fclose (fp);

	return (GMT_NOERROR);
}

void Free_GMT_Ctrl (struct GMT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;	/* Never was allocated */
	free ((void *)C);
}

void GMT_end (struct GMT_CTRL *C)
{
	/* GMT_end will clean up after us. */

	GMT_LONG i;

	gmt_put_history (C);

	/* Remove allocated hash structures */
	gmt_free_hash (C, C->session.rgb_hashnode, GMT_N_COLOR_NAMES);
	for (i = 0; i < C->session.n_fonts; i++) free ((void *)C->session.font[i].name);
	GMT_free (C, C->session.font);
#ifdef __FreeBSD__
#ifdef _i386_
	fpresetsticky (FP_X_DZ | FP_X_INV);
	fpsetmask (FP_X_DZ | FP_X_INV);
#endif
#endif

	free ((void *)C->session.SHAREDIR);
	free ((void *)C->session.HOMEDIR);
	if (C->session.USERDIR) free ((void *)C->session.USERDIR);
	if (C->session.DATADIR) free ((void *)C->session.DATADIR);
	if (C->session.TMPDIR) free ((void *)C->session.TMPDIR);
	for (i = 0; i < GMT_N_PROJ4; i++) free ((void *)C->current.proj.proj4[i].name);
	GMT_free (C, C->current.proj.proj4);

	if (C->current.setting.io_gridfile_shorthand) gmt_freeshorthand (C);

	fflush (C->session.std[GMT_OUT]);	/* Make sure output buffer is flushed */

#if 0	/* Already done in GMT_end_module */
	gmt_free_user_media (C);	/* Free any user-specified media formats */
#endif
	GMT_free_ogr (C, &(C->current.io.OGR), 1);	/* Free up the GMT/OGR structure, if used */

	/* Terminate PSL machinery (if used) */
	PSL_endsession (C->PSL);
#ifdef DEBUG
	GMT_memtrack_report (C, GMT_mem_keeper);
#endif
	Free_GMT_Ctrl (C);	/* Deallocate control structure */
}

struct GMT_CTRL * GMT_begin_module (struct GMTAPI_CTRL *API, char *mod_name, struct GMT_CTRL **Ccopy)
{	/* All GMT modules (i.e. GMT_psxy, GMT_blockmean, ...) must call GMT_begin_module
	 * as their first call and call GMT_end_module as their last call.  This
	 * allows us to capture the GMT control structure so we can reset all
	 * parameters to what they were before exiting the module. Note:
	 * 1. Session items that remain unchanged are not replicated if allocated separately.
	 * 2. Items that may grow through session are not replicated if allocated separately.
	 */

	GMT_LONG i;
	struct GMT_CTRL *C = API->GMT, *Csave = NULL;

	Csave = (struct GMT_CTRL *) calloc ((size_t)1, sizeof (struct GMT_CTRL));

	/* First memcpy over everything; this will include pointer addresses we will have to fix below */

	GMT_memcpy (Csave, C, 1, struct GMT_CTRL);

	/* Increment level counter */
	C->hidden.func_level++;		/* This lets us know how deeply we are nested when a GMT module is called */

	/* Now fix things that were allocated separately from the main GMT structure.  These are usually text strings
	 * that were allocated via strdup since the structure only have a pointer allocated. */

	/* GMT_INIT */
	Csave->init.progname = strdup (C->init.progname);
	if (C->session.n_user_media) {
		Csave->session.n_user_media = C->session.n_user_media;
		Csave->session.user_media = GMT_memory (C, NULL, C->session.n_user_media, struct GMT_MEDIA);
		Csave->session.user_media_name = GMT_memory (C, NULL, C->session.n_user_media, char *);
		for (i = 0; i < C->session.n_user_media; i++) Csave->session.user_media_name[i] = strdup (C->session.user_media_name[i]);
	}
	/* GMT_IO */
	Csave->current.io.OGR = GMT_duplicate_ogr (C, C->current.io.OGR);	/* Duplicate OGR struct, if set */
	GMT_free_ogr (C, &(C->current.io.OGR), 1);		/* Free up the GMT/OGR structure, if used */
	
	/* GMT_COMMON */
	if (C->common.U.label) Csave->common.U.label = strdup (C->common.U.label);

	/* Reset all the common.?.active settings to FALSE */

	C->common.B.active[0] = C->common.B.active[1] = C->common.K.active = C->common.O.active = FALSE;
	C->common.P.active = C->common.U.active = C->common.V.active = FALSE;
	C->common.X.active = C->common.Y.active = FALSE;
	C->common.R.active = C->common.J.active = FALSE;
	C->common.a.active = C->common.b.active[GMT_IN] = C->common.b.active[GMT_OUT] = C->common.c.active = FALSE;
	C->common.f.active[GMT_IN] = C->common.f.active[GMT_OUT] = C->common.g.active = C->common.h.active = FALSE;
	C->common.p.active = C->common.s.active = C->common.t.active = C->common.colon.active = FALSE;
	GMT_memset (C->common.b.ncol, 2, GMT_LONG);

	*Ccopy = Csave;				/* Pass back out for safe-keeping by the module until GMT_end_module is called */
	C->init.module_name = mod_name;		/* Current module in charge */
	
	return (C);
}

void gmt_free_plot_array (struct GMT_CTRL *C) {
	if (C->current.plot.n_alloc) {
		GMT_free (C, C->current.plot.x);
		GMT_free (C, C->current.plot.y);
		GMT_free (C, C->current.plot.pen);
	}
	C->current.plot.n = C->current.plot.n_alloc = 0;
}

void GMT_end_module (struct GMT_CTRL *C, struct GMT_CTRL *Ccopy)
{
	GMT_LONG i;
#if 0	/* I do not think we need the interstitial hist_cpy; just copy the pointers */
	char *hist_cpy[GMT_N_UNIQUE];
#endif

	GMT_Garbage_Collection (C->parent, C->hidden.func_level);	/* Free up all registered memory for this module level */

	/* At the end of the module we restore all GMT settings as we found them (in Ccopy) */

	/* GMT_INIT */
	/* We treat the history explicitly since we accumulate the history regardless of nested level */
#if 0
	GMT_memset (hist_cpy, GMT_N_UNIQUE, char *);
	for (i = 0; i < GMT_N_UNIQUE; i++) if (C->init.history[i]) {
		hist_cpy[i] = strdup (C->init.history[i]);	/* Copy what we have so far */
		free ((void *)C->init.history[i]);		/* Free pointers in local GMT structure */
	}
#else
	for (i = 0; i < GMT_N_UNIQUE; i++) {
		if (Ccopy->init.history[i] && Ccopy->init.history[i] != C->init.history[i]) free ((void *)Ccopy->init.history[i]);
		Ccopy->init.history[i] = C->init.history[i];
	}
#endif
	/* GMT_CURRENT */

	Ccopy->current.ps.clip_level = C->current.ps.clip_level;

	/* GMT_COMMON */

	if (Ccopy->common.U.label && Ccopy->common.U.label != C->common.U.label) free ((void *)Ccopy->common.U.label);
	Ccopy->common.U.label = C->common.U.label;

	/* GMT_PLOT */

	gmt_free_plot_array (C);	/* Free plot arrays and reset n_alloc, n */
	GMT_free_custom_symbols (C);	/* Free linked list of custom psxy[z] symbols, if any */
	gmt_free_user_media (C);	/* Free user-specified media formats */

	/* GMT_IO */
	
	GMT_free_ogr (C, &(C->current.io.OGR), 1);		/* Free up the GMT/OGR structure, if used */

	/* Overwrite C with what we saved in GMT_begin_module */
	GMT_memcpy (C, Ccopy, 1, struct GMT_CTRL);	/* Overwrite struct with things from Ccopy */
	
	/* Now fix things that were allocated separately */

	gmt_free_user_media (Ccopy);		/* Free user-specified media formats */
#if 0
	for (i = 0; i < GMT_N_UNIQUE; i++) if (hist_cpy[i]) {	/* Update the cumulative history list */
		C->init.history[i] = strdup (hist_cpy[i]);
		free ((void *)hist_cpy[i]);
	}

	/* GMT_COMMON */
	if (C->common.U.label) free ((void *)C->common.U.label);
	if (Ccopy->common.U.label) {
		C->common.U.label = strdup (Ccopy->common.U.label);
		free ((void *)Ccopy->common.U.label);
	}
#endif

	free ((void *)Ccopy);	/* Good riddance */
}

void GMT_set_env (struct GMT_CTRL *C)
{
	char *this = NULL, path[GMT_BUFSIZ];

	/* Determine C->session.SHAREDIR (directory containing coast, cpt, etc. subdirectories) */

	if ((this = getenv ("GMT5_SHAREDIR")) != CNULL)	/* GMT5_SHAREDIR was set */
		C->session.SHAREDIR = strdup (this);
	else if ((this = getenv ("GMT_SHAREDIR")) != CNULL)	/* GMT_SHAREDIR was set */
		C->session.SHAREDIR = strdup (this);
	else	/* Default is GMT_SHARE_PATH */
		C->session.SHAREDIR = strdup (GMT_SHARE_PATH);
#ifdef WIN32
	DOS_path_fix (C->session.SHAREDIR);
#endif

	/* Determine HOMEDIR (user home directory) */

	if ((this = getenv ("HOME")) != CNULL)	/* HOME was set */
		C->session.HOMEDIR = strdup (this);
#ifdef WIN32
	else if ((this = getenv ("HOMEPATH")) != CNULL)	/* HOMEPATH was set */
		C->session.HOMEDIR = strdup (this);
#endif
	else
		GMT_report (C, GMT_MSG_FATAL, "Warning: Could not determine home directory!\n");
#ifdef WIN32
	DOS_path_fix (C->session.HOMEDIR);
#endif

	/* Determine GMT_USERDIR (directory containing user replacements contents in GMT_SHAREDIR) */

	if ((this = getenv ("GMT_USERDIR")) != CNULL)	/* GMT_USERDIR was set */
		C->session.USERDIR = strdup (this);
	else if (C->session.HOMEDIR) {	/* Use default path for GMT_USERDIR (~/.gmt) */
		sprintf (path, "%s/%s", C->session.HOMEDIR, ".gmt");
		C->session.USERDIR = strdup (path);
	}
	if (access (C->session.USERDIR, R_OK)) {	/* If we cannot access this dir then we won't use it */
		free ((void *)C->session.USERDIR);
		C->session.USERDIR = CNULL;
	}
#ifdef WIN32
	DOS_path_fix (C->session.USERDIR);
#endif

#ifdef GMT_COMPAT
	/* Check if obsolete GMT_CPTDIR was specified */

	if ((this = getenv ("GMT_CPTDIR")) != CNULL) {	/* GMT_CPTDIR was set */
		GMT_report (C, GMT_MSG_FATAL, "Warning: Environment variable GMT_CPTDIR was set but is no longer used by GMT.\n");
		GMT_report (C, GMT_MSG_FATAL, "Warning: System-wide color tables are in %s/cpt.\n", C->session.SHAREDIR);
		GMT_report (C, GMT_MSG_FATAL, "Warning: Use GMT_USERDIR (%s) instead and place user-defined color tables there.\n", C->session.USERDIR);
	}
#endif

	/* Determine GMT_DATADIR (data directories) */

	if ((this = getenv ("GMT_DATADIR")) != CNULL) {	/* GMT_DATADIR was set */
		if (!strchr (this, PATH_SEPARATOR) && access (this, R_OK))	/* A single directory, but cannot be accessed */
			C->session.DATADIR = CNULL;
		else	/* A list of directories */
			C->session.DATADIR = strdup (this);
#ifdef WIN32
		DOS_path_fix (C->session.DATADIR);
#endif
	}

	/* Determine GMT_TMPDIR (for isolation mode). Needs to exist use it. */

	if ((this = getenv ("GMT_TMPDIR")) != CNULL) {	/* GMT_TMPDIR was set */
#ifdef WIN32
		if (access (this, R_OK+W_OK)) {		/* Adding the +X_OK makes Win 64 bits version crash */
#else
		if (access (this, R_OK+W_OK+X_OK)) {
#endif
			GMT_report (C, GMT_MSG_FATAL, "Warning: Environment variable GMT_TMPDIR was set to %s, but directory is not accessible.\n", this);
			GMT_report (C, GMT_MSG_FATAL, "Warning: GMT_TMPDIR needs to have mode rwx. Isolation mode switched off.\n");
			C->session.TMPDIR = CNULL;
		}
		else
			C->session.TMPDIR = strdup (this);
#ifdef WIN32
		DOS_path_fix (C->session.TMPDIR);
#endif
	}
}

#define Return { GMT_report (C, GMT_MSG_FATAL, "Found no history for option -%s\n", str); return (-1); }

GMT_LONG GMT_Complete_Options (struct GMT_CTRL *C, struct GMT_OPTION *options)
{
	/* Go through the given arguments and look for shorthands,
	 * i.e., -B, -J, -R, -X, -x, -Y, -y, -c, -p. given without arguments.
	 * If found, see if we have a matching command line history and then
	 * update that entry in the option list.
	 * Finally, keep the option arguments in the history list.
	 * However, when func_level > 1, do update the entry, but do not
	 * remember it in history. */
	GMT_LONG id, k, update, remember;
	struct GMT_OPTION *opt = NULL;
	char str[3];

	remember = (C->hidden.func_level == 1);	/* Only update the history for top level function */

	for (opt = options; opt; opt = opt->next) {
		if (!strchr ("BJRXxYycp", opt->option)) continue;	/* Not one of the shorthand options */
		update = FALSE;
		str[0] = opt->option; str[1] = str[2] = '\0';
		if (opt->option == 'J') {	/* -J is special since it can be -J or -J<code> */
			/* Always look up "J" first. It comes before "J?" and tells what the last -J was */
			for (k = 0, id = -1; k < GMT_N_UNIQUE && id == -1; k++) if (!strcmp (GMT_unique_option[k], str)) id = k;
			if (id < 0) Return;
			if (opt->arg && opt->arg[0]) {	/* Gave -J<code>[<args>] so we either use or update history and continue */
				str[1] = opt->arg[0];
				/* Remember this last -J<code> for later use as -J, but do not remember it when -Jz|Z */
				if (str[1] != 'Z' && str[1] != 'z' && remember) C->init.history[id] = strdup (&str[1]);
				if (opt->arg[1]) update = TRUE;	/* Gave -J<code><args> so we want to update history and continue */
			}
			else {
				if (!C->init.history[id]) Return;
				str[1] = C->init.history[id][0];
			}
			/* Continue looking for -J<code> */
			for (k = id + 1, id = -1; k < GMT_N_UNIQUE && id == -1; k++) if (!strcmp (GMT_unique_option[k], str)) id = k;
			if (id < 0) Return;
		}
		else {	/* Gave -R[<args>], -B[<args>] etc. so we either use or update history and continue */
			for (k = 0, id = -1; k < GMT_N_UNIQUE && id == -1; k++) if (!strcmp (GMT_unique_option[k], str)) id = k;
			if (id < 0) Return;
			if (opt->arg && opt->arg[0]) update = TRUE;	/* Gave -R<args>, -B<args> etc. so we we want to update history and continue */
		}
		if (update) {	/* Gave -J<code><args>, -R<args>, -B<args> etc. so we update history and continue */
			if (remember) C->init.history[id] = strdup (opt->arg);
		}
		else {	/* Gave -J<code>, -R, -B etc. so we complete the option and continue */
			if (!C->init.history[id]) Return;
			opt->arg = strdup (C->init.history[id]);
		}
	}

	return (GMT_NOERROR);
}

/* Here is the new -B parser with all its sub-functions */

GMT_LONG gmt_strip_colonitem (struct GMT_CTRL *C, GMT_LONG axis, const char *in, const char *pattern, char *item, char *out) {
	/* Removes the searched-for item from in, returns it in item, with the rest in out.
	 * pattern is usually ":." for title, ":," for unit, and ":" for label.
	 * ASSUMPTION: Only pass ":" after first removing titles and units
	 */

	char *s = NULL, *str = "xyz";
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
	else	/* No item to update */
		strcpy (out, in);

	if (error) {	/* Problems with decoding */
		GMT_report (C, GMT_MSG_FATAL, "ERROR: Missing terminating colon in -B string %c-component %s\n", str[axis], in);
		return (TRUE);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":.")) {	/* Problems with decoding title */
		GMT_report (C, GMT_MSG_FATAL, "ERROR: More than one title in -B string %c-component %s\n", str[axis], in);
		return (TRUE);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":,")) {	/* Problems with decoding unit */
		GMT_report (C, GMT_MSG_FATAL, "ERROR: More than one unit string in -B %c-component %s\n", str[axis], in);
		return (TRUE);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":=")) {	/* Problems with decoding prefix */
		GMT_report (C, GMT_MSG_FATAL, "ERROR: More than one prefix string in  -B component %s\n", in);
		return (TRUE);
	}
	if (strstr (out, pattern)) {	/* Problems with decoding label */
		GMT_report (C, GMT_MSG_FATAL, "ERROR: More than one label string in  -B component %s\n", in);
		return (TRUE);
	}
	return (GMT_NOERROR);
}

void gmt_handle_atcolon (struct GMT_CTRL *C, char *txt, GMT_LONG old)
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

GMT_LONG gmt_split_info_strings (struct GMT_CTRL *C, const char *in, char *x_info, char *y_info, char *z_info) {
	/* Take the -B string (minus the leading -B) and chop into 3 strings for x, y, and z */

	GMT_LONG mute = FALSE, i, n_slash, s_pos[2];

	x_info[0] = y_info[0] = z_info[0] = '\0';

	for (i = n_slash = 0; in[i] && n_slash < 3; i++) {
		if (in[i] == ':') mute = !mute;
		if (in[i] == '/' && !mute) s_pos[n_slash++] = i;	/* Axis-separating slash, not a slash in a label */
	}

	if (n_slash == 3) {
		GMT_report (C, GMT_MSG_FATAL, "Error splitting -B string %s\n", in);
		return (1);
	}

	if (n_slash == 2) {	/* Got x/y/z */
		i = strlen (in);
		strncpy (x_info, in, (size_t)s_pos[0]);					x_info[s_pos[0]] = '\0';
		strncpy (y_info, &in[s_pos[0]+1], (size_t)(s_pos[1] - s_pos[0] - 1));	y_info[s_pos[1] - s_pos[0] - 1] = '\0';
		strncpy (z_info, &in[s_pos[1]+1], (size_t)(i - s_pos[1] - 1));		z_info[i - s_pos[1] - 1] = '\0';
	}
	else if (n_slash == 1) {	/* Got x/y */
		i = strlen (in);
		strncpy (x_info, in, (size_t)s_pos[0]);					x_info[s_pos[0]] = '\0';
		strncpy (y_info, &in[s_pos[0]+1], (size_t)(i - s_pos[0] - 1));		y_info[i - s_pos[0] - 1] = '\0';
	}
	else {	/* Got x with implicit copy to y */
		strcpy (x_info, in);
		strcpy (y_info, in);
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_init_custom_annot (struct GMT_CTRL *C, struct GMT_PLOT_AXIS *A, GMT_LONG *n_int)
{
	/* Reads a file with one or more records of the form
	 * value	types	[label]
	 * where value is the coordinate of the tickmark, types is a combination
	 * of a|i (annot or interval annot), f (tick), or g (gridline).
	 * The a|i will take a label string (or sentence).
	 * The item argument specifies which type to consider [a|i,f,g].  We return
	 * an array with coordinates and labels, and set interval to TRUE if applicable.
	 */
	GMT_LONG k, n_errors = 0;
	char line[GMT_BUFSIZ], type[8];
	FILE *fp = GMT_fopen (C, A->file_custom, "r");

	GMT_memset (n_int, 4, GMT_LONG);
	while (GMT_fgets (C, line, GMT_BUFSIZ, fp)) {
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
					GMT_report (C, GMT_MSG_FATAL, "Error: Unrecognixed type (%c) in custom file %s.\n", type[k], A->file_custom);
					n_errors++;
					break;
			}
		}
	}
	GMT_fclose (C, fp);
	if (n_int[0] && n_int[1]) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Cannot mix interval and regular annotations in custom file %s.\n", A->file_custom);
		n_errors++;
	}
	return (n_errors);
}

GMT_LONG gmt_set_titem (struct GMT_CTRL *C, struct GMT_PLOT_AXIS *A, double val, double phase, char flag, char unit) {
	/* Load the values into the appropriate GMT_PLOT_AXIS_ITEM structure */

	struct GMT_PLOT_AXIS_ITEM *I = NULL;
	char *format = NULL;

	if (A->type == GMT_TIME) {	/* Strict check on time intervals */
		if (GMT_verify_time_step (C, irint (val), unit)) GMT_exit (EXIT_FAILURE);
		if ((fmod (val, 1.0) > GMT_CONV_LIMIT)) {
			GMT_report (C, GMT_MSG_FATAL, "ERROR: Time step interval (%g) must be an integer\n", val);
			GMT_exit (EXIT_FAILURE);
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
			GMT_report (C, GMT_MSG_FATAL, "Bad flag (%c) passed to gmt_set_titem\n", flag);
			GMT_exit (EXIT_FAILURE);
			break;
	}

	if (phase != 0.0) A->phase = phase;	/* phase must apply to entire axis */
	if (I->active) {
		GMT_report (C, GMT_MSG_FATAL, "Warning: Axis sub-item %c set more than once (typo?)\n", flag);
		return (GMT_NOERROR);
	}
#ifdef GMT_COMPAT
	if (unit == 'c' || unit == 'C') {
		GMT_report (C, GMT_MSG_COMPAT, "Warning: Unit c (arcseconds) is deprecated; use s instead.\n");
		unit = 's';
	}
#endif
	I->type = flag;
	I->unit = unit;
	I->interval = val;
	I->flavor = 0;
	I->active = TRUE;
	I->upper_case = FALSE;
	format = (C->current.map.frame.primary) ? C->current.setting.format_time[0] : C->current.setting.format_time[1];
	switch (format[0]) {	/* This parameter controls which version of month/day textstrings we use for plotting */
		case 'F':	/* Full name, upper case */
			I->upper_case = TRUE;
		case 'f':	/* Full name, lower case */
			I->flavor = 0;
			break;
		case 'A':	/* Abbreviated name, upper case */
			I->upper_case = TRUE;
		case 'a':	/* Abbreviated name, lower case */
			I->flavor = 1;
			break;
		case 'C':	/* 1-char name, upper case */
			I->upper_case = TRUE;
		case 'c':	/* 1-char name, lower case */
			I->flavor = 2;
			break;
		default:
			break;
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_decode_tinfo (struct GMT_CTRL *C, GMT_LONG axis, char flag, char *in, struct GMT_PLOT_AXIS *A) {
	/* Decode the annot/tick segments of the clean -B string pieces */

	char *t = NULL, *s = NULL, unit, *str = "xyz";
	double val = 0.0, phase = 0.0;

	if (!in) return (GMT_NOERROR);	/* NULL pointer passed */

	if (flag == 'c') {	/* Custom annotation arrangement */
		GMT_LONG k, n_int[4];
		char *list = "aifg";
		if (!(GMT_access (C, &in[1], R_OK))) {
			A->file_custom = strdup (&in[1]);
			A->special = GMT_CUSTOM;
			if (gmt_init_custom_annot (C, A, n_int)) return (-1);	/* See what ticks, anots, gridlines etc are requested */
			for (k = 0; k < 4; k++) {
				if (n_int[k] == 0) continue;
				flag = list[k];
				if (!C->current.map.frame.primary) flag = (char)toupper ((int)flag);
				gmt_set_titem (C, A, 0.0, 0.0, flag, 0);	/* Store the findings for this segment */
			}
			if (n_int[1]) A->item[GMT_ANNOT_UPPER+!C->current.map.frame.primary].special = TRUE;
		}
		else
			GMT_report (C, GMT_MSG_FATAL, "ERROR: Cannot access custom file in -B string %c-component %s\n", str[axis], in);
		return (GMT_NOERROR);
	}
	
	t = in;

	/* Here, t must point to a valid number.  If t[0] is not [+,-,.] followed by a digit we have an error */

	/* Decode interval, get pointer to next segment */
	if ((val = strtod (t, &s)) < 0.0 && C->current.proj.xyz_projection[A->id] != GMT_LOG10) {	/* Interval must be >= 0 */
		GMT_report (C, GMT_MSG_FATAL, "ERROR: Negative interval in -B option (%c-component, %c-info): %s\n", str[axis], flag, in);
		return (3);
	}
	if (s[0] && (s[0] == '-' || s[0] == '+')) {	/* Phase shift information given */
		t = s;
		phase = strtod (t, &s);
	}

	/* Appended one of the allowed units, or l or p for log10/pow */
#ifdef GMT_COMPAT
	if (s[0] && strchr ("YyOoUuKkJjDdHhMmSsCcrRlp", s[0]))
#else
	if (s[0] && strchr ("YyOoUuKkJjDdHhMmSsrRlp", s[0]))
#endif
		unit = s[0];
	else if (A->type == GMT_TIME)				/* Default time system unit implied */
		unit = C->current.setting.time_system.unit;
	else
		unit = 0;	/* Not specified */

	if (!C->current.map.frame.primary) flag = (char) toupper ((int)flag);

	gmt_set_titem (C, A, val, phase, flag, unit);				/* Store the findings for this segment */

	C->current.map.frame.draw = TRUE;
	
	return (0);
}

GMT_LONG gmt_parse_B_option (struct GMT_CTRL *C, char *in) {
	/* gmt_parse_B_option scans an argument string and extract parameters that
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

	char out1[GMT_BUFSIZ], out2[GMT_BUFSIZ], out3[GMT_BUFSIZ], info[3][GMT_BUFSIZ];
	struct GMT_PLOT_AXIS *A = NULL;
	GMT_LONG i, j, k, ignore, g = 0, error = 0;

	if (!in || !in[0]) return (GMT_PARSE_ERROR);	/* -B requires an argument */

	switch (in[0]) {
		case 's':
			C->current.map.frame.primary = FALSE; k = 1; break;
		case 'p':
			C->current.map.frame.primary = TRUE; k = 1; break;
		default:
			C->current.map.frame.primary = TRUE; k = 0; break;
	}

	/* C->current.map.frame.side[] may be set already when parsing gmt.conf flags */

	if (!C->current.map.frame.init) {	/* First time we initialize stuff */
		for (i = 0; i < 3; i++) {
			GMT_memset (&C->current.map.frame.axis[i], 1, struct GMT_PLOT_AXIS);
			C->current.map.frame.axis[i].id = (int)i;
			for (j = 0; j < 6; j++) {
				C->current.map.frame.axis[i].item[j].parent = i;
				C->current.map.frame.axis[i].item[j].id = j;
			}
			if (C->current.proj.xyz_projection[i] == GMT_TIME) C->current.map.frame.axis[i].type = GMT_TIME;
		}
		C->current.map.frame.header[0] = '\0';
		C->current.map.frame.init = TRUE;
		C->current.map.frame.draw = FALSE;
	}

	for (i = strlen (in) - 1, ignore = FALSE; !C->current.map.frame.paint && !error && i >= 0; i--) {	/** Look for +g<fill */
		if (in[i] == ':') ignore = !ignore;
		if (ignore) continue;	/* Not look inside text items */
		if (in[i] == '+' && in[i+1] == 'g') {	/* Found +g<fill> */
			error += GMT_getfill (C, &in[i+2], &C->current.map.frame.fill);
			if (!error) {
				C->current.map.frame.paint = TRUE;
				g = i;
				in[g] = '\0';	/* Chop off +g for now */
			}
		}
	}
	
	error += gmt_strip_colonitem (C, 0, &in[k], ":.", C->current.map.frame.header, out1);	/* Extract header string, if any */
	GMT_enforce_rgb_triplets (C, C->current.map.frame.header, GMT_TEXT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */

	i = gmt_decode_wesnz (C, out1, C->current.map.frame.side, &C->current.map.frame.draw_box);		/* Decode WESNZwesnz+ flags, if any */
	out1[i] = '\0';	/* Strip the WESNZwesnz+ flags off */

	gmt_split_info_strings (C, out1, info[0], info[1], info[2]);	/* Chop/copy the three axis strings */

	for (i = 0; i < 3; i++) {	/* Process each axis separately */

		if (!info[i][0]) continue;	 /* Skip empty format string */
		if (info[i][0] == '0' && !info[i][1]) {	 /* Skip format '0' */
			C->current.map.frame.draw = TRUE;
			continue;
		}

		gmt_handle_atcolon (C, info[i], 0);	/* Temporarily modify text escape @: to @^ to avoid : parsing trouble */
		GMT_enforce_rgb_triplets (C, info[i], GMT_BUFSIZ);				/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
		error += gmt_strip_colonitem (C, i, info[i], ":,", C->current.map.frame.axis[i].unit, out1);	/* Pull out annotation unit, if any */
		error += gmt_strip_colonitem (C, i, out1, ":=", C->current.map.frame.axis[i].prefix, out2);	/* Pull out annotation prefix, if any */
		error += gmt_strip_colonitem (C, i, out2, ":", C->current.map.frame.axis[i].label, out3);	/* Pull out axis label, if any */
		gmt_handle_atcolon (C, C->current.map.frame.axis[i].label, 1);	/* Restore any @^ to @: */
		gmt_handle_atcolon (C, C->current.map.frame.axis[i].prefix, 1);	/* Restore any @^ to @: */
		gmt_handle_atcolon (C, C->current.map.frame.axis[i].unit, 1);	/* Restore any @^ to @: */

		/* Parse the annotation/tick info string */
		if (out3[0] == 'c')
			error += gmt_decode_tinfo (C, i, 'c', out3, &C->current.map.frame.axis[i]);
		else {	/* Parse from back for 'a', 'f', 'g' chunks */
			for (k = strlen (out3) - 1; k >= 0; k--) {
				if (out3[k] == 'a' || out3[k] == 'f' || out3[k] == 'g') {
					error += gmt_decode_tinfo (C, i, out3[k], &out3[k+1], &C->current.map.frame.axis[i]);
					out3[k] = '\0';	/* Replace with terminator */
				}
				else if (k == 0)	/* If no [a|f|g] then 'a' */
					error += gmt_decode_tinfo (C, i, 'a', out3, &C->current.map.frame.axis[i]);
			}
		}

		/* Make sure we have ticks to match annotation stride */
		A = &C->current.map.frame.axis[i];
		if (A->item[GMT_ANNOT_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			GMT_memcpy (&A->item[GMT_TICK_UPPER], &A->item[GMT_ANNOT_UPPER], 1, struct GMT_PLOT_AXIS_ITEM);
		if (A->item[GMT_ANNOT_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			GMT_memcpy (&A->item[GMT_TICK_LOWER], &A->item[GMT_ANNOT_LOWER], 1, struct GMT_PLOT_AXIS_ITEM);
		/* Note that item[].type will say 'a' or 'A' in these cases, so we know when minor ticks were not set */

		/* Set the grid interval the same as annotation interval when not set yet */
		if (A->item[GMT_ANNOT_UPPER].active && A->item[GMT_GRID_UPPER].active && A->item[GMT_GRID_UPPER].interval == 0.0)	/* Set grid stride = annot stride */
			GMT_memcpy (&A->item[GMT_GRID_UPPER], &A->item[GMT_ANNOT_UPPER], 1, struct GMT_PLOT_AXIS_ITEM);
		if (A->item[GMT_ANNOT_LOWER].active && A->item[GMT_GRID_LOWER].active && A->item[GMT_GRID_LOWER].interval == 0.0)	/* Set grid stride = annot stride */
			GMT_memcpy (&A->item[GMT_GRID_LOWER], &A->item[GMT_ANNOT_LOWER], 1, struct GMT_PLOT_AXIS_ITEM);
	}

	/* Check if we asked for linear projections of geographic coordinates and did not specify a unit - if so set degree symbol as unit */
	if (C->current.proj.projection == GMT_LINEAR && C->current.setting.map_degree_symbol != gmt_none) {
		for (i = 0; i < 2; i++) {
			if (C->current.io.col_type[GMT_IN][i] & GMT_IS_GEO && C->current.map.frame.axis[i].unit[0] == 0) {
				C->current.map.frame.axis[i].unit[0] = '-';
				C->current.map.frame.axis[i].unit[1] = (char)C->current.setting.ps_encoding.code[C->current.setting.map_degree_symbol];
				C->current.map.frame.axis[i].unit[2] = '\0';
			}
		}
	}
	if (g) in[g] = '+';	/* Restore + */
	
	return (error);
}

GMT_LONG gmt_project_type (char *args, GMT_LONG *pos, GMT_LONG *width_given)
{
	/* Parse the start of the -J option to determine the projection type.
	 * If the first character of args is uppercase, width_given is set to 1.
	 * Pos returns the position of the first character of the parameters
	 * following the projections type.
	 * The return value is the projection type ID (see gmt_project.h), or
	 * GMT_NO_PROJ when unsuccessful.
	 */

	char t;

	/* Check for upper case */

	*width_given = (args[0] >= 'A' && args[0] <= 'Z');

	/* Compared the first part of the -J arguments against a number of Proj4
	   projection names (followed by a slash) or the 1- or 2-letter abbreviation
	   used prior to GMT 4.2.2. Case is ignored */

	if ((*pos = GMT_strlcmp ("aea/"      , args))) return (GMT_ALBERS);
	if ((*pos = GMT_strlcmp ("aeqd/"     , args))) return (GMT_AZ_EQDIST);
	if ((*pos = GMT_strlcmp ("cyl_stere/", args))) return (GMT_CYL_STEREO);
	if ((*pos = GMT_strlcmp ("cass/"     , args))) return (GMT_CASSINI);
	if ((*pos = GMT_strlcmp ("cea/"      , args))) return (GMT_CYL_EQ);
	if ((*pos = GMT_strlcmp ("eck4/"     , args))) return (GMT_ECKERT4);
	if ((*pos = GMT_strlcmp ("eck6/"     , args))) return (GMT_ECKERT6);
	if ((*pos = GMT_strlcmp ("eqc/"      , args))) return (GMT_CYL_EQDIST);
	if ((*pos = GMT_strlcmp ("eqdc/"     , args))) return (GMT_ECONIC);
	if ((*pos = GMT_strlcmp ("gnom/"     , args))) return (GMT_GNOMONIC);
	if ((*pos = GMT_strlcmp ("hammer/"   , args))) return (GMT_HAMMER);
	if ((*pos = GMT_strlcmp ("laea/"     , args))) return (GMT_LAMB_AZ_EQ);
	if ((*pos = GMT_strlcmp ("lcc/"      , args))) return (GMT_LAMBERT);
	if ((*pos = GMT_strlcmp ("merc/"     , args))) return (GMT_MERCATOR);
	if ((*pos = GMT_strlcmp ("mill/"     , args))) return (GMT_MILLER);
	if ((*pos = GMT_strlcmp ("moll/"     , args))) return (GMT_MOLLWEIDE);
	if ((*pos = GMT_strlcmp ("nsper/"    , args))) return (GMT_GENPER);
	if ((*pos = GMT_strlcmp ("omerc/"    , args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = GMT_strlcmp ("omercp/"   , args))) return (GMT_OBLIQUE_MERC_POLE);
	if ((*pos = GMT_strlcmp ("ortho/"    , args))) return (GMT_ORTHO);
	if ((*pos = GMT_strlcmp ("polar/"    , args))) return (GMT_POLAR);
	if ((*pos = GMT_strlcmp ("poly/"     , args))) return (GMT_POLYCONIC);
	if ((*pos = GMT_strlcmp ("robin/"    , args))) return (GMT_ROBINSON);
	if ((*pos = GMT_strlcmp ("sinu/"     , args))) return (GMT_SINUSOIDAL);
	if ((*pos = GMT_strlcmp ("stere/"    , args))) return (GMT_STEREO);
	if ((*pos = GMT_strlcmp ("tmerc/"    , args))) return (GMT_TM);
	if ((*pos = GMT_strlcmp ("utm/"      , args))) return (GMT_UTM);
	if ((*pos = GMT_strlcmp ("vandg/"    , args))) return (GMT_VANGRINTEN);
	if ((*pos = GMT_strlcmp ("wintri/"   , args))) return (GMT_WINKEL);
	if ((*pos = GMT_strlcmp ("xy/"       , args))) return (GMT_LINEAR);
	if ((*pos = GMT_strlcmp ("z/"        , args))) return (GMT_ZAXIS);

	/* These older codes (up to GMT 4.2.1) took 2 characters */

	if ((*pos = GMT_strlcmp ("kf", args))) return (GMT_ECKERT4);
	if ((*pos = GMT_strlcmp ("ks", args))) return (GMT_ECKERT6);
	if ((*pos = GMT_strlcmp ("oa", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = GMT_strlcmp ("ob", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = GMT_strlcmp ("oc", args))) return (GMT_OBLIQUE_MERC_POLE);

	/* Finally, check only the first letter (used until GMT 4.2.1) */

	*pos = 1;
	t = (char)tolower(args[0]);
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

GMT_LONG gmt_scale_or_width (struct GMT_CTRL *C, char *scale_or_width, double *value) {
	/* Scans character that may contain a scale (1:xxxx or units per degree) or a width.
	   Return 1 upon error */
	GMT_LONG n;
	C->current.proj.units_pr_degree = strncmp (scale_or_width, "1:", (size_t)2);	/* FALSE if scale given as 1:xxxx */
	if (C->current.proj.units_pr_degree)
		*value = GMT_to_inch (C, scale_or_width);
	else {
		n = sscanf (scale_or_width, "1:%lf", value);
		if (n != 1 || *value < 0.0) return (1);
		*value = 1.0 / (*value * C->current.proj.unit);
		if (C->current.proj.gave_map_width) {
			GMT_report (C, GMT_MSG_FATAL, "Syntax error -J option: Cannot specify map width with 1:xxxx format\n");
			return (1);
		}
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_parse_J_option (struct GMT_CTRL *C, char *args)
{
	/* gmt_parse_J_option scans the arguments given and extracts the parameters needed
	 * for the specified map projection. These parameters are passed through the
	 * C->current.proj structure.  The function returns TRUE if an error is encountered.
	 */

	GMT_LONG i, j, k = 9, m, n, nlen, slash, l_pos[3], p_pos[3], t_pos[3], d_pos[3], id, project;
	GMT_LONG n_slashes = 0, width_given, last_pos, error = FALSE, skip = FALSE;
	double c, az, GMT_units[3] = {0.01, 0.0254, 1.0};      /* No of meters in a cm, inch, m */
	char mod, args_cp[GMT_BUFSIZ], txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256];
	char txt_d[GMT_TEXT_LEN256], txt_e[GMT_TEXT_LEN256], last_char;
	char txt_arr[11][GMT_TEXT_LEN256];

	GMT_memset (l_pos, 3, GMT_LONG);	GMT_memset (p_pos, 3, GMT_LONG);
	GMT_memset (t_pos, 3, GMT_LONG);	GMT_memset (d_pos, 3, GMT_LONG);

	project = gmt_project_type (args, &i, &width_given);
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
		C->current.proj.units_pr_degree = (k == -1) ? TRUE : FALSE;
		C->current.io.col_type[GMT_OUT][GMT_X] = C->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;		/* This may be overridden by mapproject -I */
		if (project != GMT_LINEAR) {
			C->current.proj.gave_map_width = width_given;
			C->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON, C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
		}
	}

	C->current.proj.unit = GMT_units[GMT_INCH];	/* No of meters in an inch */
	n = 0;	/* Initialize with no fields found */

	switch (project) {
		case GMT_LINEAR:	/* Linear x/y scaling */
			C->current.proj.compute_scale[GMT_X] = C->current.proj.compute_scale[GMT_Y] = width_given;

			/* Default is not involving geographical coordinates */
			C->current.io.col_type[GMT_IN][GMT_X] = C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_UNKNOWN;

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
				if (l_pos[GMT_X] || l_pos[GMT_Y] || p_pos[GMT_X] || p_pos[GMT_Y] || t_pos[GMT_X] || t_pos[GMT_Y] || d_pos[GMT_X] || d_pos[GMT_Y]) error = TRUE;
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

			strcpy (args_cp, args);	/* Since GMT_to_inch modifies the string */
			if (slash) args_cp[slash] = 0;	/* Chop off y part */
			if ((i = MAX (l_pos[GMT_X], p_pos[GMT_X])) > 0)
				args_cp[i] = 0;	/* Chop off log or power part */
			else if (t_pos[GMT_X] > 0)
				args_cp[t_pos[GMT_X]] = 0;	/* Chop off time part */
			else if (d_pos[GMT_X] > 0)	/* Chop of trailing 'd' */
				args_cp[d_pos[GMT_X]] = 0;
			if (!skip) {
				if (k >= 0)	/* Scale entered as 1:mmmmm - this implies -R is in meters */
					C->current.proj.pars[0] = C->session.u2u[GMT_M][GMT_INCH] / atof (&args_cp[2]);
				else
					C->current.proj.pars[0] = GMT_to_inch (C, args_cp);	/* x-scale */
			}
			if (l_pos[GMT_X] > 0)
				C->current.proj.xyz_projection[GMT_X] = GMT_LOG10;
			else if (p_pos[GMT_X] > 0) {
				C->current.proj.xyz_projection[GMT_X] = GMT_POW;
				C->current.proj.pars[2] = atof (&args[p_pos[GMT_X]+1]);	/* pow to raise x */
			}
			else if (t_pos[GMT_X] > 0) {	/* Add option to append time_systems or epoch/unit later */
				C->current.proj.xyz_projection[GMT_X] = GMT_TIME;
				C->current.io.col_type[GMT_IN][GMT_X] = (args[t_pos[GMT_X]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME;
			}

			if (d_pos[GMT_X] > 0) C->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;

			if (slash) {	/* Separate y-scaling desired */
				strcpy (args_cp, &args[slash+1]);	/* Since GMT_to_inch modifies the string */
				if ((i = MAX (l_pos[GMT_Y], p_pos[GMT_Y])) > 0)
					args_cp[i-slash-1] = 0;	/* Chop off log or power part */
				else if (t_pos[GMT_Y] > 0)
					args_cp[t_pos[GMT_Y]-slash-1] = 0;	/* Chop off log or power part */
				else if (d_pos[GMT_Y] > 0)
					args_cp[d_pos[GMT_Y]-slash-1] = 0;	/* Chop of trailing 'd' part */
				if (!skip) C->current.proj.pars[1] = GMT_to_inch (C, args_cp);	/* y-scale */

				if (l_pos[GMT_Y] > 0)
					C->current.proj.xyz_projection[GMT_Y] = GMT_LOG10;
				else if (p_pos[GMT_Y] > 0) {
					C->current.proj.xyz_projection[GMT_Y] = GMT_POW;
					C->current.proj.pars[3] = atof (&args[p_pos[GMT_Y]+1]);	/* pow to raise y */
				}
				else if (t_pos[GMT_Y] > 0) {	/* Add option to append time_systems or epoch/unit later */
					C->current.proj.xyz_projection[GMT_Y] = GMT_TIME;
					C->current.io.col_type[GMT_IN][GMT_Y] = (args[t_pos[GMT_Y]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME;
				}
				if (d_pos[GMT_Y] > 0) C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
			}
			else {	/* Just copy x parameters */
				C->current.proj.xyz_projection[GMT_Y] = C->current.proj.xyz_projection[GMT_X];
				if (!skip) C->current.proj.pars[1] = C->current.proj.pars[0];
				C->current.proj.pars[3] = C->current.proj.pars[2];
				if (C->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_GEO) C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
			}

			/* Not both sizes can be zero, but if one is, we will adjust to the scale of the other */
			if (C->current.proj.pars[GMT_X] == 0.0 && C->current.proj.pars[GMT_Y] == 0.0) error = TRUE;
			break;

		case GMT_ZAXIS:	/* 3D plot */
			C->current.proj.compute_scale[GMT_Z] = width_given;
			error = (n_slashes > 0);
			C->current.io.col_type[GMT_IN][GMT_Z] = GMT_IS_UNKNOWN;

			/* Find occurrences of l, p, or t */
			for (j = 0; args[j]; j++) {
				if (strchr ("Ll", (int)args[j])) l_pos[GMT_Z] = j;
				if (strchr ("Pp", (int)args[j])) p_pos[GMT_Z] = j;
				if (strchr ("Tt", (int)args[j])) t_pos[GMT_Z] = j;
			}

			/* Distinguish between p for points and p<power> for scaling */

			n = strlen (args);
			if (p_pos[GMT_Z]) {
				i = p_pos[GMT_Z] + 1;
				if (i == n || strchr ("LlTtDdGg", (int)args[i]))	/* This p is for points since no power is following */
					p_pos[GMT_Z] = 0;
				else if (strchr ("Pp", (int)args[i]))	/* The 2nd p is the p for power */
					p_pos[GMT_Z]++;
			}

			/* Get arguments */

			strcpy (args_cp, args);	/* Since GMT_to_inch modifies the string */
			if ((i = MAX (l_pos[GMT_Z], p_pos[GMT_Z])) > 0)
				args_cp[i] = 0;
			else if (t_pos[GMT_Z] > 0)
				args_cp[t_pos[GMT_Z]] = 0;
			C->current.proj.z_pars[0] = GMT_to_inch (C, args_cp);	/* z-scale */

			if (l_pos[GMT_Z] > 0)
				C->current.proj.xyz_projection[GMT_Z] = GMT_LOG10;
			else if (p_pos[GMT_Z] > 0) {
				C->current.proj.xyz_projection[GMT_Z] = GMT_POW;
				C->current.proj.z_pars[1] = atof (&args[p_pos[GMT_Z]+1]);	/* pow to raise z */
			}
			else if (t_pos[GMT_Z] > 0) {
				C->current.proj.xyz_projection[GMT_Z] = GMT_TIME;
				C->current.io.col_type[GMT_IN][GMT_Z] = (args[t_pos[GMT_Z]] == 'T') ? GMT_IS_ABSTIME : GMT_IS_RELTIME;
			}
			if (C->current.proj.z_pars[0] == 0.0) error = TRUE;
			C->current.proj.JZ_set = TRUE;
			break;

		case GMT_POLAR:		/* Polar (theta,r) */
			C->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON, C->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_FLOAT;
			if (args[0] == 'a' || args[0] == 'A') {
				C->current.proj.got_azimuths = TRUE;	/* using azimuths instead of directions */
				i = 1;
			}
			else {
				C->current.proj.got_azimuths = FALSE;
				i = 0;
			}
			j = strlen (args) - 1;
			if (args[j] == 'r') {	/* Gave optional r for reverse (elevations, presumably) */
				C->current.proj.got_elevations = TRUE;
				args[j] = '\0';	/* Temporarily chop off the r */
			}
			else if (args[j] == 'z') {	/* Gave optional z for annotating depths rather than radius */
				C->current.proj.z_down = TRUE;
				args[j] = '\0';	/* Temporarily chop off the z */
			}
			else
				C->current.proj.got_elevations = C->current.proj.z_down = FALSE;
			if (n_slashes == 1) {	/* Gave optional zero-base angle [0] */
				n = sscanf (args, "%[^/]/%lf", txt_a, &C->current.proj.pars[1]);
				if (n == 2) C->current.proj.pars[0] = GMT_to_inch (C, &txt_a[i]);
				error = (C->current.proj.pars[0] <= 0.0 || n != 2);
			}
			else if (n_slashes == 0) {
				C->current.proj.pars[0] = GMT_to_inch (C, &args[i]);
				n = (args) ? 1 : 0;
				error = (C->current.proj.pars[0] <= 0.0 || n != 1);
			}
			else
				error = TRUE;
			if (C->current.proj.got_elevations) args[j] = 'r';	/* Put the r back in the argument */
			if (C->current.proj.z_down) args[j] = 'z';	/* Put the z back in the argument */
			if (C->current.proj.got_azimuths) C->current.proj.pars[1] = -C->current.proj.pars[1];	/* Because azimuths go clockwise */
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
			C->current.proj.pars[0] = C->session.d_NaN;	/* Will be replaced by central meridian either below or in GMT_map_init_... */
			if (n_slashes == 0)
				n = sscanf (args, "%s", txt_b);
			else if (n_slashes == 1) {
				n = sscanf (args, "%[^/]/%s", txt_a, txt_b);
				error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
			}
			error += gmt_scale_or_width (C, txt_b, &C->current.proj.pars[1]);
			error += !(n = n_slashes + 1);
			break;

		case GMT_CYL_EQ:	/* Cylindrical Equal Area */
		case GMT_CYL_EQDIST:	/* Equidistant Cylindrical */
		case GMT_CYL_STEREO:	/* Cylindrical Stereographic */
		case GMT_CASSINI:	/* Cassini */
		case GMT_MERCATOR:	/* Mercator */
		case GMT_TM:		/* Transverse Mercator */
		case GMT_POLYCONIC:	/* Polyconic */
			C->current.proj.pars[0] = C->session.d_NaN;
			C->current.proj.pars[1] = 0.0;
			txt_a[0] = txt_b[0] = 0;
			if (n_slashes == 0)
				n = sscanf (args, "%s", txt_c);
			else if (n_slashes == 1)
				n = sscanf (args, "%[^/]/%s", txt_a, txt_c);
			else if (n_slashes == 2)
				n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (txt_a[0]) error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
			if (txt_b[0]) error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
			error += gmt_scale_or_width (C, txt_c, &C->current.proj.pars[2]);
			error += ((project == GMT_CYL_EQ || project == GMT_MERCATOR || project == GMT_TM || project == GMT_POLYCONIC)
				&& fabs (C->current.proj.pars[1]) >= 90.0);
			error += !(n = n_slashes + 1);
			break;

		case GMT_ALBERS:	/* Albers Equal-area Conic */
		case GMT_ECONIC:	/* Equidistant Conic */
		case GMT_LAMBERT:	/* Lambert Conformal Conic */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_c, GMT_IS_LAT, &C->current.proj.pars[2]), txt_c);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_d, GMT_IS_LAT, &C->current.proj.pars[3]), txt_d);
			error += gmt_scale_or_width (C, txt_e, &C->current.proj.pars[4]);
			error += (C->current.proj.pars[2] == C->current.proj.pars[3]);
			error += !(n_slashes == 4 && n == 5);
			break;

		case GMT_ORTHO:
			C->current.proj.g_debug = 0;
			C->current.proj.g_box = C->current.proj.g_outside = C->current.proj.g_longlat_set = C->current.proj.g_radius = C->current.proj.g_auto_twist = FALSE;
			C->current.proj.g_sphere = TRUE; /* force spherical as default */
			C->current.proj.pars[5] = C->current.proj.pars[6] = C->current.proj.pars[7] = 0.0;

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
					n = sscanf (args, "%[^/]/%[^/]/1:%lf", txt_a, txt_b, &C->current.proj.pars[3]);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, &C->current.proj.pars[3]);
				if (C->current.proj.pars[3] != 0.0) C->current.proj.pars[3] = 1.0 / (C->current.proj.pars[3] * C->current.proj.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				C->current.proj.pars[3] = GMT_to_inch (C, txt_d);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					C->current.proj.pars[3] = GMT_to_inch (C, txt_d);
					error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_e, GMT_IS_LAT, &C->current.proj.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_c, GMT_IS_LAT, &C->current.proj.pars[2]), txt_c);
			error += (C->current.proj.pars[2] <= 0.0 || C->current.proj.pars[2] > 180.0 || C->current.proj.pars[3] <= 0.0 || (k >= 0 && width_given));
			error += (project == GMT_GNOMONIC && C->current.proj.pars[2] >= 90.0);
			error += (project == GMT_ORTHO && C->current.proj.pars[2] >= 180.0);
			break;

		case GMT_STEREO:	/* Stereographic */
			strcpy (txt_c, "90");	/* Initialize default horizon */
			if (k >= 0) {	/* Scale entered as 1:mmmmm */
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/1:%lf", txt_a, txt_b, &C->current.proj.pars[3]);
				else if (n_slashes == 3) {	/* with true scale at specified latitude */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_e, &C->current.proj.pars[3]);
					error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_e, GMT_IS_LAT, &C->current.proj.pars[4]), txt_e);
					C->current.proj.pars[5] = 1.0;	/* flag for true scale case */
				}
				else if (n_slashes == 4) {
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, txt_e, &C->current.proj.pars[3]);
					error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_e, GMT_IS_LAT, &C->current.proj.pars[4]), txt_e);
					C->current.proj.pars[5] = 1.0;	/* flag for true scale case */
				}
				if (C->current.proj.pars[3] != 0.0) C->current.proj.pars[3] = 1.0 / (C->current.proj.pars[3] * C->current.proj.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				C->current.proj.pars[3] = GMT_to_inch (C, txt_d);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					C->current.proj.pars[3] = GMT_to_inch (C, txt_d);
					error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_e, GMT_IS_LAT, &C->current.proj.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_c, GMT_IS_LON, &C->current.proj.pars[2]), txt_c);
			error += (C->current.proj.pars[2] <= 0.0 || C->current.proj.pars[2] >= 180.0 || C->current.proj.pars[3] <= 0.0 || (k >= 0 && width_given));
			break;

		case GMT_GENPER:	/* General perspective */

			C->current.proj.g_debug = 0;
			C->current.proj.g_box = C->current.proj.g_outside = C->current.proj.g_longlat_set = C->current.proj.g_radius = C->current.proj.g_auto_twist = FALSE;
			C->current.proj.g_sphere = TRUE; /* force spherical as default */

			i = 0;
			for (j = 0 ; j < 2 ; j++) {
				if (args[j] == 'd') {         /* standard genper debugging */
					C->current.proj.g_debug = 1;
					i++;
				} else if (args[j] == 'D') {  /* extensive genper debugging */
					C->current.proj.g_debug = 2;
					i++;
				} else if (args[j] == 'X') {  /* extreme genper debugging */
					C->current.proj.g_debug = 3;
					i++;
				} else if (args[j] == 's') {
					C->current.proj.g_sphere = TRUE;
					i++;
				} else if (args[j] == 'e') {
					C->current.proj.g_sphere = FALSE;
					i++;
				}
			}

			C->current.proj.pars[4] = C->current.proj.pars[5] = C->current.proj.pars[6] = C->current.proj.pars[7] = C->current.proj.pars[8] = C->current.proj.pars[9] = 0.0;

			if (C->current.proj.g_debug > 1) {
				GMT_message (C, "genper: arg '%s' n_slashes %ld k %ld\n", args, n_slashes, j);
				GMT_message (C, "initial error %ld\n", error);
				GMT_message (C, "j = %ld\n", j);
				GMT_message (C, "width_given %ld\n", width_given);
			}

			n = sscanf(args+i, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s",
				&(txt_arr[0][0]), &(txt_arr[1][0]), &(txt_arr[2][0]), &(txt_arr[3][0]),
				&(txt_arr[4][0]), &(txt_arr[5][0]), &(txt_arr[6][0]), &(txt_arr[7][0]),
				&(txt_arr[8][0]), &(txt_arr[9][0]), &(txt_arr[10][0]));

			if (C->current.proj.g_debug > 1) {
				for (i = 0 ; i < n ; i ++) {
					GMT_message (C, "txt_arr[%ld] '%s'\n", i, &(txt_arr[i][0]));
				}
				fflush (NULL);
			}

			if (k >= 0) {
				/* Scale entered as 1:mmmmm */
				m = sscanf(&(txt_arr[n-1][0]),"1:%lf", &C->current.proj.pars[2]);
				if (C->current.proj.pars[2] != 0.0) {
					C->current.proj.pars[2] = 1.0 / (C->current.proj.pars[2] * C->current.proj.unit);
				}
				error += (m == 0) ? 1 : 0;
				if (error) GMT_message (C, "scale entered but couldn't read\n");
			} else  if (width_given) {
				C->current.proj.pars[2] = GMT_to_inch (C, &(txt_arr[n-1][0]));
			} else {
				C->current.proj.pars[2] = GMT_to_inch (C, &(txt_arr[n-2][0]));
				/*            C->current.proj.pars[3] = GMT_ddmmss_to_degree(txt_i); */
				error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, &(txt_arr[n-1][0]), GMT_IS_LAT, &C->current.proj.pars[3]), &(txt_arr[n-1][0]));
				if (error) GMT_message (C, "error in reading last lat value\n");
			}
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, &(txt_arr[0][0]), GMT_IS_LON, &C->current.proj.pars[0]), &(txt_arr[0][0]));
			if (error) GMT_message (C, "error is reading longitude '%s'\n", &(txt_arr[0][0]));
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, &(txt_arr[1][0]), GMT_IS_LAT, &C->current.proj.pars[1]), &(txt_arr[1][0]));
			if (error) GMT_message (C, "error reading latitude '%s'\n", &(txt_arr[1][0]));

			/* g_alt    C->current.proj.pars[4] = atof(txt_c); */
			nlen = strlen(&(txt_arr[2][0]));
			if (txt_arr[2][nlen-1] == 'r') {
				C->current.proj.g_radius = TRUE;
				txt_arr[2][nlen-1] = 0;
			}
			error += GMT_verify_expectations (C, GMT_IS_FLOAT, GMT_scanf (C, &(txt_arr[2][0]), GMT_IS_FLOAT, &C->current.proj.pars[4]), &(txt_arr[2][0]));
			if (error) GMT_message (C, "error reading altitude '%s'\n", &(txt_arr[2][0]));

			/* g_az    C->current.proj.pars[5] = atof(txt_d); */
			nlen = strlen(&(txt_arr[3][0]));
			if (txt_arr[3][nlen-1] == 'l' || txt_arr[3][nlen-1] == 'L') {
				C->current.proj.g_longlat_set = TRUE;
				txt_arr[3][nlen-1] = 0;
			}
			error += GMT_verify_expectations (C, GMT_IS_GEO, GMT_scanf (C, &(txt_arr[3][0]), GMT_IS_GEO, &C->current.proj.pars[5]), &(txt_arr[3][0]));
			if (error) GMT_message (C, "error reading azimuth '%s'\n", &(txt_arr[3][0]));

			/* g_tilt    C->current.proj.pars[6] = atof(txt_e); */
			nlen = strlen(&(txt_arr[4][0]));
			if (txt_arr[4][nlen-1] == 'l' || txt_arr[4][nlen-1] == 'L') {
				C->current.proj.g_longlat_set = TRUE;
				txt_arr[4][nlen-1] = 0;
			}
			error += GMT_verify_expectations (C, GMT_IS_GEO, GMT_scanf (C, &(txt_arr[4][0]), GMT_IS_GEO, &C->current.proj.pars[6]), &(txt_arr[4][0]));
			if (error) GMT_message (C, "error reading tilt '%s'\n", &(txt_arr[4][0]));

			if (n > 6) {
				/*g_twist   C->current.proj.pars[7] = atof(txt_f); */
				nlen = strlen(&(txt_arr[5][0]));
				if (txt_arr[5][nlen-1] == 'n') {
					C->current.proj.g_auto_twist = TRUE;
					txt_arr[5][nlen-1] = 0;
				}
				error += GMT_verify_expectations (C, GMT_IS_GEO, GMT_scanf (C, &(txt_arr[5][0]), GMT_IS_GEO, &C->current.proj.pars[7]), &(txt_arr[5][0]));
				if (error) GMT_message (C, "error reading twist '%s'\n", &(txt_arr[5][0]));

				/*g_width   C->current.proj.pars[8] = atof(txt_f); */
				if (n > 7) {
					error += GMT_verify_expectations (C, GMT_IS_GEO, GMT_scanf (C, &(txt_arr[6][0]), GMT_IS_GEO, &C->current.proj.pars[8]), &(txt_arr[6][0]));
					if (error) GMT_message (C, "error reading width '%s'\n", &(txt_arr[6][0]));

					if (n > 8) {
						/* g_height  C->current.proj.pars[9] = atof(txt_g); */
						error += GMT_verify_expectations (C, GMT_IS_GEO, GMT_scanf (C, &(txt_arr[7][0]), GMT_IS_GEO, &C->current.proj.pars[9]), &(txt_arr[7][0]));
						if (error) GMT_message (C, "error height '%s'\n", &(txt_arr[7][0]));
					}
				}
			}
			error += (C->current.proj.pars[2] <= 0.0 || (k >= 0 && width_given));
			if (error) GMT_message (C, "final error %ld\n", error);
			break;

		case GMT_OBLIQUE_MERC:		/* Oblique mercator, specifying origin and azimuth or second point */
			if (n_slashes == 3) {
				n = sscanf (args, "%[^/]/%[^/]/%lf/%s", txt_a, txt_b, &az, txt_e);
				error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
				error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
				c = 10.0;	/* compute point 10 degrees from origin along azimuth */
				C->current.proj.pars[2] = C->current.proj.pars[0] + atand (sind (c) * sind (az) / (cosd (C->current.proj.pars[1]) * cosd (c) - sind (C->current.proj.pars[1]) * sind (c) * cosd (az)));
				C->current.proj.pars[3] = d_asind (sind (C->current.proj.pars[1]) * cosd (c) + cosd (C->current.proj.pars[1]) * sind (c) * cosd (az));
			}
			else if (n_slashes == 4) {
				n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
				error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
				error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_c, GMT_IS_LON, &C->current.proj.pars[2]), txt_c);
				error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_d, GMT_IS_LAT, &C->current.proj.pars[3]), txt_d);
			}
			error += gmt_scale_or_width (C, txt_e, &C->current.proj.pars[4]);
			C->current.proj.pars[6] = 0.0;
			error += !(n == n_slashes + 1);
			break;

		case GMT_OBLIQUE_MERC_POLE:	/* Oblique mercator, specifying orgin and pole */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_a, GMT_IS_LON, &C->current.proj.pars[0]), txt_a);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_b, GMT_IS_LAT, &C->current.proj.pars[1]), txt_b);
			error += GMT_verify_expectations (C, GMT_IS_LON, GMT_scanf (C, txt_c, GMT_IS_LON, &C->current.proj.pars[2]), txt_c);
			error += GMT_verify_expectations (C, GMT_IS_LAT, GMT_scanf (C, txt_d, GMT_IS_LAT, &C->current.proj.pars[3]), txt_d);
			if (C->current.proj.pars[3] < 0.0) {	/* Flip from S hemisphere to N */
				C->current.proj.pars[3] = -C->current.proj.pars[3];
				C->current.proj.pars[2] += 180.0;
				if (C->current.proj.pars[2] >= 360.0) C->current.proj.pars[2] -= 360.0;
			}
			error += gmt_scale_or_width (C, txt_e, &C->current.proj.pars[4]);
			C->current.proj.pars[6] = 1.0;
			error += !(n_slashes == 4 && n == 5);
			project = GMT_OBLIQUE_MERC;
			break;

		case GMT_UTM:	/* Universal Transverse Mercator */
			n = sscanf (args, "%[^/]/%s", txt_a, txt_b);
			C->current.proj.pars[0] = atof (txt_a);
			switch (args[0]) {
				case '-':	/* Enforce Southern hemisphere convention for y */
					C->current.proj.utm_hemisphere = -1;
					break;
				case '+':	/* Enforce Norther hemisphere convention for y */
					C->current.proj.utm_hemisphere = +1;
					break;
				default:	/* Decide in gmt_map_setup based on -R */
					C->current.proj.utm_hemisphere = 0;
					break;
			}
			mod = (char)toupper ((int)txt_a[strlen(txt_a)-1]);	/* Check if UTM zone has a valid latitude modifier */
			error = 0;
			if (mod >= 'A' && mod <= 'Z') {	/* Got fully qualified UTM zone, e.g., 33N */
				C->current.proj.utm_zoney = (GMT_LONG)mod;
				C->current.proj.utm_hemisphere = -1;
				if (mod >= 'N') C->current.proj.utm_hemisphere = +1;
				if (mod == 'I' || mod == 'O') error++;	/* No such zones */
			}
			C->current.proj.pars[0] = fabs (C->current.proj.pars[0]);
			C->current.proj.utm_zonex = irint (C->current.proj.pars[0]);
			error += gmt_scale_or_width (C, txt_b, &C->current.proj.pars[1]);
			error += !(n_slashes == 1 && n == 2);
			error += (C->current.proj.utm_zonex < 1 || C->current.proj.utm_zonex > 60);	/* Zones must be 1-60 */
			break;

		default:
			error = TRUE;
			project = GMT_NO_PROJ;
			break;
	}

	if (project != GMT_ZAXIS) C->current.proj.projection = project;
	if (width_given > 1) args[last_pos] = last_char;	/* Restore modifier */

	return (error);
}

GMT_LONG gmt_get_unit (char c)
{
	/* Converts c, i, and p into 0,1,3 */

	GMT_LONG i;
	switch ((int)c) {
		case 'c':	/* cm */
			i = GMT_CM;
			break;
		case 'i':	/* inch */
			i = GMT_INCH;
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

GMT_LONG GMT_parse_symbol_option (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd)
{
	/* mode = 0 for 2-D (psxy) and = 1 for 3-D (psxyz); cmd = 1 when called to process command line options */
	GMT_LONG decode_error = 0, bset = 0, j, n, k, len, slash = 0, one, colon, check = TRUE, old_style, col_off = mode;
	char symbol_type, txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256], text_cp[GMT_TEXT_LEN256], *c = NULL, *s = NULL;
	static char *allowed_symbols[2] = {"-+AaBbCcDdEefGgHhIiJjmNnpqRrSsTtVvWwxy", "-+AabCcDdEefGgHhIiJjmNnOopqRrSsTtUuVvWwxy"};
	static char *bar_symbols[2] = {"Bb", "-BbOoUu"};

	p->n_required = p->convert_angles = p->n_nondim = 0;
	p->user_unit = p->shrink = p->read_size = p->base_set = p->u_set = FALSE;
	p->font = C->current.setting.font_annot[0];

	/* Col_off is the col number of first parameter after (x,y) [or (x,y,z) if mode == 1)].
	   However, if size is not given then that is requred too so col_off++ */
	
	if (text[0] != 'q' && (s = strstr (text, "+s"))) {	/* Gave a symbol size scaling relation */
		k = strlen (text) - 1;	/* Last character position */
		s[0] = '\0';		/* Temporarily separate this modifer from the rest of the symbol option (restored at end of function) */
		p->convert_size = (text[k] == 'l') ? 2 : 1;		/* If last char is l we want log10 conversion */
		if (p->convert_size == 2) text[k] = '\0';		/* Temporarily remove the l */
		n = sscanf ((char *)(s+2), "%[^,],%[^l]", txt_a, txt_b);	/* Get the 1-2 pieces */
		if (n == 0) GMT_report (C, GMT_MSG_FATAL, "-S...+s contains bad scale info\n");
		p->scale = GMT_to_inch (C, txt_a);	/* The scale may have units */
		if (n == 2) p->origin = atof (txt_b);	/* Origin is in data units */
		if (p->convert_size == 2 && n == 2) p->origin = log10 (p->origin);	/* To simplify calculations later */
		p->origin *= p->scale;		/* So now size = (given_size | log10(given_size)) * p->scale - p->origin */
		if (p->convert_size == 2) text[k] = 'l';	/* Replace the l we removed */
	}
	if (!text[0]) {	/* No symbol or size given */
		p->size_x = p->size_y = 0.0;
		symbol_type = '*';
		col_off++;
	}
	else if (isdigit ((int)text[0]) || text[0] == '.') {	/* Size, but no symbol given */
		n = sscanf (text, "%[^/]/%s", txt_a, txt_b);
		p->size_x = p->given_size_x = GMT_to_inch (C, txt_a);
		if (n == 2)
			p->size_y = p->given_size_y = GMT_to_inch (C, txt_b);
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
			if (GMT_getfont (C, c, &p->font)) GMT_report (C, GMT_MSG_FATAL, "-Sl contains bad font (set to %s)\n", GMT_putfont (C, p->font));
		}
		if (text[1] == '/') {	/* No size given */
			symbol_type = 'l';
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			col_off++;
		}
		else {
			n = sscanf (text_cp, "%c%[^/]/%s", &symbol_type, txt_a, p->string);
			p->size_x = p->given_size_x = GMT_to_inch (C, txt_a);
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
			p->size_x = p->given_size_x = GMT_to_inch (C, txt_a);
		}
	}
	else if (text[0] == 'm') {	/* mathangle gets separate treatment because of modifiers */
		k = (strchr ("bfl", text[1])) ? 2 : 1;	/* Skip the modifier b, f, or l */
		n = sscanf (text, "%c", &symbol_type);
		if (!text[k]) {	/* No size nor unit */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			col_off++;
		}
		else if (strchr (GMT_DIM_UNITS, (int) text[k])) {	/* No size given, only unit information */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if ((p->u = gmt_get_unit (text[k])) < 0) decode_error = TRUE; else p->u_set = TRUE;
			col_off++;
		}
		else
			p->size_x = p->given_size_x = GMT_to_inch (C, &text[k]);
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && strchr (GMT_DIM_UNITS, (int) text[1])) {	/* Symbol, but no size given (size assumed given on command line), only unit information */
		n = sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		if (text[1] && (p->u = gmt_get_unit (text[1])) < 0) decode_error = TRUE; else p->u_set = TRUE;
		col_off++;
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && (text[1] == '\n' || !text[1])) {	/* Symbol, but no size given (size assumed given on command line) */
		n = sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		col_off++;
	}
	else if (strchr (bar_symbols[mode], (int) text[0])) {	/* Bar, column, cube with size */

		/* Bar:		-Sb|B<size_x|y>[c|i|p|u][b<base>]				*/
		/* Column:	-So|O<size_x>[c|i|p][/<ysize>[c|i|p]][u][b<base>]	*/
		/* Cube:	-Su|U<size_x>[c|i|p|u]	*/

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
				p->size_x = p->given_size_x = GMT_to_inch (C, txt_a);
				p->size_y = p->given_size_y = GMT_to_inch (C, txt_b);
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
					p->size_x = p->given_size_x = GMT_to_inch (C, txt_a);
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
			p->size_x = p->given_size_x = GMT_to_inch (C, txt_a);
			if (n == 3)
				p->size_y = p->given_size_y = GMT_to_inch (C, txt_b);
			else if (n == 2)
				p->size_y = p->given_size_y = p->size_x;
			else
				decode_error = TRUE;
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
			p->size_x *= 1.25331413732;	/* To equal area of circle with same diameter */
		case 'd':
			p->symbol = GMT_SYMBOL_DIAMOND;
			break;
		case 'E':	/* Expect axis in km to be scaled based on -J */
			p->convert_angles = 1;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in km, not inches or cm etc */
			p->nondim_col[p->n_nondim++] = 4 + mode;
		case 'e':
			p->symbol = GMT_SYMBOL_ELLIPSE;
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			check = FALSE;
			break;

		case 'f':	/* Fronts: -Sf<spacing>/<size>[dir][type][:<offset>]	*/
			p->symbol = GMT_SYMBOL_FRONT;
			p->f.f_off = 0.0;
			strcpy (text_cp, text);
			if ((c = strchr (text_cp, ':'))) {	/* Gave :<offset>, set it and strip it off */
				c++;	/* Skip over the colon */
				p->f.f_off = GMT_to_inch (C, c);
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

#ifdef GMT_COMPAT
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
#endif
				default:
					p->f.f_sense = GMT_FRONT_CENTERED;
					break;
			}

			if (old_style) {
				GMT_report (C, GMT_MSG_COMPAT, "Warning in Option -Sf: Symbols l|L|r|R are deprecated in GMT 5\n");
			}
			else {
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
							GMT_report (C, GMT_MSG_FATAL, "Error in Option -Sf: Must specify (l)eft-lateral or (r)ight-lateral slip\n");
							GMT_exit (EXIT_FAILURE);
						}
						break;
				}
			}

			text_cp[len] = 0;	/* Gets rid of the [dir][type] flags, if present */

			/* Pull out and get spacing and size */

			sscanf (&text_cp[1], "%[^/]/%s", txt_a, txt_b);
			p->f.f_gap = (txt_a[0] == '-') ? atof (txt_a) : GMT_to_inch (C, txt_a);
			p->f.f_len = GMT_to_inch (C, txt_b);
			check = FALSE;
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
			p->convert_angles = 1;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in km, not inches or cm etc */
			p->nondim_col[p->n_nondim++] = 4 + mode;
		case 'j':
			p->symbol = GMT_SYMBOL_ROTRECT;
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			check = FALSE;
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
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -Sl option: No string given\n");
				decode_error++;
			}
			p->string[k] = 0;
			break;
		case 'm':
			p->symbol = GMT_SYMBOL_MARC;
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
			p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Angle */
			p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Angle */
			break;
		case 'N':
			p->size_x *= 1.14948092619;	/* To equal area of circle with same diameter */
		case 'n':
			p->symbol = GMT_SYMBOL_PENTAGON;
			break;
		case 'o':	/*3-D symbol */
			p->shade3D = TRUE;
		case 'O':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_COLUMN;
			if (bset) {
				p->base = atof (&text[bset+1]);
				p->base_set = TRUE;
			}
			if (mode == 0) {
				decode_error = TRUE;
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -S option: Symbol type %c is 3-D only\n", symbol_type);
			}
			break;
		case 'P':
		case 'p':
			p->symbol = GMT_SYMBOL_DOT;
			if (p->size_x == 0.0) p->size_x = GMT_DOT_SIZE;
			check = FALSE;
			break;
		case 'q':	/* Quoted lines: -Sq[d|n|l|x]<info>[:<labelinfo>] */
			p->symbol = GMT_SYMBOL_QUOTED_LINE;
			for (j = 1, colon = 0; text[j]; j++) if (text[j] == ':') colon = j;
			if (colon) {	/* Gave :<labelinfo> */
				text[colon] = 0;
				decode_error += GMT_contlabel_info (C, 'S', &text[1], &p->G);
				decode_error += GMT_contlabel_specs (C, &text[colon+1], &p->G);
			}
			else
				decode_error += GMT_contlabel_info (C, 'S', &text[1], &p->G);
			check = FALSE;
			break;
		case 'r':
			p->symbol = GMT_SYMBOL_RECT;
			p->n_required = 2;
			check = FALSE;
			break;
		case 'R':
			p->symbol = GMT_SYMBOL_RNDRECT;
			p->n_required = 3;
			check = FALSE;
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
			p->shade3D = TRUE;
		case 'U':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_CUBE;
			if (mode == 0) {
				decode_error = TRUE;
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -S option: Symbol type %c is 3-D only\n", symbol_type);
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
				k = gmt_get_unit (text[len]);
				if (k >= 0) { p->u = k; p->u_set = TRUE; }
				p->v_norm = atof (&text[j+1]);
				if (p->v_norm > 0.0)
					p->v_shrink = 1.0 / p->v_norm;
				else
					p->v_norm = 0.0;
				text[j] = 0;	/* Chop off the shrink part */
			}
			else
				p->v_norm = -1.0;
			if (text[one]) {
				/* It is possible that the user have appended a unit modifier after
				 * the <size> argument (which here are vector attributes).  We use that
				 * to set the unit, but only if the vector attributes themselves have
				 * units. (If not we would override MEASURE_UNIT without cause).
				 * So, -SV0.1i/0.2i/0.3ic will expect 4th column to have length in cm
				 * while SV0.1i/0.2i/0.3i expects data units in MEASURE_UNIT
				 */

				if (isalpha ((int)text[len]) && isalpha ((int)text[len-1])) {
					p->u = gmt_get_unit (text[len]);
					if (p->u >= 0) p->u_set = TRUE;
					text[len] = 0;
				}
				sscanf (&text[one], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				p->v_width  = GMT_to_inch (C, txt_a);
				p->h_length = GMT_to_inch (C, txt_b);
				p->h_width  = GMT_to_inch (C, txt_c);
			}
			if (p->v_norm >= 0.0) text[j] = 'n';	/* Put back the n<shrink> part */
			p->n_required = 2;
			check = FALSE;
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
			p->custom = GMT_get_custom_symbol (C, text_cp);
			p->n_required = p->custom->n_required;
			break;
		default:
			decode_error = TRUE;
			GMT_report (C, GMT_MSG_FATAL, "Syntax error -S option: Unrecognized symbol type %c\n", symbol_type);
			break;
	}
	if (p->given_size_x == 0.0 && check) {
		p->read_size = TRUE;
		p->n_required++;
		if (p->symbol == GMT_SYMBOL_COLUMN) p->n_required++;
	}
	if (bset || cmd) { /* Since we may not know if we have logarithmic projection at this point, skip the next checks. */ }
	else if (p->symbol == GMT_SYMBOL_BARX)
		p->base = (C->current.proj.xyz_projection[GMT_X] == GMT_LOG10) ? 1.0 : 0.0;
	else if (p->symbol == GMT_SYMBOL_BARY)
		p->base = (C->current.proj.xyz_projection[GMT_Y] == GMT_LOG10) ? 1.0 : 0.0;
	else if (p->symbol == GMT_SYMBOL_COLUMN)
		p->base = (C->current.proj.xyz_projection[GMT_Z] == GMT_LOG10) ? 1.0 : 0.0;

	if (p->convert_size) s[0] = '+';	/* Restore what we temporarily removed */

	return (decode_error);
}

void GMT_init_scales (struct GMT_CTRL *C, GMT_LONG unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name) {
	/* unit is 0-6 and stands for m, km, miles, nautical miles, inch, cm, or point */
	/* fwd_scale is used to convert user distance units to meter */
	/* inv_scale is used to convert meters to user distance units */
	/* inch_to_unit is used to convert internal inches to users units (c, i, p) */
	/* unit_to_inch is used to convert users units (c, i, p) to internal inches */
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

	switch (C->current.setting.proj_length_unit) {
		case GMT_CM:
			*inch_to_unit = 2.54;
			strcpy (unit_name, "cm");
			break;
		case GMT_INCH:
			*inch_to_unit = 1.0;
			strcpy (unit_name, "inch");
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

GMT_LONG GMT_check_scalingopt (struct GMT_CTRL *C, char option, char unit, char *unit_name) {
	GMT_LONG mode;

	switch (unit) {
		case '\0':
		case 'e':
			mode = 0;
			strcpy (unit_name, "m");
			break;
		case 'k':
			mode = 1;
			strcpy (unit_name, "km");
			break;
		case 'M':
			mode = 2;
			strcpy (unit_name, "miles");
			break;
		case 'n':
			mode = 3;
			strcpy (unit_name, "nautical miles");
			break;
		case 'i':
			mode = 4;
			strcpy (unit_name, "inch");
			break;
		case 'c':
			mode = 5;
			strcpy (unit_name, "cm");
			break;
		case 'p':
			mode = 6;
			strcpy (unit_name, "point");
			break;
		case 'f':
			mode = 7;
			strcpy (unit_name, "feet");
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "GMT ERROR Option -%c: Only append one of %s|%s\n", option, GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS2_DISPLAY);
			GMT_exit (EXIT_FAILURE);
	}

	return (mode);
}

GMT_LONG GMT_set_measure_unit (struct GMT_CTRL *C, char unit) {
	/* Option to override the GMT measure unit default */
	GMT_LONG k;

	if ((k = gmt_get_unit (unit)) < 0) return (GMT_MAP_BAD_MEASURE_UNIT);
	C->current.setting.proj_length_unit = k;
	return (GMT_NOERROR);
}

#ifdef GMT_COMPAT
GMT_LONG backwards_SQ_parsing (struct GMT_CTRL *C, char option, char *item) {
	/* Use to parse various -S -Q options when backwardsness has been enabled */
	GMT_LONG j;
	
	GMT_report (C, GMT_MSG_COMPAT, "Warning: Option -%c[-]<mode>[/<threshold>] is deprecated. Use -n<mode>[+a][+t<threshold>] instead.\n", (int)option);
	
	for (j = 0; j < 3 && item[j]; j++) {
		switch (item[j]) {
			case '-':
				C->common.n.antialias = FALSE; break;
			case 'n':
				C->common.n.interpolant = BCR_NEARNEIGHBOR; break;
			case 'l':
				C->common.n.interpolant = BCR_BILINEAR; break;
			case 'b':
				C->common.n.interpolant = BCR_BSPLINE; break;
			case 'c':
				C->common.n.interpolant = BCR_BICUBIC; break;
			case '/':
				C->common.n.threshold = atof (&item[j+1]);
				if (C->common.n.threshold < 0.0 || C->common.n.threshold > 1.0) {
					GMT_report (C, GMT_MSG_FATAL, "Error: Interpolation threshold must be in [0,1] range\n");
					return (1);
				}
				break;
			default:
				GMT_report (C, GMT_MSG_FATAL, "Syntax error: Specify -%c[-]b|c|l|n[/threshold] to set grid interpolation mode.\n", option);
				return (1);
				break;
		}
	}
	return (GMT_NOERROR);
}
#endif

#define GMT_more_than_once(C,active) (GMT_check_condition (C, active, "Warning: Option -%c given more than once\n", option))

GMT_LONG GMT_parse_common_options (struct GMT_CTRL *C, char *list, char option, char *item)
{
	/* GMT_parse_common_options interprets the command line for the common, unique options
	 * -B, -J, -K, -O, -P, -R, -U, -V, -X, -Y, -b, -c, -f, -g, -h, -i, -n, -o, -p, -r, -s, -t, -:, -- and -^.
	 * The list passes all of these that we should consider.
	 */

	GMT_LONG error = 0, i, j_type;

	if (!list || !strchr (list, option)) return (FALSE);	/* Not a common option we accept */

#ifdef GMT_COMPAT
#define GMT_COMPAT_OPT(new) if (strchr (list, new)) { GMT_report (C, GMT_MSG_COMPAT, "Warning: Option -%c is deprecated. Use -%c instead.\n", option, new); option = new; }
	/* Translate some options */
	switch (option) {
		case 'E': GMT_COMPAT_OPT ('p'); break;
		case 'F': GMT_COMPAT_OPT ('r'); break;
		case 'H': GMT_COMPAT_OPT ('h'); break;
	}
#endif

	switch (option) {	/* Handle parsing of this option, if allowed here */

		case 'B':
			switch (item[0]) {	/* Check for -B[p] and -Bs */
				case 's':
					//error += GMT_check_condition (C, C->common.B.active[1], "Warning: Option -Bs given more than once\n");
					C->common.B.active[1] = TRUE;
					break;
				default:
					//error += GMT_check_condition (C, C->common.B.active[0], "Warning: Option -B[p] given more than once\n");
					C->common.B.active[0] = TRUE;
					break;
			}
			if (!error) error = gmt_parse_B_option (C, item);
			break;

		case 'J':
			j_type = (item && (item[0] == 'Z' || item[0] == 'z')) ? 2 : 1;
			error += (GMT_check_condition (C, C->common.J.active & j_type, "Warning: Option -J given more than once\n") || gmt_parse_J_option (C, item));
			C->common.J.active |= j_type;
			break;

		case 'K':
			GMT_more_than_once (C, C->common.K.active);
			C->common.K.active = TRUE;
			break;

		case 'O':
			GMT_more_than_once (C, C->common.O.active);
			C->common.O.active = TRUE;
			break;

		case 'P':
			GMT_more_than_once (C, C->common.P.active);
			C->common.P.active = TRUE;
			break;

#ifdef GMT_COMPAT
		case 'Q':
		case 'S':
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Option -%c is deprecated. Use -n instead.\n", option);
			error += backwards_SQ_parsing (C, option, item);
			break;
#endif

		case 'R':
			error += (GMT_more_than_once (C, C->common.R.active) || gmt_parse_R_option (C, item));
			C->common.R.active = TRUE;
			break;

		case 'U':
			error += (GMT_more_than_once (C, C->common.U.active) || gmt_parse_U_option (C, item));
			C->common.U.active = TRUE;
			break;

		case 'V':
			GMT_more_than_once (C, C->common.V.active);
			C->common.V.active = TRUE;
			if (item && item[0]) {	/* Specified a verbosity level */
				i = atoi (item);
				if (i < GMT_MSG_SILENCE || i > GMT_MSG_DEBUG) {
					GMT_report (C, -GMT_MSG_FATAL, "Error: Option -V verbosity levels are %d-%d\n", GMT_MSG_SILENCE, GMT_MSG_DEBUG);
					error++;
				}
				else
					C->current.setting.verbose = i;
			}
			else
				C->current.setting.verbose = GMT_MSG_NORMAL;
			break;

		case 'x':
		case 'X':
			error += (GMT_more_than_once (C, C->common.X.active) || gmt_parse_XY_option (C, GMT_X, item));
			C->common.X.active = TRUE;
			break;

		case 'y':
		case 'Y':
			error += (GMT_more_than_once (C, C->common.Y.active) || gmt_parse_XY_option (C, GMT_Y, item));
			C->common.Y.active = TRUE;
			break;

#ifdef GMT_COMPAT
		case 'Z':	/* Backwards compatibility */
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Option -Z[<zlevel>] is deprecated. Use -p<azim>/<elev>[/<zlevel>] instead.\n");
			if (item && item[0]) C->current.proj.z_level = atof (item);
			break;
#endif

		case 'a':
			error += (GMT_more_than_once (C, C->common.a.active) || gmt_parse_a_option (C, item));
			C->common.a.active = TRUE;
			break;

		case 'b':
			switch (item[0]) {
				case 'i':
					error += GMT_check_condition (C, C->common.b.active[GMT_IN], "Warning Option -bi given more than once\n");
					C->common.b.active[GMT_IN] = TRUE;
					break;
				case 'o':
					error += GMT_check_condition (C, C->common.b.active[GMT_OUT], "Warning Option -bo given more than once\n");
					C->common.b.active[GMT_OUT] = TRUE;
					break;
				default:
					error += GMT_check_condition (C, C->common.b.active[GMT_IN] + C->common.b.active[GMT_OUT], "Warning Option -b given more than once\n");
					C->common.b.active[GMT_IN] = C->common.b.active[GMT_OUT] = TRUE;
					break;
			}
			error += gmt_parse_b_option (C, item);
			break;

		case 'c':
			error += (GMT_more_than_once (C, C->common.c.active) || gmt_parse_c_option (C, item));
			C->common.c.active = TRUE;
			break;

		case 'f':
			switch (item[0]) {
				case 'i':
					error += GMT_check_condition (C, C->common.f.active[GMT_IN], "Warning Option -fi given more than once\n");
					C->common.f.active[GMT_IN] = TRUE;
					break;
				case 'o':
					error += GMT_check_condition (C, C->common.f.active[GMT_OUT], "Warning Option -fo given more than once\n");
					C->common.f.active[GMT_OUT] = TRUE;
					break;
				default:
					error += GMT_check_condition (C, C->common.f.active[GMT_IN] | C->common.f.active[GMT_OUT], "Warning Option -f given more than once\n");
					C->common.f.active[GMT_IN] = C->common.f.active[GMT_OUT] = TRUE;
					break;
			}
			error += gmt_parse_f_option (C, item);
			break;

		case 'g':
			error += gmt_parse_g_option (C, item);
			C->common.g.active = TRUE;
			break;

		case 'h':
			error += (GMT_more_than_once (C, C->common.h.active) || gmt_parse_h_option (C, item));
			C->common.h.active = TRUE;
			break;

		case 'i':
			error += (GMT_more_than_once (C, C->common.i.active) || gmt_parse_i_option (C, item));
			C->common.i.active = TRUE;
			break;

#ifdef GMT_COMPAT
		case 'M':	/* Backwards compatibility */
		case 'm':
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Option -%c is deprecated. Segment headers are automatically identified.\n", option);
			break;
#endif

		case 'n':
			error += (GMT_more_than_once (C, C->common.n.active) || gmt_parse_n_option (C, item));
			C->common.n.active = TRUE;
			break;

		case 'o':
			error += (GMT_more_than_once (C, C->common.o.active) || gmt_parse_o_option (C, item));
			C->common.o.active = TRUE;
			break;

		case 'p':
			error += (GMT_more_than_once (C, C->common.p.active) || gmt_parse_p_option (C, item));
			C->common.p.active = TRUE;
			break;

		case 'r':
			error += GMT_more_than_once (C, C->common.r.active);
			C->common.r.active = TRUE;
			break;

		case 's':
			error += (GMT_more_than_once (C, C->common.s.active) || gmt_parse_s_option (C, item));
			C->common.s.active = TRUE;
			break;

		case 't':
			error += GMT_more_than_once (C, C->common.t.active);
			C->common.t.active = TRUE;
			C->common.t.value = atof (item);
			break;

		case ':':
			error += (GMT_more_than_once (C, C->common.colon.active) || gmt_parse_colon_option (C, item));
			C->common.colon.active = TRUE;
			break;

		case '^':
			if (C->common.synopsis.active) GMT_report (C, GMT_MSG_FATAL, "Warning: Option - given more than once\n");
			C->common.synopsis.active = TRUE;
			break;

		case '-':
			GMT_hash_init (C, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);
			error += gmt_parse_dash_option (C, item);
			gmt_free_hash (C, keys_hashnode, GMT_N_KEYS);
			break;

		case '>':	/* Registered output file; nothing to do here */
			break;

		default:	/* Here we end up if an unrecognized option is passed (should not happen, though) */
			GMT_report (C, GMT_MSG_FATAL, "Option -%c is not a recognized common option\n", option);
			return (TRUE);
			break;
	}

	/* On error, give syntax message */

	if (error) GMT_syntax (C, option);

	return (error);
}

GMT_LONG gmt_scanf_epoch (struct GMT_CTRL *C, char *s, GMT_LONG *rata_die, double *t0) {

	/* Read a string which must be in one of these forms:
		[-]yyyy-mm-dd[T| [hh:mm:ss.sss]]
		[-]yyyy-Www-d[T| [hh:mm:ss.sss]]
	   Hence, data and clock can be separated by 'T' or ' ' (space), and the clock string is optional.
	   In fact, seconds can be decimal or integer, or missing. Minutes and hour are optional too.
	   Examples: 2000-01-01, 2000-01-01T, 2000-01-01 00:00, 2000-01-01T00, 2000-01-01T00:00:00.000
	*/

	double ss = 0.0;
	GMT_LONG i, yy, mo, dd, hh = 0, mm = 0, rd;
	char tt[8];

	i = 0;
	while (s[i] && s[i] == ' ') i++;
	if (!(s[i])) return (-1);
	if (strchr (&s[i], 'W') ) {	/* ISO calendar string, date with or without clock */
		if (sscanf (&s[i], "%5" GMT_LL "d-W%2" GMT_LL "d-%1" GMT_LL "d%[^0-9:-]%2" GMT_LL "d:%2" GMT_LL "d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (GMT_iso_ywd_is_bad (yy, mo, dd) ) return (-1);
		rd = GMT_rd_from_iywd (C, yy, mo, dd);
	}
	else {				/* Gregorian calendar string, date with or without clock */
		if (sscanf (&s[i], "%5" GMT_LL "d-%2" GMT_LL "d-%2" GMT_LL "d%[^0-9:-]%2" GMT_LL "d:%2" GMT_LL "d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (GMT_g_ymd_is_bad (yy, mo, dd) ) return (-1);
		rd = GMT_rd_from_gymd (C, yy, mo, dd);
	}
	if (GMT_hms_is_bad (hh, mm, ss)) return (-1);

	*rata_die = rd;								/* Rata day number of epoch */
	*t0 =  (GMT_HR2SEC_F * hh + GMT_MIN2SEC_F * mm + ss) * GMT_SEC2DAY;	/* Fractional day (0<= t0 < 1) since rata_die of epoch */
	return (GMT_NOERROR);
}

GMT_LONG GMT_init_time_system_structure (struct GMT_CTRL *C, struct GMT_TIME_SYSTEM *time_system) {
	/* Processes strings time_system.unit and time_system.epoch to produce a time system scale
	   (units in seconds), inverse scale, and rata die number and fraction of the epoch (days).
	   Return values: 0 = no error, 1 = unit error, 2 = epoch error, 3 = unit and epoch error.
	*/
	GMT_LONG error = GMT_NOERROR;

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
#ifdef GMT_COMPAT
		case 'c':
		case 'C':
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Unit c (seconds) is deprecated; use s instead.\n");
			time_system->scale = 1.0;
			break;
#endif
		default:
			error += 1;
			break;
	}

	/* Set inverse scale and store it to avoid divisions later */
	time_system->i_scale = 1.0 / time_system->scale;

	/* Now convert epoch into rata die number and fraction */
	if (gmt_scanf_epoch (C, time_system->epoch, &time_system->rata_die, &time_system->epoch_t0)) error += 2;

	if (error & 1) {
		GMT_report (C, GMT_MSG_FATAL, "Warning: TIME_UNIT is invalid.  Default assumed.\n");
		GMT_report (C, GMT_MSG_FATAL, "Choose one only from y o d h m s\n");
		GMT_report (C, GMT_MSG_FATAL, "Corresponding to year month day hour minute second\n");
		GMT_report (C, GMT_MSG_FATAL, "Note year and month are simply defined (365.2425 days and 1/12 of a year)\n");
	}
	if (error & 2) {
		GMT_report (C, GMT_MSG_FATAL, "Warning: TIME_EPOCH format is invalid.  Default assumed.\n");
		GMT_report (C, GMT_MSG_FATAL, "    A correct format has the form [-]yyyy-mm-ddThh:mm:ss[.xxx]\n");
		GMT_report (C, GMT_MSG_FATAL, "    or (using ISO weekly calendar)   yyyy-Www-dThh:mm:ss[.xxx]\n");
		GMT_report (C, GMT_MSG_FATAL, "    An example of a correct format is:  2000-01-01T12:00:00\n");
	}
	return (error);
}

void GMT_set_pad (struct GMT_CTRL *C, GMT_LONG pad)
{	/* Changes the 4 GMT default pad values to given isotropic pad */
	C->current.io.pad[XLO] = C->current.io.pad[XHI] = C->current.io.pad[YLO] = C->current.io.pad[YHI] = pad;
}

GMT_LONG GMT_init_fonts (struct GMT_CTRL *C)
{
	GMT_LONG i = 0, n_GMT_fonts, n_alloc = 0;
	char buf[GMT_BUFSIZ], fullname[GMT_BUFSIZ];
	FILE *in = NULL;

	/* Loads the available fonts for this installation */

	/* First the standard 35 PostScript fonts from Adobe */

	GMT_getsharepath (C, "pslib", "PS_font_info", ".d", fullname);
	if ((in = fopen (fullname, "r")) == NULL) {
		GMT_report (C, -GMT_MSG_FATAL, "Error: Cannot open %s", fullname);
		GMT_exit (EXIT_FAILURE);
	}

	GMT_set_meminc (C, GMT_SMALL_CHUNK);	/* Only allocate a small amount */
	while (fgets (buf, GMT_BUFSIZ, in)) {
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
		if (i == n_alloc) n_alloc = GMT_malloc (C, C->session.font, i, n_alloc, struct GMT_FONTSPEC);
		if (sscanf (buf, "%s %lf %*d", fullname, &C->session.font[i].height) != 2) {
			GMT_report (C, GMT_MSG_FATAL, "Error: Trouble decoding font info for font %ld\n", i);
			GMT_exit (EXIT_FAILURE);
		}
		C->session.font[i++].name = strdup (fullname);
	}
	fclose (in);
	C->session.n_fonts = n_GMT_fonts = i;

	/* Then any custom fonts */

	if (GMT_getsharepath (C, "pslib", "CUSTOM_font_info", ".d", fullname)) {	/* Decode Custom font file */
		if ((in = fopen (fullname, "r")) == NULL) {
			GMT_report (C, -GMT_MSG_FATAL, "Error: Cannot open %s", fullname);
			GMT_exit (EXIT_FAILURE);
		}

		while (fgets (buf, GMT_BUFSIZ, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			if (i == n_alloc) n_alloc = GMT_malloc (C, C->session.font, i, n_alloc, struct GMT_FONTSPEC);
			if (sscanf (buf, "%s %lf %*d", fullname, &C->session.font[i].height) != 2) {
				GMT_report (C, GMT_MSG_FATAL, "Error: Trouble decoding custom font info for font %ld\n", i - n_GMT_fonts);
				GMT_exit (EXIT_FAILURE);
			}
			C->session.font[i++].name = strdup (fullname);
		}
		fclose (in);
		C->session.n_fonts = i;
	}
	(void)GMT_malloc (C, C->session.font, 0, i, struct GMT_FONTSPEC);
	GMT_reset_meminc (C);
	return (GMT_NOERROR);
}

struct GMT_CTRL *New_GMT_Ctrl () {	/* Allocate and initialize a new common control structure */
	GMT_LONG i;
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
	struct GMT_CTRL *C = NULL;
	struct ELLIPSOID ref_ellipsoid[GMT_N_ELLIPSOIDS] = {   /* This constant is created by GNUmakefile - do not edit */
	#include "gmt_ellipsoids.h"	/* This include file is created by GNUmakefile - do not edit */
	};
	struct DATUM datum[GMT_N_DATUMS] = {     /* This constant is created by GNUmakefile - do not edit */
	#include "gmt_datums.h"		/* This include file is created by GNUmakefile - do not edit */
	};

	/* Alloc using calloc since GMT_memory may use resources not yet initialized */
	C = (struct GMT_CTRL *) calloc ((size_t)1, sizeof (struct GMT_CTRL));
	GMT_memcpy (C->current.setting.ref_ellipsoid, ref_ellipsoid, 1, ref_ellipsoid);
	GMT_memcpy (C->current.setting.proj_datum, datum, 1, datum);

	/* Assign the three std* pointers */

	C->session.std[GMT_IN]  = stdin;
	C->session.std[GMT_OUT] = stdout;
	C->session.std[GMT_ERR] = stderr;

#ifdef DEBUG
	GMT_memtrack_init (C, &GMT_mem_keeper);	/* Helps us determine memory leaks */
	C->session.min_meminc = GMT_MIN_MEMINC;
	C->session.max_meminc = GMT_MAX_MEMINC;
#endif

	GMT_set_env (C);	/* Get GMT_SHAREDIR and other environment path parameters */

	GMT_init_fonts (C);	/* Load in available font names */

	/* Set up hash table for colornames (used to convert <colorname> to <r/g/b>) */

	GMT_hash_init (C, C->session.rgb_hashnode, GMT_color_name, GMT_N_COLOR_NAMES, GMT_N_COLOR_NAMES);

	/* Initialize values whose defaults are not necessarily 0/FALSE/NULL */

	/* MAP settings */

	GMT_init_distaz (C, GMT_MAP_DIST_UNIT, GMT_GREATCIRCLE, GMT_MAP_DIST);	/* Default spherical distance calculations in m */

	C->current.map.n_lon_nodes = 360;
	C->current.map.n_lat_nodes = 180;
	C->current.map.frame.check_side = C->current.map.frame.horizontal = FALSE;
	C->current.map.dlon = (C->common.R.wesn[XHI] - C->common.R.wesn[XLO]) / C->current.map.n_lon_nodes;
	C->current.map.dlat = (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) / C->current.map.n_lat_nodes;

	/* PLOT settings */

	C->current.plot.mode_3D = 3;	/* Draw both fore and/or back 3-D box lines [1 + 2] */

	/* PROJ settings */

	C->current.proj.projection = GMT_NO_PROJ;
	/* We need some defaults here for the cases where we do not actually set these with GMT_map_setup */
	C->current.proj.fwd_x = C->current.proj.fwd_y = C->current.proj.fwd_z = (PFL) GMT_translin;
	C->current.proj.inv_x = C->current.proj.inv_y = C->current.proj.inv_z = (PFL) GMT_itranslin;
	/* z_level will be updated in GMT_init_three_D, but if it doesn't, it does not matter,
	 * because by default, z_scale = 0.0 */
	C->current.proj.z_level = DBL_MAX;
	C->current.proj.xyz_pos[GMT_X] = C->current.proj.xyz_pos[GMT_Y] = C->current.proj.xyz_pos[GMT_Z] = TRUE;
	C->current.proj.z_project.view_azimuth = 180.0;
	C->current.proj.z_project.view_elevation = 90.0;
	C->current.proj.z_project.plane = -1;	/* Initialize no perspective projection */
	C->current.proj.z_project.level = 0.0;
	for (i = 0; i < 4; i++) C->current.proj.edge[i] = TRUE;
	GMT_grdio_init (C);
	GMT_set_pad (C, 2);	/* Default is to load in grids with 2 rows/cols for boundary padding */
	C->current.proj.f_horizon = 90.0;
	C->current.proj.proj4 = GMT_memory (C, NULL, GMT_N_PROJ4, struct GMT_PROJ4);
	for (i = 0; i < GMT_N_PROJ4; i++) {	/* Load up proj4 structure once and for all */
		C->current.proj.proj4[i].name = strdup (GMT_proj4[i].name);
		C->current.proj.proj4[i].id = GMT_proj4[i].id;
	}
	/* TIME_SYSTEM settings */
	strcpy (C->current.setting.time_system.epoch, "2000-01-01T12:00:00");
	C->current.setting.time_system.unit = 'd';

	/* INIT settings */

	GMT_memcpy (C->session.u2u, u2u, 1, u2u);
	for (i = 0; i < 4; i++) strcpy (C->session.unit_name[i], unit_name[i]);
	GMT_make_fnan (C->session.f_NaN);
	GMT_make_dnan (C->session.d_NaN);
	for (i = 0; i < 3; i++) C->session.no_rgb[i] = -1.0;

	return (C);
}

struct GMT_CTRL *GMT_begin (char *session, GMT_LONG mode)
{
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
	 */

	char *module_name = "GMT_begin";
	char path[GMT_TEXT_LEN256];
	struct GMT_CTRL *C = NULL;

#ifdef __FreeBSD__
#ifdef _i386_
	/* allow divide by zero -- Inf */
	fpsetmask (fpgetmask () & ~(FP_X_DZ | FP_X_INV));
#endif
#endif
	C = (struct GMT_CTRL *)New_GMT_Ctrl ();		/* Allocate and initialize a new common control structure */
	C->init.progname = strdup (session);		/* We use the calling programs session name as program name */
	C->init.module_name = module_name;		/* This will be reset by the GMT modules we call */

	if (mode == GMTAPI_GMTPSL) {			/* The application will need PSL */
		C->PSL = (struct PSL_CTRL *)New_PSL_Ctrl (session);	/* Allocate a PSL control structure */
		if (!C->PSL) {
			GMT_report (C, -GMT_MSG_FATAL, "Error: Could not initialize PSL - Aborting.\n");
			return (NULL);
		}
		C->PSL->init.unit = PSL_INCH;					/* We use inches internally in PSL */
		PSL_beginsession (C->PSL);					/* Initializes the session and sets a few defaults */
		/* Reset session defaults to the chosen GMT settings; these are fixed for the entire PSL session */
		PSL_setdefaults (C->PSL, C->current.setting.ps_magnify, C->current.setting.ps_page_rgb, C->current.setting.ps_encoding.name);
	}

	GMT_io_init (C);		/* Init the table i/o structure before parsing GMT defaults */

	/* Initialize the standard GMT system default settings from the system file */

	sprintf (path, "%s/conf/gmt.conf", C->session.SHAREDIR);
	if (access (path, R_OK)) {
		GMT_report (C, -GMT_MSG_FATAL, "Error: Could not find system defaults file - Aborting.\n");
		return (NULL);
	}
	GMT_loaddefaults (C, path);	/* Load GMT system default settings [and PSL settings if selected] */
	GMT_getdefaults (C, CNULL);	/* Override using local GMT default settings (if any) [and PSL if selected] */

	/* There is no longer a -m option in GMT 5 so multi segments are now always TRUE.
	   However, in GMT_COMPAT mode the -mi and -mo options WILL turn off multi in the other direction. */
	C->current.io.multi_segments[GMT_IN] = TRUE;
	C->current.io.multi_segments[GMT_OUT] = FALSE;	/* Will be turned TRUE when either of two situation arises: */
	/* 1. We read a multisegment header
	   2. The -g option is set which will create gaps and thus multiple segments
	 */

	/* Initialize the output and plot format machinery for ddd:mm:ss[.xxx] strings from the default format strings.
	 * While this is also done in the default parameter loop it is possible that when a decimal plain format has been selected
	 * the format_float_out string has not yet been processed.  We clear that up by processing again here. */

	gmt_geo_C_format (C);
	gmt_plot_C_format (C);
	
	/* Set default for -n parameters */
	C->common.n.antialias = TRUE; C->common.n.interpolant = BCR_BICUBIC; C->common.n.threshold = 0.5;

	gmt_get_history (C);	/* Process and store command shorthands passed to the application */

	if (C->current.setting.io_gridfile_shorthand) gmt_setshorthand (C);	/* Load the short hand mechanism from .gmt_io */

	return (C);
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

void GMT_setmode (struct GMT_CTRL *C, int direction)
{
	/* Changes the stream to deal with BINARY rather than TEXT data */

	FILE *fp = NULL;
	static const char *IO_direction[2] = {"Input", "Output"};

	if (C->common.b.active[direction]) {	/* User wants native binary i/o */

		fp = (direction == 0) ? C->session.std[GMT_IN] : C->session.std[GMT_OUT];
		fflush (fp);	/* Should be untouched but anyway... */
#ifdef _WIN32
		GMT_report (C, GMT_MSG_DEBUG, "Set binary mode for %s\n", IO_direction[direction]);
		setmode (fileno (fp), _O_BINARY);
#else
		_fsetmode (fp, "b");
#endif
	}
}

#endif	/* SET_IO_MODE */

#if defined (WIN32) || defined (__MINGW32__)
#include <stdarg.h>
#ifdef GMT_MATLAB
#include "mex.h"
#endif
/* Due to the DLL boundary cross problem on Windows we are forced to have the following, otherwise
   defined as macro, implemented as a function. */
int GMT_message (struct GMT_CTRL *C, char *format, ...) {
#ifdef GMT_MATLAB
	char line[GMT_BUFSIZ];
#endif
	va_list args;
	va_start (args, format);
#ifdef GMT_MATLAB
	/* Version used by Matlab MEXs that are not able to print to stdout/stderr */
	vsnprintf (line, GMT_BUFSIZ, format, args);
	mexPrintf ("%s", line);
#else
	vfprintf (C->session.std[GMT_ERR], format, args);
#endif
	va_end (args);

	return (0);
}

int GMT_fprintf (FILE *stream, char *format, ...) {
	va_list args;
	va_start (args, format);
	vfprintf (stream, format, args);
	va_end (args);

	return (0);
}
#if 0
/* Comment out for now since not used for now */
int GMT_fscanf (FILE *stream, char *format, ...) {
	va_list args;
	va_start (args, format);
	vfscanf (stream, format, args);
	va_end (args);

	return (0);
}
#endif
#endif
