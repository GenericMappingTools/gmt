/*	$Id: gmt_mgg_header2.c,v 1.20 2007-03-05 21:47:10 pwessel Exp $
 *
 *	Code donated by David Divens, NOAA/NGDC
 *	Distributed under the GNU Public License (see COPYING for details)
 *	This is the README file:
 ------------------------------------------------------------------------
 Adding MGG file format to GMT
-----------------------------
1) Modify gmt_grdio.h
   - Increment GMT_N_GRD_FORMATS.
     This number is one based, the formats are zero based so this
     value will be one greater than the grid number you are creating.
     
2) Modify gmt_customio.c
   - Make sure both gmt_mgg_header2.h and gmt_mgg_header2.c are in the same
     directory as gmt_customio.c

   - Scroll down to the comment
     FORMAT # N:

   - Add the following lines:
    id = N;
    GMT_io_readinfo[id]  = (PFI) mgg2_read_grd_info;
    GMT_io_writeinfo[id] = (PFI) mgg2_write_grd_info;
    GMT_io_readgrd[id]   = (PFI) mgg2_read_grd;
    GMT_io_writegrd[id]  = (PFI) mgg2_write_grd;

   - Jump down to the end of the file and add this line:
    #include "gmt_mgg_header2.c"

2) Modify gmt_customio.h
   - At the end of this file add:
    #include "gmt_mgg_header2."

Build GMT with new format by typing:
> make all

Test format by checking a grid:
> grdinfo gnaXXXXX.g03=N
--------------------------------------------------------------------*/

/* Comments added by P. Wessel:
 *
 * 1) GRD98 can now support pixel grids since 11/24/2006.
 * 2) Learned that 1/x_inc and 1/y_inc are stored as integers.  This means
 *    GRD98 imposes restrictions on x_inc & y_inc.
 */

#include "gmt_mgg_header2.h"

#define MIN_PER_DEG		60.0
#define SEC_PER_MIN		60.0
#define SEC_PER_DEG		(SEC_PER_MIN * MIN_PER_DEG)
#define BYTE_SIZE

static double dms2degrees(int deg, int min, int sec)
{
    double  decDeg = (double)deg;

    decDeg += (double)min / MIN_PER_DEG;
    decDeg += (double)sec / SEC_PER_DEG;

    return decDeg;
}

static void degrees2dms(double degrees, int *deg, int *min, int *sec)
{
	/* Round off to the nearest half second */
	if (degrees < 0) degrees -= (0.5 / SEC_PER_DEG);

	*deg = (int)degrees;
	degrees -= *deg;

	degrees *= MIN_PER_DEG;
	*min = (int)(degrees);
	degrees -= *min;

	*sec = (int)(degrees * SEC_PER_MIN);
}

static void GMT2MGG2(struct GRD_HEADER *gmt, MGG_GRID_HEADER_2 *mgg)
{
	double f;
	memset(mgg, 0, sizeof(MGG_GRID_HEADER_2));
	
	mgg->version     = MGG_MAGIC_NUM + VERSION;
	mgg->length      = sizeof(MGG_GRID_HEADER_2);
	mgg->dataType    = 1;
	
	mgg->cellRegistration = gmt->node_offset;
	mgg->lonNumCells = gmt->nx;
	f  = gmt->x_inc * SEC_PER_DEG;
	mgg->lonSpacing  = (int)rint(f);
	if (fabs (f - (double)mgg->lonSpacing) > GMT_CONV_LIMIT) {
		fprintf (stderr, "%s: GRD98 format requires n = 1/x_inc to be an integer! Your n = %g (aborting)\n", GMT_program, f);
		GMT_exit (EXIT_FAILURE);
	}
	degrees2dms(gmt->x_min, &mgg->lonDeg, &mgg->lonMin, &mgg->lonSec);
	
	mgg->latNumCells = gmt->ny;
	f  = gmt->y_inc * SEC_PER_DEG;
	mgg->latSpacing  = (int)rint(gmt->y_inc * SEC_PER_DEG);
	if (fabs (f - (double)mgg->latSpacing) > GMT_CONV_LIMIT) {
		fprintf (stderr, "%s: GRD98 format requires n = 1/y_inc to be an integer! Your n = %g (aborting)\n", GMT_program, f);
		GMT_exit (EXIT_FAILURE);
	}
	degrees2dms(gmt->y_max, &mgg->latDeg, &mgg->latMin, &mgg->latSec);

	/* Default values */
	mgg->gridRadius  = -1;
	mgg->precision   = DEFAULT_PREC;
	mgg->nanValue    = MGG_NAN_VALUE;
	mgg->numType     = sizeof(int);
	mgg->minValue    = (int)rint(gmt->z_min * mgg->precision);
	mgg->maxValue    = (int)rint(gmt->z_max * mgg->precision);

	/* Data fits in two byte boundry */
	if ((-SHRT_MAX <= mgg->minValue) && (mgg->maxValue <= SHRT_MAX)) {
		mgg->numType = sizeof(short);
		mgg->nanValue = (short)SHRT_MIN;
	}
#ifdef BYTE_SIZE
	/* Data fits in one byte boundry */
	if ((gmt->z_min >= 0) && (gmt->z_max <= 127)) {
		mgg->numType   = sizeof(char);
		mgg->nanValue  = (char)255;
		mgg->precision = 1;
		mgg->minValue  = (int)gmt->z_min;
		mgg->maxValue  = (int)gmt->z_max;
	}
#endif
}

static void MGG2_2GMT(MGG_GRID_HEADER_2 *mgg, struct GRD_HEADER *gmt)
{
	int one_or_zero;
	
	memset(gmt, 0, sizeof(struct GRD_HEADER));
	
	gmt->type = GMT_grd_format_decoder ("rf");
	gmt->node_offset    = mgg->cellRegistration;
	one_or_zero	    = 1 - gmt->node_offset;
	gmt->nx             = mgg->lonNumCells;
	gmt->x_min          = dms2degrees(mgg->lonDeg, mgg->lonMin, mgg->lonSec);
	gmt->x_inc          = dms2degrees(0, 0, mgg->lonSpacing);
	gmt->x_max          = gmt->x_min + (gmt->x_inc * (gmt->nx - one_or_zero));

	gmt->ny             = mgg->latNumCells;
	gmt->y_max          = dms2degrees(mgg->latDeg, mgg->latMin, mgg->latSec);
	gmt->y_inc          = dms2degrees(0, 0, mgg->latSpacing);
	gmt->y_min          = gmt->y_max - (gmt->y_inc * (gmt->ny - one_or_zero));
 
	gmt->z_min          = (double)mgg->minValue / (double)mgg->precision;
	gmt->z_max          = (double)mgg->maxValue / (double)mgg->precision;
	gmt->z_scale_factor = 1.0;
	gmt->z_add_offset   = 0;
}

static void swap_word(void* ptr)
{
#ifdef GMTSWAP
   unsigned char *tmp = ptr;
   unsigned char a = tmp[0];
   tmp[0] = tmp[1];
   tmp[1] = a;
#endif
}

static void swap_long(void *ptr)
{
#ifdef GMTSWAP
   unsigned char *tmp = ptr;
   unsigned char a = tmp[0];
   tmp[0] = tmp[3];
   tmp[3] = a;

   a = tmp[1];
   tmp[1] = tmp[2];
   tmp[2] = a;
#endif
}

static void swap_header(MGG_GRID_HEADER_2 *header)
{
   int i;
   swap_long(&header->version);
   swap_long(&header->length);
   swap_long(&header->dataType);
   swap_long(&header->latDeg);
   swap_long(&header->latMin);
   swap_long(&header->latSec);
   swap_long(&header->latSpacing);
   swap_long(&header->latNumCells);
   swap_long(&header->lonDeg);
   swap_long(&header->lonMin);
   swap_long(&header->lonSec);
   swap_long(&header->lonSpacing);
   swap_long(&header->lonNumCells);
   swap_long(&header->minValue);
   swap_long(&header->maxValue);
   swap_long(&header->gridRadius);
   swap_long(&header->precision);
   swap_long(&header->nanValue);
   swap_long(&header->numType);
   swap_long(&header->waterDatum);
   swap_long(&header->dataLimit);
   swap_long(&header->cellRegistration);
   for (i = 0; i < GRD98_N_UNUSED; i++) swap_long(&header->unused[i]);
}

int GMT_is_mgg2_grid (char *file)
{	/* Determine if file is a GRD98 file */
	FILE *fp = NULL;
	MGG_GRID_HEADER_2 mggHeader;

	if (!strcmp(file, "=")) {	/* Cannot check on pipes */
		fprintf (stderr, "GMT Fatal Error: Cannot guess grid format type if grid is passed via pipe!\n");
		GMT_exit (EXIT_FAILURE);
	}
	if ((fp = GMT_fopen(file, GMT_io.r_mode)) == NULL) {
		fprintf(stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		GMT_exit (EXIT_FAILURE);
	}

	memset(&mggHeader, '\0', sizeof(MGG_GRID_HEADER_2));
	if (GMT_fread(&mggHeader, sizeof(MGG_GRID_HEADER_2), 1, fp) != 1) {
		fprintf(stderr, "GMT Fatal Error: Error reading file %s!\n", file);
		GMT_exit (EXIT_FAILURE);
	}

	/* Swap header bytes if necessary */
	swap_header(&mggHeader);

	/* Check the magic number and size of header */
	if (mggHeader.version < MGG_MAGIC_NUM + VERSION) return (-1);	/* Not this kind of file */
	return (GMT_grd_format_decoder ("rf"));
}

int mgg2_read_grd_info (struct GRD_HEADER *header)
{
	FILE			*fp = NULL;
	MGG_GRID_HEADER_2	mggHeader;

	if (!strcmp(header->name, "=")) {
		fp = GMT_stdin;
	} else if ((fp = GMT_fopen(header->name, GMT_io.r_mode)) == NULL) {
		fprintf(stderr, "GMT Fatal Error: Could not open file %s!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}

	memset(&mggHeader, '\0', sizeof(MGG_GRID_HEADER_2));
	if (GMT_fread(&mggHeader, sizeof(MGG_GRID_HEADER_2), 1, fp) != 1) {
		fprintf(stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}

	/* Swap header bytes if necessary */
	swap_header(&mggHeader);

	/* Check the magic number and size of header */
	if (mggHeader.version < MGG_MAGIC_NUM + VERSION) {
		fprintf(stderr, "GMT Fatal Error: Unrecognized header, expected 0x%04X saw 0x%04X\n", MGG_MAGIC_NUM + VERSION, mggHeader.version);
		GMT_exit (EXIT_FAILURE);
	}

	if (mggHeader.length != sizeof(MGG_GRID_HEADER_2)) {
		fprintf(stderr, "GMT Fatal Error: Invalid grid header size, expected %d, found %d\n", (int)sizeof(MGG_GRID_HEADER_2), mggHeader.length);
		GMT_exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdin) GMT_fclose(fp);
	
	MGG2_2GMT(&mggHeader, header);
	
	return FALSE;
}

int mgg2_write_grd_info (struct GRD_HEADER *header)
{
	FILE			*fp;
	MGG_GRID_HEADER_2	mggHeader;
	
	if (!strcmp(header->name, "=")) {
		fp = GMT_stdout;
	} else if ((fp = GMT_fopen(header->name, GMT_io.w_mode)) == NULL) {
		fprintf(stderr, "GMT Fatal Error: Could not create file %s!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}
	
	GMT2MGG2(header, &mggHeader);

	/* Swap header bytes if necessary */
	swap_header(&mggHeader);

	if (GMT_fwrite (&mggHeader, sizeof(MGG_GRID_HEADER_2), 1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdout) GMT_fclose (fp);
	
	return FALSE;
}

int mgg2_read_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int pad[], BOOLEAN complex)
{
	MGG_GRID_HEADER_2	mggHeader;
	FILE  *fp     = NULL;
	int  *tLong  = NULL;
	short *tShort = NULL;
	char  *tChar  = NULL;
	int first_col, last_col, first_row, last_row, nm, kk, one_or_zero;
	int i, j, j2, ij, width_in, width_out, height_in, i_0_out, inc = 1;
	BOOLEAN piping = FALSE, geo = FALSE;
	double half_or_zero, x, small;
	long long_offset;	/* For fseek only */
	
	if (complex) {
		fprintf (stderr, "GMT Fatal Error: MGG grdfile %s cannot hold complex data!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}

	if (!strcmp (header->name, "=")) {
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (header->name, GMT_io.r_mode)) != NULL) {
		GMT_fread(&mggHeader, 1, sizeof(MGG_GRID_HEADER_2), fp);
        swap_header(&mggHeader);
		if (mggHeader.numType == 0) mggHeader.numType = sizeof(int);
	}

	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file! (%s)", header->name);
		GMT_exit (EXIT_FAILURE);
	}
	
	if (w == 0.0 && e == 0.0) {	/* Get entire file and return */
		nm = header->nx * header->ny;
		tLong  = (int *) GMT_memory (CNULL, nm, sizeof (int), "mgg_read_grd");
		tShort = (short *) tLong;
		tChar  = (char *) tLong;

		if (GMT_fread ((void *)tLong, mggHeader.numType, nm, fp) != (size_t)nm) {
			fprintf (stderr, "GMT Fatal Error: Error reading file! (%s)", header->name);
			GMT_exit (EXIT_FAILURE);
		}
		for (i = 0; i < nm; i++) {
			/* 4-byte values */
			if (mggHeader.numType == sizeof(int)) {
                swap_long(&tLong[i]);
				if (tLong[i] == mggHeader.nanValue) grid[i] = GMT_f_NaN;
				else grid[i] = (float) tLong[i] / (float) mggHeader.precision;
			}

			/* 2-byte values */
			else if (mggHeader.numType == sizeof(short)) {
                swap_word(&tShort[i]);
				if (tShort[i] == mggHeader.nanValue) grid[i] = GMT_f_NaN;
				else grid[i] = (float) tShort[i] / (float) mggHeader.precision;;
			}
			
			/* 1-byte values */
			else if (mggHeader.numType == sizeof(char)) {
				if (tChar[i] == mggHeader.nanValue) grid[i] = GMT_f_NaN;
				else grid[i] = (float) tChar[i] / (float) mggHeader.precision;;

			}

			else {
				fprintf(stderr, "GMT Fatal Error: Unknown datalen %d\n", mggHeader.numType);
				exit(-1);
			}
		}
		GMT_free ((void *)tLong);
		GMT_fclose(fp);
		return (FALSE);
	}

	/* Must deal with a subregion */

	if (w < header->x_min || e > header->x_max) geo = TRUE;	/* Dealing with periodic grid */
	
	one_or_zero = (header->node_offset) ? 0 : 1;
	
	/* Get dimension of subregion */
	
	width_in  = irint ((e - w) / header->x_inc) + one_or_zero;
	height_in = irint ((n - s) / header->y_inc) + one_or_zero;
	
	/* Get first and last row and column numbers */
	
	small = 0.1 * header->x_inc;
	first_col = (int)floor ((w - header->x_min + small) / header->x_inc);
	last_col  = (int)ceil ((e - header->x_min - small) / header->x_inc) - 1 + one_or_zero;
	small = 0.1 * header->y_inc;
	first_row = (int)floor ((header->y_max - n + small) / header->y_inc);
	last_row  = (int)ceil ((header->y_max - s - small) / header->y_inc) - 1 + one_or_zero;

	if ((last_col - first_col + 1) > width_in) last_col--;
	if ((last_row - first_row + 1) > height_in) last_row--;
	if ((last_col - first_col + 1) > width_in) first_col++;
	if ((last_row - first_row + 1) > height_in) first_row++;
	
	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];
	
	i_0_out = pad[0];		/* Edge offset in output */
	
	if (geo) {	/* Must rollover in longitudes */
		int *k;
		tLong  = (int *) GMT_memory (CNULL, header->nx, sizeof (int), "mgg_read_grd");
		tShort = (short *)tLong;
		tChar  = (char *)tLong;

		k = (int *) GMT_memory (CNULL, width_in, sizeof (int), "mgg_read_grd");
		
		half_or_zero = (header->node_offset) ? 0.5 : 0.0;
		small = 0.1 * header->x_inc;	/* Anything smaller than 0.5 dx will do */
		for (i = 0; i < width_in; i++) {
			x = w + (i + half_or_zero) * header->x_inc;
			if ((header->x_min - x) > small)
				x += 360.0;
			else if ((x - header->x_max) > small)
				x -= 360.0;
			k[i] = GMT_x_to_i (x, header->x_min, header->x_inc, half_or_zero, header->nx);
		}
		if (piping)	{ /* Skip data by reading it */
			for (j = 0; j < first_row; j++) {
				if (GMT_fread ((void *) tLong, mggHeader.numType, header->nx, fp) != (size_t)header->nx) {
					fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
					GMT_exit (EXIT_FAILURE);
				}
			}
		} else { /* Simply seek by it */
			long_offset = (long)(first_row * header->nx * mggHeader.numType);
			GMT_fseek (fp, long_offset, 1);
		}
		
		for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
			if (GMT_fread ((void *) tLong, mggHeader.numType, header->nx, fp) != (size_t)header->nx) {
				fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
				GMT_exit (EXIT_FAILURE);
			}
			
			ij = (j2 + pad[3]) * width_out + i_0_out;
			for (i = 0; i < width_in; i++) {
				kk = ij+i*inc;
				if (mggHeader.numType == sizeof(int)) {
					swap_long(&tLong[k[i]]);
					if (tLong[k[i]] == mggHeader.nanValue) grid[kk] = GMT_f_NaN;
					else grid[kk] = (float) tLong[k[i]] / (float) mggHeader.precision;
				}
				
				else if (mggHeader.numType == sizeof(short)) {
					swap_word(&tShort[k[i]]);
					if (tShort[k[i]] == mggHeader.nanValue) grid[kk] = GMT_f_NaN;
					else grid[kk] = (float) tShort[k[i]] / (float) mggHeader.precision;;
				}
				
				else if (mggHeader.numType == sizeof(char)) {
					if (tChar[k[i]] == mggHeader.nanValue) grid[kk] = GMT_f_NaN;
					else grid[kk] = (float) tChar[k[i]] / (float) mggHeader.precision;;
				}

				else {
					fprintf(stderr, "GMT Fatal Error: Unknown datalen %d\n", mggHeader.numType);
					exit(-1);
				}
			}
		}
		if (piping)	{ /* Skip data by reading it */
			for (j = last_row + 1; j < header->ny; j++) {
				if (GMT_fread ((void *) tLong, mggHeader.numType, header->nx, fp) != (size_t)header->nx) {
					fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
					GMT_exit (EXIT_FAILURE);
				}
			}
		}
			
		GMT_free ((void *)k);
	}
	else {	/* A bit easier here */
		tLong = (int *) GMT_memory (CNULL, header->nx, sizeof (int), "mgg_read_grd");
		tShort = (short *) tLong;
		tChar  = (char *) tLong;

		if (piping)	{ /* Skip data by reading it */
			for (j = 0; j < first_row; j++) {
				if (GMT_fread ((void *) tLong, mggHeader.numType, header->nx, fp) != (size_t)header->nx) {
					fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
					GMT_exit (EXIT_FAILURE);
				}
			}
		} else {/* Simply seek by it */
			long_offset = (long) (first_row * header->nx * mggHeader.numType);
			GMT_fseek (fp, long_offset, 1);
		}
		
		for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
			ij = (j2 + pad[3]) * width_out + i_0_out;
			if (GMT_fread ((void *) tLong, mggHeader.numType, header->nx, fp) != (size_t)header->nx) {
				fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
				GMT_exit (EXIT_FAILURE);
			}
			
			for (i = 0; i < width_in; i++) {
				kk = ij+i;
				/* 4-byte values */
				if (mggHeader.numType == sizeof(int)) {
					swap_long(&tLong[first_col + i]);
					if (tLong[first_col + i] == mggHeader.nanValue) grid[kk] = GMT_f_NaN;
					else grid[kk] = (float)tLong[first_col + i] / (float) mggHeader.precision;
				}
				
				/* 2-byte values */
				else if (mggHeader.numType == sizeof(short)) {
					swap_word(&tShort[first_col + i]);
					if (tShort[first_col + i] == mggHeader.nanValue) grid[kk] = GMT_f_NaN;
					else grid[kk] = (float) tShort[first_col + i] / (float) mggHeader.precision;;
				} 
				
				/* 1-byte values */
				else if (mggHeader.numType == sizeof(char)) {
					if (tChar[first_col + i] == mggHeader.nanValue) grid[kk] = GMT_f_NaN;
					else grid[kk] = (float) tChar[first_col + i] / (float) mggHeader.precision;;
				}

				else {
					fprintf(stderr, "GMT Fatal Error: Unknown datalen %d\n", mggHeader.numType);
					exit(-1);
				}
			}
		}

		if (piping)	{/* Skip data by reading it */
			for (j = last_row + 1; j < header->ny; j++) {
				if (GMT_fread ((void *) tLong, mggHeader.numType, header->nx, fp)) {
					fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", header->name);
					GMT_exit (EXIT_FAILURE);
				}
			}
		}
	}
	
	header->nx = width_in;
	header->ny = height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = 0; j < header->ny; j++) {
		for (i = 0; i < header->nx; i++) {
			ij = (j + pad[3]) * width_out + i + pad[0];
			if (GMT_is_fnan ((double) grid[ij])) continue;
			header->z_min = MIN (header->z_min, grid[ij]);
			header->z_max = MAX (header->z_max, grid[ij]);
		}
	}
	if (fp != stdin) fclose (fp);
	
	GMT_free ((void *)tLong);
	return (FALSE);
}

	
int mgg2_write_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	MGG_GRID_HEADER_2	mggHeader;
	int i, i2, nm, kk, *k;
	int j, ij, j2, width_in, width_out, height_out, one_or_zero;
	int first_col, last_col, first_row, last_row;
	
	BOOLEAN geo = FALSE;
	double half_or_zero, small, x;
	
	int  *tLong;
	short *tShort;
	char  *tChar;
	FILE *fp;
	
	if (complex) {
		fprintf (stderr, "GMT Fatal Error: MGG grdfile %s cannot hold complex data!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}
	if (!strcmp (header->name, "=")) {
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (header->name, GMT_io.w_mode)) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}
	
	if (w == 0.0 && e == 0.0) {	/* Write entire file and return */
		/* Find min/max of data */
		
		nm = header->nx * header->ny;
		header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
		for (i = 0; i < nm; i++) {
			ij = (complex) ? 2 * i : i;
			if (GMT_is_fnan ((double)grid[ij])) continue;
			header->z_min = MIN (header->z_min, grid[ij]);
			header->z_max = MAX (header->z_max, grid[ij]);
		}
		GMT2MGG2(header, &mggHeader);
		swap_header(&mggHeader);
		if (GMT_fwrite (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1, fp) != 1) {
			fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", header->name);
			GMT_exit (EXIT_FAILURE);
		}
		swap_header(&mggHeader);

		tLong  = (int *) GMT_memory (CNULL, nm, sizeof (int), "mgg_write_grd");
		tShort = (short*)tLong;
		tChar  = (char*)tLong;
		
		for (i = 0; i < nm; i++) {
			if (GMT_is_fnan ((double)grid[i])) {
				if (mggHeader.numType == sizeof(int)) {
			       tLong[i]  = mggHeader.nanValue;
			       swap_long(&tLong[i]);
				} else if (mggHeader.numType == sizeof(short)) {
					tShort[i] = (short)mggHeader.nanValue;
					swap_word(&tShort[i]);
				} else if (mggHeader.numType == sizeof(char)) {
					tChar[i]  = (char)mggHeader.nanValue;
				} else {
					fprintf(stderr, "GMT Fatal Error: Unknown numType %d\n", mggHeader.numType);
					exit(-1);
				}
			} else {
				if (grid[i] > -0.1 && grid[i] < 0.0) grid[i] = (float)(-0.1);

				if (mggHeader.numType == sizeof(int)) {
					tLong[i] = (int)rint((double)grid[i] * mggHeader.precision);
					swap_long(&tLong[i]);
				}
				
				else if (mggHeader.numType == sizeof(short)) {
					tShort[i] = (short) rint((double)grid[i] * mggHeader.precision);
					swap_word(&tShort[i]);
				}
				
				else if (mggHeader.numType == sizeof(char)) {
					tChar[i]  = (char) rint((double)grid[i] * mggHeader.precision);
				}

				else {
					fprintf(stderr, "GMT Fatal Error: Unknown numType %d\n", mggHeader.numType);
					exit(-1);
				}
			}
		}
		if (GMT_fwrite (tLong, mggHeader.numType, nm, fp) != (size_t)nm) {
			fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", header->name);
			GMT_exit (EXIT_FAILURE);
		}
		GMT_free ((void *)tLong);
		return (FALSE);
	}

	if (w < header->x_min || e > header->x_max) geo = TRUE;	/* Dealing with periodic grid */
	
	one_or_zero = (header->node_offset) ? 0 : 1;
	
	/* Get dimension of subregion to write */
	
	width_out  = irint ((e - w) / header->x_inc) + one_or_zero;
	height_out = irint ((n - s) / header->y_inc) + one_or_zero;
	
	/* Get first and last row and column numbers */
	
	small     = 0.1 * header->x_inc;
	first_col = (int)floor ((w - header->x_min + small) / header->x_inc);
	last_col  = (int)ceil ((e - header->x_min - small) / header->x_inc) -1 + one_or_zero;
	small     = 0.1 * header->y_inc;
	first_row = (int)floor ((header->y_max - n + small) / header->y_inc);
	last_row  = (int)ceil ((header->y_max - s - small) / header->y_inc) -1 + one_or_zero;
	
	if ((last_col - first_col + 1) > width_out) last_col--;
	if ((last_row - first_row + 1) > height_out) last_row--;
	if ((last_col - first_col + 1) > width_out) first_col++;
	if ((last_row - first_row + 1) > height_out) first_row++;

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	
	/* Find xmin/zmax */
	
	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[3]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[0]; i <= last_col; i++, i2++) {
			ij = j2 * width_in + i2;
			if (GMT_is_fnan ((double)grid[ij])) continue;
			header->z_min = MIN (header->z_min, grid[ij]);
			header->z_max = MAX (header->z_max, grid[ij]);
		}
	}
	
	/* store header information and array */
	GMT2MGG2(header, &mggHeader);
	swap_header(&mggHeader);
	if (GMT_fwrite (&mggHeader, sizeof (MGG_GRID_HEADER_2), 1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", header->name);
		GMT_exit (EXIT_FAILURE);
	}
	swap_header(&mggHeader);

	tLong  = (int *) GMT_memory (CNULL, width_in, sizeof (int), "mgg_write_grd");
	tShort = (short *) tLong;
	tChar  = (char *)  tLong;
	
	if (geo) {
		k = (int *) GMT_memory (CNULL, width_out, sizeof (int), "mgg_write_grd");
		
		half_or_zero = (header->node_offset) ? 0.5 : 0.0;
		small = 0.1 * header->x_inc;	/* Anything smaller than 0.5 dx will do */
		for (i = 0; i < width_out; i++) {
			x = w + (i + half_or_zero) * header->x_inc;
			if ((header->x_min - x) > small)
				x += 360.0;
			else if ((x - header->x_max) > small)
				x -= 360.0;
			k[i] = GMT_x_to_i (x, header->x_min, header->x_inc, half_or_zero, header->nx);
		}
		i2 = first_col + pad[0];
		for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
			ij = j2 * width_in + i2;
			for (i = 0; i < width_out; i++) {
				kk = ij+k[i];
				if (GMT_is_fnan ((double)grid[kk])) {
					if (mggHeader.numType == sizeof(int))       tLong[i]  = mggHeader.nanValue;
					else if (mggHeader.numType == sizeof(short)) tShort[i] = (short)mggHeader.nanValue;
					else if (mggHeader.numType == sizeof(char))  tChar[i] = (char)mggHeader.nanValue;
					else {
						fprintf(stderr, "GMT Fatal Error: Unknown numType %d\n", mggHeader.numType);
						exit(-1);
					}
				} else {
					if (grid[kk] > -0.1 && grid[kk] < 0) grid[kk] = (float)(-0.1);

					if (mggHeader.numType == sizeof(int)) {
						tLong[i] = (int)rint ((double)grid[kk] * mggHeader.precision);
					}
					
					else if (mggHeader.numType == sizeof(short)) {
						tShort[i] = (short) rint((double)grid[kk] * mggHeader.precision);
					}
					
					else if (mggHeader.numType == sizeof(char)) {
						tChar[i] = (char) rint((double)grid[kk] * mggHeader.precision);
					}
					
					else {
						fprintf(stderr, "GMT Fatal Error: Unknown numType %d\n", mggHeader.numType);
						exit(-1);
					}
				}
				if (mggHeader.numType == sizeof(int)) swap_long(&tLong[i]);
				else if (mggHeader.numType == sizeof(short)) swap_word(&tShort[i]);
			}
			GMT_fwrite ((void *)tLong, mggHeader.numType, width_out, fp);
		}
		GMT_free ((void *)k);
	}
	else {
		i2 = first_col + pad[0];
		for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
			ij = j2 * width_in + i2;
			for (i = 0; i < width_out; i++) {
				kk = ij+i;
				if (GMT_is_fnan ((double)grid[kk])) {
					if (mggHeader.numType == sizeof(int))       tLong[i]  = mggHeader.nanValue;
					else if (mggHeader.numType == sizeof(short)) tShort[i] = (short)mggHeader.nanValue;
					else if (mggHeader.numType == sizeof(char))  tChar[i] = (char)mggHeader.nanValue;
					else {
						fprintf(stderr, "GMT Fatal Error: Unknown numType %d\n", mggHeader.numType);
						exit(-1);
					}
				} else {
					if (grid[kk] > -0.1 && grid[kk] < 0) grid[kk] = (float)(-0.1);
					if (mggHeader.numType == sizeof(int)) {
						tLong[i] = (int) rint ((double)grid[kk] * mggHeader.precision);
					}
					
					else if (mggHeader.numType == sizeof(short)) {
						tShort[i] = (short) rint ((double)grid[kk] * mggHeader.precision);
					}
					
					else if (mggHeader.numType == sizeof(char)) {
						tChar[i] = (char) rint ((double)grid[kk] * mggHeader.precision);
					}

					else {
						fprintf(stderr, "GMT Fatal Error: Unknown numType %d\n", mggHeader.numType);
						exit(-1);
					}
				}
				if (mggHeader.numType == sizeof(int)) swap_long(&tLong[i]);
				else if (mggHeader.numType == sizeof(short)) swap_word(&tShort[i]);
			}
			GMT_fwrite ((void *)tLong, mggHeader.numType, width_out, fp);
		}
	}
	if (fp != GMT_stdout) GMT_fclose (fp);
	
	GMT_free ((void *)tLong);
	
	return (FALSE);
}
