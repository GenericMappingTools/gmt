/*
 *	$Id$
 */
#include "wvs.h"
#include "shore.h"

#define GSHHS_MAX_DELTA 65535		/* Largest value to store in a ushort, used as largest dx or dy in bin  */

struct GMT3_POLY h;

struct BIN {
	struct SEGMENT *first_seg;
	struct SEGMENT *current_seg;
} *bin;

struct SEGMENT_HEADER seg_head;
struct GMT3_FILE_HEADER file_head;	
struct GMT3_BIN_HEADER *bin_head;	

int *ix, *iy;
int *xx, *yy;
int *GSHHS_parent;

void give_bad_message_and_exit (int id, int kind, int pt);

int main (int argc, char **argv) {
	GMT_LONG first, crossed_x, crossed_y;
	
	int i, k, kk, test_long, nn, np, j, n_alloc, start_i, i_x_1, i_x_2, i_y_1, i_y_2, nbins, b;
	int n_final = 0, n_init = 0, dx_1, dx_2, dx, dy, i_x_1mod, i_y_1mod, last_i, i_x_2mod, i_y_2mod;
	int x_x_c = 0, x_y_c = 0, y_x_c = 0, y_y_c = 0, last_x_bin, x_x_index, i_x_3, i_y_3, x_origin, y_origin;
	int noise, se, ne, nw, zero, one, two, three, ij, n, comp_segments(), skip = 0, BSIZE, BIN_NX, BIN_NY, B_WIDTH;
	int n_id = 0,  n_corner = 0, jump = 0, add = 0, n_seg = 0, ns, nclose = 0, n_x_exact = 0, n_y_exact = 0;
	
	char *node_file = 0, file[512];
	
	float *node;
	
	double rx, ry, SHORT_FACTOR;
	
	FILE *fp_in, *fp_pt, *fp_bin, *fp_seg, *fp_par;
	
	struct SEGMENT *s, **ss;
	struct	LONGPAIR p;
	struct GRD_HEADER n_head;
	
		
	if (argc != 5) {
		fprintf (stderr, "usage: polygon_to_bins coast.base bsize nodes.grd binned_prefix\n");
		fprintf (stderr, "bsize must be 1, 2, 5, 10, or 20 degrees\n");
		exit (-1);
	}
	
	argc = GMT_begin (argc, argv);
	
	BSIZE = atoi (argv[2]);
	if (! (BSIZE == 1 || BSIZE == 2 || BSIZE == 5 || BSIZE == 10 || BSIZE == 20)) {
		fprintf (stderr, "polygon_to_bins: Bin_size must be 1, 2, 5, 10, or 20\n");
		exit (-1);
	}
	BSIZE *= 60;	/* Now in minutes */
	BIN_NX = (360 * 60) / BSIZE;
	BIN_NY = (180 * 60) / BSIZE;
	B_WIDTH = (MILL * BSIZE) / 60;			/* Bin width in micro-degrees */
	SHORT_FACTOR = (65535.0 / (double)B_WIDTH);	/* This must be a double.  converts microdegrees to short units  */ 
	noise = ceil (1.0 / SHORT_FACTOR);		/* Add to corner points so they dont fall exactly on corner */
	
	fp_in = fopen (argv[1], "r");
	
	node_file = argv[3];

	nbins = BIN_NX * BIN_NY;
	
	/* Allocate bin array  */
	
#ifdef DEBUG
	GMT_memtrack_off (GMT->dbg.mem_keeper);
#endif
	bin = (struct BIN *) GMT_memory(VNULL, nbins, sizeof(struct BIN), "polygon_to_bins");
	GSHHS_parent = (int *) GMT_memory(VNULL, N_POLY, sizeof(int), "polygon_to_bins");
	for (i = 0; i < N_POLY; i++) GSHHS_parent[i] = -2;	/* Fill with bad value so we can check later that it got overwritten */
	last_x_bin = BIN_NX - 1;
	
	while (pol_readheader (&h, fp_in) == 1) {
		
		n_id++;
		n_init += h.n;
		GSHHS_parent[h.id] = h.parent;
		/*if (h.id%100 == 0) { */
		if (h.id%1 == 0) {
			fprintf (stderr,"polygon_to_bins: Binning polygon %d\r", h.id);
			k = 0;
		}
		if (h.id == 1046) {
			k = 0;
		}
		n_alloc = h.n + 1;
		ix = (int *) GMT_memory (VNULL, n_alloc, sizeof (int), "polygon_to_bins");
		iy = (int *) GMT_memory (VNULL, n_alloc, sizeof (int), "polygon_to_bins");
		xx = (int *) GMT_memory (VNULL, n_alloc, sizeof (int), "polygon_to_bins");
		yy = (int *) GMT_memory (VNULL, n_alloc, sizeof (int), "polygon_to_bins");
		
		if (h.id == 3075)
			k = 0;
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_dump: Error reading file.\n");
				exit(-1);
			}
			if (p.x < 0) p.x += M360;
			if (p.x == M360) p.x -= M360;
			p.y += M90;
			if (p.x%B_WIDTH == 0 && p.y%B_WIDTH == 0) {
				p.y += noise;	/* move up a bit */
				n_corner++;
				fprintf(stderr,"\nPOLY %d WENT EXACTLY THROUGH THE CORNER.  MOVED UP!\n", h.id);
			}
			else if (p.y%B_WIDTH == 0) {	/* Exactly on y boundary */
				p.y += noise;	/* move up a bit */
				n_y_exact++;
			}
			else if (p.x%B_WIDTH == 0 && (p.x+noise) <= M360) {	/* Exactly on x boundary */
				p.x += noise;	/* move right a bit */
				n_x_exact++;
			}
			ix[k] = p.x;
			iy[k] = p.y;
		}
			
		if (!(ix[0] == ix[h.n-1] && iy[0] == iy[h.n-1])) {	/* Close */
			ix[h.n] = ix[0];
			iy[h.n] = iy[0];
			h.n++;
			nclose++;
		}
		
		i_x_1 = (ix[0] / B_WIDTH);
		i_y_1=  (iy[0] / B_WIDTH);
		xx[0] = ix[0];	yy[0] = iy[0];
		nn = 1;
		first = TRUE;
		start_i = 0;
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
						start_i = nn;
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
						start_i = nn;
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
						start_i = nn;
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
					start_i = nn;
					first = FALSE;
				}
				xx[nn] = x_x_c;
				yy[nn] = x_y_c;
				nn++;
			}
			else if (crossed_y) {	/* Crossed y-gridline only */
				if (first) {
					start_i = nn;
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
				xx = (int *) GMT_memory ((void *)xx, n_alloc, sizeof (int), "polygon_to_bins");
				yy = (int *) GMT_memory ((void *)yy, n_alloc, sizeof (int), "polygon_to_bins");
			}
			i_x_1 = i_x_2;
			i_y_1 = i_y_2;
		}

		n_final += nn;
		
		/* Here, last point [nn-1] == first point [0].  We want to duplicate the start_i
		   point so that chain begins and ends on an edge [start_i] */
		   
		if ((nn+start_i+1) > n_alloc) {	
			n_alloc = nn + start_i + 1;
			xx = (int *) GMT_memory ((void *)xx, n_alloc, sizeof (int), "polygon_to_bins");
			yy = (int *) GMT_memory ((void *)yy, n_alloc, sizeof (int), "polygon_to_bins");
		}

		for (i = 1, j = nn; i <= start_i; i++, j++) {	/* Append beginning to tail */
			xx[j] = xx[i];
			yy[j] = yy[i];
		}
		
		/* Array now starts at start_i, not 0 */
		
		/* Bin array was allocated at top; nbins was defined; all prs in bin[] were initially NULL.  */

		n_seg = 0;
		if (first) {
			/* This polygon lies entirely inside one bin.  Get bin, and stick it in there */
			b = (BIN_NY - (yy[0] / B_WIDTH) - 1) * BIN_NX + xx[0] / B_WIDTH;

			/* Create a string header  */
			
			if (bin[b].current_seg) {
				bin[b].current_seg->next_seg = (struct SEGMENT *)GMT_memory(VNULL, 1, sizeof(struct SEGMENT), "polygon_to_bins");
				bin[b].current_seg = bin[b].current_seg->next_seg;
			}
			else {
				/* This is the first string we put in this bin  */
				bin[b].first_seg = (struct SEGMENT *) GMT_memory(VNULL, 1, sizeof(struct SEGMENT), "polygon_to_bins");
				bin[b].current_seg = bin[b].first_seg;
			}
			s = bin[b].current_seg;
			
			/* Put level, nps, etc. into this header */
			
			s->GSHHS_ID = h.id;
			s->level = h.level;
			s->n = nn;
			s->entry = 4;
			s->exit = 4;
			s->p_area = rint (h.area * 10.0);	/* Store area in 1/10 of 1 km^2 */
			if (h.river & 1) s->p_area = -s->p_area;	/* River-lakes are marked by negative area */
			s->p_area_fraction = irint (1e6 * h.area_res / h.area);	/* 1e6 * fraction of full-resolution polygon area */
			s->p = (struct SHORT_PAIR *)GMT_memory(VNULL, s->n, sizeof(struct SHORT_PAIR), "polygon_to_bins");
			for (k = 0; k < s->n; k++) {
				/* Don't forget that this modulo calculation for DX doesn't work when you have a right/top edge (this wont happen for this polygon though !!!   */
				test_long = irint ((xx[k] % B_WIDTH) * SHORT_FACTOR);
				if (test_long < 0 || test_long > GSHHS_MAX_DELTA) give_bad_message_and_exit (h.id, 0, k);
				s->p[k].dx = (ushort) test_long;
				test_long = irint((yy[k] % B_WIDTH) * SHORT_FACTOR);
				if (test_long < 0 || test_long > GSHHS_MAX_DELTA) give_bad_message_and_exit (h.id, 1, k);
				s->p[k].dy = (ushort) test_long;
			}
			
		}
		
		else {
		
		i_x_1 = (xx[start_i] / B_WIDTH);
		i_x_1mod = xx[start_i] % B_WIDTH;
		i_y_1 = (yy[start_i] / B_WIDTH);
		i_y_1mod = yy[start_i] % B_WIDTH;
		last_i = start_i;
		for (i = start_i + 1, j = 1; j < nn; j++, i++) {
			i_x_2mod = xx[i] % B_WIDTH;
			i_y_2mod = yy[i] % B_WIDTH;
			if (i_x_2mod == 0 || i_y_2mod == 0) {	/* Exit box */
				if (i_x_2mod == 0 && i_y_2mod == 0) {
					fprintf(stderr,"\nPOLY %d WENT EXACTLY THROUGH THE CORNER.  TOUGH LUCK!\n", h.id);
					exit (-1);
				}
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
					bin[b].current_seg->next_seg = (struct SEGMENT *)GMT_memory(VNULL, 1, sizeof(struct SEGMENT), "polygon_to_bins");
					bin[b].current_seg = bin[b].current_seg->next_seg;
				}
				else {
					/* This is the first string we put in this bin  */
					bin[b].first_seg = (struct SEGMENT *)GMT_memory(VNULL, 1, sizeof(struct SEGMENT), "polygon_to_bins");
					bin[b].current_seg = bin[b].first_seg;
				}
				s = bin[b].current_seg;
				
				/* Put level, nps, etc. into this header */
				
				s->GSHHS_ID = h.id;
				s->level = h.level;
				s->n = i - last_i + 1;
				n_seg++;
				if (i_x_1mod == 0)
					s->entry = (i_x_1 == i_x_3) ? 3 : 1;
				else
					s->entry = (i_y_1 == i_y_3) ? 0 : 2;
				if (i_x_2mod == 0)
					s->exit = (i_x_2 == i_x_3) ? 3 : 1;
				else
					s->exit = (i_y_2 == i_y_3) ? 0 : 2;
				s->p_area = rint (h.area * 10.0);	/* Store area in 1/10 of 1 km^2 */
				if (h.river & 1) s->p_area = -s->p_area;	/* River-lakes are marked by negative area */
				s->p_area_fraction = irint (1e6 * h.area_res / h.area);	/* 1e6 * fraction of full-resolution polygon area */

				/* Write from last_i through i, inclusive, into this bin */
				
				x_origin = i_x_3 * B_WIDTH;
				y_origin = i_y_3 * B_WIDTH;
				s->p = (struct SHORT_PAIR *)GMT_memory(VNULL, s->n, sizeof(struct SHORT_PAIR), "polygon_to_bins");
				for (k = 0, kk = last_i; k < s->n; k++, kk++) {
					if (i_x_3 == last_x_bin && xx[kk] == 0)
						test_long = GSHHS_MAX_DELTA;
					else
						test_long = irint((xx[kk] - x_origin) * SHORT_FACTOR);
					if (test_long < 0 || test_long > GSHHS_MAX_DELTA) give_bad_message_and_exit (h.id, 0, kk);
					s->p[k].dx = (ushort) test_long;
					test_long = irint((yy[kk] - y_origin) * SHORT_FACTOR);
					if (test_long < 0 || test_long > GSHHS_MAX_DELTA) give_bad_message_and_exit (h.id, 1, kk);
					s->p[k].dy = (ushort) test_long;
				}
				
				/* Double check that the edges got the right stuff */
				
				switch (s->entry) {
					case 0:
						s->p[0].dy = 0;
						break;
					case 1:
						s->p[0].dx = GSHHS_MAX_DELTA;
						break;
					case 2:
						s->p[0].dy = GSHHS_MAX_DELTA;
						break;
					case 3:
						s->p[0].dx = 0;
						break;
				}
				k = s->n - 1;
				switch (s->exit) {
					case 0:
						s->p[k].dy = 0;
						break;
					case 1:
						s->p[k].dx = GSHHS_MAX_DELTA;
						break;
					case 2:
						s->p[k].dy = GSHHS_MAX_DELTA;
						break;
					case 3:
						s->p[k].dx = 0;
						break;
				}
				last_i = i;
				i_x_1 = i_x_2;
				i_y_1 = i_y_2;
				i_x_1mod = i_x_2mod;
				i_y_1mod = i_y_2mod;
			}
		}
		n_seg--;
		}
		
		add += n_seg;
		
		free ((void *)xx);
		free ((void *)yy);
		free ((void *)ix);
		free ((void *)iy);
		
		/* Now go back up and read next polygon */
		
	}
	
	fclose (fp_in);
	
	for (i = 0; i < n_id; i++) {
		if (GSHHS_parent[i] == -2) {
			fprintf (stderr, "polygon_to_bins: Bad parent (%d) for polygon %d (not present?)\n", GSHHS_parent[i], i);
			exit (EXIT_FAILURE);
		}
	}
	
	fprintf (stderr, "\nTotal input and output points: %d %d\n", n_init, n_final);
	fprintf (stderr, "Total polygons processed: %d\n", n_id);
	fprintf (stderr, "%d points added as duplicates at crossings\n", add);
	fprintf (stderr, "Adding edges made the database grow by %g %%\n", (100.0 * (n_final - n_init)) / n_init);

	/* Write out */
	
	sprintf (file, "%s.par", argv[4]);
	fp_par = fopen (file, "w");
	if (fwrite ((void *)&n_id, sizeof (int), 1, fp_par) != 1) {
		fprintf (stderr, "polygon_to_bins: Error writing # of GSHHS parents\n");
		exit (EXIT_FAILURE);
	}
	if (fwrite ((void *)GSHHS_parent, sizeof (int), n_id, fp_par) != n_id) {
		fprintf (stderr, "polygon_to_bins: Error writing GSHHS parents\n");
		exit (EXIT_FAILURE);
	}
	fclose (fp_par);

	bin_head = (struct GMT3_BIN_HEADER *) GMT_memory (VNULL, nbins, sizeof (struct GMT3_BIN_HEADER), "polygon_to_bins");
	
	GMT_grd_init (&n_head, argc, argv, FALSE);
	GMT_err_fail (GMT_read_grd_info (node_file, &n_head), node_file);
	node = (float *) GMT_memory (VNULL, n_head.nx * n_head.ny, sizeof (float), "polygon_to_bins");
	GMT_err_fail (GMT_read_grd (node_file, &n_head, node, 0.0, 0.0, 0.0, 0.0, GMT->current.io.pad, FALSE), node_file);
	se = 1;	ne = 1 - n_head.nx;	nw = -n_head.nx;
	
	for (b = file_head.n_segments = 0; b < nbins; b++) {
		i = b % BIN_NX;	j = (b / BIN_NX) + 1;
		ij = j * (BIN_NX + 1) + i;
		zero = (int)node[ij];	one = (int)node[ij+se];	two = (int)node[ij+ne];	three = (int)node[ij+nw];
		bin_head[b].node_levels = (zero << 9) + (one << 6) + (two << 3) + three;
		s = bin[b].first_seg;
		bin_head[b].first_seg_id = file_head.n_segments;
		while (s) {
			if (s->n == 2 && s->exit == s->entry) {	/* remove */
				skip++;
				s->n = 0;
			}
			else
				bin_head[b].n_segments++;
			s = s->next_seg;
		}
		file_head.n_segments += bin_head[b].n_segments;
	}
	free ((void *) node);
	
	fprintf (stderr, "polygon_to_bins: Start writing file %s\n", argv[4]);

	file_head.n_bins = nbins;
	file_head.n_points = n_final + add - 2*skip;
	file_head.bsize  = BSIZE;
	file_head.nx_bins = BIN_NX;
	file_head.ny_bins = BIN_NY;
	
	sprintf (file, "%s.bin", argv[4]);
	fp_bin = fopen (file, "w");
	sprintf (file, "%s.seg", argv[4]);
	fp_seg = fopen (file, "w");
	sprintf (file, "%s.pt", argv[4]);
	fp_pt = fopen (file, "w");
	
	if (fwrite ((void *)&file_head, sizeof (struct GMT3_FILE_HEADER), 1, fp_bin) != 1) {
		fprintf (stderr, "polygon_to_bins: Error writing file header\n");
		exit (-1);
	}
	
	for (b = np = ns = 0; b < nbins; b++) {
	
		if (b%100 == 0) fprintf(stderr,"polygon_to_bins: Working on bin number %d\r", b);
		
		if (fwrite ((void *)&bin_head[b], sizeof (struct GMT3_BIN_HEADER), 1, fp_bin) != 1) {
			fprintf (stderr, "polygon_to_bins: Error writing bin header for bin # %d\n", b);
			exit (-1);
		}
		
		for (s = bin[b].first_seg, n = 0; s; s = s->next_seg) if (s->n > 0) n++;
		if (n == 0) continue;
		
		/* Had at least 1 segment in this bin */
		
		ss = (struct SEGMENT **) GMT_memory (VNULL, n, sizeof (struct SEGMENT *), "polygon_to_bins");
		for (s = bin[b].first_seg, n = 0; s; s = s->next_seg) {
			if (s->n > 0) {
				ss[n] = s;
				n++;
			}
		}
		qsort ((void *)ss, n, sizeof (struct SEGMENT *), comp_segments);
		ns += n;
		
		for (i = 0; i < n; i++) {
			seg_head.GSHHS_ID = ss[i]->GSHHS_ID;
			seg_head.first_p = np;
			seg_head.p_area = ss[i]->p_area;
			seg_head.p_area_fraction = ss[i]->p_area_fraction;
			seg_head.info = (ss[i]->n << 9) + (ss[i]->level << 6) + (ss[i]->entry << 3) + ss[i]->exit;
			if (fwrite ((void *)&seg_head, sizeof (struct SEGMENT_HEADER), 1, fp_seg) != 1) {
				fprintf (stderr, "polygon_to_bins: Error writing a string header for bin # %d\n", b);
				exit (-1);
			}
			if (fwrite ((void *)(ss[i]->p), sizeof (struct SHORT_PAIR), ss[i]->n, fp_pt) != ss[i]->n) {
				fprintf (stderr, "polygon_to_bins: Error writing a string for bin # %d\n", b);
				exit (-1);
			}
			np += ss[i]->n;
			free ((void *)ss[i]->p);
			free ((void *)ss[i]);
		}
		free ((void *)ss);
	}
	fprintf(stderr,"polygon_to_bins: Working on bin number %d\r", b);

	fclose(fp_pt);
	fclose(fp_bin);
	fclose(fp_seg);
	
	fprintf (stderr, "\npolygon_to_bins: %d corner points, %d jumps, %d total pts\n", n_corner, jump, np);
	fprintf (stderr, "polygon_to_bins: Moved %d points exactly on x edge\n", n_x_exact);
	fprintf (stderr, "polygon_to_bins: Moved %d points exactly on y edge\n", n_y_exact);
	if (np != file_head.n_points)
		fprintf (stderr, "polygon_to_bins: # points written (%d) differ from actual points (%d)!\n", np, file_head.n_points);
	if (ns != file_head.n_segments)
		fprintf (stderr, "polygon_to_bins: # segments written (%d) differ from actual segments (%d)!\n", ns, file_head.n_segments);
	if (nclose) fprintf (stderr, "polygon_to_bins: %d polygons not closed?\n", nclose);
	if (skip) fprintf (stderr, "polygon_to_bins: %d polygons with 2 points only on same side removed\n", skip);
		
#ifdef DEBUG
	GMT_memtrack_on (GMT->dbg.mem_keeper);
#endif
	exit (0);
}

int comp_segments (a, b)
struct SEGMENT **a, **b; {
	if ((*a)->level < (*b)->level) return (-1);
	if ((*a)->level > (*b)->level) return (1);
	if ((*a)->n > (*b)->n) return (-1);
	if ((*a)->n < (*b)->n) return (1);
	return (0);
}

void give_bad_message_and_exit (int id, int kind, int pt)
{
	static char *type[2] = {"lon", "lat"};
	fprintf (stderr, "polygon_to_bins: Incremental %s exceeds short int range for polygon %d near point %d\n",
		type[kind], id, pt);
	fprintf (stderr, "polygon_to_bins: Most likely cause is a point separation that exceeds the bin spacing\n");
	exit (EXIT_FAILURE);
}
