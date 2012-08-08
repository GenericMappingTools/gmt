/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef INCLUDES_H
#define INCLUDES_H

#include "config.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#include <curl/curl.h>
#include "curlwrap.h"

#include "ncbytes.h"
#include "nclist.h"
#include "nclog.h"

#include "ast.h"

#include "netcdf.h"
#include "nc.h"
#include "ncdispatch.h"
#include "nclog.h"
#include "nc_logging.h"

#include "nccrnode.h"
#include "ncStreamx.h"
#include "nccrproto.h"

#include "nccr.h"
#include "crmeta.h"
#include "crdebug.h"
#include "crpath.h"
#include "crutil.h"
#include "cceconstraints.h"

#endif /*INCLUDES_H*/
