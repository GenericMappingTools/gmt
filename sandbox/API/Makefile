#	$Id$
#
#	makefile for GMT App directory

include config.mk

INCLUDES	= $(GMT_INC)

PROGS_C		= grd_fft_template.c gmt_parsing.c
PROGS_F		= test_grd.f
PROGS_CO	= $(PROGS_C:.c=.o)
PROGS_FO	= $(PROGS_F:.f=.o)
CPROGS          = $(PROGS_CO:.o=)
FPROGS          = $(PROGS_FO:.o=)
FC		= gfortran
FFLAGS		= -g -Bstatic -m64

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(CPROGS) $(FPROGS)

install:	all
		$(INSTALL) $(CPROGS)  $(FPROGS) ../../bin/$(OS)

uninstall:
		cd ../../bin/$(OS); \rm -f $(CPROGS) $(FPROGS)

spotless::	clean

clean:	
		\rm -f *.o *% core tags $(CPROGS) $(FPROGS)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grd_fft_template:	grd_fft_template.o
gmt_parsing:		gmt_parsing.o
test_grd:		test_grd.o SIO_GMT_io.o

$(CPROGS):
		$(CC) $(LDFLAGS) $@.o $(GMT_LIB) -o $@

$(FPROGS):
		$(FC) $(LDFLAGS) $@.o SIO_GMT_io.o $(GMT_LIB) -o $@
