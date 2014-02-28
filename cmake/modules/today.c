/* $Id$
 *
 * called by cmake to obtain date at build time
 */

#include <stdio.h>
#include <time.h>

#define BUFSIZE 32

int main () {
	char today_string[BUFSIZE];

	/* obtain current time as time since epoch */
	time_t clock = time (NULL);

	/* convert time since epoch to calendar time expressed as local time */
	struct tm *p_time = localtime (&clock);

	/* convert tm object to custom textual representation YYYY;mm;dd;Mmm*/
	size_t result = strftime(today_string, BUFSIZE, "%Y;%m;%d;%B", p_time);

	if ( result ) {
		/* success, print date string */
		printf ("%s", today_string);
		return 0;
	}

	/* on error, print dummy */
	printf ("1313;13;13;Undecember");
	return -1;
}
