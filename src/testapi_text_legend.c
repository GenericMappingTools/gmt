/* Testing the case of passing a virtual dataset for paragraph text to pslegend */

#include "gmt.h"
#include <string.h>

int main () {
	void *API = NULL;                 /* The API control structure */
	struct GMT_DATASET *D = NULL;	  /* Structure to hold input dataset as vectors */
	struct GMT_DATASEGMENT *S = NULL;	  /* Structure to hold input dataset as vectors */
	char input[GMT_VF_LEN] = {""};     			/* String to hold virtual input filename */
	char args[128] = {""};            			/* String to hold module command arguments */

	uint64_t dim[4] = {1, 1, 2, 0};	/* one table with one segment with two rows and no columns (just trailing text) */

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Create a dataset */
	if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (EXIT_FAILURE);
	/**/
	S = D->table[0]->segment[0];	/* Get the pointer to the only segment and add specific strings */
	S->text[0] = strdup ("P");
	S->text[1] = strdup ("T d = [0 0; 1 1; 2 1; 3 0.5; 2 0.25]");
	/* Associate our data table with a virtual file */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN|GMT_IS_REFERENCE, D, input);
	/* Prepare the module arguments */
	sprintf (args, "%s -R-3/3/-3/3 -JX12 -Baf -BWSen -F+p+ggray -Dg-1.8/2.6+w12c+jTL -P\n", input);

	/* Call the pslegend module */
	GMT_Call_Module (API, "pslegend", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
