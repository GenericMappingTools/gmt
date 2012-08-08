/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include <stdlib.h>
#include "ncbytes.h"
#include "nc_path.h"

NCPath*
ncpath_append(NCPath* path, char* name)
{
    NCPath* newsegment = NULL;
    newsegment = (NCPath*)malloc(sizeof(NCPath));
    if(newsegment == NULL) return NULL;
    newsegment->name = strdup(name);
    newsegment->next = NULL;

    if(path == NULL) {
	path = newsegment;
    } else {
        /* Find last node */
	NCPath* last = path;
        while(last->next != NULL) last = last->next;
	last->next = newsegment;
    }	
    return path;
}


void
ncpath_free(NCPath* path)
{
    while(path != NULL) {
        NCPath* curr = path;
	if(curr->name) free(curr->name);
	path = curr->next;
	free(curr);
    }
}

int
ncpath_match(NCPath* path1, NCPath* path2)
{
    while(path1 != NULL && path2 != NULL) {
	if(strcmp(path1->name,path2->name)!=0) return 0;
	path1 = path1->next;
	path2 = path2->next;
    }
    if(path1 == NULL && path2 == NULL) return 1;
    return 0;
} 

NCPath*
ncpath_dup(NCPath* path)
{
    NCPath* newpath = NULL;
    while(path != NULL) {
	ncpath_append(newpath,path->name);
	path = path->next;
    }
    return newpath;
}

char*
ncpath_tostring(NCPath* path, char* sep, ncpath_encoder encoder)
{
    NCbytes* buf;
    char* result;

    if(path == NULL) return NULL;
    if(sep == NULL) sep = ".";
    buf = ncbytesnew();
    int first = 1;
    while(path != NULL) {
	if(!first) ncbytescat(buf,path->name);
	first = 0;
	if(encoder != NULL)
	    encoder(path->name,buf);	    
	else
	    ncbytescat(buf,path->name);
	path = path->next;
    }
    result = ncbytesdup(buf);
    ncbytesfree(buf);
    return result;
}

