#       $Id: Makefile,v 1.27 2009-10-23 18:32:37 remko Exp $
#
#	makefile for meca directory
#	meca must be installed in the gmt/src directory or
#	you must fix the -L -I for the gmt-dependencies.
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)
ALLLIB		= $(GMTPS_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS)

PROGS_O		= psmeca.o pscoupe.o pspolar.o psvelo.o
MAN1		= $(PROGS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:
		\rm -f *.o *% $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PS_H) meca.h nrutil.h submeca.h utilmeca.h utilstrain.h utilvelo.h

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

psmeca$(EXE): 	psmeca.o utilmeca.o nrutil.o $(LIBGMT)
		$(CC) $(LDFLAGS) psmeca.o utilmeca.o nrutil.o $(ALLLIB) -o $@
		$(COMPRESS) $@

pscoupe$(EXE):	pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(LIBGMT)
		$(CC) $(LDFLAGS) pscoupe.o utilmeca.o submeca.o distaz.o nrutil.o $(ALLLIB) -o $@
		$(COMPRESS) $@

pspolar$(EXE): 	pspolar.o $(LIBGMT)
		$(CC) $(LDFLAGS) pspolar.o $(ALLLIB) -o $@
		$(COMPRESS) $@

psvelo$(EXE):	psvelo.o utilvelo.o utilstrain.o $(LIBGMT)
		$(CC) $(LDFLAGS) psvelo.o utilvelo.o utilstrain.o $(ALLLIB) -o $@
		$(COMPRESS) $@
