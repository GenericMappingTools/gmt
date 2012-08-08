/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapodom.c,v 1.12 2010/05/27 21:34:08 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "dapodom.h"

/**********************************************/
/* Define methods for a dimension dapodometer*/

Dapodometer*
newdapodometer(DCEslice* slices, unsigned int first, unsigned int rank)
{
    int i;
    Dapodometer* odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = rank;
    assert(odom->rank <= NC_MAX_VAR_DIMS);
    for(i=0;i<odom->rank;i++) {
	DCEslice* slice = slices+(first+i);
	odom->slices[i] = *slice;
	odom->index[i] = odom->slices[i].first;
    }    
    return odom;
}

Dapodometer*
newsimpledapodometer(DCEsegment* segment, unsigned int rank)
{
    int i;
    Dapodometer* odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = rank;
    assert(odom->rank <= NC_MAX_VAR_DIMS);
    assert(segment->slicesdefined && segment->slicesdeclized);
    for(i=0;i<odom->rank;i++) {
	DCEslice* odslice = &odom->slices[i];
	DCEslice* segslice = &segment->slices[i];
	odslice->first = 0;
	odslice->stride = 1;
	odslice->declsize = segslice->count;
	odslice->length = odslice->declsize;
	odslice->stop = odslice->declsize;
	odslice->count = odslice->declsize;
	odom->index[i] = 0;
    }    
    return odom;
}

Dapodometer*
newdapodometer1(unsigned int count)
{
    Dapodometer* odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = 1;
    odom->slices[0].first = 0;
    odom->slices[0].length = count;
    odom->slices[0].stride = 1;
    odom->slices[0].stop = count;
    odom->slices[0].declsize = count;
    odom->slices[0].count = count;
    odom->index[0] = 0;
    return odom;
}

void
freedapodometer(Dapodometer* odom)
{
    if(odom) free(odom);
}

char*
dapodometerprint(Dapodometer* odom)
{
    int i;
    static char line[1024];
    char tmp[64];
    line[0] = '\0';
    if(odom->rank == 0) {
	strcat(line,"[]");
    } else for(i=0;i<odom->rank;i++) {
	sprintf(tmp,"[%lu/%lu:%lu:%lu]",
		(unsigned long)odom->index[i],
		(unsigned long)odom->slices[i].first,
		(unsigned long)odom->slices[i].stride,
		(unsigned long)odom->slices[i].length);
	strcat(line,tmp);	
    }
    return line;
}

int
dapodometermore(Dapodometer* odom)
{
    return (odom->index[0] < odom->slices[0].stop);
}

void
dapodometerreset(Dapodometer* odom)
{
    int rank = odom->rank;
    while(rank-- > 0) {odom->index[rank] = odom->slices[rank].first;}
}

/* Convert current dapodometer settings to a single integer count*/
size_t
dapodometercount(Dapodometer* odom)
{
    int i;
    size_t offset = 0;
    for(i=0;i<odom->rank;i++) {
	offset *= odom->slices[i].declsize;
	offset += odom->index[i];
    } 
    return offset;
}

/*
Given a dapodometer, compute the total
number of elements in its space
as determined by declsize; start at
offset wheel
*/

size_t
dapodometerspace(Dapodometer* odom, unsigned int wheel)
{
    unsigned int i,rank = odom->rank;
    size_t count = 1;
    DCEslice* slice;
    ASSERT((wheel < rank));
    slice = odom->slices+wheel;
    for(i=wheel;i<rank;i++,slice++) {
	count *= slice->declsize;
    }
    return count;
}

/*
Compute the number of elements
that will be returned as the odometer
is incremented to its stop point.
*/

size_t
dapodometerpoints(Dapodometer* odom)
{
    unsigned int i,rank = odom->rank;
    size_t count = 1;
    DCEslice* slice = odom->slices;
    for(i=0;i<rank;i++,slice++) {
	size_t slicecount = (slice->length/slice->stride);
	count *= slicecount;
    }
    return count;
}

#ifdef IGNORE
/*
Return an dapodometer that covers the last tail
elements in the input dapodometer and removes
them from the input dapodometer.
*/
Dapodometer*
dapodometersplit(Dapodometer* odom, int tail)
{
    int i;
    Dapodometer* split = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(split,NULL);
    assert(odom->rank >= tail);
    split->rank = tail;
    odom->rank = odom->rank - tail;
    for(i=0;i<tail;i++) {split->slices[i] = odom->slices[odom->rank+i];}
    return split;
}
#endif

int
dapodometerincr(Dapodometer* odom)
{
    return dapodometerincrith(odom,-1);
}

int
dapodometerincrith(Dapodometer* odom, int wheel)
{
    int i; /* do not make unsigned */
    DCEslice* slice;
    if(odom->rank == 0) return 0; 
    if(wheel < 0) wheel = (odom->rank - 1);
    for(slice=odom->slices+(wheel),i=wheel;i>=0;i--,slice--) {
        odom->index[i] += slice->stride;
        if(odom->index[i] < slice->stop) break;
	if(i == 0) return 0; /* leave the 0th entry if it overflows*/
	odom->index[i] = slice->first; /* reset this position*/
    }
    return 1;
}

/**************************************************/
int
dapodometervarmcount(Dapodometer* odom, const ptrdiff_t* steps, const size_t* declsizes)
{
    int i;
    size_t offset = 0;
    for(i=0;i<odom->rank;i++) {
	size_t tmp;
	tmp = odom->index[i];
	tmp = tmp - odom->slices[i].first;
	tmp = tmp / odom->slices[i].stride;
	tmp = tmp * steps[i];
	offset += tmp;
    } 
    return offset;
}


/* Return the current set of indices */
size_t*
dapodometerindices(Dapodometer* odom)
{
    if(odom == NULL) return NULL;
    return odom->index;
}

Dapodometer*
newdapodometer2(const size_t* start, const size_t* count, const ptrdiff_t* stride,
	        unsigned int first, unsigned int rank)
{
    int i;
    Dapodometer* odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = rank;
    assert(odom->rank <= NC_MAX_VAR_DIMS);
    for(i=0;i<odom->rank;i++) {
	odom->slices[i].first = start[first+i];
	odom->slices[i].stride = (size_t)stride[first+i];
	odom->slices[i].length = count[first+i] * stride[first+i];
	odom->slices[i].stop = (odom->slices[i].first+odom->slices[i].length);
	odom->slices[i].declsize = odom->slices[i].stop;
	odom->slices[i].count = (odom->slices[i].length /odom->slices[i].stride);
	odom->index[i] = odom->slices[i].first;
    }    
    return odom;
}

Dapodometer*
newdapodometer3(int rank, size_t* dimsizes)
{
    int i;
    Dapodometer* odom = (Dapodometer*)calloc(1,sizeof(Dapodometer));
    MEMCHECK(odom,NULL);
    odom->rank = rank;
    for(i=0;i<rank;i++) {
        odom->slices[i].first = 0;
        odom->slices[i].length = dimsizes[i];
        odom->slices[i].stride = 1;
        odom->slices[i].stop = dimsizes[i];
        odom->slices[i].declsize = dimsizes[i];
	odom->slices[i].count = (odom->slices[i].length / odom->slices[i].stride);
        odom->index[i] = 0;
    }
    return odom;
}
