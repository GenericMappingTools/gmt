/*--------------------------------------------------------------------
 *	$Id$
 *--------------------------------------------------------------------*/

/* @(#)s_rint.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 * Exception:
 *	Inexact flag raised if x not equal to rint(x).
 */

#ifndef __MATH_DOFLOAT
#define __MATH_TYPE	double
#define __MATH_FN	rint
#define __MATH_FNIEEE	__ieee754_rint
#else /* __MATH_DOFLOAT */
#define __MATH_TYPE	float
#define __MATH_FN	rintf
#define __MATH_FNIEEE	__ieee754_rintf
#endif /* __MATH_DOFLOAT */

typedef unsigned __int32 u_int32_t;
typedef __int32 int32_t;

typedef union {
  double value;
  struct {
    u_int32_t lsw;
    u_int32_t msw;
  } parts;
} ieee_double_shape_type;

typedef union {
  long double value;
  struct {
    u_int32_t lswlo;
    u_int32_t lswhi;
    u_int32_t mswlo;
    u_int32_t mswhi;
  } parts;
} ieee_quad_shape_type;

typedef union {
  long double value;
  struct {
    u_int32_t lsw;
    u_int32_t nsw;
    u_int32_t msw;
  } parts;
} ieee_extended_shape_type;


/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)					\
do {								\
  ieee_double_shape_type gl_u;					\
  gl_u.value = (d);						\
  (i) = gl_u.parts.lsw;						\
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d,ix0,ix1)					\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d,v)					\
do {								\
  ieee_double_shape_type sl_u;					\
  sl_u.value = (d);						\
  sl_u.parts.lsw = (v);						\
  (d) = sl_u.value;						\
} while (0)

/* A union which permits us to convert between a float and a 32 bit
   int.  */

typedef union
{
  float value;
  u_int32_t word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)


static const double
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

__MATH_TYPE __MATH_FN(__MATH_TYPE fx) {
	int32_t i0,jj0,sx;
	u_int32_t i,i1;
	double x, w,t;
	x = (double)fx;
	EXTRACT_WORDS(i0,i1,x);
	sx = (i0>>31)&1;
	jj0 = ((i0>>20)&0x7ff)-0x3ff;
	if(jj0<20) {
	    if(jj0<0) { 	
		if(((i0&0x7fffffff)|i1)==0) return (__MATH_TYPE)x;
		i1 |= (i0&0x0fffff);
		i0 &= 0xfffe0000;
		i0 |= ((i1|-i1)>>12)&0x80000;
		SET_HIGH_WORD(x,i0);
	        w = TWO52[sx]+x;
	        t =  w-TWO52[sx];
		GET_HIGH_WORD(i0,t);
		SET_HIGH_WORD(t,(i0&0x7fffffff)|(sx<<31));
	        return (__MATH_TYPE)t;
	    } else {
		i = (0x000fffff)>>jj0;
		if(((i0&i)|i1)==0) return (__MATH_TYPE)x; /* x is integral */
		i>>=1;
		if(((i0&i)|i1)!=0) {
		    if(jj0==19) i1 = 0x40000000; else
		    i0 = (i0&(~i))|((0x20000)>>jj0);
		}
	    }
	} else if (jj0>51) {
	    if(jj0==0x400) return (__MATH_TYPE)(x+x);	/* inf or NaN */
	    else return (__MATH_TYPE)x;		/* x is integral */
	} else {
	    i = ((u_int32_t)(0xffffffff))>>(jj0-20);
	    if((i1&i)==0) return (__MATH_TYPE)x;	/* x is integral */
	    i>>=1;
	    if((i1&i)!=0) i1 = (i1&(~i))|((0x40000000)>>(jj0-20));
	}
	INSERT_WORDS(x,i0,i1);
	w = TWO52[sx]+x;
	return (__MATH_TYPE)(w-TWO52[sx]);
}

__MATH_TYPE __MATH_FNIEEE(__MATH_TYPE x) {
	return(__MATH_FN(x));
}
