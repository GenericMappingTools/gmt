/* Fixing the problem reported by https://forum.generic-mapping-tools.org/t/api-for-makecpt/1044/6.
 * Tracked down to use of gmt_M_memory allocation (with NULL pointers) rather than gmt_create_pallete
 * Fixed Nov. 17, 2020, P. Wessel
 */

#include "gmt.h"
#include <string.h>
int main () {
	char output[GMT_VF_LEN] = {""};
	char *arg[4] = {"-Ccool", "-T1/65/1", "-N", NULL};
	void *P = NULL, *API = NULL;

	/* Initialize the GMT session */
	API = GMT_Create_Session ("GMT_makecpt", 2, 0, NULL);
	/* Open a virtual file to hold the CPT */
	GMT_Open_VirtualFile (API, GMT_IS_PALETTE, GMT_IS_NONE, GMT_OUT, NULL, output);
	/* Finalize the 4 arguments with the filename */
	arg[3] = (char*) calloc(strlen(output)+3, sizeof(char));
	sprintf(arg[3], "->%s", output);
	/* Create the CPT to be returned back to us via the virtual file */
	GMT_Call_Module (API, "makecpt", 4, arg);
	/* Get the CPT in memory */
	P = GMT_Read_VirtualFile(API, output);
	/* Write the CPT to the file output.cpt */
	GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_CPT_NO_BNF, NULL, "output.cpt", P);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) return EXIT_FAILURE;
}
