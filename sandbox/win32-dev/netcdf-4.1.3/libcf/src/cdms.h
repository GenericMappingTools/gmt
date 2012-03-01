/*
 * Module:      cdms.h - CDMS user-level definitions
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
 * $Log: cdms.h,v $
 * Revision 1.1.1.1  2009/07/06 15:06:30  ed
 * added libcf to netcdf dist
 *
 * Revision 1.1  2008/06/30 16:07:44  ed
 * Beginning merge of calandar stuff.
 *
 * Revision 1.1.1.1  1997/12/09 18:57:39  drach
 * Copied from cirrus
 *
 * Revision 1.11  1997/11/24  17:28:03  drach
 * - Added QL package to cdunif
 * - Added NdimIntersect function to CDMS
 *
 * Revision 1.10  1997/11/10  19:22:28  drach
 * - Added cuvargets to cdunif, cdSlabRead to cdms
 *
 * Revision 1.9  1997/10/24  18:23:34  drach
 * - Cache netCDF unlimited dimensions
 * - Consistent with GrADS src170
 *
 * Revision 1.8  1996/04/04  21:48:02  drach
 * - Minor fix for SGI version
 *
 * Revision 1.7  1996/04/04  18:27:02  drach
 * - Added CDMS inquiry, lookup, open, and internal routines
 *
 * Revision 1.6  1996/02/23  01:21:21  drach
 * - Moved most of cdms.h to cdmsint.h (original in cdms_v1.h)
 * - Added new time model declarations to cdms.h
 * - Added -DNO_DECLARE flag to fcddrs.h
 *
 * Revision 1.5  1994/11/17  20:02:29  drach
 * Moved #endif
 *
 * Revision 1.4  1994/10/25  00:58:51  drach
 * - Added dset and var tags for get functions
 * - Added CdDeleteDset and CdDeleteVar prototypes
 *
 * Revision 1.3  1994/08/12  21:45:33  drach
 * - modified access fields to longs
 * - added dimension directions to order struct
 * - changed geom floats to doubles
 * - added level, sset struct
 * - changed varHandle args to var
 * - added templates for new routines
 *
 * Revision 1.2  1994/07/19  23:57:53  drach
 * - Added CdMinute
 * - changed unsigned struct components to signed
 *
 * Revision 1.1  1994/07/13  18:29:29  drach
 * Initial version
 *
 *
 */


/*
 * =================================================================
 *			Macros and Enums
 * =================================================================
 */
#ifndef _CDMS_H
#define _CDMS_H

#include "cddrs.h"
#include "cdunif.h"

#define CD_ATT_MAX 10240		     /* Max bytes in attribute */
#define CD_DEFAULT_DB 0
#define CD_DIM_OFFSET 0x80000		     /* Dimension ID offset */
#define CD_GLOBAL 0			     /* Null variable ID */
#define CD_GLOB_DIM_OFFSET 0x200000	     /* Global dimension offset */
#define CD_GLOB_GRID_OFFSET 0x400000	     /* Global grid offset */
#define CD_GRID_OFFSET 0x100000		     /* Grid ID offset */
#define CD_LAST_DAY -1			     /* Last day of month */
#define CD_MAX_ABSUNITS 64		     /* Max characters in absolute units */
#define CD_MAX_ABS_COMPON 7		     /* Max components in absolute time format */
#define CD_MAX_CHARTIME 48		     /* Max characters in character time */
#define CD_MAX_NAME CU_MAX_NAME		     /* Max characters in a name */
#define CD_MAX_PATH CU_MAX_PATH		     /* Max characters in a file pathname */
#define CD_MAX_RELUNITS 64		     /* Max characters in relative units */
#define CD_MAX_TIME_DELTA 64		     /* Max characters in time delta */
#define CD_MAX_VAR_COMPS 32		     /* Max number of variable components */
#define CD_MAX_VAR_DIMS CU_MAX_VAR_DIMS	     /* Max dimensions in a variable */
#define CD_NULL_DAY 1			     /* Null day value */
#define CD_NULL_HOUR 0.0		     /* Null hour value */
#define CD_NULL_ID 0			     /* Reserved ID */
#define CD_NULL_MONTH 1			     /* Null month value */
#define CD_NULL_YEAR 0			     /* Null year value, component time */
#define CD_ROOT_ENV "CDMSROOT"		     /* CDMS root environment variable */
#define CD_SUCCESS 0

typedef enum cdType {cdInvalidType = -1,
			     cdByte = CuByte,
			     cdChar = CuChar,
			     cdShort = CuShort,
			     cdInt = CuInt,
			     cdLong = CuLong,
			     cdFloat = CuFloat,
			     cdDouble = CuDouble,
			     cdLongDouble = CuLongDouble,
			     cdCharTime
} cdType;

typedef enum cdIntersectPolicy {cdRoundNearest = CW_ROUND_NEAREST,
			    cdRange = CW_RANGE
} cdIntersectPolicy;

typedef enum cdRepresent {cdExternalArray = 1, cdInternalArray, cdLinearRep} cdRepresent;
typedef enum cdOpenMode {cdReadOnly = 1, cdReadWrite} cdOpenMode;
typedef enum cdGridType {cdRectangular = 1, cdZonal, cdMeridional, cdProjected, cdQuasiRegular} cdGridType;
typedef enum cdDimType {cdLongitude = 1, cdLatitude, cdLevel, cdCalendar, cdClimatology,
				cdMonotonic} cdDimType;
typedef enum cdDimShape {cdLinearShape = 1, cdWrapped} cdDimShape;
typedef enum cdPolygonType {cdBox = 1, cdAnnulus, cdLoc, cdPoly, cdGlobal} cdPolygonType;
typedef enum cdMonths {cdJan = 1, cdFeb, cdMar, cdApr, cdMay, cdJun, cdJul, cdAug,
			       cdSep, cdOct, cdNov, cdDec } cdMonths;
typedef enum cdSeasons {cdDJF = cdDec,	/* DJF */
				cdMAM = cdMar,		/* MAM */
				cdJJA = cdJun,		/* JJA */
				cdSON = cdSep} cdSeasons;		/* SON */

typedef enum cdIntersectOpt {cdSubset = 1, cdPoint, cdIntersect} cdIntersectOpt;

enum cdLevelType {cdUnknown=0, cdTwoMeter, cdAtmosphere, cdHybrid, cdPressure, cdSeaLevel, cdSigma, cdSkin,
			   cdSoil, cdSurface, cdTopAtmos};

					     /* Dean's DRS-like functions */

typedef enum {CE_C_MAJORITY = 1, CE_FORTRAN_MAJORITY} CeMajority;
#define CU_C_MAJORITY CE_C_MAJORITY
#define CU_FORTRAN_MAJORITY CE_FORTRAN_MAJORITY

                                             /* Note: if time stuff changes, so should fcdms.h!!  */
#define cdStandardCal   0x11
#define cdClimCal        0x0
#define cdHasLeap      0x100
#define cdHasNoLeap    0x000
#define cd365Days     0x1000
#define cd360Days     0x0000
#define cdJulianCal  0x10000
#define cdMixedCal   0x20000

typedef enum cdCalenType {
	cdStandard    = ( cdStandardCal | cdHasLeap   | cd365Days),
	cdJulian      = ( cdStandardCal | cdHasLeap   | cd365Days | cdJulianCal),
	cdNoLeap      = ( cdStandardCal | cdHasNoLeap | cd365Days),
	cd360         = ( cdStandardCal | cdHasNoLeap | cd360Days),
	cdClim        = ( cdClimCal     | cdHasNoLeap | cd365Days),
	cdClimLeap    = ( cdClimCal     | cdHasLeap   | cd365Days),
	cdClim360     = ( cdClimCal     | cdHasNoLeap | cd360Days),
	cdMixed       = ( cdStandardCal | cdHasLeap   | cd365Days | cdMixedCal)
}  cdCalenType;

/*
 * =================================================================
 *			Structures
 * =================================================================
 */

/* Component time */
typedef struct {
	long 		year;		     /* Year */
	short 		month;		     /* Numerical month (1..12) */
	short 		day;		     /* Day of month (1..31) */
	double 		hour;		     /* Hour and fractional hours */
} cdCompTime;

/*
 * =================================================================
 *			Function Prototypes
 * =================================================================
 */
extern int cdDimInq(long dsetid, long dimid, char* name, char* alias, cdType* datatype, int* natts, cdDimType* type, int* subtype, long* length, cdDimShape* topology);
extern int cdDsetClose(long dsetid);
extern int cdDsetInq(long dsetid, int* ndims, int* nvars, int* ngrids, int* ngatts);
extern int cdGridInq(long dsetid, long gridid, char* name, cdGridType* type, int* ndims, long dim[], int* natts);
extern int cdVarGetCoord(long dsetid, long varid, long dimid, double indices[]);
extern int cdVarGetIndex(long dsetid, long varid, long dimid, long indices[]);
extern int cdVarGetIndexRange(long dsetid, long varid, long dimid, long* index1, long* index2);
extern int cdVarInq(long dsetid, long varid, char* name, char* alias, cdType* datatype, int* ndims, long dim[], int* natts);
extern int cdVarSetCoordRange(long dsetid, long varid, long dimid, double coord1, double coord2, cdIntersectOpt opt, long* len);
extern int cdVarSetIndex(long dsetid, long varid, long dimid, long nindices, long index[], long* len);
extern int cdVarSetIndexRange(long dsetid, long varid, long dimid,  long index1, long index2, long* len);
extern long cdDimLookup(long dsetid, char* name);
extern long cdDimLookupAlias(long dsetid, char* alias);
extern long cdDsetOpen(const char* path, cdOpenMode mode, long dbid);
extern long cdGridLookup(long dsetid, char* name);
extern long cdVarLookup(long dsetid, char* name);
extern long cdVarLookupAlias(long dsetid, char* alias);

extern void cdChar2Comp(cdCalenType timetype, char* chartime, cdCompTime* comptime);
extern void cdChar2Rel(cdCalenType timetype, char* chartime, char* relunits, double* reltime);
extern void cdComp2Char(cdCalenType timetype, cdCompTime comptime, char* time);
extern void cdComp2Rel(cdCalenType timetype, cdCompTime comptime, char* relunits, double* reltime);
extern void cdRel2Char(cdCalenType timetype, char* relunits, double reltime, char* chartime);
extern void cdRel2Comp(cdCalenType timetype, char* relunits, double reltime, cdCompTime* comptime);
extern void cdRel2Rel(cdCalenType timetype, char* relunits, double reltime, char* outunits, double* outtime);
extern int cdAbs2Comp(char *absunits, void *abstime, cdType abstimetype, cdCompTime *comptime, double *frac);
extern int cdComp2Abs(cdCompTime comptime, char *absunits, cdType abstimetype, double frac, void *abstime);
extern int cdDecodeRelativeTime(cdCalenType timetype, char* units, double time, cdCompTime* comptime);
extern int cdDecodeAbsoluteTime(char* units, void* time, cdType abstimetype, cdCompTime* comptime, double* fraction);

extern long cdSlabRead(long dsetid, long varid, const long order[], const double first[], const double last[], const double modulus[], cdIntersectPolicy policy, cdType usertype, void *values);
extern long cdNdimIntersect(long dsetid, long varid, const long order[], const double first[], const double last[], const double modulus[], cdIntersectPolicy policy, long start[], long count[], long stride[]);

extern int cdTypeLen(cdType datatype);

					     /* Dean's DRS-like extensions */
extern int cedimgeta(int fileid, int dimid, void* values, void **charvalues[]);
extern int cevargeta(int fileid, int varid, const long start[], const long count[], void* value);
extern int cemajority(CeMajority majority);
extern int cehyperslabinq(int fileid, int varid, int rank,long* order,double* fe,double* le,double* cycle, long *data_arraysize, int *dim_arraylengths);
extern int cehyperslab(int fileid, int varid, int rank,long* order,double* fe,double* le,double* cycle,double *dimarray, void *data);
extern int cehyperslaba(int fileid, int varid, int rank,long* order,double* fe,double* le,double* cycle,long dimsize, double *dimarray, void *data);
extern int cehyperslabi(int fileid, int varid, int rank,long* order,long* fe,long* le,double* cycle,void **data, int *dimsize, double *dimarray[]);


#define cudimgeta cedimgeta
#define cuvargeta cevargeta
#define cumajority cemajority
#define cuhyperslabinq cehyperslabinq
#define cuhyperslab cehyperslab
#define cuhyperslaba cehyperslaba
#define cuhyperslabi cehyperslabi

/*
 * =================================================================
 *			Globals
 * =================================================================
 */

extern CeMajority ceMajority; /* Default majority for Dean's extensions */


/*
 * =================================================================
 *	This stuff is used in the original time stuff,
 *      should eventually be moved to cdmsint.h
 * =================================================================
 */

typedef enum CdMonths {CdJan = 1, CdFeb, CdMar, CdApr, CdMay, CdJun, CdJul, CdAug,
			       CdSep, CdOct, CdNov, CdDec } CdMonths;

typedef enum CdSeasons {CdWinter = CdDec,	/* DJF */
				CdSpring = CdMar,		/* MAM */
				CdSummer = CdJun,		/* JJA */
				CdFall = CdSep} CdSeasons;		/* SON */

typedef enum CdTimeUnit {
	CdMinute = 1,
	CdHour = 2,
	CdDay = 3,
	CdWeek = 4,			     /* Always = 7 days */
	CdMonth = 5,
	CdSeason = 6,			     /* Always = 3 months */
	CdYear = 7,
	CdSecond = 8
} CdTimeUnit;

#define CdChronCal    0x1
#define CdClimCal     0x0
#define CdBaseRel    0x00
#define CdBase1970   0x10
#define CdHasLeap   0x100
#define CdNoLeap    0x000
#define Cd365      0x1000
#define Cd360      0x0000
#define CdJulianType 0x10000

typedef enum CdTimeType {
	CdChron       = ( CdChronCal | CdBase1970 | CdHasLeap | Cd365),	/* 4369 */
	CdJulianCal   = ( CdChronCal | CdBase1970 | CdHasLeap | Cd365 | CdJulianType),
	CdChronNoLeap = ( CdChronCal | CdBase1970 | CdNoLeap  | Cd365),	/* 4113 */
	CdChron360    = ( CdChronCal | CdBase1970 | CdNoLeap  | Cd360),	/*   17 */
	CdRel         = ( CdChronCal | CdBaseRel  | CdHasLeap | Cd365),	/* 4353 */
	CdRelNoLeap   = ( CdChronCal | CdBaseRel  | CdNoLeap  | Cd365),	/* 4097 */
	CdClim        = ( CdClimCal  | CdBaseRel  | CdNoLeap  | Cd365), /* 4096 */
	CdClimLeap    = ( CdClimCal  | CdBaseRel  | CdHasLeap | Cd365),
	CdClim360     = ( CdClimCal  | CdBaseRel  | CdNoLeap  | Cd365)
}  CdTimeType;

#define CdNullYear 0
#define CdNullMonth CdJan
#define CdNullDay 1
#define CdLastDay -1
#define CdNullHour 0.0

typedef struct {
	long    		year;	     /* e.g., 1979 */
	short			month;	     /* e.g., CdDec */
	short			day;	     /* e.g., 30 */
	double			hour;	     /* hour and fractional hour */
	long			baseYear;    /* base year for relative, 1970 for CdChron */
	CdTimeType		timeType;    /* e.g., CdChron */
} CdTime;

typedef struct {
	long   			count;	     /* units count  */
	CdTimeUnit		units;	     /* time interval units */
} CdDeltaTime;

#endif
