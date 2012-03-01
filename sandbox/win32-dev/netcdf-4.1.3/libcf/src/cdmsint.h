/* -*-Mode: C;-*-
 * Module:      cdmsint.h - CDMS internal database definitions
 *
 * Copyright:	1995, Regents of the University of California
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
 * $Log: cdmsint.h,v $
 * Revision 1.1.1.1  2009/07/06 15:06:30  ed
 * added libcf to netcdf dist
 *
 * Revision 1.1  2008/06/30 16:07:44  ed
 * Beginning merge of calandar stuff.
 *
 * Revision 1.1.1.1  1997/12/09 18:57:39  drach
 * Copied from cirrus
 *
 * Revision 1.3  1997/10/24  18:23:35  drach
 * - Cache netCDF unlimited dimensions
 * - Consistent with GrADS src170
 *
 * Revision 1.2  1996/04/04  18:27:04  drach
 * - Added CDMS inquiry, lookup, open, and internal routines
 *
 * Revision 1.1  1996/02/23  01:21:23  drach
 * - Moved most of cdms.h to cdmsint.h (original in cdms_v1.h)
 * - Added new time model declarations to cdms.h
 * - Added -DNO_DECLARE flag to fcddrs.h
 *
 *
 */

#ifndef _CDMSINT_H
#define _CDMSINT_H

#include <stdio.h>
#include <stddef.h>
#include "cdms.h"

/*
 * =================================================================
 *			Macros and Typedefs
 * =================================================================
 */

#define CD_NAME_DUP -2
#define CD_ALIAS_DUP -3
#define cdSymtabNentries(table) (table)->nentries /* number of entries in symbol table */
#define cdDsetVar(dset,varid) (cdVar*)((dset)->vars->entries)+((varid)-1) /* dset variable with id varid */
#define cdDsetDim(dset,dimid) (cdDim*)((dset)->dims->entries)+((dimid)-CD_DIM_OFFSET-1) /* dset dimension with id dimid */
#define cdDsetGrid(dset,gridid) (cdGrid*)((dset)->grids->entries)+((gridid)-CD_GRID_OFFSET-1) /* dset dimension with id dimid */

#define cdSetByte(vp,val) {unsigned char *ucp; ucp=(unsigned char*)(vp); *ucp=(val);}
#define cdSetShort(vp,val) {short *sp; sp=(short*)(vp); *sp=(val);}
#define cdSetInt(vp,val) {int *ip; ip=(int*)(vp); *ip=(val);}
#define cdSetLong(vp,val) {long *lp; lp=(long*)(vp); *lp=(val);}
#define cdSetFloat(vp,val) {float *fp; fp = (float *)(vp); *fp=(val);}
#define cdSetDouble(vp,val) {double *dp; dp=(double*)(vp); *dp=(val);}
#if !defined(sgi) && !defined(__alpha)
#define cdSetLongDouble(vp,val) {long double *ldp; ldp=(long double*)(vp); *ldp=(val);}
#endif
#define cdSetCharTime(vp,val) {char *cp; cp=(char*)(vp); strncpy(cp,(val),CD_MAX_CHARTIME); cp[CD_MAX_CHARTIME-1]='\0';}

					     /* For Dean's DRS-like extensions */

#define CE_FLOAT_NULL 1.0e20   /* Null float arg */
#define CE_FLOAT_DELTA 1.0e14  /* Treat floats as null if = CE_FLOAT_NULL +/- CE_FLOAT_DELTA */
#define CE_INT_NULL 0
#define CE_IS_FLOAT_NULL(x) (fabs((x)-CE_FLOAT_NULL)<=CE_FLOAT_DELTA)

typedef enum {CE_ROUND_NEAREST = 1, CE_ROUND_UP, CE_ROUND_DOWN, CE_RANGE} CeRoundPolicy;

typedef enum cdUnit {
	cdMinute = CdMinute,
	cdHour = CdHour,
	cdDay = CdDay,
	cdWeek = CdWeek,			     /* Always = 7 days */
	cdMonth = CdMonth,
	cdSeason = CdSeason,			     /* Always = 3 months */
	cdYear = CdYear,
	cdSecond = CdSecond,
	cdFraction			     /* Fractional part of absolute time */
} cdUnitTime;

/*
 * =================================================================
 *			Structures
 * =================================================================
 */

/* Symbol table index entry */
typedef struct {
	char name[CD_MAX_NAME];		     /* Name (key) */
	int index;			     /* Index into symbol table (0-origin) */
	short deleted;			     /* 1 iff entry is marked for deletion */
} cdTag;

/* Symbol table */
typedef struct {
	cdTag *nameIndex;		     /* Sorted name index */
	cdTag *aliasIndex;		     /* Sorted alias index */
	int nentries;			     /* Number of table entries */
	int length;			     /* Allocated slots for entries, may be > nentries */
	size_t entrySize;		     /* Size of each entry*/
	void *entries;			     /* Symbol table */
} cdSymtab;

/* Attribute */
typedef struct cdAtt {
	char            name[CD_MAX_NAME]; /* Attribute name */
	cdType          datatype;            /* Datatype of the attribute */
	long            length;		     /* Number of elements (NOT bytes) */
	void*           values;		     /* Pointer to internal structure for GrADS */
	struct cdAtt*   next;		     /* Next attribute */
} cdAtt;

/* Dataset */
typedef struct cdDset {
	long 		id;		     /* Dataset ID */
	long            dbid;		     /* Database ID */
	char 		path[CD_MAX_PATH];   /* Dataset pathname */
	char		fileTemp[CD_MAX_PATH]; /* File template */
	cdSymtab*       vars;		     /* Variables */
	cdSymtab*       dims;		     /* Dimensions */
	cdSymtab*       grids;		     /* Grids */
	int             ngatts;		     /* Number of global attributes */
	cdAtt*          atts;		     /* Global attributes */
	struct cdDset*  next;		     /* Next dataset */
} cdDset;

/* Database */
typedef struct cdDb {
	long            id;		     /* Database ID */
	char            path[CD_MAX_PATH];   /* Database pathname */
	cdDset*         dsets;		     /* Datasets */
	struct cdDb*    next;		     /* Next database */
} cdDb;

/* Parameter */
typedef struct {
	char 		name[CD_MAX_NAME];   /* Parameter name */
	char 		longName[CD_MAX_NAME]; /* Descriptive name */
	char 		units[CD_MAX_NAME];  /* Standard units */
} cdParm;

/* Polygon */
typedef struct {
	long 		id;		     /* Polygon ID */
	cdPolygonType 	type;		     /* cdBox, cdAnnulus, etc. */
	long 		length;		     /* Number of points */
	struct {
		double  lon;		     /* Longitude point */
		double  lat;		     /* Latitude point */
	} 		*points;	     /* Points */
} cdPolygon;

/* Variable */
typedef struct {
	long 		id;		     /* Variable ID */
	long		dsetid;		     /* Dataset ID */
	char		name[CD_MAX_NAME];   /* Parameter name (official) */
	char		alias[CD_MAX_NAME];  /* Name in file */
	char		aliasTitle[CD_MAX_NAME];  /* Title in file */
	int             natts;		     /* Number of attributes */
	cdAtt*          atts;		     /* Attribute list */
	int 		ncomps;		     /* Number of components */
	long		comps[CD_MAX_VAR_COMPS]; /* Component variable IDs, if ncomps>1 */
	void*           data;		     /* Data for implicit array definition */
	cdType		datatype;	     /* cdChar, cdInt, etc. */
	char		delta[CD_MAX_TIME_DELTA]; /* Delta, for linear rep (datatype is double, long, or chartime) */
	long            length;		     /* Number of elements */
	int 		ndims;		     /* Number of dimensions */
	long		dim[CD_MAX_VAR_DIMS];	/* Dimension IDs */
	char		fileTemp[CD_MAX_PATH];  /* File template */
	long 		gridid;		     /* Grid ID, if spatio-temporal */
	char 		initVal[CD_MAX_CHARTIME]; /* Initial value, for linear rep (datatype is double, long, or chartime)*/
	long 		levelid;	     /* Level ID, if spatio-temporal */
	char 		longName[CD_MAX_NAME];  /* Parameter title */
	cdRepresent	rep;		     /* cdExternalArray, cdInternalArray, cdLinear, ... */
	char 		stat[CD_MAX_NAME];   /* Statistic */
	long 		timeid;		     /* Time ID, if spatio-temporal */
	char 		units[CD_MAX_NAME];  /* Units in file */
} cdVar;

/* Dimension */
typedef struct cdDim{
	long 		id;		     /* Dimension ID */
	char 		name[CD_MAX_NAME];   /* Dimension name */
	char            alias[CD_MAX_NAME];  /* Dimension alias */
	int             natts;		     /* Number of attribtues */
	cdAtt*          atts;		     /* Attributes */
	cdDimType 	type;		     /* cdLongitude, cdLatitude, etc. */
	cdType          datatype;	     /* Dimension datatype */
	int 		subtype;	     /* cdSigma, cdJulian, etc., interpret using type */
	long 		length;		     /* Number of elements */
	int 		topology;	     /* E.g., cdWrapped */
	int             unlimited;	     /* 0 if fixed, 1 if unlimited length (extensible) */
	cdVar*  	boundary;	     /* Boundary variable if any */
	struct cdDim*   partition;	     /* variable with partition indices */
	cdRepresent	rep;		     /* cdExternalArray, cdInternalArray, cdLinear, ... */
	char            units[CD_MAX_NAME];  /* Dimension units */
	char		delta[CD_MAX_TIME_DELTA]; /* Delta, for linear rep (datatype is double, long, or chartime) */
	char 		initVal[CD_MAX_CHARTIME]; /* Initial value, for linear rep (datatype is double, long, or chartime) */
	void*           data;		     /* Data for implicit array definition */
} cdDim;

/* Grid */
typedef struct {
	long            id;		     /* Grid ID */
	char 		name[CD_MAX_NAME];   /* Grid name */
	int             natts;		     /* Number of attributes */
	cdAtt*          atts;		     /* Attribute list */
	cdGridType 	type;		     /* Grid type (rectangular, projected, ...) */
	long 		familyid;	     /* Grid family ID */
	long 		lon;		     /* Longitude dimension ID */
	long 		lat;		     /* Latitude dimension ID */
	long            length;		     /* Number of zones */
	int             ndims;		     /* Number of dimensions */
	long            dim[CD_MAX_VAR_DIMS]; /* Dimensions (IDs), in order of definition */
	cdVar* 		mask;		     /* Mask variable if any */
	double 		projArgs[3];	     /* Projection arguments */
} cdGrid;

/*
 * =================================================================
 *			Function Prototypes
 * =================================================================
 */

extern cdDb* cdDbLookupId(long dbid);
extern cdDset* cdDsetLookup(cdDb* db, const char* path);
extern cdDset* cdDsetLookupId(long dsetid);
extern cdDset* cdDsetInsert(cdDb* db, const char* path);
extern int cdDsetDelete(long dsetid);
extern void cdAttlistDestroy(cdAtt* attlist);
extern cdAtt* cdAttInsert(char* name, cdAtt** attlist);
extern cdAtt* cdAttlistGet(cdDset* dset, long objid);

extern void cdError(char *fmt, ...);
extern int cdInit(void);
extern cdSymtab *cdSymtabCreate(int nentries, size_t entrySize);
extern void cdSymtabDestroy(cdSymtab *table);
extern int cdSymDelete(cdSymtab *table, int id);
extern int cdSymInsert(cdSymtab *table, char *name, char *alias, void *entry);
extern int cdSymLookup(int nindex, cdTag *indexTable, char *key, int *loc);
extern int cdSymLookupName(cdSymtab *table, char *name, int *loc, void **entry);
extern int cdSymLookupAlias(cdSymtab *table, char *alias, int *loc, void **entry);

					     /* Basic time routines */
int cdParseRelunits(cdCalenType timetype, char* relunits, cdUnitTime* unit, cdCompTime* base_comptime);
int cdParseDeltaTime(cdCalenType timetype, char* deltaTime, double* value, cdUnitTime* unit);
extern void CdDayOfYear(CdTime *date, int *doy);
extern void CdDaysInMonth(CdTime *date, int *days);
extern void CdError(char *fmt, ...);
extern void CdMonthDay(int *doy, CdTime *date);
extern void Cdc2e(char *ctime, CdTimeType timeType, double *etime, long *baseYear);
extern void Cdc2h(char *ctime, CdTimeType timeType, CdTime *htime);
extern void Cde2c(double etime, CdTimeType timeType, long baseYear, char *ctime);
extern void Cde2h(double etime, CdTimeType timeType, long baseYear, CdTime *htime);
extern void Cdh2c(CdTime *htime, char *ctime);
extern void Cdh2e(CdTime *htime, double *etime);

					     /* Dean's DRS-like extensions */
extern int cealterdata(int fileid, int varid, const long count[], void* value);
extern int ce_dimget(int fileid, int dimid, double** values, long *dimlen);
extern void ce_lookup(double tab[], long n, double x, long *k);
extern int ce_lookup_with_policy(double tab[], long n, double x, CeRoundPolicy policy, double delta, long *k);
extern int ce_lookup_cycle(double tab[], long n, double x, double cycle, CeRoundPolicy policy, double delta,
			   long *k, long *icycle);
extern int ce_dimmap(int fileid, int dimid, double df, double dl, CeRoundPolicy policy, double delta, int isCycle,
		     double cycle, double** dp, long *idf, long *idl, double *xdf, double *xdl);
extern int ce_dimmapi(int fileid, int dimid, double df, double dl, CeRoundPolicy policy, double delta, int isCycle,
		      double cycle, double** dp, long *idf, long *idl, double *xdf, double *xdl);
extern int cevarinq(int fileid, int varid, char* name, CuType* datatype, int* ndims, int dimids[], int* natts);

/*
 * =================================================================
 *			Globals
 * =================================================================
 */

extern cdDb* cdCurrentDb;		     /* Database for dataset currently being parsed */
extern cdDset* cdCurrentDset;		     /* Dataset currently being parsed */
extern int cdLineno;			     /* Line number of dataset file currently being parsed */
extern long cdNextDimid;		     /* Next dimension ID */
extern long cdNextGridid;		     /* Next grid ID */
extern long cdNextVarid;		     /* Next variable ID */
extern int cdYyerrors;			     /* Number of parsing errors */




/* NB!!! Everything below this line is OBSOLETE!! */

typedef enum CdDsetType {CdExperiment = 1, CdObservation} CdDsetType;
typedef enum CdGridType {CdRegistered = 1, CdSpectral = 2} CdGridType;
typedef enum CdOpenMode {CdReadOnly = 1, CdReadWrite} CdOpenMode;
typedef enum CdQaLevel {CdUnvalidated = 1, CdValidated = 2} CdQaLevel;

					     /* See cdunif.h */
typedef enum CdType {CdInvalidType = -1,
			     CdByte = CuByte,
			     CdChar = CuChar,
			     CdShort = CuShort,
			     CdInt = CuInt,
			     CdLong = CuLong,
			     CdFloat = CuFloat,
			     CdDouble = CuDouble,
			     CdLongDouble = CuLongDouble} CdType;

typedef enum CdDimensionType {
	CdXDimension = 1,
	CdYDimension,
	CdZDimension,
	CdTDimension,
	CdPDimension
} CdDimensionType;

					     /* Sequential access cursor operations */
typedef enum {
	CdFirst = 1,
	CdLast,
	CdNext,
	CdPrev
} CdCursorOp;

/* predefined statistics */
#define CdMonthlyMeans "monthly mean"

/*
 * =================================================================
 *			Structures
 * =================================================================
 */

typedef struct {
	long   			id;	     /* Database ID */
	char			name[CU_MAX_PATH]; /* Full dataset pathname */
	CdDsetType		dsetType;    /* Experimental or observed */
} CdDsetDes;
typedef CdDsetDes *CdDset;

/* Order descriptor */
/* Each value is a dimension index, 0 .. 4, indicating */
/* the order of the dimensions on output. C-majority is observed, */
/* that is, dimension 0 is LEAST rapidly varying. A negative value */
/* indicates that the dimension is not applicable. For direction, a positive value */
/* indicates increasing values, negative indicates decreasing */
/* values. Direction for parameters refers to the canonical */
/* ordering as defined in the database. */

typedef struct {
	short  		parm;		     /* Parameter dimension, for compound elements */
	short		lon;		     /* Longitude dimension */
	short		lat;		     /* Latitude dimension */
	short		lev;		     /* Level dimension */
	short		time;		     /* Time dimension */
	short  		parmDir;	     /* Parameter dimension direction, for compound elements */
	short		lonDir;		     /* Longitude dimension direction */
	short		latDir;		     /* Latitude dimension direction */
	short		levDir;		     /* Level dimension direction */
	short		timeDir;	     /* Time dimension direction */
} CdOrder;

typedef struct {			/* specification of range of data to return */
        int 		bgnColNum; 	/* begining column number to read */
        int 		colCnt;   	/* count of colums to read */
        int 		bgnRowNum; 	/* beginning row number to read */
        int 		rowCnt;		/* count of rows to read */
	int		bgnLevNum;
	int		levCnt;
        double		bgnTime; 	/* beginning time to read; depends on timeType */
        int 		stepCnt;	/* count of time steps to read */
} CdRegion;

typedef struct {
	char   prjnName[21];		/* projection name */
	long   nx;			/* count of columns */
	long   ny;			/* count of rows */
	double lat;			/* lat of origin in degrees */
	double lon;			/* lon of origin in degrees */
	long   orig_ix;			/* column # for origin, left column is 1 */
	long   orig_iy;			/* row # for origin; top row is 1 */
	double x_int_dis;		/* distance interval between columns in km */
	double y_int_dis;		/* distance interval between rows in km */
	double parm_1;			/* geom parm 1, depends on projection */
	double parm_2;			/* geom parm 2, depends on projection */
	double parm_3;			/* geom parm 3, depends on projection */
} CdRegGeom;

typedef struct {			/* info from table grid_spct_geom */
	char   trnc_type[21];		/* spectral truncation type (triangular/etc) */ 
	long   coef_cnt;		/* count of complex coefficients used */ 
	long   max_lat_wav_num;		/* max latitudinal wavenumber (M in GRIB) */
	long   max_lon_wav_num_1;	/* max longitudinal wavenumber 1 (J in GRIB) */
	long   max_lon_wav_num_2;	/* max longitudinal wavenumber 2 (K in GRIB) */
} CdSpctGeom;

typedef struct {
	double begEtm;			     /* Beginning epochal time */
	double begLvl;			     /* Beginning level  */
	double endEtm;			     /* End epochal time */
	double endLvl;			     /* End level */
	long fileId;			     /* Database file ID */
	CdOrder storageOrder;		     /* Storage order in external file */
	long varId;			     /* Database variable ID */
} CdSlab;

typedef struct {
	long   			id;
	char			name[41];
	long			count;
	char			lvlType[41];
	char			remark[121];
	char			units[41];
	double			*values;
} CdLevel;

typedef struct {
	char   		alias[CD_MAX_NAME];	     /* name in the file  */
	CdType  	dataType;	     /* data type of var */
	long		dsetId;		     /* Dataset ID */
	CdGridType	gridType;	     /* grid type (registered |	spectral) */
	long		id;		     /* Variable ID */
	CdLevel		level;		     /* Level */
	long	  	lvlId;		     /* level database ID */
	double		maxEtm;		     /* Ending epochal time */
	double		minEtm;		     /* Beginning epochal time */
	double		missing;	     /* missing data value */
	char   		name[CD_MAX_NAME];	     /* parameter (official variable) name */
	long		nx;		     /* Number of longitudes */
	long		ny;		     /* Number of latitudes */
	long		nz;		     /* Number of levels */
	long		nt;		     /* Number of times */
	long		parmId;		     /* parameter database ID */
	char		parmUnits[CD_MAX_NAME];	     /* official units */
	CdRegGeom       regGeom;	     /* registered geometry */
	char		remark[CD_MAX_NAME];	     /* comments */
	double		scaleB;		     /* mapping from actual units to official units */
					     /* is parmUnits = (scaleM * units) + scaleB*/
	double		scaleM;
	CdSpctGeom      spctGeom;	     /* spectral geometry */
	char		statistic[CD_MAX_NAME];	/* Statistic identifier */
	CdOrder         storageOrder;	     /* storage order, in external file or internally */
	CdTimeType	timeType;	     /* time type */
	char		title[CD_MAX_NAME];	     /* parameter title */
	char		units[CD_MAX_NAME];	     /* actual units (cf. parmUnits) */
} CdVar;

/*
 * =================================================================
 *			Function Prototypes
 * =================================================================
 */

/* Connect to the database */
/*
extern int
CdConnectDb(void);
*/
/* Disconnect from the database */
/*
extern void
CdDisconnectDb(void);
*/
/* Open a dataset. Returns a handle for the dataset*/

extern CdDset
CdOpenDset(char *path,	     	             /* Dataset path name */
	   CdOpenMode openMode		     /* Open mode */
	   );


/* Close a dataset */

extern void
CdCloseDset(CdDset dset);		     /* Dataset handle */

/* Get a variable handle */

extern int
CdLookupVar(CdDset dset,		     /* Dataset handle */
	    char *varName,		     /* Variable (parameter) name*/
	    char *statistic,	             /* Statistic (if NULL, return the first variable) */
	    CdVar *var,			     /* Variable descriptor (returned) */
	    CdRegion *region);		     /* Region (returned) */

/* Read a region of data. A region is a description of a four-dimensional */
/* lon-lat-lev-time portion of data. */

extern int
CdReadRegion(CdVar *var,		     /* variable */
	     CdRegion *region,		     /* Region to read */
	     CdOrder *order,		     /* Ordering of data dimensions on return */
	     CdType datatype,   	     /* Datatype of data to be returned (input) */
	     void *data);		     /* Data returned (output) */


/* Set a region */

extern int
CdSetLon(CdVar *var,		     	     /* variable */
	 double firstLon,		     /* first longitude in range */
	 double lastLon,		     /* last longitude in range */
	 CdRegion *region);		     /* region to set */

extern int
CdSetLat(CdVar *var,		     	     /* variable */
	 double firstLat,
	 double lastLat,
	 CdRegion *region);

extern int
CdSetLevel(CdVar *var,		     	     /* variable */
	   double firstLevel,
	   double lastLevel,
	   CdRegion *region);

extern int
CdSetTime(CdVar *var,		             /* variable */
	  CdTime *start,
	  CdTime *end,
	  CdRegion *region);

/* Get dimension ranges from a region */

extern int
CdGetLon(CdVar *var,		     	     /* variable */
	 CdRegion *region,
	 double *firstLon,
	 double *lastLon);

extern int
CdGetLat(CdVar *var,		             /* variable */
	 CdRegion *region,
	 double *firstLat,
	 double *lastLat);

extern int
CdGetLevel(CdVar *var,		             /* variable */
	   CdRegion *region,
	   double *firstLevel,
	   double *lastLevel);

extern int
CdGetTime(CdVar *var,		             /* variable */
	  CdRegion *region,
	  CdTime *start,
	  CdTime *end);

/* Get region size, returns number of elements */

extern long
CdRegionSize(CdRegion *region, CdType dataType);

/* Set the order structure from human-readable form */
extern int
CdSetOrder(CdVar *var,			     /* Variable handle */
	   CdOrder *order,		     /* Returned order structure */
	   char orderSpec[11]);		     /* Order specification, any permutation such as */
                                             /* "SpSxSySzSt", "SxSySpStSz" , "StSpSzSySx" */
					     /* where S is either '+' for increasing dimension values */
					     /* in the resulting array, or '-' for decreasing values. */

/* Get the registered geometry for a variable */
extern int
CdGetRegGeom(CdVar *var,	     	     /* Variableiable */
	     CdRegGeom *varRegGeom);	     /* registered geometry (output) */

/* Get the spectral geometry for a variable */
extern int
CdGetSpctGeom(CdVar *var,		     /* Variable */
	      CdSpctGeom *varSpctGeom);	     /* spectral geometry */
	      

extern CdVar
CdSetVarCursor(CdDset dset,			     /* Dataset handle */
		char *varName,		     /* Variable name */
		char *statistic);		     /* Variable statistic, or NULL for first */

					     /* Fetch a variable */
extern CdVar
CdFetchVar(CdCursorOp op);		     /* Cursor operation */

					     /* Error routines */

#endif
