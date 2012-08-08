/*
  Copyright 2007, University Corporation for Atmospheric Research. See
  COPYRIGHT file for copying and redistribution conditions.

  This file is part of the NetCDF CF Library. 

  This file handles errors and logging.

  Ed Hartnett, 5/22/07
*/

#ifndef _CFERROR_
#define _CFERROR_

#include <config.h>
#include <stdlib.h>
#include <assert.h>

#ifdef LOGGING

/* To log something... */
void cf_log(int severity, const char *fmt, ...);
#define LOG(e) cf_log e

#else

/* These definitions will be used unless LOGGING is defined. */
#define LOG(e)
#define cf_set_log_level(e)
#endif

#endif /* _CFERROR_ */





