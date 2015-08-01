/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *--------------------------------------------------------------------*/
/*
 * Public function prototypes for GMT API option parsing.
 *
 * Author: 	Paul Wessel
 * Date:	1-JUN-2010
 * Version:	5
 *
 * The API presently consists of 52 documented functions.  For a full
 * description of the API, see the GMT_API documentation.
 * These functions have Fortran bindings as well, provided you add
 * -DFORTRAN_API to the C preprocessor flags [in ConfigUser.cmake].
 *
 * Here lie the 12 public functions used for GMT API command parsing:
 *
 * GMT_Create_Options	: Convert an array of text args to a linked option list
 * GMT_Destroy_Options	: Delete the linked option list
 * GMT_Create_Args	: Convert a struct option list back to an array of text args
 * GMT_Destroy_Args	: Delete the array of text args
 * GMT_Create_Cmd	: Convert a struct option list to a single command text. 
 * GMT_Destroy_Cmd	: Delete the command string
 * GMT_Make_Option	: Create a single option structure given arguments
 * GMT_Find_Option	: Find a specified option in the linked option list
 * GMT_Update_Option	: Update the arguments of the specified option in the list
 * GMT_Append_Option	: Append the given option to the end of the structure list
 * GMT_Delete_Option	: Delete the specified option and adjust the linked list
 * GMT_Parse_Common	: Parse the common GMT options
 *
 * This part of the API helps the developer create, manipulate, modify, find, and
 * update options that will be passed to various GMT_* modules ("The GMT programs").
 * This involves converting from text arrays (e.g., argc, argv[]) to the linked list
 * of structures used by these functions, manipulate these lists, and perhaps even
 * creating text arrays from the linked list.  All these functions are pass the
 * API pointer and if that is NULL then errors will be issued.
 */
 
#include "gmt_dev.h"

#define ptr_return(API,err,ptr) { GMTAPI_report_error(API,err); return (ptr);}
#define return_null(API,err) { GMTAPI_report_error(API,err); return (NULL);}
#define return_error(API,err) { GMTAPI_report_error(API,err); return (err);}
#define return_value(API,err,val) { GMTAPI_report_error(API,err); return (val);}

static inline struct GMTAPI_CTRL * gmt_get_api_ptr (struct GMTAPI_CTRL *ptr) {return (ptr);}

#ifdef DEBUG
int GMT_List_Args (void *V_API, struct GMT_OPTION *head)
{	/* This function dumps the options to stderr for debug purposes.
	 * It is not meant to be part of the API but to assist developers
	 * during the debug phase of development.
	 */

	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);

	if (head == NULL) return_error (API, GMT_OPTION_LIST_NULL);

	fprintf (stderr, "Options:");
	for (opt = head; opt; opt = opt->next) {
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt != head) fprintf (stderr, " ");
		if (opt->option == GMT_OPT_SYNOPSIS)		/* Produce special - command for synopsis */
			fprintf (stderr, "-");
		else if (opt->option == GMT_OPT_INFILE)		/* Option for input filename [or number] */
			fprintf (stderr, "%s", opt->arg);
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline argument */
			fprintf (stderr, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument */
			fprintf (stderr, "-%c", opt->option);

	}
	fprintf (stderr, "\n");

	return (GMT_OK);	/* No error encountered */
}
#endif

struct GMT_OPTION * GMT_Create_Options (void *V_API, int n_args_in, void *in)
{
	/* This function will loop over the n_args_in supplied command line arguments (in) and
	 * returns a linked list of GMT_OPTION structures for each program option.
	 * These will in turn will be processed by the program-specific option parsers.
	 * What actually happens is controlled by n_args_in.  There are these cases:
	 * n_args_in < 0 : This means that in is already a linked list and we just return it.
	 * n_args_in == 0: in is a single text string with multiple options (e.g., "-R0/2/0/5 -Jx1 -O -m > file")
	 *		   and we must first break this command string into separate words.
	 * n_args_in > 0 : in is an array of text strings (e.g., argv[]).
	 *
	 * Note: 1. If argv[0] is the calling program name, make sure to pass argc-1, args+1 instead.
	 *	 2. in == NULL is allowed only for n_args_in <= 0 (an empty list of options).
	 */

	int error = GMT_OK;
	unsigned int arg, first_char, n_args;
	char option, **args = NULL, **new_args = NULL, *pch = NULL;
	struct GMT_OPTION *head = NULL, *new_opt = NULL;
	struct GMT_CTRL *G = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (in == NULL && n_args_in) return_null (V_API, GMT_ARGV_LIST_NULL);	/* Gave no argument pointer but said we had at least 1 */
	if (in == NULL) return (NULL);	/* Gave no argument pointer so a null struct is returned */
	if (n_args_in < 0) return (in);	/* Already converted to linked list */
	API = gmt_get_api_ptr (V_API);	/* Convert API to a GMTAPI_CTRL pointer */
	G = API->GMT;	/* GMT control structure */
	if (n_args_in == 0) {	/* Check if a single command line, if so break into tokens */
		unsigned int pos = 0, new_n_args = 0, k;
		bool quoted;
		size_t n_alloc = GMT_SMALL_CHUNK;
		char p[GMT_BUFSIZ] = {""}, *txt_in = in;	/* Passed a single text string */
		new_args = GMT_memory (G, NULL, n_alloc, char *);
		/* txt_in can contain options that take multi-word text strings, e.g., -B+t"My title".  We avoid the problem of splitting
		 * these items by temporarily replacing spaces inside quoted strings with ASCII 31 US (Unit Separator), do the strtok on
		 * space, and then replace all ASCII 31 with space at the end (we do the same for tab using ASCII 29 GS (group separator) */
		for (k = 0, quoted = false; txt_in[k]; k++) {
			if (txt_in[k] == '\"') quoted = !quoted;	/* Initially false, becomes true at start of quote, then false when exit quote */
			else if (quoted && txt_in[k] == '\t') txt_in[k] = 29;
			else if (quoted && txt_in[k] == ' ')  txt_in[k] = 31;
		}
		while ((GMT_strtok (txt_in, " ", &pos, p))) {	/* Break up string into separate words, and strip off double quotes */
			unsigned int i, o;
			for (k = 0; p[k]; k++) if (p[k] == 29) p[k] = '\t'; else if (p[k] == 31) p[k] = ' ';	/* Replace spaces and tabs masked above */
			for (i = o = 0; p[i]; i++) if (p[i] != '\"') p[o++] = p[i];	/* Ignore double quotes */
			p[o] = '\0';
			new_args[new_n_args++] = strdup (p);
			if (new_n_args == n_alloc) {
				n_alloc += GMT_SMALL_CHUNK;
				new_args = GMT_memory (G, new_args, n_alloc, char *);
			}
		}
		for (k = 0; txt_in[k]; k++)
			if (txt_in[k] == 29) txt_in[k] = '\t'; else if (txt_in[k] == 31) txt_in[k] = ' ';	/* Replace spaces and tabs masked above */
		args = new_args;
		n_args = new_n_args;
	}
	else {
		args = (char **)in;	/* Gave an argv[] argument */
		n_args = n_args_in;
	}
	if (args == NULL && n_args) return_null (API, GMT_ARGV_LIST_NULL);	/* Conflict between # of args and args being NULL */
	
	for (arg = 0; arg < n_args; arg++) {	/* Loop over all command arguments */

		if (!args[arg]) continue;	/* Skip any NULL arguments quietly */

		if (args[arg][0] == '<' && !args[arg][1] && (arg+1) < n_args && args[arg+1][0] != '-')	/* string command with "< file" for input */
			first_char = 0, option = GMT_OPT_INFILE, arg++;
		else if (args[arg][0] == '>' && !args[arg][1] && (arg+1) < n_args && args[arg+1][0] != '-')	/* string command with "> file" for output */
			first_char = 0, option = GMT_OPT_OUTFILE, arg++;
		else if (args[arg][0] == '+' && !args[arg][1] && n_args == 1)	/* extended synopsis + */
			first_char = 1, option = GMT_OPT_USAGE, G->common.synopsis.extended = true;
		else if (args[arg][0] == '-' && args[arg][1] == '+' && !args[arg][2] && n_args == 1)	/* extended synopsis + */
			first_char = 1, option = GMT_OPT_USAGE, G->common.synopsis.extended = true;
		else if (args[arg][0] != '-')	/* Probably a file (could also be a gmt/grdmath OPERATOR or number, to be handled later by GMT_Make_Option) */
			first_char = 0, option = GMT_OPT_INFILE;
		else if (!args[arg][1])	/* Found the special synopsis option "-" */
			first_char = 1, option = GMT_OPT_SYNOPSIS;
		else if (!strcmp(args[arg], "--help"))	/* Translate '--help' to '-?' */
			first_char = 6, option = GMT_OPT_USAGE;
		else if ((isdigit ((int)args[arg][1]) || args[arg][1] == '.') && !GMT_not_numeric (API->GMT, args[arg])) /* A negative number, most likely; convert to "file" for now */
			first_char = 0, option = GMT_OPT_INFILE;
		else	/* Most likely found a regular option flag (e.g., -D45.0/3) */
			first_char = 2, option = args[arg][1];

		if ((new_opt = GMT_Make_Option (API, option, &args[arg][first_char])) == NULL)
			return_null (API, error);	/* Create the new option structure given the args, or return the error */

		if (option == GMT_OPT_INFILE && ((pch = strstr(new_opt->arg, "+b")) != NULL) && !strstr(new_opt->arg, "=gd")) {
			/* Here we deal with the case that the filename has embeded a band request for gdalread, as in img.tif+b1
			   However, the issue is that for these cases the machinery is set only to parse the request in the
			   form of img.tif=gd+b1 so the trick is to insert the '=gd' in the filename and let go.
			   JL 29-November 2014
			*/
			char t[GMT_LEN256] = {""};
			pch[0] = '\0';
			strcpy(t, new_opt->arg);
			strcat(t, "=gd"); 
			pch[0] = '+';			/* Restore what we have erased 2 lines above */
			strcat(t, pch);
			free(new_opt->arg);		/* free it so that we can extend it */
			new_opt->arg = strdup(t);
		}

		head = GMT_Append_Option (API, new_opt, head);		/* Hook new option to the end of the list (or initiate list if head == NULL) */
	}
	if (n_args_in == 0) {	/* Free up temporary arg list */
		for (arg = 0; arg < n_args; arg++) free (new_args[arg]);
		GMT_free (G, new_args);
	}

	return (head);		/* We return the linked list */
}

int GMT_Destroy_Options (void *V_API, struct GMT_OPTION **head)
{
	/* Delete the entire linked list of options, such as those created by GMT_Create_Options */

	struct GMT_OPTION *current = NULL, *delete = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	API = gmt_get_api_ptr (V_API);	/* Cast to GMTAPI_CTRL pointer */

	current = *head;
	while (current) {	/* Start at head and loop over the list and delete the options, one by one. */
		delete = current;		/* The one we want to delete */
		current = current->next;	/* But first grab the pointer to the next item */
		if (delete->arg) free (delete->arg);	/* The option had a text argument allocated by strdup, so free it first */
		GMT_free (API->GMT, delete);		/* Then free the structure which was allocated by GMT_memory */
	}
	*head = NULL;		/* Reset head to NULL value since it no longer points to any allocated memory */
	return (GMT_OK);	/* No error encountered */
}

char ** GMT_Create_Args (void *V_API, int *argc, struct GMT_OPTION *head)
{	/* This function creates a character array with the command line options that
	 * correspond to the linked options provided.  It is the inverse of GMT_Create_Options.
	 * The number of array strings is returned via *argc.
	 */

	char **txt = NULL, buffer[GMT_BUFSIZ] = {""};
	unsigned int arg = 0;
	size_t n_alloc = GMT_SMALL_CHUNK;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *G = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (head == NULL)  return_null (V_API, GMT_OPTION_LIST_NULL);	/* No list of options was given */
	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	*argc = 0;	/* Start off with no arguments */

	G = API->GMT;	/* GMT control structure */
	txt = GMT_memory (G, NULL, n_alloc, char *);

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt->option == GMT_OPT_SYNOPSIS)		/* Produce special - option for synopsis */
			sprintf (buffer, "-");
		else if (opt->option == GMT_OPT_INFILE)		/* Option for input filename [or numbers] */
			sprintf (buffer, "%s", opt->arg);
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline option with argument for some ? */
			sprintf (buffer, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument without argument */
			sprintf (buffer, "-%c", opt->option);

		txt[arg] = GMT_memory (G, NULL, strlen (buffer)+1, char);	/* Get memory for this item */

		/* Copy over the buffer contents */
		strcpy (txt[arg], buffer);
		arg++;	/* One more option added */
		if (arg == n_alloc) {	/* Need more space for our growing list */
			n_alloc += GMT_SMALL_CHUNK;
			txt = GMT_memory (G, txt, n_alloc, char *);
		}
	}
	/* OK, done processing all options */
	if (arg == 0) {	/* Found no options, so delete the list we allocated */
		GMT_free (G, txt);
	}
	else if (arg < n_alloc) {	/* Trim back on the list to fit what we want */
		txt = GMT_memory (G, txt, arg, char *);
	}
	
	*argc = arg;	/* Pass back the number of items created */
	return (txt);	/* Pass back the char* array to the calling module */
}

int GMT_Destroy_Args (void *V_API, int argc, char **args[])
{	/* Delete all text arguments, perhaps those created by GMT_Create_Args
	 * Note that a pointer to the args[] array is expected so that we can
	 * set it to NULL afterwards. */

	struct GMTAPI_CTRL *API = NULL;
	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (argc == 0 || !args) return_error (V_API, GMT_ARGV_LIST_NULL);	/* We were given no args to destroy, so there! */
	if (argc < 0) return_error (V_API, GMT_COUNTER_IS_NEGATIVE);		/* We were given a negative count! */
	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	/* Just deallocate the space taken by the list of arguments */
	while (argc--) GMT_free (API->GMT, (*args)[argc]);
	GMT_free (API->GMT, *args);	/* Free the array itself */
	return (GMT_OK);		/* No error encountered */
}

char * GMT_Create_Cmd (void *V_API, struct GMT_OPTION *head)
{	/* This function creates a single character string with the command line options that
	 * correspond to the linked options provided.
	 */

	char *txt = NULL, buffer[GMT_BUFSIZ] = {""};
	bool first = true;
	size_t length = 0, inc, n_alloc = GMT_BUFSIZ;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *G = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (head == NULL) return_null (V_API, GMT_OPTION_LIST_NULL);	/* No list of options was given */
	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	G = API->GMT;	/* GMT control structure */
	txt = GMT_memory (G, NULL, n_alloc, char);

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt->option == GMT_OPT_SYNOPSIS)		/* Produce special - option for synopsis */
			sprintf (buffer, "-");
		else if (opt->option == GMT_OPT_INFILE)		/* Option for input filename [or numbers] */
			sprintf (buffer, "%s", opt->arg);
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline option with argument for some ? */
			sprintf (buffer, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument without argument */
			sprintf (buffer, "-%c", opt->option);

		inc = strlen (buffer);
		if (!first) inc++;	/* Count the space between args */
		if ((length + inc) >= n_alloc) {	/* Will need more memory */
			n_alloc <<= 1;
			txt = GMT_memory (G, txt, n_alloc, char);
		}
		if (!first) strcat (txt, " ");	/* Add space betwen args */
		strcat (txt, buffer);
		length += inc;
		first = false;
	}
	length++;	/* Need space for trailing \0 */
	/* OK, done processing all options */
	if (length == 1)	/* Found no options, so delete the string we allocated */
		GMT_free (G, txt);
	else if (length < n_alloc)	/* Trim back on the list to fit what we want */
		txt = GMT_memory (G, txt, length, char);

	return (txt);		/* Pass back the results to the calling module */
}

int GMT_Destroy_Cmd (void *V_API, char **cmd)
{	/* Delete string created by GMT_Create_Cmd, pass its address */

	struct GMTAPI_CTRL *API = NULL;
	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (*cmd == NULL) return_error (V_API, GMT_ARG_IS_NULL);	/* No command was given */
	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	GMT_free (API->GMT, *cmd);	/* Free the command string */
	return (GMT_OK);		/* No error encountered */
}

struct GMT_OPTION *GMT_Make_Option (void *V_API, char option, char *arg)
{
	/* Create a structure option given the option character and the optional argument arg */
	struct GMT_OPTION *new_opt = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	/* Here we have a program-specific option or a file name.  In either case we create a new option structure */

	new_opt = GMT_memory (API->GMT, NULL, 1, struct GMT_OPTION);	/* Allocate one option structure */

	new_opt->option = option;		/* Assign which option character was used */
	if (!arg)				/* If arg is a NULL pointer: */
		new_opt->arg = strdup ("");	/* Copy an empty string, such that new_opt->arg[0] = '\0', which avoids */
						/* segfaults later on since few functions check for NULL pointers  */
	else {					/* If arg is set to something (may be an empty string): */
		new_opt->arg = strdup (arg);	/* Allocate space for the argument and duplicate it in the option structure */
		GMT_chop (new_opt->arg);	/* Get rid of any trailing \n \r from cross-binary use in Cygwin/Windows */
	}

	return (new_opt);		/* Pass back the pointer to the allocated option structure */
}

struct GMT_OPTION * GMT_Find_Option (void *V_API, char option, struct GMT_OPTION *head)
{
	/* Search the list for the selected option and return the pointer to the item.  Only the first occurrence will be found. */

	struct GMT_OPTION *current = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (head == NULL)  return_null (V_API, GMT_OPTION_LIST_NULL);	/* Hard to find something in a non-existant list */
	
	for (current = head; current && current->option != option; current = current->next);	/* Linearly search for the specified option */
	return (current);	/* NULL if not found */
}

int GMT_Update_Option (void *V_API, struct GMT_OPTION *opt, char *arg)
{
	/* Replaces the argument of this option with arg. */

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (opt == NULL) return_error (V_API, GMT_OPTION_IS_NULL);	/* We pass NULL as the option */
	if (arg == NULL) return_error (V_API, GMT_ARG_IS_NULL);		/* We pass NULL as the argument */
	if (opt->arg) free ((void *)opt->arg);
	opt->arg = strdup (arg);

	return (GMT_OK);	/* No error encountered */
}

struct GMT_OPTION * GMT_Append_Option (void *V_API, struct GMT_OPTION *new_opt, struct GMT_OPTION *head)
{
	/* Append this entry to the end of the linked list */
	struct GMT_OPTION *current = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (!new_opt) return_null (V_API, GMT_OPTION_IS_NULL);		/* No option was passed */
	if (!new_opt->arg) return_null (V_API, GMT_ARG_IS_NULL);	/* Option argument must not be null pointer */

	if (head == NULL) return (new_opt);			/* No list yet, let new_opt become the start of the list */

	/* Here the list already existed with head != NULL */

	if (new_opt->option == GMT_OPT_OUTFILE) {	/* Only allow one output file on command line */
		/* Search for existing output option */
		for (current = head; current->next && current->option != GMT_OPT_OUTFILE; current = current->next);
		if (current->option == GMT_OPT_OUTFILE) return_null (V_API, GMT_ONLY_ONE_ALLOWED);	/* Cannot have > 1 output file */
		/* Here current is at end so no need to loop again */
	}
	else {	/* Not an output file name so just go to end of list */
		for (current = head; current->next; current = current->next);			/* Go to end of list */
	}
	/* Append new_opt to the list */
	current->next = new_opt;
	new_opt->previous = current;

	return (head);		/* Return head of list */
}

int GMT_Delete_Option (void *V_API, struct GMT_OPTION *current)
{
	/* Remove the specified entry from the linked list.  It is assumed that current
	 * points to the correct option in the linked list. */
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (!current) return_error (V_API, GMT_OPTION_IS_NULL);		/* No option was passed */
	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	/* Remove the current option and bypass via the enx/prev pointers in the linked list */
	if (current->previous) current->previous->next = current->next;
	if (current->next) current->next->previous = current->previous;
	if (current->arg) free (current->arg);	/* Option arguments were created by strdup, so we must use free */
	GMT_free (API->GMT, current);		/* Option structure was created by GMT_memory, hence GMT_free */

	return (GMT_OK);	/* No error encountered */
}

int gmt_B_arg_inspector (struct GMT_CTRL *GMT, char *in) {
	/* Determines if the -B option indicates old GMT4-style switches and flags
	 * or if it follows the GMT 5 specification.  This is only called when
	 * compatibility mode has been compiled in; otherwise we only check GMT 5
	 * style arguments.  We return 5 for GMT5 style, 4 for GMT4 style, 9
	 * if no decision could be make and -1 if mixing of GMT4 & 5 styles are
	 * ound = that is an error. */
	size_t k, j, last;
	int gmt4 = 0, gmt5 = 0, n_colons = 0, n_slashes = 0, colon_text = 0, wesn_at_end = 0;
	bool ignore = false;	/* true if inside a colon-separated string under GMT4 style assumption */
	bool ignore5 = false;	/* true if label, title, prefix, suffix */
	bool custom = false;	/* True if -B[p|s][x|y|z]c<filename> was given; then we relax checing for .c (old second) */
	char mod = 0;
	
	if (!in || in[0] == 0) return (9);	/* Just a safety precaution, 9 means "GMT5" syntax but is is an empty string */
	last = strlen (in);
	k = (in[0] == 'p' || in[0] == 's') ? 1 : 0;	/* Skip p|s in -Bp|s */
	if (strchr ("xyz", in[k])) gmt5++;		/* Definitively GMT5 */
	if (k == 0 && !isdigit (in[0]) && strchr ("WESNwesn", in[1])) gmt5++;		/* Definitively GMT5 */
	j = k;
	while (j < last && (in[j] == 'x' || in[j] == 'y' || in[j] == 'z')) j++;
	custom = (in[j] == 'c');	/* Got -B[p|s][xyz]c<customfile> */
	for (k = 0; k < last; k++) {
		if (k && in[k] == '+' && in[k-1] == '@') {	/* Found a @+ PSL sequence, just skip */
			continue;	/* Resume processing */
		}
		if (ignore5) continue;	/* Don't look inside a GMT5 title, label, suffix, or prefix */
		if (in[k] == ':') {
#ifdef _WIN32		/* Filenames under Windows may be X:\<name> which should not trigger "colon" test */
			if (!(k && isalpha (in[k-1]) && k < last && in[k+1] == '\\'))
#endif
			if (k && in[k-1] == '@') {	/* Found a @:[<font>]: sequence, scan to end */
				k++;	/* Skip past the colon */
				while (in[k] && in[k] != ':') k++;	/* Find the matching colon */
				continue;	/* Resume processing */
			}
			ignore = !ignore, n_colons++;	/* Possibly stepping into a label/title */
			if (!ignore) continue;	/* End of title or label, skip check for next character */
			if (k < last && in[k+1] == '.') colon_text++;	/* Title */
			else if (k < last && in[k+1] == '=') colon_text++;	/* Annotation prefix */
			else if (k < last && in[k+1] == ',') colon_text++;	/* Annotation suffix */
		}
		if (ignore) continue;	/* Don't look inside a title or label */
		switch (in[k]) {
			case '/': if (mod == 0) n_slashes++; break;	/* Only GMT4 uses slashes */
			case '+':	/* Plus, might be GMT5 modifier switch */
				if      (k < last && in[k+1] == 'u') {mod = 'u'; ignore5 = true;  gmt5++;}	/* unit (suffix) settings */
				else if (k < last && in[k+1] == 'b') {mod = 'b'; ignore5 = false; gmt5++;}	/* 3-D box settings */
				else if (k < last && in[k+1] == 'g') {mod = 'g'; ignore5 = false; gmt5++;}	/* fill settings */
				else if (k < last && in[k+1] == 'o') {mod = 'o'; ignore5 = false; gmt5++;}	/* oblique pole settings */
				else if (k < last && in[k+1] == 'p') {mod = 'p'; ignore5 = true;  gmt5++;}	/* prefix settings */
				else if (k < last && in[k+1] == 'l') {mod = 'l'; ignore5 = true;  gmt5++;}	/* Label */
				else if (k < last && in[k+1] == 't') {mod = 't'; ignore5 = true;  gmt5++;}	/* title */
				else if (k && (in[k-1] == 'Z' || in[k-1] == 'z')) {ignore5 = false; gmt4++;}	/* Z-axis with 3-D box */
				break;
			case 'c':	/* If following a number this is unit c for seconds in GMT4 */
				if (!custom && k && (in[k-1] == '.' || isdigit (in[k-1]))) gmt4++;	/* Old-style second unit */
			case 'W': case 'E': case 'S': case 'N': case 'Z': case 'w': case 'e': case 'n': case 'z':	/* Not checking s as confusion with seconds */
				if (k > 1) wesn_at_end++;	/* GMT5 has -B<WESNwesn> up front while GMT4 usually has them at the end */
				break;
		}
	}
	if (!gmt5 && wesn_at_end) gmt4++;		/* Presumably got WESNwesn stuff towards the end of the option */
	if (n_colons && (n_colons % 2) == 0) gmt4++;	/* Presumably :labels: in GMT4 style as any +mod would have kicked in above */
	if (!custom && n_slashes) gmt4++;		/* Presumably / to separate axis in GMT4 style */
	if (colon_text) gmt4++;				/* Gave title, suffix, prefix in GMT4 style */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_B_arg_inspector: GMT4 = %d GMT5 = %d\n", gmt4, gmt5);
	if (gmt5 && !gmt4) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_B_arg_inspector: Detected GMT 5 style elements in -B option\n");
		return (5);
	}
	else if (gmt4 && !gmt5) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "gmt_B_arg_inspector: Detected GMT 4 style elements in -B option\n");
		return (4);
	}
	else if (gmt4 && gmt5) {	/* Mixed case is never allowed */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "gmt_B_arg_inspector: Error: Detected both GMT 4 and GMT 5 style elements in -B option. Unable to parse.\n");
		if (n_slashes) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "gmt_B_arg_inspector: Slashes no longer separate axis specifications, use -B[xyz] and repeat\n");
		if (colon_text || n_colons) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "gmt_B_arg_inspector: Colons no longer used for titles, labels, prefix, and suffix; see +t, +l, +p, +s\n");
		return (-1);
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_B_arg_inspector: Assume GMT 5 style format in -B option\n");
		return (9);
	}
}
	
int gmt_check_b_options (struct GMT_CTRL *GMT, struct GMT_OPTION *options)
{
	/* Determine how many -B options were given and if it is clear we are
	 * dealing with GMT4 or GMT5 syntax just by looking at this information.
	 * We also examine each argument for clues to version compatibility.
	 * We return 5 if it is clear this is GMT5 syntax, 4 if it is clear it
	 * is GMT 4 syntax, 9 if we cannot tell, and -1 if it is a mix of both.
	 * The latter will result in a syntax error.
	 */
	struct GMT_OPTION *opt = NULL;
	unsigned int n4_expected = 0, n_B = 0, gmt4 = 0, gmt5 = 0, k;
	int verdict;
	
	for (opt = options; opt; opt = opt->next) {	/* Loop over all given options */
		if (opt->option != 'B') continue;	/* Skip anything but -B options */
		n_B++;					/* Count how many (max 2 in GMT4 if -Bp|s given) */
		k = (opt->arg[0] == 'p' || opt->arg[0] == 's') ? 1 : 0;	/* Step over any p|s designation */
		if (k == 1) n4_expected++;		/* Count how many -Bp or -Bs were given */
		verdict = gmt_B_arg_inspector (GMT, opt->arg);	/* Check this argument, return 5, 4, 1 (undetermined) or 0 (mixing) */
		if (verdict == 4) gmt4++;
		if (verdict == 5) gmt5++;
		if (verdict == -1) gmt4++, gmt5++;	/* This is bad and will lead to a syntax error */
	}
	if (n4_expected == 0) n4_expected = 1;	/* If there are no -Bs|p options then GMT4 expects a single -B option */
	if (n_B > n4_expected) gmt5++;		/* Gave more -B options than expected for GMT4 */
	if (gmt5 && !gmt4)  return 5;		/* Matched GMT5 syntax only */
	if (gmt4 && !gmt5)  return 4;		/* Matched GMT4 syntax only */
	if (!gmt4 && !gmt5) return 9;		/* Could be either */
	return (-1);				/* Error: Cannot be both */
}

int GMT_Parse_Common (void *V_API, char *given_options, struct GMT_OPTION *options)
{
	/* GMT_Parse_Common parses the option list for a program and detects the GMT common options.
	 * These are processed in the order required by GMT regardless of order given.
	 * The settings will override values set previously by other commands.
	 * It ignores filenames and only return errors if GMT common options are misused.
	 * Note: GMT_CRITICAL_OPT_ORDER is defined in gmt_common.h
	 */

	struct GMT_OPTION *opt = NULL;
	char list[2] = {0, 0}, critical_opt_order[] = GMT_CRITICAL_OPT_ORDER;
	unsigned int i, n_errors = 0;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */

	/* Check if there are short-hand commands present (e.g., -J with no arguments); if so complete these to full options
	 * by consulting the current GMT history machinery.  If not possible then we have an error to report */

	API = gmt_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	if (GMT_Complete_Options (API->GMT, options)) return_error (API, GMT_OPTION_HISTORY_ERROR);	/* Replace shorthand failed */

	if (API->GMT->common.B.mode == 0) API->GMT->common.B.mode = gmt_check_b_options (API->GMT, options);	/* Determine the syntax of the -B option(s) */

	/* First parse the common options in the order they appear in GMT_CRITICAL_OPT_ORDER */
	for (i = 0; i < strlen (critical_opt_order); i++) {	/* These are the GMT options that must be parsed in this particular order, if present */
		if (strchr (given_options, critical_opt_order[i]) == NULL) continue;	/* Not a selected option */
		list[0] = critical_opt_order[i];	/* Just look for this particular option in the linked opt list */
		for (opt = options; opt; opt = opt->next) {
			if (opt->option != list[0]) continue;
			n_errors += GMT_parse_common_options (API->GMT, list, opt->option, opt->arg);
		}
	}

	/* Now any critical option given has been parsed.  Next we parse anything NOT a critical GMT option */
	for (i = 0; given_options[i]; i++) {	/* Loop over all options given */
		if (strchr (critical_opt_order, given_options[i])) continue;	/* Skip critical option */
		list[0] = given_options[i];	/* Just look for this particular option */
		for (opt = options; opt; opt = opt->next) {
			n_errors += GMT_parse_common_options (API->GMT, list, opt->option, opt->arg);
			if (opt->option != list[0]) continue;
		}
	}

	/* Update [read-only] pointer to the current option list */
	API->GMT->current.options = options;
	if (n_errors) return_error (API, GMT_PARSE_ERROR);	/* One or more options failed to parse */
	if (GMT_is_geographic (API->GMT, GMT_IN)) API->GMT->current.io.warn_geo_as_cartesion = false;	/* Don't need this warning */
	
	return (GMT_OK);
}
