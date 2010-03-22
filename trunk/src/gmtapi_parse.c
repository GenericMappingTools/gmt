/* Here lie bits and pieces that might be used for GMT API command parsing
 * It includes what key prototypes and functions will look like, and shows
 * and example using psbasemap as a template (because is is very short)
 *
 * Things to tackle:
 * GMT needs to rearrange the order of the options so that certain ones are
 * processed before others, .e.g, -R before -J. */
 
#include "gmtapi.h"

int GMTAPI_Create_Options (int n_args, char *args[], struct GMT_OPTION **list)
{
	/* This function will loop over all the supplied arguments and
	 * returns a linked list of structures for each  program option
	 * These in turn will be processed by the program-specific parsers
	 */
	
	GMT_LONG first_option = TRUE;	/* So we know to initialize the head of the linked list */
	int arg;
	struct GMT_OPTION *current = NULL, *head, *new;
	
	/* Probably make a copy of the args and sort them in the required order (R before J etc) */
	
	head = NULL;	/* Start off pointing to NULL, i.e., no options provided */
	
	for (arg = 0; arg < n_args; arg++) {	/* Loop over all command arguments */
	
		if (!args[arg]) continue;	/* Skip any NULL arguments */
		
		GMTAPI_Make_Option (args[arg][1], args[arg], &new);	/* Create the new option structure */
		
		if (first_option)		/* Initialize the head of the linked list */
			head = current = new;
		else {				/* Just hook it onto the end of the current end of list */
			current->next = new;
			new->previous = current;
			current = current->next;	/* Set current to the last in the list */
		}
		first_option = FALSE;		/* No longer true after the first has been processed */
	}
	
	*list = head;

	return (GMTAPI_OK);
}

int GMTAPI_Destroy_Options (struct GMT_OPTION *head)
{
	/* simply delete the entire linked list */

	struct GMT_OPTION *current, *delete;

	current = head;
	while (current) {	/* loop over the list and delete options */
		delete = current;
		current = current->next;
		if (delete->arg) GMT_free ((void *)delete->arg);
		GMT_free ((void *)delete);
	}

	return (GMTAPI_OK);
}

int GMTAPI_Create_Args (int *argc, char **args[], struct GMT_OPTION *head)
{
	/* This function regeneates a character array with the command line option that
	 * correspond to the options provided.  The inverse of GMT_Parse_Args.
	 */
	
	char **txt = NULL;
#if WHEN_READY	
	buffer[BUFSIZ];
#endif		
	size_t arg = 0, n_alloc = GMT_SMALL_CHUNK;
	struct GMT_OPTION *current;
	
	txt = (char **) GMT_memory (VNULL, n_alloc, sizeof (char *), "GMTAPI_Create_Args");
	
	for (current = head; current; current = current->next) {
		/* NOTE: arg currently has the leading -<opt> so for now we just echo it */

#if WHEN_READY	
		if (current->option)
			sprintf (buffer, "-%c%s", current->option, current->arg);
		else	/* Filename */
			sprintf (buffer, "%c", current->option);
		
		txt[arg] = (char *) GMT_memory (VNULL, strlen (buffer)+1, sizeof (char), "GMTAPI_Create_Args");
		strcpy (txt[arg], buffer);
#else
		txt[arg] = (char *) GMT_memory (VNULL, strlen (current->arg)+1, sizeof (char), "GMTAPI_Create_Args");
		strcpy (txt[arg], current->arg);
#endif		
		arg++;
		if (arg == n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			txt = (char **) GMT_memory ((void *)txt, n_alloc, sizeof (char *), "GMTAPI_Create_Args");
		}
	}
	if (arg < n_alloc) txt = (char **) GMT_memory ((void *)txt, (size_t)arg, sizeof (char *), "GMTAPI_Create_Args");

	*args = txt;
	*argc = arg;
	
	return (GMTAPI_OK);
}

int GMTAPI_Destroy_Args (int argc, char *args[])
{
	/* Just deallocate the space taken by the list of arguments */
	while (argc) GMT_free ((void *)args[argc--]);
	return (GMTAPI_OK);
}

int GMTAPI_Make_Option (char option, char *arg, struct GMT_OPTION **ptr)
{
	struct GMT_OPTION *new;

	/* Here we have a program-specific option or a file name.  In either case we create a new option structure */
	/* NOTE: For now we are keeping the leading -<opt> in arg since GMT expects it */	
	new = (struct GMT_OPTION *) GMT_memory (VNULL, 1, sizeof (struct GMT_OPTION), "GMTAPI_Create_Options");
	new->option = option;		/* Assign which option character was used (* for file) */
#ifdef WNEN_READY
	if (arg[1] && arg[2]) {		/* Allocate space for the argument and copy to structure */
		new->arg = (char *) GMT_memory (VNULL, strlen (arg) - 1, sizeof (char), "GMTAPI_Create_Options");
		strcpy (new->arg, &arg[2]);
	}
#else
	new->arg = (char *) GMT_memory (VNULL, strlen (arg) + 1, sizeof (char), "GMTAPI_Create_Options");
	strcpy (new->arg, arg);
#endif
	new->common = (arg[0] == '-' && (arg[1] == '\0' || strchr ("BHJKOPRUVXxYybcf:", (int)arg[1])));
	
	*ptr = new;
	
	return (GMTAPI_OK);
}

int GMTAPI_File_Option (char option, int ID, struct GMT_OPTION **ptr)
{
	struct GMT_OPTION *new;
	char file[BUFSIZ];

	/* Here we have a registered resource  ID */
	new = (struct GMT_OPTION *) GMT_memory (VNULL, 1, sizeof (struct GMT_OPTION), "GMTAPI_File_Option");
	new->option = option;		/* Assign which option character was used (* for file) */
	sprintf (file, "GMTAPI-#-%d", ID);
#ifdef WNEN_READY
	new->arg = (char *) GMT_memory (VNULL, strlen (file) + 1, sizeof (char), "GMTAPI_File_Option");
	strcpy (new->arg, file);
#else
	if (option == '*') {
		new->arg = (char *) GMT_memory (VNULL, strlen (file) + 1, sizeof (char), "GMTAPI_File_Option");
		strcpy (new->arg, file);
	}
	else {
		new->arg = (char *) GMT_memory (VNULL, strlen (file) + 3, sizeof (char), "GMTAPI_File_Option");
		sprintf (new->arg, "-%c%s", option, file);
	}
#endif

	*ptr = new;
	return (GMTAPI_OK);
}

int GMTAPI_Find_Option (char option, struct GMT_OPTION *head, struct GMT_OPTION *ptr)
{
	/* Search the list for the selected option and return pointer */

	struct GMT_OPTION *current;

	for (current = head; current && current->option != option; current = current->next);
	ptr = current;	/* Will be NULL if option was not found */
	return (GMTAPI_OK);
}

int GMTAPI_Update_Option (char option, char *arg, struct GMT_OPTION *head)
{
	/* If the option exists we will update the arguments, otherwise
	 * we create a new option and append it to the list */

	struct GMT_OPTION *old = NULL, *new;

	GMTAPI_Make_Option (option, arg, &new);		/* What the updated option should be */
	GMTAPI_Find_Option (option, head, old);		/* Try to see if it already existst */
	if (old) GMTAPI_Delete_Option (old);		/* Option is present - remove it  */
	GMTAPI_Append_Option (new, head);		/* Append revised option to list */
	
	return (GMTAPI_OK);
}

int GMTAPI_Append_Option (struct GMT_OPTION *new, struct GMT_OPTION *head)
{
	/* Append this entry to the end of the linked list */
	struct GMT_OPTION *current;
	
	for (current = head; current->next; current = current->next);	/* Go to end of list */
	current->next = new;
	new->previous = current;
	
	return (GMTAPI_OK);
}

int GMTAPI_Delete_Option (struct GMT_OPTION *current)
{
	/* Remove this entry from the linked list */

	if (current->previous) current->previous->next = current->next;
	if (current->next) current->next->previous = current->previous;
	if (current->arg) GMT_free ((void *)current->arg);
	GMT_free ((void *)current);
	
	return (GMTAPI_OK);
}

#if 0

/* This is how every GMT application file program.c will be written.
 * The next three routines will be automatically written by a script
 * since the only difference is the name of the program. */

#include "gmtapi.h"	/* The only include file needed */

int main (int argc, char *argv[])	: Compiled and linked to give us the usual program executable */
int GMT_program_cmd (struct GMTAPI_CTRL *API, int n_args, char *args[]) : Front end for text-command interface to program
int GMT_program (struct GMTAPI_CTRL *API, struct GMT_OPTION *options) : Front end for option interface to program

/* These three functions must be hand-crafted for each GMT programs */

int program_function (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	struct PROGRAM_CTRL *CTRL;	/* Control structure specific to program */
	
	/* This is what the GMT program actually does */
	
	/* 1. Parse the program-specific arguments */
	
	if (program_parse (API, CTRL, options)) return (GMTAPI_PARSE_ERROR);
	
	/* 2 The main action */
}	

int program_parse (struct GMTAPI_CTRL *API, struct PROGRAM_CTRL *CTRL, struct GMT_OPTION *options)
{
	/* This parses the options specific to this program and sets parameters in CTRL.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */
}

void program_usage (struct GMTAPI_CTRL *API, GMT_LONG synopsis_only)
{
	/* This displays the program synopsis and optionally full usage information.
	 */
}

#endif
