#	$Id: Makefile,v 1.22 2007-06-29 18:40:30 remko Exp $
#	makefile for misc directory
#

#----------------- stop here - nothing to edit--------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 
CDF		= -L$(NETCDF)/lib -lnetcdf

MISC_H		=

MISC_C		= gmtdigitize.c gmtstitch.c nc2xy.c psmegaplot.c
MISCPS_C	= makepattern.c

#-------------------------------------------------------------------------------
#       object file dependencies
#-------------------------------------------------------------------------------

MISC_O		= ${MISC_C:.c=.o}

MISC		= ${MISC_C:.c=}

MISCPS_O	= ${MISCPS_C:.c=.o}

MISCPS		= ${MISCPS_C:.c=}

all:            $(MISC) $(MISCPS)

$(MISC_O):	$(GMT_H) $(MISC_H)

$(MISCPS_O):	$(GMT_H) $(PS_H) $(MISC_H)

#-------------------------------------------------------------------------------

install:	all
		for i in $(MISC); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

uninstall:
		for i in $(MISC); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done

spotless:	clean

clean:	
		rm -f *.o
		for i in $(MISC); do \
			\rm -f $$i$(EXE); \
		done

#-------------------------------------------------------------------------------
#       program dependencies
#-------------------------------------------------------------------------------

$(MISC):	$(MISC_O) $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@

$(MISCPS):	$(MISCPS_O) $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<
