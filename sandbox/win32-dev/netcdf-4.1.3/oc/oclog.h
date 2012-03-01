/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCLOG_H
#define OCLOG_H

#define LOGNOTE 0
#define LOGWARN 1
#define LOGERR 2
#define LOGDBG 3

extern void oc_loginit(void);
extern void oc_setlogging(int tf);
extern void oc_logopen(const char* file);
extern void oc_logclose(void);

extern void oc_log(int tag, const char* fmt, ...);
extern void oc_logtext(int tag, const char* text);

#endif /*OCLOG_H*/
