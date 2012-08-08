/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"

#include "ocinternal.h"
#include <stdio.h>
#include <fcntl.h>

#define PREFIXLEN 8

#define ENVFLAG "OCLOGFILE"

static int ocloginit = 0;
static int oclogging = 0;
static int ocsystemfile = 0; /* 1 => we are logging something we did not open */
static char* oclogfile = NULL;
static FILE* oclogstream = NULL;

/*!\defgroup OClog OClog Management
@{*/

/*!\internal
*/

void
oc_loginit(void)
{
    const char* file = getenv(ENVFLAG); /* I hope this is portable*/
    ocloginit = 1;
    oc_setlogging(0);
    oclogfile = NULL;
    oclogstream = NULL;
    /* Use environment variables to preset oclogging state*/
    if(file != NULL && strlen(file) > 0) {
        if(oc_logopen(file)) {
	    oc_setlogging(1);
	}
    }
}

/*!
Enable/Disable logging.

\param[in] tf If 1, then turn on logging, if 0, then turn off logging.

\return The previous value of the logging flag.
*/

int
oc_setlogging(int tf)
{
    int was;
    if(!ocloginit) oc_loginit();
    was = oclogging;
    oclogging = tf;
    return was;
}

/*!
Specify a file into which to place logging output.

\param[in] file The name of the file into which to place logging output.
If the file has the value NULL, then send logging output to
stderr.

\return zero if the open failed, one otherwise.
*/

int
oc_logopen(const char* file)
{
    if(!ocloginit) oc_loginit();
    oc_logclose();
    if(file == NULL || strlen(file) == 0) {
	/* use stderr*/
	oclogstream = stderr;
	oclogfile = NULL;
	ocsystemfile = 1;
    } else if(strcmp(file,"stdout") == 0) {
	/* use stdout*/
	oclogstream = stdout;
	oclogfile = NULL;
	ocsystemfile = 1;
    } else if(strcmp(file,"stderr") == 0) {
	/* use stderr*/
	oclogstream = stderr;
	oclogfile = NULL;
	ocsystemfile = 1;
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
	    return 0;
	}
    }
    return 1;
}

/*!
Close the logging output file (unless it is stderr).
Logging is still enabled.
*/

void
oc_logclose(void)
{
    if(!ocloginit) oc_loginit();
    if(oclogstream != NULL && !ocsystemfile) {
        assert(oclogfile != NULL && oclogstream != NULL);
	fclose(oclogstream);
    }
    if(oclogfile != NULL) free(oclogfile);
    oclogstream = NULL;
    oclogfile = NULL;
    ocsystemfile = 0;
}

/*!
Send logging messages. This uses a variable
number of arguments and operates like the stdio
printf function.

\param[in] tag Indicate the kind of this log message.
\param[in] format Format specification as with printf.
*/

void
oc_log(int tag, const char* format, ...)
{
    va_list args;
    char* prefix;

    if(!ocloginit) oc_loginit();
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

    if(format != NULL) {
      va_start(args, format);
      vfprintf(oclogstream, format, args);
      va_end( args );
    }
    fprintf(oclogstream, "\n" );
    fflush(oclogstream);
}

/*!
Send arbitrarily long text as a logging message.
Each line will be sent using oc_log with the specified tag.

\param[in] tag Indicate the kind of this log message.
\param[in] text Arbitrary text to send as a logging message.
*/

void
oc_logtext(int tag, const char* text)
{
    char line[1024];
    size_t delta = 0;
    const char* eol = text;

    if(!ocloginit) oc_loginit();
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

/**@}*/
