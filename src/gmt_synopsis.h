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
 * Contains macros for presenting variations of GMT common options in program
 * synopsis - yielding consistent presentation from all programs.
 * See gmt_option.h for the GMT common options part of the API.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#ifndef GMT_SYNOPSIS_H
#define GMT_SYNOPSIS_H

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
#define GMT_SCALE	"[f][x]<lon0>/<lat0>[/<slon>]/<slat>/<length>[e|f|M|n|k|u][+l<label>][+j<just>][+p<pen>][+g<fill>][+u]"
#define GMT_INSERT	"[<u>]<xmin>/<xmax>/<ymin>/<ymax>[r]|<width>[/<height>][+c<lon>/<lat>][+p<pen>][+g<fill>]"

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

#endif /* GMT_SYNOPSIS_H */
