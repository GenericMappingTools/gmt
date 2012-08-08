/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCBYTES_H
#define OCBYTES_H 1

typedef struct OCbytes {
  int nonextendible; /* 1 => fail if an attempt is made to extend this buffer*/
  unsigned int alloc;
  unsigned int length;
  char* content;
} OCbytes;

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__) || defined(__CPLUSPLUS)
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

EXTERNC OCbytes* ocbytesnew(void);
EXTERNC void ocbytesfree(OCbytes*);
EXTERNC int ocbytessetalloc(OCbytes*,unsigned int);
EXTERNC int ocbytessetlength(OCbytes*,unsigned int);
EXTERNC int ocbytesfill(OCbytes*, char fill);

/* Produce a duplicate of the contents*/
EXTERNC char* ocbytesdup(OCbytes*);
/* Extract the contents and leave buffer empty */
EXTERNC char* ocbytesextract(OCbytes*);

/* Return the ith byte; -1 if no such index */
EXTERNC int ocbytesget(OCbytes*,unsigned int);
/* Set the ith byte */
EXTERNC int ocbytesset(OCbytes*,unsigned int,char);

/* Append one byte */
EXTERNC int ocbytesappend(OCbytes*,char); /* Add at Tail */
/* Append n bytes */
EXTERNC int ocbytesappendn(OCbytes*,void*,unsigned int); /* Add at Tail */

/* Concatenate a null-terminated string to the end of the buffer */
EXTERNC int ocbytescat(OCbytes*,char*);
/* Set the contents of the buffer; mark the buffer as non-extendible */
EXTERNC int ocbytessetcontents(OCbytes*, char*, unsigned int);

/* Following are always "in-lined"*/
#define ocbyteslength(bb) ((bb)?(bb)->length:0U)
#define ocbytesalloc(bb) ((bb)?(bb)->alloc:0U)
#define ocbytescontents(bb) ((bb && bb->content)?(bb)->content:(char*)"")
#define ocbytesextend(bb,len) ocbytessetalloc((bb),(len)+(bb->alloc))
#define ocbytesclear(bb) ((bb)?(bb)->length=0:0U)
#define ocbytesavail(bb,n) ((bb)?((bb)->alloc - (bb)->length) >= (n):0U)

#endif /*OCBYTES_H*/
