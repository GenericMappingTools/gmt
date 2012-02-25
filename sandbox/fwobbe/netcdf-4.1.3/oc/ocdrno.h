/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCDRNO_H
#define OCDRNO_H
/*
This file exports procedures
that access the internals of
oc. They are intended to be called
by the drno code to avoid at least
the appearance of breaking the oc
encapsulation. 
*/

/* DO NOT FREE THE RETURNED STRINGS */
extern OCerror ocdaperrorcode(OCconnection,char**,char**,long*);

#endif /*OCDRNO_H*/
