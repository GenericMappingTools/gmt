#	$Id$
#
#	makefile for GMT App directory

include config.mk

INCLUDES	= $(GMT_INC) $(NETCDF_INC) $(GDAL_INC)

PROGS_C		= grd_fft_template.c
PROGS_O		= $(PROGS_C:.c=.o)
PROGS           = $(PROGS_O:.o=)

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

$(PROGS):	
		$(CC) $(LDFLAGS) $@.o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
