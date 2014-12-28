/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2012-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * gmt_option.h contains the definitions for the GMT_OPTION structure
 * option-related named constants, and macros for synopsis writing.
 *
 * Author:	Paul Wessel
 * Date:	17-MAY-2013
 * Version:	5 API
 */

#ifndef _GMT_OPTION_H
#define _GMT_OPTION_H

/*============================================================ */
/*=============== GMT_OPTION Public Declaration ============== */
/*============================================================ */

/* Macros for the common GMT options used in a program's usage synopsis */

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
#define GMT_f_OPT	"-f[i|o]<info>"
#define GMT_g_OPT	"-g[a]x|y|d|X|Y|D|[<col>]z[-|+]<gap>[<unit>]"
#define GMT_h_OPT	"-h[i|o][<nrecs>][+c][+d][+r<remark>][+t<title>]"
#define GMT_i_OPT	"-i<cols>[l][s<scale>][o<offset>][,...]"
#define GMT_n_OPT	"-n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>]"
#define GMT_o_OPT	"-o<cols>[,...]"
#define GMT_p_OPT	"-p[x|y|z]<azim>/<elev>[/<zlevel>][+w<lon0>/<lat0>[/<z0>][+v<x0>/<y0>]"
#define GMT_r_OPT	"-r"
#define GMT_s_OPT	"-s[<cols>][a|r]"
#define GMT_t_OPT	"-t<transp>"
#define GMT_colon_OPT	"-:[i|o]"

/* Macro for tools that need to specify FFT information (prepend option flag, e.g., -N and put GMT_FFT_OPT inside [] ) */

#define GMT_FFT_OPT "[f|q|s|<nx>/<ny>][+a|d|l][+e|m|n][+t<width>][+w<suffix>][+z[p]]"

/* Macros for printing a tic/toc elapsed time message*/

#define GMT_tic(C) {if (C->current.setting.verbose >= GMT_MSG_TICTOC) GMT_Message(C->parent,GMT_TIME_RESET,"");}
#define GMT_toc(C,...) {if (C->current.setting.verbose >= GMT_MSG_TICTOC) GMT_Message(C->parent,GMT_TIME_ELAPSED, \
		"(%s) | %s\n", C->init.module_name, __VA_ARGS__);}

/* These are the 5 named option codes */
enum GMT_enum_opt {
	GMT_OPT_USAGE =     '?',	/* Command-line option for full usage */
	GMT_OPT_SYNOPSIS =  '^',	/* Command-line option for synopsis */
	GMT_OPT_PARAMETER = '-',	/* Command-line option for GMT defaults parameter */
	GMT_OPT_INFILE =    '<',	/* Command-line option for input file */
	GMT_OPT_OUTFILE =   '>'};	/* Command-line option for output file */

/* This struct is used to pass program options in/out of GMT modules */

struct GMT_OPTION {              /* Structure for a single GMT command option */
	char option;                 /* 1-char command line -<option> (e.g. D in -D) identifying the option (* if file) */
	char *arg;                   /* If not NULL, contains the argument for this option */
	struct GMT_OPTION *next;     /* Pointer to next option in a linked list */
	struct GMT_OPTION *previous; /* Pointer to previous option in a linked list */
};

#endif /* _GMT_OPTION_H */
