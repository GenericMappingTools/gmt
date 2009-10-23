#	$Id: Makefile,v 1.29 2009-10-23 18:32:37 remko Exp $
#
# makefile for segyprogs directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

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

$(PROGS):	$(LIBGMT) segy_io.o
		$(CC) $(LDFLAGS) $(@:.exe=).o segy_io.o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGSPS):	$(LIBGMT) segy_io.o
		$(CC) $(LDFLAGS) $(@:.exe=).o segy_io.o $(GMTPS_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
