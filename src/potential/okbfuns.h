/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * \file okbfuns.h
 * \brief
 */

#ifndef OKBFUNS_H
#define OKBFUNS_H

#include "gmt_dev.h"		/* Requires GMT to compile and link */
#include "gmt_glib.h"

struct BODY_VERTS {
	double  x, y, z;
};

struct BODY_DESC {
	unsigned int n_f, *n_v, *ind;
};

struct LOC_OR {
	double  x, y, z;
};

struct MAG_PARAM {
	double	rim[3];
};

struct MAG_VAR {		/* Used when only the modulus of magnetization varies */
	double	rk[3];
};

EXTERN_MSC double okabe (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double rho, bool is_grav,
		struct BODY_DESC bd_desc, struct BODY_VERTS *bd_vert, unsigned int km, unsigned int pm, struct LOC_OR *loc_or, struct MAG_PARAM *okabe_mag_param, struct MAG_VAR *okabe_mag_var);

#endif /* OKBFUNS_H */
