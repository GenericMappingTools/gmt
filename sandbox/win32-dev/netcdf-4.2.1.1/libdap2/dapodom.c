/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapodom.c,v 1.12 2010/05/27 21:34:08 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "dapodom.h"

/**********************************************/
/* Define methods for a dimension dapodometer*/

/* Build an odometer covering slices startslice upto, but not including, stopslice */
Dapodometer*
dapodom_fromsegment(DCEsegment* segment, size_t startslice, size_t stopslice)
{
    int i;
    Dapodometer* odom;

    assert(stopslice > startslice);
    assert((stopslice - startslice) <= NC_MAX_VAR_DIMS);
    odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = (stopslice - startslice);
    for(i=0;i<odom->rank;i++) {
	odom->start[i] = segment->slices[i+startslice].first;
	odom->count[i] = segment->slices[i+startslice].count;
	odom->stride[i] = segment->slices[i+startslice].stride;
	odom->declsize[i] = segment->slices[i+startslice].declsize;
	odom->stop[i] = odom->start[i] + odom->count[i];
	odom->index[i] = odom->start[i];
    }    
    return odom;
}

Dapodometer*
dapodom_new(size_t rank,
	    const size_t* start, const size_t* count,
	    const ptrdiff_t* stride, const size_t* size)
{
    int i;
    Dapodometer* odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = rank;
    assert(odom->rank <= NC_MAX_VAR_DIMS);
    for(i=0;i<odom->rank;i++) {
	odom->start[i] = (start != NULL ? start[i] : 0);
	odom->count[i] = (count != NULL ? count[i]
                                        : (size != NULL ? size[i] : 1));
	odom->stride[i] = (size_t)(stride != NULL ? stride[i] : 1);
	odom->declsize[i] = (size != NULL ? size[i]
				      : (odom->start[i]+odom->count[i]));
	odom->stop[i] = odom->start[i] + odom->count[i];
	odom->index[i] = odom->start[i];
    }    
    return odom;
}

void
dapodom_free(Dapodometer* odom)
{
    if(odom) free(odom);
}

#if 0
char*
dapodom_print(Dapodometer* odom)
{
    int i;
    static char line[1024];
    char tmp[64];
    line[0] = '\0';
    if(odom->rank == 0) {
	strcat(line,"[]");
    } else for(i=0;i<odom->rank;i++) {
	sprintf(tmp,"[%lu/%lu:%lu:%lu]",
		(size_t)odom->index[i],
		(size_t)odom->start[i],
		(size_t)odom->stride[i],
		(size_t)odom->length[i]);
	strcat(line,tmp);	
    }
    return line;
}
#endif

int
dapodom_more(Dapodometer* odom)
{
    return (odom->index[0] < odom->stop[0]);
}

/* Convert current dapodometer settings to a single integer count*/
off_t
dapodom_count(Dapodometer* odom)
{
    int i;
    off_t offset = 0;
    for(i=0;i<odom->rank;i++) {
	offset *= odom->declsize[i];
	offset += odom->index[i];
    } 
    return offset;
}

int
dapodom_next(Dapodometer* odom)
{
    int i; /* do not make unsigned */
    if(odom->rank == 0) return 0; 
    for(i=odom->rank-1;i>=0;i--) {
        odom->index[i] += odom->stride[i];
        if(odom->index[i] < odom->stop[i]) break;
	if(i == 0) return 0; /* leave the 0th entry if it overflows*/
	odom->index[i] = odom->start[i]; /* reset this position*/
    }
    return 1;
}

/**************************************************/
size_t
dapodom_varmcount(Dapodometer* odom, const ptrdiff_t* steps, const size_t* declsizes)
{
    int i;
    size_t offset = 0;
    for(i=0;i<odom->rank;i++) {
	size_t tmp;
	tmp = odom->index[i];
	tmp = tmp - odom->start[i];
	tmp = tmp / odom->stride[i];
	tmp = tmp * steps[i];
	offset += tmp;
    } 
    return offset;
}

/*
Given a dapodometer, compute the total
number of elements in its space.
*/

#if 0
off_t
dapodom_space(Dapodometer* odom)
{
    size_t i;
    off_t count = 1;
    for(i=0;i<odom->rank;i++) {
	count *= odom->size[i];
    }
    return count;
}
#endif
