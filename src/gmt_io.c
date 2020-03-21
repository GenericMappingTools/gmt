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
 * Table input/output in GMT can be either ASCII, binary, or netCDF COARDS files
 * and may consist of single or multiple segments.  These files are accessed
 * via the GMT->current.io.input function pointer which either points to the
 * ASCII read (gmtio_ascii_input), the binary read (gmtio_bin_input), or the
 * netCDF read (gmtio_nc_input) functions, depending on the -bi setting.
 * Similarly, writing of such tables are done via the GMT->current.io.output
 * function pointer which is set to either gmtio_ascii_output, gmtio_bin_output,
 * or GMT_nc_output [not implemented yet].
 * For special processing of z-data by xyz2grd & grd2xyz we also use the
 * GMT->current.io.input/output functions but these are reset in those two
 * programs to the corresponding read/write functions pointed to by the
 * GMT_Z_IO member ->read_item or ->write_item as these can handle additional
 * binary data types such as char, int, short etc.
 * The structure GMT_IO holds parameters that are used during the reading
 * and processing of ASCII tables.  For compliance with a wide variety of
 * binary data formats for grids and their internal nesting the GMT_Z_IO
 * structure and associated functions are used (in xyz2grd and grd2xyz)
 *
 * The following functions are here:
 *
 *  gmtlib_getuserpath  Get pathname of file in "user directories" (GMT->session.TMPDIR, CWD, HOME, GMT->session.USERDIR, GMT->session.CACHEDIR)
 *  gmt_getdatapath     Get pathname of file in "data directories" (CWD, GMT_{USER,CACHE,DATA,GRID,IMG}DIR)
 *  gmt_getsharepath    Get pathname of file in "share directories" (CWD, GMT->session.USERDIR, GMT->session.SHAREDIR tree)
 *  gmt_fopen:          Open a file using gmt_getdatapath
 *  gmt_fclose:         Close a file
 *  gmt_strncpy:        Copy n characters of a char string without triggering a Coverity issue
 *  gmtlib_io_init:     Init GMT_IO structure
 *  gmt_write_segmentheader:   Write header record for multisegment files
 *  gmt_scanf:          Robust scanf function with optional dd:mm:ss conversion
 *  gmt_set_z_io:       Set GMT_Z_IO structure based on -Z
 *  gmt_check_z_io:     Fill in implied missing row/column
 * gmt_set_geographic
 * gmt_set_cartesian
 * gmt_input_is_nan_proxy
 * gmtlib_write_tableheader
 * gmt_fgets
 * gmt_skip_xy_duplicates
 * gmt_is_ascii_record
 * gmtlib_gap_detected
 * gmt_ascii_output_col
 * gmtlib_set_gap
 * gmt_set_segmentheader
 * gmt_set_tableheader
 * gmt_z_output
 * gmt_z_input
 * gmtlib_process_binary_input
 * gmtlib_nc_get_att_text
 * gmt_input_is_bin
 * gmtlib_io_banner
 * gmt_get_cols
 * gmt_set_cols
 * gmt_access
 * gmt_get_ogr_id
 * gmtlib_trim_segheader
 * gmt_is_segment_header
 * gmtio_ascii_textinput
 * gmt_is_a_blank_line
 * gmtlib_bin_colselect
 * gmt_skip_output
 * gmtlib_set_bin_io
 * gmt_format_abstime_output
 * gmt_ascii_format_col
 * gmt_init_io_columns
 * gmt_disable_bhi_opts
 * gmt_reenable_bhi_opts
 * gmt_lon_range_adjust
 * gmt_quad_reset
 * gmt_quad_init
 * gmt_quad_add
 * gmt_quad_finalize
 * gmtlib_get_lon_minmax
 * gmt_eliminate_lon_jumps
 * gmtlib_determine_pole
 * gmt_set_seg_polar
 * gmtlib_geo_to_dms
 * gmt_add_to_record
 * gmtlib_io_binary_header
 * gmtlib_write_textrecord
 * gmt_parse_z_io
 * gmt_get_io_type
 * gmtlib_get_io_ptr
 * gmt_init_z_io
 * gmtlib_clock_C_format
 * gmtlib_date_C_format
 * gmtlib_geo_C_format
 * gmtlib_plot_C_format
 * gmt_scanf_arg
 * gmtlib_read_texttable
 * gmt_set_seg_minmax
 * gmt_set_tbl_minmax
 * gmt_set_dataset_minmax
 * gmt_parse_segment_header
 * gmt_extract_label
 * gmt_parse_segment_item
 * gmt_duplicate_ogr_seg
 * gmtlib_write_newheaders
 * gmt_set_xycolnames
 * gmt_adjust_dataset
 * gmt_alloc_segment
 * gmtlib_assign_segment
 * gmtlib_assign_vector
 * gmt_create_table
 * gmtlib_create_dataset
 * gmtlib_read_table
 * gmt_duplicate_segment
 * gmt_alloc_dataset
 * gmt_free_segment
 * gmt_free_table
 * gmtlib_free_dataset_ptr
 * gmt_free_dataset
 * gmtlib_create_image
 * gmtlib_duplicate_image
 * gmtlib_free_image_ptr
 * gmtlib_free_image
 * gmt_create_vector
 * gmtlib_alloc_univector
 * gmtlib_free_vector_ptr
 * gmt_free_vector
 * gmtlib_create_matrix
 * gmtlib_duplicate_matrix
 * gmtlib_free_matrix_ptr
 * gmtlib_free_matrix
 * gmt_not_numeric
 * gmt_conv_intext2dbl
 * gmtlib_ogr_get_type
 * gmtlib_free_ogr
 * gmtlib_duplicate_ogr
 * gmt_get_aspatial_value
 * gmt_load_aspatial_string
 * gmtlib_get_dir_list
 * gmtlib_free_dir_list
 *
 * Author:  Paul Wessel
 * Date:    1-JAN-2010
 * Version: 5
 * Now 64-bit enabled.
 */

/*!
 * \file gmt_io.c
 * \brief gmt_io.c Table input/output in GMT
 */

#include "gmt_dev.h"
#include "gmt_internals.h"
#include "common_byteswap.h"

/* A few functions needed from elsewhere */
EXTERN_MSC unsigned int gmtapi_count_objects (struct GMTAPI_CTRL *API, enum GMT_enum_family family, unsigned int geometry, unsigned int direction, int *first_ID);
EXTERN_MSC int gmtapi_unregister_io (struct GMTAPI_CTRL *API, int object_ID, unsigned int direction);
EXTERN_MSC int gmtapi_validate_id (struct GMTAPI_CTRL *API, int family, int object_ID, int direction);
EXTERN_MSC char *gmtapi_create_header_item (struct GMTAPI_CTRL *API, unsigned int mode, void *arg);

#ifdef HAVE_DIRENT_H_
#	include <dirent.h>
#endif

#ifdef HAVE_SYS_DIR_H_
#	include <sys/dir.h>
#endif

#ifndef DT_DIR
#	define DT_DIR 4
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#define return_null(GMT,err) { GMT->parent->error = err; return (NULL);}

static const char *GMT_type[GMT_N_TYPES] = {"byte", "byte", "integer", "integer", "integer", "integer",
                                            "integer", "integer", "double", "double", "string", "datetime"};
static const char *GMT_coltype_name[] = {"String", "Number", "Latitude", "Longitude", "Geographic", "RelTime", "AbsTime", "Duration", "Dimension", "Length", "Azimuth", "Angle", "String"};

/* Byteswap widths used with gmt_byteswap_file */
typedef enum {
	Int16len = 2,
	Int32len = 4,
	Int64len = 8
} SwapWidth;

/*! Macro to apply columns log/scale/offset conversion on the fly */
#define gmt_convert_col(S,x) ((S.convert) ? ((S.convert & 2) ? log10 (x) : x) * S.scale + S.offset : x)

/* These functions are defined and used below but not in any *.h file so we repeat them here */
int gmt_get_ogr_id (struct GMT_OGR *G, char *name);
void gmt_format_abstime_output (struct GMT_CTRL *GMT, double dt, char *text);
unsigned int gmt_is_segment_header (struct GMT_CTRL *GMT, char *line);

/* Local functions */

/*! . */
static inline bool outside_in_row_range (struct GMT_CTRL *GMT, int64_t row) {
	/* Returns true of this row should be skipped according to -qi[~]<rows>,...[+a|f|s] */
	bool pass;
	if (GMT->common.q.mode != GMT_RANGE_ROW_IN) return false;	/* -qi<rows> not active */
	pass = GMT->common.q.inverse[GMT_IN];
	for (unsigned int k = 0; k < GMT->current.io.n_row_ranges[GMT_IN]; k++) {
		if (row >= GMT->current.io.row_range[GMT_IN][k].first && row <= GMT->current.io.row_range[GMT_IN][k].last) {	/* row is inside this range */
			if (GMT->current.io.row_range[GMT_IN][k].inc == 1) return pass;	/* Return if we want to use this row or not */
			if ((row - GMT->current.io.row_range[GMT_IN][k].first) % GMT->current.io.row_range[GMT_IN][k].inc == 0) return pass;	/* Hit one of the steps */
		}
	}
	return !pass;
}

/*! . */
static inline bool outside_out_row_range (struct GMT_CTRL *GMT, int64_t row) {
	/* Returns true of this row should be skipped according to -qo[~]<rows>,...[+a|s] */
	bool pass;
	if (GMT->common.q.mode != GMT_RANGE_ROW_OUT) return false;	/* -qo<rows> not active */
	pass = GMT->common.q.inverse[GMT_OUT];
	for (unsigned int k = 0; k < GMT->current.io.n_row_ranges[GMT_OUT]; k++) {
		if (row >= GMT->current.io.row_range[GMT_OUT][k].first && row <= GMT->current.io.row_range[GMT_OUT][k].last) {	/* row is inside this range */
			if (GMT->current.io.row_range[GMT_OUT][k].inc == 1) return pass;	/* Return if we want to use this row or not */
			if ((row - GMT->current.io.row_range[GMT_OUT][k].first) % GMT->current.io.row_range[GMT_OUT][k].inc == 0) return pass;	/* Hit one of the steps */
		}
	}
	return !pass;
}

/*! . */
static inline bool outside_in_data_range (struct GMT_CTRL *GMT, unsigned int col) {
	/* Returns true of this row should be skipped according to -qi[~]<rangevalues>,...+c<col> */
	bool pass;
	if (GMT->common.q.mode != GMT_RANGE_DATA_IN) return false;	/* -qi<data> not active */
	pass = GMT->common.q.inverse[GMT_IN];
	if (gmt_M_is_dnan (GMT->current.io.curr_rec[col])) return !pass;	/* A NaN value is never in a range */
	for (unsigned int k = 0; k < GMT->current.io.n_row_ranges[GMT_IN]; k++) {
		if (GMT->current.io.curr_rec[col] >= GMT->current.io.data_range[GMT_IN][k].first && GMT->current.io.curr_rec[col] <= GMT->current.io.data_range[GMT_IN][k].last) return pass;	/* Inside this range at least */
	}
	return !pass;
}

/*! . */
static inline bool outside_out_data_range (struct GMT_CTRL *GMT, unsigned int col, double *data) {
	/* Returns true of this row should be skipped according to -qo[~]<rangevalues>,...+c<col> */
	bool pass = GMT->common.q.inverse[GMT_OUT];
	for (unsigned int k = 0; k < GMT->current.io.n_row_ranges[GMT_OUT]; k++) {
		if (data[col] >= GMT->current.io.data_range[GMT_OUT][k].first && data[col] <= GMT->current.io.data_range[GMT_OUT][k].last) return pass;	/* Inside this range at least */
	}
	return !pass;
}

/*! . */
static inline uint64_t gmt_n_cols_needed_for_gaps (struct GMT_CTRL *GMT, uint64_t n) {
	/* Return the actual items needed (which may be more than n if gap testing demands it) */
	if (GMT->common.g.active) return (MAX (n, GMT->common.g.n_col));	/* n or n_col (if larger) */
	return (n);	/* No gap checking, n it is */
}

/*! . */
static inline void gmt_update_prev_rec (struct GMT_CTRL *GMT, uint64_t n_use) {
	/* Update previous record before reading the new record*/
	if (GMT->current.io.need_previous) gmt_M_memcpy (GMT->current.io.prev_rec, GMT->current.io.curr_rec, n_use, double);
}

/* Begin private functions used by gmt_byteswap_file() */

#define DEBUG_BYTESWAP

/*! . */
static inline bool fwrite_check (struct GMT_CTRL *GMT, const void *ptr,
		size_t size, size_t nitems, FILE *stream) {
	if (fwrite (ptr, size, nitems, stream) != nitems) {
		char message[GMT_LEN256] = {""};
		snprintf (message, GMT_LEN256, "%s: error writing %" PRIuS " bytes to stream.\n", __func__, size * nitems);
			GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
		return true;
	}
	return false;
}

/*! . */
static inline void swap_uint16 (char *buffer, const size_t len) {
	/* byteswap uint16_t in buffer of length 'len' bytes */
	uint16_t u;
	size_t n;

	for (n = 0; n < len; n+=Int16len) {
		memcpy (&u, &buffer[n], Int16len);
		u = bswap16 (u);
		memcpy (&buffer[n], &u, Int16len);
	}
}

/*! . */
static inline void swap_uint32 (char *buffer, const size_t len) {
	/* byteswap uint32_t in buffer of length 'len' bytes */
	uint32_t u;
	size_t n;

	for (n = 0; n < len; n+=Int32len) {
		memcpy (&u, &buffer[n], Int32len);
		u = bswap32 (u);
		memcpy (&buffer[n], &u, Int32len);
	}
}

/*! . */
static inline void swap_uint64 (char *buffer, const size_t len) {
	/* byteswap uint64_t in buffer of length 'len' bytes */
	uint64_t u;
	size_t n;

	for (n = 0; n < len; n+=Int64len) {
		memcpy (&u, &buffer[n], Int64len);
		u = bswap64 (u);
		memcpy (&buffer[n], &u, Int64len);
	}
}

/* End private functions used by gmt_byteswap_file() */

#ifdef HAVE_DIRENT_H_
/*! . */
GMT_LOCAL bool gmtio_traverse_dir (const char *file, char *path) {
	/* Look for file in the directory pointed to by path, recursively */
	DIR *D = NULL;
	struct dirent *F = NULL;
	int len, d_namlen;
	bool ok = false;
	char savedpath[PATH_MAX];

 	if ((D = opendir (path)) == NULL) return (false);	/* Unable to open directory listing */
	len = (int)strlen (file);
	strncpy (savedpath, path, PATH_MAX-1);	/* Make copy of current directory path */

	while (!ok && (F = readdir (D)) != NULL) {	/* For each directory entry until end or ok becomes true */
		d_namlen = (int)strlen (F->d_name);
		if (d_namlen == 1 && F->d_name[0] == '.') continue;				/* Skip current dir */
		if (d_namlen == 2 && F->d_name[0] == '.' && F->d_name[1] == '.') continue;	/* Skip parent dir */
#ifdef HAVE_SYS_DIR_H_
		if (F->d_type == DT_DIR) {	/* Entry is a directory; must search this directory recursively */
			sprintf (path, "%s/%s", savedpath, F->d_name);
			ok = gmtio_traverse_dir (file, path);
		}
		else if (d_namlen == len && !strcmp (F->d_name, file)) {	/* Found the file in this dir (i.e., F_OK) */
			sprintf (path, "%s/%s", savedpath, file);
			ok = true;
		}
#endif /* HAVE_SYS_DIR_H_ */
	}
	(void)closedir (D);
	return (ok);	/* did or did not find file */
}
#endif /* HAVE_DIRENT_H_ */

/*! . */
GMT_LOCAL void gmtio_adjust_periodic_lon (struct GMT_CTRL *GMT, double *val) {
	while (*val > GMT->common.R.wesn[XHI] && (*val - 360.0) >= GMT->common.R.wesn[XLO])
		*val -= 360.0;
	while (*val < GMT->common.R.wesn[XLO] && (*val + 360.0) <= GMT->common.R.wesn[XLO]) *val += 360.0;
	/* If data is not inside the given range it will satisfy (lon > east) */
	/* Now it will be outside the region on the same side it started out at */
}

/*! . */
GMT_LOCAL void gmtio_adjust_projected (struct GMT_CTRL *GMT) {
	/* Case of incoming projected map coordinates that we wish to revert to lon/lat */
	if (GMT->current.proj.inv_coord_unit != GMT_IS_METER) {	/* Must first scale to meters */
		GMT->current.io.curr_rec[GMT_X] *= GMT->current.proj.m_per_unit[GMT->current.proj.inv_coord_unit];
		GMT->current.io.curr_rec[GMT_Y] *= GMT->current.proj.m_per_unit[GMT->current.proj.inv_coord_unit];
	}
	(*GMT->current.proj.inv) (GMT, &GMT->current.io.curr_rec[GMT_X], &GMT->current.io.curr_rec[GMT_Y],
	                          GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]);
}

/*! Return the floating point value associated with the aspatial value V given its type as a double */
GMT_LOCAL double gmtio_convert_aspatial_value (struct GMT_CTRL *GMT, unsigned int type, char *V) {

	double value;

	switch (type) {
		case GMT_DOUBLE:
		case GMT_FLOAT:
		case GMT_ULONG:
		case GMT_LONG:
		case GMT_UINT:
		case GMT_INT:
		case GMT_USHORT:
		case GMT_SHORT:
		case GMT_CHAR:
		case GMT_UCHAR:
			value = atof (V);
			break;
		case GMT_DATETIME:
			gmt_scanf_arg (GMT, V, GMT_IS_ABSTIME, false, &value);
			break;
		default:	/* Give NaN */
			value = GMT->session.d_NaN;
			break;
	}
	return (value);
}

/*! . */
GMT_LOCAL void gmtio_handle_bars (struct GMT_CTRL *GMT, char *in, unsigned way) {
	/* Way = 0: replace | inside quotes with ASCII 1, Way = 1: Replace ASCII 1 with | */
	/* Need to handle cases like ...|"St. George's Channel"|... with a mix of quotes. */
	char the_quote = '\0';	/* Don't know what type of quote is used */
	gmt_M_unused(GMT);
	if (in == NULL || in[0] == '\0') return;	/* No string to check */
	if (way == 0) {	/* Replace | found inside quotes with a single ASCII 1 */
		char *c = in;
		bool replace = false;
		while (*c && the_quote == '\0') {	/* Find the first quote character which we assume is what is used for a sentence which may contain another quote */
			if (*c == '\"' || *c == '\'') the_quote = *c;
			++c;
		}
		if (the_quote == '\0') the_quote = '\"';	/* No quotes found so just set the default quote */
		c = in;	/* Start over */
		while (*c) {
			if (*c == the_quote)
				replace = !replace;
			else if (*c == '|' && replace)
				*c = 1;
			++c;
		}
	}
	else /* way != 0: Replace single ASCII 1 with | */
		gmt_strrepc (in, 1, '|');
}

/*! . */
GMT_LOCAL bool gmt_skip_record (struct GMT_CTRL *GMT, struct GMT_TEXT_SELECTION *S, char *record) {
	/* Return true if the pattern was found; see gmt_set_text_selection for details */
	bool match = false;
	if (S == NULL || S->n == 0) return (true);	/* No selection criteria given, so can only return true */
	/* Could be one or n patterns to check */
	for (uint64_t k = 0; !match && k < S->n; k++) {
#if !defined(WIN32) || (defined(WIN32) && defined(HAVE_PCRE)) || (defined(WIN32) && defined(HAVE_PCRE2))
		if (S->regexp[k])
		 	match = gmtlib_regexp_match (GMT, record, S->pattern[k], S->caseless[k]);	/* true if we matched */
		else
#endif
			match = (strstr (record, S->pattern[k]) != NULL);
	}
	/* Here, match == true if we found the pattern we specified.  There are now two cases to consider:
	   1) invert is false, which means we want to skip records when match == false.
	      So if match == false and invert == false then we will skip.
	   2) invert == true, which means we want to skip recprds when match == true.
	      So if match == true and invert == true then we will skip
	   This means in either scenario, the test below is true if we want to skip. */
	return (S->invert == match);	/* Returns true if we should skip this record */
}

/*! . */
GMT_LOCAL unsigned int gmtio_ogr_decode_aspatial_values (struct GMT_CTRL *GMT, char *record, struct GMT_OGR *S) {
	/* Parse @D<vals> aspatial values; this is done once per feature (segment).  We store
 	 * both the text representation (value) and attempt to convert to double in dvalue.
 	 * We use S->n_aspatial to know how many values there are .*/
	unsigned int col = 0;
	char buffer[GMT_BUFSIZ] = {""}, *token, *stringp;

	if (S->n_aspatial == 0) return (0);	/* Nothing to do */
	if (S->tvalue == NULL) {			/* First time, allocate space */
		S->tvalue = gmt_M_memory (GMT, S->tvalue, S->n_aspatial, char *);
		S->dvalue = gmt_M_memory (GMT, S->dvalue, S->n_aspatial, double);
	}
	strncpy (buffer, record, GMT_BUFSIZ-1); /* working copy */
	gmtio_handle_bars (GMT, buffer, 0);	/* Replace vertical bars inside quotes with ASCII 1 */
	stringp = buffer;
	while ( (token = strsep (&stringp, "|")) != NULL ) {
		if (col >= S->n_aspatial) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bad OGR/GMT: @D record has more items than declared by @N\n");
			continue;
		}
		gmtio_handle_bars (GMT, token, 1);		/* Put back any vertical bars replaced above */
		gmt_M_str_free (S->tvalue[col]);	/* Free previous item */
		S->tvalue[col] = strdup (token);
		S->dvalue[col] = gmtio_convert_aspatial_value (GMT, S->type[col], token);
		col++;
	}
	if (col == (S->n_aspatial-1)) {	/* Last item was blank and hence not returned by strsep */
		S->tvalue[col] = strdup ("");	/* Allocate space for blank string */
	}
	return (col);
}

/*! Duplicate in to out, then find the first space not inside quotes and truncate string there */
GMT_LOCAL void gmtio_copy_and_truncate (char *out, char *in) {
	bool quote = false;
	while (*in && (quote || *in != ' ')) {
		*out++ = *in;	/* Copy char */
		if (*in++ == ' ') quote = !quote;	/* Wind to next space except skip if inside double quotes */
	}
	*out = '\0';	/* Terminate string */
}

/*! Parse @T aspatial types; this is done once per dataset and follows @N */
GMT_LOCAL unsigned int gmtio_ogr_decode_aspatial_types (struct GMT_CTRL *GMT, char *record, struct GMT_OGR *S) {
	unsigned int pos = 0, col = 0;
	int t;
	size_t n_alloc = 0;
	char buffer[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ];

	gmtio_copy_and_truncate (buffer, record);
	while ((gmt_strtok (buffer, "|", &pos, p))) {
		if (col >= S->n_aspatial) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bad OGR/GMT: @T record has more items than declared by @N - skipping\n");
			continue;
		}
		if (col == n_alloc) S->type = gmt_M_memory (GMT, S->type, n_alloc += GMT_TINY_CHUNK, enum GMT_enum_type);
		if ((t = gmtlib_ogr_get_type (p)) < 0) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bad OGR/GMT: @T: No such type: %s - skipping\n", p);
			continue;
		}
		S->type[col] = t;
		col++;
	}
	if (col < n_alloc) S->type = gmt_M_memory (GMT, S->type, col, enum GMT_enum_type);
	return (col);
}

GMT_LOCAL void gmtio_select_all_ogr_if_requested (struct GMT_CTRL *GMT) {
	/* If -a with no args was provided we select all available aspatial information to be added to input record */
	unsigned int k, kn;
	if (GMT->current.io.OGR == NULL) return;		/* No can do */
	if (GMT->current.io.OGR->n_aspatial == 0) return;	/* No can do */
	if (GMT->common.a.active == false) return;		/* -a not given */
	if (GMT->common.a.n_aspatial) {		/* -a parsed and stuff was found; check if -a names are correct */
		bool found;
		for (k = 0; k < GMT->common.a.n_aspatial; k++) {
			for (kn = 0, found = false; !found && kn < GMT->current.io.OGR->n_aspatial; kn++)
				found = (!strcmp (GMT->common.a.name[k], GMT->current.io.OGR->name[kn]));
			if (!found) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -a: No such named aspatial item: %s.\n", GMT->common.a.name[k]);
				GMT_exit (GMT, GMT_NOT_OUTPUT_OBJECT); return;
			}
		}
		return;
	}
	GMT->common.a.n_aspatial = GMT->current.io.OGR->n_aspatial;
	for (k = kn = 0; k < GMT->common.a.n_aspatial; k++) {
		GMT->common.a.col[k] = 2 + kn;
		GMT->common.a.ogr[k] = k;
		GMT->common.a.type[k] = GMT->current.io.OGR->type[k];
		gmt_M_str_free (GMT->common.a.name[k]);	/* Just in case */
		GMT->common.a.name[k] = strdup (GMT->current.io.OGR->name[k]);
		if (GMT->common.a.type[k] != GMT_TEXT) kn++;	/* Since that is the order in the numerical part of the record */
	}
}

/*! Decode @N aspatial names; this is done once per dataset */
GMT_LOCAL unsigned int gmtio_ogr_decode_aspatial_names (struct GMT_CTRL *GMT, char *record, struct GMT_OGR *S) {
	unsigned int pos = 0, col = 0;
	size_t n_alloc = 0;
	char buffer[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""};

	gmtio_copy_and_truncate (buffer, record);
	while ((gmt_strtok (buffer, "|", &pos, p))) {
		if (col == n_alloc) S->name = gmt_M_memory (GMT, S->name, n_alloc += GMT_TINY_CHUNK, char *);
		S->name[col++] = strdup (p);
	}
	if (col < n_alloc) S->name = gmt_M_memory (GMT, S->name, col, char *);
	return (col);
}

/*! Parsing of the GMT/OGR vector specification (v 1.0). See Appendix R */
GMT_LOCAL bool gmtio_ogr_parser (struct GMT_CTRL *GMT, char *record) {
	return (GMT->current.io.ogr_parser (GMT, record));	/* We call either the header or data parser depending on pointer */
}

GMT_LOCAL bool gmtio_ogr_data_parser (struct GMT_CTRL *GMT, char *record) {
	/* Parsing of the GMT/OGR vector specification (v 1.0) for data feature records.
 	 * We KNOW GMT->current.io.ogr == GMT_OGR_TRUE, i.e., current file is a GMT/OGR file.
	 * We also KNOW that GMT->current.io.OGR has been allocated by gmtio_ogr_header_parser.
	 * For GMT/OGR files we must parse and store the metadata in GMT->current.io.OGR,
	 * from where higher-level functions can access it.  GMT_End_IO will free the structure.
	 * This function returns true if we parsed a GMT/OGR record and false otherwise.
	 * If we encounter a parsing error we stop parsing any further by setting GMT->current.io.ogr = GMT_OGR_FALSE.
	 * We loop until all @<info> tags have been processed on this record.
	 */

	unsigned int n_aspatial;
	bool quote;
	char *p = NULL;
	struct GMT_OGR *S = NULL;

	if (record[0] != '#') return (false);			/* Not a comment record so no point looking further */
	if (!(p = strchr (record, '@'))) return (false);	/* Not an OGR/GMT record since @ was not found */

	/* Here we are reasonably sure that @? strings are OGR/GMT feature specifications */

	gmt_chop (record);	/* Get rid of linefeed etc */

	S = GMT->current.io.OGR;	/* Set S shorthand */
	quote = false;

	while (*p == '@') {
		++p;	/* Move to first char after @ */
		switch (p[0]) {	/* These are the feature tags only: @D, @P, @H */
			case 'D':	/* Aspatial data values, store in segment header  */
				if (!S->geometry) { GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @D given but no geometry set\n"); return (false);}
				n_aspatial = gmtio_ogr_decode_aspatial_values (GMT, &p[1], S);
				if (S->n_aspatial != n_aspatial) {
					GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "OGR/GMT: Some @D items not specified (set to NULL) near line %" PRIu64 "\n", GMT->current.io.rec_no);
					GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "OGR/GMT: Offending record: %s\n", record);
				}
				break;

			case 'P':	/* Polygon perimeter, store in segment header  */
				if (!(S->geometry == GMT_IS_POLYGON || S->geometry == GMT_IS_MULTIPOLYGON)) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @P only valid for polygons\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
					return (false);
				}
				S->pol_mode = GMT_IS_PERIMETER;
				break;

			case 'H':	/* Polygon hole, store in segment header  */
				if (!(S->geometry == GMT_IS_POLYGON || S->geometry == GMT_IS_MULTIPOLYGON)) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @H only valid for polygons\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
					return (false);
				}
				S->pol_mode = GMT_IS_HOLE;
				break;

			default:	/* Bad OGR record? */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: Cannot have @%c after FEATURE_DATA\n", p[0]);
				GMT->current.io.ogr = GMT_OGR_FALSE;
				break;
		}
		while (*p && (quote || *p != '@')) if (*p++ == '\"') quote = !quote;	/* Wind to next @ except skip if inside double quotes */
	}
	return (true);
}

/*! Simplify aspatial data grabbing when -a is used */
GMT_LOCAL void gmtio_align_ogr_values (struct GMT_CTRL *GMT) {
	unsigned int k;
	int id;
	if (!GMT->common.a.active) return;	/* Nothing selected with -a */
	for (k = 0; k < GMT->common.a.n_aspatial; k++) {	/* Process the requested columns */
		id = gmt_get_ogr_id (GMT->current.io.OGR, GMT->common.a.name[k]);	/* See what order in the OGR struct this -a column appear */
		GMT->common.a.ogr[k] = id;
	}
}

/*! . */
GMT_LOCAL bool gmtio_ogr_header_parser (struct GMT_CTRL *GMT, char *record) {
	/* Parsing of the GMT/OGR vector specification (v 1.0).
 	 * GMT->current.io.ogr can have three states:
	 *	GMT_OGR_UNKNOWN (-1) if not yet set [this is how it is initialized in GMTAPI_Begin_IO].
	 *	GMT_OGR_FALSE    (0) if file has been determined NOT to be a GMT/OGR file.
	 *	GMT_OGR_TRUE    (+1) if it has met the criteria and is a GMT/OGR file.
	 * For GMT/OGR files we must parse and store the metadata in GMT->current.io.OGR,
	 * from where higher-level functions can access it.  GMT_End_IO will free the structure.
	 * This function returns true if we parsed a GMT/OGR record and false otherwise.
	 * If we encounter a parsing error we stop parsing any further by setting GMT->current.io.ogr = GMT_OGR_FALSE.
	 * We loop until all @<info> tags have been processed on this record.
	 * gmtio_ogr_parser will point to this function until the header has been parsed, then it is
	 * set to point to gmtio_ogr_data_parser instead, to speed up data record processing.
	 */

	unsigned int n_aspatial, k, geometry = 0;
	bool quote;
	char *p = NULL;
	struct GMT_OGR *S = NULL;

	if (GMT->current.io.ogr == GMT_OGR_FALSE) return (false);	/* No point parsing further if we KNOW it is not OGR */
	if (record[0] != '#') return (false);			/* Not a comment record so no point looking any further */
	if (GMT->current.io.ogr == GMT_OGR_TRUE && !strncmp (record, "# FEATURE_DATA", 14)) {	/* It IS an OGR file and we found end of OGR header section and start of feature data */
		GMT->current.io.ogr_parser = &gmtio_ogr_data_parser;	/* From now on only parse for feature tags */
		gmtio_align_ogr_values (GMT);	/* Simplify copy from aspatial values to input columns as per -a option */
		return (true);
	}
	if (!(p = strchr (record, '@'))) return (false);	/* Not an OGR/GMT record since @ was not found */

	if (GMT->current.io.ogr == GMT_OGR_UNKNOWN && !strncmp (p, "@VGMT", 5)) {	/* Found the OGR version identifier, look for @G if on the same record */
		if (GMT->common.a.output) {	/* Cannot read OGR files when -a is used to define output */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot read OGR/GMT files when -a is used to define output format\n");
			GMT_exit (GMT, GMT_NOT_OUTPUT_OBJECT); return false;
		}
		GMT->current.io.ogr = GMT_OGR_TRUE;		/* File is now known to be a GMT/OGR geospatial file */
		if (!(p = strchr (&p[5], '@'))) return (true);	/* No more @ codes; goto next record */
	}
	if (GMT->current.io.ogr != GMT_OGR_TRUE) return (false);	/* No point parsing further since file is not GMT/OGR (at least not yet) */

	/* Here we are reasonably sure that @? strings are OGR/GMT header specifications */

	gmt_chop (record);	/* Get rid of linefeed etc */

	/* Allocate S the first time we get here */

	if (!GMT->current.io.OGR) GMT->current.io.OGR = gmt_M_memory (GMT, NULL, 1, struct GMT_OGR);
	S = GMT->current.io.OGR;
	quote = false;

	while (*p == '@') {
		++p;	/* Move to first char after @ */

		switch (p[0]) {	/* These are the header tags */

			case 'G':	/* Geometry */
				if (!strncmp (&p[1], "LINESTRING", 10))
					geometry = GMT_IS_LINESTRING;
				else if (p[1] == 'P') {
					if (!strncmp (&p[2], "OLYGON", 6))
						geometry = GMT_IS_POLYGON;
					else if (!strncmp (&p[2], "OINT", 4))
						geometry = GMT_IS_POINT;
				}
				else if (!strncmp (&p[1], "MULTI", 5)) {
					if (!strncmp (&p[6], "POINT", 5))
						geometry = GMT_IS_MULTIPOINT;
					else if (!strncmp (&p[6], "LINESTRING", 10))
						geometry = GMT_IS_MULTILINESTRING;
					else if (!strncmp (&p[6], "POLYGON", 7))
						geometry = GMT_IS_MULTIPOLYGON;
					else {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @G unrecognized geometry\n");
						GMT->current.io.ogr = GMT_OGR_FALSE;
						return (false);
					}
				}
				if (!S->geometry)
					S->geometry = geometry;
				else if (S->geometry != geometry) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @G cannot have different geometries\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
				}
				break;

			case 'N':	/* Aspatial name fields, store in table header */
				if (!S->geometry) {GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @N given but no geometry set\n"); return (false);}
				if (S->name) {	/* Already set */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @N Cannot have more than one per segment\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
					return (false);
				}
				else
					n_aspatial = gmtio_ogr_decode_aspatial_names (GMT, &p[1], S);
				if (S->n_aspatial == 0)
					S->n_aspatial = n_aspatial;
				else if (S->n_aspatial != n_aspatial) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @N number of items vary\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
				}
				break;

			case 'J':	/* Dataset projection strings (one of 4 kinds) */
				switch (p[1]) {
					case 'e': k = 0;	break;	/* EPSG code */
					case 'g': k = 1;	break;	/* GMT proj code */
					case 'p': k = 2;	break;	/* Proj.4 code */
					case 'w': k = 3;	break;	/* OGR WKT representation */
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @J given unknown format (%c)\n", (int)p[1]);
						GMT->current.io.ogr = GMT_OGR_FALSE;
						return (false);
				}
				S->proj[k] = strdup (&p[2]);
				break;

			case 'R':	/* Dataset region */
				if (S->region) { /* Already set */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @R can only appear once\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
					return (false);
				}
				S->region = strdup (&p[1]);
				break;

			case 'T':	/* Aspatial field types, store in table header  */
				if (!S->geometry) { GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @T given but no geometry set\n"); return (false);}
				if (S->type) {	/* Already set */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @T Cannot have more than one per segment\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
					return (false);
				}
				n_aspatial = gmtio_ogr_decode_aspatial_types (GMT, &p[1], S);
				if (S->n_aspatial == 0)
					S->n_aspatial = n_aspatial;
				else if (S->n_aspatial != n_aspatial) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @T number of items vary\n");
					GMT->current.io.ogr = GMT_OGR_FALSE;
				}
				break;

			default:	/* Just record, probably means this is NOT a GMT/OGR file after all */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad OGR/GMT: @%c not allowed before FEATURE_DATA\n", (int)p[0]);
				GMT->current.io.ogr = GMT_OGR_FALSE;
				break;
		}

		while (*p && (quote || *p != '@')) if (*p++ == '\"') quote = !quote;	/* Wind to next @ except skip if inside double quotes */
	}
	return (true);
}

/*! . */
GMT_LOCAL unsigned int gmtio_reconsider_rectype (struct GMT_CTRL *GMT) {
	/* If any text aspatial field is selected the record type must change to mixed. */

	unsigned int k, val = 0;
	if (GMT->current.io.ogr != GMT_OGR_TRUE) return (0);	/* No point checking further since file is not GMT/OGR */

	for (k = 0; k < GMT->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (GMT->common.a.col[k] < 0) continue;		/* Not meant for data columns but for segment headers */
		if (GMT->current.io.OGR->type[GMT->common.a.ogr[k]] == GMT_TEXT)
			val |= GMT_READ_TEXT;	/* Since it may not have been set */
	}
	return (val);
}

/*! . */
GMT_LOCAL unsigned int gmtio_assign_aspatial_cols (struct GMT_CTRL *GMT) {
	/* This function will load input columns with aspatial data as requested by -a.
 	 * It will then handle any possible -i scalings/offsets as well for those columns.
 	 * This is how the @D values end up in the input data record we read
     * Not: This applies to numerical aspatial columns.  Any numerical aspatial
     * columns will be appended to the current trailing text string. */

	unsigned int k, n, nt;
	if (GMT->current.io.ogr != GMT_OGR_TRUE) return (0);	/* No point checking further since file is not GMT/OGR */

	for (k = n = nt = 0; k < GMT->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (GMT->common.a.col[k] < 0) continue;	/* Not meant for data columns but for segment headers */
		if (GMT->current.io.OGR->type[GMT->common.a.ogr[k]] == GMT_TEXT) {	/* Text goes into trailing text */
			char *tvalue = GMT->current.io.OGR->tvalue[GMT->common.a.ogr[k]];
			size_t len = strlen (tvalue);
			unsigned int pos = 0;
			if (len && tvalue[0] == '\"' && tvalue[len-1] == '\"') {	/* Eliminate quotes for quoted text */
				tvalue[len-1] = '\0';
				pos = 1;
			}
			if (nt) strcat (GMT->current.io.curr_trailing_text, GMT->current.setting.io_col_separator);
			strncat (GMT->current.io.curr_trailing_text, &tvalue[pos], GMT_BUFSIZ-1);
			GMT->current.io.record.text = GMT->current.io.curr_trailing_text;	/* Since it may not have been set */
			nt++;
		}
		else {	/* Numerical adds to data columns */
			double value = GMT->current.io.OGR->dvalue[GMT->common.a.ogr[k]];
			GMT->current.io.curr_rec[GMT->common.a.col[k]] = gmt_convert_col (GMT->current.io.col[GMT_IN][GMT->common.a.col[k]], value);
			n++;
		}
	}
	return (n);	/* Only numerical columns add to the count */
}

/*! Fill a string with the aspatial columns */
void gmt_list_aspatials (struct GMT_CTRL *GMT, char buffer[]) {
	char item[GMT_LEN64] = {""};
	sprintf (buffer, "Aspatial columns:");
	for (unsigned int k = 0; k < GMT->common.a.n_aspatial; k++) {
		sprintf (item, " %s[%s]", GMT->common.a.name[k], GMT_type[GMT->common.a.type[k]]);
		strcat (buffer, item);
	}
}

/*! Returns true if record is NaN NaN [NaN NaN] etc */
GMT_LOCAL bool gmtio_is_a_NaN_line (struct GMT_CTRL *GMT, char *line) {
	unsigned int pos = 0;
	char p[GMT_LEN256] = {""};

	while ((gmt_strtok (line, GMT->current.io.scan_separators, &pos, p))) {
		gmt_str_tolower (p);
		if (strncmp (p, "nan", 3U)) return (false);
	}
	return (true);
}

/* Sub functions for gmtio_bin_input */

/*! . */
GMT_LOCAL int gmtio_x_read (struct GMT_CTRL *GMT, FILE *fp, off_t rel_move) {
	/* Used to skip rel_move bytes; no reading takes place */
	if (fseek (fp, rel_move, SEEK_CUR)) {
		GMT->current.io.status = GMT_IO_EOF;
		return (GMT_DATA_READ_ERROR);
	}
	return (GMT_OK);
}

/*! Reads the n binary doubles from input and saves to GMT->current.io.curr_rec[] */
GMT_LOCAL bool gmtio_get_binary_input (struct GMT_CTRL *GMT, FILE *fp, uint64_t n) {
	uint64_t i;

	if (n > GMT_MAX_COLUMNS) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Number of data columns (%d) exceeds limit (GMT_MAX_COLUMNS = %d)\n", n, GMT_MAX_COLUMNS);
		return (true);	/* Done with this file */
	}
	for (i = 0; i < n; i++) {
		if (GMT->current.io.fmt[GMT_IN][i].skip < 0) gmtio_x_read (GMT, fp, -GMT->current.io.fmt[GMT_IN][i].skip);	/* Pre-skip */
		if (GMT->current.io.fmt[GMT_IN][i].io (GMT, fp, 1, &GMT->current.io.curr_rec[i]) == GMT_DATA_READ_ERROR) {
			/* EOF or came up short */
			GMT->current.io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
			if (GMT->current.io.give_report && GMT->current.io.n_bad_records) {
				/* Report summary and reset */
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "This file had %" PRIu64 " data records with invalid x and/or y values\n", GMT->current.io.n_bad_records);
				GMT->current.io.n_bad_records = GMT->current.io.rec_no = GMT->current.io.data_record_number_in_set[GMT_IN] = GMT->current.io.n_clean_rec = 0;
			}
			return (true);	/* Done with this file */
		}
		if (GMT->current.io.fmt[GMT_IN][i].skip > 0) gmtio_x_read (GMT, fp, GMT->current.io.fmt[GMT_IN][i].skip);	/* Post-skip */
	}
	return (false);	/* OK so far */
}

/*! . */
GMT_LOCAL int gmtio_x_write (struct GMT_CTRL *GMT, FILE *fp, off_t n) {
	/* Used to write n bytes of space for filler on binary output */
	char c = ' ';
	off_t i;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		if (gmt_M_fwrite (&c, sizeof (char), 1U, fp) != 1U)
		return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtio_bin_output (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *ptr, char *txt) {
	/* Return 0 if record was suppressed, otherwise number of items written */
	int k;
	uint64_t i, n_out, col_pos;
	double val;
	gmt_M_unused (txt);

	if (gmt_skip_output (GMT, ptr, n)) return (-1);	/* Record was skipped via -s[a|r] */
	if (GMT->current.setting.io_lonlat_toggle[GMT_OUT])		/* Write lat/lon instead of lon/lat */
		gmt_M_double_swap (ptr[GMT_X], ptr[GMT_Y]);
	n_out = (GMT->common.o.select) ? GMT->common.o.n_cols : n;
	for (i = 0, k = 0; i < n_out; i++) {
		col_pos = (GMT->common.o.select) ? GMT->current.io.col[GMT_OUT][i].col : i;	/* Which data column to pick */
		val = (col_pos >= n) ? GMT->session.d_NaN : ptr[col_pos];	/* If we request beyond length of array, return NaN */
		if (GMT->common.d.active[GMT_OUT] && gmt_M_is_dnan (val)) val = GMT->common.d.nan_proxy[GMT_OUT];	/* Write this value instead of NaNs */
		if (gmt_M_type (GMT, GMT_OUT, col_pos) == GMT_IS_LON) gmt_lon_range_adjust (GMT->current.io.geo.range, &val);
		if (GMT->current.io.fmt[GMT_OUT][i].skip < 0) gmtio_x_write (GMT, fp, -GMT->current.io.fmt[GMT_OUT][i].skip);	/* Pre-fill */
		k += GMT->current.io.fmt[GMT_OUT][i].io (GMT, fp, 1, &val);
		if (GMT->current.io.fmt[GMT_OUT][i].skip > 0) gmtio_x_write (GMT, fp, GMT->current.io.fmt[GMT_OUT][i].skip);	/* Post-fill */
	}
	return (0);
}

/*! . */
int gmt_ascii_output_no_text (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *ptr, char *txt) {
	uint64_t i, col, last, n_out;
	int e = 0, wn = 0;
	double val;
	gmt_M_unused (txt);

	if (gmt_skip_output (GMT, ptr, n)) return (-1);	/* Record was skipped via -s[a|r] */
	n_out = (GMT->common.o.select) ? GMT->common.o.n_cols : n;

	last = n_out - 1;				/* Last filed, need to output linefeed instead of delimiter */

	for (i = 0; i < n_out && e >= 0; i++) {		/* Keep writing all fields unless there is a read error (e == -1) */
		if (GMT->common.o.select)	/* Which data column to pick */
			col = GMT->current.io.col[GMT_OUT][i].col;
		else if (GMT->current.setting.io_lonlat_toggle[GMT_OUT] && i < 2)
			col = 1 - i;	/* Write lat/lon instead of lon/lat */
		else
			col = i;	/* Just goto next column */
		val = (col >= n) ? GMT->session.d_NaN : ptr[col];	/* If we request beyond length of array, return NaN */
		if (GMT->common.d.active[GMT_OUT] && gmt_M_is_dnan (val))	/* Write this value instead of NaNs */
			val = GMT->common.d.nan_proxy[GMT_OUT];

		e = gmt_ascii_output_col (GMT, fp, val, col);	/* Write one item without any separator at the end */

		if (i == last)					/* This is the last field, must add newline */
			putc ('\n', fp);
		else if (GMT->current.setting.io_col_separator[0])		/* Not last field, and a separator is required */
			fprintf (fp, "%s", GMT->current.setting.io_col_separator);

		wn += e;
	}
	return ((e < 0) ? -1 : 0);
}

GMT_LOCAL void gmtio_output_trailing_text (struct GMT_CTRL *GMT, FILE *fp, char *txt) {
	if (GMT->common.o.word) {	/* Must output a specific word from the trailing text only */
		char *word = NULL, *orig = strdup (txt), *trail = orig;
		uint64_t col = 0;
		while (col != GMT->common.o.w_col && (word = strsep (&trail, GMT_TOKEN_SEPARATORS)) != NULL) {
			if (*word != '\0')	/* Skip empty strings */
				col++;
		}
		if (word)	/* Only write word if not NULL */
			fprintf (fp, "%s", word);
		else
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Trailing text did not have %" PRIu64 " words - no trailing word written\n", GMT->common.o.w_col);
		fprintf (fp, "\n");
		gmt_M_str_free (orig);
	}
	else	/* Output the whole enchilada */
		fprintf (fp, "%s\n", txt);
}

/*! . */
int gmtlib_ascii_output_trailing_text (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *ptr, char *txt) {

	if (gmt_skip_output (GMT, ptr, n)) return (-1);	/* Record was skipped via -s[a|r] */

	gmtio_output_trailing_text (GMT, fp, txt);

	return (0);
}

/*! . */
GMT_LOCAL int gmtio_ascii_output_with_text (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *ptr, char *txt) {
	uint64_t i, col, n_out;
	int e = 0, wn = 0;
	double val;

	if (gmt_skip_output (GMT, ptr, n)) return (-1);	/* Record was skipped via -s[a|r] */
	n_out = (GMT->common.o.select) ? GMT->common.o.n_cols : n;

	for (i = 0; i < n_out && e >= 0; i++) {		/* Keep writing all fields unless there is a read error (e == -1) */
		if (GMT->common.o.select)	/* Which data column to pick */
			col = GMT->current.io.col[GMT_OUT][i].col;
		else if (GMT->current.setting.io_lonlat_toggle[GMT_OUT] && i < 2)
			col = 1 - i;	/* Write lat/lon instead of lon/lat */
		else
			col = i;	/* Just goto next column */
		val = (col >= n) ? GMT->session.d_NaN : ptr[col];	/* If we request beyond length of array, return NaN */
		if (GMT->common.d.active[GMT_OUT] && gmt_M_is_dnan (val))	/* Write this value instead of NaNs */
			val = GMT->common.d.nan_proxy[GMT_OUT];

		e = gmt_ascii_output_col (GMT, fp, val, col);	/* Write one item without any separator at the end */

		if (GMT->current.setting.io_col_separator[0])		/* Not last field, and a separator is required */
			fprintf (fp, "%s", GMT->current.setting.io_col_separator);

		wn += e;
	}
	gmtio_output_trailing_text (GMT, fp, txt);

	return ((e < 0) ? -1 : 0);
}

GMT_LOCAL int gmtio_ascii_output (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *ptr, char *txt) {
	/* First time we decide if we have floats only or a mix and finalize pointer settings */
	if (txt && GMT->current.io.trailing_text[GMT_OUT]) {
		if (n == 0 || (GMT->common.o.select && GMT->common.o.n_cols == 0))
			GMT->current.io.output = gmtlib_ascii_output_trailing_text;	/* Just print trailing text */
		else
			GMT->current.io.output = gmtio_ascii_output_with_text;	/* Have trailing text after numerical output */
		return GMT->current.io.output (GMT, fp, n, ptr, txt);
	}
	else {
		GMT->current.io.output = gmt_ascii_output_no_text;	/* Just numbers */
		return gmt_ascii_output_no_text (GMT, fp, n, ptr, txt);
	}
}

/*! . */
void gmtlib_reset_input (struct GMT_CTRL *GMT) {
	if (GMT->current.io.output == gmtio_ascii_output_with_text) GMT->current.io.output = gmtio_ascii_output;	/* Go back to being agnostic */
}

/*! . */
GMT_LOCAL void gmtio_format_geo_output (struct GMT_CTRL *GMT, bool is_lat, double geo, char *text) {
	int k, n_items, d, m, s, m_sec, h_pos = 0;
	bool minus;
	char hemi[3] = {""}, *f = NULL;
	static char *suffix[2][2] = {{"W", "E"}, {"S", "N"}};	/* Just for decimal degrees when no_sign is true */

	if (is_lat) {	/* Column is supposedly latitudes */
		if (fabs (geo) > 90.0) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Column selected for latitude-formatting has values that exceed +/- 90; set to NaN\n");
			sprintf (text, "NaN");
			return;
		}
	}
	else gmt_lon_range_adjust (GMT->current.io.geo.range, &geo);	/* Adjust longitudes */
	if (GMT->current.io.geo.decimal) {	/* Easy */
		f = (GMT->current.io.o_format[is_lat]) ? GMT->current.io.o_format[is_lat] : GMT->current.setting.format_float_out;
		if (GMT->current.io.geo.no_sign) {
			k = (geo < 0.0) ? 0 : 1;
			sprintf (text, f, fabs(geo));
			strcat (text, suffix[is_lat][k]);
		}
		else
			sprintf (text, f, geo);
		return;
	}

	if (GMT->current.io.geo.wesn) {	/* Trailing WESN */
		if (GMT->current.io.geo.wesn == 2) hemi[h_pos++] = ' ';	/* Want space between numbers and hemisphere letter */
		if (is_lat)
			hemi[h_pos] = (gmt_M_is_zero (geo)) ? 0 : ((geo < 0.0) ? 'S' : 'N');
		else
			hemi[h_pos] = (gmt_M_is_zero (geo) || doubleAlmostEqual (geo, 180.0)) ? 0 : ((geo < 0.0) ? 'W' : 'E');
		geo = fabs (geo);
		if (hemi[h_pos] == 0) hemi[0] = 0;
	}

	for (k = n_items = 0; k < 3; k++)
		if (GMT->current.io.geo.order[k] >= 0) n_items++;	/* How many of d, m, and s are requested as integers */
	minus = gmtlib_geo_to_dms (geo, n_items, GMT->current.io.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
	if (minus) text[0] = '-';	/* Must manually insert leading minus sign when degree == 0 */
	if (GMT->current.io.geo.n_sec_decimals) {		/* Wanted fraction printed */
		if (n_items == 3)
			sprintf (&text[minus], GMT->current.io.geo.y_format, d, m, s, m_sec, hemi);
		else if (n_items == 2)
			sprintf (&text[minus], GMT->current.io.geo.y_format, d, m, m_sec, hemi);
		else
			sprintf (&text[minus], GMT->current.io.geo.y_format, d, m_sec, hemi);
	}
	else if (n_items == 3)
		sprintf (&text[minus], GMT->current.io.geo.y_format, d, m, s, hemi);
	else if (n_items == 2)
		sprintf (&text[minus], GMT->current.io.geo.y_format, d, m, hemi);
	else
		sprintf (&text[minus], GMT->current.io.geo.y_format, d, hemi);
}

/* Various functions to support {grd2xyz,xyz2grd}_func.c */

/* NOTE: In the following we check GMT->current.io.col_type[GMT_IN][2] and GMT->current.io.col_type[GMT_OUT][2] for formatting help for the first column.
 * We use column 3 ([2] or GMT_Z) instead of the first ([0]) since we really are dealing with the z in z (x,y) here
 * and the x,y are implicit from the -R -I arguments.
 */

/*! . */
GMT_LOCAL int gmtio_A_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* Can read one or more items from input records. Limitation is
	 * that they must be floating point values (no dates or ddd:mm:ss) */
	uint64_t i;
 	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		if (fscanf (fp, "%lg", &d[i]) <= 0)		/* Read was unsuccessful */
			return (GMT_DATA_READ_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_a_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* Only reads one item regardless of *n */
	char line[GMT_LEN64] = {""}, *p;
	gmt_M_unused(GMT); gmt_M_unused(n);
	if (!fgets (line, GMT_LEN64, fp)) {
		/* Read was unsuccessful */
		GMT->current.io.status = GMT_IO_EOF;
		return (GMT_DATA_READ_ERROR);
	}
	/* Find end of string */
	p = line;
	while (*p)
		++p;
	/* Remove trailing whitespace */
	while ((--p != line) && strchr (" \t,\r\n", (int)*p));
	*(p + 1) = '\0';
	/* Convert whatever it is to double */
	gmt_scanf (GMT, line, gmt_M_type (GMT, GMT_IN, GMT_Z), d);
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_c_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read int8_t aka char */
	uint64_t i;
	int8_t s;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&s, sizeof (int8_t), 1U, fp)) != 1) {
			/* Read was unsuccessful */
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_u_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read uint8_t aka unsigned char */
	uint64_t i;
	uint8_t u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint8_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) u;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_h_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read int16_t */
	uint64_t i;
	int16_t s;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&s, sizeof (int16_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_h_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped int16_t */
	uint64_t i;
	uint16_t u;
	size_t k;
	int16_t *s = (int16_t *)&u;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint16_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		u = bswap16 (u);
		d[i] = (double) *s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_H_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read uint16_t */
	uint64_t i;
	uint16_t u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint16_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) u;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_H_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped uint16_t */
	uint64_t i;
	uint16_t u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint16_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) bswap16 (u);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_i_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read int32_t */
	uint64_t i;
	int32_t s;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&s, sizeof (int32_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_i_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped int32_t */
	uint64_t i;
	uint32_t u;
	int32_t *s = (int32_t *)&u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint32_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		u = bswap32 (u);
		d[i] = (double) *s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_I_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read uint32_t */
	uint64_t i;
	uint32_t u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint32_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) u;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_I_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped uint32_t */
	uint64_t i;
	uint32_t u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint32_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) bswap32 (u);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_l_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read int64_t */
	uint64_t i;
	int64_t s;
	size_t k;

	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&s, sizeof (int64_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_l_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped int64_t */
	uint64_t i;
	uint64_t u;
	int64_t *s = (int64_t *)&u;
	size_t k;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint64_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		u = bswap64(u);
		d[i] = (double) *s;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_L_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read uint64_t */
	uint64_t i;
	uint64_t u;
	size_t k;

	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint64_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) u;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_L_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped uint64_t */
	uint64_t i;
	uint64_t u;
	size_t k;

	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u, sizeof (uint64_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) bswap64(u);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_f_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read float */
	uint64_t i;
	size_t k;
	float f;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&f, sizeof (float), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		d[i] = (double) f;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_f_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped float */
	size_t k;
	uint64_t i;
	union {
		float f;
		uint32_t bits;
	} u;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u.bits, sizeof (uint32_t), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		u.bits = bswap32 (u.bits);
		d[i] = (double) u.f;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_d_read (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read double */
	size_t k;
	uint64_t i;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&d[i], sizeof (double), 1U, fp)) != 1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_d_read_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* read byteswapped double */
	size_t k;
	uint64_t i;
	union {
		double d;
		uint64_t bits;
	} u;
	for (i = 0; i < n; ++i) {
		if ((k = gmt_M_fread (&u.bits, sizeof (uint64_t), 1U, fp)) !=1) {
			GMT->current.io.status = GMT_IO_EOF;
			return (GMT_DATA_READ_ERROR);
		}
		u.bits = bswap64 (u.bits);
		d[i] = u.d;
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_a_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write ASCII */
	uint64_t i;
	for (i = 0; i < (n - 1); ++i) {
		gmt_ascii_output_col (GMT, fp, d[i], GMT_Z);
		fprintf (fp, "\t");
	}
	/* last col */
	gmt_ascii_output_col (GMT, fp, d[i], GMT_Z);
	fprintf (fp, "\n");
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_c_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write int8_t aka char */
	uint64_t i;
	int8_t s;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		s = (int8_t) d[i];
		if (gmt_M_fwrite (&s, sizeof (int8_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_u_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write uint8_t aka unsigned char */
	uint64_t i;
	uint8_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = (uint8_t) d[i];
		if (gmt_M_fwrite (&u, sizeof (uint8_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_h_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write int16_t */
	uint64_t i;
	int16_t s;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		s = (int16_t) d[i];
		if (gmt_M_fwrite (&s, sizeof (int16_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_h_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped int16_t */
	uint64_t i;
	uint16_t u;
	int16_t *s = (int16_t *)&u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		*s = (int16_t) d[i];
		u = bswap16 (u);
		if (gmt_M_fwrite (&u, sizeof (uint16_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_H_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write uint16_t */
	uint64_t i;
	uint16_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = (uint16_t) d[i];
		if (gmt_M_fwrite (&u, sizeof (uint16_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_H_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped uint16_t */
	uint64_t i;
	uint16_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = bswap16 ((uint16_t) d[i]);
		if (gmt_M_fwrite (&u, sizeof (uint16_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_i_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write int32_t */
	uint64_t i;
	int32_t s;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		s = (int32_t) d[i];
		if (gmt_M_fwrite (&s, sizeof (int32_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_i_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped int32_t */
	uint64_t i;
	uint32_t u;
	int32_t *s = (int32_t *)&u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		*s = (int32_t) d[i];
		u = bswap32 (u);
		if (gmt_M_fwrite (&u, sizeof (uint32_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_I_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write uint32_t */
	uint64_t i;
	uint32_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = (uint32_t) d[i];
		if (gmt_M_fwrite (&u, sizeof (uint32_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_I_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped uint32_t */
	uint64_t i;
	uint32_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = bswap32 ((uint32_t) d[i]);
		if (gmt_M_fwrite (&u, sizeof (uint32_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_l_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write int64_t */
	uint64_t i;
	int64_t s;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		s = (int64_t) d[i];
		if (gmt_M_fwrite (&s, sizeof (int64_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_l_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped int64_t */
	uint64_t i;
	uint64_t u;
	int64_t *s = (int64_t *)&u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		*s = (int64_t) d[i];
		u = bswap64(u);
		if (gmt_M_fwrite (&u, sizeof (uint64_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_L_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write uint64_t */
	uint64_t i;
	uint64_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = (uint64_t) d[i];
		if (gmt_M_fwrite (&u, sizeof (int64_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_L_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped uint64_t */
	uint64_t i;
	uint64_t u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u = bswap64((uint64_t) d[i]);
		if (gmt_M_fwrite (&u, sizeof (uint64_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_f_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write float */
	uint64_t i;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		float f = (float) d[i];
		if (gmt_M_fwrite (&f, sizeof (float), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_f_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped float */
	uint64_t i;
	union {
		float f;
		uint32_t bits;
	} u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u.f = (float) d[i];
		u.bits = bswap32(u.bits);
		if (gmt_M_fwrite (&u.bits, sizeof (uint32_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_d_write (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write double */
	gmt_M_unused(GMT);
	if (gmt_M_fwrite (d, sizeof (double), n, fp) != n)
		return (GMT_DATA_WRITE_ERROR);
	return (GMT_OK);
}

/*! . */
GMT_LOCAL int gmtio_d_write_swab (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *d) {
	/* write byteswapped double */
	uint64_t i;
	union {
		double d;
		uint64_t bits;
	} u;
	gmt_M_unused(GMT);
	for (i = 0; i < n; ++i) {
		u.d = d[i];
		u.bits = bswap64 (u.bits);
		if (gmt_M_fwrite (&u.bits, sizeof (uint64_t), 1U, fp) != 1U)
			return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_OK);
}

/*! . */
GMT_LOCAL bool gmtio_get_ymdj_order (struct GMT_CTRL *GMT, char *text, struct GMT_DATE_IO *S) {
	/* Reads a YYYY-MM-DD or YYYYMMDD-like string and determines order.
	 * order[0] is the order of the year, [1] is month, etc.
	 * Items not encountered are left as -1.
	 */

	unsigned int i, j, order, n_y, n_m, n_d, n_j, n_w, error = 0;
	int k, last, n_delim;
	bool watch = false;
	const size_t s_length = strlen(text);

	gmt_M_memset (S, 1, struct GMT_DATE_IO);
	for (i = 0; i < 4; i++) S->item_order[i] = S->item_pos[i] = -1;	/* Meaning not encountered yet */

	n_y = n_m = n_d = n_j = n_w = n_delim = 0;

	i = 0;
	if (text[i] == '-') {	/* Leading hyphen means use %d and not %x.xd for integer formats */
		S->compact = true;
		i++;
	}
	for (order = 0; i < s_length; i++) {
		switch (text[i]) {
			case 'y':	/* Year */
				if (S->item_pos[0] < 0)		/* First time we encounter a y */
					S->item_pos[0] = order++;
				else if (text[i-1] != 'y')	/* Done it before, previous char must be y */
					error++;
				n_y++;
				break;
			case 'm':	/* Month */
				if (S->item_pos[1] < 0)		/* First time we encounter a m */
					S->item_pos[1] = order++;
				else if (text[i-1] != 'm')	/* Done it before, previous char must be m */
					error++;
				n_m++;
				break;
			case 'o':	/* Month name (plot output only) */
				if (S->item_pos[1] < 0)		/* First time we encounter an o */
					S->item_pos[1] = order++;
				else				/* Done it before is error here */
					error++;
				S->mw_text = true;
				n_m = 2;	/* This just flags it as properly reading 'mm' to past check below */
				break;

			case 'W':	/* ISO Week flag */
				S->iso_calendar = true;
				break;
			case 'w':	/* ISO Week */
				if (S->item_pos[1] < 0) {		/* First time we encounter a w */
					S->item_pos[1] = order++;
					if (text[i-1] != 'W') error++;	/* Must have the format W just before */
				}
				else if (text[i-1] != 'w')	/* Done it before, previous char must be w */
					error++;
				n_w++;
				break;
			case 'u':	/* ISO Week name ("Week 04") (plot output only) */
				S->iso_calendar = true;
				if (S->item_pos[1] < 0) {		/* First time we encounter a u */
					S->item_pos[1] = order++;
				}
				else 				/* Done it before is an error */
					error++;
				S->mw_text = true;
				n_w = 2;
				break;
			case 'd':	/* Day of month */
				if (S->item_pos[2] < 0)		/* First time we encounter a d */
					S->item_pos[2] = order++;
				else if (text[i-1] != 'd')	/* Done it before, previous char must be d */
					error++;
				n_d++;
				break;
			case 'j':	/* Day of year  */
				S->day_of_year = true;
				if (S->item_pos[3] < 0)		/* First time we encounter a j */
					S->item_pos[3] = order++;
				else if (text[i-1] != 'j')	/* Done it before, previous char must be j */
					error++;
				n_j++;
				break;
			default:	/* Delimiter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimiter[n_delim++][0] = text[i];
				break;
		}
	}

	/* Then get the actual order by inverting table */

	for (k = 0; k < 4; k++) for (j = 0; j < 4; j++) if (S->item_pos[j] == k) S->item_order[k] = j;
	S->Y2K_year = (n_y == 2);		/* Must supply the century when reading and take it out when writing */
	S->truncated_cal_is_ok = true;		/* May change in the next loop */
	for (i = 1, last = S->item_order[0]; S->truncated_cal_is_ok && i < 4; i++) {
		if (S->item_order[i] == -1) continue;
		if (S->item_order[i] < last) S->truncated_cal_is_ok = false;
		last = S->item_order[i];
	}
	last = (n_y > 0) + (n_m > 0) + (n_w > 0) + (n_d > 0) + (n_j > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);				/* If there are delimiters, must be one less than the items */
	if (S->iso_calendar) {		/* Check if ISO Week format is ok */
		error += (!S->truncated_cal_is_ok);
		error += (n_w != 2);			/* Gotta have 2 ww */
		error += !(n_d == 1 || n_d == 0);	/* Gotta have 1 d if present */
	}
	else {				/* Check if Gregorian format is ok */
		error += (n_w != 0);	/* Should have no w */
		error += (n_j == 3 && !(n_m == 0 && n_d == 0));	/* day of year should have m = d = 0 */
		error += (n_j == 0 && !((n_m == 2 || n_m == 0) && (n_d == 2 || n_d == 0) && n_d <= n_m));	/* mm/dd must have jjj = 0 and m >= d and m,d 0 or 2 */
		if (S->mw_text && S->item_order[last-1] == 1) watch = true;	/* Input ends with monthname */
	}
	if (error) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unacceptable date template %s\n", text);
		GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
	}
	return (watch);
}

/*! . */
GMT_LOCAL int gmtio_get_hms_order (struct GMT_CTRL *GMT, char *text, struct GMT_CLOCK_IO *S) {
	/* Reads a HH:MM:SS or HHMMSS-like string and determines order.
	 * hms_order[0] is the order of the hour, [1] is min, etc.
	 * Items not encountered are left as -1.
	 */

	int i, j, order, n_delim, sequence[3], last, n_h, n_m, n_s, n_x, n_dec, error = 0;
	bool big_to_small;
	char *p = NULL;
	ptrdiff_t off;

	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */
	sequence[0] = sequence[1] = sequence[2] = -1;

	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	n_h = n_m = n_s = n_x = n_dec = n_delim = 0;

	/* Determine if we do 12-hour clock (and what form of am/pm suffix) or 24-hour clock */

	if ((p = strstr (text, "am"))) {	/* Want 12 hour clock with am/pm */
		S->twelve_hr_clock = true;
		strcpy (S->ampm_suffix[0], "am");
		strcpy (S->ampm_suffix[1], "pm");
		off = p - text;
	}
	else if ((p = strstr (text, "AM"))) {	/* Want 12 hour clock with AM/PM */
		S->twelve_hr_clock = true;
		strcpy (S->ampm_suffix[0], "AM");
		strcpy (S->ampm_suffix[1], "PM");
		off = p - text;
	}
	else if ((p = strstr (text, "a.m."))) {	/* Want 12 hour clock with a.m./p.m. */
		S->twelve_hr_clock = true;
		strcpy (S->ampm_suffix[0], "a.m.");
		strcpy (S->ampm_suffix[1], "p.m.");
		off = p - text;
	}
	else if ((p = strstr (text, "A.M."))) {	/* Want 12 hour clock with A.M./P.M. */
		S->twelve_hr_clock = true;
		strcpy (S->ampm_suffix[0], "A.M.");
		strcpy (S->ampm_suffix[1], "P.M.");
		off = p - text;
	}
	else
		off = strlen (text);

	i = 0;
	if (text[i] == '-') {	/* Leading hyphen means use %d and not %x.xd for integer formats */
		S->compact = true;
		i++;
	}
	for (order = 0; i < off; i++) {
		switch (text[i]) {
			case 'h':	/* Hour */
				if (S->order[0] < 0)		/* First time we encountered a h */
					S->order[0] = order++;
				else if (text[i-1] != 'h')	/* Must follow a previous h */
					error++;
				n_h++;
				break;
			case 'm':	/* Minute */
				if (S->order[1] < 0)		/* First time we encountered a m */
					S->order[1] = order++;
				else if (text[i-1] != 'm')	/* Must follow a previous m */
					error++;
				n_m++;
				break;
			case 's':	/* Seconds */
				if (S->order[2] < 0)		/* First time we encountered a s */
					S->order[2] = order++;
				else if (text[i-1] != 's')	/* Must follow a previous s */
					error++;
				n_s++;
				break;
			case '.':	/* Decimal point for seconds? */
				if (text[i+1] == 'x')
					n_dec++;
				else {	/* Must be a delimiter */
					if (n_delim == 2)
						error++;
					else
						S->delimiter[n_delim++][0] = text[i];
				}
				break;
			case 'x':	/* Fraction of seconds */
				if (n_x > 0 && text[i-1] != 'x')	/* Must follow a previous x */
					error++;
				n_x++;
				break;
			default:	/* Delimiter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimiter[n_delim++][0] = text[i];
				break;
		}
	}

	/* Then get the actual order by inverting table */

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			if (S->order[j] == i) sequence[i] = j;
	for (i = 0; i < 3; i++) S->order[i] = sequence[i];
	big_to_small = true;		/* May change in the next loop */
	for (i = 1, last = S->order[0]; big_to_small && i < 3; i++) {
		if (S->order[i] == -1) continue;
		if (S->order[i] < last) big_to_small = false;
		last = S->order[i];
	}
	if (!big_to_small) error++;
	last = (n_h > 0) + (n_m > 0) + (n_s > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);	/* If there are delimiters, must be one less than the items */
	error += (!(n_h == 0 || n_h == 2) || !(n_m == 0 || n_m == 2) || !(n_s == 0 || n_s == 2));	/* h, m, s are all either 2 or 0 */
	error += (n_s > n_m || n_m > n_h);		/* Cannot have secs without m etc */
	error += (n_x && n_dec != 1);			/* .xxx is the proper form */
	error += (n_x == 0 && n_dec);			/* Period by itself and not delimiter? */
	error += (n_dec > 1);				/* Only one period with xxx */
	S->n_sec_decimals = n_x;
	S->f_sec_to_int = rint (pow (10.0, (double)S->n_sec_decimals));			/* To scale fractional seconds to an integer form */
	if (error) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unacceptable clock template %s\n", text);
		GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
	}
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtio_get_dms_order (struct GMT_CTRL *GMT, char *text, struct GMT_GEO_IO *S) {
	/* Reads a ddd:mm:ss-like string and determines order.
	 * order[0] is the order of the degree, [1] is minutes, etc.
	 * Order is checked since we only allow d, m, s in that order.
	 * Items not encountered are left as -1.
	 */

	unsigned int j, n_d, n_m, n_s, n_x, n_dec, n_period, order, error = 0;
	int sequence[3], last, i_signed, n_delim;
	size_t i1, i;
	bool big_to_small;

	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */

	n_d = n_m = n_s = n_x = n_dec = n_delim = n_period = 0;
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	sequence[0] = sequence[1] = sequence[2] = -1;

	S->range = GMT_IS_M180_TO_P180_RANGE;			/* -180/+180 range, may be overwritten below by + or - */
	S->decimal = S->no_sign = false;
	S->wesn = 0;

	i1 = strlen (text) - 1;
	for (i = order = 0; i <= i1; i++) {
		switch (text[i]) {
			case '+':	/* Want [0-360 range [Default] */
				S->range = GMT_IS_0_TO_P360_RANGE;
				if (i != 0) error++;		/* Only valid as first flag */
				break;
			case '-':	/* Want [-360-0] range [i.e., western longitudes] */
				S->range = GMT_IS_M360_TO_0_RANGE;
				if (i != 0) error++;		/* Only valid as first flag */
				break;
			case 'D':	/* Want to use decimal degrees using D_FORMAT [Default] */
				S->decimal = true;
				if (i > 1) error++;		/* Only valid as first or second flag */
				break;
			case 'F':	/* Want to use WESN to encode sign */
				S->wesn = 1;
				if (i != i1 || S->no_sign) error++;		/* Only valid as last flag */
				break;
			case 'G':	/* Want to use WESN to encode sign but have leading space */
				S->wesn = 2;
				if (i != i1 || S->no_sign) error++;		/* Only valid as last flag */
				break;
			case 'A':	/* Want no sign in plot string */
				S->no_sign = true;
				if (i != i1 || S->wesn) error++;		/* Only valid as last flag */
				break;
			case 'd':	/* Degree */
				if (S->order[0] < 0)		/* First time we encounter a d */
					S->order[0] = order++;
				else if (text[i-1] != 'd')	/* Done it before, previous char must be y */
					error++;
				n_d++;
				break;
			case 'm':	/* Minute */
				if (S->order[1] < 0)		/* First time we encounter a m */
					S->order[1] = order++;
				else if (text[i-1] != 'm')	/* Done it before, previous char must be m */
					error++;
				n_m++;
				break;
			case 's':	/* Seconds */
				if (S->order[2] < 0) {		/* First time we encounter a s */
					S->order[2] = order++;
				}
				else if (text[i-1] != 's')	/* Done it before, previous char must be s */
					error++;
				n_s++;
				break;
			case '.':	/* Decimal point for seconds? */
				n_period++;
				if (text[i+1] == 'x')
					n_dec++;
				else {	/* Must be a delimiter */
					if (n_delim == 2)
						error++;
					else
						S->delimiter[n_delim++][0] = text[i];
				}
				break;
			case 'x':	/* Fraction of seconds */
				if (n_x > 0 && text[i-1] != 'x')	/* Must follow a previous x */
					error++;
				n_x++;
				break;
			default:	/* Delimiter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimiter[n_delim++][0] = text[i];
				break;
		}
	}

	if (S->decimal) return (GMT_NOERROR);	/* Easy formatting choice */

	/* Then get the actual order by inverting table */

	for (i_signed = 0; i_signed < 3; i_signed++)
		for (j = 0; j < 3; j++)
			if (S->order[j] == i_signed) sequence[i_signed] = j;
	for (i = 0; i < 3; i++) S->order[i] = sequence[i];
	big_to_small = true;		/* May change in the next loop */
	for (i = 1, last = S->order[0]; big_to_small && i < 3; i++) {
		if (S->order[i] == -1) continue;
		if (S->order[i] < last) big_to_small = false;
		last = S->order[i];
	}
	if (!big_to_small) error++;
	last = (n_d > 0) + (n_m > 0) + (n_s > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);	/* If there are delimiters, must be one less than the items */
	error += (!(n_d == 0 || n_d == 3) || !(n_m == 0 || n_m == 2) || !(n_s == 0 || n_s == 2));	/* d, m, s are all either 2(3) or 0 */
	error += (n_s > n_m || n_m > n_d);		/* Cannot have secs without m etc */
	error += (n_x && n_dec != 1);			/* .xxx is the proper form */
	error += (n_x == 0 && n_dec);			/* Period by itself and not delimiter? */
	error += (n_dec > 1);				/* Only one period with xxx */
	S->n_sec_decimals = n_x;
	S->f_sec_to_int = rint (pow (10.0, (double)S->n_sec_decimals));			/* To scale fractional seconds to an integer form */
	if (error) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unacceptable dmmss template %s\n", text);
		GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
	}
	else if (n_period > 1)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Multiple periods in dmmss template %s is likely to lead to confusion\n", text);
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL int gmtio_scanf_clock (struct GMT_CTRL *GMT, char *s, double *val) {
	/* On failure, return -1.  On success, set val and return 0.

	Looks for apAP, but doesn't discover a failure if called with "11:13:15 Hello, Walter",
	because it will find an a.

	Doesn't check whether use of a or p matches stated intent to use twelve_hour_clock.

	ISO standard allows 24:00:00, so 86400 is not too big.
	If the day of this clock might be a day with a leap second, (this routine doesn't know that)
	then we should also allow 86401.  A value exceeding 86401 is an error.
	*/

	int k, hh, mm, add_noon = 0, hh_limit = 24;	/* ISO std allows 24:00:00  */
	double ss, x;
	char *p = NULL;

	if ( (p = strpbrk (s, "apAP") ) ) {
		switch (p[0]) {
			case 'a':
			case 'A':
				add_noon = 0;
				hh_limit = 12;
				break;
			case 'p':
			case 'P':
				add_noon = 43200;
				hh_limit = 12;
				break;
			default:
				return (-1);
				break;
		}
	}

	k = sscanf (s, GMT->current.io.clock_input.format, &hh, &mm, &ss);
	if (k == 0) return (-1);
	if (hh < 0 || hh > hh_limit) return (-1);

	x = (double)(add_noon + 3600*hh);
	if (k > 1) {
		if (mm < 0 || mm > 59) return (-1);
		x += 60*mm;
	}
	if (k > 2) {
		x += ss;
		if (x > 86401.0) return (-1);
	}
	*val = x;
	return (0);
}

/*! . */
GMT_LOCAL int gmtio_scanf_ISO_calendar (struct GMT_CTRL *GMT, char *s, int64_t *rd) {

	/* On failure, return -1.  On success, set rd and return 0.
	Assumes that year, week of year, day of week appear in that
	order only, and that the format string can handle the W.
	Assumes also that it is always OK to fill in missing bits.  */

	int k, n, ival[3];

	if ((n = sscanf (s, GMT->current.io.date_input.format, &ival[0], &ival[1], &ival[2])) <= 0) return (-1);

	/* Handle possible missing bits */
	for (k = n; k < 3; k++) ival[k] = 1;

	if (ival[1] < 1 || ival[1] > 53) return (-1);
	if (ival[2] < 1 || ival[2] > 7) return (-1);
	if (GMT->current.io.date_input.Y2K_year) {
		if (ival[0] < 0 || ival[0] > 99) return (-1);
		ival[0] = gmtlib_y2_to_y4_yearfix (GMT, ival[0]);
	}
	*rd = gmtlib_rd_from_iywd (GMT, ival[0], ival[1], ival[2]);
	return (0);
}

/*! . */
GMT_LOCAL int gmtio_scanf_g_calendar (struct GMT_CTRL *GMT, char *s, int64_t *rd) {
	/* Return -1 on failure.  Set rd and return 0 on success.

	For gregorian calendars.  */

	int i, k, ival[4];
	char month[16];

	if (GMT->current.io.date_input.day_of_year) {
		/* Calendar uses year and day of year format.  */
		if ( (k = sscanf (s, GMT->current.io.date_input.format,
			&ival[GMT->current.io.date_input.item_order[0]],
			&ival[GMT->current.io.date_input.item_order[1]]) ) == 0) return (-1);
		if (k < 2) {
			if (!GMT->current.io.date_input.truncated_cal_is_ok) return (-1);
			ival[1] = 1;	/* Set first day of year  */
		}
		if (GMT->current.io.date_input.Y2K_year) {
			if (ival[0] < 0 || ival[0] > 99) return (-1);
			ival[0] = gmtlib_y2_to_y4_yearfix (GMT, ival[0]);
		}
		k = (gmtlib_is_gleap (ival[0])) ? 366 : 365;
		if (ival[3] < 1 || ival[3] > k) return (-1);
		*rd = gmt_rd_from_gymd (GMT, ival[0], 1, 1) + ival[3] - 1;
		return (0);
	}

	/* Get here when calendar type has months and days of months.  */

	if (GMT->current.io.date_input.mw_text) {	/* Have month name abbreviation in data format */
		switch (GMT->current.io.date_input.item_pos[1]) {	/* Order of month in data string */
			case 0:	/* e.g., JAN-24-1987 or JAN-1987-24 */
				k = sscanf (s, GMT->current.io.date_input.format, month, &ival[GMT->current.io.date_input.item_order[1]],
				            &ival[GMT->current.io.date_input.item_order[2]]);
				break;
			case 1:	/* e.g., 24-JAN-1987 or 1987-JAN-24 */
				k = sscanf (s, GMT->current.io.date_input.format, &ival[GMT->current.io.date_input.item_order[0]], month,
				            &ival[GMT->current.io.date_input.item_order[2]]);
				break;
			case 2:	/* e.g., JAN-24-1987 ? */
				k = sscanf (s, GMT->current.io.date_input.format, &ival[GMT->current.io.date_input.item_order[0]],
				            &ival[GMT->current.io.date_input.item_order[1]], month);
				break;
			default:
				return (-1);
				break;
		}
		gmt_str_toupper (month);
		for (i = ival[1] = 0; i < 12 && ival[1] == 0; i++) {
			if (!strcmp (month, GMT->current.language.month_name[3][i])) ival[1] = i + 1;
		}
		if (ival[1] == 0) return (-1);	/* No match for month name */
	}
	else if ((k = sscanf (s, GMT->current.io.date_input.format, &ival[GMT->current.io.date_input.item_order[0]],
	                      &ival[GMT->current.io.date_input.item_order[1]], &ival[GMT->current.io.date_input.item_order[2]])) == 0)
		return (-1);
	if (k < 3) {
		if (GMT->current.io.date_input.truncated_cal_is_ok) {
			ival[2] = 1;	/* Set first day of month  */
			if (k == 1) ival[1] = 1;	/* Set first month of year */
		}
		else
			return (-1);
	}
	if (GMT->current.io.date_input.Y2K_year) {
		if (ival[0] < 0 || ival[0] > 99) return (-1);
		ival[0] = gmtlib_y2_to_y4_yearfix (GMT, ival[0]);
	}

	if (gmtlib_g_ymd_is_bad (ival[0], ival[1], ival[2]) ) return (-1);

	*rd = gmt_rd_from_gymd (GMT, ival[0], ival[1], ival[2]);
	return (0);
}

/*! . */
GMT_LOCAL int gmtio_scanf_calendar (struct GMT_CTRL *GMT, char *s, int64_t *rd) {
	/* On failure, return -1.  On success, set rd and return 0 */
	if (GMT->current.io.date_input.iso_calendar) return (gmtio_scanf_ISO_calendar (GMT, s, rd));
	return (gmtio_scanf_g_calendar (GMT, s, rd));
}

/*! . */
GMT_LOCAL int gmtio_scanf_geo (char *s, double *val) {
	/* Try to read a character string token stored in s, knowing that it should be a geographical variable.
	If successful, stores value in val and returns one of GMT_IS_FLOAT, GMT_IS_GEO, GMT_IS_LAT, GMT_IS_LON,
	whichever can be determined from the format of s.
	If unsuccessful, does not store anything in val and returns GMT_IS_NAN.
	This should have essentially the same functionality as the GMT3.4 gmt_scanf, except that the expectation
	is now used and returned, and this also permits a double precision format in the minutes or seconds,
	and does more error checking.  However, this is not optimized for speed (yet).  WHFS, 16 Aug 2001

	Note: Mismatch handling (e.g. this routine finds a lon but calling routine expected a lat) is not
	done here.
	*/

	int retval = GMT_IS_FLOAT, id, im;
	bool negate = false;
	unsigned int ncolons;
	size_t k;
	char scopy[GMT_LEN64] = {""}, suffix, *p = NULL, *p2 = NULL;
	double dd, dm, ds;

	k = strlen (s);
	if (k == 0 || (int)s[k - 1] < 0) return (GMT_IS_NAN);		/* On Win and debug mode isdigit assert crashes with negative argument */
	if (!(isdigit ((int)s[k-1]))) {
		suffix = s[k-1];
		switch (suffix) {
			case 'W': case 'w':
				negate = true;
				retval = GMT_IS_LON;
				break;
			case 'E': case 'e':
				retval = GMT_IS_LON;
				break;
			case 'S': case 's':
				negate = true;
				retval = GMT_IS_LAT;
				break;
			case 'N': case 'n':
				retval = GMT_IS_LAT;
				break;
			case 'G': case 'g': case 'D': case 'd':
				retval = GMT_IS_GEO;
				break;
			case '.':	/* Decimal point without decimals, e.g., 123. */
				break;
			default:
				return (GMT_IS_NAN);
				break;
		}
		k--;
	}
	if (k >= GMT_LEN64) return (GMT_IS_NAN);
	strncpy (scopy, s, k);				/* Copy all but the suffix  */
	scopy[k] = 0;
	ncolons = 0;
	if ((p = strpbrk (scopy, "dD"))) {
		/* We found a D or d.  */
		if (strlen (p) == 1 || (strpbrk (&p[1], "dD:") ) ){
			/* It is at the end, or followed by a colon or another d or D.  */
			return (GMT_IS_NAN);
		}
		/* Map it to an e, permitting FORTRAN Double Precision formats.  */
		p[0] = 'e';
	}
	p = scopy;
	while ((p2 = strpbrk (p, ":"))) {
		if (strlen (p2) == 1) return (GMT_IS_NAN);	/* Shouldn't end with a colon  */
		ncolons++;
		if (ncolons > 2) return (GMT_IS_NAN);
		p = &p2[1];
	}

	if (ncolons && retval == GMT_IS_FLOAT) retval = GMT_IS_GEO;

	dd = 0.0;
	switch (ncolons) {
		case 0:
			if ((sscanf (scopy, "%lf", &dd)) != 1) return (GMT_IS_NAN);
			break;
		case 1:
			if ((sscanf (scopy, "%d:%lf", &id, &dm)) != 2) return (GMT_IS_NAN);
			dd = dm * GMT_MIN2DEG;
			if (id < 0)	/* Negative degrees present, subtract the fractional part */
				dd = id - dd;
			else if (id > 0)	/* Positive degrees present, add the fractional part */
				dd = id + dd;
			else {			/* degree part is 0; check if a leading sign is present */
				if (scopy[0] == '-') dd = -dd;	/* Make fraction negative */
			}
			break;
		case 2:
			if ((sscanf (scopy, "%d:%d:%lf", &id, &im, &ds)) != 3) return (GMT_IS_NAN);
			dd = im * GMT_MIN2DEG + ds * GMT_SEC2DEG;
			if (id < 0)	/* Negative degrees present, subtract the fractional part */
				dd = id - dd;
			else if (id > 0)	/* Positive degrees present, add the fractional part */
				dd = id + dd;
			else {			/* degree part is 0; check if a leading sign is present */
				if (scopy[0] == '-') dd = -dd;	/* Make fraction negative */
			}
			break;
	}
	*val = (negate) ? -dd : dd;
	if (fabs (dd) > 90.0 && retval == GMT_IS_GEO) retval = GMT_IS_LON;
	return (retval);
}

/*! . */
int gmt_scanf_float (struct GMT_CTRL *GMT, char *s, double *val) {
	/* Try to decode a value from s and store
	in val.  s should not have any special format
	(neither geographical, with suffixes or
	separating colons, nor calendar nor clock).
	However, D and d are permitted to map to e
	if this would result in a success.  This
	allows Fortran Double Precision to be readable.

	On success, return GMT_IS_FLOAT and store val.
	On failure, return GMT_IS_NAN and do not touch val.
	*/

	char scopy[GMT_LEN64] = {""}, *p = NULL;
	double x;
	size_t j, k;

	if (strstr (s, "pi")) {	/* Got a number given via multiple/fraction of pi */
		/* Only allow parsing of [-|+][s]pi[f], with s and f are any number */
		GMT->current.plot.substitute_pi = true;	/* Used in formatting labels */
		k = 0;
		if (s[0] == '-') x = -1.0, k = 1;
		else if (s[0] == '+') x = 1.0, k = 1;
		else x = 1.0;
		if (isdigit (s[k])) {	/* Need an integer multiple of pi */
			double fval = atof (&s[k]);
			x *= fval;	/* Get multiples of pi */
			while (isdigit (s[k])) k++;
		}
		k += 2;	/* Now skip the pi part */
		if (s[k]) x /= atof (&s[k]);	/* Get pi fraction */
		*val = x * M_PI;	/* Scale up by pi */
		return (GMT_IS_FLOAT);
	}

	x = strtod (s, &p);
	if (p[0] == 0) {	/* Success (non-Fortran).  */
		*val = x;
		return (GMT_IS_FLOAT);
	}
	if (p[0] != 'D' && p[0] != 'd') return (GMT_IS_NAN);
	k = strlen (p);
	if (k == 1) return (GMT_IS_NAN);	/* A string ending in e would be invalid  */
	/* Make a copy of s in scopy, mapping the d or D to an e */
	j = strlen (s);
	if (j > GMT_LEN64) return (GMT_IS_NAN);
	j -= k;
	strncpy (scopy, s, j);
	scopy[j] = 'e';
	strncpy (&scopy[j+1], &p[1], GMT_LEN64-1);
	x = strtod (scopy, &p);
	if (p[0] != 0) return (GMT_IS_NAN);
	*val = x;
	return (GMT_IS_FLOAT);
}

/*! . */
GMT_LOCAL int gmtio_scanf_dim (struct GMT_CTRL *GMT, char *s, double *val) {
	/* Try to decode a value from s and store
	in val.  s is a regular float with optional
	unit info, e.g., 8.5i or 7.5c.  If a valid unit
	is found we convert the number to inch.
	We also skip any trailing modifiers like +<mods>, e.g.
	vector specifications like 0.5i+jc+b+s

	We return GMT_IS_FLOAT and pass val.
	*/

	if (isalpha ((int)s[0]) || (s[1] == 0 && (s[0] == '-' || s[0] == '+')))	/* Probably a symbol character; quietly return 0 */
		*val = 0.0;
	else {	/* Probably a dimension with optional unit.  First check if there are modifiers to ignore here */
		char *p = NULL;
		if ((p = strchr (s, '+')) && !isdigit (p[1])) { /* Found trailing +mod args [and not values like 123.33E+01c] */
			*p = 0;	/* Chop off modifier */
			*val = gmt_M_to_inch (GMT, s);	/* Get dimension */
			*p = '+';	/* Restore modifier */
		}
		else
			*val = gmt_M_to_inch (GMT, s);
	}
	return (GMT_IS_FLOAT);
}

/*! . */
int gmt_scanf_argtime (struct GMT_CTRL *GMT, char *s, double *t) {
	/* s is a string from a command-line argument.
	   The argument is known to refer to a time variable.  For example, the argument is
	   a token from -R<t_min>/<t_max>/a/b[/c/d].  However, we will permit it to be in EITHER
	   -- generic floating point format, in which case we interpret it as relative time
	      in user units since epoch;
	   OR
	   -- absolute time in a restricted format, which is to be converted to relative time.

	   The absolute format must be restricted because we cannot use '/' as a delimiter in an arg
	   string, but we might allow the user to use that in a data file (in GMT->current.setting.[in/out]put_date_format.
	   Therefore we cannot use the user's date format string here, and we hard-wire something here.

	   The relative format must be decodable by gmt_scanf_float().  It may optionally end in 't'
	   (which will be stripped off by this routine).

	   The absolute format must have a T.  If it has a clock string then it must be of the form
	   <complete_calstring>T<clockstring> or just T<clockstring>.  If it has no clockstring then
	   it must be of the form <partial or complete calstring>T.

	   A <clockstring> may be partial (e.g. hh or hh:mm) or complete (hh:mm:ss[.xxx]) but it must use
	   ':' for a delimiter and it must be readable with "%2d:%2d:%lf".
	   Also, it must be a 24 hour clock (00:00:00 to 23:59:59.xxx,
	   or 60.xxx on a leap second); no am/pm suffixes allowed.

	   A <calstring> must be of the form
	   [-]yyyy[-mm[-dd]]T readable after first '-' with "%4d-%2d-%2dT" (Gregorian year,month,day)
	   [-]yyyy[-jjj]T readable after first '-' with "%4d-%3dT" (Gregorian year, day-of-year)
	   yyyy[-Www[-d]]T (ISO week calendar)

	Upon failure, returns GMT_IS_NAN.  Upon success, sets t and returns GMT_IS_ABSTIME.
	We have it return either ABSTIME or RELTIME to indicate which one it thinks it decoded.
	This is inconsistent with the use of gmt_scanf which always returns ABSTIME, even when RELTIME is
	expected and ABSTIME conversion is done internal to the routine, as it is here.
	The reason for returning RELTIME instead is that the -R option needs
	to know which was decoded and hence which is expected as column input.
	*/

	int ival[3];
	bool negate_year = false, got_yd = false;
	int hh, mm;
	unsigned int j, k, dash;
	size_t i;
	double ss, x;
	char *pw = NULL, *pt = NULL;

	i = strlen (s) - 1;
	if (s[i] == 't') s[i] = '\0';
	if ( (pt = strchr (s, (int)'T') ) == NULL) {
		/* There is no T.  This must decode with gmt_scanf_float() or we die.  */
		if ((gmt_scanf_float (GMT, s, t)) == GMT_IS_NAN) return (GMT_IS_NAN);
		return (GMT_IS_RELTIME);
	}
	x = 0.0;	/* x will be the seconds since start of today.  */
	if (pt[1]) {	/* There is a string following the T:  Decode a clock */
		k = sscanf (&pt[1], "%2d:%2d:%lf", &hh, &mm, &ss);
		if (k == 0) return (GMT_IS_NAN);
		if (hh < 0 || hh >= 24) return (GMT_IS_NAN);
		x = GMT_HR2SEC_F * hh;
		if (k > 1) {
			if (mm < 0 || mm > 59) return (GMT_IS_NAN);
			x += GMT_MIN2SEC_F * mm;
		}
		if (k > 2) {
			if (ss < 0.0 || ss >= 61.0) return (GMT_IS_NAN);
			x += ss;
		}
	}

	k = 0;
	while (s[k] && s[k] == ' ') k++;
	if (s[k] == '-') negate_year = true;
	if (s[k] == 'T') {	/* There is no calendar.  Set day to today and use that */
		*t = gmt_rdc2dt (GMT, GMT->current.time.today_rata_die, x);
		return (GMT_IS_ABSTIME);
	}

	if (!(isdigit ((int)s[k]))) return (GMT_IS_NAN);	/* Bad format */

	if ((pw = strchr (s, (int)'W'))) {
		/* There is a W.  ISO calendar or junk.  */
		if (strlen (pw) <= strlen (pt)) return (GMT_IS_NAN);	/* The W is after the T.  Wrong format.  */
		if (negate_year) return (GMT_IS_NAN);			/* negative years not allowed in ISO calendar  */
		if ( (j = sscanf(&s[k], "%4d-W%2d-%1d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
		if (gmtlib_iso_ywd_is_bad (ival[0], ival[1], ival[2]) ) return (GMT_IS_NAN);
		*t = gmt_rdc2dt (GMT, gmtlib_rd_from_iywd (GMT, ival[0], ival[1], ival[2]), x);
		return (GMT_IS_ABSTIME);
	}

	for (i = (negate_year) ? 1 : 0; s[k+i] && s[k+i] != '-'; i++); /* Goes to first - between yyyy and jjj or yyyy and mm */
	dash = (unsigned int)i++;	/* Position of first character after the first dash (could be end of string if no dash) */
    if (s[dash] && (dash-negate_year) > 4) return (GMT_IS_NAN);  /* Clearly got some junk or perhaps not using - as delimiter */
	while (s[k+i] && !(s[k+i] == '-' || s[k+i] == 'T')) i++;	/* Goto the ending T character or get stuck on a second - */
	got_yd = ((i - dash - 1) == 3 && s[k+i] == 'T');		/* Must have a field of 3-characters between - and T to constitute a valid day-of-year format */

	if (got_yd) {	/* Gregorian yyyy-jjj calendar */
		if ((j = sscanf(&s[k], "%4d-%3d", &ival[0], &ival[1]) ) != 2)
			return (GMT_IS_NAN);
		ival[2] = 1;
	}
	else {	/* Gregorian yyyy-mm-dd calendar */
		if ((j = sscanf(&s[k], "%4d-%2d-%2d", &ival[0], &ival[1], &ival[2]) ) == 0)
			return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
	}
	if (negate_year) ival[0] = -ival[0];
	if (got_yd) {
		if (ival[1] < 1 || ival[1] > 366)
			return (GMT_IS_NAN);	/* Simple range check on day-of-year (1-366) */
		*t = gmt_rdc2dt (GMT, gmt_rd_from_gymd (GMT, ival[0], 1, 1) + ival[1] - 1, x);
	}
	else {
		if (gmtlib_g_ymd_is_bad (ival[0], ival[1], ival[2]) )
			return (GMT_IS_NAN);
		*t = gmt_rdc2dt (GMT, gmt_rd_from_gymd (GMT, ival[0], ival[1], ival[2]), x);
	}

	return (GMT_IS_ABSTIME);
}

/*! . */
GMT_LOCAL void gmtio_write_formatted_ogr_value (struct GMT_CTRL *GMT, FILE *fp, int col, int type, struct GMT_OGR_SEG *G) {
	char text[GMT_LEN64] = {""};

	switch (type) {
		case GMT_TEXT:
			fprintf (fp, "%s", G->tvalue[col]);
			break;
		case GMT_DOUBLE:
		case GMT_FLOAT:
			gmt_ascii_format_col (GMT, text, G->dvalue[col], GMT_OUT, GMT_Z);
			fprintf (fp, "%s", text);
			break;
		case GMT_CHAR:
		case GMT_UCHAR:
		case GMT_INT:
		case GMT_UINT:
		case GMT_LONG:
		case GMT_ULONG:
			fprintf (fp, "%ld", lrint (G->dvalue[col]));
			break;
		case GMT_DATETIME:
			gmt_format_abstime_output (GMT, G->dvalue[col], text);
			fprintf (fp, "%s", text);
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bad type passed to gmtio_write_formatted_ogr_value - assumed to be double\n");
			gmt_ascii_format_col (GMT, text, G->dvalue[col], GMT_OUT, GMT_Z);
			fprintf (fp, "%s", text);
			break;
	}
}

/*! . */
GMT_LOCAL void gmtio_write_ogr_segheader (struct GMT_CTRL *GMT, FILE *fp, struct GMT_DATASEGMENT *S) {
	/* Write out segment-level OGR/GMT header metadata */
	unsigned int col, virt_col;
	char *kind = "PH";
	char *sflag[7] = {"-D", "-G", "-I", "-L", "-T", "-W", "-Z"}, *quote[7] = {"", "", "\"", "\"", "\"", "", ""};
	char buffer[GMT_BUFSIZ];
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);

	if (GMT->common.a.geometry == GMT_IS_POLYGON || GMT->common.a.geometry == GMT_IS_MULTIPOLYGON)
		fprintf (fp, "# @%c\n", (int)kind[SH->ogr->pol_mode]);
	if (GMT->common.a.n_aspatial) {
		fprintf (fp, "# @D");
		for (col = 0; col < GMT->common.a.n_aspatial; col++) {
			if (col) fprintf (fp, "|");
			switch (GMT->common.a.col[col]) {
				case GMT_IS_D:	/* Pick up from -D<distance> */
				case GMT_IS_G:	/* Pick up from -G<fill> */
				case GMT_IS_I:	/* Pick up from -I<ID> */
				case GMT_IS_T:	/* Pick up from -T<text> */
				case GMT_IS_W:	/* Pick up from -W<pen> */
				case GMT_IS_Z:	/* Pick up from -Z<value> */
					virt_col = abs (GMT->common.a.col[col]) - 1;	/* So -3 becomes 2 etc */
					if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, sflag[virt_col], buffer))
						fprintf (fp, "%s%s%s", quote[virt_col], buffer, quote[virt_col]);
					break;
				case GMT_IS_L:	/* Pick up from -L<value> */
					virt_col = abs (GMT->common.a.col[col]) - 1;	/* So -3 becomes 2 etc */
					if (S->label) fprintf (fp, "%s%s%s", quote[virt_col], S->label, quote[virt_col]);
					else if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, sflag[virt_col], buffer))
						fprintf (fp, "%s%s%s", quote[virt_col], buffer, quote[virt_col]);
					break;
				default:	/* Regular column cases */
					if (SH->ogr) gmtio_write_formatted_ogr_value (GMT, fp, col, GMT->common.a.type[col], SH->ogr);
					break;
			}
		}
		fprintf (fp, "\n");
	}
}

/*! . */
GMT_LOCAL void gmtio_build_text_from_ogr (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *seg, char *buffer) {
	/* Build segment-level OGR/GMT header metadata */
	unsigned int col, virt_col, n;
	bool space = false;
	char *sflag[7] = {"-D", "-G", "-I", "-L", "-T", "-W", "-Z"};
	char **T = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;

	if (seg) SH = gmt_get_DS_hidden (seg);
	T = (seg) ? SH->ogr->tvalue     : GMT->current.io.OGR->tvalue;	/* Input means system OGR, output from segment ogr struct */
	n = (seg) ? SH->ogr->n_aspatial : GMT->common.a.n_aspatial;
	if (n == 0) return;	/* Either input was not OGR or there are no aspatial fields */
	buffer[0] = 0;
	for (col = 0; col < n; col++) {
		switch (GMT->common.a.col[col]) {
			case GMT_IS_D:	/* Format -D<distance> */
			case GMT_IS_G:	/* Format -G<fill> */
			case GMT_IS_I:	/* Format -I<ID> */
			case GMT_IS_T:	/* Format -T<text> */
			case GMT_IS_W:	/* Format -W<pen> */
			case GMT_IS_Z:	/* Format -Z<value> */
				virt_col = abs (GMT->common.a.col[col]) - 1;	/* So -3 becomes 2 etc */
				if (space) strcat (buffer, " ");
				strncat (buffer, sflag[virt_col], GMT_BUFSIZ-1);
				strncat (buffer, T[GMT->common.a.ogr[col]], GMT_BUFSIZ-1);
				space = true;
				break;
			default:	/* Regular column cases are skipped */
				break;
		}
	}
}

/*! . */
GMT_LOCAL void gmtio_build_segheader_from_ogr (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_DATASEGMENT *S) {
	/* Build segment-level OGR/GMT header metadata */
	char buffer[GMT_BUFSIZ] = {""};

	if (direction == GMT_IN && GMT->common.a.output) return;	/* Input was not OGR (but output will be) */
	gmtio_build_text_from_ogr (GMT, S, buffer);	/* Fill in the buffer for -D, -G, Z etc */

	if (gmt_polygon_is_hole (GMT, S))		/* Indicate this is a polygon hole [Default is perimeter] */
		strcat (buffer, " -Ph");
	if (S->header) {strcat (buffer, " "); strncat (buffer, S->header, GMT_BUFSIZ-1);}	/* Append rest of previous header */
	gmt_M_str_free (S->header);
	S->header = strdup (buffer);
}

/*! . */
GMT_LOCAL void gmtio_alloc_ogr_seg (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, int n_aspatial) {
	/* Allocates the OGR structure for a given segment and copies current values from table OGR segment */
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	if (SH->ogr) return;	/* Already allocated */
	SH->ogr = gmt_M_memory (GMT, NULL, 1, struct GMT_OGR_SEG);
	SH->ogr->n_aspatial = n_aspatial;
	if (n_aspatial) {
		SH->ogr->tvalue = gmt_M_memory (GMT, NULL, n_aspatial, char *);
		SH->ogr->dvalue = gmt_M_memory (GMT, NULL, n_aspatial, double);
	}
}

/*! . */
GMT_LOCAL void gmtio_copy_ogr_seg (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, struct GMT_OGR *G) {
	/* Allocates the OGR structure for a given segment and copies current values from table OGR segment */
	unsigned int col;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);

	gmtio_alloc_ogr_seg (GMT, S, G->n_aspatial);
	for (col = 0; col < G->n_aspatial; col++) {
		if (G->tvalue != NULL && G->tvalue[col])
			SH->ogr->tvalue[col] = strdup (G->tvalue[col]);
		if (G->dvalue != NULL && G->dvalue[col])
			SH->ogr->dvalue[col] = G->dvalue[col];
	}
	SH->ogr->pol_mode = G->pol_mode;
}

/*! . */
GMT_LOCAL int gmtio_prep_ogr_output (struct GMT_CTRL *GMT, struct GMT_DATASET *D) {

	int object_ID, col, stop, n_reg, item;
	uint64_t row, seg, seg1, seg2, k;
	char buffer[GMT_BUFSIZ] = {""}, in_string[GMT_VF_LEN] = {""}, out_string[GMT_VF_LEN] = {""};
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *M = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL, *SH1 = NULL, *SH2 = NULL;
	struct GMTAPI_DATA_OBJECT O;

	/* When this function is called we have already registered the output destination.  This will normally
	 * prevent us from register the data set separately in order to call GMT_gmtinfo.  We must temporarily
	 * unregister the output, do our thing, then reregister again. */

	n_reg = gmtapi_count_objects (GMT->parent, GMT_IS_DATASET, D->geometry, GMT_OUT, &object_ID);	/* Are there outputs registered already? */
	if (n_reg == 1) {	/* Yes, must save and unregister, then reregister later */
		if ((item = gmtapi_validate_id (GMT->parent, GMT_IS_DATASET, object_ID, GMT_OUT)) == GMT_NOTSET)
			return (GMT->parent->error);
		gmt_M_memcpy (&O, GMT->parent->object[item], 1, struct GMTAPI_DATA_OBJECT);
		gmtapi_unregister_io (GMT->parent, object_ID, GMT_OUT);
	}
	else {	/* Cannot have registered more than one output for OGR data */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot specify more than one output file for OGR\n");
		return (GMT_DIM_TOO_LARGE);
	}

	/* Determine w/e/s/n via GMT_gmtinfo */

	if (GMT_Open_VirtualFile (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, D, in_string) == GMT_NOTSET) {
		return (GMT->parent->error);		
	}
	if (GMT_Open_VirtualFile (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, NULL, out_string) == GMT_NOTSET) {
		return (GMT->parent->error);		
	}
	snprintf (buffer, GMT_BUFSIZ, "-C -fg -<%s ->%s --GMT_HISTORY=false", in_string, out_string);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Calling gmtinfo with args %s\n", buffer);
	if (GMT_Call_Module (GMT->parent, "gmtinfo", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Get the extent via gmtinfo */
		return (GMT->parent->error);
	}
	GMT_Close_VirtualFile (GMT->parent, in_string);
	if ((M = GMT_Read_VirtualFile (GMT->parent, out_string)) == NULL) {
		return (GMT->parent->error);
	}
	GMT_Close_VirtualFile (GMT->parent, out_string);

	/* Time to reregister the original destination */

	if ((object_ID = GMT_Register_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, D)) == GMT_NOTSET) {
		return (GMT->parent->error);
	}
	if ((item = gmtapi_validate_id (GMT->parent, GMT_IS_DATASET, object_ID, GMT_OUT)) == GMT_NOTSET) {
		return (GMT->parent->error);
	}
	gmt_M_memcpy (GMT->parent->object[item], &O, 1, struct GMTAPI_DATA_OBJECT);	/* Restore what we had before */

	T = D->table[0];
	TH = gmt_get_DT_hidden (T);
	TH->ogr = gmt_M_memory (GMT, NULL, 1, struct GMT_OGR);
	snprintf (buffer, GMT_BUFSIZ, "%.8g/%.8g/%.8g/%.8g", M->table[0]->segment[0]->data[0][0], M->table[0]->segment[0]->data[1][0],
	                                        M->table[0]->segment[0]->data[2][0], M->table[0]->segment[0]->data[3][0]);
	if (GMT_Destroy_Data (GMT->parent, &M) != GMT_OK) {
		return (GMT->parent->error);
	}
	TH->ogr->region = strdup (buffer);
	TH->ogr->proj[1] = strdup ("\"-Jx1d --PROJ_ELLIPSOID=WGS84\"");
	TH->ogr->proj[2] = strdup ("\"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs\"");
	TH->ogr->proj[3] = strdup ("\"GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]\"");
	TH->ogr->geometry = GMT->common.a.geometry;
	TH->ogr->n_aspatial = GMT->common.a.n_aspatial;
	if (TH->ogr->n_aspatial) {	/* Copy over the command-line settings */
		TH->ogr->name = gmt_M_memory (GMT, NULL, TH->ogr->n_aspatial, char *);
		TH->ogr->type = gmt_M_memory (GMT, NULL, TH->ogr->n_aspatial, enum GMT_enum_type);
		TH->ogr->dvalue = gmt_M_memory (GMT, NULL, TH->ogr->n_aspatial, double);
		for (k = 0; k < TH->ogr->n_aspatial; k++) {
			TH->ogr->name[k] = strdup (GMT->common.a.name[k]);
			TH->ogr->type[k] = GMT->common.a.type[k];
		}
		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in the table */
			S = T->segment[seg];
			SH = gmt_get_DS_hidden (S);
			gmtio_alloc_ogr_seg (GMT, S, TH->ogr->n_aspatial);	/* Copy over any feature-specific values */
			for (k = 0; k < TH->ogr->n_aspatial; k++) {	/* For each column to turn into a constant aspatial value */
				col = GMT->common.a.col[k];
				if (col < 0) continue;	/* Multisegment header entry instead */
				for (row = 1, stop = false; !stop && row < S->n_rows; ++row) {
					if (!doubleAlmostEqualZero (S->data[col][row], S->data[col][row-1]))
						stop = true;
				}
				if (stop) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "The -a option specified a constant column but its contents vary!\n");
					return (GMT_RUNTIME_ERROR);
				}
				else
					SH->ogr->dvalue[k] = S->data[col][0];
			}
		}
		/* OK, successfully passed the constant column tests, if any */
		for (seg = col = 0; seg < T->n_segments; seg++) {	/* Free up columns now stored as aspatial values */
			S = T->segment[seg];
			for (k = 0; k < TH->ogr->n_aspatial; k++)
				if (GMT->common.a.col[k] > 0)
					gmt_M_free (GMT, S->data[GMT->common.a.col[k]]);
			for (k = col = 0; k < T->n_columns; k++) {
				while (!S->data[k]) k++;	/* Next available column */
				S->data[col++] = S->data[k];	/* Update pointers */
			}
			S->n_columns = col;	/* May have lost some columns now */
		}
		T->n_columns = D->n_columns = col;	/* May have lost some columns now */
	}
	if (TH->ogr->geometry == GMT_IS_POLYGON || TH->ogr->geometry == GMT_IS_MULTIPOLYGON) {	/* Must check consistency */
		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in the table */
			if ((TH->ogr->geometry == GMT_IS_POLYGON || TH->ogr->geometry == GMT_IS_MULTIPOLYGON) &&
			     gmt_polygon_is_open (GMT, T->segment[seg]->data[GMT_X], T->segment[seg]->data[GMT_Y], T->segment[seg]->n_rows)) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "The -a option specified [M]POLY but open segments were detected!\n");
				GMT_Destroy_Data (GMT->parent, &D[GMT_OUT]);
				return (GMT_RUNTIME_ERROR);
			}
			gmtio_alloc_ogr_seg (GMT, T->segment[seg], TH->ogr->n_aspatial);	/* Copy over any feature-specific values */
			SH = gmt_get_DS_hidden (T->segment[seg]);
			SH->ogr->pol_mode = GMT_IS_PERIMETER;
			gmt_set_seg_minmax (GMT, TH->ogr->geometry, 0, T->segment[seg]);	/* Make sure min/max are set per polygon */

		}
		/* OK, they are all polygons.  Determine any polygon holes: if a point is fully inside another polygon (not on the edge) */
		for (seg1 = 0; seg1 < T->n_segments; seg1++) {	/* For each segment in the table */
			SH1 = gmt_get_DS_hidden (T->segment[seg1]);
			for (seg2 = seg1 + 1; seg2 < T->n_segments; seg2++) {	/* For each segment in the table */
				SH2 = gmt_get_DS_hidden (T->segment[seg2]);
				if (gmt_inonout (GMT, T->segment[seg1]->data[GMT_X][0], T->segment[seg1]->data[GMT_Y][0],
				                 T->segment[seg2]) == GMT_INSIDE) SH1->ogr->pol_mode = GMT_IS_HOLE;
				if (gmt_inonout (GMT, T->segment[seg2]->data[GMT_X][0], T->segment[seg2]->data[GMT_Y][0], T->segment[seg1]) == GMT_INSIDE)
					SH2->ogr->pol_mode = GMT_IS_HOLE;
			}
		}
	}
	if (TH->ogr->geometry > GMT_IS_POINT && TH->ogr->geometry != GMT_IS_MULTIPOINT) {	/* Must check for Dateline crossings */
		unsigned int n_split;
		uint64_t n_segs = T->n_segments;
		struct GMT_DATASEGMENT **L = NULL;

		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in the table */
			if (!gmt_crossing_dateline (GMT, T->segment[seg])) continue;	/* GIS-safe feature! */
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Feature %" PRIu64 " crosses the Dateline\n", seg);
			if (!GMT->common.a.clip) continue;	/* Not asked to clip */
			/* Here we must split into east and west part(s) */
			if (TH->ogr->geometry == GMT_IS_POLYGON || TH->ogr->geometry == GMT_IS_MULTIPOLYGON) {	/* Clipping must add dateline segments */
				/* Clip into two closed polygons.  Eventually, perhaps return more (eliminate bridges) */
				n_split = gmt_split_poly_at_dateline (GMT, T->segment[seg], &L);
			}
			else {	/* Clipping just needs to add crossing points, unless already present */
				/* Truncate into two or more line segments */
				n_split = gmtlib_split_line_at_dateline (GMT, T->segment[seg], &L);
			}
			if (n_split == 0) continue;	/* Might have crossed dateline but had points exactly at 180 */
			T->segment = gmt_M_memory (GMT, T->segment, n_segs + n_split - 1, struct GMT_DATASEGMENT *);	/* Allow more space for new segments */
			gmt_free_segment (GMT, &(T->segment[seg]));	/* Delete the old one */
			T->segment[seg] = L[0];			/* Hook in the first replacement */
			for (k = 1; k < n_split; k++) T->segment[n_segs++] = L[k];	/* Add the remaining segments to the end */
			gmt_M_free (GMT, L);
		}
		D->n_segments = T->n_segments = n_segs;	/* Update number of segments */

	}
	GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;	/* Select the -180/180 output range format */
	return (0);
}

/*! . */
GMT_LOCAL void gmtio_write_multilines (struct GMT_CTRL *GMT, FILE *fp, char *text, char *prefix) {
	/* Optional title(s) or remarks provided; could be several lines separated by \n */
	char p[GMT_BUFSIZ] = {""}, line[GMT_BUFSIZ] = {""};
	unsigned int pos = 0, k = 0;

	while (gmt_strtok (text, "\\", &pos, p)) {
		snprintf (line, GMT_BUFSIZ, "# %7s : %s", prefix, &p[k]);
		gmtlib_write_tableheader (GMT, fp, line);
		k = 1;	/* Need k to skip the n in \n */
	}
}

/*! . */
GMT_LOCAL void gmtio_adjust_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, uint64_t n_columns) {
	/* Change the number of columns in this segment to n_columns (free or allocate as needed) */
	uint64_t col;
	for (col = n_columns; col < S->n_columns; col++) gmt_M_free (GMT, S->data[col]);	/* Free up if n_columns < S->columns */
	S->data = gmt_M_memory (GMT, S->data, n_columns, double *);
	S->min = gmt_M_memory (GMT, S->min, n_columns, double);
	S->max = gmt_M_memory (GMT, S->max, n_columns, double);
	for (col = S->n_columns; col < n_columns; col++) {	/* Allocate new columns and initialize the min/max arrays */
		S->min[col] = +DBL_MAX;
		S->max[col] = -DBL_MAX;
		S->data[col] = gmt_M_memory (GMT, NULL, S->n_rows, double);
	}
	S->n_columns = n_columns;
}

/*! . */
GMT_LOCAL void gmtio_adjust_table (struct GMT_CTRL *GMT, struct GMT_DATATABLE *T, uint64_t n_columns) {
	/* Let table have n_columns (so either deallocate or allocate columns). */
	uint64_t seg;

	T->min = gmt_M_memory (GMT, T->min, n_columns, double);
	T->max = gmt_M_memory (GMT, T->max, n_columns, double);
	for (seg = 0; seg < T->n_segments; seg++) gmtio_adjust_segment (GMT, T->segment[seg], n_columns);
	T->n_columns = n_columns;	/* New number of n_columns */
}

/*! . */
GMT_LOCAL void gmtio_copy_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *Sout, struct GMT_DATASEGMENT *Sin) {
	/* Duplicates the segment */
	uint64_t col;
	gmt_M_unused(GMT);
	for (col = 0; col < Sin->n_columns; col++) gmt_M_memcpy (Sout->data[col], Sin->data[col], Sin->n_rows, double);
	if (Sin->text) {	/* Must also duplicate text strings */
		uint64_t row;
		for (row = 0; row < Sin->n_rows; row++) Sout->text[row] = strdup (Sin->text[row]);
	}
	gmt_M_memcpy (Sout->min, Sin->min, Sin->n_columns, double);
	gmt_M_memcpy (Sout->max, Sin->max, Sin->n_columns, double);
	Sout->n_rows = Sin->n_rows;
}

/*! . */
GMT_LOCAL void gmtio_free_ogr_seg (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S) {
	/* Frees the OGR structure for a given segment */
	unsigned int k, n;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	n = (GMT->current.io.OGR) ? GMT->current.io.OGR->n_aspatial : GMT->common.a.n_aspatial;
	if (n) {
		for (k = 0; SH->ogr->tvalue && k < n; k++) gmt_M_str_free (SH->ogr->tvalue[k]);
		gmt_M_free (GMT, SH->ogr->tvalue);
		gmt_M_free (GMT, SH->ogr->dvalue);
	}
	gmt_M_free (GMT, SH->ogr);
}

/*! . */
GMT_LOCAL void gmtio_free_univector (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *u, unsigned int type) {
	/* By taking a reference to the vector pointer we can set it to NULL when done */
	if (!u) return;	/* Nothing to deallocate */
	switch (type) {
		case GMT_UCHAR:		gmt_M_free (GMT, u->uc1); break;
		case GMT_CHAR:		gmt_M_free (GMT, u->sc1); break;
		case GMT_USHORT:	gmt_M_free (GMT, u->ui2); break;
		case GMT_SHORT:		gmt_M_free (GMT, u->si2); break;
		case GMT_UINT:		gmt_M_free (GMT, u->ui4); break;
		case GMT_INT:		gmt_M_free (GMT, u->si4); break;
		case GMT_ULONG:		gmt_M_free (GMT, u->ui8); break;
		case GMT_LONG:		gmt_M_free (GMT, u->si8); break;
		case GMT_FLOAT:		gmt_M_free (GMT, u->f4);  break;
		case GMT_DOUBLE:	gmt_M_free (GMT, u->f8);  break;
	}
}

/*! . */
GMT_LOCAL void gmtio_null_univector (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *u, unsigned int type) {
	/* Here we just set the type pointer to NULL as it was pointing to external memory */
	gmt_M_unused(GMT);
	if (!u) return;	/* Nothing to deal with */
	switch (type) {
		case GMT_UCHAR:	 u->uc1 = NULL; break;
		case GMT_CHAR:	 u->sc1 = NULL; break;
		case GMT_USHORT: u->ui2 = NULL; break;
		case GMT_SHORT:	 u->si2 = NULL; break;
		case GMT_UINT:	 u->ui4 = NULL; break;
		case GMT_INT:	 u->si4 = NULL; break;
		case GMT_ULONG:	 u->ui8 = NULL; break;
		case GMT_LONG:	 u->si8 = NULL; break;
		case GMT_FLOAT:	 u->f4  = NULL; break;
		case GMT_DOUBLE: u->f8  = NULL; break;
	}
}

/*! . */
GMT_LOCAL int gmtio_duplicate_univector (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *u_out, union GMT_UNIVECTOR *u_in, unsigned int type, uint64_t n_rows) {
	/* Allocate space for one univector according to data type */
	gmt_M_unused(GMT);
	switch (type) {
		case GMT_UCHAR:  gmt_M_memcpy (u_out->uc1, u_in->uc1, n_rows, uint8_t);   break;
		case GMT_CHAR:   gmt_M_memcpy (u_out->sc1, u_in->sc1, n_rows, int8_t);    break;
		case GMT_USHORT: gmt_M_memcpy (u_out->ui2, u_in->ui2, n_rows, uint16_t);  break;
		case GMT_SHORT:  gmt_M_memcpy (u_out->si2, u_in->si2, n_rows, int16_t);   break;
		case GMT_UINT:   gmt_M_memcpy (u_out->ui4, u_in->ui4, n_rows, uint32_t);  break;
		case GMT_INT:    gmt_M_memcpy (u_out->si4, u_in->si4, n_rows, int32_t);   break;
		case GMT_ULONG:  gmt_M_memcpy (u_out->ui8, u_in->ui8, n_rows, uint64_t);  break;
		case GMT_LONG:   gmt_M_memcpy (u_out->si8, u_in->si8, n_rows, int64_t);   break;
		case GMT_FLOAT:  gmt_M_memcpy (u_out->f4,  u_in->f4,  n_rows, float);     break;
		case GMT_DOUBLE: gmt_M_memcpy (u_out->f8,  u_in->f8,  n_rows, double);    break;
	}
	return (GMT_OK);
}

/*! Translates incoming ij (no padding) to a GMT ij (includes padding) for column-structured data */
GMT_LOCAL uint64_t gmtio_col_ij (struct GMT_Z_IO *r, struct GMT_GRID *G, uint64_t ij) {

	r->gmt_j = (unsigned int)(r->start_row + r->y_step * (ij % r->y_period));
	r->gmt_i = (unsigned int)(r->start_col + r->x_step * (ij / r->y_period));

	return (gmt_M_ijp (G->header, r->gmt_j, r->gmt_i));
}

/*! Translates incoming ij (no padding) to a GMT ij (includes padding) for row-structured data */
GMT_LOCAL uint64_t gmtio_row_ij (struct GMT_Z_IO *r, struct GMT_GRID *G, uint64_t ij) {

	r->gmt_j = (unsigned int)(r->start_row + r->y_step * (ij / r->x_period));
	r->gmt_i = (unsigned int)(r->start_col + r->x_step * (ij % r->x_period));

	return (gmt_M_ijp (G->header, r->gmt_j, r->gmt_i));
}

/*! . */
GMT_LOCAL void * gmtio_bin_input (struct GMT_CTRL *GMT, FILE *fp, uint64_t *n, int *retval) {
	/* General binary read function which calls function pointed to by GMT->current.io.read_binary to handle
	   actual reading (and possibly swabbing) */
	unsigned int status;
	uint64_t n_use, n_read;

	GMT->current.io.status = GMT_IO_DATA_RECORD;
	do {	/* Keep reading until (1) EOF, (2) got a segment record, or (3) a valid data record */
		n_use = gmt_n_cols_needed_for_gaps (GMT, *n);
		gmt_update_prev_rec (GMT, n_use);
		if (gmtio_get_binary_input (GMT, fp, n_use)) { *retval = -1; GMT->current.io.data_record_number_in_tbl[GMT_IN] = 0; return (NULL); }	/* EOF */
		GMT->current.io.rec_no++;
		GMT->current.io.data_record_number_in_set[GMT_IN]++;
		GMT->current.io.data_record_number_in_tbl[GMT_IN]++;
		GMT->current.io.data_record_number_in_seg[GMT_IN]++;
		status = gmtlib_process_binary_input (GMT, n_use);
		if (status == 1) { *retval = 0; return (NULL); }		/* A segment header */
	} while (status == 2);	/* Continue reading when record is to be skipped */
	n_read = (GMT->common.i.select) ? gmtlib_bin_colselect (GMT) : *n;	/* We may use -i and select fewer of the input columns */

	if (gmtlib_gap_detected (GMT)) { *retval = gmtlib_set_gap (GMT); return (&GMT->current.io.record); }
	GMT->current.io.has_previous_rec = true;

	*retval = (int)n_read;
	return (&GMT->current.io.record);
}

/*! . */
GMT_LOCAL struct GMT_DATATABLE *gmtio_alloc_table (struct GMT_CTRL *GMT, struct GMT_DATATABLE *Tin, uint64_t n_columns, uint64_t n_rows) {
	/* Allocate the new Table structure with same # of segments and rows/segment as input table.
	 * However, n_columns is given separately and could differ.
	 * If n_rows is > 0 we well override the Tin rows counts by using n_rows instead.  */
	unsigned int hdr, smode;
	uint64_t seg, nr;
	struct GMT_DATATABLE *T = gmt_get_table (GMT);
	struct GMT_DATATABLE_HIDDEN *TH = gmt_get_DT_hidden (T);

	T->n_segments = TH->n_alloc = Tin->n_segments;	/* Same number of segments as input table */
	T->n_headers  = Tin->n_headers;
	T->n_columns  = n_columns;		/* Separately specified n_columns */
	T->min = gmt_M_memory (GMT, NULL, n_columns, double);
	T->max = gmt_M_memory (GMT, NULL, n_columns, double);
	if (T->n_headers) {
		T->header = gmt_M_memory (GMT, NULL, Tin->n_headers, char *);
		for (hdr = 0; hdr < T->n_headers; hdr++) T->header[hdr] = strdup (Tin->header[hdr]);
	}
	T->segment = gmt_M_memory (GMT, NULL, Tin->n_segments, struct GMT_DATASEGMENT *);
	for (seg = 0; seg < T->n_segments; seg++) {
		nr = (n_rows) ? n_rows : Tin->segment[seg]->n_rows;
		smode = (Tin->segment[seg]->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
		T->segment[seg] = GMT_Alloc_Segment (GMT->parent, smode, nr, n_columns, Tin->segment[seg]->header, NULL);
		T->n_records += nr;
		if (Tin->segment[seg]->label)  T->segment[seg]->label = strdup (Tin->segment[seg]->label);
	}
	return (T);
}

/*! . */
GMT_LOCAL void gmtio_set_current_record (struct GMT_CTRL *GMT, char *text) {
	//while (*text && strchr ("#\t ", *text)) text++;	/* Skip header record indicator and leading whitespace [NO: messes up strsepzp count] */
	while (*text && strchr ("#", *text)) text++;	/* Skip header record indicator */
	if (*text)	/* Got some text to remember */
		strncpy (GMT->current.io.curr_text, text, GMT_BUFSIZ-1);
	else	/* Nutin' */
		gmt_M_memset (GMT->current.io.curr_text, GMT_BUFSIZ, char);
}

GMT_LOCAL unsigned int gmtio_get_type_name_index (unsigned int code) {
	unsigned int k;
	switch (code) {
		case GMT_IS_NAN:          k = 0;	break;
		case GMT_IS_FLOAT:        k = 1;	break;
		case GMT_IS_LAT:          k = 2;	break;
		case GMT_IS_LON:          k = 3;	break;
		case GMT_IS_GEO:          k = 4;	break;
		case GMT_IS_RELTIME:      k = 5;	break;
		case GMT_IS_ABSTIME:      k = 6;	break;
		case GMT_IS_DURATION:     k = 7;	break;
		case GMT_IS_DIMENSION:    k = 8;	break;
		case GMT_IS_GEODIMENSION: k = 9;	break;
		case GMT_IS_AZIMUTH:      k = 10;	break;
		case GMT_IS_ANGLE:        k = 11;	break;
		case GMT_IS_UNKNOWN:      k = 1;	break;
		default: k = 0;	break;
	}
	return (k);
}

GMT_LOCAL unsigned int gmtio_get_coltype_name_index (struct GMT_CTRL *GMT, unsigned int dir, unsigned int col) {
	return (gmtio_get_type_name_index (gmt_M_type (GMT, dir, col)));
}

bool gmtlib_maybe_abstime (struct GMT_CTRL *GMT, char *txt, bool *no_T) {
	size_t k;
	unsigned int n_dash, n_slash;
	gmt_M_unused (GMT);
	*no_T = false;	/* Until proven wrong */
	if (strchr (txt, 'T')) return true;	/* Might be [<date>]T[<clock>], else if is likely text */
	if (strchr (txt, 'e') || strchr (txt, 'E')) return false;	/* Check for exponentials since -0.5e-2 would trigger on two dashes */
	/* Here there are no T, but perhaps somebody forgot to add T to 2004-10-19 or 1999/04/05 ? */
	n_dash = n_slash = 0;
	for (k = 0; k < strlen (txt); k++) {	/* Count slashes and dashes */
		if (txt[k] == '-') n_dash++;
		else if (txt[k] == '/') n_slash++;
	}
	if (((n_dash + n_slash) == 2) && (n_dash == 2 || n_slash == 2)) {	/* Time */
		*no_T = true;
		return true;	/* Might be yyyy/mm/dd or yyyy-mm-dd with missing trailing T */
	}
	return false;
}

GMT_LOCAL unsigned int gmtio_physical_coltype (struct GMT_CTRL *GMT, unsigned int col) {
	/* The users's -f settings apply to the logical columns and these are the same as
	 * the physical columns except when -i is used.  This function takes the physical
	 * column (we are scanning across the physical record) and returns the corresponding
	 * logical column type. With no -i they are the same, otherwise not.
	 */
	if (GMT->common.i.select && GMT->common.i.n_cols) {	/* Logical record differs from physical */
		unsigned int k;
		for (k = 0; k < GMT->common.i.n_cols; k++) {
			if (GMT->current.io.col[GMT_IN][k].col == col)	/* Found one of the physical columns that will be column = order */
				return gmt_M_type (GMT, GMT_IN, GMT->current.io.col[GMT_IN][k].order);
		}
		/* Here we come when a physical column was not selected to be part of the logical column, so no -f is avialble.  We pass GMT_IS_UNKNOWN */
		return GMT_IS_UNKNOWN;
	}
	return gmt_M_type (GMT, GMT_IN, col);	/* Physical == logical */
}

GMT_LOCAL void gmtio_assign_col_type_if_notset (struct GMT_CTRL *GMT, unsigned int col, unsigned int type) {
	/* As gmtio_examine_current_record learns the content of each column, col, we wish to update
	 * the column type so that parsing will work later.  However, there are some complications:
	 * We are scanning the physical record, but if -i was used then the logical record (the one
	 * we actually will return to a module) will differ, so the columns are not the same, and
	 * the module may know more than we do and have already set what the column types should be,
	 * at least for critical columns.  If so we are not allowed to change that even if we may know better.
	 */
	if (GMT->common.i.select && GMT->common.i.n_cols) {	/* Logical record differs from physical */
		unsigned int k;
		for (k = 0; k < GMT->common.i.n_cols; k++) {
			if (GMT->current.io.col[GMT_IN][k].col == col) {	/* Found one of the physical columns that will be column = order */
				if (!GMT->current.io.col_set[GMT_IN][GMT->current.io.col[GMT_IN][k].order])
					gmt_set_column (GMT, GMT_IN, GMT->current.io.col[GMT_IN][k].order, type);
				if (!GMT->current.io.col_set[GMT_OUT][GMT->current.io.col[GMT_IN][k].order])
					gmt_set_column (GMT, GMT_OUT, GMT->current.io.col[GMT_IN][k].order, type);
			}
		}
	}
	else {	/* logical equal physical, so no column shopping */
		if (!GMT->current.io.col_set[GMT_IN][col])
			gmt_set_column (GMT, GMT_IN, col, type);
		if (!GMT->current.io.col_set[GMT_OUT][col])
			gmt_set_column (GMT, GMT_OUT, col, type);
	}
}

/*! . */
GMT_LOCAL unsigned int gmtio_examine_current_record (struct GMT_CTRL *GMT, char *record, size_t *tpos, uint64_t *n_columns) {
	/* Examines this data record to determine the nature of the input.  There
	 * are three different scenarios:
	 * 1. Record only contains numerical columns: Set ncols and return GMT_READ_DATA [0].
	 * 2. Record only contains text: Set ncols to 0 and return GMT_READ_TEXT [1].
	 * 3. Record contains leading numerics followed by text: Set ncols and return GMT_READ_MIXED [2].
	 * Note: The convoluted switch/test below reflects the fact that gmt_scanf_arg was set up
	 * to parse command-line arguments.  For ABSTIME these must be in ISO format while for input
	 * (which is what we are handling here) a huge variety of datetime strings are possible via
	 * the FORMAT_DATE_IN, FORMAT_CLOCK_IN settings.
	 */
	unsigned int ret_val = GMT_READ_DATA, pos = 0, col = 0, k, phys_col_type, *type = NULL;
	int got;
	bool found_text = false, no_T = false;
	char token[GMT_BUFSIZ], message[GMT_BUFSIZ] = {""};
	double value;
	static char *flavor[4] = {"", "Numerical only", "Text only", "Numerical with trailing text"};

	type = gmt_M_memory (GMT, NULL, GMT_MAX_COLUMNS, unsigned int);
	*tpos = pos;
	while (!found_text && (gmt_strtok (record, GMT->current.io.scan_separators, &pos, token))) {
		phys_col_type = gmtio_physical_coltype (GMT, col);
		if (phys_col_type == GMT_IS_ABSTIME || gmtlib_maybe_abstime (GMT, token, &no_T)) {	/* Might be ISO Absolute time; if not we got junk (and got -> GMT_IS_NAN) */
			got = gmt_scanf (GMT, token, GMT_IS_ABSTIME, &value);
		}
		else	/* Let gmt_scanf_arg figure it out for us by passing UNKNOWN since ABSTIME has been dealt above */
			got = gmt_scanf_arg (GMT, token, GMT_IS_UNKNOWN, false, &value);
		if (got == GMT_IS_NAN) {	/* Parsing failed, which means we found our first non-number; but it could also be a valid NaN */
			gmt_str_tolower (token);
			if (strncmp (token, "nan", 3U))
				found_text = true;
			else
				type[col++] = got = GMT_IS_FLOAT;
		}
		else	/* A successful numerical parsing */
				type[col++] = got;
		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION) && col <= 50) {	/* Tell user how we interpreted their first record, but not for excessively long records */
			k = gmtio_get_type_name_index (got);
			if (col>1) strcat (message, ",");
			strncat (message, GMT_coltype_name[k], GMT_BUFSIZ-1);
			if (col == 50) strcat (message, ",...");
		}
		*tpos = pos;
	}
	*n_columns = col;	/* Pass back the numerical column count */
	if (GMT->common.i.end)	/* Asked for unspecified last column on input (e.g., -i3,2,5:), supply the missing last column number */
		gmtlib_reparse_i_option (GMT, col);

	if (found_text) {	/* Determine record type */
		if (GMT->current.io.trailing_text[GMT_IN]) ret_val = (*n_columns) ? GMT_READ_MIXED : GMT_READ_TEXT;	/* Possibly update record type */
	}
	else	/* No trailing text found, reset tpos */
		*tpos = 0;
	if (GMT->current.io.OGR) {	/* A few decision specific to OGR files */
		gmtio_select_all_ogr_if_requested (GMT);	/* Finalize choice of aspatial fields */
		ret_val |= gmtio_reconsider_rectype (GMT);	/* Consider the nature of aspatial information */
	}

	/* Tell user how we interpreted their first record */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Source col types: (%s)\n", message);

	for (k = 0; k < col; k++)	/* Check if we can utilize what we learned about each physical column */
		gmtio_assign_col_type_if_notset (GMT, k, type[k]);
	gmt_M_free (GMT, type);


	if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {	/* Tell user how we interpreted their first record */
		static char *tt = " NYY";
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "ASCII source scanned: Numerical columns: %" PRIu64 ", Trailing text: %c, Record type: %s\n",
			*n_columns, tt[ret_val], flavor[ret_val]);
		if (GMT->common.i.select && GMT->common.i.n_cols > 0) {	/* Made a selection with -i */
			message[0] = '\0';
			for (pos = 0; pos < GMT->common.i.n_cols; pos++) {
				k = gmtio_get_coltype_name_index (GMT, GMT_IN, pos);
				if (pos < 50) {
					if (pos) strcat (message, ",");
					strncat (message, GMT_coltype_name[k], GMT_BUFSIZ-1);
					if (pos == 49) strcat (message, ",...");
				}
			}
			if (GMT->current.io.trailing_text[GMT_IN]) {
				if (pos) strcat (message, ",");
				strcat (message, "String");
			}
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Selected col types: (%s)\n", message);
		}
		if (GMT->common.o.select) {
			message[0] = '\0';
			for (pos = 0; pos < GMT->common.o.n_cols; pos++) {
				col = GMT->current.io.col[GMT_OUT][pos].col;
				k = gmtio_get_coltype_name_index (GMT, GMT_OUT, col);
				if (pos < 50) {
					if (pos) strcat (message, ",");
					strncat (message, GMT_coltype_name[k], GMT_BUFSIZ-1);
					if (pos == 49) strcat (message, ",...");
				}
			}
			if (GMT->current.io.trailing_text[GMT_OUT]) {
				if (pos) strcat (message, ",");
				strcat (message, "String");
			}
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Output col types: (%s)\n", message);
		}
	}

	return (ret_val);
}

GMT_LOCAL void gmtio_extract_trailing_text (struct GMT_CTRL *GMT, size_t start_of_text) {
	if (GMT->common.i.word) {	/* Need to extract a specific column from the trailing text */
		char *word = NULL, *orig = strdup (&GMT->current.io.curr_text[start_of_text]), *trail = orig;
		uint64_t col = 0;
		while (col != GMT->common.i.w_col && (word = strsep (&trail, GMT_TOKEN_SEPARATORS)) != NULL) {
			if (*word != '\0')	/* Skip empty strings */
				col++;
		}
		if (word)
			strncpy (GMT->current.io.curr_trailing_text, word, GMT_BUFSIZ-1);
		else
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Trailing text did not have %" PRIu64 " words - no trailing word read\n", GMT->common.i.w_col);
		gmt_M_str_free (orig);
	}
	else	/* Get the whole enchilada */
		strncpy (GMT->current.io.curr_trailing_text, &GMT->current.io.curr_text[start_of_text], GMT_BUFSIZ-1);
}

GMT_LOCAL inline int reached_EOF (struct GMT_CTRL *GMT) {
	GMT->current.io.status = GMT_IO_EOF;
	if (GMT->current.io.give_report && GMT->current.io.n_bad_records) {	/* Report summary and reset counters */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "This file had %" PRIu64 " data records with invalid x and/or y values\n",
			GMT->current.io.n_bad_records);
		GMT->current.io.n_bad_records = GMT->current.io.n_clean_rec = 0;
		GMT->current.io.rec_no = GMT->current.io.rec_in_tbl_no = 0;
	}
	GMT->current.io.data_record_number_in_tbl[GMT_IN] = GMT->current.io.data_record_number_in_seg[GMT_IN] = 0;
	GMT->current.io.has_previous_rec = false;

	return (-1);
}

/*! This is the lowest-most input function in GMT.  All ASCII table data are read via
 * gmt_ascii_input.  Changes here affect all programs that read such data. */
GMT_LOCAL void *gmtio_ascii_input (struct GMT_CTRL *GMT, FILE *fp, uint64_t *n, int *status) {
	uint64_t pos, col_no = 0, col_pos, n_convert, n_ok = 0, kind, add, n_use = 0;
	uint64_t n_cols_this_record = GMT_MAX_COLUMNS;
	int64_t in_col;
	size_t start_of_text;
	bool done = false, bad_record, set_nan_flag = false, add_aspatial_to_expected = false;
	char line[GMT_BUFSIZ] = {""}, *p = NULL, *token = NULL, *stringp = NULL;
	double val;
	static char * (*strscan) (char **, const char *, size_t *);	/* Pointer to strsepz or strsepzp */

	/* gmtio_ascii_input will skip blank lines and shell comment lines which start
	 * with #.  Fields may be separated by spaces, tabs, or commas.  The routine returns
	 * the actual number of items read [or 0 for segment header and -1 for EOF]
	 * If *n is passed as GMT_BUFSIZ it will be reset to the actual number of fields.
	 * If gap checking is in effect and one of the checks involves a column beyond
	 * the ones otherwise needed by the program we extend the reading so we may
	 * examine the column needed in the gap test.
	 * *status returns the number of fields read, 0 for header records, -1 for EOF.
	 * We return NULL (headers or errors) or pointer to GMT->current.io.curr_rec.
	 * If -i is in effect we must handle the modified output order as well as
	 * check for repeated input column usage.
	 */

	while (!done) {	/* Done becomes true when we successfully have read a data record */

		/* First read until we get a non-blank, non-comment record, or reach EOF */

		GMT->current.io.rec_no++;		/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		GMT->current.io.rec_in_tbl_no++;	/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		if (GMT->current.setting.io_header[GMT_IN] && GMT->current.io.rec_in_tbl_no <= GMT->current.setting.io_n_header_items) {	/* Must treat first io_n_header_items as headers */
			gmt_fgets (GMT, line, GMT_BUFSIZ, fp);	/* Get the line */
			if (GMT->common.h.mode == GMT_COMMENT_IS_RESET) continue;	/* Simplest way to replace headers on output is to ignore them on input */
			gmtio_set_current_record (GMT, line);
			GMT->current.io.status = GMT_IO_TABLE_HEADER;
#if 0
			GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on table headers on output PW: No! If we get here via -hi then no header output was requested */
#endif
			*status = 0;
			return (NULL);
		}

		/* Here we are done with any header records implied by -h */
		if (GMT->current.setting.io_blankline[GMT_IN]) {	/* Treat blank lines as segment markers, so only read a single line */
			p = gmt_fgets (GMT, line, GMT_BUFSIZ, fp);
			GMT->current.io.rec_no++, GMT->current.io.rec_in_tbl_no++;
		}
		else {	/* Default is to skip all blank lines until we get something else (or hit EOF) */
			while ((p = gmt_fgets (GMT, line, GMT_BUFSIZ, fp)) && gmt_is_a_blank_line (line)) GMT->current.io.rec_no++, GMT->current.io.rec_in_tbl_no++;
		}
		if (!p) {	/* Ran out of records, which can happen if file ends in a comment record */
			*status = reached_EOF (GMT);
			return (NULL);
		}

		GMT->current.io.data_record_number_in_set[GMT_IN]++;
		GMT->current.io.data_record_number_in_tbl[GMT_IN]++;
		GMT->current.io.data_record_number_in_seg[GMT_IN]++;
		if (outside_in_row_range (GMT, *(GMT->common.q.rec))) {	/* Records is outside desired row range of interest */
			if (!gmt_is_segment_header (GMT, line))
				continue;
		}
		/* First chop off trailing whitespace and commas */
		gmt_strstrip (line, false); /* Eliminate DOS endings and trailing white space, add linefeed */

		if (gmtio_ogr_parser (GMT, line)) continue;	/* If we parsed a GMT/OGR record we must go up to top of loop and get the next record */
		if (line[0] == GMT->current.setting.io_head_marker[GMT_IN]) {	/* Got a file header, copy it and return */
			if (GMT->common.h.mode == GMT_COMMENT_IS_RESET) continue;	/* Simplest way to replace headers on output is to ignore them on input */
			gmtio_set_current_record (GMT, line);
			GMT->current.io.status = GMT_IO_TABLE_HEADER;
			*status = 0;
			return (NULL);
		}

		if ((kind = gmt_is_segment_header (GMT, line))) {	/* Got a segment header, take action and return */
			GMT->current.io.status = GMT_IO_SEGMENT_HEADER;
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
			GMT->current.io.seg_no++;
			GMT->current.io.segment_header[0] = '\0';
			if (kind == 1) {
				/* If OGR input the also read next 1-2 records to pick up metadata */
				if (GMT->current.io.ogr == GMT_OGR_TRUE) {
					int c;
					if ((c = fgetc (fp)) == '#') {	/* Possibly, this record starts with a comment character # */
						line[0] = (char)c;	/* Since we ate the # already we place it here manually */
						gmt_fgets (GMT, &line[1], GMT_BUFSIZ-1, fp);	/* Start at position 1 since # placed already and required for gmtio_ogr_parser to work */
						gmtio_ogr_parser (GMT, line);	/* Parsed a GMT/OGR record */
						gmtio_build_text_from_ogr (GMT, NULL, GMT->current.io.segment_header);	/* Fill in the buffer for -D, -G, Z etc */
						if (strstr (line, "@H")) strcat (GMT->current.io.segment_header, " -Ph");	/* Sometimes a @P or @H record instead */
						/* May also have a second comment record with just @P or @ H so check for this case */
						if ((c = fgetc (fp)) == '#') {	/* Possibly, this record starts with a comment character # */
							line[0] = (char)c;	/* Since we ate the # already we place it here manually */
							gmt_fgets (GMT, &line[1], GMT_BUFSIZ-1, fp);	/* Start at position 1 since # placed already and required for gmtio_ogr_parser to work */
							gmtio_ogr_parser (GMT, line);	/* Parse a possible GMT/OGR record (just returns if no OGR data there) */
							if (strstr (line, "@H")) strcat (GMT->current.io.segment_header, " -Ph");	/* Add the hole designation to the polygon option */
						}
						else	/* Not a comment record; put that character back on the stream and move on */
							ungetc (c, fp);
					}
					else	/* Not a comment record; put that character back on the stream and move on */
						ungetc (c, fp);
				}
				else
					strncpy (GMT->current.io.segment_header, gmtlib_trim_segheader (GMT, line), GMT_BUFSIZ-1);
				/* Just save the header content, not the marker and leading whitespace */
			}
			/* else we got a segment break instead - and header was set to NULL */
			GMT->current.io.data_record_number_in_seg[GMT_IN] = 0;
			GMT->current.io.has_previous_rec = false;
			*status = 0;
			return (NULL);
		}

		/* Here we know we are processing a data record */

		if (GMT->common.a.active && GMT->current.io.ogr == GMT_OGR_FALSE) {	/* Cannot give -a and not be reading an OGR/GMT file */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Aspatial associations set with -a but input file is not in OGR/GMT format!\n");
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return NULL;
		}

		if (GMT->common.e.active && gmt_skip_record (GMT, GMT->common.e.select, line)) continue;	/* Fail a grep test */

		if (GMT->current.io.first_rec) {	/* Learn from the 1st record what we can about the type of data record this is */
			GMT->current.io.record_type[GMT_IN] = gmtio_examine_current_record (GMT, line, &start_of_text, &n_cols_this_record);
			if (GMT->current.io.variable_in_columns) {	/* Never finalize # of fields since it can change from rec to rec */
				*n = GMT_MAX_COLUMNS;
				strscan = (GMT->current.io.record_type[GMT_IN] & GMT_READ_TEXT) ? &strsepzp : &strsepz;	/* Need zp scanner to detect trailing text */
			}
			else {	/* Happily parsed first record and learned a few things.  Now prevent this block from being executed again for the current data file */
				GMT->current.io.first_rec = false;
				if (GMT->current.io.max_cols_to_read) {	/* A hard column count is enforced at first record */
					*n = GMT->current.io.max_cols_to_read;
					if (GMT->current.io.max_cols_to_read == n_cols_this_record && start_of_text == 0) {
						GMT->current.io.trailing_text[GMT_IN] = false;	/* Turn off reading text since none present */
						GMT->current.io.curr_trailing_text[0] = '\0';
						GMT->current.io.record.text = NULL;
					}
					GMT->current.io.record_type[GMT_IN] = (GMT->current.io.trailing_text[GMT_IN]) ? GMT_READ_MIXED : GMT_READ_DATA;	/* Since otherwise we fail to store the trailing text */
					strscan = (GMT->current.io.trailing_text[GMT_IN]) ? &strsepzp : &strsepz;	/* Need zp scanner to detect anything beyond the fixed columns as trailing text */
				}
				else {	/* Set expected cols and the scanner based on what record type we detected */
					if (start_of_text == 0) {	/* Turn off reading text since none present */
						GMT->current.io.trailing_text[GMT_IN] = false;
						GMT->current.io.curr_trailing_text[0] = '\0';
						GMT->current.io.record.text = NULL;
					}
					else if (n_cols_this_record == 0 && start_of_text) {	/* All text */
						GMT->current.io.input = &gmtio_ascii_textinput;	/* Override and use ASCII text mode */
						strcpy (GMT->current.io.curr_trailing_text, line);
						GMT->current.io.record.text = GMT->current.io.curr_trailing_text;
						GMT->current.io.record.data = NULL;
						*n = 1ULL;			/* We always return 1 item as there are no columns */
						*status = 1;
						return (&GMT->current.io.record);
					}
					*n = (GMT->common.i.select) ? GMT->common.i.n_cols : n_cols_this_record;
					strscan = (GMT->current.io.record_type[GMT_IN] & GMT_READ_TEXT) ? &strsepzp : &strsepz;	/* Need zp scanner to detect trailing text */
					add_aspatial_to_expected = true;	/* Since counts does not include any aspatial additions */
				}
			}
		}

		n_use = gmt_n_cols_needed_for_gaps (GMT, *n);	/* Gives the actual columns we need (which may > *n if gap checking is active; if gap check we also update prev_rec) */
		gmt_update_prev_rec (GMT, n_use);
		if (GMT->current.io.variable_in_columns && n_cols_this_record < n_use) n_use = n_cols_this_record;

		bad_record = set_nan_flag = false;	/* Initialize flags */
		gmtio_set_current_record (GMT, line);	/* Keep copy of current record around */
		col_no = pos = n_ok = 0;		/* Initialize counters */
		in_col = -1;				/* Since we will increment right away inside the loop */
		start_of_text = 0;			/* Position in current string record */
		GMT->current.io.curr_trailing_text[0] = '\0';	/* Start with nuthin. */

		stringp = line;
		while (!bad_record && col_no < n_use && (token = strscan (&stringp, GMT->current.io.scan_separators, &start_of_text)) != NULL) {	/* Get one field at the time until we run out or have issues */
			++in_col;	/* This is the actual column number in the input file */
			if (GMT->common.i.select) {	/* Must do special column-based processing since the -i option was set */
				if (GMT->current.io.col_skip[in_col]) continue;		/* Just skip and not even count this column */
				col_pos = GMT->current.io.col[GMT_IN][col_no].order;	/* Which data column will receive this value */
			}
			else				/* Default column order */
				col_pos = col_no;
			n_convert = gmt_scanf (GMT, token, gmt_M_type (GMT, GMT_IN, col_pos), &val);
			//n_convert = gmt_scanf (GMT, token, gmt_M_type (GMT, GMT_IN, in_col), &val);
			if (n_convert != GMT_IS_NAN && col_pos > 1 && gmt_input_is_nan_proxy (GMT, val))	/* Input matched no-data setting, so change to NaN */
				n_convert = GMT_IS_NAN;
			if (n_convert == GMT_IS_NAN) {	/* Got a NaN or it failed to decode the string */
				if (GMT->current.setting.io_nan_records || !GMT->current.io.skip_if_NaN[col_pos]) {	/* This field (or all fields) can be NaN so we pass it on */
					GMT->current.io.curr_rec[col_pos] = GMT->session.d_NaN;
					n_ok++;	/* Since NaN is considered an OK result */
				}
				else	/* Cannot have NaN in this column, flag record as bad */
					bad_record = true;
				if (GMT->current.io.skip_if_NaN[col_pos]) set_nan_flag = true;	/* Flag that we found NaN in a column that means we should skip */
				col_no++;		/* Count up number of columns found */
			}
			else {					/* Successful decode, assign the value to the input array */
				GMT->current.io.curr_rec[col_pos] = gmt_convert_col (GMT->current.io.col[GMT_IN][col_no], val);
				if (col_pos == GMT_X && gmt_M_type (GMT, GMT_IN, col_pos) & GMT_IS_LON)	/* Must account for periodicity in 360 as per current rule */
					gmtio_adjust_periodic_lon (GMT, &GMT->current.io.curr_rec[col_pos]);
				col_no++;		/* Count up number of columns found */
				n_ok++;
				while (GMT->common.i.select && col_no < GMT->common.i.n_cols && GMT->current.io.col[GMT_IN][col_no].col == GMT->current.io.col[GMT_IN][col_no-1].col) {
					/* This input column is requested more than once */
					col_pos = GMT->current.io.col[GMT_IN][col_no].order;	/* The data column that will receive this value */
					GMT->current.io.curr_rec[col_pos] = gmt_convert_col (GMT->current.io.col[GMT_IN][col_no], val);
					col_no++;
					n_ok++;
				}
			}
		}
		if (outside_in_data_range (GMT, GMT->common.q.col))	/* Must skip this record as key column value is outside desired data-range */
			continue;

		if (start_of_text) {	/* Save pointer to start of trailing text portion of the record */
			while (start_of_text < (GMT_BUFSIZ-1) && GMT->current.io.curr_text[start_of_text] && strchr (GMT->current.io.scan_separators, GMT->current.io.curr_text[start_of_text])) start_of_text++;	/* First wind to start of trailing text */
			gmtio_extract_trailing_text (GMT, start_of_text);
			GMT->current.io.record.text = GMT->current.io.curr_trailing_text;
		}

		if ((add = gmtio_assign_aspatial_cols (GMT))) {	/* We appended <add> columns given via aspatial OGR/GMT values */
			col_no += add;
			n_ok += add;
			if (add_aspatial_to_expected) *n += add, n_use += add;	/* Only do this once */
		}
		if (bad_record) {	/* This record failed our test and had NaNs */
			GMT->current.io.n_bad_records++;
			if (GMT->current.io.give_report && (GMT->current.io.n_bad_records == 1)) {	/* Report 1st occurrence of bad record */
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Encountered first invalid ASCII data record near/at line # %" PRIu64 "\n",
				            GMT->current.io.rec_no);
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Likely causes:\n");
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "(1) Invalid x and/or y values, i.e. NaNs or garbage in text strings.\n");
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "(2) Incorrect data type assumed if -J, -f are not set or set incorrectly.\n");
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "(3) The -: switch is implied but not set.\n");
			}
		}
		else if (GMT->current.io.skip_duplicates && GMT->current.io.has_previous_rec) {	/* Test to determine if we should skip repeated duplicate records with same x,y */
			done = !(GMT->current.io.curr_rec[GMT_X] == GMT->current.io.prev_rec[GMT_X] &&
			         GMT->current.io.curr_rec[GMT_Y] == GMT->current.io.prev_rec[GMT_Y]);	/* Yes, duplicate */
		}
		else
			done = true;	/* Success, we can get out of this loop and return what we got */
	}
	GMT->current.io.status = (GMT->current.io.variable_in_columns || n_ok == n_use || *n == GMT_MAX_COLUMNS) ? 0 : GMT_IO_MISMATCH;	/* Hopefully set status to 0 (OK) */
	if (*n == GMT_MAX_COLUMNS) *n = n_ok;							/* Update the number of expected fields */
	if (gmt_M_rec_is_error (GMT))
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Mismatch between actual (%d) and expected (%d) fields near line %" PRIu64 " in file %s\n",
		            col_no, *n, GMT->current.io.rec_no, GMT->current.io.filename[GMT_IN]);

	if (GMT->current.setting.io_lonlat_toggle[GMT_IN] && col_no >= 2) {
		gmt_M_double_swap (GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	}
	else if (GMT->current.proj.inv_coordinates)
		gmtio_adjust_projected (GMT);	/* Must apply inverse projection to get lon, lat */
	if (gmtlib_gap_detected (GMT)) {	/* A gap between this an previous record was detected (see -g) so we set status and return 0 */
		*status = gmtlib_set_gap (GMT);
		return (&GMT->current.io.record);
	}

	//GMT->current.io.data_record_number_in_set[GMT_IN]++;	/* Got a valid data record (which is true even if it was a gap) */
	*status = (int)n_ok;			/* Return the number of fields successfully read */
	if (set_nan_flag) {
		GMT->current.io.status |= GMT_IO_NAN;	/* Say we found NaNs */
		return (&GMT->current.io.record);	/* Pass back pointer to data array */
	}
	GMT->current.io.has_previous_rec = true;
	return ((GMT->current.io.status) ? NULL : &GMT->current.io.record);	/* Pass back pointer to data array */
}

/*! . */
GMT_LOCAL int gmtio_write_table (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, struct GMT_DATATABLE *table, bool use_GMT_io, unsigned int io_mode, unsigned int n_seg) {
	/* Writes an entire segment data set to file or wherever.
	 * Specify io_mode == GMT_WRITE_SEGMENT or GMT_WRITE_TABLE_SEGMENT to write segments to individual files.
	 * If dist is NULL we choose stdout. */

	bool ASCII, close_file = false, append, was;
	int save = 0;
	unsigned int k;
	uint64_t row = 0, seg, col;
	int *fd = NULL;
	char open_mode[4] = {""}, file[PATH_MAX] = {""}, tmpfile[PATH_MAX] = {""}, *out_file = tmpfile, *txt = NULL;
	double *out = NULL;
	FILE *fp = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = gmt_get_DT_hidden (table);
	int (*psave) (struct GMT_CTRL *, FILE *, uint64_t, double *, char *) = NULL;	/* Pointer to function writing tables */


	if (TH->mode == GMT_WRITE_SKIP) return (0);	/* Skip this table */

	append = (dest_type == GMT_IS_FILE && dest && ((char *)dest)[0] == '>');	/* Want to append to existing file */

	if (use_GMT_io) {	/* Use GMT->current.io.info settings to determine if input is ASCII/binary, else it defaults to ASCII */
		strcpy (open_mode, (append) ? GMT->current.io.a_mode : GMT->current.io.w_mode);
		ASCII = !GMT->common.b.active[GMT_OUT];
	}
	else {			/* Force ASCII mode */
		strcpy (open_mode, (append) ? "a" : "w");
		ASCII = true;
		GMT->current.io.output = GMT->session.output_ascii;	/* Override and use ASCII mode */
	}
	psave = GMT->current.io.output;		/* Save the previous pointer since we need to change it back at the end */

	switch (dest_type) {
		case GMT_IS_FILE:	/* dest is a file name */
			if (!dest) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "pointer 'dest' cannot be NULL here\n");
				GMT_exit (GMT, GMT_ARG_IS_NULL); return GMT_ARG_IS_NULL;
			}
			strncpy (file, dest, PATH_MAX-1);
			if (io_mode < GMT_WRITE_SEGMENT) {	/* Only require one destination */
				if ((fp = gmt_fopen (GMT, &file[append], open_mode)) == NULL) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot open file %s\n", &file[append]);
					GMT_exit (GMT, GMT_ERROR_ON_FOPEN); return GMT_ERROR_ON_FOPEN;
				}
				close_file = true;	/* We only close files we have opened here */
			}
			break;
		case GMT_IS_STREAM:	/* Open file pointer given, just copy */
			fp = (FILE *)dest;
			if (fp == NULL) fp = GMT->session.std[GMT_OUT];	/* Default destination */
			if (fp == GMT->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output stream>");
			break;
		case GMT_IS_FDESC:		/* Open file descriptor given, just convert to file pointer */
			fd = dest;
			if (fd && (fp = fdopen (*fd, open_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot convert file descriptor %d to stream in gmtio_write_table\n", *fd);
				GMT_exit (GMT, GMT_ERROR_ON_FDOPEN); return GMT_ERROR_ON_FDOPEN;
			}
			else
				close_file = true;	/* fdopen allocates memory */
			if (fd == NULL) fp = GMT->session.std[GMT_OUT];	/* Default destination */
			if (fp == GMT->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output file descriptor>");
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized source type %d in gmtio_write_table\n", dest_type);
			GMT_exit (GMT, GMT_NOT_A_VALID_METHOD); return GMT_NOT_A_VALID_METHOD;
			break;
	}
	was = GMT->current.io.multi_segments[GMT_OUT];
	if (io_mode < GMT_WRITE_SEGMENT) {
		if (ASCII && GMT->current.setting.io_header[GMT_OUT]) {
			for (k = 0; k < table->n_headers; k++) gmtlib_write_tableheader (GMT, fp, table->header[k]);	/* Write any existing header comments */
			gmtlib_write_newheaders (GMT, fp, table->n_columns);	/* Write general header block */
		}
		if (TH->ogr) gmtlib_write_ogr_header (fp, TH->ogr);	/* Must write OGR/GMT header */
		switch (GMT->current.setting.io_first_header) {
			case GMT_FIRST_SEGHEADER_MAYBE:
				GMT->current.io.multi_segments[GMT_OUT] = (n_seg > 1 || (table->n_segments == 1 && table->segment[0]->header));
				break;
			case GMT_FIRST_SEGHEADER_ALWAYS:
				GMT->current.io.multi_segments[GMT_OUT] = true;
				break;
			case GMT_FIRST_SEGHEADER_NEVER:
				GMT->current.io.multi_segments[GMT_OUT] = (n_seg >1);
				break;
		}
	}

	out = gmt_M_memory (GMT, NULL, table->n_columns, double);
	for (seg = 0; seg < table->n_segments; seg++) {
		S = table->segment[seg];
		SH = gmt_get_DS_hidden (S);
		if (SH->mode == GMT_WRITE_SKIP) continue;	/* Skip this segment */

		if (io_mode >= GMT_WRITE_SEGMENT) {	/* Create separate file for each segment */
			if (SH->file[GMT_OUT])
				out_file = SH->file[GMT_OUT];
			else if (io_mode == GMT_WRITE_TABLE_SEGMENT)	/* Build name with table id and seg # */
				snprintf (tmpfile, PATH_MAX, file, TH->id, seg);
			else					/* Build name with seg ids */
				snprintf (tmpfile, PATH_MAX, file, SH->id);

			if (close_file) gmt_fclose (GMT, fp);	/* Close the file since we opened it */
			if ((fp = gmt_fopen (GMT, out_file, open_mode)) == NULL) {
				gmt_M_free (GMT, out);
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot open file %s\n", out_file);
				GMT_exit (GMT, GMT_ERROR_ON_FOPEN); return GMT_ERROR_ON_FOPEN;
			}
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Writing data segment to file %s\n", out_file);
			if (ASCII && GMT->current.setting.io_header[GMT_OUT]) {
				for (k = 0; k < table->n_headers; k++)
					gmtlib_write_tableheader (GMT, fp, table->header[k]);	/* Write any existing header comments */
				gmtlib_write_newheaders (GMT, fp, table->n_columns);	/* Write general header block */
			}
		}
		if (GMT->current.io.multi_segments[GMT_OUT]) {	/* Want to write segment headers */
			if (!GMT->common.a.output && SH->ogr) gmtio_build_segheader_from_ogr (GMT, GMT_OUT, S);	/* We have access to OGR metadata */
			if (S->header)
				strncpy (GMT->current.io.segment_header, S->header, GMT_BUFSIZ-1);
			else
				GMT->current.io.segment_header[0] = '\0';
			gmt_write_segmentheader (GMT, fp, S->n_columns);
			if (SH->ogr && GMT->common.a.output) gmtio_write_ogr_segheader (GMT, fp, S);
		}
		if (SH->mode == GMT_WRITE_HEADER) continue;	/* Skip after writing segment header */
		if (SH->range && SH->range != GMT->current.io.geo.range) {	/* Segment-specific formatting for longitudes */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s Segment %d changed io.geo.range from %d to %d\n",
			            out_file, (int)seg, GMT->current.io.geo.range, SH->range);
			save = GMT->current.io.geo.range; GMT->current.io.geo.range = SH->range;
		}
		for (row = 0; row < S->n_rows; row++) {
			txt = (S->text) ? S->text[row] : NULL;
			for (col = 0; col < S->n_columns; col++) out[col] = S->data[col][row];
			GMT->current.io.output (GMT, fp, S->n_columns, out, txt);
		}
		if (SH->range) GMT->current.io.geo.range = save; 	/* Restore formatting */
		if (io_mode == GMT_WRITE_SEGMENT) gmt_fclose (GMT, fp);	/* Close the segment file */
		if (ASCII && S->text)
			GMT->current.io.output = psave;	/* Override and use ASCII mode */
	}

	if (close_file) gmt_fclose (GMT, fp);	/* Close the file since we opened it */
	gmt_M_free (GMT, out);			/* Free up allocated memory */
	if (!use_GMT_io) GMT->current.io.output = psave;	/* Restore former pointers and values */
	GMT->current.io.multi_segments[GMT_OUT] = was;

	return (0);	/* OK status */
}

/*! . */
GMT_LOCAL void * gmtio_nc_input (struct GMT_CTRL *GMT, FILE *fp, uint64_t *n, int *retval)
{	/* netCDF tables contain information about the number of records, so we can use a
	 * faster strategy: When file is opened, determine number of rows and columns and
	 * preallocate all the column vectors.  Then, when we ask for the first data record
	 * we read the entire data set.  We then simply return the values corresponding to
	 * the current row.  Note some variables may be 2-D and we then consider them as a
	 * stack of 1-D columns to loop over.
	 */
	int status;
	uint64_t n_use = 0, col;
	gmt_M_unused(fp);

	GMT->current.io.status = GMT_IO_DATA_RECORD;
	if (*n == GMT_MAX_COLUMNS) {	/* Set columns if not known yet */
		*n = GMT->current.io.ncols;			/* Number of requested columns */
	}
	if (GMT->current.io.nrec == 0) {	/* First record, read the entire file and do all scalings */
		uint64_t k, row;
		int v;
		size_t start[5], count[5];
		n_use = gmt_n_cols_needed_for_gaps (GMT, *n);	/* Specified number of output columns */
		for (v = 0, col = 0; v < GMT->current.io.nvars && col < n_use; ++v) {	/* For each named variable v ... */
			/* Copy info from current.io. t_index is generally {0,0,0,0,0}, count is generally {ndim,1,1,1,1}.
			   For 2D array: count is {ndim,ncol,1,1,1} */
			for (k = 0; k < 5; ++k) {
				start[k] = GMT->current.io.t_index[v][k];
				count[k] = GMT->current.io.count[v][k];
			}
			for (k = 0; k < GMT->current.io.count[v][1]; ++col, ++k) {	/* For each column in variable v [typically 1 unless 2-D array] */
				start[1] = k, count[1] = 1;	/* Read only k'th column in 2-D array or the only column [k = 0] in 1-D array */
				nc_get_vara_double (GMT->current.io.ncid, GMT->current.io.varid[v], start, count, GMT->hidden.mem_coord[col]);	/* Read column */
				for (row = 0; row < GMT->current.io.ndim; ++row) {	/* Loop over all records (rows) to do scaling */
					if (GMT->hidden.mem_coord[col][row] == GMT->current.io.missing_value[v])	/* Nan proxy detected */
						GMT->hidden.mem_coord[col][row] = GMT->session.d_NaN;
					else	/* Regular translation */
						GMT->hidden.mem_coord[col][row] = GMT->hidden.mem_coord[col][row] * GMT->current.io.scale_factor[v] + GMT->current.io.add_offset[v];
					GMT->hidden.mem_coord[col][row] = gmt_convert_col (GMT->current.io.col[GMT_IN][v], GMT->hidden.mem_coord[col][row]);	/* Any additional user scalings */
				}
			}
		}
		/* OK, everything is read into memory; below we simply return the items from each array at the given record number */
	}
	else if (*n > GMT->current.io.ncols) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtio_nc_input is asking for %d columns, but file has only %" PRIu64 "\n",
		            *n, GMT->current.io.ncols);
		GMT->current.io.status = GMT_IO_MISMATCH;
	}
	do {	/* Keep reading until (1) EOF, (2) got a segment record, or (3) a valid data record */
		n_use = gmt_n_cols_needed_for_gaps (GMT, *n);
		gmt_update_prev_rec (GMT, n_use);	/* Copy current record to previous record before getting the new current record */

		if (GMT->current.io.nrec == GMT->current.io.ndim) {	/* Reading past last record means EOF for netCDF files */
			GMT->current.io.status = GMT_IO_EOF;
			*retval = -1;
			return (NULL);
		}
		/* Just get values from current row in the mem_coord arrays */
		for (col = 0; col < GMT->current.io.ncols; ++col)
			GMT->current.io.curr_rec[col] = GMT->hidden.mem_coord[col][GMT->current.io.nrec];
		/* Increment record counters */
		GMT->current.io.nrec++;
		GMT->current.io.rec_no++;
		GMT->current.io.data_record_number_in_set[GMT_IN]++;
		GMT->current.io.data_record_number_in_tbl[GMT_IN]++;
		GMT->current.io.data_record_number_in_seg[GMT_IN]++;
		GMT->current.io.has_previous_rec = true;
		status = gmtlib_process_binary_input (GMT, n_use);	/* Determine if a header record, data record, or record to skip */
		if (status == 1) { *retval = 0; return (NULL); }	/* Found a segment header, meaning all columns were NaN */
		if (gmtlib_gap_detected (GMT)) {	/* The -g is in effect and was triggered due to a user-specified gap criteria */
			*retval = gmtlib_set_gap (GMT);
			return (&GMT->current.io.record);
		}
		GMT->current.io.has_previous_rec = true;
	} while (status == 2);	/* Continue reading when a record is to be skipped */

	*retval = (int)*n;
	return (&GMT->current.io.record);
}

/*! . */
GMT_LOCAL FILE *gmt_nc_fopen (struct GMT_CTRL *GMT, const char *filename, const char *mode) {
/* Open a netCDF file for column I/O. Append ?var1/var2/... to indicate the requested columns.
 * Currently only reading is supported.
 * The routine returns a fake file pointer (in fact the netCDF file ID), but stores
 * all the relevant information in the GMT->current.io struct (ncid, ndim, nrec, varid, add_offset,
 * scale_factor, missing_value). Some of these are allocated here, and have to be
 * deallocated upon gmt_fclose.
 * Also asigns GMT->current.io.col_type[GMT_IN] based on the variable attributes.
 */

	char file[PATH_MAX] = {""}, path[PATH_MAX] = {""};
	int i, j, nvars, dimids[5] = {-1, -1, -1, -1, -1}, ndims, in, id;
	size_t n, item[2];
	size_t tmp_pointer; /* To avoid "cast from pointer to integer of different size" */
	double t_value[5], dummy[2];
	char varnm[20][GMT_LEN64], long_name[GMT_LEN256] = {""}, units[GMT_LEN256] = {""};
	char varname[GMT_LEN64] = {""}, dimname[GMT_LEN64] = {""};
	struct GMT_TIME_SYSTEM time_system;
	bool by_value;

	if (mode[0] != 'r') {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_nc_fopen does not support netCDF writing mode\n");
		GMT_exit (GMT, GMT_NOT_A_VALID_DIRECTION); return NULL;
	}

	gmt_M_memset (varnm, 20 * GMT_LEN64, char);

	nvars = sscanf (filename,
		"%[^?]?%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]",
		file, varnm[0], varnm[1], varnm[2], varnm[3], varnm[4], varnm[5], varnm[6], varnm[7], varnm[8], varnm[9], varnm[10],
		varnm[11], varnm[12], varnm[13], varnm[14], varnm[15], varnm[16], varnm[17], varnm[18], varnm[19]) - 1;
	if (gmt_getdatapath (GMT, file, path, R_OK) == NULL) return (NULL);	/* No such file */
	if (nc_open (path, NC_NOWRITE, &GMT->current.io.ncid)) return (NULL);
	if (gmt_M_compat_check (GMT, 4)) {
		if (nvars <= 0) nvars = sscanf (GMT->common.b.varnames,
			"%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]",
			varnm[0], varnm[1], varnm[2], varnm[3], varnm[4], varnm[5], varnm[6], varnm[7], varnm[8], varnm[9], varnm[10],
			varnm[11], varnm[12], varnm[13], varnm[14], varnm[15], varnm[16], varnm[17], varnm[18], varnm[19]);
	}
	if (nvars <= 0)
		nc_inq_nvars (GMT->current.io.ncid, &GMT->current.io.nvars);
	else
		GMT->current.io.nvars = nvars;
	GMT->current.io.varid = gmt_M_memory (GMT, NULL, GMT->current.io.nvars, int);
	GMT->current.io.scale_factor = gmt_M_memory (GMT, NULL, GMT->current.io.nvars, double);
	GMT->current.io.add_offset = gmt_M_memory (GMT, NULL, GMT->current.io.nvars, double);
	GMT->current.io.missing_value = gmt_M_memory (GMT, NULL, GMT->current.io.nvars, double);
	GMT->current.io.ndim = GMT->current.io.nrec = 0;

	for (i = 0; i < GMT->current.io.nvars; i++) {

		/* Check for indices */
		for (j = 0; j < 5; j++) GMT->current.io.t_index[i][j] = 0, GMT->current.io.count[i][j] = 1;
		j = in = 0, by_value = false;
		while (varnm[i][j] && varnm[i][j] != '(' && varnm[i][j] != '[') j++;
		if (varnm[i][j] == '(') {
			in = sscanf (&varnm[i][j+1], "%lf,%lf,%lf,%lf", &t_value[1], &t_value[2], &t_value[3], &t_value[4]);
			varnm[i][j] = '\0';
			by_value = true;
		}
		else if (varnm[i][j] == '[') {
			in = sscanf (&varnm[i][j+1], "%" SCNuS ",%" SCNuS ",%" SCNuS ",%" SCNuS, &GMT->current.io.t_index[i][1],
			             &GMT->current.io.t_index[i][2], &GMT->current.io.t_index[i][3], &GMT->current.io.t_index[i][4]);
			varnm[i][j] = '\0';
		}

		/* Get variable ID and variable name */
		if (nvars <= 0)
			GMT->current.io.varid[i] = i;
		else
			gmt_M_err_fail (GMT, nc_inq_varid (GMT->current.io.ncid, varnm[i], &GMT->current.io.varid[i]), file);
		nc_inq_varname (GMT->current.io.ncid, GMT->current.io.varid[i], varname);

		/* Check number of dimensions */
		gmt_M_err_fail (GMT, nc_inq_varndims (GMT->current.io.ncid, GMT->current.io.varid[i], &ndims), file);
		if (ndims > 5) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "NetCDF variable %s has too many dimensions (%d)\n", varname, j);
			GMT_exit (GMT, GMT_DIM_TOO_LARGE); return NULL;
		}
		if (ndims - in < 1) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "NetCDF variable %s has %" PRIuS " dimensions, cannot specify more than %d indices; ignoring remainder\n", varname, ndims, ndims-1);
			for (j = in; j < ndims; j++) GMT->current.io.t_index[i][j] = 0;
		}
		if (ndims - in > 2)
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "NetCDF variable %s has %" PRIuS " dimensions, showing only 2\n", varname, ndims);

		/* Get information of the first two dimensions */
		nc_inq_vardimid(GMT->current.io.ncid, GMT->current.io.varid[i], dimids);
		nc_inq_dimlen(GMT->current.io.ncid, dimids[0], &n);
		if (GMT->current.io.ndim != 0 && GMT->current.io.ndim != n) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "NetCDF variable %s has different dimension (%" PRIuS ") from others (%" PRIuS ")\n",
			            varname, n, GMT->current.io.ndim);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return NULL;
		}
		GMT->current.io.count[i][0] = GMT->current.io.ndim = n;
		if (dimids[1] >= 0 && ndims - in > 1) {
			nc_inq_dimlen(GMT->current.io.ncid, dimids[1], &n);
		}
		else
			n = 1;
		GMT->current.io.count[i][1] = n;
		GMT->current.io.ncols += (int)n;

		/* If selected by value instead of index */
		for (j = 1; by_value && j <= in; j++) {
			nc_inq_dim (GMT->current.io.ncid, dimids[j], dimname, &n);
			nc_inq_varid (GMT->current.io.ncid, dimname, &id);
			item[0] = 0, item[1] = n-1;
			if (nc_get_att_double (GMT->current.io.ncid, id, "actual_range", dummy)) {
				nc_get_var1_double (GMT->current.io.ncid, id, &item[0], &dummy[0]);
				nc_get_var1_double (GMT->current.io.ncid, id, &item[1], &dummy[1]);
			}
			GMT->current.io.t_index[i][j] = lrint((t_value[j] - dummy[0]) / (dummy[1] - dummy[0]));
		}

		/* Get scales, offsets and missing values */
		if (nc_get_att_double (GMT->current.io.ncid, GMT->current.io.varid[i], "scale_factor",
		                       &GMT->current.io.scale_factor[i]))
			GMT->current.io.scale_factor[i] = 1.0;
		if (nc_get_att_double (GMT->current.io.ncid, GMT->current.io.varid[i], "add_offset",
		                       &GMT->current.io.add_offset[i]))
			GMT->current.io.add_offset[i] = 0.0;
		if (nc_get_att_double (GMT->current.io.ncid, GMT->current.io.varid[i], "_FillValue",
		                       &GMT->current.io.missing_value[i]) && nc_get_att_double (GMT->current.io.ncid, GMT->current.io.varid[i],
		                       "missing_value", &GMT->current.io.missing_value[i]))
		    GMT->current.io.missing_value[i] = GMT->session.d_NaN;

		/* Scan for geographical or time units */
		if (gmtlib_nc_get_att_text (GMT, GMT->current.io.ncid, GMT->current.io.varid[i], "long_name", long_name, GMT_LEN256)) long_name[0] = 0;
		if (gmtlib_nc_get_att_text (GMT, GMT->current.io.ncid, GMT->current.io.varid[i], "units", units, GMT_LEN256)) units[0] = 0;
		gmt_str_tolower (long_name); gmt_str_tolower (units);

		if (gmt_M_type (GMT, GMT_IN, i) == GMT_IS_FLOAT)
			{ /* Float type is preset, do not alter */ }
		else if (!strcmp (long_name, "longitude") || strstr (units, "degrees_e"))
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_LON);
		else if (!strcmp (long_name, "latitude") || strstr (units, "degrees_n"))
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_LAT);
		else if (!strcmp (long_name, "time") || !strcmp (varname, "time")) {
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_RELTIME);
			gmt_M_memcpy (&time_system, &GMT->current.setting.time_system, 1, struct GMT_TIME_SYSTEM);
			if (gmt_get_time_system (GMT, units, &time_system) || gmt_init_time_system_structure (GMT, &time_system))
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Time units [%s] in NetCDF file not recognised, defaulting to gmt.conf.\n",
				            units);
			/* Determine scale between data and internal time system, as well as the offset (in internal units) */
			GMT->current.io.scale_factor[i] = GMT->current.io.scale_factor[i] * time_system.scale * GMT->current.setting.time_system.i_scale;
			GMT->current.io.add_offset[i] *= time_system.scale;	/* Offset in seconds */
			GMT->current.io.add_offset[i] += GMT_DAY2SEC_F * ((time_system.rata_die - GMT->current.setting.time_system.rata_die) +
			                                 (time_system.epoch_t0 - GMT->current.setting.time_system.epoch_t0));
			GMT->current.io.add_offset[i] *= GMT->current.setting.time_system.i_scale;	/* Offset in internal time units */
		}
		else if (gmt_M_type (GMT, GMT_IN, i) == GMT_IS_UNKNOWN)
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_FLOAT);
	}

	GMT->current.io.input = gmtio_nc_input;
	tmp_pointer = (size_t)(-GMT->current.io.ncid);

	gmt_prep_tmp_arrays (GMT, GMT_IN, GMT->current.io.ndim, GMT->current.io.ncols);	/* Preallocate arrays for all netcdf vectors */

	return ((FILE *)tmp_pointer);
}

/*! . */
GMT_LOCAL bool gmt_file_is_readable (struct GMT_CTRL *GMT, char *path)
{	/* Returns true if readable, otherwise give error and return false */
	if (!access (path, R_OK)) return (true);	/* Readable */
	/* Get here when found, but not readable */
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Unable to read %s (permissions?)\n", path);
	return (false);	/* Cannot read, give up */
}

#if 0 /* NOT USED ??? */
GMT_LOCAL int gmtio_validate_aspatial (struct GMT_CTRL *GMT, struct GMT_OGR *G) {
	unsigned int k;
	if (GMT->current.io.ogr != GMT_OGR_TRUE) return (GMT_OK);	/* No point checking further since file is not GMT/OGR */
	for (k = 0; k < GMT->common.a.n_aspatial; k++) if (gmt_get_ogr_id (G, GMT->common.a.name[k])) return (-1);
	return (GMT_OK);
}

/* NOT USED ??? */
GMT_LOCAL int gmtio_load_aspatial_values (struct GMT_CTRL *GMT, struct GMT_OGR *G) {
	/* Uses the info in -a and OGR to replace values in the curr_rec array with aspatial values */

	unsigned int k, n;
	int id;
	for (k = n = 0; k < GMT->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if ((id = gmt_get_ogr_id (G, GMT->common.a.name[k])) == GMT_NOTSET) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "No aspatial value found for column %s\n", GMT->common.a.name[k]);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		switch (G->type[id]) {
			case GMT_DOUBLE:
			case GMT_FLOAT:
			case GMT_ULONG:
			case GMT_LONG:
			case GMT_UINT:
			case GMT_INT:
			case GMT_USHORT:
			case GMT_SHORT:
			case GMT_UCHAR:
			case GMT_CHAR:
				GMT->current.io.curr_rec[GMT->common.a.col[k]] = atof (G->tvalue[id]);
				break;
			case GMT_DATETIME:
				gmt_scanf_arg (GMT, G->tvalue[id], GMT_IS_ABSTIME, false, &GMT->current.io.curr_rec[GMT->common.a.col[k]]);
				break;
			default:	/* Do nothing (string) */
				break;
		}
		n++;
	}
	return (n);
}
#endif

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

/*! . */
int gmtlib_scanf_geodim (struct GMT_CTRL *GMT, char *s, double *val) {
	/* Try to decode a value from s and store
	in val.  s is a regular float with optional
	unit info, e.g., 8.5d or 7.5n.  If a valid unit
	is found we convert the number to km.
	We also skip any trailing modifiers like +<mods>, e.g.
	vector specifications like 0.5i+jc+b+s

	We return GMT_IS_FLOAT and pass val in km.
	*/
	char *p;
	if (isalpha ((int)s[0]) || (s[1] == 0 && (s[0] == '-' || s[0] == '+'))) {	/* Probably a symbol character; quietly return 0 */
		*val = 0.0;
		return GMT_IS_FLOAT;
	}

	if ((p = strpbrk (s, GMT_LEN_UNITS))) {	/* Find location of first valid unit */
		int c = p[0];	p[0] = '\0';	/* Chop off unit */
		*val = atof (s);
		p[0] = (char)c;	/* Restore unit */
		switch (c) {
			case 'd': *val *= GMT->current.proj.DIST_KM_PR_DEG; break;			/* arc degree */
			case 'm': *val *= GMT->current.proj.DIST_KM_PR_DEG * GMT_MIN2DEG; break;	/* arc minutes */
			case 's': *val *= GMT->current.proj.DIST_KM_PR_DEG * GMT_SEC2DEG; break;	/* arc seconds */
			case 'e': *val *= 0.001; break;						/* meters */
			case 'f': *val *= (0.001 * METERS_IN_A_FOOT); break;					/* feet */
			case 'M': *val *= (0.001 * METERS_IN_A_MILE); break;					/* miles */
			case 'n': *val *= (0.001 * METERS_IN_A_NAUTICAL_MILE); break;				/* nautical miles */
			case 'u': *val *= (0.001 * METERS_IN_A_SURVEY_FOOT); break;				/* survey feet */
		}
	}
	else	/* No unit, must assume default km */
		*val = atof (s);

	return (GMT_IS_FLOAT);
}

/*! . */
void gmt_set_geographic (struct GMT_CTRL *GMT, unsigned int dir) {
	/* Eliminate lots of repeated statements to do this: */
	gmt_set_column (GMT, dir, GMT_X, GMT_IS_LON);
	gmt_set_column (GMT, dir, GMT_Y, GMT_IS_LAT);
	if (dir == GMT_IN) gmt_init_distaz (GMT, GMT_MAP_DIST_UNIT, GMT_GREATCIRCLE, GMT_MAP_DIST);	/* Default spherical distance calculations are in meters */
}

/*! . */
void gmt_set_cartesian (struct GMT_CTRL *GMT, unsigned int dir) {
	/* Eliminate lots of repeated statements to do this: */
	gmt_set_column (GMT, dir, GMT_X, GMT_IS_FLOAT);
	gmt_set_column (GMT, dir, GMT_Y, GMT_IS_FLOAT);
}

/*! Handles non-proxy checking for input z values.  If the input value equals
 * the non_proxy then we return true so the value can be replaced by a NaN.
 */
bool gmt_input_is_nan_proxy (struct GMT_CTRL *GMT, double value) {
	if (!GMT->common.d.active[GMT_IN]) return false;	/* Not active */

	if (GMT->common.d.is_zero[GMT_IN]) return doubleAlmostEqualZero (0.0, value);	/* Change to NaN if value is zero */
	return doubleAlmostEqual (GMT->common.d.nan_proxy[GMT_IN], value);		/* Change to NaN if value ~nan_proxy */
}

/*! Appends one more metadata item to this OGR structure */
int gmtlib_append_ogr_item (struct GMT_CTRL *GMT, char *name, enum GMT_enum_type type, struct GMT_OGR *S) {
	if (S == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtio_append_ogr_item: No GMT_OGR structure available\n");
		return (GMT_PTR_IS_NULL);
	}
	S->n_aspatial++;
	S->name = gmt_M_memory (GMT, S->name, S->n_aspatial, char *);
	S->name[S->n_aspatial-1] = strdup (name);
	S->type = gmt_M_memory (GMT, S->type, S->n_aspatial, enum GMT_enum_type);
	S->type[S->n_aspatial-1] = type;
	return (GMT_NOERROR);
}

/*! . */
void gmtlib_write_ogr_header (FILE *fp, struct GMT_OGR *G) {
	/* Write out table-level OGR/GMT header metadata */
	unsigned int k, col;
	char *flavor = "egpw";

	fprintf (fp, "# @VGMT1.0 @G");
	if (G->geometry > GMT_IS_POLYGON) fprintf (fp, "MULTI");
	if (G->geometry == GMT_IS_POINT || G->geometry == GMT_IS_MULTIPOINT) fprintf (fp, "POINT\n");
	if (G->geometry == GMT_IS_LINESTRING || G->geometry == GMT_IS_MULTILINESTRING) fprintf (fp, "LINESTRING\n");
	if (G->geometry == GMT_IS_POLYGON || G->geometry == GMT_IS_MULTIPOLYGON) fprintf (fp, "POLYGON\n");
	fprintf (fp, "# @R%s\n", G->region);
	for (k = 0; k < 4; k++) {
		if (G->proj[k]) fprintf (fp, "# @J%c%s\n", flavor[k], G->proj[k]);
	}
	if (G->n_aspatial) {
		fprintf (fp, "# @N%s", G->name[0]);
		for (col = 1; col < G->n_aspatial; col++) fprintf (fp, "|%s", G->name[col]);
		fprintf (fp, "\n# @T%s", GMT_type[G->type[0]]);
		for (col = 1; col < G->n_aspatial; col++) fprintf (fp, "|%s", GMT_type[G->type[col]]);
		fprintf (fp, "\n");
	}
	fprintf (fp, "# FEATURE_DATA\n");
}

/*! . */
void gmtlib_write_tableheader (struct GMT_CTRL *GMT, FILE *fp, char *txt) {
	/* Output ASCII segment header; skip if mode is binary.
	 * We append a newline (\n) if not is present */

	if (!GMT->current.setting.io_header[GMT_OUT]) return;	/* No output headers requested */
	if (gmt_M_binary_header (GMT, GMT_OUT))		/* Must write a binary header */
		gmtlib_io_binary_header (GMT, fp, GMT_OUT);
	else if (!txt || !txt[0])				/* Blank header */
		fprintf (fp, "%c\n", GMT->current.setting.io_head_marker[GMT_OUT]);
	else if (txt[0] == GMT->current.setting.io_seg_marker[GMT_OUT])
		fprintf (fp, "%s\n", txt);
	else {
		fputc (GMT->current.setting.io_head_marker[GMT_OUT], fp);	/* Make sure we have # at start */
		while (strchr ("#\t ", *txt)) txt++;	/* Skip header record indicator and leading whitespace */
		fprintf (fp, " %s", txt);
		if (txt[strlen(txt)-1] != '\n') fputc ('\n', fp);	/* Make sure we have \n at end */
	}
}

void gmt_insert_tableheader (struct GMT_CTRL *GMT, struct GMT_DATATABLE *T, char *txt) {
	/* Add one more header record to the table */
	T->header = gmt_M_memory (GMT, T->header, T->n_headers+1, char *);
	T->header[T->n_headers++] = strdup (txt);
}

/* This version of fgets will check for input record truncation, that is,
 * the input record is longer than the given size.  Since calls to gmt_fgets
 * ASSUME they get a logical record, we will give a warning if truncation
 * occurs and read until we have consumed the linefeed, thus making the
 * i/o machinery ready for the next logical record.
 */

/*! . */
char *gmt_fgets (struct GMT_CTRL *GMT, char *str, int size, FILE *stream) {
	str[size-2] = '\0'; /* Set last but one record to 0 */
	if (!fgets (str, size, stream))
		return (NULL); /* Got nothing */

	/* fgets will always set str[size-1] = '\0' if more data than str can handle is found.
	 * Thus, we examine str[size-2].  If this is neither '\0' nor '\n' then we have only
	 * read a portion of a logical record that is longer than size.
	 */
	if (!(str[size-2] == '\0' || str[size-2] == '\n')) {
		/* Only got part of a record */
		int c, n = 0;
		/* Read char-by-char until newline is consumed */
		while ((c = fgetc (stream)) != '\n' && c != EOF)
			++n;
		if (c == '\n')
			/* We expect fgets to retain '\n', so add it */
			str[size-2] = '\n';
		else
			/* EOF without '\n' */
			--n;
		/* This will report wrong lengths if last line has no '\n' but we don't care */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Long input record (%d bytes) was truncated to first %d bytes!\n", size+n, size-2);
	}
	return (str);
}

/*! . */
int gmt_fclose (struct GMT_CTRL *GMT, FILE *stream) {
	int err;
	if (!stream || stream == NULL)  return (0);
	/* First skip any stream related to the three Unix i/o descriptors */
	if (stream == GMT->session.std[GMT_IN])  return (0);
	if (stream == GMT->session.std[GMT_OUT]) return (0);
	if (stream == GMT->session.std[GMT_ERR]) return (0);
	if ((size_t)stream == (size_t)-GMT->current.io.ncid) {
		/* Special treatment for netCDF files */
		nc_close (GMT->current.io.ncid);
		gmt_M_free (GMT, GMT->current.io.varid);
		gmt_M_free (GMT, GMT->current.io.add_offset);
		gmt_M_free (GMT, GMT->current.io.scale_factor);
		gmt_M_free (GMT, GMT->current.io.missing_value);
		GMT->current.io.ncols = 0;
		GMT->current.io.ncid = GMT->current.io.nvars = 0;
		GMT->current.io.ndim = GMT->current.io.nrec = 0;
		GMT->current.io.input = GMT->session.input_ascii;
		gmtlib_free_tmp_arrays (GMT);	/* Free up pre-allocated vectors */
		return (0);
	}
	/* Regular file */
	err = fclose (stream);
#ifdef HAVE_GDAL
	if (GMT->current.io.tempfile[0] && !access (GMT->current.io.tempfile, F_OK)) {
		/* Converted a shapefile to a temporary GMT/OGR file.  Now delete that file */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Remove temporary GMT/OGR file %s\n", GMT->current.io.tempfile);
		gmt_remove_file (GMT, GMT->current.io.tempfile);
		GMT->current.io.tempfile[0] = '\0';	/* Flag as no longer active */
	}
#endif
	return (err);
}

/*! . */
void gmt_skip_xy_duplicates (struct GMT_CTRL *GMT, bool mode) {
	/* Changes the status of the skip_duplicates setting */
	/* PW: This is needed as some algorithms testing if a point is
	 * inside or outside a polygon have trouble if there are
	 * duplicate vertices in the polygon.  This option can
	 * be set to true and such points will be skipped during
	 * the data reading step. Mode = true or false */
	GMT->current.io.skip_duplicates = mode;
}

/*! . */
bool gmt_is_ascii_record (struct GMT_CTRL *GMT, struct GMT_OPTION *head) {
	/* Modules that read/write data records one at the time via GMT_Get_Record and
	 * GMT_Put_Record needs to know they are writing to files or memory.  Only if
	 * we are writing to files (or stdout) AND no -b has bee specified can we truly
	 * say we are doing ASCII i/o.  Otherwise we must do binary i/o.  We check for
	 * the various situations and returns true or false accordingly. */

	if (GMT->common.b.active[GMT_IN] || GMT->common.b.active[GMT_OUT]) return (false);	/* Binary, so clearly false */
	if (GMT->current.io.ndim > 0) return (false);						/* netCDF, so clearly false */
	if (GMT->common.i.select || GMT->common.o.select) return (false);			/* Selected columns via -i and/or -o, so false */
	if (GMT->parent->external) {	/* External interface (e.g., mex, Python) so must check if writing files or memory */
		struct GMT_OPTION *current = head;
		while (current) {	/* Look for file names */
			if (current->option == GMT_OPT_INFILE || current->option == GMT_OPT_OUTFILE) {	/* File given, see if memory */
				if (gmt_M_file_is_memory (current->arg)) return (false);	/* Planning to read/write via memory */
			}
			current = current->next;
		}
	}
	return (true);	/* Should be able to treat record as an ASCII record */
}

/*! Determine if two points are "far enough apart" to constitude a data gap and thus "pen up" */
bool gmtlib_gap_detected (struct GMT_CTRL *GMT) {
	uint64_t i;

	if (!(GMT->common.g.active && GMT->current.io.has_previous_rec)) return (false);	/* Not active or on first point in a segment */
	/* Here we must determine if any or all of the selected gap criteria [see gmtlib_set_gap_param] are met */
	for (i = 0; i < GMT->common.g.n_methods; i++) {	/* Go through each criterion */
		if ((GMT->common.g.get_dist[i] (GMT, GMT->common.g.col[i]) > GMT->common.g.gap[i]) != GMT->common.g.match_all)
			return (!GMT->common.g.match_all);
	}
	return (GMT->common.g.match_all);
}

/*! . */
int gmt_ascii_output_col (struct GMT_CTRL *GMT, FILE *fp, double x, uint64_t col) {
	/* Formats x according to to output column number */
	char text[GMT_LEN256] = {""};

	gmt_ascii_format_col (GMT, text, x, GMT_OUT, col);
	return (fprintf (fp, "%s", text));
}

/*! . */
int gmtlib_set_gap (struct GMT_CTRL *GMT) {
	/* Data gaps are special since there is no multiple-segment header flagging the gap; thus next time the record is already read */
	GMT->current.io.status = GMT_IO_GAP;
	GMT->current.io.seg_no++;
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Data gap detected via -g; Segment header inserted near/at line # %" PRIu64 "\n",
	            GMT->current.io.rec_no);
	sprintf (GMT->current.io.segment_header, "Data gap detected via -g; Segment header inserted");
	return (0);
}

/*! Enable/Disable multi-segment headers for either input or output */
void gmt_set_segmentheader (struct GMT_CTRL *GMT, int direction, bool true_false) {
	GMT->current.io.multi_segments[direction] = true_false;
}

/*! Enable/Disable table headers for either input or output */
void gmt_set_tableheader (struct GMT_CTRL *GMT, int direction, bool true_false) {
	GMT->current.setting.io_header[direction] = true_false;
}

/*! . */
int gmt_z_output (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *data, char *txt) {
	int err;
	gmt_M_unused (txt);
	if (gmt_skip_output (GMT, data, n)) return (-1);	/* Record was skipped via -s[a|r] */
	err = GMT->current.io.write_item (GMT, fp, n, data);
	/* Cast below since the output functions are declared with uint64_t but cannot really exceed 4096... SHould change uint64_t to uint32_t */
	return (err ? -1 : 0);	/* Return -1 if failed, else n items written */
}

/* gmt_z_input and gmt_z_output are used in grd2xyz/xyz2grd to fascilitate reading of one-col items via the general i/o machinery
 * Despite taking uint64_t *n we KNOW that this value is 1 and hence column is always GMT_X. */
/*! . */
void * gmt_z_input (struct GMT_CTRL *GMT, FILE *fp, uint64_t *n, int *status) {
	if ((*status = GMT->current.io.read_item (GMT, fp, *n, GMT->current.io.curr_rec)) == GMT_DATA_READ_ERROR) {
		GMT->current.io.status = GMT_IO_EOF;
		return (NULL);
	}
	if (GMT->common.i.select)	/* We need to scale this single item */
		GMT->current.io.curr_rec[GMT_X] = gmt_convert_col (GMT->current.io.col[GMT_IN][GMT_X], GMT->current.io.curr_rec[GMT_X]);

	return (&GMT->current.io.record);
}

/*! . */
int gmtlib_process_binary_input (struct GMT_CTRL *GMT, uint64_t n_read) {
	/* Process a binary record to determine what kind of record it is. Return values:
	 * 0 = regular record; 1 = segment header (all NaNs); 2 = skip this record
	 * Also takes these optional actions:
	 *   1) -:  Flips x and u
	 *   2) Adjusts periodicity on longitudes
	 *   3) Scales plot dimensions from prevailing unit to internal inches.
	 *   4) Handles inverse projections if given projected coordinates.
	*/
	uint64_t col_no, n_NaN;
	bool bad_record = false, set_nan_flag = false;
	/* Here, GMT->current.io.curr_rec has been filled in by fread */

	/* Determine if this was a segment header, and if so return */
	for (col_no = n_NaN = 0; col_no < n_read; col_no++) {
		if (!gmt_M_is_dnan (GMT->current.io.curr_rec[col_no])) {	/* Clean data */
			if (col_no > 1 && gmt_input_is_nan_proxy (GMT, GMT->current.io.curr_rec[col_no]))	/* Input matched no-data setting, so change to NaN */
				GMT->current.io.curr_rec[col_no] = GMT->session.d_NaN;
			else if (GMT->common.i.select)	/* Cannot check here, done in gmtlib_bin_colselect instead when order is set */
				continue;
			else {	/* Still clean, so possibly adjust value and skip to next column */
				switch (gmt_M_type (GMT, GMT_IN, col_no)) {
					case GMT_IS_LON:	/* Must account for periodicity in 360 as per current rule */
						gmtio_adjust_periodic_lon (GMT, &GMT->current.io.curr_rec[col_no]);
						break;
					case GMT_IS_LAT:
						if (GMT->current.io.curr_rec[col_no] < -90.0 || GMT->current.io.curr_rec[col_no] > +90.0) {
							GMT_Report (GMT->parent, GMT_MSG_WARNING, "Latitude (%g) at line # %" PRIu64 " exceeds -|+ 90! - set to NaN\n", GMT->current.io.curr_rec[col_no], GMT->current.io.rec_no);
							GMT->current.io.curr_rec[col_no] = GMT->session.d_NaN;
						}
						break;
					case GMT_IS_DIMENSION:	/* Convert to internal inches */
						GMT->current.io.curr_rec[col_no] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
						break;
					default:	/* Nothing to do */
						break;
				}
				continue;
			}
		}
		/* We end up here if we found a NaN */
		if (!GMT->current.setting.io_nan_records && GMT->current.io.skip_if_NaN[col_no]) bad_record = true;	/* This field is not allowed to be NaN */
		if (GMT->current.io.skip_if_NaN[col_no]) set_nan_flag = true;
		n_NaN++;
	}

	if (!GMT->current.io.status && GMT->current.setting.n_bin_header_cols) {	/* Must have n_read NaNs to qualify as segment header (if enabled) */
		if (n_read >= GMT->current.setting.n_bin_header_cols && n_NaN == n_read) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Detected binary segment header near/at line # %" PRIu64 "\n", GMT->current.io.rec_no);
			GMT->current.io.status = GMT_IO_SEGMENT_HEADER;
			GMT->current.io.segment_header[0] = '\0';
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on "-mo" */
			GMT->current.io.seg_no++;
			GMT->current.io.data_record_number_in_seg[GMT_IN] = 0;
			return (1);	/* 1 means segment header */
		}
	}
	if (outside_in_row_range (GMT, *(GMT->common.q.rec))) return (2);	/* Record is outside desired row range of interest */

	if (outside_in_data_range (GMT, GMT->common.q.col)) return (2);	/* Must skip this record as key data value is outside desired data-range */

	if (bad_record) {
		GMT->current.io.n_bad_records++;
		if (GMT->current.io.give_report && GMT->current.io.n_bad_records == 1) {	/* Report 1st occurrence */
			GMT_Report (GMT->parent, GMT_MSG_WARNING,
			            "Encountered first invalid binary data record near/at line # %" PRIu64 "\n", GMT->current.io.rec_no);
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Likely causes:\n");
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "(1) Invalid x and/or y values, i.e. NaNs.\n");
		}
		return (2);	/* 2 means skip this record and try again */
	}
	else if (GMT->current.io.skip_duplicates && GMT->current.io.has_previous_rec) {	/* Test to determine if we should skip duplicate records with same x,y */
		if (GMT->current.io.curr_rec[GMT_X] == GMT->current.io.prev_rec[GMT_X] && GMT->current.io.curr_rec[GMT_Y] == GMT->current.io.prev_rec[GMT_Y]) return (2);	/* Yes, duplicate */
	}
	if (GMT->current.setting.io_lonlat_toggle[GMT_IN] && n_read >= 2)
		gmt_M_double_swap (GMT->current.io.curr_rec[GMT_X], GMT->current.io.curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	if (GMT->current.proj.inv_coordinates) gmtio_adjust_projected (GMT);	/* Must apply inverse projection to get lon, lat */
	GMT->current.io.has_previous_rec = true;

	if (set_nan_flag) GMT->current.io.status |= GMT_IO_NAN;
	return (0);	/* 0 means OK regular record */
}

/*! . */
int gmtlib_nc_get_att_text (struct GMT_CTRL *GMT, int ncid, int varid, char *name, char *text, size_t textlen) {
	/* This function is a replacement for nc_get_att_text that avoids overflow of text
	 * ncid, varid, name, text	: as in nc_get_att_text
	 * textlen			: maximum number of characters to copy to string text
	 */
	int status;
	size_t attlen;
	char *att = NULL;

	status = nc_inq_attlen (ncid, varid, name, &attlen);
	if (status != NC_NOERR) {
		*text = '\0';
		return status;
	}
	att = gmt_M_memory (GMT, NULL, attlen, char);
	status = nc_get_att_text (ncid, varid, name, att);
	if (status == NC_NOERR) {
		attlen = MIN (attlen, textlen-1); /* attlen does not include terminating '\0') */
		strncpy (text, att, attlen); /* Copy att to text */
		text[attlen] = '\0'; /* Terminate string */
	}
	else
		*text = '\0';
	gmt_M_free (GMT, att);
	return status;
}

/*! . */
int gmtlib_write_dataset (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, struct GMT_DATASET *D, bool use_GMT_io, int table) {
	/* Writes an entire data set to file or stream */
	unsigned int tbl, u_table, n_seg;
	bool close_file = false;
	int error, append = 0;
	int *fd = NULL;
	char file[PATH_MAX] = {""}, tmpfile[PATH_MAX] = {""}, open_mode[4] = {""}, *out_file = tmpfile;
	FILE *fp = NULL;
	struct GMT_DATASET_HIDDEN *DH = gmt_get_DD_hidden (D);
	struct GMT_DATATABLE_HIDDEN *TH = NULL;

	if (dest_type == GMT_IS_FILE && dest && ((char *)dest)[0] == '>') append = 1;	/* Want to append to existing file */
	if (use_GMT_io)	/* Use GMT->current.io.info settings to determine if input is ASCII/binary, else it defaults to ASCII */
		strcpy (open_mode, (append) ? GMT->current.io.a_mode : GMT->current.io.w_mode);
	else			/* Force ASCII mode */
		strcpy (open_mode, (append) ? "a" : "w");
	GMT->current.io.record_type[GMT_OUT] = D->type;

	/* Convert any destination type to stream */

	switch (dest_type) {
		case GMT_IS_FILE:	/* dest is a file name */
			if (!dest) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "'dest' pointer cannot be NULL here\n");
				return (GMT_ARG_IS_NULL);
			}
			strncpy (file, dest, PATH_MAX-1);
			if (DH->io_mode < GMT_WRITE_TABLE) {	/* Only need one destination */
				if ((fp = gmt_fopen (GMT, &file[append], open_mode)) == NULL) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot open file %s\n", &file[append]);
					return (GMT_ERROR_ON_FOPEN);
				}
				close_file = true;	/* We only close files we have opened here */
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Write Data Table to file %s\n", &file[append]);
			}
			break;
		case GMT_IS_STREAM:	/* Open file pointer given, just copy */
			fp = (FILE *)dest;
			if (fp == NULL) fp = GMT->session.std[GMT_OUT];	/* Default destination */
			if (fp == GMT->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output stream>");
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Write Data Table to %s\n", file);
			break;
		case GMT_IS_FDESC:		/* Open file descriptor given, just convert to file pointer */
			fd = dest;
			if (fd && (fp = fdopen (*fd, open_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot convert file descriptor %d to stream in gmtio_write_table\n", *fd);
				return (GMT_ERROR_ON_FDOPEN);
			}
			else
				close_file = true;	/* fdopen allocates memory */
			if (fd == NULL) fp = GMT->session.std[GMT_OUT];	/* Default destination */
			if (fp == GMT->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output file descriptor>");
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Write Data Table to %s\n", file);
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized source type %d in gmtio_write_table\n", dest_type);
			return (GMT_NOT_A_VALID_METHOD);
			break;
	}

	if (DH->io_mode == GMT_WRITE_OGR && gmtio_prep_ogr_output (GMT, D)) {	/* Must preprocess aspatial information and set metadata */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to prepare for OGR output formatting\n");
		if (close_file) gmt_fclose (GMT, fp);
		return (GMT_RUNTIME_ERROR);
	}
	for (tbl = 0; tbl < D->n_tables; tbl++) {
		if (table != GMT_NOTSET && (u_table = table) != tbl) continue;	/* Selected a specific table */
		GMT->current.io.data_record_number_in_tbl[GMT_OUT] = GMT->current.io.data_record_number_in_seg[GMT_OUT] = 0;
		if (DH->io_mode > GMT_WRITE_TABLE) {	/* Write segments to separate files; must pass original file name in case a template */
			if ((error = gmtio_write_table (GMT, dest, GMT_IS_FILE, D->table[tbl], use_GMT_io, DH->io_mode, 1))) {
				if (close_file) gmt_fclose (GMT, fp);
				return (error);
			}
		}
		else if (DH->io_mode == GMT_WRITE_TABLE) {	/* Must write this table a its own file */
			TH = gmt_get_DT_hidden (D->table[tbl]);
			if (TH->file[GMT_OUT])
				out_file = TH->file[GMT_OUT];
			else
				snprintf (tmpfile, PATH_MAX, file, TH->id);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Write Data Table to %s\n", out_file);
			n_seg = (unsigned int)((GMT->current.io.skip_headers_on_outout) ? 1 : D->table[tbl]->n_segments);
			if ((error = gmtio_write_table (GMT, out_file, GMT_IS_FILE, D->table[tbl], use_GMT_io, DH->io_mode, n_seg))) {
				if (close_file) gmt_fclose (GMT, fp);
				return (error);
			}
		}
		else {	/* Write to stream we set up earlier */
			n_seg = (unsigned int)((GMT->current.io.skip_headers_on_outout) ? 1 : D->n_segments);
			if ((error = gmtio_write_table (GMT, fp, GMT_IS_STREAM, D->table[tbl], use_GMT_io, DH->io_mode, n_seg))) {
				if (close_file) gmt_fclose (GMT, fp);
				return (error);
			}
		}
	}

	if (close_file) gmt_fclose (GMT, fp);

	return (0);	/* OK status */
}

/*! . */
bool gmt_input_is_bin (struct GMT_CTRL *GMT, const char *filename) {
	FILE *fd = NULL;

	if (GMT->common.b.active[GMT_IN]) return true;	/* Clearly a binary file */
	if (gmt_M_compat_check (GMT, 4) && GMT->common.b.varnames[0]) return true;	/* Definitely netCDF */
	if (!filename)  return false;			/* Cannot be netCDF without a filename */
	if (strchr (filename, '?'))  return true;	/* Definitely netCDF */
	/* Might still be netCDF; try to open it */
	fd = gmt_nc_fopen (GMT, filename, "r");
	if (fd) {	/* Yes, it was */
		gmt_fclose (GMT, fd);
		return true;
	}
	else return false;	/* No, must be ASCII */
}

/*! . */
FILE * gmt_fopen (struct GMT_CTRL *GMT, const char *filename, const char *mode) {
	char path[PATH_MAX], *c = NULL;
	FILE *fd = NULL;
	unsigned int first = 0;
	if (gmt_M_file_is_cache (filename)) {	/* Must be a cache file */
		first = gmt_download_file_if_not_found (GMT, filename, 0);
	}

	if (mode[0] != 'r')	/* Open file for writing (so cannot be netCDF) */
		return (fopen (&filename[first], mode));
	else if (GMT->common.b.active[GMT_IN]) {	/* Definitely not netCDF */
		if ((c = gmt_getdatapath (GMT, &filename[first], path, R_OK)) == NULL) return NULL;
		return (fopen (c, mode));
	}
	else if (gmt_M_compat_check (GMT, 4) && GMT->common.b.varnames[0])	/* Definitely netCDF */
		return (gmt_nc_fopen (GMT, &filename[first], mode));
	else if (strchr (&filename[first], '?'))	/* Definitely netCDF */
		return (gmt_nc_fopen (GMT, &filename[first], mode));
#ifdef WIN32
	else if (!strcmp (&filename[first], "NUL"))	/* Special case of /dev/null under Windows */
#else
	else if (!strcmp (&filename[first], "/dev/null"))	/* The Unix null device; catch here to avoid gmt_nc_fopen */
#endif
	{
		if ((c = gmt_getdatapath(GMT, &filename[first], path, R_OK)) == NULL) return fd;
		return (fopen (c, mode));
	}
	else {	/* Maybe netCDF */
		fd = gmt_nc_fopen (GMT, &filename[first], mode);
		if (!fd) {	/* No, was not a netCDF file */
			if ((c = gmt_getdatapath (GMT, &filename[first], path, R_OK)) != NULL) {	/* Got the file path */
#ifdef HAVE_GDAL
				char *ext = gmt_get_ext (c);	/* Get pointer to extension (or NULL if no extension) */
				if (ext && mode[0] == 'r' && !strncmp (ext, "shp", 3U)) {	/* Got a shapefile for reading */
					/* We will do a system call to ogr2ogr in order to read the shapefile */
					char cmd[GMT_BUFSIZ+GMT_LEN256] = {""};
					int error = 0;
					if (GMT->parent->tmp_dir)	/* Make unique file in temp dir */
						snprintf (GMT->current.io.tempfile, PATH_MAX, "%s/gmt_ogr_%d.gmt", GMT->parent->tmp_dir, (int)getpid());
					else	/* Make unique file in current dir */
						snprintf (GMT->current.io.tempfile, PATH_MAX, "gmt_ogr_%d.gmt", (int)getpid());
					GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Convert %s to GMT/OGR file %s\n", c, GMT->current.io.tempfile);
#if GDAL_VERSION_MAJOR >= 2
					snprintf (cmd, GMT_BUFSIZ+GMT_LEN256, "ogr2ogr -mapFieldType Integer64=Integer -f \"OGR_GMT\" %s %s", GMT->current.io.tempfile, c);
#else
					snprintf (cmd, GMT_BUFSIZ+GMT_LEN256, "ogr2ogr -f \"GMT\" %s %s", GMT->current.io.tempfile, c);
#endif
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Running %s\n", cmd);
					if ((error = system (cmd))) {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "System call [%s] FAILED with error %d.\n", cmd, error);
						return NULL;
					}
					sprintf (GMT->current.io.filename[GMT_IN], "%s <converted from %s via ogr2ogr>", GMT->current.io.tempfile, c);
					c = GMT->current.io.tempfile;	/* Open this temporary instead */
				}
#endif
                		fd = fopen (c, mode);
			}
		}
		return (fd);
	}
}

/* Table I/O routines for ASCII and binary io */

/*! Write verbose message about binary record i/o format */
int gmtlib_io_banner (struct GMT_CTRL *GMT, unsigned int direction) {
	static const char *gmt_direction[2] = {"Input", "Output"};
	char *message = NULL, skip[GMT_LEN64] = {""};
	char *letter = "-cuhHiIlLfditTn", s[2] = {0, 0};	/* letter order matches the type order in GMT_enum_type */
	uint64_t col;
	uint64_t n_bytes;
	size_t alloc = GMT_LEN256, m_len = 0, len;

#if 0
	if (GMT->current.setting.verbose < GMT_MSG_WARNING) return GMT_OK;	/* Not in verbose mode anyway */
#endif
	if (!GMT->common.b.active[direction]) return GMT_OK;	/* Not using binary i/o */
	if (GMT->common.b.type[direction] == 0) GMT->common.b.type[direction] = (direction == GMT_IN) ? 'd' : letter[GMT->current.setting.export_type];	/* Only happens for external interfaces */
	if (GMT->common.b.ncol[direction] == 0) {		/* Number of columns not set yet - delay message */
		if (direction == GMT_OUT) GMT->common.b.o_delay = true;
		return GMT_OK;
	}
	if (direction == GMT_IN && GMT->common.i.select && GMT->common.b.ncol[GMT_IN] < GMT->common.i.n_actual_cols) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Number of input columns set by -i exceeds those set by -bi!\n");
		GMT_exit (GMT, GMT_DIM_TOO_LARGE); return GMT_DIM_TOO_LARGE;
	}
	if (direction == GMT_OUT && GMT->common.o.select && GMT->common.b.ncol[GMT_OUT] < GMT->common.o.n_cols) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Number of output columns set by -o exceeds those set by -bo!\n");
		GMT_exit (GMT, GMT_DIM_TOO_LARGE); return GMT_DIM_TOO_LARGE;
	}
	message = gmt_M_memory (GMT, NULL, alloc, char);
	for (col = 0; col < GMT->common.b.ncol[direction]; col++) {	/* For each binary column of data */
		if (GMT->current.io.fmt[direction][col].skip < 0) {	/* Must skip n_bytes BEFORE reading this column */
			n_bytes = -GMT->current.io.fmt[direction][col].skip;
			snprintf (skip, GMT_LEN64, "%" PRIu64 "x", n_bytes);
			len = strlen (skip);
			if ((m_len+len) >= alloc) {
				alloc += GMT_LEN256;
				message = gmt_M_memory (GMT, message, alloc, char);
			}
			strcat (message, skip);
			m_len += len;
		}
		if (GMT->current.io.fmt[direction][col].type == 0) {	/* Still not set, use the default type */
			GMT->current.io.fmt[direction][col].type = gmt_get_io_type (GMT, GMT->common.b.type[direction]);
			GMT->current.io.fmt[direction][col].io   = gmtlib_get_io_ptr (GMT, direction, GMT->common.b.swab[direction],
			                                                              GMT->common.b.type[direction]);
		}
		s[0] = letter[GMT->current.io.fmt[direction][col].type];	/* Get data type code... */
		if ((m_len+1) >= alloc) {
			alloc += GMT_LEN256;
			message = gmt_M_memory (GMT, message, alloc, char);
		}
		m_len++;
		strcat (message, s);					/* ...and append to message */
		if (GMT->current.io.fmt[direction][col].skip > 0) {	/* Must skip n_bytes AFTER reading this column */
			n_bytes = GMT->current.io.fmt[direction][col].skip;
			snprintf (skip, GMT_LEN64, "%" PRIu64 "x", n_bytes);
			len = strlen (skip);
			if ((m_len+len) >= alloc) {
				alloc += GMT_LEN256;
				message = gmt_M_memory (GMT, message, alloc, char);
			}
			strcat (message, skip);
			m_len += len;
		}
	}
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s %d columns via binary records using format %s\n",
	            gmt_direction[direction], GMT->common.b.ncol[direction], message);
	gmt_M_free (GMT, message);
	return GMT_OK;
}

/*! . */
uint64_t gmt_get_cols (struct GMT_CTRL *GMT, unsigned int direction) {
	/* Return the number of columns currently in play in this direction.
	 * This can be complicated.. For BINARY data:
	 * On INPUT, a binary file has a known number of columns set via -bi. Internally,
	 * we read in all those columns into gmt_current_rec.  However, if -i is used then we shuffle
	 * the read values into the positions implied by -i. Further processing
	 * is thus concerned with the possibly smaller number of columns than in the file.
	 * So: n_cols is ncol[GMT_IN], but if -i was set then it is less (i.n_cols).
	 *
	 * On OUTPUT, a binary file has its number of columns set via -bo. Internally,
	 * the number of columns we hold in our array might be much larger if -o is used,
	 * as we then only write out those columns that are requested.
	 * So: n_cols is ncol[GMT_OUT], but if -o it is same as input (see above for that!)
	 *
	 * For ASCII data it is the same except for on output, where we return the output
	 * cols as set.
	 */
	uint64_t n_cols;
	if (! (direction == GMT_IN || direction == GMT_OUT)) return (GMT_NOT_A_VALID_DIRECTION);

	if (direction == GMT_IN) {
		n_cols = (GMT->common.i.select) ? GMT->common.i.n_cols : GMT->common.b.ncol[GMT_IN];
	}
	else {
		uint64_t in_n_cols = (GMT->common.i.select) ? GMT->common.i.n_cols : GMT->common.b.ncol[GMT_IN];
		if (GMT->common.b.active[GMT_OUT])
			n_cols = (GMT->common.o.select) ? in_n_cols : GMT->common.b.ncol[GMT_OUT];
		else
			n_cols = GMT->common.b.ncol[GMT_OUT];
	}
	return (n_cols);
}

/*! . */
int gmt_set_cols (struct GMT_CTRL *GMT, unsigned int direction, uint64_t expected) {
	/* Initializes the internal GMT->common.b.ncol[] settings.
	 * direction is either GMT_IN or GMT_OUT.
	 * expected is the expected or known number of columns.  Use 0 if not known.
	 * For binary input or output the number of columns must be specified.
	 * For ASCII output the number of columns must also be specified.
	 * For ASCII input the i/o machinery will set this automatically so expected is ignored.
	 * Programs that need to read an input record in order to determine how
	 * many columns on output should call this function after returning the
	 * first data record; otherwise, call it before registering the resource.
	 */
	static char *mode[2] = {"input", "output"};

	if (! (direction == GMT_IN || direction == GMT_OUT)) return (GMT_NOT_A_VALID_DIRECTION);

	if (direction == GMT_IN && GMT->common.b.ncol[direction]) {	/* Some more conditions when input n columns have been set (by -bi or by reading another file) */
		if (GMT->common.b.ncol[direction] == expected) return (GMT_OK);	/* Already set to this value */
		if (expected == 0) return (GMT_OK);	/* Nothing to do yet */
		if (GMT->common.b.active[direction]) return (GMT_OK);	/* Already set once by -bi so cannot change that here */
		/* If we get here we are ASCII input and we want to change the expected columns only */
	}

	if (expected == 0 && (direction == GMT_OUT || GMT->common.b.active[direction])) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Number of numerical %s columns has been set to 0\n", mode[direction]);
	}
	/* Here we may set the number of data columns */
	if (GMT->common.b.active[direction]) {	/* Must set uninitialized input/output pointers */
		uint64_t col;
		char type = (GMT->common.b.type[direction]) ? GMT->common.b.type[direction] : 'd';
		for (col = 0; col < expected; col++) {
			if (!GMT->current.io.fmt[direction][col].io) {
				GMT->current.io.fmt[direction][col].io = gmtlib_get_io_ptr (GMT, direction, GMT->common.b.swab[direction], type);
				GMT->current.io.fmt[direction][col].type = gmt_get_io_type (GMT, type);
			}
		}
		GMT->common.b.ncol[direction] = expected;
	}
	else {	/* ascii */
		GMT->common.b.ncol[direction] = (direction == GMT_IN && expected == 0) ? GMT_MAX_COLUMNS : expected;
		if (direction == GMT_IN) GMT->current.io.max_cols_to_read = (unsigned int)expected;
	}
	if (direction == GMT_OUT && GMT->common.b.o_delay) {	/* Issue delayed message (see gmtlib_io_banner) */
		gmtlib_io_banner (GMT, direction);
		GMT->common.b.o_delay = false;
	}
	if (direction == GMT_IN && expected && GMT->common.i.select && GMT->common.i.n_actual_cols > expected)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Number of %s columns required [%" PRIu64 "] is less that implied by -i [%" PRIu64 "]\n",
		            mode[GMT_IN], expected, GMT->common.i.n_actual_cols);
	return (GMT_OK);
}

/*! . */
char *gmtlib_getuserpath (struct GMT_CTRL *GMT, const char *stem, char *path) {
	/* stem is the name of the file, e.g., gmt.conf
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in the temporary directory (if defined),
	 * current directory, home directory and $GMT->session.USERDIR (default ~/.gmt[/cache])
	 */

	/* If a full path is given, we only look for that file directly */

#ifdef WIN32
	if (stem[0] == '/' || stem[1] == ':')
#else
	if (stem[0] == '/')
#endif
	{
		if (!access (stem, R_OK)) return (strcpy (path, stem));	/* Yes, found it */
		return (NULL);	/* No file found, give up */
	}

	/* In isolation mode (when GMT->session.TMPDIR is defined), we first look there */

	if (GMT->session.TMPDIR) {
		sprintf (path, "%s/%s", GMT->session.TMPDIR, stem);
		if (!access (path, R_OK)) return (path);
	}

	/* Then look in the current working directory */


	if (!access (stem, R_OK)) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", stem);
		return (strcpy (path, stem));	/* Yes, found it */
	}

	/* If still not found, see if there is a file in the GMT_{HOME,USER}DIR directories */

	if (GMT->session.HOMEDIR) {
		sprintf (path, "%s/%s", GMT->session.HOMEDIR, stem);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Look for file %s\n", path);
		if (!access (path, R_OK)) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", path);
			return (path);
		}
	}
	if (GMT->session.USERDIR) {
		if (strstr (stem, ".SRTMGL1.")) /* Special srtm1 subdirs */
			sprintf (path, "%s/server/srtm1/%s", GMT->session.USERDIR, stem);
		else if (strstr (stem, ".SRTMGL3.")) /* Special srtm3 subdirs */
			sprintf (path, "%s/server/srtm3/%s", GMT->session.USERDIR, stem);
		else {
			sprintf (path, "%s/%s", GMT->session.USERDIR, stem);
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Look for file %s\n", path);
			if (!access (path, R_OK)) {
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", path);
				return (path);
			}
			sprintf (path, "%s/server/%s", GMT->session.USERDIR, stem);
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Look for file %s\n", path);
			if (!access (path, R_OK)) {
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", path);
				return (path);
			}
		}
	}
	if (GMT->session.CACHEDIR) {
		sprintf (path, "%s/%s", GMT->session.CACHEDIR, stem);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Look for file %s\n", path);
		if (!access (path, R_OK)) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", path);
			return (path);
		}
	}

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Could not find file %s\n", stem);
	return (NULL);	/* No file found, give up */
}

/*! . */
char *gmt_getdatapath (struct GMT_CTRL *GMT, const char *stem, char *path, int mode) {
	/* stem is the name of the file, e.g., grid.img
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in current directory and $GMT_{USER,DATA}DIR
	 * If the dir ends in / we traverse recursively [not under Windows].
	 */
	unsigned int d, pos;
	size_t L;
	bool found;
	char *udir[6] = {GMT->session.USERDIR, GMT->session.DATADIR, GMT->session.CACHEDIR, NULL, NULL, NULL}, dir[PATH_MAX];
	char path_separator[2] = {',', '\0'}, serverdir[PATH_MAX] = {""}, srtm1dir[PATH_MAX] = {""}, srtm3dir[PATH_MAX] = {""};
#ifdef HAVE_DIRENT_H_
	size_t N;
#endif /* HAVE_DIRENT_H_ */
	bool gmt_file_is_readable (struct GMT_CTRL *GMT, char *path);

	/* First look in the current working directory */

	if (!access (stem, F_OK)) {	/* Yes, found it */
		if (mode == F_OK || gmt_file_is_readable (GMT, (char *)stem)) {	/* Yes, found it or can read it */
			strcpy (path, stem);
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", path);
			return (path);
		}
		return (NULL);	/* Cannot read, give up */
	}

	/* If we got here and a full path is given, we give up ... unless it is one of those /vsi.../ files */
	if (stem[0] == '/') {
#ifdef HAVE_GDAL
		if (gmtlib_check_url_name ((char *)stem))
			return ((char *)stem);			/* With GDAL all the /vsi-stuff is given existence credit */
		else
			return (NULL);
#else
		return (NULL);
#endif
	}

#ifdef WIN32
	if (stem[1] == ':') return (NULL);
#endif

	/* Not found, see if there is a file in the GMT_{USER,DATA}DIR directories [if set] */

	snprintf (serverdir, PATH_MAX, "%s/server", GMT->session.USERDIR);	udir[3] = serverdir;
	snprintf (srtm1dir, PATH_MAX, "%s/server/srtm1", GMT->session.USERDIR);	udir[4] = srtm1dir;
	snprintf (srtm3dir, PATH_MAX, "%s/server/srtm3", GMT->session.USERDIR);	udir[5] = srtm3dir;

	for (d = 0; d < 6; d++) {	/* Loop over USER, DATA and CACHE dirs */
		if (!udir[d]) continue;	/* This directory was not set */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Look for file %s in %s\n", stem, udir[d]);
		found = false;
		pos = 0;
		while (!found && (gmt_strtok (udir[d], path_separator, &pos, dir))) {
			L = strlen (dir);

#ifdef HAVE_DIRENT_H_
			if (dir[L-1] == '/' || (gmt_M_compat_check (GMT, 4) && dir[L-1] == '*')) {	/* Must search recursively from this dir */
				N = (dir[L-1] == '/') ? L - 1 : L - 2;
				strncpy (path, dir, N);	path[N] = 0;
				found = gmtio_traverse_dir (stem, path);
			}
			else {
#endif /* HAVE_DIRENT_H_ */
				sprintf (path, "%s/%s", dir, stem);
				found = (!access (path, F_OK));
#ifdef HAVE_DIRENT_H_
			}
#endif /* HAVE_DIRENT_H_ */
		}
		if (found && gmt_file_is_readable (GMT, path)) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found file %s\n", path);
			return (path);	/* Yes, can read it */
		}
	}

	return (NULL);	/* No file found, give up */
}

/*! . */
char *gmt_getsharepath (struct GMT_CTRL *GMT, const char *subdir, const char *stem, const char *suffix, char *path, int mode) {
	/* stem is the prefix of the file, e.g., gmt_cpt for gmt_cpt.conf
	 * subdir is an optional subdirectory name in the $GMT_SHAREDIR directory.
	 * suffix is an optional suffix to append to name
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in current directory, $GMT_USERDIR (default ~/.gmt) and $GMT_SHAREDIR/subdir
	 */

	/* First look in the current working directory */

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT: 0. Will try to find subdir=%s stem = %s suffix=%s\n", subdir, stem, suffix);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT: 1. gmt_getsharepath trying current dir\n");
	sprintf (path, "%s%s", stem, suffix);
	if (!access (path, mode)) return (path);	/* Yes, found it in current directory */

	/* Do not continue when full pathname is given */

	if (stem[0] == '/') return (NULL);
#ifdef WIN32
	if (stem[0] && stem[1] == ':') return (NULL);
#endif

	/* Not found, see if there is a file in the user's GMT_USERDIR (~/.gmt) directory */

	if (GMT->session.USERDIR) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT: 2. gmt_getsharepath trying USERDIR %s\n", GMT->session.USERDIR);
		/* Try to get file from $GMT_USERDIR */
		sprintf (path, "%s/%s%s", GMT->session.USERDIR, stem, suffix);
		if (!access (path, mode)) return (path);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT: 3. gmt_getsharepath trying USERDIR subdir %s/%s\n", GMT->session.USERDIR, subdir);
		/* Try to get file from $GMT_USERDIR/subdir */
		sprintf (path, "%s/%s/%s%s", GMT->session.USERDIR, subdir, stem, suffix);
		if (!access (path, mode)) return (path);
	}

	/* Try to get file from $GMT_SHAREDIR/subdir */

	if (subdir) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT: 4. gmt_getsharepath trying SHAREDIR subdir %s/%s\n", GMT->session.SHAREDIR, subdir);
		sprintf (path, "%s/%s/%s%s", GMT->session.SHAREDIR, subdir, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GMT: 5. gmt_getsharepath failed\n");
	return (NULL);	/* No file found, give up */
}

/*! Copy num characters from a char string without triggering a Coverity issue about
    "Buffer not null terminated" */
char *gmt_strncpy (char *dest, const char *source, size_t num) {
	/* This function is meant to be used only when Coverity annoying and insistently acuses of
	   "Buffer not null terminated". We have several of those cases where strings are not NULL terminated. */
	strncpy(dest, source, num-1);
	dest[num-1] = source[num-1];
	return dest;
}

/*! Like access but also checks the GMT_*DIR places */
int gmt_access (struct GMT_CTRL *GMT, const char* filename, int mode) {
	char file[PATH_MAX] = {""}, *c = NULL;
	unsigned int first = 0;
	int err;
	struct stat S;

	if (!filename || !filename[0]) return (-1);		/* No file given */
	if (gmt_M_file_is_memory (filename)) return (0);	/* Memory location always exists */
	if (gmt_M_file_is_cache (filename))			/* Must be a cache file */
		first = gmt_download_file_if_not_found (GMT, filename, 0);
	file[0] = '\0';		/* 'Initialize' it so we can test if it's still 'empty' after the sscanf below */
	sscanf (&filename[first], "%[^=?]", file);	/* Exclude netcdf 3-D grid extensions to make sure we get a valid file name */
	if (file[0] == '\0')
		return (-1);		/* It happens for example when parsing grdmath args and it finds an isolated  "=" */

	if ((c = gmtlib_file_unitscale (file))) c[0] = '\0';	/* Chop off any x/u unit specification */
	else if ((c = strchr (file, '+')) && strchr ("hons", c[1])) c[0] = '\0';	/* Chop off any +h hinge setting or any z-scaling specification */
	if (mode == W_OK)
		return (access (file, mode));	/* When writing, only look in current directory */
	err = stat (file, &S);	/* Stat the argument */
	if (!err && S_ISDIR (S.st_mode))	/* Path exists, but it is a directory */
		return (-1);
	if (mode == R_OK || mode == F_OK) {	/* Look in special directories when reading or just checking for existence */
		char path[PATH_MAX] = {""};
		if (gmt_M_file_is_remotedata (filename) && !strstr (filename, ".grd"))	/* A remote @earth_relief_xxm|s grid without extension */
			strcat (file, ".grd");	/* Must supply the .grd */
		return (gmt_getdatapath (GMT, file, path, mode) ? 0 : -1);
	}
	/* If we get here then mode is bad (X_OK)? */
	GMT_Report (GMT->parent, GMT_MSG_ERROR, "GMT: Bad mode (%d) passed to gmt_access\n", mode);
	return (-1);
}

/*! . */
int gmt_get_ogr_id (struct GMT_OGR *G, char *name) {
	unsigned int k;
	for (k = 0; k < G->n_aspatial; k++) if (!strcmp (name, G->name[k])) return (k);
	return (GMT_NOTSET);
}

/*! . */
char *gmtlib_trim_segheader (struct GMT_CTRL *GMT, char *line) {
	/* Trim trailing junk and return pointer to first non-space/tab/> part of segment header
	 * Do not try to free the returned pointer!
	 */
	gmt_strstrip (line, false); /* Strip trailing whitespace */
	/* Skip over leading whitespace and segment marker */
	while (*line && (isspace(*line) || *line == GMT->current.setting.io_seg_marker[GMT_IN]))
		++line;
	/* Return header string */
	return (line);
}

/*! . */
unsigned int gmt_is_segment_header (struct GMT_CTRL *GMT, char *line)
{	/* Returns 1 if this record is a GMT segment header;
	 * Returns 2 if this record is a segment breaker;
	 * Otherwise returns 0 */
	if (GMT->current.setting.io_blankline[GMT_IN] && gmt_is_a_blank_line (line)) return (2);	/* Treat blank line as segment break */
	if (GMT->current.setting.io_nanline[GMT_IN] && gmtio_is_a_NaN_line (GMT, line)) return (2);		/* Treat NaN-records as segment break */
	if (line[0] == GMT->current.setting.io_seg_marker[GMT_IN]) return (1);	/* Got a regular GMT segment header */
	return (0);	/* Not a segment header */
}

/*! . */
void * gmtio_ascii_textinput (struct GMT_CTRL *GMT, FILE *fp, uint64_t *n, int *status) {
	bool more = true;
	char line[GMT_BUFSIZ] = {""}, *p = NULL;

	/* gmtio_ascii_textinput will read one text line and return it, setting
	 * header or segment flags in the process.
	 */

	while (more) {
		/* First read until we get a non-blank, non-comment record, or reach EOF */

		GMT->current.io.rec_no++;		/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		GMT->current.io.rec_in_tbl_no++;	/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		while ((p = gmt_fgets (GMT, line, GMT_BUFSIZ, fp)) && gmtio_ogr_parser (GMT, line)) {	/* Exits loop when we successfully have read a data record */
			GMT->current.io.rec_no++;		/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
			GMT->current.io.rec_in_tbl_no++;	/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		}
		/* Here we come once any OGR headers have been parsed and we have a real (non-OGR header) record */
		if (GMT->current.setting.io_header[GMT_IN] && GMT->current.io.rec_in_tbl_no <= GMT->current.setting.io_n_header_items) {	/* Must treat first io_n_header_items as headers */
			if (GMT->common.h.mode == GMT_COMMENT_IS_RESET) continue;	/* Simplest way to replace headers on output is to ignore them on input */
			gmtio_set_current_record (GMT, line);
			GMT->current.io.status = GMT_IO_TABLE_HEADER;
			*status = 0;
			return (NULL);
		}
		/* Here we are done with any header records implied by -h */
		if (!p) {	/* Ran out of records */
			GMT->current.io.status = GMT_IO_EOF;
			*n = 0ULL;
			*status = -1;
			return (NULL);
		}
		if (line[0] == GMT->current.setting.io_head_marker[GMT_IN]) {	/* Got a file header, take action and return */
			if (GMT->common.h.mode == GMT_COMMENT_IS_RESET) continue;	/* Simplest way to replace headers on output is to ignore them on input */
			gmtio_set_current_record (GMT, line);
			GMT->current.io.status = GMT_IO_TABLE_HEADER;
			*n = 1ULL;
			*status = 0;
			return (NULL);
		}

		if (line[0] == GMT->current.setting.io_seg_marker[GMT_IN]) {	/* Got a segment header, take action and return */
			GMT->current.io.status = GMT_IO_SEGMENT_HEADER;
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
			GMT->current.io.seg_no++;
			/* Just save the header content, not the marker and leading whitespace */
			strncpy (GMT->current.io.segment_header, gmtlib_trim_segheader (GMT, line), GMT_BUFSIZ-1);
			*n = 1ULL;
			*status = 0;
			return (NULL);
		}
		if (!(GMT->common.e.active && gmt_skip_record (GMT, GMT->common.e.select, line)))	/* Fail a grep test */
			more = false;	/* Got a valid record */
	}

	/* Normal data record */

	/* First chop off trailing whitespace and commas */

	gmt_strstrip (line, false); /* Eliminate DOS endings and trailing white space */

	gmtio_set_current_record (GMT, line);
	strcpy (GMT->current.io.curr_trailing_text, GMT->current.io.curr_text);
	GMT->current.io.record.text = GMT->current.io.curr_trailing_text;

	GMT->current.io.status = GMT_IO_DATA_RECORD;
	GMT->current.io.data_record_number_in_set[GMT_IN]++;	/* Got a valid text record */
	*n = 1ULL;			/* We always return 1 item as there are no columns */
	*status = 1;
	return (&GMT->current.io.record);
}

/*! Returns true if we should skip this line (because it is blank) */
bool gmt_is_a_blank_line (char *line) {
	unsigned int i = 0;
	while (line[i] && (line[i] == ' ' || line[i] == '\t')) i++;	/* Wind past leading whitespace or tabs */
	if (line[i] == '\n' || line[i] == '\r' || line[i] == '\0') return (true);
	return (false);
}

/*! . */
uint64_t gmtlib_bin_colselect (struct GMT_CTRL *GMT) {
	/* When -i<cols> is used we must pull out and reset the current record */
	uint64_t col, order;
	static double tmp[GMT_BUFSIZ];
	struct GMT_COL_INFO *S = NULL;
	for (col = 0; col < GMT->common.i.n_cols; col++) {
		S = &(GMT->current.io.col[GMT_IN][col]);
		order = S->order;
		tmp[order] = gmt_convert_col (GMT->current.io.col[GMT_IN][col], GMT->current.io.curr_rec[S->col]);
		switch (gmt_M_type (GMT, GMT_IN, order)) {
			case GMT_IS_LON:	/* Must account for periodicity in 360 as per current rule */
				gmtio_adjust_periodic_lon (GMT, &tmp[order]);
				break;
			case GMT_IS_LAT:
				if (tmp[order] < -90.0 || tmp[order] > +90.0) {
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "Latitude (%g) at line # %" PRIu64 " exceeds -|+ 90! - set to NaN\n", tmp[order], GMT->current.io.rec_no);
					tmp[S->order] = GMT->session.d_NaN;
				}
				break;
			case GMT_IS_DIMENSION:	/* Convert to internal inches */
				tmp[order] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				break;
			default:	/* Nothing to do */
				break;
		}
	}
	gmt_M_memcpy (GMT->current.io.curr_rec, tmp, GMT->common.i.n_cols, double);
	return (GMT->common.i.n_cols);
}

/*! . */
bool gmt_skip_output (struct GMT_CTRL *GMT, double *cols, uint64_t n_cols) {
	/* Consult the -s[<cols>][a|r] setting and the cols values to determine if this record should be output */
	uint64_t c, n_nan;

	if (n_cols > GMT_MAX_COLUMNS) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Number of output data columns (%d) exceeds limit (GMT_MAX_COLUMNS = %d)\n", n_cols, GMT_MAX_COLUMNS);
		return (true);	/* Skip record since we cannot access that many columns */
	}
	GMT->current.io.data_record_number_in_set[GMT_OUT]++;
	GMT->current.io.data_record_number_in_tbl[GMT_OUT]++;
	GMT->current.io.data_record_number_in_seg[GMT_OUT]++;
	if (GMT->common.q.mode == GMT_RANGE_ROW_OUT) {
		if (outside_out_row_range (GMT, *(GMT->common.q.rec))) return (true);		/* Not in a valid row range for output */
	}
	else if (GMT->common.q.mode == GMT_RANGE_DATA_OUT) {
		if (outside_out_data_range (GMT, GMT->common.q.col, cols)) return (true);	/* Not in a valid data range for output */
	}
	if (GMT->current.setting.io_nan_mode == GMT_IO_NAN_OK) return (false);			/* Normal case; output the record */
	if (GMT->current.setting.io_nan_mode == GMT_IO_NAN_ONE) {	/* -sa: Skip records if any NaNs are found */
		for (c = 0; c < n_cols; c++) if (gmt_M_is_dnan (cols[c])) return (true);	/* Found a NaN so we skip */
		return (false);	/* No NaNs, output record */
	}
	for (c = n_nan = 0; c < GMT->current.io.io_nan_ncols; c++) {			/* Check each of the specified columns set via -s */
		if (GMT->current.io.io_nan_col[c] >= n_cols) continue;			/* Input record does not have this column */
		if (gmt_M_is_dnan (cols[GMT->current.io.io_nan_col[c]])) n_nan++;		/* Count the nan columns found */
	}
	if (n_nan < GMT->current.io.io_nan_ncols  && GMT->current.setting.io_nan_mode == GMT_IO_NAN_KEEP) return (true);	/* Skip records if -sr and not enough NaNs found */
	if (n_nan == GMT->current.io.io_nan_ncols && GMT->current.setting.io_nan_mode == GMT_IO_NAN_SKIP) return (true);	/* Skip records if -s and NaNs in specified columns */
	return (false);	/* No match, output record */
}

/*! . */
void gmtlib_set_bin_io (struct GMT_CTRL *GMT) {
	/* Make sure we point to binary input functions after processing -b option */
	if (GMT->common.b.active[GMT_IN]) {
		GMT->current.io.input = &gmtio_bin_input;
		strcpy (GMT->current.io.r_mode, "rb");
	}
	if (GMT->common.b.active[GMT_OUT]) {
		GMT->current.io.output = &gmtio_bin_output;
		strcpy (GMT->current.io.w_mode, "wb");
		strcpy (GMT->current.io.a_mode, "ab+");
	}
}

/*! . */
void gmt_format_abstime_output (struct GMT_CTRL *GMT, double dt, char *text) {
	char date[GMT_LEN16] = {""}, tclock[GMT_LEN16] = {""};

	gmt_format_calendar (GMT, date, tclock, &GMT->current.io.date_output, &GMT->current.io.clock_output, false, 1, dt);
	if (date[0] == '\0')	/* No date wanted hence don't use T */
		sprintf (text, "%s", tclock);
	else if (tclock[0] == '\0')	/* No clock wanted hence don't use T */
		sprintf (text, "%s", date);
	else	/* ISO format */
		sprintf (text, "%sT%s", date, tclock);
}

#define N_T_UNITS 5
/* Using average year and month here for durations */
static double tcount[N_T_UNITS] = {31557600.0, 2629800.0, 86400.0, 3600.0, 60.0};
static char *tformat[N_T_UNITS] = {"%uY","%2.2uM", "%2.2uD", "%2.2uH", "%2.2uM"};

/*! . */
void gmt_format_duration_output (struct GMT_CTRL *GMT, double dt, char *text) {
	unsigned int k, k0 = N_T_UNITS, n[N_T_UNITS];
	char item[GMT_LEN16] = {""};
	double sec;	/* Get seconds */
	text[0] = 'P';	/* The ISO duration designator */
	if (dt == 0.0) {	/* Zero duration */
		text[1] = '0';	text[2] = 0;
		return;
	}
	sec = dt * GMT->current.setting.time_system.scale;	/* Get seconds */
	for (k = 0; k < N_T_UNITS; k++) {	/* Add up number of time units */
		if ((n[k] = (unsigned int)floor (sec / tcount[k]))) {
			sec -= n[k] * tcount[k];
			if (k0 == N_T_UNITS) k0 = k;
		}
	}
	while (k0 < N_T_UNITS) {	/* For remaining units... */
		snprintf (item, GMT_LEN16, tformat[k0], n[k0]);
		strcat (text, item);
		k0++;
	}
	if (sec > 0.0) {	/* Also add seconds */
		if (GMT->current.io.clock_output.n_sec_decimals)
			snprintf (item, GMT_LEN16, "%0*.*fS", GMT->current.io.clock_output.n_sec_decimals+3, GMT->current.io.clock_output.n_sec_decimals, sec);
		else
			snprintf (item, GMT_LEN16, "%2.2uS", urint (sec));
		strcat (text, item);
	}
}

/*! . */
void gmt_ascii_format_col (struct GMT_CTRL *GMT, char *text, double x, unsigned int direction, uint64_t col) {
	/* Format based on column position in in or out direction */
	if (gmt_M_is_dnan (x)) {	/* NaN, just write it as a string */
		sprintf (text, "NaN");
		return;
	}
	switch (gmt_M_type (GMT, direction, col)) {
		case GMT_IS_LON:
			gmtio_format_geo_output (GMT, false, x, text);
			break;
		case GMT_IS_LAT:
			gmtio_format_geo_output (GMT, true, x, text);
			break;
		case GMT_IS_ABSTIME:
			gmt_format_abstime_output (GMT, x, text);
			break;
		case GMT_IS_DURATION:
			gmt_format_duration_output (GMT, x, text);
			break;
		default:	/* Floating point */
			if (GMT->current.io.o_format[col])	/* Specific to this column */
				sprintf (text, GMT->current.io.o_format[col], x);
			else	/* Use the general float format */
				sprintf (text, GMT->current.setting.format_float_out, x);
			break;
	}
}

void gmt_ascii_format_one (struct GMT_CTRL *GMT, char *text, double x, unsigned int type) {
	if (gmt_M_is_dnan (x)) {
		sprintf (text, "NaN");
		return;
	}
	switch (type) {
		case GMT_IS_LON:
			gmtio_format_geo_output (GMT, false, x, text);
			break;
		case GMT_IS_LAT:
			gmtio_format_geo_output (GMT, true, x, text);
			break;
		case GMT_IS_ABSTIME:
			gmt_format_abstime_output (GMT, x, text);
			break;
		default:
			sprintf (text, GMT->current.setting.format_float_out, x);
			break;
	}
}

/*! . */
void gmt_init_io_columns (struct GMT_CTRL *GMT, unsigned int dir) {
	/* Initialize (reset) information per column which may have changed due to -i -o */
	unsigned int i;
	for (i = 0; i < GMT_MAX_COLUMNS; i++) GMT->current.io.col[dir][i].col = GMT->current.io.col[dir][i].order = i;	/* Default order */
	if (dir == GMT_OUT) return;
	for (i = 0; i < GMT_MAX_COLUMNS; i++) GMT->current.io.col_skip[i] = false;	/* Consider all input columns */
}

/*! . */
void gmtlib_io_init (struct GMT_CTRL *GMT) {
	/* No need to memset the structure to NULL as this is done initlally.
	 * The assignments here are done once per GMT session as gmtlib_io_init is called
	 * from gmt_begin.  Some variables may change later due to --PAR=value parsing.
	 * gmtlib_io_init must be called before parsing of defaults. */

	unsigned int i;

	GMT->common.q.rec = &(GMT->current.io.data_record_number_in_set[GMT_IN]);	/* Default assignment for -q counter */

	/* Pointer assignment for default ASCII input functions */

	GMT->current.io.input  = GMT->session.input_ascii = &gmtio_ascii_input;
	GMT->current.io.output = &gmtio_ascii_output;

	GMT->current.io.ogr_parser = &gmtio_ogr_header_parser;		/* Parse OGR header records to start with */

	GMT->current.io.scan_separators = GMT_TOKEN_SEPARATORS;		/* Characters that may separate columns in ascii records */

	/* Assign non-zero/NULL initial values */

	GMT->current.io.give_report = true;
	GMT->current.io.seg_no = GMT->current.io.rec_no = GMT->current.io.rec_in_tbl_no = 0;	/* These gets incremented so 1 means 1st record */
	GMT->current.io.warn_geo_as_cartesion = true;	/* Not yet read geographic data while in Cartesian mode so we want to warn if we find it */
	GMT->current.io.trailing_text[GMT_IN] = GMT->current.io.trailing_text[GMT_OUT] = true;	/* Default reads and writes any trailing text */
	GMT->current.setting.io_seg_marker[GMT_IN] = GMT->current.setting.io_seg_marker[GMT_OUT] = '>';
	strcpy (GMT->current.io.r_mode, "r");
	strcpy (GMT->current.io.w_mode, "w");
	strcpy (GMT->current.io.a_mode, "a+");
	for (i = 0; i < 4; i++) {
		GMT->current.io.date_input.item_order[i] = GMT->current.io.date_input.item_pos[i] = -1;
		GMT->current.io.date_output.item_order[i] = GMT->current.io.date_output.item_pos[i] = -1;
	}
	for (i = 0; i < 3; i++) {
		GMT->current.io.clock_input.order[i] = GMT->current.io.clock_output.order[i] = GMT->current.io.geo.order[i] = -1;
	}
	strcpy (GMT->current.io.clock_input.ampm_suffix[0],  "am");
	strcpy (GMT->current.io.clock_output.ampm_suffix[0], "am");
	strcpy (GMT->current.io.clock_input.ampm_suffix[1],  "pm");
	strcpy (GMT->current.io.clock_output.ampm_suffix[1], "pm");

	gmt_init_io_columns (GMT, GMT_IN);	/* Set default input column order */
	gmt_init_io_columns (GMT, GMT_OUT);	/* Set default output column order */
	for (i = 0; i < 2; i++) GMT->current.io.skip_if_NaN[i] = true;								/* x/y must be non-NaN */
	for (i = 0; i < 2; i++) gmt_set_column (GMT, GMT_IO, i, GMT_IS_UNKNOWN);	/* Must be told [or find out] what x/y are */
	for (i = 2; i < GMT_MAX_COLUMNS; i++) gmt_set_column (GMT, GMT_IO, i, GMT_IS_FLOAT);	/* Other columns default to floats */
	gmt_M_memset (GMT->current.io.col_set[GMT_X], GMT_MAX_COLUMNS, char);	/* This is the initial state of input columns - all available to be changed by modules */
	gmt_M_memset (GMT->current.io.col_set[GMT_Y], GMT_MAX_COLUMNS, char);	/* This is the initial state of output columns - all available to be changed by modules */
	gmt_M_memset (GMT->current.io.curr_rec, GMT_MAX_COLUMNS, double);	/* Initialize current and previous records to zero */
	gmt_M_memset (GMT->current.io.prev_rec, GMT_MAX_COLUMNS, double);
	GMT->current.io.record.data = GMT->current.io.curr_rec;
}

/*! Routine will temporarily suspend any -i, -h selections for secondary inputs */
void gmt_disable_bhi_opts (struct GMT_CTRL *GMT) {
	/* Temporarily turn off any -i, -h selections */
	GMT->common.i.select = false;
	GMT->current.setting.io_header_orig = GMT->current.setting.io_header[GMT_IN];
	GMT->current.setting.io_header[GMT_IN] = false;
	/* Then deal with primary binary input selection */
	if (GMT->common.b.active[GMT_IN]) {	/* Secondary file input requires ascii */
		GMT->common.b.active[GMT_IN] = false;
		GMT->common.b.bin_primary = true;
		GMT->current.io.input = &gmtio_ascii_input;
	}
}

/*! Routine will re-enable any suspended -i, -h selections */
void gmt_reenable_bhi_opts (struct GMT_CTRL *GMT) {
	/* Turn on again any -i, -h selections */
	GMT->common.i.select = GMT->common.i.orig;
	GMT->current.setting.io_header[GMT_IN] = GMT->current.setting.io_header_orig;
	if (GMT->common.b.bin_primary) {	/* Switch back to primary i/o mode which was binary */
		GMT->common.b.active[GMT_IN] = true;
		GMT->common.b.bin_primary = false;
		GMT->current.io.input = &gmtio_bin_input;
	}
}

/*! . */
void gmt_lon_range_adjust (unsigned int range, double *lon) {
	switch (range) {	/* Adjust to the desired range */
		case GMT_IS_0_TO_P360_RANGE:		/* Make 0 <= lon <= 360 */
			while ((*lon) < 0.0) (*lon) += 360.0;
			while ((*lon) > 360.0) (*lon) -= 360.0;
			break;
		case GMT_IS_0_TO_P360:		/* Make 0 <= lon < 360 */
			while ((*lon) < 0.0) (*lon) += 360.0;
			while ((*lon) >= 360.0) (*lon) -= 360.0;
			break;
		case GMT_IS_M360_TO_0_RANGE:		/* Make -360 <= lon <= 0 */
			while ((*lon) < -360.0) (*lon) += 360.0;
			while ((*lon) > 0) (*lon) -= 360.0;
			break;
		case GMT_IS_M360_TO_0:		/* Make -360 < lon <= 0 */
			while ((*lon) <= -360.0) (*lon) += 360.0;
			while ((*lon) > 0) (*lon) -= 360.0;
			break;
		case GMT_IS_M180_TO_P180_RANGE:	/* Make -180 <= lon <= +180 */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) > 180.0) (*lon) -= 360.0;
			break;
		case GMT_IS_M180_TO_P180:	/* Make -180 <= lon < +180 [Special case where +180 is not desired] */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) >= 180.0) (*lon) -= 360.0;
			break;
		case GMT_IS_M180_TO_P270_RANGE:	/* Make -180 <= lon < +270 [Special case for GSHHG only] */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) >= 270.0) (*lon) -= 360.0;
			break;
		default:	/* Do nothing */
			break;
	}
}

/*! . */
void gmt_quad_reset (struct GMT_CTRL *GMT, struct GMT_QUAD *Q, uint64_t n_items) {
	/* Allocate and initialize the QUAD struct needed to find min/max of a set of longitudes */
	uint64_t i;

	gmt_M_unused(GMT);
	gmt_M_memset (Q, n_items, struct GMT_QUAD);	/* Set all to NULL/0 */
	for (i = 0; i < n_items; i++) {
		Q[i].min[0] = Q[i].min[1] = +DBL_MAX;
		Q[i].max[0] = Q[i].max[1] = -DBL_MAX;
		Q[i].range[0] = GMT_IS_M180_TO_P180_RANGE;
		Q[i].range[1] = GMT_IS_0_TO_P360_RANGE;
	}
}

/*! . */
struct GMT_QUAD * gmt_quad_init (struct GMT_CTRL *GMT, uint64_t n_items) {
	/* Allocate an initialize the QUAD struct needed to find min/max of longitudes */
	struct GMT_QUAD *Q = gmt_M_memory (GMT, NULL, n_items, struct GMT_QUAD);

	gmt_quad_reset (GMT, Q, n_items);

	return (Q);
}

/*! . */
void gmt_quad_add (struct GMT_CTRL *GMT, struct GMT_QUAD *Q, double x) {
	/* Update quad array for this longitude x */
	unsigned int way, quad_no;
	gmt_M_unused(GMT);
	if (gmt_M_is_dnan (x)) return;	/* Cannot handle a NaN */
	for (way = 0; way < 2; way++) {
		gmt_lon_range_adjust (Q->range[way], &x);	/* Set -180/180, then 0-360 range */
		Q->min[way] = MIN (x, Q->min[way]);
		Q->max[way] = MAX (x, Q->max[way]);
	}
	quad_no = urint (floor (x / 90.0));	/* Now x is 0-360; this yields quadrants 0-3 */
	if (quad_no == 4) quad_no = 0;		/* When x == 360.0 */
	Q->quad[quad_no] = true;		/* Our x fell in this quadrant */
}

/*! . */
unsigned int gmt_quad_finalize (struct GMT_CTRL *GMT, struct GMT_QUAD *Q) {
	/* Finalize longitude range settings */
	uint64_t n_quad;
	unsigned int way;

	n_quad = Q->quad[0] + Q->quad[1] + Q->quad[2] + Q->quad[3];		/* How many quadrants had data */
	if (Q->quad[0] && Q->quad[3])		/* Longitudes on either side of Greenwich only, must use -180/+180 notation */
		way = 0;
	else if (Q->quad[1] && Q->quad[2])	/* Longitudes on either side of the date line, must user 0/360 notation */
		way = 1;
	else if (n_quad == 2 && ((Q->quad[0] && Q->quad[2]) || (Q->quad[1] && Q->quad[3])))	/* Funny quadrant gap, pick shortest longitude extent */
		way = ((Q->max[0] - Q->min[0]) < (Q->max[1] - Q->min[1])) ? 0 : 1;
	else					/* Either will do, use default settings */
		way = (GMT->current.io.geo.range == GMT_IS_0_TO_P360_RANGE) ? 1 : 0;
	/* Final adjustments */
	if (Q->min[way] > Q->max[way]) Q->min[way] -= 360.0;
	if (Q->min[way] < 0.0 && Q->max[way] < 0.0) Q->min[way] += 360.0, Q->max[way] += 360.0;
	return (way);
}

GMT_LOCAL void gmtio_update_west_east_limits (struct GMT_CTRL *GMT, double *W, double *E, double w, double e) {
	/* Longitudes are tricky and we must find total extent by adding one new segment at the time.
	 * Extend the previous W/E extent with the new w/e to minimize final longitude range */
	double shift, best_shift = 0.0, range, range_max = DBL_MAX, WW = *W, EE = *E;
	int k;
	if (*W == *E) {	/* Initialization */
		*W = w;	*E = e;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Longitude range initialized to %g/%g\n", *W, *E);
		return;
	}
	for (k = -1; k <= +1; k++) {	/* Try to shift 2nd range by -360, 0, and +360 */
		shift = k * 360.0;
		range = MAX (*E, e + shift) - MIN (*W, w + shift);
		if (range < range_max) {	/* This is a tighter fit */
			best_shift = shift;
			range_max = range;
		}
	}
	*W = MIN (*W, w + best_shift);
	*E = MAX (*E, e + best_shift);
	/* Ensure w < e and fits inside usual limits */
	if (*W > *E) *W -= 360.0;
	if (*W < 0.0 && *E < 0.0) { *W += 360.0; *E += 360.0;}
	else if (*E > 360.0) { *W -= 360.0; *E -= 360.0;}
	if ((*E - *W) > 360.0) { *W = 0.0; *E = 360.0;}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Longitude range %g/%g + %g/%g = %g/%g\n", WW, EE, w, e, *W, *E);
}

GMT_LOCAL int compare_center (const void *p1, const void *p2) {
	const struct GMT_RANGE *a = p1, *b = p2;
	if (a->center < b->center) return (-1);
	if (a->center > b->center) return (+1);
	return (0);
}

void gmt_find_range (struct GMT_CTRL *GMT, struct GMT_RANGE *Z, uint64_t n_items, double *west, double *east) {
	/* Need to sort Z on center longitude, then build final range incrementally and return answer via west/east */
	uint64_t k;
	for (k = 0; k < n_items; k++) {	/* Find middle point, and force 0-360 range */
		Z[k].center = 0.5 * (Z[k].east + Z[k].west);
		if (Z[k].center < 0.0) Z[k].center += 360.0;
	}
	qsort (Z, n_items, sizeof (struct GMT_RANGE), compare_center);
	*west = *east = 0.0;	/* Initialized to have no range yet */
	for (k = 0; k < n_items; k++)
		gmtio_update_west_east_limits (GMT, west, east, Z[k].west, Z[k].east);
}

/*! . */
void gmtlib_get_lon_minmax (struct GMT_CTRL *GMT, double *lon, uint64_t n_rows, double *min, double *max) {
	/* Return the min/max longitude in array lon using clever quadrant checking. */
	bool all_negative = true;
	unsigned int way;
	uint64_t row;
	struct GMT_QUAD *Q = gmt_quad_init (GMT, 1);	/* Allocate and initialize one QUAD structure */

	/* We must keep separate min/max for both Dateline and Greenwich conventions */
	for (row = 0; row < n_rows; row++) {
		if (lon[row] > 0.0) all_negative = false;
		gmt_quad_add (GMT, Q, lon[row]);
	}

	/* Finalize longitude range settings */
	way = gmt_quad_finalize (GMT, Q);
	*min = Q->min[way];		*max = Q->max[way];
	if (all_negative && *min >= 0.0 && *max > 0.0) {	/* Shift the min/max to negative longitudes */
		*min -= 360.0;	*max -= 360.0;
	}
	gmt_M_free (GMT, Q);
}

/*! . */
void gmt_eliminate_lon_jumps (struct GMT_CTRL *GMT, double *lon, uint64_t n_rows) {
	/* Eliminate longitude jumps in array lon using clever quadrant checking. */
	unsigned int way;
	uint64_t row;
	struct GMT_QUAD *Q = gmt_quad_init (GMT, 1);	/* Allocate and initialize one QUAD structure */

	/* We must keep separate min/max for both Dateline and Greenwich conventions */
	for (row = 0; row < n_rows; row++) gmt_quad_add (GMT, Q, lon[row]);

	/* Finalize longitude range settings */
	way = gmt_quad_finalize (GMT, Q);
	for (row = 0; row < n_rows; row++) gmt_lon_range_adjust (Q->range[way], &lon[row]);

	gmt_M_free (GMT, Q);
}

int gmtlib_determine_pole (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n) {
	/* Determines if a polygon is a polar cap and if so, which one:
	 *  0 : Not a polar cap.
	 * -1 : A south polar cap going clockwise
	 * -2 : A south polar cap going counter-clockwise
	 * +1 : A north polar cap going clockwise
	 * +2 : A north polar cap going counter-clockwise.
	 * -99: Returned if there is an error (no points)
	 * Here, we tolerate open polygons and reuse 1st point to close it in the calculation.
	 */
	bool touched_N = false, touched_S = false, open = false;
	uint64_t row, n_unique, last_point = 0, next;
	int type = 0, n_360;
	double dlon, lon_sum = 0.0, lat_sum = 0.0, lat_S = 90.0, lat_N = -90.0;
	static char *pole[5] = {"south (CCW)", "south (CW)", "no", "north (CW)", "north (CCW)"};

	if (n == 0) return -99;	/* Nothing given */
	if (gmt_polygon_is_open (GMT, lon, lat, n)) {	/* No repeat last = first point so reuse first */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Calling gmtlib_determine_pole on an open polygon\n");
		n_unique = n;	/* Need to loop over all input points and then go to point 0 for last leg */
		open = true;
		last_point = n - 1;
	}
	else	/* Stop 1 short since last point is the duplicate first point */
		n_unique = n - 1;
	for (row = 0; row < n_unique; row++) {	/* Add up angular increments between vertices */
		next = (open && row == last_point) ? 0 : row + 1;
		gmt_M_set_delta_lon (lon[row], lon[next], dlon);	/* Handles the 360 jump cases */
		lon_sum += dlon;
		lat_sum += lat[row];
		if (doubleAlmostEqual (lat[row], +90.0)) touched_N = true;
		else if (doubleAlmostEqual (lat[row], -90.0)) touched_S = true;
		if (lat[row] < lat_S) lat_S = lat[row];
		if (lat[row] > lat_N) lat_N = lat[row];
	}
	n_360 = irint (lon_sum / 360.0);	/* This is either -1, 0, or +1 since lon_sum is either -360, 0, +360 plus some noise */
	if (n_360) {	/* test is true if contains a pole; adjust rectangular bounds and set pole flag accordingly */
		dlon = (n_360 > 0) ? 2.0 : 1.0;			/* 2 for CCW, 1 for CW paths */
		type = irint (copysign (dlon, lat_sum));	/* Here, either -2, -1 or +1, +2 */
		if (type < 0 && touched_N && lat_S > -90.0) type = -type;
		else if (type > 0 && touched_S && lat_N < 90.0) type = -type;
	}
    if (touched_N && touched_S) type = 0;   /* Cannot contain both poles */
	else if (type == 0 && touched_N) type = 1;	/* Cuts through the N pole */
	else if (type == 0 && touched_S) type = -1;	/* Cuts through the S pole */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmtlib_determine_pole: N = %" PRIu64 " Multiples of 360: %d  Residual: %g Polygon contains %s pole.\n", n, n_360, lon_sum - n_360 * 360.0, pole[type+2]);
	return (type);
}

/*! . */
void gmt_set_seg_polar (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S) {
	/* Must check if polygon is a polar cap.  We use the idea that the sum of
 	 * the change in angle along the polygon is either -/+360 (if pole is inside)
	 * or 0 if outside.  The sign (not used here) gives the handedness of the polygon.
	 * Which pole (S or N) is determined by computng the average latitude and
	 * assuming the pole is in the heimsphere most visited.  This may not be
	 * true of course. */
	int answer;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);

	if ((gmt_M_type (GMT, GMT_IN, GMT_X) & GMT_IS_GEO) == 0 || S->n_columns < 2) return;	/* No can do */
	if ((answer = gmtlib_determine_pole (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows)) == -99) return;	/* No good */
	if (answer) {	/* true if contains a pole; adjust rectangular bounds and set pole flag */
		SH->pole = (answer < 0) ? -1 : +1;
		S->min[GMT_X] = 0.0;	S->max[GMT_X] = 360.0;
		if (SH->pole == -1) SH->lat_limit = S->min[GMT_Y], S->min[GMT_Y] = -90.0;
		else if (SH->pole == +1) SH->lat_limit = S->max[GMT_Y], S->max[GMT_Y] = +90.0;
	}
	else	/* So, 0 means not polar */
		SH->pole = 0;
}

/*! . */
bool gmtlib_geo_to_dms (double val, int n_items, double fact, int *d, int *m,  int *s,  int *ix) {
	/* Convert floating point degrees to dd:mm[:ss][.xxx].  Returns true if d = 0 and val is negative */
	bool minus;
	int isec, imin;
	double sec, fsec, min, fmin, step;

	minus = (val < 0.0);
	step = (fact == 0.0) ? GMT_CONV8_LIMIT : 0.5 / fact;  	/* Precision desired in seconds (or minutes); else just deal with roundoff */

	if (n_items == 3) {		/* Want dd:mm:ss[.xxx] format */
		sec = GMT_DEG2SEC_F * fabs (val) + step;	/* Convert to seconds */
		isec = irint (floor (sec));			/* Integer seconds */
		fsec = sec - (double)isec;  			/* Leftover fractional second */
		*d = isec / GMT_DEG2SEC_I;			/* Integer degrees */
		isec -= ((*d) * GMT_DEG2SEC_I);			/* Left-over seconds in the last degree */
		*m = isec / GMT_MIN2SEC_I;			/* Integer minutes */
		isec -= ((*m) * GMT_MIN2SEC_I);			/* Leftover seconds in the last minute */
		*s = isec;					/* Integer seconds */
		*ix = irint (floor (fsec * fact));		/* Fractional seconds scaled to integer */
	}
	else if (n_items == 2) {		/* Want dd:mm[.xxx] format */
		min = GMT_DEG2MIN_F * fabs (val) + step;	/* Convert to minutes */
		imin = irint (floor (min));			/* Integer minutes */
		fmin = min - (double)imin;  			/* Leftover fractional minute */
		*d = imin / GMT_DEG2MIN_I;			/* Integer degrees */
		imin -= ((*d) * GMT_DEG2MIN_I);			/* Left-over seconds in the last degree */
		*m = imin;					/* Integer minutes */
		*s = 0;						/* No seconds */
		*ix = irint (floor (fmin * fact));		/* Fractional minutes scaled to integer */
	}
	else {		/* Want dd[.xxx] format */
		min = fabs (val) + step;			/* Convert to degrees */
		imin = irint (floor (min));			/* Integer degrees */
		fmin = min - (double)imin;  			/* Leftover fractional degree */
		*d = imin;					/* Integer degrees */
		*m = 0;						/* Integer minutes */
		*s = 0;						/* No seconds */
		*ix = irint (floor (fmin * fact));		/* Fractional degrees scaled to integer */
	}
	if (minus) {	/* OK, change sign, but watch for *d = 0 */
		if (*d)	/* Non-zero degree term is easy */
			*d = -(*d);
		else	/* Cannot change 0 to -0, so pass flag back to calling function */
			return (true);
	}
	return (false);
}

/*! . */
void gmt_add_to_record (struct GMT_CTRL *GMT, char *record, double val, uint64_t col, unsigned int way, unsigned int sep) {
	/* formats and appends val to the record texts string; way is GMT_IN|GMT_OUT
	 * If sep is 1 we prepend col separator.
	 * If sep is 2 we append col separator
	 * If sep is 1|2 do both [0 means no separator].
	 * if sep > 10 we init record then remove 10 from sep.
	 */
	char word[GMT_LEN64] = {""};
	gmt_ascii_format_col (GMT, word, val, way, col);
	if (sep >= 10) {	/* Initialize new record */
		record[0] = '\0';
		sep -= 10;
	}
	if (sep & 1) strcat (record, GMT->current.setting.io_col_separator);
	strcat (record, word);
	if (sep & 2) strcat (record, GMT->current.setting.io_col_separator);
}

/*! . */
void gmt_cat_to_record (struct GMT_CTRL *GMT, char *record, char *word, unsigned int way, unsigned int sep) {
	/* appends val to the record text string; way is GMT_IN|GMT_OUT
	 * If sep is 1 we prepend col separator.
	 * If sep is 2 we append col separator
	 * If sep is 1|2 do both [0 means no separator].
	 * if sep > 10 we init record then remove 10 from sep.
	 */
	gmt_M_unused(way);
	if (sep >= 10) {	/* Initialize new record */
		record[0] = '\0';
		sep -= 10;
	}
	if (sep & 1) strcat (record, GMT->current.setting.io_col_separator);
	strcat (record, word);
	if (sep & 2) strcat (record, GMT->current.setting.io_col_separator);
}

/*! . */
void gmt_write_segmentheader (struct GMT_CTRL *GMT, FILE *fp, uint64_t n_cols) {
	/* Output ASCII or binary segment header.
	 * ASCII header is expected to contain newline (\n) */

	uint64_t col;

	if (!GMT->current.io.multi_segments[GMT_OUT]) return;	/* No output segments requested */
	GMT->current.io.data_record_number_in_seg[GMT_OUT] = 0;	/* Reset counter */
	if (GMT->common.b.active[GMT_OUT]) {			/* Binary native file uses all NaNs */
		for (col = 0; col < n_cols; col++) GMT->current.io.output (GMT, fp, 1, &GMT->session.d_NaN, NULL);
		return;
	}
	/* Here we are doing ASCII */
	if (GMT->current.setting.io_blankline[GMT_OUT])	/* Write blank line to indicate segment break */
		fprintf (fp, "\n");
	else if (GMT->current.setting.io_nanline[GMT_OUT]) {	/* Write NaN record to indicate segment break */
		if (GMT->common.d.active[GMT_OUT]) {	/* Ah, but NaNs are to be replaced */
			gmt_ascii_output_col (GMT, fp, GMT->common.d.nan_proxy[GMT_OUT], GMT_Z);
			for (col = 1 ; col < MAX (2,n_cols); col++) {
				fprintf (fp, "%s", GMT->current.setting.io_col_separator);
				gmt_ascii_output_col (GMT, fp, GMT->common.d.nan_proxy[GMT_OUT], GMT_Z);
			}
			fprintf (fp, "\n");
		}
		else {
			for (col = 1 ; col < MAX (2,n_cols); col++)
				fprintf (fp, "NaN%s", GMT->current.setting.io_col_separator);
			fprintf (fp, "NaN\n");
		}
	}
	else if (!GMT->current.io.segment_header[0])		/* No header; perhaps via binary input with NaN-headers */
		fprintf (fp, "%c\n", GMT->current.setting.io_seg_marker[GMT_OUT]);
	else
		fprintf (fp, "%c %s\n", GMT->current.setting.io_seg_marker[GMT_OUT], GMT->current.io.segment_header);
}

/*! . */
void gmtlib_io_binary_header (struct GMT_CTRL *GMT, FILE *fp, unsigned int dir) {
	uint64_t k;
	char c = ' ';
	if (dir == GMT_IN) {	/* Use fread since we don't know if input is a stream or a file */
		size_t nr = 0;
		for (k = 0; k < GMT->current.setting.io_n_header_items; k++) nr += gmt_M_fread (&c, sizeof (char), 1U, fp);
	}
	else {
		for (k = 0; k < GMT->current.setting.io_n_header_items; k++) gmt_M_fwrite (&c, sizeof (char), 1U, fp);
	}
}

/*! . */
void gmtlib_write_textrecord (struct GMT_CTRL *GMT, FILE *fp, char *txt) {
	/* Output ASCII segment header; skip if mode is binary.
	 * We append a newline (\n) if not is present */

	if (GMT->common.b.active[GMT_OUT]) return;		/* Cannot write text records if binary output */
	if (!txt || !txt[0]) return;				/* Skip blank lines */
	fprintf (fp, "%s", txt);				/* May or may not have \n at end */
	if (txt[strlen(txt)-1] != '\n') fputc ('\n', fp);	/* Make sure we have \n at end */
}

/*! . */
bool gmt_byteswap_file (struct GMT_CTRL *GMT, FILE *outfp, FILE *infp, const SwapWidth swapwidth, const uint64_t offset, const uint64_t length) {
	/* read from *infp and write byteswapped data to *ofp
	 * swap only 'length' bytes beginning at 'offset' bytes
	 * if 'length == 0' swap until EOF */
	uint64_t bytes_read = 0;
	size_t nbytes, chunk, extrabytes = 0;
	static const size_t chunksize = 0x1000000; /* 16 MiB */
	char *buffer, message[GMT_LEN256] = {""};

	/* length must be a multiple SwapWidth */
	if ( length%swapwidth != 0 ) {
		snprintf (message, GMT_LEN256, "%s: error: length must be a multiple of %u bytes.\n", __func__, swapwidth);
		GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
		return false;
	}

	/* allocate buffer on stack to improve disk i/o */
	buffer = malloc (chunksize);
	if (buffer == NULL) {
		snprintf (message, GMT_LEN256, "%s: error: cannot malloc %" PRIuS " bytes.\n", __func__, chunksize);
		GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
		return false;
	}

	/* skip offset bytes at beginning of infp */
	while ( bytes_read < offset ) {
		chunk = chunksize < offset - bytes_read ? chunksize : offset - bytes_read;
		nbytes = fread (buffer, sizeof (char), chunk, infp);
		if (nbytes == 0) {
			if (feof (infp)) {
				/* EOF */
#ifdef DEBUG_BYTESWAP
				snprintf (message, GMT_LEN256, "%s: EOF encountered at %" PRIu64
						" (before offset at %" PRIu64 ")\n", __func__, bytes_read, offset);
				GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
#endif
				GMT->current.io.status = GMT_IO_EOF;
				gmt_M_str_free (buffer);
				return true;
			}
			snprintf (message, GMT_LEN256, "%s: error reading stream while skipping.\n", __func__);
			GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
			gmt_M_str_free (buffer);
			return false;
		}
		bytes_read += nbytes;
		/* write buffer */
		if (fwrite_check (GMT, buffer, sizeof (char), nbytes, outfp)) {
			gmt_M_str_free (buffer);
			return false;
		}
	}
#ifdef DEBUG_BYTESWAP
	if (bytes_read) {
		snprintf (message, GMT_LEN256, "%s: %" PRIu64 " bytes skipped at beginning.\n", __func__, bytes_read);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, message);
	}
#endif

	/* start swapping bytes */
	while ( length == 0 || bytes_read < offset + length ) {
		uint64_t bytes_left = length - bytes_read + offset;
		chunk = (length == 0 || bytes_left > chunksize) ? chunksize : bytes_left;
		nbytes = fread (buffer, sizeof (char), chunk, infp);
		if (nbytes == 0) {
			if (feof (infp)) {
				/* EOF */
#ifdef DEBUG_BYTESWAP
				snprintf (message, GMT_LEN256, "%s: %" PRIu64 " bytes swapped.\n", __func__, bytes_read - offset - extrabytes);
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, message);
#endif
				if (extrabytes != 0) {
					snprintf (message, GMT_LEN256, "%s: The last %" PRIuS " bytes were ignored during swapping.\n",
							__func__, extrabytes);
					GMT_Report (GMT->parent, GMT_MSG_WARNING, message);
				}
				GMT->current.io.status = GMT_IO_EOF;
				gmt_M_str_free (buffer);
				return true;
			}
			snprintf (message, GMT_LEN256, "%s: error reading stream while swapping.\n", __func__);
			GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
			gmt_M_str_free (buffer);
			return false;
		}
		bytes_read += nbytes;
#ifdef DEBUG_BYTESWAP
		snprintf (message, GMT_LEN256, "%s: read %" PRIuS " bytes into buffer of size %" PRIuS ".\n",
				__func__, nbytes, chunksize);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, message);
#endif

		/* nbytes must be a multiple of SwapWidth */
		extrabytes = nbytes % swapwidth;
		if (extrabytes != 0) {
			/* this can only happen on EOF, ignore extra bytes while swapping. */
			snprintf (message, GMT_LEN256, "%s: Read buffer contains %" PRIuS " bytes which are "
					"not aligned with the swapwidth of %" PRIuS " bytes.\n",
					__func__, nbytes, extrabytes);
			GMT_Report (GMT->parent, GMT_MSG_WARNING, message);
			nbytes -= extrabytes;
		}

		/* swap bytes in buffer */
		switch (swapwidth) {
			case Int16len:
				swap_uint16 (buffer, nbytes);
				break;
			case Int32len:
				swap_uint32 (buffer, nbytes);
				break;
			case Int64len:
			default:
				swap_uint64 (buffer, nbytes);
				break;
		}

		/* restore nbytes */
		nbytes += extrabytes;

		/* write buffer */
		if (fwrite_check (GMT, buffer, sizeof (char), nbytes, outfp)) {
			gmt_M_str_free (buffer);
			return false;
		}
	}
#ifdef DEBUG_BYTESWAP
	snprintf (message, GMT_LEN256, "%s: %" PRIu64 " bytes swapped.\n", __func__, bytes_read - offset);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, message);
#endif

	/* skip to EOF */
	while ( true ) {
		nbytes = fread (buffer, sizeof (char), chunksize, infp);
		if (nbytes == 0) {
			if (feof (infp)) {
				/* EOF */
#ifdef DEBUG_BYTESWAP
				snprintf (message, GMT_LEN256, "%s: %" PRIu64 " bytes nbytes until EOF.\n",
						__func__, bytes_read - offset - length);
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, message);
#endif
				break;
			}
			snprintf (message, GMT_LEN256, "%s: error reading stream while skipping to EOF.\n", __func__);
			GMT_Report (GMT->parent, GMT_MSG_ERROR, message);
			gmt_M_str_free (buffer);
			return false;
		}
		bytes_read += nbytes;
		/* write buffer */
		if (fwrite_check (GMT, buffer, sizeof (char), nbytes, outfp)) {
			gmt_M_str_free (buffer);
			return false;
		}
	}

	GMT->current.io.status = GMT_IO_EOF;
	gmt_M_str_free (buffer);
	return true;
}

/*! . */
int gmt_parse_z_io (struct GMT_CTRL *GMT, char *txt, struct GMT_PARSE_Z_IO *z) {
	int value;
	unsigned int i, k = 0, start;

	if (!txt) return (GMT_PARSE_ERROR);	/* Must give a non-NULL argument */
	if (!txt[0]) return (0);		/* Default -ZTLa */

	for (start = 0; !z->not_grid && txt[start] && start < 2; start++) {	/* Loop over the first 2 flags unless dataset is not a grid */

		switch (txt[start]) {

			/* These 4 cases will set the format orientation for input */

			case 'T':
			case 'B':
			case 'L':
			case 'R':
				z->format[k++] = txt[start];
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: Must begin with [TBLR][TBLR]!\n");
				return (GMT_PARSE_ERROR);
				break;
		}
	}

	for (i = start; txt[i]; i++) {	/* Loop over remaining flags */

		switch (txt[i]) {

			/* Set this if file is periodic, is grid registered, but repeating column or row is missing from input */

			case 'x':
				z->repeat[GMT_X] = true;	break;
			case 'y':
				z->repeat[GMT_Y] = true;	break;

			/* Optionally skip the given number of bytes before reading data */

			case 's':
				i++;
				if (txt[i]) {	/* Read the byte count for skipping */
					value = atoi (&txt[i]);
					if (value < 0) {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: Skip must be positive\n");
						return (GMT_PARSE_ERROR);
					}
					z->skip = value;
					while (txt[i] && isdigit ((int)txt[i])) i++;
					i--;
				}
				break;

			case 'w':
				z->swab = (k_swap_in | k_swap_out); 	break;	/* Default is swap both input and output when selected */

			/* Set read pointer depending on data format */

			case 'A': /* ASCII (next regular float (%lg) from the stream) */
			case 'a': /* ASCII (1 per record) */
			case 'c': /* Binary int8_t */
			case 'u': /* Binary uint8_t */
			case 'h': /* Binary int16_t */
			case 'H': /* Binary uint16_t */
			case 'i': /* Binary int32_t */
			case 'I': /* Binary uint32_t */
			case 'l': /* Binary int64_t */
			case 'L': /* Binary uint64_t */
			case 'f': /* Binary 4-byte float */
			case 'd': /* Binary 8-byte double */
				z->type = txt[i];
				break;

			default:
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: %c not a valid modifier!\n", txt[i]);
				return (GMT_PARSE_ERROR);
				break;
		}
	}

	return (0);
}

/*! . */
int gmt_get_io_type (struct GMT_CTRL *GMT, char type) {
	int t = -1;
	switch (type) {
		/* Set read pointer depending on data format */
		case 'a': case 'A':          break; /* ASCII */
		case 'c': t = GMT_CHAR;   break; /* Binary int8_t */
		case 'u': t = GMT_UCHAR;  break; /* Binary uint8_t */
		case 'h': t = GMT_SHORT;  break; /* Binary int16_t */
		case 'H': t = GMT_USHORT; break; /* Binary uint16_t */
		case 'i': t = GMT_INT;    break; /* Binary int32_t */
		case 'I': t = GMT_UINT;   break; /* Binary uint32_t */
		case 'l': t = GMT_LONG;   break; /* Binary int64_t */
		case 'L': t = GMT_ULONG;  break; /* Binary uint64_t */
		case 'f': t = GMT_FLOAT;  break; /* Binary 4-byte float */
		case 'd': t = GMT_DOUBLE; break; /* Binary 8-byte double */
		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Valid data type not set [%c]!\n", type);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;
			break;
	}
	return (t+1);	/* Since 0 means not set */
}

/*! . */
p_to_io_func gmtlib_get_io_ptr (struct GMT_CTRL *GMT, int direction, enum GMT_swap_direction swap, char type) {
	/* Return pointer to read or write function for this data type */
	/* swap is 0 for no swap, 1 for swap input, 2 for swap output, 3 for swap both */
	p_to_io_func p = NULL;

	switch (type) {	/* Set read pointer depending on data format */
		case 'd':	/* Binary 8-byte double */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_d_read_swab : &gmtio_d_read;
			else
				p = (swap & k_swap_out) ? &gmtio_d_write_swab : &gmtio_d_write;
			break;
		case 'A':	/* ASCII with more than one per record */
			p = (direction == GMT_IN) ? &gmtio_A_read : &gmtio_a_write;
			break;
		case 'a':	/* ASCII */
			p = (direction == GMT_IN) ? &gmtio_a_read : &gmtio_a_write;
			break;
		case 'c':	/* Binary int8_t */
			p = (direction == GMT_IN) ? &gmtio_c_read : &gmtio_c_write;
			break;
		case 'u':	/* Binary uint8_t */
			p = (direction == GMT_IN) ? &gmtio_u_read : &gmtio_u_write;
			break;
		case 'h':	/* Binary int16_t */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_h_read_swab : &gmtio_h_read;
			else
				p = (swap & k_swap_out) ? &gmtio_h_write_swab : &gmtio_h_write;
			break;
		case 'H':	/* Binary uint16_t */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_H_read_swab : &gmtio_H_read;
			else
				p = (swap & k_swap_out) ? &gmtio_H_write_swab : &gmtio_H_write;
			break;
		case 'i':	/* Binary int32_t */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_i_read_swab : &gmtio_i_read;
			else
				p = (swap & k_swap_out) ? &gmtio_i_write_swab : &gmtio_i_write;
			break;
		case 'I':	/* Binary uint32_t */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_I_read_swab : &gmtio_I_read;
			else
				p = (swap & k_swap_out) ? &gmtio_I_write_swab : &gmtio_I_write;
			break;
		case 'l':	/* Binary int64_t */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_l_read_swab : &gmtio_l_read;
			else
				p = (swap & k_swap_out) ? &gmtio_l_write_swab : &gmtio_l_write;
			break;
		case 'L':	/* Binary uint64_t */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_L_read_swab : &gmtio_L_read;
			else
				p = (swap & k_swap_out) ? &gmtio_L_write_swab : &gmtio_L_write;
			break;
		case 'f':	/* Binary 4-byte float */
			if (direction == GMT_IN)
				p = (swap & k_swap_in) ? &gmtio_f_read_swab : &gmtio_f_read;
			else
				p = (swap & k_swap_out) ? &gmtio_f_write_swab : &gmtio_f_write;
			break;
		case 'x':
			break;	/* Binary skip */

		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "%c not a valid data type!\n", type);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return NULL;
			break;
	}

	return (p);
}

/*! . */
int gmt_init_z_io (struct GMT_CTRL *GMT, char format[], bool repeat[], enum GMT_swap_direction swab, off_t skip, char type, struct GMT_Z_IO *r) {
	bool first = true;
	unsigned int k;

	gmt_M_memset (r, 1, struct GMT_Z_IO);

	for (k = 0; k < 2; k++) {	/* Loop over the two format flags */
		switch (format[k]) {
			/* These 4 cases will set the format orientation for input */
			case 'T':
				if (first) r->format = GMT_IS_ROW_FORMAT;
				r->y_step = 1;	first = false;	break;
			case 'B':
				if (first) r->format = GMT_IS_ROW_FORMAT;
				r->y_step = -1;	first = false;	break;
			case 'L':
				if (first) r->format = GMT_IS_COL_FORMAT;
				r->x_step = 1;	first = false;	break;
			case 'R':
				if (first) r->format = GMT_IS_COL_FORMAT;
				r->x_step = -1;	first = false;	break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: %c not a valid format specifier!\n", format[k]);
				GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
				break;
		}
	}

	if (!strchr ("AacuhHiIlLfd", type)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: %c not a valid data type!\n", type);
		GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;

	}

	r->x_missing = (repeat[GMT_X]) ? 1 : 0;	r->y_missing = (repeat[GMT_Y]) ? 1 : 0;
	r->skip = skip;			r->swab = swab;
	r->binary = (strchr ("Aa", type)) ? false : true;
	GMT->current.io.read_item  = gmtlib_get_io_ptr (GMT, GMT_IN,  swab, type);	/* Set read pointer depending on data format */
	GMT->current.io.write_item = gmtlib_get_io_ptr (GMT, GMT_OUT, swab, type);	/* Set write pointer depending on data format */
	GMT->common.b.type[GMT_IN] = GMT->common.b.type[GMT_OUT] = type;		/* Since -b is not setting this */
	if (r->binary) {	/* Use the binary modes (which only matters under Windoze)  */
		strcpy (GMT->current.io.r_mode, "rb");
		strcpy (GMT->current.io.w_mode, "wb");
		strcpy (GMT->current.io.a_mode, "ab+");
	}
	return (GMT_OK);
}

/*! . */
int gmt_set_z_io (struct GMT_CTRL *GMT, struct GMT_Z_IO *r, struct GMT_GRID *G) {
	/* THIS SHOULD NOT BE FATAL!
	if ((r->x_missing || r->y_missing) && G->header->registration == GMT_GRID_PIXEL_REG) return (GMT_GRDIO_RI_NOREPEAT);
	*/
	gmt_M_unused(GMT);
	r->start_col = ((r->x_step == 1) ? 0 : G->header->n_columns - 1 - r->x_missing);
	r->start_row = ((r->y_step == 1) ? r->y_missing : G->header->n_rows - 1);
	r->get_gmt_ij = (r->format == GMT_IS_COL_FORMAT) ? gmtio_col_ij : gmtio_row_ij;
	r->x_period = G->header->n_columns - r->x_missing;
	r->y_period = G->header->n_rows - r->y_missing;
	r->n_expected = ((uint64_t)r->x_period) * ((uint64_t)r->y_period);
	return (GMT_NOERROR);
}

/*! . */
void gmt_check_z_io (struct GMT_CTRL *GMT, struct GMT_Z_IO *r, struct GMT_GRID *G) {
	/* Routine to fill in the implied periodic row or column that was missing.
	 * We must allow for padding in G->data */

	unsigned int col, row;
	uint64_t k, k_top, k_bot;
	gmt_M_unused(GMT);

	if (r->x_missing) for (row = 0, k = gmt_M_ijp (G->header, row, 0); row < G->header->n_rows; row++, k += G->header->mx) G->data[k+G->header->n_columns-1] = G->data[k];
	if (r->y_missing) for (col = 0, k_top = gmt_M_ijp (G->header, 0, 0), k_bot = gmt_M_ijp (G->header, G->header->n_rows-1, 0); col < G->header->n_columns; col++) G->data[col+k_top] = G->data[col+k_bot];
}

/*! . */
void gmtlib_clock_C_format (struct GMT_CTRL *GMT, char *form, struct GMT_CLOCK_IO *S, unsigned int mode) {
	/* Determine the order of H, M, S in input and output clock strings,
	 * as well as the number of decimals in output seconds (if any), and
	 * if a 12- or 24-hour clock is used.
	 * mode is 0 for input, 1 for output, and 2 for plot output.
	 */

	S->skip = false;
	if (mode && strlen (form) == 1 && form[0] == '-') {	/* Do not want clock output or plotted */
		S->skip = true;
		return;
	}

	/* Get the order of year, month, day or day-of-year in input/output formats for dates */

	gmtio_get_hms_order (GMT, form, S);

	/* Craft the actual C-format to use for input/output clock strings */

	if (S->order[0] >= 0) {	/* OK, at least hours is needed */
		char fmt[GMT_LEN64] = {""};
		if (S->compact)
			snprintf (S->format, GMT_LEN64, "%%d");
		else
			(mode) ? snprintf (S->format, GMT_LEN64, "%%02d") : snprintf (S->format, GMT_LEN64, "%%2d");
		if (S->order[1] >= 0) {	/* Need minutes too*/
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			(mode) ? snprintf (fmt, GMT_LEN64, "%%02d") : snprintf (fmt, GMT_LEN64, "%%2d");
			strcat (S->format, fmt);
			if (S->order[2] >= 0) {	/* .. and seconds */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				if (mode) {	/* Output format */
					snprintf (fmt, GMT_LEN64, "%%02d");
					strcat (S->format, fmt);
					if (S->n_sec_decimals) {	/* even add format for fractions of second */
						snprintf (fmt, GMT_LEN64, ".%%%d.%dd", S->n_sec_decimals, S->n_sec_decimals);
						strcat (S->format, fmt);
					}
				}
				else {		/* Input format */
					sprintf (fmt, "%%lf");
					strcat (S->format, fmt);
				}
			}
		}
		if (mode && S->twelve_hr_clock) {	/* Finally add %s for the am, pm string */
			sprintf (fmt, "%%s");
			strcat (S->format, fmt);
		}
	}
}

/*! . */
void gmtlib_date_C_format (struct GMT_CTRL *GMT, char *form, struct GMT_DATE_IO *S, unsigned int mode) {
	/* Determine the order of Y, M, D, J in input and output date strings.
	 * mode is 0 for input, 1 for output, and 2 for plot output.
	 */

	int k, ywidth;
	bool no_delim, watch;
	char fmt[GMT_LEN64] = {""};

	S->skip = false;
	if (mode && strlen (form) == 1 && form[0] == '-') {	/* Do not want date output or plotted */
		S->skip = true;
		return;
	}

	/* Get the order of year, month, day or day-of-year in input/output formats for dates */

	watch = gmtio_get_ymdj_order (GMT, form, S);
	S->watch = (watch && mode == 0);

	/* Craft the actual C-format to use for i/o date strings */

	no_delim = !(S->delimiter[0][0] || S->delimiter[1][0]);	/* true for things like yyyymmdd */
	ywidth = (no_delim) ? 4 : 4+(!mode);			/* 4 or 5, depending on values */
	if (S->item_order[0] >= 0 && S->iso_calendar) {	/* ISO Calendar string: At least one item is needed */
		k = (S->item_order[0] == 0 && !S->Y2K_year) ? ywidth : 2;
		if (S->mw_text && S->item_order[0] == 1)	/* Prepare for "Week ##" format */
			snprintf (S->format, GMT_LEN64, "%%s %%02d");
		else if (S->compact)			/* Numerical formatting of week or year without leading zeros */
			sprintf (S->format, "%%d");
		else					/* Numerical formatting of week or year  */
			(mode) ? snprintf (S->format, GMT_LEN64, "%%%d.%dd", k, k) : snprintf (S->format, GMT_LEN64, "%%%dd", k);
		if (S->item_order[1] >= 0) {	/* Need another item */
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			if (S->mw_text && S->item_order[0] == 1) {	/* Prepare for "Week ##" format */
				sprintf (fmt, "%%s ");
				strcat (S->format, fmt);
			}
			else
				strcat (S->format, "W");
			if (S->compact)
				sprintf (fmt, "%%d");
			else
				(mode) ? sprintf (fmt, "%%02d") : sprintf (fmt, "%%2d");
			strcat (S->format, fmt);
			if (S->item_order[2] >= 0) {	/* and ISO day of week */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				sprintf (fmt, "%%1d");
				strcat (S->format, fmt);
			}
		}
	}
	else if (S->item_order[0] >= 0) {			/* Gregorian Calendar string: At least one item is needed */
		k = (S->item_order[0] == 0 && !S->Y2K_year) ? ywidth : 2;
		if (S->item_order[0] == 3) k = 3;	/* Day of year */
		if (S->mw_text && S->item_order[0] == 1) {	/* Prepare for "Monthname" format */
			if (mode == 0) {
				if (no_delim)
					sprintf (S->format, "%%3s");
				else
					snprintf (S->format, GMT_LEN64, "%%[^%s]", S->delimiter[0]);
			}
			else
				sprintf (S->format, "%%s");
		}
		else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
			sprintf (S->format, "%%d");
		else					/* Numerical formatting of month or year */
			(mode) ? snprintf (S->format, GMT_LEN64, "%%%d.%dd", k, k) : snprintf (S->format, GMT_LEN64, "%%%dd", k);
		if (S->item_order[1] >= 0) {	/* Need more items */
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			k = (S->item_order[1] == 0 && !S->Y2K_year) ? ywidth : 2;
			if (S->item_order[1] == 3) k = 3;	/* Day of year */
			if (S->mw_text && S->item_order[1] == 1) {	/* Prepare for "Monthname" format */
				if (mode == 0) {
					if (no_delim)
						sprintf (fmt, "%%3s");
					else
						snprintf (fmt, GMT_LEN64, "%%[^%s]", S->delimiter[1]);
				}
				else sprintf (fmt, "%%s");
			}
			else if (S->compact && !S->Y2K_year)		/* Numerical formatting of month or 4-digit year w/o leading zeros */
				sprintf (fmt, "%%d");
			else
				(mode) ? snprintf (fmt, GMT_LEN64, "%%%d.%dd", k, k) : snprintf (fmt, GMT_LEN64, "%%%dd", k);
			strcat (S->format, fmt);
			if (S->item_order[2] >= 0) {	/* .. and even more */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				k = (S->item_order[2] == 0 && !S->Y2K_year) ? ywidth : 2;
				if (S->mw_text && S->item_order[2] == 1)	/* Prepare for "Monthname" format */
					sprintf (fmt, "%%s");
				else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
					sprintf (fmt, "%%d");
				else
					(mode) ? snprintf (fmt, GMT_LEN64, "%%%d.%dd", k, k) : snprintf (fmt, GMT_LEN64, "%%%dd", k);
				strcat (S->format, fmt);
			}
		}
	}
}

/*! . */
int gmtlib_geo_C_format (struct GMT_CTRL *GMT) {
	/* Determine the output of geographic location formats. */

	struct GMT_GEO_IO *S = &GMT->current.io.geo;

	gmtio_get_dms_order (GMT, GMT->current.setting.format_geo_out, S);	/* Get the order of degree, min, sec in output formats */

	if (S->no_sign) return (GMT_IO_BAD_PLOT_DEGREE_FORMAT);

	if (S->decimal) {	/* Plain decimal degrees */
		 /* here we depend on FORMAT_FLOAT_OUT begin set.  This will not be true when FORMAT_GEO_MAP is parsed but will be
		  * handled at the end of gmt_begin.  For gmtset and --PAR later we will be OK as well. */
		if (!GMT->current.setting.format_float_out[0]) return (GMT_NOERROR); /* Quietly return and deal with this later in gmt_begin */
		sprintf (S->x_format, "%s", GMT->current.setting.format_float_out);
		sprintf (S->y_format, "%s", GMT->current.setting.format_float_out);
	}
	else {			/* Some form of dd:mm:ss */
		char fmt[GMT_LEN64] = {""};
		sprintf (S->x_format, "%%03d");
		sprintf (S->y_format, "%%02d");
		if (S->order[1] >= 0) {	/* Need minutes too */
			strcat (S->x_format, S->delimiter[0]);
			strcat (S->y_format, S->delimiter[0]);
			sprintf (fmt, "%%02d");
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		if (S->order[2] >= 0) {	/* .. and seconds */
			strcat (S->x_format, S->delimiter[1]);
			strcat (S->y_format, S->delimiter[1]);
			sprintf (fmt, "%%02d");
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		if (S->n_sec_decimals) {	/* even add format for fractions of second (or minutes or degrees) */
			snprintf (fmt, GMT_LEN64, ".%%%d.%dd", S->n_sec_decimals, S->n_sec_decimals);
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		/* Finally add %s for the W,E,S,N string (or NULL) */
		sprintf (fmt, "%%s");
		strcat (S->x_format, fmt);
		strcat (S->y_format, fmt);
	}
	return (GMT_NOERROR);
}

/*! . */
void gmtlib_plot_C_format (struct GMT_CTRL *GMT) {
	unsigned int i, j, length;
	struct GMT_GEO_IO *S = &GMT->current.plot.calclock.geo;

	/* Determine the plot geographic location formats. */

	for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) gmt_M_memset (GMT->current.plot.format[i][j], GMT_LEN256, char);

	gmtio_get_dms_order (GMT, GMT->current.setting.format_geo_map, S);	/* Get the order of degree, min, sec in output formats */

	if (S->decimal) {	/* Plain decimal degrees */
		int len;
		 /* Here we depend on FORMAT_FLOAT_OUT being set.  This will not be true when FORMAT_GEO_MAP is parsed but will be
		  * handled at the end of gmt_begin.  For gmtset and --PAR later we will be OK as well. */
		if (!GMT->current.setting.format_float_out[0]) return; /* Quietly return and deal with this later in gmt_begin */

		len = sprintf (S->x_format, "%s", GMT->current.setting.format_float_out);
		      sprintf (S->y_format, "%s", GMT->current.setting.format_float_out);
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			S->x_format[len] = (char)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol];
			S->y_format[len] = (char)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol];
			S->x_format[len+1] = S->y_format[len+1] = '\0';
		}
		strcat (S->x_format, "%s");
		strcat (S->y_format, "%s");
	}
	else {			/* Must cover all the 6 forms of dd[:mm[:ss]][.xxx] */
		char fmt[GMT_LEN256] = {""};

		/* Level 0: degrees only. index 0 is integer degrees, index 1 is [possibly] fractional degrees */

		sprintf (GMT->current.plot.format[0][0], "%%d");		/* ddd */
		if (S->order[1] == -1 && S->n_sec_decimals > 0) /* ddd.xxx format */
			snprintf (GMT->current.plot.format[0][1], GMT_LEN64, "%%d.%%%d.%dd", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd format */
			sprintf (GMT->current.plot.format[0][1], "%%d");
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			snprintf (fmt, GMT_LEN256, "%c", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);
			strcat (GMT->current.plot.format[0][0], fmt);
			strcat (GMT->current.plot.format[0][1], fmt);
		}

		/* Level 1: degrees and minutes only. index 0 is integer minutes, index 1 is [possibly] fractional minutes  */

		sprintf (GMT->current.plot.format[1][0], "%%d");	/* ddd */
		sprintf (GMT->current.plot.format[1][1], "%%d");
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);
			strcat (GMT->current.plot.format[1][0], fmt);
			strcat (GMT->current.plot.format[1][1], fmt);
		}
		strcat (GMT->current.plot.format[1][0], "%02d");
		if (S->order[2] == -1 && S->n_sec_decimals > 0) /* ddd:mm.xxx format */
			snprintf (fmt, GMT_LEN256, "%%02d.%%%d.%dd", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm format */
			sprintf (fmt, "%%02d");
		strcat (GMT->current.plot.format[1][1], fmt);
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (GMT->current.setting.map_degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[gmt_squote]);
			strcat (GMT->current.plot.format[1][0], fmt);
			strcat (GMT->current.plot.format[1][1], fmt);
		}

		/* Level 2: degrees, minutes, and seconds. index 0 is integer seconds, index 1 is [possibly] fractional seconds  */

		sprintf (GMT->current.plot.format[2][0], "%%d");
		sprintf (GMT->current.plot.format[2][1], "%%d");
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);
			strcat (GMT->current.plot.format[2][0], fmt);
			strcat (GMT->current.plot.format[2][1], fmt);
		}
		strcat (GMT->current.plot.format[2][0], "%02d");
		strcat (GMT->current.plot.format[2][1], "%02d");
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (GMT->current.setting.map_degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[gmt_squote]);
			strcat (GMT->current.plot.format[2][0], fmt);
			strcat (GMT->current.plot.format[2][1], fmt);
		}
		strcat (GMT->current.plot.format[2][0], "%02d");
		if (S->n_sec_decimals > 0)			 /* ddd:mm:ss.xxx format */
			snprintf (fmt, GMT_LEN256, "%%d.%%%d.%dd", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm:ss format */
			sprintf (fmt, "%%02d");
		strcat (GMT->current.plot.format[2][1], fmt);
		if (GMT->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the second symbol appended */
			if (GMT->current.setting.map_degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)GMT->current.setting.ps_encoding.code[gmt_dquote]);
			strcat (GMT->current.plot.format[2][0], fmt);
			strcat (GMT->current.plot.format[2][1], fmt);
		}

		/* Finally add %s for the [leading space]W,E,S,N char (or NULL) */

		for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) {
			length = (unsigned int)MAX (1, strlen (GMT->current.plot.format[i][j])) - 1;
			if (GMT->current.plot.format[i][j][length] == ':') GMT->current.plot.format[i][j][length] = '\0';	/* Chop off a trailing colon */
			strcat (GMT->current.plot.format[i][j], "%s");
		}
	}
}

/*! . */
int gmt_scanf (struct GMT_CTRL *GMT, char *s, unsigned int expectation, double *val) {
	/* Called with s pointing to a char string, expectation
	indicating what is known/required/expected about the
	format of the string.  Attempts to decode the string to
	find a double value.  Upon success, loads val and
	returns type found.  Upon failure, does not touch val,
	and returns GMT_IS_NAN.  Expectations permitted on call
	are
		GMT_IS_FLOAT	we expect an uncomplicated float.
	*/

	int type;
	char calstring[GMT_LEN64] = {""}, clockstring[GMT_LEN64] = {""}, *p = NULL;
	double x;
	int64_t rd;
	size_t callen, clocklen;

	if (s[0] == '\"') {	/* Must handle double-quoted items */
		callen = strlen (s) - 1;
		if (s[callen] == '\"') { s[callen] = '\0'; s++;}	/* Strip off trailing quote and advance pointer over the first */
	}
	if (s[0] == 'T') {	/* Numbers cannot start with letters except for clocks, e.g., T07:0 */
		if ((int)s[1] < 0 || !isdigit((int)s[1])) return (GMT_IS_NAN);	/* Clocks must have T followed by digit, e.g., T07:0 otherwise junk*/
	}
	else if (strstr (s, "pi") == NULL && isalpha ((int)s[0])) return (GMT_IS_NAN);	/* Numbers cannot start with letters */

	switch (expectation) {
		case GMT_IS_GEO: case GMT_IS_LON: case GMT_IS_LAT:
			/* True if either a lat or a lon is expected  */
			return (gmtio_scanf_geo (s, val));
			break;

	 	case GMT_IS_FLOAT:
			/* True if no special format is expected or allowed  */
			return (gmt_scanf_float (GMT, s, val));
			break;

	 	case GMT_IS_DIMENSION:
			/* True if units might be appended, e.g. 8.4i  */
			return (gmtio_scanf_dim (GMT, s, val));
			break;

	 	case GMT_IS_GEODIMENSION:
			/* True if geo-distance units might be appended, e.g. 8.4d  */
			return (gmtlib_scanf_geodim (GMT, s, val));
			break;

		case GMT_IS_RELTIME:
			/* True if we expect to read a float with no special
			   formatting (except for an optional trailing 't'), and then
			   assume it is relative time in user's units since epoch.  */
#if 0			/* PW: Never accept <number>t in data records.  This is only valid for gmt_scanf_arg */
			callen = strlen (s) - 1;
			if (s[callen] == 't') s[callen] = '\0';
#endif
			if ((gmt_scanf_float (GMT, s, val)) == GMT_IS_NAN) return (GMT_IS_NAN);
			return (GMT_IS_ABSTIME);
			break;

	 	case GMT_IS_ABSTIME:
			/* True when we expect to read calendar and/or
			   clock strings in user-specified formats.  If both
			   are present, they must be in the form
			   <calendar_string>T<clock_string>.
			   If only a calendar string is present, then either
			   <calendar_string> or <calendar_string>T are valid.
			   If only a clock string is present, then it must
			   be preceded by a T:  T<clock_string>, and the time
			   will be treated as if on day one of our calendar.  */
			callen = strlen (s);
			if (callen < 2) return (GMT_IS_NAN);	/* Maybe should be more than 2  */

			if (s[callen-1] == 'T') {	/* Got <date>T with no <clock> */
				clocklen = 0;
				if (GMT->current.io.date_input.watch)	/* Watch for shit like 2013-23-OCT */
					strncpy (calstring, s, callen);
				else
					strncpy (calstring, s, callen-1);
			}
			else if (s[0] == 'T') {	/* Got T<clock> presumably, with no <date> */
				strncpy (clockstring, &s[1], GMT_LEN64-1);
				clocklen = callen - 1;
				callen = 0;
			}
			else if ((p = strrchr (s, 'T'))) {	/* There is a T in there (but could be stuff like 2012-OCT-20 with no trailing T) */
				char *p2 = NULL;
				/* Watch for shit like 2013-OCT-23 with no trailing T */
				if (GMT->current.io.date_input.mw_text && GMT->current.io.date_input.delimiter[0][0] && (p2 = strrchr (s, GMT->current.io.date_input.delimiter[0][0])) > p) {
					/* Got a delimiter after that T, so assume it is a T in a name instead */
					strncpy (calstring, s, GMT_LEN64-1);
					clocklen = 0;
				}
				else {	/* Regular <date>T<clock> */
					*p = ' ';	/* Temporarily replace T with space */
					sscanf (s, "%s %s", calstring, clockstring);
					clocklen = strlen (clockstring);
					callen = strlen (calstring);
				}
			}
			else {	/* There is no trailing T.  Put all of s in calstring.  */
				clocklen = 0;
				strncpy (calstring, s, GMT_LEN64-1);
			}
			x = 0.0;	/* Default to 00:00:00 if no clock is given */
			if (clocklen && gmtio_scanf_clock (GMT, clockstring, &x)) return (GMT_IS_NAN);
			rd = GMT->current.time.today_rata_die;	/* Default to today if no date is given */
			if (callen && gmtio_scanf_calendar (GMT, calstring, &rd)) return (GMT_IS_NAN);
			*val = gmt_rdc2dt (GMT, rd, x);
			if (GMT->current.setting.time_is_interval) {	/* Must truncate and center on time interval */
				gmtlib_moment_interval (GMT, &GMT->current.time.truncate.T, *val, true);	/* Get the current interval */
				if (GMT->current.time.truncate.direction) {	/* Actually need midpoint of previous interval... */
					x = GMT->current.time.truncate.T.dt[0] - 0.5 * (GMT->current.time.truncate.T.dt[1] - GMT->current.time.truncate.T.dt[0]);
					gmtlib_moment_interval (GMT, &GMT->current.time.truncate.T, x, true);	/* Get the current interval */
				}
				/* Now get half-point of interval */
				*val = 0.5 * (GMT->current.time.truncate.T.dt[1] + GMT->current.time.truncate.T.dt[0]);
			}
			return (GMT_IS_ABSTIME);
			break;

	 	case GMT_IS_ARGTIME:
			return (gmt_scanf_argtime (GMT, s, val));
			break;

		case  GMT_IS_UNKNOWN:
			/* True if we don't know but must try both geographic or float formats  */
			type = gmtio_scanf_geo (s, val);
			if ((type == GMT_IS_LON) && GMT->current.io.warn_geo_as_cartesion) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "GMT: Longitude input data detected and successfully converted but will be considered Cartesian coordinates.\n");
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "GMT: If you need longitudes to be processed as periodic in 360 degrees then you must use -fg.\n");
				GMT->current.io.warn_geo_as_cartesion = false;	/* OK, done with the warning */
			}
			return (type);
			break;

		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "GMT_LOGIC_BUG: gmt_scanf() called with invalid expectation.\n");
			return (GMT_IS_NAN);
	}
}

/*! . */
GMT_LOCAL unsigned int n_trailing_chars (struct GMT_CTRL *GMT, char *text) {
	/* Try to determine if there are trailing text that is not a valid unit for a number.
	 * We do not try to scan for leading chars since Jan-01-2001T might be caught. */
	unsigned int n = 0, p = 0, n_periods = 0;	/* n is number of trailing characters after last digit (or period) */
	int k, last;
	gmt_M_unused (GMT);
	if (!text || !text[0]) return 0;
	k = last = (int)strlen(text) - 1;
	while (k >= 0 && !isdigit (text[k])) {	/* Count how many non-digits at the end of this text */
		if (text[k] == '.') p = k, n_periods++;	/* But be aware of a trailing single period in a number before unit (e.g., 30.k) */
		k--, n++;	/* Count trailing letters */
	}
	if (n_periods == 1 && p && isdigit (text[p-1]) && n <= 2)	/* May be OK if a valid unit (or nothing) afterwards and a digit before */
		n--;	/* Let the period be part of the number */
	if (n == 1 && !strchr (GMT_LEN_UNITS GMT_DIM_UNITS GMT_WESN_UNITS, text[last]))	/* Single trailing text letter but not recognized as a unit */
		n = 2;	/* To ensure it is seen as text */
	return (n);
}

/*! . */
int gmt_scanf_arg (struct GMT_CTRL *GMT, char *s, unsigned int expectation, bool cmd, double *val) {
	/* Version of gmt_scanf used for cpt & command line arguments only (not data records).
	 * It differs from gmt_scanf in that if the expectation is GMT_IS_UNKNOWN it will
	 * check to see if the argument is (1) an absolute time string, (2) a geographical
	 * location string, or if not (3) a floating point string, etc.  We carefully check
	 * to detect free-form strings which should result in a NaN.
	 * When cmd is false then we disallow the check for trailing DdGg meaning geographic
	 * since that is only valid on command line and in a data record the "d" mans degree.
	 */

	unsigned int got;

	if (s == NULL) {	/* Cannot do anything here */
		*val = GMT->session.d_NaN;
		return GMT_IS_NAN;
	}

	if (expectation == GMT_IS_UNKNOWN) {	/* Expectation for this item was not set - must be determined if possible */
		size_t len = strlen (s);
		if (len == 0) {	/* Cannot do anything here */
			*val = GMT->session.d_NaN;
			return GMT_IS_NAN;
		}
		if (len > 1) {		/* Arguments of at least 2 characters can be many things */
			char c = s[len-1];	/* Trailing letter */
			unsigned int nt = (cmd) ? 0 : n_trailing_chars (GMT, s);
			if ((s[0] == 'T' && isdigit (s[1])) || strchr (s, 'T'))	/* Found a T in the argument - must be Absolute time or it will fail as junk */
				expectation = GMT_IS_ARGTIME;
			else if (strstr (s, "pi"))	/* Found "pi" in the number - will try scanning as float */
				expectation = GMT_IS_FLOAT;
			else if (c == 't')		/* Found trailing t - assume Relative time */
				expectation = GMT_IS_ARGTIME;
			else if (nt > 1 || gmt_not_numeric (GMT, s)) {	/* No number has 2 or more letters at the end, or other junk, so return as NaN */
				*val = GMT->session.d_NaN;
				return GMT_IS_NAN;
			}
			else if (!((s[0] >= 0 && isdigit (s[0])) || s[0] == '-' || s[0] == '+' || s[0] == '.')) {	/* All other numbers must be [-|+][<num>[.]<num>][<end>] */
				*val = GMT->session.d_NaN;
				return GMT_IS_NAN;	/* Cannot be a number so return as NaN */
			}
			else if (strchr ("WE", c))		/* Found trailing W or E - assume Geographic longitudes */
				expectation = GMT_IS_LON;
			else if (strchr ("SN", c))		/* Found trailing S or N - assume Geographic latitudes */
				expectation = GMT_IS_LAT;
			else if (cmd && strchr ("DdGg", c))	/* Found trailing G or D - assume Geographic coordinate */
				expectation = GMT_IS_GEO;
			else if (strchr (s, ':'))		/* Found a : in the argument - assume Geographic coordinates */
				expectation = GMT_IS_GEO;
			else if (strchr (GMT_DIM_UNITS, c))	/* Found a trailing dimension unit (c|i|p) */
				expectation = GMT_IS_DIMENSION;
			else if (strchr (GMT_LEN_UNITS, c))	/* Found a trailing geo-length unit (d|m|s|e|f|k|M|n|u) */
				expectation = GMT_IS_GEODIMENSION;
			else 					/* Found nothing - assume floating point */
				expectation = GMT_IS_FLOAT;
		}
		else if (isdigit (s[0]))				/* Found a 1-char digit */
			expectation = GMT_IS_FLOAT;
		else {	/* Must be a single letter not digit */
			*val = GMT->session.d_NaN;
			return GMT_IS_NAN;	/* Cannot be a number so return as NaN */
		}
	}

	/* OK, here we have an expectation, now call gmt_scanf */

	got = gmt_scanf (GMT, s, expectation, val);
	return (got == GMT_IS_NAN) ? got : expectation;	/* Want to return the actual expectation here since it may be used upstream, unless parsing failed */
}

/*! . */
void gmt_set_seg_minmax (struct GMT_CTRL *GMT, unsigned int geometry, unsigned int n_cols, struct GMT_DATASEGMENT *S) {
	/* Determine the min/max values for each column in the segment.
	 * If n_cols > 0 then we only update the first n_cols */
	uint64_t row, col;

	/* In case the creation of the segment did not allocate min/max do it now */
	if (!S->min) S->min = gmt_M_memory (GMT, NULL, S->n_columns, double);
	if (!S->max) S->max = gmt_M_memory (GMT, NULL, S->n_columns, double);
	if (S->n_rows == 0) return;	/* Nothing more we can do */
	if (n_cols == 0) n_cols = (unsigned int)S->n_columns;	/* Set number of columns to work on */
	for (col = 0; col < n_cols; col++) {
		if (gmt_M_type (GMT, GMT_IN, col) == GMT_IS_LON) /* Requires separate quandrant assessment */
			gmtlib_get_lon_minmax (GMT, S->data[col], S->n_rows, &(S->min[col]), &(S->max[col]));
		else {	/* Simple Cartesian-like arrangement */
			S->min[col] = S->max[col] = S->data[col][0];
			for (row = 1; row < S->n_rows; row++) {
				if (S->data[col][row] < S->min[col]) S->min[col] = S->data[col][row];
				if (S->data[col][row] > S->max[col]) S->max[col] = S->data[col][row];
			}
		}
	}
	if (geometry & GMT_IS_POLY)
		gmt_set_seg_polar (GMT, S);
}

/*! . */
void gmt_set_tbl_minmax (struct GMT_CTRL *GMT, unsigned int geometry, struct GMT_DATATABLE *T) {
	/* Update the min/max of all segments and the entire table */
	uint64_t seg, col;
	struct GMT_DATASEGMENT *S = NULL;

	if (!T) return;	/* No table given */
	if (!T->n_columns) return;	/* No columns given */
	if (!T->min) T->min = gmt_M_memory (GMT, NULL, T->n_columns, double);
	if (!T->max) T->max = gmt_M_memory (GMT, NULL, T->n_columns, double);
	for (col = 0; col < T->n_columns; col++) {	/* Initialize */
		T->min[col] = DBL_MAX;
		T->max[col] = -DBL_MAX;
	}
	T->n_records = 0;
	for (seg = 0; seg < T->n_segments; seg++) {
		S = T->segment[seg];
		gmt_set_seg_minmax (GMT, geometry, 0, S);
		if (S->n_rows == 0) continue;
		for (col = 0; col < T->n_columns; col++) {
			if (S->min[col] < T->min[col]) T->min[col] = S->min[col];
			if (S->max[col] > T->max[col]) T->max[col] = S->max[col];
		}
		T->n_records += S->n_rows;
	}
}

/*! . */
void gmt_set_dataset_minmax (struct GMT_CTRL *GMT, struct GMT_DATASET *D) {
	uint64_t tbl, col;
	struct GMT_DATATABLE *T = NULL;
	if (!D) return;	/* No dataset given */
	if (!D->n_columns) return;	/* No columns given */
	if (!D->min) D->min = gmt_M_memory (GMT, NULL, D->n_columns, double);
	if (!D->max) D->max = gmt_M_memory (GMT, NULL, D->n_columns, double);
	for (col = 0; col < D->n_columns; col++) {	/* Initialize */
		D->min[col] = DBL_MAX;
		D->max[col] = -DBL_MAX;
	}
	D->n_records = D->n_segments = 0;
	for (tbl = 0; tbl < D->n_tables; tbl++) {
		T = D->table[tbl];
		gmt_set_tbl_minmax (GMT, D->geometry, T);
		for (col = 0; col < D->n_columns; col++) {
			if (T->min[col] < D->min[col]) D->min[col] = T->min[col];
			if (T->max[col] > D->max[col]) D->max[col] = T->max[col];
		}
		D->n_records += T->n_records;
		D->n_segments += T->n_segments;
	}
}

/*! . */
int gmt_parse_segment_header (struct GMT_CTRL *GMT, char *header, struct GMT_PALETTE *P, bool *use_fill, struct GMT_FILL *fill, struct GMT_FILL *def_fill,  bool *use_pen, struct GMT_PEN *pen, struct GMT_PEN *def_pen, unsigned int def_outline, struct GMT_OGR_SEG *G) {
	/* Scan header for occurrences of valid GMT options.
	 * The possibilities are:
	 * Fill: -G<fill>	Use the new fill and turn filling ON
	 *	 -G-		Turn filling OFF
	 *	 -G		Revert to default fill [none if not set on command line]
	 * Pens: -W<pen>	Use the new pen and turn outline ON
	 *	 -W-		Turn outline OFF
	 *	 -W		Revert to default pen [current.map_default_pen if not set on command line]
	 * z:	-Z<zval>	Obtain fill via cpt lookup using this z value
	 *	-ZNaN		Get the NaN color from the CPT
	 *
	 * header is the text string to process
	 * P is the color palette used for the -Z option
	 * use_fill is set to true, false or left alone if no change
	 * fill is the fill structure to use after this function returns
	 * def_fill holds the default fill (if any) to use if -G is found
	 * use_pen is set to true, false or left alone if no change
	 * pen is the pen structure to use after this function returns
	 * def_pen holds the default pen (if any) to use if -W is found
	 * def_outline holds the default outline setting (true/false)
	 *
	 * The function returns the sum of the following return codes:
	 * 0 = No fill, no outline
	 * 1 = Revert to default fill or successfully parsed a -G<fill> option
	 * 2 = Encountered a -Z option to set the fill based on a CPT
	 * 4 = Encountered a -W<pen> option
	 * For OGR/GMT files similar parsing occurs in that aspatial values assigned to the magic
	 * columns D, G, L, W, Z are used in the same fashion.
	 */

	unsigned int processed = 0, change = 0, col, ogr_col;
	char line[GMT_BUFSIZ] = {""}, *txt = NULL;
	double z;
	struct GMT_FILL test_fill;
	struct GMT_PEN test_pen;

	if (GMT->common.a.active) {	/* Use aspatial data instead */
#if 0 /* Just for testing OGR stuff */
		for (col = 0; col < GMT->current.io.OGR->n_aspatial; col++)
			fprintf (stderr, "OGR %d: N = %s T = %d\n", col, GMT->current.io.OGR->name[col], GMT->current.io.OGR->type[col]);
		for (col = 0; col < GMT->common.a.n_aspatial; col++)
			fprintf (stderr, "a: %d: C = %d O = %d, N = %s\n",
			         col, GMT->common.a.col[col], GMT->common.a.ogr[col], GMT->common.a.name[col]);
		for (col = 0; col < G->n_aspatial; col++)
			fprintf (stderr, "G: %d: V = %s\n", col, G->tvalue[col]);
#endif
		for (col = 0; col < GMT->common.a.n_aspatial; col++) {
			if (GMT->common.a.col[col] >= 0) continue;	/* Skip regular data column fillers */
			if (!G && !GMT->current.io.OGR->tvalue) continue;	/* Nothing set yet */
			if (!G && !GMT->current.io.OGR->dvalue) continue;	/* Nothing set yet */
			ogr_col = GMT->common.a.ogr[col];
			txt = (G) ? G->tvalue[ogr_col] : GMT->current.io.OGR->tvalue[col];
			z = (G) ? G->dvalue[ogr_col] : GMT->current.io.OGR->dvalue[col];
			switch (GMT->common.a.col[col]) {
				case GMT_IS_G:
					if (gmt_getfill (GMT, txt, fill)) {
						*use_fill = true;
						change = 1;
						processed++;	/* Processed one option */
					}
					break;
				case GMT_IS_W:
					if (gmt_getpen (GMT, txt, pen)) {
						*use_pen = true;
						change |= 4;
					}
					break;
				case GMT_IS_Z:
					if (P->categorical & 2)
						gmt_get_fill_from_key (GMT, P, txt, fill);
					else
						gmt_get_fill_from_z (GMT, P, z, fill);
					*use_fill = true;
					change |= 2;
					processed++;	/* Processed one option */
					break;
			}
		}
		return (change);
	}

	/* Standard GMT multisegment parsing */

	if (!header || !header[0]) return (0);

	if (gmt_parse_segment_item (GMT, header, "-G", line)) {	/* Found a potential -G option */
		test_fill = *def_fill;
		if (line[0] == '-') {	/* Turn fill OFF */
			fill->rgb[0] = fill->rgb[1] = fill->rgb[2] = -1.0, fill->use_pattern = false;
			*use_fill = false;
			processed++;	/* Processed one option */
		}
		else if (!line[0] || line[0] == '+') {	/* Revert to default fill */
			*fill = *def_fill;
			*use_fill = (def_fill->use_pattern || def_fill->rgb[0] != -1.0);
			if (*use_fill) change = 1;
			processed++;	/* Processed one option */
		}
		else if (!gmt_getfill (GMT, line, &test_fill)) {	/* Successfully processed a -G<fill> option */
			*fill = test_fill;
			*use_fill = true;
			change = 1;
			processed++;	/* Processed one option */
		}
		/* Failure is OK since -Gjunk may appear in text strings - we then do nothing (hence no else clause) */
	}
	if (P && gmt_parse_segment_item (GMT, header, "-Z", line)) {	/* Found a potential -Z option to set symbol r/g/b via cpt-lookup */
		if(!strncmp (line, "NaN", 3U))	{	/* Got -ZNaN */
			gmt_get_fill_from_z (GMT, P, GMT->session.d_NaN, fill);
			*use_fill = true;
			change |= 2;
			processed++;	/* Processed one option */
		}
		else {
			if (P->categorical & 2)
				gmt_get_fill_from_key (GMT, P, line, fill);
			else if (sscanf (line, "%lg", &z) == 1)
				gmt_get_fill_from_z (GMT, P, z, fill);
			*use_fill = true;
			change |= 2;
			processed++;	/* Processed one option */
		}
		/* Failure is OK since -Zjunk may appear in text strings - we then do nothing (hence no else clause) */
	}

	if (processed == 2) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Segment header has both -G and -Z options\n");	/* Giving both -G and -Z is a problem */

	if (gmt_parse_segment_item (GMT, header, "-W", line)) {	/* Found a potential -W option */
		test_pen = *def_pen;	/* Set test pen to the default, may be overruled later */
		if (line[0] == '-') {	/* Turn outline OFF */
			*pen = *def_pen;	/* Set pen to default */
			*use_pen = false;
		}
		else if (!line[0] || line[0] == '+') {	/* Revert to default pen/outline */
			*pen = *def_pen;	/* Set pen to default */
			*use_pen = def_outline;
			if (def_outline) change |= 4;
		}
		else if (!gmt_getpen (GMT, line, &test_pen)) {
			*pen = test_pen;
			*use_pen = true;
			change |= 4;
		}
		/* Failure is OK since -W may appear in text strings (hence no else clause) */
	}
	return (change);
}

/*! . */
void gmt_extract_label (struct GMT_CTRL *GMT, char *line, char *label, struct GMT_OGR_SEG *G) {
	/* Pull out the label in a -L<label> option in a segment header w./w.o. quotes.
 	 * If G is not NULL we use it (OGR stuff) instead. */
	bool done;
	unsigned int i = 0, k, j, j0;
	char *p = NULL, q[2] = {'\"', '\''};

	if (G && G->tvalue && G->tvalue[0]) { strcpy (label, G->tvalue[0]); return ;}	/* Had an OGR segment label */
	if (gmt_parse_segment_item (GMT, line, "-L", label)) return;	/* Found -L */

	label[0] = '\0';	/* Remove previous label */
	if (!line || !line[0]) return;	/* Line is empty */
	while (line[i] && (line[i] == ' ' || line[i] == '\t')) i++;	/* Bypass whitespace */

	for (k = 0, done = false; k < 2; k++) {
		if ((p = strchr (&line[i], q[k]))) {	/* Gave several double/single-quoted words as label */
			for (j0 = j = i + 1; line[j] != q[k]; j++);
			if (line[j] == q[k]) {	/* Found the matching quote */
				strncpy (label, &line[j0], j-j0);
				label[j-j0] = '\0';
				done = true;
			}
			else {			/* Missing the matching quote */
				sscanf (&line[i], "%s", label);
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Label (%s) not terminated by matching quote\n", label);
			}
		}
	}
	if (!done) sscanf (&line[i], "%s", label);
}

/*! . */
bool gmt_parse_segment_item (struct GMT_CTRL *GMT, char *in_string, char *pattern, char *out_string) {
	/* Scans the in_string for the occurrence of an option switch (e.g, -L) and
	 * if found, extracts the argument and returns it via out_string.  Function
	 * return true if the pattern was found and false otherwise.
	 * out_string must be allocated and have space for the copying */
	char *t = NULL;
	size_t k;
	gmt_M_unused(GMT);
	if (!in_string || !pattern) return (false);	/* No string or pattern passed */
	if (!(t = strstr (in_string, pattern))) return (false);	/* Option not present */
	if (!out_string) return (true);	/* If NULL is passed as out_string then we just return true if we find the option */
	out_string[0] = '\0';	/* Reset string to empty before we try to set it below */
	k = (size_t)t - (size_t)in_string; /* Position of pattern in in_string */
	if (k && !(in_string[k-1] == ' ' || in_string[k-1] == '\t')) return (false);	/* Option not first or preceded by whitespace */
	t += 2;	/* Position of the argument */
	if (t[0] == '\"')	/* Double quoted argument, must scan from next character until terminal quote */
		sscanf (++t, "%[^\"]", out_string);
	else if (t[0] == '\'')	/* Single quoted argument, must scan from next character until terminal quote */
		sscanf (++t, "%[^\']", out_string);
	else	/* Scan until next white space; stop also when there is leading white space, indicating no argument at all! */
		sscanf (t, "%[^ \t]", out_string);
	return (true);
}

/*! . */
void gmt_duplicate_ogr_seg (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S_to, struct GMT_DATASEGMENT *S_from) {
	/* Allocates the OGR structure for a given segment and copies current values from table OGR segment */
	unsigned int col;
	struct GMT_DATASEGMENT_HIDDEN *SH_to = gmt_get_DS_hidden (S_to), *SH_from = gmt_get_DS_hidden (S_from);

	if (!SH_from->ogr) return;	/* No data */
	gmtio_alloc_ogr_seg (GMT, S_to, SH_from->ogr->n_aspatial);
	for (col = 0; col < SH_from->ogr->n_aspatial; col++) {
		if (SH_from->ogr->tvalue[col]) SH_to->ogr->tvalue[col] = strdup (SH_from->ogr->tvalue[col]);
		SH_to->ogr->dvalue[col] = SH_from->ogr->dvalue[col];
	}
	SH_to->ogr->pol_mode = SH_from->ogr->pol_mode;
}

/*! . */
void gmtlib_write_newheaders (struct GMT_CTRL *GMT, FILE *fp, uint64_t n_cols) {
	/* Common ASCII header records added on output */
	if (GMT->common.b.active[GMT_OUT]) return;		/* No output headers for binary files */
	if (!GMT->current.setting.io_header[GMT_OUT]) return;	/* No output headers requested, so don't bother */
	if (GMT->common.h.title) {	/* Optional title(s) provided; could be several lines separated by \n */
		gmtio_write_multilines (GMT, fp, GMT->common.h.title, "Title");
	}

	if (GMT->common.h.multi_segment) {	/* A multi-segment record */
		gmtlib_write_tableheader (GMT, fp, gmtapi_create_header_item (GMT->parent, GMT_COMMENT_IS_MULTISEG, GMT->common.h.multi_segment));
		return;
	}

	/* Always write command line */
	gmtlib_write_tableheader (GMT, fp, gmtapi_create_header_item (GMT->parent, GMT_COMMENT_IS_COMMAND | GMT_COMMENT_IS_OPTION, GMT->current.options));
	if (GMT->common.h.remark) {	/* Optional remark(s) provided; could be several lines separated by \n */
		gmtio_write_multilines (GMT, fp, GMT->common.h.remark, "Remark");
	}
	if (GMT->common.h.add_colnames) {	/* Want output comment with column names */
		if (GMT->common.h.colnames)	/* Optional column names already provided */
			gmtlib_write_tableheader (GMT, fp, GMT->common.h.colnames);
		else if (n_cols) {	/* Generate names col1[0], col2[1] etc */
			uint64_t col, first = 1;
			char record[GMT_BUFSIZ] = {""}, txt[GMT_LEN64] = {""};
			if (n_cols >= 2) {	/* Place x and y first */
				gmt_set_xycolnames (GMT, record);
				first++;
			}
			else
				sprintf (record, "col1[0]");
			for (col = first; col < n_cols; col++) {
				snprintf (txt, GMT_LEN64, "\tcol%" PRIu64 "[%" PRIu64 "]", col+1, col);
				strcat (record, txt);
			}
			gmtlib_write_tableheader (GMT, fp, record);
		}
	}
}

/*! . */
void gmt_set_xycolnames (struct GMT_CTRL *GMT, char *string) {
	char *xy[2][2] = {{"x", "y"}, {"lon", "lat"}};
	unsigned int mode = (gmt_M_is_geographic (GMT, GMT_OUT)) ? 1 : 0;
	unsigned int ix = (GMT->current.setting.io_lonlat_toggle[GMT_OUT]) ? 1 : 0, iy;
	iy = 1 - ix;
	sprintf (string, "%s[0]\t%s[1]", xy[mode][ix], xy[mode][iy]);
}

GMT_LOCAL void gmtio_finalize_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S) {
	/* Reallocate arrays to fit current segment size */
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	if (S->n_rows > SH->n_alloc) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtio_finalize_segment: Internal error: S->n_rows > SH->n_alloc!!!");
		return;
	}
	if (S->n_rows < SH->n_alloc) {
		uint64_t col;
		for (col = 0; col < S->n_columns; col++)
			S->data[col] = gmt_M_memory (GMT, S->data[col], S->n_rows, double);
		if (S->text) S->text = gmt_M_memory (GMT, S->text, S->n_rows, char *);
		SH->n_alloc = S->n_rows;
	}
}

GMT_LOCAL void gmtio_finalize_table (struct GMT_CTRL *GMT, struct GMT_DATATABLE *T) {
	/* Reallocate size of segments to fit current counts */
	uint64_t seg;
	struct GMT_DATATABLE_HIDDEN *TH = gmt_get_DT_hidden (T);
	if (T->n_segments > TH->n_alloc) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtio_finalize_table: Internal error: T->n_segments > TH->n_alloc!!!");
		return;
	}
	if (T->n_segments < TH->n_alloc) {
		for (seg = T->n_segments; seg < TH->n_alloc; seg++)
			gmt_free_segment (GMT, &(T->segment[seg]));
		T->segment = gmt_M_memory (GMT, T->segment, T->n_segments, struct GMT_DATASEGMENT *);
		TH->n_alloc = T->n_segments;
	}
	for (seg = 0; seg < T->n_segments; seg++) gmtio_finalize_segment (GMT, T->segment[seg]);
}

void gmtlib_finalize_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *D) {
	/* Reallocate size of items to fit current counts */
	uint64_t tbl;
	struct GMT_DATASET_HIDDEN *DH = gmt_get_DD_hidden (D);
	if (D->n_tables > DH->n_alloc) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_finalize_dataset: Internal error: D->n_tables > DH->alloc!!!");
		return;
	}
	if (D->n_tables < DH->n_alloc) {
		D->table = gmt_M_memory (GMT, D->table, D->n_tables, struct GMT_DATATABLE *);
		DH->n_alloc = D->n_tables;
	}
	for (tbl = 0; tbl < D->n_tables; tbl++) gmtio_finalize_table (GMT, D->table[tbl]);
}

/*! . */
void gmt_adjust_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *D, uint64_t n_columns) {
	/* Adjust existing data set structure to have n_columns instead.  This may
	 * involve shrinking (deallocation) or growing (allocation) of columns.
	 */
	uint64_t tbl;

	for (tbl = 0; tbl < D->n_tables; tbl++) gmtio_adjust_table (GMT, D->table[tbl], n_columns);
	D->n_columns = n_columns;
}

/*! . */
int gmt_alloc_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, uint64_t n_rows, uint64_t n_columns, unsigned int mode, bool first) {
	/* (re)allocates memory for a segment of given dimensions.
	 * If first is true then the segment arrays for data/text are allocated and min/max arrays set
	 */
	uint64_t col;
	if (first && n_columns) {	/* First time we allocate the number of columns needed */
		S->data = gmt_M_memory (GMT, NULL, n_columns, double *);
		S->min = gmt_M_memory (GMT, NULL, n_columns, double);
		S->max = gmt_M_memory (GMT, NULL, n_columns, double);
		for (col = 0; col < n_columns; col++) {	/* Initialize the min/max array */
			S->min[col] = +DBL_MAX;
			S->max[col] = -DBL_MAX;
		}
		S->n_columns = n_columns;
	}
	if (!first && S->n_columns != n_columns) {	/* Error */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_alloc_segment: Cannot reallocate the number of columns in an existing segment");
		return 1;
	}
	S->n_rows  = n_rows;
	if (n_rows) {	/* Allocate or reallocate column arrays */
		struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
		for (col = 0; col < n_columns; col++) {
			if ((S->data[col] = gmt_M_memory (GMT, S->data[col], n_rows, double)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_alloc_segment: Unable to reallocate data column %" PRIu64 " to new length %" PRIu64 "\n", col, n_rows);
				return 1;
			}
		}
		if (mode & GMT_WITH_STRINGS) {	/* Also allocate the string pointers */
			if ((S->text = gmt_M_memory (GMT, S->text, n_rows, char *)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_alloc_segment: Unable to reallocate string array new length %" PRIu64 "\n", n_rows);
				return 1;
			}
		}
		SH->alloc_mode = GMT_ALLOC_INTERNALLY;
		SH->n_alloc = n_rows;
	}
	return (GMT_OK);
}

/*! . */
void gmtlib_assign_segment (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_DATASEGMENT *S, uint64_t n_rows, uint64_t n_columns) {
	/* Allocates and memcpy over vectors from GMT->hidden.mem_coord.
  	 * If n_rows > GMT_INITIAL_MEM_ROW_ALLOC then we pass the arrays and reset the tmp arrays to NULL
	 */
	uint64_t col;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	if (n_rows == 0) return;	/* Nothing to do */
	/* First allocate struct member arrays */
	S->data = gmt_M_memory (GMT, NULL, n_columns, double *);
	S->min  = gmt_M_memory (GMT, NULL, n_columns, double);
	S->max  = gmt_M_memory (GMT, NULL, n_columns, double);

	if (n_rows > GMT_INITIAL_MEM_ROW_ALLOC) {	/* Large segment, just pass allocated pointers and start over with new tmp vectors later */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmtlib_assign_segment: Pass %" PRIu64 " large arrays with length = %"
		            PRIu64 " off and get new tmp arrays\n", n_columns, n_rows);
		for (col = 0; col < n_columns; col++) {	/* Initialize the min/max array */
			if (n_rows < GMT->hidden.mem_rows)
				GMT->hidden.mem_coord[col] = gmt_M_memory (GMT, GMT->hidden.mem_coord[col], n_rows, double);	/* Trim back */
			S->data[col] = GMT->hidden.mem_coord[col];	/* Pass the pointer */
			GMT->hidden.mem_coord[col] = NULL;		/* Null this out to start over for next segment */
		}
		if (GMT->current.io.record_type[direction] & GMT_READ_TEXT) {
			if (n_rows < GMT->hidden.mem_rows)
				GMT->hidden.mem_txt = gmt_M_memory (GMT, GMT->hidden.mem_txt, n_rows, char *);	/* Trim back */
			S->text = GMT->hidden.mem_txt;	/* Pass the pointer */
			GMT->hidden.mem_txt = NULL;		/* Null this out to start over for next segment */
		}
		GMT->hidden.mem_cols = 0;	/* Flag that we need to reallocate new temp arrays for next segment, if any */
	}
	else {	/* Small segments, allocate and memcpy, leave tmp array as is for further use */
		for (col = 0; col < n_columns; col++) {	/* Initialize the min/max array */
			S->data[col] = gmt_M_memory (GMT, S->data[col], n_rows, double);
			gmt_M_memcpy (S->data[col], GMT->hidden.mem_coord[col], n_rows, double);
		}
		if (GMT->current.io.record_type[direction] & GMT_READ_TEXT) {
			uint64_t row;
			S->text = gmt_M_memory (GMT, S->text, n_rows, char *);
			for (row = 0; row < n_rows; row++) {
				S->text[row] = GMT->hidden.mem_txt[row];
				GMT->hidden.mem_txt[row] = NULL;
			}
		}
	}
	S->n_rows = n_rows;
	S->n_columns = n_columns;
	SH->alloc_mode = GMT_ALLOC_INTERNALLY;
}

/*! . */
double *gmtlib_assign_vector (struct GMT_CTRL *GMT, uint64_t n_rows, uint64_t col) {
	/* Allocates and memcpy over vectors from GMT->hidden.mem_coord.
  	 * If n_rows > GMT_INITIAL_MEM_ROW_ALLOC then we pass the arrays and reset the tmp arrays to NULL.
	 */
	double *vector = NULL;
	if (n_rows == 0) return NULL;	/* Nothing to do */

	if (n_rows > GMT_INITIAL_MEM_ROW_ALLOC) {	/* Large segment, just pass allocated pointers and start over with new tmp vectors later */
		if (n_rows < GMT->hidden.mem_rows)
			GMT->hidden.mem_coord[col] = gmt_M_memory (GMT, GMT->hidden.mem_coord[col], n_rows, double);	/* Trim back */
		vector = GMT->hidden.mem_coord[col];	/* Pass the pointer */
		GMT->hidden.mem_coord[col] = NULL;	/* Null this out to start over for next segment */
		GMT->hidden.mem_cols = 0;	/* Flag that we need to reallocate new temp arrays for next segment, if any */
	}
	else {	/* Small segments, allocate and memcpy, leave tmp array as is for further use */
		vector = gmt_M_memory (GMT, NULL, n_rows, double);
		gmt_M_memcpy (vector, GMT->hidden.mem_coord[col], n_rows, double);
	}
	return (vector);
}

struct GMT_DATASET * gmt_get_dataset (struct GMT_CTRL *GMT) {
	struct GMT_DATASET *D = NULL;
	D = gmt_M_memory (GMT, NULL, 1, struct GMT_DATASET);
	D->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_DATASET_HIDDEN);
	return (D);
}

struct GMT_DATATABLE * gmt_get_table (struct GMT_CTRL *GMT) {
	struct GMT_DATATABLE *T = NULL;
	T = gmt_M_memory (GMT, NULL, 1, struct GMT_DATATABLE);
	T->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_DATATABLE_HIDDEN);
	return (T);
}

struct GMT_DATASEGMENT * gmt_get_segment (struct GMT_CTRL *GMT) {
	struct GMT_DATASEGMENT *S = NULL;
	S = gmt_M_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
	S->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_DATASEGMENT_HIDDEN);
	return (S);
}

/*! . */
struct GMT_DATATABLE * gmt_create_table (struct GMT_CTRL *GMT, uint64_t n_segments, uint64_t n_rows, uint64_t n_columns, unsigned int mode, bool alloc_only) {
	/* Allocate the new Table structure given the specified dimensions.
	 * If n_rows == 0 it means we don't have any data yet.
	 * If n_columns == 0 it means we don't know that dimension yet.
	 * If alloc_only is true then we do NOT set the corresponding counters (i.e., n_segments),
	 * else these are set to reflect the new sizes. */
	uint64_t seg;
	bool alloc = (n_columns || mode & GMT_WITH_STRINGS);
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;

	T = gmt_get_table (GMT);
	TH = gmt_get_DT_hidden (T);
	if (!alloc_only) T->n_segments = n_segments;
	if (!alloc_only) T->n_records = n_segments * n_rows;
	TH->n_alloc = n_segments;
	if (n_columns) {
		T->min = gmt_M_memory (GMT, NULL, n_columns, double);
		T->max = gmt_M_memory (GMT, NULL, n_columns, double);
	}
	T->n_columns = n_columns;
	if (n_segments) {
		T->segment = gmt_M_memory (GMT, NULL, n_segments, struct GMT_DATASEGMENT *);
		for (seg = 0; alloc && seg < n_segments; seg++) {
			T->segment[seg] = gmt_get_segment (GMT);
			gmt_alloc_segment (GMT, T->segment[seg], n_rows, n_columns, mode, true);
			if (alloc_only) T->segment[seg]->n_rows = 0;
		}
	}

	return (T);
}

/*! . */
struct GMT_DATASET * gmtlib_create_dataset (struct GMT_CTRL *GMT, uint64_t n_tables, uint64_t n_segments, uint64_t n_rows, uint64_t n_columns, unsigned int geometry, unsigned int mode, bool alloc_only) {
	/* Create an empty data set structure with the required number of empty tables, all set to hold n_segments, each with n_rows holding n_columns.
	 * if mode & GMT_WITH_STRINGS we also allocate arrays to hold trailing text.
	 * If alloc_only is true then we leave the counters (n_records, n_rows) unchanged, else these
	 * are updated to reflect the new allocated sizes. */
	uint64_t tbl;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASET_HIDDEN *DH = NULL;

	D = gmt_get_dataset (GMT);
	DH = gmt_get_DD_hidden (D);
	if (n_columns) {
		D->min = gmt_M_memory (GMT, NULL, n_columns, double);
		D->max = gmt_M_memory (GMT, NULL, n_columns, double);
	}
	D->n_columns = n_columns;
	D->geometry = geometry;
	D->type = (mode & GMT_WITH_STRINGS) ? ((n_columns == 0) ? GMT_READ_TEXT : GMT_READ_MIXED) : GMT_READ_DATA;
	if (n_tables) D->table = gmt_M_memory (GMT, NULL, n_tables, struct GMT_DATATABLE *);
	DH->n_alloc = D->n_tables = n_tables;
	if (!alloc_only) D->n_segments = D->n_tables * n_segments;
	if (!alloc_only) D->n_records = D->n_segments * n_rows;
	for (tbl = 0; tbl < n_tables; tbl++)
		if ((D->table[tbl] = gmt_create_table (GMT, n_segments, n_rows, n_columns, mode, alloc_only)) == NULL)
			return (NULL);
	DH->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level. */
	DH->alloc_mode = GMT_ALLOC_INTERNALLY;		/* So GMT_* modules can free this memory. */
	DH->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */

	return (D);
}

/*! . */
struct GMT_DATATABLE * gmtlib_read_table (struct GMT_CTRL *GMT, void *source, unsigned int source_type, bool greenwich, unsigned int *geometry, unsigned int *data_type, bool use_GMT_io) {
	/* Reads an entire data set into a single table in memory with any number of segments */

	bool ASCII, close_file = false, header = true, no_segments, first_seg = true, poly, this_is_poly = false;
	bool pol_check, check_geometry;
	int status;
	uint64_t n_expected_fields, n_returned = 0;
	uint64_t n_read = 0, row = 0, seg = 0, col, n_poly_seg = 0, k;
	size_t n_head_alloc = GMT_TINY_CHUNK;
	char open_mode[4] = {""}, file[PATH_MAX] = {""}, line[GMT_LEN64] = {""};
	double d;
	struct GMT_RECORD *In = NULL;
	FILE *fp = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	void * (*psave) (struct GMT_CTRL *, FILE *, uint64_t *, int *) = NULL;	/* Pointer to function reading tables */

	if (use_GMT_io) {	/* Use GMT->current.io.info settings to determine if input is ASCII/binary, else it defaults to ASCII */
		if (GMT->common.b.active[GMT_IN]) {	/* May return fewer than # of binary columns if -i was set */
			n_returned = (GMT->common.i.select) ? GMT->common.i.n_cols : GMT->common.b.ncol[GMT_IN];
			n_expected_fields = GMT->common.b.ncol[GMT_IN];
		}
		else
			n_expected_fields = GMT_MAX_COLUMNS;
		strcpy (open_mode, GMT->current.io.r_mode);
		ASCII = !GMT->common.b.active[GMT_IN];
	}
	else {			/* Force ASCII mode */
		n_expected_fields = GMT_MAX_COLUMNS;	/* GMT->current.io.input will return the number of columns */
		strcpy (open_mode, "r");
		ASCII = true;
		psave = GMT->current.io.input;			/* Save the previous pointer since we need to change it back at the end */
		GMT->current.io.input = GMT->session.input_ascii;	/* Override and use ASCII mode */
	}

#ifdef SET_IO_MODE
	if (!ASCII) gmt_setmode (GMT, GMT_IN);
#endif

	if (ASCII && *geometry == GMT_IS_TEXT) {
		psave = GMT->current.io.input;			/* Save the previous pointer since we need to change it back at the end */
		GMT->current.io.input = &gmtio_ascii_textinput;	/* Override and use ASCII text mode */
		GMT->current.io.record_type[GMT_IN] = *data_type = GMT_READ_TEXT;
	}

	/* Determine input source */

	if (source_type == GMT_IS_FILE) {	/* source is a file name */
		strncpy (file, source, PATH_MAX-1);
		if ((fp = gmt_fopen (GMT, file, open_mode)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot open file %s\n", file);
			if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */
			return (NULL);
		}
		close_file = true;	/* We only close files we have opened here */
	}
	else if (source_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)source;
		if (fp == NULL) fp = GMT->session.std[GMT_IN];	/* Default input */
		if (fp == GMT->session.std[GMT_IN])
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input stream>");
	}
	else if (source_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd = source;
		if (fd && (fp = fdopen (*fd, open_mode)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot convert file descriptor %d to stream in gmtlib_read_table\n", *fd);
			if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */
			return (NULL);
		}
		if (fd == NULL) fp = GMT->session.std[GMT_IN];	/* Default input */
		if (fp == GMT->session.std[GMT_IN])
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input file descriptor>");
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized source type %d in gmtlib_read_table\n", source_type);
		if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */
		return (NULL);
	}

	/* Skip binary header first, if binary and there is a header given in # of bytes */
	if (GMT->common.b.active[GMT_IN]) {
		if (GMT->current.setting.io_n_header_items) {
			gmtlib_io_binary_header (GMT, fp, GMT_IN);
			header = false;	/* No more binary header */
		}
	}

	if (GMT->current.io.record.data == NULL) *data_type = GMT_READ_TEXT;
	In = GMT->current.io.input (GMT, fp, &n_expected_fields, &status);	/* Get first record */
	n_read++;
	if (gmt_M_rec_is_eof(GMT)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s is empty!\n", file);
		if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */
		return (NULL);
	}

	pol_check = check_geometry = ((*geometry & GMT_IS_POLY) && (*geometry & GMT_IS_LINE));	/* Have to determine if these are closed polygons or not */
	poly = (((*geometry & GMT_IS_POLY) || *geometry == GMT_IS_MULTIPOLYGON) && (*geometry & GMT_IS_LINE) == 0);	/* To enable polar cap assessment in i/o */

	if (GMT->current.io.ogr == GMT_OGR_TRUE) {	/* Reading an OGR file so we can set the geometry, and possibly poly */
		if (GMT->current.io.OGR == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "OGR parsing incomplete (is file missing OGR statements?) - abort\n");
			GMT_exit (GMT, GMT_DATA_READ_ERROR); return (NULL);
		}
		poly = (GMT->current.io.OGR->geometry == GMT_IS_POLYGON || GMT->current.io.OGR->geometry == GMT_IS_MULTIPOLYGON);
		*geometry = GMT->current.io.OGR->geometry;
		if (*geometry > GMT_IS_MULTI)
			*geometry -= GMT_IS_MULTI;
	}

	/* Allocate the Table structure with GMT_CHUNK segments, but none has any rows or columns */

	T = gmt_create_table (GMT, GMT_CHUNK, 0U, 0U, 0U, false);
	TH = gmt_get_DT_hidden (T);

	TH->file[GMT_IN] = strdup (file);
	if (header) T->header = gmt_M_memory (GMT, NULL, n_head_alloc, char *);

	while (status >= 0 && !gmt_M_rec_is_eof (GMT)) {	/* Not yet EOF */
		if (header) {	/* Only true at start of an ASCII file */
			while ((GMT->current.setting.io_header[GMT_IN] && n_read <= GMT->current.setting.io_n_header_items) ||
			        gmt_M_rec_is_table_header (GMT)) { /* Process headers */
				for (k = 0; GMT->current.io.curr_text[k] == ' '; k++);
				T->header[T->n_headers] = strdup (&GMT->current.io.curr_text[k]);
				T->n_headers++;
				if (T->n_headers == n_head_alloc) {
					n_head_alloc <<= 1;
					T->header = gmt_M_memory (GMT, T->header, n_head_alloc, char *);
				}
				In = GMT->current.io.input (GMT, fp, &n_expected_fields, &status);
				n_read++;
			}
			if (T->n_headers)
				T->header = gmt_M_memory (GMT, T->header, T->n_headers, char *);
			else {	/* No header records found */
				gmt_M_free (GMT, T->header);
				T->header = NULL;
			}
			header = false;	/* Done processing header block; other comments are GIS/OGR encoded comments */
		}

		if (gmt_M_rec_is_eof (GMT)) continue;	/* Got EOF after headers */

		no_segments = !gmt_M_rec_is_segment_header (GMT);	/* Not a multi-segment file.  We then assume file has only one segment */

		while (no_segments || (gmt_M_rec_is_segment_header (GMT) && !gmt_M_rec_is_eof (GMT))) {
			/* To use different line-distances for each segment, place the distance in the segment header */
			if (first_seg || T->segment[seg]->n_rows > 0) {
				if (!first_seg) seg++;	/* Only advance segment if last had any points */
				T->segment[seg] = gmt_get_segment (GMT);
				first_seg = false;
			}
			n_read++;
			S = T->segment[seg];
			SH = gmt_get_DS_hidden (S);
			if (ASCII && !no_segments) {	/* Only ASCII files can have info stored in multi-seg header records */
				if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-D", line)) {	/* Found a potential -D<dist> option in the header */
					if (sscanf (line, "%lg", &d) == 1) SH->dist = d;	/* If readable, assign it to dist, else leave as zero */
				}
			}
			/* Segment initialization */
			row = 0;
			if (!no_segments) {	/* Read data if we read a segment header up front, but guard against headers which sets in = NULL */
				while (!gmt_M_rec_is_eof (GMT) && (In = GMT->current.io.input (GMT, fp, &n_expected_fields, &status)) == NULL) n_read++;
			}
			if (GMT->current.io.record_type[GMT_IN] == GMT_READ_TEXT)
				S->n_columns = 0;	/* No numerical data */
			else
				S->n_columns = (n_returned) ? n_returned : n_expected_fields;	/* This is where number of columns are determined */
			no_segments = false;	/* This has now served its purpose */
		}
		if (gmt_M_rec_is_eof (GMT)) continue;	/* At EOF; get out of this loop */
		if (ASCII && !no_segments) {	/* Only ASCII files can have info stored in multi-seg header record */
			char buffer[GMT_BUFSIZ] = {""};
			if (strlen (GMT->current.io.segment_header)) {
				S->header = strdup (GMT->current.io.segment_header);
				if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-L", buffer))
					S->label = strdup (buffer);
			}
			if (GMT->current.io.ogr == GMT_OGR_TRUE) gmtio_copy_ogr_seg (GMT, S, GMT->current.io.OGR);	/* Copy over any feature-specific values */
		}

		if (poly && S->n_columns < 2) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "File %s does not have at least 2 columns required for polygons (found %d)\n",
			            file, S->n_columns);
			if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */
			return (NULL);
		}

		while (! (GMT->current.io.status & (GMT_IO_SEGMENT_HEADER | GMT_IO_GAP | GMT_IO_EOF))) {	/* Keep going until false or find a new segment header */
			if (GMT->current.io.status & GMT_IO_MISMATCH) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Mismatch between actual (%d) and expected (%d) fields near line %" PRIu64 " in file %s\n",
				            status, n_expected_fields, n_read, file);
				if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */
				return (NULL);
			}

			gmt_prep_tmp_arrays (GMT, GMT_IN, row, S->n_columns);	/* Init or reallocate tmp read vectors */
			for (col = 0; col < S->n_columns; col++) {
				GMT->hidden.mem_coord[col][row] = In->data[col];
				if (gmt_M_type (GMT, GMT_IN, col) & GMT_IS_LON) {	/* Must handle greenwich/dateline alignments */
					if (greenwich && GMT->hidden.mem_coord[col][row] > 180.0) GMT->hidden.mem_coord[col][row] -= 360.0;
					if (!greenwich && GMT->hidden.mem_coord[col][row] < 0.0)  GMT->hidden.mem_coord[col][row] += 360.0;
				}
			}
			if (GMT->current.io.record_type[GMT_IN] & GMT_READ_TEXT) {
				if (GMT->current.io.record.text) GMT->hidden.mem_txt[row] = strdup (GMT->current.io.record.text);
				*data_type = GMT_READ_MIXED;
			}

			row++;
			In = GMT->current.io.input (GMT, fp, &n_expected_fields, &status);
			while (gmt_M_rec_is_table_header (GMT)) In = GMT->current.io.input (GMT, fp, &n_expected_fields, &status);	/* Just wind past other comments */
			n_read++;
		}

		if (n_expected_fields < 2) {
			pol_check = check_geometry = poly = false;
			*geometry = GMT_IS_NONE;
		}
		if (pol_check) this_is_poly = (!gmt_polygon_is_open (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], row));	/* true if this segment is closed polygon */
		if (this_is_poly) n_poly_seg++;
		if (check_geometry) {	/* Determine if dealing with closed polygons or lines based on first segment only */
			if (this_is_poly) poly = true;
			check_geometry = false;	/* Done with one-time checking */
			*geometry = (poly) ? GMT_IS_POLY : GMT_IS_LINE;	/* Update the geometry setting */
		}
		if (poly) {	/* If file contains a polygon then we must close it if needed */
			if (gmt_M_type (GMT, GMT_IN, GMT_X) & GMT_IS_GEO) {	/* Must check for polar cap */
				double dlon = GMT->hidden.mem_coord[GMT_X][0] - GMT->hidden.mem_coord[GMT_X][row-1];
				if (!((fabs (dlon) == 0.0 || fabs (dlon) == 360.0) && GMT->hidden.mem_coord[GMT_Y][0] == GMT->hidden.mem_coord[GMT_Y][row-1])) {
					gmt_prep_tmp_arrays (GMT, GMT_IN, row, S->n_columns);	/* Maybe reallocate tmp read vectors */
					for (col = 0; col < S->n_columns; col++) GMT->hidden.mem_coord[col][row] = GMT->hidden.mem_coord[col][0];
					row++;	/* Explicitly close polygon */
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Explicitly closed open geographic polygon in file %s, segment %" PRIu64 "\n", file, seg);
				}
			}
			else if (gmt_polygon_is_open (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], row)) {	/* Cartesian closure */
				gmt_prep_tmp_arrays (GMT, GMT_IN, row, S->n_columns);	/* Init or update tmp read vectors */
				for (col = 0; col < S->n_columns; col++) GMT->hidden.mem_coord[col][row] = GMT->hidden.mem_coord[col][0];
				row++;	/* Explicitly close polygon */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Explicitly closed open Cartesian polygon in file %s, segment %" PRIu64 "\n", file, seg);
			}
			if (gmt_parse_segment_item (GMT, S->header, "-Ph", NULL)) SH->pol_mode = GMT_IS_HOLE;
			/* If this is a hole then set link from previous segment to this one */
			if (seg && gmt_polygon_is_hole (GMT, S)) {
				struct GMT_DATASEGMENT_HIDDEN *SH2 = gmt_get_DS_hidden (T->segment[seg-1]);
				SH2->next = S;
			}
		}

		if (row == 0) {	/* Empty segment; we delete to avoid problems downstream in applications */
			gmt_M_free (GMT, T->segment[seg]);
			seg--;	/* Go back to where we were */
		}
		else {	/* OK to populate segment and increment counters */
			gmtlib_assign_segment (GMT, GMT_IN, T->segment[seg], row, T->segment[seg]->n_columns);	/* Allocate and place arrays into segment */
			gmt_set_seg_minmax (GMT, *geometry, 0, T->segment[seg]);	/* Set min/max */
			T->n_records += row;		/* Total number of records so far */
			SH->id = seg;	/* Internal segment number */
		}
		/* Reallocate to free up some memory */

		if (seg == (TH->n_alloc-1)) {	/* Need to allocate more segments */
			size_t n_old_alloc = TH->n_alloc;
			TH->n_alloc <<= 1;
			T->segment = gmt_M_memory (GMT, T->segment, TH->n_alloc, struct GMT_DATASEGMENT *);
			gmt_M_memset (&(T->segment[n_old_alloc]), TH->n_alloc - n_old_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
		}

		/* If a gap was detected, forget about it now, so we can use the data for the next segment */

		GMT->current.io.status -= (GMT->current.io.status & GMT_IO_GAP);
	}
	if (close_file) gmt_fclose (GMT, fp);
	if (!use_GMT_io) GMT->current.io.input = psave;	/* Restore previous setting */

	if (first_seg) {	/* Never saw any segment or data records */
		gmt_free_table (GMT, T);
		return (NULL);
	}
	if (T->segment[seg]->n_rows == 0) {	/* Last segment was empty; we delete to avoid problems downstream in applications */
		gmt_M_free (GMT, T->segment[seg]->hidden);
		gmt_M_free (GMT, T->segment[seg]);
		if (seg == 0) {	/* Happens when we just read 1 segment header with no data */
			gmt_free_table (GMT, T);
			return (NULL);
		}
	}
	else
		seg++;
	if (check_geometry && poly && n_poly_seg != seg) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Table contains mix of polygons (%" PRIu64 ") and lines (%" PRIu64 ")\n",
		            n_poly_seg, n_poly_seg - seg);
	}
	T->segment = gmt_M_memory (GMT, T->segment, seg, struct GMT_DATASEGMENT *);
	T->n_segments = seg;
	T->n_columns = T->segment[0]->n_columns;
	/* Determine table min,max values */
	T->min = gmt_M_memory (GMT, NULL, T->n_columns, double);
	T->max = gmt_M_memory (GMT, NULL, T->n_columns, double);
	for (col = 0; col < T->n_columns; col++) {T->min[col] = DBL_MAX; T->max[col] = -DBL_MAX;}
	for (seg = 0; seg < T->n_segments; seg++) {
		S = T->segment[seg];
		SH = gmt_get_DS_hidden (S);
		for (col = 0; col < T->n_columns; col++) {
			T->min[col] = MIN (T->min[col], S->min[col]);
			T->max[col] = MAX (T->max[col], S->max[col]);
		}
		if (SH->pole) {T->min[GMT_X] = 0.0; T->max[GMT_X] = 360.0;}
	}

	return (T);
}

/*! . */
struct GMT_DATASEGMENT * gmt_duplicate_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *Sin) {
	/* Duplicates the segment */
	uint64_t col;
	struct GMT_DATASEGMENT *Sout = GMT_Alloc_Segment (GMT->parent, GMT_IS_DATASET, Sin->n_rows, Sin->n_columns, Sin->header, NULL);
	for (col = 0; col < Sin->n_columns; col++) gmt_M_memcpy (Sout->data[col], Sin->data[col], Sin->n_rows, double);
	return (Sout);
}

/*! . */
struct GMT_DATASET * gmt_alloc_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, uint64_t n_rows, uint64_t n_columns, unsigned int mode) {
	/* Allocate new dataset structure with same # of tables, segments and rows/segment as input data set.
	 * However, n_columns is given separately and could differ.  Also, if n_rows > 0 we let that override the segment row counts.
	 * We copy over headers and segment headers.
	 * mode controls how the new dataset is to be allocated;
	 * mode = GMT_ALLOC_NORMAL means we replicate the number of tables and the layout of the Din dataset
	 * mode = GMT_ALLOC_VERTICAL means we concatenate all the tables in Din into a single table for Dout
	 * mode = GMT_ALLOC_HORIZONTAL means we base the Dout size only on the first Din table
	 *	(# of segments, # of rows/segment) because tables will be pasted horizontally and not vertically.
	 */
	unsigned int hdr, smode;
	size_t len;
	uint64_t nr, tbl, seg, n_seg, seg_in_tbl;
	struct GMT_DATASET *D = gmt_get_dataset (GMT);
	struct GMT_DATASET_HIDDEN *DH = gmt_get_DD_hidden (D);
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;

	D->n_columns = (n_columns) ? n_columns : Din->n_columns;
	D->geometry = Din->geometry;
	D->type = Din->type;
	D->min = gmt_M_memory (GMT, NULL, D->n_columns, double);
	D->max = gmt_M_memory (GMT, NULL, D->n_columns, double);
	if (mode) {	/* Pack everything into a single table */
		DH->n_alloc = D->n_tables = 1;
		if (mode == GMT_ALLOC_VERTICAL)
			for (tbl = n_seg = 0; tbl < Din->n_tables; tbl++) n_seg += Din->table[tbl]->n_segments;
		else	/* mode == GMT_ALLOC_HORIZONTAL */
			n_seg = Din->table[0]->n_segments;
		D->table = gmt_M_memory (GMT, NULL, 1, struct GMT_DATATABLE *);
		T = D->table[0] = gmt_get_table (GMT);
		TH = gmt_get_DT_hidden (T);

		/* As for file headers we concatenate the headers from all tables */
		T->n_headers  = Din->table[0]->n_headers;
		if (T->n_headers) T->header = gmt_M_memory (GMT, NULL, T->n_headers, char *);
		for (hdr = 0; hdr < T->n_headers; hdr++) {	/* Concatenate headers */
			for (tbl = len = 0; tbl < Din->n_tables; tbl++) if (Din->table[tbl]->header) len += (strlen (Din->table[tbl]->header[hdr]) + 2);
			T->header[hdr] = calloc (len, sizeof (char));
			strncpy (T->header[hdr], Din->table[0]->header[hdr], len);
			if (Din->n_tables > 1) gmt_chop (T->header[hdr]);	/* Remove newline */
			for (tbl = 1; tbl < Din->n_tables; tbl++) {	/* Now go across tables to paste */
				if (tbl < (Din->n_tables - 1)) gmt_chop (Din->table[tbl]->header[hdr]);
				strcat (T->header[hdr], "\t");
				if (Din->table[tbl]->header) strcat (T->header[hdr], Din->table[tbl]->header[hdr]);
			}
		}

		D->n_segments = T->n_segments = TH->n_alloc = n_seg;
		T->n_columns = D->n_columns;
		T->segment = gmt_M_memory (GMT, NULL, n_seg, struct GMT_DATASEGMENT *);
		T->min = gmt_M_memory (GMT, NULL, D->n_columns, double);
		T->max = gmt_M_memory (GMT, NULL, D->n_columns, double);
		for (seg = tbl = seg_in_tbl = 0; seg < D->n_segments; seg++) {
			if (seg == Din->table[tbl]->n_segments) { tbl++; seg_in_tbl = 0; }	/* Go to next table */
			nr = (n_rows) ? n_rows : Din->table[tbl]->segment[seg_in_tbl]->n_rows;
			smode = (Din->table[tbl]->segment[seg_in_tbl]->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
			T->segment[seg] = GMT_Alloc_Segment (GMT->parent, smode, nr, D->n_columns, NULL, NULL);
			if (mode != GMT_ALLOC_HORIZONTAL && Din->table[tbl]->segment[seg_in_tbl]->header)
				T->segment[seg]->header = strdup (Din->table[tbl]->segment[seg_in_tbl]->header);
			D->n_records += nr;
			seg_in_tbl++;
		}
	}
	else {	/* Just copy over the same dataset layout except for columns */
		DH->n_alloc  = D->n_tables = Din->n_tables;		/* Same number of tables as input dataset */
		D->n_segments  = Din->n_segments;	/* Same number of segments as input dataset */
		D->n_records  = Din->n_records;		/* Same number of records as input dataset */
		D->table = gmt_M_memory (GMT, NULL, D->n_tables, struct GMT_DATATABLE *);
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			D->table[tbl] = gmtio_alloc_table (GMT, Din->table[tbl], D->n_columns, n_rows);
		}
	}
	DH->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level. */
	DH->alloc_mode = GMT_ALLOC_INTERNALLY;		/* So GMT_* modules can free this memory. */
	DH->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */
	return (D);
}

/*! . */
struct GMT_DATASET *gmt_duplicate_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, unsigned int mode, unsigned int *geometry) {
	/* Make an exact replica, return geometry if not NULL */
	uint64_t tbl, seg;
	struct GMT_DATASET *D = NULL;
	D = gmt_alloc_dataset (GMT, Din, 0, Din->n_columns, mode);
	gmt_M_memcpy (D->min, Din->min, Din->n_columns, double);
	gmt_M_memcpy (D->max, Din->max, Din->n_columns, double);
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			gmtio_copy_segment (GMT, D->table[tbl]->segment[seg], Din->table[tbl]->segment[seg]);
		}
		gmt_M_memcpy (D->table[tbl]->min, Din->table[tbl]->min, Din->table[tbl]->n_columns, double);
		gmt_M_memcpy (D->table[tbl]->max, Din->table[tbl]->max, Din->table[tbl]->n_columns, double);
	}
	if (geometry)
		*geometry = D->geometry;

	return (D);
}

void gmtlib_change_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *D) {
	/* Given the -o option, rearrange the columns of the dataset.
	 * This is needed by the API when returning a dataset by reference
	 * as the normal -o path for printing is not taken. */
	uint64_t tbl, seg, col, src_col;
	unsigned int *used = NULL;
	bool extend = false, adjust = false;
	double **temp = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASEGMENT *S;
	if (!GMT->common.o.select) return;	/* Nutin' to do */
	temp = gmt_M_memory (GMT, NULL, D->n_columns, double *);
	used = gmt_M_memory (GMT, NULL, D->n_columns, unsigned int);
	if (GMT->common.o.n_cols > D->n_columns)	/* Must allocate more columns first */
		extend = true;
	if (GMT->common.o.n_cols != D->n_columns)	/* Must free/realloc some columns afterwards */
		adjust = true;
	for (tbl = 0; tbl < D->n_tables; tbl++) {
		T = D->table[tbl];	/* Current atble */
		for (seg = 0; seg < T->n_segments; seg++) {
			S = T->segment[seg];	/* Current segment */
			if (extend)	{/* Allocate more arrays first */
				S->data = gmt_M_memory (GMT, S->data, GMT->common.o.n_cols, double *);
				for (col = D->n_columns; col < GMT->common.o.n_cols; col++)
					S->data[col] = gmt_M_memory (GMT, NULL, S->n_rows, double);
			}
			for (col = 0; col < S->n_columns; col++)	/* Keep pointers to original arrays */
				temp[col] = S->data[col];
			gmt_M_memset (used, S->n_columns, unsigned int);	/* Reset used counters */
			gmt_M_memset (S->data, S->n_columns, double *);	/* Reset pointers */
			/* During the first pass we can set pointers only once per source column */
			for (col = 0; col < GMT->common.o.n_cols; col++) {	/* These are final output columns */
				src_col = GMT->current.io.col[GMT_OUT][col].col;	/* Corresponding original column */
				if (used[src_col] == 0)	/* Can set pointer the first time */
					S->data[col] = temp[src_col];
				used[src_col]++;	/* Used this column one more time */
			}
			/* During second pass we must allocate space for any repeated columns */
			for (col = 0; col < GMT->common.o.n_cols; col++) {	/* These are again final output columns */
				if (S->data[col]) continue;	/* Set this one via pointer in pass one */
				S->data[col] = gmt_M_memory (GMT, NULL, S->n_rows, double);
				src_col = GMT->current.io.col[GMT_OUT][col].col;	/* Corresponding original column */
				gmt_M_memcpy (S->data[col], temp[src_col], S->n_rows, double);	/* Duplicate contents in second pass */
			}
			for (col = 0; col < S->n_columns; col++) {		/* Free the unused columns */
				if (used[col] == 0) gmt_M_free (GMT, temp[col]);
			}
			if (adjust) {	/* Modify length of min/max arrays */
				S->min  = gmt_M_memory (GMT, S->min, GMT->common.o.n_cols, double);
				S->max  = gmt_M_memory (GMT, S->max, GMT->common.o.n_cols, double);
				if (!extend) S->data = gmt_M_memory (GMT, S->data, GMT->common.o.n_cols, double *);
			}
			S->n_columns = GMT->common.o.n_cols;	/* New column count */
		}
		if (adjust) {	/* Modify length of table min/max arrays */
			T->min = gmt_M_memory (GMT, T->min, GMT->common.o.n_cols, double);
			T->max = gmt_M_memory (GMT, T->max, GMT->common.o.n_cols, double);
		}
		T->n_columns = GMT->common.o.n_cols;	/* New column count */
	}
	gmt_M_free (GMT, temp);
	gmt_M_free (GMT, used);
	if (adjust) {	/* Modify length of dataset min/max arrays */
		D->min = gmt_M_memory (GMT, D->min, GMT->common.o.n_cols, double);
		D->max = gmt_M_memory (GMT, D->max, GMT->common.o.n_cols, double);
	}
	D->n_columns = GMT->common.o.n_cols;	/* New column count */
	gmt_set_dataset_minmax (GMT, D);		/* Update column stats */
}

GMT_LOCAL void gmtio_free_segment_text (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S) {
	/* Frees any array of trailing text items */
	uint64_t row;
	if (S->text == NULL) return;	/* No text */
	for (row = 0; row < S->n_rows; row++)
		gmt_M_str_free (S->text[row]);
	gmt_M_free (GMT, S->text);
}

/*! . */
void gmt_free_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT **S) {
	/* Free memory allocated by gmtlib_read_table */

	unsigned int k;
	uint64_t col;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_DATASEGMENT *segment = *S;
	if (!segment) return;	/* Do not try to free NULL pointer */
	SH = gmt_get_DS_hidden (segment);
	if (SH->alloc_mode == GMT_ALLOC_INTERNALLY) {	/* Free data GMT allocated */
		for (col = 0; col < segment->n_columns; col++) gmt_M_free (GMT, segment->data[col]);
	}
	gmt_M_free (GMT, segment->data);
	gmt_M_free (GMT, segment->min);
	gmt_M_free (GMT, segment->max);
	gmt_M_str_free ( segment->label);
	gmt_M_str_free ( segment->header);
	for (k = 0; k < 2; k++) gmt_M_str_free (SH->file[k]);
	if (SH->ogr) gmtio_free_ogr_seg (GMT, segment);	/* OGR metadata */
	gmtio_free_segment_text (GMT, segment);
	gmt_M_free (GMT, SH);
	gmt_M_free (GMT, segment);
	*S = NULL;
}

/*! . */
void gmt_free_table (struct GMT_CTRL *GMT, struct GMT_DATATABLE *table) {
	unsigned int k;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;
	if (!table) return;		/* Do not try to free NULL pointer */
	TH = gmt_get_DT_hidden (table);
	for (k = 0; k < table->n_headers; k++) gmt_M_str_free (table->header[k]);
	gmt_M_free (GMT, table->header);
	gmt_M_free (GMT, table->min);
	gmt_M_free (GMT, table->max);
	for (k = 0; k < 2; k++) gmt_M_str_free (TH->file[k]);
	gmtlib_free_ogr (GMT, &(TH->ogr), 1);
	if (table->segment) {	/* Free segments */
		uint64_t seg;
		for (seg = 0; seg < table->n_segments; seg++) gmt_free_segment (GMT, &(table->segment[seg]));
		gmt_M_free (GMT, table->segment);
	}
	gmt_M_free (GMT, TH);
	gmt_M_free (GMT, table);
}

/*! . */
void gmtlib_free_dataset_ptr (struct GMT_CTRL *GMT, struct GMT_DATASET *data) {
	/* This takes pointer to data array and thus can return it as NULL */
	unsigned int tbl, k;
	struct GMT_DATASET_HIDDEN *DH = NULL;
	if (!data) return;	/* Do not try to free NULL pointer */
	DH = gmt_get_DD_hidden (data);
	for (tbl = 0; tbl < data->n_tables; tbl++) {
		gmt_free_table (GMT, data->table[tbl]);
	}
	gmt_M_free (GMT, data->min);
	gmt_M_free (GMT, data->max);
	gmt_M_free (GMT, data->table);
	for (k = 0; k < 2; k++) gmt_M_str_free (DH->file[k]);
	gmt_M_free (GMT, data->hidden);
}

/*! . */
void gmt_free_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET **data) {
	/* This takes pointer to data array and thus can return it as NULL */
	gmtlib_free_dataset_ptr (GMT, *data);
	gmt_M_free (GMT, *data);
}

struct GMT_IMAGE *gmt_get_image (struct GMT_CTRL *GMT) {
	struct GMT_IMAGE *I = gmt_M_memory (GMT, NULL, 1, struct GMT_IMAGE);
	I->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_IMAGE_HIDDEN);
	return (I);
}

/*! . */
struct GMT_IMAGE *gmtlib_create_image (struct GMT_CTRL *GMT) {
	/* Allocates space for a new image container. */
	struct GMT_IMAGE_HIDDEN *IH = NULL;
	struct GMT_IMAGE *I = gmt_get_image (GMT);
	IH = gmt_get_I_hidden (I);
	I->header = gmt_get_header (GMT);
	IH->alloc_mode = GMT_ALLOC_INTERNALLY;		/* Memory can be freed by GMT. */
	IH->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level. */
	IH->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */
	gmt_grd_init (GMT, I->header, NULL, false); /* Set default values */
#ifdef HAVE_GDAL
	if (GMT->current.gdal_read_in.O.mem_layout[0])
		gmt_strncpy (I->header->mem_layout, GMT->current.gdal_read_in.O.mem_layout, 4);	/* Set the current memory layout */
	else
#endif
		gmt_strncpy (I->header->mem_layout, GMT_IMAGE_LAYOUT, 4);	/* Set the default array memory layout */
	GMT_Set_Index (GMT->parent, I->header, GMT_IMAGE_LAYOUT);
	return (I);
}

/*! . */
struct GMT_IMAGE *gmtlib_duplicate_image (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, unsigned int mode) {
	/* Duplicates an entire image, including data if requested. */
	struct GMT_IMAGE *Inew = NULL;
	struct GMT_GRID_HEADER *save = NULL;
	struct GMT_IMAGE_HIDDEN *IH = NULL;

	Inew = gmtlib_create_image (GMT);
	save = Inew->header;
	IH = Inew->hidden;
	gmt_M_memcpy (Inew, I, 1, struct GMT_IMAGE);	/* Copy everything, but this also messes with header/data pointers */
	Inew->header = save;	/* Reset to correct header pointer */
	Inew->hidden = IH;	/* Reset to correct hidden pointer */
	Inew->data = NULL;	/* Reset to NULL data pointer */
	Inew->x = NULL;		/* Reset to NULL x pointer */
	Inew->y = NULL;		/* Reset to NULL y pointer */
	gmt_copy_gridheader (GMT, Inew->header, I->header);

	if ((mode & GMT_DUPLICATE_DATA) || (mode & GMT_DUPLICATE_ALLOC)) {	/* Also allocate and possibly duplicate data array */
		Inew->data = gmt_M_memory_aligned (GMT, NULL, I->header->size, char);
		if (mode & GMT_DUPLICATE_DATA) gmt_M_memcpy (Inew->data, I->data, I->header->size, char);
		Inew->x = gmt_grd_coord (GMT, Inew->header, GMT_X);	/* Get array of x coordinates */
		Inew->y = gmt_grd_coord (GMT, Inew->header, GMT_Y);	/* Get array of y coordinates */
	}
	return (Inew);
}

/*! . */
void gmtlib_free_image_ptr (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, bool free_image) {
	/* Free contents of image pointer */
	struct GMT_IMAGE_HIDDEN *IH = NULL;
	if (!I) return;	/* Nothing to deallocate */
	IH = gmt_get_I_hidden (I);
	if (free_image && I->data) {
		if (IH->alloc_mode == GMT_ALLOC_INTERNALLY)
			gmt_M_free_aligned (GMT, I->data);
	}
	if (free_image && I->alpha) {
		if (IH->alloc_mode == GMT_ALLOC_INTERNALLY)
			gmt_M_free_aligned (GMT, I->alpha);
	}
	if (I->x && I->y && free_image) {
		if (IH->alloc_mode == GMT_ALLOC_INTERNALLY) {
			gmt_M_free (GMT, I->x);
			gmt_M_free (GMT, I->y);
		}
		I->x = I->y = NULL;	/* This will remove reference to external memory since gmt_M_free would not have been called */
	}
	if (I->header) {	/* Free the header structure and anything allocated by it */
		struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (I->header);
		if (I->header->ProjRefPROJ4 && IH->alloc_mode == GMT_ALLOC_INTERNALLY)
			gmt_M_str_free (I->header->ProjRefPROJ4);
		if (I->header->ProjRefWKT && IH->alloc_mode == GMT_ALLOC_INTERNALLY)
			gmt_M_str_free(I->header->ProjRefWKT);
		if (HH->pocket && IH->alloc_mode == GMT_ALLOC_INTERNALLY)
			gmt_M_str_free (HH->pocket);
		gmt_M_free (GMT, HH);
		gmt_M_free (GMT, I->header);
	}
	if (I->colormap && IH->alloc_mode == GMT_ALLOC_INTERNALLY)
		gmt_M_free (GMT, I->colormap);

	gmt_M_free (GMT, I->hidden);
}

/*! . */
void gmtlib_free_image (struct GMT_CTRL *GMT, struct GMT_IMAGE **I, bool free_image) {
	/* By taking a reference to the image pointer we can set it to NULL when done */
	gmtlib_free_image_ptr (GMT, *I, free_image);
	gmt_M_free (GMT, *I);
}

/*! . */
struct GMT_VECTOR * gmt_create_vector (struct GMT_CTRL *GMT, uint64_t n_columns, unsigned int direction) {
	/* Allocates space for a new vector container.  No space allocated for the vectors themselves */
	struct GMT_VECTOR *V = NULL;
	struct GMT_VECTOR_HIDDEN *VH = NULL;
	gmt_M_unused(direction);

	if ((V = gmt_M_memory (GMT, NULL, 1U, struct GMT_VECTOR)) == NULL)
		return_null (GMT, GMT_MEMORY_ERROR);
	VH = V->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_VECTOR_HIDDEN);
	if (n_columns) V->data = gmt_M_memory_aligned (GMT, NULL, n_columns, union GMT_UNIVECTOR);
	if (n_columns) V->type = gmt_M_memory (GMT, NULL, n_columns, enum GMT_enum_type);
	V->n_columns = n_columns;
	/* We expect external memory for input and GMT-allocated memory on output */
	VH->alloc_mode = (direction == GMT_IN) ? GMT_ALLOC_EXTERNALLY : GMT_ALLOC_INTERNALLY;
	VH->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level */
	VH->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */

	return (V);
}

/*! . */
int gmtlib_alloc_vectors (struct GMT_CTRL *GMT, struct GMT_VECTOR *V, uint64_t n_alloc) {
	/* Allocate space for each column according to data type; V->n_rows is not touched */
	uint64_t col;
	int error;
	struct GMT_VECTOR_HIDDEN *VH = NULL;

	if (!V) return (GMT_PTR_IS_NULL);			/* Nothing to allocate to */
	if (V->n_columns == 0) return (GMT_PTR_IS_NULL);	/* No columns specified */
	if (n_alloc == 0) return (GMT_N_ROWS_NOT_SET);		/* No rows specified */
	if (!V->data) return (GMT_PTR_IS_NULL);			/* Array of columns have not been allocated */
	for (col = 0; col < V->n_columns; col++) {
		if ((error = gmtlib_alloc_univector (GMT, &V->data[col], V->type[col], n_alloc)) != GMT_NOERROR) return (error);
	}
	VH = gmt_get_V_hidden (V);
	VH->alloc_mode = GMT_ALLOC_INTERNALLY;
	return (GMT_NOERROR);
}

/*! . */
struct GMT_VECTOR * gmtlib_duplicate_vector (struct GMT_CTRL *GMT, struct GMT_VECTOR *V_in, unsigned int mode) {
	/* Duplicates a vector container - optionally allocates and duplicates the data vectors */
	struct GMT_VECTOR *V = NULL;
	unsigned int col;
	int error;
	if ((V = gmt_create_vector (GMT, V_in->n_columns, GMT_IN)) == NULL)
		return_null (GMT, GMT_MEMORY_ERROR);
	for (col = 0; col < V_in->n_columns; col++)	/* Set same data type for all vectors */
		V->type[col] = V_in->type[col];
	if ((mode & GMT_DUPLICATE_DATA) || (mode & GMT_DUPLICATE_ALLOC)) {	/* Also allocate and possibly duplicate data array */
		if ((error = gmtlib_alloc_vectors (GMT, V, V_in->n_rows)) != GMT_NOERROR)
			return_null (GMT, error);
		if (mode & GMT_DUPLICATE_DATA) {
			for (col = 0; col < V_in->n_columns; col++)
				gmtio_duplicate_univector (GMT, &V->data[col], &V_in->data[col], V->type[col], V_in->n_rows);
		}
	}
	return (V);
}

/*! . */
int gmtlib_alloc_univector (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *u, unsigned int type, uint64_t n_rows) {
	/* Allocate space for one univector according to data type */
	int error = GMT_OK;
	switch (type) {
		case GMT_UCHAR:  u->uc1 = gmt_M_memory (GMT, u->uc1, n_rows, uint8_t);   if (u->uc1 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_CHAR:   u->sc1 = gmt_M_memory (GMT, u->sc1, n_rows, int8_t);    if (u->sc1 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_USHORT: u->ui2 = gmt_M_memory (GMT, u->ui2, n_rows, uint16_t);  if (u->ui2 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_SHORT:  u->si2 = gmt_M_memory (GMT, u->si2, n_rows, int16_t);   if (u->si2 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_UINT:   u->ui4 = gmt_M_memory (GMT, u->ui4, n_rows, uint32_t);  if (u->ui4 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_INT:    u->si4 = gmt_M_memory (GMT, u->si4, n_rows, int32_t);   if (u->si4 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_ULONG:  u->ui8 = gmt_M_memory (GMT, u->ui8, n_rows, uint64_t);  if (u->ui8 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_LONG:   u->si8 = gmt_M_memory (GMT, u->si8, n_rows, int64_t);   if (u->si8 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_FLOAT:  u->f4  = gmt_M_memory (GMT, u->f4,  n_rows, float);     if (u->f4  == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_DOUBLE: u->f8  = gmt_M_memory (GMT, u->f8,  n_rows, double);    if (u->f8  == NULL) error = GMT_MEMORY_ERROR; break;
	}
	return (error);
}

/* Free the individual strings in text[] that were allocated by strdup */
GMT_LOCAL void free_text_array (uint64_t n, char **text) {
	uint64_t row;
	for (row = 0; row < n; row++) gmt_M_str_free (text[row]);
}

/*! . */
unsigned int gmtlib_free_vector_ptr (struct GMT_CTRL *GMT, struct GMT_VECTOR *V, bool free_vector) {
	/* By taking a reference to the vector pointer we can set it to NULL when done */
	/* free_vector = false means the vectors are not to be freed but the data array itself will be */
	struct GMT_VECTOR_HIDDEN *VH = NULL;
	if (!V) return 0;	/* Nothing to deallocate */
	/* Only free V->data if allocated by GMT AND free_vector is true */
	VH = gmt_get_V_hidden (V);
	if (V->data && free_vector) {
		uint64_t col;
		for (col = 0; col < V->n_columns; col++) {
			if (VH->alloc_mode == GMT_ALLOC_INTERNALLY) gmtio_free_univector (GMT, &(V->data[col]), V->type[col]);
			gmtio_null_univector (GMT, &(V->data[col]), V->type[col]);
		}
	}
	if (V->text && free_vector && VH->alloc_mode == GMT_ALLOC_INTERNALLY) {
		free_text_array (V->n_rows, V->text);
		gmt_M_free (GMT, V->text);
	}
	gmt_M_free (GMT, V->data);	/* Sometimes we free a V that has nothing allocated so must check */
	gmt_M_free (GMT, V->type);
	gmt_M_free (GMT, V->hidden);
	return (VH->alloc_mode);
}

/*! . */
void gmt_free_vector (struct GMT_CTRL *GMT, struct GMT_VECTOR **V, bool free_vector) {
	/* By taking a reference to the vector pointer we can set it to NULL when done */
	/* free_vector = false means the vectors are not to be freed but the data array itself will be */
	(void)gmtlib_free_vector_ptr (GMT, *V, free_vector);
	gmt_M_free (GMT, *V);
}

/*! . */
struct GMT_MATRIX * gmtlib_create_matrix (struct GMT_CTRL *GMT, uint64_t layers, unsigned int direction, int flag) {
	/* Allocates space for a new matrix container. */
	struct GMT_MATRIX *M = NULL;
	struct GMT_MATRIX_HIDDEN *MH = NULL;
	M = gmt_M_memory (GMT, NULL, 1, struct GMT_MATRIX);
	MH = M->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_MATRIX_HIDDEN);
	/* We expect external memory for input and GMT-allocated memory on output */
	MH->alloc_mode = (direction == GMT_IN) ? GMT_ALLOC_EXTERNALLY : GMT_ALLOC_INTERNALLY;
	MH->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level. */
	MH->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */
	M->n_layers = (layers) ? layers : 1;		/* Default to 1 if not set */
	switch (flag) {
		case GMT_IS_ROW_FORMAT:	M->shape = GMT_IS_ROW_FORMAT;  break;	/* row-major */
		case GMT_IS_COL_FORMAT:	M->shape = GMT_IS_COL_FORMAT;  break;	/* col-major */
		default: 		M->shape = GMT->parent->shape; break;	/* Default layout (row vs column) selected by GMT_Create_Session [row-major] */
	}
	return (M);
}

/*! . */
struct GMT_MATRIX * gmtlib_duplicate_matrix (struct GMT_CTRL *GMT, struct GMT_MATRIX *M_in, unsigned int mode) {
	/* Duplicates a matrix container - optionally duplicates the data array */
	struct GMT_MATRIX *M = NULL;
	M = gmt_M_memory (GMT, NULL, 1, struct GMT_MATRIX);
	gmt_M_memcpy (M, M_in, 1, struct GMT_MATRIX);
	gmt_M_memset (&M->data, 1, union GMT_UNIVECTOR);
	if ((mode & GMT_DUPLICATE_DATA) || (mode & GMT_DUPLICATE_ALLOC)) {	/* Also allocate and possibly duplicate data array */
		size_t size = M->n_rows * M->n_columns;
		if (gmtlib_alloc_univector (GMT, &(M->data), M->type, size)) {
			gmt_M_free (GMT, M);
			return (NULL);
		}
		if (mode & GMT_DUPLICATE_DATA)
			gmtio_duplicate_univector (GMT, &M->data, &M_in->data, M->type, size);
	}
	return (M);
}

/*! . */
unsigned int gmtlib_free_matrix_ptr (struct GMT_CTRL *GMT, struct GMT_MATRIX *M, bool free_matrix) {
	/* Free everything but the struct itself  */
	struct GMT_MATRIX_HIDDEN *MH = NULL;
	enum GMT_enum_alloc alloc_mode;
	if (!M) return 0;	/* Nothing to deallocate */
	/* Only free M->data if allocated by GMT AND free_matrix is true */
	MH = gmt_get_M_hidden (M);
	if (&(M->data) && free_matrix) {
		if (MH->alloc_mode == GMT_ALLOC_INTERNALLY) gmtio_free_univector (GMT, &(M->data), M->type);
		gmtio_null_univector (GMT, &(M->data), M->type);
	}
	if (M->text && free_matrix && MH->alloc_mode == GMT_ALLOC_INTERNALLY) {
		free_text_array (M->n_rows, M->text);
		gmt_M_free (GMT, M->text);
	}
	alloc_mode = MH->alloc_mode;
	gmt_M_free (GMT, M->hidden);
	return (alloc_mode);
}

void gmtlib_free_matrix (struct GMT_CTRL *GMT, struct GMT_MATRIX **M, bool free_matrix) {
	/* By taking a reference to the matrix pointer we can set it to NULL when done */
	(void)gmtlib_free_matrix_ptr (GMT, *M, free_matrix);
	gmt_M_free (GMT, *M);
}


/*!  m input matrix
	n_rows number of rows of \a m
	n_columns number of columns of \a m

   Performs in-place transposition of matrix m.
   Modified from http://programmers.stackexchange.com/questions/271713/transpose-a-matrix-without-a-buffering-one
*/

void gmtlib_union_transpose(struct GMT_CTRL *GMT, union GMT_UNIVECTOR *m, const uint64_t n_rows, const uint64_t n_columns, unsigned int type) {
	/* In-place transpose of rectangular matrix m */
	union GMT_UNIVECTOR tmp;
	int error = GMT_OK;

	switch (type) {
		case GMT_UCHAR:  tmp.uc1 = gmt_M_memory(GMT, NULL, 1, uint8_t);   if (tmp.uc1 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_CHAR:   tmp.sc1 = gmt_M_memory(GMT, NULL, 1, int8_t);    if (tmp.sc1 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_USHORT: tmp.ui2 = gmt_M_memory(GMT, NULL, 1, uint16_t);  if (tmp.ui2 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_SHORT:  tmp.si2 = gmt_M_memory(GMT, NULL, 1, int16_t);   if (tmp.si2 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_UINT:   tmp.ui4 = gmt_M_memory(GMT, NULL, 1, uint32_t);  if (tmp.ui4 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_INT:    tmp.si4 = gmt_M_memory(GMT, NULL, 1, int32_t);   if (tmp.si4 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_ULONG:  tmp.ui8 = gmt_M_memory(GMT, NULL, 1, uint64_t);  if (tmp.ui8 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_LONG:   tmp.si8 = gmt_M_memory(GMT, NULL, 1, int64_t);   if (tmp.si8 == NULL) error = GMT_MEMORY_ERROR; break;
		case GMT_FLOAT:   tmp.f4 = gmt_M_memory(GMT, NULL, 1, float);     if (tmp.f4 == NULL)  error = GMT_MEMORY_ERROR; break;
		case GMT_DOUBLE:  tmp.f8 = gmt_M_memory(GMT, NULL, 1, double);    if (tmp.f8 == NULL)  error = GMT_MEMORY_ERROR; break;
		default:          tmp.f8 = NULL;	/* To shut up the compiler about the "potentially uninitialized local variable 'tmp' used" */
	}
	if (error) {
		GMT_Report(GMT, GMT_MSG_ERROR, "gmtlib_union_transpose: Failure to allocate memory.\n");
		return;
	}

	for (uint64_t start = 0; start <= n_columns * n_rows - 1; ++start) {
		uint64_t next = start, i = 0;
		do {
			++i; next = (next % n_rows) * n_columns + next / n_rows;
		} while (next > start);
		if (next >= start && i != 1) {
			switch (type) {
				case GMT_DOUBLE:	*tmp.f8  =  m->f8[start];	break;
				case GMT_FLOAT:		*tmp.f4  =  m->f4[start];	break;
				case GMT_ULONG:		*tmp.ui8 = m->ui8[start];	break;
				case GMT_LONG:		*tmp.si8 = m->si8[start];	break;
				case GMT_UINT:		*tmp.ui4 = m->ui4[start];	break;
				case GMT_INT:		*tmp.si4 = m->si4[start];	break;
				case GMT_USHORT:	*tmp.ui2 = m->ui2[start];	break;
				case GMT_SHORT:		*tmp.si2 = m->si2[start];	break;
				case GMT_UCHAR:		*tmp.uc1 = m->uc1[start];	break;
				case GMT_CHAR:		*tmp.sc1 = m->sc1[start];	break;
			}
			next = start;
			do {
				i = (next % n_rows) * n_columns + next / n_rows;
				switch (type) {
					case GMT_DOUBLE:	m->f8[next]  = (i == start) ? *tmp.f8  :  m->f8[i];	break;
					case GMT_FLOAT:		m->f4[next]  = (i == start) ? *tmp.f4  :  m->f4[i];	break;
					case GMT_ULONG:		m->ui8[next] = (i == start) ? *tmp.ui8 : m->ui8[i];	break;
					case GMT_LONG:		m->si8[next] = (i == start) ? *tmp.si8 : m->si8[i];	break;
					case GMT_UINT:		m->ui4[next] = (i == start) ? *tmp.ui4 : m->ui4[i];	break;
					case GMT_INT:		m->si4[next] = (i == start) ? *tmp.si4 : m->si4[i];	break;
					case GMT_USHORT:	m->ui2[next] = (i == start) ? *tmp.ui2 : m->ui2[i];	break;
					case GMT_SHORT:		m->si2[next] = (i == start) ? *tmp.si2 : m->si2[i];	break;
					case GMT_UCHAR:		m->uc1[next] = (i == start) ? *tmp.uc1 : m->uc1[i];	break;
					case GMT_CHAR:		m->sc1[next] = (i == start) ? *tmp.sc1 : m->sc1[i];	break;
				}
				next = i;
			} while (next > start);
		}
	}
	gmtio_free_univector (GMT, &(tmp), type);

}

/*! . */
bool gmt_not_numeric (struct GMT_CTRL *GMT, char *text) {
	/* true if text cannot represent a valid number  However,
	 * false does not therefore mean we have a valid number because
	 * <date>T<clock> representations may use all kinds
	 * of punctuations or letters according to the various format
	 * settings in gmt.conf.  Here we just rule out things
	 * that we are sure of. */

	unsigned int i, k, n_digits = 0, n_period = 0, period = 0, n_plus = 0, n_minus = 0;
	static char *valid = "0123456789-+.:WESNT" GMT_LEN_UNITS GMT_DIM_UNITS;
	gmt_M_unused(GMT);
	if (!text) return (true);		/* NULL pointer */
	if (!strlen (text)) return (true);	/* Blank string */
	if (isalpha ((int)text[0])) return (true);	/* Numbers cannot start with letters */
	i = (int)text[0];
	if (!(text[0] == '+' || text[0] == '-' || text[0] == '.' || (i <= 255 && isdigit(i)) )) return (true);	/* Numbers must be [+|-][.][<digits>] */
	for (i = 0; text[i]; i++) {	/* Check each character */
		/* First check for ASCII values that should never appear in any number */
		if (!strchr (valid, text[i])) return (true);	/* Found a char not among valid letters */
		if (isdigit ((int)text[i])) n_digits++;
		if (text[i] == '.') {
			n_period++;
			period = i;
		}
		if (text[i] == '+') n_plus++;
		if (text[i] == '-') n_minus++;
	}
	if (n_digits == 0 || n_period > 1 || (n_plus + n_minus) > 2) return (true);
	if (n_period) {	/* Check if we have filename.ext with ext having no numbers */
		for (i = period + 1, n_digits = k = 0; text[i]; i++, k++) if (isdigit ((int)text[i])) n_digits++;
		if (k > 0 && n_digits == 0) return (true);	/* Probably a file */
	}
	return (false);	/* This may in fact be numeric */
}

bool gmt_is_float (struct GMT_CTRL *GMT, char *text) {
	/* Returns true if text is a valid floating point number.
	 * Only called if we know text is not longitude or time, etc. */
	int len;
	double dummy = 0.0;
	gmt_M_unused(GMT);

	if (sscanf (text, "%lf %n", &dummy, &len) == 1 && len == (int)strlen(text))
		return true;
	else
		return false;
}

/*! . */
unsigned int gmtlib_conv_text2datarec (struct GMT_CTRL *GMT, char *record, unsigned int ncols, double *out) {
	/* Used when we read text records and need to obtain doubles */
	/* Convert the first ncols fields in the record string to numbers that we
	 * store in GMT->current.io.curr_rec, which is what normal GMT_DATASET processing do.
	 * We stop if we run out of fields or fail to convert.  */

	unsigned int k = 0, pos = 0;
	char p[GMT_BUFSIZ];

	while (k < ncols && gmt_strtok (record, GMT->current.io.scan_separators, &pos, p)) {	/* Get each field in turn and bail when done */
		if (!(p[0] == '+' || p[0] == '-' || p[0] == '.' || isdigit ((int)p[0]))) break;	/* Numbers must be [+|-][.][<digits>] */
		if (strchr (p, '/')) break;	/* Somehow got to a color triplet? */
		gmt_scanf (GMT, p, gmt_M_type (GMT, GMT_IN, k), &out[k]);	/* Be tolerant of errors */
		k++;
	}
	return (k);
}

/*! . */
int gmtlib_ogr_get_type (char *item) {
	if (!strcmp (item, "double")   || !strcmp (item, "DOUBLE")) return (GMT_DOUBLE);
	if (!strcmp (item, "float")    || !strcmp (item, "FLOAT")) return (GMT_FLOAT);
	if (!strcmp (item, "integer")  || !strcmp (item, "INTEGER")) return (GMT_INT);
	if (!strcmp (item, "char")     || !strcmp (item, "CHAR")) return (GMT_CHAR);
	if (!strcmp (item, "string")   || !strcmp (item, "STRING")) return (GMT_TEXT);
	if (!strcmp (item, "datetime") || !strcmp (item, "DATETIME")) return (GMT_DATETIME);
	if (!strcmp (item, "logical")  || !strcmp (item, "LOGICAL")) return (GMT_UCHAR);
	return (GMT_NOTSET);
}

/*! . */
void gmtlib_free_ogr (struct GMT_CTRL *GMT, struct GMT_OGR **G, unsigned int mode) {
	/* Free up GMT/OGR structure, if used */
	unsigned int k;
	if (!(*G)) return;	/* Nothing to do */
	/* mode = 0 only frees the aspatial data value array, while mode = 1 frees the entire struct and contents */
	for (k = 0; k < (*G)->n_aspatial; k++) {
		if (mode == 1 && (*G)->name) gmt_M_str_free ((*G)->name[k]);
		if ((*G)->tvalue) gmt_M_str_free ((*G)->tvalue[k]);
	}
	gmt_M_free (GMT, (*G)->tvalue);
	gmt_M_free (GMT, (*G)->dvalue);
	if (mode == 0) return;	/* That's all we do for now */
	/* Here we free up everything */
	gmt_M_free (GMT, (*G)->name);
	gmt_M_free (GMT, (*G)->type);
	gmt_M_str_free ((*G)->region);
	for (k = 0; k < 4; k++) gmt_M_str_free ((*G)->proj[k]);
	gmt_M_free (GMT, (*G));
}

/*! . */
struct GMT_OGR * gmtlib_duplicate_ogr (struct GMT_CTRL *GMT, struct GMT_OGR *G) {
	/* Duplicate GMT/OGR structure, if used */
	unsigned int k;
	struct GMT_OGR *G_dup = NULL;
	if (!G) return (NULL);	/* Nothing to do */
	G_dup = gmt_M_memory (GMT, NULL, 1, struct GMT_OGR);
	if (G->region) G_dup->region = strdup (G->region);
	for (k = 0; k < 4; k++) if (G->proj[k]) G_dup->proj[k] = strdup (G->proj[k]);
	G_dup->geometry = G->geometry;
	if (G->n_aspatial) {
		G_dup->n_aspatial = G->n_aspatial;
		G_dup->name = gmt_M_memory (GMT, NULL, G->n_aspatial, char *);
		for (k = 0; k < G->n_aspatial; k++) if (G->name[k]) G_dup->name[k] = strdup (G->name[k]);
		G_dup->type = gmt_M_memory (GMT, NULL, G->n_aspatial, enum GMT_enum_type);
		gmt_M_memcpy (G_dup->type, G->type, G->n_aspatial, int);
	}
	return (G_dup);
}

/*! . */
double gmt_get_aspatial_value (struct GMT_CTRL *GMT, int col, struct GMT_DATASEGMENT *S) {
	/* Return the value associated with the aspatial values given for this column col */

	uint64_t k;
	int64_t scol = col;
	int id;
	char *V = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	if (S) SH = gmt_get_DS_hidden (S);
	for (k = 0; k < GMT->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (scol != GMT->common.a.col[k]) continue;	/* Not the column we want */
		id = gmt_get_ogr_id (GMT->current.io.OGR, GMT->common.a.name[k]);	/* Get the ID */
		V = (SH && SH->ogr) ? SH->ogr->tvalue[id] : GMT->current.io.OGR->tvalue[id];	/* Either from table or from segment (multi) */
		return (gmtio_convert_aspatial_value (GMT, GMT->current.io.OGR->type[id], V));
	}
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "No aspatial value found for column %d [Return NaN]\n", col);
	return (GMT->session.d_NaN);
}

/*! . */
int gmt_load_aspatial_string (struct GMT_CTRL *GMT, struct GMT_OGR *G, uint64_t col, char out[GMT_BUFSIZ]) {
	/* Uses the info in -a and OGR to retrieve the requested aspatial string */

	uint64_t k;
	int64_t scol = col, id = GMT_NOTSET;
	size_t len;
	if (GMT->current.io.ogr != GMT_OGR_TRUE) return (0);		/* No point checking further since file is not GMT/OGR */
	for (k = 0; k < GMT->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (GMT->common.a.col[k] == scol) id = k;			/* ..that matches the given column */
	}
	if (id == GMT_NOTSET) return (0);
	id = gmt_get_ogr_id (G, GMT->common.a.name[id]);
	if (id == GMT_NOTSET) return (0);
	len = strlen (G->tvalue[id]);
	gmt_M_memset (out, GMT_BUFSIZ, char);
	if (G->tvalue[id][0] == '\"' && G->tvalue[id][len-1] == '\"')	/* Skip opening and closing quotes */
		strncpy (out, &G->tvalue[id][1], len-2);
	else
		strcpy (out, G->tvalue[id]);
	return (1);
}

bool gmt_polygon_is_hole (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S) {
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	gmt_M_unused (GMT);
	return (SH->pol_mode == GMT_IS_HOLE || (SH->ogr && SH->ogr->pol_mode == GMT_IS_HOLE));
}

/*! . */
char **gmtlib_get_dir_list (struct GMT_CTRL *GMT, char *path, char *ext) {
	/* Return an array of filenames found in the given directory, or NULL if path cannot be opened.
	 * If ext is not NULL we only return filenames that end in <ext> */
	size_t n = 0, n_alloc = GMT_TINY_CHUNK;
	char **list = NULL;
#ifdef HAVE_DIRENT_H_
	DIR *D = NULL;
	struct dirent *F = NULL;
	size_t d_namlen = 0, e_len = 0;

	if (access (path, F_OK)) return NULL;	/* Quietly skip non-existent directories */
	if ((D = opendir (path)) == NULL) {	/* Unable to open directory listing */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure while opening directory %s\n", path);
		return NULL;
	}
	if (ext) e_len = strlen (ext);
	list = gmt_M_memory (GMT, NULL, n_alloc, char *);
	/* Now read the contents of the dir and add each file to array */
	while ((F = readdir (D)) != NULL) {	/* For each directory entry until end or ok becomes true */
		d_namlen = strlen (F->d_name);
		if (d_namlen == 1U && F->d_name[0] == '.') continue;			/* Skip current dir */
		if (d_namlen == 2U && F->d_name[0] == '.' && F->d_name[1] == '.') continue;	/* Skip parent dir */
#ifdef HAVE_SYS_DIR_H_
		if (F->d_type == DT_DIR) continue;	/* Entry is a directory; skip it */
#endif
		if (ext && strncmp (&F->d_name[d_namlen-e_len], ext, e_len)) continue;	/* Does not end in <ext> */
		list[n++] = strdup (F->d_name);	/* Save the file name */
		if (n == n_alloc) {		/* Allocate more memory for list */
			n_alloc <<= 1;
			list = gmt_M_memory (GMT, list, n_alloc, char *);
		}
	}
	(void)closedir (D);
#elif defined(WIN32)
	char text[PATH_MAX] = {""};
	int left;
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;

	if (access (path, F_OK)) return NULL;	/* Quietly skip non-existent directories */
	snprintf (text, PATH_MAX, "%s/*", path);
	left = PATH_MAX - (int)strlen (path) - 2;
	left -= ((ext) ? (int)strlen (ext) : 2);
	if (ext)
		strncat (text, ext, left);	/* Look for files with given ending in this dir */
	else
		strncat (text, ".*", left);	/* Look for all files in this dir */
	if ((hFind = FindFirstFile(text, &FindFileData)) == INVALID_HANDLE_VALUE) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure while opening directory %s\n", path);
		return NULL;
	}
	list = gmt_M_memory (GMT, NULL, n_alloc, char *);
	do {
		if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) {	/* Don't want the '.' and '..' names */
			list[n++] = strdup(FindFileData.cFileName);	/* Save the file name */
			if (n == n_alloc) {			/* Allocate more memory for list */
				n_alloc <<= 1;
				list = gmt_M_memory (GMT, list, n_alloc, char *);
			}
		}
	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
#else
	GMT_Report (GMT->parent, GMT_MSG_ERROR, "Your OS does not support directory listings\n");
	return NULL;
#endif /* HAVE_DIRENT_H_ */

	list = gmt_M_memory (GMT, list, n + 1, char *);	/* The final entry is NULL, indicating end of list */
	list[n] = NULL;	/* Since the realloc will not necessarily have set it to NULL */
	return (list);
}

/*! . */
void gmtlib_free_dir_list (struct GMT_CTRL *GMT, char ***addr) {
	/* Free allocated array with directory content */
	unsigned int k = 0;
	char **list = *addr;

	while (list[k]) {
		gmt_M_str_free (list[k]);
		k++;
	}
	gmt_M_free (GMT, list);
}

/*! . */
int gmt_remove_file (struct GMT_CTRL *GMT, const char *file) {
	/* Try to remove a file - give error message if it fails.  Depends on extern int errno */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Delete %s\n", file);
	if (!access (file, F_OK) && remove (file)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to remove %s! [remove error: %s]\n", file, strerror (errno));
		return errno;
	}
	return GMT_NOERROR;
}


/*! . */
int gmt_rename_file (struct GMT_CTRL *GMT, const char *oldfile, const char *newfile, unsigned int mode) {
	/* Try to rename a file - give error message if it fails.  Depends on extern int errno.
	 * mode is either GMT_COPY_FILE or GMT_RENAME_FILE */

	if (mode == GMT_COPY_FILE)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Copying %s -> %s\n", oldfile, newfile);
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rename %s -> %s\n", oldfile, newfile);

	errno = GMT_NOERROR;
	if (mode == GMT_COPY_FILE || rename (oldfile, newfile)) {	/* This may be benign as rename won't move files between different mounted partitions on a drive. Copy/remove instead */
		size_t ni, no, total = 0;
		char *chunk = NULL;
		FILE *fpi = NULL, *fpo = NULL;
		if (mode == GMT_RENAME_FILE) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Failed to rename %s -> %s! [rename error: %s].  Try copy/delete instead.\n", oldfile, newfile, strerror (errno));
		/* Open the two files */
		if ((fpo = fopen (newfile, "wb")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to create %s! [fopen error: %s]\n", newfile, strerror (errno));
			return errno;
		}
		if ((fpi = fopen (oldfile, "rb")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to open %s! [fopen error: %s]\n", oldfile, strerror (errno));
			fclose (fpo);
			return errno;
		}
		/* Get memory for reading GMT_BUFSIZ characters at the time */
		if ((chunk = calloc (GMT_BUFSIZ, sizeof (char))) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to allocate memory! [calloc error: %s]\n", strerror (errno));
			fclose (fpi);
			fclose (fpo);
			return errno;
		}
		while ((ni = fread (chunk, sizeof (char), GMT_BUFSIZ, fpi))) {	/* Read until nothing, write each chunk */
			total += ni;
			if ((no = fwrite (chunk, sizeof (char), ni, fpo)) != ni) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to write %" PRIuS " bytes to %s! [fwrite error: %s]\n", ni, newfile, strerror (errno));
				fclose (fpi);
				fclose (fpo);
				free (chunk);	/* Free the chunk */
				return errno;
			}
		}
		free (chunk);	/* Free the chunk */
		if (fclose (fpi)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to close %s! [fwrite error: %s]\n", oldfile, strerror (errno));
			fclose (fpo);
			return errno;
		}
		if (fclose (fpo)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to close %s! [fwrite error: %s]\n", newfile, strerror (errno));
			return errno;
		}
		if (total == 0)
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Source file %s was empty (?): \n", oldfile);

		/* Finally delete the old file */
		if (mode == GMT_RENAME_FILE) errno = gmt_remove_file (GMT, oldfile);
	}
	return errno;
}

void gmt_replace_backslash_in_path (char *dir) {
	size_t k = 0;
	if (dir == NULL) return;	/* No can do */
	while (dir[k]) {
		if (dir[k] == '\\') dir[k] = '/';
		k++;
	}
}

/*! . */
void gmt_set_column (struct GMT_CTRL *GMT, unsigned int direction, unsigned int col, enum gmt_col_enum type) {
	/* Sets the column type for this input or output column or both (dir == GMT_IO) */
	unsigned int start = (direction == GMT_IO) ? GMT_IN : direction;
	unsigned int stop  = (direction == GMT_IO) ? GMT_OUT : direction;
	for (unsigned int dir = start; dir <= stop; dir++) {
		GMT->current.io.col_type[dir][col] = type;
		GMT->current.io.col_set[dir][col] = 1;	/* Flag as having been set and thus should not be automatically changed */
	}
}

int gmt_mkdir (const char *path)
{	/* Simulates mkdir -p behavior in a function for Unix and Windows */
	/* Adapted from http://stackoverflow.com/a/2336245/119527 */
	const size_t len = strlen (path);
	char _path[PATH_MAX] = {""}, sep;
	char *p = NULL;

	errno = 0;	/* Global var: No error so far */

	if (len >= PATH_MAX) {	/* Make sure we don't exceed limits */
		errno = ENAMETOOLONG;
		perror ("gmt_mkdir (too long) error");
		return -1;
	}
	strcpy (_path, path);	/* Copy string so its mutable */

	/* Iterate the string */
	p = (_path[1] == ':') ? _path + 3 : _path + 1;  /* Skip any leading X: drive designators */
	while (*p) { /* Create intermediate directoreis recursively */
		if (*p == '/' || *p == '\\') {	/* Found start of next directory */
			sep = *p;	/* What separator did we use? */
			*p = '\0';	/* Temporarily truncate */

#ifndef _WIN32
			if (mkdir (_path, S_IRWXU) != 0)
#else
			if (mkdir (_path) != 0)
#endif
			{
				if (errno != EEXIST) {	/* Failed to make or visit intermediate directory */
					perror ("gmt_mkdir (intermediate) error");
					return -1;
				}
			}
			*p = sep;	/* Reset the separator */
		}
		p++;
	}

	/* Finally create the last directory name in the path */
#ifndef _WIN32
	if (mkdir (_path, S_IRWXU) != 0)
#else
	if (mkdir (_path) != 0)
#endif
	{
		if (errno != EEXIST) {
			perror ("gmt_mkdir (last dir) error");
			return -1;
		}
	}

	return 0;
}
