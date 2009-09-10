#	$Id: Makefile,v 1.25 2009-09-10 16:02:05 remko Exp $
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

INCLUDES	= -I$(srcdir) -I../mgg $(NETCDF_INC) $(GDAL_INC)

PROGS_H		= x_system.h
PROGS_O		= x_edit.o x_init.o x_list.o x_report.o x_remove.o x_solve_dc_drift.o x_update.o
PROGSPS_O	= x_setup.o x_over.o
MAN1		= $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1) x_system.1

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		\rm -f *.o *% $(PROGS) $(PROGSPS)

spotless::	clean

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(PROGS_H)
$(PROGSPS_O):	$(GMT_H) $(PROGS_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

x_edit$(EXE):		x_edit.o
x_init$(EXE):		x_init.o
x_list$(EXE):		x_list.o
x_over$(EXE):		x_over.o
x_report$(EXE):		x_report.o
x_remove$(EXE):		x_remove.o
x_setup$(EXE):		x_setup.o
x_solve_dc_drift$(EXE):	x_solve_dc_drift.o
x_update$(EXE):		x_update.o

$(PROGS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L../mgg -lgmt_mgg -L.. -lgmt $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
