/*	$Id: gmt_mgg_header2.h,v 1.1 2004-01-12 00:37:22 pwessel Exp $	*/

#ifndef _H_MGG_HEADER_2
#define _H_MGG_HEADER_2

#define MGG_MAGIC_NUM	1000000000
#define VERSION			1
#define MGG_NAN_VALUE	999999
#define DEFAULT_PREC	10

typedef struct {
	long	version;		/* 1,000,000,001 Magic_Num + Version */
	long	length;			/* 128 bytes */
    long    dataType;       /* 1=data, 2=density, 3=radius, -1=template */
	long	latDeg;
	long	latMin;
	long	latSec;
	long	latSpacing;
	long	latNumCells;
	long	lonDeg;
	long	lonMin;
	long	lonSec;
	long	lonSpacing;
	long	lonNumCells;
	long	minValue;		/* Whole meters */
	long	maxValue;		/* Whole meters */
	long	gridRadius;		/* -1 for grid radius not performed */
	long	precision;      /* 1 = whole meters, 10 = tenths of meters */
	long	nanValue;
    long    numType;        /* bytesize, pos=int, neg=float */
    long    waterDatum;     /* Vertical datum 0 = Mean Sea Level, 1 = local */
	long	dataLimit;		/* 2-byte, 4-byte -1 = float */
	long    unused1;
	long    unused2;
	long    unused3;
	long    unused4;
	long    unused5;
	long    unused6;
	long    unused7;
	long    unused8;
	long    unused9;
	long    unused10;
	long    unused11;
} MGG_GRID_HEADER_2;

int mgg2_read_grd_info(char *file, struct GRD_HEADER *header);
int mgg2_write_grd_info(char *file, struct GRD_HEADER *header);
int mgg2_read_grd(char *file, struct GRD_HEADER *header, float *grid, 
	double w, double e, double s, double n, int *pad, BOOLEAN complex);
	
int mgg2_write_grd(char *file, struct GRD_HEADER *header, float *grid,
	double w, double e, double s, double n, int *pad, BOOLEAN complex);

#endif
