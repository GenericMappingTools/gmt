#	$Id: Makefile,v 1.23 2007-10-07 07:10:41 guru Exp $
#
#		Makefile for GMT mgg supplements
#
#	The mgg supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, NOAA
#
#	Date:	05-NOV-2006
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I../x_system -I$(NETCDF)/include

PROGS_H		= gmt_mgg.h ../x_system/x_system.h
PROGS_O		= binlegs.o gmt2bin.o gmt2dat.o dat2gmt.o gmtinfo.o gmtlegs.o gmtlist.o gmtpath.o mgd77togmt.o
PROGSPS_O	= gmttrack.o
MAN1		= $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)

LIB		= libgmt_mgg.a

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(LIB) $(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -d $(libdir); \
			$(INSTALL) -m 444 $(LIB) $(libdir); \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rf -f $(LIB)
		fi

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		\rm -f *.o *% $(PROGS) $(PROGSPS)

spotless::	clean
		\rm -f $(LIB)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)
$(PROGSPS_O):	$(GMT_H) $(PS_H) $(PROGS_H)
gmt_mgg.o:	$(PROGS_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

$(LIB):		gmt_mgg.o
		$(AR) cvr $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

binlegs$(EXE):		binlegs.o
gmt2bin$(EXE):		gmt2bin.o
gmt2dat$(EXE):		gmt2dat.o
dat2gmt$(EXE):		dat2gmt.o
gmtinfo$(EXE):		gmtinfo.o
gmtlegs$(EXE):		gmtlegs.o
gmtlist$(EXE):		gmtlist.o
gmtpath$(EXE):		gmtpath.o
mgd77togmt$(EXE):	mgd77togmt.o
gmttrack$(EXE):		gmttrack.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgg -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgg -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
