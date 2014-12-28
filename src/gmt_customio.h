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
 *--------------------------------------------------------------------*/
 
/*
 * Include file for gmt_customio functions.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#ifndef GMT_CUSTOMIO_H
#define GMT_CUSTOMIO_H

/* Definition for Sun rasterfiles [THESE MUST REMAIN int32_t]  */
struct rasterfile {
	int32_t magic;     /* magic number */
	int32_t width;     /* width (pixels) of image */
	int32_t height;    /* height (pixels) of image */
	int32_t depth;     /* depth (1, 8, or 24 bits) of pixel */
	int32_t length;    /* length (bytes) of image */
	int32_t type;      /* type of file; see RT_* below */
	int32_t maptype;   /* type of colormap; see RMT_* below */
	int32_t maplength; /* length (bytes) of following map */
	/* color map follows for maplength bytes, followed by image */
};

#define	RAS_MAGIC	0x59a66a95	/* Magic number for Sun rasterfile */
#define EPS_MAGIC	0x25215053	/* Magic number for EPS file */
#define RT_OLD		0		/* Old-style, unencoded Sun rasterfile */
#define RT_STANDARD	1		/* Standard, unencoded Sun rasterfile */
#define RT_BYTE_ENCODED	2		/* Run-length-encoded Sun rasterfile */
#define RT_FORMAT_RGB	3		/* [X]RGB instead of [X]BGR Sun rasterfile */
#define RMT_NONE	0		/* maplength is expected to be 0 */
#define RMT_EQUAL_RGB	1		/* red[maplength/3], green[], blue[] follow */

EXTERN_MSC char **GMT_grdformats_sorted (struct GMT_CTRL *Ctrl);

#endif /* GMT_CUSTOMIO_H */
