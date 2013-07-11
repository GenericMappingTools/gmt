#	$Id: Makefile 11855 2013-06-30 05:22:46Z pwessel $
#
#	makefile for GMT F77 App directory

include config.mk

INCLUDES	= $(GMT_INC)

PROGS_F		= test_grd.f
PROGS_FO	= $(PROGS_F:.f=.o)
FPROGS          = $(PROGS_FO:.o=)
FC		= gfortran
FFLAGS		= -g -Bstatic -m64

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(FPROGS)

install:	all
		$(INSTALL) $(FPROGS) ../../bin/$(OS)

uninstall:
		cd ../../bin/$(OS); rm -f $(FPROGS)

spotless::	clean
		rm -rf config.mk configure autom4te.cache

clean:	
		rm -f *.o *% core tags config.log config.status $(FPROGS)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

test_grd:	test_grd.o SIO_GMT_io.o

$(FPROGS):
		$(FC) $(LDFLAGS) $@.o SIO_GMT_io.o $(GMT_LIB) -o $@
