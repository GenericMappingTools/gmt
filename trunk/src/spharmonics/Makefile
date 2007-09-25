#	$Id: Makefile,v 1.4 2007-09-25 01:50:16 remko Exp $
#	makefile for sph directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include ../makegmt.macros

CDF		= -L$(NETCDF)/lib -lnetcdf

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 

PROGS_H		=
PROGS_C		= grd2sph.c sph2grd.c
PROGS_O		= $(PROGS_C:.c=.o$(EXE))
PROGS		= $(PROGS_C:.c=)

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
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
		$(COMPRESS) $@
