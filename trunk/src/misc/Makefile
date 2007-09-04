#	$Id: Makefile,v 1.23 2007-09-04 15:22:27 remko Exp $
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

MISC_O		= $(MISC_C:.c=.o)
MISC		= $(MISC_C:.c=$(EXE))
MISCPS_O	= $(MISCPS_C:.c=.o)
MISCPS		= $(MISCPS_C:.c=$(EXE))

all:            $(MISC) $(MISCPS)

$(MISC_O):	$(GMT_H) $(MISC_H)
$(MISCPS_O):	$(GMT_H) $(PS_H) $(MISC_H)

#-------------------------------------------------------------------------------

install:	all
		$(INSTALL) $(MISC) $(bindir)

uninstall:
		for i in $(MISC); do \
			\rm -f $(bindir)/$$i; \
		done

spotless:	clean

clean:	
		rm -f *.o $(MISC)

#-------------------------------------------------------------------------------
#       program dependencies
#-------------------------------------------------------------------------------

gmtdigitize$(EXE):	gmtdigitize.o
gmtstitch$(EXE):	gmtstitch.o
nc2xy$(EXE):		nc2xy.o
psmegaplot$(EXE):	psmegaplot.o
makepattern$(EXE):	makepattern.o

$(MISC):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@

$(MISCPS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@
