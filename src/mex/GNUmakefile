#-------------------------------------------------------------------------------
#  $Id$
#
#	Guru Makefile for GMT Version 5 mex directory
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular Makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		1-JAN-2011
#-------------------------------------------------------------------------------

include Makefile
include $(GMTSRCDIR)manpages.mk

# GPROGS_C are built on the fly via sed from grd2grd.c
GPROGS_C	= grdclip.c grdcut.c grdedit.c grdfft.c grdfilter.c grdmath.c grdpaste.c grdproject.c grdsample.c grdtrend.c

$(GPROGS_C):	grd2grd.c
		sed -e "s/FUNC_MODE/GMTAPI_GMT/g" -e "s/FUNC/GMT_$*_cmd/g" grd2grd.c > $*.c

spotless::
		rm -f $(GPROGS_C)
