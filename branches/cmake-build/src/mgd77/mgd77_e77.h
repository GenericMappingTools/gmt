/* -------------------------------------------------------------------
 *	$Id$	
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *    Copyright (c) 2004-2011 by P. Wessel and M. T. Chandler
 *	File:	mgd77_e77.h
 *
 *	Include file for mgd77 programs
 *
 *	Authors:
 *		Michael Chandler and Paul Wessel
 *		School of Ocean and Earth Science and Technology
 *		University of Hawaii
 * 
 *	Date:	8-June-2007
 * 
 * ------------------------------------------------------------------*/
 
 /* E77 Error classes */
#define E77_NAV                0
#define E77_VALUE              1
#define E77_SLOPE              2
#define N_ERROR_CLASSES        3
#define N_DEFAULT_TYPES      MGD77_N_NUMBER_FIELDS

/* E77 Nav Error Types */
#define NAV_TIME_OOR         1          /* A - Time out of range */
#define NAV_TIME_NONINC      2          /* B - Non-increasing time */
#define NAV_HISPD            4          /* C - Excessive speed */
#define NAV_ON_LAND          8          /* D */
#define NAV_UNDEF           16          /* E */
#define NAV_TZ_ERROR        32          /* F - Time zone error */
#define N_NAV_TYPES          6

/* E77 Header Errata Codes */
#define E77_HDR_NAV          0
#define E77_HDR_RLS          1
#define E77_HDR_SCALE        2
#define E77_HDR_DCSHIFT      3
#define E77_HDR_RMS          4
#define E77_HDR_CORR         5
#define E77_HDR_ANOM_FAA     6
#define E77_HDR_ANOM_MAG     7
#define E77_HDR_GRID_OFFSET  8
#define E77_HDR_SIGN         9
#define E77_HDR_FLAGRANGE    10
#define E77_HDR_BCC          11
#define E77_HDR_PRECISION    12
#define E77_HDR_PDR          13
#define E77_HDR_CARTER       14
#define E77_HDR_ANOM_FAA_EOT 15
#define E77_HDR_ANOM_FAA_IGF 16

/* E77 Default Warning Codes */
#define E77_ERROR           'E'
#define E77_WARN            'W'
#define E77_INFO			'I'
#define E77_APPLY           'Y'
#define E77_REJECT          'N'
#define E77_REVIEW          '?'
