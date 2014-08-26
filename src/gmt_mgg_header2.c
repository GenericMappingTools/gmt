/*	$Id$
 *
 *	Code donated by David Divens, NOAA/NGDC
 *	Distributed under the GNU Public License (see LICENSE.TXT for details)
 *--------------------------------------------------------------------*/

/* Comments added by P. Wessel:
 *
 * 1) GRD98 can now support pixel grids since 11/24/2006.
 * 2) Learned that 1/x_inc and 1/y_inc are stored as integers.  This means
 *    GRD98 imposes restrictions on x_inc & y_inc.
 * 3) Added full support for padding and complex_mode on 3/17/2011
 */

#include "gmt_mgg_header2.h"

#define MGG_BYTE_SIZE

static void gmt_swap_word (void* ptr)
{
	unsigned char *tmp = ptr;
	unsigned char a = tmp[0];
	tmp[0] = tmp[1];
	tmp[1] = a;
}

static void gmt_swap_long (void *ptr)
{
	unsigned char *tmp = ptr;
	unsigned char a = tmp[0];
	tmp[0] = tmp[3];
	tmp[3] = a;
	a = tmp[1];
	tmp[1] = tmp[2];
	tmp[2] = a;
}

int gmt_swap_mgg_header (MGG_GRID_HEADER_2 *header)
{
	int i, version;
	/* Determine if swapping is needed */
	if (header->version == (GRD98_MAGIC_NUM + GRD98_VERSION)) return (0);	/* Version matches, No need to swap */
	version = header->version;
	gmt_swap_long (&version);
	if (version != (GRD98_MAGIC_NUM + GRD98_VERSION)) return (-1);		/* Cannot make sense of header */
	/* Here we come when we do need to swap */
	gmt_swap_long (&header->version);
	gmt_swap_long (&header->length);
	gmt_swap_long (&header->dataType);
	gmt_swap_long (&header->latDeg);
	gmt_swap_long (&header->latMin);
	gmt_swap_long (&header->latSec);
	gmt_swap_long (&header->latSpacing);
	gmt_swap_long (&header->latNumCells);
	gmt_swap_long (&header->lonDeg);
	gmt_swap_long (&header->lonMin);
	gmt_swap_long (&header->lonSec);
	gmt_swap_long (&header->lonSpacing);
	gmt_swap_long (&header->lonNumCells);
	gmt_swap_long (&header->minValue);
	gmt_swap_long (&header->maxValue);
	gmt_swap_long (&header->gridRadius);
	gmt_swap_long (&header->precision);
	gmt_swap_long (&header->nanValue);
	gmt_swap_long (&header->numType);
	gmt_swap_long (&header->waterDatum);
	gmt_swap_long (&header->dataLimit);
	gmt_swap_long (&header->cellRegistration);
	for (i = 0; i < GRD98_N_UNUSED; i++) gmt_swap_long (&header->unused[i]);
	return (1);	/* Signal we need to swap the data also */
}

static double gmt_dms2degrees (int deg, int min, int sec)
{
	double decDeg = (double)deg;

	decDeg += (double)min * GMT_MIN2DEG;
	decDeg += (double)sec * GMT_SEC2DEG;

    return decDeg;
}

static void gmt_degrees2dms (double degrees, int *deg, int *min, int *sec)
{
	/* Round off to the nearest half second */
	if (degrees < 0) degrees -= (0.5 * GMT_SEC2DEG);

	*deg = (int)degrees;
	degrees -= *deg;

	degrees *= GMT_DEG2MIN_F;
	*min = (int)(degrees);
	degrees -= *min;

	*sec = (int)(degrees * GMT_MIN2SEC_F);
}

int gmt_GMTtoMGG2 (struct GMT_GRID_HEADER *gmt, MGG_GRID_HEADER_2 *mgg)
{
	double f;
	GMT_memset (mgg, 1, MGG_GRID_HEADER_2);

	mgg->version     = GRD98_MAGIC_NUM + GRD98_VERSION;
	mgg->length      = sizeof (MGG_GRID_HEADER_2);
	mgg->dataType    = 1;

	mgg->cellRegistration = gmt->registration;
	mgg->lonNumCells = gmt->nx;
	f  = gmt->inc[GMT_X] * GMT_DEG2SEC_F;
	mgg->lonSpacing  = irint(f);
	if (fabs (f - (double)mgg->lonSpacing) > GMT_CONV8_LIMIT) return (GMT_GRDIO_GRD98_XINC);
	gmt_degrees2dms(gmt->wesn[XLO], &mgg->lonDeg, &mgg->lonMin, &mgg->lonSec);

	mgg->latNumCells = gmt->ny;
	f  = gmt->inc[GMT_Y] * GMT_DEG2SEC_F;
	mgg->latSpacing  = irint(gmt->inc[GMT_Y] * GMT_DEG2SEC_F);
	if (fabs (f - (double)mgg->latSpacing) > GMT_CONV8_LIMIT) return (GMT_GRDIO_GRD98_YINC);
	gmt_degrees2dms(gmt->wesn[YHI], &mgg->latDeg, &mgg->latMin, &mgg->latSec);

	/* Default values */
	mgg->gridRadius  = -1;
	mgg->precision   = GRD98_DEFAULT_PREC;
	mgg->nanValue    = GRD98_NAN_VALUE;
	mgg->numType     = sizeof (int);
	mgg->minValue    = irint(gmt->z_min * mgg->precision);
	mgg->maxValue    = irint(gmt->z_max * mgg->precision);

	/* Data fits in two byte boundry */
	if ((-SHRT_MAX <= mgg->minValue) && (mgg->maxValue <= SHRT_MAX)) {
		mgg->numType = sizeof (short);
		mgg->nanValue = (short)SHRT_MIN;
	}
#ifdef MGG_BYTE_SIZE
	/* Data fits in one byte boundry */
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

static void gmt_MGG2toGMT (MGG_GRID_HEADER_2 *mgg, struct GMT_GRID_HEADER *gmt)
{
	int one_or_zero;

	/* Do not memset the gmt header since it has the file name set */

	gmt->type = GMT_GRID_IS_RF;
	gmt->registration = mgg->cellRegistration;
	one_or_zero = 1 - gmt->registration;
	gmt->nx = mgg->lonNumCells;
	gmt->wesn[XLO] = gmt_dms2degrees(mgg->lonDeg, mgg->lonMin, mgg->lonSec);
	gmt->inc[GMT_X] = gmt_dms2degrees(0, 0, mgg->lonSpacing);
	gmt->wesn[XHI] = gmt->wesn[XLO] + (gmt->inc[GMT_X] * (gmt->nx - one_or_zero));

	gmt->ny = mgg->latNumCells;
	gmt->wesn[YHI] = gmt_dms2degrees(mgg->latDeg, mgg->latMin, mgg->latSec);
	gmt->inc[GMT_Y] = gmt_dms2degrees(0, 0, mgg->latSpacing);
	gmt->wesn[YLO] = gmt->wesn[YHI] - (gmt->inc[GMT_Y] * (gmt->ny - one_or_zero));

	gmt->z_min = (double)mgg->minValue / (double)mgg->precision;
	gmt->z_max = (double)mgg->maxValue / (double)mgg->precision;
	gmt->z_scale_factor = 1.0;
	gmt->z_add_offset = 0.0;
}

int GMT_is_mgg2_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Determine if file is a GRD98 file */
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;
	int ok;

	if (!strcmp (header->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (GMT, header->name, GMT->current.io.r_mode)) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	GMT_memset (&mggHeader, 1, MGG_GRID_HEADER_2);
	if (GMT_fread (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1)
		return (GMT_GRDIO_READ_FAILED);

	/* Swap header bytes if necessary; ok is 0|1 if successful and -1 if bad file */
	ok = gmt_swap_mgg_header (&mggHeader);

	/* Check the magic number and size of header */
	if (ok == -1)
		return (-1);	/* Not this kind of file */
	header->type = GMT_GRID_IS_RF;
	return GMT_NOERROR;
}

int GMT_mgg2_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;
	int ok;

	if (!strcmp (header->name, "="))
		fp = GMT->session.std[GMT_IN];
	else if ((fp = GMT_fopen (GMT, header->name, GMT->current.io.r_mode)) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	GMT_memset (&mggHeader, 1, MGG_GRID_HEADER_2);
	if (GMT_fread (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) return (GMT_GRDIO_READ_FAILED);

	/* Swap header bytes if necessary; ok is 0|1 if successful and -1 if bad file */
	ok = gmt_swap_mgg_header (&mggHeader);

	/* Check the magic number and size of header */
	if (ok == -1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unrecognized header, expected 0x%04X saw 0x%04X\n", GRD98_MAGIC_NUM + GRD98_VERSION, mggHeader.version);
		return (GMT_GRDIO_GRD98_BADMAGIC);
	}

	if (mggHeader.length != sizeof (MGG_GRID_HEADER_2)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Invalid grid header size, expected %d, found %d\n", (int)sizeof (MGG_GRID_HEADER_2), mggHeader.length);
		return (GMT_GRDIO_GRD98_BADLENGTH);
	}

	GMT_fclose (GMT, fp);

	gmt_MGG2toGMT (&mggHeader, header);

	return (GMT_NOERROR);
}

int GMT_mgg2_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;
	int err;

	if (!strcmp (header->name, "="))
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (GMT, header->name, GMT->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	if ((err = gmt_GMTtoMGG2 (header, &mggHeader))) return (err);

	if (GMT_fwrite (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int GMT_mgg2_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{
	MGG_GRID_HEADER_2 mggHeader;
	FILE *fp = NULL;
	int *tLong = NULL;
	short *tShort = NULL;
	char *tChar = NULL;
	float *tFloat = NULL;
	bool piping = false, is_float = false;
	int j, first_col, last_col, first_row, last_row, swap_all = 0;
	unsigned int width_in, height_in;
	unsigned int i, width_out, *actual_col = NULL;
	uint64_t kk, ij, j2, imag_offset;
	off_t long_offset;	/* For fseek only */
	size_t n_expected;	/* Items in one row */
	size_t size;		/* Item size */

	GMT_memset (&mggHeader, 1, MGG_GRID_HEADER_2);
	if (!strcmp (header->name, "=")) {
		fp = GMT->session.std[GMT_IN];
		piping = true;
	}
	else if ((fp = GMT_fopen (GMT, header->name, GMT->current.io.r_mode)) != NULL) {
		if (GMT_fread (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) return (GMT_GRDIO_READ_FAILED);
		swap_all = gmt_swap_mgg_header (&mggHeader);
		if (swap_all == -1) return (GMT_GRDIO_GRD98_BADMAGIC);
		if (mggHeader.numType == 0) mggHeader.numType = sizeof (int);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	is_float = (mggHeader.numType < 0 && abs (mggHeader.numType) == (int)sizeof (float));	/* Float file */

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	n_expected = header->nx;
	tLong  = GMT_memory (GMT, NULL, n_expected, int);
	tShort = (short *)tLong;	tChar  = (char *)tLong;	tFloat  = (float *)tLong;
	size = abs (mggHeader.numType);

	if (piping)	{ /* Skip data by reading it */
		for (j = 0; j < first_row; j++) if (GMT_fread ( tLong, size, n_expected, fp) != n_expected) return (GMT_GRDIO_READ_FAILED);
	} else if (first_row) { /* Simply seek by it */
		long_offset = (off_t)first_row * (off_t)n_expected * size;
		if (fseek (fp, long_offset, 1)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		if (GMT_fread ( tLong, size, n_expected, fp) != n_expected) return (GMT_GRDIO_READ_FAILED);
		ij = imag_offset + (j2 + pad[YHI]) * width_out + pad[XLO];
		for (i = 0; i < width_in; i++) {
			kk = ij + i;
			if (mggHeader.numType == sizeof (int)) {
				if (swap_all) gmt_swap_long (&tLong[actual_col[i]]);
				if (tLong[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = (float) tLong[actual_col[i]] / (float) mggHeader.precision;
			}
			else if (is_float) {
				if (swap_all) gmt_swap_long (&tLong[actual_col[i]]);
				if (tLong[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = tFloat[actual_col[i]];
			}

			else if (mggHeader.numType == sizeof (short)) {
				if (swap_all) gmt_swap_word(&tShort[actual_col[i]]);
				if (tShort[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = (float) tShort[actual_col[i]] / (float) mggHeader.precision;
			}

			else if (mggHeader.numType == sizeof (char)) {
				if (tChar[actual_col[i]] == mggHeader.nanValue) grid[kk] = GMT->session.f_NaN;
				else grid[kk] = (float) tChar[actual_col[i]] / (float) mggHeader.precision;
			}
			else {
				return (GMT_GRDIO_UNKNOWN_TYPE);
			}
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z_min, z_max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping)	{ /* Skip data by reading it */
		int ny = header->ny;
		for (j = last_row + 1; j < ny; j++) if (GMT_fread ( tLong, size, n_expected, fp) != n_expected) return (GMT_GRDIO_READ_FAILED);
	}

	GMT_free (GMT, tLong);
	GMT_free (GMT, actual_col);

	header->nx = width_in;
	header->ny = height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int GMT_mgg2_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{
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
	float *tFloat = NULL;
	FILE *fp = NULL;

	if (!strcmp (header->name, "="))
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (GMT, header->name, GMT->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	check = !isnan (header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

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

	/* store header information and array */
	if ((err = gmt_GMTtoMGG2(header, &mggHeader))) return (err);;
	if (GMT_fwrite (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1U, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);
	is_float = (mggHeader.numType < 0 && abs (mggHeader.numType) == (int)sizeof (float));	/* Float file */

	tLong = GMT_memory (GMT, NULL, width_in, int);
	tShort = (short *) tLong;	tChar = (char *)tLong;	tFloat = (float *) tLong;

	i2 = first_col + pad[XLO];
	size = abs (mggHeader.numType);
	for (ju = 0, j2 = first_row + pad[YHI]; ju < height_out; ju++, j2++) {
		ij = imag_offset + j2 * width_in + i2;
		for (iu = 0; iu < width_out; iu++) {
			kk = ij+actual_col[iu];
			if (GMT_is_fnan (grid[kk])) {
				if (mggHeader.numType == sizeof (int))       tLong[iu]  = mggHeader.nanValue;
				else if (is_float) tFloat[iu]  = (float)mggHeader.nanValue;
				else if (mggHeader.numType == sizeof (short)) tShort[iu] = (short)mggHeader.nanValue;
				else if (mggHeader.numType == sizeof (char))  tChar[iu] = (char)mggHeader.nanValue;
				else {
					return (GMT_GRDIO_UNKNOWN_TYPE);
				}
			} else {
				if (grid[kk] > -0.1 && grid[kk] < 0) grid[kk] = (float)(-0.1);

				if (mggHeader.numType == sizeof (int))
					tLong[iu] = (int)rint ((double)grid[kk] * mggHeader.precision);
				else if (is_float)
					tFloat[iu] = grid[kk];
				else if (mggHeader.numType == sizeof (short))
					tShort[iu] = (short) rint((double)grid[kk] * mggHeader.precision);
				else if (mggHeader.numType == sizeof (char))
					tChar[iu] = (char) rint((double)grid[kk] * mggHeader.precision);
				else
					return (GMT_GRDIO_UNKNOWN_TYPE);
			}
		}
		if (GMT_fwrite (tLong, size, width_out, fp) != width_out) return (GMT_GRDIO_WRITE_FAILED);
	}
	GMT_free (GMT, tLong);
	GMT_free (GMT, actual_col);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}
