#	$Id: Makefile,v 1.26 2011-03-15 02:06:37 guru Exp $
#
#
#	makefile for imgsrc directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

API_O		= gmt_img.o
API_H		= $(API_O:.o=.h)
LIB_O		= gmt_imgsubs.o
FUNCS_O		= img2grd_func.o
LIB_H		= gmt_imgsubs.h

PROGS_O		= $(FUNCS_O:_func.o=.o)
MAN1		= $(PROGS_O:.o=.1) img2google.1

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) img2google $(bindir)
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -d $(libdir) ; \
			$(INSTALL) -m 644 $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) -m 755 $(LIBAPI) $(libdir); fi \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -d $(includedir)/gmt; \
			$(INSTALL) -m 644 $(API_H) $(includedir)/gmt; \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).$(SL) $(LIBAPI:.$(LIBEXT)=).$(SL_VERSION); \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).$(SL_VERSION) $(LIBAPI:.$(LIBEXT)=).$(SL); \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS) img2google
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

LIBAPI		= libgmt_img.$(LIBEXT)

libs:		libgmt_img.a

libgmt_img.a:	$(FUNCS_O) $(LIB_O) $(API_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_img.$(SL):	libgmt_img.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(LIB_O) $(API_O) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(FUNCS_O) $(PROGS_O):	$(GMT_H) $(API_H) $(LIB_H)
$(LIB_O):	$(LIB_H)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

img2grd$(EXE):	img2grd.o

$(PROGS):	$(LIBGMT) $(LIBAPI) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(LIBAPI) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@

$(PROGS_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c
