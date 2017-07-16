#include "gmt.h"
/*
 * Testing a specific case where we wish to pass and receive a GMT_MATRIX
 * to/from a module that expect to read/write GMT_GRIDs.
 */

#define INVARTYPE int
#define OUTVARTYPE double
#define ITYPE	GMT_INT
#define OTYPE	GMT_DOUBLE

int main () {
	unsigned int k, n_rows = 3, n_cols = 4;	/* Loop variable and dimensions*/
	uint64_t dim[3] = {n_rows, n_cols, 1};		/* nrows, ncols, nlayers */
    void *API = NULL;                 			/* The API control structure */
    struct GMT_MATRIX *M[2] = {NULL, NULL};     /* Structure to hold input/output grids as matrix */
	struct GMT_MATRIX *M2 = NULL;
    char input[GMT_STR16] = {""};     			/* String to hold virtual input filename */
    char output[GMT_STR16] = {""};    			/* String to hold virtual output filename */
    char args[128] = {""};            			/* String to hold module command arguments */
	INVARTYPE in_data[n_rows*n_cols];			/* Our user's input grid(matrix) data */
	OUTVARTYPE *out_data = NULL;				/* We will inspect the modified data via this pointer */

   /* Initialize the GMT session */
    API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);

	/* Create dummy user grid in_data */
	for (k = 0; k < n_rows * n_cols; k++) in_data[k] = k + 1;
 	/* Create a blank matrix container that will hold our user in_data */
	if ((M[GMT_IN] = GMT_Create_Data (API, GMT_IS_MATRIX, GMT_IS_SURFACE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) exit (EXIT_FAILURE);
	/* Hook the user matrix up to this containers */
	GMT_Put_Matrix (API, M[GMT_IN], ITYPE, in_data);
    /* Associate our data matrix with a virtual file to "read" from */
    GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, M[GMT_IN], input);
	/* Try to impose another output type than default float: */
	M2 = GMT_Create_Data (API, GMT_IS_MATRIX, GMT_IS_SURFACE, GMT_IS_OUTPUT, NULL, NULL, NULL, 0, 0, NULL);
	GMT_Put_Matrix (API, M[GMT_OUT], OTYPE, NULL);
    /* Open another virtual file to hold the modified matrix */
    GMT_Open_VirtualFile (API, GMT_IS_MATRIX, GMT_IS_SURFACE, GMT_OUT, M2, output);
    /* Prepare the module arguments to multiply grid by 10 */
    sprintf (args, "%s 10 MUL = %s", input, output);
    /* Call the grdmath module */
    GMT_Call_Module (API, "grdmath", GMT_MODULE_CMD, args);
    /* Obtain the matrix object from the virtual file */
    M[GMT_OUT] = GMT_Read_VirtualFile (API, output);
	/* Get the pointer to the modified user data */
	out_data = GMT_Get_Matrix (API, M[GMT_OUT]);
    /* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
	/* Print out the result */
	printf ("index\trow\tcol\tin_data\tout_data\n");
	for (k = 0; k < n_rows * n_cols; k++)
		printf ("%d\t%d\t%d\t%7.1f\t%7.1f\n", k, 1 + k / n_cols, 1 + k % n_cols, (float)in_data[k], (float)out_data[k]);
	/* Destroy session, which will free all GMT-allocated memory */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
