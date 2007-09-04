#	$Id: Makefile,v 1.19 2007-09-04 15:22:27 remko Exp $
#
# makefile for segyprogs directory
#
#

#------------------------- stop here --------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I$(NETCDF)/include -I$(GMT_DEFAULT_PATH)/src
ALLLIB	= -L.. -L$(libdir) -lgmt -lgmtps -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

SEGYPLOT_H	= segy_io.h segy.h segyreel.h
SEGYPLOT_C	= pssegyz.c pssegy.c segy2grd.c

SEGYPLOT_O	= $(SEGYPLOT_C:.c=.o)
SEGYPLOT	= $(SEGYPLOT_C:.c=$(EXE))

#---------------------------------------------------------------

all:			$(SEGYPLOT) 

install:		all
			$(INSTALL) $(SEGYPLOT) $(bindir)

uninstall:	
			for i in $(SEGYPLOT); do \
				\rm -f $(bindir)/$$i; \
			done

clean:
			\rm -f *.o *.ps $(SEGYPLOT)

spotless:		clean
			\rm -f *.a

segy_io.o:		$(SEGYPLOT_H)

$(SEGYPLOT):		$(SEGYPLOT_O) segy_io.o $(GMTLIB)
			$(CC) $(LDFLAGS) segy_io.o $(@:.exe=).o -o $@ $(ALLLIB)

$(SEGYPLOT_O):		$(GMT_H) $(PS_H)
