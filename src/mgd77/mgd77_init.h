/*---------------------------------------------------------------------------
 *
 *    Copyright (c) 2005-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    See README file for copying and redistribution conditions.
 *
 *  File:	mgd77_init.h
 *
 *  Include file for mgd77.c
 *
 *  Authors:    Paul Wessel, Primary Investigator, SOEST, U. of Hawaii
 *		Michael Chandler, Affiliate Researcher, SOEST, U. of Hawaii
 *
 *  This include file contains initializations for the MGD77 system.
 *  MUST BE INCLUDED AFTER mgd77.h IN mgd77.c
 *
 *  Version:	1.1
 *  Revised:	1-JAN-2006
 *
 *-------------------------------------------------------------------------*/

/*!
 * \file mgd77_init.h
 * \brief Include file for mgd77.c
 */

bool MGD77_format_allowed[MGD77_N_FORMATS] = {true, true, true, true};	/* By default we allow opening of files in any format.  See MGD77_Ignore_Format() */

char *MGD77_suffix[MGD77_N_FORMATS] = {"nc", "m77t", "mgd77", "dat"};
