#	$Id: Makefile,v 1.26 2007-10-03 14:24:17 remko Exp $
#	makefile for misc directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 

PROGS_H		=
PROGS_O		= gmtdigitize.o gmtstitch.o nc2xy.o psmegaplot.o
PROGSPS_O	= makepattern.o
MAN1		= $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)

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

spotless::	clean

clean:	
		\rm -f *.o *% $(PROGS)

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
