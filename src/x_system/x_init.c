/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *	$Id: x_init.c,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
 *
 * XINIT will create the xx_base.b and xx_legs.b files and write out
 * the first header record (in xx_base.b) that tells which record number
 * to use next time, i.e. 1 in our case.
 *
 * Author:	Paul Wessel
 * Date:	18-JAN-1987
 * Last rev:	07-MAR-2000	POSIX
 *
 */

#include "gmt.h"
#include "x_system.h"

main (int argc, char **argv)
{
	char buffer[REC_SIZE];
	int i;
	FILE *fhandle, *fp;
	
	if (!(argc == 2 && argv[1][1] == 'I')) {
		fprintf (stderr, "xinit - Initialization of new xover databases\n\n");
		fprintf (stderr, "usage: xinit -I\n");
		exit (EXIT_FAILURE);
	}
		
	for (i = 0; i < REC_SIZE; i++) buffer[i] = ' ';
		
	/* Create the xover data base */
	
	if ((fhandle = fopen ("xx_base.b","wb")) == NULL) {
		fprintf (stderr,"xinit : Could not create xx_base.b\n");
		exit (EXIT_FAILURE);
	}
	else
		fprintf(stderr,"xinit : Successfully created xx_base.b\n");
	
	sprintf (buffer,"%10ld xx_base.b header",1L);
	if ((i = fwrite ((void *)buffer, REC_SIZE, 1, fhandle)) != 1) {
		fprintf (stderr,"write error on xx_base.b!");
		exit (EXIT_FAILURE);
	}
	else
		fprintf (stderr,"xinit : Successfully initialized xx_base.b\n");
		
	fclose (fhandle);
	
	/* Create the legs file */
	
	if ((fp = fopen ("xx_legs.b", "w")) == NULL) {
		fprintf (stderr,"xinit : Could not create xx_legs.b\n");
		exit (EXIT_FAILURE);
	}
	else
		fprintf (stderr,"xinit : Successfully created xx_legs.b\n");
	
	fclose (fp);
	
	printf ("xinit: done!\n");

	exit (EXIT_SUCCESS);

}

