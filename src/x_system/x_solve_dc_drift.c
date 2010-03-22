/*	$Id: x_solve_dc_drift.c,v 1.12 2010-03-22 18:55:47 guru Exp $
 *
 * x_solve_dc_drift reads the xx_* databases and computes the best
 * fitting drift and dc values using a least squares method.
 * The result from this program is a drift and dc-shift value for
 * each data-type (g/m/b) for all the legs in the database. To use
 * this information and correct the data, use:
 *
 *	corrected_value = uncorrected_value - dc_shift -
 *			  drift * data_point_time_in_seconds_from_year1.
 *
 * Author:	Paul Wessel
 * Date:	7-NOV-1987
 * Modified:	18-FEB-1989	PW: Fixed bugs in the for (j = 0; j < 3... loops
 *				15-AUG-1989 PW: Added new options to select data types to be used
 *				29-AUG-1989 PW: Allowed for more arrayspace for legnames
 *				06-MAR-2000 PW: POSIX
 *
 */
 
#include "gmt.h"
#include "x_system.h"

#define MAX_LEGS 5000
#define MLEGS 5000	/* Max number of legs to ignore and to use */

struct LEG leg[MAX_LEGS];

struct XOVERS crossover;
struct CORR bin;

size_t binsize = sizeof(struct CORR);
size_t legsize = sizeof(struct LEG);
GMT_LONG nlegs=0, nbadlegs=0, nuselegs = 0;
GMT_LONG get_id (char *name);
GMT_LONG findbad (char *name);
GMT_LONG finduse (char *name);

char badlegs[MLEGS][10];
char uselegs[MLEGS][10];

GMT_LONG legsum_n[MAX_LEGS][3], sum_n[3], n[3];
double dc[MAX_LEGS][3], drift[MAX_LEGS][3], sum[3], sum2[3];
double legsum_t[MAX_LEGS][3], legsum_x[MAX_LEGS][3], legsum_tt[MAX_LEGS][3], legsum_tx[MAX_LEGS][3], mean[3], stdev[3];
double sum_t1[3], sum_t2[3], sum_x[3], sum_tt1[3], sum_tt2[3], sum_t1x[3], sum_t2x[3];

int main (int argc, char **argv)
{
	GMT_LONG n_iterations = 0, iteration = 0, reset = FALSE, i, j, error = FALSE, ok, min_nx = 0;
	GMT_LONG n_x, id_1, id_2, test_area = FALSE, do_gmt[3];
	double t_1, t_2, xover, div, drift_inc, dc_inc;
	double west = 0.0, east = 360.0, south = -90.0, north = 90.0;
	GMT_LONG west_i = 0, east_i = 0, south_i = 0, north_i = 0, lon_i;
	GMT_LONG bin_on = FALSE, asc_on = FALSE, verbose = FALSE;
	char lfile[80], file[80], header[REC_SIZE], lega[10], legb[10], string[10], filea[80], fileb[80], type[3], line[BUFSIZ], *c_not_used = NULL;
	FILE *fpl = NULL, *fpb = NULL, *fpi = NULL, *fpbin = NULL, *fpasc = NULL, *fpu = NULL;
	size_t not_used = 0;

	do_gmt[0] = do_gmt[1] = do_gmt[2] = FALSE;
	type[0] = 'G';	type[1] = 'M';	type[2] = 'T';

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'X':
					fpb = fopen(&argv[i][2], "rb");
					break;
				case 'L':
					strcpy(lfile, &argv[i][2]);
					fpl = fopen(lfile, "rb");
					break;
				case 'I':	/* Legs to ignore */
					fpi = fopen(&argv[i][2], "r");
					break;
				case 'C':
					n_iterations = atoi(&argv[i][2]);
					break;
				case 'D':
					min_nx = atoi(&argv[i][2]);
					break;
				case 'S':
					reset = TRUE;
					break;
				case 'R':
					sscanf (&argv[i][2], "%lf/%lf/%lf/%lf", &west, &east, &south, &north);
					break;
				case 'U':	/* Legs to use */
					fpu = fopen(&argv[i][2], "r");
					break;
				case 'G' :
					do_gmt[0] = TRUE;
					break;
				case 'M' :
					do_gmt[1] = TRUE;
					break;
				case 'T' :
					do_gmt[2] = TRUE;
					break;
				case 'B' :
					bin_on = TRUE;
					strcpy (fileb, &argv[i][2]);
					break;
				case 'A' :
					asc_on = TRUE;
					strcpy (filea, &argv[i][2]);
					break;
				case 'V' :
					verbose = TRUE;
					break;
				default:
					error = TRUE;
					break;
			}
		}
		else
			error = TRUE;
	}
	if (fpb == NULL && (fpb = fopen ("xx_base.b", "rb")) == NULL) {
		fprintf (stderr, "Could not find file xx_base.b\n");
		exit (EXIT_FAILURE);
	}
	if (fpl == NULL) { /* Try default file */
		if ((fpl = fopen ("xx_legs.b", "rb")) == NULL) {
			fprintf (stderr, "Could not find file xx_legs.b\n");
			exit (EXIT_FAILURE);
		}
		strcpy (lfile, "xx_legs.b");
	}
	if (west > east) east += 360.;
	if (west < 0.0) {
		west += 360.0;
		east += 360.0;
	}
	if (west > east || south > north) error = TRUE;
	if (west != 0.0 || east != 360.0 || south != -90.0 || north != 90.0) {
		test_area = TRUE;
		west_i = (GMT_LONG) floor (west * 1.0E6);
		east_i = (GMT_LONG) ceil (east * 1.0E6);
		south_i =(GMT_LONG) floor ( south * 1.0E6);
		north_i = (GMT_LONG) ceil (north * 1.0E6);
	}
	if (!bin_on && !asc_on) error = TRUE;

	if (argc == 1 || error) {
		fprintf(stderr, "x_solve_dc_drift - Find crossover correction terms\n\n");
		fprintf(stderr, "usage: x_solve_dc_drift [-X<xbasefile>] [-L<xlegsfile>] [-I<ignore-file>\n");
		fprintf(stderr, "	-G -M -T -S -C<n_iterations> -D<min_nx> -B<binfile> -A<asciifile>\n");
		fprintf(stderr, "	-R<west>/<east>/<south>/<north>]\n\n");
		fprintf(stderr, "	-X -L to specify the data base files [Default is xx_base.b/xx_legs.b].\n");
		fprintf(stderr, "	-G -M -T to solve for dc/drift for those data types [Default is -G -M -T]\n");
		fprintf(stderr, "	-S will reset the old dc/drift values in the xx_legs.b file.\n");
		fprintf(stderr, "	-C specifies how many iterations. Default is interactive session\n");
		fprintf(stderr, "	-I specifies list of legs to be skipped [Default is none].\n");
		fprintf(stderr, "	-U specifies list of legs to be used [Default is all].\n");
		fprintf(stderr, "	   (Current max number of legs for -I and -U is %d\n", MLEGS);
		fprintf(stderr, "	-R to use cross-overs inside region only [Default is world].\n");
		fprintf(stderr, "	-D solves for drift only if leg has more than min_nx crossovers\n");
		fprintf(stderr, "	-B will write a binary output correction file\n");
		fprintf(stderr, "	-A will write a ASCII output correction file\n");
		fprintf(stderr, "	  Specify at least one of -A and -B\n");
		fprintf(stderr, "	-V means verbose\n");

		exit (EXIT_FAILURE);
	}
	/* Read the ignore-legs file if needed */
	if (fpi != NULL) {
		while (fgets (line, BUFSIZ, fpi)) {
			sscanf (line, "%s", badlegs[nbadlegs]);
			nbadlegs++;
			if (nbadlegs >= MLEGS) {
				fprintf (stderr, "x_solve_dc_drift: Too many legs to ignore! (> %d), recompile\n", MLEGS);
				exit (EXIT_FAILURE);
			}
		}
		fclose (fpi);
	}

	/* Read the uselegs file if needed */
	if (fpu != NULL) {
		while (fgets (line, BUFSIZ, fpu)) {
			sscanf (line, "%s", uselegs[nuselegs]);
			nuselegs++;
			if (nuselegs >= MLEGS) {
				fprintf (stderr, "x_solve_dc_drift: Too many legs to use! (> %d), recompile\n", MLEGS);
				exit (EXIT_FAILURE);
			}
		}
		fclose (fpu);
	}

	/* Read xx_legs.b file */
	i = 0;
	while (fread ((void *)&leg[i], (size_t)legsize, (size_t)1, fpl) == 1) i++;
	fclose (fpl);
	nlegs = i;

	/* Start the iterative least squares solution */
	ok = TRUE;
	if (!(do_gmt[0] || do_gmt[1] || do_gmt[2])) do_gmt[0] = do_gmt[1] = do_gmt[2] = TRUE;	/* Default is do all 3 data types */

	if (!reset) {	/* Use the previously found dc/drift values as startvalues */
		for (i = 0; i < nlegs; i++) {
			for (j = 0; j < 3; j++) {
				if (!do_gmt[j]) continue;
				dc[i][j] = leg[i].dc_shift_gmt[j];
				drift[i][j] = leg[i].drift_rate_gmt[j];
			}
		}
	}

	for (i = 0; i < nlegs; i++) {	/* Initialize counters */
		for (j = 0; j < 3; j++) {	/* For each data-type g/m/b */
			if (!do_gmt[j]) continue;
			legsum_t[i][j] =  legsum_tt[i][j] = 0.0;
			legsum_n[i][j] = 0;
		}
	}

	if (verbose) fprintf(stderr, "x_solve_dc_drift: Starts iterating\n");

	while (ok) {
		iteration++;
		for (i = 0; i < nlegs; i++) {	/* Initialize counters */
			for (j = 0; j < 3; j++)	{/* For each data-type g/m/b */
				if (!do_gmt[j]) continue;
				legsum_x[i][j] = legsum_tx[i][j] = 0.0;
			}
		}

		for (j = 0; j < 3; j++) {	/* Initialize total counters */
			if (!do_gmt[j]) continue;
			n[j] = 0;
			sum[j] = 0.0;
			sum2[j] = 0.0;
		}

		fseek (fpb, (long)REC_SIZE, SEEK_SET);

		while (fread ((void *)header, (size_t)REC_SIZE, (size_t)1, fpb) == 1) {
			sscanf(header, "%s %s %ld",lega, legb, &n_x);
			if (!strcmp(lega, legb)) {	/* Internal crossovers, skip this pair */
				fseek (fpb, (long int)(n_x*REC_SIZE), SEEK_CUR);
				continue;
			}
			if (nbadlegs > 0 && (findbad (lega) || findbad(legb))) {	/* Skip bad leg */
				fseek (fpb, (long int)(n_x*REC_SIZE), SEEK_CUR);
				continue;
			}
			if (nuselegs > 0 && !(finduse (lega) && finduse(legb))) {	/* Skip pair, not desired */
				fseek (fpb, (long int)(n_x*REC_SIZE), SEEK_CUR);
				continue;
			}


			if ((id_1 = get_id (lega)) == -1) {
				fprintf(stderr, "xsolve_dc_shift: Leg %s not found!\n", lega);
				exit (EXIT_FAILURE);
			}
			if ((id_2 = get_id (legb)) == -1) {
				fprintf(stderr, "xsolve_dc_shift: Leg %s not found!\n", legb);
				exit (EXIT_FAILURE);
			}

			for (j = 0; j < 3; j++) {	/* Set this pairs tmp-counters to zero */
				if (!do_gmt[j]) continue;
				sum_n[j]   = 0;
				sum_t1[j]  = sum_t2[j]  = 0.0;
				sum_x[j]   = 0.0;
				sum_tt1[j] = sum_tt2[j] = 0.0;
				sum_t1x[j] = sum_t2x[j] = 0.0;
			}

			/* Sum up the statistics for these crossovers */

			for (i = 0; i < n_x; i++) {
				not_used = fread ((void *)&crossover, (size_t)REC_SIZE, (size_t)1, fpb);
				if (test_area) { /* Must see if xover is inside the area specified */
					if (crossover.lat < south_i || crossover.lat > north_i) continue;
					lon_i = crossover.lon;
					while (lon_i < west_i) lon_i += 360000000;
					if (lon_i > east_i) continue;
				}
				t_1 = crossover.xtime[0] * 0.1;
				t_2 = crossover.xtime[1] * 0.1;
				for (j = 0; j < 3; j++) {
					if (!do_gmt[j]) continue;
					if (crossover.x_val[j] == NODATA) continue;	/* Skip to next data-type */
					xover = crossover.x_val[j] - dc[id_1][j] + dc[id_2][j]
						  - drift[id_1][j]*t_1 + drift[id_2][j]*t_2;
					sum_x[j] += xover;
					sum_t1x[j] += t_1*xover;
					sum_t2x[j] -= t_2*xover;	/* - since leg2 now is leg1 */
					if (iteration == 1) {	/* Do this once only */
						sum_n[j]++;
						sum_t1[j] += t_1;
						sum_t2[j] += t_2;
						sum_tt1[j] += t_1*t_1;
						sum_tt2[j] += t_2*t_2;
					}
					if (id_1 < id_2) {
						sum[j] += xover;
						sum2[j] += xover*xover;
						n[j]++;
					}
				}
			}

			for (j = 0; j < 3; j++) {	/* Add this info to each leg's totals */
				if (!do_gmt[j]) continue;
				if (iteration == 1) {
					legsum_n[id_1][j] += sum_n[j];
					legsum_t[id_1][j] += sum_t1[j];
					legsum_tt[id_1][j] += sum_tt1[j];
					legsum_n[id_2][j] += sum_n[j];
					legsum_t[id_2][j] += sum_t2[j];
					legsum_tt[id_2][j] += sum_tt2[j];
				}
				legsum_x[id_1][j] += sum_x[j];
				legsum_tx[id_1][j] += sum_t1x[j];
				legsum_x[id_2][j] -= sum_x[j];
				legsum_tx[id_2][j] += sum_t2x[j];
			}
		}

		/* Solve for the best-fitting regression lines for each leg and datatype */

		for (i = 0; i < nlegs; i++) {
			for (j = 0; j < 3; j++) {
				if (!do_gmt[j]) continue;
				div = legsum_n[i][j]*legsum_tt[i][j] - legsum_t[i][j]*legsum_t[i][j];
				if (div != 0.0) {
					drift_inc = (legsum_n[i][j] > min_nx) ?
						(legsum_n[i][j]*legsum_tx[i][j] - legsum_t[i][j]*legsum_x[i][j])/div : 0.0;
					dc_inc = (legsum_n[i][j] > min_nx) ?
						(legsum_tt[i][j]*legsum_x[i][j] - legsum_t[i][j]*legsum_tx[i][j])/div :
						legsum_x[i][j] /legsum_n[i][j];
					drift[i][j] += drift_inc;
					dc[i][j] += dc_inc;
				}
			}
		} 

		for (j = 0; j < 3; j++) {
			if (!do_gmt[j]) continue;
			if (n[j] > 1) {
				mean[j] = sum[j]/n[j];
				stdev[j] = sqrt((sum2[j] - mean[j]*sum[j])/(n[j]-1.));
			}
		}

		printf("Before iteration # %ld we have:\n", iteration);
		for (j = 0; j < 3; j++) {
			if (!do_gmt[j]) continue;
			printf("%c >>> Mean: %8.3f St.Deviation: %8.3f n: %6ld\n", type[j], mean[j], stdev[j], n[j]);
		}

		if (n_iterations == 0) {
			printf ("One more iteration?: ");
			c_not_used = fgets (string, (size_t)10, stdin);
			if (string[0] == 'N' || string[0] == 'n') ok = FALSE;
		}
		else if (iteration >= n_iterations)
			ok = FALSE;
	}

	/* Write out the new xx_legs.b file */

	if (verbose) fprintf (stderr, "x_solve_dc_drift: Creates new xx_legs.b file\n");

	for (i = 0; i < nlegs; i++) {
		for (j = 0; j < 3; j++) {
			if (!do_gmt[j]) continue;
			leg[i].dc_shift_gmt[j] = dc[i][j];
			leg[i].drift_rate_gmt[j] = drift[i][j];
		}
	}
	if (reset) {
		sprintf(file,"%s_old", lfile);
		if (rename (lfile, file))
			fprintf (stderr, "Could not rename %s to %s. Left as is.\n", file, lfile);
		else {
			fpl = fopen(lfile, "w");
			for (i = 0; i < nlegs; i++)
				not_used = fwrite((char *)&leg[i], (size_t)legsize, (size_t)1, fpl);
			fclose(fpl);
		}
	}

	/* Create correction file(s) */

	if (bin_on) {
		fpbin = fopen(fileb, "wb");
		if (verbose) fprintf (stderr, "x_solve_dc_drift: Create binary correction file %s\n", fileb);
	}
	if (asc_on) {
		fpasc = fopen(filea, "w");
		if (verbose) fprintf (stderr, "x_solve_dc_drift: Create ASCII correction file %s\n", filea);
		fprintf (fpasc, "leg\tyear\td.c.-G\tdrift-G\td.c.-M\tdrift-M\td.c.-T\tdrift-T\n");
	}

	for (i = 0; i < nlegs; i++) {
		if (nbadlegs > 0 && findbad (leg[i].name)) continue;
		if (nuselegs > 0 && !finduse (leg[i].name)) continue;
		if (bin_on) {	/* Use binary output format */
			strcpy (bin.name, leg[i].name);
			bin.year = leg[i].year;
			for (j = 0; j < 3; j++) {
				if (!do_gmt[j]) continue;
				bin.dc_shift_gmt[j] = (float) leg[i].dc_shift_gmt[j];
				bin.drift_rate_gmt[j] = (float) leg[i].drift_rate_gmt[j];
			}
			not_used = fwrite ((void *)&bin, (size_t)binsize, (size_t)1, fpbin);
		}
		if (asc_on) {	/* Use ASCII output format */
			fprintf (fpasc, "%s\t%d\t%.2f\t%g\t%.2f\t%g\t%.2f\t%g\n",
				leg[i].name,
				leg[i].year,
				leg[i].dc_shift_gmt[0],
				leg[i].drift_rate_gmt[0],
				leg[i].dc_shift_gmt[1],
				leg[i].drift_rate_gmt[1],
				leg[i].dc_shift_gmt[2],
				leg[i].drift_rate_gmt[2]);
		}
	}
	if (bin_on) fclose (fpbin);
	if (asc_on) fclose (fpasc);
	if (verbose) fprintf (stderr, "x_solve_dc_drift: Done!\n");
	exit (EXIT_SUCCESS);
}

GMT_LONG get_id (char *name)
{
	GMT_LONG left, right, mid, cmp;

	left = 0;
	right = nlegs-1;
	while (left <= right) {
		mid = (left + right)/2;
		cmp = strcmp(name, leg[mid].name);
		if (cmp < 0)
			right = mid-1;
		else if (cmp > 0)
			left = mid+1;
		else
			return (mid);
	}
	return (-1);
}

GMT_LONG findbad (char *name)
{
	GMT_LONG left, right, mid, cmp;

	left = 0;
	right = nbadlegs-1;
	while (left <= right) {
		mid = (left + right)/2;
		cmp = strcmp(name, badlegs[mid]);
		if (cmp < 0)
			right = mid-1;
		else if (cmp > 0)
			left = mid+1;
		else
			return (1);
	}
	return (0);
}

GMT_LONG finduse (char *name)
{
	GMT_LONG left, right, mid, cmp;

	left = 0;
	right = nuselegs-1;
	while (left <= right) {
		mid = (left + right)/2;
		cmp = strcmp(name, uselegs[mid]);
		if (cmp < 0)
			right = mid-1;
		else if (cmp > 0)
			left = mid+1;
		else
			return (1);
	}
	return (0);
}
