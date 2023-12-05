#!/usr/bin/env bash
#--------------------------------------------------------------------
# Copyright (c) 2015-2023 by P. Wessel
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# Contact info: http://www.soest.hawaii.edu/PT/GSFML
#--------------------------------------------------------------------
# Functions used by GSFML scripts
# These do not require GMT; they do basic stuff with Unix tools only
#
# Author:	Paul Wessel
# Date:		01-DEC-2023
# Mode:		GMT classic mode
#--------------------------------------------------------------------

function fz_get_version
{	# Just echoes the current GSFML version
	echo "2.0 [2023]"
}

function fz_cleanup() {
	rm -f *$$*
	exit $1
}

function fz_get_dim
{	# Expects a dimension with trailing unit, e,g, 15c, and returns inches.  Note we skip the first 2 chars which are option flag, e.g. -W<width>[c|i]
	echo $1 | awk '{if (substr($1,length($1),1) == "c") {print substr($1,3,length($1)-3)/2.54} else if (substr($1,length($1),1) == "i") {print substr($1,3,length($1)-3)} else {print substr($1,3)}}'
}

function fz_get_arg
{	# Expects a arg.  Note we skip the first 2 chars which are option flag, e.g. -W<width>
	echo $1 | awk '{print substr($1,3)}'
}

function fz_get_item
{	# Expects a two-char name as arg2 (e.g., az or VB) and pulls out value from header record arg1
	# Must make sure arg1 is passed as one item (i.e., in double quotes)
	echo $1 | tr ' ' '\n' | awk '{ if (substr($1,1,2) == "'$2'") print substr($1,4)}'
}

function fz_col_id
{	# Returns the column number 0-60 given the tag
	case $1 in
		XR  ) col=0  ;;
		YR  ) col=1  ;;
		DR  ) col=2  ;;
		AR  ) col=3  ;;
		ZR  ) col=4  ;;
		TL  ) col=5  ;;
		TR  ) col=6  ;;
		SD  ) col=7  ;;
		ST  ) col=8  ;;
		SB  ) col=9  ;;
		SE  ) col=10 ;;
		BL  ) col=11 ;;
		OR  ) col=12 ;;
		WD  ) col=13 ;;
		WT  ) col=14 ;;
		WB  ) col=15 ;;
		AD  ) col=16 ;;
		AT  ) col=17 ;;
		AB  ) col=18 ;;
		UT  ) col=19 ;;
		UB  ) col=20 ;;
		VT  ) col=21 ;;
		VB  ) col=22 ;;
		FT  ) col=23 ;;
		FB  ) col=24 ;;
		XDL ) col=25 ;;
		XD0 ) col=26 ;;
		XDR ) col=27 ;;
		YDL ) col=28 ;;
		YD0 ) col=29 ;;
		YDR ) col=30 ;;
		ZDL ) col=31 ;;
		ZD0 ) col=32 ;;
		ZDR ) col=33 ;;
		XTL ) col=34 ;;
		XT0 ) col=35 ;;
		XTR ) col=36 ;;
		YTL ) col=37 ;;
		YT0 ) col=38 ;;
		YTR ) col=39 ;;
		ZTL ) col=40 ;;
		ZT0 ) col=41 ;;
		ZTR ) col=42 ;;
		XBL ) col=43 ;;
		XB0 ) col=44 ;;
		XBR ) col=45 ;;
		YBL ) col=46 ;;
		YB0 ) col=47 ;;
		YBR ) col=48 ;;
		ZBL ) col=49 ;;
		ZB0 ) col=50 ;;
		ZBR ) col=51 ;;
		XEL ) col=52 ;;
		XE0 ) col=53 ;;
		XER ) col=54 ;;
		YEL ) col=55 ;;
		YE0 ) col=56 ;;
		YER ) col=57 ;;
		ZEL ) col=58 ;;
		ZE0 ) col=59 ;;
		ZER ) col=60 ;;
		* ) echo "Bad tag in fz_col"; col=0 ;;
	esac
	echo $col
}
