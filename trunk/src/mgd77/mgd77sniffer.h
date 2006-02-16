/* -------------------------------------------------------------------
 *	$Id: mgd77sniffer.h,v 1.2 2006-02-16 12:00:10 pwessel Exp $	
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
 #include <time.h>
 
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


/* Error Display Codes */
#define TYPE_WARN            0
#define TIME_WARN            1
#define DISTANCE_WARN        2
#define SPEED_WARN           3
#define VALUE_WARN           4
#define SLOPE_WARN           5
#define GRID_WARN            6
#define SUMMARY_WARN         7
#define MGD77_N_WARN_TYPES   8

/* Error classes */
#define C_TIME               0
#define C_NAV                1
#define C_OE                 2
#define C_RANGE              3
#define C_SLOPE              4
#define C_GRID               5
#define N_ERROR_CLASSES      6
#define N_DEFAULT_TYPES      MGD77_N_DATA_FIELDS

/* Time error types */
#define TIME_DUPL            1          /* A */
#define TIME_DECR            2          /* B */
#define TIME_UNSP            4          /* C */
#define TIME_OOR             8          /* D */
#define N_TIME_TYPES         4

/* Nav error types */
#define NAV_HISPD            1          /* A */
#define NAV_NEGSPD           2          /* B */
#define NAV_ON_LAND          4          /* C */
#define NAV_LAT_UNDEF        8          /* D */
#define NAV_LON_UNDEF       16          /* E */
#define N_NAV_TYPES          5

struct MGD77_GRID_INFO {
	struct GRD_HEADER grdhdr;
	struct GMT_EDGEINFO edgeinfo;
	struct GMT_BCR bcr;
	int one_or_zero, nx, ny, col, sign, g_pts;
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

EXTERN_MSC struct MGD77_SNIFFER_DEFAULTS mgd77snifferdefs[MGD77_N_DATA_FIELDS];
EXTERN_MSC void read_grid (struct MGD77_GRID_INFO *info, float **grid);
EXTERN_MSC int sample_grid (struct MGD77_GRID_INFO *info, struct MGD77_DATA_RECORD *D, double **g, float *grid, int n_grid, int n);
EXTERN_MSC void regress_ls (double *x, double *y, int n, double *stat, int col);
EXTERN_MSC void regress_rls (double *x, double *y, int nvalues, double *stat, int col);
EXTERN_MSC void regress_lms (double *x, double *y, int nvalues, double *stat, int gridField);
EXTERN_MSC void regresslms_sub (double *x, double *y, double angle0, double angle1, int nvalues, int n_angle, double *stat, int gridField);
EXTERN_MSC double lms (double *x, int n);
EXTERN_MSC double median (double *x, int n);
