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
/* 			PSL: PostScript Light (Fortran Binder)
 * PSL is a library of plot functions that create PostScript.
 * All the routines write their output to the same plotting file,
 * which can be dumped to a Postscript output device (laserwriters).
 * PSL can handle and mix text, line-drawings, and bit-map graphics
 * in both black/white and color.  Colors are specified with r,g,b
 * values in the range 0-1.
 *
 * PSL conforms to the PostScript Files Specification V 3.0.
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
 * PSL_beginaxes
 * PSL_beginclipping	: Clips plot outside the specified polygon
 * PSL_beginlayer	: Place begin object group DSC comment.
 * PSL_beginplot	: Initialize parameters for a new plot.
 * PSL_beginsession	: Creates a new PSL session
 * PSL_endaxes		: Turns off mapping of user coordinates to PS units
 * PSL_endclipping	: Restores previous clipping path
 * PSL_endlayer		: Place end object group DSC comment.
 * PSL_endplot		: Close plotfile
 * PSL_endsession	: Terminates the PSL session
 * PSL_plotarc		: Plots a circular arc
 * PSL_plotaxis		: Plots an axis with tickmarks and annotation/label
 * PSL_plotbitimage	: Plots a 1-bit image or imagemask
 * PSL_plotcolorimage	: Plots a 24-bit 2-D image using the colorimage operator
 * PSL_plotepsimage	: Inserts EPS image
 * PSL_plotline		: Plots a line
 * PSL_plotparagraph	: Plots a text paragraph
 * PSL_plotparagraphbox	: Plots a box beneath a text paragraph
 * PSL_plotpoint	: Absolute or relative move to new position (pen up or down)
 * PSL_plotpolygon	: Creates a polygon and optionally fills it
 * PSL_plotsegment	: Plots a 2-point straight line segment
 * PSL_plotsymbol	: Plots a geometric symbol and [optionally] fills it
 * PSL_plottext		: Plots textstring
 * PSL_plottextbox	: Draw a filled box around a textstring
 * PSL_plottextline	: Place labels along paths (straight or curved), set clip path, draw line
 * PSL_loadeps		: Read EPS file
 * PSL_command		: Writes a given PostScript statement to the plot file
 * PSL_comment		: Writes a comment statement to the plot file
 * PSL_setcolor		: Sets the pen color or pattern
 * PSL_setdefaults	: Change several PSL session default values
 * PSL_setdash		: Specify pattern for dashed line
 * PSL_setfill		: Sets the fill color or pattern
 * PSL_setfont		: Changes current font and possibly re-encodes it to current encoding
 * PSL_setformat	: Changes # of decimals used in color and gray specs [3]
 * PSL_setlinecap	: Changes the line cap setting
 * PSL_setlinejoin	: Changes the line join setting
 * PSL_setlinewidth	: Sets a new linewidth
 * PSL_setimage		: Sets up a image pattern fill in PS
 * PSL_setmiterlimit	: Changes the miter limit setting for joins
 * PSL_setorigin	: Translates/rotates the coordinate system
 * PSL_setparagraph	: Sets parameters used to typeset text paragraphs
 * PSL_defpen		: Encodes a pen with attributes by name in the PS output
 * PSL_definteger	: Encodes an integer by name in the PS output
 * PSL_defpoints	: Encodes a pointsize by name in the PS output
 * PSL_defcolor		: Encodes a rgb color by name in the PS output
 * PSL_deftextdim	: Sets variables for text height and width in the PS output
 * PSL_defunits:	: Encodes a dimension by name in the PS output
 *
 * For information about usage, syntax etc, see the postscriptlight_f77.1 manual pages
 *
 * Authors:	Paul Wessel, Dept. of Geology and Geophysics, SOEST, U Hawaii
 *			   pwessel@hawaii.edu
 *		Remko Scharroo, EUMETSAT, Darmstadt, Germany
 *			   Remko.Scharroo@eumetsat.int
 * Date:	28-JUL-2015
 * Version:	5.1 [64-bit enabled API edition]
 *
 * Thanks to J. Goff and L. Parkes for their contributions to an earlier version.
 *
 */

/*--------------------------------------------------------------------
 *			SYSTEM HEADER FILES
 *--------------------------------------------------------------------*/

#include <stdlib.h>
#include "postscriptlight.h"

struct PSL_CTRL *PSL_FORTRAN;        /* Global structure needed for FORTRAN-77 (we think) */

/*------------------- PUBLIC PSL API FUNCTIONS--------------------- */

int PSL_beginsession_ ()	/* Initialize PSL session */
{       /* Fortran version: We pass the hidden global GMT_FORTRAN structure */
	PSL_FORTRAN = New_PSL_Ctrl ("Fortran");
	return (PSL_beginsession (PSL_FORTRAN));
}

int PSL_endsession_ ()
{
	return (PSL_endsession (PSL_FORTRAN));
}

int PSL_beginlayer_ (int *layer)
{
	return (PSL_beginlayer (PSL_FORTRAN, *layer));
}

int PSL_endlayer_ ()
{
	return (PSL_endlayer (PSL_FORTRAN));
}

int PSL_plotarc_ (double *x, double *y, double *radius, double *az1, double *az2, int *type)
{
	 return (PSL_plotarc (PSL_FORTRAN, *x, *y, *radius, *az1, *az2, *type));
}

int PSL_plotaxis_ (double *annotation_int, char *label, double *annotfontsize, int *side, int len)
{
	return (PSL_plotaxis (PSL_FORTRAN, *annotation_int, label, *annotfontsize, *side));
}

int PSL_plotbitimage_ (double *x, double *y, double *xsize, double *ysize, int *justify, unsigned char *buffer, int *nx, int *ny, double *f_rgb, double *b_rgb)
{
	return (PSL_plotbitimage (PSL_FORTRAN, *x, *y, *xsize, *ysize, *justify, buffer, *nx, *ny, f_rgb, b_rgb));
}

int PSL_endclipping_ (int *mode) {
	return (PSL_endclipping (PSL_FORTRAN, *mode));
}

int PSL_beginclipping_ (double *x, double *y, int *n, double *rgb, int *flag)
{
	return (PSL_beginclipping (PSL_FORTRAN, x, y, *n, rgb, *flag));
}

int PSL_plotcolorimage_ (double *x, double *y, double *xsize, double *ysize, int *justify, unsigned char *buffer, int *nx, int *ny, int *nbits, int len)
{
	return (PSL_plotcolorimage (PSL_FORTRAN, *x, *y, *xsize, *ysize, *justify, buffer, *nx, *ny, *nbits));
}

int PSL_command_nonmacro (struct PSL_CTRL *PSL, char *text)
{
	PSL_command (PSL, "%s\n", text);
	return (PSL_NO_ERROR);
}

int PSL_command_ (char *text, int len)
{
	return (PSL_command_nonmacro (PSL_FORTRAN, text));
}

int PSL_comment_nonmacro (struct PSL_CTRL *PSL, char *text)
{
	if (PSL->internal.comments) PSL_command (PSL, "%%\n%% %s\n%%\n", text);
	return (PSL_NO_ERROR);
}

int PSL_comment_ (char *text, int len)
{
	return (PSL_comment_nonmacro (PSL_FORTRAN, text));
}

int PSL_free_ (void *ptr)
{
	return (PSL_free_nonmacro (ptr));
}

int PSL_beginaxes_ (double *llx, double *lly, double *width, double *height, double *x0, double *y0, double *x1, double *y1)
{
	return (PSL_beginaxes (PSL_FORTRAN, *llx, *lly, *width, *height, *x0, *y0, *x1, *y1));
}

int PSL_endaxes_ ()
{
	return (PSL_endaxes (PSL_FORTRAN));
}

int PSL_plotsymbol_ (double *x, double *y, double size[], int *symbol)
{
	return (PSL_plotsymbol (PSL_FORTRAN, *x, *y, size, *symbol));
}

int PSL_plotsegment_ (double *x0, double *y0, double *x1, double *y1)
{
	 return (PSL_plotsegment (PSL_FORTRAN, *x0, *y0, *x1, *y1));
}

int PSL_settransparencymode_ (char *mode)
{
	return (PSL_settransparencymode (PSL_FORTRAN, mode));
}

int PSL_setfill_ (struct PSL_CTRL *PSL, double *rgb, int *outline)
{
	 return (PSL_setfill (PSL_FORTRAN, rgb, *outline));
}

int PSL_setpattern_ (int *image_no, char *imagefile, int *image_dpi, double *f_rgb, double *b_rgb, int len)
{
	 return (PSL_setpattern (PSL_FORTRAN, *image_no, imagefile, *image_dpi, f_rgb, b_rgb));
}

int PSL_setimage_ (int *image_no, char *imagefile, unsigned char *image, int *image_dpi, unsigned int *dim, double *f_rgb, double *b_rgb, int len1, int len2)
{
	return (PSL_setimage (PSL_FORTRAN, *image_no, imagefile, image, *image_dpi, dim, f_rgb, b_rgb));
}

int PSL_plotepsimage_ (double *x, double *y, double *xsize, double *ysize, int *justify, unsigned char *buffer, int size, int *nx, int *ny, int *ox, int *oy)
{
	return (PSL_plotepsimage (PSL_FORTRAN, *x, *y, *xsize, *ysize, *justify, buffer, size, *nx, *ny, *ox, *oy));
}

int PSL_plotline_ (double *x, double *y, int *n, int *type)
{
	return (PSL_plotline (PSL_FORTRAN, x, y, *n, *type));
}

int PSL_plotpoint_ (double *x, double *y, int *pen)
{
	return (PSL_plotpoint (PSL_FORTRAN, *x, *y, *pen));
}

int PSL_endplot_ (int *lastpage)
{
	return (PSL_endplot (PSL_FORTRAN, *lastpage));
}

int PSL_beginplot_ (int *orientation, int *overlay, int *color_mode, char *origin, double offset[], double *page_size, char *title, int font_no[], int len1, int len2)
{
	 return (PSL_beginplot (PSL_FORTRAN, NULL, *orientation, *overlay, *color_mode, origin, offset, page_size, title, font_no));
}

int PSL_setlinecap_ (int *cap)
{
	return (PSL_setlinecap (PSL_FORTRAN, *cap));
}

int PSL_setlinejoin_ (int *join)
{
	return (PSL_setlinejoin (PSL_FORTRAN, *join));
}

int PSL_setmiterlimit_ (int *limit)
{
	return (PSL_setmiterlimit (PSL_FORTRAN, *limit));
}

int PSL_plotbox_ (double *x0, double *y0, double *x1, double *y1)
{
	return (PSL_plotbox (PSL_FORTRAN, *x0, *y0, *x1, *y1));
}

int PSL_plotpolygon_ (double *x, double *y, int *n)
{
	return (PSL_plotpolygon (PSL_FORTRAN, x, y, *n));
}

int PSL_setdash_ (char *pattern, double *offset, int len)
{
	return (PSL_setdash (PSL_FORTRAN, pattern, *offset));
}

int PSL_setfont_ (int *font_no)
{
	return (PSL_setfont (PSL_FORTRAN, *font_no));
}

int PSL_setformat_ (int *n_decimals)
{
	return (PSL_setformat (PSL_FORTRAN, *n_decimals));
}

int PSL_setlinewidth_ (double *linewidth)
{
	 return (PSL_setlinewidth (PSL_FORTRAN, *linewidth));
}

int PSL_setcolor_ (double *rgb, int *mode)
{
	 return (PSL_setcolor (PSL_FORTRAN, rgb, *mode));
}

int PSL_setdefaults_ (double *dpi, double xyscales[], double page_rgb[])
{
	 return (PSL_setdefaults (PSL_FORTRAN, *dpi, xyscales, page_rgb));
}

int PSL_plottextbox_ (double *x, double *y, double *fontsize, char *text, double *angle, int *justify, double offset[], int *mode, int len)
{
	 return (PSL_plottextbox (PSL_FORTRAN, *x, *y, *fontsize, text, *angle, *justify, offset, *mode));
}

int PSL_deftextdim_ (char *dim, double *fontsize, char *text, int len1, int len2)
{
	return (PSL_deftextdim (PSL_FORTRAN, dim, *fontsize, text));
}

int PSL_plottext_ (double *x, double *y, double *fontsize, char *text, double *angle, int *justify, int *mode, int len)
{
	return (PSL_plottext (PSL_FORTRAN, *x, *y, *fontsize, text, *angle, *justify, *mode));
}

int PSL_plottextline_ (double x[], double y[], int np[], int n_segments, int node[], char *label[], double angle[], int nlabel_per_seg[], double fontsize, int justify, double offset[], int mode)
{
	return (PSL_plottextline (PSL_FORTRAN, x, y, np, *n_segments, node, label, angle, nlabel_per_seg, *fontsize, *justify, offset, * mode));
}

int PSL_setorigin_ (double *x, double *y, double *angle, int *mode)
{
	return (PSL_setorigin (PSL_FORTRAN, *x, *y, *angle, *mode));
}

int PSL_setparagraph_ (double *line_space, double *par_width, int *par_just)
{
	return (PSL_setparagraph (PSL_FORTRAN, *line_space, *par_width, *par_just));
}

int PSL_plotparagraphbox_ (double *x, double *y, double *fontsize, char *paragraph, double *angle, int *justify, double offset[], int *mode, int len)
{
	return (PSL_plotparagraphbox (PSL_FORTRAN, *x, *y, *fontsize, paragraph, *angle, *justify, offset, *mode));
}

int PSL_plotparagraph_ (double *x, double *y, double *fontsize, char *paragraph, double *angle, int *par_just, int len)
{
	return (PSL_plotparagraph (PSL_FORTRAN, *x, *y, *fontsize, paragraph, *angle, *par_just));
}

int PSL_defunits_ (char *param, double *value, int len) {
	return (PSL_defunits (PSL_FORTRAN, param, *value));
}

int PSL_defpoints_ (char *param, double *fontsize, int len)
{
	return (PSL_defpoints (PSL_FORTRAN, param, *fontsize));
}

int PSL_definteger_ (char *param, int *value, int len)
{
	return (PSL_definteger (PSL_FORTRAN, param, *value));
}

int PSL_defpen_ (char *param, double *linewidth, char *style, double *offset, double rgb[], int len1, int len2)
{
	return (PSL_defpen (PSL_FORTRAN, param, *linewidth, style, *offset, rgb));
}

int PSL_defcolor_ (char *param, double rgb[], int len)
{
	return (PSL_defcolor (PSL_FORTRAN, param, rgb));
}

int PSL_loadeps_ (char *file, struct imageinfo *h, unsigned char **picture, int len1, int len2)
{
	return (PSL_loadeps (PSL_FORTRAN, file, h, picture));
}
