#	$Id: Makefile,v 1.1 2001-03-02 05:54:44 pwessel Exp $
#
#		Makefile for GMT mgg supplements
#
#	The mgg supplements are assumed to be installed in a subdirectory
#	under the main gmt/src directory and will refer to the gmt libraries
#	and makefile macros in the parent directory.
#	First step incolves moving the file gmtfile_paths to the directory
#	pointed to by the environmental variable $GMTHOME (see GMT README).
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, NOAA
#
#	Date:	07-FEB-2000
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
include ../makegmt.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I.. -I$(NETCDF)/include
CDF	= -L$(NETCDF)/lib -lnetcdf

MGG_H	= gmt_mgg.h

MGG_C	= binlegs.c gmt2bin.c gmt2dat.c dat2gmt.c gmtinfo.c gmtlegs.c \
	 	gmtlist.c gmtpath.c gmttrack.c mgd77togmt.c

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

MGG_O	= ${MGG_C:.c=.o}

MGG	= ${MGG_C:.c=}

all:		libgmt_mgg.a $(MGG)

$(MGG_O):	$(GMT_H) $(MGG_H)

#-------------------------------------------------------------------------------

install:	all
		for i in $(MGG); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

uninstall:
		for i in $(MGG); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done

clean:
		rm -f *.o
		for i in $(MGG); do \
			rm -f $$i$(EXE); \
		done

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

$(MGG):		libgmt_mgg.a $(MGG_O)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L. -lgmt_mgg -L.. -lpsl -lgmt $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<
