#include <stdio.h>
#include <string.h>
/* $Id: rearranger.c,v 1.2 2009-06-17 01:21:50 guru Exp $
 * Puts new GMT/GIS extra comments onto the segment header line for easy parsing
 */
main () {
	char line[BUFSIZ], feature[BUFSIZ], *s;
	int hold = 0;
	
	while (fgets (line, BUFSIZ, stdin)) {
		line[strlen(line)-1] = '\0';	/* Cut off \n */
		if (line[0] == '>') {	/* New segment */
			hold = 1;
			printf ("%s", line);
		}
		else if (line[0] == '#') {	/* Add comments to segment line */
			if ((s = strstr (line, "@D"))) {
				strcpy (feature, s);
			}
			if (hold) printf (" %s", &line[1]);
			if ((s = strstr (line, "@H")) && hold) printf (" %s", feature);
		}
		else {	/* Data line */
			if (hold) printf ("\n");	/* Finish off segment line */
			hold = 0;
			printf ("%s\n", line);
		}
	}
}