/*
 * Testing the passing of a GMT_MATRIX via GRID to grd plotters.
 */
#include "gmt_dev.h"

int main () {
	struct GMTAPI_CTRL *API = NULL;                /* The API control structure */
	struct GMT_GRID *G = NULL, *G1, *G2;    /* Structure to hold input grids */
	openmp_int row, col;
	unsigned int pad2[4] = {2, 2, 2, 2}, def_pad[4];
	uint64_t ij;

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 0U, GMT_SESSION_EXTERNAL, NULL);
	G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_READ_NORMAL, NULL, "test.grd", NULL);
	/* Duplicate with pad */
	gmt_M_memcpy (def_pad, API->GMT->current.io.pad, 4, unsigned int);	/* Save default pad */
	gmt_M_memcpy (API->GMT->current.io.pad, pad2, 4, unsigned int);		/* Change default pad */
	G1 = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA | GMT_DUPLICATE_RESET, G);
	gmt_BC_init (API->GMT, G1->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (API->GMT, G1, GMT_IN);	/* Set boundary conditions */	/* Duplicate then add pad */
	gmt_M_memcpy (API->GMT->current.io.pad, def_pad, 4, unsigned int);		/* Restore default pad */
	G2 = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, G);
	gmt_grd_pad_on (API->GMT, G2, pad2);	/* Add pad */
	gmt_BC_init (API->GMT, G2->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (API->GMT, G2, GMT_IN);	/* Set boundary conditions */	/* Destroy session */
	gmt_M_grd_loop (API->GMT, G1, row, col, ij) {
		if (!floatAlmostEqualZero (G1->data[ij], G2->data[ij]))
			fprintf (stderr, "row %d col %d values %f %f\n", row, col, G1->data[ij], G2->data[ij]);
	}
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
