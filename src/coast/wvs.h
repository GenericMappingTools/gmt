/*
 *	$Id: wvs.h,v 1.7 2009-06-09 02:26:02 guru Exp $
 */
/* wvs.h
 *
 * Here are some things we use in dealing with the wvs data.
 */

#include "gmt.h"
#define _GMT_

#define WVSDIR "/Users/pwessel/GMTdev/GMT/src/coast"

#define N_LONGEST     1450000         /* 1435084 to be exact */
#define N_POLY		205000		/* 188281 to be exact */
#define M360 360000000
#define M180 180000000
#define M90   90000000
#define MILL   1000000

#define N_CONTINENTS	6

#define EURASIA		1
#define AFRICA		2
#define NAMERICA	3
#define SAMERICA	4
#define ANTARCTICA	5
#define AUSTRALIA	6

#define OUTSIDE		0
#define INSIDE		1

#define N_EUR_O	43	/* # of points in various polygons */
#define N_EUR_I	59
#define N_AFR_I	15
#define N_AM_O	40
#define N_SAM_I	20
#define N_NAM_I	33
#define N_AUS_O	9
#define N_AUS_I	15

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

struct GMT3_POLY {
	int id;
	int n;
	int greenwich;  /* Greenwich is TRUE if Greenwich is crossed */
	int level;      /* -1 undecided, 0 ocean, 1 land, 2 lake, 3 island_in_lake, etc */
	int datelon;    /* 180 for all except eurasia (270) */
	int checked[2]; /* TRUE if polygon has been crossover checked with all peers */
	int source;     /* 0 = CIA WDBII, 1 = WVS */
	int parent;     /* -1 if top level 1, else id of polygon containing this polygon */
	int river;     /* 1 if this is level2 and river-lake, also contains cont# << 8 */
	double west, east, south, north;
	double area;    /* Area of polygon */
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
void crude_init (double *X[N_CONTINENTS][2], double *Y[N_CONTINENTS][2], int N[N_CONTINENTS][2]);
void crude_free (double *X[N_CONTINENTS][2], double *Y[N_CONTINENTS][2], int N[N_CONTINENTS][2]);
void crude_init_int (int *IX[N_CONTINENTS][2], int *IY[N_CONTINENTS][2], int N[N_CONTINENTS][2], int scale);
void crude_free_int (int *IX[N_CONTINENTS][2], int *IY[N_CONTINENTS][2], int N[N_CONTINENTS][2]);
int nothing_in_common (struct GMT3_POLY *hi, struct GMT3_POLY *hj, double *shift);

#ifndef COASTLIB
#define EUR_O_MIN_X (340 * MILL)
#define AM_O_MIN_X  (190 * MILL)
#define AUS_O_MIN_X (110 * MILL)
int ieur_o[2][N_EUR_O] = {
{340,
340,
347,
350,
360,
360,
370,
400,
410,
420,
430,
440,
465,
505,
555,
525,
520,
505,
490,
483,
480,
470,
470,
465,
465,
463,
460,
457,
458,
457,
453,
451,
449,
440,
437,
426,
422,
398,
383,
374,
366,
349,
340},
{10,
20,
30,
50,
50,
60,
70,
78,
70,
70,
75,
75,
78,
78,
66,
57,
50,
50,
35,
30,
24,
20,
10,
4,
1,
1,
3,
7,
12,
15,
15,
20,
20,
9,
7,
24,
24,
-23,
-45,
-32,
3,
3,
10}
};
int ieur_i[2][N_EUR_I] = {
{465,
464,
472,
480,
477,
487,
494,
494,
500,
510,
520,
527,
527,
531,
537,
520,
490,
480,
464,
468,
449,
445,
428,
426,
405,
394,
390,
380,
369,
374,
378,
382,
385,
392,
386,
381,
365,
360,
374,
383,
390,
402,
404,
390,
390,
398,
398,
405,
418,
406,
406,
410,
413,
432,
437,
444,
454,
460,
465},
{11,
23,
23,
28,
40,
44,
44,
56,
60,
60,
63,
63,
61,
61,
68,
68,
70,
72,
72,
76,
75,
68,
64,
68,
66,
62,
69,
68,
62,
58,
64,
67,
67,
60,
58,
54,
51,
43,
48,
43,
48,
48,
40,
40,
38,
37,
30,
15,
22,
25,
32,
32,
28,
26,
13,
24,
24,
15,
11}
};
int iafr_i[2][N_AFR_I] = {
{380,
387,
393,
399,
397,
410,
402,
389,
380,
364,
352,
345,
349,
371,
380},
{-33,
-33,
-20,
-14,
-3,
10,
10,
30,
28,
35,
32,
17,
9,
9,
-33}
};
int iam_o[2][N_AM_O] = {
{286,
292,
335,
300,
290,
280,
273,
281,
281,
300,
300,
309,
295,
290,
285,
280,
280,
282,
280,
270,
280,
280,
264,
260,
240,
230,
190,
190,
200,
225,
227,
231,
232,
233,
235,
236,
234,
234,
272,
286},
{-54,
-54,
-10,
10,
15,
10,
24,
24,
32,
45,
49,
53,
62,
62,
63,
62,
59,
57,
55,
60,
67,
70,
73,
70,
70,
73,
73,
57,
53,
58,
56,
53,
51,
50,
50,
49,
48,
30,
6,
-54}
};
int isam_i[2][N_SAM_I] = {
{289,
294,
300,
310,
310,
319,
320,
322,
320,
308,
307,
300,
298,
291,
284,
284,
281,
286,
292,
289},
{-51,
-40,
-33,
-29,
-25,
-20,
-10,
-8,
-6,
-2,
3,
6,
9,
9,
8,
3,
-5,
-15,
-17,
-51}
};
int inam_i[2][N_NAM_I] = {
{262,
260,
278,
281,
290,
286,
290,
299,
295,
288,
288,
283,
284,
281,
278,
276,
263,
265,
251,
240,
230,
220,
218,
200,
202,
198,
200,
210,
225,
239,
239,
260,
262},
{18,
30,
32,
40,
45,
47,
50,
51,
57,
58,
61,
62,
54,
50,
50,
54,
57,
67,
66,
68,
69,
67,
69,
70,
66,
62,
60,
63,
61,
51,
42,
18,
18}
};
int iaus_o[2][N_AUS_O] = {
{155,
155,
145,
130,
110,
110,
131,
138,
155},
{-40,
-24,
-9,
-9,
-20,
-36,
-36,
-40,
-40}
};
int iaus_i[2][N_AUS_I] = {
{115,
117,
130,
138,
139,
143,
148,
151,
150,
144,
140,
133,
126,
122,
115},
{-22,
-33,
-31,
-32,
-34,
-38,
-36,
-30,
-24,
-17,
-19,
-15,
-16,
-20,
-22}
};
#endif
