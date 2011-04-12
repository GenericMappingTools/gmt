/*
 *	$Id: polygon_setnodes.c,v 1.17 2011-04-12 13:06:42 remko Exp $
 */
/* polygon_setnodes is run on the final polygon file when all polygons
 * have had their level determined.  This program will determine
 * the level of each node on a bin x bin global grid, i.e., the level of
 * the innermost polygon that contains the node.  This is needed since
 * many blocks will be completely outside all polygons (mid ocean) or
 * completely inside a large enough polygon that no crossings with the
 * block borders occur (mid continent, middle of big lake etc).  We
 * store the node levels in a grdfile for convenience.
 */
 
#include "wvs.h"


struct BLOB {
	struct GMT3_POLY h;
	int start;
} blob[N_POLY];

int *lon, *lat;
int non_zero_winding2 (int xp, int yp, int *x, int *y, int n_path);

struct GRD_HEADER grdh;
float *grd;
int main (int argc, char **argv)
{
	int i, j, k, n_id, pos, n_nodes, id, intest, n, nx_minus_1, ix0, off, iy0, slow = 0;
	int iblon, iblat, ij, i0, i1, j0, j1, ii, full, cont_no, c;
	int *IX[N_CONTINENTS][2], *IY[N_CONTINENTS][2], N[N_CONTINENTS][2];
	double west, east, blon, blat, x0, w, iw;
	FILE *fp;
	struct LONGPAIR p;
	
	if (argc != 4) {
		fprintf (stderr, "usage: polygon_setnodes final_dbase.b bin_width nodegrdfile\n");
		exit (EXIT_FAILURE);
	}

	argc = GMT_begin (argc, argv);
	
	/* Grdfile will be grid registered so lon = 0 and 360 will be separate nodes.
	   We must make sure we update both lons */

	w = atof (argv[2]);
	iw = 1.0 / w;
	GMT_grd_init (&grdh, argc, argv, FALSE);
	grdh.nx = irint (360.0 * iw) + 1;
	grdh.ny = irint (180.0 * iw) + 1;
	n_nodes = grdh.nx * grdh.ny;
	grd = (float *) GMT_memory (VNULL, (size_t)n_nodes, sizeof(float), "polygon_setnodes");
        grdh.x_min = 0.0;
        grdh.x_max = 360.0;
        grdh.y_min = -90.0;
        grdh.y_max = 90.0;
        grdh.x_inc = grdh.y_inc = w;
        grdh.registration = 0;
        grdh.z_scale_factor = 1.0;
        grdh.z_add_offset = 0.0;
        strcpy (grdh.x_units, "Longitude");
        strcpy (grdh.y_units, "Latitude");
        strcpy (grdh.z_units, "Polygon Level");
        sprintf (grdh.title, "Polygon Levels in file %s", argv[1]);
        
        nx_minus_1 = grdh.nx - 1;
        
	fprintf (stderr, "Read headers\n");
	fp = fopen (argv[1], "r");
	
	/* Reset ids as we go along */
	
	n_id = pos = 0;
	while (pol_readheader (&blob[n_id].h, fp) == 1) {
		cont_no = (blob[n_id].h.river >> 8);	/* Get continent nubmer 1-6 (0 if not a continent) */
		pos += sizeof (struct GMT3_POLY);
		blob[n_id].start = pos;
		if (pol_fread (&p, 1, fp) != 1) {
			fprintf(stderr,"polygon_setnodes: Error reading file.\n");
			exit (EXIT_FAILURE);
		}
		fseek (fp, (long) (blob[n_id].h.n - 1) * sizeof(struct LONGPAIR), 1);
		pos += blob[n_id].h.n * sizeof(struct LONGPAIR);
		blob[n_id].h.id = n_id;
		n_id++;
	}

	lon = (int *) GMT_memory (VNULL, N_LONGEST, sizeof(int), "polygon_setnodes");
	lat = (int *) GMT_memory (VNULL, N_LONGEST, sizeof(int), "polygon_setnodes");
	full = (blob[0].h.n > 1000000);	/* TRUE for full resolution */
	full = FALSE;
	fprintf (stderr, "Start inside testing (Antarctica == %d)\n\n", ANTARCTICA);
	
	crude_init_int (IX, IY, N, MILL);

	for (id = 0; id < n_id; id++) {	/* For all anchor polygons */
	
		cont_no = WVS_continent (blob[id].h);	/* Get continent number 1-6 (0 if not a continent) */

		slow = (w <= 2.0 && cont_no);
		
		if (full && id == N_CONTINENTS) fprintf (stderr, "\n");
		
		if (!slow) fprintf (stderr, "P = %d\r", id);

		fseek (fp, (long)blob[id].start, 0);
		for (k = 0; k < blob[id].h.n; k++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_setnodes: Error reading file.\n");
				exit (EXIT_FAILURE);
			}
			if ((blob[id].h.greenwich & 1) && p.x > blob[id].h.datelon) p.x -= M360;
			lon[k] = p.x;
			lat[k] = p.y;
		}
		n = blob[id].h.n;
		
		west = blob[id].h.west;	east = blob[id].h.east;
		if (blob[id].h.greenwich & 1) {
			west += 360.0;
			east += 360.0;
		}
		
		if (cont_no == ANTARCTICA) {	/* Antarctica : Switch to polar coordinates r, theta */
			for (i = 0; i < n; i++) xy2rtheta_int (&lon[i], &lat[i]);
		}

		j0 = ceil ((90.0 - blob[id].h.north) * iw);
		j1 = floor ((90.0 - blob[id].h.south) * iw);
		i0 = ceil (blob[id].h.west * iw);
		i1 = floor (blob[id].h.east * iw);
		for (j = j0; j <= j1; j++) {
			blat = 90.0 - j * grdh.y_inc;;
			iblat = blat * MILL;
			for (i = i0; i <= i1; i++) {
				ii = (i < 0) ? i + nx_minus_1 : i;
				ij = j * grdh.nx + ii;
			
				blon = i * grdh.x_inc;
				iblon = blon * MILL;
			
				/* First perform simple tests based on min/max coordinates */
			
				if ( (blat > blob[id].h.north) || blat < blob[id].h.south) continue;
			
				off = 0;
				ix0 = blon;
				while (ix0 < west) {
					ix0 += 360;
					off += M360;
				}
				if (ix0 > east) continue;
			
				/* Must compare with polygon boundaries */
			
				if (full && cont_no > 0 && cont_no != ANTARCTICA ) {	/* Any continent other than Antarctica */
					c = cont_no - 1;
					intest = non_zero_winding2 (iblon + off, iblat, IX[c][OUTSIDE], IY[c][OUTSIDE], N[c][OUTSIDE]);
					if (!intest) continue;
					
					/* So point is inside crude outside. Now check if it is inside crude inside */
				
					intest = non_zero_winding2 (iblon + off, iblat, IX[c][INSIDE], IY[c][INSIDE], N[c][INSIDE]);

					if (intest == 2) {	/* way inside, set levels */
						if (grd[ij] < blob[id].h.level) grd[ij] = blob[id].h.level;
						if (i == 0) grd[ij+nx_minus_1] = grd[ij];
						if (i == nx_minus_1) grd[ij-nx_minus_1] = grd[ij];
						continue;
					}
					/* If not, we fall down to the _real_ test */
				}
			
				/* Here we need to perform complete inside test */
			
				if (!(lon[0] == lon[n-1] && lat[0] == lat[n-1])) {	/* Make sure polygon is closed */
					lon[n] = lon[0];
					lat[n] = lat[0];
					n++;
				}

				if (cont_no == ANTARCTICA) {	/* Do r-theta projection */
					ix0 = iblon;
					iy0 = iblat;
					xy2rtheta_int (&ix0, &iy0);
				}
				else {
					x0 = blon;
					if (cont_no == EURASIA && x0 > blob[id].h.east)
						x0 -= 360.0;
					else if (cont_no != EURASIA && (blob[id].h.greenwich & 1) && x0 > 180.0)
						x0 -= 360.0;
					else if (!(blob[id].h.greenwich & 1) && x0 < 0.0)
						x0 += 360.0;
					ix0 = x0 * MILL;
					iy0 = iblat;
				}
			
				intest = non_zero_winding2 (ix0, iy0, lon, lat, n);
			
				if (!intest) {	
					if (slow) fprintf (stderr, "P = %6d O = %3d/%3d\r", id, (int)blon, (int)blat);
					continue;
				}
			
				if (full && slow) fprintf (stderr, "P = %6d I = %3d/%3d\r", id, (int)blon, (int)blat);

				/* Inside or on edge is inside here */
			
				if (irint ((double)grd[ij]) < blob[id].h.level) {
					grd[ij] = (double)blob[id].h.level;
					if (i == 0) grd[ij+nx_minus_1] = grd[ij];
					if (i == nx_minus_1) grd[ij-nx_minus_1] = grd[ij];
				}
				
			}
		}
	}
	
	fprintf (stderr, "\n");
	crude_free_int (IX, IY, N);
	
	fclose (fp);
	free ((void *)lon);
	free ((void *)lat);

	GMT_err_fail (GMT_write_grd (argv[3], &grdh, grd, 0.0, 0.0, 0.0, 0.0, GMT->current.io.pad, FALSE), argv[3]);
	
	exit (EXIT_SUCCESS);
}	
