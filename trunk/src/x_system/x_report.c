/*	$Id: x_report.c,v 1.3 2005-03-04 00:48:32 pwessel Exp $
 *
 * XREPORT reads the xx_legs.b file and reports statistics for one or
 * all or the legs. The information reported are legname, year-of-cruise,
 * agency, and # cross-overs, mean, and st.dev for the datatypes selected.
 * If no data types are selected, all of them are reported.
 *
 * Author:	Paul Wessel
 * Date:	10-NOV-1987
 * Last rev:	06-MAR-2000	POSIX
 *
 */
 
#include "gmt.h"
#include "x_system.h"

int main (int argc, char *argv[])
{
	int end = FALSE, i, all_legs = TRUE, error = FALSE;
	size_t legsize = sizeof(struct LEG);
	char leg[10], gmt[3], datatype[3];
	FILE *fp = NULL;
	struct LEG thisleg;
	
	gmt[0] = gmt[1] = gmt[2] = FALSE;
	datatype[0] = 'G';
	datatype[1] = 'M';
	datatype[2] = 'T';
	
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'X':
					fp = fopen(&argv[i][2], "rb");
					break;
				case 'G':
					gmt[0] = TRUE;
					break;
				case 'M':
					gmt[1] = TRUE;
					break;
				case 'T':
					gmt[2] = TRUE;
					break;
				default:
					error = TRUE;
					break;
			}
		}
		else {
			strcpy(leg, argv[i]);
			all_legs = FALSE;
		}
	}
	if (error) {
		fprintf(stderr,"xreport - Print report of crossover information\n\n");
		fprintf(stderr,"Usage : xreport [<leg>] [-X<xlegsfile>] [-G] [-M] [-T]\n");
		fprintf(stderr,"	<leg> is the desired legs id. [Default is all legs]\n");
		fprintf(stderr,"	-X to specify alternate xx_legs.b file\n");
		fprintf(stderr,"	-G -M -T to report on gravity/magnetics/topography [Default is G, M, and T]\n");
		exit (EXIT_FAILURE);
	}
	if (fp == NULL && (fp = fopen ("xx_legs.b", "rb")) == NULL) {
		fprintf (stderr, "Could not find file xx_legs.b\n");
		exit (EXIT_FAILURE);
	}
  
  	/* If none selected, the default is all 3 data types */
  	
  	if (gmt[0] == FALSE && gmt[1] == FALSE && gmt[2] == FALSE) gmt[0] = gmt[1] = gmt[2] = TRUE;
  	
  	printf ("leg\tyear\tagency\tdata\tnx_I\tmean_I\tstdev_I\tnx_E\tmean_E\tstdev_E\n");
	while (!end && fread((void *)&thisleg, legsize, 1, fp) == 1) {
		if (all_legs || !strcmp(thisleg.name, leg)) {
			for (i = 0; i < 3; i++) {
				if (!gmt[i]) continue;
				printf("%s\t%d\t%s\t",
					thisleg.name,
					thisleg.year,
					thisleg.agency);
				printf("%c\t%d\t%8.2lf\t%7.2lf\t%6d\t%8.2lf\t%7.2lf\n",
					datatype[i],
					thisleg.n_gmtint[i],
					thisleg.mean_gmtint[i],
					thisleg.st_dev_gmtint[i],
					thisleg.n_gmtext[i],
					thisleg.mean_gmtext[i],
					thisleg.st_dev_gmtext[i]);
			}
			if (!all_legs) end = TRUE;
		}
	}
	fclose (fp);
	exit (EXIT_SUCCESS);
}

