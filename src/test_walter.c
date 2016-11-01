/* Walters code in principle.  To test we make a dummy grid thus:
       gmt grdmath -R0/100/0/100 -I1 X Y MUL = z.nc 
   The key issue to test is situations where we wish to sample a grid
   at multiple locations to obtain z, but we want full control over the
   x,y arrays which may be obtained or created by processes outside of
   GMT.  The solution is to hook such user arrays onto GMT containers
   (either GMT_VECTOR or GMT_MATRIX) and use GMT_Open_VirtualFile on these
   sources and destinations when relying on module i/o. The user must
   remember to set the n_rows dimension of the object so that GMT
   knows it has a fixed and inflexible size [By default, a GMT output
   container will expand to any size as required by the module] */

#include "gmt.h"

int main (int argc, char **argv) {
	void *API = NULL;
	uint64_t dim[2] = {0, 0}, p, k;
	double *x = NULL, *y = NULL;	/* User x,y arrays */
	int *z = NULL;	/* Just do demonstrate mixed types we use an integer user z array */
	struct GMT_GRID *G_in = NULL;
	struct GMT_VECTOR *V_in = NULL, *V_out = NULL;
	char grid[GMT_STR16] = {""}, input[GMT_STR16] = {""}, output[GMT_STR16] = {""};
	char args[256] = {""};
	(void)(argc);	
    /* Initialize a normal GMT session with 2 rows/cols for grid BC padding */
    if ((API = GMT_Create_Session (argv[0], 2U, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);
    /* Read in the grid to be used with grdtrack */
    G_in = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, "z.nc", NULL);
	/* Allocate two vector containers for input and output separately */
	dim[0] = 2;	/* Want two input columns but let length be 0 - this signals that no vector allocations should take place */
	if ((V_in = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) exit (EXIT_FAILURE);
	dim[0] = 3;	/* Want three output columns [we will share the first two with input] */
	if ((V_out = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) exit (EXIT_FAILURE);
	/* Allocate three custum vectors using malloc; here of length 7 (i.e., >= 5) */
	x = malloc (7*sizeof(double));	y = malloc (7*sizeof(double));	z = malloc (7*sizeof(int));
	/* Hook these up to our containers, reusing x,y as both in and out x/y vectors */
	GMT_Put_Vector (API, V_in,  GMT_X, GMT_DOUBLE, x);
	GMT_Put_Vector (API, V_in,  GMT_Y, GMT_DOUBLE, y);
	GMT_Put_Vector (API, V_out, GMT_X, GMT_DOUBLE, x);
	GMT_Put_Vector (API, V_out, GMT_Y, GMT_DOUBLE, y);
	GMT_Put_Vector (API, V_out, GMT_Z, GMT_INT,    z);
	V_in->n_rows = V_out->n_rows = 5;	/* Must specify how many input points we will simulate */
    /* Associate the grid with a virtual input file */
    GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, G_in, grid);
    /* Associate our input data vectors with a virtual input file */
    GMT_Open_VirtualFile (API, GMT_IS_VECTOR, GMT_IS_POINT, GMT_IN, V_in, input);
    /* Associate our output data vectors with a virtual output file */
    GMT_Open_VirtualFile (API, GMT_IS_VECTOR, GMT_IS_POINT, GMT_OUT, V_out, output);
    /* Prepare the grdtrack command-line arguments, selecting bilinear sampling */
    sprintf (args, "-G%s %s -nl > %s", grid, input, output);
	for (k = 0; k < 5; k++) {	/* Repeat our experiment 5 times, getting different random input points */
		/* Initialize the virtual files to their original state so they can be reused */
		GMT_Init_VirtualFile (API, 0, grid);
		GMT_Init_VirtualFile (API, 0, input);
		GMT_Init_VirtualFile (API, 0, output);
		/* Obtain new x,y locations for sampling the grid */
		for (p = 0; p < 5; p++) {
			x[p] = (double)100.0*rand() / (double)RAND_MAX;	/* Get x-value in 0-100 range */
			y[p] = (double)100.0*rand() / (double)RAND_MAX;	/* Get y-value in 0-100 range */
		}
    	/* Run the grdtrack module */
    	if (GMT_Call_Module (API, "grdtrack", GMT_MODULE_CMD, args)) exit (EXIT_FAILURE);
		/* Report the results to stdout */
		printf ("\nesult of simulation number %d:\n", (int)k);
		for (p = 0; p < 5; p++)
			printf ("%g\t%g\t%d\n", x[p], y[p], z[p]);
	}
	/* Close the three virtual files */
	GMT_Close_VirtualFile (API, grid);
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
    /* Destroy the GMT session */
    if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	/* Free our custom vectors */
	free (x);	free (y);	free (z);
}