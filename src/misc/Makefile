#	$Id: Makefile,v 1.34 2009-06-04 00:01:38 guru Exp $
#	makefile for misc directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

INCLUDES	= -I$(srcdir) -I$(NETCDF_INC)

PROGS_H		=
PROGS_O		= dimfilter.o gmt2kml.o kml2gmt.o gmtdigitize.o gmtdp.o gmtstitch.o nc2xy.o psmegaplot.o
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
		\rm -f *.o *% $(PROGS) $(PROGSPS)

#-------------------------------------------------------------------------------
#       object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)
$(PROGSPS_O):	$(GMT_H) $(PS_H) $(PROGS_H)

#-------------------------------------------------------------------------------
#       program rules
#-------------------------------------------------------------------------------

dimfilter$(EXE):	dimfilter.o
gmt2kml$(EXE):		gmt2kml.o
kml2gmt$(EXE):		kml2gmt.o
gmtdigitize$(EXE):	gmtdigitize.o
gmtdp$(EXE):		gmtdp.o
gmtstitch$(EXE):	gmtstitch.o
nc2xy$(EXE):		nc2xy.o
psmegaplot$(EXE):	psmegaplot.o
makepattern$(EXE):	makepattern.o

$(PROGS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -lgmt $(NC_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -lgmt -lgmtps -lpsl $(NC_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@
