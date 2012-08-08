/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCDUMP_H
#define OCDUMP_H

extern void ocdumpnode(OCnode* node);

extern void ocdumpslice(OCslice* slice);
extern void ocdumpclause(OCprojectionclause* ref);

extern void ocdumpmemory(char* memory, size_t len, int xdrencoded, int level);

extern void ocdd(OCstate*,OCnode*,int xdrencoded, int level);

extern void ocdumpdata(OCstate*,OCdata*,OCbytes*,int);

extern void ocdumpdatatree(OCstate*, struct OCdata*, OCbytes* buffer, int depth);
extern void ocdumpdatapath(OCstate*, struct OCdata*, OCbytes* buffer);

#endif /*OCDUMP_H*/
