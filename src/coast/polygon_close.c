#include "wvs.h"

int main (int argc, char **argv) {
	FILE *fp1, *fp2;
	int i, j, n1 = 0, n2 = 0, i0 = 0, j0 = 0;
	char line[GMT_BUFSIZ];
	double x1[10000], x2[10000], y1[10000], y2[10000], d, dmin = DBL_MAX;
	
	sprintf (line, "polygon.%s", argv[1]);
	fp1 = fopen (line, "r");
	while (fgets (line, GMT_BUFSIZ, fp1)) {
		sscanf (line, "%lf %lf", &x1[n1], &y1[n1]);
		n1++;
	}
	fclose (fp1);
	sprintf (line, "polygon.%s", argv[2]);
	fp2 = fopen (line, "r");
	while (fgets (line, GMT_BUFSIZ, fp2)) {
		sscanf (line, "%lf %lf", &x2[n2], &y2[n2]);
		n2++;
	}
	fclose (fp2);
	n1--;	n2--;
	for (i = 0; i < n1; i++) {
		for (j = 0; j < n2; j++) {
			d = hypot ((x1[i]-x2[i])*cosd(0.5*(y1[i] + y2[j])), y1[i] - y2[j]);
			if (d < dmin) {
				dmin = d;
				i0 = i;
				j0 = j;
			}
		}
	}
	printf ("Pol %s # %d is close to Pol %s # %d [%g m]\n", argv[1], i0, argv[2], j0, dmin*111113.0);
	return(0);
}
