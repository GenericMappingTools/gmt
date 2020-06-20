#include "gmt_dev.h"

int main () {
	unsigned int mode = GMT_SESSION_EXTERNAL;
	struct GMT_MATRIX *M_p = NULL, *M_g = NULL;
	char input_p[GMT_VF_LEN] = {""}, input_g[GMT_VF_LEN] = {""};
	char args_p[128] = {""}, args_g[128]= {""};
	struct GMTAPI_CTRL *API = NULL;

	double inc[2] = {1.0, 1.0};
	double coord[9] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};

	API = GMT_Create_Session ("test", 2U, mode, NULL);

	/* pass matrix as gridline-registered grid */
	double range_g[4] = {0.0, 2.0, 0.0, 2.0};
	if ((M_g = GMT_Create_Data (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, range_g, inc, GMT_GRID_NODE_REG, 0, NULL)) == NULL) return (EXIT_FAILURE);
	GMT_Put_Matrix (API, M_g, GMT_DOUBLE, 0, coord);
	GMT_Open_VirtualFile (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, M_g, input_g);
	sprintf (args_g, "%s -R-1/3/-1/3 -JX6c -Baf -BWSen+tGridline -K > test_matrix.ps", input_g);
	GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, args_g);
	GMT_Close_VirtualFile (API, input_g);

	/* pass matrix as pixel-registered grid */
	double range_p[4] = {-0.5, 2.5, -0.5, 2.5};
	if ((M_p = GMT_Create_Data (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, range_p, inc, GMT_GRID_PIXEL_REG, 0, NULL)) == NULL) return (EXIT_FAILURE);
	GMT_Put_Matrix (API, M_p, GMT_DOUBLE, 0, coord);
	GMT_Open_VirtualFile (API, GMT_IS_GRID|GMT_VIA_MATRIX, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, M_p, input_p);
	sprintf (args_p, "%s -R-1/3/-1/3 -JX6c -Baf -BWSen+tPixel -O -X8c >> test_matrix.ps", input_p);
	GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, args_p);
	GMT_Close_VirtualFile (API, input_p);

	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
	exit (0);
}
