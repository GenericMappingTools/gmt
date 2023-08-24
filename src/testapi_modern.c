/* Issue from GitHub: https://github.com/GenericMappingTools/gmt/issues/4518
 * Problem seems to be that gmt begin comes after GMT_Create_Session in the
 * code so there are orientation and page size issues. In gmt end, the
 * psconvert call is missing the -A.
 */

#include "gmt.h"

int main () {
	void *API = NULL;

	/* Initialize the GMT session */
	if ((API = GMT_Create_Session ("testapi_modern", 2, GMT_SESSION_RUNMODE, NULL)) == NULL)
		 return EXIT_FAILURE;
	GMT_Call_Module(API, "begin", GMT_MODULE_CMD, "testapi_modern jpg");
	GMT_Call_Module(API, "basemap", GMT_MODULE_CMD, "-BWESN -Bxa30mg30m -Bya20mg20m -JM7.27/42.27/16.25c -R5.5/41.425/9.0/43.1+r");
	GMT_Call_Module(API, "end", GMT_MODULE_CMD, "show");
	/* Destroy session */
	if (GMT_Destroy_Session (API))
		return EXIT_FAILURE;
}
