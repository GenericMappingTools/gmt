/*
 *	$Id: man_fix.c,v 1.5 2011-04-12 13:06:43 remko Exp $
 */
#include "wvs.h"

struct CHECK {
	int pos;
	struct GMT3_POLY h;
} poly[N_POLY];

struct BURP {
	int a, b, nx, f;
} x[1040];
void wipe (int a, int b);

int main (int argc, char **argv) {
	int i = 0, j, k, nk = 0, n_id, id, n_bad, np, ncut = 0, sort_on_np();
	int go[2], bad, n_skip1 = 0, n_skip2 = 0;
	FILE *fp, *fp_bad, *fp_in, *fp_fix;
	char line[80], cmd[512], file[512], s[10];
	struct	LONGPAIR p;
	
	ncut = (argc == 2) ? atoi (argv[1]) : 100000;
	
	chdir ("/home/aa4/gmt/wvs/pol");
	
	fp = fopen ("/home/aa4/gmt/wvs/headers2.b", "r");
	fread ((char *)&n_id, sizeof (int), 1, fp);
	fread ((char *)poly, sizeof (struct CHECK), n_id, fp);
	fclose (fp);

	fp = fopen ("/home/aa4/gmt/wvs/x.lis2", "r");
	
	i = 0;
	while (fgets (line, 80, fp)) {
		sscanf (line, "%d %d %d", &x[i].a, &x[i].b, &x[i].nx);
		/* if (x[i].a < 4 || x[i].b < 4) 
			x[i].f = 1; */
		if (x[i].nx > ncut) 
			x[i].f = 1;
		else
			x[i].f = 0;
		i++;
	}
	fclose (fp);
	np = i;
	
	fp_bad = fopen ("/home/aa4/gmt/wvs/bad_polygons.lis2", "r");
	while (fgets (line, 80, fp_bad)) {
		sscanf (line, "%d", &bad);
		for (i = 0; i < np; i++) {
			if (x[i].a == bad || x[i].b == bad) {
				x[i].f = 1;
				n_skip1++;
			}
		}
	}
	fclose (fp_bad);

	fp_fix = fopen ("/home/aa4/gmt/wvs/fix_polygons.lis2", "r");
	while (fgets (line, 80, fp_fix)) {
		sscanf (line, "%d", &bad);
		for (i = 0; i < np; i++) {
			if (x[i].a == bad || x[i].b == bad) {
				x[i].f = 1;
				n_skip2++;
			}
		}
	}
	fclose (fp_fix);
	
	fp_in = fopen ("/home/aa4/gmt/wvs/final_x_polygons.b", "r");
	
	fp_bad = fopen ("/home/aa4/gmt/wvs/bad_polygons.lis2", "a");
	fp_fix = fopen ("/home/aa4/gmt/wvs/fix_polygons.lis2", "a");
	
	for (i = nk = 0; i < np; i++) if (x[i].f == 1) nk++;

	fprintf (stderr, "np starts at %d. n_bad = %d, n_fix = %d, rest = %d\n", np, n_skip1, n_skip2, np - nk);

	qsort ((char *)x, np, sizeof(struct BURP), sort_on_np);
	
	do {
	
		for (i = 0; i < np && x[i].f == 1; i++);
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
		
		if (poly[x[i].a].h.source == 1) {	/* Make CIA polygon the second polygon */
			go[0] = x[i].a;	go[1] = x[i].b;
		}
		else {
			go[0] = x[i].b;	go[1] = x[i].a;
		}
		
		for (j = 0; j < 2; j++) {
				
			sprintf (file, "polygon.%d", go[j]);
			fp = fopen (file, "w");
		
			for (id = 0; id < n_id && go[j] != poly[id].h.id; id++);
			
			fseek (fp_in, poly[id].pos, 0);
		
			for (k = 0; k < poly[id].h.n; k++) {
				if (fread((char *)&p, sizeof(struct LONGPAIR), 1, fp_in) != 1) {
					fprintf(stderr,"polygon_extract: Error reading file.\n");
					exit(-1);
				}
				if ((poly[id].h.greenwich & 1) && p.x > poly[id].h.datelon) p.x -= M360;
			
				fprintf (fp, "%d\t%d\n", p.x, p.y);
			}
			fclose (fp);
		}
		
		sprintf (cmd, "XYX -- -X1.0e-6 -Y1.0e-6  polygon.%d polygon.%d", go[0], go[1]);
		system (cmd);
		printf ("Delete polygon %d [y]? ", go[1]);
		s[0] = 0;
		fgets (s, 80, stdin);
		if (s[0] == '\0') {	/* Remove from x.lis */
			fprintf (fp_bad, "%d\n", go[1]);
			fflush (fp_bad);
			printf ("Removing polygon # %d\n", go[1]);
			wipe (x[i].a, x[i].b);
			
			for (j = n_bad = 0; j < np; j++) {
				if (x[j].a == go[1] || x[j].b == go[1]) {
					x[j].nx = -1;
					n_bad++;
				}
			}
	
			qsort ((char *)x, np, sizeof(struct BURP), sort_on_np);
			np -= n_bad;
			fprintf (stderr, "rest now is %d\n", np - nk);
		}
		else if (s[0] == 'q') {
			fp = fopen ("/home/aa4/gmt/wvs/x.lis3", "w");
			for (i = 0; i < np; i++) fprintf (fp, "%d\t%d\t%d\n", x[i].a, x[i].b, x[i].nx);
			fclose (fp);
			exit (0);
		}
		else {
			fprintf (fp_fix, "%d\n", go[1]);
			fflush (fp_fix);
			x[i].f = 1;
			k++;
		}
		
	} while (np);
	
	fclose (fp_in);
	fclose (fp_bad);
	fclose (fp_fix);
	exit (0);
	
}

int sort_on_np (struct BURP *a, struct BURP *b)
{
	if (a->nx > b->nx) return (-1);
	if (a->nx < b->nx) return (1);
	return (0);
}

void wipe (int a, int b) {
	char file[80];
		
	sprintf (file, "polygon.%d", a);
	unlink (file);
	sprintf (file, "polygon.%d", b);
	unlink (file);
}

