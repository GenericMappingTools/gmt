/*--------------------------------------------------------------------
 *	$Id: gmt_mgg.h,v 1.7 2008-03-22 11:55:36 guru Exp $
 *
 *    Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

/* Requires gmt.h to already have been included */

#define GMTMGG_NODATA (-32000)	/* .gmt file NaN proxy */
#define MDEG2DEG	0.000001	/* Convert millidegrees to degrees */
#define NGDC_OLDEST_YY	39	/* Oldest NGDC cruise is from 1939 */

#define GMTMGG_TIME_MAXMONTH	61	/* 5 years is a long time for one cruise */

struct GMTMGG_TIME {
  GMT_LONG daymon[GMTMGG_TIME_MAXMONTH];	/* Cumulative number of days up to last month */
  GMT_LONG first_year;			/* The year the cruise started */
};

struct GMTMGG_REC {	/* Format of *.gmt file records */
	GMT_LONG time;
	GMT_LONG lat;
	GMT_LONG lon;
	short int gmt[3];
};

EXTERN_MSC GMT_LONG gmtmgg_date (GMT_LONG time, GMT_LONG *year, GMT_LONG *month, GMT_LONG *day, GMT_LONG *hour, GMT_LONG *minute, GMT_LONG *second, struct GMTMGG_TIME *gmt_struct);
EXTERN_MSC struct GMTMGG_TIME *gmtmgg_init (GMT_LONG year1);
EXTERN_MSC GMT_LONG gmtmgg_time (GMT_LONG *time, GMT_LONG year, GMT_LONG month, GMT_LONG day, GMT_LONG hour, GMT_LONG minute, GMT_LONG second, struct GMTMGG_TIME *gmt_struct);
EXTERN_MSC void gmtmggpath_init (char *dir);
EXTERN_MSC GMT_LONG gmtmggpath_func (char *leg_path, char *leg);
EXTERN_MSC GMT_LONG gmtmgg_decode_MGD77 (char *string, GMT_LONG tflag, struct GMTMGG_REC *record, struct GMTMGG_TIME **gmt_struct);
EXTERN_MSC char *MGG_SHAREDIR;	/* Copies GMT_SHAREDIR */
