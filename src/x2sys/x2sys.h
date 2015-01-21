/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2015 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
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
 *		1.3 GMT 5 style
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

/*!
 * \file x2sys.h
 * \brief Declaration for the X2SYS and XOVER structures.
 */

#include "mgd77/mgd77.h"
#include "gmt_dev.h"

/* Here are legacy functions for old GMT MGG supplement needed in x2sys */

struct GMTMGG_REC {	/* Format of old *.gmt file records */
	int time, lat, lon;
	short int gmt[3];
};

#define GMTMGG_NODATA (-32000)		/* .gmt file NaN proxy */
#define MDEG2DEG	0.000001	/* Convert millidegrees to degrees */

#define S_RDONLY 0000444

#define X2SYS_VERSION "1.2"

/* Make sure structure sizes are multiples of 8 */

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

struct X2SYS_FILE_INFO;	/* Forward reference */

struct X2SYS_INFO {
	/* Information of this datasets particular organization */

	char *TAG;			/* The system TAG */
	unsigned int n_fields;		/* Number of input columns */
	unsigned int n_out_columns;	/* Number of output columns */
	unsigned int n_data_cols;	/* Number of data columns (other than x,y,t) */
	size_t rec_size;		/* Number of bytes for a potential x2sys_dbase_*.b file */
	int x_col, y_col, t_col;	/* Column numbers for x, y, and t [== -1 if not set] */
	unsigned int skip;		/* Number of header records to skip */
	unsigned int flags;		/* Various processing flags for internal use */
	unsigned int *in_order;		/* Array with output column number as function of input column */
	unsigned int *out_order;	/* Array with input column number as function of output column */
	bool *use_column;		/* Array of T/F for which columns to use */
	unsigned int geodetic;		/* How longitudes should be stored: 0: (0-360), 1: (-360,0), 2 (-180/+180) */
	unsigned int dist_flag;		/* How distances are calulated: (0 = Cartesian, 1 = Flat earth, 2 = great circle, 3 = geodesic) */
	/* read_file is a pointer to function that reads this file */
	int (*read_file) (struct GMT_CTRL *, char *, double ***, struct X2SYS_INFO *, struct X2SYS_FILE_INFO *, struct GMT_IO *, uint64_t *);
	unsigned int file_type;		/* 0 = ASCII, 1 = native binary, 2 = netCDF */
	bool ascii_out;			/* true if output should be in ascii */
	bool multi_segment;		/* true if there are multiple segments in this file */
	bool geographic;		/* true if x/y data are lon/lat */
	bool ms_next;			/* true if we just read 1st record in a new segments in this file */
	char unit[2][2];		/* Units for distance (c = Cartesian, e = meter, k = km, m = miles, n = nautical miles)
	 				   and speed (c = Cartesian, e = m/s, k = km/hr, m = miles/hr, n = knots) */
	char ms_flag;			/* Multi-segment header flag */
	char suffix[16];		/* Suffix for these data files */
	char fflags[GMT_BUFSIZ];	/* Text copy of selected columns */
	char path[GMT_BUFSIZ];		/* Full path to current data file */
	char separators[8];		/* List of characters used for column separators */
	struct X2SYS_DATA_INFO *info;	/* Array of info for each data field */
};

struct X2SYS_DATA_INFO {
	double nan_proxy;	/* Value that signifies lack of data (NaN) */
	double scale;		/* Input value should be multiplied by this value */
	double offset;		/* And then add this value */
	unsigned int start_col;	/* For cardformat: starting column */
	unsigned int stop_col;	/* For cardformat: last column */
	unsigned int n_cols;	/* For cardformat: number of columns */
	bool has_nan_proxy;	/* true if there is a special value that indicates NaN */
	bool has_nans;	/* true if there are NaNs in this field */
	bool do_scale;	/* true if scale != 1 or offset != 0 */
	char name[32];		/* Name of this data type */
	char format[32];	/* Output print format for ascii conversion */
	char intype;		/* Input data type (cuhilfdaA) */
};

struct X2SYS_FILE_INFO {
	/* Information for a particular data file */
	int year;		/* Starting year for this leg */
	uint64_t n_rows;	/* Number of rows */
	uint64_t n_segments;	/* Number of segments in this file */
	uint64_t *ms_rec;	/* Pointer to array with start record for each segment */
	char name[32];		/* Name of cruise or agency */
};

struct X2SYS_BIX {
	/* Information for the track binindex setup */
	double wesn[4];		/* Left/Right/Bottom/Top edge of region */
	double inc[2];		/* Spacing between x and y bins */
	double i_bin_x;		/* 1/dx */
	double i_bin_y;		/* 1/dy */
	double time_gap;	/* We have a data-gap if two records differ by this amount in time */
	double dist_gap;	/* We have a data-gap if two records differ by this amount in distance [if there is no time column] */
	int nx_bin;	/* Number of x bins */
	int ny_bin;	/* Number of y bins */
	uint64_t nm_bin;	/* Total number of bins */
	bool periodic;	/* 1 if x is periodic */
	unsigned int *binflag;	/* The bin array */
	struct X2SYS_BIX_DATABASE *base;
	struct X2SYS_BIX_TRACK_INFO *head;
};

struct X2SYS_BIX_DATABASE {
	uint32_t bix;
	uint32_t n_tracks;
	struct X2SYS_BIX_TRACK *first_track, *last_track;
};

struct X2SYS_BIX_TRACK {
	uint32_t track_id;
	uint32_t track_flag;
	struct X2SYS_BIX_TRACK *next_track;
};
	
struct X2SYS_BIX_TRACK_INFO {
	char *trackname;
	uint32_t track_id;
	uint32_t flag;
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
	char trk[2][GMT_LEN64];	/* Track names */
	int year[2];		/* Start year for each track */
	unsigned int id[2];		/* Internal ID track numbers */
	unsigned int nx;		/* Number of crossovers */
	double start[2];		/* Time of first point for each track */
	double stop[2];			/* Time of last point for each track */
	double dist[2];			/* Length of each track */
	struct X2SYS_COE *COE;		/* Array of nx COE structures */
};

/* Global variables used by x2sys functions */

EXTERN_MSC char *X2SYS_program;
EXTERN_MSC char *X2SYS_HOME;
EXTERN_MSC double *x2sys_Y;
EXTERN_MSC struct MGD77_CONTROL M;

/* Function prototypes.  These can be accessed in user programs */

EXTERN_MSC FILE *x2sys_fopen (struct GMT_CTRL *GMT, char *fname, char *mode);
EXTERN_MSC int x2sys_access (struct GMT_CTRL *GMT, char *fname, int mode);
EXTERN_MSC void x2sys_path (struct GMT_CTRL *GMT, char *fname, char *path);

EXTERN_MSC int x2sys_read_record (struct GMT_CTRL *GMT, FILE *fp, double *data, struct X2SYS_INFO *s, struct GMT_IO *G);
EXTERN_MSC int x2sys_read_file        (struct GMT_CTRL *GMT, char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, uint64_t *n_rec);
EXTERN_MSC int x2sys_read_gmtfile     (struct GMT_CTRL *GMT, char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, uint64_t *n_rec);
EXTERN_MSC int x2sys_read_mgd77file   (struct GMT_CTRL *GMT, char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, uint64_t *n_rec);
EXTERN_MSC int x2sys_read_mgd77ncfile (struct GMT_CTRL *GMT, char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, uint64_t *n_rec);
EXTERN_MSC int x2sys_read_ncfile      (struct GMT_CTRL *GMT, char *fname, double ***data, struct X2SYS_INFO *s, struct X2SYS_FILE_INFO *p, struct GMT_IO *G, uint64_t *n_rec);
EXTERN_MSC unsigned int x2sys_n_data_cols (struct GMT_CTRL *GMT, struct X2SYS_INFO *s);
EXTERN_MSC int x2sys_read_list (struct GMT_CTRL *GMT, char *file, char ***list, unsigned int *n);
EXTERN_MSC int x2sys_read_weights (struct GMT_CTRL *GMT, char *file, char ***list, double **weights, unsigned int *nf);
EXTERN_MSC void x2sys_free_list (struct GMT_CTRL *GMT, char **list, uint64_t n);
EXTERN_MSC int x2sys_find_track (struct GMT_CTRL *GMT, char *name, char **list, unsigned int n);
int x2sys_get_tracknames (struct GMT_CTRL *GMT, struct GMT_OPTION *options, char ***tracklist, bool *cmdline);

EXTERN_MSC double *x2sys_dummytimes (struct GMT_CTRL *GMT, uint64_t n);

EXTERN_MSC void x2sys_skip_header (struct GMT_CTRL *GMT, FILE *fp, struct X2SYS_INFO *s);
EXTERN_MSC int x2sys_fclose (struct GMT_CTRL *GMT, char *fname, FILE *fp);
EXTERN_MSC void x2sys_free_info (struct GMT_CTRL *GMT, struct X2SYS_INFO *s);
EXTERN_MSC void x2sys_free_data (struct GMT_CTRL *GMT, double **data, unsigned int n, struct X2SYS_FILE_INFO *p);
EXTERN_MSC int x2sys_pick_fields (struct GMT_CTRL *GMT, char *string, struct X2SYS_INFO *s);

EXTERN_MSC int x2sys_initialize (struct GMT_CTRL *GMT, char *TAG, char *fname, struct GMT_IO *G, struct X2SYS_INFO **I);
EXTERN_MSC void x2sys_end (struct GMT_CTRL *GMT, struct X2SYS_INFO *X);

EXTERN_MSC int x2sys_set_system (struct GMT_CTRL *GMT, char *TAG, struct X2SYS_INFO **s, struct X2SYS_BIX *B, struct GMT_IO *G);
EXTERN_MSC void x2sys_bix_init (struct GMT_CTRL *GMT, struct X2SYS_BIX *B, bool alloc);
EXTERN_MSC struct X2SYS_BIX_TRACK_INFO *x2sys_bix_make_entry (struct GMT_CTRL *GMT, char *name, uint32_t id_no, uint32_t flag);
EXTERN_MSC struct X2SYS_BIX_TRACK *x2sys_bix_make_track (struct GMT_CTRL *GMT, uint32_t id, uint32_t flag);
EXTERN_MSC int x2sys_bix_read_tracks (struct GMT_CTRL *GMT, struct X2SYS_INFO *s, struct X2SYS_BIX *B, int mode, uint32_t *ID);
EXTERN_MSC int x2sys_bix_read_index (struct GMT_CTRL *GMT, struct X2SYS_INFO *s, struct X2SYS_BIX *B, bool swap);
EXTERN_MSC int x2sys_bix_get_index (struct GMT_CTRL *GMT, double x, double y, int *i, int *j, struct X2SYS_BIX *B, uint64_t *ID);

EXTERN_MSC void x2sys_path_init (struct GMT_CTRL *GMT, struct X2SYS_INFO *s);
EXTERN_MSC int x2sys_get_data_path (struct GMT_CTRL *GMT, char *track_path, char *track, char *suffix);
EXTERN_MSC int x2sys_err_pass (struct GMT_CTRL *GMT, int err, char *file);
EXTERN_MSC int x2sys_err_fail (struct GMT_CTRL *GMT, int err, char *file);
EXTERN_MSC const char * x2sys_strerror (struct GMT_CTRL *GMT, int err);

EXTERN_MSC uint64_t x2sys_read_coe_dbase (struct GMT_CTRL *GMT, struct X2SYS_INFO *s, char *dbase, char *ignorefile, double *wesn, char *fflag, int coe_kind, char *one_trk, struct X2SYS_COE_PAIR **xpairs, uint64_t *nx, uint64_t *ntracks);
EXTERN_MSC void x2sys_free_coe_dbase (struct GMT_CTRL *GMT, struct X2SYS_COE_PAIR *P, uint64_t np);
EXTERN_MSC void x2sys_get_corrtable (struct GMT_CTRL *GMT, struct X2SYS_INFO *s, char *table, uint64_t ntracks, char **trk_name, char *column, struct MGD77_AUX_INFO *aux, struct MGD77_AUXLIST *auxlist, struct MGD77_CORRTABLE ***CORR);

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
#define X2SYS_BIX_BAD_ROW	-7
#define X2SYS_BIX_BAD_COL	-8
#define X2SYS_BIX_BAD_INDEX	-9

#define X2SYS_DIST_SELECTION	0
#define X2SYS_SPEED_SELECTION	1

#define X2SYS_bit(bit) (1U << (bit))	/* Set the specified bit to 1.  1,2,4,8 etc for 0,1,2,3... */
