#	$Id: Makefile,v 1.34 2008-02-25 17:18:38 remko Exp $
#
# makefile for x2sys directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS		= $(CC_OPT) -I$(srcdir) -I../mgg -I../mgd77 $(NC_CFLAGS)
ALLLIB		= -L../mgd77 -lmgd77 -L../mgg -lgmt_mgg -L.. -lgmt $(NC_LIBS) $(LIBS)

PROGS_H		= x2sys.h
PROGS_O		= x2sys_init.o x2sys_put.o x2sys_get.o x2sys_binlist.o x2sys_datalist.o x2sys_cross.o
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
			$(INSTALL) -m 444 $(LIB) $(LIB:.$(LIBEXT)=.a) $(libdir); \
		fi
		if [ -f $(libdir)/libx2sys.dylib ]; then \
			cd $(libdir) ; \
			\mv -f libx2sys{,.4}.dylib ; \
			$(LN_S) libx2sys{.4,}.dylib ; \
		fi
		$(INSTALL) -d $(datadir)/x2sys
		for i in *.def; do \
			cp -f -p $$i $(datadir)/x2sys; \
		done

uninstall:	
		cd $(bindir); \rm -f $(PROGS)
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f $(LIB) $(LIB:.$(LIBEXT)=.a)
		fi
		\rm -rf $(datadir)/x2sys

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

clean:
		\rm -f *.o *.a *.so *.sa *.sl *.dylib *% $(PROGS)

spotless::	clean

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)
x2sys.o:	$(PROGS_H) $(GMT_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

libx2sys.a:	x2sys.o
		$(AR) cvr $@ $?
		$(RANLIB) $@

libx2sys.$(SL):	libx2sys.a
		$(LD) $(LD_OPT) x2sys.o -o $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

x2sys_init$(EXE):	x2sys_init.o
x2sys_put$(EXE):	x2sys_put.o
x2sys_get$(EXE):	x2sys_get.o
x2sys_binlist$(EXE):	x2sys_binlist.o
x2sys_datalist$(EXE):	x2sys_datalist.o
x2sys_cross$(EXE):	x2sys_cross.o

$(PROGS):	$(LIB) $(GMTLIB)
		$(CC) $(LDFLAGS) $(@:.exe=).o -o $@ -L. -lx2sys $(ALLLIB)
		$(COMPRESS) $@
