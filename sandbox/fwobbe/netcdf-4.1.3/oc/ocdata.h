/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCDATA_H
#define OCDATA_H

/* Temporary aliases */
#define Emptymode OCEMPTYMODE
#define Nullmode OCNULLMODE
#define Dimmode OCARRAYMODE
#define Recordmode OCRECORDMODE
#define Fieldmode OCFIELDMODE
#define Datamode OCSCALARMODE

typedef struct OCdimcounter {
    int rank;
    size_t index[OC_MAX_DIMS];
    size_t size[OC_MAX_DIMS];
} OCdimcounter;

extern const char StartOfoclist;
extern const char EndOfoclist;

/* Skip arbitrary dimensioned instance; Handles dimensioning.*/
extern int ocskip(OCnode* node, XDR* xdrs);

/* Skip arbitrary single instance; except for primitives
   Assumes that parent will handle arrays of compound instances
   or records of compound instances of this node type*/
extern int ocskipinstance(OCnode* node, XDR* xdrs);

extern int occountrecords(OCnode* node, XDR* xdrs, size_t* nrecordsp);

extern int ocxdrread(XDR*, char* memory, size_t, int packed, OCtype, unsigned int index, size_t count);

#endif /*OCDATA_H*/
