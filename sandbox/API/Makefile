#	$Id$
#
#	makefile for GMT App directory

include config.mk

INCLUDES	= $(GMT_INC) $(NETCDF_INC) $(GDAL_INC)

PROGS_C		= grd_fft_template.c gmt_parsing.c
PROGS_F		= test_grd.f
PROGS_O		= $(PROGS_C:.c=.o) $(PROGS_F:.f=.o)
PROGS           = $(PROGS_O:.o=)
FC		= gfortran
FFLAGS		= -g -Bstatic -m64

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) ../../bin/$(OS)

uninstall:
		cd ../../bin/$(OS); \rm -f $(PROGS)

spotless::	clean

clean:	
		\rm -f *.o *% core tags $(PROGS)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grd_fft_template:	grd_fft_template.o
gmt_parsing:		gmt_parsing.o
test_grd:		test_grd.o SIO_GMT_io.o

$(PROGS):	
		$(CC) $(LDFLAGS) $@.o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
