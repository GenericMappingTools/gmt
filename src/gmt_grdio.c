/*--------------------------------------------------------------------
 *
 * Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 *
 * G M T _ G R D I O . C   R O U T I N E S
 *
 * Generic routines that take care of all low-level gridfile input/output.
 *
 * Author:  Paul Wessel
 * Date:    1-JAN-2010
 * Version: 5
 * 64-bit Compliant: Yes
 *
 * NOTE: We do not support grids that have more rows or columns that can fit
 * in a 32-bit integer, hence all linear dimensions (row, col, etc.) are addressed
 * with 32-bit ints.  However, 1-D array indices (e.g., ij = row*n_columns + col) are
 * addressed with 64-bit integers.
 *
 * Public functions (42 [+2]):
 *
 *  gmt_grd_get_format      : Get format id, scale, offset and missing value for grdfile
 *  gmtlib_read_grd_info       : Read header from file
 *  gmtlib_read_grd            : Read data set from file (must be preceded by gmtlib_read_grd_info)
 *  gmt_update_grd_info     : Update header in existing file (must be preceded by gmtlib_read_grd_info)
 *  gmtlib_write_grd_info      : Write header to new file
 *  gmtlib_write_grd           : Write header and data set to new file
 *  gmt_set_R_from_grd
 *  gmt_grd_coord           :
 *  gmtlib_grd_real_interleave :
 *  gmt_grd_mux_demux       :
 *  gmtlib_grd_set_units       :
 *  gmt_grd_pad_status      :
 *  gmt_grd_info_syntax
 *  gmtlib_get_grdtype         :
 *  gmt_grd_data_size       :
 *  gmt_grd_set_ij_inc      :
 *  gmt_grd_format_decoder  :
 *  gmt_grd_prep_io         :
 *  gmt_set_grdinc          :
 *  gmt_set_grddim          :
 *  gmt_grd_pad_off         :
 *  gmt_grd_pad_on          :
 *  gmt_grd_pad_zero        :
 *  gmt_create_grid         :
 *  gmt_duplicate_grid      :
 *  gmt_free_grid           :
 *  gmt_set_outgrid         :
 *  gmt_change_grdreg       :
 *  gmt_grd_zminmax         :
 *  gmt_grd_minmax          :
 *  gmt_grd_detrend         :
 *  gmtlib_init_complex        :
 *  gmtlib_check_url_name      :
 *  gmt_read_img            : Read [subset from] a Sandwell/Smith *.img file
 *  gmt_grd_init            : Initialize grd header structure
 *  gmt_grd_shift           : Rotates grdfiles in x-direction
 *  gmt_grd_setregion       : Determines subset coordinates for grdfiles
 *  gmt_grd_is_global       : Determine whether grid is "global", i.e. longitudes are periodic
 *  gmt_adjust_loose_wesn   : Ensures region, increments, and n_columns/n_rows are compatible
 *  gmt_decode_grd_h_info   : Decodes a -Dstring into header text components
 *  gmt_grd_RI_verify       : Test to see if region and incs are compatible
 *  gmt_scale_and_offset_f  : Routine that scales and offsets the data in a vector
 *  gmt_grd_flip_vertical  : Flips the grid in vertical direction
 *  grdio_pack_grid         : Packs or unpacks a grid by calling gmt_scale_and_offset_f()
 *
 *  Reading images via GDAL (if enabled):
 *  gmtlib_read_image          : Read [subset of] an image via GDAL
 *  gmtlib_read_image_info     : Get information for an image via GDAL
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt_dev.h"
#include "gmt_internals.h"
#include "common_byteswap.h"

struct GRD_PAD {	/* Local structure */
	double wesn[4];
	unsigned int pad[4];
};

/* These functions live in other files and are extern'ed in here */
EXTERN_MSC int gmt_is_nc_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmt_is_native_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmt_is_ras_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmt_is_srf_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmt_is_mgg2_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmt_is_agc_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmt_is_esri_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
#ifdef HAVE_GDAL
EXTERN_MSC int gmt_is_gdal_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
#endif
EXTERN_MSC void gmtapi_close_grd (struct GMT_CTRL *GMT, struct GMT_GRID *G);

/* Local functions */

GMT_LOCAL inline struct GMT_GRID    * grdio_get_grid_data (struct GMT_GRID *ptr) {return (ptr);}
GMT_LOCAL inline struct GMT_IMAGE   * grdio_get_image_data (struct GMT_IMAGE *ptr) {return (ptr);}

/*! gmt_M_grd_get_size computes grid size including the padding, and doubles it if complex values */
GMT_LOCAL size_t gmt_grd_get_size (struct GMT_GRID_HEADER *h) {
	return ((((h->complex_mode & GMT_GRID_IS_COMPLEX_MASK) > 0) + 1ULL) * h->mx * h->my);
}

GMT_LOCAL int grdio_grd_layout (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, unsigned int complex_mode, unsigned int direction) {
	/* Checks or sets the array arrangement for a complex array */
	size_t needed_size;	/* Space required to hold both components of a complex grid */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	if ((complex_mode & GMT_GRID_IS_COMPLEX_MASK) == 0) return GMT_OK;	/* Regular, non-complex grid, nothing special to do */

	needed_size = 2ULL * ((size_t)header->mx) * ((size_t)header->my);	/* For the complex array */
	if (header->size < needed_size) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Complex grid not large enough to hold both components!.\n");
		GMT_exit (GMT, GMT_DIM_TOO_SMALL); return GMT_DIM_TOO_SMALL;
	}
	if (direction == GMT_IN) {	/* About to read in a complex component; another one might have been read in earlier */
		if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Demultiplexing complex grid before reading can take place.\n");
			gmt_grd_mux_demux (GMT, header, grid, GMT_GRID_IS_SERIAL);
		}
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == GMT_GRID_IS_COMPLEX_MASK) {	/* Already have both component; this will overwrite one of them */
			unsigned int type = (complex_mode & GMT_GRID_IS_COMPLEX_REAL) ? 0 : 1;
			char *kind[2] = {"read", "imaginary"};
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Overwriting previously stored %s component in complex grid.\n", kind[type]);
		}
		header->complex_mode |= complex_mode;	/* Update the grids complex mode */
	}
	else {	/* About to write out a complex component */
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == 0) {	/* Not a complex grid */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Asking to write out complex components from a non-complex grid.\n");
			GMT_exit (GMT, GMT_WRONG_MATRIX_SHAPE); return GMT_WRONG_MATRIX_SHAPE;
		}
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) && (header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) == 0) {
			/* Programming error: Requesting to write real components when there are none */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Complex grid has no real components that can be written to file.\n");
			GMT_exit (GMT, GMT_WRONG_MATRIX_SHAPE); return GMT_WRONG_MATRIX_SHAPE;
		}
		else if ((header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG) && (header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG) == 0) {
			/* Programming error: Requesting to write imag components when there are none */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Complex grid has no imaginary components that can be written to file.\n");
			GMT_exit (GMT, GMT_WRONG_MATRIX_SHAPE); return GMT_WRONG_MATRIX_SHAPE;
		}
		if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {	/* Must first demultiplex the grid */
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Demultiplexing complex grid before writing can take place.\n");
			gmt_grd_mux_demux (GMT, header, grid, GMT_GRID_IS_SERIAL);
		}
	}
	/* header->arrangement might now have been changed accordingly */
	return GMT_OK;
}

GMT_LOCAL void grdio_grd_parse_xy_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, char *file, unsigned int direction) {
	/* Decode the optional +u|U<unit> and determine scales */
	enum gmt_enum_units u_number;
	unsigned int mode = 0;
	char *c = NULL, *name = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	if (gmt_M_is_geographic (GMT, direction)) return;	/* Does not apply to geographic data */
	name = (file) ? file : HH->name;
	if ((c = gmtlib_file_unitscale (name)) == NULL) return;	/* Did not find any modifier */
	mode = (c[1] == 'u') ? 0 : 1;
	u_number = gmtlib_get_unit_number (GMT, c[2]);		/* Convert char unit to enumeration constant for this unit */
	if (u_number == GMT_IS_NOUNIT) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Grid file x/y unit specification %s was unrecognized (part of file name?) and is ignored.\n", c);
		return;
	}
	/* Got a valid unit */
	HH->xy_unit_to_meter[direction] = GMT->current.proj.m_per_unit[u_number];	/* Converts unit to meters */
	if (mode) HH->xy_unit_to_meter[direction] = 1.0 / HH->xy_unit_to_meter[direction];	/* Wanted the inverse */
	HH->xy_unit[direction] = u_number;	/* Unit ID */
	HH->xy_adjust[direction] |= 1;		/* Says we have successfully parsed and readied the x/y scaling */
	HH->xy_mode[direction] = mode;
	c[0] = '\0';	/* Chop off the unit specification from the file name */
}

GMT_LOCAL void grdio_grd_xy_scale (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int direction) {
	unsigned int k;
	/* Apply the scaling of wesn,inc as given by the header's xy_* settings.
	 * After reading a grid it will have wesn/inc in meters.
	 * Before writing a grid, it may have units changed back to original units
	 * or scaled to another set of units */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	if (direction == GMT_IN) {
		if (HH->xy_adjust[direction] == 0) return;	/* Nothing to do */
		if (HH->xy_adjust[GMT_IN] & 2) return;		/* Already scaled them */
		for (k = 0; k < 4; k++) h->wesn[k] *= HH->xy_unit_to_meter[GMT_IN];
		for (k = 0; k < 2; k++) h->inc[k]  *= HH->xy_unit_to_meter[GMT_IN];
		HH->xy_adjust[GMT_IN] = 2;	/* Now the grid is ready for use and in meters */
		if (HH->xy_mode[direction])
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Input grid file x/y unit was converted from meters to %s after reading.\n", GMT->current.proj.unit_name[HH->xy_unit[GMT_IN]]);
		else
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Input grid file x/y unit was converted from %s to meters after reading.\n", GMT->current.proj.unit_name[HH->xy_unit[GMT_IN]]);
	}
	else if (direction == GMT_OUT) {	/* grid x/y are assumed to be in meters */
		if (HH->xy_adjust[GMT_OUT] & 1) {	/* Was given a new unit for output */
			for (k = 0; k < 4; k++) h->wesn[k] /= HH->xy_unit_to_meter[GMT_OUT];
			for (k = 0; k < 2; k++) h->inc[k]  /= HH->xy_unit_to_meter[GMT_OUT];
			HH->xy_adjust[GMT_OUT] = 2;	/* Now we are ready for writing */
			if (HH->xy_mode[GMT_OUT])
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output grid file x/y unit was converted from %s to meters before writing.\n", GMT->current.proj.unit_name[HH->xy_unit[GMT_OUT]]);
			else
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output grid file x/y unit was converted from meters to %s before writing.\n", GMT->current.proj.unit_name[HH->xy_unit[GMT_OUT]]);
		}
		else if (HH->xy_adjust[GMT_IN] & 2) {	/* Just undo old scaling */
			for (k = 0; k < 4; k++) h->wesn[k] /= HH->xy_unit_to_meter[GMT_IN];
			for (k = 0; k < 2; k++) h->inc[k]  /= HH->xy_unit_to_meter[GMT_IN];
			HH->xy_adjust[GMT_IN] -= 2;	/* Now it is back to where we started */
			if (HH->xy_mode[GMT_OUT])
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output grid file x/y unit was reverted back to %s from meters before writing.\n", GMT->current.proj.unit_name[HH->xy_unit[GMT_IN]]);
			else
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output grid file x/y unit was reverted back from meters to %s before writing.\n", GMT->current.proj.unit_name[HH->xy_unit[GMT_IN]]);
		}
	}
}

/* Routines to see if a particular grd file format is specified as part of filename. */

GMT_LOCAL void grdio_expand_filename (struct GMT_CTRL *GMT, const char *file, char *fname) {
	bool found;
	unsigned int i;
	size_t f_length, length;

	if (GMT->current.setting.io_gridfile_shorthand) {	/* Look for matches */
		f_length = strlen (file);
		for (i = 0, found = false; !found && i < GMT->session.n_shorthands; ++i) {
			length = strlen (GMT->session.shorthand[i].suffix);
			found = (length > f_length) ? false : !strncmp (&file[f_length - length], GMT->session.shorthand[i].suffix, length);
		}
		if (found) {	/* file ended in a recognized shorthand extension */
			--i;
			sprintf (fname, "%s=%s", file, GMT->session.shorthand[i].format);
		}
		else
			strcpy (fname, file);
	}
	else	/* Simply copy the full name */
		strcpy (fname, file);
}

enum Grid_packing_mode {
	k_grd_pack = 0, /* scale and offset before writing to disk */
	k_grd_unpack    /* remove scale and offset after reading packed data */
};

GMT_LOCAL void grdio_pack_grid (struct GMT_CTRL *Ctrl, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, unsigned pack_mode) {
	size_t n_representations = 0; /* number of distinct values >= 0 that a signed integral type can represent */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (pack_mode == k_grd_pack && (HH->z_scale_autoadjust || HH->z_offset_autoadjust)) {
		switch (Ctrl->session.grdformat[header->type][1]) {
			case 'b':
				n_representations = 128;         /* exp2 (8 * sizeof (int8_t)) / 2 */
				break;
			case 's':
				n_representations = 32768;       /* exp2 (8 * sizeof (int16_t)) / 2 */
				break;
			case 'i':
				/* A single precision float's significand has a precision of 24 bits.
				 * In order to avoid round-off errors, we must not use all 2^32
				 * n_representations of an int32_t. */
				n_representations = 0x1000000;   /* exp2 (24) */
				break;
			/* default: do not auto-scale floating point */
		}
	}

	if (n_representations != 0) {
		/* Calculate auto-scale and offset */
		gmt_grd_zminmax (Ctrl, header, grid); /* Calculate z_min/z_max */
		if (HH->z_offset_autoadjust) {
			/* shift to center values around 0 but shift only by integral value */
			double z_range = header->z_max - header->z_min;
			if (isfinite (z_range))
				header->z_add_offset = rint(z_range / 2.0 + header->z_min);
		}
		if (HH->z_scale_autoadjust) {
			/* scale z-range to use all n_representations */
			double z_max = header->z_max - header->z_add_offset;
			double z_min = fabs(header->z_min - header->z_add_offset);
			double z_0_n_range = MAX (z_max, z_min); /* use [0,n] range because of signed int */
			--n_representations;                     /* subtract 1 for NaN value */
			if (isnormal (z_0_n_range))
				header->z_scale_factor = z_0_n_range / n_representations;
		}
	}

	/* Do actual packing/unpacking: */
	switch (pack_mode) {
		case k_grd_unpack:
			gmt_scale_and_offset_f (Ctrl, grid, header->size, header->z_scale_factor, header->z_add_offset);
			/* Adjust z-range in header: */
			header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
			header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;
			if (header->z_scale_factor < 0.0) gmt_M_double_swap (header->z_min, header->z_max);
			break;
		case k_grd_pack:
			gmt_scale_and_offset_f (Ctrl, grid, header->size, 1.0/header->z_scale_factor, -header->z_add_offset/header->z_scale_factor);
			break;
		default:
			assert (false); /* grdio_pack_grid() called with illegal pack_mode */
	}
}

GMT_LOCAL int grdio_parse_grd_format_scale_old (struct GMT_CTRL *Ctrl, struct GMT_GRID_HEADER *header, char *format) {
	/* parses format string after =-suffix: ff/scale/offset/invalid
	 * ff:      can be one of [abcegnrs][bsifd]
	 * scale:   can be any non-zero normalized number or 'a' for scale and
	 *          offset auto-adjust, defaults to 1.0 if omitted
	 * offset:  can be any finite number or 'a' for offset auto-adjust, defaults to 0 if omitted
	 * invalid: can be any finite number, defaults to NaN if omitted
	 * scale and offset may be left empty (e.g., ns//a will auto-adjust the offset only)
	 */

	char type_code[3];
	char *p;
	int err; /* gmt_M_err_trap */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	/* decode grid type */
	strncpy (type_code, format, 2);
	type_code[2] = '\0';
	if (type_code[0] == '/')		/* user passed a scale with no id code  =/scale[...]. Assume "nf" */
		header->type = GMT_GRID_IS_NF;
	else {
		err = gmt_grd_format_decoder (Ctrl, type_code, &header->type); /* update header type id */
		if (err != GMT_NOERROR)
			return err;
	}

	/* parse scale/offset/invalid if any */
	p = strchr (format, '/');
	if (p != NULL && *p) {
		++p;
		/* parse scale */
		if (*p == 'a')
			HH->z_scale_autoadjust = HH->z_offset_autoadjust = true;
		else
			sscanf (p, "%lf", &header->z_scale_factor);
	}
	else
		return GMT_NOERROR;

	p = strchr (p, '/');
	if (p != NULL && *p) {
		++p;
		/* parse offset */
		if (*p != 'a') {
			HH->z_offset_autoadjust = false;
			sscanf (p, "%lf", &header->z_add_offset);
		}
		else
			HH->z_offset_autoadjust = true;
	}
	else
		return GMT_NOERROR;

	p = strchr (p, '/');
	if (p != NULL && *p) {
		++p;
		/* parse invalid value */
#ifdef DOUBLE_PRECISION_GRID
		sscanf (p, "%lf", &header->nan_value);
#else
		sscanf (p, "%f", &header->nan_value);
#endif

		/* header->nan_value should be of same type as (float)*grid to avoid
		 * round-off errors. For example, =gd///-3.4028234e+38:gtiff, would fail
		 * otherwise because the GTiff'd NoData values are of type double but the
		 * grid is truncated to float.
		 * Don't allow infitiy: */
		if (!isfinite (header->nan_value))
			header->nan_value = (gmt_grdfloat)NAN;
	}

	return GMT_NOERROR;
}

GMT_LOCAL int grdio_parse_grd_format_scale_new (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, char *format) {
	/* parses format string after = suffix: ff[+f<scale>][+o<offset>/][+n<invalid>]
	 * ff:      can be one of [abcegnrs][bsifd]
	 * scale:   can be any non-zero normalized number or 'a' for scale and
	 *          offset auto-adjust, defaults to 1.0 if omitted
	 * offset:  can be any finite number or 'a' for offset auto-adjust, defaults to 0 if omitted
	 * invalid: can be any finite number, defaults to NaN if omitted
	 * scale and offset may be left empty (e.g., ns//a will auto-adjust the offset only)
	 */

	char type_code[3];
	char p[GMT_BUFSIZ] = {""};
	unsigned int pos = 0, uerr = 0;
	int err; /* gmt_M_err_trap */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	/* decode grid type */
	strncpy (type_code, format, 2);
	type_code[2] = '\0';
	if (type_code[0] == '+')		/* User passed a scale, offset, or nan-modifier with no id code, e.g.,  =+s<scale>[...]. Assume "nf" format */
		header->type = GMT_GRID_IS_NF;
	else {	/* Match given code with known codes */
		err = gmt_grd_format_decoder (GMT, type_code, &header->type); /* update header type id */
		if (err != GMT_NOERROR)
			return err;
	}

	while (gmt_getmodopt (GMT, 0, format, "bnos", &pos, p, &uerr) && uerr == 0) {	/* Looking for +b, +n, +o, +s */
		switch (p[0]) {
			case 'b':	/* bands */
				break;
			case 'n':	/* Nan value */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_parse_grd_format_scale_new: Using %s to represent missing value (NaN)\n", &p[1]);
				header->nan_value = (gmt_grdfloat)atof (&p[1]);
				/* header->nan_value should be of same type as (gmt_grdfloat)*grid to avoid
				 * round-off errors. For example, =gd+n-3.4028234e+38:gtiff, would fail
				 * otherwise because the GTiff'd NoData values are of type double but the
				 * grid is truncated to gmt_grdfloat.  Don't allow infinity: */
				if (!isfinite (header->nan_value))
					header->nan_value = (gmt_grdfloat)NAN;
				break;
			case 'o':	/* parse offset */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_parse_grd_format_scale_new: Setting offset as %s\n", &p[1]);
				if (p[1] != 'a') {
					HH->z_offset_autoadjust = false;
					HH->z_offset_given = true;
					header->z_add_offset = atof (&p[1]);
				}
				else
					HH->z_offset_autoadjust = true;
				break;
			case 's':	/* parse scale */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_parse_grd_format_scale_new: Setting scale as %s\n", &p[1]);
				if (p[1] == 'a') /* This sets both scale and offset to auto */
					HH->z_scale_autoadjust = HH->z_offset_autoadjust = true;
				else {
					header->z_scale_factor = atof (&p[1]);
					HH->z_scale_given = true;
				}
				break;
			default:	/* These are caught in gmt_getmodopt so break is just for Coverity */
				break;
		}
	}
	return (uerr) ? GMT_NOT_A_VALID_MODIFIER : GMT_NOERROR;
}

GMT_LOCAL int grdio_parse_grd_format_scale (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, char *format) {
	int code;
	if (strstr(format, "+s") || strstr(format, "+o") || strstr(format, "+n"))
		code = grdio_parse_grd_format_scale_new (GMT, header, format);
	else
		code = grdio_parse_grd_format_scale_old (GMT, header, format);
	return (code);
}

GMT_LOCAL bool eq (double this, double that, double inc) {
	/* this and that are the same value if less than 10e-4 * inc apart */
	return ((fabs (this - that) / inc) < GMT_CONV4_LIMIT);
}

GMT_LOCAL int grdio_padspace (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double *wesn, unsigned int *pad, struct GRD_PAD *P) {
	/* When padding is requested it is usually used to set boundary conditions based on
	 * two extra rows/columns around the domain of interest.  BCs like natural or periodic
	 * can then be used to fill in the pad.  However, if the domain is taken from a grid
	 * whose full domain exceeds the region of interest we are better off using the extra
	 * data to fill those pad rows/columns.  Thus, this function tries to determine if the
	 * input grid has the extra data we need to fill the BC pad with observations. */
	bool wrap;
	unsigned int side, n_sides = 0;
	double wesn2[4];
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	gmt_M_unused(GMT);

	/* First copy over original settings to the Pad structure */
	gmt_M_memset (P, 1, struct GRD_PAD);					/* Initialize to zero */
	gmt_M_memcpy (P->pad, pad, 4, int);					/* Duplicate the pad */
	if (!wesn) return (false);						/* No subset requested */
	if (wesn[XLO] == wesn[XHI] && wesn[YLO] == wesn[YHI]) return (false);	/* Subset not set */
	if (eq (wesn[XLO], header->wesn[XLO], header->inc[GMT_X]) && eq (wesn[XHI], header->wesn[XHI], header->inc[GMT_X])
		&& eq (wesn[YLO], header->wesn[YLO], header->inc[GMT_Y]) && eq (wesn[YHI], header->wesn[YHI], header->inc[GMT_Y]))
		return (false);	/* Subset equals whole area */
	gmt_M_memcpy (P->wesn, wesn, 4, double);					/* Copy the subset boundaries */
	if (pad[XLO] == 0 && pad[XHI] == 0 && pad[YLO] == 0 && pad[YHI] == 0) return (false);	/* No padding requested */

	/* Determine if data exist for a pad on all four sides.  If not we give up */
	wrap = gmt_grd_is_global (GMT, header);	/* If global wrap then we cannot be outside */
	if ((wesn2[XLO] = wesn[XLO] - pad[XLO] * header->inc[GMT_X]) < header->wesn[XLO] && !wrap)	/* Cannot extend west/xmin */
		{ n_sides++; wesn2[XLO] = wesn[XLO]; }
	else	/* OK to load left pad with data */
		P->pad[XLO] = 0;
	if ((wesn2[XHI] = wesn[XHI] + pad[XHI] * header->inc[GMT_X]) > header->wesn[XHI] && !wrap)	/* Cannot extend east/xmax */
		{ n_sides++; wesn2[XHI] = wesn[XHI]; }
	else	/* OK to load right pad with data */
		P->pad[XHI] = 0;
	if ((wesn2[YLO] = wesn[YLO] - pad[YLO] * header->inc[GMT_Y]) < header->wesn[YLO])	/* Cannot extend south/ymin */
		{ n_sides++; wesn2[YLO] = wesn[YLO]; }
	else	/* OK to load bottom pad with data */
		P->pad[YLO] = 0;
	if ((wesn2[YHI] = wesn[YHI] + pad[YHI] * header->inc[GMT_Y]) > header->wesn[YHI])	/* Cannot extend north/ymax */
		{ n_sides++; wesn2[YHI] = wesn[YHI]; }
	else	/* OK to load top pad with data */
		P->pad[YHI] = 0;
	if (n_sides == 4) return (false);	/* No can do */

	/* Here we know that there is enough input data to fill some or all of the BC pad with actual data values */
	/* We have temporarily set padding to zero (since the pad is now part of the region) for those sides we can extend */

	/* Temporarily enlarge the region so it now includes the padding we need */
	gmt_M_memcpy (P->wesn, wesn2, 4, double);

	/* Set BC */
	for (side = 0; side < 4; side++) {
		if (P->pad[side] == 0)
			HH->BC[side] = GMT_BC_IS_DATA;
	}

	return (true);	/* Return true so the calling function can take appropriate action */
}

GMT_LOCAL void handle_pole_averaging (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, gmt_grdfloat f_value, int pole)
{
	uint64_t node;
	unsigned int col = 0;
	char *name[3] = {"south", "", "north"};
	gmt_M_unused(GMT);

	if (pole == -1)
		node = gmt_M_ijp (header, header->n_rows-1, 0);	/* First node at S pole */
	else
		node = gmt_M_ijp (header, 0, 0);		/* First node at N pole */
	if (gmt_M_type (GMT, GMT_OUT, GMT_Z) == GMT_IS_AZIMUTH || gmt_M_type (GMT, GMT_OUT, GMT_Z) == GMT_IS_ANGLE) {	/* Must average azimuths */
		uint64_t orig = node;
		double s, c, sum_s = 0.0, sum_c = 0.0;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Average %d angles at the %s pole\n", header->n_columns, name[pole+1]);
		for (col = 0; col < header->n_columns; col++, node++) {
			sincosd ((double)grid[node], &s, &c);
			sum_s += s;	sum_c += c;
		}
		f_value = (gmt_grdfloat)atan2d (sum_s, sum_c);
		node = orig;
	}
	for (col = 0; col < header->n_columns; col++, node++) grid[node] = f_value;
}

GMT_LOCAL void grdio_grd_check_consistency (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid) {
	/* Enforce before writing a grid that periodic grids with repeating columns
	 * agree on the node values in those columns; if different replace with average.
	 * This only affects geographic grids of 360-degree extent with gridline registration.
	 * Also, if geographic grid with gridline registration, if the N or S pole row is present
	 * we ensure that they all have identical values, otherwise replace by mean value */
	unsigned int row = 0, col = 0;
	unsigned int we_conflicts = 0, p_conflicts = 0;
	uint64_t left = 0, right = 0, node = 0;

	if (header->registration == GMT_GRID_PIXEL_REG) return;	/* Not gridline registered */
	if (gmt_M_is_cartesian (GMT, GMT_OUT)) return;		/* Not geographic */
	if (header->wesn[YLO] == -90.0) {	/* Check consistency of S pole duplicates */
		double sum;
		node = gmt_M_ijp (header, header->n_rows-1, 0);	/* First node at S pole */
		sum = grid[node++];
		p_conflicts = 0;
		for (col = 1; col < header->n_columns; col++, node++) {
			if (grid[node] != grid[node-1]) p_conflicts++;
			sum += grid[node];
		}
		if (p_conflicts) {
			gmt_grdfloat f_value = (gmt_grdfloat)(sum / header->n_columns);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected %u inconsistent values at south pole. Values fixed by setting all to average row value.\n", p_conflicts);
			handle_pole_averaging (GMT, header, grid, f_value, -1);
		}
	}
	if (header->wesn[YHI] == +90.0) {	/* Check consistency of N pole duplicates */
		double sum;
		node = gmt_M_ijp (header, 0, 0);	/* First node at N pole */
		sum = grid[node++];
		p_conflicts = 0;
		for (col = 1; col < header->n_columns; col++, node++) {
			if (grid[node] != grid[node-1]) p_conflicts++;
			sum += grid[node];
		}
		if (p_conflicts) {
			gmt_grdfloat f_value = (gmt_grdfloat)(sum / header->n_columns);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected %u inconsistent values at north pole. Values fixed by setting all to average row value.\n", p_conflicts);
			handle_pole_averaging (GMT, header, grid, f_value, +1);
		}
	}
	if (!gmt_M_360_range (header->wesn[XLO], header->wesn[XHI])) return;	/* Not 360-degree range */

	for (row = 0; row < header->n_rows; row++) {
		left = gmt_M_ijp (header, row, 0);	/* Left node */
		right = left + header->n_columns - 1;		/* Right node */
		if (grid[right] != grid[left]) {
			grid[right] = grid[left];
			we_conflicts++;
		}
	}
	if (we_conflicts)
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Detected %u inconsistent values along periodic east boundary of grid. Values fixed by duplicating west boundary.\n", we_conflicts);
}

GMT_LOCAL void grdio_grd_wipe_pad (struct GMT_CTRL *GMT, struct GMT_GRID *G) {
	/* Reset padded areas to 0. */
	unsigned int row;
	size_t ij0;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (G->header);

	if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Calling grdio_grd_wipe_pad on interleaved complex grid! Programming error?\n");
		return;
	}
	if (G->header->pad[YHI]) gmt_M_memset (G->data, G->header->mx * G->header->pad[YHI], gmt_grdfloat);	/* Wipe top pad */
	if (G->header->pad[YLO]) {	/* Wipe bottom pad */
		ij0 = gmt_M_ij (G->header, G->header->my - G->header->pad[YLO], 0);	/* Index of start of bottom pad */
		gmt_M_memset (&(G->data[ij0]), G->header->mx * G->header->pad[YLO], gmt_grdfloat);
	}
	if (G->header->pad[XLO] == 0 && G->header->pad[XHI] == 0) return;	/* Nothing to do */
	for (row = G->header->pad[YHI]; row < G->header->my - G->header->pad[YLO]; row++) {	/* Wipe left and right pad which is trickier */
		ij0 = gmt_M_ij (G->header, row, 0);	/* Index of this row's left column (1st entry in west pad) */
		if (G->header->pad[XLO]) gmt_M_memset (&(G->data[ij0]), G->header->pad[XLO], gmt_grdfloat);
		ij0 += (G->header->mx - G->header->pad[XHI]);	/* Start of this rows east pad's 1st column */
		if (G->header->pad[XHI]) gmt_M_memset (&(G->data[ij0]), G->header->pad[XHI], gmt_grdfloat);
	}
}

GMT_LOCAL void grdio_pad_grd_off_sub (struct GMT_GRID *G, gmt_grdfloat *data) {
	/* Remove the current grid pad and shuffle all rows to the left */
	uint64_t ijp, ij0;
	unsigned int row;
	for (row = 0; row < G->header->n_rows; row++) {
		ijp = gmt_M_ijp (G->header, row, 0);	/* Index of start of this row's first column in padded grid  */
		ij0 = gmt_M_ij0 (G->header, row, 0);	/* Index of start of this row's first column in unpadded grid */
		gmt_M_memcpy (&(data[ij0]), &(data[ijp]), G->header->n_columns, gmt_grdfloat);	/* Only copy the n_columns data values */
	}
}

GMT_LOCAL void grdio_pad_grd_on_sub (struct GMT_CTRL *GMT, struct GMT_GRID *G, struct GMT_GRID_HEADER *h_old, gmt_grdfloat *data) {
	/* Use G for dimensions but operate on data array which points to either the real or imaginary section */
	uint64_t ij_new, ij_old, row, col, start_last_new_row, end_last_old_row;

	/* See if the index of start of last new row exceeds index of last node in old grid */
	start_last_new_row = gmt_M_get_nm (GMT, G->header->pad[YHI] + G->header->n_rows - 1, G->header->pad[XLO] + G->header->n_columns + G->header->pad[XHI]) + G->header->pad[XLO];
	end_last_old_row   = gmt_M_get_nm (GMT, h_old->pad[YHI] + h_old->n_rows - 1, h_old->pad[XLO] + h_old->n_columns + h_old->pad[XHI]) + h_old->pad[XLO] + h_old->n_columns - 1;
	if (start_last_new_row > end_last_old_row && (start_last_new_row - end_last_old_row) > G->header->n_columns) {        /* May copy whole rows from bottom to top */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_pad_grd_on_sub can copy row-by-row\n");
		for (row = G->header->n_rows; row > 0; row--) {
			ij_new = gmt_M_ijp (G->header, row-1, 0);   /* Index of this row's first column in new padded grid  */
			ij_old = gmt_M_ijp (h_old, row-1, 0);       /* Index of this row's first column in old padded grid */
			gmt_M_memcpy (&(data[ij_new]), &(data[ij_old]), G->header->n_columns, gmt_grdfloat);
		}
	}
	else {	/* Must do it from bottom to top on a per node basis */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_pad_grd_on_sub must copy node-by-node\n");
		ij_new = gmt_M_ijp (G->header, G->header->n_rows-1, G->header->n_columns-1); /* Index of this row's last column in new padded grid  */
		ij_old = gmt_M_ijp (h_old, h_old->n_rows-1, h_old->n_columns-1);     /* Index of this row's last column in old padded grid */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_pad_grd_on_sub: last ij_new = %d, last ij_old = %d\n", (int)ij_new, (int)ij_old);

		if (ij_new > ij_old) {	/* Can go back to front */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_pad_grd_on_sub: Must loop from end to front\n");
			for (row = G->header->n_rows; row > 0; row--) {
				ij_new = gmt_M_ijp (G->header, row-1, G->header->n_columns-1); /* Index of this row's last column in new padded grid  */
				ij_old = gmt_M_ijp (h_old, row-1, h_old->n_columns-1);     /* Index of this row's last column in old padded grid */
				for (col = 0; col < G->header->n_columns; col++)
					data[ij_new--] = data[ij_old--];
			}
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "grdio_pad_grd_on_sub: Must loop from front to end\n");
			for (row = 0; row < G->header->n_rows; row++) {
				ij_new = gmt_M_ijp (G->header, row, 0); /* Index of this row's last column in new padded grid  */
				ij_old = gmt_M_ijp (h_old, row, 0);     /* Index of this row's last column in old padded grid */
				for (col = 0; col < G->header->n_columns; col++)
					data[ij_new++] = data[ij_old++];
			}
		}
	}
	grdio_grd_wipe_pad (GMT, G);	/* Set pad areas to 0 */
}

GMT_LOCAL void grdio_pad_grd_zero_sub (struct GMT_GRID *G, gmt_grdfloat *data) {
	unsigned int row, col, nx1;
	uint64_t ij_f, ij_l;

	if (G->header->pad[YHI]) gmt_M_memset (data, G->header->pad[YHI] * G->header->mx, gmt_grdfloat);		/* Zero the top pad */
	nx1 = G->header->n_columns - 1;	/* Last column */
	gmt_M_row_loop (NULL, G, row) {
		ij_f = gmt_M_ijp (G->header, row,   0);				/* Index of first column this row  */
		ij_l = gmt_M_ijp (G->header, row, nx1);				/* Index of last column this row */
		for (col = 1; col <= G->header->pad[XLO]; col++) data[ij_f-col] = 0.0f;	/* Zero the left pad at this row */
		for (col = 1; col <= G->header->pad[XHI]; col++) data[ij_l+col] = 0.0f;	/* Zero the left pad at this row */
	}
	if (G->header->pad[YLO]) {
		int pad = G->header->pad[XLO];
		ij_f = gmt_M_ijp (G->header, G->header->n_rows, -pad);				/* Index of first column of bottom pad  */
		gmt_M_memset (&(data[ij_f]), G->header->pad[YLO] * G->header->mx, gmt_grdfloat);	/* Zero the bottom pad */
	}
}

GMT_LOCAL struct GMT_GRID_HEADER *grdio_duplicate_gridheader (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* Duplicates a grid header. */
	struct GMT_GRID_HEADER *hnew = gmt_get_header (GMT);
	gmt_copy_gridheader (GMT, hnew, h);
	return (hnew);
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

#ifdef DEBUG
/* Uncomment this to have gmt_grd_dump be called and do something */
/* #define GMT_DUMPING */
#ifdef GMT_DUMPING
void gmt_grd_dump (struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, bool is_complex, char *txt) {
	unsigned int row, col;
	uint64_t k = 0U;
	fprintf (stderr, "Dump [%s]:\n---------------------------------------------\n", txt);
	for (row = 0; row < header->my; row++) {
		if (is_complex)
			for (col = 0; col < header->mx; col++, k+= 2) fprintf (stderr, "(%g,%g)\t", grid[k], grid[k+1]);
		else
			for (col = 0; col < header->mx; col++, k++) fprintf (stderr, "%g\t", grid[k]);
		fprintf (stderr, "\n");
	}
	fprintf (stderr, "---------------------------------------------\n");
}
#else	/* Just a dummy */
void gmt_grd_dump (struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, bool is_complex, char *txt) {
	gmt_M_unused(header); gmt_M_unused(grid); gmt_M_unused(is_complex); gmt_M_unused(txt);
	/* Nothing */
}
#endif
#endif

void gmt_copy_gridheader (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *to, struct GMT_GRID_HEADER *from) {
	/* Destination must exist */
	struct GMT_GRID_HEADER_HIDDEN *Hfrom = gmt_get_H_hidden (from), *Hto = gmt_get_H_hidden (to);
	gmt_M_unused(GMT);
	if (to->ProjRefWKT) gmt_M_str_free (to->ProjRefWKT);		/* Since we will duplicate via from */
	if (to->ProjRefPROJ4) gmt_M_str_free (to->ProjRefPROJ4);	/* Since we will duplicate via from */
	if (Hto->pocket) gmt_M_str_free (Hto->pocket);			/* Since we will duplicate via from */
	gmt_M_memcpy (to, from, 1, struct GMT_GRID_HEADER);		/* Copies full contents but also duplicates the hidden address */
	to->hidden = Hto;	/* Restore the original hidden address in to */
	gmt_M_memcpy (to->hidden, from->hidden, 1, struct GMT_GRID_HEADER_HIDDEN);	/* Copies full contents of hidden area */
	/* Must deal with three pointers individually */
	if (from->ProjRefWKT) to->ProjRefWKT = strdup (from->ProjRefWKT);
	if (from->ProjRefPROJ4) to->ProjRefPROJ4 = strdup (from->ProjRefPROJ4);
	if (Hfrom->pocket) Hto->pocket = strdup (Hfrom->pocket);
}

/*! gmt_grd_is_global returns true for a geographic grid with exactly 360-degree range (with or without repeating column) */
bool gmt_grd_is_global (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	bool global;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);
	gmt_M_unused(GMT);
	global = (HH->grdtype == GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT || HH->grdtype == GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT);
	return (global);
}

/*! gmt_grd_is_polar returns true for a geographic grid with exactly 180-degree range in latitude (with or without repeating column) */
bool gmt_grd_is_polar (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	if (!gmt_M_y_is_lat (GMT, GMT_IN))
		return false;
	if (gmt_M_180_range (h->wesn[YHI], h->wesn[YLO]))
		return true;
	if (fabs (h->n_rows * h->inc[GMT_Y] - 180.0) < GMT_CONV4_LIMIT)
		return true;
	return false;
}

void gmt_set_R_from_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* When no -R was given we will inherit the region from the grid.  However,
	 * many grids are hobbled by not clearly specifying they are truly global grids.
	 * What frequently happens is that gridnode-registered grids omit the repeating
	 * column in the east, leading to regions such as -R0/359/-90/90 for a 1-degree grid.
	 * Since these are clearly global we do now want to pass 0/359 to the projection
	 * machinery but 0/360.  Hence we test if the grid is truly global and make this decision. */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	gmt_M_memcpy (GMT->common.R.wesn, header->wesn, 4, double);	/* Initially we set -R as is from grid header */
	if (HH->grdtype != GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT) return;	/* Nothing to do */
	if (!gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && fabs (header->n_columns * header->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
		/* The w/e need to state the complete 360 range: Let east = 360 + west */
		GMT->common.R.wesn[XHI] = GMT->common.R.wesn[XLO] + 360.0;
	}
	if (!gmt_M_180_range (GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]) && fabs (header->n_rows * header->inc[GMT_Y] - 180.0) < GMT_CONV4_LIMIT) {
		/* The s/n need to state the complete 180 range */
		GMT->common.R.wesn[YLO] = -90.0;
		GMT->common.R.wesn[YHI] = +90.0;
	}
}

void gmtlib_grd_get_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Set input data types for columns 0, 1 and 2 based on unit strings for
	   grid coordinates x, y and z.
	   When "Time": transform the data scale and offset to match the current time system.
	*/
	unsigned int i;
	char string[3][GMT_LEN256], *units = NULL;
	double scale = 1.0, offset = 0.0;
	struct GMT_TIME_SYSTEM time_system;

	/* Copy unit strings */
	strncpy (string[0], header->x_units, GMT_GRID_UNIT_LEN80);
	strncpy (string[1], header->y_units, GMT_GRID_UNIT_LEN80);
	strncpy (string[2], header->z_units, GMT_GRID_UNIT_LEN80);

	/* Parse the unit strings one by one */
	for (i = 0; i < 3; i++) {
		/* Skip parsing when input data type is already set */
		if (GMT->current.io.col_type[GMT_IN][i] & GMT_IS_GEO) continue;
		if (GMT->current.io.col_type[GMT_IN][i] & GMT_IS_RATIME) {
			GMT->current.proj.xyz_projection[i] = GMT_TIME;
			continue;
		}

		/* Change name of variable and unit to lower case for comparison */
		gmt_str_tolower (string[i]);

		if ((!strncmp (string[i], "longitude", 9U) || strstr (string[i], "degrees_e")) && (header->wesn[XLO] > -360.0 && header->wesn[XHI] <= 360.0)) {
			/* Input data type is longitude */
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_LON);
		}
		else if ((!strncmp (string[i], "latitude", 8U) || strstr (string[i], "degrees_n")) && (header->wesn[YLO] >= -90.0 && header->wesn[YHI] <= 90.0)) {
			/* Input data type is latitude */
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_LAT);
		}
		else if (!strcmp (string[i], "time") || !strncmp (string[i], "time [", 6U)) {
			/* Input data type is time */
			gmt_set_column (GMT, GMT_IN, i, GMT_IS_RELTIME);
			GMT->current.proj.xyz_projection[i] = GMT_TIME;

			/* Determine coordinates epoch and units (default is internal system) */
			gmt_M_memcpy (&time_system, &GMT->current.setting.time_system, 1, struct GMT_TIME_SYSTEM);
			units = strchr (string[i], '[');
			if (!units || gmt_get_time_system (GMT, ++units, &time_system) || gmt_init_time_system_structure (GMT, &time_system))
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Time units [%s] in grid not recognised, defaulting to gmt.conf.\n", units);

			/* Determine scale between grid and internal time system, as well as the offset (in internal units) */
			scale = time_system.scale * GMT->current.setting.time_system.i_scale;
			offset = (time_system.rata_die - GMT->current.setting.time_system.rata_die) + (time_system.epoch_t0 - GMT->current.setting.time_system.epoch_t0);
			offset *= GMT_DAY2SEC_F * GMT->current.setting.time_system.i_scale;

			/* Scale data scale and extremes based on scale and offset */
			if (i == 0) {
				header->wesn[XLO] = header->wesn[XLO] * scale + offset;
				header->wesn[XHI] = header->wesn[XHI] * scale + offset;
				header->inc[GMT_X] *= scale;
			}
			else if (i == 1) {
				header->wesn[YLO] = header->wesn[YLO] * scale + offset;
				header->wesn[YHI] = header->wesn[YHI] * scale + offset;
				header->inc[GMT_Y] *= scale;
			}
			else {
				header->z_add_offset = header->z_add_offset * scale + offset;
				header->z_scale_factor *= scale;
			}
		}
	}
}

double * gmt_grd_coord (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, int dir) {
	/* Allocate, compute, and return the x- or y-coordinates for a grid */
	unsigned int k;
	double *coord = NULL;
	assert (dir == GMT_X || dir == GMT_Y);
	if (dir == GMT_X) {
		coord = gmt_M_memory (GMT, NULL, header->n_columns, double);
		for (k = 0; k < header->n_columns; k++) coord[k] = gmt_M_grd_col_to_x (GMT, k, header);
	}
	else if (dir == GMT_Y) {
		coord = gmt_M_memory (GMT, NULL, header->n_rows, double);
		for (k = 0; k < header->n_rows; k++) coord[k] = gmt_M_grd_row_to_y (GMT, k, header);
	}

	return (coord);
}

void gmtlib_grd_real_interleave (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *data) {
	/* Sub-function since this step is also needed in the specific case when a module
	 * calls GMT_Read_Data with GMT_GRID_IS_COMPLEX_REAL but input comes via an
	 * external interface (MATLAB, Python) and thus has not been multiplexed yet.
	 * We assume the data array is already large enough to hold the double-sized grid.
	 */
	uint64_t row, col, col_1, col_2, left_node_1, left_node_2;
	gmt_M_unused(GMT);
	/* Here we have a grid with RRRRRR..._________ and want R_R_R_R_... */
	for (row = header->my; row > 0; row--) {	/* Going from last to first row */
		left_node_1 = gmt_M_ij (header, row-1, 0);	/* Start of row in RRRRR layout */
		left_node_2 = 2 * left_node_1;			/* Start of same row in R_R_R_ layout */
		for (col = header->mx, col_1 = col - 1, col_2 = 2*col - 1; col > 0; col--, col_1--) { /* Go from right to left */
			data[left_node_2+col_2] = 0.0f;	col_2--;	/* Set the Imag component to zero */
			data[left_node_2+col_2] = data[left_node_1+col_1];	col_2--;
		}
	}
}

void gmt_grd_mux_demux (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *data, unsigned int desired_mode) {
	/* Multiplex and demultiplex complex grids.
 	 * Complex grids are read/written by dealing with just one component: real or imag.
	 * Thus, at read|write the developer must specify which component (GMT_CMPLX_REAL|IMAG).
	 * For fast disk i/o we read complex data in serial.  I.e., if we ask for GMT_CMPLX_REAL
	 * then the array will contain RRRRR....________, where ______ is unused space for the
	 * imaginary components.  Likewise, if we requested GMT_CMPLX_IMAG then the array will
	 * be returned as _______...IIIIIII....
	 * Operations like FFTs typically required the data to be interleaved, i.e., in the
	 * form RIRIRIRI.... Then, when the FFT work is done and we wish to write out the
	 * result we will need to demultiplex the array back to its serial RRRRR....IIIII
	 * format before writing takes place.
	 * gmt_grd_mux_demux performs either multiplex or demultiplex, depending on desired_mode.
	 * If grid is not complex then we just return doing nothing.
	 * Note: At this point the grid is mx * my and we visit all the nodes, including the pads.
	 * hence we use header->mx/my and gmt_M_ij below.
	 */
	uint64_t row, col, col_1, col_2, left_node_1, left_node_2, offset, ij, ij2;
	gmt_grdfloat *array = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (! (desired_mode == GMT_GRID_IS_INTERLEAVED || desired_mode == GMT_GRID_IS_SERIAL)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "gmt_grd_mux_demux called with inappropriate mode - skipped.\n");
		return;
	}
	if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == 0) return;	/* Nuthin' to do */
	if (HH->arrangement == desired_mode) return;				/* Already has the right layout */

	/* In most cases we will actually have RRRRR...______ or _____...IIIII..
	 * which means half the array is empty and it is easy to shuffle. However,
	 * in the case with actual RRRR...IIII there is no simple way to do this inplace;
	 * see http://stackoverflow.com/questions/1777901/array-interleaving-problem */

	if (desired_mode == GMT_GRID_IS_INTERLEAVED) {	/* Must multiplex the grid */
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == GMT_GRID_IS_COMPLEX_MASK) {
			/* Transform from RRRRR...IIIII to RIRIRIRIRI... */
			/* Implement properly later; for now waste memory by duplicating [memory is cheap and plentiful] */
			/* One advantage is that the padding is all zero by virtue of the allocation */
			array = gmt_M_memory_aligned (GMT, NULL, header->size, gmt_grdfloat);
			offset = header->size / 2;	/* Position of 1st row in imag portion of RRRR...IIII... */
			for (row = 0; row < header->my; row++) {	/* Going from first to last row */
				for (col = 0; col < header->mx; col++) {
					ij = gmt_M_ij (header, row, col);	/* Position of an 'R' in the RRRRR portion */
					ij2 = 2 * ij;
					array[ij2++] = data[ij];
					array[ij2] = data[ij+offset];
				}
			}
			gmt_M_memcpy (data, array, header->size, gmt_grdfloat);	/* Overwrite serial array with interleaved array */
			gmt_M_free (GMT, array);
		}
		else if (header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) {
			/* Here we have RRRRRR..._________ and want R_R_R_R_... */
			gmtlib_grd_real_interleave (GMT, header, data);
		}
		else {
			/* Here we have _____...IIIII and want _I_I_I_I */
			offset = header->size / 2;	/* Position of 1st row in imag portion of ____...IIII... */
			for (row = 0; row < header->my; row++) {	/* Going from first to last row */
				left_node_1 = gmt_M_ij (header, row, 0);		/* Start of row in _____IIII layout not counting ____*/
				left_node_2 = 2 * left_node_1;			/* Start of same row in _I_I_I... layout */
				left_node_1 += offset;				/* Move past length of all ____... */
				for (col_1 = 0, col_2 = 1; col_1 < header->mx; col_1++, col_2 += 2) {
					data[left_node_2+col_2] = data[left_node_1+col_1];
					data[left_node_1+col_1] = 0.0f;	/* Set the Real component to zero */
				}
			}
		}
	}
	else if (desired_mode == GMT_GRID_IS_SERIAL) {	/* Must demultiplex the grid */
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == GMT_GRID_IS_COMPLEX_MASK) {
			/* Transform from RIRIRIRIRI... to RRRRR...IIIII  */
			/* Implement properly later; for now waste memory by duplicating [memory is cheap and plentiful] */
			/* One advantage is that the padding is all zero by virtue of the allocation */
			array = gmt_M_memory_aligned (GMT, NULL, header->size, gmt_grdfloat);
			offset = header->size / 2;	/* Position of 1st row in imag portion of RRRR...IIII... */
			for (row = 0; row < header->my; row++) {	/* Going from first to last row */
				for (col = 0; col < header->mx; col++) {
					ij = gmt_M_ij (header, row, col);	/* Position of an 'R' in the RRRRR portion */
					ij2 = 2 * ij;
					array[ij] = data[ij2++];
					array[ij+offset] = data[ij2];
				}
			}
			gmt_M_memcpy (data, array, header->size, gmt_grdfloat);	/* Overwrite interleaved array with serial array */
			gmt_M_free (GMT, array);
		}
		else if (header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) {
			/* Here we have R_R_R_R_... and want RRRRRR..._______  */
			for (row = 0; row < header->my; row++) {	/* Doing from first to last row */
				left_node_1 = gmt_M_ij (header, row, 0);	/* Start of row in RRRRR... */
				left_node_2 = 2 * left_node_1;		/* Start of same row in R_R_R_R... layout */
				for (col_1 = col_2 = 0; col_1 < header->mx; col_1++, col_2 += 2) {
					data[left_node_1+col_1] = data[left_node_2+col_2];
				}
			}
			offset = header->size / 2;			/* Position of 1st _ in RRRR...____ */
			gmt_M_memset (&data[offset], offset, gmt_grdfloat);	/* Wipe _____ portion clean */
		}
		else {	/* Here we have _I_I_I_I and want _____...IIIII */
			offset = header->size / 2;	/* Position of 1st row in imag portion of ____...IIII... */
			for (row = header->my; row > 0; row--) {	/* Going from last to first row */
				left_node_1 = gmt_M_ij (header, row, 0);	/* Start of row in _____IIII layout not counting ____*/
				left_node_2 = 2 * left_node_1;		/* Start of same row in _I_I_I... layout */
				left_node_1 += offset;			/* Move past length of all ____... */
				for (col = header->mx, col_1 = col - 1, col_2 = 2*col - 1; col > 0; col--, col_1--, col_2 -= 2) { /* Go from right to left */
					data[left_node_1+col_1] = data[left_node_2+col_2];
				}
			}
			gmt_M_memset (data, offset, gmt_grdfloat);	/* Wipe leading _____ portion clean */
		}
	}
	HH->arrangement = desired_mode;
}

int gmt_grd_get_format (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, bool magic) {
	/* This functions does a couple of things:
	 * 1. It tries to determine what kind of grid file this is. If a file is openeed for
	 *    reading we see if (a) a particular format has been specified with
	 *    the =<code> suffix, or (b) we are able to guess the format based on known
	 *    characteristics of various formats, or (c) assume the default grid format.
	 *    If a file is opened for writing, only option (a) and (c) apply.
	 *    If we cannot obtain the format we return an error.
	 * 2. We strip the suffix off. The relevant info is stored in the header struct.
	 * 3. In case of netCDF grids, the optional ?<varname> is stripped off as well.
	 *    The info is stored in header->varname.
	 * 4. If a file is open for reading, we set header->name to the full path of the file
	 *    by searching in current dir and the various GMT_*DIR paths.
	 */

	size_t i = 0, j;
	int val;
	unsigned int direction = (magic) ? GMT_IN : GMT_OUT, pos = 0;
	char tmp[GMT_LEN512] = {""};	/* But it's copied at most 256 chars into header->name so 256 should do */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (file[0] == '@') pos = 1;	/* At this point we will already have downloaded any remote file so skip the @ */
	grdio_grd_parse_xy_units (GMT, header, &file[pos], direction);	/* Parse and strip xy scaling via +u<unit> modifier */

	grdio_expand_filename (GMT, &file[pos], HH->name);	/* May append a suffix to header->name */

	/* Must reset scale and invalid value because sometimes headers from input grids are
	 * 'recycled' and used for output grids that may have a different type and z-range: */
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;
	header->nan_value      = (gmt_grdfloat)NAN;

	i = strcspn (HH->name, "=");	/* get number of chars until first '=' or '\0' */
	j = strcspn (HH->name, "+");	/* get number of chars until first '+' or '\0' */

	if (HH->name[i] == '\0' && (HH->name[j] && strchr ("ons", HH->name[j+1]))) {	/* No grid type but gave valid modifiers */
		/* parse grid format string: */
		if ((val = grdio_parse_grd_format_scale (GMT, header, &HH->name[j])) != GMT_NOERROR)
			return val;
		HH->name[j] = '\0';	/* Chop off since we did got the scalings */
	}
	if (HH->name[i]) {	/* Reading or writing when =suffix is present: get format type, scale, offset and missing value */
		i++;
		/* parse grid format string: */
		if ((val = grdio_parse_grd_format_scale (GMT, header, &HH->name[i])) != GMT_NOERROR)
			return val;
		if (header->type == GMT_GRID_IS_GD && HH->name[i+2] && HH->name[i+2] == '?') {	/* A SUBDATASET request for GDAL */
			char *pch = strstr(&HH->name[i+3], "::");
			if (pch) {		/* The file name was omitted within the SUBDATASET. Must put it there for GDAL */
				strncpy (tmp, &HH->name[i+3], pch - &HH->name[i+3] + 1);
				strcat (tmp, "\"");	strncat(tmp, HH->name, i-1);	strcat(tmp, "\"");
				if (strlen(&pch[1]) < (GMT_LEN512-strlen(tmp)-1)) strncat (tmp, &pch[1], GMT_LEN512-1);
				strncpy (HH->name, tmp, GMT_LEN256-1);
			}
			else
				memmove (HH->name, &HH->name[i+3], strlen(&HH->name[i+3])+1);
			magic = 0;	/* We don't want it to try to prepend any path */
		} /* if (header->type == GMT_GRID_IS_GD && HH->name[i+2] && HH->name[i+2] == '?') */
		else if (header->type == GMT_GRID_IS_GD && HH->name[i+2] && HH->name[i+2] == '+' && HH->name[i+3] == 'b') { /* A Band request for GDAL */
			HH->pocket = strdup(&HH->name[i+4]);
			HH->name[i-1] = '\0';
		}
		else if (header->type == GMT_GRID_IS_GD && HH->name[i+2] && strchr(&HH->name[i+2], ':')) {
			char *pch;
			size_t nc_limit = 2147483647U;	/* 2^31 - 1 is the max length of a 1-D array in netCDF */
			pch = strchr(&HH->name[i+2], ':');
			HH->pocket = strdup(++pch);
			HH->name[i-1] = '\0';			/* Done, rip the driver/outtype info from file name */
			if (!strncmp (HH->pocket, "GMT", 3U) && header->nm > nc_limit) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Your grid contains more than 2^31 - 1 nodes (%" PRIu64 ") and cannot be stored with the deprecated GMT netCDF format via GDAL.\n", header->nm);
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Please choose another grid format such as the default netCDF 4 COARDS-compliant grid format.\n");
				return (GMT_GRDIO_BAD_DIM);	/* Grid is too large */
			}
		}
		else {
			j = (i == 1) ? i : i - 1;
			HH->name[j] = 0;
		}
		sscanf (HH->name, "%[^?]?%s", tmp, HH->varname);    /* Strip off variable name */
		if (magic) {	/* Reading: possibly prepend a path from GMT_[GRID|DATA|IMG]DIR */
			if (header->type != GMT_GRID_IS_GD || !gmtlib_check_url_name(tmp))	/* Do not try path stuff with Web files (accessed via GDAL) */
				if (!gmt_getdatapath (GMT, tmp, HH->name, R_OK))
					return (GMT_GRDIO_FILE_NOT_FOUND);
		}
		else		/* Writing: store truncated pathname */
			strncpy (HH->name, tmp, GMT_LEN256-1);
	} /* if (header->name[i]) */
	else if (magic) {	/* Reading: determine file format automatically based on grid content */
		int choice = 0;
		sscanf (HH->name, "%[^?]?%s", tmp, HH->varname);    /* Strip off variable name */
#ifdef HAVE_GDAL
		/* Check if file is an URL */
		if (gmtlib_check_url_name(HH->name)) {
			/* Then check for GDAL grid */
			if (gmt_is_gdal_grid (GMT, header) == GMT_NOERROR)
				return (GMT_NOERROR);
		}
#endif
		if (!gmt_getdatapath (GMT, tmp, HH->name, R_OK))
			return (GMT_GRDIO_FILE_NOT_FOUND);	/* Possibly prepended a path from GMT_[GRID|DATA|IMG]DIR */
		/* First check if we have a netCDF grid. This MUST be first, because ?var needs to be stripped off. */
		if ((val = gmt_is_nc_grid (GMT, header)) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Continue only when file was not a pipe or when nc_open didn't like the file or when the grid was COARDS-compliant. */
		if (val != GMT_GRDIO_NC_NO_PIPE && val != GMT_GRDIO_OPEN_FAILED && val != GMT_GRDIO_NC_NOT_COARDS)
			return (val);
		/* Then check for native binary GMT grid */
		if ((choice = gmt_is_native_grid (GMT, header)) == GMT_NOERROR)
			return (GMT_NOERROR);
		else if (choice == GMT_GRDIO_NONUNIQUE_FORMAT)
			return (GMT_GRDIO_NONUNIQUE_FORMAT);
		/* Next check for Sun raster grid */
		if (gmt_is_ras_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for Golden Software surfer grid */
		if (gmt_is_srf_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for NGDC GRD98 grid */
		if (gmt_is_mgg2_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for AGC grid */
		if (gmt_is_agc_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for ESRI grid */
		if (gmt_is_esri_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
#ifdef HAVE_GDAL
		/* Then check for GDAL grid */
		if (gmt_is_gdal_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
#endif
		return (GMT_GRDIO_UNKNOWN_FORMAT);	/* No supported format found */
	}
	else {			/* Writing: get format type, scale, offset and missing value from GMT->current.setting.io_gridfile_format */
		if (sscanf (HH->name, "%[^?]?%s", tmp, HH->varname) > 1)
			strncpy (HH->name, tmp, GMT_LEN256-1);    /* Strip off variable name */
		/* parse grid format string: */
		if ((val = grdio_parse_grd_format_scale (GMT, header, GMT->current.setting.io_gridfile_format)) != GMT_NOERROR)
			return val;
	}
	if (header->type == GMT_GRID_IS_AF)
		header->nan_value = 0.0f; /* NaN value for AGC format */
	return (GMT_NOERROR);
}

void gmtlib_grd_set_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Set unit strings for grid coordinates x, y and z based on
	   output data types for columns 0, 1, and 2.
	*/
	unsigned int i;
	char *string[3] = {NULL, NULL, NULL}, unit[GMT_GRID_UNIT_LEN80] = {""};
	char date[GMT_LEN16] = {""}, clock[GMT_LEN16] = {""};
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	/* Copy pointers to unit strings */
	string[0] = header->x_units;
	string[1] = header->y_units;
	string[2] = header->z_units;

	/* Use input data type as backup fr output data type */
	for (i = 0; i < 3; i++)
		if (gmt_M_type (GMT, GMT_OUT, i) == GMT_IS_UNKNOWN) GMT->current.io.col_type[GMT_OUT][i] = GMT->current.io.col_type[GMT_IN][i];

	/* Catch some anomalies */
	if (gmt_M_type (GMT, GMT_OUT, GMT_X) == GMT_IS_LAT && gmt_M_type (GMT, GMT_OUT, GMT_Y) == GMT_IS_LAT) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Output type for X-coordinate of grid %s is LAT. Replaced by LON.\n", HH->name);
		gmt_set_column (GMT, GMT_OUT, GMT_X, GMT_IS_LON);

	}
	if (gmt_M_type (GMT, GMT_OUT, GMT_Y) == GMT_IS_LON && gmt_M_type (GMT, GMT_OUT, GMT_X) == GMT_IS_LON) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Output type for Y-coordinate of grid %s is LON. Replaced by LAT.\n", HH->name);
		gmt_set_column (GMT, GMT_OUT, GMT_Y, GMT_IS_LAT);
	}

	/* Set unit strings one by one based on output type */
	for (i = 0; i < 3; i++) {
		switch (gmt_M_type (GMT, GMT_OUT, i)) {
			case GMT_IS_LON:
				strcpy (string[i], "longitude [degrees_east]"); break;
			case GMT_IS_LAT:
				strcpy (string[i], "latitude [degrees_north]"); break;
			case GMT_IS_ABSTIME:
			case GMT_IS_RELTIME:
			case GMT_IS_RATIME:
				/* Determine time unit */
				switch (GMT->current.setting.time_system.unit) {
					case 'y':
						strcpy (unit, "years"); break;
					case 'o':
						strcpy (unit, "months"); break;
					case 'd':
						strcpy (unit, "days"); break;
					case 'h':
						strcpy (unit, "hours"); break;
					case 'm':
						strcpy (unit, "minutes"); break;
					default:
						strcpy (unit, "seconds"); break;
				}
				gmt_format_calendar (GMT, date, clock, &GMT->current.io.date_output, &GMT->current.io.clock_output, false, 1, 0.0);
				snprintf (string[i], GMT_GRID_UNIT_LEN80, "time [%s since %s %s]", unit, date, clock);
				/* Warning for non-double grids */
				if (i == 2 && GMT->session.grdformat[header->type][1] != 'd')
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "Use double precision output grid to avoid loss of significance of time coordinate.\n");
				break;
		}
	}
}

bool gmt_grd_pad_status (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, unsigned int *pad) {
	/* Determines if this grid has padding at all (pad = NULL) OR
	 * if pad is given, determines if the pads are different.
	 * Return codes are:
	 * 1) If pad == NULL:
	 *    false: Grid has zero padding.
	 *    true:  Grid has non-zero padding.
	 * 2) If pad contains the desired pad:
	 *    true:  Grid padding matches pad exactly.
	 *    false: Grid padding failed to match pad exactly.
	 */
	unsigned int side;
	gmt_M_unused(GMT);

	if (pad) {	/* Determine if the grid's pad differ from given pad (false) or not (true) */
		for (side = 0; side < 4; side++) if (header->pad[side] != pad[side]) return (false);	/* Pads differ */
		return (true);	/* Pads match */
	}
	else {	/* We just want to determine if the grid has padding already (true) or not (false) */
		for (side = 0; side < 4; side++) if (header->pad[side]) return (true);	/* Grid has a pad */
		return (false);	/* Grid has no pad */
	}
}

int gmtlib_get_grdtype (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_GRID_HEADER *h) {
	/* Determine if input or output grid is Cartesian or geographic, and if so if longitude range is <360, ==360, or >360 */
	char *dir[2] = {"input", "output"};
	if (gmt_M_x_is_lon (GMT, direction)) {	/* Data set is geographic with x = longitudes */
		if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Geographic %s grid, longitudes span exactly 360\n", dir[direction]);
			/* If w/e is 360 and gridline reg then we have a repeat entry for 360.  For pixel there are never repeat pixels */
			return ((h->registration == GMT_GRID_NODE_REG) ? GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT : GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT);
		}
		else if (fabs (h->n_columns * h->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Geographic %s grid, longitude cells span exactly 360\n", dir[direction]);
			/* If n*xinc = 360 and previous test failed then we do not have a repeat node */
			return (GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT);
		}
		else if ((h->wesn[XHI] - h->wesn[XLO]) > 360.0) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Geographic %s grid, longitudes span more than 360\n", dir[direction]);
			return (GMT_GRID_GEOGRAPHIC_MORE360);
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Geographic %s grid, longitudes span less than 360\n", dir[direction]);
			return (GMT_GRID_GEOGRAPHIC_LESS360);
		}
	}
	else if (h->wesn[YLO] >= -90.0 && h->wesn[YHI] <= 90.0) {	/* Here we simply advice the user if grid looks like geographic but is not set as such */
		if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Cartesian %s grid, yet x spans exactly 360 and -90 <= y <= 90.\n", dir[direction]);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "     To make sure the grid is recognized as geographical and global, use the -fg option\n");
			return (GMT_GRID_CARTESIAN);
		}
		else if (fabs (h->n_columns * h->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Cartesian %s grid, yet x cells span exactly 360 and -90 <= y <= 90.\n", dir[direction]);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "     To make sure the grid is recognized as geographical and global, use the -fg option\n");
			return (GMT_GRID_CARTESIAN);
		}
	}
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Cartesian %s grid\n", dir[direction]);
	return (GMT_GRID_CARTESIAN);
}

GMT_LOCAL void doctor_geo_increments (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Check for sloppy arc min/sec increments due to divisions by 60 or 3600 */
	double round_inc, scale, inc, slop;
	unsigned int side;
	static char *type[2] = {"longitude", "latitude"};

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Call doctor_geo_increments on a geographic grid\n");
	for (side = GMT_X; side <= GMT_Y; side++) {	/* Check both increments */
		scale = (header->inc[side] < GMT_MIN2DEG) ? 3600.0 : 60.0;	/* Check for clean multiples of minutes or seconds */
		inc = header->inc[side] * scale;
		round_inc = rint (inc);
		slop = fabs (inc - round_inc);
		if (slop > 0 && slop < GMT_CONV4_LIMIT) {
			inc = header->inc[side];
			header->inc[side] = round_inc / scale;
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Round-off patrol changed geographic grid increment for %s from %.18g to %.18g\n",
				type[side], inc, header->inc[side]);
		}
	}
}

GMT_LOCAL void grdio_round_off_patrol (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* This function is called after the read info functions return.  We use it to make
	 * sure there are no tiny inconsistencies between grid increments and limits, and
	 * also check that geographic latitudes are within bounds. For geographic data we
	 * also examine if the increment * 60 or 3600 is very close (< 1e-4) to an integer,
	 * in which case we reset it to the exact reciprocal value. */
	unsigned int k;
	double norm_v, round_v, d, slop;
	static char *type[4] = {"xmin", "xmax", "ymin", "ymax"};

	if (gmt_M_is_geographic (GMT, GMT_IN) && (header->wesn[XHI] - header->wesn[XLO] - header->inc[GMT_X]) <= 360.0) {	/* Correct any slop in geographic increments */
		doctor_geo_increments (GMT, header);
		if ((header->wesn[YLO]+90.0) < (-GMT_CONV4_LIMIT*header->inc[GMT_Y]))
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Round-off patrol found south latitude outside valid range (%.16g)!\n", header->wesn[YLO]);
		if ((header->wesn[YHI]-90.0) > (GMT_CONV4_LIMIT*header->inc[GMT_Y]))
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Round-off patrol found north latitude outside valid range (%.16g)!\n", header->wesn[YHI]);
	}

	/* If boundaries are close to multiple of inc/2 fix them */
	for (k = XLO; k <= YHI; k++) {	/* Check all limits for closeness to 0.5*increments */
		d = 0.5 * ((k < YLO) ? header->inc[GMT_X] : header->inc[GMT_Y]);
		norm_v = header->wesn[k] / d;
		round_v = rint (norm_v);
		slop = fabs (norm_v - round_v);
		if (slop > GMT_CONV12_LIMIT && slop < GMT_CONV4_LIMIT) {	/* Ignore super tiny-slop and larger mismatches */
			header->wesn[k] = round_v * d;
			norm_v = header->wesn[k];
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Round-off patrol changed grid limit for %s from %.16g to %.16g\n",
				type[k], norm_v, header->wesn[k]);
		}
	}
}

int gmtlib_read_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header) {
	/* file:	File name
	 * header:	grid structure header
	 * Note: The header reflects what is actually in the file, and all the dimensions
	 * reflect the number of rows, cols, size, pads etc.  However, if gmtlib_read_grd is
	 * called requesting a subset then these will be reset accordingly.
	 */

	int err;	/* Implied by gmt_M_err_trap */
	unsigned int n_columns, n_rows;
	double scale, offset;
	gmt_grdfloat invalid;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;

	gmt_M_err_trap (gmt_grd_get_format (GMT, file, header, true));	/* Get format and also parse any +s<scl> +o<offset> +n<nan> modifiers */
	HH = gmt_get_H_hidden (header);

	/* remember scale, offset, and invalid: */
	scale = header->z_scale_factor;
	offset = header->z_add_offset;
	invalid = header->nan_value;

	gmt_M_err_trap ((*GMT->session.readinfo[header->type]) (GMT, header));
	GMT_Set_Index (GMT->parent, header, GMT_GRID_LAYOUT);

	grdio_grd_xy_scale (GMT, header, GMT_IN);	/* Possibly scale wesn,inc */

	/* restore non-default scale, offset, and invalid: */
	if (HH->z_scale_given)	/* User used +s<scl> */
		header->z_scale_factor = scale;
	if (HH->z_offset_given)	/* User used +s<off> */
		header->z_add_offset = offset;
	if (isfinite(invalid))	/* Means user used +n<invalid> */
		header->nan_value = invalid;

	gmtlib_grd_get_units (GMT, header);
	grdio_round_off_patrol (GMT, header);	/* Ensure limit/inc consistency */
	//gmtlib_clean_global_headers (GMT, header);

	HH->grdtype = gmtlib_get_grdtype (GMT, GMT_IN, header);

	gmt_M_err_pass (GMT, gmt_grd_RI_verify (GMT, header, 0), file);
	n_columns = header->n_columns;	n_rows = header->n_rows;	/* Save copy */
	gmt_set_grddim (GMT, header);	/* Set all integer dimensions and xy_off */

	/* Sanity check for grid that may have been created oddly.  Inspired by
	 * Geomapapp output where -R was set to outside of pixel boundaries instead
	 * of standard -R settings, yet with node_offset = gridline... */

	if (abs((int)(header->n_columns - n_columns)) == 1 && abs((int)(header->n_rows - n_rows)) == 1) {
       		header->n_columns = n_columns;    header->n_rows = n_rows;
 		if (header->registration == GMT_GRID_PIXEL_REG) {
 			header->registration = GMT_GRID_NODE_REG;
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Grid has wrong registration type. Switching from pixel to gridline registration\n");
		}
		else {
 			header->registration = GMT_GRID_PIXEL_REG;
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Grid has wrong registration type. Switching from gridline to pixel registration\n");
		}
	}

	/* unpack z-range: */
	header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
	header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;

	return (GMT_NOERROR);
}

int gmtlib_write_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header) {
	/* file:	File name
	 * header:	grid structure header
	 */

	int err;	/* Implied by gmt_M_err_trap */

	gmt_M_err_trap (gmt_grd_get_format (GMT, file, header, false));

	grdio_grd_xy_scale (GMT, header, GMT_OUT);	/* Possibly scale wesn,inc */
	/* pack z-range: */
	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	return ((*GMT->session.writeinfo[header->type]) (GMT, header));
}

int gmt_update_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header) {
	/* file:	- IGNORED -
	 * header:	grid structure header
	 */

	/* pack z-range: */
	gmt_M_unused(file);
	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	return ((*GMT->session.updateinfo[header->type]) (GMT, header));
}

int gmtlib_read_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double *wesn, unsigned int *pad, int complex_mode) {
	/* file:	- IGNORED -
	 * header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to extract  [Use entire file if NULL or contains 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	bool expand;		/* true or false */
	int err;		/* Implied by gmt_M_err_trap */
	struct GRD_PAD P;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	complex_mode &= GMT_GRID_IS_COMPLEX_MASK;	/* Remove any non-complex flags */
	/* If we are reading a 2nd grid (e.g., real, then imag) we must update info about the file since it will be a different file */
	if (header->complex_mode && (header->complex_mode & complex_mode) == 0) gmt_M_err_trap (gmt_grd_get_format (GMT, file, header, true));

	expand = grdio_padspace (GMT, header, wesn, pad, &P);	/* true if we can extend the region by the pad-size to obtain real data for BC */

	grdio_grd_layout (GMT, header, grid, complex_mode & GMT_GRID_IS_COMPLEX_MASK, GMT_IN);	/* Deal with complex layout */

	gmt_M_err_trap ((*GMT->session.readgrd[header->type]) (GMT, header, grid, P.wesn, P.pad, complex_mode));

	if (header->z_min == DBL_MAX && header->z_max == -DBL_MAX) /* No valid data values in the grid */
		header->z_min = header->z_max = NAN;
	if (expand) /* Must undo the region extension and reset n_columns, n_rows using original pad  */
		gmt_M_memcpy (header->wesn, wesn, 4, double);
	gmt_M_grd_setpad (GMT, header, pad);	/* Copy the pad to the header */
	gmt_set_grddim (GMT, header);		/* Update all dimensions */
	HH->grdtype = gmtlib_get_grdtype (GMT, GMT_IN, header);	/* Since may change if a subset */
	if (expand) gmt_grd_zminmax (GMT, header, grid);	/* Reset min/max since current extrema includes the padded region */
	grdio_pack_grid (GMT, header, grid, k_grd_unpack); /* revert scale and offset */
	gmt_BC_init (GMT, header);	/* Initialize grid interpolation and boundary condition parameters */

	return (GMT_NOERROR);
}

int gmtlib_write_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double *wesn, unsigned int *pad, int complex_mode) {
	/* file:	File name
	 * header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to write out  [Use entire file if NULL or contains 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	int err;	/* Implied by gmt_M_err_trap */

	gmt_M_err_trap (gmt_grd_get_format (GMT, file, header, false));
	grdio_pack_grid (GMT, header, grid, k_grd_pack); /* scale and offset */
	grdio_grd_xy_scale (GMT, header, GMT_OUT);	/* Possibly scale wesn,inc */

	grdio_grd_layout (GMT, header, grid, complex_mode, GMT_OUT);	/* Deal with complex layout */
	grdio_grd_check_consistency (GMT, header, grid);			/* Fix east repeating columns and polar values */
	err = (*GMT->session.writegrd[header->type]) (GMT, header, grid, wesn, pad, complex_mode);
	if (GMT->parent->leave_grid_scaled == 0) grdio_pack_grid (GMT, header, grid, k_grd_unpack); /* revert scale and offset to leave grid as it was before writing unless session originated from gm*/
	return (err);
}

size_t gmt_grd_data_size (struct GMT_CTRL *GMT, unsigned int format, gmt_grdfloat *nan_value) {
	/* Determine size of data type and set NaN value, if not yet done so (integers only) */

	switch (GMT->session.grdformat[format][1]) {
		case 'b':
			if (isnan (*nan_value)) *nan_value = CHAR_MIN;
			return (sizeof (char));
			break;
		case 's':
			if (isnan (*nan_value)) *nan_value = SHRT_MIN;
			return (sizeof (int16_t));
			break;
		case 'i':
			if (isnan (*nan_value)) *nan_value = INT_MIN;
			/* Intentionally fall through */
		case 'm':
			return (sizeof (int32_t));
			break;
		case 'f':
			return (sizeof (float));
			break;
		case 'd':
			return (sizeof (double));
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unknown grid data type: %c\n", GMT->session.grdformat[format][1]);
			return (GMT_GRDIO_UNKNOWN_TYPE);
	}
}

void gmt_grd_set_ij_inc (struct GMT_CTRL *GMT, unsigned int n_columns, int *ij_inc) {
	/* Set increments to the 4 nodes with ij as lower-left node, from a node at (i,j).
	 * n_columns may be header->n_columns or header->mx depending on pad */
	int s_nx = n_columns;	/* A signed version */
	gmt_M_unused(GMT);
	ij_inc[0] = 0;		/* No offset relative to itself */
	ij_inc[1] = 1;		/* The node one column to the right relative to ij */
	ij_inc[2] = 1 - s_nx;	/* The node one column to the right and one row up relative to ij */
	ij_inc[3] = -s_nx;	/* The node one row up relative to ij */
}

int gmt_grd_format_decoder (struct GMT_CTRL *GMT, const char *code, unsigned int *type_id) {
	/* Returns the integer grid format ID that goes with the specified 2-character code */
	if (isdigit ((int)code[0])) {
		/* File format number given, look for old code */
		unsigned id_candidate = (unsigned) abs (atoi (code));
		if (id_candidate > 0 && id_candidate < GMT_N_GRD_FORMATS) {
			*type_id = id_candidate;
			return GMT_NOERROR;
		}
	}
	else {
		/* Character code given */
		unsigned i;
		for (i = 1; i < GMT_N_GRD_FORMATS; i++) {
			if (strncmp (GMT->session.grdformat[i], code, 2) == 0) {
				*type_id = i;
				return GMT_NOERROR;
			}
		}
	}

	return GMT_GRDIO_UNKNOWN_ID;
}

int gmt_grd_RI_verify (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int mode) {
	/* mode - 0 means we are checking an existing grid, mode = 1 means we test a new -R -I combination */
	/* gmt_grd_RI_verify -- routine to check grd R and I compatibility
	 *
	 * Author:	W H F Smith
	 * Date:	20 April 1998
	 */

	unsigned int error = 0;

	if (!strcmp (GMT->init.module_name, "grdedit")) return (GMT_NOERROR);	/* Separate handling in grdedit to allow grdedit -A */

	switch (gmt_minmaxinc_verify (GMT, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], GMT_CONV4_LIMIT)) {
		case 3:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid x increment <= 0.0\n");
			error++;
			break;
		case 2:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid x range <= 0.0\n");
			if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_ERROR,
			                         "Make sure west < east for geographic coordinates\n");
			error++;
			break;
		case 1:
			GMT_Report (GMT->parent, GMT_MSG_ERROR,
			            "(x_max-x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			error++;
		default:
			/* Everything is OK */
			break;
	}

	switch (gmt_minmaxinc_verify (GMT, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], GMT_CONV4_LIMIT)) {
		case 3:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid y increment <= 0.0\n");
			error++;
			break;
		case 2:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid y range <= 0.0\n");
			error++;
			break;
		case 1:
			GMT_Report (GMT->parent, GMT_MSG_ERROR,
			            "(y_max-y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			error++;
		default:
			/* Everything is OK */
			break;
	}
	if (error) return ((mode == 0) ? GMT_GRDIO_RI_OLDBAD : GMT_GRDIO_RI_NEWBAD);

	/* Final polish for geo grids that are global to ensure clean -R settings for the cases -Rg and -Rd */

	if (gmt_M_x_is_lon (GMT, GMT_IN)) {
		if (fabs (h->wesn[XLO]) < GMT_CONV12_LIMIT) h->wesn[XLO] = 0.0;
		else if (fabs (180.0+h->wesn[XLO]) < GMT_CONV12_LIMIT) h->wesn[XLO] = -180.0;
		else if (fabs (h->wesn[XLO]-180.0) < GMT_CONV12_LIMIT) h->wesn[XLO] = +180.0;
		else if (fabs (h->wesn[XLO]-360.0) < GMT_CONV12_LIMIT) h->wesn[XLO] = +360.0;
		if (fabs (h->wesn[XHI]) < GMT_CONV12_LIMIT) h->wesn[XHI] = 0.0;
		else if (fabs (180.0+h->wesn[XHI]) < GMT_CONV12_LIMIT) h->wesn[XHI] = -180.0;
		else if (fabs (h->wesn[XHI]-180.0) < GMT_CONV12_LIMIT) h->wesn[XHI] = +180.0;
		else if (fabs (h->wesn[XHI]-360.0) < GMT_CONV12_LIMIT) h->wesn[XHI] = +360.0;
	}
	if (gmt_M_y_is_lat (GMT, GMT_IN)) {
		if (fabs (90.0+h->wesn[YLO]) < GMT_CONV12_LIMIT) h->wesn[YLO] = -90.0;
		if (fabs (h->wesn[YLO]-90.0) < GMT_CONV12_LIMIT) h->wesn[YLO] = +90.0;
	}
	return (GMT_NOERROR);
}

int gmt_grd_prep_io (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double wesn[], unsigned int *width, unsigned int *height, int *first_col, int *last_col, int *first_row, int *last_row, unsigned int **index) {
	/* Determines which rows and columns to extract to extract from a grid, based on w,e,s,n.
	 * This routine first rounds the w,e,s,n boundaries to the nearest gridlines or pixels,
	 * then determines the first and last columns and rows, and the width and height of the subset (in cells).
	 * The routine also returns and array of the x-indices in the source grid to be used in the target (subset) grid.
	 * All integers represented positive definite items hence unsigned variables.
	 */

	bool geo = false;
	unsigned int one_or_zero, col, *actual_col = NULL;	/* Column numbers */
	double small = 0.1, half_or_zero, x;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "region: %g %g, grid: %g %g\n", wesn[XLO], wesn[XHI], header->wesn[XLO], header->wesn[XHI]);

	half_or_zero = (header->registration == GMT_GRID_PIXEL_REG) ? 0.5 : 0.0;

	if (!gmt_M_is_subset (GMT, header, wesn)) {	/* Get entire file */
		*width  = header->n_columns;
		*height = header->n_rows;
		*first_col = *first_row = 0;
		*last_col  = header->n_columns - 1;
		*last_row  = header->n_rows - 1;
		gmt_M_memcpy (wesn, header->wesn, 4, double);
	}
	else {				/* Must deal with a subregion */
		if (gmt_M_x_is_lon (GMT, GMT_IN))
			geo = true;	/* Geographic data for sure */
		else if (wesn[XLO] < header->wesn[XLO] || wesn[XHI] > header->wesn[XHI])
			geo = true;	/* Probably dealing with periodic grid */

		x = fabs (header->wesn[YLO] - wesn[YLO]);	/* if |x| < GMT_CONV4_LIMIT * header->inc[GMT_Y] we set wesn to the grid limit */
		if (x > 0.0 && x < GMT_CONV4_LIMIT * header->inc[GMT_Y]) wesn[YLO] = header->wesn[YLO];	/* Avoid snafu */
		x = fabs (header->wesn[YHI] - wesn[YHI]);	/* if |x| < GMT_CONV4_LIMIT * header->inc[GMT_Y] we set wesn to the grid limit */
		if (x > 0.0 && x < GMT_CONV4_LIMIT * header->inc[GMT_Y]) wesn[YHI] = header->wesn[YHI];	/* Avoid snafu */

		if (wesn[YLO] < header->wesn[YLO] || wesn[YHI] > header->wesn[YHI]) return (GMT_GRDIO_DOMAIN_VIOLATION);	/* Calling program goofed... */

		one_or_zero = (header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;

		/* Make sure w,e,s,n are proper multiples of x_inc,y_inc away from x_min,y_min */

		gmt_M_err_pass (GMT, gmt_adjust_loose_wesn (GMT, wesn, header), HH->name);

		/* Get dimension of subregion */

		*width  = urint ((wesn[XHI] - wesn[XLO]) * HH->r_inc[GMT_X]) + one_or_zero;
		*height = urint ((wesn[YHI] - wesn[YLO]) * HH->r_inc[GMT_Y]) + one_or_zero;

		/* Get first and last row and column numbers */

		*first_col = irint (floor ((wesn[XLO] - header->wesn[XLO]) * HH->r_inc[GMT_X] + small));
		*last_col  = irint (ceil  ((wesn[XHI] - header->wesn[XLO]) * HH->r_inc[GMT_X] - small)) - 1 + one_or_zero;
		*first_row = irint (floor ((header->wesn[YHI] - wesn[YHI]) * HH->r_inc[GMT_Y] + small));
		*last_row  = irint (ceil  ((header->wesn[YHI] - wesn[YLO]) * HH->r_inc[GMT_Y] - small)) - 1 + one_or_zero;
	}

	actual_col = gmt_M_memory (GMT, NULL, *width, unsigned int);
	if (geo) {
		small = 0.1 * header->inc[GMT_X];
		for (col = 0; col < (*width); col++) {
			x = gmt_M_col_to_x (GMT, col, wesn[XLO], wesn[XHI], header->inc[GMT_X], half_or_zero, *width);
			if (header->wesn[XLO] - x > small)
				x += 360.0;
			else if (x - header->wesn[XHI] > small)
				x -= 360.0;
			actual_col[col] = (unsigned int)gmt_M_grd_x_to_col (GMT, x, header);
		}
	}
	else {	/* Normal ordering */
		for (col = 0; col < (*width); col++) actual_col[col] = (*first_col) + col;
	}

	*index = actual_col;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "-> region: %g %g, grid: %g %g\n", wesn[XLO], wesn[XHI], header->wesn[XLO], header->wesn[XHI]);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "row: %d %d, col: %d %d\n", *first_row, *last_row, *first_col, *last_col);

	return (GMT_NOERROR);
}

GMT_LOCAL void gmt_decode_grd_h_info_old (struct GMT_CTRL *GMT, char *input, struct GMT_GRID_HEADER *h) {
	/* Given input string, copy elements into string portions of h.
		 By default use "/" as the field separator. However, if the first and
		 last character of the input string is the same AND that character
		 is non-alpha-numeric, use the first character as a separator. This
		 is to allow "/" as part of the fields.
		 If a field is blank [or has an equals sign - backwards compatibility], skip it.
		 This routine is usually called if -D<input> was given by user,
		 and after gmt_grd_init() has been called. */

	char *ptr, *stringp = input, sep[] = "/";
	bool copy;
	unsigned int entry = 0;
	size_t len = 0;
	double d;

	if (input[0] != input[strlen(input)-1]) {}
	else if (input[0] == '=') {}
	else if (input[0] >= 'A' && input[0] <= 'Z') {}
	else if (input[0] >= 'a' && input[0] <= 'z') {}
	else if (input[0] >= '0' && input[0] <= '9') {}
	else {
		sep[0] = input[0];
		++stringp; /* advance past first field separator */
	}

	while ((ptr = strsep (&stringp, sep)) != NULL) { /* using strsep because of possible empty fields */
		if (*ptr != '\0' || strcmp (ptr, "=") == 0 || strcmp (ptr, " ") == 0) { /* entry is not blank or "=" or " " */
			len = strlen (ptr);
			copy = (len > 1 || ptr[0] != '-');	/* Copy unless a "-" was given */
			switch (entry) {
				case 0:
					gmt_M_memset (h->x_units, GMT_GRID_UNIT_LEN80, char);
					if (len >= GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
								"X unit string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_UNIT_LEN80);
					if (copy) strncpy (h->x_units, ptr, GMT_GRID_UNIT_LEN80-1);
					break;
				case 1:
					gmt_M_memset (h->y_units, GMT_GRID_UNIT_LEN80, char);
					if (len >= GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
								"Y unit string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_UNIT_LEN80);
					if (copy) strncpy (h->y_units, ptr, GMT_GRID_UNIT_LEN80-1);
					break;
				case 2:
					gmt_M_memset (h->z_units, GMT_GRID_UNIT_LEN80, char);
					if (len >= GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
								"Z unit string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_UNIT_LEN80);
					if (copy) strncpy (h->z_units, ptr, GMT_GRID_UNIT_LEN80-1);
					break;
				case 3:
				 	d = strtod (ptr, NULL);
					if (d != 0.0) h->z_scale_factor = d;	/* Don't want scale factor to become zero */
					break;
				case 4:
					h->z_add_offset = strtod (ptr, NULL);
					break;
				case 5:
					h->nan_value = strtof (ptr, NULL);
					break;
				case 6:
					gmt_M_memset (h->title, GMT_GRID_TITLE_LEN80, char);
					if (len >= GMT_GRID_TITLE_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
								"Title string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_TITLE_LEN80);
					if (copy) strncpy (h->title, ptr, GMT_GRID_TITLE_LEN80-1);
					break;
				case 7:
					gmt_M_memset (h->remark, GMT_GRID_REMARK_LEN160, char);
					if (len >= GMT_GRID_REMARK_LEN160)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
								"Remark string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_REMARK_LEN160);
					if (copy) strncpy (h->remark, ptr, GMT_GRID_REMARK_LEN160-1);
					break;
				default:
					break;
			}
		}
		entry++;
	}
}

int gmt_decode_grd_h_info (struct GMT_CTRL *GMT, char *input, struct GMT_GRID_HEADER *h) {
	size_t k, n_slash = 0;
	unsigned int uerr = 0;
	bool old = false;

	for (k = 0; k < strlen (input); k++) if (input[k] == '/') n_slash++;
	if (!(input[0] == '+' && (strstr (input, "+x") || strstr (input, "+y") || strstr (input, "+z") || strstr (input, "+s") || \
	    strstr (input, "+o") || strstr (input, "+n") || strstr (input, "+t") || strstr (input, "+r")))) {	/* Cannot be new syntax */
		old = (n_slash > 4);	/* Pretty sure this is the old syntax of that many slashes */
	}
	if (old)	/* Old syntax: -D<xname>/<yname>/<zname>/<scale>/<offset>/<invalid>/<title>/<remark> */
		gmt_decode_grd_h_info_old (GMT, input, h);
	else {	/* New syntax: -D[+x<xname>][+yyname>][+z<zname>][+s<scale>][+ooffset>][+n<invalid>][+t<title>][+r<remark>] */
		char word[GMT_LEN256] = {""};
		unsigned int pos = 0;
		double d;
		while (gmt_getmodopt (GMT, 'D', input, "xyzsontr", &pos, word, &uerr) && uerr == 0) {
			switch (word[0]) {
				case 'x':	/* Revise x-unit name */
					gmt_M_memset (h->x_units, GMT_GRID_UNIT_LEN80, char);
					if (strlen(word) > GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
							"x_unit string exceeds upper length of %d characters (truncated)\n",
							GMT_GRID_UNIT_LEN80);
					if (word[1]) strncpy (h->x_units, &word[1], GMT_GRID_UNIT_LEN80-1);
					break;
				case 'y':	/* Revise y-unit name */
					gmt_M_memset (h->y_units, GMT_GRID_UNIT_LEN80, char);
					if (strlen(word) > GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
							"y_unit string exceeds upper length of %d characters (truncated)\n",
							GMT_GRID_UNIT_LEN80);
					if (word[1]) strncpy (h->y_units, &word[1], GMT_GRID_UNIT_LEN80-1);
					break;
				case 'z':	/* Revise z-unit name */
					gmt_M_memset (h->z_units, GMT_GRID_UNIT_LEN80, char);
					if (strlen(word) > GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
							"z_unit string exceeds upper length of %d characters (truncated)\n",
							GMT_GRID_UNIT_LEN80);
					if (word[1]) strncpy (h->z_units, &word[1], GMT_GRID_UNIT_LEN80-1);
					break;
				case 's':	/* Revise the scale */
				 	d = strtod (&word[1], NULL);
					if (d != 0.0) h->z_scale_factor = d;	/* Don't want scale factor to become zero */
					break;
				case 'o':	/* Revise the offset */
					h->z_add_offset = strtod (&word[1], NULL);
					break;
				case 'n':	/* Revise the nan-value */
					h->nan_value = strtof (&word[1], NULL);
					break;
				case 't':	/* Revise the title */
					gmt_M_memset (h->title, GMT_GRID_TITLE_LEN80, char);
					if (strlen(word) > GMT_GRID_TITLE_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
							"WTitle string exceeds upper length of %d characters (truncated)\n",
							GMT_GRID_TITLE_LEN80);
					if (word[1]) strncpy (h->title, &word[1], GMT_GRID_TITLE_LEN80-1);
					break;
				case 'r':	/* Revise the title */
					gmt_M_memset (h->remark, GMT_GRID_REMARK_LEN160, char);
					if (strlen(word) > GMT_GRID_REMARK_LEN160)
						GMT_Report (GMT->parent, GMT_MSG_WARNING,
							"Remark string exceeds upper length of %d characters (truncated)\n",
							GMT_GRID_REMARK_LEN160);
					if (word[1]) strncpy (h->remark, &word[1], GMT_GRID_REMARK_LEN160-1);
					break;
				default:	/* These are caught in gmt_getmodopt so break is just for Coverity */
					break;
			}
		}
	}
	return (int)uerr;
}

void gmt_grd_info_syntax (struct GMT_CTRL *GMT, char option) {
	/* Display the option for setting grid metadata in grdedit etc. */
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t-%c Append grid header information as one string composed of one or\n", option);
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t   more modifiers; items not listed will remain unchanged:\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +x[<name>]   Sets the x-unit name; leave blank to reset\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +y[<name>]   Sets the y-unit name; leave blank to reset\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +z[<name>]   Sets the z-unit name; leave blank to reset\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +t[<title>]  Sets the grid title;  leave blank to reset\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +r[<remark>] Sets the grid remark; leave blank to reset\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +s<scale>    Sets the z-scale\n");
	GMT_Message (GMT->parent, GMT_TIME_NONE, "\t     +o<offset>   Sets the z-offset\n");
}

void gmt_set_grdinc (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* Update grid increments based on w/e/s/n, n_columns/n_rows, and registration */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);
	gmt_M_unused(GMT);
	h->inc[GMT_X] = gmt_M_get_inc (GMT, h->wesn[XLO], h->wesn[XHI], h->n_columns, h->registration);
	h->inc[GMT_Y] = gmt_M_get_inc (GMT, h->wesn[YLO], h->wesn[YHI], h->n_rows, h->registration);
	HH->r_inc[GMT_X] = 1.0 / h->inc[GMT_X];	/* Get inverse increments to avoid divisions later */
	HH->r_inc[GMT_Y] = 1.0 / h->inc[GMT_Y];
}

void gmt_set_grddim (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* Assumes pad is set and then computes n_columns, n_rows, mx, my, nm, size, xy_off based on w/e/s/n.  */
	h->n_columns = gmt_M_grd_get_nx (GMT, h);		/* Set n_columns, n_rows based on w/e/s/n and offset */
	h->n_rows = gmt_M_grd_get_ny (GMT, h);
	h->mx = gmt_M_grd_get_nxpad (h, h->pad);	/* Set mx, my based on h->{n_columns,n_rows} and the current pad */
	h->my = gmt_M_grd_get_nypad (h, h->pad);
	h->nm = gmt_M_grd_get_nm (h);		/* Sets the number of actual data items */
	h->size = gmt_grd_get_size (h);	/* Sets the number of items (not bytes!) needed to hold this array, which includes the padding (size >= nm) */
	h->xy_off = 0.5 * h->registration;
	gmt_set_grdinc (GMT, h);
}

void gmt_grd_init (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, struct GMT_OPTION *options, bool update) {
	/* gmt_grd_init initializes a grd header to default values and copies the
	 * options to the header variable command.
	 * update = true if we only want to update command line */

	int i;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (update)	/* Only clean the command history */
		gmt_M_memset (header->command, GMT_GRID_COMMAND_LEN320, char);
	else {		/* Wipe the slate clean */
		void *ptr = HH->index_function;	/* Keep these two */
		char mem[4];
		gmt_M_memcpy (mem, header->mem_layout, 4, char);
		gmt_M_memset (header, 1, struct GMT_GRID_HEADER);
		HH->index_function = ptr;
		header->hidden = HH;
		gmt_M_memcpy (header->mem_layout, mem, 4, char);

		/* Set the variables that are not initialized to 0/false/NULL */
		header->z_scale_factor = 1.0;
		HH->row_order   	   = k_nc_start_south; /* S->N */
		HH->z_id         	  = -1;
		header->n_bands        = 1; /* Grids have at least one band but images may have 3 (RGB) or 4 (RGBA) */
		header->z_min          = GMT->session.d_NaN;
		header->z_max          = GMT->session.d_NaN;
		header->nan_value      = GMT->session.f_NaN;
		if (gmt_M_is_geographic (GMT, GMT_OUT)) {
			strcpy (header->x_units, "longitude [degrees_east]");
			strcpy (header->y_units, "latitude [degrees_north]");
		}
		else {
			strcpy (header->x_units, "x");
			strcpy (header->y_units, "y");
		}
		strcpy (header->z_units, "z");
		gmt_M_grd_setpad (GMT, header, GMT->current.io.pad); /* Assign default pad */
	}

	/* Always update command line history, if given */

	if (options) {
		size_t len;
		struct GMTAPI_CTRL *API = GMT->parent;
		int argc = 0; char **argv = NULL, *c = NULL;
		char file[GMT_LEN64] = {""}, *txt = NULL;

		if ((argv = GMT_Create_Args (API, &argc, options)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Could not create argc, argv from linked structure options!\n");
			return;
		}
		strncpy (header->command, GMT->init.module_name, GMT_GRID_COMMAND_LEN320-1);
		len = strlen (header->command);
		for (i = 0; len < GMT_GRID_COMMAND_LEN320 && i < argc; i++) {
			if (gmtlib_file_is_srtmlist (API, argv[i])) {	/* Want to replace the srtm list with the original @earth_relief_xxx name instead */
				snprintf (file, GMT_LEN64, "@earth_relief_0%cs", argv[i][strlen(argv[i])-8]);
				txt = file;
			}
			else if (gmt_M_file_is_remotedata (argv[i]) && (c = strstr (argv[i], ".grd"))) {
				c[0] = '\0';
				snprintf (file, GMT_LEN64, "%s", argv[i]);
				c[0] = '.';
				txt = file;
			}
			else
				txt = argv[i];
			len += strlen (txt) + 1;
			if (len >= GMT_GRID_COMMAND_LEN320) continue;
			strcat (header->command, " ");
			strcat (header->command, txt);
		}
		if (len < GMT_GRID_COMMAND_LEN320)
			header->command[len] = 0;
		else /* Must truncate */
			header->command[GMT_GRID_COMMAND_LEN320-1] = 0;
		snprintf (header->title, GMT_GRID_TITLE_LEN80, "Produced by %s", GMT->init.module_name);
		GMT_Destroy_Args (API, argc, &argv);
	}
}

void gmt_grd_shift (struct GMT_CTRL *GMT, struct GMT_GRID *G, double shift) {
	/* Rotate geographical, global grid in e-w direction
	 * This function will shift a grid by shift degrees.
	 * It is only called when we know the grid is geographic. */

	unsigned int row, n_warn = 0;
	int col, n_shift, width, actual_col;
	bool gridline;
	uint64_t ij;
	gmt_grdfloat *tmp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (G->header);

	n_shift = irint (shift * HH->r_inc[GMT_X]);
	width = irint (360.0 * HH->r_inc[GMT_X]);
	if (width > (int)G->header->n_columns) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot rotate grid, width is too small\n");
		return;
	}

	/* Shift boundaries */

	tmp = gmt_M_memory (GMT, NULL, G->header->n_columns, gmt_grdfloat);

	G->header->wesn[XLO] += shift;
	G->header->wesn[XHI] += shift;
	if (G->header->wesn[XHI] < 0.0) {
		G->header->wesn[XLO] += 360.0;
		G->header->wesn[XHI] += 360.0;
	}
	else if (G->header->wesn[XHI] > 360.0) {
		G->header->wesn[XLO] -= 360.0;
		G->header->wesn[XHI] -= 360.0;
	}

	gridline = (width < (int)G->header->n_columns);	/* Gridline-registrered grids will have width = n_columns-1, pixel grids have width = n_columns */

	if (gridline) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Repeating column now at %g/%g\n", G->header->wesn[XLO], G->header->wesn[XHI]);


	for (row = 0; row < G->header->n_rows; row++) {
		ij = gmt_M_ijp (G->header, row, 0);
		if (gridline && G->data[ij] != G->data[ij+width]) n_warn++;
		for (col = 0; col < (int)G->header->n_columns; col++) {
			actual_col = (col - n_shift) % width;
			if (actual_col < 0) actual_col += width;
			tmp[actual_col] = G->data[ij+col];
		}
		if (gridline) tmp[width] = tmp[0];	/* Set new repeating column */
		gmt_M_memcpy (&G->data[ij], tmp, G->header->n_columns, gmt_grdfloat);
	}
	gmt_M_free (GMT, tmp);

	if (n_warn)
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Inconsistent values at repeated longitude nodes (%g and %g) for %d rows\n",
			G->header->wesn[XLO], G->header->wesn[XHI], n_warn);
}

int gmt_grd_setregion (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, double *wesn, unsigned int interpolant) {
	/* gmt_grd_setregion determines what w,e,s,n should be passed to gmtlib_read_grd.
	 * It does so by using GMT->common.R.wesn which have been set correctly by map_setup.
	 * Use interpolant to indicate if (and how) the grid is interpolated after this call.
	 * This determines possible extension of the grid to allow interpolation (without padding).
	 *
	 * Here are some considerations about the boundary we need to match, assuming the grid is gridline oriented:
	 * - When the output is to become pixels, the outermost point has to be beyond 0.5 cells inside the region
	 * - When linear interpolation is needed afterwards, the outermost point needs to be on the region edge or beyond
	 * - When the grid is pixel oriented, the limits need to go outward by another 0.5 cells
	 * - When the region is global, do not extend the longitudes outward (otherwise you create wrap-around issues)
	 * So to determine the boundary, we go inward from there.
	 *
	 * Return values are as follows:
	 * 0 = Grid is entirely outside Region
	 * 1 = Region is equal to or entirely inside Grid
	 * 2 = All other
	 */

	bool grid_global;
	double shift_x, x_range, off;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	/* First make an educated guess whether the grid and region are geographical and global */
	grid_global = gmt_grd_is_global (GMT, h);

	switch (interpolant) {
		case BCR_BILINEAR:
			off = 0.0;
			break;
		case BCR_BSPLINE:
		case BCR_BICUBIC:
			off = 1.5;
			break;
		default:
			off = -0.5;
			break;
	}
	if (h->registration == GMT_GRID_PIXEL_REG) off += 0.5;

	/* Initial assignment of wesn */
	wesn[YLO] = GMT->common.R.wesn[YLO] - off * h->inc[GMT_Y], wesn[YHI] = GMT->common.R.wesn[YHI] + off * h->inc[GMT_Y];
	if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && gmt_M_x_is_lon (GMT, GMT_IN)) off = 0.0;
	wesn[XLO] = GMT->common.R.wesn[XLO] - off * h->inc[GMT_X], wesn[XHI] = GMT->common.R.wesn[XHI] + off * h->inc[GMT_X];

	if (GMT->common.R.oblique && !gmt_M_is_rect_graticule (GMT)) {	/* Used -R... with oblique boundaries - return entire grid */
		if (wesn[XHI] < h->wesn[XLO])	/* Make adjustments so GMT->current.proj.[w,e] jives with h->wesn */
			shift_x = 360.0;
		else if (wesn[XLO] > h->wesn[XHI])
			shift_x = -360.0;
		else
			shift_x = 0.0;

		wesn[XLO] = h->wesn[XLO] + lrint ((wesn[XLO] - h->wesn[XLO] + shift_x) * HH->r_inc[GMT_X]) * h->inc[GMT_X];
		wesn[XHI] = h->wesn[XHI] + lrint ((wesn[XHI] - h->wesn[XHI] + shift_x) * HH->r_inc[GMT_X]) * h->inc[GMT_X];
		wesn[YLO] = h->wesn[YLO] + lrint ((wesn[YLO] - h->wesn[YLO]) * HH->r_inc[GMT_Y]) * h->inc[GMT_Y];
		wesn[YHI] = h->wesn[YHI] + lrint ((wesn[YHI] - h->wesn[YHI]) * HH->r_inc[GMT_Y]) * h->inc[GMT_Y];

		/* Make sure we do not exceed grid domain (which can happen if GMT->common.R.wesn exceeds the grid limits) */
		if (wesn[XLO] < h->wesn[XLO] && !grid_global) wesn[XLO] = h->wesn[XLO];
		if (wesn[XHI] > h->wesn[XHI] && !grid_global) wesn[XHI] = h->wesn[XHI];
		if (wesn[YLO] < h->wesn[YLO]) wesn[YLO] = h->wesn[YLO];
		if (wesn[YHI] > h->wesn[YHI]) wesn[YHI] = h->wesn[YHI];

		/* If North or South pole are within the map boundary, we need all longitudes but restrict latitudes */
		if (!GMT->current.map.outside (GMT, 0.0, +90.0)) wesn[XLO] = h->wesn[XLO], wesn[XHI] = h->wesn[XHI], wesn[YHI] = h->wesn[YHI];
		if (!GMT->current.map.outside (GMT, 0.0, -90.0)) wesn[XLO] = h->wesn[XLO], wesn[XHI] = h->wesn[XHI], wesn[YLO] = h->wesn[YLO];
		return (grid_global ? 1 : 2);
	}

	/* First set and check latitudes since they have no complications */
	wesn[YLO] = MAX (h->wesn[YLO], h->wesn[YLO] + floor ((wesn[YLO] - h->wesn[YLO]) * HH->r_inc[GMT_Y] + GMT_CONV4_LIMIT) * h->inc[GMT_Y]);
	wesn[YHI] = MIN (h->wesn[YHI], h->wesn[YLO] + ceil  ((wesn[YHI] - h->wesn[YLO]) * HH->r_inc[GMT_Y] - GMT_CONV4_LIMIT) * h->inc[GMT_Y]);

	if (wesn[YHI] <= wesn[YLO]) {	/* Grid must be outside chosen -R */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your grid y's or latitudes appear to be outside the map region and will be skipped.\n");
		return (0);
	}

	/* Periodic grid with 360 degree range is easy */

	if (grid_global) {
		wesn[XLO] = h->wesn[XLO] + floor ((wesn[XLO] - h->wesn[XLO]) * HH->r_inc[GMT_X] + GMT_CONV4_LIMIT) * h->inc[GMT_X];
		wesn[XHI] = h->wesn[XLO] + ceil  ((wesn[XHI] - h->wesn[XLO]) * HH->r_inc[GMT_X] - GMT_CONV4_LIMIT) * h->inc[GMT_X];
		/* For the odd chance that xmin or xmax are outside the region: bring them in */
		if (wesn[XHI] - wesn[XLO] >= 360.0) {
			while ((wesn[XLO]) < GMT->common.R.wesn[XLO]) wesn[XLO] += h->inc[GMT_X];
			while ((wesn[XHI]) > GMT->common.R.wesn[XHI]) wesn[XHI] -= h->inc[GMT_X];
		}
		return (1);
	}
	if (GMT->current.map.is_world) {
		wesn[XLO] = h->wesn[XLO], wesn[XHI] = h->wesn[XHI];
		return (1);
	}

	/* Shift a geographic grid 360 degrees up or down to maximize the amount of longitude range */

	if (gmt_M_x_is_lon (GMT, GMT_IN)) {
		if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) {
			wesn[XLO] = h->wesn[XLO], wesn[XHI] = h->wesn[XHI];
			return (1);
		}
		x_range = MIN (wesn[XLO], h->wesn[XHI]) - MAX (wesn[XHI], h->wesn[XLO]);
		if (MIN (wesn[XLO], h->wesn[XHI] + 360.0) - MAX (wesn[XHI], h->wesn[XLO] + 360.0) > x_range)
			shift_x = 360.0;
		else if (MIN (wesn[XLO], h->wesn[XHI] - 360.0) - MAX (wesn[XHI], h->wesn[XLO] - 360.0) > x_range)
			shift_x = -360.0;
		else
			shift_x = 0.0;
		h->wesn[XLO] += shift_x;
		h->wesn[XHI] += shift_x;
	}

	wesn[XLO] = MAX (h->wesn[XLO], h->wesn[XLO] + floor ((wesn[XLO] - h->wesn[XLO]) * HH->r_inc[GMT_X] + GMT_CONV4_LIMIT) * h->inc[GMT_X]);
	wesn[XHI] = MIN (h->wesn[XHI], h->wesn[XLO] + ceil  ((wesn[XHI] - h->wesn[XLO]) * HH->r_inc[GMT_X] - GMT_CONV4_LIMIT) * h->inc[GMT_X]);

	if (wesn[XHI] <= wesn[XLO]) {	/* Grid may is outside chosen -R in longitude */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your grid x's or longitudes appear to be outside the map region and will be skipped.\n");
		return (0);
	}
	return (2);
}

int gmt_adjust_loose_wesn (struct GMT_CTRL *GMT, double wesn[], struct GMT_GRID_HEADER *header) {
	/* Used to ensure that sloppy w,e,s,n values are rounded to the gridlines or pixels in the referenced grid.
	 * Upon entry, the boundaries w,e,s,n are given as a rough approximation of the actual subset needed.
	 * The routine will limit the boundaries to the grids region and round w,e,s,n to the nearest gridline or
	 * pixel boundaries (depending on the grid orientation).
	 * Warnings are produced when the w,e,s,n boundaries are adjusted, so this routine is currently not
	 * intended to throw just any values at it (although one could).
	 */

	bool global, error = false;
	double val, dx, small;
	char format[GMT_LEN256] = {""};
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	switch (gmt_minmaxinc_verify (GMT, wesn[XLO], wesn[XHI], header->inc[GMT_X], GMT_CONV4_LIMIT)) {	/* Check if range is compatible with x_inc */
		case 3:
			return (GMT_GRDIO_BAD_XINC);
			break;
		case 2:
			return (GMT_GRDIO_BAD_XRANGE);
			break;
		default:
			/* Everything is seemingly OK */
			break;
	}
	switch (gmt_minmaxinc_verify (GMT, wesn[YLO], wesn[YHI], header->inc[GMT_Y], GMT_CONV4_LIMIT)) {	/* Check if range is compatible with y_inc */
		case 3:
			return (GMT_GRDIO_BAD_YINC);
			break;
		case 2:
			return (GMT_GRDIO_BAD_YRANGE);
			break;
		default:
			/* Everything is OK */
			break;
	}
	global = gmt_grd_is_global (GMT, header);

	if (!global) {
		if (gmt_M_x_is_lon (GMT, GMT_IN)) {
			/* If longitudes are all west of range or all east of range, try moving them by 360 degrees east or west */
			if (wesn[XHI] < header->wesn[XLO])
				wesn[XLO] += 360.0, wesn[XHI] += 360.0;
			else if (wesn[XLO] > header->wesn[XHI])
				wesn[XLO] -= 360.0, wesn[XHI] -= 360.0;
		}
		if (header->wesn[XLO] - wesn[XLO] > GMT_CONV4_LIMIT) wesn[XLO] = header->wesn[XLO], error = true;
		if (wesn[XHI] - header->wesn[XHI] > GMT_CONV4_LIMIT) wesn[XHI] = header->wesn[XHI], error = true;
	}
	if (header->wesn[YLO] - wesn[YLO] > GMT_CONV4_LIMIT) wesn[YLO] = header->wesn[YLO], error = true;
	if (wesn[YHI] - header->wesn[YHI] > GMT_CONV4_LIMIT) wesn[YHI] = header->wesn[YHI], error = true;
	if (error)
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Region exceeds grid domain. Region reduced to grid domain.\n");

	if (!(gmt_M_x_is_lon (GMT, GMT_IN) && gmt_M_360_range (wesn[XLO], wesn[XHI]) && global)) {    /* Do this unless a 360 longitude wrap */
		small = GMT_CONV4_LIMIT * header->inc[GMT_X];

		val = header->wesn[XLO] + lrint ((wesn[XLO] - header->wesn[XLO]) * HH->r_inc[GMT_X]) * header->inc[GMT_X];
		dx = fabs (wesn[XLO] - val);
		if (gmt_M_x_is_lon (GMT, GMT_IN)) dx = fmod (dx, 360.0);
		if (dx > small) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING,
			            "(w - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			snprintf (format, GMT_LEN256, "w reset from %s to %s\n",
			          GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (GMT->parent, GMT_MSG_WARNING, format, wesn[XLO], val);
			wesn[XLO] = val;
		}

		val = header->wesn[XLO] + lrint ((wesn[XHI] - header->wesn[XLO]) * HH->r_inc[GMT_X]) * header->inc[GMT_X];
		dx = fabs (wesn[XHI] - val);
		if (gmt_M_x_is_lon (GMT, GMT_IN)) dx = fmod (dx, 360.0);
		if (dx > small) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING,
			            "(e - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			snprintf (format, GMT_LEN256, "e reset from %s to %s\n",
			          GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (GMT->parent, GMT_MSG_WARNING, format, wesn[XHI], val);
			wesn[XHI] = val;
		}
	}

	/* Check if s,n are a multiple of y_inc offset from y_min - if not adjust s, n */
	small = GMT_CONV4_LIMIT * header->inc[GMT_Y];

	val = header->wesn[YLO] + lrint ((wesn[YLO] - header->wesn[YLO]) * HH->r_inc[GMT_Y]) * header->inc[GMT_Y];
	if (fabs (wesn[YLO] - val) > small) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "(s - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n",
		            GMT_CONV4_LIMIT);
		snprintf (format, GMT_LEN256, "s reset from %s to %s\n",
		          GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, format, wesn[YLO], val);
		wesn[YLO] = val;
	}

	val = header->wesn[YLO] + lrint ((wesn[YHI] - header->wesn[YLO]) * HH->r_inc[GMT_Y]) * header->inc[GMT_Y];
	if (fabs (wesn[YHI] - val) > small) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "(n - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n",
		            GMT_CONV4_LIMIT);
		snprintf (format, GMT_LEN256, "n reset from %s to %s\n",
		          GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, format, wesn[YHI], val);
		wesn[YHI] = val;
	}
	return (GMT_NOERROR);
}

void gmt_scale_and_offset_f (struct GMT_CTRL *GMT, gmt_grdfloat *data, size_t length, double scale, double offset) {
	/* Routine that does the data conversion and sanity checking before
	 * calling scale_and_offset_f() to scale and offset the data in a grid */
	gmt_grdfloat scale_f  = (gmt_grdfloat)scale;
	gmt_grdfloat offset_f = (gmt_grdfloat)offset;

	if (scale_f == 1.0 && offset_f == 0.0)
		return; /* No work needed */

	/* Sanity checks */
	if (!isnormal (scale)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Scale must be a non-zero normalized number (%g).\n", scale);
		scale_f = 1.0f;
	}
	if (!isfinite (offset)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Offset must be a finite number (%g).\n", offset);
		offset_f = 0.0f;
	}

	/* Call workhorse */
	scale_and_offset_f (data, length, scale_f, offset_f);
}

int gmt_read_img (struct GMT_CTRL *GMT, char *imgfile, struct GMT_GRID *Grid, double *in_wesn, double scale, unsigned int mode, double lat, bool init) {
	/* Function that reads an entire Sandwell/Smith Mercator grid and stores it like a regular
	 * GMT grid.  If init is true we also initialize the Mercator projection.  Lat should be 0.0
	 * if we are dealing with standard 72 or 80 img latitude; else it must be specified.
	 */

	int status, first_i;
	unsigned int min, actual_col, n_cols, row, col, first;
	uint64_t ij;
	off_t n_skip;
	int16_t *i2 = NULL;
	uint16_t *u2 = NULL;
	char file[PATH_MAX];
	struct stat buf;
	FILE *fp = NULL;
	double wesn[4], wesn_all[4];
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (Grid->header);

	first = gmt_download_file_if_not_found (GMT, imgfile, GMT_CACHE_DIR);
	if (!gmt_getdatapath (GMT, &imgfile[first], file, R_OK)) return (GMT_GRDIO_FILE_NOT_FOUND);
	if (stat (file, &buf)) return (GMT_GRDIO_STAT_FAILED);	/* Inquiry about file failed somehow */

	switch (buf.st_size) {	/* Known sizes are 1 or 2 min at lat_max = ~72, ~80, or ~85.  Set exact latitude */
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_85*GMT_IMG_ITEMSIZE:
			lat = GMT_IMG_MAXLAT_85;
			min = 1;
			break;
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_80*GMT_IMG_ITEMSIZE:
			lat = GMT_IMG_MAXLAT_80;
			min = 1;
			break;
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_72*GMT_IMG_ITEMSIZE:
			lat = GMT_IMG_MAXLAT_72;
			min = 1;
			break;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_85*GMT_IMG_ITEMSIZE:
			lat = GMT_IMG_MAXLAT_85;
			min = 2;
			break;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE:
			lat = GMT_IMG_MAXLAT_80;
			min = 2;
			break;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_72*GMT_IMG_ITEMSIZE:
			lat = GMT_IMG_MAXLAT_72;
			min = 2;
			break;
		case GMT_IMG_NLON_4M*GMT_IMG_NLAT_4M_72*GMT_IMG_ITEMSIZE:	/* Test grids only */
			lat = GMT_IMG_MAXLAT_72;
			min = 4;
			break;
		default:
			if (lat == 0.0) return (GMT_GRDIO_BAD_IMG_LAT);
			min = (buf.st_size > GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE) ? 1 : 2;
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "img file %s has unusual size - grid increment defaults to %d min\n", file, min);
			break;
	}

	wesn_all[XLO] = GMT_IMG_MINLON;	wesn_all[XHI] = GMT_IMG_MAXLON;
	wesn_all[YLO] = -lat;		wesn_all[YHI] = lat;
	if (!in_wesn || (in_wesn[XLO] == in_wesn[XHI] && in_wesn[YLO] == in_wesn[YHI])) {	/* Default is entire grid */
		gmt_M_memcpy (wesn, wesn_all, 4, double);
	}
	else	/* Use specified subset */
		gmt_M_memcpy (wesn, in_wesn, 4, double);

	if ((fp = gmt_fopen (GMT, file, "rb")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Reading img grid from file %s (scale = %g mode = %d lat = %g)\n",
	            &imgfile[first], scale, mode, lat);
	Grid->header->inc[GMT_X] = Grid->header->inc[GMT_Y] = min / 60.0;

	if (init) {
		/* Select plain Mercator on a sphere with -Jm1 -R0/360/-lat/+lat */
		GMT->current.setting.proj_ellipsoid = gmt_get_ellipsoid (GMT, "Sphere");
		GMT->current.proj.units_pr_degree = true;
		GMT->current.proj.pars[0] = 180.0;
		GMT->current.proj.pars[1] = 0.0;
		GMT->current.proj.pars[2] = 1.0;
		GMT->current.proj.projection = GMT->current.proj.projection_GMT = GMT_MERCATOR;
		gmt_set_geographic (GMT, GMT_IN);
		GMT->common.J.active = true;

		gmt_M_err_pass (GMT, gmt_proj_setup (GMT, wesn_all), file);
	}

	if (wesn[XLO] < 0.0 && wesn[XHI] < 0.0) wesn[XLO] += 360.0, wesn[XHI] += 360.0;

	/* Project lon/lat boundaries to Mercator units */
	gmt_geo_to_xy (GMT, wesn[XLO], wesn[YLO], &Grid->header->wesn[XLO], &Grid->header->wesn[YLO]);
	gmt_geo_to_xy (GMT, wesn[XHI], wesn[YHI], &Grid->header->wesn[XHI], &Grid->header->wesn[YHI]);

	/* Adjust boundaries to multiples of increments, making sure we are inside bounds */
	Grid->header->wesn[XLO] = MAX (GMT_IMG_MINLON, floor (Grid->header->wesn[XLO] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X]);
	Grid->header->wesn[XHI] = MIN (GMT_IMG_MAXLON, ceil (Grid->header->wesn[XHI] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X]);
	if (Grid->header->wesn[XLO] > Grid->header->wesn[XHI]) Grid->header->wesn[XLO] -= 360.0;
	Grid->header->wesn[YLO] = MAX (0.0, floor (Grid->header->wesn[YLO] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y]);
	Grid->header->wesn[YHI] = MIN (GMT->current.proj.rect[YHI], ceil (Grid->header->wesn[YHI] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y]);
	/* Allocate grid memory */

	Grid->header->registration = GMT_GRID_PIXEL_REG;	/* These are always pixel grids */
	if ((status = gmt_grd_RI_verify (GMT, Grid->header, 1))) {	/* Final verification of -R -I; return error if we must */
		gmt_fclose (GMT, fp);
		return (status);
	}
	gmt_M_grd_setpad (GMT, Grid->header, GMT->current.io.pad);			/* Assign default pad */
	gmt_set_grddim (GMT, Grid->header);					/* Set all dimensions before returning */
	Grid->data = gmt_M_memory_aligned (GMT, NULL, Grid->header->size, gmt_grdfloat);

	n_cols = (min == 1) ? GMT_IMG_NLON_1M : GMT_IMG_NLON_2M;		/* Number of columns (10800 or 21600) */
	first_i = irint (floor (Grid->header->wesn[XLO] * HH->r_inc[GMT_X]));				/* first tile partly or fully inside region */
	if (first_i < 0) first_i += n_cols;
	n_skip = lrint (floor ((GMT->current.proj.rect[YHI] - Grid->header->wesn[YHI]) * HH->r_inc[GMT_Y]));	/* Number of rows clearly above y_max */
	if (fseek (fp, n_skip * n_cols * GMT_IMG_ITEMSIZE, SEEK_SET)) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_SEEK_FAILED);
	}

	i2 = gmt_M_memory (GMT, NULL, n_cols, int16_t);
	for (row = 0; row < Grid->header->n_rows; row++) {	/* Read all the rows, offset by 2 boundary rows and cols */
		if (gmt_M_fread (i2, sizeof (int16_t), n_cols, fp) != n_cols) {
			gmt_M_free (GMT, i2);
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_READ_FAILED);	/* Failed to get one row */
		}
#ifndef WORDS_BIGENDIAN
		u2 = (uint16_t *)i2;
		for (col = 0; col < n_cols; col++)
			u2[col] = bswap16 (u2[col]);
#endif
#ifdef DEBUG
		if (row == 0) {
			uint16_t step, max_step = 0;
			for (col = 1; col < n_cols; col++) {
				step = abs (i2[col] - i2[col-1]);
				if (step > max_step) max_step = step;
			}
			if (max_step > 32768)
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s probably needs to byteswapped (max change = %u)\n", file, max_step);
		}
#endif
		ij = gmt_M_ijp (Grid->header, row, 0);
		for (col = 0, actual_col = first_i; col < Grid->header->n_columns; col++) {	/* Process this row's values */
			switch (mode) {
				case 0:	/* No encoded track flags, do nothing */
					break;
				case 1:	/* Remove the track flag on odd (constrained) points */
					if (i2[actual_col]%2) i2[actual_col]--;
					break;
				case 2:	/* Remove the track flag on odd (constrained) points and set unconstrained to NaN */
					i2[actual_col] = (i2[actual_col]%2) ? i2[actual_col] - 1 : SHRT_MIN;
					break;
				case 3:	/* Set odd (constrained) points to 1 and set unconstrained to 0 */
					i2[actual_col] %= 2;
					break;
			}
			Grid->data[ij+col] = (gmt_grdfloat)((mode == 3) ? i2[actual_col] : (i2[actual_col] * scale));
			if (++actual_col == n_cols) actual_col = 0;	/* Wrapped around 360 */
		}
	}
	gmt_M_free (GMT, i2);
	gmt_fclose (GMT, fp);
	if (init) {
		gmt_M_memcpy (GMT->common.R.wesn, wesn, 4, double);
		GMT->common.J.active = false;
	}
	gmt_BC_init (GMT, Grid->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (GMT, Grid, GMT_IN);	/* Set boundary conditions */
	HH->has_NaNs = GMT_GRID_NO_NANS;	/* No nans in img grids */
	return (GMT_NOERROR);
}

void gmt_grd_pad_off (struct GMT_CTRL *GMT, struct GMT_GRID *G) {
	/* Shifts the grid contents so there is no pad.  The remainder of
	 * the array is not reset and should not be addressed, but
	 * we set it to zero just in case.
	 * If pad is zero then we do nothing.
	 */
	bool is_complex;
	uint64_t nm;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (G->header);

	if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Calling gmt_grd_pad_off on interleaved complex grid! Programming error?\n");
		return;
	}
	if (!gmt_grd_pad_status (GMT, G->header, NULL)) return;	/* No pad so nothing to do */

	/* Here, G has a pad which we need to eliminate */
	is_complex = (G->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK);
	if (!is_complex || (G->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL))
		grdio_pad_grd_off_sub (G, G->data);	/* Remove pad around real component only or entire normal grid */
	if (is_complex && (G->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG))
		grdio_pad_grd_off_sub (G, &G->data[G->header->size/2]);	/* Remove pad around imaginary component */
	nm = G->header->nm;	/* Number of nodes in one component */
	if (is_complex) nm *= 2;	/* But there might be two */
	if (G->header->size > nm) {	/* Just wipe the remaineder of the array to be sure */
		size_t n_to_cleen = G->header->size - nm;
		gmt_M_memset (&(G->data[nm]), n_to_cleen, gmt_grdfloat);	/* nm is 1st position after last row */
	}
	gmt_M_memset (G->header->pad, 4, int);	/* Pad is no longer active */
	gmt_set_grddim (GMT, G->header);		/* Update all dimensions to reflect the padding */
}

void gmt_grd_pad_on (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int *pad) {
 	/* Shift grid content from a non-padded (or differently padded) to a padded organization.
	 * We check that the grid size can handle this and allocate more space if needed.
	 * If pad matches the grid's pad then we do nothing.
	 */
	bool is_complex;
	size_t size;
	struct GMT_GRID_HEADER *h = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (G->header);

	if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Calling gmt_grd_pad_off on interleaved complex grid! Programming error?\n");
		return;
	}
	if (gmt_grd_pad_status (GMT, G->header, pad)) return;	/* Already padded as requested so nothing to do */
	if (pad[XLO] == 0 && pad[XHI] == 0 && pad[YLO] == 0 && pad[YHI] == 0) {	/* Just remove the existing pad entirely */
		gmt_grd_pad_off (GMT, G);
		return;
	}
	/* Here the pads differ (or G has no pad at all) */
	is_complex = (G->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK);
	size = ((size_t)gmt_M_grd_get_nxpad (G->header, pad)) * ((size_t)gmt_M_grd_get_nypad (G->header, pad));	/* New array size after pad is added */
	if (is_complex) size *= 2;	/* Twice the space for complex grids */
	if (size > G->header->size) {	/* Must allocate more space, but since no realloc for aligned memory we must do it the hard way */
		gmt_grdfloat *f = NULL;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Extend grid via copy onto larger memory-aligned grid\n");
		f = gmt_M_memory_aligned (GMT, NULL, size, gmt_grdfloat);	/* New, larger grid size */
		gmt_M_memcpy (f, G->data, G->header->size, gmt_grdfloat);	/* Copy over previous grid values */
		gmt_M_free_aligned (GMT, G->data);			/* Free previous aligned grid memory */
		G->data = f;						/* Attach the new, larger aligned memory */
		G->header->size = size;					/* Update the size */
	}
	/* Because G may have a pad that is nonzero (but different from pad) we need a different header structure in the macros below */
	h = grdio_duplicate_gridheader (GMT, G->header);

	gmt_M_grd_setpad (GMT, G->header, pad);	/* G->header->pad is now set to specified dimensions in pad */
	gmt_set_grddim (GMT, G->header);	/* Update all dimensions to reflect the new padding */
	if (is_complex && (G->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG))
		grdio_pad_grd_on_sub (GMT, G, h, &G->data[size/2]);	/* Add pad around imaginary component first */
	if (!is_complex || (G->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL))
		grdio_pad_grd_on_sub (GMT, G, h, G->data);	/* Add pad around real component */
	gmt_M_free (GMT, h->hidden);	/* Done with this header hidden struct */
	gmt_M_free (GMT, h);	/* Done with this header */
}

void gmt_grd_pad_zero (struct GMT_CTRL *GMT, struct GMT_GRID *G) {
	/* Sets all boundary row/col nodes to zero and sets
	 * the header->BC to GMT_IS_NOTSET.
	 */
	bool is_complex;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (G->header);

	if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Calling gmt_grd_pad_off on interleaved complex grid! Programming error?\n");
		return;
	}
	if (!gmt_grd_pad_status (GMT, G->header, NULL)) return;	/* No pad so nothing to do */
	if (HH->BC[XLO] == GMT_BC_IS_NOTSET && HH->BC[XHI] == GMT_BC_IS_NOTSET && HH->BC[YLO] ==
		GMT_BC_IS_NOTSET && HH->BC[YHI] == GMT_BC_IS_NOTSET)
		return;	/* No BCs set so nothing to do */			/* No pad so nothing to do */
	/* Here, G has a pad with BCs which we need to reset */
	is_complex = (G->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK);
	if (!is_complex || (G->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL))
		grdio_pad_grd_zero_sub (G, G->data);
	if (is_complex && (G->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG))
		grdio_pad_grd_zero_sub (G, &G->data[G->header->size/2]);
	gmt_M_memset (HH->BC, 4U, int);	/* BCs no longer set for this grid */
}

struct GMT_GRID *gmt_get_grid (struct GMT_CTRL *GMT) {
	struct GMT_GRID *G = NULL;
	G = gmt_M_memory (GMT, NULL, 1, struct GMT_GRID);
	G->hidden = gmt_M_memory (GMT, NULL, 1, struct GMT_GRID_HIDDEN);
	return (G);
}

struct GMT_GRID *gmt_create_grid (struct GMT_CTRL *GMT) {
	/* Allocates space for a new grid container.  No space allocated for the gmt_grdfloat grid itself */
	struct GMT_GRID *G = NULL;
	struct GMT_GRID_HIDDEN *GH = NULL;

	G = gmt_get_grid (GMT);
	GH = gmt_get_G_hidden (G);
	G->header = gmt_get_header (GMT);
	gmt_grd_init (GMT, G->header, NULL, false); /* Set default values */
	GMT_Set_Index (GMT->parent, G->header, GMT_GRID_LAYOUT);
	GH->alloc_mode = GMT_ALLOC_INTERNALLY;		/* Memory can be freed by GMT. */
	GH->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level. */
	GH->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */
	return (G);
}

struct GMT_GRID *gmt_duplicate_grid (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int mode) {
	/* Duplicates an entire grid, including data if requested. */
	struct GMT_GRID *Gnew = NULL;

	Gnew = gmt_create_grid (GMT);
	gmt_copy_gridheader (GMT, Gnew->header, G->header);

	if ((mode & GMT_DUPLICATE_DATA) || (mode & GMT_DUPLICATE_ALLOC)) {	/* Also allocate and possibly duplicate data array */
		if ((mode & GMT_DUPLICATE_RESET) && !gmt_grd_pad_status (GMT, G->header, GMT->current.io.pad)) {
			/* Pads differ and we requested resetting the pad */
			gmt_M_grd_setpad (GMT, Gnew->header, GMT->current.io.pad);	/* Set default pad size */
			gmt_set_grddim (GMT, Gnew->header);	/* Update size dimensions given the change of pad */
			if (mode & GMT_DUPLICATE_DATA) {	/* Per row since grid sizes will not the same */
				uint64_t node_in, node_out;
				unsigned int row;
				Gnew->data = gmt_M_memory_aligned (GMT, NULL, Gnew->header->size, gmt_grdfloat);
				gmt_M_row_loop (GMT, G, row) {
					node_in  = gmt_M_ijp (G->header, row, 0);
					node_out = gmt_M_ijp (Gnew->header, row, 0);
					gmt_M_memcpy (&Gnew->data[node_out], &G->data[node_in], G->header->n_columns, gmt_grdfloat);
				}
			}
		}
		else {	/* Can do fast copy */
			Gnew->data = gmt_M_memory_aligned (GMT, NULL, G->header->size, gmt_grdfloat);
			if (mode & GMT_DUPLICATE_DATA) gmt_M_memcpy (Gnew->data, G->data, G->header->size, gmt_grdfloat);
		}

		Gnew->x = gmt_grd_coord (GMT, Gnew->header, GMT_X);	/* Get array of x coordinates */
		Gnew->y = gmt_grd_coord (GMT, Gnew->header, GMT_Y);	/* Get array of y coordinates */
	}
	return (Gnew);
}

void gmt_free_header (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER **header) {
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GRID_HEADER *h = *header;
	if (h == NULL) return;	/* Nothing to deallocate */
	/* Free the header structure and anything allocated by it */
	HH = gmt_get_H_hidden (h);
	if (!GMT->parent->external) {
		gmt_M_str_free (h->ProjRefWKT);
		gmt_M_str_free (h->ProjRefPROJ4);
	}
	gmt_M_str_free (HH->pocket);
	gmt_M_free (GMT, h->hidden);
	gmt_M_free (GMT, *header);
}

unsigned int gmtgrdio_free_grid_ptr (struct GMT_CTRL *GMT, struct GMT_GRID *G, bool free_grid) {
	/* By taking a reference to the grid pointer we can set it to NULL when done */
	struct GMT_GRID_HIDDEN *GH = NULL;
	enum GMT_enum_alloc alloc_mode;
	if (!G) return 0;	/* Nothing to deallocate */
	/* Only free G->data if allocated by GMT AND free_grid is true */
	GH = gmt_get_G_hidden (G);
	if (G->data && free_grid) {
		if (GH->alloc_mode == GMT_ALLOC_INTERNALLY) gmt_M_free_aligned (GMT, G->data);
		G->data = NULL;	/* This will remove reference to external memory since gmt_M_free_aligned would not have been called */
	}
	if (G->x && G->y && free_grid) {
		if (GH->alloc_mode == GMT_ALLOC_INTERNALLY) {
			gmt_M_free (GMT, G->x);
			gmt_M_free (GMT, G->y);
		}
		G->x = G->y = NULL;	/* This will remove reference to external memory since gmt_M_free would not have been called */
	}
	if (GH->extra) gmtapi_close_grd (GMT, G);	/* Close input file used for row-by-row i/o */
	alloc_mode = GH->alloc_mode;
	gmt_M_free (GMT, G->hidden);
	gmt_free_header (GMT, &(G->header));	/* Free the header structure and anything allocated by it */
	return (alloc_mode);
}

void gmt_free_grid (struct GMT_CTRL *GMT, struct GMT_GRID **G, bool free_grid) {
	/* By taking a reference to the grid pointer we can set it to NULL when done */
	(void)gmtgrdio_free_grid_ptr (GMT, *G, free_grid);
	gmt_M_free (GMT, *G);
}

int gmt_set_outgrid (struct GMT_CTRL *GMT, char *file, bool separate, struct GMT_GRID *G, struct GMT_GRID **Out) {
	/* In most situations we can recycle the input grid to be the output grid as well.  However, there
	 * are a few situations when we must override this situation:
	 *   1) When OpenMP is enabled and calculations depend on nearby nodes.
	 *   2) The input grid is a read-only memory location
	 *   3) The intended output file is a memory location.
	 * To avoid wasting memory we try to reuse the input array when
	 * it is possible. We return true when new memory had to be allocated.
	 * Note we duplicate the grid if we must so that Out always has the input
	 * data in it (directly or via the pointer).  */
	struct GMT_GRID_HIDDEN *GH = gmt_get_G_hidden (G);

	if (separate || gmt_M_file_is_memory (file) || GH->alloc_mode == GMT_ALLOC_EXTERNALLY) {	/* Cannot store results in a non-GMT read-only input array */
		if ((*Out = GMT_Duplicate_Data (GMT->parent, GMT_IS_GRID, GMT_DUPLICATE_DATA, G)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to duplicate grid! - this is not a good thing and may crash this module\n");
			(*Out) = G;
		}
		else {
			struct GMT_GRID_HIDDEN *GH = gmt_get_G_hidden (*Out);
			GH->alloc_mode = GMT_ALLOC_INTERNALLY;
		}
		return (true);
	}
	/* Here we may overwrite the input grid and just pass the pointer back */
	(*Out) = G;
	return (false);
}

int gmtgrdio_init_grdheader (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_GRID_HEADER *header, struct GMT_OPTION *options,
                             uint64_t dim[], double wesn[], double inc[], unsigned int registration, unsigned int mode) {
	/* Convenient way of setting a header struct wesn, inc, and registration, then compute dimensions, etc. */
	double wesn_dup[4] = {0.0, 0.0, 0.0, 0.0}, inc_dup[2] = {0.0, 0.0};
	unsigned int n_layers = 1;
	char *regtype[2] = {"gridline", "pixel"};
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	gmt_M_unused(mode);

	if (registration & GMT_GRID_DEFAULT_REG) registration |= GMT->common.R.registration;	/* Set the default registration */
	registration = (registration & 1);	/* Knock off any GMT_GRID_DEFAULT_REG bit */
	if (dim && wesn == NULL && inc == NULL) {	/* Gave dimension instead, set range and inc (1/1) while considering registration */
		gmt_M_memset (wesn_dup, 4, double);
		wesn_dup[XHI] = (double)(dim[GMT_X]);
		wesn_dup[YHI] = (double)(dim[GMT_Y]);
		inc_dup[GMT_X] = inc_dup[GMT_Y] = 1.0;
		if (registration == GMT_GRID_NODE_REG) wesn_dup[XHI] -= 1.0, wesn_dup[YHI] -= 1.0;
		if (dim[GMT_Z] > 1) n_layers = (unsigned int)dim[GMT_Z];
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Grid/Image dimensions imply w/e/s/n = 0/%g/0/%g, inc = 1/1, %s registration, n_layers = %u\n",
			wesn_dup[XHI], wesn_dup[YHI], regtype[registration], n_layers);
	}
	else {	/* Must infer dimension etc from wesn, inc, registration */
		if (wesn == NULL) {	/* Must select -R setting */
			if (!GMT->common.R.active[RSET]) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "No w/e/s/n given and no -R in effect.  Cannot initialize new grid\n");
				GMT_exit (GMT, GMT_ARG_IS_NULL); return GMT_ARG_IS_NULL;
			}
		}
		else	/* In case user is passing header->wesn etc we must save them first as gmt_grd_init will clobber them */
			gmt_M_memcpy (wesn_dup, wesn, 4, double);
		if (inc == NULL) {	/* Must select -I setting */
			if (!GMT->common.R.active[ISET]) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "No increment given and no -I in effect.  Cannot initialize new grid\n");
				GMT_exit (GMT, GMT_ARG_IS_NULL); return GMT_ARG_IS_NULL;
			}
		}
		else	/* In case user is passing header->inc etc we must save them first as gmt_grd_init will clobber them */
			gmt_M_memcpy (inc_dup, inc, 2, double);
		if (dim && dim[GMT_Z] > 1) n_layers = (unsigned int)dim[GMT_Z];
		if (inc != NULL) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Grid/Image dimensions imply w/e/s/n = %g/%g/%g/%g, inc = %g/%g, %s registration, n_layers = %u\n",
			            wesn_dup[XLO], wesn_dup[XHI], wesn_dup[YLO], wesn_dup[YHI], inc[GMT_X], inc[GMT_Y], regtype[registration], n_layers);
		}
	}
	/* Clobber header and reset */
	gmt_grd_init (GMT, header, options, false);	/* This is for new grids only so update is always false */
	if (dim == NULL && wesn == NULL)
		gmt_M_memcpy (header->wesn, GMT->common.R.wesn, 4, double);
	else
		gmt_M_memcpy (header->wesn, wesn_dup, 4, double);
	if (dim == NULL && inc == NULL)
		gmt_M_memcpy (header->inc, GMT->common.R.inc, 2, double);
	else
		gmt_M_memcpy (header->inc, inc_dup, 2, double);
	header->registration = registration;
	/* Copy row-order from R.row_order, if set */
	if (GMT->common.R.row_order) HH->row_order = GMT->common.R.row_order;
	/* Mode may contain complex mode information */
	header->complex_mode = (mode & GMT_GRID_IS_COMPLEX_MASK);
	HH->grdtype = gmtlib_get_grdtype (GMT, direction, header);
	gmt_RI_prepare (GMT, header);	/* Ensure -R -I consistency and set n_columns, n_rows in case of meter units etc. */
	gmt_M_err_pass (GMT, gmt_grd_RI_verify (GMT, header, 1), "");
	gmt_M_grd_setpad (GMT, header, GMT->current.io.pad);	/* Assign default GMT pad */
	if (dim) header->n_bands = n_layers;
	gmt_set_grddim (GMT, header);	/* Set all dimensions before returning */
	gmtlib_grd_get_units (GMT, header);
	gmt_BC_init (GMT, header);	/* Initialize grid interpolation and boundary condition parameters */
	HH->grdtype = gmtlib_get_grdtype (GMT, direction, header);	/* Set grid type (i.e. periodicity for global grids) */
	return (GMT_NOERROR);
}

int gmt_change_grdreg (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, unsigned int registration) {
	unsigned int old_registration;
	double F;
	gmt_M_unused(GMT);
	/* Adjust the grid header to the selected registration, if different.
	 * In all cases we return the original registration. */

	old_registration = header->registration;
	if (old_registration == registration) return (old_registration);	/* Noting to do */

	F = (header->registration == GMT_GRID_PIXEL_REG) ? 0.5 : -0.5;	/* Pixel will shrink w/e/s/n, gridline will extend */
	header->wesn[XLO] += F * header->inc[GMT_X];
	header->wesn[XHI] -= F * header->inc[GMT_X];
	header->wesn[YLO] += F * header->inc[GMT_Y];
	header->wesn[YHI] -= F * header->inc[GMT_Y];

	header->registration = registration;
	header->xy_off = 0.5 * header->registration;
	return (old_registration);
}

void gmt_grd_zminmax (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, gmt_grdfloat *z) {
	/* Reset the xmin/zmax values in the header */
	unsigned int row, col;
	uint64_t node, n = 0;

	h->z_min = DBL_MAX;	h->z_max = -DBL_MAX;
	for (row = 0; row < h->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (h, row, 0); col < h->n_columns; col++, node++) {
			if (isnan (z[node]))
				continue;
			/* Update z_min, z_max */
			h->z_min = MIN (h->z_min, (double)z[node]);
			h->z_max = MAX (h->z_max, (double)z[node]);
			n++;
		}
	}
	if (n == 0) h->z_min = h->z_max = GMT->session.d_NaN;	/* No non-NaNs in the entire grid */
}

void gmt_grd_minmax (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double xyz[2][3]) {
	/* Determine a grid's global min and max locations and z values; return via xyz */
	unsigned int row, col, i;
	uint64_t ij, i_minmax[2] = {0, 0};
	gmt_grdfloat z_extreme[2] = {FLT_MAX, -FLT_MAX};
	gmt_M_unused(GMT);

	gmt_M_grd_loop (GMT, Grid, row, col, ij) {
		if (gmt_M_is_fnan (Grid->data[ij])) continue;
		if (Grid->data[ij] < z_extreme[0]) {
			z_extreme[0] = Grid->data[ij];
			i_minmax[0]  = ij;
		}
		if (Grid->data[ij] > z_extreme[1]) {
			z_extreme[1] = Grid->data[ij];
			i_minmax[1]  = ij;
		}
	}
	for (i = 0; i < 2; i++) {	/* 0 is min, 1 is max */
		xyz[i][GMT_X] = gmt_M_grd_col_to_x (GMT, gmt_M_col (Grid->header, i_minmax[i]), Grid->header);
		xyz[i][GMT_Y] = gmt_M_grd_row_to_y (GMT, gmt_M_row (Grid->header, i_minmax[i]), Grid->header);
		xyz[i][GMT_Z] = z_extreme[i];
	}
}

void gmt_grd_detrend (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, unsigned mode, double *coeff) {
	/* mode = 0 (GMT_FFT_REMOVE_NOTHING):  Do nothing.
	 * mode = 1 (GMT_FFT_REMOVE_MEAN):  Remove the mean value (returned via a[0])
	 * mode = 2 (GMT_FFT_REMOVE_MID):   Remove the mid value value (returned via a[0])
	 * mode = 3 (GMT_FFT_REMOVE_TREND): Remove the best-fitting plane by least squares (returned via a[0-2])
	 *
	 * Note: The grid may be complex and contain real, imag, or both components.  The data should
	 * be in serial layout so we may loop over the components and do our thing.  Only the real
	 * components coefficients are returned.
	 */

	unsigned int col, row, one_or_zero, start_component = 0, stop_component = 0, component;
	uint64_t ij, offset;
	bool is_complex = false;
	double x_half_length, one_on_xhl, y_half_length, one_on_yhl;
	double sumx2, sumy2, data_var_orig = 0.0, data_var = 0.0, var_redux, x, y, z, a[3];
	char *comp[2] = {"real", "imaginary"};
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (Grid->header);

	gmt_M_memset (coeff, 3, double);

	if (HH->trendmode != GMT_FFT_REMOVE_NOTHING) {	/* Already removed the trend */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Grid has already been detrending - no action taken\n");
		return;
	}
	if (mode == GMT_FFT_REMOVE_NOTHING) {	/* Do nothing */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "No detrending selected\n");
		return;
	}
	HH->trendmode = mode;	/* Update grid header */
	if (HH->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Demultiplexing complex grid before detrending can take place.\n");
		gmt_grd_mux_demux (GMT, Grid->header, Grid->data, GMT_GRID_IS_SERIAL);
	}

	if (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) {	/* Complex grid */
		is_complex = true;
		start_component = (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) ? 0 : 1;
		stop_component  = (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG) ? 1 : 0;
	}

	for (component = start_component; component <= stop_component; component++) {	/* Loop over 1 or 2 components */
		offset = component * Grid->header->size / 2;	/* offset to start of this component in grid */
		gmt_M_memset (a, 3, double);
		if (mode == GMT_FFT_REMOVE_MEAN) {	/* Remove mean */
			for (row = 0; row < Grid->header->n_rows; row++) for (col = 0; col < Grid->header->n_columns; col++) {
				ij = gmt_M_ijp (Grid->header,row,col) + offset;
				z = Grid->data[ij];
				a[0] += z;
				data_var_orig += z * z;
			}
			a[0] /= Grid->header->nm;
			for (row = 0; row < Grid->header->n_rows; row++) for (col = 0; col < Grid->header->n_columns; col++) {
				ij = gmt_M_ijp (Grid->header,row,col) + offset;
				Grid->data[ij] -= (gmt_grdfloat)a[0];
				z = Grid->data[ij];
				data_var += z * z;
			}
			var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
			if (is_complex)
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Mean value removed from %s component: %.8g Variance reduction: %.2f\n",
				            comp[component], a[0], var_redux);
			else
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Mean value removed: %.8g Variance reduction: %.2f\n", a[0], var_redux);
		}
		else if (mode == GMT_FFT_REMOVE_MID) {	/* Remove mid value */
			double zmin = DBL_MAX, zmax = -DBL_MAX;
			for (row = 0; row < Grid->header->n_rows; row++) for (col = 0; col < Grid->header->n_columns; col++) {
				ij = gmt_M_ijp (Grid->header,row,col) + offset;
				z = Grid->data[ij];
				data_var_orig += z * z;
				if (z < zmin) zmin = z;
				if (z > zmax) zmax = z;
			}
			a[0] = 0.5 * (zmin + zmax);	/* Mid value */
			for (row = 0; row < Grid->header->n_rows; row++) for (col = 0; col < Grid->header->n_columns; col++) {
				ij = gmt_M_ijp (Grid->header,row,col) + offset;
				Grid->data[ij] -= (gmt_grdfloat)a[0];
				z = Grid->data[ij];
				data_var += z * z;
			}
			var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
			if (is_complex)
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Mid value removed from %s component: %.8g Variance reduction: %.2f\n",
				            comp[component], a[0], var_redux);
			else
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Mid value removed: %.8g Variance reduction: %.2f\n", a[0], var_redux);
		}
		else {	/* Here we wish to remove a LS plane */

			/* Let plane be z = a0 + a1 * x + a2 * y.  Choose the
			   center of x,y coordinate system at the center of
			   the array.  This will make the Normal equations
			   matrix G'G diagonal, so solution is trivial.  Also,
			   spend some multiplications on normalizing the
			   range of x,y into [-1,1], to avoid roundoff error.
			 */

			one_or_zero = (Grid->header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;
			x_half_length = 0.5 * (Grid->header->n_columns - one_or_zero);
			one_on_xhl = 1.0 / x_half_length;
			y_half_length = 0.5 * (Grid->header->n_rows - one_or_zero);
			one_on_yhl = 1.0 / y_half_length;

			sumx2 = sumy2 = data_var = 0.0;

			for (row = 0; row < Grid->header->n_rows; row++) {
				y = one_on_yhl * (row - y_half_length);
				for (col = 0; col < Grid->header->n_columns; col++) {
					x = one_on_xhl * (col - x_half_length);
					ij = gmt_M_ijp (Grid->header,row,col) + offset;
					z = Grid->data[ij];
					data_var_orig += z * z;
					a[0] += z;
					a[1] += z*x;
					a[2] += z*y;
					sumx2 += x*x;
					sumy2 += y*y;
				}
			}
			a[0] /= Grid->header->nm;
			a[1] /= sumx2;
			a[2] /= sumy2;
			for (row = 0; row < Grid->header->n_rows; row++) {
				y = one_on_yhl * (row - y_half_length);
				for (col = 0; col < Grid->header->n_columns; col++) {
					ij = gmt_M_ijp (Grid->header,row,col) + offset;
					x = one_on_xhl * (col - x_half_length);
					Grid->data[ij] -= (gmt_grdfloat)(a[0] + a[1]*x + a[2]*y);
					data_var += (Grid->data[ij] * Grid->data[ij]);
				}
			}
			var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
			data_var = sqrt (data_var / (Grid->header->nm - 1));
			/* Rescale a1,a2 into user's units, in case useful later */
			a[1] *= (2.0 / (Grid->header->wesn[XHI] - Grid->header->wesn[XLO]));
			a[2] *= (2.0 / (Grid->header->wesn[YHI] - Grid->header->wesn[YLO]));
			if (is_complex)
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Plane removed from %s component. Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g Variance reduction: %.2f\n",
				            comp[component], a[0], data_var, a[1], a[2], var_redux);
			else
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Plane removed.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g Variance reduction: %.2f\n",
				            a[0], data_var, a[1], a[2], var_redux);
		}
		if (component == 0) gmt_M_memcpy (coeff, a, 3, double);	/* Return the real component results */
	}
}

bool gmtlib_init_complex (struct GMT_GRID_HEADER *header, unsigned int complex_mode, uint64_t *imag_offset) {
	/* Sets complex-related parameters based on the input complex_mode variable:
	 * If complex_mode & GMT_GRID_NO_HEADER then we do NOT want to write a header [output only; only some formats]
	 * If grid is the imaginary components of a complex grid then we compute the offset
	 * from the start of the complex array where the first imaginary value goes, using the serial arrangement.
	 */
	bool do_header = !(complex_mode & GMT_GRID_NO_HEADER);	/* Want no header if this bit is set */
	/* Imaginary components are stored after the real components if complex */
	*imag_offset = (complex_mode & GMT_GRID_IS_COMPLEX_IMAG) ? header->size / 2ULL : 0ULL;

	return (do_header);
}

/* Reverses the grid vertically, that is, from north up to south up or vice versa. */
void gmt_grd_flip_vertical (void *gridp, const unsigned n_cols32, const unsigned n_rows32, const unsigned n_stride32, size_t cell_size) {
	/* Note: when grid is complex, pass 2x n_rows */
	size_t row, n_cols = n_cols32, n_rows = n_rows32;
	size_t rows_over_2 = (size_t) floor (n_rows / 2.0);
	size_t stride = n_cols;	/* stride is the distance between rows. defaults to n_cols */
	char *grid = (char*)gridp;
	char *tmp = calloc (n_cols, cell_size);
	char *top, *bottom;

	if (n_stride32 != 0)
		stride = (size_t)n_stride32;

	for (row = 0; row < rows_over_2; ++row) {
		/* pointer to top row: */
		top = grid + row * stride * cell_size;
		/* pointer to bottom row: */
		bottom = grid + ( (n_rows - row) * stride - stride ) * cell_size;
		memcpy (tmp, top, n_cols * cell_size);    /* save top row */
		memcpy (top, bottom, n_cols * cell_size); /* copy bottom to top */
		memcpy (bottom, tmp, n_cols * cell_size); /* copy tmp to bottom */
	}
	gmt_M_str_free (tmp);
}

bool gmtlib_check_url_name (char *fname) {
	/* File names starting as below should not be tested for existence or reading permissions as they
	   are either meant to be accessed on the fly (http & ftp) or they are compressed. So, if any of
	   the conditions holds true, returns true. All cases are read via GDAL support or other. */
	if (gmt_M_file_is_url (fname) ||
	    !strncmp(fname,"/vsizip/", 8)  ||
	    !strncmp(fname,"/vsigzip/",9)  ||
	    !strncmp(fname,"/vsicurl/",9)  ||
	    !strncmp(fname,"/vsimem/", 8)  ||
	    !strncmp(fname,"/vsitar/", 8)) {
#ifdef WIN32
		/* On Windows libcurl does not care about the cerificates file (see https://github.com/curl/curl/issues/1538)
		   and would fail, so no choice but prevent certificates verification.
		   However, a CURL_CA_BUNDLE=/path/to/curl-ca-bundle.crt will overcome this and will consult the crt file. */
		if (!getenv ("GDAL_HTTP_UNSAFESSL") && !getenv("CURL_CA_BUNDLE"))
			putenv ("GDAL_HTTP_UNSAFESSL=YES");
#endif
		return true;
	}
	else
		return false;
}

unsigned int gmtlib_expand_headerpad (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, double *new_wesn, unsigned int *orig_pad, double *orig_wesn) {
	unsigned int tmp_pad[4] = {0, 0, 0, 0}, delta[4] = {0, 0, 0, 0}, k = 0;
	/* When using subset with memory grids we cannot actually cut the grid but instead
	 * must temporarily change the pad to match the desired inner region wesn.  This means
	 * the pads will change and can be quite large. */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	gmt_M_memcpy (tmp_pad, h->pad, 4, unsigned int);	/* Initialize new pad to the original pad */
	/* First determine which (and how many, k) of the 4 new boundaries are inside the original region and update the padding: */
	if (new_wesn[XLO] > h->wesn[XLO]) k++, tmp_pad[XLO] += urint ((new_wesn[XLO] - h->wesn[XLO]) * HH->r_inc[GMT_X]);
	if (new_wesn[XHI] < h->wesn[XHI]) k++, tmp_pad[XHI] += urint ((h->wesn[XHI] - new_wesn[XHI]) * HH->r_inc[GMT_X]);
	if (new_wesn[YLO] > h->wesn[YLO]) k++, tmp_pad[YLO] += urint ((new_wesn[YLO] - h->wesn[YLO]) * HH->r_inc[GMT_Y]);
	if (new_wesn[YHI] < h->wesn[YHI]) k++, tmp_pad[YHI] += urint ((h->wesn[YHI] - new_wesn[YHI]) * HH->r_inc[GMT_Y]);
	if (k) {	/* Yes, pad will change since region is different for k of the 4 sides */
		for (k = 0; k < 4; k++) delta[k] = tmp_pad[k] - h->pad[k];	/* Columns with data being passed as padding */
		gmt_M_memcpy (orig_pad, h->pad, 4, unsigned int);	/* Place the original grid pad in the provided array */
		gmt_M_memcpy (orig_wesn, h->wesn, 4, double);		/* Place the original grid wesn in the provided array */
		gmt_M_memcpy (h->pad, tmp_pad, 4, unsigned int);	/* Place the new pad in the grid header */
		gmt_M_memcpy (h->wesn, new_wesn, 4, double);		/* Place the new wesn in the grid header */
		gmt_set_grddim (GMT, h);	/* This recomputes n_columns|n_rows. */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmtlib_expand_headerpad: %d pad sides changed. Now %u/%u/%u/%u\n",
		            k, h->pad[XLO], h->pad[XHI], h->pad[YLO], h->pad[YHI]);
		for (k = 0; k < 4; k++) {	/* If pad now contains data then change the BC to reflect this */
			if (delta[k] >= orig_pad[k]) HH->BC[k] = GMT_BC_IS_DATA;
		}
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmtlib_expand_headerpad: No pad adjustment needed\n");
	return k;
}

void gmtlib_contract_headerpad (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int *orig_pad, double *orig_wesn) {
	/* When using subset with memory grids we must reset the pad back to the original setting when done */
	if (h == NULL) return;	/* Nothing for us to work with */
	gmt_M_memcpy (h->pad, orig_pad, 4, unsigned int);	/* Place the original pad in the grid header */
	gmt_M_memcpy (h->wesn, orig_wesn, 4, double);		/* Place the orig_pad wesn in the grid header */
	gmt_set_grddim (GMT, h);	/* This recomputes n_columns|n_rows. */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmtlib_contract_headerpad: Pad and wesn reset to original values\n");
}

void gmtlib_contract_pad (struct GMT_CTRL *GMT, void *object, int family, unsigned int *orig_pad, double *orig_wesn) {
	/* When using subset with memory grids we must reset the pad back to the original setting when done */
	struct GMT_GRID_HEADER *h = NULL;
	if (family == GMT_IS_GRID) {
		struct GMT_GRID *G = grdio_get_grid_data (object);
		if (G) h = G->header;
	}
	else if (family == GMT_IS_IMAGE) {
		struct GMT_IMAGE *I = grdio_get_image_data (object);
		if (I) h = I->header;
	}
	gmtlib_contract_headerpad (GMT, h, orig_pad, orig_wesn);
}

GMT_LOCAL int get_extension_period (char *file) {
	int i, pos_ext = 0;
	for (i = (int)strlen(file) - 1; i > 0; i--) {
		if (file[i] == '.') { 	/* Beginning of file extension */
			pos_ext = i;
			break;
		}
	}
	return (pos_ext);
}

int gmt_raster_type (struct GMT_CTRL *GMT, char *file) {
	/* Returns the type of the file (either grid or image).
	 * We use the file extension to make these decisions:
	 * GMT_IS_IMAGE: In this context, this means a plain image
	 *	that does not have any georeference information in it.
	 *	The image types that fall into this category are
	 *	GIF, JPG, RAS, PNG, BMP, WEBP, PBM, RGB.
	 *	These are all detected by looking for magic bytes.
	 * GMT_IS_GRID: In this context, this means file has an extension
	 *	that could be used by images or images with geospatial
	 *	data and even floating point values, such as geotiff.
	 *	THus the list of types in this category are
	 *	TIF, JP2, IMG (ERDAS).  These are detected via magic bytes.
	 * GMT_NOTSET: This covers anything that fails to land in the
	 *	other two categories.
	 */
	FILE *fp = NULL;
	unsigned char data[16] = {""};
	char *F = NULL, *p = NULL, path[PATH_MAX] = {""};
	int j, code, pos_ext;

	if (!file) return (GMT_ARG_IS_NULL);	/* Gave nothing */
	if (gmt_M_file_is_cache (file) || gmt_M_file_is_url (file)) {	/* Must download, then modify the name */
		j = gmt_download_file_if_not_found (GMT, file, 0);
		F = strdup (&file[j]);
	}
	else
		F = strdup (file);

	if ((p = strstr(F, "=gd")) != NULL) *p = '\0';	/* Chop off any =gd<stuff> so that the opening test doesn't fail */
	if (!gmt_getdatapath (GMT, F, path, R_OK)) {
		gmt_M_str_free (F);
		return GMT_GRDIO_FILE_NOT_FOUND;
	}
	pos_ext = get_extension_period (path) + 1;	/* Start of extension */
	if ((fp = fopen (path, "rb")) == NULL) {
		gmt_M_str_free (F);
		return (GMT_ERROR_ON_FOPEN);
	}
	gmt_M_str_free (F);
	if (gmt_M_fread (data, sizeof (unsigned char), 16, fp) < 16) {
		fclose (fp);
		return (GMT_GRDIO_READ_FAILED);	/* Failed to get one row */
	}
	fclose (fp);

	/* Different magic chars for different image formats:
	   .jpg:  FF D8 FF
	   .png:  89 50 4E 47 0D 0A 1A 0A
	   .gif:  GIF87a
	          GIF89a
	   .bmp:  BM
	   .webp: RIFF ???? WEBP
	   .ras   59 A6 6A 95
	   .pbm   P 1-6
	   .rgb   01 da
	   .tif   49 49 2A 00 or 4D 4D 00 2A
	   .jp2   00 00 00 0C 6A 50 20 20 0D 0A 87 0A
	   .img   EHFA_HEADER_TAG
	   .ige   ERDAS_IMG_EXTERNAL_RASTER
	   .fits  53 49 4d 50 4c 45
	   .ewc   06 02 01 02
 	*/

	switch (data[0]) {
		case 0x00:	/* JP2 */
			code = ( !strncmp( (const char *)data, "\x00\x00\x00\x0C\x6A\x50\x20\x20\x0d\x0a\x87\x0a", 12 )) ? GMT_IS_GRID : GMT_NOTSET;	break;

		case 0x01:	/* SGI Iris */
			code = (( data[1] == 0xda )) ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 0x06:	/* EWC */
			code = ( !strncmp( (const char *)data, "\x06\x02\x01\x02", 4 )) ? GMT_IS_GRID : GMT_NOTSET;	break;

		case 0x53:	/* FITS */
			code = ( !strncmp( (const char *)data, "\x53\x49\x4d\x50\x4c\x45", 6 )) ? GMT_IS_GRID : GMT_NOTSET;	break;

		case 0x59:	/* Sun raster */
			code =  ( !strncmp( (const char *)data, "\x59\xA6\x6A\x95", 4 )) ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 0x77:	/* OZI */
			if (data[1] == 0x80 && gmt_strlcmp (&path[pos_ext], "ozfx3"))
				code = GMT_IS_GRID;
			else if (data[1] == 0x78 && gmt_strlcmp (&path[pos_ext], "ozf2"))
				code = GMT_IS_GRID;
			else
				code = GMT_NOTSET;
			break;
		case 0x89:	/* PNG */
			code = ( !strncmp( (const char *)data, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8 )) ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 0xFF:	/* JPG */
			code = ( !strncmp( (const char *)data, "\xFF\xD8\xFF", 3 )) ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 'B':	/* BMP */
			code = (( data[1] == 'M' && gmt_strlcmp (&path[pos_ext], "bmp"))) ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 'E':	/* IMG or IGE */
			code = ( !strncmp( (const char *)data, "EHFA_HEADER_TAG", 15 ) || !strncmp( (const char *)data, "ERDAS_IMG_EXTER", 15 ) ) ? GMT_IS_GRID : GMT_NOTSET;	break;

		case 'G':	/* GIF */
			code = ( !strncmp( (const char *)data, "GIF87a", 6 ) || !strncmp( (const char *)data, "GIF89a", 6 ) ) ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 'I':	/* TIF */
			code = ( !strncmp( (const char*)data, "\x49\x49\x2A\x00", 4 )) ? GMT_IS_GRID : GMT_NOTSET;	break;

		case 'M':	/* TIF */
			code = ( !strncmp( (const char*)data, "\x4D\x4D\x00\x2A", 4 )) ? GMT_IS_GRID : GMT_NOTSET;	break;

		case 'P':	/* PPM (also check that extension starts with p since the magic bytes are a bit weak ) */
			code = (data[1] >= '1' && data[1] <= '6' && tolower (path[pos_ext]) == 'p') ? GMT_IS_IMAGE : GMT_NOTSET;	break;

		case 'R':	/* Google WEBP */
			if ( !strncmp ((const char *)data, "RIFF", 4) || !strncmp ((const char *)(data+8), "WEBP", 4))
				code = GMT_IS_IMAGE;
			else
				code = GMT_NOTSET;
			break;

		default:	/* Just consider file extensions at this point */
			/* .sid (MrSID), .ecw (ECW), .kap (BSB), .gen (ADRG), .map (OZI) */
			if (gmt_strlcmp (&path[pos_ext], "sid") || gmt_strlcmp (&path[pos_ext], "ecw") ||
			    gmt_strlcmp (&path[pos_ext], "kap") || gmt_strlcmp (&path[pos_ext], "gen") ||
				gmt_strlcmp (&path[pos_ext], "map"))
				code = GMT_IS_GRID;
			else
				code =  GMT_NOTSET;
			break;
	}

	if (code == GMT_IS_IMAGE)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s considered a valid image instead of grid. Open via GDAL\n", file);
	else if (code == GMT_IS_GRID)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s may be image or grid.  Open via GDAL for checking\n", file);
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s is most likely a grid. Open in GMT as grid\n", file);

	return code;
}

int gmt_img_sanitycheck (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	/* Make sure that img Mercator grids are not used with map projections for plotting */

	if (strncmp (h->remark, "Spherical Mercator Projected with -Jm1 -R", 41U)) return GMT_NOERROR;	/* Not a Mercator img grid since missing the critical remark format */
	if (h->registration == GMT_GRID_NODE_REG) return GMT_NOERROR;		/* Cannot be a Mercator img grid since they are pixel registered */
	if (GMT->current.proj.projection == GMT_LINEAR) return GMT_NOERROR;	/* Only linear projection is allowed with this projected grid */
	GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot use a map projection with an already projected grid (spherical Mercator img grid).  Use -Jx or -JX.\n");
	return GMT_PROJECTION_ERROR;
}

#ifdef HAVE_GDAL
GMT_LOCAL void gdal_free_from (struct GMT_CTRL *GMT, struct GMT_GDALREAD_OUT_CTRL *from_gdalread) {
	int i;
	if (from_gdalread->band_field_names) {
		for (i = 0; i < from_gdalread->RasterCount; i++ )
			if (from_gdalread->band_field_names[i].DataType)
				gmt_M_str_free (from_gdalread->band_field_names[i].DataType);	/* Those were allocated with strdup */
		gmt_M_free (GMT, from_gdalread->band_field_names);
	}
	if (from_gdalread->ColorMap) gmt_M_free (GMT, from_gdalread->ColorMap);	/* Maybe we will have a use for this in future, but not yet */
}

int gmtlib_read_image_info (struct GMT_CTRL *GMT, char *file, bool must_be_image, struct GMT_IMAGE *I) {
	size_t k;
	char *p;
	double dumb;
	struct GMT_GDALREAD_IN_CTRL *to_gdalread = NULL;
	struct GMT_GDALREAD_OUT_CTRL *from_gdalread = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (I->header);

	/* Allocate new control structures */
	to_gdalread   = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALREAD_IN_CTRL);
	from_gdalread = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALREAD_OUT_CTRL);

	to_gdalread->M.active = true;	/* Get metadata only */

	k = strlen (file) - 1;
	while (k && file[k] && file[k] != '+') k--;	/* See if we have a band request */
	if (k && file[k+1] == 'b') {
		/* Yes we do. Put the band string into the 'pocket' where gmtlib_read_image will look and finish the request */
		HH->pocket = strdup (&file[k+2]);
		file[k] = '\0';
	}
	if ((p = strstr (file, "=gd")) != NULL) *p = '\0';	/* Chop off any =<stuff> */

	if (gmt_gdalread (GMT, file, to_gdalread, from_gdalread)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading image with gdalread.\n");
		gmt_M_free (GMT, to_gdalread);
		gdal_free_from (GMT, from_gdalread);
		gmt_M_free (GMT, from_gdalread);
		return (GMT_GRDIO_READ_FAILED);
	}
	if (must_be_image && from_gdalread->band_field_names != NULL && strcmp(from_gdalread->band_field_names[0].DataType, "Byte")) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Using data type other than byte (unsigned char) is not implemented\n");
		gmt_M_free (GMT, to_gdalread);
		gdal_free_from (GMT, from_gdalread);
		gmt_M_free (GMT, from_gdalread);
		return (GMT_NOT_A_VALID_TYPE);
	}

	I->color_interp    = from_gdalread->color_interp;     /* Must find out how to release this mem */
	I->n_indexed_colors = from_gdalread->nIndexedColors;
	gmt_M_str_free (I->header->ProjRefPROJ4);		/* Make sure we don't leak due to a previous copy */
	gmt_M_str_free (I->header->ProjRefWKT);
	I->header->ProjRefPROJ4 = from_gdalread->ProjRefPROJ4;
	I->header->ProjRefWKT   = from_gdalread->ProjRefWKT;
	I->header->inc[GMT_X] = from_gdalread->hdr[7];
	I->header->inc[GMT_Y] = from_gdalread->hdr[8];
	I->header->n_columns = from_gdalread->RasterXsize;
	I->header->n_rows = from_gdalread->RasterYsize;
	I->header->n_bands = from_gdalread->RasterCount;
	I->header->registration = (int)from_gdalread->hdr[6];
	I->header->wesn[XLO] = from_gdalread->Corners.LL[0];
	I->header->wesn[XHI] = from_gdalread->Corners.UR[0];
	I->header->wesn[YLO] = from_gdalread->Corners.LL[1];
	I->header->wesn[YHI] = from_gdalread->Corners.UR[1];
	if (I->header->wesn[YHI] < I->header->wesn[YLO]) {	/* Simple images have that annoying origin at UL and y positive down */
		dumb = I->header->wesn[YHI];
		I->header->wesn[YHI] = I->header->wesn[YLO];
		I->header->wesn[YLO] = dumb;
	}
	HH->grdtype = gmtlib_get_grdtype (GMT, GMT_IN, I->header);

	gmt_set_grddim (GMT, I->header);		/* This recomputes n_columns|n_rows. Dangerous if -R is not compatible with inc */
	GMT_Set_Index (GMT->parent, I->header, GMT_IMAGE_LAYOUT);

	gmt_M_free (GMT, to_gdalread);
	gdal_free_from (GMT, from_gdalread);
	gmt_M_free (GMT, from_gdalread);

	return (GMT_NOERROR);
}

int gmtlib_read_image (struct GMT_CTRL *GMT, char *file, struct GMT_IMAGE *I, double *wesn, unsigned int *pad, unsigned int complex_mode) {
	/* file:	- IGNORED -
	 * image:	array with final image
	 * wesn:	Sub-region to extract  [Use entire file if NULL or contains 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of image, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	int    i;
	bool   expand;
	char   strR[GMT_LEN128];
	struct GRD_PAD P;
	struct GMT_GDALREAD_IN_CTRL  *to_gdalread = NULL;
	struct GMT_GDALREAD_OUT_CTRL *from_gdalread = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (I->header);
	gmt_M_unused(complex_mode);

	expand = grdio_padspace (GMT, I->header, wesn, pad, &P);	/* true if we can extend the region by the pad-size to obtain real data for BC */

	/*gmt_M_err_trap ((*GMT->session.readgrd[header->type]) (GMT, header, image, P.wesn, P.pad, complex_mode));*/

	/* Allocate new control structures */
	to_gdalread   = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALREAD_IN_CTRL);
	from_gdalread = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALREAD_OUT_CTRL);

	if (GMT->common.R.active[RSET]) {
		snprintf (strR, GMT_LEN128, "%.10f/%.10f/%.10f/%.10f", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI],
		          GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		to_gdalread->R.region = strR;
		/*to_gdalread->R.active = true;*/	/* Wait until we really know how to use it */
	}

	if (HH->pocket) {				/* See if we have a band request */
		to_gdalread->B.active = true;
		to_gdalread->B.bands = HH->pocket;	/* Band parsing and error testing is done in gmt_gdalread */
	}

	to_gdalread->p.pad = (int)pad[0];	/* Only 'square' padding allowed */
	to_gdalread->p.active = (pad[0] > 0);
	to_gdalread->I.active = true;		/* Means that image in I->data will be BIP interleaved */

	/* Tell gmt_gdalread that we already have the memory allocated and send in the *data pointer */
	to_gdalread->c_ptr.active = true;
	to_gdalread->c_ptr.grd = I->data;

	if (gmt_gdalread (GMT, file, to_gdalread, from_gdalread)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading image with gdalread.\n");
		gmt_M_free (GMT, to_gdalread);
		for (i = 0; i < from_gdalread->RasterCount; i++)
			gmt_M_str_free (from_gdalread->band_field_names[i].DataType);	/* Those were allocated with strdup */
		gmt_M_free (GMT, from_gdalread->band_field_names);
		gmt_M_free (GMT, from_gdalread);
		return (GMT_GRDIO_READ_FAILED);
	}

	if (to_gdalread->O.mem_layout[0]) 	/* If a different mem_layout request was applied in gmt_gdalread than we must update */
		gmt_strncpy(I->header->mem_layout, to_gdalread->O.mem_layout, 4);

	if (to_gdalread->B.active) gmt_M_str_free (HH->pocket);		/* It was allocated by strdup. Free it for an eventual reuse. */

	I->colormap = from_gdalread->ColorMap;
	I->n_indexed_colors  = from_gdalread->nIndexedColors;
	I->header->n_bands = from_gdalread->nActualBands;	/* What matters here on is the number of bands actually read */

	if (expand) {	/* Must undo the region extension and reset n_columns, n_rows */
		I->header->n_columns -= (int)(P.pad[XLO] + P.pad[XHI]);
		I->header->n_rows -= (int)(P.pad[YLO] + P.pad[YHI]);
		gmt_M_memcpy (I->header->wesn, wesn, 4, double);
		I->header->nm = gmt_M_get_nm (GMT, I->header->n_columns, I->header->n_rows);
	}
	gmt_M_grd_setpad (GMT, I->header, pad);	/* Copy the pad to the header */

	gmt_M_free (GMT, to_gdalread);
	for (i = 0; i < from_gdalread->RasterCount; i++)
		gmt_M_str_free (from_gdalread->band_field_names[i].DataType);	/* Those were allocated with strdup */
	gmt_M_free (GMT, from_gdalread->band_field_names);
	gmt_M_free (GMT, from_gdalread);
	gmt_BC_init (GMT, I->header);	/* Initialize image interpolation and boundary condition parameters */

	return (GMT_NOERROR);
}
#endif
