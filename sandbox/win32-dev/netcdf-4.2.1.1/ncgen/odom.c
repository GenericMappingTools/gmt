/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/odom.c,v 1.5 2010/05/27 21:34:18 dmh Exp $ */

#include "includes.h"
#include "odom.h"

/**************************************************/
/* Define methods for a dimension odometer*/

Odometer*
newodometer(Dimset* dimset, size_t* startp, size_t* countp)
{
    int i;
    Odometer* odom;
    ASSERT(dimset != NULL);
    ASSERT(dimset->ndims > 0);
    odom = (Odometer*)emalloc(sizeof(Odometer));
    if(odom == NULL) return NULL;
    odom->origin = odom;
    odom->offset = 0;
    odom->rank = dimset->ndims;
    ASSERT(odom->rank <= NC_MAX_VAR_DIMS);
    for(i=0;i<odom->rank;i++) {
	odom->declsize[i] = dimset->dimsyms[i]->dim.declsize;
	odom->start[i] = (startp == NULL ? 0
                                         : startp[i]);
	odom->count[i] = (countp == NULL ? odom->declsize[i]
                                         : countp[i]);
	odom->index[i] = odom->start[i];
	/* verify */
	ASSERT(odom->start[i] + odom->count[i] <= odom->declsize[i]);
    }    
    return odom;
}

Odometer*
newsubodometer(Odometer* origin, Dimset* dimset, int start, int stop)
{
    Odometer* odom;
    ASSERT(dimset != NULL);
    ASSERT(dimset->ndims > 0 && dimset->ndims >= stop);
    ASSERT(stop > start);
    odom = (Odometer*)emalloc(sizeof(Odometer));
    if(odom == NULL) return NULL;
    odom->origin = origin;
    odom->offset = start;
    odom->rank = (stop - start);
    ASSERT(odom->rank <= NC_MAX_VAR_DIMS);
    return odom;
}

void
odometerfree(Odometer* odom)
{
    if(odom) efree(odom);
}

char*
odometerprint(Odometer* odom)
{
    int i;
    static char line[1024];
    char tmp[64];
    line[0] = '\0';
    if(odom->origin->rank == 0) {
	strcat(line,"[]");
    } else for(i=0;i<odom->rank;i++) {
	int ioffset = i + odom->offset;
	sprintf(tmp,"[%lu/%lu..%lu:%lu]",
		odom->origin->index[ioffset],
		odom->origin->start[ioffset],
		odom->origin->declsize[ioffset],
		odom->origin->count[ioffset]
               );
	strcat(line,tmp);	
    }
    return line;
}

int
odometermore(Odometer* odom)
{
    size_t index,start,count;
    int offset = odom->offset;
    ASSERT(odom->rank > 0);
    index = odom->origin->index[offset];
    start = odom->origin->start[offset];
    count = odom->origin->count[offset];
    if(index < start + count) return 1;
    /* reset the zero'th wheel before returning */
    odom->origin->index[offset] = odom->origin->start[offset];
    return 0;    
}

int
odometerincr(Odometer* odom)
{
    int i;
    int last = odom->rank-1;
    ASSERT(odom->rank > 0);
    for(i=last;i>=0;i--) {
	int ioffset = i+odom->offset;
        odom->origin->index[ioffset]++;
	if(odom->origin->index[ioffset]
           < (odom->origin->start[ioffset]
              +odom->origin->count[ioffset]))
	    break;
	if(i==0) break; /* leave 0'th for next more() check */
	odom->origin->index[ioffset] = odom->origin->start[ioffset];
    }
    return i; /* return rightmost incremented */
}


/*
Suppose we have the declaration int F[2][5][3];
There are obviously a total of 2 X 5 X 3 = 30 integers in F.
Thus, these three dimensions will be reduced to a single
dimension of size 30 (0..29).
A particular point in the three dimensions, say [x][y][z], is reduced to
a number in the range 0..29 by computing ((x*5)+y)*3+z
*/

size_t
odometeroffset(Odometer* odom)
{
    int i;
    size_t count = 0;
    for(i=0;i<odom->rank;i++) {
	int ioffset = i+odom->offset;
	if(i > 0) count *= odom->origin->declsize[ioffset];
	count += odom->origin->index[ioffset];
    } 
    return count;
}
