/*	$Id: gmt_mgg_header2.h,v 1.2 2004-09-28 19:24:13 pwessel Exp $	*/

#ifndef _H_MGG_HEADER_2
#define _H_MGG_HEADER_2

#define MGG_MAGIC_NUM	1000000000
#define VERSION			1
#define MGG_NAN_VALUE	999999
#define DEFAULT_PREC	10

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
	int	precision;      /* 1 = whole meters, 10 = tenths of meters */
	int	nanValue;
	int     numType;        /* bytesize, pos=int, neg=float */
	int     waterDatum;     /* Vertical datum 0 = Mean Sea Level, 1 = local */
	int	dataLimit;		/* 2-byte, 4-byte -1 = float */
	int    unused1;
	int    unused2;
	int    unused3;
	int    unused4;
	int    unused5;
	int    unused6;
	int    unused7;
	int    unused8;
	int    unused9;
	int    unused10;
	int    unused11;
} MGG_GRID_HEADER_2;

int mgg2_read_grd_info(char *file, struct GRD_HEADER *header);
int mgg2_write_grd_info(char *file, struct GRD_HEADER *header);
int mgg2_read_grd(char *file, struct GRD_HEADER *header, float *grid, 
	double w, double e, double s, double n, int *pad, BOOLEAN complex);
	
int mgg2_write_grd(char *file, struct GRD_HEADER *header, float *grid,
	double w, double e, double s, double n, int *pad, BOOLEAN complex);

#endif
