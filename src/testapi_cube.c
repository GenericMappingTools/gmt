#include "gmt_dev.h"
/*
 * Testing the use of user cubes
 * For now we can only write grid layers on output, not a single 3-D cube.
 * Once that changes the output name will just be cube-out.nc.
 */

int main () {
	/* Run the test using the specified in and out types */
	unsigned int mode = GMT_SESSION_EXTERNAL;
	double range[6] = {3.0, 8.0, 4.0, 6.0, 2.0, 4.0};
	struct GMT_CUBE *C = NULL;     /* Structure to hold input datasets as matrix */
	struct GMTAPI_CTRL *API = NULL;

	/* Initialize a GMT session */
	API = GMT_Create_Session ("test", 2U, mode, NULL);
 	/* Create a blank matrix container that will hold our user in_data */
	if ((C = GMT_Read_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, range, "cube.nc", NULL)) == NULL) return (EXIT_FAILURE);
	/* Hook the user input array up to this container */
	if (GMT_Write_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, "cube-out-%g.nc", C)) return (EXIT_FAILURE);
	/* Free the cube memory */
	GMT_Destroy_Data (API, &C);
	/* Destroy session, which will free all GMT-allocated memory */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
