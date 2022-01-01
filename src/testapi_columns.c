#include "gmt.h"
/*
 * Testing the implementation of -i in the API.
 * The test script api/apicolumns_icol.sh will run this and compare.
 *
 * We read in the file @wus_gps_final_crowell.txt which has 3277 records of 7 columns.
 * * We will use -i to select some of the columns, including a repeat column, and
 *   apply the scale/offset modifiers
 *
 * The equivalent command line that gives the comparing data is here:
 * gmt convert @wus_gps_final_crowell.txt -i1,0,3,3+d2,2,5+s10,6+o10 --FORMAT_FLOAT_OUT=%8.3f
 * which gave clean formatted output like this:
 * 34.754	-118.146	  15.280	   7.640	 -13.544	   6.900	  10.170
 * 34.503	-118.255	  21.994	  10.997	 -23.513	   7.460	  10.032
 * 34.566	-119.264	  23.275	  11.637	 -28.284	   7.300	  10.082
 * 33.375	-117.565	  28.953	  14.476	 -26.153	   5.470	   9.957
 *
 * Notes: 1) Another issue to fix for datasets, so k starts at 1
 */
int main () {
	void *API = NULL;                 /* The API control structure */
	void *data[2] = {NULL, NULL};
	const char *convert = "-i1,0,3,3+d2,2,5+s10,6+o10 --FORMAT_FLOAT_OUT=%8.3f";
	const char *file[3] = {"dataset.txt", "matrix.txt", "vector.txt"};
	char cmd[128], filename[128] = {""}, input[GMT_VF_LEN] = {""}, output[GMT_VF_LEN] = {""};
	unsigned int family[3] = {GMT_IS_DATASET, GMT_IS_MATRIX, GMT_IS_VECTOR};
	unsigned int reference[3] = {GMT_IS_DUPLICATE, GMT_IS_REFERENCE, GMT_IS_REFERENCE};
	unsigned int k, via[3] = {0, GMT_VIA_MATRIX, GMT_VIA_VECTOR};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	for (k = 1; k < 3; k++) {
		/* Read in our data table to memory */
		data[GMT_IN] = GMT_Read_Data (API, family[k], GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, "@wus_gps_final_crowell.txt", NULL);
		/* Create a virtual file for this input data set */
		GMT_Open_VirtualFile (API, GMT_IS_DATASET|via[k], GMT_IS_POINT, GMT_IN|reference[k], data[GMT_IN], input);
		/* Create a virtual file to hold the output */
		GMT_Open_VirtualFile (API, GMT_IS_DATASET|via[k], GMT_IS_POINT, GMT_OUT, data[GMT_OUT], output);
		/* Create command for module gmtconvert */
		sprintf (cmd, "%s %s ->%s", convert, input, output);
		/* Call the gmtconvert module */
		GMT_Call_Module (API, "gmtconvert", GMT_MODULE_CMD, cmd);
		/* Obtain the output container from the output virtual file */
		data[GMT_OUT] = GMT_Read_VirtualFile (API, output);
		/* Write the result out to a table */
		sprintf (filename, "%s", file[k]);
		GMT_Write_Data (API, family[k], GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_NORMAL|GMT_IO_RESET, NULL, filename, data[GMT_OUT]);
		/* Destroy the two memory resources */
		GMT_Destroy_Data (API, &data[GMT_IN]);
		GMT_Destroy_Data (API, &data[GMT_OUT]);
	}
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
