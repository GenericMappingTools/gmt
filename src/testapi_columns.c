#include "gmt.h"
/*
 * Testing the implementation of -i, -o, etc in the API.
 * The test script api/apicolumns_io.sh will run this and compare.
 *
 * We read in the file @wus_gps_final_crowell.txt which has 3277 records of 7 columns.
 * We will test various things here:
 * We will pass -qi0:19 and thus only read 20 rows
 * We will use -i to select some of the columns, including a repeat column, and
 *   apply the scale/offset modifiers
 * We will use -qo5:10 to just write 6 of those records.
 *
 * The equivalent command line that gives the comparing data is here:
 * gmt convert @wus_gps_final_crowell.txt -qi0:19 -i1,0,3,3,2,5+s10,6+o10 -qo5:10 --FORMAT_FLOAT_OUT=%8.3f
 * which gave clean formatted output like this:
 * 33.332	-117.159	  26.923	  26.923	 -24.876	   6.410	   9.962
 * 34.420	-119.606	  26.503	  26.503	 -27.888	   4.000	  10.045
 * 34.531	-120.346	  32.742	  32.742	 -29.632	   4.870	  10.065
 * 35.003	-119.838	  27.247	  27.247	 -25.570	   3.540	  10.007
 * 35.084	-120.584	  33.325	  33.325	 -28.717	   4.000	  10.043
 * 35.855	-120.799	  33.060	  33.060	 -25.520	   4.470	  10.041
 */
int main () {
	void *API = NULL;                 /* The API control structure */
	void *data[2] = {NULL, NULL};
	const char *convert = "-qi0:19 -i1,0,3,3,2,5+s10,6+o10 -qo5:10 --FORMAT_FLOAT_OUT=%8.3f";
	const char *file[3] = {"dataset.txt", "matrix.txt", "vector.txt"};
	char cmd[GMT_LEN128], input[GMT_VF_LEN] = {""}, out[GMT_VF_LEN] = {""};
	unsigned int family[3] = {GMT_IS_DATASET, GMT_IS_MATRIX, GMT_IS_VECTOR};
	unsigned int k, via[3] = {0, GMT_VIA_MATRIX, GMT_VIA_VECTOR};

	/* Initialize the GMT session */
	API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	for (k = 0; k < 3; k++) {
		/* Read in our data table to memory */
		data[GMT_IN] = GMT_Read_Data (API, family[k], GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, "@wus_gps_final_crowell.txt", NULL);
		/* Create a virtual file for this input data set */
		GMT_Open_VirtualFile (API, family[k]|via[k], GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, data[GMT_IN], input);
		/* Create a virtual file to hold the output */
		GMT_Open_VirtualFile (API, family[k]|via[k], GMT_IS_POINT, GMT_OUT, data[GMT_OUT], output);
		/* Create command for module gmtconvert*/
		sprintf (cmd, "%s %s ->%s", convert, input, output);
		/* Call the gmtconvert module */
		GMT_Call_Module (API, "gmtconvert", GMT_MODULE_CMD, args);
		/* Obtain the output container from the output virtual file */
		data[GMT_OUT] = GMT_Read_VirtualFile (API, output);
		/* Write the result out to a table */
		GMT_Write_Data (API, family[k]|via[k], GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_NORMAL, NULL, file[k], data[GMT_OUT]);
		/* Destroy the two memory resources */
		GMT_Destroy_Data (API, &data[GMT_IN]);
		GMT_Destroy_Data (API, &data[GMT_OUT]);
	}
	/* Destroy the GMT session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
