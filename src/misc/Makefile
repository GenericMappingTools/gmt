#	$Id: Makefile,v 1.24 2007-09-24 00:38:02 remko Exp $
#	makefile for misc directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 
CDF		= -L$(NETCDF)/lib -lnetcdf

PROGS_H		=
PROGS_C		= gmtdigitize.c gmtstitch.c nc2xy.c psmegaplot.c
PROGSPS_C	= makepattern.c

PROGS_O		= $(PROGS_C:.c=.o)
PROGS		= $(PROGS_C:.c=$(EXE))
PROGSPS_O	= $(PROGSPS_C:.c=.o)
PROGSPS		= $(PROGSPS_C:.c=$(EXE))
MAN1		= $(PROGS_C:.c=.1) $(PROGSPS_C:.c=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:            $(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless:	clean

clean:	
		rm -f *.o $(PROGS)

#-------------------------------------------------------------------------------
#       object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)
$(PROGSPS_O):	$(GMT_H) $(PS_H) $(PROGS_H)

#-------------------------------------------------------------------------------
#       program rules
#-------------------------------------------------------------------------------

gmtdigitize$(EXE):	gmtdigitize.o
gmtstitch$(EXE):	gmtstitch.o
nc2xy$(EXE):		nc2xy.o
psmegaplot$(EXE):	psmegaplot.o
makepattern$(EXE):	makepattern.o

$(PROGS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt -lgmtps -lpsl $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
