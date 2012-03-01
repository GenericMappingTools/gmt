/*
 * Copyright 1993-1996 University Corporation for Atmospheric Research/Unidata
 * 
 * Portions of this software were developed by the Unidata Program at the 
 * University Corporation for Atmospheric Research.
 * 
 * Access and use of this software shall impose the following obligations
 * and understandings on the user. The user is granted the right, without
 * any fee or cost, to use, copy, modify, alter, enhance and distribute
 * this software, and any derivative works thereof, and its supporting
 * documentation for any purpose whatsoever, provided that this entire
 * notice appears in all copies of the software, derivative works and
 * supporting documentation.  Further, UCAR requests that the user credit
 * UCAR/Unidata in any publications that result from the use of this
 * software or in any product that includes this software. The names UCAR
 * and/or Unidata, however, may not be used in any advertising or publicity
 * to endorse or promote any products or commercial entity unless specific
 * written permission is obtained from UCAR/Unidata. The user also
 * understands that UCAR/Unidata is not obligated to provide the user with
 * any support, consulting, training or assistance of any kind with regard
 * to the use, operation and performance of this software nor to provide
 * the user with any updates, revisions, new versions or "bug fixes."
 * 
 * THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* "$Id$" */

#ifndef _NCCRDISPATCH_H
#define _NCCRDISPATCH_H

#include <stddef.h> /* size_t, ptrdiff_t */
#include "netcdf.h"
#include "ncdispatch.h"

#if defined(__cplusplus)
extern "C" {
#endif
/*
 * The Interface
 */


/**/

EXTERNL int
NCCR_new_nc(NC**);

/* WARNING: this signature differs from external nc_open API*/
EXTERNL int
NCCR_open(const char *path, int mode,
         int basepe, size_t *chunksizehintp,
         int use_parallel, void* mpidata,
         struct NC_Dispatch* dispatch, NC** ncp);

EXTERNL int
NCCR_close(int ncid);

EXTERNL int
NCCR_get_vara(int ncid, int varid,
	      const size_t* startp,
	      const size_t* countp,
	      void* data,
	      nc_type externaltype0);

EXTERNL int NCCR_close(int ncid);
EXTERNL int NCCR_sync(int ncid);
EXTERNL int NCCR_abort(int ncid);

/* End _var */

EXTERNL int NCCR_initialize(void);

#if defined(__cplusplus)
}
#endif

#endif /*_NCCRDISPATCH_H*/
