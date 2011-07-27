#	$Id$
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

API_O		= gmt_sph.o
API_H		= $(API_O:.o=.h)
FUNCS_O		= sphtriangulate_func.o sphdistance_func.o sphinterpolate_func.o

LIB_O		= sph.o
LIB_H		= sph.h

PROGS_O		= $(FUNCS_O:_func.o=.o)
MAN1		= $(PROGS_O:.o=.1)

LIB_C		= sph.c
SCRIPTS		= sph_ex_1.sh sph_ex_2.sh sph_ex_3.sh sph_ex_4.sh

#-------------------------------------------------------------------------------
all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(includedir)/gmt; \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL_DIR) $(libdir) ; \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).{$(SL),$(SL_VERSION)}; \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).{$(SL_VERSION),$(SL)}; \
		fi

uninstall:
		cd $(bindir); rm -f $(PROGS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir)/gmt; rm -f $(API_H);\
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); rm -f $(LIBAPI:.$(LIBEXT)=).{a,$(SL),$(SL_VERSION)}; \
		fi

install-man:	$(MAN1)
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:
		\rm -f *.o *.a *.$(SL) *% $(PROGS)

ex:		$(SCRIPTS)
		for i in $(SCRIPTS); do \
			sh $$i; \
		done

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_sph.$(LIBEXT)

libs:		libgmt_sph.a

libgmt_sph.a:	$(FUNCS_O) $(LIB_O) $(API_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_sph.$(SL):	libgmt_sph.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(LIB_O) $(API_O) $(GMT_LIB) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(FUNCS_O) $(PROGS_O):	$(LIB_H) $(GMT_H) $(PS_H) $(API_H) 
$(LIB_O):	$(LIB_H) $(GMT_H) stripack.c ssrfpack.c

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

sphtriangulate$(EXE):	sphtriangulate.o
sphdistance$(EXE):	sphdistance.o
sphinterpolate$(EXE):	sphinterpolate.o

$(PROGS):	$(PROGS_O) $(LIBAPI) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lgmt_sph $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) $(LIBAPI) -o $@

$(PROGS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c
