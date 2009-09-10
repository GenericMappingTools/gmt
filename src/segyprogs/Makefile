#	$Id: Makefile,v 1.27 2009-09-10 02:21:36 guru Exp $
#
# makefile for segyprogs directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

INCLUDES	= -I$(srcdir) -I$(NETCDF_INC) -I$(GDAL_INC)

PROGS_H		= segy_io.h segy.h segyreel.h
PROGS_O		= segy2grd.o
PROGSPS_O	= pssegyz.o pssegy.o
MAN1		= $(PROGS_O:.o=.1) $(PROGSPS_O:.o=.1)

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS) $(PROGSPS)

install:	all
		$(INSTALL) $(PROGS) $(PROGSPS) $(bindir)

uninstall:	
		cd $(bindir); \rm -f $(PROGS) $(PROGSPS)

clean:
		\rm -f *.o *% $(PROGS) $(PROGSPS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

segy_io.o:	$(PROGS_H)
$(PROGS_O):	$(GMT_H) $(PROGS_H)
$(PROGSPS_O):	$(GMT_H) $(PS_H) $(PROGS_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

pssegyz$(EXE):	pssegyz.o
pssegy$(EXE):	pssegy.o
segy2grd$(EXE):	segy2grd.o

$(PROGS):	$(GMTLIB) segy_io.o
		$(CC) $(LDFLAGS) $(@:.exe=).o segy_io.o -L.. -lgmt $(NC_LIBS) $(GDAL_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(GMTLIB) segy_io.o
		$(CC) $(LDFLAGS) $(@:.exe=).o segy_io.o -L.. -lgmt -lgmtps -lpsl $(NC_LIBS) $(GDAL_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@
