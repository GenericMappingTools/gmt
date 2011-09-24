/*
 *	$Id$
 */
/* wvs.h
 *
 * Here are some things we use in dealing with the wvs data.
 */

#include "gmt.h"
#define _GMT_
#define _WVS_H_
#define WVSDIR "/Users/pwessel/GMTdev/GMT/src/coast"

#define N_LONGEST     1450000         /* 1435084 to be exact */
#define N_POLY		205000		/* 188281 to be exact */
#define M360 360000000
#define M180 180000000
#define M90   90000000
#define MILL   1000000
#define I_MILL   1e-6

#define N_CONTINENTS	6

#define EURASIA		1
#define AFRICA		2
#define NAMERICA	3
#define SAMERICA	4
#define ANTARCTICA	5
#define AUSTRALIA	6

#define OUTSIDE		0
#define INSIDE		1

#define WVS_continent(h) (h.river >> 8);	/* Get continent number 1-6 (or 0 if not a continent) */

typedef unsigned short ushort;

struct	LONGPAIR {
	int	x;
	int	y;
};

struct	FLAGPAIR {	/* Used for checking strings  */
	int	x;
	int	y;
	int	k;
};

struct	RAWSEG_HEADER {	/* These and LONGPAIRs are written by read_wvs  */
	int	n;
	int	rank;
};

struct	SEG_HEADER {
	int	id;
	int	rank;
	int	n;
	struct LONGPAIR	first;	/* First point in string  */
	struct LONGPAIR	last;	/* Last point in string  */
};

/* Note: Sibling number means which full resolution polygon matches a DP-reduced polygon in the h-i-l-c datasets.
  For res = f the sibling number is not set, obviously. */

struct GMT3_POLY {
	int id;
	int n;
	int greenwich;	/* (greenwich & 1) is TRUE if Greenwich is crossed */
	int level;      /* -1 undecided, 0 ocean, 1 land, 2 lake, 3 island_in_lake, etc */
	int datelon;    /* 180 for all except eurasia (270) */
	int source;     /* 0 = CIA WDBII, 1 = WVS */
	int parent;     /* -1 if top level 1, else id of polygon containing this polygon */
	int ancestor;	/* The sibling id for lower resolutions */
	int river;	/* 1 if this is level2 and river-lake, also contains cont# << 8 */
	double west, east, south, north;	/* Bounding box */
	double area;		/* Area of polygon measured at full resolution */
	double area_res;	/* Area of polygon at the present resololution */
};

extern int pol_readheader (struct GMT3_POLY *h, FILE *fp);
extern int pol_writeheader (struct GMT3_POLY *h, FILE *fp);
extern int pol_fread (struct LONGPAIR *p, size_t n_items, FILE *fp);
extern int pol_fwrite (struct LONGPAIR *p, size_t n_items, FILE *fp);
extern int pol_readheader2 (struct GMT3_POLY *h, FILE *fp);
extern int pol_writeheader2 (struct GMT3_POLY *h, FILE *fp);
extern int pol_fread2 (struct LONGPAIR *p, size_t n_items, FILE *fp);
extern int pol_fwrite2 (struct LONGPAIR *p, size_t n_items, FILE *fp);
extern double area_size (double x[], double y[], int n, int *sign);
extern int non_zero_winding2 (int xp, int yp, int *x, int *y, int n_path);
extern void area_init ();
extern int Douglas_Peucker (double x_source[], double y_source[], int n_source, double band, int index[]);
extern int Douglas_Peucker_i (int x_source[], int y_source[], int n_source, double band, int index[]);
extern void crude_init (double *X[N_CONTINENTS][2], double *Y[N_CONTINENTS][2], int N[N_CONTINENTS][2]);
extern void crude_free (double *X[N_CONTINENTS][2], double *Y[N_CONTINENTS][2], int N[N_CONTINENTS][2]);
extern void crude_init_int (int *IX[N_CONTINENTS][2], int *IY[N_CONTINENTS][2], int N[N_CONTINENTS][2], int scale);
extern void crude_free_int (int *IX[N_CONTINENTS][2], int *IY[N_CONTINENTS][2], int N[N_CONTINENTS][2]);
extern int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY *hj, double *shift);
extern void xy2rtheta (double *lon, double *lat);
extern void xy2rtheta_int (int *ilon, int *ilat);
extern void rtheta2xy (double *lon, double *lat);
