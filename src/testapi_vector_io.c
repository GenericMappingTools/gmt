#include "gmt.h"
/*
 * Testing the reading and writing of GMT_VECTOR from/to ASCII files.
 * The test script api/apivec_io.sh will run this and make a plot.
 */
int main () {
	void *API = NULL;                 /* The API control structure */
	struct GMT_VECTOR *V = NULL;     /* Structure to hold input/output dataset as vectors */

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Read in our data table to memory */
	V = GMT_Read_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, "@hotspots.txt", NULL);
	/* Write the vector out to a table */
	GMT_Write_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_NORMAL, NULL, "test_hotspots.txt", V);
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
