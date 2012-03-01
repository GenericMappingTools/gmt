/*LINTLIBRARY*/

/* This file is here because microsoft doesn't seem to provide
   a getopt function. Shame on you, Bill Gates! */

#include <string.h>

#define EOF	(-1)

int             opterr = 1;
int             optind = 1;
int             optopt;
char           *optarg;

int
getopt(int argc,  char *const argv[], const char *opts)
{
    static int      sp = 1;
    register int    c;
    register char  *cp;

    if (sp == 1)
	if (optind >= argc ||
	    argv[optind][0] != '-' || argv[optind][1] == '\0')
	    return (EOF);
	else if (strcmp(argv[optind], "--") == 0) {
	    optind++;
	    return (EOF);
	}
    optopt = c = argv[optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
	if (argv[optind][++sp] == '\0') {
	    optind++;
	    sp = 1;
	}
	return ('?');
    }
    if (*++cp == ':') {
	if (argv[optind][sp + 1] != '\0')
	    optarg = &argv[optind++][sp + 1];
	else if (++optind >= argc) {
	    sp = 1;
	    return ('?');
	}
	else
	    optarg = argv[optind++];
	sp = 1;
    }
    else {
	if (argv[optind][++sp] == '\0') {
	    sp = 1;
	    optind++;
	}
	optarg = NULL;
    }
    return (c);
}
