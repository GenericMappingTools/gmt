#include "gmt.h"
/*
 * Testing the use of user data provided via a GMT_VECTOR to pstext,
 * passing both numerical vectors and a string array.
 */

/* Dimensions of the test dataset */
#define NCOLS 3
#define NROWS 2

int main () {
	void *API = NULL;							/* The API control structure */
	struct GMT_VECTOR *V = NULL;				/* Structure to hold input dataset as vectors */
	char input[GMT_VF_LEN] = {""};     			/* String to hold virtual input filename */
	char args[128] = {""};            			/* String to hold module command arguments */

	uint64_t dim[4] = {NCOLS, NROWS, 1, 0};		/* ncols, nrows, nlayers, type */
	/* two data points */
	double x[2] = {5.0, 5.0};
	double y[2] = {3.0, 8.0};
	double angle[2] = {30.0, 60.0};
	char *strings[NROWS] = {"ML 18p,1,blue First label", "MR 32p,2,red Second label"};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Create a dataset */
	if ((V = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/* Hook the three vectors up to this container */
	GMT_Put_Vector(API, V, 0, GMT_DOUBLE, x);
	GMT_Put_Vector(API, V, 1, GMT_DOUBLE, y);
	GMT_Put_Vector(API, V, 2, GMT_DOUBLE, angle);
	/* Hook the user text array up to this container */
	GMT_Put_Strings(API, GMT_IS_VECTOR, V, strings);
	/* Associate our data table with a virtual file */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, V, input);
	/* Prepare the module arguments */
	sprintf (args, "%s -JX10c -R0/10/0/10 -Baf -F+a+j+f", input);
	/* Call the pstext module */
	GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
