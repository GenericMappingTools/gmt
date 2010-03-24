/*
 *	$Id: x_list.c,v 1.11 2010-03-24 02:36:45 guru Exp $
 *
 * XLIST produces ASCII listings of cross-over info. The xx_base.b-file
 * contains longitude(x), latitude(y), time1(t1), time2(t2),  heading1(h),
 * heading2(h), crossover-values for gravity(g), magnetics(m), and
 * bathymetry(b), plus the average data value at the cross-over point for
 * gravity(G), magnetics(M), and bathymetry(B), and the user may extract any 
 * combination of these 10 parameters + the legnames + time (t1 for external,
 * abs(t2-t1) for internal) by using the option -txyhgmbGMB. The sequence in
 * which the flag characters appear determines the sequence in which the
 * parameters will be printed out.  If no options is specified, the default
 * is -txygmbGMBhl.
 * To select data inside an area, use the -W -E -S -N options:
 *	E.g. all data between 10 and 30 degree East: -W10. -E30.
 * To create a Scientist Helper ASCII Table format file, use -H.
 * 2, 1, or no legnames may be passed on the command line. If two is
 * passed, the external cross-overs generated between these legs are
 * returned. If one is passed, all the external cross-overs involving
 * this legs are returned (or internal if -I is used). No legs means
 * that all external (or internal) cross-overs will be returned.
 *
 * Author:	Paul Wessel
 * Date:	21-JAN-1988
 * Revised:	18-FEB-1989
 *		26-JUN-1989	PW: Print out NaN if no xover. Also -gmb will now
 *				    work even if 1-2 of the xvalues are NaN
 *		06-MAR-2000	PW: POSIX
 *
 */
 
#include "gmt.h"
#include "x_system.h"

#define CORRFILE "/usr/lib/gmt/xx_corrections.b"
#define MAXLEGS 10000

struct CORR **bin;

size_t binsize = sizeof(struct CORR);
GMT_LONG nlegs = 0;
GMT_LONG nbadlegs=0;

char badlegs[BUFSIZ][10];

GMT_LONG get_id (char *name);
GMT_LONG findleg (char *name);

int main (int argc, char *argv[])
{
	GMT_LONG i, j, id1, id2, time, no[12];
	GMT_LONG nval = 0, ok, n_x, nlegs_in = 0, strike, heading;
	char error, g, m, b, SH_format = FALSE, once = FALSE;
	char corrfile[80], correct = FALSE, inter = FALSE, leg[2][10];
	char internal, legchoice = FALSE, header[REC_SIZE], shift_ok, skip, swap;
	char lega[10], legb[10], verbose = FALSE, line[BUFSIZ];
	double lat, lon , xgrv, xmag, xtop, grv, mag, top, t1, t2;
	double west = 0., east = 360., south = -90., north = 90.;
	FILE *fp = NULL, *fpc = NULL, *fpi = NULL;

	struct XOVERS crossover;

	error = FALSE;
	g = m = b = FALSE;

	/* Check and interpret the command line arguments */

	for (i =1; !error && i < argc; i++) {
		if (argv[i][0] == '-') {
			for (j = 1; argv[i][j]; j++) {
				switch(argv[i][j]) {
					case 'g':		/* Gravity xover is requested */
						no[nval++] = 3;
						g = TRUE;
						break;
					case 'm':		/* Magnetics xover is requested */
						no[nval++] = 4;
						m = TRUE;
						break;
					case 'b':		/* Bathymetry xover is requested */
						no[nval++] = 5;
						b = TRUE;
						break;
					case 'G':		/* Gravity is requested */
						no[nval++] = 6;
						g = TRUE;
						break;
					case 'M':		/* Magnetics is requested */
						no[nval++] = 7;
						m = TRUE;
						break;
					case 'B':		/* Bathymetry is requested */
						no[nval++] = 8;
						b = TRUE;
						break;
					case 'x':		/* Longitude is requested */
						no[nval++] = 1;
						break;
					case 'y':		/* Latitude is requested */
						no[nval++] = 2;
						break;
					case 't':		/* Time (in sec) is requested */
						no[nval++] = 0;
						break;
					case 'h':		/*  Heading is requested */
						no[nval++] = 9;
						break;
					case 'l':		/* Legnames are requested */
						no[nval++] = 10;
						no[nval++] = 11;
						break;
					case 'X':
						fp = fopen (&argv[i][j+1], "rb");
						while (argv[i][j]) j++;
						j--;
						break;
					case 'Z':		/* List of legs to ignore */
						fpi = fopen (&argv[i][j+1], "r");
						while (argv[i][j]) j++;
						j--;
						break;
					case 'R':		/* Region specified */
						sscanf (&argv[i][2], "%lf/%lf/%lf/%lf", &west, &east, &south, &north);
						while (argv[i][j]) j++;
						j--;
						break;
					case 'H':		/* Output using Scientist Helper format */
						SH_format = TRUE;
						break;
					case 'V':
						verbose = TRUE;
						break;
					case 'I':		/* Internal Crossovers only */
						inter = TRUE;
						break;
					case 'C':
						if (argv[i][2] == 0)	/* Use default corrfile */
							strcpy (corrfile, CORRFILE);
						else
							strcpy (corrfile, &argv[i][2]);
						correct = TRUE;
						while (argv[i][j]) j++;
	 					j--;
						break;
					default:		/* Options not recognized */
						error = TRUE;
						break;
				}
			}
		}
		else {
			if (nlegs_in < 2) {
				strcpy (leg[nlegs_in], argv[i]);
				nlegs_in++;
				legchoice = TRUE;
			}
			else
				error = TRUE;
		}
	}

	/* Check that the options selected are mutually consistent */

	if (east < west || south > north) error = TRUE;
	if (west < 0.0) {
		west += 360.0;
		east += 360.0;
	}
	if (nlegs_in != 1 && inter) error = TRUE;
	if (nval > 12) error = TRUE;
	if (nlegs_in == 2 && !strcmp(leg[0],leg[1])) {
		inter = TRUE;
		nlegs_in = 1;
	}
	else if (nlegs_in == 2) {
		inter = FALSE;
		if (strcmp(leg[0], leg[1]) > 0) {	/* Not in alphabetical order */
			strcpy (lega, leg[0]);
			strcpy (leg[0], leg[1]);
			strcpy (leg[1], lega);
		}
	}
	if (argc == 1 || error) {	/* Display usage */
		fprintf(stderr,"xlist - Extract information from xover databases\n\n");
		fprintf(stderr,"usage: xlist [leg1] [leg2] [-<dataflags>] [%s]\n", GMT_Rgeo_OPT);
		fprintf(stderr,"	[-X<xbase>] [-H] [-I] [-V] [-C<corrfile> -Z<ignorefile>]\n\n");
		fprintf(stderr,"	Dataflags:\n");
		fprintf(stderr,"	  l means legnames\n");
		fprintf(stderr,"	  t means list time (delta time for internal COEs)\n");
		fprintf(stderr,"	  x means list longitude\n");
		fprintf(stderr,"	  y means list latitude\n");
		fprintf(stderr,"	  g means list gravity cross-over\n");
		fprintf(stderr,"	  m means list magnetics cross-over\n");
		fprintf(stderr,"	  b means list bathymetry cross-over\n");
		fprintf(stderr,"	  G means list average gravity\n");
		fprintf(stderr,"	  M means list average magnetics\n");
		fprintf(stderr,"	  B means list average bathymetry\n");
		fprintf(stderr,"	  h means list heading\n\n");
		fprintf(stderr,"	-R let only data inside the region pass through. [Default is world]\n");
		fprintf(stderr,"	-H will write 1 header record\n");
		fprintf(stderr,"	-I means return internal cross-overs only [Default is external]\n");
		fprintf(stderr,"	-C<file> applies crossover corrections to the data. If no file name\n");
		fprintf(stderr,"	   is given, the default correction file is assumed\n");
		fprintf(stderr,"	-Z will ignore those legs that appear in the ignorefile\n");
		fprintf(stderr,"	-V means verbose\n");
		fprintf(stderr,"	Default is -txygmbGMBhl\n");
		fprintf(stderr,"	The data is written out in the sequence specified.\n");
		exit (EXIT_FAILURE);
	}

	if (correct) {	/* Read correction table */
		if ((fpc = fopen (corrfile,"rb")) == NULL) {
			fprintf(stderr, "Could not read correction file %s\n", corrfile);
			exit (EXIT_FAILURE);
		}
		bin = (struct CORR **) malloc(sizeof(struct CORR *)*MAXLEGS);
		i = 0;
		bin[i] = (struct CORR *) malloc(binsize);
		while (fread((void *)bin[i], binsize, (size_t)1, fpc) == (size_t)1) {
			i++;
			bin[i] = (struct CORR *) malloc(binsize);
		}
		fclose(fpc);
		nlegs = i;
	}

	/* Read the ignore-legs file if needed */
	if (fpi != NULL) {
		while (fgets (line, BUFSIZ, fpi)) {
			sscanf(line, "%s", badlegs[nbadlegs]);
			nbadlegs++;
		}
		fclose(fpi);
	}


	/* Sort the  order in which the parameters appear */

	if (nval == 0) {		/* Nothing selected, default used */
		g = m = b = TRUE;	/* No data was specified so default is used */
		for (i = 0; i < 12; i++)
			no[i] = i;
		nval = 12;
	}	 

	if (fp == NULL && (fp = fopen ("xx_base.b","rb")) == NULL) {
		fprintf(stderr,"Could not open xx_base.b\n");
		exit (EXIT_FAILURE);
	}

	/* Read first record of file containing n_records */

	ok = fread ((void *)header, (size_t)REC_SIZE, (size_t)1, fp);
	ok = fread ((void *)header, (size_t)REC_SIZE, (size_t)1, fp);


	if (SH_format) {
		/* Write out header record */
		for (i = 0; i < nval; i++) {
			switch(no[i]) {
				case 1:	/* Print out time header */
					printf("Time(s)");
					break;
				case 2:	/* Print out latitude */
					printf("lat");
					break;
				case 3:	/* Print out longitude */
					printf ("lon");
					break;
				case 4:	/* Print out gravity cross-overs */
		 			printf ("xfaa");
		 			break;
				case 5:	/* Print out magnetics cross-overs */
		 			printf ("xmag");
					break;
				case 6:	/* Print out bathymetry cross-overs */
					printf ("xtopo");
					break;
				case 7:	/* Print out gravity */
		 			printf ("faa");
		 			break;
				case 8:	/* Print out magnetics */
		 			printf ("mag");
					break;
				case 9:	/* Print out bathymetry */
					printf ("topo");
					break;
				case 10: /* Print out heading */
					printf ("Azimuth");
					break;
				case 11: /* Print out leg_1 */
					printf ("Leg_1");
					break;
				case 12: /* Print out leg_2 */
					printf ("Leg_2");
					break;
			}
			if ((i+1) < nval)
				printf ("\t");
			else
				printf ("\n");
		}
	}

	/* Start reading data from file */

	once = (inter && legchoice) || (nlegs_in == 2);

	while (ok) {
		sscanf(header, "%s %s %" GMT_LL "d",lega, legb, &n_x);
		internal = !strcmp(lega, legb);
		skip = FALSE;
		if ((inter && !internal) || (!inter && internal))
			skip = TRUE;
		else if (internal)
			skip = strcmp(leg[0], lega);
		else {	/* External */
			if (nlegs_in == 1)
				skip = (strcmp(leg[0], lega) && strcmp(leg[0], legb));
			else if (nlegs_in == 2)
				skip = (strcmp(leg[0], lega) || strcmp(leg[1], legb));
		}
		if (!skip && nbadlegs > 0 && (findleg (lega) || findleg(legb)))	/* Skip bad leg */
			skip = TRUE;
		if (skip)
			fseek (fp, (long int)n_x*REC_SIZE, SEEK_CUR);
		else {
			swap = (!internal && nlegs_in == 1 && !strcmp(lega,legb)) ? TRUE : FALSE;
			id1 = get_id(lega);
			id2 = (internal) ? id1 : get_id(legb);
			shift_ok = (id1 >=0 && id2 >= 0);
			if (verbose) fprintf (stderr, "%s %s nx=%ld\n", lega, legb, n_x);
			for (j = 0; j < n_x; j++) {
				if ((ok = fread ((void *)&crossover, (size_t)REC_SIZE, (size_t)1, fp)) != (size_t)1) {
					fprintf (stderr, "Read error on xx_base.b\n");
					exit (EXIT_FAILURE);
				}
				strike = 0;
				if (!g || (g && crossover.x_val[0] == NODATA)) strike++;
				if (!m || (m && crossover.x_val[1] == NODATA)) strike++;
				if (!b || (b && crossover.x_val[2] == NODATA)) strike++;
				if (strike == 3) continue;
				lat = (double) crossover.lat*0.000001;
				lon = (double) crossover.lon*0.000001;

				/* Check is lat/lon is outside specified area */

				if (lat < south || lat > north) continue;
				while (lon < west) lon += 360.0;
				if (lon > east) continue;
				t1 = crossover.xtime[0] * 0.1;	/* Since xover time was stored as sec * 10 */
				t2 = crossover.xtime[1] * 0.1;
				for (i = 0; i < nval; i++) {
					switch(no[i]) {
						case 0:	/* Print out time */
							if (internal)
								time = (GMT_LONG)fabs(t2-t1);
							else
								time = (GMT_LONG)((swap) ? t2 : t1);
							printf ("%ld", time);
							break;
						case 1:	/* Print out longitude */
							if (lon > 360.0) lon -= 360.0;
							printf ("%.6g", lon);
							break;
						case 2:	/* Print out latitude */
							printf ("%.6g",lat);
							break;
						case 3:	/* Print out gravity cross-over */
							if (crossover.x_val[0] == NODATA)
								printf ("NaN");
							else {
								xgrv = crossover.x_val[0];
								if (shift_ok) xgrv -= bin[id1]->dc_shift_gmt[0] + bin[id1]->drift_rate_gmt[0] * t1 
								- bin[id2]->dc_shift_gmt[0] - bin[id2]->drift_rate_gmt[0] * t2;
								if (swap) xgrv = -xgrv;
		 						printf ("%.2g", xgrv);
		 					}
		 					break;
						case 4:	/* Print out magnetics cross-over */
							if (crossover.x_val[1] == NODATA)
								printf ("NaN");
							else {
								xmag = crossover.x_val[1];
								if (shift_ok) xmag -= bin[id1]->dc_shift_gmt[1] + bin[id1]->drift_rate_gmt[1] * t1
								- bin[id2]->dc_shift_gmt[1] - bin[id2]->drift_rate_gmt[1] * t2;
								if (swap) xmag = -xmag;
		 						printf ("%ld", (GMT_LONG) xmag);
		 					}
							break;
						case 5:	/* Print out bathymetry cross-over */
							if (crossover.x_val[2] == NODATA)
								printf ("NaN");
							else {
								xtop = crossover.x_val[2];
								if (shift_ok) xtop -= bin[id1]->dc_shift_gmt[2] + bin[id1]->drift_rate_gmt[2] * t1
								- bin[id2]->dc_shift_gmt[2] - bin[id2]->drift_rate_gmt[2] * t2;
								if (swap) xtop = -xtop;
								printf ("%ld", (GMT_LONG) xtop);
							}
							break;
						case 6:	/* Print out gravity */
							if (crossover.gmt[0] == NODATA)
								printf ("NaN");
							else {
								grv = crossover.gmt[0];
								if (shift_ok) grv -= bin[id1]->dc_shift_gmt[0] + bin[id1]->drift_rate_gmt[0] * t1 
								- bin[id2]->dc_shift_gmt[0] - bin[id2]->drift_rate_gmt[0] * t2;
		 						printf ("%.2g", grv);
		 					}
		 					break;
						case 7:	/* Print out magnetics */
							if (crossover.gmt[1] == NODATA)
								printf ("NaN");
							else {
								mag = crossover.gmt[1];
								if (shift_ok) mag -= bin[id1]->dc_shift_gmt[1] + bin[id1]->drift_rate_gmt[1] * t1
								- bin[id2]->dc_shift_gmt[1] - bin[id2]->drift_rate_gmt[1] * t2;
		 						printf ("%ld", (GMT_LONG) mag);
		 					}
							break;
						case 8:	/* Print out bathymetry */
							if (crossover.gmt[2] == NODATA)
								printf ("NaN");
							else {
								top = crossover.gmt[2];
								if (shift_ok) top -= bin[id1]->dc_shift_gmt[2] + bin[id1]->drift_rate_gmt[2] * t1
								- bin[id2]->dc_shift_gmt[2] - bin[id2]->drift_rate_gmt[2] * t2;
								printf ("%ld", (GMT_LONG) top);
							}
							break;
						case 9: /* Print out heading */
							heading = (swap) ? crossover.xhead[1] : crossover.xhead[0];
							printf ("%ld", heading);
							break;
						case 10: /* Print out leg_1 */
							(swap) ? printf ("%s", legb) : printf ("%s", lega);
							break;
						case 11: /* Print out leg_1 */
							(swap) ? printf ("%s", lega) : printf ("%s", legb);
							break;
					}
					if ((i+1) < nval)
						printf ("\t");
					else
						printf ("\n");
				}
          		}
          		if (once) {	/* done */
          			fclose (fp);
          			exit (EXIT_FAILURE);
          		}
          	}
		ok = fread ((void *)header, (size_t)REC_SIZE, (size_t)1, fp);
	}
	fclose (fp);

	exit (EXIT_SUCCESS);

}

GMT_LONG get_id (char *name)
{
	GMT_LONG left, right, mid, cmp;

	left = 0;
	right = nlegs-1;
	while (left <= right) {
		mid = (left + right)/2;
		cmp = strcmp(name, bin[mid]->name);
		if (cmp < 0)
			right = mid-1;
		else if (cmp > 0)
			left = mid+1;
		else
			return (mid);
	}
	return (-1);
}

GMT_LONG findleg (char *name)
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
