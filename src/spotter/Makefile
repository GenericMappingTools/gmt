#	$Id: Makefile,v 1.2 2001-03-05 15:50:44 pwessel Exp $
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
#	Date:	07-FEB-2000
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
-include ../makegmt.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I.. -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf

SPOT_H	= spotter.h

SPOT_C	= backtracker.c hotspotter.c originator.c

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

SPOT_O	= ${SPOT_C:.c=.o}

SPOT	= ${SPOT_C:.c=}

all:		libspotter.a $(SPOT)

$(SPOT_O):	$(GMT_H) $(SPOT_H)

#-------------------------------------------------------------------------------

install:	all
		for i in $(SPOT); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

uninstall:
		for i in $(SPOT); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done

clean:
		rm -f *.o
		for i in $(SPOT); do \
			rm -f $$i$(EXE); \
		done

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

$(SPOT):	libspotter.a $(SPOT_O)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lspotter -L.. -lpsl -lgmt $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<
