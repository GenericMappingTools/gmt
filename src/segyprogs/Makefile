#	$Id: Makefile,v 1.1 2001-03-02 05:54:44 pwessel Exp $
#
# makefile for segyprogs directory
#
#
include ../makegmt.macros

#------------------------- stop here --------------------------

CFLAGS	= $(CC_OPT) $(WIN32) -I.. -I$(NETCDF)/include -I$(GMT_DEFAULT_PATH)/src
ALLLIB	= -L..  -L$(GMT_DEFAULT_PATH)/bin -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

SEGYPLOT_H	= segy_io.h segy.h segyreel.h
SEGYPLOT_C	= pssegyz.c pssegy.c

SEGYPLOT_O	= ${SEGYPLOT_C:.c=.o}

SEGYPLOT		= ${SEGYPLOT_C:.c=}

#---------------------------------------------------------------

all:			$(SEGYPLOT) 

install:		all
			for i in $(SEGYPLOT); do \
				$(INSTALL) $$i$(EXE) $(bindir); \
			done

uninstall:	
			for i in $(SEGYPLOT); do \
				\rm -f $(bindir)/$$i$(EXE); \
			done

clean:
			\rm -f *.o
			for i in $(SEGYPLOT); do \
				\rm -f $$i$(EXE); \
			done

spotless:		clean
			\rm -f *.a

$(SEGYPLOT):		$(SEGYPLOT_O) segy_io.o
			$(CC) $(CFLAGS) segy_io.o $@.o -o $@ $(ALLLIB) $(LDFLAGS)

.c.o:
			$(CC) -c $(CFLAGS) $<
