#       $Id: Makefile,v 1.29 2011-03-15 02:06:37 guru Exp $
#
#	makefile for meca directory
#	meca must be installed in the gmt/src directory or
#	you must fix the -L -I for the gmt-dependencies.
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)
ALLLIB		= $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS)

API_O		= gmt_meca.o
API_H		= $(API_O:.o=.h)
LIB_O		= utilmeca.o nrutil.o submeca.o distaz.o
FUNCS_O		= psmeca_func.o pspolar_func.o psvelo_func.o pscoupe_func.o
PROGS_O		= $(FUNCS_O:_func.o=.o)

LIB_H		= meca.h nrutil.h submeca.h utilmeca.h

MAN1		= $(PROGS_O:.o=.1)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -d $(includedir)/gmt; \
			$(INSTALL) -m 644 $(API_H) $(includedir)/gmt; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir)/gmt; \rm -f $(API_H);\
		fi

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:
		\rm -f *.o *.a *.$(SL) *% $(PROGS)

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_meca.$(LIBEXT)

libs:		libgmt_meca.a

libgmt_meca.a:	$(FUNCS_O) $(LIB_O) $(API_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_meca.$(SL):	libgmt_meca.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(LIB_O) $(API_O) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(FUNCS_O) $(PROGS_O):	$(GMT_H) $(PS_H) $(API_H) $(LIB_H)
$(LIB_O):	$(GMT_H) $(PS_H) $(LIB_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

psmeca$(EXE):	psmeca.o
psvelo$(EXE):	psvelo.o
pscoupe$(EXE):	pscoupe.o
pspolar$(EXE):	pspolar.o

$(PROGS):	$(LIBGMT) $(LIBAPI) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(LIBAPI) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMTPSL -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c

