#	$Id: Makefile,v 1.41 2007-09-24 00:38:02 remko Exp $
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
#	Date:	1-MAR-2006
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf

PROGS_H		= mgd77.h mgd77defaults.h mgd77_IGF_coeffs.h mgd77sniffer.h mgd77snifferdefaults.h mgd77magref.h mgd77_codes.h
PROGS_C		= mgd77convert.c mgd77info.c mgd77list.c mgd77path.c mgd77manage.c mgd77sniffer.c 
PROGSPS_C	= mgd77track.c

LIB_C	= mgd77.c
LIB_H	= mgd77.h mgd77_functions.c mgd77_functions.h mgd77defaults.h
LIB_O	= $(LIB_C:.c=.o)
LIB	= libmgd77.a

PROGS_O		= $(PROGS_C:.c=.o)
PROGS		= $(PROGS_C:.c=$(EXE))
PROGSPS_O	= $(PROGSPS_C:.c=.o)
PROGSPS		= $(PROGSPS_C:.c=$(EXE))
MAN1		= $(PROGS_C:.c=.1) $(PROGSPS_C:.c=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(LIB) $(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -m 444 $(LIB_H) $(includedir); \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -m 444 $(LIB) $(libdir); \
			if [ -f $(LIB:.a=.$(SL)) ]; then \
				$(INSTALL) -m 444 $(LIB:.a=.$(SL)) $(libdir); \
			fi; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir); \rm -f $(LIB_H)
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f $(LIB) $(LIB:.a=.$(SL))
			\rm -f $(libdir)/libmgd77.a $(libdir)/libmgd77.$(SL); \
		fi

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		rm -f *.o *% $(PROGS) $(PROGSPS)

spotless:	clean
		rm -f $(LIB)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(PROGS_H) $(GMT_H)
$(PROGSPS_O):	$(PROGS_H) $(GMT_H) $(PS_H)
$(LIB_O):	$(LIB_H)


#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

$(LIB):		$(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------
mgd77convert$(EXE):	mgd77convert.o
mgd77info$(EXE):	mgd77info.o
mgd77list$(EXE):	mgd77list.o
mgd77path$(EXE):	mgd77path.o
mgd77manage$(EXE):	mgd77manage.o
mgd77sniffer$(EXE):	mgd77sniffer.o
mgd77track$(EXE):	mgd77track.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lmgd77 -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lmgd77 -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
