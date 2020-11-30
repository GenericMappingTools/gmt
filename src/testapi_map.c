#include "gmt.h"
/* Used to examine https://github.com/GenericMappingTools/gmt/issues/4518
 * Set do_jpg = 1 to replicate that, otherwise we make a PS that can be
 * compared with the original PS so the test can run as normal.
 */
int main () {
	void *API;
	int do_jpg = 0;

	/* Initialize the GMT session */
	if ((API = GMT_Create_Session ("GMT_plot", 2, 0, NULL)) == NULL)
   	return EXIT_FAILURE;
	if (do_jpg)
		GMT_Call_Module (API, "begin", GMT_MODULE_CMD, "toto_api jpg");
	else
		GMT_Call_Module (API, "begin", GMT_MODULE_CMD, "toto_api ps");
	GMT_Call_Module (API, "basemap", GMT_MODULE_CMD, "-BWESN -Bxa30mg30m -Bya20mg20m -JM7.27/42.27/16.25c -R5.5/41.425/9.0/43.1r");
	if (do_jpg)
		GMT_Call_Module (API, "end", GMT_MODULE_CMD, "show");
	else
		GMT_Call_Module (API, "end", GMT_MODULE_CMD, NULL);
	/* Destroy session */
	if (GMT_Destroy_Session (API)) 
		return EXIT_FAILURE;
}
