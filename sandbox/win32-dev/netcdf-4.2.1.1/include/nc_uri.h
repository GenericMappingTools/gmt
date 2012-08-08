/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef NC_URI_H
#define NC_URI_H

/*! This is an open structure meaning
	it is ok to directly access its fields*/
typedef struct NC_URI {
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
} NC_URI;

extern int nc_uriparse(const char* s, NC_URI** nc_uri);
extern void nc_urifree(NC_URI* nc_uri);

/* Replace the constraints */
extern void nc_urisetconstraints(NC_URI*,const char* constraints);

/* Construct a complete NC_ URI; caller frees returned string */

/* Define flags to control what is included */
#define NC_URICONSTRAINTS	1
#define NC_URIUSERPWD	  	2
#define NC_URIPARAMS	  	4
#define NC_URIALL	  	(NC_URICONSTRAINTS|NC_URIUSERPWD|NC_URIPARAMS)

extern char* nc_uribuild(NC_URI*,const char* prefix, const char* suffix, int flags);


/* Param Management */
extern int nc_uridecodeparams(NC_URI* nc_uri);
extern int nc_urisetparams(NC_URI* nc_uri,const char*);

/*! 0 result => entry not found; 1=>found; result holds value (may be null).
    In any case, the result is imutable and should not be free'd.
*/
extern int nc_urilookup(NC_URI*, const char* param, const char** result);

#endif /*NC_URI_H*/
