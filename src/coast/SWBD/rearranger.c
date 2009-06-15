#include <stdio.h>
#include <string.h>
/* $Id: rearranger.c,v 1.1 2009-06-15 20:15:49 guru Exp $
 * Puts new GMT/GIS extra comments onto the segment header line for easy parsing
 */
main () {
	char line[BUFSIZ];
	int hold = 0;
	
	while (fgets (line, BUFSIZ, stdin)) {
		line[strlen(line)-1] = '\0';	/* Cut off \n */
		if (line[0] == '>') {	/* New segment */
			hold = 1;
			printf ("%s", line);
		}
		else if (line[0] == '#') {	/* Add comments to segment line */
			if (hold) printf (" %s", &line[1]);
		}
		else {	/* Data line */
			if (hold) printf ("\n");	/* Finish off segment line */
			hold = 0;
			printf ("%s\n", line);
		}
	}
}