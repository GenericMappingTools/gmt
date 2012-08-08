/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef CRPATH_H
#define CRPATH_H

/* In order to allow greater lattitude in
   what characters are allowed in names,
   we avoid stringifying the pathname; instead
   we use a linked list of names.
*/

/* Note: the matching  code is current in crutil.c */

typedef struct CRpath {
    char* name; /* this segment of the full path */
    struct CRpath* next;
} CRpath;

extern CRpath* crpathappend(CRpath*,char*);
extern void crpathfree(CRpath*);
extern int crpathmatch(CRpath*,CRpath*);
extern char* crpathstring(CRpath*);
extern CRpath*  crpathdup(CRpath*);

#endif /*CRPATH_H*/
