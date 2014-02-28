/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * gmt_dwc.h contains definitions for using the DCW in GMT.
 * DCW - The Digital Chart of the World
 *
 * Author:	Paul Wessel
 * Date:	10-MAR-2013
 * Version:	5 API
 */

#ifndef _GMT_DCW_H
#define _GMT_DCW_H

#define DCW_OPT "-F<code1,code2,...>[+l|L][+g<fill>][+p<pen>][+r|R[<incs>]]"

enum GMT_DCW_modes {
	GMT_DCW_REGION	= 1,
	GMT_DCW_PLOT	= 2,
	GMT_DCW_DUMP	= 4,
	GMT_DCW_EXTRACT	= 8
};

struct GMT_DCW_SELECT {	/* -F<DWC-options> */
	bool region;		/* Determine region from polygons instead of -R */
	bool adjust;		/* Round/adjust the region from polygons using the incs */
	bool extend;		/* Extend region rather than quantize it */
	char *codes;		/* Comma separated list of codes with modifiers */
	unsigned int mode;	/* 0 = get -R, 1 = paint, 2 = dump */
	double inc[4];		/* Increments for rounded region */
	struct GMT_PEN pen;	/* Pen for outline [no outline] */
	struct GMT_FILL fill;	/* Fill for polygons */
};

EXTERN_MSC unsigned int GMT_DCW_list (struct GMT_CTRL *GMT, unsigned list_mode);
EXTERN_MSC unsigned int GMT_DCW_parse (struct GMT_CTRL *GMT, char option, char *args, struct GMT_DCW_SELECT *F);
EXTERN_MSC void GMT_DCW_option (struct GMTAPI_CTRL *API, char option, unsigned int plot);
EXTERN_MSC struct GMT_DATASET * GMT_DCW_operation (struct GMT_CTRL *GMT, struct GMT_DCW_SELECT *F, double wesn[], unsigned int mode);

#endif /* _GMT_DCW_H */
