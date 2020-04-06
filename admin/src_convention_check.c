/*
 * src_convention_check.c is used to check if we are following our own naming
 * convention for functions in the API (GMT_*), the core library (gmt_*),
 * the inter-file functions (gmtlib_*) or the in-file static function
 * (gmtfile_*).  We also try to determine how many files a function is called
 * in to look for candidate for static functions.
 * Options:
 *			-e Only list the external funtions and skip static ones
 *			-f Only list functions and not where they are called
 *
 * Paul Wessel, April 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NFILES 500
#define NFUNCS 10000

struct FUNCTION {
	char name[64];	/* Name of function */
	char file[64];	/* Name if file it is declared in */
	int local;		/* True if function declared static */
	unsigned int n_files;	/* How many files referenced */
	unsigned int n_calls;	/* How many times referenced */
	char *in;
};

int find_function (struct FUNCTION *F, int N, char *name) {
	int k;
	for (k = 0; k < N; k++)
		if (!strcmp (F[k].name, name)) return k;
	return -1;
}

static int compare_n (const void *v1, const void *v2) {
	const struct FUNCTION *F1 = v1, *F2 = v2;
	if (F1->n_files < F2->n_files) return +1;
	if (F1->n_calls > F2->n_files) return -1;
	if (F1->n_files < F2->n_calls) return +1;
	if (F1->n_calls > F2->n_calls) return -1;
	return (0);
}

int main (int argc, char **argv) {
	int k, f, w, s, n, is_static, n_funcs = 0, comment = 0, brief = 0, ext = 0;
	size_t L;
	char line[512] = {""};
	char word[6][64], type[2] = {'E', 'S'}, *p;
	struct FUNCTION F[NFUNCS];
	FILE *fp;

	if (argc == 1) {
		fprintf (stderr, "usage: src_convention_check [-e] [-f] *.c > log\n");
		fprintf (stderr, "	-e Only list external functions [all]\n");
		fprintf (stderr, "	-f Only list function stats and not where called [full log]\n");
		exit (1);
	}
	for (k = 1; k < argc; k++) {	/* For each input file */
		if (strcmp (argv[k], "-f") == 0) {	/* Only list functions and not where called */
			brief = 1;
			continue;
		}
		if (strcmp (argv[k], "-e") == 0) {	/* Only list external functions and not static */
			ext = 1;
			continue;
		}
		if ((fp = fopen (argv[k], "r")) == NULL) continue;
		comment = 0;
		while (fgets (line, 512, fp)) {
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
			n = sscanf (line, "%s %s %s %s %s", word[0], word[1], word[2], word[3], word[4]);
			if (n < 2) continue;
			w = is_static = 0;
			if (!strcmp (word[w], "if") || !strcmp (word[w], "for") || !strcmp (word[w], "while") || !strcmp (word[w], "else")) continue;
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
			if (L > 5 && strncmp (&word[w][s], "GMT_", 4U) == 0) continue;	/* Skip API functions */
			if (word[w][L-1] == '_') continue;	/* Skip FORTRAN wrappers */
			if ((p = strchr (word[w], '('))) p[0] = '\0';	/* Change functionname(args) to functionname */
			if (strcmp (&word[w][s], "main") == 0) continue;	/* Skip main functions in modules */
			if (strcmp (&word[w][s], "{") == 0)
				p = NULL;
			if ((f = find_function (F, n_funcs, &word[w][s])) == -1) {
				f = n_funcs++;	/* Add one more */
				strncpy (F[f].name, &word[w][s], 63);
				strncpy (F[f].file, argv[k], 63);
				F[f].local = is_static;
				F[f].in = calloc (NFILES, 1U);
			}
			if (n_funcs == NFUNCS) {
				fprintf (stderr, "Out of function space\n");
				exit (-1);
			}
		}
		fclose (fp);
	}
	/* Look for function calls */
	for (k = 1; k < argc; k++) {	/* For each input file */
		if ((fp = fopen (argv[k], "r")) == NULL) continue;
		while (fgets (line, 512, fp)) {
			if (line[0] == '/' || line[1] == '*') continue;	/* Comment */
			if (strchr (" \t", line[0]) == NULL) continue;
			if (strchr (line, '[')) continue;	/* Got some array */
			if (strchr (line, '(') == NULL) continue;	/* Not a function call */
			for (f = 0; f < n_funcs; f++) {
				L = strlen (F[f].name);
				if ((p = strstr (line, F[f].name)) && strlen (p) > L && (p[L] == '(' || p[L] == ' ')) {	/* Found a call to this function */
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

	/* Report */
	printf ("NFILES  FUNCTION                                    NCALLS TYPE DECLARED-IN\n");
	for (f = 0; f < n_funcs; f++) {
		printf ("%4d\t%-40s\t%4d\t%c\t%s\n", F[f].n_files, F[f].name, F[f].n_calls, type[F[f].local], F[f].file);
		if (brief) continue;
		for (k = 1; k < argc; k++)	/* For each input file */
			if (F[f].in[k] && strcmp (argv[k], F[f].file)) printf ("\t\t%s\n", argv[k]);
		free ((void *)F[f].in);
	}
}