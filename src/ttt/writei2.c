/*
 * writei2.c
 *
 * writei2.c - Reformat global ASCII bathy data into a GMT compatible *.i2 files
 * 		This version assumes the 0.5 min grid is pixel registered
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2024
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

/* Reproduce GMT grid header to use in TTT */

#define GRD_COMMAND_LEN 320
#define GRD_REMARK_LEN  160
#define GRD_TITLE_LEN    80
#define GRD_UNIT_LEN     80

#define ARRAY_SIZE	1048576L

struct GRD_HEADER {     /* Based on public GMT Documentation */
        int nx;                         /* Number of columns */
        int ny;                         /* Number of rows */
        int node_offset;                /* 0 for node grids, 1 for pixel grids */
        double x_min;                   /* Minimum x coordinate */
        double x_max;                   /* Maximum x coordinate */
        double y_min;                   /* Minimum y coordinate */
        double y_max;                   /* Maximum y coordinate */
        double z_min;                   /* Minimum z value */
        double z_max;                   /* Maximum z value */
        double x_inc;                   /* x increment */
        double y_inc;                   /* y increment */
        double z_scale_factor;          /* grd values must be multiplied by this */
        double z_add_offset;            /* After scaling, add this */
        char x_units[GRD_UNIT_LEN];     /* units in x-direction */
        char y_units[GRD_UNIT_LEN];     /* units in y-direction */
        char z_units[GRD_UNIT_LEN];     /* grid value units */
        char title[GRD_TITLE_LEN];      /* name of data set */
        char command[GRD_COMMAND_LEN];  /* name of generating command */
        char remark[GRD_REMARK_LEN];    /* comments re this data set */
};

int main (int argc, char **argv)
{
	size_t ij, nm, k;
	double dx;
	short int z[ARRAY_SIZE];
	char line[BUFSIZ];
	FILE *fp = NULL;
	struct GRD_HEADER h;
	
	if (argc != 7) {
		fprintf (stderr, "writei2 - Converting Geoware z data to a GMT native binary short int grdfile\n\n");
		fprintf (stderr, "usage: writei2 <west> <east> <south> <north> <dx_in_min> <file.i2> < inputstream\n");
		exit (-1);
	}

	if ((fp = fopen (argv[6], "wb")) == NULL) {
		fprintf (stderr, "writei2: Failed to create %s\n", argv[6]);
		exit (-1);
	}
	
	memset ((void *)&h, sizeof (struct GRD_HEADER), 0);
	h.x_min = atof (argv[1]);	/* West */
	h.x_max = atof (argv[2]);	/* East */
	h.y_min = atof (argv[3]);	/* South */
	h.y_max = atof (argv[4]);	/* North */
	dx = atof (argv[5]);		/* dx and dy in minutes */
	h.x_inc = h.y_inc = dx / 60.0;
	h.z_scale_factor = 1.0;		/* scale */
	h.z_add_offset = 0.0;		/* offset */
	if (dx < 1.0) {
		sprintf (h.title, "%d arc second global bathymetry for TTT 3.2", (int)(60.0 * dx + 0.00005));
		h.node_offset = 0;		/* gridline registrated */
	}
	else {
		sprintf (h.title, "%d arc minute global bathymetry for TTT 3.2", (int)(dx +0.00005));
		h.node_offset = 1;		/* pixel registrated */
	}
	strcpy (h.remark, "Produced by Geoware, 2024");
	strcpy (h.command, "Converted from ASCII z-tables using utility writei2");
	strcpy (h.x_units,  "degree");
	strcpy (h.y_units,  "degree");
	strcpy (h.z_units,  "m");

	h.nx = (int) floor ((h.x_max - h.x_min) / h.x_inc + (1 - h.node_offset) + 0.00005);
	h.ny = (int) floor ((h.y_max - h.y_min) / h.y_inc + (1 - h.node_offset) + 0.00005);
	nm = ((size_t)h.nx) * ((size_t)h.ny);

	h.z_min = DBL_MAX;
	h.z_max = -DBL_MAX;
	
	/* Because GRD_HEADER is not 64-bit aligned we must write it in parts */
	if (fwrite ((void *)&h.nx, 3*sizeof (int), (size_t)1, fp) != 1 || fwrite ((void *)&h.x_min, sizeof (struct GRD_HEADER) - ((long)&h.x_min - (long)&h.nx), (size_t)1, fp) != 1) {
                fprintf (stderr, "TTT write Fatal Error: Error writing file %s!\n", argv[7]);
                exit (EXIT_FAILURE);
        }
	for (ij = k = 0; ij < nm; ij++) {
		fgets (line, BUFSIZ, stdin);
		if (line[0] == 'N') 
			z[k] = SHRT_MAX;
		else {
			z[k] = (short int)atoi (line);
			if ((double)z[k] < h.z_min) h.z_min = (double)z[k];
			if ((double)z[k] > h.z_max) h.z_max = (double)z[k];
		}
		k++;
		if (k == ARRAY_SIZE) {	/* Time to write a chunk */
			fwrite ((void *)z, sizeof (short int), ARRAY_SIZE, fp);			/* Write z array chunk */
			k = 0;
		}
	}
	if (k) fwrite ((void *)z, sizeof (short int), k, fp);			/* Write remainder of z array */
	/* Rewrite header now that zmin/max have been obtained */
	fseek (fp, 0L, SEEK_SET);	/* Go back to start of file */
	if (fwrite ((void *)&h.nx, 3*sizeof (int), (size_t)1, fp) != 1 || fwrite ((void *)&h.x_min, sizeof (struct GRD_HEADER) - ((long)&h.x_min - (long)&h.nx), (size_t)1, fp) != 1) {
                fprintf (stderr, "TTT write Fatal Error: Error writing file %s!\n", argv[7]);
                exit (EXIT_FAILURE);
        }

	fclose (fp);
	
	exit (0);
}
