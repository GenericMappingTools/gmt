#	$Id: Makefile,v 1.58 2009-09-10 02:21:36 guru Exp $
#
#		Makefile for GMT mgd77 supplements
#
#	The mgd77 supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Authors:	Paul Wessel & Mike Chandler, SOEST, U. of Hawaii
#
#	Date:	20-JUN-2008
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

INCLUDES	= -I$(srcdir) -I$(NETCDF_INC) -I$(GDAL_INC)

LIB_O		= mgd77.o
LIB_H		= mgd77.h mgd77_functions.h mgd77defaults.h mgd77_recalc.h cm4_functions.h
LIB		= libmgd77.$(LIBEXT)

PROGS_H		= mgd77_IGF_coeffs.h mgd77sniffer.h mgd77snifferdefaults.h mgd77magref.h mgd77_codes.h
PROGS_O		= mgd77convert.o mgd77info.o mgd77list.o mgd77path.o mgd77manage.o mgd77sniffer.o mgd77magref.o
PROGSPS_O	= mgd77track.o
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
			\mv -f libmgd77.{$(SL),$(SL_VERSION)} ; \
			$(LN_S) libmgd77.{$(SL_VERSION),$(SL)} ; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir); \rm -f $(LIB_H);\
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f libmgd77.{a,$(SL),$(SL_VERSION)}; \
		fi

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		\rm -f *% *.o *.a *.$(SL) $(PROGS) $(PROGSPS)

spotless::	clean

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(LIB_H) $(PROGS_H) $(GMT_H)
$(PROGSPS_O):	$(LIB_H) $(PROGS_H) $(GMT_H) $(PS_H)
$(LIB_O):	$(LIB_H) $(GMT_H) cm4_functions.c

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libmgd77.a:	$(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libmgd77.$(SL):	libmgd77.a
		$(LD) $(LD_OPT) $(LIB_O) -L.. -lgmt $(NC_LIBS) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------
mgd77convert$(EXE):	mgd77convert.o
mgd77info$(EXE):	mgd77info.o
mgd77list$(EXE):	mgd77list.o
mgd77path$(EXE):	mgd77path.o
mgd77manage$(EXE):	mgd77manage.o
mgd77sniffer$(EXE):	mgd77sniffer.o
mgd77magref$(EXE):	mgd77magref.o
mgd77track$(EXE):	mgd77track.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lmgd77 -L.. -lgmt $(NC_LIBS) $(GDAL_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lmgd77 -L.. -lgmt -lgmtps -lpsl $(NC_LIBS) $(GDAL_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@
