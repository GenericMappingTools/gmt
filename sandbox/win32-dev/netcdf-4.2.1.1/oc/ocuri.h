/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCURI_H
#define OCURI_H

/*! This is an open structure meaning
	it is ok to directly access its fields*/
typedef struct OCURI {
    char* uri;        /* as passed by the caller */
    char* protocol;
    char* user; /* from user:password@ */
    char* password; /* from user:password@ */
    char* host;	      /*!< host*/
    char* port;	      /*!< host */
    char* file;	      /*!< file */
    char* constraint; /*!< projection+selection */
    char* projection; /*!< without leading '?'*/
    char* selection;  /*!< with leading '&'*/
    char* params;     /* all params */
    char** paramlist;    /*!<null terminated list */
} OCURI;

extern int ocuriparse(const char* s, OCURI** ocuri);
extern void ocurifree(OCURI* ocuri);

/* Replace the constraints */
extern void ocurisetconstraints(OCURI*,const char* constraints);

/* Construct a complete OC URI; caller frees returned string */

/* Define flags to control what is included */
#define OCURICONSTRAINTS 1
#define OCURIUSERPWD	  2
#define OCURIPARAMS	  4

extern char* ocuribuild(OCURI*,const char* prefix, const char* suffix, int flags);


/* Param Management */
extern int ocuridecodeparams(OCURI* ocuri);
extern int ocurisetparams(OCURI* ocuri,const char*);

/*! NULL result => entry not found.
    Empty value should be represented as a zero length string */
extern const char* ocurilookup(OCURI*, const char* param);

#endif /*OCURI_H*/
