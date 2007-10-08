#	$Id: Makefile,v 1.6 2007-10-08 21:26:08 remko Exp $
#	makefile for sph directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include ../makegmt.macros

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 

PROGS_H		=
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

$(PROGS_O):	$(GMT_H) $(PROGS_H)

#-------------------------------------------------------------------------------
#       program rules
#-------------------------------------------------------------------------------

grd2sph$(EXE):	grd2sph.o
sph2grd$(EXE):	sph2grd.o

$(PROGS):
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
