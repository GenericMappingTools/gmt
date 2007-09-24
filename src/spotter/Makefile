#	$Id: Makefile,v 1.21 2007-09-24 00:38:02 remko Exp $
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
#	Date:	02-FEB-2006
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf

PROGS_H	= spotter.h
PROGS_C	= backtracker.c grdrotater.c hotspotter.c originator.c rotconverter.c
PROGS_O	= $(PROGS_C:.c=.o)
PROGS	= $(PROGS_C:.c=$(EXE))
MAN1	= $(PROGS_C:.c=.1)

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

spotless:	clean
		\rm -f $(LIB)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)

libspotter.o:	$(PROGS_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

$(LIB):		libspotter.o
		$(AR) cvr $(LIB) $?
		$(RANLIB) $(LIB)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

backtracker$(EXE):	backtracker.o
grdrotater$(EXE):	grdrotater.o
hotspotter$(EXE):	horspotter.o
originator$(EXE):	originator.o
rotconverter$(EXE):	rotconverter.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lspotter -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
