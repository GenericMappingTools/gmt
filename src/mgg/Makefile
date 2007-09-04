#	$Id: Makefile,v 1.19 2007-09-04 15:22:27 remko Exp $
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

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../x_system -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf

MGG_H	= gmt_mgg.h ../x_system/x_system.h

MGG_C	= binlegs.c gmt2bin.c gmt2dat.c dat2gmt.c gmtinfo.c gmtlegs.c \
	 	gmtlist.c gmtpath.c mgd77togmt.c

MGGPS_C	= gmttrack.c

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

MGG_O	= $(MGG_C:.c=.o)
MGG	= $(MGG_C:.c=$(EXE))
MGGPS_O	= $(MGGPS_C:.c=.o)
MGGPS	= $(MGGPS_C:.c=$(EXE))

all:		libgmt_mgg.a $(MGG) $(MGGPS)

$(MGG_O):	$(GMT_H) $(MGG_H)
$(MGGPS_O):	$(GMT_H) $(PS_H) $(MGG_H)

#-------------------------------------------------------------------------------

install:	all
		$(INSTALL) $(MGG) $(MGGPS) $(bindir)
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -m 444 libgmt_mgg.a $(libdir); \
			if [ -f libgmt_mgg.$(SL) ]; then \
				$(INSTALL) -m 444 libgmt_mgg.$(SL) $(libdir); \
			fi; \
		fi

uninstall:
		for i in $(MGG) $(MGGPS); do \
			\rm -f $(bindir)/$$i; \
		done
		if [ ! $(libdir) = $(srcdir) ]; then \
			\rm -f $(libdir)/libgmt_mgg.a $(libdir)/libgmt_mgg.$(SL); \
		fi

clean:
		rm -f *.o $(MGG) $(MGGPS)

spotless:	clean
		rm -f *.a

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

libgmt_mgg.a:	gmt_mgg.o
		$(AR) cvr libgmt_mgg.a $?
		$(RANLIB) libgmt_mgg.a

gmt_mgg.o:	$(MGG_H)

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(MGG):		libgmt_mgg.a $(MGG_O) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgg -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@

$(MGGPS):	libgmt_mgg.a $(MGGPS_O) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_mgg -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@
