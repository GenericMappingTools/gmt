/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

/*!
 * \file modeltime.h
 * \brief
 */

#ifndef MODELTIME_H
#define MODELTIME_H

/* Miscellaneous model time structure and external functions used in grdseamount and grdflexure */

struct GMT_MODELTIME {  /* Hold info about time */
    double value;   /* Time as given by user in years */
    double scale;   /* Scale factor from years to formatted unit time */
    char unit;  /* Either M (Myr), k (kyr), or blank (implies y) */
    char tag[GMT_LEN32];    /* Formatted time tag probably with unit */
    unsigned int u; /* For custom labeling: Either 0 (yr), 1 (kyr), or 2 (Myr) */
};

EXTERN_MSC void gmt_modeltime_name (struct GMT_CTRL *GMT, char *file, char *format, struct GMT_MODELTIME *T);
EXTERN_MSC unsigned int gmt_modeltime_array (struct GMT_CTRL *GMT, char *arg, bool *log, struct GMT_MODELTIME **T_array);
EXTERN_MSC int gmt_modeltime_validate (struct GMT_CTRL *GMT, char option, char *file);
EXTERN_MSC double gmt_get_modeltime (char *A, char *unit, double *scale);

#endif /* MODELTIME_H */
