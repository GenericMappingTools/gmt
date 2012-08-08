/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NC_PATH_H
#define NC_PATH_H

/* In order to allow greater lattitude in
   what characters are allowed in names,
   we avoid stringifying the pathname; instead
   we use a linked list of names.
*/

typedef struct NCPath {
    char* name; /* this segment of the full path */
    struct NCPath* next;
} NCPath;

extern NCPath* ncpath_append(NCPath*,char*);
extern void ncpath_free(NCPath*);
extern int ncpath_match(NCPath*,NCPath*);
extern NCPath* ncpath_dup(NCPath*);

/* Define function ptr for encoding */
struct NCBytes;
typedef char* (*ncpath_encoder)(char*,struct NCbytes*);

/* Caller free's return value */
extern char* ncpath_tostring(NCPath* path, char* sep, ncpath_encoder);

#endif /*NC_PATH_H*/
