#	$Id: Makefile,v 1.20 2007-09-04 15:22:27 remko Exp $
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

SPOT_H	= spotter.h
SPOT_C	= backtracker.c grdrotater.c hotspotter.c originator.c rotconverter.c

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

SPOT_O	= $(SPOT_C:.c=.o)
SPOT	= $(SPOT_C:.c=$(EXE))

all:		libspotter.a $(SPOT)

$(SPOT_O):	$(GMT_H) $(SPOT_H)

#-------------------------------------------------------------------------------

install:	all
		$(INSTALL) $(SPOT) $(bindir)

uninstall:
		for i in $(SPOT); do \
			\rm -f $(bindir)/$$i; \
		done

clean:
		rm -f *.o $(SPOT)

spotless:	clean
		rm -f *.a

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

libspotter.a:	libspotter.o
		$(AR) cvr libspotter.a $?
		$(RANLIB) libspotter.a

libspotter.o:	$(SPOT_H)

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(SPOT):	libspotter.a $(SPOT_O) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lspotter -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
