#	$Id: Makefile,v 1.12 2011-04-14 22:18:30 guru Exp $
#	makefile for sph directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC)

PROGS_O		= grd2sph.o sph2grd.o

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:            $(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		cd $(bindir); \rm -f $(PROGS)

install-man:	;
uninstall-man:	;

spotless:	clean

clean:	
		\rm -f *.o *% $(PROGS)

#-------------------------------------------------------------------------------
#       object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H)

#-------------------------------------------------------------------------------
#       program rules
#-------------------------------------------------------------------------------

grd2sph$(EXE):	grd2sph.o
sph2grd$(EXE):	sph2grd.o

$(PROGS):
		$(CC) $(LDFLAGS) $(@:.exe=).o $(GMT_LIB) $(NETCDF_LIB) $(LIBS) -o $@
