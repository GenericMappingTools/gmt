/*	$Id: x_update.c,v 1.13 2011-03-05 20:46:26 guru Exp $
 *
 * XUPDATE will read a xover.d-file that contains a series of crossovers. The first
 * record contains leg1 year1 leg2 year2, and the next n records has all the
 * crossover info for this pair of legs. At the end of this sequence, the
 * next head record has the names of the next pair etc.
 * XUPDATE maintains two files:
 *  1) xx_base.b    - File containing all the individual crossovers for 
 *		      all legs. (The file is unformatted binary)
 *  2) xx_legs.b    - Lookup table. Each leg has one record that contain 
 *		      this leg's statistics, agency, startyear, etc.
 *		      (Binary unformatted)
 *  New information is appended to xx_base.b, while xx_legs.b is kept in
 *  alphabetical order. The xover.d-file is the output from the program XOVER.
 *
 * Programmer:	Paul Wessel
 * Date:	29-OCT-1987
 * Revised:	18-FEB-1989
 *		8-JUL-1989	PW: Fixed bug in loop over g, m, and t
 *		16-FEB-1990	PW: Changed fscanf to fgets
 *		06-MAR-2000	PW: POSIX
 *
 */

#include "gmt.h"

#include "x_system.h"

#define MAX_NR 40000	/* Max number of crossover for one leg */
#define S_RDONLY 0000444

/* Global variables */

void append_leg (struct LEG *leg, struct LEG*leg_head);
GMT_LONG read_xx_legs (FILE *fp, struct LEG **L);
struct LEG *make_leg (char *text);
struct LEG *find_leg (char *text, struct LEG*leg_head);

int main (int argc, char *argv[])
{

	static char *xformat = "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf";

	struct XOVERS crossover[MAX_NR];

	struct LEG *leg_head = NULL;

	FILE *fxb = NULL;		/* File handle for xx_base.b */
	FILE *fxl = NULL;		/* File handle for xx_legs.b file */


	char lega[10], legb[10];	/* two legnames */
	char header[REC_SIZE];		/* header buffer for xx_base.b */
	char buffer[BUFSIZ];		/* Input data buffer */
	char *nread;
	GMT_LONG n_legs;			/* number of legs */
	GMT_LONG n_rec;			/* rec counters */
	GMT_LONG n_x;			/* number of crossovers in a file */
	GMT_LONG n_x_gmt[3];			/* number of g/m/t crossovers in a file */
	GMT_LONG year1, year2;		/* Start year for each leg */
	GMT_LONG i, j, nxcheck = 0;
  
	double sum_gmt[3], sum2_gmt[3], tmpsum_1, tmpsum_2, tmpsum2_1, tmpsum2_2;
	double xlat, xlon;		/* Crossover coordinates in decimal degree */
	double xtime1, xtime2;		/* Time at crossover point along each track */
	double xgmt[3];			/* gmt crossover value */
	double gmt[3];			/* GMT at crossover point */
	double xhead1, xhead2;		/* Heading at crossover along each track */
  

	FILE *fp;			/* pointer to crossover file */

	GMT_LONG internal;		/* TRUE if leg1 == leg2 */
	GMT_LONG error = FALSE, ok, verbose = FALSE, warning = FALSE;

	struct LEG *leg1, *leg2 = VNULL;
  
	fp = NULL;
	for (i = 1; !error && i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'X':
					fxb = fopen (&argv[i][2], "rb+");
					break;
				case 'L':
					fxl = fopen(&argv[i][2], "rb");
					break;
				case 'V':
					verbose = TRUE;
					break;
				case 'W':
					warning = TRUE;
					nxcheck = atoi (&argv[i][2]);
					break;
				default:
					error = TRUE;
					break;
			}
		}
		else if ((fp = fopen(argv[i],"r")) == NULL) {
			fprintf(stderr,"x_update : Could not find/open %s\n",argv[i]);
			exit (EXIT_FAILURE);
		}
	}
	if (fp == NULL) error = TRUE;
	if (argc == 1 || error) {
		fprintf(stderr,"x_update - add crossover info to xover database\n");
		fprintf(stderr,"Usage : x_update xoverfile [-X<xbase> -L<legbase> -V -W<nxovers>]\n");
		fprintf(stderr,"	-X to specify alternative xx_base.b file\n");
		fprintf(stderr,"	-L to specify alternative xx_legs.b file\n");
		fprintf(stderr,"	-V means verbose\n");
		fprintf(stderr,"	-W prints a message if two legs generate more than nxovers\n");
		exit (EXIT_FAILURE);
	}
  
	/* First open (if possible) the 2 key files */

	if (fxb == NULL && (fxb = fopen("xx_base.b","rb+")) == NULL) {
		fprintf(stderr,"x_update : Could not find/open xx_base.b\n");
		exit (EXIT_FAILURE);
	}

	if (fxl == NULL && (fxl = fopen("xx_legs.b","rb")) == NULL) {
		fprintf(stderr,"x_update : Could not find/open xx_legs.b\n");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < REC_SIZE; i++) header[i] = ' ';

	n_legs = read_xx_legs(fxl, &leg_head);	/* Initiate list of leg structures */

	fseek (fxb, 0L, SEEK_SET);		/* Go to start of file */
	if (fread ((void *)header, (size_t)REC_SIZE, (size_t)1, fxb) != 1) {	/* Get next recno to write */
		fprintf(stderr,"x_update : Read error on xx_base.b\n");
		exit (EXIT_FAILURE);
	}
	sscanf (header,"%" GMT_LL "d", &n_rec);
	fseek (fxb, 0L, SEEK_END);		/* Go to end of file */
	for (i = 0; i < REC_SIZE; i++) header[i] = ' ';

	/* Start reading xover info to add to database */
	leg1 = NULL;
	nread = fgets (buffer, BUFSIZ, fp);
	while (nread) {
		sscanf (buffer,"%s %" GMT_LL "d %s %" GMT_LL "d",lega, &year1, legb, &year2);
		internal = (!strcmp(lega,legb)) ? TRUE : FALSE;

		if (leg1 != NULL && strcmp(leg1->name,lega) == 0)  {
			/* Use the same leg */
		}
		else if ((leg1 = find_leg(lega, leg_head)) == NULL) {
			leg1 = make_leg(lega);
			leg1->year = (int)year1;
			append_leg(leg1, leg_head);
		}
      
		if (!internal && (leg2 = find_leg(legb, leg_head)) == NULL) {
			leg2 = make_leg(legb);
			leg2->year = (int)year2;
			append_leg(leg2, leg_head);
		}

		/* Then, read xover info from file for this pair */

		n_x_gmt[0] = n_x_gmt[1] = n_x_gmt[2] = n_x = 0;
		sum_gmt[0] = sum_gmt[1] = sum_gmt[2] = 0.0;
		sum2_gmt[0] = sum2_gmt[1] = sum2_gmt[2] = 0.0;
		nread = fgets (buffer, BUFSIZ, fp);
		ok = TRUE;
		while (nread && ok) {
			sscanf(buffer,xformat, &xlat, &xlon, &xtime1, &xtime2, &xgmt[0],
				&xgmt[1], &xgmt[2], &gmt[0], &gmt[1], &gmt[2], &xhead1, &xhead2);
			crossover[n_x].lat = irint (xlat * 1.0e6);
			crossover[n_x].lon = irint (xlon * 1.0e6);
			crossover[n_x].xtime[0] = irint (xtime1 * 10.0);
			crossover[n_x].xtime[1] = irint (xtime2 * 10.0);
			for (j = 0; j < 3; j++) {
				crossover[n_x].x_val[j] = (float)xgmt[j];
				crossover[n_x].gmt[j] = (gmt[j] == NODATA) ? NODATA : (short) rint (gmt[j]);
			}
			crossover[n_x].xhead[0] = (short) rint (xhead1);
			crossover[n_x].xhead[1] = (short) rint (xhead2);
			for (j = 0; j < 3; j++) {
				if (xgmt[j] == NODATA) continue;
				sum_gmt[j] += xgmt[j];
				sum2_gmt[j] += xgmt[j] * xgmt[j];
				n_x_gmt[j]++;
			}
			n_x++;
			if (n_x == MAX_NR) {
				fprintf(stderr, "x_update: Out of memory for crossover[] structures! recompile\n");
				exit (EXIT_FAILURE);
			}
			nread = fgets (buffer, BUFSIZ, fp);
			ok = (nread == NULL) ? FALSE : (strlen(buffer) > 30);
		}

		/* Copy this info to xx_base.b */

		sprintf (header,"%s %s %10ld", lega, legb, n_x);
		if (fwrite ((void *)header, (size_t)REC_SIZE, (size_t)1, fxb) != (size_t)1) {
			fprintf(stderr,"x_update : Write header error at rec %ld\n", n_rec);
			exit (EXIT_FAILURE);
		}
		if (fwrite((void *)crossover, (size_t)REC_SIZE, (size_t)n_x, fxb) != (size_t)n_x) {
			fprintf(stderr,"x_update : Write data error at rec %ld\n", n_rec);
			exit (EXIT_FAILURE);
		}
      
		/* Update leg-structure(s) */

		if (internal) {
			for (j = 0; j < 3; j++) {
				leg1->mean_gmtint[j] = (n_x_gmt[j]) ? sum_gmt[j]/n_x_gmt[j] : 0.0;
				leg1->st_dev_gmtint[j] = (n_x_gmt[j] > 1) ?
					sqrt((double) ((sum2_gmt[j] - sum_gmt[j]*leg1->mean_gmtint[j])/(n_x_gmt[j]-1))) : 
					0.0;
				leg1->n_gmtint[j] = (int)n_x_gmt[j];
			}
			leg1->n_x_int = (int)n_x;
		}
		else {	/* External */
			for (j = 0; j < 3; j++) {

				/* First, get old sum and sum-of-squares for both legs */
				tmpsum_1 = leg1->mean_gmtext[j] * leg1->n_gmtext[j];
				tmpsum2_1 = (leg1->n_gmtext[j]-1)*leg1->st_dev_gmtext[j]*leg1->st_dev_gmtext[j] +
					leg1->mean_gmtext[j] * tmpsum_1;
				tmpsum_2 = leg2->mean_gmtext[j] * leg2->n_gmtext[j];
				tmpsum2_2 = (leg2->n_gmtext[j]-1)*leg2->st_dev_gmtext[j]*leg2->st_dev_gmtext[j] +
					leg2->mean_gmtext[j] * tmpsum_2;

				/* Then, recompute nxs, means, and st.devs */

				leg1->n_gmtext[j] += (int)n_x_gmt[j];
				leg2->n_gmtext[j] += (int)n_x_gmt[j];
				leg1->mean_gmtext[j] = (leg1->n_gmtext[j]) ?
					(tmpsum_1 + sum_gmt[j])/leg1->n_gmtext[j] : 0.0;
				leg2->mean_gmtext[j] = (leg2->n_gmtext[j]) ?
					(tmpsum_2 - sum_gmt[j])/leg2->n_gmtext[j] : 0.0;
				tmpsum_1 = leg1->mean_gmtext[j] * leg1->n_gmtext[j];
				tmpsum_2 = leg2->mean_gmtext[j] * leg2->n_gmtext[j];
				leg1->st_dev_gmtext[j]  = (leg1->n_gmtext[j] > 1) ?
					sqrt((double) ((tmpsum2_1 + sum2_gmt[j] -
					tmpsum_1*leg1->mean_gmtext[j])/(leg1->n_gmtext[j] - 1))) : 0.;
				leg2->st_dev_gmtext[j]  = (leg2->n_gmtext[j] > 1) ?
					sqrt((double) ((tmpsum2_2 + sum2_gmt[j] -
					tmpsum_2*leg2->mean_gmtext[j])/(leg2->n_gmtext[j] - 1))) : 0.;
			}
		}

		n_rec += n_x + 1;
		if (verbose) fprintf(stderr,"x_update: Updated legs %s and %s\n",lega,legb);
		if (warning && n_x > nxcheck) printf ("x_update: %s %s generated %ld crossovers\n", lega, legb, n_x);
	}
	fclose (fp);
  
	/* Here all the info has been read and processed. Now we must create a new xx_legs.b file */

	/* Write first rec for next time x_update is used */

	fseek (fxb, 0L, SEEK_SET);
	sprintf(header,"%10ld xx_base.b header",n_rec);
	if (fwrite((void *)header, (size_t)REC_SIZE, (size_t)1, fxb) != (size_t)1) {
		fprintf(stderr,"x_update : Write header error for n_rec = %ld\n",n_rec);
		exit (EXIT_FAILURE);
	}
	fclose(fxb);

	/* Close the legbase file, rename it and make a new one */

	fclose(fxl);
	if (rename("xx_legs.b","xx_legs_old.b")) {
		fprintf(stderr,"x_update: Could not rename -> xx_legs.b to xx_legs_old.b\n");
		exit (EXIT_FAILURE);
	}
	fxl = fopen("xx_legs.b","wb");

	for (leg1 = leg_head->next_leg; leg1; leg1 = leg1->next_leg) {
		if (fwrite((void *)leg1, sizeof(struct LEG), (size_t)1, fxl) != (size_t)1) {
			fprintf(stderr, "x_update: Write error for leg %s\n",leg1->name);
			exit (EXIT_FAILURE);
		}
	}
	fclose(fxl);
	exit (EXIT_SUCCESS);
}

void append_leg (struct LEG *leg, struct LEG*leg_head)
{
	struct LEG *current;
	/* First, search for position in list */
	for (current = leg_head; current->next_leg && 
		strcmp(current->next_leg->name,leg->name) < 0;
		current = current->next_leg);
	/* Then append */
	leg->next_leg = current->next_leg;
	current->next_leg = leg;
}
    
GMT_LONG read_xx_legs (FILE *fxl, struct LEG **L) {
	GMT_LONG nlegs = 0;
	struct LEG *new_leg, *last_leg, *leg_head;
	leg_head = make_leg("-----");
	new_leg = make_leg("-----");
	last_leg = leg_head;
	while (fread((void *)new_leg, sizeof(struct LEG), (size_t)1, fxl) == 1) {
		nlegs++;
		last_leg->next_leg = new_leg;
		last_leg = last_leg->next_leg;
		new_leg = make_leg("-----");
	}
	free((char *) new_leg);
	*L = leg_head;
	return (nlegs);
}

struct LEG *make_leg (char *text)
{
	GMT_LONG j;
	struct LEG *new_leg;
	if ((new_leg = (struct LEG *) malloc(sizeof(struct LEG))) == NULL) {
		fprintf(stderr,"x_update : Could not allocate memory for leg %s\n",text);
		exit (EXIT_FAILURE);
	}
	strcpy(new_leg->name,text);
	strcpy(new_leg->agency,"--------");
	new_leg->year = 0;
	new_leg->n_x_int = 0;
	new_leg->n_x_ext = 0;
	for (j = 0; j < 3; j++) {
		new_leg->n_gmtint[j] = 0;
		new_leg->n_gmtext[j] = 0;
		new_leg->mean_gmtint[j] = 0.;
		new_leg->mean_gmtext[j] = 0.;
		new_leg->st_dev_gmtint[j] = 0.;
		new_leg->st_dev_gmtext[j] = 0.;
		new_leg->dc_shift_gmt[j] = 0.;
		new_leg->drift_rate_gmt[j] = 0.;
	}

	return (new_leg);
}

struct LEG *find_leg (char *text, struct LEG*leg_head)
{
	struct LEG *leg;
	GMT_LONG test1 = -1;
	for (leg = leg_head->next_leg; leg && (test1 = strcmp(leg->name,text)) < 0; leg = leg->next_leg);
	if (test1 == 0)
		return (leg);
	else
		return (NULL);
}

