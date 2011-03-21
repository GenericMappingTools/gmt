/*--------------------------------------------------------------------
 *	$Id: grdraster_func.c,v 1.3 2011-03-21 18:36:46 guru Exp $
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
/* grdraster.c -- read a rasterfile and extract a region as a grid file.
 *
 * This is a complete rewrite for GMT version 3.0.  This is based on
 * the earlier versions written by me for installations at Scripps and
 * NOAA, and does not resemble the version grdraster.c_supplied which
 * was used at Hawaii.
 *
 * Author:	Walter H. F. Smith
 * Date:	1 April, 2010
 * Version:	5 API
 *
 * Brief synopsis: grdraster reads and extracts a subset of large (global)
 * grids using metadata from the grdraster.info table.
 */

#include "gmt_dbase.h"

#if WORDS_BIGENDIAN == 0
#define MY_ENDIAN 'L'	/* This machine is Little endian */
#else
#define MY_ENDIAN 'B'	/* This machine is Little endian */
#endif

struct GRDRASTER_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct G {	/* -G<output_grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double xinc, yinc;
	} I;
	struct T {	/* -T<output_table> */
		GMT_LONG active;
		char *file;
	} T;
};

struct GRDRASTER_INFO {
	struct GRD_HEADER h;
	GMT_LONG id;		/* File number  */
	GMT_LONG nglobal;	/* If not 0, ras is global and i%nglobal makes it periodic  */
	GMT_LONG nanflag;
	GMT_LONG nanset;	/* True if raster uses nanflag to signal NaN  */
	GMT_LONG skip;		/* Skip this number of header bytes when opening file  */
	GMT_LONG swap_me;	/* TRUE if data set need to be swapped */
	GMT_LONG geo;		/* TRUE if we believe x/y is lon/lat, FALSE otherwise */
	char type;
};

void convert_u_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO ras, float *row, unsigned char *buffer)
{
	GMT_LONG i, tempval;
	for (i = 0; i < ras.h.nx; i++) {
		tempval = (GMT_LONG)buffer[i];
		if (ras.nanset && tempval == ras.nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras.h.z_scale_factor != 1.0) row[i] *= (float)ras.h.z_scale_factor;
			if (ras.h.z_add_offset != 0.0) row[i] += (float)ras.h.z_add_offset;
		}
	}
	return;
}

void convert_c_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO ras, float *row, char *buffer)
{
	GMT_LONG	i, tempval;
	for (i = 0; i < ras.h.nx; i++) {
		tempval = (GMT_LONG)buffer[i];
		if (ras.nanset && tempval == ras.nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras.h.z_scale_factor != 1.0) row[i] *= (float)ras.h.z_scale_factor;
			if (ras.h.z_add_offset != 0.0) row[i] += (float)ras.h.z_add_offset;
		}
	}
	return;
}

void convert_d_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO ras, float *row, short unsigned int *buffer)
{
	GMT_LONG	i, tempval;
	for (i = 0; i < ras.h.nx; i++) {
		if (ras.swap_me) buffer[i] = GMT_swab2 (buffer[i]);

		tempval = buffer[i];
		if (ras.nanset && tempval == ras.nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras.h.z_scale_factor != 1.0) row[i] *= (float)ras.h.z_scale_factor;
			if (ras.h.z_add_offset != 0.0) row[i] += (float)ras.h.z_add_offset;
		}
	}
	return;
}

void convert_i_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO ras, float *row, short int *buffer)
{
	GMT_LONG	i, tempval;
	for (i = 0; i < ras.h.nx; i++) {
		if (ras.swap_me) buffer[i] = GMT_swab2 (buffer[i]);

		tempval = buffer[i];
		if (ras.nanset && tempval == ras.nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras.h.z_scale_factor != 1.0) row[i] *= (float)ras.h.z_scale_factor;
			if (ras.h.z_add_offset != 0.0) row[i] += (float)ras.h.z_add_offset;
		}
	}
	return;
}

void convert_l_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO ras, float *row, int *buffer)
{
	GMT_LONG	i, tempval;
	for (i = 0; i < ras.h.nx; i++) {
		if (ras.swap_me) buffer[i] = GMT_swab4 (buffer[i]);

		tempval = buffer[i];
		if (ras.nanset && tempval == ras.nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras.h.z_scale_factor != 1.0) row[i] *= (float)ras.h.z_scale_factor;
			if (ras.h.z_add_offset != 0.0) row[i] += (float)ras.h.z_add_offset;
		}
	}
	return;
}

GMT_LONG get_byte_size (struct GMT_CTRL *GMT, char type) {
	/* Return byte size of each item, or 0 if bits */
	int ksize;
	switch (type) {
		case 'b':
			ksize = 0;
			break;
		case 'c':
		case 'u':
			ksize = 1;
			break;
		case 'd':
		case 'h':
		case 'i':
			ksize = 2;
			break;
		case 'l':
			ksize = 4;
			break;
		default:
			GMT_message (GMT, "ERROR: Invalid data type [%c]\n", (int)type);
			return (EXIT_FAILURE);
			break;
	}
	return (ksize);
}

GMT_LONG load_rasinfo (struct GMT_CTRL *GMT, struct GRDRASTER_INFO **ras, char endian)
{
	/* Read the file grdraster.info
		Store the i'th row of the file in rasinfo[i].h.command.
		Store the filename in rasinfo[i].h.remark.
		Store the description in rasinfo[i].h.title.
		Store the units in rasinfo[i].h.z_units.
		After all has parsed correctly, truncate rasinfo[i].h.command
			so it can be printed out as an abbreviated description
			for the user.
		Figure out if file is global, and set nglobal.
		Set nx and ny also.

	Return 0 if cannot read files correctly, or nrasters if successful.  */

	GMT_LONG i, j, length, stop_point, nfound = 0, ksize = 0, n_alloc, expected_size, object_ID, n_fields, delta, error = 0;
	double global_lon, lon_tol;
	char path[BUFSIZ], buf[GRD_REMARK_LEN], dir[GRD_REMARK_LEN], *l = NULL, *record = NULL, *file = NULL;
	struct GRDRASTER_INFO *rasinfo = NULL;
	struct STAT F;

	/* Find and open the file grdraster.info */

	if (!(GMT_getdatapath (GMT, "grdraster.info", dir) || GMT_getsharepath (GMT, "dbase", "grdraster", ".info", dir))) {
		GMT_report (GMT, GMT_MSG_FATAL, "ERROR cannot find file grdraster.info\n");
		return (0);
	}
	file = dir;

	if (GMT_Register_IO (GMT->parent, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_TEXT, GMT_IN, (void **)&file, NULL, NULL, &object_ID)) return (0);

	if ((error = GMT_Begin_IO (GMT->parent, GMT_IS_TEXTSET, GMT_IN, GMT_BY_REC))) return (1);	/* Enables data input and sets access mode */

	/* Truncate the pathname of grdraster.info to just the directory name */

	if ((l = strstr (dir, "grdraster.info"))) *l = '\0';

	n_alloc = GMT_SMALL_CHUNK;
	rasinfo = GMT_memory (GMT, NULL, n_alloc, struct GRDRASTER_INFO);
	
	while ((n_fields = GMT_Get_Record (GMT->parent, GMT_READ_TEXT, (void **)&record)) != EOF) {	/* Keep returning records until we reach EOF */
		if (GMT_REC_IS_ERROR (GMT)) return (GMT_RUNTIME_ERROR);

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;		/* Skip segment headers */
		
		/* Strip off trailing "\r\n" */
		GMT_chop (record);
		length = strlen(record);
		if (length == 0) continue;	/* Skip blank lines */

		strcpy (rasinfo[nfound].h.command, record);

		/* Find the integer file name first:  */
		i = 0;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' ||  rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (File number conversion error).\n");
			return(0);
		}
		j = i+1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' ||  rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (File number conversion error).\n");
			return(0);
		}
		
		strncpy (buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i] = '\0';
		if ( (sscanf(buf, "%ld", &rasinfo[nfound].id) ) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (File number conversion error).\n");
			return(0);
		}

		/* Now find the title string:  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] != '"') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Title string conversion error).\n");
			return(0);
		}
		j = i+1;
		while (j < length && (rasinfo[nfound].h.command[j] != '"') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Title string conversion error).\n");
			return(0);
		}
		i++;
		if (i == j) {
			GMT_message (GMT, "Error reading grdraster.info (Title string conversion error).\n");
			return(0);
		}
		strncpy(rasinfo[nfound].h.title, &rasinfo[nfound].h.command[i], (size_t)j-i);
		rasinfo[nfound].h.title[j-i] = '\0';

		/* Now find the z_unit string:  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] != '"') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Units string conversion error).\n");
			return(0);
		}
		j = i+1;
		while (j < length && (rasinfo[nfound].h.command[j] != '"') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Units string conversion error).\n");
			return(0);
		}
		i++;
		if (i == j) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Units string conversion error).\n");
			return(0);
		}
		strncpy(rasinfo[nfound].h.z_units, &rasinfo[nfound].h.command[i], (size_t)j-i);
		rasinfo[nfound].h.z_units[j-i] = '\0';

		/* Now find the -R string:  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] != '-') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (-R string conversion error).\n");
			return(0);
		}
		i += 2;	/* Skip past the -R */
		j = i+1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' ||  rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (-R string conversion error).\n");
			return(0);
		}
		strncpy(buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i]='\0';
		if (strchr (buf, ':') || strchr (buf, 'W') || strchr (buf, 'E') || strchr (buf, 'S') || strchr (buf, 'N')) {
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;
			GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
		}
		else {
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_FLOAT;
			GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_FLOAT;
		}
			
		GMT->common.R.active = FALSE;	/* Forget that -R was used before */
		if (GMT_parse_common_options (GMT, "R", 'R', buf)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (-R string conversion error).\n");
			return(0);
		}
		rasinfo[nfound].h.wesn[XLO] = GMT->common.R.wesn[XLO];
		rasinfo[nfound].h.wesn[XHI] = GMT->common.R.wesn[XHI];
		rasinfo[nfound].h.wesn[YLO] = GMT->common.R.wesn[YLO];
		rasinfo[nfound].h.wesn[YHI] = GMT->common.R.wesn[YHI];
		rasinfo[nfound].geo = (fabs (rasinfo[nfound].h.wesn[XLO]) > 360.0 || fabs (rasinfo[nfound].h.wesn[XHI]) > 360.0 || fabs (rasinfo[nfound].h.wesn[YLO]) > 90.0 || fabs (rasinfo[nfound].h.wesn[YHI]) > 90.0) ? FALSE : TRUE;
		/* Now find the -I string:  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] != '-') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (-I string conversion error).\n");
			return(0);
		}
		j = i+1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' ||  rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length || i+2 >= j) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (-I string conversion error).\n");
			return(0);
		}
		i += 2;
		strncpy(buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i]='\0';
		if (GMT_getinc (GMT, buf, &rasinfo[nfound].h.inc[GMT_X], &rasinfo[nfound].h.inc[GMT_Y]) ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (-I string conversion error).\n");
			return(0);
		}

		/* Get P or G:  */
		i = j+1;
		while(i < length && !(rasinfo[nfound].h.command[i] == 'P' || rasinfo[nfound].h.command[i] == 'G') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (P or G not found).\n");
			return(0);
		}
		rasinfo[nfound].h.registration = (rasinfo[nfound].h.command[i] == 'P') ? 1 : 0;
		
		/* Check if we have optional G (geographic) or C (Cartesian) that should override the auto test above */
		
		if (rasinfo[nfound].h.command[i+1] == 'G') {	/* Explicit geographic grid */
			rasinfo[nfound].geo = TRUE;
			i++;
		}
		else if (rasinfo[nfound].h.command[i+1] == 'C') {	/* Explicit Cartesian grid */
			rasinfo[nfound].geo = FALSE;
			i++;
		}
			
		stop_point = i + 1;

		/* Get type  */
		j = i + 1;
		while (j < length && (rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Type conversion error).\n");
			return(0);
		}
		switch (rasinfo[nfound].h.command[j]) {
			case 'b':
			case 'c':
			case 'd':
			case 'i':
			case 'l':
			case 'u':
				rasinfo[nfound].type = rasinfo[nfound].h.command[j];
				break;
			default:
				GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Invalid type).\n");
				return(0);
		}

		/* Get scale factor  */
		i = j + 1;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Scale factor conversion error).\n");
			return(0);
		}
		j = i + 1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Scale factor conversion error).\n");
			return(0);
		}
		strncpy(buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i] = '\0';
		if ( (sscanf(buf, "%lf", &rasinfo[nfound].h.z_scale_factor) ) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Scale factor conversion error).\n");
			return(0);
		}

		/* Get offset  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Offset conversion error).\n");
			return(0);
		}
		j = i + 1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Offset conversion error).\n");
			return(0);
		}
		strncpy(buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i] = '\0';
		if ( (sscanf(buf, "%lf", &rasinfo[nfound].h.z_add_offset) ) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Offset conversion error).\n");
			return(0);
		}

		/* Get NaNflag  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (NaN flag conversion error).\n");
			return(0);
		}
		j = i + 1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
		if (j == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (NaN flag conversion error).\n");
			return(0);
		}
		strncpy(buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i] = '\0';
		if (buf[0] == 'n' || buf[0] == 'N') {
			rasinfo[nfound].nanset = 0;
		}
		else {
			rasinfo[nfound].nanset = 1;
			if ( (sscanf(buf, "%ld", &rasinfo[nfound].nanflag) ) != 1) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (NaN flag conversion error).\n");
				return(0);
			}
		}

		
		/* Get filename:  */
		i = j+1;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i == length) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (File name conversion error).\n");
			return(0);
		}
		j = i + 1;
		while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
		strncpy(buf, &rasinfo[nfound].h.command[i], (size_t)j-i);
		buf[j-i] = '\0';
#if _WIN32
		for (i = 0; buf[i]; i++) if (buf[i] == '/') buf[i] = DIR_DELIM;
#else
		for (i = 0; buf[i]; i++) if (buf[i] == '\\') buf[i] = DIR_DELIM;
#endif

		strcpy (rasinfo[nfound].h.remark, buf);

		/* Decode SWAP flag or SKIP command, if present  */

		i = j + 1;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i < length) {	/* Swap or skip flag set*/
			switch (rasinfo[nfound].h.command[i]) {
				case 'L':
				case 'l':	/* Little endian byte order */
					rasinfo[nfound].swap_me = (endian != 'L');	/* Must swap */
					break;
				case 'B':
				case 'b':	/* Big endian byte order */
					rasinfo[nfound].swap_me = (endian != 'B');	/* Must swap */
					break;
#ifdef GMT_COMPAT
				case 'H':
				case 'h':	/* Give header size for skipping */
					rasinfo[nfound].skip = atoi (&rasinfo[nfound].h.command[i+1]);	/* Must skip header */
					break;
#endif
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Byte order conversion error).\n");
					return (0);
			}
		}
		j = i + 1;
		
		/* Decode 2nd SWAP flag or SKIP command, if present  */

		i = j + 1;
		while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
		if (i < length) {	/* Swap or skip flag set*/
			switch (rasinfo[nfound].h.command[i]) {
				case 'L':
				case 'l':	/* Little endian byte order */
					rasinfo[nfound].swap_me = (endian != 'L');	/* Must swap */
					break;
				case 'B':
				case 'b':	/* Big endian byte order */
					rasinfo[nfound].swap_me = (endian != 'B');	/* Must swap */
					break;
#ifdef GMT_COMPAT
				case 'H':
				case 'h':	/* Give header size for skipping */
					rasinfo[nfound].skip = atoi (&rasinfo[nfound].h.command[i+1]);	/* Must skip header */
					break;
#endif
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "Error reading grdraster.info (Byte order conversion error).\n");
					return (0);
			}
		}

		/* Get here when all is OK for this line:  */
		global_lon = 360.0 - (1 - rasinfo[nfound].h.registration) * rasinfo[nfound].h.inc[GMT_X];
		lon_tol = 0.01 * rasinfo[nfound].h.inc[GMT_X];
		global_lon -= lon_tol;	/* make sure we don't fail to find a truly global file  */
		if (rasinfo[nfound].geo && rasinfo[nfound].h.wesn[XHI] - rasinfo[nfound].h.wesn[XLO] >= global_lon) {
			rasinfo[nfound].nglobal = irint (360.0 / rasinfo[nfound].h.inc[GMT_X]);
		}
		else {
			rasinfo[nfound].nglobal = 0;
		}

		rasinfo[nfound].h.command[stop_point] = '\0';

		i = irint ((rasinfo[nfound].h.wesn[XHI] - rasinfo[nfound].h.wesn[XLO])/rasinfo[nfound].h.inc[GMT_X]);
		rasinfo[nfound].h.nx = (int)((rasinfo[nfound].h.registration) ? i : i + 1);
		j = irint ((rasinfo[nfound].h.wesn[YHI] - rasinfo[nfound].h.wesn[YLO])/rasinfo[nfound].h.inc[GMT_Y]);
		rasinfo[nfound].h.ny = (int)((rasinfo[nfound].h.registration) ? j : j + 1);
		
		if ((ksize = get_byte_size (GMT, rasinfo[nfound].type)) == 0)
			expected_size = (GMT_LONG)(ceil (GMT_get_nm (rasinfo[nfound].h.nx, rasinfo[nfound].h.ny) * 0.125) + rasinfo[nfound].skip);
		else
			expected_size = (GMT_LONG)(GMT_get_nm (rasinfo[nfound].h.nx, rasinfo[nfound].h.ny) * ksize + rasinfo[nfound].skip);
		if (STAT (GMT_getdatapath (GMT, rasinfo[nfound].h.remark, path), &F)) {	/* Inquiry about file failed somehow */
			GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Unable to stat file %s [%s] - Skipping it.\n", rasinfo[nfound].h.remark, path);
			continue;
		}
		else
			delta = (GMT_LONG)F.st_size - expected_size;
		if (delta == GRD_HEADER_SIZE)
			rasinfo[nfound].skip = GRD_HEADER_SIZE;	/* Must skip GMT grd header */
		else if (delta) {
			GMT_report (GMT, GMT_MSG_FATAL, "Metadata conflict: Actual size of file %s [%ld] differs from expected [%ld]. Verify file and its grdraster.info details.\n", rasinfo[nfound].h.remark, (GMT_LONG)F.st_size, expected_size);
			return (0);
		}
		nfound++;
		
		if (nfound == n_alloc) {
			n_alloc <<= 1;
			rasinfo = GMT_memory (GMT, rasinfo, n_alloc, struct GRDRASTER_INFO);
		}
	}
	if ((error = GMT_End_IO (GMT->parent, GMT_IN, 0))) return (error);	/* Disables further data input */

	if (nfound > 0) rasinfo = GMT_memory (GMT, rasinfo, nfound, struct GRDRASTER_INFO);
	
	*ras = rasinfo;
	return (nfound);
}

void *New_grdraster_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDRASTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDRASTER_CTRL);
	
	return ((void *)C);
}

void Free_grdraster_Ctrl (struct GMT_CTRL *GMT, struct GRDRASTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	if (C->T.file) free ((void *)C->T.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdraster_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;
	struct GRDRASTER_INFO *rasinfo = NULL;
	GMT_LONG i, nrasters;

	if (!(nrasters = load_rasinfo (GMT, &rasinfo, MY_ENDIAN))) {
		GMT_message (GMT, "ERROR reading grdraster.info file.\n");
		return (EXIT_FAILURE);
	}
	GMT_message (GMT, "grdraster %s [API] - Extract a region from a raster and save in a grid file\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdraster <file number>|<text> %s [-G<grdfilename>] [%s]\n", GMT_Rgeo_OPT, GMT_Id_OPT);
	GMT_message (GMT, "\t[-T<tblfilename>] [%s] [%s]\n", GMT_bo_OPT, GMT_o_OPT);

	GMT_message (GMT, "\t<file number> (#) or <text> corresponds to one of the datasets listed.\n");
	GMT_message (GMT, "\t[<text> can be a unique substring of the description].\n\n");
	GMT_message (GMT, "#	Data Description	Unit	Coverage		Spacing	Registration\n");
	GMT_message (GMT, "------------------------------------------------------------------------------------\n");
	for (i = 0; i < nrasters; i++) GMT_message (GMT, "%s\n", rasinfo[i].h.command);
	GMT_message (GMT, "------------------------------------------------------------------------------------\n\n");
#if WORDS_BIGENDIAN == 0
	GMT_message (GMT, "grdraster default binary byte order is Little-endian\n");
#else
	GMT_message (GMT, "grdraster default binary byte order is Big-endian\n");
#endif
	GMT_free (GMT, rasinfo);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-R specifies the west, east, south, and north edges of the area.\n");
	GMT_message (GMT, "\t   Use dd:mm format for regions given in degrees and minutes.\n");
	GMT_message (GMT, "\t   Append r if -R specifies the longitudes/latitudes of the lower left\n");
	GMT_message (GMT, "\t   and upper right corners of a rectangular area.  If r is appended\n");
	GMT_message (GMT, "\t   you must also specify a projection with -J (set scale = 1).\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-G sets the filename for output grid.\n");
	GMT_message (GMT, "\t-I specifies the sampling interval of the grid [Default is raster spacing].\n");
	GMT_message (GMT, "\t   Give -Idx or -Idx/dy if dy not equal dx.  Append m for minutes.\n");
	GMT_message (GMT, "\t   (-I does not do any filtering; it just sub-samples the raster.)\n");
	GMT_explain_options (GMT, "j");
	GMT_message (GMT, "\t-T sets the filename for output ASCII table with xyz triplets.\n");
	GMT_message (GMT, "\t   To get binary triplets, see -bo.  Cannot be used with -G.\n");
	GMT_explain_options (GMT, "VD3o.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdraster_parse (struct GMTAPI_CTRL *C, struct GRDRASTER_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdraster and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */
			case 'G':
				Ctrl->G.active = TRUE;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, &Ctrl->I.xinc, &Ctrl->I.yinc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	/* Check that arguments were valid:  */
	GMT_check_lattice (GMT, &Ctrl->I.xinc, &Ctrl->I.yinc, NULL, &Ctrl->I.active);
	
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "GMT SYNTAX ERROR:  Must specify -R option.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.xinc <= 0.0 || Ctrl->I.yinc <= 0.0), "GMT SYNTAX ERROR -I option.  Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, n_files != 1, "GMT SYNTAX ERROR -I option.  You must specify only one raster file ID.\n");
#ifndef GMT_COMPAT	/* In old version we default to triplet output if -G was not set */
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && Ctrl->T.active, "GMT SYNTAX ERROR:  You must select only one of -G or -T.\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->T.active), "GMT SYNTAX ERROR:  You must select either -G or -T.\n");
#endif

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdraster_Ctrl (GMT, Ctrl); GMT_free (GMT, rasinfo); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdraster (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, j, k, ksize = 0, iselect, imult, jmult, nrasters, ij_offset;
	GMT_LONG irasstart, jrasstart, n_nan, iras, jras, ij, ijras, jseek;
	GMT_LONG error = FALSE, firstread, nm, nmask = 0;
	
	char *buffer = NULL, *tselect = NULL, match[GRD_REMARK_LEN];
	unsigned char *ubuffer = NULL;
	static unsigned char maskset[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	
	float *floatrasrow = NULL;
	
	double tol, grdlatorigin, grdlonorigin, raslatorigin, raslonorigin, *x = NULL, y, out[3];

	FILE *fp = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GRDRASTER_INFO myras;
	struct GRDRASTER_INFO *rasinfo = NULL;
	struct GRDRASTER_CTRL *Ctrl = NULL;
	struct GMT_OPTION *r_opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdraster_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdraster_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdraster", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRb", "", options))) Return (error);
	Ctrl = (struct GRDRASTER_CTRL *) New_grdraster_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdraster_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdraster main code ----------------------------*/

	GMT_memset (&myras, 1, struct GRDRASTER_INFO);
	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	if (!(nrasters = load_rasinfo (GMT, &rasinfo, MY_ENDIAN)) || !rasinfo) {
		GMT_message (GMT, "ERROR reading grdraster.info file.\n");
		Return (EXIT_FAILURE);
	}
	GMT_report (GMT, GMT_MSG_VERBOSE, "Found %ld data sets in grdraster.info\n", nrasters);
	
	/* Since load_rasinfo processed -R options we need to re-parse the main -R */
	
	GMT_Find_Option (GMT->parent, 'R', options, &r_opt);
	GMT->common.R.active = FALSE;	/* Forget that -R was used before */
	if (GMT_parse_common_options (GMT, "R", 'R', r_opt->arg)) {
		GMT_message (GMT, "Error reprocessing -R?.\n");
		Return (EXIT_FAILURE);
	}
	
	/* Check if given argument is an integer ID.  If so, assign iselect, else set it to -1 */
	
	tselect = strdup (Ctrl->In.file);
	GMT_str_toupper (tselect);	/* Make it upper case - which wont affect integers */
	for (j = i = 0; tselect[j] && i == 0; j++) if (!isdigit ((int)tselect[j])) i = 1;
	iselect = (i == 0) ? atoi (tselect) : -1;
	for (i = 0, j = -1; !error && i < nrasters; i++) {
		if (iselect != -1) {	/* We gave an integer ID */
			if (rasinfo[i].id == iselect) {
				if (j == -1)
					j = i;
				else {
					GMT_message (GMT, "ERROR:  At least two rasters have the same file number in grdraster.info\n");
					error++;
				}
			}
		}
		else {	/* We gave a text snippet to match in command */
			strcpy (match, rasinfo[i].h.command);
			GMT_str_toupper (match);	/* Make it upper case  */
			if (strstr (match, tselect)) {	/* Found a matching text string */
				if (j == -1)
					j = i;
				else {
					GMT_message (GMT, "ERROR:  At least two rasters have the same text [%s] in grdraster.info\n", tselect);
					error++;
				}
			}
		}
	}
	if (j == -1) {
		if (iselect != -1)
			GMT_message (GMT, "ERROR:  No raster with file number %ld in grdraster.info\n", iselect);
		else
			GMT_message (GMT, "ERROR:  No raster with text %s in grdraster.info\n", tselect);
		error++;
	}
	else
		myras = rasinfo[j];

#ifdef GMT_COMPAT	/* In old version we default to triplet output if -G was not set */
	if (!Ctrl->G.active) Ctrl->T.active = TRUE;
#endif

	if (error) {
		GMT_free_grid (GMT, &Grid, FALSE);
		Return (EXIT_FAILURE);
	}

	/* OK, here we have a recognized dataset ID */
	
	Grid->header->wesn[XLO] = GMT->common.R.wesn[XLO];
	Grid->header->wesn[XHI] = GMT->common.R.wesn[XHI];
	Grid->header->wesn[YLO] = GMT->common.R.wesn[YLO];
	Grid->header->wesn[YHI] = GMT->common.R.wesn[YHI];		

	GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = (myras.geo) ? GMT_IS_LON : GMT_IS_FLOAT;
	GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = (myras.geo) ? GMT_IS_LAT : GMT_IS_FLOAT;

	/* Everything looks OK so far.  If (Ctrl->I.active) verify that it will work, else set it.  */
	if (Ctrl->I.active) {
		Grid->header->inc[GMT_X] = Ctrl->I.xinc;
		Grid->header->inc[GMT_Y] = Ctrl->I.yinc;
		tol = 0.01 * myras.h.inc[GMT_X];
		imult = irint(Grid->header->inc[GMT_X] / myras.h.inc[GMT_X]);
		if (imult < 1 || fabs(Grid->header->inc[GMT_X] - imult * myras.h.inc[GMT_X]) > tol) error++;
		tol = 0.01 * myras.h.inc[GMT_Y];
		jmult = irint(Grid->header->inc[GMT_Y] / myras.h.inc[GMT_Y]);
		if (jmult < 1 || fabs(Grid->header->inc[GMT_Y] - jmult * myras.h.inc[GMT_Y]) > tol) error++;
		if (error) {
			GMT_message (GMT, "ERROR:  Your -I option does not create a grid which fits the selected raster (%s)\n", myras.h.command);
			Return (EXIT_FAILURE);
		}
	}
	else {
		Grid->header->inc[GMT_X] = myras.h.inc[GMT_X];
		Grid->header->inc[GMT_Y] = myras.h.inc[GMT_Y];
		imult = jmult = 1;
	}

	if (GMT->common.R.oblique && GMT->current.proj.projection != GMT_NO_PROJ) {
		GMT_err_fail (GMT, GMT_map_setup (GMT, Grid->header->wesn), "");
		
		Grid->header->wesn[XLO] = floor (GMT->common.R.wesn[XLO] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[XHI] = ceil  (GMT->common.R.wesn[XHI] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YLO] = floor (GMT->common.R.wesn[YLO] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		Grid->header->wesn[YHI] = ceil  (GMT->common.R.wesn[YHI] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		
		if (GMT->current.setting.verbose && rint (Grid->header->inc[GMT_X] * 60.0) == (Grid->header->inc[GMT_X] * 60.0)) {	/* Spacing in even minutes */
			GMT_LONG w, e, s, n, wm, em, sm, nm;
			
			w = (GMT_LONG) floor (Grid->header->wesn[XLO]);	wm = (GMT_LONG) irint ((Grid->header->wesn[XLO] - w) * 60.0);
			e = (GMT_LONG) floor (Grid->header->wesn[XHI]);	em = (GMT_LONG) irint ((Grid->header->wesn[XHI] - e) * 60.0);
			s = (GMT_LONG) floor (Grid->header->wesn[YLO]);	sm = (GMT_LONG) irint ((Grid->header->wesn[YLO] - s) * 60.0);
			n = (GMT_LONG) floor (Grid->header->wesn[YHI]);	nm = (GMT_LONG) irint ((Grid->header->wesn[YHI] - n) * 60.0);
			GMT_message (GMT, "%s -> -R%ld:%2.2ld/%ld:%2.2ld/%ld:%2.2ld/%ld:%2.2ld\n", r_opt->arg, w, wm, e, em, s, sm, n, nm);
		}
		else if (GMT->current.setting.verbose)
			GMT_message (GMT, "%s -> -R%g/%g/%g/%g\n", r_opt->arg, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI]);
	}
	
	/* Now Enforce that wesn will fit inc[GMT_X], inc[GMT_Y].  Set nx, ny but reset later based on G or P  */
	tol = 0.01 * Grid->header->inc[GMT_X];
	Grid->header->nx = irint((Grid->header->wesn[XHI] - Grid->header->wesn[XLO])/Grid->header->inc[GMT_X]);
	if (fabs ((Grid->header->wesn[XHI] - Grid->header->wesn[XLO]) - Grid->header->inc[GMT_X] * Grid->header->nx) > tol) error++;
	tol = 0.01 * Grid->header->inc[GMT_Y];
	Grid->header->ny = irint((Grid->header->wesn[YHI] - Grid->header->wesn[YLO])/Grid->header->inc[GMT_Y]);
	if (fabs ((Grid->header->wesn[YHI] - Grid->header->wesn[YLO]) - Grid->header->inc[GMT_Y] * Grid->header->ny) > tol) error++;
	if (error) {	/* Must cleanup and give warning */
		Grid->header->wesn[XLO] = floor (Grid->header->wesn[XLO] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[XHI] =  ceil (Grid->header->wesn[XHI] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YLO] = floor (Grid->header->wesn[YLO] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		Grid->header->wesn[YHI] =  ceil (Grid->header->wesn[YHI] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		Grid->header->nx = irint ((Grid->header->wesn[XHI] - Grid->header->wesn[XLO]) / Grid->header->inc[GMT_X]);
		Grid->header->ny = irint ((Grid->header->wesn[YHI] - Grid->header->wesn[YLO]) / Grid->header->inc[GMT_Y]);
		GMT_message (GMT, "WARNING:  Your -R option does not create a region divisible by inc[GMT_X], inc[GMT_Y].\n");
		if (GMT_IS_ZERO (rint (Grid->header->inc[GMT_X] * 60.0) - Grid->header->inc[GMT_X] * 60.0)) {	/* Spacing in even minutes */
			GMT_LONG w, e, s, n, wm, em, sm, nm;
			w = (GMT_LONG) floor (Grid->header->wesn[XLO]);	wm = (GMT_LONG) irint ((Grid->header->wesn[XLO] - w) * 60.0);
			e = (GMT_LONG) floor (Grid->header->wesn[XHI]);	em = (GMT_LONG) irint ((Grid->header->wesn[XHI] - e) * 60.0);
			s = (GMT_LONG) floor (Grid->header->wesn[YLO]);	sm = (GMT_LONG) irint ((Grid->header->wesn[YLO] - s) * 60.0);
			n = (GMT_LONG) floor (Grid->header->wesn[YHI]);	nm = (GMT_LONG) irint ((Grid->header->wesn[YHI] - n) * 60.0);
			if (!GMT->common.R.oblique)
				GMT_message (GMT, "WARNING:  Region reset to -R%ld:%2.2ld/%ld:%2.2ld/%ld:%2.2ld/%ld:%2.2ld.\n", w, wm, e, em, s, sm, n, nm);
			else
				GMT_message (GMT, "WARNING:  Region reset to -R%ld:%2.2ld/%ld:%2.2ld/%ld:%2.2ld/%ld:%2.2ldr\n", w, wm, s, sm, e, em, n, nm);
		}
		else {
			if (!GMT->common.R.oblique)
				GMT_message (GMT, "WARNING:  Region reset to -R%g/%g/%g/%g.\n", Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI]);
			else
				GMT_message (GMT, "WARNING:  Region reset to -R%g/%g/%g/%gr.\n", Grid->header->wesn[XLO], Grid->header->wesn[YLO], Grid->header->wesn[XHI], Grid->header->wesn[YHI]);
		}
		error = 0;
	}

	/* Now we are ready to go:  */
	if (!myras.h.registration) {
		Grid->header->nx++;
		Grid->header->ny++;
	}
	strcpy (Grid->header->title, myras.h.title);
	strcpy (Grid->header->z_units, myras.h.z_units);
	strcpy (Grid->header->remark, myras.h.remark);
	if (myras.geo) {
		strcpy (Grid->header->x_units, "Longitude [degrees_east]");
		strcpy (Grid->header->y_units, "Latitude [degrees_north]");
	}
	else {
		strcpy (Grid->header->x_units, "x");
		strcpy (Grid->header->y_units, "y");
	}
	Grid->header->registration = myras.h.registration;
	Grid->header->z_min = DBL_MAX;
	Grid->header->z_max = -DBL_MAX;
	Grid->header->xy_off = 0.5 * Grid->header->registration;
	myras.h.xy_off = 0.5 * myras.h.registration;

	grdlatorigin = GMT_row_to_y (0, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], Grid->header->xy_off, Grid->header->ny);
	grdlonorigin = GMT_col_to_x (0, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], Grid->header->xy_off, Grid->header->nx);
	raslatorigin = GMT_row_to_y (0, myras.h.wesn[YLO], myras.h.wesn[YHI], myras.h.inc[GMT_Y], myras.h.xy_off, myras.h.ny);
	raslonorigin = GMT_col_to_x (0, myras.h.wesn[XLO], myras.h.wesn[XHI], myras.h.inc[GMT_X], myras.h.xy_off, myras.h.nx);
	irasstart = irint ((grdlonorigin - raslonorigin) / myras.h.inc[GMT_X]);
	jrasstart = irint ((raslatorigin - grdlatorigin) / myras.h.inc[GMT_Y]);
	if (myras.nglobal) while (irasstart < 0) irasstart += myras.nglobal;
	n_nan = 0;

	/* Get space:  */
	if (Ctrl->T.active) {	/* Need just space for one row */
		Grid->data = GMT_memory (GMT, NULL, Grid->header->nx, float);
		x = GMT_memory (GMT, NULL, Grid->header->nx, double);
		for (i = 0; i < Grid->header->nx; i++) x[i] = GMT_col_to_x (i, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], Grid->header->xy_off, Grid->header->nx);
		ij_offset = 0;
		if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_REC))) Return (error);			/* Enables data output and sets access mode */
		if ((error = GMT_set_cols (GMT, GMT_OUT, 3))) Return (error);
	} else {	/* Need entire grid */
		nm = GMT_get_nm (Grid->header->nx, Grid->header->ny);
		Grid->data = GMT_memory (GMT, NULL, nm, float);
		ij_offset = Grid->header->nx;
		if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);			/* Enables data output and sets access mode */
	}
	
	ksize = get_byte_size (GMT, myras.type);
	if (ksize == 0) {	/* Bits; Need to read the whole thing:  */
		nmask = (GMT_LONG)ceil (myras.h.nx * myras.h.ny * 0.125);
		ubuffer = GMT_memory (GMT, NULL, nmask, unsigned char);
	}
	else {	/* Need to read by rows, and convert each row to float:  */
		buffer = GMT_memory (GMT, NULL, ksize * myras.h.nx, char);
		floatrasrow = GMT_memory (GMT, NULL, myras.h.nx, float);
	}

	/* Now open file and do it:  */
	
	if ( (fp = GMT_fopen (GMT, myras.h.remark, "rb") ) == NULL) {
		GMT_message (GMT, "ERROR opening %s for read.\n", myras.h.remark);
		Return (EXIT_FAILURE);
	}
	if (myras.skip && GMT_fseek (fp, (long) (myras.skip), SEEK_CUR) ) {
		GMT_message (GMT, "ERROR skipping %ld bytes in %s.\n", myras.skip, myras.h.remark);
		Return (EXIT_FAILURE);
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Reading from raster %s\n", myras.h.remark);
	if (myras.swap_me) GMT_report (GMT, GMT_MSG_NORMAL, "Data from %s will be byte-swapped\n", myras.h.remark);

	if (myras.type == 'b') {
		if ( (GMT_fread((void *)ubuffer, sizeof (unsigned char), (size_t)nmask, fp)) != (size_t)nmask) {
			GMT_message (GMT, "ERROR:  Failure to read a bitmap raster from %s.\n", myras.h.remark);
			GMT_free (GMT, ubuffer);
			GMT_fclose (GMT, fp);
			Return (EXIT_FAILURE);
		}
		for (j = 0, jras = jrasstart; j < Grid->header->ny; j++, jras += jmult) {
			y = GMT_row_to_y (j, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], Grid->header->xy_off, Grid->header->ny);
			if (jras < 0 || jras > myras.h.ny) {
				/* This entire row is outside the raster:  */
				for (i = 0, ij = j * ij_offset; i < Grid->header->nx; i++, ij++) Grid->data[ij] = GMT->session.f_NaN;
				n_nan += Grid->header->nx;
			}
			else {
				iras = irasstart;
				ijras = jras * myras.h.nx;
				for (i = 0, ij = j * ij_offset; i < Grid->header->nx; i++, ij++) {
					if (myras.nglobal && iras >= myras.nglobal) iras = iras%myras.nglobal;
					if (iras < 0 || iras >= myras.h.nx) {
						Grid->data[ij] = GMT->session.f_NaN;
						n_nan++;
					}
					else {
						k = ijras + iras;
						Grid->data[ij] = (float)((ubuffer[k/8] & maskset[k%8]) ? 1.0 : 0.0);
						if (Grid->data[ij] > Grid->header->z_max) Grid->header->z_max = Grid->data[ij];
						if (Grid->data[ij] < Grid->header->z_min) Grid->header->z_min = Grid->data[ij];
					}
					iras += imult;
				}
			}
			if (Ctrl->T.active) {
				out[1] = y;
				for (i = 0; i < Grid->header->nx; i++) {
					out[0] = x[i];
					out[2] = Grid->data[i];
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
				}
			}
		}
		GMT_free (GMT, ubuffer);
	}
	else {
		firstread = TRUE;
		for (j = 0, jras = jrasstart; j < Grid->header->ny; j++, jras += jmult) {
			y = GMT_row_to_y (j, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], Grid->header->xy_off, Grid->header->ny);
			if (jras < 0 || jras > myras.h.ny) {
				/* This entire row is outside the raster:  */
				for (i = 0, ij = j * ij_offset; i < Grid->header->nx; i++, ij++) Grid->data[ij] = GMT->session.f_NaN;
				n_nan += Grid->header->nx;
			}
			else {
				if (firstread) {
					jseek = (jras != 0) ? jras : 0;
					firstread = FALSE;
				}
				else if (jmult > 1)
					jseek = jmult - 1;
				else
					jseek = 0;
				/* This will be slow on SGI because seek is broken there */
				if (jseek && GMT_fseek (fp, (long) (jseek * ksize * myras.h.nx), SEEK_CUR) ) {
					GMT_message (GMT, "ERROR seeking in %s\n", myras.h.remark);
					GMT_fclose (GMT, fp);
					GMT_free (GMT, buffer);
					Return (EXIT_FAILURE);
				}
				if ( (GMT_fread((void *)buffer, (size_t)ksize, (size_t)myras.h.nx, fp)) != (size_t)myras.h.nx) {
					GMT_message (GMT, "ERROR reading in %s\n", myras.h.remark);
					GMT_fclose (GMT, fp);
					GMT_free (GMT, buffer);
					Return (EXIT_FAILURE);
				}
#ifdef DEBUG
				GMT_report (GMT, GMT_MSG_NORMAL, "Doing line %6.6ld\r", j);
#endif
				switch (myras.type) {
					case 'u':
						convert_u_row (GMT, myras, floatrasrow, (unsigned char *)buffer);
						break;
					case 'c':
						convert_c_row (GMT, myras, floatrasrow, buffer);
						break;
					case 'd':
						convert_d_row (GMT, myras, floatrasrow, (unsigned short int *)buffer);
						break;
					case 'i':
						convert_i_row (GMT, myras, floatrasrow, (short int *)buffer);
						break;
					case 'l':
						convert_l_row (GMT, myras, floatrasrow, (int *)buffer);
						break;
				}
				iras = irasstart;
				for (i = 0, ij = j * ij_offset; i < Grid->header->nx; i++, ij++) {
					if (myras.nglobal && iras >= myras.nglobal) iras = iras%myras.nglobal;
					if (iras < 0 || iras >= myras.h.nx) {
						Grid->data[ij] = GMT->session.f_NaN;
						n_nan++;
					}
					else {
						Grid->data[ij] = floatrasrow[iras];
						if (Grid->data[ij] > Grid->header->z_max) Grid->header->z_max = Grid->data[ij];
						if (Grid->data[ij] < Grid->header->z_min) Grid->header->z_min = Grid->data[ij];
					}
					iras += imult;
				}
			}
			if (Ctrl->T.active) {
				out[1] = y;
				for (i = 0; i < Grid->header->nx; i++) {
					out[0] = x[i];
					out[2] = Grid->data[i];
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
				}
			}
		}
		GMT_free (GMT, buffer);
		GMT_free (GMT, floatrasrow);
	}
	GMT_fclose (GMT, fp);
	GMT_free (GMT, rasinfo);
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Finished reading from %s\n", myras.h.remark);
	GMT_report (GMT, GMT_MSG_NORMAL, "min max and # NaN found: %g %g %ld\n", Grid->header->z_min, Grid->header->z_max, n_nan);

	if (n_nan == Grid->header->nx * Grid->header->ny) GMT_report (GMT, GMT_MSG_NORMAL, "WARNING - Your grid file is entirely full of NaNs.\n");

	if (Ctrl->T.active) {
		GMT_free (GMT, x);
	}
	else {
		if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);
	}
	
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */
	
	Return (GMT_OK);
}
