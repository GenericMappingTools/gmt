#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <curl/curl.h>

#include "ast_runtime.h"
#include "ast_curl.h"

struct AST_CALLBACK_DATA {
    size_t alloc;
    size_t pos;
    char* data;
};

static size_t WriteMemoryCallback(void*, size_t, size_t, void*);

ast_err
ast_curlopen(CURL** curlp)
{
    ast_err stat = AST_NOERR;
    CURLcode cstat;
    CURL* curl;
    /* initialize curl*/
    curl = curl_easy_init();
    if(curl == NULL)
        stat = AST_ECURL;
    else {
        cstat = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
        if(cstat != CURLE_OK)
            stat = AST_ECURL;
        /*some servers don't like requests that are made without a user-agent*/
        cstat = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        if(cstat != CURLE_OK)
            stat = AST_ECURL;
    }
    if(curlp)
        *curlp = curl;
    return stat;
}

ast_err
ast_curlclose(CURL* curl)
{
    if(curl != NULL)
        curl_easy_cleanup(curl);
    return AST_NOERR;
}

ast_err
ast_fetchurl(CURL* curl, char* url, bytes_t* buf, long* filetime)
{
    ast_err stat = AST_NOERR;
    CURLcode cstat = CURLE_OK;
    struct AST_CALLBACK_DATA callback_data;

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
        ast_log("curl error: %s; ignored",
               curl_easy_strerror(cstat));
        cstat = CURLE_OK;
    }
    if(cstat != CURLE_OK) goto fail;

    /* pull the data */
    if(buf) {
	buf->nbytes = callback_data.pos;
	buf->bytes = callback_data.data;
    }

    /* Get the last modified time */
    if(filetime != NULL)
        cstat = curl_easy_getinfo(curl,CURLINFO_FILETIME,filetime);
    if(cstat != CURLE_OK) goto fail;
    return stat;

fail:
    ast_log("curl error: %s", curl_easy_strerror(cstat));
    return AST_ECURL;
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *cdata)
{
    size_t realsize = size * nmemb;
    struct AST_CALLBACK_DATA* callback_data = (struct AST_CALLBACK_DATA*)cdata;

    if(realsize == 0)
       ast_log("WriteMemoryCallback: zero sized chunk");

    if(callback_data->alloc == 0) {
	callback_data->data = (char*)malloc(realsize);
	callback_data->alloc = realsize;
	callback_data->pos = 0;
    }

    if(callback_data->alloc - callback_data->pos < realsize) {
	callback_data->data = (char*)realloc(callback_data->data,
					     callback_data->alloc+realsize);
	callback_data->alloc += realsize;
    }
    memcpy(callback_data->data+callback_data->pos,ptr,realsize);
    callback_data->pos += realsize;
    return realsize;
}

long
ast_fetchhttpcode(CURL* curl)
{
    long httpcode;
    CURLcode cstat = CURLE_OK;
    /* Extract the http code */
    cstat = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&httpcode);
    if(cstat != CURLE_OK) httpcode = 0;
    return httpcode;
}

int
ast_fetchlastmodified(CURL* curl, char* url, long* filetime)
{
    ast_err stat = AST_NOERR;
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
    ast_log("curl error: %s\n", curl_easy_strerror(cstat));
    return AST_ECURL;
}
