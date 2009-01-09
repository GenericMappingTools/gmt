/*--------------------------------------------------------------------
 *	$Id: gmt_mgg.h,v 1.10 2009-01-09 04:02:35 guru Exp $
 *
 *    Copyright (c) 1991-2009 by P. Wessel and W. H. F. Smith
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

/* Requires gmt.h to already have been included */

#define GMTMGG_NODATA (-32000)	/* .gmt file NaN proxy */
#define MDEG2DEG	0.000001	/* Convert millidegrees to degrees */
#define NGDC_OLDEST_YY	39	/* Oldest NGDC cruise is from 1939 */

#define GMTMGG_TIME_MAXMONTH	61	/* 5 years is a long time for one cruise */

struct GMTMGG_TIME {
  int daymon[GMTMGG_TIME_MAXMONTH];	/* Cumulative number of days up to last month */
  int first_year;			/* The year the cruise started */
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
EXTERN_MSC void gmtmggpath_init (char *dir);
EXTERN_MSC int gmtmggpath_func (char *leg_path, char *leg);
EXTERN_MSC int gmtmgg_decode_MGD77 (char *string, int tflag, struct GMTMGG_REC *record, struct GMTMGG_TIME **gmt_struct);
EXTERN_MSC void gmtmgg_end ();

EXTERN_MSC char *MGG_SHAREDIR;	/* Copies GMT_SHAREDIR */
