#include "gmt_dev.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

/*! . */
void gmt_usage_line (unsigned int mode, size_t MLENGTH, int type, char *in_line) {
	/* Clever printf scheme to output the contents of line which are basically
	 * words separated by whitespace.  We split this over as many lines that are
	 * necessary to keep a max width. */

	size_t i, o, length_of_line, start = 0, stop = 0, previous_stop, n_words, last_output_length, this_output_length;
	char *regular_tab = "  ", *no_tab = "", *T = no_tab, c;
	char *line_break[2] = {"..", "\xe2\x8f\x8e"};	/* Index 0 is plain, 1 is ISO code */
	//char *line_continue[2] = {"\t  ...", "\t.."};	/* Index 0 is plain, 1 is ISO code */
	char line[GMT_BUFSIZ] = {""};
	char *line_continue[2] = {"  ..", "  .."};	/* Index 0 is plain, 1 is ISO code */
	bool breaking;	/* true when we must break the line */

	MLENGTH -= 2;	/* Allow for the 2-space TAB */
	for (i = o = 0; i < strlen (in_line); i++) {
		if (in_line[i] == '\t') line[o++] = ' ', line[o++] = ' ';
		else line[o++] = in_line[i];
	}
	line[o] = '\0';
	length_of_line = strlen (line);
	while (start < length_of_line) {	/* While there is still more text on the current line... */
		while (start < length_of_line && line[start] == ' ') start++;	/* Advance past spaces */
		stop = start;	n_words = 0;	/* Note where we are */
		while ((stop - start) < MLENGTH && stop < length_of_line) {	/* While we are within the length requirements... */
			last_output_length = stop - start;	/* Remember previous increase */
			while (stop < length_of_line && line[stop] == ' ') stop++;	/* Advance past spaces */
			previous_stop = stop;	/* Remember last stop in case we go too far */
			while (stop < length_of_line && !(line[stop] == ' ')) stop++;	/* Now continue to the next space between options */
			n_words++;
			//fprintf (stderr, "last_output_length = %d n_words = %d\n", last_output_length, n_words);
		}
		this_output_length = stop - start;	/* This is the length of the proposed outline line */
		if (this_output_length == MLENGTH || n_words == 1 || stop == length_of_line) previous_stop = stop;	/* We OK this decision */
		breaking = false;
		//if (this_output_length > irint (1.1*MLENGTH) && (this_output_length - last_output_length) > irint (0.3 * MLENGTH)) {
		if (this_output_length > (MLENGTH-3)) {
			/* We get here is the current line exceeds the desired length by 10% and that the
			 * item that put us over the limit is > 30% of the desired length, then we break. */
			//fprintf (stderr, "Need to break.  this_output_length = %d previous_stop = %d\n", this_output_length, previous_stop);
			/* Rewind to ! MLENGTH after start */
			stop = start + MLENGTH - 3;	/* Initial guess for a break point */
			//while (stop < length_of_line && line[stop] != ']') stop++;	/* Look for the first ] to indicate end of argument min-option */
			while (stop && line[stop] != '[') stop--;	/* Look for the first ] to indicate end of argument min-option */
			if (stop < length_of_line) {	/* OK, break here */
				//previous_stop = stop + 1;
				previous_stop = stop ;
				breaking = true;
			}
		}
		/* Print current line */
		if (previous_stop < length_of_line) {	/* Must chop off the remaining line before printing */
			c = line[previous_stop];	/* Remember the char we replace with 0 */
			line[previous_stop] = '\0';	/* Truncate */
		}
		if (breaking)	/* Time to do a hard break with break-symbol */
			fprintf (stderr, "%s%s%s\n", T, &line[start], line_break[type]);
		else	/* Print out the reset of this line, possibly starting with continue-symbol */
			fprintf (stderr, "%s%s\n", T, &line[start]);
		if (previous_stop < length_of_line)	/* Replace the 0 with what we saved and continue from there */
			line[previous_stop] = c;
		start = previous_stop;	/* Now move start to where we ended */
		T = (breaking) ? line_continue[type] : regular_tab;	/* See if we need a continue symbol or just regular tab */
	}
}

int main (int argc, char *argv[]) {
	char text[2048] = {""};
	int length_of_line = 80, type = 0, n_columns;
	const char *name = "somemodule";
#ifdef WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    n_columns = csbi.srWindow.Right;
#else
	struct winsize w;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &w);
    n_columns = w.ws_col;
#endif
	sprintf (text, "usage: %s <grid> [-A] [-C] [%s] [-E[a|e|h|l|r|t|v]] [-G<outgrid>] [%s] [-L[+n|p]] [-N<table>] [%s] [-S] [-T] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]",
		name, GMT_GRDEDIT2D, GMT_J_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);


	if (argc > 1) {	/* Gave a length argument */
		if (argv[1][0] == '-') length_of_line = n_columns;	/* use the terminal width */
		else length_of_line = atoi (argv[1]);	/* Use what was given to us */
	}
	if (argc == 3) type = 1;	/* If we gave a 2nd arg it means use fancy ISO line-break symbol */
	//fprintf (stderr, "ORIG: %s\n\n\n", text);
	fprintf (stderr, "WRAP: %d characters\n\n", length_of_line);
	gmt_usage_line (GMT_TIME_NONE, length_of_line, type, text);
}
