/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef HTTP_H
#define HTTP_H 1

extern int curlopen(CURL** curlp);
extern void curlclose(CURL*);

extern int ocfetchurl(CURL*, char*, OCbytes*, long*);
extern int ocfetchurl_file(CURL*, char*, FILE*, unsigned long*, long*);

extern long ocfetchhttpcode(CURL* curl);

extern int ocfetchlastmodified(CURL* curl, char* url, long* filetime);

extern int occurlopen(CURL** curlp);
extern void occurlclose(CURL* curlp);

#endif /*HTTP_H*/
