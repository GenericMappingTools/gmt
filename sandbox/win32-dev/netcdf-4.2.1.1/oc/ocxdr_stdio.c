/* @(#)xdr_stdio.c	2.1 88/07/29 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)xdr_stdio.c 1.16 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_stdio.c, XDR implementation on standard i/o file.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * This set of routines implements a XDR on a stdio stream.
 * XDR_ENCODE serializes onto the stream, XDR_DECODE de-serializes
 * from the stream.
 */

/*
Modified to track the current position to avoid
the file io operation.
Not sure if this worth the effort because
I don't actually know the cost to doing
an fseek
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ocinternal.h"

#ifdef HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif

#ifdef WIN32
#include <winsock2.h>

#define strcasecmp stricmp

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#endif

#include <stdio.h>

#ifdef HAVE_RPC_XDR_H
#include <rpc/xdr.h>
#else
#include <xdr.h>
#endif

#ifdef _AIX
#include <netinet/in.h>
#endif

static bool_t	ocxdrstdio_getlong();
static bool_t	ocxdrstdio_putlong();
static bool_t	ocxdrstdio_getbytes();
static bool_t	ocxdrstdio_putbytes();
static u_int	ocxdrstdio_getpos();
static bool_t	ocxdrstdio_setpos();
static int *	ocxdrstdio_inline();
static void	ocxdrstdio_destroy();

/* Need this to keep AIX quiet */
#ifdef _AIX
typedef long* (*LOCALINLINE)();
#endif

/*
 * Ops vector for stdio type XDR
 */
static struct xdr_ops	ocxdrstdio_ops = {
	ocxdrstdio_getlong,	/* deseraialize a 32 bit int */
	ocxdrstdio_putlong,	/* seraialize a 32 bit  int */
	ocxdrstdio_getbytes,	/* deserialize counted bytes */
	ocxdrstdio_putbytes,	/* serialize counted bytes */
	ocxdrstdio_getpos,	/* get offset in the stream */
	ocxdrstdio_setpos,	/* set offset in the stream */
#ifdef _AIX
	(LOCALINLINE)ocxdrstdio_inline,	/* prime stream for inline macros */
#else
	ocxdrstdio_inline,	/* prime stream for inline macros */
#endif
	ocxdrstdio_destroy	/* destroy stream */
};

/*
 * Initialize a stdio xdr stream.
 * Sets the xdr stream handle xdrs for use on the stream file.
 * Operation flag is set to op.
 */
void
ocxdrstdio_create(xdrs, file, op)
	register XDR *xdrs;
	FILE *file;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &ocxdrstdio_ops;
	xdrs->x_private = (caddr_t)file;
	xdrs->x_handy = 0;
	xdrs->x_base = 0;
	xdrs->x_public = 0;
}

/*
 * Destroy a stdio xdr stream.
 * Cleans up the xdr stream handle xdrs previously set up by ocxdrstdio_create.
 */
static void
ocxdrstdio_destroy(xdrs)
	register XDR *xdrs;
{
	(void)fflush((FILE *)xdrs->x_private);
	/* xx should we close the file ?? */
}

static bool_t
ocxdrstdio_getlong(xdrs, lp)
	XDR *xdrs;
	register unsigned int *lp;
{
#ifdef IGNORE
FILE* f = (FILE*)xdrs->x_private;
long fpos = ftell(f);
#endif

if(fread((caddr_t)lp,sizeof(unsigned int),1,(FILE *)(xdrs->x_private)) != 1)
		return (FALSE);
#ifndef mc68000
	*lp = ocntoh(*lp);
#endif
	xdrs->x_public += sizeof(unsigned int);
	return (TRUE);
}

static bool_t
ocxdrstdio_putlong(xdrs, lp)
	XDR *xdrs;
	unsigned int* lp;
{
#ifndef mc68000
	unsigned int mycopy = ochton(*lp);
	lp = &mycopy;
#endif
	if(fwrite((caddr_t)lp, sizeof(unsigned int), 1, (FILE *)xdrs->x_private) != 1)
		return (FALSE);
	xdrs->x_public += sizeof(unsigned int);
	return (TRUE);
}

static bool_t
ocxdrstdio_getbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	u_int len;
{
	if((len != 0) && (fread(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	xdrs->x_public += len;
	return (TRUE);
}

static bool_t
ocxdrstdio_putbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	u_int len;
{
	if((len != 0)
           && (fwrite(addr,(int)len,1,(FILE *)xdrs->x_private) != 1))
		return (FALSE);
	xdrs->x_public += len;
	return (TRUE);
}

static u_int
ocxdrstdio_getpos(xdrs)
	XDR *xdrs;
{
    unsigned long result = (unsigned long)xdrs->x_public;
    return (unsigned int) result;
/*	return ((u_int) ftell((FILE *)xdrs->x_private)); */
}

static bool_t
ocxdrstdio_setpos(xdrs, pos) 
	XDR *xdrs;
	u_int pos;
{ 
        if(pos == (unsigned long)xdrs->x_public) return TRUE;
        xdrs->x_public = (caddr_t)(unsigned long)pos;
	return ((fseek((FILE *)xdrs->x_private, (long)pos, 0) < 0) ?
		FALSE : TRUE);
}

static int *
ocxdrstdio_inline(xdrs, len)
	XDR *xdrs;
	u_int len;
{

	/*
	 * Must do some work to implement this: must insure
	 * enough data in the underlying stdio buffer,
	 * that the buffer is aligned so that we can indirect through a
	 * long *, and stuff this pointer in xdrs->x_buf.  Doing
	 * a fread or fwrite to a scratch buffer would defeat
	 * most of the gains to be had here and require storage
	 * management on this buffer, so we don't do this.
	 */
	return (NULL);
}
