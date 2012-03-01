/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/

#ifndef DAPODOM_H
#define DAPODOM_H 1

typedef struct Dapodometer {
    int            rank;
    DCEslice        slices[NC_MAX_VAR_DIMS];
    size_t         index[NC_MAX_VAR_DIMS];
} Dapodometer;

/* Odometer operators*/
extern Dapodometer* newdapodometer(DCEslice* slices, unsigned int first, unsigned int count);

extern Dapodometer* newsimpledapodometer(struct DCEsegment*,unsigned int);

extern Dapodometer* newdapodometer1(unsigned int count);
extern Dapodometer* newdapodometer2(const size_t*, const size_t*,
                      const ptrdiff_t*, unsigned int, unsigned int);
extern Dapodometer* newdapodometer3(int, size_t*);

extern void freedapodometer(Dapodometer*);
extern char* dapodometerprint(Dapodometer* odom);

extern int dapodometermore(Dapodometer* odom);
extern int dapodometerincr(Dapodometer* odo);
extern int dapodometerincrith(Dapodometer* odo,int);
extern size_t dapodometercount(Dapodometer* odo);
extern void dapodometerreset(Dapodometer*);
extern Dapodometer* dapodometersplit(Dapodometer* odom, int tail);

extern size_t dapodometerspace(Dapodometer* odom, unsigned int wheel);
extern size_t dapodometerpoints(Dapodometer*);

extern size_t* dapodometerindices(Dapodometer*);
extern int dapodometervarmcount(Dapodometer*, const ptrdiff_t*, const size_t*);

#endif /*DAPODOM_H*/
