#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.3 2008-02-20 01:11:45 myself Exp $
#
#	Guru Makefile for GMT Version 4.x sph directory
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular Makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		17-FEB-2008
#-------------------------------------------------------------------------------

include makefile
include $(GMTSRCDIR)manpages.mk

Fortran77/Src/Sp/src.f:	772

772:
		wget http://www.netlib.org/toms/772
		sh 772

stripack.c:	Fortran77/Src/Sp/src.f
	f2c -r8 Fortran77/Src/Sp/src.f
	mv -f src.c stripack.c
