/*
 *	$Id: shore.h,v 1.6 2011-03-15 02:06:37 guru Exp $
 */
#ifndef _WVS_H_
#define MILL (1000000)
#define GSHHS_MAX_DELTA 65535				/* Largest value to store in a ushort, used as largest dx or dy in bin  */
#define M360 360000000
#define M180 180000000
#define M90   90000000

typedef unsigned short ushort;
#endif

struct SEGMENT {
	int GSHHS_ID;		/* Original GSHHS ID for the polygon that this segment is part of */
	int GSHHS_parent;	/* ID of polygon that the GSHHS_id polygon is contained by (-1 if level == 1) */
	int level;
	int n;
	int entry, exit;		/* 4 for a poly completely inside bin; else w,e,s,n = 3,1,0,2  */
	int p_area;			/* Area of polygon this segment belongs to (in 0.1 * km^2) */
	int p_area_fraction;		/* Percent of full-resolution area for this resolution [100 for full] */
	struct SEGMENT *next_seg;
	struct SHORT_PAIR *p;		/* A chain of x,y points is tacked on here  */
};

struct SHORT_PAIR {
	ushort	dx;	/* Relative distance from SW corner of bin, units of B_WIDTH/GSHHS_MAX_DELTA  */
	ushort	dy;
};

struct SEGMENT_HEADER {
	int GSHHS_ID;		/* Original GSHHS ID for the polygon that this segment is part of */
	int GSHHS_parent;	/* ID of polygon that the GSHHS_id polygon is contained by (-1 if level == 1) */
	int info;		/* Combination of n, entry, exit, level */
	int p_area;		/* Area of polygon from which this segment belongs */
	int p_area_fraction;	/* Percent of full-resolution area for this resolution [100 for full] */
	int first_p;		/* Id of first point */
};

struct GMT3_FILE_HEADER {
	int n_bins;		/* Number of blocks */
	int n_points;		/* Total number of points */
	int bsize;		/* Bin size in minutes */
	int nx_bins;		/* # of bins in 0-360 */
	int ny_bins;		/* # of bins in -90 - +90 */
	int n_segments;		/* Total number of segments */
};	

struct GMT3_BIN_HEADER {
	int first_seg_id;
	short n_segments;
	short node_levels;	/* Stores the level of the four corner */
};	

