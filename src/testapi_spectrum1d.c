#include "gmt.h"
#include <math.h>
#include <string.h>
/*
 * Replicate issue raised in https://forum.generic-mapping-tools.org/t/api-for-spectrum1d/1072.
 * Per OP it yields memory trespassing when valgrind is run and crashes on second run.
 * Problem was for GMT_IS_DATASET and GMT_IS_DUPLICATE we did not register the duplicate container
 * so it got lost in the shuffle.
 */

int main () {
	unsigned int i, m = 128;
	double sampling = 1.;
	void *API;
	struct GMT_DATASET *Din = NULL;
	struct GMT_DATASET *Dout = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	char input[GMT_VF_LEN] = {""};
	char output[GMT_VF_LEN] = {""};
	uint64_t par[] = {1, 1, 512, 1};
	char command[2*GMT_VF_LEN + 64];

	/* Initialize the GMT session */
	API = GMT_Create_Session ("GMT_spectrum1d", 2, 0, NULL);
	/* Create the data */
	Din = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_CONTAINER_AND_DATA, par, NULL, NULL, 0, -1, NULL);
	S = Din->table[0]->segment[0];
	for (i = 0; i < 512; i++) 
		S->data[0][i] = sin(2.*(double)i) + 2.*cos((double)i); // stupid data, just for testing
	/* Open a virtual file to hold the data */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, Din, input);
	/* Open a virtual file to hold the result */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, NULL, output);
	/* Create the parameters string */
	sprintf (command, "%s -S%d -D%lf -W -N ->%s", input, 2*m, sampling, output);
	/* Call the module */
	GMT_Call_Module (API, "spectrum1d", GMT_MODULE_CMD, command);
	/* Free input stuff */
	GMT_Close_VirtualFile (API, input);
	GMT_Destroy_Data (API, &Din);
	/* Get the spectrum in memory */
	Dout = GMT_Read_VirtualFile (API, output);
	/* Close output file */
	GMT_Close_VirtualFile (API, output);
	/* Print the result on stdout */
	S = Dout->table[0]->segment[0];
	for (i = 0; i < S->n_rows; i++) 
		printf("%f %f\n", S->data[0][i], S->data[1][i]);
	if (S->n_rows != m)
		printf("%d rows (%d expected)\n", (int)S->n_rows, m);
	/* Free output */
	GMT_Destroy_Data (API, &Dout);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
}
