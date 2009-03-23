#	$Id: Makefile,v 1.7 2009-03-23 21:12:11 myself Exp $
#
#		Makefile for GMT supplement sph
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	12-MAR-2009
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

F2C	= /sw
INCLUDES = -I$(GMTHOME)/include -I$(NETCDF_INC) -I$(F2C)/include
F2CLIB	= -L$(F2C)/lib -lf2c

CODE_C		= sphtriangulate.c sphdistance.c sphinterpolate.c
MAN1		= $(CODE_C:.c=.1)
SCRIPTS		= sph_ex_1.sh sph_ex_2.sh sph_ex_3.sh sph_ex_4.sh
STRILIB		= libstripack.a
SSRFLIB		= libssrfpack.a
SPHLIB		= libsph.a
#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

CODE_O	= ${CODE_C:.c=.o}

CODE	= ${CODE_C:.c=}

all:		$(CODE)

#-------------------------------------------------------------------------------

install:	all
		for i in $(CODE); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		rm -f *.o
		for i in $(CODE); do \
			rm -f $$i$(EXE); \
		done

ex:		$(SCRIPTS)
		for i in $(SCRIPTS); do \
			sh $$i; \
		done
		
#-------------------------------------------------------------------------------
#	STRILIB library
#-------------------------------------------------------------------------------

library:	$(STRILIB) $(SSRFLIB)

$(STRILIB):	stripack.o
#		g95 -c -O stripack.f90
		$(AR) cvr libstripack.a stripack.o
		$(RANLIB) libstripack.a

stripack.o:	stripack.c

#-------------------------------------------------------------------------------
#	SSRFLIB library
#-------------------------------------------------------------------------------

$(SSRFLIB):	ssrfpack.o
		$(AR) cvr libssrfpack.a ssrfpack.o
		$(RANLIB) libssrfpack.a

ssrfpack.o:	ssrfpack.c

#-------------------------------------------------------------------------------
#       sph library
#-------------------------------------------------------------------------------

$(SPHLIB):	sph.o
	$(AR) cvr $@ $?
	$(RANLIB) $@

sph.o:	sph.c sph.h $(GMT_H)

spotless::	clean
	rm -rf $(STRILIB) $(SSRFLIB) $(SPHLIB)
	
#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(CODE):	$(CODE_O) $(STRILIB) $(SSRFLIB) $(SPHLIB)
	$(CC) $(CFLAGS) $@.o -L$(GMTHOME)/lib -lgmt -lpsl $(NC_LIBS) -L. -lssrfpack  -lstripack -lsph $(F2CLIB) $(LIBS) -o $@
