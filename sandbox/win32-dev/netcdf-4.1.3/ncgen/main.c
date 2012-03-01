/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/main.c,v 1.33 2010/05/26 21:43:36 dmh Exp $ */

#include "includes.h"
#include "offsets.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

/* Default is netcdf-3 mode 1 */
#define DFALTCMODE 0

extern void init_netcdf(void);
extern void parse_init(void);
extern int ncgparse(void);

/* Default is netcdf-3 mode 1 */
#define DFALTCMODE 0

/* For error messages */
char* progname;
char* cdlname;

int kflag_flag; /* 1 => -k was specified on command line*/
int cmode_modifier;
int nofill_flag;
char* mainname; /* name to use for main function; defaults to "main"*/
int c_flag;
int binary_flag;
int f77_flag;
int cml_flag;
int java_flag; /* 1=> use netcdf java interface (=>usingclassic)*/

size_t nciterbuffersize;

struct Vlendata* vlendata;

char *netcdf_name; /* command line -o file name */
char *datasetname; /* name from the netcdf <name> {} */

/* Misc. flags*/
int usingclassic;
int allowspecial; /* are special attributes ok to use */

extern FILE *ncgin;

/* Forward */
static char* ubasename ( const char* av0 );
void usage ( void );
int main ( int argc, char** argv );

/* Define tables vs modes for legal -k values*/
struct Kvalues legalkinds[NKVALUES] = {
    {"1", 0},
    {"classic", 0},

/* The 64-bit offset kind (2)  should only be used if actually needed */
    {"2", NC_64BIT_OFFSET},
    {"64-bit-offset", NC_64BIT_OFFSET},
    {"64-bit offset", NC_64BIT_OFFSET},

    /* NetCDF-4 HDF5 format*/
    {"3", NC_NETCDF4},
    {"hdf5", NC_NETCDF4},
    {"netCDF-4", NC_NETCDF4},
    {"netcdf-4", NC_NETCDF4},
    {"netcdf4", NC_NETCDF4},
    {"enhanced", NC_NETCDF4},
    /* NetCDF-4 HDF5 format, but using only nc3 data model */
    {"4", NC_NETCDF4 | NC_CLASSIC_MODEL},
    {"hdf5-nc3", NC_NETCDF4 | NC_CLASSIC_MODEL},
    {"netCDF-4 classic model", NC_NETCDF4 | NC_CLASSIC_MODEL},
    {"enhanced-nc3", NC_NETCDF4 | NC_CLASSIC_MODEL},

    /* null terminate*/
    {NULL,0}
};

struct Languages {
    char* name;
    int*  flag;
} legallanguages[] = {
{"b", &binary_flag},
{"c", &c_flag},
{"C", &c_flag},
{"f77", &f77_flag},
{"fortran77", &f77_flag},
{"Fortran77", &f77_flag},
{"j", &java_flag},
{"java", &java_flag},
{NULL,NULL}
};

/* The default minimum iterator size depends
   on whether we are doing binary or language
   based output.
*/
#define DFALTBINNCITERBUFFERSIZE  0x40000 /* about 250k bytes */
#define DFALTLANGNCITERBUFFERSIZE  0x4000 /* about 15k bytes */

/* strip off leading path */
/* result is malloc'd */

static char *
ubasename(const char *av0)
{
    char *logident = nulldup(av0);
    char* sep;

    sep = strrchr(logident,'/');
#ifdef MSDOS
    if(sep == NULL) sep = strrchr(logident,'\\');
#endif
    if(sep == NULL) return logident;
    sep++; /* skip past the separator */
    return sep;
}

void
usage(void)
{
    derror("Usage: %s [ -b ] [ -c ] [ -f ] [ -k kind ] [ -x ] [-S struct-format] [-M <name> [ -o outfile]  [ file ... ]",
	   progname);
    derror("netcdf library version %s", nc_inq_libvers());
}

int
main(
	int argc,
	char *argv[])
{
    int c;
    FILE *fp;
    int languages = 0;

#ifdef __hpux
    setlocale(LC_CTYPE,"");
#endif
    
    init_netcdf();

    opterr = 1;			/* print error message if bad option */
    progname = ubasename(argv[0]);
    cdlname = "-";
    netcdf_name = NULL;
    datasetname = NULL;
    c_flag = 0;
    f77_flag = 0;
    cml_flag = 0;
    java_flag = 0;
    binary_flag = 0;
    kflag_flag = 0;
    cmode_modifier = 0;
    nofill_flag = 0;
    mainname = "main";
    nciterbuffersize = 0;

#if _CRAYMPP && 0
    /* initialize CRAY MPP parallel-I/O library */
    (void) par_io_init(32, 32);
#endif

    while ((c = getopt(argc, argv, "bcfk:l:no:v:xdM:D:B:")) != EOF)
      switch(c) {
	case 'd':
	  debug = 1;	  
	  break;
	case 'D':
	  debug = atoi(optarg);
	  break;
	case 'c': /* for c output, old version of "-lc" */
	  c_flag = 1;
	  fprintf(stderr,"-c is deprecated: please use -lc\n");
	  break;
	case 'f': /* for f77 output, old version of "-lf" */
	  f77_flag = 1;
	  fprintf(stderr,"-f is deprecated: please use -lf77\n");
	  break;
	case 'b': /* for binary netcdf output, ".nc" extension */
	  binary_flag = 1;
	  break;
        case 'l': /* specify language, instead of using -c or -f or -b */
	    {
		struct Languages* langs;
		char* lang_name = (char*) emalloc(strlen(optarg)+1);
		(void)strcpy(lang_name, optarg);
		for(langs=legallanguages;langs->name != NULL;langs++) {
		    if(strcmp(lang_name,langs->name)==0) {
			*(langs->flag) = 1;
		        break;
		    }
		}
		if(langs->name == NULL) {
		    derror("%s: output language %s not implemented", 
			   progname, lang_name);
		    return(1);
		}
	    }
	  break;
	case 'n':		/* old version of -b, uses ".cdf" extension */
	  binary_flag = -1;
	  break;
	case 'o':		/* to explicitly specify output name */
	  netcdf_name = nulldup(optarg);
	  break;
	case 'x': /* set nofill mode to speed up creation of large files */
	  nofill_flag = 1;
	  break;
        case 'v': /* a deprecated alias for "kind" option */
	    /*FALLTHRU*/
        case 'k': /* for specifying variant of netCDF format to be generated 
                     Possible values are:
                     1 (=> classic 32 bit)
                     2 (=> classic 64 bit)
                     3 (=> enhanced)
                     4 (=> classic, but stored in an enhanced file format)
                     Also provide string versions of above
                     "classic"
                     "64-bit-offset"
                     "64-bit offset"
		     "enhanced" | "hdf5" | "netCDF-4"
                     "enhanced-nc3" | "hdf5-nc3" | "netCDF-4 classic model"
		   */
	    {
		struct Kvalues* kvalue;
		char *kind_name = (char *) emalloc(strlen(optarg)+1);
		if (! kind_name) {
		    derror ("%s: out of memory", progname);
		    return(1);
		}
		(void)strcpy(kind_name, optarg);
	        for(kvalue=legalkinds;kvalue->name;kvalue++) {
		    if(strcmp(kind_name,kvalue->name) == 0) {
		        cmode_modifier = kvalue->mode;
			break;
		    }
		}
		if(kvalue->name == NULL) {
		   derror("Invalid format: %s",kind_name);
		   return 2;
		}
		kflag_flag = 1;
	    }
	  break;
	case 'M': /* Determine the name for the main function */
	    mainname = nulldup(optarg);
	    break;
	case 'B':
	  nciterbuffersize = atoi(optarg);
	  break;
	case '?':
	  usage();
	  return(8);
      }

    /* check for multiple or no language spec */
    if(c_flag) languages++;
    if(binary_flag) languages++;
    if(f77_flag)languages++;
    if(cml_flag) languages++;
    if(java_flag) languages++;
    if(languages > 1) {
	fprintf(stderr,"Please specify only one language\n");
    } else if(languages == 0) {
	binary_flag = 1; /* binary is default */
    }

    /* Compute/default the iterator buffer size */
    if(binary_flag) {
	if(nciterbuffersize == 0 )
	    nciterbuffersize = DFALTBINNCITERBUFFERSIZE;
    } else {
	if(nciterbuffersize == 0)
	    nciterbuffersize = DFALTLANGNCITERBUFFERSIZE;
    }

#ifndef ENABLE_C
    if(c_flag) {
	  fprintf(stderr,"C not currently supported\n");
	  exit(1);
    }
#endif
#ifndef ENABLE_BINARY
    if(binary_flag) {
	  fprintf(stderr,"Binary netcdf not currently supported\n");
	  exit(1);
    }
#endif
#ifndef ENABLE_JAVA
    if(java_flag) {
	  fprintf(stderr,"Java not currently supported\n");
	  exit(1);
    }
#else
    if(java_flag && strcmp(mainname,"main")==0) mainname = "Main";
#endif
#ifndef ENABLE_F77
    if(f77_flag) {
	  fprintf(stderr,"F77 not currently supported\n");
	  exit(1);
    }
#endif

    argc -= optind;
    argv += optind;

    if (argc > 1) {
	derror ("%s: only one input file argument permitted",progname);
	return(6);
    }

    fp = stdin;
    if (argc > 0 && strcmp(argv[0], "-") != 0) {
	if ((fp = fopen(argv[0], "r")) == NULL) {
	    derror ("can't open file %s for reading: ", argv[0]);
	    perror("");
	    return(7);
	}
	cdlname = (char*)emalloc(NC_MAX_NAME);
	cdlname = nulldup(argv[0]);
	if(strlen(cdlname) > NC_MAX_NAME) cdlname[NC_MAX_NAME] = '\0';
    }

    /* Initially set up the cmode value and related items*/
    if(kflag_flag == 0) cmode_modifier = DFALTCMODE;

    usingclassic = 0;
    allowspecial = 0;

    /* Do first pass on flags */
    if((cmode_modifier & NC_NETCDF4) != 0) {
	allowspecial = 1;
	if((cmode_modifier & NC_CLASSIC_MODEL)) {
	    usingclassic = 1;
	} else {
	    usingclassic = 0;
	}	    
    } else {
	usingclassic = 1;
    }

    /* F77 || STD java => classic */
    if(f77_flag || java_flag) {
	usingclassic=1;
	cmode_modifier &= ~(NC_NETCDF4);
    }

    /* Standard Unidata java interface => usingclassic */

    parse_init();
    ncgin = fp;
    if(debug >= 2) {ncgdebug=1;}
    if(ncgparse() != 0) return 1;

    /* Recompute flags */
    if((cmode_modifier & NC_NETCDF4) != 0) {
	allowspecial = 1;
	if((cmode_modifier & NC_CLASSIC_MODEL)) {
	    usingclassic = 1;
	} else {
	    usingclassic = 0;
	}	    
    } else {
	usingclassic = 1;
    }

    /* Complain if still usingclassic =1 && a cdf4
       construct was used
    */
    if(usingclassic && getmarkcdf4() != NULL) {
	verror(getmarkcdf4());
	return 1;
    }


#ifndef USE_NETCDF4
    allowspecial = 0;
#endif

    processsemantics();
    define_netcdf();

    return 0;
}
END_OF_MAIN();

void
init_netcdf(void) /* initialize global counts, flags */
{
    compute_alignments();
    memset((void*)&nullconstant,0,sizeof(Constant));
    fillconstant = nullconstant;
    fillconstant.nctype = NC_FILLVALUE;

    codebuffer = bbNew();
    stmt = bbNew();
}
