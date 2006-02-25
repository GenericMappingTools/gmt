/* -------------------------------------------------------------------
 *	$Id: mgd77sniffer.h,v 1.7 2006-02-25 02:17:32 pwessel Exp $	
 *
 *	File:	mgd77sniffer.h
 *
 *	Include file for mgd77sniffer
 *
 *	Authors:
 *		Michael Chandler and Paul Wessel
 *		School of Ocean and Earth Science and Technology
 *		University of Hawaii
 * 
 *	Date:	23-Feb-2004
 * 
 * ------------------------------------------------------------------*/
 
 #include "gmt.h"
 #include "mgd77.h"
 
/* Constants */
#define MGD77_N_DATA_FIELDS         27
#define MGD77_NM_PER_DEGREE         60
#define MGD77_METERS_PER_NM         1852
#define MGD77_DEG_TO_KM             (6371.0087714 * D2R)
#define MGD77_NAV_PRECISION_KM		0.00111               /* km */
#define MGD77_MAX_SPEED             10                    /* m/s */
#define MGD77_MEDIAN_SPEED          4.7                   /* m/s */
#define MGD77_MAX_DT                900                   /* sec */
#define MGD77_MAX_DS                5                     /* km */
#define MGD77_MAX_DUPLICATES        50
#define MGD77_FATHOMS_PER_METER     0.546805453181423
#define MGD77_MIN_RLS_BINS          20
#define MGD77_MIN_RLS_PTS           100
#define MGD77_N_MAG_RF              13
#define MGD77_MAX_SEARCH            50

/* LMS Limits - Obtained from 4616 bathy and 1657 gravity cruises */
#define MGD77_MIN_DEPTH_SLOPE       0.900404              /* Q2.5 */
#define MGD77_MAX_DEPTH_SLOPE       1.1149               /* Q97.5 */
#define MGD77_MAX_DEPTH_INTERCEPT   0
#define MGD77_MAX_DEPTH_MISFIT      140.676                /* Q95 */
#define MGD77_MIN_FAA_SLOPE         0.868674     /*Q25(Q2.5=-0.17)*/
#define MGD77_MAX_FAA_SLOPE         1.06862              /* Q97.5 */
#define MGD77_MAX_FAA_INTERCEPT     19.1557                /* Q95 */
#define MGD77_MAX_FAA_MISFIT        7.98608                /* Q95 */

/* Verbose Error Display Codes */
#define TYPE_WARN            0
#define TIME_WARN            1
#define DISTANCE_WARN        2
#define SPEED_WARN           3
#define VALUE_WARN           4
#define SLOPE_WARN           5
#define GRID_WARN            6
#define SUMMARY_WARN         7
#define MGD77_N_WARN_TYPES   8

/* E77 Error classes */
#define E77_NAV                0
#define E77_VALUE              1
#define E77_SLOPE              2
#define E77_GRID               3
#define N_ERROR_CLASSES        4
#define N_DEFAULT_TYPES      MGD77_N_NUMBER_FIELDS

/* E77 Nav Error Types */
#define NAV_TIME_OOR         1          /* A */
#define NAV_TIME_DECR        2          /* B */
#define NAV_HISPD            4          /* C */
#define NAV_ON_LAND          8          /* D */
#define NAV_LAT_UNDEF       16          /* E */
#define NAV_LON_UNDEF       32          /* F */
#define N_NAV_TYPES          6

/* E77 Header Errata Codes */
#define E77_HDR_SCALE        1
#define E77_HDR_OFFSET       2
#define E77_HDR_BCC          3
#define E77_HDR_PRECISION    4

/*  MGD77 bit-pattern w/ E77 alpha characters
|-------------------------------------------------|----------|
| X W V U T S R Q P O N M L K J I H G F E D C B A | E77 Code |
| - - - - - - - - - - - - - - - - - - - - - - - - | - - - - -|
| n f e g m d m m m m b b d t p l l m h d m y t d | F  I     |
| q a o o s i s a t t t c e w t o a i o a o e z r | i  D     |
| c a t b d u e g f f c c p t c n t n u y n a   t | e        |
|       s   r n   2 1     t           r   t r     | l        |
|             s           h               h       | d        |
| - - - - - - - - - - - - - - - - - - - - - - - - | - - - - -|
| 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 | Bit place|
| - G C G C C - G G G - - G G - - - T T T T T - - | Bit type |
|-------------------------------------------------|----------|
  Bit types: (G)eophysical, (C)orrection, (T)ime */

struct MGD77_GRID_INFO {
	struct GRD_HEADER grdhdr;
	struct GMT_EDGEINFO edgeinfo;
	struct GMT_BCR bcr;
	int one_or_zero, nx, ny, col, sign, g_pts, format, mode, mx, interpolate;
	double scale, max_lat;
	char abbrev[8];
	char fname[32];
};

struct MGD77_SNIFFER_DEFAULTS {
	char abbrev[8];      /* MGD77 field name abbreviations  */
	double minValue;     /* minimum accepted field value    */
	double maxValue;     /* maximum accepted field value    */
	double delta;        /* RLS decimation binsize interval */
	double maxTimeGrad;  /* maximum +/- time derivative     */
	double maxSpaceGrad; /* maximum +/- space derivative    */
	double maxDiff;      /* maximum +/- value change        */
	double maxArea;      /* maximum grid offset area        */
};

struct MGD77_ERROR {
	unsigned int flags[6];
};

struct MGD77_MAG_RF {
	char *model;        /* Reference field model name */
	int code;           /* Reference field code       */
	int start;          /* Model start year           */
	int end;            /* Model end year             */
};

EXTERN_MSC struct MGD77_SNIFFER_DEFAULTS mgd77snifferdefs[MGD77_N_DATA_FIELDS];
EXTERN_MSC void read_grid (struct MGD77_GRID_INFO *info, float **grid, double w, double e, double s, double n, BOOLEAN bilinear, double threshold);
EXTERN_MSC int sample_grid (struct MGD77_GRID_INFO *info, struct MGD77_DATA_RECORD *D, double **g, float *grid, int n_grid, int n);
EXTERN_MSC void regress_ls (double *x, double *y, int n, double *stat, int col, double S_xx);
EXTERN_MSC void regress_rls (double *x, double *y, int nvalues, double *stat, int col, double S_xx);
EXTERN_MSC void regress_lms (double *x, double *y, int nvalues, double *stat, int gridField);
EXTERN_MSC void regresslms_sub (double *x, double *y, double angle0, double angle1, int nvalues, int n_angle, double *stat, int gridField);
EXTERN_MSC double lms (double *x, int n);
EXTERN_MSC double median (double *x, int n);
