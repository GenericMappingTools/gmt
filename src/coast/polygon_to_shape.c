/*
 *	$Id$
 * 
 *	Reads a polygon file and creates a multisegment GMT file with
 *	appropriate GIS tags so ogr2ogr can convert it to a shapefile.
 */

#include "gmt.h"
#include "wvs.h"

#define M270 270000000

struct POLYGON {
	struct GMT3_POLY h;
	struct	LONGPAIR *p;
} P[N_POLY];


int main (int argc, char **argv)
{
	FILE *fp_in, *fp;
	int n_id = 0, id, k, level, x, x0, y0, ymin = M90, ymax = -M90, hemi, first;
	GMT_LONG np, nx;
	char file[GMT_BUFSIZ], cmd[GMT_BUFSIZ], *SRC[2] = {"WDBII", "WVS"}, *H = "EW";
	double*lon = NULL, *lat = NULL, *xx, *yy;
	EXTERN_MSC GMT_LONG GMT_wesn_clip (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx);
        
	argc = GMT_begin (argc, argv);
	
	/* Set up some GMT variables needed later */
	GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;
	GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;
	/* Supply dummy linear proj */
	GMT->current.proj.projection = GMT->current.proj.xyz_projection[GMT_X] = GMT->current.proj.xyz_projection[GMT_Y] = GMT_LINEAR;
	GMT->current.proj.pars[0] = GMT->current.proj.pars[1] = 1.0;
	GMT_err_fail (GMT_map_setup (-180.0, 180.0, -90.0, 90.0), "");
	
	if (argc < 2 || argc > 3) {
		fprintf (stderr,"usage: polygon_to_shape file_res.b prefix\n");
		fprintf (stderr,"	file_res.b is the binary local file with all polygon info for a resolution\n");
		fprintf (stderr,"	prefix is used to form the files prefix_L[1-4].gmt\n");
		fprintf (stderr,"	These are then converted to shapefiles via ogr2ogr\n");
		exit (EXIT_FAILURE);
	}
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&P[n_id].h, fp_in) == 1) {
		P[n_id].p = (struct LONGPAIR *) GMT_memory (VNULL, P[n_id].h.n, sizeof (struct LONGPAIR), "polygon_to_shape");
		if (pol_fread (P[n_id].p, P[n_id].h.n, fp_in) != P[n_id].h.n) {
			fprintf(stderr,"polygon_to_shape: Error reading file.\n");
			exit(-1);
		}
		for (k = 0; k < P[n_id].h.n; k++) {
			if (P[n_id].p[k].y < ymin) ymin = P[n_id].p[k].y;
			if (P[n_id].p[k].y > ymax) ymax = P[n_id].p[k].y;
		}
		n_id++;
	}
	fclose (fp_in);
	fprintf (stderr, "polygon_to_shape: Found %d polygons\n", n_id);

	GMT_err_fail (GMT_map_setup (-180, 180, -90.0, 90.0), "");
	for (level = 1; level <= 4; level++) {	/* Make separate files for each level*/
		sprintf (file, "%s_L%d.gmt", argv[2], level);
		if ((fp = fopen (file, "w")) == NULL) {
			fprintf(stderr,"polygon_to_shape: Error creating file %s.\n", file);
			exit(-1);
		}
		fprintf (fp, "# @VGMT­1.0 @GPOLYGON @Nid|level|source|parent_id|sibling_id|area @Tchar|integer|char|integer|integer|double\n");
/*		fprintf (fp, "# @R-180/180/%.6f/%.6f @Jp\"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs\"\n# FEATURE DATA\n", ymin*I_MILL, ymax*I_MILL); */
		fprintf (fp, "# @R-180/180/-90/%.6f @Jp\"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs\"\n# FEATURE DATA\n", ymax*I_MILL);
		for (id = 0; id < n_id; id++) {
			if (P[id].h.level != level) continue;
			/* Here we found a polygon of the required level.  Write out polygon tag and info */
			
			if (P[id].h.id == 4) {
				P[id].h.n--;	/* Skip the duplicate point */
				for (hemi = 0; hemi < 2; hemi++) {
					fprintf (fp, "> GSHHS polygon Id = %d-%c Level = %d Area = %.12g\n# @P @D%d-%c|%d|%s|%d|%d|%.12g\n",
						P[id].h.id, H[hemi], P[id].h.level, P[id].h.area, P[id].h.id, H[hemi], P[id].h.level, SRC[P[id].h.source], P[id].h.parent, P[id].h.ancestor, P[id].h.area);
					for (k = 0, first = TRUE; k < P[id].h.n; k++) {	/* Set up lons that go -20 to + 192 */
						if (hemi == 0) {
							if (P[id].p[k].x > M180) continue;
							x = P[id].p[k].x;
							if (first) {x0 = x; y0 = P[id].p[k].y;}
						}
						else if (hemi == 1) {
							if (P[id].p[k].x < M180) continue;
							x = P[id].p[k].x - M360;
							if (first) {x0 = x; y0 = P[id].p[k].y;}
						}
						fprintf (fp, "%.6f\t%.6f\n", x * I_MILL, P[id].p[k].y * I_MILL);
						first = FALSE;
					}
					x = (hemi == 0) ? 0 : -M180;
					fprintf (fp, "%.6f\t%.6f\n%.6f\t%.6f\n%.6f\t%.6f\n", x * I_MILL, -90.0, x0 * I_MILL, -90.0, x0 * I_MILL, y0 * I_MILL);
				}
				
			}
			else if (P[id].h.west < 180.0 && P[id].h.east > 180.0) {	/* Straddles dateline; must split into two parts thanx to GIS brilliance */
				lon = (double *)GMT_memory (VNULL, sizeof (double), P[id].h.n, GMT->init.progname);
				lat = (double *)GMT_memory (VNULL, sizeof (double), P[id].h.n, GMT->init.progname);
				for (k = 0; k < P[id].h.n; k++) {	/* Set up lons that go -20 to + 192 */
					x = (P[id].p[k].x > M270) ? P[id].p[k].x - M360 : P[id].p[k].x;
					lon[k] = x * I_MILL;	lat[k] = P[id].p[k].y * I_MILL;
				}
				for (hemi = 0; hemi < 2; hemi++) {
					if ((np = GMT_wesn_clip (lon, lat, P[id].h.n, &xx, &yy, &nx)) == 0) {
						fprintf (stderr, "%s: Error: Straddling 180 but not two parts?\n", GMT->init.progname);
						continue;
					}
					fprintf (fp, "> GSHHS polygon Id = %d-%c Level = %d Area = %.12g\n# @P @D%d-%c|%d|%s|%d|%d|%.12g\n",
						P[id].h.id, H[hemi], P[id].h.level, P[id].h.area, P[id].h.id, H[hemi], P[id].h.level, SRC[P[id].h.source], P[id].h.parent, P[id].h.ancestor, P[id].h.area);
					for (k = 0; k < np; k++) GMT_xy_to_geo (&xx[k], &yy[k], xx[k], yy[k]);	/* Undo projection first */
					fprintf (fp, "%.6f\t%.6f\n", xx[0], yy[0]);
					for (k = 1; k < np; k++) {
						if (!( GMT_IS_ZERO(xx[k]-xx[k-1]) && GMT_IS_ZERO(yy[k]-yy[k-1]))) fprintf (fp, "%.6f\t%.6f\n", xx[k], yy[k]);
					}
					GMT_free ((void *)xx);	GMT_free ((void *)yy);
					for (k = 0; k < P[id].h.n; k++) lon[k] -= 360.0;	/* Set up lons that go -360 to -tiny */
				}
				GMT_free ((void *)lon);	GMT_free ((void *)lat);
			}
			else {	/* No problems, just write as is */
				fprintf (fp, "> GSHHS polygon Id = %d Level = %d Area = %.12g\n# @P @D%d|%d|%s|%d|%d|%.12g\n",
					P[id].h.id, P[id].h.level, P[id].h.area, P[id].h.id, P[id].h.level, SRC[P[id].h.source], P[id].h.parent, P[id].h.ancestor, P[id].h.area);
				for (k = 0; k < P[id].h.n; k++) {
					x = (P[id].p[k].x > P[id].h.datelon) ? P[id].p[k].x - M360 : P[id].p[k].x;
					fprintf (fp, "%.6f\t%.6f\n", x * I_MILL, P[id].p[k].y * I_MILL);
				}
			}
		}
		fclose (fp);	/* Done with this set */
	}
	
	for (id = 0; id < n_id; id++) GMT_free ((void *)P[id].p);
	
	fprintf (stderr,"Now convert to ESRI Shapefiles: ");
	for (level = 1; level <= 4; level++) {	/* Make separate files for each level*/
		sprintf (cmd, "ogr2ogr -f \"ESRI Shapefile\" %s_L%d %s_L%d.gmt\n", argv[2], level, argv[2], level);
		system (cmd);
		fprintf (stderr, "%d", level);
	}
	fprintf (stderr," done\nThe shapefiles will be in directories %s_L[1-4]\n", argv[2]);
	
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}
