/*
 *	$Id$
 *
 * segment_connect clean_segment_file.b tmp_poly_segment_file.b closed_poly.b
 *
 * Finds neighboring segments and combines them into longer chains [possibly
 * polygons] provided the endpoints are closer than some cutoff distance.
 * 
 * Paul Wessel, July, 1994.
 */

#include "wvs.h"

#define	MAX_STR_LEN 8192	/* I ran some stats and found 6137 as the max  */
#define	MAX_N_STR 34000		/* I ran some stats and found 33428 actually  */
#define	MAX_INT 1000000000

#define GAP	250	/* gap in meters */

struct BUDDY {
	int id;
	int dist;
	int tie;
};

struct LINK {
	int id;
	int pos;
	int n;
	int used;
	struct LONGPAIR end[2];
	struct BUDDY buddy[2][5];
} seg[MAX_N_STR];

double M_PR_DEG;
int i_great_circle_dist(struct LONGPAIR A, struct LONGPAIR B );
void gwrite (struct LONGPAIR p[], int n);

int main (int argc, char **argv)
{
	FILE	*fp, *fp2, *fp3;
	struct RAWSEG_HEADER hin;
	struct LONGPAIR *p, p_last, p_first, p_dummy;
	int	id, j, k, pos, ns, i, dd[2][2], kmin[2], ii, jj, tie, start_id, done, n_pieces, id2;
	int h_pos, n_new, n, cutoff = GAP, chain = 0, n_islands = 0, n_trouble = 0, n_closed = 0, first;
	int n_alloc = 10000, do_close;

	if (argc != 4 || (fp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr,"usage: segment_connect clean_segment_file tmp_poly_segment_file.b closed_polyfile.b\n");
		exit(-1);
	}

	fp2 = fopen (argv[2], "w");
	fp3 = fopen (argv[3], "w");
	
	M_PR_DEG = 111319.49;
	memset ((char *)seg, 0, MAX_N_STR * sizeof (struct LINK));
	
	p = (struct LONGPAIR *) GMT_memory (CNULL, 10000, sizeof(struct LONGPAIR), "segment_connect");
	
	/* First read in all segment info */
	
	id = pos = ns = 0;
	/* while (ns < 2000 && (fread((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp)) == 1) { */
	while (fread((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp) == 1) {

		if (hin.n > n_alloc) {
			n_alloc = hin.n;
			p = (struct LONGPAIR *) GMT_memory ((char *)p, hin.n, sizeof(struct LONGPAIR), "segment_connect");
		}
		
		if ((fread((char *)p, sizeof(struct LONGPAIR), hin.n, fp)) != hin.n) {
			fprintf(stderr,"segment_connect: Error reading file.\n");
			exit(-1);
		}
		
		/* Write out any closed polygons now */
		
		do_close = (p[0].x == p[hin.n-1].x && p[0].y == p[hin.n-1].y);
		if (!do_close)
			do_close = (i_great_circle_dist (p[0], p[hin.n-1]) < cutoff);
		
		if (do_close) {
			if (fwrite((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp3) != 1) {
				fprintf(stderr,"segment_connect: Error writing file 2.\n");
				exit(-1);
			}
			if (fwrite((char *)p, sizeof(struct LONGPAIR), hin.n, fp3) != hin.n) {
				fprintf(stderr,"segment_connect: Error writing file 2.\n");
				exit(-1);
			}
			n_islands++;
		}
		else {
	
			seg[id].id = id;
			seg[id].pos = pos;
			seg[id].n = hin.n;
			seg[id].end[0] = p[0];
			seg[id].end[1] = p[hin.n-1];
			for (i = 0; i < 5; i++) seg[id].buddy[0][i].dist = seg[id].buddy[1][i].dist = MAX_INT;
			ns++;
			id++;
		}

		pos += (sizeof(struct RAWSEG_HEADER) + hin.n * sizeof (struct LONGPAIR));
	}
	fclose (fp3);
	fprintf (stderr, "%d island polygons written to file %s\n", n_islands, argv[3]);
	
	for (i = 0; i < ns; i++) {

		for (j = i; j < ns; j++) {
		
			if (i == j) {
				dd[0][0] = MAX_INT;
				dd[0][1] = i_great_circle_dist (seg[i].end[0], seg[i].end[1]);
    				kmin[0] = 1;
    				kmin[1] = 0;
				dd[1][0] = dd[1][1] = MAX_INT;
			}
			else {
				dd[0][0] = i_great_circle_dist (seg[i].end[0], seg[j].end[0]);
				dd[0][1] = i_great_circle_dist (seg[i].end[0], seg[j].end[1]);
				dd[1][0] = i_great_circle_dist (seg[i].end[1], seg[j].end[0]);
				dd[1][1] = i_great_circle_dist (seg[i].end[1], seg[j].end[1]);
    				for (ii = 0; ii < 2; ii++) kmin[ii] = (dd[ii][0] < dd[ii][1]) ? 0 : 1;
    			}
    			
    			for (ii = 0; ii < 2; ii++) {
    				jj = kmin[ii];
    				if (dd[ii][jj] < seg[i].buddy[ii][0].dist) {
    					for (k = 4; k > 0; k--) seg[i].buddy[ii][k] = seg[i].buddy[ii][k-1];
					seg[i].buddy[ii][0].id = j;
					seg[i].buddy[ii][0].dist = dd[ii][jj];
					seg[i].buddy[ii][0].tie = jj;
    				}
    				if (dd[ii][jj] < seg[j].buddy[jj][0].dist) {
    					for (k = 4; k > 0; k--) seg[j].buddy[jj][k] = seg[j].buddy[jj][k-1];
  					seg[j].buddy[jj][0].id = i;
					seg[j].buddy[jj][0].dist = dd[ii][jj];
					seg[j].buddy[jj][0].tie = ii;
    				}
    			}
		}
	}
	
	fp3 = fopen ("link.dat", "w");
	for (i = 0; i < ns; i++) fprintf (fp3, "%d\t%d\t%d\n", i, seg[i].buddy[0][0].dist, seg[i].buddy[1][0].dist);
	fclose (fp3);

	start_id = done = 0;
	p_dummy.x = p_dummy.y = -1;
	
	while (!done) {
	
		/* Find the 'beginning' of the chain that this segment belongs to */
		
		done = FALSE;
		id = start_id;
		tie = n_pieces = 0;
		while (!done && seg[id].buddy[tie][0].dist < cutoff && !seg[seg[id].buddy[tie][0].id].used) {
			id2 = seg[id].buddy[tie][0].id;
			if (id2 == start_id)
				done = TRUE;
			if (id2 == id) {
				done = TRUE;
				n_trouble++;
			}
			else {
				tie = !seg[id].buddy[tie][0].tie;
				id = id2;
				n_pieces++;
			}
		}
				
		/* This should be the beginning.  Now trace forward and dump out the chain */
		
		/* First dump start segment */
		
		start_id = id;
		h_pos = ftell (fp2);
		
		if (fwrite((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp2) != 1) {
			fprintf(stderr,"segment_connect: Error writing file 2.\n");
			exit(-1);
		}
		
		p_first = p_last = p_dummy;
		n_new = 0;
		k = 0;
		done = FALSE;
		first = TRUE;
		/* printf ("> next chain\n"); */
		do {
			fseek (fp, seg[id].pos, 0);
			fread((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp);
			if ((fread((char *)p, sizeof(struct LONGPAIR), hin.n, fp)) != hin.n) {
				fprintf(stderr,"segment_connect: Error reading file.\n");
				exit(-1);
			}
			if (tie == 0) {
				if (p[0].x == p_last.x && p[0].y == p_last.y) {
					j = 1;
					n = hin.n - 1;
				}
				else {
					j = 0;
					n = hin.n;
				}
				/* gwrite (&p[j], n); */
				if (fwrite((char *)&p[j], sizeof(struct LONGPAIR), n, fp2) != n) {
					fprintf(stderr,"segment_connect: Error writing file 2.\n");
					exit(-1);
				}
				p_last = p[hin.n-1];
				if (first) p_first = p[0];
			}
			else {	/* Must reverse */
				if (p[hin.n-1].x == p_last.x && p[hin.n-1].y == p_last.y) {
					j = 1;
					n = hin.n - 1;
				}
				else {
					j = 0;
					n = hin.n;
				}
				for (i = hin.n-1-j; i >= 0; i--) {
					/* gwrite (&p[i], 1); */
					if (fwrite((char *)&p[i], sizeof(struct LONGPAIR), 1, fp2) != 1) {
						fprintf(stderr,"segment_connect: Error writing file 2.\n");
						exit(-1);
					}
				}
				p_last = p[0];
				if (first) p_first = p[hin.n-1];
			}
			first = FALSE;
			n_new += n;
			tie = !tie;
			seg[id].used = TRUE;
			if (seg[id].buddy[tie][0].dist < cutoff && !seg[seg[id].buddy[tie][0].id].used) {
				id2 = seg[id].buddy[tie][0].id;
				tie = seg[id].buddy[tie][0].tie;
				done = (id2 == start_id || id2 == id);
				id = id2;
			}
			else
				done = TRUE;
			k++;
		}
		while (!done);
		
		if (p_first.x == p_last.x && p_first.y == p_last.y) n_closed++;
		
		/* Update segment header */
		
		fseek (fp2, h_pos, 0);
		hin.n = n_new;
		if (fwrite((char *)&hin, sizeof(struct RAWSEG_HEADER), 1, fp2) != 1) {
			fprintf(stderr,"segment_connect: Error writing file 2.\n");
			exit(-1);
		}
		fseek (fp2, 0, 2);
		fprintf (stderr, "segment_connect: Chain # %d contained %d segments and %d points\r", chain, k, n_new);
		chain++;
		
		start_id = 0;
		while (start_id < ns && seg[start_id].used) start_id++;
		done = (start_id == ns);
	}
	fclose (fp2);
	fclose (fp);	
	free ((char *)p);

	fprintf (stderr, "\nsegment_connect: %d old segments, %d new segments, %d of them closed, %d trouble spots\n", ns, chain, n_closed, n_trouble);
	exit(0);
}

int i_great_circle_dist(struct LONGPAIR A, struct LONGPAIR B )
{
	/* great circle distance on a sphere in meters */
	double c;

	if (A.x == B.x && A.y == B.y) return(0);

	c = GMT_great_circle_dist((double)A.x*1e-6, (double)A.y*1e-6, (double)B.x*1e-6, (double)B.y*1e-6);

	return((int)(c * M_PR_DEG));
}

void gwrite (struct LONGPAIR p[], int n) {
	int i;
	for (i = 0; i < n; i++) printf ("%g\t%g\n", 1.0e-6*p[i].x, 1.0e-6*p[i].y);
}
