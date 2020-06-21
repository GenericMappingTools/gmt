#include "gmt.h"
/*
 * Testing a specific case where we wish to pass and receive a GMT_VECTOR or GMT_MATRIX
 * to/from a module that expect to read/write GMT_DATASETs.
 */
int main () {
	void *API = NULL;                 /* The API control structure */
	struct GMT_VECTOR *V = NULL;	  /* Structure to hold input dataset as vectors */
	char input[GMT_VF_LEN] = {""};     			/* String to hold virtual input filename */
	char args[128] = {""};            			/* String to hold module command arguments */

	uint64_t dim[4] = {2, 2, 1, 0};
	double x[2] = {5.0, 5.0};
	double y[2] = {3.0, 8.0};
	double angle[2] = {30.0, 60.0};
	char *strings[2] = {"ML First label", "MR Second label"};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Create a dataset */
	if ((V = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/**/
	GMT_Put_Vector(API, V, 0, GMT_DOUBLE, x);
	GMT_Put_Vector(API, V, 1, GMT_DOUBLE, y);
	GMT_Put_Vector(API, V, 2, GMT_DOUBLE, angle);
	GMT_Put_Strings(API, GMT_IS_VECTOR, V, strings);
	/* Associate our data table with a virtual file */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN, V, input);
	/* Prepare the module arguments */
	sprintf (args, "%s -JX10c -R0/10/0/10 -Baf -F+a+j -Vd > testapi_vector_strings.ps", input);
	/* Call the psxy module */
	GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
