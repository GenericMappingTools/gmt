#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.22 2011-07-19 22:12:03 guru Exp $
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
#	Date:		17-MAY-2009
#-------------------------------------------------------------------------------

include Makefile
include $(GMTSRCDIR)manpages.mk

# While f2c is no longer needed I have left these notes on how to make f2c from source
# for both 32- and 64-bit systems.

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
		stripack_prep.sh
		rm -rf src.c D772

ssrfpack.c:
		wget http://www.netlib.org/toms/773
		mkdir -p D773
		tail -n +4 773 > D773/773
		(cd D773; sh 773)
		ssrfpack_prep.sh
		rm -rf src.c 773 # D773

spotless::
		rm -rf D772 D773 f2c sph_ex_?.ps libf2c.zip*
