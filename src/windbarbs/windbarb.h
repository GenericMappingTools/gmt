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

#ifndef _GMT_WINDBARB_H
#define _GMT_WINDBARB_H

struct GMT_BARB_ATTR {
	/* Container for common attributes for plot attributes of wind barbs */
	unsigned int status;	/* Bit flags for barb position information (see GMT_enum_vecattr above) */
	bool parsed_v4;		/* true if we parsed old-style <barbwidth/barblength/barbangle/barbscale> attribute */
	float width;		/* Width of wind barb in inches */
	float length;		/* Length of wind barb inches */
	float angle;		/* Angle of wind barb */
	float scale;		/* Scale of wind barb */
	struct GMT_PEN pen;	/* Pen for outline */
	struct GMT_FILL fill;	/* Fill */
};

EXTERN_MSC void gmt_barb_syntax (struct GMT_CTRL *GMT, char option, char *string, unsigned int mode);
EXTERN_MSC int gmt_draw_barb (struct GMT_CTRL *GMT, double x0, double y0, double lat, double theta, double spd, struct GMT_BARB_ATTR B, struct GMT_PEN *pen, struct GMT_FILL *fill, unsigned int outline);
EXTERN_MSC int gmt_init_barb_param (struct GMT_CTRL *GMT, struct GMT_BARB_ATTR *B, bool set, bool outline, struct GMT_PEN *pen, bool do_fill, struct GMT_FILL *fill);
EXTERN_MSC int gmt_parse_barb (struct GMT_CTRL *GMT, char *text, struct GMT_BARB_ATTR *B);

#endif /* _GMT_WINDBARB_H */
