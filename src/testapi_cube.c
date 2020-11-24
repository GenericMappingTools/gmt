#include "gmt.h"
/*
 * Testing the use of user cubes
 * For now we can only write grid layers on output, not a single 3-D cube.
 * Once that changes the output name will just be cube-out.nc.
 */

int main () {
	/* Run the test using the specified in and out types */
	char input[GMT_VF_LEN] = {""}, output[GMT_VF_LEN] = {""}, cmd[256] = {""};
	unsigned int mode = GMT_SESSION_EXTERNAL;
	double range[6] = {3.0, 8.0, 4.0, 6.0, 2.0, 4.0};
	struct GMT_CUBE *C = NULL, *C2 = NULL;     /* Structure to hold cubes */
	struct GMTAPI_CTRL *API = NULL;

	/* Initialize a GMT session */
	API = GMT_Create_Session ("test", 2U, mode, NULL);
 	/* Read in a small 3-D data cube from file */
	if ((C = GMT_Read_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, range, "cube.nc", NULL)) == NULL) return (EXIT_FAILURE);
	/* Test basic writing of cube to file (for now a stack of 2-D grids) */
	if (GMT_Write_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, "cube-out-%g.nc", C)) return (EXIT_FAILURE);
	/* Interpolate the cube by passing it as a memory object to sphinterpolate and retrieve a memory cube*/
	GMT_Open_VirtualFile (API, GMT_IS_CUBE, GMT_IS_VOLUME, GMT_IN, C, input);
	GMT_Open_VirtualFile (API, GMT_IS_CUBE, GMT_IS_VOLUME, GMT_OUT, NULL, output);
	sprintf (cmd, "%s -T4 -G%s", input, output);
	/* Run the command */
	GMT_Call_Module (API, "grdinterpolate", GMT_MODULE_CMD, cmd);
	/* Get the data cube from virtual file */
	C2 = GMT_Read_VirtualFile (API, output);
	/* Write the grid to file */
	if (GMT_Write_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, "newlayer.nc", C2)) return (EXIT_FAILURE);
	/* Free the cube memories */
	GMT_Destroy_Data (API, &C);
	GMT_Destroy_Data (API, &C2);
	/* Destroy session, which will free all GMT-allocated memory */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
