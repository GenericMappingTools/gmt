/*	$Id$
 *
 *	Code donated by David Divens, NOAA/NGDC
 *	Distributed under the GNU Public License (see LICENSE.TXT for details)
 */
#ifndef _H_MGG_HEADER_2
#define _H_MGG_HEADER_2

enum GMT_enum_grd98 {GRD98_MAGIC_NUM = 1000000000,
	GRD98_VERSION		= 1,
	GRD98_NAN_VALUE		= 999999,
	GRD98_DEFAULT_PREC	= 10,
	GRD98_N_UNUSED		= 10};

typedef struct {
	int	version;		/* 1,000,000,001 Magic_Num + Version */
	int	length;			/* 128 bytes */
	int     dataType;       /* 1=data, 2=density, 3=radius, -1=template */
	int	latDeg;
	int	latMin;
	int	latSec;
	int	latSpacing;
	int	latNumCells;
	int	lonDeg;
	int	lonMin;
	int	lonSec;
	int	lonSpacing;
	int	lonNumCells;
	int	minValue;		/* Whole meters */
	int	maxValue;		/* Whole meters */
	int	gridRadius;		/* -1 for grid radius not performed */
	int	precision;     	 /* 1 = whole meters, 10 = tenths of meters */
	int	nanValue;
	int     numType;        /* bytesize, pos=int, neg=float */
	int     waterDatum;     /* Vertical datum 0 = Mean Sea Level, 1 = local */
	int	dataLimit;		/* 2-byte, 4-byte -1 = float */
	int	cellRegistration;	/* 1 for pixel (cell centered), 0 for gridline */
	int    unused[GRD98_N_UNUSED];	/* Unused 4byte ints */
} MGG_GRID_HEADER_2;

#endif
