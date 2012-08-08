/*
  Copyright 2007, University Corporation for Atmospheric Research. See
  COPYRIGHT file for copying and redistribution conditions.

  This file is part of the NetCDF CF Library. 

  This file handles errors and logging.

  Ed Hartnett, 5/22/07

  $Id$
*/

#include <config.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <cferror.h>
#include <assert.h>

/* This contents of this file get skipped if LOGGING is not defined
 * during compile. */
#ifdef LOGGING

/* This is the severity level of messages which will be logged. Use
   severity 0 for errors, 1 for important log messages, 2 for less
   important, etc. */
int cf_log_level = -1;

/* This function prints out a message, if the severity of the message
   is lower than the global cf_log_level. To use it, do something like
   this:
   
   cf_log(0, "this computer will explode in %d seconds", i);

   After the first arg (the severity), use the rest like a normal
   printf statement. Output will appear on stdout.

   This function is heavily based on the function in section 15.5 of
   the C FAQ. */
void cf_log(int severity, const char *fmt, ...)
{
   va_list argp;
   int t;

   /* If the severity is greater than the log level, we don' care to
      print this message. */
   if (severity > cf_log_level)
      return;

   /* If the severity is zero, this is an error. Otherwise insert that
      many tabs before the message. */
   if (!severity)
      fprintf(stdout, "ERROR: ");
   for (t=0; t<severity; t++)
      fprintf(stdout, "\t");

   /* Print out the variable list of args with vprintf. */
   va_start(argp, fmt);
   vfprintf(stdout, fmt, argp);
   va_end(argp);
   
   /* Put on a final linefeed. */
   fprintf(stdout, "\n");
   fflush(stdout);
}

/* Use this to set the global log level. Set it to CF_TURN_OFF_LOGGING
   (-1) to turn off all logging. Set it to 0 to show only errors, and
   to higher numbers to show more and more logging details. */
int 
cf_set_log_level(int new_level)
{
   /* Now remember the new level. */
   cf_log_level = new_level;
   LOG((4, "log_level changed to %d", cf_log_level));
   return 0;
} 

#endif /* LOGGING */
