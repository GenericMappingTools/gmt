/*
     $Id$
     $Log: drscdf.h,v $
     Revision 1.1.1.1  2009/07/06 15:06:30  ed
     added libcf to netcdf dist

     Revision 1.1  2008/06/30 16:07:44  ed
     Beginning merge of calandar stuff.

 * Revision 2.8  1995/10/26  23:22:46  drach
 * - Added automatic copy of dictionary to/from NSL, NSL version
 *
 * Revision 2.7  1995/03/30  00:50:34  drach
 * Added disclaimer
 *
 * Revision 2.6  1995/01/30  19:14:22  drach
 * Minor revisions
 *
 * Revision 2.5  1994/11/23  19:16:58  drach
 * Added function declarations for K&R and ANSI C.
 *
 * Revision 2.4  1993/10/21  01:23:01  drach
 * Changed name of sync options for consistency, added sync error.
 *
 * Revision 2.3  1993/10/20  17:16:29  drach
 * Define autosync options.
 *
 * Revision 2.2  1993/07/14  22:30:30  drach
 * Removed text after #endif, etc.
 *
 * Revision 2.1  1993/07/13  01:12:47  drach
 * Merged Sun, Unicos, SGI, and HP versions.
 *
# Revision 2.3  1992/10/14  23:14:54  drach
# Added putdic options
#
# Revision 2.2  1992/10/06  00:53:59  drach
# Added putdic errors.
#
# Revision 2.1  1992/05/22  01:07:07  drach
# removed drsmg (error messages structure)
#
# Revision 2.0  1992/03/07  00:12:24  drach
# Entered into RCS
#
                     Data Retrieval and Storage System

**********************************************************************

			DISCLAIMER

   This software was prepared as an account of work sponsored by an
   agency of the United States Government. Neither the United
   States Government nor the University of California nor any of
   their employees, makes any warranty, express or implied, or
   assumes any liability or responsibility for the accuracy,
   completeness, or usefulness of any information, apparatus,
   product, or process disclosed, or represents that its use would
   not infringe privately owned rights. Reference herein to any
   specific commercial products, process, or service by trade name,
   trademark, manufacturer, or otherwise, does not necessarily
   constitute or imply its endorsement, recommendation, or favoring
   by the United States Government or the University of California.
   The views and opinions of authors expressed herein do not
   necessarily state or reflect those of the United States
   Government or the University of California, and shall not be
   used for advertising or product endorsement purposes.
   
**********************************************************************



*/
/*#######################################################################
 *                drscdf.h
 *

 *#######################################################################*/
/*     General definitions */
#ifndef __drscdf_h
#define __drscdf_h

#define IDRS_NOVALUE 0
#define IDRS_DEFAULT 0
     
     
/*     DimensionType */
#define IDRS_EQUALLY_SPACED 1
#define IDRS_UNEQUALLY_SPACED 2

/*     FileStatus */
#define IDRS_READ 1
#define IDRS_CREATE 2
#define IDRS_EXTEND 3

/*     MachineName */
#define IDRS_SUN 1
#define IDRS_CRAY 2

#ifdef cray

#define IDRS_MACHINE IDRS_CRAY
#define IDRS_BYTES_PER_WORD 8
#define IDRS_NSL 8			     /* Aslun flag: file is on NSL UniTree */
#define IDRS_NOCOPY 16			     /* Aslun flag: do not copy dictionary from NSL on open */
#else
#define IDRS_MACHINE IDRS_SUN
#define IDRS_BYTES_PER_WORD 4

#endif

/*     DRSVersion */
/*     DRS_VERSION is current version */
/*     DRS_MAXVERSION is max allowed version before version */
/*        is considered 'novalue' */
#define DRS_VERSION 2.1
#define DRS_MAXVERSION 10.0
     
/*     Inquiry operators (INQDICT) */
#define IDRS_GETFIRSTVAR 1
#define IDRS_GETNEXTVAR 2

/*     Standard argument lengths */
#define IDRS_SOURCELEN 121
#define IDRS_NAMELEN 17
#define IDRS_TITLELEN 81
#define IDRS_UNITSLEN 41
#define IDRS_DATELEN 9
#define IDRS_TIMELEN 9
#define IDRS_TYPELEN 9

#ifdef cray

#define IDRS_FILENAMELEN 248
#else
#define IDRS_FILENAMELEN 1024

#endif


typedef char DRS_SOURCE[IDRS_SOURCELEN];
typedef char DRS_NAME[IDRS_NAMELEN];
typedef char DRS_TITLE[IDRS_TITLELEN];
typedef char DRS_UNITS[IDRS_UNITSLEN];
typedef char DRS_DATE[IDRS_DATELEN];
typedef char DRS_TIME[IDRS_TIMELEN];
typedef char DRS_TYPE[IDRS_TYPELEN];
typedef char DRS_FILENAME[IDRS_FILENAMELEN];
            
/*     Cray-to-IEEE translation parameters */
#define IDRS_LEFTHALFWORD 0
#define IDRS_RIGHTHALFWORD 4

/*     ElementType */
#define IDRS_I4 1
#define IDRS_I8 2
#define IDRS_IEEE_R4 3
#define IDRS_CRAY_R8 4
#define IDRS_ASCII 5
#define IDRS_USER 6
#ifndef __EXTENDED_DRS_DATATYPES
#define __EXTENDED_DRS_DATATYPES
#define IDRS_I1 7
#define IDRS_I2 8
#define IDRS_IEEE_R8 9
#define IDRS_CRAY_R16 10
#define IDRS_IEEE_R16 11
#endif

				/* Synchronization options */
#define IDRS_SYNC_OFF 1
#define IDRS_SYNC_ON 2

/*     Error reporting */
#define IDRS_NOREPORT 1
#define IDRS_FATAL 2
#define IDRS_WARNING 3
#define IDRS_INTERNAL 4
     
/*     Putdic options */
#define IDRS_BLANKS_ARE_NULL 1
#define IDRS_BLANKS_ARE_LITERAL 2

/*     Error definitions */
#define IDRS_SUCCESS 0
#define IDRS_NOMEMORY 1
#define IDRS_BINFAILED 2002
#define IDRS_BADLEN 3
#define IDRS_NOMONO 4
#define IDRS_NOCOMPARISON 2005
#define IDRS_VDBNOTFOUND 6
#define IDRS_BADDIM 7
#define IDRS_NOTMONOTONE 8
#define IDRS_DICTREADERROR 9
#define IDRS_NODICTFILE 10
#define IDRS_BADLU 11
#define IDRS_BADTYPE 12
#define IDRS_AMBIGUITYEXISTS 13
#define IDRS_CANNOTADDDATA 14
#define IDRS_DICTFULL 15
#define IDRS_VERSION1FILE 1016
#define IDRS_NEWFILEFORMAT 1017
#define IDRS_CANNOTREADHEADER 18
#define IDRS_CANNOTREADDATA 19
#define IDRS_BADDIMNAME 20
#define IDRS_TOOMANYFILES 21
#define IDRS_CANNOTOPENDICT 22
#define IDRS_CANNOTOPENDATA 23
#define IDRS_BADSTATUS 24
#define IDRS_BADDIMTYPE 25
#define IDRS_INDEXHIGH 2026
#define IDRS_INDEXLOW 2027
#define IDRS_INDEXBETWEEN 2028
#define IDRS_NORANGE 29
#define IDRS_SAVEBUFOVERFLOW 30
#define IDRS_BADERRLEVEL 31
#define IDRS_ERROROUTOFRANGE 32
#define IDRS_CANNOTWRITEHEADER 33
#define IDRS_CANNOTWRITEDATA 34
#define IDRS_BADCHARLEN 35
#define IDRS_BADOPER 36
#define IDRS_NOMOREVARS 1037
#define IDRS_DICTALREADYOPEN 38
#define IDRS_LOOKUPFAILED 2039
#define IDRS_DICTWRITEERROR 40
#define IDRS_DICTEXTENDERROR 41
#define IDRS_DATEXTENDERROR 42
#define IDRS_DICTRUNCATEERR 43
#define IDRS_DATTRUNCATEERR 44
#define IDRS_BADIEEEFP 45
#define IDRS_BADCRAYFP 46
#define IDRS_BADCRAYINT 47
#define IDRS_CANNOTCONVERT 48
#define IDRS_INEXACTMATCH 1049
#define IDRS_DUPLICATEVAR 50
#define IDRS_CANNOTWRITEDIC 51
#define IDRS_BADSYNCOPT 52

#undef PROTO
#ifdef __STDC__
#define PROTO(x) x
#else
#define PROTO(x) ()
#endif

extern int Aslun PROTO((int lud,char* dicfil,int lu,char* datfil,int istat));
extern int Cllun PROTO((int lu));
extern int Cluvdb PROTO((void));
extern int Drstest PROTO((int ierr));
extern int Getdat PROTO((int lu,void* a,int isize));
extern int Getcdim PROTO((int idim,char* source,char* name,char* title,char* units,int* dtype,int reqlen,float* var,int* retlen));
extern int GetcdimD PROTO((int idim,char* source,char* name,char* title,char* units,int* dtype,int reqlen,double* var,int* retlen));
extern int Getedim PROTO((int n,char* dsrc,char* dna,char* dti,char* dun,int* dtype,int* idim,float* df,float* dl));
extern int GetedimD PROTO((int n,char* dsrc,char* dna,char* dti,char* dun,int* dtype,int* idim,double* df,double* dl));
extern int Getelemd PROTO((int* type,int* bpe));
extern int Getname PROTO((char* source,char* name,char* title,char* units,char* date,char* time,char* typed,int* nd));
extern int Getrge2 PROTO((int lu,int idim,double elem1,double elem2,int* ind1,int* ind2,float* dlow,float* dhigh));
extern int Getslab PROTO((int lu,int rank,int* order,float* fe,float* le,float* cycle,void* data,int* datadim));
extern int Getvdim PROTO((int idim,char* source,char* title,int reqlen,float* var,int* retlen));
extern int Inqdict PROTO((int lu,int oper));
extern int Inqlun PROTO((int lu,char* datafile,int* nvar,float* version));
extern int Prdict PROTO((int lup,int lu));
extern int Putdat PROTO((int lu,void* a));
extern int Putdic PROTO((int lu, int iopt));
extern int Putvdim PROTO((int lu,int len,float* dimvar,int* i1,int* i2));
extern int Setdate PROTO((char* date,char* time));
extern int Setdim PROTO((int n,char* dna,char* dun,int idim,double df,double dl));
extern int Seterr PROTO((int ierrlun,int reportlevel));
extern int Setname PROTO((char* source,char* name,char* title,char* units,char* typed));
extern int Setrep PROTO((int irep));
extern int Setvdim PROTO((int n,char* dso,char* dna,char* dti,char* dun,double df,double dl));
#ifdef NSL_DRS
extern int drsn2lcp PROTO((char* infile, char* outfile));
extern int drsl2ncp PROTO((char* infile, char* outfile));
#endif

#endif

