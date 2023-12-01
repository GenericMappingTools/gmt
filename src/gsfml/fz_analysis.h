/*
  * Copyright (c) 2015-2023 by P. Wessel
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * Contact info: http://www.soest.hawaii.edu/PT/GSFML
 *--------------------------------------------------------------------
 *
 * Named indices for output table for fzanalyzer trace analysis.
 *
 * Author:	Paul Wessel
 * Date:	01-DEC-2023 (requires GMT >= 6)
 */

#ifndef _FZ_ANALYSIS_H
#define _FZ_ANALYSIS_H

#include "gsfml_config.h"

#define DEF_D_WIDTH	25.0	/* Default width of central corridor */
#define DEF_L_MIN	0.0	/* Minimum compression for search */
#define DEF_L_MAX	1.0	/* Maximum compression for search */
#define DEF_L_INC	0.05	/* Increment for compression search */
#define DEF_M_MIN	0.0	/* Minimum asymmetry for search */
#define DEF_M_MAX	1.0	/* Maximum asymmetry for search */
#define DEF_M_INC	0.05	/* Increment for asymmetry search */
#define DEF_W_MIN	1.0	/* Minimum width for search */
#define DEF_W_MAX	50.0	/* Maximum width for search */
#define DEF_W_INC	1.0	/* Increment for width search */

#define N_FZ_ANALYSIS_COLS	61	/* Number of output columns for final result */

#define POS_XR	0	/* Longitude of raw digitizing */
#define POS_YR	1	/* Latitude of raw digitizing */
#define POS_DR	2	/* Distance at point along raw digitized trace */
#define POS_AR	3	/* Azimuth at point along raw digitized trace */
#define POS_ZR	4	/* Data value at point of raw digitizing */
#define POS_TL	5	/* Crustal age estimate at left side of FZ (negative distances) */
#define POS_TR	6	/* Crustal age estimate at right side of FZ (positive distances) */
#define POS_SD	7	/* Offset of data minimum (in km) from raw line origin */
#define POS_ST	8	/* Offset of trough model minimum (in km) from raw line origin */
#define POS_SB	9	/* Offset of blend model minimum (in km) from raw line origin */
#define POS_SE	10	/* Offset of blend model maximum slope (in km) from raw line origin */
#define POS_BL	11	/* Best asymmetry value [0-1] */
#define POS_OR	12	/* Orientation of model profile (-1 =>old on negative dist side, +1 => old on positive dist side) */
#define POS_WD	13	/* Width of data trough */
#define POS_WT	14	/* Width of model trough (trough) */
#define POS_WB	15	/* Width of model trough (blend) */
#define POS_AD	16	/* Peak-to-trough amplitude from data */
#define POS_AT	17	/* Peak-to-trough amplitude from model (trough) */
#define POS_AB	18	/* Peak-to-trough amplitude from model (blend) */
#define POS_UT	19	/* Flank relative amplitude from model (trough) */
#define POS_UB	20	/* Flank relative amplitude from model (blend) */
#define POS_VT	21	/* Variance reduction (%) from model (trough) */
#define POS_VB	22	/* Variance reduction (%) from model (blend) */
#define POS_FT	23	/* F-statistic for model (trough) */
#define POS_FB	24	/* F-statistic for model (blend) */
#define POS_XDL	25	/* Longitude of data minimum left bounds */
#define POS_XD0	26	/* Longitude of data minimum (trough) */
#define POS_XDR	27	/* Longitude of data minimum right bounds */
#define POS_YDL	28	/* Latitude of data minimum left bounds */
#define POS_YD0	29	/* Latitude of data minimum (trough) */
#define POS_YDR	30	/* Latitude of data minimum right bounds */
#define POS_ZDL	31	/* Data value of data minimum left bounds */
#define POS_ZD0	32	/* Data value of data minimum (trough) */
#define POS_ZDR	33	/* Data value of data minimum right bounds */
#define POS_XTL	34	/* Longitude of model minimum (trough) left bounds */
#define POS_XT0	35	/* Longitude of model minimum (trough) */
#define POS_XTR	36	/* Longitude of model minimum (trough) right bounds */
#define POS_YTL	37	/* Latitude of model minimum (trough) left bounds */
#define POS_YT0	38	/* Latitude of model minimum (trough) */
#define POS_YTR	39	/* Latitude of model minimum (trough) right bounds */
#define POS_ZTL	40	/* Model value at (trough) left bounds */
#define POS_ZT0	41	/* Model value at minimum (trough) */
#define POS_ZTR	42	/* Model value at (trough) right bounds */
#define POS_XBL	43	/* Longitude of model minimum (blend) left bounds */
#define POS_XB0	44	/* Longitude of model minimum (blend) */
#define POS_XBR	45	/* Longitude of model minimum (blend) right bounds */
#define POS_YBL	46	/* Latitude of model minimum (blend) left bounds */
#define POS_YB0	47	/* Latitude of model minimum (blend) */
#define POS_YBR	48	/* Latitude of model minimum (blend) right bounds */
#define POS_ZBL	49	/* Model value at (blend) left bounds */
#define POS_ZB0	50	/* Model value at minimum (blend) */
#define POS_ZBR	51	/* Model value at (blend) right bounds */
#define POS_XEL	52	/* Longitude of model max slope (blend) left bounds */
#define POS_XE0	53	/* Longitude of model max slope (blend) */
#define POS_XER	54	/* Longitude of model max slope (blend) right bounds */
#define POS_YEL	55	/* Latitude of model max slope (blend) left bounds */
#define POS_YE0	56	/* Latitude of model max slope (blend) */
#define POS_YER	57	/* Latitude of model max slope (blend) right bounds */
#define POS_ZEL	58	/* Model value at max slope (blend) left bounds */
#define POS_ZE0	59	/* Model value at max slope (blend) */
#define POS_ZER	60	/* Model value at max slope (blend) right bounds */

#define FZ_PAC	0	/* Array index for "Pacific" (isostatic edge dipole) model */
#define FZ_ATL	1	/* Array index for "Atlantic" (isostatic symmetric trough) model */
#define FZ_EMP	2	/* Array index for empirical model */
#endif /* _FZ_ANALYSIS_H */
