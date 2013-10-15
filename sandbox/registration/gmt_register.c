/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------
 *
 * gmt_register.c contains code to register the user machine, if user is agreeable.
 *
 * Author:	Paul Wessel
 * Date:	12-OCT-2013
 * Version:	5
 *
 *
/* Functions used to register a new user, via an opt-in question.  We need to do
 * this as NSF wishes us to demonstrate broad usage of GMT, in particular within
 * the US science community.  However, showing a broader appeal could be seen as
 * useful as well.  Users of GMT will help ensure the continued funding and hence
 * development and maintenance of GMT by agreeing to opt-in and provide their IP.
 * We skip users whose machine is on a private network or other situations where
 * an IP cannot be determined.
 */

char *gmt_get_IP (void) {
	/* Try to obtain the local IP */
	char buffer[BUFSIZ] = {""};
	if (gethostname (buffer, BUFSIZ)) return NULL;	/* Could not return an address */
	if (!strncmp ("10.",  buffer, 3U)) return NULL;	/* Private network */
	if (!strncmp ("172.", buffer, 4U)) return NULL;	/* Private network */
	if (!strncmp ("192.", buffer, 4U)) return NULL;	/* Private network */
	return strdup (buffer);
}

void GMT_first_time_registration (struct GMT_CTRL *GMT)
{	/* If first time, create registration file and ask user to opt-in to share it with us */
	char file[BUFSIZ] = {""};
#if defined(__APPLE__)
	char *OS = "OS X";
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	char *OS = "Linux";
#elif defined(__CYGWIN__)
	char *OS = "Cygwin";
#elif defined(_WIN64) || defined(__MINGW64__)
	char *OS = "Windows 64-bit";
#elif defined(_WIN32) || defined(WIN32) || defined(__MINGW32__)
	char *OS = "Windows 32-bit";
#elif defined(__sun)
	char *OS = "Solaris";
#elif defined(__hpux)
	char *OS = "HP-UX";
#else
	char *OS = "Other";
#endif
	if (GMT->session.HOMEDIR == NULL || GMT->session.HOMEDIR[0] == '\0') return;	/* Cannot find HOME, bail */
	sprintf (file, "%s/.gmtregistration", GMT->session.HOMEDIR);	/* User's registration file */
	if (access (file, R_OK)) {	/* Not registered */
		time_t now = time (NULL);
		char *address = gmt_get_IP (), line[BUFSIZ] = {""}, *date = ctime (&now);
		FILE *fp = NULL;
		if (address == NULL) return;			/* Could not determine IP */
		if ((fp = fopen (file, "w")) == NULL) return;	/* Could not write registration file */
		/* Write the registration file; its presence will prevent us from asking this again. */
		GMT_chop (date);	/* Eliminate trailing newline */
		fprintf (fp, "#%s|%s|%s|%s\n", date, address, OS, GMT_PACKAGE_VERSION);
		fclose (fp);
		
		fprintf (stderr, "\n" \
		 	  	 "------------------------------------------------------------------------\n"     \
		 	 	 "|                      VOLUNTARY GMT REGISTRATION                      |\n"     \
		 	  	 "------------------------------------------------------------------------\n"     \
			 	 "[This is a test - no registration is taking place regardless of answers]\n\n"   \
				 "Thank you for installing GMT 5!   In order to develop and maintain GMT and\n"   \
				 "provide timely updates we must demonstrate to the US funding agencies that\n"   \
				 "GMT is widely used, particularly in the US.  It would help us tremendously\n"   \
				 "if you would agree to let us retrieve the following information about your\n"   \
				 "particular GMT installation:\n\n");
		fprintf (stderr, "\tDate:    %s\n",   date);
		fprintf (stderr, "\tAddress: %s\n",   address);
		fprintf (stderr, "\tOS:      %s\n",   OS);
		fprintf (stderr, "\tGMT:     %s\n\n", GMT_PACKAGE_VERSION);
		free (address);
		fprintf (stderr, "The information has been saved to ~/.gmtregistration which will prevent us\n"   \
				 "from asking you these questions again.\n\n"					  \
				 "==> Do you agree to share this information with the GMT team? (y/n) [y]: ");
		fgets (line, BUFSIZ, stdin);
		if (line[0] == 'Y' || line[0] == 'y' || line[0] == '\n') {	/* User opted in */
			fprintf (stderr, "\nThank you for your cooperation.  The file %s\n", file);
			fprintf (stderr, "was sent to the University of Hawaii at Manoa.\n");
			/* Add sending the file here via sockets to a port on gmtserver.soest.hawaii.edu.
			 * The listening program could write the content of this file to gmtreg/<timestamp>.txt */
		}
		else	/* User opted out */
			fprintf (stderr, "\nThank you for considering our request.   Should you change your mind, just\n" \
				 	 "remove the registration file %s.\n", file);
		fprintf (stderr, "\nThe GMT Team\n");
	}
}
