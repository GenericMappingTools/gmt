#include "gmt.h"
int main (int argc, char *argv[]) {
    void *API;                        /* The API control structure */
    struct GMT_VECTOR *V = NULL;     /* Structure to hold input dataset */
    char input[GMT_STR16] = {""};     /* String to hold virtual input filename */
    char output[GMT_STR16] = {""};    /* String to hold virtual output filename */
    char args[128] = {""};            /* String to hold module command arguments */

    /* Initialize the GMT session */
    API = GMT_Create_Session ("test", 2U, 0, NULL);
    /* Read in our data table to memory */
    V = GMT_Read_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_PLP, GMT_READ_NORMAL, NULL, "table_5.11", NULL);
    /* Associate our data table with a virtual file */
    GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_PLP, V, input);
    /* Create a virtual file to hold the sampled points */
    GMT_Create_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_VECTOR, GMT_IS_PLP, output);
    /* Prepare the module arguments */
    sprintf (args, "-sa %s -Gtopo.nc ->%s", input, output);
    /* Call the greenspline module */
    GMT_Call_Module (API, "grdtrack", GMT_MODULE_CMD, args);
    /* Obtain the data from the virtual file */
    V = GMT_Read_VirtualFile (API, output);
    /* Write the data to file */
    if (GMT_Write_Data (API, GMT_IS_VECTOR, GMT_IS_FILE, GMT_IS_PLP, 0, NULL, "junk.txt", V)) return EXIT_FAILURE;
    /* Destroy the GMT session */
    if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
};
