/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ocinternal.h"
#include <stdio.h>
#include <fcntl.h>

#define PREFIXLEN 8

#define ENVFLAG "OCLOGFILE"

static int ocloginit = 0;
static int oclogging = 0;
static char* oclogfile = NULL;
static FILE* oclogstream = NULL;

void
oc_loginit(void)
{
    ocloginit = 1;
    oc_setlogging(0);
    oclogfile = NULL;
    oclogstream = NULL;
    /* Use environment variables to preset oclogging state*/
    /* I hope this is portable*/
    if(getenv(ENVFLAG) != NULL) {
	const char* file = getenv(ENVFLAG);
	oc_setlogging(1);
	oc_logopen(file);
    }
}

void
oc_setlogging(int tf)
{
    if(!ocloginit) oc_loginit();
    oclogging = tf;
}

void
oc_logopen(const char* file)
{
    if(!ocloginit) oc_loginit();
    if(oclogfile != NULL) {
	fclose(oclogstream);
	free(oclogfile);
	oclogfile = NULL;
    }
    if(file == NULL || strlen(file) == 0) {
	/* use stderr*/
	oclogstream = stderr;
	oclogfile = NULL;
    } else {
	int fd;
	oclogfile = (char*)malloc(strlen(file)+1);
	strcpy(oclogfile,file);
	oclogstream = NULL;
	/* We need to deal with this file carefully
	   to avoid unauthorized access*/
	fd = open(oclogfile,O_WRONLY|O_APPEND|O_CREAT,0600);
	if(fd >= 0) {
	    oclogstream = fdopen(fd,"a");
	} else {
	    free(oclogfile);
	    oclogfile = NULL;
	    oc_setlogging(0);
	}
    }
}

void
oc_logclose(void)
{
    if(oclogfile != NULL && oclogstream != NULL) {
	fclose(oclogstream);
	oclogstream = NULL;
	if(oclogfile != NULL) free(oclogfile);
	oclogfile = NULL;
    }
}

void
oc_log(int tag, const char* fmt, ...)
{
    va_list args;
    char* prefix;
    if(!oclogging || oclogstream == NULL) return;

    switch (tag) {
    case LOGWARN: prefix = "Warning:"; break;
    case LOGERR:  prefix = "Error:  "; break;
    case LOGNOTE: prefix = "Note:   "; break;
    case LOGDBG:  prefix = "Debug:  "; break;
    default:
        fprintf(oclogstream,"Error:  Bad log prefix: %d\n",tag);
	prefix = "Error:  ";
	break;
    }
    fprintf(oclogstream,"%s:",prefix);

    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(oclogstream, fmt, args);
      va_end( args );
    }
    fprintf(oclogstream, "\n" );
    fflush(oclogstream);
}

void
oc_logtext(int tag, const char* text)
{
    char line[1024];
    size_t delta = 0;
    const char* eol = text;

    if(!oclogging || oclogstream == NULL) return;

    while(*text) {
	eol = strchr(text,'\n');
	if(eol == NULL)
	    delta = strlen(text);
	else
	    delta = (eol - text);
	if(delta > 0) memcpy(line,text,delta);
	line[delta] = '\0';
	fprintf(oclogstream,"        %s\n",line);
	text = eol+1;
    }
}
