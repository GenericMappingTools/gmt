/*--------------------------------------------------------------------
 *  Testing API conversions, common settings, and parameters via
 *  GMT_Get_Value, GMT_Get_Default, GMT_Get_Common
 */

#include "gmt.h"		/* All programs using the GMT API needs this */
#include <strings.h>

#define GMT_OPTIONS	"-BIJKOPRUVXYafghinorst"	/* All the GMT common options */

void report (char *name, int count, double par[])
{
	int k;
	fprintf (stderr, "Got %s: ret = %d", name, count);
	for (k = 0; k < count; k++) fprintf (stderr, "\t%.12g", par[k]);
	fprintf (stderr, "\n");
}

int main (int argc, char *argv[])
{
	int ret, k;
	double value[100];
	char input[BUFSIZ], parameter[BUFSIZ], *commons = GMT_OPTIONS, string[2] = {0, 0};
	struct GMT_OPTION *options = NULL;		/* Linked list of program options */
	void *API = NULL;				/* The API pointer assigned below */

	/* 2. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], 2U, 0U)) == NULL) return EXIT_FAILURE;

	/* 3. Program initialization and parsing */

	options = GMT_Prep_Options (API, argc-1, (argv+1));	/* Set or get option list */

	/* Parse the commont GMT command-line options */
	if (GMT_Parse_Common (API, GMT_OPTIONS, options)) return EXIT_FAILURE;


	/* ---------------------------- This is the gmt_io main code ----------------------------*/

	GMT_Message (API, GMT_TIME_CLOCK, "Enter various lengths, distances, coordinates, either one-by-one or comma/slash separated.  End with -:\n");
	while (scanf ("%s", input) == 1 && input[0]) {
		if (!strcmp (input, "-")) break;
		ret = GMT_Get_Value (API, input, value);
		report (input, ret, value);
	}
	
	GMT_Message (API, GMT_TIME_CLOCK, "Enter name of GMT default parameter.  End with -:\n");
	while (scanf ("%s", input) == 1 && input[0]) {
		if (!strcmp (input, "-")) break;
		ret = GMT_Get_Default (API, input, parameter);
		if (ret == 0) fprintf (stderr, "%20s = %s\n", input, parameter);
	}
	
	GMT_Message (API, GMT_TIME_CLOCK, "These are the GMT common options and values you specified:\n");
	for (k = 0; k < strlen (commons); k++) {
		if ((ret = GMT_Get_Common (API, commons[k], value)) == -1) continue;
		string[0] = commons[k];
		report (string, ret, value);
	}
	
	/* Destroy GMT session and let GMT garbage collection free used memory */
	GMT_Destroy_Options (API, &options);
	if (GMT_Destroy_Session (API) != GMT_NOERROR) return EXIT_FAILURE;
}
