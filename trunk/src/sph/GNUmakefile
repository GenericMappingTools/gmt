#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.12 2009-03-25 18:46:09 guru Exp $
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
#	Date:		12-MAR-2009
#-------------------------------------------------------------------------------

include makefile
include $(GMTSRCDIR)manpages.mk

toms:
		$(MAKE) -B stripack.c ssrfpack.c

stripack.c:
		wget http://www.netlib.org/toms/772
		mkdir -p D772
		mv 772 D772
		(cd D772; sh 772)
		f2c -r8 D772/Fortran77/Src/Sp/src.f
		mv -f src.c stripack.c
		rm -rf D772

ssrfpack.c:
		wget http://www.netlib.org/toms/773
		mkdir -p D773
		tail -n +4 773 > D773/773
		(cd D773; sh 773)
		f2c -r8 D773/Src/Sp/src.f
		sed -f ssrfpack.sed src.c > ssrfpack.c
		rm -rf src.c 773 D773

spotless::
		rm -rf D772 D773 sph_ex_?.pdf
