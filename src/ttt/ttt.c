/*
 * COPYRIGHT (c) 1993-2017 GEOWARE
 * 
 * ttt.c
 *
 * PROGRAM:	ttt.c
 * PURPOSE:	compute tsunami travel time files from bathymetry grids
 * AUTHOR:	Paul Wessel, GEOWARE
 * DATE:	June 16 1993
 * UPDATED:	Jan 1 2024
 * VERSION:	4.0
 *
 *
 * ttt calculates estimated tsunami travel times from an epicenter to all points
 * on a bathymetry grid.  The shallow-water wave approximation is used which means
 * that the propagation velocity is given by sqrt (depth * normal_gravity).  The
 * propagation is calculated on a grid using a very accurate 120-point Huygens con-
 * struction method that introduces a minimum of error.  In the case of constant
 * water depth the travel-time contours should be circular; here they are 120-sided
 * polygons which means the error is < +0.26 %, i.e., travel times can be up to 0.26 %
 * too long.  For a speedier solution the user may choose to use a coarser construction
 * which has more error but executes faster.  The max error for the various choices
 * of nodes are approximately 8.1 % (8), 2.76 % (16), 1.30 % (32), 0.73 % (48),
 * 0.49 % (64), and 0.26 % (120).  Note that the error is always >= 0 and thus is biased.
 * By default, ttt normalizes the travel time predictions so that the error
 * discussed above is distributed evenly (i.e., prediction error has ~0 mean).
 * This feature can be turned OFF with the -B option.
 * Note if ttt is compiled with the -DTTT120 preprocessor directive, then a 120-node
 * calculation is hardwired to save if-testing during run-time.
 *
 * The input bathymetry data must be in GMT short int binary native format.  To avoid
 * ambiguities in travel time construction it is best that landlocked bodies of
 * water be set to positive (topography) values or NaN (== 22767 for short int).
 * A warning message will be issued if landlocked bodies are found.
 *
 * Starting with version 3.0, ttt can process global grids and use geographical
 * boundary conditions.  Grids can now be in pixel- or gridline registration.
 * Starting with version 3.1, ttt can now be compiled in 64-bit mode and handle
 * huge grids.  See --enable-64 and --enable-large in the configure script.
 * Starting with version 3.2, ttt can now also write ESRI ASCII or float grids.
 *
 * LICENSE AGREEMENT:
 * ttt may be installed on one or more workstation in the same physical building or
 * office.  The source code (obfuscated for protection) MAY NOT be shared with others
 * and is only distributed to make compilation on a wide range of platforms possible.
 * ttt is proprietary software and subject to US Copyright laws.
 */

//#define TEST
//#define FLAT
#include "ttt.h"
#include "ttt_macro.h"
#include "ttt_subs.c"	/* Include common functions shared with ttt_pick */

#define T_LOCAL	0
#define T_UTC	1

#ifdef WORDS_BIGENDIAN
#define ENDIAN 1
#else
#define ENDIAN 0
#endif

/* ----------------------------   Functions used by ttt ------------------------------*/

/* Binary tree manipulation, modified after Sedgewick's Algorithms in C */

static void treeinitialize (struct TTT_INFO **H, struct TTT_INFO **T)
{	/* Initialize the binary tree to have a head (0 tt) and a tail (-1 tt) */
	struct TTT_INFO *list_head = NULL, *list_tail = NULL;
	list_tail = (struct TTT_INFO *) malloc(sizeof *list_tail);
	list_tail->l = list_tail;	list_tail->r = list_tail;	list_tail->tt = list_tail->ij = -1;
	list_head = (struct TTT_INFO *) malloc(sizeof *list_head);
	list_head->r = list_tail;	list_head->tt = 0;	list_head->ij = 0;
	*H = list_head;	*T = list_tail;
}

static void treeinsert (TTT_LONG tt, TTT_LONG ij, struct TTT_INFO *list_head, struct TTT_INFO *list_tail)
{	/* Insert a new node for the given travel time tt */
	struct TTT_INFO *p = list_head, *x = list_head->r;
	while (x != list_tail) {
		p = x;
		x = (tt < x->tt) ? x->l : x->r;
	}
	x = (struct TTT_INFO *) malloc(sizeof *x);
	x->tt = tt;	x->ij = ij;
	x->l = list_tail;	x->r = list_tail;
	if (tt < p->tt) p->l = x; else p->r = x;
}

static TTT_LONG treesmallest (struct TTT_INFO *list_head, struct TTT_INFO *list_tail, TTT_LONG *ij)
{	/* Return the smallest travel time in the list */
	struct TTT_INFO *x = list_head->r;
	while (x->l != list_tail) x = x->l;
	*ij = x->ij;
	return (x->tt);
}

static void treedelete (TTT_LONG tt, TTT_LONG ij, struct TTT_INFO *list_head, struct TTT_INFO *list_tail) {
	/* Find the tree node whose travel time and node ij matches the specified item and remove the tree node */
	struct TTT_INFO *c, *p, *t, *x;
	list_tail->tt = tt;	list_tail->ij = ij;
	p = list_head;	x = list_head->r;
	while (!(tt == x->tt && ij == x->ij)) {
		p = x;
		x = (tt < x->tt) ? x->l : x->r;
	}
	t = x;
	if (t->r == list_tail) x = x->l;
	else if (t->r->l == list_tail) {
		x = x->r;	x->l = t->l;
	}
	else {
		c = x->r;
		while (c->l->l != list_tail) c = c->l;
		x = c->l;	c->l = x->r;
		x->l = t->l;	x->r = t->r;
	}
	if (t == NULL)
		fprintf (stderr, "treedelete: Tried to free NULL pointer\n");
	else {
		if (t == list_tail)
			fprintf (stderr, "treedelete: Tried to free listhead pointer\n");
		else
			free (t);
	}
	if (tt < p->tt) p->l = x; else p->r = x;
}

/* Check if a source location is inside grid region */

static void ttt_check_if_inside (double *q_lon, double *q_lat, struct GRD_HEADER *h)
{
	TTT_LONG error = FALSE;
	
	if (*q_lat < h->wesn[YLO] || *q_lat > h->wesn[YHI]) error = TRUE;
	*q_lon -= 360.0;
	while (*q_lon < h->wesn[XLO]) *q_lon += 360.0;
	if (*q_lon > h->wesn[XHI]) error = TRUE;
	
	if (error) {
		fprintf (stderr, "ttt: Epicenter outside map area!\n");
		exit (19);
	}
}

/* Check if a source location is on land or over water.  If on land and search is TRUE it
   will relocate the source to the closest node over water */

static TTT_LONG ttt_check_source (float *s, struct GRD_HEADER *h, TTT_LONG i_quake, TTT_LONG j_quake, TTT_LONG ij_quake, double *q_lon, double *q_lat, TTT_LONG search_nearest, double search_radius, double search_depth, TTT_LONG verbose)
{
	BOOLEAN node_on_land, too_shallow = FALSE;
	TTT_LONG i, j, ij, row_width, imin, imax, jmin, jmax;
	double x, y, z, xx, yy, zz, dx, d, shortest_dist;

	ij = ij_quake;
	node_on_land = (TTT_is_fnan (s[ij]) || s[ij] >= 0.0);

	if (node_on_land && !search_nearest) {	/* Do not search, exit since quake is on land */

		fprintf (stderr, "\nttt: Error: Epicenter on land - exiting");
		exit (20);
	}
	
	/* For the purpose of this search, if the current node depth is shallower than what we specify we are effectively "on land" */

	z = s[ij];
	if (z > search_depth) too_shallow = node_on_land = TRUE;	/* Too shallow, set node_on_land as TRUE */
	
	if (!node_on_land) return 0;		/* We are in deep enough water, simply return */

	row_width = h->nx + 2 * N_PAD;

	/* Epicenter on land, must find nearest point */
	
	if (too_shallow)
		fprintf (stderr, "\nttt: Warning: Epicenter in too shallow water - searching for nearest node deeper than %g...", search_depth);
	else
		fprintf (stderr, "\nttt: Warning: Epicenter on land - searching for nearest substitute...");
		
	dx = h->inc[TTT_X] * cos (*q_lat * D2R);
	imin = MAX (0, i_quake - (TTT_LONG)ceil (search_radius / dx));
	imax = MIN (h->nx - 1, i_quake + (TTT_LONG)ceil (search_radius / dx));
	jmin = MAX (0, j_quake - (TTT_LONG)ceil (search_radius / h->inc[TTT_Y]));
	jmax = MIN (h->ny - 1, j_quake + (TTT_LONG)ceil (search_radius / h->inc[TTT_Y]));
		
	shortest_dist = 180.0;
	xx = yy = zz = 0.0;
	for (j = jmin; j <= jmax; j++) {
		y = ttt_j_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
		for (i = imin; i <= imax; i++) {
			ij = IJ(i, j, row_width, N_PAD);
			node_on_land = (TTT_is_fnan (s[ij]) || s[ij] >= 0.0);
			if (node_on_land) continue;
			z = s[ij];
			if (z >= search_depth) continue;	/* Too shallow */
			x = ttt_i_to_x (i, h->wesn[XLO], h->wesn[XHI], h->inc[TTT_X], h->xy_off, h->nx);
			d = ttt_great_circle_dist (*q_lon, *q_lat, x, y);
			if (d < shortest_dist) {
				/* ij_quake = ij; For debug purposes only */
				shortest_dist = d;
				xx = x;	yy = y;	zz = z;
			}
		}
	}
		
	if (shortest_dist == 180.0) {	
		if (search_depth < 0.0)	
			fprintf (stderr, "none found within %g degrees and deeper than %g - exiting\n", search_radius, search_depth);
		else
			fprintf (stderr, "none found within %g degrees - exiting\n", search_radius);
		exit (21);
	}
	else {
		*q_lon = xx;
		*q_lat = yy;
		if (search_depth < 0.0) zz = search_depth;
		fprintf (stderr, "using %g/%g (water depth = %g)\n", xx, yy, zz);
		if (verbose == 2) printf ("%g\t%g\t%g\n", xx, yy, zz);
	}
	return (1);
}

/* Set up relative node offsets from current node to neighbors */

static TTT_LONG ttt_init_offset (TTT_LONG *ip, TTT_LONG *jp, TTT_LONG *p, struct GRD_HEADER *h, TTT_LONG n_nodes)
{
	TTT_LONG row_width, need_to_use, i;
	static TTT_LONG I[M_ACCESS] = {	/* These two include files are created by ttt_angle_analysis.sh */
#include "ttt_i.h"
	};
	static TTT_LONG J[M_ACCESS] = 	{
#include "ttt_j.h"
	};   
	
	/* Nodes are numbered as below (X marks center (current) node;  We only calculate
	 * travel time to the nodes 0-119.  Nodes 120-255 are used to estimate slowness.  The
	 * travel time from X to any of the 120-255 nodes goes through intermediate points.
	 *
	 *
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	119	 o	116	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	255	246	252	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	243	234	240	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	231	222	228	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	219	210	216	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	103	202	100	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	111	 o	 o	095	 o	191	178	188	 o	092	 o	 o	108	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	107	207	199	 o	175	167	079	158	076	164	172	 o	196	204	104	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	195	183	087	071	063	055	150	052	060	068	084	180	192	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	083	155	047	147	039	134	036	144	044	152	080	 o	 o	 o	 o	 o	 o	 o	 o
	 *
	 *	 o	 o	 o	 o	 o	 o	 o	091	171	067	043	139	023	130	020	028	136	040	064	168	088	 o	 o	 o	 o	 o	 o
	 *
	 *	 o	 o	 o	 o	 o	 o	 o	163	059	143	027	127	015	122	012	124	024	140	056	160	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	115	251	239	227	215	099	187	075	051	035	019	011	007	002	004	008	016	032	048	072	184	096	212	224	236	248	112
	 *	
	 *	 o	245	233	221	209	201	177	157	149	133	129	121	001	 X	000	120	128	132	148	156	176	200	208	220	232	244	 o
	 *	
	 *	113	249	237	225	213	097	185	073	049	033	017	009	005	003	006	010	018	034	050	074	186	098	214	226	238	250	114
	 *
	 *	 o	 o	 o	 o	 o	 o	 o	161	057	141	025	125	013	123	014	126	026	142	058	162	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	089	169	065	041	137	029	021	131	022	030	138	042	066	170	090	 o	 o	 o	 o	 o	 o
	 *
	 *	 o	 o	 o	 o	 o	 o	 o	 o	081	153	045	145	037	135	038	146	046	154	082	 o	 o	 o	 o	 o	 o	 o	 o
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	193	181	085	069	061	053	151	054	062	070	086	182	203	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	105	205	197	 o	173	165	077	159	078	062	166	174	 o	198	206	106	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	109	 o	 o	093	 o	189	179	190	 o	094	 o	 o	110	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	101	203	102	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	217	211	218	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	229	223	230	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	241	235	242	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	253	247	254	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 
	 *	
	 *	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	117	 o	118	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 o	 
	 */
	
	row_width = h->nx + 2 * N_PAD;

	memcpy ((void *)ip, (void *)I, M_ACCESS * sizeof(TTT_LONG));
	memcpy ((void *)jp, (void *)J, M_ACCESS * sizeof(TTT_LONG));
	for (i = 0; i < M_ACCESS; i++) p[i] = -jp[i]*row_width + ip[i];	/* -vs because jp is positive up and rows increase down */
	
	switch (n_nodes) {	/* Return highest node number that need to be accessed */
		case 8:
			need_to_use = 8;
			break;
		case 16:
			need_to_use = 16;
			break;
		case 32:
			need_to_use = 72;
			break;
		case 48:
			need_to_use = 84;
			break;
		case 64:
			need_to_use = 92;
			break;
		case 120:
			need_to_use = 256;
			break;
		default:
			need_to_use = 0;
			fprintf (stderr, "ttt: Internal snafu. Number of nodes = %" TTT_LL "d\n", n_nodes);
			break;
	}

	return (need_to_use);
}

/* This routine indicates which slowness nodes are needed to calculate an incremental
 * travel time from current node, for all surrounding nodes [120].  This information is
 * encoded using bit arithmetic since only an ON/OFF flag is required per node.
 * NOTE: Some compilers may give a warning about exceeding integer range for some of
 * the assignments below.  This is not true since unsigned integers are used. */

#ifdef TEST
static void pbin (unsigned int a, unsigned int set) {

    for (int i = 31; i >= 0; i--) {
        if (a & 1) printf ("+%d", (31-i)+set*32);
        a >>= 1;
    }
}
#endif

static void ttt_set_use_bits (TTT_LONG n_nodes)
{
	TTT_LONG i;
#ifdef TEST
	int set, last_set;
#endif
	/* Because there are 256 nodes to address and we don't have 256-bit integers we
	 * use 256/32 = 8 32-bit integers to hold the 256 bits. I use 32-bit in case
	 * we are compiled on 32-bit machines. (Here, M_BITSETS = 8) */
	memset ((void *)use_bits, 0, N_CALC * M_BITSETS * sizeof (unsigned int));
	
	for (i = 0; i < 8; i++) use_bits[0][i] = BIT(i);	/* First L = 1 and L = sqrt(2) nodes only use one s-node each */
	
	/* L = sqrt(5) nodes involves 3 nodes */
	use_bits[0][8]  = BIT(0) + BIT(4) + BIT(8);
	use_bits[0][9]  = BIT(1) + BIT(5) + BIT(9);
	use_bits[0][10] = BIT(0) + BIT(6) + BIT(10);
	use_bits[0][11] = BIT(1) + BIT(7) + BIT(11);
	use_bits[0][12] = BIT(2) + BIT(4) + BIT(12);
	use_bits[0][13] = BIT(3) + BIT(5) + BIT(13);
	use_bits[0][14] = BIT(3) + BIT(6) + BIT(14);
	use_bits[0][15] = BIT(2) + BIT(7) + BIT(15);
	
	/* L = sqrt(10) involves nodes 5 nodes: 4 from set 0: [0-31] and 1 from set 3: [96-127] */
	use_bits[0][16] = use_bits[0][8]  + BIT(16);	use_bits[3][16] = BIT(120);
	use_bits[0][17] = use_bits[0][9]  + BIT(17);	use_bits[3][17] = BIT(121);
	use_bits[0][18] = use_bits[0][10] + BIT(18);	use_bits[3][18] = BIT(120);
	use_bits[0][19] = use_bits[0][11] + BIT(19);	use_bits[3][19] = BIT(121);
	use_bits[0][20] = use_bits[0][12] + BIT(20);	use_bits[3][20] = BIT(122);
	use_bits[0][21] = use_bits[0][13] + BIT(21);	use_bits[3][21] = BIT(123);
	use_bits[0][22] = use_bits[0][14] + BIT(22);	use_bits[3][22] = BIT(123);
	use_bits[0][23] = use_bits[0][15] + BIT(23);	use_bits[3][23] = BIT(122);

	/* L = sqrt(13) involves nodes 5 nodes: 4 from set 0: [0-31] and 1 from set 3: [96-127] */
	use_bits[0][24] = use_bits[0][8]  + BIT(24);	use_bits[3][24] = BIT(124);
	use_bits[0][25] = use_bits[0][9]  + BIT(25);	use_bits[3][25] = BIT(125);
	use_bits[0][26] = use_bits[0][10] + BIT(26);	use_bits[3][26] = BIT(126);
	use_bits[0][27] = use_bits[0][11] + BIT(27);	use_bits[3][27] = BIT(127);
	use_bits[0][28] = use_bits[0][12] + BIT(28);	use_bits[3][28] = BIT(124);
	use_bits[0][29] = use_bits[0][13] + BIT(29);	use_bits[3][29] = BIT(125);
	use_bits[0][30] = use_bits[0][14] + BIT(30);	use_bits[3][30] = BIT(126);
	use_bits[0][31] = use_bits[0][15] + BIT(31);	use_bits[3][31] = BIT(127);

	/* L = sqrt(17) involves nodes 7 nodes: 4 from set 0: [0-31], 1 from set 1: [32-63], 1 from set 3: [96-127], 1 from set 4: [128-159] */
	use_bits[0][32] = use_bits[0][16]; use_bits[1][32] = BIT(32); use_bits[3][32] = BIT(120); use_bits[4][32] = BIT(128);
	use_bits[0][33] = use_bits[0][17]; use_bits[1][33] = BIT(33); use_bits[3][33] = BIT(121); use_bits[4][33] = BIT(129);
	use_bits[0][34] = use_bits[0][18]; use_bits[1][34] = BIT(34); use_bits[3][34] = BIT(120); use_bits[4][34] = BIT(128);
	use_bits[0][35] = use_bits[0][19]; use_bits[1][35] = BIT(35); use_bits[3][35] = BIT(121); use_bits[4][35] = BIT(129);
	use_bits[0][36] = use_bits[0][20]; use_bits[1][36] = BIT(36); use_bits[3][36] = BIT(122); use_bits[4][36] = BIT(130);
	use_bits[0][37] = use_bits[0][21]; use_bits[1][37] = BIT(37); use_bits[3][37] = BIT(123); use_bits[4][37] = BIT(131);
	use_bits[0][38] = use_bits[0][22]; use_bits[1][38] = BIT(38); use_bits[3][38] = BIT(123); use_bits[4][38] = BIT(131);
	use_bits[0][39] = use_bits[0][23]; use_bits[1][39] = BIT(39); use_bits[3][39] = BIT(122); use_bits[4][39] = BIT(130);

	/* L = sqrt(25) involves nodes 7 nodes: 4 from set 0: [0-31], 1 from set 1: [32-63], 1 from set 3: [96-127], 1 from set 4: [128-159] */
	use_bits[0][40] = use_bits[0][24]; use_bits[1][40] = BIT(40); use_bits[3][40] = BIT(124); use_bits[4][40] = BIT(136);
	use_bits[0][41] = use_bits[0][25]; use_bits[1][41] = BIT(41); use_bits[3][41] = BIT(125); use_bits[4][41] = BIT(137);
	use_bits[0][42] = use_bits[0][26]; use_bits[1][42] = BIT(42); use_bits[3][42] = BIT(126); use_bits[4][42] = BIT(138);
	use_bits[0][43] = use_bits[0][27]; use_bits[1][43] = BIT(43); use_bits[3][43] = BIT(127); use_bits[4][43] = BIT(139);
	use_bits[0][44] = use_bits[0][28]; use_bits[1][44] = BIT(44); use_bits[3][44] = BIT(124); use_bits[4][44] = BIT(136);
	use_bits[0][45] = use_bits[0][29]; use_bits[1][45] = BIT(45); use_bits[3][45] = BIT(125); use_bits[4][45] = BIT(137);
	use_bits[0][46] = use_bits[0][30]; use_bits[1][46] = BIT(46); use_bits[3][46] = BIT(126); use_bits[4][46] = BIT(138);
	use_bits[0][47] = use_bits[0][31]; use_bits[1][47] = BIT(47); use_bits[3][47] = BIT(127); use_bits[4][47] = BIT(139);

	/* L = sqrt(26) involves nodes 9 nodes: 4 from set 0: [0-31], 2 from set 1: [32-63], 1 from set 3: [96-127], 2 from set 4: [128-159] */
	use_bits[0][48] = use_bits[0][16]; use_bits[1][48] = BIT(32) + BIT(48); use_bits[3][48] = BIT(120); use_bits[4][48] = BIT(128) + BIT(132);
	use_bits[0][49] = use_bits[0][17]; use_bits[1][49] = BIT(33) + BIT(49); use_bits[3][49] = BIT(121); use_bits[4][49] = BIT(129) + BIT(133);
	use_bits[0][50] = use_bits[0][18]; use_bits[1][50] = BIT(34) + BIT(50); use_bits[3][50] = BIT(120); use_bits[4][50] = BIT(128) + BIT(132);
	use_bits[0][51] = use_bits[0][19]; use_bits[1][51] = BIT(35) + BIT(51); use_bits[3][51] = BIT(121); use_bits[4][51] = BIT(129) + BIT(133);
	use_bits[0][52] = use_bits[0][20]; use_bits[1][52] = BIT(36) + BIT(52); use_bits[3][52] = BIT(122); use_bits[4][52] = BIT(130) + BIT(134);
	use_bits[0][53] = use_bits[0][21]; use_bits[1][53] = BIT(37) + BIT(53); use_bits[3][53] = BIT(123); use_bits[4][53] = BIT(131) + BIT(135);
	use_bits[0][54] = use_bits[0][22]; use_bits[1][54] = BIT(38) + BIT(54); use_bits[3][54] = BIT(123); use_bits[4][54] = BIT(131) + BIT(135);
	use_bits[0][55] = use_bits[0][23]; use_bits[1][55] = BIT(39) + BIT(55); use_bits[3][55] = BIT(122); use_bits[4][55] = BIT(130) + BIT(134);

	/* L = sqrt(29) involves nodes 9 nodes: 5 from set 0: [0-31], 2 from set 1: [32-63], 1 from set 3: [96-127], 1 from set 4: [128-159] */
	use_bits[0][56] = use_bits[0][16] + BIT(24); use_bits[1][56] = BIT(32) + BIT(56); use_bits[3][56] = BIT(120); use_bits[4][56] = BIT(140);
	use_bits[0][57] = use_bits[0][17] + BIT(25); use_bits[1][57] = BIT(33) + BIT(57); use_bits[3][57] = BIT(121); use_bits[4][57] = BIT(141);
	use_bits[0][58] = use_bits[0][18] + BIT(26); use_bits[1][58] = BIT(34) + BIT(58); use_bits[3][58] = BIT(120); use_bits[4][58] = BIT(142);
	use_bits[0][59] = use_bits[0][19] + BIT(27); use_bits[1][59] = BIT(35) + BIT(59); use_bits[3][59] = BIT(121); use_bits[4][59] = BIT(143);
	use_bits[0][60] = use_bits[0][20] + BIT(28); use_bits[1][60] = BIT(36) + BIT(60); use_bits[3][60] = BIT(122); use_bits[4][60] = BIT(144);
	use_bits[0][61] = use_bits[0][21] + BIT(29); use_bits[1][61] = BIT(37) + BIT(61); use_bits[3][61] = BIT(123); use_bits[4][61] = BIT(145);
	use_bits[0][62] = use_bits[0][22] + BIT(30); use_bits[1][62] = BIT(38) + BIT(62); use_bits[3][62] = BIT(123); use_bits[4][62] = BIT(146);
	use_bits[0][63] = use_bits[0][23] + BIT(31); use_bits[1][63] = BIT(39) + BIT(63); use_bits[3][63] = BIT(122); use_bits[4][63] = BIT(147);

	/* L = sqrt(34) involves nodes 9 nodes: 5 from set 0: [0-31], 1 from set 1: [32-63], 1 from set 2: [64-95], 1 from set 3: [96-127], 1 from set 4: [128-159] */
	use_bits[0][64] = use_bits[0][56]; use_bits[1][64] = BIT(40); use_bits[2][64] = BIT(64);  use_bits[3][64] = BIT(124); use_bits[4][64] = BIT(140);
	use_bits[0][65] = use_bits[0][57]; use_bits[1][65] = BIT(41); use_bits[2][65] = BIT(65);  use_bits[3][65] = BIT(125); use_bits[4][65] = BIT(141);
	use_bits[0][66] = use_bits[0][58]; use_bits[1][66] = BIT(42); use_bits[2][66] = BIT(66);  use_bits[3][66] = BIT(126); use_bits[4][66] = BIT(142);
	use_bits[0][67] = use_bits[0][59]; use_bits[1][67] = BIT(43); use_bits[2][67] = BIT(67);  use_bits[3][67] = BIT(127); use_bits[4][67] = BIT(143);
	use_bits[0][68] = use_bits[0][60]; use_bits[1][68] = BIT(44); use_bits[2][68] = BIT(68);  use_bits[3][68] = BIT(124); use_bits[4][68] = BIT(144);
	use_bits[0][69] = use_bits[0][61]; use_bits[1][69] = BIT(45); use_bits[2][69] = BIT(69);  use_bits[3][69] = BIT(125); use_bits[4][69] = BIT(145);
	use_bits[0][70] = use_bits[0][62]; use_bits[1][70] = BIT(46); use_bits[2][70] = BIT(70);  use_bits[3][70] = BIT(126); use_bits[4][70] = BIT(146);
	use_bits[0][71] = use_bits[0][63]; use_bits[1][71] = BIT(47); use_bits[2][71] = BIT(71);  use_bits[3][71] = BIT(127); use_bits[4][71] = BIT(147);

	/* L = sqrt(37) involves nodes 11 nodes: 4 from set 0: [0-31], 2 from set 1: [32-63], 1 from set 2: [64-95], 1 from set 3: [96-127], 3 from set 4: [128-159] */
	use_bits[0][72] = use_bits[0][48]; use_bits[1][72] = BIT(32) + BIT(48); use_bits[2][72] = BIT(72);  use_bits[3][72] = BIT(120); use_bits[4][72] = BIT(128) + BIT(132) + BIT(148);
	use_bits[0][73] = use_bits[0][49]; use_bits[1][73] = BIT(33) + BIT(49); use_bits[2][73] = BIT(73);  use_bits[3][73] = BIT(121); use_bits[4][73] = BIT(129) + BIT(133) + BIT(149);
	use_bits[0][74] = use_bits[0][50]; use_bits[1][74] = BIT(34) + BIT(50); use_bits[2][74] = BIT(74);  use_bits[3][74] = BIT(120); use_bits[4][74] = BIT(128) + BIT(132) + BIT(148);
	use_bits[0][75] = use_bits[0][51]; use_bits[1][75] = BIT(35) + BIT(51); use_bits[2][75] = BIT(75);  use_bits[3][75] = BIT(121); use_bits[4][75] = BIT(129) + BIT(133) + BIT(149);
	use_bits[0][76] = use_bits[0][52]; use_bits[1][76] = BIT(36) + BIT(52); use_bits[2][76] = BIT(76);  use_bits[3][76] = BIT(122); use_bits[4][76] = BIT(130) + BIT(134) + BIT(150);
	use_bits[0][77] = use_bits[0][53]; use_bits[1][77] = BIT(37) + BIT(53); use_bits[2][77] = BIT(77);  use_bits[3][77] = BIT(123); use_bits[4][77] = BIT(131) + BIT(135) + BIT(151);
	use_bits[0][78] = use_bits[0][54]; use_bits[1][78] = BIT(38) + BIT(54); use_bits[2][78] = BIT(78);  use_bits[3][78] = BIT(123); use_bits[4][78] = BIT(131) + BIT(135) + BIT(151);
	use_bits[0][79] = use_bits[0][55]; use_bits[1][79] = BIT(39) + BIT(55); use_bits[2][79] = BIT(79);  use_bits[3][79] = BIT(122); use_bits[4][79] = BIT(130) + BIT(134) + BIT(150);

	/* L = sqrt(41) involves nodes 9 nodes: 4 from set 0: [0-31], 1 from set 1: [32-63], 1 from set 2: [64-95], 1 from set 3: [96-127], 2 from set 4: [128-159] */
	use_bits[0][80] = use_bits[0][40]; use_bits[1][80] = BIT(40); use_bits[2][80] = BIT(80);  use_bits[3][80] = BIT(124); use_bits[4][80] = BIT(136) + BIT(152);
	use_bits[0][81] = use_bits[0][41]; use_bits[1][81] = BIT(41); use_bits[2][81] = BIT(81);  use_bits[3][81] = BIT(125); use_bits[4][81] = BIT(137) + BIT(153);
	use_bits[0][82] = use_bits[0][42]; use_bits[1][82] = BIT(42); use_bits[2][82] = BIT(82);  use_bits[3][82] = BIT(126); use_bits[4][82] = BIT(138) + BIT(154);
	use_bits[0][83] = use_bits[0][43]; use_bits[1][83] = BIT(43); use_bits[2][83] = BIT(83);  use_bits[3][83] = BIT(127); use_bits[4][83] = BIT(139) + BIT(155);
	use_bits[0][84] = use_bits[0][44]; use_bits[1][84] = BIT(44); use_bits[2][84] = BIT(84);  use_bits[3][84] = BIT(124); use_bits[4][84] = BIT(136) + BIT(152);
	use_bits[0][85] = use_bits[0][45]; use_bits[1][85] = BIT(45); use_bits[2][85] = BIT(85);  use_bits[3][85] = BIT(125); use_bits[4][85] = BIT(137) + BIT(153);
	use_bits[0][86] = use_bits[0][46]; use_bits[1][86] = BIT(46); use_bits[2][86] = BIT(86);  use_bits[3][86] = BIT(126); use_bits[4][86] = BIT(138) + BIT(154);
	use_bits[0][87] = use_bits[0][47]; use_bits[1][87] = BIT(47); use_bits[2][87] = BIT(87);  use_bits[3][87] = BIT(127); use_bits[4][87] = BIT(139) + BIT(155);

	/* L = sqrt(58) involves nodes 13 nodes: 5 from set 0: [0-31], 2 from set 1: [32-63], 2 from set 2: [64-95], 1 from set 3: [96-127], 1 from set 4: [128-159], 2 from set 5: [160-191] */
	use_bits[0][88] = use_bits[0][56]; use_bits[1][88] = BIT(32) + BIT(56); use_bits[2][88] = BIT(64) + BIT(88); use_bits[3][88] = BIT(120); use_bits[4][88] = BIT(140); use_bits[5][88] = BIT(160) + BIT(168);
	use_bits[0][89] = use_bits[0][57]; use_bits[1][89] = BIT(33) + BIT(57); use_bits[2][89] = BIT(65) + BIT(89); use_bits[3][89] = BIT(121); use_bits[4][89] = BIT(141); use_bits[5][89] = BIT(161) + BIT(169);
	use_bits[0][90] = use_bits[0][58]; use_bits[1][90] = BIT(34) + BIT(58); use_bits[2][90] = BIT(66) + BIT(90); use_bits[3][90] = BIT(120); use_bits[4][90] = BIT(142); use_bits[5][90] = BIT(162) + BIT(170);
	use_bits[0][91] = use_bits[0][59]; use_bits[1][91] = BIT(35) + BIT(59); use_bits[2][91] = BIT(67) + BIT(91); use_bits[3][91] = BIT(121); use_bits[4][91] = BIT(143); use_bits[5][91] = BIT(163) + BIT(171);
	use_bits[0][92] = use_bits[0][60]; use_bits[1][92] = BIT(36) + BIT(60); use_bits[2][92] = BIT(68) + BIT(92); use_bits[3][92] = BIT(122); use_bits[4][92] = BIT(144); use_bits[5][92] = BIT(164) + BIT(172);
	use_bits[0][93] = use_bits[0][61]; use_bits[1][93] = BIT(37) + BIT(61); use_bits[2][93] = BIT(69) + BIT(93); use_bits[3][93] = BIT(123); use_bits[4][93] = BIT(145); use_bits[5][93] = BIT(165) + BIT(173);
	use_bits[0][94] = use_bits[0][62]; use_bits[1][94] = BIT(38) + BIT(62); use_bits[2][94] = BIT(70) + BIT(94); use_bits[3][94] = BIT(123); use_bits[4][94] = BIT(146); use_bits[5][94] = BIT(166) + BIT(174);
	use_bits[0][95] = use_bits[0][63]; use_bits[1][95] = BIT(39) + BIT(63); use_bits[2][95] = BIT(71) + BIT(95); use_bits[3][95] = BIT(122); use_bits[4][95] = BIT(147); use_bits[5][95] = BIT(167) + BIT(175);

	/* L = sqrt(65) involves nodes 15 nodes: 4 from set 0: [0-31], 2 from set 1: [32-63], 1 from set 2: [64-95], 2 from set 3: [96-127], 4 from set 4: [128-159], 2 from set 5: [160-191] */
	use_bits[0][96]  = use_bits[0][72]; use_bits[1][96]  = use_bits[1][48]; use_bits[2][96]  = BIT(72); use_bits[3][96]  =  BIT(96) + BIT(120); use_bits[4][96]  = BIT(128) + BIT(132) + BIT(148) + BIT(156); use_bits[5][96]  = BIT(176) + BIT(184);
	use_bits[0][97]  = use_bits[0][73]; use_bits[1][97]  = use_bits[1][49]; use_bits[2][97]  = BIT(73); use_bits[3][97]  =  BIT(97) + BIT(121); use_bits[4][97]  = BIT(129) + BIT(133) + BIT(149) + BIT(157); use_bits[5][97]  = BIT(177) + BIT(185);
	use_bits[0][98]  = use_bits[0][74]; use_bits[1][98]  = use_bits[1][50]; use_bits[2][98]  = BIT(74); use_bits[3][98]  =  BIT(98) + BIT(120); use_bits[4][98]  = BIT(128) + BIT(132) + BIT(148) + BIT(156); use_bits[5][98]  = BIT(176) + BIT(186);
	use_bits[0][99]  = use_bits[0][75]; use_bits[1][99]  = use_bits[1][51]; use_bits[2][99]  = BIT(75); use_bits[3][99]  =  BIT(99) + BIT(121); use_bits[4][99]  = BIT(129) + BIT(133) + BIT(149) + BIT(157); use_bits[5][99]  = BIT(177) + BIT(187);
	use_bits[0][100] = use_bits[0][76]; use_bits[1][100] = use_bits[1][52]; use_bits[2][100] = BIT(76); use_bits[3][100] = BIT(100) + BIT(122); use_bits[4][100] = BIT(130) + BIT(134) + BIT(150) + BIT(158); use_bits[5][100] = BIT(178) + BIT(188);
	use_bits[0][101] = use_bits[0][77]; use_bits[1][101] = use_bits[1][53]; use_bits[2][101] = BIT(77); use_bits[3][101] = BIT(101) + BIT(123); use_bits[4][101] = BIT(131) + BIT(135) + BIT(151) + BIT(159); use_bits[5][101] = BIT(179) + BIT(189);
	use_bits[0][102] = use_bits[0][78]; use_bits[1][102] = use_bits[1][54]; use_bits[2][102] = BIT(78); use_bits[3][102] = BIT(102) + BIT(123); use_bits[4][102] = BIT(131) + BIT(135) + BIT(151) + BIT(159); use_bits[5][102] = BIT(179) + BIT(190);
	use_bits[0][103] = use_bits[0][79]; use_bits[1][103] = use_bits[1][55]; use_bits[2][103] = BIT(79); use_bits[3][103] = BIT(103) + BIT(122); use_bits[4][103] = BIT(130) + BIT(134) + BIT(150) + BIT(158); use_bits[5][103] = BIT(178) + BIT(191);

	/* L = sqrt(85) involves nodes 13 nodes: 4 from set 0: [0-31], 1 from set 1: [32-63], 1 from set 2: [64-95], 2 from set 3: [96-127], 2 from set 4: [128-159], 1 from set 5: [160-191], 2 from set 6: [192-223] */
	use_bits[0][104] = use_bits[0][56]; use_bits[1][104] = BIT(40); use_bits[2][104] = BIT(80); use_bits[3][104] = BIT(104) + BIT(124); use_bits[4][104] = BIT(136) + BIT(152); use_bits[5][104] = BIT(180); use_bits[6][104] = BIT(192) + BIT(204);
	use_bits[0][105] = use_bits[0][57]; use_bits[1][105] = BIT(41); use_bits[2][105] = BIT(81); use_bits[3][105] = BIT(105) + BIT(125); use_bits[4][105] = BIT(137) + BIT(153); use_bits[5][105] = BIT(181); use_bits[6][105] = BIT(193) + BIT(205);
	use_bits[0][106] = use_bits[0][58]; use_bits[1][106] = BIT(42); use_bits[2][106] = BIT(82); use_bits[3][106] = BIT(106) + BIT(126); use_bits[4][106] = BIT(138) + BIT(154); use_bits[5][106] = BIT(182); use_bits[6][106] = BIT(194) + BIT(206);
	use_bits[0][107] = use_bits[0][59]; use_bits[1][107] = BIT(43); use_bits[2][107] = BIT(83); use_bits[3][107] = BIT(107) + BIT(127); use_bits[4][107] = BIT(139) + BIT(155); use_bits[5][107] = BIT(183); use_bits[6][107] = BIT(195) + BIT(207);
	use_bits[0][108] = use_bits[0][60]; use_bits[1][108] = BIT(44); use_bits[2][108] = BIT(84); use_bits[3][108] = BIT(108) + BIT(124); use_bits[4][108] = BIT(136) + BIT(152); use_bits[5][108] = BIT(180); use_bits[6][108] = BIT(196) + BIT(204);
	use_bits[0][109] = use_bits[0][61]; use_bits[1][109] = BIT(45); use_bits[2][109] = BIT(85); use_bits[3][109] = BIT(109) + BIT(125); use_bits[4][109] = BIT(137) + BIT(153); use_bits[5][109] = BIT(181); use_bits[6][109] = BIT(197) + BIT(205);
	use_bits[0][110] = use_bits[0][62]; use_bits[1][110] = BIT(46); use_bits[2][110] = BIT(86); use_bits[3][110] = BIT(110) + BIT(126); use_bits[4][110] = BIT(138) + BIT(154); use_bits[5][110] = BIT(182); use_bits[6][110] = BIT(198) + BIT(206);
	use_bits[0][111] = use_bits[0][63]; use_bits[1][111] = BIT(47); use_bits[2][111] = BIT(87); use_bits[3][111] = BIT(111) + BIT(127); use_bits[4][111] = BIT(139) + BIT(155); use_bits[5][111] = BIT(183); use_bits[6][111] = BIT(199) + BIT(207);

	/* L = sqrt(170) involves nodes 25 nodes: 4 from set 0: [0-31], 2 from set 1: [32-63], 1 from set 2: [64-95], 3 from set 3: [96-127],                    4 from set 4: [128-159],                                      2 from set 5: [160-191],                4 from set 6: [192-223],                                      5 from set 7: [224-255] */
	use_bits[0][112] = use_bits[0][72]; use_bits[1][112] = BIT(32) + BIT(48); use_bits[2][112] = BIT(72); use_bits[3][112] =  BIT(96) + BIT(112) + BIT(120); use_bits[4][112] = BIT(128) + BIT(132) + BIT(148) + BIT(156); use_bits[5][112] = BIT(176) + BIT(184); use_bits[6][112] = BIT(200) + BIT(208) + BIT(212) + BIT(220); use_bits[7][112] = BIT(224) + BIT(232) + BIT(236) + BIT(244) + BIT(248);
	use_bits[0][113] = use_bits[0][73]; use_bits[1][113] = BIT(33) + BIT(49); use_bits[2][113] = BIT(73); use_bits[3][113] =  BIT(97) + BIT(113) + BIT(121); use_bits[4][113] = BIT(129) + BIT(133) + BIT(149) + BIT(157); use_bits[5][113] = BIT(177) + BIT(185); use_bits[6][113] = BIT(201) + BIT(209) + BIT(213) + BIT(221); use_bits[7][113] = BIT(225) + BIT(233) + BIT(237) + BIT(245) + BIT(249);
	use_bits[0][114] = use_bits[0][74]; use_bits[1][114] = BIT(34) + BIT(50); use_bits[2][114] = BIT(74); use_bits[3][114] =  BIT(98) + BIT(114) + BIT(120); use_bits[4][114] = BIT(128) + BIT(132) + BIT(148) + BIT(156); use_bits[5][114] = BIT(176) + BIT(186); use_bits[6][114] = BIT(200) + BIT(208) + BIT(214) + BIT(220); use_bits[7][114] = BIT(226) + BIT(232) + BIT(238) + BIT(244) + BIT(250);
	use_bits[0][115] = use_bits[0][75]; use_bits[1][115] = BIT(35) + BIT(51); use_bits[2][115] = BIT(75); use_bits[3][115] =  BIT(99) + BIT(115) + BIT(121); use_bits[4][115] = BIT(129) + BIT(133) + BIT(149) + BIT(157); use_bits[5][115] = BIT(177) + BIT(187); use_bits[6][115] = BIT(201) + BIT(209) + BIT(215) + BIT(221); use_bits[7][115] = BIT(227) + BIT(233) + BIT(239) + BIT(245) + BIT(251);
	use_bits[0][116] = use_bits[0][76]; use_bits[1][116] = BIT(36) + BIT(52); use_bits[2][116] = BIT(76); use_bits[3][116] = BIT(100) + BIT(116) + BIT(122); use_bits[4][116] = BIT(130) + BIT(134) + BIT(150) + BIT(158); use_bits[5][116] = BIT(178) + BIT(188); use_bits[6][116] = BIT(202) + BIT(210) + BIT(216) + BIT(222); use_bits[7][116] = BIT(228) + BIT(234) + BIT(240) + BIT(246) + BIT(252);
	use_bits[0][117] = use_bits[0][77]; use_bits[1][117] = BIT(37) + BIT(53); use_bits[2][117] = BIT(77); use_bits[3][117] = BIT(101) + BIT(117) + BIT(123); use_bits[4][117] = BIT(131) + BIT(135) + BIT(151) + BIT(159); use_bits[5][117] = BIT(179) + BIT(189); use_bits[6][117] = BIT(203) + BIT(211) + BIT(217) + BIT(223); use_bits[7][117] = BIT(229) + BIT(235) + BIT(241) + BIT(247) + BIT(253);
	use_bits[0][118] = use_bits[0][78]; use_bits[1][118] = BIT(38) + BIT(54); use_bits[2][118] = BIT(78); use_bits[3][118] = BIT(102) + BIT(118) + BIT(123); use_bits[4][118] = BIT(131) + BIT(135) + BIT(151) + BIT(159); use_bits[5][118] = BIT(179) + BIT(190); use_bits[6][118] = BIT(203) + BIT(211) + BIT(218) + BIT(223); use_bits[7][118] = BIT(230) + BIT(235) + BIT(242) + BIT(247) + BIT(254);
	use_bits[0][119] = use_bits[0][79]; use_bits[1][119] = BIT(39) + BIT(55); use_bits[2][119] = BIT(79); use_bits[3][119] = BIT(103) + BIT(119) + BIT(122); use_bits[4][119] = BIT(130) + BIT(134) + BIT(150) + BIT(158); use_bits[5][119] = BIT(178) + BIT(191); use_bits[6][119] = BIT(202) + BIT(210) + BIT(219) + BIT(222); use_bits[7][119] = BIT(231) + BIT(234) + BIT(243) + BIT(246) + BIT(255);

#ifdef TEST
	printf ("\n");
	for (i = 0; i < n_nodes; i++) {
		printf ("%3.3lu:", i);
		for (set = M_BITSETS-1, last_set = 100; set >= 0 && last_set == 100; set--)
			if (use_bits[set][i]) last_set = set;
		for (set = 0; set <= last_set; set++)
			pbin (use_bits[set][i], set);
		printf ("\n");
	}
#endif
}

/* Initialize the distances from center node to neighbor nodes */

static void ttt_init_distances (struct GRD_HEADER *h, TTT_LONG n_nodes)
{
	TTT_LONG j;
#ifndef FLAT
	double latitude;
#endif
		
	ttt_set_use_bits (n_nodes);

	dx = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	dr = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	
	if (n_nodes > 8) {
		d5x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d5y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	}
	if (n_nodes > 16) {
		d10x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d10y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d13x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d13y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	}
	if (n_nodes > 32) {
		d17x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d17y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d25x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d25y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	}
	if (n_nodes > 48) {
		d26x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d26y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d29x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d29y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	}
	if (n_nodes > 64) {
		d34x  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d34y  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d37x  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d37y  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d41x  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d41y  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d58x  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d58y  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d65x  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d65y  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d85x  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d85y  = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d170x = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
		d170y = (double *)ttt_memory (NULL, h->ny, sizeof (double), "ttt");
	}
	
	dy = 0.5 * h->inc[TTT_Y] * DEGREE_TO_METER;	/* 0.5 factor put here rather than in main huygens loop */
		
	for (j = 0; j < h->ny; j++) {
#ifdef FLAT
		dx[j] = 0.5 * h->inc[TTT_X] * DEGREE_TO_METER;
#else
		latitude = ttt_j_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
		dx[j] = 0.5 * h->inc[TTT_X] * DEGREE_TO_METER * cos (D2R * latitude);
#endif
		dr[j] = hypot (dx[j], dy);
		
		if (n_nodes == 8) continue;
		
		d5x[j] = hypot (0.5 * dx[j], dy);
		d5y[j] = hypot (dx[j], 0.5 * dy);

		if (n_nodes == 16) continue;

		d10x[j] = hypot (ONE_3RD * dx[j], dy);
		d10y[j] = hypot (dx[j], ONE_3RD * dy);
		d13x[j] = hypot (TWO_3RD * dx[j], dy);
		d13y[j] = hypot (dx[j], TWO_3RD * dy);

		if (n_nodes == 32) continue;

		d17x[j] = hypot (0.25 * dx[j], dy);
		d17y[j] = hypot (dx[j], 0.25 * dy);
		d25x[j] = hypot (0.75 * dx[j], dy);
		d25y[j] = hypot (dx[j], 0.75 * dy);

		if (n_nodes == 48) continue;

		d26x[j] = hypot (0.2 * dx[j], dy);
		d26y[j] = hypot (dx[j], 0.2 * dy);
		d29x[j] = hypot (0.4 * dx[j], dy);
		d29y[j] = hypot (dx[j], 0.4 * dy);

		if (n_nodes == 64) continue;

		d34x[j]  = hypot (0.6 * dx[j], dy);
		d34y[j]  = hypot (dx[j], 0.6 * dy);
		d37x[j]  = hypot (ONE_6TH * dx[j], dy);
		d37y[j]  = hypot (dx[j], ONE_6TH * dy);
		d41x[j]  = hypot (0.8 * dx[j], dy);
		d41y[j]  = hypot (dx[j], 0.8 * dy);
		d58x[j]  = hypot (THREE_7TH * dx[j], dy);
		d58y[j]  = hypot (dx[j], THREE_7TH * dy);
		d65x[j]  = hypot (0.125 * dx[j], dy);
		d65y[j]  = hypot (dx[j], 0.125 * dy);
		d85x[j]  = hypot (SIX_7TH * dx[j], dy);
		d85y[j]  = hypot (dx[j], SIX_7TH * dy);
		d170x[j] = hypot (ONE_13TH * dx[j], dy);
		d170y[j] = hypot (dx[j], ONE_13TH * dy);
	}
}

static double ttt_gravity (double latitude)
{	/* Returns normal gravity at this latitude per GRS-80 */
#ifdef TEST
	(void) (latitude);
	return (9.81);
#else	
	double s = sin (D2R * latitude);
	s *= s;	/* sin squared */
	return (G_E * (1.0 + G_C1 * s + G_C2 * s * s));
#endif
}

/* Convert bathymetry to slowness (in s/m) using the relation s = 1/sqrt (depth(m) * normal_gravity).
 * Since dt = d * s (in sec) but we want dt in T_UNIT we put the factor 1/T_UNIT into the slowness values */

static void ttt_calc_slowness (struct GRD_HEADER *h, float *s, double sign, double depth_threshold)
{
	TTT_LONG i, j, k, p, row_width, col_height, adjustment;
	BOOLEAN depth_ramp = FALSE;
	double norm_grav, *latitude = NULL, a, new_z, depth_cutoff;
	float TTT_NaN;
	
	TTT_make_NaN (TTT_NaN);

	row_width  = h->nx + 2 * N_PAD;
	col_height = h->ny + 2 * N_PAD;
	adjustment = (h->registration) ? 1 : 0;
	if (depth_threshold > 1e-6) depth_ramp = TRUE;
	
	/* Set up latitude array that handles pole wrap (in the event we go to poles) */
	
	latitude = (double *)ttt_memory (NULL, col_height, sizeof (double), "ttt");
	for (j = 0; j < h->ny; j++) latitude[j+N_PAD] = ttt_j_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
	for (p = 1; p <= N_PAD; p++) {
		latitude[N_PAD-p] = latitude[N_PAD+p-adjustment];
		latitude[col_height-N_PAD-1+p] = latitude[col_height-N_PAD-1-p+adjustment];
	}

	/* So, if we include poles then lat array is set to give correct grav; otherwise
	 * s[k] will be zero outside the interior grid and NaN will be assigned */
	 
	sign /= T_UNIT;	/* To get integer time units instead of seconds */
	a = (depth_ramp) ? 0.25 / depth_threshold : 0.0;
	depth_cutoff = 2.0 * depth_threshold;
	for (j = k = 0; j < col_height; j++) {	/* Work on extended grid */
		norm_grav = ttt_gravity (latitude[j]);	/* GRS-80 normal gravity */
		if (depth_ramp) {
			for (i = 0; i < row_width; i++, k++) {
				if (s[k] >= 0.0)	/* Any positive topography (land) gets set to NaN */
					s[k] = TTT_NaN;
				else if (s[k] < depth_cutoff)	/* Regular depths are used as is */
					s[k] = (float)(sign / sqrt ((double)(-s[k] * norm_grav)));
				else {	/* Quadratic tapering to threshold */
					new_z = a * s[k] * s[k] + depth_threshold;
					s[k] = (float)(sign / sqrt ((double)(-new_z * norm_grav)));
				}
			}
		}
		else	/* No ramp, just land or ocean */
			for (i = 0; i < row_width; i++, k++) s[k] = (s[k] >= 0.0) ? TTT_NaN : (float)(sign / sqrt ((double)(-s[k] * norm_grav)));
	}
}

/* Main routine for realtime (i.e. from bathymetry) calculation of travel times */

static void ttt_huygens_realtime (struct GRD_HEADER *h, TTT_LONG *ij_quake, TTT_LONG n_sources, float *s, TTT_LONG n_nodes, TTT_LONG *ip, TTT_LONG *jp, TTT_LONG *p, TTT_LONG max_nodes, TTT_LONG verbose)
{
	TTT_LONG ij, kk, k, nm_grid, row_width, col_height, i, j, x_on_pad, y_on_pad, i0, j0, i_k, j_k, try = 0;
	TTT_LONG n_left, i360, nx_half, nx1, adjustment, ew_wrap, ignore_east, *tt_min = NULL, n_insert = 0;
	TTT_LONG last_percent = 0, next_tt, current_tt, set = 1, percent = 0, tt_inc[N_CALC];
	TTT_LONG tt_inc_min = LONG_MAX, tt_inc_max = 0;
	unsigned int u, su, bset, n_bitsets = M_BITSETS, ok_to_use[N_CALC], limit[M_BITSETS-1], nan_bits[M_BITSETS];
	double c, ss;
	float TTT_NaN;
	struct TTT_INFO *list_tail = NULL, *list_head = NULL;
#ifdef TREE_DEBUG
	FILE *fp = fopen ("debug.txt", "w");
#endif	
#ifdef TTT120
	(void)(n_nodes);
	(void)(max_nodes);
#endif	
	/* Misc. initializations */

	row_width  = h->nx + 2 * N_PAD;
	col_height = h->ny + 2 * N_PAD;
	nx_half = h->nx / 2;
	nx1 = (h->registration) ? h->nx : h->nx - 1;
	adjustment = (h->registration) ? 1 : 0;
	ew_wrap = 2 * h->ny - 2 + adjustment;
	
	TTT_make_NaN (TTT_NaN);
	nm_grid = n_left = row_width * col_height;

	/* This routine will visit all points on the grid and calculate incremental travel times to all the
	 * neighboring nodes [8 to 92, depending on -N].  These are added to the travel time to the current
	 * node to obtain suggestions for total travel time to all the neighboring nodes.  Obviously, for each
	 * time we do this we get different travel times to the same nodes.  We must keep track of the shortest
	 * travel time for any node visited as well as maintain a list of which node has the lowest travel time
	 * after the current node [this is were we will go to next].  We achieve this by maintaining a binary tree
	 * structure.  When the tree is empty we have reached the end.
	 */

	tt_min = (TTT_LONG *) ttt_memory (NULL, nm_grid, sizeof (TTT_LONG), "ttt");

	/* Initialize the tt_min array: LAND if not reachable, OCEAN if not used yet */
	
	i360 = row_width - N_PAD - 1;	/* Since xmin == xmax we temporarily ignore the x = xmax column if gridnode-registration */
	ignore_east = (h->periodic && !h->registration);
	/* Loop over the entire grid, including the pad */
	for (j = 0; j < col_height; j++) {
		y_on_pad = (j < N_PAD || j >= (col_height - N_PAD));	/* True of outside grid and on the y pad */
		for (i = 0; i < row_width; i++) {
			k = IJ(i,j,row_width,0);	/* Index of starting node of this row, i.e., the first node in the west pad */
			x_on_pad = (i < N_PAD || i >= (row_width - N_PAD) || (ignore_east && i == i360));
			if (x_on_pad || y_on_pad || TTT_is_fnan(s[k])) {
				tt_min[k] = LAND;
				n_left--;
			}
			else
				tt_min[k] = OCEAN;
		}
	}
	
	c = 100.0 / n_left;	/* Used to report the percentage of work completed inside the main loop */
		
	/* Create the binary tree head and tail */
	
	treeinitialize (&list_head, &list_tail);

	/* Initialize the node(s) with the epicenter(s) */
	
	for (k = 0; k < n_sources; k++) {
		if (tt_min[ij_quake[k]] == 0) continue;		/* Already set to 0 (Can happen if the same coordinate was given more than once) */
		tt_min[ij_quake[k]] = 0;			/* tt to the node at the quake is zero */
		treeinsert (0, ij_quake[k], list_head, list_tail);	/* Insert into our binary tree */
	}
		 
	/* Start off from the first in the list */

	current_tt = list_head->r->tt;	/* Travel times start at 0 */
	ij = list_head->r->ij;		/* Get node index */
	j0 = (ij / row_width) - N_PAD;
	i0 = (ij % row_width) - N_PAD;
	tt_min[ij] = USED;		/* Mark this node as used */
	treedelete (current_tt, ij, list_head, list_tail);	/* Remove from binary tree */
#ifdef TTT120
	limit[0] = 16U;
	limit[1] = 32U;
	limit[2] = 64U;
	limit[3] = 88U;
	limit[4] = 104U;
	limit[5] = 112U;
	limit[6] = 120U;
#else
	n_bitsets = (unsigned int)ceil (max_nodes / 32.0);
	limit[0] = MIN(16U,  (unsigned int)n_nodes);
	limit[1] = MIN(32U,  (unsigned int)n_nodes);
	limit[2] = MIN(64U,  (unsigned int)n_nodes);
	limit[3] = MIN(88U,  (unsigned int)n_nodes);
	limit[4] = MIN(104U, (unsigned int)n_nodes);
	limit[5] = MIN(112U, (unsigned int)n_nodes);
	limit[6] = MIN(120U, (unsigned int)n_nodes);
#endif
	n_left--;
	memset ((void *)tt_inc, 0, N_CALC * sizeof (TTT_LONG));

	do {	/* While there is more work to be done */
		try++;
		/* Set bit flags so we only calculate incremental travel times to nodes that still are in the running */

		memset ((void *)nan_bits, 0, M_BITSETS * sizeof (unsigned int));
		for (u = bset = 0; bset < n_bitsets; bset++) {	/* Set as many sets of 32-bit flags as there are max_nodes to be used */
			for (su = 0; su < 32U; su++, u++) { /* For the next set of 32 nodes... */
				if (TTT_is_fnan(S(u)) || S(u) >= 0.0) nan_bits[bset] |= BIT(u);
			}
		}
	
		/* Initialize ok_to_use flags and set all inc tts to LAND */
		for (u = 0; u < limit[0]; u++)		/* Only depends on set 0 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0);
		for (u = 16U; u < limit[1]; u++)		/* Depends on sets 0, 3 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0 && (nan_bits[3] & use_bits[3][u]) == 0);
		for (u = 32U; u < limit[2]; u++)		/* Depends on sets 0, 1, 3, 4 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0 && (nan_bits[1] & use_bits[1][u]) == 0 && (nan_bits[3] & use_bits[3][u]) == 0 && (nan_bits[4] & use_bits[4][u]) == 0);
		for (u = 64U; u < limit[3]; u++)		/* Depends on sets 0-4 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0 && (nan_bits[1] & use_bits[1][u]) == 0 && (nan_bits[2] & use_bits[2][u]) == 0 && (nan_bits[3] & use_bits[3][u]) == 0 && (nan_bits[4] & use_bits[4][u]) == 0);
		for (u = 88U; u < limit[4]; u++)	/* Depends on sets 0-5 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0 && (nan_bits[1] & use_bits[1][u]) == 0 && (nan_bits[2] & use_bits[2][u]) == 0 && (nan_bits[3] & use_bits[3][u]) == 0 && (nan_bits[4] & use_bits[4][u]) == 0 && (nan_bits[5] & use_bits[5][u]) == 0);
		for (u = 104U; u < limit[5]; u++)	/* Depends on sets 0-6 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0 && (nan_bits[1] & use_bits[1][u]) == 0 && (nan_bits[2] & use_bits[2][u]) == 0 && (nan_bits[3] & use_bits[3][u]) == 0 && (nan_bits[4] & use_bits[4][u]) == 0 && (nan_bits[5] & use_bits[5][u]) == 0 && (nan_bits[6] & use_bits[6][u]) == 0);
		for (u = 112U; u < limit[6]; u++)	/* Depends on sets 0-7 */
			ok_to_use[u] = ((nan_bits[0] & use_bits[0][u]) == 0 && (nan_bits[1] & use_bits[1][u]) == 0 && (nan_bits[2] & use_bits[2][u]) == 0 && (nan_bits[3] & use_bits[3][u]) == 0 && (nan_bits[4] & use_bits[4][u]) == 0 && (nan_bits[5] & use_bits[5][u]) == 0 && (nan_bits[6] & use_bits[6][u]) == 0 && (nan_bits[7] & use_bits[7][u]) == 0);

		/* j0 is row number (i.e., latitude indicator) or current node */
	
		//for (u = 64; u < 120; u++) if (u >= 72) ok_to_use[u] = 0;	/* Only allow these new nodes */
		
		/* Remember that here, all non-NaN slownesses are negative values. */
		/* Below, the -0.5 will be added to save the overhead of calling the rint function */

#ifndef TTT120
		switch (n_nodes) {	/* We know n_nodes is 8, 16, 32, 48, 64, or 120 */

			/* NOTE there are no breaks after each case.  This is intentional;  the
			   full 120-node solution also needs all the terms for the lower nodes.
			   Because we use individually named macros we must call them one by one.
			 */

			case 120:	/* Fullblown solution at super precision */
#endif

				/* Do sqrt(34) nodes */

				if (ok_to_use[64]) ss = SS_064, tt_inc[64] = GET_TT_INC (d34y[j0]);
				if (ok_to_use[65]) ss = SS_065, tt_inc[65] = GET_TT_INC (d34y[j0]);
				if (ok_to_use[66]) ss = SS_066, tt_inc[66] = GET_TT_INC (d34y[j0]);
				if (ok_to_use[67]) ss = SS_067, tt_inc[67] = GET_TT_INC (d34y[j0]);
				if (ok_to_use[68]) ss = SS_068, tt_inc[68] = GET_TT_INC (d34x[j0]);
				if (ok_to_use[69]) ss = SS_069, tt_inc[69] = GET_TT_INC (d34x[j0]);
				if (ok_to_use[70]) ss = SS_070, tt_inc[70] = GET_TT_INC (d34x[j0]);
				if (ok_to_use[71]) ss = SS_071, tt_inc[71] = GET_TT_INC (d34x[j0]);

				/* Do sqrt(37) nodes */

				if (ok_to_use[72]) ss = SS_072, tt_inc[72] = GET_TT_INC (d37y[j0]);
				if (ok_to_use[73]) ss = SS_073, tt_inc[73] = GET_TT_INC (d37y[j0]);
				if (ok_to_use[74]) ss = SS_074, tt_inc[74] = GET_TT_INC (d37y[j0]);
				if (ok_to_use[75]) ss = SS_075, tt_inc[75] = GET_TT_INC (d37y[j0]);
				if (ok_to_use[76]) ss = SS_076, tt_inc[76] = GET_TT_INC (d37x[j0]);
				if (ok_to_use[77]) ss = SS_077, tt_inc[77] = GET_TT_INC (d37x[j0]);
				if (ok_to_use[78]) ss = SS_078, tt_inc[78] = GET_TT_INC (d37x[j0]);
				if (ok_to_use[79]) ss = SS_079, tt_inc[79] = GET_TT_INC (d37x[j0]);

				/* Do sqrt(41) nodes */

				if (ok_to_use[80]) ss = SS_080, tt_inc[80] = GET_TT_INC (d41y[j0]);
				if (ok_to_use[81]) ss = SS_081, tt_inc[81] = GET_TT_INC (d41y[j0]);
				if (ok_to_use[82]) ss = SS_082, tt_inc[82] = GET_TT_INC (d41y[j0]);
				if (ok_to_use[83]) ss = SS_083, tt_inc[83] = GET_TT_INC (d41y[j0]);
				if (ok_to_use[84]) ss = SS_084, tt_inc[84] = GET_TT_INC (d41x[j0]);
				if (ok_to_use[85]) ss = SS_085, tt_inc[85] = GET_TT_INC (d41x[j0]);
				if (ok_to_use[86]) ss = SS_086, tt_inc[86] = GET_TT_INC (d41x[j0]);
				if (ok_to_use[87]) ss = SS_087, tt_inc[87] = GET_TT_INC (d41x[j0]);

				/* Do sqrt(58) nodes */

				if (ok_to_use[88]) ss = SS_088, tt_inc[88] = GET_TT_INC (d58y[j0]);
				if (ok_to_use[89]) ss = SS_089, tt_inc[89] = GET_TT_INC (d58y[j0]);
				if (ok_to_use[90]) ss = SS_090, tt_inc[90] = GET_TT_INC (d58y[j0]);
				if (ok_to_use[91]) ss = SS_091, tt_inc[91] = GET_TT_INC (d58y[j0]);
				if (ok_to_use[92]) ss = SS_092, tt_inc[92] = GET_TT_INC (d58x[j0]);
				if (ok_to_use[93]) ss = SS_093, tt_inc[93] = GET_TT_INC (d58x[j0]);
				if (ok_to_use[94]) ss = SS_094, tt_inc[94] = GET_TT_INC (d58x[j0]);
				if (ok_to_use[95]) ss = SS_095, tt_inc[95] = GET_TT_INC (d58x[j0]);

				/* Do sqrt(65) nodes */

				if (ok_to_use[96])  ss = SS_096, tt_inc[96]  = GET_TT_INC (d65y[j0]);
				if (ok_to_use[97])  ss = SS_097, tt_inc[97]  = GET_TT_INC (d65y[j0]);
				if (ok_to_use[98])  ss = SS_098, tt_inc[98]  = GET_TT_INC (d65y[j0]);
				if (ok_to_use[99])  ss = SS_099, tt_inc[99]  = GET_TT_INC (d65y[j0]);
				if (ok_to_use[100]) ss = SS_100, tt_inc[100] = GET_TT_INC (d65x[j0]);
				if (ok_to_use[101]) ss = SS_101, tt_inc[101] = GET_TT_INC (d65x[j0]);
				if (ok_to_use[102]) ss = SS_102, tt_inc[102] = GET_TT_INC (d65x[j0]);
				if (ok_to_use[103]) ss = SS_103, tt_inc[103] = GET_TT_INC (d65x[j0]);

				/* Do sqrt(85) nodes */

				if (ok_to_use[104]) ss = SS_104, tt_inc[104] = GET_TT_INC (d85y[j0]);
				if (ok_to_use[105]) ss = SS_105, tt_inc[105] = GET_TT_INC (d85y[j0]);
				if (ok_to_use[106]) ss = SS_106, tt_inc[106] = GET_TT_INC (d85y[j0]);
				if (ok_to_use[107]) ss = SS_107, tt_inc[107] = GET_TT_INC (d85y[j0]);
				if (ok_to_use[108]) ss = SS_108, tt_inc[108] = GET_TT_INC (d85x[j0]);
				if (ok_to_use[109]) ss = SS_109, tt_inc[109] = GET_TT_INC (d85x[j0]);
				if (ok_to_use[110]) ss = SS_110, tt_inc[110] = GET_TT_INC (d85x[j0]);
				if (ok_to_use[111]) ss = SS_111, tt_inc[111] = GET_TT_INC (d85x[j0]);

				/* Do sqrt(85) nodes */

				if (ok_to_use[112]) ss = SS_112, tt_inc[112] = GET_TT_INC (d170y[j0]);
				if (ok_to_use[113]) ss = SS_113, tt_inc[113] = GET_TT_INC (d170y[j0]);
				if (ok_to_use[114]) ss = SS_114, tt_inc[114] = GET_TT_INC (d170y[j0]);
				if (ok_to_use[115]) ss = SS_115, tt_inc[115] = GET_TT_INC (d170y[j0]);
				if (ok_to_use[116]) ss = SS_116, tt_inc[116] = GET_TT_INC (d170x[j0]);
				if (ok_to_use[117]) ss = SS_117, tt_inc[117] = GET_TT_INC (d170x[j0]);
				if (ok_to_use[118]) ss = SS_118, tt_inc[118] = GET_TT_INC (d170x[j0]);
				if (ok_to_use[119]) ss = SS_119, tt_inc[119] = GET_TT_INC (d170x[j0]);

#ifndef TTT120
			case 64:	/* Solution at extra full precision */
#endif

				/* Do sqrt(26) nodes */
			
				if (ok_to_use[48]) ss = SS_048, tt_inc[48] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[49]) ss = SS_049, tt_inc[49] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[50]) ss = SS_050, tt_inc[50] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[51]) ss = SS_051, tt_inc[51] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[52]) ss = SS_052, tt_inc[52] = GET_TT_INC (d26x[j0]);
				if (ok_to_use[53]) ss = SS_053, tt_inc[53] = GET_TT_INC (d26x[j0]);
				if (ok_to_use[54]) ss = SS_054, tt_inc[54] = GET_TT_INC (d26x[j0]);
				if (ok_to_use[55]) ss = SS_055, tt_inc[55] = GET_TT_INC (d26x[j0]);

				/* Do sqrt(29) nodes */
			
				if (ok_to_use[56]) ss = SS_056, tt_inc[56] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[57]) ss = SS_057, tt_inc[57] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[58]) ss = SS_058, tt_inc[58] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[59]) ss = SS_059, tt_inc[59] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[60]) ss = SS_060, tt_inc[60] = GET_TT_INC (d29x[j0]);
				if (ok_to_use[61]) ss = SS_061, tt_inc[61] = GET_TT_INC (d29x[j0]);
				if (ok_to_use[62]) ss = SS_062, tt_inc[62] = GET_TT_INC (d29x[j0]);
				if (ok_to_use[63]) ss = SS_063, tt_inc[63] = GET_TT_INC (d29x[j0]);

#ifndef TTT120
			case 48:	/* Solution at high precision */
#endif

				/* Do sqrt(17) nodes */
			
				if (ok_to_use[32]) ss = SS_032, tt_inc[32] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[33]) ss = SS_033, tt_inc[33] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[34]) ss = SS_034, tt_inc[34] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[35]) ss = SS_035, tt_inc[35] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[36]) ss = SS_036, tt_inc[36] = GET_TT_INC (d17x[j0]);
				if (ok_to_use[37]) ss = SS_037, tt_inc[37] = GET_TT_INC (d17x[j0]);
				if (ok_to_use[38]) ss = SS_038, tt_inc[38] = GET_TT_INC (d17x[j0]);
				if (ok_to_use[39]) ss = SS_039, tt_inc[39] = GET_TT_INC (d17x[j0]);

				/* Do sqrt(25) nodes */
			
				if (ok_to_use[40]) ss = SS_040, tt_inc[40] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[41]) ss = SS_041, tt_inc[41] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[42]) ss = SS_042, tt_inc[42] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[43]) ss = SS_043, tt_inc[43] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[44]) ss = SS_044, tt_inc[44] = GET_TT_INC (d25x[j0]);
				if (ok_to_use[45]) ss = SS_045, tt_inc[45] = GET_TT_INC (d25x[j0]);
				if (ok_to_use[46]) ss = SS_046, tt_inc[46] = GET_TT_INC (d25x[j0]);
				if (ok_to_use[47]) ss = SS_047, tt_inc[47] = GET_TT_INC (d25x[j0]);

#ifndef TTT120
			case 32:	/* Solution at intermediate precision */
#endif

				/* Do sqrt(10) nodes */
			
				if (ok_to_use[16]) ss = SS_016, tt_inc[16] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[17]) ss = SS_017, tt_inc[17] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[18]) ss = SS_018, tt_inc[18] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[19]) ss = SS_019, tt_inc[19] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[20]) ss = SS_020, tt_inc[20] = GET_TT_INC (d10x[j0]);
				if (ok_to_use[21]) ss = SS_021, tt_inc[21] = GET_TT_INC (d10x[j0]);
				if (ok_to_use[22]) ss = SS_022, tt_inc[22] = GET_TT_INC (d10x[j0]);
				if (ok_to_use[23]) ss = SS_023, tt_inc[23] = GET_TT_INC (d10x[j0]);
			
				/* Do sqrt(13) nodes */
			
				if (ok_to_use[24]) ss = SS_024, tt_inc[24] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[25]) ss = SS_025, tt_inc[25] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[26]) ss = SS_026, tt_inc[26] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[27]) ss = SS_027, tt_inc[27] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[28]) ss = SS_028, tt_inc[28] = GET_TT_INC (d13x[j0]);
				if (ok_to_use[29]) ss = SS_029, tt_inc[29] = GET_TT_INC (d13x[j0]);
				if (ok_to_use[30]) ss = SS_030, tt_inc[30] = GET_TT_INC (d13x[j0]);
				if (ok_to_use[31]) ss = SS_031, tt_inc[31] = GET_TT_INC (d13x[j0]);

#ifndef TTT120
			case 16:	/* Solution at low precision */
#endif

				/* Then do sqrt(5) nodes */
	
				if (ok_to_use[8])  ss = SS_008, tt_inc[8]  = GET_TT_INC (d5y[j0]);
				if (ok_to_use[9])  ss = SS_009, tt_inc[9]  = GET_TT_INC (d5y[j0]);
				if (ok_to_use[10]) ss = SS_010, tt_inc[10] = GET_TT_INC (d5y[j0]);
				if (ok_to_use[11]) ss = SS_011, tt_inc[11] = GET_TT_INC (d5y[j0]);
				if (ok_to_use[12]) ss = SS_012, tt_inc[12] = GET_TT_INC (d5x[j0]);
				if (ok_to_use[13]) ss = SS_013, tt_inc[13] = GET_TT_INC (d5x[j0]);
				if (ok_to_use[14]) ss = SS_014, tt_inc[14] = GET_TT_INC (d5x[j0]);
				if (ok_to_use[15]) ss = SS_015, tt_inc[15] = GET_TT_INC (d5x[j0]);
			
#ifndef TTT120
			case 8:	/* Solution at crude precision */
#endif

				/* First do sqrt(1) nodes */
			
				if (ok_to_use[0]) ss = SS_000, tt_inc[0] = GET_TT_INC (dx[j0]);
				if (ok_to_use[1]) ss = SS_001, tt_inc[1] = GET_TT_INC (dx[j0]);
				if (ok_to_use[2]) ss = SS_002, tt_inc[2] = GET_TT_INC (dy);
				if (ok_to_use[3]) ss = SS_003, tt_inc[3] = GET_TT_INC (dy);
			
				/* Then do sqrt(2) nodes */
			
				if (ok_to_use[4]) ss = SS_004, tt_inc[4] = GET_TT_INC (dr[j0]);
				if (ok_to_use[5]) ss = SS_005, tt_inc[5] = GET_TT_INC (dr[j0]);
				if (ok_to_use[6]) ss = SS_006, tt_inc[6] = GET_TT_INC (dr[j0]);
				if (ok_to_use[7]) ss = SS_007, tt_inc[7] = GET_TT_INC (dr[j0]);
#ifndef TTT120
		}
#endif

		/* Here, all incremental travel times that may be considered have been set */

		/* Store current value and look for fastest path to neigboring nodes */

		s[ij] = (float)(UNIT_TO_HOUR * current_tt);
		n_left--;	/* Because last node is set after the loop */
	
#ifdef TTT120
		for (k = 0; k < N_CALC; k++) {		/* Add increment in travel times to all surrounding nodes */
#else
		for (k = 0; k < n_nodes; k++) {		/* Add increment in travel times to all surrounding nodes */
#endif
			if (!ok_to_use[k]) continue;	/* Node is on land or has NaN - skip */
			if (h->periodic) {		/* Carefully handle periodicity in longitude and wrap over poles */
				if ((i_k = i0 + ip[k]) < 0)
					i_k += nx1;	/* Jump to east boundary */
				else if (i_k >= h->nx)
					i_k -= nx1;	/* Jump to west boundary */
				j_k = j0 - jp[k];	/* -ve becaues jp is positive up */
				/* For polar wrap, reflect latitude and phase-shift longitude by 180 */
				if (h->wrap_n && j_k < 0) {	/* N polar wrap */
					j_k = adjustment - j_k;
					i_k = (i_k + nx_half) % h->nx;
				}
				else if (h->wrap_s && j_k >= h->ny) { /* S polar wrap */
					j_k = ew_wrap - j_k;
					i_k = (i_k + nx_half) % h->nx;
				}
				kk = IJ(i_k,j_k,row_width,N_PAD);
			}
			else
				kk = ij + p[k];		/* Rectangular domain with NaN padding */
						
			if (tt_min[kk] > OCEAN) continue;	/* Node already propagated to or on land - skip */
			if (labs(tt_inc[k]) > tt_inc_max) tt_inc_max = labs(tt_inc[k]);
			else if (labs(tt_inc[k]) < tt_inc_min) tt_inc_min = labs(tt_inc[k]);
			next_tt = current_tt - tt_inc[k];	/* minus, because tt_inc is negative since ss is negative */
#if 0
			if (next_tt < 0) {
				fprintf (stderr, "Got NaN tt_inc[%ld]\n", k);
				set = 0;
				ss = SS_002; fprintf (stderr, "SS_002 = %g \n", ss);
				ss = SS_004; fprintf (stderr, "SS_004 = %g \n", ss);
				ss = SS_012; fprintf (stderr, "SS_012 = %g \n", ss);
				ss = SS_020; fprintf (stderr, "SS_020 = %g \n", ss);
				ss = SS_036; fprintf (stderr, "SS_036 = %g \n", ss);
				ss = SS_052; fprintf (stderr, "SS_052 = %g \n", ss);
				ss = SS_076; fprintf (stderr, "SS_076 = %g \n", ss);
				ss = s[ij+p[122]]; fprintf (stderr, "SS_122 = %g \n", ss);
				ss = s[ij+p[130]]; fprintf (stderr, "SS_130 = %g \n", ss);
				ss = s[ij+p[134]]; fprintf (stderr, "SS_134 = %g \n", ss);
				ss = s[ij+p[150]]; fprintf (stderr, "SS_150 = %g \n", ss);
			}
#endif			
			if (tt_min[kk] == OCEAN) {					/* First time visiting this node */
				treeinsert (next_tt, kk, list_head, list_tail);		/* Insert new entry into our tree */
				tt_min[kk] = next_tt;
				n_insert++;
			}
			else if (next_tt < tt_min[kk]) {				/* Faster than previous - replace with new value */
				treedelete (tt_min[kk], kk, list_head, list_tail);	/* Remove old entry */
				treeinsert (next_tt, kk, list_head, list_tail);		/* Insert new entry into our tree */
				tt_min[kk] = next_tt;
			}				
		}

		/* Here, the first entry in the tree is the one with shortest travel-time */
		 
		if (list_head->r == list_tail) {	/* Premature exit - some landlocked nodes never reached */
			fprintf (stderr, "\nttt: Warning: %" TTT_LL "d Landlocked nodes not reached\n", n_left);
			n_left = 0;	/* This will let us exit the do-while loop */
		}
		else {	/* Get the next smallest travel time node */
			current_tt = treesmallest (list_head, list_tail, &ij);	/* Get smallest TT... */
			treedelete (current_tt, ij, list_head, list_tail);	/* ...and remove this entry */
			tt_min[ij] = USED;					/* Mark this node as used (i.e., propagated to) */
			j0 = (ij / row_width) - N_PAD;
			i0 = (ij % row_width) - N_PAD;
			n_insert--;
		}
		if (verbose) {	/* Update progress display */
			set++;
			percent = (TTT_LONG) (c * set + 0.5);
			if (percent != last_percent) {
				fprintf (stderr, "ttt: Completed %3" TTT_LL "d %%\r", percent);
				last_percent = percent;
			}
		}
#ifdef TREE_DEBUG
		fprintf (fp, "%" TTT_LL "d\t%" TTT_LL "d\t%" TTT_LL "d\t%" TTT_LL "d\n", i0, j0, ij, n_insert);
#endif
	
	} while (n_left);	/* Used to be while (list_head->r != list_tail) but sometimes we have dead ends in propagation (up a fjord) and may run out of nodes */
#ifdef TREE_DEBUG
	fclose (fp);
#endif
	if (ij >= 0) s[ij] = (float)(UNIT_TO_HOUR * current_tt);	/* Last node to be set */

	if (ignore_east) {	/* Set the right side if a repeating yet unassigned column */
		for (j = k = 0; j < col_height; j++, k += row_width) s[k+i360] = s[k+N_PAD];;	/* Set east node == west node along border */
	}
	
#ifdef BUILD
	{	/* Used to dump out the (x,y,NaN) values not reached by propagation */
		double x0, y0;
		FILE *fp = fopen ("bad.d", "w");
		for (j = 0; j < h->ny; j++) {
			y0 = ttt_j_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
			for (i = 0; i < h->nx; i++) {
				ij = IJ(i, j, row_width, N_PAD);
				if (s[ij] < 0.0) {
					x0 = ttt_i_to_x (i, h->wesn[XLO], h->wesn[XHI], h->inc[TTT_X], h->xy_off, h->nx);
					fprintf (fp, "%g\t%g\tNaN\n", x0, y0);
				}
			}
		}
		fclose (fp);
	}
#endif
	/* Set uninitialized nodes (i.e., lakes not reached by wave) to not-a-number */

	for (k = 0; k < nm_grid; k++) if (s[k] < 0.0) s[k] = TTT_NaN;
	
	free ((void *)tt_min);
	
	if (verbose) fprintf (stderr, "ttt: Completed %3" TTT_LL "d %%\n", percent);
	
	fprintf(stderr, "tt_inc min/max = %" TTT_LL "d/%" TTT_LL "d\n", tt_inc_min, tt_inc_max);
}

/* Initialize header and save the calculated travel times */

static void ttt_store_ttt (char *file_name, float *tt, struct GRD_HEADER *h, char *source_file, double *q_lon, double *q_lat, TTT_LONG n_sources, int mm, int dd, int yy, int hh, int mi, int ss, TTT_LONG remove_bias, double ttt_rate, TTT_LONG n_nodes, TTT_LONG out_format, TTT_LONG t_fmt, int argc, char **argv, TTT_LONG verbose)
{
	TTT_LONG i, j, k, string_len, row_width, n_trunc = 0, decasec;
	short *short_row = NULL;
	double x = 0.0, y = 0.0, f = 1.0;
	float TTT_NaN;
	char file[BUFSIZ];
	char *format[3] = {" (local)", " (UTC)", ""};
	FILE *fp = NULL;

	TTT_make_NaN (TTT_NaN);

	if (remove_bias) {
		/* f is set so that the n-sided polygon has same area as a circle */
		f = 2 * M_PI / n_nodes;
		f = sqrt ( sin (f) / f);
		if (verbose) fprintf (stderr, "Normalization factor = %g\n", f);
	}

	if (source_file) {	/* Get average location */
		for (i = 0; i < n_sources; i++) x += q_lon[i], y += q_lat[i];
		x /= n_sources;
		y /= n_sources;
	}
	else {
		x = q_lon[0];
		y = q_lat[0];
	}
	if (x > 180.0) x -= 360.0;	/* Report in -180/180 range */
	if (yy == 0) t_fmt = 2;	/* No time given */
	sprintf (h->title, "Tsunami Travel Times from ttt %s", TTT_VERSION);
	sprintf (h->remark, "%g %g %d %d %d %d %d %d %g [x, y, M, D, Y, hh, mm, ss%s, rate (s/km)]", x, y, mm, dd, yy, hh, mi, ss, ttt_rate, format[t_fmt]);
	strcpy (h->x_units,"longitude [degrees_east]");
	strcpy (h->y_units,"latitude [degrees_north]");
	strcpy (h->z_units, "hour");
	if (out_format == 1) {	/* Write 2-byte shorts instead of 4-byte floats but scale_factor has been set to decode deca-seconds */
		short_row = (short int *)ttt_memory (NULL, h->nx, sizeof (short int), "ttt");
	}
	strcpy (h->command, argv[0]);
	string_len = (TTT_LONG)strlen (h->command);
	for (j = 1; string_len < GRD_COMMAND_LEN && j < argc; j++) {
		string_len += (TTT_LONG)strlen (argv[j]) + 1;
		if (string_len > GRD_COMMAND_LEN) continue;
		strcat (h->command, " ");
		strcat (h->command, argv[j]);
	}
	h->command[string_len] = 0;	
	row_width = h->nx + 2 * N_PAD;
	if (h->wesn[XHI] > 360.0) h->wesn[XLO] -= 360.0, h->wesn[XHI] -= 360.0;
	h->z_min = +DBL_MAX;
	h->z_max = -DBL_MAX;
	for (j = 0; j < h->ny; j++) for (i = 0, k = IJ(0,j,row_width,N_PAD); i < h->nx; i++, k++) {
		if (TTT_is_fnan (tt[k])) continue;
		if (remove_bias) tt[k] *= (float)f;
		h->z_min = MIN (h->z_min, tt[k]);
		h->z_max = MAX (h->z_max, tt[k]);
	}
	if (out_format == 1) {	/* Convert to deca-seconds */
		h->z_min = (double) irint (h->z_min * 360.0);
		h->z_max = (double) irint (h->z_max * 360.0);
		h->z_scale_factor = 1.0 / 360.0;
		fprintf (stderr, "ttt: Convert hours to deca-seconds to fit in 2-byte integers\n");
	}	
	sprintf (file, "%s.%s", file_name, TTT_ext[out_format]);
	if (verbose) fprintf (stderr, "Create file %s\n", file);
	if ((fp = fopen (file, TTT_wmode[out_format])) == NULL) {
		fprintf (stderr, "ttt: Error creating file %s\n", file);
		exit (22);
	}
	if (out_format >= 2) {	/* Write ESRI header and maybe open separate flt file */
		char *order[2] = {"LSBFIRST", "MSBFIRST"}, *qqq[2] = {"center", "corner"};
		int c = '\n';
		fprintf (fp, "ncols         %d\nnrows         %d\n", h->nx, h->ny);
		fprintf (fp, "xll%s     %.12f%cyll%s     %.12f\n", qqq[h->registration], h->wesn[XLO], c, qqq[h->registration], h->wesn[YLO]);
		fprintf (fp, "cellsize      %.18f\n", h->inc[TTT_X]);
		fprintf (fp, "NODATA_value  %g\n", ESRI_NAN);
		if (out_format == 2) {	/* ESRI binary */
			fprintf (fp, "byteorder     %s\n", order[ENDIAN]);
			fclose (fp);	/* Done with header file, now open binary file */
			sprintf (file, "%s.flt", file_name);
			if (verbose) fprintf (stderr, "Create file %s\n", file);
			if ((fp = fopen (file, "wb")) == NULL) {
				fprintf (stderr, "ttt: Error creating file %s\n", file);
				exit (22);
			}
		}
	}

	/* Write GMT native header */
	if (out_format < 2 && (fwrite ((void *)&h->nx, 3*sizeof (int), (size_t)1, fp) != 1 || fwrite ((void *)h->wesn, sizeof (struct GRD_HEADER) - ((long)h->wesn - (long)&h->nx), (size_t)1, fp) != 1)) {
		fprintf (stderr, "ttt: Error writing header for file %s\n", file_name);
		exit (23);
	}

	for (j = 0, k = N_PAD * row_width + N_PAD; j < h->ny; j++, k += row_width) {
		if (out_format == 1) {	/* Short int binary format */
			for (i = 0; i < h->nx; i++) {
				if (TTT_is_fnan (tt[k+i]))
					short_row[i] = SHRT_MIN;	/* Replace with usual GMT NaN indicator for short ints */
				else {
					decasec = irint (360.0 * tt[k+i]);	/* Convert hours to deca-seconds */
					if (decasec >= SHRT_MAX) {
						short_row[i] = SHRT_MIN;
						n_trunc++;
					}
					else
						short_row[i] = decasec;
				}
			}
			if (fwrite ((void *)short_row, sizeof (short int), h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error writing file %s\n", file_name);
				exit (23);
			}
		}
		else {
			if (out_format >= 2) {	/* 4-byte float format */
				for (i = 0; i < h->nx; i++) {
					if (TTT_is_fnan (tt[k+i])) tt[k+i] = ESRI_NAN;
					if (out_format == 3) {	/* Ascii exchange format */
						fprintf (fp, "%g", tt[k+i]);
						if (i < (h->nx-1)) fprintf (fp, " ");
					}
				}
			}
			if (out_format == 3)
				fprintf (fp, "\n");
			else if (fwrite ((void *)&tt[k], sizeof (float), h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error writing file %s\n", file_name);
				exit (23);
			}
		}
	}
	fclose (fp);
	if (out_format == 1) {
		free ((void *)short_row);
		if (n_trunc) fprintf (stderr, "ttt: Warning: %" TTT_LL "d points exceeded short int range - set to NaN (%d)\n", n_trunc, SHRT_MIN);
	}
}

/* Free memory used by real-time huygens construction */
static void ttt_free_memory (TTT_LONG n_nodes)
{
	free ((void *)dx);
	free ((void *)dr);
	
	if (n_nodes > 8) {
		free ((void *)d5x);
		free ((void *)d5y);
	}
	if (n_nodes > 16) {
		free ((void *)d10x);
		free ((void *)d10y);
		free ((void *)d13x);
		free ((void *)d13y);
	}
	if (n_nodes > 32) {
		free ((void *)d17x);
		free ((void *)d17y);
		free ((void *)d25x);
		free ((void *)d25y);
	}
	if (n_nodes > 48) {
		free ((void *)d26x);
		free ((void *)d26y);
		free ((void *)d29x);
		free ((void *)d29y);
	}
	if (n_nodes > 64) {
		free ((void *)d34x);
		free ((void *)d34y);
		free ((void *)d37x);
		free ((void *)d37y);
		free ((void *)d41x);
		free ((void *)d41y);
		free ((void *)d58x);
		free ((void *)d58y);
		free ((void *)d65x);
		free ((void *)d65y);
		free ((void *)d85x);
		free ((void *)d85y);
		free ((void *)d170x);
		free ((void *)d170y);
	}
}

static double ttt_tslope (struct GRD_HEADER *h, float *tt, TTT_LONG i_quake, TTT_LONG j_quake)
{
	/* Calculate the average change in travel time with distance away from the node.
	 * This will be used as a first-order uncertainty term in travel time given an
	 * uncertainty in epicenter location.
	 * This version only applies to the quake location whose tt == 0.
	 */

	TTT_LONG i, j, ij, imin, imax, jmin, jmax, row_width;
	double search_radius = 0.25, dx, x0, y0, x, y, d, sr = 0.0, st = 0.0, ttt_rate;
	row_width = h->nx + 2 * N_PAD;
		
	y0 = ttt_j_to_y (j_quake, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
	x0 = ttt_i_to_x (i_quake, h->wesn[XLO], h->wesn[XHI], h->inc[TTT_X], h->xy_off, h->nx);
	dx = h->inc[TTT_X] * cos (D2R * y0);
	imin = MAX (0, i_quake - (TTT_LONG)ceil (search_radius / dx));
	imax = MIN (h->nx - 1, i_quake + (TTT_LONG)ceil (search_radius / dx));
	jmin = MAX (0, j_quake - (TTT_LONG)ceil (search_radius / h->inc[TTT_Y]));
	jmax = MIN (h->ny - 1, j_quake + (TTT_LONG)ceil (search_radius / h->inc[TTT_Y]));
		
	for (j = jmin; j <= jmax; j++) {
		y = ttt_j_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
		for (i = imin; i <= imax; i++) {
			ij = IJ(i, j, row_width, N_PAD);
			if (TTT_is_fnan (tt[ij])) continue;
			x = ttt_i_to_x (i, h->wesn[XLO], h->wesn[XHI], h->inc[TTT_X], h->xy_off, h->nx);
			d = ttt_great_circle_dist (x0, y0, x, y);
			if (d > search_radius) continue;
			/* OK, inside 0.25 degree circle, tally up */
			sr += d;
			st += tt[ij];
		}
	}
	
	ttt_rate = (sr > 0.0) ? 3600.0 * (st / sr) * (R2D / 6371.008) : 0.0;	/* Convert to sec/km */
	
	return (ttt_rate);
}

/* Allocates space and reads bathymetry from short int bathymetry file (GMT format # 2) */

static void ttt_read_bathymetry (char *file_name, struct GRD_HEADER *h, float **s, double west, double east, double south, double north, TTT_LONG file_format, TTT_LONG swabbing)
{	/* file_format = 1 (float) or 2 (short int) */
	TTT_LONG i, j, k, p, half, i180, adjustment, skip_x, skip_y, wrap_i, nx, ny, row_width, col_height;
	unsigned int *i_ptr = NULL, wrap_check = 0, periodic = 0;
	size_t item_size;
	/* size_t len = strlen (file_name); */
	short int *s_record = NULL;
	float *f_record = NULL;
	FILE *fp = NULL;
	float TTT_NaN;
	TTT_make_NaN (TTT_NaN);

	if ((fp = fopen (file_name, "rb")) == NULL) {
		fprintf (stderr, "ttt: Error creating file %s\n", file_name);
		exit (17);
	}

	if (fread ((void *)&h->nx, 3*sizeof (int), (size_t)1, fp) != 1 || fread ((void *)&h->wesn[XLO], sizeof (struct GRD_HEADER) - ((long)&h->wesn[XLO] - (long)&h->nx), (size_t)1, fp) != 1) {
		fprintf (stderr, "ttt: Error reading header from file %s\n", file_name);
		exit (18);
	}
	
	if (west == east && south == north) {
		west  = h->wesn[XLO];	east  = h->wesn[XHI];
		south = h->wesn[YLO];	north = h->wesn[YHI];
	}
	else {
		while (west > h->wesn[XLO]) west -= 360.0, east -= 360.0;
		while (west < h->wesn[XLO]) west += 360.0, east += 360.0;
		periodic = (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < CONV_LIMIT);	/* Periodic in 360 longitude */
	}
	nx = irint((east-west)/h->inc[TTT_X]) + !h->registration;
	ny = irint((north-south)/h->inc[TTT_Y]) + !h->registration;
	skip_x = irint((west-h->wesn[XLO])/h->inc[TTT_X]);
	skip_y = irint((h->wesn[YHI] - north)/h->inc[TTT_Y]);
	wrap_check = (skip_x && periodic && h->registration == 0);	/* Must loop out for the repeating node value at west/east */
	row_width  = nx + 2 * N_PAD;
	col_height = ny + 2 * N_PAD;

	if (sizeof (TTT_LONG) == 4) {	/* 32-bit compilation */
		if (((((double)row_width) * ((double)col_height) * sizeof (float)) / pow (2.0, 32.0)) >= 1.0) {
			fprintf (stderr, "ttt: File %s requires more memory than is available in 32-bit mode.\n", file_name);
			fprintf (stderr, "ttt: Recompile and run in 64-bit mode.\n");
			exit (24);
		}
	}
	*s = (float *)ttt_memory (NULL, row_width * col_height, sizeof (float), "ttt");
	if (file_format == 1) {
		item_size = sizeof (float);
		f_record = (float *)ttt_memory (NULL, h->nx, item_size, "ttt");
	}
	else {
		item_size = sizeof (short int);
		s_record = (short int *)ttt_memory (NULL, h->nx, item_size, "ttt");
	}

	if (skip_y) fseek (fp, (long)(skip_y * h->nx * item_size), SEEK_CUR);
	if (file_format == 1) {
		for (j = 0; j < ny; j++) {
			if (fread ((void *)f_record, item_size, h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error reading file %s\n", file_name);
				exit (18);
			}
			k = IJ(0,j,row_width,N_PAD);
			for (i = 0; i < nx; i++, k++) {
				wrap_i = skip_x + i;	/* This may march off the right end of grid, hence next 2 lines: */
				if (wrap_i >= h->nx && wrap_check) wrap_i++;	/* Wrapping around periodic gridline-registrered grid we must skip the duplicate repeating node */
				p = wrap_i % h->nx;	/* Cannot become zero when wrap_check is true */
				if (swabbing) {
					i_ptr = (unsigned int *)&f_record[p];
					*i_ptr = TTT_swab4 (*i_ptr);
				}
				(*s)[k] = f_record[p];
			}
		}
		free ((void *)f_record);
	}
	else {
		for (j = 0; j < ny; j++) {
			if (fread ((void *)s_record, item_size, h->nx, fp) != (size_t)h->nx) {
				fprintf (stderr, "ttt: Error reading file %s\n", file_name);
				exit (18);
			}
			k = IJ(0,j,row_width,N_PAD);
			for (i = 0; i < nx; i++, k++) {
				wrap_i = skip_x + i;	/* This may march off the right end of grid, hence next 2 lines: */
				if (wrap_i >= h->nx && wrap_check) wrap_i++;	/* Wrapping around periodic gridline-registrered grid we must skip the duplicate repeating node */
				p = wrap_i % h->nx;	/* Cannot become zero when wrap_check is true */
				if (swabbing) s_record[p] = TTT_swab2 (s_record[p]);
				(*s)[k] = (s_record[p] == SHRT_MAX || s_record[p] == SHRT_MIN) ? TTT_NaN : (float) s_record[p];
			}
		}
		free ((void *)s_record);
	}
	fclose (fp);
	
	/* Modify header if subset */
	
	h->wesn[XLO] = west;	h->wesn[XHI] = east;	h->nx = (unsigned int)nx;
	h->wesn[YLO] = south;	h->wesn[YHI] = north;	h->ny = (unsigned int)ny;
	
	/* Apply BC to the strips */
	
	adjustment = (h->registration) ? 1 : 0;	/* Adjust if pixel registration */
	if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < CONV_LIMIT) {	/* Periodic in 360 longitude */
		h->periodic = TRUE;
		for (j = 0; j < h->ny; j++) {
			k = IJ(0,j,row_width,N_PAD);	/* First point inside grid at west side */
			for (p = 1; p <= N_PAD; p++) {
				(*s)[k+h->nx+p-1] = (*s)[k+p-adjustment];	/* Set east pad = west inside */
				(*s)[k-p] = (*s)[k+h->nx-p-1+adjustment];	/* Set west pad = east inside */
			}
		}
	}
	half = h->nx / 2;
	if (fabs (h->wesn[YHI] - 90.0) < CONV_LIMIT && h->periodic) {	/* N pole wrap */
		h->wrap_n = TRUE;
		k = IJ(0,0,row_width,N_PAD);
		for (i = -N_PAD; i < (h->nx + N_PAD); i++) {
			i180 = i + half;
			if (i180 >= row_width) i180 -= row_width;
			for (p = 1; p <= N_PAD; p++) (*s)[k+i-p*row_width] = (*s)[k+i180+(p-adjustment)*row_width];
		}
	}
	if (fabs (h->wesn[YLO] + 90.0) < CONV_LIMIT && h->periodic) {	/* S pole wrap */
		h->wrap_s = TRUE;
		k = IJ(0,h->ny-1,row_width,N_PAD);
		for (i = -N_PAD; i < (h->nx + N_PAD); i++) {
			i180 = i + half;
			if (i180 >= row_width) i180 -= row_width;
			for (p = 1; p <= N_PAD; p++) (*s)[k+i+p*row_width] = (*s)[k+i180-(p-adjustment)*row_width];
		}
	}
}

int main (int argc, char **argv)
{
	/* LOCAL VARIABLE DECLARATIONS (see ttt.h for global variables and definitions) */

	TTT_LONG i, j, n;		/* Misc. counters */
	TTT_LONG row_width;		/* row-dimension of extended grid with 13 boundary rows and columns */
	TTT_LONG *i_quake = NULL, *j_quake = NULL;	/* column and row number of current source point */
	TTT_LONG ip[M_ACCESS];		/* Relative grid i offsets from current node to all 256 neighbors */
	TTT_LONG jp[M_ACCESS];		/* Relative grid j offsets from current node to all 256 neighbors */
	TTT_LONG p[M_ACCESS];		/* Relative grid ij offsets from current node to all 256 neighbors */
	TTT_LONG n_sources = 0;		/* Number of point sources making up the fault zone [1] */
	TTT_LONG n_nodes = N_CALC;	/* Number of nodes to use in the Huygens construction */
	TTT_LONG n_files = 0;		/* Number of input files given.  Must equal 1 for ttt to run */
	TTT_LONG n_alloc = 1;		/* Allocated size of quake arrays */
	TTT_LONG *ij_quake = NULL;	/* Array holding the node-numbers of all point sources */
	TTT_LONG search_nearest = FALSE;	/* TRUE if ttt is allowed to move point-sources from land to nearest water node */
	TTT_LONG error = FALSE;		/* Error flag if something goes wrong */
	TTT_LONG verbose = FALSE;	/* If TRUE display progress information to screen */
	TTT_LONG quick = FALSE;		/* Give program synopsis and exit */
	TTT_LONG remove_bias = TRUE;	/* If TRUE will normalize travel times to remove error bias */
	TTT_LONG out_format = 0;	/* Default is to write 4-byte float GMT grdfiles */
	TTT_LONG max_nodes;		/* The maximum nodes used in this calculation (set at run-time) */
	TTT_LONG in_format = 2;		/* Default bathymetry files are short int (GMT format #2) */
	TTT_LONG swabbing;		/* 1 if we must do byte-swabbing on the fly */
	TTT_LONG t_format = T_UTC;	/* Default time format is UTC */

	int mm, dd, yy, hh, mi, sc;	/* Time of earthquake (month, day, year, hour, minute, sconds) */

	double *q_lon = NULL, *q_lat = NULL;	/* Arrays with coordinates of all point sources */
	double search_radius = RADIUS;	/* Search radius used to look for water nodes nearest land point-sources */
	double search_depth = 0.0;	/* The water node must be at least this deep */
	double depth_threshold = 0.0;	/* Minimum depth for any node */
	double ttt_rate;		/* The average increase in travel time (in sec) per km of distance from quake */
	double west, east;		/* Sub-region of grid to use */
	double south, north;		/* Sub-region of grid to use */

	float *tt = NULL;		/* Array holding travel time grid */
	char tttfile[BUFSIZ];		/* Name of output file with travel times */
	char bathy_file[BUFSIZ];	/* Name of file with bathymetry */
	char name_stem[BUFSIZ];		/* Prefix from which to compose .i2 file name */
	char *input_file = NULL;	/* Pointer to inputfile as given on command line */
	char *source_file = NULL;	/* Pointer to file with multiple source locations */
	char line[BUFSIZ];		/* Misc. text variables */
	char t_w[16], t_e[16];
	char t_s[16], t_n[16];
	char tlon[32], tlat[32];

	struct GRD_HEADER h;		/* Header structure with info on grid region and size */

	FILE *fp = NULL;		/* File pointer to current file */

	/* Initializations */

	mm = dd = yy = hh = mi = sc = 0;
	west = east = south = north = 0.0;
		
	q_lon = (double *) ttt_memory (NULL, n_alloc, sizeof (double), "ttt");
	q_lat = (double *) ttt_memory (NULL, n_alloc, sizeof (double), "ttt");
	
	name_stem[0] = tttfile[0] = '\0';
	if ((TTT_DIR = getenv ("TTT_DIR")) == NULL) {	/* Directory where all ttt files are expected to live */
		TTT_DIR = TTT_DIR_DEFAULT;
	}
		
	for (i = 1; !error && i < argc; i++) {	/* Check input arguments */

		if (argv[i][0] != '-') {	/* File name or prefix given */
			input_file = argv[i];
			n_files++;
			continue;
		}

		switch (argv[i][1]) {

			case '\0':	/* Give short synopsis */

				quick = error = TRUE;
				break;

			case 'B':	/* Do NOT scale travel times so that prediction error is ~unbiased */

				remove_bias = FALSE;
				break;

			case 'e':	/* Get file with multiple epicenters making up a non-point source */

				source_file = &argv[i][2];
				fp = fopen (source_file, "r");
				while (fgets (line, 512, fp) != NULL) {
					if (line[0] == '#' || line[0] == '>') continue;	/* Skip comments and multiseg headers */
					n = sscanf (line, "%s %s", tlon, tlat);
					if (n != 2) {
						fprintf (stderr, "ttt: Error reading source location record # %" TTT_LL "d\n", n_sources);
						exit (1);
					}
					q_lon[n_sources] = txt2fl (tlon, 'W');
					q_lat[n_sources] = txt2fl (tlat, 'S');
					n_sources++;
					if (n_sources == n_alloc) {
						n_alloc += CHUNK;
						q_lon = (double *) ttt_memory ((void *)q_lon, n_alloc, sizeof (double), "ttt");
						q_lat = (double *) ttt_memory ((void *)q_lat, n_alloc, sizeof (double), "ttt");
					}
				}
				fclose (fp);
				q_lon = (double *) ttt_memory ((void *)q_lon, n_sources, sizeof (double), "ttt");
				q_lat = (double *) ttt_memory ((void *)q_lat, n_sources, sizeof (double), "ttt");
				break;

			case 'D':
				depth_threshold = atof (&argv[i][2]);
				break;
				
			case 'E':	/* Get a single epicenter point source */

				n = sscanf (&argv[i][2], "%[^/]/%s", tlon, tlat);
				if (n != 2) {
					fprintf (stderr, "ttt: Error in -E: Error decoding quake location!\n");
					exit (2);
				}
				q_lon[0] = txt2fl (tlon, 'W');
				q_lat[0] = txt2fl (tlat, 'S');
				n_sources = 1;
				break;

			case 'I':	/* OBSOLETE: Use -Gi instead.  Writes 2-byte int (in decaseconds) file instead of 4-byte floats (in hours) */

				out_format = 1;
				break;

			case 'G':	/* Write ASCII or 4-byte binary float (in hours) ESRI Gridfloat files (header and raster) */

				switch (argv[i][2]) {
					case 'a':	/* ESRI ascii exchange format */
						out_format = 3;
						break;
					case 'b':	/* ESRI binary gridfloat format */
						out_format = 2;
						break;
					case 'i':	/* 2-byte int (in decaseconds) GMT native grid format */
						out_format = 1;
						break;
					case 'f':	/* 4-byte float (in hours) GMT native grid format [Default] */
						out_format = 0;
						break;
					default:
						fprintf (stderr, "ttt: Choose output formats from a, b, f, or i [f]!\n");
						exit (25);
						break;
				}
				break;

			case 'o':	/* Info about time of quake */
				t_format = T_LOCAL;	/* No break, just setting this parameter if -o is used instead of -O */
			case 'O':	/* Info about time of quake */

				n = sscanf (&argv[i][2], "%d/%d/%d/%d/%d/%d", &mm, &dd, &yy, &hh, &mi, &sc);
				if (n < 5) {
					fprintf (stderr, "ttt: Error in -O: Error decoding time of quake!\n");
					exit (3);
				}
				if ((mm < 1 || mm > 12) || (dd < 1 || dd > 31) || (hh < 0 || hh > 23) || (mi < 0 || mi > 59) || (sc < 0 || sc >= 60)) {
					fprintf (stderr, "ttt: Error in -O: time of quake out of range!\n");
					exit (4);
				}
				break;
#ifndef TTT120
			case 'N':	/* How many nodes to use in calculations [120] */

				n_nodes = atoi (&argv[i][2]);
				if (!(n_nodes == 8 || n_nodes == 16 || n_nodes == 32 || n_nodes == 48 || n_nodes == 64 || n_nodes == 120)) {
					fprintf (stderr, "ttt: Error in -N: Choose among 8, 16, 32, 48, 64, or 120!\n");
					exit (5);
				}
				break;
#endif

			case 'R':	/* Sub region */

				sscanf (&argv[i][2], "%[^/]/%[^/]/%[^/]/%s", t_w, t_e, t_s, t_n);
				west  = txt2fl (t_w, 'W');
				east  = txt2fl (t_e, 'W');
				south = txt2fl (t_s, 'S');
				north = txt2fl (t_n, 'S');
				break;

			case 'S':	/* Search for nearest water-node if epicenter is on land */

				search_nearest = TRUE;
				if (argv[i][2] && (n = sscanf (&argv[i][2], "%lf/%lf", &search_radius, &search_depth))) {
					if (search_radius < 0.0) {
						fprintf (stderr, "ttt: Error in -S:  Search radius must be > 0!\n");
						exit (6);
					}
					if (n == 2 && search_depth > 0.0) {
						fprintf (stderr, "ttt: Error in -S:  Water depth must be < 0!\n");
						exit (6);
					}
				}
				break;

			case 'T':	/* Output prefix for ttt file */

				strcpy (tttfile, &argv[i][2]);
				break;

			case 'V':	/* Talk to the user */

				verbose = TRUE;
				if (argv[i][2] == 'l') verbose = 2;	/* Report quake node */
				break;

			default:
				error = TRUE;
				break;
		}
	}
	
	if (error || argc == 1) {
#ifdef TTT120
		fprintf (stderr, "ttt64 %s - Compute tsunami travel times - Geoware (c) 1993-%s\n", TTT_VERSION, TTT_YEAR);
		fprintf (stderr, "\t\tContact information: geoware@geoware-online.com\n\n");
		fprintf (stderr, "Usage: ttt <inputfile> [-B] [-D<zmin>] [-E<lon/lat> or -e<file>] [-Ga|b|f|i]\n");
#else
		fprintf (stderr, "ttt %s - Compute tsunami travel times - Geoware (c) 1993-%s\n", TTT_VERSION, TTT_YEAR);
		fprintf (stderr, "\t\tContact information: geoware@geoware-online.com\n\n");
		fprintf (stderr, "Usage: ttt <inputfile> [-B] [-D<zmin>] [-E<lon/lat> or -e<file>] [-Ga|b|f|i] [-N<nodes>]\n");
#endif
		fprintf (stderr, "	[-O<mm/dd/yyyy/hh/mi>[/<ss>]] [-R<w/e/s/n>] [-S[<radius>][/<depth>]] [-T<ttt_prefix>] [-V[l]]\n\n");
		if (quick) exit (EXIT_FAILURE);
		fprintf (stderr, "	ttt attempts to decode <inputfile> using the following order:\n");
		fprintf (stderr, "	   1. If filename ends in \".b\" it is read as bathymetry data [GMT binary float format].\n");
		fprintf (stderr, "	   2. If filename ends in \".i2\" it is read as bathymetry data [GMT binary short format].\n");
		fprintf (stderr, "	   3. If $TTT_DIR/<inputfile>.i2 exists it will be used as bathymetry data [GMT binary short format].\n\n");
		fprintf (stderr, "\n\tOPTIONS:\n\n");
		fprintf (stderr, "	-B Do NOT normalize the travel times [Default normalizes to avoid bias].\n");
		fprintf (stderr, "	-D Specify a minimum water depth [0].  Depths shallower than 2*z0 will be adjusted\n");
		fprintf (stderr, "	   so that depth goes quadratically from z0 to 2*z0 instead of 0-2*z0.\n");
		fprintf (stderr, "	-E Set the location of the epicenter.\n");
		fprintf (stderr, "	-e Give filename with multiple \"epicenters\" to mimic a non-point source.\n");
		fprintf (stderr, "	-G Store travel times in one of four formats.  Choose from\n");
		fprintf (stderr, "	    -Ga Store travel times as ESRI Arc/Info ASCII Grid Interchange format (*.asc).\n");
		fprintf (stderr, "	    -Gb Store travel times as ESRI Gridfloat file (separate header *.hdr and binary *.flt files).\n");
		fprintf (stderr, "	    -Gf Store travel times as GMT native float grid (*.b) [Default].\n");
		fprintf (stderr, "	    -Gi Store travel times as GMT native 2-byte int grid with units of 10 sec (*.i2).\n");
		fprintf (stderr, "	   Do no supply a file extension with -T as -G will set it.\n");
#ifndef TTT120
		fprintf (stderr, "	-N Number of Huygens nodes to use (8, 16, 32, 48, 64 or 120) [%" TTT_LL "d]\n", n_nodes);
#endif
		fprintf (stderr, "	-O Set earthquake origin time (UTC).  Use lower case -o if local time is used.\n");
		fprintf (stderr, "	-R Specify a sub-region of the grid [Use entire grid].\n");
		fprintf (stderr, "	-S Substitute nearest ocean node if epicenter is on land.\n");
		fprintf (stderr, "	   Optionally, append search radius in degrees [%g].\n", search_radius);
		fprintf (stderr, "	   Furthermore, you may append the minimum depth you want to place epicenter [%g].\n", search_depth);
		fprintf (stderr, "	-T Set prefix for the output grid file with travel times in hrs [ttt].\n");
		fprintf (stderr, "	   Extension will be added automatically (see -G for file formats).\n");
		fprintf (stderr, "	-V Give feedback while running [Default is quiet].\n");
		fprintf (stderr, "	   Append l to see relocated epicenter (if using -S).\n");

		exit (EXIT_FAILURE);
	}
	
	if (n_files > 1) {
		fprintf (stderr, "ttt: Only one input file may be specified!\n");
		exit (7);
	}
	if (n_files == 0) {
		fprintf (stderr, "ttt: Must specify an input file!\n");
		exit (8);
	}
	if (west > east || south > north) {
		fprintf (stderr, "ttt: Sub-region not compatible with grid domain!\n");
		exit (9);
	}

	/* First check if file name ends in .i2 for binary bathymetry files */

	j = ttt_file_prefix (input_file);	/* 0 if no extension */
	if (j && !strncmp (&input_file[j], ".b", 2)) {	/* Filename ends in .b */
		strcpy (bathy_file, input_file);	/* Use as is */
		in_format = 1;	/* Float grid */
	}
	else if (j && (!strncmp (&input_file[j], ".i2", 3) || !strncmp (&input_file[j], ".asc", 4) || !strncmp (&input_file[j], ".flt", 4)))	/* Filename ends in .i2, .asc, or .flt */
		strcpy (bathy_file, input_file);	/* Use as is */
	else	/* For no extension we assume the user wants a short int file in the TTT_DIR directory */
		sprintf (bathy_file, "%s/%s.i2", TTT_DIR, input_file);

	if (access (bathy_file, R_OK)) {	/* No such file found */
		fprintf (stderr, "ttt: Could not find file named %s!\n", bathy_file);
		fprintf (stderr, "ttt: Note: TTT_DIR currently set to %s\n", TTT_DIR);
		exit (11);
	}

	if (!tttfile[0]) strcpy (tttfile, "ttt");	/* Provide a default output file name, according to format */
	
	if (verbose) fprintf (stderr, "ttt: Preparing data\r");
	
	/* Calculate travel times directly from bathymetry grid in real time.  First read file */

	if (verbose) fprintf (stderr, "ttt: Get bathymetry...");

	swabbing = ttt_read_header (bathy_file, &h, west, east, south, north);

	/* Check that all source points are inside grid array */

	for (i = 0; i < n_sources; i++) ttt_check_if_inside (&q_lon[i], &q_lat[i], &h);
	
	ttt_read_bathymetry (bathy_file, &h, &tt, west, east, south, north, in_format, swabbing);
		
	i_quake  = (TTT_LONG *) ttt_memory (NULL, n_sources, sizeof (TTT_LONG), "ttt");
	j_quake  = (TTT_LONG *) ttt_memory (NULL, n_sources, sizeof (TTT_LONG), "ttt");
	ij_quake = (TTT_LONG *) ttt_memory (NULL, n_sources, sizeof (TTT_LONG), "ttt");
	
	row_width = h.nx + 2 * N_PAD;
	
	/* Make sure all source locations are over water.  If not [optionally] move them to the nearest water node */

	for (i = 0; i < n_sources; i++) {

		if (h.periodic) {	/* Handle periodicity in longitude */
			while (q_lon[i] <  h.wesn[XLO]) q_lon[i] += 360.0;
			while (q_lon[i] >= h.wesn[XHI]) q_lon[i] -= 360.0;
		}
		i_quake[i] = ttt_x_to_i (q_lon[i], h.wesn[XLO], h.inc[TTT_X], h.xy_off, h.nx);
		j_quake[i] = ttt_y_to_j (q_lat[i], h.wesn[YLO], h.inc[TTT_Y], h.xy_off, h.ny);

		ij_quake[i] = IJ(i_quake[i], j_quake[i], row_width, N_PAD);	/* Node position of this source */

		if (ttt_check_source (tt, &h, i_quake[i], j_quake[i], ij_quake[i], &q_lon[i], &q_lat[i], search_nearest, search_radius, search_depth, verbose)) { /* on land, move it */
			if (h.periodic) {	/* Handle periodicity in longitude */
				while (q_lon[i]  < h.wesn[XLO]) q_lon[i] += 360.0;
				while (q_lon[i] >= h.wesn[XHI]) q_lon[i] -= 360.0;
			}
			i_quake[i] = ttt_x_to_i (q_lon[i], h.wesn[XLO], h.inc[TTT_X], h.xy_off, h.nx);
			j_quake[i] = ttt_y_to_j (q_lat[i], h.wesn[YLO], h.inc[TTT_Y], h.xy_off, h.ny);
			ij_quake[i] = IJ(i_quake[i], j_quake[i], row_width, N_PAD);	/* New node position of this source */
		}
		else if (verbose == 2) {
			double xp, yp;
			xp = ttt_i_to_x (i_quake[i], h.wesn[XLO], h.wesn[XHI], h.inc[TTT_X], h.xy_off, h.nx);
			yp = ttt_j_to_y (j_quake[i], h.wesn[YLO], h.wesn[YHI], h.inc[TTT_Y], h.xy_off, h.ny);
			printf ("%g\t%g\n", xp, yp);
		}
	}

	/* Calculate travel times directly from bathymetry grid */

	if (verbose) fprintf (stderr, "Calculate slowness...");

	ttt_calc_slowness (&h, tt, -1.0, depth_threshold);		/* Obtain slownesses from bathymetry */
		
	if (verbose) fprintf (stderr, "Initialize offsets/distances...");

	max_nodes = ttt_init_offset (ip, jp, p, &h, n_nodes);	/* Initialize node offsets */

	ttt_init_distances (&h, n_nodes);		/* Calculate distances to neigboring nodes */
	
	/* Do Huygens construction */

	if (verbose) fprintf (stderr, "Calculate ttt...\n");

	ttt_huygens_realtime (&h, ij_quake, n_sources, tt, n_nodes, ip, jp, p, max_nodes, verbose);

	ttt_free_memory (n_nodes);
	
	/* Get uncertainty term */
	
	ttt_rate = ttt_tslope (&h, tt, i_quake[0], j_quake[0]);
	
	if (verbose) fprintf (stderr, "Write ttt...");

	ttt_store_ttt (tttfile, tt, &h, source_file, q_lon, q_lat, n_sources, mm, dd, yy, hh, mi, sc, remove_bias, ttt_rate, n_nodes, out_format, t_format, argc, argv, verbose);
	
	free ((void *)tt);
	free ((void *)q_lon);
	free ((void *)q_lat);
	free ((void *)i_quake);
	free ((void *)j_quake);
	free ((void *)ij_quake);
	
	if (verbose) fprintf (stderr, "Done\n");

	exit (EXIT_SUCCESS);
}
