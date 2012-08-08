/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCDUMP_H
#define OCDUMP_H

extern void ocdumpnode(OCnode* node);

extern void ocdumpslice(OCslice* slice);
extern void ocdumpclause(OCprojectionclause* ref);

extern void ocdumpmemory(char* memory, int len, int bod);
extern void ocdumppacket(char* memory, int len, int bod);

extern void ocdumpfile(FILE* file, int datastart);

extern void ocdumpmemdata(OCmemdata*,OCbytes*);

extern void ocdd(OCstate*,OCnode*);

#endif /*OCDUMP_H*/
