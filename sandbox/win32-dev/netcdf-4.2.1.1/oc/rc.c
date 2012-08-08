/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ocinternal.h"
#include "ocdebug.h"
#include "ocdata.h"
#include "occontent.h"
#include "oclog.h"

#include "rc.h"

#define RTAG ']'
#define LTAG '['

#define TRIMCHARS " \t\r\n"

#define TRIM(x) rctrimright(rctrimleft((x),TRIMCHARS),TRIMCHARS)

#define HTTPPREFIXDEPRECATED "CURL."
#define HTTPPREFIX           "HTTP."

/* the .dodsrc triple store */
struct OCTriplestore* ocdodsrc = NULL;

static int parseproxy(OCstate* state, char* v);
static int rcreadline(FILE* f, char* more, int morelen);
static char* rctrimright(char* more, char* trimchars);
static char* rctrimleft(char* more, char* trimchars);

static void ocdodsrcdump(char* msg, struct OCTriple*, int ntriples);

static char* curllookup(char* suffix,char* url);

/* The Username and password are in the URL if the URL is of the form:
 * http://<name>:<passwd>@<host>/....
 */
int
occredentials_in_url(const char *url)
{
	char *pos = strstr(url, "http://");
	if (!pos)
		return FALSE;
	pos += 7;
	if (strchr(pos, '@') && strchr(pos, ':'))
		return TRUE;

	return FALSE;
}

int
ocextract_credentials(const char *url, char **name, char **pw, char **result_url)
{
	char *pos;
	char *end;
	char *middle;
	int up_len = 0;
	int mid_len = 0;
	int midpas_len = 0;
	int url_len = 0;

	if (strchr(url, '@')) {
		pos = strstr(url, "http://");
		if (pos)
			pos += 7;
		middle = strchr(pos, ':');
		mid_len = middle - pos;
		*name = malloc(sizeof(char) * (mid_len + 1));
		strncpy(*name, pos, mid_len);
		(*name)[mid_len] = '\0';

		if (middle)
			middle += 1;

		end = strchr(middle, '@');
		midpas_len = end - middle;
		*pw = malloc(sizeof(char) * (midpas_len + 1));
		strncpy(*pw, middle, midpas_len);
		(*pw)[midpas_len] = '\0';

		up_len = end - pos;
		url_len = strlen(url) - up_len;

		*result_url = malloc(sizeof(char) * (url_len + 1));
		if (!result_url)
			return OC_ENOMEM;

		strncpy(*result_url, url, pos - url);
		strncpy(*result_url + (pos - url), end + 1, url_len - (pos - url));

#if 0
		fprintf(stderr, "URL without username and password: %s:%d\n", sURL, url_len );
		fprintf(stderr, "URL username and password: %s:%d\n", sUP, up_len);
		fprintf(stderr, "URL username: %s:%d\n", sUser, mid_len);
		fprintf(stderr, "URL password: %s:%d\n", sPassword, midpas_len);
#endif
		(*result_url)[url_len] = '\0';

		return OC_NOERR;
	}
	else {
		return OC_EIO;
	}
}

static int
rcreadline(FILE* f, char* more, int morelen)
{
    int i = 0;
    int c = getc(f);
    if(c < 0) return 0;
    for(;;) {
	if(i < morelen)  /* ignore excess characters */
	    more[i++]=c;
	c = getc(f);
	if(c < 0) break; /* eof */
        if(c == '\n') break; /* eol */
    }
    /* null terminate more */
    more[i] = '\0';
    return 1;
}

/* Trim specified characters from front/left */
static char*
rctrimleft(char* more, char* trimchars)
{
    char* p = more;
    int c;
    while((c=*p) != '\0') {if(strchr(trimchars,c) != NULL) p++; else break;}
    return p;
}

/* Trim specified characters from end/right */
static char*
rctrimright(char* more, char* trimchars)
{
    int len = strlen(more);
    char* p = more + (len - 1);
    while(p != more) {if(strchr(trimchars,*p) != NULL) p--; else break;}
    /* null terminate */
    p[1] = '\0';
    return more;
}

static int
parseproxy(OCstate* state, char* v)
{
    char *host_pos = NULL;
    char *port_pos = NULL;

    if(strlen(v) == 0) return OC_NOERR; /* nothing there*/
    if (occredentials_in_url(v)) {
        char *result_url = NULL;
        ocextract_credentials(v, &state->creds.username,
                            &state->creds.password,
                            &result_url);
        v = result_url;
    }
    /* allocating a bit more than likely needed ... */
    host_pos = strstr(v, "http://");
    if (host_pos)
        host_pos += strlen("http://");

    port_pos = strchr(host_pos, ':');
    if (port_pos) {
        int host_len;
        char *port_sep = port_pos;
        port_pos++;
        *port_sep = '\0';
        host_len = strlen(host_pos);
        state->proxy.host = malloc(sizeof(char) * host_len + 1);
        if (!state->proxy.host)
            return OC_ENOMEM;

        strncpy(state->proxy.host, host_pos, host_len);
        state->proxy.host[host_len + 1] = '\0';

        state->proxy.port = atoi(port_pos);
    } else {
        int host_len = strlen(host_pos);
        state->proxy.host = malloc(sizeof(char) * host_len + 1);
        if (!state->proxy.host)
            return OC_ENOMEM;

        strncpy(state->proxy.host, host_pos, host_len);
        state->proxy.host[host_len + 1] = '\0';

        state->proxy.port = 80;
    }
#if 0
    state->proxy.host[v_len] = '\0';
    state->proxy.port = atoi(v);
    s_len = strlen(v);
    state->proxy.user = malloc(sizeof(char) * s_len + 1);
    if (!state->proxy.user)
        return OC_ENOMEM;
     strncpy(state->proxy.user, v, s_len);
     state->proxy.user[s_len] = '\0';
     p_len = strlen(v);
     state->proxy.password = malloc(sizeof(char) * p_len + 1);
     if (!state->proxy.password)
         return OC_ENOMEM;
     strncpy(state->proxy.password, v, p_len);
     state->proxy.password[p_len] = '\0';
#endif /*0*/
     if (ocdebug > 1) {
         oc_log(LOGNOTE,"host name: %s", state->proxy.host);
         oc_log(LOGNOTE,"user name: %s", state->creds.username);
#ifdef INSECURE
         oc_log(LOGNOTE,"password: %s", state->creds.password);
#endif
         oc_log(LOGNOTE,"port number: %d", state->proxy.port);
    }
    if(v) free(v);
    return OC_NOERR;
}

/* insertion sort the triplestore based on url */
static void
sorttriplestore(void)
{
    int i, nsorted;
    struct OCTriple* sorted = NULL;

    if(ocdodsrc->ntriples <= 1) return; /* nothing to sort */
   if(ocdebug > 2)
        ocdodsrcdump("initial:",ocdodsrc->triples,ocdodsrc->ntriples);

    sorted = (struct OCTriple*)malloc(sizeof(struct OCTriple)*ocdodsrc->ntriples);
    if(sorted == NULL) {
        oc_log(LOGERR,"sorttriplestore: out of memory");
        return;
    }

    nsorted = 0;
    while(nsorted < ocdodsrc->ntriples) {
	int largest;
	/* locate first non killed entry */
	for(largest=0;largest<ocdodsrc->ntriples;largest++) {
            if(ocdodsrc->triples[largest].key[0] != '\0') break;
	}
        OCASSERT(ocdodsrc->triples[largest].key[0] != '\0');
	for(i=0;i<ocdodsrc->ntriples;i++) {
	    if(ocdodsrc->triples[i].key[0] != '\0') { /* avoid empty slots */
	        int lexorder = strcmp(ocdodsrc->triples[i].url,ocdodsrc->triples[largest].url);
   	        int leni = strlen(ocdodsrc->triples[i].url);
 	        int lenlarge = strlen(ocdodsrc->triples[largest].url);
	        /* this defines the ordering */
	        if(leni == 0 && lenlarge == 0) continue; /* if no urls, then leave in order */
	        if(leni != 0 && lenlarge == 0) largest = i;
	        else if(lexorder > 0) largest = i;
	    }
	}
	/* Move the largest entry */
	OCASSERT(ocdodsrc->triples[largest].key[0] != 0);
	sorted[nsorted] = ocdodsrc->triples[largest];
	ocdodsrc->triples[largest].key[0] = '\0'; /* kill entry */
	nsorted++;
      if(ocdebug > 2)
            ocdodsrcdump("pass:",sorted,nsorted);
    }    

    memcpy((void*)ocdodsrc->triples,(void*)sorted,sizeof(struct OCTriple)*nsorted);
    free(sorted);

    if(ocdebug > 0)
	ocdodsrcdump("final .dodsrc order:",ocdodsrc->triples,ocdodsrc->ntriples);
}

/* Create a triple store from a .dodsrc */
int
ocdodsrc_read(char* basename, char *in_file_name)
{
    char line0[MAXRCLINESIZE];
    FILE *in_file = NULL;
    int linecount = 0;

    if(ocdodsrc == NULL) {
        ocdodsrc = (struct OCTriplestore*)malloc(sizeof(struct OCTriplestore));
        if(ocdodsrc == NULL) {
	    oc_log(LOGERR,"ocdodsrc_read: out of memory");
	    return 0;
	}
    }
    ocdodsrc->ntriples = 0;

    in_file = fopen(in_file_name, "r"); /* Open the file to read it */
    if (in_file == NULL) {
	oc_log(LOGERR, "Could not open configuration file: %s",basename);
	return OC_EPERM;
    }

    for(;;) {
	char *line,*key,*value;
        if(!rcreadline(in_file,line0,sizeof(line0))) break;
	linecount++;
	if(linecount >= MAXRCLINES) {
	    oc_log(LOGERR, ".dodsrc has too many lines");
	    return 0;
	}	    	
	line = line0;
	/* check for comment */
        if (line[0] == '#') continue;
	/* trim leading blanks */
	line = rctrimleft(line,TRIMCHARS);
	if(strlen(line) >= MAXRCLINESIZE) {
	    oc_log(LOGERR, "%s line too long: %s",basename,line0);
	    return 0;
	}	    	
        /* parse the line */
	ocdodsrc->triples[ocdodsrc->ntriples].url[0] = '\0'; /*assume no url*/
	if(line[0] == LTAG) {
	    char* url = ++line;
	    char* rtag = strchr(line,RTAG);
	    if(rtag == NULL) {
		oc_log(LOGERR, "Malformed [url] in %s entry: %s",basename,line);
		continue;
	    }	    
	    line = rtag + 1;
	    *rtag = '\0';
	    /* trim again */
   	    line = rctrimleft(line,TRIMCHARS);
	    /* save the url */
	    strcpy(ocdodsrc->triples[ocdodsrc->ntriples].url,TRIM(url));
	}
	if(strlen(line)==0) continue; /* empty line */
	/* split off key and value */
	key=line;
	value = strchr(line, '=');
	if(value == NULL) {
	    /* add fake '=1' */
	    if(strlen(line) + strlen("=1") >= MAXRCLINESIZE) {
		oc_log(LOGERR, "%s entry too long: %s",basename,line);
		continue;
	    }
	    strcat(line,"=1");
	    value = strchr(line,'=');
	}
        *value = '\0';
	value++;
	strcpy(ocdodsrc->triples[ocdodsrc->ntriples].key,TRIM(key));
	strcpy(ocdodsrc->triples[ocdodsrc->ntriples].value,TRIM(value));
	ocdodsrc->ntriples++;
    }
    fclose(in_file);
    sorttriplestore();
    return 1;
}


int
ocdodsrc_process(OCstate* state)
{
    char* value;
    char* url = ocuribuild(state->uri,NULL,NULL,0);
    if(ocdodsrc == NULL) return 0;
    value = curllookup("DEFLATE",url);
    if(value != NULL) {
        if(atoi(value)) state->curlflags.compress = 1;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"Compression: %ld", state->curlflags.compress);
    }
    if((value = curllookup("VERBOSE",url)) != NULL) {
        if(atoi(value)) state->curlflags.verbose = 1;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"curl.verbose: %ld", state->curlflags.verbose);
    }
    if((value = curllookup("TIMEOUT",url)) != NULL) {
        if(atoi(value)) state->curlflags.timeout = atoi(value);
        if(ocdebug > 0)
            oc_log(LOGNOTE,"curl.timeout: %ld", state->curlflags.timeout);
    }

    if((value = curllookup("COOKIEFILE",url)) != NULL) {
        state->curlflags.cookiefile = strdup(TRIM(value));
        if(!state->curlflags.cookiefile) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"COOKIEFILE: %s", state->curlflags.cookiefile);
    }
    if((value = curllookup("COOKIEJAR",url))
       || (value = curllookup("COOKIE_JAR",url))) {
        state->curlflags.cookiejar = strdup(TRIM(value));
        if(!state->curlflags.cookiejar) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"COOKIEJAR: %s", state->curlflags.cookiejar);
    }

    /* Some servers (e.g. thredds) appear to require a place
       to put cookies in order for some security functions to work
    */
    if(state->curlflags.cookiejar == NULL
       && state->curlflags.cookiefile == NULL) {
	state->curlflags.cookiefile = strdup("");
    }

    if((value = curllookup("PROXY_SERVER",url)) != NULL) {
        int stat = parseproxy(state,TRIM(value));
        if(stat != OC_NOERR) return stat;
    }

    if((value = curllookup("SSL.VALIDATE",url)) != NULL) {
        if(atoi(value)) state->ssl.validate = 1;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"CURL.SSL.VALIDATE: %ld", state->ssl.validate);
    }

    if((value = curllookup("SSL.CERTIFICATE",url)) != NULL) {
        state->ssl.certificate = strdup(TRIM(value));
        if(!state->ssl.certificate) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"CREDENTIALS.SSL.CERTIFICATE: %s", state->ssl.certificate);
    }

    if((value = curllookup("SSL.KEY",url)) != NULL) {
        state->ssl.key = strdup(TRIM(value));
        if(!state->ssl.key) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"CREDENTIALS.SSL.KEY: %s", state->ssl.key);
    }

    if((value = curllookup("SSL.KEYPASSWORD",url)) != NULL) {
        state->ssl.keypasswd = strdup(TRIM(value));
        if(!state->ssl.keypasswd) return OC_ENOMEM;
#ifdef INSECURE
        if(ocdebug > 0)
            oc_log(LOGNOTE,"CREDENTIALS.SSL.KEYPASSWORD: %s", state->ssl.keypasswd);
#endif
    }

    if((value = curllookup("SSL.CAINFO",url)) != NULL) {
        state->ssl.cainfo = strdup(TRIM(value));
        if(!state->ssl.cainfo) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"SSL.CAINFO: %s", state->ssl.cainfo);
    }

    if((value = curllookup("SSL.CAPATH",url)) != NULL) {
        state->ssl.capath = strdup(TRIM(value));
        if(!state->ssl.capath) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"SSL.CAPATH: %s", state->ssl.capath);
    }

    if((value = curllookup("CREDENTIALS.USER",url)) != NULL) {
        state->creds.username = strdup(TRIM(value));
        if(!state->creds.username) return OC_ENOMEM;
        if(ocdebug > 0)
            oc_log(LOGNOTE,"CREDENTIALS.USER: %s", state->creds.username);
    }

    if((value = curllookup("CREDENTIALS.PASSWORD",url)) != NULL) {
        state->creds.password = strdup(TRIM(value));
        if(!state->creds.password) return OC_ENOMEM;
    }
    /* else ignore */    

    free(url);

    return OC_NOERR;
}
    
char*
ocdodsrc_lookup(char* key, char* url)
{
    int i,found;
    struct OCTriple* triple = ocdodsrc->triples;
    if(key == NULL || ocdodsrc == NULL) return NULL;
    if(url == NULL) url = "";
    /* Assume that the triple store has been properly sorted */
    for(found=0,i=0;i<ocdodsrc->ntriples;i++,triple++) {
	int triplelen = strlen(triple->url);
	int t;
	if(strcmp(key,triple->key) != 0) continue; /* keys do not match */
	/* If the triple entry has no url, then use it (because we have checked all other cases)*/
	if(triplelen == 0) {found=1;break;}
	/* do url prefix comparison */
	t = strncmp(url,triple->url,triplelen);
	if(t ==  0) {found=1; break;}
    }
    if(ocdebug > 2)
    {
	if(found) {
	    fprintf(stderr,"lookup %s: [%s]%s = %s\n",url,triple->url,triple->key,triple->value);
	}
    }    
    return (found ? triple->value : NULL);
}


static void
ocdodsrcdump(char* msg, struct OCTriple* triples, int ntriples)
{
    int i;
    if(msg != NULL) fprintf(stderr,"%s\n",msg);
    if(ocdodsrc == NULL) {
	fprintf(stderr,"<EMPTY>\n");
	return;
    }
    if(triples == NULL) triples= ocdodsrc->triples;
    if(ntriples < 0 ) ntriples= ocdodsrc->ntriples;
    for(i=0;i<ntriples;i++) {
        fprintf(stderr,"\t%s\t%s\t%s\n",
		(strlen(triples[i].url)==0?"--":triples[i].url),
		triples[i].key,
		triples[i].value);
    }
}

/* Isolate the "CURL." prefix to allow changing to something else */
static char*
curllookup(char* suffix, char* url)
{
    char key[2048];
    char* value = NULL;
    strcpy(key,HTTPPREFIX);
    strcat(key,suffix);
    value = ocdodsrc_lookup(key,url);
    if(value == NULL) {
	strcpy(key,HTTPPREFIXDEPRECATED);
	strcat(key,suffix);
        value = ocdodsrc_lookup(key,url);
    }
    return value;
}
