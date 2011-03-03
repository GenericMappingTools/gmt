/*-----------------------------------------------------------------
 *	$Id: x2sys.h,v 1.56 2011-03-03 21:02:51 guru Exp $
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
/* x2sys.h contains the declaration for the X2SYS and XOVER structures
 * used in the XSYSTEM programs
 *
 * Author:	Paul Wessel
 * Date:	18-OCT-2005
 * Version:	1.1, based on the spirit of the old xsystem code
 *		1.2 Includes support for MGD77+
 *
 */

/* X2SYS IS POSIX.1 COMPLIANT AND REQUIRES GMT AND ITS MGG supplement */

/*-----------------------------------------------------------------

 The data files that can be understood by X2SYS can either be ASCII
 or binary files.  In either case, the file formats must obey some
 simple and reasonable rules:

  ASCII data records:
  -------------------

    -Type 1:  columns separated by tabs, spaces, or commas:
	Each field type must be set to 'a' for ascii string
    -Type 2:  FORTRAN card format, all glued together:
	Each field type must be set to 'A' for ascii string
	and the start-stop column must be appended at the end of
	the info line.

  BINARY data records:
  --------------------

  Each field of the input data file must be identified as one of the
  following types:

	c - signed 1-byte character
	u - unsigned 1-byte character
	h - signed 2-byte integer
	i - signed 4-byte integer
	l - signed 4- or 8-byte integer (long)
	f - 4-byte floating point single precision
	d - 8-byte floating point double precision

	E.g., the GMT MGG format is iiihhh

  This information must be conveyed via the *.def files (-D)
------------------------------------------------------------------*/

#include "mgd77.h"
#include "gmt_mgg.h"

#ifdef WIN32
#define _chmod(path,mode) chmod(path,mode)
extern int _chmod (const char *path, int mode);
#else
#include <sys/stat.h>
#endif

#define S_RDONLY 0000444

#define X2SYS_VERSION "1.1"

/* Make sure structure sizes are multiples of 8 */

struct X2SYS_TRK_INFO {			/* Structure with info about one track */
	char name[32];			/* Name of track */
	int year;			/* Year the track was collected */
	int nx_int;			/* Total number of internal track cross-over points */
	int nx_ext;			/* Total number of external track cross-over points */
	int flag;			/* Processing flags */
};

struct X2SYS_SET {		/* Structure with info for a data type along the track */
	char id_set[16];		/* Name of this data type or set */
	int nx_int;			/* Number of internal data crossovers */
	int nx_ext;			/* Number of external data crossovers */
	double mean_int;		/* Mean data internal xover value */
	double mean_ext;		/* Mean data external xover value */
	double stdev_int;		/* St. Dev. of the internal data crossovers */
	double stdev_ext;		/* Same for external xovers */
	double dc_shift;		/* Best fitting d.c.-shift for data set */
	double drift_rate;		/* Best fitting drift rate for data set [units/sec] */
};

struct X2SYS_TRK {			/* Structure for each track */
	struct X2SYS_TRK_INFO info;	/* Track-level information */
	struct X2SYS_SET *set;		/* Array of structures; one for each data set */
	struct X2SYS_TRK *next_track;	/* Pointer to next track */
};

struct X2SYS_XOVER_SET {		/* Structure with info on one data type cross-over values */
	double x_val;			/* Data cross-over mismatch */
	double ave;			/* Average data value at crossover */
	double xhead[2];		/* Azimuth at cross-over along track 1 and 2 */
};

struct X2SYS_CORR {		/* Structure with the corrections for each leg */
	char id_name[16];		/* Name of track */
	int year;			/* Year the track was collected */
	double dc_shift;		/* Best fitting d.c.-shift for data set */
	double drift_rate;		/* Best fitting drift rate for data set [units/sec] */
};

struct X2SYS_INFO {
	/* Information of this datasets particular organization */

	char *TAG;			/* The system TAG */
	int n_fields;			/* Number of input columns */
	int n_out_columns;		/* Number of output columns */
	int n_data_cols;		/* Number of data columns (other than x,y,t) */
	size_t rec_size;		/* Number of bytes for a potential x2sys_dbase_*.b file */
	int x_col[2], y_col[2];		/* Column numbers for x, y, for input and output */
	int t_col[2];			/* Column numbers for t, for input and output */
	int skip;			/* Number of header records to skip */
	int flags;			/* Various processing flags for internal use */
	int *out_order;			/* Array with column number in the order for output */
	int *use_column;		/* Array of T/F for which columns to use */
	int geodetic;			/* How longitudes should be stored: 0: (0-360), 1: (-360,0), 2 (-180/+180) */
	int dist_flag;			/* How distances are calulated: (0 = Cartesian, 1 = Flat earth, 2 = great circle, 3 = geodesic) */
	PFI read_file;			/* Pointer to function that reads this file */
	int file_type;			/* 0 = ASCII, 1 = native binary, 2 = netCDF */
	GMT_LONG ascii_out;		/* TRUE if output should be in ascii */
	GMT_LONG multi_segment;		/* TRUE if there are multiple segments in this file */
	GMT_LONG geographic;		/* TRUE if x/y data are lon/lat */
	GMT_LONG ms_next;		/* TRUE if we just read 1st record in a new segments in this file */
	char unit[2][2];		/* Units for distance (c = Cartesian, e = meter, k = km, m = miles, n = nautical miles)
	 				   and speed (c = Cartesian, e = m/s, k = km/hr, m = miles/hr, n = knots) */
	char ms_flag;			/* Multi-segment header flag */
	char suffix[16];		/* Suffix for these data files */
	char fflags[BUFSIZ];		/* Text copy of selected columns */
	char path[BUFSIZ];		/* Full path to current data file */
	struct X2SYS_DATA_INFO *info;	/* Array of info for each data field */
};

struct X2SYS_DATA_INFO {
	double nan_proxy;	/* Value that signifies lack of data (NaN) */
	double scale;		/* Input value should be multiplied by this value */
	double offset;		/* And then add this value */
	int start_col;		/* For cardformat: starting column */
	int stop_col;		/* For cardformat: last column */
	int n_cols;		/* For cardformat: number of columns */
	GMT_LONG has_nan_proxy;	/* TRUE if there is a special value that indicates NaN */
	GMT_LONG has_nans;	/* TRUE if there are NaNs in this field */
	GMT_LONG do_scale;	/* TRUE if scale != 1 or offset != 0 */
	char name[32];		/* Name of this data type */
	char format[32];	/* Output print format for ascii conversion */
	char intype;		/* Input data type (cuhilfdaA) */
};

struct X2SYS_FILE_INFO {
	/* Information for a particular data file */
	int year;		/* Starting year for this leg */
	GMT_LONG n_rows;		/* Number of rows */
	GMT_LONG n_segments;		/* Number of segments in this file */
	GMT_LONG *ms_rec;		/* Pointer to array with start record for each segment */
	char name[32];		/* Name of cruise or agency */
};

struct X2SYS_BIX {
	/* Information for the track binindex setup */
	double x_min, x_max;	/* Left/Right edge of region */
	double y_min, y_max;	/* Bottom/Top edge of region */
	double bin_x;		/* Spacing between x bins */
	double bin_y;		/* Spacing between y bins */
	double i_bin_x;		/* 1/dx */
	double i_bin_y;		/* 1/dy */
	double time_gap;	/* We have a data-gap if two records differ by this amount in time */
	double dist_gap;	/* We have a data-gap if two records differ by this amount in distance [if there is no time column] */
	int nx_bin;		/* Number of x bins */
	int ny_bin;		/* Number of y bins */
	GMT_LONG nm_bin;		/* Total number of bins */
	int periodic;		/* 1 if x is periodic */
	unsigned int *binflag;	/* The bin array */
	struct X2SYS_BIX_DATABASE *base;
	struct X2SYS_BIX_TRACK_INFO *head;
};

struct X2SYS_BIX_DATABASE {
	int bix;
	int n_tracks;
	struct X2SYS_BIX_TRACK *first_track, *last_track;
};

struct X2SYS_BIX_TRACK {
	int track_id;
	int track_flag;
	struct X2SYS_BIX_TRACK *next_track;
};
	
struct X2SYS_BIX_TRACK_INFO {
	char *trackname;
	int track_id;
	int flag;
	struct X2SYS_BIX_TRACK_INFO *next_info;
};

#define COE_X	0	/* Position of crossover x in data array */
#define COE_Y	1	/* Position of crossover y in data array */
#define COE_T	2	/* Time at crossover in data array */
#define COE_Z	3	/* Observed z value at crossover in data array */
#define COE_D	4	/* Distance at crossover in data array */
#define COE_H	5	/* Azimuth at crossover in data array */
#define COE_V	6	/* Velocity at crossover in data array */

struct X2SYS_COE {	/* Holds the information for a single crossover */
	double data[2][7];
};

struct X2SYS_COE_PAIR {	/* Holds the information for COE between a pair of tracks */
	char trk[2][GMT_TEXT_LEN];	/* Track names */
	int id[2];			/* Internal ID track numbers */
	int year[2];			/* Start year for each track */
	double start[2];		/* Time of first point for each track */
	double stop[2];			/* Time of last point for each track */
	double dist[2];			/* Length of each track */
	int nx;				/* Number of crossovers */
	struct X2SYS_COE *COE;		/* Array of nx COE structures */
};

/* Global variables used by x2sys functions */

extern char *X2SYS_program;
extern char *X2SYS_HOME;
extern double *x2sys_Y;
extern char *x2sys_xover_format;
extern char *x2sys_xover_header;
extern char *x2sys_header;
extern struct MGD77_CONTROL M;

/* Function prototypes.  These can be accessed in user programs */

extern FILE *x2sys_fopen (char *fname, char *mode);
extern int x2sys_access (char *fname, int mode);
extern void x2sys_path (char *fname, char *path);

extern int x2sys_read_record (FILE *fp, double *data, struct X2SYS_INFO *s, struct GMT_IO *G);
extern int x2sys_read_file (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec);
extern int x2sys_read_gmtfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec);
extern int x2sys_read_mgd77file (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec);
extern int x2sys_read_mgd77ncfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec);
extern int x2sys_read_ncfile (char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, GMT_LONG *n_rec);
extern int x2sys_xover_output (FILE *fp, int n, double out[]);
extern int x2sys_n_data_cols (struct X2SYS_INFO *s);
extern int x2sys_read_list (char *file, char ***list, int *n);
extern int x2sys_read_weights (char *file, char ***list, double **weights, int *nf);
extern void x2sys_free_list (char **list, int n);
extern int x2sys_find_track (char *name, char **list, int n);
int x2sys_get_tracknames (int argc, char **argv, char ***tracklist, GMT_LONG *cmdline);

extern double *x2sys_dummytimes (GMT_LONG n);

extern void x2sys_skip_header (FILE *fp, struct X2SYS_INFO *s);
extern int x2sys_fclose (char *fname, FILE *fp);
extern void x2sys_free_info (struct X2SYS_INFO *s);
extern void x2sys_free_data (double **data, int n, struct X2SYS_FILE_INFO *p);
extern int x2sys_pick_fields (char *string, struct X2SYS_INFO *s);

extern int x2sys_initialize (char *TAG, char *fname, struct GMT_IO *G, struct X2SYS_INFO **I);
extern void x2sys_end (struct X2SYS_INFO *X);

extern int x2sys_set_system (char *TAG, struct X2SYS_INFO **s, struct X2SYS_BIX *B, struct GMT_IO *G);
extern void x2sys_bix_init (struct X2SYS_BIX *B, GMT_LONG alloc);
extern struct X2SYS_BIX_TRACK_INFO *x2sys_bix_make_entry (char *name, int id_no, int flag);
extern struct X2SYS_BIX_TRACK *x2sys_bix_make_track (int id, int flag);
extern int x2sys_bix_read_tracks (struct X2SYS_INFO *s, struct X2SYS_BIX *B, int mode, int *ID);
extern int x2sys_bix_read_index (struct X2SYS_INFO *s, struct X2SYS_BIX *B, GMT_LONG swap);
extern int x2sys_bix_get_ij (double x, double y, GMT_LONG *i, GMT_LONG *j, struct X2SYS_BIX *B, GMT_LONG *ID);

extern void x2sys_path_init (struct X2SYS_INFO *s);
extern int x2sys_get_data_path (char *track_path, char *track, char *suffix);
extern int x2sys_err_pass (int err, char *file);
extern void x2sys_err_fail (int err, char *file);
extern const char * x2sys_strerror (int err);

extern GMT_LONG x2sys_read_coe_dbase (struct X2SYS_INFO *s, char *dbase, char *ignorefile, double *wesn, char *fflag, int coe_kind, char *one_trk, struct X2SYS_COE_PAIR **xpairs, GMT_LONG *nx, int *ntracks);
extern void x2sys_free_coe_dbase (struct X2SYS_COE_PAIR *P, GMT_LONG np);
extern void x2sys_get_corrtable (struct X2SYS_INFO *s, char *table, int ntracks, char **trk_name, char *column, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist, struct MGD77_CORRTABLE ***CORR);

#define X2SYS_ASCII		0
#define X2SYS_BINARY		1
#define X2SYS_NETCDF		2

#define X2SYS_NOERROR		0
#define X2SYS_FCLOSE_ERR	-1
#define X2SYS_BAD_DEF		-2
#define X2SYS_BAD_COL		-3
#define X2SYS_TAG_NOT_SET	-4
#define X2SYS_BAD_ARG		-5
#define X2SYS_CONFLICTING_ARGS	-6
#define X2SYS_BIX_BAD_J		-7
#define X2SYS_BIX_BAD_I		-8
#define X2SYS_BIX_BAD_IJ	-9

#define X2SYS_DIST_SELECTION	0
#define X2SYS_SPEED_SELECTION	1
