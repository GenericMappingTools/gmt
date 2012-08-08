/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapalign.h,v 1.3 2009/09/23 22:26:00 dmh Exp $
 *********************************************************************/
#ifndef ALIGN_H
#define ALIGN_H 1

typedef struct NCtypealignment {
    char* typename;
    int alignment;
} NCtypealignment;

/* Define indices for every primitive C type */
/* NAT => NOT-A-TYPE*/
#define NCCTYPENAT       0
#define NCCTYPECHAR      1
#define NCCTYPEUCHAR     2
#define NCCTYPESHORT     3
#define NCCTYPEUSHORT    4
#define NCCTYPEINT       5
#define NCCTYPEUINT      6
#define NCCTYPELONG      7
#define NCCTYPEULONG     8
#define NCCTYPELONGLONG  9
#define NCCTYPEULONGLONG 10
#define NCCTYPEFLOAT     11
#define NCCTYPEDOUBLE    12
#define NCCTYPEPTR       13
#define NCCTYPENCVLEN    14

/* Capture in struct and in a vector*/
typedef struct NCtypealignset {
    NCtypealignment charalign;	  /* char*/
    NCtypealignment ucharalign;	  /* unsigned char*/
    NCtypealignment shortalign;	  /* short*/
    NCtypealignment ushortalign;	  /* unsigned short*/
    NCtypealignment intalign;	  /* int*/
    NCtypealignment uintalign;	  /* unsigned int*/
    NCtypealignment longalign;	  /* long*/
    NCtypealignment ulongalign;	  /* unsigned long*/
    NCtypealignment longlongalign;  /* long long*/
    NCtypealignment ulonglongalign; /* unsigned long long*/
    NCtypealignment floatalign;	  /* float*/
    NCtypealignment doublealign;	  /* double*/
    NCtypealignment ptralign;	  /* void**/
    NCtypealignment ncvlenalign;	  /* nc_vlen_t*/
} NCtypealignset;

typedef NCtypealignment NCtypealignvec;

extern void compute_nccalignments(void);
extern unsigned int ncctypealignment(int nctype);
extern int nccpadding(unsigned long offset, int alignment);

#endif /*ALIGN_H*/
