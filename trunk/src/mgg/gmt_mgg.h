/*--------------------------------------------------------------------
 *	$Id: gmt_mgg.h,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
 *
 *    Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

/* Requires gmt.h to already have been included */

#define GMTMGG_NODATA (-32000)	/* .gmt file NaN proxy */
#define MDEG2DEG	0.000001	/* Convert millidegrees to degrees */
#define NGDC_OLDEST_YY	39	/* Oldest NGDC cruise is from 1939 */

struct GMTMGG_TIME {
  int daymon[61];	/* Cumulative number of days up to last month */
  int first_year;	/* The year the cruise started */
};

struct GMTMGG_REC {	/* Format of *.gmt file records */
	int time;
	int lat;
	int lon;
	short int gmt[3];
};

EXTERN_MSC int gmtmgg_date (int time, int *year, int *month, int *day, int *hour, int *minute, int *second, struct GMTMGG_TIME *gmt_struct);
EXTERN_MSC struct GMTMGG_TIME *gmtmgg_init (int year1);
EXTERN_MSC int gmtmgg_time (int *time, int year, int month, int day, int hour, int minute, int second, struct GMTMGG_TIME *gmt_struct);
EXTERN_MSC void gmtmggpath_init (void);
EXTERN_MSC int gmtmggpath_func (char *leg_path, char *leg);
EXTERN_MSC int gmtmgg_decode_MGD77 (char *string, int tflag, struct GMTMGG_REC *record, struct GMTMGG_TIME **gmt_struct);
