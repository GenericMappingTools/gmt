/*
 * Copyright 1993-1996 University Corporation for Atmospheric Research/Unidata
 * 
 * Portions of this software were developed by the Unidata Program at the 
 * University Corporation for Atmospheric Research.
 * 
 * Access and use of this software shall impose the following obligations
 * and understandings on the user. The user is granted the right, without
 * any fee or cost, to use, copy, modify, alter, enhance and distribute
 * this software, and any derivative works thereof, and its supporting
 * documentation for any purpose whatsoever, provided that this entire
 * notice appears in all copies of the software, derivative works and
 * supporting documentation.  Further, UCAR requests that the user credit
 * UCAR/Unidata in any publications that result from the use of this
 * software or in any product that includes this software. The names UCAR
 * and/or Unidata, however, may not be used in any advertising or publicity
 * to endorse or promote any products or commercial entity unless specific
 * written permission is obtained from UCAR/Unidata. The user also
 * understands that UCAR/Unidata is not obligated to provide the user with
 * any support, consulting, training or assistance of any kind with regard
 * to the use, operation and performance of this software nor to provide
 * the user with any updates, revisions, new versions or "bug fixes."
 * 
 * THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* "$Id$" */

#include "config.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <curl/curl.h>
#include "curlwrap.h"

#include "nclist.h"
#include "nclog.h"

#include "netcdf.h"
#include "nc.h"
#include "nc4internal.h"
#include "nccr.h"
#include "ast.h"
#include "crutil.h"

static char* combinecredentials(const char* user, const char* pwd);
static size_t WriteMemoryCallback(void*, size_t, size_t, void*);

/* Condition on libcurl version */
#ifndef HAVE_CURLOPT_KEYPASSWD
/* Set up an alias */
#define CURLOPT_KEYPASSWD CURLOPT_SSLKEYPASSWD
#endif

struct NCCR_CALLBACK_DATA {
    size_t alloc;
    size_t pos;
    unsigned char* data;
};

static size_t WriteMemoryCallback(void*, size_t, size_t, void*);

int
nccr_curlopen(CURL** curlp)
{
    int stat = NC_NOERR;
    CURLcode cstat;
    CURL* curl;
    /* initialize curl*/
    curl = curl_easy_init();
    if(curl == NULL)
        stat = NC_ECURL;
    else {
        cstat = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
        if(cstat != CURLE_OK)
            stat = NC_ECURL;
        /*some servers don't like requests that are made without a user-agent*/
        cstat = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        if(cstat != CURLE_OK)
            stat = NC_ECURL;
    }
    if(curlp)
        *curlp = curl;
    return stat;
}

int
nccr_curlclose(CURL* curl)
{
    if(curl != NULL)
        curl_easy_cleanup(curl);
    return NC_NOERR;
}

int
nccr_fetchurl(NCCDMR* cdmr, CURL* curl, char* url, bytes_t* buf, long* filetime)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;
    struct NCCR_CALLBACK_DATA callback_data;
    int index, first;

    /* If required, report the url */
    if(cdmr->controls & SHOWFETCH) {
	nclog(NCLOGNOTE,"fetch url: %s",url);
    }

    callback_data.alloc = 0;

    /* Set the URL */
    cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
    if(cstat != CURLE_OK)
            goto fail;

    /* send all data to this function  */
    cstat = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    if(cstat != CURLE_OK)
            goto fail;

    /* we pass our file to the callback function */
    cstat = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&callback_data);
    if(cstat != CURLE_OK)
            goto fail;

    /* One last thing; always try to get the last modified time */
    cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);

    /* fetch */
    cstat = curl_easy_perform(curl);
    if(cstat == CURLE_PARTIAL_FILE) {
        /* Log it but otherwise ignore */
        nclog(NCLOGERR,"curl error: %s; ignored",
               curl_easy_strerror(cstat));
        cstat = CURLE_OK;
    }
    if(cstat != CURLE_OK) goto fail;

    /* pull the data */
    if(buf) {
	buf->nbytes = callback_data.pos;
	buf->bytes = callback_data.data;
	callback_data.data = NULL;
    }

    /* Get the last modified time */
    if(filetime != NULL)
        cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
    if(cstat != CURLE_OK) goto fail;

    /* Check for potential html return */
    /* skip leading whitespace */
    for(first=0;first<buf->nbytes;first++) {
	char* p = strchr(" \t\r\n",buf->bytes[first]);
	if(p == NULL)
	    break;
    }
    index = crstrindex((char*)buf->bytes,"<html");
    if(index >= 0) {
	int endex;
	/* Search for </html> */
	endex = crstrindex((char*)buf->bytes,"</html>");	
	if(endex >= 0) endex += 7; else endex = buf->nbytes-1;
        nclog(NCLOGWARN,"Probable Server error");
	nclogtextn(NCLOGWARN,(char*)buf->bytes+first,endex-first);
	nclogtext(NCLOGWARN,"\n");
    }
    return stat;

fail:
    nclog(NCLOGERR,"curl error: %s", curl_easy_strerror(cstat));
    return NC_ECURL;
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *cdata)
{
    size_t realsize = size * nmemb;
    struct NCCR_CALLBACK_DATA* callback_data = (struct NCCR_CALLBACK_DATA*)cdata;

    if(realsize == 0)
       nclog(NCLOGERR,"WriteMemoryCallback: zero sized chunk");

    if(callback_data->alloc == 0) {
	callback_data->data = (unsigned char*)malloc(realsize);
	callback_data->alloc = realsize;
	callback_data->pos = 0;
    }

    if(callback_data->alloc - callback_data->pos < realsize) {
	callback_data->data = (unsigned char*)realloc(callback_data->data,
					     callback_data->alloc+realsize);
	callback_data->alloc += realsize;
    }
    memcpy(callback_data->data+callback_data->pos,ptr,realsize);
    callback_data->pos += realsize;
    return realsize;
}

long
nccr_fetchhttpcode(CURL* curl)
{
    long httpcode;
    CURLcode cstat = CURLE_OK;
    /* Extract the http code */
    cstat = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&httpcode);
    if(cstat != CURLE_OK) httpcode = 0;
    return httpcode;
}

int
nccr_fetchlastmodified(CURL* curl, char* url, long* filetime)
{
    int stat = NC_NOERR;
    CURLcode cstat = CURLE_OK;

    /* Set the URL */
    cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
    if(cstat != CURLE_OK)
        goto fail;

    /* Ask for head */
    cstat = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30); /* 30sec timeout*/
    cstat = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
    cstat = curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    cstat = curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    cstat = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);

    cstat = curl_easy_perform(curl);
    if(cstat != CURLE_OK) goto fail;
    if(filetime != NULL)
        cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
    if(cstat != CURLE_OK) goto fail;

    return stat;

fail:
    nclog(NCLOGERR,"curl error: %s\n", curl_easy_strerror(cstat));
    return NC_ECURL;
}

/**************************************************/

/* Set various general curl flags */
int
nccr_set_curl_flags(CURL* curl,  NCCDMR* nccr)
{
    CURLcode cstat = CURLE_OK;
    NCCURLSTATE* state = &nccr->curl;

#ifdef CURLOPT_ENCODING
    if (state->compress) {
        cstat = curl_easy_setopt(curl, CURLOPT_ENCODING, 'deflate, gzip');
        if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOP_ENCODING=deflat, gzip"));
#endif
    }
#endif

    if (state->cookiejar || state->cookiefile) {
        cstat = curl_easy_setopt(curl, CURLOPT_COOKIESESSION, 1);
        if (cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOP_COOKIESESSION=1"));
#endif
    }
    if (state->cookiejar) {
        cstat = curl_easy_setopt(curl, CURLOPT_COOKIEJAR, state->cookiejar);
        if (cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOP_COOKIEJAR=%s",state->cookiejar);
#endif
    }
    if (state->cookiefile) {
        cstat = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, state->cookiefile);
        if (cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_COOKIEFILE=%s",state->cookiefile);
#endif
    }
    if (state->verbose) {
        cstat = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        if (cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_VERBOSE=%ld",1L);
#endif
    }

    /* Following are always set */
    cstat = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_FOLLOWLOCATION=%ld",1L);
#endif
    return NC_NOERR;

fail:
    return NC_ECURL;
}

int
nccr_set_proxy(CURL* curl, NCCDMR* nccr)
{
    CURLcode cstat;
    struct NCCURLSTATE* state = &nccr->curl;

    cstat = curl_easy_setopt(curl, CURLOPT_PROXY, state->host);
    if (cstat != CURLE_OK) return NC_ECURL;
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_PROXY=%s",state->host);
#endif

    cstat = curl_easy_setopt(curl, CURLOPT_PROXYPORT, state->port);
    if (cstat != CURLE_OK) return NC_ECURL;
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_PROXYPORT=%d",state->port);
#endif

    if (state->username) {
        char *combined = combinecredentials(state->username,state->password);
        if (!combined) return NC_ENOMEM;
        cstat = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, combined);
        free(combined);
        if (cstat != CURLE_OK) return NC_ECURL;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_PROXYUSERPWD=%s",combined);
#endif
#ifdef CURLOPT_PROXYAUTH
        cstat = curl_easy_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
        if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
       LOG((LOGNOTE,"CURLOPT_PROXYAUTH=%ld",(long)CURLAUTH_ANY);
#endif
#endif
    }
    return NC_NOERR;
}

int
nccr_set_ssl(CURL* curl, NCCDMR* nccr)
{
    CURLcode cstat = CURLE_OK;
    struct NCCURLSTATE* state = &nccr->curl;
    long verify = (state->validate?1L:0L);
    cstat=curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify);
    if (cstat != CURLE_OK) goto fail;
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_SSL_VERIFYPEER=%ld",verify);
#endif
    cstat=curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, (verify?2L:0L));
    if (cstat != CURLE_OK) goto fail;
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_SSL_VERIFYHOST=%ld",(verify?2L:0L));
#endif
    {
        if(state->certificate) {
            cstat = curl_easy_setopt(curl, CURLOPT_SSLCERT, state->certificate);
            if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_SSLCERT=%s",state->certificate);
#endif
        }
        if(state->key) {
            cstat = curl_easy_setopt(curl, CURLOPT_SSLKEY, state->key);
            if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_SSLKEY=%s",state->key);
#endif
        }
        if(state->keypasswd) {
            cstat = curl_easy_setopt(curl, CURLOPT_KEYPASSWD, state->keypasswd);
            if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_SSLKEY=%s",state->key);
#endif
        }
        if(state->cainfo) {
            cstat = curl_easy_setopt(curl, CURLOPT_CAINFO, state->cainfo);
            if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_CAINFO=%s",state->cainfo);
#endif
        }
        if(state->capath) {
            cstat = curl_easy_setopt(curl, CURLOPT_CAPATH, state->capath);
            if(cstat != CURLE_OK) goto fail;
#ifdef DEBUG
        LOG((LOGNOTE,"CURLOPT_CAPATH=%s",state->capath);
#endif
        }
    }    
    return NC_NOERR;

fail:
    return NC_ECURL;
}

/* This is called with arguments while the other functions in this file are
 * used with global values read from the.dodsrc file. The reason is that
 * we may have multiple password sources.
 */
int
nccr_set_user_password(CURL* curl, const char *userC, const char *passwordC)
{
    CURLcode cstat;
    char* combined = NULL;

    if(userC == NULL && passwordC == NULL) return NC_NOERR;
    if(userC == NULL) userC = "";
    if(passwordC == NULL) passwordC = "";

    combined = combinecredentials(userC,passwordC);
    if (!combined) return NC_ENOMEM;
    cstat = curl_easy_setopt(curl, CURLOPT_USERPWD, combined);
    if (cstat != CURLE_OK) goto done;
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_USERPWD=%s",combined);
#endif
    cstat = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long) CURLAUTH_ANY);
    if (cstat != CURLE_OK) goto done;
#ifdef DEBUG
    LOG((LOGNOTE,"CURLOPT_HTTPAUTH=%ld",(long)CURLAUTH_ANY);
#endif

done:
    if(combined != NULL) free(combined);
    return (cstat == CURLE_OK?NC_NOERR:NC_ECURL);
}

static char*
combinecredentials(const char* user, const char* pwd)
{
    int userPassSize = strlen(user) + strlen(pwd) + 2;
    char *userPassword = malloc(sizeof(char) * userPassSize);
    if (!userPassword) {
        nclog(NCLOGERR,"Out of Memory");
        return NULL;
    }
    strcpy(userPassword, user);
    strcat(userPassword, ":");
    strcat(userPassword, pwd);
    return userPassword;
}
