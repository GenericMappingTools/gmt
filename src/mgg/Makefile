#	$Id: Makefile,v 1.29 2009-08-17 17:27:29 remko Exp $
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

INCLUDES	= -I$(srcdir) -I../x_system -I$(NETCDF_INC)

LIB_O		= gmt_mgg.o
LIB_H		= gmt_mgg.h carter.h
LIB		= libgmt_mgg.$(LIBEXT)

PROGS_H		= ../x_system/x_system.h
PROGS_O		= binlegs.o gmt2bin.o gmt2dat.o dat2gmt.o gmtinfo.o gmtlegs.o gmtlist.o gmtpath.o mgd77togmt.o
PROGSPS_O	= gmttrack.o
MAN1		= $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(LIB) $(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -d $(includedir); \
			$(INSTALL) -m 644 $(LIB_H) $(includedir); \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -d $(libdir); \
			$(INSTALL) -m 644 $(LIB:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) -m 755 $(LIB) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f libgmt_mgg.{$(SL),$(SL_VERSION)} ; \
			$(LN_S) libgmt_mgg.{$(SL_VERSION),$(SL)} ; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir); \rm -f $(LIB_H);\
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f libgmt_mgg.{a,$(SL),$(SL_VERSION)}; \
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

$(PROGS_O):	$(LIB_H) $(PROGS_H) $(GMT_H)
$(PROGSPS_O):	$(LIB_H) $(PROGS_H) $(GMT_H) $(PS_H)
$(LIB_O):	$(LIB_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libgmt_mgg.a:	$(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_mgg.$(SL):	libgmt_mgg.a
		$(LD) $(LD_OPT) $(LIB_O) -L.. -lgmt $(NC_LIBS) $(LIBS) -o $@

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
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgg -L.. -lgmt $(NC_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgg -L.. -lgmt -lgmtps -lpsl $(NC_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@
