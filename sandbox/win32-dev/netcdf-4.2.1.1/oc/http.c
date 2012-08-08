/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
 See the COPYRIGHT file for more information. */

#include "config.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "ocinternal.h"
#include "ocdebug.h"
#include "http.h"
#include "rc.h"

static size_t WriteFileCallback(void*, size_t, size_t, void*);
static size_t WriteMemoryCallback(void*, size_t, size_t, void*);

struct Fetchdata {
	FILE* stream;
	size_t size;
};

long
ocfetchhttpcode(CURL* curl)
{
    long httpcode;
    CURLcode cstat = CURLE_OK;
    /* Extract the http code */
    cstat = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&httpcode);
    if(cstat != CURLE_OK) httpcode = 0;
    return httpcode;
}

int
ocfetchurl_file(CURL* curl, char* url, FILE* stream,
		unsigned long* sizep, long* filetime)
{
	int stat = OC_NOERR;
	CURLcode cstat = CURLE_OK;
	struct Fetchdata fetchdata;

	/* Set the URL */
	cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
	if (cstat != CURLE_OK)
		goto fail;

	/* send all data to this function  */
	cstat = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
	if (cstat != CURLE_OK)
		goto fail;

	/* we pass our file to the callback function */
	cstat = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&fetchdata);
	if (cstat != CURLE_OK)
		goto fail;

        /* One last thing; always try to get the last modified time */
        cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);

	fetchdata.stream = stream;
	fetchdata.size = 0;
	cstat = curl_easy_perform(curl);
	if (cstat != CURLE_OK) {
	    goto fail;
	}

	if (stat == OC_NOERR) {
	    /* return the file size*/
#ifdef OCOCDBG
	    oc_log(LOGNOTE,"filesize: %lu bytes",fetchdata.size);
#endif
	    if (sizep != NULL)
		*sizep = fetchdata.size;
	    /* Get the last modified time */
	    if(filetime != NULL)
                cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
            if(cstat != CURLE_OK) goto fail;
	}
	return OCTHROW(stat);

fail: oc_log(LOGERR, "curl error: %s", curl_easy_strerror(cstat));
	return OCTHROW(OC_ECURL);
}

int
ocfetchurl(CURL* curl, char* url, OCbytes* buf, long* filetime)
{
	int stat = OC_NOERR;
	CURLcode cstat = CURLE_OK;
	size_t len;

	/* Set the URL */
	cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
	if (cstat != CURLE_OK)
		goto fail;

	/* send all data to this function  */
	cstat = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	if (cstat != CURLE_OK)
		goto fail;

	/* we pass our file to the callback function */
	cstat = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)buf);
	if (cstat != CURLE_OK)
		goto fail;

        /* One last thing; always try to get the last modified time */
        cstat = curl_easy_setopt(curl, CURLOPT_FILETIME, (long)1);

	cstat = curl_easy_perform(curl);
	if(cstat == CURLE_PARTIAL_FILE) {
	    /* Log it but otherwise ignore */
	    oc_log(LOGWARN, "curl error: %s; ignored",
		   curl_easy_strerror(cstat));
	    cstat = CURLE_OK;
	}
	if(cstat != CURLE_OK) goto fail;

        /* Get the last modified time */
	if(filetime != NULL)
            cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
        if(cstat != CURLE_OK) goto fail;

	/* Null terminate the buffer*/
	len = ocbyteslength(buf);
	ocbytesappend(buf, '\0');
	ocbytessetlength(buf, len); /* dont count null in buffer size*/
#ifdef OCOCDBG
	oc_log(LOGNOTE,"buffersize: %lu bytes",(unsigned long)ocbyteslength(buf));
#endif

	return OCTHROW(stat);

fail:
	oc_log(LOGERR, "curl error: %s", curl_easy_strerror(cstat));
	return OCTHROW(OC_ECURL);
}

static size_t
WriteFileCallback(void* ptr, size_t size, size_t nmemb,	void* data)
{
	size_t realsize = size * nmemb;
	size_t count;
	struct Fetchdata* fetchdata;
	fetchdata = (struct Fetchdata*) data;
        if(realsize == 0)
	    oc_log(LOGWARN,"WriteFileCallback: zero sized chunk");
	count = fwrite(ptr, size, nmemb, fetchdata->stream);
	if (count > 0) {
		fetchdata->size += (count * size);
	} else {
	    oc_log(LOGWARN,"WriteFileCallback: zero sized write");
	}
#ifdef OCPROGRESS
        oc_log(LOGNOTE,"callback: %lu bytes",(unsigned long)realsize);
#endif
	return count;
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	OCbytes* buf = (OCbytes*) data;
        if(realsize == 0)
	    oc_log(LOGWARN,"WriteMemoryCallback: zero sized chunk");
	/* Optimize for reading potentially large dods datasets */
	if(!ocbytesavail(buf,realsize)) {
	    /* double the size of the packet */
	    ocbytessetalloc(buf,2*ocbytesalloc(buf));
	}
	ocbytesappendn(buf, ptr, realsize);
#ifdef OCPROGRESS
        oc_log(LOGNOTE,"callback: %lu bytes",(unsigned long)realsize);
#endif
	return realsize;
}

#if 0
static void
assembleurl(DAPURL* durl, OCbytes* buf, int what)
{
	encodeurltext(durl->url,buf);
	if(what & WITHPROJ) {
		ocbytescat(buf,"?");
		encodeurltext(durl->projection,buf);
	}
	if(what & WITHSEL) encodeurltext(durl->selection,buf);

}

static char mustencode="";
static char hexchars[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f',
};

static void
encodeurltext(char* text, OCbytes* buf)
{
	/* Encode the URL to handle illegal characters */
	len = strlen(url);
	encoded = ocmalloc(len*4+1); /* should never be larger than this*/
	if(encoded==NULL) return;
	p = url; q = encoded;
	while((c=*p++)) {
		if(strchr(mustencode,c) != NULL) {
			char tmp[8];
			int hex1, hex2;
			hex1 = (c & 0x0F);
			hex2 = (c & 0xF0) >> 4;
			tmp[0] = '0'; tmp[1] = 'x';
			tmp[2] = hexchars[hex2]; tmp[3] = hexchars[hex1];
			tmp[4] = '\0';
			ocbytescat(buf,tmp);
		} else *q++ = (char)c;
	}

}

#endif

int
occurlopen(CURL** curlp)
{
	int stat = OC_NOERR;
	CURLcode cstat;
	CURL* curl;
	/* initialize curl*/
	curl = curl_easy_init();
	if (curl == NULL)
		stat = OC_ECURL;
	else {
		cstat = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
		if (cstat != CURLE_OK)
			stat = OC_ECURL;
		/* some servers don't like requests that are made without a user-agent */
		cstat = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		if (cstat != CURLE_OK)
			stat = OC_ECURL;
	}
	if (curlp)
		*curlp = curl;
	return OCTHROW(stat);
}

void
occurlclose(CURL* curl)
{
	if (curl != NULL)
		curl_easy_cleanup(curl);
}

int
ocfetchlastmodified(CURL* curl, char* url, long* filetime)
{
    int stat = OC_NOERR;
    CURLcode cstat = CURLE_OK;

    /* Set the URL */
    cstat = curl_easy_setopt(curl, CURLOPT_URL, (void*)url);
    if (cstat != CURLE_OK)
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

    return OCTHROW(stat);

fail:
    oc_log(LOGERR, "curl error: %s", curl_easy_strerror(cstat));
    return OCTHROW(OC_ECURL);
}
