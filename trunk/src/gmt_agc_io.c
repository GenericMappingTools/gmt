/*      $Id: gmt_agc_io.c,v 1.1 2006-04-09 11:20:17 pwessel Exp $	*/
/*-----------------------------------------------------------
 * Format # :	21
 * Type :	Atlantic Geoscience Genter (AGC) format
 * Prefix :	GMT_agc_
 * Author :	Paul Wessel, by modifying code from Robert Helie
 * Date :	09-APR-2006
 * Purpose:	to transform to/from AGC grid file format
 * Functions :	GMT_agc_read_grd_info, GMT_agc_write_grd_info,
 *		GMT_agc_write_grd_info, GMT_agc_read_grd, GMT_agc_write_grd
 *-----------------------------------------------------------*/

# define ZBLOCKWIDTH 	40
# define ZBLOCKHEIGHT 	40
# define RECORDLENGTH 	1614
# define FIRSTZ  	12
# define REMARKSIZE	160
# define PREHEADSIZE	12
# define POSTHEADSIZE	2
# define BUFFHEADSIZE	(6 + POSTHEADSIZE)

# define AGCHEADINDICATOR	"agchd:"
# define HEADINDSIZE		6
# define PARAMSIZE	(int)((REMARKSIZE - HEADINDSIZE) / BUFFHEADSIZE)

# define AGCNODATA 0.0


int GMT_agc_read_grd_info (struct GRD_HEADER *header)
{
	FILE *fp;
	int i, one_or_zero = 0;
	float recdata[RECORDLENGTH];
	float agchead[BUFFHEADSIZE];
	void SaveAGCHeader (char *remark, float *agchead);

	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = GMT_fopen (header->name, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", header->name);
		exit (EXIT_FAILURE);
	}

	fread (recdata, sizeof(float), RECORDLENGTH, fp);
	
	header->y_min = recdata[0];
	header->y_max = recdata[1];
	header->x_min = recdata[2];
	header->x_max = recdata[3];
	header->y_inc = recdata[4];
	header->x_inc = recdata[5];
	for (i = 6; i < FIRSTZ; agchead[i - 6] = recdata[i], i++);
	agchead[BUFFHEADSIZE - 2] = recdata[RECORDLENGTH - 2];
	agchead[BUFFHEADSIZE - 1] = recdata[RECORDLENGTH - 1];
	header->nx = rint ((header->x_max - header->x_min) / header->x_inc) + 1 + one_or_zero;
	header->ny = rint ((header->y_max - header->y_min) / header->y_inc) + 1 + one_or_zero;
	header->y_order = (int)((header->y_max - header->y_min) / (ZBLOCKHEIGHT * header->y_inc) + 1.001);
	SaveAGCHeader (header->remark, agchead);
	
	if (fp != GMT_stdin) GMT_fclose (fp);

	return (FALSE);
}

int GMT_agc_write_grd_info (struct GRD_HEADER *header)
{
	int i;
	FILE *fp;
	float prez[PREHEADSIZE], postz[POSTHEADSIZE];

	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (header->name, "rb+")) == NULL && (fp = fopen (header->name, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", header->name);
		exit (EXIT_FAILURE);
	}
	
	prez[0] = (float)header->y_min;
	prez[1] = (float)header->y_max;
	prez[2] = (float)header->x_min;
	prez[3] = (float)header->x_max;
	prez[4] = (float)header->y_inc;
	prez[5] = (float)header->x_inc;

	for (i = 6; i < PREHEADSIZE; prez[i] = 0, i++);
	prez[PREHEADSIZE - 1] = 1614;
	postz[0] = postz[1] = 0;

	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);
}

int GMT_agc_read_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_in;			/* Number of columns in subregion */
	int inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	int i, j, ij, i_0_out;		/* Misc. counters */
	int *k;				/* Array with indices */
	int type;			/* Data type */
	int size;			/* Length of data type */
	int datablockcol, datablockrow, n_read, rowstart, rowend, colstart, colend, row, col;
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN piping = FALSE;		/* TRUE if we read input pipe instead of from file */
	BOOLEAN check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	void *tmp;			/* Array pointer for reading in rows of data */
	float z[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	void ReadRecord (FILE *fpi, int recnum, float *z);
	
	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (header->name, "rb")) != NULL)	/* Skip header */
		fseek (fp, (long) (RECORDLENGTH * sizeof(float)), SEEK_CUR);
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", header->name);
		exit (EXIT_FAILURE);
	}

	type = GMT_grdformats[header->type][1];
	size = GMT_grd_data_size (header->type, &header->nan_value);
	check = !GMT_is_dnan (header->nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */

	if (complex) {	/* Need twice as much output space since we load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}

	/* Allocate memory for one row of data (for reading purposes) */

	tmp = (void *) GMT_memory (VNULL, (size_t)header->nx, size, "GMT_native_read_grd");

	/* Rows are read south to north */
	
	datablockcol = datablockrow = 0;
	while ( !feof(fp) )  
	{
		ReadRecord(fp, n_read, (float *)z);
  		n_read++;
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN(rowstart + ZBLOCKHEIGHT, header->ny);
		for (i = 0, row = rowstart; row < rowend; i++, row++) 
		{
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN(colstart + ZBLOCKWIDTH, header->nx);
			for (j = 0, col = colstart; col < colend; j++, col++)
			{
				ij = (header->ny - 1 - row + pad[2]) * header->nx + col + pad[0];
				grid[ij] = (!(z[j][i])) ? header->nan_value : z[j][i];
			}
		}

		if (++datablockrow >= header->y_order)
		{
			datablockrow = 0;
			datablockcol++;
		}
	}

	if (fp != stdin) GMT_fclose (fp);
	
	return (FALSE);
}

int GMT_agc_write_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	float outz[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	int rowstart, rowend, colstart, colend, datablockcol, datablockrow;
	int i, j, ij, row, col, nlat;
	FILE *fp;
	float prez[PREHEADSIZE], postz[POSTHEADSIZE];
	void WriteRecord (FILE *file, float *rec, float *prerec, float *postrec);

	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (header->name, "rb+")) == NULL && (fp = fopen (header->name, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", header->name);
		exit (EXIT_FAILURE);
	}
	
	prez[0] = (float)header->y_min;
	prez[1] = (float)header->y_max;
	prez[2] = (float)header->x_min;
	prez[3] = (float)header->x_max;
	prez[4] = (float)header->y_inc;
	prez[5] = (float)header->x_inc;

	for (i = 6; i < PREHEADSIZE; prez[i] = 0, i++);
	prez[PREHEADSIZE - 1] = 1614;
	postz[0] = postz[1] = 0;

	nlat = (int)((prez[1] - prez[0]) / (ZBLOCKHEIGHT * prez[4]) + 1.001);
	datablockcol = datablockrow = 0;
	do 
	{
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN(rowstart + ZBLOCKHEIGHT, header->ny);
		for (i = 0, row = rowstart; row < rowend; i++, row++) 
		{
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN(colstart + ZBLOCKWIDTH, header->nx);
			for (j = 0, col = colstart; col < colend; j++, col++)
			{
				ij = (header->ny - 1 - row) * header->nx + col;
				outz[j][i] = (isnan((double)grid[ij])) ? header->nan_value : grid[ij];
			}
		} 

		WriteRecord (fp, (float*)outz, prez, postz);

		if (++datablockrow >= nlat)
		{
			datablockrow = 0;
			datablockcol++;
		}
	} while (rowend != header->ny || colend != header->nx);

	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);
}

void SaveAGCHeader (char *remark, float *agchead)
{
	char floatvalue[PARAMSIZE];
	int i, j;

	strcpy(remark, AGCHEADINDICATOR);
	for (i = 0; i < BUFFHEADSIZE; i++) {
		sprintf(floatvalue, "%f", agchead[i]);
		for (j = strlen(floatvalue); j < PARAMSIZE; j++) strcat (floatvalue, " ");
		strcat (remark, floatvalue);
	}
}

void ReadRecord (FILE *fpi, int recnum, float *z)
{ 
	int nitems;
	float garbage[FIRSTZ];

	fread (garbage, sizeof(float), FIRSTZ, fpi);
	nitems = fread(z, sizeof(float), ZBLOCKWIDTH * ZBLOCKHEIGHT, fpi);

	if (nitems != (ZBLOCKWIDTH * ZBLOCKHEIGHT) && !feof(fpi)) 	/* Bad stuff */
		fprintf(stderr, "Bad at rec # %d\n", recnum);
	fread (garbage, sizeof(float), RECORDLENGTH - (ZBLOCKWIDTH * ZBLOCKHEIGHT + FIRSTZ), fpi);
}

void WriteRecord (FILE *file, float *rec, float *prerec, float *postrec)
{
	fwrite (prerec, sizeof(float), PREHEADSIZE, file);
	fwrite (rec, sizeof(float), ZBLOCKWIDTH * ZBLOCKHEIGHT, file);
	fwrite( postrec, sizeof(float), POSTHEADSIZE, file); 
}

void WriteData (float *inz, FILE *outfile, int nx, int ny, float *prez, float *postz, float nodata)
{
	float outz[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	int rowstart, rowend, colstart, colend, datablockcol, datablockrow;
	int i, j, ij, row, col, nlat;
	void WriteRecord (FILE *file, float *rec, float *prerec, float *postrec);

	nlat = (int)((prez[1] - prez[0]) / (ZBLOCKHEIGHT * prez[4]) + 1.001);
	datablockcol = datablockrow = 0;
	do 
	{
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN(rowstart + ZBLOCKHEIGHT, ny);
		for (i = 0, row = rowstart; row < rowend; i++, row++) 
		{
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN(colstart + ZBLOCKWIDTH, nx);
			for (j = 0, col = colstart; col < colend; j++, col++)
			{
				ij = (ny - 1 - row) * nx + col;
				outz[j][i] = (isnan((double)inz[ij])) ? nodata : inz[ij];
			}
		} 

		WriteRecord (outfile, (float*)outz, prez, postz);

		if (++datablockrow >= nlat)
		{
			datablockrow = 0;
			datablockcol++;
		}
	} while ( rowend != ny || colend != nx );
}
