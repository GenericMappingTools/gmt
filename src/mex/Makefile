#		$Id: Makefile,v 1.12 2008-09-06 04:04:34 guru Exp $
#
#	Makefile for gmt <--> octave/matlab interface programs
#	Use with GMT 4.3.0 or later
#
# Note that you must have write permission to mkdir and write to $MEX_XDIR and $MEX_MDIR
# 
#----------------------------------------------------------------------------
#	The rest should should be ok. NO EDITING BEYOND HERE, unless your
#	Octave/Matlab setup was not auto-detected by the configure file
#	If so, please let us know what you needed to change.
#----------------------------------------------------------------------------

GMTSRCDIR	= ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

.SUFFIXES:	.m .$(MEX_EXT)

GMT     	= -L.. -lgmt $(NC_LIBS)
FLAGS		= -I$(srcdir) $(MEX_INC) -I$(NETCDF_INC)

PROGS_M		= grdinfo.m grdread.m grdwrite.m imgread.m
PROGS_MEX	= grdinfo.$(MEX_EXT) grdread.$(MEX_EXT) grdwrite.$(MEX_EXT)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS_MEX)

install:	all
		$(INSTALL) -d $(MEX_XDIR)
		if [ -w $(MEX_XDIR) ] ; then \
			$(INSTALL) $(PROGS_MEX) $(MEX_XDIR); \
		else \
			$(INSTALL) $(PROGS_MEX) $(bindir); \
		fi
		$(INSTALL) -d $(MEX_MDIR)
		if [ -w $(MEX_MDIR) ] ; then \
			$(INSTALL) -m 644 $(PROGS_M) $(MEX_MDIR); \
		else \
			$(INSTALL) -m 644 $(PROGS_M) $(bindir); \
		fi

uninstall:
		if [ -w $(MEX_XDIR) ] ; then \
			cd $(MEX_XDIR); \
		else \
			cd $(bindir); \
		fi; \rm -f $(PROGS_MEX)
		if [ -w $(MEX_MDIR) ] ; then \
			cd $(MEX_MDIR); \
		else \
			cd $(bindir); \
		fi; \rm -f $(PROGS_M)

install-man uninstall-man manpages:	;

spotless:	clean

clean:
		\rm -f .gmt* *% *.o .mexrc.sh $(PROGS_MEX)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grdinfo.$(MEX_EXT):	$(GMT_H) $(GMTLIB) grdinfo.c grdinfo.m
		$(MEX_BLD) $(FLAGS) grdinfo.c $(GMT) $(MEX_LIB) 

grdread.$(MEX_EXT):	$(GMT_H) $(GMTLIB) grdread.c grdread.m
		$(MEX_BLD) $(FLAGS) grdread.c $(GMT) $(MEX_LIB) 

grdwrite.$(MEX_EXT):	$(GMT_H) $(GMTLIB) grdwrite.c grdwrite.m
		$(MEX_BLD) $(FLAGS) grdwrite.c $(GMT) $(MEX_LIB)
