/*	$Id: gshhstograss.c,v 1.5 2005-03-04 00:48:31 pwessel Exp $
 *
 * PROGRAM:     gshhstograss.c
 * AUTHOR:      Simon Cox (simon@ned.dem.csiro.au) &
		Paul Wessel (wessel@soest.hawaii.edu)
 * DATE:	April. 27, 1996
 * PURPOSE:     To extract ASCII data from binary shoreline data
 *	        as described in the 1996 Wessel & Smith JGR Data Analysis Note
 *	        and write files in dig_ascii format for import as GRASS vector maps
 * VERSION:	1.2 18-MAY-1999: Explicit binary open for DOS
 * VERSION:	1.4 05-SEP-2000: Swab done automatically
 *		1.5 11-SEP-2004: Updated to work with GSHHS database v1.3
 */

#include "gshhs.h"
#include <unistd.h>
#include <string.h>
#include <time.h>

int main (argc, argv)
int     argc;
char **argv;

#define ARGS  "i:x:X:y:Y:"

{
	double w, e, s, n, area, lon, lat;
	double minx = -360., maxx = 360., miny = -90., maxy = 90.;
	char buf[64], source, *progname, *dataname, dig_name[24], att_name[24], cats_name[24];
	static char *slevel[] = { "null" , "land" , "lake" , "island_in_lake" , "pond_in_island_in_lake" };
	FILE    *fp,*dig_ascii,*dig_att,*dig_cats;
	int     k, max = 270000000, flip, n_read;
	struct  POINT p;
	struct GSHHS h;
	int c, max_id=0;
	time_t tloc;

	optarg = NULL;
	progname = argv[0];

	if (argc < 3) {
		fprintf (stderr, "%s reads gshhs_[f|h|i|l|c].b files\n\tand writes GRASS compatible ascii vector format files.\n", progname);
		fprintf (stderr, "The resulting files are called dig_[ascii|att|cats].gshhs_[f|h|i|l|c]\n");
		fprintf (stderr, "copy these into grassmapset/dig_[ascii|att|cats]/gshhs_[f|h|i|l|c]\n");
		fprintf (stderr, "and then import them into the GRASS database using v.in.ascii and v.support.\n");
		fprintf (stderr, "usage:  %s -i gshhs_[f|h|i|l|c].b [-x minx] [-X maxx] [-y miny] [-Y maxy]\n", progname);
		exit(-1);
	}

	while ((c = getopt(argc, argv, ARGS)) != -1)
	    switch (c) {
		case 'i':
			dataname= optarg;
			break;
		case 'x':
			minx= atof(optarg);
			break;
		case 'X':
			maxx= atof(optarg);
			break;
		case 'y':
			miny= atof(optarg);
			break;
		case 'Y':
			maxy= atof(optarg);
			break;
		case '?':
		    fprintf (stderr, "%s:  Bad option %c.\n", progname, c);
		    exit(-2);
	    }
	if ((fp = fopen (dataname, "rb")) == NULL ) {
		fprintf (stderr, "%s:  Could not find file %s.\n", progname, dataname);
		exit(-1);
	}
	if( minx > maxx ){
		fprintf (stderr, "%s:  minx %f > maxx %f.\n", progname, minx, maxx);
		exit(-2);
	}
	if( miny > maxy ){
		fprintf (stderr, "%s:  miny %f > maxy %f.\n", progname, miny, maxy);
		exit(-2);
	}

/* now change the final . in the datafilename to a null ie a string terminator */
	*strrchr(dataname,056)= 0;

	strcpy(dig_name,"dig_ascii.");  strcat(dig_name,dataname);
	if ((dig_ascii = fopen (dig_name, "w")) == NULL ) {
		fprintf (stderr, "%s:  Could not open file %s for writing.\n", progname, dig_name);
		exit(-1);
	}
	strcpy(att_name,"dig_att.");    strcat(att_name,dataname);
	if ((dig_att = fopen (att_name, "w")) == NULL ) {
		fprintf (stderr, "%s:  Could not open file %s for writing.\n", progname, att_name);
		exit(-1);
	}
	strcpy(cats_name,"dig_cats.");  strcat(cats_name,dataname);
	if ((dig_cats = fopen (cats_name, "w")) == NULL ) {
		fprintf (stderr, "%s:  Could not open file %s for writing.\n", progname, cats_name);
		exit(-1);
	}

	fprintf(dig_ascii,"ORGANIZATION: \n");
	time(&tloc);
	fprintf(dig_ascii,"DIGIT DATE:   %s",ctime(&tloc));
	fprintf(dig_ascii,"DIGIT NAME:   %s\n",cuserid(buf));
	fprintf(dig_ascii,"MAP NAME:     Global Shorelines\n");
	fprintf(dig_ascii,"MAP DATE:     2004\n");
	fprintf(dig_ascii,"MAP SCALE:    1\n");
	fprintf(dig_ascii,"OTHER INFO:   \n");
	fprintf(dig_ascii,"ZONE:	 0\n");
	fprintf(dig_ascii,"WEST EDGE:    %f\n",minx);
	fprintf(dig_ascii,"EAST EDGE:    %f\n",maxx);
	fprintf(dig_ascii,"SOUTH EDGE:   %f\n",miny);
	fprintf(dig_ascii,"NORTH EDGE:   %f\n",maxy);
	fprintf(dig_ascii,"MAP THRESH:   0.0001\n");
	fprintf(dig_ascii,"VERTI:\n");


	fprintf(dig_cats,"# %6d categories\n",999999);
	fprintf(dig_cats,"Global Shorelines\n");
	fprintf(dig_cats,"\n");
	fprintf(dig_cats,"0.0 0.0 0.0 0.0\n");
	fprintf(dig_cats,"0:unknown\n");

	n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	flip = (! (h.level > 0 && h.level < 5));

	while (n_read == 1) {

		if (flip) {
			h.id = swabi4 ((unsigned int)h.id);
			h.n  = swabi4 ((unsigned int)h.n);
			h.level = swabi4 ((unsigned int)h.level);
			h.west  = swabi4 ((unsigned int)h.west);
			h.east  = swabi4 ((unsigned int)h.east);
			h.south = swabi4 ((unsigned int)h.south);
			h.north = swabi4 ((unsigned int)h.north);
			h.area  = swabi4 ((unsigned int)h.area);
			h.version   = swabi4 ((unsigned int)h.version);
			h.greenwich = swabi2 ((unsigned int)h.greenwich);
			h.source    = swabi2 ((unsigned int)h.source);
		}
		w = h.west  * 1.0e-6;
		e = h.east  * 1.0e-6;
		s = h.south * 1.0e-6;
		n = h.north * 1.0e-6;
		source = (h.source == 1) ? 'W' : 'C';
		area = 0.1 * h.area;

		if( ( w < maxx && e > minx ) && ( s < maxy && n > miny ) ){
			fprintf(dig_ascii,"L %d\n",h.n);
			if( h.id > max_id )     max_id= h.id;
			fprintf(dig_cats,"%d:%s\n",h.id,slevel[h.level]);
		}

		for (k = 0; k < h.n; k++) {

			if (fread ((void *)&p, (size_t)sizeof(struct POINT), (size_t)1, fp) != 1) {
				fprintf (stderr, "gshhs:  Error reading file.\n");
				exit(-1);
			}
			if (flip) {
				p.x = swabi4 ((unsigned int)p.x);
				p.y = swabi4 ((unsigned int)p.y);
			}
			lon = (h.greenwich && p.x > max) ? p.x * 1.0e-6 -360.0 : p.x * 1.0e-6;
			lat = p.y * 1.0e-6;
			if( ( w < maxx && e > minx ) && ( s < maxy && n > miny ) ){
				if(k==1)	fprintf(dig_att,"L  %15f %15f  %9d	  \n",lon,lat,h.id);
				fprintf(dig_ascii," %f %f\n",lat,lon);
			}
		}
		max = 180000000;	/* Only Eurasiafrica needs 270 */
 
		n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	}

	fclose(fp);
	fclose(dig_ascii);
	fclose(dig_att);
	/* now fix up the number of categories */
	fseek (dig_cats, 0L, 0);
	fprintf(dig_cats,"# %6d categories\n", max_id);
	fclose(dig_cats);

	exit (EXIT_SUCCSESS);
}
