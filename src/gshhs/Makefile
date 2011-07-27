#	$Id$
#
#	makefile for gshhs directory
#	Does not depend on GMT src or libraries
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)

API_O		= gmt_gshhs.o
FUNCS_O		= gshhs_func.o
PROGS_API_O	= $(FUNCS_O:_func.o=.o)
PROGS_API	= $(PROGS_API_O:.o=$(EXE))
PROGS_O		= gshhs_dp.o gshhstograss.o
LIB_H		= gshhs.h
MAN1		= $(PROGS_O:.o=.1) gshhs.1

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS) $(PROGS_API)

install:	all
		$(INSTALL) $(PROGS_API) $(PROGS) $(bindir)
		if [ ! .. -ef $(libdir) ]; then \
			$(INSTALL_DIR) $(libdir) ; \
			$(INSTALL_DATA) $(LIBAPI:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) $(LIBAPI) $(libdir); fi \
		fi
		if [ ! .. -ef $(includedir) ]; then \
			$(INSTALL_DIR) $(includedir)/gmt; \
			$(INSTALL_DATA) $(API_H) $(includedir)/gmt; \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f $(LIBAPI:.$(LIBEXT)=).{$(SL),$(SL_VERSION)}; \
			$(LN_S) $(LIBAPI:.$(LIBEXT)=).{$(SL_VERSION),$(SL)}; \
		fi

uninstall:
		cd $(bindir); \rm -f $(PROGS)
		if [ ! .. -ef $(includedir) ]; then \
			cd $(includedir)/gmt; \rm -f $(API_H);\
		fi
		if [ ! .. -ef $(libdir) ]; then \
			cd $(libdir); \rm -f $(LIBAPI:.$(LIBEXT)=).{a,$(SL),$(SL_VERSION)}; \
		fi

install-man:	$(MAN1)
		$(INSTALL_DATA) $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean

clean:	
		\rm -f *.o *.a *.$(SL) $(PROGS_API) $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(FUNCS_O) $(PROGS_API_O):	$(GMT_H) $(LIB_H) $(API_H)
$(PROGS_O):	$(GMT_H) $(LIB_H)

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

LIBAPI		= libgmt_gshhs.$(LIBEXT)

libs:		libgmt_gshhs.a

libgmt_gshhs.a:	$(FUNCS_O) $(API_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libgmt_gshhs.$(SL):	libgmt_gshhs.a
		$(LD) $(LD_OPT) $(FUNCS_O) $(API_O) $(GMT_LIB) $(LIBS) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

gshhs$(EXE):		gshhs.o
gshhs_dp$(EXE):		gshhs_dp.o
gshhstograss$(EXE):	gshhstograss.o

$(PROGS):	$(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@

$(PROGS_API):	$(PROGS_API_O) $(LIBAPI)
		$(CC) $(LDFLAGS) $(@:.exe=).o $(LIBAPI) $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@

$(PROGS_API_O):	../gmtprogram.c
		$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -DFUNC_MODE=GMTAPI_GMT -DFUNC=GMT_$* -c -o $*.o ../gmtprogram.c
