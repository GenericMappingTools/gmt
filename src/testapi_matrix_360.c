#include "gmt.h"

/* Testing the passing of a matrix to grdimage for global projection
 * when the central meridian changes. This currently fails as in
 * https://github.com/GenericMappingTools/pygmt/issues/515#issue-655281714
 */

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_MATRIX *M = NULL;
	char input[GMT_VF_LEN], args[256] = {""};
	struct GMTAPI_CTRL *API = NULL;

	API = GMT_Create_Session ("test", 2U, mode, NULL);

	/* Read in earth_relief_01d as a matrix from text file and set correct region, inc, registration */
	M = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, "@earth_relief_01d", NULL);
	M->range[0] = -180;	M->range[1] = 180; M->range[2] = -90;	M->range[3] = 90.0;
	M->inc[0] = M->inc[1] = 1.0;
	M->registration = 1;
	/* Create a virtual file to pass as a grid */
	GMT_Open_VirtualFile (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_IN, M, input);
	/* Call grdimage with central longitude 0, which is the center of the grid */
	sprintf (args, "%s -Rg -JH0/6i -Bg30 -K -Cgeo -P", input);
	GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, args);
	GMT_Init_VirtualFile (API, 0, input);
	/* Call grdimage with central longitude 98.75W, which is arbitrary */
	sprintf (args, "%s -R -JH98.7/6i -Bg30 -K -Cgeo -O -Y3.25i", input);
	GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, args);
	GMT_Init_VirtualFile (API, 0, input);
	/* Call grdimage with central longitude 180 which means grid needs to be rotated 180 */
	sprintf (args, "%s -R -JH180/6i -Bg30 -O -Cgeo -Y3.25i", input);
	GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, args);
	GMT_Close_VirtualFile (API, input);

	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
