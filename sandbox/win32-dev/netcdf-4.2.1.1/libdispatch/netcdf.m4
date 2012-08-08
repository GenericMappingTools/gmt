/*
 Copyright 2010 University Corporation for Atmospheric
 Research/Unidata. See COPYRIGHT file for more info.

 This file defines most of the netcdf API in terms of the dispatch
 functions along with a few functions that are overlays over the
 dispatch functions.

 "$Id$" 
*/

#include "ncdispatch.h"
#define INITCOORD1 if(coord_one[0] != 1) {int i; for(i=0;i<NC_MAX_VAR_DIMS;i++) coord_one[i] = 1;}

static nc_type longtype = (sizeof(long) == sizeof(int)?NC_INT:NC_INT64);
/*static nc_type ulongtype = (sizeof(unsigned long) == sizeof(unsigned int)?NC_UINT:NC_UINT64);*/

NC_Dispatch* NC3_dispatch_table = NULL;

#ifdef USE_NETCDF4
NC_Dispatch* NC4_dispatch_table = NULL;
#endif

#ifdef USE_DAP
NC_Dispatch* NCD3_dispatch_table = NULL;
#endif

#if defined(USE_DAP) && defined(USE_NETCDF4)
NC_Dispatch* NCD4_dispatch_table = NULL;
#endif


#ifndef NC_ENOTNC4
#define NC_ENOTNC4 (-111)
#endif

#ifndef X_INT_MAX
#define  X_INT_MAX 2147483647
#endif


/**************************************************/

int
NC_testurl(const char* path)
{
#ifdef USE_DAP
    void* tmpurl = NULL;
    if(NCDAP_urlparse(path,&tmpurl) == NC_NOERR) {
	NCDAP_urlfree(tmpurl);
	return 1;
    }
#endif
    return 0;
}

int
NC_urlmodel(const char* path)
{
    int model = 0;
#ifdef USE_DAP
    void* tmpurl = NULL;
    if(NCDAP_urlparse(path,&tmpurl) == NC_NOERR) {
	if(NCDAP_urllookup(tmpurl,"netcdf4")
	   || NCDAP_urllookup(tmpurl,"netcdf-4")) {
	    model = 4;
	} else if(NCDAP_urllookup(tmpurl,"netcdf3")
	   || NCDAP_urllookup(tmpurl,"netcdf-3")) {
	    model = 3;
	} else {
	    model = 0;
	}
	NCDAP_urlfree(tmpurl);
    }
#endif
    return model;
}

/* Override dispatch table management */
static NC_Dispatch* NC_dispatch_override = NULL;

/* Override dispatch table management */
NC_Dispatch*
NC_get_dispatch_override(void) {
    return NC_dispatch_override;
}

void NC_set_dispatch_override(NC_Dispatch* d)
{
    NC_dispatch_override = d;
}

/**************************************************/
/* Wrapper */


/* Overlay by treating the tables as arrays of void*.
   Overlay rules are:
        overlay    base    merge
        -------    ----    -----
          null     null     null
          null      y        y
           x       null      x
           x        y        x
*/

int
NC_dispatch_overlay(const NC_Dispatch* overlay, const NC_Dispatch* base, NC_Dispatch* merge)
{
    void** voverlay = (void**)overlay;
    void** vmerge;
    int i, count = sizeof(NC_Dispatch) / sizeof(void*);
    /* dispatch table must be exact multiple of sizeof(void*) */
    assert(count * sizeof(void*) == sizeof(NC_Dispatch));
    *merge = *base;
    vmerge = (void**)merge;
    for(i=0;i<count;i++) {
        if(voverlay[i] == NULL) continue;
        vmerge[i] = voverlay[i];
    }
    /* Finally, the merge model should always be the overlay model */
    merge->model = overlay->model;
    return NC_NOERR;
}

/**************************************************/
/* Output type specific interface */

/* Public */

/**************************************************/

/**************************************************/

#ifdef USE_DAP

/* allow access to nc_urlparse and params while minimizing exposing nc.h */
int
NCDAP_urlparse(const char* s, void** dapurlp)
{
    DAPURL* dapurl = NULL;
    dapurl = calloc(1,sizeof(DAPURL));
    if(dapurl == 0) return NC_ENOMEM;
    if(!dapurlparse(s,dapurl)) {
	dapurlclear(dapurl);
	free(dapurl);
	return NC_EINVAL;
    }
    if(dapurlp) *dapurlp = dapurl;
    return NC_NOERR;
}

void
NCDAP_urlfree(void* durl)
{
    DAPURL* dapurl = (DAPURL*)durl;
    if(dapurl != NULL) {
	dapurlclear(dapurl);
	free(dapurl);
    }
}

const char*
NCDAP_urllookup(void* durl, const char* param)
{
    DAPURL* dapurl = (DAPURL*)durl;
    if(param == NULL || strlen(param) == 0 || dapurl == NULL) return NULL;
    return dapurllookup(dapurl,param);
}

#else /*!USE_DAP*/
int
NCDAP_urlparse(const char* s, void** dapurlp)
{
    return NC_EINVAL;
}

void
NCDAP_urlfree(void* durl)
{
    return;
}

const char*
NCDAP_urllookup(void* durl, const char* param)
{
    return NULL;
}

#endif /*!USE_DAP*/

/**************************************************/
/* M4 generated */
dnl
dnl NCGETVAR1(Abbrev, Type)
dnl
define(`NCGETVAR1',dnl
`dnl
int
nc_get_var1_$1(int ncid, int varid, const size_t *coord, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    INITCOORD1;
    return NC_get_var1(ncid,varid,coord,(void*)value,T_$1);
}
')dnl
NCGETVAR1(text,char)
NCGETVAR1(schar,signed char)
NCGETVAR1(uchar,unsigned char)
NCGETVAR1(short,short)
NCGETVAR1(int,int)
NCGETVAR1(long,long)
dnl NCGETVAR1(ulong,ulong)
NCGETVAR1(float,float)
NCGETVAR1(double,double)
NCGETVAR1(ubyte,unsigned char)
NCGETVAR1(ushort,unsigned short)
NCGETVAR1(uint,unsigned int)
NCGETVAR1(longlong,long long)
NCGETVAR1(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVAR1(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVAR1(Abbrev, Type)
dnl
define(`NCPUTVAR1',dnl
`dnl
int
nc_put_var1_$1(int ncid, int varid, const size_t *coord, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    INITCOORD1;
    return NC_put_var1(ncid,varid,coord,(void*)value,T_$1);
}
')dnl
NCPUTVAR1(text,char)
NCPUTVAR1(schar,signed char)
NCPUTVAR1(uchar,unsigned char)
NCPUTVAR1(short,short)
NCPUTVAR1(int,int)
NCPUTVAR1(long,long)
dnl NCPUTVAR1(ulong,ulong)
NCPUTVAR1(float,float)
NCPUTVAR1(double,double)
NCPUTVAR1(ubyte,unsigned char)
NCPUTVAR1(ushort,unsigned short)
NCPUTVAR1(uint,unsigned int)
NCPUTVAR1(longlong,long long)
NCPUTVAR1(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVAR1(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVAR(Abbrev, Type)
dnl
define(`NCGETVAR',dnl
`dnl
int
nc_get_var_$1(int ncid, int varid, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_var(ncid,varid,(void*)value,T_$1);
}
')dnl
NCGETVAR(text,char)
NCGETVAR(schar,signed char)
NCGETVAR(uchar,unsigned char)
NCGETVAR(short,short)
NCGETVAR(int,int)
NCGETVAR(long,long)
dnl NCGETVAR(ulong,ulong)
NCGETVAR(float,float)
NCGETVAR(double,double)
NCGETVAR(ubyte,unsigned char)
NCGETVAR(ushort,unsigned short)
NCGETVAR(uint,unsigned int)
NCGETVAR(longlong,long long)
NCGETVAR(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVAR(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVAR(Abbrev, Type)
dnl
define(`NCPUTVAR',dnl
`dnl
int
nc_put_var_$1(int ncid, int varid, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_var(ncid,varid,(void*)value,T_$1);
}
')dnl
NCPUTVAR(text,char)
NCPUTVAR(schar,signed char)
NCPUTVAR(uchar,unsigned char)
NCPUTVAR(short,short)
NCPUTVAR(int,int)
NCPUTVAR(long,long)
dnl NCPUTVAR(ulong,ulong)
NCPUTVAR(float,float)
NCPUTVAR(double,double)
NCPUTVAR(ubyte,unsigned char)
NCPUTVAR(ushort,unsigned short)
NCPUTVAR(uint,unsigned int)
NCPUTVAR(longlong,long long)
NCPUTVAR(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVAR(string,char*)
#endif /*USE_NETCDF4*/


dnl
dnl NCPUTVARA(Abbrv, Type)
dnl
define(`NCPUTVARA',dnl
`dnl
int
nc_put_vara_$1(int ncid, int varid,
	 const size_t *start, const size_t *edges, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_vara(ncid,varid,start,edges,(void*)value,T_$1);
}
')dnl
NCPUTVARA(text,char)
NCPUTVARA(schar,signed char)
NCPUTVARA(uchar,unsigned char)
NCPUTVARA(short,short)
NCPUTVARA(int,int)
NCPUTVARA(long,long)
dnl NCPUTVARA(ulong,ulong)
NCPUTVARA(float,float)
NCPUTVARA(double,double)
NCPUTVARA(ubyte,unsigned char)
NCPUTVARA(ushort,unsigned short)
NCPUTVARA(uint,unsigned int)
NCPUTVARA(longlong,long long)
NCPUTVARA(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVARA(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVARA(Abbrv, Type)
dnl
define(`NCGETVARA',dnl
`dnl
int
nc_get_vara_$1(int ncid, int varid,
	 const size_t *start, const size_t *edges, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_vara(ncid,varid,start,edges,(void*)value,T_$1);
}
')dnl
NCGETVARA(text,char)
NCGETVARA(schar,signed char)
NCGETVARA(uchar,unsigned char)
NCGETVARA(short,short)
NCGETVARA(int,int)
NCGETVARA(long,long)
dnl NCGETVARA(ulong,ulong)
NCGETVARA(float,float)
NCGETVARA(double,double)
NCGETVARA(ubyte,unsigned char)
NCGETVARA(ushort,unsigned short)
NCGETVARA(uint,unsigned int)
NCGETVARA(longlong,long long)
NCGETVARA(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVARA(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVARM(Abbrv, Type)
dnl
define(`NCPUTVARM',dnl
`dnl
int
nc_put_varm_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride, const ptrdiff_t * imapp,
            const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_varm(ncid,varid,start,edges,stride,imapp,(void*)value,T_$1);
}
')dnl
NCPUTVARM(text,char)
NCPUTVARM(schar,signed char)
NCPUTVARM(uchar,unsigned char)
NCPUTVARM(short,short)
NCPUTVARM(int,int)
NCPUTVARM(long,long)
dnl NCPUTVARM(ulong,ulong)
NCPUTVARM(float,float)
NCPUTVARM(double,double)
NCPUTVARM(ubyte,unsigned char)
NCPUTVARM(ushort,unsigned short)
NCPUTVARM(uint,unsigned int)
NCPUTVARM(longlong,long long)
NCPUTVARM(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVARM(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVARM(Abbrv, Type)
dnl
define(`NCGETVARM',dnl
`dnl
int
nc_get_varm_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride, const ptrdiff_t * imapp,
            $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_varm(ncid,varid,start,edges,stride,imapp,(void*)value,T_$1);
}
')dnl
NCGETVARM(text,char)
NCGETVARM(schar,signed char)
NCGETVARM(uchar,unsigned char)
NCGETVARM(short,short)
NCGETVARM(int,int)
NCGETVARM(long,long)
dnl NCGETVARM(ulong,ulong)
NCGETVARM(float,float)
NCGETVARM(double,double)
NCGETVARM(ubyte,unsigned char)
NCGETVARM(ushort,unsigned short)
NCGETVARM(uint,unsigned int)
NCGETVARM(longlong,long long)
NCGETVARM(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVARM(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCPUTVARS(Abbrv, Type)
dnl
define(`NCPUTVARS',dnl
`dnl
int
nc_put_vars_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride,
            const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_put_vars(ncid,varid,start,edges,stride,(void*)value,T_$1);
}
')dnl
NCPUTVARS(text,char)
NCPUTVARS(schar,signed char)
NCPUTVARS(uchar,unsigned char)
NCPUTVARS(short,short)
NCPUTVARS(int,int)
NCPUTVARS(long,long)
dnl NCPUTVARS(ulong,ulong)
NCPUTVARS(float,float)
NCPUTVARS(double,double)
NCPUTVARS(ubyte,unsigned char)
NCPUTVARS(ushort,unsigned short)
NCPUTVARS(uint,unsigned int)
NCPUTVARS(longlong,long long)
NCPUTVARS(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCPUTVARS(string,char*)
#endif /*USE_NETCDF4*/

dnl
dnl NCGETVARS(Abbrv, Type)
dnl
define(`NCGETVARS',dnl
`dnl
int
nc_get_vars_$1(int ncid, int varid,
	    const size_t *start, const size_t *edges,
	    const ptrdiff_t * stride,
            $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return NC_get_vars(ncid,varid,start,edges,stride,(void*)value,T_$1);
}
')dnl
NCGETVARS(text,char)
NCGETVARS(schar,signed char)
NCGETVARS(uchar,unsigned char)
NCGETVARS(short,short)
NCGETVARS(int,int)
NCGETVARS(long,long)
dnl NCGETVARS(ulong,ulong)
NCGETVARS(float,float)
NCGETVARS(double,double)
NCGETVARS(ubyte,unsigned char)
NCGETVARS(ushort,unsigned short)
NCGETVARS(uint,unsigned int)
NCGETVARS(longlong,long long)
NCGETVARS(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NCGETVARS(string,char*)
#endif /*USE_NETCDF4*/


dnl
dnl NC_GET_ATT(Abbrv, Type)
dnl
define(`NC_GET_ATT',dnl
`dnl
int
nc_get_att_$1(int ncid, int varid, const char *name, $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_att(ncid,varid,name,(void*)value,T_$1);
}
')dnl
NC_GET_ATT(text,char)
NC_GET_ATT(schar, signed char)
NC_GET_ATT(uchar, unsigned char)
NC_GET_ATT(short, short)
NC_GET_ATT(int, int)
NC_GET_ATT(long,long)
dnl NC_GET_ATT(ulong,ulong)
NC_GET_ATT(float, float)
NC_GET_ATT(double, double)
NC_GET_ATT(ubyte,unsigned char)
NC_GET_ATT(ushort,unsigned short)
NC_GET_ATT(uint,unsigned int)
NC_GET_ATT(longlong,long long)
NC_GET_ATT(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
NC_GET_ATT(string,char*)
#endif /*USE_NETCDF4*/


dnl
dnl NC_PUT_ATT(Abbrv, Type)
dnl
define(`NC_PUT_ATT',dnl
`dnl
int
nc_put_att_$1(int ncid, int varid, const char *name,
	nc_type type, size_t nelems, const $2 *value)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_att(ncid,varid,name,type,nelems,(void*)value,T_$1);
}
')dnl
NC_PUT_ATT(schar, signed char)
NC_PUT_ATT(uchar, unsigned char)
NC_PUT_ATT(short, short)
NC_PUT_ATT(int, int)
NC_PUT_ATT(long,long)
dnl NC_PUT_ATT(ulong,ulong)
NC_PUT_ATT(float, float)
NC_PUT_ATT(double, double)
NC_PUT_ATT(ubyte,unsigned char)
NC_PUT_ATT(ushort,unsigned short)
NC_PUT_ATT(uint,unsigned int)
NC_PUT_ATT(longlong,long long)
NC_PUT_ATT(ulonglong,unsigned long long)
#ifdef USE_NETCDF4
/*NC_PUT_ATT(string,char*) defined separately */
#endif /*USE_NETCDF4*/

