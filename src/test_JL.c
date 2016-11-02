#include "gmt.h"
/*
 * Testing a specific case where we wish to pass and receive a GMT_VECTOR or GMT_MATRIX
 * to/from a module that expect to read/write GMT_DATASETs.
 */
int main () {
    void *API = NULL;                 /* The API control structure */
    struct GMT_VECTOR *V[2] = {NULL, NULL};     /* Structure to hold input/output dataset as vectors */
    struct GMT_MATRIX *M[2] = {NULL, NULL};     /* Structure to hold input/output dataset as matrix */
    char input[GMT_STR16] = {""};     			/* String to hold virtual input filename */
    char output[GMT_STR16] = {""};    			/* String to hold virtual output filename */
    char args[128] = {""};            			/* String to hold module command arguments */

    /* Initialize the GMT session */
    API = GMT_Create_Session ("test", 2U, GMT_SESSION_EXTERNAL, NULL);
	/* FIRST TEST GMT_VECTOR */
    /* Read in our data table to memory */
    V[GMT_IN] = GMT_Read_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_PLP, GMT_READ_NORMAL, NULL, "belgium.txt", NULL);
    /* Associate our data table with a virtual file */
    GMT_Open_VirtualFile (API, GMT_IS_VECTOR, GMT_IS_PLP, GMT_IN, V[GMT_IN], input);
    /* Create a virtual file to hold the sampled points */
    GMT_Open_VirtualFile (API, GMT_IS_VECTOR, GMT_IS_PLP, GMT_OUT, NULL, output);
    /* Prepare the module arguments */
    sprintf (args, "-sa %s -Gtopo.nc ->%s", input, output);
    /* Call the grdtrack module */
    GMT_Call_Module (API, "grdtrack", GMT_MODULE_CMD, args);
    /* Obtain the data from the virtual file */
    V[GMT_OUT] = GMT_Read_VirtualFile (API, output);
    /* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
    /* Write the data to file */
    if (GMT_Write_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_PLP, 0, NULL, "vjunk.txt", V[GMT_OUT])) return EXIT_FAILURE;

	/* NEXT TEST GMT_MATRIX */
    M[GMT_IN] = GMT_Read_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_PLP, GMT_READ_NORMAL, NULL, "belgium.txt", NULL);
    /* Associate our data table with a virtual file */
    GMT_Open_VirtualFile (API, GMT_IS_MATRIX, GMT_IS_PLP, GMT_IN, M[GMT_IN], input);
    /* Create a virtual file to hold the sampled points */
    GMT_Open_VirtualFile (API, GMT_IS_MATRIX, GMT_IS_PLP, GMT_OUT, NULL, output);
    /* Prepare the module arguments */
    sprintf (args, "-sa %s -Gtopo.nc ->%s", input, output);
    /* Call the grdtrack module */
    GMT_Call_Module (API, "grdtrack", GMT_MODULE_CMD, args);
    /* Obtain the data from the virtual file */
    M[GMT_OUT] = GMT_Read_VirtualFile (API, output);
    /* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
    /* Write the data to file */
    if (GMT_Write_Data (API, GMT_IS_MATRIX, GMT_IS_FILE, GMT_IS_PLP, 0, NULL, "mjunk.txt", M[GMT_OUT])) return EXIT_FAILURE;
    /* Destroy the GMT session */
    if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
