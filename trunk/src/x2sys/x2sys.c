/*-----------------------------------------------------------------
 *	$Id: x2sys.c,v 1.149 2011-03-05 21:24:29 guru Exp $
 *
 *      Copyright (c) 1999-2011 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/* x2sys.c contains the source code for the X2SYS crossover library
 * libx2sys.a.  The code is copylefted under the GNU Public Library
 * License.
 *
 * The following functions are external and user-callable form other
 * programs:
 *
 * x2sys_set_system	: Initialize X2SYS via the specified TAG
 * x2sys_initialize	: Reads the definition info file for current data files
 * x2sys_read_file	: Reads and returns the entire data matrix
 * x2sys_read_gmtfile	: Specifically reads an old .gmt file
 * x2sys_read_mgd77file : Specifically reads an ASCII MGD77 file
 * x2sys_read_mgd77ncfile : Specifically reads an netCDF MGD77+ file
 * x2sys_read_ncfile	: Specifically reads an COARDS netCDF file
 * x2sys_read_list	: Read an ascii list of track names
 * x2sys_dummytimes	: Make dummy times for tracks missing times
 * x2sys_n_data_cols	: Gives number of data columns in this data set
 * x2sys_fopen		: Opening files with error message and exit
 * x2sys_fclose		: Closes files and gives error messages
 * x2sys_skip_header	: Skips the header record(s) in the open file
 * x2sys_read_record	: Reads and returns one record from the open file
 * x2sys_pick_fields	: Decodes the -F<fields> flag of desired columns
 * x2sys_free_info	: Frees the information structure
 * x2sys_free_data	: Frees the data matrix
 * x2sys_read_coe_dbase : Reads into memory the entire COE ascii database
 * x2sys_free_coe_dbase : Free the array of COE structures
 *------------------------------------------------------------------
 * Core crossover functions are part of GMT:
 * GMT_init_track	: Prepares a track for crossover analysis
 * GMT_crossover	: Calculates crossovers for two data sets
 * GMT_x_alloc		: Allocate space for crossovers
 * GMT_x_free		: Free crossover structure
 * GMT_ysort		: Sorting routine used in x2sys_init_track [Hidden]
 *------------------------------------------------------------------
 * These routines are local to x2sys and used by the above routines:
 *
 * x2sys_set_home	: Initializes X2SYS paths
 * x2sys_record_length	: Returns the record length of current file
 *
 *------------------------------------------------------------------
 * Author:	Paul Wessel
 * Date:	20-SEP-2008
 * Version:	1.1, based on the spirit of the old xsystem code
 *
 */

#include "x2sys.h"

/* Global variables used by X2SYS functions */

char *X2SYS_HOME;
char *X2SYS_program;

char *x2sys_xover_format = "%9.5lf %9.5lf %10.1lf %10.1lf %9.2lf %9.2lf %9.2lf %8.1lf %8.1lf %8.1lf %5.1lf %5.1lf\n";
char *x2sys_xover_header = "%s %ld %s %ld\n";
char *x2sys_header = "> %s %ld %s %ld %s\n";
struct MGD77_CONTROL M;

void x2sys_set_home (void);
int x2sys_record_length (struct X2SYS_INFO *s);
int get_first_year (double t);

#define MAX_DATA_PATHS 32
char *x2sys_datadir[MAX_DATA_PATHS];	/* Directories where track data may live */
int n_x2sys_paths = 0;			/* Number of these directories */

void x2sys_path (char *fname, char *path)
{
	sprintf (path, "%s%c%s", X2SYS_HOME, DIR_DELIM, fname);
}

FILE *x2sys_fopen (char *fname, char *mode)
{
	FILE *fp;
	char file[BUFSIZ];

	if (mode[0] == 'w') {	/* Writing: Do this only in X2SYS_HOME */
		x2sys_path (fname, file);
		fp = fopen (file, mode);
	}
	else {			/* Reading: Try both current directory and X2SYS_HOME */
		if ((fp = fopen (fname, mode)) == NULL) {	/* Not in current directory, try $X2SYS_HOME */
			x2sys_path (fname, file);
			fp = fopen (file, mode);
		}
	}
	return (fp);
}

int x2sys_access (char *fname,  int mode)
{
	int k;
	char file[BUFSIZ];
	x2sys_path (fname, file);
	if ((k = access (file, mode))) {	/* Not in X2SYS_HOME directory */
		k = access (fname, mode);	/* Try in current directory */
	}
	return (k);
}

int x2sys_fclose (char *fname, FILE *fp)
{

	if (fclose (fp)) return (X2SYS_FCLOSE_ERR);
	return (X2SYS_NOERROR);
}

void x2sys_skip_header (FILE *fp, struct X2SYS_INFO *s)
{
	int i;
	char line[BUFSIZ], *unused = NULL;

	if (s->file_type == X2SYS_ASCII) {	/* ASCII, skip records */
		for (i = 0; i < s->skip; i++) unused = fgets (line, BUFSIZ, fp);
	}
	else if (s->file_type == X2SYS_BINARY) {			/* Native binary, skip bytes */
		fseek (fp, (long)s->skip, SEEK_CUR);
	}
}

int x2sys_initialize (char *TAG, char *fname, struct GMT_IO *G,  struct X2SYS_INFO **I)
{
	/* Reads the format definition file and sets all information variables */

	int i = 0, c;
	size_t n_alloc = GMT_TINY_CHUNK;
	FILE *fp;
	struct X2SYS_INFO *X;
	char line[BUFSIZ], cardcol[80], yes_no;

	x2sys_set_home ();

	X = (struct X2SYS_INFO *) GMT_memory (VNULL, n_alloc, sizeof (struct X2SYS_INFO), "x2sys_initialize");
	X->TAG = strdup (TAG);
	X->info = (struct X2SYS_DATA_INFO *) GMT_memory (VNULL, n_alloc, sizeof (struct X2SYS_DATA_INFO), "x2sys_initialize");
	X->file_type = X2SYS_ASCII;
	X->x_col[GMT_IN] = X->y_col[GMT_IN] = X->t_col[GMT_IN] = -1;
	X->ms_flag = '>';	/* Default multisegment header flag */
	sprintf (line, "%s%c%s.def", TAG, DIR_DELIM, fname);
	X->dist_flag = 0;	/* Cartesian distances */

	if ((fp = x2sys_fopen (line, "r")) == NULL) return (X2SYS_BAD_DEF);

	X->unit[X2SYS_DIST_SELECTION][0] = 'k';		X->unit[X2SYS_DIST_SELECTION][1] = '\0';	/* Initialize for geographic data (km ad m/s) */
	X->unit[X2SYS_SPEED_SELECTION][0] = 'e';	X->unit[X2SYS_SPEED_SELECTION][1] = '\0';
	if (!strcmp (fname, "gmt")) {
		X->read_file = (PFI) x2sys_read_gmtfile;
		X->geographic = TRUE;
		X->geodetic = 0;
		X->dist_flag = 2;	/* Creat circle distances */
	}
	else if (!strcmp (fname, "mgd77+")) {
		X->read_file = (PFI) x2sys_read_mgd77ncfile;
		X->geographic = TRUE;
		X->geodetic = 0;
		X->dist_flag = 2;	/* Creat circle distances */
		MGD77_Init (&M);	/* Initialize MGD77 Machinery */
	}
	else if (!strcmp (fname, "mgd77")) {
		X->read_file = (PFI) x2sys_read_mgd77file;
		X->geographic = TRUE;
		X->geodetic = 0;
		X->dist_flag = 2;	/* Creat circle distances */
		MGD77_Init (&M);	/* Initialize MGD77 Machinery */
	}
	else {
		X->read_file = (PFI) x2sys_read_file;
		X->dist_flag = 0;			/* Cartesian distances */
		X->unit[X2SYS_DIST_SELECTION][0] = 'c';	/* Reset to Cartesian */
		X->unit[X2SYS_SPEED_SELECTION][0] = 'c';	/* Reset to Cartesian */
	}
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '\0') continue;
		if (line[0] == '#') {
			if (!strncmp (line, "#SKIP",   (size_t)5)) X->skip = atoi (&line[6]);
			if (!strncmp (line, "#ASCII",  (size_t)6)) X->file_type = X2SYS_ASCII;
			if (!strncmp (line, "#BINARY", (size_t)7)) X->file_type = X2SYS_BINARY;
			if (!strncmp (line, "#NETCDF", (size_t)7)) X->file_type = X2SYS_NETCDF;
			if (!strncmp (line, "#GEO", (size_t)4)) X->geographic = TRUE;
			if (!strncmp (line, "#MULTISEG", (size_t)9)) {
				X->multi_segment = TRUE;
				sscanf (line, "%*s %c", &X->ms_flag);
			}
			continue;
		}
		GMT_chop (line);	/* Remove trailing CR or LF */

		sscanf (line, "%s %c %c %lf %lf %lf %s %s", X->info[i].name, &X->info[i].intype, &yes_no, &X->info[i].nan_proxy, &X->info[i].scale, &X->info[i].offset, X->info[i].format, cardcol);
		if (X->info[i].intype == 'A') {	/* ASCII Card format */
			sscanf (cardcol, "%d-%d", &X->info[i].start_col, &X->info[i].stop_col);
			X->info[i].n_cols = X->info[i].stop_col - X->info[i].start_col + 1;
		}
		c = (int)X->info[i].intype;
		if (tolower (c) == 'a') X->file_type = X2SYS_ASCII;
		c = (int)yes_no;
		if (tolower (c) != 'Y') X->info[i].has_nan_proxy = TRUE;
		if (!(X->info[i].scale == 1.0 && X->info[i].offset == 0.0)) X->info[i].do_scale = TRUE;
		if (!strcmp (X->info[i].name, "x") || !strcmp (X->info[i].name, "lon"))  X->x_col[GMT_IN] = i;
		if (!strcmp (X->info[i].name, "y") || !strcmp (X->info[i].name, "lat"))  X->y_col[GMT_IN] = i;
		if (!strcmp (X->info[i].name, "t") || !strcmp (X->info[i].name, "time")) X->t_col[GMT_IN] = i;
		i++;
		if (i == (int)n_alloc) {
			n_alloc <<= 1;
			X->info = (struct X2SYS_DATA_INFO *) GMT_memory ((void *)X->info, n_alloc, sizeof (struct X2SYS_DATA_INFO), "x2sys_initialize");
		}

	}
	fclose (fp);
	if (X->file_type == X2SYS_NETCDF) X->read_file = (PFI) x2sys_read_ncfile;
	
	if (i < (int)n_alloc) X->info = (struct X2SYS_DATA_INFO *) GMT_memory ((void *)X->info, (size_t)i, sizeof (struct X2SYS_DATA_INFO), "x2sys_initialize");
	X->n_fields = X->n_out_columns = i;

	if (X->file_type == X2SYS_BINARY) {	/* Binary mode needed */
		strcpy (G->r_mode, "rb");
		strcpy (G->w_mode, "wb");
		strcpy (G->a_mode, "ab+");
	}
	X->out_order  = (int *) GMT_memory (VNULL, sizeof (int), (size_t)X->n_fields, "x2sys_initialize");
	X->use_column = (int *) GMT_memory (VNULL, sizeof (int), (size_t)X->n_fields, "x2sys_initialize");
	for (i = 0; i < X->n_fields; i++) {	/* Default is same order and use all columns */
		X->out_order[i] = i;
		X->use_column[i] = 1;
		G->in_col_type[i] = G->out_col_type[i] = (X->x_col[GMT_IN] == i) ? GMT_IS_LON : ((X->y_col[GMT_IN] == i) ? GMT_IS_LAT : GMT_IS_UNKNOWN);
	}
	X->x_col[GMT_OUT] = X->x_col[GMT_IN];
	X->y_col[GMT_OUT] = X->y_col[GMT_IN];
	X->t_col[GMT_OUT] = X->t_col[GMT_IN];
	X->n_data_cols = x2sys_n_data_cols (X);
	X->rec_size = (8 + X->n_data_cols) * sizeof (double);

	*I = X;
	return (X2SYS_NOERROR);
}

void x2sys_end (struct X2SYS_INFO *X)
{	/* Free allcoated memory */
	int id;
	if (X2SYS_HOME) GMT_free ((void *)X2SYS_HOME);
	if (!X) return;
	if (X->out_order) GMT_free ((void *)X->out_order);
	if (X->use_column) GMT_free ((void *)X->use_column);
	free ((void *)X->TAG);	/* free since allocated by strdup */
	x2sys_free_info (X);
	for (id = 0; id < n_x2sys_paths; id++) GMT_free  ((void *)x2sys_datadir[id]);
	MGD77_end (&M);
}

int x2sys_record_length (struct X2SYS_INFO *s)
{
	int i, rec_length = 0;

	for (i = 0; i < s->n_fields; i++) {
		switch (s->info[i].intype) {
			case 'c':
			case 'u':
				rec_length += 1;
				break;
			case 'h':
				rec_length += 2;
				break;
			case 'i':
			case 'f':
				rec_length += 4;
				break;
			case 'l':
				rec_length += sizeof (long);
				break;
			case 'd':
				rec_length += 8;
				break;
		}
	}
	return (rec_length);
}

int x2sys_n_data_cols (struct X2SYS_INFO *s)
{
	int i, n = 0;

	for (i = 0; i < s->n_out_columns; i++) {	/* Loop over all possible fields in this data set */
		if (i == s->x_col[GMT_IN]) continue;
		if (i == s->y_col[GMT_IN]) continue;
		if (i == s->t_col[GMT_IN]) continue;
		n++;	/* Only count data columns */
	}

	return (n);
}

int x2sys_pick_fields (char *string, struct X2SYS_INFO *s)
{
	/* Scan the -Fstring and select which columns to use and which order
	 * they should appear on output.  Default is all columns and the same
	 * order as on input.  Once this is set you can loop through i = 0:n_out_columns
	 * and use out_order[i] to get the original column number.
	 */

	char line[BUFSIZ], p[BUFSIZ];
	int i = 0, j;
	GMT_LONG pos = 0;

	strncpy (s->fflags, string, (size_t)BUFSIZ);
	strncpy (line, string, (size_t)BUFSIZ);	/* Make copy for later use */
	memset ((void *)s->use_column, 0, (size_t)(s->n_fields * sizeof (int)));

	while ((GMT_strtok (line, ",", &pos, p))) {
		j = 0;
		while (j < s->n_fields && strcmp (p, s->info[j].name)) j++;
		if (j < s->n_fields) {
			s->out_order[i] = j;
			s->use_column[j] = 1;
			/* Reset x,y,t indices */
			if (j == s->x_col[GMT_IN]) s->x_col[GMT_OUT] = i;
			if (j == s->y_col[GMT_IN]) s->y_col[GMT_OUT] = i;
			if (j == s->t_col[GMT_IN]) s->t_col[GMT_OUT] = i;
		}
		else {
			fprintf (stderr, "X2SYS: ERROR: Unknown column name %s\n", p);
			return (X2SYS_BAD_COL);
		}
		i++;
	}

	s->n_out_columns = i;
	
	return (X2SYS_NOERROR);
}

void x2sys_set_home (void)
{
	char *this;

	if (X2SYS_HOME) return;	/* Already set elsewhere */

	if ((this = getenv ("X2SYS_HOME")) != CNULL) {	/* Set user's default path */
		X2SYS_HOME = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 1), (size_t)1, "x2sys_set_home");
		strcpy (X2SYS_HOME, this);
	}
	else {	/* Default to the x2sys dir under GMT_SHAREDIR */
		X2SYS_HOME = (char *) GMT_memory (VNULL, (size_t)(strlen (GMT_SHAREDIR) + 7), (size_t)1, "x2sys_set_home");
		sprintf (X2SYS_HOME, "%s%cx2sys", GMT_SHAREDIR, DIR_DELIM);
	}
}

void x2sys_free_info (struct X2SYS_INFO *s)
{
	GMT_free ((void *)s->info);
	GMT_free ((void *)s);
}

void x2sys_free_data (double **data, int n, struct X2SYS_FILE_INFO *p)
{
	int i;

	for (i = 0; i < n; i++) {
		if (data[i]) GMT_free ((void *)data[i]);
	}
	GMT_free ((void *)data);
	if (p->ms_rec) GMT_free ((void *)p->ms_rec);
}

double *x2sys_dummytimes (GMT_LONG n)
{
	GMT_LONG i;
	double *t;

	/* Make monotonically increasing dummy time sequence */

	t = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "x2sys_dummytimes");

	for (i = 0; i < n; i++) t[i] = (double)i;

	return (t);
}

/*
 * x2sys_data_read:  Read subroutine for x2_sys data input.
 * This function will read one logical record of ascii or
 * binary data from the open file, and return with a double
 * array called data[] with each data value in it.
 */

int x2sys_read_record (FILE *fp, double *data, struct X2SYS_INFO *s, struct GMT_IO *G)
{
	int j, k, i;
	size_t n_read = 0;
	GMT_LONG pos, error = FALSE;
	char line[BUFSIZ], buffer[GMT_TEXT_LEN], p[BUFSIZ], c;
	unsigned char u;
	short int h;
	float f;
	long L;
	double NaN;

	GMT_make_dnan(NaN);

	for (j = 0; !error && j < s->n_fields; j++) {

		switch (s->info[j].intype) {

			case 'A':	/* ASCII Card Record, must extract columns */
				if (j == 0) {
					s->ms_next = FALSE;
					if (!fgets (line, BUFSIZ, fp)) return (-1);
					while (line[0] == '#' || line[0] == s->ms_flag) {
						if (!fgets (line, BUFSIZ, fp)) return (-1);
						if (s->multi_segment) s->ms_next = TRUE;
					}
					GMT_chop (line);	/* Remove trailing CR or LF */
				}
				strncpy (buffer, &line[s->info[j].start_col], (size_t)s->info[j].n_cols);
				buffer[s->info[j].n_cols] = 0;
				if (GMT_scanf (buffer, G->in_col_type[j], &data[j]) == GMT_IS_NAN) data[j] = GMT_d_NaN;
				break;

			case 'a':	/* ASCII Record, get all columns directly */
				k = 0;
				s->ms_next = FALSE;
				if (!fgets (line, BUFSIZ, fp)) return (-1);
				while (line[0] == '#' || line[0] == s->ms_flag) {
					if (!fgets (line, BUFSIZ, fp)) return (-1);
					if (s->multi_segment) s->ms_next = TRUE;
				}
				GMT_chop (line);	/* Remove trailing CR or LF */
				pos = 0;
				while ((GMT_strtok (line, " ,\t\n", &pos, p)) && k < s->n_fields) {
					if (GMT_scanf (p, G->in_col_type[k], &data[k]) == GMT_IS_NAN) data[k] = GMT_d_NaN;
					k++;;
				}
				return ((k != s->n_fields) ? -1 : 0);
				break;

			case 'c':	/* Binary signed 1-byte character */
				n_read += fread ((void *)&c, sizeof (char), (size_t)1, fp);
				data[j] = (double)c;
				break;

			case 'u':	/* Binary unsigned 1-byte character */
				n_read += fread ((void *)&u, sizeof (unsigned char), (size_t)1, fp);
				data[j] = (double)u;
				break;

			case 'h':	/* Binary signed 2-byte integer */
				n_read += fread ((void *)&h, sizeof (short int), (size_t)1, fp);
				data[j] = (double)h;
				break;

			case 'i':	/* Binary signed 4-byte integer */
				n_read += fread ((void *)&i, sizeof (int), (size_t)1, fp);
				data[j] = (double)i;
				break;

			case 'l':	/* Binary signed 4/8-byte integer (long) */
				n_read += fread ((void *)&L, sizeof (long), (size_t)1, fp);
				data[j] = (double)L;
				break;

			case 'f':	/* Binary signed 4-byte float */
				n_read += fread ((void *)&f, sizeof (float), (size_t)1, fp);
				data[j] = (double)f;
				break;

			case 'd':	/* Binary signed 8-byte float */
				n_read += fread ((void *)&data[j], sizeof (double), (size_t)1, fp);
				break;

			default:
				error = TRUE;
				break;
		}
	}

	/* Change nan-proxies to NaNs and apply any data scales and offsets */

	for (i = 0; i < s->n_fields; i++) {
		if (s->info[i].has_nan_proxy && data[i] == s->info[i].nan_proxy)
			data[i] = NaN;
		else if (s->info[i].do_scale)
			data[i] = data[i] * s->info[i].scale + s->info[i].offset;
		if (GMT_is_dnan (data[i])) s->info[i].has_nans = TRUE;
		if (i == s->x_col[GMT_IN] && s->geographic) GMT_lon_range_adjust (s->geodetic, &data[i]);
	}

	return ((error || n_read != s->n_fields) ? -1 : 0);
}
 
int x2sys_read_file (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec)
{
	/* Reads the the file given and returns the selected columns (or all)
	 * from all data records.  The data matrix is return in the pointer data.
	 */

	GMT_LONG i, j;
	size_t n_alloc;
	FILE *fp;
	double **z, *field;
	char path[BUFSIZ];

	if (x2sys_get_data_path (path, fname, s->suffix)) {
		fprintf (stderr, "x2sys_read_file : Cannot find track %s\n", fname);
  		return (-1);
	}
	if ((fp = fopen (path, G->r_mode)) == NULL) {
		fprintf (stderr, "x2sys_read_file : Cannot open file %s\n", path);
  		return (-1);
	}
	strcpy (s->path, path);

	n_alloc = GMT_CHUNK;

	field = (double *) GMT_memory (VNULL, (size_t)s->n_fields, sizeof (double), "x2sys_read_file");
	z = (double **) GMT_memory (VNULL, (size_t)s->n_out_columns, sizeof (double *), "x2sys_read_file");
	for (i = 0; i < s->n_out_columns; i++) z[i] = (double *) GMT_memory (VNULL, n_alloc, sizeof (double), "x2sys_read_file");
	p->ms_rec = (GMT_LONG *) GMT_memory (VNULL, n_alloc, sizeof (GMT_LONG), "x2sys_read_file");
	x2sys_skip_header (fp, s);
	p->n_segments = (s->multi_segment) ? -1 : 0;	/* So that first increment sets it to 0 */

	j = 0;
	while (!x2sys_read_record (fp, field, s, G)) {	/* Gets the next data record */
		/* Only copy the requested fields */
		for (i = 0; i < s->n_out_columns; i++) z[i][j] = field[s->out_order[i]];
		if (s->multi_segment && s->ms_next) p->n_segments++;
		p->ms_rec[j] = p->n_segments;
		j++;
		if (j == (GMT_LONG)n_alloc) {	/* Get more */
			n_alloc <<= 1;
			for (i = 0; i < s->n_out_columns; i++) z[i] = (double *) GMT_memory ((void *)z[i], n_alloc, sizeof (double), "x2sys_read_file");
			p->ms_rec = (GMT_LONG *) GMT_memory ((void *)p->ms_rec, n_alloc, sizeof (GMT_LONG), "x2sys_read_file");
		}
	}

	fclose (fp);
	GMT_free ((void *)field);
	for (i = 0; i < s->n_out_columns; i++) z[i] = (double *) GMT_memory ((void *)z[i], (size_t)j, sizeof (double), "x2sys_read_file");
	p->ms_rec = (GMT_LONG *) GMT_memory ((void *)p->ms_rec, (size_t)j, sizeof (GMT_LONG), "x2sys_read_file");

	*data = z;

	p->n_rows = j;
	p->year = 0;
	strncpy (p->name, fname, (size_t)32);
	*n_rec = p->n_rows;

	return (X2SYS_NOERROR);
}

int x2sys_read_gmtfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec)
{
	/* Reads the entire contents of the file given and returns the
	 * number of data records.  The data matrix is return in the
	 * pointer data.  The input file format is the venerable GMT
	 * MGG format from old Lamont by Wessel and Smith.
	 */

	int year, n_records, rata_day;
	GMT_LONG i, j;
	char path[BUFSIZ];
	FILE *fp;
	double **z, field[6];
	double NaN, t_off;
	struct GMTMGG_REC record;

	GMT_make_dnan(NaN);

 	if (n_x2sys_paths) {
  		if (x2sys_get_data_path (path, fname, s->suffix)) return (GMT_GRDIO_FILE_NOT_FOUND);
	}
	else {
		char name[80];
		if (!(s->flags & 1)) {	/* Must init gmt file paths */
			gmtmggpath_init (GMT_SHAREDIR);
			s->flags |= 1;
		}
		strncpy (name, fname, (size_t)80);
		if (strstr (fname, ".gmt")) name[strlen(fname)-4] = 0;	/* Name includes .gmt suffix, remove it */
	  	if (gmtmggpath_func (path, name)) return (GMT_GRDIO_FILE_NOT_FOUND);
		
	}
	strcpy (s->path, path);
	if ((fp = fopen (path, G->r_mode)) == NULL) {
		fprintf (stderr, "x2sys_read_file : Cannot open file %s\n", path);
  		return (-1);
	}

	if (fread ((void *)&year, sizeof (int), (size_t)1, fp) != 1) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not read leg year from %s\n", path);
		return (-1);
	}
	p->year = year;
	rata_day = GMT_rd_from_gymd (year, 1, 1);	/* Get the rata day for start of cruise year */
	t_off = GMT_rdc2dt (rata_day, 0.0);		/* Secs to start of day */
	
	if (fread ((void *)&n_records, sizeof (int), (size_t)1, fp) != 1) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not read n_records from %s\n", path);
		return (GMT_GRDIO_READ_FAILED);
	}
	p->n_rows = n_records;
	memset ((void *)p->name, 0, (size_t)32);

	if (fread ((void *)p->name, (size_t)10, sizeof (char), fp) != 1) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not read agency from %s\n", path);
		return (GMT_GRDIO_READ_FAILED);
	}

	z = (double **) GMT_memory (VNULL, (size_t)s->n_out_columns, sizeof (double *), "x2sys_read_gmtfile");
	for (i = 0; i < s->n_out_columns; i++) z[i] = (double *) GMT_memory (VNULL, (size_t)p->n_rows, sizeof (double), "x2sys_read_gmtfile");

	for (j = 0; j < p->n_rows; j++) {

		if (fread ((void *)&record, (size_t)18, (size_t)1, fp) != 1) {
			fprintf (stderr, "x2sys_read_gmtfile: Could not read record %ld from %s\n", j, path);
			return (GMT_GRDIO_READ_FAILED);
		}
		/* Convert the 6 items to doubles */
		field[0] = record.time * gmtdefs.time_system.i_scale + t_off;	/* To get GMT time keeping */
		field[1] = record.lat * MDEG2DEG;
		field[2] = record.lon * MDEG2DEG;
		field[3] = (record.gmt[0] == GMTMGG_NODATA) ? NaN : 0.1 * record.gmt[0];
		field[4] = (record.gmt[1] == GMTMGG_NODATA) ? NaN : record.gmt[1];
		field[5] = (record.gmt[2] == GMTMGG_NODATA) ? NaN : record.gmt[2];
		/* Only copy the requested fields */
		for (i = 0; i < s->n_out_columns; i++) z[i][j] = field[s->out_order[i]];
	}

	fclose (fp);

	p->ms_rec = NULL;
	p->n_segments = 0;

	*n_rec = p->n_rows;
	*data = z;

	return (X2SYS_NOERROR);
}

int x2sys_read_mgd77file (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec)
{
	GMT_LONG i, j;
	int col[MGD77_N_DATA_EXTENDED], n_alloc = GMT_CHUNK;
	char path[BUFSIZ], *tvals[MGD77_N_STRING_FIELDS];
	double **z, dvals[MGD77_N_DATA_EXTENDED];
	struct MGD77_HEADER H;
	struct MGD77_CONTROL M;
	double NaN;

	GMT_make_dnan(NaN);
	MGD77_Init (&M);			/* Initialize MGD77 Machinery */

  	if (n_x2sys_paths) {
  		if (x2sys_get_data_path (path, fname, s->suffix)) return (GMT_GRDIO_FILE_NOT_FOUND);
		if (MGD77_Open_File (path, &M, 0)) return (GMT_GRDIO_OPEN_FAILED);
	}
	else if (MGD77_Open_File (fname, &M, 0))
		return (GMT_GRDIO_FILE_NOT_FOUND);
	strcpy (s->path, M.path);
	
	if (MGD77_Read_Header_Record (fname, &M, &H)) {
		fprintf (stderr, "%s: Error reading header sequence for cruise %s\n", X2SYS_program, fname);
		return (GMT_GRDIO_READ_FAILED);
	}

	for (i = 0; i < MGD77_N_STRING_FIELDS; i++) tvals[i] = (char *) GMT_memory (VNULL, (size_t)9, sizeof (char), "x2sys_read_mgd77file");
	z = (double **) GMT_memory (VNULL, (size_t)s->n_fields, sizeof (double *), "x2sys_read_mgd77file");
	for (i = 0; i < s->n_fields; i++) z[i] = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "x2sys_read_mgd77file");
	for (i = 0; i < s->n_out_columns; i++) {
		col[i] = MGD77_Get_Column (s->info[s->out_order[i]].name, &M);
	}

	p->year = 0;
	j = 0;
	while (!MGD77_Read_Data_Record (&M, &H, dvals, tvals)) {		/* While able to read a data record */
		GMT_lon_range_adjust (s->geodetic, &dvals[MGD77_LONGITUDE]);
		for (i = 0; i < s->n_out_columns; i++) z[i][j] = dvals[col[i]];
		if (p->year == 0 && !GMT_is_fnan (dvals[0])) p->year = get_first_year (dvals[0]);
		j++;
		if (j == n_alloc) {
			n_alloc <<= 1;
			for (i = 0; i < s->n_fields; i++) z[i] = (double *) GMT_memory ((void *)z[i], (size_t)n_alloc, sizeof (double), "x2sys_read_mgd77file");
		}
	}
	MGD77_Close_File (&M);
	MGD77_Free_Header_Record (&M, &H);	/* Free up header structure */
	MGD77_end (&M);

	strncpy (p->name, fname, (size_t)32);
	p->n_rows = j;
	for (i = 0; i < s->n_fields; i++) z[i] = (double *) GMT_memory ((void *)z[i], (size_t)p->n_rows, sizeof (double), "x2sys_read_mgd77file");

	p->ms_rec = NULL;
	p->n_segments = 0;
	for (i = 0; i < MGD77_N_STRING_FIELDS; i++) GMT_free ((void *)tvals[i]);

	*data = z;
	*n_rec = p->n_rows;
	
	return (X2SYS_NOERROR);
}

int get_first_year (double t)
{
	/* obtain yyyy/mm/dd and return year */
	GMT_cal_rd rd;
	double s;
	struct GMT_gcal CAL;
	GMT_dt2rdc (t, &rd, &s);
	GMT_gcal_from_rd (rd, &CAL);
	return (CAL.year);
}

int x2sys_read_mgd77ncfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec)
{
	int i;
	char path[BUFSIZ];
	double **z;
	struct MGD77_DATASET *S;
	struct MGD77_CONTROL M;

	MGD77_Init (&M);			/* Initialize MGD77 Machinery */
	M.format  = MGD77_FORMAT_CDF;		/* Set input file's format to netCDF */
	for (i = 0; i < MGD77_N_FORMATS; i++) MGD77_format_allowed[i] = (M.format == i) ? TRUE : FALSE;	/* Only allow the specified input format */

	for (i = 0; i < s->n_out_columns; i++) strcpy (M.desired_column[i], s->info[s->out_order[i]].name);	/* Set all the required fields */
	M.n_out_columns = s->n_out_columns;
	
	S = MGD77_Create_Dataset ();	/* Get data structure w/header */

  	if (n_x2sys_paths) {
  		if (x2sys_get_data_path (path, fname, s->suffix)) return (GMT_GRDIO_FILE_NOT_FOUND);
		if (MGD77_Open_File (path, &M, 0)) return (GMT_GRDIO_OPEN_FAILED);
	}
	else if (MGD77_Open_File (fname, &M, 0))
		return (GMT_GRDIO_FILE_NOT_FOUND);
	strcpy (s->path, M.path);

	if (MGD77_Read_Header_Record (fname, &M, &S->H)) {	/* Returns info on all columns */
		fprintf (stderr, "x2sys_read_mgd77ncfile: Error reading header sequence for cruise %s\n", fname);
     		return (GMT_GRDIO_READ_FAILED);
	}

	if (MGD77_Read_Data (fname, &M, S)) {	/* Only gets the specified columns and barfs otherwise */
		fprintf (stderr, "x2sys_read_mgd77ncfile: Error reading data set for cruise %s\n", fname);
     		return (GMT_GRDIO_READ_FAILED);
	}
	MGD77_Close_File (&M);

	z = (double **) GMT_memory (VNULL, (size_t)M.n_out_columns, sizeof (double *), "x2sys_read_mgd77ncfile");
	for (i = 0; i < M.n_out_columns; i++) z[i] = (double *) S->values[i];

	strncpy (p->name, fname, (size_t)32);
	p->n_rows = S->H.n_records;
	p->ms_rec = NULL;
	p->n_segments = 0;
	p->year = S->H.meta.Departure[0];
	for (i = 0; i < MGD77_N_SETS; i++) if (S->flags[i]) GMT_free ((void *)S->flags[i]);
	MGD77_Free_Header_Record (&M, &(S->H));	/* Free up header structure */
	GMT_free ((void *)S);
	MGD77_end (&M);

	*data = z;
	*n_rec = p->n_rows;

	return (X2SYS_NOERROR);
}

int x2sys_read_ncfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec)
{
	int i, n_fields, n_expect = GMT_MAX_COLUMNS;
	size_t j;
	char path[BUFSIZ];
	double **z, *in;
	FILE *fp;
	
  	if (x2sys_get_data_path (path, fname, s->suffix)) return (GMT_GRDIO_FILE_NOT_FOUND);
	strcat (path, "?");	/* Set all the required fields */
	for (i = 0; i < s->n_out_columns; i++) {
		if (i) strcat (path, "/");
		strcat (path, s->info[s->out_order[i]].name);
	}
	
	strcpy (s->path, path);

	GMT_parse_b_option ("c");	/* Tell GMT this is a netCDF file */
	
	if ((fp = GMT_fopen (path, "r")) == NULL)  {	/* Error in opening file */
		fprintf (stderr, "x2sys_read_ncfile: Error opening file %s\n", fname);
     		return (GMT_GRDIO_READ_FAILED);
	}

	z = (double **) GMT_memory (VNULL, (size_t)s->n_out_columns, sizeof (double *), "x2sys_read_ncfile");
	for (i = 0; i < s->n_out_columns; i++) z[i] = GMT_memory (VNULL, (size_t)GMT_io.ndim, sizeof (double), "x2sys_read_ncfile");

	for (j = 0; j < GMT_io.ndim; j++) {
		if ((n_fields = GMT_input (fp, &n_expect, &in)) != s->n_out_columns) {
			fprintf (stderr, "x2sys_read_ncfile: Error reading file %s at record %ld\n", fname, (GMT_LONG)j);
	     		return (GMT_GRDIO_READ_FAILED);
		}
		for (i = 0; i < s->n_out_columns; i++) z[i][j] = in[i];
	}
	strncpy (p->name, fname, (size_t)32);
	p->n_rows = GMT_io.ndim;
	p->ms_rec = NULL;
	p->n_segments = 0;
	p->year = 0;
	GMT_fclose (fp);

	*data = z;
	*n_rec = p->n_rows;

	return (X2SYS_NOERROR);
}

int x2sys_xover_output (FILE *fp, int n, double out[])
{
	/* Write old xover formatted output.  This assumes data files are .gmt files */

	/* y x t1 t2 X1 X2 X3 M1 M2 M3 h1 h2 */

	fprintf (fp, x2sys_xover_format, out[1], out[0], out[2], out[3], out[9], out[11], out[13], out[8], out[10], out[12], out[6], out[7]);
	return (12);
}

int x2sys_read_list (char *file, char ***list, int *nf)
{	/* Read a file with track names or track file names; strip off extensions */
	int n_alloc = GMT_CHUNK, k, dot, n = 0;
	char **p, line[BUFSIZ], name[GMT_TEXT_LEN];
	FILE *fp;

	*list = NULL;	*nf = 0;
	if ((fp = x2sys_fopen (file, "r")) == NULL) {
  		fprintf (stderr, "x2sys_read_list : Cannot find track list file %s in either current or X2SYS_HOME directories\n", file);
		return (GMT_GRDIO_FILE_NOT_FOUND);
	}
	
	p = (char **) GMT_memory (VNULL, (size_t)n_alloc, sizeof (char *), "x2sys_read_list");

	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '>' || line[0] == '\0') continue;	/* Skip various comments and blank lines */
		GMT_chop (line);	/* Remove trailing CR or LF */
		sscanf (line, "%s", name);
		for (k = strlen(name) - 1, dot = -1; dot == -1 && k >= 0; k--) if (name[k] == '.') dot = k;
		if (dot > 0) name[dot] = '\0';	/* Chop off extension */
		p[n] = strdup (name);
		n++;
		if (n == n_alloc) {
			n_alloc <<= 1;
			p = (char **) GMT_memory ((void *)p, (size_t)n_alloc, sizeof (char *), "x2sys_read_list");
		}
	}
	fclose (fp);

	p = (char **) GMT_memory ((void *)p, (size_t)n, sizeof (char *), "x2sys_read_list");

	*list = p;
	*nf = n;

	return (X2SYS_NOERROR);
}

int x2sys_read_weights (char *file, char ***list, double **weights, int *nf)
{
	int n_alloc = GMT_CHUNK, n = 0;
	char **p, line[BUFSIZ], name[GMT_TEXT_LEN];
	double *W, this_w;
	FILE *fp;

	*list = NULL;	*weights = NULL, *nf = 0;
	if ((fp = x2sys_fopen (file, "r")) == NULL) return (GMT_GRDIO_FILE_NOT_FOUND);	/* Quietly return if no file is found since name may be a weight */
	
	p = (char **) GMT_memory (VNULL, (size_t)n_alloc, sizeof (char *), "x2sys_read_weights");
	W = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "x2sys_read_weights");

	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '>' || line[0] == '\0') continue;	/* Skip various comments and blank lines */
		GMT_chop (line);	/* Remove trailing CR or LF */
		if (sscanf (line, "%s %lg", name, &this_w) != 2) {
	 		fprintf (stderr, "x2sys_read_weights : Error parsing file %s near line %d\n", file, n);
			return (GMT_GRDIO_FILE_NOT_FOUND);
			
		}
		p[n] = strdup (name);
		W[n] = this_w;
		n++;
		if (n == n_alloc) {
			n_alloc <<= 1;
			p = (char **) GMT_memory ((void *)p, (size_t)n_alloc, sizeof (char *), "x2sys_read_weights");
		}
	}
	fclose (fp);

	p = (char **) GMT_memory ((void *)p, (size_t)n, sizeof (char *), "x2sys_read_weights");
	W = (double *) GMT_memory ((void *)W, (size_t)n_alloc, sizeof (double), "x2sys_read_weights");

	*list = p;
	*weights = W;
	*nf = n;

	return (X2SYS_NOERROR);
}

void x2sys_free_list (char **list, int n)
{	/* Properly free memory allocated by x2sys_read_list */
	int i;
	for (i = 0; i < n; i++) free ((void *)list[i]);
	if (list) GMT_free ((void *)list);
}

int x2sys_set_system (char *TAG, struct X2SYS_INFO **S, struct X2SYS_BIX *B, struct GMT_IO *G)
{
	char tag_file[BUFSIZ], line[BUFSIZ], p[BUFSIZ], sfile[BUFSIZ], suffix[16], unit[2][2];
	int geodetic = 0, n, k, dist_flag = 0;
	GMT_LONG pos = 0;
	double dist;
	GMT_LONG geographic = FALSE, n_given[2] = {FALSE, FALSE}, c_given = FALSE;
	FILE *fp;
	struct X2SYS_INFO *s;
	
	if (!TAG) return (X2SYS_TAG_NOT_SET);
	
	x2sys_set_home ();

	memset ((void *)B, 0, sizeof (struct X2SYS_BIX));
	memset ((void *)unit, 0, 4*sizeof (char));
	B->bin_x = B->bin_y = 1.0;
	B->x_min = 0.0;	B->x_max = 360.0;	B->y_min = -90.0;	B->y_max = +90.0;
	B->time_gap = B->dist_gap = dist = DBL_MAX;	/* Default is no data gap */
	B->periodic = sfile[0] = suffix[0] = 0;

	sprintf (tag_file, "%s%c%s.tag", TAG, DIR_DELIM, TAG);
	if ((fp = x2sys_fopen (tag_file, "r")) == NULL) {	/* Not in current directory */
		fprintf (stderr,"%s: Could not find/open file %s either in current of X2SYS_HOME directories\n", X2SYS_program, tag_file);
		return (GMT_GRDIO_FILE_NOT_FOUND);
	}
	
	while (fgets (line, BUFSIZ, fp) && line[0] == '#');	/* Skip comment records */
	GMT_chop (line);	/* Remove trailing CR or LF */

	while ((GMT_strtok (line, " \t", &pos, p))) {	/* Process the -C -D -G -I -N -M -R -W arguments from the header */
		if (p[0] == '-') {
			switch (p[1]) {
				/* Common parameters */
				case 'M':
				case 'R':
				case 'm':
					if (GMT_parse_common_options (p, &B->x_min, &B->x_max, &B->y_min, &B->y_max)) {
						fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
						return (GMT_GRDIO_READ_FAILED);
					}
					break;

				/* Supplemental parameters */

				case 'C':	/* Distance calculation flag */
					if (p[2] == 'c') dist_flag = 0;
					if (p[2] == 'f') dist_flag = 1;
					if (p[2] == 'g') dist_flag = 2;
					if (p[2] == 'e') dist_flag = 3;
					if (dist_flag < 0 || dist_flag > 3) {
						fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
						return (X2SYS_BAD_ARG);
					}
					c_given = TRUE;
					break;
				case 'D':
					strcpy (sfile, &p[2]);
					break;
				case 'E':
					strcpy (suffix, &p[2]);
					break;
				case 'G':	/* Geographical coordinates, set discontinuity */
					geographic = TRUE;
					geodetic = 0;
					if (p[2] == 'd') geodetic = 2;
					break;
				case 'I':
					if (GMT_getinc (&p[2], &B->bin_x, &B->bin_y)) {
						fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
						return (GMT_GRDIO_READ_FAILED);
					}
					break;
				case 'N':	/* Distance and speed unit selection */
					switch (p[2]) {
						case 'd':	/* Distance unit selection */
							k = X2SYS_DIST_SELECTION;
							break;
						case 's':	/* Speed unit selection */
							k = X2SYS_SPEED_SELECTION;
							break;
						default:
							fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
							return (X2SYS_BAD_ARG);
							break;
					}
					if (k == X2SYS_DIST_SELECTION || k == X2SYS_SPEED_SELECTION) {
						unit[k][0] = p[3];
						if (!strchr ("cekmn", (int)unit[k][0])) {
							fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
							return (X2SYS_BAD_ARG);
						}
						n_given[k] = TRUE;
					}
					break;
				case 'W':
					switch (p[2]) {
						case 't':
							B->time_gap = atof (&p[3]);
							break;
						case 'd':
							B->dist_gap = atof (&p[3]);
							break;
						default:	/* Backwards compatible with old -Wtgap/dgap option */
							n = sscanf (&p[2], "%lf/%lf", &B->time_gap, &dist);
							if (n == 2) B->dist_gap = dist;
						break;
					}
					break;
				default:
					fprintf (stderr, "%s: Bad arg in x2sys_set_system! (%s)\n", X2SYS_program, p);
					return (X2SYS_BAD_ARG);
					break;
			}
		}
	}
	x2sys_err_pass (x2sys_fclose (tag_file, fp), tag_file);
	
	x2sys_err_pass (x2sys_initialize (TAG, sfile, G, &s), sfile);	/* Initialize X2SYS and info structure */

	if (B->time_gap < 0.0) {
		fprintf (stderr, "%s: Error -Wt: maximum gap must be > 0!\n", GMT_program);
		exit (EXIT_FAILURE);
	}
	if (B->dist_gap < 0.0) {
		fprintf (stderr, "%s: Error -Wd: maximum gap must be > 0!\n", GMT_program);
		exit (EXIT_FAILURE);
	}
	if (geographic) {
		if (! (n_given[X2SYS_DIST_SELECTION] || n_given[X2SYS_SPEED_SELECTION])) {	/* Set defaults for geographic data */
			unit[X2SYS_DIST_SELECTION][0] = 'k';
			unit[X2SYS_SPEED_SELECTION][0] = 'e';
			n_given[X2SYS_DIST_SELECTION] = n_given[X2SYS_SPEED_SELECTION] = TRUE;
		}
		if (!c_given) {	/* Default is great circle distances */
			dist_flag = 2;
			c_given = TRUE;
		}
		if (geodetic == 0 && (B->x_min < 0 || B->x_max < 0)) {
			fprintf (stderr, "%s: Your -R and -G settings are contradicting each other!\n", X2SYS_program);
			return (X2SYS_CONFLICTING_ARGS);
		}
		else if  (geodetic == 2 && (B->x_min > 0 && B->x_max > 0)) {
			fprintf (stderr, "%s: Your -R and -G settings are contradicting each other!\n", X2SYS_program);
			return (X2SYS_CONFLICTING_ARGS);
		}
		if (c_given && dist_flag == 0) {
			fprintf (stderr, "%s: Your -C and -G settings are contradicting each other!\n", X2SYS_program);
			return (X2SYS_CONFLICTING_ARGS);
		}
		if (n_given[X2SYS_DIST_SELECTION] && unit[X2SYS_DIST_SELECTION][0] == 'c') {
			fprintf (stderr, "%s: Your -Nd and -G settings are contradicting each other!\n", X2SYS_program);
			return (X2SYS_CONFLICTING_ARGS);
		}
		if (n_given[X2SYS_SPEED_SELECTION] && unit[X2SYS_SPEED_SELECTION][0] == 'c') {
			fprintf (stderr, "%s: Your -Ns and -G settings are contradicting each other!\n", X2SYS_program);
			return (X2SYS_CONFLICTING_ARGS);
		}
		s->geographic = TRUE;
		s->geodetic = geodetic;	/* Override setting */
		if (GMT_360_RANGE (B->x_max, B->x_min)) B->periodic = 1;
	}
	if (n_given[X2SYS_DIST_SELECTION]) s->unit[X2SYS_DIST_SELECTION][0] = unit[X2SYS_DIST_SELECTION][0];
	if (n_given[X2SYS_SPEED_SELECTION]) s->unit[X2SYS_SPEED_SELECTION][0] = unit[X2SYS_SPEED_SELECTION][0];
	if (c_given) s->dist_flag = dist_flag;
	if (GMT_io.multi_segments[GMT_IN]) {	/* Files have multiple segments; make sure this is also set in s */
		s->multi_segment = TRUE;
		s->ms_flag = GMT_io.EOF_flag[GMT_IN];
	}
	if (suffix[0])
		strcpy (s->suffix, suffix);
	else
		strcpy (s->suffix, sfile);
		
	x2sys_path_init (s);		/* Prepare directory paths to data */
	
	*S = s;
	return (X2SYS_NOERROR);
}

void x2sys_bix_init (struct X2SYS_BIX *B, GMT_LONG alloc)
{
	B->i_bin_x = 1.0 / B->bin_x;
	B->i_bin_y = 1.0 / B->bin_y;
	B->nx_bin = irint ((B->x_max - B->x_min) * B->i_bin_x);
	B->ny_bin = irint ((B->y_max - B->y_min) * B->i_bin_y);
	B->nm_bin = B->nx_bin * B->ny_bin;
	if (alloc) B->binflag = (unsigned int *) GMT_memory (VNULL, (size_t)B->nm_bin, sizeof (unsigned int), X2SYS_program);
}

struct X2SYS_BIX_TRACK_INFO *x2sys_bix_make_entry (char *name, int id_no, int flag)
{
	struct X2SYS_BIX_TRACK_INFO *I;
	I = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory (VNULL, (size_t)1, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);
	I->trackname = strdup (name);
	I->track_id = id_no;
	I->flag = flag;
	I->next_info = NULL;
	return (I);
}

struct X2SYS_BIX_TRACK *x2sys_bix_make_track (int id, int flag)
{
	struct X2SYS_BIX_TRACK *T;
	T = (struct X2SYS_BIX_TRACK *) GMT_memory (VNULL, (size_t)1, sizeof (struct X2SYS_BIX_TRACK), X2SYS_program);
	T->track_id = id;
	T->track_flag = flag;
	T->next_track = NULL;
	return (T);
}

int x2sys_bix_read_tracks (struct X2SYS_INFO *S, struct X2SYS_BIX *B, int mode, int *ID)
{
	/* Reads the binned track listing which is ASCII */
	/* mode = 0 gives linked list, mode = 1 gives fixed array */
	int id, flag, last_id = -1;
	size_t n_alloc = GMT_CHUNK;
	char track_file[BUFSIZ], track_path[BUFSIZ], line[BUFSIZ], name[BUFSIZ], *unused = NULL;
	FILE *ftrack;
	struct X2SYS_BIX_TRACK_INFO *this_info = VNULL;

	sprintf (track_file, "%s%c%s_tracks.d", S->TAG, DIR_DELIM, S->TAG);
	x2sys_path (track_file, track_path);

	if ((ftrack = fopen (track_path, "r")) == NULL) return (GMT_GRDIO_FILE_NOT_FOUND);

#ifdef DEBUG
	GMT_memtrack_off (GMT_mem_keeper);
#endif
	if (mode == 1)
		B->head = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory (VNULL, n_alloc, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);
	else
		B->head = this_info = x2sys_bix_make_entry ("-", 0, 0);

	unused = fgets (line, BUFSIZ, ftrack);	/* Skip header record */
	while (fgets (line, BUFSIZ, ftrack)) {
		if (line[0] == '#' || line[0] == '>' || line[0] == '\0') continue;	/* Skip various comments and blank lines */
		GMT_chop (line);	/* Remove trailing CR or LF */
		sscanf (line, "%s %d %d", name, &id, &flag);
		if (mode == 1) {
			if (id >= (int)n_alloc) {
				while (id >= (int)n_alloc) n_alloc += GMT_CHUNK;
				B->head = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory ((void *)B->head, n_alloc, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);
			}
			B->head[id].track_id = id;
			B->head[id].flag = flag;
			B->head[id].trackname = strdup (name);
		}
		else {
			this_info->next_info = x2sys_bix_make_entry (name, id, flag);
			this_info = this_info->next_info;
		}
		if (id > last_id) last_id = id;
	}
	fclose (ftrack);
	last_id++;
	if (mode == 1) B->head = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory ((void *)B->head, (size_t)last_id, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);
#ifdef DEBUG
	GMT_memtrack_on (GMT_mem_keeper);
#endif

	*ID = last_id;
	
	return (X2SYS_NOERROR);
}

int x2sys_bix_read_index (struct X2SYS_INFO *S, struct X2SYS_BIX *B, GMT_LONG swap)
{
	/* Reads the binned index file which is native binary and thus swab is an issue */
	char index_file[BUFSIZ], index_path[BUFSIZ];
	FILE *fbin;
	int index = 0, flag, no_of_tracks, id, i;
	size_t not_used = 0;

	sprintf (index_file, "%s%c%s_index.b", S->TAG, DIR_DELIM, S->TAG);
	x2sys_path (index_file, index_path);

	if ((fbin = fopen (index_path, "rb")) == NULL) {
		fprintf (stderr,"%s: Could not open %s\n", X2SYS_program, index_path);
		return (GMT_GRDIO_OPEN_FAILED);
	}
#ifdef DEBUG
	GMT_memtrack_off (GMT_mem_keeper);
#endif
	B->base = (struct X2SYS_BIX_DATABASE *) GMT_memory (VNULL, (size_t)B->nm_bin, sizeof (struct X2SYS_BIX_DATABASE), X2SYS_program);

	while ((fread ((void *)(&index), sizeof (int), (size_t)1, fbin)) == 1) {
		not_used = fread ((void *)(&no_of_tracks), sizeof (int), (size_t)1, fbin);
		if (!swap && (index < 0 || no_of_tracks < 0)) swap = TRUE;	/* A negative index or no_of_tracks must mean that swapping is needed */
		if (swap) {
			index = GMT_swab4 (index);
			no_of_tracks = GMT_swab4 (no_of_tracks);
		}
		B->base[index].first_track = B->base[index].last_track = x2sys_bix_make_track (0, 0);
		for (i = 0; i < no_of_tracks; i++) {
			not_used = fread ((void *)(&id), sizeof (int), (size_t)1, fbin);
			not_used = fread ((void *)(&flag), sizeof (int), (size_t)1, fbin);
			if (swap) {
				id = GMT_swab4 (id);
				flag = GMT_swab4 (flag);
			}
			B->base[index].last_track->next_track = x2sys_bix_make_track (id, flag);
			B->base[index].last_track = B->base[index].last_track->next_track;
			B->base[index].n_tracks++;
		}
	}
#ifdef DEBUG
	GMT_memtrack_on (GMT_mem_keeper);
#endif
	fclose (fbin);
	return (X2SYS_NOERROR);
}

int x2sys_bix_get_ij (double x, double y, GMT_LONG *i, GMT_LONG *j, struct X2SYS_BIX *B, GMT_LONG *ID)
{
	GMT_LONG index = 0;

	*j = (y == B->y_max) ? B->ny_bin - 1 : (int)floor ((y - B->y_min) * B->i_bin_y);
	if ((*j) < 0 || (*j) >= B->ny_bin) {
		fprintf (stderr, "x2sys_binlist: j (%ld) outside range implied by -R -I! [0-%d>\n", *j, B->ny_bin);
		return (X2SYS_BIX_BAD_J);
	}
	*i = (x == B->x_max) ? B->nx_bin - 1 : (int)floor ((x - B->x_min)  * B->i_bin_x);
	if (B->periodic) {
		while (*i < 0) *i += B->nx_bin;
		while (*i >= B->nx_bin) *i -= B->nx_bin;
	}
	if ((*i) < 0 || (*i) >= B->nx_bin) {
		fprintf (stderr, "x2sys_binlist: i (%ld) outside range implied by -R -I! [0-%d>\n", *i, B->nx_bin);
		return (X2SYS_BIX_BAD_I);
	}
	index = (*j) * B->nx_bin + (*i);
	if (index < 0 || index >= B->nm_bin) {
		fprintf (stderr, "x2sys_binlist: Index (%ld) outside range implied by -R -I! [0-%ld>\n", index, B->nm_bin);
		return (X2SYS_BIX_BAD_IJ);
	}

	*ID  = index;
	return (X2SYS_NOERROR);
}

/* x2sys_path_init reads the X2SYS_HOME/TAG/TAG_paths.txt file and gets all
 * the data directories (if any) for this TAG.
 */
 
void x2sys_path_init (struct X2SYS_INFO *S)
{
	int i;
	char file[BUFSIZ], line[BUFSIZ];
	FILE *fp;

	x2sys_set_home ();

	sprintf (file, "%s%c%s%c%s_paths.txt", X2SYS_HOME, DIR_DELIM, S->TAG, DIR_DELIM, S->TAG);

	n_x2sys_paths = 0;

	if ((fp = fopen (file, "r")) == NULL) {
		if (gmtdefs.verbose) {
			fprintf (stderr, "%s: Warning: path file %s for %s files not found\n", X2SYS_program, file, S->TAG);
			fprintf (stderr, "%s: (Will only look in current directory for such files)\n", X2SYS_program);
			fprintf (stderr, "%s: (mgd77[+] also looks in MGD77_HOME and mgg looks in GMTSHARE/mgg)\n", X2SYS_program);
		}
		return;
	}

	while (fgets (line, BUFSIZ, fp) && n_x2sys_paths < MAX_DATA_PATHS) {
		if (line[0] == '#' || line[0] == '>' || line[0] == '\0') continue;	/* Skip various comments and blank lines */
		GMT_chop (line);	/* Remove trailing CR or LF */
		x2sys_datadir[n_x2sys_paths] = GMT_memory (VNULL, (size_t)1, (size_t)(strlen (line)+1), "x2sys_path_init");
#if _WIN32
		for (i = 0; line[i]; i++) if (line[i] == '/') line[i] = DIR_DELIM;
#else
		for (i = 0; line[i]; i++) if (line[i] == '\\') line[i] = DIR_DELIM;
#endif
		strcpy (x2sys_datadir[n_x2sys_paths], line);
		n_x2sys_paths++;
		if (n_x2sys_paths == MAX_DATA_PATHS) fprintf (stderr, "%s: Reached maximum directory (%d) count in %s!\n", X2SYS_program, MAX_DATA_PATHS, file);
	}
	fclose (fp);
}

/* x2sys_get_data_path takes a track name as argument and returns the full path
 * to where this data file can be found.  x2sys_path_init must be called first.
 */
 
int x2sys_get_data_path (char *track_path, char *track, char *suffix)
{
	int id;
	GMT_LONG add_suffix = FALSE;
	char geo_path[BUFSIZ], dotsuffix[GMT_TEXT_LEN];

	if (track[0] == '/' || track[1] == ':') {	/* Full path given, just return it */
		strcpy(track_path, track);
		return (0);
	}
	
	/* Check if we need to append suffix */
	
	sprintf (dotsuffix, ".%s", suffix);
	add_suffix = strncmp (&track[strlen(track)-strlen(dotsuffix)], dotsuffix, strlen(dotsuffix));	/* Need to add suffix? */

	/* First look in current directory */

	if (add_suffix)
		sprintf (geo_path, "%s.%s", track, suffix);
	else
		strcpy (geo_path, track);
	if (!access(geo_path, R_OK)) {
		strcpy(track_path, geo_path);
		return (0);
	}

	/* Then look elsewhere */

	for (id = 0; id < n_x2sys_paths; id++) {
		if (add_suffix)
			sprintf (geo_path, "%s%c%s.%s", x2sys_datadir[id], DIR_DELIM, track, suffix);
		else
			sprintf (geo_path, "%s%c%s", x2sys_datadir[id], DIR_DELIM, track);
		if (!access (geo_path, R_OK)) {
			strcpy (track_path, geo_path);
			return (0);
		}
	}
	return(1);	/* Schwinehund! */
}

const char * x2sys_strerror (int err)
{
/* Returns the error string for a given error code "err"
   Passes "err" on to nc_strerror if the error code is not one we defined */
	switch (err) {
		case X2SYS_FCLOSE_ERR:
			return "Error from fclose";
		case X2SYS_BAD_DEF:
			return "Cannot find format definition file in either current or X2SYS_HOME directories";
		case X2SYS_BAD_COL:
			return "Unrecognized string";
		case X2SYS_TAG_NOT_SET:
			return "TAG has not been set";
		case X2SYS_BAD_ARG:
			return "Unrecognized argument";
		case X2SYS_CONFLICTING_ARGS:
			return "Conflicting arguments";
		case X2SYS_BIX_BAD_J:
			return "Bad j index";
		case X2SYS_BIX_BAD_I:
			return "Bad i index";
		case X2SYS_BIX_BAD_IJ:
			return "Bad ij index";
		default:	/* default passes through to GMT error */
			return GMT_strerror(err);
	}
}

int x2sys_err_pass (int err, char *file)
{
	if (err == X2SYS_NOERROR) return (err);
	/* When error code is non-zero: print error message and pass error code on */
	if (file && file[0])
		fprintf (stderr, "%s: %s [%s]\n", X2SYS_program, x2sys_strerror(err), file);
	else
		fprintf (stderr, "%s: %s\n", X2SYS_program, x2sys_strerror(err));
	return (err);
}

void x2sys_err_fail (int err, char *file)
{
	if (err == X2SYS_NOERROR) return;
	/* When error code is non-zero: print error message and exit */
	if (file && file[0])
		fprintf (stderr, "%s: %s [%s]\n", X2SYS_program, x2sys_strerror(err), file);
	else
		fprintf (stderr, "%s: %s\n", X2SYS_program, x2sys_strerror(err));
	GMT_exit (EXIT_FAILURE);
}

/* Functions dealing with the reading of the COE ascii database */

GMT_LONG x2sys_read_coe_dbase (struct X2SYS_INFO *S, char *dbase, char *ignorefile, double *wesn, char *fflag, int coe_kind, char *one_trk, struct X2SYS_COE_PAIR **xpairs, GMT_LONG *nx, int *nt)
{
	 /* S:		The X2SYS_INFO structure
	 * dbase:	Name of the crossover data file [NULL for stdin]
	 * ignorefile:	Name of file with track names to ignore [or NULL if none]
	 * wesn:	Rectangular box to limit COE locations [NULL or 4-array with all zeros means no limit]
	 * fflag:	The name of the chosen field (e.g., faa)
	 * coe_kind: 	1 for internal, 2 for external, 3 [or 0] for both
	 * one_trk:	NULL to get coes from all pairs; give a track name for pairs only involving that track
	 * xpairs:	The return array of pair structures; number of pairs returned by function call
	 */

	FILE *fp;
	struct X2SYS_COE_PAIR *P;
	char line[BUFSIZ], txt[BUFSIZ], kind[BUFSIZ], fmt[BUFSIZ], trk[2][GMT_TEXT_LEN], t_txt[2][GMT_TEXT_LEN], start[2][GMT_TEXT_LEN];
	char x_txt[GMT_TEXT_LEN], y_txt[GMT_TEXT_LEN], d_txt[2][GMT_TEXT_LEN], h_txt[2][GMT_TEXT_LEN], v_txt[2][GMT_TEXT_LEN], z_txt[2][GMT_TEXT_LEN];
	char stop[2][GMT_TEXT_LEN], info[2][3*GMT_TEXT_LEN], **trk_list, **ignore = NULL, *t = NULL;
	GMT_LONG p, n_pairs;
	int i, k, n_alloc_x, n_alloc_p, n_alloc_t, year[2], id[2], n_ignore = 0, n_tracks = 0, n_items, our_item = -1;
	GMT_LONG more, skip, two_values = FALSE, check_box, keep = TRUE, no_time = FALSE;
	double x, m, lon, dist[2], d_val;

	fp = stdin;	/* Default to stdin if dbase is NULL */
	if (dbase && (fp = fopen (dbase, "r")) == NULL) {
		fprintf (stderr, "%s: ERROR: Unable to open crossover file %s\n", GMT_program, dbase);
		exit (EXIT_FAILURE);
	}

	n_alloc_p = n_alloc_t = GMT_CHUNK;
	P = (struct X2SYS_COE_PAIR *) GMT_memory (VNULL, (size_t)n_alloc_p, sizeof (struct X2SYS_COE_PAIR), GMT_program);

	while (fgets (line, BUFSIZ, fp) && line[0] == '#') {	/* Process header recs */
		GMT_chop (line);	/* Get rid of [CR]LF */
		/* Looking to process these two [three] key lines:
		 * # Tag: MGD77
		   # Command: x2sys_cross ... [in later versions only]
		   # lon	lat	t_1|i_1	t_2|i_2	dist_1	dist_2	head_1	head_2	vel_1	vel_2	twt_1	twt_2	depth_1	depth_2	...
		 */
		if (!strncmp (line, "# Tag:", 6)) {	/* Found the # TAG record */
			if (strcmp (S->TAG, &line[7])) {	/* -Ttag and this TAG do not match */
				fprintf (stderr, "%s: ERROR: Crossover file %s has a tag (%s) that differs from specified tag (%s) - aborting\n", GMT_program, dbase, &line[7], S->TAG);
				exit (EXIT_FAILURE);
			}
			continue;	/* Goto next record */
		}
		if (!strncmp (line, "# Command:", 10)) {	/* Found the # Command record */
			continue;	/* Goto next record */
		}
		if (!strncmp (line, "# ", 2)) {	/* Found the possible # lon lat ... record */
			sscanf (&line[2], "%*s %*s %s %*s %*s %*s %*s %*s %*s %*s %s", kind, txt);	/* Get first column name after lon/x etc */
			if (strchr (txt, '_')) {	/* A column name with underscore; we thus assume this is the correct record */
				char ptr[BUFSIZ];
				GMT_LONG pos = 0, item = 0;
				no_time = !strcmp (kind, "i_1");	/* No time in this database */
				if (txt[strlen(txt)-1] == '1') two_values = TRUE;	/* Option -2 was used */
				while (our_item == -1 && (GMT_strtok (&line[2], " \t", &pos, ptr))) {    /* Process all tokens */
					item++;
					i = strlen (ptr) - 1;
					while (i >= 0 && ptr[i] != '_') i--;	/* Start at end and find last underscore */
					if (i < 0) continue;		/* First records 'lon' & 'lat' have no '_' */
					strncpy (txt, ptr, i);
					txt[i] = '\0';
					if (!strcmp (txt, fflag)) our_item = item;	/* Found the desired column */
				}
			}
		}
	}
	
	/* Here, line holds the next record which will be the first > ... multisegment header for a crossing pair */

	our_item -= 10;		/* Account for the 10 common items */
	if (our_item < 0) {
		fprintf (stderr, "%s: ERROR: Crossover file %s does not have the specified column %s - aborting\n", GMT_program, dbase, fflag);
		exit (EXIT_FAILURE);
	}

	if (ignorefile && (k = x2sys_read_list (ignorefile, &ignore, &n_ignore)) != X2SYS_NOERROR) {
		fprintf (stderr, "%s: ERROR: Ignore file %s cannot be read - aborting\n", GMT_program, ignorefile);
		exit (EXIT_FAILURE);
	}

	check_box = (wesn && !(wesn[0] == wesn[1] && wesn[2] == wesn[3]));	/* Specified a rectangular box */

	/* OK, our file has the required column name, lets build the format statement */

	sprintf (fmt, "%%s %%s %%s %%s %%s %%s %%s %%s %%s %%s");	/* The standard 10 items up front */
	for (i = 1; i < our_item; i++) strcat (fmt, " %*s");	/* The items to skip */
	strcat (fmt, " %s %s");	/* The item we want */

	trk_list = (char **) GMT_memory (VNULL, (size_t)n_alloc_t, sizeof (char *), GMT_program);
	
	more = TRUE;
	n_pairs = *nx = 0;
	while (more) {	/* Read dbase until EOF */
		GMT_chop (line);	/* Get rid of [CR]LF */
		if (line[0] == '#') {	/* Skip a comment lines */
			while (fgets (line, BUFSIZ, fp) && line[0] == '#');	/* Skip header recs */
			continue;	/* Return to top of while loop */
		}
		n_items = sscanf (&line[2], "%s %d %s %d %s %s", trk[0], &year[0], trk[1], &year[1], info[0], info[1]);
		for (i = 0; i < (int)strlen (trk[0]); i++) if (trk[0][i] == '.') trk[0][i] = '\0';
		for (i = 0; i < (int)strlen (trk[1]); i++) if (trk[1][i] == '.') trk[1][i] = '\0';
		skip = FALSE;
		if (!(coe_kind & 1) && !strcmp (trk[0], trk[1])) skip = TRUE;	/* Do not want internal crossovers */
		if (!(coe_kind & 2) && strcmp (trk[0], trk[1])) skip = TRUE;	/* Do not want external crossovers */
		if (one_trk && (strcmp (one_trk, trk[0]) && strcmp (one_trk, trk[1]))) skip = TRUE;	/* Looking for a specific track and these do not match */
		if (!skip && n_ignore) {	/* See if one of the tracks are in the ignore list */
			for (i = 0; !skip && i < n_ignore; i++) if (!strcmp (trk[0], ignore[i]) || !strcmp (trk[1], ignore[i])) skip = TRUE;
		}
		if (skip) {	/* Skip this pair's data records */
			while ((t = fgets (line, BUFSIZ, fp)) && line[0] != '>');
			more = (t != NULL);
			continue;	/* Back to top of loop */
		}
		for (k = 0; k < 2; k++) {	/* Process each track */
			id[k] = x2sys_find_track (trk[k], trk_list, n_tracks);	/* Return track id # for this leg */
			if (id[k] == -1) {
				/* Leg not in the data base yet, add it */
				trk_list[n_tracks] = strdup (trk[k]);
				id[k] = n_tracks++;
				if (n_tracks == n_alloc_t) {
					n_alloc_t <<= 1;
					trk_list = (char **) GMT_memory ((void *)trk_list, (size_t)n_alloc_t, sizeof (char *), GMT_program);
				}
			}
		}
		/* Sanity check - make sure we dont already have this pair */
		for (p = 0, skip = FALSE; !skip && p < n_pairs; p++) {
			if ((P[p].id[0] == id[0] && P[p].id[1] == id[1]) || (P[p].id[0] == id[1] && P[p].id[1] == id[0])) {
				fprintf (stderr, "%s: Warning: Pair %s and %s appear more than once - skipped\n", GMT_program, trk[0], trk[1]);
				skip = TRUE;
			}
		}
		if (skip) {
			while ((t = fgets (line, BUFSIZ, fp)) && line[0] != '>');	/* Skip this pair's data records */
			more = (t != NULL);
			continue;	/* Back to top of loop */
		}
		
		/* OK, new pair */
		
		p = n_pairs;
		for (k = 0; k < 2; k++) {	/* Copy the values we found */
			sscanf (info[k], "%[^/]/%[^/]/%lg", start[k], stop[k], &dist[k]);
			strcpy (P[p].trk[k], trk[k]);
			P[p].id[k] = id[k];
			P[p].year[k] = year[k];
			if (n_items == 4)	/* Old format with no start/dist stuff */
				P[p].start[k] = P[p].stop[k] = dist[k] = GMT_d_NaN;
			else if (!strcmp (start[k], "NaN") || !strcmp (stop[k], "NaN"))	/* No time for this track */
				P[p].start[k] = P[p].stop[k] = GMT_d_NaN;
			else {
				if (GMT_verify_expectations (GMT_IS_ABSTIME, GMT_scanf (start[k], GMT_IS_ABSTIME, &P[p].start[k]), start[k])) {
					fprintf (stderr, "%s: ERROR: Header time specification tstart%d (%s) in wrong format\n", GMT_program, (k+1), start[k]);
					exit (EXIT_FAILURE);
				}
				if (GMT_verify_expectations (GMT_IS_ABSTIME, GMT_scanf (stop[k], GMT_IS_ABSTIME, &P[p].stop[k]), stop[k])) {
					fprintf (stderr, "%s: ERROR: Header time specification tstop%d (%s) in wrong format\n", GMT_program, (k+1), stop[k]);
					exit (EXIT_FAILURE);
				}
			}
			P[p].dist[k] = dist[k];
		}
		n_pairs++;
		if (n_pairs == n_alloc_p) {
			n_alloc_p <<= 1;
			P = (struct X2SYS_COE_PAIR *) GMT_memory ((void *)P, (size_t)n_alloc_p, sizeof (struct X2SYS_COE_PAIR), GMT_program);
		}
		n_alloc_x = GMT_SMALL_CHUNK;
		P[p].COE = (struct X2SYS_COE *) GMT_memory (VNULL, (size_t)n_alloc_x, sizeof (struct X2SYS_COE), GMT_program);
		k = 0;
		while ((t = fgets (line, BUFSIZ, fp)) && !(line[0] == '>' || line[0] == '#')) {	/* As long as we are reading data records */
			GMT_chop (line);	/* Get rid of [CR]LF */
			sscanf (line, fmt, x_txt, y_txt, t_txt[0], t_txt[1], d_txt[0], d_txt[1], h_txt[0], h_txt[1], v_txt[0], v_txt[1], z_txt[0], z_txt[1]);
			if (GMT_scanf (x_txt, GMT_IS_FLOAT, &d_val) == GMT_IS_NAN) d_val = GMT_d_NaN;
			P[p].COE[k].data[0][COE_X] = d_val;
			if (GMT_scanf (y_txt, GMT_IS_FLOAT, &d_val) == GMT_IS_NAN) d_val = GMT_d_NaN;
			P[p].COE[k].data[0][COE_Y] = d_val;

			for (i = 0; i < 2; i++) {
				if (GMT_scanf (d_txt[i], GMT_IS_FLOAT, &d_val) == GMT_IS_NAN) d_val = GMT_d_NaN;
				P[p].COE[k].data[i][COE_D] = d_val;

				if (GMT_scanf (h_txt[i], GMT_IS_FLOAT, &d_val) == GMT_IS_NAN) d_val = GMT_d_NaN;
				P[p].COE[k].data[i][COE_H] = d_val;

				if (GMT_scanf (v_txt[i], GMT_IS_FLOAT, &d_val) == GMT_IS_NAN) d_val = GMT_d_NaN;
				P[p].COE[k].data[i][COE_V] = d_val;

				if (GMT_scanf (z_txt[i], GMT_IS_FLOAT, &d_val) == GMT_IS_NAN) d_val = GMT_d_NaN;
				P[p].COE[k].data[i][COE_Z] = d_val;

				if (no_time || !strcmp (t_txt[i], "NaN"))
					P[p].COE[k].data[i][COE_T] = GMT_d_NaN;
				else if (GMT_verify_expectations (GMT_IS_ABSTIME, GMT_scanf (t_txt[i], GMT_IS_ABSTIME, &P[p].COE[k].data[i][COE_T]), t_txt[i])) {
					fprintf (stderr, "%s: ERROR: Time specification t%d (%s) in wrong format\n", GMT_program, (i+1), t_txt[i]);
					exit (EXIT_FAILURE);
				}
			}
			if (!two_values) {	/* Modify z to return the two values at the crossover point */
				x = 0.5 * P[p].COE[k].data[0][COE_Z]; m = P[p].COE[k].data[1][COE_Z];
				P[p].COE[k].data[0][COE_Z] = m + x;
				P[p].COE[k].data[1][COE_Z] = m - x;
			}
			if (check_box) {	/* Must pass coordinate check */
				keep = TRUE;
				if (P[p].COE[k].data[0][COE_Y] < wesn[2] || P[p].COE[k].data[0][COE_Y] > wesn[3])	/* Cartesian y or latitude */
					keep = FALSE;
				else if (S->geographic) {	/* Be cautions regarding longitude test */
					lon = P[p].COE[k].data[0][COE_X];
					while (lon > wesn[0]) lon -= 360.0;
					while (lon < wesn[0]) lon += 360.0;
					if (lon > wesn[1]) keep = FALSE;
				}
				else if (P[p].COE[k].data[0][COE_X] < wesn[0] || P[p].COE[k].data[0][COE_X] > wesn[1])	/* Cartesian x */
					keep = FALSE;
			}
			if (keep) {	/* Duplicate the coordinates at the crossover and increment k */
				P[p].COE[k].data[1][COE_X] = P[p].COE[k].data[0][COE_X];
				P[p].COE[k].data[1][COE_Y] = P[p].COE[k].data[0][COE_Y];
				k++;
			}
			if (k == n_alloc_x) {
				n_alloc_x <<= 1;
				P[p].COE = (struct X2SYS_COE *) GMT_memory ((void *)P[p].COE, (size_t)n_alloc_x, sizeof (struct X2SYS_COE), GMT_program);
			}
		}
		more = (t != NULL);
		if (k == 0) {	/* No COE, probably due to wesn check */
			GMT_free ((void *)P[p].COE);
			n_pairs--;	/* To reset this value since the top of the loop will do n_pairs++ */
		}
		else {
			P[p].COE = (struct X2SYS_COE *) GMT_memory ((void *)P[p].COE, (size_t)k, sizeof (struct X2SYS_COE), GMT_program);
			P[p].nx = k;
			*nx += k;
		}
	}
	fclose (fp);
	if (n_pairs == 0) {	/* No pairs found, probably due to wesn check */
		GMT_free ((void *)P);
		*xpairs = NULL;
	}
	else {
		P = (struct X2SYS_COE_PAIR *) GMT_memory ((void *)P, (size_t)n_pairs, sizeof (struct X2SYS_COE_PAIR), GMT_program);
		*xpairs = P;
	}
	x2sys_free_list (trk_list, n_tracks);
	x2sys_free_list (ignore, n_ignore);
	
	*nt = n_tracks;
	return (n_pairs);
}

void x2sys_free_coe_dbase (struct X2SYS_COE_PAIR *P, GMT_LONG np)
{	/* Free up the memory associated with P as created by x2sys_read_coe_dbase */
	GMT_LONG p;
	for (p = 0; p < np; p++) GMT_free ((void *)P[p].COE);
	GMT_free ((void *)P);
}

int x2sys_find_track (char *name, char **list, int n)
{	/* Return track id # for this leg or -1 if not found */
	int i = 0;
	if (!list) return (-1);	/* Null pointer passed */
	for (i = 0; i < n; i++) if (!strcmp (name, list[i])) return (i);
	return (-1);
}

int x2sys_get_tracknames (int argc, char **argv, char ***filelist, GMT_LONG *cmdline)
{	/* Return list of track names given on command line or via =list mechanism.
	 * The names do not have the track extension. */
	int i, A, list = 0, add_chunk, n_alloc;
	char **file, *p;
	
	/* Backwards checking for :list in addition to the new 9as in mgd77) =list mechanism */
	for (A = 1; !list && A < argc; A++) if (argv[A][0] == ':' || argv[A][0] == '=') list = A;

	if (list) {	/* Got a file with a list of filenames */
		*cmdline = FALSE;
		if (x2sys_read_list (&argv[list][1], filelist, &A) != X2SYS_NOERROR) {
			fprintf (stderr, "%s: Error: Could not open list with filenames %s!\n", GMT_program, &argv[list][1]);
			exit (EXIT_FAILURE);
		}
		file = *filelist;
	}
	else {		/* Get files from command line */
		add_chunk = n_alloc = GMT_CHUNK;
		file = (char **)GMT_memory (VNULL, (size_t)n_alloc, sizeof (char *), GMT_program);

		*cmdline = TRUE;
		for (i = 1, A = 0; i < argc; i++) {
			if (argv[i][0] == '-') continue;	/* Skip options */

			file[A++] = strdup (argv[i]);
			if (A == n_alloc) {
				add_chunk <<= 1;
				n_alloc += add_chunk;
				file = (char **)GMT_memory ((void *)file, (size_t)n_alloc, sizeof (char *), GMT_program);
			}
		}
		file = (char **)GMT_memory ((void *)file, (size_t)A, sizeof (char *), GMT_program);
		*filelist = file;
	}
	/* Strip off any extensions */
	
	for (i = 0; i < A; i++) {
		if ((p = strchr (file[i], '.'))) file[i][(int)(p-file[i])] = '\0';
	}

	return (A);
}

int separate_aux_columns (int n_items, char **item_name, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist)
{	/* Used in x2sys_get_corrtable */
	int i, j, k, this_aux, n_aux;
	/* Based on what item_name contains, we copy over info on the 3 aux fields (dist, azim, vel) from auxlist to aux */
	for (i = k = n_aux = 0; i < n_items; i++) {
		for (j = 0, this_aux = MGD77_NOT_SET; j < N_GENERIC_AUX && this_aux == MGD77_NOT_SET; j++) if (!strcmp (auxlist[j].name, item_name[i])) this_aux = j;
		if (this_aux != MGD77_NOT_SET) {	/* Found a request for an auxillary column  */
			aux[n_aux].type = auxlist[this_aux].type;
			aux[n_aux].text = auxlist[this_aux].text;
			aux[n_aux].pos = k;
			auxlist[this_aux].requested = TRUE;
			n_aux++;
		}
	}
	return (n_aux);
}

void x2sys_get_corrtable (struct X2SYS_INFO *S, char *ctable, int ntracks, char **trk_name, char *column, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist, struct MGD77_CORRTABLE ***CORR)
{	/* Load an ephemeral correction table */
	/* Pass aux as NULL if the auxillary columns do not matter (only used by x2sys_datalist) */
	int i, k = 0, n_items, n_aux = 0, n_cols, missing;
	char path[BUFSIZ], **item_names = NULL, **col_name = NULL, **aux_name = NULL;
	
	if (!ctable) {	/* Try default correction table */
		sprintf (path, "%s%c%s%c%s_corrections.txt", X2SYS_HOME, DIR_DELIM, S->TAG, DIR_DELIM, S->TAG);
		if (access (path, R_OK)) {
			fprintf (stderr, "%s: No default X2SYS Correction table (%s) for %s found!\n", GMT_program, path, S->TAG);
			exit (EXIT_FAILURE);
		}
		ctable = path;
	}
	if (column) {	/* Must build list of the 7 standard COE database column names */
		n_cols = 7;
		col_name = (char **) GMT_memory (VNULL, n_cols, sizeof (char *), GMT_program);
		col_name[COE_X] = (S->geographic) ? strdup ("lon") : strdup ("x");
		col_name[COE_Y] = (S->geographic) ? strdup ("lat") : strdup ("y");
		col_name[COE_T] = strdup ("time");
		col_name[COE_Z] = strdup (column);
		col_name[COE_D] = strdup ("dist");
		col_name[COE_H] = strdup ("azim");
		col_name[COE_V] = strdup ("vel");
	}
	else {	/* Use what is available in the data files */
		n_cols = S->n_out_columns;
		col_name = (char **) GMT_memory (VNULL, n_cols, sizeof (char *), GMT_program);
		for (i = 0; i < n_cols; i++) col_name[i] = strdup (S->info[S->out_order[i]].name);
	}
	n_items = MGD77_Scan_Corrtable (ctable, trk_name, ntracks, n_cols, col_name, &item_names, 0);
	if (aux && (n_aux = separate_aux_columns (n_items, item_names, aux, auxlist))) {	/* Determine which auxillary columns are requested (if any) */
		aux_name = (char **) GMT_memory (VNULL, n_aux, sizeof (char *), GMT_program);
		for (i = 0; i < n_aux; i++) aux_name[i] = strdup (auxlist[aux[i].type].name);
	}
	for (i = missing = 0; i < n_items; i++) {
		if (MGD77_Match_List (item_names[i], n_cols, col_name) == MGD77_NOT_SET) {	/* Requested column not among data cols */
			if (n_aux && (k = MGD77_Match_List (item_names[i], n_aux, aux_name)) == MGD77_NOT_SET) {
				fprintf (stderr, "%s: X2SYS Correction table (%s) requires a column (%s) not present in COE database or auxillary columns\n", GMT_program, ctable, item_names[i]);
				missing++;
			}
			else
				auxlist[aux[k].type].requested = TRUE;
		}
	}
	x2sys_free_list (item_names, n_items);
	x2sys_free_list (aux_name, n_aux);
	if (!missing) MGD77_Parse_Corrtable (ctable, trk_name, ntracks, n_cols, col_name, 0, CORR);
	x2sys_free_list (col_name, n_cols);
	if (missing) exit (EXIT_FAILURE);
}
