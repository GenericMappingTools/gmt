/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
  See the COPYRIGHT file for more information. */

#ifndef OCCLIENTPARAMS_H
#define OCCLIENTPARAMS_H

extern OClist* ocparamdecode(OCstate*);
extern const char* ocparamlookup(OCstate*, const char*);
extern void ocparamset(OCstate*,const char*);

#endif /*OCCLIENTPARAMS_H*/
