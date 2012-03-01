/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Public header-file for the Unidata units(3) library.
 */

#ifndef UT_UNITS_H_INCLUDED
#define UT_UNITS_H_INCLUDED

#include "udunits2.h"

#define UT_NAMELEN              32      /* maximum length of a unit string
					 * (including all prefixes and EOS) */

/*
 *	Macro for declaring functions regardless of the availability of 
 *	function prototypes.  NB: will need double parens in actual use (e.g.
 *	"int func PROTO((int a, char *cp))").
 */
#ifndef	PROTO
#   if defined(__STDC__) || defined(__GNUC__) \
	|| defined(__cplusplus) || defined(c_plusplus)
#	define	PROTO(a)	a
#   else
#	define	PROTO(a)	()
#   endif
#endif


#define	UT_EOF		 1	/* end-of-file encountered */
#define	UT_ENOFILE	-1	/* no units-file */
#define	UT_ESYNTAX	-2	/* syntax error */
#define	UT_EUNKNOWN	-3	/* unknown specification */
#define	UT_EIO		-4	/* I/O error */
#define	UT_EINVALID	-5	/* invalid unit-structure */
#define	UT_ENOINIT	-6	/* package not initialized */
#define	UT_ECONVERT	-7	/* two units are not convertable */
#define	UT_EALLOC	-8	/* memory allocation failure */
#define UT_ENOROOM	-9	/* insufficient room supplied */
#define UT_ENOTTIME	-10	/* not a unit of time */
#define UT_DUP		-11	/* duplicate unit */

#define UT_MAXNUM_BASE_QUANTITIES	10

typedef double	UtOrigin;	/* origin datatype */
typedef double	UtFactor;       /* conversion-factor datatype */

/*
 *  Unit-structure.  Don't rely on or mess with the members!
 */
typedef struct utUnit {
    ut_unit*		unit2;
} utUnit;

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Initialize the units(3) package.
 */
extern int	utInit		PROTO((
    const char	*path
));

/*
 * Indicate if the units(3) package has been initialized.
 */
extern int	utIsInit	PROTO((void));

/*
 * Initialize a unit-structure.
 */
extern void	utIni		PROTO((
    utUnit* const	unit
));

/*
 *	Decode a formatted unit specification into a unit-structure.
 */
extern int	utScan		PROTO((
    const char	*spec,
    utUnit	*up
));

/*
 *	Convert a temporal value into a UTC Gregorian date and time.
 */
extern int	utCalendar	PROTO((
    double		value,
    const utUnit	*unit,
    int			*year,
    int			*month,
    int			*day,
    int			*hour,
    int			*minute,
    float		*second
));

/*
 *	Convert a date into a temporal value.  The date is assumed to
 *	use the Gregorian calendar if on or after noon, October 15, 1582;
 *	otherwise, the date is assumed to use the Julian calendar.
 */
extern int	utInvCalendar	PROTO((
    int			year,
    int			month,
    int			day,
    int			hour,
    int			minute,
    double		second,
    const utUnit	*unit,
    double		*value
));

/*
 *	Indicate if a unit structure refers to a unit of time.
 */
extern int	utIsTime	PROTO((
    const utUnit	*up
));

/*
 *	Indicate if a unit structure has an origin.
 */
extern int	utHasOrigin	PROTO((
    const utUnit	*up
));

/*
 *	Clear a unit structure.
 */
extern utUnit*	utClear		PROTO((
    utUnit	*unit
));

/*
 *	Copy a unit-strcture.
 */
extern utUnit*	utCopy		PROTO((
    const utUnit	*source,
    utUnit		*dest
));

/*
 *	Multiply one unit-structure by another.
 */
extern utUnit*	utMultiply	PROTO((
    const utUnit	*term1,
    const utUnit	*term2, 
    utUnit		*result
));

/*
 *	Divide one unit-structure by another.
 */
extern utUnit*	utDivide	PROTO((
    const utUnit	*numer,
    const utUnit	*denom, 
    utUnit		*result
));

/*
 *	Form the reciprocal of a unit-structure.
 */
extern utUnit*	utInvert	PROTO((
    const utUnit	*source,
    utUnit		*dest
));

/*
 *	Raise a unit-structure to a power.
 */
extern utUnit*	utRaise		PROTO((
    const utUnit	*source,
    int			power,
    utUnit		*result
));

/*
 *	Shift the origin of a unit-structure by an arithmetic amount.
 */
extern utUnit*	utShift		PROTO((
    const utUnit	*source,
    double		amount,
    utUnit		*result
));

/*
 *	Scale a unit-structure.
 */
extern utUnit*	utScale		PROTO((
    const utUnit	*source,
    double		factor,
    utUnit		*result
));

/*
 *	Compute the conversion factor between two unit-structures.
 */
extern int	utConvert	PROTO((
    const utUnit	*from,
    const utUnit	*to,
    double		*slope,
    double		*intercept
));

/*
 *	Encode a unit-structure into a formatted unit-specification.
 */
extern int	utPrint		PROTO((
    const utUnit	*unit,
    char		**buf
));

/*
 *  Add a unit-structure to the units-table.
 */
extern int	utAdd		PROTO((
    char		*name,
    int			HasPlural,
    const utUnit	*unit
));

/*
 *  Return the unit-structure corresponding to a unit-specification.
 */
extern int	utFind		PROTO((
    char	*spec,
    utUnit	*up
));

/*
 * Free the resources of a unit-structure.
 */
extern void	utFree		PROTO((
    utUnit* const	unit
));

/*
 *	Terminate use of this package.
 */
extern void	utTerm		PROTO((void));

#ifdef __cplusplus
}
#endif

#endif	/* UT_UNITS_H_INCLUDED not defined */
