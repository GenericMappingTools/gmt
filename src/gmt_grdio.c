/*--------------------------------------------------------------------
 * $Id$
 *
 * Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Contact info: gmt.soest.hawaii.edu
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
 * with 32-bit ints.  However, 1-D array indeces (e.g., ij = row*nx + col) are
 * addressed with 64-bit integers.
 *
 * GMT functions include:
 *
 *  For i/o on regular GMT grids:
 *  GMT_grd_get_format :   Get format id, scale, offset and missing value for grdfile
 *  GMT_read_grd_info :    Read header from file
 *  GMT_read_grd :         Read data set from file (must be preceded by GMT_read_grd_info)
 *  GMT_update_grd_info :  Update header in existing file (must be preceded by GMT_read_grd_info)
 *  GMT_write_grd_info :   Write header to new file
 *  GMT_write_grd :        Write header and data set to new file
 *
 *  For special img and (via GDAL) reading:
 *  GMT_read_img           Read [subset from] a Sandwell/Smith *.img file
 *  GMT_read_image         GDAL: Read [subset of] an image via GDAL
 *  GMT_read_image_info    GDAL: Get information for an image via GDAL
 *
 * Additional supporting grid routines:
 *
 *  GMT_grd_init           Initialize grd header structure
 *  GMT_grd_shift          Rotates grdfiles in x-direction
 *  GMT_grd_setregion      Determines subset coordinates for grdfiles
 *  GMT_grd_is_global      Determine whether grid is "global", i.e. longitudes are periodic
 *  GMT_adjust_loose_wesn  Ensures region, increments, and nx/ny are compatible
 *  GMT_decode_grd_h_info  Decodes a -Dstring into header text components
 *  GMT_grd_RI_verify      Test to see if region and incs are compatible
 *  GMT_scale_and_offset_f Routine that scales and offsets the data in a vector
 *  GMT_pack_grid          Packs or unpacks a grid by calling GMT_scale_and_offset_f()
 *
 * All functions that begin with lower case gmt_* are private to this file only.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt_dev.h"
#include "gmt_internals.h"
#include "common_byteswap.h"

struct GRD_PAD {
	double wesn[4];
	unsigned int pad[4];
};

/* These functions live in other files and are extern'ed in here */
int gmt_nc_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, char job);
int gmt_cdf_grd_info (struct GMT_CTRL *GMT, int ncid, struct GMT_GRID_HEADER *header, char job);
int GMT_is_nc_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_is_native_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_is_ras_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_is_srf_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_is_mgg2_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_is_agc_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_is_esri_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
#ifdef HAVE_GDAL
int GMT_is_gdal_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
#endif

EXTERN_MSC void gmt_close_grd (struct GMT_CTRL *GMT, struct GMT_GRID *G);

/* GENERIC I/O FUNCTIONS FOR GRIDDED DATA FILES */
#ifdef DEBUG
//#define GMT_DUMPING	/* Uncomment this to have grd_dump be called and do something */
#ifdef GMT_DUMPING
void grd_dump (struct GMT_GRID_HEADER *header, float *grid, bool is_complex, char *txt)
{
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
void grd_dump (struct GMT_GRID_HEADER *header, float *grid, bool is_complex, char *txt)
{
	GMT_UNUSED(header); GMT_UNUSED(grid); GMT_UNUSED(is_complex); GMT_UNUSED(txt);
	/* Nothing */
}
#endif
#endif

int gmt_grd_layout (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, unsigned int complex_mode, unsigned int direction)
{	/* Checks or sets the array arrangement for a complex array */
	size_t needed_size;	/* Space required to hold both components of a complex grid */

	if ((complex_mode & GMT_GRID_IS_COMPLEX_MASK) == 0) return GMT_OK;	/* Regular, non-complex grid, nothing special to do */

	needed_size = 2ULL * header->mx * header->my;	/* For the complex array */
	if (header->size < needed_size) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal Error: Complex grid not large enough to hold both components!.\n");
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	if (direction == GMT_IN) {	/* About to read in a complex component; another one might have been read in earlier */
		if (header->arrangement == GMT_GRID_IS_INTERLEAVED) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Demultiplexing complex grid before reading can take place.\n");
			GMT_grd_mux_demux (GMT, header, grid, GMT_GRID_IS_SERIAL);
		}
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == GMT_GRID_IS_COMPLEX_MASK) {	/* Already have both component; this will overwrite one of them */
			unsigned int type = (complex_mode & GMT_GRID_IS_COMPLEX_REAL) ? 0 : 1;
			char *kind[2] = {"read", "imaginary"};
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Overwriting previously stored %s component in complex grid.\n", kind[type]);
		}
		header->complex_mode |= complex_mode;	/* Update the grids complex mode */
	}
	else {	/* About to write out a complex component */
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == 0) {	/* Not a complex grid */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal Error: Asking to write out complex components from a non-complex grid.\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		if ((complex_mode & GMT_GRID_IS_COMPLEX_REAL) && (header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) == 0) {
			/* Programming error: Requesting to write real components when there are none */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal Error: Complex grid has no real components that can be written to file.\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		else if ((complex_mode & GMT_GRID_IS_COMPLEX_IMAG) && (header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG) == 0) {
			/* Programming error: Requesting to write imag components when there are none */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal Error: Complex grid has no imaginary components that can be written to file.\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		if (header->arrangement == GMT_GRID_IS_INTERLEAVED) {	/* Must first demultiplex the grid */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Demultiplexing complex grid before writing can take place.\n");
			GMT_grd_mux_demux (GMT, header, grid, GMT_GRID_IS_SERIAL);
		}
	}
	/* header->arrangment might now have been changed accordingly */
	return GMT_OK;
}

void gmt_grd_parse_xy_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, char *file, unsigned int direction)
{	/* Decode the optional +u|U<unit> and determine scales */
	enum GMT_enum_units u_number;
	unsigned int mode = 0;
	char *c = NULL, *name = NULL;

	if (GMT_is_geographic (GMT, direction)) return;	/* Does not apply to geographic data */
	name = (file) ? file : h->name;
	if ((c = GMT_file_unitscale (name)) == NULL) return;	/* Did not find any modifier */
	mode = (c[1] == 'u') ? 0 : 1;
	u_number = GMT_get_unit_number (GMT, c[2]);		/* Convert char unit to enumeration constant for this unit */
	if (u_number == GMT_IS_NOUNIT) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Grid file x/y unit specification %s was unrecognized (part of file name?) and is ignored.\n", c);
		return;
	}
	/* Got a valid unit */
	h->xy_unit_to_meter[direction] = GMT->current.proj.m_per_unit[u_number];	/* Converts unit to meters */
	if (mode) h->xy_unit_to_meter[direction] = 1.0 / h->xy_unit_to_meter[direction];	/* Wanted the inverse */
	h->xy_unit[direction] = u_number;	/* Unit ID */
	h->xy_adjust[direction] |= 1;		/* Says we have successfully parsed and readied the x/y scaling */
	h->xy_mode[direction] = mode;
	c[0] = '\0';	/* Chop off the unit specification from the file name */
}

void gmt_grd_xy_scale (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int direction)
{
	unsigned int k;
	/* Apply the scaling of wesn,inc as given by the header's xy_* settings.
	 * After reading a grid it will have wesn/inc in meters.
	 * Before writing a grid, it may have units changed back to original units
	 * or scaled to anoter set of units */

	if (direction == GMT_IN) {
		if (h->xy_adjust[direction] == 0) return;	/* Nothing to do */
		if (h->xy_adjust[GMT_IN] & 2) return;		/* Already scaled them */
		for (k = 0; k < 4; k++) h->wesn[k] *= h->xy_unit_to_meter[GMT_IN];
		for (k = 0; k < 2; k++) h->inc[k]  *= h->xy_unit_to_meter[GMT_IN];
		h->xy_adjust[GMT_IN] = 2;	/* Now the grid is ready for use and in meters */
		if (h->xy_mode[direction])
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Input grid file x/y unit was converted from meters to %s after reading.\n", GMT->current.proj.unit_name[h->xy_unit[GMT_IN]]);
		else
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Input grid file x/y unit was converted from %s to meters after reading.\n", GMT->current.proj.unit_name[h->xy_unit[GMT_IN]]);
	}
	else if (direction == GMT_OUT) {	/* grid x/y are assumed to be in meters */
		if (h->xy_adjust[GMT_OUT] & 1) {	/* Was given a new unit for output */
			for (k = 0; k < 4; k++) h->wesn[k] /= h->xy_unit_to_meter[GMT_OUT];
			for (k = 0; k < 2; k++) h->inc[k]  /= h->xy_unit_to_meter[GMT_OUT];
			h->xy_adjust[GMT_OUT] = 2;	/* Now we are ready for writing */
			if (h->xy_mode[GMT_OUT])
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Output grid file x/y unit was converted from %s to meters before writing.\n", GMT->current.proj.unit_name[h->xy_unit[GMT_OUT]]);
			else
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Output grid file x/y unit was converted from meters to %s before writing.\n", GMT->current.proj.unit_name[h->xy_unit[GMT_OUT]]);
		}
		else if (h->xy_adjust[GMT_IN] & 2) {	/* Just undo old scaling */
			for (k = 0; k < 4; k++) h->wesn[k] /= h->xy_unit_to_meter[GMT_IN];
			for (k = 0; k < 2; k++) h->inc[k]  /= h->xy_unit_to_meter[GMT_IN];
			h->xy_adjust[GMT_IN] -= 2;	/* Now it is back to where we started */
			if (h->xy_mode[GMT_OUT])
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Output grid file x/y unit was reverted back to %s from meters before writing.\n", GMT->current.proj.unit_name[h->xy_unit[GMT_IN]]);
			else
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Output grid file x/y unit was reverted back from meters to %s before writing.\n", GMT->current.proj.unit_name[h->xy_unit[GMT_IN]]);
		}
	}
}

/* Routines to see if a particular grd file format is specified as part of filename. */

void gmt_expand_filename (struct GMT_CTRL *GMT, char *file, char *fname)
{
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

double * GMT_grd_coord (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, int dir)
{	/* Allocate, compute, and return the x- or y-coordinates for a grid */
	unsigned int k;
	double *coord = NULL;
	assert (dir == GMT_X || dir == GMT_Y);
	if (dir == GMT_X) {
		coord = GMT_memory (GMT, NULL, header->nx, double);
		for (k = 0; k < header->nx; k++) coord[k] = GMT_grd_col_to_x (GMT, k, header);
	}
	else if (dir == GMT_Y) {
		coord = GMT_memory (GMT, NULL, header->ny, double);
		for (k = 0; k < header->ny; k++) coord[k] = GMT_grd_row_to_y (GMT, k, header);
	}

	return (coord);
}

enum Grid_packing_mode {
	k_grd_pack = 0, /* scale and offset before writing to disk */
	k_grd_unpack    /* remove scale and offset after reading packed data */
};

void GMT_pack_grid (struct GMT_CTRL *Ctrl, struct GMT_GRID_HEADER *header, float *grid, unsigned pack_mode) {
	size_t n_representations = 0; /* number of distinct values >= 0 that a signed integral type can represent */

	if (pack_mode == k_grd_pack && (header->z_scale_autoadust || header->z_offset_autoadust)) {
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
		GMT_grd_zminmax (Ctrl, header, grid); /* Calculate z_min/z_max */
		if (header->z_offset_autoadust) {
			/* shift to center values around 0 but shift only by integral value */
			double z_range = header->z_max - header->z_min;
			if (isfinite (z_range))
				header->z_add_offset = rint(z_range / 2.0 + header->z_min);
		}
		if (header->z_scale_autoadust) {
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
			GMT_scale_and_offset_f (Ctrl, grid, header->size, header->z_scale_factor, header->z_add_offset);
			/* Adjust z-range in header: */
			header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
			header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;
			break;
		case k_grd_pack:
			GMT_scale_and_offset_f (Ctrl, grid, header->size, 1.0/header->z_scale_factor, -header->z_add_offset/header->z_scale_factor);
			break;
		default:
			assert (false); /* GMT_pack_grid() called with illegal pack_mode */
	}
}

int parse_grd_format_scale (struct GMT_CTRL *Ctrl, struct GMT_GRID_HEADER *header, char *format) {
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
	int err; /* GMT_err_trap */

	/* decode grid type */
	strncpy (type_code, format, 2);
	type_code[2] = '\0';
	err = GMT_grd_format_decoder (Ctrl, type_code, &header->type); /* update header type id */
	if (err != GMT_NOERROR)
		return err;

	/* parse scale/offset/invalid if any */
	p = strchr (format, '/');
	if (p != NULL && *p) {
		++p;
		/* parse scale */
		if (*p == 'a')
			header->z_scale_autoadust = header->z_offset_autoadust = true;
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
			header->z_offset_autoadust = false;
			sscanf (p, "%lf", &header->z_add_offset);
		}
		else
			header->z_offset_autoadust = true;
	}
	else
		return GMT_NOERROR;

	p = strchr (p, '/');
	if (p != NULL && *p) {
		++p;
		/* parse invalid value */
		sscanf (p, "%f", &header->nan_value);

		/* header->nan_value should be of same type as (float)*grid to avoid
		 * round-off errors. For example, =gd///-3.4028234e+38:gtiff, would fail
		 * otherwise because the GTiff'd NoData values are of type double but the
		 * grid is truncated to float.
		 * Don't allow infitiy: */
		if (!isfinite (header->nan_value))
			header->nan_value = (float)NAN;
	}

	return GMT_NOERROR;
}

void GMT_grd_mux_demux (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *data, unsigned int desired_mode)
{	/* Multiplex and demultiplex complex grids.
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
	 * GMT_grd_mux_demux performs either multiplex or demultiplex, depending on desired_mode.
	 * If grid is not complex then we just return doing nothing.
	 * Note: At this point the grid is mx * my and we visit all the nodes, including the pads.
	 * hence we use header->mx/my and GMT_IJ below.
	 */
	uint64_t row, col, col_1, col_2, left_node_1, left_node_2, offset, ij, ij2;
	float *array = NULL;

	if (! (desired_mode == GMT_GRID_IS_INTERLEAVED || desired_mode == GMT_GRID_IS_SERIAL)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_grd_mux_demux called with inappropriate mode - skipped.\n");
		return;
	}
	if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == 0) return;	/* Nuthin' to do */
	if (header->arrangement == desired_mode) return;				/* Already has the right layout */

	/* In most cases we will actually have RRRRR...______ or _____...IIIII..
	 * which means half the array is empty and it is easy to shuffle. However,
	 * in the case with actual RRRR...IIII there is no simple way to do this inplace;
	 * see http://stackoverflow.com/questions/1777901/array-interleaving-problem */

	if (desired_mode == GMT_GRID_IS_INTERLEAVED) {	/* Must multiplex the grid */
		if ((header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) == GMT_GRID_IS_COMPLEX_MASK) {
			/* Transform from RRRRR...IIIII to RIRIRIRIRI... */
			/* Implement properly later; for now waste memory by duplicating [memory is cheap and plentiful] */
			/* One advantage is that the padding is all zero by virtue of the allocation */
			array = GMT_memory_aligned (GMT, NULL, header->size, float);
			offset = header->size / 2;	/* Position of 1st row in imag portion of RRRR...IIII... */
			for (row = 0; row < header->my; row++) {	/* Going from first to last row */
				for (col = 0; col < header->mx; col++) {
					ij = GMT_IJ (header, row, col);	/* Position of an 'R' in the RRRRR portion */
					ij2 = 2 * ij;
					array[ij2++] = data[ij];
					array[ij2] = data[ij+offset];
				}
			}
			GMT_memcpy (data, array, header->size, float);	/* Overwrite serial array with interleaved array */
			GMT_free (GMT, array);
		}
		else if (header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) {
			/* Here we have RRRRRR..._________ and want R_R_R_R_... */
			for (row = header->my; row > 0; row--) {	/* Going from last to first row */
				left_node_1 = GMT_IJ (header, row-1, 0);	/* Start of row in RRRRR layout */
				left_node_2 = 2 * left_node_1;			/* Start of same row in R_R_R_ layout */
				for (col = header->mx, col_1 = col - 1, col_2 = 2*col - 1; col > 0; col--, col_1--) { /* Go from right to left */
					data[left_node_2+col_2] = 0.0f;	col_2--;	/* Set the Imag component to zero */
					data[left_node_2+col_2] = data[left_node_1+col_1];	col_2--;
				}
			}
		}
		else {
			/* Here we have _____...IIIII and want _I_I_I_I */
			offset = header->size / 2;	/* Position of 1st row in imag portion of ____...IIII... */
			for (row = 0; row < header->my; row++) {	/* Going from first to last row */
				left_node_1 = GMT_IJ (header, row, 0);		/* Start of row in _____IIII layout not counting ____*/
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
			array = GMT_memory_aligned (GMT, NULL, header->size, float);
			offset = header->size / 2;	/* Position of 1st row in imag portion of RRRR...IIII... */
			for (row = 0; row < header->my; row++) {	/* Going from first to last row */
				for (col = 0; col < header->mx; col++) {
					ij = GMT_IJ (header, row, col);	/* Position of an 'R' in the RRRRR portion */
					ij2 = 2 * ij;
					array[ij] = data[ij2++];
					array[ij+offset] = data[ij2];
				}
			}
			GMT_memcpy (data, array, header->size, float);	/* Overwrite interleaved array with serial array */
			GMT_free (GMT, array);
		}
		else if (header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) {
			/* Here we have R_R_R_R_... and want RRRRRR..._______  */
			for (row = 0; row < header->my; row++) {	/* Doing from first to last row */
				left_node_1 = GMT_IJ (header, row, 0);	/* Start of row in RRRRR... */
				left_node_2 = 2 * left_node_1;		/* Start of same row in R_R_R_R... layout */
				for (col_1 = col_2 = 0; col_1 < header->mx; col_1++, col_2 += 2) {
					data[left_node_1+col_1] = data[left_node_2+col_2];
				}
			}
			offset = header->size / 2;			/* Position of 1st _ in RRRR...____ */
			GMT_memset (&data[offset], offset, float);	/* Wipe _____ portion clean */
		}
		else {	/* Here we have _I_I_I_I and want _____...IIIII */
			offset = header->size / 2;	/* Position of 1st row in imag portion of ____...IIII... */
			for (row = header->my; row > 0; row--) {	/* Going from last to first row */
				left_node_1 = GMT_IJ (header, row, 0);	/* Start of row in _____IIII layout not counting ____*/
				left_node_2 = 2 * left_node_1;		/* Start of same row in _I_I_I... layout */
				left_node_1 += offset;			/* Move past length of all ____... */
				for (col = header->mx, col_1 = col - 1, col_2 = 2*col - 1; col > 0; col--, col_1--, col_2 -= 2) { /* Go from right to left */
					data[left_node_1+col_1] = data[left_node_2+col_2];
				}
			}
			GMT_memset (data, offset, float);	/* Wipe leading _____ portion clean */
		}
	}
	header->arrangement = desired_mode;
}

int GMT_grd_get_format (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, bool magic)
{
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
	 *    by seaching in current dir and the various GMT_*DIR paths.
	 */

	size_t i = 0, j;
	int val;
	unsigned int direction = (magic) ? GMT_IN : GMT_OUT;
	char tmp[GMT_BUFSIZ];

	gmt_grd_parse_xy_units (GMT, header, file, direction);	/* Parse and strip xy scaling via +u<unit> modifier */

	gmt_expand_filename (GMT, file, header->name);	/* May append a suffix to header->name */

	/* Must reset scale and invalid value because sometimes headers from input grids are
	 * 'recycled' and used for output grids that may have a different type and z-range: */
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;
	header->nan_value      = (float)NAN;

	i = strcspn (header->name, "="); /* get number of chars until first '=' or '\0' */

	if (header->name[i]) {	/* Reading or writing when =suffix is present: get format type, scale, offset and missing value */
		i++;
		/* parse grid format string: */
		if ((val = parse_grd_format_scale (GMT, header, &header->name[i])) != GMT_NOERROR)
			return val;
		if (header->type == GMT_GRID_IS_GD && header->name[i+2] && header->name[i+2] == '?') {	/* A SUBDATASET request for GDAL */
			char *pch = strstr(&header->name[i+3], "::");
			if (pch) {		/* The file name was omitted within the SUBDATASET. Must put it there for GDAL */
				tmp[0] = '\0';
				strncpy (tmp, &header->name[i+3], pch - &header->name[i+3] + 1);
				strcat (tmp, "\"");	strncat(tmp, header->name, i-1);	strcat(tmp, "\"");
				strcat (tmp, &pch[1]);
				strncpy (header->name, tmp, GMT_LEN256);
			}
			else
				strncpy (header->name, &header->name[i+3], GMT_LEN256);
			magic = 0;	/* We don't want it to try to prepend any path */
		} /* if (header->type == GMT_GRID_IS_GD && header->name[i+2] && header->name[i+2] == '?') */
		else if (header->type == GMT_GRID_IS_GD && header->name[i+2] && header->name[i+2] == '+' && header->name[i+3] == 'b') { /* A Band request for GDAL */
			header->pocket = strdup(&header->name[i+4]);
			header->name[i-1] = '\0';
		}
		else if (header->type == GMT_GRID_IS_GD && header->name[i+2] && strstr(&header->name[i+2], ":")) {
			char *pch;
			pch = strstr(&header->name[i+2], ":");
			header->pocket = strdup(++pch);
			header->name[i-1] = '\0';			/* Done, rip the driver/outtype info from file name */
		}
		else {
			j = (i == 1) ? i : i - 1;
			header->name[j] = 0;
		}
		sscanf (header->name, "%[^?]?%s", tmp, header->varname);    /* Strip off variable name */
		if (magic) {	/* Reading: possibly prepend a path from GMT_[GRID|DATA|IMG]DIR */
			if (header->type != GMT_GRID_IS_GD || !GMT_check_url_name(tmp))	/* Do not try path stuff with Web files (accessed via GDAL) */
				if (!GMT_getdatapath (GMT, tmp, header->name, R_OK))
					return (GMT_GRDIO_FILE_NOT_FOUND);
		}
		else		/* Writing: store truncated pathname */
			strncpy (header->name, tmp, GMT_LEN256);
	} /* if (header->name[i]) */
	else if (magic) {	/* Reading: determine file format automatically based on grid content */
		int choice = 0;
		sscanf (header->name, "%[^?]?%s", tmp, header->varname);    /* Strip off variable name */
#ifdef HAVE_GDAL
		/* Check if file is an URL */
		if (GMT_check_url_name(header->name)) {
			/* Then check for GDAL grid */
			if (GMT_is_gdal_grid (GMT, header) == GMT_NOERROR)
				return (GMT_NOERROR);
		}
#endif
		if (!GMT_getdatapath (GMT, tmp, header->name, R_OK))
			return (GMT_GRDIO_FILE_NOT_FOUND);	/* Possibly prepended a path from GMT_[GRID|DATA|IMG]DIR */
		/* First check if we have a netCDF grid. This MUST be first, because ?var needs to be stripped off. */
		if ((val = GMT_is_nc_grid (GMT, header)) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Continue only when file was a pipe or when nc_open didn't like the file. */
		if (val != GMT_GRDIO_NC_NO_PIPE && val != GMT_GRDIO_OPEN_FAILED)
			return (val);
		/* Then check for native binary GMT grid */
		if ((choice = GMT_is_native_grid (GMT, header)) == GMT_NOERROR)
			return (GMT_NOERROR);
		else if (choice == GMT_GRDIO_NONUNIQUE_FORMAT)
			return (GMT_GRDIO_NONUNIQUE_FORMAT);
		/* Next check for Sun raster grid */
		if (GMT_is_ras_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for Golden Software surfer grid */
		if (GMT_is_srf_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for NGDC GRD98 grid */
		if (GMT_is_mgg2_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for AGC grid */
		if (GMT_is_agc_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
		/* Then check for ESRI grid */
		if (GMT_is_esri_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
#ifdef HAVE_GDAL
		/* Then check for GDAL grid */
		if (GMT_is_gdal_grid (GMT, header) == GMT_NOERROR)
			return (GMT_NOERROR);
#endif
		return (GMT_GRDIO_UNKNOWN_FORMAT);	/* No supported format found */
	}
	else {			/* Writing: get format type, scale, offset and missing value from GMT->current.setting.io_gridfile_format */
		if (sscanf (header->name, "%[^?]?%s", tmp, header->varname) > 1)
			strncpy (header->name, tmp, GMT_LEN256);    /* Strip off variable name */
		/* parse grid format string: */
		if ((val = parse_grd_format_scale (GMT, header, GMT->current.setting.io_gridfile_format)) != GMT_NOERROR)
			return val;
	}
	if (header->type == GMT_GRID_IS_AF)
		header->nan_value = 0.0f; /* NaN value for AGC format */
	return (GMT_NOERROR);
}

void gmt_grd_set_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	/* Set unit strings for grid coordinates x, y and z based on
	   output data types for columns 0, 1, and 2.
	*/
	unsigned int i;
	char *string[3] = {NULL, NULL, NULL}, unit[GMT_GRID_UNIT_LEN80] = {""};
	char date[GMT_LEN16] = {""}, clock[GMT_LEN16] = {""};

	/* Copy pointers to unit strings */
	string[0] = header->x_units;
	string[1] = header->y_units;
	string[2] = header->z_units;

	/* Use input data type as backup fr output data type */
	for (i = 0; i < 3; i++)
		if (GMT->current.io.col_type[GMT_OUT][i] == GMT_IS_UNKNOWN) GMT->current.io.col_type[GMT_OUT][i] = GMT->current.io.col_type[GMT_IN][i];

	/* Catch some anomalies */
	if (GMT->current.io.col_type[GMT_OUT][GMT_X] == GMT_IS_LAT) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Output type for X-coordinate of grid %s is LAT. Replaced by LON.\n", header->name);
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
	}
	if (GMT->current.io.col_type[GMT_OUT][GMT_Y] == GMT_IS_LON) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Output type for Y-coordinate of grid %s is LON. Replaced by LAT.\n", header->name);
		GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	}

	/* Set unit strings one by one based on output type */
	for (i = 0; i < 3; i++) {
		switch (GMT->current.io.col_type[GMT_OUT][i]) {
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
			GMT_format_calendar (GMT, date, clock, &GMT->current.io.date_output, &GMT->current.io.clock_output, false, 1, 0.0);
			sprintf (string[i], "time [%s since %s %s]", unit, date, clock);
			/* Warning for non-double grids */
			if (i == 2 && GMT->session.grdformat[header->type][1] != 'd')
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Use double precision output grid to avoid loss of significance of time coordinate.\n");
			break;
		}
	}
}

void gmt_grd_get_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
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
		GMT_str_tolower (string[i]);

		if ((!strncmp (string[i], "longitude", 9U) || strstr (string[i], "degrees_e")) && (header->wesn[XLO] > -360.0 && header->wesn[XHI] <= 360.0)) {
			/* Input data type is longitude */
			GMT->current.io.col_type[GMT_IN][i] = GMT_IS_LON;
		}
		else if ((!strncmp (string[i], "latitude", 8U) || strstr (string[i], "degrees_n")) && (header->wesn[YLO] >= -90.0 && header->wesn[YHI] <= 90.0)) {
			/* Input data type is latitude */
			GMT->current.io.col_type[GMT_IN][i] = GMT_IS_LAT;
		}
		else if (!strcmp (string[i], "time") || !strncmp (string[i], "time [", 6U)) {
			/* Input data type is time */
			GMT->current.io.col_type[GMT_IN][i] = GMT_IS_RELTIME;
			GMT->current.proj.xyz_projection[i] = GMT_TIME;

			/* Determine coordinates epoch and units (default is internal system) */
			GMT_memcpy (&time_system, &GMT->current.setting.time_system, 1, struct GMT_TIME_SYSTEM);
			units = strchr (string[i], '[');
			if (!units || GMT_get_time_system (GMT, ++units, &time_system) || GMT_init_time_system_structure (GMT, &time_system))
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Time units [%s] in grid not recognised, defaulting to gmt.conf.\n", units);

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

bool GMT_grd_pad_status (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, unsigned int *pad)
{	/* Determines if this grid has padding at all (pad = NULL) OR
	 * if pad is given, determines if the pads are different.
	 * Return codes are:
	 * 1) If pad == NULL:
	 *    false: Grid has zero padding.
	 *    true:  Grid has non-zero padding.
	 * 2) If pad contains the desired pad:
	 *    true:  Grid padding matches pad exactly.
	 *    false: Grid padding failed to match pad exactly.
	 */
	GMT_UNUSED(GMT);
	unsigned int side;

	if (pad) {	/* Determine if the grid's pad differ from given pad (false) or not (true) */
		for (side = 0; side < 4; side++) if (header->pad[side] != pad[side]) return (false);	/* Pads differ */
		return (true);	/* Pads match */
	}
	else {	/* We just want to determine if the grid has padding already (true) or not (false) */
		for (side = 0; side < 4; side++) if (header->pad[side]) return (true);	/* Grid has a pad */
		return (false);	/* Grid has no pad */
	}
}

int gmt_padspace (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double *wesn, unsigned int *pad, struct GRD_PAD *P)
{	/* When padding is requested it is usually used to set boundary conditions based on
	 * two extra rows/columns around the domain of interest.  BCs like natural or periodic
	 * can then be used to fill in the pad.  However, if the domain is taken from a grid
	 * whose full domain exceeds the region of interest we are better off using the extra
	 * data to fill those pad rows/columns.  Thus, this function tries to determine if the
	 * input grid has the extra data we need to fill the BC pad with observations. */
	GMT_UNUSED(GMT);
	bool wrap;
	unsigned int side, n_sides = 0;
	double wesn2[4];

	/* First copy over original settings to the Pad structure */
	GMT_memset (P, 1, struct GRD_PAD);					/* Initialize to zero */
	GMT_memcpy (P->pad, pad, 4, int);					/* Duplicate the pad */
	if (!wesn) return (false);						/* No subset requested */
	if (wesn[XLO] == wesn[XHI] && wesn[YLO] == wesn[YHI]) return (false);	/* Subset not set */
	if (wesn[XLO] == header->wesn[XLO] && wesn[XHI] == header->wesn[XHI] && wesn[YLO] == header->wesn[YLO] && wesn[YHI] == header->wesn[YHI])
		return (false);	/* Subset equals whole area */
	GMT_memcpy (P->wesn, wesn, 4, double);					/* Copy the subset boundaries */
	if (pad[XLO] == 0 && pad[XHI] == 0 && pad[YLO] == 0 && pad[YHI] == 0) return (false);	/* No padding requested */

	/* Determine if data exist for a pad on all four sides.  If not we give up */
	wrap = GMT_grd_is_global (GMT, header);	/* If global wrap then we cannot be outside */
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
	GMT_memcpy (P->wesn, wesn2, 4, double);

	/* Set BC */
	for (side = 0; side < 4; side++) {
		if (P->pad[side] == 0)
			header->BC[side] = GMT_BC_IS_DATA;
	}

	return (true);	/* Return true so the calling function can take appropriate action */
}

int gmt_get_grdtype (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h)
{	/* Determine if grid is Cartesian or geographic, and if so if longitude range is <360, ==360, or >360 */
	if (GMT_x_is_lon (GMT, GMT_IN)) {	/* Data set is geographic with x = longitudes */
		if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Geographic grid, longitudes span exactly 360\n");
			/* If w/e is 360 and gridline reg then we have a repeat entry for 360.  For pixel there are never repeat pixels */
			return ((h->registration == GMT_GRID_NODE_REG) ? GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT : GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT);
		}
		else if (fabs (h->nx * h->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Geographic grid, longitude cells span exactly 360\n");
			/* If n*xinc = 360 and previous test failed then we do not have a repeat node */
			return (GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT);
		}
		else if ((h->wesn[XHI] - h->wesn[XLO]) > 360.0) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Geographic grid, longitudes span more than 360\n");
			return (GMT_GRID_GEOGRAPHIC_MORE360);
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Geographic grid, longitudes span less than 360\n");
			return (GMT_GRID_GEOGRAPHIC_LESS360);
		}
	}
	else if (h->wesn[YLO] >= -90.0 && h->wesn[YHI] <= 90.0) {	/* Here we simply advice the user if grid looks like geographic but is not set as such */
		if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Cartesian grid, yet x spans exactly 360 and -90 <= y <= 90.\n");
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "     To make sure the grid is recognized as geographical and global, use the -fg option\n");
			return (GMT_GRID_CARTESIAN);
		}
		else if (fabs (h->nx * h->inc[GMT_X] - 360.0) < GMT_CONV4_LIMIT) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Cartesian grid, yet x cells span exactly 360 and -90 <= y <= 90.\n");
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "     To make sure the grid is recognized as geographical and global, use the -fg option\n");
			return (GMT_GRID_CARTESIAN);
		}
	}
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Grid is Cartesian\n");
	return (GMT_GRID_CARTESIAN);
}

void gmt_grd_check_consistency (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid)
{	/* Enforce before writing a grid that periodic grids with repeating columns
	 * agree on the node values in those columns; if different replace with average.
	 * This only affects geographic grids of 360-degree extent with gridline registration.
	 * Also, if geographic grid with gridline registration, if the N or S pole row is present
	 * we ensure that they all have identical values, otherwise replace by mean value */
	unsigned int row = 0, col = 0;
	unsigned int we_conflicts = 0, p_conflicts = 0;
	uint64_t left = 0, right = 0, node = 0;

	if (header->registration == GMT_GRID_PIXEL_REG) return;	/* Not gridline registered */
	if (!GMT_is_geographic (GMT, GMT_OUT)) return;		/* Not geographic */
	if (header->wesn[YLO] == -90.0) {	/* Check consistency of S pole duplicates */
		double sum;
		node = GMT_IJP (header, 0, 0);	/* First node at S pole */
		sum = grid[node++];
		p_conflicts = 0;
		for (col = 1; col < header->nx; col++, node++) {
			if (grid[node] != grid[node-1]) p_conflicts++;
			sum += grid[node];
		}
		if (p_conflicts) {
			float f_value = (float)(sum / header->nx);
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: detected %u inconsistent values at south pole. Values fixed by setting all to average row value.\n", p_conflicts);
			node = GMT_IJP (header, 0, 0);	/* First node at S pole */
			for (col = 0; col < header->nx; col++, node++) grid[node] = f_value;
		}
	}
	if (header->wesn[YHI] == +90.0) {	/* Check consistency of N pole duplicates */
		double sum;
		node = GMT_IJP (header, header->ny-1, 0);	/* First node at N pole */
		sum = grid[node++];
		p_conflicts = 0;
		for (col = 1; col < header->nx; col++, node++) {
			if (grid[node] != grid[node-1]) p_conflicts++;
			sum += grid[node];
		}
		if (p_conflicts) {
			float f_value = (float)(sum / header->nx);
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: detected %u inconsistent values at north pole. Values fixed by setting all to average row value.\n", p_conflicts);
			node = GMT_IJP (header, header->ny-1, 0);	/* First node at N pole */
			for (col = 0; col < header->nx; col++, node++) grid[node] = f_value;
		}
	}
	if (!GMT_360_RANGE (header->wesn[XLO], header->wesn[XHI])) return;	/* Not 360-degree range */

	for (row = 0; row < header->ny; row++) {
		left = GMT_IJP (header, row, 0);	/* Left node */
		right = left + header->nx - 1;		/* Right node */
		if (grid[right] != grid[left]) {
			grid[right] = grid[left];
			we_conflicts++;
		}
	}
	if (we_conflicts)
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: detected %u inconsistent values along periodic east boundary of grid. Values fixed by duplicating west boundary.\n", we_conflicts);
}

int GMT_read_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header)
{	/* file:	File name
	 * header:	grid structure header
	 * Note: The header reflects what is actually in the file, and all the dimensions
	 * reflect the number of rows, cols, size, pads etc.  However, if GMT_read_grd is
	 * called requesting a subset then these will be reset accordingly.
	 */

	int err;	/* Implied by GMT_err_trap */
	unsigned int nx, ny;
	double scale, offset;
	float invalid;

	/* Save parameters on file name suffix before issuing GMT->session.readinfo */
	GMT_err_trap (GMT_grd_get_format (GMT, file, header, true));

	/* remember scale, offset, and invalid: */
	scale = header->z_scale_factor;
	offset = header->z_add_offset;
	invalid = header->nan_value;

	GMT_err_trap ((*GMT->session.readinfo[header->type]) (GMT, header));

	gmt_grd_xy_scale (GMT, header, GMT_IN);	/* Possibly scale wesn,inc */

	/* restore non-default scale, offset, and invalid: */
	if (scale != 1.0)
		header->z_scale_factor = scale;
	if (offset != 0.0)
		header->z_add_offset = offset;
	if (isfinite(invalid))
		header->nan_value = invalid;

	gmt_grd_get_units (GMT, header);
	header->grdtype = gmt_get_grdtype (GMT, header);

	GMT_err_pass (GMT, GMT_grd_RI_verify (GMT, header, 0), file);
	nx = header->nx;	ny = header->ny;	/* Save copy */
	GMT_set_grddim (GMT, header);	/* Set all integer dimensions and xy_off */

	/* Sanity check for grid that may have been created oddly.  Inspired by
	 * Geomapapp output where -R was set to outside of pixel boundaries insteda
	 * of standard -R settings, yet with node_offset = gridline... */

	if (abs((int)(header->nx - nx)) == 1 && abs((int)(header->ny - ny)) == 1) {
       		header->nx = nx;    header->ny = ny;
 		if (header->registration == GMT_GRID_PIXEL_REG) {
 			header->registration = GMT_GRID_NODE_REG;
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Grid has wrong registration type. Switching from pixel to gridline registration\n");
		}
		else {
 			header->registration = GMT_GRID_PIXEL_REG;
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Grid has wrong registration type. Switching from gridline to pixel registration\n");
		}
	}

	/* unpack z-range: */
	header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
	header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;

	return (GMT_NOERROR);
}

int GMT_write_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header)
{	/* file:	File name
	 * header:	grid structure header
	 */

	int err;	/* Implied by GMT_err_trap */

	GMT_err_trap (GMT_grd_get_format (GMT, file, header, false));

	gmt_grd_xy_scale (GMT, header, GMT_OUT);	/* Possibly scale wesn,inc */
	/* pack z-range: */
	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	gmt_grd_set_units (GMT, header);
	return ((*GMT->session.writeinfo[header->type]) (GMT, header));
}

int GMT_update_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header)
{	/* file:	- IGNORED -
	 * header:	grid structure header
	 */

	/* pack z-range: */
	GMT_UNUSED(file);
	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	gmt_grd_set_units (GMT, header);
	return ((*GMT->session.updateinfo[header->type]) (GMT, header));
}

int GMT_read_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, float *grid, double *wesn, unsigned int *pad, int complex_mode)
{	/* file:	- IGNORED -
	 * header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to extract  [Use entire file if NULL or contains 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	bool expand;		/* true or false */
	int err;		/* Implied by GMT_err_trap */
	struct GRD_PAD P;

	complex_mode &= GMT_GRID_IS_COMPLEX_MASK;	/* Remove any non-complex flags */
	/* If we are reading a 2nd grid (e.g., real, then imag) we must update info about the file since it will be a different file */
	if (header->complex_mode && (header->complex_mode & complex_mode) == 0) GMT_err_trap (GMT_grd_get_format (GMT, file, header, true));

	expand = gmt_padspace (GMT, header, wesn, pad, &P);	/* true if we can extend the region by the pad-size to obtain real data for BC */

	gmt_grd_layout (GMT, header, grid, complex_mode & GMT_GRID_IS_COMPLEX_MASK, GMT_IN);	/* Deal with complex layout */

	GMT_err_trap ((*GMT->session.readgrd[header->type]) (GMT, header, grid, P.wesn, P.pad, complex_mode));

	if (expand) /* Must undo the region extension and reset nx, ny using original pad  */
		GMT_memcpy (header->wesn, wesn, 4, double);
	header->grdtype = gmt_get_grdtype (GMT, header);	/* Since may change if a subset */
	GMT_grd_setpad (GMT, header, pad);	/* Copy the pad to the header */
	GMT_set_grddim (GMT, header);		/* Update all dimensions */
	if (expand) GMT_grd_zminmax (GMT, header, grid);	/* Reset min/max since current extrema includes the padded region */
	GMT_pack_grid (GMT, header, grid, k_grd_unpack); /* revert scale and offset */
	GMT_BC_init (GMT, header);	/* Initialize grid interpolation and boundary condition parameters */

	return (GMT_NOERROR);
}

int GMT_write_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, float *grid, double *wesn, unsigned int *pad, int complex_mode)
{	/* file:	File name
	 * header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to write out  [Use entire file if NULL or contains 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	int err;	/* Implied by GMT_err_trap */

	GMT_err_trap (GMT_grd_get_format (GMT, file, header, false));
	gmt_grd_set_units (GMT, header);
	GMT_pack_grid (GMT, header, grid, k_grd_pack); /* scale and offset */
	gmt_grd_xy_scale (GMT, header, GMT_OUT);	/* Possibly scale wesn,inc */

	gmt_grd_layout (GMT, header, grid, complex_mode, GMT_OUT);	/* Deal with complex layout */
	gmt_grd_check_consistency (GMT, header, grid);			/* Fix east repeating columns and polar values */
	err = (*GMT->session.writegrd[header->type]) (GMT, header, grid, wesn, pad, complex_mode);
	if (GMT->parent->leave_grid_scaled == 0) GMT_pack_grid (GMT, header, grid, k_grd_unpack); /* revert scale and offset to leave grid as it was before writing unless session originated from gm*/
	return (err);
}

size_t GMT_grd_data_size (struct GMT_CTRL *GMT, unsigned int format, float *nan_value)
{
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
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unknown grid data type: %c\n", GMT->session.grdformat[format][1]);
			return (GMT_GRDIO_UNKNOWN_TYPE);
	}
}

void GMT_grd_set_ij_inc (struct GMT_CTRL *GMT, unsigned int nx, int *ij_inc)
{	/* Set increments to the 4 nodes with ij as lower-left node, from a node at (i,j).
	 * nx may be header->nx or header->mx depending on pad */
	GMT_UNUSED(GMT);
	int s_nx = nx;	/* A signed version */
	ij_inc[0] = 0;		/* No offset relative to itself */
	ij_inc[1] = 1;		/* The node one column to the right relative to ij */
	ij_inc[2] = 1 - s_nx;	/* The node one column to the right and one row up relative to ij */
	ij_inc[3] = -s_nx;	/* The node one row up relative to ij */
}

int GMT_grd_format_decoder (struct GMT_CTRL *GMT, const char *code, unsigned int *type_id) {
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

/* gmt_grd_RI_verify -- routine to check grd R and I compatibility
 *
 * Author:	W H F Smith
 * Date:	20 April 1998
 */

int GMT_grd_RI_verify (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int mode)
{
	/* mode - 0 means we are checking an existing grid, mode = 1 means we test a new -R -I combination */

	unsigned int error = 0;

	if (!strcmp (GMT->init.module_name, "grdedit")) return (GMT_NOERROR);	/* Separate handling in grdedit to allow grdedit -A */

	switch (GMT_minmaxinc_verify (GMT, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], GMT_CONV4_LIMIT)) {
		case 3:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: grid x increment <= 0.0\n");
			error++;
			break;
		case 2:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: grid x range <= 0.0\n");
			if (GMT_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Make sure west < east for geographic coordinates\n");
			error++;
			break;
		case 1:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: (x_max-x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			error++;
		default:
			/* Everything is OK */
			break;
	}

	switch (GMT_minmaxinc_verify (GMT, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], GMT_CONV4_LIMIT)) {
		case 3:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: grid y increment <= 0.0\n");
			error++;
			break;
		case 2:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: grid y range <= 0.0\n");
			error++;
			break;
		case 1:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: (y_max-y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			error++;
		default:
			/* Everything is OK */
			break;
	}
	if (error) return ((mode == 0) ? GMT_GRDIO_RI_OLDBAD : GMT_GRDIO_RI_NEWBAD);
	return (GMT_NOERROR);
}

int GMT_grd_prep_io (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double wesn[], unsigned int *width, unsigned int *height, int *first_col, int *last_col, int *first_row, int *last_row, unsigned int **index)
{
	/* Determines which rows and columns to extract to extract from a grid, based on w,e,s,n.
	 * This routine first rounds the w,e,s,n boundaries to the nearest gridlines or pixels,
	 * then determines the first and last columns and rows, and the width and height of the subset (in cells).
	 * The routine also returns and array of the x-indices in the source grid to be used in the target (subset) grid.
	 * All integers represented positive definite items hence unsigned variables.
	 */

	bool geo = false;
	unsigned int one_or_zero, col, *actual_col = NULL;	/* Column numbers */
	double small = 0.1, half_or_zero, x;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "region: %g %g, grid: %g %g\n", wesn[XLO], wesn[XHI], header->wesn[XLO], header->wesn[XHI]);

	half_or_zero = (header->registration == GMT_GRID_PIXEL_REG) ? 0.5 : 0.0;

	if (!GMT_is_subset (GMT, header, wesn)) {	/* Get entire file */
		*width  = header->nx;
		*height = header->ny;
		*first_col = *first_row = 0;
		*last_col  = header->nx - 1;
		*last_row  = header->ny - 1;
		GMT_memcpy (wesn, header->wesn, 4, double);
	}
	else {				/* Must deal with a subregion */
		if (GMT_x_is_lon (GMT, GMT_IN))
			geo = true;	/* Geographic data for sure */
		else if (wesn[XLO] < header->wesn[XLO] || wesn[XHI] > header->wesn[XHI])
			geo = true;	/* Probably dealing with periodic grid */

		if (wesn[YLO] < header->wesn[YLO] || wesn[YHI] > header->wesn[YHI]) return (GMT_GRDIO_DOMAIN_VIOLATION);	/* Calling program goofed... */

		one_or_zero = (header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;

		/* Make sure w,e,s,n are proper multiples of x_inc,y_inc away from x_min,y_min */

		GMT_err_pass (GMT, GMT_adjust_loose_wesn (GMT, wesn, header), header->name);

		/* Get dimension of subregion */

		*width  = urint ((wesn[XHI] - wesn[XLO]) * header->r_inc[GMT_X]) + one_or_zero;
		*height = urint ((wesn[YHI] - wesn[YLO]) * header->r_inc[GMT_Y]) + one_or_zero;

		/* Get first and last row and column numbers */

		*first_col = irint (floor ((wesn[XLO] - header->wesn[XLO]) * header->r_inc[GMT_X] + small));
		*last_col  = irint (ceil  ((wesn[XHI] - header->wesn[XLO]) * header->r_inc[GMT_X] - small)) - 1 + one_or_zero;
		*first_row = irint (floor ((header->wesn[YHI] - wesn[YHI]) * header->r_inc[GMT_Y] + small));
		*last_row  = irint (ceil  ((header->wesn[YHI] - wesn[YLO]) * header->r_inc[GMT_Y] - small)) - 1 + one_or_zero;
	}

	actual_col = GMT_memory (GMT, NULL, *width, unsigned int);
	if (geo) {
		small = 0.1 * header->inc[GMT_X];
		for (col = 0; col < (*width); col++) {
			x = GMT_col_to_x (GMT, col, wesn[XLO], wesn[XHI], header->inc[GMT_X], half_or_zero, *width);
			if (header->wesn[XLO] - x > small)
				x += 360.0;
			else if (x - header->wesn[XHI] > small)
				x -= 360.0;
			actual_col[col] = (unsigned int)GMT_grd_x_to_col (GMT, x, header);
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

void GMT_decode_grd_h_info (struct GMT_CTRL *GMT, char *input, struct GMT_GRID_HEADER *h) {
	/* Given input string, copy elements into string portions of h.
		 By default use "/" as the field separator. However, if the first and
		 last character of the input string is the same AND that character
		 is non-alpha-numeric, use the first character as a separator. This
		 is to allow "/" as part of the fields.
		 If a field is blank or has an equals sign, skip it.
		 This routine is usually called if -D<input> was given by user,
		 and after GMT_grd_init() has been called. */

	char *ptr, *stringp = input, sep[] = "/";
	unsigned int entry = 0;

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
		if (*ptr != '\0' || strcmp (ptr, "=") == 0) { /* entry is not blank or "=" */
			switch (entry) {
				case 0:
					GMT_memset (h->x_units, GMT_GRID_UNIT_LEN80, char);
					if (strlen(ptr) >= GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_NORMAL,
								"Warning: X unit string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_UNIT_LEN80);
					strncpy (h->x_units, ptr, GMT_GRID_UNIT_LEN80);
					break;
				case 1:
					GMT_memset (h->y_units, GMT_GRID_UNIT_LEN80, char);
					if (strlen(ptr) >= GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_NORMAL,
								"Warning: Y unit string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_UNIT_LEN80);
					strncpy (h->y_units, ptr, GMT_GRID_UNIT_LEN80);
					break;
				case 2:
					GMT_memset (h->z_units, GMT_GRID_UNIT_LEN80, char);
					if (strlen(ptr) >= GMT_GRID_UNIT_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_NORMAL,
								"Warning: Z unit string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_UNIT_LEN80);
					strncpy (h->z_units, ptr, GMT_GRID_UNIT_LEN80);
					break;
				case 3:
					h->z_scale_factor = strtod (ptr, NULL);
					break;
				case 4:
					h->z_add_offset = strtod (ptr, NULL);
					break;
				case 5:
					h->nan_value = strtof (ptr, NULL);
					break;
				case 6:
					if (strlen(ptr) >= GMT_GRID_TITLE_LEN80)
						GMT_Report (GMT->parent, GMT_MSG_NORMAL,
								"Warning: Title string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_TITLE_LEN80);
					strncpy (h->title, ptr, GMT_GRID_TITLE_LEN80);
					break;
				case 7:
					if (strlen(ptr) >= GMT_GRID_REMARK_LEN160)
						GMT_Report (GMT->parent, GMT_MSG_NORMAL,
								"Warning: Remark string exceeds upper length of %d characters (truncated)\n",
								GMT_GRID_REMARK_LEN160);
					strncpy (h->remark, ptr, GMT_GRID_REMARK_LEN160);
					break;
				default:
					break;
			}
		}
		entry++;
	}
	return;
}

void GMT_set_grdinc (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h)
{
	/* Update grid increments based on w/e/s/n, nx/ny, and registration */
	GMT_UNUSED(GMT);
	h->inc[GMT_X] = GMT_get_inc (GMT, h->wesn[XLO], h->wesn[XHI], h->nx, h->registration);
	h->inc[GMT_Y] = GMT_get_inc (GMT, h->wesn[YLO], h->wesn[YHI], h->ny, h->registration);
	h->r_inc[GMT_X] = 1.0 / h->inc[GMT_X];	/* Get inverse increments to avoid divisions later */
	h->r_inc[GMT_Y] = 1.0 / h->inc[GMT_Y];
}

void GMT_set_grddim (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h)
{	/* Assumes pad is set and then computes nx, ny, mx, my, nm, size, xy_off based on w/e/s/n.  */
	h->nx = GMT_grd_get_nx (GMT, h);		/* Set nx, ny based on w/e/s/n and offset */
	h->ny = GMT_grd_get_ny (GMT, h);
	h->mx = gmt_grd_get_nxpad (h, h->pad);	/* Set mx, my based on h->{nx,ny} and the current pad */
	h->my = gmt_grd_get_nypad (h, h->pad);
	h->nm = gmt_grd_get_nm (h);		/* Sets the number of actual data items */
	h->size = gmt_grd_get_size (h);		/* Sets the number of items (not bytes!) needed to hold this array, which includes the padding (size >= nm) */
	h->xy_off = 0.5 * h->registration;
	GMT_set_grdinc (GMT, h);
}

void GMT_grd_init (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, struct GMT_OPTION *options, bool update)
{	/* GMT_grd_init initializes a grd header to default values and copies the
	 * options to the header variable command.
	 * update = true if we only want to update command line */

	int i;

	if (update)	/* Only clean the command history */
		GMT_memset (header->command, GMT_GRID_COMMAND_LEN320, char);
	else {		/* Wipe the slate clean */
		GMT_memset (header, 1, struct GMT_GRID_HEADER);

		/* Set the variables that are not initialized to 0/false/NULL */
		header->z_scale_factor = 1.0;
		header->row_order      = k_nc_start_south; /* S->N */
		header->z_id           = -1;
		header->n_bands        = 1; /* Grids have at least one band but images may have 3 (RGB) or 4 (RGBA) */
		header->z_min          = GMT->session.d_NaN;
		header->z_max          = GMT->session.d_NaN;
		header->nan_value      = GMT->session.f_NaN;
		if (GMT_is_geographic (GMT, GMT_OUT)) {
			strcpy (header->x_units, "longitude [degrees_east]");
			strcpy (header->y_units, "latitude [degrees_north]");
		}
		else {
			strcpy (header->x_units, "x");
			strcpy (header->y_units, "y");
		}
		strcpy (header->z_units, "z");
		GMT_grd_setpad (GMT, header, GMT->current.io.pad); /* Assign default pad */
	}

	/* Always update command line history, if given */

	if (options) {
		size_t len;
		struct GMTAPI_CTRL *API = GMT->parent;
		int argc = 0; char **argv = NULL;

		if ((argv = GMT_Create_Args (API, &argc, options)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not create argc, argv from linked structure options!\n");
			return;
		}
		strncpy (header->command, GMT->init.module_name, GMT_GRID_COMMAND_LEN320);
		len = strlen (header->command);
		for (i = 0; len < GMT_GRID_COMMAND_LEN320 && i < argc; i++) {
			len += strlen (argv[i]) + 1;
			if (len >= GMT_GRID_COMMAND_LEN320) continue;
			strcat (header->command, " ");
			strcat (header->command, argv[i]);
		}
		header->command[len] = 0;
		sprintf (header->title, "Produced by %s", GMT->init.module_name);
		GMT_Destroy_Args (API, argc, &argv);
	}
}

void GMT_grd_shift (struct GMT_CTRL *GMT, struct GMT_GRID *G, double shift)
{
	/* Rotate geographical, global grid in e-w direction
	 * This function will shift a grid by shift degrees.
	 * It is only called when we know the grid is geographic. */

	unsigned int col, row, width, n_warn = 0;
	int n_shift, actual_col;
	bool gridline;
	uint64_t ij;
	float *tmp = NULL;

	n_shift = irint (shift * G->header->r_inc[GMT_X]);
	width = urint (360.0 * G->header->r_inc[GMT_X]);
	if (width > G->header->nx) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot rotate grid, width is too small\n");
		return;
	}
	gridline = (width < G->header->nx);	/* Gridline-registrered grids will have width = nx-1, pixel grids have width = nx */

	tmp = GMT_memory (GMT, NULL, G->header->nx, float);

	for (row = 0; row < G->header->ny; row++) {
		ij = GMT_IJP (G->header, row, 0);
		if (gridline && G->data[ij] != G->data[ij+width]) n_warn++;
		for (col = 0; col < G->header->nx; col++) {
			actual_col = (col - n_shift) % width;
			if (actual_col < 0) actual_col += width;
			tmp[actual_col] = G->data[ij+col];
		}
		GMT_memcpy (&G->data[ij], tmp, G->header->nx, float);
	}
	GMT_free (GMT, tmp);

	/* Shift boundaries */

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

	if (n_warn)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Inconsistent values at repeated longitude nodes (%g and %g) for %d rows\n",
			G->header->wesn[XLO], G->header->wesn[XHI], n_warn);
}

int GMT_grd_setregion (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, double *wesn, unsigned int interpolant)
{
	/* GMT_grd_setregion determines what w,e,s,n should be passed to GMT_read_grd.
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

	/* First make an educated guess whether the grid and region are geographical and global */
	grid_global = GMT_grd_is_global (GMT, h);

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
	if (GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && GMT_x_is_lon (GMT, GMT_IN)) off = 0.0;
	wesn[XLO] = GMT->common.R.wesn[XLO] - off * h->inc[GMT_X], wesn[XHI] = GMT->common.R.wesn[XHI] + off * h->inc[GMT_X];

	if (GMT->common.R.oblique && !GMT_IS_RECT_GRATICULE (GMT)) {	/* Used -R... with oblique boundaries - return entire grid */
		if (wesn[XHI] < h->wesn[XLO])	/* Make adjustments so GMT->current.proj.[w,e] jives with h->wesn */
			shift_x = 360.0;
		else if (wesn[XLO] > h->wesn[XHI])
			shift_x = -360.0;
		else
			shift_x = 0.0;

		wesn[XLO] = h->wesn[XLO] + lrint ((wesn[XLO] - h->wesn[XLO] + shift_x) * h->r_inc[GMT_X]) * h->inc[GMT_X];
		wesn[XHI] = h->wesn[XHI] + lrint ((wesn[XHI] - h->wesn[XHI] + shift_x) * h->r_inc[GMT_X]) * h->inc[GMT_X];
		wesn[YLO] = h->wesn[YLO] + lrint ((wesn[YLO] - h->wesn[YLO]) * h->r_inc[GMT_Y]) * h->inc[GMT_Y];
		wesn[YHI] = h->wesn[YHI] + lrint ((wesn[YHI] - h->wesn[YHI]) * h->r_inc[GMT_Y]) * h->inc[GMT_Y];

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
	wesn[YLO] = MAX (h->wesn[YLO], h->wesn[YLO] + floor ((wesn[YLO] - h->wesn[YLO]) * h->r_inc[GMT_Y] + GMT_CONV4_LIMIT) * h->inc[GMT_Y]);
	wesn[YHI] = MIN (h->wesn[YHI], h->wesn[YLO] + ceil  ((wesn[YHI] - h->wesn[YLO]) * h->r_inc[GMT_Y] - GMT_CONV4_LIMIT) * h->inc[GMT_Y]);

	if (wesn[YHI] <= wesn[YLO]) {	/* Grid must be outside chosen -R */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Your grid y's or latitudes appear to be outside the map region and will be skipped.\n");
		return (0);
	}

	/* Periodic grid with 360 degree range is easy */

	if (grid_global) {
		wesn[XLO] = h->wesn[XLO] + floor ((wesn[XLO] - h->wesn[XLO]) * h->r_inc[GMT_X] + GMT_CONV4_LIMIT) * h->inc[GMT_X];
		wesn[XHI] = h->wesn[XLO] + ceil  ((wesn[XHI] - h->wesn[XLO]) * h->r_inc[GMT_X] - GMT_CONV4_LIMIT) * h->inc[GMT_X];
		/* For the odd chance that xmin or xmax are outside the region: bring them in */
		if (wesn[XHI] - wesn[XLO] >= 360.0) {
			while (wesn[XLO] < GMT->common.R.wesn[XLO]) wesn[XLO] += h->inc[GMT_X];
			while (wesn[XHI] > GMT->common.R.wesn[XHI]) wesn[XHI] -= h->inc[GMT_X];
		}
		return (1);
	}
	if (GMT->current.map.is_world) {
		wesn[XLO] = h->wesn[XLO], wesn[XHI] = h->wesn[XHI];
		return (1);
	}

	/* Shift a geographic grid 360 degrees up or down to maximize the amount of longitude range */

	if (GMT_x_is_lon (GMT, GMT_IN)) {
		if (GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) {
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

	wesn[XLO] = MAX (h->wesn[XLO], h->wesn[XLO] + floor ((wesn[XLO] - h->wesn[XLO]) * h->r_inc[GMT_X] + GMT_CONV4_LIMIT) * h->inc[GMT_X]);
	wesn[XHI] = MIN (h->wesn[XHI], h->wesn[XLO] + ceil  ((wesn[XHI] - h->wesn[XLO]) * h->r_inc[GMT_X] - GMT_CONV4_LIMIT) * h->inc[GMT_X]);

	if (wesn[XHI] <= wesn[XLO]) {	/* Grid is outside chosen -R in longitude */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Your grid x's or longitudes appear to be outside the map region and will be skipped.\n");
		return (0);
	}
	return (grid_global ? 1 : 2);
}

int GMT_adjust_loose_wesn (struct GMT_CTRL *GMT, double wesn[], struct GMT_GRID_HEADER *header)
{
	/* Used to ensure that sloppy w,e,s,n values are rounded to the gridlines or pixels in the referenced grid.
	 * Upon entry, the boundaries w,e,s,n are given as a rough approximation of the actual subset needed.
	 * The routine will limit the boundaries to the grids region and round w,e,s,n to the nearest gridline or
	 * pixel boundaries (depending on the grid orientation).
	 * Warnings are produced when the w,e,s,n boundaries are adjusted, so this routine is currently not
	 * intended to throw just any values at it (although one could).
	 */

	bool global, error = false;
	double val, dx, small;
	char format[GMT_LEN64] = {""};

	switch (GMT_minmaxinc_verify (GMT, wesn[XLO], wesn[XHI], header->inc[GMT_X], GMT_CONV4_LIMIT)) {	/* Check if range is compatible with x_inc */
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
	switch (GMT_minmaxinc_verify (GMT, wesn[YLO], wesn[YHI], header->inc[GMT_Y], GMT_CONV4_LIMIT)) {	/* Check if range is compatible with y_inc */
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
	global = GMT_grd_is_global (GMT, header);

	if (!global) {
		if (GMT_x_is_lon (GMT, GMT_IN)) {
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
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Region exceeds grid domain. Region reduced to grid domain.\n");

	if (!(GMT_x_is_lon (GMT, GMT_IN) && GMT_360_RANGE (wesn[XLO], wesn[XHI]) && global)) {    /* Do this unless a 360 longitude wrap */
		small = GMT_CONV4_LIMIT * header->inc[GMT_X];

		val = header->wesn[XLO] + lrint ((wesn[XLO] - header->wesn[XLO]) * header->r_inc[GMT_X]) * header->inc[GMT_X];
		dx = fabs (wesn[XLO] - val);
		if (GMT_x_is_lon (GMT, GMT_IN)) dx = fmod (dx, 360.0);
		if (dx > small) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: (w - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			sprintf (format, "Warning: w reset from %s to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, format, wesn[XLO], val);
			wesn[XLO] = val;
		}

		val = header->wesn[XLO] + lrint ((wesn[XHI] - header->wesn[XLO]) * header->r_inc[GMT_X]) * header->inc[GMT_X];
		dx = fabs (wesn[XHI] - val);
		if (GMT_x_is_lon (GMT, GMT_IN)) dx = fmod (dx, 360.0);
		if (dx > small) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: (e - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
			sprintf (format, "Warning: e reset from %s to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, format, wesn[XHI], val);
			wesn[XHI] = val;
		}
	}

	/* Check if s,n are a multiple of y_inc offset from y_min - if not adjust s, n */
	small = GMT_CONV4_LIMIT * header->inc[GMT_Y];

	val = header->wesn[YLO] + lrint ((wesn[YLO] - header->wesn[YLO]) * header->r_inc[GMT_Y]) * header->inc[GMT_Y];
	if (fabs (wesn[YLO] - val) > small) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: (s - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
		sprintf (format, "Warning: s reset from %s to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, format, wesn[YLO], val);
		wesn[YLO] = val;
	}

	val = header->wesn[YLO] + lrint ((wesn[YHI] - header->wesn[YLO]) * header->r_inc[GMT_Y]) * header->inc[GMT_Y];
	if (fabs (wesn[YHI] - val) > small) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: (n - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_CONV4_LIMIT);
		sprintf (format, "Warning: n reset from %s to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, format, wesn[YHI], val);
		wesn[YHI] = val;
	}
	return (GMT_NOERROR);
}

void GMT_scale_and_offset_f (struct GMT_CTRL *GMT, float *data, size_t length, double scale, double offset) {
	/* Routine that does the data conversion and sanity checking before
	 * calling scale_and_offset_f() to scale and offset the data in a grid */
	float scale_f  = (float)scale;
	float offset_f = (float)offset;

	if (scale_f == 1.0 && offset_f == 0.0)
		return; /* No work needed */

	/* Sanity checks */
	if (!isnormal (scale)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Scale must be a non-zero normalized number (%g).\n", scale);
		scale_f = 1.0f;
	}
	if (!isfinite (offset)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Offset must be a finite number (%g).\n", offset);
		offset_f = 0.0f;
	}

	/* Call workhorse */
	scale_and_offset_f (data, length, scale_f, offset_f);
}

int GMT_read_img (struct GMT_CTRL *GMT, char *imgfile, struct GMT_GRID *Grid, double *in_wesn, double scale, unsigned int mode, double lat, bool init)
{
	/* Function that reads an entire Sandwell/Smith Mercator grid and stores it like a regular
	 * GMT grid.  If init is true we also initialize the Mercator projection.  Lat should be 0.0
	 * if we are dealing with standard 72 or 80 img latitude; else it must be specified.
	 */

	int status, first_i;
	unsigned int min, actual_col, n_cols, row, col;
	uint64_t ij;
	off_t n_skip;
	int16_t *i2 = NULL;
	uint16_t *u2 = NULL;
	char file[GMT_BUFSIZ];
	struct stat buf;
	FILE *fp = NULL;
	double wesn[4], wesn_all[4];

	if (!GMT_getdatapath (GMT, imgfile, file, R_OK)) return (GMT_GRDIO_FILE_NOT_FOUND);
	if (stat (file, &buf)) return (GMT_GRDIO_STAT_FAILED);	/* Inquiry about file failed somehow */

	switch (buf.st_size) {	/* Known sizes are 1 or 2 min at lat_max = ~72, ~80, or ~85 */
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_85*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_85;
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_80*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_80;
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_72*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_72;
			min = 1;
			break;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_85*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_85;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_80;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_72*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_72;
			min = 2;
			break;
		default:
			if (lat == 0.0) return (GMT_GRDIO_BAD_IMG_LAT);
			min = (buf.st_size > GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE) ? 1 : 2;
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "img file %s has unusual size - grid increment defaults to %d min\n", file, min);
			break;
	}

	wesn_all[XLO] = GMT_IMG_MINLON;	wesn_all[XHI] = GMT_IMG_MAXLON;
	wesn_all[YLO] = -lat;		wesn_all[YHI] = lat;
	if (!in_wesn || (in_wesn[XLO] == in_wesn[XHI] && in_wesn[YLO] == in_wesn[YHI])) {	/* Default is entire grid */
		GMT_memcpy (wesn, wesn_all, 4, double);
	}
	else	/* Use specified subset */
		GMT_memcpy (wesn, in_wesn, 4, double);

	if ((fp = GMT_fopen (GMT, file, "rb")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Reading img grid from file %s (scale = %g mode = %d lat = %g)\n", imgfile, scale, mode, lat);
	Grid->header->inc[GMT_X] = Grid->header->inc[GMT_Y] = min / 60.0;

	if (init) {
		/* Select plain Mercator on a sphere with -Jm1 -R0/360/-lat/+lat */
		GMT->current.setting.proj_ellipsoid = GMT_get_ellipsoid (GMT, "Sphere");
		GMT->current.proj.units_pr_degree = true;
		GMT->current.proj.pars[0] = 180.0;
		GMT->current.proj.pars[1] = 0.0;
		GMT->current.proj.pars[2] = 1.0;
		GMT->current.proj.projection = GMT_MERCATOR;
		GMT_set_geographic (GMT, GMT_IN);
		GMT->common.J.active = true;

		GMT_err_pass (GMT, GMT_map_setup (GMT, wesn_all), file);
	}

	if (wesn[XLO] < 0.0 && wesn[XHI] < 0.0) wesn[XLO] += 360.0, wesn[XHI] += 360.0;

	/* Project lon/lat boundaries to Mercator units */
	GMT_geo_to_xy (GMT, wesn[XLO], wesn[YLO], &Grid->header->wesn[XLO], &Grid->header->wesn[YLO]);
	GMT_geo_to_xy (GMT, wesn[XHI], wesn[YHI], &Grid->header->wesn[XHI], &Grid->header->wesn[YHI]);

	/* Adjust boundaries to multiples of increments, making sure we are inside bounds */
	Grid->header->wesn[XLO] = MAX (GMT_IMG_MINLON, floor (Grid->header->wesn[XLO] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X]);
	Grid->header->wesn[XHI] = MIN (GMT_IMG_MAXLON, ceil (Grid->header->wesn[XHI] / Grid->header->inc[GMT_X]) * Grid->header->inc[GMT_X]);
	if (Grid->header->wesn[XLO] > Grid->header->wesn[XHI]) Grid->header->wesn[XLO] -= 360.0;
	Grid->header->wesn[YLO] = MAX (0.0, floor (Grid->header->wesn[YLO] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y]);
	Grid->header->wesn[YHI] = MIN (GMT->current.proj.rect[YHI], ceil (Grid->header->wesn[YHI] / Grid->header->inc[GMT_Y]) * Grid->header->inc[GMT_Y]);
	/* Allocate grid memory */

	Grid->header->registration = GMT_GRID_PIXEL_REG;	/* These are always pixel grids */
	if ((status = GMT_grd_RI_verify (GMT, Grid->header, 1))) return (status);	/* Final verification of -R -I; return error if we must */
	GMT_grd_setpad (GMT, Grid->header, GMT->current.io.pad);			/* Assign default pad */
	GMT_set_grddim (GMT, Grid->header);					/* Set all dimensions before returning */
	Grid->data = GMT_memory_aligned (GMT, NULL, Grid->header->size, float);

	n_cols = (min == 1) ? GMT_IMG_NLON_1M : GMT_IMG_NLON_2M;		/* Number of columns (10800 or 21600) */
	first_i = irint (floor (Grid->header->wesn[XLO] * Grid->header->r_inc[GMT_X]));				/* first tile partly or fully inside region */
	if (first_i < 0) first_i += n_cols;
	n_skip = lrint (floor ((GMT->current.proj.rect[YHI] - Grid->header->wesn[YHI]) * Grid->header->r_inc[GMT_Y]));	/* Number of rows clearly above y_max */
	if (fseek (fp, n_skip * n_cols * GMT_IMG_ITEMSIZE, SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);

	i2 = GMT_memory (GMT, NULL, n_cols, int16_t);
	for (row = 0; row < Grid->header->ny; row++) {	/* Read all the rows, offset by 2 boundary rows and cols */
		if (GMT_fread (i2, sizeof (int16_t), n_cols, fp) != n_cols)
			return (GMT_GRDIO_READ_FAILED);	/* Get one row */
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
			if (max_step > 32768) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "File %s probably needs to byteswapped (max change = %u)\n", file, max_step);
		}
#endif
		ij = GMT_IJP (Grid->header, row, 0);
		for (col = 0, actual_col = first_i; col < Grid->header->nx; col++) {	/* Process this row's values */
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
			Grid->data[ij+col] = (float)((mode == 3) ? i2[actual_col] : (i2[actual_col] * scale));
			if (++actual_col == n_cols) actual_col = 0;	/* Wrapped around 360 */
		}
	}
	GMT_free (GMT, i2);
	GMT_fclose (GMT, fp);
	if (init) {
		GMT_memcpy (GMT->common.R.wesn, wesn, 4, double);
		GMT->common.J.active = false;
	}
	GMT_BC_init (GMT, Grid->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, Grid, GMT_IN);	/* Set boundary conditions */
	return (GMT_NOERROR);
}

void gmt_grd_wipe_pad (struct GMT_CTRL *GMT, struct GMT_GRID *G)
{	/* Reset padded areas to 0. */
	unsigned int row;
	size_t ij0;

	if (G->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Calling gmt_grd_wipe_pad on interleaved complex grid! Programming error?\n");
		return;
	}
	if (G->header->pad[YHI]) GMT_memset (G->data, G->header->mx * G->header->pad[YHI], float);	/* Wipe top pad */
	if (G->header->pad[YLO]) {	/* Wipe bottom pad */
		ij0 = GMT_IJ (G->header, G->header->my - G->header->pad[YLO], 0);	/* Index of start of bottom pad */
		GMT_memset (&(G->data[ij0]), G->header->mx * G->header->pad[YLO], float);
	}
	if (G->header->pad[XLO] == 0 && G->header->pad[XHI] == 0) return;	/* Nothing to do */
	for (row = G->header->pad[YHI]; row < G->header->my - G->header->pad[YLO]; row++) {	/* Wipe left and right pad which is trickier */
		ij0 = GMT_IJ (G->header, row, 0);	/* Index of this row's left column (1st entry in west pad) */
		if (G->header->pad[XLO]) GMT_memset (&(G->data[ij0]), G->header->pad[XLO], float);
		ij0 += (G->header->mx - G->header->pad[XHI]);	/* Start of this rows east pad's 1st column */
		if (G->header->pad[XHI]) GMT_memset (&(G->data[ij0]), G->header->pad[XHI], float);
	}
}

void grd_pad_off_sub (struct GMT_GRID *G, float *data)
{
	uint64_t ijp, ij0;
	unsigned int row;
	for (row = 0; row < G->header->ny; row++) {
		ijp = GMT_IJP (G->header, row, 0);	/* Index of start of this row's first column in padded grid  */
		ij0 = GMT_IJ0 (G->header, row, 0);	/* Index of start of this row's first column in unpadded grid */
		GMT_memcpy (&(data[ij0]), &(data[ijp]), G->header->nx, float);	/* Only copy the nx data values */
	}
}

void GMT_grd_pad_off (struct GMT_CTRL *GMT, struct GMT_GRID *G)
{	/* Shifts the grid contents so there is no pad.  The remainder of
	 * the array is not reset and should not be addressed, but
	 * we set it to zero just in case.
	 * If pad is zero then we do nothing.
	 */
	bool is_complex;
	uint64_t nm;
	if (G->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Calling GMT_grd_pad_off on interleaved complex grid! Programming error?\n");
		return;
	}
	if (!GMT_grd_pad_status (GMT, G->header, NULL)) return;	/* No pad so nothing to do */

	/* Here, G has a pad which we need to eliminate */
	is_complex = (G->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK);
	if (!is_complex || (G->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL))
		grd_pad_off_sub (G, G->data);	/* Remove pad around real component only or entire normal grid */
	if (is_complex && (G->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG))
		grd_pad_off_sub (G, &G->data[G->header->size/2]);	/* Remove pad around imaginary component */
	nm = G->header->nm;	/* Number of nodes in one component */
	if (is_complex) nm *= 2;	/* But there might be two */
	if (G->header->size > nm) {	/* Just wipe the remaineder of the array to be sure */
		size_t n_to_cleen = G->header->size - nm;
		GMT_memset (&(G->data[nm]), n_to_cleen, float);	/* nm is 1st position after last row */
	}
	GMT_memset (G->header->pad, 4, int);	/* Pad is no longer active */
	GMT_set_grddim (GMT, G->header);		/* Update all dimensions to reflect the padding */
}

void grd_pad_on_sub (struct GMT_CTRL *GMT, struct GMT_GRID *G, struct GMT_GRID_HEADER *h_old, float *data)
{	/* Use G for dimensions but operate on data array which points to either the real or imaginary section */
	unsigned int row;
	uint64_t ij_new, ij_old;
	for (row = G->header->ny; row > 0; row--) {
		ij_new = GMT_IJP (G->header, row-1, 0);	/* Index of start of this row's first column in new padded grid  */
		ij_old = GMT_IJP (h_old, row-1, 0);	/* Index of start of this row's first column in old padded grid */
		GMT_memcpy (&(data[ij_new]), &(data[ij_old]), G->header->nx, float);
	}
	gmt_grd_wipe_pad (GMT, G);	/* Set pad areas to 0 */
}

void GMT_grd_pad_on (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int *pad)
{ 	/* Shift grid content from a non-padded (or differently padded) to a padded organization.
	 * We check that the grid size can handle this and allocate more space if needed.
	 * If pad matches the grid's pad then we do nothing.
	 */
	bool is_complex;
	size_t size;
	struct GMT_GRID_HEADER *h = NULL;

	if (G->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Calling GMT_grd_pad_off on interleaved complex grid! Programming error?\n");
		return;
	}
	if (GMT_grd_pad_status (GMT, G->header, pad)) return;	/* Already padded as requested so nothing to do */
	if (pad[XLO] == 0 && pad[XHI] == 0 && pad[YLO] == 0 && pad[YHI] == 0) {	/* Just remove the existing pad entirely */
		GMT_grd_pad_off (GMT, G);
		return;
	}
	/* Here the pads differ (or G has no pad at all) */
	is_complex = (G->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK);
	size = gmt_grd_get_nxpad (G->header, pad) * gmt_grd_get_nypad (G->header, pad);	/* New array size after pad is added */
	if (is_complex) size *= 2;	/* Twice the space for complex grids */
	if (size > G->header->size) {	/* Must allocate more space, but since no realloc for aligned memory we must do it the hard way */
		float *f = NULL;
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Extend grid via copy onto larger memory-aligned grid\n");
		f = GMT_memory_aligned (GMT, NULL, size, float);	/* New, larger grid size */
		GMT_memcpy (f, G->data, G->header->size, float);	/* Copy over previous grid values */
		GMT_free_aligned (GMT, G->data);			/* Free previous aligned grid memory */
		G->data = f;						/* Attach the new, larger aligned memory */
		G->header->size = size;					/* Update the size */
	}
	/* Because G may have a pad that is nonzero (but different from pad) we need a different header structure in the macros below */
	h = GMT_duplicate_gridheader (GMT, G->header);

	GMT_grd_setpad (GMT, G->header, pad);	/* G->header->pad is now set to specified dimensions in pad */
	GMT_set_grddim (GMT, G->header);	/* Update all dimensions to reflect the new padding */
	if (is_complex && (G->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG))
		grd_pad_on_sub (GMT, G, h, &G->data[size/2]);	/* Add pad around imaginary component first */
	if (!is_complex || (G->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL))
		grd_pad_on_sub (GMT, G, h, G->data);	/* Add pad around real component */
	GMT_free (GMT, h);	/* Done with this header */
}

void grd_pad_zero_sub (struct GMT_GRID *G, float *data)
{
	unsigned int row, col, nx1;
	uint64_t ij_f, ij_l;

	if (G->header->pad[YHI]) GMT_memset (data, G->header->pad[YHI] * G->header->mx, float);		/* Zero the top pad */
	nx1 = G->header->nx - 1;	/* Last column */
	GMT_row_loop (NULL, G, row) {
		ij_f = GMT_IJP (G->header, row,   0);				/* Index of first column this row  */
		ij_l = GMT_IJP (G->header, row, nx1);				/* Index of last column this row */
		for (col = 1; col <= G->header->pad[XLO]; col++) data[ij_f-col] = 0.0f;	/* Zero the left pad at this row */
		for (col = 1; col <= G->header->pad[XHI]; col++) data[ij_l+col] = 0.0f;	/* Zero the left pad at this row */
	}
	if (G->header->pad[YLO]) {
		int pad = G->header->pad[XLO];
		ij_f = GMT_IJP (G->header, G->header->ny, -pad);				/* Index of first column of bottom pad  */
		GMT_memset (&(data[ij_f]), G->header->pad[YLO] * G->header->mx, float);	/* Zero the bottom pad */
	}
}

void GMT_grd_pad_zero (struct GMT_CTRL *GMT, struct GMT_GRID *G)
{	/* Sets all boundary row/col nodes to zero and sets
	 * the header->BC to GMT_IS_NOTSET.
	 */
	bool is_complex;
	if (G->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Calling GMT_grd_pad_off on interleaved complex grid! Programming error?\n");
		return;
	}
	if (!GMT_grd_pad_status (GMT, G->header, NULL)) return;	/* No pad so nothing to do */
	if (G->header->BC[XLO] == GMT_BC_IS_NOTSET && G->header->BC[XHI] == GMT_BC_IS_NOTSET && G->header->BC[YLO] == GMT_BC_IS_NOTSET && G->header->BC[YHI] == GMT_BC_IS_NOTSET) return;	/* No BCs set so nothing to do */			/* No pad so nothing to do */
	/* Here, G has a pad with BCs which we need to reset */
	is_complex = (G->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK);
	if (!is_complex || (G->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL))
		grd_pad_zero_sub (G, G->data);
	if (is_complex && (G->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG))
		grd_pad_zero_sub (G, &G->data[G->header->size/2]);
	GMT_memset (G->header->BC, 4U, int);	/* BCs no longer set for this grid */
}

struct GMT_GRID * GMT_create_grid (struct GMT_CTRL *GMT)
{	/* Allocates space for a new grid container.  No space allocated for the float grid itself */
	struct GMT_GRID *G = NULL;

	G = GMT_memory (GMT, NULL, 1, struct GMT_GRID);
	G->header = GMT_memory (GMT, NULL, 1, struct GMT_GRID_HEADER);
	GMT_grd_init (GMT, G->header, NULL, false); /* Set default values */
	G->alloc_mode = GMT_ALLOCATED_BY_GMT;		/* Memory can be freed by GMT. */
	G->alloc_level = GMT->hidden.func_level;	/* Must be freed at this level. */
	G->id = GMT->parent->unique_var_ID++;		/* Give unique identifier */
	return (G);
}

struct GMT_GRID_HEADER *GMT_duplicate_gridheader (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h)
{	/* Duplicates a grid header. */
	struct GMT_GRID_HEADER *hnew = NULL;

	hnew = GMT_memory (GMT, NULL, 1, struct GMT_GRID_HEADER);
	GMT_memcpy (hnew, h, 1, struct GMT_GRID_HEADER);
	return (hnew);
}

struct GMT_GRID *GMT_duplicate_grid (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int mode)
{	/* Duplicates an entire grid, including data if requested. */
	struct GMT_GRID *Gnew = NULL;

	Gnew = GMT_create_grid (GMT);
	GMT_memcpy (Gnew->header, G->header, 1, struct GMT_GRID_HEADER);
	if ((mode & GMT_DUPLICATE_DATA) || (mode & GMT_DUPLICATE_ALLOC)) {	/* Also allocate and possiblhy duplicate data array */
		Gnew->data = GMT_memory_aligned (GMT, NULL, G->header->size, float);
		if (mode & GMT_DUPLICATE_DATA) GMT_memcpy (Gnew->data, G->data, G->header->size, float);
	}
	return (Gnew);
}

unsigned int GMT_free_grid_ptr (struct GMT_CTRL *GMT, struct GMT_GRID *G, bool free_grid)
{	/* By taking a reference to the grid pointer we can set it to NULL when done */
	if (!G) return 0;	/* Nothing to deallocate */
	/* Only free G->data if allocated by GMT AND free_grid is true */
	if (G->data && free_grid) {
		if (G->alloc_mode == GMT_ALLOCATED_BY_GMT) GMT_free_aligned (GMT, G->data);
		G->data = NULL;	/* This will remove reference to external memory since GMT_free_aligned would not have been called */
	}
	if (G->extra) gmt_close_grd (GMT, G);	/* Close input file used for row-by-row i/o */
	//if (G->header && G->alloc_mode == GMT_ALLOCATED_BY_GMT) GMT_free (GMT, G->header);
	if (G->header) {	/* Free the header structure and anything allocated by it */
		if (G->header->pocket) free (G->header->pocket);
		GMT_free (GMT, G->header);
	}
	return (G->alloc_mode);
}

void GMT_free_grid (struct GMT_CTRL *GMT, struct GMT_GRID **G, bool free_grid)
{	/* By taking a reference to the grid pointer we can set it to NULL when done */
	(void)GMT_free_grid_ptr (GMT, *G, free_grid);
	if (*G) GMT_free (GMT, *G);
}

int GMT_set_outgrid (struct GMT_CTRL *GMT, char *file, struct GMT_GRID *G, struct GMT_GRID **Out)
{	/* When the input grid is a read-only memory location then we cannot use
	 * the same grid to hold the output results but must allocate a separate
	 * grid.  To avoid wasting memory we try to reuse the input array when
	 * it is possible. We return true when new memory had to be allocated.
	 * Note we duplicate the grid if we must so that Out always has the input
	 * data in it (directly or via the pointer).  */

	if (GMT_File_Is_Memory (file) || G->alloc_mode == GMT_ALLOCATED_EXTERNALLY) {	/* Cannot store results in a non-GMT read-only input array */
		*Out = GMT_duplicate_grid (GMT, G, GMT_DUPLICATE_DATA);
		(*Out)->alloc_mode = GMT_ALLOCATED_BY_GMT;
		return (true);
	}
	/* Here we may overwrite the input grid and just pass the pointer back */
	(*Out) = G;
	return (false);
}

int gmt_init_grdheader (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, struct GMT_OPTION *options, double wesn[], double inc[], unsigned int registration, unsigned int mode)
{	/* Convenient way of setting a header struct wesn, inc, and registartion, then compute dimensions, etc. */
	double wesn_dup[4], inc_dup[2];
	if ((mode & GMT_VIA_OUTPUT) && wesn == NULL && inc == NULL) return (GMT_NOERROR);	/* OK for creating blank container for output */
	if (wesn == NULL) {	/* Must select -R setting */
		if (!GMT->common.R.active) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No wesn given and no -R in effect.  Cannot initialize new grid\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
	}
	else	/* In case user is passing header->wesn etc we must save them first as GMT_grd_init will clobber them */
		GMT_memcpy (wesn_dup, wesn, 4, double);
	if (inc == NULL) {	/* Must select -I setting */
		if (!GMT->common.API_I.active) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No inc given and no -I in effect.  Cannot initialize new grid\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
	}
	else	/* In case user is passing header->inc etc we must save them first as GMT_grd_init will clobber them */
		GMT_memcpy (inc_dup,  inc,  2, double);
	/* Clobber header and reset */
	GMT_grd_init (GMT, header, options, false);	/* This is for new grids only so update is always false */
	if (wesn == NULL)
		GMT_memcpy (header->wesn, GMT->common.R.wesn, 4, double);
	else
		GMT_memcpy (header->wesn, wesn_dup, 4, double);
	if (inc == NULL)
		GMT_memcpy (header->inc, GMT->common.API_I.inc, 2, double);
	else
		GMT_memcpy (header->inc, inc_dup, 2, double);
	/* registration may contain complex mode information */
	if (registration & GMT_GRID_DEFAULT_REG) registration |= GMT->common.r.registration;	/* Set the default registration */
	header->registration = (registration & 1);
	header->complex_mode = (registration & GMT_GRID_IS_COMPLEX_MASK);
	header->grdtype = gmt_get_grdtype (GMT, header);
	GMT_RI_prepare (GMT, header);	/* Ensure -R -I consistency and set nx, ny in case of meter units etc. */
	GMT_err_pass (GMT, GMT_grd_RI_verify (GMT, header, 1), "");
	GMT_grd_setpad (GMT, header, GMT->current.io.pad);	/* Assign default GMT pad */
	GMT_set_grddim (GMT, header);	/* Set all dimensions before returning */
	GMT_BC_init (GMT, header);	/* Initialize grid interpolation and boundary condition parameters */
	return (GMT_NOERROR);
}

int gmt_alloc_grid (struct GMT_CTRL *GMT, struct GMT_GRID *Grid)
{	/* Use information in Grid header to allocate the grid data.
	 * We assume gmt_init_grdheader has been called. */

	if (Grid->data) return (GMT_PTR_NOT_NULL);
	if (Grid->header->size == 0U) return (GMT_SIZE_IS_ZERO);
	if ((Grid->data = GMT_memory_aligned (GMT, NULL, Grid->header->size, float)) == NULL) return (GMT_MEMORY_ERROR);
	return (GMT_NOERROR);
}

int gmt_alloc_image (struct GMT_CTRL *GMT, struct GMT_IMAGE *Image)
{	/* Use information in Image header to allocate the image data.
	 * We assume gmt_init_grdheader has been called. */

	if (Image->data) return (GMT_PTR_NOT_NULL);
	if (Image->header->size == 0U) return (GMT_SIZE_IS_ZERO);
	if ((Image->data = GMT_memory (GMT, NULL, Image->header->size * Image->header->n_bands, unsigned char)) == NULL) return (GMT_MEMORY_ERROR);
	return (GMT_NOERROR);
}

int GMT_change_grdreg (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, unsigned int registration)
{
	GMT_UNUSED(GMT);
	unsigned int old_registration;
	double F;
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

void GMT_grd_zminmax (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, float *z)
{	/* Reset the xmin/zmax values in the header */
	unsigned int row, col;
	uint64_t node, n = 0;

	h->z_min = DBL_MAX;	h->z_max = -DBL_MAX;
	for (row = 0; row < h->ny; row++) {
		for (col = 0, node = GMT_IJP (h, row, 0); col < h->nx; col++, node++) {
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

void GMT_grd_minmax (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double xyz[2][3])
{	/* Determine a grid's global min and max locations and z values; return via xyz */
	GMT_UNUSED(GMT);
	unsigned int row, col, i;
	uint64_t ij, i_minmax[2] = {0, 0};
	float z_extreme[2] = {FLT_MAX, -FLT_MAX};

	GMT_grd_loop (GMT, Grid, row, col, ij) {
		if (GMT_is_fnan (Grid->data[ij])) continue;
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
		xyz[i][GMT_X] = GMT_grd_col_to_x (GMT, GMT_col (Grid->header, i_minmax[i]), Grid->header);
		xyz[i][GMT_Y] = GMT_grd_row_to_y (GMT, GMT_row (Grid->header, i_minmax[i]), Grid->header);
		xyz[i][GMT_Z] = z_extreme[i];
	}
}

void GMT_grd_detrend (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, unsigned mode, double *coeff)
{
	/* mode = 0 (GMT_FFT_REMOVE_NOTHING):  Do nothing.
	 * mode = 1 (GMT_FFT_REMOVE_MEAN):  Remove the mean value (returned via a[0])
	 * mode = 2 (GMT_FFT_REMOVE_MID):   Remove the mid value value (returned via a[0])
	 * mode = 3 (GMT_FFT_REMOVE_TREND): Remove the best-fitting plane by least squares (returned via a[0-2])
	 *
	 * Note: The grid may be complex and contain real, imag, or both components.  The data should
	 * be in serial layout so we may loop over the compoents and do our thing.  Only the real
	 * components coefficients are returned.
	 */

	unsigned int col, row, one_or_zero, start_component = 0, stop_component = 0, component;
	uint64_t ij, offset;
	bool is_complex = false;
	double x_half_length, one_on_xhl, y_half_length, one_on_yhl;
	double sumx2, sumy2, data_var_orig = 0.0, data_var = 0.0, var_redux, x, y, z, a[3];
	char *comp[2] = {"real", "imaginary"};

	GMT_memset (coeff, 3, double);

	if (Grid->header->trendmode != GMT_FFT_REMOVE_NOTHING) {	/* Already removed the trend */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Grid has already been detrending - no action taken\n");
		return;
	}
	if (mode == GMT_FFT_REMOVE_NOTHING) {	/* Do nothing */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "No detrending selected\n");
		return;
	}
	Grid->header->trendmode = mode;	/* Update grid header */
	if (Grid->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Demultiplexing complex grid before detrending can take place.\n");
		GMT_grd_mux_demux (GMT, Grid->header, Grid->data, GMT_GRID_IS_SERIAL);
	}

	if (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_MASK) {	/* Complex grid */
		is_complex = true;
		start_component = (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) ? 0 : 1;
		stop_component  = (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG) ? 1 : 0;
	}

	for (component = start_component; component <= stop_component; component++) {	/* Loop over 1 or 2 components */
		offset = component * Grid->header->size / 2;	/* offset to start of this component in grid */
		GMT_memset (a, 3, double);
		if (mode == GMT_FFT_REMOVE_MEAN) {	/* Remove mean */
			for (row = 0; row < Grid->header->ny; row++) for (col = 0; col < Grid->header->nx; col++) {
				ij = GMT_IJP (Grid->header,row,col) + offset;
				z = Grid->data[ij];
				a[0] += z;
				data_var_orig += z * z;
			}
			a[0] /= Grid->header->nm;
			for (row = 0; row < Grid->header->ny; row++) for (col = 0; col < Grid->header->nx; col++) {
				ij = GMT_IJP (Grid->header,row,col) + offset;
				Grid->data[ij] -= (float)a[0];
				z = Grid->data[ij];
				data_var += z * z;
			}
			var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
			if (is_complex)
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Mean value removed from %s component: %.8g Variance reduction: %.2f\n", comp[component], a[0], var_redux);
			else
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Mean value removed: %.8g Variance reduction: %.2f\n", a[0], var_redux);
		}
		else if (mode == GMT_FFT_REMOVE_MID) {	/* Remove mid value */
			double zmin = DBL_MAX, zmax = -DBL_MAX;
			for (row = 0; row < Grid->header->ny; row++) for (col = 0; col < Grid->header->nx; col++) {
				ij = GMT_IJP (Grid->header,row,col) + offset;
				z = Grid->data[ij];
				data_var_orig += z * z;
				if (z < zmin) zmin = z;
				if (z > zmax) zmax = z;
			}
			a[0] = 0.5 * (zmin + zmax);	/* Mid value */
			for (row = 0; row < Grid->header->ny; row++) for (col = 0; col < Grid->header->nx; col++) {
				ij = GMT_IJP (Grid->header,row,col) + offset;
				Grid->data[ij] -= (float)a[0];
				z = Grid->data[ij];
				data_var += z * z;
			}
			var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
			if (is_complex)
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Mid value removed from %s component: %.8g Variance reduction: %.2f\n", comp[component], a[0], var_redux);
			else
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Mid value removed: %.8g Variance reduction: %.2f\n", a[0], var_redux);
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
			x_half_length = 0.5 * (Grid->header->nx - one_or_zero);
			one_on_xhl = 1.0 / x_half_length;
			y_half_length = 0.5 * (Grid->header->ny - one_or_zero);
			one_on_yhl = 1.0 / y_half_length;

			sumx2 = sumy2 = data_var = 0.0;

			for (row = 0; row < Grid->header->ny; row++) {
				y = one_on_yhl * (row - y_half_length);
				for (col = 0; col < Grid->header->nx; col++) {
					x = one_on_xhl * (col - x_half_length);
					ij = GMT_IJP (Grid->header,row,col) + offset;
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
			for (row = 0; row < Grid->header->ny; row++) {
				y = one_on_yhl * (row - y_half_length);
				for (col = 0; col < Grid->header->nx; col++) {
					ij = GMT_IJP (Grid->header,row,col) + offset;
					x = one_on_xhl * (col - x_half_length);
					Grid->data[ij] -= (float)(a[0] + a[1]*x + a[2]*y);
					data_var += (Grid->data[ij] * Grid->data[ij]);
				}
			}
			var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
			data_var = sqrt (data_var / (Grid->header->nm - 1));
			/* Rescale a1,a2 into user's units, in case useful later */
			a[1] *= (2.0 / (Grid->header->wesn[XHI] - Grid->header->wesn[XLO]));
			a[2] *= (2.0 / (Grid->header->wesn[YHI] - Grid->header->wesn[YLO]));
			if (is_complex)
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Plane removed from %s component.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g Variance reduction: %.2f\n", comp[component], a[0], data_var, a[1], a[2], var_redux);
			else
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Plane removed.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g Variance reduction: %.2f\n", a[0], data_var, a[1], a[2], var_redux);
		}
		if (component == 0) GMT_memcpy (coeff, a, 3, double);	/* Return the real component results */
	}
}

bool GMT_init_complex (struct GMT_GRID_HEADER *header, unsigned int complex_mode, uint64_t *imag_offset)
{	/* Sets complex-related parameters based on the input complex_mode variable:
	 * If complex_mode & GMT_GRID_NO_HEADER then we do NOT want to write a header [output only; only some formats]
	 * If grid is the imaginary components of a complex grid then we compute the offset
	 * from the start of the complex array where the first imaginary value goes, using the serial arrangement.
	 */

	bool do_header = !(complex_mode & GMT_GRID_NO_HEADER);	/* Want no header if this bit is set */
	/* Imaginary components are stored after the real components if complex */
	*imag_offset = (complex_mode & GMT_GRID_IS_COMPLEX_IMAG) ? header->size / 2ULL : 0ULL;

	return (do_header);
}

bool GMT_check_url_name (char *fname) {
	/* File names starting as below should not be tested for existance or reading permissions as they
	   are either meant to be accessed on the fly (http & ftp) or they are compressed. So, if any of
	   the conditions holds true, returns true. All cases are read via GDAL support or other. */
	if ( !strncmp(fname,"http:",5)        ||
		!strncmp(fname,"https:",6)    ||
		!strncmp(fname,"ftp:",4)      ||
		!strncmp(fname,"/vsizip/",8)  ||
		!strncmp(fname,"/vsigzip/",9) ||
		!strncmp(fname,"/vsicurl/",9) ||
		!strncmp(fname,"/vsimem/",8)  ||
		!strncmp(fname,"/vsitar/",8) )

		return (true);
	else
		return (false);
}

#ifdef HAVE_GDAL
int GMT_read_image_info (struct GMT_CTRL *GMT, char *file, struct GMT_IMAGE *I) {
	int i;
	size_t k;
	double dumb;
	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;

	/* Allocate new control structures */
	to_gdalread   = GMT_memory (GMT, NULL, 1, struct GDALREAD_CTRL);
	from_gdalread = GMT_memory (GMT, NULL, 1, struct GD_CTRL);

	to_gdalread->M.active = true;	/* Get metadata only */

	k = strlen (file) - 1;
	while (k && file[k] && file[k] != '+') k--;	/* See if we have a band request */
	if (k && file[k+1] == 'b') {
		/* Yes we do. Put the band string into the 'pocket' where GMT_read_image will look and finish the request */
		I->header->pocket = strdup (&file[k+2]);
		file[k] = '\0';
	}

	if (GMT_gdalread (GMT, file, to_gdalread, from_gdalread)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading image with gdalread.\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	if ( strcmp(from_gdalread->band_field_names[0].DataType, "Byte") ) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Using data type other than byte (unsigned char) is not implemented\n");
		return (EXIT_FAILURE);
	}

	I->ColorInterp  = from_gdalread->ColorInterp;		/* Must find out how to release this mem */
	I->header->ProjRefPROJ4 = from_gdalread->ProjectionRefPROJ4;
	I->header->ProjRefWKT   = from_gdalread->ProjectionRefWKT;
	I->header->inc[GMT_X] = from_gdalread->hdr[7];
	I->header->inc[GMT_Y] = from_gdalread->hdr[8];
	I->header->nx = from_gdalread->RasterXsize;
	I->header->ny = from_gdalread->RasterYsize;
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

	GMT_set_grddim (GMT, I->header);		/* This recomputes nx|ny. Dangerous if -R is not compatible with inc */

	GMT_free (GMT, to_gdalread);
	for ( i = 0; i < from_gdalread->RasterCount; ++i )
		free (from_gdalread->band_field_names[i].DataType);	/* Those were allocated with strdup */
	if (from_gdalread->band_field_names) GMT_free (GMT, from_gdalread->band_field_names);
	if (from_gdalread->ColorMap) GMT_free (GMT, from_gdalread->ColorMap);	/* Maybe we will have a use for this in future, but not yet */
	GMT_free (GMT, from_gdalread);

	return (GMT_NOERROR);
}

int GMT_read_image (struct GMT_CTRL *GMT, char *file, struct GMT_IMAGE *I, double *wesn, unsigned int *pad, unsigned int complex_mode) {
	/* file:	- IGNORED -
	 * image:	array with final image
	 * wesn:	Sub-region to extract  [Use entire file if NULL or contains 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of image, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	GMT_UNUSED(complex_mode);
	int i;
	bool expand;
	struct GRD_PAD P;
	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;

	expand = gmt_padspace (GMT, I->header, wesn, pad, &P);	/* true if we can extend the region by the pad-size to obtain real data for BC */

	/*GMT_err_trap ((*GMT->session.readgrd[header->type]) (GMT, header, image, P.wesn, P.pad, complex_mode));*/

	/* Allocate new control structures */
	to_gdalread   = GMT_memory (GMT, NULL, 1, struct GDALREAD_CTRL);
	from_gdalread = GMT_memory (GMT, NULL, 1, struct GD_CTRL);

	if (GMT->common.R.active) {
		char strR [128];
		sprintf (strR, "%.10f/%.10f/%.10f/%.10f", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI],
							  GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		to_gdalread->R.region = strR;
		/*to_gdalread->R.active = true;*/	/* Wait untill we really know how to use it */
	}

	if (I->header->pocket) {				/* See if we have a band request */
		to_gdalread->B.active = true;
		to_gdalread->B.bands = I->header->pocket;	/* Band parsing and error testing is done in gmt_gdalread */
	}

	to_gdalread->p.active = to_gdalread->p.pad = (int)GMT->current.io.pad[0];	/* Only 'square' padding allowed */
	to_gdalread->I.active = true; 			/* Means that image in I->data will be BIP interleaved */

	/* Tell gmt_gdalread that we already have the memory allocated and send in the *data pointer */
	to_gdalread->c_ptr.active = true;
	to_gdalread->c_ptr.grd = I->data;

	if (GMT_gdalread (GMT, file, to_gdalread, from_gdalread)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading image with gdalread.\n");
		return (GMT_GRDIO_READ_FAILED);
	}

	if (to_gdalread->B.active) {
		free(I->header->pocket);		/* It was allocated by strdup. Free it for an eventual reuse. */
		I->header->pocket = NULL;
	}

	I->ColorMap = from_gdalread->ColorMap;
	I->header->n_bands = from_gdalread->nActualBands;	/* What matters here on is the number of bands actually read */

	if (expand) {	/* Must undo the region extension and reset nx, ny */
		I->header->nx -= (int)(P.pad[XLO] + P.pad[XHI]);
		I->header->ny -= (int)(P.pad[YLO] + P.pad[YHI]);
		GMT_memcpy (I->header->wesn, wesn, 4, double);
		I->header->nm = GMT_get_nm (GMT, I->header->nx, I->header->ny);
	}
	GMT_grd_setpad (GMT, I->header, pad);	/* Copy the pad to the header */

	GMT_free (GMT, to_gdalread);
	for (i = 0; i < from_gdalread->RasterCount; i++)
		free(from_gdalread->band_field_names[i].DataType);	/* Those were allocated with strdup */
	GMT_free (GMT, from_gdalread->band_field_names);
	GMT_free (GMT, from_gdalread);
	GMT_BC_init (GMT, I->header);	/* Initialize image interpolation and boundary condition parameters */

	return (GMT_NOERROR);
}
#endif
