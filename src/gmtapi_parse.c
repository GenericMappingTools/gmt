/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Date:	1-JAN-2010
 * Version:	1.0
 *
 * Here lie the public functions used for GMT API command parsing.
 * They are:
 *
 * GMT_Create_Options	: Convert an array of text args to a linked option list
 * GMT_Destroy_Options	: Delete the linked option list
 * GMT_Create_Args	: Convert a struct option list back to an array of text args
 * GMT_Destroy_Args	: Delete the array of text args
 * GMT_Create_Cmd	: Convert a struct option list to a single command text. 
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
 
#include "gmt.h"

#define ptr_return(API,err,ptr) { GMT_Report_Error(API,err); return (ptr);}
#define return_null(API,err) { GMT_Report_Error(API,err); return (NULL);}
#define return_error(API,err) { GMT_Report_Error(API,err); return (TRUE);}
#define return_value(API,err,val) { GMT_Report_Error(API,err); return (val);}

#ifdef DEBUG
GMT_LONG GMT_List_Args (struct GMTAPI_CTRL *API, struct GMT_OPTION *head)
{	/* This function dumps the options to stderr for debug purposes.
	 * It is not meant to be part of the API but to assist developers
	 * during the debug phase of development.
	 */

	struct GMT_OPTION *opt = NULL;

	if (head == NULL) return_error (API, GMT_OPTION_LIST_NULL);

	fprintf (stderr, "Options:");
	for (opt = head; opt; opt = opt->next) {
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt != head) fprintf (stderr, " ");
		if (opt->option == GMTAPI_OPT_SYNOPSIS)			/* Produce special - command for synopsis */
			fprintf (stderr, "-");
		else if (opt->option == GMTAPI_OPT_INFILE)		/* Option for input filename [or number] */
			fprintf (stderr, "%s", opt->arg);
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline argument */
			fprintf (stderr, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument */
			fprintf (stderr, "-%c", opt->option);

	}
	fprintf (stderr, "\n");

	return (API->error = GMT_OK);	/* No error encountered */
}
#endif

struct GMT_OPTION * GMT_Create_Options (struct GMTAPI_CTRL *API, GMT_LONG n_args_in, void *in)
{
	/* This function will loop over the n_args_in supplied command line arguments (in) and
	 * returns a linked list of GMT_OPTION structures for each program option.
	 * These will in turn will be processed by the program-specific option parsers.
	 * Note: If argv[0] is the calling program name you should pass argc-1, args+1 instead.
	 * We assume list is not associated with any allocated memory but will be assigned to
	 * a structure allocated by this function.  Note: If n_args_in == 0 we assume in
	 * is a string with many options (e.g., "-R0/2/0/5 -Jx1 -O -m > file") and we must
	 * first break this command string into separate words.
	 */

	GMT_LONG arg, first_char, n_args, error = GMT_OK;
	char option, **args = NULL, **new_args = NULL;
	struct GMT_OPTION *head = NULL, *new = NULL;
	struct GMT_CTRL *G = API->GMT;	/* GMT control structure */

	if (API == NULL) return_null (API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */

	if (n_args_in == 0) {	/* Check if a single command line, if so break into tokens */
		GMT_LONG pos = 0, new_n_args = 0;
		size_t n_alloc = GMT_SMALL_CHUNK;
		char p[GMT_BUFSIZ], *txt_in = in;	/* Passed a single text string */
		new_args = GMT_memory (G, NULL, n_alloc, char *);

		while ((GMT_strtok (txt_in, " ", &pos, p))) {	/* Break up string into separate words */
			new_args[new_n_args++] = strdup (p);
			if (new_n_args == n_alloc) {
				n_alloc += GMT_SMALL_CHUNK;
				new_args = GMT_memory (G, new_args, n_alloc, char *);
			}
		}
		args = new_args;
		n_args = new_n_args;
	}
	else {
		args = (char **)in;	/* Gave an argv[] argument */
		n_args = n_args_in;
	}
	for (arg = 0; arg < n_args; arg++) {	/* Loop over all command arguments */

		if (!args[arg]) continue;	/* Skip any NULL arguments quietly */

		if (args[arg][0] == '<' && !args[arg][1] && (arg+1) < n_args && args[arg+1][0] != '-') {	/* string command with "< file" for input */
			first_char = 0;	option = GMTAPI_OPT_INFILE;	arg++;
		}
		else if (args[arg][0] == '>' && !args[arg][1] && (arg+1) < n_args && args[arg+1][0] != '-') {	/* string command with "> file" for output */
			first_char = 0;	option = GMTAPI_OPT_OUTFILE;	arg++;
		}
		else if (args[arg][0] != '-') {	/* Probably a file (could also be a gmt/grdmath OPERATOR or number, to be handled later by GMT_Make_Option) */
			first_char = 0;	option = GMTAPI_OPT_INFILE;
		}
		else if (!args[arg][1]) {	/* Found the special synopsis option "-" */
			first_char = 1, option = GMTAPI_OPT_SYNOPSIS;
		}
		else {		/* Most likely found a regular option flag (e.g., -D45.0/3) */
			/* Yet, negative numbers pose a problem as their leading - is seen as an option.  Next we address this */
			if ((isdigit ((int)args[arg][1]) || args[arg][1] == '.') && !GMT_not_numeric (API->GMT, args[arg])) {
				first_char = 0, option = GMTAPI_OPT_INFILE;		/* A negative number, most likely; convert to "file" for now */
			}
			else {	/* Seems like a regular option setting */
				first_char = 2, option = args[arg][1];
			}
		}

		if ((new = GMT_Make_Option (API, option, &args[arg][first_char])) == NULL) return_null (API, error);	/* Create the new option structure given the args, or return the error */

		head = GMT_Append_Option (API, new, head);		/* Hook new option to the end of the list (or initiate list if head == NULL) */
	}
	if (n_args_in == 0) {	/* Free up temporary arg list */
		for (arg = 0; arg < n_args; arg++) free (new_args[arg]);
		GMT_free (G, new_args);
	}

	API->error = GMT_OK;	/* No error encountered */
	return (head);		/* We return the linked list */
}

GMT_LONG GMT_Destroy_Options (struct GMTAPI_CTRL *API, struct GMT_OPTION **head)
{
	/* Delete the entire linked list of options, such as those created by GMT_Create_Options */

	struct GMT_OPTION *current = NULL, *delete = NULL;

	if (API == NULL) return_error (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (*head == NULL) return (FALSE);				/* Cannot delete a non-existant list */

	current = *head;
	while (current) {	/* Start at head and loop over the list and delete the options, one by one. */
		delete = current;
		current = current->next;
		if (delete->arg) free (delete->arg);	/* This option had a text argument allocated by strdup, so free it */
		GMT_free (API->GMT, delete);		/* Then free the structure which was allocated by GMT_memory */
	}
	*head = NULL;	/* Reset head to NULL value since it no longer points to any allocated memory */
	return (API->error = GMT_OK);	/* No error encountered */
}

char ** GMT_Create_Args (struct GMTAPI_CTRL *API, GMT_LONG *argc, struct GMT_OPTION *head)
{	/* This function creates a character array with the command line options that
	 * correspond to the linked options provided.  It is the inverse of GMT_Create_Options.
	 */

	char **txt = NULL, buffer[GMT_BUFSIZ];
	GMT_LONG arg = 0;
	size_t n_alloc = GMT_SMALL_CHUNK;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *G = API->GMT;	/* GMT control structure */

	if (API  == NULL) return_null (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (head == NULL) return_null (API, GMT_OPTION_LIST_NULL);	/* No list of options was given */

	*argc = 0;	/* Start off with no arguments */

	txt = GMT_memory (G, NULL, n_alloc, char *);

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt->option == GMTAPI_OPT_SYNOPSIS)		/* Produce special - option for synopsis */
			sprintf (buffer, "-");
		else if (opt->option == GMTAPI_OPT_INFILE)		/* Option for input filename [or numbers] */
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
	API->error = GMT_OK;	/* No error encountered */
	return (txt);		/* Pass back the char* array to the calling module */
}

GMT_LONG GMT_Destroy_Args (struct GMTAPI_CTRL *API, GMT_LONG argc, char *args[])
{	/* Delete all text arguments, perhaps those created by GMT_Create_Args */

	if (API == NULL) return_error (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (argc == 0 || !args) return_error (API, GMT_ARGV_LIST_NULL);	/* We were given no args to destroy, so there! */
	/* Just deallocate the space taken by the list of arguments */
	while (argc--) GMT_free (API->GMT, args[argc]);
	GMT_free (API->GMT, args);
	return (API->error = GMT_OK);	/* No error encountered */
}

char * GMT_Create_Cmd (struct GMTAPI_CTRL *API, struct GMT_OPTION *head)
{	/* This function creates a single character string with the command line options that
	 * correspond to the linked options provided.
	 */

	char *txt = NULL, buffer[GMT_BUFSIZ];
	GMT_LONG length = 0, inc, first = TRUE;
	size_t n_alloc = GMT_BUFSIZ;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *G = API->GMT;	/* GMT control structure */

	if (API == NULL) return_null (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (head == NULL) return_null (API, GMT_OPTION_LIST_NULL);	/* No list of options was given */

	txt = GMT_memory (G, NULL, n_alloc, char);

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt->option == GMTAPI_OPT_SYNOPSIS)		/* Produce special - option for synopsis */
			sprintf (buffer, "-");
		else if (opt->option == GMTAPI_OPT_INFILE)		/* Option for input filename [or numbers] */
			sprintf (buffer, "%s", opt->arg);
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline option with argument for some ? */
			sprintf (buffer, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument without argument */
			sprintf (buffer, "-%c", opt->option);

		inc = strlen (buffer);
		if (!first) inc++;	/* Count the space */
		if ((length + inc) >= n_alloc) txt = GMT_memory (G, NULL, n_alloc *= 2, char);
		if (!first) strcat (txt, " ");
		strcat (txt, buffer);
		length += inc;
		first = FALSE;
	}
	length++;	/* Need space for trailing \0 */
	/* OK, done processing all options */
	if (length == 1) {	/* Found no options, so delete the string we allocated */
		GMT_free (G, txt);
	}
	else if (length < n_alloc) {	/* Trim back on the list to fit what we want */
		txt = GMT_memory (G, txt, length, char);
	}

	API->error = GMT_OK;	/* No error encountered */
	return (txt);		/* Pass back the results to the calling module */
}

struct GMT_OPTION * GMT_Prep_Options (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{	/* Either we passed an option struct list or we passed argc, argv and must convert to get option list */
	struct GMT_OPTION *options = NULL;
	if (mode < 0)	/* Gave a list of options already */	
		options = args;
	else		/* Build them from text arguments */
		options = GMT_Create_Options (API, mode, args);
	return (options);
}

struct GMT_OPTION * GMT_Make_Option (struct GMTAPI_CTRL *API, char option, char *arg)
{
	/* Create a structure option given the option character and the optional argument arg */
	struct GMT_OPTION *new = NULL;

	if (API == NULL) return_null (API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */

	/* Here we have a program-specific option or a file name.  In either case we create a new option structure */

	new = GMT_memory (API->GMT, NULL, 1, struct GMT_OPTION);

	new->option = option;		/* Assign which option character was used */
	if (!arg)				/* If arg is a NULL pointer: */
		new->arg = strdup ("");		/* Copy an empty string, such that new->arg[0] = '\0', which avoids */
						/* segfaults later on since few functions check for NULL pointers  */
	else {					/* If arg is set to something (may be an empty string): */
		new->arg = strdup (arg);	/* Allocate space for the argument and duplicate it in the option structure */
		GMT_chop (new->arg);		/* Get rid of any trailing \n \r from cross-binary use in Cygwin/Windows */
	}

	API->error = GMT_OK;	/* No error encountered */
	return (new);		/* Pass back the pointer to the allocated option structure */
}

struct GMT_OPTION * GMT_Find_Option (struct GMTAPI_CTRL *API, char option, struct GMT_OPTION *head)
{
	/* Search the list for the selected option and return the pointer to the item.  Only the first occurrence will be found. */

	struct GMT_OPTION *current = NULL;

	if (API ==  NULL) return_null (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (head == NULL) return_null (API, GMT_OPTION_LIST_NULL);	/* Hard to find something in a non-existant list */
	
	for (current = head; current && current->option != option; current = current->next);	/* Linearly search for the specified option */
	API->error = (current) ? GMT_OK : GMT_OPTION_NOT_FOUND;	/* Set error code explicitly since it is not a fatal error */
	return (current);
}

GMT_LONG GMT_Update_Option (struct GMTAPI_CTRL *API, char option, char *arg, struct GMT_OPTION *head)
{
	/* If the option exists in the list we will update the arguments, otherwise
	 * we create a new option and append it to the end of the list.
	 * Note: The option order may be changed by this function. */

	struct GMT_OPTION *old = NULL, *new = NULL;

	if (API == NULL) return_error (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (head == NULL) return_error (API, GMT_OPTION_LIST_NULL);	/* We were given no list to update */
	if ((new = GMT_Make_Option (API, option, arg)) == NULL) return_error (API, API->error);		/* new holds what the updated option should be */
	if ((old = GMT_Find_Option (API, option, head)) == NULL) return_error (API, API->error);	/* Try to see if it already exists */
	GMT_Delete_Option (API, old);	/* Option is present in the list - remove it first */
	if ((head = GMT_Append_Option (API, new, head)) == NULL) return_error (API, API->error);	/* Append revised option to list */

	return (API->error = GMT_OK);	/* No error encountered */
}

struct GMT_OPTION * GMT_Append_Option (struct GMTAPI_CTRL *API, struct GMT_OPTION *new, struct GMT_OPTION *head)
{
	/* Append this entry to the end of the linked list */
	struct GMT_OPTION *current = NULL;

	if (API == NULL) return_null (API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (!new) return_null (API, GMT_OPTION_IS_NULL);	/* No option was passed */
	if (!new->arg) return_null (API, GMT_ARG_IS_NULL);	/* Option argument must not be null pointer */

	if (head == NULL) ptr_return (API, GMT_OK, new);	/* No list yet, let new become the start of the list */

	/* Here the list already existed with head != NULL */

	if (new->option == GMTAPI_OPT_OUTFILE) {	/* Only allow one output file on command line */
		/* Search for existing output option */
		for (current = head; current->next && current->option != GMTAPI_OPT_OUTFILE; current = current->next);
		if (current->option == GMTAPI_OPT_OUTFILE) return_null (API, GMT_ONLY_ONE_ALLOWED);	/* Cannot have > 1 output file */
		/* Here current is at end so no need to loop again */
	}
	else {	/* Not an output file name so just go to end of list */
		for (current = head; current->next; current = current->next);			/* Go to end of list */
	}
	/* Append new to the list */
	current->next = new;
	new->previous = current;

	API->error = GMT_OK;	/* No error encountered */
	return (head);		/* Return head of list */
}

GMT_LONG GMT_Delete_Option (struct GMTAPI_CTRL *API, struct GMT_OPTION *current)
{
	/* Remove the specified entry from the linked list.  It is assumed that current
	 * points to the correct option in the linked list. */

	if (API == NULL) return_error (API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (!current) return_error (API, GMT_OPTION_IS_NULL);		/* No option was passed */

	/* Remove the current option and bypass via the enx/prev pointers in the linked list */
	if (current->previous) current->previous->next = current->next;
	if (current->next) current->next->previous = current->previous;
	if (current->arg) free (current->arg);	/* Option arguments were created by strdup, so we must use free */
	GMT_free (API->GMT, current);		/* Option structure was created by GMT_memory, hence GMT_free */

	return (API->error = GMT_OK);	/* No error encountered */
}

GMT_LONG GMT_Parse_Common (struct GMTAPI_CTRL *API, char *sorted, char *unsorted, struct GMT_OPTION *options)
{
	/* GMT_Parse_Common parses the option list for two types of common options:
	 * sorted   = list of options to be processed in the order they appear in sorted_list
	 * unsorted = list of options to be processed in the order they appear in options
	 * The settings will override values set previously by other commands.
	 * It ignores filenames and only return errors if GMT common options are misused.
	 */

	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = API->GMT;
	char list[2] = {0, 0};
	GMT_LONG i, n_errors = 0;

	if (API == NULL) return_error (API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */

	/* Check if there are short-hand commands present (e.g., -J with no arguments); if so complete these to full options
	 * by consulting the current GMT history machinery.  If not possible then we have an error to report */

	if (GMT_Complete_Options (API->GMT, options)) return_error (API, GMT_OPTION_HISTORY_ERROR);	/* Replace shorthand failed */

	/* Parse the common options in the order they appear in "sorted" */

	for (i = 0; sorted[i]; i++) {
		list[0] = sorted[i];	/* Just look for this particular option */
		for (opt = options; opt; opt = opt->next) n_errors += GMT_parse_common_options (GMT, list, opt->option, opt->arg);
	}

	/* Parse the common options in "unsorted" in the order they appear in the options list */

	for (opt = options; opt; opt = opt->next) n_errors += GMT_parse_common_options (GMT, unsorted, opt->option, opt->arg);

	return ((API->error = (n_errors) ? GMT_PARSE_ERROR : GMT_OK));
}
