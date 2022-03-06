#include "gmt.h"
/*
 * Testing the use GMT_Put_Vector and its ability to convert ascii stuff.
 */

/* Dimensions of the test dataset */
#define NCOLS 4
#define NROWS 2

int main () {
	void *API = NULL;							/* The API control structure */
	struct GMT_VECTOR *V = NULL;				/* Structure to hold input dataset as vectors */

	uint64_t dim[4] = {NCOLS, NROWS, 1, 0};		/* ncols, nrows, nlayers, type */
	/* two records with cartesian, time, lon, lat data */
	char *S0[NROWS] = {"134.9", "202"};
	char *S1[NROWS] = {"2020-06-01T14:55:33", "2020-06005T16:00:50.75"};
	char *S2[NROWS] = {"12:45:36W", "19:48E"};
	char *S3[NROWS] = {"16:15:00S", "29:30N"};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("testapi_putvector", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* Create a dataset */
	V = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL);
	/* Hook the five converted text vectors up to this container */
	GMT_Put_Vector(API, V, 0, GMT_TEXT|GMT_DOUBLE, S0);
	GMT_Put_Vector(API, V, 1, GMT_TEXT|GMT_LONG, S1);
	GMT_Put_Vector(API, V, 2, GMT_TEXT|GMT_DOUBLE, S2);
	GMT_Put_Vector(API, V, 3, GMT_TEXT|GMT_DOUBLE, S3);
	/* Write table to stdout */
	GMT_Write_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_SET, NULL, NULL, V);
	/* Destroy the GMT session */
	GMT_Destroy_Session (API);
};
