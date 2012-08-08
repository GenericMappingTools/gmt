/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef HTTP_H
#define HTTP_H 1

extern int curlopen(CURL** curlp);
extern void curlclose(CURL*);

extern int ocfetchurl(CURL*, const char*, OCbytes*, long*);
extern int ocfetchurl_file(CURL*, const char*, FILE*, off_t*, long*);

extern long ocfetchhttpcode(CURL* curl);

extern int ocfetchlastmodified(CURL* curl, char* url, long* filetime);

extern int occurlopen(CURL** curlp);
extern void occurlclose(CURL* curlp);

extern int ocping(const char* url);

#endif /*HTTP_H*/
