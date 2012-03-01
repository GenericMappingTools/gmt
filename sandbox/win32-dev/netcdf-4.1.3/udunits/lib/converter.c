/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Value converters for the udunits(3) library.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"		/* this module's API */

typedef struct {
    cv_converter*	(*clone)(cv_converter*);
    double		(*convertDouble)
	(const cv_converter*, double);
    float*		(*convertFloats)
	(const cv_converter*, const float*, size_t, float*);
    double*		(*convertDoubles)
	(const cv_converter*, const double*, size_t, double*);
    int			(*getExpression)
	(const cv_converter*, char* buf, size_t, const char*);
    void		(*free)(cv_converter*);
} ConverterOps;

typedef struct {
    ConverterOps*	ops;
} ReciprocalConverter;

typedef struct {
    ConverterOps*	ops;
    double		value;
} ScaleConverter;

typedef struct {
    ConverterOps*	ops;
    double		value;
} OffsetConverter;

typedef struct {
    ConverterOps*	ops;
    double		slope;
    double		intercept;
} GalileanConverter;

typedef struct {
    ConverterOps*	ops;
    double		logE;
} LogConverter;

typedef struct {
    ConverterOps*	ops;
    double		base;
} ExpConverter;

typedef struct {
    ConverterOps*	ops;
    cv_converter*	first;
    cv_converter*	second;
} CompositeConverter;

union cv_converter {
    ConverterOps*	ops;
    ScaleConverter	scale;
    OffsetConverter	offset;
    GalileanConverter	galilean;
    LogConverter	log;
    ExpConverter	exp;
    CompositeConverter	composite;
};

#define CV_CLONE(conv)		((conv)->ops->clone(conv))

#define IS_TRIVIAL(conv)	((conv)->ops == &trivialOps)
#define IS_RECIPROCAL(conv)	((conv)->ops == &reciprocalOps)
#define IS_SCALE(conv)		((conv)->ops == &scaleOps)
#define IS_OFFSET(conv)		((conv)->ops == &offsetOps)
#define IS_GALILEAN(conv)	((conv)->ops == &galileanOps)
#define IS_LOG(conv)		((conv)->ops == &logOps)


static void
nonFree(
    cv_converter* const conv)
{
}


static void
cvSimpleFree(
    cv_converter* const conv)
{
    free(conv);
}


static int
cvNeedsParentheses(
    const char* const	string)
{
    return strpbrk(string, " \t") != NULL &&
	(string[0] != '(' || string[strlen(string)-1] != ')');
}


/*******************************************************************************
 * Trivial Converter:
 ******************************************************************************/

static cv_converter*
trivialClone(
    cv_converter* const	conv)
{
    return cv_get_trivial();
}


static double
trivialConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return value;
}


static float*
trivialConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	(void)memmove(out, in, count*sizeof(float));
    }

    return out;
}


static double*
trivialConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	(void)memmove(out, in, count*sizeof(double));
    }

    return out;
}


static int
trivialGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    return snprintf(buf, max, "%s", variable);
}


static ConverterOps	trivialOps = {
    trivialClone,
    trivialConvertDouble,
    trivialConvertFloats,
    trivialConvertDoubles,
    trivialGetExpression,
    nonFree};

static cv_converter	trivialConverter = {&trivialOps};


/*******************************************************************************
 * Reciprocal Converter:
 ******************************************************************************/

static cv_converter*
reciprocalClone(
    cv_converter* const	conv)
{
    return cv_get_inverse();
}

static double
reciprocalConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return 1.0 / value;
}


static float*
reciprocalConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = (float)(1.0f / in[i]);
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = (float)(1.0f / in[i]);
	}
    }

    return out;
}


static double*
reciprocalConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = 1.0 / in[i];
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = 1.0 / in[i];
	}
    }

    return out;
}


static int
reciprocalGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    return
	cvNeedsParentheses(variable)
	? snprintf(buf, max, "1/(%s)", variable)
	: snprintf(buf, max, "1/%s", variable);
}


static ConverterOps	reciprocalOps = {
    reciprocalClone,
    reciprocalConvertDouble,
    reciprocalConvertFloats,
    reciprocalConvertDoubles,
    reciprocalGetExpression,
    nonFree};

static cv_converter	reciprocalConverter = {&reciprocalOps};


/*******************************************************************************
 * Scale Converter:
 ******************************************************************************/

static cv_converter*
scaleClone(
    cv_converter* const	conv)
{
    return cv_get_scale(conv->scale.value);
}


static double
scaleConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return conv->scale.value * value;
}


static float*
scaleConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = (float)(conv->scale.value * in[i]);
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = (float)(conv->scale.value * in[i]);
	}
    }

    return out;
}


static double*
scaleConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = conv->scale.value * in[i];
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = conv->scale.value * in[i];
	}
    }

    return out;
}


static int
scaleGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    return
	cvNeedsParentheses(variable)
	? snprintf(buf, max, "%g*(%s)", conv->scale.value, variable)
	: snprintf(buf, max, "%g*%s", conv->scale.value, variable);
}


static ConverterOps	scaleOps = {
    scaleClone,
    scaleConvertDouble,
    scaleConvertFloats,
    scaleConvertDoubles,
    scaleGetExpression,
    cvSimpleFree};


/*******************************************************************************
 * Offset Converter:
 ******************************************************************************/

static cv_converter*
offsetClone(
    cv_converter* const	conv)
{
    return cv_get_offset(conv->offset.value);
}


static double
offsetConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return conv->offset.value + value;
}


static float*
offsetConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = (float)(conv->offset.value + in[i]);
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = (float)(conv->offset.value + in[i]);
	}
    }

    return out;
}


static double*
offsetConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = conv->offset.value + in[i];
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = conv->offset.value + in[i];
	}
    }

    return out;
}


static int
offsetGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    const int	oper = conv->offset.value < 0 ? '-' : '+';

    return
	cvNeedsParentheses(variable)
	    ? snprintf(buf, max, "(%s) %c %g", variable, oper, 
		fabs(conv->offset.value))
	    : snprintf(buf, max, "%s %c %g", variable, oper, 
		fabs(conv->offset.value));
}


static ConverterOps	offsetOps = {
    offsetClone,
    offsetConvertDouble,
    offsetConvertFloats,
    offsetConvertDoubles,
    offsetGetExpression,
    cvSimpleFree};


/*******************************************************************************
 * Galilean Converter:
 ******************************************************************************/

static cv_converter*
cvGalileanClone(
    cv_converter* const	conv)
{
    return cv_get_galilean(conv->galilean.slope, conv->galilean.intercept);
}


static double
galileanConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return conv->galilean.slope * value + conv->galilean.intercept;
}


static float*
galileanConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = (float)(conv->galilean.slope * in[i] +
		    conv->galilean.intercept);
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = (float)(conv->galilean.slope * in[i] +
		    conv->galilean.intercept);
	}
    }

    return out;
}


static double*
galileanConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = conv->galilean.slope * in[i] + 
		    conv->galilean.intercept;
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = conv->galilean.slope * in[i] + 
		    conv->galilean.intercept;
	}
    }

    return out;
}


static int
galileanGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    const int	oper = conv->galilean.intercept < 0 ? '-' : '+';

    return
	cvNeedsParentheses(variable)
	    ? snprintf(buf, max, "%g*(%s) %c %g", conv->galilean.slope,
		variable, oper, fabs(conv->galilean.intercept))
	    : snprintf(buf, max, "%g*%s %c %g", conv->galilean.slope, variable,
		oper, fabs(conv->galilean.intercept));
}


static ConverterOps	galileanOps = {
    cvGalileanClone,
    galileanConvertDouble,
    galileanConvertFloats,
    galileanConvertDoubles,
    galileanGetExpression,
    cvSimpleFree};


/*******************************************************************************
 * Logarithmic Converter:
 ******************************************************************************/

static cv_converter*
cvLogClone(
    cv_converter* const	conv)
{
    return
        cv_get_log(
            conv->log.logE == M_LOG2E
                ? 2
                : conv->log.logE == 1
                    ? M_E
                    : conv->log.logE == M_LOG10E
                        ? 10
                        : exp(conv->log.logE));
}


static double
logConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return log(value) * conv->log.logE;
}


static float*
logConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = (float)(log(in[i]) * conv->log.logE);
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = (float)(log(in[i]) * conv->log.logE);
	}
    }

    return out;
}


static double*
logConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = log(in[i]) * conv->log.logE;
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = log(in[i]) * conv->log.logE;
	}
    }

    return out;
}


static int
logGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    return 
        conv->log.logE == M_LOG2E
            ? snprintf(buf, max, "lb(%s)", variable)
            : conv->log.logE == 1
                ? snprintf(buf, max, "ln(%s)", variable)
                : conv->log.logE == M_LOG10E
                    ? snprintf(buf, max, "lg(%s)", variable)
                    : snprintf(buf, max, "%g*ln(%s)", conv->log.logE, variable);
}


static ConverterOps	logOps = {
    cvLogClone,
    logConvertDouble,
    logConvertFloats,
    logConvertDoubles,
    logGetExpression,
    cvSimpleFree};


/*******************************************************************************
 * Exponential Converter:
 ******************************************************************************/

static cv_converter*
expClone(
    cv_converter* const	conv)
{
    return cv_get_pow(conv->exp.base);
}


static double
expConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return pow(conv->exp.base, value);
}

static float*
expConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = (float)(pow(conv->exp.base, in[i]));
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = (float)(pow(conv->exp.base, in[i]));
	}
    }

    return out;
}


static double*
expConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	size_t	i;

	if (in < out) {
	    for (i = count; i-- > 0;)
		out[i] = pow(conv->exp.base, in[i]);
	}
	else {
	    for (i = 0; i < count; i++)
		out[i] = pow(conv->exp.base, in[i]);
	}
    }

    return out;
}


static int
expGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    return 
	cvNeedsParentheses(variable)
	    ? snprintf(buf, max, "pow(%g, (%s))", conv->exp.base, variable)
	    : snprintf(buf, max, "pow(%g, %s)", conv->exp.base, variable);
}


static ConverterOps	expOps = {
    expClone,
    expConvertDouble,
    expConvertFloats,
    expConvertDoubles,
    expGetExpression,
    cvSimpleFree};


/*******************************************************************************
 * Composite Converter:
 ******************************************************************************/

static cv_converter*
compositeClone(
    cv_converter* const	conv)
{
    return cv_combine(conv->composite.first, conv->composite.second);
}


static double
compositeConvertDouble(
    const cv_converter* const	conv,
    const double		value)
{
    return
	cv_convert_double(conv->composite.second,
	    cv_convert_double(((CompositeConverter*)conv)->first, value));
}


static float*
compositeConvertFloats(
    const cv_converter* const	conv,
    const float* const		in,
    const size_t		count,
    float* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	out =
	    cv_convert_floats(
		conv->composite.second,
		cv_convert_floats(conv->composite.first, in, count, out),
		count,
		out);
    }

    return out;
}


static double*
compositeConvertDoubles(
    const cv_converter* const	conv,
    const double* const		in,
    const size_t		count,
    double* 			out)
{
    if (conv == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
	out = 
	    cv_convert_doubles(
		conv->composite.second,
		cv_convert_doubles(conv->composite.first, in, count, out),
		count,
		out);
    }

    return out;
}


static void
compositeFree(
    cv_converter* const	conv)
{
    cv_free(conv->composite.first);
    cv_free(conv->composite.second);
    free(conv);
}


static int
compositeGetExpression(
    const cv_converter* const	conv,
    char* const			buf,
    const size_t		max,
    const char* const		variable)
{
    char	tmpBuf[132];
    int		nchar = cv_get_expression(conv->composite.first, buf, max,
	variable);

    if (nchar >= 0) {
	buf[max-1] = 0;

	if (cvNeedsParentheses(buf)) {
	    nchar = snprintf(tmpBuf, sizeof(tmpBuf), "(%s)", buf);
	}
	else {
	    (void)strncpy(tmpBuf, buf, sizeof(tmpBuf));

	    tmpBuf[sizeof(tmpBuf)-1] = 0;
	}

	nchar = cv_get_expression(conv->composite.second, buf, max, tmpBuf);
    }

    return nchar;
}


static ConverterOps	compositeOps = {
    compositeClone,
    compositeConvertDouble,
    compositeConvertFloats,
    compositeConvertDoubles,
    compositeGetExpression,
    compositeFree};


/*******************************************************************************
 * Public API:
 ******************************************************************************/

/*
 * Returns the trivial converter (i.e., y = x).
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Returns:
 *	The trivial converter.
 */
cv_converter*
cv_get_trivial()
{
    return &trivialConverter;
}


/*
 * Returns the reciprocal converter (i.e., y = 1/x).
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Returns:
 *	The reciprocal converter.
 */
cv_converter*
cv_get_inverse()
{
    return &reciprocalConverter;
}


/*
 * Returns a converter that multiplies values by a number (i.e., y = ax).
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Arguments:
 *	slope	The number by which to multiply values.
 * Returns:
 *	NULL	Necessary memory couldn't be allocated.
 *	else	A converter that will multiply values by the given number.
 */
cv_converter*
cv_get_scale(
    const double	slope)
{
    cv_converter*	conv;

    if (slope == 1) {
	conv = &trivialConverter;
    }
    else {
	conv = malloc(sizeof(ScaleConverter));

	if (conv != NULL) {
	    conv->ops = &scaleOps;
	    conv->scale.value = slope;
	}
    }

    return conv;
}


/*
 * Returns a converter that adds a number to values (i.e., y = x + b).
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Arguments:
 *	offset	The number to be added.
 * Returns:
 *	NULL	Necessary memory couldn't be allocated.
 *	else	A converter that adds the given number to values.
 */
cv_converter*
cv_get_offset(
    const double	offset)
{
    cv_converter*	conv;

    if (offset == 0) {
	conv = &trivialConverter;
    }
    else {
	conv = malloc(sizeof(OffsetConverter));

	if (conv != NULL) {
	    conv->ops = &offsetOps;
	    conv->offset.value = offset;
	}
    }

    return conv;
}


/*
 * Returns a Galilean converter (i.e., y = ax + b).
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Arguments:
 *	slope	The number by which to multiply values.
 *	offset	The number to be added.
 * Returns:
 *	NULL	Necessary memory couldn't be allocated.
 *	else	A Galilean converter corresponding to the inputs.
 */
cv_converter*
cv_get_galilean(
    const double	slope,
    const double	intercept)
{
    cv_converter*	conv;

    if (slope == 1) {
	conv = cv_get_offset(intercept);
    }
    else if (intercept == 0) {
	conv = cv_get_scale(slope);
    }
    else {
	cv_converter*	tmp = malloc(sizeof(GalileanConverter));

	if (tmp != NULL) {
	    tmp->ops = &galileanOps;
	    tmp->galilean.slope = slope;
	    tmp->galilean.intercept = intercept;
	    conv = tmp;
	}
    }

    return conv;
}


/*
 * Returns a logarithmic converter (i.e., y = log(x/x0) in some base).
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Arguments:
 *	base		The logarithmic base (e.g., 2, M_E, 10).  Must be
 *                      greater than one.
 * Returns:
 *	NULL		"base" is invalid or necessary memory couldn't be
 *			allocated.
 *	else		A logarithmic converter corresponding to the inputs.
 */
cv_converter*
cv_get_log(
    const double	base)
{
    cv_converter*	conv;

    if (base <= 1) {
	conv = NULL;
    }
    else {
	conv = malloc(sizeof(LogConverter));

	if (conv != NULL) {
	    conv->ops = &logOps;
	    conv->log.logE = 
                base == 2
                    ? M_LOG2E
                    : base == M_E
                        ? 1
                        : base == 10
                            ? M_LOG10E
                            : 1/log(base);
	}
    }

    return conv;
}


/*
 * Returns an exponential converter (i.e., y = pow(b, x) in some base "b").
 * When finished with the converter, the client should pass the converter to
 * cv_free().
 *
 * Arguments:
 *	base		The desired base.  Must be positive.
 * Returns:
 *	NULL		"base" is invalid or necessary memory couldn't be
 *			allocated.
 *	else		An exponential converter corresponding to the inputs.
 */
cv_converter*
cv_get_pow(
    const double	base)
{
    cv_converter*	conv;

    if (base <= 0) {
	conv = NULL;
    }
    else {
	conv = malloc(sizeof(ExpConverter));

	if (conv != NULL) {
	    conv->ops = &expOps;
	    conv->exp.base = base;
	}
    }

    return conv;
}


/*
 * Returns a converter corresponding to the sequential application of two
 * other converters.  The returned converter should be passed to cv_free() when
 * it is no longer needed.
 *
 * Arguments:
 *	first	The converter to be applied first.  May be passed to cv_free()
 *		upon return.
 *	second	The converter to be applied second.  May be passed to cv_free()
 *		upon return.
 * Returns:
 *	NULL	Either "first" or "second" is NULL or necessary memory couldn't
 *		be allocated.
 *      else    A converter corresponding to the sequential application of the
 *              given converters.  If one of the input converters is the trivial
 *              converter, then the returned converter will be the other input
 *              converter.
 */
cv_converter*
cv_combine(
    cv_converter* const	first,
    cv_converter* const	second)
{
    cv_converter*	conv;

    if (first == NULL || second == NULL) {
	conv = NULL;
    }
    else if (IS_TRIVIAL(first)) {
	conv = CV_CLONE(second);
    }
    else if (IS_TRIVIAL(second)) {
	conv = CV_CLONE(first);
    }
    else {
	conv = NULL;

	if (IS_RECIPROCAL(first)) {
	    if (IS_RECIPROCAL(second)) {
		conv = cv_get_trivial();
	    }
	}
	else if (IS_SCALE(first)) {
	    if (IS_SCALE(second)) {
		conv = cv_get_scale(first->scale.value * second->scale.value);
	    }
	    else if (IS_OFFSET(second)) {
		conv = cv_get_galilean(first->scale.value, second->offset.value);
	    }
	    else if (IS_GALILEAN(second)) {
		conv = cv_get_galilean(
		    first->scale.value * second->galilean.slope, 
		    second->galilean.intercept);
	    }
	}
	else if (IS_OFFSET(first)) {
	    if (IS_SCALE(second)) {
		conv = cv_get_galilean(second->scale.value, 
		    first->offset.value * second->scale.value);
	    }
	    else if (IS_OFFSET(second)) {
		conv = cv_get_offset(first->offset.value + second->offset.value);
	    }
	    else if (IS_GALILEAN(second)) {
		conv = cv_get_galilean(second->galilean.slope, 
		    first->offset.value * second->galilean.slope +
			second->galilean.intercept);
	    }
	}
	else if (IS_GALILEAN(first)) {
	    if (IS_SCALE(second)) {
		conv = cv_get_galilean(
		    second->scale.value * first->galilean.slope,
		    second->scale.value * first->galilean.intercept);
	    }
	    else if (IS_OFFSET(second)) {
		conv = cv_get_galilean(first->galilean.slope,
		    first->galilean.intercept + second->offset.value);
	    }
	    else if (IS_GALILEAN(second)) {
		conv = cv_get_galilean(
		    second->galilean.slope * first->galilean.slope,
		    second->galilean.slope * first->galilean.intercept +
			second->galilean.intercept);
	    }
	}

	if (conv == NULL) {
	    /*
	     * General case: create a composite converter.
	     */
	    cv_converter*	c1 = CV_CLONE(first);
            int                 error = 1;

            if (c1 != NULL) {
                cv_converter*	c2 = CV_CLONE(second);

                if (c2 != NULL) {
                    conv = malloc(sizeof(CompositeConverter));

                    if (conv != NULL) {
                        conv->composite.ops = &compositeOps;
                        conv->composite.first = c1;
                        conv->composite.second = c2;
                        error = 0;
                    }                   /* "conv" allocated */

                    if (error)
                        cv_free(c2);
                }                       /* "c2" allocated */

                if (error)
                    cv_free(c1);
            }                           /* "c1" allocated */
	}                               /* "conv != NULL" */
    }                                   /* "first" & "second" not trivial */

    return conv;
}


/*
 * Frees resources associated with a converter.  Use of the converter argument
 * subsequent to this function may result in undefined behavior.
 *
 * Arguments:
 *	conv	The converter to have its resources freed or NULL.  The
 *		converter must have been returned by this module.
 */
void
cv_free(
    cv_converter* const	conv)
{
    if (conv != NULL) {
	conv->ops->free((cv_converter*)conv);
    }
}


/*
 * Converts a float.
 *
 * Arguments:
 *	converter	Pointer to the converter.
 *	value		The value to be converted.
 * Returns:
 *	The converted value.
 */
float
cv_convert_float(
    const cv_converter*	converter,
    const float		value)
{
    return (float)converter->ops->convertDouble(converter, value);
}


/*
 * Converts a double.
 *
 * Arguments:
 *	converter	Pointer to the converter.
 *	value		The value to be converted.
 * Returns:
 *	The converted value.
 */
double
cv_convert_double(
    const cv_converter*	converter,
    const double	value)
{
    return converter->ops->convertDouble(converter, value);
}


/*
 * Converts an array of floats.
 *
 * Arguments:
 *	converter	Pointer to the converter.
 *	in		Pointer to the values to be converted.  The array may
 *			overlap "out".
 *	count		The number of values to be converted.
 *	out		Pointer to the output array for the converted values.
 *			The array may overlap "in".
 * Returns:
 *	NULL		"converter", "in", or "out" is NULL.
 *	else		Pointer to the output array, "out".
 */
float*
cv_convert_floats(
    const cv_converter*	converter,
    const float* const	in,
    const size_t	count,
    float*		out)
{
    if (converter == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
        out = converter->ops->convertFloats(converter, in, count, out);
    }

    return out;
}


/*
 * Converts an array of doubles.
 *
 * Arguments:
 *	converter	Pointer to the converter.
 *	in		Pointer to the values to be converted.  The array may
 *			overlap "out".
 *	count		The number of values to be converted.
 *	out		Pointer to the output array for the converted values.
 *			The array may overlap "in".
 * Returns:
 *	NULL		"converter", "in", or "out" is NULL.
 *	else		Pointer to the output array, "out".
 */
double*
cv_convert_doubles(
    const cv_converter*	converter,
    const double* const	in,
    const size_t	count,
    double*		out)
{
    if (converter == NULL || in == NULL || out == NULL) {
	out = NULL;
    }
    else {
    	out = converter->ops->convertDoubles(converter, in, count, out);
    }

    return out;
}


/*
 * Returns a string expression representation of a converter.
 *
 * Arguments:
 *	conv		The converter.
 *	buf		The buffer into which to write the expression.
 *	max		The size of the buffer.
 *	variable	The string to be used as the input value for the
 *			converter.
 * RETURNS
 *	<0	An error was encountered.
 *	else	The number of bytes formatted excluding the terminating null.
 */
int
cv_get_expression(
    const cv_converter* const	conv,
    char* const			buf,
    size_t			max,
    const char* const		variable)
{
    return conv->ops->getExpression(conv, buf, max, variable);
}
