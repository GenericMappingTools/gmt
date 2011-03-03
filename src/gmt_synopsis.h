/*--------------------------------------------------------------------
 *	$Id: gmt_synopsis.h,v 1.17 2011-03-03 21:02:51 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
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
 *--------------------------------------------------------------------*/
/*
 * Contains macros for presenting the GMT common options in program
 * synopsis - yielding consistent presentation from all programs.
 * See common_options.txt for macros for the man pages.
 *
 * Author:	Paul Wessel
 * Date:	6-APR-2006
 * Version:	4
 *
 */

#ifndef GMT_SYNOPSIS_H
#define GMT_SYNOPSIS_H

#define GMT_B_OPT	"-B<params>"
#define GMT_H_OPT	"-H[i][<nrec>]"
#define GMT_Ho_OPT	"-H[<nrec>]"
#define GMT_I_OPT	"-I<xinc>[u][=|+][/<yinc>[u][=|+]]"
#define GMT_inc_OPT	"<xinc>[u][=|+][/<yinc>[u][=|+]]"
#define GMT_Id_OPT	"-I<xinc>[m|c][/<yinc>[m|c]]"
#define GMT_J_OPT	"-J<params>"
#define GMT_Jx_OPT	"-Jx|X<params>"
#define GMT_Jz_OPT	"-Jz|Z<params>"
#define GMT_Rgeo_OPT	"-R<west>/<east>/<south>/<north>[r]"
#define GMT_Rgeoz_OPT	"-R<west>/<east>/<south>/<north>[/<zmin/zmax>][r]"
#define GMT_Rx_OPT	"-R<xmin>/<xmax>/<ymin>/<ymax>[r]"
#define GMT_U_OPT	"-U[<just>/<dx>/<dy>/][c|<label>]"
#define GMT_X_OPT	"-X[a|c|r]<x_shift>[u]"
#define GMT_Y_OPT	"-Y[a|c|r]<x_shift>[u]"
#define GMT_c_OPT	"-c<ncopies>"
#define GMT_t_OPT	"-:[i|o]"

/* Use b, f when applies to both i and o, else use only the bi, bo, fi, fo variants */

#define GMT_b_OPT	"-b[i|o][s|S|d|D[<ncol>]|c[<var1>/...]]"
#define GMT_bi_OPT	"-bi[s|S|d|D[<ncol>]|c[<var1>/...]]"
#define GMT_bo_OPT	"-bo[s|S|d|D[<ncol>]|c[<var1>/...]]"
#define GMT_f_OPT	"-f[i|o]<colinfo>"
#define GMT_fi_OPT	"-f<colinfo>"
#define GMT_fo_OPT	"-f<colinfo>"
#define GMT_g_OPT	"-g[a]x|y|d|X|Y|D|[<col>]z[-|+]<gap>[unit]"
#define GMT_m_OPT	"-m[i|o][<flag>]"
#define GMT_mi_OPT	"-m[<flag>]"
#define GMT_mo_OPT	"-m[<flag>]"

/* Options for map rose and scale, used in pscoast and psbasemap */

#define GMT_TROSE	"-T[f|m][x]<lon0>/<lat0>/<diameter>[/<info>][:w,e,s,n:][+<gint>[/<mint>]]"
#define GMT_SCALE	"-L[f][x]<lon0>/<lat0>[/<slon>]/<slat>/<length>[m|n|k][+l<label>][+j<just>][+p<pen>][+f<fill>][+u]"

/* Argument to *contour programs */

#define GMT_CONTG	"-G[d|f|n|l|L|x|X]<params>"

/* Options for pseudo perspective views */
#define GMT_E_OPT       "-E<azim>/<elev>[+w<lon>/<lat>[<z>][+v<x0>/<y0>]"

/* Options for coastline extraction  */
#define GMT_A_OPT       "-A<min_area>[/<min_level>/<max_level>][+r|l][+p<percent>]"

/* Used in tools that sets grdheader information via a -D option */

#define GMT_GRDEDIT	"-D<xname>/<yname>/<zname>/<scale>/<offset>/<title>/<remark>"

#endif /* GMT_SYNOPSIS_H */
