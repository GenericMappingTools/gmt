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

static void
initodometer(Odometer* odom, Dimset* dimset, size_t* startp, size_t* countp)
{
    int i;
    odom->rank = (dimset==NULL?0:dimset->ndims);
    ASSERT(odom->rank <= NC_MAX_VAR_DIMS);
    for(i=0;i<odom->rank;i++) {
	odom->declsize[i] = dimset->dimsyms[i]->dim.declsize;
	odom->unlimitedsize[i] = dimset->dimsyms[i]->dim.unlimitedsize;
	odom->start[i] = 0;
	odom->count[i] = odom->declsize[i];
	if(odom->unlimitedsize[i] > 0)odom->count[i] = odom->unlimitedsize[i];
	if(startp != NULL) {
	    odom->start[i] = startp[i];
	    odom->count[i] = countp[i];
	}
	odom->index[i] = odom->start[i];
    }    
}

Odometer*
newodometer(Dimset* dimset, size_t* start, size_t* count)
{
    Odometer* odom = (Odometer*)emalloc(sizeof(Odometer));
    initodometer(odom,dimset,start,count);
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
    if(odom->rank == 0) {
	strcat(line,"[]");
    } else for(i=0;i<odom->rank;i++) {
	sprintf(tmp,"[%lu..(%lu/%lu)]",
		odom->index[i],
		odom->unlimitedsize[i],
		odom->declsize[i]);
	strcat(line,tmp);	
    }
    return line;
}

int
odometermore(Odometer* odom)
{
    return (odom->rank > 0
            && odom->index[0] < (odom->start[0]+odom->count[0]));
}

/*
Suppose we have the declaration int F[2][5][3];
There are obviously a total of 2 X 5 X 3 = 30 integers in F.
Thus, these three dimensions will be reduced to a single
dimension of size 30 (0..29).
A particular point in the three dimensions, say [x][y][z], is reduced to
a number in the range 0..29 by computing ((x*5)+y)*3+z
*/

unsigned long
odometercount(Odometer* odom)
{
    int i;
    unsigned long count = 0;
    for(i=0;i<odom->rank;i++) {
	if(i > 0) count *= odom->count[i];
	count += odom->index[i];
    } 
    return count;
}

void
odometerreset(Odometer* odom)
{
    int rank = odom->rank;
    while(rank-- > 0) {odom->index[rank] = odom->start[rank];}
}

/*
Given an odometer compute the total
number of values it would return starting at a
given wheel position; unlimited dimensions
are treated as having size 0;
*/

size_t
odometertotal(Odometer* odom, int wheel)
{
    int i;
    unsigned long count = 1;
    for(i=wheel;i<odom->rank;i++) {
	count *= odom->count[i];
    }
    return count;
}

int
odometerincr(Odometer* odom)
{
    int i;
    int last = odom->rank-1;
    ASSERT(odom->rank > 0);
    for(i=last;i>=0;i--) {
        odom->index[i]++;
	if(odom->index[i] < (odom->start[i]+odom->count[i])) break;
	if(i == 0) break; /* leave the 0th entry if it overflows*/
	odom->index[i] = odom->start[i];
    }
    return i; /* return rightmost incremented */
}

/* compute the total n-dimensional size as 1 long array;
   stop if we encounter an unlimited dimension
*/
size_t
odomsubarray(Odometer* odom, int index)
{
    size_t result = 1;
    int i;
    for(i=index;i<odom->rank;i++) {
	/* treat unlimited as having size 1 because it will be a sublist */
	if(odom->declsize[i] == 0) break; /*unlimited*/
        result *= odom->declsize[i];
    }
    return result;
}

/* compute the total offset represented by the current wheel
   positions for indices 0 .. index.
*/
size_t
odomprefixcount(Odometer* odom, int index)
{
    size_t result = 1;
    int i;
    for(i=0;i<=index;i++) {
        result *= odom->count[i];
    }
    return result;
}

