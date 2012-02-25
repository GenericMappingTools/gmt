/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCCONTENT_H
#define OCCONTENT_H

/*! Specifies the OCcontent*/
typedef struct OCcontent {
    unsigned int magic;
    OCmode mode;
    struct OCstate* state;
    struct OCnode* node;
    struct OCtree* tree;
    size_t index; /* dim, record, or field*/
    size_t maxindex; /* 0 => max unknown */
    int    packed; /* track OC_String and OC_Byte specially*/
    struct {
	int valid; /* if the offset field contains a valid offset */
        unsigned int offset; /* location of this content in the xdr data */
    } xdrpos;
    OCmemdata* memdata; /* iff compiled */
    struct OCcontent* next;
} OCcontent;

extern struct OCcontent* ocnewcontent(struct OCstate*);
extern void ocfreecontent(struct OCstate*, struct OCcontent*);

extern int ocrootcontent(struct OCstate*, struct OCnode*, struct OCcontent*);
extern int ocarraycontent(struct OCstate*, struct OCcontent*, struct OCcontent*, size_t);
extern int ocrecordcontent(struct OCstate*, struct OCcontent*, struct OCcontent*, size_t);
extern int ocfieldcontent(struct OCstate*, struct OCcontent*, struct OCcontent*, size_t);

extern int ocgetcontent(struct OCstate*, struct OCcontent*, void* memory, size_t memsize,
                            size_t start, size_t count);


extern size_t ocarraycount(struct OCstate*, struct OCcontent*);
extern size_t ocrecordcount(struct OCstate*, struct OCcontent*);
extern size_t ocfieldcount(struct OCstate*, struct OCcontent*);

extern OCmode occontentmode(struct OCstate* conn, struct OCcontent*);
extern struct OCnode* occontentnode(struct OCstate* conn, struct OCcontent*);
extern size_t occontentindex(struct OCstate* conn, struct OCcontent*);

/* These are not really for external use.*/
extern struct OCcontent* ocresetcontent(struct OCstate*, OCcontent*);
extern struct OCcontent* occlonecontent(struct OCstate*, OCcontent*);

#endif /*OCCONTENT_H*/
