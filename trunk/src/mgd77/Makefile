#	$Id: Makefile,v 1.32 2006-12-06 18:13:51 remko Exp $
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

MGD77_H	= mgd77.h mgd77defaults.h mgd77_IGF_coeffs.h mgd77sniffer.h mgd77snifferdefaults.h mgd77magref.h mgd77_codes.h

MGD77_C	= mgd77convert.c mgd77info.c mgd77list.c mgd77path.c mgd77manage.c mgd77sniffer.c

MGD77PS_C	= mgd77track.c

LIB_C	= mgd77.c
LIB_H	= mgd77.h mgd77_functions.h mgd77defaults.h
LIB_O	= $(LIB_C:.c=.o)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

MGD77_O		= $(MGD77_C:.c=.o)

MGD77		= $(MGD77_C:.c=)

MGD77PS_O	= $(MGD77PS_C:.c=.o)

MGD77PS		= $(MGD77PS_C:.c=)

all:		libmgd77.a $(MGD77) $(MGD77PS)
$(MGD77_O):	$(MGD77_H) $(GMT_H)
$(MGD77PS_O):	$(MGD77_H) $(GMT_H) $(PS_H)
$(LIB_O):	$(LIB_H)

#-------------------------------------------------------------------------------

install:	all
		for i in $(MGD77) $(MGD77PS); do \
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
		for i in $(MGD77) $(MGD77PS); do \
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
		for i in $(MGD77) $(MGD77PS); do \
			rm -f $$i$(EXE); \
		done

spotless:	clean
		rm -f *.a

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

libs:		libmgd77.a

libmgd77.a:	$(LIB_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(MGD77):	$(MGD77_O) libmgd77.a $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lmgd77 -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@

$(MGD77PS):	$(MGD77PS_O) libmgd77.a $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lmgd77 -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<

#---------------------------- CVS GURU EXTENSIONS-------------------------------
sinclude makefile.guru
