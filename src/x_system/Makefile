#	$Id: Makefile,v 1.6 2002-03-14 16:52:16 pwessel Exp $
#
#		Makefile for GMT x_system supplements
#
#	The x_system supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory as well as the gmt_mgg
#	library in the src/mgg subdirectory (which must be installed first).
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	07-MAR-2000
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
include ../makegmt.macros
#

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../mgg -I$(NETCDF)/include
ALLLIB  = -L../mgg -lgmt_mgg -L.. -L$(libdir) -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

ALL_C	= x_edit.c x_init.c x_list.c x_over.c x_report.c x_remove.c x_setup.c x_solve_dc_drift.c x_update.c

ALL_O	= ${ALL_C:.c=.o}

ALL	= ${ALL_C:.c=}

all:	$(ALL)

#-------------------------------------------------------------------------------

install:	all
		for i in $(ALL); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

uninstall:
		for i in $(ALL); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done

clean:
		rm -f *.o
		for i in $(ALL); do \
			rm -f $$i$(EXE); \
		done

spotless:	clean

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(ALL):		$(ALL_O)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o $(ALLLIB) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<

$(ALL_O):	x_system.h
