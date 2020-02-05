/*
 *	Code donated by David Divens, NOAA/NGDC
 *	Distributed under the GNU Public License (see LICENSE.TXT for details)
 *--------------------------------------------------------------------*/

/* Comments added by P. Wessel:
 *
 * 1) GRD98 can now support pixel grids since 11/24/2006.
 * 2) Learned that 1/x_inc and 1/y_inc are stored as integers.  This means
 *    GRD98 imposes restrictions on x_inc & y_inc.
 * 3) Added full support for padding and complex_mode on 3/17/2011
 *
 * Public functions (5):
 *
 *	gmt_is_mgg2_grid	:
 *	gmt_mgg2_read_grd_info  :
 *	gmt_mgg2_write_grd_info	:
 *	gmt_mgg2_read_grd       :
 *	gmt_mgg2_write_grd      :
 */

#include "gmt_mgg_header2.h"

#define MGG_BYTE_SIZE

/* Local functions */

GMT_LOCAL void grd98_swap_word (void* ptr) {
	unsigned char *tmp = ptr;
	unsigned char a = tmp[0];
	tmp[0] = tmp[1];
	tmp[1] = a;
}

GMT_LOCAL void grd98_swap_long (void *ptr) {
	unsigned char *tmp = ptr;
	unsigned char a = tmp[0];
	tmp[0] = tmp[3];
	tmp[3] = a;
	a = tmp[1];
	tmp[1] = tmp[2];
	tmp[2] = a;
}

GMT_LOCAL int grd98_swap_mgg_header (MGG_GRID_HEADER_2 *header) {
	int i, version;
	/* Determine if swapping is needed */
	if (header->version == (GRD98_MAGIC_NUM + GRD98_VERSION)) return (0);	/* Version matches, No need to swap */
	version = header->version;
	grd98_swap_long (&version);
	if (version != (GRD98_MAGIC_NUM + GRD98_VERSION)) return (-1);		/* Cannot make sense of header */
	/* Here we come when we do need to swap */
	grd98_swap_long (&header->version);
	grd98_swap_long (&header->length);
	grd98_swap_long (&header->dataType);
	grd98_swap_long (&header->latDeg);
	grd98_swap_long (&header->latMin);
	grd98_swap_long (&header->latSec);
	grd98_swap_long (&header->latSpacing);
	grd98_swap_long (&header->latNumCells);
	grd98_swap_long (&header->lonDeg);
	grd98_swap_long (&header->lonMin);
	grd98_swap_long (&header->lonSec);
	grd98_swap_long (&header->lonSpacing);
	grd98_swap_long (&header->lonNumCells);
	grd98_swap_long (&header->minValue);
	grd98_swap_long (&header->maxValue);
	grd98_swap_long (&header->gridRadius);
	grd98_swap_long (&header->precision);
	grd98_swap_long (&header->nanValue);
	grd98_swap_long (&header->numType);
	grd98_swap_long (&header->waterDatum);
	grd98_swap_long (&header->dataLimit);
	grd98_swap_long (&header->cellRegistration);
	for (i = 0; i < GRD98_N_UNUSED; i++) grd98_swap_long (&header->unused[i]);
	return (1);	/* Signal we need to swap the data also */
}

GMT_LOCAL double grd98_dms2degrees (int deg, int min, int sec) {
	double decDeg = (double)deg;

	decDeg += (double)min * GMT_MIN2DEG;
	decDeg += (double)sec * GMT_SEC2DEG;

    return decDeg;
}

GMT_LOCAL void grd98_degrees2dms (double degrees, int *deg, int *min, int *sec) {
	/* Round off to the nearest half second */
	if (degrees < 0) degrees -= (0.5 * GMT_SEC2DEG);

	*deg = (int)degrees;
	degrees -= *deg;

	degrees *= GMT_DEG2MIN_F;
	*min = (int)(degrees);
	degrees -= *min;

	*sec = (int)(degrees * GMT_MIN2SEC_F);
}

GMT_LOCAL int grd98_GMTtoMGG2 (struct GMT_GRID_HEADER *gmt, MGG_GRID_HEADER_2 *mgg) {
	double f;
	gmt_M_memset (mgg, 1, MGG_GRID_HEADER_2);

	mgg->version     = GRD98_MAGIC_NUM + GRD98_VERSION;
	mgg->length      = sizeof (MGG_GRID_HEADER_2);
	mgg->dataType    = 1;

	mgg->cellRegistration = gmt->registration;
	mgg->lonNumCells = gmt->n_columns;
	f  = gmt->inc[GMT_X] * GMT_DEG2SEC_F;
	mgg->lonSpacing  = irint(f);
	if (fabs (f - (double)mgg->lonSpacing) > GMT_CONV8_LIMIT) return (GMT_GRDIO_GRD98_XINC);
	grd98_degrees2dms(gmt->wesn[XLO], &mgg->lonDeg, &mgg->lonMin, &mgg->lonSec);

	mgg->latNumCells = gmt->n_rows;
	f  = gmt->inc[GMT_Y] * GMT_DEG2SEC_F;
	mgg->latSpacing  = irint(gmt->inc[GMT_Y] * GMT_DEG2SEC_F);
	if (fabs (f - (double)mgg->latSpacing) > GMT_CONV8_LIMIT) return (GMT_GRDIO_GRD98_YINC);
	grd98_degrees2dms(gmt->wesn[YHI], &mgg->latDeg, &mgg->latMin, &mgg->latSec);

	/* Default values */
	mgg->gridRadius  = -1;
	mgg->precision   = GRD98_DEFAULT_PREC;
	mgg->nanValue    = GRD98_NAN_VALUE;
	mgg->numType     = sizeof (int);
	mgg->minValue    = irint(gmt->z_min * mgg->precision);
	mgg->maxValue    = irint(gmt->z_max * mgg->precision);

	/* Data fits in two byte boundary */
	if ((-SHRT_MAX <= mgg->minValue) && (mgg->maxValue <= SHRT_MAX)) {
		mgg->numType = sizeof (short);
		mgg->nanValue = (short)SHRT_MIN;
	}
#ifdef MGG_BYTE_SIZE
	/* Data fits in one byte boundary */
	if ((gmt->z_min >= 0) && (gmt->z_max <= 127)) {
		mgg->numType   = sizeof (char);
		mgg->nanValue  = (char)255;
		mgg->precision = 1;
		mgg->minValue  = irint (gmt->z_min);
		mgg->maxValue  = irint (gmt->z_max);
	}
#endif
	return (GMT_NOERROR);
}

GMT_LOCAL void grd98_MGG2toGMT (MGG_GRID_HEADER_2 *mgg, struct GMT_GRID_HEADER *gmt) {
	int one_or_zero;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (gmt);

	/* Do not memset the gmt header since it has the file name set */

	gmt->type = GMT_GRID_IS_RF;
	gmt->registration = mgg->cellRegistration;
	one_or_zero = 1 - gmt->registration;
	gmt->n_columns = mgg->lonNumCells;
	gmt->wesn[XLO] = grd98_dms2degrees(mgg->lonDeg, mgg->lonMin, mgg->lonSec);
	gmt->inc[GMT_X] = grd98_dms2degrees(0, 0, mgg->lonSpacing);
	gmt->wesn[XHI] = gmt->wesn[XLO] + (gmt->inc[GMT_X] * (gmt->n_columns - one_or_zero));

	gmt->n_rows = mgg->latNumCells;
	gmt->wesn[YHI] = grd98_dms2degrees(mgg->latDeg, mgg->latMin, mgg->latSec);
	gmt->inc[GMT_Y] = grd98_dms2degrees(0, 0, mgg->latSpacing);
	gmt->wesn[YLO] = gmt->wesn[YHI] - (gmt->inc[GMT_Y] * (gmt->n_rows - one_or_zero));

	gmt->z_min = (double)mgg->minValue / (double)mgg->precision;
	gmt->z_max = (double)mgg->maxValue / (double)mgg->precision;
	gmt->z_scale_factor = 1.0;
	gmt->z_add_offset = 0.0;
	switch (mgg->numType) {
		case  1:	HH->orig_datatype = GMT_CHAR;  break;
		case  2:	HH->orig_datatype = GMT_SHORT; break;
		case  4:	HH->orig_datatype = GMT_INT;   break;
		case -4:	HH->orig_datatype = GMT_FLOAT; break;
		default:	HH->orig_datatype = GMT_INT;   break;
	}
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

int gmt_is_mgg2_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Determine if file is a GRD98 file */
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;
	int ok;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = gmt_fopen (GMT, HH->name, GMT->current.io.r_mode)) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	gmt_M_memset (&mggHeader, 1, MGG_GRID_HEADER_2);
	if (gmt_M_fread (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}

	/* Swap header bytes if necessary; ok is 0|1 if successful and -1 if bad file */
	ok = grd98_swap_mgg_header (&mggHeader);

	/* Check the magic number and size of header */
	if (ok == -1) {
		gmt_fclose (GMT, fp);
		return (-1);	/* Not this kind of file */
	}
	header->type = GMT_GRID_IS_RF;
	gmt_fclose (GMT, fp);
	return GMT_NOERROR;
}

int gmt_mgg2_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;
	int ok;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))
		fp = GMT->session.std[GMT_IN];
	else if ((fp = gmt_fopen (GMT, HH->name, GMT->current.io.r_mode)) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	gmt_M_memset (&mggHeader, 1, MGG_GRID_HEADER_2);
	if (gmt_M_fread (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}

	/* Swap header bytes if necessary; ok is 0|1 if successful and -1 if bad file */
	ok = grd98_swap_mgg_header (&mggHeader);

	/* Check the magic number and size of header */
	if (ok == -1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized header, expected 0x%04X saw 0x%04X\n",
		            GRD98_MAGIC_NUM + GRD98_VERSION, mggHeader.version);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_GRD98_BADMAGIC);
	}

	if (mggHeader.length != sizeof (MGG_GRID_HEADER_2)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Invalid grid header size, expected %d, found %d\n",
		            (int)sizeof (MGG_GRID_HEADER_2), mggHeader.length);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_GRD98_BADLENGTH);
	}

	gmt_fclose (GMT, fp);

	grd98_MGG2toGMT (&mggHeader, header);

	return (GMT_NOERROR);
}

int gmt_mgg2_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;
	int err;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = gmt_fopen (GMT, HH->name, GMT->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	if ((err = grd98_GMTtoMGG2 (header, &mggHeader)) != 0) {
		gmt_fclose (GMT, fp);
		return (err);
	}

	if (gmt_M_fwrite (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_WRITE_FAILED);
	}

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_mgg2_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	MGG_GRID_HEADER_2 mggHeader;
	FILE *fp = NULL;
	int *tLong = NULL;
	short *tShort = NULL;
	char *tChar = NULL;
	gmt_grdfloat *tFloat = NULL;
	bool piping = false, is_float = false;
	int j, first_col, last_col, first_row, last_row, swap_all = 0;
	unsigned int width_in, height_in;
	unsigned int i, width_out, *actual_col = NULL;
	uint64_t kk, ij, j2, imag_offset;
	off_t long_offset;	/* For fseek only */
	size_t n_expected;	/* Items in one row */
	size_t size;		/* Item size */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	gmt_M_memset (&mggHeader, 1, MGG_GRID_HEADER_2);
	if (!strcmp (HH->name, "=")) {
		fp = GMT->session.std[GMT_IN];
		piping = true;
	}
	else if ((fp = gmt_fopen (GMT, HH->name, GMT->current.io.r_mode)) != NULL) {
		if (gmt_M_fread (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) {
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_READ_FAILED);
		}
		swap_all = grd98_swap_mgg_header (&mggHeader);
		if (swap_all == -1) {
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_GRD98_BADMAGIC);
		}
		if (mggHeader.numType == 0) mggHeader.numType = sizeof (int);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	is_float = (mggHeader.numType < 0 && abs (mggHeader.numType) == (int)sizeof (gmt_grdfloat));	/* Float file */

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_col), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	n_expected = header->n_columns;
	tLong  = gmt_M_memory (GMT, NULL, n_expected, int);
	tShort = (short *)tLong;	tChar = (char *)tLong;	tFloat = (gmt_grdfloat *)tLong;
	size = abs (mggHeader.numType);

	if (piping)	{ /* Skip data by reading it */
		for (j = 0; j < first_row; j++) if (gmt_M_fread ( tLong, size, n_expected, fp) != n_expected) {
			gmt_fclose (GMT, fp);
			gmt_M_free (GMT, actual_col);	gmt_M_free (GMT, tLong);
			return (GMT_GRDIO_READ_FAILED);
		}
	}
	else if (first_row) { /* Simply seek by it */
		long_offset = (off_t)first_row * (off_t)n_expected * size;
		if (fseek (fp, long_offset, 1)) {
			gmt_fclose (GMT, fp);
			gmt_M_free (GMT, actual_col);	gmt_M_free (GMT, tLong);
			return (GMT_GRDIO_SEEK_FAILED);
		}
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	HH->has_NaNs = GMT_GRID_NO_NANS;	/* We are about to check for NaNs and if none are found we retain 1, else 2 */
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		if (gmt_M_fread (tLong, size, n_expected, fp) != n_expected) {
			gmt_fclose (GMT, fp);
			gmt_M_free (GMT, actual_col);	gmt_M_free (GMT, tLong);
			return (GMT_GRDIO_READ_FAILED);
		}
		ij = imag_offset + (j2 + pad[YHI]) * width_out + pad[XLO];
		for (i = 0; i < width_in; i++) {
			kk = ij + i;
			if (mggHeader.numType == sizeof (int)) {
				if (swap_all) grd98_swap_long (&tLong[actual_col[i]]);
				if (tLong[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = (gmt_grdfloat) tLong[actual_col[i]] / (gmt_grdfloat) mggHeader.precision;
			}
			else if (is_float) {
				if (swap_all) grd98_swap_long (&tLong[actual_col[i]]);
				if (tLong[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = tFloat[actual_col[i]];
			}

			else if (mggHeader.numType == sizeof (short)) {
				if (swap_all) grd98_swap_word(&tShort[actual_col[i]]);
				if (tShort[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = (gmt_grdfloat) tShort[actual_col[i]] / (gmt_grdfloat) mggHeader.precision;
			}

			else if (mggHeader.numType == sizeof (char)) {
				if (tChar[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = (gmt_grdfloat) tChar[actual_col[i]] / (gmt_grdfloat) mggHeader.precision;
			}
			else {
				gmt_fclose (GMT, fp);
				gmt_M_free (GMT, actual_col);
				gmt_M_free (GMT, tLong);
				return (GMT_GRDIO_UNKNOWN_TYPE);
			}
			if (gmt_M_is_fnan (grid[kk])) {
				HH->has_NaNs = GMT_GRID_HAS_NANS;
				continue;
			}
			/* Update z_min, z_max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (header->z_min == DBL_MAX && header->z_max == -DBL_MAX) /* No valid data values in the grid */
		header->z_min = header->z_max = NAN;
	if (piping)	{ /* Skip data by reading it */
		int n_rows = header->n_rows;
		for (j = last_row + 1; j < n_rows; j++) {
			if (gmt_M_fread ( tLong, size, n_expected, fp) != n_expected) {
				gmt_M_free (GMT, tLong);
				gmt_M_free (GMT, actual_col);
				gmt_fclose (GMT, fp);
				return (GMT_GRDIO_READ_FAILED);
			}
		}
	}

	gmt_M_free (GMT, tLong);
	gmt_M_free (GMT, actual_col);

	header->n_columns = width_in;
	header->n_rows = height_in;
	gmt_M_memcpy (header->wesn, wesn, 4, double);

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_mgg2_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	MGG_GRID_HEADER_2 mggHeader;
	bool is_float = false, check;
	int i, j, err;
	unsigned int i2, ju, iu, width_out, height_out, width_in, *actual_col = NULL;
	int first_col, last_col, first_row, last_row;
	uint64_t ij, kk, j2, imag_offset;
	size_t size;

	int *tLong = NULL;
	short *tShort = NULL;
	char *tChar = NULL;
	gmt_grdfloat *tFloat = NULL;
	FILE *fp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = gmt_fopen (GMT, HH->name, GMT->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	check = !isnan (header->nan_value);

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	gmt_M_memcpy (header->wesn, wesn, 4, double);

	/* Find xmin/zmax */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		ij = imag_offset + j2 * width_in;
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			kk = ij + i2;
			if (isnan (grid[kk])) {
				if (check) grid[kk] = header->nan_value;
			}
			else {
				header->z_min = MIN (header->z_min, (double)grid[kk]);
				header->z_max = MAX (header->z_max, (double)grid[kk]);
			}
		}
	}
	if (header->z_min == DBL_MAX && header->z_max == -DBL_MAX) /* No valid data values in the grid */
		header->z_min = header->z_max = NAN;

	/* store header information and array */
	if ((err = grd98_GMTtoMGG2(header, &mggHeader)) != 0) {
		gmt_fclose (GMT, fp);
		gmt_M_free (GMT, actual_col);
		return (err);
	}
	if (gmt_M_fwrite (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) {
		gmt_M_free (GMT, actual_col);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_WRITE_FAILED);
	}
	is_float = (mggHeader.numType < 0 && abs (mggHeader.numType) == (int)sizeof (gmt_grdfloat));	/* Float file */

	tLong = gmt_M_memory (GMT, NULL, width_in, int);
	tShort = (short *) tLong;	tChar = (char *)tLong;	tFloat = (gmt_grdfloat *) tLong;

	i2 = first_col + pad[XLO];
	size = abs (mggHeader.numType);
	for (ju = 0, j2 = first_row + pad[YHI]; ju < height_out; ju++, j2++) {
		ij = imag_offset + j2 * width_in + i2;
		for (iu = 0; iu < width_out; iu++) {
			kk = ij+actual_col[iu];
			if (gmt_M_is_fnan (grid[kk])) {
				if (mggHeader.numType == sizeof (int))       tLong[iu]  = mggHeader.nanValue;
				else if (is_float) tFloat[iu]  = (gmt_grdfloat)mggHeader.nanValue;
				else if (mggHeader.numType == sizeof (short)) tShort[iu] = (short)mggHeader.nanValue;
				else if (mggHeader.numType == sizeof (char))  tChar[iu] = (char)mggHeader.nanValue;
				else {
					gmt_M_free (GMT, tLong);
					gmt_M_free (GMT, actual_col);
					gmt_fclose (GMT, fp);
					return (GMT_GRDIO_UNKNOWN_TYPE);
				}
			} else {
				if (grid[kk] > -0.1 && grid[kk] < 0) grid[kk] = (gmt_grdfloat)(-0.1);

				if (mggHeader.numType == sizeof (int))
					tLong[iu] = (int)rint ((double)grid[kk] * mggHeader.precision);
				else if (is_float)
					tFloat[iu] = grid[kk];
				else if (mggHeader.numType == sizeof (short))
					tShort[iu] = (short) rint((double)grid[kk] * mggHeader.precision);
				else if (mggHeader.numType == sizeof (char))
					tChar[iu] = (char) rint((double)grid[kk] * mggHeader.precision);
				else {
					gmt_M_free (GMT, tLong);
					gmt_M_free (GMT, actual_col);
					gmt_fclose (GMT, fp);
					return (GMT_GRDIO_UNKNOWN_TYPE);
				}
			}
		}
		if (gmt_M_fwrite (tLong, size, width_out, fp) != width_out) {
			gmt_M_free (GMT, tLong);
			gmt_M_free (GMT, actual_col);
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_WRITE_FAILED);
		}
	}
	gmt_M_free (GMT, tLong);
	gmt_M_free (GMT, actual_col);

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}
