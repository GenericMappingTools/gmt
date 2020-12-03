#include "gmt.h"
/* Used to examine https://github.com/GenericMappingTools/gmt/issues/4518
 * Give an graphic extension to replicate that, otherwise we make a PS that can be
 * compared with the original PS so the test can run as normal.
 * Run testapi_map jpg to show the result is properly cropped.
 * [add "| 458752" to GMT_SESSION_RUNMODE to simulate -Vd ]
 */
int main (int argc, char *argv[]) {
	void *API;

	/* Initialize the GMT session */
	if ((API = GMT_Create_Session ("GMT_plot", 2, GMT_SESSION_RUNMODE, NULL)) == NULL)
		return EXIT_FAILURE;
	if (argc > 1) {	/* Gave a particular graphics format (we hope - no checking here) */
		char string[64] = {""};
		sprintf (string, "apimap %s", argv[1]);
		GMT_Call_Module (API, "begin", GMT_MODULE_CMD, string);
	}
	else	/* Default to PostScript */
		GMT_Call_Module (API, "begin", GMT_MODULE_CMD, "apimap ps");
	GMT_Call_Module (API, "basemap", GMT_MODULE_CMD, "-BWESN -Bxa30mg30m -Bya20mg20m -JM7.27/42.27/15c -R5.5/41.425/9.0/43.1r");
	if (argc > 1)
		GMT_Call_Module (API, "end", GMT_MODULE_CMD, "show");
	else
		GMT_Call_Module (API, "end", GMT_MODULE_CMD, NULL);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) 
		return EXIT_FAILURE;
}
