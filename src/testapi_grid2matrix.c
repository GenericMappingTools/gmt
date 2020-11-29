#include "gmt.h"

/* Testing reading a grid into a matrix, then writing the matrix to a grid file.
 * The test script apigrd2mat,sh will compare the two.
 */

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	double wesn[4] = {-90.0, 120.0, -45.0, 50.0};
	struct GMT_MATRIX *M1 = NULL, *M2 = NULL;
	struct GMTAPI_CTRL *API = NULL;

	API = GMT_Create_Session ("test", 2U, mode, NULL);

	/* Read in earth_relief_01d as a matrix from the remote grid */
	M1 = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, "@earth_relief_01d_g", NULL);
	/* Write out the matrix to another grid file */
	GMT_Write_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_WRITE_NORMAL, NULL, "dump_01d.grd", M1);
	/* Read in earth_relief_01d as a matrix from the remote grid */
	M2 = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, wesn, "@earth_relief_01d_g", NULL);
	/* Write out the matrix to another grid file */
	GMT_Write_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_WRITE_NORMAL, NULL, "cut_01d.grd", M2);

	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
