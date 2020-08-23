#include "gmt.h"
#include <string.h>

int main (int argc, char **argv) {
	void *API = NULL;                /* The API control structure */
	struct GMT_MATRIX *M = NULL;    /* Structure to hold input matrix */
	char input[GMT_VF_LEN] = {""};	/* String to hold virtual input filename */
	char args[128] = {""};         	/* String to hold module command arguments */
	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	M = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, "2d-matrix.txt", NULL);
	/* Associate our matrix with a virtual file */
	GMT_Open_VirtualFile (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_IN, M, input);

	/* Prepare the module arguments */
	if (argc >= 2 && !strcmp (argv[1], "-I")) /* Want shading */
		sprintf (args, "%s -JX6i -P -Baf -Cgeo -I+d", input);
	else /* Just the image */
		sprintf (args, "%s -JX6i -P -Baf -Cgeo", input);
	/* Call the grdimage module */
	GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, args);

	/* Close the virtual file */
	GMT_Close_VirtualFile (API, input);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
