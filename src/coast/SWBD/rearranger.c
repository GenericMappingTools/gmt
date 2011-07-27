#include <stdio.h>
#include <string.h>
/* $Id$
 * Puts new GMT/GIS extra comments onto the segment header line for easy parsing
* If an argument is given it is added to the multisegment headers
 */
int main (int argc, char **argv) {
	char line[GMT_BUFSIZ], feature[GMT_BUFSIZ], *s, *ID = NULL;
	int hold = 0, seg_no = 0;
	if (argc == 2) ID = argv[1];	/* Passed a filename ID to write to all segment headers */
	while (fgets (line, GMT_BUFSIZ, stdin)) {
		line[strlen(line)-1] = '\0';	/* Cut off \n */
		if (line[0] == '>') {	/* New segment */
			hold = 1;
			printf ("%s", line);
			if (ID) printf (" -L%s-%d", ID, seg_no++);
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
