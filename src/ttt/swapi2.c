/*
 * swapi2.c
 *
 * swapi2.c - Byte-swap short in GMT native grids
 * Fixes two problems:
 * 1) Binary grids were made on bigendian hardware, whereas PCs are little-endian
 * 2) Also was made by mistake on G5 before headerfix, thus having a 4-byte pad
 *    in the middle of the header structure.
 *
 * Author:	Paul Wessel
 * Date:	24-MAR-2024
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#define swab_2(data) ((((data) & 0xff) << 8) | ((unsigned short) (data) >> 8))
#define swab_4(data) \
        (((data) << 24) | (((data) << 8) & 0x00ff0000) | \
        (((data) >> 8) & 0x0000ff00) | ((unsigned int)(data) >> 24))

/* Reproduce GMT grid header to use in TTT */

#define GRD_COMMAND_LEN 320
#define GRD_REMARK_LEN  160
#define GRD_TITLE_LEN    80
#define GRD_UNIT_LEN     80

struct GRD_HEADER {     /* Based on public GMT Documentation */
        int nx;                         /* Number of columns */
        int ny;                         /* Number of rows */
        int node_offset;                /* 0 for node grids, 1 for pixel grids */
        int pad;                /*BAD GRID WAS WRITTEN ON G5 64-BIT */
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
	int i, nm, flip = 1;
	short int int2, *z = NULL;
	FILE *fp = NULL;
	struct GRD_HEADER h;
	void double_swab (double *x, int flip);
	
	if (argc != 3) {
		fprintf (stderr, "swapi2 - Byte-swap for TTT short int grids\n\n");
		fprintf (stderr, "usage: swapi2 oldfile newfile\n");
		exit (-1);
	}

	if ((fp = fopen (argv[1], "rb")) == NULL) {
		fprintf (stderr, "swapi2: Failed to open file %s for reading\n", argv[1]);
		exit (-1);
	}

	if (fread ((void *)&h, sizeof (struct GRD_HEADER), (size_t)1, fp) != 1) {
                fprintf (stderr, "swapi2: Error reading header from file %s!\n", argv[1]);
                exit (EXIT_FAILURE);
        }
	
	/* Swap header bytes */
	
	h.nx = swab_4 (h.nx);
	h.ny = swab_4 (h.ny);
	h.node_offset = swab_4 (h.node_offset);
	double_swab (&h.x_min, flip);
	double_swab (&h.x_max, flip);
	double_swab (&h.y_min, flip);
	double_swab (&h.y_max, flip);
	double_swab (&h.z_min, flip);
	double_swab (&h.z_max, flip);
	double_swab (&h.x_inc, flip);
	double_swab (&h.y_inc, flip);
	double_swab (&h.z_scale_factor, flip);
	double_swab (&h.z_add_offset, flip);
	
	nm = h.nx * h.ny;

	z = (short int *) calloc ((size_t)nm, sizeof (short int));
	if (fread ((void *)z, sizeof (short int), nm, fp) != (size_t)nm) {
		fprintf (stderr, "swapi2: Error reading grid array from file %s!\n", argv[1]);
		exit (EXIT_FAILURE);
 	}
	if (fclose (fp)) {
		fprintf (stderr, "swapi2: Error closing file %s!\n", argv[1]);
		exit (EXIT_FAILURE);
	}
	
	h.z_min = DBL_MAX;
	h.z_max = -DBL_MAX;
	for (i = 0; i < nm; i++) {
		int2 = swab_2 (z[i]);
		z[i] = int2;
		if (z[i] == SHRT_MAX || z[i] == SHRT_MIN) continue;	/* Skip the NaNs */
		if ((double)z[i] < h.z_min) h.z_min = (double)z[i];
		if ((double)z[i] > h.z_max) h.z_max = (double)z[i];
	}

	if ((fp = fopen (argv[2], "wb")) == NULL) {
		fprintf (stderr, "swapi2: Failed to overwrite gridfile %s\n", argv[2]);
		exit (-1);
	}
	
	/* Because GRD_HEADER is not 64-bit aligned we must write it in parts */
	
	if (fwrite ((void *)&h.nx, 3*sizeof (int), (size_t)1, fp) != 1 || fwrite ((void *)&h.x_min, sizeof (struct GRD_HEADER) - ((long)&h.x_min - (long)&h.nx), (size_t)1, fp) != 1) {
                fprintf (stderr, "swapi2: Error writing header to file %s!\n", argv[2]);
                exit (EXIT_FAILURE);
        }
	if (fwrite ((void *)z, sizeof (short int), nm, fp) != (size_t)nm) {
		fprintf (stderr, "swapi2: Error writing grid array to file %s!\n", argv[2]);
		exit (EXIT_FAILURE);
 	}
	if (fclose (fp)) {
		fprintf (stderr, "swapi2: Error closing file %s!\n", argv[2]);
		exit (EXIT_FAILURE);
	}
	
	free ((void *)z);
	
	fprintf (stderr, "swapi2: Corrected %s -> %s\n", argv[1], argv[2]);

	exit (0);
}

void double_swab (double *x, int flip)
{	/* byte swap a double precision value */
	unsigned int *p, save;
	
	p = (unsigned int *)x;				/* Now, p[0] and p[1] are the two 4-byte words */
	if (flip) {
		save = p[0];	p[0] = p[1];	p[1] = save;	/* Exchange the two words */
	}
	p[0] = swab_4 (p[0]);	p[1] = swab_4 (p[1]);	/* Swab the byte order, and x is now swabed too */
	if (fabs(*x) < 1e-200) *x = 0.0;
}
