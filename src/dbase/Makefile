#	$Id: Makefile,v 1.3 2001-03-09 18:32:11 pwessel Exp $
#
#	makefile for dbase directory
#
# Uncomment SWAP to build in byte-order swapping of all data
#SWAP	= -DSWAP_ME
#----------------- stop here - nothing to edit ------------------

include ../makegmt.macros

CDF		= -L$(NETCDF)/lib -lnetcdf
CFLAGS		= $(CC_OPT) $(WIN32) $(SWAP) -I.. -I$(NETCDF)/include

all:		grdraster

install:	all
		$(INSTALL) grdraster$(EXE) $(bindir)

uninstall:
		\rm -f $(bindir)/grdraster$(EXE)

grdraster:	grdraster.o
		$(CC) $(CFLAGS) $(LDFLAGS) grdraster.o -L.. -lgmt -lpsl $(CDF) $(LIBS) -o grdraster

spotless:	clean

clean:	
		rm -f *.o grdraster$(EXE)

.c.o:
		$(CC) -c $(CFLAGS) $<
