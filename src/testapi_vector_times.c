#include "gmt.h"
int main () {
	void *API = NULL;                 /* The API control structure */
	struct GMT_VECTOR *V = NULL;	  /* Structure to hold input dataset as vectors */
	char input[GMT_VF_LEN] = {""};     			/* String to hold virtual input filename */
	char args[128] = {""};            			/* String to hold module command arguments */

	uint64_t dim[4] = {2, 4, 1, 0};
	char *x[4] = {"2021-03-01", "2021-03-02", "2021-03-03", "2021-03-04"};
	double y[4] = {0.0, 1.0, 2.0, 3.0};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Create a dataset */
	if ((V = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/**/
	GMT_Put_Vector(API, V, 0, GMT_TEXT, x);
	GMT_Put_Vector(API, V, 1, GMT_DOUBLE, y);
	/* Associate our data table with a virtual file */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, V, input);
	/* Prepare the module arguments */
	sprintf (args, "%s -JX10c/5c -R2021-03-01/2021-03-04/-0.1/30 -Baf -BWSen -P", input);
	/* Call the psxy module */
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
