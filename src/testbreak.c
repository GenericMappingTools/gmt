#include "gmt_dev.h"
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

struct GMT_WORD {
	char *word;
	unsigned int space;
};

struct GMT_WORD * gmt_split_words (char *line) {
	/* Split line into an array of words where words are separated by either
	 * a space (like between options) or the occurrence of "][" sequences. */
	struct GMT_WORD *array = NULL;
	unsigned int n = 0, c, start = 0, next, end, j, p, space = 0, n_alloc = GMT_LEN512;
	array = calloc (n_alloc, sizeof (struct GMT_WORD));
	while (line[start]) {	/* More to chop up */
		/* Find the next break location */
		p = start;
		while (line[p] && !(line[p] == ' ' || (line[p] == ']' && line[p+1] == '['))) p++;
		end = next = p;
		array[n].space = space;
		if (line[p] == ' ')	/* Skip the space to start at next word */
			next = p + 1, space = 1;
		else if (line[p] == ']') {	/* Include this char then break */
			next = ++end, space = 0;
		}
		array[n].word = calloc (end - start + 1, sizeof (char));	/* Allocate space for word */
		for (j = start, c = 0; j < end; j++, c++) array[n].word[c] = line[j];
		n++;	/* Got our word */
		start = next;
	}
	array = realloc (array, (n+1)*sizeof (struct GMT_WORD));
	fprintf (stderr, "Found %d words\n", n);
	for (j = 0; j < n; j++)
		fprintf (stderr, "%2.2d: [%d] %s\n", j, array[j].space, array[j].word);
	fprintf (stderr, "\n");

	return (array);
}

void gmt_usage_line (unsigned int mode, unsigned int MLENGTH, char *in_line) {
	struct GMT_WORD *W = gmt_split_words (in_line);
	unsigned int width = MLENGTH - 2, k, current_width = 0;
#ifdef WIN32
	char *brk = "\xe2\x80\xa6";
#else
	char *brk = "\xe2\x80\xa6";
#endif
	if (mode) {	/* Start with 2-spaces in */
		fprintf (stderr, "  ");	/* Starting 2-spaces */
		current_width = 2;
	}
	for (k = 0; W[k].word; k++) {
		width = (W[k+1].space) ? MLENGTH : MLENGTH - 1;
		if ((current_width + strlen (W[k].word) + W[k].space) < width) {	/* Word will fit */
			if (W[k].space) fprintf (stderr, " ");
			fprintf (stderr, "%s", W[k].word);
			current_width += strlen (W[k].word) + W[k].space;
			free (W[k].word);
		}
		else {	/* Must split */
			if (W[k].space) /* No break character since space separation */
				fprintf (stderr, "\n  "), current_width = 2;
			else
				fprintf (stderr, "%s\n   %s", brk, brk), current_width = 4;
			W[k].space = 0;	/* No leading space if starting the line */
			k--;	/* Since k will be incremented but we did not write this word on this line */
		}
	}
	fprintf (stderr, "\n");
	free (W);
}

int main (int argc, char *argv[]) {
	unsigned int n_columns;
#ifdef WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    n_columns = csbi.srWindow.Right;
#else
	struct winsize w;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);
    n_columns = w.ws_col;
#endif
    if (argc == 3) n_columns = atoi (argv[2]);
	fprintf (stderr, "WRAP: %d characters\n\n", n_columns);
	gmt_usage_line (0, n_columns, argv[1]);
	return 0;
}
