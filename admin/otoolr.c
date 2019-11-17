/* Recursively find all shared libraries referenced by the executables and
 * shared libraries given on the command line.
 * The output list must be run through sort -u to remove duplicates.
 * An example command line might be
 *
 * otoolr <builddir> /opt/local/bin/ffmpeg /opt/local/bin/gm ... | sort -u > t.lis
 *
 * The <builddir> is prepended if any of the executables or dylib files have relative paths,
 * otherwise not used.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

struct LINK {
	/* Struct with shared library name and pointer to next */
	char name[256];
	struct LINK *next;
};

int used (struct LINK *list, char *name) {
	/* Determine if this library has already been processed */
	struct LINK *this = list;
	while (this && strcmp (name, this->name))
		this = this->next;
	return (this ? 1 : 0);
}

struct LINK * last (struct LINK *list) {
	/* Find the last link in the chain so we can append to it */
	struct LINK *this = list;
	while (this->next)
		this = this->next;
	return (this);
}

void printlist (struct LINK *list) {
	/* March through and print the entries except anything in /Users, free up along the way */
	struct LINK *this = list, *prt;
	while (this->next) {
		prt = this->next;
		if (!strstr (prt->name, "/Users")) printf ("%s\n", prt->name);
		free (this);
		this = prt;
	}
}

/* Recursive function. Set verb = 1 for debugging */

void get_list (char *top, char *name, int level, struct LINK *list) {
	/* Recursive scanner of entries returned by otool -L */
	char process[256] = {""}, next[256] = {""}, line[256] = {""}, dir[256] = {""}, *c = NULL;
	int k, library = (strstr (name, ".dylib") != NULL), verb = 0;
	FILE *fp = NULL;
	struct LINK *this = NULL;
	
	strcpy (dir, top);
	c = strrchr (dir, '/');	c[0] = '\0';
	if (name[0] == '/')	/* Full path */
		sprintf (process, "otool -L %s", name);
	else if (library)	/* In the build dirs path */
		sprintf (process, "otool -L %s/%s", dir, name);
	else	/* Executable */	/* In the bin dir */
		sprintf (process, "otool -L %s/bin/%s", top, name);
	if (verb) fprintf (stderr, "[%d-%d] Running: %s\n", level, library, process);
	if ((fp = popen (process, "r")) == NULL) return;
	fgets (line, 256U, fp);		/* Skip header record */
	while (fgets (line, 256U, fp)) {
		k = 0; while (isspace (line[k])) k++;	/* Scan to start of library name */
		sscanf (&line[k], "%s", next);	/* Get the shared library file name */
		if (strstr (line, "/usr/lib") || strstr (line, "/System/Library") || (library && strstr (line, name))) {
			/* Skip system files or itself */
			if (verb) fprintf (stderr, "[%d-%d] Skip: %s\n", level, library, next);
			continue;
		}
		
		if (used (list, next)) {	/* Already processed this library */
			if (verb) fprintf (stderr, "[%d-%d] Used: %s\n", level, library, next);
			continue;
		}
		if (next[0] == '/') {
			if (verb) fprintf (stderr, "[%d] Test: %s\n", level, next);
			this = last (list);
			this->next = calloc (1, sizeof (struct LINK));
			strcpy (this->next->name, next);
			get_list (top, next, level + 1, list);
		}
	}
	pclose (fp);
}

int main (int argc, char **argv) {
	int k;
	struct LINK *list = calloc (1, sizeof (struct LINK));
	strcpy (list->name, "HEAD");	/* Unused head of the linked list */
	for (k = 2; k < argc; k++)	/* Try all the arguments (argv[1] is the builddir) */
		get_list (argv[1], argv[k], 0, list);
	printlist (list);
}
