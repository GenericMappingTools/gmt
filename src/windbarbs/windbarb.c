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
/*
 *
 *			W I N D B A R B . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * windbarb.c contains code related to plotting wind barbs.  These functions
 * requires we pass both the GMT and PSL control strucure pointers.
 *
 * Author:	HIGAKI, Masakazu
 * Date:	1-JUN-2017
 * Version:	6 API
 *
 * PUBLIC Functions include (4):
 *
 *      gmt_barb_syntax          : Show wind barb option syntax
 *      gmt_draw_barb            : Plot wind barb
 *      gmt_init_barb_param      : Initialize wind barb parameters
 *      gmt_parse_barb           : Parse wind barb options
 *
 */
#include "gmt_dev.h"
#include "windbarb.h"

/*! Use mode to control which options are displayed */
void gmt_barb_syntax (struct GMT_CTRL *GMT, char option, char *string, unsigned int mode) {
	/* Mode is composed of bit-flags to control which lines are printed.
	 * Items without if-test are common to all barbs.
	 * 1	= Accepts +z (not mathangle)
	 */
	struct GMTAPI_CTRL *API = GMT->parent;
	GMT_Usage (API, 1, "\n-%c<params>", option);
	GMT_Usage (API, -2, "%s Append length of wind barbs, with optional modifiers:", string);
	GMT_Usage (API, 3, "+a<angle> to set angle of wind barb [120]");
	GMT_Usage (API, 3, "+g<fill> to set fill or use - to turn off fill [default fill].");
	GMT_Usage (API, 3, "+j<just> to justify wind barb at (b)eginning [default], (e)nd, or (c)enter.");
	GMT_Usage (API, 3, "+p[-][<pen>] to set pen attributes, prepend - to turn off outlines [default pen and outline].");
	GMT_Usage (API, 3, "+s[scale] to set the wind speed which corresponds to a long wind barb [default 5]");
	GMT_Usage (API, 3, "+w[width] to set the width of wind barbs");
	if (mode & 1) GMT_Usage (API, 3, "+z if (u,v) wind components are given instead of (azimuth,speed) on input.");
}

int gmt_draw_barb (struct GMT_CTRL *GMT, double x0, double y0, double lat, double theta, double spd, struct GMT_BARB_ATTR B, struct GMT_PEN *pen, struct GMT_FILL *fill, unsigned int outline)
{
	double w1, dx, dy, y;
	double s, c;
	int i, ispd, n = 0, n_pennant, n_barb;
	struct GMT_CUSTOM_SYMBOL symbol;
	struct GMT_CUSTOM_SYMBOL_ITEM bs[GMT_LEN256];
	double size[1];
	char *text = "\0";

	if (spd == 0.) return 0;

	/* make struct of custom symbol for wind barbs */
	strcpy (symbol.name, "barb");
	symbol.first = bs;

	bs[n].action = GMT_SYMBOL_ROTATE;
	bs[n].p[0] = theta-90.;
	bs[n].next = bs+1;
	n ++;

	/* staff of wind barb */
	ispd = irint (spd / B.scale * 2.);
	n_pennant =  ispd / 10;
	n_barb    = (ispd % 10) / 2;
	if (ispd % 2) n_barb ++;
	w1 = 1./6.;
	y = w1 * (n_pennant + n_barb + 1);
	if (y < 1.) y = 1.;

	bs[n].x = 0.;
	bs[n].y = 0.;
	bs[n].action = GMT_SYMBOL_MOVE;
	bs[n].next   = bs+n+1;
	n ++;
	bs[n].x = 0.;
	bs[n].y = y;
	bs[n].action = GMT_SYMBOL_DRAW;
	bs[n].next   = bs+n+1;
	n ++;

	/* pennants and barbs */
	if (ispd == 1) y -= w1;  /* space for short barb */

	/* on the southern hemisphere, put barbs on the righthand side of staff */
	sincosd (copysign(B.angle, lat), &s, &c);
	dx =  s * B.width / B.length;
	dy = -c * B.width / B.length;

	while (ispd > 0 && n < GMT_LEN256-3) {
		bs[n].x = 0.;
		bs[n].y = y;
		bs[n].action = GMT_SYMBOL_MOVE;
		bs[n].next   = bs+n+1;
		n ++;

		/* pennants */
		if (ispd >= 10) {
			bs[n].x = dx;
			bs[n].y = y - w1 + dy;
			bs[n].action = GMT_SYMBOL_DRAW;
			bs[n].next   = bs+n+1;
			n ++;

			y -= w1;
			bs[n].x = 0;
			bs[n].y = y;

			ispd -= 10;
			if (ispd < 10) y -= w1; /* space between pennant and barb */
		} else if (ispd >= 2) {
			bs[n].x =     dx;
			bs[n].y = y + dy;
			ispd -= 2;
			y -= w1;
		} else {	 /* short barb */
			bs[n].x =     dx/2.;
			bs[n].y = y + dy/2.;
			ispd --;
		}
		bs[n].action = GMT_SYMBOL_DRAW;
		bs[n].next   = bs+n+1;
		n ++;
	}

	for(i = 0; i < n; i ++) {
		bs[i].fill = NULL;
		bs[i].pen  = NULL;
	}
	bs[n-1].next = NULL;

	size[0] = B.length;
	return gmt_draw_custom_symbol (GMT, x0, y0, size, text, &symbol, pen, fill, outline);
}

/*! Deal with pen/fill settings */
int gmt_init_barb_param (struct GMT_CTRL *GMT, struct GMT_BARB_ATTR *B, bool set, bool outline, struct GMT_PEN *pen, bool do_fill, struct GMT_FILL *fill) {
	bool no_outline = false, no_fill = false;
	if (set) {	/* Determine proper settings for fill or outline */
		if (outline && (B->status & PSL_VEC_OUTLINE2) == 0) B->pen = *pen;	/* If no +p<pen> but -W<pen> was used, use same pen for barbs */
		else if (!outline && B->status & PSL_VEC_OUTLINE2) *pen = B->pen;	/* If no -W<pen> was set but +p<pen> given, use same pen for barbs */
		else if (!outline && (B->status & PSL_VEC_OUTLINE2) == 0) no_outline = true;
		if (do_fill && (B->status & PSL_VEC_FILL2) == 0) B->fill = *fill;	/* If no +g<fill> but -G<fill> was used, use same fill for barbs */
		else if (!do_fill && B->status & PSL_VEC_FILL2) no_fill = false;		/* If no -G<fill> was set but +g<fill> given, we do nothing here */
		else if (!do_fill && (B->status & PSL_VEC_FILL2) == 0) no_fill = true;	/* Neither -G<fill> nor +g<fill> were set */
		if (no_outline && no_fill && (B->status & PSL_VEC_HEADS)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot draw wind barbs without specifying at least one of outline or fill.\n");
			return 1;
		}
	}
	return 0;
}

/*! Parser for -Q */
GMT_LOCAL int gmt_parse_barb_v5 (struct GMT_CTRL *GMT, char *text, struct GMT_BARB_ATTR *B) {

	unsigned int pos = 0, k, error = 0;
	bool p_opt = false, g_opt = false;
	int j;
	char p[GMT_BUFSIZ];

	B->pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &B->fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	B->status = 0;	/* Start with no flags turned on */
	for (k = 0; text[k] && text[k] != '+'; k++);	/* Either find the first plus or run out or chars */
	strncpy (p, text, k); p[k] = 0;

	while ((gmt_strtok (&text[k], "+", &pos, p))) {	/* Parse any +<modifier> statements */
		switch (p[0]) {
			case 'a': B->angle = (float)atof (&p[1]);	break;	/* Wind barb angle [120] */
			case 'g':	/* Pennant fill +g[-|<fill>]*/
				g_opt = true;	/* Marks that +g was used */
				if (p[1] == '-') break; /* Do NOT turn on fill */
				B->status |= PSL_VEC_FILL;
				if (p[1]) {
					if (gmt_getfill (GMT, &p[1], &B->fill)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +g<fill> modifier %c\n", &p[1]);
						error++;
					}
					B->status |= PSL_VEC_FILL2;
				}
				break;
			case 'j':	/* Wind barb justification */
				switch (p[1]) {
					case 'b': B->status |= PSL_VEC_JUST_B;	break;	/* Input (x,y) refers to beginning point */
					case 'c': B->status |= PSL_VEC_JUST_C;	break;	/* Input (x,y) refers to center point */
					case 'e': B->status |= PSL_VEC_JUST_E;	break;	/* Input (x,y) refers to end point */
					default:  /* Bad justifier code */
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +j<just> modifier %c\n", p[1]);
						error++;
						break;
				}
				break;
			case 'p':	/* pen and outline +p[-][<pen>] */
				p_opt = true;	/* Marks that +p was used */
				if (p[1] == '-')	/* Do NOT turn on outlines */
					j = 2;
				else {
					j = 1;
					B->status |= PSL_VEC_OUTLINE;
				}
				if (p[j]) {	/* Change default pen */
					if (gmt_getpen (GMT, &p[j], &B->pen)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +p<pen> modifier %c\n", &p[1]);
						error++;
					}
					B->status |= PSL_VEC_OUTLINE2;	/* Flag that a pen specification was given */
				}
				break;
			case 's':	/* Scale of long wind barbs */
				B->scale = (float)atof (&p[1]);
				break;
			case 'w':	/* Wind barb width */
				B->width = (float)gmt_M_to_inch (GMT, &p[1]);
				break;
			case 'z':	/* Input (angle,length) are vector components (dx,dy) instead */
				B->status |= PSL_VEC_COMPONENTS;
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad modifier +%c\n", p[0]);
				error++;
				break;
		}
	}
	if (!g_opt) B->status |= PSL_VEC_FILL;	/* Default is to fill barb with current fill unless (a) no fill given or (b) turned off with +g- */
	if (!p_opt) B->status |= PSL_VEC_OUTLINE;	/* Default is to draw barb outline with current pen unless explicitly turned off with +p- */

	/* Default value for the width of wind barb */
	if (B->width <= 0.0 && B->length > 0.0) B->width = B->length / 2;

	/* Set parameters */
	gmt_init_barb_param (GMT, B, false, false, NULL, false, NULL);

	return (error);
}

/*! Parser for -Q */
int gmt_parse_barb (struct GMT_CTRL *GMT, char *text, struct GMT_BARB_ATTR *B) {

	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""};
	unsigned int error = 0;
	int j;

	if (gmt_M_compat_check (GMT, 4) && (strchr (text, '/') && !strchr (text, '+'))) {	/* Old-style args */
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Barb barbwidth/barblength/barbangle/barbscale is deprecated; see -Q documentation.\n");
		if (text[0]) {	/* We specified the four parameters */
			if (sscanf (text, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d) != 4) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -Q option: Could not decode barbwidth/barblength/barbangle/barbscale\n");
				error++;
			}
			else {	/* Turn the old args into new args */
				B->width = (float)gmt_M_to_inch (GMT, txt_a);
				B->length = (float)gmt_M_to_inch (GMT, txt_b);
				B->angle = (float)atof (txt_c);
				B->scale = (float)atof (txt_d);
			}
		}
		B->status |= (PSL_VEC_JUST_B + PSL_VEC_FILL);	/* Start filled barb at node location */
	}
	else {
		if (text[0] == '+') {	/* No size (use default), just attributes */
			error += gmt_parse_barb_v5 (GMT, text, B);
		}
		else {	/* Size, plus possible attributes */
			j = sscanf (text, "%[^+]%s", txt_a, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
			if (j == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
			if (j >= 1) B->length = gmt_M_to_inch (GMT, txt_a);	/* Length of barb */
			error += gmt_parse_barb_v5 (GMT, txt_b, B);
		}
	}

	return (error);
}
