/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef CRUTIL_H
#define CRUTIL_H

/*Forward*/
struct CRnode;


extern int crstrindex(char* s, char* match);

extern void crcollectnodepath(struct CRnode*, NClist* path);

#endif /*CRUTIL_H*/
