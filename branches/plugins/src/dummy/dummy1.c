/*--------------------------------------------------------------------
 * $Id$
 */

#include "gmt_dev.h"

#include "plugin.h"

int GMT_dummy1 (void *V_API, int mode, void *args) {
	int status = 0; /* Status code from GMT API */
	struct GMT_OPTION *head = NULL; /* Linked list of options */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);

	if (API == NULL)
		return (GMT_NOT_A_SESSION);

	/* Create linked options */
	if ((head = GMT_Create_Options (API, 0, "GMT_COMPATIBILITY")) == NULL)
		return (API->error);

	/* Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, GMT_ID_GMTGET, -1, head);
	if (status) {
		GMT_Report (API, GMT_MSG_NORMAL, "error %d\n", status);
		return (API->error);
	}

	/* Destroy local linked option list */
	if (GMT_Destroy_Options (API, &head))
		return (API->error);

	return (GMT_OK);
}
