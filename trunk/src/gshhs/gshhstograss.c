/*	$Id: gshhstograss.c,v 1.29 2011-05-19 02:51:15 remko Exp $
*
* PROGRAM:   gshhstograss.c
* AUTHOR:    Simon Cox (simon@ned.dem.csiro.au),
*            Paul Wessel (wessel@soest.hawaii.edu),
*			 Markus Metz (markus_metz@gmx.de)
* DATE:	April. 27, 1996
* PURPOSE:     To extract ASCII data from binary shoreline data
*	       as described in the 1996 Wessel & Smith JGR Data Analysis Note
*	       and write files in dig_ascii format for import as GRASS vector maps
* VERSION:	
*		1.2 18-MAY-1999: Explicit binary open for DOS
* 		1.4 05-SEP-2000: Swab done automatically
*		1.5 11-SEP-2004: Updated to work with GSHHS database v1.3
*		1.6 02-MAY-2006: Updated to work with GSHHS database v1.4
*			05-SEP-2007: Removed reliance on getop and made changes
*				    	so it will compile under Windows
* 		1.7 08-APR-2008: 
*			level no longer only 1, see bug fix for gshhs 1.7
*			lat lon swapped for output, was wrong
*			added two category entries in two layers for each line
*			layer 1: cat = level, allows creation of a short table
*					containing level numbers and labels
*			layer 2: cat = unique polygon ID, with level nr, 
*                   in case unique IDs for each line are wanted
*			improved acknowledging of user-defined extends
*
*	This program is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation; version 2 or any later version.
*
*  This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*/

#include "gshhs.h"
#include <string.h>
#include <sys/types.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <time.h>

void help_msg(char *progname);
void usage_msg (char *progname);
void getusername(char *user);

int main (int argc, char **argv)
{
	int i = 1;
	double w, e, s, n, area, lon, lat;
	double minx = -360., maxx = 360., miny = -90., maxy = 90.;
	char source, *progname, *dataname = NULL, user[40], ascii_name[40], att1_name[40], att2_name[40];
	static char *slevel[] = { "unknown" , "land" , "lake" , "island in lake" , "pond in island in lake"};
	int shore_levels = 5;
	FILE	*fp = NULL, *ascii_fp = NULL, *att1_fp = NULL, *att2_fp = NULL;
	int k, max = 270000000, flip, n_read, level, version, greenwich, src, river, shorelines;
	struct POINT p;
	struct GSHHS h;
	int max_id=0;
	time_t tloc;

	progname = argv[0];

	if (argc == 0) {
		usage_msg(progname);
		exit(EXIT_FAILURE);
	}

	while (i < argc) {
		if (argv[i][0] != '-') {
			fprintf (stderr, "%s:  Unrecognized argument %s.\n", progname, argv[i]);
			exit (EXIT_FAILURE);
		}
		switch (argv[i][1]) {
			case 'i':
				dataname = argv[++i];
				break;
			case 'x':
				minx = atof (argv[++i]);
				break;
			case 'X':
				maxx = atof (argv[++i]);
				break;
			case 'y':
				miny = atof (argv[++i]);
				break;
			case 'Y':
				maxy = atof(argv[++i]);
				break;
			case 'h':
				help_msg(progname);
				exit (EXIT_FAILURE);
			default:
				fprintf (stderr, "%s: Bad option %c.\n", progname, argv[i][1]);
				usage_msg(progname);
				exit (EXIT_FAILURE);
		}
		i++;
	}

	if (argc < 3 || !dataname) {
		usage_msg (progname);
		exit(EXIT_FAILURE);
	}
		
	if ((fp = fopen (dataname, "rb")) == NULL ) {
		fprintf (stderr, "%s: Could not find file %s.\n", progname, dataname);
		exit (EXIT_FAILURE);
	}
	if( minx > maxx ){
		fprintf (stderr, "%s: minx %f > maxx %f.\n", progname, minx, maxx);
		exit (EXIT_FAILURE);
	}
	if( miny > maxy ){
		fprintf (stderr, "%s: miny %f > maxy %f.\n", progname, miny, maxy);
		exit (EXIT_FAILURE);
	}

	/* now change the final . in the datafilename to a null i.e. a string terminator */
	*strrchr(dataname,056)= 0;

	shorelines = strstr(dataname,"gshhs") ? 1 : 0;

	strcpy(ascii_name,"grass_");
	strcat(ascii_name,dataname);
	strcat(ascii_name,".ascii");
	if ((ascii_fp = fopen (ascii_name, "w")) == NULL ) {
		fprintf (stderr, "%s: Could not open file %s for writing.\n", progname, ascii_name);
		exit(EXIT_FAILURE);
	}

	if (shorelines) {
		strcpy(att1_name,"grass_");
		strcat(att1_name,dataname);
		strcat(att1_name,"_layer_1.sh");
		if ((att1_fp = fopen (att1_name, "w")) == NULL ) {
			fprintf (stderr, "%s: Could not open file %s for writing.\n", progname, att1_name);
			exit(EXIT_FAILURE);
		}
	}

	strcpy(att2_name,"grass_");
	strcat(att2_name,dataname);
	strcat(att2_name,"_layer_2.sh");
	if ((att2_fp = fopen (att2_name, "w")) == NULL ) {
		fprintf (stderr, "%s: Could not open file %s for writing.\n", progname, att2_name);
		exit(EXIT_FAILURE);
	}

	/* write header for GRASS ASCII vector */
	fprintf(ascii_fp,"ORGANIZATION: \n");
	time(&tloc);
	fprintf(ascii_fp,"DIGIT DATE:   %s",ctime(&tloc));
	getusername(user);
	fprintf(ascii_fp,"DIGIT NAME:   %s\n",user);
	fprintf(ascii_fp,"MAP NAME:     Global Shorelines\n");
	fprintf(ascii_fp,"MAP DATE:     2004\n");
	fprintf(ascii_fp,"MAP SCALE:    1\n");
	fprintf(ascii_fp,"OTHER INFO:   \n");
	fprintf(ascii_fp,"ZONE:	       0\n");
	fprintf(ascii_fp,"WEST EDGE:    %f\n",minx);
	fprintf(ascii_fp,"EAST EDGE:    %f\n",maxx);
	fprintf(ascii_fp,"SOUTH EDGE:   %f\n",miny);
	fprintf(ascii_fp,"NORTH EDGE:   %f\n",maxy);
	fprintf(ascii_fp,"MAP THRESH:   0.0001\n");
	fprintf(ascii_fp,"VERTI:\n");


	/* prepare scripts to import categories */
	fprintf(att2_fp,"#!/bin/sh\n\n");
	fprintf(att2_fp,"# %6d categories, starting at 0\n\n",999999);
	fprintf(att2_fp,"GRASS_VECTOR=\"%s\"\n\n",dataname);

	if (shorelines) {
		fprintf(att1_fp,"#!/bin/sh\n\n");
		fprintf(att1_fp,"GRASS_VECTOR=\"%s\"\n\n",dataname);
		fprintf(att1_fp,"v.db.addtable map=$GRASS_VECTOR table=${GRASS_VECTOR}_layer_1 layer=1 \'columns=cat integer,level varchar(40)\'\n");

		fprintf(att2_fp,"v.db.addtable map=$GRASS_VECTOR table=${GRASS_VECTOR}_layer_2 layer=2 \'columns=cat integer,level_nr integer,level varchar(40)\'\n");
	}
	else {
		fprintf(att2_fp,"v.db.addtable map=$GRASS_VECTOR table=${GRASS_VECTOR}_layer_2 layer=2 \'columns=cat integer,level_nr integer\'\n");
	}

	fprintf(att2_fp,"echo \"\"\n");
	fprintf(att2_fp,"echo \"Inserting level numbers, might take some time...\"\n");

	/* read lines from binary gshhs database */
	n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	version = (h.flag >> 8) & 255;
	flip = (version != GSHHS_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */

	while (n_read == 1) {

		if (flip) {
			h.id = swabi4 ((unsigned int)h.id);
			h.n = swabi4 ((unsigned int)h.n);
			h.west = swabi4 ((unsigned int)h.west);
			h.east = swabi4 ((unsigned int)h.east);
			h.south = swabi4 ((unsigned int)h.south);
			h.north = swabi4 ((unsigned int)h.north);
			h.area = swabi4 ((unsigned int)h.area);
			h.flag = swabi4 ((unsigned int)h.flag);
			h.container  = swabi4 ((unsigned int)h.container);
			h.ancestor  = swabi4 ((unsigned int)h.ancestor);
		}
		level = h.flag & 255;
		version = (h.flag >> 8) & 255;
		greenwich = (h.flag >> 16) & 1;
		src = (h.flag >> 24) & 1;
		river = (h.flag >> 25) & 1;
		w = h.west * GSHHS_SCL;
		e = h.east * GSHHS_SCL;
		s = h.south * GSHHS_SCL;
		n = h.north * GSHHS_SCL;
		source = (src == 1) ? 'W' : 'C';
		area = 0.1 * h.area;

		if( ( w <= maxx && e >= minx ) && ( s <= maxy && n >= miny ) ){
			fprintf(ascii_fp,"L %d 2\n",h.n);
			if( h.id > max_id ) max_id= h.id;

			fprintf(att2_fp,"v.db.update map=$GRASS_VECTOR layer=2 column=level_nr value=\'%d\' where=cat=%d\n",level,h.id);

			for (k = 0; k < h.n; k++) {

				if (fread ((void *)&p, (size_t)sizeof(struct POINT), (size_t)1, fp) != 1) {
					fprintf (stderr, "%s:  Error reading file %s.b.\n", progname, dataname);
					exit(EXIT_FAILURE);
				}
				if (flip) {
					p.x = swabi4 ((unsigned int)p.x);
					p.y = swabi4 ((unsigned int)p.y);
				}
				lon = p.x * GSHHS_SCL;
				if (greenwich && p.x > max) lon -= 360.0;
				lat = p.y * GSHHS_SCL;
				fprintf(ascii_fp," %f %f\n",lon,lat);
			}
			/* add categories in two layers
			  layer 1: cat = level
			  layer 2: cat = h.id
			*/
			fprintf(ascii_fp," 1 %d\n",level);
			fprintf(ascii_fp," 2 %d\n",h.id);
		}
		else {
			fprintf(stderr,"line %d skipped\n", h.id);
			fseek (fp, (long)(h.n * sizeof(struct POINT)), SEEK_CUR);
		}
		max = 180000000;	/* Only Eurasiafrica needs 270 */

		n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
	}
	
	/* don't print level names for borders and rivers */
	if (shorelines) {
		fprintf(att2_fp,"echo \"Inserting level labels, might take some time...\"\n");
		for ( level = 0; level < shore_levels; level++ ) {
			fprintf(att1_fp,"v.db.update map=$GRASS_VECTOR layer=1 column=level value=\'%s\' where=cat=%d\n",slevel[level],level);
			fprintf(att2_fp,"v.db.update map=$GRASS_VECTOR layer=2 column=level value=\'%s\' where=level_nr=%d\n",slevel[level],level);
		}
		fprintf(att2_fp,"echo \"Done.\"\n");

		fclose(att1_fp);
	}
	
	/* now fix up the number of categories */
	fseek (att2_fp, 0L, 0);
	fprintf(att2_fp,"#!/bin/sh\n\n");
	fprintf(att2_fp,"# %6d categories, starting at 0\n\n",max_id + 1);

	fclose(fp);
	fclose(ascii_fp);
	fclose(att2_fp);

	exit (EXIT_SUCCESS);
}

void getusername (char *user)
{
#ifndef WIN32
#include <pwd.h>
	struct passwd *pw = NULL;
	pw = getpwuid (getuid ());
	if (pw) {
		strcpy (user, pw->pw_name);
		return;
	}
#endif
	strcpy (user, "unknown");
	return;
}

void help_msg (char *progname) {

	fprintf (stderr,"gshhs to GRASS ASCII export tool\n\n");
	fprintf (stderr, "  %s reads *.b files of the GSHHS database and\n  writes GRASS compatible ascii vector format files.\n\n", progname);
	fprintf (stderr, "  The resulting GRASS ASCII vector file is called\n  grass_[gshhs|wdb_borders|wdb_rivers]_[f|h|i|l|c].ascii\n\n");
	fprintf (stderr, "  Scripts to import attributes to GRASS are saved as\n  grass_[gshhs|wdb_borders|wdb_rivers]_[f|h|i|l|c]_layer_[1|2].sh\n\n");
	fprintf (stderr, "  Layer 1: cat = level\n  Attribute table has level nr and level label, not produced for\n  borders and rivers.\n");
	fprintf (stderr, "  Layer 2: cat = polygon ID as in GSHHS database\n  Attribute table has polygon id, level nr and, for shorelines, level label.\n\n");
	fprintf (stderr, "  Import the *.ascii file into a GRASS database using v.in.ascii.\n");
	fprintf (stderr, "  If the GRASS vector has a name different from\n  [gshhs|wdb_borders|wdb_rivers]_[f|h|i|l|c].b,\n  change the variable GRASS_VECTOR at the beginning of the scripts.\n");
	fprintf (stderr, "  Set permission to execute the scripts, run script for desired layer.\n\n");
	
}

void usage_msg (char *progname) {

	fprintf (stderr,"gshhs to GRASS ASCII export tool\n");
	fprintf (stderr, "usage:  %s [-h] -i <input>.b [-x minx] [-X maxx] [-y miny] [-Y maxy]\n\n", progname);
	fprintf (stderr, "Arguments to %s:\n", progname);
	fprintf (stderr, " -h            print description and help\n");
	fprintf (stderr, " -i <input>.b  input file from GSHHS database\n");
	fprintf (stderr, " -x minx       western limit in decimal degrees\n");
	fprintf (stderr, " -X maxx       eastern limit in decimal degrees\n");
	fprintf (stderr, " -y miny       southern limit in decimal degrees\n");
	fprintf (stderr, " -Y maxy       northern limit in decimal degrees\n");
	fprintf (stderr, "\n");
	
}
