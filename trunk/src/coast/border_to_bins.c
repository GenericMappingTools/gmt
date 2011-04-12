/*
 *	$Id: border_to_bins.c,v 1.11 2011-04-12 13:06:42 remko Exp $
 */
/* border_to_bins will read political boundaries and rivers files and bin
 * the segments similar to polygon_to_bins, except there is no need to
 * worry about connectivity and levels so we just store the segments
 * in the order received
 */
 
#include "wvs.h"

#define GSHHS_MAX_DELTA 65535		/* Largest value to store in a ushort, used as largest dx or dy in bin  */

struct GMT3_POLY h;

struct BIN {
	struct SEGMENT *first_seg;
	struct SEGMENT *current_seg;
} *bin;

struct SEGMENT {
	int n;
	int level;
	struct SEGMENT *next_seg;
	struct SHORT_PAIR *p;		/* A chain of x,y points is tacked on here  */
};

struct SHORT_PAIR {
	ushort	dx;	/* Relative distance from SW corner of bin, units of B_WIDTH/GSHHS_MAX_DELTA  */
	ushort	dy;
};

struct SEGMENT_HEADER {
	ushort n;		/* n points */
	ushort level;		/* Hierarchical level of feature  */
	int first_p;		/* Id of first point */
} seg_head;

struct GMT3_FILE_HEADER {
	int n_bins;		/* Number of blocks */
	int n_points;		/* Total number of points */
	int bsize;		/* Bin size in minutes */
	int nx_bins;		/* # of bins in 0-360 */
	int ny_bins;		/* # of bins in -90 - +90 */
	int n_segments;		/* Total number of segments */
} file_head;	

struct GMT3_BIN_HEADER {
	int first_seg_id;
	int n_segments;
} *bin_head;	

int *ix, *iy;
int *xx, *yy;
int die (int id);

int main (int argc, char **argv) {
	GMT_LONG first, crossed_x, crossed_y;
	
	int i, k, kk, test_long, nn, np, j, n_alloc, i_x_1, i_x_2, i_y_1, i_y_2, nbins, b;
	int n_final = 0, n_init = 0, dx_1, dx_2, dx, dy, i_x_1mod, i_y_1mod, last_i, i_x_2mod, i_y_2mod, new = 0;
	int x_x_c = 0, x_y_c = 0, y_x_c = 0, y_y_c = 0, last_x_bin, x_x_index, i_x_3, i_y_3, x_origin, y_origin;
	int noise, n, n_id = 0, n_corner = 0, nx, ny, jump = 0, n_seg = 0, add = 0, n_int, ns = 0;
	int this_level = 2, BSIZE, BIN_NX, BIN_NY, B_WIDTH;
	char file[512];
	
	double rx, ry, dxr, dyr, SHORT_FACTOR;
	
	FILE *fp_in, *fp_pt, *fp_bin, *fp_seg;
	
	struct SEGMENT *s, *next;
	struct	LONGPAIR p;
		
	if (argc != 5) {
		fprintf (stderr, "usage: border_to_bins lines.b bsize binned_prefix rank\n");
		fprintf (stderr, "bsize must be 1, 2, 5, 10, or 20 degrees\n");
		exit (-1);
	}
	BSIZE = atoi (argv[2]);
	this_level = atoi (argv[4]);
	if (! (BSIZE == 1 || BSIZE == 2 || BSIZE == 5 || BSIZE == 10 || BSIZE == 20)) {
		fprintf (stderr, "border_to_bins: Bin_size must be 1, 2, 5, 10, or 20\n");
		exit (-1);
	}
	BSIZE *= 60;	/* Now in minutes */
	BIN_NX = (360 * 60) / BSIZE;
	BIN_NY = (180 * 60) / BSIZE;
	B_WIDTH = (MILL * BSIZE) / 60;			/* Bin width in micro-degrees */
	SHORT_FACTOR = (65535.0 / (double)B_WIDTH);	/* This must be a double.  converts microdegrees to short units  */ 
	
	fprintf (stderr, "border_to_bins: Getting rank %d\n", this_level);

	argc = GMT_begin (argc, argv);
	
	fp_in = fopen (argv[1], "r");

	noise = ceil (1.0 / SHORT_FACTOR);	/* Add to corner points so they dont fall exactly on corner */

	nbins = BIN_NX * BIN_NY;
	
	/* Allocate bin array  */
	
	bin = (struct BIN *)GMT_memory(CNULL, nbins, sizeof(struct BIN), "border_to_bins");

	last_x_bin = BIN_NX - 1;
	
	while (pol_readheader (&h, fp_in) == 1) {
		
		h.id = n_id;
		n_id++;
		if (h.level != this_level) {
			fseek (fp_in, h.n * sizeof(struct LONGPAIR), 1);
			continue;
		}
		n_init += h.n;

		if (h.id%100 == 0) fprintf (stderr,"border_to_bins: Binning line %d\r", h.id);
		
		n_alloc = h.n + 10;
		ix = (int *) GMT_memory (CNULL, n_alloc, sizeof (int), "border_to_bins");
		iy = (int *) GMT_memory (CNULL, n_alloc, sizeof (int), "border_to_bins");
		xx = (int *) GMT_memory (CNULL, n_alloc, sizeof (int), "border_to_bins");
		yy = (int *) GMT_memory (CNULL, n_alloc, sizeof (int), "border_to_bins");
		
		if (pol_fread (&p, 1, fp_in) != 1) {
			fprintf(stderr,"border_to_bins: Error reading file.\n");
			exit(-1);
		}
		if (p.x%B_WIDTH == 0) p.x += noise;
		if (p.x >= M360) p.x -= M360;
		if (p.y%B_WIDTH == 0) p.y += noise;
		p.y += M90;
		ix[0] = p.x;
		iy[0] = p.y;
		n_int = 0;
		for (k = kk = 1; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"border_to_bins: Error reading file.\n");
				exit(-1);
			}
			if (p.x%B_WIDTH == 0) p.x += noise;
			if (p.x >= M360) p.x -= M360;
			if (p.y%B_WIDTH == 0) p.y += noise;
			p.y += M90;
			if (p.x%B_WIDTH == 0 && p.y%B_WIDTH == 0) n_corner++;
			dx = (p.x - ix[kk-1]);
			dy = (p.y - iy[kk-1]);
			if (abs(dx) > M180) dx = (int) copysign ((double)(M360 - abs (dx)), (double)-dx);
			if (abs (dx) > B_WIDTH) {	/* Must add intermediate poitns*/
				nx = (abs(dx) / B_WIDTH) + 1;
				ny = (abs(dy) / B_WIDTH) + 1;
				n = MAX (nx, ny);
				dxr = (double)dx / n;
				dyr = (double)dy / n;
				for (i = 0; i < n-1; i++) {
					ix[kk] = ix[kk-1] + rint (dxr);
					iy[kk] = iy[kk-1] + rint (dyr);
					if (ix[kk] < 0) ix[kk] += M360;
					if (ix[kk] >= M360) ix[kk] -= M360;
					kk++;
					n_int++;
				}
			}
			else if (abs (dy) > B_WIDTH) {	/* Must add intermediate poitns*/
				nx = (abs(dx) / B_WIDTH) + 1;
				ny = (abs(dy) / B_WIDTH) + 1;
				n = MAX (nx, ny);
				dxr = (double)dx / n;
				dyr = (double)dy / n;
				for (i = 0; i < n-1; i++) {
					ix[kk] = ix[kk-1] + rint (dxr);
					iy[kk] = iy[kk-1] + rint (dyr);
					kk++;
					n_int++;
				}
			}
			ix[kk] = p.x;
			iy[kk] = p.y;
			kk++;
		}
		
		new += n_int;
		h.n = kk;
		if (n_int > 10)
			n_seg = 0;
		i_x_1 = (ix[0] / B_WIDTH);
		i_y_1=  (iy[0] / B_WIDTH);
		xx[0] = ix[0];	yy[0] = iy[0];
		nn = 1;
		if (h.id == 92)
			n_seg = 0;
		first = TRUE;
		for (i = 1; i < h.n; i++) {	/* Find grid crossings */
			dx = ix[i] - ix[i-1];
			dy = iy[i] - iy[i-1];
			i_x_2 = (ix[i] / B_WIDTH);
			i_y_2 = (iy[i] / B_WIDTH);
			
			crossed_x = crossed_y = FALSE;
			if (i_x_1 != i_x_2) {	/* Crossed x-gridline */
				x_x_index = MAX (i_x_1, i_x_2);
				k = MIN (i_x_1, i_x_2);
				if (x_x_index == last_x_bin && k == 0) {
					x_x_index = 0;	/* Because of wrap */
					x_x_c = x_x_index * B_WIDTH;
					dx_1 = (i_x_1 == 0) ? ix[i-1] : 360000000 - ix[i-1];
					dx_2 = (i_x_2 == 0) ? ix[i] : 360000000 - ix[i];
					dx = dx_1 + dx_2;
				}
				else {
					x_x_c = x_x_index * B_WIDTH;
					dx_1 = x_x_c - ix[i-1];
					if (abs (i_x_2 - i_x_1) > 1) {
						jump++;
					}
				}
				
				x_y_c = iy[i-1] + dx_1 * ((double)(iy[i] - iy[i-1]) / (double) dx);
				crossed_x = TRUE;
			}
			if (i_y_1 != i_y_2) {	/* Crossed y-gridline */
				y_y_c = MAX (i_y_1, i_y_2) * B_WIDTH;
				y_x_c = ix[i-1] + (y_y_c - iy[i-1]) * ((double)dx / (double)dy);
				if (y_x_c < 0) y_x_c += M360;
				if (y_x_c >= M360) y_x_c -= M360;
				crossed_y = TRUE;
				if (abs (i_y_2 - i_y_1) > 1) {
					jump++;
				}
			}
			if (crossed_x && crossed_y) {	/* Cut a corner */
				dx = x_x_c - ix[i-1];
				if (abs(dx) > M180) dx = M360 - abs(dx);		
				rx = hypot ((double)dx, (double)(x_y_c - iy[i-1]));			
				dx = y_x_c - ix[i-1];
				if (abs(dx) > M180) dx = M360 - abs(dx);		
				ry = hypot ((double)dx, (double)(y_y_c - iy[i-1]));
				if (rx == ry) {	/* Cut through corner */
					if (first) {
						first = FALSE;
					}
					
					if (dx < 0) {
						xx[nn] = x_x_c + noise;
						yy[nn] = x_y_c;
						nn++;
						xx[nn] = x_x_c;
						yy[nn] = x_y_c + noise;
						nn++;
					}
					else {
						xx[nn] = x_x_c;
						yy[nn] = x_y_c + noise;
						nn++;
						xx[nn] = x_x_c + noise;
						yy[nn] = x_y_c;
						nn++;
					}
				}
				else if (rx < ry) {
					if (first) {
						first = FALSE;
					}
					xx[nn] = x_x_c;
					yy[nn] = x_y_c;
					nn++;		
					xx[nn] = y_x_c;
					yy[nn] = y_y_c;
					nn++;
				}
				else {
					if (first) {
						first = FALSE;
					}
					xx[nn] = y_x_c;
					yy[nn] = y_y_c;
					nn++;
					xx[nn] = x_x_c;
					yy[nn] = x_y_c;
					nn++;
				}
			}
			else if (crossed_x) {	/* Crossed x-gridline only */
				if (first) {
					first = FALSE;
				}
				xx[nn] = x_x_c;
				yy[nn] = x_y_c;
				nn++;
			}
			else if (crossed_y) {	/* Crossed y-gridline only */
				if (first) {
					first = FALSE;
				}
				xx[nn] = y_x_c;
				yy[nn] = y_y_c;
				nn++;
			}
			xx[nn] = ix[i];
			yy[nn] = iy[i];
			nn++;
			if (nn > (n_alloc-5)) {
				n_alloc += GMT_CHUNK;
				xx = (int *) GMT_memory ((char *)xx, n_alloc, sizeof (int), "polygon_to_bins");
				yy = (int *) GMT_memory ((char *)yy, n_alloc, sizeof (int), "polygon_to_bins");
			}
			i_x_1 = i_x_2;
			i_y_1 = i_y_2;
		}

		n_final += nn;
				
		/* Bin array was allocated at top; nbins was defined; all prs in bin[] were initially NULL.  */

		n_seg = 0;
		if (first) {
			/* This line lies entirely inside one bin.  Get bin, and stick it in there */
			b = (BIN_NY - (yy[0] / B_WIDTH) - 1) * BIN_NX + xx[0] / B_WIDTH;

			/* Create a string header  */
			
			if (bin[b].current_seg) {
				bin[b].current_seg->next_seg = (struct SEGMENT *)GMT_memory(CNULL, 1, sizeof(struct SEGMENT), "border_to_bins");
				bin[b].current_seg = bin[b].current_seg->next_seg;
			}
			else {
				/* This is the first string we put in this bin  */
				bin[b].first_seg = (struct SEGMENT *) GMT_memory(CNULL, 1, sizeof(struct SEGMENT), "border_to_bins");
				bin[b].current_seg = bin[b].first_seg;
			}
			s = bin[b].current_seg;
			
			/* Put np into this header */
			
			s->n = nn;
			s->p = (struct SHORT_PAIR *)GMT_memory(CNULL, s->n, sizeof(struct SHORT_PAIR), "border_to_bins");
			for (k = 0; k < s->n; k++) {
				/* Don't forget that this modulo calculation for DX doesn't work when you have a right/top edge (this wont happen for this polygon though !!!   */
				test_long = irint ((xx[k] % B_WIDTH) * SHORT_FACTOR);
				if (test_long < 0 || test_long > GSHHS_MAX_DELTA) die (h.id);
				s->p[k].dx = (ushort) test_long;
				test_long = irint((yy[k] % B_WIDTH) * SHORT_FACTOR);
				if (test_long < 0 || test_long > GSHHS_MAX_DELTA) die (h.id);
				s->p[k].dy = (ushort) test_long;
			}
			
			free ((char *)xx);
			free ((char *)yy);
			free ((char *)ix);
			free ((char *)iy);
			
			continue;
		}

		/* If we had an inside line, we break out here and go get another one.
		 * If not, execution proceeds with the following */

		i_x_1 = (xx[0] / B_WIDTH);
		i_x_1mod = xx[0] % B_WIDTH;
		i_y_1 = (yy[0] / B_WIDTH);
		i_y_1mod = yy[0] % B_WIDTH;
		last_i = 0;
		for (i = 1, j = 1; j < nn; j++, i++) {
			i_x_2mod = xx[i] % B_WIDTH;
			i_y_2mod = yy[i] % B_WIDTH;
			if (i_x_2mod == 0 || i_y_2mod == 0 || j == (nn-1)) {	/* Exit box or end of line */
				if (i - last_i > 1) {
					/* There is at least 1 inside point.  Use it to get bin */
					k = (i + last_i)/2;
					i_x_3 = xx[k] / B_WIDTH;
					i_y_3 = yy[k] / B_WIDTH;
				}
				else {
					/* We have only 2 ps in this bin; avg them to get bin; check Greenwich */
					if (xx[i] == 0)
						i_x_3 = xx[last_i] / B_WIDTH;
					else if (xx[last_i] == 0)
						i_x_3 = xx[i] / B_WIDTH;
					else
						i_x_3 = ((xx[i] + xx[last_i])/2) / B_WIDTH;
					i_y_3 = ((yy[i] + yy[last_i])/2) / B_WIDTH;
				}
				b = (BIN_NY - i_y_3 - 1) * BIN_NX + i_x_3;

				i_x_2 = (xx[i] / B_WIDTH);
				i_y_2 = (yy[i] / B_WIDTH);

				/* Create a string header  */
				
				if (bin[b].current_seg) {
					bin[b].current_seg->next_seg = (struct SEGMENT *)GMT_memory(CNULL, 1, sizeof(struct SEGMENT), "border_to_bins");
					bin[b].current_seg = bin[b].current_seg->next_seg;
				}
				else {
					/* This is the first string we put in this bin  */
					bin[b].first_seg = (struct SEGMENT *)GMT_memory(CNULL, 1, sizeof(struct SEGMENT), "border_to_bins");
					bin[b].current_seg = bin[b].first_seg;
				}
				s = bin[b].current_seg;
				
				/* Put level, nps, etc. into this header */
				
				s->n = i - last_i + 1;
				s->level = h.level;
				n_seg++;

				/* Write from last_i through i, inclusive, into this bin */
				
				x_origin = i_x_3 * B_WIDTH;
				y_origin = i_y_3 * B_WIDTH;
				s->p = (struct SHORT_PAIR *)GMT_memory(CNULL, s->n, sizeof(struct SHORT_PAIR), "border_to_bins");
				for (k = 0, kk = last_i; k < s->n; k++, kk++) {
					if (i_x_3 == last_x_bin && xx[kk] == 0)
						test_long = GSHHS_MAX_DELTA;
					else
						test_long = irint((xx[kk] - x_origin) * SHORT_FACTOR);
					if (test_long < 0 || test_long > GSHHS_MAX_DELTA) die (h.id);
					s->p[k].dx = (ushort) test_long;
					test_long = irint((yy[kk] - y_origin) * SHORT_FACTOR);
					if (test_long < 0 || test_long > GSHHS_MAX_DELTA) die (h.id);
					s->p[k].dy = (ushort) test_long;
				}
				
				last_i = i;
				i_x_1 = i_x_2;
				i_y_1 = i_y_2;
				i_x_1mod = i_x_2mod;
				i_y_1mod = i_y_2mod;
			}
		}
		
		free ((char *)xx);
		free ((char *)yy);
		free ((char *)ix);
		free ((char *)iy);
		
		/* Now go back up and read next polygon */
		
		n_seg--;
		add += n_seg;
	}
	
	fclose (fp_in);
	
	fprintf (stderr, "\nTotal input points %d\n", n_init);
	fprintf (stderr, "Total output points %d\n", n_final);
	fprintf (stderr, "Total lines processed %d\n", n_id);
	fprintf (stderr, "# of points added as duplicates at crossings = %d\n", add);
	fprintf (stderr, "Adding edges made the database grow by %g %%\n", (100.0 * (n_final - n_init)) / n_init);

	/* Write out */
	
	bin_head = (struct GMT3_BIN_HEADER *) GMT_memory (CNULL, nbins, sizeof (struct GMT3_BIN_HEADER), "border_to_bins");
	
	for (b = file_head.n_segments = 0; b < nbins; b++) {
		s = bin[b].first_seg;
		bin_head[b].first_seg_id = file_head.n_segments;
		while (s) {
			bin_head[b].n_segments++;
			s = s->next_seg;
		}
		file_head.n_segments += bin_head[b].n_segments;
	}
	
	fprintf (stderr, "border_to_bins: Start writing file %s\n", argv[3]);

	file_head.n_bins = nbins;
	file_head.n_points = n_final + add;
	file_head.bsize  = BSIZE;
	file_head.nx_bins = BIN_NX;
	file_head.ny_bins = BIN_NY;
	
	sprintf (file, "%s.bin", argv[3]);
	fp_bin = fopen (file, "w");
	sprintf (file, "%s.seg", argv[3]);
	fp_seg = fopen (file, "w");
	sprintf (file, "%s.pt", argv[3]);
	fp_pt = fopen (file, "w");
	
	if (fwrite ((char *)&file_head, sizeof (struct GMT3_FILE_HEADER), 1, fp_bin) != 1) {
		fprintf (stderr, "border_to_bins: Error writing file header\n");
		exit (-1);
	}
	
	for (b = np = 0; b < nbins; b++) {
	
		if (b%100 == 0) fprintf(stderr,"border_to_bins: Working on bin number %d\r", b);
		
		if (fwrite ((char *)&bin_head[b], sizeof (struct GMT3_BIN_HEADER), 1, fp_bin) != 1) {
			fprintf (stderr, "border_to_bins: Error writing bin header for bin # %d\n", b);
			exit (-1);
		}
		
		if (!bin[b].first_seg) continue;
				
		for (s = bin[b].first_seg; s; s = s->next_seg) {
			seg_head.first_p = np;
			seg_head.n = s->n;
			seg_head.level = s->level;
			if (fwrite ((char *)&seg_head, sizeof (struct SEGMENT_HEADER), 1, fp_seg) != 1) {
				fprintf (stderr, "border_to_bins: Error writing a string header for bin # %d\n", b);
				exit (-1);
			}
			if (fwrite ((char *)(s->p), sizeof (struct SHORT_PAIR), s->n, fp_pt) != s->n) {
				fprintf (stderr, "border_to_bins: Error writing a string for bin # %d\n", b);
				exit (-1);
			}
			free ((char *)s->p);
			np += s->n;
			ns++;
		}
		/* Free memory */
		
		next = bin[b].first_seg->next_seg;
		while (next) {
			s = next;
			next = next->next_seg;
			free ((char *)s);
		}
		free ((char *)bin[b].first_seg);
	}

	fclose(fp_pt);
	fclose(fp_bin);
	fclose(fp_seg);
	
	fprintf (stderr, "\nborder_to_bins: %d corner points, %d jumps, %d total pts\n", n_corner, jump, np);
	if (np != file_head.n_points)
		fprintf (stderr, "border_to_bins: # points written (%d) differ from actual points (%d)!\n", np, file_head.n_points);
	if (ns != file_head.n_segments)
		fprintf (stderr, "border_to_bins: # segments written (%d) differ from actual segments (%d)!\n", ns, file_head.n_segments);

	exit (0);
}

int die (int id)
{
	fprintf(stderr,"IDIOT.  SNAFU IN SHORT INTEGER MATH. id = %d\n", id);
	exit (-1);
}

