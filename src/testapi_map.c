#include "gmt.h"
/* Used to examine https://github.com/GenericMappingTools/gmt/issues/4518
 * Give an argument to replicate that, otherwise we make a PS that can be
 * compared with the original PS so the test can run as normal.
 */
int main (char **argv, int argc) {
	void *API;

	/* Initialize the GMT session */
	if ((API = GMT_Create_Session ("GMT_plot", 2, GMT_SESSION_RUNMODE, NULL)) == NULL)
   	return EXIT_FAILURE;
	if (argc > 1)
		GMT_Call_Module (API, "begin", GMT_MODULE_CMD, "toto_api jpg");
	else
		GMT_Call_Module (API, "begin", GMT_MODULE_CMD, "toto_api ps");
	GMT_Call_Module (API, "basemap", GMT_MODULE_CMD, "-BWESN -Bxa30mg30m -Bya20mg20m -JM7.27/42.27/16.25c -R5.5/41.425/9.0/43.1r");
	if (argc > 1)
		GMT_Call_Module (API, "end", GMT_MODULE_CMD, "show");
	else
		GMT_Call_Module (API, "end", GMT_MODULE_CMD, NULL);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) 
		return EXIT_FAILURE;
}
