#	$Id: Makefile,v 1.16 2007-09-04 15:22:27 remko Exp $
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

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../mgg -I$(NETCDF)/include
ALLLIB  = -L../mgg -lgmt_mgg -L.. -L$(libdir) -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

ALL_C	= x_edit.c x_init.c x_list.c x_over.c x_report.c x_remove.c x_setup.c x_solve_dc_drift.c x_update.c
ALL_O	= $(ALL_C:.c=.o)
ALL	= $(ALL_C:.c=$(EXE))

all:	$(ALL)

#-------------------------------------------------------------------------------

install:	all
		$(INSTALL) $(ALL) $(bindir)

uninstall:
		for i in $(ALL); do \
			\rm -f $(bindir)/$$i; \
		done

clean:
		rm -f *.o $(ALL)

spotless:	clean

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(ALL):		$(ALL_O) $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) $(@:.exe=).o $(ALLLIB) -o $@

$(ALL_O):	x_system.h $(GMT_H)
