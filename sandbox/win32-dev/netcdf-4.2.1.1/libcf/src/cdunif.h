/* -*-Mode: C;-*-
 * Module:      cdunif - cdunif uniform I/O  include file
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
 * $Log: cdunif.h,v $
 * Revision 1.1.1.1  2009/07/06 15:06:30  ed
 * added libcf to netcdf dist
 *
 * Revision 1.1  2008/06/30 16:07:44  ed
 * Beginning merge of calandar stuff.
 *
 * Revision 1.1.1.1  1997/12/09 18:57:39  drach
 * Copied from cirrus
 *
 * Revision 1.14  1997/11/24  17:28:04  drach
 * - Added QL package to cdunif
 * - Added NdimIntersect function to CDMS
 *
 * Revision 1.13  1997/11/10  19:22:29  drach
 * - Added cuvargets to cdunif, cdSlabRead to cdms
 *
 * Revision 1.12  1997/01/06  17:47:13  drach
 * - Added HDF to cdunif
 *
 * Revision 1.11  1996/02/23  01:21:24  drach
 * - Moved most of cdms.h to cdmsint.h (original in cdms_v1.h)
 * - Added new time model declarations to cdms.h
 * - Added -DNO_DECLARE flag to fcddrs.h
 *
 * Revision 1.10  1995/10/16  18:56:33  drach
 * - Added CuInt datatype, DEC Alpha version
 *
 * Revision 1.9  1995/03/09  00:35:18  drach
 * Added netCDF, upgraded cureadarray with casting, user-specified indices
 *
 * Revision 1.8  1995/01/18  02:53:38  drach
 * - Explicitly set CuFileType enums
 *
 * Revision 1.7  1994/12/20  01:19:45  drach
 * - Added cdrra and cureadarray functions
 *
 * Revision 1.6  1994/12/17  00:43:02  drach
 * - add cugetlu, cufreelu
 *
 * Revision 1.5  1994/12/16  00:45:42  drach
 * - Added invalid CuType
 *
 * Revision 1.4  1994/12/14  02:33:01  drach
 * - Added comment
 *
 * Revision 1.3  1994/11/23  23:00:06  drach
 * *** empty log message ***
 *
 * Revision 1.2  1994/11/23  22:56:23  drach
 * Changed hyperlong to long double.
 *
 * Revision 1.1  1994/11/18  23:42:27  drach
 * Initial version
 *
 *
 */
#ifndef _CDUNIF_H
#define _CDUNIF_H

/*
 * =================================================================
 *			Macros and Enums
 * =================================================================
 */

#define CU_FATAL 1			     /* Exit immediately on fatal error */
#define CU_VERBOSE 2			     /* Report errors */

#define CU_GLOBAL -1			     /* Global varid */
#define CU_MAX_LU 99			     /* Max logical unit for Fortran I/O */
#define CU_MAX_NAME 128			     /* Max characters in a name */
#define CU_MAX_PATH 256			     /* Max characters in a file pathname */
#define CU_MAX_VAR_DIMS 32		     /* Max dimensions in a variable */
#define CU_SETRRA(a,id,i,value) a->indices[id][i]=value	/* Set a right-ragged array element */

					     /* Note: integer types must be grouped, also floats (cf cuCast) */
					     /* IF MODIFIED, SHOULD ALSO MODIFY CDMS.H !!*/
typedef enum CuType {CuInvalidType = -1, CuByte = 1, CuChar, CuShort, CuInt, CuLong, CuFloat, CuDouble, CuLongDouble} CuType;

					     /* Note: For cuseterropts to work correctly,
					      * valid formats must begin at 0 and increase
					      * sequentially, with CuNumberOfFormats the last entry.
					      */
typedef enum CuFileType {CuUnknown = -1, CuDrs=0, CuGrads=1, CuNetcdf=2, CuHdf=3, CuQL=4, CuPop=5, CuNumberOfFormats} CuFileType;
typedef enum CuDimType {CuGlobalDim = 1, CuLocalDim} CuDimType;

typedef struct {
	long rank;	/* number of dimensions */
	long *arraySize; /* vector of lengths of dimensions of base array */
	long *dimensionSize; /* vector of lengths of index vectors */
	long **indices;		/* index vectors; indices[k] is vector of length dimensionSize[k] */
} CuRRA;

/*
 * =================================================================
 *			Function prototypes
 * =================================================================
 */

extern int cuopenread(const char* controlpath, const char* datapath);
extern int cuclose(int fileid);
extern int cuinquire(int fileid, int* ngdims, int* nvars, int* natts, int* recdim);
extern int cudimid(int fileid, int varid, const char* name);
extern int cudiminq(int fileid, int dimid, char* dimname, char* dimunits, CuType* dataType, CuDimType* dimtype, int* varid, long* length);
extern int cudimget(int fileid, int dimid, void* values);
extern int cuvarid(int fileid, const char* name);
extern int cuvarinq(int fileid, int varid, char* name, CuType* datatype, int* ndims, int dimids[], int* natts);
extern int cuvarget(int fileid, int varid, const long start[], const long count[], void* value);
extern int cuvargets(int fileid, int varid, const long order[], const long start[], const long count[], const long stride[], CuType usertype, void *values);
extern int cuattinq(int fileid, int varid, const char* name, CuType* datatype, int* len);
extern int cuattget(int fileid, int varid, const char* name, void* value);
extern int cuattname(int fileid, int varid, int attnum, char* name);
extern int cutypelen(CuType datatype);
extern int cusetlu(int lu1, int lu2);
extern int cugetlu(int* lu1, int* lu2);
extern int cufreelu(int lu);
extern void cuseterropts(int erropts);
extern int cugeterropts(void);
extern int cugeterr(void);

extern int cureadarray(int fileid, int varid, CuRRA *vIndices, CuRRA *uIndices,
		       const long transpose[], CuType usertype, void *userArray);
extern CuRRA *cucreateRRA(long rank, const long arraySize[], const long dimensionSize[]);
extern void cudestroyRRA(CuRRA *);
extern int cusortRRA(CuRRA *, CuRRA *, const long []);
extern void cuprintRRA(CuRRA *);

/*
 * =================================================================
 *			Globals
 * =================================================================
 */

extern int cuErrOpts;			     /* Error options */
extern int cuErrorOccurred;		     /* True iff cdError was called */

/*
 * =================================================================
 *			Error returns
 * =================================================================
 */

#define CU_SERROR       -1      /* System error */
#define	CU_SUCCESS	0	/* Success */
#define	CU_EBADID	1	/* Bad ID passed to driver layer */
#define	CU_OPENFILES	2	/* Too many files open */
#define	CU_EINVAL	4	/* Invalid Argument */
#define	CU_ENOTINDEFINE	6	/* Operation not allowed in data mode (netCDF only) */
#define	CU_EINDEFINE	7	/* Operation not allowed in define mode (netCDF only) */
#define	CU_EINVALCOORDS	8	/* Coordinates out of Domain */
#define CU_ENOTATT	11	/* Attribute not found */
#define CU_EBADTYPE	13	/* Not a cdunif data type */
#define CU_EBADDIM	14	/* Invalid dimension id */
#define CU_ENOTVAR	17	/* Variable not found */
#define CU_EGLOBAL	18	/* Action prohibited on CU_GLOBAL varid */
#define CU_ENOTCU	19	/* Not a file supported by cdunif*/
#define CU_EMAXNAME     21      /* CU_MAX_NAME exceeded */
#define CU_ENOVARS      51      /* File has no variables */
#define CU_EINTERN      52      /* cdunif internal error */
#define CU_EBADFORM     53      /* Format recognized but not supported */
#define CU_DRIVER       54      /* Driver layer error */
#define CU_EINVLU       55      /* Invalid logical unit (DRS only) */
#define CU_EOPEN        56      /* File open error */
#define CU_ENOCAST      57      /* Cannot cast between user and file datatypes */

#endif
