/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ocinternal.h"
#include "ocdebug.h"
#include "ocrc.h"

/* Condition on libcurl version */
/* Set up an alias as needed */
#ifndef HAVE_CURLOPT_KEYPASSWD
#define CURLOPT_KEYPASSWD CURLOPT_SSLKEYPASSWD
#endif


static char* combinecredentials(const char* user, const char* pwd);

void
oc_curl_debug(OCstate* state)
{
    curl_easy_setopt(state->curl,CURLOPT_VERBOSE,1);
    curl_easy_setopt(state->curl,CURLOPT_ERRORBUFFER,(void*)state->curlerror);
}

void
oc_curl_printerror(OCstate* state)
{
    fprintf(stderr,"curl error details: %s\n",state->curlerror);
}

/* Determine if this version of curl supports
       "file://..." &/or "https://..." urls.
*/
void
oc_curl_protocols(struct OCGLOBALSTATE* state)
{
    const char* const* proto; /*weird*/
    curl_version_info_data* curldata;
    curldata = curl_version_info(CURLVERSION_NOW);
    for(proto=curldata->protocols;*proto;proto++) {
        if(strcmp("file",*proto)==0) {state->curl.proto_file=1;break;}
        if(strcmp("http",*proto)==0) {state->curl.proto_https=1;break;}
    }
    if(ocdebug > 0) {
        oc_log(LOGNOTE,"Curl file:// support = %d",state->curl.proto_file);
        oc_log(LOGNOTE,"Curl https:// support = %d",state->curl.proto_https);
    }
}


/* Set various general curl flags */
int
ocset_curl_flags(OCstate* state)
{
    CURLcode cstat = CURLE_OK;
    CURL* curl = state->curl;
    struct OCcurlflags* flags = &state->curlflags;

#ifdef CURLOPT_ENCODING
    if (flags->compress) {
	cstat = curl_easy_setopt(curl, CURLOPT_ENCODING,"deflate, gzip");
	if(cstat != CURLE_OK) goto done;
	OCDBG(1,"CURLOPT_ENCODING=deflate, gzip");
    }
#endif
    if (flags->cookiejar || flags->cookiefile) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
	if (cstat != CURLE_OK) goto done;
	OCDBG(1,"CURLOPT_COOKIESESSION=1");
    }
    if (flags->cookiejar) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEJAR, flags->cookiejar);
	if (cstat != CURLE_OK) goto done;
	OCDBG1(1,"CURLOPT_COOKIEJAR=%s",flags->cookiejar);
    }
    if (flags->cookiefile) {
	cstat = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, flags->cookiefile);
	if (cstat != CURLE_OK) goto done;
	OCDBG1(1,"CURLOPT_COOKIEFILE=%s",flags->cookiefile);
    }
    if (flags->verbose) {
	cstat = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	if (cstat != CURLE_OK) goto done;
	OCDBG1(1,"CURLOPT_VERBOSE=%ld",1L);
    }

    if (flags->timeout) {
	cstat = curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)flags->timeout);
	if (cstat != CURLE_OK) goto done;
	OCDBG1(1,"CURLOPT_TIMEOUT=%ld",1L);
    }

    /* Following are always set */
    cstat = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    OCDBG1(1,"CURLOPT_FOLLOWLOCATION=%ld",1L);
    cstat = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    OCDBG1(1,"CURLOPT_MAXREDIRS=%ld",10L);

    cstat = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, state->error.curlerrorbuf);
    OCDBG1(1,"CURLOPT_ERRORBUFFER",0);

done:
    return cstat;
}

int
ocset_proxy(OCstate* state)
{
    CURLcode cstat;
    CURL* curl = state->curl;
    struct OCproxy *proxy = &state->proxy;
    struct OCcredentials *creds = &state->creds;

    cstat = curl_easy_setopt(curl, CURLOPT_PROXY, proxy->host);
    if (cstat != CURLE_OK) return OC_ECURL;
    OCDBG1(1,"CURLOPT_PROXY=%s",proxy->host);

    cstat = curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxy->port);
    if (cstat != CURLE_OK) return OC_ECURL;
    OCDBG1(1,"CURLOPT_PROXYPORT=%d",proxy->port);

    if (creds->username) {
        char *combined = combinecredentials(creds->username,creds->password);
        if (!combined) return OC_ENOMEM;
        cstat = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, combined);
        if (cstat != CURLE_OK) return OC_ECURL;
	OCDBG1(1,"CURLOPT_PROXYUSERPWD=%s",combined);
#ifdef CURLOPT_PROXYAUTH
        cstat = curl_easy_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
        if(cstat != CURLE_OK) goto fail;
	OCDBG1(1,"CURLOPT_PROXYAUTH=%ld",(long)CURLAUTH_ANY);
#endif
        free(combined);
    }
    return OC_NOERR;
}

int
ocset_ssl(OCstate* state)
{
    CURLcode cstat = CURLE_OK;
    CURL* curl = state->curl;
    struct OCSSL* ssl = &state->ssl;
    long verify = (ssl->validate?1L:0L);
    cstat=curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify);
    if (cstat != CURLE_OK) goto fail;
    OCDBG1(1,"CURLOPT_SSL_VERIFYPEER=%ld",verify);
    cstat=curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, (verify?2L:0L));
    if (cstat != CURLE_OK) goto fail;
    OCDBG1(1,"CURLOPT_SSL_VERIFYHOST=%ld",(verify?2L:0L));
#ifdef OCIGNORE
    if(verify)
#endif
    {
        if(ssl->certificate) {
            cstat = curl_easy_setopt(curl, CURLOPT_SSLCERT, ssl->certificate);
            if(cstat != CURLE_OK) goto fail;
	    OCDBG1(1,"CURLOPT_SSLCERT=%s",ssl->certificate);
        }
        if(ssl->key) {
            cstat = curl_easy_setopt(curl, CURLOPT_SSLKEY, ssl->key);
            if(cstat != CURLE_OK) goto fail;
	    OCDBG1(1,"CURLOPT_SSLKEY=%s",ssl->key);
        }
        if(ssl->keypasswd) {
            /* libcurl prior to 7.16.4 used 'CURLOPT_SSLKEYPASSWD' */
            cstat = curl_easy_setopt(curl, CURLOPT_KEYPASSWD, ssl->keypasswd);
            if(cstat != CURLE_OK) goto fail;
	    OCDBG1(1,"CURLOPT_SSLKEY=%s",ssl->key);
        }
        if(ssl->cainfo) {
            cstat = curl_easy_setopt(curl, CURLOPT_CAINFO, ssl->cainfo);
            if(cstat != CURLE_OK) goto fail;
	    OCDBG1(1,"CURLOPT_CAINFO=%s",ssl->cainfo);
        }
        if(ssl->capath) {
            cstat = curl_easy_setopt(curl, CURLOPT_CAPATH, ssl->capath);
            if(cstat != CURLE_OK) goto fail;
	    OCDBG1(1,"CURLOPT_CAPATH=%s",ssl->capath);
        }
        {
            cstat = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, ssl->verifypeer);
            if(cstat != CURLE_OK) goto fail;
	    OCDBG1(1,"CURLOPT_SSL_VERIFYPEER=%d",ssl->verifypeer);
        }
    }    
    return OC_NOERR;

fail:
    return OC_ECURL;
}

/* This is called with arguments while the other functions in this file are
 * used with global values read from the.dodsrc file. The reason is that
 * we may have multiple password sources.
 */
int
ocset_user_password(OCstate* state)
{
    CURLcode cstat;
    CURL* curl = state->curl;
    char* combined = NULL;
    const char* userC = state->creds.username;
    const char* passwordC = state->creds.password;

    if(userC == NULL || passwordC == NULL) return OC_NOERR;

    combined = combinecredentials(userC,passwordC);
    if (!combined) return OC_ENOMEM;
    cstat = curl_easy_setopt(curl, CURLOPT_USERPWD, combined);
    if (cstat != CURLE_OK) goto done;
    OCDBG1(1,"CURLOPT_USERPWD=%s",combined);
    cstat = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long) CURLAUTH_ANY);
    if (cstat != CURLE_OK) goto done;
    OCDBG1(1,"CURLOPT_HTTPAUTH=%ld",(long)CURLAUTH_ANY);

done:
    if(combined != NULL) free(combined);
    return (cstat == CURLE_OK?OC_NOERR:OC_ECURL);
}


static char*
combinecredentials(const char* user, const char* pwd)
{
    int userPassSize = strlen(user) + strlen(pwd) + 2;
    char *userPassword = malloc(sizeof(char) * userPassSize);
    if (!userPassword) {
        oc_log(LOGERR,"Out of Memory\n");
	return NULL;
    }
    strcpy(userPassword, user);
    strcat(userPassword, ":");
    strcat(userPassword, pwd);
    return userPassword;
}
