/* ttt_subs.c
 * Functions used by both ttt.c and ttt_pick.c
 *
 * COPYRIGHT (c) 1993-2024 GEOWARE
 * P. Wessel, Jan 1, 2024
 */

/* Generic function used to allocate memory */

static void double_swab (double *d) {
	unsigned int jj, *ii = (unsigned int *)d;  
	jj = TTT_swab4 (ii[0]);
 	ii[0] = TTT_swab4 (ii[1]);
	ii[1] = jj;
}

static void *ttt_memory (void *prev_addr, TTT_LONG n, size_t item_size, char *progname) {
        void *tmp = NULL;

        if (n == 0) return ((void *) NULL); /* Take care of n = 0 */

        if (prev_addr) {
                if ((tmp = realloc ((void *) prev_addr, (size_t)(n * item_size))) == NULL) {
                        fprintf (stderr, "Fatal Error: %s could not reallocate more memory, n = %" TTT_LL "d\n", progname, n);
                        exit (99);
                }
        }
        else {
                if ((tmp = calloc ((size_t)n, item_size)) == NULL) {
                        fprintf (stderr, "Fatal Error: %s could not allocate memory, n = %" TTT_LL "d\n", progname, n);
                        exit (99);
                }
        }
        return (tmp);
}       

static TTT_LONG ttt_file_prefix (char *file) {
	TTT_LONG i;
	for (i = (TTT_LONG)strlen (file); i && file[i] != '.'; i--);	/* Find and return start of the period of an extension, or 0 */
	return (i);
}

static TTT_LONG ttt_file_format (char *file) {
	TTT_LONG i, in_format;
	if (!(i = ttt_file_prefix (file))) {
		fprintf (stderr, "ttt: TTT file (%s) has no extension\n", file);
		exit (17);
	}
	i++;	/* Move to start of extension */
	/* Determine which extension and hence which format */

	for (in_format = 0; in_format < 5; in_format++) if (!strcmp (&file[i], TTT_ext[in_format])) break;
	if (in_format == 5) {
		fprintf (stderr, "ttt: TTT file (%s) has no vaid extension (asc, b, i2, flt)\n", file);
		exit (17);
	}
	if (in_format == 4) in_format = TTT_IS_GRIDFLOAT;
	return (in_format);
}

static void ttt_skip_header (FILE *fp, TTT_LONG in_format) {
	int c;
	char record[BUFSIZ];
	
	if (in_format == TTT_IS_GRIDFLOAT) return;	/* No header in the binary file */
	if (in_format == TTT_IS_GMT_FLOAT || in_format == TTT_IS_GMT_SHORT) {	/* Skip a GMT binary header */
		if (fseek (fp, (long)GRD_HEADER_SIZE, SEEK_SET)) {
			fprintf (stderr, "ttt: Error seeking past header\n");
			exit (18);
		}
		return;
	}
	/* Here we have in_format == TTT_IS_ESRIASCII */
	for (c = 0; c < 5; c++) fgets (record, BUFSIZ, fp);	/* Skip 5 required recs (nx,ny,xmin,ymin,xinc) */
	c = fgetc (fp);	/* Get first char of next line... */
	ungetc (c, fp);	/* ...and put it back where it came from */
	if (c == 'n' || c == 'N') fgets (record, BUFSIZ, fp);	/* Skip optional nodata record */
}

/* Reads header from bathymetry grid file */

static TTT_LONG ttt_read_header_gmt (char *file_name, struct GRD_HEADER *h, double west, double east, double south, double north) {
	TTT_LONG nx, swabbing = FALSE;
	FILE *fp = NULL;

	if ((fp = fopen (file_name, "rb")) == NULL) {
		fprintf (stderr, "ttt: Error opening file %s\n", file_name);
		exit (17);
	}

	if (fread ((void *)&h->nx, 3*sizeof (int), (size_t)1, fp) != 1 || fread ((void *)h->wesn, sizeof (struct GRD_HEADER) - ((long)&h->wesn - (long)&h->nx), (size_t)1, fp) != 1) {
		fprintf (stderr, "ttt: Error reading header from file %s\n", file_name);
		exit (18);
	}
	/* Check for swab */
	nx = irint ((h->wesn[XHI] - h->wesn[XLO]) / h->inc[TTT_X]) + !h->registration;
	if (nx != (TTT_LONG)h->nx) {
		swabbing = TRUE;
		h->nx = TTT_swab4 (h->nx);
		if (nx != h->nx) {
			fprintf (stderr, "ttt: Error decoding file %s\n", file_name);
			exit (18);
		}
		h->ny = TTT_swab4 (h->ny);
		h->registration = TTT_swab4 (h->registration);
		double_swab (&(h->wesn[XLO]));
		double_swab (&(h->wesn[XHI]));
		double_swab (&(h->inc[TTT_X]));
		double_swab (&(h->wesn[YLO]));
		double_swab (&(h->wesn[YHI]));
		double_swab (&(h->inc[TTT_Y]));
		double_swab (&(h->z_min));
		double_swab (&(h->z_max));
		double_swab (&(h->z_scale_factor));
		double_swab (&(h->z_add_offset));
	}
	h->xy_off = 0.5 * h->registration;
	if (west == 0.0 && south == north) return (swabbing);	/* Not requesting a subset */
	
	if (south < h->wesn[YLO] || south >= h->wesn[YHI] || north > h->wesn[YHI]) {
		fprintf (stderr, "ttt: Sub-region not compatible with grid domain of %s\n", file_name);
		exit (9);
	}
	h->periodic = (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < CONV_LIMIT);	/* Periodic in 360 longitude */
	while (west > h->wesn[XLO]) west -= 360.0, east -= 360.0;
	while (west < h->wesn[XLO]) west += 360.0, east += 360.0;
	if (east > h->wesn[XHI] && !h->periodic) {
		fprintf (stderr, "ttt: Sub-region not compatible with grid domain of %s\n", file_name);
		exit (9);
	}
	return (swabbing);
}

static TTT_LONG ttt_read_header (char *file_name, struct GRD_HEADER *h, double west, double east, double south, double north) {
	TTT_LONG in_format, swap = FALSE;
	int c;
	char *p = NULL, record[BUFSIZ], file[BUFSIZ], item[64];
	FILE *fp = NULL;
	
	in_format = ttt_file_format (file_name);	/* Determine which extension and hence which format */
	
	if (in_format == TTT_IS_GMT_FLOAT || in_format == TTT_IS_GMT_SHORT) return (ttt_read_header_gmt (file_name, h, west, east, south, north));	/* GMT native grids */
	
	/* Here we must deal with either a separate header file or the first part of the ascii data file */
	
	if (in_format == TTT_IS_GRIDFLOAT) {	/* Must open the .hdr file instead */
		strcpy (file, file_name);
		p = strstr (file, ".flt");
		strcpy (p, ".hdr");
	}
	else
		strcpy (file, file_name);
	if ((fp = fopen (file, TTT_rmode[in_format])) == NULL) {
		fprintf (stderr, "ttt: Error opening file %s\n", file);
		exit (17);
	}
	
	(void)fgets (record, BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &h->nx) != 1) {
		fprintf (stderr, "ttt: Error decoding ncols record in file %s\n", file);
		exit (17);
	}
	(void)fgets (record, BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &h->ny) != 1) {
		fprintf (stderr, "ttt: Error decoding nrows record in file %s\n", file);
		exit (17);
	}
	(void)fgets (record, BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &h->wesn[XLO]) != 1) {
		fprintf (stderr, "ttt: Error decoding xll record in file %s\n", file);
		exit (17);
	}
	if (!strncmp (record, "xllcorner", (size_t)9) || !strncmp (record, "XLLCORNER", (size_t)9)) h->registration = GMT_PIXEL_REG;	/* Pixel grid */
	(void)fgets (record, BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &h->wesn[YLO]) != 1) {
		fprintf (stderr, "ttt: Error decoding yll record in file %s\n", file);
		exit (17);
	}
	if (!strncmp (record, "yllcorner", (size_t)9) || !strncmp (record, "YLLCORNER", (size_t)9)) h->registration = GMT_PIXEL_REG;	/* Pixel grid */
	(void)fgets (record, BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &h->inc[TTT_X]) != 1) {
		fprintf (stderr, "ttt: Error decoding cellsize record in file %s\n", file);
		exit (17);
	}
	/* Handle the optional nodata_value record */
	c = fgetc (fp);	/* Get first char of next line... */
	ungetc (c, fp);	/* ...and put it back where it came from */
	if (c == 'n' || c == 'N') {	/*	Assume this is a nodata_value record since we found an 'n|N' */
		(void)fgets (record, BUFSIZ, fp);
		if (sscanf (record, "%*s %lf", &h->nan_value) != 1) {
			fprintf (stderr, "ttt: Error decoding nodata_value record in file %s\n", file);
			exit (17);
		}
	}
	if (in_format == TTT_IS_GRIDFLOAT) {	/* Handle the BYTEORDER record */
		(void)fgets (record, BUFSIZ, fp);
		if (sscanf (record, "%*s %s", item) != 1) {
			fprintf (stderr, "ttt: Error decoding byteorder record in file %s\n", file);
			exit (17);
		}
#ifdef WORDS_BIGENDIAN
		swap = (item[0] == 'L');
#else
		swap = (item[0] != 'L');
#endif
	}
	h->z_scale_factor = 1.0;	h->z_add_offset = 0.0;	/* Set the defaults */
	h->inc[TTT_Y] = h->inc[TTT_X];	/* All such grids are square */
	h->wesn[XHI] = h->wesn[XLO] + (h->nx - 1 + h->registration) * h->inc[TTT_X];
	h->wesn[YHI] = h->wesn[YLO] + (h->ny - 1 + h->registration) * h->inc[TTT_Y];
	
	return (swap);
}

static TTT_LONG ttt_skip_records (FILE *fp, struct GRD_HEADER *h, TTT_LONG n_records, size_t item_size, TTT_LONG in_format) {
	TTT_LONG i;
	float f;
	if (n_records == 0) return (0);	/* Nothing to skip */
	if (in_format != TTT_IS_ESRIASCII) return (fseek (fp, (long)(n_records * h->nx * item_size), SEEK_CUR));
	/* Here we must skip ascii points */
	for (i = 0; i < n_records; i++) if (fscanf (fp, "%f", &f) != 1) return (-1);
	return (0);
}

/* Allocates space and reads bathymetry from one of four formats */

static TTT_LONG ttt_read_grid (char *file_name, struct GRD_HEADER *h, float **s, double west, double east, double south, double north, TTT_LONG pad, TTT_LONG swabbing) {	/* file_format = 0 (float) or 1 (short int), 2 (hdr/flt), 3 (asc) */
	TTT_LONG i, j, k, p, half, i180, adjustment, swap, skip_x, skip_y, wrap_i, nx, ny, row_width, col_height, in_format;
	unsigned int *i_ptr = NULL, wrap_check = 0, periodic = 0;
	size_t item_size;
	short int *s_record = NULL;
	float *f_record = NULL;
	FILE *fp = NULL;
	float TTT_NaN;
	TTT_make_NaN (TTT_NaN);

	swap = ttt_read_header (file_name, h, west, east, south, north);
	if (swap != swabbing) {
		fprintf (stderr, "ttt: Mismatch in swabbing for file %s\n", file_name);
		exit (17);
	}
	
	in_format = ttt_file_format (file_name);	/* Determine which extension and hence which format */

	if ((fp = fopen (file_name, TTT_rmode[in_format])) == NULL) {
		fprintf (stderr, "ttt: Error reading file %s\n", file_name);
		exit (17);
	}
	ttt_skip_header (fp, in_format);	/* Position pointer to start of data section, if needed */

	if (west == east && south == north) {	/* No subset specified, use entire grid */
		west = h->wesn[XLO];	east = h->wesn[XHI];
		south = h->wesn[YLO];	north = h->wesn[YHI];
	}
	else {
		while (west > h->wesn[XLO]) west -= 360.0, east -= 360.0;
		while (west < h->wesn[XLO]) west += 360.0, east += 360.0;
		periodic = (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < CONV_LIMIT);	/* Periodic in 360 longitude */
	}
	nx = irint ((east-west)/h->inc[TTT_X]) + !h->registration;
	ny = irint ((north-south)/h->inc[TTT_Y]) + !h->registration;
	skip_x  = irint ((west-h->wesn[XLO])/h->inc[TTT_X]);
	skip_y = irint ((h->wesn[YHI] - north)/h->inc[TTT_Y]);
	wrap_check = (skip_x && periodic && h->registration == 0);	/* Must loop out for the repeating node value at west/east */
	row_width = nx + 2 * pad;
	col_height = ny + 2 * pad;

	if (sizeof (TTT_LONG) == 4) {	/* 32-bit compilation */
		if (((((double)row_width) * ((double)col_height) * sizeof (float)) / pow (2.0, 32.0)) >= 1.0) {
			fprintf (stderr, "ttt: File %s requires more memory than is available in 32-bit mode.\n", file_name);
			fprintf (stderr, "ttt: Recompile and run in 64-bit mode.\n");
			exit (24);
		}
	}
	*s = (float *)ttt_memory (NULL, row_width * col_height, sizeof (float), "ttt");
	if (in_format == TTT_IS_GMT_SHORT) {	/* Use short int storage */
		item_size = sizeof (short int);
		s_record = (short int *)ttt_memory (NULL, (TTT_LONG)h->nx, item_size, "ttt");
	}
	else {	/* Use float storage */
		item_size = sizeof (float);
		f_record = (float *)ttt_memory (NULL, (TTT_LONG)h->nx, item_size, "ttt");
	}

	if (ttt_skip_records (fp, h, skip_y, item_size, in_format)) {
		fprintf (stderr, "ttt: Error reading file %s\n", file_name);
		exit (18);
	}
	
	if (in_format == TTT_IS_GMT_SHORT) {	/* Short integer data reading deca-seconds */
		for (j = 0; j < ny; j++) {
			if (fread ((void *)s_record, item_size, h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error reading file %s\n", file_name);
				exit (18);
			}
			k = IJ(0,j,row_width,pad);
			for (i = 0; i < nx; i++, k++) {
				wrap_i = skip_x + i;	/* This may march off the right end of grid, hence next 2 lines: */
				if (wrap_i >= h->nx && wrap_check) wrap_i++;	/* Wrapping around periodic gridline-registrered grid we must skip the duplicate repeating node */
				p = wrap_i % h->nx;	/* Cannot become zero when wrap_check is true */
				if (swabbing) s_record[p] = TTT_swab2 (s_record[p]);
				(*s)[k] = (s_record[p] == SHRT_MAX || s_record[p] == SHRT_MIN) ? TTT_NaN : (float) (s_record[p] * h->z_scale_factor + h->z_add_offset);
			}
		}
		free ((void *)s_record);
	}
	else if (in_format != TTT_IS_ESRIASCII) {	/* Binary floats */
		for (j = 0; j < ny; j++) {
			if (fread ((void *)f_record, item_size, h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error reading file %s\n", file_name);
				exit (18);
			}
			k = IJ(0,j,row_width,pad);
			for (i = 0; i < nx; i++, k++) {
				wrap_i = skip_x + i;	/* This may march off the right end of grid, hence next 2 lines: */
				if (wrap_i >= h->nx && wrap_check) wrap_i++;	/* Wrapping around periodic gridline-registrered grid we must skip the duplicate repeating node */
				p = wrap_i % h->nx;	/* Cannot become zero when wrap_check is true */
				if (swabbing) {
					i_ptr = (unsigned int *)&f_record[p];
					*i_ptr = TTT_swab4 (*i_ptr);
				}
				(*s)[k] = (float) (f_record[p] * h->z_scale_factor + h->z_add_offset);
			}
		}
		free ((void *)f_record);
	}
	else {	/* Ascii grid */
		for (j = 0; j < ny; j++) {
			for (i = 0; i < h->nx; i++) {	/* Read the entire ascii row */
				if (fscanf (fp, "%f", &f_record[i]) != 1) {
					fprintf (stderr, "ttt: Error reading file %s\n", file_name);
					exit (18);
				}
			}
			k = IJ(0,j,row_width,pad);
			for (i = 0; i < nx; i++, k++) {
				wrap_i = skip_x + i;	/* This may march off the right end of grid, hence next 2 lines: */
				if (wrap_i >= h->nx && wrap_check) wrap_i++;	/* Wrapping around periodic gridline-registrered grid we must skip the duplicate repeating node */
				p = wrap_i % h->nx;	/* Cannot become zero when wrap_check is true */
				(*s)[k] = (f_record[p] == h->nan_value) ? TTT_NaN : (float) f_record[p];
			}
		}
		free ((void *)f_record);
	}
	fclose (fp);
	
	/* Modify header if we read a subset */
	
	h->wesn[XLO] = west;	h->wesn[XHI] = east;	h->nx = (unsigned int)nx;
	h->wesn[YLO] = south;	h->wesn[YHI] = north;	h->ny = (unsigned int)ny;
	
	/* Apply BC to the strips */
	
	adjustment = (h->registration) ? 1 : 0;
	if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < CONV_LIMIT) {	/* Periodic in 360 longitude */
		h->periodic = TRUE;
		for (j = 0; j < h->ny; j++) {
			k = IJ(0,j,row_width,pad);
			for (p = 1; p <= pad; p++) {
				(*s)[k+h->nx+p-1] = (*s)[k+p-adjustment];
				(*s)[k-p] = (*s)[k+h->nx-p-1+adjustment];
			}
		}
	}
	half = h->nx / 2;
	if (fabs (h->wesn[YHI] - 90.0) < CONV_LIMIT && h->periodic) {	/* N pole wrap */
		h->wrap_n = TRUE;
		k = IJ(0,0,row_width,pad);
		for (i = -pad; i < (h->nx + pad); i++) {
			i180 = i + half;
			if (i180 >= row_width) i180 -= row_width;
			for (p = 1; p <= pad; p++) (*s)[k+i-p*row_width] = (*s)[k+i180+(p-adjustment)*row_width];
		}
	}
	if (fabs (h->wesn[YLO] + 90.0) < CONV_LIMIT && h->periodic) {	/* S pole wrap */
		h->wrap_s = TRUE;
		k = IJ(0,h->ny-1,row_width,pad);
		for (i = -pad; i < (h->nx + pad); i++) {
			i180 = i + half;
			if (i180 >= row_width) i180 -= row_width;
			for (p = 1; p <= pad; p++) (*s)[k+i+p*row_width] = (*s)[k+i180-(p-adjustment)*row_width];
		}
	}
	return (0);
}

static double ttt_great_circle_dist (double lon1, double lat1, double lon2, double lat2) {
	/* great circle distance on a sphere in degrees */

	double C, a, b, c, cosC, cosa, cosb, cosc, sina, sinb;

	if ((lat1 == lat2) && (lon1 == lon2)) return (0.0);

	a = D2R * (90.0 - lat2);
	b = D2R * (90.0 - lat1);

	C = D2R * (lon2 - lon1);

	sina = sin (a);
	cosa = cos (a);
	sinb = sin (b);
	cosb = cos (b);
	cosC = cos (C);

	cosc = cosa * cosb + sina * sinb * cosC;
	if (cosc<-1.0) c=M_PI; else if (cosc>1) c=0.0; else c=acos(cosc);

	return (c * R2D);
}

static double txt2fl (char *t, char neg) {
	int i, n;
	double x, d, m, s;

	for (i = n = 0; t[i]; i++) if (t[i] == ':') n++, t[i] = ' ';
	switch (n) {
		case 2:	/* ddd:mm:ss.xx */
			sscanf (t, "%lf %lf %lf", &d, &m, &s);
			x = d + m/60.0 + s/3600.0;
			break;
		case 1:	/* ddd:mm */
			sscanf (t, "%lf %lf", &d, &m);
			x = d + m/60.0;
			break;
		default:	/* ddd.xx */
			x = atof (t);
			break;
	}
	if (t[0] == '-' && x > 0.0) x = -x;
	if (toupper((int)t[strlen(t)-1]) == neg) x = -x;
	return (x);
}

#if HAVE_HYPOT == 0
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

static double TTT_hypot (double x, double y) {

	double	a, b, c, d, r, s, t;

	/* A complete implementation of IEEE exceptional values
	would also return +Inf if either x or y is +/- Inf  */

	if (x == 0.0) return (fabs(y));
	if (y == 0.0) return (fabs(x));

	a = fabs(x);
	b = fabs(y);

	/* If POSIX defined M_SQRT2 (= sqrt(2.0) to machine
	precision) then we could say
		if (a == b) return (a * M_SQRT2);
	*/

	if (a < b) {
		c = b;
		d = a/b;
	}
	else if (a == b) {
		return (a * M_SQRT2);	/* Added 17 Aug 2001  */
	}
	else {
		c = a;
		d = b/a;
	}

	/* DBL_EPSILON is defined in POSIX float.h
	as the smallest positive e such that 1+e > 1
	in floating point.  */

	if (d < DBL_EPSILON) return (c);

	s = d*d;
	if (s > DBL_EPSILON) return (c * sqrt (1.0 + s) );

	t = 1.0 + d;
	s = (2 * (d/t)) / t;

	r = t * sqrt (1.0 - s);

	if (r <= 1.0) return (c);

	return (c * r);
}

#endif
