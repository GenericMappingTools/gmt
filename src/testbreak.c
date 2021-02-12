#include "gmt_dev.h"

/*! . */
void gmt_usage_line (unsigned int mode, size_t MLENGTH, int type, char *line) {
	/* Clever printf scheme to output the contents of line which are basically
	 * words separated by whitespace.  We split this over as many lines that are
	 * necessary to keep a max width. */

	size_t L = strlen (line), a = 0, b = 0, b_last, n, last, this_L;
	char *tab = "\t", *tabc = "\t  ...", *none = "", *T = none, c;
	char *brk[2] = {"...", "\xe2\x8f\x8e"};
	char *cnt[2] = {"\t  ...", "\t  "};
	bool breaking;
	while (a < L) {
		while (a < L && line[a] == ' ') a++;
		b = a;	n = 0;
		while ((b - a) <= MLENGTH && b < L) {
			last = b - a;
			while (b < L && line[b] == ' ') b++;
			b_last = b;
			while (b < L && !(line[b] == ' ')) b++;
			n++;
			//fprintf (stderr, "last = %d n = %d\n", last, n);
		}
		this_L = b - a;
		if (this_L == MLENGTH || n == 1 || b == L) b_last = b;
		breaking = false;
		if (this_L > irint (1.1*MLENGTH) && (this_L - last) > irint (0.3 * MLENGTH)) {
			//fprintf (stderr, "Need to break.  this_L = %d b_last = %d\n", this_L, b_last);
			/* Rewind to ! MLENGTH after a */
			b = a + MLENGTH - 3;
			while (b < L && line[b] != ']') b++;
			if (b < L) {
				b_last = b + 1;
				breaking = true;
			}
		}
		/* Print line */
		if (b_last < L) {
			c = line[b_last];
			line[b_last] = '\0';
		}
		if (breaking)
			fprintf (stderr, "%s%s%s\n", T, &line[a], brk[type]);
		else
			fprintf (stderr, "%s%s\n", T, &line[a]);
		if (b_last < L)
			line[b_last] = c;
		a = b_last;
		T = (breaking) ? cnt[type] : tab;
	}
}

int main (int argc, char *argv[]) {
	char text[2048] = {""};
	int L = 80, type = 0;
	const char *name = "somemodule";
	sprintf (text, "usage: %s <grid> [-A] [-C] [%s] [-E[a|e|h|l|r|t|v]] [-G<outgrid>] [%s] [-L[+n|p]] [-N<table>] [%s] [-S] [-T] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]",
		name, GMT_GRDEDIT2D, GMT_J_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (argc > 1) L = atoi (argv[1]);
	if (argc == 3) type = 1;
	fprintf (stderr, "ORIG: %s\n\n\n", text);
	fprintf (stderr, "WRAP: %d characters\n\n", L);
	gmt_usage_line (GMT_TIME_NONE, L, type, text);
}
