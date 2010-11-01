#	$Id: Makefile,v 1.52 2010-11-01 22:31:53 remko Exp $
#
# makefile for x2sys directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) -I../mgg -I../mgd77 $(NETCDF_INC) $(GDAL_INC)
ALLLIB		= -L../mgd77 -lmgd77 -L../mgg -lgmt_mgg $(GMT_LIB) $(LIBS)

LIB_O		= x2sys.o
LIB_H		= x2sys.h
LIB		= libx2sys.$(LIBEXT)

PROGS_H		=
PROGS_O		= x2sys_init.o x2sys_put.o x2sys_get.o x2sys_binlist.o x2sys_datalist.o \
		  x2sys_cross.o x2sys_list.o x2sys_report.o x2sys_solve.o x2sys_merge.o
MAN1		= $(PROGS_O:.o=.1)

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! $(includedir) = $(srcdir) ]; then \
			$(INSTALL) -d $(includedir); \
			$(INSTALL) -m 644 $(LIB_H) $(includedir); \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -d $(libdir); \
			$(INSTALL) -m 644 $(LIB:.$(LIBEXT)=.a) $(libdir); \
			if [ ! $(LIBEXT) = a ]; then $(INSTALL) -m 755 $(LIB) $(libdir); fi \
		fi
		if [ $(LIBEXT) != a ] && [ $(SL) != $(SL_VERSION) ]; then \
			cd $(libdir) ; \
			\mv -f libx2sys.$(SL) libx2sys.$(SL_VERSION) ; \
			$(LN_S) libx2sys.$(SL_VERSION) libx2sys.$(SL) ; \
		fi

uninstall:	
		cd $(bindir); \rm -f $(PROGS)
		if [ ! $(includedir) = $(srcdir) ]; then \
			cd $(includedir); \rm -f $(LIB_H);\
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f libx2sys.a libx2sys.$(SL) libx2sys.$(SL_VERSION); \
		fi

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		\rm -f *% *.o *.a *.$(SL) $(PROGS)

spotless::	clean

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(LIB_H) $(PROGS_H) $(GMT_H)
$(LIB_O):	$(LIB_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libx2sys.a:	$(LIB_O)
		$(AR) cvr $@ $?
		$(RANLIB) $@

libx2sys.$(SL):	libx2sys.a
		$(LD) $(LD_OPT) x2sys.o $(ALLLIB) -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

x2sys_init$(EXE):	x2sys_init.o
x2sys_put$(EXE):	x2sys_put.o
x2sys_get$(EXE):	x2sys_get.o
x2sys_binlist$(EXE):	x2sys_binlist.o
x2sys_datalist$(EXE):	x2sys_datalist.o
x2sys_cross$(EXE):	x2sys_cross.o
x2sys_list$(EXE):	x2sys_list.o
x2sys_report$(EXE):	x2sys_report.o
x2sys_solve$(EXE):	x2sys_solve.o
x2sys_merge$(EXE):	x2sys_merge.o

$(PROGS):	$(LIB) $(LIBGMT)
		$(CC) $(LDFLAGS) $(@:.exe=).o -o $@ -L. -lx2sys $(ALLLIB) $(NETCDF_LIB) $(GDAL_LIB)
		$(COMPRESS) $@
