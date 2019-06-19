/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * The PUBLIC functions (68) are:
 *
 *	gmtlib_explain_options		Prints explanations for the common options\n
 *	gmt_parse_common_options	Interprets common options, such as -B, -R, --\n
 *	gmt_getdefaults			Initializes the GMT global parameters\n
 *	gmt_putdefaults			Dumps the GMT global parameters\n
 *	gmt_pickdefaults
 *	gmt_setdefaults
 *	gmt_hash_init			Initializes a hash\n
 *	gmt_hash_lookup			Key - id lookup using hashing\n
 *	gmt_begin			Gets history and init parameters\n
 *	gmt_end				Cleans up and returns\n
 *	gmt_putcolor			Encode color argument into textstring\n
 *	gmt_putrgb			Encode color argument into r/g/b textstring\n
 *	gmtlib_puthsv			Encode color argument into h-s-v textstring\n
 *	gmtlib_putcmyk			Encode color argument into c/m/y/k textstring
 *	gmtlib_putfill
 *	gmtlib_setparameter		Sets a default value given keyword,value-pair\n
 *	gmtlib_putparameter
 *	gmt_GSHHG_syntax
 *	gmt_label_syntax
 *	gmt_cont_syntax
 *	gmt_inc_syntax
 *	gmt_fill_syntax
 *	gmt_pen_syntax
 *	gmt_rgb_syntax
 *	gmt_refpoint_syntax
 *	gmt_mapinset_syntax
 *	gmt_mapscale_syntax
 *	gmt_maprose_syntax
 *	gmt_mappanel_syntax
 *	gmt_dist_syntax
 *	gmt_vector_syntax
 *	gmt_segmentize_syntax
 *	gmt_img_syntax
 *	gmt_syntax
 *	gmt_default_error
 *	gmt_check_region
 *	gmt_parse_R_option
 *	gmt_parse_range
 *	gmt_parse_d_option
 *	gmt_parse_i_option
 *	gmt_parse_o_option
 *	gmt_parse_model
 *	gmt_parse_segmentize
 *	gmt_check_binary_io
 *	gmt_parse_g_option
 *	gmt_get_V
 *	gmt_convert_units
 *	gmtlib_unit_lookup
 *	gmt_get_time_system
 *	gmt_init_vector_param
 *	gmt_parse_vector
 *	gmt_parse_symbol_option
 *	gmt_init_scales
 *	gmtlib_get_unit_number
 *	gmt_check_scalingopt
 *	gmt_set_measure_unit
 *	gmt_set_pad
 *	gmt_check_filearg
 *	gmt_setmode
 *	gmt_message
 *	gmtlib_report_func
 *	gmtlib_get_num_processors
 *	gmt_loaddefaults		Reads the GMT global parameters from gmt.conf\n
 *	gmt_get_ellipsoid		Returns ellipsoid id based on name\n
 *	gmt_init_time_system_structure  Does what it says\n
 */

#include "gmt_dev.h"
#include <stdarg.h>
#include "gmt_internals.h"
#include "common_runpath.h"

#ifdef GMT_MATLAB
#	include <mex.h>
#endif

#define USER_MEDIA_OFFSET 1000

#define GMT_def(case_val) *GMT->session.u2u[GMT_INCH][gmtlib_unit_lookup(GMT, GMT->current.setting.given_unit[case_val], GMT->current.setting.proj_length_unit)], GMT->current.setting.given_unit[case_val]

#define GMT_more_than_once(GMT,active) (gmt_M_check_condition (GMT, active, "Option -%c given more than once\n", option))

#define GMT_COMPAT_INFO "Please see " GMT_DOC_URL "/changes.html#new-features-in-gmt-5 for more information.\n"
#define GMT_COMPAT_WARN GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Parameter %s is deprecated.\n" GMT_COMPAT_INFO, GMT_keywords[case_val])
#define GMT_COMPAT_CHANGE(new_P) GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Parameter %s is deprecated. Use %s instead.\n" GMT_COMPAT_INFO, GMT_keywords[case_val], new_P)
#define GMT_COMPAT_TRANSLATE(new_P) error = (gmt_M_compat_check (GMT, 4) ? GMT_COMPAT_CHANGE (new_P) + gmtlib_setparameter (GMT, new_P, value, core) : gmtinit_badvalreport (GMT, keyword))
#define GMT_COMPAT_OPT(new_P) if (strchr (list, option)) { GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -%c is deprecated. Use -%c instead.\n" GMT_COMPAT_INFO, option, new_P); option = new_P; }

/* Leave a record that this keyword is no longer a default one
   So far, only gmtset calls this function with core = true, but this is a too fragile solution */
#define GMT_KEYWORD_UPDATE(val) if (core) GMT_keywords_updated[val] = true

EXTERN_MSC void gmt_grdio_init (struct GMT_CTRL *GMT);	/* Defined in gmt_customio.c and only used here */
EXTERN_MSC void gmt_fft_initialization (struct GMT_CTRL *GMT);
EXTERN_MSC void gmt_fft_cleanup (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtapi_garbage_collection (struct GMTAPI_CTRL *API, int level);	/* From gmt_api.c */
EXTERN_MSC void gmt_translin (struct GMT_CTRL *GMT, double forw, double *inv);				/* Forward linear	*/
EXTERN_MSC void gmt_itranslin (struct GMT_CTRL *GMT, double *forw, double inv);				/* Inverse linear	*/

/*--------------------------------------------------------------------*/
/* Load private fixed array parameters from include files */
/*--------------------------------------------------------------------*/

struct GMT5_params {
	const int code;
	const char *name;
};

/* These are the active GMT5 keywords, containing no backwards-compatible variants.
 * Also, some grouped keywords such as FONT and FONT_ANNOT are also not listed since they are not in gmt.conf.
 * If new keywords are added they need to be added here as well as to gmt_keywords.txt. */

static struct GMT5_params GMT5_keywords[]= {
	{ 1, "COLOR Parameters"},
	{ 0, "COLOR_BACKGROUND"},
	{ 0, "COLOR_FOREGROUND"},
	{ 0, "COLOR_NAN"},
	{ 0, "COLOR_MODEL"},
	{ 0, "COLOR_HSV_MIN_S"},
	{ 0, "COLOR_HSV_MAX_S"},
	{ 0, "COLOR_HSV_MIN_V"},
	{ 0, "COLOR_HSV_MAX_V"},
	{ 1, "DIR Parameters"},
	{ 0, "DIR_CACHE"},
	{ 0, "DIR_DATA"},
	{ 0, "DIR_DCW"},
	{ 0, "DIR_GSHHG"},
	{ 1, "FONT Parameters"},
	{ 0, "FONT_ANNOT_PRIMARY"},
	{ 0, "FONT_ANNOT_SECONDARY"},
	{ 0, "FONT_HEADING"},
	{ 0, "FONT_LABEL"},
	{ 0, "FONT_LOGO"},
	{ 0, "FONT_TAG"},
	{ 0, "FONT_TITLE"},
	{ 1, "FORMAT Parameters"},
	{ 0, "FORMAT_CLOCK_IN"},
	{ 0, "FORMAT_CLOCK_OUT"},
	{ 0, "FORMAT_CLOCK_MAP"},
	{ 0, "FORMAT_DATE_IN"},
	{ 0, "FORMAT_DATE_OUT"},
	{ 0, "FORMAT_DATE_MAP"},
	{ 0, "FORMAT_GEO_OUT"},
	{ 0, "FORMAT_GEO_MAP"},
	{ 0, "FORMAT_FLOAT_OUT"},
	{ 0, "FORMAT_FLOAT_MAP"},
	{ 0, "FORMAT_TIME_PRIMARY_MAP"},
	{ 0, "FORMAT_TIME_SECONDARY_MAP"},
	{ 0, "FORMAT_TIME_STAMP"},
	{ 1, "GMT Miscellaneous Parameters"},
	{ 0, "GMT_AUTO_DOWNLOAD"},
	{ 0, "GMT_DATA_URL"},
	{ 0, "GMT_DATA_URL_LIMIT"},
	{ 0, "GMT_COMPATIBILITY"},
	{ 0, "GMT_CUSTOM_LIBS"},
	{ 0, "GMT_EXPORT_TYPE"},
	{ 0, "GMT_EXTRAPOLATE_VAL"},
	{ 0, "GMT_FFT"},
	{ 0, "GMT_HISTORY"},
	{ 0, "GMT_INTERPOLANT"},
	{ 0, "GMT_LANGUAGE"},
	{ 0, "GMT_TRIANGULATE"},
	{ 0, "GMT_VERBOSE"},
	{ 1, "I/O Parameters"},
	{ 0, "IO_COL_SEPARATOR"},
	{ 0, "IO_FIRST_HEADER"},
	{ 0, "IO_GRIDFILE_FORMAT"},
	{ 0, "IO_GRIDFILE_SHORTHAND"},
	{ 0, "IO_HEADER"},
	{ 0, "IO_HEADER_MARKER"},
	{ 0, "IO_N_HEADER_RECS"},
	{ 0, "IO_NAN_RECORDS"},
	{ 0, "IO_NC4_CHUNK_SIZE"},
	{ 0, "IO_NC4_DEFLATION_LEVEL"},
	{ 0, "IO_LONLAT_TOGGLE"},
	{ 0, "IO_SEGMENT_BINARY"},
	{ 0, "IO_SEGMENT_MARKER"},
	{ 1, "MAP Parameters"},
	{ 0, "MAP_ANNOT_MIN_ANGLE"},
	{ 0, "MAP_ANNOT_MIN_SPACING"},
	{ 0, "MAP_ANNOT_OBLIQUE"},
	{ 0, "MAP_ANNOT_OFFSET_PRIMARY"},
	{ 0, "MAP_ANNOT_OFFSET_SECONDARY"},
	{ 0, "MAP_ANNOT_ORTHO"},
	{ 0, "MAP_DEFAULT_PEN"},
	{ 0, "MAP_DEGREE_SYMBOL"},
	{ 0, "MAP_FRAME_AXES"},
	{ 0, "MAP_FRAME_PEN"},
	{ 0, "MAP_FRAME_TYPE"},
	{ 0, "MAP_FRAME_WIDTH"},
	{ 0, "MAP_GRID_CROSS_SIZE_PRIMARY"},
	{ 0, "MAP_GRID_CROSS_SIZE_SECONDARY"},
	{ 0, "MAP_GRID_PEN_PRIMARY"},
	{ 0, "MAP_GRID_PEN_SECONDARY"},
	{ 0, "MAP_HEADING_OFFSET"},
	{ 0, "MAP_LABEL_OFFSET"},
	{ 0, "MAP_LINE_STEP"},
	{ 0, "MAP_LOGO"},
	{ 0, "MAP_LOGO_POS"},
	{ 0, "MAP_ORIGIN_X"},
	{ 0, "MAP_ORIGIN_Y"},
	{ 0, "MAP_POLAR_CAP"},
	{ 0, "MAP_SCALE_HEIGHT"},
	{ 0, "MAP_TICK_LENGTH_PRIMARY"},
	{ 0, "MAP_TICK_LENGTH_SECONDARY"},
	{ 0, "MAP_TICK_PEN_PRIMARY"},
	{ 0, "MAP_TICK_PEN_SECONDARY"},
	{ 0, "MAP_TITLE_OFFSET"},
	{ 0, "MAP_VECTOR_SHAPE"},
	{ 1, "Projection Parameters"},
	{ 0, "PROJ_AUX_LATITUDE"},
	{ 0, "PROJ_DATUM"},
	{ 0, "PROJ_ELLIPSOID"},
	{ 0, "PROJ_GEODESIC"},
	{ 0, "PROJ_LENGTH_UNIT"},
	{ 0, "PROJ_MEAN_RADIUS"},
	{ 0, "PROJ_SCALE_FACTOR"},
	{ 1, "PostScript Parameters"},
	{ 0, "PS_CHAR_ENCODING"},
	{ 0, "PS_COLOR_MODEL"},
	{ 0, "PS_COMMENTS"},
	{ 0, "PS_CONVERT"},
	{ 0, "PS_IMAGE_COMPRESS"},
	{ 0, "PS_LINE_CAP"},
	{ 0, "PS_LINE_JOIN"},
	{ 0, "PS_MITER_LIMIT"},
	{ 0, "PS_MEDIA"},
	{ 0, "PS_PAGE_COLOR"},
	{ 0, "PS_PAGE_ORIENTATION"},
	{ 0, "PS_SCALE_X"},
	{ 0, "PS_SCALE_Y"},
	{ 0, "PS_TRANSPARENCY"},
	{ 1, "Calendar/Time Parameters"},
	{ 0, "TIME_EPOCH"},
	{ 0, "TIME_IS_INTERVAL"},
	{ 0, "TIME_INTERVAL_FRACTION"},
	{ 0, "TIME_LEAP_SECONDS"},
	{ 0, "TIME_REPORT"},
	{ 0, "TIME_UNIT"},
	{ 0, "TIME_WEEK_START"},
	{ 0, "TIME_Y2K_OFFSET_YEAR"},
	{ -1, NULL}
};

#include "gmt_keycases.h"				/* Get all the default case values */
static char *GMT_keywords[GMT_N_KEYS] = {		/* Names of all parameters in gmt.conf */
#include "gmt_keywords.h"
};

bool GMT_keywords_updated[GMT_N_KEYS] = {false};	/* Will be set to 'true' when individual keywords are set via gmtset */

static char *GMT_unique_option[GMT_N_UNIQUE] = {	/* The common GMT command-line options [ just the subset that accepts arguments (e.g., -O is not listed) ] */
#include "gmt_unique.h"
};

static char *GMT_media_name[GMT_N_MEDIA] = {		/* Names of all recognized paper formats */
#include "gmt_media_name.h"
};
static struct GMT_MEDIA GMT_media[GMT_N_MEDIA] = {	/* Sizes in points of all paper formats */
#include "gmt_media_size.h"
};

static char *gmt_M_color_name[GMT_N_COLOR_NAMES] = {	/* Names of all the X11 colors */
#include "gmt_colornames.h"
};

static char *GMT_weekdays[7] = {	/* Days of the week in English [Default] */
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static char *GMT_just_string[12] = {	/* Strings to specify justification */
	"", "BL", "BC", "BR", "", "ML", "MC", "MR", "", "TL", "TC", "TR"
};

/* ISO Font encodings.  Ensure that the order of PSL_ISO_names matches order of includes below */

static char *PSL_ISO_name[] = {
	"PSL_Standard",
	"PSL_Standard+",
	"PSL_ISOLatin1",
	"PSL_ISOLatin1+",
	"PSL_ISO-8859-1",
	"PSL_ISO-8859-2",
	"PSL_ISO-8859-3",
	"PSL_ISO-8859-4",
	"PSL_ISO-8859-5",
	"PSL_ISO-8859-6",
	"PSL_ISO-8859-7",
	"PSL_ISO-8859-8",
	"PSL_ISO-8859-9",
	"PSL_ISO-8859-10",
	"PSL_ISO-8859-13",
	"PSL_ISO-8859-14",
	"PSL_ISO-8859-15",
	NULL
};

static char *PSL_ISO_encoding[] = {
#include "PSL_Standard.h"
#include "PSL_Standard+.h"
#include "PSL_ISOLatin1.h"
#include "PSL_ISOLatin1+.h"
#include "PSL_ISO-8859-1.h"
#include "PSL_ISO-8859-2.h"
#include "PSL_ISO-8859-3.h"
#include "PSL_ISO-8859-4.h"
#include "PSL_ISO-8859-5.h"
#include "PSL_ISO-8859-6.h"
#include "PSL_ISO-8859-7.h"
#include "PSL_ISO-8859-8.h"
#include "PSL_ISO-8859-9.h"
#include "PSL_ISO-8859-10.h"
#include "PSL_ISO-8859-13.h"
#include "PSL_ISO-8859-14.h"
#include "PSL_ISO-8859-15.h"
NULL
};

/* Listing of "Standard" 35 PostScript fonts found on most PS printers
 * plus the 4 Japanese fonts we have supported since GMT 3.
 * The fontheight is the height of A for unit fontsize. */

#define GMT_N_STANDARD_FONTS 39
static struct GMT_FONTSPEC GMT_standard_fonts[GMT_N_STANDARD_FONTS] = {
#include "standard_adobe_fonts.h"
};

/* List of GMT common keyword/options pairs */
#ifdef USE_GMT_KWD
struct GMT_KW_DICT gmt_kw_common[] = {
	{'R', "region"},
	{'J', "proj"},
	{'U', "timestamp"},
	{'V', "verbose"},
	{'X', "xoff"},
	{'Y', "yoff"},
	{'a', "aspatial"},
	{'b', "binary"},
	{'d', "nodata"},
	{'e', "find"},
	{'f', "coltypes"},
	{'g', "gap"},
	{'h', "header"},
	{'i', "incol"},
	{'o', "outcol"},
	{'n', "interpol"},
	{'p', "perspective"},
	{'r', "registration"},
	{'t', "transparency"},
	{'x', "cores"},
	{':', "order"},
	{'\0', ""}	/* End of list marked with empty code and string */
};
#endif

/* Local variables to gmt_init.c */

static struct GMT_HASH keys_hashnode[GMT_N_KEYS];

/* List ps at end since it causes a renaming of ps- to ps only.  Also allow jpeg and tiff spellings */

#include "gmt_gsformats.h"
static char gmt_session_code[] =    { 'f',   'j',    'j',   'g',   'G', 'm',   't',    't',   'b',   'e',  'p',    0};

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

int64_t gmt_parse_range (struct GMT_CTRL *GMT, char *p, int64_t *start, int64_t *stop);

 /* Local functions */

#if defined (WIN32) /* Use Windows API */
#include <Windows.h>
EXTERN_MSC char *dlerror (void);

/*! . */
GMT_LOCAL bool gmtinit_file_lock (struct GMT_CTRL *GMT, int fd) {
	OVERLAPPED over = { 0 };
	HANDLE hand = (HANDLE)_get_osfhandle(fd);
	if (!LockFileEx(hand, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &over)) /* Will block until exclusive lock is acquired */
	{
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Exclusive lock could not be acquired (%s)\n", dlerror());
		return false;
	}
	return true;
}

/*! . */
GMT_LOCAL bool gmtinit_file_unlock (struct GMT_CTRL *GMT, int fd) {
	HANDLE hand = (HANDLE)_get_osfhandle(fd);
	if (!UnlockFile(hand, 0, 0, 0, 1))
	{
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Failed to release lock (%s)\n", dlerror());
		return false;
	}
	return true;
}

#elif defined (HAVE_FCNTL_H_) /* Use POSIX fcntl */
/*! . */
GMT_LOCAL bool gmtinit_file_lock (struct GMT_CTRL *GMT, int fd) {
	int status;
	struct flock lock;
	lock.l_type = F_WRLCK;		/* Lock for exclusive reading/writing */
	lock.l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock.l_start = lock.l_len = 0;

	if ((status = fcntl (fd, F_SETLKW, &lock))) /* Will block until exclusive lock is acquired */
	{
		int errsv = status; /* make copy of status */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Exclusive lock could not be acquired (%s)\n", strerror(errsv));
		return false;
	}
	return true;
}

/*! . */
GMT_LOCAL bool gmtinit_file_unlock (struct GMT_CTRL *GMT, int fd) {
	int status;
	struct flock lock;
	lock.l_type = F_UNLCK;		/* Release lock and close file */
	lock.l_whence = SEEK_SET;	/* These three apply lock to entire file */
	lock.l_start = lock.l_len = 0;

	if ((status = fcntl (fd, F_SETLK, &lock))) /* Release lock */
	{
		int errsv = status; /* make copy of status */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Failed to release lock (%s)\n", strerror(errsv));
		return false;
	}
	return true;
}

#else /* Not Windows and fcntl not available */
/*! . */
GMT_LOCAL bool gmtinit_file_lock (struct GMT_CTRL *GMT, int fd) {
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File locking not supported.\n");
	return false;
}

/*! . */
GMT_LOCAL bool gmtinit_file_unlock (struct GMT_CTRL *GMT, int fd) {
	return false;
}
#endif

#ifdef USE_GMT_KWD

/*! . */
GMT_LOCAL int gmtinit_find_kw (struct GMTAPI_CTRL *API, struct GMT_KW_DICT *kw, char *arg) {
	/* Determine if this arg is found in the keyword list */
	int k;
	gmt_M_unused (API);
	if (kw == NULL) return -1;	/* No list to search in yet */
	for (k = 0; kw[k].name[0]; k++) {
		if (!strncmp (arg, kw[k].name, strlen(kw[k].name))) break;	/* Match found */
	}
	return (kw[k].code) ? k : -1;	/* Either return index to match or -1 if not found */
}

/*! . */
GMT_LOCAL void gmtinit_kw_replace (struct GMTAPI_CTRL *API, struct GMT_KW_DICT *module_kw, struct GMT_OPTION **options) {
	/* Loop over given options and replace any recognized long-form --parameter[=value]
	 * with the corresponding short option version -<code>[value] */
	struct GMT_OPTION *opt = NULL;
	struct GMT_KW_DICT *kw = NULL;
	char text[GMT_LEN256] = {""}, *e = NULL;
	int k;
	
	for (opt = *options; opt; opt = opt->next) {
		if (opt->option != GMT_OPT_PARAMETER) continue;	/* Cannot be a --keyword[=value] pair */
		if (isupper (opt->arg[0])) continue;		/* Skip the upper-case  GMT Default parameter settings */
		e = strchr (opt->arg, '=');			/* Get location of equal sign, if present */
		if (e) e[0] = '\0';				/* Cut off =value for now so opt->arg only has the keyword */
		if ((k = gmtinit_find_kw (API, gmt_kw_common, opt->arg)) >= 0)	/* Got common kw pair */
			kw = gmt_kw_common;
		else if ((k = gmtinit_find_kw (API, module_kw, opt->arg)) >= 0)	/* Got module kw pair */
			kw = module_kw;
		else
			continue;			/* Got nothing */
		/* Do the long to short option substitution */
		opt->option = kw[k].code;		/* Update the option character first */
		if (e) {
			e[0] = '=';			/* Put back the = character */
			sprintf (text, "%s", &e[1]);	/* Get string with the argument */
		}
		else
			text[0] = '\0';			/* No argument to pass on */
		gmt_M_str_free (opt->arg);		/* Free old par=value string argument */
		GMT_Report (API, GMT_MSG_DEBUG, "Converting long-format --%s to -%c%s\n", opt->arg, opt->option, text);
		opt->arg = strdup (text);		/* Allocate copy of new argument */
	}
}
#endif

GMT_LOCAL int get_psl_encoding (const char *encoding) {
	/* Return the specified encoding ID */
	int k = 0, match = 0;
	while (PSL_ISO_name[k] && (match = strcmp (encoding, PSL_ISO_name[k])) != 0) k++;
	return (match == 0) ? k : -1;
}

GMT_LOCAL int gmtinit_get_uservalue (struct GMT_CTRL *GMT, char *txt, int type, double *value, char *err_msg) {
	/* Use to get a single data value of given type and exit if error, and return GMT_PARSE_ERROR */
	int kind;
	if ((kind = gmt_scanf (GMT, txt, type, value)) == GMT_IS_NAN) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error %s: %s\n", err_msg, txt);
		GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
	}
	return 0;
}

/*! . */
GMT_LOCAL int gmtinit_parse_h_option (struct GMT_CTRL *GMT, char *item) {
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
	if (item[0] == 'i')	{/* Apply to input only */
		col = GMT_IN;
		strncpy (GMT->common.g.string, item, GMT_LEN64-1);	/* Verbatim copy */
	}
	else if (item[0] == 'o')	/* Apply to output only */
		col = GMT_OUT;
	else {			/* Apply to both input and output columns */
		k = 0;
		strncpy (GMT->common.g.string, item, GMT_LEN64-1);	/* Verbatim copy */
	}
	if (item[k]) {	/* Specified how many records for input */
		if (col == GMT_OUT) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Can only set the number of input header records; %s ignored\n", &item[k]);
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
		while ((gmt_strtok (c, "+", &pos, p))) {
			switch (p[0]) {
				case 'd':	/* Delete existing headers */
					GMT->common.h.mode = GMT_COMMENT_IS_RESET;
					break;
				case 'c':	/* Add column names record */
					GMT->common.h.add_colnames = true;
					break;
				case 'm':	/* Add a multi-segment record plus some eventual text */
					gmt_M_str_free (GMT->common.h.multi_segment);
					GMT->common.h.multi_segment = strdup (&p[1]);
					break;
				case 'r':	/* Add specific text remark */
					gmt_M_str_free (GMT->common.h.remark);
					GMT->common.h.remark = strdup (&p[1]);
					break;
				case 't':	/* Add specific text title */
					gmt_M_str_free (GMT->common.h.title);
					GMT->common.h.title = strdup (&p[1]);
					break;
				default:	/* Bad modifier */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized modifier +%c.\n", p[0]);
					error++;
					break;
			}
		}

	}
	if ((c = strstr (item, "+t"))) *c = '\0';	/* Truncate the -h...+t<txt> option to avoid duplicate title output in command */
	return (error);
}

/*! . */
GMT_LOCAL void gmtinit_setautopagesize (struct GMT_CTRL *GMT) {
	/* In GMT modern mode we use the largest possible square page (~11x11 meters), portrait mode, and then crop automatically */
	GMT->current.setting.ps_page_size[0] = GMT->current.setting.ps_page_size[1] = GMT_PAPER_DIM;	/* Max area in points */
	GMT->current.setting.ps_media = -USER_MEDIA_OFFSET;
	GMT->current.setting.ps_orientation = PSL_PORTRAIT;
}

/*! . */
GMT_LOCAL int gmtinit_rectR_to_geoR (struct GMT_CTRL *GMT, char unit, double rect[], double out_wesn[], bool get_R) {
	/* If user gives -Re|f|k|M|n<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>][r] then we must
	 * call GMT_mapproject to convert this to geographic degrees.
	 * get_R is true when this is done to obtain the -R setting.  */

	int proj_class;
	uint64_t dim[GMT_DIM_SIZE] = {1, 1, 2, 2};	/* Just a single data table with one segment with two 2-column records */
	bool was_R, was_J;
	double wesn[4];
	char buffer[GMT_LEN256] = {""}, in_string[GMT_STR16] = {""}, out_string[GMT_STR16] = {""};
	struct GMT_DATASET *In = NULL, *Out = NULL;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Call gmtinit_rectR_to_geoR to convert projected -R to geo -R\n");
	if (gmt_M_is_dnan (GMT->current.proj.lon0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Central meridian is not known; cannot convert -R<unit>... to geographic corners\n");
		return (GMT_MAP_NO_PROJECTION);
	}
	if (gmt_M_is_dnan (GMT->current.proj.lat0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Projection standard latitude is not known; cannot convert -R<unit>... to geographic corners\n");
		return (GMT_MAP_NO_PROJECTION);
	}
	/* Create dataset to hold the rect coordinates */
	if ((In = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (GMT_MEMORY_ERROR);

	In->table[0]->segment[0]->data[GMT_X][0] = rect[XLO];
	In->table[0]->segment[0]->data[GMT_Y][0] = rect[YLO];
	In->table[0]->segment[0]->data[GMT_X][1] = rect[XHI];
	In->table[0]->segment[0]->data[GMT_Y][1] = rect[YHI];
	In->table[0]->segment[0]->n_rows = 2;

	/* Set up machinery to call mapproject */

	/* Register In as input virtual file and define an output virtual file */
	if (GMT_Open_VirtualFile (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, In, in_string) == GMT_NOTSET)
		return (GMT->parent->error);
	if (GMT_Open_VirtualFile (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, NULL, out_string) == GMT_NOTSET)
		return (GMT->parent->error);

	was_R = GMT->common.R.active[RSET];	was_J = GMT->common.J.active;
	GMT->common.R.active[RSET] = GMT->common.J.active = false;	/* To allow new entries */

	/* Determine suitable -R setting for this projection */

	/* Default w/e/s/n is small patch centered on projection center - this may change below */
	wesn[XLO] = GMT->current.proj.lon0 - 1.0;		wesn[XHI] = GMT->current.proj.lon0 + 1.0;
	wesn[YLO] = MAX (GMT->current.proj.lat0 -1.0, -90.0);	wesn[YHI] = MIN (GMT->current.proj.lat0 + 1.0, 90.0);

	proj_class = GMT->current.proj.projection_GMT / 100;	/* 1-4 for valid projections */
	if (GMT->current.proj.projection_GMT == GMT_AZ_EQDIST) proj_class = 4;	/* Make -JE use global region */
	switch (proj_class) {
		case 1:	/* Cylindrical: pick small equatorial patch centered on central meridian */
			if (GMT->current.proj.projection_GMT == GMT_UTM && gmt_UTMzone_to_wesn (GMT, GMT->current.proj.utm_zonex, GMT->current.proj.utm_zoney, GMT->current.proj.utm_hemisphere, wesn)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "UTM projection insufficiently specified to auto-determine geographic region\n");
				return (GMT_MAP_NO_PROJECTION);
			}
			else {
				wesn[YLO] = -1.0;	wesn[YHI] = 1.0;
			}
			break;
		case 2: /* Conical: Use default patch */
			break;
		case 3: /* Azimuthal: Use default patch, or hemisphere for polar projections */
			if (doubleAlmostEqualZero (GMT->current.proj.lat0, 90.0)) {
				wesn[YLO] = 0.0;	wesn[YHI] = 90.0;
				wesn[XLO] = GMT->current.proj.lon0 - 180.0;	wesn[XHI] = GMT->current.proj.lon0 + 180.0;
			}
			else if (doubleAlmostEqualZero (GMT->current.proj.lat0, -90.0)) {
				wesn[YLO] = -90.0;	wesn[YHI] = 0.0;
				wesn[XLO] = GMT->current.proj.lon0 - 180.0;	wesn[XHI] = GMT->current.proj.lon0 + 180.0;
			}
			break;
		case 4: /* Global: Give global region */
			wesn[XLO] = 0.0;	wesn[XHI] = 360.0;	wesn[YLO] = -90.0;	wesn[YHI] = 90.0;
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "No map projection specified to auto-determine geographic region\n");
			break;
	}
	snprintf (buffer, GMT_LEN256, "-R%g/%g/%g/%g -J%s -I -F%c -C -bi2d -bo2d -<%s ->%s --GMT_HISTORY=false",
		wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], GMT->common.J.string, unit, in_string, out_string);
	if (get_R) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Obtaining geographic corner coordinates via mapproject %s\n", buffer);
	if (GMT_Call_Module (GMT->parent, "mapproject", GMT_MODULE_CMD, buffer) != GMT_OK)	/* Get the corners in degrees via mapproject */
		return (GMT->parent->error);
	GMT->common.R.active[RSET] = was_R;	GMT->common.J.active = was_J;
	if ((Out = GMT_Read_VirtualFile (GMT->parent, out_string)) == NULL)
		return (GMT->parent->error);
	/* Close the virtual files */
	if (GMT_Close_VirtualFile (GMT->parent, in_string) != GMT_NOERROR)
		return (GMT->parent->error);
	if (GMT_Close_VirtualFile (GMT->parent, out_string) != GMT_NOERROR)
		return (GMT->parent->error);
	out_wesn[XLO] = Out->table[0]->segment[0]->data[GMT_X][0];
	out_wesn[YLO] = Out->table[0]->segment[0]->data[GMT_Y][0];
	out_wesn[XHI] = Out->table[0]->segment[0]->data[GMT_X][1];
	out_wesn[YHI] = Out->table[0]->segment[0]->data[GMT_Y][1];
	/* Free memory used for projection */
	if (GMT_Destroy_Data (GMT->parent, &In) != GMT_OK)
		return (GMT->parent->error);
	if (GMT_Destroy_Data (GMT->parent, &Out) != GMT_OK)
		return (GMT->parent->error);

	if (get_R) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
		"Region selection -R%s is replaced by the equivalent geographic region -R%.12g/%.12g/%.12g/%.12g+r\n",
		GMT->common.R.string, out_wesn[XLO], out_wesn[YLO], out_wesn[XHI], out_wesn[YHI]);


	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_parse_X_option (struct GMT_CTRL *GMT, char *text) {
	/* Syntax: -Xa|r|f|c<off>[unit], -X[-|+]w[/<n>][-|+]<off>[unit], where
	 * w is the width of the previous plot command. */
	int i = 0;
	if (!text || !text[0]) {	/* Default is -Xr0 */
		GMT->current.ps.origin[GMT_X] = GMT->common.X.mode = 'r';
		GMT->current.setting.map_origin[GMT_X] = 0.0;
		return (GMT_NOERROR);
	}
	switch (text[0]) {
		case 'r': case 'a': case 'f': case 'c':
			GMT->current.ps.origin[GMT_X] = GMT->common.X.mode = text[0]; i++; break;
		default:
			GMT->current.ps.origin[GMT_X] = GMT->common.X.mode = 'r'; break;
	}
	if (text[i]) {	/* Gave some argument */
		if (strchr (text, 'w')) {	/* Used previous width as part of offset */
			GMT->current.setting.map_origin[GMT_X] = GMT->current.map.last_width;
			if (text[i] == '-') {	/* Wanted the negative width */
				GMT->current.setting.map_origin[GMT_X] *= -1;
				i++;	/* Skip the minus sign */
			}
			else if (text[i] == '+')	/* In case the user explicitly gave a + */
				i++;	/* Skip the plus sign */
			i++;	/* Skip past the w */
			if (text[i] == '/') {	/* Wanted a fraction of the width */
				i++;	/* Skip the slash */
				GMT->current.setting.map_origin[GMT_X] /= atof (&text[i]);
				while (isdigit (text[i]) || text[i] == '.') i++;	/* Wind past the denominator */
			}
			/* Now add the offset the user added, if given */
			if (text[i]) GMT->current.setting.map_origin[GMT_X] += gmt_M_to_inch (GMT, &text[i]);
		}
		else
			GMT->current.setting.map_origin[GMT_X] = gmt_M_to_inch (GMT, &text[i]);
	}
	else	/* Allow use of -Xc or -Xf meaning -Xc0 or -Xf0 */
		GMT->current.setting.map_origin[GMT_X] = 0.0;
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_parse_Y_option (struct GMT_CTRL *GMT, char *text) {
	/* Syntax: -Ya|r|f|c<off>[unit], -Y[-|+]h[/<n>][-|+]<off>[unit], where
	 * h is the height of the previous plot command. */
	int i = 0;
	if (!text || !text[0]) {	/* Default is -Yr0 */
		GMT->current.ps.origin[GMT_Y] = GMT->common.Y.mode = 'r';
		GMT->current.setting.map_origin[GMT_Y] = 0.0;
		return (GMT_NOERROR);
	}
	switch (text[0]) {
		case 'r': case 'a': case 'f': case 'c':
			GMT->current.ps.origin[GMT_Y] = GMT->common.Y.mode = text[0]; i++; break;
		default:
			GMT->current.ps.origin[GMT_Y] = GMT->common.Y.mode = 'r'; break;
	}
	if (text[i]) {	/* Gave some argument */
		if (strchr (text, 'h')) {	/* Used previous height as part of offset */
			GMT->current.setting.map_origin[GMT_Y] = GMT->current.map.last_height;
			if (text[i] == '-') {	/* Wanted the negative height */
				GMT->current.setting.map_origin[GMT_Y] *= -1;
				i++;	/* Skip the minus sign */
			}
			else if (text[i] == '+')	/* In case the user explicitly gave a + */
				i++;	/* Skip the plus sign */
			i++;	/* Skip past the h */
			if (text[i] == '/') {	/* Wanted a fraction of the height */
				i++;	/* Skip the slash */
				GMT->current.setting.map_origin[GMT_Y] /= atof (&text[i]);
				while (isdigit (text[i]) || text[i] == '.') i++;	/* Wind past the denominator */
			}
			/* Now add the offset the user added, if given */
			if (text[i]) GMT->current.setting.map_origin[GMT_Y] += gmt_M_to_inch (GMT, &text[i]);
		}
		else
			GMT->current.setting.map_origin[GMT_Y] = gmt_M_to_inch (GMT, &text[i]);
	}
	else	/* Allow use of -Yc or -Yf meaning -Yc0 or -Yf0 */
		GMT->current.setting.map_origin[GMT_Y] = 0.0;
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_ogr_get_geometry (char *item) {
	if (!strcmp (item, "point")  || !strcmp (item, "POINT")) return (GMT_IS_POINT);
	if (!strcmp (item, "mpoint") || !strcmp (item, "MPOINT")) return (GMT_IS_MULTIPOINT);
	if (!strcmp (item, "line")   || !strcmp (item, "LINE")) return (GMT_IS_LINESTRING);
	if (!strcmp (item, "mline")  || !strcmp (item, "MLINE")) return (GMT_IS_MULTILINESTRING);
	if (!strcmp (item, "poly")   || !strcmp (item, "POLY")) return (GMT_IS_POLYGON);
	if (!strcmp (item, "mpoly")  || !strcmp (item, "MPOLY")) return (GMT_IS_MULTIPOLYGON);
	return (GMT_NOTSET);
}


/*! . */
GMT_LOCAL int gmtinit_parse_a_option (struct GMT_CTRL *GMT, char *arg) {
	/* -a<col>=<name>[:<type>][,<col>...][+g|G<geometry>] */
	unsigned int pos = 0;
	int col, a_col = GMT_Z, t;
	char p[GMT_BUFSIZ] = {""}, name[GMT_BUFSIZ] = {""}, A[GMT_LEN64] = {""}, *s = NULL, *c = NULL;
	if (!arg) return (GMT_PARSE_ERROR);	/* -a requires a non-NULL argument */
	//if (!arg[0]) return (GMT_PARSE_ERROR);	/* -a requires an argument */
	strncpy (GMT->common.a.string, arg, GMT_LEN256-1);	/* Verbatim copy */

	if ((s = strstr (arg, "+g")) || (s = strstr (arg, "+G"))) {	/* Also got +g|G<geometry> */
		if ((t = (int)gmtinit_ogr_get_geometry (s+2)) < 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -a: No such geometry: %s.\n", s+2);
			return (GMT_PARSE_ERROR);
		}
		GMT->common.a.geometry = t;
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
	while ((gmt_strtok (arg, ",", &pos, p))) {	/* Another col=name argument */
		if ((c = strchr (p, ':'))) {	/* Also got :<type> */
			if ((t = gmtlib_ogr_get_type (c+1)) < 0) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -a: No such type: %s.\n", c+1);
				return (GMT_PARSE_ERROR);
			}
			GMT->common.a.type[GMT->common.a.n_aspatial] = t;
			c[0] = '\0';	/* Truncate off the type */
		}
		else
			GMT->common.a.type[GMT->common.a.n_aspatial] = GMT_DOUBLE;
		if (strchr (p, '=')) {	/* Got col=name */
			if (sscanf (p, "%[^=]=%s", A, name) != 2) return (GMT_PARSE_ERROR);	/* Did not get two items */
		}
		else {	/* Auto-fill col as the next col starting at GMT_Z */
			snprintf (A, GMT_LEN64, "%d", a_col++);
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
		gmt_M_str_free (GMT->common.a.name[GMT->common.a.n_aspatial]);	/* Free any previous names */
		GMT->common.a.name[GMT->common.a.n_aspatial] = strdup (name);
		GMT->common.a.n_aspatial++;
		if (GMT->common.a.n_aspatial == MAX_ASPATIAL) return (GMT_PARSE_ERROR);	/* Too many items */
	}
	if (s) s[0] = '+';	/* Restore the geometry part */
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_parse_b_option (struct GMT_CTRL *GMT, char *text) {
	/* GMT5 Syntax:	-b[i][cvar1/var2/...] or -b[i|o]<n><type>[,<n><type>]...
	 * GMT4 Syntax:	-b[i][o][s|S][d|D][<n>][c[<var1>/<var2>/...]]
	 * -b with no args means both in and out have double precision binary i/o, with #columns determined by module
	 * -bi or -bo means the same for that direction only.
	 * -bif or -bof or any other letter means that type instead of double.
	 * Note to developers: It is allowed NOT to specify anything (e.g., -bi) or not specify how many columns (e.g., -bif).
	 * If those are not set then there are two opportunities in the modules to correct this:
	 *   1) gmtlib_io_banner is called from GMT_Init_IO and if things are not set we default to the default data type [double].
	 *   2) gmt_set_cols sets the actual columns needed for in or out and is also use to set un-initialized data read pointers.
	 */

	unsigned int i, col = 0, id = GMT_IN, swap_flag;
	int k, ncol = 0;
	bool endian_swab = false, swab = false, error = false, i_or_o = false, set = false, v4_parse = false;
	char *p = NULL, c;

	if (!text) return (GMT_PARSE_ERROR);	/* -B requires an argument even if it is blank */
	/* First determine if there is an endian modifier supplied */
	if ((p = strchr (text, '+'))) {	/* Yes */
		*p = '\0';	/* Temporarily chop off the modifier */
		switch (p[1]) {
			case 'B':	case 'b':
			case 'L':	case 'l':
				swab = (toupper (p[1]) != GMT_ENDIAN);
				break;	/* Must swap */
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -b: Bad endian modifier +%c\n", (int)p[1]);
				return (GMT_PARSE_ERROR);
				break;
		}
		if (strchr (text, 'w')) {	/* Cannot do individual swap when endian has been indicated */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -b: Cannot use both w and endian modifiers\n");
			return (GMT_PARSE_ERROR);
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
	if (gmt_M_compat_check (GMT, 4)) {	/* GMT4 */
		if (text[k] == 'c' && !text[k+1]) {	/* Ambiguous case "-bic" which MAY mean netCDF */
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Syntax warning: -b[i]c now applies to character tables, not to netCDF\n");
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Syntax warning: If input file is netCDF, just leave out -b[i]c\n");
			GMT->common.b.type[id] = 'c';
			v4_parse = true;	/* Yes, we parsed a GMT4-compatible option */
		}
		else if (text[k] == 'c' && text[k+1] != ',') {	/* netCDF with list of variables */
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Syntax warning: -b[i]c<varlist> is deprecated. Use <file>?<varlist> instead.\n");
			GMT->common.b.nc[id] = true;
			GMT->common.b.active[id] = false;	/* Binary is 'false' if netCDF is to be read */
			strncpy (GMT->common.b.varnames, &text[k+1], GMT_BUFSIZ-1);	/* Copy the list of netCDF variable names */
			v4_parse = true;	/* Yes, we parsed a GMT4-compatible option */
		}
	}
	if (!v4_parse && text[k] && strchr ("cuhHiIfd" GMT_OPT ("sSD"), text[k]) && (!text[k+1] || (text[k+1] == 'w' && !text[k+2] ))) {	/* Just save the type for the entire record */
		GMT->common.b.type[id] = text[k];			/* Set the default column type to the first (and possibly only data type) listed */
		if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: Must switch s,S,D to f, f(with swab), and d (with swab) */
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
					if (gmt_M_compat_check (GMT, 4)) {
						GMT_Report (GMT->parent, GMT_MSG_COMPAT,
							"The -b option with type s, S, or D are deprecated; Use <n>f or <n>d, with w to indicate swab\n");
						if (c == 'S' || c == 'D') swab = true;
						if (c == 'S' || c == 's') c = 'f';
						else if (c == 'D') c = 'd';
						if (ncol == 0) ncol = 1;	/* In order to make -bs work as before */
					}
					else {
						error = true;
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Malformed -b argument [%s]\n", text);
						gmt_syntax (GMT, 'b');
						break;
					} /* Deliberate fall-through to these cases here */
				case 'c': case 'u': /* int8_t, uint8_t */
				case 'h': case 'H': /* int16_t, uint16_t */
				case 'i': case 'I': /* int32_t, uint32_t */
				case 'l': case 'L': /* int64_t, uint64_t */
				case 'f': case 'd': /* float, double */
					if (gmt_M_compat_check (GMT, 4) && c == 'd' && ncol == 0) {
						ncol = 1;	/* In order to make -bd work as before */
						GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-b[o]d is deprecated; Use <n>d to indicate how many columns\n");
					}
					if (text[i+1] == 'w')	/* Want to swab the input or output first */
						swab = true;
					set = true;	/* Meaning we have set the data type */
					if (ncol == 0) {	/* Just specifying type, no columns yet */
						GMT->common.b.type[id] = c;	/* Set default to double */
					}
					swap_flag = (swab) ? id + 1 : 0;	/* 0 for no swap, 1 if swap input, 2 if swap output */
					for (k = 0; k < ncol; k++, col++) {	/* Assign io function pointer and data type for each column */
						GMT->current.io.fmt[id][col].io = gmtlib_get_io_ptr (GMT, id, swap_flag, c);
						GMT->current.io.fmt[id][col].type = gmt_get_io_type (GMT, c);
						if (!i_or_o) {	/* Must also set output */
							GMT->current.io.fmt[GMT_OUT][col].io = gmtlib_get_io_ptr (GMT, GMT_OUT, swap_flag, c);
							GMT->current.io.fmt[GMT_OUT][col].type = gmt_get_io_type (GMT, c);
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
						return (GMT_PARSE_ERROR);
					}
					while (text[++i] && isdigit ((int)text[i])); /* Wind past the digits */
					--i;	/* Then go back to last digit since for loop will do i++ after this */
					break;
				case ',':
					break;	/* Comma between sequences are optional and just ignored */
				case 'w':		/* Turn off the swap on a per-item basis unless it was set via +L|B */
					if (!endian_swab)
						swab = false;
					break;
				default:
					error = true;
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Malformed -b argument [%s]\n", text);
					gmt_syntax (GMT, 'b');
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
			GMT->current.io.fmt[id][col].io   = gmtlib_get_io_ptr (GMT, id, swab, 'd');
			GMT->current.io.fmt[id][col].type = gmt_get_io_type (GMT, 'd');
			if (!i_or_o) {	/* Must also set output */
				GMT->current.io.fmt[GMT_OUT][col].io   = gmtlib_get_io_ptr (GMT, GMT_OUT, swab, 'd');
				GMT->current.io.fmt[GMT_OUT][col].type = gmt_get_io_type (GMT, 'd');
			}
		}
	}

	if (!i_or_o) {	/* Specified neither i or o so let settings apply to both */
		GMT->common.b.nc[GMT_OUT] = GMT->common.b.nc[GMT_IN];
		GMT->common.b.active[GMT_OUT] = GMT->common.b.active[GMT_IN];
		GMT->common.b.ncol[GMT_OUT] = GMT->common.b.ncol[GMT_IN];
		GMT->common.b.type[GMT_OUT] = GMT->common.b.type[GMT_IN];
		if (GMT->common.b.swab[GMT_IN] == k_swap_in) GMT->common.b.swab[GMT_OUT] = k_swap_out;
		strncpy (GMT->common.b.string, text, GMT_LEN256-1);
	}
	else if (id == GMT_IN)
		strncpy (GMT->common.b.string, text, GMT_LEN256-1);


	gmtlib_set_bin_io (GMT);	/* Make sure we point to binary i/o functions after processing -b option */

	if (p) *p = '+';	/* Restore the + sign we gobbled up earlier */
	return (error);
}

/*! Routine will decode the -f[i|o]<col>|<colrange>[t|T|g|c],... arguments */
GMT_LOCAL int gmtinit_parse_f_option (struct GMT_CTRL *GMT, char *arg) {

	char copy[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""};
	unsigned int dir, k = 1, ic, pos = 0, code, *col = NULL;
	size_t len;
	int64_t i, start = -1, stop = -1, inc;
	enum gmt_enum_units unit = GMT_IS_METER;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -f requires an argument */

	if (arg[0] == 'i') {	/* Apply to input columns only */
		dir = GMT_IN;
		col = GMT->current.io.col_type[GMT_IN];
		strncpy (GMT->common.f.string, arg, GMT_LEN64-1);	/* Verbatim copy */
	}
	else if (arg[0] == 'o') {	/* Apply to output columns only */
		dir = GMT_OUT;
		col = GMT->current.io.col_type[GMT_OUT];
	}
	else {			/* Apply to both input and output columns */
		dir = GMT_IO;
		k = 0;
		strncpy (GMT->common.f.string, arg, GMT_LEN64-1);	/* Verbatim copy */
	}

	strncpy (copy, &arg[k], GMT_BUFSIZ-1);	/* arg should NOT have a leading i|o part */

	if (copy[0] == 'c') {	/* Got -f[i|o]c which is shorthand for -f[i|o]0f,1f (Cartesian) */
		if (dir == GMT_IO) {
			gmt_set_cartesian (GMT, GMT_IN);
			gmt_set_cartesian (GMT, GMT_OUT);
		}
		else {
			col[GMT_X] = GMT_IS_FLOAT;
			col[GMT_Y] = GMT_IS_FLOAT;
		}
		pos = 1;
		start = stop = 1;
	}
	else if (copy[0] == 'g' || copy[0] == 'p') {	/* Got -f[i|o]g which is shorthand for -f[i|o]0x,1y, or -fp[<unit>] (see below) */
		if (dir == GMT_IO) {
			gmt_set_geographic (GMT, GMT_IN);
			gmt_set_geographic (GMT, GMT_OUT);
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
			if ((unit = gmtlib_get_unit_number (GMT, copy[1])) == GMT_IS_NOUNIT) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Malformed -f argument [%s] - bad projected unit\n", arg);
				return 1;
			}
			pos++;
		}
		GMT->current.proj.inv_coordinates = true;
		GMT->current.proj.inv_coord_unit = unit;
	}

	while ((gmt_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process it */
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
			case 'd':	/* Length dimension (with possible unit) */
				code = GMT_IS_DIMENSION;
				break;
			default:	/* No suffix, consider it an error */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Malformed -f argument [%s]\n", arg);
				return 1;
				break;
		}

		/* Now set the code for these columns */

		for (i = start; i <= stop; i += inc)
			gmt_set_column (GMT, dir, (unsigned int)i, code);
	}
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_compare_cols (const void *point_1, const void *point_2) {
	/* Sorts cols into ascending order  */
	if (((struct GMT_COL_INFO *)point_1)->col < ((struct GMT_COL_INFO *)point_2)->col) return (-1);
	if (((struct GMT_COL_INFO *)point_1)->col > ((struct GMT_COL_INFO *)point_2)->col) return (+1);
	return (0);
}

/*! parse any --PARAM[=value] arguments */
GMT_LOCAL int gmtinit_parse_dash_option (struct GMT_CTRL *GMT, char *text) {
	int n;
	char *this_c = NULL, message[GMT_LEN128] = {""};
	if (!text)
		return (GMT_NOERROR);

	/* print version and exit */
	if (strcmp (text, "version") == 0) {
		snprintf (message, GMT_LEN128, "%s\n", GMT_PACKAGE_VERSION_WITH_GIT_REVISION);
		GMT->parent->print_func (stdout, message);
		/* cannot call gmt_M_free_options() from here, so we are leaking on exit.
		 * struct GMTAPI_CTRL *G = GMT->parent;
		 * if (GMT_Destroy_Session (G))
		 *   GMT_exit (GMT, GMT_PARSE_ERROR); */
		GMT_exit (GMT, GMT_NOERROR);
	}

	/* print GMT folders and exit */
	if (strcmp (text, "show-datadir") == 0) {
		snprintf (message, GMT_LEN128, "%s\n", GMT->session.SHAREDIR);
		GMT->parent->print_func (stdout, message);
		/* leaking on exit same as above. */
		GMT_exit (GMT, GMT_NOERROR);
	}

	if ((this_c = strchr (text, '='))) {
		/* Got --PAR=VALUE */
		this_c[0] = '\0';	/* Temporarily remove the '=' character */
		n = gmtlib_setparameter (GMT, text, &this_c[1], false);
		this_c[0] = '=';	/* Put it back were it was */
	}
	else {
		/* Got --PAR */
		n = gmtlib_setparameter (GMT, text, "true", false);
	}
	if (!strncmp (text, "PS_CHAR_ENCODING", 16U)) GMT->current.ps.switch_set = true;
	return (n);
}

GMT_LOCAL int gmtinit_count_x_terms (char *txt, int64_t *xstart, int64_t *xstop) {
	/* Process things like xxxx, x4, etc and find the number of x items.
	 * We return the start=stop as the number of x's */

	unsigned int n = 0;
	size_t len = strlen (txt), k = 0;
	while (k < len) {
		if (isdigit (txt[k+1])) {	/* Gave things like x3 */
			n += atoi (&txt[k+1]);
			k++;
			while (txt[k] && isdigit (txt[k])) k++;	/* Wind pass the number */
		}
		else {	/* Just one x */
			n++;
			k++;
		}
	}
	*xstart = *xstop = n;	/* Just a single x combo */
	return 0;
}

GMT_LOCAL int gmtinit_trend_modifiers (struct GMT_CTRL *GMT, char option, char *c, unsigned int dim, struct GMT_MODEL *M) {
	char p[GMT_BUFSIZ] = {""};
	unsigned int pos = 0;
	int k, sdim = dim;

	if (dim >= 2) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "INTERNAL Error gmtinit_trend_modifiers: was passed dim >= 2 (%u)\n", dim);
		return -1;
	}

	/* Gave one or more modifiers */

	while ((gmt_strtok (c, "+", &pos, p))) {
		switch (p[0]) {
			case 'o':	/* Origin of axes */
				if ((k = GMT_Get_Values (GMT->parent, &p[1], M->origin, 2)) < 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Unable to parse the +o arguments (%s)\n", option, &p[1]);
					return -1;
				}
				else if (k != sdim) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Did not provide %u arguments to +o\n", option, dim);
					return -1;
				}
				for (k = 0; k < sdim; k++) M->got_origin[k] = true;
				break;
			case 'r':
				M->robust = true;
				break;
			case 'l':
				if ((k = GMT_Get_Values (GMT->parent, &p[1], M->period, 2)) < 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Unable to parse the +l argument (%s)\n", option, &p[1]);
					return -1;
				}
				else if (k != sdim) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Did not provide %u arguments to +l\n", option, dim);
					return -1;
				}
				for (k = 0; k < sdim; k++) M->got_period[k] = true;
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Unrecognized modifier +%s\n", option, p);
				return -1;
				break;
		}
	}
	return 0;
}

GMT_LOCAL char *gmtinit_old_trendsyntax (struct GMT_CTRL *GMT, char option, char *in_arg) {
	/* Deal with backwards compatibilities: -N[f]<nmodel>[r], where nmodel meant # of terms in polynomial */
	char *arg = strdup (in_arg);
	size_t end = strlen (arg) - 1;
	int order;
	if ((isdigit (arg[0]) || (end > 1 && arg[0] == 'f' && isdigit (arg[2]))) && ((arg[end] == 'r' && arg[end-1] != '+') || isdigit (arg[end])))
	{
		/* Old GMT4-like syntax. If compatibility allows it we rewrite using new syntax so we only have one parser below */
		if (gmt_M_compat_check (GMT, 5)) {	/* Allow old-style syntax */
			char new[GMT_BUFSIZ] = {""}, term[GMT_LEN16] = {""};
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-%c%s is deprecated; see usage for new syntax\n", option, arg);
			if (arg[0] != 'f') new[0] = 'p';	/* So we start with f or p */
			if (arg[end] == 'r') {
				arg[end] = '\0';	/* Chop off the r */
				order = atoi (arg);
				if (new[0] == 'p') order--;
				snprintf (term, GMT_LEN16, "%d", order);
				strcat (new, term);
				strcat (new, "+r");	/* Add robust flag */
			}
			else {
				order = atoi (arg);
				if (new[0] == 'p') order--;
				snprintf (term, GMT_LEN16, "%d", order);
				strcat (new, term);
			}
			gmt_M_str_free (arg);	/* Wipe old setting */
			arg = strdup (new);	/* Place revised args */
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Old-style arguments given and chosen compatibility mode does not allow it\n", option);
			gmt_M_str_free (arg);
			return NULL;
		}
	}
	return arg;
}

GMT_LOCAL int compare_terms (const void *term_1v, const void *term_2v) {
	const struct GMT_MODEL_TERM *term_1 = term_1v, *term_2 = term_2v;
	/* Ensure polynomial terms are listed before Fourier terms */
	if (term_1->kind  < term_2->kind)  return (-1);
	if (term_1->kind  > term_2->kind)  return (+1);
	if (term_1->order < term_2->order) return (-1);
	if (term_1->order > term_2->order) return (+1);
	return (0);
}

/*! . */
GMT_LOCAL int gmtinit_parse_model1d (struct GMT_CTRL *GMT, char option, char *in_arg, struct GMT_MODEL *M) {
	/* Parsing for -N arguments in trend1d
	 * Parse -N[p|P|f|F|c|C|s|S|x|X]<list-of-terms>[,...][+l<length>][+o<origin>][+r].
	 * p means polynomial model.
	 * c means cosine series.
	 * s means sine series.
	 * f means both cosine and sine (Fourier series).
	 * <list-of-terms> is either a single order (e.g., 2) or a range (e.g., 0-3)
	 * Give one or more lists separated by commas.
	 * We add the basis x^p, cos(2*pi*p/X), and/or sin(2*pi*p/X) depending on selection.
 	 * Indicate robust fit by appending +r.
	 * Set data origin via +o<orig> and fundamental period via +l<length>
	 */

	unsigned int pos = 0, n_model = 0, k, j, n_P = 0, n_p = 0;
	int64_t order, xstart, xstop, step;
	bool got_pol = false, single = false;
	enum GMT_enum_model kind;
	char p[GMT_BUFSIZ] = {""}, *this_range = NULL, *arg = NULL, *name = "pcs", *c = NULL;

	if (!in_arg || !in_arg[0]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: No arguments given!\n", option);
		return -1;	/* No arg given */
	}
	/* Deal with backwards compatibilities fro GMT4: -N[f]<nmodel>[r] */
	arg = gmtinit_old_trendsyntax (GMT, option, in_arg);	/* Returns a possibly recreated option string */
	if ((c = strchr (arg, '+'))) {	/* Gave one or more modifiers */
		if (gmtinit_trend_modifiers (GMT, option, c, 1U, M)) {
			gmt_M_str_free (arg);
			return -1;
		}
		c[0] = '\0';	/* Chop off modifiers in arg before processing the model settings */
	}
	while ((gmt_strtok (arg, ",", &pos, p))) {	/* For each item in the series... */
		/* Here, p will hold one instance of [P|p|F|f|C|c|S|s|x]<list-of-terms> */
		if (!strchr ("CFSPcfspx", p[0])) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad basis function type (%c)\n", option, p[0]);
			gmt_M_str_free (arg);
			return -1;
		}
		this_range = &p[1];
		single = false;
		switch (p[0]) {	/* What kind of basis function? */
			case 'p': case 'P': case 'x': kind = GMT_POLYNOMIAL; break;
			case 'c': case 'C': kind = GMT_COSINE; break;
			case 's': case 'S': kind = GMT_SINE; break;
			case 'f': case 'F': kind = GMT_FOURIER; break;
			default: kind = GMT_POLYNOMIAL; break;	/* To please gcc */

		}
		if (p[0] == 'x')	{	/* Single building block and not all items of given order */
			single = true;
			gmtinit_count_x_terms (p, &xstart, &xstop);
		}
		else if ((step = gmt_parse_range (GMT, this_range, &xstart, &xstop)) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad basis function order (%s)\n", option, this_range);
			gmt_M_str_free (arg);
			return -1;
		}
		if (!single && islower (p[0])) xstart = 0;	/* E.g., p3 should become p0-3 */
		if (p[0] == 'p') n_p++;
		if (p[0] == 'P') n_P++;
		/* Here we have range xstart to xstop and component kind */

		for (order = xstart; order <= xstop; order++) {
			if (!got_pol && order == 0) {	/* When order == 0 for trig function we substitute polynomial of order 0 instead */
				M->term[n_model].kind = GMT_POLYNOMIAL;
				M->term[n_model].order[GMT_X] = (unsigned int)order;
				got_pol = M->intercept = true;
				n_model++;
				M->type |= 1;	/* Polynomial */
				continue;
			}
			/* For each order given in the range, or just this particular order */
			switch (kind) {
				case GMT_POLYNOMIAL:	/* Add one or more polynomial basis */
				case GMT_CHEBYSHEV:		/* Just to keep compiler happy */
					M->term[n_model].kind = GMT_POLYNOMIAL;
					M->term[n_model].order[GMT_X] = (unsigned int)order;
					M->type |= 1;	/* Polynomial */
					got_pol = true;
					if (M->term[n_model].order[GMT_X] == 0) M->intercept = true;
					n_model++;
					break;
				case GMT_FOURIER:	/* Add a Fourier basis (2 parts) */
					if (order == 0) continue;	/* Only to constant via the polynomial parts */
					for (j = 0; j < 2; j++) {	/* Loop over cosine and sine in x */
						M->term[n_model].kind = GMT_COSINE+j;
						M->term[n_model].order[GMT_X] = (unsigned int)order;
						n_model++;
					}
					M->type |= 2;	/* Fourier */
					break;
				case GMT_COSINE:	/* Add a Cosine basis (1 or 2 parts) */
					if (order == 0) continue;	/* Only to constant via the polynomial parts */
					M->term[n_model].kind = GMT_COSINE;
					M->term[n_model].order[GMT_X] = (unsigned int)order;
					M->type |= 2;	/* Fourier */
					n_model++;
					break;
				case GMT_SINE:	/* Add a Sine basis (1 or 2 parts) */
					if (order == 0) continue;	/* Only to constant via the polynomial parts */
					M->term[n_model].kind = GMT_SINE;
					M->term[n_model].order[GMT_X] = (unsigned int)order;
					M->type |= 2;	/* Fourier */
					n_model++;
					break;
			}
			if (n_model == GMT_N_MAX_MODEL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
				gmt_M_str_free (arg);
				return -1;
			}
		}
	}
	gmt_M_str_free (arg);

	/* Sort so Trig/Fourier terms are last in the list */

	qsort (M->term, n_model, sizeof (struct GMT_MODEL_TERM), compare_terms);

	/* Make sure there are no duplicates */

	for (k = 0; k < n_model; k++) {
		if (M->term[k].kind <= GMT_CHEBYSHEV) M->n_poly++;	/* Set how many poly terms there are in model */
		for (j = k+1; j < n_model; j++) {
			if (M->term[k].kind == M->term[j].kind && M->term[k].order[GMT_X] == M->term[j].order[GMT_X] && M->term[k].order[GMT_Y] == M->term[j].order[GMT_Y] && M->term[k].type == M->term[j].type) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Basis %c%u occurs more than once!\n", option, name[M->term[k].kind], M->term[k].order[GMT_X]);
				return -1;
			}
		}
	}
	if (!single && n_P == 0 && n_p == 1) M->chebyshev = true;	/* Can fit via Chebyshev polynomials */
	M->n_terms = n_model;

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {	/* Report our trend model y(x) if -V */
		char *way[2] = {"last squares", "robust"}, report[GMT_BUFSIZ] = {""}, piece[GMT_LEN64] = {""};
		if (!M->intercept) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning -%c: No intercept term (p0) given\n", option);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Solving for %u terms using a %s norm.  The model:\n", n_model, way[M->robust]);
		sprintf (report, "y(x) = ");
		for (k =  0; k < n_model; k++) {
			switch (M->term[k].kind) {
				case GMT_POLYNOMIAL:
				case GMT_CHEBYSHEV:
					if (M->term[k].order[GMT_X] == 0)
						sprintf (piece, "%c", 'a');
					else
						sprintf (piece, "%c * x^%d", 'a'+ k, M->term[k].order[GMT_X]);
					break;
				case GMT_COSINE:
					sprintf (piece, "%c * cos (%d*x)", 'a'+k, M->term[k].order[GMT_X]);
					break;
				case GMT_SINE:
					sprintf (piece, "%c * sin (%d*x)", 'a'+k, M->term[k].order[GMT_X]);
					break;
			}
			strcat (report, piece);
			((n_model - k) > 1) ? strcat (report, " + ") : strcat (report, "\n");
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, report);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "In the above, x = 2*(x - x_mid)/(x_max - x_min) for polynomials and x = 2*pi*(x - origin)/length for Fourier components\n");
		if (M->chebyshev) {
			if (M->type & 1) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "The complete polynomial will be represented via Chebyshev polynomials\n");
		}
		else {
			if (M->type & 1) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "The partial polynomial will be represented via powers of x\n");
		}
	}
	/* If we can use Chebyshev polynomials then we switch the kind to indicate this */
	if (M->chebyshev) for (k =  0; k < n_model; k++) if (M->term[k].kind == GMT_POLYNOMIAL) M->term[k].kind = GMT_CHEBYSHEV;
	return (0);
}

#if 0
GMT_LOCAL int gmtinit_count_xy_terms (char *txt, int64_t *xstart, int64_t *xstop, int64_t *ystart, int64_t *ystop) {
	/* Process things like xxxxyy, x4y2, etc and find the number of x and y items.
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

/*! . */
GMT_LOCAL int gmtinit_parse_model2d (struct GMT_CTRL *GMT, char option, char *in_arg, struct GMT_MODEL *M) {
	/* Parse -N[p|P|f|F|c|C|s|S|x|X|y|Y][x|y]<list-of-terms>[,...][+l<lengths>][+o<origins>][+r] for trend2d, grdtrend.
	 * p means polynomial.
	 * c means cosine.  You may optionally add x|y to only add basis for that dimension [both]
	 * s means sine.  Optionally append x|y [both]
	 * f means both cosine and sine (Fourier series).  Optionally append x|y [both]
	 * list-of-terms is either a single order (e.g., 2) or a range (e.g., 0-3)
	 * Give one or more lists separated by commas.
	 * In 2-D, for polynomial the order means all p products of x^m*y^n where m+n == p
	 *   To only have some of these terms you must instead specify which ones you want,
	 *   e.g., xxxy (or x3y) and yyyx (or y3x) for just those two.
	 *   For Fourier we add these 4 terms per order:
	 *   cos(2*pi*p*x/X), sin(2*pi*p*x/X), cos(2*pi*p*y/Y), sin(2*pi*p*y/Y)
	 *   To only add basis in x or y you must apped x|y after the c|s|f.
 	 * Indicate robust fit by appending +r
	 */

	unsigned int pos = 0, n_model = 0, n_parts, k, j;
	int64_t order, xstart, xstop, ystart, ystop, step;
	bool got_intercept, single;
	enum GMT_enum_model kind;
	char p[GMT_BUFSIZ] = {""}, *this_range = NULL, *arg = NULL, *name = "pcs", *c = NULL;

	if (!in_arg || !in_arg[0]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: No arguments given!\n", option);
		return -1;	/* No arg given */
	}
	/* Deal with backwards compatibilities: -N<nmodel>[r] for 2-D */
	arg = gmtinit_old_trendsyntax (GMT, option, in_arg);
	if ((c = strchr (arg, '+'))) {	/* Gave one or more modifiers */
		if (gmtinit_trend_modifiers (GMT, option, c, 2U, M)) return -1;
		c[0] = '\0';	/* Chop off modifiers in arg before processing settings */
	}
	while ((gmt_strtok (arg, ",", &pos, p))) {
		/* Here, p will be one instance of [P|p|F|f|C|c|S|s][x|y]<list-of-terms> */
		if (!strchr ("CFSPcfsp", p[0])) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad basis function type (%c)\n", option, p[0]);
			return -1;
		}
		this_range = &p[1];
		n_parts = 1;	/* Normally just one basis function at the time but f implies both c and s */
		single = false;
		switch (p[0]) {	/* What kind of basis function? */
			case 'p': case 'P': kind = GMT_POLYNOMIAL; break;
			case 'c': case 'C': kind = GMT_COSINE; break;
			case 's': case 'S': kind = GMT_SINE; break;
			case 'f': case 'F': kind = GMT_FOURIER; break;

		}
		if (p[1] == 'x' || p[1] == 'y')	{	/* Single building block and not all items of given order */
			single = true;
			gmtinit_count_xy_terms (&p[1], &xstart, &xstop, &ystart, &ystop);
		}
		else if ((step = gmt_parse_range (GMT, this_range, &xstart, &xstop)) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad basis function order (%s)\n", option, this_range);
			return -1;
		}
		if (islower (p[0])) xstart = ystart = 0;
		if (kind != GMT_POLYNOMIAL && !got_intercept) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Cosine|Sine cannot start with order 0.  Use p0 to add a constant\n", option);
			return -1;
		}
		/* Here we have range and kind */

		/* For the Fourier components we need to distinguish between things like cos(x)*sin(y), sin(x)*cos(y), cos(x), etc.  We use these 8 type flags:
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
					if (!single) {
						ystart = 0;
						ystop = order;
					}
					for (k = ystart; k <= ystop; k++) {
						M->term[n_model].kind = GMT_POLYNOMIAL;
						M->term[n_model].order[GMT_X] = (unsigned int)(order - k);
						M->term[n_model].order[GMT_Y] = (unsigned int)k;
						if (M->term[k].order[GMT_X] == 0) got_intercept = true;
						n_model++;
						if (n_model == GMT_N_MAX_MODEL) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
							return -1;
						}
					}
					break;
				case GMT_FOURIER:	/* Add a Fourier basis (2 or 4 parts) */
					if (order == 0) continue;	/* Only to constant via the polynomial parts */
					if (!single) {
						ystart = ystop = order;
					}
					for (j = 0; j < 2; j++) {	/* Loop over cosine and sine in x */
						for (k = 0; k < 2; k++) {	/* Loop over cosine and sine in y, if 2-D */
							M->term[n_model].kind = GMT_FOURIER;
							M->term[n_model].type = 4 + 2*j + k;	/* CC, CS, SC, SS */
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
					if (order == 0) continue;	/* Only to constant via the polynomial parts */
					if (!single) {
						ystart = ystop = order;
					}
					/* cosine in x? */
					M->term[n_model].kind = GMT_COSINE;
					M->term[n_model].type = 0;	/* C- */
					M->term[n_model].order[GMT_X] = (unsigned int)order;
					n_model++;
					if (n_model == GMT_N_MAX_MODEL) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Exceeding max basis functions (%d) \n", option, GMT_N_MAX_MODEL);
						return -1;
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
					if (!single) {
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
	gmt_M_str_free (arg);
	/* Make sure there are no duplicates */

	for (k = 0; k < n_model; k++) {
		for (j = k+1; j < n_model; j++) {
			if (M->term[k].kind == M->term[j].kind && M->term[k].order[GMT_X] == M->term[j].order[GMT_X] && M->term[k].order[GMT_Y] == M->term[j].order[GMT_Y] && M->term[k].type == M->term[j].type) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Basis %cx%uy%u occurs more than once!\n", option, name[M->term[k].kind], M->term[k].order[GMT_X], M->term[k].order[GMT_Y]);
				return -1;
			}
		}
	}
	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {	/* Report our findings */
		char *way[2] = {"last squares", "robust"};
		if (!got_intercept) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning -%c: No intercept term (p0) given\n", option);
		fprintf (stderr, "Fitting %u terms using a %s norm\n", n_model, way[M->robust]);
		for (k = j = 0; k < n_model; k++, j++) {
			fprintf (stderr, "Model basis %d is of type %c and order %u/%u\n", k, name[M->term[k].kind], M->term[k].order[GMT_X], M->term[k].order[GMT_Y]);
		}
	}
	M->n_terms = n_model;
	return (0);
}
/*! . */
int gmt_parse_model (struct GMT_CTRL *GMT, char option, char *in_arg, unsigned int dim, struct GMT_MODEL *M) {
	if (dim == 1)
		return (gmtinit_parse_model1d (GMT, option, in_arg, M));
	else
		return (gmtinit_parse_model2d (GMT, option, in_arg, M));
}

#endif

/* Some local macros to make the test below possible to understand */
#define is_plus(item,k)  (item[k-1] == '+')	/* Previous char is a plus, so maybe start of modifier */
#define is_label(item,k) (item[k] == 'c' && (item[k+1] == '+' || item[k+1] == '\0'))	/* Modifier c followed by another modifier or end of string */
#define is_just(item,k)  (item[k] == 'j' && (strchr ("LCRBMT", item[k+1]) && strchr ("LCRBMT", item[k+2])))	/* Is +j<just> */
#define is_off(item,k)   (item[k] == 'o' && strchr ("-+.0123456789", item[k+1]))	/* Looks like +onumber> */

/*! Parse the -U option.  Full syntax: -U[<label>][+c][+j<just>][+o]<dx>/<dy>]  Old syntax was -U[[<just>]/<dx>/<dy>/][c|<label>] */
GMT_LOCAL int gmtinit_parse_U_option (struct GMT_CTRL *GMT, char *item) {
	int just = 1, error = 0;

	GMT->current.setting.map_logo = true;
	if (!item || !item[0]) return (GMT_NOERROR);	/* Just basic -U with no args */

	if ((strstr (item, "+c")) || strstr (item, "+j") || strstr (item, "+o")) {	/* New syntax */
		unsigned int pos = 0, uerr = 0;
		int k = 1, len = (int)strlen (item);
		char word[GMT_LEN256] = {""}, *c = NULL;
		/* Find the first +c|j|o that looks like it may be a modifier and not random text */
		while (k < len && !(is_plus(item,k) && (is_label(item,k) || is_just(item,k) || is_off(item,k)))) k++;
		c = &item[k-1];	/* Start of the modifier */
		c[0] = '\0';	/* Chop off the + so we can parse the label, if any */
		if (item[0]) strncpy (GMT->current.ps.map_logo_label, item, GMT_LEN256-1);	/* Got a label */
		c[0] = '+';	/* Restore modifiers */
		while (gmt_getmodopt (GMT, 'U', c, "cjo", &pos, word, &uerr) && uerr == 0) {
			switch (word[0]) {
				case 'c':	/* Maybe +c but only if at end of followed by another modifier */
					if (word[1] == '+' || word[1] == '\0')	/* Use command string */
						GMT->current.ps.logo_cmd = true;
					break;
				case 'j':	/* Maybe +j if the next two letters are from LCRBMT */
					if (strchr ("LCRBMT", word[1]) && strchr ("LCRBMT", word[2]))
						just = gmt_just_decode (GMT, &word[1], GMT->current.setting.map_logo_justify);
					break;
				case 'o':	/* Maybe +o if next letter could be part of a number */
					if (strchr ("-+.0123456789", word[1])) {	/* Seems to be a number */
						if ((k = gmt_get_pair (GMT, &word[1], GMT_PAIR_DIM_DUP, GMT->current.setting.map_logo_pos)) < 2) error++;
					}
					break;
				default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
			}
		}
		GMT->current.setting.map_logo_justify = just;
	}
	else {	/* Old syntax or just -U */
		int n = 0, n_slashes;
		char txt_j[GMT_LEN256] = {""}, txt_x[GMT_LEN256] = {""}, txt_y[GMT_LEN256] = {""};
	
		n_slashes = gmtlib_count_char (GMT, item, '/');	/* Count slashes to detect [<just>]/<dx>/<dy>/ presence */

		if (n_slashes >= 2) {	/* Probably gave -U[<just>]/<dx>/<dy>[/<string>] */
			if (item[0] == '/') { /* No justification given */
				n = sscanf (&item[1], "%[^/]/%[^/]/%[^\n]", txt_x, txt_y, GMT->current.ps.map_logo_label);
				just = 1;	/* Default justification is LL */
			}
			else {
				n = sscanf (item, "%[^/]/%[^/]/%[^/]/%[^\n]", txt_j, txt_x, txt_y, GMT->current.ps.map_logo_label);
				just = gmt_just_decode (GMT, txt_j, GMT->current.setting.map_logo_justify);
			}
			if (just < 0) {
				/* Garbage before first slash: we simply have -U<string> */
				strncpy (GMT->current.ps.map_logo_label, item, GMT_LEN256-1);
			}
			else {
				GMT->current.setting.map_logo_justify = just;
				GMT->current.setting.map_logo_pos[GMT_X] = gmt_M_to_inch (GMT, txt_x);
				GMT->current.setting.map_logo_pos[GMT_Y] = gmt_M_to_inch (GMT, txt_y);
			}
		}
		else
			strncpy (GMT->current.ps.map_logo_label, item, GMT_LEN256-1);
		if (GMT->current.ps.map_logo_label[0] == 'c' && GMT->current.ps.map_logo_label[1] == 0) {	/* Old way of asking for +c */
			GMT->current.ps.logo_cmd = true;
			GMT->current.ps.map_logo_label[0] = '\0';
		}
		if ((item[0] == '/' && n_slashes == 1) || (item[0] == '/' && n_slashes >= 2 && n < 2)) error++;
	}
	return (error);
}

/*! -x[[-]<ncores>] */
#ifdef GMT_MP_ENABLED
GMT_LOCAL int gmtinit_parse_x_option (struct GMT_CTRL *GMT, char *arg) {
	GMT->common.x.active = true;
	if (!arg) return (GMT_PARSE_ERROR);	/* -x requires a non-NULL argument */
	if (arg[0] == '\0')	/* Use all processors */
		GMT->common.x.n_threads = gmtlib_get_num_processors();
	else
		GMT->common.x.n_threads = atoi (arg);

	if (GMT->common.x.n_threads == 0)
		GMT->common.x.n_threads = 1;
	else if (GMT->common.x.n_threads < 0)
		GMT->common.x.n_threads = MAX(gmtlib_get_num_processors() - GMT->common.x.n_threads, 1);		/* Max-n but at least one */
	return (GMT_NOERROR);
}
#endif

/*! . */
GMT_LOCAL int gmtinit_parse_colon_option (struct GMT_CTRL *GMT, char *item) {
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
		if (gmt_M_type (GMT, way, GMT_X) == GMT_IS_UNKNOWN && gmt_M_type (GMT, way, GMT_Y) == GMT_IS_UNKNOWN)	/* Don't know what x/y is yet */
			GMT->current.setting.io_lonlat_toggle[way] = true;
		else if (gmt_M_type (GMT, way, GMT_X) == GMT_IS_FLOAT && gmt_M_type (GMT, way, GMT_Y) == GMT_IS_FLOAT)	/* Cartesian x/y vs y/x cannot be identified */
			GMT->current.setting.io_lonlat_toggle[way] = true;
		else if (gmt_M_is_geographic (GMT, way))	/* Lon/lat becomes lat/lon */
			GMT->current.setting.io_lonlat_toggle[way] = true;
		else if (gmt_M_type (GMT, way, GMT_X) == GMT_IS_LAT && gmt_M_type (GMT, way, GMT_Y) == GMT_IS_LON)	/* Already lat/lon! */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "-:%s given but %s order already set by -f; -:%s ignored.\n", mode[way+off], dir[way], mode[way+off]);
	}
	if (error) GMT->current.setting.io_lonlat_toggle[GMT_IN] = GMT->current.setting.io_lonlat_toggle[GMT_OUT] = false;	/* Leave in case we had errors */
	return (error);
}

/*! Compute reverse col-separation before mapping */
GMT_LOCAL double gmtinit_neg_col_dist (struct GMT_CTRL *GMT, uint64_t col) {
	return (GMT->current.io.prev_rec[col] - GMT->current.io.curr_rec[col]);
}

/*! Compute forward col-separation before mapping */
GMT_LOCAL double gmtinit_pos_col_dist (struct GMT_CTRL *GMT, uint64_t col) {
	return (GMT->current.io.curr_rec[col] - GMT->current.io.prev_rec[col]);
}

/*! Compute absolute col-separation before mapping */
GMT_LOCAL double gmtinit_abs_col_dist (struct GMT_CTRL *GMT, uint64_t col) {
	return (fabs (GMT->current.io.curr_rec[col] - GMT->current.io.prev_rec[col]));
}

/*! Compute reverse col-separation after mapping */
GMT_LOCAL double gmtinit_neg_col_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	double X[2][2];
	gmt_geo_to_xy (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	gmt_geo_to_xy (GMT, GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][0] - X[col][1]);
}

/*! Compute forward col-separation after mapping */
GMT_LOCAL double gmtinit_pos_col_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	double X[2][2];
	gmt_geo_to_xy (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	gmt_geo_to_xy (GMT, GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (X[col][1] - X[col][0]);
}

/*! Compute forward col-separation after mapping */
GMT_LOCAL double gmtinit_abs_col_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	double X[2][2];
	gmt_geo_to_xy (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], &X[GMT_X][0], &X[GMT_Y][0]);
	gmt_geo_to_xy (GMT, GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y], &X[GMT_X][1], &X[GMT_Y][1]);
	return (fabs (X[col][1] - X[col][0]));
}

/*! Compute point-separation after mapping */
GMT_LOCAL double gmtinit_xy_map_dist (struct GMT_CTRL *GMT, uint64_t col) {
	gmt_M_unused(col);
	return (gmtlib_cartesian_dist_proj (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
GMT_LOCAL double gmtinit_xy_deg_dist (struct GMT_CTRL *GMT, uint64_t col) {
	gmt_M_unused(col);
	return (gmtlib_great_circle_dist_degree (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
GMT_LOCAL double gmtinit_xy_true_dist (struct GMT_CTRL *GMT, uint64_t col) {
	gmt_M_unused(col);
	return (gmt_great_circle_dist_meter (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! . */
GMT_LOCAL double gmtinit_xy_cart_dist (struct GMT_CTRL *GMT, uint64_t col) {
	gmt_M_unused(col);
	return (gmtlib_cartesian_dist (GMT, GMT->current.io.prev_rec[GMT_X], GMT->current.io.prev_rec[GMT_Y], GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]));
}

/*! Parse the -n option for 2-D grid resampling parameters -n[b|c|l|n][+a][+t<BC>][+<threshold>] */
int gmtinit_parse_n_option (struct GMT_CTRL *GMT, char *item) {
	unsigned int pos = 0, j, k = 1;
	char p[GMT_LEN256] = {""};

	strncpy (GMT->common.n.string, item, GMT_LEN64-1);	/* Make copy of -n argument verbatim */
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
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Use %s to set 2-D grid interpolation mode.\n", GMT_n_OPT);
			return (1);
			break;
	}

	/* Now look for +modifiers */

	while ((gmt_strtok (&item[k], "+", &pos, p))) {
		switch (p[0]) {
			case 'a':	/* Turn off antialias */
				GMT->common.n.antialias = false;
				break;
			case 'b':	/* Set BCs */
				GMT->common.n.bc_set = true;
				/* coverity[buffer_size_warning] */	/* Do not remove this comment */
				gmt_strncpy (GMT->common.n.BC, &p[1], 4U);
				for (j = 0; j < MIN (4,strlen (GMT->common.n.BC)); j++) {
					switch (GMT->common.n.BC[j]) {
						case 'g': case 'p': case 'x': case 'y': break;
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
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Use %s to set 2-D grid interpolation mode.\n", GMT_n_OPT);
				return (1);
				break;
		}
	}
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_parse_p_option (struct GMT_CTRL *GMT, char *item) {
	unsigned int k, l = 0, pos = 0, error = 0;
	double az, el = 0.0, z = 0.0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	char p[GMT_LEN256] = {""}, *c = NULL;

	/* -p[x|y|z]<azim>[/<elev>[/<zlevel>]][+w<lon0>/<lat0>[/<z0>][+v<x0>/<y0>] */

	if (!GMT->common.J.active) {
		gmt_set_missing_options (GMT, "J");	/* If mode is modern and -J exist in the history, and if an overlay we may add these from history automatically */
		if (!GMT->common.J.active)
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning -p option works best in consort with -J (and -R or a grid)\n");
	}
	switch (item[0]) {
		case 'x': GMT->current.proj.z_project.view_plane = GMT_X + GMT_ZW; l++; break;
		case 'y': GMT->current.proj.z_project.view_plane = GMT_Y + GMT_ZW; l++;	break;
		case 'z': GMT->current.proj.z_project.view_plane = GMT_Z + GMT_ZW; l++; break;
		default: GMT->current.proj.z_project.view_plane  = GMT_Z + GMT_ZW; break;
	}

	if ((c = gmt_first_modifier (GMT, item, "vw"))) c[0] = '\0';	/* Chop off modifiers so we can parse the info */

	if ((k = sscanf (&item[l], "%lf/%lf/%lf", &az, &el, &z)) < 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -p (%s): Syntax is %s\n", item, GMT_p_OPT);
		return GMT_PARSE_ERROR;
	}
	if (k == 1) { GMT->common.p.do_z_rotation = true; el = 90.0;}
	if (el <= 0.0 || el > 90.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -p option: Elevation must be in 0-90 range\n");
		return GMT_PARSE_ERROR;
	}
	if (c) {	/* Now process any modifiers */
		pos = 0;
		c[0] = '+';	/* Restore that character */
		while (gmt_getmodopt (GMT, 'p', c, "vw", &pos, p, &error) && error == 0) {
			switch (p[0]) {
				case 'v':	/* View point given in projected coordinates */
					if (sscanf (&p[1], "%[^/]/%s", txt_a, txt_b) != 2) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -p (%s): Syntax is %s\n", p, GMT_p_OPT);
						return GMT_PARSE_ERROR;
					}
					GMT->current.proj.z_project.view_x = gmt_M_to_inch (GMT, txt_a);
					GMT->current.proj.z_project.view_y = gmt_M_to_inch (GMT, txt_b);
					GMT->current.proj.z_project.view_given = true;
					break;
				case 'w':	/* Specify fixed World point in user's coordinates */
					if (sscanf (&p[1], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c) < 2) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -p (%s): Syntax is %s\n", p, GMT_p_OPT);
						return GMT_PARSE_ERROR;
					}
					error += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), &GMT->current.proj.z_project.world_x), txt_a);
					error += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), &GMT->current.proj.z_project.world_y), txt_b);
					if (k == 3) error += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf (GMT, txt_c, gmt_M_type (GMT, GMT_IN, GMT_Z), &GMT->current.proj.z_project.world_z), txt_c);
					GMT->current.proj.z_project.world_given = true;
					break;
				default:	/* These are caught in gmt_getmodopt so break is just for Coverity */
					break;
			}
		}
		c[0] = '\0';	/* Chop off all modifiers so az/el/z can be determined */
		if (!GMT->common.p.do_z_rotation) GMT->current.proj.z_project.fixed = true;
	}

	if (k >= 2) {
		GMT->current.proj.z_project.view_azimuth   = az;
		GMT->current.proj.z_project.view_elevation = el;
		if (k == 3) GMT->current.proj.z_level      = z;
	}
	else
		GMT->common.p.z_rotation = az;

	return (error);
}

/*! . */
GMT_LOCAL bool gmtinit_parse_s_option (struct GMT_CTRL *GMT, char *item) {
	unsigned int error = 0, n, pos = 0;
	int64_t i, start = -1, stop = -1, inc;
	char p[GMT_BUFSIZ] = {""}, tmp[GMT_MAX_COLUMNS] = {""}, *c = NULL;
	/* Parse the -s option.  Full syntax: -s[<cols>][+r|a] Old syntax was -s[<cols>][r|a] */

	gmt_M_memset (GMT->current.io.io_nan_col, GMT_MAX_COLUMNS, int);
	GMT->current.io.io_nan_col[0] = GMT_Z;	/* The default is to examine the z-column */
	GMT->current.io.io_nan_ncols = 1;		/* Default is that single z column */
	GMT->current.setting.io_nan_mode = GMT_IO_NAN_SKIP;	/* Plain -s */
	if (!item || !item[0]) return (false);	/* Nothing more to do */
	strncpy (GMT->common.s.string, item, GMT_LEN64-1);	/* Make copy of -n argument verbatim */
	if ((c = strstr (item, "+a")))
		GMT->current.setting.io_nan_mode = GMT_IO_NAN_ONE, c[0] = '\0';		/* Set -s+a */
	else if ((c = strstr (item, "+r")))
		GMT->current.setting.io_nan_mode = GMT_IO_NAN_KEEP, c[0] = '\0';		/* Set -s+r */
	n = (int)strlen (item);
	if (n == 0) {
		if (c) c[0] = '+';	/* Restore string */
		return (false);		/* Nothing more to do */
	}
	if (item[n-1] == 'a') GMT->current.setting.io_nan_mode = GMT_IO_NAN_ONE, n--;		/* Old syntax set -sa */
	else if (item[n-1] == 'r') GMT->current.setting.io_nan_mode = GMT_IO_NAN_KEEP, n--;	/* Old syntax set -sr */
	if (n == 0) return (false);		/* No column arguments to process */
	/* Here we have user-supplied column information */
	for (i = 0; i < GMT_MAX_COLUMNS; i++) tmp[i] = -1;
	while (!error && (gmt_strtok (item, ",", &pos, p))) {	/* While it is not empty, process it */
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
	if (c) c[0] = '+';	/* Restore string */

	return (false);
}

/*! Check that special map-related codes are present - if not give warning */
GMT_LOCAL void gmtinit_verify_encodings (struct GMT_CTRL *GMT) {

	/* First check for degree symbol */

	if (GMT->current.setting.ps_encoding.code[gmt_ring] == 32 && GMT->current.setting.ps_encoding.code[gmt_degree] == 32) {	/* Neither /ring or /degree encoded */
		gmt_message (GMT, "Selected character encoding does not have suitable degree symbol - will use space instead\n");
	}
	else if (GMT->current.setting.map_degree_symbol == gmt_ring && GMT->current.setting.ps_encoding.code[gmt_ring] == 32) {		/* want /ring but only /degree is encoded */
		gmt_message (GMT, "Selected character encoding does not have ring symbol - will use degree symbol instead\n");
		GMT->current.setting.map_degree_symbol = gmt_degree;
	}
	else if (GMT->current.setting.map_degree_symbol == gmt_degree && GMT->current.setting.ps_encoding.code[gmt_degree] == 32) {	/* want /degree but only /ring is encoded */
		gmt_message (GMT, "Selected character encoding does not have degree symbol - will use ring symbol instead\n");
		GMT->current.setting.map_degree_symbol = gmt_ring;
	}

	/* Then single quote for minute symbol... */

	if (GMT->current.setting.map_degree_symbol < 2 && GMT->current.setting.ps_encoding.code[gmt_squote] == 32) {
		gmt_message (GMT, "Selected character encoding does not have minute symbol (single quote) - will use space instead\n");
	}

	/* ... and double quote for second symbol */

	if (GMT->current.setting.map_degree_symbol < 2 && GMT->current.setting.ps_encoding.code[gmt_dquote] == 32) {
		gmt_message (GMT, "Selected character encoding does not have second symbol (double quote) - will use space instead\n");
	}
}

/*! . */
GMT_LOCAL bool gmtinit_true_false_or_error (char *value, bool *answer) {
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
GMT_LOCAL int gmtinit_decode4_wesnz (struct GMT_CTRL *GMT, const char *in, unsigned int side[], bool *draw_box, int part) {
	/* Scans the WESNZwesnz+ flags at the end of string "in" and sets the side/drawbox parameters
	 * and returns the length of the remaining string.  Assumes any +g<fill> has been removed from in.
	 */

	int i, k;
	bool go = true;

	GMT->current.map.frame.set_frame[part]++;
	if (GMT->current.map.frame.set_frame[GMT_PRIMARY] > 1 || GMT->current.map.frame.set_frame[GMT_SECONDARY] > 1) {
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error -B: <WESNZ-framesettings> given more than once!\n");
		return (1);
	}
	i = (int)strlen (in);
	if (i == 0) return (0);

	for (k = 0, i--; go && i >= 0 && strchr ("WESNZwesnz+", in[i]); i--) {
		if (k == 0 && part == 0) {	/* Wipe out default values when the first flag is found */
			for (k = 0; k < 5; k++) side[k] = 0;
			*draw_box = false;
		}
		if (in[i] == 's') {	/* Since s can mean both "draw south axis" and "seconds", check further */
			if (side[S_SIDE]) go = false;	/* If S was set already then s probably means seconds */
			else if (i && in[i-1] == ',') go = true;	/* Special case of ,s to indicate south, e.g. -B30,s */
			else if (i && (in[i-1] == '.' || isdigit ((int)in[i-1]))) go = false;	/* Probably seconds, e.g. -B30s */
			if (!go) { i++; continue; }	/* Break out of loop */
		}
		switch (in[i]) {
			/* Draw AND Annotate */
			case 'W': side[W_SIDE] |= GMT_AXIS_ALL; break;
			case 'E': side[E_SIDE] |= GMT_AXIS_ALL; break;
			case 'S': side[S_SIDE] |= GMT_AXIS_ALL; break;
			case 'N': side[N_SIDE] |= GMT_AXIS_ALL; break;
			case 'Z': side[Z_SIDE] |= GMT_AXIS_ALL; break;
			/* Just Draw and tick */
			case 'w': side[W_SIDE] |= GMT_AXIS_BARB; break;
			case 'e': side[E_SIDE] |= GMT_AXIS_BARB; break;
			case 's': side[S_SIDE] |= GMT_AXIS_BARB; break;
			case 'n': side[N_SIDE] |= GMT_AXIS_BARB; break;
			case 'z': side[Z_SIDE] |= GMT_AXIS_BARB; break;
			/* Draw 3-D box */
			case '+': *draw_box = true; break;
		}
	}
	if (i >= 0 && in[i] == ',') i--;	/* Special case for -BCcustomfile,WESNwesn to avoid the filename being parsed for WESN */

	return (i+1);	/* Return remaining string length */
}

GMT_LOCAL void gmtinit_reset_colformats (struct GMT_CTRL *GMT) {
	unsigned int i;
	for (i = 0; i < GMT_MAX_COLUMNS; i++) if (GMT->current.io.o_format[i])
		gmt_M_str_free (GMT->current.io.o_format[i]);
}

/*! . */
GMT_LOCAL void gmtinit_parse_format_float_out (struct GMT_CTRL *GMT, char *value) {

	unsigned int pos = 0, col = 0, k;
	char fmt[GMT_LEN64] = {""};
	strncpy (GMT->current.setting.format_float_out_orig, value, GMT_LEN256-1);
	if (strchr (value, ',')) {
		unsigned int start = 0, stop = 0, error = 0;
		char *p = NULL;
		/* Look for multiple comma-separated format statements of type [<cols>:]<format>.
		 * Last format also becomes the default for unspecified columns */
		gmtinit_reset_colformats (GMT);	/* Wipe previous settings */
		while ((gmt_strtok (value, ",", &pos, fmt))) {
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
		strncpy (GMT->current.setting.format_float_out, GMT->current.io.o_format[col], GMT_LEN64-1);
	}
	else if (strchr (value, ' ')) {
		/* Look for N space-separated format statements of type <format1> <format2> <format3> ...
		 * and let these apply to the first N output columns.
		 * Last format also becomes the default for unspecified columns. */
		gmtinit_reset_colformats (GMT);	/* Wipe previous settings */
		k = 0;
		while ((gmt_strtok (value, " ", &pos, fmt)))
			GMT->current.io.o_format[k++] = strdup (fmt);
		if (k) strncpy (GMT->current.setting.format_float_out, GMT->current.io.o_format[k-1], GMT_LEN64-1);
	}
	else {	/* No columns, set the default format */
		gmtinit_reset_colformats (GMT);	/* Wipe previous settings */
		strncpy (GMT->current.setting.format_float_out, value, GMT_LEN64-1);
	}
}

/*! . */
GMT_LOCAL bool gmtinit_badvalreport (struct GMT_CTRL *GMT, const char *keyword) {
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized keyword %s. You may have been using a deprecated GMT3 or GMT4 keyword.\nChange keyword or use with GMT_COMPATIBILITY=4. " GMT_COMPAT_INFO, keyword);
	return (true);
}

/*! . */
GMT_LOCAL int gmtinit_savedefaults (struct GMT_CTRL *GMT, char *file) {
	bool header = false;
	int case_val;
	unsigned int k = 0, current_group = 0;
	FILE *fpo = NULL;

	if (file[0] == '-' && !file[1])
		fpo = GMT->session.std[GMT_OUT];
	else if ((fpo = fopen (file, "w")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not create file %s\n", file);
		return (-1);
	}

	fprintf (fpo, "#\n# GMT %d.%d.%d Defaults file\n", GMT_MAJOR_VERSION, GMT_MINOR_VERSION, GMT_RELEASE_VERSION);
	while (GMT5_keywords[k].name != NULL) {
		if (GMT5_keywords[k].code == 1) {	/* Start of new group */
			current_group = k++;
			header = false;
			continue;
		}
		case_val = gmt_hash_lookup (GMT, GMT5_keywords[k].name, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
		if (case_val >= 0 && !GMT_keywords_updated[case_val])
			{k++; continue;}	/* If equal to default, skip it */
		if ((case_val == GMTCASE_PS_MEDIA || case_val == GMTCASE_PAPER_MEDIA) && GMT->current.setting.run_mode == GMT_MODERN && GMT->current.ps.crop_to_fit)
			{k++; continue;}	/* Unless ps format is requested we do not honor PS_MEDIA requests */
		if (!header) {
			fprintf (fpo, "#\n# %s\n#\n", GMT5_keywords[current_group].name);
			header = true;
		}
		fprintf (fpo, "%-30s = %s\n", GMT5_keywords[k].name, gmtlib_putparameter (GMT, GMT5_keywords[k].name));
		k++;
	}

	if (fpo != GMT->session.std[GMT_OUT]) fclose (fpo);

	return (0);
}

/*! . */
GMT_LOCAL void gmtinit_append_trans (char *text, double transparency) {
	char trans[GMT_LEN64] = {""};
	if (!gmt_M_is_zero (transparency) && text[0] != '-') {	/* Append nonzero transparency */
		snprintf (trans, GMT_LEN64, "@%ld", lrint (100.0 * transparency));
		strcat (text, trans);
	}
}

/*! Checks if t fits the format [+|-][xxxx][.][yyyy][e|E[+|-]nn]. */
GMT_LOCAL bool gmtinit_is_valid_number (char *t) {
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
GMT_LOCAL int gmtinit_hash (struct GMT_CTRL *GMT, const char *v, unsigned int n_hash) {
	int h;
	gmt_M_unused(GMT);
	assert (v!=NULL); /* We are in trouble if we get a NULL pointer here */
	for (h = 0; *v != '\0'; v++) h = (64 * h + (*v)) % n_hash;
	while (h < 0) h += n_hash;
	return (h);
}

/*! Read user's gmt.io file and initialize shorthand notation */
GMT_LOCAL int gmtinit_setshorthand (struct GMT_CTRL *GMT) {
	unsigned int id, n = 0;
	size_t n_alloc = 0;
	char file[PATH_MAX] = {""}, line[GMT_BUFSIZ] = {""}, a[GMT_LEN64] = {""}, b[GMT_LEN64] = {""};
	char c[GMT_LEN64] = {""}, d[GMT_LEN64] = {""}, e[GMT_LEN64] = {""};
	FILE *fp = NULL;

	GMT->session.n_shorthands = 0; /* By default there are no shorthands unless gmt.io is found */

	if (!gmtlib_getuserpath (GMT, "gmt.io", file)) {
		if (!gmt_getsharepath (GMT, "", "gmt.io", "", file, R_OK)) {	/* try non-hidden file in ~/.gmt */
			if (gmt_M_compat_check (GMT, 4)) {	/* Look for obsolete .gmt_io files */
				if (!gmtlib_getuserpath (GMT, ".gmt_io", file)) {
					if (!gmt_getsharepath (GMT, "", "gmt_io", "", file, R_OK))	/* try non-hidden file in ~/.gmt */
						return GMT_OK;
				}
			}
			else
				return GMT_OK;
		}
	}
	if ((fp = fopen (file, "r")) == NULL)
		return GMT_OK;

	gmt_set_meminc (GMT, GMT_TINY_CHUNK); /* Only allocate a small amount */
	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf (line, "%s %s %s %s %s", a, b, c, d, e) != 5) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding file %s.  Bad format? [%s]\n", file, line);
			fclose (fp);
			GMT_exit (GMT, GMT_DATA_READ_ERROR); return GMT_DATA_READ_ERROR;
		}

		if (n == n_alloc)
			GMT->session.shorthand = gmt_M_malloc (GMT, GMT->session.shorthand, n, &n_alloc, struct GMT_SHORTHAND);

		GMT->session.shorthand[n].suffix = strdup (a);
		if (gmt_grd_format_decoder (GMT, b, &id) != GMT_NOERROR) {
			/* no valid type id */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unknown shorthand format [%s]\n", file, b);
			fclose (fp);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;
		}
		snprintf (line, GMT_BUFSIZ, "%s/%s/%s/%s", b, c, d, e); /* ff/scale/offset/invalid */
		GMT->session.shorthand[n].format = strdup (line);
		++n;
	}
	fclose (fp);

	n_alloc = GMT->session.n_shorthands = n;
	gmt_reset_meminc (GMT);
	GMT->session.shorthand = gmt_M_malloc (GMT, GMT->session.shorthand, 0, &n_alloc, struct GMT_SHORTHAND);
	return GMT_OK;
}

/*! . */
GMT_LOCAL void gmtinit_freeshorthand (struct GMT_CTRL *GMT) {/* Free memory used by shorthand arrays */
	unsigned int i;

	if (GMT->session.n_shorthands == 0)
		return;

	for (i = 0; i < GMT->session.n_shorthands; ++i) {
		gmt_M_str_free (GMT->session.shorthand[i].suffix);
		gmt_M_str_free (GMT->session.shorthand[i].format);
	}
	gmt_M_free (GMT, GMT->session.shorthand);
}

/*! . */
GMT_LOCAL int gmtinit_get_history (struct GMT_CTRL *GMT) {
	int id;
	size_t len = strlen ("BEGIN GMT " GMT_PACKAGE_VERSION);
	bool done = false, process = false;
	char line[GMT_BUFSIZ] = {""}, hfile[PATH_MAX] = {""}, cwd[PATH_MAX] = {""};
	char option[GMT_LEN64] = {""}, value[GMT_BUFSIZ] = {""};
	FILE *fp = NULL; /* For gmt.history file */
	static struct GMT_HASH unique_hashnode[GMT_N_UNIQUE];

	if (!(GMT->current.setting.history & GMT_HISTORY_READ))
		return (GMT_NOERROR); /* gmt.history mechanism has been disabled */

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Enter: gmtinit_get_history\n");

	/* This is called once per GMT Session by GMT_Create_Session via gmt_begin and before any GMT_* module is called.
	 * It loads in the known shorthands found in the gmt.history file
	 */

	/* If current directory is writable, use it; else use the home directory */

	if (getcwd (cwd, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory.\n");
	}
	if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Modern mode: Use the workflow directory and one history per figure */
		int fig = gmt_get_current_figure (GMT->parent);
		sprintf (hfile, "%s/%s.%d", GMT->parent->gwf_dir, GMT_HISTORY_FILE, fig);
	}
	else if (GMT->session.TMPDIR)			/* Isolation mode: Use GMT->session.TMPDIR/gmt.history */
		sprintf (hfile, "%s/%s", GMT->session.TMPDIR, GMT_HISTORY_FILE);
	else if (!access (cwd, W_OK))		/* Current directory is writable */
		sprintf (hfile, "%s", GMT_HISTORY_FILE);
	else if (GMT->session.HOMEDIR)	/* Try home directory instead */
		sprintf (hfile, "%s/%s", GMT->session.HOMEDIR, GMT_HISTORY_FILE);
	else {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "No writable directory found for gmt history - skipping it.\n");
		return (GMT_NOERROR);
	}
	if ((fp = fopen (hfile, "r+")) == NULL) /* In order to place an exclusive lock, fp must be open for writing */
		return (GMT_NOERROR);	/* OK to be unsuccessful in opening this file */

	gmt_hash_init (GMT, unique_hashnode, GMT_unique_option, GMT_N_UNIQUE, GMT_N_UNIQUE);

	/* When we get here the file exists */
	gmtinit_file_lock (GMT, fileno(fp));
	/* Format of GMT gmt.history is as follow:
	 * BEGIN GMT <version>		This is the start of parsable section
	 * OPT ARG
	 * where OPT is a 1- or 2-char code, e.g., R, X, JM, JQ, Js.  ARG is the argument.
	 * Exception: if OPT = J then ARG is just the first character of the argument  (e.g., M).
	 * File ends when we find
	 * END				This is the end of parsable section
	 */

	while (!done && fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments lines */
		gmt_chop (line);		/* Remove linefeed,CR */
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
		if (option[0] == '@') {	/* PostScript information */
			if (option[1] == 'C')	/* Read clip level */
				GMT->current.ps.clip_level = atoi (value);
			else if (option[1] == 'L')	/* Read PS layer */
				GMT->current.ps.layer = atoi (value);
			continue;
		}
		if ((id = gmt_hash_lookup (GMT, option, unique_hashnode, GMT_N_UNIQUE, GMT_N_UNIQUE)) < 0) continue;	/* Quietly skip malformed lines */
		if (GMT->init.history[id])
			gmt_M_str_free (GMT->init.history[id]);
		GMT->init.history[id] = strdup (value);
	}

	/* Close the file */
	gmtinit_file_unlock (GMT, fileno(fp));
	fclose (fp);

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Exit:  gmtinit_get_history\n");

	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_put_history (struct GMT_CTRL *GMT) {
	int id;
	bool empty;
	char hfile[PATH_MAX] = {""}, cwd[PATH_MAX] = {""};
	FILE *fp = NULL; /* For gmt.history file */

	if (GMT->parent->clear) {	/* gmt clear history was called, override put history once */
		GMT->parent->clear = false;
		return (GMT_NOERROR); /* gmt.history mechanism has been disabled */
	}
	
	if (!(GMT->current.setting.history & GMT_HISTORY_WRITE)) {
		if (GMT->current.setting.run_mode == GMT_MODERN && GMT->current.setting.history == GMT_HISTORY_OFF)
			GMT->current.setting.history = GMT->current.setting.history_orig;
		return (GMT_NOERROR); /* gmt.history mechanism has been disabled */
	}

	/* This is called once per GMT Session by gmt_end via GMT_Destroy_Session.
	 * It writes out the known shorthands to the gmt.history file
	 */

	/* Do we even need to write? If empty, simply skip */
	for (id = 0, empty = true; id < GMT_N_UNIQUE && empty; id++) {
		if (GMT->init.history[id]) empty = false;	/* Have something to write */
	}
	if (empty) return (GMT_NOERROR);

	/* If current directory is writable, use it; else use the home directory */

	if (getcwd (cwd, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory.\n");
	}
	if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Modern mode: Use the workflow directory */
		int fig = gmt_get_current_figure (GMT->parent);
		sprintf (hfile, "%s/%s.%d", GMT->parent->gwf_dir, GMT_HISTORY_FILE, fig);
	}
	else if (GMT->session.TMPDIR)			/* Classic isolation mode: Use GMT->session.TMPDIR/gmt.history */
		sprintf (hfile, "%s/%s", GMT->session.TMPDIR, GMT_HISTORY_FILE);
	else if (!access (cwd, W_OK))	/* Current directory is writable */
		sprintf (hfile, "%s", GMT_HISTORY_FILE);
	else if (GMT->session.HOMEDIR)	/* Try home directory instead */
		sprintf (hfile, "%s/%s", GMT->session.HOMEDIR, GMT_HISTORY_FILE);
	else {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine a writeable directory - gmt history not updated.\n");
		return (GMT_NOERROR);
	}
	if ((fp = fopen (hfile, "w")) == NULL) return (-1);	/* Not OK to be unsuccessful in creating this file */

	/* When we get here the file is open */
	if (!gmtinit_file_lock (GMT, fileno(fp)))
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Directory %s is not locked for exclusive access. Multiple gmt processes running at once could corrupt history file.\n", hfile);

	fprintf (fp, "# GMT %d Session common arguments shelf\n", GMT_MAJOR_VERSION);
	fprintf (fp, "BEGIN GMT " GMT_PACKAGE_VERSION "\n");
	for (id = 0; id < GMT_N_UNIQUE; id++) {
		if (!GMT->init.history[id]) continue;	/* Not specified */
		fprintf (fp, "%s\t%s\n", GMT_unique_option[id], GMT->init.history[id]);
	}
	if (GMT->current.ps.clip_level) fprintf (fp, "@C\t%d\n", GMT->current.ps.clip_level); /* Write clip level */
	if (GMT->current.ps.layer) fprintf (fp, "@L\t%d\n", GMT->current.ps.layer); /* Write PS layer, if non-zero */
	fprintf (fp, "END\n");

	/* Close the file */
	gmtinit_file_unlock (GMT, fileno(fp));
	fclose (fp);

	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL void gmtinit_free_plot_array (struct GMT_CTRL *GMT) {
	if (GMT->current.plot.n_alloc) {
		gmt_M_free (GMT, GMT->current.plot.x);
		gmt_M_free (GMT, GMT->current.plot.y);
		gmt_M_free (GMT, GMT->current.plot.pen);
	}
	GMT->current.plot.n = GMT->current.plot.n_alloc = 0;
}

GMT_LOCAL void trim_off_any_slash_at_end (char *dir) {
	size_t L = strlen (dir);	/* Get length of dir */
	if (L && (dir[L-1] == '/' || dir[L-1] == '\\')) dir[L-1] = '\0', L--;	/* Remove a trailing slash */
	if (L) L--;	/* L is now at last character in dir */
	while (L && dir[L] == ' ') dir[L] = '\0', L--;	/* Remove trailing spaces in directory names */
}

/*! . */
GMT_LOCAL int gmtinit_set_env (struct GMT_CTRL *GMT) {
	char *this_c = NULL, path[PATH_MAX+1];
	static char *how[2] = {"detected", "created"};
	unsigned int u = 0, c = 0;
	int err;
	struct GMTAPI_CTRL *API = GMT->parent;
	struct stat S;

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

	/* Note: gmtinit_set_env cannot use GMT_Report because the verbose level is not yet set */

	if ((this_c = getenv ("GMT6_SHAREDIR")) != NULL)	/* GMT6_SHAREDIR was set */
		GMT->session.SHAREDIR = strdup (this_c);
	else if ((this_c = getenv ("GMT5_SHAREDIR")) != NULL)	/* GMT5_SHAREDIR was set */
		GMT->session.SHAREDIR = strdup (this_c);
	else if ((this_c = getenv ("GMT_SHAREDIR")) != NULL) /* GMT_SHAREDIR was set */
		GMT->session.SHAREDIR = strdup (this_c);
#ifdef SUPPORT_EXEC_IN_BINARY_DIR
	else if (running_in_bindir_src)
		/* Use ${GMT_SOURCE_DIR}/share to simplify debugging and running in GMT_BINARY_DIR */
		GMT->session.SHAREDIR = strdup (GMT_SHARE_DIR_DEBUG);
#endif
	else if (!access (GMT_SHARE_DIR, F_OK|R_OK))		/* Found in hardcoded GMT_SHARE_DIR pointing to an existant directory */
		GMT->session.SHAREDIR = strdup (GMT_SHARE_DIR);
	else {
		/* SHAREDIR still not found, make a smart guess based on runpath: */
		if (gmt_guess_sharedir (path, GMT->init.runtime_bindir))
			GMT->session.SHAREDIR = strdup (path);
		else {
			/* Still not found */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not locate share directory for GMT.\n");
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
	}

	gmt_dos_path_fix (GMT->session.SHAREDIR);
	trim_off_any_slash_at_end (GMT->session.SHAREDIR);
	GMT_Report (API, GMT_MSG_DEBUG, "GMT->session.SHAREDIR = %s\n", GMT->session.SHAREDIR);

	/* Determine HOMEDIR (user home directory) */

	if ((this_c = getenv ("HOME")) != NULL)				/* HOME was set */
		GMT->session.HOMEDIR = strdup (this_c);
#ifdef WIN32
	else if ((this_c = getenv ("USERPROFILE")) != NULL)	/* USERPROFILE was set */
		GMT->session.HOMEDIR = strdup (this_c);
	else if ((this_c = getenv ("HOMEPATH")) != NULL)	/* HOMEPATH was set */
		GMT->session.HOMEDIR = strdup (this_c);
#endif
	else {
		/* If HOME not set: use root directory instead (http://gmt.soest.hawaii.edu/issues/710) */
		GMT->session.HOMEDIR = strdup ("/"); /* Note: Windows will use the current drive if drive letter unspecified. */
#ifdef DEBUG
		GMT_Report (API, GMT_MSG_NORMAL, "HOME environment not set. Using root directory instead.\n");
#endif
	}
	if (GMT->session.HOMEDIR) {	/* Mostly to keep Coverity happy */
		gmt_dos_path_fix (GMT->session.HOMEDIR);
		trim_off_any_slash_at_end (GMT->session.HOMEDIR);
		GMT_Report (API, GMT_MSG_DEBUG, "GMT->session.HOMEDIR = %s\n", GMT->session.HOMEDIR);
	}

	/* Determine GMT_USERDIR (directory containing user replacements contents in GMT_SHAREDIR) */

	if ((this_c = getenv ("GMT_USERDIR")) != NULL)		/* GMT_USERDIR was set */
		GMT->session.USERDIR = strdup (this_c);
	else if (GMT->session.HOMEDIR) {	/* Use default path for GMT_USERDIR (~/.gmt) */
		sprintf (path, "%s/%s", GMT->session.HOMEDIR, ".gmt");
		GMT->session.USERDIR = strdup (path);
		u = 1;
	}
	if (GMT->session.USERDIR) {
		gmt_dos_path_fix (GMT->session.USERDIR);
		trim_off_any_slash_at_end (GMT->session.USERDIR);
	}
	if (GMT->session.USERDIR != NULL) {
		err = stat (GMT->session.USERDIR, &S);	/* Stat the userdir path (which may not exist) */
		if (err == ENOENT && gmt_mkdir (GMT->session.USERDIR)) { /* Path does not exist so we create that dir */
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create GMT User directory : %s\n", GMT->session.USERDIR);
			GMT_Report (API, GMT_MSG_NORMAL, "Auto-downloading of @earth_relief_##m|s.grd files has been disabled.\n");
			GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;
			gmt_M_str_free (GMT->session.USERDIR);
		}
	}
	if ((this_c = getenv ("GMT_CACHEDIR")) != NULL)		/* GMT_CACHEDIR was set */
		GMT->session.CACHEDIR = strdup (this_c);
	else if (GMT->session.USERDIR != NULL) {	/* Use default path for GMT_CACHEDIR as GMT_USERDIR/cache */
		sprintf (path, "%s/%s", GMT->session.USERDIR, "cache");
		GMT->session.CACHEDIR = strdup (path);
		c = 1;
	}
	if (GMT->session.CACHEDIR) {
		gmt_dos_path_fix (GMT->session.CACHEDIR);
		trim_off_any_slash_at_end (GMT->session.CACHEDIR);
	}
	if (GMT->session.CACHEDIR != NULL) {
		err = stat (GMT->session.CACHEDIR, &S);	/* Stat the cachedir path (which may not exist) */
		if (err == ENOENT && gmt_mkdir (GMT->session.CACHEDIR)) {	/* Path does not exist so we create that dir */
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create GMT User cache directory : %s\n", GMT->session.CACHEDIR);
			GMT_Report (API, GMT_MSG_NORMAL, "Auto-downloading of cache data has been disabled.\n");
			GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;
			gmt_M_str_free (GMT->session.CACHEDIR);
		}
	}

	if ((this_c = getenv ("GMT_SESSIONDIR")) != NULL)		/* GMT_SESSIONDIR was set */
		API->session_dir = strdup (this_c);
	else if (GMT->session.USERDIR != NULL) {	/* Use GMT_USERDIR/sessions as default path for GMT_SESSIONDIR */
		sprintf (path, "%s/%s", GMT->session.USERDIR, "sessions");
		API->session_dir = strdup (path);
	}
	else {	/* Use the temp dir as the session dir */
		API->session_dir = strdup (API->tmp_dir);
		GMT_Report (API, GMT_MSG_NORMAL, "No GMT User directory set, GMT session dir selected: %s\n", API->session_dir);
	}
	if (API->session_dir) {
		gmt_dos_path_fix (API->session_dir);
		trim_off_any_slash_at_end (API->session_dir);
	}
	if (API->session_dir != NULL) {
		err = stat (API->session_dir, &S);	/* Stat the session path (which may not exist) */
		if (err == ENOENT && gmt_mkdir (API->session_dir)) { /* Path does not exist so we create that dir */
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create GMT User sessions directory : %s\n", API->session_dir);
			GMT_Report (API, GMT_MSG_NORMAL, "Modern mode will fail.\n");
		}
		else if (err == 0) {	/* Path already exists, check why */
			if (!S_ISDIR (S.st_mode))	/* Path already exists, but it is not a directory */
				GMT_Report (API, GMT_MSG_NORMAL, "A file named %s already exist and prevents us creating a session directory by that name\n", API->session_dir);
			else if (S_ISDIR (S.st_mode) && (S.st_mode & S_IWUSR) == 0)	/* Directory already exists but is not writeable */
				GMT_Report (API, GMT_MSG_NORMAL, "Session directory %s already exist but is not writeable\n", API->session_dir);
		}
	}
	if (GMT->session.USERDIR)  GMT_Report (API, GMT_MSG_DEBUG, "GMT->session.USERDIR = %s [%s]\n",  GMT->session.USERDIR,  how[u]);
	if (GMT->session.CACHEDIR) GMT_Report (API, GMT_MSG_DEBUG, "GMT->session.CACHEDIR = %s [%s]\n", GMT->session.CACHEDIR, how[c]);

	if (gmt_M_compat_check (GMT, 4)) {
		/* Check if obsolete GMT_CPTDIR was specified */

		if ((this_c = getenv ("GMT_CPTDIR")) != NULL) {		/* GMT_CPTDIR was set */
			GMT_Report (API, GMT_MSG_VERBOSE, "Environment variable GMT_CPTDIR was set but is no longer used by GMT.\n");
			GMT_Report (API, GMT_MSG_VERBOSE, "System-wide color tables are in %s/cpt.\n", GMT->session.SHAREDIR);
			GMT_Report (API, GMT_MSG_VERBOSE, "Use GMT_USERDIR (%s) instead and place user-defined color tables there.\n", GMT->session.USERDIR);
		}
	}

	if ((this_c = getenv ("GMT_DATA_URL")) != NULL)		/* GMT_DATA_URL was set */
		GMT->session.DATAURL = strdup (this_c);
	else
		GMT->session.DATAURL = strdup (GMT_DATA_URL);	/* SOEST default */
	if (GMT->session.DATAURL)
		trim_off_any_slash_at_end (GMT->session.DATAURL);

	/* Determine GMT_DATADIR (data directories) */

	if ((this_c = getenv ("GMT_DATADIR")) != NULL) {		/* GMT_DATADIR was set */
		if (strchr (this_c, PATH_SEPARATOR) || access (this_c, R_OK) == 0) {
			/* A list of directories or a single directory that is accessible */
			GMT->session.DATADIR = strdup (this_c);
			gmt_dos_path_fix (GMT->session.DATADIR);
		}
#ifdef WIN32
		else if (strchr(this_c, ':')) {		/* May happen to have ':' as a path separator when running a MSYS bash shell*/
			GMT->session.DATADIR = strdup(this_c);
			gmt_dos_path_fix(GMT->session.DATADIR);
		}
#endif
	}

	/* Determine GMT_TMPDIR (for isolation mode). Needs to exist use it. */

	if ((this_c = getenv ("GMT_TMPDIR")) != NULL) {		/* GMT_TMPDIR was set */
		if (access (this_c, R_OK|W_OK|X_OK)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Environment variable GMT_TMPDIR was set to %s, but directory is not accessible.\n", this_c);
			GMT_Report (API, GMT_MSG_VERBOSE, "GMT_TMPDIR needs to have mode rwx. Isolation mode switched off.\n");
			GMT->session.TMPDIR = NULL;
		}
		else {
			GMT->session.TMPDIR = strdup (this_c);
			gmt_dos_path_fix (GMT->session.TMPDIR);
			trim_off_any_slash_at_end (GMT->session.TMPDIR);
		}
	}
	return GMT_OK;
}

/* Here is the new -B parser with all its sub-functions */

#ifdef WIN32
/*! . */
GMT_LOCAL void gmtinit_handle_dosfile (struct GMT_CTRL *GMT, char *in, int this_mark) {
	/* Because (1) we use colons to indicate start/stop of text labels and
	 * (2) under Windows, a colon can be part of a path (e.g., C:\dir\file)
	 * we need to temporarily replace <drive>:\ with <drive>;\ so that this
	 * path colon does not interfere with the rest of the parsing.  Once the
	 * colon items have been parsed, we replace the ; back to : */
	int i, len, other = 1 - this_mark;
	char mark[2] = {':', ';'};
	gmt_M_unused(GMT);

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
GMT_LOCAL int gmtinit_strip_colonitem (struct GMT_CTRL *GMT, int axis, const char *in, const char *pattern, char *item, char *out) {
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
		if (in[i] != ':') error = true;		/* Missing terminating colon */
		i++;					/* Skip the ending colon */
		while (in[i]) out[k++] = in[i++];	/* Copy rest to out... */
		out[k] = '\0';				/* .. and terminate */
	}
	else	/* No item to update */
		strcpy (out, in);

	if (error) {	/* Problems with decoding */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Missing terminating colon in -B string %c-component %s\n", str[axis], in);
		return (1);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":.")) {	/* Problems with decoding title */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "More than one title in -B string %c-component %s\n", str[axis], in);
		return (1);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":,")) {	/* Problems with decoding unit */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "More than one unit string in -B %c-component %s\n", str[axis], in);
		return (1);
	}
	if (strstr (out, pattern) && !strcmp (pattern, ":=")) {	/* Problems with decoding prefix */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "More than one prefix string in  -B component %s\n", in);
		return (1);
	}
	if (strstr (out, pattern)) {	/* Problems with decoding label */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "More than one label string in  -B component %s\n", in);
		return (1);
	}
#ifdef _WIN32
	gmtinit_handle_dosfile (GMT, item, 1);	/* Undo any DOS files like X;/ back to X:/ */
#endif
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL void gmtinit_handle_atcolon (struct GMT_CTRL *GMT, char *txt, int old_p) {
	/* Way = 0: Replaces @:<size>: and @:: with @^<size>^ and @^^ to avoid trouble in -B:label: parsing;
	 * Way = 1: Restores it the way it was. */
	int new_p;
	char *item[2] = {"@:", "@^"}, mark[2] = {':', '^'}, *s = NULL;
	gmt_M_unused(GMT);

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
GMT_LOCAL int gmtinit_split_info_strings (struct GMT_CTRL *GMT, const char *in, char *x_info, char *y_info, char *z_info) {

	bool mute = false;
	size_t i, n_slash, s_pos[2];

	x_info[0] = y_info[0] = z_info[0] = '\0';

	for (i = n_slash = 0; in[i] && n_slash < 3; i++) {
		if (in[i] == ':') mute = !mute;
		if (in[i] == '/' && !mute) s_pos[n_slash++] = i;	/* Axis-separating slash, not a slash in a label */
	}

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
		GMT->current.map.frame.set_both = true;
	}
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_init_custom_annot (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS *A, int *n_int) {
	/* Reads a file with one or more records of the form
	 * value	types	[label]
	 * where value is the coordinate of the tickmark, types is a combination
	 * of a|i (annot or interval annot), f (tick), or g (gridline).
	 * The a|i will take a label string (or sentence).
	 * The item argument specifies which type to consider [a|i,f,g].  We return
	 * an array with coordinates and labels, and set interval to true if applicable.
	 */
	int error, k, n_errors = 0;
	bool save_trailing;
	unsigned int save_coltype, save_max_cols_to_read;
	uint64_t row;
	char type[GMT_LEN8] = {""};
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	/* Temporarily change what data type col one is */
	save_coltype = GMT->current.io.col_type[GMT_IN][GMT_X];
	save_trailing = GMT->current.io.trailing_text[GMT_IN];
	save_max_cols_to_read = GMT->current.io.max_cols_to_read;
	GMT->current.io.col_type[GMT_IN][GMT_X] = gmt_M_type (GMT, GMT_IN, A->id);
	gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading this file */
	GMT->current.io.record_type[GMT_IN] = GMT_READ_MIXED;
	GMT->current.io.trailing_text[GMT_IN] = true;
	if ((error = GMT_Set_Columns (GMT->parent, GMT_IN, 1, GMT_COL_FIX)) != GMT_NOERROR) return (1);
	if ((D = GMT_Read_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, A->file_custom, NULL)) == NULL) {
		GMT->current.io.col_type[GMT_IN][GMT_X] = save_coltype;
		return (1);
	}
	GMT->current.io.col_type[GMT_IN][GMT_X] = save_coltype;
	GMT->current.io.trailing_text[GMT_IN] = save_trailing;
	GMT->current.io.max_cols_to_read = save_max_cols_to_read;
	gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */

	gmt_M_memset (n_int, 4, int);
	S = D->table[0]->segment[0];	/* All we got */

	for (row = 0; row < S->n_rows; row++) {
		k = sscanf (S->text[row], "%s", type);
		if (k != 1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad format record [%s] at row %d in custom file %s.\n", S->text[row], (int)row, A->file_custom);
			n_errors++;
			continue;
		}
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
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognixed type (%c) at row %d in custom file %s.\n", type[k], (int)row, A->file_custom);
					n_errors++;
					break;
			}
		}
	}
	GMT_Destroy_Data (GMT->parent, &D);

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Processed custom annotations via %s for axis %d.\n", A->file_custom, A->id);
	if (n_int[0] && n_int[1]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot mix interval and regular annotations in custom file %s.\n", A->file_custom);
		n_errors++;
	}
	return (n_errors);
}

/*! Load the values into the appropriate GMT_PLOT_AXIS_ITEM structure */
GMT_LOCAL int gmtinit_set_titem (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS *A, char *in, char flag, char axis, int custom) {

	struct GMT_PLOT_AXIS_ITEM *I = NULL;
	char *format = NULL, *t = NULL, *s = NULL, unit = 0;
	double phase = 0.0, val = 0.0;

	t = in;

	/* Here, t must point to a valid number.  If t[0] is not [+,-,.] followed by a digit we have an error */

	if (strstr (t, "pi"))	/* Treat pi-fractions separately */
		gmt_scanf_float (GMT, in, &val);
	else {
		/* Decode interval, get pointer to next segment */
		if ((val = strtod (t, &s)) < 0.0 && GMT->current.proj.xyz_projection[A->id] != GMT_LOG10) {	/* Interval must be >= 0 */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Negative interval in -B option (%c-component, %c-info): %s\n", axis, flag, in);
			return (3);
		}
		if (s[0] && (s[0] == '-' || s[0] == '+')) {	/* Phase shift information given */
			t = s;
			phase = strtod (t, &s);
		}
	}
	if (val == 0.0 && t[0] && t == s) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad interval in -B option (%c-component, %c-info): %s gave interval = 0\n", axis, flag, in);
		return (3);
	}

	/* Appended one of the allowed units, or l or p for log10/pow */
	if (s && s[0] && strchr ("YyOoUuKkJjDdHhMmSsCcrRlp", s[0]))
		unit = s[0];
	else if (A->type == GMT_TIME)	/* Default time system unit implied, but use s if custom to avoid resetting of flag below */
		unit = (A->special == GMT_CUSTOM) ? 's' : GMT->current.setting.time_system.unit;
	else
		unit = 0;	/* Not specified */

	if (!GMT->current.map.frame.primary) flag = (char) toupper ((int)flag);

	if (A->type == GMT_TIME) {	/* Strict check on time intervals */
		if (gmtlib_verify_time_step (GMT, irint (val), unit)) {
			GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
		}
		if ((fmod (val, 1.0) > GMT_CONV8_LIMIT)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Time step interval (%g) must be an integer\n", val);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;
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
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad flag (%c) passed to gmtinit_set_titem\n", flag);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;
			break;
	}

	if (phase != 0.0) A->phase = phase;	/* phase must apply to entire axis */
	if (I->active) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Axis sub-item %c set more than once (typo?)\n", flag);
		return (GMT_NOERROR);
	}
	if (unit == 'c' || unit == 'C') {
		if (gmt_M_compat_check (GMT, 4)) {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Unit c (arcseconds) is deprecated; use s instead.\n");
			unit = 's';
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Unit %c not recognized.\n", unit);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;
		}
	}
	I->type = flag;
	I->unit = unit;
	I->interval = val;
	I->flavor = 0;
	I->active = true;
	if (!custom && in[0] && val == 0.0) I->active = false;
	I->upper_case = false;
	format = (GMT->current.map.frame.primary) ? GMT->current.setting.format_time[GMT_PRIMARY] : GMT->current.setting.format_time[GMT_SECONDARY];
	switch (format[0]) {	/* This parameter controls which version of month/day textstrings we use for plotting */
		case 'F':	/* Full name, upper case */
			I->upper_case = true;
			/* Fall through on purpose to 'f' */
		case 'f':	/* Full name, lower case */
			I->flavor = 0;
			break;
		case 'A':	/* Abbreviated name, upper case */
			I->upper_case = true;
			/* Fall through on purpose to 'a' */
		case 'a':	/* Abbreviated name, lower case */
			I->flavor = 1;
			break;
		case 'C':	/* 1-char name, upper case */
			I->upper_case = true;
			/* Fall through on purpose to 'c' */
		case 'c':	/* 1-char name, lower case */
			I->flavor = 2;
			break;
		default:
			break;
	}

	GMT->current.map.frame.draw = true;
	if (axis == 'z') GMT->current.map.frame.drawz = true;

	return (GMT_NOERROR);
}

/*! Decode the annot/tick segments of the clean -B string pieces */
GMT_LOCAL int gmtinit_decode_tinfo (struct GMT_CTRL *GMT, int axis, char flag, char *in, struct GMT_PLOT_AXIS *A) {

	char *str = "xyz";

	if (!in) return (GMT_NOERROR);	/* NULL pointer passed */

	if (flag == 'c') {	/* Custom annotation arrangement */
		int k, n_int[4];
		char *list = "aifg";
		if (!(gmt_access (GMT, &in[1], R_OK))) {
			gmt_M_str_free (A->file_custom);
			A->file_custom = strdup (&in[1]);
			A->special = GMT_CUSTOM;
			if (gmtinit_init_custom_annot (GMT, A, n_int)) return (-1);	/* See what ticks, anots, gridlines etc are requested */
			for (k = 0; k < 4; k++) {
				if (n_int[k] == 0) continue;
				flag = list[k];
				if (!GMT->current.map.frame.primary) flag = (char)toupper ((int)flag);
				gmtinit_set_titem (GMT, A, "0", flag, str[axis], true);	/* Store the findings for this segment */
			}
			if (n_int[1]) A->item[GMT_ANNOT_UPPER+!GMT->current.map.frame.primary].special = true;
			GMT->current.map.frame.draw = true;
			if (axis == GMT_Z) GMT->current.map.frame.drawz = true;
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot access custom file in -B string %c-component %s\n", str[axis], &in[1]);
	}
	else
		gmtinit_set_titem (GMT, A, in, flag, str[axis], false);

	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_parse4_B_option (struct GMT_CTRL *GMT, char *in) {
	/* gmtinit_parse4_B_option scans an argument string and extract parameters that
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
	strncpy (GMT->common.B.string[i], in, GMT_LEN256-1);	/* Keep a copy of the actual option(s) */

	/* GMT->current.map.frame.side[] may be set already when parsing gmt.conf flags */

	if (!GMT->current.map.frame.init) {	/* First time we initialize stuff */
		for (i = 0; i < 3; i++) {
			gmt_M_memset (&GMT->current.map.frame.axis[i], 1, struct GMT_PLOT_AXIS);
			GMT->current.map.frame.axis[i].id = i;
			for (j = 0; j < 6; j++) GMT->current.map.frame.axis[i].item[j].parent = i;
			if (GMT->current.proj.xyz_projection[i] == GMT_TIME) GMT->current.map.frame.axis[i].type = GMT_TIME;
		}
		GMT->current.map.frame.header[0] = '\0';
		GMT->current.map.frame.init = true;
		GMT->current.map.frame.draw = false;
		GMT->current.map.frame.set_frame[GMT_PRIMARY] = GMT->current.map.frame.set_frame[GMT_SECONDARY] = 0;
	}

#ifdef _WIN32
	gmtinit_handle_dosfile (GMT, in, 0);	/* Temporarily replace DOS files like X:/ with X;/ to avoid colon trouble */
#endif

	for (i = (int)strlen(in) - 1, ignore = false; !GMT->current.map.frame.paint && !error && i >= 0; i--) {	/** Look for +g<fill */
		if (in[i] == ':') ignore = !ignore;
		if (ignore) continue;	/* Not look inside text items */
		if (in[i] == '+' && in[i+1] == 'o') {	/* Found +o<plon>/<plat> */
			double lon, lat;
			char A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""};
			if (GMT->current.proj.projection_GMT == GMT_OBLIQUE_MERC) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Cannot specify oblique gridlines for the oblique Mercator projection\n");
				error++;
			}
			GMT->current.map.frame.obl_grid = true;
			if (sscanf (&in[i+2], "%[^/]/%[^+]", A, B) != 2) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Did not find the expected format +o<plon>/<plat>\n");
				error++;
			}
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, A, GMT_IS_LON, &lon), A);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, B, GMT_IS_LAT, &lat), B);
			if (GMT->current.proj.projection_GMT != GMT_OBLIQUE_MERC) gmtlib_set_oblique_pole_and_origin (GMT, lon, lat, 0.0, 0.0);
			o = i;
			in[o] = '\0';	/* Chop off +o for now */
		}
		if (in[i] == '+' && in[i+1] == 'g') {	/* Found +g<fill> */
			strncpy (out1, &in[i+2], GMT_BUFSIZ-1);	/* Make a copy of the fill argument */
#ifdef _WIN32
			gmtinit_handle_dosfile (GMT, out1, 1);	/* Undo any DOS files like X;/ back to X:/ */
#endif
			if (gmt_getfill (GMT, out1, &GMT->current.map.frame.fill)) error++;
			if (!error) {
				GMT->current.map.frame.paint = true;
				g = i;
				in[g] = '\0';	/* Chop off +g for now */
			}
		}
	}
	/* Note that gmtinit_strip_colonitem calls gmtinit_handle_dosfile so that the item return has been processed for DOS path restoration */
	error += gmtinit_strip_colonitem (GMT, 0, &in[k], ":.", GMT->current.map.frame.header, out1);	/* Extract header string, if any */
	gmtlib_enforce_rgb_triplets (GMT, GMT->current.map.frame.header, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */

	i = gmtinit_decode4_wesnz (GMT, out1, GMT->current.map.frame.side, &GMT->current.map.frame.draw_box, part);		/* Decode WESNZwesnz+ flags, if any */
	out1[i] = '\0';	/* Strip the WESNZwesnz+ flags off */

	gmtinit_split_info_strings (GMT, out1, info[0], info[1], info[2]);	/* Chop/copy the three axis strings */

	for (i = 0; i < 3; i++) {	/* Process each axis separately */

		if (!info[i][0]) continue;	 /* Skip empty format string */
		if (info[i][0] == '0' && !info[i][1]) {	 /* Skip format '0' */
			GMT->current.map.frame.draw = true;
			if (i == GMT_Z) GMT->current.map.frame.drawz = true;
			continue;
		}

		gmtinit_handle_atcolon (GMT, info[i], 0);	/* Temporarily modify text escape @: to @^ to avoid : parsing trouble */
		gmtlib_enforce_rgb_triplets (GMT, info[i], GMT_BUFSIZ);				/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
		error += gmtinit_strip_colonitem (GMT, i, info[i], ":,", GMT->current.map.frame.axis[i].unit, out1);	/* Pull out annotation unit, if any */
		error += gmtinit_strip_colonitem (GMT, i, out1, ":=", GMT->current.map.frame.axis[i].prefix, out2);	/* Pull out annotation prefix, if any */
		error += gmtinit_strip_colonitem (GMT, i, out2, ":", GMT->current.map.frame.axis[i].label, out3);	/* Pull out axis label, if any */
		gmtinit_handle_atcolon (GMT, GMT->current.map.frame.axis[i].label, 1);	/* Restore any @^ to @: */
		gmtinit_handle_atcolon (GMT, GMT->current.map.frame.axis[i].prefix, 1);	/* Restore any @^ to @: */
		gmtinit_handle_atcolon (GMT, GMT->current.map.frame.axis[i].unit, 1);	/* Restore any @^ to @: */

		if (GMT->current.map.frame.axis[i].prefix[0]) {	/* Deal with space/no space before prefix */
			char workspace[GMT_LEN64] = {""};
			if (GMT->current.map.frame.axis[i].prefix[0] == '-') /* Don't want a space */
				strncpy (workspace, &GMT->current.map.frame.axis[i].prefix[1], GMT_LEN64-1);
			else {	/* Want a space */
				workspace[0] = ' ';	/* The leading space */
				strncpy (&workspace[1], GMT->current.map.frame.axis[i].prefix, GMT_LEN64-1);
			}
			gmt_M_memcpy (GMT->current.map.frame.axis[i].prefix, workspace, GMT_LEN64, char);
		}
		if (GMT->current.map.frame.axis[i].unit[0]) {	/* Deal with space/no space before unit */
			char workspace[GMT_LEN64] = {""};
			if (GMT->current.map.frame.axis[i].unit[0] == '-') /* Don't want a space */
				strncpy (workspace, &GMT->current.map.frame.axis[i].unit[1], GMT_LEN64-1);
			else {	/* Want a space */
				workspace[0] = ' ';	/* The leading space */
				strncpy (&workspace[1], GMT->current.map.frame.axis[i].unit, GMT_LEN64-1);
			}
			gmt_M_memcpy (GMT->current.map.frame.axis[i].unit, workspace, GMT_LEN64, char);
		}

		if (out3[0] == '\0') continue;	/* No intervals */
		GMT->current.map.frame.set = true;	/* Got here so we are setting intervals */

		/* Parse the annotation/tick info string */
		if (out3[0] == 'c')
			error += gmtinit_decode_tinfo (GMT, i, 'c', out3, &GMT->current.map.frame.axis[i]);
		else {	/* Parse from back for 'a', 'f', 'g' chunks */
			for (k = (int)strlen (out3) - 1; k >= 0; k--) {
				if (out3[k] == 'a' || out3[k] == 'f' || out3[k] == 'g') {
					error += gmtinit_decode_tinfo (GMT, i, out3[k], &out3[k+1], &GMT->current.map.frame.axis[i]);
					out3[k] = '\0';	/* Done with this chunk; replace with terminator */
				}
				else if (k == 0)	/* If no [a|f|g] then it is implicitly 'a' */
					error += gmtinit_decode_tinfo (GMT, i, 'a', out3, &GMT->current.map.frame.axis[i]);
			}
		}

		/* Make sure we have ticks to match annotation stride */
		A = &GMT->current.map.frame.axis[i];
		if (A->item[GMT_ANNOT_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			gmt_M_memcpy (&A->item[GMT_TICK_UPPER], &A->item[GMT_ANNOT_UPPER], 1, struct GMT_PLOT_AXIS_ITEM);
		if (A->item[GMT_ANNOT_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			gmt_M_memcpy (&A->item[GMT_TICK_LOWER], &A->item[GMT_ANNOT_LOWER], 1, struct GMT_PLOT_AXIS_ITEM);
		/* Note that item[].type will say 'a', 'A', 'i' or 'I' in these cases, so we know when minor ticks were not set */
	}

	/* Check if we asked for linear projections of geographic coordinates and did not specify a unit - if so set degree symbol as unit */
	if (GMT->current.proj.projection_GMT == GMT_LINEAR && GMT->current.setting.map_degree_symbol != gmt_none) {
		for (i = 0; i < 2; i++) {
			if (gmt_M_type (GMT, GMT_IN, i) & GMT_IS_GEO && GMT->current.map.frame.axis[i].unit[0] == 0) {
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
void gmt_handle5_plussign (struct GMT_CTRL *GMT, char *in, char *mods, unsigned way) {
	/* Way = 0: replace any +<letter> with <letter> NOT in <mods> with ASCII 1<letter>
	 *	    We only skip +<letter> the first time it is found (if in <mods>).
	 * Way = 1: Replace ASCII 1 with + */
	gmt_M_unused(GMT);
	if (in == NULL || in[0] == '\0') return;	/* No string to check */
	if (way == 0) {	/* Replace any +<letter> with <letter> NOT in <mods> with ASCII 1<letter> */
		size_t n = strlen (mods);
		char *c = in, *p = NULL;
		unsigned int *used = gmt_M_memory (GMT, NULL, n, unsigned int);
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
			p = NULL;
			if (c[1] && (p = strchr (mods, c[1]))) {	/* Any of ours */
				unsigned int k = (p - mods);
				if (used[k]) p = NULL;
				else used[k]++;
			}
			if (!p) /* Not one of the +<mods> cases (or already fixed) so we can replace the + by 1 */
				*c = 1;
			++c;
		}
		gmt_M_free (GMT, used);
	}
	else /* way != 0: Replace single ASCII 1 with + */
		gmt_strrepc (in, 1, '+');
}

/*! Scans the WESNZ[1234]wesnz[1234]lrbtu flags and sets the side/drawbox parameters
 * and returns the length of the remaining string.
 */
GMT_LOCAL int gmtinit_decode5_wesnz (struct GMT_CTRL *GMT, const char *in, bool check) {

	unsigned int k, error = 0, f_side[5] = {0, 0, 0, 0, 0}, z_axis[4] = {0, 0, 0, 0};
	bool s_given = false;
	if (check) {	/* true if coming via -B, false if parsing gmt.conf */
		GMT->current.map.frame.set_frame[GMT_PRIMARY]++, GMT->current.map.frame.set_frame[GMT_SECONDARY]++;
		if (GMT->current.map.frame.set_frame[GMT_PRIMARY] > 1 || GMT->current.map.frame.set_frame[GMT_SECONDARY] > 1) {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Error -B: <WESNZ-framesettings> given more than once!\n");
			return (1);
		}
	}
	for (k = 0; in[k]; k++) {
		switch (in[k]) {
			/* Draw, Annotate, and Tick */
			case 'W': f_side[W_SIDE] |= GMT_AXIS_ALL; s_given = true; break;
			case 'E': f_side[E_SIDE] |= GMT_AXIS_ALL; s_given = true; break;
			case 'S': f_side[S_SIDE] |= GMT_AXIS_ALL; s_given = true; break;
			case 'N': f_side[N_SIDE] |= GMT_AXIS_ALL; s_given = true; break;
			case 'Z': f_side[Z_SIDE] |= GMT_AXIS_ALL; s_given = true; break;
			/* Just Draw and Tick */
			case 'w': f_side[W_SIDE] |= GMT_AXIS_BARB; s_given = true; break;
			case 'e': f_side[E_SIDE] |= GMT_AXIS_BARB; s_given = true; break;
			case 's': f_side[S_SIDE] |= GMT_AXIS_BARB; s_given = true; break;
			case 'n': f_side[N_SIDE] |= GMT_AXIS_BARB; s_given = true; break;
			case 'z': f_side[Z_SIDE] |= GMT_AXIS_BARB; s_given = true; break;
			/* Just Draw */
			case 'l': f_side[W_SIDE] |= GMT_AXIS_DRAW; s_given = true; break;
			case 'r': f_side[E_SIDE] |= GMT_AXIS_DRAW; s_given = true; break;
			case 'b': f_side[S_SIDE] |= GMT_AXIS_DRAW; s_given = true; break;
			case 't': f_side[N_SIDE] |= GMT_AXIS_DRAW; s_given = true; break;
			case 'u': f_side[Z_SIDE] |= GMT_AXIS_DRAW; s_given = true; break;
			/* Draw 3-D box */
			case '+':
				if (in[k+1] == 'b')	/* Got +b appended to MAP_FRAME_AXES, possibly */
					GMT->current.map.frame.draw_box = true;
				else if (in[k+1] == 'n')	/* Got +n appended to MAP_FRAME_AXES, means no frame nor annotations desired */
					GMT->current.map.frame.no_frame = true;
				else if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Modifier + in MAP_FRAME_AXES is deprecated; use +b instead.\n");
					GMT->current.map.frame.draw_box = true;
				}
				else {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Modifier + in MAP_FRAME_AXES not recognized.\n");
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
		gmt_M_memcpy (GMT->current.map.frame.side, f_side, 5, unsigned int);	/* Overwrite the GMT defaults */
		GMT->current.map.frame.no_frame = false;
		GMT->current.map.frame.draw = true;
		if (check && f_side[Z_SIDE]) GMT->current.map.frame.drawz = true;
	}
	if (GMT->current.map.frame.no_frame) gmt_M_memset (GMT->current.map.frame.side, 5, unsigned int);	/* Set all to nothing */
	if (z_axis[0] || z_axis[1] || z_axis[2] || z_axis[3]) gmt_M_memcpy (GMT->current.map.frame.z_axis, z_axis, 4, unsigned int);	/* Overwrite the GMT defaults */
	return (error);
}

/*! . */
GMT_LOCAL int gmtinit_parse5_B_frame_setting (struct GMT_CTRL *GMT, char *in) {
	unsigned int pos = 0, k, error = 0, is_frame = 0;
	char p[GMT_BUFSIZ] = {""}, text[GMT_BUFSIZ] = {""}, *mod = NULL;
	double pole[2];

	/* Parsing of -B<framesettings> */

	/* First determine that the given -B<in> string is indeed the framesetting option.  If not return -1 */

	if (strchr ("pxy", in[0])) return (-1);	/* -B[p[xy] is definitively not the frame settings (-Bz is tricker; see below) */
	if (strstr (in, "+b")) is_frame++;	/* Found a +b so likely frame */
	if (strstr (in, "+g")) is_frame++;	/* Found a +g so likely frame */
	if (strstr (in, "+n")) is_frame++;	/* Found a +n so likely frame */
	if (strstr (in, "+o")) is_frame++;	/* Found a +o so likely frame */
	if (strstr (in, "+t")) is_frame++;	/* Found a +t so likely frame */
	if (in[0] != 'z' && strchr ("WESNZwenzlrbtu", in[0])) is_frame++;	/* Found one of the side specifiers so likely frame (check on z since -Bzaf could trick it) */
	if (in[0] == 's' && (in[1] == 0 || strchr ("WESNZwenzlrbtu", in[1]) != NULL)) is_frame++;	/* Found -Bs (just draw south axis) or -Bs<another axis flag> */
	if (in[0] == 'z' && !is_frame && (in[1] == 0 || strchr ("WESNwenlrbtu", in[1]) != NULL)) is_frame++;	/* Found -Bz in frame context, e.g. -Bzwn */
	if (is_frame == 0) return (-1);		/* No, nothing matched */

	/* OK, here we are pretty sure this is a frame -B statement */

	strncpy (text, in, GMT_BUFSIZ-1);
	gmt_handle5_plussign (GMT, text, "bgnot", 0);	/* Temporarily change double plus-signs to double ASCII 1 to avoid +<modifier> angst */
	GMT->current.map.frame.header[0] = '\0';

	if ((mod = strchr (text, '+'))) {	/* Find start of modifiers, if any */
		while ((gmt_strtok (mod, "+", &pos, p))) {	/* Parse any +<modifier> statements */
			switch (p[0]) {
				case 'b':	/* Activate 3-D box and x-z, y-z gridlines (if selected) */
					GMT->current.map.frame.draw_box = true;
					break;
				case 'g':	/* Paint the basemap interior */
					if (p[1] == 0 || gmt_getfill (GMT, &p[1], &GMT->current.map.frame.fill)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +g<fill> argument %s\n", &p[1]);
						error++;
					}
					GMT->current.map.frame.paint = true;
					break;
				case 'n':	/* Turn off frame entirely; this is also done in gmtinit_decode5_wesnz */
					GMT->current.map.frame.no_frame = true;
					break;
				case 'o':	/* Specify pole for oblique gridlines */
					if (GMT->current.proj.projection_GMT == GMT_OBLIQUE_MERC) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL,
							"Syntax error -B option: Cannot specify oblique gridlines for the oblique Mercator projection\n");
						error++;
					}
					else if (!p[1] || (k = GMT_Get_Values (GMT->parent, &p[1], pole, 2)) != 2) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +o[<plon>/<plat>] argument %s\n", &p[1]);
						error++;
					}
					else {	/* Successful parsing of pole */
						GMT->current.map.frame.obl_grid = true;
						gmtlib_set_oblique_pole_and_origin (GMT, pole[GMT_X], pole[GMT_Y], 0.0, 0.0);
					}
					break;
				case 't':
					if (p[1]) {	/* Actual title was appended */
						strncpy (GMT->current.map.frame.header, &p[1], GMT_LEN256-1);
						gmt_handle5_plussign (GMT, GMT->current.map.frame.header, NULL, 1);	/* Recover any non-modifier plus signs */
						gmtlib_enforce_rgb_triplets (GMT, GMT->current.map.frame.header, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
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
	error += gmtinit_decode5_wesnz (GMT, text, true);

	return (error);
}

void gmt_init_B (struct GMT_CTRL *GMT) {
	unsigned int no, k;
	for (no = 0; no < 3; no++) {
		gmt_M_memset (&GMT->current.map.frame.axis[no], 1, struct GMT_PLOT_AXIS);
		GMT->current.map.frame.axis[no].id = no;
		for (k = 0; k < 6; k++) GMT->current.map.frame.axis[no].item[k].parent = no;
		if (GMT->current.proj.xyz_projection[no] == GMT_TIME) GMT->current.map.frame.axis[no].type = GMT_TIME;
	}
	GMT->common.B.string[0][0] = GMT->common.B.string[1][0] = '\0';
	GMT->current.map.frame.init = true;
	GMT->current.map.frame.draw = false;
	GMT->current.map.frame.set_frame[GMT_PRIMARY] = GMT->current.map.frame.set_frame[GMT_SECONDARY] = 0;
}

/*! . */
GMT_LOCAL int gmtinit_parse5_B_option (struct GMT_CTRL *GMT, char *in) {
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
	 * 	<intervals>[+a<angle>|n|p][+L|l<label>][+S|s<altlabel>][+p<prefix>][+u<unit>]
	 *   and each <intervals> is a concatenation of one or more [t][value][<unit>]
	 *    		+a<angle> sets a fixed annotation angle with respect to axis (Cartesian only), n or p for normal or parallel
	 *    		+l<label> as labels for the respective axes [none]. Use +L for only horizontal labels
	 *    		+s<altlabel> as alternate label for the far axis [same as <label>]. Use +S for only horizontal labels
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

	char string[GMT_BUFSIZ] = {""}, orig_string[GMT_BUFSIZ] = {""}, text[GMT_BUFSIZ] = {""}, *mod = NULL, *the_axes = "xyz";
	struct GMT_PLOT_AXIS *A = NULL;
	unsigned int no;
	int k, error = 0;
	bool side[3] = {false, false, false}, implicit = false;

	if (!in || !in[0]) return (GMT_PARSE_ERROR);	/* -B requires an argument */

	if (!GMT->current.map.frame.init)	/* First time we initialize stuff */
		gmt_init_B (GMT);

	if ((error = gmtinit_parse5_B_frame_setting (GMT, in)) >= 0) return (error);	/* Parsed the -B frame settings separately */
	error = 0;	/* Reset since otherwise it is -1 */

	/* Below here are the axis settings only -B[p|s][x|y|z] */
	switch (in[0]) {
		case 's': GMT->current.map.frame.primary = false; k = 1; break;
		case 'p': GMT->current.map.frame.primary = true;  k = 1; break;
		default:  GMT->current.map.frame.primary = true;  k = 0; break;
	}
	no = (GMT->current.map.frame.primary) ? 0 : 1;
	if (GMT->common.B.string[no][0]) {	/* Append this option */
		char group_sep[2] = {" "};
		group_sep[0] = GMT_ASCII_GS;
		strcat (GMT->common.B.string[no], group_sep);
		strncat (GMT->common.B.string[no], in, GMT_LEN256-1);
	}
	else
		strncpy (GMT->common.B.string[no], in, GMT_LEN256-1);	/* Keep a copy of the actual option(s) */

	/* Set which axes this option applies to */
	while (in[k] && strchr (the_axes, in[k])) {	/* As long as there are leading x,y,z axes specifiers */
		switch (in[k]) {	/* We specified a named axis */
			case 'x': side[GMT_X] = true; break;
			case 'y': side[GMT_Y] = true; break;
			case 'z': side[GMT_Z] = true; break;
		}
		k++;
	}
	if (!(side[GMT_X] || side[GMT_Y] || side[GMT_Z])) GMT->current.map.frame.set_both = side[GMT_X] = side[GMT_Y] = implicit = true;	/* If no axis were named we default to both x and y */

	strncpy (text, &in[k], GMT_BUFSIZ-1);	/* Make a copy of the input, starting after the leading -B[p|s][xyz] indicators */
	gmt_handle5_plussign (GMT, text, "aLlpsSu", 0);	/* Temporarily change any +<letter> except +L|l, +p, +S|s, +u to ASCII 1 to avoid interference with +modifiers */
	k = 0;					/* Start at beginning of text and look for first occurrence of +L|l, +p, +S|s or +u */
	while (text[k] && !(text[k] == '+' && strchr ("aLlpSsu", text[k+1]))) k++;
	gmt_M_memset (orig_string, GMT_BUFSIZ, char);
	strncpy (orig_string, text, k);		/* orig_string now has the interval information */
	gmt_handle5_plussign (GMT, orig_string, NULL, 1);	/* Recover any non-modifier plus signs */
	if (text[k]) mod = &text[k];		/* mod points to the start of the modifier information in text*/
	for (no = 0; no < 3; no++) {		/* Process each axis separately */
		if (!side[no]) continue;	/* Except we did not specify this axis */
		if (no == GMT_Z) GMT->current.map.frame.drawz = true;
		if (!text[0]) continue;	 	/* Skip any empty format string */
		if (text[0] == '0' && !text[1]) {	 /* Understand format '0' to mean "no annotation, ticks, or gridlines" */
			GMT->current.map.frame.draw = GMT->current.map.frame.drawz = true;	/* But we do wish to draw the frame */
			continue;
		}

		if (mod) {	/* Process the given axis modifiers */
			unsigned int pos = 0;
			char p[GMT_BUFSIZ];
			while ((gmt_strtok (mod, "+", &pos, p))) {	/* Parse any +<modifier> statements */
				switch (p[0]) {
					case 'a':	/* Set annotation angle */
						if (gmt_M_is_geographic (GMT, GMT_IN)) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Cannot use +a for geographic basemaps\n");
							error++;
						}
						else if (no == 0) {	/* Variable angles are only allowed for the x-axis */
							if (p[1] == 'n')	/* +an is short for +a90 or normal to x-axis */
								GMT->current.map.frame.axis[no].angle = 90.0;
							else if (p[1] == 'p')	/* +ap is short for +a0 or parallel to x-axis */
								GMT->current.map.frame.axis[no].angle = 0.0;
							else	/* Assume a variable angle */
								GMT->current.map.frame.axis[no].angle = atof (&p[1]);
							if (GMT->current.map.frame.axis[no].angle < -90.0 || GMT->current.map.frame.axis[no].angle > 90.0) {
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: +a<angle> must be in the -90 to +90 range\n");
								error++;
							}
							else
								GMT->current.map.frame.axis[no].use_angle = true;
						}
						else if (no == 1) {	/* Only +an|p is allowed for y-axis */
							GMT->current.map.frame.axis[no].use_angle = true;
							if (p[1] == 'n')	/* +an is code for normal to y-axis;*/
								GMT->current.map.frame.axis[no].angle = 0.0;
							else if (p[1] == 'p')	/* +ap is code for normal to y-axis; this triggers ortho=false later */
								GMT->current.map.frame.axis[no].angle = 90.0;
							else if (!implicit) {	/* Means user gave -By...+a<angle> which is no good */
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Only modifiers +an|p is allowed for the y-axis\n");
								error++;
							}
							else	/* Probably did -Baf+a30 and only meant that to apply to the x-axis */
								GMT->current.map.frame.axis[no].use_angle = false;
						}
						else if (!implicit)
							GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "-B option: The +a modifier only applies to the x and y axes; selection for %c-axis ignored\n", the_axes[no]);
						break;
					case 'L':	/* Force horizontal axis label */
						GMT->current.map.frame.axis[no].label_mode = 1;
						/* Fall through on purpose to 'l' */
					case 'l':	/* Axis label */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No axis label given after +l|L\n");
							error++;
						}
						else {
							strncpy (GMT->current.map.frame.axis[no].label, &p[1], GMT_LEN256-1);
							gmt_handle5_plussign (GMT, GMT->current.map.frame.axis[no].label, NULL, 1);	/* Recover any non-modifier plus signs */
							gmtlib_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].label, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
						}
						break;
					case 'p':	/* Annotation prefix */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No annotation prefix given after +p\n");
							error++;
						}
						else {
							strncpy (GMT->current.map.frame.axis[no].prefix, &p[1], GMT_LEN64-1);
							gmt_handle5_plussign (GMT, GMT->current.map.frame.axis[no].prefix, NULL, 1);	/* Recover any non-modifier plus signs */
							gmtlib_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].prefix, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
						}
						break;
					case 'S':	/* Force horizontal secondary axis label */
						GMT->current.map.frame.axis[no].label_mode = 1;
						/* Fall through on purpose */
					case 's':	/* Axis secondary label (optional) */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No secondary axis label given after +s|S\n");
							error++;
						}
						else {
							strncpy (GMT->current.map.frame.axis[no].secondary_label, &p[1], GMT_LEN256-1);
							gmt_handle5_plussign (GMT, GMT->current.map.frame.axis[no].secondary_label, NULL, 1);	/* Recover any non-modifier plus signs */
							gmtlib_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].secondary_label, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
						}
						break;
					case 'u':	/* Annotation unit */
						if (p[1] == 0) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: No annotation suffix given after +u\n");
							error++;
						}
						else {
							strncpy (GMT->current.map.frame.axis[no].unit, &p[1], GMT_LEN64-1);
							gmt_handle5_plussign (GMT, GMT->current.map.frame.axis[no].unit, NULL, 1);	/* Recover any non-modifier plus signs */
							gmtlib_enforce_rgb_triplets (GMT, GMT->current.map.frame.axis[no].unit, GMT_LEN256);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
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
		if (strstr (orig_string, "pi")) GMT->current.map.frame.axis[no].substitute_pi = true;	/* Use pi in formatting labels */

		gmt_M_memset (string, GMT_BUFSIZ, char);
		strcpy (string, orig_string);	/* Make a copy of string as it gets messed with below */
		if (string[0] == 'c')		/* Special custom annotation information given via file */
			error += gmtinit_decode_tinfo (GMT, no, 'c', string, &GMT->current.map.frame.axis[no]);
		else {				/* Parse from back of string for 'a', 'f', 'g' chunks */
			for (k = (int)strlen (string) - 1; k >= 0; k--) {
				if (string[k] == 'a' || string[k] == 'f' || string[k] == 'g') {
					error += gmtinit_decode_tinfo (GMT, no, string[k], &string[k+1], &GMT->current.map.frame.axis[no]);
					string[k] = '\0';	/* Done with this chunk; replace with terminator */
				}
				else if (k == 0)		/* If no [a|f|g] given then it is implicitly 'a' */
					error += gmtinit_decode_tinfo (GMT, no, 'a', string, &GMT->current.map.frame.axis[no]);
			}
		}

		/* Make sure we have ticks to match specified annotation stride */
		A = &GMT->current.map.frame.axis[no];
		if (A->item[GMT_ANNOT_UPPER].active && !A->item[GMT_TICK_UPPER].active)	/* Set frame ticks = annot stride */
			gmt_M_memcpy (&A->item[GMT_TICK_UPPER], &A->item[GMT_ANNOT_UPPER], 1, struct GMT_PLOT_AXIS_ITEM);
		if (A->item[GMT_ANNOT_LOWER].active && !A->item[GMT_TICK_LOWER].active)	/* Set frame ticks = annot stride */
			gmt_M_memcpy (&A->item[GMT_TICK_LOWER], &A->item[GMT_ANNOT_LOWER], 1, struct GMT_PLOT_AXIS_ITEM);
		/* Note that item[].type will say 'a', 'A', 'i' or 'I' in these cases, so we know when minor ticks were not set */
	}

	/* Check if we asked for linear projections of geographic coordinates and did not specify a unit (suffix) - if so set degree symbol as unit */
	if (GMT->current.proj.projection_GMT == GMT_LINEAR && GMT->current.setting.map_degree_symbol != gmt_none) {
		for (no = 0; no < 2; no++) {
			if (gmt_M_type (GMT, GMT_IN, no) & GMT_IS_GEO && GMT->current.map.frame.axis[no].unit[0] == 0) {
				GMT->current.map.frame.axis[no].unit[0] = (char)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol];
				GMT->current.map.frame.axis[no].unit[1] = '\0';
			}
		}
	}

	return (error);
}

/*! . */
int gmtlib_parse_B_option (struct GMT_CTRL *GMT, char *in) {
	int error = 0;
	if (GMT->common.B.mode == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: Calling gmtlib_parse_B_option before gmt_check_b_options somehow\n");
		error = 1;
	}
	else if (GMT->common.B.mode == -1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Mixing of GMT 4 and 5 level syntax is not possible\n");
		error = 1;
	}
	else if (GMT->common.B.mode == 4) {	/* Got GMT 4 syntax */
		if (gmt_M_compat_check (GMT, 4))
			error = gmtinit_parse4_B_option (GMT, in);
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -B option: Cannot use GMT 4 syntax except in GMT classic with compatibility mode\n");
			error = 1;
		}
	}
	else	/* Clean GMT5 syntax */
		error = gmtinit_parse5_B_option (GMT, in);

	return (error);
}

/*! . */
GMT_LOCAL int gmtinit_project_type (char *args, int *pos, bool *width_given) {
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

	if ((*pos = (int)gmt_strlcmp ("aea/"      , args))) return (GMT_ALBERS);
	if ((*pos = (int)gmt_strlcmp ("aeqd/"     , args))) return (GMT_AZ_EQDIST);
	if ((*pos = (int)gmt_strlcmp ("cyl_stere/", args))) return (GMT_CYL_STEREO);
	if ((*pos = (int)gmt_strlcmp ("cass/"     , args))) return (GMT_CASSINI);
	if ((*pos = (int)gmt_strlcmp ("cea/"      , args))) return (GMT_CYL_EQ);
	if ((*pos = (int)gmt_strlcmp ("eck4/"     , args))) return (GMT_ECKERT4);
	if ((*pos = (int)gmt_strlcmp ("eck6/"     , args))) return (GMT_ECKERT6);
	if ((*pos = (int)gmt_strlcmp ("eqc/"      , args))) return (GMT_CYL_EQDIST);
	if ((*pos = (int)gmt_strlcmp ("eqdc/"     , args))) return (GMT_ECONIC);
	if ((*pos = (int)gmt_strlcmp ("gnom/"     , args))) return (GMT_GNOMONIC);
	if ((*pos = (int)gmt_strlcmp ("hammer/"   , args))) return (GMT_HAMMER);
	if ((*pos = (int)gmt_strlcmp ("laea/"     , args))) return (GMT_LAMB_AZ_EQ);
	if ((*pos = (int)gmt_strlcmp ("lcc/"      , args))) return (GMT_LAMBERT);
	if ((*pos = (int)gmt_strlcmp ("merc/"     , args))) return (GMT_MERCATOR);
	if ((*pos = (int)gmt_strlcmp ("mill/"     , args))) return (GMT_MILLER);
	if ((*pos = (int)gmt_strlcmp ("moll/"     , args))) return (GMT_MOLLWEIDE);
	if ((*pos = (int)gmt_strlcmp ("nsper/"    , args))) return (GMT_GENPER);
	if ((*pos = (int)gmt_strlcmp ("omerc/"    , args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = (int)gmt_strlcmp ("omercp/"   , args))) return (GMT_OBLIQUE_MERC_POLE);
	if ((*pos = (int)gmt_strlcmp ("ortho/"    , args))) return (GMT_ORTHO);
	if ((*pos = (int)gmt_strlcmp ("polar/"    , args))) return (GMT_POLAR);
	if ((*pos = (int)gmt_strlcmp ("poly/"     , args))) return (GMT_POLYCONIC);
	if ((*pos = (int)gmt_strlcmp ("robin/"    , args))) return (GMT_ROBINSON);
	if ((*pos = (int)gmt_strlcmp ("sinu/"     , args))) return (GMT_SINUSOIDAL);
	if ((*pos = (int)gmt_strlcmp ("stere/"    , args))) return (GMT_STEREO);
	if ((*pos = (int)gmt_strlcmp ("tmerc/"    , args))) return (GMT_TM);
	if ((*pos = (int)gmt_strlcmp ("utm/"      , args))) return (GMT_UTM);
	if ((*pos = (int)gmt_strlcmp ("vandg/"    , args))) return (GMT_VANGRINTEN);
	if ((*pos = (int)gmt_strlcmp ("wintri/"   , args))) return (GMT_WINKEL);
	if ((*pos = (int)gmt_strlcmp ("xy/"       , args))) return (GMT_LINEAR);
	if ((*pos = (int)gmt_strlcmp ("z/"        , args))) return (GMT_ZAXIS);

	/* These older codes (up to GMT 4.2.1) took 2 characters */

	if ((*pos = (int)gmt_strlcmp ("kf", args))) return (GMT_ECKERT4);
	if ((*pos = (int)gmt_strlcmp ("ks", args))) return (GMT_ECKERT6);
	if ((*pos = (int)gmt_strlcmp ("oa", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = (int)gmt_strlcmp ("ob", args))) return (GMT_OBLIQUE_MERC);
	if ((*pos = (int)gmt_strlcmp ("oc", args))) return (GMT_OBLIQUE_MERC_POLE);

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
GMT_LOCAL int gmtinit_scale_or_width (struct GMT_CTRL *GMT, char *scale_or_width, double *value) {
	/* Scans character that may contain a scale (1:xxxx or units per degree) or a width.
	   Return 1 upon error. Here we want to make an exception for users giving a scale
	   of 1 in grdproject and mapproject as it is most likely meant to be 1:1. */
	int n, answer;
	if (isalpha (scale_or_width[0])) return (1);	/* Neither scale nor width can start with a letter - probably junk input */
	answer = strncmp (scale_or_width, "1:", 2U);	/* 0 if scale given as 1:xxxx */
	GMT->current.proj.units_pr_degree = (answer != 0);
	if (GMT->current.proj.units_pr_degree) {	/* Check if we got "1" and this is grd|map-project */
		size_t k = strlen (scale_or_width);
		if (k == 1 && scale_or_width[0] == '1' && gmt_M_is_grdmapproject (GMT)) {	/* OK, pretend we got 1:1 */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning -J option: Your scale of 1 was interpreted to mean 1:1 since no plotting is involved.\n");
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "If a scale of 1 was intended, please append a unit from %s.\n", GMT_DIM_UNITS_DISPLAY);
			gmtinit_scale_or_width (GMT, strcat(scale_or_width,":1"), value);
			return (GMT_NOERROR);
		}

		*value = gmt_M_to_inch (GMT, scale_or_width);
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
	if (gmt_M_is_zero (*value)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Your scale or width (%s) resulted in a zero value.\n", scale_or_width);
		return (1);
	}
	else if (gmt_M_is_geographic (GMT, GMT_IN) && (*value) < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Geographic scale (%s) cannot be negative.\n", scale_or_width);
		return (1);
	}
	GMT->current.proj.pars[15] = *value;	/* Store the scale here so we always know where to find it */
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtinit_autoscale (char *arg) {
	/* Check for 0[d], -0[d], +0[d] */
	if (!strcmp (arg, "-0") || !strcmp (arg, "-0d")) return -1;	/* Want same as other axis but negative direction */
	if (!strcmp (arg,  "0") || !strcmp (arg,  "0d") || !strcmp (arg, "+0") || !strcmp (arg, "+0d")) return +1;	/* Same as other axis and same direction */
	return 0;	/* No */
}

/*! . */
GMT_LOCAL bool gmtinit_parse_J_option (struct GMT_CTRL *GMT, char *args) {
	/* gmtinit_parse_J_option scans the arguments given and extracts the parameters needed
	 * for the specified map projection. These parameters are passed through the
	 * GMT->current.proj structure.  The function returns true if an error is encountered.
	 */

	int i, j, k = 0, m, n, nlen, slash, l_pos[3], p_pos[3], t_pos[3], d_pos[3], id, project;
	int n_slashes = 0, last_pos, error = 0;
	unsigned int mod_flag = 0;
	bool width_given = false;
	double c, az, GMT_units[3] = {0.01, 0.0254, 1.0};      /* No of meters in a cm, inch, m */
	char mod, args_cp[GMT_BUFSIZ] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	char txt_d[GMT_LEN256] = {""}, txt_e[GMT_LEN256] = {""}, last_char;
	char txt_arr[11][GMT_LEN256];

	if (args == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -J option: No argument for parsing\n");
		return (true);
	}
	gmt_M_memset (l_pos, 3, int);	gmt_M_memset (p_pos, 3, int);
	gmt_M_memset (t_pos, 3, int);	gmt_M_memset (d_pos, 3, int);
	if (!GMT->common.J.active)	/* Down want to clobber this during -Jz/Z after the horizontal part has been set */
		GMT->current.proj.lon0 = GMT->current.proj.lat0 = GMT->session.d_NaN;	/* Projection center, to be set via -J */

	project = gmtinit_project_type (args, &i, &width_given);
	if (project == GMT_NO_PROJ) return (true);	/* No valid projection specified */

	if (project == GMT_ZAXIS)
		strncpy (GMT->common.J.zstring, args, GMT_LEN128-1);	/* Verbatim copy of -Jz|Z */
	else
		strncpy (GMT->common.J.string, args, GMT_LEN128-1);	/* Verbatim copy or map -J */
	args += i;	/* Skip to first argument */
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

	n_slashes = gmtlib_count_char (GMT, args, '/');	/* Count slashes to distinguis args */

	/* Differentiate between general perspective and orthographic projection based on number of slashes */
	if (project == GMT_GENPER || project == GMT_ORTHO) {
		if (n_slashes >= 5)
			project = GMT_GENPER;
		else
			project = GMT_ORTHO;
	}

	if (project != GMT_ZAXIS) {
		/* Check to see if scale is specified in 1:xxxx */
		for (j = (int)strlen (args), k = -1; j > 0 && k < 0 && args[j] != '/'; j--)
			if (args[j] == ':') k = j + 1;
		GMT->current.proj.units_pr_degree = (k == -1) ? true : false;
		gmt_set_cartesian (GMT, GMT_OUT);	/* This may be overridden by mapproject -I */
		if (project != GMT_LINEAR)
			gmt_set_geographic (GMT, GMT_IN);
		GMT->current.proj.gave_map_width = mod_flag;
	}

	GMT->current.proj.unit = GMT_units[GMT_INCH];	/* No of meters in an inch */
	n = 0;	/* Initialize with no fields found */
	switch (project) {
		case GMT_LINEAR:	/* Linear x/y scaling */
			gmt_set_cartesian (GMT, GMT_IN);	/* This will be overridden below if -Jx or -Jp, for instance */
			GMT->current.proj.compute_scale[GMT_X] = GMT->current.proj.compute_scale[GMT_Y] = width_given;

			/* Default is not involving geographical coordinates */
			gmt_set_column (GMT, GMT_IO, GMT_X, GMT_IS_UNKNOWN);
			gmt_set_column (GMT, GMT_IO, GMT_Y, GMT_IS_UNKNOWN);

			error += (n_slashes > 1) ? 1 : 0;

			/* Find occurrences of /, l, p, t, or d */
			for (j = 0, slash = 0; args[j] && slash == 0; j++)
				if (args[j] == '/') slash = j;
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

			strncpy (args_cp, args, GMT_BUFSIZ-1);	/* Since gmt_M_to_inch modifies the string */
			if (slash) args_cp[slash] = 0;	/* Chop off y part */
			if (gmtinit_autoscale (args_cp) == +1)
				GMT->current.proj.autoscl[GMT_X] = 1;	/* Want same scale as for y; compute width from x-range */
			else if (gmtinit_autoscale (args_cp) == -1)
				GMT->current.proj.autoscl[GMT_X] = -1;	/* Want same scale as for y but reverse direction; compute width from x-range */
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
				GMT->current.proj.pars[0] = gmt_M_to_inch (GMT, args_cp);	/* x-scale */
			GMT->current.proj.xyz_projection[GMT_X] = GMT_LINEAR;
			if (l_pos[GMT_X] > 0)
				GMT->current.proj.xyz_projection[GMT_X] = GMT_LOG10;
			else if (p_pos[GMT_X] > 0) {
				GMT->current.proj.xyz_projection[GMT_X] = GMT_POW;
				GMT->current.proj.pars[2] = atof (&args[p_pos[GMT_X]+1]);	/* pow to raise x */
			}
			else if (t_pos[GMT_X] > 0) {	/* Add option to append time_systems or epoch/unit later */
				GMT->current.proj.xyz_projection[GMT_X] = GMT_TIME;
				gmt_set_column (GMT, GMT_IN, GMT_X, (args[t_pos[GMT_X]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME);
			}

			if (d_pos[GMT_X] > 0) gmt_set_column (GMT, GMT_IN, GMT_X, GMT_IS_LON);

			if (slash) {	/* Separate y-scaling desired */
				strncpy (args_cp, &args[slash+1], GMT_BUFSIZ-1);	/* Since gmt_M_to_inch modifies the string */
				if (gmtinit_autoscale (args_cp) == +1)
					GMT->current.proj.autoscl[GMT_Y] = 1;	/* Want same scale as for x; compute height from y-range */
				else if (gmtinit_autoscale (args_cp) == -1)
					GMT->current.proj.autoscl[GMT_Y] = -1;	/* Want same scale as for x but reverse direction; compute height from y-range */
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
					GMT->current.proj.pars[1] = gmt_M_to_inch (GMT, args_cp);	/* y-scale */

				GMT->current.proj.xyz_projection[GMT_Y] = GMT_LINEAR;
				if (l_pos[GMT_Y] > 0)
					GMT->current.proj.xyz_projection[GMT_Y] = GMT_LOG10;
				else if (p_pos[GMT_Y] > 0) {
					GMT->current.proj.xyz_projection[GMT_Y] = GMT_POW;
					GMT->current.proj.pars[3] = atof (&args[p_pos[GMT_Y]+1]);	/* pow to raise y */
				}
				else if (t_pos[GMT_Y] > 0) {	/* Add option to append time_systems or epoch/unit later */
					GMT->current.proj.xyz_projection[GMT_Y] = GMT_TIME;
					gmt_set_column (GMT, GMT_IN, GMT_Y, (args[t_pos[GMT_Y]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME);
				}
				if (d_pos[GMT_Y] > 0) gmt_set_column (GMT, GMT_IN, GMT_Y, GMT_IS_LAT);
			}
			else {	/* Just copy x parameters */
				GMT->current.proj.xyz_projection[GMT_Y] = GMT->current.proj.xyz_projection[GMT_X];
				GMT->current.proj.pars[1] = GMT->current.proj.pars[0];
				GMT->current.proj.pars[3] = GMT->current.proj.pars[2];
				/* Assume -JX<width>[unit]d means a linear geographic plot so x = lon and y = lat */
				if (gmt_M_type (GMT, GMT_IN, GMT_X) & GMT_IS_LON) gmt_set_column (GMT, GMT_IN, GMT_Y, GMT_IS_LAT);
			}
			/* Not both sizes can be zero, but if one is, we will adjust to the scale of the other */
			if (GMT->current.proj.pars[GMT_X] == 0.0 && GMT->current.proj.pars[GMT_Y] == 0.0) error++;
			break;

		case GMT_ZAXIS:	/* 3D plot */
			GMT->current.proj.compute_scale[GMT_Z] = width_given;
			error += (n_slashes > 0) ? 1 : 0;
			gmt_set_column (GMT, GMT_IN, GMT_Z, GMT_IS_UNKNOWN);

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

			strncpy (args_cp, args, GMT_BUFSIZ-1);	/* Since gmt_M_to_inch modifies the string */
			if ((i = MAX (l_pos[GMT_Z], p_pos[GMT_Z])) > 0)
				args_cp[i] = 0;
			else if (t_pos[GMT_Z] > 0)
				args_cp[t_pos[GMT_Z]] = 0;
			GMT->current.proj.z_pars[0] = gmt_M_to_inch (GMT, args_cp);	/* z-scale */

			GMT->current.proj.xyz_projection[GMT_Z] = GMT_LINEAR;
			if (l_pos[GMT_Z] > 0)
				GMT->current.proj.xyz_projection[GMT_Z] = GMT_LOG10;
			else if (p_pos[GMT_Z] > 0) {
				GMT->current.proj.xyz_projection[GMT_Z] = GMT_POW;
				GMT->current.proj.z_pars[1] = atof (&args[p_pos[GMT_Z]+1]);	/* pow to raise z */
			}
			else if (t_pos[GMT_Z] > 0) {
				GMT->current.proj.xyz_projection[GMT_Z] = GMT_TIME;
				gmt_set_column (GMT, GMT_IN, GMT_Z, (args[t_pos[GMT_Z]] == 'T') ?  GMT_IS_ABSTIME : GMT_IS_RELTIME);
			}
			if (GMT->current.proj.z_pars[0] == 0.0) error++;
			GMT->current.proj.JZ_set = true;
			break;

		case GMT_POLAR:		/* Polar (theta,r) */
			gmt_set_column (GMT, GMT_IN, GMT_X, GMT_IS_LON);
			gmt_set_column (GMT, GMT_IN, GMT_Y, GMT_IS_FLOAT);
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
				if (n == 2) GMT->current.proj.pars[0] = gmt_M_to_inch (GMT, &txt_a[i]);
				error += (GMT->current.proj.pars[0] <= 0.0 || n != 2) ? 1 : 0;
			}
			else if (n_slashes == 0) {
				GMT->current.proj.pars[0] = gmt_M_to_inch (GMT, &args[i]);
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
				error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = 0.0;
			}
			error += gmtinit_scale_or_width (GMT, txt_b, &GMT->current.proj.pars[1]);
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
				error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				GMT->current.proj.lon0 = GMT->current.proj.pars[0];
			}
			if (txt_b[0]) {
				error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
				GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			}
			error += gmtinit_scale_or_width (GMT, txt_c, &GMT->current.proj.pars[2]);
			error += ((project == GMT_CYL_EQ || project == GMT_MERCATOR || project == GMT_POLYCONIC)
				&& fabs (GMT->current.proj.pars[1]) >= 90.0);
			error += !(n == n_slashes + 1);
			break;

		case GMT_ALBERS:	/* Albers Equal-area Conic */
		case GMT_ECONIC:	/* Equidistant Conic */
		case GMT_LAMBERT:	/* Lambert Conformal Conic */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_c, GMT_IS_LAT, &GMT->current.proj.pars[2]), txt_c);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_d, GMT_IS_LAT, &GMT->current.proj.pars[3]), txt_d);
			error += gmtinit_scale_or_width (GMT, txt_e, &GMT->current.proj.pars[4]);
			error += !(n_slashes == 4 && n == 5);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_ORTHO:
			/* Reset any genper related settings */
			GMT->current.proj.g_debug = 0;
			GMT->current.proj.g_box = GMT->current.proj.g_outside = GMT->current.proj.g_longlat_set =
				GMT->current.proj.g_radius = GMT->current.proj.g_auto_twist = false;
			GMT->current.proj.g_sphere = true; /* force spherical as default */
			GMT->current.proj.pars[5] = GMT->current.proj.pars[6] = GMT->current.proj.pars[7] = 0.0;
			/* Intentional fall-through (no break) */
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
				GMT->current.proj.pars[3] = gmt_M_to_inch (GMT, txt_d);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)	/* Got lon0/lat0/radius/lat */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)	/* Got lon0/lat0/lat_h/radius/lat */
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					GMT->current.proj.pars[3] = gmt_M_to_inch (GMT, txt_d);
					if (gmtinit_get_uservalue (GMT, txt_e, gmt_M_type (GMT, GMT_IN, GMT_Y), &c, "oblique latitude")) return 1;
					if (c <= -90.0 || c >= 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);  /* As co-latitude it may be 180 so cannot parse as latitude */
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
					if (gmtinit_get_uservalue (GMT, txt_e, gmt_M_type (GMT, GMT_IN, GMT_Y), &c, "oblique latitude")) return 1;
					if (c < -90.0 || c > 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
					GMT->current.proj.pars[5] = 1.0;	/* flag for true scale case */
				}
				else if (n_slashes == 4) {
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/1:%lf", txt_a, txt_b, txt_c, txt_e, &GMT->current.proj.pars[3]);
					if (gmtinit_get_uservalue (GMT, txt_e, gmt_M_type (GMT, GMT_IN, GMT_Y), &c, "oblique latitude")) return 1;
					if (c < -90.0 || c > 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
					GMT->current.proj.pars[5] = 1.0;	/* flag for true scale case */
				}
				if (GMT->current.proj.pars[3] != 0.0) GMT->current.proj.pars[3] = 1.0 / (GMT->current.proj.pars[3] * GMT->current.proj.unit);
			}
			else if (width_given) {
				if (n_slashes == 2)
					n = sscanf (args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_d);
				else if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				GMT->current.proj.pars[3] = gmt_M_to_inch (GMT, txt_d);
			}
			else {	/* Scale entered as radius/lat */
				if (n_slashes == 3)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_d, txt_e);
				else if (n_slashes == 4)
					n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				if (n == n_slashes + 1) {
					GMT->current.proj.pars[3] = gmt_M_to_inch (GMT, txt_d);
					if (gmtinit_get_uservalue (GMT, txt_e, gmt_M_type (GMT, GMT_IN, GMT_Y), &c, "oblique latitude")) return 1;
					if (c <= -90.0 || c >= 90.0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
						error++;
					}
					else
						error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_e, GMT_IS_LAT, &GMT->current.proj.pars[4]), txt_e);
				}
			}
			error += (n != n_slashes + 1);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);
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
				gmt_message (GMT, "genper: arg '%s' n_slashes %d k %d\n", args, n_slashes, j);
				gmt_message (GMT, "initial error %d\n", error);
				gmt_message (GMT, "j = %d\n", j);
				gmt_message (GMT, "width_given %d\n", width_given);
			}

			n = sscanf(args+i, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s",
				&(txt_arr[0][0]), &(txt_arr[1][0]), &(txt_arr[2][0]), &(txt_arr[3][0]),
				&(txt_arr[4][0]), &(txt_arr[5][0]), &(txt_arr[6][0]), &(txt_arr[7][0]),
				&(txt_arr[8][0]), &(txt_arr[9][0]), &(txt_arr[10][0]));

			if (GMT->current.proj.g_debug > 1) {
				for (i = 0 ; i < n ; i ++) {
					gmt_message (GMT, "txt_arr[%d] '%s'\n", i, &(txt_arr[i][0]));
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
				if (error) gmt_message (GMT, "scale entered but couldn't read\n");
			}
			else  if (width_given) {
				GMT->current.proj.pars[2] = gmt_M_to_inch (GMT, &(txt_arr[n-1][0]));
			}
			else {
				GMT->current.proj.pars[2] = gmt_M_to_inch (GMT, &(txt_arr[n-2][0]));
				/*            GMT->current.proj.pars[3] = GMT_ddmmss_to_degree(txt_i); */
				if (gmtinit_get_uservalue (GMT, &(txt_arr[n-1][0]), gmt_M_type (GMT, GMT_IN, GMT_Y), &c, "oblique latitude")) return 1;
				if (c <= -90.0 || c >= 90.0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Oblique latitude must be in -90 to +90 range\n");
					error++;
				}
				else
					error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, &(txt_arr[n-1][0]), GMT_IS_LAT, &GMT->current.proj.pars[3]), &(txt_arr[n-1][0]));
				if (error) gmt_message (GMT, "error in reading last lat value\n");
			}
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, &(txt_arr[0][0]), GMT_IS_LON, &GMT->current.proj.pars[0]), &(txt_arr[0][0]));
			if (error) gmt_message (GMT, "error is reading longitude '%s'\n", &(txt_arr[0][0]));
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, &(txt_arr[1][0]), GMT_IS_LAT, &GMT->current.proj.pars[1]), &(txt_arr[1][0]));
			if (error) gmt_message (GMT, "error reading latitude '%s'\n", &(txt_arr[1][0]));

			/* g_alt    GMT->current.proj.pars[4] = atof(txt_c); */
			nlen = (int)strlen(&(txt_arr[2][0]));
			if (txt_arr[2][nlen-1] == 'r') {
				GMT->current.proj.g_radius = true;
				txt_arr[2][nlen-1] = 0;
			}
			error += gmt_verify_expectations (GMT, GMT_IS_FLOAT, gmt_scanf (GMT, &(txt_arr[2][0]), GMT_IS_FLOAT, &GMT->current.proj.pars[4]), &(txt_arr[2][0]));
			if (error) gmt_message (GMT, "error reading altitude '%s'\n", &(txt_arr[2][0]));

			/* g_az    GMT->current.proj.pars[5] = atof(txt_d); */
			nlen = (int)strlen(&(txt_arr[3][0]));
			if (txt_arr[3][nlen-1] == 'l' || txt_arr[3][nlen-1] == 'L') {
				GMT->current.proj.g_longlat_set = true;
				txt_arr[3][nlen-1] = 0;
			}
			error += gmt_verify_expectations (GMT, GMT_IS_GEO, gmt_scanf (GMT, &(txt_arr[3][0]), GMT_IS_GEO, &GMT->current.proj.pars[5]), &(txt_arr[3][0]));
			if (error) gmt_message (GMT, "error reading azimuth '%s'\n", &(txt_arr[3][0]));

			/* g_tilt    GMT->current.proj.pars[6] = atof(txt_e); */
			nlen = (int)strlen(&(txt_arr[4][0]));
			if (txt_arr[4][nlen-1] == 'l' || txt_arr[4][nlen-1] == 'L') {
				GMT->current.proj.g_longlat_set = true;
				txt_arr[4][nlen-1] = 0;
			}
			error += gmt_verify_expectations (GMT, GMT_IS_GEO, gmt_scanf (GMT, &(txt_arr[4][0]), GMT_IS_GEO, &GMT->current.proj.pars[6]), &(txt_arr[4][0]));
			if (error) gmt_message (GMT, "error reading tilt '%s'\n", &(txt_arr[4][0]));

			if (n > 6) {
				/*g_twist   GMT->current.proj.pars[7] = atof(txt_f); */
				nlen = (int)strlen(&(txt_arr[5][0]));
				if (txt_arr[5][nlen-1] == 'n') {
					GMT->current.proj.g_auto_twist = true;
					txt_arr[5][nlen-1] = 0;
				}
				error += gmt_verify_expectations (GMT, GMT_IS_GEO, gmt_scanf (GMT, &(txt_arr[5][0]), GMT_IS_GEO, &GMT->current.proj.pars[7]), &(txt_arr[5][0]));
				if (error) gmt_message (GMT, "error reading twist '%s'\n", &(txt_arr[5][0]));

				/*g_width   GMT->current.proj.pars[8] = atof(txt_f); */
				if (n > 7) {
					error += gmt_verify_expectations (GMT, GMT_IS_GEO, gmt_scanf (GMT, &(txt_arr[6][0]), GMT_IS_GEO, &GMT->current.proj.pars[8]), &(txt_arr[6][0]));
					if (error) gmt_message (GMT, "error reading width '%s'\n", &(txt_arr[6][0]));

					if (n > 8) {
						/* g_height  GMT->current.proj.pars[9] = atof(txt_g); */
						error += gmt_verify_expectations (GMT, GMT_IS_GEO, gmt_scanf (GMT, &(txt_arr[7][0]), GMT_IS_GEO, &GMT->current.proj.pars[9]), &(txt_arr[7][0]));
						if (error) gmt_message (GMT, "error height '%s'\n", &(txt_arr[7][0]));
					}
				}
			}
			error += (GMT->current.proj.pars[2] <= 0.0 || (k >= 0 && width_given));
			if (error) gmt_message (GMT, "final error %d\n", error);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_OBLIQUE_MERC:		/* Oblique mercator, specifying origin and azimuth or second point */
			GMT->current.proj.N_hemi = (strchr ("AB", GMT->common.J.string[1]) == NULL) ? true : false;	/* Upper case -JoA, -JoB allows S pole views */
			if (n_slashes == 3) {
				n = sscanf (args, "%[^/]/%[^/]/%lf/%s", txt_a, txt_b, &az, txt_e);
				error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
				c = 10.0;	/* compute point 10 degrees from origin along azimuth */
				GMT->current.proj.pars[2] = GMT->current.proj.pars[0] + atand (sind (c) * sind (az) / (cosd (GMT->current.proj.pars[1]) * cosd (c) - sind (GMT->current.proj.pars[1]) * sind (c) * cosd (az)));
				GMT->current.proj.pars[3] = d_asind (sind (GMT->current.proj.pars[1]) * cosd (c) + cosd (GMT->current.proj.pars[1]) * sind (c) * cosd (az));
			}
			else if (n_slashes == 4) {
				n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
				error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
				error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
				error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);
				error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_d, GMT_IS_LAT, &GMT->current.proj.pars[3]), txt_d);
			}
			error += gmtinit_scale_or_width (GMT, txt_e, &GMT->current.proj.pars[4]);
			GMT->current.proj.pars[6] = 0.0;
			error += !(n == n_slashes + 1);
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_OBLIQUE_MERC_POLE:	/* Oblique mercator, specifying origin and pole */
			GMT->current.proj.N_hemi = (GMT->common.J.string[1] != 'C') ? true : false;	/* Upper case -JoC allows S pole views */
			n = sscanf (args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &GMT->current.proj.pars[0]), txt_a);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &GMT->current.proj.pars[1]), txt_b);
			error += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_c, GMT_IS_LON, &GMT->current.proj.pars[2]), txt_c);
			error += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_d, GMT_IS_LAT, &GMT->current.proj.pars[3]), txt_d);
			if (GMT->current.proj.pars[3] < 0.0) {	/* Flip from S hemisphere to N */
				GMT->current.proj.o_spole = true;
				GMT->current.proj.pars[3] = -GMT->current.proj.pars[3];
				GMT->current.proj.pars[2] += 180.0;
				if (GMT->current.proj.pars[2] >= 360.0) GMT->current.proj.pars[2] -= 360.0;
			}
			error += gmtinit_scale_or_width (GMT, txt_e, &GMT->current.proj.pars[4]);
			GMT->current.proj.pars[6] = 1.0;
			error += !(n_slashes == 4 && n == 5);
			project = GMT_OBLIQUE_MERC;
			GMT->current.proj.lon0 = GMT->current.proj.pars[0];	GMT->current.proj.lat0 = GMT->current.proj.pars[1];
			break;

		case GMT_UTM:	/* Universal Transverse Mercator */
			if (!strchr (args, '/')) {	/* No UTM zone given, must obtain from -R */
				GMT->current.proj.pars[0] = -1.0;	/* Flag we need zone to be set later */
				error += gmtinit_scale_or_width (GMT, args, &GMT->current.proj.pars[1]);
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
				error += gmtinit_scale_or_width (GMT, txt_b, &GMT->current.proj.pars[1]);
				error += !(n_slashes == 1 && n == 2);
			}
			break;

		default:
			error++;
			project = GMT_NO_PROJ;
			break;
	}

	if (project != GMT_ZAXIS) {
		GMT->current.proj.projection = project;
		GMT->current.proj.projection_GMT = project;		/* Make a copy to use when using the Proj4 lib */
	}
	if (mod_flag > 1) args[last_pos] = last_char;	/* Restore modifier */

	return (error > 0);
}

/*! Converts c, i, and p into 0,1,3 */
GMT_LOCAL int gmtinit_get_unit (struct GMT_CTRL *GMT, char c) {

	int i;
	switch ((int)c) {
		case 'c':	/* cm */
			i = GMT_CM;
			break;
		case 'i':	/* inch */
			i = GMT_INCH;
			break;
		case 'm':	/* meter */
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Specifying a plot distance unit in meters is deprecated; use c, i, or p.\n");
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

/*! . */
GMT_LOCAL int gmtinit_parse_front (struct GMT_CTRL *GMT, char *text, struct GMT_SYMBOL *S) {
	/* Parser for -Sf<tickgap>[/<ticklen>][+l|+r][+<type>][+o<offset>][+p<pen>]
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
	S->f.f_gap = (txt_a[0] == '-') ? atof (txt_a) : gmt_M_to_inch (GMT, txt_a);
	S->f.f_len = (n == 1) ? 0.15 * S->f.f_gap : gmt_M_to_inch (GMT, txt_b);

	S->f.f_symbol = GMT_FRONT_FAULT;	/* Default is the fault symbol */
	S->f.f_angle = 20.0;			/* Default slip arrow angle */
	S->f.f_sense = GMT_FRONT_CENTERED;	/* Default is centered symbols unless +l or +r is found */
	S->f.f_pen = 0;				/* Draw outline with pen set via -W, i.e., same as frontline */
	while ((gmt_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
		switch (p[0]) {
			case 'b':	S->f.f_symbol = GMT_FRONT_BOX;		break;	/* [half-]square front */
			case 'c':	S->f.f_symbol = GMT_FRONT_CIRCLE;	break;	/* [half-]circle front */
			case 'f':	S->f.f_symbol = GMT_FRONT_FAULT;	break;	/* Fault front */
			case 'l':	S->f.f_sense  = GMT_FRONT_LEFT;		break;	/* Symbols to the left */
			case 'r':	S->f.f_sense  = GMT_FRONT_RIGHT;	break;	/* Symbols to the right */
			case 's':	S->f.f_symbol = GMT_FRONT_SLIP;				/* Strike-slip front */
						if (p[1]) S->f.f_angle = atof (&p[1]);		/* Set slip arrow angle */
						break;
			case 'S':	S->f.f_symbol = GMT_FRONT_SLIPC;		/* Strike-slip front */
					S->f.f_angle = (p[1]) ? atof (&p[1]) : 40.0;	/* Set curved slip arrow angle [40] */
					break;
			case 't':	S->f.f_symbol = GMT_FRONT_TRIANGLE;	break;	/* Triangle front */
			case 'o':	S->f.f_off = gmt_M_to_inch (GMT, &p[1]);	break;	/* Symbol offset along line */
			case 'p':	if (p[1]) {	/* Get alternate pen for front-symbol outline [-W] */
						if (gmt_getpen (GMT, &p[1], &S->f.pen)) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +p<pen> argument %s\n", &p[1]);
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
GMT_LOCAL int gmtinit_parse_text (struct GMT_CTRL *GMT, char *text, struct GMT_SYMBOL *S) {

	unsigned int pos = 0, k, j, slash, error = 0;
	if ((!strstr (text, "+t") && strchr (text, '/')) || strchr (text, '%')) {	/* GMT4 syntax */
		char *c = NULL;
		if (gmt_M_compat_check (GMT, 4)) {
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning in Option -Sl: Sl<size>/<string>[%<font>] is deprecated syntax\n");
			if ((c = strchr (text, '%'))) {	/* Gave font name or number, too */
				*c = 0;	/* Chop off the %font info */
				c++;		/* Go to next character */
				if (gmt_getfont (GMT, c, &S->font)) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-Sl contains bad font (set to %s)\n", gmt_putfont (GMT, &S->font));
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
		while ((gmt_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
			switch (p[0]) {
				case 'f':	/* Change font */
					if (gmt_getfont (GMT, &p[1], &S->font))
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-Sl contains bad +<font> modifier (set to %s)\n", gmt_putfont (GMT, &S->font));
					break;
				case 'j':	S->justify = gmt_just_decode (GMT, &p[1], PSL_NO_DEF);	break;	/* text justification */
				case 't':	strncpy (S->string, &p[1], GMT_LEN256-1);	break;	/* Get the symbol text */
				default:
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error option -Sl: Bad modifier +%c\n", p[0]);
					error++;
					break;
			}
		}
	}

	return (error);
}

/*! Loads the m_per_unit array with the scaling factors that converts various units to meters.
 * Also sets all the names for the units.
 * See gmt_project.h for enums that can be used as array indices) */

GMT_LOCAL void gmtinit_init_unit_conversion (struct GMT_CTRL *GMT) {

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
GMT_LOCAL int gmtinit_scanf_epoch (struct GMT_CTRL *GMT, char *s, int64_t *rata_die, double *t0) {

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
	char tt[GMT_LEN8];

	i = 0;
	while (s[i] && s[i] == ' ') i++;
	if (!(s[i])) return (-1);
	if (strchr (&s[i], 'W') ) {	/* ISO calendar string, date with or without clock */
		if (sscanf (&s[i], "%5d-W%2d-%1d%[^0-9:-]%2d:%2d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (gmtlib_iso_ywd_is_bad (yy, mo, dd) ) return (-1);
		rd = gmtlib_rd_from_iywd (GMT, yy, mo, dd);
	}
	else {				/* Gregorian calendar string, date with or without clock */
		if (sscanf (&s[i], "%5d-%2d-%2d%[^0-9:-]%2d:%2d:%lf", &yy, &mo, &dd, tt, &hh, &mm, &ss) < 3) return (-1);
		if (gmtlib_g_ymd_is_bad (yy, mo, dd) ) return (-1);
		rd = gmt_rd_from_gymd (GMT, yy, mo, dd);
	}
	if (gmt_M_hms_is_bad (hh, mm, ss)) return (-1);

	*rata_die = rd;								/* Rata day number of epoch */
	*t0 =  (GMT_HR2SEC_F * hh + GMT_MIN2SEC_F * mm + ss) * GMT_SEC2DAY;	/* Fractional day (0<= t0 < 1) since rata_die of epoch */
	return (GMT_NOERROR);
}

/*! Scan a PostScript encoding string and look for degree, ring and other special encodings.
 * Use Brute Force and Ignorance.
 * Scanning to find the codes for 7 symbols we plot but whose code depends on character set
 * (ring, degree, colon, squote, dquote, minus, hyphen).
 */

GMT_LOCAL int gmtinit_load_encoding (struct GMT_CTRL *GMT) {
	char symbol[GMT_LEN256] = {""};
	unsigned int code = 0, pos = 0;
	int k;
	struct GMT_ENCODING *enc = &GMT->current.setting.ps_encoding;

	snprintf (symbol, GMT_LEN256, "PSL_%s", enc->name);	/* Prepend the PSL_ prefix */
	if ((k = get_psl_encoding (symbol)) == -1) {
		GMT_exit (GMT, GMT_ERROR_ON_FOPEN); return GMT_RUNTIME_ERROR;
	}

	while ((gmt_strtok (PSL_ISO_encoding[k], " /\t\n", &pos, symbol))) {	/* Here, symbol is reused */
		if (strcmp (symbol, "[") == 0)	/* We have found the start of the encoding array. */ {
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
		else if (strcmp (symbol, "minus") == 0)
			enc->code[gmt_minus] = code;
		else if (strcmp (symbol, "hyphen") == 0)
			enc->code[gmt_hyphen] = code;
		code++;
	}

	return (GMT_NOERROR);
}

GMT_LOCAL void gmtinit_def_us_locale (struct GMT_CTRL *GMT) {

	/* GMT Time language file for US (english) mode [US] */

	/* Month record */
	strcpy (GMT->current.language.month_name[0][0], "January"); strcpy (GMT->current.language.month_name[1][0], "Jan");
	strcpy (GMT->current.language.month_name[2][0], "J");       strcpy (GMT->current.language.month_name[3][0], "JAN");
	strcpy (GMT->current.language.month_name[0][1], "February");strcpy (GMT->current.language.month_name[1][1], "Feb");
	strcpy (GMT->current.language.month_name[2][1], "F");       strcpy (GMT->current.language.month_name[3][1], "FEB");
	strcpy (GMT->current.language.month_name[0][2], "March");   strcpy (GMT->current.language.month_name[1][2], "Mar");
	strcpy (GMT->current.language.month_name[2][2], "M");       strcpy (GMT->current.language.month_name[3][2], "MAR");
	strcpy (GMT->current.language.month_name[0][3], "April");   strcpy (GMT->current.language.month_name[1][3], "Apr");
	strcpy (GMT->current.language.month_name[2][3], "A");       strcpy (GMT->current.language.month_name[3][3], "APR");
	strcpy (GMT->current.language.month_name[0][4], "May");     strcpy (GMT->current.language.month_name[1][4], "May");
	strcpy (GMT->current.language.month_name[2][4], "M");       strcpy (GMT->current.language.month_name[3][4], "MAY");
	strcpy (GMT->current.language.month_name[0][5], "June");    strcpy (GMT->current.language.month_name[1][5], "Jun");
	strcpy (GMT->current.language.month_name[2][5], "J");       strcpy (GMT->current.language.month_name[3][5], "JUN");
	strcpy (GMT->current.language.month_name[0][6], "July");    strcpy (GMT->current.language.month_name[1][6], "Jul");
	strcpy (GMT->current.language.month_name[2][6], "J");       strcpy (GMT->current.language.month_name[3][6], "JUL");
	strcpy (GMT->current.language.month_name[0][7], "August");  strcpy (GMT->current.language.month_name[1][7], "Aug");
	strcpy (GMT->current.language.month_name[2][7], "A");       strcpy (GMT->current.language.month_name[3][7], "AUG");
	strcpy (GMT->current.language.month_name[0][8], "September");strcpy(GMT->current.language.month_name[1][8], "Sep");
	strcpy (GMT->current.language.month_name[2][8], "S");       strcpy (GMT->current.language.month_name[3][8], "SEP");
	strcpy (GMT->current.language.month_name[0][9], "October"); strcpy (GMT->current.language.month_name[1][9], "Oct");
	strcpy (GMT->current.language.month_name[2][9], "O");       strcpy (GMT->current.language.month_name[3][9], "OCT");
	strcpy (GMT->current.language.month_name[0][10],"November");strcpy (GMT->current.language.month_name[1][10],"Nov");
	strcpy (GMT->current.language.month_name[2][10],"N");       strcpy (GMT->current.language.month_name[3][10],"NOV");
	strcpy (GMT->current.language.month_name[0][11],"December");strcpy (GMT->current.language.month_name[1][11],"Dec");
	strcpy (GMT->current.language.month_name[2][11],"D");       strcpy (GMT->current.language.month_name[3][11],"DEC");

	/* Week name record */
	strcpy (GMT->current.language.week_name[0], "Week");        strcpy (GMT->current.language.week_name[1], "Wk");
	strcpy (GMT->current.language.week_name[2], "W");

	/* Weekday record */
	strcpy (GMT->current.language.day_name[0][0], "Sunday");   strcpy (GMT->current.language.day_name[1][0], "Sun");
	strcpy (GMT->current.language.day_name[2][0], "S");
	strcpy (GMT->current.language.day_name[0][1], "Monday");   strcpy (GMT->current.language.day_name[1][1], "Mon");
	strcpy (GMT->current.language.day_name[2][1], "M");
	strcpy (GMT->current.language.day_name[0][2], "Tuesday");  strcpy (GMT->current.language.day_name[1][2], "Tue");
	strcpy (GMT->current.language.day_name[2][2], "T");
	strcpy (GMT->current.language.day_name[0][3], "Wednesday");strcpy (GMT->current.language.day_name[1][3], "Wed");
	strcpy (GMT->current.language.day_name[2][3], "W");
	strcpy (GMT->current.language.day_name[0][4], "Thursday"); strcpy (GMT->current.language.day_name[1][4], "Thu");
	strcpy (GMT->current.language.day_name[2][4], "T");
	strcpy (GMT->current.language.day_name[0][5], "Friday");   strcpy (GMT->current.language.day_name[1][5], "Fri");
	strcpy (GMT->current.language.day_name[2][5], "F");
	strcpy (GMT->current.language.day_name[0][6], "Saturday"); strcpy (GMT->current.language.day_name[1][6], "Sat");
	strcpy (GMT->current.language.day_name[2][6], "S");

	/* Compass name record */
	strcpy (GMT->current.language.cardinal_name[0][0], "West"); strcpy (GMT->current.language.cardinal_name[1][0], "W");
	strcpy (GMT->current.language.cardinal_name[2][0], "W");
	strcpy (GMT->current.language.cardinal_name[0][1], "East"); strcpy (GMT->current.language.cardinal_name[1][1], "E");
	strcpy (GMT->current.language.cardinal_name[2][1], "E");
	strcpy (GMT->current.language.cardinal_name[0][2], "South"); strcpy (GMT->current.language.cardinal_name[1][2], "S");
	strcpy (GMT->current.language.cardinal_name[2][2], "S");
	strcpy (GMT->current.language.cardinal_name[0][3], "North"); strcpy (GMT->current.language.cardinal_name[1][3], "N");
	strcpy (GMT->current.language.cardinal_name[2][3], "N");
}

/*! . */
int gmt_get_V (char arg) {
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
GMT_LOCAL int gmtinit_parse_V_option (struct GMT_CTRL *GMT, char arg) {
	int mode = gmt_get_V (arg);
	if (mode < 0) return true;	/* Error in parsing */
	GMT->current.setting.verbose = (unsigned int)mode;
	return 0;
}

/*! . */
GMT_LOCAL unsigned int gmtinit_key_lookup (char *name, char **list, unsigned int n) {
	unsigned int i;

	for (i = 0; i < n && strcmp (name, list[i]); i++);
	return (i);
}

/*! . */
GMT_LOCAL int gmtinit_get_language (struct GMT_CTRL *GMT) {
	FILE *fp = NULL;
	char file[PATH_MAX] = {""}, line[GMT_BUFSIZ] = {""}, full[16] = {""}, abbrev[16] = {""}, c[16] = {""}, dwu;
	char *months[12];

	int i, nm = 0, nw = 0, nu = 0, nc = 0;

	if (!strcmp(GMT->current.setting.language, "us")) {
		gmtinit_def_us_locale (GMT);
		return 0;
	}

	gmt_M_memset (months, 12, char *);

	sprintf (line, "gmt_%s", GMT->current.setting.language);
	gmt_getsharepath (GMT, "localization", line, ".locale", file, R_OK);
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Could not load language %s - revert to us (English)!\n",
			GMT->current.setting.language);
		gmt_getsharepath (GMT, "localization", "gmt_us", ".locale", file, R_OK);
		if ((fp = fopen (file, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not find %s!\n", file);
			GMT_exit (GMT, GMT_ERROR_ON_FOPEN); return GMT_ERROR_ON_FOPEN;
		}
		strcpy (GMT->current.setting.language, "us");
	}

	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;
		sscanf (line, "%c %d %s %s %s", &dwu, &i, full, abbrev, c);
		if (i <= 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Index in %s is zero or negative!\n", line);
			fclose (fp);
			GMT_exit (GMT, GMT_NOT_A_VALID_PARAMETER); return GMT_NOT_A_VALID_PARAMETER;
		}
		if (dwu == 'M') {	/* Month record */
			if (i > 12) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Month index in %s exceeds 12!\n", line);
				fclose (fp);
				GMT_exit (GMT, GMT_NOT_A_VALID_PARAMETER); return GMT_NOT_A_VALID_PARAMETER;
			}
			strncpy (GMT->current.language.month_name[0][i-1], full, GMT_LEN16-1);
			strncpy (GMT->current.language.month_name[1][i-1], abbrev, GMT_LEN16-1);
			strncpy (GMT->current.language.month_name[2][i-1], c, GMT_LEN16-1);
			gmt_str_toupper(abbrev);
			strncpy (GMT->current.language.month_name[3][i-1], abbrev, GMT_LEN16-1);
			nm += i;
		}
		else if (dwu == 'W') {	/* Weekday record */
			if (i > 7) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Weekday index in %s exceeds 7!\n", line);
				fclose (fp);
				GMT_exit (GMT, GMT_NOT_A_VALID_PARAMETER); return GMT_NOT_A_VALID_PARAMETER;
			}
			strncpy (GMT->current.language.day_name[0][i-1], full, GMT_LEN16-1);
			strncpy (GMT->current.language.day_name[1][i-1], abbrev, GMT_LEN16-1);
			strncpy (GMT->current.language.day_name[2][i-1], c, GMT_LEN16-1);
			nw += i;
		}
		else if (dwu == 'U') {			/* Week name record */
			strncpy (GMT->current.language.week_name[0], full, GMT_LEN16-1);
			strncpy (GMT->current.language.week_name[1], abbrev, GMT_LEN16-1);
			strncpy (GMT->current.language.week_name[2], c, GMT_LEN16-1);
			nu += i;
		}
		else {	/* Compass name record */
			if (i > 4) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cardinal name index in %s exceeds 4!\n", line);
				fclose (fp);
				GMT_exit (GMT, GMT_NOT_A_VALID_PARAMETER); return GMT_NOT_A_VALID_PARAMETER;
			}
			strncpy (GMT->current.language.cardinal_name[0][i-1], full, GMT_LEN16-1);
			strncpy (GMT->current.language.cardinal_name[1][i-1], abbrev, GMT_LEN16-1);
			strncpy (GMT->current.language.cardinal_name[2][i-1], c, GMT_LEN16-1);
			nc += i;
		}
	}
	fclose (fp);
	if (!(nm == 78 && nw == 28 && nu == 1 && nc == 10)) {	/* Sums of 1-12, 1-7, 1, and 1-4, respectively */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Mismatch between expected and actual contents in %s!\n", file);
		GMT_exit (GMT, GMT_NOT_A_VALID_PARAMETER); return GMT_NOT_A_VALID_PARAMETER;
	}
	return (GMT_NOERROR);
}

/*! . */
void gmtinit_conf (struct GMT_CTRL *GMT) {
	int i, error = 0;
	double const pt = 1.0/72.0;	/* points to inch */
	/* Initialize all the settings to standard SI settings */

		/* FORMAT group */

	/* FORMAT_CLOCK_IN */
	strcpy(GMT->current.setting.format_clock_in, "hh:mm:ss");
	gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_in, &GMT->current.io.clock_input, 0);
	/* FORMAT_DATE_IN */
	strcpy (GMT->current.setting.format_date_in, "yyyy-mm-dd");
	gmtlib_date_C_format (GMT, GMT->current.setting.format_date_in, &GMT->current.io.date_input, 0);
	/* FORMAT_CLOCK_OUT */
	strcpy (GMT->current.setting.format_clock_out, "hh:mm:ss");
	gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_out, &GMT->current.io.clock_output, 1);
	/* FORMAT_DATE_OUT */
	strcpy (GMT->current.setting.format_date_out, "yyyy-mm-dd");
	gmtlib_date_C_format (GMT, GMT->current.setting.format_date_out, &GMT->current.io.date_output, 1);
	/* FORMAT_GEO_OUT */
	strcpy (GMT->current.setting.format_geo_out, "D");
	gmtlib_geo_C_format (GMT);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of gmt_begin */
	/* FORMAT_CLOCK_MAP */
	strcpy (GMT->current.setting.format_clock_map, "hh:mm:ss");
	gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_map, &GMT->current.plot.calclock.clock, 2);
	/* FORMAT_DATE_MAP */
	strcpy (GMT->current.setting.format_date_map, "yyyy-mm-dd");
	gmtlib_date_C_format (GMT, GMT->current.setting.format_date_map, &GMT->current.plot.calclock.date, 2);
	/* FORMAT_GEO_MAP */
	strcpy (GMT->current.setting.format_geo_map, "ddd:mm:ss");
	gmtlib_plot_C_format (GMT);	/* Update format statements */
	/* FORMAT_TIME_PRIMARY_MAP */
	strcpy (GMT->current.setting.format_time[GMT_PRIMARY], "full");
	/* FORMAT_TIME_SECONDARY_MAP */
	strcpy (GMT->current.setting.format_time[GMT_SECONDARY], "full");
	/* FORMAT_FLOAT_OUT */
	strcpy (GMT->current.setting.format_float_out, "%.12g");
	strcpy (GMT->current.setting.format_float_out_orig, "%.12g");
	/* FORMAT_FLOAT_MAP */
	strcpy (GMT->current.setting.format_float_map, "%.12g");
	/* FORMAT_TIME_STAMP */
	strcpy (GMT->current.setting.format_time_stamp, "%Y %b %d %H:%M:%S");

		/* FONT group */

	/* FONT_ANNOT_PRIMARY */
	error += gmt_getfont (GMT, "12p,Helvetica,black", &GMT->current.setting.font_annot[GMT_PRIMARY]);
	GMT->current.setting.given_unit[GMTCASE_FONT_ANNOT_PRIMARY] = 'p';
	/* FONT_ANNOT_SECONDARY */
	error += gmt_getfont (GMT, "14p,Helvetica,black", &GMT->current.setting.font_annot[GMT_SECONDARY]);
	GMT->current.setting.given_unit[GMTCASE_FONT_ANNOT_SECONDARY] = 'p';
	/* FONT_HEADING */
	error += gmt_getfont (GMT, "32p,Helvetica,black", &GMT->current.setting.font_heading);
	GMT->current.setting.given_unit[GMTCASE_FONT_HEADING] = 'p';
	/* FONT_TITLE */
	error += gmt_getfont (GMT, "24p,Helvetica,black", &GMT->current.setting.font_title);
	GMT->current.setting.given_unit[GMTCASE_FONT_TITLE] = 'p';
	/* FONT_LABEL */
	error += gmt_getfont (GMT, "16p,Helvetica,black", &GMT->current.setting.font_label);
	GMT->current.setting.given_unit[GMTCASE_FONT_LABEL] = 'p';
	/* FONT_TAG */
	error += gmt_getfont (GMT, "20p,Helvetica,black", &GMT->current.setting.font_tag);
	GMT->current.setting.given_unit[GMTCASE_FONT_TAG] = 'p';
	/* FONT_LOGO */
	error += gmt_getfont (GMT, "8p,Helvetica,black", &GMT->current.setting.font_logo);
	GMT->current.setting.given_unit[GMTCASE_FONT_LOGO] = 'p';

		/* MAP group */

	/* MAP_ANNOT_OFFSET_PRIMARY, MAP_ANNOT_OFFSET_SECONDARY */
	GMT->current.setting.map_annot_offset[GMT_PRIMARY] = GMT->current.setting.map_annot_offset[GMT_SECONDARY] = 5 * pt; /* 5p */
	GMT->current.setting.given_unit[GMTCASE_MAP_ANNOT_OFFSET_PRIMARY] = 'p';
	GMT->current.setting.given_unit[GMTCASE_MAP_ANNOT_OFFSET_SECONDARY] = 'p';
	/* MAP_ANNOT_OBLIQUE */
	GMT->current.setting.map_annot_oblique = GMT_OBL_ANNOT_ANYWHERE;
	/* MAP_ANNOT_MIN_ANGLE */
	GMT->current.setting.map_annot_min_angle = 20;
	/* MAP_ANNOT_MIN_SPACING */
	GMT->current.setting.map_annot_min_spacing = 0; /* 0p */
	GMT->current.setting.given_unit[GMTCASE_MAP_ANNOT_MIN_SPACING] = 'p';
	/* MAP_ANNOT_ORTHO */
	strcpy (GMT->current.setting.map_annot_ortho, "we");
	/* MAP_DEGREE_SYMBOL (degree) */
	GMT->current.setting.map_degree_symbol = gmt_degree;
	/* MAP_FRAME_AXES */
	strcpy (GMT->current.setting.map_frame_axes, "WESNZ");
	for (i = 0; i < 5; i++) GMT->current.map.frame.side[i] = 0;	/* Unset default settings */
	GMT->current.map.frame.draw_box = false;
	error += gmtinit_decode5_wesnz (GMT, "WESNZ", false);
	/* MAP_DEFAULT_PEN */
	error += gmt_getpen (GMT, "default,black", &GMT->current.setting.map_default_pen);
	/* MAP_FRAME_PEN */
	error += gmt_getpen (GMT, "thicker,black", &GMT->current.setting.map_frame_pen);
	/* MAP_FRAME_TYPE (fancy) */
	GMT->current.setting.map_frame_type = GMT_IS_FANCY;
	GMT->current.setting.map_graph_extension_unit = GMT_GRAPH_EXTENSION_UNIT;	/* Defaults for graph */
	GMT->current.setting.map_graph_extension = GMT_GRAPH_EXTENSION;
	/* MAP_FRAME_WIDTH */
	GMT->current.setting.map_frame_width = 5 * pt; /* 5p */
	GMT->current.setting.given_unit[GMTCASE_MAP_FRAME_WIDTH] = 'p';
	/* MAP_GRID_CROSS_SIZE_PRIMARY, MAP_GRID_CROSS_SIZE_SECONDARY */
	GMT->current.setting.map_grid_cross_size[GMT_PRIMARY] = GMT->current.setting.map_grid_cross_size[GMT_SECONDARY] = 0; /* 0p */
	GMT->current.setting.given_unit[GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY] = 'p';
	GMT->current.setting.given_unit[GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY] = 'p';
	/* MAP_GRID_PEN_PRIMARY */
	error += gmt_getpen (GMT, "default,black", &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
	/* MAP_GRID_PEN_SECONDARY */
	error += gmt_getpen (GMT, "thinner,black", &GMT->current.setting.map_grid_pen[GMT_SECONDARY]);
	/* MAP_HEADING_OFFSET */
	GMT->current.setting.map_heading_offset = 18 * pt;	/* 18p */
	GMT->current.setting.given_unit[GMTCASE_MAP_HEADING_OFFSET] = 'p';
	/* MAP_LABEL_OFFSET */
	GMT->current.setting.map_label_offset = 8 * pt;	/* 8p */
	GMT->current.setting.given_unit[GMTCASE_MAP_LABEL_OFFSET] = 'p';
	/* MAP_LINE_STEP */
	GMT->current.setting.map_line_step = 0.75 * pt;	/* 0.75p */
	GMT->current.setting.given_unit[GMTCASE_MAP_LINE_STEP] = 'p';
	/* MAP_LOGO */
	GMT->current.setting.map_logo = false;
	/* MAP_LOGO_POS */
	GMT->current.setting.map_logo_justify = PSL_BL;	/* BL */
	GMT->current.setting.map_logo_pos[GMT_X] = GMT->current.setting.map_logo_pos[GMT_Y] = -54 * pt;	/* -54p */
	GMT->current.setting.given_unit[GMTCASE_MAP_LOGO_POS] = 'p';
	/* MAP_ORIGIN_X, MAP_ORIGIN_Y */
	GMT->current.setting.map_origin[GMT_X] = GMT->current.setting.map_origin[GMT_Y] = 1;	/* 1i */
	GMT->current.setting.given_unit[GMTCASE_MAP_ORIGIN_X] = 'i';
	GMT->current.setting.given_unit[GMTCASE_MAP_ORIGIN_Y] = 'i';
	/* MAP_POLAR_CAP */
	GMT->current.setting.map_polar_cap[0] = 85;
	GMT->current.setting.map_polar_cap[1] = 90;
	/* MAP_SCALE_HEIGHT */
	GMT->current.setting.map_scale_height = 5 * pt;	/* 5p */
	GMT->current.setting.given_unit[GMTCASE_MAP_SCALE_HEIGHT] = 'p';
	/* MAP_TICK_LENGTH_PRIMARY */
	GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = 5 * pt;	/* 5p */
	GMT->current.setting.map_tick_length[GMT_TICK_UPPER] = 2.5 * pt;	/* 2.5p */
	GMT->current.setting.given_unit[GMTCASE_MAP_TICK_LENGTH_PRIMARY] = 'p';
	/* MAP_TICK_LENGTH_SECONDARY */
	GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = 15 * pt;	/* 15p */
	GMT->current.setting.map_tick_length[GMT_TICK_LOWER] = 3.75 * pt;	/* 3.75p */
	GMT->current.setting.given_unit[GMTCASE_MAP_TICK_LENGTH_SECONDARY] = 'p';
	/* MAP_TICK_PEN_PRIMARY */
	error += gmt_getpen (GMT, "thinner,black", &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);
	/* MAP_TICK_PEN_SECONDARY */
	error += gmt_getpen (GMT, "thinner,black", &GMT->current.setting.map_tick_pen[GMT_SECONDARY]);
	/* MAP_TITLE_OFFSET */
	GMT->current.setting.map_title_offset = 14 * pt;	/* 14p */
	GMT->current.setting.given_unit[GMTCASE_MAP_TITLE_OFFSET] = 'p';
	/* MAP_VECTOR_SHAPE */
	GMT->current.setting.map_vector_shape = 0;

		/* COLOR group */

	/* COLOR_BACKGROUND */
	error += gmt_getrgb (GMT, "black", GMT->current.setting.color_patch[GMT_BGD]);
	/* COLOR_FOREGROUND */
	error += gmt_getrgb (GMT, "white", GMT->current.setting.color_patch[GMT_FGD]);
	/* COLOR_MODEL */
	GMT->current.setting.color_model = GMT_RGB;
	/* COLOR_NAN */
	error += gmt_getrgb (GMT, "127.5", GMT->current.setting.color_patch[GMT_NAN]);
	/* COLOR_HSV_MIN_S */
	GMT->current.setting.color_hsv_min_s = 1;
	/* COLOR_HSV_MAX_S */
	GMT->current.setting.color_hsv_max_s = 0.1;
	/* COLOR_HSV_MIN_V */
	GMT->current.setting.color_hsv_min_v = 0.3;
	/* COLOR_HSV_MAX_V */
	GMT->current.setting.color_hsv_max_v = 1;

		/* PS group */

	/* PS_CHAR_ENCODING */
	strcpy (GMT->current.setting.ps_encoding.name, "ISOLatin1+");
	gmtinit_load_encoding (GMT);
	/* PS_COLOR_MODEL */
	GMT->current.setting.ps_color_mode = PSL_RGB;
	/* PS_IMAGE_COMPRESS */
	if (GMT->PSL) {	/* Only when using PSL in this session */
#ifdef HAVE_ZLIB
		GMT->PSL->internal.compress = PSL_DEFLATE;
		GMT->PSL->internal.deflate_level = 5;
#else
		/* Silently fall back to LZW compression when ZLIB not available */
		GMT->PSL->internal.compress = PSL_LZW;
#endif
		/* PS_LINE_CAP */
		GMT->PSL->internal.line_cap = PSL_BUTT_CAP;
		/* PS_LINE_JOIN */
		GMT->PSL->internal.line_join = PSL_MITER_JOIN;
		/* PS_MITER_LIMIT */
		GMT->PSL->internal.miter_limit = 35;
	}
	/* PS_PAGE_COLOR */
	error += gmt_getrgb (GMT, "white", GMT->current.setting.ps_page_rgb);
	/* PS_PAGE_ORIENTATION */
	/* PS_MEDIA */
	if (GMT->current.setting.run_mode == GMT_MODERN) {
		gmtinit_setautopagesize (GMT);
	}
	else {
		GMT->current.setting.ps_orientation = PSL_LANDSCAPE;
		i = gmtinit_key_lookup ("a4", GMT_media_name, GMT_N_MEDIA);
		/* Use the specified standard format */
		GMT->current.setting.ps_media = i;
		GMT->current.setting.ps_page_size[0] = GMT_media[i].width;
		GMT->current.setting.ps_page_size[1] = GMT_media[i].height;
	}
	/* PS_SCALE_X */
	GMT->current.setting.ps_magnify[GMT_X] = 1;
	/* PS_SCALE_Y */
	GMT->current.setting.ps_magnify[GMT_Y] = 1;
	/* PS_TRANSPARENCY */
	strcpy (GMT->current.setting.ps_transpmode, "Normal");
	/* PS_CONVERT */
	strcpy (GMT->current.setting.ps_convert, GMT_SESSION_CONVERT);
	/* PS_COMMENTS */
	if (GMT->PSL) GMT->PSL->internal.comments = 0;	/* Only when using PSL in this session */

		/* IO group */

	/* IO_COL_SEPARATOR */
	strcpy (GMT->current.setting.io_col_separator, "\t");
	/* IO_FIRST_HEADER */
	GMT->current.setting.io_first_header = 0;
	/* IO_GRIDFILE_FORMAT */
	strcpy (GMT->current.setting.io_gridfile_format, "nf");
	/* IO_GRIDFILE_SHORTHAND */
	GMT->current.setting.io_gridfile_shorthand = false;
	/* IO_HEADER */
	GMT->current.setting.io_header[GMT_IN] = GMT->current.setting.io_header[GMT_OUT] = false;
	/* IO_HEADER_MARKER */
	GMT->current.setting.io_head_marker[GMT_OUT] = GMT->current.setting.io_head_marker[GMT_IN] = '#';
	/* IO_N_HEADER_RECS */
	GMT->current.setting.io_n_header_items = 0;
	/* IO_NAN_RECORDS (pass) */
	GMT->current.setting.io_nan_records = true;
	/* IO_NC4_CHUNK_SIZE (auto) */
	GMT->current.setting.io_nc4_chunksize[0] = k_netcdf_io_chunked_auto;
	/* IO_NC4_DEFLATION_LEVEL */
	GMT->current.setting.io_nc4_deflation_level = 3;
	/* IO_LONLAT_TOGGLE */
	GMT->current.setting.io_lonlat_toggle[GMT_IN] = false;
	/* We got false/f/0 or true/t/1. Set outgoing setting to the same as the ingoing. */
	GMT->current.setting.io_lonlat_toggle[GMT_OUT] = GMT->current.setting.io_lonlat_toggle[GMT_IN];
	/* IO_SEGMENT_BINARY */
	GMT->current.setting.n_bin_header_cols = 2;
	/* IO_SEGMENT_MARKER */
	GMT->current.setting.io_seg_marker[GMT_OUT] = GMT->current.setting.io_seg_marker[GMT_IN] = '>';

		/* PROJ group */

	/* PROJ_AUX_LATITUDE (authalic) */
	GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2A;
	/* PROJ_ELLIPSOID */
	GMT->current.setting.proj_ellipsoid = gmt_get_ellipsoid (GMT, "WGS-84");
	gmtlib_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
	/* PROJ_DATUM (Not implemented yet) */
	/* PROJ_GEODESIC */
	GMT->current.setting.proj_geodesic = GMT_GEODESIC_VINCENTY;
	gmtlib_init_geodesic (GMT);	/* Set function pointer depending on the geodesic selected */
	/* PROJ_LENGTH_UNIT */
	GMT->current.setting.proj_length_unit = GMT_CM;
	/* PROJ_MEAN_RADIUS */
	GMT->current.setting.proj_mean_radius = GMT_RADIUS_AUTHALIC;
	gmtlib_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
	/* PROJ_SCALE_FACTOR (default) */
	GMT->current.setting.proj_scale_factor = -1.0;

		/* GMT group */

	/* GMT_COMPATIBILITY */
	GMT->current.setting.compatibility = (GMT->current.setting.run_mode == GMT_CLASSIC) ? 4 : 6;
	/* GMTCASE_GMT_AUTO_DOWNLOAD */
	GMT->current.setting.auto_download = GMT_YES_DOWNLOAD;
	/* GMTCASE_GMT_DATA_URL_LIMIT */
	GMT->current.setting.url_size_limit = 0;
	/* GMT_CUSTOM_LIBS (default to none) */
	/* GMT_EXPORT_TYPE */
	GMT->current.setting.export_type = GMT_DOUBLE;
	/* GMT_EXTRAPOLATE_VAL (NaN) */
	GMT->current.setting.extrapolate_val[0] = GMT_EXTRAPOLATE_NONE;
	/* GMT_FFT */
	GMT->current.setting.fft = k_fft_auto;
	/* GMT_HISTORY */
	GMT->current.setting.history = (GMT_HISTORY_READ | GMT_HISTORY_WRITE);
	/* GMT_INTERPOLANT */
	GMT->current.setting.interpolant = GMT_SPLINE_AKIMA;
	/* GMT_LANGUAGE */
	strcpy (GMT->current.setting.language, "us");
	/* GMT_TRIANGULATE */
#ifdef TRIANGLE_D
	GMT->current.setting.triangulate = GMT_TRIANGLE_SHEWCHUK;
#else
	GMT->current.setting.triangulate = GMT_TRIANGLE_WATSON;
#endif
	/* GMT_VERBOSE (compat) */
	error += gmtinit_parse_V_option (GMT, 'c');

		/* DIR group */

	/* DIR_DATA (Empty) */
	/* DIR_USER (Empty) */
	/* DIR_CACHE (Empty) */
	/* DIR_DCW */
	if (GMT->session.DCWDIR)
		gmt_M_str_free (GMT->session.DCWDIR);
	GMT->session.DCWDIR = strdup (DCW_INSTALL_PATH);
	/* DIR_GSHHG */
	if (GMT->session.GSHHGDIR)
		gmt_M_str_free (GMT->session.GSHHGDIR);
	GMT->session.GSHHGDIR = strdup (GSHHG_INSTALL_PATH);

		/* TIME group */

	/* TIME_EPOCH */
	strcpy (GMT->current.setting.time_system.epoch, "1970-01-01T00:00:00");
	(void) gmt_init_time_system_structure (GMT, &GMT->current.setting.time_system);
	/* TIME_IS_INTERVAL */
	GMT->current.setting.time_is_interval = false;
	/* TIME_INTERVAL_FRACTION */
	GMT->current.setting.time_interval_fraction = 0.5;
	gmtinit_get_language (GMT);	/* Load in names and abbreviations in chosen language */
	/* TIME_REPORT */
	GMT->current.setting.timer_mode = GMT_NO_TIMER;
	/* TIME_UNIT */
	GMT->current.setting.time_system.unit = 's';
	(void) gmt_init_time_system_structure (GMT, &GMT->current.setting.time_system);
	/* TIME_WEEK_START */
	GMT->current.setting.time_week_start = gmtinit_key_lookup ("Monday", GMT_weekdays, 7);
	/* TIME_Y2K_OFFSET_YEAR */
	GMT->current.setting.time_Y2K_offset_year = 1950;
	GMT->current.time.Y2K_fix.y2_cutoff = GMT->current.setting.time_Y2K_offset_year % 100;
	GMT->current.time.Y2K_fix.y100 = GMT->current.setting.time_Y2K_offset_year - GMT->current.time.Y2K_fix.y2_cutoff;
	GMT->current.time.Y2K_fix.y200 = GMT->current.time.Y2K_fix.y100 + 100;

	if (error)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized value during gmtdefaults initialization.\n");

	if (!strncmp (GMT_DEF_UNITS, "US", 2U))
		gmtinit_conf_US (GMT);	/* Override with US settings */
}

/*! . */
void gmtinit_conf_US (struct GMT_CTRL *GMT) {
	int i, case_val;
	/* Update the settings to US where they differ from standard SI settings:
	 *     Setting			SI			US
	 * --------------------------------------------
	 * PROJ_LENGTH_UNIT		cm	 		inch
	 * PS_CHAR_ENCODING		ISOLatin1+	Standard+
	 * PS_MEDIA				a4			letter
	 * TIME_WEEK_START		Monday		Sunday
	 */

	/* PROJ_LENGTH_UNIT */
	case_val = gmt_hash_lookup (GMT, "PROJ_LENGTH_UNIT", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
	if (case_val >= 0) GMT_keywords_updated[case_val] = true;
	GMT->current.setting.proj_length_unit = GMT_INCH;
	/* PS_CHAR_ENCODING */
	case_val = gmt_hash_lookup (GMT, "PS_CHAR_ENCODING", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
	if (case_val >= 0) GMT_keywords_updated[case_val] = true;
	strcpy (GMT->current.setting.ps_encoding.name, "Standard+");
	gmtinit_load_encoding (GMT);
	/* PS_MEDIA */
	if (GMT->current.setting.run_mode == GMT_MODERN)
		gmtinit_setautopagesize (GMT);
	else {
		case_val = gmt_hash_lookup (GMT, "PS_MEDIA", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
		if (case_val >= 0) GMT_keywords_updated[case_val] = true;
		i = gmtinit_key_lookup ("letter", GMT_media_name, GMT_N_MEDIA);
		/* Use the specified standard format */
		GMT->current.setting.ps_media = i;
		GMT->current.setting.ps_page_size[0] = GMT_media[i].width;
		GMT->current.setting.ps_page_size[1] = GMT_media[i].height;
	}
	/* TIME_WEEK_START */
	case_val = gmt_hash_lookup (GMT, "TIME_WEEK_START", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
	if (case_val >= 0) GMT_keywords_updated[case_val] = true;
	GMT->current.setting.time_week_start = gmtinit_key_lookup ("Sunday", GMT_weekdays, 7);
}

/*! . */
GMT_LOCAL int gmtinit_init_fonts (struct GMT_CTRL *GMT) {
	unsigned int i = 0, n_GMT_fonts;
	size_t n_alloc = 0;
	char fullname[GMT_BUFSIZ] = {""};

	/* Loads all available fonts for this installation */

	/* First the standard 35 + 4 PostScript fonts from Adobe */

	gmt_set_meminc (GMT, GMT_SMALL_CHUNK);	/* Only allocate a small amount [64] */
	GMT->session.font = gmt_M_malloc (GMT, GMT->session.font, 0, &n_alloc, struct GMT_FONTSPEC);

	/* First the standard 35 + 4 PostScript fonts from Adobe */
	gmt_M_memcpy (GMT->session.font, GMT_standard_fonts, GMT_N_STANDARD_FONTS, struct GMT_FONTSPEC);
	GMT->session.n_fonts = n_GMT_fonts = i = GMT_N_STANDARD_FONTS;

	/* Then any custom fonts:
	   To add additional fonts, create a file called PSL_custom_fonts.txt
	   in GMT/share/postscriptlight and add your extra font information there.
	   The fontheight below is the height of A for unit fontsize.
	   Encoded = 0 if we may re-encode this font as needed. */

	if (gmt_getsharepath (GMT, "postscriptlight", "PSL_custom_fonts", ".txt", fullname, R_OK)) {	/* Decode Custom font file */
		FILE *in = NULL;
		char buf[GMT_BUFSIZ] = {""};
		if ((in = fopen (fullname, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open %s\n", fullname);
			GMT_exit (GMT, GMT_ERROR_ON_FOPEN); return GMT_ERROR_ON_FOPEN;
		}

		while (fgets (buf, GMT_BUFSIZ, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			if (i == n_alloc) GMT->session.font = gmt_M_malloc (GMT, GMT->session.font, i, &n_alloc, struct GMT_FONTSPEC);
			if (sscanf (buf, "%s %lf %*d", fullname, &GMT->session.font[i].height) != 2) {
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Trouble decoding custom font info [%s].  Skipping this font.\n", buf);
				continue;
			}
			if (strlen (fullname) >= GMT_LEN32) {
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Font %s exceeds %d characters and will be truncated\n", fullname, GMT_LEN32);
				fullname[GMT_LEN32-1] = '\0';
			}
			strncpy (GMT->session.font[i].name, fullname, GMT_LEN32-1);
			i++;
		}
		fclose (in);
		GMT->session.n_fonts = i;
	}
	n_alloc = i;
	GMT->session.font = gmt_M_malloc (GMT, GMT->session.font, 0, &n_alloc, struct GMT_FONTSPEC);
	gmt_reset_meminc (GMT);
	return (GMT_NOERROR);
}

/*! Gets the rata die of today */
GMT_LOCAL void gmtinit_set_today (struct GMT_CTRL *GMT) {
	time_t right_now = time (NULL);			/* Unix time right now */
	struct tm *moment = gmtime (&right_now);	/* Convert time to a TM structure */
	/* Calculate rata die from yy, mm, and dd */
	/* tm_mon is 0-11, so add 1 for 1-12 range, tm_year is years since 1900, so add 1900, but tm_mday is 1-31 so use as is */
	GMT->current.time.today_rata_die = gmt_rd_from_gymd (GMT, 1900 + moment->tm_year, moment->tm_mon + 1, moment->tm_mday);
}

/*! . */
GMT_LOCAL void gmtinit_free_user_media (struct GMT_CTRL *GMT) {
	/* Free any user-specified media formats */
	unsigned int i;

	if (GMT->session.n_user_media == 0) return;	/* Nothing to free */

	for (i = 0; i < GMT->session.n_user_media; i++)
		gmt_M_str_free (GMT->session.user_media_name[i]);
	gmt_M_free (GMT, GMT->session.user_media_name);
	gmt_M_free (GMT, GMT->session.user_media);
	GMT->session.n_user_media = 0;
}

/*! . */
GMT_LOCAL unsigned int gmtinit_load_user_media (struct GMT_CTRL *GMT) {
	/* Load any user-specified media formats */
	size_t n_alloc = 0;
	unsigned int n = 0;
	double w, h;
	char line[GMT_BUFSIZ] = {""}, file[PATH_MAX] = {""}, media[GMT_LEN64] = {""};
	FILE *fp = NULL;

	gmt_getsharepath (GMT, "postscriptlight", "gmt_custom_media", ".conf", file, R_OK);
	if ((fp = fopen (file, "r")) == NULL) return (0);	/* Not a critical file so no error if we cannot read it */

	gmtinit_free_user_media (GMT);	/* Free any previously allocated user-specified media formats */
	gmt_set_meminc (GMT, GMT_TINY_CHUNK);	/* Only allocate a small amount */
	while (fgets (line, GMT_BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments and blank lines */

		if (sscanf (line, "%s %lg %lg", media, &w, &h) != 3) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding file %s.  Bad format? [%s]\n", file, line);
			fclose (fp);
			GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
		}

		gmt_str_tolower (media);	/* Convert string to lower case */

		if (n == n_alloc) {
			size_t k = n_alloc;	/* So we don't update n_alloc in the first gmt_M_malloc call */
			GMT->session.user_media = gmt_M_malloc (GMT, GMT->session.user_media, n, &k, struct GMT_MEDIA);
			GMT->session.user_media_name = gmt_M_malloc (GMT, GMT->session.user_media_name, n, &n_alloc, char *);
		}
		GMT->session.user_media_name[n] = strdup (media);
		GMT->session.user_media[n].width  = w;
		GMT->session.user_media[n].height = h;
		n++;
	}
	fclose (fp);

	n_alloc = n;
	GMT->session.user_media = gmt_M_malloc (GMT, GMT->session.user_media, 0, &n_alloc, struct GMT_MEDIA);
	GMT->session.user_media_name = gmt_M_malloc (GMT, GMT->session.user_media_name, 0, &n_alloc, char *);
	gmt_reset_meminc (GMT);

	GMT->session.n_user_media = n;

	return (n);
}

/*! . */
GMT_LOCAL void gmtinit_free_dirnames (struct GMT_CTRL *GMT) {
	/* Free any directory settings */

	gmt_M_str_free (GMT->session.SHAREDIR);
	gmt_M_str_free (GMT->session.HOMEDIR);
	gmt_M_str_free (GMT->session.DATADIR);
	gmt_M_str_free (GMT->session.DCWDIR);
	gmt_M_str_free (GMT->session.GSHHGDIR);
	gmt_M_str_free (GMT->session.USERDIR);
	gmt_M_str_free (GMT->session.CACHEDIR);
	gmt_M_str_free (GMT->session.TMPDIR);
	gmt_M_str_free (GMT->session.CUSTOM_LIBS);
	gmt_M_str_free (GMT->session.DATAURL);
}


/*! . */
GMT_LOCAL void gmtinit_free_GMT_ctrl (struct GMT_CTRL *GMT) {	/* Deallocate control structure */
	if (!GMT) return;	/* Never was allocated */
	gmt_M_str_free (GMT);
}

/*! . */
GMT_LOCAL struct GMT_CTRL *gmtinit_new_GMT_ctrl (struct GMTAPI_CTRL *API, const char *session, unsigned int pad) {	/* Allocate and initialize a new common control structure */
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
	struct ELLIPSOID ref_ellipsoid[GMT_N_ELLIPSOIDS] = {   /* This constant is created by CMake - do not edit */
	#include "gmt_ellipsoids.h"                            /* This include file is created by CMake - do not edit */
	};
	struct DATUM datum[GMT_N_DATUMS] = {    /* This constant is created by CMake - do not edit */
	#include "gmt_datums.h"                 /* This include file is created by CMake - do not edit */
	};
	gmt_M_unused(session);

	GMT_Report (API, GMT_MSG_DEBUG, "Enter: gmtinit_new_GMT_ctrl\n");
	/* Alloc using calloc since gmt_M_memory may use resources not yet initialized */
	GMT = calloc (1U, sizeof (struct GMT_CTRL));
	gmt_M_memcpy (GMT->current.setting.ref_ellipsoid, ref_ellipsoid, 1, ref_ellipsoid);
	gmt_M_memcpy (GMT->current.setting.proj_datum, datum, 1, datum);

	/* Assign the daddy */
	GMT->parent = API;

	/* Assign the three std* pointers */

	GMT->session.std[GMT_IN]  = stdin;
	GMT->session.std[GMT_OUT] = stdout;
	GMT->session.std[GMT_ERR] = stderr;

	/* Set default verbosity level */
	GMT->current.setting.verbose = GMT_MSG_COMPAT;

#ifdef MEMDEBUG
	gmt_memtrack_init (GMT);	/* Helps us determine memory leaks */
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
	gmt_runtime_libdir (path);
#endif
	GMT->init.runtime_libdir = strdup (path);

	gmtinit_set_env (GMT);	/* Get GMT_SHAREDIR and other environment path parameters */

	gmtinit_init_fonts (GMT);	/* Load in available font names */

	/* Initialize values whose defaults are not necessarily 0/false/NULL */

	/* MAP settings */

	gmt_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);	/* Default distance calculations are in user units */

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
	/* We need some defaults here for the cases where we do not actually set these with gmt_map_setup */
	GMT->current.proj.fwd_x = GMT->current.proj.fwd_y = GMT->current.proj.fwd_z = &gmt_translin;
	GMT->current.proj.inv_x = GMT->current.proj.inv_y = GMT->current.proj.inv_z = &gmt_itranslin;
	/* z_level will be updated in GMT_init_three_D, but if it doesn't, it does not matter,
	 * because by default, z_scale = 0.0 */
	GMT->current.proj.z_level = DBL_MAX;
	GMT->current.proj.xyz_pos[GMT_X] = GMT->current.proj.xyz_pos[GMT_Y] = GMT->current.proj.xyz_pos[GMT_Z] = true;
	GMT->current.proj.z_project.view_azimuth = 180.0;
	GMT->current.proj.z_project.view_elevation = 90.0;
	GMT->current.proj.z_project.plane = -1;	/* Initialize no perspective projection */
	GMT->current.proj.z_project.level = 0.0;
	for (i = 0; i < 4; i++) GMT->current.proj.edge[i] = true;
	gmt_grdio_init (GMT);
	gmt_set_pad (GMT, pad); /* Sets default number of rows/cols for boundary padding in this session */
	GMT->current.proj.f_horizon = 90.0;
	GMT->current.proj.proj4 = gmt_M_memory (GMT, NULL, GMT_N_PROJ4, struct GMT_PROJ4);
	for (i = 0; i < GMT_N_PROJ4; i++) {	/* Load up proj4 structure once and for all */
		GMT->current.proj.proj4[i].name = strdup (GMT_proj4[i].name);
		GMT->current.proj.proj4[i].id = GMT_proj4[i].id;
	}
	/* TIME_SYSTEM settings */
	strcpy (GMT->current.setting.time_system.epoch, "2000-01-01T12:00:00");
	GMT->current.setting.time_system.unit = 'd';

	/* INIT settings */

	gmt_M_memcpy (GMT->session.u2u, u2u, 1, u2u);
	for (i = 0; i < 4; i++) strncpy (GMT->session.unit_name[i], unit_name[i], 7U);
	gmt_M_make_fnan (GMT->session.f_NaN);
	gmt_M_make_dnan (GMT->session.d_NaN);
	for (i = 0; i < 3; i++) GMT->session.no_rgb[i] = -1.0;

#ifdef _OPENMP
	/* Set the default number of threads = number of available cores, but only under OpenMP */
	GMT->common.x.n_threads = gmtlib_get_num_processors();
#endif

	/* Set the names of the default CPTs */
	strcpy (GMT->init.cpt[0], "rainbow");	/* GMT default CPT unless overridden by data type specific CPT */
	strcpy (GMT->init.cpt[1], "geo");	/* GMT default CPT for earth_relief grids */
	strcpy (GMT->init.cpt[2], "srtm");	/* GMT default CPT for srtm_relief grids */
	
	GMT_Report (API, GMT_MSG_DEBUG, "Exit:  gmtinit_new_GMT_ctrl\n");
	return (GMT);
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

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
 * However, gmtlib_explain_options is not called directly but via GMT_Option which
 * do accept a list of comma-separated options and there are the normal GMT common
 * option letters, sometimes with modifiers, and it translate between those and the
 * crazy cases below.\n
 * Remaining cases for additional options: A,H,L,M,N,T,W,e,m,q,u,v,w
 */

GMT_LOCAL void explain_R_geo (struct GMT_CTRL *GMT) {
	gmt_message (GMT, "\t-R Specify the min/max coordinates of your data region in user units.\n");
	gmt_message (GMT, "\t   Use dd:mm[:ss] for regions given in arc degrees, minutes [and seconds].\n");
	gmt_message (GMT, "\t   Use -R<xmin/xmax/ymin/ymax>+<unit> for regions given in projected coordinates.\n");
	gmt_message (GMT, "\t     with <unit> selected from %s.\n", GMT_LEN_UNITS2_DISPLAY);
	gmt_message (GMT, "\t   Use [yyy[-mm[-dd]]]T[hh[:mm[:ss[.xxx]]]] format for time axes.\n");
	gmt_message (GMT, "\t   Append +r if -R specifies the coordinates of the lower left and\n");
	gmt_message (GMT, "\t     upper right corners of a rectangular area.\n");
	if (GMT->current.setting.run_mode == GMT_MODERN)
	gmt_message (GMT, "\t   Use -Re and -Ra to set exact or approximate regions based on your input data (if applicable).\n");
	gmt_message (GMT, "\t   Use -R<gridfile> to use its limits (and increments if applicable).\n");
	gmt_message (GMT, "\t   Use -Rg and -Rd as shorthands for -R0/360/-90/90 and -R-180/180/-90/90.\n");
	gmt_message (GMT, "\t   Derive region from closed polygons from the Digital Chart of the World (DCW):\n");
	gmt_message (GMT, "\t     Append a comma-separated list of ISO 3166 codes for countries to set region, i.e.,\n");
	gmt_message (GMT, "\t     <code1>,<code2>,... etc., using the 2-character ISO country codes (see pscoast -E+l for list).\n");
	gmt_message (GMT, "\t     To select a state of a country (if available), append .state, e.g, US.TX for Texas.\n");
	gmt_message (GMT, "\t     To select a whole continent, give =AF|AN|AS|EU|OC|NA|SA as <code>.\n");
	gmt_message (GMT, "\t     Use +r to modify the region from polygon(s): Append <inc>, <xinc>/<yinc>, or <winc>/<einc>/<sinc>/<ninc>\n");
	gmt_message (GMT, "\t     to round region to these multiples; use +R to extend region by those increments instead [0].\n");
}

void gmtlib_explain_options (struct GMT_CTRL *GMT, char *options) {

	char u, *GMT_choice[2] = {"OFF", "ON"}, *V_code = "qncvld";
	double s;
	unsigned int k;
	size_t s_length;

	if (!options) return;
	if (GMT->common.synopsis.extended) return;	/* Only want to list module-specific options, i.e gave + instead of - */
	u = GMT->session.unit_name[GMT->current.setting.proj_length_unit][0];
	s = GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];	/* Convert from internal inch to users unit */

	s_length = strlen(options);
	for (k = 0; k < s_length; k++) {

		switch (options[k]) {

		case 'B':	/* Tickmark option */

			gmt_message (GMT, "\t-B Specify both (1) basemap frame settings and (2) axes parameters.\n");
			gmt_message (GMT, "\t   Frame settings are modified via an optional single invocation of\n");
			gmt_message (GMT, "\t     -B[<axes>][+b][+g<fill>][+n][+o<lon>/<lat>][+t<title>]\n");
			gmt_message (GMT, "\t   Axes parameters are specified via one or more invocations of\n");
			gmt_message (GMT, "\t     -B[p|s][x|y|z]<info>\n\n");
			gmt_message (GMT, "\t   1. Frame settings control which axes to plot, frame fill, title, and type of gridlines:\n");
			gmt_message (GMT, "\t     <axes> is a combination of W,E,S,N,Z and plots those axes only [Default is WESNZ (all)].\n");
			gmt_message (GMT, "\t     Use lower case w,e,s,n,z just to draw and tick (but not annotate) those axes,\n");
			gmt_message (GMT, "\t     and use l,r,b,t,u just to draw (but not annotate and tick) those axes.\n");
			gmt_message (GMT, "\t     For 3-D plots the Z|z[<corners>][+b] controls the vertical axis.  The <corners> specifies\n");
			gmt_message (GMT, "\t     at which corner(s) to erect the axis via a combination of 1,2,3,4; 1 means lower left corner,\n");
			gmt_message (GMT, "\t     2 is lower right, etc., in a counter-clockwise order. [Default automatically selects one axis].\n");
			gmt_message (GMT, "\t     The optional +b will erect a 3-D frame box to outline the 3-D domain [no frame box]. The +b\n");
			gmt_message (GMT, "\t     is also required for x-z or y-z gridlines to be plotted (if such gridlines are selected below).\n");
			gmt_message (GMT, "\t     Append +g<fill> to paint the inside of the map region before further plotting [no fill].\n");
			gmt_message (GMT, "\t     Append +n to have no frame and annotations whatsoever [Default is controlled by WESNZ/wesnz].\n");
			gmt_message (GMT, "\t     Append +o<plon>/<plat> to draw oblique gridlines about this pole [regular gridlines].\n");
			gmt_message (GMT, "\t     Note: the +o modifier is ignored unless gridlines are specified via the axes parameters (below).\n");
			gmt_message (GMT, "\t     Append +t<title> to place a title over the map frame [no title].\n");
			gmt_message (GMT, "\t   2. Axes settings control the annotation, tick, and grid intervals and labels.\n");
			gmt_message (GMT, "\t     The full axes specification is\n");
			gmt_message (GMT, "\t       -B[p|s][x|y|z]<intervals>[+a<angle>|n|p][+l|L<label>][+p<prefix>][+s|S<secondary_label>][+u<unit>]\n");
			gmt_message (GMT, "\t     Alternatively, you may break this syntax into two separate -B options:\n");
			gmt_message (GMT, "\t       -B[p|s][x|y|z][+a<angle>|n|p][+l|L<label>][+p<prefix>][+s|S<secondary_label>][+u<unit>]\n");
			gmt_message (GMT, "\t       -B[p|s][x|y|z]<intervals>\n");
			gmt_message (GMT, "\t     There are two levels of annotations: Primary and secondary (most situations only require primary).\n");
			gmt_message (GMT, "\t     The -B[p] selects (p)rimary annotations while -Bs specifies (s)econdary annotations.\n");
			gmt_message (GMT, "\t     The [x|y|z] selects which axes the settings apply to.  If none are given we default to xy.\n");
			gmt_message (GMT, "\t     To specify different settings for different axes you must repeat the -B axes option for\n");
			gmt_message (GMT, "\t     each dimension., i.e., provide separate -B[p|s]x, -B[p|s]y, and -B[p|s]z settings.\n");
			gmt_message (GMT, "\t     To prepend a prefix to each annotation (e.g., $ 10, $ 20 ...), add +p<prefix>.\n");
			gmt_message (GMT, "\t     To append a unit to each annotation (e.g., 5 km, 10 km ...), add +u<unit>.\n");
			gmt_message (GMT, "\t     Cartesian x-axis takes optional +a<angle> for slanted or +an for orthogonal annotations [+ap].\n");
			gmt_message (GMT, "\t     Cartesian y-axis takes optional +ap for parallel annotations [+an].\n");
			gmt_message (GMT, "\t     To label an axis, add +l<label>.  Use +L to enforce horizontal labels for y-axes.\n");
			gmt_message (GMT, "\t     For another axis label on the opposite axis, use +s|S as well.\n");
			gmt_message (GMT, "\t     Use quotes if any of the <label>, <prefix> or <unit> have spaces.\n");
			gmt_message (GMT, "\t     For Cartesian axes you can have different labels on the left vs right or bottom vs top\n");
			gmt_message (GMT, "\t     by separating the two labels with ||, e.g., +l\"Left label||Right label\".\n");
			gmt_message (GMT, "\t     Geographic map annotations will automatically have degree, minute, seconds units.\n");
			gmt_message (GMT, "\t     The <intervals> setting controls the annotation spacing and is a textstring made up of one or\n");
			gmt_message (GMT, "\t     more substrings of the form [a|f|g][<stride>[+-<phase>][<unit>]], where the (optional) a\n");
			gmt_message (GMT, "\t     indicates annotation and major tick interval, f minor tick interval, and g grid interval.\n");
			gmt_message (GMT, "\t     Here, <stride> is the spacing between ticks or annotations, the (optional)\n");
			gmt_message (GMT, "\t     <phase> specifies phase-shifted annotations/ticks by that amount, and the (optional)\n");
			gmt_message (GMT, "\t     <unit> specifies the <stride> unit [Default is the unit implied in -R]. There can be\n");
			gmt_message (GMT, "\t     no spaces between the substrings; just append items to make one very long string.\n");
			gmt_message (GMT, "\t     For custom annotations or intervals, let <intervals> be c<intfile>; see man page for details.\n");
			gmt_message (GMT, "\t     The optional <unit> modifies the <stride> value accordingly.  For geographic maps you may use\n");
			gmt_message (GMT, "\t       d: arc degree [Default].\n");
			gmt_message (GMT, "\t       m: arc minute.\n");
			gmt_message (GMT, "\t       s: arc second.\n");
			gmt_message (GMT, "\t     For time axes, several units are recognized:\n");
			gmt_message (GMT, "\t       Y: year - plot using all 4 digits.\n");
			gmt_message (GMT, "\t       y: year - plot only last 2 digits.\n");
			gmt_message (GMT, "\t       O: month - format annotation according to FORMAT_DATE_MAP.\n");
			gmt_message (GMT, "\t       o: month - plot as 2-digit integer (1-12).\n");
			gmt_message (GMT, "\t       U: ISO week - format annotation according to FORMAT_DATE_MAP.\n");
			gmt_message (GMT, "\t       u: ISO week - plot as 2-digit integer (1-53).\n");
			gmt_message (GMT, "\t       r: Gregorian week - 7-day stride from chosen start of week (%s).\n",
			             GMT_weekdays[GMT->current.setting.time_week_start]);
			gmt_message (GMT, "\t       K: ISO weekday - plot name of weekdays in selected language [%s].\n", GMT->current.setting.language);
			gmt_message (GMT, "\t       k: weekday - plot number of the day in the week (see TIME_WEEK_START).\n");
			gmt_message (GMT, "\t       D: day - format annotation according to FORMAT_DATE_MAP, which also determines whether\n");
			gmt_message (GMT, "\t                we should plot day of month (1-31) or day of year (1-366).\n");
			gmt_message (GMT, "\t       d: day - plot as 2- (day of month) or 3- (day of year) integer.\n");
			gmt_message (GMT, "\t       R: Same as d but annotates from start of Gregorian week.\n");
			gmt_message (GMT, "\t       H: hour - format annotation according to FORMAT_CLOCK_MAP.\n");
			gmt_message (GMT, "\t       h: hour - plot as 2-digit integer (0-23).\n");
			gmt_message (GMT, "\t       M: minute - format annotation according to FORMAT_CLOCK_MAP.\n");
			gmt_message (GMT, "\t       m: minute - plot as 2-digit integer (0-59).\n");
			gmt_message (GMT, "\t       S: second - format annotation according to FORMAT_CLOCK_MAP.\n");
			gmt_message (GMT, "\t       s: second - plot as 2-digit integer (0-59; 60-61 if leap seconds are enabled).\n");
			gmt_message (GMT, "\t     Cartesian intervals take no units.\n");
			gmt_message (GMT, "\t     When <stride> is omitted, a reasonable value will be determined automatically, e.g., -Bafg.\n");
			gmt_message (GMT, "\t     Log10 axis: Append l to annotate log10 (value) or p for 10^(log10(value)) [Default annotates value].\n");
			gmt_message (GMT, "\t     Power axis: Append p to annotate value at equidistant pow increments [Default is nonlinear].\n");
			gmt_message (GMT, "\t     See psbasemap man pages for more details and examples of all settings.\n");
			break;

		case 'b':	/* Condensed tickmark option */

			gmt_message (GMT, "\t-B Specify both (1) basemap frame settings and (2) axes parameters.\n");
			gmt_message (GMT, "\t   (1) Frame settings are modified via an optional single invocation of\n");
			gmt_message (GMT, "\t     -B[<axes>][+g<fill>][+n][+o<lon>/<lat>][+t<title>]\n");
			gmt_message (GMT, "\t   (2) Axes parameters are specified via one or more invocations of\n");
			gmt_message (GMT, "\t       -B[p|s][x|y|z]<intervals>[+a<angle>][+l<label>][+p<prefix>][+u<unit>]\n");
			gmt_message (GMT, "\t   <intervals> is composed of concatenated [<type>]<stride>[<unit>][l|p] sub-strings\n");
			gmt_message (GMT, "\t   See psbasemap man page for more details and examples of all settings.\n");
			break;

		case 'J':	/* Map projection option */

			gmt_message (GMT, "\t-J Select the map proJection. The projection type is identified by a 1- or\n");
			gmt_message (GMT, "\t   2-character ID (e.g. 'm' or 'kf') or by an abbreviation followed by a slash\n");
			gmt_message (GMT, "\t   (e.g. 'cyl_stere/'). When using a lower-case ID <scale> can be given either as 1:<xxxx>\n");
			gmt_message (GMT, "\t   or in %s/degree along the standard parallel. Alternatively, when the projection ID is\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t   Capitalized, <scale>|<width> denotes the width of the plot in %s\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t   Append h for map height, + for max map dimension, and - for min map dimension.\n");
			gmt_message (GMT, "\t   When the central meridian (lon0) is optional and omitted, the center of the\n");
			gmt_message (GMT, "\t   longitude range specified by -R is used. The default standard parallel is the equator\n");
			gmt_message (GMT, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is given.\n");

			gmt_message (GMT, "\t   -Ja|A<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Lambert Azimuthal Equal Area)\n");
			gmt_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			gmt_message (GMT, "\t     <horizon> is max distance from center of the projection (<= 180, default 90).\n");
			gmt_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is the distance\n");
			gmt_message (GMT, "\t     in %s to the oblique parallel <lat>.\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			gmt_message (GMT, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Albers Equal-Area Conic)\n");
			gmt_message (GMT, "\t     Give origin, 2 standard parallels, and true scale\n");

			gmt_message (GMT, "\t   -Jc|C<lon0>/<lat0><scale>|<width> (Cassini)\n\t     Give central point and scale\n");

			gmt_message (GMT, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<scale>|<width> (Cylindrical Stereographic)\n");
			gmt_message (GMT, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			gmt_message (GMT, "\t     <lat0> = 66.159467 (Miller's modified Gall), 55 (Kamenetskiy's First),\n");
			gmt_message (GMT, "\t     45 (Gall Stereographic), 30 (Bolshoi Sovietskii Atlas Mira), 0 (Braun)\n");

			gmt_message (GMT, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Equidistant Conic)\n");
			gmt_message (GMT, "\t     Give origin, 2 standard parallels, and true scale\n");

			gmt_message (GMT, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Azimuthal Equidistant)\n");
			gmt_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			gmt_message (GMT, "\t     <horizon> is max distance from center of the projection (<= 180, default 180).\n");
			gmt_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is the distance\n");
			gmt_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			gmt_message (GMT, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Gnomonic)\n");
			gmt_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			gmt_message (GMT, "\t     <horizon> is max distance from center of the projection (< 90, default 60).\n");
			gmt_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			gmt_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			gmt_message (GMT, "\t   -Jg|G<lon0>/<lat0>/<scale>|<width> (Orthographic)\n");
			gmt_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			gmt_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			gmt_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			gmt_message (GMT, "\t   -Jg|G<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale>|<width> (General Perspective)\n");
			gmt_message (GMT, "\t     <lon0>/<lat0> is the center of the projection.\n");
			gmt_message (GMT, "\t     <altitude> is the height (in km) of the viewpoint above local sea level\n");
			gmt_message (GMT, "\t        - if <altitude> less than 10 then it is the distance \n");
			gmt_message (GMT, "\t        from center of earth to viewpoint in earth radii\n");
			gmt_message (GMT, "\t        - if <altitude> has a suffix of 'r' then it is the radius \n");
			gmt_message (GMT, "\t        from the center of earth in kilometers\n");
			gmt_message (GMT, "\t     <azimuth> is azimuth east of North of view\n");
			gmt_message (GMT, "\t     <tilt> is the upward tilt of the plane of projection\n");
			gmt_message (GMT, "\t       if <tilt> < 0 then viewpoint is centered on the horizon\n");
			gmt_message (GMT, "\t     <twist> is the CW twist of the viewpoint in degree\n");
			gmt_message (GMT, "\t     <width> is width of the viewpoint in degree\n");
			gmt_message (GMT, "\t     <height> is the height of the viewpoint in degrees\n");
			gmt_message (GMT, "\t     <scale> can also be given as <radius>/<lat>, where <radius> is distance\n");
			gmt_message (GMT, "\t     in %s to the oblique parallel <lat>. \n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			gmt_message (GMT, "\t   -Jh|H[<lon0>/]<scale>|<width> (Hammer-Aitoff)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Ji|I[<lon0>/]<scale>|<width> (Sinusoidal)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Jj|J[<lon0>/]<scale>|<width> (Miller)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Jkf|Kf[<lon0>/]<scale>|<width> (Eckert IV)\n\t     Give central meridian (opt) and scale\n");
			gmt_message (GMT, "\t   -Jk|K[s][<lon0>/]<scale>|<width> (Eckert VI)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scale>|<width> (Lambert Conformal Conic)\n");
			gmt_message (GMT, "\t     Give origin, 2 standard parallels, and true scale\n");

			gmt_message (GMT, "\t   -Jm|M[<lon0>/[<lat0>/]]<scale>|<width> (Mercator).\n");
			gmt_message (GMT, "\t     Give central meridian (opt), true scale parallel (opt), and scale\n");

			gmt_message (GMT, "\t   -Jn|N[<lon0>/]<scale>|<width> (Robinson projection)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Jo|O<parameters> (Oblique Mercator).  Specify one of three definitions:\n");
			gmt_message (GMT, "\t     -Jo|O[a|A]<lon0>/<lat0>/<azimuth>/<scale>|<width>\n");
			gmt_message (GMT, "\t       Give origin, azimuth of oblique equator, and scale at oblique equator\n");
			gmt_message (GMT, "\t     -Jo|O[b|B]<lon0>/<lat0>/<lon1>/<lat1>/<scale>|<width>\n");
			gmt_message (GMT, "\t       Give origin, second point on oblique equator, and scale at oblique equator\n");
			gmt_message (GMT, "\t     -Jo|Oc|C<lon0>/<lat0>/<lonp>/<latp>/<scale>|<width>\n");
			gmt_message (GMT, "\t       Give origin, pole of projection, and scale at oblique equator\n");
			gmt_message (GMT, "\t       Specify region in oblique degrees OR use -R<>r\n");
			gmt_message (GMT, "\t       Upper-case A|B|C removes enforcement of a northern hemisphere pole.\n");

			gmt_message (GMT, "\t   -Jp|P[a]<scale>|<width>[/<base>][r|z] (Polar (theta,radius))\n");
			gmt_message (GMT, "\t     Linear scaling for polar coordinates.\n");
			gmt_message (GMT, "\t     Optionally append 'a' to -Jp or -JP to use azimuths (CW from North) instead of directions (CCW from East) [default].\n");
			gmt_message (GMT, "\t     Give scale in %s/units, and append theta value for angular offset (base) [0]\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t     Append r to reverse radial direction (s/n must be in 0-90 range) or z to annotate depths rather than radius [Default]\n");

			gmt_message (GMT, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scale>|<width> ((American) Polyconic)\n");
			gmt_message (GMT, "\t     Give central meridian (opt), reference parallel (opt, default = equator), and scale\n");

			gmt_message (GMT, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scale>|<width> (Equidistant Cylindrical)\n");
			gmt_message (GMT, "\t     Give central meridian (opt), standard parallel (opt), and scale\n");
			gmt_message (GMT, "\t     <lat0> = 61.7 (Min. linear distortion), 50.5 (R. Miller equirectangular),\n");
			gmt_message (GMT, "\t     45 (Gall isographic), 43.5 (Min. continental distortion), 42 (Grafarend & Niermann),\n");
			gmt_message (GMT, "\t     37.5 (Min. overall distortion), 0 (Plate Carree, default)\n");

			gmt_message (GMT, "\t   -Jr|R[<lon0>/]<scale>|<width> (Winkel Tripel)\n\t     Give central meridian and scale\n");

			gmt_message (GMT, "\t   -Js|S<lon0>/<lat0>[/<horizon>]/<scale>|<width> (Stereographic)\n");
			gmt_message (GMT, "\t     <lon0>/<lat0> is the center or the projection.\n");
			gmt_message (GMT, "\t     <horizon> is max distance from center of the projection (< 180, default 90).\n");
			gmt_message (GMT, "\t     <scale> is either <1:xxxx> (true at pole) or <slat>/<1:xxxx> (true at <slat>)\n");
			gmt_message (GMT, "\t     or <radius>/<lat> (distance in %s to the [oblique] parallel <lat>.\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);

			gmt_message (GMT, "\t   -Jt|T<lon0>/[<lat0>/]<scale>|<width> (Transverse Mercator).\n\t         Give central meridian and scale\n");
			gmt_message (GMT, "\t     Optionally, also give the central parallel (default = equator)\n");

			gmt_message (GMT, "\t   -Ju|U<zone>/<scale>|<width> (UTM)\n");
			gmt_message (GMT, "\t     Give zone (A,B,Y,Z, or 1-60 (negative for S hemisphere) or append GMT-X) and scale\n");
			gmt_message (GMT, "\t     Or give -Ju|U<scale>|<width> to have the UTM zone determined from the region.\n");

			gmt_message (GMT, "\t   -Jv|V[<lon0>/]<scale>|<width> (van der Grinten)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Jw|W[<lon0>/]<scale>|<width> (Mollweide)\n\t     Give central meridian (opt) and scale\n");

			gmt_message (GMT, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scale>|<width> (Cylindrical Equal-area)\n");
			gmt_message (GMT, "\t     Give central meridian (opt), standard parallel (opt) and scale\n");
			gmt_message (GMT, "\t     <lat0> = 50 (Balthasart), 45 (Gall), 37.5 (Hobo-Dyer), 37.4 (Trystan Edwards),\n");
			gmt_message (GMT, "\t              37.0666 (Caster), 30 (Behrmann), 0 (Lambert, default)\n");

			gmt_message (GMT, "\t   -Jx|X<x-scale|<width>[/<y-scale|height>] (Linear, log, power scaling)\n");
			gmt_message (GMT, "\t     <scale> in %s/units (or 1:xxxx). Optionally, append to <x-scale> and/or <y-scale>:\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t       d         Geographic coordinate (in degrees)\n");
			gmt_message (GMT, "\t       l         Log10 projection\n");
			gmt_message (GMT, "\t       p<power>  x^power projection\n");
			gmt_message (GMT, "\t       t         Calendar time projection using relative time coordinates\n");
			gmt_message (GMT, "\t       T         Calendar time projection using absolute time coordinates\n");
			gmt_message (GMT, "\t     Use / to specify separate x/y scaling (e.g., -Jx0.5c/0.3c).  If 1:xxxxx is used it implies -R is in meters.\n");
			gmt_message (GMT, "\t     If -JX is used then give axes lengths rather than scales.\n");
			break;

		case 'j':	/* Condensed version of J */

			gmt_message (GMT, "\t-J Select map proJection. (<scale> in %s/degree, <width> in %s)\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit],
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t   Append h for map height, or +|- for max|min map dimension.\n");
			gmt_message (GMT, "\t   Azimuthal projections set -Rg unless polar aspect or -R<...>r is set.\n\n");

			gmt_message (GMT, "\t   -Ja|A<lon0>/<lat0>[/<hor>]/<scl (or <radius>/<lat>)|<width> (Lambert Azimuthal EA)\n");

			gmt_message (GMT, "\t   -Jb|B<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Albers Conic EA)\n");

			gmt_message (GMT, "\t   -Jcyl_stere|Cyl_stere/[<lon0>/[<lat0>/]]<lat1>/<lat2>/<scl>|<width> (Cylindrical Stereographic)\n");

			gmt_message (GMT, "\t   -Jc|C<lon0>/<lat0><scl>|<width> (Cassini)\n");

			gmt_message (GMT, "\t   -Jd|D<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Equidistant Conic)\n");

			gmt_message (GMT, "\t   -Je|E<lon0>/<lat0>[/<horizon>]/<scl (or <radius>/<lat>)|<width>  (Azimuthal Equidistant)\n");

			gmt_message (GMT, "\t   -Jf|F<lon0>/<lat0>[/<horizon>]/<scl (or <radius>/<lat>)|<width>  (Gnomonic)\n");

			gmt_message (GMT, "\t   -Jg|G<lon0>/<lat0>/<scl (or <radius>/<lat>)|<width>  (Orthographic)\n");

			gmt_message (GMT, "\t   -Jg|G[<lon0>/]<lat0>[/<horizon>|/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>]/<scl>|<width> (General Perspective)\n");

			gmt_message (GMT, "\t   -Jh|H[<lon0>/]<scl>|<width> (Hammer-Aitoff)\n");

			gmt_message (GMT, "\t   -Ji|I[<lon0>/]<scl>|<width> (Sinusoidal)\n");

			gmt_message (GMT, "\t   -Jj|J[<lon0>/]<scl>|<width> (Miller)\n");

			gmt_message (GMT, "\t   -Jkf|Kf[<lon0>/]<scl>|<width> (Eckert IV)\n");

			gmt_message (GMT, "\t   -Jks|Ks[<lon0>/]<scl>|<width> (Eckert VI)\n");

			gmt_message (GMT, "\t   -Jl|L<lon0>/<lat0>/<lat1>/<lat2>/<scl>|<width> (Lambert Conformal Conic)\n");

			gmt_message (GMT, "\t   -Jm|M[<lon0>/[<lat0>/]]<scl>|<width> (Mercator)\n");

			gmt_message (GMT, "\t   -Jn|N[<lon0>/]<scl>|<width> (Robinson projection)\n");

			gmt_message (GMT, "\t   -Jo|O (Oblique Mercator).  Specify one of three definitions:\n");
			gmt_message (GMT, "\t      -Jo|O[a|A]<lon0>/<lat0>/<azimuth>/<scl>|<width>\n");
			gmt_message (GMT, "\t      -Jo|O[b|B]<lon0>/<lat0>/<lon1>/<lat1>/<scl>|<width>\n");
			gmt_message (GMT, "\t      -Jo|Oc|C<lon0>/<lat0>/<lonp>/<latp>/<scl>|<width>\n");

			gmt_message (GMT, "\t   -Jpoly|Poly/[<lon0>/[<lat0>/]]<scl>|<width> ((American) Polyconic)\n");

			gmt_message (GMT, "\t   -Jq|Q[<lon0>/[<lat0>/]]<scl>|<width> (Equidistant Cylindrical)\n");

			gmt_message (GMT, "\t   -Jr|R[<lon0>/]<scl>|<width> (Winkel Tripel)\n");

			gmt_message (GMT, "\t   -Js|S<lon0>/<lat0>/[<horizon>/]<scl> (or <slat>/<scl> or <radius>/<lat>)|<width> (Stereographic)\n");

			gmt_message (GMT, "\t   -Jt|T<lon0>/[<lat0>/]<scl>|<width> (Transverse Mercator)\n");

			gmt_message (GMT, "\t   -Ju|U[<zone>/]<scl>|<width> (UTM)\n");

			gmt_message (GMT, "\t   -Jv|V<lon0>/<scl>|<width> (van der Grinten)\n");

			gmt_message (GMT, "\t   -Jw|W<lon0>/<scl>|<width> (Mollweide)\n");

			gmt_message (GMT, "\t   -Jy|Y[<lon0>/[<lat0>/]]<scl>|<width> (Cylindrical Equal-area)\n");

			gmt_message (GMT, "\t   -Jp|P[a]<scl>|<width>[/<origin>][r|z] (Polar [azimuth] (theta,radius))\n");

			gmt_message (GMT, "\t   -Jx|X<x-scl>|<width>[d|l|p<power>|t|T][/<y-scl>|<height>[d|l|p<power>|t|T]] (Linear, log, and power projections)\n");
			gmt_message (GMT, "\t   (See psbasemap for more details on projection syntax)\n");
			break;

		case 'I':	/* Near-common option for grid increments */
			gmt_inc_syntax (GMT, 'I', false);
			break;

		case 'K':	/* Append-more-PostScript-later */
			if (GMT->current.setting.run_mode == GMT_CLASSIC && !GMT->current.setting.use_modern_name)	/* -K don't exist in modern mode */
				gmt_message (GMT, "\t-K Allow for more plot code to be appended later.\n");
			break;

		case 'O':	/* Overlay plot */

			if (GMT->current.setting.run_mode == GMT_CLASSIC && !GMT->current.setting.use_modern_name)	/* -O don't exist in modern mode */
				gmt_message (GMT, "\t-O Set Overlay plot mode, i.e., append to an existing plot.\n");
			break;

		case 'P':	/* Portrait or landscape */

			if (GMT->current.setting.run_mode == GMT_CLASSIC && !GMT->current.setting.use_modern_name)	/* -P don't exist in modern mode */
				gmt_message (GMT, "\t-P Set Portrait page orientation [%s].\n", GMT_choice[GMT->current.setting.ps_orientation]);
			break;

		case 'S':	/* CarteSian Region option */

			gmt_message (GMT, "\t-R Specify the xmin/xmax/ymin/ymax coordinates of data region in user units.\n");
			gmt_message (GMT, "\t   Use [yyy[-mm[-dd]]]T[hh[:mm[:ss[.xxx]]]] format for time coordinates.\n");
			gmt_message (GMT, "\t   Or, give a gridfile to use its region (and increments, registration if applicable).\n");
			break;

		case 'G':	/* Geographic Region option */

			explain_R_geo (GMT);
			break;

		case 'R':	/* Generic [Default] Region option */

			explain_R_geo (GMT);
			gmt_message (GMT, "\t   Or use -R<code><x0>/<y0>/<n_columns>/<n_rows> for origin and grid dimensions, where\n");
			gmt_message (GMT, "\t     <code> is a 2-char combo from [T|M|B][L|C|R] (top/middle/bottom/left/center/right)\n");
			gmt_message (GMT, "\t     and grid spacing must be specified via -I<dx>[/<dy>] (also see -r).\n");
			break;

		case 'z':	/* Region addition for 3-D */

			gmt_message (GMT, "\t   Append /zmin/zmax coordinates for the vertical domain limits.\n");
			break;

		case 'r':	/* Region option for 3-D */

			gmt_message (GMT, "\t-R Specify the xyz min/max coordinates of the plot window in user units.\n");
			gmt_message (GMT, "\t   Use dd:mm[:ss] for regions given in degrees, minutes [and seconds].\n");
			gmt_message (GMT, "\t   Append r if first 4 arguments to -R specify the longitudes/latitudes\n");
			gmt_message (GMT, "\t   of the lower left and upper right corners of a rectangular area.\n");
			gmt_message (GMT, "\t   Or, give a gridfile to use its limits (and increments if applicable).\n");
			break;

		case 'U':	/* Plot time mark and [optionally] command line */

			gmt_message (GMT, "\t-U Plot GMT Unix System Time stamp [and optionally appended text or command line].\n");
			gmt_message (GMT, "\t   You may also set the justification point and the relative position of stamp\n");
			gmt_message (GMT, "\t   [+jBL+o-54p/-54p].  Add +c to have the command line plotted [%s].\n", GMT_choice[GMT->current.setting.map_logo]);
			break;

		case 'V':	/* Verbose */

			gmt_message (GMT, "\t-V Change the verbosity level (currently %c).\n", V_code[GMT->current.setting.verbose]);
			gmt_message (GMT, "\t   Choose among 6 levels; each level adds more messages:\n");
			gmt_message (GMT, "\t     q - Quiet, not even fatal error messages.\n");
			gmt_message (GMT, "\t     n - Normal verbosity: only error messages.\n");
			gmt_message (GMT, "\t     c - Also produce compatibility warnings [Default when no -V is used].\n");
			gmt_message (GMT, "\t     v - Verbose progress messages [Default when -V is used].\n");
			gmt_message (GMT, "\t     l - Long verbose progress messages.\n");
			gmt_message (GMT, "\t     d - Debugging messages.\n");
			break;

		case 'X':
		case 'Y':	/* Reset plot origin option */

			gmt_message (GMT, "\t-X -Y Shift origin of plot to (<xshift>, <yshift>).\n");
			gmt_message (GMT, "\t   Prepend r for shift relative to current point (default), prepend a for temporary\n");
			gmt_message (GMT, "\t   adjustment of origin, prepend f to position relative to lower left corner of page,\n");
			gmt_message (GMT, "\t   prepend c for offset of center of plot to center of page.\n");
			gmt_message (GMT, "\t   For overlays (-O), the default setting is [r0], otherwise [f%g%c].\n",
			             GMT->current.setting.map_origin[GMT_Y] * s, u);
			break;

		case 'x':	/* Just linear -Jx|X allowed for this program */

			gmt_message (GMT, "\t-Jx|X for linear projection.  Scale in %s/units (or width in %s).\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit],
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t    Use / to specify separate x/y scaling.\n");
			gmt_message (GMT, "\t    If -JX is used then give axes lengths in %s rather than scales.\n",
			             GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			break;

#ifdef GMT_MP_ENABLED
		case 'y':	/* Number of threads (reassigned from -x in GMT_Option) */
			gmt_message (GMT, "\t-x Limit the number of cores used in multi-threaded algorithms.\n");
			gmt_message (GMT, "\t   Default uses all available cores [%d].\n", gmtlib_get_num_processors());
			gmt_message (GMT, "\t   -x<n>  Select <n> cores (up to all available).\n");
			gmt_message (GMT, "\t   -x-<n> Select (all - <n>) cores (or at least 1).\n");
			break;
#endif
		case 'Z':	/* Vertical scaling for 3-D plots */

			gmt_message (GMT, "\t   -JZ|z For z component of 3-D projections.  Same syntax as -JX|x, i.e.,\n");
			gmt_message (GMT, "\t   -Jz|Z<z-scl>|<height>[d|l|p<power>|t|T] (Linear, log, and power projections)\n");
			break;

		case 'a':	/* -a option for aspatial field substitution into data columns */

			gmt_message (GMT, "\t-a Give one or more comma-separated <col>=<name> associations\n");
			gmt_message (GMT, "\t   [Default selects all aspatial fields].\n");
			break;

		case 'C':	/* -b binary option with input only */

			gmt_message (GMT, "\t-bi For binary input; [<n>]<type>[w][+l|b]; <type> = c|u|h|H|i|I|l|L|f|D.\n");
			break;

		case '0':	/* -bi/-bo addendum when input format is unknown */

			gmt_message (GMT, "\t    Prepend <n> for the number of columns for each <type>.\n");
			break;

		case '1':	/* -bi/-bo addendum when input format is unknown */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':

			gmt_message (GMT, "\t    Prepend <n> for the number of columns for each <type> in binary file(s) [%c].\n", options[k]);
			break;

		case 'D':	/* -b binary option with output only */

			gmt_message (GMT, "\t-bo For binary output; append <type>[w][+l|b]; <type> = c|u|h|H|i|I|l|L|f|D..\n");
			break;

		case 'c':	/* -c option advances subplot panel focus under modern mode */

			if (GMT->current.setting.run_mode == GMT_MODERN || GMT->current.setting.use_modern_name)	/* -c has no use in classic */
				gmt_message (GMT, "\t-c Move to next subplot panel or append row,col of desired panel.\n");
			break;

		case 'd':	/* -d option to tell GMT the relationship between NaN and a nan-proxy for input/output */

			gmt_message (GMT, "\t-d On input, replace <nodata> with NaN; on output do the reverse.\n");
			break;

		case 'e':	/* -e option for ASCII grep operation on data records */

			gmt_message (GMT, "\t-e Only accept input data records that contain the string \"pattern\".\n");
			gmt_message (GMT, "\t   Use -e~\"pattern\" to only accept data records that DO NOT contain this pattern.\n");
			gmt_message (GMT, "\t   If your pattern begins with ~, escape it with \\~.  To match against\n");
			gmt_message (GMT, "\t   extended regular expressions use -e[~]/regexp/[i] (i for case-insensitive).\n");
			gmt_message (GMT, "\t   Give +f<file> for a file list with such patterns, one per line.\n");
			gmt_message (GMT, "\t   To give a single pattern starting with +f, escape it with \\+f.\n");
			break;

		case 'k':	/* -di option to tell GMT the relationship between NaN and a nan-proxy for input */

			gmt_message (GMT, "\t-di Replace any <nodata> in input data with NaN.\n");
			break;

		case 'l':	/* -do option to tell GMT the relationship between NaN and a nan-proxy for output */

			gmt_message (GMT, "\t-do Replace any NaNs in output data with <nodata>.\n");
			break;

		case 'f':	/* -f option to tell GMT which columns are time (and optionally geographical) */

			gmt_message (GMT, "\t-f Special formatting of input/output columns (time or geographical).\n");
			gmt_message (GMT, "\t   Specify i(nput) or o(utput) [Default is both input and output].\n");
			gmt_message (GMT, "\t   Give one or more columns (or column ranges) separated by commas.\n");
			gmt_message (GMT, "\t   Append T (Calendar format), t (time relative to TIME_EPOCH),\n");
			gmt_message (GMT, "\t   f (floating point), x (longitude), y (latitude) to each item.\n");
			gmt_message (GMT, "\t   -f[i|o]g means -f[i|o]0x,1y (geographic coordinates).\n");
			gmt_message (GMT, "\t   -f[i|o]c means -f[i|o]0-1f (Cartesian coordinates).\n");
			gmt_message (GMT, "\t   -fp[<unit>] means input x,y are in projected coordinates.\n");
			break;

		case 'g':	/* -g option to tell GMT to identify data gaps based on point separation */

			gmt_message (GMT, "\t-g Use data point separations to determine if there are data gaps.\n");
			gmt_message (GMT, "\t   Append x|X or y|Y to identify data gaps in x or y coordinates,\n");
			gmt_message (GMT, "\t   respectively, and append d|D for distance gaps.  Upper case X|Y|D means\n");
			gmt_message (GMT, "\t   we first project the points (requires -J).  Append <gap>[unit][+n|p]; +n uses\n");
			gmt_message (GMT, "\t   d=prev-curr, +p uses d=curr-prev [d=|curr-prev|]; d must exceed <gap> to detect a gap.\n");
			gmt_message (GMT, "\t   For geographic data: choose from %s [Default is meter (%c)].\n", GMT_LEN_UNITS2_DISPLAY, GMT_MAP_DIST_UNIT);
			gmt_message (GMT, "\t   For gaps based on mapped coordinates: choose unit from %s [%s].\n",
			             GMT_DIM_UNITS_DISPLAY, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			gmt_message (GMT, "\t   For time data: the unit is controlled by TIME_UNIT.\n");
			gmt_message (GMT, "\t   For generic data: the unit is as the data implies (user units).\n");
			gmt_message (GMT, "\t   Repeat the -g option to specify multiple criteria, and add -ga\n");
			gmt_message (GMT, "\t   to indicate that all criteria must be met [just one must be met].\n");
			break;

		case 'h':	/* Header */

			gmt_message (GMT, "\t-h[i][<n>][+c][+d][+r<remark>][+t<title>] Input/output file has [%d] Header record(s) [%s]\n",
			             GMT->current.setting.io_n_header_items, GMT_choice[GMT->current.setting.io_header[GMT_IN]]);
			gmt_message (GMT, "\t   Optionally, append i for input only and/or number of header records [0].\n");
			gmt_message (GMT, "\t     -hi turns off the writing of all headers on output.\n");
			gmt_message (GMT, "\t   Append +c to add header record with column information [none].\n");
			gmt_message (GMT, "\t   Append +d to delete headers before adding new ones [Default will append headers].\n");
			gmt_message (GMT, "\t   Append +r to add a <remark> comment to the output [none].\n");
			gmt_message (GMT, "\t   Append +t to add a <title> comment to the output [none].\n");
			gmt_message (GMT, "\t     (these strings may contain \\n to indicate line-breaks)\n");
			gmt_message (GMT, "\t   For binary files, <n> is considered to mean number of bytes.\n");
			break;

		case 'i':	/* -i option for input column order */

			gmt_message (GMT, "\t-i Sets alternate input column order and optional transformations [Default reads all columns in order].\n");
			gmt_message (GMT, "\t   Append list of columns; t = trailing text. Use -in for just numerical input.\n");
			break;

		case 'A':	/* -j option for spherical distance calculation mode */

			gmt_message (GMT, "\t-j Sets spherical distance calculation mode for modules that offer that flexibility.\n");
			gmt_message (GMT, "\t   Append f for Flat Earth, g for Great Circle [Default], and e for Ellipsoidal mode.\n");
			break;

		case 'n':	/* -n option for grid resampling parameters in BCR */

			gmt_message (GMT, "\t-n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>] Specify the grid interpolation mode.\n");
			gmt_message (GMT, "\t   (b = B-spline, c = bicubic, l = bilinear, n = nearest-neighbor) [Default is bicubic].\n");
			gmt_message (GMT, "\t   Append +a to switch off antialiasing (except for l) [Default: on].\n");
			gmt_message (GMT, "\t   Append +b<BC> to change boundary conditions.  <BC> can be either:\n");
			gmt_message (GMT, "\t     g for geographic, p for periodic, and n for natural boundary conditions.\n");
			gmt_message (GMT, "\t     For p and n you may optionally append x or y [default is both]:\n");
			gmt_message (GMT, "\t       x for periodic boundary conditions on x,\n");
			gmt_message (GMT, "\t       y for periodic boundary conditions on y.\n");
			gmt_message (GMT, "\t   [Default: Natural conditions, unless grid is known to be geographic].\n");
			gmt_message (GMT, "\t   Append +c to clip interpolated grid to input z-min/max [Default may exceed limits].\n");
			gmt_message (GMT, "\t   Append +t<threshold> to change the minimum weight in vicinity of NaNs. A threshold of\n");
			gmt_message (GMT, "\t   1.0 requires all nodes involved in interpolation to be non-NaN; 0.5 will interpolate\n");
			gmt_message (GMT, "\t   about half way from a non-NaN to a NaN node [Default: 0.5].\n");
			break;

		case 'o':	/* -o option for output column order */

			gmt_message (GMT, "\t-o Set alternate output column order [Default writes all columns in order].\n");
			gmt_message (GMT, "\t   Append list of columns; t = trailing text. Use -on for just numerical output.\n");
			break;

		case 'p':	/* Enhanced pseudo-perspective 3-D plot settings */
		case 'E':	/* GMT4: For backward compatibility */
			if (gmt_M_compat_check (GMT, 4) || options[k] == 'p') {
				gmt_message (GMT, "\t-%c Select a 3-D pseudo perspective view.  Append the\n", options[k]);
				gmt_message (GMT, "\t   <azimuth>/<elevation> of the viewpoint [180/90].\n");
				gmt_message (GMT, "\t   When used with -Jz|Z, optionally add /<zlevel> for frame level [bottom of z-axis].\n");
				gmt_message (GMT, "\t   Prepend x or y to plot against the “wall” x = level or y = level [z].\n");
				gmt_message (GMT, "\t   Optionally, append +w<lon0>/<lat0>[/<z0>] to specify a fixed coordinate point\n");
				gmt_message (GMT, "\t   or +v<x0>/<y0> for a fixed projected point [region center and page center].\n");
				gmt_message (GMT, "\t   For a plain rotation about the z-axis, give rotation angle only\n");
				gmt_message (GMT, "\t   and optionally use +w or +v to select location of axis [plot origin].\n");
			}
			break;

		case 's':	/* Output control for records where z are NaN */

			gmt_message (GMT, "\t-s Suppress output for records whose z-value (col = 2) equals NaN\n");
			gmt_message (GMT, "\t   [Default prints all records].\n");
			gmt_message (GMT, "\t   Append <cols> to examine other column(s) instead [2].\n");
			gmt_message (GMT, "\t   Append +a to suppress records where any column equals NaN, or\n");
			gmt_message (GMT, "\t   append +r to reverse the suppression (only output z = NaN records).\n");
			break;

		case 'F':	/* -r Pixel registration option  */

			gmt_message (GMT, "\t-r Set (g)ridline- or (p)ixel-registration [Default].\n");
			gmt_message (GMT, "\t   If not given we default to gridline registration\n");
			break;

		case 't':	/* -t layer transparency option  */

			gmt_message (GMT, "\t-t Set the layer PDF transparency from 0-100 [Default is 0; opaque].\n");
			break;

		case ':':	/* lon/lat [x/y] or lat/lon [y/x] */

			gmt_message (GMT, "\t-: Swap 1st and 2nd column on input and/or output [%s/%s].\n",
			             GMT_choice[GMT->current.setting.io_lonlat_toggle[GMT_IN]], GMT_choice[GMT->current.setting.io_lonlat_toggle[GMT_OUT]]);
			break;

		case '.':	/* Trailer message */

			gmt_message (GMT, "\t-^ (or -) Print short synopsis message.\n");
			gmt_message (GMT, "\t-+ (or +) Print longer synopsis message.\n");
			gmt_message (GMT, "\t-? (or no arguments) Print this usage message.\n");
			gmt_message (GMT, "\t--PAR=<value> Temporarily override GMT default setting(s) (repeatable).\n");
			gmt_message (GMT, "\t(See gmt.conf man page for GMT default parameters).\n");
			break;

		case '<':	/* Table input */

			gmt_message (GMT, "\t<table> is one or more data files (in ASCII, binary, netCDF).\n");
			gmt_message (GMT, "\t   If no files are given, standard input is read.\n");
			break;

		default:	/* Pass through, no error, might be things like -y not available */
			break;
		}
	}
}

/*! GSHHG subset specification */
/*!
	\param GMT ...
	\param option ...
*/
void gmt_GSHHG_syntax (struct GMT_CTRL *GMT, char option) {
 	gmt_message (GMT, "\t-%c Place limits on coastline features from the GSHHG data base.\n", option);
	gmt_message (GMT, "\t   Features smaller than <min_area> (in km^2) or of levels (0-4) outside the min-max levels\n");
	gmt_message (GMT, "\t   will be skipped [0/4 (4 means lake inside island inside lake)].\n");
	gmt_message (GMT, "\t   Select +a and one or two codes to control how Antarctica is handled:\n");
	gmt_message (GMT, "\t     Add g to use shelf ice grounding line for Antarctica coastline, or\n");
	gmt_message (GMT, "\t     Add i to use ice/water front for Antarctica coastline [Default].\n");
	gmt_message (GMT, "\t     Add s to skip Antarctica (all data south of %dS) [use all], or\n", abs(GSHHS_ANTARCTICA_LIMIT));
	gmt_message (GMT, "\t     Add S to skip anything BUT Antarctica (all data north of %dS) [use all].\n", abs(GSHHS_ANTARCTICA_LIMIT));
	gmt_message (GMT, "\t   Append +r to only get riverlakes from level 2, or +l to only get lakes [both].\n");
	gmt_message (GMT, "\t   Append +p<percent> to exclude features whose size is < <percent>%% of the full-resolution feature [use all].\n");
}

/*! Contour/line specifications in *contour and psxy[z] */
/*!
	\param GMT ...
	\param indent The number of spaces to indent after the TAB
	\param kind  kind = 0 for *contour and 1 for psxy[z]
*/
void gmt_label_syntax (struct GMT_CTRL *GMT, unsigned int indent, unsigned int kind) {
	unsigned int i;
	char pad[16], *type[3] = {"Contour", "Line", "Decorated line"};
	char *feature[3] = {"label", "label", "symbol"};

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	gmt_message (GMT, "%s +a<angle> will place all %s at a fixed angle.\n", pad, feature[kind]);
	gmt_message (GMT, "%s Or, specify +an (line-normal) or +ap (line-parallel) [Default].\n", pad);
	if (kind == 0) {
		gmt_message (GMT, "%s   For +ap, you may optionally append u for up-hill\n", pad);
		gmt_message (GMT, "%s   and d for down-hill cartographic annotations.\n", pad);
	}
	if (kind < 2) gmt_message (GMT, "%s +c<dx>[/<dy>] sets clearance between label and text box [15%%].\n", pad);
	gmt_message (GMT, "%s +d turns on debug which draws helper points and lines.\n", pad);
	if (kind < 2) gmt_message (GMT, "%s +e delays the plotting of the text as text clipping is set instead.\n", pad);
	if (kind < 2) gmt_message (GMT, "%s +f sets specified label font [Default is %s].\n", pad, gmt_putfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]));
	if (kind < 2)
		gmt_message (GMT, "%s +g[<color>] paints text box [transparent]; append color [white].\n", pad);
	else
		gmt_message (GMT, "%s +g<fill> sets the fill for the symbol [transparent]\n", pad);
	if (kind == 1) gmt_message (GMT, "%s +h hide the lines [Draw the lines].\n", pad);
	if (kind < 2) gmt_message (GMT, "%s +j<just> sets %s justification [Default is MC].\n", pad, feature[kind]);
	if (kind == 1) {
		gmt_message (GMT, "%s +l<text> Use text as label (quote text if containing spaces).\n", pad);
		gmt_message (GMT, "%s +L<d|D|f|h|n|N|x>[<unit>] Sets label according to given flag:\n", pad);
		gmt_message (GMT, "%s   d Cartesian plot distance; append a desired unit from %s.\n", pad, GMT_DIM_UNITS_DISPLAY);
		gmt_message (GMT, "%s   D Map distance; append a desired unit from %s.\n", pad, GMT_LEN_UNITS_DISPLAY);
		gmt_message (GMT, "%s   f Label is last column of given label location file.\n", pad);
		gmt_message (GMT, "%s   h Use segment header labels (via -Lstring).\n", pad);
		gmt_message (GMT, "%s   n Use the current segment number (starting at 0).\n", pad);
		gmt_message (GMT, "%s   N Use current file number / segment number (starting at 0/0).\n", pad);
		gmt_message (GMT, "%s   x Like h, but us headers in file with crossing lines instead.\n", pad);
	}
	if (kind < 2)
		gmt_message (GMT, "%s +n<dx>[/<dy>] to nudge label along line (+N for along x/y axis); ignored with +v.\n", pad);
	else
		gmt_message (GMT, "%s +n<dx>[/<dy>] to nudge symbol along line (+N for along x/y axis).\n", pad);
	if (kind < 2)gmt_message (GMT, "%s +o to use rounded rectangular text box [Default is rectangular].\n", pad);
	gmt_message (GMT, "%s +p[<pen>] draw outline of textbox [Default is no outline].\n", pad);
	gmt_message (GMT, "%s   Optionally append a pen [Default is default pen].\n", pad);
	if (kind == 2) gmt_message (GMT, "%s +s<symbol><size> specifies the decorative symbol and its size.\n", pad);
	if (kind < 2) {
		gmt_message (GMT, "%s +r<rmin> skips labels where radius of curvature < <rmin> [0].\n", pad);
		gmt_message (GMT, "%s +t[<file>] saves (x y angle label) to <file> [%s_labels.txt].\n", pad, type[kind%2]);
		gmt_message (GMT, "%s +u<unit> to append unit to all labels.\n", pad);
	}
	if (kind == 0) gmt_message (GMT, "%s  If z is appended we use the z-unit from the grdfile [no unit].\n", pad);
	if (kind < 2) gmt_message (GMT, "%s +v for placing curved text along path [Default is straight].\n", pad);
	gmt_message (GMT, "%s +w sets how many (x,y) points to use for angle calculation [auto].\n", pad);
	if (kind == 1) {
		gmt_message (GMT, "%s +x[first,last] adds <first> and <last> to these two labels [,'].\n", pad);
		gmt_message (GMT, "%s   This modifier is only allowed if -SqN2 is used.\n", pad);
	}
	if (kind < 2) gmt_message (GMT, "%s +=<prefix> to give all labels a prefix.\n", pad);
}

/*! Contour/line label placement specifications in *contour and psxy[z] */
/*!
	\param GMT ...
	\param indent The number of spaces to indent after the TAB
	\param kind  kind = 0 for *contour and 1 for psxy[z]
*/
void gmt_cont_syntax (struct GMT_CTRL *GMT, unsigned int indent, unsigned int kind) {
	unsigned int i;
	double gap;
	char pad[16];
	char *type[3] = {"contour", "quoted line", "decorated line"};
	char *feature[3] = {"label", "label", "symbol"};

	gap = 4.0 * GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];

	pad[0] = '\t';	for (i = 1; i <= indent; i++) pad[i] = ' ';	pad[i] = '\0';
	gmt_message (GMT, "%sd<dist>[%s] or D<dist>[%s]  [Default is d%g%c].\n",
		pad, GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY, gap, GMT->session.unit_name[GMT->current.setting.proj_length_unit][0]);
	gmt_message (GMT, "%s   d: Give distance between %ss with specified map unit in %s.\n", pad, feature[kind], GMT_DIM_UNITS_DISPLAY);
	gmt_message (GMT, "%s   D: Specify geographic distance between %ss in %s,\n", pad, feature[kind], GMT_LEN_UNITS_DISPLAY);
	gmt_message (GMT, "%s   The first %s appears at <frac>*<dist>; change by appending /<frac> [0.25].\n", pad, feature[kind]);
	gmt_message (GMT, "%sf<file.d> reads file <file.d> and places %ss at locations\n", pad, feature[kind]);
	gmt_message (GMT, "%s   that match individual points along the %ss.\n", pad, type[kind]);
	gmt_message (GMT, "%sl|L<line1>[,<line2>,...] Give start and stop coordinates for\n", pad);
	gmt_message (GMT, "%s   straight line segments; %ss will be placed where these\n", pad, feature[kind]);
	gmt_message (GMT, "%s   lines intersect %ss.  The format of each <line>\n", pad, type[kind]);
	gmt_message (GMT, "%s   is <start>/<stop>, where <start> or <stop> = <lon/lat> or a\n", pad);
	gmt_message (GMT, "%s   2-character XY key that uses the \"pstext\"-style justification\n", pad);
	gmt_message (GMT, "%s   format to specify a point on the map as [LCR][BMT].\n", pad);
	if (kind == 0) {
		gmt_message (GMT, "%s   In addition, you can use Z-, Z+ to mean the global\n", pad);
		gmt_message (GMT, "%s   minimum and maximum locations in the grid.\n", pad);
	}
	gmt_message (GMT, "%s   L Let point pairs define great circles [Straight lines].\n", pad);
	gmt_message (GMT, "%sn|N<n_%s> sets number of equidistant %ss per %s.\n", pad, feature[kind], feature[kind], type[kind]);
	gmt_message (GMT, "%s   N: Starts %s exactly at the start of %s\n", pad, feature[kind], type[kind]);
	gmt_message (GMT, "%s     [Default centers the %ss on the %s].\n", pad, feature[kind], type[kind]);
	gmt_message (GMT, "%s   N-1 places a single %s at start of the %s, while\n", pad, feature[kind], type[kind]);
	gmt_message (GMT, "%s   N+1 places a single %s at the end of the %s.\n", pad, feature[kind], type[kind]);
	gmt_message (GMT, "%s   Append /<min_dist> to enforce a minimum spacing between\n", pad);
	gmt_message (GMT, "%s   consecutive %ss [0]\n", pad, feature[kind]);
	if (kind == 1) {
		gmt_message (GMT, "%ss|S<n_%s> sets number of equidistant %s per segmented %s.\n", pad, feature[kind], feature[kind], type[kind]);
		gmt_message (GMT, "%s   Same as n|N but splits input lines to series of 2-point segments first.\n", pad);
	}
	gmt_message (GMT, "%sx|X<xfile.d> reads the multi-segment file <xfile.d> and places\n", pad);
	gmt_message (GMT, "%s   %ss at intersections between %ss and lines in\n", pad, feature[kind], type[kind]);
	gmt_message (GMT, "%s   <xfile.d>.  Use X to resample the lines first.\n", pad);
	if (kind < 2) {
		gmt_message (GMT, "%s   For all options, append +r<radius>[unit] to specify minimum\n", pad);
		gmt_message (GMT, "%s   radial separation between labels [0]\n", pad);
	}
}

/*! Widely used in most programs that need grid increments to be set */
/*!
	\param GMT ...
	\param option ...
	\param error ...
*/
void gmt_inc_syntax (struct GMT_CTRL *GMT, char option, bool error) {
	if (error) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c Specify increment(s) and optionally append units or flags.\n", option);
	gmt_message (GMT, "\t   Full syntax is <xinc>[%s][+e|n][/<yinc>[%s][+e|n]]]\n", GMT_LEN_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY);
	gmt_message (GMT, "\t   For geographic regions in degrees you can optionally append units from this list:\n");
	gmt_message (GMT, "\t   (d)egree [Default], (m)inute, (s)econd, m(e)ter, (f)oot, (k)ilometer, (M)ile, (n)autical mile, s(u)rvey foot.\n");
	gmt_message (GMT, "\t   Append +e to adjust the region to fit increments [Adjust increment to fit domain].\n");
	gmt_message (GMT, "\t   Alternatively, specify number of nodes by appending +n. Then, the increments\n");
	gmt_message (GMT, "\t   are calculated from the given domain and node-registration settings\n");
	gmt_message (GMT, "\t   (see Appendix B for details).  Note: If -R<grdfile> was used then -%c\n", option);
	gmt_message (GMT, "\t   (and -R and maybe -r) have been set; use -%c to override those increments.\n", option);
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_fill_syntax (struct GMT_CTRL *GMT, char option, char *string) {
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c<fill> %s Specify <fill> as one of:\n", option, string);
	gmt_message (GMT, "\t   1) <gray> or <red>/<green>/<blue>, all in the range 0-255;\n");
	gmt_message (GMT, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	gmt_message (GMT, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	gmt_message (GMT, "\t   4) any valid color name;\n");
	gmt_message (GMT, "\t   5) P|p<pattern>[+b<color>][+f<color>][+r<dpi>];\n");
	gmt_message (GMT, "\t      Give <pattern> number from 1-90 or a filename, optionally add +r<dpi> [300].\n");
	gmt_message (GMT, "\t      Optionally, use +f,+b to change fore- or background colors (set - for transparency).\n");
	gmt_message (GMT, "\t   For PDF fill transparency, append @<transparency> in the range 0-100 [0 = opaque].\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_pen_syntax (struct GMT_CTRL *GMT, char option, char *string, unsigned int mode) {
	/* mode = 1 (bezier option), 2 = end trim, 4 = vector heads, 7 = all, 8 = CPT interactions */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c ", option);
	if (strstr (string, "%s"))
		gmt_message (GMT, string, gmt_putpen (GMT, &GMT->current.setting.map_default_pen));
	else
		gmt_message (GMT, string);
	gmt_message (GMT, "\n\t   <pen> is a comma-separated list of three optional items in the order:\n");
	gmt_message (GMT, "\t       <width>[%s], <color>, and <style>[%s].\n", GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
	gmt_message (GMT, "\t   <width> >= 0.0 sets pen width (default units are points); alternatively a pen\n");
	gmt_message (GMT, "\t       name: Choose among faint, default, or [thin|thick|fat][er|est], or obese.\n");
	gmt_message (GMT, "\t   <color> = (1) <gray> or <red>/<green>/<blue>, all in range 0-255,\n");
	gmt_message (GMT, "\t             (2) <c>/<m>/<y>/<k> in 0-100%% range,\n");
	gmt_message (GMT, "\t             (3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1,\n");
	gmt_message (GMT, "\t             (4) any valid color name.\n");
	gmt_message (GMT, "\t   <style> = (1) pattern of dashes (-) and dots (.), scaled by <width>.\n");
	gmt_message (GMT, "\t             (2) \"dashed\", \"dotted\", \"dashdot\", \"dotdash\", or \"solid\".\n");
	gmt_message (GMT, "\t             (3) <pattern>:<offset>; <pattern> holds lengths (default unit points)\n");
	gmt_message (GMT, "\t                 of any number of lines and gaps separated by underscores.\n");
	gmt_message (GMT, "\t                 <offset> shifts elements from start of the line [0].\n");
	gmt_message (GMT, "\t   For PDF stroke transparency, append @<transparency> in the range 0-100%% [0 = opaque].\n");
	if (mode)
		gmt_message (GMT, "\t   Additional line attribute modifiers are also available.  Choose from:\n");
	if (mode & 2) {
		gmt_message (GMT, "\t     +o<offset>[unit] Trim the line from the end inward by the specified amount.\n");
		gmt_message (GMT, "\t        Choose <unit> as plot distances (%s) or map distances (%s) [Cartesian].\n", GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY);
		gmt_message (GMT, "\t        To trim the two ends differently, give two offsets separated by a slash (/).\n");
	}
	if (mode & 1)
		gmt_message (GMT, "\t     +s Draw line using a Bezier spline in the PostScript [Linear spline].\n");
	if (mode & 4) {
		gmt_message (GMT, "\t     +v[b|e]<vecspecs> Add vector head with the given specs at the ends of lines.\n");
		gmt_message (GMT, "\t        Use +ve and +vb separately to give different endings (+v applies to both).\n");
		gmt_message (GMT, "\t        See vector specifications for details. Note: +v must be last modifier for a pen.\n");
	}
	if (mode & 8) {
		gmt_message (GMT, "\t     +c Controls how pens and fills are affected if a CPT is specified via -C:\n");
		gmt_message (GMT, "\t          Append l to let pen colors follow the CPT setting.\n");
		gmt_message (GMT, "\t          Append f to let fill/font colors follow the CPT setting.\n");
		gmt_message (GMT, "\t          Default activates both effects.\n");
	}
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_rgb_syntax (struct GMT_CTRL *GMT, char option, char *string) {
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c<color> %s Specify <color> as one of:\n", option, string);
	gmt_message (GMT, "\t   1) <gray> or <red>/<green>/<blue>, all in range 0-255;\n");
	gmt_message (GMT, "\t   2) <c>/<m>/<y>/<k> in range 0-100%%;\n");
	gmt_message (GMT, "\t   3) <hue>-<sat>-<val> in ranges 0-360, 0-1, 0-1;\n");
	gmt_message (GMT, "\t   4) any valid color name.\n");
	gmt_message (GMT, "\t   For PDF fill transparency, append @<transparency> in the range 0-100%% [0 = opaque].\n");
}


void gmt_refpoint_syntax (struct GMT_CTRL *GMT, char *option, char *string, unsigned int kind, unsigned int part) {
	/* For -Dg|j|J|n|x */
	char *type[GMT_ANCHOR_NTYPES] = {"logo", "image", "legend", "color-bar", "inset", "map scale", "map rose", "vertical scale"}, *tab[2] = {"", "     "};
	unsigned int shift = (kind == GMT_ANCHOR_INSET) ? 1 : 0;	/* Add additional "tab" to front of message */
	if (part & 1) {	/* Here string is message, or NULL */
		if (string) gmt_message (GMT, "\t-%s %s\n", option, string);
		gmt_message (GMT, "\t   %sPositioning is specified via one of four coordinate systems:\n", tab[shift]);
		gmt_message (GMT, "\t   %s  Use -%sg to specify <refpoint> with map coordinates.\n", tab[shift], option);
		gmt_message (GMT, "\t   %s  Use -%sj or -%sJ to specify <refpoint> with 2-char justification code (BL, MC, etc).\n", tab[shift], option, option);
		gmt_message (GMT, "\t   %s  Use -%sn to specify <refpoint> with normalized coordinates in 0-1 range.\n", tab[shift], option);
		gmt_message (GMT, "\t   %s  Use -%sx to specify <refpoint> with plot coordinates.\n", tab[shift], option);
		gmt_message (GMT, "\t   %sAll except -%sx require the -R and -J options to be set.\n", tab[shift], option);
		gmt_message (GMT, "\t   %sUse J if item should be placed outside the map frame and j if inside.\n", tab[shift]);
	}
	/* May need to place other things in the middle */
	if (part & 2) {	/* Here string is irrelevant */
		char *just[GMT_ANCHOR_NTYPES] = {"BL", "BL", "BL", "BL", "BL", "MC", "MC", "ML"};
		gmt_message (GMT, "\t   %sAppend 2-char +j<justify> code to associate that anchor point on the %s with <refpoint>.\n", tab[shift], type[kind]);
		gmt_message (GMT, "\t   %sIf +j<justify> is not given then <justify> will default to the same as <refpoint> (with -%sj),\n", tab[shift], option);
		gmt_message (GMT, "\t   %s  or the mirror opposite of <refpoint> (with -%sJ), or %s (else).\n", tab[shift], option, just[kind]);
		gmt_message (GMT, "\t   %sOptionally, append +o<dx>[/<dy>] to offset %s from <refpoint> in direction implied by <justify> [0/0].\n", tab[shift], type[kind]);
	}
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_mapinset_syntax (struct GMT_CTRL *GMT, char option, char *string) {
	/* Only called in psbasemap.c for now */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c %s\n", option, string);
	gmt_message (GMT, "\t     Specify the map inset region using one of three specifications:\n");
	gmt_message (GMT, "\t     a) Give <west>/<east>/<south>/<north> of geographic rectangle bounded by meridians and parallels.\n");
	gmt_message (GMT, "\t        Append +r if coordinates are the lower left and upper right corners of a rectangular area.\n");
	gmt_message (GMT, "\t     b) Give <xmin>/<xmax>/<ymin>/<ymax>+u<unit> of bounding rectangle in projected coordinates.\n");
	gmt_message (GMT, "\t     c) Set reference point and dimensions of the inset:\n");
	gmt_refpoint_syntax (GMT, "D", NULL, GMT_ANCHOR_INSET, 1);
	gmt_message (GMT, "\t        Append +w<width>[<u>]/<height>[<u>] of bounding rectangle (<u> is unit).\n");
	gmt_refpoint_syntax (GMT, "D", NULL, GMT_ANCHOR_INSET, 2);
	gmt_message (GMT, "\t     Append +s<file> to save inset lower left corner and dimensions to <file>.\n");
	gmt_message (GMT, "\t     Append +t to translate plot origin to the lower left corner of the inset.\n");
	gmt_message (GMT, "\t   Set panel attributes separately via the -F option.\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_mapscale_syntax (struct GMT_CTRL *GMT, char option, char *string) {
	/* Used in psbasemap and pscoast */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c %s\n", option, string);
	gmt_refpoint_syntax (GMT, "L", NULL, GMT_ANCHOR_MAPSCALE, 3);
	gmt_message (GMT, "\t   Use +c<slat> (with central longitude) or +c<slon>/<slat> to specify scale origin for geographic projections.\n");
	gmt_message (GMT, "\t   Set scale length with +w<length> and (for geo projection) append a unit from %s [km].\n", GMT_LEN_UNITS2_DISPLAY);
	gmt_message (GMT, "\t   Several modifiers are optional:\n");
	gmt_message (GMT, "\t   Add +f to draw a \"fancy\" scale [Default is plain].\n");
	gmt_message (GMT, "\t   By default, the scale label equals the distance unit name and is placed on top [+at].  Use the +l<label>\n");
	gmt_message (GMT, "\t   and +a<align> mechanisms to specify another label and placement (t,b,l,r).  For the fancy scale,\n");
	gmt_message (GMT, "\t   +u appends units to annotations while for plain scale it uses unit abbreviation instead of name as label.\n");
	gmt_message (GMT, "\t   To get a vertical scale instead for Cartesian plots, append +v.\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_maprose_syntax (struct GMT_CTRL *GMT, char option, char *string) {
	/* Used in psbasemap and pscoast */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c %s\n", option, string);
	gmt_message (GMT, "\t   Choose between a directional (-Td) or magnetic (-Tm) rose.\n");
	gmt_message (GMT, "\t   Both share most modifiers for locating and sizing the rose.\n");
	gmt_refpoint_syntax (GMT, "Td|m", NULL, GMT_ANCHOR_MAPROSE, 3);
	gmt_message (GMT, "\t   Set the diameter of the rose with modifier +w<width>.\n");
	gmt_message (GMT, "\t   Several modifiers are optional:\n");
	gmt_message (GMT, "\t   Add labels with +l, which places the letters W, E, S, N at the cardinal points.\n");
	gmt_message (GMT, "\t     Optionally, append comma-separated west, east, south, north labels instead.\n");
	gmt_message (GMT, "\t   Directional rose: Add +f to draws a \"fancy\" rose [Default is plain].\n");
	gmt_message (GMT, "\t     Optionally, add <level> of fancy rose: 1 draws E-W, N-S directions [Default],\n");
	gmt_message (GMT, "\t     2 adds NW-SE and NE-SW, while 3 adds WNW-ESE, NNW-SSE, NNE-SSW, and ENE-WSW directions.\n");
	gmt_message (GMT, "\t   Magnetic compass rose:  Optional add +d<dec>[/<dlabel>], where <dec> is the\n");
	gmt_message (GMT, "\t     magnetic declination and <dlabel> is an optional label for the magnetic compass needle.\n");
	gmt_message (GMT, "\t     If +d does not include <dlabel> we default to \"delta = <declination>\".\n");
	gmt_message (GMT, "\t     Set <dlabel> to \"-\" to disable the declination label.\n");
	gmt_message (GMT, "\t     Append +p<pen> to draw outline of secondary (outer) circle [no circle].\n");
	gmt_message (GMT, "\t     Append +i<pen> to draw outline of primary (inner) circle [no circle].\n");
	gmt_message (GMT, "\t     Append +t<pint>[/<sint>] to override default primary and secondary annotation/tick interval(s) [30/5/1].\n");
	gmt_message (GMT, "\t   If the North label = \'*\' then a north star is plotted instead of the label.\n");
}

/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_mappanel_syntax (struct GMT_CTRL *GMT, char option, char *string, unsigned int kind) {
	/* Called by gmtlogo, psimage, pslegend, psscale */
	char *type[5] = {"logo", "image", "legend", "scale", "vertical scale"};
	assert (kind < 5);
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c %s\n", option, string);
	gmt_message (GMT, "\t   Without further options: draw border around the %s panel (using MAP_FRAME_PEN)\n", type[kind]);
	gmt_message (GMT, "\t     [Default is no border].\n");
	gmt_message (GMT, "\t   Append +c<clearance> where <clearance> is <gap>, <xgap/ygap>, or <lgap/rgap/bgap/tgap> [%gp].\n", GMT_FRAME_CLEARANCE);
	gmt_message (GMT, "\t     Note: For a map inset the default clearance is zero.\n");
#ifdef DEBUG
	gmt_message (GMT, "\t   Append +d to draw guide lines for debugging.\n");
#endif
	gmt_message (GMT, "\t   Append +g<fill> to set the fill for the %s panel [Default is no fill].\n", type[kind]);
	gmt_message (GMT, "\t   Append +i[[<gap>/]<pen>] to add a secondary inner frame boundary [Default gap is %gp].\n", GMT_FRAME_GAP);
	gmt_message (GMT, "\t   Append +p[<pen>] to draw the border and optionally change the border pen [%s].\n",
		gmt_putpen (GMT, &GMT->current.setting.map_frame_pen));
	gmt_message (GMT, "\t   Append +r[<radius>] to plot rounded rectangles instead [Default radius is %gp].\n", GMT_FRAME_RADIUS);
	gmt_message (GMT, "\t   Append +s[<dx>/<dy>/][<shade>] to plot a shadow behind the %s panel [Default is %gp/%g/gray50].\n", type[kind], GMT_FRAME_CLEARANCE, -GMT_FRAME_CLEARANCE);
}
/*! .
	\param GMT ...
	\param option ...
	\param string ...
*/
void gmt_dist_syntax (struct GMT_CTRL *GMT, char option, char *string) {
	/* Used by many modules */
	if (string[0] == ' ') GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option.  Correct syntax:\n", option);
	gmt_message (GMT, "\t-%c %s\n", option, string);
	gmt_message (GMT, "\t   Append e (meter), f (foot), k (km), M (mile), n (nautical mile), u (survey foot)\n");
	gmt_message (GMT, "\t   d (arc degree), m (arc minute), or s (arc second) [%c].\n", GMT_MAP_DIST_UNIT);
	gmt_message (GMT, "\t   Spherical distances are based on great-circle calculations;\n");
	gmt_message (GMT, "\t   see -j<mode> for other modes of measurements.\n");
}

/*! Use mode to control which options are displayed */
void gmt_vector_syntax (struct GMT_CTRL *GMT, unsigned int mode) {
	/* Mode is composed of bit-flags to control which lines are printed.
	 * Items without if-test are common to all vectors.
	 * 1	= Accepts +j (not mathangle)
	 * 2	= Accepts +s (not mathangle)
	 * 4	= Accepts +p (not mathangle)
	 * 8	= Accepts +g (not mathangle)
	 * 16	= Accepts +z (not mathangle, geovector)
	 */
	gmt_message (GMT, "\t   Append length of vector head, with optional modifiers:\n");
	gmt_message (GMT, "\t   [Left and right are defined by looking from start to end of vector]\n");
	gmt_message (GMT, "\t     +a<angle> to set angle of the vector head apex [30]\n");
	gmt_message (GMT, "\t     +b to place a vector head at the beginning of the vector [none].\n");
	gmt_message (GMT, "\t       Append t for terminal, c for circle, s for square, a for arrow [Default],\n");
	gmt_message (GMT, "\t       i for tail, A for plain arrow, and I for plain tail.\n");
	gmt_message (GMT, "\t       Append l|r to only draw left or right side of this head [both sides].\n");
	gmt_message (GMT, "\t     +e to place a vector head at the end of the vector [none].\n");
	gmt_message (GMT, "\t       Append t for terminal, c for circle, s for square, a for arrow [Default],\n");
	gmt_message (GMT, "\t       i for tail, A for plain arrow, and I for plain tail.\n");
	gmt_message (GMT, "\t       Append l|r to only draw left or right side of this head [both sides].\n");
	if (mode & 8) gmt_message (GMT, "\t     +g<fill> to set head fill or use - to turn off fill [default fill].\n");
	gmt_message (GMT, "\t     +h sets the vector head shape in -2/2 range [%g].\n", GMT->current.setting.map_vector_shape);
	if (mode & 1) gmt_message (GMT, "\t     +j<just> to justify vector at (b)eginning [default], (e)nd, or (c)enter.\n");
	gmt_message (GMT, "\t     +l to only draw left side of all specified vector heads [both sides].\n");
	gmt_message (GMT, "\t     +m[f|r] to place vector head at mid-point of segment [Default expects +b|+e].\n");
	gmt_message (GMT, "\t       Specify f or r for forward|reverse direction [forward].\n");
	gmt_message (GMT, "\t       Append t for terminal, c for circle, s for square, or a for arrow [Default].\n");
	gmt_message (GMT, "\t       Append l|r to only draw left or right side of this head [both sides].\n");
	gmt_message (GMT, "\t     +n<norm> to shrink attributes if vector length < <norm> [none].\n");
	gmt_message (GMT, "\t     +o[<plon/plat>] sets pole [north pole] for great or small circles; only give length via input.\n");
	if (mode & 4) gmt_message (GMT, "\t     +p[-][<pen>] to set pen attributes, prepend - to turn off head outlines [default pen and outline].\n");
	gmt_message (GMT, "\t     +q if start and stop opening angle is given instead of (azimuth,length) on input.\n");
	gmt_message (GMT, "\t     +r to only draw right side of all specified vector heads [both sides].\n");
	if (mode & 2) gmt_message (GMT, "\t     +s if (x,y) coordinates of tip is given instead of (azimuth,length) on input.\n");
	gmt_message (GMT, "\t     +t[b|e]<trim(s)>[unit] to shift begin or end position along vector by given amount [no shifting].\n");
	if (mode & 16) gmt_message (GMT, "\t     +z if (dx,dy) vector components are given instead of (azimuth,length) on input.\n");
	if (mode & 16) gmt_message (GMT, "\t       Append <scale>[unit] to convert components to length in given unit.\n");
}

/*! Use mode to control which options are displayed */
void gmt_segmentize_syntax (struct GMT_CTRL *GMT, char option, unsigned int mode) {
	/* mode == 0 for formatting and 1 for plotting */
	char *verb[2] = {"Form", "Draw"}, *count[2] = {"four", "three"};
	gmt_message (GMT, "\t-%c Alter the way points are connected and the data are segmented.\n", option);
	gmt_message (GMT, "\t    Append one of %s line connection schemes:\n", count[mode]);
	gmt_message (GMT, "\t     c: %s continuous line segments for each group [Default].\n", verb[mode]);
	gmt_message (GMT, "\t     r: %s line segments from a reference point reset for each group.\n", verb[mode]);
	gmt_message (GMT, "\t     n: %s networks of line segments between all points in each group.\n", verb[mode]);
	if (mode == 0) gmt_message (GMT, "\t     v: Form vector line segments suitable for psxy -Sv|=<size>+s\n");
	gmt_message (GMT, "\t     Optionally, append one of five ways to define a \"group\":\n");
	gmt_message (GMT, "\t       a: All data is consider a single group; reference point is first point in the group.\n");
	gmt_message (GMT, "\t       f: Each file is a separate group; reference point is reset to first point in the group.\n");
	gmt_message (GMT, "\t       s: Each segment is a group; reference point is reset to first point in the group [Default].\n");
	gmt_message (GMT, "\t       r: Each segment is a group, but reference point is reset to each point in the group.\n");
	gmt_message (GMT, "\t          Only available with the -%cr scheme.\n", option);
	gmt_message (GMT, "\t       <refpoint> : Specify a fixed external reference point instead.\n");
}

/*! For programs that can read *.img grids */
void gmt_img_syntax (struct GMT_CTRL *GMT) {
	gmt_message (GMT, "\t      Give filename and append comma-separated scale, mode, and optionally max latitude.\n");
	gmt_message (GMT, "\t      The scale (typically 0.1 or 1) is used to multiply after read; give mode as follows:\n");
	gmt_message (GMT, "\t        0 = img file with no constraint code, interpolate to get data at track.\n");
	gmt_message (GMT, "\t        1 = img file with constraints coded, interpolate to get data at track.\n");
	gmt_message (GMT, "\t        2 = img file with constraints coded, gets data only at constrained points, NaN elsewhere.\n");
	gmt_message (GMT, "\t        3 = img file with constraints coded, gets 1 at constraints, 0 elsewhere.\n");
	gmt_message (GMT, "\t        For mode 2|3 you may want to consider the -n+t<threshold> setting.\n");
}

/*! . */
void gmt_syntax (struct GMT_CTRL *GMT, char option) {
	/* The function print to stderr the syntax for the option indicated by
	 * the variable <option>.  Only the common parameter options are covered
	 */

	char *u = GMT->session.unit_name[GMT->current.setting.proj_length_unit];

	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option. Correct syntax:\n", option);

	switch (option) {

		case 'B':	/* Tickmark option */
			gmt_message (GMT, "\t-B[p|s][x|y|z]<intervals>[+a<angle>|n|p][+l|L<label>][+p<prefix>][+u<unit>] -B[<axes>][+b][+g<fill>][+o<lon>/<lat>][+t<title>] OR\n");
			gmt_message (GMT, "\t-B[p|s][x|y|z][a|f|g]<tick>[m][l|p] -B[p|s][x|y|z][+l<label>][+p<prefix>][+u<unit>] -B[<axes>][+b][+g<fill>][+o<lon>/<lat>][+t<title>]\n");
			break;

		case 'J':	/* Map projection option */
			switch (GMT->current.proj.projection_GMT) {
				case GMT_LAMB_AZ_EQ:
					gmt_message (GMT, "\t-Ja<lon0>/<lat0>[/<horizon>]/<scale> OR -JA<lon0>/<lat0>[/<horizon>]/<width>\n");
					gmt_message (GMT, "\t  <horizon> is distance from center to perimeter (<= 180, default 90)\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ALBERS:
					gmt_message (GMT, "\t-Jb<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JB<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CASSINI:
					gmt_message (GMT, "\t-Jc<lon0>/<lat0><scale> OR -JC<lon0>/<lat0><width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree ,or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_STEREO:
					gmt_message (GMT, "\t-Jcyl_stere/[<lon0>/[<lat0>/]]<scale> OR -JCyl_stere/[<lon0>/[<lat0>/]]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECONIC:
					gmt_message (GMT, "\t-Jd<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JD<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_AZ_EQDIST:
					gmt_message (GMT, "\t-Je<lon0>/<lat0>[/<horizon>]/<scale> OR -JE<lon0>/<lat0>[/<horizon>/<width>\n");
					gmt_message (GMT, "\t  <horizon> is distance from center to perimeter (<= 180, default 180)\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GNOMONIC:
					gmt_message (GMT, "\t-Jf<lon0>/<lat0>[/<horizon>]/<scale> OR -JF<lon0>/<lat0>[/<horizon>]/<width>\n");
					gmt_message (GMT, "\t  <horizon> is distance from center to perimeter (< 90, default 60)\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_ORTHO:
					gmt_message (GMT, "\t-Jg<lon0>/<lat0>[/<horizon>]/<scale> OR -JG<lon0>/<lat0>[/<horizon>]/<width>\n");
					gmt_message (GMT, "\t  <horizon> is distance from center to perimeter (<= 90, default 90)\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_GENPER:
					gmt_message (GMT, "\t-Jg<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<scale> OR\n\t-JG<lon0>/<lat0>/<altitude>/<azimuth>/<tilt>/<twist>/<Width>/<Height>/<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_HAMMER:
					gmt_message (GMT, "\t-Jh[<lon0>/]<scale> OR -JH[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_SINUSOIDAL:
					gmt_message (GMT, "\t-Ji[<lon0>/]<scale> OR -JI[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MILLER:
					gmt_message (GMT, "\t-Jj[<lon0>/]<scale> OR -JJ[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT4:
					gmt_message (GMT, "\t-Jkf[<lon0>/]<scale> OR -JKf[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ECKERT6:
					gmt_message (GMT, "\t-Jk[s][<lon0>/]<scale> OR -JK[s][<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_LAMBERT:
					gmt_message (GMT, "\t-Jl<lon0>/<lat0>/<lat1>/<lat2>/<scale> OR -JL<lon0>/<lat0>/<lat1>/<lat2>/<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MERCATOR:
					gmt_message (GMT, "\t-Jm[<lon0>/[<lat0>/]]<scale> OR -JM[<lon0>/[<lat0>/]]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_ROBINSON:
					gmt_message (GMT, "\t-Jn[<lon0>/]<scale> OR -JN[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_OBLIQUE_MERC:
					gmt_message (GMT, "\t-Jo[a]<lon0>/<lat0>/<azimuth>/<scale> OR -JO[a]<lon0>/<lat0>/<azimuth>/<width>\n");
					gmt_message (GMT, "\t-Jo[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<scale> OR -JO[b]<lon0>/<lat0>/<b_lon>/<b_lat>/<width>\n");
					gmt_message (GMT, "\t-Joc<lon0>/<lat0>/<lonp>/<latp>/<scale> OR -JOc<lon0>/<lat0>/<lonp>/<latp>/<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/oblique degree, or use <width> in %s\n", u, u);
					break;
				case GMT_WINKEL:
					gmt_message (GMT, "\t-Jr[<lon0>/]<scale> OR -JR[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLYCONIC:
					gmt_message (GMT, "\t-Jpoly/[<lon0>/[<lat0>/]]<scale> OR -JPoly/[<lon0>/[<lat0>/]]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQDIST:
					gmt_message (GMT, "\t-Jq[<lon0>/[<lat0>/]]<scale> OR -JQ[<lon0>/[<lat0>/]]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_STEREO:
					gmt_message (GMT, "\t-Js<lon0>/<lat0>[/<horizon>]/<scale> OR -JS<lon0>/<lat0>[/<horizon>]/<width>\n");
					gmt_message (GMT, "\t  <horizon> is distance from center to perimeter (< 180, default 90)\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx>, <lat>/<1:xxxx>, or <radius> (in %s)/<lat>, or use <width> in %s\n", u, u);
					break;
				case GMT_TM:
					gmt_message (GMT, "\t-Jt<lon0>/[<lat0>/]<scale> OR -JT<lon0>/[<lat0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_UTM:
					gmt_message (GMT, "\t-Ju<zone>/<scale> OR -JU<zone>/<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					gmt_message (GMT, "\t  <zone is A, B, 1-60[w/ optional C-X except I, O], Y, Z\n");
					break;
				case GMT_VANGRINTEN:
					gmt_message (GMT, "\t-Jv<lon0>/<scale> OR -JV[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_MOLLWEIDE:
					gmt_message (GMT, "\t-Jw[<lon0>/]<scale> OR -JW[<lon0>/]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_CYL_EQ:
					gmt_message (GMT, "\t-Jy[<lon0>/[<lat0>/]]<scale> OR -JY[<lon0>/[<lat0>/]]<width>\n");
					gmt_message (GMT, "\t  <scale> is <1:xxxx> or %s/degree, or use <width> in %s\n", u, u);
					break;
				case GMT_POLAR:
					gmt_message (GMT, "\t-Jp[a]<scale>[/<origin>][r] OR -JP[a]<width>[/<origin>][r]\n");
					gmt_message (GMT, "\t  <scale> is %s/units, or use <width> in %s\n", u, u);
					gmt_message (GMT, "\t  Optionally, prepend a for azimuths, append theta as origin [0],\n");
					gmt_message (GMT, "\t  or append r to reverse radial coordinates.\n");
					break;
				case GMT_LINEAR:
					gmt_message (GMT, "\t-Jx<x-scale>|<width>[d|l|p<power>|t|T][/<y-scale>|<height>[d|l|p<power>|t|T]], scale in %s/units\n", u);
					gmt_message (GMT, "\t-Jz<z-scale>[l|p<power>], scale in %s/units\n", u);
					gmt_message (GMT, "\tUse / to specify separate x/y scaling (e.g., -Jx0.5/0.3.).  Not allowed with 1:xxxxx\n");
					gmt_message (GMT, "\tUse -JX (and/or -JZ) to give axes lengths rather than scales\n");
					break;
				default:
					gmt_message (GMT, "\tProjection not recognized!\n");
					break;
			}
			break;

		case 'K':
			gmt_message (GMT, "\t-%c More PostScript content will follow\n", option);
			break;

		case 'O':
			gmt_message (GMT, "\t-%c This is a PostScript overlay\n", option);
			break;

		case 'P':
			gmt_message (GMT, "\t-%c Turn on portrait mode\n", option);
			break;

		case 'R':	/* Region option */
			gmt_message (GMT, "\t-R<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>]\n");
			gmt_message (GMT, "\t  Append r if giving lower left and upper right coordinates\n");
			gmt_message (GMT, "\t-Rg or -Rd for global domain\n");
			gmt_message (GMT, "\t-R<grdfile> to take the domain from a grid file\n");
			break;

		case 'U':	/* Set time stamp option */
			gmt_message (GMT, "\t%s\n", GMT_U_OPT);
			gmt_message (GMT, "\t   Plot the time stamp and optional command line or text.\n");
			break;

		case 'X':
			gmt_message (GMT, "\t%s\n", GMT_X_OPT);
			gmt_message (GMT, "\tPrepend a for temporary adjustment, c for center of page reference,\n");
			gmt_message (GMT, "\tf for lower left corner of page reference, r (or none) for relative to\n");
			gmt_message (GMT, "\tcurrent position; u is unit (c, i, p).\n");
			break;

		case 'Y':
			gmt_message (GMT, "\t%s\n", GMT_Y_OPT);
			gmt_message (GMT, "\tPrepend a for temporary adjustment, c for center of page reference,\n");
			gmt_message (GMT, "\tf for lower left corner of page reference, r (or none) for relative to\n");
			gmt_message (GMT, "\tcurrent position; u is unit (c, i, p).\n");
			break;

		case 'Z':
			if (gmt_M_compat_check (GMT, 4)) {
				gmt_message (GMT, "\t-Z<zlevel> set zlevel of basemap\n");
			}
			break;

		case 'a':	/* -a option for aspatial field substitution into data columns */

			gmt_message (GMT, "\t%s\n", GMT_a_OPT);
			gmt_message (GMT, "\t  Specify the aspatial field information.\n");
			break;

		case 'b':	/* Binary i/o option  */
			gmt_message (GMT, "\t%s\n", GMT_b_OPT);
			gmt_message (GMT, "\t   Binary data, add i for input, o for output [Default is both].\n");
			gmt_message (GMT, "\t   Here, t is c|u|h|H|i|I|l|L|f|d [Default is d (double)].\n");
			gmt_message (GMT, "\t   Prepend the number of data columns.\n");
			gmt_message (GMT, "\t   Append w to byte swap an item; append +L|B to fix endianness of file.\n");
			break;

		case 'f':	/* Column information option  */
			gmt_message (GMT, "\t%s\n", GMT_f_OPT);
			gmt_message (GMT, "\t   Column information, add i for input, o for output [Default is both].\n");
			gmt_message (GMT, "\t   <colinfo> is <colno>|<colrange>u, where column numbers start at 0\n");
			gmt_message (GMT, "\t   a range is given as <first>-<last>, e.g., 2-5., u is type:\n");
			gmt_message (GMT, "\t   t: relative time, T: absolute time, f: floating point,\n");
			gmt_message (GMT, "\t   x: longitude, y: latitude, g: short for 0x,1y.\n");
			break;

		case 'g':
			gmt_message (GMT, "\t%s\n", GMT_g_OPT);
			gmt_message (GMT, "\t   (Consult manual)\n");
			break;

		case 'h':	/* Header */

			gmt_message (GMT, "\t%s\n", GMT_h_OPT);
			gmt_message (GMT, "\t   Specify if Input/output file has header record(s)\n");
			gmt_message (GMT, "\t   Optionally, append i for input only and/or number of header records [0].\n");
			gmt_message (GMT, "\t     -hi turns off the writing of all headers on output.\n");
			gmt_message (GMT, "\t   Append +c to add header record with column information [none].\n");
			gmt_message (GMT, "\t   Append +d to delete headers before adding new ones [Default will append headers].\n");
			gmt_message (GMT, "\t   Append +r to add a <remark> comment to the output [none].\n");
			gmt_message (GMT, "\t   Append +t to add a <title> comment to the output [none].\n");
			gmt_message (GMT, "\t     (these strings may contain \\n to indicate line-breaks)\n");
			gmt_message (GMT, "\t   For binary files, <n> is considered to mean number of bytes.\n");
			break;

		case 'i':	/* -i option for input column order */

			gmt_message (GMT, "\t%s\n", GMT_i_OPT);
			gmt_message (GMT, "\t   Sets alternate input column order and/or scaling [Default reads all columns in order].\n");
			break;

		case 'n':	/* -n option for grid resampling parameters in BCR */

			gmt_message (GMT, "\t%s\n", GMT_n_OPT);
			gmt_message (GMT, "\t   Determine the grid interpolation mode.\n");
			gmt_message (GMT, "\t   (b = B-spline, c = bicubic, l = bilinear, n = nearest-neighbor) [Default: bicubic].\n");
			gmt_message (GMT, "\t   Append +a switch off antialiasing (except for l) [Default: on].\n");
			gmt_message (GMT, "\t   Append +b<BC> to change boundary conditions.  <BC> can be either:\n");
			gmt_message (GMT, "\t   g for geographic boundary conditions, or one or both of\n");
			gmt_message (GMT, "\t   x for periodic boundary conditions on x,\n");
			gmt_message (GMT, "\t   y for periodic boundary conditions on y.\n");
			gmt_message (GMT, "\t   [Default: Natural conditions, unless grid is known to be geographic].\n");
			gmt_message (GMT, "\t   Append +c to clip interpolated grid to input z-min/max [Default may exceed limits].\n");
			gmt_message (GMT, "\t   Append +t<threshold> to change the minimum weight in vicinity of NaNs. A threshold of\n");
			gmt_message (GMT, "\t   1.0 requires all nodes involved in interpolation to be non-NaN; 0.5 will interpolate\n");
			gmt_message (GMT, "\t   about half way from a non-NaN to a NaN node [Default: 0.5].\n");
			break;

		case 'o':	/* -o option for output column order */

			gmt_message (GMT, "\t%s\n", GMT_o_OPT);
			gmt_message (GMT, "\t   Sets alternate output column order and/or scaling [Default writes all columns in order].\n");
			break;

		case 'p':
			gmt_message (GMT, "\t%s\n", GMT_p_OPT);
			gmt_message (GMT, "\t   Azimuth and elevation (and zlevel) of the viewpoint [180/90/bottom z-axis].\n");
			gmt_message (GMT, "\t   Append +w and +v to set coordinates to a fixed viewpoint\n");
			break;

		case 's':	/* Skip records with NaN as z */
			gmt_message (GMT, "\t%s\n", GMT_s_OPT);
			gmt_message (GMT, "\t   Skip records whose <col> [2] output is NaN.\n");
			gmt_message (GMT, "\t   a skips if ANY columns is NaN, while r reverses the action.\n");
			break;

		case 't':	/* -t layer transparency option  */
			gmt_message (GMT, "\t%s\n", GMT_t_OPT);
			gmt_message (GMT, "\t   Set the layer PDF transparency from 0-100 [Default is 0; opaque].\n");
			break;

		case 'x':	/* Number of threads */
			gmt_message (GMT, "\t%s\n", GMT_x_OPT);
			gmt_message (GMT, "\t   Control the number of processors used in multi-threading.\n");
			gmt_message (GMT, "\t   -x+a Use all available processors.\n");
			gmt_message (GMT, "\t   -xn Use n processors (not more than max available off course).\n");
			gmt_message (GMT, "\t   -x-n Use (all - n) processors.\n");
			break;

		case ':':	/* lon/lat vs lat/lon i/o option  */
			gmt_message (GMT, "\t%s\n", GMT_colon_OPT);
			gmt_message (GMT, "\t   Interpret first two columns, add i for input, o for output [Default is both].\n");
			gmt_message (GMT, "\t   Swap 1st and 2nd column on input and/or output.\n");
			break;

		case '-':	/* --PAR=value  */
			gmt_message (GMT, "\t--<PARAMETER>=<value>.\n");
			gmt_message (GMT, "\t   See gmt.conf for list of parameters.\n");
			break;

		default:
			break;
	}
}

int gmt_default_error (struct GMT_CTRL *GMT, char option) {
	/* gmt_default_error ignores all the common options that have already been processed and returns
	 * true if the option is not an already processed common option.
	 */

	int error = 0;

	switch (option) {

		case '-': break;	/* Skip indiscriminently */
		case '=': break;	/* Skip indiscriminently */
		case '>': break;	/* Skip indiscriminently since dealt with internally */
		case 'B': error += (GMT->common.B.active[GMT_PRIMARY] == false && GMT->common.B.active[GMT_SECONDARY] == false); break;
		case 'J': error += GMT->common.J.active == false; break;
		case 'K': error += GMT->common.K.active == false; break;
		case 'O': error += GMT->common.O.active == false; break;
		case 'P': error += GMT->common.P.active == false; break;
		case 'R': error += GMT->common.R.active[RSET] == false; break;
		case 'U': error += GMT->common.U.active == false; break;
		case 'V': error += GMT->common.V.active == false; break;
		case 'X': error += GMT->common.X.active == false; break;
		case 'Y': error += GMT->common.Y.active == false; break;
		case 'a': error += GMT->common.a.active == false; break;
		case 'b': error += ((GMT->common.b.active[GMT_IN] == false && GMT->common.b.nc[GMT_IN] == false) \
			&& (GMT->common.b.active[GMT_OUT] == false && GMT->common.b.nc[GMT_OUT] == false)); break;
		case 'd': error += (GMT->common.d.active[GMT_IN] == false && GMT->common.d.active[GMT_OUT] == false); break;
		case 'e': error += GMT->common.e.active == false; break;
		case 'f': error += (GMT->common.f.active[GMT_IN] == false &&  GMT->common.f.active[GMT_OUT] == false); break;
		case 'g': error += GMT->common.g.active == false; break;
		case 'H':
			if (gmt_M_compat_check (GMT, 4)) {
				error += GMT->common.h.active == false;
			}
			else
				error++;
			break;
		case 'h': error += GMT->common.h.active == false; break;
		case 'i': error += GMT->common.i.active == false; break;
		case 'j': error += GMT->common.j.active == false; break;
		case 'n': error += GMT->common.n.active == false; break;
		case 'o': error += GMT->common.o.active == false; break;
		case 'Z':
			if (!gmt_M_compat_check (GMT, 4)) error++;
			break;
		case 'E':
			if (gmt_M_compat_check (GMT, 4))
				error += GMT->common.p.active == false;
			else
				error++;
			break;
		case 'p': error += GMT->common.p.active == false; break;
		case 'm': if (!gmt_M_compat_check (GMT, 4)) error++; break;
		case 'S': if (!gmt_M_compat_check (GMT, 4)) error++; break;
		case 'F': if (!gmt_M_compat_check (GMT, 4)) error++; break;
		case 'r': error += GMT->common.R.active[GSET] == false; break;
		case 's': error += GMT->common.s.active == false; break;
		case 't': error += GMT->common.t.active == false; break;
#ifdef GMT_MP_ENABLED
		case 'x': error += GMT->common.x.active == false; break;
#endif
		case ':': error += GMT->common.colon.active == false; break;

		default:
			/* Not a processed common options */
			error++;
			break;
	}

	if (error) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized option -%c\n", option);
	return (error);
}

/*! If region is given then we must have w < e and s < n */
bool gmt_check_region (struct GMT_CTRL *GMT, double wesn[]) {
	gmt_M_unused(GMT);
	return ((wesn[XLO] >= wesn[XHI] || wesn[YLO] >= wesn[YHI]));
}

/*! . */
int gmt_parse_R_option (struct GMT_CTRL *GMT, char *arg) {
	unsigned int i, icol, pos, error = 0, n_slash = 0, first = 0;
	int got, col_type[2], expect_to_read;
	size_t length;
	bool inv_project = false, scale_coord = false, got_r, got_country, no_T = false, done[3] = {false, false, false};
	char text[GMT_BUFSIZ] = {""}, item[GMT_BUFSIZ] = {""}, string[GMT_BUFSIZ] = {""}, r_unit = 0, *c = NULL, *d = NULL, *ptr = NULL;
	double p[6];

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* Got nothing */
	strncpy (item, arg, GMT_BUFSIZ-1);	/* Copy locally */

	if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Must handle any internal history regarding increments and registration */
		/* Here, item may be of the form <region>[+I<incs>][+G[P|G][B|T]] if -R was given to a non-plotting module */
		if ((c = strstr (item, "+G")) != NULL) {	/* Got grid registration */
			GMT->common.R.active[GSET] = true;
			GMT->common.R.registration = strchr(&c[2], 'P') != NULL || strchr(&c[2], 'p') != NULL ? GMT_GRID_PIXEL_REG : GMT_GRID_NODE_REG;
			GMT->common.R.row_order = strchr(&c[2], 'T') != NULL || strchr(&c[2], 't') != NULL ? k_nc_start_north : k_nc_start_south;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT modern: Obtained grid registration and/or row order from -R%s\n", item);
		}
		if ((d = strstr (item, "+I")) != NULL) {	/* Got grid increments */
			if (gmt_getinc (GMT, &d[2], GMT->common.R.inc)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GMT modern: Error parsing the grid spacing.\n");
				return (GMT_PARSE_ERROR);
			}
			GMT->common.R.active[ISET] = true;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT modern: Obtained grid spacing from -R%s\n", item);
		}
		/* Chop off modifiers. This has to be after the above ifs, to allow +I and +G in any order */
		if (c) c[0] = '\0';
		if (d) d[0] = '\0';
	}

	/* Parse the -R option.  Full syntax: -R<grdfile> or -Rg or -Rd or -R[L|C|R][B|M|T]<x0>/<y0>/<n_columns>/<n_rows> or -R[g|d]w/e/s/n[/z0/z1][r] */
	length = strlen (item) - 1;
	n_slash = gmtlib_count_char (GMT, item, '/');
	got_r = (strstr (item, "+r") != NULL);
	got_country = (got_r || (strstr (item, "+R") != NULL));	/* May have given DCW (true of +R, maybe if +r since the latter also means oblique) */

	strncpy (GMT->common.R.string, item, GMT_LEN256-1);	/* Verbatim copy */

	if (n_slash == 3 && !got_country && ((strchr ("LCR", item[0]) && strchr ("TMB", item[1])) || (strchr ("LCR", item[1]) && strchr ("TMB", item[0])))) {	/* Extended -R option using coordinate codes and grid increments */
		char X[2][GMT_LEN64] = {"", ""}, code[3] = {""};
		double xdim, ydim, orig[2] = {0.0, 0.0};
		int n_columns, n_rows, just, part;
		gmt_M_memcpy (code, item, 2, char);
		if ((just = gmt_just_decode (GMT, code, PSL_NO_DEF)) == -99) {	/* Since justify not in correct format */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -R: Unrecognized justification code %s\n", code);
			return (GMT_PARSE_ERROR);
		}
		if (sscanf (&item[2], "%[^/]/%[^/]/%d/%d", X[0], X[1], &n_columns, &n_rows) != 4) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -R%s<lon0>/<lat0>/<n_columns>/<n_rows>: Did not get 4 items\n", code);
			return (GMT_PARSE_ERROR);
		}
		for (icol = GMT_X; icol <= GMT_Y; icol++) {
			if (gmt_M_type (GMT, GMT_IN, icol) == GMT_IS_UNKNOWN) {	/* No -J or -f set, proceed with caution */
				got = gmt_scanf_arg (GMT, X[icol], gmt_M_type (GMT, GMT_IN, icol), true, &orig[icol]);
				if (got & GMT_IS_GEO)
					gmt_set_column (GMT, GMT_IN, icol, got);
				else if (got & GMT_IS_RATIME) {
					gmt_set_column (GMT, GMT_IN, icol, got);
					GMT->current.proj.xyz_projection[icol] = GMT_TIME;
				}
			}
			else {	/* Things are set, do or die */
				expect_to_read = (gmt_M_type (GMT, GMT_IN, icol) & GMT_IS_RATIME) ? GMT_IS_ARGTIME : gmt_M_type (GMT, GMT_IN, icol);
				error += gmt_verify_expectations (GMT, expect_to_read, gmt_scanf (GMT, X[icol], expect_to_read, &orig[icol]), X[icol]);
			}
		}
		if (error) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -R%s<lon0>/<lat0>/<n_columns>/<n_rows>: Could not parse coordinate pair\n", code);
			return (GMT_PARSE_ERROR);
		}
		if (n_columns <= 0 || n_rows <= 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in -R%s<lon0>/<lat0>/<n_columns>/<n_rows>: Must have positive dimensions\n", code);
			return (GMT_PARSE_ERROR);
		}
		/* Finally set up -R */
		if (!GMT->common.R.active[GSET]) n_columns--, n_rows--;	/* Needed to get correct dimensions */
		xdim = n_columns * GMT->common.R.inc[GMT_X];
		ydim = n_rows * GMT->common.R.inc[GMT_Y];
		part = just / 4;	/* Need any multiples of 4 in just */
		GMT->common.R.wesn[XLO] = orig[GMT_X] - 0.5 * ((just%4)-1) * xdim;
		GMT->common.R.wesn[YLO] = orig[GMT_Y] - 0.5 * part * ydim;
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
		gmt_set_geographic (GMT, GMT_IN);
		return (GMT_NOERROR);
	}
	ptr = item;	/* To avoid compiler warning that item cannot be NULL */
	if (!gmt_M_file_is_memory (ptr) && ptr[0] == '@') {	/* Must be a cache file */
		first = gmt_download_file_if_not_found (GMT, item, 0);
	}
	if (!gmt_access (GMT, &item[first], R_OK)) {	/* Gave a readable file, presumably a grid */
		struct GMT_GRID *G = NULL;
		if ((G = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, &item[first], NULL)) == NULL) {	/* Read header */
			return (GMT->parent->error);
		}
		if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Handle round-off in actual_range for latitudes */
			if (gmt_M_is_Npole (G->header->wesn[YHI])) G->header->wesn[YHI] = +90.0;
			if (gmt_M_is_Spole (G->header->wesn[YLO])) G->header->wesn[YLO] = -90.0;
		}
		if ((GMT->current.proj.projection_GMT == GMT_UTM || GMT->current.proj.projection_GMT == GMT_TM || GMT->current.proj.projection_GMT == GMT_STEREO)) {	/* Perhaps we got an [U]TM or stereographic grid? */
			if (fabs (G->header->wesn[XLO]) > 360.0 || fabs (G->header->wesn[XHI]) > 360.0 \
			  || fabs (G->header->wesn[YLO]) > 90.0 || fabs (G->header->wesn[YHI]) > 90.0) {	/* Yes we probably did, but cannot be sure */
				inv_project = true;
				r_unit = 'e';	/* Must specify the "missing" leading e for meter */
				sprintf (string, "%.16g/%.16g/%.16g/%.16g", G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI]);
			}
		}
		if (!inv_project) {	/* Got grid with degrees or regular Cartesian */
			struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (G->header);
			gmt_M_memcpy (GMT->common.R.wesn, G->header->wesn, 4, double);
			if (GMT->common.R.active[ISET] == false) gmt_M_memcpy (GMT->common.R.inc, G->header->inc, 2, double);	/* Do not override settings given via -I */
			GMT->common.R.registration = (GMT->common.R.active[GSET]) ? !G->header->registration : G->header->registration;	/* Set, or toggle registration if -r was set */
			GMT->common.R.row_order = HH->row_order;	/* Copy the row order */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG,
				"-R<grdfile> converted to -R%.16g/%.16g/%.16g/%.16g\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
#if 0
			/* Next bit removed because of issue #592. Should not change the boundaries of the grid */
			if (GMT->common.R.registration == GMT_GRID_NODE_REG && doubleAlmostEqualZero (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO] + GMT->common.R.inc[GMT_X], 360.0)) {
				/* Geographic grid with gridline registration that does not contain the repeating column, but is still 360 range */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG,
				            "-R<file> with gridline registration and non-repeating column detected; return full 360 degree range for -R\n");
				if (gmt_M_is_zero (GMT->common.R.wesn[XLO]) || doubleAlmostEqualZero (GMT->common.R.wesn[XLO], -180.0))
					GMT->common.R.wesn[XHI] = GMT->common.R.wesn[XLO] + 360.0;
				else
					GMT->common.R.wesn[XLO] = GMT->common.R.wesn[XHI] - 360.0;
			}
#endif
			GMT->common.R.wesn[ZLO] = G->header->z_min;	GMT->common.R.wesn[ZHI] = G->header->z_max;
			GMT->common.R.active[ISET] = GMT->common.R.active[GSET] = GMT->common.R.active[FSET] = true;
			if (GMT_Destroy_Data (GMT->parent, &G) != GMT_OK)
				return (GMT->parent->error);
			return (GMT_NOERROR);
		}
	}
	else if ((item[0] == 'g' || item[0] == 'd') && n_slash == 3) {	/* Here we have a region appended to -Rd|g */
		gmt_set_geographic (GMT, GMT_IN);
		strncpy (string, &item[1], GMT_BUFSIZ-1);
		GMT->current.io.geo.range = (item[0] == 'g') ? GMT_IS_0_TO_P360_RANGE : GMT_IS_M180_TO_P180_RANGE;
	}
	else if ((isupper ((int)item[0]) && isupper ((int)item[1])) || item[0] == '=' || strchr (item, ',')) {
		/* Region specified via country codes with optional round off/extension, e.g., -RNO+r1 or -R=EU */
		struct GMT_DCW_SELECT info;
		gmt_M_memset (&info, 1, struct GMT_DCW_SELECT);	/* To ensure it is all NULL, 0 */
		if ((error = gmt_DCW_parse (GMT, 'R', item, &info))) return error;
		(void) gmt_DCW_operation (GMT, &info, GMT->common.R.wesn, GMT_DCW_REGION);	/* Get region */
		gmt_DCW_free (GMT, &info);
		if (fabs (GMT->common.R.wesn[XLO]) > 1000.0) return (GMT_MAP_NO_REGION);
		if (strstr (item, "=AN")) {	/* Antarctica is the only polar cap polygon so w,e,s must be reset */
			GMT->common.R.wesn[XLO] = -180.0;
			GMT->common.R.wesn[XHI] = +180.0;
			GMT->common.R.wesn[YLO] = -90.0;
		}
		if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] > 0.0)
			GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
		else
			GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
		gmt_set_geographic (GMT, GMT_IN);
		return (GMT_NOERROR);
	}
	else if ((c = gmt_first_modifier (GMT, item, "u"))) {	/* Got +u<unit> */
		c[0] = '\0';	/* Chop off all modifiers so range can be determined */
		strncpy (string, item, GMT_BUFSIZ-1);
		r_unit = c[2];	/* The data unit */
		c[0] = '+';	/* Restore */
		if (gmt_M_is_linear (GMT))	/* Just scale up the values */
			scale_coord = true;
		else
			inv_project = true;
	}
	else if (strchr (GMT_LEN_UNITS2, item[0])) {	/* Obsolete: Specified min/max in projected distance units */
		strncpy (string, &item[1], GMT_BUFSIZ-1);
		r_unit = item[0];	/* The leading unit */
		if (gmt_M_is_linear (GMT))	/* Just scale up the values */
			scale_coord = true;
		else
			inv_project = true;
	}
	else if (item[length] != 'r' && !strstr (item, "+r") && (GMT->current.proj.projection_GMT == GMT_UTM || GMT->current.proj.projection_GMT == GMT_TM || GMT->current.proj.projection_GMT == GMT_STEREO)) {
		/* Just _might_ be getting -R in meters, better check if argument is too large to be degrees */
		double rect[4] = {0.0, 0.0, 0.0, 0.0};
		int n_rect_read;
		strncpy (string, item, GMT_BUFSIZ-1);	/* Try to read these as 4 limits in meters */
		n_rect_read = sscanf (string, "%lg/%lg/%lg/%lg", &rect[XLO], &rect[XHI], &rect[YLO], &rect[YHI]);
		if (n_rect_read == 4 && (fabs (rect[XLO]) > 360.0 || fabs (rect[XHI]) > 360.0 || fabs (rect[YLO]) > 90.0 || fabs (rect[YHI]) > 90.0)) {	/* Oh, yeah... */
			inv_project = true;
			r_unit = 'e';	/* Must specify the "missing" leading e for meter */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "For a UTM or TM projection, your region %s is too large to be in degrees and thus assumed to be in meters\n", string);
		}
	}
	else	/* Plain old -Rw/e/s/n */
		strncpy (string, item, GMT_BUFSIZ-1);

	/* Now decode the string */

	length = strlen (string) - 1;
	col_type[0] = col_type[1] = 0;
	if ((c = gmt_first_modifier (GMT, string, "r"))) {	/* Got +r */
		GMT->common.R.oblique = true;
		c[0] = '\0';	/* Remove the trailing +r so gmt_scanf will work */
	}
	else if (string[length] == 'r') {	/* Obsolete */
		GMT->common.R.oblique = true;
		string[strlen(string)-1] = '\0';	/* Remove the trailing r so gmt_scanf will work */
	}
	else
		GMT->common.R.oblique = false;
	i = pos = 0;
	gmt_M_memset (p, 6, double);
	while ((gmt_strtok (string, "/", &pos, text))) {
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
		if (inv_project)	/* input is distance units */
			p[i] = atof (text);
		else {
			bool maybe_time = gmtlib_maybe_abstime (GMT, text, &no_T);	/* Will flag things like 1999-12-03 or 2000/09/4 as abstime with missing T */
			if (maybe_time || gmt_M_type (GMT, GMT_IN, icol) == GMT_IS_UNKNOWN || gmt_M_type (GMT, GMT_IN, icol) == GMT_IS_FLOAT) {	/* Because abstime specs must be parsed carefully we must do more checking */
				/* Here, -J or -f may have ben set, proceed with caution */
				if (no_T) strcat (text, "T");	/* Add the missing trailing 'T' in an ISO date */
				got = gmt_scanf_arg (GMT, text, GMT_IS_UNKNOWN, true, &p[i]);
				if (got == GMT_IS_LON || got == GMT_IS_LAT)	/* If clearly lon or lat we say so */
					gmt_set_column (GMT, GMT_IN, icol, got), done[icol] = true;
				else if (got == GMT_IS_GEO)	/* If clearly geographical we say so */
					gmt_set_column (GMT, GMT_IN, icol, got), done[icol] = true;
				else if ((got & GMT_IS_RATIME) || got == GMT_IS_ARGTIME) {	/* If we got time then be specific */
					if (got == GMT_IS_ARGTIME) got = GMT_IS_ABSTIME;
					/* While the -R arg may be abstime, -J or -f may have indicated that data are relative time, so we must check before imposing our got: */
					if (!(gmt_M_type (GMT, GMT_IN, icol) & GMT_IS_RATIME)) gmt_set_column (GMT, GMT_IN, icol, got), done[icol] = true;	/* Only set if not already set to a time flavor */
					GMT->current.proj.xyz_projection[icol] = GMT_TIME;
				}
				else if (got == GMT_IS_FLOAT) {	/* Don't want to set col type prematurely ince -R0/13:30E/... would first find Cartesian and then fail on 13:30E */
					if (done[icol] && gmt_M_type (GMT, GMT_IN, icol) == GMT_IS_UNKNOWN)	/* 2nd time for this column and still not set, then FLOAT is OK */
						gmt_set_column (GMT, GMT_IN, icol, got);
				}
				else {	/* Testing this, could we ever get here with sane data? */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not parse %s into Geographical, Cartesian, or Temporal coordinates!", text);
					error++;
				}
			}
			else {	/* Things are already prescribed by -J or -f, do or die */
				expect_to_read = (gmt_M_type (GMT, GMT_IN, icol) & GMT_IS_RATIME) ? GMT_IS_ARGTIME : gmt_M_type (GMT, GMT_IN, icol);
				error += gmt_verify_expectations (GMT, expect_to_read, gmt_scanf (GMT, text, expect_to_read, &p[i]), text);
			}
		}
		if (error) return (error);

		i++;
	}
	if (GMT->common.R.oblique) {
		gmt_M_double_swap (p[2], p[1]);	/* So w/e/s/n makes sense */
		gmt_M_memcpy (GMT->common.R.wesn_orig, p, 4, double);	/* Save these in case they get enlarged by oblique projections */
	}
	if (inv_project) {	/* Convert rectangular distances to geographic corner coordinates */
		double wesn[4] = {0.0, 0.0, 0.0, 0.0};
		GMT->common.R.oblique = false;
		error += gmtinit_rectR_to_geoR (GMT, r_unit, p, wesn, true);
		gmt_M_memcpy (p, wesn, 4, double);
		GMT->common.R.oblique = true;
		gmt_set_geographic (GMT, GMT_IN);
	}
	else if (scale_coord) {	/* Just scale x/y coordinates to meters according to given unit */
		double fwd_scale, inv_scale = 0.0, inch_to_unit = 0, unit_to_inch = 0;
		int k_unit;
		k_unit = gmtlib_get_unit_number (GMT, item[0]);
		gmt_init_scales (GMT, k_unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, NULL);
		for (pos = 0; pos < 4; pos++) p[pos] *= inv_scale;
	}
	if (gmt_M_type (GMT, GMT_IN, GMT_X) == GMT_IS_GEO && gmt_M_type (GMT, GMT_IN, GMT_Y) == GMT_IS_GEO) {	/* Two geographical coordinates means lon,lat */
		gmt_set_column (GMT, GMT_IN, GMT_X, GMT_IS_LON);
		gmt_set_column (GMT, GMT_IN, GMT_Y, GMT_IS_LAT);
	}
	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Arrange so geographic region always has w < e */
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
	if (i < 4 || i > 6 || ((!GMT->common.R.oblique && gmt_check_region (GMT, p)) || (i == 6 && p[4] >= p[5]))) error++;
	gmt_M_memcpy (GMT->common.R.wesn, p, 6, double);	/* This will probably be reset by gmt_map_setup */
	if (i == 6 && !GMT->current.proj.JZ_set) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "-R with six parameters but no -Jz|Z given - ignore zmin/zmax\n");
		GMT->common.R.wesn[ZLO] = GMT->common.R.wesn[ZHI] = 0.0;
	}
	return (error);
}

/*! . */
int64_t gmt_parse_range (struct GMT_CTRL *GMT, char *p, int64_t *start, int64_t *stop) {
	/* Parses p looking for range or columns or individual columns.
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

/*! . */
unsigned int gmt_parse_d_option (struct GMT_CTRL *GMT, char *arg) {
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
	if (first == GMT_IN) strncpy (GMT->common.d.string, arg, GMT_LEN64-1);	/* Verbatim copy */
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL unsigned int gmtinit_parse_e_option (struct GMT_CTRL *GMT, char *arg) {
	/* Parse the -e option.  Full syntax: -e[~]\"search string\" */

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -e requires an argument */
	strncpy (GMT->common.e.string, arg, GMT_LEN64-1);	/* Make copy of -e argument verbatim */
	GMT->common.e.select = gmt_set_text_selection (GMT, arg);

	return (GMT_NOERROR);
}

/*! Routine will decode the -i<col>|<colrange>|t[+l][+s<scale>][+o<offset>],... arguments or just -in */
int gmt_parse_i_option (struct GMT_CTRL *GMT, char *arg) {

	char copy[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, word[GMT_LEN256] = {""}, *c = NULL;
	bool new_style;
	unsigned int k = 0, pos = 0, pos_p, uerr = 0;
	int convert;
	int64_t i, start = -1, stop = -1, inc;
	double scale, offset;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -i requires an argument */

	strncpy (copy, arg, GMT_BUFSIZ-1);

	GMT->current.io.trailing_text[GMT_IN] = GMT->current.io.trailing_text[GMT_OUT] = false;	/* When using -i you have to specifically add column t to parse trailing text */
	if (!strcmp (arg, "n")) return GMT_NOERROR;	/* We just wanted to process the numerical columns */

	new_style = (strstr (arg, "+s") || strstr (arg, "+o") || strstr (arg, "+l"));

	strncpy (GMT->common.i.string, arg, GMT_LEN64-1);	/* Verbatim copy */
	for (i = 0; i < GMT_MAX_COLUMNS; i++) GMT->current.io.col_skip[i] = true;	/* Initially, no input column is requested */

	while ((gmt_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process the comma-separated sections */
		convert = 0, scale = 1.0, offset = 0.0;	/* Reset for next column selection */
		if (new_style) {	/* New format as of 5.4: -i<col>|<colrange>[+l][+s<scale>][+o<offset>],... */
			if ((c = gmt_first_modifier (GMT, p, "los"))) {	/* Process modifiers */
				pos_p = 0;	/* Reset to start of new word */
				while (gmt_getmodopt (GMT, 'i', c, "los", &pos_p, word, &uerr) && uerr == 0) {
					switch (word[0]) {
						case 'l': convert |= 2; break;
						case 'o': convert |= 1; offset = atof (&word[1]); break;
						case 's': convert |= 1; scale  = atof (&word[1]); break;
						default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
					}
				}
				c[0] = '\0';	/* Chop off all modifiers so range can be determined */
				if (uerr) return (GMT_PARSE_ERROR);
			}
		}
		else {	/* Old-style: -i<col>|<colrange>[l][s<scale>][o<offset>],... */
			char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
			if ((c = strchr (p, 'o'))) {	/* Look for offset */
				c[0] = '\0';	/* Wipe out the 'o' so that next scan terminates there */
				convert |= 1;
				offset = atof (&c[1]);
			}
			if ((c = strchr (p, 's'))) {	/* Look for scale factor */
				c[0] = '\0';	/* Wipe out the 's' so that next scan terminates there */
				if (gmt_M_compat_check (GMT, 4)) {	/* GMT4 */
					i = (int)strlen (p) - 1;
					convert = (p[i] == 'l') ? 2 : 1;
					i = sscanf (&c[1], "%[^/]/%[^l]", txt_a, txt_b);
					if (i == 0) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-i...s contains bad scale info\n");
						return (GMT_PARSE_ERROR);
					}
					scale = atof (txt_a);
					if (i == 2) offset = atof (txt_b);
				} else {
					convert |= 1;
					scale = atof (&c[1]);
				}
			}
			if ((c = strchr (p, 'l'))) {	/* Look for log indicator */
				c[0] = '\0';	/* Wipe out the 's' so that next scan terminates there */
				convert |= 2;
			}
		}

		if (!strcmp (p, "t"))	/* Got the trailing test "column" */
			GMT->current.io.trailing_text[GMT_IN] = GMT->current.io.trailing_text[GMT_OUT] = true;
		else {	/* Now process column range */
			if ((inc = gmt_parse_range (GMT, p, &start, &stop)) == 0) return (GMT_PARSE_ERROR);

			/* Now set the code for these columns */

			for (i = start; i <= stop; i += inc, k++) {
				GMT->current.io.col_skip[i] = false;	/* Do not skip these */
				GMT->current.io.col[GMT_IN][k].col = (unsigned int)i;	/* Requested physical column */
				GMT->current.io.col[GMT_IN][k].order = k;		/* Requested logical order of columns */
				GMT->current.io.col[GMT_IN][k].convert = convert;
				GMT->current.io.col[GMT_IN][k].scale = scale;
				GMT->current.io.col[GMT_IN][k].offset = offset;
			}
		}
	}
	qsort (GMT->current.io.col[GMT_IN], k, sizeof (struct GMT_COL_INFO), gmtinit_compare_cols);
	GMT->common.i.n_cols = k;
	if (k) {	/* Because the user may have repeated some columns we also determine how many unique columns were requested */
		GMT->common.i.n_actual_cols = 1;
		for (i = 1; i < k; i++) if (GMT->current.io.col[GMT_IN][i].col != GMT->current.io.col[GMT_IN][i-1].col)
			GMT->common.i.n_actual_cols++;
	}
	GMT->common.i.orig = GMT->common.i.select = true;

#if 0
	if (GMT->common.i.n_cols == 0 && GMT->current.io.trailing_text[GMT_IN]) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-it is not allowed, need at least 1-2 leading numerical columns\n");
		return (GMT_PARSE_ERROR);
	}
#endif
	return (GMT_NOERROR);
}

int gmt_parse_j_option (struct GMT_CTRL *GMT, char *arg) {
	int err = GMT_NOERROR;
	if (arg == NULL) return GMT_PARSE_ERROR;	/* Must supply the arg */
	switch (arg[0]) {
		case 'c': GMT->common.j.mode = GMT_NO_MODE; break;
		case 'e': GMT->common.j.mode = GMT_GEODESIC; break;
		case 'f': GMT->common.j.mode = GMT_FLATEARTH; break;
		case 'g': case '\0': GMT->common.j.mode = GMT_GREATCIRCLE; break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-j argument %s is not one of the valid modes e|f|g\n", arg);
			err = GMT_PARSE_ERROR;
			break;
	}
	strncpy (GMT->common.j.string, arg, GMT_LEN8-1);
	return (err);
}

int gmt_parse_l_option (struct GMT_CTRL *GMT, char *arg) {
	if (GMT->current.setting.run_mode == GMT_CLASSIC) {	/* Not in modern mode */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-l is only allowed in modern mode\n");
		return GMT_PARSE_ERROR;
	}
	if (arg == NULL || arg[0] == '\0') return GMT_PARSE_ERROR;	/* Must supply the label arg */
	strncpy (GMT->common.l.label, arg, GMT_LEN128-1);
	return (GMT_NOERROR);
}

/*! Routine will decode the -[<col>|<colrange>|t,... arguments or just -on */
int gmt_parse_o_option (struct GMT_CTRL *GMT, char *arg) {

	char copy[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""};
	unsigned int pos = 0;
	uint64_t k = 0;
	int64_t i, start = -1, stop = -1, inc;

	if (!arg || !arg[0]) return (GMT_PARSE_ERROR);	/* -o requires an argument */

	strncpy (copy, arg, GMT_BUFSIZ-1);

	GMT->current.io.trailing_text[GMT_OUT] = false;	/* When using -o you have to specifically add column t to parse trailing text */
	if (! strcmp (arg, "n")) return GMT_NOERROR;	/* We just wanted to select numerical output only */

	while ((gmt_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process it */
		if (!strcmp (p, "t"))
			GMT->current.io.trailing_text[GMT_OUT] = true;	/* Include trailing text */
		else {
			if ((inc = gmt_parse_range (GMT, p, &start, &stop)) == 0) return (GMT_PARSE_ERROR);

			/* Now set the code for these columns */

			for (i = start; i <= stop; i += inc, k++) {
				GMT->current.io.col[GMT_OUT][k].col = (unsigned int)i;	/* Requested order of columns */
				GMT->current.io.col[GMT_OUT][k].order = (unsigned int)k;		/* Requested order of columns */
			}
		}
	}
	GMT->common.o.n_cols = k;
	if (GMT->common.b.active[GMT_OUT] && GMT->common.b.ncol[GMT_OUT] == 0) GMT->common.b.ncol[GMT_OUT] = GMT->common.b.ncol[GMT_IN];	/* Since -o machinery will march through */
	GMT->common.o.select = true;
	return (GMT_NOERROR);
}

/*! . */
int gmt_parse_model (struct GMT_CTRL *GMT, char option, char *in_arg, unsigned int dim, struct GMT_MODEL *M) {
	/* This may eventually switch on dim, but for now it is just 1D */
	gmt_M_unused(dim);
	return (gmtinit_parse_model1d (GMT, option, in_arg, M));
}

/*! . */
unsigned int gmt_parse_segmentize (struct GMT_CTRL *GMT, char option, char *in_arg, unsigned int mode, struct GMT_SEGMENTIZE *S) {
	/* Parse segmentizing options in gmt convert (mode == 0) or psxy (mode == 1).
	 * Syntax is given below (assuming option = -F here):
	 * -F[c|n|r|v][a|f|s|R] or -Fr<origin>
	 * where c = continuous [Defuult], n = network, r = reference point, and v = vectors.
	 * a = all files, f = per file, s = per segment [Default], r = per record.
	 * Four different segmentizing schemes:
	 * 1) -Fc: Continuous lines.  By default, lines are drawn on a segment by segment basis.
	 *    Thus, -F or -Fc or -Fcs or -Fs is the standard default.  However, if we use
	 *    -Fcf or -Ff then we ignore segment headers WITHIN each file, except for the first header
	 *   in each file.  In other words, all points in a file will be considered continuous.
	 *   Finally, using -Fca or -Fa then all points in all fields are considered continuous and
	 *   only the first segment header in the first file is considered.  So only a|f|r is allowed.
	 * 2) -Fn: Network.  For each group of points we connect each point with every other point.
	 *   The modifiers a,f,s control what the "group" is.  With s, we construct a separate
	 *   network for each segment, with f we group all segments in a file and construct a
	 *   network for all those points, while with a with consider all points in the dataset
	 *   to be one group. So only a|f|s is allowed.
	 * 3) -Fr: Ref point.  Here, we construct line segments from the given reference point to
	 *   each of the points in the file.  If refpoint is given as two slash-separated coordinates
	 *   then the refpoint is fixed throughout this construction.  However, refpoint may also be
	 *   given as a, f, s and if so we pick the first point in the dataset, or first point in each
	 *   file, or the first point in each segment to update the actual reference point.
	 * 4) -Fv: Vectorize.  Here, consecutive points are turned into vector segments such as used
	 *   by psxy -Sv+s or external applications.  Again, appending a|f|s controls if we should
	 *   honor the segment headers [Default is -Fvs if -Fv is given]. Only a|f|s is allowed.
	 */

	unsigned int k, errors = 0;
	switch (in_arg[0]) {	/* First set method */
		case 'c': k = 1;	S->method  = SEGM_CONTINUOUS;	break;
		case 'n': k = 1;	S->method  = SEGM_NETWORK;	break;
		case 'r': k = 1;	S->method  = SEGM_REFPOINT;	break;
		case 'v': k = 1;	S->method  = SEGM_VECTOR;	break;
		default:  k = 0;	S->method  = SEGM_CONTINUOUS;	break;
	}

	switch (in_arg[k]) {	/* Now set level */
		case 's': case '\0': S->level = SEGM_SEGMENT;	break;
		case 'a': S->level = SEGM_DATASET;	break;
		case 'f': S->level = SEGM_TABLE;	break;
		case 'r': S->level = SEGM_RECORD;	break;
		default:	/* Must be a reference point but only if method is refpoint */
			if (S->method == SEGM_REFPOINT && strchr (in_arg, '/')) {	/* Gave arguments for an origin */
				S->level = SEGM_ORIGIN;
				if ((k = gmt_get_pair (GMT, &in_arg[k], GMT_PAIR_COORD, S->origin)) < 2) errors++;
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Expected reference point coordinates but got this: %s\n", option, &in_arg[k]);
				errors++;
			}
			break;
	}
	if (S->method == SEGM_CONTINUOUS && S->level == SEGM_SEGMENT) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Selecting -Fc, -Fs, or -Fcs yields no change\n", option);
		errors++;
	}
	if (S->method != SEGM_REFPOINT && S->level == SEGM_RECORD) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Only -Fr may accept refpoint = r\n", option);
		errors++;
	}
	if (mode == 1 && S->method == SEGM_VECTOR)	/* Only available for gmtconvert */
		errors++;
	return (errors);
}

/*! . */
int gmt_check_binary_io (struct GMT_CTRL *GMT, uint64_t n_req) {
	int n_errors = 0;

	/* Check the binary options that are used with most GMT programs.
	 * GMT is the pointer to the GMT structure.
	 * n_req is the number of required columns. If 0 then it relies on
	 *    GMT->common.b.ncol[GMT_IN] to be non-zero.
	 * Return value is the number of errors that where found.
	 */

	if (!GMT->common.b.active[GMT_IN]) return (GMT_NOERROR);	/* Let machinery figure out input cols for ASCII */

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

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Provides %d, expects %d-column binary data\n", GMT->common.b.ncol[GMT_IN], n_req);

	return (n_errors);
}

/*! . */
int gmt_parse_g_option (struct GMT_CTRL *GMT, char *txt) {
	int i, kk, k = 0, c, gap = 0;
	char *t = NULL;
	/* Process the GMT gap detection option for parameters (leading [+|-] for gap is deprecated) */
	/* Syntax, e.g., -g[x|X|y|Y|d|D|[<col>]z]<gap>[d|m|s|e|f|k|M|n|c|i|p][+n|p] or -ga */

	if (!txt || !txt[0]) return (GMT_PARSE_ERROR);	/* -g requires an argument */
	if ((i = GMT->common.g.n_methods) == GMT_N_GAP_METHODS) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot specify more than %d gap criteria\n", GMT_N_GAP_METHODS);
		return (1);
	}
	strncpy (GMT->common.g.string, txt, GMT_LEN64-1);	/* Verbatim copy */

	gmt_set_segmentheader (GMT, GMT_OUT, true);	/* -g gap checking implies -mo if not already set */

	if (txt[0] == 'a') {	/* For multiple criteria, specify that all criteria be met [default is any] */
		k++;
		GMT->common.g.match_all = true;
		if (!txt[k]) return (1);	/* Just a single -ga */
	}
	
	if ((t = strstr (txt, "+n"))) {	/* Gave modifier +n for negative gap check */
		gap = -1;
		t[0] = '\0';	/* Chop off modifier */
	}
	else if ((t = strstr (txt, "+p"))) {	/* Gave modifier +p for positive gap check */
		gap = +1;
		t[0] = '\0';	/* Chop off modifier */
	}
	
	/* Check if we have deprecated syntax and if that is allowed by the compatibility setting */
	kk = k;
	while (txt[kk] && isdigit ((int)txt[kk])) kk++;	/* Skip past until we find z */
	if (strchr ("xXyYdDz", txt[kk]) && strchr ("-+", txt[kk+1])) {	/* Deprecated way of setting the gap */
		if (gmt_M_compat_check (GMT, 6))
			GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Leading sign for gaps in -g is deprecated; use modifiers +n or +p instead\n");
		else if (txt[kk+1] == '-') {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gap value is negative; see -g syntax\n");
			if (t) t[0] = '+';	/* Restore modifiers */
			return (1);
		}
		if (gap == 0) gap = (txt[kk+1] == '-') ? -1 : +1;	/* Use the gap variable in the tests below */
	}
	
	switch (txt[k]) {	/* Determine method used for gap detection */
		case 'x':	/* Difference in user's x-coordinates used for test */
			GMT->common.g.method[i] = (gap == -1) ? GMT_NEGGAP_IN_COL : ((gap == +1) ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = GMT_X;
			break;
		case 'X':	/* Difference in user's mapped x-coordinates used for test */
			GMT->common.g.method[i] = (gap == -1) ? GMT_NEGGAP_IN_MAP_COL : ((gap == +1) ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			GMT->common.g.col[i] = GMT_X;
			break;
		case 'y':	/* Difference in user's y-coordinates used for test */
			GMT->common.g.method[i] = (gap == -1) ? GMT_NEGGAP_IN_COL : ((gap == +1) ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = GMT_Y;
			break;
		case 'Y':	/* Difference in user's mapped y-coordinates used for test */
			GMT->common.g.method[i] = (gap == -1) ? GMT_NEGGAP_IN_MAP_COL : ((gap == +1) ? GMT_POSGAP_IN_MAP_COL : GMT_ABSGAP_IN_MAP_COL);
			GMT->common.g.col[i] = GMT_Y;
			break;
		case 'd':	/* Great circle (if geographic data) or Cartesian distance used for test */
			GMT->common.g.method[i] = (gmt_M_is_geographic (GMT, GMT_IN)) ? GMT_GAP_IN_GDIST : GMT_GAP_IN_CDIST;
			GMT->common.g.col[i] = -1;
			break;
		case 'D':	/* Cartesian mapped distance used for test */
			GMT->common.g.method[i] = GMT_GAP_IN_PDIST;
			GMT->common.g.col[i] = -1;
			break;
		case 'z':	/* Difference in user's z-coordinates used for test */
			GMT->common.g.method[i] = (gap == -1) ? GMT_NEGGAP_IN_COL : ((gap == +1) ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
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
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", txt[k]);
				if (t) t[0] = '+';	/* Restore modifiers */
				return (1);
			}
			GMT->common.g.method[i] = (gap == -1) ? GMT_NEGGAP_IN_COL : ((gap == +1) ? GMT_POSGAP_IN_COL : GMT_ABSGAP_IN_COL);
			GMT->common.g.col[i] = atoi (&txt[c]);
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad gap selector (%c).  Choose from x|y|d|X|Y|D|[<col>]z\n", txt[0]);
			if (t) t[0] = '+';	/* Restore modifiers */
			return (1);
			break;
	}
	switch (GMT->common.g.method[i]) {
		case GMT_NEGGAP_IN_COL:
			GMT->common.g.get_dist[i] = &gmtinit_neg_col_dist;
			break;
		case GMT_POSGAP_IN_COL:
			GMT->common.g.get_dist[i] = &gmtinit_pos_col_dist;
			break;
		case GMT_ABSGAP_IN_COL:
			GMT->common.g.get_dist[i] = &gmtinit_abs_col_dist;
			break;
		case GMT_NEGGAP_IN_MAP_COL:
			GMT->common.g.get_dist[i] = &gmtinit_neg_col_map_dist;
			break;
		case GMT_POSGAP_IN_MAP_COL:
			GMT->common.g.get_dist[i] = &gmtinit_pos_col_map_dist;
			break;
		case GMT_ABSGAP_IN_MAP_COL:
			GMT->common.g.get_dist[i] = &gmtinit_abs_col_map_dist;
			break;
		case GMT_GAP_IN_GDIST:
			GMT->common.g.get_dist[i] = &gmtinit_xy_true_dist;
			break;
		case GMT_GAP_IN_CDIST:
			GMT->common.g.get_dist[i] = &gmtinit_xy_cart_dist;
			break;
		case GMT_GAP_IN_PDIST:
			GMT->common.g.get_dist[i] = &gmtinit_xy_map_dist;
			break;
		default:
			break;	/* Already set, or will be reset below  */
	}
	k++;	/* Skip to start of gap value */
	if (txt[k] == '-' || txt[k] == '+') k++;	/* Skip sign */
	if ((GMT->common.g.gap[i] = atof (&txt[k])) == 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Gap value must be non-zero\n");
		if (t) t[0] = '+';	/* Restore modifiers */
		return (1);
	}
	if (GMT->common.g.method[i] == GMT_GAP_IN_GDIST) {	/* Convert any gap given to meters */
		switch (txt[strlen(txt)-1]) {	/* Process unit information */
			case 'd':	/* Arc degrees, reset pointer */
				GMT->common.g.get_dist[i] = &gmtinit_xy_deg_dist;
				GMT->common.g.method[i] = GMT_GAP_IN_DDIST;
				break;
			case 'm':	/* Arc minutes, reset pointer */
				GMT->common.g.get_dist[i] = &gmtinit_xy_deg_dist;
				GMT->common.g.method[i] = GMT_GAP_IN_DDIST;
				GMT->common.g.gap[i] *= GMT_MIN2DEG;
				break;
			case 's':	/* Arc seconds, reset pointer */
				GMT->common.g.get_dist[i] = &gmtinit_xy_deg_dist;
				GMT->common.g.method[i] = GMT_GAP_IN_DDIST;
				GMT->common.g.gap[i] *= GMT_SEC2DEG;
				break;
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
	if ((uint64_t)(GMT->common.g.col[i] + 1) > GMT->common.g.n_col) GMT->common.g.n_col = (uint64_t)(GMT->common.g.col[i] + 1);	/* Needed when checking since it may otherwise not be read */
	GMT->common.g.n_methods++;
	if (t) t[0] = '+';	/* Restore modifiers */
	return (GMT_NOERROR);
}

/*! . */
char gmt_set_V (int mode) {
	char val = 0;
	switch (mode) {
		case GMT_MSG_QUIET:			val = 'q'; break;
		case GMT_MSG_NORMAL:		val = 'n'; break;
		case GMT_MSG_TICTOC:		val = 't'; break;
		case GMT_MSG_COMPAT:		val = 'c'; break;
		case GMT_MSG_VERBOSE:		val = 'v'; break;
		case GMT_MSG_LONG_VERBOSE:	val = 'l'; break;
		case GMT_MSG_DEBUG:			val = 'd'; break;
		default: break;
	}
	return val;
}

EXTERN_MSC int GMT_get_V (char arg);		/* For backward compatibility in MEX for 5.2 */
int GMT_get_V (char arg) {
	return gmt_get_V (arg);
}

/*! . */
int gmt_loaddefaults (struct GMT_CTRL *GMT, char *file) {
	static int gmt_version_major = GMT_PACKAGE_VERSION_MAJOR;
	unsigned int error = 0, rec = 0, ver;
	char line[GMT_BUFSIZ] = {""}, keyword[GMT_LEN256] = {""}, value[GMT_BUFSIZ] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (file, "r")) == NULL) return (-1);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Reading GMT Default parameters from file: %s\n", file);
	while (fgets (line, GMT_BUFSIZ, fp)) {
		rec++;
		gmt_chop (line); /* Get rid of [\r]\n */
		if (rec == 1 && (line[0] == 'S' || line[0] == 'U'))	{	/* An old GMT4 gmt.conf got in the way */
			fclose (fp);
			return (GMT_NOERROR);
		}

		if (rec != 2) { /* Nothing */ }
		else if (strlen (line) < 7 || (ver = strtol (&line[6], NULL, 10)) < 5 )
			gmt_message (GMT, "Your gmt.conf file (%s) may not be GMT %d compatible\n", file, gmt_version_major);
		else if (!strncmp (&line[6], "5.0.0", 5))
			gmt_message (GMT, "Your gmt.conf file (%s) is of version 5.0.0 and may need to be updated. Use \"gmtset -G%s\"\n", file, file);
		if (line[0] == '#') continue;	/* Skip comments */
		if (line[0] == '\0') continue;	/* Skip Blank lines */

		keyword[0] = value[0] = '\0';	/* Initialize */
		sscanf (line, "%s = %[^\n]", keyword, value);

		if (gmtlib_setparameter (GMT, keyword, value, false))
			error++;
		else {
			int case_val = gmt_hash_lookup (GMT, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
			if (case_val >= 0) GMT_keywords_updated[case_val] = true;		/* Leave a record that this keyword is no longer a default one */
		}
	}

	fclose (fp);
	gmtinit_verify_encodings (GMT);

	if (error) gmt_message (GMT, "%d GMT Defaults conversion errors in file %s!\n", error, file);

	return (GMT_NOERROR);
}

void gmtinit_update_keys (struct GMT_CTRL *GMT, bool arg) {
	gmt_M_unused(GMT);
	if (arg == false)
		gmt_M_memset (GMT_keywords_updated, GMT_N_KEYS, bool);
	else {
		for (unsigned int k = 0; k < GMT_N_KEYS; k++)
			GMT_keywords_updated[k] = true;
	}
}

/*! . */
unsigned int gmt_setdefaults (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {
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
			n_errors += gmtlib_setparameter (GMT, opt->arg, &opt->arg[p+1], true);
			opt->arg[p] = '=';	/* Restore the equal sign */
		}
		else if (!param)			/* Keep parameter name */
			param = opt->arg;
		else {					/* This must be value */
			n_errors += gmtlib_setparameter (GMT, param, opt->arg, true);
			param = NULL;	/* Get ready for next parameter */
		}
	}

	if (param != NULL)	/* param should be NULL unless no value were added */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Last GMT Defaults parameter from command options had no value\n");

	if (n_errors) GMT_Report (GMT->parent, GMT_MSG_NORMAL, " %d GMT Defaults conversion errors from command options\n", n_errors);
	return (n_errors);
}

/*! . */
unsigned int gmtlib_setparameter (struct GMT_CTRL *GMT, const char *keyword, char *value, bool core) {
	unsigned int pos;
	size_t len;
	int i, ival, case_val, manual, limit;
	bool error = false, tf_answer = false;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, lower_value[GMT_BUFSIZ] = {""};

	double dval;
	gmt_M_unused(core);

	if (!value) return (1);		/* value argument missing */
	strncpy (lower_value, value, GMT_BUFSIZ-1);	/* Get a lower case version */
	gmt_str_tolower (lower_value);
	len = strlen (value);

	case_val = gmt_hash_lookup (GMT, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		/* FORMAT GROUP */
		case GMTCASE_INPUT_CLOCK_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_CLOCK_IN");
			break;
		case GMTCASE_FORMAT_CLOCK_IN:
			strncpy (GMT->current.setting.format_clock_in, value, GMT_LEN64-1);
			gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_in, &GMT->current.io.clock_input, 0);
			break;
		case GMTCASE_INPUT_DATE_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_DATE_IN");
			break;
		case GMTCASE_FORMAT_DATE_IN:
			strncpy (GMT->current.setting.format_date_in, value, GMT_LEN64-1);
			gmtlib_date_C_format (GMT, GMT->current.setting.format_date_in, &GMT->current.io.date_input, 0);
			break;
		case GMTCASE_OUTPUT_CLOCK_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_CLOCK_OUT");
			break;
		case GMTCASE_FORMAT_CLOCK_OUT:
			strncpy (GMT->current.setting.format_clock_out, value, GMT_LEN64-1);
			gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_out, &GMT->current.io.clock_output, 1);
			break;
		case GMTCASE_OUTPUT_DATE_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_DATE_OUT");
			break;
		case GMTCASE_FORMAT_DATE_OUT:
			strncpy (GMT->current.setting.format_date_out, value, GMT_LEN64-1);
			gmtlib_date_C_format (GMT, GMT->current.setting.format_date_out, &GMT->current.io.date_output, 1);
			break;
		case GMTCASE_OUTPUT_DEGREE_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_GEO_OUT");
			break;
		case GMTCASE_FORMAT_GEO_OUT:
			strncpy (GMT->current.setting.format_geo_out, value, GMT_LEN64-1);
			gmtlib_geo_C_format (GMT);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of gmt_begin */
			break;
		case GMTCASE_PLOT_CLOCK_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_CLOCK_MAP");
			break;
		case GMTCASE_FORMAT_CLOCK_MAP:
			strncpy (GMT->current.setting.format_clock_map, value, GMT_LEN64-1);
			gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_map, &GMT->current.plot.calclock.clock, 2);
			break;
		case GMTCASE_PLOT_DATE_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_DATE_MAP");
			break;
		case GMTCASE_FORMAT_DATE_MAP:
			strncpy (GMT->current.setting.format_date_map, value, GMT_LEN64-1);
			gmtlib_date_C_format (GMT, GMT->current.setting.format_date_map, &GMT->current.plot.calclock.date, 2);
			break;
		case GMTCASE_PLOT_DEGREE_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_GEO_MAP");
			break;
		case GMTCASE_FORMAT_GEO_MAP:
			strncpy (GMT->current.setting.format_geo_map, value, GMT_LEN64-1);
			gmtlib_plot_C_format (GMT);	/* Update format statements */
			break;
		case GMTCASE_FORMAT_TIME_MAP:
			error = gmtlib_setparameter (GMT, "FORMAT_TIME_PRIMARY_MAP", value, core) +
			        gmtlib_setparameter (GMT, "FORMAT_TIME_SECONDARY_MAP", value, core);
			break;
		case GMTCASE_TIME_FORMAT_PRIMARY:
			GMT_COMPAT_TRANSLATE ("FORMAT_TIME_PRIMARY_MAP");
			break;
		case GMTCASE_FORMAT_TIME_PRIMARY_MAP:
			strncpy (GMT->current.setting.format_time[GMT_PRIMARY], value, GMT_LEN64-1);
			break;
		case GMTCASE_TIME_FORMAT_SECONDARY:
			GMT_COMPAT_TRANSLATE ("FORMAT_TIME_SECONDARY_MAP");
			break;
		case GMTCASE_FORMAT_TIME_SECONDARY_MAP:
			strncpy (GMT->current.setting.format_time[GMT_SECONDARY], value, GMT_LEN64-1);
			break;
		case GMTCASE_D_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_FLOAT_OUT");
			break;
		case GMTCASE_FORMAT_FLOAT_OUT:
			gmtinit_parse_format_float_out (GMT, value);
			break;
		case GMTCASE_FORMAT_FLOAT_MAP:
			strncpy (GMT->current.setting.format_float_map, value, GMT_LEN64-1);
			break;
		case GMTCASE_UNIX_TIME_FORMAT:
			GMT_COMPAT_TRANSLATE ("FORMAT_TIME_STAMP");
			break;
		case GMTCASE_FORMAT_TIME_STAMP:
			strncpy (GMT->current.setting.format_time_stamp, value, GMT_LEN256-1);
			break;

		/* FONT GROUP */

		case GMTCASE_FONT:	/* Special to set all fonts */
			error = gmtlib_setparameter (GMT, "FONT_ANNOT_PRIMARY", value, core) +
			        gmtlib_setparameter (GMT, "FONT_ANNOT_SECONDARY", value, core) +
			        gmtlib_setparameter (GMT, "FONT_TITLE", value, core) +
			        gmtlib_setparameter (GMT, "FONT_TAG", value, core) +
			        gmtlib_setparameter (GMT, "FONT_HEADING", value, core) +
			        gmtlib_setparameter (GMT, "FONT_LABEL", value, core);
			/*      FONT_LOGO is purposely skipped */
			break;
		case GMTCASE_FONT_ANNOT:
			error = gmtlib_setparameter (GMT, "FONT_ANNOT_PRIMARY", value, core) +
			        gmtlib_setparameter (GMT, "FONT_ANNOT_SECONDARY", value, core);
			break;
		case GMTCASE_ANNOT_FONT_PRIMARY:
			GMT_COMPAT_TRANSLATE ("FONT_ANNOT_PRIMARY");
			break;
		case GMTCASE_FONT_ANNOT_PRIMARY:
			if (value[0] == '+') {
				/* When + is prepended, scale fonts, offsets and ticklengths relative to FONT_ANNOT_PRIMARY (except LOGO font) */
				double scale = GMT->current.setting.font_annot[GMT_PRIMARY].size;
				if (gmt_getfont (GMT, &value[1], &GMT->current.setting.font_annot[GMT_PRIMARY])) error = true;
				scale = GMT->current.setting.font_annot[GMT_PRIMARY].size / scale;
				GMT->current.setting.font_annot[GMT_SECONDARY].size *= scale;
				GMT->current.setting.font_label.size *= scale;
				GMT->current.setting.font_heading.size *= scale;
				GMT->current.setting.font_tag.size *= scale;
				GMT->current.setting.font_title.size *= scale;
				GMT->current.setting.map_annot_offset[GMT_PRIMARY] *= scale;
				GMT->current.setting.map_annot_offset[GMT_SECONDARY] *= scale;
				GMT->current.setting.map_heading_offset *= scale;
				GMT->current.setting.map_label_offset *= scale;
				GMT->current.setting.map_title_offset *= scale;
				GMT->current.setting.map_frame_width *= scale;
				GMT->current.setting.map_tick_length[GMT_PRIMARY] *= scale;
				GMT->current.setting.map_tick_length[GMT_SECONDARY] *= scale;
				GMT->current.setting.map_tick_length[2] *= scale;
				GMT->current.setting.map_tick_length[3] *= scale;
				if (core) {		/* Need to update more than just FONT_ANNOT_PRIMARY */
					int p = gmt_hash_lookup (GMT, "FONT_ANNOT_SECONDARY", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_ANNOT_OFFSET_PRIMARY", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_ANNOT_OFFSET_SECONDARY", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "FONT_LABEL", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_LABEL_OFFSET", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "FONT_TITLE", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_TITLE_OFFSET", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_TICK_LENGTH_PRIMARY", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_TICK_LENGTH_SECONDARY", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
					p = gmt_hash_lookup (GMT, "MAP_FRAME_WIDTH", keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);
					if (p >= 0) GMT_keywords_updated[p] = true;		/* Leave a record that this keyword is no longer a default one */
				}
			}
			else
				if (gmt_getfont (GMT, value, &GMT->current.setting.font_annot[GMT_PRIMARY])) error = true;
			break;
		case GMTCASE_ANNOT_FONT_SECONDARY:
			GMT_COMPAT_TRANSLATE ("FONT_ANNOT_SECONDARY");
			break;
		case GMTCASE_FONT_ANNOT_SECONDARY:
			if (gmt_getfont (GMT, value, &GMT->current.setting.font_annot[GMT_SECONDARY])) error = true;
			break;
		case GMTCASE_FONT_HEADING:
			if (gmt_getfont (GMT, value, &GMT->current.setting.font_heading)) error = true;
			break;
		case GMTCASE_FONT_TITLE:
			if (gmt_getfont (GMT, value, &GMT->current.setting.font_title)) error = true;
			break;
		case GMTCASE_FONT_TAG:
			if (gmt_getfont (GMT, value, &GMT->current.setting.font_tag)) error = true;
			break;
		case GMTCASE_LABEL_FONT:
			GMT_COMPAT_TRANSLATE ("FONT_LABEL");
			break;
		case GMTCASE_FONT_LABEL:
			if (gmt_getfont (GMT, value, &GMT->current.setting.font_label)) error = true;
			break;
		case GMTCASE_FONT_LOGO:
			if (gmt_getfont (GMT, value, &GMT->current.setting.font_logo)) error = true;
			break;

		/* FONT GROUP ... obsolete options */

		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY:
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_COMPAT_CHANGE ("FONT_ANNOT_PRIMARY");
				dval = gmt_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0) {
					GMT->current.setting.font_annot[GMT_PRIMARY].size = dval;
					GMT_KEYWORD_UPDATE (GMTCASE_FONT_ANNOT_PRIMARY);
				}
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY:
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_COMPAT_CHANGE ("FONT_ANNOT_SECONDARY");
				dval = gmt_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0) {
					GMT->current.setting.font_annot[GMT_SECONDARY].size = dval;
					GMT_KEYWORD_UPDATE (GMTCASE_FONT_ANNOT_SECONDARY);
				}
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_HEADER_FONT_SIZE:
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_COMPAT_CHANGE ("FONT_TITLE");
				dval = gmt_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0) {
					GMT->current.setting.font_title.size = dval;
					GMT_KEYWORD_UPDATE (GMTCASE_FONT_TITLE);
				}
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_LABEL_FONT_SIZE:
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_COMPAT_CHANGE ("FONT_LABEL");
				dval = gmt_convert_units (GMT, value, GMT_PT, GMT_PT);
				if (dval > 0.0) {
					GMT->current.setting.font_label.size = dval;
					GMT_KEYWORD_UPDATE (GMTCASE_FONT_LABEL);
				}
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;

		/* MAP GROUP */

		case GMTCASE_ANNOT_OFFSET_PRIMARY:
			GMT_COMPAT_TRANSLATE ("MAP_ANNOT_OFFSET_PRIMARY");
			break;
		case GMTCASE_MAP_ANNOT_OFFSET:
			error = gmtlib_setparameter (GMT, "MAP_ANNOT_OFFSET_PRIMARY", value, core) +
			        gmtlib_setparameter (GMT, "MAP_ANNOT_OFFSET_SECONDARY", value, core);
			break;
		case GMTCASE_MAP_ANNOT_OFFSET_PRIMARY:
			GMT->current.setting.map_annot_offset[GMT_PRIMARY] = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_ANNOT_OFFSET_SECONDARY:
			GMT_COMPAT_TRANSLATE ("MAP_ANNOT_OFFSET_SECONDARY");
			break;
		case GMTCASE_MAP_ANNOT_OFFSET_SECONDARY:
			GMT->current.setting.map_annot_offset[GMT_SECONDARY] = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_OBLIQUE_ANNOTATION:
			GMT_COMPAT_TRANSLATE ("MAP_ANNOT_OBLIQUE");
			break;
		case GMTCASE_MAP_ANNOT_OBLIQUE:
			ival = atoi (value);
			if (ival >= GMT_OBL_ANNOT_LON_X_LAT_Y && ival < GMT_OBL_ANNOT_FLAG_LIMIT)
				GMT->current.setting.map_annot_oblique = ival;
			else
				error = true;
			break;
		case GMTCASE_ANNOT_MIN_ANGLE:
			GMT_COMPAT_TRANSLATE ("MAP_ANNOT_MIN_ANGLE");
			break;
		case GMTCASE_MAP_ANNOT_MIN_ANGLE:
			dval = atof (value);
			if (dval < 0.0)
				error = true;
			else
				GMT->current.setting.map_annot_min_angle = dval;
			break;
		case GMTCASE_ANNOT_MIN_SPACING:
			GMT_COMPAT_TRANSLATE ("MAP_ANNOT_MIN_SPACING");
			break;
		case GMTCASE_MAP_ANNOT_MIN_SPACING:
			if (value[0] == '-')	/* Negative */
				error = true;
			else
				GMT->current.setting.map_annot_min_spacing = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_Y_AXIS_TYPE:
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_COMPAT_CHANGE ("MAP_ANNOT_ORTHO");
				if (!strcmp (lower_value, "ver_text")) {
					strncpy (GMT->current.setting.map_annot_ortho, "", 5U);
					GMT_KEYWORD_UPDATE (GMTCASE_MAP_ANNOT_ORTHO);
				}
				else if (!strcmp (lower_value, "hor_text")) {
					strncpy (GMT->current.setting.map_annot_ortho, "we", 5U);
					GMT_KEYWORD_UPDATE (GMTCASE_MAP_ANNOT_ORTHO);
				}
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_MAP_ANNOT_ORTHO:
			strncpy (GMT->current.setting.map_annot_ortho, lower_value, 5U);
			break;
		case GMTCASE_DEGREE_SYMBOL:
			GMT_COMPAT_TRANSLATE ("MAP_DEGREE_SYMBOL");
			break;
		case GMTCASE_MAP_DEGREE_SYMBOL:
			if (value[0] == '\0' || !strcmp (lower_value, "degree"))	/* Default */
				GMT->current.setting.map_degree_symbol = gmt_degree;
			else if (!strcmp (lower_value, "ring"))
				GMT->current.setting.map_degree_symbol = gmt_ring;
			else if (!strcmp (lower_value, "colon"))
				GMT->current.setting.map_degree_symbol = gmt_colon;
			else if (!strcmp (lower_value, "none"))
				GMT->current.setting.map_degree_symbol = gmt_none;
			else
				error = true;
			gmtlib_plot_C_format (GMT);	/* Update format statement since degree symbol may have changed */
			break;
		case GMTCASE_BASEMAP_AXES:
			GMT_COMPAT_TRANSLATE ("MAP_FRAME_AXES");
			break;
		case GMTCASE_MAP_FRAME_AXES:
			strncpy (GMT->current.setting.map_frame_axes, value, 5U);
			for (i = 0; i < 5; i++) GMT->current.map.frame.side[i] = 0;	/* Unset default settings */
			GMT->current.map.frame.draw_box = false;
			error += (bool)gmtinit_decode5_wesnz (GMT, value, false);
			break;

		case GMTCASE_BASEMAP_FRAME_RGB:
			GMT_COMPAT_TRANSLATE ("MAP_DEFAULT_PEN");
			break;
		case GMTCASE_MAP_DEFAULT_PEN:
			i = (value[0] == '+') ? 1 : 0;	/* If plus is added, copy color to MAP_*_PEN settings */
			error = gmt_getpen (GMT, &value[i], &GMT->current.setting.map_default_pen);
			if (i == 1) {
				gmt_M_rgb_copy (&GMT->current.setting.map_grid_pen[GMT_PRIMARY].rgb, &GMT->current.setting.map_default_pen.rgb);
				gmt_M_rgb_copy (&GMT->current.setting.map_grid_pen[GMT_SECONDARY].rgb, &GMT->current.setting.map_default_pen.rgb);
				gmt_M_rgb_copy (&GMT->current.setting.map_frame_pen.rgb  , &GMT->current.setting.map_default_pen.rgb);
				gmt_M_rgb_copy (&GMT->current.setting.map_tick_pen[GMT_PRIMARY].rgb, &GMT->current.setting.map_default_pen.rgb);
				gmt_M_rgb_copy (&GMT->current.setting.map_tick_pen[GMT_SECONDARY].rgb, &GMT->current.setting.map_default_pen.rgb);
			}
			break;
		case GMTCASE_FRAME_PEN:
			GMT_COMPAT_TRANSLATE ("MAP_FRAME_PEN");
			break;
		case GMTCASE_MAP_FRAME_PEN:
			error = gmt_getpen (GMT, value, &GMT->current.setting.map_frame_pen);
			break;
		case GMTCASE_BASEMAP_TYPE:
			GMT_COMPAT_TRANSLATE ("MAP_FRAME_TYPE");
			break;
		case GMTCASE_MAP_FRAME_TYPE:
			if (!strcmp (lower_value, "plain"))
				GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
			else if (!strncmp (lower_value, "graph", 5U)) {
				char *c = NULL;
				GMT->current.setting.map_frame_type = GMT_IS_GRAPH;
				if ((c = strchr (lower_value, ','))) {	/* Also specified vector extension setting */
					size_t last = strlen (lower_value) - 1;
					char unit = lower_value[last];
					if (unit == GMT_GRAPH_EXTENSION_UNIT) {	/* Want extension as percentage of axis length */
						GMT->current.setting.map_graph_extension_unit = unit;
						GMT->current.setting.map_graph_extension = atof (&c[1]);
					}
					else {	/* Read and convert value with unit to inches */
						GMT->current.setting.map_graph_extension_unit = gmtlib_unit_lookup (GMT, unit, GMT->current.setting.proj_length_unit);	/* Will warn if c is not 0, 'c', 'i', 'p' */
						GMT->current.setting.map_graph_extension = gmt_M_to_inch (GMT, &c[1]);
					}
				}
				else {	/* Reset to default settings */
					GMT->current.setting.map_graph_extension_unit = GMT_GRAPH_EXTENSION_UNIT;
					GMT->current.setting.map_graph_extension = GMT_GRAPH_EXTENSION;
				}
			}
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
			GMT_COMPAT_TRANSLATE ("MAP_FRAME_WIDTH");
			break;
		case GMTCASE_MAP_FRAME_WIDTH:
			dval = gmt_M_to_inch (GMT, value);
			if (dval > 0.0)
				GMT->current.setting.map_frame_width = dval;
			else
				error = true;
			break;
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY:
			GMT_COMPAT_TRANSLATE ("MAP_GRID_CROSS_SIZE_PRIMARY");
			break;
		case GMTCASE_MAP_GRID_CROSS_SIZE:
			dval = gmt_M_to_inch (GMT, value);
			if (dval >= 0.0)
				GMT->current.setting.map_grid_cross_size[GMT_PRIMARY] = GMT->current.setting.map_grid_cross_size[GMT_SECONDARY] = dval;
			else
				error = true;
			break;
		case GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY:
			dval = gmt_M_to_inch (GMT, value);
			if (dval >= 0.0)
				GMT->current.setting.map_grid_cross_size[GMT_PRIMARY] = dval;
			else
				error = true;
			break;
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY:
			GMT_COMPAT_TRANSLATE ("MAP_GRID_CROSS_SIZE_SECONDARY");
			break;
		case GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY:
			dval = gmt_M_to_inch (GMT, value);
			if (dval >= 0.0)
				GMT->current.setting.map_grid_cross_size[GMT_SECONDARY] = dval;
			else
				error = true;
			break;
		case GMTCASE_MAP_GRID_PEN:
			error = gmtlib_setparameter (GMT, "MAP_GRID_PEN_PRIMARY", value, core) +
			        gmtlib_setparameter (GMT, "MAP_GRID_PEN_SECONDARY", value, core);
			break;
		case GMTCASE_GRID_PEN_PRIMARY:
			GMT_COMPAT_TRANSLATE ("MAP_GRID_PEN_PRIMARY");
			break;
		case GMTCASE_MAP_GRID_PEN_PRIMARY:
			error = gmt_getpen (GMT, value, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
			break;
		case GMTCASE_GRID_PEN_SECONDARY:
			GMT_COMPAT_TRANSLATE ("MAP_GRID_PEN_SECONDARY");
			break;
		case GMTCASE_MAP_GRID_PEN_SECONDARY:
			error = gmt_getpen (GMT, value, &GMT->current.setting.map_grid_pen[GMT_SECONDARY]);
			break;
		case GMTCASE_MAP_HEADING_OFFSET:
			GMT->current.setting.map_heading_offset = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_LABEL_OFFSET:
			GMT_COMPAT_TRANSLATE ("MAP_LABEL_OFFSET");
			break;
		case GMTCASE_MAP_LABEL_OFFSET:
			GMT->current.setting.map_label_offset = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_LINE_STEP:
			GMT_COMPAT_TRANSLATE ("MAP_LINE_STEP");
			break;
		case GMTCASE_MAP_LINE_STEP:
			if ((GMT->current.setting.map_line_step = gmt_M_to_inch (GMT, value)) <= 0.0) {
				GMT->current.setting.map_line_step = 0.01;
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "%s <= 0, reset to %g %s\n", keyword, GMT->current.setting.map_line_step, GMT->session.unit_name[GMT_INCH]);
			}
			break;
		case GMTCASE_UNIX_TIME:
			GMT_COMPAT_TRANSLATE ("MAP_LOGO");
			break;
		case GMTCASE_MAP_LOGO:
			error = gmtinit_true_false_or_error (lower_value, &GMT->current.setting.map_logo);
			break;
		case GMTCASE_UNIX_TIME_POS:
			GMT_COMPAT_TRANSLATE ("MAP_LOGO_POS");
			break;
		case GMTCASE_MAP_LOGO_POS:
			i = sscanf (value, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (i == 2) {
				GMT->current.setting.map_logo_pos[GMT_X] = gmt_M_to_inch (GMT, txt_a);
				GMT->current.setting.map_logo_pos[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
			}
			else if (i == 3) {	/* New style, includes justification, introduced in GMT 4.3.0 */
				GMT->current.setting.map_logo_justify = gmt_just_decode (GMT, txt_a, PSL_NO_DEF);
				GMT->current.setting.map_logo_pos[GMT_X] = gmt_M_to_inch (GMT, txt_b);
				GMT->current.setting.map_logo_pos[GMT_Y] = gmt_M_to_inch (GMT, txt_c);
			}
			else
				error = true;
			break;
		case GMTCASE_X_ORIGIN:
			GMT_COMPAT_TRANSLATE ("MAP_ORIGIN_X");
			break;
		case GMTCASE_MAP_ORIGIN_X:
			GMT->current.setting.map_origin[GMT_X] = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_Y_ORIGIN:
			GMT_COMPAT_TRANSLATE ("MAP_ORIGIN_Y");
			break;
		case GMTCASE_MAP_ORIGIN_Y:
			GMT->current.setting.map_origin[GMT_Y] = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_POLAR_CAP:
			GMT_COMPAT_TRANSLATE ("MAP_POLAR_CAP");
			break;
		case GMTCASE_MAP_POLAR_CAP:
			if (!strcmp (lower_value, "none")) {	/* Means reset to no cap -> lat = 90, dlon = 0 */
				GMT->current.setting.map_polar_cap[0] = 90.0;
				GMT->current.setting.map_polar_cap[1] = 0.0;
			}
			else {
				double inc[2];
				i = sscanf (lower_value, "%[^/]/%s", txt_a, txt_b);
				if (i != 2)
					error = true;
				else
					error = gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_a, GMT_IS_LAT, &GMT->current.setting.map_polar_cap[0]), txt_a);
				if (gmt_getinc (GMT, txt_b, inc)) error = true;
				GMT->current.setting.map_polar_cap[1] = inc[GMT_X];
			}
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			dval = gmt_M_to_inch (GMT, value);
			if (dval <= 0.0)
				error = true;
			else
				GMT->current.setting.map_scale_height = dval;
			break;
		case GMTCASE_TICK_LENGTH:
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_CHANGE ("MAP_TICK_LENGTH");
				GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = gmt_M_to_inch (GMT, value);
				GMT->current.setting.map_tick_length[GMT_TICK_UPPER]  = 0.50 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
				GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = 3.00 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
				GMT->current.setting.map_tick_length[GMT_TICK_LOWER]  = 0.75 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
				GMT_KEYWORD_UPDATE (GMTCASE_MAP_TICK_LENGTH_PRIMARY);
				GMT_KEYWORD_UPDATE (GMTCASE_MAP_TICK_LENGTH_SECONDARY);
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_MAP_TICK_LENGTH:
			error = gmtlib_setparameter (GMT, "MAP_TICK_LENGTH_PRIMARY", value, core) +
			        gmtlib_setparameter (GMT, "MAP_TICK_LENGTH_SECONDARY", value, core);
			break;
		case GMTCASE_MAP_TICK_LENGTH_PRIMARY:
			i = sscanf (value, "%[^/]/%s", txt_a, txt_b);
			GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = gmt_M_to_inch (GMT, txt_a);
			GMT->current.setting.map_tick_length[GMT_TICK_UPPER]  = (i > 1) ? gmt_M_to_inch (GMT, txt_b) : 0.50 * GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];
			break;
		case GMTCASE_MAP_TICK_LENGTH_SECONDARY:
			i = sscanf (value, "%[^/]/%s", txt_a, txt_b);
			GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = gmt_M_to_inch (GMT, txt_a);
			GMT->current.setting.map_tick_length[GMT_TICK_LOWER]  = (i > 1) ? gmt_M_to_inch (GMT, txt_b) : 0.25 * GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER];
			break;
		case GMTCASE_TICK_PEN:
			GMT_COMPAT_TRANSLATE ("MAP_TICK_PEN");
			break;
		case GMTCASE_MAP_TICK_PEN:
			error = gmtlib_setparameter (GMT, "MAP_TICK_PEN_PRIMARY", value, core) +
			        gmtlib_setparameter (GMT, "MAP_TICK_PEN_SECONDARY", value, core);
			break;
		case GMTCASE_MAP_TICK_PEN_PRIMARY:
			error = gmt_getpen (GMT, value, &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);
			break;
		case GMTCASE_MAP_TICK_PEN_SECONDARY:
			error = gmt_getpen (GMT, value, &GMT->current.setting.map_tick_pen[GMT_SECONDARY]);
			break;
		case GMTCASE_HEADER_OFFSET:
			GMT_COMPAT_TRANSLATE ("MAP_TITLE_OFFSET");
			break;
		case GMTCASE_MAP_TITLE_OFFSET:
			GMT->current.setting.map_title_offset = gmt_M_to_inch (GMT, value);
			break;
		case GMTCASE_VECTOR_SHAPE:
			GMT_COMPAT_TRANSLATE ("MAP_VECTOR_SHAPE");
			break;
		case GMTCASE_MAP_VECTOR_SHAPE:
			dval = atof (value);
			if (dval < -2.0 || dval > 2.0)
				error = true;
			else
				GMT->current.setting.map_vector_shape = dval;
			break;

		/* COLOR GROUP */

		case GMTCASE_COLOR_BACKGROUND:
			error = gmt_getrgb (GMT, value, GMT->current.setting.color_patch[GMT_BGD]);
			break;
		case GMTCASE_COLOR_FOREGROUND:
			error = gmt_getrgb (GMT, value, GMT->current.setting.color_patch[GMT_FGD]);
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
			else if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
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
			error = gmt_getrgb (GMT, value, GMT->current.setting.color_patch[GMT_NAN]);
			break;
		case GMTCASE_HSV_MIN_SATURATION:
			GMT_COMPAT_TRANSLATE ("COLOR_HSV_MIN_S");
			break;
		case GMTCASE_COLOR_HSV_MIN_S:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_min_s = dval;
			break;
		case GMTCASE_HSV_MAX_SATURATION:
			GMT_COMPAT_TRANSLATE ("COLOR_HSV_MAX_S");
			break;
		case GMTCASE_COLOR_HSV_MAX_S:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_max_s = dval;
			break;
		case GMTCASE_HSV_MIN_VALUE:
			GMT_COMPAT_TRANSLATE ("COLOR_HSV_MIN_V");
			break;
		case GMTCASE_COLOR_HSV_MIN_V:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_min_v = dval;
			break;
		case GMTCASE_HSV_MAX_VALUE:
			GMT_COMPAT_TRANSLATE ("COLOR_HSV_MAX_V");
			break;
		case GMTCASE_COLOR_HSV_MAX_V:
			dval = atof (value);
			if (dval < 0.0 || dval > 1.0)
				error = true;
			else
				GMT->current.setting.color_hsv_max_v = dval;
			break;

		/* PS GROUP */

		case GMTCASE_CHAR_ENCODING:
			GMT_COMPAT_TRANSLATE ("PS_CHAR_ENCODING");
			break;
		case GMTCASE_PS_CHAR_ENCODING:
			strncpy (GMT->current.setting.ps_encoding.name, value, GMT_LEN64-1);
			gmtinit_load_encoding (GMT);
			gmtlib_plot_C_format (GMT);	/* Since a chance in char set chances what is degree, ring, etc. */
			break;
		case GMTCASE_PS_COLOR:
			GMT_COMPAT_TRANSLATE ("PS_COLOR_MODEL");
			break;
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
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				ival = atoi (value);
				if (ival > 0)
					GMT->current.setting.ps_copies = ival;
				else
					error = true;
			}
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_DOTS_PR_INCH:
		case GMTCASE_PS_DPI:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_EPS:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
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
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "PS_IMAGE_COMPRESS = DEFLATE not available, falling back to LZW.\n");
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
			GMT_COMPAT_TRANSLATE ("PS_PAGE_COLOR");
			break;
		case GMTCASE_PS_PAGE_COLOR:
			error = gmt_getrgb (GMT, value, GMT->current.setting.ps_page_rgb);
			break;
		case GMTCASE_PAGE_ORIENTATION:
			GMT_COMPAT_TRANSLATE ("PS_PAGE_ORIENTATION");
			break;
		case GMTCASE_PS_PAGE_ORIENTATION:
			if (GMT->current.setting.run_mode == GMT_CLASSIC) {	/* Ignore under modern mode */
				if (!strcmp (lower_value, "landscape"))
					GMT->current.setting.ps_orientation = PSL_LANDSCAPE;
				else if (!strcmp (lower_value, "portrait"))
					GMT->current.setting.ps_orientation = PSL_PORTRAIT;
				else
					error = true;
			}
			break;
		case GMTCASE_PAPER_MEDIA:
			GMT_COMPAT_TRANSLATE ("PS_MEDIA");
			break;
		case GMTCASE_PS_MEDIA:
			if (GMT->current.setting.run_mode == GMT_MODERN && !strcmp (lower_value, "auto")) {
				gmtinit_setautopagesize (GMT);
				break;
			}
			manual = false;
			len--;
			if (lower_value[len] == '-') {	/* Manual Feed selected */
				lower_value[len] = '\0';
				manual = true;
			}
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				if (lower_value[len] == '+') {	/* EPS format selected */
					lower_value[len] = '\0';
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Production of EPS format is no longer supported, remove + after paper size\n");
				}
			}
			if ((i = gmtinit_key_lookup (lower_value, GMT_media_name, GMT_N_MEDIA)) < GMT_N_MEDIA) {
				/* Use the specified standard format */
				GMT->current.setting.ps_media = i;
				GMT->current.setting.ps_page_size[0] = GMT_media[i].width;
				GMT->current.setting.ps_page_size[1] = GMT_media[i].height;
			}
			else if (gmtinit_load_user_media (GMT) &&
				(pos = gmtinit_key_lookup (lower_value, GMT->session.user_media_name, GMT->session.n_user_media)) < GMT->session.n_user_media) {
				/* Use a user-specified format */
					GMT->current.setting.ps_media = pos + USER_MEDIA_OFFSET;
					GMT->current.setting.ps_page_size[0] = GMT->session.user_media[pos].width;
					GMT->current.setting.ps_page_size[1] = GMT->session.user_media[pos].height;
				}
			else {
				/* A custom paper size in W x H points (or in inch/c if units are appended) */
				if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
					pos = (strncmp (lower_value, "custom_", 7U) ? 0 : 7);
				}
				else
					pos = 0;
				if (gmt_strtok (lower_value, "x", &pos, txt_a))	/* Returns width and update pos */
					GMT->current.setting.ps_page_size[0] = gmt_convert_units (GMT, txt_a, GMT_PT, GMT_PT);
				if (gmt_strtok (lower_value, "x", &pos, txt_b))	/* Returns height and update pos */
					GMT->current.setting.ps_page_size[1] = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
				if (GMT->current.setting.ps_page_size[0] <= 0.0) error = true;
				if (GMT->current.setting.ps_page_size[1] <= 0.0) error = true;
				GMT->current.setting.ps_media = -USER_MEDIA_OFFSET;
			}
			if (!error && manual) GMT->current.setting.ps_page_size[0] = -GMT->current.setting.ps_page_size[0];
			break;
		case GMTCASE_GLOBAL_X_SCALE:
			GMT_COMPAT_TRANSLATE ("PS_SCALE_X");
			break;
		case GMTCASE_PS_SCALE_X:
			dval = atof (value);
			if (dval > 0.0)
				GMT->current.setting.ps_magnify[GMT_X] = dval;
			else
				error = true;
			break;
		case GMTCASE_GLOBAL_Y_SCALE:
			GMT_COMPAT_TRANSLATE ("PS_SCALE_Y");
			break;
		case GMTCASE_PS_SCALE_Y:
			dval = atof (value);
			if (dval > 0.0)
				GMT->current.setting.ps_magnify[GMT_Y] = dval;
			else
				error = true;
			break;
		case GMTCASE_TRANSPARENCY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Transparency is now part of pen and fill specifications.  TRANSPARENCY is ignored\n");
			else
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_TRANSPARENCY:
			strncpy (GMT->current.setting.ps_transpmode, value, GMT_LEN16-1);
			break;
		case GMTCASE_PS_CONVERT:
			strncpy (GMT->current.setting.ps_convert, value, GMT_LEN256-1);
			break;
		case GMTCASE_PS_VERBOSE:
			GMT_COMPAT_TRANSLATE ("PS_COMMENTS");
			break;
		case GMTCASE_PS_COMMENTS:
			if (!GMT->PSL) return (0);	/* Not using PSL in this session */
			error = gmtinit_true_false_or_error (lower_value, &tf_answer);
			GMT->PSL->internal.comments = (tf_answer) ? 1 : 0;
			break;

		/* IO GROUP */

		case GMTCASE_FIELD_DELIMITER:
			GMT_COMPAT_TRANSLATE ("IO_COL_SEPARATOR");
			break;
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
		case GMTCASE_IO_FIRST_HEADER:
			if (!strcmp (lower_value, "maybe"))
				GMT->current.setting.io_first_header = GMT_FIRST_SEGHEADER_MAYBE;
			else if (!strcmp (lower_value, "always"))
				GMT->current.setting.io_first_header = GMT_FIRST_SEGHEADER_ALWAYS;
			else if (!strcmp (lower_value, "never"))
				GMT->current.setting.io_first_header = GMT_FIRST_SEGHEADER_NEVER;
			else
				error = true;
			break;
		case GMTCASE_GRIDFILE_FORMAT:
			GMT_COMPAT_TRANSLATE ("IO_GRIDFILE_FORMAT");
			break;
		case GMTCASE_IO_GRIDFILE_FORMAT:
			strncpy (GMT->current.setting.io_gridfile_format, value, GMT_LEN64-1);
			break;
		case GMTCASE_GRIDFILE_SHORTHAND:
			GMT_COMPAT_TRANSLATE ("IO_GRIDFILE_SHORTHAND");
			break;
		case GMTCASE_IO_GRIDFILE_SHORTHAND:
			error = gmtinit_true_false_or_error (lower_value, &GMT->current.setting.io_gridfile_shorthand);
			break;
		case GMTCASE_IO_HEADER:
			error = gmtinit_true_false_or_error (lower_value, &GMT->current.setting.io_header[GMT_IN]);
			GMT->current.setting.io_header[GMT_OUT] = GMT->current.setting.io_header[GMT_IN];
			break;
		case GMTCASE_IO_HEADER_MARKER:
			if (len == 0)	/* Blank gives default */
				GMT->current.setting.io_head_marker[GMT_OUT] = GMT->current.setting.io_head_marker[GMT_IN] = '#';
			else {
				int dir;
				char txt[2][GMT_LEN32];
				if (strchr (value, ',')) {	/* Got separate header record markers for input,output */
					sscanf (value, "%[^,],%s", txt[GMT_IN], txt[GMT_OUT]);
				}
				else {	/* Just duplicate */
					strncpy (txt[GMT_IN], value, GMT_LEN32-1);	strncpy (txt[GMT_OUT], value, GMT_LEN32-1);
				}
				for (dir = 0; dir < 2; dir++)
					GMT->current.setting.io_head_marker[dir] = txt[dir][0];
			}
			break;
		case GMTCASE_N_HEADER_RECS:
			GMT_COMPAT_TRANSLATE ("IO_N_HEADER_RECS");
			break;
		case GMTCASE_IO_N_HEADER_RECS:
			ival = atoi (value);
			if (ival < 0)
				error = true;
			else
				GMT->current.setting.io_n_header_items = ival;
			break;
		case GMTCASE_NAN_RECORDS:
			GMT_COMPAT_TRANSLATE ("IO_NAN_RECORDS");
			break;
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
			else if ((i = sscanf (value, "%" PRIuS " , %" PRIuS, /* Chunk size: vert,hor */
			         &GMT->current.setting.io_nc4_chunksize[0], &GMT->current.setting.io_nc4_chunksize[1])) > 0) {
				if (i == 1) /* Use chunk size for both horizontal and vertical dimension */
					GMT->current.setting.io_nc4_chunksize[1] = GMT->current.setting.io_nc4_chunksize[0];
				if (GMT->current.setting.io_nc4_chunksize[0] <= k_netcdf_io_chunked_auto ||
				    GMT->current.setting.io_nc4_chunksize[1] <= k_netcdf_io_chunked_auto)
					/* Chunk size too small */
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
			GMT_COMPAT_TRANSLATE ("IO_LONLAT_TOGGLE");
			break;
		case GMTCASE_IO_LONLAT_TOGGLE:
			if (!gmtinit_true_false_or_error (lower_value, &GMT->current.setting.io_lonlat_toggle[GMT_IN]))
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
					strncpy (txt[GMT_IN], value, GMT_LEN256-1);	strncpy (txt[GMT_OUT], value, GMT_LEN256-1);
				}
				for (dir = 0; dir < 2; dir++) {
					switch (txt[dir][0]) {
						case 'B':
							GMT->current.setting.io_blankline[dir] = true;
							GMT->current.setting.io_seg_marker[dir] = 'B';
							break;
						case 'N':
							GMT->current.setting.io_nanline[dir] = true;
							GMT->current.setting.io_seg_marker[dir] = 'N';
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
			if (!strncmp (lower_value, "none", 4U)) /* Use lat as is [backwards compatibility] */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_NONE;
			else if (!strncmp (lower_value, "geodetic", 8U)) /* Use lat as is */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_NONE;
			else if (!strncmp (lower_value, "authalic", 8U)) /* Authalic latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2A;
			else if (!strncmp (lower_value, "conformal", 9U)) /* Conformal latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2C;
			else if (!strncmp (lower_value, "geocentric", 10U)) /* Geocentric latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2O;
			else if (!strncmp (lower_value, "meridional", 10U)) /* Meridional latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2M;
			else if (!strncmp (lower_value, "parametric", 10U)) /* Parametric latitude */
				GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2P;
			else
				error = true;
			gmtlib_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
			break;

		case GMTCASE_ELLIPSOID:
			GMT_COMPAT_TRANSLATE ("PROJ_ELLIPSOID");
			break;
		case GMTCASE_PROJ_ELLIPSOID:
			ival = gmt_get_ellipsoid (GMT, value);
			if (ival < 0)
				error = true;
			else
				GMT->current.setting.proj_ellipsoid = ival;
			gmtlib_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
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
			gmtlib_init_geodesic (GMT);	/* Set function pointer depending on the geodesic selected */
			break;

		case GMTCASE_MEASURE_UNIT:
			GMT_COMPAT_TRANSLATE ("PROJ_LENGTH_UNIT");
			break;
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
			gmtlib_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
			break;

		case GMTCASE_MAP_SCALE_FACTOR:
			GMT_COMPAT_TRANSLATE ("PROJ_SCALE_FACTOR");
			break;
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
			limit = (GMT->current.setting.run_mode == GMT_CLASSIC) ? 4 : 6;
			if (ival < limit) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_COMPATIBILITY: Expects values from %d to %d; reset to %d.\n", limit, GMT_MAJOR_VERSION, limit);
				GMT->current.setting.compatibility = 4;
			}
			else if (ival > GMT_MAJOR_VERSION) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_COMPATIBILITY: Expects values from %d to %d; reset to %d.\n", limit, GMT_MAJOR_VERSION, GMT_MAJOR_VERSION);
				GMT->current.setting.compatibility = GMT_MAJOR_VERSION;
			}
			else
				GMT->current.setting.compatibility = ival;
			break;

		case GMTCASE_GMT_AUTO_DOWNLOAD:
			if (!strncmp (lower_value, "on", 3))
				GMT->current.setting.auto_download = GMT_YES_DOWNLOAD;
			else if (!strncmp (lower_value, "off", 3))
				GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_AUTO_DOWNLOAD: Expects either on or off\n");
				GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;
			}
			break;

		case GMTCASE_GMT_DATA_URL:	/* The default is set by cmake, see ConfigDefault.cmake */
			if (*value) {
				if (GMT->session.DATAURL) {
					if ((strcmp (GMT->session.DATAURL, value) == 0))
						break; /* stop here if string in place is equal */
					gmt_M_str_free (GMT->session.DATAURL);
				}
				/* Set session DATAURL dir */
				GMT->session.DATAURL = strdup (value);
			}
			break;

		case GMTCASE_GMT_DATA_URL_LIMIT:	/* The default is set by cmake, see ConfigDefault.cmake */
			if (!strcmp (lower_value, "0") || !strncmp (lower_value, "unlim", 5U))
				GMT->current.setting.url_size_limit = 0;
			else {
				size_t f, k = len - 1;
				if (k && (lower_value[k] == 'b' || lower_value[k] == 'B')) k--;
				switch (lower_value[k]) {
					case 'k':	case 'K':	f = 1024;		break;
					case 'm':	case 'M':	f = 1024*1024;		break;
					case 'g':	case 'G':	f = 1024*1024*1024;	break;
					default:	f = 1;	break;
				}
				GMT->current.setting.url_size_limit = atoi (lower_value) * f;
			}
			break;

		case GMTCASE_GMT_CUSTOM_LIBS:
			if (*value) {
				if (GMT->session.CUSTOM_LIBS) {
					if ((strcmp (GMT->session.CUSTOM_LIBS, value) == 0))
						break; /* stop here if string in place is equal */
					gmt_M_str_free (GMT->session.CUSTOM_LIBS);
				}
				/* Set Extension shared libraries */
				GMT->session.CUSTOM_LIBS = strdup (value);
			}
			break;

		case GMTCASE_GMT_EXPORT_TYPE:
			if (!strncmp (lower_value, "double", 6U))
				GMT->current.setting.export_type = GMT_DOUBLE;
			else if (!strncmp (lower_value, "single", 6U))
				GMT->current.setting.export_type = GMT_FLOAT;
			else if (!strncmp (lower_value, "float", 5U))
				GMT->current.setting.export_type = GMT_FLOAT;
			else if (!strncmp (lower_value, "long", 4U))
				GMT->current.setting.export_type = GMT_LONG;
			else if (!strncmp (lower_value, "ulong", 5U))
				GMT->current.setting.export_type = GMT_ULONG;
			else if (!strncmp (lower_value, "int", 3U))
				GMT->current.setting.export_type = GMT_INT;
			else if (!strncmp (lower_value, "uint", 4U))
				GMT->current.setting.export_type = GMT_UINT;
			else if (!strncmp (lower_value, "short", 5U))
				GMT->current.setting.export_type = GMT_SHORT;
			else if (!strncmp (lower_value, "ushort", 6U))
				GMT->current.setting.export_type = GMT_USHORT;
			else if (!strncmp (lower_value, "char", 4U))
				GMT->current.setting.export_type = GMT_CHAR;
			else if (!strncmp (lower_value, "byte", 4U))
				GMT->current.setting.export_type = GMT_UCHAR;
			else if (!strncmp (lower_value, "uchar", 5U))
				GMT->current.setting.export_type = GMT_UCHAR;
			else
				error = true;
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
			GMT_COMPAT_TRANSLATE ("GMT_HISTORY");
			break;
		case GMTCASE_GMT_HISTORY:
			if      (strspn (lower_value, "1t"))
				GMT->current.setting.history = (GMT_HISTORY_READ | GMT_HISTORY_WRITE);
			else if (strchr (lower_value, 'r'))
				GMT->current.setting.history = GMT_HISTORY_READ;
			else if (strspn (lower_value, "0f"))
				GMT->current.setting.history = GMT_HISTORY_OFF;
			else
				error = true;
			break;
		case GMTCASE_INTERPOLANT:
			GMT_COMPAT_TRANSLATE ("GMT_INTERPOLANT");
			break;
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
		case GMTCASE_GMT_LANGUAGE:
			strncpy (GMT->current.setting.language, lower_value, GMT_LEN64-1);
			gmtinit_get_language (GMT);	/* Load in names and abbreviations in chosen language */
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
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_CHANGE ("GMT_VERBOSE");
				ival = atoi (value) + 2;
				if (ival >= GMT_MSG_QUIET && ival <= GMT_MSG_DEBUG) {
					GMT->current.setting.verbose = ival;
					GMT_KEYWORD_UPDATE (GMTCASE_GMT_VERBOSE);
				}
				else
					error = true;
			}
			else
				error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_GMT_VERBOSE:
			error = gmtinit_parse_V_option (GMT, lower_value[0]);
			break;

		/* DIR GROUP */

		case GMTCASE_DIR_CACHE:
			if (*value) {
				if (GMT->session.CACHEDIR) {
					if ((strcmp (GMT->session.CACHEDIR, value) == 0))
						break; /* stop here if string in place is equal */
					gmt_M_str_free (GMT->session.CACHEDIR);
				}
				/* Set session CACHEDIR dir */
				GMT->session.CACHEDIR = strdup (value);
			}
			break;
		case GMTCASE_DIR_DATA:
			if (*value) {
				if (GMT->session.DATADIR) {
					if ((strcmp (GMT->session.DATADIR, value) == 0))
						break; /* stop here if string in place is equal */
					gmt_M_str_free (GMT->session.DATADIR);
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
					gmt_M_str_free (GMT->session.DCWDIR);
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
					gmt_M_str_free (GMT->session.GSHHGDIR);
				}
				/* Set session GSHHG dir */
				GMT->session.GSHHGDIR = strdup (value);
			}
			break;

		/* TIME GROUP */

		case GMTCASE_TIME_EPOCH:
			strncpy (GMT->current.setting.time_system.epoch, value, GMT_LEN64-1);
			(void) gmt_init_time_system_structure (GMT, &GMT->current.setting.time_system);
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
		case GMTCASE_TIME_LANGUAGE:
			GMT_COMPAT_TRANSLATE ("GMT_LANGUAGE");
			break;
		case GMTCASE_WANT_LEAP_SECONDS:
			GMT_COMPAT_TRANSLATE ("TIME_LEAP_SECONDS");
			break;
		case GMTCASE_TIME_LEAP_SECONDS:
			error = gmtinit_true_false_or_error (lower_value, &GMT->current.setting.time_leap_seconds);
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
			(void) gmt_init_time_system_structure (GMT, &GMT->current.setting.time_system);
			break;
		case GMTCASE_TIME_SYSTEM:
			error = gmt_get_time_system (GMT, lower_value, &GMT->current.setting.time_system);
			(void) gmt_init_time_system_structure (GMT, &GMT->current.setting.time_system);
			GMT_KEYWORD_UPDATE (GMTCASE_TIME_UNIT);
			GMT_KEYWORD_UPDATE (GMTCASE_TIME_EPOCH);
			break;
		case GMTCASE_TIME_WEEK_START:
			ival = gmtinit_key_lookup (value, GMT_weekdays, 7);
			if (ival < 0 || ival >= 7) {
				error = true;
				GMT->current.setting.time_week_start = 0;
			}
			else
				GMT->current.setting.time_week_start = ival;
			break;
		case GMTCASE_Y2K_OFFSET_YEAR:
			GMT_COMPAT_TRANSLATE ("TIME_Y2K_OFFSET_YEAR");
			break;
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else	/* Not recognized so give error message */
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_DIR_TMP:
		case GMTCASE_DIR_USER:
			/* Setting ignored, were active previously in GMT5 but no longer */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Parameter %s (previously introduced in GMT5) is deprecated.\n" GMT_COMPAT_INFO, GMT_keywords[case_val]);
			break;

		default:
			error = true;
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized keyword %s.\n", keyword);
			break;
	}

	/* Store possible unit.  For most cases these are irrelevant as no unit is expected */
	if (case_val >= 0) {
		if (len && strchr (GMT_DIM_UNITS, value[len-1])) GMT->current.setting.given_unit[case_val] = value[len-1];

		if (error)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: %s given illegal value (%s)!\n", keyword, value);
		else
			GMT_KEYWORD_UPDATE (case_val);
	}
	return ((error) ? 1 : 0);
}

/*! . */
char *gmtlib_putparameter (struct GMT_CTRL *GMT, const char *keyword) {
	/* value must hold at least GMT_BUFSIZ chars */
	static char value[GMT_BUFSIZ] = {""}, txt[GMT_LEN8];
	int case_val;
	bool error = false;
	char pm[2] = {'+', '-'}, *ft[2] = {"false", "true"};

	gmt_M_memset (value, GMT_BUFSIZ, char);
	if (!keyword) return (value);		/* keyword argument missing */

	case_val = gmt_hash_lookup (GMT, keyword, keys_hashnode, GMT_N_KEYS, GMT_N_KEYS);

	switch (case_val) {
		/* FORMAT GROUP */
		case GMTCASE_INPUT_CLOCK_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_CLOCK_IN:
			strncpy (value, GMT->current.setting.format_clock_in,  GMT_BUFSIZ-1);
			break;
		case GMTCASE_INPUT_DATE_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_DATE_IN:
			strncpy (value, GMT->current.setting.format_date_in, GMT_BUFSIZ-1);
			break;
		case GMTCASE_OUTPUT_CLOCK_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_CLOCK_OUT:
			strncpy (value, GMT->current.setting.format_clock_out, GMT_BUFSIZ-1);
			break;
		case GMTCASE_OUTPUT_DATE_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_DATE_OUT:
			strncpy (value, GMT->current.setting.format_date_out, GMT_BUFSIZ-1);
			break;
		case GMTCASE_OUTPUT_DEGREE_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_GEO_OUT:
			strncpy (value, GMT->current.setting.format_geo_out, GMT_BUFSIZ-1);
			break;
		case GMTCASE_PLOT_CLOCK_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_CLOCK_MAP:
			strncpy (value, GMT->current.setting.format_clock_map, GMT_BUFSIZ-1);
			break;
		case GMTCASE_PLOT_DATE_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_DATE_MAP:
			strncpy (value, GMT->current.setting.format_date_map, GMT_BUFSIZ-1);
			break;
		case GMTCASE_PLOT_DEGREE_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_GEO_MAP:
			strncpy (value, GMT->current.setting.format_geo_map, GMT_BUFSIZ-1);
			break;
		case GMTCASE_TIME_FORMAT_PRIMARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_TIME_PRIMARY_MAP:
			strncpy (value, GMT->current.setting.format_time[GMT_PRIMARY], GMT_BUFSIZ-1);
			break;
		case GMTCASE_TIME_FORMAT_SECONDARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_TIME_SECONDARY_MAP:
			strncpy (value, GMT->current.setting.format_time[GMT_SECONDARY], GMT_BUFSIZ-1);
			break;
		case GMTCASE_D_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_FLOAT_OUT:
			strncpy (value, GMT->current.setting.format_float_out_orig, GMT_BUFSIZ-1);
			break;
		case GMTCASE_FORMAT_FLOAT_MAP:
			strncpy (value, GMT->current.setting.format_float_map, GMT_BUFSIZ-1);
			break;
		case GMTCASE_UNIX_TIME_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FORMAT_TIME_STAMP:
			strncpy (value, GMT->current.setting.format_time_stamp, GMT_BUFSIZ-1);
			break;

		/* FONT GROUP */

		case GMTCASE_ANNOT_FONT_PRIMARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_ANNOT_PRIMARY:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]), GMT_BUFSIZ-1);
			break;
		case GMTCASE_ANNOT_FONT_SECONDARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_ANNOT_SECONDARY:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_annot[GMT_SECONDARY]), GMT_BUFSIZ-1);
			break;
		case GMTCASE_FONT_HEADING:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_heading), GMT_BUFSIZ-1);
			break;
		case GMTCASE_HEADER_FONT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_TITLE:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_title), GMT_BUFSIZ-1);
			break;
		case GMTCASE_FONT_TAG:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_tag), GMT_BUFSIZ-1);
			break;
		case GMTCASE_LABEL_FONT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_FONT_LABEL:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_label), GMT_BUFSIZ-1);
			break;

		case GMTCASE_FONT_LOGO:
			strncpy (value, gmt_putfont (GMT, &GMT->current.setting.font_logo), GMT_BUFSIZ-1);
			break;

		/* FONT GROUP ... obsolete options */

		case GMTCASE_ANNOT_FONT_SIZE_PRIMARY:
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				snprintf (value, GMT_LEN256, "%g", GMT->current.setting.font_annot[GMT_PRIMARY].size);
			}
			else
				error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_ANNOT_FONT_SIZE_SECONDARY:
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				snprintf (value, GMT_LEN256, "%g", GMT->current.setting.font_annot[GMT_SECONDARY].size);
			}
			else
				error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_HEADER_FONT_SIZE:
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				snprintf (value, GMT_LEN256, "%g", GMT->current.setting.font_title.size);
			}
			else
				error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_LABEL_FONT_SIZE:
			if (gmt_M_compat_check (GMT, 4)) {	/* GMT4: */
				GMT_COMPAT_WARN;
				snprintf (value, GMT_LEN256, "%g", GMT->current.setting.font_label.size);
			}
			else
				error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;

		/* MAP GROUP */

		case GMTCASE_ANNOT_OFFSET_PRIMARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_OFFSET_PRIMARY:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_annot_offset[GMT_PRIMARY] GMT_def(GMTCASE_MAP_ANNOT_OFFSET_PRIMARY));
			break;
		case GMTCASE_ANNOT_OFFSET_SECONDARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_OFFSET_SECONDARY:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_annot_offset[GMT_SECONDARY] GMT_def(GMTCASE_MAP_ANNOT_OFFSET_SECONDARY));
			break;
		case GMTCASE_OBLIQUE_ANNOTATION:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_OBLIQUE:
			snprintf (value, GMT_LEN256, "%d", GMT->current.setting.map_annot_oblique);
			break;
		case GMTCASE_ANNOT_MIN_ANGLE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_MIN_ANGLE:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.map_annot_min_angle);
			break;
		case GMTCASE_ANNOT_MIN_SPACING:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_MIN_SPACING:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_annot_min_spacing GMT_def(GMTCASE_MAP_ANNOT_MIN_SPACING));
			break;
		case GMTCASE_Y_AXIS_TYPE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ANNOT_ORTHO:
			strncpy (value, GMT->current.setting.map_annot_ortho, GMT_BUFSIZ-1);
			break;
		case GMTCASE_DEGREE_SYMBOL:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_AXES:
			strncpy (value, GMT->current.setting.map_frame_axes, GMT_BUFSIZ-1);
			break;
		case GMTCASE_BASEMAP_FRAME_RGB:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_DEFAULT_PEN:
			snprintf (value, GMT_LEN256, "%s", gmt_putpen (GMT, &GMT->current.setting.map_default_pen));
			break;
		case GMTCASE_FRAME_PEN:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_PEN:
			snprintf (value, GMT_LEN256, "%s", gmt_putpen (GMT, &GMT->current.setting.map_frame_pen));
			break;
		case GMTCASE_BASEMAP_TYPE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_TYPE:
			if (GMT->current.setting.map_frame_type == GMT_IS_PLAIN)
				strcpy (value, "plain");
			else if (GMT->current.setting.map_frame_type == GMT_IS_GRAPH) {
				strcpy (value, "graph");
				if (GMT->current.setting.map_graph_extension_unit != GMT_GRAPH_EXTENSION_UNIT || !doubleAlmostEqual (GMT->current.setting.map_graph_extension, GMT_GRAPH_EXTENSION)) {
					char tmp[GMT_LEN32] = {""};
					/* Not the default, specify what we are using */
					if (GMT->current.setting.map_graph_extension_unit == GMT_GRAPH_EXTENSION_UNIT)	/* Extension in percent */
						snprintf (tmp, GMT_LEN32, ",%g%%", GMT->current.setting.map_graph_extension);
					else {
						double s = GMT->session.u2u[GMT_INCH][GMT->current.setting.map_graph_extension_unit];
						snprintf (tmp, GMT_LEN32, ",%g%c", s*GMT->current.setting.map_graph_extension, GMT->session.unit_name[GMT->current.setting.map_graph_extension_unit][0]);
					}
					strcat (value, tmp);
				}
			}
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_FRAME_WIDTH:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_frame_width GMT_def(GMTCASE_MAP_FRAME_WIDTH));
			break;
		case GMTCASE_GRID_CROSS_SIZE_PRIMARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_grid_cross_size[GMT_PRIMARY] GMT_def(GMTCASE_MAP_GRID_CROSS_SIZE_PRIMARY));
			break;
		case GMTCASE_GRID_CROSS_SIZE_SECONDARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_grid_cross_size[GMT_SECONDARY] GMT_def(GMTCASE_MAP_GRID_CROSS_SIZE_SECONDARY));
			break;
		case GMTCASE_GRID_PEN_PRIMARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_PEN_PRIMARY:
			snprintf (value, GMT_LEN256, "%s", gmt_putpen (GMT, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]));
			break;
		case GMTCASE_GRID_PEN_SECONDARY:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_GRID_PEN_SECONDARY:
			snprintf (value, GMT_LEN256, "%s", gmt_putpen (GMT, &GMT->current.setting.map_grid_pen[GMT_SECONDARY]));
			break;
		case GMTCASE_MAP_HEADING_OFFSET:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_heading_offset GMT_def(GMTCASE_MAP_HEADING_OFFSET));
			break;
		case GMTCASE_LABEL_OFFSET:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LABEL_OFFSET:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_label_offset GMT_def(GMTCASE_MAP_LABEL_OFFSET));
			break;
		case GMTCASE_LINE_STEP:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LINE_STEP:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_line_step GMT_def(GMTCASE_MAP_LINE_STEP));
			break;
		case GMTCASE_UNIX_TIME:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LOGO:
			snprintf (value, GMT_LEN256, "%s", ft[GMT->current.setting.map_logo]);
			break;
		case GMTCASE_UNIX_TIME_POS:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_LOGO_POS:
			snprintf (value, GMT_LEN256, "%s/%g%c/%g%c", GMT_just_string[GMT->current.setting.map_logo_justify],
			GMT->current.setting.map_logo_pos[GMT_X] GMT_def(GMTCASE_MAP_LOGO_POS),
			GMT->current.setting.map_logo_pos[GMT_Y] GMT_def(GMTCASE_MAP_LOGO_POS));
			break;
		case GMTCASE_X_ORIGIN:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ORIGIN_X:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_origin[GMT_X] GMT_def(GMTCASE_MAP_ORIGIN_X));
			break;
		case GMTCASE_Y_ORIGIN:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_ORIGIN_Y:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_origin[GMT_Y] GMT_def(GMTCASE_MAP_ORIGIN_Y));
			break;
		case GMTCASE_POLAR_CAP:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_POLAR_CAP:
			if (doubleAlmostEqual (GMT->current.setting.map_polar_cap[0], 90.0))
				snprintf (value, GMT_LEN256, "none");
			else
				snprintf (value, GMT_LEN256, "%g/%g", GMT->current.setting.map_polar_cap[0], GMT->current.setting.map_polar_cap[1]);
			break;
		case GMTCASE_MAP_SCALE_HEIGHT:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_scale_height GMT_def(GMTCASE_MAP_SCALE_HEIGHT));
			break;
		case GMTCASE_MAP_TICK_LENGTH:
		case GMTCASE_TICK_LENGTH:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_TICK_LENGTH_PRIMARY:
			snprintf (value, GMT_LEN256, "%g%c/%g%c",
			GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] GMT_def(GMTCASE_MAP_TICK_LENGTH_PRIMARY),
			GMT->current.setting.map_tick_length[GMT_TICK_UPPER] GMT_def(GMTCASE_MAP_TICK_LENGTH_PRIMARY));
			break;
		case GMTCASE_MAP_TICK_LENGTH_SECONDARY:
			snprintf (value, GMT_LEN256, "%g%c/%g%c",
			GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] GMT_def(GMTCASE_MAP_TICK_LENGTH_SECONDARY),
			GMT->current.setting.map_tick_length[GMT_TICK_LOWER] GMT_def(GMTCASE_MAP_TICK_LENGTH_SECONDARY));
			break;
		case GMTCASE_MAP_TICK_PEN:
		case GMTCASE_TICK_PEN:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_TICK_PEN_PRIMARY:
			snprintf (value, GMT_LEN256, "%s", gmt_putpen (GMT, &GMT->current.setting.map_tick_pen[GMT_PRIMARY]));
			break;
		case GMTCASE_MAP_TICK_PEN_SECONDARY:
			snprintf (value, GMT_LEN256, "%s", gmt_putpen (GMT, &GMT->current.setting.map_tick_pen[GMT_SECONDARY]));
			break;
		case GMTCASE_HEADER_OFFSET:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_TITLE_OFFSET:
			snprintf (value, GMT_LEN256, "%g%c", GMT->current.setting.map_title_offset GMT_def(GMTCASE_MAP_TITLE_OFFSET));
			break;
		case GMTCASE_VECTOR_SHAPE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_MAP_VECTOR_SHAPE:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.map_vector_shape);
			break;

		/* COLOR GROUP */

		case GMTCASE_COLOR_BACKGROUND:
			snprintf (value, GMT_LEN256, "%s", gmt_putcolor (GMT, GMT->current.setting.color_patch[GMT_BGD]));
			break;
		case GMTCASE_COLOR_FOREGROUND:
			snprintf (value, GMT_LEN256, "%s", gmt_putcolor (GMT, GMT->current.setting.color_patch[GMT_FGD]));
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
			snprintf (value, GMT_LEN256, "%s", gmt_putcolor (GMT, GMT->current.setting.color_patch[GMT_NAN]));
			break;
		case GMTCASE_HSV_MIN_SATURATION:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MIN_S:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.color_hsv_min_s);
			break;
		case GMTCASE_HSV_MAX_SATURATION:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MAX_S:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.color_hsv_max_s);
			break;
		case GMTCASE_HSV_MIN_VALUE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MIN_V:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.color_hsv_min_v);
			break;
		case GMTCASE_HSV_MAX_VALUE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_COLOR_HSV_MAX_V:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.color_hsv_max_v);
			break;

		/* PS GROUP */

		case GMTCASE_CHAR_ENCODING:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_CHAR_ENCODING:
			strncpy (value, GMT->current.setting.ps_encoding.name, GMT_BUFSIZ-1);
			break;
		case GMTCASE_PS_COLOR:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
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
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_COMPAT_WARN;
				snprintf (value, GMT_LEN256, "%d", GMT->current.setting.ps_copies);
			}
			else
				error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_DOTS_PR_INCH:
		case GMTCASE_PS_DPI: GMT_COMPAT_WARN;
			if (!gmt_M_compat_check (GMT, 4)) error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
			break;
		case GMTCASE_PS_EPS: GMT_COMPAT_WARN;
			if (!gmt_M_compat_check (GMT, 4)) error = gmtinit_badvalreport (GMT, keyword);	/* Not recognized so give error message */
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
					snprintf (value, GMT_LEN256, "deflate,%u", GMT->PSL->internal.deflate_level);
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
			snprintf (value, GMT_LEN256, "%d", GMT->PSL->internal.miter_limit);
			break;
		case GMTCASE_PAGE_COLOR:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_PAGE_COLOR:
			snprintf (value, GMT_LEN256, "%s", gmt_putcolor (GMT, GMT->current.setting.ps_page_rgb));
			break;
		case GMTCASE_PAGE_ORIENTATION:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_PAGE_ORIENTATION:
			if (GMT->current.setting.ps_orientation == PSL_LANDSCAPE)
				strcpy (value, "landscape");
			else if (GMT->current.setting.ps_orientation == PSL_PORTRAIT)
				strcpy (value, "portrait");
			else
				strcpy (value, "undefined");
			break;
		case GMTCASE_PAPER_MEDIA:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_MEDIA:
			if (GMT->current.setting.ps_media == -USER_MEDIA_OFFSET)
				snprintf (value, GMT_LEN256, "%gx%g", fabs (GMT->current.setting.ps_page_size[0]), fabs (GMT->current.setting.ps_page_size[1]));
			else if (GMT->current.setting.ps_media >= USER_MEDIA_OFFSET)
				snprintf (value, GMT_LEN256, "%s", GMT->session.user_media_name[GMT->current.setting.ps_media-USER_MEDIA_OFFSET]);
			else if (GMT->current.setting.ps_media < GMT_N_MEDIA)
				snprintf (value, GMT_LEN256, "%s", GMT_media_name[GMT->current.setting.ps_media]);
			if (GMT->current.setting.ps_page_size[0] < 0.0)
				strcat (value, "-");
			else if (GMT->current.setting.ps_page_size[1] < 0.0)
				strcat (value, "+");
			break;
		case GMTCASE_GLOBAL_X_SCALE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_SCALE_X:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.ps_magnify[GMT_X]);
			break;
		case GMTCASE_GLOBAL_Y_SCALE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_SCALE_Y:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.ps_magnify[GMT_Y]);
			break;
		case GMTCASE_TRANSPARENCY:
			if (gmt_M_compat_check (GMT, 4))
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Transparency is now part of pen and fill specifications.  TRANSPARENCY is ignored\n");
			else
				error = gmtinit_badvalreport (GMT, keyword);
			break;
		case GMTCASE_PS_TRANSPARENCY:
			strncpy (value, GMT->current.setting.ps_transpmode, GMT_LEN16-1);
			break;
		case GMTCASE_PS_CONVERT:
			strncpy (value, GMT->current.setting.ps_convert, GMT_BUFSIZ-1);
			break;
		case GMTCASE_PS_VERBOSE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PS_COMMENTS:
			if (!GMT->PSL) return (NULL);	/* Not using PSL in this session */
			snprintf (value, GMT_LEN256, "%s", ft[GMT->PSL->internal.comments]);
			break;

		/* IO GROUP */

		case GMTCASE_FIELD_DELIMITER:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
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
				strncpy (value, GMT->current.setting.io_col_separator, GMT_BUFSIZ-1);
			break;
		case GMTCASE_IO_FIRST_HEADER:
			if (GMT->current.setting.io_first_header == GMT_FIRST_SEGHEADER_MAYBE)
				strcpy (value, "maybe");
			else if (GMT->current.setting.io_first_header == GMT_FIRST_SEGHEADER_ALWAYS)
				strcpy (value, "always");
			else
				strcpy (value, "never");
			break;
		case GMTCASE_GRIDFILE_FORMAT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_GRIDFILE_FORMAT:
			strncpy (value, GMT->current.setting.io_gridfile_format, GMT_BUFSIZ-1);
			break;
		case GMTCASE_GRIDFILE_SHORTHAND:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_GRIDFILE_SHORTHAND:
			snprintf (value, GMT_LEN256, "%s", ft[GMT->current.setting.io_gridfile_shorthand]);
			break;
		case GMTCASE_IO_HEADER:
			snprintf (value, GMT_LEN256, "%s", ft[GMT->current.setting.io_header[GMT_IN]]);
			break;
		case GMTCASE_IO_HEADER_MARKER:
			value[0] = '\0';
			if (GMT->current.setting.io_head_marker[GMT_OUT] != GMT->current.setting.io_head_marker[GMT_IN]) {
				snprintf (txt, 8U, "%c,", GMT->current.setting.io_head_marker[GMT_IN]);	strcat (value, txt);
				snprintf (txt, 8U, "%c", GMT->current.setting.io_head_marker[GMT_OUT]);	strcat (value, txt);
			}
			else {
				snprintf (txt, 8U, "%c", GMT->current.setting.io_head_marker[GMT_IN]);	strcat (value, txt);
			}
			break;
		case GMTCASE_N_HEADER_RECS:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_IO_N_HEADER_RECS:
			snprintf (value, GMT_LEN256, "%d", GMT->current.setting.io_n_header_items);
			break;
		case GMTCASE_NAN_RECORDS:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
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
				snprintf (value, GMT_LEN256, "%" PRIuS ",%" PRIuS, /* chunk size: lat,lon */
						 GMT->current.setting.io_nc4_chunksize[0],
						 GMT->current.setting.io_nc4_chunksize[1]);
			break;
		case GMTCASE_IO_NC4_DEFLATION_LEVEL:
			snprintf (value, GMT_LEN256, "%u", GMT->current.setting.io_nc4_deflation_level);
			break;
		case GMTCASE_XY_TOGGLE:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
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
				snprintf (value, GMT_LEN256, "%" PRIu64, GMT->current.setting.n_bin_header_cols);
			break;

		case GMTCASE_IO_SEGMENT_MARKER:
			value[0] = '\0';
			if (GMT->current.setting.io_seg_marker[GMT_OUT] != GMT->current.setting.io_seg_marker[GMT_IN]) {
				if ((GMT->current.setting.io_seg_marker[GMT_IN] == 'N' && !GMT->current.setting.io_nanline[GMT_IN]) || (GMT->current.setting.io_seg_marker[GMT_IN] == 'B' && !GMT->current.setting.io_blankline[GMT_IN])) value[0] = '\\';
				snprintf (txt, 8U, "%c,", GMT->current.setting.io_seg_marker[GMT_IN]);	strcat (value, txt);
				if ((GMT->current.setting.io_seg_marker[GMT_IN] == 'N' && !GMT->current.setting.io_nanline[GMT_IN]) || (GMT->current.setting.io_seg_marker[GMT_IN] == 'B' && !GMT->current.setting.io_blankline[GMT_IN])) strcat (value, "\\");
				snprintf (txt, 8U, "%c", GMT->current.setting.io_seg_marker[GMT_OUT]);	strcat (value, txt);
			}
			else {
				if ((GMT->current.setting.io_seg_marker[GMT_IN] == 'N' && !GMT->current.setting.io_nanline[GMT_IN]) || (GMT->current.setting.io_seg_marker[GMT_IN] == 'B' && !GMT->current.setting.io_blankline[GMT_IN])) value[0] = '\\';
				snprintf (txt, 8U, "%c", GMT->current.setting.io_seg_marker[GMT_IN]);	strcat (value, txt);
			}
			break;

		/* PROJ GROUP */

		case GMTCASE_PROJ_AUX_LATITUDE:
			switch (GMT->current.setting.proj_aux_latitude) {
				case GMT_LATSWAP_NONE:
					strcpy (value, "geodetic");
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_ELLIPSOID:
			if (GMT->current.setting.proj_ellipsoid < GMT_N_ELLIPSOIDS - 1)	/* Custom ellipse */
				snprintf (value, GMT_LEN256, "%s", GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].name);
			else if (gmt_M_is_spherical (GMT))
				snprintf (value, GMT_LEN256, "%f", GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius);
			else
				snprintf (value, GMT_LEN256, "%f,%f", GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius,
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_LENGTH_UNIT:
			snprintf (value, GMT_LEN256, "%s", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_PROJ_SCALE_FACTOR:
			if (doubleAlmostEqual (GMT->current.setting.proj_scale_factor, -1.0)) /* Default scale for chosen projection */
				strcpy (value, "default"); /* Default scale for chosen projection */
			else
				snprintf (value, GMT_LEN256, "%g", GMT->current.setting.proj_scale_factor);
			break;

		/* GMT GROUP */

		case GMTCASE_GMT_COMPATIBILITY:
			snprintf (value, GMT_LEN256, "%u", GMT->current.setting.compatibility);
			break;

		case GMTCASE_GMT_AUTO_DOWNLOAD:
			strncpy (value, (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) ? "off" : "on", GMT_BUFSIZ-1);
			break;

		case GMTCASE_GMT_DATA_URL:	/* The default is set by cmake, see ConfigDefault.cmake */
			strncpy (value, (GMT->session.DATAURL) ? GMT->session.DATAURL : "", GMT_BUFSIZ-1);
			break;

		case GMTCASE_GMT_DATA_URL_LIMIT:
			if (GMT->current.setting.url_size_limit == 0)
				strcpy (value, "unlimited");
			else if (GMT->current.setting.url_size_limit < 1024)
				sprintf (value, "%" PRIu64, (uint64_t)GMT->current.setting.url_size_limit);
			else if (GMT->current.setting.url_size_limit < 1024*1024)
				sprintf (value, "%" PRIu64 "Kb", (uint64_t)GMT->current.setting.url_size_limit/1024);
			else if (GMT->current.setting.url_size_limit < 1024*1024*1024)
				sprintf (value, "%" PRIu64 "Mb", (uint64_t)GMT->current.setting.url_size_limit/(1024*1024));
			else
				sprintf (value, "%" PRIu64 "Gb", (uint64_t)GMT->current.setting.url_size_limit/(1024*1024*1024));
			break;

		case GMTCASE_GMT_CUSTOM_LIBS:
			strncpy (value, (GMT->session.CUSTOM_LIBS) ? GMT->session.CUSTOM_LIBS : "", GMT_BUFSIZ-1);
			break;

		case GMTCASE_GMT_EXPORT_TYPE:
			if (GMT->current.setting.export_type == GMT_DOUBLE)
				strcpy (value, "double");
			else if (GMT->current.setting.export_type == GMT_FLOAT)
				strcpy (value, "single");
			else if (GMT->current.setting.export_type == GMT_LONG)
				strcpy (value, "long");
			else if (GMT->current.setting.export_type == GMT_ULONG)
				strcpy (value, "ulong");
			else if (GMT->current.setting.export_type == GMT_INT)
				strcpy (value, "int");
			else if (GMT->current.setting.export_type == GMT_UINT)
				strcpy (value, "uint");
			else if (GMT->current.setting.export_type == GMT_SHORT)
				strcpy (value, "short");
			else if (GMT->current.setting.export_type == GMT_USHORT)
				strcpy (value, "ushort");
			else if (GMT->current.setting.export_type == GMT_CHAR)
				strcpy (value, "char");
			else if (GMT->current.setting.export_type == GMT_UCHAR)
				strcpy (value, "byte");
			break;

		case GMTCASE_GMT_EXTRAPOLATE_VAL:
			if (GMT->current.setting.extrapolate_val[0] == GMT_EXTRAPOLATE_NONE)
				strcpy (value, "NaN");
			else if (GMT->current.setting.extrapolate_val[0] == GMT_EXTRAPOLATE_SPLINE)
				strcpy (value, "extrap");
			else if (GMT->current.setting.extrapolate_val[0] == GMT_EXTRAPOLATE_CONSTANT)
				snprintf (value, GMT_LEN256, "extrapval,%g", GMT->current.setting.extrapolate_val[1]);
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_HISTORY:
			if (GMT->current.setting.history & GMT_HISTORY_WRITE)
				snprintf (value, GMT_LEN256, "true");
			else if (GMT->current.setting.history & GMT_HISTORY_READ)
				snprintf (value, GMT_LEN256, "readonly");
			else
				snprintf (value, GMT_LEN256, "false");
			break;
		case GMTCASE_INTERPOLANT:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
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
		case GMTCASE_TIME_LANGUAGE:
		case GMTCASE_GMT_LANGUAGE:
			strncpy (value, GMT->current.setting.language, GMT_LEN64-1);
			gmtinit_get_language (GMT);	/* Load in names and abbreviations in chosen language */
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
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_GMT_VERBOSE:
			switch (GMT->current.setting.verbose) {
				case GMT_MSG_QUIET:         strcpy (value, "quiet");	break;
				case GMT_MSG_NORMAL:        strcpy (value, "normal");	break;
				case GMT_MSG_VERBOSE:       strcpy (value, "verbose");	break;
				case GMT_MSG_LONG_VERBOSE:  strcpy (value, "long_verbose");	break;
				case GMT_MSG_DEBUG:         strcpy (value, "debug");	break;
				default:                    strcpy (value, "compat");	break;
			}
			break;

		/* DIR GROUP */

		case GMTCASE_DIR_CACHE:
			/* Force update of session.CACHEDIR before copying the string */
			strncpy (value, (GMT->session.CACHEDIR) ? GMT->session.CACHEDIR : "", GMT_BUFSIZ-1);
			break;
		case GMTCASE_DIR_DATA:
			/* Force update of session.DATADIR before copying the string */
			strncpy (value, (GMT->session.DATADIR) ? GMT->session.DATADIR : "", GMT_BUFSIZ-1);
			break;
		case GMTCASE_DIR_DCW:
			/* Force update of session.DCWDIR before copying the string */
			strncpy (value, (GMT->session.DCWDIR) ? GMT->session.DCWDIR : "", GMT_BUFSIZ-1);
			break;
		case GMTCASE_DIR_GSHHG:
			/* Force update of session.GSHHGDIR before copying the string */
			gmt_shore_adjust_res (GMT, 'c');
			strncpy (value, (GMT->session.GSHHGDIR) ? GMT->session.GSHHGDIR : "", GMT_BUFSIZ-1);
			break;

		/* TIME GROUP */

		case GMTCASE_TIME_EPOCH:
			strncpy (value, GMT->current.setting.time_system.epoch, GMT_LEN64-1);
			break;
		case GMTCASE_TIME_IS_INTERVAL:
			if (GMT->current.setting.time_is_interval)
				snprintf (value, GMT_LEN256, "%c%d%c", pm[GMT->current.time.truncate.direction], GMT->current.time.truncate.T.step, GMT->current.time.truncate.T.unit);
			else
				snprintf (value, GMT_LEN256, "off");
			break;
		case GMTCASE_TIME_INTERVAL_FRACTION:
			snprintf (value, GMT_LEN256, "%g", GMT->current.setting.time_interval_fraction);
			break;
		case GMTCASE_WANT_LEAP_SECONDS:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_TIME_LEAP_SECONDS:
			snprintf (value, GMT_LEN256, "%s", ft[GMT->current.setting.time_leap_seconds]);
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
			snprintf (value, GMT_LEN256, "%s", GMT_weekdays[GMT->current.setting.time_week_start]);
			break;
		case GMTCASE_Y2K_OFFSET_YEAR:
			if (gmt_M_compat_check (GMT, 4))	/* GMT4: */
				GMT_COMPAT_WARN;
			else { error = gmtinit_badvalreport (GMT, keyword); break; }	/* Not recognized so give error message */
		case GMTCASE_TIME_Y2K_OFFSET_YEAR:
			snprintf (value, GMT_LEN256, "%d", GMT->current.setting.time_Y2K_offset_year);
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
int gmt_pickdefaults (struct GMT_CTRL *GMT, bool lines, struct GMT_OPTION *options) {
	int error = GMT_OK, n = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_RECORD Out;
	char *param, record[GMT_BUFSIZ] = {""};

	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		return (GMT->parent->error);
	}
	if (GMT_Begin_IO (GMT->parent, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {
		return (GMT->parent->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Set_Geometry (GMT->parent, GMT_OUT, GMT_IS_TEXT) != GMT_NOERROR) {	/* Sets output geometry */
		return (GMT->parent->error);
	}
	Out.data = NULL;
	for (opt = options; opt; opt = opt->next) {
		if (!(opt->option == '<' || opt->option == '#') || !opt->arg)
			continue;		/* Skip other and empty options */
		if (lines) record[0] = '\0';	/* Start over */
		if (!lines && n)
			strcat (record, " ");	/* Separate by spaces */
		param = gmtlib_putparameter (GMT, opt->arg);
		if (*param == '\0') {
			/* if keyword unknown */
			error = GMT_OPTION_NOT_FOUND;
			break;
		}
		if (lines) {	/* Separate lines */
			Out.text = param;
			GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, &Out);
		}
		else
			strncat (record, param, GMT_BUFSIZ-1);
		n++;
	}
	if (!lines && n) {
		Out.text = record;
		GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, &Out);		/* Separate lines */
	}
	if (GMT_End_IO (GMT->parent, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		return (GMT->parent->error);
	}
	return error;
}

/*! Dumps the GMT parameters to file or standard output */
void gmt_putdefaults (struct GMT_CTRL *GMT, char *this_file) {
	/* ONLY USED BY GMTSET AND GMTDEFAULTS */
	if (this_file)	/* File name is defined: use it as is */
		gmtinit_savedefaults (GMT, this_file);
	else {	/* Use local dir, tempdir, or workflow dir */
		char path[PATH_MAX] = {""};
		if (GMT->current.setting.run_mode == GMT_MODERN)	/* Modern mode: Use the workflow directory */
			sprintf (path, "%s/gmt.conf", GMT->parent->gwf_dir);
		else if (GMT->session.TMPDIR)	/* Write GMT->session.TMPDIR/gmt.conf */
			sprintf (path, "%s/gmt.conf", GMT->session.TMPDIR);
		else	/* Write gmt.conf in current directory */
			sprintf (path, "gmt.conf");
		gmtinit_savedefaults (GMT, path);
	}
}

/*! Read user's gmt.conf file and initialize parameters */
void gmt_getdefaults (struct GMT_CTRL *GMT, char *this_file) {
	char file[PATH_MAX];

	if (this_file)	/* Defaults file is specified */
		gmt_loaddefaults (GMT, this_file);
	else {	/* Use local dir, tempdir, or workflow dir (modern mode) */
		if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Modern mode: Use the workflow directory */
			char path[PATH_MAX] = {""};
			sprintf (path, "%s/gmt.conf", GMT->parent->gwf_dir);
			gmt_loaddefaults (GMT, path);
		}
		else if (gmtlib_getuserpath (GMT, "gmt.conf", file))
			gmt_loaddefaults (GMT, file);
	}
}

/*! Creates the name (if equivalent) or the string r[/g/b] corresponding to the RGB triplet or a pattern.
 * Example: gmtlib_putfill (GMT, fill) may produce "white" or "1/2/3" or "p300/7"
 */
char *gmtlib_putfill (struct GMT_CTRL *GMT, struct GMT_FILL *F) {

	static char text[PATH_MAX+GMT_LEN256] = {""};
	int i;

	if (F->use_pattern) {
		if (F->pattern_no)
			snprintf (text, PATH_MAX+GMT_LEN256, "p%d/%d", F->dpi, F->pattern_no);
		else
			snprintf (text, PATH_MAX+GMT_LEN256, "p%d/%s", F->dpi, F->pattern);
	}
	else if (F->rgb[0] < -0.5)
		sprintf (text, "-");
	else if ((i = gmtlib_getrgb_index (GMT, F->rgb)) >= 0)
		snprintf (text, PATH_MAX+GMT_LEN256, "%s", gmt_M_color_name[i]);
	else if (gmt_M_is_gray (F->rgb))
		snprintf (text, PATH_MAX+GMT_LEN256, "%.5g", gmt_M_q(gmt_M_s255(F->rgb[0])));
	else
		snprintf (text, PATH_MAX+GMT_LEN256, "%.5g/%.5g/%.5g", gmt_M_t255(F->rgb));
	gmtinit_append_trans (text, F->rgb[3]);
	return (text);
}

/*! Creates the name (if equivalent) or the string r[/g/b] corresponding to the RGB triplet.
 * Example: gmt_putcolor (GMT, rgb) may produce "white" or "1/2/3"
 */
char *gmt_putcolor (struct GMT_CTRL *GMT, double *rgb) {

	static char text[GMT_LEN256] = {""};
	int i;

	if (rgb[0] < -0.5)
		sprintf (text, "-");
	else if ((i = gmtlib_getrgb_index (GMT, rgb)) >= 0)
		snprintf (text, GMT_LEN256, "%s", gmt_M_color_name[i]);
	else if (gmt_M_is_gray(rgb))
		snprintf (text, GMT_LEN256, "%.5g", gmt_M_q(gmt_M_s255(rgb[0])));
	else
		snprintf (text, GMT_LEN256, "%.5g/%.5g/%.5g", gmt_M_t255(rgb));
	gmtinit_append_trans (text, rgb[3]);
	return (text);
}

/*! Creates t the string r/g/b corresponding to the RGB triplet */
char *gmt_putrgb (struct GMT_CTRL *GMT, double *rgb) {

	static char text[GMT_LEN256] = {""};
	gmt_M_unused(GMT);

	if (rgb[0] < -0.5)
		sprintf (text, "-");
	else
		snprintf (text, GMT_LEN256, "%.5g/%.5g/%.5g", gmt_M_t255(rgb));
	gmtinit_append_trans (text, rgb[3]);
	return (text);
}

/*! Creates the string c/m/y/k corresponding to the CMYK quadruplet */
char *gmtlib_putcmyk (struct GMT_CTRL *GMT, double *cmyk) {

	static char text[GMT_LEN256] = {""};
	gmt_M_unused(GMT);

	if (cmyk[0] < -0.5)
		sprintf (text, "-");
	else
		snprintf (text, GMT_LEN256, "%.5g/%.5g/%.5g/%.5g", gmt_M_q(cmyk[0]), gmt_M_q(cmyk[1]), gmt_M_q(cmyk[2]), gmt_M_q(cmyk[3]));
	gmtinit_append_trans (text, cmyk[4]);
	return (text);
}

/*! Creates the string h/s/v corresponding to the HSV triplet */
char *gmtlib_puthsv (struct GMT_CTRL *GMT, double *hsv) {

	static char text[GMT_LEN256] = {""};
	gmt_M_unused(GMT);

	if (hsv[0] < -0.5)
		sprintf (text, "-");
	else
		snprintf (text, GMT_LEN256, "%.5g-%.5g-%.5g", gmt_M_q(hsv[0]), gmt_M_q(hsv[1]), gmt_M_q(hsv[2]));
	gmtinit_append_trans (text, hsv[3]);
	return (text);
}

/*! . */
double gmt_convert_units (struct GMT_CTRL *GMT, char *string, unsigned int default_unit, unsigned int target_unit) {
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

	given_unit = gmtlib_unit_lookup (GMT, c, default_unit);	/* Will warn if c is not 0, 'c', 'i', 'p' */

	if (!gmtinit_is_valid_number (string))
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "%s not a valid number and may not be decoded properly.\n", string);

	value = atof (string) * GMT->session.u2u[given_unit][target_unit];
	if (have_unit) string[len-1] = (char)GMT->session.unit_name[given_unit][0];	/* Put back the (implied) given unit */

	return (value);
}

/*! . */
unsigned int gmtlib_unit_lookup (struct GMT_CTRL *GMT, int c, unsigned int unit) {
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
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Length unit %c not supported - revert to default unit [%s]\n", (int)c, GMT->session.unit_name[unit]);
			break;
	}

	return (unit);
}

/*! . */
int gmt_hash_lookup (struct GMT_CTRL *GMT, const char *key, struct GMT_HASH *hashnode, unsigned int n, unsigned int n_hash) {
	int i;
	unsigned int ui, k;

	i = gmtinit_hash (GMT, key, n_hash);			/* Get initial hash key */

	if (i < 0 || (ui = i) >= n) return (-1);	/* Bad key */
	if (hashnode[ui].n_id == 0) return (-1);	/* No entry for this hash value */
	/* Must search among the entries with identical hash value ui, starting at item k = 0 */
	k = 0;
	while (k < hashnode[ui].n_id && strcmp (hashnode[ui].key[k], key)) k++;
	if (k == hashnode[ui].n_id) return (-1);	/* Bad key; no match found */
	return (hashnode[ui].id[k]);			/* Return array index that goes with this key */
}

/*! Set up hash table */
int gmt_hash_init (struct GMT_CTRL *GMT, struct GMT_HASH *hashnode, char **keys, unsigned int n_hash, unsigned int n_keys) {
	unsigned int i, next;
	int entry;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Enter: gmt_hash_init\n");
	gmt_M_memset (hashnode, n_hash, struct GMT_HASH);	/* Start with NULL everywhere */
	for (i = 0; i < n_keys; i++) {
		entry = gmtinit_hash (GMT, keys[i], n_hash);
		next = hashnode[entry].n_id;
		if (next == GMT_HASH_MAXDEPTH) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s makes hash-depth exceed hard-wired limit of %d - increment GMT_HASH_MAXDEPTH in gmt_hash.h and recompile GMT\n", keys[i], GMT_HASH_MAXDEPTH);
			GMT_exit (GMT, GMT_DIM_TOO_SMALL); return GMT_DIM_TOO_SMALL;
		}
		hashnode[entry].key[next] = keys[i];
		hashnode[entry].id[next]  = i;
		hashnode[entry].n_id++;
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Exit:  gmt_hash_init\n");
	return GMT_OK;
}

/*! Return ID of requested ellipsoid, or -1 if not found */
int gmt_get_ellipsoid (struct GMT_CTRL *GMT, char *name) {
	int i, n;
	char line[GMT_LEN128], ename[GMT_LEN64];
	double pol_radius;

	/* Try to get ellipsoid from the default list; use case-insensitive checking */

	strncpy (ename, name, GMT_LEN64-1);		/* Make a copy of name */
	gmt_str_tolower (ename);	/* Convert to lower case */
	for (i = 0; i < GMT_N_ELLIPSOIDS; i++) {
		strcpy (line, GMT->current.setting.ref_ellipsoid[i].name);
		gmt_str_tolower (line);	/* Convert to lower case */
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
		if (!gmt_M_is_spherical (GMT)) GMT->current.setting.ref_ellipsoid[i].flattening = 1.0 / GMT->current.setting.ref_ellipsoid[i].flattening;
	}
	if (n == 1) return (i);

	if (gmt_M_compat_check (GMT, 4)) {
		FILE *fp = NULL;
		char path[PATH_MAX];
		double slop;
		/* Try to open as file first in (1) current dir, then in (2) $GMT->session.SHAREDIR */

		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Assigning PROJ_ELLIPSOID a file name is deprecated, use <a>,<inv_f> instead\n");
		gmt_getsharepath (GMT, NULL, name, "", path, R_OK);

		if ((fp = fopen (name, "r")) != NULL || (fp = fopen (path, "r")) != NULL) {
			/* Found file, now get parameters */
			while (fgets (line, GMT_LEN128, fp) && (line[0] == '#' || line[0] == '\n'));
			fclose (fp);
			n = sscanf (line, "%s %d %lf %lf %lf", GMT->current.setting.ref_ellipsoid[i].name,
				&GMT->current.setting.ref_ellipsoid[i].date, &GMT->current.setting.ref_ellipsoid[i].eq_radius,
				&pol_radius, &GMT->current.setting.ref_ellipsoid[i].flattening);
			if (n != 5) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error decoding user ellipsoid parameters (%s)\n", line);
				GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
			}

			if (pol_radius == 0.0) {} /* Ignore semi-minor axis */
			else if (gmt_M_is_spherical (GMT)) {
				/* zero flattening means we must compute flattening from the polar and equatorial radii: */

				GMT->current.setting.ref_ellipsoid[i].flattening = 1.0 - (pol_radius / GMT->current.setting.ref_ellipsoid[i].eq_radius);
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "user-supplied ellipsoid has implicit flattening of %.8f\n", GMT->current.setting.ref_ellipsoid[i].flattening);
			}
			/* else check consistency: */
			else if ((slop = fabs (GMT->current.setting.ref_ellipsoid[i].flattening - 1.0 +
			         (pol_radius/GMT->current.setting.ref_ellipsoid[i].eq_radius))) > 1.0e-8) {
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
				            "Possible inconsistency in user ellipsoid parameters (%s) [off by %g]\n", line, slop);
			}
			return (i);
		}
	}

	return (-1);
}

/*! . */
bool gmt_get_time_system (struct GMT_CTRL *GMT, char *name, struct GMT_TIME_SYSTEM *time_system) {
	/* Convert TIME_SYSTEM into TIME_EPOCH and TIME_UNIT.
	   TIME_SYSTEM can be one of the following: j2000, jd, mjd, s1985, unix, dr0001, rata
	   or any string in the form "TIME_UNIT since TIME_EPOCH", like "seconds since 1985-01-01".
	   This function only splits the strings, no validation or analysis is done.
	   See gmt_init_time_system_structure for that.
	   TIME_SYSTEM = other is completely ignored.
	*/
	char *epoch = NULL;
	gmt_M_unused(GMT);

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
		strncpy (time_system->epoch, epoch, GMT_LEN64-1);
		time_system->unit = name[0];
		if (!strncmp (name, "mon", 3U)) time_system->unit = 'o';
	}
	else
		return (true);
	return (false);
}

/*! . */
void gmt_end (struct GMT_CTRL *GMT) {
	/* gmt_end will clean up after us. */

	unsigned int i;

	gmtinit_put_history (GMT);

	/* Remove font structures */
	gmt_M_free (GMT, GMT->session.font);
#ifdef __FreeBSD__
#ifdef _i386_
	fpresetsticky (FP_X_DZ | FP_X_INV);
	fpsetmask (FP_X_DZ | FP_X_INV);
#endif
#endif

	gmt_M_str_free (GMT->init.runtime_bindir);
	gmt_M_str_free (GMT->init.runtime_libdir);
	gmt_M_str_free (GMT->init.runtime_library);
	gmt_M_str_free (GMT->init.runtime_plugindir);
	gmtinit_free_dirnames (GMT);
	for (i = 0; i < GMT_N_PROJ4; i++)
		gmt_M_str_free (GMT->current.proj.proj4[i].name);
	gmt_M_free (GMT, GMT->current.proj.proj4);
	for (i = 0; i < GMT_N_UNIQUE; i++)
		gmt_M_str_free (GMT->init.history[i]);
	gmtinit_reset_colformats (GMT);	/* Wipe settings */
	for (i = 0; i < GMT->common.a.n_aspatial; i++)
		gmt_M_str_free (GMT->common.a.name[i]);
	gmt_M_str_free (GMT->common.h.title);
	gmt_M_str_free (GMT->common.h.remark);
	gmt_M_str_free (GMT->common.h.colnames);

	if (GMT->current.setting.io_gridfile_shorthand) gmtinit_freeshorthand (GMT);

	fflush (GMT->session.std[GMT_OUT]);	/* Make sure output buffer is flushed */

	gmtlib_free_ogr (GMT, &(GMT->current.io.OGR), 1);	/* Free up the GMT/OGR structure, if used */
	gmtlib_free_tmp_arrays (GMT);			/* Free emp memory for vector io or processing */
	gmtinit_free_user_media (GMT);
	/* Terminate PSL machinery (if used) */
	PSL_endsession (GMT->PSL);
#ifdef MEMDEBUG
	gmt_memtrack_report (GMT);
	gmt_M_str_free (GMT->hidden.mem_keeper);
#endif

	gmtinit_free_GMT_ctrl (GMT);	/* Deallocate control structure */
}

/*! . */
GMT_LOCAL struct GMT_CTRL *gmt_begin_module_sub (struct GMTAPI_CTRL *API, const char *lib_name, const char *mod_name, struct GMT_CTRL **Ccopy) {
	/* All GMT modules (i.e. GMT_psxy, GMT_blockmean, ...) must call gmt_init_module
	 * as their first call and call gmt_end_module as their last call.  This
	 * allows us to capture the GMT control structure so we can reset all
	 * parameters to what they were before exiting the module. Note:
	 * 1. Session items that remain unchanged are not replicated if allocated separately.
	 * 2. Items that may grow through session are not replicated if allocated separately.
	 */

	unsigned int i;
	struct GMT_CTRL *GMT = API->GMT, *Csave = NULL;

	Csave = calloc (1U, sizeof (struct GMT_CTRL));

	gmtlib_free_tmp_arrays (GMT);			/* Free temp memory for vector io or processing */

	/* First memcpy over everything; this will include pointer addresses we will have to fix below */

	gmt_M_memcpy (Csave, GMT, 1, struct GMT_CTRL);

	/* Increment level uint64_t */
	GMT->hidden.func_level++;		/* This lets us know how deeply we are nested when a GMT module is called */

	/* Now fix things that were allocated separately from the main GMT structure.  These are usually text strings
	 * that were allocated via strdup since the structure only have a pointer allocated. */

	/* GMT_INIT */
	if (GMT->session.n_user_media) {
		Csave->session.n_user_media = GMT->session.n_user_media;
		Csave->session.user_media = gmt_M_memory (GMT, NULL, GMT->session.n_user_media, struct GMT_MEDIA);
		Csave->session.user_media_name = gmt_M_memory (GMT, NULL, GMT->session.n_user_media, char *);
		for (i = 0; i < GMT->session.n_user_media; i++) Csave->session.user_media_name[i] = strdup (GMT->session.user_media_name[i]);
	}

	/* GMT_PLOT */
	if (GMT->current.plot.n_alloc) {
		Csave->current.plot.n_alloc = GMT->current.plot.n_alloc;
		Csave->current.plot.x = gmt_M_memory (GMT, NULL, GMT->current.plot.n_alloc, double);
		Csave->current.plot.y = gmt_M_memory (GMT, NULL, GMT->current.plot.n_alloc, double);
		Csave->current.plot.pen = gmt_M_memory (GMT, NULL, GMT->current.plot.n_alloc, unsigned int);
		gmt_M_memcpy (Csave->current.plot.x, GMT->current.plot.x, GMT->current.plot.n_alloc, double);
		gmt_M_memcpy (Csave->current.plot.y, GMT->current.plot.y, GMT->current.plot.n_alloc, double);
		gmt_M_memcpy (Csave->current.plot.pen, GMT->current.plot.pen, GMT->current.plot.n_alloc, unsigned int);
	}

	/* GMT_IO */
	Csave->current.io.OGR = gmtlib_duplicate_ogr (GMT, GMT->current.io.OGR);	/* Duplicate OGR struct, if set */
	gmtlib_free_ogr (GMT, &(GMT->current.io.OGR), 1);		/* Free up the GMT/OGR structure, if used */

	gmt_M_memset (Csave->current.io.o_format, GMT_MAX_COLUMNS, char *);
	for (i = 0; i < GMT_MAX_COLUMNS; i++)
		if (GMT->current.io.o_format[i]) Csave->current.io.o_format[i] = strdup (GMT->current.io.o_format[i]);

	/* GMT_COMMON */
	if (GMT->common.U.label) Csave->common.U.label = strdup (GMT->common.U.label);
	for (i = 0; i < GMT->common.a.n_aspatial; i++)
		if (GMT->common.a.name[i]) Csave->common.a.name[i] = strdup (GMT->common.a.name[i]);
	if (GMT->common.h.title) Csave->common.h.title = strdup (GMT->common.h.title);
	if (GMT->common.h.remark) Csave->common.h.remark = strdup (GMT->common.h.remark);
	if (GMT->common.h.colnames) Csave->common.h.colnames = strdup (GMT->common.h.colnames);

	/* DIR NAMES */

	Csave->session.GSHHGDIR = (GMT->session.GSHHGDIR) ? strdup (GMT->session.GSHHGDIR) : NULL;
	Csave->session.DCWDIR = (GMT->session.DCWDIR) ? strdup (GMT->session.DCWDIR) : NULL;
	Csave->session.SHAREDIR = (GMT->session.SHAREDIR) ? strdup (GMT->session.SHAREDIR) : NULL;
	Csave->session.HOMEDIR = (GMT->session.HOMEDIR) ? strdup (GMT->session.HOMEDIR) : NULL;
	Csave->session.USERDIR = (GMT->session.USERDIR) ? strdup (GMT->session.USERDIR) : NULL;
	Csave->session.CACHEDIR = (GMT->session.CACHEDIR) ? strdup (GMT->session.CACHEDIR) : NULL;
	Csave->session.DATADIR = (GMT->session.DATADIR) ? strdup (GMT->session.DATADIR) : NULL;
	Csave->session.TMPDIR = (GMT->session.TMPDIR) ? strdup (GMT->session.TMPDIR) : NULL;
	Csave->session.CUSTOM_LIBS = (GMT->session.CUSTOM_LIBS) ? strdup (GMT->session.CUSTOM_LIBS) : NULL;
	Csave->session.DATAURL = (GMT->session.DATAURL) ? strdup (GMT->session.DATAURL) : NULL;

	/* Reset all the common.?.active settings to false */

	GMT->common.B.active[GMT_PRIMARY] = GMT->common.B.active[GMT_SECONDARY] = GMT->common.K.active = GMT->common.O.active = false;
	GMT->common.P.active = GMT->common.U.active = GMT->common.V.active = false;
	GMT->common.X.active = GMT->common.Y.active = false;
	GMT->common.R.active[RSET] = GMT->common.R.active[ISET] = GMT->common.R.active[GSET] = GMT->common.R.active[FSET] = GMT->common.J.active = false;
	GMT->common.a.active = GMT->common.b.active[GMT_IN] = GMT->common.b.active[GMT_OUT] = false;
	GMT->common.f.active[GMT_IN] = GMT->common.f.active[GMT_OUT] = GMT->common.g.active = GMT->common.h.active = false;
	GMT->common.p.active = GMT->common.s.active = GMT->common.t.active = GMT->common.colon.active = false;
	gmt_M_memset (GMT->common.b.ncol, 2, int);

	*Ccopy = Csave; /* Pass back out for safe-keeping by the module until gmt_end_module is called */

	GMT->init.module_name = mod_name;
	GMT->init.module_lib  = lib_name;

	return (GMT);
}

/* Subplot functions */

GMT_LOCAL int get_current_panel (struct GMTAPI_CTRL *API, int fig, unsigned int *row, unsigned int *col, double gap[], char *tag, unsigned int *first) {
	/* Gets the current subplot panel, returns 1 if found and 0 otherwise.
	 * If gap == NULL that means that not finding a panel file is OK since it may be the first; we are just trying to get row,col. */
	char file[PATH_MAX] = {""};
	FILE *fp = NULL;
	int ios;
	sprintf (file, "%s/gmt.panel.%d", API->gwf_dir, fig);
	if (access (file, F_OK))	{	/* Panel selection file not available so we are not doing subplots */
		if (gap == NULL) {	/* By default we do the first panel */
			*row = *col = UINT_MAX;
			return GMT_NOERROR;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "get_current_panel: No current panel selected so not in subplot mode\n");
		API->error = GMT_NOERROR;
		return GMT_RUNTIME_ERROR;	/* It is an "error" in the sense we don't have a panel situation */
	}
	/* Here there is a current panel, get it */
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s!\n", file);
		API->error = GMT_RUNTIME_ERROR;
		return GMT_RUNTIME_ERROR;
	}
	if (gap == NULL) {
		if ((ios = fscanf (fp, "%d %d %*lg %*lg %*lg %*lg %*d", row, col)) != 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to decode record from %s!\n", file);
			API->error = GMT_RUNTIME_ERROR;
			fclose (fp);
			return GMT_RUNTIME_ERROR;
		}
	}
	else if ((ios = fscanf (fp, "%d %d %lg %lg %lg %lg %d %[^\n]", row, col, &gap[XLO], &gap[XHI], &gap[YLO], &gap[YHI], first, tag)) != 8) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to decode record from %s!\n", file);
		API->error = GMT_RUNTIME_ERROR;
		fclose (fp);
		return GMT_RUNTIME_ERROR;
	}
	fclose (fp);
	if (*row == 0 || *col == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Current panel has row or column outsiden range!\n");
		API->error = GMT_RUNTIME_ERROR;
		return GMT_RUNTIME_ERROR;
	}
	(*row)--;	(*col)--;	/* Since they were given on a 1-n,1-m basis */
	return GMT_NOERROR;
}

int gmt_set_current_panel (struct GMTAPI_CTRL *API, int fig, unsigned int row, unsigned int col, double gap[], char *label, unsigned first) {
	/* Update gmt.panel with current pane's (row,col) and write first as 0 or 1.
	 * first should be 1 the first time we visit this panel so that the automatic -B
	 * and panel tag stuff only happens once. */
	char file[PATH_MAX] = {""}, *L = NULL;
	static char *dummy = "@";	/* Signify "use the the auto label" */
	FILE *fp = NULL;
	L = (label && label[0]) ? label : dummy;
	sprintf (file, "%s/gmt.panel.%d", API->gwf_dir, fig);
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s!\n", file);
		API->error = GMT_RUNTIME_ERROR;
		return GMT_RUNTIME_ERROR;
	}
	if (gap == NULL)
		fprintf (fp, "%d %d 0 0 0 0 %d %s\n", row, col, first, L);
	else
		fprintf (fp, "%d %d %g %g %g %g %d %s\n", row, col, gap[XLO], gap[XHI], gap[YLO], gap[YHI], first, L);
	fclose (fp);
	API->error = GMT_NOERROR;
	return GMT_NOERROR;
}

int gmt_get_next_panel (struct GMTAPI_CTRL *API, int fig, unsigned int *row, unsigned int *col) {
	/* Auto-advance to next panel, with initialization at first panel.  The order of advancement
	 * was set by -A's +v modifier and is found by reading the subplotorder file */
	
	unsigned int n_rows, n_cols, order;
	char file[PATH_MAX] = {""};
	FILE *fp = NULL;
	
	sprintf (file, "%s/gmt.subplotorder.%d", API->gwf_dir, fig);
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s!\n", file);
		API->error = GMT_ERROR_ON_FOPEN;
		return GMT_ERROR_ON_FOPEN;
	}
	/* Read the matrix dimensions and the marching order */
	if (fscanf (fp, "%d %d %d", &n_rows, &n_cols, &order) != 3) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to read file %s!\n", file);
		API->error = GMT_DATA_READ_ERROR;
		fclose (fp);
		return GMT_DATA_READ_ERROR;
	}
	fclose (fp);
	
	/* If the panel file does not exist we initialize to row = col = 0 */
	if (get_current_panel (API, fig, row, col, NULL, NULL, NULL)) {	/* Not good */
		API->error = GMT_RUNTIME_ERROR;
		return GMT_RUNTIME_ERROR;
	}
	
	if (*row == UINT_MAX && *col == UINT_MAX)	/* First panel */
		*row = *col = 1;
	else {	/* Auto-advance to next panel */
		if (order == GMT_IS_COL_FORMAT) {	/* Going down columns */
			if (*row == (n_rows-1)) /* Top of next column */
				*row = 0, (*col)++;
			else	/* Down current column */
				(*row)++;
		}
		else {	/* Going across rows */
			if (*col == (n_cols-1)) /* Start of next row */
				*col = 0, (*row)++;
			else	/* Across current row */
				(*col)++;
		}
		(*row)++;	(*col)++;	/* Since they are needed on a 1-n,1-m basis */
	}
		
	API->error = GMT_NOERROR;
	return GMT_NOERROR;
}

/*! Return information about current panel */
struct GMT_SUBPLOT *gmt_subplot_info (struct GMTAPI_CTRL *API, int fig) {
	/* Only called under modern mode */
	char file[PATH_MAX] = {""}, line[PATH_MAX] = {""}, tmp[GMT_LEN16] = {""}, *c = NULL;
	bool found = false;
	unsigned int row, col, first, k;
	int n;
	double gap[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_SUBPLOT *P = NULL;
	FILE *fp = NULL;

	API->error = GMT_RUNTIME_ERROR;
	if (get_current_panel (API, fig, &row, &col, gap, tmp, &first))	/* No panel or there was an error */
		return NULL;

	/* Now read subplot information file */
	sprintf (file, "%s/gmt.subplot.%d", API->gwf_dir, fig);
	if (access (file, F_OK))	{	/* Subplot information file not available */
		GMT_Report (API, GMT_MSG_NORMAL, "No subplot information file found!\n");
		return NULL;
	}
	/* Here there is an information file, get it */
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s!\n", file);
		return NULL;
	}

	P = &(API->GMT->current.plot.panel);	/* Lazy shorthand only */
	P->dir[GMT_X] = P->dir[GMT_Y] = +1;	/* Default direction of Cartesian axis if -JX */

	/* Now read it */
	while (!found && fgets (line, PATH_MAX, fp)) {
		if (line[0] == '\n')	/* Blank line */
			continue;
		if (line[0] == '#') {	/* Comment line */
			if (!strncmp (line, "# ORIGIN:", 9U))
				sscanf (&line[10], "%lg %lg", &P->origin[GMT_X], &P->origin[GMT_Y]);
			else if (!strncmp (line, "# DIMENSION:", 12U))
				sscanf (&line[13], "%lg %lg", &P->dim[GMT_X], &P->dim[GMT_Y]);
			else if (!strncmp (line, "# PARALLEL:", 11U))
				P->parallel = atoi (&line[12]);
			else if (!strncmp (line, "# DIRECTION:", 12U))
				sscanf (&line[13], "%d %d", &P->dir[GMT_X], &P->dir[GMT_Y]);
			continue;
		}
 		if ((n = sscanf (line, "%*d %d %d %d %d", &P->row, &P->col, &P->nrows, &P->ncolumns)) != 4) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error decoding row/col from subplot information file %s.  Bad format? [%s] (n=%d)\n", file, line, n);
			fclose (fp);
			return NULL;
		}
		if (row > P->nrows || col > P->ncolumns) {
			GMT_Report (API, GMT_MSG_NORMAL, "Selected current panel (%d,%d) exceeds dimension of current subplot (%dx%d)]\n", row, col, P->nrows, P->ncolumns);
			fclose (fp);
			return NULL;
		}
		if (P->row == row && P->col == col) {	/* Found it */
			if ((n = sscanf (line, "%*d %*d %*d %*d %*d %lg %lg %lg %lg %s %lg %lg %lg %lg %s %s %s %s",
				&P->x, &P->y, &P->w, &P->h, P->tag, &P->off[GMT_X], &P->off[GMT_Y], &P->clearance[GMT_X], &P->clearance[GMT_Y], P->refpoint, P->justify, P->fill, P->pen)) != 13) {
				GMT_Report (API, GMT_MSG_NORMAL, "Error decoding subplot information file %s.  Bad format? [%s] (n=%d)\n", file, line, n);
				fclose (fp);
				return NULL;
			}
			if (P->fill[0] == '-') P->fill[0] = '\0';	/* - means no fill */
			if (P->pen[0] == '-') P->pen[0] = '\0';		/* - means no pen */
			P->first = first;
			gmt_M_memcpy (P->gap, gap, 4, double);
			if (strcmp (tmp, "@")) strncpy (P->tag, tmp, GMT_LEN16-1);	/* Replace auto-tag with manually added tag */
			if ((c = strchr (line, GMT_ASCII_GS)) == NULL) {	/* Get the position before frame setting */
				GMT_Report (API, GMT_MSG_NORMAL, "Error decoding subplot information file %s.  Bad format? [%s] (n=%d)\n", file, line, n);
				fclose (fp);
				return NULL;
			}
			c++;	k = 0;	/* Now at start of axes */
			while (*c != GMT_ASCII_GS) P->Baxes[k++] = *(c++);	/* Copy it over until end */
			c++;	k = 0;	/* Now at start of xaxis */
			while (*c != GMT_ASCII_GS) P->Bxlabel[k++] = *(c++);	/* Copy it over until end */
			c++;	k = 0;	/* Now at start of yaxis */
			while (*c != GMT_ASCII_GS) P->Bylabel[k++] = *(c++);	/* Copy it over until end */
			c++;	k = 0;	/* Now at start of xannot */
			while (*c != GMT_ASCII_GS) P->Bxannot[k++] = *(c++);	/* Copy it over until end */
			c++;	k = 0;	/* Now at start of yannot */
			while (*c != GMT_ASCII_GS) P->Byannot[k++] = *(c++);	/* Copy it over until end */
			found = true;	/* We are done */
		}
	}
	fclose (fp);
	if (!found) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to match specified row,col with subplot information\n", file);
		return NULL;
	}
	API->error = GMT_NOERROR;
	P->active = 1;
	return (P);
}

/*! Determine if the current module is a PostScript-producing module that will be writing PostScript */
GMT_LOCAL bool is_PS_module (struct GMTAPI_CTRL *API, const char *name, const char *keys, struct GMT_OPTION **in_options) {
	struct GMT_OPTION *opt = NULL, *options = NULL;

	if (keys == NULL || in_options == NULL) return false;	/* Definitively classic code, possibly MB system */
	if (keys[0] == '\0' || (strstr (keys, ">X}") == NULL && strstr (keys, ">?}") == NULL)) return false;	/* Can never produce PostScript */

	options = *in_options;

	if (!strncmp (name, "gmtinfo", 7U)) return false;	/* Does not ever return PS */

	/* Must do more specific checking since some of the PS producers take options that turns them into other things... */
	if (!strncmp (name, "psbasemap", 9U)) {	/* Check for -A option */
		if ((opt = GMT_Find_Option (API, 'A', options))) return false;	/* -A writes dataset */
	}
	else if (!strncmp (name, "pscoast", 7U)) {	/* Check for -M -E options */
		if ((opt = GMT_Find_Option (API, 'M', options))) return false;	/* -M writes dataset */
		if ((opt = GMT_Find_Option (API, 'J', options))) return true;	/* -J writes PS regardless of -E */
		if ((opt = GMT_Find_Option (API, 'E', options)) == NULL) return true;	/* Without -E writes PS */
		if (strstr (opt->arg, "+g") || strstr (opt->arg, "+p")) return true;	/* -E...+g|p writes PS */
		if (strstr (opt->arg, "+r") || strstr (opt->arg, "+R")) return false;	/* -E...+r|R writes dataset */
	}
	else if (!strncmp (name, "grdimage", 8U)) {	/* Check for -A option */
		if ((opt = GMT_Find_Option (API, 'A', options))) return false;	/* -A writes image */
	}
	else if (!strncmp (name, "grdcontour", 10U)) {	/* Check for -D option */
		if ((opt = GMT_Find_Option (API, 'D', options))) return false;	/* -D writes dataset */
	}
	else if (!strncmp (name, "pscontour", 9U)) {	/* Check for -D option */
		if ((opt = GMT_Find_Option (API, 'D', options))) return false;	/* -D writes dataset */
	}
	else if (!strncmp (name, "pshistogram", 11U)) {	/* Check for -I option */
		if ((opt = GMT_Find_Option (API, 'I', options))) return false;	/* -I writes dataset */
	}
	else if (!strncmp (name, "pssolar", 7U)) {	/* Check for -M -I options */
		if ((opt = GMT_Find_Option (API, 'M', options))) return false;	/* -M writes dataset */
		if ((opt = GMT_Find_Option (API, 'I', options))) return false;	/* -I writes dataset */
	}
	return true;	/* Remaining PostScript producing modules always write PostScript */
}

GMT_LOCAL void gmtinit_round_wesn (double wesn[], bool geo) {	/* Use data range to round to nearest reasonable multiples */
	bool set[2] = {false, false};
	unsigned int side, item;
	double mag, inc, range[2] = {0.0, 0.0}, s;
	range[GMT_X] = wesn[XHI] - wesn[XLO];
	range[GMT_Y] = wesn[YHI] - wesn[YLO];
	if (geo) {	/* Special checks due to periodicity */
		if (range[GMT_X] > 306.0) {	/* If within 15% of a full 360 we promote to 360 */
			wesn[XLO] = 0.0;	wesn[XHI] = 360.0;
			set[GMT_X] = true;
		}
		if (range[GMT_Y] > 153.0) {	/* If within 15% of a full 180 we promote to 180 */
			wesn[YLO] = -90.0;	wesn[YHI] = 90.0;
			set[GMT_Y] = true;
		}
	}
	for (side = GMT_X, item = XLO; side <= GMT_Y; side++) {
		if (set[side]) continue;	/* Done above */
		mag = rint (log10 (range[side])) - 1.0;
		inc = pow (10.0, mag);
		if ((range[side] / inc) > 10.0) inc *= 2.0;	/* Factor of 2 in the rounding */
		if ((range[side] / inc) > 10.0) inc *= 2.5;	/* Factor of 5 in the rounding */
		s = 1.0;
		if (geo) {	/* Use arc integer minutes or seconds if possible */
			if (inc < 1.0 && inc > 0.05) {	/* Nearest arc minute */
				s = 60.0; inc = 1.0;
				if ((s * range[side] / inc) > 10.0) inc *= 2.0;	/* 2 arcmin */
				if ((s * range[side] / inc) > 10.0) inc *= 2.5;	/* 5 arcmin */
			}
			else if (inc < 0.1 && inc > 0.005) {	/* Nearest arc second */
				s = 3600.0; inc = 1.0;
				if ((s * range[side] / inc) > 10.0) inc *= 2.0;	/* 2 arcsec */
				if ((s * range[side] / inc) > 10.0) inc *= 2.5;	/* 5 arcsec */
			}
		}
		wesn[item] = (floor (s * wesn[item] / inc) * inc) / s;	item++;
		wesn[item] = (ceil  (s * wesn[item] / inc) * inc) / s;	item++;
	}
}

GMT_LOCAL int gmtlib_get_region_from_data (struct GMTAPI_CTRL *API, int family, bool exact, struct GMT_OPTION **options, double wesn[]) {
	/* Determines the data region by examining the input data.  This could be a grid or datasets.  The
	 * latter may be one or many files, or none, meaning we must capture stdin.  If we do, then we must
	 * add that temporary file to the module's options, otherwise we cannot read the data a 2nd time.
	 * We return the wesn array with the exact or rounded region, depending on the setting of exact.
	 */
	unsigned int item;
	bool geo;
	struct GMT_GRID *G = NULL;
	struct GMT_OPTION *opt = NULL, *head = NULL, *tmp = NULL;
	struct GMT_DATASET *Out = NULL;
	char virt_file[GMT_STR16] = {""}, tmpfile[PATH_MAX] = {""}, *list = "bfi:";
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;

	switch (family) {
		case GMT_IS_GRID:
			if ((opt = GMT_Find_Option (API, GMT_OPT_INFILE, *options)) == NULL) return GMT_NO_INPUT;	/* Got no input argument*/
			if (gmt_access (API->GMT, opt->arg, R_OK)) return GMT_FILE_NOT_FOUND;	/* No such file found */
			if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL)
				return API->error;	/* Failure to read grid header */
			gmt_M_memcpy (wesn, G->header->wesn, 4, double);	/* Copy over the grid region */
			HH = gmt_get_H_hidden (G->header);
			if (!exact) gmtinit_round_wesn (wesn, HH->grdtype > 0);	/* Use grid w/e/s/n to round to nearest reasonable multiples */
			if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) return API->error;	/* Failure to destroy the temporary grid structure */
			break;

		case GMT_IS_DATASET:
			for (opt = *options; opt; opt = opt->next) {	/* Loop over all options */
				if (opt->option != GMT_OPT_INFILE) continue;	/* Look for input files we can append to new list */
				if ((tmp = GMT_Make_Option (API, GMT_OPT_INFILE, opt->arg)) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
					return API->error;	/* Failure to make new option and append to list */
			}
			if (head == NULL) {	/* User gave no input so we must process stdin */
				/* Make name for a temporary file */
				FILE *fp = NULL;
				char *file = NULL;
				void *content = NULL;
				size_t n_read = 0;
#ifndef _WIN32
				int fd = 0;
#endif

				GMT_Report (API, GMT_MSG_DEBUG, "gmtlib_get_region_from_data: Must save stdin to a temporary file.\n");
				if (API->tmp_dir)			/* Have a recognized temp directory */
					sprintf (tmpfile, "%s/", API->tmp_dir);
				strcat (tmpfile, "gmt_saved_stdin.XXXXXX");	/* The XXXXXX will be replaced by mktemp */
#ifdef _WIN32
				if ((file = mktemp (tmpfile)) == NULL) {
					GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_region_from_data: Could not create temporary file name.\n");
					return GMT_RUNTIME_ERROR;
				}
				if ((fp = fopen (file, API->GMT->current.io.w_mode)) == NULL) {
					GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_region_from_data: Could not open temporary file %s.\n", file);
					return GMT_RUNTIME_ERROR;
				}
#else
				if ((fd = mkstemp (tmpfile)) == -1) {
					GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_region_from_data: Could not create and open temporary file %s.\n", tmpfile);
					return GMT_RUNTIME_ERROR;
				}
				file = tmpfile;
				if ((fp = fdopen (fd, API->GMT->current.io.w_mode)) == NULL) {
					GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_region_from_data: Could not fdopen temporary file %s.\n", file);
					return GMT_RUNTIME_ERROR;
				}
#endif
				/* Dump stdin to that temp file */
				GMT_Report (API, GMT_MSG_DEBUG, "gmtlib_get_region_from_data: Send stdin to %s.\n", file);
				if ((content = malloc (GMT_BUFSIZ)) == NULL) {
					GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_region_from_data: Unable to allocate %d bytes for buffer.\n", GMT_BUFSIZ);
				    fclose (fp);
					return GMT_RUNTIME_ERROR;
				}
				while ((n_read = fread (content, 1, GMT_BUFSIZ, API->GMT->session.std[GMT_IN]))) {
			        if (fwrite (content, 1, n_read, fp) != n_read) {
						GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_region_from_data: fwrite failure.\n");
						free (content);
				    	fclose (fp);
						return GMT_RUNTIME_ERROR;
					}
			    }
			    fclose (fp);
				free (content);
				if ((tmp = GMT_Make_Option (API, GMT_OPT_INFILE, file)) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
					return API->error;	/* Failure to make new option or append to list */
				if ((tmp = GMT_Make_Option (API, GMT_OPT_INFILE, file)) == NULL || (*options = GMT_Append_Option (API, tmp, *options)) == NULL)
					return API->error;	/* Failure to append option to calling module option list */
				GMT_Report (API, GMT_MSG_DEBUG, "gmtlib_get_region_from_data: Replace stdin input with -<%s.\n", file);
			}

			/* Here we have all the input options OR a single tempfile input option.  Now look for special modifiers and add those too */
			for (item = 0; item < strlen (list); item++) {
				if ((opt = GMT_Find_Option (API, list[item], *options)) == NULL) continue;
				if ((tmp = GMT_Make_Option (API, list[item], opt->arg)) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
					return API->error;	/* Failure to make new option or append to list */
			}
			opt = GMT_Find_Option (API, 'f', head);	/* See if we have -f */
			geo = (opt && (opt->arg[0] == 'g' || strchr (opt->arg, 'x')));	/* Geographic data (could still fail with some odd -f I guess) */
			if (!geo && (opt = GMT_Find_Option (API, 'J', *options))) {	/* Passed -J but no geo via -fg */
				if (strchr ("xXpP", opt->arg[0]) == NULL || (toupper (opt->arg[0]) == 'X' && opt->arg[strlen(opt->arg)-1] == 'd')) {	/* Geographic projection of some sort */
					if ((tmp = GMT_Make_Option (API, 'f', "g")) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
						return API->error;	/* Failure to make new option or append to list */
				}
			}
			if ((tmp = GMT_Make_Option (API, 'C', NULL)) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
				return API->error;	/* Failure to make new option -C or append to list */
			if ((tmp = GMT_Make_Option (API, '-', "GMT_HISTORY=false")) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
				return API->error;	/* Failure to make new option -- or append to list */

			/* Set up virtual file to hold the result of gmt info */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, NULL, virt_file) == GMT_NOTSET)
				return (API->error);
			if ((tmp = GMT_Make_Option (API, GMT_OPT_OUTFILE, virt_file)) == NULL || (head = GMT_Append_Option (API, tmp, head)) == NULL)
		        return API->error;	/* Failure to make new output option or append to the list */

			if (GMT_Call_Module (API, "gmtinfo", GMT_MODULE_OPT, head) != GMT_OK)	/* Get the data domain via gmtinfo */
				return (API->error);

			if (GMT_Destroy_Options (API, &head))	/* Free the temporary option list */
				return (API->error);

			if ((Out = GMT_Read_VirtualFile (API, virt_file)) == NULL)
				return (API->error);
			/* Close the virtual files */
			if (GMT_Close_VirtualFile (API, virt_file) != GMT_NOERROR)
				return (API->error);
			/* Get the four values from the first and only output record */
			wesn[XLO] = Out->table[0]->segment[0]->data[0][0];
			wesn[XHI] = Out->table[0]->segment[0]->data[1][0];
			wesn[YLO] = Out->table[0]->segment[0]->data[2][0];
			wesn[YHI] = Out->table[0]->segment[0]->data[3][0];
			if (GMT_Destroy_Data (API, &Out) != GMT_OK)
				return (API->error);
			geo = gmt_M_is_geographic (API->GMT, GMT_IN);
			if (!exact) gmtinit_round_wesn (wesn, geo);	/* Use data range to round to nearest reasonable multiples */
			break;
		default:
			GMT_Report (API, GMT_MSG_DEBUG, "gmtlib_get_region_from_data: Family %d not supported", family);
			return GMT_NOT_A_VALID_FAMILY;
			break;
	}
	return GMT_NOERROR;
}

/*! Add -R<grid> for those modules that may implicitly obtain the region via a grid */
GMT_LOCAL int gmtinit_set_missing_R_from_grid (struct GMTAPI_CTRL *API, const char *args, bool exact, struct GMT_OPTION **options) {
	/* When a module uses -R indirectly via a grid then we need to set that explicitly in the options.
	 * Modules with this issue have "g" in their THIS_MODULE_NEEDS string.
	 */
	struct GMT_OPTION *opt = NULL;
	double wesn[4] = {0.0, 0.0, 0.0, 0.0};
	char region[GMT_LEN256] = {""};
	int err = GMT_NOERROR;
	gmt_M_unused(args);

	/* Here we know the module is using a grid to get -R implicitly */
	if ((err = gmtlib_get_region_from_data (API, GMT_IS_GRID, exact, options, wesn)))
		return err;

	sprintf (region, "%.16g/%.16g/%.16g/%.16g", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
	if ((opt = GMT_Make_Option (API, 'R', region)) == NULL)
		return API->error;	/* Failure to make option */
	if ((*options = GMT_Append_Option (API, opt, *options)) == NULL)
		return API->error;	/* Failure to append option */

	return GMT_NOERROR;
}

/*! Add -Rw/e/s/n for those modules that may implicitly obtain the region via input dataset(s) */
GMT_LOCAL int gmtinit_set_missing_R_from_datasets (struct GMTAPI_CTRL *API, const char *args, bool exact, struct GMT_OPTION **options) {
	/* When a module uses -R indirectly via input data then we need to set that explicitly in the options.
	 * Modules with this issue have "d" in their THIS_MODULE_NEEDS string.
	 * If exact is true then we return exact -R; otherwise we round outwards.
	 */
	char region[GMT_LEN256] = {""};
	int err = GMT_NOERROR;
	double wesn[4] = {0.0, 0.0, 0.0, 0.0};
	struct GMT_OPTION *opt = NULL;
	gmt_M_unused(args);

	/* Here we know the module is using a grid to get -R implicitly */
	if ((err = gmtlib_get_region_from_data (API, GMT_IS_DATASET, exact, options, wesn)))
		return err;
	sprintf (region, "%.16g/%.16g/%.16g/%.16g", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
	if ((opt = GMT_Make_Option (API, 'R', region)) == NULL)
		return API->error;	/* Failure to make option */
	if ((*options = GMT_Append_Option (API, opt, *options)) == NULL)
		return API->error;	/* Failure to append option */
	GMT_Report (API, GMT_MSG_DEBUG, "Modern: Adding -R%s to options.\n", opt->arg);

	return GMT_NOERROR;
}

/*! Add -R<grid> for those modules that may implicitly obtain the region via a grid */
GMT_LOCAL int gmtinit_determine_R_option_from_data (struct GMTAPI_CTRL *API, const char *args, bool exact, struct GMT_OPTION **options) {
	/* When a module uses -R indirectly via a grid then we need to set that explicitly in the options.
	 * Modules with this issue have "g" in their THIS_MODULE_NEEDS string.
	 * Modules that read datasets may have "d" in their THIS_MODULE_NEEDS which forces us to determine the region.
	 */

	if (GMT_Find_Option (API, 'R', *options)) return GMT_NOERROR;	/* Set explicitly, so do nothing */
	if (strchr (args, 'g'))	/* Use the input grid to add a valid -R into the options */
		return (gmtinit_set_missing_R_from_grid (API, args, exact, options));
	else if (strchr (args, 'd'))	/* Use input dataset(s) to find and add -R in this module */
		return (gmtinit_set_missing_R_from_datasets (API, args, exact, options));
	/* Nothing could be done */
	return GMT_NOERROR;
}

/*! Search the list for the -J? option (? != 'z|Z) and return the pointer to the item. */
GMT_LOCAL struct GMT_OPTION * gmt_find_J_option (void *V_API, struct GMT_OPTION *head) {
	struct GMT_OPTION *current = NULL, *ptr = NULL;
	gmt_M_unused(V_API);

	if (head == NULL) return (NULL);	/* Hard to find something in a non-existent list */

	for (current = head; ptr == NULL && current; current = current->next) {	/* Linearly search for the specified option */
		if (current->option == 'J' && !(current->arg[0] == 'z' || current->arg[0] == 'Z'))
			ptr = current;
	}
	return (ptr);	/* NULL if not found */
}

GMT_LOCAL unsigned int strip_R_from_E_in_pscoast (struct GMT_CTRL *GMT, struct GMT_OPTION *options, char r_code[]) {
	/* Separate out any region-specific parts from one or more -E arguments and
	 * pass those separately to a new -R instead (if -R not given).
	 * Return code is bitflags:
	 *	0 : No _r|R or +l|L given, most likely just setting countries and implicitly -R
	 * 	1 : Found a region-modifying modifier +r or +R
	 * 	2 : Found a list-request +l or +L.  Not plotting or region desired.
	 */
	char p[GMT_LEN256] = {""}, *c = NULL;
	char e_code[GMT_LEN256] = {""}, r_opt[GMT_LEN128] = {""};
	unsigned int pos, n_errors = 0, answer = 0;
	struct GMT_OPTION *E = options;

	while ((E = GMT_Find_Option (GMT->parent, 'E', E))) {	/* For all -E options */
		c = NULL;
		if ((c = strchr (E->arg, '+')))
			c[0] = '\0';	/* Temporarily hide the modifiers */
		if (r_code[0]) strcat (r_code, ",");	/* Accumulate all codes across multiple -E options */
		strcat (r_code, E->arg);	/* Append country codes only */
		strncpy (e_code, E->arg, GMT_LEN256-1);	/* Duplicate country codes only */
		if (c) {	/* Now process the modifiers */
			c[0] = '+';	/* Unhide the modifiers */
			pos = 0;	/* Initialize position counter for this string */
			while (gmt_getmodopt (GMT, 'E', c, "lLgprRw", &pos, p, &n_errors) && n_errors == 0) {
				switch (p[0]) {
					case 'r': case 'R':
						if (r_opt[0] == 0) {	/* Only set this once */
							r_opt[0] = '+';	strncat (r_opt, p, GMT_LEN128-2);
						}
						break;
					case 'w': break;	/* Do nothing with defunct +w that was never documented anyway */
					case 'l': case 'L':
						answer |= 2;	/* Set this flag then fall through on purpose to default: */
					default: strcat (e_code, "+"); strcat (e_code, p); break;	/* Append as is */
				}
			}
		}
		gmt_M_str_free (E->arg);	E->arg = strdup (e_code);	/* Update -E argument */
		E = E->next;	/* Go to next option so that GMT_Find_Option will get the next -E or NULL */
	}
	if (r_opt[0]) strcat (r_code, r_opt);	/* This string is returned back for possible use by -R */
	if (r_opt[0]) answer |= 1;	/* answer & 1 set if +r or +R was used */
	return (answer);
}

GMT_LOCAL bool is_region_geographic (struct GMT_CTRL *GMT, struct GMT_OPTION *options, const char *module) {
	/* Determine of -R<args> imply geographic or Cartesian domain */
	struct GMT_OPTION *opt = NULL;
	unsigned int n_slashes;
	size_t len;
	/* First deal with all the modules that only involve geographic data */
	if (!strncmp (module, "grdlandmask", 11U)) return true;
	if (!strncmp (module, "pscoast", 7U)) return true;
	if (!strncmp (module, "pssolar", 7U)) return true;
	if (!strncmp (module, "sph2grd", 7U)) return true;
	if (!strncmp (module, "sphdistance", 11U)) return true;
	if (!strncmp (module, "sphinterpolate", 14U)) return true;
	if (!strncmp (module, "img2grd", 7U)) return true;
	if (!strncmp (module, "pscoupe", 7U)) return true;
	if (!strncmp (module, "psmeca", 6U)) return true;
	if (!strncmp (module, "pspolar", 7U)) return true;
	if (!strncmp (module, "pssac", 5U)) return true;
	if (!strncmp (module, "psvelo", 6U)) return true;
	if (!strncmp (module, "mgd77track", 10U)) return true;
	if (!strncmp (module, "grdpmodeler", 11U)) return true;
	if (!strncmp (module, "grdrotater", 10U)) return true;
	if (!strncmp (module, "grdspotter", 10U)) return true;
	if (!strncmp (module, "polespotter", 11U)) return true;
	if ((opt = GMT_Find_Option (GMT->parent, 'R', options)) == NULL) return false;	/* Should not happen but lets just say Cartesian for now */
	n_slashes = gmtlib_count_char (GMT, opt->arg, '/');	/* Distinguies -Rw/e/s/n from other things */
	/* Check if -R[=]<code>[,<code>,...][+r|R] which means use country name etc to set region; clearly geographical */
	if (n_slashes == 0 && ((isupper ((int)opt->arg[0]) && isupper ((int)opt->arg[1])) || opt->arg[0] == '=' || strchr (opt->arg, ',') || strstr (opt->arg, "+r") || strstr (opt->arg, "+R"))) return true;
	if (!gmt_access (GMT, opt->arg, F_OK)) {	/* Gave a grid file */
		struct GMT_GRID *G = NULL;
		if ((G = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL)	/* Read header */
			return (false);
		if (GMT_Destroy_Data (GMT->parent, &G) != GMT_OK)
			return (false);
		return (gmt_M_is_geographic (GMT, GMT_IN));
	}
	len = strlen (opt->arg);
	if (n_slashes == 0) {	/* Giving continent or country code(s) or -Rg|d */
		if (len == 1 && strchr ("dg", opt->arg[0])) return true;	/* Gave -Rg or -Rd */
		else if (len == 2 || opt->arg[0] == '=' || strchr (opt->arg, ',')) return true;	/* Giving continent or country code(s) or a list of them */
		else if (len == 5 && opt->arg[2] == '.') return true;	/* Gave a single state, e.g., US.TX */
	}
	else if (n_slashes == 3) {
		if (strchr (opt->arg, 'T')) return false;	/* Giving dateTclock or similar */
		else if (strchr (opt->arg, ':')) return true;	/* Giving ddd:mm or similar */
		else if (strchr (opt->arg, 'W') || strchr (opt->arg, 'E') || strchr (opt->arg, 'S')|| strchr (opt->arg, 'N')) return true;	/* Giving ddd:mm or similar */
		else if (strstr (opt->arg, "+r")) return true;	/* Gave oblique region so geographic */
	}
	return false;	/* Default to Cartesian */
}

GMT_LOCAL int set_modern_mode_if_oneliner (struct GMTAPI_CTRL *API, struct GMT_OPTION **options) {
	/* Determine if user is attempting a modern mode one-liner plot */
	unsigned int pos;
	int error, k;
	char figure[GMT_LEN128] = {""}, session[GMT_LEN128] = {""}, p[GMT_LEN16] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	for (opt = *options; opt; opt = opt->next)	/* Loop over all options */
		if (opt->option == 'O' || opt->option == 'K') return GMT_NOERROR;	/* Cannot be a one-liner if -O or -K are involved */
	/* No -O -K present, so go ahead and check */
	for (opt = *options; opt; opt = opt->next) {	/* Loop over all options */
		if (strlen (opt->arg) < 1) continue;	/* ps is the shortest format extension */
		sprintf (figure, "%c%s", opt->option, opt->arg);	/* So -png,jpg, which would parse as -p with arg ng,jpg, are reunited to png,jpg */
		if ((c = strchr (figure, ','))) c[0] = 0;	/* Chop off other format for the initial id test */
		if ((k = gmt_get_graphics_id (API->GMT, figure)) == GMT_NOTSET) continue;	/* Not a quicky one-liner option */
		/* Make sure all formats are valid */
		if (c) c[0] = ',';	/* Restore any comma we found */
		pos = 0;
		while (gmt_strtok (figure, ",", &pos, p)) {	/* Check each format to make sure each is OK */
			if ((k = gmt_get_graphics_id (API->GMT, p)) == GMT_NOTSET) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unrecognized graphics format %s\n", p);
				return GMT_NOTSET;
			}
		}
		if (opt->next && opt->next->option == GMT_OPT_INFILE) {	/* Found a -ext[,ext,ext,...] <prefix> pair */
			sprintf (session, "%s %s", opt->next->arg, figure);
			/* Remove the one-liner options before the parser chokes on them */
			if (GMT_Delete_Option (API, opt->next, options) || GMT_Delete_Option (API, opt, options)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove -ext <prefix> options in set_modern_mode_if_oneliner.\n");
				return GMT_NOTSET;
			}
			API->GMT->hidden.func_level++;	/* Must do this here since it has not yet been increased by gmt_begin_module_sub ! */

			if ((error = GMT_Call_Module (API, "begin", GMT_MODULE_CMD, session))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to call module begin from set_modern_mode_if_oneliner.\n");
				return GMT_NOTSET;
			}
			API->GMT->current.setting.run_mode = GMT_MODERN;
			gmtinit_setautopagesize (API->GMT);
			API->GMT->current.ps.oneliner = true;	/* Special flag */
			API->GMT->hidden.func_level--;	/* Restore to what we had */
			return GMT_NOERROR;	/* All set */
		}
	}
	return GMT_NOERROR;
}

GMT_LOCAL int gmtinit_get_last_dimensions (struct GMTAPI_CTRL *API, int fig) {
	/* Get dimensions of previous plot, if any */
	FILE *fp = NULL;
	char file[PATH_MAX] = {""};
	if (API->gwf_dir == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtinit_get_last_dimensions: No workflow directory set\n");
		return GMT_NOT_A_VALID_DIRECTORY;
	}
	sprintf (file, "%s/gmt.canvas.%d", API->gwf_dir, fig);
	/* See if there is a gmt.canvas file to read for this figure */
	if (access (file, R_OK))	/* No gmt.canvas file available for current figure so return 0 */
		return GMT_NOERROR;
	/* Get previous dimensions */
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtinit_get_last_dimensions: Could not open file %s for figure %d\n", file, fig);
		return GMT_ERROR_ON_FOPEN;
	}
	if (fscanf (fp, "%lg %lg", &API->GMT->current.map.last_width, &API->GMT->current.map.last_height) != 2) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtinit_get_last_dimensions: Could not read dimensions from file %s for figure %d\n", file, fig);
		fclose (fp);
		return GMT_DATA_READ_ERROR;
	}
	fclose (fp);
	return (GMT_NOERROR);
}

GMT_LOCAL int gmtinit_set_last_dimensions (struct GMTAPI_CTRL *API) {
	/* Save dimensions of current plot */
	int fig;
	FILE *fp = NULL;
	char file[PATH_MAX] = {""};
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)  return GMT_NOERROR;	/* Not in modern mode */
	if (API->GMT->current.map.width == 0.0) return GMT_NOERROR;	/* No dimensions set yet */
	if (API->gwf_dir == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtinit_set_last_dimensions: No workflow directory set\n");
		return GMT_NOT_A_VALID_DIRECTORY;
	}
	fig = gmt_get_current_figure (API);
	sprintf (file, "%s/gmt.canvas.%d", API->gwf_dir, fig);
	/* Write current dimensions */
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtinit_set_last_dimensions: Could not create file %s for figure %d\n", file, fig);
		return GMT_ERROR_ON_FOPEN;
	}
	fprintf (fp, "%lg %lg\n", API->GMT->current.map.width, API->GMT->current.map.height);
	fclose (fp);
	return (GMT_NOERROR);
}

/*! Prepare options if missing and initialize module */
struct GMT_CTRL *gmt_init_module (struct GMTAPI_CTRL *API, const char *lib_name, const char *mod_name, const char *keys, const char *required, struct GMT_OPTION **options, struct GMT_CTRL **Ccopy) {
	/* For modern runmode only - otherwise we simply call gmt_begin_module_sub.
	 * We must consult the required string.  It may contain options that we need to set implicitly.
	 * Possible letters in the required string are:
	 *	R  The -R option is required for this program and if missing we will hunt for the last region in the history.
	 *	r  The -R option may be required depending on other settings.  If it becomes required and no -R<region> was
	 *     given then we hunt for one in the history.  This happens after GMT_Parse_Common has been called.
	 *  g  The region is required but if none is given the we simply use -R<grid>.  This applies to those modules that
	 *     have a required grid for input (e.g., modules like grdimage).
	 *  d  If no -R<region> is given AND there are no region in the history, then determine a suitable region
	 *     from the input dataset.  This enables automatic region determination via gmtinfo.
	 *  J  The -J option is required and if missing we will hunt for the last region in the history.
	 *  j  The -J option may be required depending on other settings.  If it becomes required and no -J<info> was
	 *     given then we hunt for one in the history. This happens after GMT_Parse_Common has been called.
	 *
	 * Note: 1. If no -J can be found in the history we provide either -JQ15c (geographic data) or -JX15c (Cartesian).
	 *
	 * Modules like pslegend has "rj" since -R -J are not required if -Dx is used but required for other settings.
	 * Modules like blockmean, surface has "R" since it is never cool to autodetermine grid domains as this also
	 *  depends on grid spacing, for instance.
	 * Modules like grdview has "g" since they always have a grid domain to fall back on in the absence of -R.
	 * Modules like psxy has "d" so we can make a quick map without specifying -R.
	 */

	bool is_PS;
	unsigned int srtm_res = 0;
	struct GMT_OPTION *E = NULL, *opt = NULL, *opt_R = NULL;
	struct GMT_CTRL *GMT = API->GMT;
	API->error = GMT_NOERROR;

#ifdef USE_GMT_KWD
	gmtinit_kw_replace (API, NULL, options);
#endif
	/* Making -R<country-codes> globally available means it must affect history, etc.  The simplest fix here is to
	 * make sure pscoast -E, if passing old +r|R area settings via -E, is split into -R before GMT_Parse_Common is called */

	if (options && gmt_M_compat_check (GMT, 5) && !strncmp (mod_name, "pscoast", 7U) && (E = GMT_Find_Option (API, 'E', *options)) && (opt = GMT_Find_Option (API, 'R', *options)) == NULL) {
		/* Running pscoast -E without -R: Must make sure any the region-information in -E is added as args to new -R.
		 * If there are no +r|R in the -E then we consult the history to see if there is an -R in effect. */
		char r_code[GMT_LEN256] = {""};
		bool add_R = true;
		unsigned int E_flags;
		E_flags = strip_R_from_E_in_pscoast (GMT, *options, r_code);
		if (GMT->current.setting.run_mode == GMT_MODERN && !(E_flags & 1)) {	/* Just country codes and plot settings, no region specs */
			int id = gmtlib_get_option_id (0, "R");		/* The -RP history item */
			if (!GMT->init.history[id]) id++;		/* No history for -RP, increment to -RG as fallback */
			if (GMT->init.history[id]) add_R = false;	/* There is history for -R so -R will be added below */
		}
		if (E_flags & 2) add_R = false;	/* No -R should be set since we want a country code listing only */
		if (add_R) {	/* Need to add a specific -R option that carries the information set via -E */
			if ((opt = GMT_Make_Option (API, 'R', r_code)) == NULL) return NULL;	/* Failure to make -R option */
			if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append -R option */
		}
	}
	else if (options && gmt_M_compat_check (GMT, 6) && !strncmp (mod_name, "psrose", 6U) && (opt = GMT_Find_Option (API, 'J', *options)) == NULL) {
		/* Running psrose with old -S[n]<radius syntax.  Need to replace with new -J [-S] syntax */
		struct GMT_OPTION *S = GMT_Find_Option (API, 'S', *options);
		if (S) {	/* Gave -S option */
			char j_code[GMT_LEN256] = {""};
			unsigned int k, norm = (S->arg[0] == 'n') ? 1 : 0;
			double radius;
			k = norm;
			if (norm == 0 && S->arg[strlen(S->arg)-1] == 'n') {	/* Old-style -S<radius>[unit]n syntax */
				norm = 2;
				S->arg[strlen(S->arg)-1] = '\0';
			}
			radius = gmt_M_to_inch (GMT, &S->arg[k]);	/* Get the radius, now in inches */
			sprintf (j_code, "X%gi", 2 * radius);
			if ((opt = GMT_Make_Option (API, 'J', j_code)) == NULL) return NULL;		/* Failed to make -J option */
			if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failed to append -J option */
			if (norm) {	/* Need a plain -S for normalization */
				if (GMT_Update_Option (API, S, "")) return NULL;		/* Failed to update -S */
			}
			else {	/* Remove the S option */
				if (GMT_Delete_Option (API, S, options)) return NULL;		/* Failed to remove -S */
			}
		}
		else {	/* No -S option given either, so user expects default radius and no normalization */
			if ((opt = GMT_Make_Option (API, 'J', "X6i")) == NULL) return NULL;		/* Failure to make -J option */
			if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append -J option */
		}
	}

	is_PS = is_PS_module (API, mod_name, keys, options);	/* true if module will produce PS */
	if (is_PS) {
		if (set_modern_mode_if_oneliner (API, options))	/* Look out for modern -png mymap and similar specs */
			return NULL;
	}

	GMT->current.ps.active = is_PS;		/* true if module will produce PS */

	if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Make sure options conform to this mode's harsh rules: */
		unsigned int n_errors = 0;
		int id, fig;
		bool got_R = false, got_J = false, exceptionb, exceptionp;
		char arg[GMT_LEN256] = {""}, scl[GMT_LEN64] = {""};
		struct GMT_OPTION *opt_J = NULL;
		struct GMT_SUBPLOT *P = NULL;

		fig = gmt_get_current_figure (API);	/* Get current figure number */

		gmtinit_get_last_dimensions (API, fig);	/* Get dimensions of previous plot, if any */

		GMT->current.ps.initialize = false;	/* Start from scratch */

		opt_R = GMT_Find_Option (API, 'R', *options);
		opt_J = gmt_find_J_option (API, *options);
		if (GMT->hidden.func_level == GMT_CONTROLLER) {	/* The -R -J -O -K prohibition only applies to top-level module call */
			/* 1. No -O allowed */
			if ((opt = GMT_Find_Option (API, 'O', *options))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Option -O not allowed for modern GMT mode.\n");
				n_errors++;
			}
			/* 2. No -K allowed */
			if ((opt = GMT_Find_Option (API, 'K', *options))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Option -K not allowed for modern GMT mode.\n");
				n_errors++;
			}
			/* 3. No -R option without arguments is allowed at top module to reach here (we may add -R later if history is available) */
			if (opt_R) {	/* Gave -R option */
				if (opt_R->arg[0] == '\0') {
					GMT_Report (API, GMT_MSG_NORMAL, "Shorthand -R not allowed for modern GMT mode.\n");
					n_errors++;
				}
				else if (!strncmp (opt_R->arg, "auto", 4U) || (opt_R->arg[0] == 'a' && opt_R->arg[1] == '\0'))	{	/* -Ra[uto] determines smart -R from data */
					if (GMT->current.ps.active) {
						if (GMT_Delete_Option (API, opt_R, options)) n_errors++;	/* Must remove old -R so next function can add a complete -R */
						n_errors += gmtinit_determine_R_option_from_data (API, required, false, options);
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Shorthand -Ra[uto] not allowed for non-plotting modules.\n");
						n_errors++;
					}
				}
				else if (!strncmp (opt_R->arg, "exact", 5U) || (opt_R->arg[0] == 'e' && opt_R->arg[1] == '\0'))	{	/* -Re[xact] determines exact -R from data */
					if (GMT->current.ps.active) {
						if (GMT_Delete_Option (API, opt_R, options)) n_errors++;	/* Must remove old -R so next function can add a complete -R */
						n_errors += gmtinit_determine_R_option_from_data (API, required, true, options);
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Shorthand -Re[xact] not allowed for non-plotting modules.\n");
						n_errors++;
					}
				}
				got_R = true;
			}
			/* 4. No -J without arguments are allowed at top module to reach here (we may add -J later if history is available) */
			if (opt_J) {
				if (opt_J->arg[0] == '\0') {
					GMT_Report (API, GMT_MSG_NORMAL, "Shorthand -J not allowed for modern GMT mode.\n");
					n_errors++;
				}
				got_J = true;
			}
			if (n_errors) {	/* Oh, well, live and learn */
				API->error = GMT_OPTION_NOT_ALLOWED;
				return NULL;
			}

			if (GMT->current.ps.active)	/* true if module will produce PS */
				(void)gmt_set_psfilename (GMT);	/* Sets GMT->current.ps.initialize=true if the expected (and hidden) PS plot file cannot be found */
		}
		else {	/* Not top-level, meaning these are modules called by other modules and they better have set -R -J if required */
			if (opt_R) got_R = true;
			if (opt_J) got_J = true;
		}

		/* Check if a subplot operation is in effect */
		if ((opt = GMT_Find_Option (API, 'c', *options))) {	/* Got -crow,col for subplot so must update gmt.panel */
			unsigned int row = 0, col = 0;
			if (opt->arg[0])	/* Gave an argument so presumably row,col */
				sscanf (opt->arg, "%d,%d", &row, &col);
			else {	/* if there is a current panel, we move to the next, otherwise set to first */
				if (gmt_get_next_panel (API, fig, &row, &col)) return NULL;	/* Bad */
			}
			if (gmt_set_current_panel (API, fig, row, col, NULL, NULL, 1)) return NULL;
			if (GMT_Delete_Option (API, opt, options)) n_errors++;	/* Remove -c option here */
		}
		/* Need to check for an active subplot, but NOT if the current call is "gmt subplot end" or psscale */
		exceptionb = (!strncmp (mod_name, "psscale", 7U));
		exceptionp = ((!strncmp (mod_name, "subplot", 7U) && *options && !strncmp ((*options)->arg, "end", 3U)));
		if (GMT->current.ps.active && !exceptionp && (P = gmt_subplot_info (API, fig))) {	/* Yes, so set up current panel settings */
			bool frame_set = false, x_set = false, y_set = false;
			char *c = NULL;
			if (exceptionb == 0 && P->first == 1) {
				/* Examine all -B settings and add/merge the panel settings */
				for (opt = *options; opt; opt = opt->next) {	/* Loop over all options */
					if (opt->option != 'B') continue;	/* Just interested in -B here */
					/* Deal with the frame option check first */
					if (strchr ("WESNwesnlrbt", opt->arg[0]))	/* User is overriding the axes settings - that is their choice */
						frame_set = true;
					else if (strstr (opt->arg, "+t") || strstr (opt->arg, "+g")) {	/* No axis specs means we have to add default */
						/* Frame but no sides specified.  Insert the required sides */
						sprintf (arg, "%s", P->Baxes);
						strncat (arg, opt->arg, GMT_LEN256-1);
						GMT_Update_Option (API, opt, arg);
						frame_set = true;
					}
					else if (opt->arg[0] == 'x') {	/* Gave specific x-setting */
						if (opt->arg[1] == '+')	{	/* No x-axis annot/tick given, prepend default determined in subplot  */
							sprintf (arg, "x%s%s", P->Bxannot, &opt->arg[1]);
							GMT_Update_Option (API, opt, arg);
						}
						if (P->Bxlabel[0]) {	/* Provided a label during subplot initialization */
							strcpy (arg, opt->arg);	/* Start with what we were given */
						 	if ((c = strstr (arg, "+l"))) {	/* See if we must append x label set during subplot call */
								c += 2;
								if (c[0] == '\0') strcat (arg, P->Bxlabel);	/* Yes, +l was empty so add preset label */
							}
							else {	/* No panel-speicific label override, use the preset label */
								strcat (arg, "+l");
								strcat (arg, P->Bxlabel);
							}
							GMT_Update_Option (API, opt, arg);
						}
						x_set = true;
					}
					else if (opt->arg[0] == 'y') {	/* Gave specific y-setting */
						if (opt->arg[1] == '+')	{	/* No x-axis annot/tick set, prepend default af */
							sprintf (arg, "y%s%s", P->Byannot, &opt->arg[1]);
							GMT_Update_Option (API, opt, arg);
						}
						if (P->Bylabel[0]) {
							strcpy (arg, opt->arg);	/* Start with what we were given */
						 	if ((c = strstr (arg, "+l"))) {	/* See if we must append y label set during subplot call */
								c += 2;
								if (c[0] == '\0') strcat (arg, P->Bylabel);	/* Yes, +l was empty so add preset label */
							}
							else {	/* No panel-specific label override, use the preset label */
								strcat (arg, "+l");
								strcat (arg, P->Bylabel);
							}
							GMT_Update_Option (API, opt, arg);
						}
						y_set = true;
					}
					else /* Gave a common x and y setting; keep as given */
						x_set = y_set = true;
				}
				if (!frame_set) {	/* Did not specify frame setting so impose the subplot choices */
					if (P->Baxes[0]) {	/* Gave frame settings */
						if ((opt = GMT_Make_Option (API, 'B', P->Baxes)) == NULL) return NULL;
					}
					else if ((opt = GMT_Make_Option (API, 'B', "0")) == NULL) return NULL;	/* Add -B0 to just draw frame */
					if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				}
				if (!x_set) {	/* Did not specify x-axis setting either via -Bx or -B so do that now */
					sprintf (arg, "x%s", P->Bxannot);	/* Start with the x tick,annot,grid choices */
					if (P->Bxlabel[0]) {strcat (arg, "+l"); strcat (arg, P->Bxlabel);}	/* Add label, if active */
					if ((opt = GMT_Make_Option (API, 'B', arg)) == NULL) return NULL;	/* Failure to make option */
					if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				}
				if (!y_set) {	/* Did not specify y-axis setting so do that now */
					sprintf (arg, "y%s", P->Byannot);	/* Start with the x tick,annot,grid choices */
					if (P->Bylabel[0]) {strcat (arg, "+l"); strcat (arg, P->Bylabel);}	/* Add label, if active */
					if ((opt = GMT_Make_Option (API, 'B', arg)) == NULL) return NULL;	/* Failure to make option */
					if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				}
			}
			if (GMT->hidden.func_level == GMT_CONTROLLER) {	/* Top-level function called by subplot needs to handle positioning and possibly set -J */
				/* Set -X -Y for absolute positioning */
				sprintf (arg, "a%gi", P->origin[GMT_X] + P->x);
				if ((opt = GMT_Make_Option (API, 'X', arg)) == NULL) return NULL;	/* Failure to make option */
				if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				sprintf (arg, "a%gi", P->origin[GMT_Y] + P->y);
				if ((opt = GMT_Make_Option (API, 'Y', arg)) == NULL) return NULL;	/* Failure to make option */
				if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				if (opt_J && !(strchr ("xX", opt_J->arg[0]) && (strchr (opt_J->arg, 'l') || strchr (opt_J->arg, 'p') || strchr (opt_J->arg, '/')))) {
					char *c = NULL;
					/* Gave -J that passed the log/power/special check, must append /1 as dummy scale/width */
					if (P->dir[GMT_X] == -1 || P->dir[GMT_Y] == -1)	/* Nonstandard Cartesian directions */
						sprintf (scl, "%gi/%gi",  P->dir[GMT_X]*P->w, P->dir[GMT_Y]*P->h);
					else	/* Just append dummy width */
						sprintf (scl, "%gi",  P->w);
					if ((c = strchr (opt_J->arg, '?'))) {	/* Scale or width was marked with ? */
						c[0] = '\0';
						sprintf (arg, "%s%s", opt_J->arg, scl);	/* Replace the ? with actual width */
						strcat (arg, &c[1]);	/* Append the rest of the arguments */
						c[0] = '?';
					}
					else if (strlen (opt_J->arg) == 1  || !strchr (opt_J->arg, '/'))	/* No markings, assume we just append */
						sprintf (arg, "%s%s", opt_J->arg, scl);	/* Append the dummy width as only argument */
					else
						sprintf (arg, "%s/%s", opt_J->arg, scl);	/* Append the dummy width to other arguments */
					GMT_Update_Option (API, opt_J, arg);	/* Failure to append option */
					GMT_Report (API, GMT_MSG_DEBUG, "Modern mode: Func level %d, Updated -J option to use -J%s.\n", GMT->hidden.func_level, opt_J->arg);
				}
			}
		}

		if (got_R == false && (strchr (required, 'R') || strchr (required, 'g') || strchr (required, 'd'))) {	/* Need a region but no -R was set */
			/* First consult the history */
			id = gmtlib_get_option_id (0, "R");	/* The -RP history item */
			if (GMT->current.ps.active || !strncmp (mod_name, "pscoast", 7U)) {	/* A plotting module (or pscoast which may run -M); first check -RP history */
				if (!GMT->init.history[id]) id++;	/* No history for -RP, increment to -RG as fallback */
			}
			else id++;	/* Only examine -RG history if not a plotter */
			if (GMT->init.history[id]) {	/* There is history for -R */
				if ((opt = GMT_Make_Option (API, 'R', "")) == NULL) return NULL;	/* Failure to make option */
				if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				GMT_Report (API, GMT_MSG_DEBUG, "Modern mode: Added -R to options since history is available.\n");
			}
			else if (strchr (required, 'g') || strchr (required, 'd')) {	/* No history but can examine input data sets */
				if (gmtinit_determine_R_option_from_data (API, required, true, options)) {
					GMT_Report (API, GMT_MSG_DEBUG, "Modern mode: Error determining the region from input data.\n");
				}
			}
		}
		if (got_J == false && strchr (required, 'J')) {	/* Need a projection but no -J was set */
			if ((id = gmtlib_get_option_id (0, "J")) >= 0 && GMT->init.history[id]) {	/* There is history for -J */
				/* Must now search for actual option since -J only has the code (e.g., -JM) */
				/* Continue looking for -J<code> */
				char str[3] = {"J"};
				str[1] = GMT->init.history[id][0];
				if ((id = gmtlib_get_option_id (id + 1, str)) >= 0 && GMT->init.history[id]) {	/* There is history for this -J */
					if ((opt = GMT_Make_Option (API, 'J', "")) == NULL) return NULL;	/* Failure to make option */
					if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
					GMT_Report (API, GMT_MSG_DEBUG, "Modern: Adding -J to options since there is history available.\n");
					got_J = true;
				}
			}
			if (got_J == false) {	/* No history, apply default projection, but watch out for subplots */
				static char *arg[2] = {"X15c", "Q15c+"};
				unsigned int geo = is_region_geographic (GMT, *options, mod_name);
				if (P && (P->dir[GMT_X] == -1 || P->dir[GMT_Y] == -1))	/* Nonstandard Cartesian axes directions */
					sprintf (scl, "X%gi/%gi",  P->dir[GMT_X]*P->w, P->dir[GMT_Y]*P->h);
				else	/* Just append dummy width */
					sprintf (scl, "%s",  arg[geo]);
				if ((opt = GMT_Make_Option (API, 'J', scl)) == NULL) return NULL;	/* Failure to make option */
				if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return NULL;	/* Failure to append option */
				GMT_Report (API, GMT_MSG_DEBUG, "Modern: Adding -J%s to options since there is no history available.\n", scl);
			}
		}
	}

	if (options && (opt = GMT_Find_Option (API, GMT_OPT_INFILE, *options))) {
		bool ocean;
		if (gmtlib_file_is_srtmrequest (API, opt->arg, &srtm_res, &ocean)) {
			unsigned int level = GMT->hidden.func_level;	/* Since we will need to increment prematurely since gmt_begin_module_sub has not been reached yet */
			char *list = NULL;
			double sgn, res[4] = {0.0, 1.0/3600.0, 0.0, 3.0/3600.0};	/* Only access 1 or 3 here */
			struct GMT_OPTION *opt_J = GMT_Find_Option (API, 'J', *options);
			opt_R = GMT_Find_Option (API, 'R', *options);
			if (opt_R == NULL) {
				GMT_Report (API, GMT_MSG_DEBUG, "Cannot select %s as input without specifying a region with -R!\n", opt->arg);
				return NULL;
			}
			/* Replace the magic reference to SRTM with a file list of SRTM tiles.
			 * Because GMT_Parse_Common has not been called yet, no -J has been processed yet.
			 * Since -J may be needed if -R does oblique or give a region in projected units
			 * we must also parse -J here first before parsing -R */
			if (opt_J) gmtinit_parse_J_option (GMT, opt_J->arg);
			opt_R = GMT_Find_Option (API, 'R', *options);
			API->GMT->hidden.func_level++;	/* Must do this here in case gmt_parse_R_option calls mapproject */
			gmt_parse_R_option (GMT, opt_R->arg);
			API->GMT->hidden.func_level = level;	/* Reset to what it should be */
			/* Enforce multiple of 1s or 3s in wesn so region is in phase with tiles */
			sgn = (GMT->common.R.wesn[XLO] < 0.0) ? -1.0 : +1.0;
			GMT->common.R.wesn[XLO] = sgn * ceil (fabs (GMT->common.R.wesn[XLO]) / res[srtm_res]) * res[srtm_res];
			sgn = (GMT->common.R.wesn[XHI] < 0.0) ? -1.0 : +1.0;
			GMT->common.R.wesn[XHI] = sgn * ceil  (fabs (GMT->common.R.wesn[XHI]) / res[srtm_res]) * res[srtm_res];
			sgn = (GMT->common.R.wesn[YLO] < 0.0) ? -1.0 : +1.0;
			GMT->common.R.wesn[YLO] = sgn * ceil (fabs (GMT->common.R.wesn[YLO]) / res[srtm_res]) * res[srtm_res];
			sgn = (GMT->common.R.wesn[YHI] < 0.0) ? -1.0 : +1.0;
			GMT->common.R.wesn[YHI] = sgn * ceil  (fabs (GMT->common.R.wesn[YHI]) / res[srtm_res]) * res[srtm_res];
			/* Get a file with a list of all needed srtm tiles */
			list = gmtlib_get_srtmlist (API, GMT->common.R.wesn, srtm_res, ocean);
			/* Replace the @[earth|srtm]_relief_0[1|3s file name with this local list */
			gmt_M_str_free (opt->arg);
			opt->arg = list;
			GMT->common.R.active[RSET] = false;	/* Since we will need to parse it again officially in GMT_Parse_Common */
		}
		else if (gmt_M_file_is_remotedata (opt->arg) && !strstr (opt->arg, ".grd")) {
			char *file = malloc (strlen(opt->arg)+5);
			sprintf (file, "%s.grd", opt->arg);
			gmt_M_str_free (opt->arg);
			opt->arg = file;
		}
	}

	/* Here we can call the rest of the initialization */

	return (gmt_begin_module_sub (API, lib_name, mod_name, Ccopy));
}

/*! Backwards compatible gmt_begin_module function for external modules built with GMT 5.3 or learlier */
struct GMT_CTRL * gmt_begin_module (struct GMTAPI_CTRL *API, const char *lib_name, const char *mod_name, struct GMT_CTRL **Ccopy) {
	API->GMT->current.setting.run_mode = GMT_CLASSIC;	/* Since gmt_begin_module is 5.3 or earlier */
	return (gmt_init_module (API, lib_name, mod_name, "", "", NULL, Ccopy));
}

/*! . */
void gmt_end_module (struct GMT_CTRL *GMT, struct GMT_CTRL *Ccopy) {
	unsigned int i, V_level = GMT->current.setting.verbose;	/* Keep copy of currently selected level */
	bool pass_changes_back;
	struct GMT_DEFAULTS saved_settings;

#ifdef HAVE_GDAL
	/* If that's the case, clean up anu GDAL CT object */
	if (GMT->current.gdal_read_in.hCT_fwd)
		OCTDestroyCoordinateTransformation(GMT->current.gdal_read_in.hCT_fwd);
	if (GMT->current.gdal_read_in.hCT_inv)
		OCTDestroyCoordinateTransformation(GMT->current.gdal_read_in.hCT_inv);
	GMT->current.gdal_read_in.hCT_fwd = GMT->current.gdal_read_in.hCT_inv = NULL;
#endif

	if (GMT->hidden.func_level == GMT_TOP_MODULE && GMT->current.ps.oneliner) {
		GMT->current.ps.oneliner = false;
		if ((i = GMT_Call_Module (GMT->parent, "end", GMT_MODULE_CMD, ""))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to call module end for a one-liner plot.\n");
			return;
		}
		GMT->current.setting.run_mode = GMT_CLASSIC;	/* all pau with modern session */
	}

	if (GMT->hidden.func_level == GMT_TOP_MODULE && GMT->parent->log_level == GMT_LOG_ONCE) {	/* Reset logging to default at the end of a top-level module */
		fclose (GMT->session.std[GMT_ERR]);
		GMT->session.std[GMT_ERR] = stderr;
		GMT->parent->log_level = GMT_LOG_OFF;
	}

	gmtinit_set_last_dimensions (GMT->parent);	/* Save canvas size */

	if (GMT->current.proj.n_geodesic_approx) {
		GMT_Report(GMT->parent, GMT_MSG_DEBUG, "Of % " PRIu64 " geodesic calls, % " PRIu64 " exceeded the iteration limit of 50.\n",
		           GMT->current.proj.n_geodesic_calls, GMT->current.proj.n_geodesic_approx);
	}

	gmtapi_garbage_collection (GMT->parent, GMT->hidden.func_level);	/* Free up all registered memory for this module level */

	/* At the end of the module we restore all GMT settings as we found them (in Ccopy) */

	pass_changes_back = (!strncmp (GMT->init.module_name, "gmtset", 6U));	/* gmtset is special as we want it to affect the session */
	if (pass_changes_back) gmt_M_memcpy (&saved_settings, &(GMT->current.setting), 1U, struct GMT_DEFAULTS);

	/* GMT_INIT */

	if (GMT->current.setting.run_mode == GMT_MODERN && !GMT->current.ps.active && GMT->common.R.active[RSET]) {	/* Modern mode: Add enhanced RG history, if possible */
		/* For grid regions we add history under RG in the form <region>[+I<incs>][+GP|G] */
		char RG[GMT_LEN256] = {""}, tmp[GMT_LEN64] = {""};
		int id = gmtlib_get_option_id (0, "R") + 1;	/* The RG history slot */
		if (GMT->init.history[id]) gmt_M_str_free (GMT->init.history[id]);
		sprintf (RG, "%.16g/%.16g/%.16g/%.16g", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		if (GMT->common.R.active[ISET]) {	/* Also want grid increment saved */
			sprintf (tmp, "+I%.16g/%.16g", GMT->common.R.inc[GMT_X], GMT->common.R.inc[GMT_Y]);
			strcat (RG, tmp);
		}
		if (GMT->common.R.active[GSET]) {	/* Also want grid registration saved */
			sprintf (tmp, "+G%c%c", GMT->common.R.registration == GMT_GRID_PIXEL_REG ? 'P' : 'G', GMT->common.R.row_order == k_nc_start_north ? 'T' : 'B');
			strcat (RG, tmp);
		}
		GMT->init.history[id] = strdup (RG);
	}

	/* We treat the history explicitly since we accumulate the history regardless of nested level */

	for (i = 0; i < GMT_N_UNIQUE; i++)
		Ccopy->init.history[i] = GMT->init.history[i];

	/* GMT_CURRENT */

	Ccopy->current.ps.clip_level = GMT->current.ps.clip_level;
	Ccopy->current.ps.layer = GMT->current.ps.layer;
	Ccopy->current.ps.active = GMT->current.ps.active;
	Ccopy->current.ps.initialize = GMT->current.ps.initialize;

	/* GMT_COMMON */

	if (Ccopy->common.U.label != GMT->common.U.label) gmt_M_str_free (Ccopy->common.U.label);
	Ccopy->common.U.label = GMT->common.U.label;
	for (i = 0; i < GMT->common.a.n_aspatial; i++) gmt_M_str_free (GMT->common.a.name[i]);
	gmt_M_str_free (GMT->common.h.title);
	gmt_M_str_free (GMT->common.h.remark);
	gmt_M_str_free (GMT->common.h.colnames);
	if (GMT->common.e.active) gmt_free_text_selection (GMT, &GMT->common.e.select);

	/* GMT_PLOT */

	gmtinit_free_plot_array (GMT);	/* Free plot arrays and reset n_alloc, n */
	gmtlib_free_custom_symbols (GMT);	/* Free linked list of custom psxy[z] symbols, if any */
	gmtinit_free_user_media (GMT);	/* Free user-specified media formats */

	/* GMT_IO */

	gmtlib_free_ogr (GMT, &(GMT->current.io.OGR), 1);	/* Free up the GMT/OGR structure, if used */
	gmtlib_free_tmp_arrays (GMT);		/* Free emp memory for vector io or processing */
	gmtinit_reset_colformats (GMT);		/* Wipe previous settings */
	gmtinit_free_dirnames (GMT);		/* Wipe previous dir names */

	gmt_fft_cleanup (GMT); /* Clean FFT resources */

	/* Overwrite GMT with what we saved in gmt_init_module */
	gmt_M_memcpy (GMT, Ccopy, 1, struct GMT_CTRL);	/* Overwrite struct with things from Ccopy */
	/* ALL POINTERS IN GMT ARE NOW JUNK AGAIN */
	if (pass_changes_back) gmt_M_memcpy (&(GMT->current.setting), &saved_settings, 1U, struct GMT_DEFAULTS);
	GMT->current.setting.verbose = V_level;	/* Pass the currently selected level back up */


	/* Try this to fix valgrind leaks */

	GMT->session.GSHHGDIR = (Ccopy->session.GSHHGDIR) ? strdup (Ccopy->session.GSHHGDIR) : NULL;
	GMT->session.DCWDIR = (Ccopy->session.DCWDIR) ? strdup (Ccopy->session.DCWDIR) : NULL;
	GMT->session.SHAREDIR = (Ccopy->session.SHAREDIR) ? strdup (Ccopy->session.SHAREDIR) : NULL;
	GMT->session.HOMEDIR = (Ccopy->session.HOMEDIR) ? strdup (Ccopy->session.HOMEDIR) : NULL;
	GMT->session.USERDIR = (Ccopy->session.USERDIR) ? strdup (Ccopy->session.USERDIR) : NULL;
	GMT->session.CACHEDIR = (Ccopy->session.CACHEDIR) ? strdup (Ccopy->session.CACHEDIR) : NULL;
	GMT->session.DATADIR = (Ccopy->session.DATADIR) ? strdup (Ccopy->session.DATADIR) : NULL;
	GMT->session.TMPDIR = (Ccopy->session.TMPDIR) ? strdup (Ccopy->session.TMPDIR) : NULL;
	GMT->session.CUSTOM_LIBS = (Ccopy->session.CUSTOM_LIBS) ? strdup (Ccopy->session.CUSTOM_LIBS) : NULL;
	GMT->session.DATAURL = (Ccopy->session.DATAURL) ? strdup (Ccopy->session.DATAURL) : NULL;

	/* Now fix things that were allocated separately */
	if (Ccopy->session.n_user_media) {
		GMT->session.n_user_media = Ccopy->session.n_user_media;
		GMT->session.user_media = gmt_M_memory (GMT, NULL, Ccopy->session.n_user_media, struct GMT_MEDIA);
		GMT->session.user_media_name = gmt_M_memory (GMT, NULL, Ccopy->session.n_user_media, char *);
		for (i = 0; i < Ccopy->session.n_user_media; i++) GMT->session.user_media_name[i] = strdup (Ccopy->session.user_media_name[i]);
	}

	gmtinit_free_user_media (Ccopy);		/* Free user-specified media formats */

	/* These are because they are kept between module calls in a same session. So we need to reset them
	 * except for Symbol and ZapfDingbats, etc. */
	if (GMT->hidden.func_level == GMT_CONTROLLER) {	/* Only when top-level module ends */
		GMT->current.setting.verbose = GMT_MSG_COMPAT;
		for (i = 0; i < (unsigned int)GMT->PSL->internal.N_FONTS; i++)
			GMT->PSL->internal.font[i].encoded = GMT->PSL->internal.font[i].encoded_orig;
		GMT->PSL->current.fontsize = 0;
	}

	gmt_M_str_free (Ccopy);	/* Good riddance */
}

void gmt_check_if_modern_mode_oneliner (struct GMTAPI_CTRL *API, int argc, char *argv[], bool gmt_main) {
	/* Determine if user is attempting a modern mode one-liner plot, and if so, set run mode to GMT_MODERN.
	 * This is needed since there is not gmt begin | end sequence in this case.
	 * Also, if a user wants to get the usage message for a modern mode module then it is also a type
	 * of one-liner and thus we set to GMT_MODERN as well, but only for modern module names. */
	
	unsigned modern = 0, pos, k = 0;
	int n_args = argc - 1;
	char figure[GMT_LEN128] = {""}, p[GMT_LEN16] = {""}, *c = NULL;
	bool usage = false;

	if (n_args == 0) return;    /* This is just typing gmt with no args */
	if (gmt_main) {
		n_args--;	/* Count number of args after module name */
		k = 1;
	}
	API->GMT->current.setting.use_modern_name = gmtlib_is_modern_name (API, argv[k]);
	
	if (API->GMT->current.setting.run_mode == GMT_MODERN && !strncmp (argv[k], "ps", 2U)) {	/* Gave classic ps* name in modern mode */
		char not_used[GMT_LEN32] = {""};
		const char *mod_name = gmt_current_name (argv[k], not_used);
		GMT_Report (API, GMT_MSG_VERBOSE, "Detected a classic module name (%s) in modern mode - please use the modern mode name %s instead.\n", argv[k], mod_name);
	}
	if (API->GMT->current.setting.use_modern_name) {
		if (n_args == 0) {	/* Gave none or a single argument */
			if (API->GMT->current.setting.run_mode == GMT_CLASSIC)
				API->usage = true;	/* Modern mode name given with no args so not yet in modern mode - allow it to get usage */
			return;
		}
		if (n_args == 1) {	/* Gave a single argument */
			if (argv[argc-1][0] == '+' && argv[argc-1][1] == '\0') {	/* Gave + */
				modern = 1;
			}
			else if (argv[argc-1][0] == '-' && (argv[argc-1][1] == '\0' || argv[argc-1][1] == GMT_OPT_USAGE || argv[argc-1][1] == GMT_OPT_SYNOPSIS)) {	/* Gave a single argument */
				modern = 1;
			}
			if (modern) usage = true;
		}
	}
	/* Must check if a one-liner with special graphics format settings were given, e.g., -png map */
	for (k = 1; !modern && k < (unsigned int)argc; k++) {
		if (argv[k][0] != '-') continue;	/* Skip file names */
		if (strlen (argv[k]) < 3 || strlen (argv[k]) >= GMT_LEN128) continue;	/* -ps is the shortest format extension, and very long args are filenames*/
		strncpy (figure, &argv[k][1],  GMT_LEN128-1);	/* Get a local copy so we can mess with it, but skip the leading - */
		if ((c = strchr (figure, ','))) c[0] = 0;	/* Chop off other formats for the initial id test */
		if (gmt_get_graphics_id (API->GMT, figure) != GMT_NOTSET) {	/* Found a valid one-liner option */
			modern = 1;	/* Seems like it is, but check the rest of the formats, if there are more */
			if (c == NULL) continue;	/* Nothing else to check */
			/* Make sure any other formats are valid, too */
			if (c) c[0] = ',';	/* Restore any comma we found */
			pos = 0;
			while (modern && gmt_strtok (figure, ",", &pos, p)) {	/* Check each format to make sure each is OK */
				if (gmt_get_graphics_id (API->GMT, p) == GMT_NOTSET)
					modern = 0;
			}
		}
	}
	if (modern) {	/* This is indeed a modern mode one-liner */
		API->GMT->current.setting.run_mode = GMT_MODERN;
		API->usage = usage;
	}
	if (API->GMT->current.setting.run_mode == GMT_MODERN)	/* If running in modern mode we want to use modern names */
		API->GMT->current.setting.use_modern_name = true;
}

/*! Update vector head length and width parameters based on size_z and v_angle, and deal with pen/fill settings */
int gmt_init_vector_param (struct GMT_CTRL *GMT, struct GMT_SYMBOL *S, bool set, bool outline, struct GMT_PEN *pen, bool do_fill, struct GMT_FILL *fill) {
	bool no_outline = false, no_fill = false;
	if (set) {	/* Determine proper settings for head fill or outline */
		if (outline && (S->v.status & PSL_VEC_OUTLINE2) == 0) S->v.pen = *pen;	/* If no +p<pen> but -W<pen> was used, use same pen for vector heads */
		else if (!outline && S->v.status & PSL_VEC_OUTLINE2) *pen = S->v.pen;	/* If no -W<pen> was set but +p<pen> given, use same pen for vector tails */
		else if (!outline && (S->v.status & PSL_VEC_OUTLINE2) == 0) no_outline = true;
		if (do_fill && (S->v.status & PSL_VEC_FILL2) == 0) S->v.fill = *fill;	/* If no +g<fill> but -G<fill> was used, use same fill for vector heads */
		else if (!do_fill && S->v.status & PSL_VEC_FILL2) no_fill = false;		/* If no -G<fill> was set but +g<fill> given, we do nothing here */
		else if (!do_fill && (S->v.status & PSL_VEC_FILL2) == 0) no_fill = true;	/* Neither -G<fill> nor +g<fill> were set */
		if (no_outline && no_fill && (S->v.status & PSL_VEC_HEADS)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot draw vector heads without specifying at least one of head outline or head fill.\n");
			return 1;
		}
	}
	if (gmt_M_is_zero (S->size_x)) return 0;	/* Not set yet */
	if (!(S->symbol == GMT_SYMBOL_VECTOR_V4 || S->v.parsed_v4)) {
		S->v.h_length = (float)S->size_x;
		S->v.h_width = (float)(2.0 * S->v.h_length * tand (0.5 * S->v.v_angle));
	}
	return 0;
}

/*! Parser for -Sv|V, -S=, and -Sm */
int gmt_parse_vector (struct GMT_CTRL *GMT, char symbol, char *text, struct GMT_SYMBOL *S) {

	unsigned int pos = 0, k, f = 0, end, error = 0;
	size_t len;
	bool p_opt = false, g_opt = false;
	int j;
	char p[GMT_BUFSIZ];
	double value[2];

	S->v.pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &S->v.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	S->v.status = 0;	/* Start with no flags turned on */
	S->v.v_angle = 30.0f;	S->v.v_norm = -1.0f;	S->v.v_stem = 0.1f;
	S->v.v_kind[0] = S->v.v_kind[1] = PSL_VEC_ARROW;
	S->v.v_shape = (float)GMT->current.setting.map_vector_shape;	/* Can be overridden with +h<shape> */
	for (k = 0; text[k] && text[k] != '+'; k++);	/* Either find the first plus or run out or chars */
	strncpy (p, text, k); p[k] = 0;

	while ((gmt_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
		switch (p[0]) {
			case 'a': S->v.v_angle = (float)atof (&p[1]);	break;	/* Vector head opening angle [30] */
			case 'b':	/* Vector head at beginning point */
				S->v.status |= PSL_VEC_BEGIN;
				switch (p[1]) {
					case 'a': S->v.v_kind[0] = PSL_VEC_ARROW;	break; /* Explicitly selected arrow head */
					case 'A': S->v.v_kind[0] = PSL_VEC_ARROW_PLAIN;	break;
					case 'i': S->v.v_kind[0] = PSL_VEC_TAIL;	break;
					case 'I': S->v.v_kind[0] = PSL_VEC_TAIL_PLAIN;	break;
					case 'c': S->v.v_kind[0] = PSL_VEC_CIRCLE;	break;
					case 's': S->v.v_kind[0] = PSL_VEC_SQUARE;	break;
					case 't': S->v.v_kind[0] = PSL_VEC_TERMINAL;	break;
			  	 	case 'l': S->v.v_kind[0] = PSL_VEC_ARROW;	S->v.status |= PSL_VEC_BEGIN_L;	break;	/* Only left  half of head requested */
			  	  	case 'r': S->v.v_kind[0] = PSL_VEC_ARROW;	S->v.status |= PSL_VEC_BEGIN_R;	break;	/* Only right half of head requested */
					default:  S->v.v_kind[0] = PSL_VEC_ARROW;	break;
				}
				if (p[1] && p[2]) {
					if (p[2] == 'l') S->v.status |= PSL_VEC_BEGIN_L;	/* Only left  half of head requested */
	  	  			else if (p[2] == 'r') S->v.status |= PSL_VEC_BEGIN_R;	/* Only right half of head requested */
				}
				break;
			case 'e':	/* Vector head at end point */
				S->v.status |= PSL_VEC_END;
				switch (p[1]) {
					case 'a': S->v.v_kind[1] = PSL_VEC_ARROW;	break;	/* Explicitly selected arrow head */
					case 'A': S->v.v_kind[1] = PSL_VEC_ARROW_PLAIN;	break;
					case 'i': S->v.v_kind[1] = PSL_VEC_TAIL;	break;
					case 'I': S->v.v_kind[1] = PSL_VEC_TAIL_PLAIN;	break;
					case 'c': S->v.v_kind[1] = PSL_VEC_CIRCLE;	break;
					case 's': S->v.v_kind[1] = PSL_VEC_SQUARE;	break;
					case 't': S->v.v_kind[1] = PSL_VEC_TERMINAL;	break;
			  	 	case 'l': S->v.v_kind[1] = PSL_VEC_ARROW;	S->v.status |= PSL_VEC_END_L;	break;	/* Only left  half of head requested */
			  	  	case 'r': S->v.v_kind[1] = PSL_VEC_ARROW;	S->v.status |= PSL_VEC_END_R;	break;	/* Only right half of head requested */
					default:  S->v.v_kind[1] = PSL_VEC_ARROW;	break;
				}
				if (p[1] && p[2]) {
					if (p[2] == 'l') S->v.status |= PSL_VEC_END_L;	/* Only left half of head requested */
	  	  			else if (p[2] == 'r') S->v.status |= PSL_VEC_END_R;	/* Only right half of head requested */
				}
				break;
			case 'g':	/* Vector head fill +g[-|<fill>]*/
				g_opt = true;	/* Marks that +g was used */
				if (p[1] == '-') break; /* Do NOT turn on fill */
				S->v.status |= PSL_VEC_FILL;
				if (p[1]) {
					if (gmt_getfill (GMT, &p[1], &S->v.fill)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +g<fill> argument %s\n", &p[1]);
						error++;
					}
					S->v.status |= PSL_VEC_FILL2;
				}
				else
					S->v.status |= PSL_VEC_FILL;
				break;
			case 'h':	/* Vector shape [MAP_VECTOR_SHAPE] */
				S->v.v_shape = (float)atof (&p[1]);
				if (S->v.v_shape < -2.0 || S->v.v_shape > 2.0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +h<shape> modifier value: Must be in -2/+2 range\n");
					error++;
				}
				break;
			case 'j':	/* Vector justification */
				if (symbol == 'm') {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-Sm does not accept +j<just> modifiers.\n");
					error++;
				}
				else {
					switch (p[1]) {
						case 'b': S->v.status |= PSL_VEC_JUST_B;	break;	/* Input (x,y) refers to vector beginning point */
						case 'c': S->v.status |= PSL_VEC_JUST_C;	break;	/* Input (x,y) refers to vector center point */
						case 'e': S->v.status |= PSL_VEC_JUST_E;	break;	/* Input (x,y) refers to vector end point */
						default:  /* Bad justifier code */
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +j<just> modifier %c\n", p[1]);
							error++;
							break;
					}
				}
				break;
			case 'l': S->v.status |= (PSL_VEC_BEGIN_L + PSL_VEC_END_L);	break;	/* Obsolete modifier for left halves at active heads */
			case 'm':	/* Vector head at midpoint of segment */
				switch (p[1]) {
					case '\0':	f = 1;	S->v.status |= PSL_VEC_MID_FWD;	break;	/* Place forward-pointing arrow head at center of segment */
					case 'f':	f = 2;	S->v.status |= PSL_VEC_MID_FWD;	break;	/* Place forward-pointing arrow head at center of segment */
					case 'r':	f = 2;	S->v.status |= PSL_VEC_MID_BWD;	break;	/* Place backward-pointing arrow head at center of segment */
					case 'a': case 'c': case 's': case 't':	f = 1;	S->v.status |= PSL_VEC_MID_FWD;	break;	/* Handle these below */
					default:  /* Bad direction code */
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +m<dir> modifier %c\n", p[1]);
						error++;
						break;
				}
				end = (S->v.status & PSL_VEC_MID_FWD) ? PSL_END : PSL_BEGIN;	/* Which head-type to use at center */
				switch (p[f]) {	/* Optional types */
					case 'a': S->v.v_kind[end] = PSL_VEC_ARROW;	break;	/* Explicitly selected arrow head */
					case 'A': S->v.v_kind[end] = PSL_VEC_ARROW_PLAIN;	break;
					case 'i': S->v.v_kind[end] = PSL_VEC_TAIL;	break;
					case 'I': S->v.v_kind[end] = PSL_VEC_TAIL_PLAIN;	break;
					case 'c': S->v.v_kind[end] = PSL_VEC_CIRCLE;	break;
					case 's': S->v.v_kind[end] = PSL_VEC_SQUARE;	break;
					case 't': S->v.v_kind[end] = PSL_VEC_TERMINAL;	break;
			  	 	case 'l': S->v.v_kind[end] = PSL_VEC_ARROW;	S->v.status |= PSL_VEC_END_L;	break;	/* Only left  half of head requested */
			  	  	case 'r': S->v.v_kind[end] = PSL_VEC_ARROW;	S->v.status |= PSL_VEC_END_R;	break;	/* Only right half of head requested */
					default:  S->v.v_kind[end] = PSL_VEC_ARROW;	break;	/* Default is arrow */
				}
				if (p[f] && p[f+1]) {
	  	  			if (p[f+1] == 'l') S->v.status |= PSL_VEC_END_L;	/* Only left  half of head requested */
	  	  			else if (p[f+1] == 'r') S->v.status |= PSL_VEC_END_R;	/* Only right half of head requested */
				}
				break;
			case 'n':	/* Vector shrinking head */
				len = strlen (p);
				j = (symbol == 'v' || symbol == 'V') ? gmtinit_get_unit (GMT, p[len-1]) : -1;	/* Only -Sv|V takes unit */
				S->v.v_norm = (float)atof (&p[1]);	/* This is normalizing length in given units, not (yet) converted to inches or degrees (but see next lines) */
				if (symbol == '=') {	/* Since norm distance is in km we convert to spherical degrees */
					if (strchr (GMT_DIM_UNITS GMT_LEN_UNITS2, p[len-1]) && p[len-1] != 'k') {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Vector shrink length limit for geovectors must be given in km!\n");
						error++;
					}
					S->v.v_norm /= (float)GMT->current.proj.DIST_KM_PR_DEG;
				}
				else if (j >= GMT_CM)	/* Convert length from given unit to inches */
					S->v.v_norm *= GMT->session.u2u[j][GMT_INCH];
				else	/* Convert length from default unit to inches */
					S->v.v_norm *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				/* Here, v_norm is either in inches (if Cartesian vector) or spherical degrees (if geovector) */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Vector shrink scale v_norm = %g\n", S->v.v_norm);
				break;
			case 'o':	/* Sets oblique pole for small or great circles */
				S->v.status |= PSL_VEC_POLE;
				if (!p[1]) {	/* Gave no pole, use North pole */
					S->v.pole[GMT_X] = 0.0f;	S->v.pole[GMT_Y] = 90.0f;
				}
				else if ((j = GMT_Get_Values (GMT->parent, &p[1], value, 2)) != 2) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +o[<plon>/<plat>] argument %s\n", &p[1]);
					error++;
				}
				else {	/* Successful parsing of pole */
					S->v.pole[GMT_X] = (float)value[GMT_X];	S->v.pole[GMT_Y] = (float)value[GMT_Y];}
				break;
			case 'p':	/* Vector pen and head outline +p[-][<pen>] */
				p_opt = true;	/* Marks that +p was used */
				if (p[1] == '-')	/* Do NOT turn on head outlines */
					j = 2;
				else {	/* Do turn on head outlines */
					j = 1;
					S->v.status |= PSL_VEC_OUTLINE;
				}
				if (p[j]) {	/* Change default vector pen */
					if (gmt_getpen (GMT, &p[j], &S->v.pen)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +p<pen> argument %s\n", &p[1]);
						error++;
					}
					S->v.status |= PSL_VEC_OUTLINE2;	/* Flag that a pen specification was given */
				}
				/* So status may here be both PSL_VEC_OUTLINE (draw head outline) and PSL_VEC_OUTLINE2 (do it with this pen) */
				break;
			case 'q': S->v.status |= PSL_VEC_ANGLES;	break;	/* Expect start,stop angle rather than length in input */
			case 'r': S->v.status |= (PSL_VEC_BEGIN_R + PSL_VEC_END_R);	break;	/* Obsolete modifier for right halves at active heads */
			case 's': S->v.status |= PSL_VEC_JUST_S;	break;	/* Input (angle,length) are vector end point (x,y) instead */
			case 't':	/* Get endpoint trim(s) */
				switch (p[1]) {
					case 'b':	f = 2;	S->v.status |= PSL_VEC_OFF_BEGIN;	break;	/* Shift begin point by some trim amount */
					case 'e':	f = 2;	S->v.status |= PSL_VEC_OFF_END;		break;	/* Shift end point by some trim amount */
					default:  	f = 1;	S->v.status |= (PSL_VEC_OFF_BEGIN+PSL_VEC_OFF_END);	break;	/* Do both */
				}
				if ((j = gmt_get_pair (GMT, &p[f], GMT_PAIR_DIM_DUP, value)) < 1)  {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +t <trim> values %s\n", &p[f]);
					error++;
				}
				else {	/* Successful parsing of trim(s) */
					if (S->v.status & PSL_VEC_OFF_BEGIN) S->v.v_trim[0] = (float)value[0];
					if (S->v.status & PSL_VEC_OFF_END)   S->v.v_trim[1] = (float)value[1];
				}
				break;
			case 'z':	/* Input (angle,length) are vector components (dx,dy) instead */
				S->v.status |= PSL_VEC_COMPONENTS;
				S->v.comp_scale = (float)gmt_convert_units (GMT, &p[1], GMT->current.setting.proj_length_unit, GMT_INCH);
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad modifier +%c\n", p[0]);
				error++;
				break;
		}
	}
	if ((S->v.status & PSL_VEC_MID_FWD || S->v.status & PSL_VEC_MID_BWD) && (S->v.status & PSL_VEC_BEGIN || S->v.status & PSL_VEC_END)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot combine mid-point vector head (+m) with end-point heads (+b | +e)\n");
		error++;
	}
	if (!g_opt) S->v.status |= PSL_VEC_FILL;	/* Default is to fill vector head with current fill unless (a) no fill given or (b) turned off with +g- */
	if (!p_opt) S->v.status |= PSL_VEC_OUTLINE;	/* Default is to draw vector head outline with current pen unless explicitly turned off with +p- */

	/* Set head parameters */
	gmt_init_vector_param (GMT, S, false, false, NULL, false, NULL);

	return (error);
}

#define GMT_VECTOR_CODES "mMvV="	/* The vector symbol codes */

/*! . */
int gmt_parse_symbol_option (struct GMT_CTRL *GMT, char *text, struct GMT_SYMBOL *p, unsigned int mode, bool cmd) {
	/* mode = 0 for 2-D (psxy) and = 1 for 3-D (psxyz); cmd = true when called to process command line options */
	int decode_error = 0, bset = 0, j, n, k, slash = 0, colon, col_off = mode, len, n_z = 0;
	bool check = true, degenerate = false, add_to_base = false;
	unsigned int ju;
	char symbol_type, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, text_cp[GMT_LEN256] = {""}, diameter[GMT_LEN32] = {""}, *c = NULL;
	static char *allowed_symbols[2] = {"~=-+AaBbCcDdEefGgHhIiJjMmNnpqRrSsTtVvWwxy", "=-+AabCcDdEefGgHhIiJjMmNnOopqRrSsTtUuVvWwxy"};
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
	p->font = GMT->current.setting.font_annot[GMT_PRIMARY];
	if (p->read_size)  p->given_size_x = p->given_size_y = p->size_x = p->size_y = 0.0;
	p->factor = 1.0;

	/* col_off is the col number of first parameter after (x,y) [or (x,y,z) if mode == 1)].
	   However, if size is not given then that is required too so col_off++ */

	if (!strncmp (text, "E-", 2U) || !strncmp (text, "J-", 2U)) {
		/* Special degenerate geographic ellipse and rectangle symbols, remove the - to avoid parsing issues */
		degenerate = true;
		if (text[2]) strncpy (diameter, &text[2], GMT_LEN32-1);	/* Gave circle diameter on command line */
		text[1] = 0;
	}
	if (!text[0]) {	/* No symbol or size given */
		p->size_x = p->size_y = 0.0;
		symbol_type = '*';
		col_off++;
		if (cmd) p->read_size_cmd = true;
		if (cmd) p->read_symbol_cmd = 1;
	}
	else if (isdigit ((int)text[0]) || text[0] == '.') {	/* Size, but no symbol given */
		n = sscanf (text, "%[^/]/%s", txt_a, txt_b);
		p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
		if (n == 2)
			p->size_y = p->given_size_y = gmt_M_to_inch (GMT, txt_b);
		else if (n == 1)
			p->size_y = p->given_size_y = p->size_x;
		else
			decode_error = true;
		symbol_type = '*';
		if (cmd) p->read_symbol_cmd = 1;
	}
	else if (text[0] == 'l') {	/* Letter symbol is special case */
		strncpy (text_cp, text, GMT_LEN256-1);	/* Copy for processing later */
		symbol_type = 'l';
		if (!text[1]) {	/* No size or text given */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if (cmd) p->read_size_cmd = true;
			col_off++;
		}
		else if (text[1] == '+' || (text[1] == '/' && gmt_M_compat_check (GMT, 4))) {	/* No size given */
			/* Any deprecate message comes below so no need here */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			col_off++;
		}
		else {
			n = sscanf (&text_cp[1], "%[^+]+%*s", txt_a);
			p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
			decode_error = (n != 1);
		}
	}
	else if (text[0] == 'k' || text[0] == 'K') {	/* Custom symbol spec -Sk|K<path_to_custom_symbol>[/<size>[unit]]*/
		/* <path_to_custom_symbol> may contains slashes (UNIX) or backslashes (Windows) so we search from the end of the string: */
		for (j = (int)strlen (text)-1; j > 0 && text[j] != '/'; --j);	/* Determine last slash */
		if (j == 0) {	/* No slash, i.e., no symbol size given (and no UNIX path either) */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			sscanf (text, "%c%s", &symbol_type, text_cp);
			col_off++;
		}
		else {	/* Found a slash, is it separating the size or part of UNIX path? */
			text[j] = ' ';	/* Temporary remove the slash we found */
			sscanf (text, "%c%s %s", &symbol_type, text_cp, txt_a);
			text[j] = '/';	/* Restore the bugger */
			if (strchr("CcIiPp", txt_a[strlen(txt_a) - 1])) {	/* If last char equals a unit char... */
				char t[GMT_LEN64] = {""};
				strncpy (t, txt_a, strlen(txt_a) - 1);	/* Make a copy of what we found minus the unit char */
				if (gmtinit_is_valid_number(t))         /* txt_a without the unit char is a size string. Decode it. */
					p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
				else                                /* txt_a is just the symbol name and text_cp has the directory */
					{strcat(text_cp, "/");	strcat(text_cp, txt_a);col_off++;}
			}
			else {                                  /* It still can be a size even though no unit was given */
				if (gmtinit_is_valid_number (txt_a))     /* Yes, it is */
					p->size_x = p->given_size_x = gmt_M_to_inch(GMT, txt_a);
				else {                              /* No, txt_a is the symbol name. */
					strcat(text_cp, "/");	strcat(text_cp, txt_a);
					if (p->size_x == 0.0) p->size_x = p->given_size_x;
					col_off++;
				}
			}
		}
	}
	else if (strchr (GMT_VECTOR_CODES, text[0])) {
		/* Vectors gets separate treatment because of optional modifiers [+j<just>+b+e+s+l+r+a<angle>+n<norm>] */
		int one;
		char arg[GMT_LEN64] = {""}, txt_c[GMT_LEN256] = {""};
		n = sscanf (text, "%c%[^+]", &symbol_type, arg);	/* arg should be symbols size with no +<modifiers> at the end */
		if (n == 1) strncpy (arg, &text[1], GMT_LEN64-1);	/* No modifiers present, set arg to text following symbol code */
		k = 1;
		p->v.parsed_v4 = false;
		if (strchr (text, '/') && !strchr (text, '+')) {
			/* Gave old-style arrow dimensions; cannot exactly reproduce GMT 4 arrows since those were polygons */
			p->v.status |= PSL_VEC_END;		/* Default is head at end */
			p->size_y = p->given_size_y = 0.0;
			one = (strchr ("bhstBHST", text[1])) ? 2 : 1;
			sscanf (&text[one], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			p->v.v_width  = (float)gmt_M_to_inch (GMT, txt_a);
			p->v.h_length = (float)gmt_M_to_inch (GMT, txt_b);
			p->v.h_width  = (float)(gmt_M_to_inch (GMT, txt_c) * 2.0);
			p->v.v_angle  = (float)atand ((0.5 * p->v.h_width / p->v.h_length) * 2.0);
			p->v.parsed_v4 = true;
			p->size_x = p->given_size_x = p->v.h_length;
		}
		else if (strchr ("vV", symbol_type) && text[1] && strchr ("bhstBHST", text[1])) {	/* Old style */
			//GMT_Report (GMT->parent, GMT_MSG_COMPAT, "bhstBHST vector modifiers is deprecated GMT3/4 syntax; see -S%c for current syntax.\n", text[0]);
			p->v.status |= PSL_VEC_END;		/* Default is head at end */
			k = 2;
			strncpy (arg, &text[2], GMT_LEN64-1);
		}
		if (text[k] && strchr (GMT_DIM_UNITS, (int) text[k])) {	/* No size given, only unit information */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if ((j = gmtinit_get_unit (GMT, text[k])) < 0) decode_error = true; else { p->u = j; p->u_set = true;}
			col_off++;
			if (cmd) p->read_size_cmd = true;
		}
		else if (!text[k] || text[k] == '+') {	/* No size nor unit, just possible attributes */
			if (p->size_x == 0.0) p->size_x = p->given_size_x;
			if (p->size_y == 0.0) p->size_y = p->given_size_y;
			if (p->size_x == 0.0)	/* If symbol size was given on command line then we don't want to read it again */
				col_off++;
			if (cmd) p->read_size_cmd = true;
		}
		else if (!p->v.parsed_v4) {	/* Need to get size */
			if (cmd) p->read_size_cmd = false;
			p->size_x = p->given_size_x = gmt_M_to_inch (GMT, arg), check = false;
		}
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && strchr (GMT_DIM_UNITS, (int) text[1])) {
		/* Symbol, but no size given (size assumed given on command line), only unit information */
		sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		if (text[1]) {	/* Gave unit information */
			if ((j = gmtinit_get_unit (GMT, text[1])) < 0)
				decode_error = true;
			else {
				p->u = j; p->u_set = true;
			}
		}
		col_off++;
	}
	else if (strchr (allowed_symbols[mode], (int) text[0]) && (text[1] == '\n' || !text[1])) {
		/* Symbol, but no size given (size assumed given on command line) */
		sscanf (text, "%c", &symbol_type);
		if (p->size_x == 0.0) p->size_x = p->given_size_x;
		if (p->size_y == 0.0) p->size_y = p->given_size_y;
		col_off++;
	}
	else if (strchr (bar_symbols[mode], (int) text[0])) {	/* Bar, column, cube with size */

		/* Bar:		-Sb|B[<size_x|size_y>[c|i|p|u]][+b|B[<base>]]				*/
		/* Column:	-So|O[<size_x>[c|i|p|u][/<ysize>[c|i|p|u]]][+b|B[<base>]]	*/
		/* Cube:	-Su|U[<size_x>[c|i|p|u]]	*/

		for (j = 1; text[j]; j++) {	/* Look at chars following the symbol code */
			if (text[j] == '/') slash = j;
			if (text[j] == 'b' || text[j] == 'B') bset = j;	/* Basically not worry about +b|B vs b|B by just checking for b|B */
		}
		if ((c = strstr (text, "+z")) || (c = strstr (text, "+Z"))) {	/* Got +z|Z<nz> */
			n_z = atoi (&c[2]);
			if (c[1] == 'Z') p->accumulate = true;	/* Getting dz1 dz2 ... etc and not z1 z1 ... */
			c[0] = '\0';	/* Temporarily chop this off... */
		}
		strncpy (text_cp, text, GMT_LEN256-1);
		if (c) c[0] = '+';	/* ...and restore it */
		if (bset) {	/* Chop off the b|B<base> from copy to avoid confusion when parsing.  <base> is always in user units */
			if (text_cp[bset] == 'B') add_to_base = true;
			if (text_cp[bset-1] == '+')	/* Gave +b|B */
				text_cp[bset-1] = 0;
			else	/* Handle backwards compatible case of just b[<base>] */
				text_cp[bset] = 0;
		}
		if (slash) {	/* Separate x/y sizes */
			n = sscanf (text_cp, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);
			decode_error = (n != 3);
			if (((len = (int)strlen (txt_a)) > 0) && txt_a[len-1] == 'u') {
				p->user_unit[GMT_X] = true;	/* Specified xwidth in user units */
				txt_a[len-1] = '\0';	/* Chop off the 'u' */
			}
			if (((len = (int)strlen (txt_b)) > 0) && txt_b[len-1] == 'u') {
				p->user_unit[GMT_Y] = true;	/* Specified ywidth in user units */
				txt_b[len-1] = '\0';	/* Chop off the 'u' */
			}
			if (p->user_unit[GMT_X]) {
				if (gmtinit_get_uservalue (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), &p->given_size_x, "-Sb|B|o|O|u|u x-size value")) return GMT_PARSE_ERROR;
				p->size_x = p->given_size_x;
			}
			else
				p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
			if (p->user_unit[GMT_Y]) {
				if (gmtinit_get_uservalue (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), &p->given_size_y, "-Sb|B|o|O|u|u y-size value")) return GMT_PARSE_ERROR;
				p->size_y = p->given_size_y;
			}
			else
				p->size_y = p->given_size_y = gmt_M_to_inch (GMT, txt_b);
		}
		else {	/* Only a single x = y size */
			n = sscanf (text_cp, "%c%s", &symbol_type, txt_a);
			if ((len = (int)strlen (txt_a)) && txt_a[len-1] == 'u') {
				p->user_unit[GMT_X] = p->user_unit[GMT_Y] = true;	/* Specified xwidth [=ywidth] in user units */
				txt_a[len-1] = '\0';	/* Chop off the 'u' */
			}
			if (n == 2) {	/* Gave size */
				if (p->user_unit[GMT_X]) {
					if (gmtinit_get_uservalue (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), &p->given_size_x, "-Sb|B|o|O|u|u x-size value")) return GMT_PARSE_ERROR;
					p->size_x = p->size_y = p->given_size_y = p->given_size_x;
				}
				else
					p->size_x = p->size_y = p->given_size_x = p->given_size_y = gmt_M_to_inch (GMT, txt_a);
			}
			else {
				if (p->size_x == 0.0) p->size_x = p->given_size_x;
				if (p->size_y == 0.0) p->size_y = p->given_size_y;
			}
		}
	}
	else {	/* Everything else */
		char s_upper;
		size_t len;
		n = sscanf (text, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);
		s_upper = (char)toupper ((int)symbol_type);
		if (strchr ("FVQM~", s_upper))	/* "Symbols" that do not take a normal symbol size */
			p->size_y = p->given_size_y = 0.0;
		else if (s_upper == 'W') {	/* Wedges and spiders: -Sw|W<diameter>[/<inner_diameter>][+a[<dr>][+r[<da>]]] */
			unsigned int type = 0;
			char *c = NULL;
			if ((c = gmt_first_modifier (GMT, text, "apr"))) c[0] = '\0';	/* Chop off modifiers so we can parse the info */
			n = sscanf (text, "%c%[^/]/%s", &symbol_type, txt_a, txt_b);	/* Redo since we need txt_b without modifiers */
			len = strlen (txt_a);
			if (strchr (GMT_LEN_UNITS, txt_a[len-1])) {	/* Geo-wedge with radius given in a distance unit */
				(void)gmtlib_scanf_geodim (GMT, txt_a, &p->w_radius);
				p->size_y = p->given_size_y = 0.0;
				check = false;
				p->w_active = true;
			}
			else if (symbol_type == 'W') {	/* Can take either plot diameter or geo diameter so must check */
				if (strchr (GMT_DIM_UNITS, txt_a[len-1]))	/* Gave a plot unit diameter, convert to inches */
					p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
				else {	/* Add the missing k for km */
					strcat (txt_a, "k");
					(void)gmtlib_scanf_geodim (GMT, txt_a, &p->w_radius);
					p->size_y = p->given_size_y = 0.0;
					check = false;
					p->w_active = true;
				}
			}
			else	/* Cartesian wedge in plot units */
				p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
			if (txt_b[0]) {	/* Gave an inner diameter as well instead of the default of 0 */
				if (p->w_active) /* Geo-wedge with distance units */
					(void)gmtlib_scanf_geodim (GMT, txt_b, &p->w_radius_i);
				else	/* Gave plot units */
					p->w_radius_i = gmt_M_to_inch (GMT, txt_b);
			}
			if (c) {	/* Now process any modifiers */
				char q[GMT_LEN256] = {""};
				unsigned int pos = 0, error = 0;
				c[0] = '+';	/* Restore that character */
				while (gmt_getmodopt (GMT, 'S', c, "apr", &pos, q, &error) && error == 0) {
					switch (q[0]) {
						case 'a':	/* Arc(s) */
							type |= GMT_WEDGE_ARCS;	/* Arc only */
							if (q[1]) {	/* Got delta_r */
								if (p->w_active)	/* Geo-wedge */
									(void)gmtlib_scanf_geodim (GMT, &q[1], &p->w_dr);
								else
									p->w_dr = gmt_M_to_inch (GMT, &q[1]);
							}
							break;
						case 'p':	/* Spider pen, stored in the vector structure */
							if (gmt_getpen (GMT, &q[1], &p->v.pen)) {
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +p<pen> argument %s\n", &q[1]);
								error++;
							}
							p->v.status = PSL_VEC_OUTLINE2;	/* Flag that a pen specification was given */
							break;
						case 'r':	/* Radial lines */
							type |= GMT_WEDGE_RADII;	/* Radial lines */
							if (q[1])	/* Got delta_az */
								p->w_da = atof (&q[1]);
							break;
						default:	/* These are caught in gmt_getmodopt so break is just for Coverity */
							break;
					}
				}
			}
			p->w_type = type;
		}
		else {
			p->size_x = p->given_size_x = gmt_M_to_inch (GMT, txt_a);
			if (n == 3)
				p->size_y = p->given_size_y = gmt_M_to_inch (GMT, txt_b);
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
			p->symbol = PSL_XDASH;
			break;
		case 'A':
			p->factor = 1.67289326141;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 'a' */
		case 'a':
			p->symbol = PSL_STAR;
			break;
		case 'B':
			p->symbol = GMT_SYMBOL_BARX;
			if (bset) {
				if (text[bset+1] == '\0') {	/* Read it from data file (+ means probably +z<col>) */
					p->base_set = 2;
					p->n_required = 1;
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* base in user units */
				}
				else {
					if (gmtinit_get_uservalue (GMT, &text[bset+1], gmt_M_type (GMT, GMT_IN, GMT_X), &p->base, "-SB base value")) return GMT_PARSE_ERROR;
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
					if (gmtinit_get_uservalue (GMT, &text_cp[bset+1], gmt_M_type (GMT, GMT_IN, GMT_Y), &p->base, "-Sb base value")) return GMT_PARSE_ERROR;
					p->base_set = 1;
				}
			}
			break;
		case 'C':
		case 'c':
			p->symbol = PSL_CIRCLE;
			break;
		case 'D':
			p->factor = 1.25331413732;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 'd' */
		case 'd':
			p->symbol = PSL_DIAMOND;
			break;
		case 'E':	/* Expect axis in km to be scaled based on -J */
			p->symbol = PSL_ELLIPSE;
			p->convert_angles = 1;
			if (degenerate) {	/* Degenerate ellipse = circle */
				if (diameter[0]) {	/* Gave a fixed diameter as symbol size */
					(void)gmtlib_scanf_geodim (GMT, diameter, &p->size_y);
					p->size_x = p->size_y;
				}
				else {	/* Must read from data file */
					p->n_required = 1;	/* Only expect diameter */
					p->nondim_col[p->n_nondim++] = 2 + mode;	/* Since diameter is in geo-units, not inches or cm etc */
				}
			}
			else {
				p->n_required = 3;
				p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
				p->nondim_col[p->n_nondim++] = 3 + mode;	/* Since they are in geo-units, not inches or cm etc */
				p->nondim_col[p->n_nondim++] = 4 + mode;
			}
			check = false;
			break;
		case 'e':
			p->symbol = PSL_ELLIPSE;
			/* Expect angle in degrees, then major and major axes in plot units */
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			check = false;
			break;

		case 'f':	/* Fronts: -Sf<spacing>[/<size>][+r+l][+f+t+s+c+b][+o<offset>][+p<pen>]	[WAS: -Sf<spacing>/<size>[dir][type][:<offset>]	*/
			p->symbol = GMT_SYMBOL_FRONT;
			p->f.f_off = 0.0;	p->f.f_symbol = GMT_FRONT_FAULT;	p->f.f_sense = GMT_FRONT_CENTERED;
			p->f.f_angle = 20.0;			/* Default slip arrow angle */
			check = false;
			if (!text[1]) {	/* No args given, must parse segment header later */
				p->fq_parse = true;	/* This will be set to false once at least one header has been parsed */
				break;
			}
			strncpy (text_cp, text, GMT_LEN256-1);
			if (gmt_M_compat_check (GMT, 4)) {
				len = (int)strlen (text_cp) - 1;
				if (strchr (text_cp, ':') || (!strchr (text_cp, '+') && len > 0 && strchr ("bcflrst", text_cp[len]))) {	/* Old style */
					GMT_Report (GMT->parent, GMT_MSG_COMPAT,
					            "Warning in Option -Sf: Sf<spacing>/<size>[dir][type][:<offset>] is deprecated syntax\n");
					if ((c = strchr (text_cp, ':'))) {	/* Gave :<offset>, set it and strip it off */
						c++;	/* Skip over the colon */
						p->f.f_off = gmt_M_to_inch (GMT, c);
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
					p->f.f_gap = (txt_a[0] == '-') ? atof (txt_a) : gmt_M_to_inch (GMT, txt_a);
					p->f.f_len = gmt_M_to_inch (GMT, txt_b);
				}
				else
					gmtinit_parse_front (GMT, text_cp, p);	/* Parse new -Sf syntax */
			}
			else
				gmtinit_parse_front (GMT, text_cp, p);	/* Parse new -Sf syntax */
			if (p->f.f_sense == GMT_FRONT_CENTERED && p->f.f_symbol == GMT_FRONT_SLIP) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL,
				            "Error in Option -Sf: Must specify (l)eft-lateral or (r)ight-lateral slip\n");
				GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
			}
			if (gmt_M_is_zero (p->f.f_gap) || gmt_M_is_zero (p->f.f_len)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in Option -Sf: Neither <gap> nor <ticklength> can be zero!\n");
				GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
			}
			if (p->f.f_gap < 0.0) {	/* Gave -# of ticks desired */
				k = irint (fabs (p->f.f_gap));
				if (k == 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in Option -Sf: Number of front ticks cannot be zero!\n");
					GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
				}
				if (!gmt_M_is_zero (p->f.f_off)) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL,
					            "Error in Option -Sf: +<offset> cannot be used when number of ticks is specified!\n");
					GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
				}
			}
			p->fq_parse = false;	/* No need to parse more later */
			break;
		case 'G':
			p->factor = 1.05390736526;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 'g' */
		case 'g':
			p->symbol = PSL_OCTAGON;
			break;
		case 'H':
			p->factor = 1.09963611079;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 'h' */
		case 'h':
			p->symbol = PSL_HEXAGON;
			break;
		case 'I':
			p->factor = 1.55512030156;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 'i' */
		case 'i':
			p->symbol = PSL_INVTRIANGLE;
			break;
		case 'J':	/* Expect dimensions in km to be scaled based on -J */
			p->symbol = PSL_ROTRECT;
			p->convert_angles = 1;
			if (degenerate) {	/* Degenerate rectangle = square with zero angle */
				if (diameter[0]) {	/* Gave a fixed diameter as symbol size */
					(void)gmtlib_scanf_geodim (GMT, diameter, &p->size_y);
					p->size_x = p->size_y;
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
			p->symbol = PSL_ROTRECT;
			p->n_required = 3;
			p->nondim_col[p->n_nondim++] = 2 + mode;	/* Angle */
			check = false;
			break;
		case 'l':
			p->symbol = GMT_SYMBOL_TEXT;
			if (gmtinit_parse_text (GMT, text_cp, p)) decode_error++;
			break;
		case 'M':
		case 'm':
			p->symbol = PSL_MARC;
			p->n_required = 3;	/* Need radius, angle1 and angle2 */
			if (gmt_parse_vector (GMT, symbol_type, text, p)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S%c option\n", symbol_type);
				decode_error++;
			}
			if (symbol_type == 'M') p->v.status |= PSL_VEC_MARC90;	/* Flag means we will plot right angle symbol if angles extend 90 exactly */
			p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Angle */
			p->nondim_col[p->n_nondim++] = 4 + col_off;	/* Angle */
			break;
		case 'N':
			p->factor = 1.14948092619;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 'n' */
		case 'n':
			p->symbol = PSL_PENTAGON;
			break;
		case 'o':	/*3-D symbol */
			p->shade3D = true;
			/* Fall through on purpose to 'O' */
		case 'O':	/* Same but now enabling shading */
			p->symbol = GMT_SYMBOL_COLUMN;
			if (n_z) {
				p->n_required = n_z;	/* Need more than one z value from file */
				for (k = 0; k < n_z; k++) p->nondim_col[p->n_nondim++] = 2 + k;	/* all band z in user units */
			}
			if (bset) {
				if (text[bset+1] == '\0' || text[bset+1] == '+') {	/* Read it from data file */
					p->base_set = 2;
					p->n_required ++;
					p->nondim_col[p->n_nondim++] = 2 + col_off;	/* base in user units */
				}
				else {
					if (gmtinit_get_uservalue (GMT, &text[bset+1], gmt_M_type (GMT, GMT_IN, GMT_Z), &p->base, "-So|O base value")) return GMT_PARSE_ERROR;
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
			p->symbol = PSL_DOT;
			if (p->size_x == 0.0 && !p->read_size) {	/* User forgot to set size */
				p->size_x = GMT_DOT_SIZE;
				check = false;
			}
			break;
		case 'q':	/* Quoted lines: -Sq[d|n|l|s|x]<info>[:<labelinfo>] */
			p->symbol = GMT_SYMBOL_QUOTED_LINE;
			check = false;
			if (!text[1]) {	/* No args given, must parse segment header later */
				p->fq_parse = true;	/* This will be set to false once at least one header has been parsed */
				break;
			}
			for (j = 1, colon = 0; text[j]; j++) if (text[j] == ':') colon = j;
			if (colon) {	/* Gave :<labelinfo> */
				text[colon] = 0;
				gmt_contlabel_init (GMT, &p->G, 0);
				decode_error += gmt_contlabel_info (GMT, 'S', &text[1], &p->G);
				decode_error += gmt_contlabel_specs (GMT, &text[colon+1], &p->G);
				if (!cmd && gmt_contlabel_prep (GMT, &p->G, NULL)) decode_error++;
			}
			else
				decode_error += gmt_contlabel_info (GMT, 'S', &text[1], &p->G);
			p->fq_parse = false;	/* No need to parse more later */
			break;
		case 'r':
			p->symbol = PSL_RECT;
			p->n_required = 2;
			check = false;
			break;
		case 'R':
			p->symbol = PSL_RNDRECT;
			p->n_required = 3;
			check = false;
			break;
		case 'S':
			p->factor = 1.25331413732;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 's' */
		case 's':
			p->symbol = PSL_SQUARE;
			break;
		case 'T':
			p->factor = 1.55512030156;	/* To equal area of circle with same diameter */
			p->size_x *= p->factor;
			/* Fall through on purpose to 't' */
		case 't':
			p->symbol = PSL_TRIANGLE;
			break;
		case 'u':	/*3-D symbol */
			p->shade3D = true;
			/* Fall through on purpose to 'U' */
		case 'U':	/* Same but disable shading */
			p->symbol = GMT_SYMBOL_CUBE;
			if (mode == 0) {
				decode_error = true;
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S option: Symbol type %c is 3-D only\n", symbol_type);
			}
			break;
		case 'V':
			p->convert_angles = 1;
			/* Fall through on purpose to 'v' */
		case 'v':
			p->symbol = PSL_VECTOR;
			if (!gmt_M_compat_check (GMT, 4) || (strchr (text, '+') || !p->v.parsed_v4)) {	/* Check if new syntax before decoding */
				if (gmt_parse_vector (GMT, symbol_type, text, p)) {	/* Error decoding new vector syntax */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S%c option\n", symbol_type);
					decode_error++;
				}
				if (!(p->v.status & PSL_VEC_JUST_S)) p->nondim_col[p->n_nondim++] = 2 + col_off;
			}
			else {	/* Parse old-style vector specs */
				int one = 2;
				if (p->v.status & PSL_VEC_BEGIN) p->v.status -= PSL_VEC_BEGIN;	/* Remove previous setting */
				switch (text[1]) {	/* Check if s(egment), h(ead), b(alance center), or t(ail) have been specified */
					case 'S':	/* Input (x,y) refers to vector head (the tip), double heads */
						p->v.status |= PSL_VEC_BEGIN;
						/* Fall through on purpose to 's' */
					case 's':	/* Input (x,y) refers to vector head (the tip), head  at end */
						p->v.status |= (PSL_VEC_JUST_S + PSL_VEC_END);
						break;
					case 'H':	/* Input (x,y) refers to vector head (the tip), double heads */
						p->v.status |= PSL_VEC_BEGIN;
						/* Fall through on purpose to 'sh' */
					case 'h':	/* Input (x,y) refers to vector head (the tip), single head */
						p->v.status |= (PSL_VEC_JUST_E + PSL_VEC_END);
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
					case 'B':	/* Input (x,y) refers to balance point of vector, double heads */
						p->v.status |= PSL_VEC_BEGIN;
						/* Fall through on purpose to 'b' */
					case 'b':	/* Input (x,y) refers to balance point of vector, single head */
						p->v.status |= (PSL_VEC_JUST_C + PSL_VEC_END);
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
					case 'T':	/* Input (x,y) refers to tail of vector, double heads */
						p->v.status |= PSL_VEC_BEGIN;
						/* Fall through on purpose to 't' */
					case 't':	/* Input (x,y) refers to tail of vector [Default], single head */
						p->v.status |= (PSL_VEC_JUST_B + PSL_VEC_END);
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
					default:	/* No modifier given, default to tail, single head */
						p->v.status |= (PSL_VEC_JUST_B + PSL_VEC_END);
						one = 1;
						p->nondim_col[p->n_nondim++] = 2 + mode;
						break;
				}
				for (j = one; text[j] && text[j] != 'n'; j++);
				len = (int)strlen(text) - 1;
				if (text[j] == 'n') {	/* Normalize option used */
					k = gmtinit_get_unit (GMT, text[len]);
					p->v.v_norm = (float)atof (&text[j+1]);
					if (k >= GMT_CM)	/* Convert length from given units to inch */
						p->v.v_norm *= GMT->session.u2u[k][GMT_INCH];
					else	/* Convert from default units to inch */
						p->v.v_norm *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
					text[j] = 0;	/* Chop off the shrink part */
					/* Here, p->v.v_norm will be in inches */
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
						k = gmtinit_get_unit (GMT, text[len]);
						if (k >= 0) { p->u = k; p->u_set = true;}
						text[len] = 0;
					}
					if (!p->v.parsed_v4) {
						sscanf (&text[one], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
						p->v.v_width  = (float)gmt_M_to_inch (GMT, txt_a);
						p->v.h_length = (float)gmt_M_to_inch (GMT, txt_b);
						p->v.h_width  = (float)(gmt_M_to_inch (GMT, txt_c) * 2.0);
						p->v.v_angle = (float)(atand (0.5 * p->v.h_width / p->v.h_length) * 2.0);
					}
				}
				if (p->v.v_norm >= 0.0) text[j] = 'n';	/* Put back the n<shrink> part */
			}
			p->n_required = 2;
			break;
		case 'W':
			p->convert_angles = 1;
			/* Fall through on purpose to 'w' */
		case 'w':
			p->symbol = PSL_WEDGE;
			p->n_required = 2;
			p->nondim_col[p->n_nondim++] = 2 + col_off;	/* Start angle */
			p->nondim_col[p->n_nondim++] = 3 + col_off;	/* Stop angle */
			if (p->w_active) p->convert_angles = 0;	/* Expect azimuths directly */
			break;
		case '+':
			p->symbol =  PSL_PLUS;
			break;
		case 'x':
			p->symbol = PSL_CROSS;
			break;
		case 'y':
			p->symbol = PSL_YDASH;
			break;
		case 'z':
			p->symbol = GMT_SYMBOL_ZDASH;
			break;
		case 'K':
			if (cmd) p->read_symbol_cmd = 2;	/* Kustom symbol given as -SK implies we must read text data records */
			if (cmd) p->read_size_cmd = true;
			/* Fall through on purpose to 'k' */
		case 'k':
			p->custom = gmtlib_get_custom_symbol (GMT, text_cp);
			if (!p->custom) {
				GMT->init.n_custom_symbols = 0;
				decode_error++;
				break;
			}
			p->symbol = GMT_SYMBOL_CUSTOM;
			p->n_required = p->custom->n_required;
			for (ju = p->n_nondim = 0; ju < p->n_required; ju++) {	/* Flag input columns that are NOT lengths */
				if (p->custom->type[ju] != GMT_IS_DIMENSION) p->nondim_col[p->n_nondim++] = 2 + col_off + ju;
			}
			break;
		case '=':
			p->symbol = GMT_SYMBOL_GEOVECTOR;
			p->convert_angles = 1;
			p->n_required = 2;
			if (gmt_parse_vector (GMT, symbol_type, text, p)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S= option\n");
				decode_error++;
			}
			if (p->v.status & PSL_VEC_POLE) {	/* Small circle vector */
				if (p->v.status & PSL_VEC_ANGLES) {
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
		case '~':	/* Decorated lines: -S~[d|n|l|s|x]<info>[:<symbolinfo>] */
			p->symbol = GMT_SYMBOL_DECORATED_LINE;
			check = false;
			if (!text[1]) {	/* No args given, must parse segment header later */
				p->fq_parse = true;	/* This will be set to false once at least one header has been parsed */
				break;
			}
			for (j = 1, colon = 0; text[j]; j++) if (text[j] == ':') colon = j;
			if (colon) {	/* Gave :<symbolinfo> */
				text[colon] = 0;
				gmtlib_decorate_init (GMT, &p->D, 0);
				decode_error += gmtlib_decorate_info (GMT, 'S', &text[1], &p->D);
				decode_error += gmtlib_decorate_specs (GMT, &text[colon+1], &p->D);
				if (!cmd && gmt_decorate_prep (GMT, &p->D, NULL)) decode_error++;
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -S~ option: No symbol information given\n");
				decode_error++;
			}
			p->fq_parse = false;	/* No need to parse more later */
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
	if (add_to_base) p->base_set |= 4;	/* Means to add base value to height offset to get actual z at top */
	return (decode_error);
}

/*! . */
int gmt_init_scales (struct GMT_CTRL *GMT, unsigned int unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name) {
	/* unit is 0-8 (see gmt_project.h for enums) and stands for m, km, mile, nautical mile, inch, cm, point, foot, or (US) survey foot */
	/* fwd_scale is used to convert user distance units to meter */
	/* inv_scale is used to convert meters to user distance units */
	/* inch_to_unit is used to convert internal inches to users units (c, i, p) */
	/* unit_to_inch is used to convert users units (c, i, p) to internal inches */
	/* unit_name (unless NULL) is set to the name of the user's map measure unit (cm/inch/point) */

	if (unit >= GMT_N_UNITS) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT Unit id must be 0-%d\n", GMT_N_UNITS-1);
		GMT_exit (GMT, GMT_DIM_TOO_LARGE); return GMT_DIM_TOO_LARGE;
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
			if (gmt_M_compat_check (GMT, 4)) {
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
enum gmt_enum_units gmtlib_get_unit_number (struct GMT_CTRL *GMT, char unit) {
	enum gmt_enum_units mode;
	gmt_M_unused(GMT);

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
unsigned int gmt_check_scalingopt (struct GMT_CTRL *GMT, char option, char unit, char *unit_name) {
	int smode;
	unsigned int mode;

	if ((smode = gmtlib_get_unit_number (GMT, unit)) == GMT_IS_NOUNIT) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT ERROR Option -%c: Only append one of %s|%s\n",
		            option, GMT_DIM_UNITS_DISPLAY, GMT_LEN_UNITS2_DISPLAY);
		GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
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
int gmt_set_measure_unit (struct GMT_CTRL *GMT, char unit) {
	int k;

	if ((k = gmtinit_get_unit (GMT, unit)) < 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad plot measure selected (%c); use c, i, or p.\n", unit);
		return (GMT_MAP_BAD_MEASURE_UNIT);
	}
	GMT->current.setting.proj_length_unit = k;
	return (GMT_NOERROR);
}

/*! Use to parse various -S -Q options when backwardsness has been enabled */
int gmtinit_backwards_SQ_parsing (struct GMT_CTRL *GMT, char option, char *item) {
	int j;

	GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -%c[-]<mode>[/<threshold>] is deprecated. Use -n<mode>[+a][+t<threshold>] instead.\n", option);

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
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Interpolation threshold must be in [0,1] range\n");
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

unsigned int gmt_parse_inc_option (struct GMT_CTRL *GMT, char option, char *item) {
	/* Closest thing to parsing a global -I option we can get */
	if (gmt_getinc (GMT, item, GMT->common.R.inc)) {
		gmt_inc_syntax (GMT, option, 1);
		return 1U;
	}
	GMT->common.R.active[ISET] = true;
	return GMT_NOERROR;
}

#ifdef HAVE_GDAL
GMT_LOCAL int parse_proj4 (struct GMT_CTRL *GMT, char *item, char *dest) {
	/* Deal with proj.4 or EPSGs passed in -J option */
	char  *item_t1 = NULL, *item_t2 = NULL, wktext[32] = {""}, *pch;
	bool   do_free = false;
	int    error = 0, scale_pos;
	size_t k, len;
	double sc;

	if (item[0] == '+') {
		bool found = false;
		len = strlen (item);
		for (k = 1; k < len; k++) {			/* Search for glued tokens */
			if (item[k] == '+' && item[k-1] != ' ') {
				found = true;
				break;
			}
		}
		if (found) {			/* OK, need to break up things like +x=0+y=0 into "+x=0 +y=0" */
			item_t1 = gmt_strrep (item, "+", " +");
			do_free = true;		/* Signal that we must free item_t1 */
		}
		else
			item_t1 = item;

		sprintf(GMT->common.J.proj4string, "%s", item);		/* Copy the proj.4 string */
	}
	else if (!strncmp(item, "EPSG:", 5) || !strncmp(item, "epsg:", 5))
		item_t1 = &item[5];		/* Drop the EPSG: part because gmt_impotproj4 is not expecting it */
	else
		item_t1 = item;

	/* Don't remember anymore if we still need to call gmt_importproj4(). We don't for simple
	   mapproject usage but maybe we still need for mapping purposes. To-be-rediscovered.
	*/
	item_t2 = gmt_importproj4 (GMT, item_t1, &scale_pos);		/* This is GMT -J proj string */
	if (item_t2 && !GMT->current.ps.active && !strcmp(item_t2, "/1:1")) {
		/* Even though it failed to do the mapping we can still use it in mapproject */
		GMT->current.proj.projection_GMT = GMT_NO_PROJ;
		GMT->current.proj.is_proj4 = true;
		GMT->current.proj.pars[14] = 1;
	}
	else if (item_t2) {
		char *pch2;
		len = strlen(item_t2);
		if (item_t2[len-1] == 'W') {				/* See if scale is in fact a width */
			item_t2[0] = (char)toupper(item_t2[0]);		/* and let the GMT machinery detect this fact */
			item_t2[len-1] = '\0';
		}
		if (scale_pos != 0) {	/* Because if == 0 than it means we have a scale only string (i.e. no GMT mapped proj) */
			error += (gmt_M_check_condition (GMT, GMT->common.J.active, "Option -J given more than once\n") ||
			                                 gmtinit_parse_J_option (GMT, item_t2));
		}
		else		/* Not particularly useful yet because mapproject will fail anyway when scale != 1:1 */
			GMT->current.proj.projection_GMT = GMT_NO_PROJ;

		/* Check if the scale is 1 or 1:1, and don't get fooled with, for example, 1:10 */
		pch = &item_t2[scale_pos];
		if ((pch2 = strchr(pch, ':')) != NULL) {
			if ((sc = atof(&pch2[1])) == 1)
				GMT->current.proj.pars[14] = 1;
		}
		else if ((sc = atof(pch)) == 1)
			GMT->current.proj.pars[14] = 1;

		free (item_t2);			/* Cannot be freed before */
	}
	else
		return 1;

	if (isdigit(item[0]))
		sprintf (dest, "EPSG:%s", item);
	else if (strstr(item, "EPSG:") || strstr(item, "epsg:"))
		sprintf (dest, "%s", item);
	else
		sprintf (dest, "%s", item_t1);

	/* For the proj.4 string detect if this projection is supported by GDAL. If not will append a +wktext later */
	if (!strncmp(item, "+proj=", 6) && GDAL_VERSION_NUM < 2050000) {	/* Almost for sure GDAL 2.5 doesn't need this */
		char prjcode[8] = {""};
		k = 6;
		while (item[k] && (item[k] != '+' && item[k] != ' ' && item[k] != '\t' && item[k] != '/')) k++;
		strncpy(prjcode, &item[6], k - 6);

		/* List taken from https://github.com/OSGeo/gdal/blob/trunk/gdal/ogr/ogr_srs_proj4.cpp#L616  */
		if (strcmp(prjcode, "longlat") &&
			strcmp(prjcode, "latlong") &&
			strcmp(prjcode, "geocent") &&
			strcmp(prjcode, "bonne") &&
			strcmp(prjcode, "cass") &&
			strcmp(prjcode, "nzmg") &&
			strcmp(prjcode, "cea") &&
			strcmp(prjcode, "tmerc") &&
			strcmp(prjcode, "etmerc") &&
			strcmp(prjcode, "utm") &&
			strcmp(prjcode, "merc") &&
			strcmp(prjcode, "stere") &&
			strcmp(prjcode, "sterea") &&
			strcmp(prjcode, "eqc") &&
			strcmp(prjcode, "gstmerc") &&
			strcmp(prjcode, "gnom") &&
			strcmp(prjcode, "ortho") &&
			strcmp(prjcode, "laea") &&
			strcmp(prjcode, "aeqd") &&
			strcmp(prjcode, "eqdc") &&
			strcmp(prjcode, "mill") &&
			strcmp(prjcode, "moll") &&
			strcmp(prjcode, "eck1") &&
			strcmp(prjcode, "eck2") &&
			strcmp(prjcode, "eck3") &&
			strcmp(prjcode, "eck4") &&
			strcmp(prjcode, "eck5") &&
			strcmp(prjcode, "eck6") &&
			strcmp(prjcode, "poly") &&
			strcmp(prjcode, "aea") &&
			strcmp(prjcode, "robin") &&
			strcmp(prjcode, "vandg") &&
			strcmp(prjcode, "sinu") &&
			strcmp(prjcode, "gall") &&
			strcmp(prjcode, "goode") &&
			strcmp(prjcode, "igh") &&
			strcmp(prjcode, "geos") &&
			strcmp(prjcode, "lcc") &&
			strcmp(prjcode, "omerc") &&
			strcmp(prjcode, "somerc") &&
			strcmp(prjcode, "krovak") &&
			strcmp(prjcode, "iwm_p") &&
			strcmp(prjcode, "wag1") &&
			strcmp(prjcode, "wag2") &&
			strcmp(prjcode, "wag3") &&
			strcmp(prjcode, "wag4") &&
			strcmp(prjcode, "wag5") &&
			strcmp(prjcode, "wag6") &&
			strcmp(prjcode, "wag7") &&
			strcmp(prjcode, "qsc") &&
			strcmp(prjcode, "sch") &&
			strcmp(prjcode, "tpeqd")) {

			sprintf(wktext, " +wktext");	/* Projection NOT internally supported by GDAL */
			if (!strstr(item, "+ellps") && !strstr(item, "+a=") && !strstr(item, "+R="))
				strcat(dest, " +ellps=WGS84");
		}
	}

	if (do_free) free (item_t1);			/* When we got a glued +proj=... and had to insert spaces */

	if ((pch = strchr(dest, '/')) != NULL || (pch = strstr(dest, "+width=")) != NULL || (pch = strstr(dest, "+scale=")) != NULL)
		/* If we have a scale, drop it before passing the string to GDAL */
		pch[0] = '\0';

	if (wktext[0]) strcat(dest, wktext);	/* Append a +wktext to make this projection recognized by GDAL */

	return error;
}
#endif

/*! gmt_parse_common_options interprets the command line for the common, unique options
 * -B, -J, -K, -O, -P, -R, -U, -V, -X, -Y, -b, -c, -f, -g, -h, -i, -j, -n, -o, -p, -r, -s, -t, -:, -- and -^.
 * The list passes all of these that we should consider.
 * The API will also consider -I for grid increments.
 */
int gmt_parse_common_options (struct GMT_CTRL *GMT, char *list, char option, char *item) {

	int error = 0, i = 0;	/* The i and i+= GMT_more_than_once are there to avoid compiler warnings... */

	if (!list || !strchr (list, option)) return (0);	/* Not a common option we accept */

	if (gmt_M_compat_check (GMT, 4)) {
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
				case 's': GMT->common.B.active[GMT_SECONDARY] = true; break;
				default:  GMT->common.B.active[GMT_PRIMARY] = true; break;
			}
			if (!error) {
				if (GMT->current.setting.run_mode == GMT_MODERN) {
					if (item[0] == '\0')
						error = gmtlib_parse_B_option (GMT, "af");	/* Default -B setting if just -B is given since -B is not a shorthand under modern mode */
					else if (item[0] == 'x' && item[1] == '\0')
						error = gmtlib_parse_B_option (GMT, "xaf");	/* Default -B setting if just -B is given since -B is not a shorthand under modern mode */
					else if (item[0] == 'y' && item[1] == '\0')
						error = gmtlib_parse_B_option (GMT, "yaf");	/* Default -B setting if just -B is given since -B is not a shorthand under modern mode */
					else if (item[0] == 'z' && item[1] == '\0')
						error = gmtlib_parse_B_option (GMT, "zaf");	/* Default -B setting if just -B is given since -B is not a shorthand under modern mode */
					else
						error = gmtlib_parse_B_option (GMT, item);
				}
				else
					error = gmtlib_parse_B_option (GMT, item);
			}
			break;

		case 'I':
			if (GMT->hidden.func_level > GMT_CONTROLLER) return (0);	/* Just skip if we are inside a GMT module. -I is an API common option only */
			error = gmt_parse_inc_option (GMT, 'I', item);
			GMT->common.R.active[ISET] = true;
			break;

		case 'J':
			if (item && (item[0] == 'Z' || item[0] == 'z')) {	/* -JZ or -Jz */
				error += (gmt_M_check_condition (GMT, GMT->common.J.zactive, "Option -JZ|z given more than once\n") ||
				                                 gmtinit_parse_J_option (GMT, item));
				GMT->common.J.zactive = true;
			}
			else if (item && (item[0] == '+' || isdigit(item[0]) || !strncmp(item, "EPSG:", 5) || !strncmp(item, "epsg:", 5))) {
#ifdef HAVE_GDAL
				char   source[1024] = {""}, dest[1024] = {""}, *pch;
				bool two = false;

				/* When reading from gmt.history, those are prefixed with a '+' but we must remove it */
				if (!strncmp(item, "+EPSG", 5) || !strncmp(item, "+epsg", 5) || (item[0] == '+' && isdigit(item[1])))
					item++;

				/* Distinguish between the +to key extension and the +towgs84 key */
				if ((pch = strstr(item, "+to")) != NULL) {
					if (pch[3] == ' ' || pch[3] == '\t' || pch[3] == '+' || pch[3] == 'E' || pch[3] == 'e' || isdigit(pch[3]))
						two = true;
				}

				if (!two) {		/* A single CRS */
					sprintf(source, "+proj=latlong +datum=WGS84");
					error = parse_proj4 (GMT, item, dest);
				}
				else {
					pch[0] = '\0';
					error  = parse_proj4 (GMT, item, source);
					error += parse_proj4 (GMT, &pch[3], dest);
				}
				GMT->current.gdal_read_in.hCT_fwd = gmt_OGRCoordinateTransformation (GMT, source, dest);
				GMT->current.gdal_read_in.hCT_inv = gmt_OGRCoordinateTransformation (GMT, dest, source);
				GMT->current.proj.projection      = GMT_PROJ4_PROJS;		/* This now make it use the proj4 lib */
				GMT->common.J.active = true;
			}
#else
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "PROJ.4 can only be used with GDAL linked GMT.\n");
				error = 1;
			}
#endif
			else {	/* Horizontal map projection */
				error += (gmt_M_check_condition (GMT, GMT->common.J.active, "Option -J given more than once\n") ||
				                                 gmtinit_parse_J_option (GMT, item));
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
			if (GMT->current.setting.run_mode == GMT_CLASSIC) {
				i += GMT_more_than_once (GMT, GMT->common.P.active);
				GMT->common.P.active = true;
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option in GMT modern mode\n", option);
				return (1);
			}
			break;

		case 'Q':
		case 'S':
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -%c is deprecated. Use -n instead.\n" GMT_COMPAT_INFO, option);
				error += gmtinit_backwards_SQ_parsing (GMT, option, item);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
				return (1);
			}
			break;

		case 'R':
			error += (GMT_more_than_once (GMT, GMT->common.R.active[RSET]) || gmt_parse_R_option (GMT, item));
			GMT->common.R.active[RSET] = true;
			break;

		case 'U':
			error += (GMT_more_than_once (GMT, GMT->common.U.active) || gmtinit_parse_U_option (GMT, item));
			GMT->common.U.active = true;
			break;

		case 'V':
			i += GMT_more_than_once (GMT, GMT->common.V.active);
			GMT->common.V.active = true;
			if (item && item[0]) {	/* Specified a verbosity level */
				if (gmtinit_parse_V_option (GMT, item[0])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unknown argument to -V option, -V%c\n", item[0]);
					error++;
				}
			}
			else
				GMT->current.setting.verbose = GMT_MSG_VERBOSE;
			break;

		case 'X':
			error += (GMT_more_than_once (GMT, GMT->common.X.active) || gmtinit_parse_X_option (GMT, item));
			GMT->common.X.active = true;
			break;

		case 'Y':
			error += (GMT_more_than_once (GMT, GMT->common.Y.active) || gmtinit_parse_Y_option (GMT, item));
			GMT->common.Y.active = true;
			break;

		case 'Z':	/* GMT4 Backwards compatibility */
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -Z[<zlevel>] is deprecated. Use -p<azim>/<elev>[/<zlevel>] instead.\n"
				            GMT_COMPAT_INFO);
				if (item && item[0]) {
					if (gmtinit_get_uservalue (GMT, item, gmt_M_type (GMT, GMT_IN, GMT_Z), &GMT->current.proj.z_level, "-Z zlevel value"))
						return 1;
				}
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
				return (1);
			}
			break;

		case 'a':
			error += (GMT_more_than_once (GMT, GMT->common.a.active) || gmtinit_parse_a_option (GMT, item));
			GMT->common.a.active = true;
			break;

		case 'b':
			switch (item[0]) {
				case 'i':
					error += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN], "Warning Option -bi given more than once\n");
					GMT->common.b.active[GMT_IN] = true;
					break;
				case 'o':
					error += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_OUT], "Warning Option -bo given more than once\n");
					GMT->common.b.active[GMT_OUT] = true;
					break;
				default:
					error += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] + GMT->common.b.active[GMT_OUT],
					                                "Warning Option -b given more than once\n");
					GMT->common.b.active[GMT_IN] = GMT->common.b.active[GMT_OUT] = true;
					break;
			}
			error += gmtinit_parse_b_option (GMT, item);
			break;

		case 'd':
			switch (item[0]) {
				case 'i':
					error += gmt_M_check_condition (GMT, GMT->common.d.active[GMT_IN], "Warning Option -di given more than once\n");
					break;
				case 'o':
					error += gmt_M_check_condition (GMT, GMT->common.d.active[GMT_OUT], "Warning Option -do given more than once\n");
					break;
				default:
					error += gmt_M_check_condition (GMT, GMT->common.d.active[GMT_IN] + GMT->common.d.active[GMT_OUT],
					                                "Warning Option -d given more than once\n");
					break;
			}
			error += gmt_parse_d_option (GMT, item);
			break;

		case 'e':
			error += (GMT_more_than_once (GMT, GMT->common.e.active) || gmtinit_parse_e_option (GMT, item));
			GMT->common.e.active = true;
			break;

		case 'f':
			switch (item[0]) {
				case 'i':
#if 0
					error += gmt_M_check_condition (GMT, GMT->common.f.active[GMT_IN], "Warning Option -fi given more than once\n");
#endif
					GMT->common.f.active[GMT_IN] = true;
					break;
				case 'o':
#if 0
					error += gmt_M_check_condition (GMT, GMT->common.f.active[GMT_OUT], "Warning Option -fo given more than once\n");
#endif
					GMT->common.f.active[GMT_OUT] = true;
					break;
				default:
#if 0
					error += gmt_M_check_condition (GMT, GMT->common.f.active[GMT_IN] | GMT->common.f.active[GMT_OUT], "Warning Option -f given more than once\n");
#endif
					GMT->common.f.active[GMT_IN] = GMT->common.f.active[GMT_OUT] = true;
					break;
			}
			error += gmtinit_parse_f_option (GMT, item);
			break;

		case 'g':
			error += gmt_parse_g_option (GMT, item);
			GMT->common.g.active = true;
			break;

		case 'h':
			error += (GMT_more_than_once (GMT, GMT->common.h.active) || gmtinit_parse_h_option (GMT, item));
			GMT->common.h.active = true;
			break;

		case 'i':
			error += (GMT_more_than_once (GMT, GMT->common.i.active) || gmt_parse_i_option (GMT, item));
			GMT->common.i.active = true;
			break;

		case 'j':
			error += (GMT_more_than_once (GMT, GMT->common.j.active) || gmt_parse_j_option (GMT, item));
			GMT->common.j.active = true;
			break;

		case 'l':
			error += (GMT_more_than_once (GMT, GMT->common.l.active) || gmt_parse_l_option (GMT, item));
			GMT->common.l.active = true;
			break;

		case 'M':	/* Backwards compatibility */
		case 'm':
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -%c is deprecated. Segment headers are automatically identified.\n", option);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
				return (1);
			}
			break;

		case 'n':
			error += (GMT_more_than_once (GMT, GMT->common.n.active) || gmtinit_parse_n_option (GMT, item));
			GMT->common.n.active = true;
			break;

		case 'o':
			error += (GMT_more_than_once (GMT, GMT->common.o.active) || gmt_parse_o_option (GMT, item));
			GMT->common.o.active = true;
			break;

		case 'p':
			error += (GMT_more_than_once (GMT, GMT->common.p.active) || gmtinit_parse_p_option (GMT, item));
			GMT->common.p.active = true;
			break;

		case 'r':
			error += GMT_more_than_once (GMT, GMT->common.R.active[GSET]);
			GMT->common.R.active[GSET] = true;
			if (item[0]) {	/* Gave argument for specific registration */
				switch (item[0]) {
					case 'p': GMT->common.R.registration = GMT_GRID_PIXEL_REG; break;
					case 'g': GMT->common.R.registration = GMT_GRID_NODE_REG; break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -r: Syntax is -r[g|p]\n");
						error++;
						break;
				}
			}
			else	/* By default, -r means pixel registration */
				GMT->common.R.registration = GMT_GRID_PIXEL_REG;
			break;

		case 's':
			error += (GMT_more_than_once (GMT, GMT->common.s.active) || gmtinit_parse_s_option (GMT, item));
			GMT->common.s.active = true;
			break;

		case 't':
			error += GMT_more_than_once (GMT, GMT->common.t.active);
			if (item[0]) {
				GMT->common.t.value = atof (item);
				if (GMT->common.t.value < 0.0 || GMT->common.t.value > 100.0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -t: Transparency must be in (0-100]%% range!\n");
					GMT->common.t.value = 0.0;
					error++;
				}
				GMT->common.t.active = true;
			}
			else if (!strncmp (GMT->init.module_name, "psxy", 4U) || !strncmp (GMT->init.module_name, "pstext", 6U)) {	/* Modules psxy, psxyz, and pstext can do variable transparency */
				GMT->common.t.active = GMT->common.t.variable = true;
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -t was not given any value (please add transparency in (0-100]0%% range)!\n");
				error++;
			}
			break;

#ifdef GMT_MP_ENABLED
		case 'x':
			error += (GMT_more_than_once (GMT, GMT->common.x.active) || gmtinit_parse_x_option (GMT, item));
			GMT->common.x.active = true;
			break;
#endif

		case ':':
			error += (GMT_more_than_once (GMT, GMT->common.colon.active) || gmtinit_parse_colon_option (GMT, item));
			GMT->common.colon.active = true;
			break;

		case '^':
			if (GMT->common.synopsis.active) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option - given more than once\n");
			GMT->common.synopsis.active = true;
			break;

		case '-':
			error += gmtinit_parse_dash_option (GMT, item);
			break;

		case '>':	/* Registered output file; nothing to do here */
			break;

		default:	/* Here we end up if an unrecognized option is passed (should not happen, though) */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Option -%c is not a recognized common option\n", option);
			return (1);
			break;
	}

	/* On error, give syntax message */

	if (error) {
		gmt_syntax (GMT, option);
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Offending option -%c%s\n", option, item);
	}

	return (error);
}

/*! . */
int gmt_init_time_system_structure (struct GMT_CTRL *GMT, struct GMT_TIME_SYSTEM *time_system) {
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
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Unit c (seconds) is deprecated; use s instead.\n");
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
	if (gmtinit_scanf_epoch (GMT, time_system->epoch, &time_system->rata_die, &time_system->epoch_t0)) error += 2;

	if (error & 1) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "TIME_UNIT is invalid.  Default assumed.\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Choose one only from y o d h m s\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Corresponding to year month day hour minute second\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Note year and month are simply defined (365.2425 days and 1/12 of a year)\n");
	}
	if (error & 2) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "TIME_EPOCH format is invalid.  Default assumed.\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "    A correct format has the form [-]yyyy-mm-ddThh:mm:ss[.xxx]\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "    or (using ISO weekly calendar)   yyyy-Www-dThh:mm:ss[.xxx]\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "    An example of a correct format is:  2000-01-01T12:00:00\n");
	}
	return (error);
}

int gmtlib_get_option_id (int start, char *this_option) {
	/* Search the GMT_unique_option list starting at given position for this_option */
	int k, id = -1;
	for (k = start; k < GMT_N_UNIQUE && id == -1; k++)
		if (!strcmp (GMT_unique_option[k], this_option)) id = k;	/* Got entry index into history array for requested option */
	return (id);
}

/*! Discover if a certain option was set in the history and re-set it */
int gmt_set_missing_options (struct GMT_CTRL *GMT, char *options) {
	/* When a module discovers it needs -R or -J and it maybe was not given
	 * see if we can tease out the answer from the history and parse it.
	 * Currently used in gmt_get_refpoint where we may learn that -R -J will
	 * indeed be required.  We then check if they have been given.  If not,
	 * then under classic mode we abort, while under modern mode we add them,
	 * if possible.
	 */
	int id = 0, j, err = 0;
	char str[3] = {""};

	if (GMT->current.setting.run_mode == GMT_CLASSIC) return GMT_NOERROR;	/* Do nothing */
	if (GMT->current.ps.active && GMT->current.ps.initialize) return GMT_NOERROR;	/* Cannot use history unless overlay */

	assert (options);	/* Should never be NULL */

	for (j = 0; options[j]; j++) {	/* Do this for all required options listed */
		assert (strchr ("JR", options[j]));	/* Only J and/or R should be present in options */
		if (options[j] == 'R' && GMT->common.R.active[RSET]) continue;	/* Set already */
		if (options[j] == 'J' && GMT->common.J.active) continue;	/* Set already */
		/* Must dig around in the history array */
		gmt_M_memset (str, 3, char);
		str[0] = options[j];
		if ((id = gmtlib_get_option_id (0, str)) == -1) continue;	/* Not an option we have history for yet */
		if (options[j] == 'R' && !GMT->current.ps.active) id++;		/* Examine -RG history if not a plotter */
		if (GMT->init.history[id] == NULL) continue;	/* No history for this option */
		if (options[j] == 'J') {	/* Must now search for actual option since -J only has the code (e.g., -JM) */
			/* Continue looking for -J<code> */
			str[1] = GMT->init.history[id][0];
			if ((id = gmtlib_get_option_id (id + 1, str)) == -1) continue;	/* Not an option we have history for yet */
			if (GMT->init.history[id] == NULL) continue;	/* No history for this option */
		}
		/* Here we should have a parsable command option */
		err += gmt_parse_common_options (GMT, str, options[j], GMT->init.history[id]);
	}
	return ((err) ? GMT_PARSE_ERROR : GMT_NOERROR);
}

unsigned int gmt_add_R_if_modern_and_true (struct GMT_CTRL *GMT, const char *needs, bool do_it) {
	if (strchr (needs, 'r') == NULL) return GMT_NOERROR;	/* -R is not a conditional option */
	if (do_it)
		return (gmt_set_missing_options (GMT, "R"));
	return GMT_NOERROR;
}

/*! Changes the 4 GMT default pad values to given isotropic pad */
void gmt_set_pad (struct GMT_CTRL *GMT, unsigned int pad) {
	GMT->current.io.pad[XLO] = GMT->current.io.pad[XHI] = GMT->current.io.pad[YLO] = GMT->current.io.pad[YHI] = pad;
}

/*! . */
struct GMT_CTRL *gmt_begin (struct GMTAPI_CTRL *API, const char *session, unsigned int pad) {
	/* gmt_begin is called once by GMT_Create_Session and does basic
	 * one-time initialization of GMT before the GMT modules take over.
	 * It will load in the gmt.conf settings from the share dir and
	 * reset them with the user's gmt.conf settings (if any).
	 * It then does final processing of defaults so that all internal
	 * GMT parameters are properly initialized and ready to go. This
	 * means it is possible to write a functioning GMT application that
	 * does not require the use of any GMT modules.  However,
	 * most GMT applications will call various GMT modules and these
	 * may need to process additional --PAR=value arguments. This will
	 * require renewed processing of defaults and takes place in gmt_init_module
	 * which is called at the start of all GMT modules.  This basically
	 * performs a save/restore operation so that when the GMT module
	 * returns the GMT structure is restored to its original values.
	 *
	 * Note: We do not call GMT_exit here since API is not given and
	 * API->do_not_exit have not been modified by external API yet.
	 */

	struct GMT_CTRL *GMT = NULL;
	char version[GMT_LEN8] = {""};
#if defined(HAVE_GDAL) && (GDAL_VERSION_MAJOR >= 3)
	char *path1 = NULL, *path2 = NULL, *paths[2] = {NULL, NULL};
	int  local_count = 0;
#endif

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

	GMT = gmtinit_new_GMT_ctrl (API, session, pad);	/* Allocate and initialize a new common control structure */
	API->GMT = GMT;

#if defined(HAVE_GDAL) && (GDAL_VERSION_MAJOR >= 3)
	/* Here we allow users to declare a different location for the GDAL's GDAL_DATA and
	   PROJ4 PROJ_LIB directories. The later may be particularly useful in these days of
	   transition to GDAL3.0, which imply PROJ4 V6 but many (different) PROJ_LIB from PROJ4 < 6
	   are still trailing around.
	   If neither of those ENV var is provided, we default to GMT share/GDAL_DATA, if that
	   dir exists. In case it does, then it's expected to contain both the files from GDAL's
	   GDAL_DATA (data) and PROJ4 PRPJ_LIB (share). This will be the case on Windows where
	   the installer will create and fill that directory.
	*/
	if ((path1 = getenv ("LOCAL_GDAL_DATA")) != NULL) paths[local_count++] = path1;
	if ((path2 = getenv ("LOCAL_PROJ_LIB")) != NULL)  paths[local_count++] = path2;
	if (!local_count) {			/* If none of the above was provided, default to share/GDAL_DATA */
		char dir[GMT_LEN256];
		sprintf (dir, "%s/GDAL_DATA", API->GMT->session.SHAREDIR);
		if (access (dir, F_OK) == 0) {		/* ... if it exists */
			paths[0] = strdup(dir);
			local_count = -1;
		}
	}
	if (local_count) {		/* Means we have a request to use custom GDAL/PROJ4 data dirs */
		OSRSetPROJSearchPaths(paths);
		if (local_count < 0)  free (paths[0]);		/* This case was strdup allocated, so it can be freed */
	}
#endif

	sprintf (version, "GMT%d", GMT_MAJOR_VERSION);
	GMT_Report (API, GMT_MSG_DEBUG, "Enter: New_PSL_Ctrl\n");
	GMT->PSL = New_PSL_Ctrl (version);		/* Allocate a PSL control structure */
	GMT_Report (API, GMT_MSG_DEBUG, "Exit:  New_PSL_Ctrl\n");
	if (!GMT->PSL) {
		GMT_Message (API, GMT_TIME_NONE, "Error: Could not initialize PSL - Aborting.\n");
		gmtinit_free_GMT_ctrl (GMT);	/* Deallocate control structure */
		return NULL;
	}

	GMT_Report (API, GMT_MSG_DEBUG, "Enter: gmt_manage_workflow\n");
	if (gmt_manage_workflow (API, GMT_USE_WORKFLOW, NULL)) {
		GMT_Message (API, GMT_TIME_NONE, "Error: Could not initialize the GMT workflow - Aborting.\n");
		gmtinit_free_GMT_ctrl (GMT);	/* Deallocate control structure */
		return NULL;
	}
	GMT_Report (API, GMT_MSG_DEBUG, "Exit : gmt_manage_workflow\n");

	GMT->PSL->init.unit = PSL_INCH;					/* We use inches internally in PSL */
	GMT_Report (API, GMT_MSG_DEBUG, "Enter: PSL_beginsession\n");
	PSL_beginsession (GMT->PSL, API->external, GMT->session.SHAREDIR, GMT->session.USERDIR);	/* Initializes the session and sets a few defaults */
	GMT_Report (API, GMT_MSG_DEBUG, "Exit : PSL_beginsession\n");
	/* Reset session defaults to the chosen GMT settings; these are fixed for the entire PSL session */
	GMT_Report (API, GMT_MSG_DEBUG, "Enter: PSL_setdefaults\n");
	PSL_setdefaults (GMT->PSL, GMT->current.setting.ps_magnify, GMT->current.setting.ps_page_rgb, GMT->current.setting.ps_encoding.name);
	GMT_Report (API, GMT_MSG_DEBUG, "Exit : PSL_setdefaults\n");

	GMT_Report (API, GMT_MSG_DEBUG, "Enter: gmtlib_io_init\n");
	gmtlib_io_init (GMT);		/* Init the table i/o structure before parsing GMT defaults */
	GMT_Report (API, GMT_MSG_DEBUG, "Exit : gmtlib_io_init\n");

	gmtinit_init_unit_conversion (GMT);	/* Set conversion factors from various units to meters */

	gmt_hash_init (GMT, keys_hashnode, GMT_keywords, GMT_N_KEYS, GMT_N_KEYS);	/* Initialize hash table for GMT defaults */

	/* Set up hash table for colornames (used to convert <colorname> to <r/g/b>) */

	gmt_hash_init (GMT, GMT->session.rgb_hashnode, gmt_M_color_name, GMT_N_COLOR_NAMES, GMT_N_COLOR_NAMES);

	gmtinit_conf (GMT);	/* Initialize the standard GMT system default settings */

	GMT_Report (API, GMT_MSG_DEBUG, "Enter: gmt_getdefaults\n");
	gmt_getdefaults (GMT, NULL);	/* Override using local GMT default settings (if any) [and PSL if selected] */
	GMT_Report (API, GMT_MSG_DEBUG, "Exit:  gmt_getdefaults\n");

	if (API->runmode) GMT->current.setting.run_mode = GMT_MODERN;	/* Enforced at API Creation */

	/* There is no longer a -m option in GMT so multi segments are now always true.
	   However, in GMT_COMPAT mode the -mi and -mo options WILL turn off multi in the other direction. */
	gmt_set_segmentheader (GMT, GMT_IN, true);
	gmt_set_segmentheader (GMT, GMT_OUT, false);	/* Will be turned true when either of two situation arises: */
	/* 1. We read a multisegment header
	   2. The -g option is set which will create gaps and thus multiple segments
	 */

	/* Initialize the output and plot format machinery for ddd:mm:ss[.xxx] strings from the default format strings.
	 * While this is also done in the default parameter loop it is possible that when a decimal plain format has been selected
	 * the format_float_out string has not yet been processed.  We clear that up by processing again here. */

	GMT_Report (API, GMT_MSG_DEBUG, "Enter: gmtlib_plot_C_format\n");
	gmtlib_geo_C_format (GMT);
	gmtlib_plot_C_format (GMT);
	GMT_Report (API, GMT_MSG_DEBUG, "Exit:  gmtlib_plot_C_format\n");

	/* Set default for -n parameters */
	GMT->common.n.antialias = true; GMT->common.n.interpolant = BCR_BICUBIC; GMT->common.n.threshold = 0.5;

	gmtinit_get_history (GMT);	/* Process and store command shorthands passed to the application */

	if (GMT->current.setting.io_gridfile_shorthand) gmtinit_setshorthand (GMT);	/* Load the short hand mechanism from gmt.io */

	gmt_fft_initialization (GMT);	/* Determine which FFT algos are available and set pointers */

	gmtinit_set_today (GMT);	/* Determine today's rata die value */

	return (GMT);
}

unsigned int gmtlib_get_pos_of_filename (const char *url) {
	/* Takes an URL and finds start of the filename. If given just a filename it returns 0 */
	size_t pos = strlen (url);
	assert (pos > 0);
	pos--;	/* Last character in name */
	while (url[pos] && pos > 0 && url[pos] != '/') pos--;	/* Wind to first slash */
	if (url[pos] == '/') pos++;	/* First letter after last slash */
	if (url[pos] == '@') pos++;	/* Step over leading @ for cache files */
	return (unsigned int)pos;
}

GMT_LOCAL bool check_if_we_must_download (struct GMT_CTRL *GMT, const char *file, unsigned int kind) {
	/* Returns false if file already present */
	unsigned int pos = gmtlib_get_pos_of_filename (file);	/* Find start of filename */
	if (kind == GMT_URL_QUERY)
		return true;	/* A query can never exist locally */
	else if (kind == GMT_REGULAR_FILE)
		return false;	/* A query must exist locally */
	else if (kind == GMT_DATA_FILE) {	/* Special remote data set @earth_relief_xxm|s grid request */
		bool found;
		if (strstr (file, ".grd")) {	/* User already gave the extension */
			found = (!gmt_access (GMT, &file[1], F_OK));    /* Not found so need to download */
		}
		else {	/* Must append the extension .grd */
			char *tmpfile = malloc (strlen(file)+5);
			sprintf (tmpfile, "%s.grd", &file[1]);
			found = (!gmt_access (GMT, tmpfile, F_OK));	/* Not found so need to download */
			gmt_M_str_free (tmpfile);
		}
		return !found;
	}
	else if (!gmt_access (GMT, &file[pos], F_OK))
		return false;	/* File exists already so no need to download */
	return true;
}

bool gmtlib_file_is_downloadable (struct GMT_CTRL *GMT, const char *file, unsigned int *kind) {
	/* Returns true if file is a known GMT-distributable file and download is enabled */
	/* Return immediately if no auto-download is disabled or file is found locally */
#ifdef DEBUG
	static char *fkind[5] = {"Regular File", "Cache File", "Data File", "URL File", "URL Query"};
#endif
	*kind = GMT_REGULAR_FILE;	/* Default is a regular file */
	if (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) return false;	/* Not enabled */
	if (file == NULL) return false;	/* Return immediately if file is NULL */
	if (gmt_M_file_is_remotedata (file))	/* Special remote data set @earth_relief_xxm|s grid request */
		*kind = GMT_DATA_FILE;
	else if (gmt_M_file_is_cache (file))	/* Special @<filename> syntax for GMT cache files */
		*kind = GMT_CACHE_FILE;
	else if (gmt_M_file_is_url(file)) {	/* Full URL given */
		if (strchr (file, '?') == NULL)
			*kind = GMT_URL_FILE;
		else
			*kind = GMT_URL_QUERY;	/* These we will never check for access and must rerun each time */
	}
#ifdef DEBUG
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s: Type is %s\n", file, fkind[*kind]);
#endif
	return (check_if_we_must_download (GMT, file, *kind));
}

/*! . */
bool gmt_check_filearg (struct GMT_CTRL *GMT, char option, char *file, unsigned int direction, unsigned int family) {
	/* Return true if a file arg was given and, if direction is GMT_IN, check that the file
	 * exists and is readable. Otherwise we return false. */
	unsigned int pos = 0, kind = 0;
	bool not_url = false;
	char message[GMT_LEN16] = {""};
	if (option == GMT_OPT_INFILE)
		sprintf (message, "for input file");
	else if (option == GMT_OPT_OUTFILE)
		sprintf (message, "for output file");
	else
		snprintf (message, GMT_LEN16, "option -%c", option);

	if (!file || file[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error %s: No filename provided\n", message);
		return false;	/* No file given */
	}
	if (direction == GMT_OUT) return true;		/* Cannot check any further */
	if (file[0] == '=') pos = 1;	/* Gave a list of files with =<filelist> mechanism in x2sys */
	not_url = !gmtlib_file_is_downloadable (GMT, file, &kind);	/* not_url may become false if this could potentially be obtained from GMT ftp site */
	if (kind == GMT_CACHE_FILE || kind == GMT_DATA_FILE) pos = 1;	/* Has leading '@' in name so must skip that letter when checking if it exists locally */
	else if (kind == GMT_URL_FILE) pos = gmtlib_get_pos_of_filename (file);	/* Find start of filename */
	if (!not_url && kind == 0 && (family == GMT_IS_GRID || family == GMT_IS_IMAGE))	/* Only grid and images can be URLs so far */
		not_url = !gmtlib_check_url_name (&file[pos]);
	if (not_url) {
		if (gmt_access (GMT, &file[pos], F_OK)) {	/* Cannot find the file anywhere GMT looks */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error %s: No such file (%s)\n", message, &file[pos]);
			return false;	/* Could not find this file */
		}
		if (gmt_access (GMT, &file[pos], R_OK)) {	/* Cannot read this file (permissions?) */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error %s: Cannot read file (%s) - check permissions\n", message, &file[pos]);
			return false;	/* Could not find this file */
		}
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
void gmt_setmode (struct GMT_CTRL *GMT, int direction) {

	FILE *fp = NULL;
	static const char *IO_direction[2] = {"Input", "Output"};

	if (GMT->common.b.active[direction]) {	/* User wants native binary i/o */

		fp = (direction == 0) ? GMT->session.std[GMT_IN] : GMT->session.std[GMT_OUT];
		fflush (fp);	/* Should be untouched but anyway... */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Set binary mode for %s\n", IO_direction[direction]);
		setmode (fileno (fp), _O_BINARY);
	}
}

#endif	/* SET_IO_MODE */

/*! . */
int gmt_message (struct GMT_CTRL *GMT, char *format, ...) {
	char line[GMT_BUFSIZ];
	va_list args;
	va_start (args, format);
	vsnprintf (line, GMT_BUFSIZ, format, args);
	GMT->parent->print_func (GMT->session.std[GMT_ERR], line);
	va_end (args);
	return (0);
}

/*! . */
int gmtlib_report_func (struct GMT_CTRL *GMT, unsigned int level, const char *source_line, const char *format, ...) {
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

int gmt_remove_dir (struct GMTAPI_CTRL *API, char *dir, bool recreate) {
	/* Delete all files in a directory, then the directory itself.
	 * It is assumed that we created the directory so that there is only
	 * one level, i.e., there are no sub-directories inside the directory. */
	unsigned int n_files, k;
	int error = GMT_NOERROR;
	char **filelist = NULL, *here = NULL;
	struct GMT_CTRL *GMT = API->GMT;	/* Shorthand */
	if (access (dir, F_OK)) {
		GMT_Report (API, GMT_MSG_NORMAL, "No directory named %s\n", dir);
		return GMT_FILE_NOT_FOUND;
	}
	if ((here = getcwd (NULL, 0)) == NULL) {	/* Get the current directory */
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot determine current directory!\n");
		return GMT_RUNTIME_ERROR;
	}
	if (chdir (dir)) {
		perror (dir);
		return GMT_RUNTIME_ERROR;
	}
	if ((n_files = (unsigned int)gmtlib_glob_list (GMT, "*", &filelist))) {
		for (k = 0; k < n_files; k++) {
			if (gmt_remove_file (GMT, filelist[k]))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove %s [permissions?]\n", filelist[k]);
		}
		gmtlib_free_list (GMT, filelist, n_files);	/* Free the file list */
	}
	if (chdir (here)) {		/* Get back to where we were */
		perror (here);
		gmt_M_str_free (here);
		return GMT_RUNTIME_ERROR;
	}
	gmt_M_str_free (here);
	if (rmdir (dir)) {	/* Unable to delete the directory */
		perror (dir);
		error = GMT_RUNTIME_ERROR;
	}
	else if (recreate) {	/* Create an empty directory to replace what we deleted */
		if (gmt_mkdir (dir)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to recreate directory : %s\n", dir);
			error = GMT_RUNTIME_ERROR;
		}
	}
	return error;
}

GMT_LOCAL FILE *open_figure_file (struct GMTAPI_CTRL *API, unsigned int mode, int *err) {
	char line[PATH_MAX] = {""}, *opt[2] = {"r", "a"};
	FILE *fp = NULL;
	sprintf (line, "%s/gmt.figures", API->gwf_dir);	/* Path to gmt.figures */
	*err = 0;	/* No error yet */
	if (mode == 0 && access (line, F_OK)) {
		GMT_Report (API, GMT_MSG_DEBUG, "No figure file %s - nothing to do\n", line);
		return NULL;
	}
	/* Here the file gmt.figures exists and we must read in the entries */
	if ((fp = fopen (line, opt[mode])) == NULL) {
		if (mode == 1) {	/* Failure to open existing or create new gmt.figures file */
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to open/create %s with mode %s\n", line, opt[mode]);
			*err = 1;	/* No error yet */
		}
	}
	return fp;
}

int gmt_get_graphics_id (struct GMT_CTRL *GMT, const char *format) {
	int code = 0;
	gmt_M_unused(GMT);
	while (gmt_session_format[code] && strcmp (format, gmt_session_format[code]))
		code++;
	return (gmt_session_format[code]) ? code : -1;
}

/*! . */
GMT_LOCAL void get_session_name_format (struct GMTAPI_CTRL *API, char prefix[GMT_LEN256], char formats[GMT_LEN256]) {
	/* Read the session name [and graphics format] from file GMT_SESSION_FILE */
	int n;
	char file[PATH_MAX] = {""};
	FILE *fp = NULL;
	sprintf (file, "%s/%s", API->gwf_dir, GMT_SESSION_FILE);
	if (access (file, F_OK)) {	/* Use default session name and format */
		strcpy (prefix, GMT_SESSION_NAME);
		strcpy (formats, gmt_session_format[GMT_SESSION_FORMAT]);
		return;
	}
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to open session file %s\n", file);
		return;
	}
	if ((n = fscanf (fp, "%s %s\n", prefix, formats)) < 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to read from session file %s\n", file);
		fclose (fp);
		return;
	}
	if (n == 1)	/* Assign default format */
		strcpy (formats, gmt_session_format[GMT_SESSION_FORMAT]);
	GMT_Report (API, GMT_MSG_DEBUG, "Got session name as %s and default graphics formats as %s\n", prefix, formats);
	fclose (fp);
}

/*! . */
int gmtlib_read_figures (struct GMT_CTRL *GMT, unsigned int mode, struct GMT_FIGURE **figs) {
	/* Load or count any figures stored in the queue file gmt.figures.
	 * mode = 0:	Count how many figures in the queue.
	 * mode = 1:	Also return array of GMT_FIGURE structs.
	 * mode = 2:	As 1 but insert session figure in the list.
	 * If there are errors encountered then report them and return -1.
	 * Else we return how many found and the pointer figs. */
	char line[PATH_MAX] = {""};
	unsigned int n_alloc = GMT_TINY_CHUNK, k = 0;
	int err, n;
	struct GMT_FIGURE *fig = NULL;
	FILE *fp = NULL;

	if ((fp = open_figure_file (GMT->parent, 0, &err)) == NULL && mode < 2) {	/* No such file, nothing to do */
		return 0;
	}
	/* Here the file gmt.figures may exists and we must read in the entries */
	if (mode > 0) fig = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_FIGURE);
	if (mode == 2) {	/* Insert default session figure */
		get_session_name_format (GMT->parent, fig[0].prefix, fig[0].formats);
		fig[0].ID = 0;	k = 1;
	}
	while (fp && fgets (line, PATH_MAX, fp)) {
		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (mode >= 1) {
			gmt_chop (line);
			if ((n = sscanf (line, "%d %s %s %s", &fig[k].ID, fig[k].prefix, fig[k].formats, fig[k].options)) < 3) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to read from figure file\n");
				fclose (fp);
				gmt_M_free (GMT, fig);
				return 0;
			}
			if (n == 3) fig[k].options[0] = '\0';
			if (++k >= n_alloc) {
				n_alloc += GMT_TINY_CHUNK;
				fig = gmt_M_memory (GMT, fig, n_alloc, struct GMT_FIGURE);
			}
		}
		else	/* Just count them */
			k++;
	}
	if (fp) fclose (fp);
	if (mode > 0) {
		if (k == 0)	/* Blank file, nothing to report */
			gmt_M_free (GMT, fig);
		else if (k < n_alloc)
			fig = gmt_M_memory (GMT, fig, k, struct GMT_FIGURE);
		*figs = fig;	/* Pass out what we found */
	}
	return (k);
}

bool gmtlib_fig_is_ps (struct GMT_CTRL *GMT) {
	int n_figs;
	unsigned int pos = 0;
	bool PS = false;
	char p[GMT_LEN64] = {""};
	struct GMT_FIGURE *fig = NULL;

	if ((n_figs = gmtlib_read_figures (GMT, 2, &fig)) == GMT_NOTSET) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to determine number of figures\n");
		return false;
	}
	n_figs--;	/* Id of current figure */
	while (gmt_strtok (fig[n_figs].formats, ",", &pos, p)) {	/* Check each format to make sure each is OK */
		if (!strcmp (p, "ps")) PS = true;
	}
	if (!PS && strchr (fig[n_figs].options, 'P')) PS = true;	/* Down want square paper size when P is given explicitly */
	gmt_M_free (GMT, fig);
	return (PS);
}

GMT_LOCAL int put_session_name (struct GMTAPI_CTRL *API, char *arg) {
	/* Write the session name to file GMT_SESSION_FILE unless default */
	FILE *fp = NULL;
	char file[PATH_MAX] = {""};
	if (arg == NULL) return GMT_NOERROR;	/* Nothing to do, which means we accept the defaults */
	GMT_Report (API, GMT_MSG_DEBUG, "Set session name to be %s\n", arg);
	sprintf (file, "%s/%s", API->gwf_dir, GMT_SESSION_FILE);
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to create session file %s\n", file);
		return GMT_ERROR_ON_FOPEN;
	}
	fprintf (fp, "%s\n", arg);
	fclose (fp);
	return GMT_NOERROR;
}

GMT_LOCAL int process_figures (struct GMTAPI_CTRL *API) {
	char cmd[GMT_BUFSIZ] = {""}, fmt[GMT_LEN16] = {""}, option[GMT_LEN256] = {""}, p[GMT_LEN256] = {""}, mark;
	struct GMT_FIGURE *fig = NULL;
	bool not_PS;
	int error, k, f, nf, n_figs;
	unsigned int pos = 0;

 	if (API->gwf_dir == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "No workflow directory set\n");
		return GMT_NOT_A_VALID_DIRECTORY;
	}
	if ((n_figs = gmtlib_read_figures (API->GMT, 2, &fig)) == GMT_NOTSET) {	/* Auto-insert the hidden gmt_0.ps- file which may not have been used */
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open gmt.figures for reading\n");
		return GMT_ERROR_ON_FOPEN;
	}

	if (n_figs) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Process GMT figure queue: %d figures found\n", n_figs);

	for (k = 0; k < n_figs; k++) {
		if (!strcmp (fig[k].prefix, "-")) continue;	/* Unnamed outputs are left for manual psconvert calls by external APIs */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing GMT figure #%d [%s %s %s]\n", fig[k].ID, fig[k].prefix, fig[k].formats, fig[k].options);
		f = nf = 0;
		while (gmt_session_format[f]) {	/* Go through the list and build array for -T arguments */
			if (strstr (fig[k].formats, gmt_session_format[f])) fmt[nf++] = gmt_session_code[f];
			f++;
		}
		for (f = 0; f < nf; f++) {	/* Loop over all desired output formats */
			mark = '-';	/* This is the last char in extension for a half-baked GMT PostScript file */
			sprintf (cmd, "%s/gmt_%d.ps%c", API->gwf_dir, fig[k].ID, mark);	/* Check if the file exists */
			if (access (cmd, F_OK)) {	/* No such file, check if the fully baked file is there instead */
				mark = '+';	/* This is the last char in extension for a fully-baked GMT PostScript file */
				sprintf (cmd, "%s/gmt_%d.ps%c", API->gwf_dir, fig[k].ID, mark);	/* Check if the file exists */
				if (access (cmd, F_OK)) {	/* No such file ether, give up; warn if a fig set via gmt figure (k > 0) */
					if (k) GMT_Report (API, GMT_MSG_VERBOSE, "Figure # %d was registered but no matching PostScript-|+ file found - skipping\n", fig[k].ID, cmd);
					continue;
				}
			}
			/* Here the file exists and we can call psconvert. Note we still pass *.ps- even if *.ps+ was found since psconvert will do the same check */
			sprintf (cmd, "%s/gmt_%d.ps- -T%c -F%s", API->gwf_dir, fig[k].ID, fmt[f], fig[k].prefix);
			not_PS = (fmt[f] != 'p');	/* Do not add convert options if plain PS */
			/* Append psconvert optional settings */
			if (fig[k].options[0]) {	/* Append figure-specific settings */
				pos = 0;	/* Reset position counter */
				while ((gmt_strtok (fig[k].options, ",", &pos, p))) {
					if (not_PS || p[0] == 'M') {	/* Only -M is allowed if PS is the formst */
						sprintf (option, " -%s", p);	/* Create proper ps_convert option syntax */
						strcat (cmd, option);
					}
				}
			}
			else if (API->GMT->current.setting.ps_convert[0]) {	/* Supply chosen session settings for psconvert */
				pos = 0;	/* Reset position counter */
				while ((gmt_strtok (API->GMT->current.setting.ps_convert, ",", &pos, p))) {
					if (not_PS || p[0] == 'M') {	/* Only -M is allowed if PS is the formst */
						sprintf (option, " -%s", p);	/* Create proper ps_convert option syntax */
						strcat (cmd, option);
					}
				}
			}
			GMT_Report (API, GMT_MSG_DEBUG, "psconvert: %s\n", cmd);
			if ((error = GMT_Call_Module (API, "psconvert", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Failed to call psconvert\n");
				gmt_M_free (API->GMT, fig);
				return error;
			}
		}
	}
	gmt_M_free (API->GMT, fig);

	return GMT_NOERROR;
}

int gmtlib_set_current_figure (struct GMTAPI_CTRL *API, char *prefix, int this_k) {
	/* Write the current figure number and prefix to the gmt.current file */
	FILE *fp = NULL;
	char file[PATH_MAX] = {""};
	if (API->gwf_dir == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_set_current_figure: No workflow directory set\n");
		return GMT_NOT_A_VALID_DIRECTORY;
	}
	sprintf (file, "%s/gmt.current", API->gwf_dir);
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_set_current_figure: Could not create file %s\n", file);
		return GMT_ERROR_ON_FOPEN;
	}
	fprintf (fp, "%d\t%s\n", this_k, prefix);
	fclose (fp);
	return GMT_NOERROR;
}

int gmt_get_current_figure (struct GMTAPI_CTRL *API) {
	/* Get the current figure number and prefix from the gmt.current file */
	FILE *fp = NULL;
	int fig_no = 0;
	char file[PATH_MAX] = {""};
	if (API->gwf_dir == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmt_get_current_figure: No workflow directory set\n");
		return GMT_NOT_A_VALID_DIRECTORY;
	}
	sprintf (file, "%s/gmt.current", API->gwf_dir);
	/* See if there is a gmt.current file to read */
	if (access (file, R_OK))	/* No gmt.current file available so return 0 */
		return fig_no;
	/* Dealing with many figures so get the current one */
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmt_get_current_figure: Could not open file %s\n", file);
		return GMT_ERROR_ON_FOPEN;
	}
	if (fscanf (fp, "%d", &fig_no) != 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmt_get_current_figure: Could not read fig number from file %s\n", file);
		fclose (fp);
		return GMT_DATA_READ_ERROR;
	}
	fclose (fp);
	return (fig_no);
}

GMT_LOCAL bool is_integer (char *L) {
	/* Return true if string L is not an integer or is empty */
	if (!L || L[0] == '\0') return false;
	for (size_t k = 0; k < strlen (L); k++) {
		if (!isdigit (L[k])) return false;	/* Got a bad boy */
	}
	return true;	/* Everything came up roses */
}

int gmt_add_figure (struct GMTAPI_CTRL *API, char *arg) {
	/* Add another figure to the gmt.figure queue.
	 * arg = "[prefix] [format] [options]"
	 * Rules: No prefix may start with a hyphen
	 *		  prefix is required on the command line
	 *		  format defaults to pdf on command line
	 * The hidden gmt_###.ps- file starts numbering at 1 to
	 * indicate it was set via gmt figure.  Scripts that are
	 * not using gmt figure just builds gmt_0.ps-.
	 * Since gmt_add_figure is called by figure.c and arguments
	 * have been vetted we don't need to check much here except
	 * when gmt figure is called to select an earlier plot and
	 * make it the current figure.
	 */
	int n = 0, n_figs, this_k = 0, k, err;
	bool found = false;
	char prefix[GMT_LEN256] = {""}, formats[GMT_LEN64] = {""}, options[GMT_LEN128] = {""};
	char *L = NULL;
	struct GMT_FIGURE *fig = NULL;
	FILE *fp = NULL;

	if (API->gwf_dir == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmt figure: No workflow directory set\n");
		return GMT_NOT_A_VALID_DIRECTORY;
	}
	if (API->external && arg == NULL) {	/* For external calls, if no args we supply - - */
		/* This means the external API will call psconvert directly */
		prefix[0] = formats[0] = '-';
	}
	else {	/* For regular command line use, the figure prefix is required; the rest is optional */
		if (arg == NULL || arg[0] == '\0') {	/* This is clearly not allowed */
			GMT_Report (API, GMT_MSG_NORMAL, "gmt figure: No argument given\n");
			return GMT_ARG_IS_NULL;
		}
		/* Separate prefix, format, and convert arguments.  We know n >= 1 here */

		n = sscanf (arg, "%s %s %s", prefix, formats, options);
	}
	/* See what we have registered so far */
	n_figs = gmtlib_read_figures (API->GMT, 1, &fig);	/* Number and names of figures so far [0] */
	/* Did we already register this figure? */
	for (k = 0; !found && k < n_figs; k++) {
		if (!strcmp (prefix, fig[k].prefix)) {
			found = true;
			this_k = k + 1;	/* Since 0 is the hidden file used when figure is not set */
		}
	}
	gmt_M_free (API->GMT, fig);
	if (found && !(formats[0] == '-' || n == 1)) {
		/* Want to make an already registered figure the current figure */
		GMT_Report (API, GMT_MSG_NORMAL, "gmt figure: Cannot select an existing figure and change its format.\n");
		return GMT_RUNTIME_ERROR;
	}
	else if (!found) {	/* Here we have a new valid entry */
		this_k = n_figs + 1;	/* This is the ID number of the new figure we are adding */
		if (n == 1) /* Only got a prefix so must set the default pdf format */
			strncpy (formats, gmt_session_format[GMT_SESSION_FORMAT], GMT_LEN64-1);
		if ((fp = open_figure_file (API, 1, &err)) == NULL)	/* Failure to open existing or create new file */
			return GMT_ERROR_ON_FOPEN;
		/* Append the new entry */
		fprintf (fp, "%d\t%s\t%s", this_k, prefix, formats);
		if (options[0])	{	/* Append the options string */
			GMT_Report (API, GMT_MSG_DEBUG, "New figure: %d\t%s\t%s\t%s\n", this_k, prefix, formats, options);
			fprintf (fp, "\t%s", options);
		}
		else
			GMT_Report (API, GMT_MSG_DEBUG, "New figure: %d\t%s\t%s", this_k, prefix, formats);

		fprintf (fp, "\n");
		fclose (fp);
	}
	/* Set the current figure */
	if (gmtlib_set_current_figure (API, prefix, this_k))
		return GMT_ERROR_ON_FOPEN;

	/* See if movie set up a set of frame labels */

	if ((L = getenv ("MOVIE_N_LABELS")) != NULL) {	/* MOVIE_N_LABELS was set */
		unsigned int T, n_tags;
		char file[PATH_MAX] = {""}, name[GMT_LEN32] = {""};
		if (!is_integer (L)) {
			GMT_Report (API, GMT_MSG_NORMAL, "MOVIE_N_LABELS = %s but must be an integer\n", L);
			return GMT_RUNTIME_ERROR;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "New figure: Found special MOVIE_N_LABELS = %s\n", L);
		sprintf (file, "%s/gmt.movie", API->gwf_dir);
		if ((fp = fopen (file, "w")) == NULL) {	/* Not good */
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot create file %s\n", file);
			return GMT_ERROR_ON_FOPEN;
		}
		fprintf (fp, "# movie label information file\n");
		n_tags = atoi (L);
		for (T = 0; T < n_tags; T++) {
			sprintf (name, "MOVIE_LABEL_ARG%d", T);
			if ((L = getenv (name)) != NULL) {	/* MOVIE_LABEL_ARG# was set */
				/* Special processing for gmt movie: If -L is used to set labeling then we
			 	* need to ensure the label is on top of the frame and unaffected by any
			 	* -X -Y that may have been used in the main script. We do this by implementing
			 	* the label via a PSL procedure that is called at the end of the file.
			 	* This is set up by gmt_plotinit in gmt_plot.c */
				fprintf (fp, "%s\n", L);
			}
		}
		fclose (fp);
	}

	return GMT_NOERROR;
}

int gmt_truncate_file (struct GMTAPI_CTRL *API, char *file, size_t size) {
	/* Trim back the file to what it was when it was young and half-baked */
	if (!file || file[0] == '\0' || size == 0) return GMT_NOERROR;
#ifdef WIN32
	{
		FILE *fp = NULL;

		if ((fp = fopen (file, "a")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", file);
			return GMT_FILE_NOT_FOUND;
		}
		if (_chsize (fileno (fp), (long)size)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to truncate file %s (via _chsize) back to %" PRIuS " bytes\n", file, size);
			fclose (fp);
			return errno;
		}
		fclose (fp);
	}
#else
	if (truncate (file, size)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to truncate file %s (via truncate) back to %" PRIuS " bytes\n", file, size);
		return errno;
	}
#endif
	return GMT_NOERROR;
}

int gmt_legend_file (struct GMTAPI_CTRL *API, char *file) {
	/* Under modern mode, determines the name for the legend file for the current plot,
	 * then returns 0 if file does not already exist and 1 if it does.
	 * Returns -1 if under classic mode. */
	unsigned int fig;
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) return GMT_NOTSET;	/* This is a modern mode only feature */
	/* Get figure number */
	fig = gmt_get_current_figure (API);
	sprintf (file, "%s/gmt_%d.legend", API->gwf_dir, fig);
	if (access (file, R_OK)) return 0;	/* No such file yet */
	return 1;	/* Found it */
}

int gmt_add_legend_item (struct GMTAPI_CTRL *API, char *symbol, char *size, struct GMT_FILL *fill, struct GMT_PEN *pen, char *label) {
	/* Add another entry to the legends file for automatic legend building */
	char file[PATH_MAX] = {""};
	FILE *fp = NULL;

	if (gmt_legend_file (API, file) == GMT_NOTSET) return GMT_NOERROR;	/* This is a modern mode only feature */
	if ((fp = fopen (file, "r")) == NULL) {	/* Does not exist yet, create */
		if ((fp = fopen (file, "w")) == NULL) {	/* Could not create, WTF */
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to create file %s\n", file);
			return GMT_ERROR_ON_FOPEN;
		}
		fprintf (fp, "# Automatically created entries for input to legend\n");
		fprintf (fp, "# S <dx1> <symbol> <size> <fill> <pen> <dx2> <label>\n");
	}
	/* Add the new entry */
	fprintf (fp, "S - %s %s ", symbol, size);
	(fill) ? fprintf (fp, "%s ", gmtlib_putfill (API->GMT, fill)) : fprintf (fp, "- ");
	(pen)  ? fprintf (fp, "%s ", gmt_putpen (API->GMT, pen))      : fprintf (fp, "- ");
	fprintf (fp, "- %s\n", label);
	fclose (fp);
	
	return GMT_NOERROR;
}

/*! . */
int gmt_manage_workflow (struct GMTAPI_CTRL *API, unsigned int mode, char *text) {
	/* Manage the GMT workflow.  Mode can take the following values:
	 *   GMT_BEGIN_WORKFLOW: Start a new GMT workflow and initialize a work flow directory.
	 *   GMT_USE_WORKFLOW:	Continue using the current work flow directory
	 *   GMT_END_WORKFLOW:	Finalize the workflow.
	 */

	/* Set workflow directory */
	char file[PATH_MAX] = {""}, dir[PATH_MAX] = {""};
	static char *type[2] = {"classic", "modern"}, *smode[3] = {"Use", "Begin", "End"}, *fstatus[4] = {"found", "not found", "created", "removed"};
	int err = 0, fig, error = GMT_NOERROR;
	struct stat S;

	sprintf (dir, "%s/gmt%d.%s", API->session_dir, GMT_MAJOR_VERSION, API->session_name);
	API->gwf_dir = strdup (dir);
	err = stat (API->gwf_dir, &S);	/* Stat the gwf_dir path (which may not exist) */

	switch (mode) {
		case GMT_BEGIN_WORKFLOW:	/* Must create a new temporary directory */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Creating a workflow directory %s\n", API->gwf_dir);
			/* We only get here when gmt begin is called */
			if (err == 0 && !S_ISDIR (S.st_mode)) {	/* Path already exists, but it is not a directory */
				GMT_Report (API, GMT_MSG_NORMAL, "A file named %s already exist and prevents us creating a workflow directory by that name\n", API->gwf_dir);
				error = GMT_RUNTIME_ERROR;
			}
			else if (err == 0 && S_ISDIR (S.st_mode) && (S.st_mode & S_IWUSR) == 0) {	/* Directory already exists and is not writeable */
				GMT_Report (API, GMT_MSG_NORMAL, "Workflow directory %s already exist but is not writeable\n", API->gwf_dir);
				error = GMT_RUNTIME_ERROR;
			}
			else {	/* Create the new workflow directory */
				/* To avoid the weird CID 167015 that says:
				toctou: Calling function mkdir that uses API->gwf_dir after a check function.
				This can cause a time-of-check, time-of-use race condition.
				we will use "dir" instead of "API->gwf_dir" below  */
				if (err == 0 && S.st_mode & S_IWUSR) {	/* Remove abandoned directory */
					GMT_Report (API, GMT_MSG_DEBUG, "Workflow directory %s already exist and is writeable (we will delete are recreate)\n", API->gwf_dir);
					error = gmt_remove_dir (API, dir, true);	/* Remove and recreate */
				}
				else {	/* Can create a fresh new directory */
					if (gmt_mkdir (dir)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a workflow directory : %s\n", API->gwf_dir);
						error = GMT_RUNTIME_ERROR;
					}
				}
			}
			if (error) return (error);			/* Bail at this point */
			gmtinit_conf (API->GMT);			/* Get the original system defaults */
			gmt_getdefaults (API->GMT, NULL);		/* Overload user defaults */
			sprintf (dir, "%s/gmt.conf", API->gwf_dir);	/* Reuse dir string for saving gmt.conf to this dir */
			API->GMT->current.setting.run_mode = GMT_MODERN;	/* Enable modern mode here so putdefaults can skip writing PS_MEDIA if not PostScript output */
			gmt_putdefaults (API->GMT, dir);		/* Write current GMT defaults to this sessions gmt.conf file in the workflow directory */
			error = put_session_name (API, text);		/* Store session name */
			API->GMT->current.setting.history_orig = API->GMT->current.setting.history;	/* Temporarily turn off history so nothing is copied into the workflow dir */
			API->GMT->current.setting.history = GMT_HISTORY_OFF;	/* Turn off so that no history is copied into the workflow directory */
			GMT_Report (API, GMT_MSG_DEBUG, "%s Workflow.  Session Name = %s. Directory %s %s.\n", smode[mode], API->session_name, API->gwf_dir, fstatus[2]);
			break;
		case GMT_USE_WORKFLOW:
			/* We always get here except when gmt begin | end are called. */
			/* If the workflow directory exists then we are in modern mode, else in classic */
			API->GMT->current.setting.run_mode = (err == 0) ? GMT_MODERN : GMT_CLASSIC;
			break;
		case GMT_END_WORKFLOW:
			/* We only get here when gmt end is called */
			/* Check if a subplot was left hanging */
			fig = gmt_get_current_figure (API);	/* Get current figure number */
			sprintf (file, "%s/gmt.subplot.%d", API->gwf_dir, fig);
			if (!access (file, R_OK))	/* subplot end was never called */
				GMT_Report (API, GMT_MSG_NORMAL, "subplot was never completed - plot items in last panel may be missing\n");
			GMT_Report (API, GMT_MSG_DEBUG, "%s Workflow.  Session Name = %s. Directory %s %s.\n", smode[mode], API->session_name, API->gwf_dir, fstatus[3]);
			if ((error = process_figures (API)))
				GMT_Report (API, GMT_MSG_NORMAL, "process_figures returned error %d\n", error);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Destroying the current workflow directory %s\n", API->gwf_dir);
			if (gmt_remove_dir (API, dir, false))
				GMT_Report (API, GMT_MSG_VERBOSE, "Unable to remove directory %s [permissions?]\n", dir);
			API->GMT->current.setting.run_mode = GMT_CLASSIC;	/* Disable modern mode */
			break;
		default:
			GMT_Report (API, GMT_MSG_NORMAL, "Illegal mode (%d) passed to gmt_manage_workflow\n", mode);
			break;
	}
	if (API->GMT->current.setting.run_mode == GMT_MODERN) {
		GMT_Report (API, GMT_MSG_DEBUG, "GMT now running in %s mode [Session Name = %s]\n", type[API->GMT->current.setting.run_mode], API->session_name);
		API->GMT->current.setting.use_modern_name = true;
	}
	return error;
}

#if 0	/* Maybe use later - things seems to work OK for now with what we have */

/*! Codes from gmt file types */
enum GMT_enum_ftypes {
	GMT_IS_URL = 1,
	GMT_IS_QUERY = 2,
	GMT_IS_CDF_ATTR = 4,
	GMT_IS_GRD_ATTR = 8,
	GMT_IS_GDAL_ATTR = 16,
	GMT_IS_CACHE = 32,
	GMT_IS_DATA = 64};

unsigned int gmt_file_type (struct GMT_CTRL *GMT, const char *file, unsigned int pos[]) {
	/* Determine what kind of file argument was given.  Return a code
	 * that reflects a sum of several bit flags:
	 * 1:	Gave an URL, 0 otherwise
	 * 2:	Gave an URL that is a query, 0 otherwise
	 * 4:	Specified a netCDF slice/layer attribute, 0 otherwise
	 * 8:	Specified a GMT grid format =<id>[<modifiers>] attribute, 0 otherwise
	 * 16:	GDAL file called via =gd[...]
	 * 32:	Special @earth_relief grid to be downloaded if not found
	 */
	unsigned int code = 0;
	char *a = NULL, *c = NULL, *e = NULL, *p = NULL, *q = NULL, *s = NULL;

	pos[0] = pos[1] = 0;	/* Initialize */
	if (file == NULL) return 0;	/* Nada */
	a = strchr  (file, '&');	/* Address of the first ? or NULL */
	e = strchr  (file, '=');	/* Address of the first = or NULL */
	p = strrchr (file, '+');	/* Address of the last + or NULL */
	q = strchr  (file, '?');	/* Address of the first ? or NULL */
	s = strrchr (file, '/');	/* Address of the last / or NULL */
	/* First distinguish between cache, urls, and special data files from the GMT server */
	if (gmt_M_file_is_cache (file)) {	/* Special @<filename> syntax for GMT cache files */
		code |= GMT_CACHE_FILE;
		pos[0] = 1;	/* Must skip the first character to find the file name */
	}
	else if (gmt_M_file_is_url (file))	/* Full URL given */
		code |= GMT_IS_URL;
	else if (!strncmp (file, GMT_DATA_PREFIX, strlen(GMT_DATA_PREFIX)) && strstr (file, ".grd"))	/* Useful data set distributed by GMT */
		code |= GMT_IS_DATA;

	/* Now try to detect subtleries like netcdf slices and grid attributes */

	if ((c = strstr (file, "=gd"))) {
		code |= GMT_IS_GDAL_ATTR;	/* Only GDAL references would have these characters */
		pos[1] = (unsigned int) (c - file);
	}
	else if ((code & GMT_IS_URL) && q && (a || (e && e > s))) {	/* Only URL quieries have ampersands in them */
		code |= GMT_IS_QUERY;
		pos[1] = (unsigned int) (q - file);
	}
	else if (q) {	/* Question-mark is more complicated */
		if (strchr (file, '[') || strchr (file, '('))	/* Assume netCDF slicing: file.nc?pressure[2,1] or file.nc?pressure(24,10) */
			code |= GMT_IS_CDF_ATTR;
		else if (s && s > q)	/* Assume netCDF variable selections: file.nc?time/lat/lon */
			code |= GMT_IS_CDF_ATTR;
		else if (e && (s == NULL || s > e))	/* Must be a GMT grid with old-style attributes: junk.grd=bf/0/1/32767 */
			code |= GMT_IS_GRD_ATTR;
		else if (e && p && p > e)	/* Must be a GMT grid with new-style attributes: junk.grd=bf+s<scale>+o<scale>+n<nan> */
			code |= GMT_IS_GRD_ATTR;
		else 	/* Assume netCDF variable selection: file.nc?slp */
			code |= GMT_IS_CDF_ATTR;
		pos[1] = (unsigned int) (q - file);
	}
	else if (e)	{
		pos[1] = (unsigned int) (e - file);
		if (s == NULL || s > e)	/* Must be a GMT grid with old-style attributes: junk.grd=bf/0/1/32767 */
			code |= GMT_IS_GRD_ATTR;
		else if (p && p > e)	/* Must be a GMT grid with new-style attributes: junk.grd=bf+s<scale>+o<scale>+n<nan> */
			code |= GMT_IS_GRD_ATTR;
		else if (strlen (e) == 3)	/* Must be a GMT grid with format only: junk.grd=bf */
			code |= GMT_IS_GRD_ATTR;
		else	/* Weird file with = in the name? */
			pos[1] = 0;
	}
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "gmt_file_type: File %s returning code = %u, with pos[0] = %u and pos[1] = %u\n", file, code, pos[0], pos[1]);
	return code;
}
#endif

void gmt_auto_offsets_for_colorbar (struct GMT_CTRL *GMT, double offset[], int justify) {
	/* We wish to examine the previous -B setting for information as to which axes was
	 * annotated and possibly labeled, and if the colorbar is requested to be placed on
	 * such an axis side we need to make more space by increasing the offset. This is
	 * only possible under modern mode since classic updated -B in the history. */

	char side, axis, B_delim[2] = {30, 0}, p[GMT_BUFSIZ] = {""};	/* Use ASCII 30 RS Record Separator between -B strings */
	char file[PATH_MAX] = {""};
	unsigned int pos = 0;
	bool add_label = false, add_annot = false, axis_set = false;
	double GMT_LETTER_HEIGHT = 0.736;
	FILE *fp = NULL;
	/* Initialize the default settings before considering any -B history */
	offset[GMT_OUT] = GMT->current.setting.map_label_offset + GMT->current.setting.map_frame_width;
	offset[GMT_IN]  = GMT->current.setting.map_label_offset;

	if (GMT->current.setting.run_mode == GMT_CLASSIC) return;	/* No can do */

	switch (justify) {	/* Only the four sides are automated */
		case PSL_TC: side = 'N'; axis = 'x'; break;
		case PSL_BC: side = 'S'; axis = 'x'; break;
		case PSL_ML: side = 'W'; axis = 'y'; break;
		case PSL_MR: side = 'E'; axis = 'y'; break;
		default: return; break;	/* No auto-adjust for the rest */
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Determined colorbar side = %c and axis = %c\n", side, axis);

	sprintf (file, "%s/gmt%d.%s/gmt.frame", GMT->parent->session_dir, GMT_MAJOR_VERSION, GMT->parent->session_name);
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "No file %s with frame information - no adjustments made\n", file);
		return;
	}
	fgets (file, PATH_MAX, fp);	fclose (fp);	/* Recycle file to hold the -B arguments */
	while (file[0] && gmt_strtok (file, B_delim, &pos, p)) {	/* Parse the -B options from last call */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "B item = %s\n", p);
		if (p[0] == axis && strstr (p, "+l")) add_label = true;	/* User specified a axis label on that side */
		if (strchr ("WESNwesn", p[0])) {	/* Gave a -B<axis> option */
			axis_set = true;
			if (strchr (p, side)) add_annot = true;
		}
	}
	/* If -BWE.. was not set we must rely on MAP_FRAME_AXES default setting */
	if (!axis_set && strchr (GMT->current.setting.map_frame_axes, side)) add_annot = true;
	if (add_label && gmt_M_is_geographic (GMT, GMT_IN)) add_label = false;	/* Not allowed anyway */
	/* Time to make updates, if any */
	if (add_annot) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Adding annotation space\n");
		offset[GMT_OUT] += MAX(0,GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER]);	/* Any tick length */
		offset[GMT_OUT] += (GMT_LETTER_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between axis and annotations */
	}
	if (add_label) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Adding label space\n");
		offset[GMT_OUT] += (GMT_LETTER_HEIGHT * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_label_offset);
	}
}

bool gmt_option_set (struct GMT_CTRL *GMT, bool *active, unsigned int *errors) {
	/* General function used to check if an option has already been processed once.
	 * We use this to prevent user errors such as "-Q45.7 -O -P -Q33" where -Q is given twice. */
	gmt_M_unused (GMT);
	if (*active) {		/* Already set, so return error and increase errors count */
		(*errors)++;
		return true;
	}
	else {	/* First time here, set to true and return false */
		*active = true;
		return false;
	}
}

unsigned int gmtlib_count_char (struct GMT_CTRL *GMT, char *txt, char it) {
	unsigned int i, n;
	gmt_M_unused (GMT);
	for (i = n = 0; txt[i]; i++) if (txt[i] == it) n++;
	return (n);
}

/*! Return the number of CPU cores */
int gmtlib_get_num_processors() {
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

int gmt_report_usage (struct GMTAPI_CTRL *API, struct GMT_OPTION *options, unsigned int special, int (*usage)(struct GMTAPI_CTRL *, int)) {
	/* Handle the way classic and modern mode modules report their usage messages.
	 * Set special == 1 if the classic module can be run with no options at all and still be expected to do things (e.g., silently read stdin) */
	int code = GMT_NOERROR;	/* Default is no usage message was requested and we move on to parsing the arguments */
	if (API->GMT->current.setting.run_mode == GMT_MODERN) {	/* Under modern mode we always require an option like -? or -^ to call usage */
		if (options) {	/* Modern mode will only print the usage if one of the usage-options are given (but see exception for one-liners) */
			if (options->option == GMT_OPT_USAGE)	/* Return the usage message */
				code = GMT_OPT_USAGE;
			if (options->option == GMT_OPT_SYNOPSIS)	/* Return the synopsis message */
				code = GMT_SYNOPSIS;
		}
		else if (API->usage)	/* One-liner modern mode with no args must give usage */
			code = GMT_OPT_USAGE;
	}
	else if (special) {	/* Some classic modules require an option to show usage, otherwise expect input (e.g., gmtinfo) */
		if (options && options->option == GMT_OPT_USAGE)
			code = GMT_OPT_USAGE;
		if (options && options->option == GMT_OPT_SYNOPSIS)
			code = GMT_SYNOPSIS;
	}
	else {	/* Regular classic module behavior */
		if (!options || options->option == GMT_OPT_USAGE)
			code = GMT_OPT_USAGE;
		else if (options->option == GMT_OPT_SYNOPSIS)
			code = GMT_SYNOPSIS;
	}
	
	if (code)	/* Must call the usage function */
		usage (API, code);
	return (code);	
}
