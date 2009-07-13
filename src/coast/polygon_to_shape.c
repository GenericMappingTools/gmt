/*
 *	$Id: polygon_to_shape.c,v 1.2 2009-07-13 18:49:36 guru Exp $
 * 
 *	Reads a polygon file and creates a multisegment GMT file with
 *	appropriate GIS tags so ogr2ogr can convert it to a shapefile.
 */

#include "wvs.h"

struct POLYGON {
	struct GMT3_POLY h;
	struct	LONGPAIR *p;
} P[N_POLY];


int main (int argc, char **argv)
{
	FILE *fp_in, *fp;
	int n_id = 0, id, k, level, x, ymin = M90, ymax = -M90;
	char file[BUFSIZ], cmd[BUFSIZ], *SRC[2] = {"WDBII", "WVS"};
        
	if (argc < 2 || argc > 3) {
		fprintf (stderr,"usage:  polygon_to_shape file_res.b prefix\n");
		fprintf (stderr,"	file_res.b is the binary local file with all polygon info for a resolution\n");
		fprintf (stderr,"	prefix is used to form the files prefix_L[1-4].gmt\n");
		fprintf (stderr,"	These are then converted to shapefiles via ogr2ogr\n");
		exit (EXIT_FAILURE);
	}
	fp_in = fopen(argv[1], "r");
		
	while (pol_readheader (&P[n_id].h, fp_in) == 1) {
		P[n_id].p = (struct LONGPAIR *) GMT_memory (VNULL, P[n_id].h.n, sizeof (struct LONGPAIR), "polygon_to_shape");
		if (pol_fread (P[n_id].p, P[n_id].h.n, fp_in) != P[n_id].h.n) {
			fprintf(stderr,"polygon_to_shape:  ERROR  reading file.\n");
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

	for (level = 1; level <= 4; level++) {	/* Make separate files for each level*/
		sprintf (file, "%s_L%d.gmt", argv[2], level);
		if ((fp = fopen (file, "w")) == NULL) {
			fprintf(stderr,"polygon_to_shape:  ERROR  creating file %s.\n", file);
			exit(-1);
		}
		fprintf (fp, "# @VGMT­1.0 @GPOLYGON @NGSHHS_id|GSHHS_level|GSHHS_source|GSHHS_parent_id|GSHHS_sibling_id|GSHHS_area @Tinteger|integer|char|integer|integer|double\n");
		fprintf (fp, "# @R-180/180/%.6f/%.6f @Jp\"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs\"\n# FEATURE DATA\n", ymin*I_MILL, ymax*I_MILL);
		for (id = 0; id < n_id; id++) {
			if (P[id].h.level != level) continue;
			/* Here we found a polygon of the required level.  Write out polygon tag and info */
			
			fprintf (fp, "> GSHHS polygon Id = %d Level = %d Area = %.12g\n# @P @D%d|%d|%s|%d|%d|%.12g\n",
				P[id].h.id, P[id].h.level, P[id].h.area, P[id].h.id, P[id].h.level, SRC[P[id].h.source], P[id].h.parent, P[id].h.ancestor, P[id].h.area);
			for (k = 0; k < P[id].h.n; k++) {
				x = (P[id].p[k].x > P[id].h.datelon) ? P[id].p[k].x - M360 : P[id].p[k].x;
				fprintf (fp, "%.6f\t%.6f\n", x * I_MILL, P[id].p[k].y * I_MILL);
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
	
	exit (EXIT_SUCCESS);
}
