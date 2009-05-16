#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.18 2009-05-16 02:59:02 guru Exp $
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

getf2c:
		wget http://www.netlib.org/f2c/libf2c.zip
		mkdir f2c
		unzip libf2c.zip -d f2c
		
libf2c64:
		sed -f f2c64.sed f2c/makefile.u > f2c/makefile.u64
		$(MAKE) -C f2c -f makefile.u64 all
		@echo "Copy the 64-bit f2c/libf2c.a and f2c/f2c.h to where you need them"

libf2c:	
		sed -f f2c32.sed f2c/makefile.u > f2c/makefile.u32
		$(MAKE) -C f2c -f makefile.u32 all
		@echo "Copy the 32-bit f2c/libf2c.a and f2c/f2c.h to where you need them"

toms:
		$(MAKE) -B stripack.c ssrfpack.c

stripack.c:
		wget http://www.netlib.org/toms/772
		mkdir -p D772
		mv 772 D772
		(cd D772; sh 772)
		f2c -r8 D772/Fortran77/Src/Sp/src.f
		sed -f stripack.sed src.c > stripack.c
		rm -rf src.c D772

ssrfpack.c:
		wget http://www.netlib.org/toms/773
		mkdir -p D773
		tail -n +4 773 > D773/773
		(cd D773; sh 773)
		f2c -r8 D773/Src/Sp/src.f
		sed -f ssrfpack.sed src.c > ssrfpack.c
		rm -rf src.c 773 D773

stripack_nof2c.c:
		wget http://www.netlib.org/toms/772
		mkdir -p D772
		mv 772 D772
		(cd D772; sh 772)
		stripack_prep.sh
		rm -rf src.c D772

ssrfpack_nof2c.c:
		wget http://www.netlib.org/toms/773
		mkdir -p D773
		tail -n +4 773 > D773/773
		(cd D773; sh 773)
		ssrfpack_prep.sh
		rm -rf src.c 773 # D773

sphinterpolate2:	sphinterpolate2.o $(STRILIB2) $(SSRFLIB2) $(SPHLIB)
	$(CC) $(CFLAGS) $@.o -L.. -lgmt -lpsl $(NC_LIBS) -L. -lsph -lssrfpack2  -lstripack2 $(LIBS) -o $@

sphinterpolate2.o:	sphinterpolate.c
	$(CC) $(CFLAGS) -I.. sphinterpolate.c -c -o sphinterpolate2.o

#-------------------------------------------------------------------------------
#	STRILIB2 library
#-------------------------------------------------------------------------------
STRILIB2		= libstripack2.a
SSRFLIB2		= libssrfpack2.a

library2:	$(STRILIB2) $(SSRFLIB2)

$(STRILIB2):	stripack_nof2c.o
		$(AR) cvr libstripack2.a stripack_nof2c.o
		$(RANLIB) libstripack2.a

#-------------------------------------------------------------------------------
#	SSRFLIB2 library
#-------------------------------------------------------------------------------

$(SSRFLIB2):	ssrfpack_nof2c.o
		$(AR) cvr libssrfpack2.a ssrfpack_nof2c.o
		$(RANLIB) libssrfpack2.a
		
spotless::
		rm -rf D772 D773 f2c sph_ex_?.ps libf2c.zip*
