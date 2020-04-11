/*
 * src_convention_check.c is used to analyze if we are following our own naming
 * convention for functions in the API (GMT_*), the core library (gmt_*),
 * the inter-file functions (gmtlib_*) or the in-file static function
 * (<gmtfile>_*).  We also try to determine in how many files a function is called
 * in an effort to look for candidates for conversion to static functions.
 *
 * Options:
 *			-e Only list the external functions and skip static ones
 *			-f Only list functions and not where they are called
 *			-o Write scan to stdout [/tmp/gmt/scan.txt]
 *			-v Extra progress verbosity
 *			-w Only report functions with a warning for possible wrong name
 *
 * Weaknesses: There are 3rd party files we probably should skip, and it is
 * not yet clear what convention to use in supplements.
 *
 * Paul Wessel, April 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#define NFILES 500
#define NFUNCS 10000

struct FUNCTION {
	char name[64];			/* Name of function */
	char file[64];			/* Name if file it is declared in */
	int api;				/* 1 if declared extern in gmt.h */
	int declared_dev;		/* 1 if declared extern in gmt_prototypes.h */
	int declared_lib;		/* 1 if declared extern in gmt_internals.h */
	int declared_local;		/* 1 if function declared static */
	int rec;
	unsigned int n_files;	/* How many files referenced */
	unsigned int n_calls;	/* How many times referenced */
	char *in;				/* Array to mark which files this functions appears in */
};

int find_function (struct FUNCTION *F, int N, char *name) {
	/* Return 1 if function already found, else 0 */
	int k;
	for (k = 0; k < N; k++)
		if (!strcmp (F[k].name, name)) return k;
	return -1;
}

static int compare_n (const void *v1, const void *v2) {
	/* Compare function for qsort to arrange functions based on how many files they
	 * appear in and (if tied) how many times called */
	const struct FUNCTION *F1 = v1, *F2 = v2;
	if (F1->n_files < F2->n_files) return +1;
	if (F1->n_calls > F2->n_files) return -1;
	if (F1->n_files < F2->n_calls) return +1;
	if (F1->n_calls > F2->n_calls) return -1;
	return (0);
}

/* Include NULL-terminated arrays with function names.
 * These include files are created on the fly by src_convention_check.sh and then
 * this code is compiled */

static char *modules[] = {
#include "/tmp/gmt/modules.h"
	NULL
};

static char *API[] = {
#include "/tmp/gmt/api.h"
	NULL
};

static char *libdev[] = {
#include "/tmp/gmt/prototypes.h"
	NULL
};

static char *libint[] = {
#include "/tmp/gmt/internals.h"
	NULL
};

static int is_recognized (char *name, char *list[]) {
	/* Return 1 if name appears in the list, else 0 */
	int k = 0;
	while (list[k]) {
		if (!strcmp (name, list[k]))
			return 1;
		else k++;
	}
	return 0;
}

static void get_contraction (char *name, char *prefix) {
	/* Remove underscores to build prefix, e.g., gmt_support -> gmtsupport */
	unsigned k, j;
	for (k = j = 0; name[k] != '.'; k++)
		if (name[k] != '_') prefix[j++] = name[k];
	prefix[j] = '\0';
	if (!strcmp (prefix, "postscriptlight")) strcpy (prefix, "psl");	/* Accepted legacy shorthand */
}

static void wipe_line (char *line) {
	/* Blank out things in quotes to avoid having " so this func () is bad" be seen as a call */
	int c, quote;
	char *p = NULL;
	for (c = quote = 0; c < strlen (line); c++) {
		if (line[c] == '\"') { quote = !quote; continue; }
		if (quote) line[c] = ' ';
	}
	if ((p = strstr (line, "/*"))) p[0] = '\0';	/* Chop of trailing comment */
}

int main (int argc, char **argv) {
	int k, f, w, s, n, c, t, is_static, err, n_funcs = 0, comment = 0, brief = 0, ext = 0, log = 1, verbose = 0, warn_only = 0;
	int set_dev, set_lib, rec;
	size_t L;
	char line[512] = {""}, prefix[64] = {""};
	char word[6][64], type[4] = {'S', 'D', 'L', 'A'}, *p, *q, message[128] = {""};
	char *err_msg[4] = {"", "Change to gmt_* ", "Change to gmtlib_* ", "Change to <file>_* "};
	struct FUNCTION F[NFUNCS];
	FILE *fp, *out = stdout;

	if (argc == 1) {
		fprintf (stderr, "usage: src_convention_check [-e] [-f] [-o] *.c > log\n");
		fprintf (stderr, "	-e Only list external functions [all]\n");
		fprintf (stderr, "	-f Only list function stats and not where called [full log]\n");
		fprintf (stderr, "	-o Write main results to stdout [/tmp/gmt/gmt/scan.txt\n");
		fprintf (stderr, "	-v Extra verbose output [minimal verbosity]\n");
		fprintf (stderr, "	-w Only write lines with warnings of wrong naming]\n");
		exit (1);
	}

	if (argc >= NFILES) {
		fprintf (stderr, "src_convention_check: Out of file space - increase NFILES and rebuild\n");
		exit (-1);		
	}
	
	/* Check that prototypes all are called gmt_* and that internals are all called gmtlib_* */

	fprintf (stderr, "src_convention_check: 0. Scanning include files for proper names\n");
	k = 0;
	while (API[k] != NULL) {
		if (strncmp (API[k], "GMT_", 4U)) fprintf (stderr, "gmt.h: Wrongly named function %s\n", API[k]);
		k++;
	}
	k = 0;
	while (libdev[k] != NULL) {
		if (strncmp (libdev[k], "gmt_", 4U)) fprintf (stderr, "gmt_prototypes.h: Wrongly named function %s\n", libdev[k]);
		k++;
	}
	k = 0;
	while (libint[k] != NULL) {
		if (strncmp (libint[k], "gmtlib_", 4U)) fprintf (stderr, "gmt_internals.h: Wrongly named function %s\n", libint[k]);
		k++;
	}
	
	fprintf (stderr, "src_convention_check: 1. Scanning all codes for function declarations\n");
	for (k = 1; k < argc; k++) {	/* For each input file */
		if (strcmp (argv[k], "-e") == 0) {	/* Only list external functions and not static */
			ext = 1;
			continue;
		}
		if (strcmp (argv[k], "-f") == 0) {	/* Only list functions and not where called */
			brief = 1;
			continue;
		}
		if (strcmp (argv[k], "-o") == 0) {	/* Write to stdout */
			log = 0;
			continue;
		}
		if (strcmp (argv[k], "-v") == 0) {	/* Extra verbos */
			verbose = 1;
			continue;
		}
		if (strcmp (argv[k], "-w") == 0) {	/* Warnings only e*/
			warn_only = 1;
			continue;
		}

		if ((fp = fopen (argv[k], "r")) == NULL) {
			fprintf (stderr, "src_convention_check: Unable to open %s for reading\n", (argv[k]));
			continue;
		}
		if (verbose) fprintf (stderr, "\tsrc_convention_check: Scanning %s\n", argv[k]);
		set_lib = (strstr (argv[k], "gmt_") != NULL || strstr (argv[k], "common_") != NULL);	/* Called in a library file */
		comment = rec = 0;
		while (fgets (line, 512, fp)) {
			rec++;
			if (!comment && strstr (line, "/*") && strstr (line, "*/") == NULL)	/* Start of multi-line comment */
				comment = 1;
			else if (comment && strstr (line, "*/")) {	/* End of multi-line comment with this line */
				comment = 0;
				continue;
			}
			if (comment) continue;
			if (strchr (" \t#/{", line[0])) continue;
			if (line[1] == '*') continue;	/* Comment */
			if (strchr (line, '(') == NULL) continue;
			if (!strncmp (line, "EXTERN", 6U)) continue;
			if (!strncmp (line, "extern", 6U)) continue;
			if (strstr (line, "typedef")) continue;
			wipe_line (line);
			n = sscanf (line, "%s %s %s %s %s %s", word[0], word[1], word[2], word[3], word[4], word[5]);
			if (n < 2) continue;
			w = is_static = 0;
			if (!strcmp (word[w], "if") || !strcmp (word[w], "for") || !strcmp (word[w], "while") || !strcmp (word[w], "else") || !strcmp (word[w], "enum")) continue;
			if (strchr (word[0], ':')) continue;	/* goto label */
			if (strchr (word[0], '(')) continue;	/* function call */
			if (!strcmp (word[w], "GMT_LOCAL") || !strcmp (word[w], "static")) {
				w = is_static = 1;
			}
			if (ext && is_static) continue;	/* Only list external functions since -e is set */
			if (!strncmp (word[w], "inline", 6U))
				w++;	/* Skip inline  */
			if (!strncmp (word[w], "const", 5U))
				w++;	/* Skip inline  */
			if (!strncmp (word[w], "struct", 6U)) {	/* Skip "struct NAME" */
				w += 2;
				if (strchr (word[w], '{')) continue;	/* Was a structure definition */
			}
			else if (!strncmp (word[w], "unsigned", 8U) || !strncmp (word[w], "signed", 6U))
				w += 2;	/* Skip "unsigned short" */
			else	/* skip "double", etc. */
				w++;
			if (w >= n) continue;
			if (!strcmp (word[w], "*") || !strcmp (word[w], "**"))
				w++;	/* Skip a lonely * */
			if (w >= n) continue;
			s = (word[w][0] == '*') ? 1 : 0;	/* Skip leading * if there is no space */
			if (strchr (&word[w][s], '[')) continue;	/* Got some array */
			L = strlen (word[w]);
			if (strcmp (&word[w][s], "parse") == 0 || strcmp (&word[w][s], "usage") == 0 || strcmp (&word[w][s], "New_Ctrl") == 0) continue;	/* Let these be named as is */
			//if (L > 5 && strncmp (&word[w][s], "GMT_", 4U) == 0) continue;	/* Skip GMT API functions */
			if (L > 5 && strncmp (&word[w][s], "PSL_", 4U) == 0) continue;	/* Skip PSL functions */
			if (word[w][L-1] == '_') continue;	/* Skip FORTRAN wrappers */
			if ((p = strchr (word[w], '('))) p[0] = '\0';	/* Change functionname(args) to functionname */
			if (strcmp (&word[w][s], "main") == 0) continue;	/* Skip main functions in modules */
			if (strlen (&word[w][s]) == 0) continue;
			if ((f = find_function (F, n_funcs, &word[w][s])) == -1) {
				f = n_funcs++;	/* Add one more */
				strncpy (F[f].name, &word[w][s], 63);
				strncpy (F[f].file, argv[k], 63);
				if (is_recognized (F[f].name, API))
					F[f].api = 1;
				else if (is_recognized (F[f].name, libdev))
					F[f].declared_dev = 1;
				else if (is_recognized (F[f].name, libint))
					F[f].declared_lib = 1;
				if (L > 5 && strncmp (F[f].name, "GMT_", 4U) == 0 && is_recognized (&F[f].name[4], modules))
					F[f].api = 1;
				F[f].declared_local = is_static;
				F[f].rec = rec;
				F[f].in = calloc (NFILES, 1U);
			}
			if (n_funcs == NFUNCS) {
				fprintf (stderr, "src_convention_check: Out of function space - increase NFUNCS and rebuild\n");
				exit (-1);
			}
		}
		fclose (fp);
	}
	/* Look for function calls */
	fprintf (stderr, "src_convention_check: 2. Scanning all codes for function calls\n");
	for (k = 1; k < argc; k++) {	/* For each input file */
		if ((fp = fopen (argv[k], "r")) == NULL) continue;
		if (verbose) fprintf (stderr, "\tsrc_convention_check: Scanning %s\n", argv[k]);
		set_dev = is_recognized (argv[k], modules);	/* Called in a module */
		set_lib = (strstr (argv[k], "gmt_") != NULL || strstr (argv[k], "common_") != NULL);	/* Called in a library file */
		rec = 0;
		while (fgets (line, 512, fp)) {
			rec++;
			if (line[0] == '/' || line[1] == '*') continue;	/* Comment */
			if (strchr (" \t", line[0]) == NULL) continue;	/* Not a called function */
			wipe_line (line);
			if (strchr (line, '(') == NULL) continue;	/* Not a function call */
			t = 0;
			while (strchr ("\t", line[t])) t++; /* Wind past leading white space */
			for (f = 0; f < n_funcs; f++) {
				L = strlen (F[f].name);
				if ((p = strstr (&line[t], F[f].name)) == NULL) continue;
				q = p-1;	/* Previous char (which we know exists, so q[0] below is valid */
				if (strlen (p) > (L+2) && (p[L] == '(' || (p[L] == ' ' && p[L+1] == '(')) && strchr (" \t()", q[0])) {	/* Found a call to this function */
					F[f].in[k] = 1;
					F[f].n_calls++ ;
				}
			}
		}
	}
	for (f = 0; f < n_funcs; f++) {
		for (k = 1; k < argc; k++)	/* For each input file */
			if (F[f].in[k]) F[f].n_files++;
	}
	qsort (F, n_funcs, sizeof (struct FUNCTION), compare_n);

	fprintf (stderr, "src_convention_check: Write the report\n");
	/* Report */
	if (log) out = fopen ("/tmp/gmt/scan.txt", "w");
	fprintf (out, "LIBRARY CODES: A = GMT API, D = GMTDEV LIB, L = INTERNAL LIB, S = STATIC\n");
	fprintf (out, "NFILES  FUNCTION                               NCALLS LIB  DECLARED-IN-FILE                   LINE  MESSAGE\n");
	for (f = 0; f < n_funcs; f++) {
		err = 0;
		p = basename (F[f].file);
		get_contraction (p, prefix);
		L = strlen (prefix);
		k = (F[f].declared_local) ? 0 : ((F[f].declared_dev) ? 1 : 2);
		if (F[f].api) k = 3;
		if (F[f].declared_local) {
			if (strncmp (F[f].name, prefix, L)) err = 3;
		}
		else if (F[f].declared_dev) {
			if (strncmp (F[f].name, "gmt_", 4U)) err = 1;
		}
		else if (F[f].declared_lib) {
			if (strncmp (F[f].name, "gmtlib_", 7U)) err = 2;
		}
		if (err == 3) {
			if (F[f].n_files > 1)
				strcpy (message, err_msg[err]);
			else {
				sprintf (message, "Change to %s_* ", prefix);
			}
		}
		else
			strcpy (message, err_msg[err]);
		if (!F[f].declared_local && F[f].n_files <= 1 && !F[f].api) {
			strcat (message, "Static function candidate");
			err = 3;
		}
		if (!warn_only || err) {
			fprintf (out, "%5d   %-40s %4d  %c   %-32s %6d  %s\n", F[f].n_files, F[f].name, F[f].n_calls, type[k], F[f].file, F[f].rec, message);
			if (brief) {	/* Done with this, free memory */
				free ((void *)F[f].in);
				continue;
			}
			for (k = 1; k < argc; k++)	/* For each input file, report each incident */
				if (F[f].in[k] && strcmp (argv[k], F[f].file)) fprintf (out, "\t\t%s\n", argv[k]);
		}
		free ((void *)F[f].in);
	}
	if (log) {
		fclose (out);
		fprintf (stderr, "src_convention_check: Report written to /tmp/gmt/scan.txt\n");
	}
}