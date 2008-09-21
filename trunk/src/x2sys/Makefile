#	$Id: Makefile,v 1.44 2008-09-21 00:30:06 guru Exp $
#
# makefile for x2sys directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

INCLUDES	= -I$(srcdir) -I../mgg -I../mgd77 -I$(NETCDF_INC)
ALLLIB		= -L../mgd77 -lmgd77 -L../mgg -lgmt_mgg -L.. -lgmt $(LIBS)

PROGS_H		= x2sys.h
PROGS_O		= x2sys_init.o x2sys_put.o x2sys_get.o x2sys_binlist.o x2sys_datalist.o \
		  x2sys_cross.o x2sys_list.o x2sys_solve.o
MAN1		= $(PROGS_O:.o=.1)
LIB		= libx2sys.$(LIBEXT)

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
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
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f $(LIB) $(LIB:.$(LIBEXT)=.a)
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

$(PROGS_O):	$(GMT_H) $(PROGS_H)
x2sys.o:	$(PROGS_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

.SUFFIXES:	.$(SL)

libx2sys.a:	x2sys.o
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
x2sys_solve$(EXE):	x2sys_solve.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -o $@ -L. -lx2sys $(ALLLIB) $(NC_LIBS)
		$(COMPRESS) $@
