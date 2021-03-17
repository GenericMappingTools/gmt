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
	 * a space (like between options) or the occurrence of "][" sequences.
	 * These are the places where we are allowed to break the line. */
	struct GMT_WORD *array = NULL;
	unsigned int n = 0, c, start = 0, next, end, j, stop, space = 0, n_alloc = GMT_LEN512;
	array = calloc (n_alloc, sizeof (struct GMT_WORD));
	while (line[start]) {	/* More to chop up */
		/* Find the next break location */
		stop = start;
		while (line[stop] && !(line[stop] == ' ' || (line[stop] == ']' && line[stop+1] == '['))) stop++;
		end = next = stop;	/* Mark likely end */
		array[n].space = space;	/* Do we need a leading space (set via previous word)? */
		if (line[stop] == ' ') {	/* Skip the space to start over at next word */
			while (line[stop] == ' ') stop++;	/* In case there are more than one space */
			next = stop; space = 1;
		}
		else if (line[stop] == ']') {	/* Include this char then break */
			next = ++end, space = 0;
		}
		array[n].word = calloc (end - start + 1, sizeof (char));	/* Allocate space for word */
		for (j = start, c = 0; j < end; j++, c++) array[n].word[c] = line[j];
		n++;	/* Got another word */
		start = next;	/* Advance to start of next word */
	}
	/* Finalize array length - keep one extra to indicate end of array */
	array = realloc (array, (n+1)*sizeof (struct GMT_WORD));
#if 1
	fprintf (stderr, "Found %d words\n", n);
	for (j = 0; j < n; j++)
		fprintf (stderr, "%2.2d: [%d] %s\n", j, array[j].space, array[j].word);
	fprintf (stderr, "\n");
#endif

	return (array);
}

void gmt_usage_line (unsigned int mode, unsigned int MLENGTH, char *in_line) {
	/* Break the in_ine across multiple lines determined by the terminal line width MLENGTH */
	struct GMT_WORD *W = gmt_split_words (in_line);
	unsigned int width, k, current_width = 0;
	char *brk = "\xe2\x8f\x8e", *cnt = "\xe2\x80\xa6";	/* return symbol and ellipsis */
	char message[BUFSIZ] = {""};
#ifdef WIN32
	SetConsoleOutputCP (CP_UTF8);
#endif
	if (mode) {	/* Start with 2-spaces in */
		strcat (message, "  ");	/* Starting 2-spaces */
		current_width = 2;
	}
	for (k = 0; W[k].word; k++) {	/* As long as there are more words... */
		width = (W[k+1].space) ? MLENGTH : MLENGTH - 1;	/* May need one space for ellipsis at end */
		if ((current_width + strlen (W[k].word) + W[k].space) < width) {	/* Word will fit on current line */
			if (W[k].space)	/* This word requires a leading space */
				strcat (message, " ");
			strcat (message, W[k].word);
			current_width += strlen (W[k].word) + W[k].space;	/* Update line width so far */
			free (W[k].word);	/* Free the word we are done with */
			if (W[k+1].word == NULL)	/* Finalize the last line */
				strcat (message, "\n");
		}
		else {	/* Must split at the current break point and continue on next line */
			if (W[k].space) { /* No break character needed since space separation is expected */
				strcat (message, "\n  ");
				current_width = 2;	/* Indent normal 2 spaces */
			}
			else {	/* Split in the middle of an option so append ellipsis and start with one too */
				strcat (message, brk);
				strcat (message, "\n   ");
				strcat (message, cnt);
				current_width = 4;
			}
			W[k].space = 0;	/* No leading space if starting a the line */
			k--;	/* Since k will be incremented by loop but we did not write this word yet */
		}
	}
	free (W);	/* Free the structure array */
	fprintf (stderr, "%s", message);
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
