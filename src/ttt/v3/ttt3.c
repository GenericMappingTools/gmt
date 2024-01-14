/*
 * COPYRIGHT (c) 1993-2024 GEOWARE
 * 
 * ttt3.c
 *
 * PROGRAM:	ttt.c
 * PURPOSE:	compute tsunami travel time files from bathymetry grids
 * AUTHOR:	Paul Wessel, GEOWARE
 * DATE:	June 16 1993
 * UPDATED:	January 1 2024
 * VERSION:	3.2.2
 *
 *
 * ttt calculates estimated tsunami travel times from an epicenter to all points
 * on a bathymetry grid.  The shallow-water wave approximation is used which means
 * that the propagation velocity is given by sqrt (depth * normal_gravity).  The
 * propagation is calculated on a grid using a very accurate 64-point Huygens con-
 * struction method that introduces a minimum of error.  In the case of constant
 * water depth the travel-time contours should be circular; here they are 64-sided
 * polygons which means the error is < +0.49 %, i.e., travel times can be up to 0.49 %
 * too long.  For a speedier solution the user may choose to use a coarser construction
 * which has more error but executes faster.  The max error for the various choices
 * of nodes are approximately 8.1 % (8), 2.76 % (16), 1.30 % (32), 0.73 % (48), or as
 * mentioned 0.49 % (64).  Note that the error is always >= 0 and thus is biased.
 * By default, ttt normalizes the travel time predictions so that the error
 * discussed above is distributed evenly (i.e., prediction error has ~0 mean).
 * This feature can be turned OFF with the -B option.
 * Note if ttt is compiled with the -DTTT64 preprocessor directive, then a 64-node
 * calculation is hardwired to save if-testing during run-time.
 *
 * The input bathymetry data must be in GMT short int binary native format.  To avoid
 * ambiguities in travel time construction it is best that landlocked bodies of
 * water be set to positive (topography) values or NaN (== 32767 for short int).
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

#include "ttt3.h"
#include "ttt3_macro.h"
#include "../ttt_subs.c"	/* Include common functions shared with ttt_pick */

#define T_LOCAL	0
#define T_UTC	1

#ifdef WORDS_BIGENDIAN
#define ENDIAN 1
#else
#define ENDIAN 0
#endif

/* ----------------------------   Functions used by ttt ------------------------------*/

/* Binary tree manipulation, modified after Sedgewick's Algorithms in C */

void treeinitialize (struct TTT_INFO **H, struct TTT_INFO **T)
{
	struct TTT_INFO *list_head = NULL, *list_tail = NULL;
	list_tail = (struct TTT_INFO *) malloc(sizeof *list_tail);
	list_tail->l = list_tail;	list_tail->r = list_tail;	list_tail->tt = list_tail->ij = -1;
	list_head = (struct TTT_INFO *) malloc(sizeof *list_head);
	list_head->r = list_tail;	list_head->tt = 0;	list_head->ij = 0;
	*H = list_head;	*T = list_tail;
}

void treeinsert (TTT_LONG tt, TTT_LONG ij, struct TTT_INFO *list_head, struct TTT_INFO *list_tail)
{
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

TTT_LONG treesmallest (struct TTT_INFO *list_head, struct TTT_INFO *list_tail, TTT_LONG *ij)
{
	struct TTT_INFO *x = list_head->r;
	while (x->l != list_tail) x = x->l;
	*ij = x->ij;
	return (x->tt);
}

void treedelete (TTT_LONG tt, TTT_LONG ij, struct TTT_INFO *list_head, struct TTT_INFO *list_tail) {
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
	free (t);
	if (tt < p->tt) p->l = x; else p->r = x;
}

/* Check if a source location is inside grid region */

void ttt_check_if_inside (double *q_lon, double *q_lat, struct GRD_HEADER *h)
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

TTT_LONG ttt_check_source (float *s, struct GRD_HEADER *h, TTT_LONG i_quake, TTT_LONG j_quake, TTT_LONG ij_quake, double *q_lon, double *q_lat, TTT_LONG search_nearest, double search_radius, double search_depth, TTT_LONG verbose)
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
				ij_quake = ij;
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

/* Set up relative node offsets from current node to neigbors */

TTT_LONG ttt_init_offset (TTT_LONG *ip, TTT_LONG *jp, TTT_LONG *p, struct GRD_HEADER *h, TTT_LONG n_nodes)
{
	TTT_LONG row_width, need_to_use, i;
	TTT_LONG I[M_ACCESS] = { +1,-1, 0, 0,+1,-1,+1,-1,+2,-2,+2,-2,
			     +1,-1,+1,-1,+3,-3,+3,-3,+1,-1,+1,-1,
			     +3,-3,+3,-3,+2,-2,+2,-2,+4,-4,+4,-4,
			     +1,-1,+1,-1,+4,-4,+4,-4,+3,-3,+3,-3,
			     +5,-5,+5,-5,+1,-1,+1,-1,+5,-5,+5,-5,
			     +2,-2,+2,-2,+2,-2, 0, 0,+2,-2,+2,-2,
			     +3,-3, 0, 0,+4,-4, 0, 0,+3,-3,+3,-3,
			     +4,-4,+4,-4,+2,-2,+2,-2};
	TTT_LONG J[M_ACCESS] = {  0, 0,+1,-1,-1,+1,+1,-1,-1,+1,+1,-1,
			     -2,+2,+2,-2,-1,+1,+1,-1,-3,+3,+3,-3,
			     -2,+2,+2,-2,-3,+3,+3,-3,-1,+1,+1,-1,
			     -4,+4,+4,-4,-3,+3,+3,-3,-4,+4,+4,-4,
			     -1,+1,+1,-1,-5,+5,+5,-5,-2,+2,+2,-2,
			     -5,+5,+5,-5, 0, 0,+2,-2,-2,+2,+2,-2,
			      0, 0,+3,-3, 0, 0,+4,-4,-3,+3,+3,-3,
			      -2,+2,+2,-2,-4,+4,+4,-4};   
	
	/* Nodes are numbered as below (X marks center (current) node;  We only calculate
	 * travel time to the nodes 0-63.  Nodes 64-91 are used to estimate slowness.  The
	 * travel time from X to any of the 64-91 nodes goes through intermediate points.
	 *
	 *
	 *	o	o	o	63	55	*	52	60	o	o	o
	 *
	 *	o	o	47	91	39	79	36	88	44	o	o
	 *
	 *	o	43	83	31	23	75	20	28	80	40	o
	 *
	 *	59	87	27	71	15	67	12	68	24	84	56
	 *
	 *	51	35	19	11	7	3	4	8	16	32	48
	 *
	 *	o	77	73	65	1	X	0	64	72	76	o
	 *
	 *	49	33	17	9	5	2	6	10	18	34	50
	 *
	 *	57	85	25	69	13	66	14	70	26	86	58
	 *
	 *	o	41	81	29	21	74	22	30	82	42	o
	 *
	 *	o	o	45	89	37	78	38	90	46	o	o
	 *
	 *	o	o	o	61	53	o	54	62	o	o	o	 
	 */
	
	
	row_width = h->nx + 2 * N_PAD;

	memcpy ((void *)ip, (void *)I, M_ACCESS * sizeof(TTT_LONG));
	memcpy ((void *)jp, (void *)J, M_ACCESS * sizeof(TTT_LONG));
	for (i = 0; i < M_ACCESS; i++) p[i] = jp[i]*row_width + ip[i];
	
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
		default:
			need_to_use = 0;
			fprintf (stderr, "ttt: Internal snafu. Number of nodes = %" TTT_LL "d\n", n_nodes);
			break;
	}

	return (need_to_use);
}

/* This routine indicates which slowness nodes are needed to calculate an incremental
 * travel time from current node, for all surrounding nodes [64].  This information is
 * encoded using bit arithmetic since only an ON/OFF flag is required per node.  The node
 * numbers passed to LBIT, MBIT and HBIT are the same as in ttt_init_offset().
 * NOTE: Some compilers may give a warning about exceeding integer range for some of
 * the assignments below.  This is not true since unsigned integers are used. */

void ttt_set_use_bits (void)
{
	TTT_LONG i;
	
	memset ((void *)low_use_bits,  0, N_CALC * sizeof (unsigned int));
	memset ((void *)med_use_bits,  0, N_CALC * sizeof (unsigned int));
	memset ((void *)high_use_bits, 0, N_CALC * sizeof (unsigned int));
	
	for (i = 0; i < 8; i++) low_use_bits[i] = LBIT (i);
	
	low_use_bits[8]  = LBIT (0) + LBIT (4) + LBIT (8);
	low_use_bits[9]  = LBIT (1) + LBIT (5) + LBIT (9);
	low_use_bits[10] = LBIT (0) + LBIT (6) + LBIT (10);
	low_use_bits[11] = LBIT (1) + LBIT (7) + LBIT (11);
	low_use_bits[12] = LBIT (3) + LBIT (4) + LBIT (12);
	low_use_bits[13] = LBIT (2) + LBIT (5) + LBIT (13);
	low_use_bits[14] = LBIT (2) + LBIT (6) + LBIT (14);
	low_use_bits[15] = LBIT (3) + LBIT (7) + LBIT (15);
	
	low_use_bits[16] = low_use_bits[8]  + LBIT (16);	high_use_bits[16] = HBIT (64);
	low_use_bits[17] = low_use_bits[9]  + LBIT (17);	high_use_bits[17] = HBIT (65);
	low_use_bits[18] = low_use_bits[10] + LBIT (18);	high_use_bits[18] = HBIT (64);
	low_use_bits[19] = low_use_bits[11] + LBIT (19);	high_use_bits[19] = HBIT (65);
	low_use_bits[20] = low_use_bits[12] + LBIT (20);	high_use_bits[20] = HBIT (67);
	low_use_bits[21] = low_use_bits[13] + LBIT (21);	high_use_bits[21] = HBIT (66);
	low_use_bits[22] = low_use_bits[14] + LBIT (22);	high_use_bits[22] = HBIT (66);
	low_use_bits[23] = low_use_bits[15] + LBIT (23);	high_use_bits[23] = HBIT (67);

	low_use_bits[24] = low_use_bits[8]  + LBIT (24);	high_use_bits[24] = HBIT (68);
	low_use_bits[25] = low_use_bits[9]  + LBIT (25);	high_use_bits[25] = HBIT (69);
	low_use_bits[26] = low_use_bits[10] + LBIT (26);	high_use_bits[26] = HBIT (70);
	low_use_bits[27] = low_use_bits[11] + LBIT (27);	high_use_bits[27] = HBIT (71);
	low_use_bits[28] = low_use_bits[12] + LBIT (28);	high_use_bits[28] = HBIT (68);
	low_use_bits[29] = low_use_bits[13] + LBIT (29);	high_use_bits[29] = HBIT (69);
	low_use_bits[30] = low_use_bits[14] + LBIT (30);	high_use_bits[30] = HBIT (70);
	low_use_bits[31] = low_use_bits[15] + LBIT (31);	high_use_bits[31] = HBIT (71);

	low_use_bits[32] = low_use_bits[16];	med_use_bits[32] = MBIT (32);	high_use_bits[32] = HBIT (64) + HBIT (72);
	low_use_bits[33] = low_use_bits[17];	med_use_bits[33] = MBIT (33);	high_use_bits[33] = HBIT (65) + HBIT (73);
	low_use_bits[34] = low_use_bits[18];	med_use_bits[34] = MBIT (34);	high_use_bits[34] = HBIT (64) + HBIT (72);
	low_use_bits[35] = low_use_bits[19];	med_use_bits[35] = MBIT (35);	high_use_bits[35] = HBIT (65) + HBIT (73);
	low_use_bits[36] = low_use_bits[20];	med_use_bits[36] = MBIT (36);	high_use_bits[36] = HBIT (67) + HBIT (75);
	low_use_bits[37] = low_use_bits[21];	med_use_bits[37] = MBIT (37);	high_use_bits[37] = HBIT (66) + HBIT (74);
	low_use_bits[38] = low_use_bits[22];	med_use_bits[38] = MBIT (38);	high_use_bits[38] = HBIT (66) + HBIT (74);
	low_use_bits[39] = low_use_bits[23];	med_use_bits[39] = MBIT (39);	high_use_bits[39] = HBIT (67) + HBIT (75);

	low_use_bits[40] = low_use_bits[24];	med_use_bits[40] = MBIT (40);	high_use_bits[40] = HBIT (68) + HBIT (80);
	low_use_bits[41] = low_use_bits[25];	med_use_bits[41] = MBIT (41);	high_use_bits[41] = HBIT (69) + HBIT (81);
	low_use_bits[42] = low_use_bits[26];	med_use_bits[42] = MBIT (42);	high_use_bits[42] = HBIT (70) + HBIT (82);
	low_use_bits[43] = low_use_bits[27];	med_use_bits[43] = MBIT (43);	high_use_bits[43] = HBIT (71) + HBIT (83);
	low_use_bits[44] = low_use_bits[28];	med_use_bits[44] = MBIT (44);	high_use_bits[44] = HBIT (68) + HBIT (80);
	low_use_bits[45] = low_use_bits[29];	med_use_bits[45] = MBIT (45);	high_use_bits[45] = HBIT (69) + HBIT (81);
	low_use_bits[46] = low_use_bits[30];	med_use_bits[46] = MBIT (46);	high_use_bits[46] = HBIT (70) + HBIT (82);
	low_use_bits[47] = low_use_bits[31];	med_use_bits[47] = MBIT (47);	high_use_bits[47] = HBIT (71) + HBIT (83);

	low_use_bits[48] = low_use_bits[16];	med_use_bits[48] = MBIT (32) + MBIT (48);	high_use_bits[48] = high_use_bits[32] + HBIT (76);
	low_use_bits[49] = low_use_bits[17];	med_use_bits[49] = MBIT (33) + MBIT (49);	high_use_bits[49] = high_use_bits[33] + HBIT (77);
	low_use_bits[50] = low_use_bits[18];	med_use_bits[50] = MBIT (34) + MBIT (50);	high_use_bits[50] = high_use_bits[34] + HBIT (76);
	low_use_bits[51] = low_use_bits[19];	med_use_bits[51] = MBIT (35) + MBIT (51);	high_use_bits[51] = high_use_bits[35] + HBIT (77);
	low_use_bits[52] = low_use_bits[20];	med_use_bits[52] = MBIT (36) + MBIT (52);	high_use_bits[52] = high_use_bits[36] + HBIT (79);
	low_use_bits[53] = low_use_bits[21];	med_use_bits[53] = MBIT (37) + MBIT (53);	high_use_bits[53] = high_use_bits[37] + HBIT (78);
	low_use_bits[54] = low_use_bits[22];	med_use_bits[54] = MBIT (38) + MBIT (54);	high_use_bits[54] = high_use_bits[38] + HBIT (78);
	low_use_bits[55] = low_use_bits[23];	med_use_bits[55] = MBIT (39) + MBIT (55);	high_use_bits[55] = high_use_bits[39] + HBIT (79);

	low_use_bits[56] = low_use_bits[16] + LBIT (24);	med_use_bits[56] = MBIT (32) + MBIT (56);	high_use_bits[56] = HBIT (64) + HBIT (84);
	low_use_bits[57] = low_use_bits[17] + LBIT (25);	med_use_bits[57] = MBIT (33) + MBIT (57);	high_use_bits[57] = HBIT (65) + HBIT (85);
	low_use_bits[58] = low_use_bits[18] + LBIT (26);	med_use_bits[58] = MBIT (34) + MBIT (58);	high_use_bits[58] = HBIT (64) + HBIT (86);
	low_use_bits[59] = low_use_bits[19] + LBIT (27);	med_use_bits[59] = MBIT (35) + MBIT (59);	high_use_bits[59] = HBIT (65) + HBIT (87);
	low_use_bits[60] = low_use_bits[20] + LBIT (28);	med_use_bits[60] = MBIT (36) + MBIT (60);	high_use_bits[60] = HBIT (67) + HBIT (88);
	low_use_bits[61] = low_use_bits[21] + LBIT (29);	med_use_bits[61] = MBIT (37) + MBIT (61);	high_use_bits[61] = HBIT (66) + HBIT (89);
	low_use_bits[62] = low_use_bits[22] + LBIT (30);	med_use_bits[62] = MBIT (38) + MBIT (62);	high_use_bits[62] = HBIT (66) + HBIT (90);
	low_use_bits[63] = low_use_bits[23] + LBIT (31);	med_use_bits[63] = MBIT (39) + MBIT (63);	high_use_bits[63] = HBIT (67) + HBIT (91);

#ifdef TEST
	for (i = 0; i < 64; i++) printf ("%" TTT_LL "d\t%u\t%u\t%u\n", i, low_use_bits[i], med_use_bits[i], high_use_bits[i]);
#endif
}

/* Initialize the distances from center node to neighbor nodes */

void ttt_init_distances (struct GRD_HEADER *h, TTT_LONG n_nodes)
{
	TTT_LONG j;
	double latitude;
		
	ttt_set_use_bits ();

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
	
	dy = 0.5 * h->inc[TTT_Y] * DEGREE_TO_METER;	/* 0.5 factor put here rather than in main huygens loop */
		
	for (j = 0; j < h->ny; j++) {
		latitude = ttt_j_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[TTT_Y], h->xy_off, h->ny);
#ifdef FLAT
		dx[j] = 0.5 * h->inc[TTT_X] * DEGREE_TO_METER;
#else
		dx[j] = 0.5 * h->inc[TTT_X] * DEGREE_TO_METER * cos (D2R * latitude);
#endif
		dr[j] = hypot (dx[j], dy);
		
		if (n_nodes == 8) continue;
		
		d5x[j] = hypot (0.5 * dx[j], dy);
		d5y[j] = hypot (dx[j], 0.5 * dy);

		if (n_nodes == 16) continue;

		d10x[j] = hypot (THIRD * dx[j], dy);
		d10y[j] = hypot (dx[j], THIRD * dy);
		d13x[j] = hypot (TWO_THIRD * dx[j], dy);
		d13y[j] = hypot (dx[j], TWO_THIRD * dy);

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
	}
}

double ttt_gravity (double latitude)
{	/* Returns normal gravity at this latitude per GRS-80 */
#ifdef TEST
	return (9.81);
#else	
	double s;
	s = sin (D2R * latitude);
	s *= s;	/* sin squared */
	return (G_E * (1.0 + G_C1 * s + G_C2 * s * s));
#endif
}

/* Convert bathymetry to slowness (in s/m) using the relation s = 1/sqrt (depth(m) * normal_gravity).
 * Since dt = d * s (in sec) but we want dt in T_UNIT we put the factor 1/T_UNIT into the slowness values */

void ttt_calc_slowness (struct GRD_HEADER *h, float *s, double sign, double depth_threshold)
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
	 * s[k] will be zero outside the interior grid and NaN will be assinged */
	 
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

void ttt_huygens_realtime (struct GRD_HEADER *h, TTT_LONG *ij_quake, TTT_LONG n_sources, float *s, TTT_LONG n_nodes, TTT_LONG *ip, TTT_LONG *jp, TTT_LONG *p, TTT_LONG max_nodes, TTT_LONG verbose)
{
	TTT_LONG ij, kk, k, nm_grid, row_width, col_height, i, j, x_on_pad, y_on_pad, i0, j0, i_k, j_k;
	TTT_LONG n_left, i360, nx_half, nx1, adjustment, ew_wrap, ignore_east, *tt_min = NULL;
	TTT_LONG last_percent = 0, next_tt, current_tt, set = 1, percent = 0, tt_inc[N_CALC];
	unsigned int ok_to_use[M_ACCESS];
	unsigned int u, low_nan_bits, med_nan_bits, high_nan_bits;
	double c, ss;
	float TTT_NaN;
	struct TTT_INFO *list_tail = NULL, *list_head = NULL;
	
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
	for (j = 0; j < col_height; j++) {
		y_on_pad = (j < N_PAD || j >= (col_height - N_PAD));
		for (i = 0; i < row_width; i++) {
			k = IJ(i,j,row_width,0);
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
		
	/* First initialize the node(s) with the epicenter(s) */
	
	treeinitialize (&list_head, &list_tail);

	for (k = 0; k < n_sources; k++) {
		if (tt_min[ij_quake[k]] == 0) continue;		/* Already set to 0 (Can happen if the same coordinate was given more than once) */
		tt_min[ij_quake[k]] = 0;			/* tt to the node at the quake is zero */
		treeinsert (0, ij_quake[k], list_head, list_tail);	/* Insert into our heap */
	}
		 
	/* Start off from the first in the list */

	current_tt = list_head->r->tt;	/* Travel times start at 0 */
	ij = list_head->r->ij;	/* Get node index */
	j0 = (ij / row_width) - N_PAD;
	i0 = (ij % row_width) - N_PAD;
	tt_min[ij] = USED;				/* Mark this node as used */
	treedelete (current_tt, ij, list_head, list_tail);	/* Remove from binary tree */
	
	n_left--;
	
	do {	/* While there is more work to be done */

		/* Set bit flags so we only calculate incremental travel times to nodes that still are in the running */

		low_nan_bits = med_nan_bits = high_nan_bits = 0;
	
#ifdef TTT64
		for (u = 0;  u < 32U; u++) if (TTT_is_fnan(S(u)) || S(u) >= 0.0) low_nan_bits |= LBIT(u);
		for (u = 32; u < 64U; u++) if (TTT_is_fnan(S(u)) || S(u) >= 0.0) med_nan_bits |= MBIT(u);
		for (u = 64; u < 92U; u++) if (TTT_is_fnan(S(u)) || S(u) >= 0.0) high_nan_bits |= HBIT(u);		
#else
		for (u = 0;  u < (unsigned int)MIN(n_nodes, 32); u++) if (TTT_is_fnan(S(u)) || S(u) >= 0.0) low_nan_bits |= LBIT(u);
		for (u = 32; u < (unsigned int)MIN(n_nodes, 64); u++) if (TTT_is_fnan(S(u)) || S(u) >= 0.0) med_nan_bits |= MBIT(u);
		for (u = 64; u < (unsigned int)max_nodes; u++) if (TTT_is_fnan(S(u)) || S(u) >= 0.0) high_nan_bits |= HBIT(u);
#endif
	
		for (u = 0; u < n_nodes; u++) {	/* Initialize ok_to_use flags and set all inc tts to LAND */
			ok_to_use[u] = ((low_nan_bits & low_use_bits[u]) == 0 && (med_nan_bits & med_use_bits[u]) == 0 && (high_nan_bits & high_use_bits[u]) == 0);
		}
	
		/* j0 is row number (i.e., latitude indicator) or current node */
	
		/* Remember that here, all non-NaN slownesses are negative values. */
		/* Below, the -0.5 will be added to save the overhead of calling the rint function */

#ifndef TTT64
		switch (n_nodes) {	/* We know n_nodes is 8, 16, 32, 48, or 64 */

			/* NOTE there are no breaks after each case.  This is intentional;  the
			   full 64-node solution also needs all the terms for the lower nodes. */

			case 64:	/* Fullblown solution at full precision */
#endif

				/* Do sqrt(26) nodes */
			
				if (ok_to_use[48]) ss = SS_48, tt_inc[48] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[49]) ss = SS_49, tt_inc[49] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[50]) ss = SS_50, tt_inc[50] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[51]) ss = SS_51, tt_inc[51] = GET_TT_INC (d26y[j0]);
				if (ok_to_use[52]) ss = SS_52, tt_inc[52] = GET_TT_INC (d26x[j0]);
				if (ok_to_use[53]) ss = SS_53, tt_inc[53] = GET_TT_INC (d26x[j0]);
				if (ok_to_use[54]) ss = SS_54, tt_inc[54] = GET_TT_INC (d26x[j0]);
				if (ok_to_use[55]) ss = SS_55, tt_inc[55] = GET_TT_INC (d26x[j0]);

				/* Do sqrt(29) nodes */
			
				if (ok_to_use[56]) ss = SS_56, tt_inc[56] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[57]) ss = SS_57, tt_inc[57] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[58]) ss = SS_58, tt_inc[58] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[59]) ss = SS_59, tt_inc[59] = GET_TT_INC (d29y[j0]);
				if (ok_to_use[60]) ss = SS_60, tt_inc[60] = GET_TT_INC (d29x[j0]);
				if (ok_to_use[61]) ss = SS_61, tt_inc[61] = GET_TT_INC (d29x[j0]);
				if (ok_to_use[62]) ss = SS_62, tt_inc[62] = GET_TT_INC (d29x[j0]);
				if (ok_to_use[63]) ss = SS_63, tt_inc[63] = GET_TT_INC (d29x[j0]);

#ifndef TTT64
			case 48:	/* Solution at high precision */
#endif

				/* Do sqrt(17) nodes */
			
				if (ok_to_use[32]) ss = SS_32, tt_inc[32] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[33]) ss = SS_33, tt_inc[33] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[34]) ss = SS_34, tt_inc[34] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[35]) ss = SS_35, tt_inc[35] = GET_TT_INC (d17y[j0]);
				if (ok_to_use[36]) ss = SS_36, tt_inc[36] = GET_TT_INC (d17x[j0]);
				if (ok_to_use[37]) ss = SS_37, tt_inc[37] = GET_TT_INC (d17x[j0]);
				if (ok_to_use[38]) ss = SS_38, tt_inc[38] = GET_TT_INC (d17x[j0]);
				if (ok_to_use[39]) ss = SS_39, tt_inc[39] = GET_TT_INC (d17x[j0]);

				/* Do sqrt(25) nodes */
			
				if (ok_to_use[40]) ss = SS_40, tt_inc[40] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[41]) ss = SS_41, tt_inc[41] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[42]) ss = SS_42, tt_inc[42] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[43]) ss = SS_43, tt_inc[43] = GET_TT_INC (d25y[j0]);
				if (ok_to_use[44]) ss = SS_44, tt_inc[44] = GET_TT_INC (d25x[j0]);
				if (ok_to_use[45]) ss = SS_45, tt_inc[45] = GET_TT_INC (d25x[j0]);
				if (ok_to_use[46]) ss = SS_46, tt_inc[46] = GET_TT_INC (d25x[j0]);
				if (ok_to_use[47]) ss = SS_47, tt_inc[47] = GET_TT_INC (d25x[j0]);

#ifndef TTT64
			case 32:	/* Solution at intermediate precision */
#endif

				/* Do sqrt(10) nodes */
			
				if (ok_to_use[16]) ss = SS_16, tt_inc[16] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[17]) ss = SS_17, tt_inc[17] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[18]) ss = SS_18, tt_inc[18] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[19]) ss = SS_19, tt_inc[19] = GET_TT_INC (d10y[j0]);
				if (ok_to_use[20]) ss = SS_20, tt_inc[20] = GET_TT_INC (d10x[j0]);
				if (ok_to_use[21]) ss = SS_21, tt_inc[21] = GET_TT_INC (d10x[j0]);
				if (ok_to_use[22]) ss = SS_22, tt_inc[22] = GET_TT_INC (d10x[j0]);
				if (ok_to_use[23]) ss = SS_23, tt_inc[23] = GET_TT_INC (d10x[j0]);
			
				/* Do sqrt(13) nodes */
			
				if (ok_to_use[24]) ss = SS_24, tt_inc[24] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[25]) ss = SS_25, tt_inc[25] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[26]) ss = SS_26, tt_inc[26] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[27]) ss = SS_27, tt_inc[27] = GET_TT_INC (d13y[j0]);
				if (ok_to_use[28]) ss = SS_28, tt_inc[28] = GET_TT_INC (d13x[j0]);
				if (ok_to_use[29]) ss = SS_29, tt_inc[29] = GET_TT_INC (d13x[j0]);
				if (ok_to_use[30]) ss = SS_30, tt_inc[30] = GET_TT_INC (d13x[j0]);
				if (ok_to_use[31]) ss = SS_31, tt_inc[31] = GET_TT_INC (d13x[j0]);

#ifndef TTT64
			case 16:	/* Solution at low precision */
#endif

				/* Then do sqrt(5) nodes */
	
				if (ok_to_use[8])  ss = SS_08, tt_inc[8]  = GET_TT_INC (d5y[j0]);
				if (ok_to_use[9])  ss = SS_09, tt_inc[9]  = GET_TT_INC (d5y[j0]);
				if (ok_to_use[10]) ss = SS_10, tt_inc[10] = GET_TT_INC (d5y[j0]);
				if (ok_to_use[11]) ss = SS_11, tt_inc[11] = GET_TT_INC (d5y[j0]);
				if (ok_to_use[12]) ss = SS_12, tt_inc[12] = GET_TT_INC (d5x[j0]);
				if (ok_to_use[13]) ss = SS_13, tt_inc[13] = GET_TT_INC (d5x[j0]);
				if (ok_to_use[14]) ss = SS_14, tt_inc[14] = GET_TT_INC (d5x[j0]);
				if (ok_to_use[15]) ss = SS_15, tt_inc[15] = GET_TT_INC (d5x[j0]);
			
#ifndef TTT64
			case 8:	/* Solution at crude precision */
#endif

				/* First do sqrt(1) nodes */
			
				if (ok_to_use[0]) ss = SS_00, tt_inc[0] = GET_TT_INC (dx[j0]);
				if (ok_to_use[1]) ss = SS_01, tt_inc[1] = GET_TT_INC (dx[j0]);
				if (ok_to_use[2]) ss = SS_02, tt_inc[2] = GET_TT_INC (dy);
				if (ok_to_use[3]) ss = SS_03, tt_inc[3] = GET_TT_INC (dy);
			
				/* Then do sqrt(2) nodes */
			
				if (ok_to_use[4]) ss = SS_04, tt_inc[4] = GET_TT_INC (dr[j0]);
				if (ok_to_use[5]) ss = SS_05, tt_inc[5] = GET_TT_INC (dr[j0]);
				if (ok_to_use[6]) ss = SS_06, tt_inc[6] = GET_TT_INC (dr[j0]);
				if (ok_to_use[7]) ss = SS_07, tt_inc[7] = GET_TT_INC (dr[j0]);
#ifndef TTT64
		}
#endif

		/* Here, all incremental travel times that may be considered have been set */

		/* Store current value and look for fastest path to neigboring nodes */

		s[ij] = (float)(UNIT_TO_HOUR * current_tt);
		n_left--;	/* Because last node is set after the loop */
	
#ifdef TTT64
		for (k = 0; k < 64; k++) {		/* Add increment in travel times to all surrounding nodes */
#else
		for (k = 0; k < n_nodes; k++) {		/* Add increment in travel times to all surrounding nodes */
#endif
		
			if (!ok_to_use[k]) continue;		/* Node is on land - skip */
			
			if (h->periodic) {		/* Carefully handle periodicity in longitude and wrap over poles */
				if ((i_k = i0 + ip[k]) < 0)
					i_k += nx1;	/* Jump to east boundary */
				else if (i_k >= h->nx)
					i_k -= nx1;	/* Jump to west boundary */
				j_k = j0 + jp[k];
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
			
			next_tt = current_tt - tt_inc[k];	/* minus because tt_inc is negative since ss is negative */
			
			if (tt_min[kk] == OCEAN) {					/* First time visiting this node */
				treeinsert (next_tt, kk, list_head, list_tail);		/* Insert new entry into our tree */
				tt_min[kk] = next_tt;
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
		else {	/* Got the next smallest travel time node */
			current_tt = treesmallest (list_head, list_tail, &ij);	/* Get smallest TT */
			treedelete (current_tt, ij, list_head, list_tail);	/* Remove this entry */
			tt_min[ij] = USED;					/* Mark this node as used (i.e., propagated to) */
			j0 = (ij / row_width) - N_PAD;
			i0 = (ij % row_width) - N_PAD;
		}
		if (verbose) {	/* Update progress display */
			set++;
			percent = (TTT_LONG) (c * set + 0.5);
			if (percent != last_percent) {
				fprintf (stderr, "ttt: Completed %3" TTT_LL "d %%\r", percent);
				last_percent = percent;
			}
		}
	
	} while (n_left);	/* Used to be while (list_head->r != list_tail) but sometimes we have dead ends in propagation (up a fjord) and may run out of nodes */

	if (ij >= 0) s[ij] = (float)(UNIT_TO_HOUR * current_tt);	/* Last node to be set */

	if (ignore_east) {	/* Set the right side if a repeating yet unassigned column */
		for (j = k = 0; j < col_height; j++, k += row_width) s[k+i360] = s[k+N_PAD];
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
}

/* Initialize header and save the calculated travel times */

void ttt_store_ttt (char *file_name, float *tt, struct GRD_HEADER *h, char *source_file, double *q_lon, double *q_lat, TTT_LONG n_sources, int mm, int dd, int yy, int hh, int mi, int ss, TTT_LONG remove_bias, double ttt_rate, TTT_LONG n_nodes, TTT_LONG out_format, TTT_LONG t_fmt, int argc, char **argv, TTT_LONG verbose)
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
void ttt_free_memory (TTT_LONG n_nodes)
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
}

double ttt_tslope (struct GRD_HEADER *h, float *tt, TTT_LONG i_quake, TTT_LONG j_quake)
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

void ttt_read_bathymetry (char *file_name, struct GRD_HEADER *h, float **s, double west, double east, double south, double north, TTT_LONG file_format, TTT_LONG swabbing)
{	/* file_format = 1 (float) or 2 (short int) */
	TTT_LONG i, j, k, p, len, half, i180, adjustment, skip_x, skip_y, wrap_i, nx, ny, row_width, col_height;
	unsigned int *i_ptr = NULL, wrap_check = 0, periodic = 0;
	size_t item_size;
	short int *s_record = NULL;
	float *f_record = NULL;
	FILE *fp = NULL;
	float TTT_NaN;
	TTT_make_NaN (TTT_NaN);

	len = strlen (file_name);
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
	
	h->wesn[XLO] = west;	h->wesn[XHI] = east;	h->nx = nx;
	h->wesn[YLO] = south;	h->wesn[YHI] = north;	h->ny = ny;
	
	/* Apply BC to the strips */
	
	adjustment = (h->registration) ? 1 : 0;
	if (fabs (h->wesn[XHI] - h->wesn[XLO] - 360.0) < CONV_LIMIT) {	/* Periodic in 360 longitude */
		h->periodic = TRUE;
		for (j = 0; j < h->ny; j++) {
			k = IJ(0,j,row_width,N_PAD);
			for (p = 1; p <= N_PAD; p++) {
				(*s)[k+h->nx+p-1] = (*s)[k+p-adjustment];
				(*s)[k-p] = (*s)[k+h->nx-p-1+adjustment];
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
	TTT_LONG row_width;		/* row-dimension of extended grid with 5 boundary rows and columns */
	TTT_LONG *i_quake = NULL, *j_quake = NULL;	/* column and row number of current source point */
	TTT_LONG ip[M_ACCESS];		/* Relative grid i offsets from current node to all 92 neighbors */
	TTT_LONG jp[M_ACCESS];		/* Relative grid j offsets from current node to all 92 neighbors */
	TTT_LONG p[M_ACCESS];		/* Relative grid ij offsets from current node to all 92 neighbors */
	TTT_LONG n_sources = 0;		/* Number of point sources making up the fault zone [1] */
	TTT_LONG n_nodes = 64;		/* Number of nodes to use in the Huygens construction */
	TTT_LONG n_files = 0;		/* Number of input files given.  Muts equal 1 for ttt to run */
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

	/* Initialization */

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
						break;
						exit (25);
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
#ifndef TTT64
			case 'N':	/* How many nodes to use in calculations [48] */

				n_nodes = atoi (&argv[i][2]);
				if (!(n_nodes == 8 || n_nodes == 16 || n_nodes == 32 || n_nodes == 48 || n_nodes == 64)) {
					fprintf (stderr, "ttt: Error in -N: Choose among 8, 16, 32, 48, 64!\n");
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
#ifdef TTT64
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
#ifndef TTT64
		fprintf (stderr, "	-N Number of Huygens nodes to use (8, 16, 32, 48, or 64) [%" TTT_LL "d]\n", n_nodes);
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
			while (q_lon[i] < h.wesn[XLO]) q_lon[i] += 360.0;
			while (q_lon[i] >= h.wesn[XHI]) q_lon[i] -= 360.0;
		}
		i_quake[i] = ttt_x_to_i (q_lon[i], h.wesn[XLO], h.inc[TTT_X], h.xy_off, h.nx);
		j_quake[i] = ttt_y_to_j (q_lat[i], h.wesn[YLO], h.inc[TTT_Y], h.xy_off, h.ny);

		ij_quake[i] = IJ(i_quake[i], j_quake[i], row_width, N_PAD);	/* Node position of this source */

		if (ttt_check_source (tt, &h, i_quake[i], j_quake[i], ij_quake[i], &q_lon[i], &q_lat[i], search_nearest, search_radius, search_depth, verbose)) { /* on land, move it */
			if (h.periodic) {	/* Handle periodicity in longitude */
				while (q_lon[i] < h.wesn[XLO]) q_lon[i] += 360.0;
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
