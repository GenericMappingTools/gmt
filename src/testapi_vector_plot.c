#include "gmt.h"
int main () {
	void *API = NULL;                 /* The API control structure */
	struct GMT_VECTOR *V = NULL;	  /* Structure to hold input dataset as vectors */
	char input[GMT_VF_LEN] = {""};     			/* String to hold virtual input filename */
	char args[128] = {""};            			/* String to hold module command arguments */

	uint64_t dim[4] = {2, 2, 1, 0};
	double x[2] = {5.0, 5.0};
	double y[2] = {3.0, 8.0};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Create a dataset */
	if ((V = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/**/
	GMT_Put_Vector(API, V, 0, GMT_DOUBLE, x);
	GMT_Put_Vector(API, V, 1, GMT_DOUBLE, y);
	/* Associate our data table with a virtual file */
	//GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, V, input);
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, V, input);
	/* Prepare the module arguments */
	sprintf (args, "%s -JX10c -R0/10/0/10 -Baf -W1p,black+ve0.2c -P", input);
	/* Call the psxy module */
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
