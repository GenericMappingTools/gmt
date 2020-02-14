/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2009-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *--------------------------------------------------------------------*/
/* 			PSL: PostScript Light
 *
 * PSL is a library of plot functions that create PostScript.
 * All the routines write their output to the same plotting file,
 * which can be dumped to a Postscript output device (laserwriters).
 * PSL can handle and mix text, line-drawings, and bit-map graphics
 * in both black/white and color.  Colors are specified with r,g,b
 * values in the range 0-1.
 *
 * PSL conforms to the Encapsulated PostScript Files Specification V 3.0.
 *
 * C considerations:
 *	Include postscriptlight.h in your program.
 *	All floating point data are assumed to be of type double.
 *	All integer data are assumed to be of type long.
 *
 * FORTRAN considerations:
 *	All floating point data are assumed to be DOUBLE PRECISION
 *	All integer data are assumed to be a long INTEGER, i.e. INTEGER*8
 *
 *	When passing (from FORTRAN to C) a fixed-length character variable which has
 *	blanks at the end, append '\0' (null character) after the last non-blank
 *	character.  This is so that C will know where the character string ends.
 *	It is NOT sufficient to pass, for example, "string(1:string_length)".
 *
 * List of API functions:
 *
 * PSL_beginaxes
 * PSL_beginclipping	   : Clips plot outside the specified polygon
 * PSL_beginlayer	   : Place begin object group DSC comment.
 * PSL_beginplot	   : Initialize parameters for a new plot.
 * PSL_beginsession	   : Creates a new PSL session
 * PSL_copy		   : Writes the given string as is to the PS output
 * PSL_endaxes		   : Turns off mapping of user coordinates to PS units
 * PSL_endclipping	   : Restores previous clipping path
 * PSL_endlayer		   : Place end object group DSC comment.
 * PSL_endplot		   : Close plotfile
 * PSL_endsession	   : Terminates the PSL session
 * PSL_getplot		   : Return string with entire PS code
 * PSL_plotarc		   : Plots a circular arc
 * PSL_plotaxis		   : Plots an axis with tickmarks and annotation/label
 * PSL_plotbitimage	   : Plots a 1-bit image or imagemask
 * PSL_plotcolorimage	   : Plots a 24-bit 2-D image using the colorimage operator
 * PSL_plotepsimage	   : Inserts EPS image
 * PSL_plotline		   : Plots a line
 * PSL_plotparagraph	   : Plots a text paragraph
 * PSL_plotparagraphbox	   : Plots a box beneath a text paragraph
 * PSL_plotpoint	   : Absolute or relative move to new position (pen up or down)
 * PSL_plotpolygon	   : Creates a polygon and optionally fills it
 * PSL_plotsegment	   : Plots a 2-point straight line segment
 * PSL_plotsymbol	   : Plots a geometric symbol and [optionally] fills it
 * PSL_plottext		   : Plots textstring
 * PSL_plottextbox	   : Draw a filled box around a textstring
 * PSL_plottextline	   : Place labels along paths (straight or curved), set clip path, draw line
 * PSL_loadeps		   : Read EPS file into string
 * PSL_command		   : Writes a given PostScript statement to the plot file
 * PSL_comment		   : Writes a comment statement to the plot file
 * PSL_makecolor	   : Returns string with PostScript command to set a new color
 * PSL_makepen		   : Returns string with PostScript command to set a new pen
 * PSL_setcolor		   : Sets the pen color or pattern
 * PSL_setcurrentpoint     : Sets the current point
 * PSL_setdefaults	   : Change several PSL session default values
 * PSL_setdash		   : Specify pattern for dashed line
 * PSL_setfill		   : Sets the fill color or pattern
 * PSL_setfont		   : Changes current font and possibly re-encodes it to current encoding
 * PSL_setformat	   : Changes # of decimals used in color and gray specs [3]
 * PSL_setlinecap	   : Changes the line cap setting
 * PSL_setlinejoin	   : Changes the line join setting
 * PSL_setlinewidth	   : Sets a new linewidth
 * PSL_setmiterlimit	   : Changes the miter limit setting for joins
 * PSL_setimage		   : Sets up a image pattern fill in PS
 * PSL_setorigin	   : Translates/rotates the coordinate system
 * PSL_setparagraph	   : Sets parameters used to typeset text paragraphs
 * PSL_settransparencymode : Set a new mode for how transparency is understoody
 * PSL_settransparency     : Set a new transparency
 * PSL_defpen		   : Encodes a pen with attributes by name in the PS output
 * PSL_definteger	   : Encodes an integer by name in the PS output
 * PSL_defpoints	   : Encodes a pointsize by name in the PS output
 * PSL_defcolor		   : Encodes a rgb color by name in the PS output
 * PSL_deftextdim	   : Sets variables for text height and width in the PS output
 * PSL_defunits:	   : Encodes a dimension by name in the PS output
 *
 * For information about usage, syntax etc, see the postscriptlight documentation
 *
 * Authors:	Paul Wessel, Dept. of Geology and Geophysics, SOEST, U Hawaii
 *			   pwessel@hawaii.edu
 *		Remko Scharroo, EUMETSAT, Darmstadt, Germany
 *			   Remko.Scharroo@eumetsat.int
 * Date:	13-OCT-2017
 * Version:	6.0 [64-bit enabled API edition, decoupled from GMT]
 *
 * Thanks to J. Goff and L. Parkes for their contributions to an earlier version.
 *
 */

/*--------------------------------------------------------------------
 *			SYSTEM HEADER FILES
 *--------------------------------------------------------------------*/

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>         /* Exact-width integer types */
#include "postscriptlight.h"
#ifdef HAVE_CTYPE_H_
#	include <ctype.h>
#endif
#ifdef HAVE_ASSERT_H_
#	include <assert.h>
#else
#	define assert(e) ((void)0)
#endif
#ifdef HAVE_UNISTD_H_
#	include <unistd.h>
#endif

#ifdef __CYGWIN__	/* See http://gmt.soest.hawaii.edu/boards/1/topics/5428 */
#ifdef __x86_64
#define lrint(x) ((long int)(int)lrint(x))
#endif
#endif

/*
 * Windows headers
 */

#ifdef HAVE_IO_H_
#	include <io.h>
#endif

#ifdef HAVE_PROCESS_H_
#	include <process.h>
#endif


#ifdef HAVE_ZLIB
#	include <zlib.h>
#endif

#ifndef PATH_MAX
#	define PATH_MAX 1024
#endif
/* Size prefixes for printf/scanf for size_t and ptrdiff_t */
#ifdef _MSC_VER
#	define PRIuS "Iu"  /* printf size_t */
#else
#	define PRIuS "zu"  /* printf size_t */
#endif

/* Define bswap32 */
#undef bswap32
#ifdef HAVE___BUILTIN_BSWAP32
#	define bswap32 __builtin_bswap32
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#	define bswap32 gnuc_bswap32
static inline uint32_t inline_bswap32 (uint32_t x) {
	return
		(((x & 0xFF000000U) >> 24) |
		 ((x & 0x00FF0000U) >>  8) |
		 ((x & 0x0000FF00U) <<  8) |
		 ((x & 0x000000FFU) << 24));
}
	static inline uint32_t gnuc_bswap32(uint32_t x) {
		if (__builtin_constant_p(x))
			x = inline_bswap32(x);
		else
			__asm__("bswap %0" : "+r" (x));
		return x;
	}
#elif defined HAVE__BYTESWAP_ULONG /* HAVE___BUILTIN_BSWAP32 */
#	define bswap32 _byteswap_ulong
#else /* HAVE___BUILTIN_BSWAP32 */
	static inline uint32_t inline_bswap32 (uint32_t x) {
		return
			(((x & 0xFF000000U) >> 24) |
			 ((x & 0x00FF0000U) >>  8) |
			 ((x & 0x0000FF00U) <<  8) |
			 ((x & 0x000000FFU) << 24));
	}
#	define bswap32 inline_bswap32
#endif /* HAVE___BUILTIN_BSWAP32 */

/* Macro for exit since this should be returned when called from Matlab */
#ifdef DO_NOT_EXIT
#define PSL_exit(code) return(code)
#else
#define PSL_exit(code) exit(code)
#endif

#define PSL_M_unused(x) (void)(x)

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

/* Include the 90 hardwired hachure patterns */
#include "PSL_patterns.h"

/* Listing of "Standard" 35 PostScript fonts found on most PS printers
 * plus the 4 Japanese fonts we have supported since GMT 3.
 * The fontheight is the height of A for unit fontsize. */

#define PSL_N_STANDARD_FONTS 39
static struct PSL_FONT PSL_standard_fonts[PSL_N_STANDARD_FONTS] = {
#include "standard_adobe_fonts.h"
};

/*--------------------------------------------------------------------
 *		     STANDARD CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif
#ifndef R2D
#define R2D (180.0/M_PI)
#endif
#ifndef D2R
#define D2R (M_PI/180.0)
#endif
#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

/* GMT normally gets these macros from unistd.h */
#ifndef HAVE_UNISTD_H_
#	define R_OK 4
#	define W_OK 2
#	ifdef WIN32
#		define X_OK R_OK /* X_OK == 1 crashes on Windows */
#	else
#		define X_OK 1
#	endif
#	define F_OK 0
#endif /* !HAVE_UNISTD_H_ */

/* access is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in io.h */
#if defined HAVE__ACCESS && !defined HAVE_ACCESS
#	define access _access
#endif

#if defined HAVE_STRTOK_S && !defined HAVE_STRTOK_R
#	define strtok_r strtok_s
#elif !defined HAVE_STRTOK_R
/* define custom function */
#endif

/* getpid is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in process.h */
#if defined HAVE__GETPID && !defined HAVE_GETPID
#	define getpid _getpid
#endif

/*--------------------------------------------------------------------
 *			PSL CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#define PS_LANGUAGE_LEVEL       2
#define PSL_Version             "5.0"
#define PSL_SMALL               1.0e-10
#define PSL_PAGE_HEIGHT_IN_PTS  842     /* A4 height */
#define PSL_PEN_LEN		128	/* Style length string */
#define PSL_SUBSUP_SIZE		0.7	/* Relative size of sub/sup-script to normal size */
#define PSL_SCAPS_SIZE		0.85	/* Relative size of small caps to normal size */
#define PSL_SUB_DOWN		0.25	/* Baseline shift down in font size for subscript */
#define PSL_SUP_UP_LC		0.35	/* Baseline shift up in font size for superscript after lowercase letter */
#define PSL_SUP_UP_UC		0.35	/* Baseline shift up in font size for superscript after uppercase letter */
#if 0
/* These are potential revisions to some of the settings above but remains to be tested */
#define PSL_SUBSUP_SIZE		0.58	/* Relative size of sub/sup-script to normal size */
#define PSL_SCAPS_SIZE		0.80	/* Relative size of small caps to normal size */
#define PSL_SUB_DOWN		0.25	/* Baseline shift down in font size for subscript */
#define PSL_SUP_UP_LC		0.35	/* Baseline shift up in font size for superscript after lowercase letter */
#define PSL_SUP_UP_UC		0.45	/* Baseline shift up in font size for superscript after uppercase letter */
#define PSL_SUBSUP_SIZE		0.58	/* Relative size of sub/sup-script to normal size */
#define PSL_SCAPS_SIZE		0.80	/* Relative size of small caps to normal size */
#define PSL_SUB_DOWN		0.33	/* Baseline shift down in font size for subscript */
#define PSL_SUP_UP		0.33	/* Baseline shift up in font size for superscript */
#endif

/*--------------------------------------------------------------------
 *			PSL FUNCTION MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#define PSL_s255(s) (s * 255.0)								/* Conversion from 0-1 to 0-255 range */
#define PSL_u255(s) ((unsigned char)rint(PSL_s255(s)))					/* Conversion from 0-1 to 0-255 range */
#define PSL_t255(t) PSL_u255(t[0]),PSL_u255(t[1]),PSL_u255(t[2])			/* ... same for triplet */
#define PSL_q255(q) PSL_u255(q[0]),PSL_u255(q[1]),PSL_u255(q[2]),PSL_u255(q[3])		/* ... same for quadruplet */
#define PSL_YIQ(rgb) (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2])			/* How B/W TV's convert RGB to Gray */
#define PSL_eq(a,b) (fabs((a)-(b)) < PSL_SMALL)						/* If two color component are ~identical */
#define PSL_is_gray(rgb) (PSL_eq(rgb[0],rgb[1]) && PSL_eq(rgb[1],rgb[2]))		/* If the rgb is a color and not gray */
#define PSL_same_rgb(a,b) (PSL_eq(a[0],b[0]) && PSL_eq(a[1],b[1]) && PSL_eq(a[2],b[2]) && PSL_eq(a[3],b[3]))	/* If two colors are ~identical */
#define PSL_rgb_copy(a,b) memcpy((void*)a,(void*)b,4*sizeof(double));			/* Copy RGB[T] triplets: a = b */

#define PSL_memory(C,ptr,n,type) (type*)psl_memory(C,(void*)ptr,(size_t)(n),sizeof(type))	/* Easier macro for psl_memory */

/* Special macros and structure for PSL_plotparagraph */

#define PSL_NO_SPACE		0
#define PSL_ONE_SPACE		1
#define PSL_COMPOSITE_1		8
#define PSL_COMPOSITE_2		16
#define PSL_SYMBOL_FONT		12
#define PSL_CHUNK		2048

#define PSL_CLOSE_INTERIOR	16

/* Indices for use with PSL->current.sup_up[] */
#define PSL_LC	0
#define PSL_UC	1

struct PSL_WORD {	/* Used for type-setting text */
	int font_no;
	int flag;
	int index;
	int baseshift;
	int fontsize;
	double rgb[4];
	char *txt;
};

struct PSL_COLOR {
	double rgb[4];	/* r/g/b plus alpha (PDF only) */
};

/* Special macros and structure for color(sic) maps-> */

#define PSL_INDEX_BITS 8	/* PostScript indices may be 12 bit */
			/* But we only do 8 bits for now. */
#define PSL_MAX_COLORS (1<<PSL_INDEX_BITS)

typedef struct
{
	size_t ncolors;
	unsigned char colors[PSL_MAX_COLORS][3];
} *psl_colormap_t;

typedef struct
{
	unsigned char *buffer;
	psl_colormap_t colormap;
} *psl_indexed_image_t;

typedef struct {
	size_t nbytes;
	int depth;
	unsigned char *buffer;
} *psl_byte_stream_t;

/* These are used when the PDF pdfmark extension for transparency is used. */

#define N_PDF_TRANSPARENCY_MODES	16
static const char *PDF_transparency_modes[N_PDF_TRANSPARENCY_MODES] = {
	"Color", "ColorBurn", "ColorDodge", "Darken",
	"Difference", "Exclusion", "HardLight", "Hue",
	"Lighten", "Luminosity", "Multiply", "Normal",
	"Overlay", "Saturation", "SoftLight", "Screen"
};

#ifdef WIN32

#ifndef HAVE_STRSEP
/* Copyright (C) 2004, 2007, 2009-2012 Free Software Foundation, Inc.

   Written by Yoann Vandoorselaere <yoann@prelude-ids.org>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

char *strsep (char **stringp, const char *delim) {
	char *start = *stringp;
	char *ptr;

	if (start == NULL)
		return NULL;

	/* Optimize the case of no delimiters.  */
	if (delim[0] == '\0') {
		*stringp = NULL;
		return start;
    }

	/* Optimize the case of one delimiter.  */
	if (delim[1] == '\0')
		ptr = strchr (start, delim[0]);
	else
	/* The general case.  */
	ptr = strpbrk (start, delim);
	if (ptr == NULL) {
		*stringp = NULL;
		return start;
	}

	*ptr = '\0';
	*stringp = ptr + 1;
	return start;
}
#endif /* ifndef HAVE_STRSEP */

/* SUpport for differences between UNIX and DOS paths */

static void psl_strlshift (char *string, size_t n) {
	/* Left shift a string by n characters */
	size_t len;
	assert (string != NULL); /* NULL pointer */

	if ((len = strlen(string)) <= n ) {
		/* String shorter than shift width */
		*string = '\0'; /* Truncate entire string */
		return;
	}

	/* Move entire string back */
	memmove(string, string + n, len + 1);
}

static void psl_strrepc (char *string, int c, int r) {
	/* Replaces all occurrences of c in the string with r */
	assert (string != NULL); /* NULL pointer */
	do {
		if (*string == c)
			*string = (char)r;
	} while (*(++string)); /* repeat until \0 reached */
}

/* Turn '/c/dir/...' paths into 'c:/dir/...'
 * Must do it in a loop since dir may be several ';'-separated dirs */
static void psl_dos_path_fix (char *dir) {
	size_t n, k;

	if (!dir || (n = strlen (dir)) < 2U)
		/* Given NULL or too short dir to work */
		return;

	if (!strncmp (dir, "/cygdrive/", 10U))
		/* May happen for example when Cygwin sets GMT_SHAREDIR */
		psl_strlshift (dir, 9); /* Chop '/cygdrive' */

	/* Replace dumb backslashes with slashes */
	psl_strrepc (dir, '\\', '/');

	/* If dir begins with '/' and is 2 long, as in '/c', replace with 'c:' */
	if (n == 2U && dir[0] == '/') {
		dir[0] = dir[1];
		dir[1] = ':';
		return;
	}

	/* If dir is longer than 2 and, e.g., '/c/', replace with 'c:/' */
	if (n > 2U && dir[0] == '/' && dir[2] == '/' && isalpha ((int)dir[1])) {
		dir[0] = dir[1];
		dir[1] = ':';
	}

	/* Do the same with dirs separated by ';' but do not replace '/c/j/...' with 'c:j:/...' */
	for (k = 4; k < n-2; k++) {
		if ( (dir[k-1] == ';' && dir[k] == '/' && dir[k+2] == '/' && isalpha ((int)dir[k+1])) ) {
			dir[k] = dir[k+1];
			dir[k+1] = ':';
		}
	}

	/* Replace ...:C:/... by ...;C:/... as that was a multi-path set by a e.g. bash shell (msys or cygwin) */
	for (k = 4; k < n-2; k++) {
		if ((dir[k-1] == ':' && dir[k+1] == ':' && dir[k+2] == '/' && isalpha ((int)dir[k])) )
			dir[k-1] = ';';
		else if ((dir[k-1] == ':' && dir[k] == '/' && dir[k+2] == '/' && isalpha ((int)dir[k+1])) ) {
			/* The form ...:/C/... will become ...;C:/... */
			dir[k-1] = ';';
			dir[k] = dir[k+1];
			dir[k+1] = ':';
		}
	}
}
#else
# define psl_dos_path_fix(e) ((void)0) /* dummy function */
#endif

/* ----------------------------------------------------------------------
 * Support functions used in PSL_* functions.
 * ----------------------------------------------------------------------
 */

static void *psl_memory (struct PSL_CTRL *PSL, void *prev_addr, size_t nelem, size_t size) {
	/* Multi-functional memory allocation subroutine.
	   If prev_addr is NULL, allocate new memory of nelem elements of size bytes.
	   	Ignore when nelem == 0.
	   If prev_addr exists, reallocate the memory to a larger or smaller chunk of nelem elements of size bytes.
	   	When nelem = 0, free the memory.
	*/

	void *tmp = NULL;
	const char *m_unit[4] = {"bytes", "kb", "Mb", "Gb"};
	double mem;
	int k;

	if (prev_addr) {
		if (nelem == 0) { /* Take care of n == 0 */
			PSL_free (prev_addr);
			return (NULL);
		}
		if ((tmp = realloc ( prev_addr, nelem * size)) == NULL) {
			mem = (double)(nelem * size);
			k = 0;
			while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
			PSL_message (PSL, PSL_MSG_ERROR, "Error: Could not reallocate more memory [%.2f %s, %" PRIuS " items of %" PRIuS " bytes]\n",
			             mem, m_unit[k], nelem, size);
			return (NULL);
		}
	}
	else {
		if (nelem == 0) return (NULL); /* Take care of n = 0 */
		if ((tmp = calloc (nelem, size)) == NULL) {
			mem = (double)(nelem * size);
			k = 0;
			while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
			PSL_message (PSL, PSL_MSG_ERROR, "Error: Could not allocate memory [%.2f %s, %" PRIuS " items of %" PRIuS " bytes]\n",
			             mem, m_unit[k], nelem, size);
			return (NULL);
		}
	}
	return (tmp);
}

/* Things to do with UTF8 */

/* Try to convert UTF-8 accented characters to PostScriptLight octal codes.
 * This depends on which character set we have.  We will limit this to just
 * Standard, Standard+, ISOILatin1, and ISOLatin1+. Of these, Standard will
 * only work for some of the encoded letters while the three others should
 * all be fine.
 *   We also handle the differences between hyphens and minus symbol.
 * In ISOLatin1 the hyphen key on the keyboard results in a minus sign while
 * in Standard it gives a hyphen.  In GMT we want minus signs in annotations
 * contours and other numerical negative values. This behavior is controlled
 * by the setting in PSL_settextmode. */


static unsigned int psl_ut8_code_to_ISOLatin (char code) {
	/* This is called when the previous character in a string has octal 0303 */
	unsigned int kode = (unsigned char)code;

	return (kode >= 0200 && kode <= 0277) ? kode += 64 : 0;
}

static void psl_fix_utf8 (struct PSL_CTRL *PSL, char *in_string) {
	/* Given in_string check if UTF8 characters are present and if so replace with PSL octal codes.  Assumes ISOLatin1+ */
	unsigned int k, kout, use, utf8_codes = 0;
	bool do_minus = (PSL->current.use_minus == PSL_TXTMODE_MINUS);
	char *out_string = NULL;

	if (!strncmp (PSL->init.encoding, "Standard+", 9U) && do_minus) {	/* For Standard+ encoding we may need to swap leading minus values encoded as hyphen with the actual minus symbol */
		for (k = 0; in_string[k]; k++) {
			if ((k == 0 || in_string[k-1] != '@') && in_string[k] == 0055)	/* Found a hyphen which we interpret to be a minus sign */
				in_string[k] = 0224;	/* Minus is octal 224 in Standard+ but not present in just Standard */
		}
	}

	if (strncmp (PSL->init.encoding, "ISOLatin1", 9U)) return;	/* Do nothing unless ISOLatin[+] */

	for (k = 0; in_string[k]; k++) {
		if ((unsigned char)(in_string[k]) == 0303 || (unsigned char)(in_string[k]) == 0305)
			utf8_codes++;	/* Count them up */
		else if (k == 0 || in_string[k-1] != '@') {
			if ((unsigned char)in_string[k] == 0255 && do_minus)
				in_string[k] = 0055;	/* Minus symbol is octal 0055 in ISOLatin1 */
			else if ((unsigned char)in_string[k] == 0055 && !do_minus)
				in_string[k] = 0255;	/* Hyphen symbol is octal 0255 in ISOLatin1 */
		}
	}
	if (utf8_codes == 0) return;	/* Nothing to do */

	out_string = PSL_memory (PSL, NULL, strlen(in_string) + 1, char);	/* Get a new string of same length (extra byte for '\0') */

	for (k = kout = 0; in_string[k]; k++) {
		if ((unsigned char)(in_string[k]) == 0303) {    /* Found octal 303 */
			k++;	/* Skip the control code */
			if ((use = psl_ut8_code_to_ISOLatin (in_string[k])))       /* Found a 2-char utf8 combo, replace with single octal code from our table */
				out_string[kout++] = use;
			else {    /* Not a recognized code - just output both as they were given */
				out_string[kout++] = in_string[k-1];
				out_string[kout++] = in_string[k];
			}
		}
		else if ((unsigned char)(in_string[k]) == 0305) {    /* Found Ydieresis, ae, AE, L&l-slash and the S,Z,s,z carons */
			k++;	/* Skip the control code */
			switch ((unsigned char)in_string[k]) {	/* These 9 chars are placed all over the table so must have individual cases */
				case 0201: use = 0203; break;	/* Lslash */
				case 0202: use = 0213; break;	/* lslash */
				case 0222: use = 0200; break;	/* ae */
				case 0223: use = 0210; break;	/* AE */
				case 0240: use = 0206; break;	/* Scaron */
				case 0241: use = 0177; break;	/* scaron */
				case 0270: use = 0211; break;	/* Ydieresis */
				case 0275: use = 0212; break;	/* Zcaron */
				case 0276: use = 0037; break;	/* zcaron */
				default:   use = 0;    break;	/* Not one of the recognized ones in our table */
			}
			if (use)	/* Found a 2-char utf8 combo */
				out_string[kout++] = use;
			else  {    /* Not a recognized code - just output both as they were given */
				out_string[kout++] = in_string[k-1];
				out_string[kout++] = in_string[k];
			}
		}
		else    /* Just output char as was given */
			out_string[kout++] = in_string[k];
	}
	memset (in_string, 0, strlen (in_string));		/* Set old in_string to NULL */
	strncpy (in_string, out_string, strlen (out_string));	/* Overwrite old string with possibly adjusted string */
	PSL_free (out_string);
}

/* This one is NOT static since needed in psimage, at least for now */

unsigned char *psl_gray_encode (struct PSL_CTRL *PSL, size_t *nbytes, unsigned char *input) {
	/* Recode RGB stream as gray-scale stream */

	size_t in, out, nout;
	unsigned char *output = NULL;

	nout = *nbytes / 3;
	output = PSL_memory (PSL, NULL, nout, unsigned char);

	for (in = out = 0; in < *nbytes; out++, in += 3) output[out] = (char) lrint (PSL_YIQ ((&input[in])));
	*nbytes = nout;
	return (output);
}

/* Define local (static) support functions called inside the public PSL functions */

static int psl_ix (struct PSL_CTRL *PSL, double x) {
	/* Convert user x to PS dots */
	return (PSL->internal.x0 + (int)lrint (x * PSL->internal.x2ix));
}

static int psl_iy (struct PSL_CTRL *PSL, double y) {
	/* Convert user y to PS dots */
	return (PSL->internal.y0 + (int)lrint (y * PSL->internal.y2iy));
}

static int psl_iz (struct PSL_CTRL *PSL, double z) {
	/* Convert user distances to PS dots */
	return ((int)lrint (z * PSL->internal.dpu));
}

static int psl_ip (struct PSL_CTRL *PSL, double p) {
	/* Convert PS points to PS dots */
	return ((int)lrint (p * PSL->internal.dpp));
}

static int psl_shorten_path_new (struct PSL_CTRL *PSL, double *x, double *y, int n, int *ix, int *iy, int mode) {
	/* Simplifies the (x,y) array by converting it to pixel coordinates (ix,iy)
	 * and eliminating repeating points and intermediate points along straight
	 * line segments.  The result is the fewest points needed to draw the path
	 * and still look exactly like the original path.  However, if mode == 1 we do
	 * no shortening. */

	int i, k, dx, dy;
	int d, db, bx, by, j, ij;

	if (n < 2) return (n);	/* Not a path to start with */

	for (i = 0; i < n; i++) {	/* Convert all coordinates to integers at current scale */
		ix[i] = psl_ix (PSL, x[i]);
		iy[i] = psl_iy (PSL, y[i]);
	}
	if (mode == 1) return (n);

	/* Skip intermediate points that are "close" to the line between point i and point j, where
	   "close" is defined as less than 1 "dot" (the PostScript resolution) in either direction.
	   A point is always close when it coincides with one of the end points (i or j).
	   An intermediate point is also considered "far" when it is beyond i or j.
	   Algorithm requires that |dx by - bx dy| >= max(|dx|,|dy|) for points to be "far".
	*/
	for (i = k = 0, j = 2; j < n; j++) {
		dx = ix[j] - ix[i];
		dy = iy[j] - iy[i];
		d = MAX(abs((int)dx),abs((int)dy));
		/* We know that d can be zero. That is OK, since it will only happen when (dx,dy) = (0,0).
		   And in that cases all intermediate points will always be "far" */
		for (ij = j - 1; ij > i; ij--) {
			bx = ix[ij] - ix[i];
			/* Check if the intermediate point is outside the x-range between points i and j.
			   In case of a vertical line, any point with a different x-coordinate is "far" */
			if (dx > 0) {
				if (bx < 0 || bx > dx) break;
			}
			else {
				if (bx > 0 || bx < dx) break;
			}
			by = iy[ij] - iy[i];
			/* Check if the intermediate point is outside the y-range between points i and j.
			   In case of a horizontal line, any point with a different y-coordinate is "far" */
			if (dy > 0) {
				if (by < 0 || by > dy) break;
			}
			else {
				if (by > 0 || bx < dy) break;
			}
			/* Generic case where the intermediate point is within the x- and y-range */
			db = abs((int)(dx * by) - (int)(bx * dy));
			if (db >= d) break; /* Point ij is "far" from line connecting i and j */
		}
		if (ij > i) {	/* Some intermediate point failed test */
			i = j - 1;
			k++;
			ix[k] = ix[i];
			iy[k] = iy[i];
		}
	}

	/* We have gotten to the last point. If this is a duplicate, skip it */
	if (ix[k] != ix[n-1] || iy[k] != iy[n-1]) {
		k++;
		ix[k] = ix[n-1];
		iy[k] = iy[n-1];
	}
	k++;

	return (k);
}

static int psl_shorten_path_old (struct PSL_CTRL *PSL, double *x, double *y, int n, int *ix, int *iy, int mode) {
	/* Simplifies the (x,y) array by converting it to pixel coordinates (ix,iy)
	 * and eliminating repeating points and intermediate points along straight
	 * line segments.  The result is the fewest points needed to draw the path
	 * and still look exactly like the original path.  However, if mode == 1 we do
	 * no shortening. */

	int i, k, dx, dy;
	int old_dir = 0, new_dir;
	double old_slope = -DBL_MAX, new_slope;
	/* These seeds for old_slope and old_dir make sure that first point gets saved */

	if (n < 2) return (n);	/* Not a path to start with */

	for (i = 0; i < n; i++) {	/* Convert all coordinates to integers at current scale */
		ix[i] = psl_ix (PSL, x[i]);
		iy[i] = psl_iy (PSL, y[i]);
	}
	if (mode == 1) return (n);

	/* The only truly unique point is the starting point; all else must show increments
	 * relative to the previous point */

	/* First point is the anchor. We will find at least one point, unless all points are the same */
	for (i = k = 0; i < n - 1; i++) {
		dx = ix[i+1] - ix[i];
		dy = iy[i+1] - iy[i];
		if (dx == 0 && dy == 0) continue;	/* Skip duplicates */
		new_slope = (dx == 0) ? copysign (DBL_MAX, (double)dy) : ((double)dy) / ((double)dx);
		new_dir = (dx >= 0) ? 1 : -1;
		if (new_slope != old_slope || new_dir != old_dir) {
			ix[k] = ix[i];
			iy[k] = iy[i];
			k++;
			old_slope = new_slope;
			old_dir = new_dir;
		}
	}

	/* If all points are the same, we get here with k = 0, so we can exit here now with 1 point */
	if (k < 1) return (1);

	/* Last point (k cannot be < 1 so k-1 >= 0) */
	if (ix[k-1] != ix[n-1] || iy[k-1] != iy[n-1]) {	/* Do not do slope check on last point since we must end there */
		ix[k] = ix[n-1];
		iy[k] = iy[n-1];
		k++;
	}

	return (k);
}

#define N_LENGTH_THRESHOLD 100000000
static int psl_shorten_path (struct PSL_CTRL *PSL, double *x, double *y, int n, int *ix, int *iy, int mode) {
	if (n > N_LENGTH_THRESHOLD)
		return psl_shorten_path_old (PSL, x, y, n, ix, iy, mode);
	else
		return psl_shorten_path_new (PSL, x, y, n, ix, iy, mode);
}

static int psl_forcelinewidth (struct PSL_CTRL *PSL, double linewidth) {
	if (linewidth < 0.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Selected linewidth is negative (%g), ignored\n", linewidth);
		return (PSL_BAD_WIDTH);
	}
	PSL_command (PSL, "%d W\n", psl_ip (PSL, linewidth));
	PSL->current.linewidth = linewidth;
	return (PSL_NO_ERROR);
}

static void psl_set_real_array (struct PSL_CTRL *PSL, const char *prefix, double *array, int n) {
	/* These are raw and not scaled */
	int i;
	PSL_command (PSL, "/PSL_%s [ ", prefix);
	for (i = 0; i < n; i++) {
		PSL_command (PSL, "%.2f ", array[i]);
		if (((i+1)%10) == 0) PSL_command (PSL, "\n\t");
	}
	PSL_command (PSL, "] def\n");
}

void psl_set_int_array (struct PSL_CTRL *PSL, const char *prefix, int *array, int n) {
	/* These are raw and not scaled */
	int i;
	PSL_command (PSL, "/PSL_%s [ ", prefix);
	for (i = 0; i < n; i++) {
		PSL_command (PSL, "%d ", array[i]);
		if (((i+1)%10) == 0) PSL_command (PSL, "\n\t");
	}
	PSL_command (PSL, "] def\n");
}

void psl_set_txt_array (struct PSL_CTRL *PSL, const char *prefix, char *array[], int n) {
	int i;
	char *outtext = NULL;
	PSL_command (PSL, "/PSL_%s [\n", prefix);
	for (i = 0; i < n; i++) {
		outtext = psl_prepare_text (PSL, array[i]);	/* Expand escape codes and fix utf-8 characters */
		PSL_command (PSL, "\t(%s)\n", outtext);
		PSL_free (outtext);
	}
	PSL_command (PSL, "] def\n", n);
}

static void psl_set_reducedpath_arrays (struct PSL_CTRL *PSL, double *x, double *y, int npath, int *n, int *m, int *node) {
	/* These are used by PSL_plottextline.  We make sure there are no point pairs that would yield dx = dy = 0 (repeat point)
	 * at the resolution we are using (0.01 DPI units), hence a new n (possibly shorter) is returned. */
	int i, j, k, p, ii, kk, this_i, this_j, last_i, last_j, i_offset = 0, k_offset = 0, n_skipped, ntot = 0, new_tot = 0, *new_n = NULL;
	char *use = NULL;
	if (x == NULL || y == NULL) return;	/* No path */
	for (p = 0; p < npath; p++) ntot += n[p];	/* Determine total number of points */
	/* Since we need dx/dy from these we preprocess to avoid any nasty surprises with repeat points */
	use = PSL_memory (PSL, NULL, ntot, char);
	new_n = PSL_memory (PSL, NULL, npath, int);
	for (p = 0; p < npath; p++) {
		this_i = this_j = INT_MAX;
		for (ii = j = n_skipped = k = 0; ii < n[p]; ii++) {
			last_i = this_i;	last_j = this_j;
			i = ii + i_offset;	/* Index into concatenated x,y arrays */
			this_i = 100 * psl_ix (PSL, x[i]);	/* Simulates the digits written by a %.2lf format */
			this_j = 100 * psl_iy (PSL, y[i]);
			if (this_i == last_i && this_j == last_j)	/* Repeat point, skip it */
				n_skipped++;
			else {	/* Not a repeat point, use it */
				use[i] = true;
				j++;
			}
			kk = k + k_offset;	/* Index into concatenated node array */
			if (k < m[p] && node[kk] == ii && n_skipped) {	/* Adjust node pointer since we are removing points and upsetting the node order */
				node[kk++] -= n_skipped;
				k++;
			}
		}
		new_n[p] = j;
		new_tot += j;
		i_offset += n[p];
		k_offset += m[p];

	}

	PSL_comment (PSL, "Set concatenated coordinate arrays for line segments:\n");
	PSL_command (PSL, "/PSL_path_x [ ");
	for (i = k = 0; i < ntot; i++) {
		if (!use[i]) continue;
		PSL_command (PSL, "%d ", psl_ix (PSL, x[i]));
		k++;
		if ((k%10) == 0) PSL_command (PSL, "\n\t");
	}
	PSL_command (PSL, "] def\n");
	PSL_command (PSL, "/PSL_path_y [ ");
	for (i = k = 0; i < ntot; i++) {
		if (!use[i]) continue;
		PSL_command (PSL, "%d ", psl_iy (PSL, y[i]));
		k++;
		if ((k%10) == 0) PSL_command (PSL, "\n\t");
	}
	PSL_command (PSL, "] def\n");
	PSL_comment (PSL, "Set array with number of points per line segments:\n");
	psl_set_int_array (PSL, "path_n", new_n, npath);
	if (k > PSL_MaxOpStack_Size) PSL_message (PSL, PSL_MSG_WARNING, "Warning: PSL array placed has %d items - may exceed gs_init.ps MaxOpStack setting [%d].\n", k, PSL_MaxOpStack_Size);

	/* Free up temp arrays */
	PSL_free (use);
	PSL_free (new_n);
	return;
}

static void psl_set_path_arrays (struct PSL_CTRL *PSL, const char *prefix, double *x, double *y, int npath, int *n) {
	/* Set coordinates arrays in PS units */
	int i, ntot = 0;
	char txt[64] = {""};

	if (x == NULL || y == NULL) return;		/* No path */
	for (i = 0; i < npath; i++) ntot += n[i];	/* Determine total number of points */

	PSL_comment (PSL, "Set coordinate arrays for text label placements:\n");
	PSL_command (PSL, "/PSL_%s_x [ ", prefix);
	for (i = 0; i < ntot; i++) {
		PSL_command (PSL, "%d ", psl_ix (PSL, x[i]));
		if (((i+1)%10) == 0) PSL_command (PSL, "\n\t");
	}
	PSL_command (PSL, "] def\n");
	PSL_command (PSL, "/PSL_%s_y [ ", prefix);
	for (i = 0; i < ntot; i++) {
		PSL_command (PSL, "%d ", psl_iy (PSL, y[i]));
		if (((i+1)%10) == 0) PSL_command (PSL, "\n\t");
	}
	PSL_command (PSL, "] def\n");
	sprintf (txt, "%s_n", prefix);
	psl_set_int_array (PSL, txt, n, npath);
}

static void psl_set_attr_arrays (struct PSL_CTRL *PSL, int *node, double *angle, char **txt, int npath, int m[]) {
	/* This function sets PSL arrays for attributes needed to place contour labels and quoted text labels.
	 * node:	specifies where along each segments there should be labels [NULL if not curved text]
	 * angle:	specifies angle of text for each item
	 * txt:		is the text labels for each item
	 * npath:	the number of segments (curved text) or number of text items (straight text)
	 * m:		array of length npath with number of labels per segment
	 */
	int i, nlab = 0;

	for (i = 0; i < npath; i++) nlab += m[i];	/* Determine total number of labels */
	if (node != NULL) {	/* Curved text has node array */
		PSL_comment (PSL, "Set array with nodes of PSL_path_x|y for text placement:\n");
		psl_set_int_array (PSL, "label_node", node, nlab);
		PSL_comment (PSL, "Set array with number of labels per line segment:\n");
		psl_set_int_array (PSL, "label_n", m, npath);
	}
	PSL_comment (PSL, "Set array with baseline angle for each text label:\n");
	psl_set_real_array (PSL, "label_angle", angle, nlab);
	PSL_comment (PSL, "Set array with the text labels:\n");
	psl_set_txt_array (PSL, "label_str", txt, nlab);

	return;
}

static void psl_rgb_to_hsv (double rgb[], double hsv[]) {
	double diff;
	int i, imax = 0, imin = 0;

	/* This had checks using rgb value in doubles (e.g. (max_v == xr)), which failed always on some compilers.
	   Changed to integer logic: 2009-02-05 by RS.
	*/
	for (i = 1; i < 3; i++) {
		if (rgb[i] > rgb[imax]) imax = i;
		if (rgb[i] < rgb[imin]) imin = i;
	}
	diff = rgb[imax] - rgb[imin];
	hsv[0] = 0.0;
	hsv[1] = (PSL_eq(rgb[imax],0.0)) ? 0.0 : diff / rgb[imax];
	hsv[2] = rgb[imax];
	if (PSL_eq(hsv[1],0.0)) return;	/* Hue is undefined */
	hsv[0] = 120.0 * imax + 60.0 * (rgb[(imax + 1) % 3] - rgb[(imax + 2) % 3]) / diff;
	if (hsv[0] < 0.0) hsv[0] += 360.0;
	if (hsv[0] > 360.0) hsv[0] -= 360.0;
}

#if 0 /* Not used */
static void psl_cmyk_to_rgb (double rgb[], double cmyk[]) {
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* CMYK is in 0-1, RGB will be in 0-1 range */

	int i;

	for (i = 0; i < 3; i++) rgb[i] = 1.0 - cmyk[i] - cmyk[3];
}
#endif
static void psl_rgb_to_cmyk_char (unsigned char rgb[], unsigned char cmyk[]) {
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* RGB is in 0-255, CMYK will be in 0-255 range */

	int i;

	for (i = 0; i < 3; i++) cmyk[i] = 255 - rgb[i];
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Black */
	for (i = 0; i < 3; i++) cmyk[i] -= cmyk[3];
}

static void psl_rgb_to_cmyk (double rgb[], double cmyk[]) {
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* RGB is in 0-1, CMYK will be in 0-1 range */

	int i;

	for (i = 0; i < 3; i++) cmyk[i] = 1.0 - rgb[i];
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Black */
	for (i = 0; i < 3; i++) cmyk[i] -= cmyk[3];
	for (i = 0; i < 4; i++) {
	    if (cmyk[i] < 0.0005) cmyk[i] = 0.0;	/* Needs some explanation... */
	}
}

static unsigned char *psl_cmyk_encode (struct PSL_CTRL *PSL, size_t *nbytes, unsigned char *input) {
	/* Recode RGB stream as CMYK stream */

	size_t in, out, nout;
	unsigned char *output = NULL;

	nout = *nbytes / 3 * 4;
	output = PSL_memory (PSL, NULL, nout, unsigned char);

	for (in = out = 0; in < *nbytes; out += 4, in += 3) psl_rgb_to_cmyk_char (&input[in], &output[out]);
	*nbytes = nout;
	return (output);
}

static void psl_remove_spaces (char *label[], int n_labels, int m[]) {
	int i, k, j, n_tot = n_labels;

	if (m)
		for (i = 0; i < n_labels; i++) n_tot += m[i];	/* Count number of labels */

	for (i = 0; i < n_tot; i++) {	/* Strip leading and trailing blanks */
		for (k = 0; label[i][k] == ' '; k++);	/* Count # of leading blanks */
		if (k > 0) {	/* Shift text to start, eliminating spaces */
			j = 0;
			while (label[i][k]) {
				label[i][j] = label[i][j+k];
				j++;
			}
			label[i][j] = 0;
		}
		/* Then strip off trailing blanks, if any */
		for (j = (int)strlen (label[i]) - 1; label[i][j] == ' '; j--) label[i][j] = 0;
	}
}

static void psl_prepare_buffer (struct PSL_CTRL *C, size_t len) {
	/* Ensure buffer is large enough to accept additional text of length len */
	size_t new_len = C->internal.n + len;       /* Need a buffer at least this large */
	if (new_len < C->internal.n_alloc) return;  /* Already have a buffer that is large enough */
	while (new_len > C->internal.n_alloc)       /* Wind past what is needed, growing by 1.75 */
		C->internal.n_alloc = (size_t)(C->internal.n_alloc * 1.75);
	if ((C->internal.buffer = PSL_memory (C, C->internal.buffer, C->internal.n_alloc, char)) == NULL) {
		PSL_message (C, PSL_MSG_ERROR, "Error: Could not allocate %d additional buffer space - this will not end well\n", len);
	}
}

static size_t psl_a85_encode (struct PSL_CTRL *PSL, const unsigned char *src_buf, size_t nbytes) {
	/* Encode 4-byte binary data from src_buf to 5-byte ASCII85
	 * Special cases: 0x00000000 is encoded as z
	 * Encoded data is stored in dst_buf and written to file in
	 * one go, which is faster than writing one char at a time.
	 * The function returns the output buffer size. */
	size_t dst_buf_size;
	unsigned char *dst_buf, *dst_ptr;
	const unsigned char *src_ptr = src_buf, *src_end = src_buf + nbytes;
	const unsigned int max_line_len = 95; /* number of chars after which a newline is inserted */

	if (!nbytes)
		/* Ignore empty input */
		return 0;

	/* dst_buf has to be large enough to hold data + line endings */
	dst_buf_size = (size_t)(nbytes * 1.25 + 1);      /* output buffer is at least 1.25 times larger */
	dst_buf_size += dst_buf_size / max_line_len + 4; /* add more space for '\n' and delimiter */
	dst_ptr = dst_buf = PSL_memory (PSL, NULL, dst_buf_size, unsigned char); /* output buffer */

	do { /* for each quad in src_buf while src_ptr < src_end */
		const size_t ilen = nbytes > 4 ? 4 : nbytes, olen = ilen + 1;
		static unsigned int line_len = 0;
		unsigned int i, n = 0, byte;
		int j;
		unsigned char quintuple[5] = { 0 };

		/* Wrap 4 chars into a 4-byte integer */
		for (i = 0; i < ilen; ++i) {
			byte = *src_ptr++;
			n += byte << (24 - 8*i);
		}

		if (n == 0 && ilen == 4) {
			/* Set the only output byte to "z" */
			*dst_ptr++ = 'z';
			++line_len;
			continue;
		}

		/* Else determine output 5-tuple */
		for (j = 4; j >= 0; --j) {
			quintuple[j] = (unsigned char) ((n % 85) + '!');
			n = n / 85;
		}

		/* Copy olen bytes to dst_buf */
		memcpy (dst_ptr, quintuple, olen);
		line_len += (unsigned int)olen;
		dst_ptr += olen;

		/* Insert newline when line exceeds 95 characters */
		if (line_len + 1 > max_line_len) {
			*dst_ptr++ = '\n';
			line_len = 0;
		}
	} while (nbytes -= 4, src_ptr < src_end); /* end do */

	{
		/* Mark the end of the Adobe ASCII85-encoded string: */
		const unsigned char delimiter[] = "~>\n";
		memcpy (dst_ptr, delimiter, 3);
		dst_ptr += 3;
	}

	{
		/* Write buffer to file and clean up */
		const size_t buf_size = dst_ptr - dst_buf;
		assert (buf_size <= dst_buf_size); /* check length */
		if (PSL->internal.memory) {
			psl_prepare_buffer (PSL, buf_size); /* Make sure we have enough memory to hold the entire EPS */
			strncat (&(PSL->internal.buffer[PSL->internal.n]), (const char *)dst_buf, buf_size);
			PSL->internal.n += buf_size;
		}
		else
			fwrite (dst_buf, sizeof(char), buf_size, PSL->internal.fp);
		PSL_free (dst_buf);
		return buf_size;
	}
}

#define ESC 128

static unsigned char *psl_rle_encode (struct PSL_CTRL *PSL, size_t *nbytes, unsigned char *input) {
	/* Run Length Encode a buffer of nbytes. */

	size_t count = 0, out = 0, in = 0, i;
	unsigned char pixel, *output = NULL;

	i = MAX (512, *nbytes) + 136;	/* Maximum output length */
	output = PSL_memory (PSL, NULL, i, unsigned char);

	/* Loop scanning all input bytes. Abort when inflating after processing at least 512 bytes */
	while (count < *nbytes && (out < in || out < 512)) {
		in = count;
		pixel = input[in++];
		while (in < *nbytes && in - count < 127 && input[in] == pixel) in++;
		if (in - count == 1) {	/* No more duplicates. How many non-duplicates were there? */
			while (in < *nbytes && (in - count) < 127 && ((input[in] != input[in-1] || in > 1) && input[in] != input[in-2])) in++;
			while (in < *nbytes && input[in] == input[in-1]) in--;
			output[out++] = (unsigned char)(in - count - 1);
			for (i = count; i < in; i++) output[out++] = input[i];
		}
		else {		/* Write out a runlength */
			output[out++] = (unsigned char)(count - in + 1);
			output[out++] = pixel;
		}
		count = in;
	}

	/* Write end of data marker */
	output[out++] = 128;

	/* Drop the compression when end result is bigger than original */
	if (out > in) {
		PSL_message (PSL, PSL_MSG_INFORMATION, "RLE inflated %d to %d bytes. No compression done.\n", in, out);
		PSL_free (output);
		return (NULL);
	}

	/* Return number of output bytes and output buffer */
	PSL_message (PSL, PSL_MSG_INFORMATION, "RLE compressed %d to %d bytes (%.1f%% savings)\n", in, out, 100.0f*(1.0f-(float)out/in));
	*nbytes = out;
	return (output);
}

static psl_byte_stream_t psl_lzw_putcode (psl_byte_stream_t stream, short int incode) {
	static short int eod = 257;
	static size_t bit_count = 0;
	static size_t bit_buffer = 0;

	/* Add incode to buffer and output 1 or 2 bytes */
	bit_buffer |= (size_t) incode << (32 - stream->depth - bit_count);
	bit_count += stream->depth;
	while (bit_count >= 8) {
		stream->buffer[stream->nbytes] = (unsigned char)(bit_buffer >> 24);
		stream->nbytes++;
		bit_buffer <<= 8;
		bit_count -= 8;
	}
	if (incode == eod) {	/* Flush buffer */
		stream->buffer[stream->nbytes] = (unsigned char)(bit_buffer >> 24);
		stream->nbytes++;
		bit_buffer = 0;
		bit_count = 0;
	}
	return (stream);
}

static unsigned char *psl_lzw_encode (struct PSL_CTRL *PSL, size_t *nbytes, unsigned char *input) {
	/* LZW compress a buffer of nbytes. */

	static int ncode = 4096*256;
	int i, index;
	size_t in = 0;
	static short int clear = 256, eod = 257;
	short int table = 4095;	/* Initial value forces clearing of table on first byte */
	short int bmax = 0, pre, oldpre, ext, *code = NULL;
	psl_byte_stream_t output;
	unsigned char *buffer = NULL;

	i = (int)MAX (512, *nbytes) + 8;	/* Maximum output length */
	output = (psl_byte_stream_t)psl_memory (PSL, NULL, 1U, sizeof (*output));
	output->buffer = PSL_memory (PSL, NULL, i, unsigned char);
	code = PSL_memory (PSL, NULL, ncode, short int);

	output->nbytes = 0;
	output->depth = 9;
	pre = input[in++];

	/* Loop scanning all input bytes. Abort when inflating after processing at least 512 bytes */
	while (in < *nbytes && (output->nbytes < in || output->nbytes < 512)) {
		if (table >= 4095) {	/* Refresh code table */
			output = psl_lzw_putcode (output, clear);
			memset (code, 0, ncode * sizeof(*code));
			table = eod + 1;
			bmax = clear * 2;
			output->depth = 9;
		}

		ext = input[in++];
		oldpre = pre;
		index = (pre << 8) + ext;
		pre = code[index];

		if (pre == 0) {		/* Add new entry to code table */
			code[index] = table;
			table++;
			output = psl_lzw_putcode (output, oldpre);
			pre = ext;
			if (table == bmax) {
				bmax <<= 1;
				output->depth++;
			}
		}
	}

	/* Output last byte and End-of-Data */
	output = psl_lzw_putcode (output, pre);
	output = psl_lzw_putcode (output, eod);

	/* Drop the compression when end result is bigger than original */
	if (output->nbytes > in) {
		PSL_message (PSL, PSL_MSG_INFORMATION, "LZW inflated %d to %d bytes. No compression done.\n", in, output->nbytes);
		PSL_free (code);
		PSL_free (output->buffer);
		PSL_free (output);
		return (NULL);
	}

	/* Return number of output bytes and output buffer; release code table */
	PSL_message (PSL, PSL_MSG_INFORMATION, "LZW compressed %d to %d bytes (%.1f%% savings)\n", in, output->nbytes, 100.0f*(1.0f-(float)output->nbytes/in));
	*nbytes = output->nbytes;
	buffer = output->buffer;
	PSL_free (code);
	PSL_free (output);
	return (buffer);
}

static unsigned char *psl_deflate_encode (struct PSL_CTRL *PSL, size_t *nbytes, unsigned char *input) {
	/* DEFLATE a buffer of nbytes using ZLIB. */
#ifdef HAVE_ZLIB
	const size_t ilen = *nbytes;
	size_t olen = *nbytes - 1; /* Output buffer is 1 smaller than input */
	unsigned char *output;
	int level = PSL->internal.deflate_level == 0 ? Z_DEFAULT_COMPRESSION : PSL->internal.deflate_level; /* Compression level */
	int zstatus;
	z_stream strm;

	/* Initialize zlib for compression */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if (deflateInit (&strm, level) != Z_OK) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Cannot initialize ZLIB stream: %s", strm.msg);
		return NULL;
	}

	output = PSL_memory (PSL, NULL, olen, unsigned char); /* Allocate output buffer */

	strm.avail_in  = (int)ilen;   /* number of bytes in input buffer */
	strm.next_in   = input;       /* input buffer */
	strm.avail_out = (int)olen;   /* number of bytes available in output buffer */
	strm.next_out  = output;      /* output buffer */

	zstatus = deflate (&strm, Z_FINISH); /* deflate whole chunk */
	deflateEnd (&strm);                  /* deallocate zlib memory */

	if (zstatus != Z_STREAM_END) {
		/* "compressed" size is larger or other failure */
		PSL_message (PSL, PSL_MSG_INFORMATION, "Warning: no deflate compression done.\n");
		PSL_free (output);
		return NULL;
	}

	/* Return number of output bytes and output buffer */
	olen = olen - strm.avail_out; /* initial size - size left */
	PSL_message (PSL, PSL_MSG_INFORMATION, "DEFLATE compressed %" PRIuS " to %" PRIuS " bytes (%.1f%% savings at compression level %d)\n",
		ilen, olen, 100.0f*(1.0f-(float)olen/ilen), level == Z_DEFAULT_COMPRESSION ? 6 : level);
	*nbytes = olen;
	return output;

#else /* HAVE_ZLIB */
	/* ZLIB not available */
	PSL_M_unused(nbytes);
	PSL_M_unused(input);
	PSL_message (PSL, PSL_MSG_WARNING, "Cannot DEFLATE because ZLIB is not available.\n");
	return NULL;
#endif /* HAVE_ZLIB */
}

static void psl_stream_dump (struct PSL_CTRL *PSL, unsigned char *buffer, int nx, int ny, int nbits, int compress, int encode, int mask) {
	/* Writes a stream of bytes in ascii85 or hex, performs RGB to CMYK
	 * conversion and compression.
	 * buffer	= stream of bytes
	 * nx, ny	= image dimensions in pixels
	 * nbits	= depth of image pixels in bits
	 * compress	= no (0), rle (1), lzw (2), or deflate (3) compression
	 * encode	= ascii85 (0) or hex (1)
	 * mask		= image (0), imagemask (1), or neither (2)
	 */
	size_t nbytes, i;
	unsigned line_length = 0;
	unsigned char *buffer1 = NULL, *buffer2 = NULL;
	const char *kind_compress[] = {"", "/RunLengthDecode filter", "/LZWDecode filter", "/FlateDecode filter"};
	const char *kind_mask[] = {"image", "imagemask"};

	nx = abs (nx);
	nbytes = ((size_t)nbits * (size_t)nx + 7) / (size_t)8 * (size_t)ny;

	/* Transform RGB stream to CMYK or Gray stream */
	if (PSL->internal.color_mode == PSL_CMYK && nbits == 24)
		buffer1 = psl_cmyk_encode (PSL, &nbytes, buffer);
	else if (PSL->internal.color_mode == PSL_GRAY && nbits == 24)
		buffer1 = psl_gray_encode (PSL, &nbytes, buffer);
	else
		buffer1 = buffer;

	/* Perform selected compression method */
	if (compress == PSL_RLE)
		buffer2 = psl_rle_encode (PSL, &nbytes, buffer1);
	else if (compress == PSL_LZW)
		buffer2 = psl_lzw_encode (PSL, &nbytes, buffer1);
	else if (compress == PSL_DEFLATE)
		buffer2 = psl_deflate_encode (PSL, &nbytes, buffer1);
	else
		buffer2 = NULL;

	if (!buffer2)	{ /* If compression failed, or no compression requested */
		compress = PSL_NONE;
		buffer2 = buffer1;
	}

	/* Output image dictionary */
	if (mask < 2) {
		PSL_command (PSL, "/Width %d /Height %d /BitsPerComponent %d\n", nx, ny, MIN(nbits,8));
		PSL_command (PSL, "   /ImageMatrix [%d 0 0 %d 0 %d] /DataSource currentfile", nx, -ny, ny);
		if (encode == PSL_ASCII85) PSL_command (PSL, " /ASCII85Decode filter");
		if (compress) PSL_command (PSL, " %s", kind_compress[compress]);
		PSL_command (PSL, "\n>> %s\n", kind_mask[mask]);
	}
	if (encode == PSL_ASCII85) {
		/* Convert 4-tuples to ASCII85 5-tuples and write buffer to file */
		psl_a85_encode (PSL, buffer2, nbytes);
	}
	else {
		/* Regular hexadecimal encoding */
		for (i = 0; i < nbytes; i++) {
			PSL_command (PSL, "%02X", buffer2[i]); line_length += 2;
			if (line_length > 95) { PSL_command (PSL, "\n"); line_length = 0; }
		}
	}
	if (mask == 2) PSL_command (PSL, "%s", kind_compress[compress]);

	/* Clear newly created buffers, but maintain original */
	if (buffer2 != buffer1) PSL_free (buffer2);
	if (buffer1 != buffer ) PSL_free (buffer1);
}

static int psl_putfont (struct PSL_CTRL *PSL, double fontsize) {
	if (fontsize == PSL->current.fontsize) return (PSL_NO_ERROR);
	PSL->current.fontsize = fontsize;
	PSL_command (PSL, "%d F%d\n", psl_ip (PSL, fontsize), PSL->current.font_no);
	return (PSL_NO_ERROR);
}

static int psl_encodefont (struct PSL_CTRL *PSL, int font_no) {
	if (PSL->init.encoding == NULL) return (PSL_NO_ERROR);		/* Already have StandardEncoding by default */
	if (PSL->internal.font[font_no].encoded) return (PSL_NO_ERROR);	/* Already re-encoded or should not be re-encoded ever */

	/* Re-encode fonts with Standard+ or ISOLatin1[+] encodings */
	PSL_command (PSL, "PSL_font_encode %d get 0 eq {%s_Encoding /%s /%s PSL_reencode PSL_font_encode %d 1 put} if", font_no, PSL->init.encoding, PSL->internal.font[font_no].name, PSL->internal.font[font_no].name, font_no);
	(PSL->internal.comments) ? PSL_command (PSL, "\t%% Set this font\n") : PSL_command (PSL, "\n");
	PSL->internal.font[font_no].encoded = 1;
	return (PSL_NO_ERROR);
}

/*! . */
static int psl_getfont (struct PSL_CTRL *PSL, char *name) {
	int ret = 0;	/* Return Helvetica if we cannot figure it out */
	char *c = NULL;
	/* Return font number from strings like 33% or Helvetica-Bold% */
	if (!name || !name[0] || name[0] == '%') return (-1);
	if ((c = strchr (name, '%'))) c[0] = '\0';	/* Chop off trailing % */
	if (isdigit ((unsigned char) name[0])) {	/* Starts with number */
		if (!isdigit ((unsigned char) name[strlen(name)-1]))
			ret = -1;	/* Starts with digit, ends with something else: cannot be */
		else
			ret = atoi (name);
		if (ret < 0 || ret >= PSL->internal.N_FONTS) {
			PSL_message (PSL, PSL_MSG_ERROR, "Error: font number %s outside the valid range - reset to 0\n", name);
			ret = 0;
		}
	}
	else {	/* Does not start with number. Try known font name */
		int i;
		char q, *m = NULL;
		if ((m = strchr (name, 0255)))
			q = m[0];
		if (m) m[0] = '-';	/* Temporarily replace hyphen with minus */
		for (i = 0; i < PSL->internal.N_FONTS && strcmp (name, PSL->internal.font[i].name); i++);
		if (i < PSL->internal.N_FONTS)
			ret = i;
		else {
			PSL_message (PSL, PSL_MSG_ERROR, "Error: font %s not recognized - reset to %s\n", name, PSL->internal.font[0].name);
			ret = 0;
		}
		if (m) m[0] = q;	/* Restore hyphen */
	}
	if (c) c[0] = '%';	/* Restore trailing % */
	return (ret);
}

char *psl_prepare_text (struct PSL_CTRL *PSL, char *text) {

/*	Adds escapes for misc parenthesis, brackets etc.
	Will also translate to some European characters such as the @a, @e
	etc escape sequences. Calling function must REMEMBER to free memory
	allocated by string */
	const char *psl_scandcodes[16][5] = {	/* Short-hand conversion for some European characters in both Undefined [0], Standard [1], Standard+ [2], ISOLatin1 [3], and ISOLatin1+ [4] encoding */
		{ "AA", "AA"   , "\\375", "\\305", "\\305"},	/* Aring */
		{ "AE", "\\341", "\\341", "\\306", "\\306"},	/* AE */
		{ "OE", "\\351", "\\351", "\\330", "\\330"},	/* Oslash */
		{ "aa", "aa"   , "\\376", "\\345", "\\345"},	/* aring */
		{ "ae", "\\361", "\\361", "\\346", "\\346"},	/* ae */
		{ "oe", "\\371", "\\371", "\\370", "\\370"},	/* oslash */
		{ "C" , "C"    , "\\201", "\\307", "\\307"},	/* Ccedilla */
		{ "N" , "N"    , "\\204", "\\321", "\\321"},	/* Ntilde */
		{ "U" , "UE"   , "\\335", "\\334", "\\334"},	/* Udieresis */
		{ "c" , "c"    , "\\215", "\\347", "\\347"},	/* ccedilla */
		{ "n" , "n"    , "\\227", "\\361", "\\361"},	/* ntilde */
		{ "ss", "\\373", "\\373", "\\337", "\\337"},	/* germandbls */
		{ "u" , "ue"   , "\\370", "\\374", "\\374"},	/* udieresis */
		{ "i" , "i"    , "\\354", "\\355", "\\355"},	/* iaccute */
		{ "@" , "\\100", "\\100", "\\100", "\\100"},	/* atsign */
		{ "*" , "\\312", "\\217", "\\260", "\\260"}	/* degree */
	};
	char *string = NULL;
	int i = 0, j = 0, font;
	int he = 0;		/* PSL Historical Encoding (if any) */

	if (!text) return NULL;

	psl_encodefont (PSL, PSL->current.font_no);

	if (strcmp ("Standard+", PSL->init.encoding) == 0)
		he = 2;
	else if (strcmp ("Standard", PSL->init.encoding) == 0)
		he = 1;
	else if (strcmp ("ISOLatin1+", PSL->init.encoding) == 0)
		he = 4;
	else if (strcmp ("ISOLatin1", PSL->init.encoding) == 0)
		he = 3;

	string = PSL_memory (PSL, NULL, 2 * PSL_BUFSIZ, char);
	while (text[i]) {
		if (he && text[i] == '@') {
			i++;
			switch (text[i]) {
				case 'A':
					strcat (string, psl_scandcodes[0][he]);
					j += (int)strlen(psl_scandcodes[0][he]); i++;
					break;
				case 'E':
					strcat (string, psl_scandcodes[1][he]);
					j += (int)strlen(psl_scandcodes[1][he]); i++;
					break;
				case 'O':
					strcat (string, psl_scandcodes[2][he]);
					j += (int)strlen(psl_scandcodes[2][he]); i++;
					break;
				case 'a':
					strcat (string, psl_scandcodes[3][he]);
					j += (int)strlen(psl_scandcodes[3][he]); i++;
					break;
				case 'e':
					strcat (string, psl_scandcodes[4][he]);
					j += (int)strlen(psl_scandcodes[4][he]); i++;
					break;
				case 'o':
					strcat (string, psl_scandcodes[5][he]);
					j += (int)strlen(psl_scandcodes[5][he]); i++;
					break;
				case 'C':
					strcat (string, psl_scandcodes[6][he]);
					j += (int)strlen(psl_scandcodes[6][he]); i++;
					break;
				case 'N':
					strcat (string, psl_scandcodes[7][he]);
					j += (int)strlen(psl_scandcodes[7][he]); i++;
					break;
				case 'U':
					strcat (string, psl_scandcodes[8][he]);
					j += (int)strlen(psl_scandcodes[8][he]); i++;
					break;
				case 'c':
					strcat (string, psl_scandcodes[9][he]);
					j += (int)strlen(psl_scandcodes[9][he]); i++;
					break;
				case 'n':
					strcat (string, psl_scandcodes[10][he]);
					j += (int)strlen(psl_scandcodes[10][he]); i++;
					break;
				case 's':
					strcat (string, psl_scandcodes[11][he]);
					j += (int)strlen(psl_scandcodes[11][he]); i++;
					break;
				case 'u':
					strcat (string, psl_scandcodes[12][he]);
					j += (int)strlen(psl_scandcodes[12][he]); i++;
					break;
				case 'i':
					strcat (string, psl_scandcodes[13][he]);
					j += (int)strlen(psl_scandcodes[13][he]); i++;
					break;
				case '@':
					strcat (string, psl_scandcodes[14][he]);
					j += (int)strlen(psl_scandcodes[14][he]); i++;
					break;
				case '.':
					strcat (string, psl_scandcodes[15][he]);
					j += (int)strlen(psl_scandcodes[15][he]); i++;
					break;
				case '%':	/* Font switcher */
					if ((font = psl_getfont (PSL, &text[i+1])) >= 0)
						psl_encodefont (PSL, font);
					string[j++] = '@';
					string[j++] = text[i++];	/* Just copy over the rest */
					while (text[i] != '%') string[j++] = text[i++];
					break;
				case '~':	/* Symbol font toggle */
					psl_encodefont (PSL, PSL_SYMBOL_FONT);
					/* Fall through and place the text? */
				default:
					string[j++] = '@';
					string[j++] = text[i++];
					break;
			}
		}
		else {
			switch (text[i]) {    /* NEED TO BE ESCAPED!!!! for PostScript*/
				case '{':
				case '}':
				case '[':
				case ']':
				case '(':
				case ')':
				case '<':
				case '>':
					if (j > 0 && string[MAX(j-1,0)] == '\\')	/* ALREADY ESCAPED... */
						string[j++] = text[i++];
					else {
						strcat(string, "\\"); j++;
						string[j++] = text[i++];
					}
					break;
				default:
					string[j++] = text[i++];
					break;
			}
		}
	}

	psl_fix_utf8 (PSL, string);

	return (string);
}

static int psl_pattern_cleanup (struct PSL_CTRL *PSL) {
	int image_no, k = 0;

	for (image_no = 0; image_no < PSL_N_PATTERNS * 2; image_no++)
		if (PSL->internal.pattern[image_no].status) k++;
	if (k == 0) return (PSL_NO_ERROR);
	PSL_comment (PSL, "Undefine patterns and images\n");
	for (image_no = 0; image_no < PSL_N_PATTERNS * 2; image_no++) {
		if (PSL->internal.pattern[image_no].status) {
			PSL_command (PSL, "currentdict /image%d undef\n", image_no+1);
			PSL_command (PSL, "currentdict /pattern%d undef\n", image_no+1);
		}
	}
	return (PSL_NO_ERROR);
}

static int psl_patch (struct PSL_CTRL *PSL, double *x, double *y, int np) {
	/* Like PSL_plotpolygon but intended for small polygons (< 20 points).  No checking for
	 * shorter path by calling psl_shorten_path as in PSL_plotpolygon.
	 */

	int ix[20], iy[20], i, n, n1;

	if (np > 20) return (PSL_plotpolygon (PSL, x, y, np));	/* Must call PSL_plotpolygon instead */

	ix[0] = psl_ix (PSL, x[0]);	/* Convert inch to absolute pixel position for start of quadrilateral */
	iy[0] = psl_iy (PSL, y[0]);

	for (i = n = 1, n1 = 0; i < np; i++) {	/* Same but check if new point represent a different pixel */
		ix[n] = psl_ix (PSL, x[i]);
		iy[n] = psl_iy (PSL, y[i]);
		if (ix[n] != ix[n1] || iy[n] != iy[n1]) n++, n1++;
	}
	if (ix[0] == ix[n1] && iy[0] == iy[n1]) n--, n1--;	/* Closepath will do this automatically */

	if (n < 1) return (PSL_NO_POLYGON);	/* 0 points don't make a polygon */

	n1 = --n;
	for (i = n - 1; i >= 0; i--, n--) PSL_command (PSL, "%d %d ", ix[n] - ix[i], iy[n] - iy[i]);
	PSL_command (PSL, "%d %d %d SP\n", n1, ix[0], iy[0]);
	return (PSL_NO_ERROR);
}

static char *psl_getsharepath (struct PSL_CTRL *PSL, const char *subdir, const char *stem, const char *suffix, char *path) {
	/* stem is the name of the file, e.g., PSL_custom_fonts.txt
	 * subdir is an optional subdirectory name in the PSL->internal.SHAREDIR directory.
	 * suffix is an optional suffix to append to name
	 * path is the full path to the file in question
	 * Returns the full pathname if a workable path was found
	 * Looks for file stem in current directory, PSL->internal.USERDIR, and PSL->internal.SHAREDIR[/subdir]
	 */

	/* First look in the current working directory */

	sprintf (path, "%s%s", stem, suffix);
	if (!access (path, R_OK)) return (path);	/* Yes, found it in current directory */

	/* Do not continue when full pathname is given */

#ifdef WIN32
	if (stem[0] == '\\' || stem[1] == ':') return (NULL);
#else
	if (stem[0] == '/') return (NULL);
#endif

	/* Not found, see if there is a file in the user's PSL->internal.USERDIR directory */

	if (PSL->internal.USERDIR) {
		sprintf (path, "%s/%s%s", PSL->internal.USERDIR, stem, suffix);
		if (!access (path, R_OK)) return (path);
        	sprintf (path, "%s/cache/%s%s", PSL->internal.USERDIR, stem, suffix);
        	if (!access (path, R_OK)) return (path);
	}

	/* Try to get file from PSL->internal.SHAREDIR/subdir */

	if (subdir) {
		sprintf (path, "%s/%s/%s%s", PSL->internal.SHAREDIR, subdir, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	/* Finally try file in PSL->internal.SHAREDIR (for backward compatibility) */

	sprintf (path, "%s/%s%s", PSL->internal.SHAREDIR, stem, suffix);
	if (!access (path, R_OK)) return (path);

	return (NULL);	/* No file found, give up */
}

static int psl_place_encoding (struct PSL_CTRL *PSL, const char *encoding) {
	/* Write the specified encoding string to file */
	int k = 0, match = 0;
	while (PSL_ISO_name[k] && (match = strcmp (encoding, PSL_ISO_name[k])) != 0) k++;
	if (match == 0)
		PSL_command (PSL, "%s", PSL_ISO_encoding[k]);
	else {
		PSL_message (PSL, PSL_MSG_ERROR, "Fatal Error: Could not find ISO encoding %s\n", encoding);
		PSL_exit (EXIT_FAILURE);
	}
	return 0;
}

/* psl_bulkcopy copies the given long static string (defined in PSL_strings.h)
 * to the postscript output verbatim or after stripping comments.
 */

#include "PSL_strings.h"	/* Static char copies of the three former include files */

static void psl_bulkcopy (struct PSL_CTRL *PSL, const char *text) {
	char *buf = NULL, *string = NULL;
	int i;

	if (!strcmp (text, "PSL_label"))
		string = strdup (PSL_label_str);
	else if (!strcmp (text, "PSL_prologue"))
		string = strdup (PSL_prologue_str);
	else if (!strcmp (text, "PSL_text"))
		string = strdup (PSL_text_str);

	while ((buf = strsep (&string, "\n")) != NULL) {
		if (PSL->internal.comments) {
			/* We copy every line, including the comments, except those starting '%-' */
			if (buf[0] == '%' && buf[1] == '-') continue;
			PSL_command (PSL, "%s\n", buf);
		}
		else {
			/* Here we remove the comments */
			i = 0;
			while (buf[i] && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')) i++;	/* Find first non-blank character */
			if (!buf[i]) continue;								/* Blank line, skip */
			if (buf[i] == '%' && buf[i+1] != '%') continue;					/* Comment line, skip */
			/* Output this line, but skip trailing comments (while watching for DSC %% comments) */
			/* Find the end of important stuff on the line (i.e., look for start of trailing comments) */
			for (i = 1; buf[i] && !(buf[i] == '%' && buf[i-1] != '%'); i++);
			i--;										/* buf[i] is the last character to be output */
			while (i && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')) i--;		/* Remove white-space prior to the comment */
			buf[++i] = '\0';			/* Add end-line character and print */
			PSL_command (PSL, "%s\n", buf);
		}
	}
	PSL_free (string);
}

static struct PSL_WORD *psl_add_word_part (struct PSL_CTRL *PSL, char *word, int length, int fontno, double fontsize, int sub, int super, int small, int under, int space, double rgb[]) {
	/* For flag: bits 1 and 2 give number of spaces to follow (0, 1, or 2)
	 * bit 3 == 1 means leading TAB
	 * bit 4 == 1 means Composite 1 character
	 * bit 5 == 1 means Composite 2 character
	 * bit 6 == 1 means underline word
	 */

	int i = 0;
	int c;
	int tab = false;
	double fs;
	struct PSL_WORD *new_word = NULL;

	if (!length) length = (int)strlen (word);
	while (word[i] && word[i] == '\t') {	/* Leading tab(s) means indent once */
		tab = true;
		i++;
		length--;
	}

	new_word = PSL_memory (PSL, NULL, 1, struct PSL_WORD);
	new_word->txt = PSL_memory (PSL, NULL, length+1, char);
	fs = fontsize * PSL->internal.dpp;

	strncpy (new_word->txt, &word[i], (size_t)length);
	new_word->font_no = fontno;
	if (small) {	/* Small caps is on */
		new_word->fontsize = (int)lrint (PSL->current.scapssize * fs);
		for (i = 0; new_word->txt[i]; i++) {
			c = (int)new_word->txt[i];
			new_word->txt[i] = (char) toupper (c);
		}
	}
	else if (super) {
		new_word->fontsize = (int)lrint (PSL->current.subsupsize * fs);
		new_word->baseshift = (int)lrint (PSL->current.sup_up[PSL_LC] * fs);
	}
	else if (sub) {
		new_word->fontsize = (int)lrint (PSL->current.subsupsize * fs);
		new_word->baseshift = (int)lrint (-PSL->current.sub_down * fs);
	}
	else
		new_word->fontsize = (int)lrint (fs);

	new_word->flag = space;
	if (tab) new_word->flag |= 4;	/* 3rd bit indicates tab, then add space after word */
	if (under) new_word->flag |= 32;	/* 6rd bit indicates underline */
	PSL_rgb_copy (new_word->rgb, rgb);

	return (new_word);
}

static void psl_freewords (struct PSL_WORD **word, int n_words) {
	/* Free all the words and their texts */
	int k;
	for (k = 0; k < n_words; k++) {
		PSL_free (word[k]->txt);
		PSL_free (word[k]);
	}
}

static int psl_paragraphprocess (struct PSL_CTRL *PSL, double y, double fontsize, char *paragraph) {
	/* Typeset one or more paragraphs.  Separate paragraphs by adding \r to end of last word in a paragraph.
	 * This is a subfunction that simply place all the text attributes on the stack.
	 */
	int n, p, n_scan, last_k = -1, error = 0, old_font, font, after, len, n_alloc_txt;
	int *font_unique = NULL;
	unsigned int i, i1, i0, j, k, n_items, n_font_unique, n_rgb_unique;
	size_t n_alloc, n_words = 0;
	double old_size, last_rgb[4] = {0.0, 0.0, 0.0, 0.0}, rgb[4] = {0.0, 0.0, 0.0, 0.0};
	int sub_on, super_on, scaps_on, symbol_on, font_on, size_on, color_on, under_on, plain_word = false, escape;
	char *c = NULL, *clean = NULL, test_char, **text = NULL, *lastp = NULL, *copy = NULL;
	const char *sep = " ";
	struct PSL_WORD **word = NULL, **rgb_unique = NULL;

	if (fontsize == 0.0) return (PSL_NO_ERROR);	/* Nothing to do if text has zero size */

	sub_on = super_on = scaps_on = symbol_on = font_on = size_on = color_on = under_on = false;

	/* Break input string into words (sorta based on old pstext) */
	n_alloc = PSL_CHUNK;
	text = (char **) PSL_memory (PSL, NULL, n_alloc, char *);
	copy = strdup (paragraph);	/* Need copy since strtok_r will mess with the text */
	c = strtok_r (copy, sep, &lastp);	/* Found first word */
	while (c) {	/* Found another word */
		text[n_words] = strdup (c);
		len = (int)strlen(text[n_words]) - 1;
		if (text[n_words][len] == '\r') {	/* New paragraph */
			text[n_words][len] = '\0';	/* chop off CR */
			n_words++;
			if (n_words == n_alloc) {
				n_alloc <<= 1;
				text = (char **) PSL_memory (PSL, text, n_alloc, char *);
			}
			text[n_words] = strdup ("");	/* This adds an empty string */
		}
		n_words++;
		if (n_words == n_alloc) {
			n_alloc <<= 1;
			text = (char **) PSL_memory (PSL, text, n_alloc, char *);
		}
		c = strtok_r (NULL, sep, &lastp);
	}
	text = (char **) PSL_memory (PSL, text, n_words, char *);
	PSL_free (copy);

	/* Now process the words into pieces we can typeset. */

	n_alloc = PSL_CHUNK;
	old_font = font = PSL->current.font_no;
	old_size = fontsize;
	PSL_rgb_copy (rgb, PSL->current.rgb[PSL_IS_STROKE]);	/* Initial font color is current color */

	word = PSL_memory (PSL, NULL, n_alloc, struct PSL_WORD *);

	for (i = k = 0; i < n_words; i++) {

		clean = psl_prepare_text (PSL, text[i]);	/* Escape special characters and European character shorthands */

		if ((c = strchr (clean, '@')) != NULL) {	/* Found a @ escape command */
			i0 = 0;
			i1 = (int) (c - clean);

			if (i1 > i0)
				word[k++] = psl_add_word_part (PSL, &clean[i0], i1 - i0, font, fontsize, sub_on, super_on, scaps_on, under_on, PSL_NO_SPACE, rgb);
			if (k == n_alloc) {
				n_alloc <<= 1;
				word = PSL_memory (PSL, word, n_alloc, struct PSL_WORD *);
			}

			i1++;	/* Skip the @ */

			while (clean[i1]) {

				escape = (clean[i1-1] == '@');	/* i1 char is an escape argument */
				test_char = (escape) ? clean[i1] : 'A';		/* Only use clean[i1] if it is an escape modifier */
				plain_word = false;

				switch (test_char) {

					case '!':	/* 2 Composite characters */
						i1++;
						if (clean[i1] == '\\') { /* First char is Octal code character */
							word[k++] = psl_add_word_part (PSL, &clean[i1], 4, font, fontsize, sub_on, super_on, scaps_on, under_on, PSL_COMPOSITE_1, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k++] = psl_add_word_part (PSL, &clean[i1], 1, font, fontsize, sub_on, super_on, scaps_on, under_on, PSL_COMPOSITE_1, rgb);
							i1++;
						}
						if (k == n_alloc) {
							n_alloc <<= 1;
							word = PSL_memory (PSL, word, n_alloc, struct PSL_WORD *);
						}
						if (clean[i1] == '\\') { /* 2nd char is Octal code character */
							word[k] = psl_add_word_part (PSL, &clean[i1], 4, font, fontsize, sub_on, super_on, scaps_on, under_on, PSL_COMPOSITE_2, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k] = psl_add_word_part (PSL, &clean[i1], 1, font, fontsize, sub_on, super_on, scaps_on, under_on, PSL_COMPOSITE_2, rgb);
							i1++;
						}
						if (!clean[i1]) word[k]->flag++;	/* New word after this composite */
						k++;
						if (k == n_alloc) {
							n_alloc <<= 1;
							word = PSL_memory (PSL, word, n_alloc, struct PSL_WORD *);
						}
						break;

					case '~':	/* Toggle symbol font */
						i1++;
						symbol_on = !symbol_on;
						font = (font == PSL_SYMBOL_FONT) ? old_font : PSL_SYMBOL_FONT;
						break;

					case '%':	/* Switch font option */
						i1++;
						font_on = !font_on;
						if (clean[i1] == '%') {
							font = old_font;
							i1++;
						}
						else {
							old_font = font;
							font = psl_getfont (PSL, &clean[i1]);
							while (clean[i1] != '%') i1++;
							i1++;
						}
						break;

					case '_':	/* Toggle Underline */
						i1++;
						under_on = !under_on;
						break;

					case '-':	/* Toggle Subscript */
						i1++;
						sub_on = !sub_on;
						break;

					case '+':	/* Toggle Subscript */
						i1++;
						super_on = !super_on;
						break;

					case '#':	/* Toggle Small caps */
						i1++;
						scaps_on = !scaps_on;
						break;

					case ':':	/* Change font size */
						i1++;
						size_on = !size_on;
						if (clean[i1] == ':') {
							fontsize = old_size;
							i1++;
						}
						else {
							fontsize = atof (&clean[i1]);
							while (clean[i1] != ':') i1++;
							i1++;
						}
						break;

					case ';':	/* Change font color */
						i1++;
						color_on = !color_on;
						if (clean[i1] == ';') {
							PSL_rgb_copy (rgb, last_rgb);
							i1++;
						}
						else {
							PSL_rgb_copy (last_rgb, rgb);
							j = i1;
							while (clean[j] != ';') j++;
							clean[j] = 0;
							n_scan = sscanf (&clean[i1], "%lg/%lg/%lg", &rgb[0], &rgb[1], &rgb[2]);
							if (n_scan == 1) {	/* Got gray shade */
								rgb[0] /= 255.0;	/* Normalize to 0-1 range */
								rgb[1] = rgb[2] = rgb[0];
								if (rgb[0] < 0.0 || rgb[0] > 1.0) error++;
							}
							else if (n_scan == 3) {	/* Got r/g/b */
								for (p = 0; p < 3; p++) {
									rgb[p] /= 255.0;	/* Normalize to 0-1 range */
									if (rgb[p] < 0.0 || rgb[p] > 1.0) error++;
								}
							}
							else	/* Got crap */
								error++;

							clean[j] = ';';
							i1 = j + 1;
						}
						break;

					default:	/* Regular text to copy */

						j = i1;
						while (clean[j] && clean[j] != '@') j++;
						after = (clean[j]) ? PSL_NO_SPACE : 1;
						plain_word = true;
						word[k++] = psl_add_word_part (PSL, &clean[i1], j-i1, font, fontsize, sub_on, super_on, scaps_on, under_on, after, rgb);
						if (k == n_alloc) {
							n_alloc <<= 1;
							word = PSL_memory (PSL, word, n_alloc, struct PSL_WORD *);
						}
						i1 = (clean[j]) ? j + 1 : j;
						break;
				}
				while (clean[i1] == '@') i1++;	/* SKip @ character */

			} /* End loop over word with @ in it */

			if (!plain_word && (last_k = k - 1) >= 0) {	/* Allow space if text ends with @ commands only */
				word[last_k]->flag &= 60;
				word[last_k]->flag |= 1;
			}
		}
		else {	/* Plain word, no worries */
			word[k++] = psl_add_word_part (PSL, clean, 0, font, fontsize, sub_on, super_on, scaps_on, under_on, PSL_ONE_SPACE, rgb);
			if (k == n_alloc) {
				n_alloc <<= 1;
				word = PSL_memory (PSL, word, n_alloc, struct PSL_WORD *);
			}
		}

		PSL_free (clean);	/* Reclaim this memory */
		PSL_free (text[i]);	/* Since strdup created it */

	} /* End of word loop */

	if (sub_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Sub-scripting not terminated [%s]\n", paragraph);
	if (super_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Super-scripting not terminated [%s]\n", paragraph);
	if (scaps_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Small-caps not terminated [%s]\n", paragraph);
	if (symbol_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Symbol font change not terminated [%s]\n", paragraph);
	if (size_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font-size change not terminated [%s]\n", paragraph);
	if (color_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font-color change not terminated [%s]\n", paragraph);
	if (under_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Text underline not terminated [%s]\n", paragraph);

	PSL_free (text);	/* Reclaim this memory */
	n_alloc_txt = k;	/* Number of items in word array that might have text allocations */
	k--;			/* Index of last word */
	while (k && !word[k]->txt) k--;	/* Skip any blank lines at end */
	n_items = k + 1;

	for (i0 = 0, i1 = 1 ; i1 < n_items-1; i1++, i0++) {	/* Loop for periods ending sentences and indicate 2 spaces to follow */
		size_t len = strlen(word[i0]->txt);
		if (len > 0 && isupper ((int)word[i1]->txt[0]) && word[i0]->txt[len-1] == '.') {
			word[i0]->flag &= 60;	/* Sets bits 1 & 2 to zero */
			word[i0]->flag |= 2;	/* Specify 2 spaces */
		}
		if (!word[i1]->txt[0]) {	/* No space at end of paragraph */
			word[i0]->flag &= 60;
			word[i1]->flag &= 60;
		}
	}
	if (i1 >= n_items) i1 = n_items - 1;	/* one-word fix */
	word[i1]->flag &= 60;	/* Last word not followed by anything */

	/* Set each word's index of the corresponding unique color entry */

	rgb_unique = PSL_memory (PSL, NULL, n_items, struct PSL_WORD *);
	for (n_rgb_unique = i = 0; i < n_items; i++) {
		for (j = 0; j < n_rgb_unique && !PSL_same_rgb(word[i]->rgb,rgb_unique[j]->rgb); j++) {}
		if (j == n_rgb_unique) rgb_unique[n_rgb_unique++] = word[i];
		word[i]->index = j;
	}

	/* Replace each word's font with the index of the corresponding unique font entry */

	font_unique = PSL_memory (PSL, NULL, n_items, int);
	for (n_font_unique = i = 0; i < n_items; i++) {
		for (j = 0; j < n_font_unique && word[i]->font_no != font_unique[j]; j++) {}
		if (j == n_font_unique) font_unique[n_font_unique++] = word[i]->font_no;
		word[i]->font_no = j;
	}

	/* Time to write out to PS file */

	/* Load PSL_text procedures from file for now */

	if (!PSL->internal.text_init) {
		psl_bulkcopy (PSL, "PSL_text");
		PSL->internal.text_init = true;
	}

	PSL_comment (PSL, "PSL_plotparagraph begin:\n");

	PSL_comment (PSL, "Define array of fonts:\n");
	PSL_command (PSL, "/PSL_fontname\n");
	for (i = 0 ; i < n_font_unique; i++) PSL_command (PSL, "/%s\n", PSL->internal.font[font_unique[i]].name);
	PSL_command (PSL, "%d array astore def\n", n_font_unique);
	PSL_free (font_unique);

	PSL_comment (PSL, "Initialize variables:\n");
	PSL_command (PSL, "/PSL_n %d def\n", n_items);
	PSL_command (PSL, "/PSL_n1 %d def\n", n_items - 1);
	PSL_defunits (PSL, "PSL_y0", y);
	PSL_command (PSL, "/PSL_spaces [() ( ) (  ) ] def\n");
	PSL_command (PSL, "/PSL_lastfn -1 def\n/PSL_lastfz -1 def\n/PSL_lastfc -1 def\n");
	PSL_command (PSL, "/PSL_UL 0 def\n/PSL_show {ashow} def\n");

	PSL_comment (PSL, "Define array of words:\n");
	PSL_command (PSL, "/PSL_word");
	for (i = n = 0 ; i < n_items; i++) {
		PSL_command (PSL, "%c(%s)", (n) ? ' ' : '\n', word[i]->txt);
		n += (int)strlen (word[i]->txt) + 1; if (n >= 60) n = 0;
	}
	PSL_command (PSL, "\n%d array astore def\n", n_items);

	PSL_comment (PSL, "Define array of word font numbers:\n");
	PSL_command (PSL, "/PSL_fnt");
	for (i = 0 ; i < n_items; i++) PSL_command (PSL, "%c%d", (i%25) ? ' ' : '\n', word[i]->font_no);
	PSL_command (PSL, "\n%d array astore def\n", n_items);

	PSL_comment (PSL, "Define array of word fontsizes:\n");
	PSL_command (PSL, "/PSL_size");
	for (i = 0 ; i < n_items; i++) PSL_command (PSL, "%c%d", (i%15) ? ' ' : '\n', word[i]->fontsize);
	PSL_command (PSL, "\n%d array astore def\n", n_items);

	PSL_comment (PSL, "Define array of word spaces to follow:\n");
	PSL_command (PSL, "/PSL_flag");
	for (i = 0 ; i < n_items; i++) PSL_command (PSL, "%c%d", (i%25) ? ' ' : '\n', word[i]->flag);
	PSL_command (PSL, "\n%d array astore def\n", n_items);

	PSL_comment (PSL, "Define array of word baseline shifts:\n");
	PSL_command (PSL, "/PSL_bshift");
	for (i = 0 ; i < n_items; i++) PSL_command (PSL, "%c%d", (i%25) ? ' ' : '\n', word[i]->baseshift);
	PSL_command (PSL, "\n%d array astore def\n", n_items);

	PSL_comment (PSL, "Define array of word colors indices:\n");
	PSL_command (PSL, "/PSL_color");
	for (i = 0 ; i < n_items; i++) PSL_command (PSL, "%c%d", (i%25) ? ' ' : '\n', word[i]->index);
	PSL_command (PSL, "\n%d array astore def\n", n_items);

	PSL_comment (PSL, "Define array of word colors:\n");
	PSL_command (PSL, "/PSL_rgb\n");
	for (i = 0 ; i < n_rgb_unique; i++) PSL_command (PSL, "%.3g %.3g %.3g\n", rgb_unique[i]->rgb[0], rgb_unique[i]->rgb[1], rgb_unique[i]->rgb[2]);
	PSL_command (PSL, "%d array astore def\n", 3 * n_rgb_unique);
	PSL_free (rgb_unique);

	PSL_comment (PSL, "Define array of word widths:\n");
	PSL_command (PSL, "/PSL_width %d array def\n", n_items);
	PSL_command (PSL, "/PSL_max_word_width 0 def\n");
	PSL_command (PSL, "0 1 PSL_n1 {");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Determine word width given the font and fontsize for each word\n" : "\n");
	PSL_command (PSL, "  /i edef");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Loop index i\n" : "\n");
	PSL_command (PSL, "  PSL_size i get PSL_fontname PSL_fnt i get get Y");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Get and set font and size\n" : "\n");
	PSL_command (PSL, "  PSL_width i PSL_word i get stringwidth pop put");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Calculate and store width\n": "\n");
	PSL_command (PSL, "  PSL_width i get PSL_max_word_width gt { /PSL_max_word_width PSL_width i get def} if");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Keep track of widest word\n": "\n");
	PSL_command (PSL, "} for\n");
	PSL_command (PSL, "PSL_max_word_width PSL_parwidth gt { /PSL_parwidth PSL_max_word_width def } if");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Auto-widen paragraph width if widest word exceeds it\n": "\n");

	PSL_comment (PSL, "Define array of word char counts:\n");
	PSL_command (PSL, "/PSL_count %d array def\n", n_items);
	PSL_command (PSL, "0 1 PSL_n1 {PSL_count exch dup PSL_word exch get length put} for\n");

	PSL_comment (PSL, "For composite chars, set width and count to zero for 2nd char:\n");
	PSL_command (PSL, "1 1 PSL_n1 {\n  /k edef\n  PSL_flag k get 16 and 16 eq {\n");
	PSL_command (PSL, "    /k1 k 1 sub def\n    /w1 PSL_width k1 get def\n    /w2 PSL_width k get def\n");
	PSL_command (PSL, "    PSL_width k1 w1 w2 gt {w1} {w2} ifelse put\n    PSL_width k 0 put\n");
	PSL_command (PSL, "    PSL_count k 0 put\n  } if\n} for\n");

	psl_freewords (word, n_alloc_txt);
	PSL_free (word);
	return (sub_on|super_on|scaps_on|symbol_on|font_on|size_on|color_on|under_on);
}

static void psl_get_origin (double xt, double yt, double xr, double yr, double r, double *xo, double *yo, double *b1, double *b2) {
 /* finds origin so that distance is r to the two points given */
	double a0, b0, c0, A, B, C, q, sx1, sx2, sy1, sy2;

	a0 = (xt - xr) / (yr - yt);
	b0 = 0.5 * (xr*xr + yr*yr - xt*xt - yt*yt)/(yr - yt);
	c0 = b0 - yt;
	A = 1 + a0*a0;
	B = 2*(c0*a0 - xt);
	C = xt*xt - r*r + c0*c0;
	q = sqrt (B*B - 4*A*C);
	sx1 = 0.5* (-B + q)/A;
	sx2 = 0.5* (-B - q)/A;
	sy1 = b0 + a0 * sx1;
	sy2 = b0 + a0 * sx2;

	if (hypot (sx1, sy1) < r) {
	    *xo = sx1;
	    *yo = sy1;
	}
	else {
	    *xo = sx2;
	    *yo = sy2;
	}
	*b1 = R2D * atan2 (yr - *yo, xr - *xo);
	*b2 = R2D * atan2 (yt - *yo, xt - *xo);
}

static int psl_mathrightangle (struct PSL_CTRL *PSL, double x, double y, double param[]) {
	/* Called from psl_matharc for the special case of right angle only; no heads involved */
	double size, xx[3], yy[3];

	PSL_comment (PSL, "Start of Math right angle\n");
	PSL_command (PSL, "V %d %d T %lg R\n", psl_ix (PSL, x), psl_iy (PSL, y), param[1]);
	size = param[0] / M_SQRT2;

	xx[0] = xx[1] = size;	xx[2] = 0.0;
	yy[0] = 0.0;	yy[1] = yy[2] = size;
	PSL_plotline (PSL, xx, yy, 3, PSL_MOVE|PSL_STROKE);
	PSL_command (PSL, "U \n");
	PSL_comment (PSL, "End of Math right angle\n");
	return (PSL_NO_ERROR);
}

static int psl_matharc (struct PSL_CTRL *PSL, double x, double y, double param[]) {
	/* psl_matharc draws a mathematical opening angle indicator with center at
	 * (x,y), radius, and start,stop angles.  At the ends we may plot a vector
	 * head that is composed of circular arcs. As a special case we can plot
	 * the straight angle symbol when the angles subtend 90 degrees.
	 *
	 * param must hold up to 8 values:
	 * param[0] = radius, param[1] = angle1, param[2] = angle2,
	 * param[3] = headlength, param[4] = headwidth, param[5] = penwidth(inch)
 	 * param[6] = vector-shape (0-1), param[7] = status bit flags
	 * param[8] = begin head type;	param[9] = end head type)
	 * param[10] = begin trim (degrees);	param[11] = end trim (degrees)
	 * param[12] = head penwidth
	 * add 4 to param[6] if you want to use a straight angle
	 * symbol if the opening is 90.  */

	int i, side[2], heads, outline, fill, sign[2] = {+1, -1};
	unsigned int status, kind[2];
	double head_arc_length, head_half_width, arc_width, da, da_c, xt, yt, sa, ca, sb, cb, r, r2, xr, yr, xl, yl, xo, yo, shape, h_penwidth;
	double angle[2], tangle[2], off[2], A, B, bo1, bo2, xi, yi, bi1, bi2, xv, yv, rshift[2] = {0.0, 0.0}, circ_r, xx[2], yy[2], trim[2];
	char *line[2] = {"N", "P S"}, *dump[2] = {"", "fs"}, *end[2] = {"start", "end"};

	status = (unsigned int)lrint (param[7]);
	if (status & PSL_VEC_MARC90 && fabs (90.0 - fabs (param[2]-param[1])) < 1.0e-8) {	/* Right angle */
		return (psl_mathrightangle (PSL, x, y, param));
	}
	PSL_comment (PSL, "Start of Math arc\n");
	/* Make any adjustments caused by trim */
	trim[PSL_BEGIN] = (status & PSL_VEC_OFF_BEGIN) ? param[10] : 0.0;
	trim[PSL_END]   = (status & PSL_VEC_OFF_END)   ? param[11] : 0.0;
	PSL_command (PSL, "V %d %d T\n", psl_ix (PSL, x), psl_iy (PSL, y));
	kind[PSL_BEGIN] = (unsigned int)lrint (param[8]);
	kind[PSL_END] = (unsigned int)lrint (param[9]);
	r = param[0];				  /* Radius of arc in inch */
	angle[PSL_BEGIN] = param[1] + trim[PSL_BEGIN]; angle[PSL_END] = param[2] - trim[PSL_END]; /* Start/stop angles of arc, possibly adjusted */
	head_arc_length = param[3];		  /* Head length in inch */
	head_half_width = 0.5 * param[4];	  /* Head half-width in inch */
	arc_width = param[5];			  /* Arc width in inch */
	shape = param[6];			  /* Vector head shape (0-1) */
	h_penwidth = param[12];
	heads = PSL_vec_head (status);		  /* 1 = at beginning, 2 = at end, 3 = both */
	outline = ((status & PSL_VEC_OUTLINE) > 0);
	fill = ((status & PSL_VEC_FILL) > 0);
	circ_r = sqrt (head_arc_length * head_half_width / M_PI);	/* Same area as vector head */

	da = head_arc_length * 180.0 / (M_PI * r);	/* Angle corresponding to the arc length */
	da_c = circ_r * 180.0 / (M_PI * r);	/* Angle corresponding to the circle length */

	for (i = 0; i < 2; i++) {	/* Possibly shorten angular arc if arrow heads take up space */
		side[i] = PSL_vec_side (status, i);		  /* -1 = left-only, +1 = right-only, 0 = normal head for this end */
		tangle[i] = angle[i];	/* Angle if no head is present */
		off[i] = (kind[i] == PSL_VEC_ARROW) ? sign[i]*da*(1.0-0.5*shape) : 0.0;		/* Arc length from tip to backstop */
		if ((heads & (i+1)) && side[i] && kind[i] == PSL_VEC_CIRCLE) off[i] -= 0.5 * sign[i] * da_c;
		if (heads & (i+1)) tangle[i] += off[i];	/* Change arc angle by headlength or half-circle arc */
	}
	side[PSL_BEGIN] = -side[PSL_BEGIN];	/* Because of it was initially implemented */
	/* rshift kicks in when we want a half-arrow head.  In that case we don't want it to be
	 * exactly half since the vector line will then stick out 1/2 line thickness.  So we adjust
	 * for this half-thickness by adding/subtracting from the radius accordingly, using r2,
	 * but only if the two heads agree. */
	rshift[PSL_BEGIN] = 0.5 * side[PSL_BEGIN] * arc_width;
	rshift[PSL_END] = 0.5 * side[PSL_END] * arc_width;

	PSL_setlinewidth (PSL, arc_width * PSL_POINTS_PER_INCH);
	PSL_plotarc (PSL, 0.0, 0.0, r, tangle[PSL_BEGIN], tangle[PSL_END], PSL_MOVE | PSL_STROKE);	/* Draw the (possibly shortened) arc */
	if (status & PSL_VEC_MID_FWD) {	/* Want forward-pointing mid-point head instead of at end */
		angle[PSL_END] = 0.5 * (angle[PSL_BEGIN] + angle[PSL_END]);	heads = 2;
		if (kind[PSL_END] == PSL_VEC_ARROW) angle[PSL_END] += 0.5 * da;
		tangle[PSL_END] = angle[PSL_END] + off[PSL_END];
	}
	else if (status & PSL_VEC_MID_BWD) {	/* Want backwards-pointing mid-point head instead of at beginning */
		angle[PSL_BEGIN] = 0.5 * (angle[PSL_BEGIN] + angle[PSL_END]);		heads = 1;
		if (kind[PSL_BEGIN] == PSL_VEC_ARROW) angle[PSL_BEGIN] -= 0.5 * da;
		tangle[PSL_BEGIN] = angle[PSL_BEGIN] + off[PSL_BEGIN];
	}
	if (heads) {	/* Will draw at least one head */
		PSL_setfill (PSL, PSL->current.rgb[PSL_IS_FILL], 1);	/* Set fill for head(s) */
		PSL_command (PSL, "PSL_vecheadpen\n");	/* Switch to vector head pen */
		psl_forcelinewidth (PSL, 2.0 * h_penwidth);	/* Force pen width update; double width due to clipping below */
	}

	for (i = 0; i < 2; i++) {	/* For both ends */
		if ((heads & (i+1)) == 0) continue;	/* No arrow head at this angle */
		PSL_comment (PSL, "Mathangle head at %s\n", end[i]);
		A = D2R * angle[i];	sa = sin (A);	ca = cos (A);
		r2 = r + sign[i] * rshift[i];
		xt = r2 * ca;	yt = r2 * sa;	/* Tip coordinates */
		switch (kind[i]) {
			case PSL_VEC_ARROW:
				B = D2R * (angle[i] + sign[i] * da);	sb = sin (B);	cb = cos (B);
				PSL_command (PSL, "V\n");	/* Do this inside gsave/restore since we are clipping */
				if (side[i] != +sign[i]) {	/* Need right side of arrow head */
					xr = (r2 + head_half_width) * cb;	yr = (r2 + head_half_width) * sb;	/* Outer flank coordinates */
					psl_get_origin (xt, yt, xr, yr, r2, &xo, &yo, &bo1, &bo2);
					if (i == 0 && bo2 > bo1)
						bo2 -= 360.0;
					else if (i == 1 && bo1 > bo2)
						bo1 -= 360.0;

					PSL_plotarc (PSL, xo, yo, r2, bo2, bo1, PSL_MOVE);	/* Draw the arrow arc from tip to outside flank */
					A = D2R * (tangle[i]);	sa = sin (A);	ca = cos (A);
					xv = r2 * ca - xr;	yv = r2 * sa - yr;	/* Back point coordinates */
					PSL_plotpoint (PSL, xv, yv, PSL_REL);		/* Connect to back point */
				}
				else {	/* Draw from tip to center back reduced by shape */
					PSL_plotarc (PSL, 0.0, 0.0, r2, angle[i], tangle[i], PSL_MOVE);
				}
				if (side[i] != -sign[i]) {	/* Need left side of arrow head */
					xl = (r2 - head_half_width) * cb;	yl = (r2 - head_half_width) * sb;	/* Inner flank coordinates */
					psl_get_origin (xt, yt, xl, yl, r2, &xi, &yi, &bi1, &bi2);
					if (i == 0 && bi1 < bi2)
						bi1 += 360.0;
					else if (i == 1 && bi1 > bi2)
						bi1 -= 360.0;
					PSL_plotarc (PSL, xi, yi, r2, bi1, bi2, PSL_DRAW);		/* Draw the arrow arc from tip to outside flank */
				}
				else {	/* Draw from center back reduced by shape to tip */
					PSL_plotarc (PSL, 0.0, 0.0, r2, tangle[i], angle[i], PSL_DRAW);
				}
				PSL_command (PSL, "P clip %s %s U\n", dump[fill], line[outline]);
				break;
			case PSL_VEC_CIRCLE:
				PSL_command (PSL, "V\n");	/* Do this inside gsave/restore since we are clipping */
				if (side[i] == -1)	/* Need left side */
					PSL_plotarc (PSL, xt, yt, circ_r, angle[i]+90.0, angle[i]+270.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				else if (side[i] == +1)	/* Need right side */
					PSL_plotarc (PSL, xt, yt, circ_r, angle[i]-90.0, angle[i]+90.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				else
					PSL_plotarc (PSL, xt, yt, circ_r, 0.0, 360.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				PSL_command (PSL, "P clip %s %s U\n", dump[fill], line[outline]);
				break;
			case PSL_VEC_TERMINAL:
				xt = r * ca;	yt = r * sa;	/* Tip coordinates */
		 		xx[0] = xx[1] = xt;	yy[0] = yy[1] = yt;
				if (side[i] == -1)	{	/* Need left side */
				 	xx[0] = (r-head_half_width) * ca;	yy[0] = (r-head_half_width) * sa;
				}
				else if (side[i] == +1) {	/* Need right side */
				 	xx[1] = (r+head_half_width) * ca;	yy[1] = (r+head_half_width) * sa;
				}
				else {
				 	xx[0] = (r-head_half_width) * ca;	yy[0] = (r-head_half_width) * sa;
				 	xx[1] = (r+head_half_width) * ca;	yy[1] = (r+head_half_width) * sa;
				}
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE);	/* Set up path */
				break;
		}
	}

	PSL_command (PSL, "U \n");
	PSL_comment (PSL, "End of Math arc\n");
	return (PSL_NO_ERROR);
}

static int psl_search_userimages (struct PSL_CTRL *PSL, char *imagefile) {
	int i = 0;
	if (imagefile == NULL) return -1;
	while (i < PSL->internal.n_userimages) {
		if (!strcmp (PSL->internal.user_image[i], imagefile))	/* Yes, found it */
			return (i);
		i++;	/* No, go to next */
	}
	return -1;	/* Not found */
}

static int psl_pattern_init (struct PSL_CTRL *PSL, int image_no, char *imagefile, unsigned char *image, unsigned int width, unsigned int height, unsigned int depth) {
	int k, i;
	unsigned char *picture = NULL;
	/* image_no is 1-90 (PSL_N_PATTERNS) if a standard PSL pattern, else we examine imagefile.
	 * User images are numbered PSL_N_PATTERNS+1,2,3 etc. */
	k = image_no - 1;	/* Array index */
	if ((image_no > 0 && image_no <= PSL_N_PATTERNS)) {	/* Premade pattern yet not used, assign settings */
		if (PSL->internal.pattern[k].status) return (image_no);	/* Already initialized this pattern once, just return */
		picture = PSL_pattern[k];
	}
	else {	/* User image, check to see if already used */
		if (imagefile == NULL) {
			PSL_message (PSL, PSL_MSG_ERROR, "Error: Gave NULL as imagefile name\n");
			PSL_exit (EXIT_FAILURE);
		}
		i = psl_search_userimages (PSL, imagefile);	/* i = 0 is the first user image */
		if (i >= 0) return (PSL_N_PATTERNS + i + 1);	/* Already registered, just return number */
		if (PSL->internal.n_userimages > (PSL_N_PATTERNS-1)) {
			PSL_message (PSL, PSL_MSG_ERROR, "Error: Already maintaining %d user images and cannot accept any more\n", PSL->internal.n_userimages+1);
			PSL_exit (EXIT_FAILURE);
		}
		/* Must initialize a previously unused image */
		PSL->internal.user_image[PSL->internal.n_userimages] = PSL_memory (PSL, NULL, strlen (imagefile)+1, char);
		strcpy (PSL->internal.user_image[PSL->internal.n_userimages], imagefile);
		PSL->internal.n_userimages++;
		image_no = PSL_N_PATTERNS + PSL->internal.n_userimages;
		k = image_no - 1;	/* Array index */
		picture = image;
	}

	/* Store size, depth and bogus DPI setting */
	PSL->internal.pattern[k].nx = width;
	PSL->internal.pattern[k].ny = height;
	PSL->internal.pattern[k].depth = depth;
	PSL->internal.pattern[k].status = 1;
	PSL->internal.pattern[k].dpi = -999;

	PSL_comment (PSL, "Define pattern %d\n", image_no);

	PSL_command (PSL, "/image%d {<~\n", image_no);
	psl_stream_dump (PSL, picture, PSL->internal.pattern[k].nx, PSL->internal.pattern[k].ny, PSL->internal.pattern[k].depth, PSL->internal.compress, PSL_ASCII85, 2);
	PSL_command (PSL, "} def\n");

	return (image_no);
}

#ifdef PSL_WITH_GMT4_SUPPORT
/* This code is included so we may offer backwards compatibility with GMT 4 old-school
 * polygon vectors.  It is not documented and should not be used by non-GMT developers.
 */
void psl_vector_v4 (struct PSL_CTRL *PSL, double x, double y, double param[], double rgb[], int outline)
{
	/* Old GMT4 vector symbol:
	 * param[0] = xtip;
	 * param[1] = ytip;
	 * param[2] = tailwidth;
	 * param[3] = headlength;
	 * param[4] = headwidth;
	 * param[5] = headshape;
	 * Will make sure that arrow has a finite width in PS coordinates */

	double angle, xtail, ytail, xtip, ytip, tailwidth, headlength, headwidth, headshape;
	int w2, length, hw, hl, hl2, hw2, l2;

	xtail = x;	ytail = y;	xtip = param[0];	ytip = param[1];
	length = psl_iz (PSL, hypot (xtail-xtip, ytail-ytip));	/* Vector length in PS units */
	if (length == 0) return;	/* NULL vector */

	tailwidth  = param[2];
	headlength = param[3];
	headwidth  = param[4];
	headshape  = param[5];
	if (outline & 8)
		PSL_setfill (PSL, rgb, outline - 8);
	else
		PSL_setfill (PSL, rgb, outline);
	angle = atan2 ((ytip-ytail),(xtip-xtail)) * R2D;			/* Angle vector makes with horizontal, in radians */
	PSL_command (PSL, "V %d %d T ", psl_ix (PSL, xtail), psl_ix (PSL, ytail));	/* Temporarily set tail point the local origin (0, 0) */
	if (angle != 0.0) PSL_command (PSL, "%g R ", angle);		/* Rotate so vector is horizontal in local coordinate system */
	w2 = psl_ix (PSL, 0.5 * tailwidth);	if (w2 == 0) w2 = 1;	/* Half-width of vector tail */
	hw = psl_ix (PSL, headwidth);	if (hw == 0) hw = 1;		/* Width of vector head */
	hl = psl_ix (PSL, headlength);								/* Length of vector head */
	hl2 = psl_ix (PSL, 0.5 * headshape * headlength);			/* Cut-in distance due to slanted back-side of arrow head */
	hw2 = hw - w2;		/* Distance from tail side to head side (vertically) */
	if (outline & 8) {	/* Double-headed vector */
		l2 = length - 2 * hl + 2 * hl2;							/* Inside length between start of heads */
		PSL_command (PSL, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d Sv U\n",
				hl2, hw2, -l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -hl2, hw2, hl, -hw);
	}
	else {			/* Single-headed vector */
		l2 = length - hl + hl2;								/* Length from tail to start of slanted head */
		PSL_command (PSL, "%d %d %d %d %d %d %d %d %d %d %d SV U\n",
			-l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -w2);
	}
}
#endif

#define FIN_SLANT_COS	0.707106781187	/* I.e., 45 degrees slant */
#define FIN_LENGTH_SCALE	0.66666667	/* 2/3 the length of the vector */
#define FIN_HEIGHT_SCALE	0.5	/* 1/2 the width of the vector */

static int psl_vector (struct PSL_CTRL *PSL, double x, double y, double param[]) {
	/* Will make sure that arrow has a finite width in PS coordinates.
	 * param must hold up to 12 values:
	 * param[0] = xtip;		param[1] = ytip;
	 * param[2] = tailwidth;	param[3] = headlength;	param[4] = headwidth;
	 * param[5] = headshape;	param[6] = status bit flags
	 * param[7] = begin head type;	param[8] = end head type
	 * param[9] = begin trim value;	param[10] = end trim value.
	 * param[11] = head penwidth
	 */

	double angle, xtip, ytip, r, s, tailwidth, headlength, headwidth, headshape, length_inch;
	double xx[6], yy[6], xc[3], yc[3], off[2], yshift[2], trim[2], xp = 0.0, h_penwidth;
	int length, asymmetry[2], n, heads, outline, fill, status;
	unsigned int kind[2];
	char *line[2] = {"N", "P S"}, *dump[2] = {"", "fs"};

	xtip = param[0];	ytip = param[1];
	length_inch = hypot (x-xtip, y-ytip);					/* Vector length in inches */
	length = psl_iz (PSL, length_inch);					/* Vector length in PS units */
	if (length == 0) return (PSL_NO_ERROR);					/* NULL vector */
	angle = atan2 (ytip-y, xtip-x) * R2D;					/* Angle vector makes with horizontal, in degrees */
	status = lrint (param[6]);
	h_penwidth = param[11];
	/* Make any adjustments caused by trim */
	trim[PSL_BEGIN] = (status & PSL_VEC_OFF_BEGIN) ? param[9]  : 0.0;
	trim[PSL_END]   = (status & PSL_VEC_OFF_END)   ? param[10] : 0.0;
	if (fabs (angle) == 90.0) {	/* Vertical segment; only adjust y coordinates */
		y += copysign (trim[PSL_BEGIN], angle);	ytip -= copysign (trim[PSL_END], angle);
	}
	else {	/* General case, use trig */
		double s, c, a = angle * D2R;
		s = sin (a);		c = cos (a);
		x += c * trim[PSL_BEGIN];	y += s * trim[PSL_BEGIN];
		xtip -= c * trim[PSL_END];	ytip -= s * trim[PSL_END];
	}
	length_inch = hypot (x-xtip, y-ytip);					/* Recalculate vector length in inches */
	length = psl_iz (PSL, length_inch);					/* Vector length in PS units */
	if (length == 0) return (PSL_NO_ERROR);					/* NULL vector */
	tailwidth = param[2];
	headlength = param[3];	headwidth = 0.5 * param[4];	headshape = param[5];
	kind[PSL_BEGIN] = (unsigned int)lrint (param[7]);
	kind[PSL_END] = (unsigned int)lrint (param[8]);
	off[PSL_BEGIN] = (kind[PSL_BEGIN] == PSL_VEC_ARROW) ? 0.5 * (2.0 - headshape) * headlength : 0.0;
	off[PSL_END] = (kind[PSL_END] == PSL_VEC_ARROW) ? 0.5 * (2.0 - headshape) * headlength : 0.0;
	if (kind[PSL_BEGIN] == PSL_VEC_ARROW_PLAIN) off[PSL_BEGIN] = 0.5 * tailwidth *  headlength / headwidth;
	else if (kind[PSL_BEGIN] == PSL_VEC_TAIL) off[PSL_BEGIN] = FIN_SLANT_COS * headwidth + FIN_LENGTH_SCALE * headlength - tailwidth;
	if (kind[PSL_END] == PSL_VEC_ARROW_PLAIN) off[PSL_END] = 0.5 * tailwidth *  headlength / headwidth;
	else if (kind[PSL_END] == PSL_VEC_TAIL) off[PSL_END] = FIN_SLANT_COS * headwidth + FIN_LENGTH_SCALE * headlength - tailwidth;
	heads = PSL_vec_head (status);		  /* 1 = at beginning, 2 = at end, 3 = both */
	PSL_setlinewidth (PSL, tailwidth * PSL_POINTS_PER_INCH);	/* Inherits color from current pen */
	outline = ((status & PSL_VEC_OUTLINE) > 0);
	fill = ((status & PSL_VEC_FILL) > 0);
	asymmetry[PSL_BEGIN] = -PSL_vec_side (status, 0);	  /* -1 = left-only, +1 = right-only, 0 = normal head at beginning */
	asymmetry[PSL_END] = PSL_vec_side (status, 1);		  /* -1 = left-only, +1 = right-only, 0 = normal head at beginning */
	r = sqrt (headlength * headwidth / M_PI);	/* Same circle area as vector head */
	s = sqrt (headlength * headwidth)/2;		/* Same square 	area as vector head */
	PSL_comment (PSL, "Start of Cartesian vector\n");
	PSL_command (PSL, "V %d %d T ", psl_ix (PSL, x), psl_iy (PSL, y));	/* Temporarily set tail point the local origin (0, 0) */
	if (angle != 0.0) PSL_command (PSL, "%g R\n", angle);			/* Rotate so vector is horizontal in local coordinate system */
	/* Make any adjustments caused by trim */
	xx[0] = (heads & 1) ? off[PSL_BEGIN] : 0.0;
	xx[1] = (heads & 2) ? length_inch - off[PSL_END] : length_inch;
	if (heads & 1 && asymmetry[PSL_BEGIN] && kind[PSL_BEGIN] == PSL_VEC_CIRCLE) xx[0] = -r;
	if (heads & 2 && asymmetry[PSL_END] && kind[PSL_END] == PSL_VEC_CIRCLE) xx[1] += r;
	if (xx[1] > xx[0]) PSL_plotsegment (PSL, xx[0], 0.0, xx[1], 0.0);		/* Draw vector line body unless head length equals or exceeds total length */

	if (status & PSL_VEC_MID_FWD) {	/* Want forward-pointing mid-point head instead of at end */
		xp = -0.5 * length_inch;	heads = 2;
		if (kind[PSL_END] == PSL_VEC_ARROW) xp += 0.5 * headlength;
	}
	else if (status & PSL_VEC_MID_BWD) {	/* Want backwards-pointing mid-point head instead of at beginning */
		xp = 0.5 * length_inch;	heads = 1;
		if (kind[PSL_BEGIN] == PSL_VEC_ARROW) xp -= 0.5 * headlength;
	}
	if (heads == 0) {	/* No heads requested */
		PSL_command (PSL, "U\n");
		PSL_comment (PSL, "End of Cartesian vector\n");
		return (PSL_NO_ERROR);
	}

	/* Must switch asymmetry for start head since implemented backwards */
	yshift[PSL_BEGIN] = 0.5 * asymmetry[PSL_BEGIN] * tailwidth;
	yshift[PSL_END] = 0.5 * asymmetry[PSL_END] * tailwidth;

	if (heads & 1) {	/* Need head at beginning, pointing backwards */
		double f = (kind[PSL_BEGIN] == PSL_VEC_ARROW_PLAIN) ? 4.0 : 2.0;
		PSL_comment (PSL, "Cartesian vector head at start\n");
		PSL_command (PSL, "PSL_vecheadpen\n");		/* Switch to vector head pen */
		psl_forcelinewidth (PSL, f * h_penwidth);	/* Force pen width update */
		switch (kind[PSL_BEGIN]) {
			case PSL_VEC_ARROW:
				xx[0] = xp; yy[0] = -yshift[PSL_BEGIN];	n = 1;	/* Vector tip */
				if (asymmetry[PSL_BEGIN] != +1) {	/* Need left side */
					xx[n] = xp + headlength; yy[n++] = -headwidth;
				}
				if (asymmetry[PSL_BEGIN] || headshape != 0.0) {	/* Need center back of head */
					xx[n] = xp + 0.5 * (2.0 - headshape) * headlength; yy[n++] = -yshift[PSL_BEGIN];
				}
				if (asymmetry[PSL_BEGIN] != -1) {	/* Need right side */
					xx[n] = xp + headlength; yy[n++] = headwidth;
				}
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE);	/* Set up path */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				break;
			case PSL_VEC_ARROW_PLAIN:
				/* Must set up clip path (xc,yc) to ensure tip is at end of vector, AND double
				 * the pen thickness since half will be clipped. */
				n = 0;
				xc[0] = xp + headlength; yc[0] = -headwidth;
				if (asymmetry[PSL_BEGIN] != +1) {	/* Need left side */
					xx[n] = xp + headlength; yy[n++] = -headwidth;
				}
				xx[n] = xp; yy[n++] = -yshift[PSL_BEGIN];	/* Vector tip */
				xc[1] = xp; yc[1] = -yshift[PSL_BEGIN];	/* Vector tip */
				if (asymmetry[PSL_BEGIN] != -1) {	/* Need right side */
					xx[n] = xp + headlength; yy[n++] = headwidth;
				}
				xc[2] = xp + headlength; yc[2] = headwidth;
				PSL_command (PSL, "V "); /* Place under gsave/grestore since changing pen */
				PSL_plotline (PSL, xc, yc, 3, PSL_MOVE);	/* Set up clip path */
				PSL_command (PSL, "P clip N ");
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE|PSL_STROKE);	/* Plot arrow head */
				//PSL_setlinewidth (PSL, tailwidth * PSL_POINTS_PER_INCH);
				PSL_command (PSL, "U\n");
				break;
			case PSL_VEC_TAIL:
				xx[0] = xp + tailwidth + off[PSL_BEGIN]; yy[0] = -yshift[PSL_BEGIN];	n = 1;	/* Vector tip */
				if (asymmetry[PSL_BEGIN] != +1) {	/* Need left side */
					xx[n] = xp + tailwidth - FIN_SLANT_COS * headwidth + off[PSL_BEGIN]; yy[n++] = -FIN_HEIGHT_SCALE * headwidth;
					xx[n] = xx[n-1] - FIN_LENGTH_SCALE * headlength; yy[n++] = -FIN_HEIGHT_SCALE * headwidth;
				}
				xx[n] = xp + tailwidth - FIN_LENGTH_SCALE * headlength + off[PSL_BEGIN]; yy[n++] = -yshift[PSL_BEGIN];
				if (asymmetry[PSL_BEGIN] != -1) {	/* Need right side */
					xx[n] = xp + tailwidth - FIN_SLANT_COS * headwidth - FIN_LENGTH_SCALE * headlength + off[PSL_BEGIN]; yy[n++] = FIN_HEIGHT_SCALE * headwidth;
					xx[n] = xx[n-1] + FIN_LENGTH_SCALE * headlength; yy[n++] = FIN_HEIGHT_SCALE * headwidth;
				}
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE);	/* Set up path */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				if (asymmetry[PSL_BEGIN] == 0) {	/* Draw feather center */
					PSL_command (PSL, "V 0 W ");
					xx[1] = xp + tailwidth - headlength + off[PSL_BEGIN]; yy[1] = -yshift[PSL_BEGIN];
					PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[1]);				/* Draw vector line body */
					PSL_command (PSL, "U\n");
				}
				break;
			case PSL_VEC_TAIL_PLAIN:
				n = 0;
				if (asymmetry[PSL_BEGIN] != +1) {	/* Need left side */
					xx[n] = xp - headlength; yy[n++] = -headwidth;
				}
				xx[n] = xp; yy[n++] = -yshift[PSL_BEGIN];	/* Vector tip */
				if (asymmetry[PSL_BEGIN] != -1) {	/* Need right side */
					xx[n] = xp - headlength; yy[n++] = headwidth;
				}
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE|PSL_STROKE);	/* Set up path */
				break;
			case PSL_VEC_CIRCLE:
				if (asymmetry[PSL_BEGIN] == -1)	/* Need left side */
					PSL_plotarc (PSL, xp, 0.0, r, 0.0, 180.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				else if (asymmetry[PSL_BEGIN] == +1)	/* Need right side */
					PSL_plotarc (PSL, xp, 0.0, r, 180.0, 360.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				else
					PSL_plotarc (PSL, xp, 0.0, r, 0.0, 360.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				break;
			case PSL_VEC_SQUARE:
				xx[0] = xx[3] = xp - s;	xx[1] = xx[2] = xp + s;
				if (asymmetry[PSL_BEGIN] == -1)	{	/* Left side */
					yy[0] = yy[1] = s;
					yy[2] = yy[3] = 0.0;
				}
				else if (asymmetry[PSL_BEGIN] == +1) {	/* Right side */
					yy[0] = yy[1] = 0.0;
					yy[2] = yy[3] = -s;
				}
				else {
					yy[0] = yy[1] = +s;
					yy[2] = yy[3] = -s;
				}
				PSL_plotline (PSL, xx, yy, 4, PSL_MOVE);	/* Set up path */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				break;
			case PSL_VEC_TERMINAL:
				xx[0] = xx[1] = xp;	yy[0] = yy[1] = 0.0;	/* Terminal line */
				if (asymmetry[PSL_BEGIN] == -1)	/* Left side */
					yy[1] = headwidth;
				else if (asymmetry[PSL_BEGIN] == +1)	/* Right side */
					yy[1] = -headwidth;
				else {
					yy[0] = -headwidth;
					yy[1] = +headwidth;
				}
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE);	/* Set up path */
				break;
		}
	}
	PSL_command (PSL, "U\n");
	if (heads & 2) {	/* Need head at end, pointing forwards */
		double f = (kind[PSL_END] == PSL_VEC_ARROW_PLAIN) ? 4.0 : 2.0;
		PSL_comment (PSL, "Cartesian vector head at end\n");
		PSL_command (PSL, "V %d %d T ", psl_ix (PSL, xtip), psl_iy (PSL, ytip));	/* Temporarily set head point the local origin (0, 0) */
		if (angle != 0.0) PSL_command (PSL, "%g R\n", angle);			/* Rotate so vector is horizontal in local coordinate system */
		PSL_command (PSL, "PSL_vecheadpen\n");		/* Switch to vector head pen */
		psl_forcelinewidth (PSL, f * h_penwidth);	/* Force pen width update */
		switch (kind[PSL_END]) {
			case PSL_VEC_ARROW:
				xx[0] = xp; yy[0] = yshift[PSL_END];	n = 1;	/* Vector tip */
				if (asymmetry[PSL_END] != +1) {	/* Need left side */
					xx[n] = xp - headlength; yy[n++] = headwidth;
				}
				if (asymmetry[PSL_END] || headshape != 0.0) {	/* Need center back of head */
					xx[n] = xp -0.5 * (2.0 - headshape) * headlength; yy[n++] = yshift[PSL_END];
				}
				if (asymmetry[PSL_END] != -1) {	/* Need right side */
					xx[n] = xp - headlength; yy[n++] = -headwidth;
				}
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE);	/* Set up path */
				PSL_command (PSL, "P clip %s %s \n", dump[fill], line[outline]);
				break;	/* Finalize, then reset outline parameter */
			case PSL_VEC_ARROW_PLAIN:
				n = 0;
				xc[0] = xp - headlength; yc[0] = -headwidth;
				if (asymmetry[PSL_END] != +1) {	/* Need left side */
					xx[n] = xp - headlength; yy[n++] = -headwidth;
				}
				xx[n] = xp; yy[n++] = -yshift[PSL_END];	/* Vector tip */
				xc[1] = xp; yc[1] = -yshift[PSL_END];	/* Vector tip */
				if (asymmetry[PSL_END] != -1) {	/* Need right side */
					xx[n] = xp - headlength; yy[n++] = headwidth;
				}
				xc[2] = xp - headlength; yc[2] = headwidth;
				PSL_command (PSL, "V "); /* Place under gsave/grestore since changing pen */
				PSL_plotline (PSL, xc, yc, 3, PSL_MOVE);	/* Set up clip path */
				PSL_command (PSL, "P clip N ");
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE|PSL_STROKE);	/* Plot arrow head */
				//PSL_setlinewidth (PSL, tailwidth * PSL_POINTS_PER_INCH);
				PSL_command (PSL, "U\n");
				break;
			case PSL_VEC_TAIL:
				xx[0] = xp - tailwidth - off[PSL_END]; yy[0] = -yshift[PSL_END];	n = 1;	/* Vector tip */
				if (asymmetry[PSL_END] != +1) {	/* Need left side */
					xx[n] = xp - tailwidth + FIN_SLANT_COS * headwidth - off[PSL_END]; yy[n++] = -FIN_HEIGHT_SCALE * headwidth;
					xx[n] = xx[n-1] + FIN_LENGTH_SCALE * headlength; yy[n++] = -FIN_HEIGHT_SCALE * headwidth;
				}
				xx[n] = xp - tailwidth + FIN_LENGTH_SCALE * headlength - off[PSL_END]; yy[n++] = -yshift[PSL_END];
				if (asymmetry[PSL_END] != -1) {	/* Need right side */
					xx[n] = xp - tailwidth + FIN_SLANT_COS * headwidth + FIN_LENGTH_SCALE * headlength - off[PSL_END]; yy[n++] = FIN_HEIGHT_SCALE * headwidth;
					xx[n] = xx[n-1] - FIN_LENGTH_SCALE * headlength; yy[n++] = FIN_HEIGHT_SCALE * headwidth;
				}
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE);	/* Set up path */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				if (asymmetry[PSL_END] == 0) {	/* Draw feather center */
					PSL_command (PSL, "V 0 W ");
					xx[1] = xp - tailwidth + headlength - off[PSL_END]; yy[1] = -yshift[PSL_END];
					PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[1]);				/* Draw vector line body */
					PSL_command (PSL, "U\n");
				}
				break;
			case PSL_VEC_TAIL_PLAIN:
				n = 0;
				if (asymmetry[PSL_END] != +1) {	/* Need left side */
					xx[n] = xp + headlength; yy[n++] = -headwidth;
				}
				xx[n] = xp; yy[n++] = -yshift[PSL_END];	/* Vector tip */
				if (asymmetry[PSL_END] != -1) {	/* Need right side */
					xx[n] = xp + headlength; yy[n++] = headwidth;
				}
				PSL_plotline (PSL, xx, yy, n, PSL_MOVE|PSL_STROKE);	/* Set up path */
				break;
			case PSL_VEC_CIRCLE:
				if (asymmetry[PSL_END] == -1)	/* Need left side */
					PSL_plotarc (PSL, xp, 0.0, r, 0.0, 180.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				else if (asymmetry[PSL_END] == +1)	/* Need right side */
					PSL_plotarc (PSL, xp, 0.0, r, 180.0, 360.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				else
					PSL_plotarc (PSL, xp, 0.0, r, 0.0, 360.0, PSL_MOVE);	/* Draw the (possibly shortened) arc */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				break;
			case PSL_VEC_SQUARE:
				xx[0] = xx[3] = xp - s;	xx[1] = xx[2] = xp + s;
				if (asymmetry[PSL_END] == -1)	{	/* Left side */
					yy[0] = yy[1] = s;
					yy[2] = yy[3] = 0.0;
				}
				else if (asymmetry[PSL_END] == +1) {	/* Right side */
					yy[0] = yy[1] = 0.0;
					yy[2] = yy[3] = -s;
				}
				else {
					yy[0] = yy[1] = +s;
					yy[2] = yy[3] = -s;
				}
				PSL_plotline (PSL, xx, yy, 4, PSL_MOVE);	/* Set up path */
				PSL_command (PSL, "P clip %s %s ", dump[fill], line[outline]);
				break;
			case PSL_VEC_TERMINAL:
				xx[0] = xx[1] = xp;	yy[0] = yy[1] = 0.0;	/* Terminal line */
				if (asymmetry[PSL_END] == -1)	/* Left side */
					yy[1] = headwidth;
				else if (asymmetry[PSL_END] == +1)	/* Right side */
					yy[1] = -headwidth;
				else {
					yy[0] = -headwidth;
					yy[1] = +headwidth;
				}
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE|PSL_STROKE);	/* Set up path */
				break;
		}
		PSL_command (PSL, "U\n");
	}
	PSL_comment (PSL, "End of Cartesian vector\n");
	return (PSL_NO_ERROR);
}

static int psl_wedge (struct PSL_CTRL *PSL, double x, double y, double param[]) {
	/* Takes care of plotting a wedge.
	 * param may hold up to 11 values; only 8 used here.
	 * param[0] = radius;
	 * param[1] = start angle;	param[2] = end angle;
	 * param[3] = status bit flags;	param[4] = inner_radius [0]
	 * param[5] = dr [0];		param[6] = da [0]
	 * param[7] = do_fill (1) | do_outline (2)
	 */

	double xx[3], yy[3];
	int status = lrint (param[3]), flags = lrint (param[7]);
	bool windshield = (param[4] > 0.0);	/* Flag that we have an inner-tube */
	bool fill = flags & 1, outline = flags & 2;

	if (status == 0 && !windshield) {	/* Good old plain pie wedge */
		PSL_command (PSL, "%d %g %g %d %d Sw\n", psl_iz (PSL, param[0]), param[1], param[2], psl_ix (PSL, x), psl_iy (PSL, y));
		return (PSL_NO_ERROR);
	}
	/* Somewhat more involved */
	if (fill) {	/* Paint wedge given fill first but not outline (if desired) */
		if (windshield)
			PSL_command (PSL, "V %d %d T 0 0 %d %g %g arc 0 0 %d %g %g arcn P fs U\n", psl_ix (PSL, x), psl_iy (PSL, y),
				psl_iz (PSL, param[0]), param[1], param[2], psl_iz (PSL, param[4]), param[2], param[1]);
		else
			PSL_command (PSL, "%d %g %g %d %d 2 copy M 5 2 roll arc fs\n", psl_iz (PSL, param[0]), param[1], param[2], psl_ix (PSL, x), psl_iy (PSL, y));
	}
	/* Next, if spiderweb is desired we need to set up a save/restore section and change the pen to PSL_spiderpen */
	if (status) PSL_command (PSL, "V PSL_spiderpen\n");
	if (status & 1) {	/* Draw one or more arcs */
		if (param[5] > 0.0) {	/* Array of arcs requested */
			double r = (windshield) ? ceil (param[4] / param[5]) * param[5] : param[5];	/* Either start at first arc inside windshield or the first zero-length arc of wedge */
			while (r <= (param[0]+PSL_SMALL)) {
				PSL_plotarc (PSL, x, y, r, param[1], param[2], PSL_MOVE | PSL_STROKE);	/* Draw the arcs */
				r += param[5];	/* Go to next radial distance */
			}
		}
		else {	/* Just draw outer and possibly inner arcs */
			PSL_plotarc (PSL, x, y, param[0], param[1], param[2], PSL_MOVE | PSL_STROKE);	/* Draw the outer arc */
			if (windshield)	/* Draw the inner arc */
				PSL_plotarc (PSL, x, y, param[4], param[1], param[2], PSL_MOVE | PSL_STROKE);
		}
	}
	if (status & 2) {	/* Draw one or more radial lines */
		if (param[6] > 0.0) {	/* Array of lines requestedii */
			double a = ceil (param[1] / param[6]) * param[6];	/* First angle of desired multiple inside range */
			while (a <= (param[2]+PSL_SMALL)) {
				xx[0] = x + param[4] * cos (D2R * a);	yy[0] = y + param[4] * sin (D2R * a);
				xx[1] = x + param[0] * cos (D2R * a);	yy[1] = y + param[0] * sin (D2R * a);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE+PSL_STROKE);	/* Plot radial line */
				a += param[6];	/* Go to next angle */
			}
		}
		else {	/* Just draw the start and stop radii */
			if (windshield) {	/* These are two separate lines not connecting */
				xx[0] = x + param[4] * cos (D2R * param[1]);	yy[0] = y + param[4] * sin (D2R * param[1]);
				xx[1] = x + param[0] * cos (D2R * param[1]);	yy[1] = y + param[0] * sin (D2R * param[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE+PSL_STROKE);	/* Plot jaw */
				xx[0] = x + param[4] * cos (D2R * param[2]);	yy[0] = y + param[4] * sin (D2R * param[2]);
				xx[1] = x + param[0] * cos (D2R * param[2]);	yy[1] = y + param[0] * sin (D2R * param[2]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE+PSL_STROKE);	/* Plot jaw */
			}
			else {	/* Open triangular jaw */
				xx[0] = x + param[0] * cos (D2R * param[1]);	yy[0] = y + param[0] * sin (D2R * param[1]);
				xx[1] = x;				yy[1] = y;
				xx[2] = x + param[0] * cos (D2R * param[2]);	yy[2] = y + param[0] * sin (D2R * param[2]);
				PSL_plotline (PSL, xx, yy, 3, PSL_MOVE+PSL_STROKE);	/* Plot jaw */
			}
		}
	}
	if (status) PSL_command (PSL, "U\n");	/* Restore graphics state after messing with spiders */
	if (outline) {	/* Draw wedge outline on top */
		if (windshield)
			PSL_command (PSL, "V %d %d T 0 0 %d %g %g arc 0 0 %d %g %g arcn P os U\n", psl_ix (PSL, x), psl_iy (PSL, y),
				psl_iz (PSL, param[0]), param[1], param[2], psl_iz (PSL, param[4]), param[2], param[1]);
		else
			PSL_command (PSL, "%d %g %g %d %d 2 copy M 5 2 roll arc os\n", psl_iz (PSL, param[0]), param[1], param[2], psl_ix (PSL, x), psl_iy (PSL, y));
	}
	return (PSL_NO_ERROR);
}

static void psl_get_uppercase (char *new_c, char *old_c) {
	int i = 0, c;
	while (old_c[i]) {
	 	c = toupper ((int)old_c[i]);
		new_c[i++] = (char)c;
	}
	new_c[i] = 0;
}

static void psl_freeplot (struct PSL_CTRL *PSL) {
	/* Simply eliminate any buffer for memory-writing PS */
	if (PSL->internal.buffer) PSL_free (PSL->internal.buffer);	/* Remove any previous plot buffer */
	PSL->internal.n_alloc = PSL->internal.n = 0;
	PSL->internal.pmode = 0;
}

#if 0 /* Not used */
static void psl_defunits_array (struct PSL_CTRL *PSL, const char *param, double *array, int n) {
	/* These are used by PSL_plottextline */
	int i;
	PSL_command (PSL, "/%s\n", param);
	for (i = 0; i < n; i++) PSL_command (PSL, "%.2f\n", array[i] * PSL->internal.dpu);
	PSL_command (PSL, "%d array astore def\n", n);
}
#endif

static void psl_def_font_encoding (struct PSL_CTRL *PSL) {
	/* Initialize book-keeping for font encoding and write font macros */

	int i;

	/* Initialize T/F array for font reencoding so that we only do it once
	 * for each font that is used */

	PSL_command (PSL, "/PSL_font_encode ");
	for (i = 0; i < PSL->internal.N_FONTS; i++) PSL_command (PSL, "0 ");
	PSL_command (PSL, "%d array astore def", PSL->internal.N_FONTS);
	(PSL->internal.comments) ? PSL_command (PSL, "\t%% Initially zero\n") : PSL_command (PSL, "\n");

	/* Define font macros (see postscriptlight.h for details on how to add fonts) */

	for (i = 0; i < PSL->internal.N_FONTS; i++) PSL_command (PSL, "/F%d {/%s Y}!\n", i, PSL->internal.font[i].name);
}

static int psl_bitreduce (struct PSL_CTRL *PSL, unsigned char *buffer, int nx, int ny, size_t ncolors) {
	/* Reduce an 8-bit stream to 1-, 2- or 4-bit stream */
	int in, out, i, j, nout, nbits;

	/* Number of colors determines number of bits */
	if (ncolors <= 2)
		nbits = 1;
	else if (ncolors <= 4)
		nbits = 2;
	else if (ncolors <= 16)
		nbits = 4;
	else
		return (8);

	/* "Compress" bytes line-by-line. The number of bits per line should be multiple of 8
	   But when it isn't overflow is prevent by extra size allocation done in psl_makecolormap */
	out = 0;
	nx = abs (nx);
	nout = (nx * nbits + 7) / 8;
	for (j = 0; j < ny; j++) {
		in = j * nx;
		if (nbits == 1) {
			for (i = 0; i < nout; i++) {
				buffer[out++] = (buffer[in] << 7) + (buffer[in+1] << 6) + (buffer[in+2] << 5) + (buffer[in+3] << 4) + (buffer[in+4] << 3) + (buffer[in+5] << 2) + (buffer[in+6] << 1) + buffer[in+7];
				in += 8;
			}
		}
		else if (nbits == 2) {
			for (i = 0; i < nout; i++) {
				buffer[out++] = (buffer[in] << 6) + (buffer[in+1] << 4) + (buffer[in+2] << 2) + buffer[in+3];
				in += 4;
			}
		}
		else if (nbits == 4) {
			for (i = 0; i < nout; i++) {
				buffer[out++] = (buffer[in] << 4) + buffer[in+1];
				in += 2;
			}
		}
	}

	PSL_message (PSL, PSL_MSG_INFORMATION, "Image depth reduced to %d bits\n", nbits);
	return (nbits);
}

static int psl_bitimage_cmap (struct PSL_CTRL *PSL, double f_rgb[], double b_rgb[]) {
	/* Print colormap for 1-bit image or imagemask. Returns value of "polarity":
	 * 0 = Paint 0 bits foreground color, leave 1 bits transparent
	 * 1 = Paint 1 bits background color, leave 0 bits transparent
	 * 2 = Paint 0 bits foreground color, paint 1 bits background color
	 * ! Note that odd return values indicate that the bitmap has to be
	 * ! inverted before plotting, either explicitly, or through a mapping
	 * ! function in the PostScript image definition.
	 */
	int polarity;
	double f_cmyk[4], b_cmyk[4];

	PSL_command (PSL, " [/Indexed /Device");
	if (b_rgb[0] < 0.0) {
		/* Background is transparent */
		polarity = 0;
		if (PSL_is_gray (f_rgb))
			PSL_command (PSL, "Gray 0 <%02X>", PSL_u255(f_rgb[0]));
		else if (PSL->internal.color_mode == PSL_GRAY)
			PSL_command (PSL, "Gray 0 <%02X>", PSL_u255(PSL_YIQ(f_rgb)));
		else if (PSL->internal.color_mode == PSL_CMYK) {
			psl_rgb_to_cmyk (f_rgb, f_cmyk);
			PSL_command (PSL, "CMYK 0 <%02X%02X%02X%02X>", PSL_q255(f_cmyk));
		}
		else
			PSL_command (PSL, "RGB 0 <%02X%02X%02X>", PSL_t255(f_rgb));
	}
	else if (f_rgb[0] < 0.0) {
		/* Foreground is transparent */
		polarity = 1;
		if (PSL_is_gray (b_rgb))
			PSL_command (PSL, "Gray 0 <%02X>", PSL_u255(b_rgb[0]));
		else if (PSL->internal.color_mode == PSL_GRAY)
			PSL_command (PSL, "Gray 0 <%02X>", PSL_u255(PSL_YIQ(b_rgb)));
		else if (PSL->internal.color_mode == PSL_CMYK) {
			psl_rgb_to_cmyk (b_rgb, b_cmyk);
			PSL_command (PSL, "CMYK 0 <%02X%02X%02X%02X>", PSL_q255(b_cmyk));
		}
		else
			PSL_command (PSL, "RGB 0 <%02X%02X%02X>", PSL_t255(b_rgb));
	}
	else {
		/* Colored foreground and background */
		polarity = 2;
		if (PSL_is_gray (b_rgb) && PSL_is_gray (f_rgb))
			PSL_command (PSL, "Gray 1 <%02X%02X>", PSL_u255(f_rgb[0]), PSL_u255(b_rgb[0]));
		else if (PSL->internal.color_mode == PSL_GRAY)
			PSL_command (PSL, "Gray 1 <%02X%02X>", PSL_u255(PSL_YIQ(f_rgb)), PSL_u255(PSL_YIQ(b_rgb)));
		else if (PSL->internal.color_mode == PSL_CMYK) {
			psl_rgb_to_cmyk (f_rgb, f_cmyk);
			psl_rgb_to_cmyk (b_rgb, b_cmyk);
			PSL_command (PSL, "CMYK 1 <%02X%02X%02X%02X%02X%02X%02X%02X>", PSL_q255(f_cmyk), PSL_q255(b_cmyk));
		}
		else
			PSL_command (PSL, "RGB 1 <%02X%02X%02X%02X%02X%02X>", PSL_t255(f_rgb), PSL_t255(b_rgb));
	}
	PSL_command (PSL, "] setcolorspace");

	return (polarity);
}

/* Make sure that all memory is freed upon return.
   This way is simpler than freeing buffer, red, green, blue, entry individually at every return
 */
#define Return(code) {PSL_free (buffer); PSL_free (entry); PSL_free (red); PSL_free (green); PSL_free (blue); return (code);}

static int psl_get_boundingbox (struct PSL_CTRL *PSL, FILE *fp, int *llx, int *lly, int *trx, int *try,
	double *hires_llx, double *hires_lly, double *hires_trx, double *hires_try) {

	int nested = 0;
	char buf[PSL_BUFSIZ];

	/* Set default BoundingBox and HiResBoundingBox */

	*hires_llx = *llx = 0; *hires_trx = *trx = 720; *hires_lly = *lly = 0; *hires_try = *try = 720;

	/* Fish for the BoundingBox and the HiResBoundingBox. It assumes the line with HiResBoundingBox
	   always follows the BoundingBox line. */

	while (fgets(buf, PSL_BUFSIZ, fp) != NULL) {
		if (!strncmp(buf, "%%Begin", 7U))
			++nested;
		else if (nested && !strncmp(buf, "%%End", 5U))
			--nested;
		else if (!nested) {
			if  (!strncmp(buf, "%%BoundingBox:", 14U) && !strstr(buf, "(atend)")) {
				if (sscanf(strchr(buf, ':') + 1, "%d %d %d %d", llx, lly, trx, try) < 4) return 1;
				*hires_llx = *llx;
				*hires_lly = *lly;
				*hires_trx = *trx;
				*hires_try = *try;
				if (fgets(buf, PSL_BUFSIZ, fp) != NULL) {
					if  (!strncmp(buf, "%%HiResBoundingBox:", 19U) && !strstr(buf, "(atend)")) {
						if (sscanf(strchr(buf, ':') + 1, "%lg %lg %lg %lg", hires_llx, hires_lly, hires_trx, hires_try) < 4) return -1;
					}
				}
				return 0;
			}
		}
	}

	PSL_message (PSL, PSL_MSG_WARNING, "Warning: No proper BoundingBox, defaults assumed: %d %d %d %d\n", *llx, *lly, *trx, *try);
	return 1;
}

static int psl_init_fonts (struct PSL_CTRL *PSL) {
	FILE *in = NULL;
	int n_PSL_fonts;
	unsigned int i = 0;
	size_t n_alloc = 64;
	char buf[PSL_BUFSIZ];
	char fullname[PSL_BUFSIZ];

	PSL->internal.font = PSL_memory (PSL, NULL, n_alloc, struct PSL_FONT);

	/* Loads the available fonts for this installation */

	/* First the standard 35 PostScript fonts from Adobe + 4 Japanese fonts */
	memcpy (PSL->internal.font, PSL_standard_fonts, PSL_N_STANDARD_FONTS * sizeof (struct PSL_FONT));
	PSL->internal.N_FONTS = n_PSL_fonts = i = PSL_N_STANDARD_FONTS;

	/* Then any custom fonts */

	if (psl_getsharepath (PSL, "postscriptlight", "PSL_custom_fonts", ".txt", fullname)) {
		if ((in = fopen (fullname, "r")) == NULL) {	/* File exist but opening fails? WTF! */
			PSL_message (PSL, PSL_MSG_ERROR, "Fatal Error: ");
			perror (fullname);
			PSL_exit (EXIT_FAILURE);
		}

		while (fgets (buf, PSL_BUFSIZ, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			if (sscanf (buf, "%s %lf %d", fullname, &PSL->internal.font[i].height, &PSL->internal.font[i].encoded) != 3) {
				PSL_message (PSL, PSL_MSG_ERROR, "Warning: Trouble decoding custom font info [%s].  Skipping this font\n", buf);
				continue;
			}
			if (strlen (fullname) >= PSL_NAME_LEN) {
				PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font name %s exceeds %d characters and will be truncated\n", fullname, PSL_NAME_LEN);
				fullname[PSL_NAME_LEN-1] = '\0';
			}
			strncpy (PSL->internal.font[i].name, fullname, PSL_NAME_LEN-1);
			i++;
			if (i == n_alloc) {
				n_alloc <<= 1;
				PSL->internal.font = PSL_memory (PSL, PSL->internal.font, n_alloc, struct PSL_FONT);
			}
		}
		fclose (in);
		PSL->internal.N_FONTS = i;
	}
	else {
		PSL_message (PSL, PSL_MSG_INFORMATION, "No PSL_custom_fonts.txt found\n");
	}

	/* Final allocation of font array */
	PSL->internal.font = PSL_memory (PSL, PSL->internal.font, PSL->internal.N_FONTS, struct PSL_FONT);
	return PSL_NO_ERROR;
}

static char *psl_putdash (struct PSL_CTRL *PSL, char *pattern, double offset) {
	/* Writes the dash pattern */
	static char text[PSL_BUFSIZ];
	char mark = '[';
	size_t len = 0;
	if (pattern && pattern[0]) {
		while (*pattern) {
			sprintf (&text[len], "%c%d", mark, psl_ip (PSL, atof(pattern)));
			while (*pattern && *pattern != ' ') pattern++;
			while (*pattern && *pattern == ' ') pattern++;
			mark = ' ';
			len = strlen(text);
		}
		sprintf (&text[len], "] %d B", psl_ip (PSL, offset));
	}
	else
		sprintf (text, "[] 0 B");	/* Reset to continuous line */
	return (text);
}

static void psl_computeBezierControlPoints (struct PSL_CTRL *PSL, double *K, int n, double **P1, double **P2) {
	/* Translated from https://www.particleincell.com/wp-content/uploads/2012/06/bezier-spline.js */
	int i;
	double *p1 = NULL, *p2 = NULL, *a = NULL, *b = NULL, *c = NULL, *r = NULL;
	double m;
	p1 = PSL_memory (PSL, NULL, n, double);
	p2 = PSL_memory (PSL, NULL, n, double);
	a = PSL_memory (PSL, NULL, n, double);
	b = PSL_memory (PSL, NULL, n, double);
	c = PSL_memory (PSL, NULL, n, double);
	r = PSL_memory (PSL, NULL, n, double);

	n--;	/* Now id of last knot */

	/* left most segment*/
	a[0] = 0.0;
	b[0] = 2.0;
	c[0] = 1.0;
	r[0] = K[0] + 2.0 * K[1];

	/* internal segments*/
	for (i = 1; i < n - 1; i++)
	{
		a[i] = 1.0;
		b[i] = 4.0;
		c[i] = 1.0;
		r[i] = 4.0 * K[i] + 2.0 * K[i+1];
	}

	/* right segment*/
	a[n-1] = 2.0;
	b[n-1] = 7.0;
	c[n-1] = 0.0;
	r[n-1] = 8.0 * K[n-1] + K[n];

	/* solves Ax=b with the Thomas algorithm (from Wikipedia)*/
	for (i = 1; i < n; i++)
	{
		m = a[i] / b[i-1];
		b[i] = b[i] - m * c[i - 1];
		r[i] = r[i] - m*r[i-1];
	}

	/* Evaluate p1 */
	p1[n-1] = r[n-1] / b[n-1];
	for (i = n - 2; i >= 0; --i)
		p1[i] = (r[i] - c[i] * p1[i+1]) / b[i];

	/* we have p1, now compute p2*/
	for (i = 0; i < n-1; i++)
		p2[i] = 2.0 * K[i+1] - p1[i+1];

	p2[n-1] = 0.5 * (K[n] + p1[n-1]);

	*P1 = p1;	*P2 = p2;
	PSL_free (a);	PSL_free (b);	PSL_free (c);	PSL_free (r);
}

static psl_indexed_image_t psl_makecolormap (struct PSL_CTRL *PSL, unsigned char *buffer, int nx, int ny, int nbits) {
	/* When image consists of less than PSL_MAX_COLORS colors, the image can be
	 * indexed to safe a significant amount of space.
	 * The image and colormap are returned as a struct psl_indexed_image_t.
	 *
	 * It is important that the first RGB tuple is mapped to index 0.
	 * This is used for color masked images.
	 */
	size_t i, j, npixels;	/* Need 64-bit ints to avoid overflow of int */
	psl_colormap_t colormap;
	psl_indexed_image_t image;

	if (abs (nbits) != 24) return (NULL);		/* We only index into the RGB colorspace. */

	npixels = ((size_t)abs (nx)) * ((size_t)ny);

	colormap = psl_memory (PSL, NULL, 1U, sizeof (*colormap));
	colormap->ncolors = 0;
	image = psl_memory (PSL, NULL, 1U, sizeof (*image));
	image->buffer = psl_memory (PSL, NULL, npixels+8, sizeof (*image->buffer));	/* Add 8 to avoid overflow access in psl_bitreduce() */
	image->colormap = colormap;

	if (nx < 0) {
		/* Copy the colour mask value into index 0 */
		colormap->colors[0][0] = buffer[0];
		colormap->colors[0][1] = buffer[1];
		colormap->colors[0][2] = buffer[2];
		colormap->ncolors++;
		buffer += 3;		/* Skip to start of image */
	}

	for (i = 0; i < npixels; i++) {
		for (j = 0; j < colormap->ncolors; j++)
			if (colormap->colors[j][0] == buffer[0] && colormap->colors[j][1] == buffer[1] && colormap->colors[j][2] == buffer[2]) {
				image->buffer[i] = (unsigned char)j;
				break;
			}

		if (j == colormap->ncolors) {
			if (colormap->ncolors == PSL_MAX_COLORS) {	/* Too many colors to index. */
				PSL_free (image->buffer);
				PSL_free (image);
				PSL_free (colormap);
				PSL_message (PSL, PSL_MSG_INFORMATION, "Too many colors to make colormap - using 24-bit direct color instead.\n");
				return (NULL);
			}
			image->buffer[i] = (unsigned char)j;
			colormap->colors[j][0] = buffer[0];
			colormap->colors[j][1] = buffer[1];
			colormap->colors[j][2] = buffer[2];
			colormap->ncolors++;
		}
		buffer += 3;
	}

	/* There's no need for a color map when the number of colors is the same as the number of pixels.
	   Then you're better off with a compressed 24-bit color image instead. */
	if (colormap->ncolors >= npixels)  {
		PSL_free (image->buffer);
		PSL_free (image);
		PSL_free (colormap);
		PSL_message (PSL, PSL_MSG_INFORMATION, "Use of colormap is inefficient - using 24-bit direct color instead.\n");
		return (NULL);
	}

	PSL_message (PSL, PSL_MSG_INFORMATION, "Colormap of %" PRIuS " colors created\n", colormap->ncolors);
	return (image);
}

static char *psl_putcolor (struct PSL_CTRL *PSL, double rgb[]) {
	static char text[PSL_BUFSIZ];

	if (PSL_eq (rgb[0], -1.0)) {
		/* Ignore, no color set */
		text[0] = '\0';
	}
	else if (PSL_eq (rgb[0], -3.0)) {
		/* Pattern fill */
		sprintf (text, "pattern%ld I", lrint(rgb[1]));
	}
	else if (PSL_is_gray (rgb)) {
		/* Gray scale, since R==G==B */
		sprintf (text, PSL->current.bw_format, rgb[0]);
	}
	else if (PSL->internal.color_mode == PSL_GRAY) {
		/* Gray scale, forced by user */
		sprintf (text, PSL->current.bw_format, PSL_YIQ(rgb));
	}
	else if (PSL->internal.color_mode == PSL_RGB) {
		/* Full color, RGB mode */
		sprintf (text, PSL->current.rgb_format, rgb[0], rgb[1], rgb[2]);
	}
	else if (PSL->internal.color_mode == PSL_CMYK) {
		/* CMYK mode */
		double cmyk[4];
		psl_rgb_to_cmyk (rgb, cmyk);
		sprintf (text, PSL->current.cmyk_format, cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
	}
	else {
		/* HSV mode */
		double hsv[3];
		psl_rgb_to_hsv (rgb, hsv);
		sprintf (text, PSL->current.hsv_format, hsv[0], hsv[1], hsv[2]);
	}
	if (!PSL_eq (rgb[3], 0.0)) {
		/* Transparency */
		sprintf (&text[strlen(text)], " %g /%s PSL_transp", 1.0 - rgb[3], PSL->current.transparency_mode);
	}
	return (text);
}

static const char *psl_putusername () {
	const char *unknown = "unknown";
#ifdef HAVE_GETPWUID
#include <pwd.h>
	struct passwd *pw = NULL;
	pw = getpwuid (getuid ());
	if (pw) return (pw->pw_name);
#endif
	return (unknown);
}

/*------------------- PUBLIC PSL API FUNCTIONS--------------------- */

struct PSL_CTRL *New_PSL_Ctrl (char *session) {
	struct PSL_CTRL *PSL = NULL;
	unsigned int i;

	/* Initialize the PSL structure */

	PSL = calloc (1U, sizeof (struct PSL_CTRL));
	if (session) PSL->init.session = strdup (session);
	for (i = 0; i < 3; i++) PSL->init.page_rgb[i] = -1.0;		/* Not set */
	/* Initialize a few global variables */
	strcpy (PSL->current.bw_format, "%.3lg A");			/* Default format used for grayshade value */
	strcpy (PSL->current.rgb_format, "%.3lg %.3lg %.3lg C");	/* Same, for RGB triplets */
	strcpy (PSL->current.hsv_format, "%.3lg %.3lg %.3lg H");	/* Same, for HSV triplets */
	strcpy (PSL->current.cmyk_format, "%.3lg %.3lg %.3lg %.3lg K");	/* Same, for CMYK quadruples */

	return (PSL);
}

int PSL_beginsession (struct PSL_CTRL *PSL, unsigned int flags, char *sharedir, char *userdir) {
	/* Allocate a new common control structure and initialize PSL session
	 * If sharedir, userdir are NULL and flags&1 == 1 then we look for environmental parameters
	 * 		PSL_SHAREDIR and PSL_USERDIR; otherwise we assign then from the args (even if NULL).
	 * If flags&2 == 2 then PSL is being called from an external interface so some things will live
	 *	beyond the end of a module.
	 */
	unsigned int i, search;
	char *this_c = NULL;

	search = (flags & 1);	/* If 1 then we look for environmental parameters */
	PSL->init.runmode = (flags & 2);	/* If 2 then we are being called from an environment where many modules can be called during a session */
	/* Initialize the PSL structure to default values unless already set */

	if (PSL->init.err == NULL) PSL->init.err = stderr;		/* Possible redirect of error messages */
	if (PSL->init.unit < 0 || PSL->init.unit > 3) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Measure unit %d is not in valid range (0-3)! Using 0 (cm)\n", PSL->init.unit);
		PSL->init.unit = PSL_CM;
	}
	if (PSL->init.copies == 0) PSL->init.copies = 1;		/* Once copy of each plot */
	if (PSL->init.magnify[0] == 0.0) PSL->init.magnify[0] = 1.0;	/* Default magnification global scales */
	if (PSL->init.magnify[1] == 0.0) PSL->init.magnify[1] = 1.0;	/* Default magnification global scales */
	if (PSL->init.page_rgb[0] < 0.0) for (i = 0; i < 3; i++) PSL->init.page_rgb[i] = 1.0;		/* Default paper color */

	/* Determine SHAREDIR (directory containing the postscriptlight subdirectory)
	 * but only if not passed via argument list */

	if ((this_c = sharedir) == NULL && search) this_c = getenv ("PSL_SHAREDIR");
	if (this_c) {	/* Did find a sharedir */
		PSL->internal.SHAREDIR = strdup (this_c);
		psl_dos_path_fix (PSL->internal.SHAREDIR);
		if (access(PSL->internal.SHAREDIR, R_OK)) {
			PSL_message (PSL, PSL_MSG_ERROR, "Error: Could not access PSL_SHAREDIR %s.\n", PSL->internal.SHAREDIR);
			PSL_exit (EXIT_FAILURE);
		}
	}
	else {	/* No sharedir found */
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Could not locate PSL_SHAREDIR.\n");
		PSL_exit (EXIT_FAILURE);
	}

	/* Determine USERDIR (directory containing user replacements contents in SHAREDIR) */

	if ((this_c = userdir) == NULL && search) this_c = getenv ("PSL_USERDIR");
	if (this_c) {	/* Did find a userdir */
		PSL->internal.USERDIR = strdup (this_c);
		psl_dos_path_fix (PSL->internal.USERDIR);
		if (access (PSL->internal.USERDIR, R_OK)) {
			PSL_message (PSL, PSL_MSG_ERROR, "Warning: Could not access PSL_USERDIR %s.\n", PSL->internal.USERDIR);
			PSL_free (PSL->internal.USERDIR);
		}
	}

	if (!PSL->init.encoding) PSL->init.encoding = strdup ("Standard");		/* Character encoding to use */
	return (psl_init_fonts (PSL));								/* Load the available font information */
}

int PSL_endsession (struct PSL_CTRL *PSL) {
	/* Free up memory used by the PSL control structure */
	int i;
	if (!PSL) return (PSL_NO_SESSION);	/* Never was allocated */

	psl_freeplot (PSL);
	PSL_free (PSL->internal.font);
	for (i = 0; i < PSL->internal.n_userimages; i++) PSL_free (PSL->internal.user_image[i]);
	PSL_free (PSL->internal.SHAREDIR);
	PSL_free (PSL->internal.USERDIR);
	PSL_free (PSL->init.encoding);
	PSL_free (PSL->init.session);
	PSL_free (PSL);
	return (PSL_NO_ERROR);
}

int PSL_beginlayer (struct PSL_CTRL *PSL, int layer) {
 	/* Issue begin group command */
	PSL_command (PSL, "%%%%BeginObject PSL_Layer_%d\n", layer);
	return (PSL_NO_ERROR);
}

int PSL_endlayer (struct PSL_CTRL *PSL) {
	/* Issue end group command */
	PSL_command (PSL, "%%%%EndObject\n");
	return (PSL_NO_ERROR);
}

int PSL_copy (struct PSL_CTRL *PSL, const char *txt) {
	/* Just copies the given text as is to the PSL output stream or buffer */
	if (PSL->internal.memory) {
		size_t len = strlen (txt);
		psl_prepare_buffer (PSL, len); /* Make sure we have enough memory to hold the text */
		strncat (&(PSL->internal.buffer[PSL->internal.n]), txt, len);
		PSL->internal.n += len;
	}
	else	/* Just write to the PS file */
		fprintf (PSL->internal.fp, "%s\n", txt);
	return (PSL_NO_ERROR);
}

int PSL_plotarc (struct PSL_CTRL *PSL, double x, double y, double radius, double az1, double az2, int type) {
	/* Plot an arc with radius running in azimuth from az1 to az2.
	 * Type is a combination of the following:
	 * PSL_DRAW   (0) : Draw a line segment
	 * PSL_MOVE   (1) : Move to the new anchor point (x,y) first
	 * PSL_STROKE (2) : Stroke the line
	 */
	int ir;

	if (fabs (az1 - az2) > 360.0) return (PSL_BAD_RANGE);
	if (radius < 0.0) return (PSL_BAD_SIZE);
	ir = psl_iz (PSL, radius);
	if (type & PSL_MOVE) PSL_command (PSL, "N ");
	PSL_command (PSL, "%d %d %d %g %g arc", psl_ix(PSL, x), psl_iy(PSL, y), ir, az1, az2);
	if (az1 > az2) PSL_command(PSL, "n");
	PSL_command (PSL, (type & PSL_STROKE) ? " S\n" : "\n");
	return (PSL_NO_ERROR);
}

int PSL_plotaxis (struct PSL_CTRL *PSL, double annotation_int, char *label, double annotfontsize, int side) {
	/* Expects PSL_beginaxes to have been called first */
	int annot_justify, label_justify, i, j, ndig = 0, k, reverse = false;
	double angle, dy, scl, val, annot_off, label_off, xx, sign, x, y, length, val0, val1;
	char text[PSL_BUFSIZ], format[PSL_BUFSIZ];

	k = 2 * (side % 2);	/* Start index for x [0] or y [2] in axis_limit */
	/* Get position and limit values from PSL_beginaxes settings */
	x = PSL->internal.axis_pos[0];	y = PSL->internal.axis_pos[1];
	val0 = MIN(PSL->internal.axis_limit[k], PSL->internal.axis_limit[k+1]);
	val1 = MAX(PSL->internal.axis_limit[k], PSL->internal.axis_limit[k+1]);
	if ((val1 - val0) == 0.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Axis val0 == val1!\n");
		return (PSL_BAD_RANGE);
	}
	reverse = (PSL->internal.axis_limit[k] > PSL->internal.axis_limit[k+1]);

	sprintf (text, "%g", annotation_int);	/* Try to compute a useful format */
	for (i = 0; text[i] && text[i] != '.'; i++);
	if (text[i]) {	/* Found a decimal point */
		for (j = i + 1; text[j]; j++);
		ndig = j - i - 1;
	}
	if (ndig > 0)
		sprintf (format, "%%.%df", ndig);
	else
		strcpy (format, "%g");

	if (side == 1) x += PSL->internal.axis_dim[0];	/* Right y-axis */
	if (side == 2) y += PSL->internal.axis_dim[1];	/* Top x-axis */
	length = PSL->internal.axis_dim[side%2];	/* Length of this axis */
	angle = (side%2) ? 90.0 : 0.0;			/* May have to rotate 90 degrees */
	sign = (side < 2) ? -1.0 : 1.0;			/* Which side of axis to annotate/tick */
	annot_justify = label_justify = (side < 2) ? -10 : -2;	/* And how to justify */
	dy = sign * annotfontsize * PSL->internal.p2u;	/* Font size in user units */

	PSL_command (PSL, "\nV %d %d T %g R\n", psl_iz (PSL, x), psl_iz (PSL, y), angle);
	PSL_command (PSL, "N 0 0 M %d 0 D S\n", psl_iz (PSL, length));
	scl = length / (val1 - val0);
	annot_off = dy;
	label_off = 2.5 * dy;	/* Label offset is 250% of annotation font size */
	dy *= 0.5;

	val = ceil (val0 / annotation_int) * annotation_int;	/* Start at multiple of annotation interval */
	while (val <= (val1+PSL_SMALL)) {
		xx = (val - val0) * scl;
		if (reverse) xx = length - xx;
		PSL_command (PSL, "%d 0 M 0 %d D S\n", psl_iz (PSL, xx), psl_iz (PSL, dy));
		PSL_command (PSL, "%d %d M ", psl_iz (PSL, xx), psl_iz (PSL, annot_off));
		sprintf (text, format, val);
		PSL_plottext (PSL, xx, annot_off, -annotfontsize, text, 0.0, annot_justify, 0);
		val += annotation_int;
	}
	length *= 0.5;	/* Half-point on axis for plotting label at 150% the annotation font size */
	PSL_command (PSL, "%d %d M ", psl_iz (PSL, length), psl_iz (PSL, label_off));
	PSL_plottext (PSL, length, label_off, -annotfontsize*1.5, label, 0.0, label_justify, 0);
	PSL_command (PSL, "U\n");
	return (PSL_NO_ERROR);
}

int PSL_plotbitimage (struct PSL_CTRL *PSL, double x, double y, double xsize, double ysize, int justify, unsigned char *buffer, int nx, int ny, double f_rgb[], double b_rgb[]) {
	/* Plots a 1-bit image or imagemask.
	 * x, y		: Position of image (in units)
	 * xsize, ysize	: image size in units (if 0, adjust to keep the original aspect ratio)
	 * justify	: Indicate which corner x,y refers to (see graphic)
	 * buffer	: Image bit buffer
	 * nx, ny	: Size of image (in pixels)
	 * f_rgb	: Foreground color for 1 bits (if f_rgb[0] < 0, make transparent)
	 * b_rgb	: Background color for 0 bits (if b_rgb[0] < 0, make transparent)
	 *
	 *   9       10      11
	 *   |----------------|
	 *   5    <image>     7
	 *   |----------------|
	 *   1       2        3
	 */
	int inv;

	/* If one of [xy]size is 0, keep the aspect ratio */
	if (PSL_eq (xsize, 0.0)) xsize = (ysize * nx) / ny;
	if (PSL_eq (ysize, 0.0)) ysize = (xsize * ny) / nx;

	/* Correct origin (x,y) in case of justification */
	if (justify > 1) {      /* Move the new origin so (0,0) is lower left of box */
		x -= 0.5 * ((justify + 3) % 4) * xsize;
		y -= 0.5 * (int)(justify / 4) * ysize;
	}

	PSL_comment (PSL, "Start of 1-bit image\n");
	PSL_command (PSL, "V N %d %d T %d %d scale", psl_ix(PSL, x), psl_iy(PSL, y), psl_iz (PSL, xsize), psl_iz (PSL, ysize));
	inv = psl_bitimage_cmap (PSL, f_rgb, b_rgb) % 2;
	PSL_command (PSL, "\n<< /ImageType 1 /Decode [%d %d] ", inv, 1-inv);
	psl_stream_dump (PSL, buffer, nx, ny, 1, PSL->internal.compress, PSL_ASCII85, (int)(f_rgb[0] < 0.0 || b_rgb[0] < 0.0));

	PSL_command (PSL, "U\n");
	PSL_comment (PSL, "End of 1-bit image\n");
	return (PSL_NO_ERROR);
}

int PSL_endclipping (struct PSL_CTRL *PSL, int n) {
	/* n > 0 means restore clipping n times
	 * n == PSL_ALL_CLIP restores all current clippings.
	 */

	if (n == PSL_ALL_CLIP) {	/* Undo all recorded levels of clipping paths */
		PSL_command (PSL, "PSL_nclip {PSL_cliprestore} repeat\n");	/* Undo all levels of clipping and reset clip count */
		PSL_comment (PSL, "Clipping is currently OFF\n");
		PSL->current.nclip = 0;
	}
	else if (n == 1) {	/* Undo one level of clipping paths */
		PSL_command (PSL, "PSL_cliprestore\n");	/* Undo mode levels of clipping and reduce clip count */
		PSL_comment (PSL, "Clipping reduced by 1 level\n");
		PSL->current.nclip--;
	}
	else if (n > 0) {	/* Undo mode levels of clipping paths */
		PSL_command (PSL, "%d {PSL_cliprestore} repeat\n", n);	/* Undo mode levels of clipping and reduce clip count */
		PSL_comment (PSL, "Clipping reduced by %d levels\n", n);
		PSL->current.nclip -= n;
	}
	return (PSL_NO_ERROR);
}

int PSL_beginclipping (struct PSL_CTRL *PSL, double *x, double *y, int n, double rgb[], int flag) {
	/* Any plotting outside the path defined by x,y will be clipped.
	 * use PSL_endclipping to restore the original clipping path.
	 * n    : number of x,y pairs (i.e. path length)
	 * rgb  : optional paint (use rgb[0] = -1 to avoid paint)
	 * flag : 0 = continue adding pieces to the clipping path
	 *        1 = start new clipping path (more follows)
	 *        2 = end clipping path (this is the last segment)
	 *        3 = this is the complete clipping path (start to end)
	 * 	  Add 4 to omit use even-odd clipping [nonzero-winding rule].
	 */
	if (flag & 1) {	/* First segment in (possibly multi-segmented) clip-path */
		PSL_comment (PSL, "Start of polygon clip path\n");
		PSL_command (PSL, "clipsave\n");
	}

	if (n > 0) {
		int close_interior = 0;
		if ((flag & 3) != 3) close_interior = PSL_CLOSE_INTERIOR;
		PSL_plotline (PSL, x, y, n, PSL_MOVE | close_interior);	/* Must not close path since first point not given ! */
	}

	if (flag & 2) {	/* End path and [optionally] fill */
		if (!PSL_eq(rgb[0],-1.0)) PSL_command (PSL, "V %s eofill U ", psl_putcolor (PSL, rgb));
		PSL->current.nclip++;
		PSL_command (PSL, (flag & 4) ? "PSL_eoclip N\n" : "PSL_clip N\n");
		PSL_comment (PSL, "End of polygon clip path.  Polygon clipping is currently ON\n");
	}
	return (PSL_NO_ERROR);
}

int PSL_plotcolorimage (struct PSL_CTRL *PSL, double x, double y, double xsize, double ysize, int justify, unsigned char *buffer, int nx, int ny, int nbits) {
	/* Plots a 24-bit color image in Grayscale, RGB or CMYK mode.
	 * When the number of unique colors does not exceed PSL_MAX_COLORS, the routine will index
	 * 24-bit RGB images and then attempt to reduce the depth of the indexed image to 1, 2 or 4 bits.
	 *
	 * x, y		: lower left position of image in plot units
	 * xsize, ysize	: image size in units (if 0, adjust to keep the original aspect ratio)
	 * justify	: indicates what corner x,y refers to (see graphic below)
	 * buffer	: contains the bytes for the image
	 * nx, ny	: pixel dimension
	 * nbits	: number of bits per pixel (1, 2, 4, 8, 24)
	 *
	 * Special cases:
	 * nx < 0	: 8- or 24-bit image contains a color mask (first 1 or 3 bytes)
	 * nbits < 0	: "Hardware" interpolation requested
	 *
	 *   9       10      11
	 *   |----------------|
	 *   5    <image>     7
	 *   |----------------|
	 *   1       2        3
	 */
	int id, it;
	const char *colorspace[3] = {"Gray", "RGB", "CMYK"};			/* What kind of image we are writing */
	const char *decode[3] = {"0 1", "0 1 0 1 0 1", "0 1 0 1 0 1 0 1"};	/* What kind of color decoding */
	const char *type[3] = {"1", "4 /MaskColor[0]", "1 /Interpolate true"};
	psl_indexed_image_t image;

	/* If one of [xy]size is 0, keep the aspect ratio */
	if (PSL_eq (xsize, 0.0)) xsize = (ysize * nx) / ny;
	if (PSL_eq (ysize, 0.0)) ysize = (xsize * ny) / nx;

	/* Correct origin (x,y) in case of justification */
	if (justify > 1) {      /* Move the new origin so (0,0) is lower left of box */
		x -= 0.5 * ((justify + 3) % 4) * xsize;
		y -= 0.5 * (int)(justify / 4) * ysize;
	}

	/* Gray scale, CMYK or RGB encoding/colorspace */
	id = (PSL->internal.color_mode == PSL_GRAY || abs (nbits) < 24) ? 0 : (PSL->internal.color_mode == PSL_CMYK ? 2 : 1);
	/* Colormask or interpolate */
	it = nx < 0 ? 1 : (nbits < 0 ? 2 : 0);

	if (PSL->internal.color_mode != PSL_GRAY && (image = psl_makecolormap (PSL, buffer, nx, ny, nbits))) {
		/* Creation of colormap was successful */
		nbits = psl_bitreduce (PSL, image->buffer, nx, ny, image->colormap->ncolors);

		PSL_comment (PSL, "Start of indexed %s image [%d bit]\n", colorspace[id], nbits);
		PSL_command (PSL, "V N %d %d T %d %d scale [/Indexed /Device%s %" PRIuS " <\n", psl_ix(PSL, x), psl_iy(PSL, y), psl_iz (PSL, xsize), psl_iz (PSL, ysize), colorspace[id], image->colormap->ncolors - 1);
		psl_stream_dump (PSL, &image->colormap->colors[0][0], (int)image->colormap->ncolors, 1, 24, 0, PSL_HEX, 2);
		PSL_command (PSL, ">] setcolorspace\n<< /ImageType %s /Decode [0 %d] ", type[it], (1<<nbits)-1);
		psl_stream_dump (PSL, image->buffer, nx, ny, nbits, PSL->internal.compress, PSL_ASCII85, 0);
		PSL_command (PSL, "U\n");
		PSL_comment (PSL, "End of indexed %s image\n", colorspace[id]);

		/* Clear the newly created image buffer and colormap */
		PSL_free (image->buffer);
		PSL_free (image->colormap);
		PSL_free (image);
	}
	else {
		/* Export full gray scale, RGB or CMYK image */
		nbits = abs (nbits);

		PSL_comment (PSL, "Start of %s image [%d bit]\n", colorspace[id], nbits);
		PSL_command (PSL, "V N %d %d T %d %d scale /Device%s setcolorspace", psl_ix(PSL, x), psl_iy(PSL, y), psl_iz (PSL, xsize), psl_iz (PSL, ysize),  colorspace[id]);

		if (it == 1 && nbits == 24) {	/* Do PS Level 3 image type 4 with colormask */
			PSL_command (PSL, "\n<< /ImageType 4 /MaskColor [%d %d %d]", (int)buffer[0], (int)buffer[1], (int)buffer[2]);
			buffer += 3;
		}
		else if (it == 1 && nbits == 8) {	/* Do PS Level 3 image type 4 with colormask */
			PSL_command (PSL, "\n<< /ImageType 4 /MaskColor [%d]", (int)buffer[0]);
			buffer++;
		}
		else		/* Do PS Level 2 image, optionally with interpolation */
			PSL_command (PSL, "\n<< /ImageType %s", type[it]);

		PSL_command (PSL, " /Decode [%s] ", decode[id]);
		psl_stream_dump (PSL, buffer, nx, ny, nbits, PSL->internal.compress, PSL_ASCII85, 0);
		PSL_command (PSL, "U\n");
		PSL_comment (PSL, "End of %s image\n", colorspace[id]);
	}
	return (PSL_NO_ERROR);
}

int PSL_free_nonmacro (void *addr) {
	PSL_free (addr);
	return (PSL_NO_ERROR);
}

int PSL_beginaxes (struct PSL_CTRL *PSL, double llx, double lly, double width, double height, double x0, double y0, double x1, double y1) {
	/* Set the box location and user x and y ranges */
	double range;
	PSL->internal.axis_limit[0] = x0;	PSL->internal.axis_limit[1] = x1;
	PSL->internal.axis_limit[2] = y0;	PSL->internal.axis_limit[3] = y1;
	PSL->internal.axis_pos[0] = llx;	PSL->internal.axis_pos[1] = lly;
	PSL->internal.axis_dim[0] = width;	PSL->internal.axis_dim[1] = height;
	range = x1 - x0;
	PSL->internal.x0 = psl_ix (PSL, llx - x0 * width / range);
	PSL->internal.x2ix = (width / range) * PSL->internal.dpu;
	range = y1 - y0;
	PSL->internal.y0 = psl_iy (PSL, lly - y0 * height / range);
	PSL->internal.y2iy = (height / range) * PSL->internal.dpu;
	return (PSL_NO_ERROR);
}

int PSL_endaxes (struct PSL_CTRL *PSL) {
	/* Turn off user coordinates to PS coordinates scaling */
	memset (PSL->internal.axis_limit, 0, 4 * sizeof (double));
	PSL->internal.x0 = PSL->internal.y0 = 0;
	PSL->internal.x2ix = PSL->internal.y2iy = PSL->internal.dpu;
	return (PSL_NO_ERROR);
}

int PSL_plotsymbol (struct PSL_CTRL *PSL, double x, double y, double size[], int symbol) {
	/* Plotting standard symbols
	 * A) 6 non-fillable symbols +-mpxy,
	 * B) 9 fillable symbol codes acdhignst, and
	 * C) The 7 fillable and multi-parameter symbols ejmrRwv.
	 * For A and B, size[0] holds the diameter of the circumscribing circle,
	 * whereas for C other parameters are contained in the array (see below).
	 */
	int status = PSL_NO_ERROR;

	switch (symbol) {
		/* Line-only symbols. size[0] = diameter of circumscribing circle. */

		case PSL_CROSS:		/* Cross */
		case PSL_DOT:		/* Single dot */
		case PSL_PLUS:		/* Plus */
		case PSL_XDASH:		/* Horizontal line segment */
		case PSL_YDASH:		/* Vertical line segment */
			PSL_command (PSL, "%d %d %d S%c\n", psl_iz (PSL, 0.5 * size[0]), psl_ix (PSL, x), psl_iy (PSL, y), (char)symbol);
			break;

		/* One-parameter Fillable symbols. size[0] = diameter of circumscribing circle. */

		case PSL_STAR:		/* Star */
		case PSL_CIRCLE:	/* Circle */
		case PSL_DIAMOND:	/* Diamond */
		case PSL_HEXAGON:	/* Hexagon */
		case PSL_INVTRIANGLE:	/* Inverted triangle */
		case PSL_OCTAGON:	/* Octagon */
		case PSL_PENTAGON:	/* Pentagon */
		case PSL_SQUARE:	/* Square */
		case PSL_TRIANGLE:	/* Triangle */
			PSL_command (PSL, "%d %d %d S%c\n", psl_iz (PSL, 0.5 * size[0]), psl_ix (PSL, x), psl_iy (PSL, y), (char)symbol);
			break;

		/* Multi-parameter fillable symbols */

		case PSL_WEDGE:		/* A wedge or pie-slice. size[0] = radius, size[1..2] = azimuth range of arc */
			psl_wedge (PSL, x, y, size);
#if 0
			PSL_command (PSL, "%d %g %g %d %d Sw\n", psl_iz (PSL, size[0]), size[1], size[2], psl_ix (PSL, x), psl_iy (PSL, y));
#endif
			break;
		case PSL_MARC:		/* An arc with optional arrows. size[0] = radius, size[1..2] = azimuth range of arc, size[3] = shape, size[4] = arrows (0 = none, 1 = backward, 2 = forward, 3 = both) */
			psl_matharc (PSL, x, y, size);
			break;
		case PSL_ELLIPSE:	/* An ellipse. size[0] = angle of major axis, size[1..2] = length of major and minor axis */
			PSL_command (PSL, "%d %d %g %d %d Se\n", psl_iz (PSL, 0.5 * size[1]), psl_iz (PSL, 0.5 * size[2]), size[0], psl_ix (PSL, x), psl_iy (PSL, y));
			break;
		case PSL_RECT:		/* A rectangle. size[0..1] = width and height */
			PSL_command (PSL, "%d %d %d %d Sr\n", psl_iz (PSL, size[1]), psl_iz (PSL, size[0]), psl_ix (PSL, x), psl_iy (PSL, y));
			break;
		case PSL_RNDRECT:	/* A rounded rectangle. size[0..1] = width and height, size[2] = radius */
			PSL_command (PSL, "%d %d %d %d %d SR\n", psl_iz (PSL, size[1]), psl_iz (PSL, size[0]), psl_iz (PSL, size[2]), psl_ix (PSL, x), psl_iy (PSL, y));
			break;
		case PSL_ROTRECT:	/* A rotated rectangle. size[0] = angle, size[1..2] = width and height */
			PSL_command (PSL, "%d %d %g %d %d Sj\n", psl_iz (PSL, size[2]), psl_iz (PSL, size[1]), size[0], psl_ix (PSL, x), psl_iy (PSL, y));
			break;
		case PSL_VECTOR:	/* A zero-, one- or two-headed vector (x,y = tail coordinates) */
			status = psl_vector (PSL, x, y, size);
			break;
		default:
			status = PSL_BAD_SYMBOL;
			PSL_message (PSL, PSL_MSG_ERROR, "Error: Unknown symbol code %c\n", (int)symbol);
			break;
	}
	return (status);
}

int PSL_plotsegment (struct PSL_CTRL *PSL, double x0, double y0, double x1, double y1) {
	/* Short line segment */
	int ix, iy;

	ix = psl_ix (PSL, x0);
	iy = psl_iy (PSL, y0);
	PSL->internal.ix = psl_ix (PSL, x1);
	PSL->internal.iy = psl_iy (PSL, y1);
	PSL_command (PSL, "N %d %d M %d %d D S\n", ix, iy, PSL->internal.ix - ix, PSL->internal.iy - iy);
	return (PSL_NO_ERROR);
}

int PSL_setcurrentpoint (struct PSL_CTRL *PSL, double x, double y) {
	/* Set the current point only */
	PSL->internal.ix = psl_ix (PSL, x);
	PSL->internal.iy = psl_iy (PSL, y);
	PSL_command (PSL, "%d %d M\n", PSL->internal.ix, PSL->internal.iy);
	return (PSL_NO_ERROR);
}

int PSL_settransparency (struct PSL_CTRL *PSL, double transparency) {
	/* Updates the current PDF transparency only */
	if (transparency < 0.0 || transparency > 1.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Bad transparency value [%g] - ignored\n", transparency);
		return (PSL_BAD_RANGE);
	}
	if (transparency == PSL->current.transparency) return (PSL_NO_ERROR);	/* Quietly return if same as before */

	PSL_command (PSL, "%g /%s PSL_transp\n", 1.0 - transparency, PSL->current.transparency_mode);
	PSL->current.transparency = transparency;	/* Remember current setting */
	return (PSL_NO_ERROR);
}

int PSL_settransparencymode (struct PSL_CTRL *PSL, const char *mode) {
	/* Updates the current PDF transparency mode */
	int k, ok;
	if (!mode || !mode[0]) return (PSL_NO_ERROR);	/* Quietly returned if not given an argument */
	for (k = ok = 0; !ok && k < N_PDF_TRANSPARENCY_MODES; k++)
		if (!strcmp (PDF_transparency_modes[k], mode)) ok = 1;
	if (!ok) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Unknown PDF transparency mode %s - ignored\n", mode);

	strncpy (PSL->current.transparency_mode, mode, 15U);	/* Keep one character for null terminator */
	return (PSL_NO_ERROR);
}

int PSL_setfill (struct PSL_CTRL *PSL, double rgb[], int outline) {
	/* Set fill style for polygons and switch outline on or off.
	 * rgb[0] = -3: set fill pattern, rgb[1] is pattern number
	 * rgb[0] = -2: ignore. Do not change fill. Leave untouched.
	 * rgb[0] = -1: switch off filling of polygons
	 * rgb[0] >= 0: rgb is the fill color with R G B in 0-1 range.
	 * outline = -2: ignore. Do not change outline setting.
	 * outline =  0: switch outline off.
	 * outline =  1: switch outline on
	 */

	if (PSL_eq (rgb[0], -2.0))
		{ /* Skipped, no fill specified */ }
	else if (PSL_same_rgb (rgb, PSL->current.rgb[PSL_IS_FILL]))
		{ /* Skipped, fill already set */ }
	else if (PSL_eq (rgb[0], -1.0)) {
		PSL_command (PSL, "FQ\n");
		PSL_rgb_copy (PSL->current.rgb[PSL_IS_FILL], rgb);
	}
	else if (PSL_eq (rgb[3], 0.0) && !PSL_eq (PSL->current.rgb[PSL_IS_STROKE][3], 0.0)) {
		/* If stroke color is transparent and fill is not, explicitly set transparency for fill */
		PSL_command (PSL, "{%s 1 /Normal PSL_transp} FS\n", psl_putcolor (PSL, rgb));
		PSL_rgb_copy (PSL->current.rgb[PSL_IS_FILL], rgb);
	}
	else {	/* Set new r/g/b fill, after possibly changing fill transparency */
		PSL_command (PSL, "{%s} FS\n", psl_putcolor (PSL, rgb));
		PSL_rgb_copy (PSL->current.rgb[PSL_IS_FILL], rgb);
	}

	if (outline <= -2)
		{ /* Skipped, no change of outline */ }
	else if (PSL->current.outline != outline) {
		assert (outline == 0 || outline == 1);
		PSL_command (PSL, "O%d\n", outline);
		PSL->current.outline = outline;
	}

	return (PSL_NO_ERROR);
}

int PSL_setpattern (struct PSL_CTRL *PSL, int image_no, char *imagefile, int image_dpi, double f_rgb[], double b_rgb[]) {
	/* Set up pattern fill, either by using image number or imagefile name
	 * image_no:	Number of the standard PSL fill pattern (use negative when file name used instead)
	 * imagefile:	Name of image file
	 * image_dpi:	Resolution of image on the page
	 * f_rgb:	Foreground color used for set bits (1) (1-bit only)
	 * b_rgb:	Background color used for unset bits (0) (1-bit only)
	 * Returns image number
	 * DEPRECATED
	 */
	(void)(image_no); (void)(imagefile); (void)(image_dpi); (void)(f_rgb); (void)(b_rgb);
	PSL_message (PSL, PSL_MSG_ERROR, "Warning: PSL_setpattern has been deprecated - see PSL_setimage instead\n");
	return (PSL_NO_ERROR);
}

int PSL_loadimage (struct PSL_CTRL *PSL, char *file, struct imageinfo *header, unsigned char **image) {
	/* DEPRECATED */
	(void)(file); (void)(header); (void)(image);
	PSL_message (PSL, PSL_MSG_ERROR, "Warning: PSL_loadimage has been deprecated - see PSL_loadeps instead\n");
	return (PSL_NO_ERROR);
}

int PSL_setimage (struct PSL_CTRL *PSL, int image_no, char *imagefile, unsigned char *image, int image_dpi, unsigned int dim[], double f_rgb[], double b_rgb[]) {
	/* Set up image pattern fill
	 * image_no:	Number of the standard PSL fill pattern (use negative when file name used instead)
	 * imagefile:	Name of image file (not used if image_no = [1,90])
	 * image:	The bytestream making up the image (not used if image_no = [1,90])
	 * image_dpi:	Resolution of image on the page
	 * dim:		Image number of columns, rows, and bit depth (not used if image_no = [1,90])
	 * f_rgb:	Foreground color used for set bits (1) (1-bit only)
	 * b_rgb:	Background color used for unset bits (0) (1-bit only)
	 * Returns image number
	 */

	int mask, id, inv, k;
	uint64_t nx, ny;
	const char *colorspace[3] = {"Gray", "RGB", "CMYK"};			/* What kind of image we are writing */
	const char *decode[3] = {"0 1", "0 1 0 1 0 1", "0 1 0 1 0 1 0 1"};	/* What kind of color decoding */
	const char *kind_mask[2] = {"image", "imagemask"};

	/* Determine if image was used before */

	if ((image_no > 0 && image_no <= PSL_N_PATTERNS) && !PSL->internal.pattern[image_no-1].status)	/* Unused predefined */
		image_no = psl_pattern_init (PSL, image_no, NULL, NULL, 64, 64, 1);
	else if (image_no < 0) {	/* User image, check if already used */
		int i = psl_search_userimages (PSL, imagefile);	/* i = 0 is the first user image */
		if (i == -1)	/* Not found or no previous user images loaded */
			image_no = psl_pattern_init (PSL, -1, imagefile, image, dim[0], dim[1], dim[2]);
		else
			image_no = PSL_N_PATTERNS + i + 1;
	}
	k = image_no - 1;	/* Image array index */
	nx = PSL->internal.pattern[k].nx;
	ny = PSL->internal.pattern[k].ny;

	id = (PSL->internal.color_mode == PSL_CMYK) ? 2 : 1;
	mask = (PSL->internal.pattern[k].depth == 1 && (f_rgb[0] < 0.0 || b_rgb[0] < 0.0));

	/* When DPI or colors have changed, the /pattern procedure needs to be rewritten */

	if (PSL->internal.pattern[k].dpi != image_dpi ||
		!PSL_same_rgb(PSL->internal.pattern[k].f_rgb,f_rgb) ||
		!PSL_same_rgb(PSL->internal.pattern[k].b_rgb,b_rgb)) {

		PSL_comment (PSL, "Setup %s fill using pattern %d\n", kind_mask[mask], image_no);
		if (image_dpi) {	/* Use given DPI */
			nx = lrint (nx * PSL->internal.dpu / image_dpi);
			ny = lrint (ny * PSL->internal.dpu / image_dpi);
		}
		PSL_command (PSL, "/pattern%d {V %" PRIu64 " %" PRIu64 " scale", image_no, nx, ny);
		PSL_command (PSL, "\n<< /PaintType 1 /PatternType 1 /TilingType 1 /BBox [0 0 1 1] /XStep 1 /YStep 1 /PaintProc\n   {begin");

		if (PSL->internal.pattern[k].depth == 1) {	/* 1-bit bitmap basis */
			inv = psl_bitimage_cmap (PSL, f_rgb, b_rgb) % 2;
			PSL_command (PSL, "\n<< /ImageType 1 /Decode [%d %d]", inv, 1-inv);
		}
		else
			PSL_command (PSL, " /Device%s setcolorspace\n<< /ImageType 1 /Decode [%s]", colorspace[id], decode[id]);
		PSL_command (PSL, " /Width %d /Height %d /BitsPerComponent %d",
		             PSL->internal.pattern[k].nx, PSL->internal.pattern[k].ny, MIN(PSL->internal.pattern[k].depth,8));
		PSL_command (PSL, "\n   /ImageMatrix [%d 0 0 %d 0 %d] /DataSource image%d\n>> %s end}\n>> matrix makepattern U} def\n",
		             PSL->internal.pattern[k].nx, -PSL->internal.pattern[k].ny, PSL->internal.pattern[k].ny,
		             image_no, kind_mask[mask]);

		PSL->internal.pattern[k].dpi = image_dpi;
		PSL_rgb_copy (PSL->internal.pattern[k].f_rgb, f_rgb);
		PSL_rgb_copy (PSL->internal.pattern[k].b_rgb, b_rgb);
	}

	return (image_no);
}

int PSL_plotepsimage (struct PSL_CTRL *PSL, double x, double y, double xsize, double ysize, int justify, unsigned char *buffer, struct imageinfo *h) {
	/* Plots an EPS image
	 * x,y		: Position of image (in plot coordinates)
	 * xsize, ysize	: Size of image (in user units)
	 * justify	: Indicate which corner (x,y) refers to (see graphic)
	 * buffer	: EPS file (buffered)
	 * h        : Image buffer header
	 *
	 *   9       10      11
	 *   |----------------|
	 *   5    <image>     7
	 *   |----------------|
	 *   1       2        3
	 */
	double width, height;

	/* If one of [xy]size is 0, keep the aspect ratio */
	width = h->trx - h->llx;
	height = h->try - h->lly;
	if (PSL_eq (xsize, 0.0)) xsize = ysize * width / height;
	if (PSL_eq (ysize, 0.0)) ysize = xsize * height / width;

	/* Correct origin (x,y) in case of justification */
	if (justify > 1) {      /* Move the new origin so (0,0) is lower left of box */
		x -= 0.5 * ((justify + 3) % 4) * xsize;
		y -= 0.5 * (int)(justify / 4) * ysize;
	}

	PSL_command (PSL, "PSL_eps_begin\n");
	PSL_command (PSL, "%d %d T %g %g scale\n", psl_ix (PSL, x), psl_iy (PSL, y), xsize * PSL->internal.dpu / width, ysize * PSL->internal.dpu / height);
	PSL_command (PSL, "%g %g T\n", -h->llx, -h->lly);
	PSL_command (PSL, "N %g %g M %g %g L %g %g L %g %g L P clip N\n", h->llx, h->lly, h->trx, h->lly, h->trx, h->try, h->llx, h->try);
	PSL_command (PSL, "%%%%BeginDocument: psimage.eps\n");
	if (PSL->internal.memory) {
		psl_prepare_buffer (PSL, h->length); /* Make sure we have enough memory to hold the EPS */
		strncat (&(PSL->internal.buffer[PSL->internal.n]), (char *)buffer, h->length);
		PSL->internal.n += h->length;
	}
	else
		fwrite (buffer, 1U, (size_t)h->length, PSL->internal.fp);
	PSL_command (PSL, "%%%%EndDocument\n");
	PSL_command (PSL, "PSL_eps_end\n");
	return (PSL_NO_ERROR);
}

int PSL_plotline (struct PSL_CTRL *PSL, double *x, double *y, int n, int type) {
	/* Plot a (portion of a) line. This can be a line from start to finish, or a portion of it, depending
	 * on the type argument. Optionally, the line can be stroked (using the current pen), closed.
	 * Type is a combination of the following:
	 * PSL_DRAW   (0) : Draw a line segment
	 * PSL_MOVE   (1) : Move to a new anchor point (x[0], y[0]) first
	 * PSL_STROKE (2) : Stroke the line
	 * PSL_CLOSE  (8) : Close the line back to the beginning of this segment, this is done automatically
	 *                  when the first and last point are the same and PSL_MOVE is on.
	 */
	int i, i0 = 0, *ix = NULL, *iy = NULL;

	if (n < 1) return (PSL_NO_ERROR);	/* Cannot deal with empty lines */
	if (type < 0) type = -type;		/* Should be obsolete now */

	/* First remove unnecessary points that have zero curvature */

	ix = PSL_memory (PSL, NULL, n, int);
	iy = PSL_memory (PSL, NULL, n, int);

	n = psl_shorten_path (PSL, x, y, n, ix, iy, 0);

	/* If first and last point are the same, close the polygon and drop the last point
	 * (but only if this segment runs start to finish)
	 */

	if (n > 1 && (type & PSL_MOVE) && (ix[0] == ix[n-1] && iy[0] == iy[n-1]) && (type & PSL_CLOSE_INTERIOR) == 0) {n--; type |= PSL_CLOSE;}

	if (type & PSL_MOVE) {
		PSL_command (PSL, "%d %d M\n", ix[0], iy[0]);
		PSL->internal.ix = ix[0];
		PSL->internal.iy = iy[0];
		i0++;
		if (n == 1) PSL_command (PSL, "0 0 D\n");	/* Add at least a zero length line */
	}

	for (i = i0; i < n; i++) {
		if (ix[i] != PSL->internal.ix || iy[i] != PSL->internal.iy)
			PSL_command (PSL, "%d %d D\n", ix[i] - PSL->internal.ix, iy[i] - PSL->internal.iy);
		PSL->internal.ix = ix[i];
		PSL->internal.iy = iy[i];
	}
	if (type & PSL_STROKE && type & PSL_CLOSE)
		PSL_command (PSL, "P S\n");	/* Close and stroke the path */
	else if (type & PSL_CLOSE)
		PSL_command (PSL, "P\n");	/* Close the path */
	else if (type & PSL_STROKE)
		PSL_command (PSL, "S\n");	/* Stroke the path */

	PSL_free (ix);
	PSL_free (iy);

	return (PSL_NO_ERROR);
}

int PSL_plotcurve (struct PSL_CTRL *PSL, double *x, double *y, int n, int type) {
	/* Plot a (portion of a) Bezier curve. This can be a line from start to finish, or a portion of it, depending
	 * on the type argument. Optionally, the line can be stroked (using the current pen), closed.
	 * Type is a combination of the following:
	 * PSL_DRAW   (0) : Draw a line segment
	 * PSL_MOVE   (1) : Move to a new anchor point (x[0], y[0]) first [REQUIRED]
	 * PSL_STROKE (2) : Stroke the line
	 * PSL_CLOSE  (8) : Close the line back to the beginning of this segment, this is done automatically
	 *                  when the first and last point are the same and PSL_MOVE is on.
	 */
	int i = 0, *ix = NULL, *iy = NULL;
	double *Px1 = NULL, *Py1 = NULL, *Px2 = NULL, *Py2 = NULL;

	if (n < 1) return (PSL_NO_ERROR);	/* Cannot deal with empty lines */
	if (type < 0) type = -type;		/* Should be obsolete now */

	psl_computeBezierControlPoints (PSL, x, n, &Px1, &Px2);
	psl_computeBezierControlPoints (PSL, y, n, &Py1, &Py2);

	/* First convert knots to integers */

	ix = PSL_memory (PSL, NULL, n, int);
	iy = PSL_memory (PSL, NULL, n, int);

	n = psl_shorten_path (PSL, x, y, n, ix, iy, 1);

	/* If first and last point are the same, close the polygon and drop the last point
	 * (but only if this segment runs start to finish)
	 */

	if (n > 1 && (type & PSL_MOVE) && (ix[0] == ix[n-1] && iy[0] == iy[n-1])) type |= PSL_CLOSE;

	/* Move to (and set) currentpoint */
	PSL_command (PSL, "%d %d M\n", ix[0], iy[0]);
	n--;
	while (i < n) {
		PSL_command (PSL, "%d %d ", psl_ix (PSL, Px1[i]), psl_iy (PSL, Py1[i]));
		PSL_command (PSL, "%d %d ", psl_ix (PSL, Px2[i]), psl_iy (PSL, Py2[i]));
		i++;	/* Go to end point of segment */
		PSL_command (PSL, "%d %d curveto\n", ix[i], iy[i]);
	}
	PSL_free (Px1);	PSL_free (Py1);	PSL_free (Px2);	PSL_free (Py2);
	i--;	/* ID of last point */
	PSL->internal.ix = ix[i];
	PSL->internal.iy = iy[i];
	if (type & PSL_STROKE && type & PSL_CLOSE)
		PSL_command (PSL, "P S\n");	/* Close and stroke the path */
	else if (type & PSL_CLOSE)
		PSL_command (PSL, "P\n");	/* Close the path */
	else if (type & PSL_STROKE)
		PSL_command (PSL, "S\n");	/* Stroke the path */

	PSL_free (ix);
	PSL_free (iy);

	return (PSL_NO_ERROR);
}

int PSL_plotpoint (struct PSL_CTRL *PSL, double x, double y, int pen) {
	int ix, iy, idx, idy;

	/* Convert user coordinates to dots */
	ix = psl_ix (PSL, x);
	iy = psl_iy (PSL, y);

	if (pen & PSL_REL) {
		/* Relative move or relative draw */
		if (pen & PSL_STROKE) {
			/* Always draw-stroke even when displacement is 0 */
			PSL_command (PSL, "%d %d D S\n", ix, iy);
		}
		else if (ix == 0 && iy == 0)
			return (PSL_NO_ERROR);
		else if (pen & PSL_MOVE)
			PSL_command (PSL, "%d %d G\n", ix, iy);
		else
			PSL_command (PSL, "%d %d D\n", ix, iy);
		PSL->internal.ix += ix;	/* Update absolute position */
		PSL->internal.iy += iy;
	}
	else {
		/* Absolute move or absolute draw converted to relative */
		idx = ix - PSL->internal.ix;
		idy = iy - PSL->internal.iy;
		if (pen & PSL_STROKE) {
			/* Always draw-stroke even when displacement is 0 */
			PSL_command (PSL, "%d %d D S\n", idx, idy);
		}
		else if (pen & PSL_MOVE) {
			/* Do this always, even if idx = idy = 0, just to be sure we are where we are supposed to be */
			PSL_command (PSL, "%d %d M\n", ix, iy);
		}
		else if (idx == 0 && idy == 0)
			return (PSL_NO_ERROR);
		else {
			/* Convert to relative draw to have smaller numbers */
			PSL_command (PSL, "%d %d D\n", idx, idy);
		}
		PSL->internal.ix = ix;	/* Update absolute position */
		PSL->internal.iy = iy;
	}
	return (PSL_NO_ERROR);
}

int PSL_endplot (struct PSL_CTRL *PSL, int lastpage) {
	/* Finalizes the current plot layer; see PSL_endsession for terminating PSL session. */

	if (PSL->init.runmode == 0) {
		psl_pattern_cleanup (PSL);
		memset (PSL->internal.pattern, 0, 2*PSL_N_PATTERNS*sizeof (struct PSL_PATTERN));	/* Reset all pattern info since the file is now closed */
	}
	PSL_setdash (PSL, NULL, 0.0);
	if (!PSL_eq (PSL->current.rgb[PSL_IS_STROKE][3], 0.0)) PSL_command (PSL, "1 /Normal PSL_transp\n");

	if (lastpage) {
		PSL_command (PSL, "\ngrestore\n");	/* End encapsulation of main body for this plot */
		PSL_comment (PSL, "Run PSL movie label completion function, if defined\n");
		PSL_command (PSL, "PSL_movie_label_completion /PSL_movie_label_completion {} def\n");	/* Run then make it a null function */
		PSL_comment (PSL, "Run PSL movie progress indicator completion function, if defined\n");
		PSL_command (PSL, "PSL_movie_prog_indicator_completion /PSL_movie_prog_indicator_completion {} def\n");	/* Run then make it a null function */
		PSL_command (PSL, "%%PSL_Begin_Trailer\n");
		PSL_command (PSL, "%%%%PageTrailer\n");
		if (PSL->init.runmode) {
			psl_pattern_cleanup (PSL);
			memset (PSL->internal.pattern, 0, 2*PSL_N_PATTERNS*sizeof (struct PSL_PATTERN));	/* Reset all pattern info since the file is now closed */
		}
		PSL_comment (PSL, "Reset transformations and call showpage\n");
		PSL_command (PSL, "U\nshowpage\n");
		PSL_command (PSL, "\n%%%%Trailer\n");
		PSL_command (PSL, "\nend\n");
		PSL_command (PSL, "%%%%EOF\n");
	}
	else if (PSL->internal.origin[0] == 'a' || PSL->internal.origin[1] == 'a') {	/* Restore the origin of the plotting */
		if (PSL->internal.comments)  PSL_command (PSL, "%% Reset plot origin:\n");
		PSL_command (PSL, "%d %d TM\n", PSL->internal.origin[0] == 'a' ? -psl_iz(PSL, PSL->internal.offset[0]) : 0,
			PSL->internal.origin[1] == 'a' ? -psl_iz(PSL, PSL->internal.offset[1]) : 0);
	}
	if (PSL->internal.memory) {	/* Finalize memory buffer allocation */
		memset (&PSL->internal.buffer[PSL->internal.n], 0, (PSL->internal.n_alloc-PSL->internal.n)*sizeof (char));	/* Wipe the unused stuff */
		PSL->internal.n_alloc = PSL->internal.n;	/* Shrink allocated memory to what is needed to hold the PS */
		PSL->internal.buffer  = PSL_memory (PSL, PSL->internal.buffer, PSL->internal.n_alloc, char);
		if (lastpage) PSL->internal.pmode |= 2;	/* We provided a trailer */
	}
	else {	/* Dealing with files or stdout */
		if (PSL->internal.fp != stdout && PSL->internal.call_level == 1) {
			fclose (PSL->internal.fp);	/* Only level 1 can close the file (if not stdout) */
			PSL->internal.fp = NULL;
		}
	}
	PSL->internal.call_level--;	/* Done with this module call */
	return (PSL_NO_ERROR);
}

char * PSL_getplot (struct PSL_CTRL *PSL) {
	/* Simply pass the plot back to caller  */
	if (!PSL->internal.memory) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Cannot get a plot since memory output was not activated!\n");
		return (NULL);
	}
	if (!PSL->internal.buffer) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: No plot in memory available!\n");
		return (NULL);
	}
	return (PSL->internal.buffer);
}

int PSL_beginplot (struct PSL_CTRL *PSL, FILE *fp, int orientation, int overlay, int color_mode, char origin[], double offset[], double page_size[], char *title, int font_no[]) {
/* fp:		Output stream or NULL for standard output
   orientation:	0 = landscape, 1 = portrait.  If orientation &2 then we write to memory array [Default is to fp]
		If orientation&4 then we must reissue font encoding due to a change in charset
   overlay:	true if this is an overlay plot [false means print headers and macros first]
   color_mode:	0 = RGB color, 1 = CMYK color, 2 = HSV color, 3 = Gray scale
   origin:	Two characters indicating origin of new position for x and y respectively:
		'r' = Relative to old position (default)
		'a' = Relative to old position and resets at PSL_endplot
		'f' = Relative to lower left corner of the page
		'c' = Relative to center of the page
   offset:	Location of new origin relative to what is specified by "origin" (in user units)
   page_size:	Physical width and height of paper used in points
   title:	Title of the plot (or NULL if not specified)
   font_no:	Array of font numbers used in the document (or NULL if not determined)
*/
	int i, manual_feed = false, err = 0, change_charset = 0;
	double no_rgb[4] = {-1.0, -1.0, -1.0, 0.0}, dummy_rgb[4] = {-2.0, -2.0, -2.0, 0.0}, black[4] = {0.0, 0.0, 0.0, 0.0}, scl;
	time_t right_now;
	const char *uname[4] = {"cm", "inch", "meter", "point"}, xy[2] = {'x', 'y'};
	const double units_per_inch[4] = {2.54, 1.0, 0.0254, 72.0};	/* cm, inch, m, points per inch */
	char PSL_encoding[64] = {""};

	if (!PSL) return (PSL_NO_SESSION);	/* Never was allocated */

	PSL->internal.memory = (orientation & PSL_MEMORY);	/* true if we wish to write PS to memory instead of to file */
	if (PSL->internal.memory) orientation -= PSL_MEMORY;
	change_charset = (orientation & PSL_CHANGESET);	/* true if we must update the character set */
	if (change_charset) orientation -= PSL_CHANGESET;

	/* Save original initialization settings */

	PSL->internal.call_level++;	/* Becomes 1 for first module calling it, 2 if that module calls for plotting, etc */
	if (PSL->internal.call_level == 1)
		PSL->internal.fp = (fp == NULL) ? stdout : fp;	/* For higher levels we reuse existing file pointer */
	PSL->internal.overlay = overlay;
	memcpy (PSL->init.page_size, page_size, 2 * sizeof(double));

	PSL->internal.color_mode = color_mode;
	if (!origin)
		PSL->internal.origin[0] = PSL->internal.origin[1] = 'r';
	else
		PSL->internal.origin[0] = origin[0], PSL->internal.origin[1] = origin[1];
	PSL->internal.p_width  = fabs (page_size[0]);
	PSL->internal.p_height = fabs (page_size[1]);
	manual_feed = (page_size[0] < 0.0);			/* Want Manual Request for paper */
	PSL_settransparencymode (PSL, "Normal");		/* Default PDF transparency mode */
	PSL_setfontdims (PSL, PSL_SUBSUP_SIZE, PSL_SCAPS_SIZE, PSL_SUP_UP_LC, PSL_SUP_UP_UC, PSL_SUB_DOWN);	/* Default sub/sup/scaps dimensions */

	PSL->current.linewidth = -1.0;				/* Will be changed by PSL_setlinewidth */
	PSL_rgb_copy (PSL->current.rgb[PSL_IS_STROKE], dummy_rgb);		/* Will be changed by PSL_setcolor */
	PSL->current.outline = -1;				/* Will be changed by PSL_setfill */
	PSL_rgb_copy (PSL->current.rgb[PSL_IS_FILL], dummy_rgb);	/* Will be changed by PSL_setfill */

	PSL->internal.dpu = PSL_DOTS_PER_INCH / units_per_inch[PSL->init.unit];	/* Dots per unit resolution of output device */
	PSL->internal.dpp = PSL_DOTS_PER_INCH / units_per_inch[PSL_PT];		/* Dots per point resolution of output device */
	PSL->internal.x2ix = PSL->internal.dpu;					/* Scales x coordinates to dots */
	PSL->internal.y2iy = PSL->internal.dpu;					/* Scales y coordinates to dots */
	PSL->internal.x0 = PSL->internal.y0 = 0;				/* Offsets for x and y when mapping user x,y to PS ix,iy */
	PSL->internal.p2u = PSL->internal.dpp / PSL->internal.dpu;		/* Converts dimensions in points to user units */

	right_now = time ((time_t *)0);
	PSL->internal.landscape = !(overlay || orientation);	/* Only rotate if not overlay and not Portrait */
	PSL->internal.offset[0] = offset[0];
	PSL->internal.offset[1] = offset[1];

	/* Initialize global variables */
	strcpy (PSL->current.bw_format, "%.3lg A");			/* Default format used for grayshade value */
	strcpy (PSL->current.rgb_format, "%.3lg %.3lg %.3lg C");	/* Same, for RGB triplets */
	strcpy (PSL->current.hsv_format, "%.3lg %.3lg %.3lg H");	/* Same, for HSV triplets */
	strcpy (PSL->current.cmyk_format, "%.3lg %.3lg %.3lg %.3lg K");	/* Same, for CMYK quadruples */

	/* In case this is the last overlay, set the Bounding box coordinates to be used atend */

	if (overlay) {	/* Must issue PSL header - this is the start of a new panel */
		if (change_charset) {
			PSL_comment (PSL, "Encode fonts using selected character set: %s\n", PSL->init.encoding);
			sprintf (PSL_encoding, "PSL_%s", PSL->init.encoding);	/* Prepend the PSL_ prefix */
			err = psl_place_encoding (PSL, PSL_encoding);
			if (err) return err;
			psl_def_font_encoding (PSL);		/* Initialize book-keeping for font encoding and write font macros */
		}
		if (PSL->current.complete) {	/* Execute the panel completion function, then disable again */
			PSL_comment (PSL, "Run PSL completion function from last overlay, if defined\n");
			PSL_command (PSL, "PSL_plot_completion /PSL_plot_completion {} def\n");	/* Run then make it a null function */
			PSL->current.complete = 0;
		}
	}
	else {	/* Must issue PSL header - this is the start of a new plot */

		if (PSL->internal.memory) {	/* Will be writing to memory so need to set that up */
			psl_freeplot (PSL);	/* Free any previous plot laying around */
			PSL->internal.buffer  = PSL_memory (PSL, NULL, PSL_MEM_ALLOC, char);
			PSL->internal.n_alloc = PSL_MEM_ALLOC;
			PSL->internal.n	      = 0;
			PSL->internal.pmode   = 1;	/*	Header of plot will be written below */
		}

		PSL_command (PSL, "%%!PS-Adobe-3.0\n");

		/* Write definitions of macros to plotfile */

		PSL_command (PSL, "%%%%BoundingBox: 0 0 %d %d\n", lrint (PSL->internal.p_width), lrint (PSL->internal.p_height));
		/* The spaces below are to accommodate eventual need by psconvert when working with in-memory-PS */
		PSL_command (PSL, "%%%%HiResBoundingBox: 0 0 %.4lf %.4lf             \n", PSL->internal.p_width, PSL->internal.p_height);
		if (title) {
			PSL_command (PSL, "%%%%Title: %s\n", title);
			PSL_command (PSL, "%%%%Creator: %s\n", PSL->init.session);
		}
		else {
			PSL_command (PSL, "%%%%Title: PSL v%s document\n", PSL_Version);
			PSL_command (PSL, "%%%%Creator: PSL\n");
		}
		PSL_command (PSL, "%%%%For: %s\n", psl_putusername());
		if (font_no) {
			PSL_command (PSL, "%%%%DocumentNeededResources: font");
			for (i = 0; i < PSL_MAX_EPS_FONTS && font_no[i] != -1; i++) PSL_command (PSL, " %s", PSL->internal.font[font_no[i]].name);
			PSL_command (PSL, "\n");
		}

		PSL_command (PSL, "%%%%CreationDate: %s", ctime(&right_now));
		PSL_command (PSL, "%%%%LanguageLevel: %d\n", PS_LANGUAGE_LEVEL);
		PSL_command (PSL, "%%%%DocumentData: Clean7Bit\n");
		if (PSL->internal.landscape)
			PSL_command (PSL, "%%%%Orientation: Landscape\n");
		else
			PSL_command (PSL, "%%%%Orientation: Portrait\n");
		PSL_command (PSL, "%%%%Pages: 1\n");
		PSL_command (PSL, "%%%%EndComments\n\n");

		PSL_command (PSL, "%%%%BeginProlog\n");
		psl_bulkcopy (PSL, "PSL_prologue");	/* General PS code */
		sprintf (PSL_encoding, "PSL_%s", PSL->init.encoding);	/* Prepend the PSL_ prefix */
		err = psl_place_encoding (PSL, PSL_encoding);
		if (err) return err;
		psl_def_font_encoding (PSL);		/* Initialize book-keeping for font encoding and write font macros */

		psl_bulkcopy (PSL, "PSL_label");	/* PS code for label line annotations and clipping */
		PSL_command (PSL, "%%%%EndProlog\n\n");

		PSL_command (PSL, "%%%%BeginSetup\n");
		PSL_command (PSL, "/PSLevel /languagelevel where {pop languagelevel} {1} ifelse def\n");
		PSL_command (PSL, "PSLevel 1 gt { << /WhiteIsOpaque true >> setpagedevice } if\n");
		if (manual_feed)	/* Manual media feed requested */
			PSL_command (PSL, "PSLevel 1 gt { << /ManualFeed true >> setpagedevice } if\n");
		else if (PSL->internal.p_width > 0.0 && PSL->internal.p_height > 0.0)	/* Specific media selected */
			PSL_command (PSL, "PSLevel 1 gt { << /PageSize [%g %g] /ImagingBBox null >> setpagedevice } if\n",
			             PSL->internal.p_width, PSL->internal.p_height);
		if (PSL->init.copies > 1) PSL_command (PSL, "/#copies %d def\n", PSL->init.copies);
		PSL_command (PSL, "%%%%EndSetup\n\n");

		PSL_command (PSL, "%%%%Page: 1 1\n\n");

		PSL_command (PSL, "%%%%BeginPageSetup\n");
		PSL_comment (PSL, "Init coordinate system and scales\n");
		scl = 1.0 / PSL->internal.dpp;
		PSL_comment (PSL, "Scale initialized to %g, so 1 %s equals %g Postscript units\n", scl, uname[PSL->init.unit], PSL->internal.dpu);

		PSL_command (PSL, "V ");
		if (PSL->internal.landscape) PSL_command (PSL, "%g 0 T 90 R ", PSL->internal.p_width);
		PSL_command (PSL, "%g %g scale\n", PSL->init.magnify[0] * scl, PSL->init.magnify[1] * scl);
		PSL_command (PSL, "%%%%EndPageSetup\n\n");

		if (!(PSL_is_gray(PSL->init.page_rgb) && PSL_eq(PSL->init.page_rgb[0],1.0)))	/* Change background color from white but not if PSL_no_pagefill is set via psconvert */
			PSL_command (PSL, "systemdict /PSL_no_pagefill known not {clippath %s F N} if\n", psl_putcolor (PSL, PSL->init.page_rgb));
		PSL_comment (PSL, "End of PSL header\n");

		/* Save page size */
		PSL_defpoints (PSL, "PSL_page_xsize", PSL->internal.landscape ? PSL->internal.p_height : PSL->internal.p_width);
		PSL_defpoints (PSL, "PSL_page_ysize", PSL->internal.landscape ? PSL->internal.p_width : PSL->internal.p_height);

		PSL_command (PSL, "/PSL_plot_completion {} def\n");	/* Initialize custom procedure as a null function */
		PSL_command (PSL, "/PSL_movie_label_completion {} def\n");	/* Initialize custom procedure as a null function */
		PSL_command (PSL, "/PSL_movie_prog_indicator_completion {} def\n");	/* Initialize custom procedure as a null function */

		/* Write out current settings for cap, join, and miter; these may be changed by user at any time later */
		i = PSL->internal.line_cap;	PSL->internal.line_cap = PSL_BUTT_CAP;		PSL_setlinecap (PSL, i);
		i = PSL->internal.line_join;	PSL->internal.line_join = PSL_MITER_JOIN;	PSL_setlinejoin (PSL, i);
		i = PSL->internal.miter_limit;	PSL->internal.miter_limit = PSL_MITER_DEFAULT;	PSL_setmiterlimit (PSL, i);
		PSL_command (PSL, "%%PSL_End_Header\n");
		PSL_command (PSL, "gsave\n");	/* Begin encapsulation of main body for this plot */
	}

	/* Set default line color and no-rgb */
	PSL_setcolor (PSL, black, PSL_IS_STROKE);
	PSL_setfill (PSL, no_rgb, 0);

	/* Set origin of the plot */

	if (PSL->internal.comments)  PSL_command (PSL, "%% Set plot origin:\n");
	for (i = 0; i < 2; i++) {
		switch (PSL->internal.origin[i]) {
			case 'f': PSL_command (PSL, "%d PSL_%corig sub ", psl_iz (PSL, offset[i]), xy[i]); break;
			case 'c': PSL_command (PSL, "%d PSL_%corig sub PSL_page_%csize 2 div add ", psl_iz (PSL, offset[i]), xy[i], xy[i]); break;
			default : PSL_command (PSL, "%d ", psl_iz (PSL, offset[i])); break;
		}
	}
	PSL_command (PSL, "TM\n");

	return (PSL_NO_ERROR);
}

int PSL_setlinecap (struct PSL_CTRL *PSL, int cap) {
	if (cap != PSL->internal.line_cap) {
		PSL_command (PSL, "%d setlinecap\n", cap);
		PSL->internal.line_cap = cap;
	}
	return (PSL_NO_ERROR);
}

int PSL_setlinejoin (struct PSL_CTRL *PSL, int join) {
	if (join != PSL->internal.line_join) {
		PSL_command (PSL, "%d setlinejoin\n", join);
		PSL->internal.line_join = join;
	}
	return (PSL_NO_ERROR);
}

int PSL_setmiterlimit (struct PSL_CTRL *PSL, int limit) {
	if (limit != PSL->internal.miter_limit) {
		PSL_command (PSL, "%g setmiterlimit\n", (limit == 0) ? 10.0 : 1.0 / sin (0.5 * limit * D2R));
		PSL->internal.miter_limit = limit;
	}
	return (PSL_NO_ERROR);
}

int PSL_plotbox (struct PSL_CTRL *PSL, double x0, double y0, double x1, double y1) {
	/* Draw rectangle with corners (x0,y0) and (x1,y1) */
	int llx, lly;
	llx = psl_ix (PSL, x0);
	lly = psl_iy (PSL, y0);
	PSL_command (PSL, "%d %d %d %d Sb\n", psl_iy (PSL, y1) - lly, psl_ix (PSL, x1) - llx, llx, lly);
	return (PSL_NO_ERROR);
}

int PSL_plotpolygon (struct PSL_CTRL *PSL, double *x, double *y, int n) {
	/* Draw and optionally fill polygons. If 20 or fewer points we use
	 * the more expedited psl_patch function
	 */

	if (n <= 20)
		psl_patch (PSL, x, y, n);	/* Small polygons can use the patch function */
	else {
		PSL_plotline (PSL, x, y, n, PSL_MOVE);	/* No stroke or close path yet; see next line */
		PSL_command (PSL, "FO\n");		/* Close polygon and stroke/fill as set by PSL_setfill */
	}

	return (PSL_NO_ERROR);
}

int PSL_setexec (struct PSL_CTRL *PSL, int action) {
	/* Enables of disables the execution of a PSL_plot_completion function at start of a PSL_plotinit overlay */
	PSL->current.complete = (action) ? 1 : 0;
	return (PSL_NO_ERROR);
}

int PSL_setdash (struct PSL_CTRL *PSL, char *style, double offset) {
	/* Line structure in points
	 * offset from currentpoint in points
	 * style = "1 2", offset = 0:
	 *   1 point of line, 2 points of space, start at current point
	 * style = "5 3 1 3", offset = 2:
	 *   5 points line, 3 points space, 1 points line, 3 points space,
	 *   starting 2 points from current point.
	 */

	if (PSL->current.style[0] == '\0') { /* No previous style, so previous offset does not matters */
		if (!style || style[0] == '\0') return (PSL_NO_ERROR);	/* No new style given, so just return */
	}
	/* Here, we have a previous non-NULL style */
	if (!style || style[0] == '\0') {	/* No style wanted going forwards, so we do a full reset */
		memset (PSL->current.style, 0, PSL_PEN_LEN);
		PSL->current.offset = 0;
		PSL_command (PSL, "[] 0 B\n");
		return (PSL_NO_ERROR);
	}
	/* Here we have a previous style AND we have specified a (possibly) new style */
	if (PSL_eq(offset,PSL->current.offset) && !strcmp (style, PSL->current.style)) return (PSL_NO_ERROR);	/* Same as before, so just return */
	/* Finally, a new style has been given that differs from the previous and we need to update our settings */
	PSL->current.offset = offset;
	strncpy (PSL->current.style, style, PSL_PEN_LEN);
	PSL_command (PSL, "%s\n", psl_putdash (PSL, style, offset));
	return (PSL_NO_ERROR);
}

int PSL_setfont (struct PSL_CTRL *PSL, int font_no) {
	if (font_no == PSL->current.font_no) return (PSL_NO_ERROR);	/* Already set */
	if (font_no < 0 || font_no >= PSL->internal.N_FONTS) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Selected font (%d) out of range (0-%d); reset to 0\n", font_no, PSL->internal.N_FONTS-1);
		font_no = 0;
	}
	PSL->current.font_no = font_no;
	PSL->current.fontsize = 0.0;	/* Forces "%d F%d" to be written on next call to psl_putfont */
	/* Encoding will be done by subsequent calls inside the text-producing routines through calls to psl_encodefont [PS: testing line below] */
	psl_encodefont (PSL, PSL->current.font_no);

	return (PSL_NO_ERROR);
}

int PSL_setfontdims (struct PSL_CTRL *PSL, double supsub, double scaps, double sup_lc, double sup_uc, double sdown) {
	/* Adjust settings of sub/super/small caps attributes */
	if (supsub <= 0.0 || supsub >= 1.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Size of sub/super-script (%g) exceed allowable range, reset to %^g\n", supsub, PSL_SUBSUP_SIZE);
		supsub = PSL_SUBSUP_SIZE;
	}
	if (scaps <= 0.0 || scaps >= 1.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Size of small caps text (%g) exceed allowable range, reset to %^g\n", scaps, PSL_SCAPS_SIZE);
		scaps = PSL_SUBSUP_SIZE;
	}
	if (sup_lc <= 0.0 || sup_lc >= 1.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Amount of baseline shift for lower-case super-scripts (%g) exceed allowable range, reset to %^g\n",
		             sup_lc, PSL_SUP_UP_LC);
		sup_lc = PSL_SUBSUP_SIZE;
	}
	if (sup_uc <= 0.0 || sup_uc >= 1.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Amount of baseline shift for upper-case super-scripts (%g) exceed allowable range, reset to %^g\n",
		             sup_uc, PSL_SUP_UP_UC);
		sup_uc = PSL_SUBSUP_SIZE;
	}
	if (sdown <= 0.0 || sdown >= 1.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Amount of baseline shift for sub-scripts (%g) exceed allowable range, reset to %^g\n",
		             sdown, PSL_SUB_DOWN);
		sdown = PSL_SUBSUP_SIZE;
	}
	PSL->current.subsupsize = supsub;
	PSL->current.scapssize  = scaps;
	PSL->current.sub_down   = sdown;
	PSL->current.sup_up[PSL_LC] = sup_lc;
	PSL->current.sup_up[PSL_UC] = sup_uc;

	return (PSL_NO_ERROR);
}

int PSL_setformat (struct PSL_CTRL *PSL, int n_decimals) {
	/* Sets number of decimals used for rgb/gray specifications [3] */
	if (n_decimals < 1 || n_decimals > 3)
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Selected decimals for color out of range (%d), ignored\n", n_decimals);
	else {
		sprintf (PSL->current.bw_format, "%%.%df A", n_decimals);
		sprintf (PSL->current.rgb_format, "%%.%df %%.%df %%.%df C", n_decimals, n_decimals, n_decimals);
		sprintf (PSL->current.hsv_format, "%%.%df %%.%df %%.%df H", n_decimals, n_decimals, n_decimals);
		sprintf (PSL->current.cmyk_format, "%%.%df %%.%df %%.%df %%.%df K", n_decimals, n_decimals, n_decimals, n_decimals);
	}
	return (PSL_NO_ERROR);
}

int PSL_setlinewidth (struct PSL_CTRL *PSL, double linewidth) {
	if (linewidth < 0.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Selected linewidth is negative (%g), ignored\n", linewidth);
		return (PSL_BAD_WIDTH);
	}
	if (linewidth == PSL->current.linewidth) return (PSL_NO_ERROR);

	PSL_command (PSL, "%d W\n", psl_ip (PSL, linewidth));
	PSL->current.linewidth = linewidth;
	return (PSL_NO_ERROR);
}

int PSL_setcolor (struct PSL_CTRL *PSL, double rgb[], int mode) {
	/* Set the pen (PSL_IS_STROKE) color or fill (PSL_IS_FILL) color or pattern
	 * rgb[0] = -3: set pattern, rgb[1] is pattern number setup by PSL_setpattern
	 * rgb[0] = -2: ignore. Do not change pen color. Leave untouched.
	 * rgb[0] = -1: ignore. Do not change pen color. Leave untouched.
	 * rgb[0] >= 0: rgb is the color with R G B in 0-1 range.
	 */
	if (!rgb) return (PSL_NO_ERROR);	/* NULL args to be ignored */
	if (mode == PSL_IS_FONT) {	/* Internally update font color but set stroke color */
		PSL_rgb_copy (PSL->current.rgb[mode], rgb);
		mode = PSL_IS_STROKE;
	}
	if (PSL_eq (rgb[0], -2.0) || PSL_eq (rgb[0], -1.0)) return (PSL_NO_ERROR);	/* Settings to be ignored */
	if (PSL_same_rgb (rgb, PSL->current.rgb[mode])) return (PSL_NO_ERROR);	/* Same color as already set */

	/* Because psl_putcolor does not set transparency if it is 0%, we reset it here when needed */
	if (PSL_eq (rgb[3], 0.0) && !PSL_eq (PSL->current.rgb[mode][3], 0.0)) PSL_command (PSL, "1 /Normal PSL_transp ");

	/* Then, finally, set the color using psl_putcolor */
	PSL_command (PSL, "%s\n", psl_putcolor (PSL, rgb));

	/* Update the current stroke/fill color information */

	PSL_rgb_copy (PSL->current.rgb[mode], rgb);
	return (PSL_NO_ERROR);
}

char * PSL_makepen (struct PSL_CTRL *PSL, double linewidth, double rgb[], char *pattern, double offset) {
	/* Creates a text string with the corresponding PS command to set the pen */
	static char buffer[PSL_BUFSIZ];
	sprintf (buffer, "%d W %s %s", psl_ip (PSL, linewidth), psl_putcolor (PSL, rgb), psl_putdash (PSL, pattern, offset));
	return (buffer);
}

char * PSL_makefont (struct PSL_CTRL *PSL, double size, double rgb[]) {
	/* Creates a text string with the corresponding PS command to set the font (current font) */
	static char buffer[PSL_BUFSIZ];
	sprintf (buffer, "%s %d F%d", psl_putcolor (PSL, rgb), psl_ip (PSL, size), PSL->current.font_no);
	return (buffer);
}

char * PSL_makefontsize (struct PSL_CTRL *PSL, double size) {
	/* Creates a text string with the corresponding PS command to set the font (current font) with no color info */
	static char buffer[PSL_BUFSIZ];
	sprintf (buffer, "%d F%d", psl_ip (PSL, size), PSL->current.font_no);
	return (buffer);
}

char * PSL_makecolor (struct PSL_CTRL *PSL, double rgb[]) {
	/* Creates a text string with the corresponding PS command to set the color */
	static char buffer[PSL_BUFSIZ];
	sprintf (buffer, "%s", psl_putcolor (PSL, rgb));
	return (buffer);
}

int PSL_settextmode (struct PSL_CTRL *PSL, int mode) {
	/* Change from laissez-faire to replacing hyphens with minus sign char code */
	switch (mode) {
		case PSL_TXTMODE_HYPHEN:
			PSL->current.use_minus = PSL_TXTMODE_HYPHEN;
			break;
		case PSL_TXTMODE_MINUS:
			PSL->current.use_minus = PSL_TXTMODE_MINUS;
			break;
		default:
			PSL_message (PSL, PSL_MSG_ERROR, "Error: bad argument passed to PSL_settextmode (%d)!\n", mode);
			return (PSL_BAD_FLAG);
			break;
	}
	return (PSL_NO_ERROR);
}

int PSL_setdefaults (struct PSL_CTRL *PSL, double xyscales[], double page_rgb[], char *encoding) {
	/* Changes the standard PSL defaults for:
	 * xyscales:	Global x- and y-scale magnifier [1.0, 1.0]
	 * page_rgb:	Page color [white = 1/1/1]; give NULL to leave unchanged.
	 *
	 * Only non-zero values will result in a change */

	if (xyscales[0] != 0.0) PSL->init.magnify[0] = xyscales[0];	/* Change plot x magnifier */
	if (xyscales[1] != 0.0) PSL->init.magnify[1] = xyscales[1];	/* Change plot y magnifier */
	if (page_rgb) PSL_rgb_copy (PSL->init.page_rgb, page_rgb);	/* Change media color */
	if (PSL->init.encoding && encoding && strcmp (PSL->init.encoding, encoding)) {
		PSL_free (PSL->init.encoding);
		PSL->init.encoding = strdup (encoding);
	}
	else if (!PSL->init.encoding)
		PSL->init.encoding = (encoding) ? strdup (encoding) : strdup ("Standard");
	return (PSL_NO_ERROR);
}

int PSL_plottextbox (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *text, double angle, int justify, double offset[], int mode) {
	/* Plot a box to be later filled with text. The box is
	 * filled according to the current fill style (set by PSL_setfill).
	 * Note that this routine does not actually show the text. Use
	 * PSL_plottext for that after calling PSL_plottextbox
	 * x,y = location of string
	 * fontsize = fontsize in points. Use negative to indicate that anchor has already been set.
	 * text = text to be boxed in
	 * angle = angle with baseline (horizontal)
	 * justify indicates what x,y refers to, see fig below
	 * mode = 1 makes rounded corners (if offset is nonzero); 0 gives straight corners
	 * offset[0-1] = Horizontal/vertical space between box border and text
	 *
	 *
	 *   9       10      11
	 *   |----------------|
	 *   5  <textstring>  7
	 *   |----------------|
	 *   1       2        3
	 */

	/* PS strings to be used dependent on "justify%4". Empty string added for unused value. */
	const char *align[4] = {"0", "-2 div", "neg", ""};
	int i = 0, j, x_just, y_just, new_anchor;
	double dx, dy;

	if (fontsize == 0.0) return (PSL_NO_ERROR);	/* Nothing to do if text has zero size */
	new_anchor = (fontsize > 0.0);
	fontsize = fabs (fontsize);

	if (strlen (text) >= (PSL_BUFSIZ-1)) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: text_item > %d long!\n", PSL_BUFSIZ);
		return (PSL_BAD_TEXT);
	}

	dx = offset[0];	dy = offset[1];
	if (dx <= 0.0 || dy <= 0.0) mode = false;
	PSL_comment (PSL, "PSL_plottextbox begin:\n");
	psl_encodefont (PSL, PSL->current.font_no);
	psl_putfont (PSL, fontsize);
	PSL_command (PSL, "V\n");

	if (justify < 0)  {	/* Strip leading and trailing blanks */
		for (i = 0; text[i] == ' '; i++);
		for (j = (int)strlen (text) - 1; text[j] == ' '; j--) text[j] = 0;
		justify = -justify;
	}

	PSL_deftextdim (PSL, "PSL_dim", fontsize, &text[i]);	/* Set the string dimensions in PS */
	PSL_defunits (PSL, "PSL_dx", dx);
	PSL_defunits (PSL, "PSL_dy", dy);

	/* Got to anchor point */

	if (new_anchor) {	/* Set a new anchor point */
		PSL->internal.ix = psl_ix (PSL, x);
		PSL->internal.iy = psl_iy (PSL, y);
		PSL_command (PSL, "%d %d T ", PSL->internal.ix, PSL->internal.iy);
	}

	if (angle != 0.0) PSL_command (PSL, "%.3g R ", angle);
	if (justify > 1) {			/* Move the new origin so (0,0) is lower left of box */
		x_just = (justify + 3) % 4;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		y_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(x_just) ? PSL_command (PSL, "PSL_dim_w %s ", align[x_just]) : PSL_command (PSL, "0 ");
		(y_just) ? PSL_command (PSL, "PSL_dim_h %s ", align[y_just]) : PSL_command (PSL, "0 ");
		PSL_command (PSL, "T\n");
	}
	/* Here, (0,0) is lower point of textbox with no clearance yet */
	PSL_command (PSL, "PSL_dim_h PSL_dim_d sub PSL_dy 2 mul add PSL_dim_x1 PSL_dim_x0 sub PSL_dx 2 mul add ");
	if (mode)
		PSL_command (PSL, "%d PSL_dim_x0 PSL_dx sub PSL_dim_d PSL_dy sub SB\n", psl_iz (PSL, MIN (dx, dy)));
	else
		PSL_command (PSL, "PSL_dim_x0 PSL_dx sub PSL_dim_d PSL_dy sub Sb\n");
	PSL_command (PSL, "U\n");
	PSL_comment (PSL, "PSL_plottextbox end:\n");
	strncpy (PSL->current.string, &text[i], PSL_BUFSIZ - 1);	/* Save the string with one left for null terminator */
	return (PSL_NO_ERROR);
}

int PSL_deftextdim (struct PSL_CTRL *PSL, const char *dim, double fontsize, char *text) {
	/* Will calculate the dimension of the given text string.
	 * Because of possible escape sequences we need to examine the string
	 * carefully.  The dimensions will be set in PostScript as dim_w, dim_h, dim_d
	 * The width (dim_w) is determined by "stringwidth" and includes some whitespace, making,
	 * for example, all numerical digits the same width (which we want). The height (dim_h)
	 * is measured from the baseline and does not include any depth (below the baseline).
	 * Finally, dim_d is the (negative) depth.
	 * We try to produce the "stringwidth" result also when the string includes
	 * escape sequences.
	 * If dim is given as "-w", "-h", "-d" or "-b", do not assign dimensions, but leave width, height,
	 * depth or both width and height on the PostScript stack.
	 */

	char *tempstring = NULL, *piece = NULL, *piece2 = NULL, *ptr = NULL, *string = NULL, *plast = NULL, previous[BUFSIZ] = {""};
	int dy, font, sub_on, super_on, scaps_on, symbol_on, font_on, size_on, color_on, under_on, old_font, last_chr, kase = PSL_LC;
	bool last_sub = false, last_sup = false, supersub;
	double orig_size, small_size, size, scap_size, ustep[2], dstep;

	if (strlen (text) >= (PSL_BUFSIZ-1)) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: text_item > %d long!\n", PSL_BUFSIZ);
		return (PSL_BAD_TEXT);
	}

	string = psl_prepare_text (PSL, text);	/* Check for escape sequences */

	psl_encodefont (PSL, PSL->current.font_no);
	psl_putfont (PSL, fontsize);

	if (!strchr (string, '@')) {	/* Plain text string */
		if (dim[0] == '-')
			PSL_command (PSL, "(%s) s%c ", string, dim[1]);
		else
			PSL_command (PSL, "(%s) V MU 0 0 M E /%s_w edef FP pathbbox N /%s_h edef /%s_x1 edef /%s_d edef /%s_x0 edef U\n",
			             string, dim, dim, dim, dim, dim);
		PSL_free (string);
		return (PSL_NO_ERROR);
	}

	/* Here, we have special request for Symbol font and sub/superscript
	 * @~ toggles between Symbol font and default font
	 * @%<fontno>% switches font number <fontno>; give @%% to reset
	 * @- toggles between subscript and normal text
	 * @+ toggles between superscript and normal text
	 * @# toggles between Small caps and normal text
	 * @! will make a composite character of next two characters
	 * Use @@ to print a single @
	 */

	piece  = PSL_memory (PSL, NULL, 2 * PSL_BUFSIZ, char);
	piece2 = PSL_memory (PSL, NULL, PSL_BUFSIZ, char);

	font = old_font = PSL->current.font_no;
	orig_size = size = fontsize;
	small_size = size * PSL->current.subsupsize;	/* Sub-script/Super-script set at given fraction of font size */
	scap_size = size * PSL->current.scapssize;	/* Small caps set at given fraction of font size */
	ustep[PSL_LC] = PSL->current.sup_up[PSL_LC] * size;	/* Super-script baseline raised by given fraction of font size for lower case*/
	ustep[PSL_UC] = PSL->current.sup_up[PSL_UC] * size;	/* Super-script baseline raised by given fraction of font size for upper case */
	dstep = PSL->current.sub_down * size;		/* Sub-script baseline lowered by given fraction of font size */
	sub_on = super_on = scaps_on = symbol_on = font_on = size_on = color_on = under_on = false;
	supersub = (strstr (string, "@-@+") || strstr (string, "@+@-"));	/* Check for sub/super combo */
	tempstring = PSL_memory (PSL, NULL, strlen(string)+1, char);	/* Since strtok steps on it */
	strcpy (tempstring, string);
	ptr = strtok_r (tempstring, "@", &plast);
	PSL_command (PSL, "V MU 0 0 M ");	/* Initialize currentpoint */
	if (string[0] != '@') {
		PSL_command (PSL, "(%s) FP ", ptr);
		last_chr = ptr[strlen(ptr)-1];
		ptr = strtok_r (NULL, "@", &plast);
		kase = ((last_chr > 0 && last_chr < 255) && islower (last_chr)) ? PSL_LC : PSL_UC;
	}

	while (ptr) {
		if (ptr[0] == '!') {	/* Composite character */
			ptr++;
			if (ptr[0] == '\\')	/* Octal code */
				ptr += 4;
			else
				ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '~') {	/* Symbol font toggle */
			symbol_on = !symbol_on;
			font = (font == PSL_SYMBOL_FONT) ? old_font : PSL_SYMBOL_FONT;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '%') {	/* Switch font option */
			font_on = !font_on;
			ptr++;
			if (ptr[0] == '%')
				font = old_font;
			else if (ptr[0]) {
				old_font = font;
				font = psl_getfont (PSL, ptr);
			}
			while (*ptr != '%') ptr++;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '-') {	/* Subscript toggle  */
			ptr++;
			sub_on = !sub_on;
			if (sub_on) {
				if (last_sup)	/* Just did a super-script, must reset horizontal position */
					PSL_command (PSL, "PSL_last_width neg 0 G ");	/* Rewind position to orig baseline */
				else if (supersub)	/* Need to remember the width of the subscript */
					PSL_command (PSL, "/PSL_last_width %d F%d (%s) sw def\n", psl_ip (PSL, small_size), font, ptr);	/* Compute width of subscript text */
				if (ptr[0]) strcpy (previous, ptr);	/* Keep copy of possibly previous text */
			}
			else
				last_sub = (last_sup || ptr[0] == 0) ? supersub : false;	/* Only true when this is a possibility */
			size = (sub_on) ? small_size : fontsize;
			dy = (sub_on) ? -psl_ip (PSL, dstep) : psl_ip (PSL, dstep);
			PSL_command (PSL, "0 %d G ", dy);
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '+') {	/* Superscript toggle */
			ptr++;
			super_on = !super_on;
			if (super_on) {
				if (last_sub)	/* Just did a sub-script, must reset horizontal position */
					PSL_command (PSL, "PSL_last_width neg 0 G ");	/* Rewind position to orig baseline */
				else if (supersub)	/* Need to remember the width of the superscript */
					PSL_command (PSL, "/PSL_last_width %d F%d (%s) sw def\n", psl_ip (PSL, small_size), font, ptr);	/* Compute width of subscript text */
				if (ptr[0]) strcpy (previous, ptr);	/* Keep copy of possibly previous text */
			}
			else
				last_sup = (last_sub || ptr[0] == 0) ? supersub : false;	/* Only true when this is a possibility */
			size = (super_on) ? small_size : fontsize;
			dy = (super_on) ? psl_ip (PSL, ustep[kase]) : -psl_ip (PSL, ustep[kase]);
			PSL_command (PSL, "0 %d G ", dy);
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '#') {	/* Small caps toggle */
			scaps_on = !scaps_on;
			size = (scaps_on) ? scap_size : fontsize;
			ptr++;
			(scaps_on) ? psl_get_uppercase (piece, ptr) : (void) strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == ':') {	/* Font size change */
			size_on = !size_on;
			ptr++;
			if (ptr[0] == ':')
				size = fontsize = orig_size;
			else {
				size = fontsize = atof (ptr);
				while (*ptr != ':') ptr++;
			}
			small_size = size * PSL->current.subsupsize;
			scap_size = size * PSL->current.scapssize;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == ';') {	/* Color change */
			color_on = !color_on;
			ptr++;
			while (*ptr != ';') ptr++;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '_') {	/* Small caps toggle */
			under_on = !under_on;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else {	/* Not recognized or @@ for a single @ */
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
			last_sub = last_sup = false;
		}
		if (strlen (piece) > 0) {
			if (last_sub && last_sup) {	/* May possibly need to move currentpoint a bit to the widest piece */
				PSL_command (PSL, "/PSL_last_width PSL_last_width (%s) sw sub dup 0 lt {pop 0} if def\n", previous);	/* Compute width of superscript text and see if we must move a bit */
				PSL_command (PSL, "PSL_last_width 0 G ");	/* Rewind position to orig baseline */
				last_sub = last_sup = false;
			}
			PSL_command (PSL, "%d F%d (%s) FP ", psl_ip (PSL, size), font, piece);
			last_chr = ptr[strlen(piece)-1];
			if (!super_on && (last_chr > 0 && last_chr < 255)) kase = (islower (last_chr)) ? PSL_LC : PSL_UC;
		}
		ptr = strtok_r (NULL, "@", &plast);
	}

	if (dim[0] == '-' && dim[1] == 'w')
		PSL_command (PSL, "pathbbox N pop exch pop add U ");
	else if (dim[0] == '-' && dim[1] == 'h')
		PSL_command (PSL, "pathbbox N 4 1 roll pop pop pop U ");
	else if (dim[0] == '-' && dim[1] == 'd')
		PSL_command (PSL, "pathbbox N pop pop exch pop U ");
	else if (dim[0] == '-' && dim[1] == 'H')
		PSL_command (PSL, "pathbbox N exch pop exch sub exch pop U ");
	else if (dim[0] == '-' && dim[1] == 'b')
		PSL_command (PSL, "pathbbox N 4 1 roll exch pop add exch U ");
	else
		PSL_command (PSL, "pathbbox N /%s_h edef /%s_x1 edef /%s_d edef /%s_x0 edef /%s_w %s_x1 %s_x0 add def U\n",
		             dim, dim, dim, dim, dim, dim, dim);

	PSL_free (tempstring);
	PSL_free (piece);
	PSL_free (piece2);
	PSL_free (string);

	if (sub_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Sub-scripting not terminated [%s]\n", text);
	if (super_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Super-scripting not terminated [%s]\n", text);
	if (scaps_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Small-caps not terminated [%s]\n", text);
	if (symbol_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Symbol font change not terminated [%s]\n", text);
	if (size_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font-size change not terminated [%s]\n", text);
	if (color_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font-color change not terminated [%s]\n", text);
	if (under_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Text underline not terminated [%s]\n", text);

	return (sub_on|super_on|scaps_on|symbol_on|font_on|size_on|color_on|under_on);
}

int PSL_plottext (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *text, double angle, int justify, int mode) {
	/* General purpose text plotter for single line of text.  For paragraphs, see PSL_plotparagraph.
	* PSL_plottext positions and justifies the text string according to the parameters given.
	* The adjustments requires knowledge of font metrics and characteristics; hence all such
	* adjustments are passed on to the PostScript interpreter who will calculate the offsets.
	* The arguments to PSL_plottext are as follows:
	*
	* x,y:		location of string
	* fontsize:	fontsize in points.  If negative, assume currentpoint is already set,
	*		else we use x, y to set a new currentpoint.
	* text:		text string to be plotted in the current color (set by PSL_setcolor).
	*		If NULL is given then we assume PSL_plottextbox has just been called.
	* angle:	angle between text baseline and the horizontal.
	* justify:	indicates where on the textstring the x,y point refers to, see fig below.
	*		If negative then we strip leading and trailing blanks from the text.
	*		0 means no justification (already done separately).
	* mode:	0 = normal text filled with solid color; 1 = draw outline of text using
	*		the current line width and color; the text is filled with the current fill
	*		(if set, otherwise no filling is taking place); 2 = no outline, but text fill
	*		is a pattern so we use the outline path and not the show operator;
	*       3 = same as 1, except that half the outline width is plotted on the outside
	*       of the filled text, so none of the text font is obscured by the outline
	*		(If the text is not filled, 1 operates the same as 3).
	*
	*   9	    10      11
	*   |----------------|
	*   5       6        7
	*   |----------------|
	*   1	    2	     3
	*/

	char *piece = NULL, *piece2 = NULL, *ptr = NULL, *string = NULL, previous[BUFSIZ] = {""}, *plast = NULL;
	/* PS strings to be used dependent on "mode" */
	const char *op[4] = {"Z", "false charpath fs", "false charpath fs", "false charpath V S U fs"};
	/* PS strings to be used dependent on "justify". Empty strings added for unused values. */
	const char *justcmd[12] = {"", "bl ", "bc ", "br ", "", "ml ", "mc ", "mr ", "", "tl ", "tc ", "tr "};
	/* PS strings to be used dependent on "justify%4". Empty string added for unused value. */
	const char *align[4] = {"0", "-2 div", "neg", ""};
	int dy, i = 0, j, font, x_just, y_just, upen, ugap;
	int sub_on, super_on, scaps_on, symbol_on, font_on, size_on, color_on, under_on, old_font, n_uline, start_uline, stop_uline, last_chr, kase = PSL_LC;
	bool last_sub = false, last_sup = false, supersub;
	double orig_size, small_size, size, scap_size, ustep[2], dstep, last_rgb[4] = {0.0, 0.0, 0.0, 0.0};

	if (fontsize == 0.0) return (PSL_NO_ERROR);	/* Nothing to do if text has zero size */

	if (fontsize > 0.0) {	/* Set a new anchor point */
		PSL->internal.ix = psl_ix (PSL, x);
		PSL->internal.iy = psl_iy (PSL, y);
		PSL_command (PSL, "%d %d M ", PSL->internal.ix, PSL->internal.iy);
	}
	else
		fontsize = -fontsize;
	psl_encodefont (PSL, PSL->current.font_no);
	psl_putfont (PSL, fontsize);

	if (text) {
		if (strlen (text) >= (PSL_BUFSIZ-1)) {	/* We gotta have some limit on how long a single string can be... */
			PSL_message (PSL, PSL_MSG_ERROR, "Warning: text_item > %d long - text not plotted!\n", PSL_BUFSIZ);
			return (PSL_BAD_TEXT);
		}
		if (justify < 0)  {	/* Strip leading and trailing blanks */
			for (i = 0; text[i] == ' '; i++);
			for (j = (int)strlen (text) - 1; text[j] == ' '; j--) text[j] = 0;
			justify = -justify;
		}
		string = psl_prepare_text (PSL, &text[i]);	/* Check for escape sequences */
	}
	else {
		justify = abs (justify);	/* Just make sure since the stripping has already occurred */
		string = psl_prepare_text (PSL, PSL->current.string);	/* Check for escape sequences */
	}

	if (angle != 0.0) PSL_command (PSL, "V %.3g R ", angle);

	if (!strchr (string, '@')) {	/* Plain text ... this is going to be easy! */
		PSL_command (PSL, "(%s) %s%s", string, justcmd[justify], op[mode]);
		if (mode == 1) PSL_command (PSL, " S");
		else if (mode > 1) PSL_command (PSL, " N");
		PSL_command (PSL, (angle != 0.0 ) ? " U\n" : "\n");
		PSL_free (string);
		return (PSL_NO_ERROR);
	}

	/* For more difficult cases we use the PSL_deftextdim machinery to get the size of the font box */

	if (justify > 1) {
		x_just = (justify + 3) % 4;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		y_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		if (x_just && y_just) {
			PSL_deftextdim (PSL, "-b", fontsize, string);	/* Get width and height of string */
			PSL_command (PSL, "%s exch %s exch G\n", align[y_just], align[x_just]);
		}
		else if (x_just) {
			PSL_deftextdim (PSL, "-w", fontsize, string);	/* Get width of string */
			PSL_command (PSL, "%s 0 G\n", align[x_just]);
		}
		else {
			PSL_deftextdim (PSL, "-h", fontsize, string);	/* Get height of string */
			PSL_command (PSL, "%s 0 exch G\n", align[y_just]);
		}
	}

	/* Here, we have special request for Symbol font and sub/superscript
	 * @~ toggles between Symbol font and default font
	 * @%<fontno>% switches font number <fontno>; give @%% to reset
	 * @- toggles between subscript and normal text
	 * @+ toggles between superscript and normal text
	 * @# toggles between Small caps and normal text
	 * @! will make a composite character of next two characters
	 * Use @@ to print a single @
	 */

	piece  = PSL_memory (PSL, NULL, 2 * PSL_BUFSIZ, char);
	piece2 = PSL_memory (PSL, NULL, PSL_BUFSIZ, char);

	/* Now we can start printing text items */

	supersub = (strstr (string, "@-@+") || strstr (string, "@+@-"));	/* Check for sub/super combo */
	ptr = strtok_r (string, "@", &plast);
	if(string[0] != '@') {	/* String has @ but not at start - must deal with first piece explicitly */
		PSL_command (PSL, "(%s) %s\n", ptr, op[mode]);
		last_chr = ptr[strlen(ptr)-1];
		ptr = strtok_r (NULL, "@", &plast);
		kase = ((last_chr > 0 && last_chr < 255) && islower (last_chr)) ? PSL_LC : PSL_UC;

	}

	font = old_font = PSL->current.font_no;
	sub_on = super_on = scaps_on = symbol_on = font_on = size_on = color_on = under_on = false;
	size = orig_size = fontsize;
	small_size = size * PSL->current.subsupsize;
	scap_size = size * PSL->current.scapssize;
	ustep[PSL_LC] = PSL->current.sup_up[PSL_LC] * size;	/* Super-script baseline raised by given fraction of font size for lower case*/
	ustep[PSL_UC] = PSL->current.sup_up[PSL_UC] * size;	/* Super-script baseline raised by given fraction of font size for upper case */
	dstep = PSL->current.sub_down * size;
	upen = psl_ip (PSL, 0.025 * size);	/* Underline pen thickness */
	ugap = psl_ip (PSL, 0.075 * size);	/* Underline shift */
	start_uline = stop_uline = n_uline = 0;

	while (ptr) {	/* Loop over all the sub-text items separated by escape characters */
		if (ptr[0] == '!') {	/* Composite character */
			ptr++;
			if (ptr[0] == '\\') {	/* Octal code */
				strncpy (piece, ptr, 4U);
				piece[4] = 0;
				ptr += 4;
			}
			else {
				piece[0] = ptr[0];	piece[1] = 0;
				ptr++;
			}
			if (ptr[0] == '\\') {	/* Octal code again */
				strncpy (piece2, ptr, 4U);
				piece2[4] = 0;
				ptr += 4;
			}
			else {
				piece2[0] = ptr[0];	piece2[1] = 0;
				ptr++;
			}
			/* Try to center justify these two character to make a composite character - may not be right */
			PSL_command (PSL, "%d F%d (%s) E exch %s -2 div dup 0 G\n", psl_ip (PSL, size), font, piece2, op[mode]);
			PSL_command (PSL, "(%s) E -2 div dup 0 G exch %s sub neg dup 0 lt {pop 0} if 0 G\n", piece, op[mode]);
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '~') {	/* Symbol font */
			symbol_on = !symbol_on;
			font = (font == PSL_SYMBOL_FONT) ? old_font : PSL_SYMBOL_FONT;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '%') {	/* Switch font option */
			font_on = !font_on;
			ptr++;
			if (*ptr == '%')
				font = old_font;
			else if (*ptr) {
				old_font = font;
				font = psl_getfont (PSL, ptr);
				psl_encodefont (PSL, font);
			}
			while (*ptr != '%') ptr++;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '-') {	/* Subscript toggle  */
			ptr++;
			sub_on = !sub_on;
			if (sub_on) {
				if (last_sup)	/* Just did a super-script, must reset horizontal position */
					PSL_command (PSL, "PSL_last_width neg 0 G ");	/* Rewind position to orig baseline */
				else if (supersub)	/* Need to remember the width of the subscript */
					PSL_command (PSL, "/PSL_last_width %d F%d (%s) sw def\n", psl_ip (PSL, small_size), font, ptr);	/* Compute width of subscript text */
				if (ptr[0]) strcpy (previous, ptr);	/* Keep copy of possibly previous text */
			}
			else
				last_sub = (last_sup || ptr[0] == 0) ? supersub : false;	/* Only true when this is a possibility */
			size = (sub_on) ? small_size : fontsize;
			dy = (sub_on) ? -psl_ip (PSL, dstep) : psl_ip (PSL, dstep);
			PSL_command (PSL, "0 %d G ", dy);
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '+') {	/* Superscript toggle */
			ptr++;
			super_on = !super_on;
			if (super_on) {
				if (last_sub)	/* Just did a sub-script, must reset horizontal position */
					PSL_command (PSL, "PSL_last_width neg 0 G ");	/* Rewind position to orig baseline */
				else if (supersub)	/* Need to remember the width of the superscript */
					PSL_command (PSL, "/PSL_last_width %d F%d (%s) sw def\n", psl_ip (PSL, small_size), font, ptr);	/* Compute width of subscript text */
				if (ptr[0]) strcpy (previous, ptr);	/* Keep copy of possibly previous text */
			}
			else
				last_sup = (last_sub || ptr[0] == 0) ? supersub : false;	/* Only true when this is a possibility */
			size = (super_on) ? small_size : fontsize;
			dy = (super_on) ? psl_ip (PSL, ustep[kase]) : -psl_ip (PSL, ustep[kase]);
			PSL_command (PSL, "0 %d G ", dy);
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '#') {	/* Small caps */
			scaps_on = !scaps_on;
			size = (scaps_on) ? scap_size : fontsize;
			ptr++;
			(scaps_on) ? psl_get_uppercase (piece, ptr) : (void) strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == ':') {	/* Font size change */
			size_on = !size_on;
			ptr++;
			if (ptr[0] == ':')	/* Reset size */
				size = fontsize = orig_size;
			else {
				size = fontsize = atof (ptr);
				while (*ptr != ':') ptr++;
			}
			small_size = size * PSL->current.subsupsize;	scap_size = size * PSL->current.scapssize;
			ustep[PSL_LC] = PSL->current.sup_up[PSL_LC] * size;
			ustep[PSL_UC] = PSL->current.sup_up[PSL_UC] * size;
			dstep = PSL->current.sub_down * size;
			upen = psl_ip (PSL, 0.025 * size);	/* Underline pen thickness */
			ugap = psl_ip (PSL, 0.075 * size);	/* Underline shift */
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == ';') {	/* Font color change. r/g/b in 0-255 */
			int n_scan, k, error = false;
			double rgb[4];
			color_on = !color_on;
			ptr++;
			if (ptr[0] == ';') {	/* Reset color to previous value */
				PSL_command (PSL, "%s ", psl_putcolor (PSL, last_rgb));
				PSL_rgb_copy (PSL->current.rgb[PSL_IS_FONT], last_rgb);	/* Update present color */
			}
			else {
				char *s = NULL;
				j = 0;
				while (ptr[j] != ';') j++;
				ptr[j] = 0;
				if ((s = strchr (ptr, '@')) != NULL) {	/* Also gave transparency */
					rgb[3] = atof (&s[1]) / 100.0;
					s[0] = 0;
				}
				else
					rgb[3] = 0.0;
				n_scan = sscanf (ptr, "%lg/%lg/%lg", &rgb[0], &rgb[1], &rgb[2]);
				if (n_scan == 1) {	/* Got gray shade */
					rgb[0] /= 255.0;	/* Normalize to 0-1 */
					rgb[1] = rgb[2] = rgb[0];
					if (rgb[0] < 0.0 || rgb[0] > 1.0) error++;
				}
				else if (n_scan == 3) {	/* Got r/g/b */
					for (k = 0; k < 3; k++) {
						rgb[k] /= 255.0;	/* Normalize to 0-1 */
						if (rgb[k] < 0.0 || rgb[k] > 1.0) error++;
					}
				}
				else {	/* Got crap */
					PSL_message (PSL, PSL_MSG_ERROR, "Warning: Bad color change (%s) - ignored\n", ptr);
					error++;
				}

				ptr[j] = ';';
				if (s) s[0] = '@';
				while (*ptr != ';') ptr++;
				if (!error) {
					PSL_command (PSL, "%s ", psl_putcolor (PSL, rgb));
					PSL_rgb_copy (last_rgb, PSL->current.rgb[PSL_IS_FONT]);	/* Save previous color */
					PSL_rgb_copy (PSL->current.rgb[PSL_IS_FONT], rgb);	/* Update present color */
				}
			}
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else if (ptr[0] == '_') {	/* Toggle underline */
			under_on = !under_on;
			n_uline++;
			if (n_uline%2)
				start_uline = true;
			else
				stop_uline = true;
			ptr++;
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		}
		else
			strncpy (piece, ptr, 2 * PSL_BUFSIZ);
		if (start_uline) PSL_command (PSL, "currentpoint /y0_u edef /x0_u edef\n");
		if (stop_uline) PSL_command (PSL, "V %d W currentpoint pop /x1_u edef x0_u y0_u %d sub M x1_u x0_u sub 0 D S x1_u y0_u M U\n", upen, ugap);
		start_uline = stop_uline = false;
		if (strlen (piece) > 0) {
			if (last_sub && last_sup) {	/* May possibly need to move currentpoint a bit to the widest piece */
				PSL_command (PSL, "/PSL_last_width PSL_last_width (%s) sw sub dup 0 lt {pop 0} if def\n", previous);	/* Compute width of superscript text and see if we must move a bit */
				PSL_command (PSL, "PSL_last_width 0 G ");	/* Rewind position to orig baseline */
				last_sub = last_sup = false;
			}
			PSL_command (PSL, "%d F%d (%s) %s\n", psl_ip (PSL, size), font, piece, op[mode]);
			last_chr = ptr[strlen(piece)-1];
			if (!super_on && (last_chr > 0 && last_chr < 255)) kase = (islower(last_chr)) ? PSL_LC : PSL_UC;
		}
		ptr = strtok_r (NULL, "@", &plast);
	}
	if (mode == 1) PSL_command (PSL, "S\n");
	else if (mode > 1) PSL_command (PSL, "N\n");
	if (angle != 0.0) PSL_command (PSL, "U\n");
	PSL->current.fontsize = 0.0;	/* Force reset */

	PSL_free (piece);
	PSL_free (piece2);
	PSL_free (string);

	if (sub_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Sub-scripting not terminated [%s]\n", text);
	if (super_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Super-scripting not terminated [%s]\n", text);
	if (scaps_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Small-caps not terminated [%s]\n", text);
	if (symbol_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Symbol font change not terminated [%s]\n", text);
	if (size_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font-size change not terminated [%s]\n", text);
	if (color_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Font-color change not terminated [%s]\n", text);
	if (under_on) PSL_message (PSL, PSL_MSG_ERROR, "Warning: Text underline not terminated [%s]\n", text);

	return (sub_on|super_on|scaps_on|symbol_on|font_on|size_on|color_on|under_on);
}

int PSL_plottextline (struct PSL_CTRL *PSL, double x[], double y[], int np[], int n_segments, void *arg1, void *arg2, char *label[], double angle[], int nlabel_per_seg[], double fontsize, int justify, double offset[], int mode) {
	/* Placing text along lines, setting up text clippaths, and drawing the lines */
	/* x,y		Array containing the concatenated label path of all segments
	 * np		Array containing length of each label path segment in concatenated path
	 * n_segments	Number of line segments
	 * arg1, arg2   These are pointers to two arrays, depending on whether we have curved or straight baselines:
	 *  If curved baselines then
	 *  arg1 = node		Index into x/y array of label plot positions per segment (i.e., start from 0 for each new segment)
	 *  arg2 = NULL		Not used
	 *  If straight text baselines then
	 *  arg1 = xp		Array of x coordinates of where labels will be placed
	 *  arg2 = yp		Array of y coordinates of where labels will be placed
	 * label	Array of text labels
	 * angle	Text angle for each label
	 * nlabel_per_seg	Array containing number of labels per segment
	 * fontsize	Constant fontsize of all label texts [font use is the current font set with PSL_setfont]
	 * just		Justification of text relative to label coordinates [constant for all labels]
	 * offset	Clearances between text and textbox [constant]
	 * mode		= 1: We place all the PSL variables required to use the text for clipping of painting.
	 * 			= 2: We paint the text that is stored in the PSL variables.
	 * 			= 4: We use the text stored in the PSL variables to set up a clip path.  Clipping is turned ON.
	 * 			= 8: We draw the paths.
	 * 			= 16: We turn clip path OFF.
	 * 			= 32: We want rounded rectangles instead of straight rectangular boxes [straight text only].
	 * 			= 64: Typeset text along path [straight text].
	 * 			= 128: Fill box
	 * 			= 256: Draw box
	 */

	bool curved = ((mode & PSL_TXT_CURVED) == PSL_TXT_CURVED);	/* True if baseline must follow line path */
	int extras, kind = (curved) ? 1 : 0;
	char *name[2] = {"straight", "curved"}, *ext[2] = {"clip", "labels"};

	if (mode & 1) {	/* Lay down PSL variables */
		int i = 0, n_labels = 0;

		if (n_segments <= 0) return (PSL_NO_ERROR);	/* Nothing to do yet */
		if (fontsize == 0.0) return (PSL_NO_ERROR);	/* Nothing to do if text has zero size */

		if (justify < 0) psl_remove_spaces (label, n_segments, nlabel_per_seg);	/* Strip leading and trailing spaces */
		for (i = 0; i < n_segments; i++) n_labels += nlabel_per_seg[i];	/* Count number of labels */

		/* Set clearance and text height parameters */
		PSL_comment (PSL, "Set constants for textbox clearance:\n");
		PSL_defunits (PSL, "PSL_gap_x", offset[0]);		/* Set text clearance in x direction */
		PSL_defunits (PSL, "PSL_gap_y", offset[1]);		/* Set text clearance in y direction */

		/* Set PSL arrays and constants for this set of lines and labels */
		if (curved)	/* Set PSL array for curved baselines [also used to draw lines if selected] */
			psl_set_reducedpath_arrays (PSL, x, y, n_segments, np, nlabel_per_seg, arg1);
		psl_set_attr_arrays (PSL, (curved) ? arg1 : NULL, angle, label, n_segments, nlabel_per_seg);
		psl_set_int_array   (PSL, "label_n", nlabel_per_seg, n_segments);
		PSL_definteger (PSL, "PSL_n_paths", n_segments);
		PSL_definteger (PSL, "PSL_n_labels", n_labels);
		if (!curved)	/* Set PSL array for text location with straight baselines */
			psl_set_path_arrays (PSL, "txt", arg1, arg2, 1, &n_labels);
		PSL_comment (PSL, "Estimate text heights:\n");
		PSL_command (PSL, "PSL_set_label_heights\n");	/* Estimate text heights */
	}

	extras = mode & (PSL_TXT_ROUND | PSL_TXT_FILLBOX | PSL_TXT_DRAWBOX);	/* This just gets these bit settings, if present */
	if (mode & PSL_TXT_SHOW) {	/* Lay down visible text */
		PSL_comment (PSL, "Display the texts:\n");
		PSL_command (PSL, "%d PSL_%s_path_labels\n", PSL_TXT_SHOW|extras, name[kind]);
	}
	if (mode & PSL_TXT_CLIP_ON) {	/* Set up text clip paths and turn clipping ON */
		PSL_comment (PSL, "Set up text clippath and turn clipping ON:\n");
		if (mode & PSL_TXT_CLIP_OFF) PSL_command (PSL, "V\n");
		PSL_command (PSL, "%d PSL_%s_path_%s\n", PSL_TXT_CLIP_ON|extras, name[kind], ext[kind]);
		PSL->current.nclip++;	/* Increment clip level */
	}
	if (mode & PSL_TXT_DRAW) {	/* Draw the lines whose coordinates are in the PSL already */
		PSL_comment (PSL, "Draw the text line segments:\n");
		if (curved) 	/* The coordinates are in the PSL already so use PLS function */
			PSL_command (PSL, "PSL_draw_path_lines N\n");
		else {	/* Must draw lines here instead with PSL_plotline */
			int k, offset = 0;
			for (k = 0; k < n_segments; k++) {	/* Draw each segment line */
				PSL_command (PSL, "PSL_path_pen %d get cvx exec\n", k);	/* Set this segment's pen */
				PSL_plotline (PSL, &x[offset], &y[offset], np[k], PSL_MOVE|PSL_STROKE);
				offset += np[k];
			}
		}
	}
	PSL->current.font_no = -1;	/* To force setting of next font since the PSL stuff might have changed it */
	if (mode & PSL_TXT_CLIP_OFF) {	/* Turn OFF Clipping and bail */
		PSL_comment (PSL, "Turn label clipping OFF:\n");
		PSL_endclipping (PSL, 1);	/* Decrease clipping by one level */
		PSL_command (PSL, "U\n");
	}
	return (PSL_NO_ERROR);
}

int PSL_setorigin (struct PSL_CTRL *PSL, double x, double y, double angle, int mode) {
	/* mode = PSL_FWD: Translate origin, then rotate axes.
	 * mode = PSL_INV: Rotate axes, then translate origin. */

	if (mode != PSL_FWD && !PSL_eq(angle,0.0)) PSL_command (PSL, "%g R\n", angle);
	if (!PSL_eq(x,0.0) || !PSL_eq(y,0.0)) PSL_command (PSL, "%d %d T\n", psl_ix (PSL, x), psl_iy (PSL, y));
	if (mode == PSL_FWD && !PSL_eq(angle,0.0)) PSL_command (PSL, "%g R\n", angle);
	return (PSL_NO_ERROR);
}

int PSL_setparagraph (struct PSL_CTRL *PSL, double line_space, double par_width, int par_just) {
	/* Initializes PSL parameters used to typeset paragraphs with PSL_plotparagraph */

	if (par_just < PSL_BL || par_just > PSL_JUST) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Bad paragraph justification (%d)\n", par_just);
		return (PSL_BAD_JUST);
	}
	if (line_space <= 0.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Bad line spacing (%g)\n", line_space);
		return (PSL_BAD_VALUE);
	}
	if (par_width <= 0.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Bad paragraph width (%g)\n", par_width);
		return (PSL_BAD_VALUE);
	}

	PSL_comment (PSL, "PSL_setparagraph settings:\n");
	PSL_defunits (PSL, "PSL_linespace", line_space);
	PSL_defunits (PSL, "PSL_parwidth", par_width);
	PSL_command (PSL, "/PSL_parjust %d def\n", par_just);
	return (PSL_NO_ERROR);
}

int PSL_plotparagraphbox (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *paragraph, double angle, int justify, double offset[], int mode) {
	/* Determines the text box that fits the given typeset paragraph and fills/strokes with current fill/pen.
	 * mode = 0 (PSL_RECT_STRAIGHT), 1 (PSL_RECT_ROUNDED), 2 (PSL_RECT_CONVEX) or 3 (PSL_RECT_CONCAVE).
	 */
	int error = 0;
	if (offset[0] < 0.0 || offset[1] < 0.0) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Bad paragraphbox text offset (%g/%g)\n", offset[0], offset[1]);
		return (PSL_BAD_VALUE);
	}
	if (mode < PSL_RECT_STRAIGHT || mode > PSL_RECT_CONCAVE) {
		PSL_message (PSL, PSL_MSG_ERROR, "Warning: Bad paragraphbox mode (%d)\n", mode);
		return (PSL_BAD_VALUE);
	}

	if ((error = psl_paragraphprocess (PSL, y, fontsize, paragraph)) != PSL_NO_ERROR) return (error);

	PSL_command (PSL, "V ");
	PSL_setorigin (PSL, x, y, angle, PSL_FWD);		/* To original point */

	/* Do the relative horizontal justification */

	PSL_defunits (PSL, "PSL_xgap", offset[0]);
	PSL_defunits (PSL, "PSL_ygap", offset[1]);

	PSL_command (PSL, "0 0 M\n0 PSL_textjustifier");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Just get paragraph height\n" : "\n");

	/* Adjust origin for box justification */

	PSL_command (PSL, "/PSL_justify %d def\n", justify);
	PSL_command (PSL, "/PSL_x0 PSL_parwidth PSL_justify 1 sub 4 mod 0.5 mul neg mul def\n");
	if (justify > 8)	/* Top row */
		PSL_command (PSL, "/PSL_y0 0 def\n");
	else if (justify > 4)	/* Middle row */
		PSL_command (PSL, "/PSL_y0 PSL_parheight 2 div def\n");
	else			/* Bottom row */
		PSL_command (PSL, "/PSL_y0 PSL_parheight def\n");
	PSL_command (PSL, "/PSL_txt_y0 PSL_top neg def\n");

	/* Make upper left textbox corner the origin */

	PSL_command (PSL, "PSL_x0 PSL_y0 T\n");

	PSL_comment (PSL, "Start PSL box beneath text block:\n");
	if (mode == PSL_RECT_CONVEX) {	/* Create convex box path */
		PSL_command (PSL, "/PSL_h PSL_parheight 2 div PSL_ygap add def\n");
		PSL_command (PSL, "/PSL_w PSL_parwidth 2 div PSL_xgap add def\n");
		PSL_command (PSL, "/PSL_rx PSL_w PSL_w mul PSL_xgap PSL_xgap mul add 2 PSL_xgap mul div def\n");
		PSL_command (PSL, "/PSL_ry PSL_h PSL_h mul PSL_ygap PSL_ygap mul add 2 PSL_ygap mul div def\n");
		PSL_command (PSL, "/PSL_ax PSL_w PSL_rx PSL_xgap sub atan def\n");
		PSL_command (PSL, "/PSL_ay PSL_h PSL_ry PSL_ygap sub atan def\n");
		PSL_comment (PSL, "PSL_path:\n");
		PSL_command (PSL, "PSL_xgap neg PSL_ygap M\n");
		PSL_command (PSL, "PSL_ry PSL_xgap 2 mul sub PSL_parheight 2 div neg PSL_ry 180 PSL_ay sub 180 PSL_ay add arc\n");
		PSL_command (PSL, "PSL_parwidth 2 div PSL_parheight 2 PSL_ygap mul add PSL_rx sub neg PSL_rx 270 PSL_ax sub 270 PSL_ax add arc\n");
		PSL_command (PSL, "PSL_parwidth PSL_xgap 2 mul add PSL_ry sub PSL_parheight 2 div neg PSL_ry PSL_ay dup neg exch arc\n");
		PSL_command (PSL, "PSL_parwidth 2 div PSL_ygap 2 mul PSL_rx sub PSL_rx 90 PSL_ax sub 90 PSL_ax add arc\n");
	}
	else if (mode == PSL_RECT_CONCAVE) {	/* Create concave box path */
		PSL_command (PSL, "/PSL_h PSL_parheight 2 div PSL_ygap 2 mul add def\n");
		PSL_command (PSL, "/PSL_w PSL_parwidth 2 div PSL_xgap 2 mul add def\n");
		PSL_command (PSL, "/PSL_rx PSL_w PSL_w mul PSL_xgap PSL_xgap mul add 2 PSL_xgap mul div def\n");
		PSL_command (PSL, "/PSL_ry PSL_h PSL_h mul PSL_ygap PSL_ygap mul add 2 PSL_ygap mul div def\n");
		PSL_command (PSL, "/PSL_ax PSL_w PSL_rx PSL_xgap sub atan def\n");
		PSL_command (PSL, "/PSL_ay PSL_h PSL_ry PSL_ygap sub atan def\n");
		PSL_comment (PSL, "PSL_path:\n");
		PSL_command (PSL, "PSL_xgap 2 mul neg PSL_ygap 2 mul M\n");
		PSL_command (PSL, "PSL_xgap PSL_ry add neg PSL_parheight 2 div neg PSL_ry PSL_ay dup neg arcn\n");
		PSL_command (PSL, "PSL_parwidth 2 div PSL_parheight PSL_ygap add PSL_rx add neg PSL_rx 90 PSL_ax add 90 PSL_ax sub arcn\n");
		PSL_command (PSL, "PSL_parwidth PSL_xgap add PSL_ry add PSL_parheight 2 div neg PSL_ry 180 PSL_ay add 180 PSL_ay sub arcn\n");
		PSL_command (PSL, "PSL_parwidth 2 div PSL_ygap PSL_rx add PSL_rx 270 PSL_ax add 270 PSL_ax sub arcn\n");
	}
	else if (mode == PSL_RECT_ROUNDED) {	/* Create rounded box path */
		PSL_command (PSL, "/XL PSL_xgap neg def\n");
		PSL_command (PSL, "/XR PSL_parwidth PSL_xgap add def\n");
		PSL_command (PSL, "/YT PSL_ygap def\n");
		PSL_command (PSL, "/YB PSL_parheight PSL_ygap add neg def\n");
		PSL_command (PSL, "/PSL_r PSL_xgap PSL_ygap lt {PSL_xgap} {PSL_ygap} ifelse def\n");
		PSL_comment (PSL, "PSL_path:\n");
		PSL_command (PSL, "XL PSL_r add YB M\n");
		PSL_command (PSL, "XR YB XR YT PSL_r arct XR YT XL YT PSL_r arct\n");
		PSL_command (PSL, "XL YT XL YB PSL_r arct XL YB XR YB PSL_r arct\n");
	}
	else {	/* PSL_RECT_STRAIGHT */
		PSL_command (PSL, "/XL PSL_xgap neg def\n");
		PSL_command (PSL, "/XR PSL_parwidth PSL_xgap add def\n");
		PSL_command (PSL, "/YT PSL_ygap def\n");
		PSL_command (PSL, "/YB PSL_parheight PSL_ygap add neg def\n");
		PSL_comment (PSL, "PSL_path:\n");
		PSL_command (PSL, "XL YT M XL YB L XR YB L XR YT L\n");
	}
	PSL_command (PSL, "FO U\n");
	PSL_comment (PSL, "End PSL box beneath text block:\n");

	return (PSL_NO_ERROR);
}

int PSL_plotparagraph (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *paragraph, double angle, int justify) {
	/* Typeset one or more paragraphs.  Separate paragraphs by adding \r to end of last word in a paragraph.
 	 * To lay down a text box first, see PSL_plotparagraphbox. */
	int error = 0;

	if (fontsize == 0.0) return (PSL_NO_ERROR);	/* Nothing to do if text has zero size */

	/* If paragraph is NULL then PSL_plotparagraphbox has been called so we don't need to write the paragraph info to the PS file */
	if (paragraph && (error = psl_paragraphprocess (PSL, y, fontsize, paragraph)) != PSL_NO_ERROR) return (error);

	PSL_command (PSL, "V ");
	PSL_setorigin (PSL, x, y, angle, PSL_FWD);		/* To original point */

	/* Do the relative horizontal justification */

	PSL_command (PSL, "0 0 M\n0 PSL_textjustifier");
	(PSL->internal.comments) ? PSL_command (PSL, "\t%% Just get paragraph height\n") : PSL_command (PSL, "\n");

	/* Adjust origin for box justification */

	PSL_command (PSL, "/PSL_justify %d def\n", justify);
	PSL_command (PSL, "/PSL_x0 PSL_parwidth PSL_justify 1 sub 4 mod 0.5 mul neg mul def\n");
	if (justify > 8)	/* Top row */
		PSL_command (PSL, "/PSL_y0 0 def\n");
	else if (justify > 4)	/* Middle row */
		PSL_command (PSL, "/PSL_y0 PSL_parheight 2 div def\n");
	else			/* Bottom row */
		PSL_command (PSL, "/PSL_y0 PSL_parheight def\n");
	PSL_command (PSL, "/PSL_txt_y0 PSL_top neg def\n");

	/* Make upper left textbox corner the origin */

	PSL_command (PSL, "PSL_x0 PSL_y0 T\n");

	/* Adjust origin so 0,0 is lower left corner of first character on baseline */

	PSL_command (PSL, "0 PSL_txt_y0 T");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Move to col 0 on first baseline\n" : "\n");
	PSL_command (PSL, "0 0 M\n1 PSL_textjustifier U");
	PSL_command (PSL, (PSL->internal.comments) ? "\t%% Place the paragraph\n" : "\n");

	return (PSL_NO_ERROR);
}

int PSL_defunits (struct PSL_CTRL *PSL, const char *param, double value) {
	PSL_command (PSL, "/%s %d def\n", param, psl_iz (PSL, value));
	return (PSL_NO_ERROR);
}

int PSL_defpoints (struct PSL_CTRL *PSL, const char *param, double fontsize) {
	PSL_command (PSL, "/%s %d def\n", param, psl_ip (PSL, fontsize));
	return (PSL_NO_ERROR);
}

int PSL_definteger (struct PSL_CTRL *PSL, const char *param, int value) {
	PSL_command (PSL, "/%s %d def\n", param, value);
	return (PSL_NO_ERROR);
}

int PSL_defpen (struct PSL_CTRL *PSL, const char *param, double linewidth, char *style, double offset, double rgb[]) {
	/* Function to set line pen attributes */
	PSL_command (PSL, "/%s {%d W %s %s} def\n", param, psl_ip (PSL, linewidth), psl_putcolor (PSL, rgb), psl_putdash (PSL, style, offset));
	return (PSL_NO_ERROR);
}

int PSL_defcolor (struct PSL_CTRL *PSL, const char *param, double rgb[]) {
	PSL_command (PSL, "/%s {%s} def\n", param, psl_putcolor (PSL, rgb));
	return (PSL_NO_ERROR);
}

/* Helpers to make sure fp is closed and tmp_file is removed on return. */
#define Return1(x) {code = x; fclose (fp); return (code);}
#define Return2(x) {code = x; fclose (fp); remove (tmp_file); return (code);}

int PSL_loadeps (struct PSL_CTRL *PSL, char *file, struct imageinfo *h, unsigned char **picture) {
	/* PSL_loadeps reads an Encapsulated PostScript file, If picture == NULL we just return h. */

	int n, p, llx, lly, trx, try, BLOCKSIZE=4096;
	int32_t value;
	unsigned char *buffer = NULL;
	FILE *fp = NULL;

	/* Open PostScript file */

	if ((fp = fopen (file, "rb")) == NULL) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Cannot open image file %s!\n", file);
		return (PSL_READ_FAILURE);
	}

	/* Check magic key */

	if (fread (&value, sizeof (int32_t), 1, fp) != 1) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Failure reading EPS magic key from %s\n", file);
		fclose (fp);
		return (-1);
	}
#ifndef WORDS_BIGENDIAN
	value = bswap32 (value);
#endif
	if (value != EPS_MAGIC) {
		PSL_message (PSL, PSL_MSG_ERROR, "Error: Could not find EPS magic key in %s\n", file);
		fclose (fp);
		return (-1);
	}
	h->magic = (int)value;

	/* Scan for BoundingBox */

	psl_get_boundingbox (PSL, fp, &llx, &lly, &trx, &try, &h->llx, &h->lly, &h->trx, &h->try);

	/* Fill header struct with appropriate values */
	h->magic = EPS_MAGIC;
	h->width = trx - llx;
	h->height = try - lly;
	h->depth = 0;
	h->length = 0;	/* Not read yet */
	h->type = RT_EPS;
	h->maptype = RMT_NONE;
	h->maplength = 0;
	h->xorigin = llx;
	h->yorigin = lly;

	if (picture == NULL) {
		fclose (fp);
		return (0);	/* Just wanted dimensions */
	}

	/* Rewind and load into buffer */

	n=0;
	fseek (fp, (off_t)0, SEEK_SET);
	buffer = PSL_memory (PSL, NULL, BLOCKSIZE, unsigned char);
	while ((p = (int)fread ((unsigned char *)buffer + n, 1U, (size_t)BLOCKSIZE, fp)) == BLOCKSIZE) {
		n+=BLOCKSIZE;
		buffer = PSL_memory (PSL, buffer, n+BLOCKSIZE, unsigned char);
	}
	fclose (fp);

	n += p;
	buffer = PSL_memory (PSL, buffer, n, unsigned char);

	/* Now set length */
	h->length = n;

	*picture = buffer;
	return (0);
}

/* Due to the DLL boundary cross problem on Windows we are forced to have the following, otherwise
   defined as macros, implemented as functions. However, macros proved to be problematic too
   on Unixes, so now we have functions only. */
int PSL_command (struct PSL_CTRL *C, const char *format, ...) {
	va_list args;
	va_start (args, format);
	if (C->internal.memory) {	/* Send command to memory buffer */
		char tmp_buffer[4096] = {""};		/* Have to use this large array because sometimes we get the char encoding array, which is large. */
		size_t len = vsnprintf (tmp_buffer, 4096, format, args);
		psl_prepare_buffer (C, len);
		C->internal.buffer[C->internal.n] = '\0';	/* Play safe before the strcat of next line. Otherwise trash in the middle may occur */
		strncat (&(C->internal.buffer[C->internal.n]), tmp_buffer, len);
		C->internal.n += len;
	}
	else	/* Write command to stream */
		vfprintf (C->internal.fp, format, args);
	va_end (args);
	return (0);
}

int PSL_comment (struct PSL_CTRL *C, const char *format, ...) {
	va_list args;
	if (!C->internal.comments) return (0);
	va_start (args, format);
	if (C->internal.memory) {	/* Send comments to memory buffer */
		char tmp_buffer[PSL_BUFSIZ] = {""};
		size_t len = vsnprintf (tmp_buffer, PSL_BUFSIZ, format, args);
		psl_prepare_buffer (C, len + 6); /* The string plus the leading 4 and trailing 2 chars */
		strncat (&(C->internal.buffer[C->internal.n]), "%\n% ", 4U);
		C->internal.n += 4;
		strncat (&(C->internal.buffer[C->internal.n]), tmp_buffer, len);
		C->internal.n += len;
		strncat (&(C->internal.buffer[C->internal.n]), "%\n", 2U);
		C->internal.n += 2;
	}
	else {	/* Write comments to stream */
		fprintf (C->internal.fp, "%%\n%% ");
		vfprintf (C->internal.fp, format, args);
		fprintf (C->internal.fp, "%%\n");
	}
	va_end (args);
	return (0);
}

int PSL_initerr (struct PSL_CTRL *C, const char *format, ...) {
	va_list args;
	va_start (args, format);
	vfprintf (C->init.err, format, args);
	va_end (args);
	return (0);
}

int PSL_message (struct PSL_CTRL *C, int level, const char *format, ...) {
	va_list args;
	FILE *fp = (C == NULL) ? stderr : C->init.err;
	if (C && level > C->internal.verbose) return (0);
#ifdef DEBUG
	fprintf (fp, "PSL:%s:%d: ", __FILE__, __LINE__);
#else
	fprintf (fp, "PSL: ");
#endif
	va_start (args, format);
	vfprintf (fp, format, args);
	va_end (args);
	return (0);
}

FILE *PSL_fopen (struct PSL_CTRL *C, char *file, char *mode) {
	if (C->internal.fp == NULL) {	/* Open the plot file unless fp already set */
		if ((C->internal.fp = fopen (file, mode)) == NULL) {
			PSL_message (C, PSL_MSG_ERROR, "PSL_fopen error: Unable to open file %s with mode %s!\n", file, mode);
		}
	}
	return (C->internal.fp);
}

int PSL_fclose (struct PSL_CTRL *C) {
	/* Close except if stdout */
	int err = 0;
	if (C->internal.fp && C->internal.fp != stdout)
		err = fclose (C->internal.fp);
	C->internal.fp = NULL;
	return (err);
}

#ifndef HAVE_RINT
#include "s_rint.c"
#endif
