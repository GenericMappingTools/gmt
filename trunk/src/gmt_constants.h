/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * gmt_constants.h contains definitions of constants used throught GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_CONSTANTS_H
#define _GMT_CONSTANTS_H

/*=====================================================================================
 *	GMT API CONSTANTS DEFINITIONS
 *===================================================================================*/

#include "gmt_error_codes.h"		/* All API error codes are defined here */

/*--------------------------------------------------------------------
 *			GMT CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef TWO_PI
#define TWO_PI        6.28318530717958647692
#endif
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4          0.78539816339744830962
#endif
#ifndef M_E
#define	M_E		2.7182818284590452354
#endif
#ifndef M_SQRT2
#define	M_SQRT2		1.41421356237309504880
#endif
#ifndef M_LN2_INV
#define	M_LN2_INV	(1.0 / 0.69314718055994530942)
#endif
#ifndef M_EULER
#define M_EULER		0.577215664901532860606512	/* Euler's constant (gamma) */
#endif

#define GMT_CONV_LIMIT	1.0e-8		/* Fairly tight convergence limit or "close to zero" limit */
#define GMT_SMALL	1.0e-4		/* Needed when results aren't exactly zero but close */

#define GMT_DIM_UNITS	"cip"		/* Plot dimensions in cm, inch, or point */
#define GMT_LEN_UNITS2	"efkMnu"	/* Distances in meter, foot, survey foot, km, Mile, nautical mile */
#define GMT_LEN_UNITS	"dmsefkMnu"	/* Distances in arc-{degree,minute,second} or meter, foot, km, Mile, nautical mile, survey foot */
#define GMT_DIM_UNITS_DISPLAY	"c|i|p"			/* Same, used to display as options */
#define GMT_LEN_UNITS_DISPLAY	"d|m|s|e|f|k|M|n|u"	/* Same, used to display as options */
#define GMT_LEN_UNITS2_DISPLAY	"e|f|k|M|n|u"		/* Same, used to display as options */
#define GMT_DEG2SEC_F	3600.0
#define GMT_DEG2SEC_I	3600
#define GMT_SEC2DEG	(1.0 / GMT_DEG2SEC_F)
#define GMT_DEG2MIN_F	60.0
#define GMT_DEG2MIN_I	60
#define GMT_MIN2DEG	(1.0 / GMT_DEG2MIN_F)
#define GMT_MIN2SEC_F	60.0
#define GMT_MIN2SEC_I	60
#define GMT_SEC2MIN	(1.0 / GMT_MIN2SEC_F)
#define GMT_DAY2HR_F	24.0
#define GMT_DAY2HR_I	24
#define GMT_HR2DAY	(1.0 / GMT_DAY2HR_F)
#define GMT_DAY2MIN_F	1440.0
#define GMT_DAY2MIN_I	1440
#define GMT_MIN2DAY	(1.0 / GMT_DAY2MIN_F)
#define GMT_DAY2SEC_F	86400.0
#define GMT_DAY2SEC_I	86400
#define GMT_SEC2DAY	(1.0 / GMT_DAY2SEC_F)
#define GMT_HR2SEC_F	3600.0
#define GMT_HR2SEC_I	3600
#define GMT_SEC2HR	(1.0 / GMT_HR2SEC_F)
#define GMT_HR2MIN_F	60.0
#define GMT_HR2MIN_I	60
#define GMT_MIN2HR	(1.0 / GMT_HR2MIN_F)

#define GMT_YR2SEC_F	(365.2425 * 86400.0)
#define GMT_MON2SEC_F	(365.2425 * 86400.0 / 12.0)

#define GMT_DEC_SIZE	0.54	/* Size of a decimal number compared to point size */
#define GMT_PER_SIZE	0.30	/* Size of a decimal point compared to point size */

#define GMT_PEN_LEN	128
#define GMT_PENWIDTH	0.25	/* Default pen width in points */

/* Synopsis-related constants */
#define GMT_inc_OPT	"<xinc>[<unit>][=|+][/<yinc>[<unit>][=|+]]"
#define GMT_Id_OPT	"-I<xinc>[m|s][/<yinc>[m|s]]"
#define GMT_Jx_OPT	"-Jx|X<args>"
#define GMT_Jz_OPT	"-Jz|Z<args>"
#define GMT_Rgeo_OPT	"-R<west>/<east>/<south>/<north>[r]"
#define GMT_Rgeoz_OPT	"-R<west>/<east>/<south>/<north>[/<zmin>/<zmax>][r]"
#define GMT_Rx_OPT	"-R[<unit>]<xmin>/<xmax>/<ymin>/<ymax>[r]"

/* Use b, f, h, when applies to both i and o, else use only the bi, bo, fi, fo variants */

#define GMT_bi_OPT	"-bi[<ncol>][t][w][+L|B]"
#define GMT_bo_OPT	"-bo[<ncol>][t][w][+L|B]"
#define GMT_fi_OPT	"-f<info>"
#define GMT_fo_OPT	"-f<info>"
#define GMT_ho_OPT	"-ho[<nrecs>][+c][+d][+r<remark>][+t<title>]"

/* For options needing a length or radius */

#define GMT_DIST_OPT	"[-|+]<dist>[<unit>]"
#define GMT_RADIUS_OPT	"[-|+]<radius>[<unit>]"

/* Options for map rose, scale and insert, used in pscoast and psbasemap */

#define GMT_TROSE	"[f|m][x]<lon0>/<lat0>/<diameter>[/<info>][:w,e,s,n:][+<gint>[/<mint>]]"
#define GMT_SCALE	"[f][x]<lon0>/<lat0>[/<slon>]/<slat>/<length>[e|f|M|n|k|u][+l<label>][+j<just>][+p<pen>][+f<fill>][+u]"
#define GMT_INSERT	"[<u>]<xmin>/<xmax>/<ymin>/<ymax>[r]|<width>[/<height>][+c<lon>/<lat>][+p<pen>][+f<fill>]"

/* Argument to *contour programs */

#define GMT_CONTG	"-G[d|f|n|l|L|x|X]<args>"
#define GMT_CONTT	"-T[+|-][<gap>[c|i|p]/<length>[c|i|p]][:[<labels>]]"

/* Options for coastline extraction  */
#ifdef NEW_GSHHG
#define GMT_A_OPT       "-A<min_area>[/<min_level>/<max_level>][+ag|i|s][+r|l][+p<percent>]"
#else
#define GMT_A_OPT       "-A<min_area>[/<min_level>/<max_level>][+as][+r|l][+p<percent>]"
#endif

/* Used in tools that sets grdheader information via a -D option */

#define GMT_GRDEDIT	"-D<xname>/<yname>/<zname>/<scale>/<offset>/<invalid>/<title>/<remark>"


#endif  /* _GMT_CONSTANTS_H */
