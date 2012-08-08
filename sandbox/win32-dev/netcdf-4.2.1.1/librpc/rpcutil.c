/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

#include "rpc_includes.h"

/* Define a local version of strindex */
int
rpcstrindex(char* s, char* match)
{
    size_t mlen = strlen(match);
    size_t slen = strlen(s);
    int i,j;
    if(slen < mlen) return -1;
    slen -= mlen;
    for(i=0;i<slen;i++) {
	int found = 1;
        for(j=0;j<mlen;j++) {
	    if(s[i+j] != match[j]) {found=0; break;}
	}
	if(found)
	    return i;
    }
    return -1;
}


/* Collect the set of parent group nodes of node */
void
rpccollectnodepath(CRnode* node, NClist* path)
{
    Group* group;
    if(node == NULL) return;
    nclistpush(path,(ncelem)node);
    group = node->group;
    while(group != NULL) {
	nclistinsert(path,0,(ncelem)group);
	group = ((CRnode*)group)->group;
    }
}
