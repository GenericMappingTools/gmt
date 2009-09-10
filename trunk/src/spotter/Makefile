#	$Id: Makefile,v 1.32 2009-09-10 16:02:06 remko Exp $
#
#		Makefile for GMT spotter supplements
#
#	The spotter supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	18-OCT-2007
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

PROGS_H		= spotter.h
PROGS_O		= backtracker.o grdrotater.o grdspotter.o hotspotter.o originator.o rotconverter.o
MAN1		= $(PROGS_O:.o=.1)
LIB		= libspotter.a

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		libspotter.a $(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		\rm -f *.o *% $(PROGS)

spotless::	clean
		\rm -f $(LIB)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

libspotter.o:	$(PROGS_H) $(GMT_H)
$(PROGS_O):	$(GMT_H) $(PROGS_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

$(LIB):		libspotter.o
		$(AR) cvr $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

backtracker$(EXE):	backtracker.o
grdrotater$(EXE):	grdrotater.o
grdspotter$(EXE):	grdspotter.o
hotspotter$(EXE):	hotspotter.o
originator$(EXE):	originator.o
rotconverter$(EXE):	rotconverter.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lspotter -L.. -lgmt $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
