#	$Id: Makefile,v 1.6 2005-02-15 21:15:18 pwessel Exp $
#	makefile for misc directory
#

#----------------- stop here - nothing to edit--------------------

include ../makegmt.macros

CDF		= -L$(NETCDF)/lib -lnetcdf

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 

MISC_H		=

MISC_C		= psmegaplot.c makepattern.c

#-------------------------------------------------------------------------------
#       object file dependencies
#-------------------------------------------------------------------------------

MISC_O		= ${MISC_C:.c=.o}

MISC		= ${MISC_C:.c=}

all:            $(MISC)

$(MISC_O):	$(GMT_H) $(MISC_H)

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

$(MISC):	$(MISC_O)
		$(CC) $(CFLAGS) $(LDFLAGS) $@.o -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@

.c.o:
		$(CC) -c $(CFLAGS) $<
