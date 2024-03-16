#include "gmt.h"
/*
 * Testing the use of user grid memory allocated externally.
 */

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_GRID *G = NULL;
	struct GMTAPI_CTRL *API = NULL;
	uint64_t dim[2] = {5, 4};
	char args[1000] = {""};
	char input[GMT_VF_LEN] = {""};
	gmt_grdfloat *data = NULL;

	API = GMT_Create_Session ("testapi_gmtgrid", 2U, mode, NULL);

	/* Create a container for the grid */
	G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 2, NULL);

	/* Allocate memory of the data array in the external program (C or PyGMT) */
	data = (gmt_grdfloat *)malloc(sizeof(gmt_grdfloat) * 9 * 8);
	for (int i = 0; i < 9 * 8; i++) data[i] = (gmt_grdfloat)i;

	/* Assign the user data to the GMT_GRID structure */
	G->data = data;

	/* Using the grid */
	GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, G, input);
	sprintf (args, "%s", input);
	GMT_Call_Module (API, "grd2xyz", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);

	/* Free the data array allocated by the external program */
	free (data);

	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
