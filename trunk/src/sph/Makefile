#	$Id: Makefile,v 1.1.1.1 2008-02-19 05:30:04 myself Exp $
#
#		Makefile for GMT supplement sph
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	17-FEB-2007
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

F2C	= /sw
CFLAGS	= $(CC_OPT) $(WIN32) -I$(GMTHOME)/include -I$(NETCDF)/include -I$(F2C)/include
CDF	= -L$(NETCDF)/lib -lnetcdf
F2CLIB	= -L$(F2C)/lib -lf2c

CODE_C	= sphtriangulate.c sphdistance.c
MAN1		= $(CODE_C:.c=.1)

STRLIB		= libstripack.a
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

test1:
		sphtriangulate testdata.txt -Qv -A > testresults.txt
		psxy -Rg -JG30/30/6i -M -L -W1.5p,red testresults.txt -B0:."Test of 32 points": -K -P > testresults.ps
		psxy testdata.txt -R -J -O -Sc0.02i -Gblack >> testresults.ps
		gv testresults.ps &

test2:
		sphtriangulate tritest.txt > tritestres.txt
		psxy -Rg -JG40/25/6i -M -L -W0.25p tritestres.txt -B0:."Test of 15 points": -K -P > tritestres.ps
		psxy tritest.txt -R -J -O -Sc0.02i -Gblack >> tritestres.ps
		gv tritestres.ps &


#-------------------------------------------------------------------------------
#	STRLIB library
#-------------------------------------------------------------------------------

library:	$(STRLIB)

$(STRLIB):	stripack.o
#		g95 -c -O stripack.f90
		$(AR) cvr libstripack.a stripack.o
		$(RANLIB) libstripack.a

stripack.o:	stripack.c

#-------------------------------------------------------------------------------
#       sph library
#-------------------------------------------------------------------------------

$(SPHLIB):	sph.o
	$(AR) cvr $@ $?
	$(RANLIB) $@

spotless::	clean
#	rm -rf libstripack.a stripack.f90 772 stripack.c Fortran77
	
#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(CODE):	$(CODE_O) libstripack.a libsph.a
	$(CC) $(CFLAGS) $@.o -L$(GMTHOME)/lib -lgmt -lpsl $(CDF) -L. -lstripack -lsph $(F2CLIB) $(LIBS) -o $@
#	g95 -g $@.o -L$(GMTHOME)/lib -lgmt -lpsl $(CDF) -L. -lstripack $(LIBS) -o $@
