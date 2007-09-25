#	$Id: Makefile,v 1.28 2007-09-25 01:50:16 remko Exp $
#
# makefile for x2sys directory
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

GMTSRCDIR = ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

CFLAGS	= $(CC_OPT) $(WIN32) -I$(srcdir) -I../mgg -I../mgd77 -I$(NETCDF)/include
ALLLIB	= -L../mgd77 -lmgd77 -L../mgg -lgmt_mgg -L.. -L$(libdir) -lgmt -lpsl -L$(NETCDF)/lib -lnetcdf $(LIBS)

PROGS_H	= x2sys.h
PROGS_C	= x2sys_init.c x2sys_put.c x2sys_get.c x2sys_binlist.c x2sys_datalist.c x2sys_cross.c
PROGS_O	= $(PROGS_C:.c=.o)
PROGS	= $(PROGS_C:.c=$(EXE))
MAN1	= $(PROGS_C:.c=.1)
LIB	= libx2sys.a

#---------------------------------------------------------------
#	software targets
#---------------------------------------------------------------

all:		$(PROGS)

install:	all
		$(INSTALL) $(PROGS) $(bindir)
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -m 444 libx2sys.a $(libdir); \
		fi
		$(INSTALL) -d $(datadir)/x2sys
		for i in *.def; do \
			cp -f -p $$i $(datadir)/x2sys; \
		done

uninstall:	
		cd $(bindir); \rm -f $(PROGS)
		if [ ! $(libdir) = $(srcdir) ]; then \
			cd $(libdir); \rm -f $(LIB)
		fi
		\rm -rf $(datadir)/x2sys

install-man:	$(MAN1)
		$(INSTALL) -m 644 $(MAN1) $(mandir)/man1

uninstall-man:
		cd $(mandir)/man1; \rm -f $(MAN1)

spotless::	clean
		\rm -f $(LIB)

clean:
		\rm -f *.o *% $(PROGS)

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(GMT_H) $(PROGS_H)
libx2sys.o:	$(PROGS_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

libx2sys.a:	x2sys.o
		$(AR) cvr libx2sys.a $?
		$(RANLIB) libx2sys.a

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
