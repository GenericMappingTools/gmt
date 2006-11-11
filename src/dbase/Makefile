#	$Id: Makefile,v 1.9 2006-11-11 15:07:16 remko Exp $
#
#	makefile for dbase directory
#
# Uncomment SWAP to build in byte-order swapping of all data
#SWAP	= -DSWAP_ME
#----------------- stop here - nothing to edit ------------------

include ../makegmt.macros
GMTSRCDIR	= ..
include ../gmtalldeps.macros

CDF		= -L$(NETCDF)/lib -lnetcdf
CFLAGS		= $(CC_OPT) $(WIN32) $(SWAP) -I$(srcdir) -I$(NETCDF)/include

all:		grdraster

install:	all
		$(INSTALL) grdraster$(EXE) $(bindir)

uninstall:
		\rm -f $(bindir)/grdraster$(EXE)

grdraster:	grdraster.o $(GMTLIB)
		$(CC) $(CFLAGS) $(LDFLAGS) grdraster.o -L.. -L$(libdir) -lgmt $(CDF) $(LIBS) -o grdraster
		$(COMPRESS) grdraster$(EXE)

spotless:	clean

clean:	
		rm -f *.o grdraster$(EXE)

.c.o:		$(GMT_H)
		$(CC) -c $(CFLAGS) $<

#---------------------------- CVS GURU EXTENSIONS-------------------------------
sinclude makefile.guru
