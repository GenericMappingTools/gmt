#		$Id: Makefile,v 1.8 2008-04-19 23:52:01 guru Exp $
#
#	Makefile for gmt <--> octave/matlab conversion programs
#	Use with GMT 4.3.0 or later
#
# Note that if you have write permission in $MEX_XDIR and $MEX_MDIR
# then that is where install will place the mex and m files.  Otherwise,
# we will put them in $(bindir).
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

PROGS_M		= grdinfo.m grdread.m grdwrite.m
PROGS_MEX	= grdinfo.$(MEX_EXT) grdread.$(MEX_EXT) grdwrite.$(MEX_EXT)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS_MEX)

install:	all
		if [ -w $(MEX_XDIR) ] ; then \
			$(INSTALL) $(PROGS_MEX) $(MEX_XDIR); \
		else \
			$(INSTALL) $(PROGS_MEX) $(bindir); \
		fi
		if [ -w $(MEX_MDIR) ] ; then \
			$(INSTALL) $(PROGS_M) $(MEX_MDIR); \
		else \
			$(INSTALL) $(PROGS_M) $(bindir); \
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
		\rm -f .gmt* *% .mexrc.sh $(PROGS_MEX)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grdinfo.$(MEX_EXT):	$(GMT_H) $(GMTLIB) grdinfo.c grdinfo.m
		$(MEX_BLD) $(FLAGS) grdinfo.c $(GMT) $(MEX_LIB) 

grdread.$(MEX_EXT):	$(GMT_H) $(GMTLIB) grdread.c grdread.m
		$(MEX_BLD) $(FLAGS) grdread.c $(GMT) $(MEX_LIB) 

grdwrite.$(MEX_EXT):	$(GMT_H) $(GMTLIB) grdwrite.c grdwrite.m
		$(MEX_BLD) $(FLAGS) grdwrite.c $(GMT) $(MEX_LIB)
