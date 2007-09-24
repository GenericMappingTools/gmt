#	$Id: Makefile,v 1.20 2007-09-24 00:38:02 remko Exp $
#
# makefile for segyprogs directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include -I$(GMT_DEFAULT_PATH)/src
ALLLIB	= -L.. -L$(libdir) -lgmt -lgmtps -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

PROGS_H	= segy_io.h segy.h segyreel.h
PROGS_C	= pssegyz.c pssegy.c segy2grd.c
PROGS_O	= $(PROGS_C:.c=.o)
PROGS	= $(PROGS_C:.c=$(EXE))
MAN1	= $(PROGS_C:.c=.1)

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS) 

install:	all
		$(INSTALL) $(PROGS) $(bindir)

uninstall:	
		cd $(bindir); \rm -f $(PROGS)

clean:
		\rm -f *.o *% $(PROGS)

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless:	clean

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PS_H) $(PROGS_H)
segy_io.o:	$(PROGS_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

pssegyz$(EXE):	pssegyz.o
pssegy$(EXE):	pssegy.o
segy2grd$(EXE):	pssegy2grd.o

$(PROGS):	segy_io.o $(GMTLIB)
		$(CC) $(LDFLAGS) segy_io.o $(@:.exe=).o -o $@ $(ALLLIB)
		$(COMPRESS) $@
