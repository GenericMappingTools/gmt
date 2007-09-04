#	$Id: Makefile,v 1.2 2007-09-04 15:22:27 remko Exp $
#	makefile for sph directory
#

#----------------- stop here - nothing to edit--------------------

include ../makegmt.macros

CDF		= -L$(NETCDF)/lib -lnetcdf

CFLAGS		= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include 

MISC_H		=
MISC_C		= grd2sph.c sph2grd.c

#-------------------------------------------------------------------------------
#       object file dependencies
#-------------------------------------------------------------------------------

MISC_O		= $(MISC_C:.c=.o$(EXE))
MISC		= $(MISC_C:.c=)

all:            $(MISC)

$(MISC_O):	$(GMT_H) $(MISC_H)

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

$(MISC):	$(MISC_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o $@
