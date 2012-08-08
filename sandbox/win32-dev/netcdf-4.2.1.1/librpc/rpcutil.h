/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef RPCUTIL_H
#define RPCUTIL_H

/*Forward*/
struct RPCnode;

extern int rpcstrindex(char* s, char* match);

extern void rpccollectnodepath(struct RPCnode*, NClist* path);

#endif /*RPCUTIL_H*/
