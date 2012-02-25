/* Copyright 2006, Unidata/UCAR. See COPYRIGHT file for more
   details. 

   This program checks a netCDF file to see how well it complies with
   the CF Metadata conventions.

   Ed Hartnett, 11/23/06

   $Id$
*/

#include <config.h>
#include <stdio.h>
#include <unistd.h> /* for getopt */
#include <netcdf.h>
#include <libcf.h>

/* This struct is used to keep track of user options from the command
 * line. */
typedef struct
{
      int verbose;
} OPTIONS_T;

/* This is a message to the user about how to use cfcheck. */
#define USAGE   "\
  file             Name of netCDF file\n"

/* This prints out a helpful message to the user when they screw up or
 * are confused and ask for help. */
static void
usage()
{
   fprintf(stderr, "cfcheck file\n%s", USAGE);
   fprintf(stderr, "netcdf library version %s\n", nc_inq_libvers());
}

/* This function checks one file for cf conventions. */
static int
do_cfcheck(char *filename, OPTIONS_T *options)
{
   printf("Checking CF conventions for file %s.\n", filename);
   return CF_NOERR;
}

int 
main(int argc, char** argv)
{
   extern int optind;
   extern int opterr;
   extern char *optarg;
   char c;
   OPTIONS_T options;
   int i;

   /* If the user called ncdump without arguments, print the usage
    * message and return peacefully. */
   if (argc <= 1)
   {
      usage();
      return 0;
   }

   /* See what options the user wants, and set flags in the OPTIONS_T
    * stuct. */
   while ((c = getopt(argc, argv, "?:v")) != EOF)
   {
      switch(c) 
      {
	 case '?':
	    usage();
	    return 0;
	 case 'v':
	    options.verbose = 1;
	    return 0;
      }
      argc -= optind;
      argv += optind;
   }

   /* Check each file in turn. */
   for (i = 1; i < argc; i++)
      if (argc > 0) 
	 do_cfcheck(argv[i], &options);

   return CF_NOERR;
}
