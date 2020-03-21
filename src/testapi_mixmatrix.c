#include "gmt_dev.h"
/*
 * Testing the use of user data provided via a GMT_MATRIX
 * to psxy and pstext, passing both a coordinate matrix and a string array.
 */

/* Dimensions of our test dataset */
#define NROWS	3
#define NCOLS	2
#define NM	(NROWS * NCOLS)

int main () {
	/* Run the test using the specified in and out types */
	uint64_t dim[4] = {NCOLS, NROWS, 1, 0};		/* ncols, nrows, nlayers, type */
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_MATRIX *M = NULL;     /* Structure to hold input datasets as matrix */
	char input[GMT_VF_LEN] = {""};    /* String to hold virtual input filename */
	char args[128] = {""};           /* String to hold module command arguments */
	int coord[NM] = {1, 2, 2, 3, 3, 4};	/* 3 points */
	char *strings[NROWS] = {"First label", "Second label", "Third label"};
	struct GMTAPI_CTRL *API = NULL;

	/* Initialize a GMT session */
	API = GMT_Create_Session ("test", 2U, mode, NULL);
 	/* Create a blank matrix container that will hold our user in_data */
	if ((M = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/* Hook the user input array up to this container */
	GMT_Put_Matrix (API, M, GMT_INT, 0, coord);
	/* Hook the user text array up to this container */
	GMT_Put_Strings (API, GMT_IS_MATRIX, M, strings);
	/* Associate our matrix container with a virtual dataset file to "read" from */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, M, input);
	/* Prepare the module arguments to plot the points in psxy */
	sprintf (args, "%s -R0/5/0/5 -JX5i -P -Baf -Sc0.1i -Gred -K > testapi_mixmatrix.ps", input);
	/* Call the psxy module */
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, args);
	/* Reuse the virtual file */
	GMT_Init_VirtualFile (API, 0, input);
	/* Prepare the module arguments to plot the strings in pstext */
	sprintf (args, "%s -R -J -O -F+f14p+jLB -DJ0.1i >> testapi_mixmatrix.ps", input);
	/* Call the pstext module */
	GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, args);
	/* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	/* Destroy session, which will free all GMT-allocated memory */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
