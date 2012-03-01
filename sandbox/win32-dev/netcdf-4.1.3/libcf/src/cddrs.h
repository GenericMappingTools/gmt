/* -*-Mode: C;-*-
 * Module:      cddrs.h - include file for C DRS wrapper routines
 *
 * Copyright:	1994, Regents of the University of California
 *		This software may not be distributed to others without
 *		permission of the author.
 *
 * Author:      Bob Drach, Lawrence Livermore National Laboratory
 *              drach@llnl.gov
 *
 * Version:     $Id$
 *
 * Revision History:
 *
 * $Log: cddrs.h,v $
 * Revision 1.1.1.1  2009/07/06 15:06:30  ed
 * added libcf to netcdf dist
 *
 * Revision 1.1  2008/06/30 16:07:44  ed
 * Beginning merge of calandar stuff.
 *
 * Revision 1.13  1997/09/26  21:45:26  drach
 * - Added HDF
 * - Repaired fouled up cddrs includes
 *
 * Revision 1.10  1995/07/12  22:07:49  drach
 * - Add cw_get_fileid function
 *
 * Revision 1.9  1995/06/09  22:44:15  drach
 * Added extensions for string length and dimension types
 *
 * Revision 1.8  1995/02/15  20:54:58  drach
 * - Added IDRS_VECTOR as synonym for IDRS_UNEQUALLY_SPACED
 *
 * Revision 1.7  1995/01/21  00:52:24  drach
 * - Added compatibility defines
 *
 * Revision 1.6  1995/01/18  02:53:05  drach
 * - Added majority flags
 *
 * Revision 1.5  1995/01/13  01:02:30  drach
 * Added getnd prototype
 *
 * Revision 1.4  1994/12/17  00:42:37  drach
 * - removed CW_MAX_LU (use CU_MAX_LU in cdunif instead)
 *
 * Revision 1.3  1994/12/16  00:44:43  drach
 * - Included drscdf.h
 *
 * Revision 1.2  1994/12/14  02:32:43  drach
 * - Modified extern function declarations
 * - Added CwRoundPolicy typedef
 * - Added null defs
 *
 * Revision 1.1  1994/11/23  22:56:22  drach
 * Initial version.
 *
 *
 */

#ifndef __cddrs_h
#define __cddrs_h

#include "drscdf.h"

#define CW_STRING_NULL " "		     /* Null character string */
#define CW_FLOAT_NULL 1.0e20		     /* Null float arg */
#define CW_FLOAT_DELTA 1.0e14		     /* Treat floats as null if = CW_FLOAT_NULL +/- CW_FLOAT_DELTA */
#define CW_INT_NULL 0			     /* Null int arg */
#define CW_MAX_NAME 128			     /* Max characters in a name (= CU_MAX_NAME) */
#define IDRS_VECTOR IDRS_UNEQUALLY_SPACED    /* Synonym for unequally-spaced dimension flag */

					     /* For compatibility with older versions of drscdf.h */
#ifndef __EXTENDED_DRS_DATATYPES
#define __EXTENDED_DRS_DATATYPES
#define IDRS_I1 7
#define IDRS_I2 8
#define IDRS_IEEE_R8 9
#define IDRS_CRAY_R16 10
#define IDRS_IEEE_R16 11
#endif

typedef enum {CW_ROUND_NEAREST = 1, CW_ROUND_UP, CW_ROUND_DOWN, CW_RANGE} CwRoundPolicy;
typedef enum {CW_C_MAJORITY = 1, CW_FORTRAN_MAJORITY} CwMajority;
typedef enum {CW_STANDARD = 1, CW_EXTENDED} CwExtensionOption;
typedef enum {CW_LOCAL = 1, CW_SHARED, CW_IMPLICIT_SHARED} CwDimensionType;

#undef PROTO
#if defined(__STDC__) || (OS_NAME == AIX)
#define PROTO(x) x
#else
#define PROTO(x) ()
#endif

extern int cw_aslun PROTO((int lud,char* dicfil,int lu,char* datfil,int istat));
extern int cw_cllun PROTO((int lu));
extern int cw_cluvdb PROTO((void));
extern int cw_drstest PROTO((int ierr));
extern int cw_getdat PROTO((int lu,void* a,int isize));
extern int cw_getcdim PROTO((int idim,char* source,char* name,char* title,char* units,int* dtype,int reqlen,float* var,int* retlen));
extern int cw_getcdimD PROTO((int idim,char* source,char* name,char* title,char* units,int* dtype,int reqlen,double* var,int* retlen));
extern int cw_getedim PROTO((int n,char* dsrc,char* dna,char* dti,char* dun,int* dtype,int* idim,float* df,float* dl));
extern int cw_getedimD PROTO((int n,char* dsrc,char* dna,char* dti,char* dun,int* dtype,int* idim,double* df,double* dl));
extern int cw_getelemd PROTO((int* type,int* bpe));
extern int cw_get_fileid PROTO((int lu));
extern int cw_getname PROTO((char* source,char* name,char* title,char* units,char* date,char* time,char* typed,int* nd));
extern int cw_getnd PROTO((int* nd));
extern int cw_getrge2 PROTO((int lu,int idim,double elem1,double elem2,int* ind1,int* ind2,float* dlow,float* dhigh));
extern int cw_getrge2D PROTO((int lu,int idim,double elem1,double elem2,int* ind1,int* ind2,double* dlow,double* dhigh));
extern int cw_getslab PROTO((int lu,int rank,int* order,float* fe,float* le,float* cycle,void* data,int* datadim));
extern int cw_inqdict PROTO((int lu,int oper));
extern int cw_inqlun PROTO((int lu,char* datafile,int* nvar,float* version));
extern int cw_majority PROTO((CwMajority majority));
extern int cw_set_dimension_option PROTO((CwExtensionOption option));
extern int cw_set_string_option PROTO((CwExtensionOption option));
extern int cw_setdim PROTO((int n,char* dna,char* dun,int idim,double df,double dl));
extern int cw_seterr PROTO((int ierrlun,int reportlevel));
extern int cw_setname PROTO((char* source,char* name,char* title,char* units,char* typed));
extern int cw_setvdim PROTO((int n,char* dso,char* dna,char* dti,char* dun,double df,double dl));

					     /* Compatibility functions */
extern int cw_putdat PROTO((int lu,void* a));
extern int cw_putdic PROTO((int lu, int iopt));
extern int cw_putvdim PROTO((int lu,int len,float* dimvar,int* i1,int* i2));
extern int cw_setdate PROTO((char* date,char* time));
extern int cw_setrep PROTO((int irep));

#ifdef CDCOMPAT
#define Aslun cw_aslun
#define Cllun cw_cllun
#define Cluvdb cw_cluvdb
#define Drstest cw_drstest
#define Getdat cw_getdat
#define Getcdim cw_getcdim
#define GetcdimD cw_getcdimD
#define Getedim cw_getedim
#define GetedimD cw_getedimD
#define Getelemd cw_getelemd
#define Getname cw_getname
#define Getnd cw_getnd
#define Getrge2 cw_getrge2
#define Getslab cw_getslab
#define Inqdict cw_inqdict
#define Inqlun cw_inqlun
#define Majority cw_majority
#define Setdim cw_setdim
#define Seterr cw_seterr
#define Setname cw_setname
#define Setvdim cw_setvdim
#define Putdat cw_putdat
#define Putdic cw_putdic
#define Putvdim cw_putvdim
#define Setdate cw_setdate
#define Setrep cw_setrep
#endif

#endif
