#include "gmt.h"

/* Test calling grdfill on a grid with an asymmetric pad
 */

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_MATRIX *M_g = NULL;
	char input_g[GMT_VF_LEN] = {""};
	char args_g[128]= {""};
	struct GMTAPI_CTRL *API = NULL;

	API = GMT_Create_Session ("test", 2U, mode, NULL);
	
	M_g = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, "@static_earth_relief.nc", NULL);

	GMT_Open_VirtualFile (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, M_g, input_g);

	sprintf (args_g, "%s -R-54/-50/-23/-16 -Goutput.nc -Ac0 -N349", input_g);
	GMT_Call_Module (API, "grdfill", GMT_MODULE_CMD, args_g);
	GMT_Close_VirtualFile (API, input_g);

	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
