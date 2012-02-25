/*
 * This file contains `cfortran.h' definitions that are specific to the
 * netCDF-3 package.
 */


#ifndef UD_NCFORTRAN_H
#define UD_NCFORTRAN_H

#include "netcdf_f.h"
#include "cfortran.h"
#include "nfconfig.inc"

/*
 * The type of a Fortran INTEGER:
 */
#if NF_INT_IS_C_INT
#   define	NF_INTEGER	int
#   define	NF_INT		INT
#elif NF_INT_IS_C_LONG
#   define	NF_INTEGER	long
#   define	NF_INT		LONG
#else
#   include "NF_INT_IS_C_... not defined"
#endif

/*
 * Input, Fortran INTEGER converted to C "int" (helper macro):
 */
#define FINT2CINT_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define FINT2CINT_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define FINT2CINT_cfN(  T,A)		const NF_INTEGER *A
#define FINT2CINT_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define FINT2CINT_cfT(M,I,A,B,D)	(int)*A


/*
 * Input, Fortran INTEGER converted to C "size_t":
 */
#define FINT2CSIZET_cfINT(N,A,B,X,Y,Z)		SIMPLE_cfINT(N,A,B,X,Y,Z)
#define FINT2CSIZET_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define FINT2CSIZET_cfN(  T,A)			const NF_INTEGER *A
#define FINT2CSIZET_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define FINT2CSIZET_cfT(M,I,A,B,D)		(size_t)*A


/*
 * Input/Output, I/O size hint:
 */
#define PCHUNKSIZEHINT_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PCHUNKSIZEHINT_cfTYPE		NF_INTEGER
#define PCHUNKSIZEHINT_cfSTR(N,T,A,B,C,D,E)	\
					_(CFARGS,N)(T,PCHUNKSIZEHINT,A,B,C,D,E)
#define PCHUNKSIZEHINT_cfH(S,U,B)
#define PCHUNKSIZEHINT_cfQ(B)		size_t B;
#define PCHUNKSIZEHINT_cfT(M,I,A,B,D)	((void*)(B=*A), &B)
#define PCHUNKSIZEHINT_cfR(A,B,D)	*A=B;


/*
 * Output, C "int" converted to Fortran INTEGER (helper macro):
 */
#define PCINT2FINT_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PCINT2FINT_cfTYPE		NF_INTEGER
#define PCINT2FINT_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PCINT2FINT,A,B,C,D,E)
#define PCINT2FINT_cfH(S,U,B)
#define PCINT2FINT_cfQ(B)		int B;
#define PCINT2FINT_cfT(M,I,A,B,D)	&B
#define PCINT2FINT_cfR(A,B,D)		*A=B;


/*
 * Input, Fortran index converted to C index:
 */
#define FNDX2CNDX_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define FNDX2CNDX_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define FNDX2CNDX_cfN(  T,A)		const NF_INTEGER *A
#define FNDX2CNDX_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define FNDX2CNDX_cfT(M,I,A,B,D)	(*A-1)


/*
 * Output, C index converted to Fortran index:
 */
#define PCNDX2FNDX_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PCNDX2FNDX_cfTYPE		NF_INTEGER
#define PCNDX2FNDX_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PCNDX2FNDX,A,B,C,D,E)
#define PCNDX2FNDX_cfH(S,U,B)
#define PCNDX2FNDX_cfQ(B)		int B;
#define PCNDX2FNDX_cfT(M,I,A,B,D)	&B
#define PCNDX2FNDX_cfR(A,B,D)		*A=B+1;


/*******************************************************************************
 * Character buffer:
 */

#define CBUF_cfINT(N,A,B,X,Y,Z)		STRING_cfINT(N,A,B,X,Y,Z)
#define CBUF_cfSEP(T,  B)		STRING_cfSEP(T,B)
#define CBUF_cfN(  T,A)			STRING_cfN(T,A)
#define CBUF_cfSTR(N,T,A,B,C,D,E)	STRING_cfSTR(N,T,A,B,C,D,E)
#if defined(vmsFortran)
#   define CBUF_cfT(M,I,A,B,D)		A->dsc$a_pointer
#elif defined(CRAYFortran)
#   define CBUF_cfT(M,I,A,B,D)		_fcdtocp(A)
#else
#   define CBUF_cfT(M,I,A,B,D)		A
#endif


/*******************************************************************************
 * netCDf attributes:
 */

#define TEXTATT			STRING
#define PTEXTATT		PSTRING

#define INT1ATT			INT1VARV
#define PINT1ATT		PINT1VARV

#define INT2ATT			INT2VARV
#define PINT2ATT		PINT2VARV

#define INTATT			INTVARV
#define PINTATT			PINTVARV

#define REALATT			REALVARV
#define PREALATT		PREALVARV

#define DOUBLEATT		DOUBLEVARV
#define PDOUBLEATT		PDOUBLEVARV


/*
 * Input, attribute ID:
 */
#define ATTID	FNDX2CNDX


/*
 * Output, attribute ID:
 */
#define PATTID	PCNDX2FNDX


/*******************************************************************************
 * netCDf type:
 */


/*
 * Input, netCDF type:
 */
#define TYPE_cfINT(N,A,B,X,Y,Z)		SIMPLE_cfINT(N,A,B,X,Y,Z)
#define TYPE_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define TYPE_cfN(  T,A)			NF_INTEGER *A
#define TYPE_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define TYPE_cfT(M,I,A,B,D)		((nc_type)*A)


/*
 * Output, netCDF type:
 */
#define PTYPE_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PTYPE_cfTYPE			NF_INTEGER
#define PTYPE_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PTYPE,A,B,C,D,E)
#define PTYPE_cfH(S,U,B)
#define PTYPE_cfQ(B)			nc_type B;
#define PTYPE_cfT(M,I,A,B,D)		&B
#define PTYPE_cfR(A,B,D)		*A=B;


/*******************************************************************************
 * netCDf number-of-<whatever>:
 */


/*
 * Input, number-of-dimensions:
 */
#define NDIMS_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define NDIMS_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define NDIMS_cfN(  T,A)		NF_INTEGER *fndims
#define NDIMS_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define NDIMS_cfT(M,I,A,B,D)		((int)*fndims)


/*
 * Output number-of-dimensions:
 */
#define	PNDIMS	PCINT2FINT


/*
 * Input number-of-variables:
 */
#define NVARS	FINT2CINT


/*
 * Output number-of-variables:
 */
#define	PNVARS	PNDIMS


/*
 * Input number-of-attributes:
 */
#define NATTS	FINT2CINT


/*
 * Output number-of-attributes:
 */
#define	PNATTS	PNDIMS

/*
 * Output format version number
 */
#define PFORMAT  PNDIMS


/*******************************************************************************
 * netCDf variables:
 */


/*
 * Input, CHARACTER*(*) variable:
 */
#define TEXTVAR			STRING


/*
 * Output, CHARACTER*(*) variable:
 */
#define PTEXTVAR		PSTRING


/*
 * Input, CHARACTER*(*) variable array:
 */
#define TEXTVARV		STRING


/*
 * Output, CHARACTER*(*) variable array:
 */
#define PTEXTVARV		PSTRING


/*
 * Input, INTEGER*1 variable:
 */
#define INT1VAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define INT1VAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#if NF_INT1_IS_C_SIGNED_CHAR
#   define INT1VAR_cfN(  T,A)		const signed char *A
#elif NF_INT1_IS_C_SHORT
#   define INT1VAR_cfN(  T,A)		const short *A
#elif NF_INT1_IS_C_INT
#   define INT1VAR_cfN(  T,A)		const int *A
#elif NF_INT1_IS_C_LONG
#   define INT1VAR_cfN(  T,A)		const long *A
#endif
#define INT1VAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define INT1VAR_cfT(M,I,A,B,D)		A


/*
 * Output, INTEGER*1 variable:
 */
#define PINT1VAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define PINT1VAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#if NF_INT1_IS_C_SIGNED_CHAR
#   define PINT1VAR_cfN(  T,A)		signed char *A
#elif NF_INT1_IS_C_SHORT
#   define PINT1VAR_cfN(  T,A)		short *A
#elif NF_INT1_IS_C_INT
#   define PINT1VAR_cfN(  T,A)		int *A
#elif NF_INT1_IS_C_LONG
#   define PINT1VAR_cfN(  T,A)		long *A
#endif
#define PINT1VAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define PINT1VAR_cfT(M,I,A,B,D)		A


/*
 * Input, INTEGER*1 variable array:
 */
#define INT1VARV	INT1VAR


/*
 * Output, INTEGER*1 variable array:
 */
#define PINT1VARV	PINT1VAR


/*
 * Input, INTEGER*2 variable:
 */
#define INT2VAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define INT2VAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#if NF_INT2_IS_C_SHORT
#   define INT2VAR_cfN(  T,A)		const short *A
#elif NF_INT2_IS_C_INT
#   define INT2VAR_cfN(  T,A)		const int *A
#elif NF_INT2_IS_C_LONG
#   define INT2VAR_cfN(  T,A)		const long *A
#endif
#define INT2VAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define INT2VAR_cfT(M,I,A,B,D)		A


/*
 * Output, INTEGER*2 variable:
 */
#define PINT2VAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define PINT2VAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#if NF_INT2_IS_C_SHORT
#   define PINT2VAR_cfN(  T,A)		short *A
#elif NF_INT2_IS_C_INT
#   define PINT2VAR_cfN(  T,A)		int *A
#elif NF_INT2_IS_C_LONG
#   define PINT2VAR_cfN(  T,A)		long *A
#endif
#define PINT2VAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define PINT2VAR_cfT(M,I,A,B,D)		A


/*
 * Input, INTEGER*2 variable array:
 */
#define INT2VARV		INT2VAR


/*
 * Output, INTEGER*2 variable array:
 */
#define PINT2VARV		PINT2VAR


/*
 * Input, INTEGER variable:
 */
#define INTVAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define INTVAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define INTVAR_cfN(  T,A)		const NF_INTEGER *A
#define INTVAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define INTVAR_cfT(M,I,A,B,D)		A


/*
 * Output, INTEGER variable:
 */
#define PINTVAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define PINTVAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define PINTVAR_cfN(  T,A)		NF_INTEGER *A
#define PINTVAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define PINTVAR_cfT(M,I,A,B,D)		A


/*
 * Input, INTEGER variable array:
 */
#define INTVARV			INTVAR


/*
 * Output, INTEGER variable array:
 */
#define PINTVARV		PINTVAR


/*
 * Input, REAL variable:
 */
#define REALVAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define REALVAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#if NF_REAL_IS_C_DOUBLE
#   define REALVAR_cfN(  T,A)		const double *A
#else
#   define REALVAR_cfN(  T,A)		const float *A
#endif
#define REALVAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define REALVAR_cfT(M,I,A,B,D)		A


/*
 * Output, REAL variable:
 */
#define PREALVAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define PREALVAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#if NF_REAL_IS_C_DOUBLE
#   define PREALVAR_cfN(  T,A)		double *A
#else
#   define PREALVAR_cfN(  T,A)		float *A
#endif
#define PREALVAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define PREALVAR_cfT(M,I,A,B,D)		A


/*
 * Input, REAL variable array:
 */
#define REALVARV		REALVAR


/*
 * Output, REAL variable array:
 */
#define PREALVARV		PREALVAR


/*
 * Input, DOUBLEPRECISION variable:
 */
#define DOUBLEVAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define DOUBLEVAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define DOUBLEVAR_cfN(  T,A)		const double *A
#define DOUBLEVAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define DOUBLEVAR_cfT(M,I,A,B,D)	A


/*
 * Output, DOUBLEPRECISION variable:
 */
#define PDOUBLEVAR_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define PDOUBLEVAR_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define PDOUBLEVAR_cfN(  T,A)		double *A
#define PDOUBLEVAR_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define PDOUBLEVAR_cfT(M,I,A,B,D)	A


/*
 * Input, DOUBLEPRECISION variable array:
 */
#define DOUBLEVARV		DOUBLEVAR


/*
 * Output, DOUBLEPRECISION variable array:
 */
#define PDOUBLEVARV		PDOUBLEVAR


/*******************************************************************************
 * Miscellaneious netCDF stuff:
 */


/*
 * Output, `size_t' variable:
 */
#define PSIZET_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PSIZET_cfTYPE			NF_INTEGER
#define PSIZET_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PSIZET,A,B,C,D,E)
#define PSIZET_cfH(S,U,B)
#define PSIZET_cfQ(B)			size_t B;
#define PSIZET_cfT(M,I,A,B,D)		&B
#define PSIZET_cfR(A,B,D)		*A=B;


/*
 * Input dimension-ID:
 */
#define DIMID	FNDX2CNDX


/*
 * Output, dimension-ID:
 */
#define PDIMID_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PDIMID_cfTYPE			NF_INTEGER
#define PDIMID_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PDIMID,A,B,C,D,E)
#define PDIMID_cfH(S,U,B)
#define PDIMID_cfQ(B)			int B = -1;
#define PDIMID_cfT(M,I,A,B,D)		&B
#define PDIMID_cfR(A,B,D)		*A=(B == -1 ? -1 : B+1);


/*
 * Input, dimension-ID vector:
 */
#define DIMIDS_cfINT(N,A,B,X,Y,Z)	DOUBLE_cfINT(N,A,B,X,Y,Z)
#define DIMIDSVVVVVVV_cfTYPE		NF_INTEGER
#define DIMIDS_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,DIMIDS,A,B,C,D,E)
#define DIMIDS_cfH(S,U,B)
#define DIMIDS_cfQ(B)			int B[NC_MAX_DIMS];
#define DIMIDS_cfT(M,I,A,B,D)		f2c_dimids(*fndims, A, B)
#define DIMIDS_cfR(A,B,D)


/*
 * Output, dimension-ID vector:
 */
#define PDIMIDS_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PDIMIDS_cfTYPE			NF_INTEGER
#define PDIMIDS_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PDIMIDS,A,B,C,D,E)
#define PDIMIDS_cfH(S,U,B)
#define PDIMIDS_cfQ(B)			int B[NC_MAX_DIMS];
#define PDIMIDS_cfT(M,I,A,B,D)		B
#define PDIMIDS_cfR(A,B,D)		c2f_dimids(*fncid, *fvarid-1, B, A);

/*
 * Input, chunksizes vector:
 */
#define CHUNKSIZES_cfINT(N,A,B,X,Y,Z)	        DOUBLE_cfINT(N,A,B,X,Y,Z)
#define CHUNKSIZESVVVVVVV_cfTYPE		NF_INTEGER
#define CHUNKSIZES_cfSTR(N,T,A,B,C,D,E)	        _(CFARGS,N)(T,CHUNKSIZES,A,B,C,D,E)
#define CHUNKSIZES_cfH(S,U,B)
#define CHUNKSIZES_cfQ(B)			int B[NC_MAX_DIMS];
#define CHUNKSIZES_cfT(M,I,A,B,D)		f2c_chunksizes(*fncid, *fvarid-1, A, B)
#define CHUNKSIZES_cfR(A,B,D)


/*
 * Output, chunksizes vector:
 */
#define PCHUNKSIZES_cfINT(N,A,B,X,Y,Z)	PDOUBLE_cfINT(N,A,B,X,Y,Z)
#define PCHUNKSIZES_cfTYPE			NF_INTEGER
#define PCHUNKSIZES_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,PCHUNKSIZES,A,B,C,D,E)
#define PCHUNKSIZES_cfH(S,U,B)
#define PCHUNKSIZES_cfQ(B)			int B[NC_MAX_DIMS];
#define PCHUNKSIZES_cfT(M,I,A,B,D)		B
#define PCHUNKSIZES_cfR(A,B,D)		c2f_chunksizes(*fncid, *fvarid-1, B, A);


/*
 * Input, netCDF dataset ID:
 */
#define NCID_cfINT(N,A,B,X,Y,Z)		SIMPLE_cfINT(N,A,B,X,Y,Z)
#define NCID_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define NCID_cfN(  T,A)			NF_INTEGER *fncid
#define NCID_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define NCID_cfT(M,I,A,B,D)		*fncid


/*
 * Two, input, netCDF dataset IDs:
 */
#define NCID1	FINT2CINT
#define NCID2	FINT2CINT


/*
 * Output, netCDF dataset ID:
 */
#define PNCID	PCINT2FINT


/*
 * Input, netCDF variable ID:
 */
#define VARID_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define VARID_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define VARID_cfN(  T,A)		NF_INTEGER *fvarid
#define VARID_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define VARID_cfT(M,I,A,B,D)		(*fvarid-1)


/*
 * Two, input, netCDF variable IDs:
 */
#define VARID1_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define VARID1_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define VARID1_cfN(  T,A)		NF_INTEGER *fvarid1
#define VARID1_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define VARID1_cfT(M,I,A,B,D)		(*fvarid1-1)

#define VARID2_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define VARID2_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define VARID2_cfN(  T,A)		NF_INTEGER *fvarid2
#define VARID2_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define VARID2_cfT(M,I,A,B,D)		(*fvarid2-1)


/*
 * Output, netCDF variable ID:
 */
#define PVARID	PCNDX2FNDX

/*
 * Input, netCDF field index:
 */
#define FIELDIDX_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define FIELDIDX_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define FIELDIDX_cfN(  T,A)		NF_INTEGER *ffieldidx
#define FIELDIDX_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define FIELDIDX_cfT(M,I,A,B,D)		(*ffieldidx-1)

/*
 * Input, co-ordinate vector:
 */
#define COORDS_cfINT(N,A,B,X,Y,Z)	DOUBLE_cfINT(N,A,B,X,Y,Z)
#define COORDSVVVVVVV_cfTYPE		NF_INTEGER
#define COORDS_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,COORDS,A,B,C,D,E)
#define COORDS_cfH(S,U,B)
#define COORDS_cfQ(B)			size_t B[NC_MAX_DIMS];
#define COORDS_cfT(M,I,A,B,D)		f2c_coords(*fncid, *fvarid-1, A, B)
#define COORDS_cfR(A,B,D)


/*
 * Input count:
 */
#define COUNT_cfINT(N,A,B,X,Y,Z)	SIMPLE_cfINT(N,A,B,X,Y,Z)
#define COUNT_cfSEP(T,  B)		SIMPLE_cfSEP(T,B)
#define COUNT_cfN(  T,A)		const NF_INTEGER *A
#define COUNT_cfSTR(N,T,A,B,C,D,E)	SIMPLE_cfSTR(N,T,A,B,C,D,E)
#define COUNT_cfT(M,I,A,B,D)		(size_t)*A


/*
 * Output count:
 */
#define PCOUNT	PSIZET


/*
 * Input, count vector:
 */
#define COUNTS_cfINT(N,A,B,X,Y,Z)	DOUBLE_cfINT(N,A,B,X,Y,Z)
#define COUNTSVVVVVVV_cfTYPE		NF_INTEGER
#define COUNTS_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,COUNTS,A,B,C,D,E)
#define COUNTS_cfH(S,U,B)
#define COUNTS_cfQ(B)			size_t B[NC_MAX_DIMS];
#define COUNTS_cfT(M,I,A,B,D)		f2c_counts(*fncid, *fvarid-1, A, B)
#define COUNTS_cfR(A,B,D)


/*
 * Input, stride vector:
 */
#define STRIDES_cfINT(N,A,B,X,Y,Z)	DOUBLE_cfINT(N,A,B,X,Y,Z)
#define STRIDESVVVVVVV_cfTYPE		NF_INTEGER
#define STRIDES_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,STRIDES,A,B,C,D,E)
#define STRIDES_cfH(S,U,B)
#define STRIDES_cfQ(B)			ptrdiff_t B[NC_MAX_DIMS];
#define STRIDES_cfT(M,I,A,B,D)		f2c_strides(*fncid, *fvarid-1, A, B)
#define STRIDES_cfR(A,B,D)


/*
 * Input, mapping vector:
 */
#define MAPS_cfINT(N,A,B,X,Y,Z)		DOUBLE_cfINT(N,A,B,X,Y,Z)
#define MAPSVVVVVVV_cfTYPE		NF_INTEGER
#define MAPS_cfSTR(N,T,A,B,C,D,E)	_(CFARGS,N)(T,MAPS,A,B,C,D,E)
#define MAPS_cfH(S,U,B)
#define MAPS_cfQ(B)			ptrdiff_t B[NC_MAX_DIMS];
#define MAPS_cfT(M,I,A,B,D)		f2c_maps(*fncid, *fvarid-1, A, B)
#define MAPS_cfR(A,B,D)


/*******************************************************************************
 * The following is for f2c-support only.
 */

#if defined(f2cFortran) && !defined(pgiFortran) && !defined(gFortran)

/*
 * The f2c(1) utility on BSD/OS and Linux systems adds an additional
 * underscore suffix (besides the usual one) to global names that have
 * an embedded underscore.  For example, `nfclose' becomes `nfclose_',
 * but `nf_close' becomes `nf_close__.  Consequently, we have to modify
 * some names.
 */
#ifdef LOGGING
#define nf_set_log_level        nf_set_log_level_
#endif /* LOGGING */
#define nf_inq_libvers		nf_inq_libvers_
#define nf_strerror		nf_strerror_
#define nf_issyserr		nf_issyserr_
#define nf_create		nf_create_
#define nf_open			nf_open_
#define nf_set_fill		nf_set_fill_
#define nf_set_default_format	nf_set_default_format_
#define nf_redef		nf_redef_
#define nf_enddef		nf_enddef_
#define nf_sync			nf_sync_
#define nf_abort		nf_abort_
#define nf_close		nf_close_
#define nf_delete		nf_delete_
#define nf_inq			nf_inq_
#define nf_inq_ndims		nf_inq_ndims_
#define nf_inq_nvars		nf_inq_nvars_
#define nf_inq_natts		nf_inq_natts_
#define nf_inq_unlimdim		nf_inq_unlimdim_
#define nf_inq_format		nf_inq_format_
#define nf_def_dim		nf_def_dim_
#define nf_inq_dimid		nf_inq_dimid_
#define nf_inq_dim		nf_inq_dim_
#define nf_inq_dimname		nf_inq_dimname_
#define nf_inq_dimlen		nf_inq_dimlen_
#define nf_rename_dim		nf_rename_dim_
#define nf_inq_att		nf_inq_att_
#define nf_inq_attid		nf_inq_attid_
#define nf_inq_atttype		nf_inq_atttype_
#define nf_inq_attlen		nf_inq_attlen_
#define nf_inq_attname		nf_inq_attname_
#define nf_copy_att		nf_copy_att_
#define nf_rename_att		nf_rename_att_
#define nf_del_att		nf_del_att_
#define nf_put_att_text		nf_put_att_text_
#define nf_get_att_text		nf_get_att_text_
#define nf_put_att_int1		nf_put_att_int1_
#define nf_get_att_int1		nf_get_att_int1_
#define nf_put_att_int2		nf_put_att_int2_
#define nf_get_att_int2		nf_get_att_int2_
#define nf_put_att_int		nf_put_att_int_
#define nf_get_att_int		nf_get_att_int_
#define nf_put_att_real		nf_put_att_real_
#define nf_get_att_real		nf_get_att_real_
#define nf_put_att_double	nf_put_att_double_
#define nf_get_att_double	nf_get_att_double_
#define nf_def_var		nf_def_var_
#define nf_inq_var		nf_inq_var_
#define nf_inq_varid		nf_inq_varid_
#define nf_inq_varname		nf_inq_varname_
#define nf_inq_vartype		nf_inq_vartype_
#define nf_inq_varndims		nf_inq_varndims_
#define nf_inq_vardimid		nf_inq_vardimid_
#define nf_inq_varnatts		nf_inq_varnatts_
#define nf_rename_var		nf_rename_var_
#define nf_copy_var		nf_copy_var_
#define nf_put_var_text		nf_put_var_text_
#define nf_get_var_text		nf_get_var_text_
#define nf_put_var_int1		nf_put_var_int1_
#define nf_get_var_int1		nf_get_var_int1_
#define nf_put_var_int2		nf_put_var_int2_
#define nf_get_var_int2		nf_get_var_int2_
#define nf_put_var_int		nf_put_var_int_
#define nf_get_var_int		nf_get_var_int_
#define nf_put_var_real		nf_put_var_real_
#define nf_get_var_real		nf_get_var_real_
#define nf_put_var_double	nf_put_var_double_
#define nf_get_var_double	nf_get_var_double_
#define nf_put_var1_text	nf_put_var1_text_
#define nf_get_var1_text	nf_get_var1_text_
#define nf_put_var1_int1	nf_put_var1_int1_
#define nf_get_var1_int1	nf_get_var1_int1_
#define nf_put_var1_int2	nf_put_var1_int2_
#define nf_get_var1_int2	nf_get_var1_int2_
#define nf_put_var1_int		nf_put_var1_int_
#define nf_get_var1_int		nf_get_var1_int_
#define nf_put_var1_real	nf_put_var1_real_
#define nf_get_var1_real	nf_get_var1_real_
#define nf_put_var1_double	nf_put_var1_double_
#define nf_get_var1_double	nf_get_var1_double_
#define nf_put_vara_text	nf_put_vara_text_
#define nf_get_vara_text	nf_get_vara_text_
#define nf_put_vara_int1	nf_put_vara_int1_
#define nf_get_vara_int1	nf_get_vara_int1_
#define nf_put_vara_int2	nf_put_vara_int2_
#define nf_get_vara_int2	nf_get_vara_int2_
#define nf_put_vara_int		nf_put_vara_int_
#define nf_get_vara_int		nf_get_vara_int_
#define nf_put_vara_real	nf_put_vara_real_
#define nf_get_vara_real	nf_get_vara_real_
#define nf_put_vara_double	nf_put_vara_double_
#define nf_get_vara_double	nf_get_vara_double_
#define nf_put_vars_text	nf_put_vars_text_
#define nf_get_vars_text	nf_get_vars_text_
#define nf_put_vars_int1	nf_put_vars_int1_
#define nf_get_vars_int1	nf_get_vars_int1_
#define nf_put_vars_int2	nf_put_vars_int2_
#define nf_get_vars_int2	nf_get_vars_int2_
#define nf_put_vars_int		nf_put_vars_int_
#define nf_get_vars_int		nf_get_vars_int_
#define nf_put_vars_real	nf_put_vars_real_
#define nf_get_vars_real	nf_get_vars_real_
#define nf_put_vars_double	nf_put_vars_double_
#define nf_get_vars_double	nf_get_vars_double_
#define nf_put_varm_text	nf_put_varm_text_
#define nf_get_varm_text	nf_get_varm_text_
#define nf_put_varm_int1	nf_put_varm_int1_
#define nf_get_varm_int1	nf_get_varm_int1_
#define nf_put_varm_int2	nf_put_varm_int2_
#define nf_get_varm_int2	nf_get_varm_int2_
#define nf_put_varm_int		nf_put_varm_int_
#define nf_get_varm_int		nf_get_varm_int_
#define nf_put_varm_real	nf_put_varm_real_
#define nf_get_varm_real	nf_get_varm_real_
#define nf_put_varm_double	nf_put_varm_double_
#define nf_get_varm_double	nf_get_varm_double_
#define nf__create              nf__create_
#define nf__create_mp           nf__create_mp_
#define nf__enddef              nf__enddef_
#define nf__open                nf__open_
#define nf__open_mp             nf__open_mp_
#define nf_delete_mp            nf_delete_mp_
#define nf_inq_base_pe          nf_inq_base_pe_
#define nf_set_base_pe          nf_set_base_pe_

#ifdef USE_NETCDF4
#define nf_create_par nf_create_par_
#define nf_open_par nf_open_par_
#define nf_var_par_access nf_var_par_access_
#define nf_inq_ncid nf_inq_ncid_
#define nf_inq_grps nf_inq_grps_
#define nf_inq_grpname nf_inq_grpname_
#define nf_inq_grp_parent nf_inq_grp_parent_
#define nf_inq_grp_ncid nf_inq_grp_ncid_
#define nf_inq_varids nf_inq_varids_
#define nf_inq_dimids nf_inq_dimids_
#define nf_inq_typeids nf_inq_typeids_
#define nf_inq_typeid nf_inq_typeid_
#define nf_def_grp nf_def_grp_
#define nf_def_compound nf_def_compound_
#define nf_insert_compound nf_insert_compound_
#define nf_insert_array_compound nf_insert_array_compound_
#define nf_inq_type nf_inq_type_
#define nf_inq_compound nf_inq_compound_
#define nf_inq_compound_name nf_inq_compound_name_
#define nf_inq_compound_size nf_inq_compound_size_
#define nf_inq_compound_nfields nf_inq_compound_nfields_
#define nf_inq_compound_field nf_inq_compound_field_
#define nf_inq_compound_fieldname nf_inq_compound_fieldname_
#define nf_inq_compound_fieldindex nf_inq_compound_fieldindex_
#define nf_inq_compound_fieldtype nf_inq_compound_fieldtype_
#define nf_inq_compound_fieldndims nf_inq_compound_fieldndims_
#define nf_inq_compound_fielddim_sizes nf_inq_compound_fielddim_sizes_
#define nf_def_vlen nf_def_vlen_
#define nf_inq_vlen nf_inq_vlen_
#define nf_free_vlen nf_free_vlen_
#define nf_inq_user_type nf_inq_user_type_
#define nf_put_att nf_put_att_
#define nf_get_att nf_get_att_
#define nf_def_enum nf_def_enum_
#define nf_insert_enum nf_insert_enum_
#define nf_inq_enum nf_inq_enum_
#define nf_inq_enum_member nf_inq_enum_member_
#define nf_inq_enum_ident nf_inq_enum_ident_
#define nf_def_opaque nf_def_opaque_
#define nf_inq_opaque nf_inq_opaque_
#define nf_put_var nf_put_var_
#define nf_put_var1 nf_put_var1_
#define nf_put_vara nf_put_vara_
#define nf_put_vars nf_put_vars_
#define nf_put_varm nf_put_varm_
#define nf_get_var nf_get_var_
#define nf_get_var1 nf_get_var1_
#define nf_get_vara nf_get_vara_
#define nf_get_vars nf_get_vars_
#define nf_get_varm nf_get_varm_
#define nf_put_var_int64 nf_put_var_int64_
#define nf_put_var1_int64 nf_put_var1_int64_
#define nf_put_vara_int64 nf_put_vara_int64_
#define nf_put_vars_int64 nf_put_vars_int64_
#define nf_put_varm_int64 nf_put_varm_int64_
#define nf_get_var_int64 nf_get_var_int64_
#define nf_get_var1_int64 nf_get_var1_int64_
#define nf_get_vara_int64 nf_get_vara_int64_
#define nf_get_vars_int64 nf_get_vars_int64_
#define nf_get_varm_int64 nf_get_varm_int64_
#define nf_def_var_chunking nf_def_var_chunking_
#define nf_def_var_deflate nf_def_var_deflate_
#define nf_def_var_szip nf_def_var_szip_
#define nf_def_var_fletcher32 nf_def_var_fletcher32_
#define nf_inq_var_chunking nf_inq_var_chunking_
#define nf_inq_var_deflate nf_inq_var_deflate_
#define nf_inq_var_szip nf_inq_var_szip_
#define nf_inq_var_fletcher32 nf_inq_var_fletcher32_
#define nf_def_var_endian nf_def_var_endian_
#define nf_inq_var_endian nf_inq_var_endian_
#define nf_def_var_fill nf_def_var_fill_
#define nf_inq_var_fill nf_inq_var_fill_
#define nf_get_vlen_element nf_get_vlen_element_
#define nf_inq_compound_fieldoffset nf_inq_compound_fieldoffset_
#define nf_inq_grp_full nf_inq_grp_full_
#define nf_inq_grpname_full nf_inq_grpname_full_
#define nf_inq_grpname_len nf_inq_grpname_len_
#define nf_put_vlen_element nf_put_vlen_element_
#define nf_inq_grp_full_ncid nf_inq_grp_full_ncid_
#define nf_set_chunk_cache nf_set_chunk_cache_
#define nf_get_chunk_cache nf_get_chunk_cache_
#define nf_set_var_chunk_cache nf_set_var_chunk_cache_
#define nf_get_var_chunk_cache nf_get_var_chunk_cache_
#define nf_free_string nf_free_string_
#define nf_free_vlens nf_free_vlens_

#endif /* USE_NETCDF4 */

#endif	/* f2cFortran */

#if defined(DLL_NETCDF) /* define when library is a DLL */
#  if defined(NC_DLL_EXPORT) /* define when building the library */
#   define extern extern __declspec(dllexport)
#  else
#   define extern extern __declspec(dllimport)
#  endif
#endif	/* defined(DLL_NETCDF) */

#endif	/* header-file lockout */
