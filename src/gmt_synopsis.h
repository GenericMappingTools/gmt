/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *--------------------------------------------------------------------*/
/*
 * Contains macros for presenting variations of GMT common options in program
 * synopsis - yielding consistent presentation from all programs.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 */

/*!
 * \file gmt_synopsis.h
 * \brief Macros for presenting variations of GMT common options in program synopsis.
 */

#ifndef GMT_SYNOPSIS_H
#define GMT_SYNOPSIS_H

#define GMT_inc_OPT	"<xinc>[<unit>][+e|n][/<yinc>[<unit>][+e|n]]"
#define GMT_Id_OPT	"-I<xinc>[m|s][/<yinc>[m|s]]"
#define GMT_Jx_OPT	"-Jx|X<args>"
#define GMT_Jz_OPT	"-Jz|Z<args>"
#define GMT_Rgeo_OPT	"-R<west>/<east>/<south>/<north>[+r]"
#define GMT_Rgeoz_OPT	"-R<west>/<east>/<south>/<north>[/<zmin>/<zmax>][+r]"
#define GMT_Rx_OPT	"-R[<unit>]<xmin>/<xmax>/<ymin>/<ymax>[+r]"

/* Use b, f, h, when applies to both i and o, else use only the bi, bo, fi, fo variants */

#define GMT_bi_OPT	"-bi[<ncol>][t][w][+l|b]"
#define GMT_bo_OPT	"-bo[<ncol>][t][w][+l|b]"
#define GMT_di_OPT	"-di<nodata>"
#define GMT_do_OPT	"-do<nodata>"
#define GMT_fi_OPT	"-f<info>"
#define GMT_fo_OPT	"-f<info>"
#define GMT_ho_OPT	"-ho[<nrecs>][+c][+d][+r<remark>][+t<title>]"
#define GMT_PAR_OPT	"--PAR=<value>"

#ifdef GMT_MP_ENABLED
#define GMT_x_OPT	" [-x[[-]<n>]] "	/* Must add spaces and brackets here and place via %s since it may be blank */
#define GMT_ADD_x_OPT	"x"
#else	/* No Open-MP support */
#define GMT_x_OPT	" "
#define GMT_ADD_x_OPT	""
#endif

/* For options needing a length or radius */

#define GMT_DIST_OPT	"<dist>[<unit>]"
#define GMT_RADIUS_OPT	"<radius>[<unit>]"

/* Options for map rose, scale and insert, used in pscoast and psbasemap */

#define GMT_XYANCHOR	"[g|j|J|n|x]<refpoint>"
#define GMT_JUSTIFY	"[+j<justify>]"
#define GMT_OFFSET	"[+o<dx>[/<dy>]]"
#define GMT_TROSE_DIR	GMT_XYANCHOR "+w<width>[+f[<level>]]" GMT_JUSTIFY "[+l<w,e,s,n>]" GMT_OFFSET
#define GMT_TROSE_MAG	GMT_XYANCHOR "+w<width>[+d[<dec>[/<dlabel>]]][+i<pen>]" GMT_JUSTIFY "[+l<w,e,s,n>][+p<pen>][+t<ints>]" GMT_OFFSET
#define GMT_SCALE	GMT_XYANCHOR "+c[<slon>/]<slat>+w<length>[e|f|M|n|k|u][+a<align>][+f]" GMT_JUSTIFY "[+l[<label>]]" GMT_OFFSET "[+u]"
#define GMT_INSET_A	GMT_XYANCHOR "+w<width>[<unit>][/<height>[<unit>]]" GMT_JUSTIFY GMT_OFFSET
#define GMT_INSET_B	"<xmin>/<xmax>/<ymin>/<ymax>[+r][+u<unit>]"
#define GMT_INSET_A_CL	GMT_XYANCHOR "+w<width>[<unit>][/<height>[<unit>]]" GMT_JUSTIFY GMT_OFFSET "[+s<file>][+t]"
#define GMT_INSET_B_CL	"<xmin>/<xmax>/<ymin>/<ymax>[+r][+s<file>][+t][+u<unit>]"

/* Options for background panel, used in gmtlogo, psimage, pslegend and psscale */

#define GMT_PANEL	"[+c<clearance(s)>][+g<fill>][+i[[<gap>/]<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<fill>]]"

/* Argument to *contour programs */

#define GMT_CONTG	"-G[d|f|n|l|L|x|X]<args>"
#define GMT_CONTT	"-T[h|l][+a][+d<gap>[c|i|p][/<length>[c|i|p]]][+l[<labels>]]"

/* Options for coastline extraction  */
#define GMT_A_OPT       "-A<min_area>[/<min_level>/<max_level>][+a[g|i][s|S]][+r|l][+p<percent>]"

/* Used in tools that sets grdheader information via a -D option */

#define GMT_GRDEDIT	"-D[+x<xname>][+y<yname>][+z<zname>][+s<scale>][+o<offset>][+n<invalid>][+t<title>][+r<remark>]"

/*! Macros for the common GMT options used in a program's usage synopsis */

#define GMT_B_OPT	"-B<args>"
#define GMT_I_OPT	"-I<xinc>[<unit>][+e|n][/<yinc>[<unit>][+e|n]]"
#define GMT_J_OPT	"-J<args>"
#define GMT_R2_OPT	"-R<xmin>/<xmax>/<ymin>/<ymax>[+u<unit>][+r]"
#define GMT_R3_OPT	"-R<xmin>/<xmax>/<ymin>/<ymax>[/<zmin>/<zmax>][+u<unit>][+r]"
#define GMT_U_OPT	"-U[<label>][+c][+j<just>][+o<dx>/<dy>]"
#define GMT_V_OPT	"-V[<level>]"
#define GMT_X_OPT	"-X[a|c|f|r]<xshift>[<unit>]"
#define GMT_Y_OPT	"-Y[a|c|f|r]<yshift>[<unit>]"
#define GMT_a_OPT	"-a<col>=<name>[,...]"
#define GMT_b_OPT	"-b[i|o][<ncol>][t][w][+l|b]"
#define GMT_d_OPT	"-d[i|o]<nodata>"
#define GMT_e_OPT	"-e[~]<pattern>"
#define GMT_f_OPT	"-f[i|o]<info>"
#define GMT_g_OPT	"-g[a]x|y|d|X|Y|D|[<col>]z<gap>[<unit>][+n|p]"
#define GMT_h_OPT	"-h[i|o][<nrecs>][+c][+d][+r<remark>][+t<title>]"
#define GMT_i_OPT	"-i<cols>[+l][+s<scale>][+o<offset>][,...][,t[<word>]]"
#define GMT_j_OPT	"-je|f|g"
#define GMT_l_OPT	"-l<label>[<mods>]"
#define GMT_n_OPT	"-n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>]"
#define GMT_o_OPT	"-o<cols>[,...][t[<word>]]"
#define GMT_p_OPT	"-p[x|y|z]<azim>[/<elev>[/<zlevel>]][+w<lon0>/<lat0>[/<z0>][+v<x0>/<y0>]"
#define GMT_r_OPT	"-r[g|p]"
#define GMT_s_OPT	"-s[<cols>][+a|r]"
#define GMT_t_OPT	"-t<transp>"
#define GMT_tv_OPT	"-t[<transp>]"
#define GMT_colon_OPT	"-:[i|o]"

/*! Macro for tools that need to specify FFT information (prepend option flag, e.g., -N and put GMT_FFT_OPT inside [] ) */

#define GMT_FFT_OPT "[f|s|<n_columns>/<n_rows>][+a|d|l][+e|m|n][+t<width>][+v][+w<suffix>][+z[p]]"

#endif /* GMT_SYNOPSIS_H */
