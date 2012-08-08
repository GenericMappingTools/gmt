/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

#include "nclog.h"

#define PREFIXLEN 8
#define MAXTAGS 256
#define NCTAGDFALT "Log";

static int ncinitlog = 0;
static int nclogging = 0;
static int ncsystemfile = 0;
static char* nclogfile = NULL;
static FILE* nclogstream = NULL;

static int nctagsize = 0;
static char** nctagset = NULL;
static char* nctagdfalt = NULL;
static char* nctagsetdfalt[] = {"Warning","Error","Note","Debug"};
static char* nctagname(int tag);

void
ncloginit(void)
{
    const char* file = getenv(ENVFLAG);
    ncinitlog = 1;
    ncsetlogging(0);
    nclogfile = NULL;
    nclogstream = NULL;
    /* Use environment variables to preset nclogging state*/
    /* I hope this is portable*/
    if(file != NULL && strlen(file) > 0) {
        if(nclogopen(file)) {
	    ncsetlogging(1);
	}
    }
    nctagdfalt = NCTAGDFALT;
    nctagset = nctagsetdfalt;
}

void
ncsetlogging(int tf)
{
    if(!ncinitlog) ncloginit();
    nclogging = tf;
}

int
nclogopen(const char* file)
{
    if(!ncinitlog) ncloginit();
    nclogclose();
    if(file == NULL || strlen(file) == 0) {
	/* use stderr*/
	nclogstream = stderr;
	nclogfile = NULL;
	ncsystemfile = 1;
    } else if(strcmp(file,"stdout") == 0) {
	/* use stdout*/
	nclogstream = stdout;
	nclogfile = NULL;
	ncsystemfile = 1;
    } else if(strcmp(file,"stderr") == 0) {
	/* use stderr*/
	nclogstream = stderr;
	nclogfile = NULL;
	ncsystemfile = 1;
    } else {
	int fd;
	nclogfile = strdup(file);
	nclogstream = NULL;
	/* We need to deal with this file carefully
	   to avoid unauthorized access*/
	fd = open(nclogfile,O_WRONLY|O_APPEND|O_CREAT,0600);
	if(fd >= 0) {
	    nclogstream = fdopen(fd,"a");
	} else {
	    free(nclogfile);
	    nclogfile = NULL;
	    nclogstream = NULL;
	    ncsetlogging(0);
	    return 0;
	}
	ncsystemfile = 0;
    }
    return 1;
}

void
nclogclose(void)
{
    if(nclogstream != NULL && !ncsystemfile) {
	fclose(nclogstream);
    }
    if(nclogfile != NULL) free(nclogfile);
    nclogstream = NULL;
    nclogfile = NULL;
    ncsystemfile = 0;
}

void
nclog(int tag, const char* fmt, ...)
{
    va_list args;
    char* prefix;
    if(!nclogging || nclogstream == NULL) return;

    prefix = nctagname(tag);
    fprintf(nclogstream,"%s:",prefix);

    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(nclogstream, fmt, args);
      va_end( args );
    }
    fprintf(nclogstream, "\n" );
    fflush(nclogstream);
}

void
nclogtext(int tag, const char* text)
{
    nclogtextn(tag,text,strlen(text));
}

void
nclogtextn(int tag, const char* text, size_t count)
{
    if(!nclogging || nclogstream == NULL) return;
    fwrite(text,1,count,nclogstream);
    fflush(nclogstream);
}

/* The tagset is null terminated */
void
nclogsettags(char** tagset, char* dfalt)
{
    nctagdfalt = dfalt;
    if(tagset == NULL) {
	nctagsize = 0;
    } else {
        int i;
	/* Find end of the tagset */
	for(i=0;i<MAXTAGS;i++) {if(tagset[i]==NULL) break;}
	nctagsize = i;
    }
    nctagset = tagset;
}

static char*
nctagname(int tag)
{
    if(tag < 0 || tag >= nctagsize) {
	return nctagdfalt;
    } else {
	return nctagset[tag];
    }
}
