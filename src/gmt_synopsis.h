/*--------------------------------------------------------------------
 *	$Id: gmt_synopsis.h,v 1.19 2011-03-28 22:34:28 remko Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Contains macros for presenting the GMT common options in program
 * synopsis - yielding consistent presentation from all programs.
 * See common_options.txt for macros for the man pages.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#ifndef GMT_SYNOPSIS_H
#define GMT_SYNOPSIS_H

#define GMT_B_OPT	"-B<args>"
#define GMT_I_OPT	"-I<xinc>[u][=|+][/<yinc>[u][=|+]]"
#define GMT_inc_OPT	"<xinc>[u][=|+][/<yinc>[u][=|+]]"
#define GMT_Id_OPT	"-I<xinc>[m|c][/<yinc>[m|c]]"
#define GMT_J_OPT	"-J<args>"
#define GMT_Jx_OPT	"-Jx|X<args>"
#define GMT_Jz_OPT	"-Jz|Z<args>"
#define GMT_Rgeo_OPT	"-R<west>/<east>/<south>/<north>[r]"
#define GMT_Rgeoz_OPT	"-R<west>/<east>/<south>/<north>[/<zmin/zmax>][r]"
#define GMT_Rx_OPT	"-R<xmin>/<xmax>/<ymin>/<ymax>[r]"
#define GMT_U_OPT	"-U[<just>/<dx>/<dy>/][c|<label>]"
#define GMT_V_OPT	"-V[<level>]"
#define GMT_X_OPT	"-X[a|c|r]<xshift>[u]"
#define GMT_Y_OPT	"-Y[a|c|r]<yshift>[u]"

/* Use b, f, h, when applies to both i and o, else use only the bi, bo, fi, fo variants */

#define GMT_a_OPT	"-a<col>=<name>[,...]"
#define GMT_b_OPT	"-b[i|o][s|S|d|D[<ncol>]|c[<var1>/...]]"
#define GMT_c_OPT	"-c<ncopies>"
#define GMT_bi_OPT	"-bi[s|S|d|D[<ncol>]|c[<var1>/...]]"
#define GMT_bo_OPT	"-bo[s|S|d|D[<ncol>]|c[<var1>/...]]"
#define GMT_f_OPT	"-f[i|o]<info>"
#define GMT_fi_OPT	"-f<info>"
#define GMT_fo_OPT	"-f<info>"
#define GMT_g_OPT	"-g[a]x|y|d|X|Y|D|[<col>]z[-|+]<gap>[unit]"
#define GMT_h_OPT	"-h[i|o][<nrec>]"
#define GMT_ho_OPT	"-ho[<nrec>]"
#define GMT_i_OPT	"-i<cols>[l][s<scale>][o<offset>][,...]"
#define GMT_o_OPT	"-o<cols>[,...]"
#define GMT_p_OPT	"-p[x|y|z]<azim>/<elev>[/zlevel][+w<lon0>/<lat0>[/<z0>][+v<x0>/<y0>]"
#define GMT_r_OPT	"-r"
#define GMT_s_OPT	"-s[<cols>][r]"
#define GMT_t_OPT	"-t<transp>"
#define GMT_colon_OPT	"-:[i|o]"

/* For options needing a length or radius */

#define GMT_DIST_OPT	"[-|+]<dist>[unit]"
#define GMT_RADIUS_OPT	"[-|+]<radius>[unit]"

/* Options for map rose and scale, used in pscoast and psbasemap */

#define GMT_TROSE	"-T[f|m][x]<lon0>/<lat0>/<diameter>[/<info>][:w,e,s,n:][+<gint>[/<mint>]]"
#define GMT_SCALE	"-L[f][x]<lon0>/<lat0>[/<slon>]/<slat>/<length>[m|n|k][+l<label>][+j<just>][+p<pen>][+f<fill>][+u]"

/* Argument to *contour programs */

#define GMT_CONTG	"-G[d|f|n|l|L|x|X]<args>"
#define GMT_CONTT	"-T[+|-][<gap>[c|i|p]/<length>[c|i|p]][:[<labels>]]"

/* Options for coastline extraction  */
#define GMT_A_OPT       "-A<min_area>[/<min_level>/<max_level>][+r|l][+p<percent>]"

/* Used in tools that sets grdheader information via a -D option */

#define GMT_GRDEDIT	"-D<xname>/<yname>/<zname>/<scale>/<offset>/<title>/<remark>"

#endif /* GMT_SYNOPSIS_H */
