#include "gmt.h"
int main () {
    void *API;                        /* The API control structure */
    struct GMT_DATASET *D = NULL;     /* Structure to hold input dataset */
    struct GMT_GRID *G = NULL;        /* Structure to hold output grid */
    char input[GMT_STR16] = {""};     /* String to hold virtual input filename */
    char output[GMT_STR16] = {""};    /* String to hold virtual output filename */
    char args[128] = {""};            /* String to hold module command arguments */

    /* Initialize the GMT session */
    API = GMT_Create_Session ("test", 2U, 0, NULL);
    /* Read in our data table to memory */
    D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_PLP, GMT_READ_NORMAL, NULL, "table_5.11", NULL);
    /* Associate our data table with a virtual file */
    GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IN, D, input);
    /* Create a virtual file to hold the resulting grid */
    GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, output);
    /* Prepare the module arguments */
    sprintf (args, "-R0/7/0/7 -I0.2 -D1 -St0.3 %s -G%s", input, output);
    /* Call the greenspline module */
    GMT_Call_Module (API, "greenspline", GMT_MODULE_CMD, args);
    /* Obtain the grid from the virtual file */
    G = GMT_Read_VirtualFile (API, output);
    /* Close the virtual files */
	GMT_Close_VirtualFile (API, input);
	GMT_Close_VirtualFile (API, output);
    /* Write the grid to file */
    if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, "junk.nc", G)) return EXIT_FAILURE;
    /* Destroy the GMT session */
    if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
