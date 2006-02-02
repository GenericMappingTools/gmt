#	$Id: GNUmakefile,v 1.4 2006-02-02 00:17:14 pwessel Exp $
#
#		GNU Makefile for GMT mgd77 supplements
#
#	The mgd77 supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Authors:	Paul Wessel & Mike Chandler, SOEST, U. of Hawaii
#
#	Date:	14-OCT-2005
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
include ../makegmt.macros

CFLAGS	= $(CC_OPT) -Wall $(WIN32) -I$(srcdir) -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf

MGD77_H	= mgd77.h mgd77defaults.h mgd77_IGF_coeffs.h mgd77_codes.h

MGD77_C	= mgd77convert.c mgd77info.c mgd77list.c mgd77path.c mgd77manage.c mgd77track.c

LIB_C	= mgd77.c
LIB_H	= mgd77.h mgd77_functions.h mgd77defaults.h
LIB_O	= $(LIB_C:.c=.o)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

MGD77_O	= $(MGD77_C:.c=.o)

MGD77	= $(MGD77_C:.c=)

$(MGD77_O):	$(MGD77_H)
$(LIB_O):	$(LIB_H)

#-------------------------------------------------------------------------------

all:		libmgd77.a $(MGD77)

install:	all
		for i in $(MGD77); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done
		if [ ! -d $(includedir) ]; then \
			mkdir -p $(includedir); \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			for i in $(LIB_H); do \
				$(INSTALL) -m 444 $$i $(includedir); \
			done; \
		fi
		if [ ! -d $(libdir) ]; then \
			mkdir -p $(libdir); \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -m 444 libmgd77.a $(libdir); \
			if [ -f libmgd77.$(SL) ]; then \
				$(INSTALL) -m 444 libmgd77.$(SL) $(libdir); \
			fi; \
		fi

uninstall:
		for i in $(MGD77); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done
		if [ ! $(includedir) = $(srcdir) ]; then \
			for i in $(LIB_H); do \
				\rm -f $(includedir)/$$i; \
			done; \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			\rm -f $(libdir)/libmgd77.a; \
			if [ -f libmgd77.$(SL) ]; then \
				\rm -f $(libdir)/libmgd77.$(SL); \
			fi; \
		fi

clean:
		rm -f *.o
		for i in $(MGD77); do \
			rm -f $$i$(EXE); \
		done

spotless:	clean
		rm -f *.a mgd77_functions.h

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

libs:		libmgd77.a

libmgd77.a:	mgd77.o
		$(AR) cvur $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(MGD77):	$(MGD77_O) libmgd77.a
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lmgd77 -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<

mgd77_functions.h:	mgd77netcdfhelper.sh mgd77.h
		$(SHELL) mgd77netcdfhelper.sh

mgd77_codes.h:		mgd77_codes.sh
		$(SHELL) mgd77_codes.sh

mgd77_codes.txt:	mgd77_codes.sh
		$(SHELL) mgd77_codes.sh
