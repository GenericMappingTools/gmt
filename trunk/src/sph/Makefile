#	$Id: Makefile,v 1.19 2010-09-14 23:57:31 guru Exp $
#
#		Makefile for GMT supplement sph
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	17-MAY-2009
#

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES = -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

CODE_C		= sphtriangulate.c sphdistance.c sphinterpolate.c
LIB_C		= sph.c
MAN1		= $(CODE_C:.c=.1)
SCRIPTS		= sph_ex_1.sh sph_ex_2.sh sph_ex_3.sh sph_ex_4.sh
SPHLIB		= libsph.a
#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

CODE_O	= ${CODE_C:.c=.o}
CODE	= ${CODE_C:.c=}
LIB_O	= ${LIB_C:.c=.o}

all:		$(CODE)

#-------------------------------------------------------------------------------

install:	all
		for i in $(CODE); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		rm -f *.o
		for i in $(CODE); do \
			rm -f $$i$(EXE); \
		done

ex:		$(SCRIPTS)
		for i in $(SCRIPTS); do \
			sh $$i; \
		done
		
#-------------------------------------------------------------------------------
#       sph library
#-------------------------------------------------------------------------------

$(SPHLIB):	$(LIB_O)
	$(AR) cvr $@ $?
	$(RANLIB) $@

sph.o:	sph.c sph.h stripack.c ssrfpack.c $(GMT_H)

spotless::	clean
	rm -rf $(SPHLIB)
	
#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(CODE):	$(CODE_O) $(SPHLIB)
	$(CC) $(LDFLAGS) $(@:.exe=).o $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) -L. -lsph  $(LIBS) -o $@
