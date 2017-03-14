/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#define THIS_MODULE_NAME	"grdraster"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Extract subregion from a binary raster and save as a GMT grid"
#define THIS_MODULE_KEYS	"GG},TD)"
#define THIS_MODULE_NEEDS	"R"

#include "gmt_dev.h"
#include "common_byteswap.h"

#define GMT_PROG_OPTIONS "-JRVbdh"

EXTERN_MSC void gmt_str_toupper (char *string);

struct GRDRASTER_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct T {	/* -T<output_table> */
		bool active;
		char *file;
	} T;
};

struct GRDRASTER_INFO {
	struct GMT_GRID_HEADER h;
	unsigned int id;	/* File number  */
	int nglobal;	/* If not 0, ras is global and i%nglobal makes it periodic  */
	int nanflag;
	off_t skip;		/* Skip this number of header bytes when opening file  */
	bool nanset;		/* True if raster uses nanflag to signal NaN  */
	bool swap_me;	/* true if data set need to be swapped */
	bool geo;		/* true if we believe x/y is lon/lat, false otherwise */
	char type;
};

GMT_LOCAL inline void convert_u_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO *ras, float *row, char *buffer) {
	/* convert unsigned char */
	unsigned int i;
	unsigned char tempval;
	for (i = 0; i < ras->h.n_columns; ++i) {
		gmt_M_memcpy (&tempval, &buffer[i], 1, char);
		if (ras->nanset && tempval == ras->nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras->h.z_scale_factor != 1.0) row[i] *= (float)ras->h.z_scale_factor;
			if (ras->h.z_add_offset != 0.0) row[i] += (float)ras->h.z_add_offset;
		}
	}
	return;
}

GMT_LOCAL inline void convert_c_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO *ras, float *row, char *buffer) {
	/* convert char */
	unsigned int i;
	char tempval;
	for (i = 0; i < ras->h.n_columns; ++i) {
		tempval = buffer[i];
		if (ras->nanset && tempval == ras->nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras->h.z_scale_factor != 1.0) row[i] *= (float)ras->h.z_scale_factor;
			if (ras->h.z_add_offset != 0.0) row[i] += (float)ras->h.z_add_offset;
		}
	}
	return;
}

GMT_LOCAL inline void convert_d_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO *ras, float *row, char *buffer) {
	/* convert uint16_t */
	unsigned int i;
	uint16_t tempval;
	for (i = 0; i < ras->h.n_columns; ++i) {
		gmt_M_memcpy (&tempval, &buffer[i * sizeof(uint16_t)], 1, uint16_t);
		if (ras->swap_me)
			tempval = bswap16 (tempval);
		if (ras->nanset && tempval == ras->nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras->h.z_scale_factor != 1.0) row[i] *= (float)ras->h.z_scale_factor;
			if (ras->h.z_add_offset != 0.0) row[i] += (float)ras->h.z_add_offset;
		}
	}
	return;
}

GMT_LOCAL inline void convert_i_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO *ras, float *row, char *buffer) {
	/* convert int16_t */
	unsigned int i;
	int16_t tempval;
	uint16_t *u = (uint16_t *)&tempval;
	for (i = 0; i < ras->h.n_columns; i++) {
		gmt_M_memcpy (&tempval, &buffer[i * sizeof(int16_t)], 1, int16_t);
		if (ras->swap_me)
			*u = bswap16 (*u);
		if (ras->nanset && tempval == ras->nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras->h.z_scale_factor != 1.0) row[i] *= (float)ras->h.z_scale_factor;
			if (ras->h.z_add_offset != 0.0) row[i] += (float)ras->h.z_add_offset;
		}
	}
	return;
}

GMT_LOCAL inline void convert_l_row (struct GMT_CTRL *GMT, struct GRDRASTER_INFO *ras, float *row, char *buffer) {
	/* convert int32_t */
	unsigned int i;
	int32_t tempval;
	uint32_t *u = (uint32_t *)&tempval;
	for (i = 0; i < ras->h.n_columns; i++) {
		gmt_M_memcpy (&tempval, &buffer[i * sizeof(int32_t)], 1, int32_t);
		if (ras->swap_me)
			*u = bswap32 (*u);
		if (ras->nanset && tempval == ras->nanflag) {
			row[i] = GMT->session.f_NaN;
		}
		else {
			row[i] = (float)tempval;
			if (ras->h.z_scale_factor != 1.0) row[i] *= (float)ras->h.z_scale_factor;
			if (ras->h.z_add_offset != 0.0) row[i] += (float)ras->h.z_add_offset;
		}
	}
	return;
}

GMT_LOCAL void reset_coltype (struct GMT_CTRL *GMT, char *Rarg) {
	/* Because grdraster can deal with either geographic or Cartesian data
	 * we need to be able to recognized geo arguments and set col_type correctly. */
		size_t last;
	if (!Rarg || (last = strlen (Rarg) == 0)) return;	/* Unable to do anything */
	last--;	/* Index of last character in Rarg */
	if (strchr (Rarg, ':') || strchr ("WESN", Rarg[last]))
		gmt_set_geographic (GMT, GMT_IN);
	else	/* Treat as Cartesian */
		gmt_set_cartesian (GMT, GMT_IN);
}

GMT_LOCAL unsigned int get_byte_size (struct GMT_CTRL *GMT, char type) {
	/* Return byte size of each item, or 0 if bits */
	int ksize;
	switch (type) {
		case 'b':
			ksize = 0;	break;
		case 'c':
		case 'u':
			ksize = 1;	break;
		case 'd':
		case 'h':
		case 'i':
			ksize = 2;	break;
		case 'l':
			ksize = 4;	break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Invalid data type [%c]\n", (int)type);
			return (GMT_NOT_A_VALID_TYPE);
			break;
	}
	return (ksize);
}

GMT_LOCAL int load_rasinfo (struct GMT_CTRL *GMT, struct GRDRASTER_INFO **ras, char endian) {
	/* Read the file grdraster.info
		Store the i'th row of the file in rasinfo[i].h.command.
		Store the filename in rasinfo[i].h.remark.
		Store the description in rasinfo[i].h.title.
		Store the units in rasinfo[i].h.z_units.
		After all has parsed correctly, truncate rasinfo[i].h.command
			so it can be printed out as an abbreviated description
			for the user.
		Figure out if file is global, and set nglobal.
		Set n_columns and n_rows also.

	Return 0 if cannot read files correctly, or nrasters if successful.  */

	int nfound = 0, ksize = 0, n_alloc;
	uint64_t expected_size, tbl, seg, row;
	size_t length, i, j, stop_point;
	off_t delta;
	double global_lon, lon_tol;
	char path[GMT_BUFSIZ] = {""}, buf[GMT_GRID_REMARK_LEN160] = {""}, *record = NULL, *file = "grdraster.info", *ptr = NULL;
	struct GRDRASTER_INFO *rasinfo = NULL;
	struct GMT_TEXTSET *In = NULL;
	struct stat F;

	/* Find and open the file grdraster.info */

	if (!gmt_access (GMT, file, R_OK))	/* Found in current directory */
		ptr = file;
	else if (gmt_getdatapath (GMT, file, path, F_OK) || gmt_getsharepath (GMT, "dbase", file, "", path, F_OK))
		ptr = path;	/* Found it in the data or share path/dbase directory */
	else {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Error: Unable to find file %s.\n", file);
		return (0);
	}
	if ((In = GMT_Read_Data (GMT->parent, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, ptr, NULL)) == NULL) {
		return (0);
	}

	/* Truncate the pathname of grdraster.info to just the directory name */

	n_alloc = GMT_SMALL_CHUNK;
	rasinfo = gmt_M_memory (GMT, NULL, n_alloc, struct GRDRASTER_INFO);

	for (tbl = 0; tbl < In->n_tables; tbl++) {	/* We only expect one table but who knows what the user does */
		for (seg = 0; seg < In->table[tbl]->n_segments; seg++) {	/* We only expect one segment in each table but again... */
			for (row = 0; row < In->table[tbl]->segment[seg]->n_rows; row++) {	/* Finally processing the rows */
				record = In->table[tbl]->segment[seg]->data[row];
				if (record[0] == '#') continue;	/* Skip all headers */

				/* Strip off trailing "\r\n" */
				gmt_chop (record);
				length = strlen(record);
				if (length == 0) continue;	/* Skip blank lines */

				strncpy (rasinfo[nfound].h.command, record, GMT_GRID_COMMAND_LEN320-1);

				/* Find the integer file name first */
				i = 0;
				while (i < length && (rasinfo[nfound].h.command[i] == ' ' ||  rasinfo[nfound].h.command[i] == '\t') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (File number conversion error).\n");
					continue;
				}
				j = i+1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' ||  rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (File number conversion error).\n");
					continue;
				}

				strncpy (buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i] = '\0';
				if ( (sscanf(buf, "%d", &rasinfo[nfound].id) ) != 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (File number conversion error).\n");
					continue;
				}

				/* Now find the title string */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] != '"') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Title string conversion error).\n");
					continue;
				}
				j = i+1;
				while (j < length && (rasinfo[nfound].h.command[j] != '"') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Title string conversion error).\n");
					continue;
				}
				i++;
				if (i == j) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Title string conversion error).\n");
					continue;
				}
				strncpy(rasinfo[nfound].h.title, &rasinfo[nfound].h.command[i], j-i);
				rasinfo[nfound].h.title[j-i] = '\0';

				/* Now find the z_unit string */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] != '"') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Units string conversion error).\n");
					continue;
				}
				j = i+1;
				while (j < length && (rasinfo[nfound].h.command[j] != '"') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Units string conversion error).\n");
					continue;
				}
				i++;
				if (i == j) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Units string conversion error).\n");
					continue;
				}
				strncpy(rasinfo[nfound].h.z_units, &rasinfo[nfound].h.command[i], j-i);
				rasinfo[nfound].h.z_units[j-i] = '\0';

				/* Now find the -R string */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] != '-') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (-R string conversion error).\n");
					continue;
				}
				i += 2;	/* Skip past the -R */
				j = i+1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' ||  rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (-R string conversion error).\n");
					continue;
				}
				strncpy(buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i]='\0';
				reset_coltype (GMT, buf);	/* Make sure geo coordinates will be recognized */
				GMT->common.R.active = false;	/* Forget that -R was used before */
				if (gmt_parse_common_options (GMT, "R", 'R', buf)) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (-R string conversion error).\n");
					continue;
				}
				rasinfo[nfound].h.wesn[XLO] = GMT->common.R.wesn[XLO];
				rasinfo[nfound].h.wesn[XHI] = GMT->common.R.wesn[XHI];
				rasinfo[nfound].h.wesn[YLO] = GMT->common.R.wesn[YLO];
				rasinfo[nfound].h.wesn[YHI] = GMT->common.R.wesn[YHI];
				rasinfo[nfound].geo = (fabs (rasinfo[nfound].h.wesn[XLO]) > 360.0 || fabs (rasinfo[nfound].h.wesn[XHI]) > 360.0 || fabs (rasinfo[nfound].h.wesn[YLO]) > 90.0 || fabs (rasinfo[nfound].h.wesn[YHI]) > 90.0) ? false : true;
				/* Now find the -I string */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] != '-') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (-I string conversion error).\n");
					continue;
				}
				j = i+1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' ||  rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length || i+2 >= j) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (-I string conversion error).\n");
					continue;
				}
				i += 2;
				strncpy(buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i]='\0';
				if (gmt_getinc (GMT, buf, rasinfo[nfound].h.inc) ) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (-I string conversion error).\n");
					continue;
				}

				/* Get P or G */
				i = j+1;
				while(i < length && !(rasinfo[nfound].h.command[i] == 'P' || rasinfo[nfound].h.command[i] == 'G') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (P or G not found).\n");
					continue;
				}
				rasinfo[nfound].h.registration = (rasinfo[nfound].h.command[i] == 'P') ? 1 : 0;

				/* Check if we have optional G (geographic) or C (Cartesian) that should override the auto test above */

				if (rasinfo[nfound].h.command[i+1] == 'G') {	/* Explicit geographic grid */
					rasinfo[nfound].geo = true;
					i++;
				}
				else if (rasinfo[nfound].h.command[i+1] == 'C') {	/* Explicit Cartesian grid */
					rasinfo[nfound].geo = false;
					i++;
				}

				stop_point = i + 1;

				/* Get type  */
				j = i + 1;
				while (j < length && (rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Type conversion error).\n");
					continue;
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
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Invalid type).\n");
						continue;
				}

				/* Get scale factor  */
				i = j + 1;
				while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Scale factor conversion error).\n");
					continue;
				}
				j = i + 1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Scale factor conversion error).\n");
					continue;
				}
				strncpy(buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i] = '\0';
				if ( (sscanf(buf, "%lf", &rasinfo[nfound].h.z_scale_factor) ) != 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Scale factor conversion error).\n");
					continue;
				}

				/* Get offset  */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Offset conversion error).\n");
					continue;
				}
				j = i + 1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Offset conversion error).\n");
					continue;
				}
				strncpy(buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i] = '\0';
				if ( (sscanf(buf, "%lf", &rasinfo[nfound].h.z_add_offset) ) != 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Offset conversion error).\n");
					continue;
				}

				/* Get NaNflag  */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (NaN flag conversion error).\n");
					continue;
				}
				j = i + 1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
				if (j == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (NaN flag conversion error).\n");
					continue;
				}
				strncpy(buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i] = '\0';
				if (buf[0] == 'n' || buf[0] == 'N') {
					rasinfo[nfound].nanset = 0;
				}
				else {
					rasinfo[nfound].nanset = 1;
					if ( (sscanf(buf, "%d", &rasinfo[nfound].nanflag) ) != 1) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (NaN flag conversion error).\n");
						continue;
					}
				}

				/* Get filename */
				i = j+1;
				while (i < length && (rasinfo[nfound].h.command[i] == ' ' || rasinfo[nfound].h.command[i] == '\t') ) i++;
				if (i == length) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (File name conversion error).\n");
					continue;
				}
				j = i + 1;
				while (j < length && !(rasinfo[nfound].h.command[j] == ' ' || rasinfo[nfound].h.command[j] == '\t') ) j++;
				strncpy(buf, &rasinfo[nfound].h.command[i], j-i);
				buf[j-i] = '\0';

				strncpy (rasinfo[nfound].h.remark, buf, GMT_GRID_REMARK_LEN160-1);

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
						case 'H':
						case 'h':	/* GMT4 LEVEL: Give header size for skipping */
							if (gmt_M_compat_check (GMT, 4)) {
								GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: H<skip>field is deprecated; header is detected automatically.\n");
								rasinfo[nfound].skip = (off_t)atoi (&rasinfo[nfound].h.command[i+1]);	/* Must skip header */
							}
							else {	/* Not allowing backwards compatibility */
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Byte order conversion error).\n");
								continue;
							}
							break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Byte order conversion error).\n");
							continue;
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
						case 'H':
						case 'h':	/* GMT4 LEVEL: Give header size for skipping */
							if (gmt_M_compat_check (GMT, 4)) {
								GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: H<skip>field is deprecated; header is detected automatically.\n");
								rasinfo[nfound].skip = (off_t)atoi (&rasinfo[nfound].h.command[i+1]);	/* Must skip header */
							}
							else {	/* Not allowing backwards compatibility */
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Byte order conversion error).\n");
								continue;
							}
							break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Skipping record in grdraster.info (Byte order conversion error).\n");
							continue;
					}
				}

				/* Get here when all is OK for this line */
				global_lon = 360.0 - (1 - rasinfo[nfound].h.registration) * rasinfo[nfound].h.inc[GMT_X];
				lon_tol = 0.01 * rasinfo[nfound].h.inc[GMT_X];
				global_lon -= lon_tol;	/* make sure we don't fail to find a truly global file  */
				if (rasinfo[nfound].geo && rasinfo[nfound].h.wesn[XHI] - rasinfo[nfound].h.wesn[XLO] >= global_lon) {
					rasinfo[nfound].nglobal = irint (360.0 / rasinfo[nfound].h.inc[GMT_X]);
				}
				else
					rasinfo[nfound].nglobal = 0;

				rasinfo[nfound].h.command[stop_point] = '\0';

				i = lrint ((rasinfo[nfound].h.wesn[XHI] - rasinfo[nfound].h.wesn[XLO])/rasinfo[nfound].h.inc[GMT_X]);
				rasinfo[nfound].h.n_columns = (unsigned int)((rasinfo[nfound].h.registration) ? i : i + 1);
				j = lrint ((rasinfo[nfound].h.wesn[YHI] - rasinfo[nfound].h.wesn[YLO])/rasinfo[nfound].h.inc[GMT_Y]);
				rasinfo[nfound].h.n_rows = (unsigned int)((rasinfo[nfound].h.registration) ? j : j + 1);

				if ((ksize = get_byte_size (GMT, rasinfo[nfound].type)) == 0)
					expected_size = lrint ((ceil (gmt_M_get_nm (GMT, rasinfo[nfound].h.n_columns, rasinfo[nfound].h.n_rows) * 0.125)) + rasinfo[nfound].skip);
				else
					expected_size = (gmt_M_get_nm (GMT, rasinfo[nfound].h.n_columns, rasinfo[nfound].h.n_rows) * ksize + rasinfo[nfound].skip);
				if (gmt_getdatapath (GMT, rasinfo[nfound].h.remark, path, F_OK) || gmt_getsharepath (GMT, "dbase", rasinfo[nfound].h.remark, "", path, F_OK)) {
					strncpy (rasinfo[nfound].h.remark, path, GMT_GRID_REMARK_LEN160-1);
					stat (path, &F);
				}
				else {	/* Inquiry about file failed somehow */
					GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Unable to find file %s - Skipping it.\n", rasinfo[nfound].h.remark);
					continue;
				}

				delta = F.st_size - expected_size;
				if (delta == GMT_GRID_HEADER_SIZE)
					rasinfo[nfound].skip = (off_t)GMT_GRID_HEADER_SIZE;	/* Must skip GMT grd header */
				else if (delta) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Metadata conflict: Actual size of file %s [%" PRIi64 "] differs from expected [%" PRIu64 "]. Verify file and its grdraster.info details.\n", rasinfo[nfound].h.remark, (int64_t)F.st_size, expected_size);
					continue;
				}
				nfound++;

				if (nfound == n_alloc) {
					n_alloc <<= 1;
					rasinfo = gmt_M_memory (GMT, rasinfo, n_alloc, struct GRDRASTER_INFO);
				}
			}
		}
	}
	
	if (!nfound)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No valid records or no existing grid files found in grdraster.info\n");
	else
		rasinfo = gmt_M_memory (GMT, rasinfo, nfound, struct GRDRASTER_INFO);

	*ras = rasinfo;
	return (nfound);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDRASTER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDRASTER_CTRL);

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDRASTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->T.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	struct GRDRASTER_INFO *rasinfo = NULL;
	int i, nrasters;

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdraster <file number>|<text> %s [-G<outgrid>]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T<table>] [%s] [%s]\n\t[%s] [%s]\n", GMT_Id_OPT, GMT_bo_OPT, GMT_do_OPT, GMT_ho_OPT, GMT_o_OPT);

	GMT_Message (API, GMT_TIME_NONE, "\t<file number> (#) or <text> corresponds to one of the datasets listed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[<text> can be a unique substring of the description].\n\n");
	GMT_Message (API, GMT_TIME_NONE, "#	Data Description	Unit	Coverage		Spacing	Registration\n");
	GMT_Message (API, GMT_TIME_NONE, "------------------------------------------------------------------------------------\n");
	nrasters = load_rasinfo (API->GMT, &rasinfo, GMT_ENDIAN);
	for (i = 0; i < nrasters; i++) GMT_Message (API, GMT_TIME_NONE, "%s\n", rasinfo[i].h.command);
	GMT_Message (API, GMT_TIME_NONE, "------------------------------------------------------------------------------------\n\n");
#ifdef WORDS_BIGENDIAN
	GMT_Message (API, GMT_TIME_NONE, "grdraster default binary byte order is Big-endian.\n\n");
#else
	GMT_Message (API, GMT_TIME_NONE, "grdraster default binary byte order is Little-endian.\n\n");
#endif
	gmt_M_free (API->GMT, rasinfo);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "Rg");
	GMT_Message (API, GMT_TIME_NONE, "\t   If r is appended you must also specify a projection with -J (set scale = 1).\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set the filename for output grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify the sampling interval of the grid [Default is raster spacing].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -I<xinc> or -I<xinc>/<yinc> if <yinc> not equal <xinc>.  Append m for minutes or s for seconds.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (-I does not do any filtering; it just sub-samples the raster.)\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the filename for output ASCII table with xyz triplets.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To get binary triplets, see -bo.  Cannot be used with -G.\n");
	GMT_Option (API, "V,bo3,do,h,o,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDRASTER_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdraster and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input text as data description */
			case '#':	/* Input ID number as data set selection */
				Ctrl->In.active = true;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */
			case 'G':
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = true;
				if (gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that arguments were valid */
	gmt_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error -I option: You must specify only one raster file ID.\n");
	if (gmt_M_compat_check (GMT, 4)) {	/* GMT4 LEVEL: In old version we default to triplet output if -G was not set */
		n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && Ctrl->T.active, "Syntax error: You must select only one of -G or -T.\n");
		n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->T.active), "Syntax error: You must select either -G or -T.\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_M_free (GMT, rasinfo); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdraster (void *V_API, int mode, void *args) {
	unsigned int i, j, k, ksize = 0, iselect, imult, jmult, nrasters, row, col;
	unsigned int ijras, jseek, jras2, iras2;
	uint64_t n_nan;
	int jrasstart, irasstart, iras, jras, error = 0;
	bool firstread;
	
	uint64_t ij;
	
	size_t nmask = 0, n_expected;

	char *buffer = NULL, *tselect = NULL, match[GMT_GRID_REMARK_LEN160];
	unsigned char *ubuffer = NULL;
	static unsigned char maskset[8] = {128, 64, 32, 16, 8, 4, 2, 1};

	float *floatrasrow = NULL;

	double tol, grdlatorigin, grdlonorigin, raslatorigin, raslonorigin, *x = NULL, y, out[3];

	FILE *fp = NULL;

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL;
	struct GRDRASTER_INFO myras;
	struct GRDRASTER_INFO *rasinfo = NULL;
	struct GRDRASTER_CTRL *Ctrl = NULL;
	struct GMT_OPTION *r_opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	/* Hardwire a -fg setting since this is geographic data */
	gmt_set_geographic (GMT, GMT_IN);
	gmt_set_geographic (GMT, GMT_OUT);
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdraster main code ----------------------------*/

	if ((nrasters = load_rasinfo (GMT, &rasinfo, GMT_ENDIAN)) == 0) Return (GMT_RUNTIME_ERROR);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %d data sets in grdraster.info\n", nrasters);

	/* Since load_rasinfo processed -R options we need to re-parse the main -R */

	r_opt = GMT_Find_Option (GMT->parent, 'R', options);
	GMT->common.R.active = false;	/* Forget that -R was used before */
	reset_coltype (GMT, r_opt->arg);	/* Make sure geo coordinates will be recognized */
	if (gmt_parse_common_options (GMT, "R", 'R', r_opt->arg)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error reprocessing -R?.\n");
		Return (GMT_RUNTIME_ERROR);
	}

	/* Check if given argument is an integer ID.  If so, assign iselect, else set it to UINT_MAX */

	tselect = strdup (Ctrl->In.file);
	gmt_str_toupper (tselect);	/* Make it upper case - which won't affect integers */
	for (j = i = 0; tselect[j] && i == 0; j++) if (!isdigit ((int)tselect[j])) i = 1;
	if (i == 0)
		iselect = atoi (tselect);
	else
		iselect = UINT_MAX;
	for (i = 0, j = UINT_MAX; !error && i < nrasters; i++) {
		if (iselect != UINT_MAX) {	/* We gave an integer ID */
			if (rasinfo[i].id == iselect) {
				if (j == UINT_MAX)
					j = i;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "At least two rasters have the same file number in grdraster.info\n");
					error++;
				}
			}
		}
		else {	/* We gave a text snippet to match in command */
			strncpy (match, rasinfo[i].h.command, GMT_GRID_REMARK_LEN160-1);
			gmt_str_toupper (match);	/* Make it upper case  */
			if (strstr (match, tselect)) {	/* Found a matching text string */
				if (j == UINT_MAX)
					j = i;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "At least two rasters have the same text [%s] in grdraster.info\n", tselect);
					error++;
				}
			}
		}
	}
	if (j == UINT_MAX) {
		if (iselect != UINT_MAX)
			GMT_Report (API, GMT_MSG_NORMAL, "No raster with file number %d in grdraster.info\n", iselect);
		else
			GMT_Report (API, GMT_MSG_NORMAL, "No raster with text %s in grdraster.info\n", tselect);
		error++;
	}
	else {
		gmt_M_memset (&myras, 1, struct GRDRASTER_INFO);
		myras = rasinfo[j];
	}

	if (gmt_M_compat_check (GMT, 4)) {	/* GMT4 LEVEL: In old version we default to triplet output if -G was not set */
		if (!Ctrl->G.active) Ctrl->T.active = true;
	}

	if (error) {
		free (tselect);
		Return (GMT_RUNTIME_ERROR);
	}

	/* OK, here we have a recognized dataset ID */

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, NULL, Ctrl->I.inc,
	                             GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
		free (tselect);
		Return (API->error);
	}

	gmt_M_memcpy (Grid->header->wesn, GMT->common.R.wesn, 4, double);

	if (myras.geo) {
		gmt_set_geographic (GMT, GMT_IN);
		gmt_set_geographic (GMT, GMT_OUT);
	}
	else {
		gmt_set_cartesian (GMT, GMT_IN);
		gmt_set_cartesian (GMT, GMT_OUT);
	}

	/* Everything looks OK so far.  If (Ctrl->I.active) verify that it will work, else set it.  */
	if (Ctrl->I.active) {
		gmt_M_memcpy (Grid->header->inc, Ctrl->I.inc, 2, double);
		tol = 0.01 * myras.h.inc[GMT_X];
		imult = urint (Grid->header->inc[GMT_X] / myras.h.inc[GMT_X]);
		if (imult < 1 || fabs(Grid->header->inc[GMT_X] - imult * myras.h.inc[GMT_X]) > tol) error++;
		tol = 0.01 * myras.h.inc[GMT_Y];
		jmult = urint (Grid->header->inc[GMT_Y] / myras.h.inc[GMT_Y]);
		if (jmult < 1 || fabs(Grid->header->inc[GMT_Y] - jmult * myras.h.inc[GMT_Y]) > tol) error++;
		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "Your -I option does not create a grid which fits the selected raster (%s)\n",
			            myras.h.command);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else {
		gmt_M_memcpy (Grid->header->inc,  myras.h.inc, 2, double);
		imult = jmult = 1;
	}

	if (GMT->common.R.oblique && GMT->current.proj.projection != GMT_NO_PROJ) {
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, Grid->header->wesn), "")) Return (GMT_PROJECTION_ERROR);

		Grid->header->wesn[XLO] = floor (GMT->common.R.wesn[XLO] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[XHI] = ceil  (GMT->common.R.wesn[XHI] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YLO] = floor (GMT->common.R.wesn[YLO] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		Grid->header->wesn[YHI] = ceil  (GMT->common.R.wesn[YHI] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];

		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE) && rint (Grid->header->inc[GMT_X] * 60.0) == (Grid->header->inc[GMT_X] * 60.0)) {	/* Spacing in even minutes */
			int w, e, s, n, wm, em, sm, nm;

			w = irint (floor (Grid->header->wesn[XLO]));	wm = irint ((Grid->header->wesn[XLO] - w) * 60.0);
			e = irint (floor (Grid->header->wesn[XHI]));	em = irint ((Grid->header->wesn[XHI] - e) * 60.0);
			s = irint (floor (Grid->header->wesn[YLO]));	sm = irint ((Grid->header->wesn[YLO] - s) * 60.0);
			n = irint (floor (Grid->header->wesn[YHI]));	nm = irint ((Grid->header->wesn[YHI] - n) * 60.0);
			GMT_Report (API, GMT_MSG_VERBOSE, "%s -> -R%d:%02d/%d:%02d/%d:%02d/%d:%02d\n", r_opt->arg, w, wm, e, em, s, sm, n, nm);
		}
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "%s -> -R%g/%g/%g/%g\n",
			            r_opt->arg, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI]);
	}

	/* Now Enforce that wesn will fit inc[GMT_X], inc[GMT_Y].  Set n_columns, n_rows but reset later based on G or P  */
	tol = 0.01 * Grid->header->inc[GMT_X];
	Grid->header->n_columns = urint ((Grid->header->wesn[XHI] - Grid->header->wesn[XLO])/Grid->header->inc[GMT_X]);
	if (fabs ((Grid->header->wesn[XHI] - Grid->header->wesn[XLO]) - Grid->header->inc[GMT_X] * Grid->header->n_columns) > tol) error++;
	tol = 0.01 * Grid->header->inc[GMT_Y];
	Grid->header->n_rows = urint ((Grid->header->wesn[YHI] - Grid->header->wesn[YLO])/Grid->header->inc[GMT_Y]);
	if (fabs ((Grid->header->wesn[YHI] - Grid->header->wesn[YLO]) - Grid->header->inc[GMT_Y] * Grid->header->n_rows) > tol) error++;
	if (error) {	/* Must cleanup and give warning */
		Grid->header->wesn[XLO] = floor (Grid->header->wesn[XLO] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[XHI] =  ceil (Grid->header->wesn[XHI] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YLO] = floor (Grid->header->wesn[YLO] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		Grid->header->wesn[YHI] =  ceil (Grid->header->wesn[YHI] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y];
		Grid->header->n_columns = urint ((Grid->header->wesn[XHI] - Grid->header->wesn[XLO]) / Grid->header->inc[GMT_X]);
		Grid->header->n_rows = urint ((Grid->header->wesn[YHI] - Grid->header->wesn[YLO]) / Grid->header->inc[GMT_Y]);
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Your -R option does not create a region divisible by inc[GMT_X], inc[GMT_Y].\n");
		if (doubleAlmostEqualZero (rint (Grid->header->inc[GMT_X] * 60.0), Grid->header->inc[GMT_X] * 60.0)) {	/* Spacing in even minutes */
			int w, e, s, n, wm, em, sm, nm;
			w = irint (floor (Grid->header->wesn[XLO]));	wm = irint ((Grid->header->wesn[XLO] - w) * 60.0);
			e = irint (floor (Grid->header->wesn[XHI]));	em = irint ((Grid->header->wesn[XHI] - e) * 60.0);
			s = irint (floor (Grid->header->wesn[YLO]));	sm = irint ((Grid->header->wesn[YLO] - s) * 60.0);
			n = irint (floor (Grid->header->wesn[YHI]));	nm = irint ((Grid->header->wesn[YHI] - n) * 60.0);
			if (!GMT->common.R.oblique)
				GMT_Report (API, GMT_MSG_NORMAL, "Warning: Region reset to -R%d:%02d/%d:%02d/%d:%02d/%d:%02d.\n",
				            w, wm, e, em, s, sm, n, nm);
			else
				GMT_Report (API, GMT_MSG_NORMAL, "Warning: Region reset to -R%d:%02d/%d:%02d/%d:%02d/%d:%02dr\n",
				            w, wm, s, sm, e, em, n, nm);
		}
		else {
			if (!GMT->common.R.oblique)
				GMT_Report (API, GMT_MSG_NORMAL, "Warning: Region reset to -R%g/%g/%g/%g.\n",
				            Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI]);
			else
				GMT_Report (API, GMT_MSG_NORMAL, "Warning: Region reset to -R%g/%g/%g/%gr.\n",
				            Grid->header->wesn[XLO], Grid->header->wesn[YLO], Grid->header->wesn[XHI], Grid->header->wesn[YHI]);
		}
		error = 0;
	}

	/* Now we are ready to go */
	if (!myras.h.registration) {
		Grid->header->n_columns++;
		Grid->header->n_rows++;
	}
	strncpy (Grid->header->title, myras.h.title, GMT_GRID_TITLE_LEN80-1);
	strncpy (Grid->header->z_units, myras.h.z_units, GMT_GRID_UNIT_LEN80-1);
	strncpy (Grid->header->remark, myras.h.remark, GMT_GRID_REMARK_LEN160-1);
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
	gmt_set_grddim (GMT, Grid->header);
	gmt_M_err_fail (GMT, gmt_grd_RI_verify (GMT, Grid->header, 1), Ctrl->G.file);
	myras.h.xy_off = 0.5 * myras.h.registration;

	grdlatorigin = gmt_M_row_to_y (GMT, 0, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], Grid->header->xy_off, Grid->header->n_rows);
	grdlonorigin = gmt_M_col_to_x (GMT, 0, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], Grid->header->xy_off, Grid->header->n_columns);
	raslatorigin = gmt_M_row_to_y (GMT, 0, myras.h.wesn[YLO], myras.h.wesn[YHI], myras.h.inc[GMT_Y], myras.h.xy_off, myras.h.n_rows);
	raslonorigin = gmt_M_col_to_x (GMT, 0, myras.h.wesn[XLO], myras.h.wesn[XHI], myras.h.inc[GMT_X], myras.h.xy_off, myras.h.n_columns);
	irasstart = irint ((grdlonorigin - raslonorigin) / myras.h.inc[GMT_X]);
	jrasstart = irint ((raslatorigin - grdlatorigin) / myras.h.inc[GMT_Y]);
	if (myras.nglobal) while (irasstart < 0) irasstart += myras.nglobal;
	n_nan = 0;

	/* Get space */
	if (Ctrl->T.active) {	/* Need just space for one row */
		unsigned int col;
		Grid->data = gmt_M_memory_aligned (GMT, NULL, Grid->header->n_columns, float);
		x = gmt_M_memory (GMT, NULL, Grid->header->n_columns, double);
		for (col = 0; col < Grid->header->n_columns; col++) x[col] = gmt_M_col_to_x (GMT, col, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->inc[GMT_X], Grid->header->xy_off, Grid->header->n_columns);
		if ((error = gmt_set_cols (GMT, GMT_OUT, 3)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			gmt_M_free (GMT, x);
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			gmt_M_free (GMT, x);
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			gmt_M_free (GMT, x);
			Return (API->error);
		}
	} else {	/* Need an entire (padded) grid */
		Grid->data = gmt_M_memory_aligned (GMT, NULL, Grid->header->size, float);
	}

	ksize = get_byte_size (GMT, myras.type);
	if (ksize == 0) {	/* Bits; Need to read the whole thing */
		nmask = lrint (ceil (myras.h.n_columns * myras.h.n_rows * 0.125));
		ubuffer = gmt_M_memory (GMT, NULL, nmask, unsigned char);
	}
	else {	/* Need to read by rows, and convert each row to float */
		buffer = gmt_M_memory (GMT, NULL, ksize * myras.h.n_columns, char);
		floatrasrow = gmt_M_memory (GMT, NULL, myras.h.n_columns, float);
	}

	/* Now open file and do it */

	if ((fp = gmt_fopen (GMT, myras.h.remark, "rb") ) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "ERROR opening %s for read.\n", myras.h.remark);
		gmt_M_free (GMT, ubuffer);		gmt_M_free (GMT, buffer);	gmt_M_free (GMT, floatrasrow);
		Return (GMT_ERROR_ON_FOPEN);
	}
	if (myras.skip && fseek (fp, myras.skip, SEEK_CUR)) {
		GMT_Report (API, GMT_MSG_NORMAL, "ERROR skipping %" PRIi64 " bytes in %s.\n", (int64_t)myras.skip, myras.h.remark);
		gmt_M_free (GMT, ubuffer);		gmt_M_free (GMT, buffer);	gmt_M_free (GMT, floatrasrow);
		Return (GMT_RUNTIME_ERROR);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Reading from raster %s\n", myras.h.remark);
	if (myras.swap_me) GMT_Report (API, GMT_MSG_VERBOSE, "Data from %s will be byte-swapped\n", myras.h.remark);

	if (myras.type == 'b') {	/* Must handle bit rasters a bit differently */
		if ((gmt_M_fread (ubuffer, sizeof (unsigned char), nmask, fp)) != nmask) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Failure to read a bitmap raster from %s.\n", myras.h.remark);
			gmt_M_free (GMT, ubuffer);		gmt_M_free (GMT, buffer);	gmt_M_free (GMT, floatrasrow);
			gmt_fclose (GMT, fp);
			Return (GMT_GRID_READ_ERROR);
		}
		for (row = 0, jras = jrasstart; row < Grid->header->n_rows; row++, jras += jmult) {
			y = gmt_M_row_to_y (GMT, row, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], Grid->header->xy_off, Grid->header->n_rows);
			ij = (Ctrl->T.active) ? 0 : gmt_M_ijp (Grid->header, row, 0);	/* Either we just have one row (with no padding) or we have a padded grid */
			if (jras < 0 || (jras2 = jras) > myras.h.n_rows) {
				/* This entire row is outside the raster */
				for (col = 0; col < Grid->header->n_columns; col++, ij++) Grid->data[ij] = GMT->session.f_NaN;
				n_nan += Grid->header->n_columns;
			}
			else {
				iras = irasstart;
				ijras = jras * myras.h.n_columns;
				
				for (col = 0; col < Grid->header->n_columns; col++, ij++) {
					if (myras.nglobal && iras >= myras.nglobal) iras = iras%myras.nglobal;
					if (iras < 0 || (iras2 = iras) >= myras.h.n_columns) {
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
			if (Ctrl->T.active) {	/* Just dump the row as xyz triplets */
				out[1] = y;
				for (col = 0; col < Grid->header->n_columns; col++) {
					out[0] = x[col];
					out[2] = Grid->data[col];
					GMT_Put_Record (API, GMT_WRITE_DATA, out);
				}
			}
		}
		gmt_M_free (GMT, ubuffer);
	}
	else {
		firstread = true;
		n_expected = myras.h.n_columns;
		for (row = 0, jras = jrasstart; row < Grid->header->n_rows; row++, jras += jmult) {
			y = gmt_M_row_to_y (GMT, row, Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->inc[GMT_Y], Grid->header->xy_off, Grid->header->n_rows);
			ij = (Ctrl->T.active) ? 0 : gmt_M_ijp (Grid->header, row, 0);	/* Either we just have one row (no padding) or we have a padded grid */
			if (jras < 0 || (jras2 = jras) > myras.h.n_rows) {
				/* This entire row is outside the raster */
				for (col = 0; col < Grid->header->n_columns; col++, ij++) Grid->data[ij] = GMT->session.f_NaN;
				n_nan += Grid->header->n_columns;
			}
			else {
				if (firstread) {
					jseek = (jras != 0) ? jras : 0;
					firstread = false;
				}
				else if (jmult > 1)
					jseek = jmult - 1;
				else
					jseek = 0;
				/* This will be slow on SGI because seek is broken there */
				if (jseek && fseek (fp, (off_t)jseek * (off_t)ksize * (off_t)myras.h.n_columns, SEEK_CUR) ) {
					GMT_Report (API, GMT_MSG_NORMAL, "ERROR seeking in %s\n", myras.h.remark);
					gmt_fclose (GMT, fp);
					gmt_M_free (GMT, buffer);	gmt_M_free (GMT, floatrasrow);
					Return (GMT_GRID_READ_ERROR);
				}
				if ((gmt_M_fread (buffer, ksize, n_expected, fp)) != n_expected) {
					GMT_Report (API, GMT_MSG_NORMAL, "ERROR reading in %s\n", myras.h.remark);
					gmt_fclose (GMT, fp);
					gmt_M_free (GMT, buffer);	gmt_M_free (GMT, floatrasrow);
					Return (GMT_GRID_READ_ERROR);
				}
#ifdef DEBUG
				GMT_Report (API, GMT_MSG_VERBOSE, "Doing line %06d\r", j);
#endif
				switch (myras.type) {
					case 'u':	/* unsigned char */
						convert_u_row (GMT, &myras, floatrasrow, buffer);
						break;
					case 'c':	/* char */
						convert_c_row (GMT, &myras, floatrasrow, buffer);
						break;
					case 'd':	/* uint16_t */
						convert_d_row (GMT, &myras, floatrasrow, buffer);
						break;
					case 'i':	/* int16_t */
						convert_i_row (GMT, &myras, floatrasrow, buffer);
						break;
					case 'l':	/* int32_t */
						convert_l_row (GMT, &myras, floatrasrow, buffer);
						break;
				}
				iras = irasstart;
				for (col = 0; col < Grid->header->n_columns; col++, ij++) {
					if (myras.nglobal && iras >= myras.nglobal) iras = iras%myras.nglobal;
					if (iras < 0 || (iras2 = iras) >= myras.h.n_columns) {
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
			if (Ctrl->T.active) {	/* Just dump the row as xyz triplets */
				out[GMT_Y] = y;
				for (col = 0; col < Grid->header->n_columns; col++) {
					out[GMT_X] = x[col];
					out[GMT_Z] = Grid->data[col];
					GMT_Put_Record (API, GMT_WRITE_DATA, out);
				}
			}
		}
		gmt_M_free (GMT, buffer);
		gmt_M_free (GMT, floatrasrow);
	}
	gmt_fclose (GMT, fp);

	GMT_Report (API, GMT_MSG_VERBOSE, "Finished reading from %s\n", myras.h.remark);
	GMT_Report (API, GMT_MSG_VERBOSE, "min max and # NaN found: %g %g %" PRIu64 "\n", Grid->header->z_min, Grid->header->z_max, n_nan);

	if (n_nan == Grid->header->n_columns * Grid->header->n_rows) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Your grid file is entirely full of NaNs.\n");

	if (Ctrl->T.active) {
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		gmt_M_free (GMT, x);
	}
	else {
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	Return (GMT_NOERROR);
}
