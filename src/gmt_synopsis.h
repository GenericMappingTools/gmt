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
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

/*!
 * \file gmt_synopsis.h
 * \brief Macros for presenting variations of GMT common options in program synopsis.
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
#define GMT_di_OPT	"-di<nodata>"
#define GMT_do_OPT	"-do<nodata>"
#define GMT_fi_OPT	"-f<info>"
#define GMT_fo_OPT	"-f<info>"
#define GMT_ho_OPT	"-ho[<nrecs>][+c][+d][+r<remark>][+t<title>]"

/* For options needing a length or radius */

#define GMT_DIST_OPT	"[-|+]<dist>[<unit>]"
#define GMT_RADIUS_OPT	"[-|+]<radius>[<unit>]"

/* Options for map rose, scale and insert, used in pscoast and psbasemap */

#define GMT_TROSE	"[f|m][x]<lon0>/<lat0>/<diameter>[/<info>][:w,e,s,n:][+<gint>[/<mint>]]"
#define GMT_SCALE	"[f][x]<lon0>/<lat0>[/<slon>]/<slat>/<length>[e|f|M|n|k|u][+l<label>][+j<just>][+u]"
#define GMT_INSERT_A	"[g|j|n|x]<anchor>/<width>[<unit>]/<height>[<unit>][/<justify>[/<dx>/<dy>]]"
#define GMT_INSERT_B	"[<unit>]<xmin>/<xmax>/<ymin>/<ymax>[r]"

/* Options for background panel, used in gmtlogo, psimage, pslegend and psscale */

#define GMT_PANEL	"-F[+c<clearance(s)>][+g<fill>][+i[[<gap>/]<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<fill>]]"

/* Argument to *contour programs */

#define GMT_CONTG	"-G[d|f|n|l|L|x|X]<args>"
#define GMT_CONTT	"-T[+|-][<gap>[c|i|p]/<length>[c|i|p]][:[<labels>]]"

/* Options for coastline extraction  */
#define GMT_A_OPT       "-A<min_area>[/<min_level>/<max_level>][+ag|i|s|S][+r|l][+p<percent>]"

/* Used in tools that sets grdheader information via a -D option */

#define GMT_GRDEDIT	"-D<xname>/<yname>/<zname>/<scale>/<offset>/<invalid>/<title>/<remark>"

/*! Macros for the common GMT options used in a program's usage synopsis */

#define GMT_B_OPT	"-B<args>"
#define GMT_I_OPT	"-I<xinc>[<unit>][=|+][/<yinc>[<unit>][=|+]]"
#define GMT_J_OPT	"-J<args>"
#define GMT_R2_OPT	"-R[<unit>]<xmin>/<xmax>/<ymin>/<ymax>[r]"
#define GMT_R3_OPT	"-R[<unit>]<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>][r]"
#define GMT_U_OPT	"-U[<just>/<dx>/<dy>/][c|<label>]"
#define GMT_V_OPT	"-V[<level>]"
#define GMT_X_OPT	"-X[a|c|r]<xshift>[<unit>]"
#define GMT_Y_OPT	"-Y[a|c|r]<yshift>[<unit>]"
#define GMT_a_OPT	"-a<col>=<name>[,...]"
#define GMT_b_OPT	"-b[i|o][<ncol>][t][w][+L|B]"
#define GMT_c_OPT	"-c<ncopies>"
#define GMT_d_OPT	"-d[i|o]<nodata>"
#define GMT_f_OPT	"-f[i|o]<info>"
#define GMT_g_OPT	"-g[a]x|y|d|X|Y|D|[<col>]z[-|+]<gap>[<unit>]"
#define GMT_h_OPT	"-h[i|o][<nrecs>][+c][+d][+r<remark>][+t<title>]"
#define GMT_i_OPT	"-i<cols>[l][s<scale>][o<offset>][,...]"
#define GMT_n_OPT	"-n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>]"
#define GMT_o_OPT	"-o<cols>[,...]"
#define GMT_p_OPT	"-p[x|y|z]<azim>/<elev>[/<zlevel>][+w<lon0>/<lat0>[/<z0>][+v<x0>/<y0>]"
#define GMT_r_OPT	"-r"
#define GMT_s_OPT	"-s[<cols>][a|r]"
#define GMT_t_OPT	"-t<+a|[-]n>"
#define GMT_x_OPT	"-x<threads>"
#define GMT_colon_OPT	"-:[i|o]"

/*! Macro for tools that need to specify FFT information (prepend option flag, e.g., -N and put GMT_FFT_OPT inside [] ) */

#define GMT_FFT_OPT "[f|q|s|<nx>/<ny>][+a|d|l][+e|m|n][+t<width>][+w<suffix>][+z[p]]"

#endif /* GMT_SYNOPSIS_H */
