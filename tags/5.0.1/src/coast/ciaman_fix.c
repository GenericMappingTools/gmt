/*
 *	$Id$
 */
#include "wvs.h"

#define LAST		20		/* record # of last entry in x.lis that belongs to clean polygons */
#define MILL 		1000000

struct CHECK {
	int pos;
	struct GMT3_POLY h;
} poly[N_POLY];

struct BURP {
	int a, b, nx, src, finished;
} x[16608];

void wipea (int a);
void wipeb (int b);

int main (int argc, char **argv) {
	int i = 0, j, k, nk = 0, n_id, id, n_bad, np, ncut = 100000, sort_on_np();
	int go[2], bad, n_skip1 = 0, n_skip2 = 0, pos, n_id1, id2, iw, ie, is, in;
	FILE *fp, *fp_bad, *fp_in, *fp_in1, *fp_in2, *fp_fix;
	char line[80], cmd[512], file[512], s[10];
	struct	LONGPAIR p;
	
	if (argc == 4) {
		ncut = atoi (argv[3]);
		fprintf(stderr,"ciaman_fix: ncut = %d\n", ncut);
	}
	
	
	/* Usage: ciaman_fix final_x_polygons.b final_cia2_polygons.b */
	
	fp_in1 = fopen(argv[1], "r");
	fp_in2 = fopen(argv[2], "r");
	
	n_id = pos = 0;
	while (pol_readheader (&poly[n_id].h, fp_in1) == 1) {
		pos += sizeof (struct GMT3_POLY);
		poly[n_id].pos = pos;
		poly[n_id].h.level = 1;
		fseek (fp_in1, poly[n_id].h.n * sizeof(struct LONGPAIR), 1);
		pos += poly[n_id].h.n * sizeof(struct LONGPAIR);
		n_id++;
	}
	n_id1 = n_id;
	
	pos = 0;
	while (pol_readheader (&poly[n_id].h, fp_in2) == 1) {
		pos += sizeof (struct GMT3_POLY);
		poly[n_id].pos = pos;
		poly[n_id].h.level = -1;
		poly[n_id].h.source = 0;
		fseek (fp_in2, poly[n_id].h.n * sizeof(struct LONGPAIR), 1);
		pos += poly[n_id].h.n * sizeof(struct LONGPAIR);
		n_id++;
	}
	
	chdir ("/home/aa4/gmt/wvs/pol");
	
	fp = fopen ("/home/aa4/gmt/wvs/x.lis_2", "r");
	
	i = 0;
	while (fgets (line, 80, fp)) {
		sscanf (line, "%d %d %d", &x[i].a, &x[i].b, &x[i].nx);
		x[i].src = (i < LAST) ? 0 : 1;
			
		if (x[i].nx > ncut) 
			x[i].finished = 1;
		else
			x[i].finished = 0;
		i++;
	}
	fclose (fp);
	np = i;
	
	fp_bad = fopen ("/home/aa4/gmt/wvs/bad_polygons.lis_2", "r");
	while (fp_bad && fgets (line, 80, fp_bad)) {
		sscanf (line, "%d", &bad);
		for (i = 0; i < np; i++) {
			if (x[i].a == bad || x[i].b == bad) {
				x[i].finished = 1;
				n_skip1++;
			}
		}
	}
	if (fp_bad) fclose (fp_bad);

	fp_fix = fopen ("/home/aa4/gmt/wvs/fix_polygons.lis_2", "r");
	while (fp_fix && fgets (line, 80, fp_fix)) {
		sscanf (line, "%d", &bad);
		for (i = 0; i < np; i++) {
			if (x[i].a == bad || x[i].b == bad) {
				x[i].finished = 1;
				n_skip2++;
			}
		}
	}
	if (fp_fix) fclose (fp_fix);
	
	fp_bad = fopen ("/home/aa4/gmt/wvs/bad_polygons.lis_2", "a");
	fp_fix = fopen ("/home/aa4/gmt/wvs/fix_polygons.lis_2", "a");
	
	for (i = nk = 0; i < np; i++) if (x[i].finished == 1) nk++;

	
	for (i = 0, id = n_id1; i < np; i++) {
		if (x[i].src == 0) continue;	/* Must check crossings of CIA & CIA */
		for (id = n_id1; id < n_id && x[i].b != poly[id].h.id; id++);
		fprintf (fp_bad, "%d\n", x[i].b);
		for (j = i+1; j < np; j++) if (!x[j].finished && x[j].b == x[i].b) {
			x[j].finished = 1;
			nk++;
		}
		x[i].finished = 1;
		nk++;
	}
	fflush (fp_bad);
		
	fprintf (stderr, "np starts at %d. n_bad = %d, n_fix = %d, rest = %d\n", np, n_skip1, n_skip2, np - nk);

	qsort (x, np, sizeof(struct BURP), sort_on_np);
	
	do {
	
		for (i = 0; i < np && x[i].finished == 1; i++);
		if (i == np) {
			printf ("Done with all!\n");
			exit (0);
		}
		
		printf ("Checking %d vs %d (nx = %d)\n ", x[i].a, x[i].b, x[i].nx);
		/* do {
			i++;
			printf ("Ok to check %d vs %d (nx = %d) ? ", x[i].a, x[i].b, x[i].nx);
			gets (s);
		}
		while (s[0] == 'n'); */
		
		/* Make new CIA polygon the second polygon */
		
		go[0] = x[i].a;	go[1] = x[i].b;

		
		/* Do file i */
		
		sprintf (file, "polygon.%d", go[0]);
		fp = fopen (file, "w");
		
		for (id = 0; id < n_id && go[0] != poly[id].h.id; id++);
		for (id2 = n_id1; id2 < n_id && go[1] != poly[id2].h.id; id2++);
		
		iw = MILL * floor (poly[id2].h.west);
		ie = MILL * ceil (poly[id2].h.east);
		is = MILL * floor (poly[id2].h.south);
		in = MILL * ceil (poly[id2].h.north);
		
		fp_in = (x[i].src == 0) ? fp_in1 : fp_in2;
		fseek (fp_in, poly[id].pos, 0);
		
		for (k = 0; k < poly[id].h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"ciaman_fix: Error reading file.\n");
				exit(-1);
			}
			if (p.y > in || p.y < is) continue;
			if ((poly[id].h.greenwich & 1) && p.x > poly[id].h.datelon) p.x -= M360;
			if (p.x > ie || p.x < iw) continue;
			
			fprintf (fp, "%d\t%d\n", p.x, p.y);
		}
		fclose (fp);
		
		/* Do file j */
		
		sprintf (file, "polygon.%d", go[1]);
		fp = fopen (file, "w");
		
		fseek (fp_in2, poly[id2].pos, 0);
		
		for (k = 0; k < poly[id2].h.n; k++) {
			if (pol_fread (&p, 1, fp_in2) != 1) {
				fprintf(stderr,"ciaman_fix: Error reading file.\n");
				exit(-1);
			}
			if ((poly[id2].h.greenwich & 1) && p.x > poly[id2].h.datelon) p.x -= M360;
			
			fprintf (fp, "%d\t%d\n", p.x, p.y);
		}
		fclose (fp);
		
		sprintf (cmd, "XYX -- -X1.0e-6 -Y1.0e-6  polygon.%d polygon.%d", go[0], go[1]);
		system (cmd);
		printf ("Delete new polygon %d [y]? ", go[1]);
		s[0] = 0;
		fgets (s, 80, stdin);
		if (s[0] == '\0') {	/* Remove from x.lis */
			fprintf (fp_bad, "%d\n", go[1]);
			fflush (fp_bad);
			printf ("Removing polygon # %d\n", go[1]);
			wipeb (x[i].b);
			wipea (x[i].a);
			
			for (j = n_bad = 0; j < np; j++) {
				if (x[j].a == go[1] || x[j].b == go[1]) {
					x[j].nx = -1;
					n_bad++;
				}
			}
	
			qsort (x, np, sizeof(struct BURP), sort_on_np);
			np -= n_bad;
			fprintf (stderr, "rest now is %d\n", np - nk);
		}
		else if (s[0] == 'Y') {	/* Remove first polygon from x.lis */
			fprintf (fp_bad, "%d\n", go[0]);
			fflush (fp_bad);
			printf ("Removing polygon # %d\n", go[0]);
			wipea (x[i].a);
			wipeb (x[i].b);
			
			for (j = n_bad = 0; j < np; j++) {
				if (x[j].a == go[0] || x[j].b == go[0]) {
					x[j].nx = -1;
					n_bad++;
				}
			}
	
			qsort (x, np, sizeof(struct BURP), sort_on_np);
			np -= n_bad;
			fprintf (stderr, "rest now is %d\n", np - nk);
		}
		else if (s[0] == 'q') {
			fp = fopen ("/home/aa4/gmt/wvs/x.lis_2a", "w");
			for (i = 0; i < np; i++) fprintf (fp, "%d\t%d\t%d\n", x[i].a, x[i].b, x[i].nx);
			fclose (fp);
			wipeb (x[i].b);
			wipea (x[i].a);
			exit (0);
		}
		else if (s[0] == 'n') {
			fprintf (fp_fix, "%d\n", go[1]);
			fflush (fp_fix);
			x[i].finished = 1;
			k++;
			wipea (x[i].a);
		}
		else if (s[0] == 'N') {
			fprintf (fp_fix, "%d\n", go[0]);
			fflush (fp_fix);
			x[i].finished = 1;
			k++;
			wipeb (x[i].b);
		}
		
	} while (np);
	
	fclose (fp_in1);
	fclose (fp_in2);
	fclose (fp_bad);
	fclose (fp_fix);
	exit(0);
}

int sort_on_np (struct BURP *a, struct BURP *b)
{
	if (a->nx > b->nx) return (-1);
	if (a->nx < b->nx) return (1);
	return (0);
}

void wipea (int a) {
	char file[80];
		
	sprintf (file, "polygon.%d", a);
	unlink (file);
}

void wipeb (int b) {
	char file[80];
		
	sprintf (file, "polygon.%d", b);
	unlink (file);
}
