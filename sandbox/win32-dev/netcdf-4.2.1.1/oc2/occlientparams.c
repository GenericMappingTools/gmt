/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ocinternal.h"
#include "ocdebug.h"

#define LBRACKET '['
#define RBRACKET ']'



/*
Client parameters are assumed to be
one or more instances of bracketed pairs:
e.g "[...][...]...".
The bracket content in turn is assumed to be a
comma separated list of <name>=<value> pairs.
e.g. x=y,z=,a=b.
If the same parameter is specifed more than once,
then the first occurrence is used; this is so that
is possible to forcibly override user specified
parameters by prefixing.
IMPORTANT: client parameter string is assumed to
have blanks compress out.
*/

int
ocparamdecode(OCstate* state)
{
    int i;
    i = ocuridecodeparams(state->uri);
    return i?OC_NOERR:OC_EBADURL;
}


const char*
ocparamlookup(OCstate* state, const char* key)
{
    if(state == NULL || key == NULL || state->uri == NULL) return NULL;
    return ocurilookup(state->uri,key);
}

int
ocparamset(OCstate* state, const char* params)
{
    int i;
    i = ocurisetparams(state->uri,params);
    return i?OC_NOERR:OC_EBADURL;
}

#ifdef OCIGNORE
void
ocparamfree(OClist* params)
{
    int i;
    if(params == NULL) return;
    for(i=0;i<oclistlength(params);i++) {
	char* s = (char*)oclistget(params,i);
	if(s != NULL) free((void*)s);
    }
    oclistfree(params);
}

/*
Delete the entry.
return value = 1 => found and deleted;
               0 => param not found
*/
int
ocparamdelete(OClist* params, const char* clientparam)
{
    int i,found = 0;
    if(params == NULL || clientparam == NULL) return 0;
    for(i=0;i<oclistlength(params);i+=2) {
	char* name = (char*)oclistget(params,i);
	if(strcmp(clientparam,name)==0) {found=1; break;}
    }
    if(found) {
	oclistremove(params,i+1); /* remove value */
	oclistremove(params,i); /* remove name */
    }
    return found;
}


/*
Insert new client param (name,value);
return value = 1 => not already defined
               0 => param already defined (no change)
*/
int
ocparaminsert(OClist* params, const char* clientparam, const char* value)
{
    int i;
    if(params == NULL || clientparam == NULL) return 0;
    for(i=0;i<oclistlength(params);i+=2) {
	char* name = (char*)oclistget(params,i);
	if(strcmp(clientparam,name)==0) return 0;
    }
    /* not found, append */
    oclistpush(params,(ocelem)nulldup(clientparam));
    oclistpush(params,(ocelem)nulldup(value));
    return 1;
}


/*
Replace new client param (name,value);
return value = 1 => replacement performed
               0 => insertion performed
*/
int
ocparamreplace(OClist* params, const char* clientparam, const char* value)
{
    int i;
    if(params == NULL || clientparam == NULL) return 0;
    for(i=0;i<oclistlength(params);i+=2) {
	char* name = (char*)oclistget(params,i);
	if(strcmp(clientparam,name)==0) {
	    oclistinsert(params,i+1,(ocelem)nulldup(value));
	    return 1;
	}
    }
    ocparaminsert(params,clientparam,value);
    return 0;
}
#endif
