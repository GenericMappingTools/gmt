#include "gmt.h"
#include <math.h>
#include <string.h>
/*
 * Replicate issue raised in https://github.com/GenericMappingTools/gmt/issues/5890.
 * Expects files I1.txt and I2.txt to have been made. If argument is given then we
 * pass the two virtual datasets as duplicates, else as references.
 */

int main (int argc, char *argv[]) {
	void *API;
	unsigned int mode = (argc == 2) ? GMT_IS_REFERENCE : GMT_IS_DUPLICATE;
	struct GMT_DATASET *D1 = NULL, *D2 = NULL;
	struct GMT_DATASET *Dout = NULL;
	char input1[GMT_VF_LEN] = {""}, input2[GMT_VF_LEN] = {""};
	char output[GMT_VF_LEN] = {""};
	char command[3*GMT_VF_LEN + 64];
	char *outfile = argv[1];

	/* Initialize the GMT session */
	API = GMT_Create_Session ("GMT_spatial", 2, 0, NULL);
	/* Read the data */
	if ((D1 = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, "I1.txt", NULL)) == NULL) return EXIT_FAILURE;
	if ((D2 = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, "I2.txt", NULL)) == NULL) return EXIT_FAILURE;
	/* Open a virtual files to hold the data */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|mode, D1, input1);
	GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|mode, D2, input2);
	/* Open a virtual file to hold the result */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, NULL, output);
	/* Create the parameters string */
	sprintf (command, "%s %s -Ie -Fl ->%s", input1, input2, output);
	/* Call the module */
	GMT_Call_Module (API, "gmtspatial", GMT_MODULE_CMD, command);
	/* Free input resources */
	GMT_Close_VirtualFile (API, input1);
	GMT_Close_VirtualFile (API, input2);
	GMT_Destroy_Data (API, &D1);
	GMT_Destroy_Data (API, &D2);
	/* Get the result in memory */
	Dout = GMT_Read_VirtualFile (API, output);
	/* Close output virtual file */
	GMT_Close_VirtualFile (API, output);
	/* Print the result on stdout */
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, outfile, Dout) != GMT_NOERROR) return EXIT_FAILURE;
	/* Free output */
	GMT_Destroy_Data (API, &Dout);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
}
