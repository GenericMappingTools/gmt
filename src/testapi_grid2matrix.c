#include "gmt_dev.h"

/* Testing reading a grid into a matrix, then writing the matrix to a grid file.
 * The test script apigrd2mat,sh will compare the two.
 */

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_MATRIX *M = NULL;
	struct GMTAPI_CTRL *API = NULL;

	API = GMT_Create_Session ("test", 2U, mode, NULL);

	/* Read in earth_relief_01d as a matrix from the remote grid */
	M = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, "@earth_relief_01d", NULL);
	/* Write out the matrix to another grid file */
	GMT_Write_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_WRITE_NORMAL, NULL, "dump_01d.grd", M);

	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
