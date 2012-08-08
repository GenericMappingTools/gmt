/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef ODOM_H
#define ODOM_H 1

typedef struct Odometer {
    int rank;
    int offset;
    struct Odometer* origin;
    size_t    start[NC_MAX_VAR_DIMS];
    size_t    count[NC_MAX_VAR_DIMS];
    size_t    index[NC_MAX_VAR_DIMS];
    size_t declsize[NC_MAX_VAR_DIMS];
} Odometer;

/*Forward*/
struct Dimset;

/* Odometer operators*/
extern Odometer* newodometer(struct Dimset*, size_t* startp, size_t* countp);
extern Odometer* newsubodometer(Odometer*, struct Dimset*, int, int);

extern Odometer* newsubodometer(Odometer*, struct Dimset*,
			        int start, int stop);

extern void odometerfree(Odometer*);
extern char* odometerprint(Odometer* odom);

extern int odometermore(Odometer* odom);
extern int odometerincr(Odometer* odo);
extern size_t odometeroffset(Odometer* odom);

#endif /*ODOM_H*/
