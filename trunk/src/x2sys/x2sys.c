/*-----------------------------------------------------------------
 *	$Id: x2sys.c,v 1.34 2005-09-05 00:46:16 pwessel Exp $
 *
 *      Copyright (c) 1999-2001 by P. Wessel
 *      See COPYING file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 of the License.
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
 * x2sys_initialize	: Reads the definition info file for current data files
 * x2sys_read_file	: Reads and returns the entire data matrix
 * x2sys_read_gmtfile	: Specifically reads an old .gmt file
 * x2sys_read_mgd77file : Specifically reads an MGD77 file
 * x2sys_read_list	: Read an ascii list of track names
 * x2sys_distances	: Calculate cumulative distances along track
 * x2sys_dummytimes	: Make dummy times for tracks missing times
 * x2sys_n_data_cols	: Gives number of data columns in this data set
 * x2sys_fopen		: Opening files with error message and exit
 * x2sys_fclose		: Closes files and gives error messages
 * x2sys_skip_header	: Skips the header record(s) in the open file
 * x2sys_read_record	: Reads and returns one record from the open file
 * x2sys_output_record	: Writes one data record to stdout
 * x2sys_pick_fields	: Decodes the -F<fields> flag of desired columns
 * x2sys_free_info	: Frees the information structure
 * x2sys_free_data	: Frees the data matrix
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
 * Date:	18-MAY-2004
 * Version:	1.1, based on the spirit of the old xsystem code
 *
 */

#include "x2sys.h"

/* Global variables used by X2SYS functions */

char *X2SYS_HOME;
char *X2SYS_program;

char *x2sys_xover_format = "%9.5lf %9.5lf %10.1lf %10.1lf %9.2lf %9.2lf %9.2lf %8.1lf %8.1lf %8.1lf %5.1lf %5.1lf\n";
char *x2sys_xover_header = "%s %d %s %d\n";
char *x2sys_header = "> %s %d %s %d\n";
struct MGD77_CONTROL M;

void x2sys_set_home (void);
int x2sys_record_length (struct X2SYS_INFO *s);
int MGD77_items[MGD77_DATA_COLS] = { 0, MGD77_LONGITUDE, MGD77_LATITUDE, 10, 11, 14, 15, 16, 18, 19, 20, 21, 22};

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

	if (mode[0] == 'w') {	/* Writing: Do this in X2SYS_HOME */
		x2sys_path (fname, file);
		if ((fp = fopen (file, mode)) == NULL) {	/* Not in X2SYS_HOME directory either */
			fprintf (stderr, "x2sys: Error from fopen on %s using mode %s\n", file, mode);
			exit (EXIT_FAILURE);
		}
	}
	else {			/* Reading: Try both current directory and X2SYS_HOME */
		if ((fp = fopen (fname, mode)) == NULL) {	/* Not in current directory */
			x2sys_path (fname, file);
			if ((fp = fopen (file, mode)) == NULL) {	/* Not in X2SYS_HOME directory either */
				fprintf (stderr, "x2sys: Error from fopen on %s using mode %s\n", fname, mode);
				exit (EXIT_FAILURE);
			}
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

void x2sys_fclose (char *fname, FILE *fp)
{

	if (fclose (fp)) {
		fprintf (stderr, "x2sys: Error from fclose on %s\n", fname);
		exit (EXIT_FAILURE);
	}
}

void x2sys_skip_header (FILE *fp, struct X2SYS_INFO *s)
{
	int i;
	char line[BUFSIZ];

	if (s->ascii_in) {	/* ASCII, skip records */
		for (i = 0; i < s->skip; i++) fgets (line, BUFSIZ, fp);
	}
	else {			/* Binary, skip bytes */
		fseek (fp, (long)s->skip, SEEK_CUR);
	}
}

/*
 * x2sys_data_read:  Read subroutine for x2_sys data input.
 * This function will read one logical record of ascii or
 * binary data from the open file, and return with a double
 * array called data[] with each data value in it.
 */

int x2sys_read_record (FILE *fp, double *data, struct X2SYS_INFO *s, struct GMT_IO *G)
{
	int j, k, i, n_read = 0, pos;
	BOOLEAN error = FALSE;
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
					fgets (line, BUFSIZ, fp);	/* Get new record */
					while (line[0] == '#' || line[0] == s->ms_flag) {
						fgets (line, BUFSIZ, fp);
						if (s->multi_segment) s->ms_next = TRUE;
					}
					GMT_chop (line);	/* Remove trailing CR or LF */
				}
				strncpy (buffer, &line[s->info[j].start_col], s->info[j].n_cols);
				buffer[s->info[j].n_cols] = 0;
				GMT_scanf (buffer, G->in_col_type[j], &data[j]);
				break;

			case 'a':	/* ASCII Record, get all columns directly */
				k = 0;
				s->ms_next = FALSE;
				fgets (line, BUFSIZ, fp);
				while (line[0] == '#' || line[0] == s->ms_flag) {
					fgets (line, BUFSIZ, fp);
					if (s->multi_segment) s->ms_next = TRUE;
				}
				GMT_chop (line);	/* Remove trailing CR or LF */
				pos = 0;
				while ((GMT_strtok (line, " ,\t\n", &pos, p))) {
					GMT_scanf (p, G->in_col_type[k], &data[k]);
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
				data[j] = (double)i;
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
		if (i == s->x_col && s->geographic) GMT_lon_range_adjust (s->geodetic, &data[i]);
	}

	return ((error || n_read != s->n_fields) ? -1 : 0);
}
 
int x2sys_read_file (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G)
{
	/* Reads the entire contents of the file given and returns the
	 * number of data records.  The data matrix is return in the
	 * pointer data.
	 */

	int i, j;
	size_t n_alloc;
	FILE *fp;
	double **z, *rec;
	char path[BUFSIZ];

 	if (n_x2sys_paths) {
  		if (x2sys_get_data_path (path, fname)) {
   			fprintf (stderr, "x2sys_read_file : Cannot find track %s\n", fname);
     			return (-1);
  		}
  		if ((fp = fopen (path, "rb")) == NULL) {
   			fprintf (stderr, "x2sys_read_mgd77file : Cannot open file %s\n", path);
     			return (-1);
  		}
	}
	else if ((fp = fopen (fname, "rb")) == NULL) {
		fprintf (stderr, "x2sys_read_file: Could not open %s\n", fname);
		return (-1);
	}

	n_alloc = GMT_CHUNK;

	rec = (double *) GMT_memory (VNULL, s->n_fields, sizeof (double), "x2sys_read_file");
	z = (double **) GMT_memory (VNULL, s->n_fields, sizeof (double *), "x2sys_read_file");
	for (i = 0; i < s->n_fields; i++) z[i] = (double *) GMT_memory (VNULL, n_alloc, sizeof (double), "x2sys_read_file");
	p->ms_rec = (int *) GMT_memory (VNULL, n_alloc, sizeof (int), "x2sys_read_file");
	x2sys_skip_header (fp, s);
	if (s->multi_segment) p->n_segments = -1;	/* So that first increment sets it to 0 */

	j = 0;
	while (!x2sys_read_record (fp, rec, s, G)) {	/* Gets the next data record */
		for (i = 0; i < s->n_fields; i++) z[i][j] = rec[i];
		if (s->multi_segment && s->ms_next) p->n_segments++;
		p->ms_rec[j] = p->n_segments;
		j++;
		if (j == (int)n_alloc) {	/* Get more */
			n_alloc += GMT_CHUNK;
			for (i = 0; i < s->n_fields; i++) z[i] = (double *) GMT_memory ((void *)z[i], n_alloc, sizeof (double), "x2sys_read_file");
			p->ms_rec = (int *) GMT_memory ((void *)p->ms_rec, n_alloc, sizeof (int), "x2sys_read_file");
		}
	}

	fclose (fp);
	for (i = 0; i < s->n_fields; i++) z[i] = (double *) GMT_memory ((void *)z[i], (size_t)j, sizeof (double), "x2sys_read_file");
	p->ms_rec = (int *) GMT_memory ((void *)p->ms_rec, (size_t)j, sizeof (int), "x2sys_read_file");

	*data = z;

	p->n_rows = j;
	p->year = 0;
	strncpy (p->name, fname, 32);

	return (j);
}

struct X2SYS_INFO *x2sys_initialize (char *fname, struct GMT_IO *G)
{
	/* Reads the format definition file and sets all information variables */

	int i = 0, c;
	size_t n_alloc = 10;
	FILE *fp;
	struct X2SYS_INFO *X;
	char line[BUFSIZ], cardcol[80], yes_no;

	x2sys_set_home ();

	X = (struct X2SYS_INFO *) GMT_memory (VNULL, n_alloc, sizeof (struct X2SYS_INFO), "x2sys_initialize");
	X->info = (struct X2SYS_DATA_INFO *) GMT_memory (VNULL, n_alloc, sizeof (struct X2SYS_DATA_INFO), "x2sys_initialize");
	X->ascii_in = TRUE;
	X->x_col = X->y_col = X->t_col = -1;
	X->ms_flag = '>';	/* Default multisegment header flag */
	sprintf (line, "%s%c%s.def", X2SYS_HOME, DIR_DELIM, fname);

	fp = x2sys_fopen (line, "r");

	if (!strcmp (fname, "gmt")) {
		X->read_file = (PFI) x2sys_read_gmtfile;
		X->geographic = TRUE;
		X->geodetic = 0;
	}
	else if (!strcmp (fname, "mgd77")) {
		X->read_file = (PFI) x2sys_read_mgd77file;
		X->geographic = TRUE;
		X->geodetic = 0;
		MGD77_Init (&M, TRUE);			/* Initialize MGD77 Machinery */
	}
	else
		X->read_file = (PFI) x2sys_read_file;

	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '\0') continue;
		if (line[0] == '#') {
			if (!strncmp (line, "#SKIP ", 6)) X->skip = atoi (&line[6]);
			if (!strncmp (line, "#BINARY ", 7)) X->ascii_in = FALSE;
			if (!strncmp (line, "#GEO ", 3)) X->geographic = TRUE;
			if (!strncmp (line, "#MULTISEG ", 9)) {
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
		if (tolower (c) != 'a') X->ascii_in = FALSE;
		c = (int)yes_no;
		if (tolower (c) != 'Y') X->info[i].has_nan_proxy = TRUE;
		if (!(X->info[i].scale == 1.0 && X->info[i].offset == 0.0)) X->info[i].do_scale = TRUE;
		if (!strcmp (X->info[i].name, "x") || !strcmp (X->info[i].name, "lon"))  X->x_col = i;
		if (!strcmp (X->info[i].name, "y") || !strcmp (X->info[i].name, "lat"))  X->y_col = i;
		if (!strcmp (X->info[i].name, "t") || !strcmp (X->info[i].name, "time")) X->t_col = i;
		i++;
		if (i == (int)n_alloc) {
			n_alloc += 10;
			X->info = (struct X2SYS_DATA_INFO *) GMT_memory ((void *)X->info, n_alloc, sizeof (struct X2SYS_DATA_INFO), "x2sys_initialize");
		}

	}
	fclose (fp);

	if (i < (int)n_alloc) X->info = (struct X2SYS_DATA_INFO *) GMT_memory ((void *)X->info, i, sizeof (struct X2SYS_DATA_INFO), "x2sys_initialize");
	X->n_fields = X->n_out_columns = i;

	X->out_order  = (int *) GMT_memory (VNULL, sizeof (int), (size_t)X->n_fields, "x2sys_initialize");
	X->use_column = (int *) GMT_memory (VNULL, sizeof (int), (size_t)X->n_fields, "x2sys_initialize");
	for (i = 0; i < X->n_fields; i++) {	/* Default is same order and use all columns */
		X->out_order[i] = i;
		X->use_column[i] = 1;
		G->in_col_type[i] = G->out_col_type[i] = (X->x_col == i) ? GMT_IS_LON : ((X->y_col == i) ? GMT_IS_LAT : GMT_IS_UNKNOWN);
	}
	X->n_data_cols = x2sys_n_data_cols (X);
	X->rec_size = (8 + X->n_data_cols) * sizeof (double);

	return (X);
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

	for (i = 0; i < s->n_fields; i++) {
		if (i == s->x_col) continue;
		if (i == s->y_col) continue;
		if (i == s->t_col) continue;
		if (!s->use_column[i]) continue;
		n++;
	}

	return (n);
}

int x2sys_output_record (FILE *fp, double data[], struct X2SYS_INFO *s)
{
	int i, k;

	if (s->ascii_out) {
		for (i = 0; i < s->n_out_columns-1; i++) {
			k = s->out_order[i];
			(GMT_is_dnan (data[k])) ? fprintf (fp, "NaN") : fprintf (fp, s->info[k].format, data[k]);
			fputc ('\t', fp);
		}
		k = s->out_order[i];
		(GMT_is_dnan (data[k])) ? fprintf (fp, "NaN") : fprintf (fp, s->info[k].format, data[k]);
		fputc ('\n', fp);
	}
	else {	/* Binary */
		for (i = 0; i < s->n_out_columns; i++) {
			k = s->out_order[i];
			fwrite ((void *)&data[k], sizeof (double), (size_t)1, fp);
		}
	}
	return (s->n_out_columns);
}

void x2sys_pick_fields (char *string, struct X2SYS_INFO *s)
{
	/* Scan the -Fstring and select which columns to use and which order
	 * they should appear on output.  Default is all columns and the same
	 * order as on input
	 */

	char line[BUFSIZ], p[BUFSIZ];
	int i = 0, j, pos = 0;

	strncpy (line, string, BUFSIZ);
	memset ((void *)s->use_column, 0, (size_t)(s->n_fields * sizeof (int)));

	while ((GMT_strtok (line, ",", &pos, p))) {
		j = 0;
		while (j < s->n_fields && strcmp (p, s->info[j].name)) j++;
		if (j < s->n_fields) {
			s->out_order[i] = j;
			s->use_column[j] = 1;
		}
		else {
			fprintf (stderr, "X2SYS: ERROR: Unknown column name %s\n", p);
			exit (EXIT_FAILURE);
		}
		i++;
	}

	s->n_out_columns = i;
}

void x2sys_set_home (void)
{
	char *this;

	if (X2SYS_HOME) return;	/* Already set elsewhere */

	if ((this = getenv ("X2SYS_HOME")) != CNULL) {	/* Set user's default path */
		X2SYS_HOME = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 1), 1, "x2sys_set_home");
		strcpy (X2SYS_HOME, this);
	}
	else if ((this = getenv ("GMTHOME")) != CNULL) {	/* Assume GMT path */
		X2SYS_HOME = (char *) GMT_memory (VNULL, (size_t)(strlen (this) + 13), 1, "x2sys_set_home");
		sprintf (X2SYS_HOME, "%s%cshare%cx2sys", this, DIR_DELIM, DIR_DELIM);
	}
	else {	/* Set default path */
#ifdef _WIN32
		X2SYS_HOME = (char *) GMT_memory (VNULL, (size_t)23, 1, "x2sys_set_home");
		strcpy (X2SYS_HOME, "C:\\usr\\local\\gmt\\x2sys");
#else
		X2SYS_HOME = (char *) GMT_memory (VNULL, (size_t)21, 1, "x2sys_set_home");
		strcpy (X2SYS_HOME, "/usr/local/gmt/x2sys");
#endif
	}
}

void x2sys_free_info (struct X2SYS_INFO *s)
{
	GMT_free ((void *)s->info);
	GMT_free ((void *)s);
}

void x2sys_free_data (double **data, int n)
{
	int i;

	for (i = 0; i < n; i++) GMT_free ((void *)data[i]);
	GMT_free ((void *)data);
}

double *x2sys_distances (double x[], double y[], int n, int dist_flag)
{
	int this, prev;
	double *d, km_pr_deg;

	km_pr_deg = 0.001 * 2.0 * M_PI * 6371008.7714 / 360.0;

	d = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "x2sys_distances");

	switch (dist_flag) {

		case 0:	/* Cartesian distances */

			for (this = 1, prev = 0; this < n; this++, prev++)
				d[this] = d[prev] + hypot (x[this] - x[prev], y[this] - y[prev]);
			break;

		case 1:	/* Flat earth distances in km*/

			for (this = 1, prev = 0; this < n; this++, prev++)
				d[this] = d[prev] + GMT_flatearth_dist_km (x[this], y[this], x[prev], y[prev]);
			break;

		case 2:	/* Great circle distances in km */

			for (this = 1, prev = 0; this < n; this++, prev++)
				d[this] = d[prev] + GMT_great_circle_dist_km (x[this], y[this], x[prev], y[prev]);
			break;
		case 3:	/* Geodesic distances in km */

			for (this = 1, prev = 0; this < n; this++, prev++)
				d[this] = d[prev] + GMT_geodesic_dist_km (x[this], y[this], x[prev], y[prev]);
			break;
		default:
			fprintf (stderr, "x2sys: Error: Wrong flag passed to x2sys_distances (%d)\n", dist_flag);
			exit (EXIT_FAILURE);
			break;
	}

	return (d);
}

double *x2sys_dummytimes (int n)
{
	int i;
	double *t;

	/* Make monotonically increasing dummy time sequence */

	t = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "x2sys_dummytimes");

	for (i = 0; i < n; i++) t[i] = (double)i;

	return (t);
}

int x2sys_read_gmtfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G)
{
	/* Reads the entire contents of the file given and returns the
	 * number of data records.  The data matrix is return in the
	 * pointer data.  The input file format is the venerable GMT
	 * MGG format from old Lamont by Wessel and Smith.
	 */

	int i, j;
	char gmtfile[BUFSIZ], name[80];
	FILE *fp;
	double **z;
	double NaN;
	struct GMTMGG_REC record;

	GMT_make_dnan(NaN);

	if (!(s->flags & 1)) {	/* Must init gmt file paths */
		gmtmggpath_init ();
		s->flags |= 1;
	}

	strncpy (name, fname, 80);
	if (strstr (fname, ".gmt"))	/* Name includes .gmt suffix */
		name[strlen(fname)-4] = 0;

  	if (gmtmggpath_func (gmtfile, name)) {
   		fprintf (stderr, "x2sys_read_gmtfile : Cannot find leg %s\n", name);
     		return (-1);
  	}

	if ((fp = fopen (gmtfile, "rb")) == NULL) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not open %s\n", gmtfile);
		return (-1);
	}

	if (fread ((void *)&(p->year), sizeof (int), (size_t)1, fp) != 1) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not read leg year from %s\n", gmtfile);
		return (-1);
	}
	if (fread ((void *)&(p->n_rows), sizeof (int), (size_t)1, fp) != 1) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not read n_records from %s\n", gmtfile);
		return (-1);
	}
	memset ((void *)p->name, 0, (size_t)32);

	if (fread ((void *)p->name, (size_t)10, sizeof (char), fp) != 1) {
		fprintf (stderr, "x2sys_read_gmtfile: Could not read agency from %s\n", gmtfile);
		return (-1);
	}

	z = (double **) GMT_memory (VNULL, (size_t)6, sizeof (double *), "x2sys_read_gmtfile");
	for (i = 0; i < 6; i++) z[i] = (double *) GMT_memory (VNULL, (size_t)p->n_rows, sizeof (double), "x2sys_read_gmtfile");

	for (j = 0; j < p->n_rows; j++) {

		if (fread ((void *)&record, (size_t)18, (size_t)1, fp) != 1) {
			fprintf (stderr, "x2sys_read_gmtfile: Could not read record %d from %s\n", j, gmtfile);
			exit (EXIT_FAILURE);
		}

		z[0][j] = record.time;
		z[1][j] = record.lat * MDEG2DEG;
		z[2][j] = record.lon * MDEG2DEG;
		z[3][j] = (record.gmt[0] == GMTMGG_NODATA) ? NaN : 0.1 * record.gmt[0];
		z[4][j] = (record.gmt[1] == GMTMGG_NODATA) ? NaN : record.gmt[1];
		z[5][j] = (record.gmt[2] == GMTMGG_NODATA) ? NaN : record.gmt[2];

	}

	fclose (fp);

	p->ms_rec = NULL;
	p->n_segments = 0;

	*data = z;

	return (p->n_rows);
}

int x2sys_read_mgd77file (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G)
{
	int i, j, n_alloc = GMT_CHUNK;
	char path[BUFSIZ];
	double **z;
	FILE *fp;
	struct MGD77_DATA_RECORD D;
	struct MGD77_HEADER_RECORD H;
	struct MGD77_CONTROL M;
	double NaN;

	GMT_make_dnan(NaN);
	MGD77_Init (&M, TRUE);			/* Initialize MGD77 Machinery */

  	if (n_x2sys_paths) {
  		if (x2sys_get_data_path (path, fname)) {
   			fprintf (stderr, "x2sys_read_mgd77file : Cannot find leg %s\n", fname);
     			return (-1);
  		}
		if (MGD77_Open_File (path, &M, 0)) {
   			fprintf (stderr, "x2sys_read_mgd77file : Cannot find file %s\n", path);
     			return (-1);
  		}
	}
	else if (MGD77_Open_File (fname, &M, 0)) {
   		fprintf (stderr, "x2sys_read_mgd77file : Cannot find file %s\n", fname);
     		return (-1);
  	}

	if (MGD77_Read_Header_Record (&M, &H)) {
		fprintf (stderr, "%s: Error reading header sequence for cruise %s\n", X2SYS_program, fname);
		exit (EXIT_FAILURE);
	}

	z = (double **) GMT_memory (VNULL, (size_t)MGD77_DATA_COLS, sizeof (double *), "x2sys_read_mgd77file");
	for (i = 0; i < MGD77_DATA_COLS; i++) z[i] = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "x2sys_read_mgd77file");

	j = 0;
	while (!MGD77_Read_Data_Record (&M, &D)) {		/* While able to read a data record */
		z[0][j] = D.time;
		GMT_lon_range_adjust (s->geodetic, &D.number[MGD77_LONGITUDE]);
		for (i = 1; i < MGD77_DATA_COLS; i++) z[i][j] = D.number[MGD77_items[i]];

		j++;
		if (j == n_alloc) {
			n_alloc += GMT_CHUNK;
			for (i = 0; i < MGD77_DATA_COLS; i++) z[i] = (double *) GMT_memory ((void *)z[i], (size_t)n_alloc, sizeof (double), "x2sys_read_mgd77file");
		}
	}
	MGD77_Close_File (&M);

	strncpy (p->name, fname, 32);
	p->n_rows = j;
	for (i = 0; i < MGD77_DATA_COLS; i++) z[i] = (double *) GMT_memory ((void *)z[i], (size_t)p->n_rows, sizeof (double), "x2sys_read_mgd77file");

	p->ms_rec = NULL;
	p->n_segments = 0;

	*data = z;

	return (p->n_rows);
}

int x2sys_xover_output (FILE *fp, int n, double out[])
{
	/* Write old xover formatted output.  This assumes data files are .gmt files */

	/* y x t1 t2 X1 X2 X3 M1 M2 M3 h1 h2 */

	fprintf (fp, x2sys_xover_format, out[1], out[0], out[2], out[3], out[9], out[11], out[13], out[8], out[10], out[12], out[6], out[7]);
	return (12);
}

int x2sys_read_list (char *file, char ***list)
{
	int n_alloc = GMT_CHUNK, n = 0;
	char **p, line[BUFSIZ], name[GMT_TEXT_LEN];
	FILE *fp;

	fp = x2sys_fopen (file, "r");

	p = (char **) GMT_memory (VNULL, n_alloc, sizeof (char *), "x2sys_read_list");

	while (fgets (line, BUFSIZ, fp)) {
		GMT_chop (line);	/* Remove trailing CR or LF */
		sscanf (line, "%s", name);
		p[n] = (char *) GMT_memory (VNULL, (size_t)(strlen(name)+1), sizeof (char), "x2sys_read_list");
		strcpy (p[n], name);
		n++;
		if (n == n_alloc) {
			n_alloc += GMT_CHUNK;
			p = (char **) GMT_memory ((void *)p, n_alloc, sizeof (char *), "x2sys_read_list");
		}
	}
	fclose (fp);

	p = (char **) GMT_memory ((void *)p, n, sizeof (char *), "x2sys_read_list");

	*list = p;

	return (n);
}

void x2sys_set_system (char *TAG, struct X2SYS_INFO **s, struct X2SYS_BIX *B, struct GMT_IO *G)
{
	char tag_file[BUFSIZ], line[BUFSIZ], p[BUFSIZ], sfile[BUFSIZ];
	int geodetic = 0, pos = 0, n;
	double dist;
	BOOLEAN geographic = FALSE;
	FILE *fp;

	x2sys_set_home ();

	memset ((void *)B, 0, sizeof (struct X2SYS_BIX));
	B->bin_x = B->bin_y = 1.0;
	B->x_min = 0.0;	B->x_max = 360.0;	B->y_min = -90.0;	B->y_max = +90.0;
	B->time_gap = B->dist_gap = dist = DBL_MAX;	/* Default is no data gap */
	B->periodic = sfile[0] = 0;

	sprintf (tag_file, "%s.tag", TAG);
	if ((fp = x2sys_fopen (tag_file, "r")) == NULL) {
		fprintf (stderr,"%s: Could not open file %s\n", X2SYS_program, tag_file);
		exit (EXIT_FAILURE);
	}
	while (fgets (line, BUFSIZ, fp) && line[0] == '#');	/* Skip comment records */
	GMT_chop (line);	/* Remove trailing CR or LF */

	GMT_set_processed_option (8, FALSE);	/* In case -R has been processed before */
	
	while ((GMT_strtok (line, " \t", &pos, p))) {	/* Process the -D -I -R -G -W arguments from the header */
		if (p[0] == '-') {
			switch (p[1]) {
				/* Common parameters */
				case 'R':
					if (GMT_get_common_args (&p[2], &B->x_min, &B->x_max, &B->y_min, &B->y_max)) {
						fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
						exit (EXIT_FAILURE);
					}
					break;

				/* Supplemental parameters */

				case 'D':
					strcpy (sfile, &p[2]);
					break;
				case 'G':	/* Geographical coordinates, set discontinuity */
					geographic = TRUE;
					geodetic = 0;
					if (p[2] == 'd') geodetic = 2;
					break;
				case 'I':
					if (GMT_getinc (&p[2], &B->bin_x, &B->bin_y)) {
						fprintf (stderr, "%s: Error processing %s setting in %s!\n", X2SYS_program, &p[1], tag_file);
						exit (EXIT_FAILURE);
					}
				case 'W':
					n = sscanf (&p[2], "%lf/%lf", &B->time_gap, &dist);
					if (n == 2) B->dist_gap = dist;
					break;
				default:
					fprintf (stderr, "%s: Bad arg in x2sys_set_system! (%s)\n", X2SYS_program, p);
					exit (EXIT_FAILURE);
					break;
			}
		}
	}
	x2sys_fclose (tag_file, fp);
	
	*s = x2sys_initialize (sfile, G);	/* Initialize X2SYS and info structure */

	if (geographic) {
		if (geodetic == 0 && (B->x_min < 0 || B->x_max < 0)) {
			fprintf (stderr, "%s: Your -R and -G settings are contradicting each other!\n", X2SYS_program);
			exit (EXIT_FAILURE);
		}
		else if  (geodetic == 2 && (B->x_min > 0 && B->x_max > 0)) {
			fprintf (stderr, "%s: Your -R and -G settings are contradicting each other!\n", X2SYS_program);
			exit (EXIT_FAILURE);
		}
		(*s)->geographic = TRUE;
		(*s)->geodetic = geodetic;	/* Override setting */
		if (fabs (fabs (B->x_max - B->x_min) - 360.0) <= GMT_CONV_LIMIT) B->periodic = 1;
	}
	x2sys_path_init (TAG);		/* Prepare directory paths to data */
}

void x2sys_bix_init (struct X2SYS_BIX *B, BOOLEAN alloc)
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
	I->trackname = (char *) GMT_memory (VNULL, (size_t)(strlen(name)+1), sizeof (char), X2SYS_program);
	strcpy (I->trackname, name);
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

int x2sys_bix_read_tracks (char *TAG, struct X2SYS_BIX *B, int mode)
{
	/* mode = 0 gives linked list, mode = 1 gives fixed array */
	int id, flag, last_id = -1;
	size_t n_alloc = GMT_CHUNK;
	char track_file[BUFSIZ], track_path[BUFSIZ], line[BUFSIZ], name[BUFSIZ];
	FILE *ftrack;
	struct X2SYS_BIX_TRACK_INFO *this_info = VNULL;

	sprintf (track_file, "%s_tracks.d", TAG);
	x2sys_path (track_file, track_path);

	if ((ftrack = GMT_fopen (track_path, "r")) == NULL) {
		fprintf (stderr, "%s: Could not find %s\n", X2SYS_program, track_file);
		exit (EXIT_FAILURE);
	}


	if (mode == 1)
		B->head = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory (VNULL, n_alloc, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);
	else
		B->head = this_info = x2sys_bix_make_entry ("-", 0, 0);

	fgets (line, BUFSIZ, ftrack);	/* Skip header record */
	while (fgets (line, BUFSIZ, ftrack)) {
		GMT_chop (line);	/* Remove trailing CR or LF */
		sscanf (line, "%s %d %d", name, &id, &flag);
		if (mode == 1) {
			if (id >= (int)n_alloc) {
				while (id >= (int)n_alloc) n_alloc += GMT_CHUNK;
				B->head = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory ((void *)B->head, n_alloc, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);
			}
			B->head[id].track_id = id;
			B->head[id].flag = flag;
			B->head[id].trackname = (char *) GMT_memory (VNULL, (size_t)(strlen(name)+1), sizeof (char), X2SYS_program);
			strcpy (B->head[id].trackname, name);
		}
		else {
			this_info->next_info = x2sys_bix_make_entry (name, id, flag);
			this_info = this_info->next_info;
		}
		if (id > last_id) last_id = id;
	}
	GMT_fclose (ftrack);
	last_id++;
	if (mode == 1) B->head = (struct X2SYS_BIX_TRACK_INFO *) GMT_memory ((void *)B->head, (size_t)last_id, sizeof (struct X2SYS_BIX_TRACK_INFO), X2SYS_program);

	return (last_id);
}

void x2sys_bix_read_index (char *TAG, struct X2SYS_BIX *B)
{
	char index_file[BUFSIZ], index_path[BUFSIZ];
	FILE *fbin;
	int index = 0, no_of_tracks, i, id, flag;

	sprintf (index_file, "%s_index.b",  TAG);
	x2sys_path (index_file, index_path);

	if ((fbin = GMT_fopen (index_path, "rb")) == NULL) {
		fprintf (stderr,"%s: Could not open %s\n", X2SYS_program, index_path);
		exit (EXIT_FAILURE);
	}

	B->base = (struct X2SYS_BIX_DATABASE *) GMT_memory (VNULL, (size_t)B->nm_bin, sizeof (struct X2SYS_BIX_DATABASE), X2SYS_program);

	while ((fread ((void *)(&index), sizeof (int), (size_t)1, fbin)) == 1) {
		fread ((void *)(&no_of_tracks), sizeof (int), (size_t)1, fbin);
		B->base[index].first_track = B->base[index].last_track = x2sys_bix_make_track (0, 0);
		for (i = 0; i < no_of_tracks; i++) {
			fread ((void *)(&id), sizeof (int), (size_t)1, fbin);
			fread ((void *)(&flag), sizeof (int), (size_t)1, fbin);
			B->base[index].last_track->next_track = x2sys_bix_make_track (id, flag);
			B->base[index].last_track = B->base[index].last_track->next_track;
			B->base[index].n_tracks++;
		}
	}
	GMT_fclose (fbin);
}

int x2sys_bix_get_ij (double x, double y, int *i, int *j, struct X2SYS_BIX *B)
{
	int index = 0;

	*j = (y == B->y_max) ? B->ny_bin - 1 : (int)floor ((y - B->y_min) * B->i_bin_y);
	if ((*j) < 0 || (*j) >= B->ny_bin) {
		fprintf (stderr, "x2sys_binlist: j (%d) outside range implied by -R -I! [0-%d>\n", *j, B->ny_bin);
		exit (EXIT_FAILURE);
	}
	*i = (x == B->x_max) ? B->nx_bin - 1 : (int)floor ((x - B->x_min)  * B->i_bin_x);
	if (B->periodic) {
		while (*i < 0) *i += B->nx_bin;
		while (*i >= B->nx_bin) *i -= B->nx_bin;
	}
	if ((*i) < 0 || (*i) >= B->nx_bin) {
		fprintf (stderr, "x2sys_binlist: i (%d) outside range implied by -R -I! [0-%d>\n", *i, B->nx_bin);
		exit (EXIT_FAILURE);
	}
	index = (*j) * B->nx_bin + (*i);
	if (index < 0 || index >= B->nm_bin) {
		fprintf (stderr, "x2sys_binlist: Index (%d) outside range implied by -R -I! [0-%d>\n", index, B->nm_bin);
		exit (EXIT_FAILURE);
	}

	return (index);
}

/* gmtmggpath_init reads the SHAREDIR/share/mgg/gmtfile_paths file and gets all
 * the gmtfile directories.
 */
 
void x2sys_path_init (char *TAG)
{
	int i;
	char file[BUFSIZ], line[BUFSIZ];
	FILE *fp;

	x2sys_set_home ();

	sprintf (file, "%s%c%s_paths.txt", X2SYS_HOME, DIR_DELIM, TAG);

	n_x2sys_paths = 0;

	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "%s: Warning: path file %s for %s files not found\n", X2SYS_program, file, TAG);
		fprintf (stderr, "%s: (Will only look in current directory for such files)\n", X2SYS_program);
		return;
	}

	while (fgets (line, BUFSIZ, fp) && n_x2sys_paths < MAX_DATA_PATHS) {
		if (line[0] == '#') continue;	/* Comments */
		if (line[0] == ' ' || line[0] == '\0') continue;	/* Blank line */
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
 
int x2sys_get_data_path (char *track_path, char *track)
{
	int id;
	char geo_path[BUFSIZ];

	/* First look in current directory */

	if (!access(track, R_OK)) {
		strcpy(track_path, track);
		return (0);
	}

	/* Then look elsewhere */

	for (id = 0; id < n_x2sys_paths; id++) {
		sprintf (geo_path, "%s%c%s", x2sys_datadir[id], DIR_DELIM, track);
		if (!access (geo_path, R_OK)) {
			strcpy (track_path, geo_path);
			return (0);
		}
	}
	return(1);
}
