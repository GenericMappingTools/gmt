/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/odom.h,v 1.5 2010/05/27 21:34:18 dmh Exp $ */

#ifndef ODOM_H
#define ODOM_H 1

typedef struct Odometer {
    int rank;
    int wholepoint; /*point where whole dimensions are being used; -1=>none*/
    size_t index[NC_MAX_VAR_DIMS];
    size_t start[NC_MAX_VAR_DIMS];
    size_t count[NC_MAX_VAR_DIMS];
    size_t unlimitedsize[NC_MAX_VAR_DIMS];
    size_t declsize[NC_MAX_VAR_DIMS]; /* 0 => unlimited */
} Odometer;

struct Dimset; /* forward */

/* Odometer operators*/
extern Odometer* newodometer(struct Dimset*, size_t* startp, size_t* countp);
extern void odometerfree(Odometer*);
extern char* odometerprint(Odometer* odom);
extern int odomupdate(Odometer*, size_t* startp, size_t* countp);

extern int odometermore(Odometer* odom);
extern int odometerincr(Odometer* odo);
extern unsigned long odometercount(Odometer* odo);
extern void odometerreset(Odometer*);
extern size_t odometertotal(Odometer*,int);
extern size_t odomsubarray(Odometer* odom, int index);
extern size_t odomprefixcount(Odometer* odom, int index);

#endif /*ODOM_H*/
