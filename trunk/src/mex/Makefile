#		$Id: Makefile,v 1.5 2007-10-08 21:26:08 remko Exp $
#
#	Makefile for gmt <--> matlab conversion programs
#	Use with GMT 4.x
#
# Note that if you have write permission in $MATLAB/toolbox/local
# then that is where install will place the files.  Otherwise, it
# will put them in $(bindir).
#----------------------------------------------------------------------------
#	The rest should should be ok. NO EDITING BEYOND HERE, unless your
#	Matlab setup was not auto-detected by the configure file
#	If so, please let us know what you needed to change.
#----------------------------------------------------------------------------

GMTSRCDIR	= ../
include $(GMTSRCDIR)makegmt.macros
include $(GMTSRCDIR)gmtalldeps.macros

.SUFFIXES:	.m .$(MATLAB_EXT)

MEX		= $(MATLAB)/bin/mex
MEXLIB		= -L$(MATLAB)/extern/lib/$(MATLAB_MEX) -l$(MATLAB_LIB)
GMT     	= -L.. -lgmt
FLAGS		= -I$(srcdir) -I$(MATLAB)/extern/include -I$(NETCDF)/include -O

PROGS_H		= grdinfo.m grdread.m grdwrite.m
PROGS_M		= grdinfo.$(MATLAB_EXT) grdread.$(MATLAB_EXT) grdwrite.$(MATLAB_EXT)

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS_M)

install:	all
		if [ -w $(MATLAB)/toolbox/local ] ; then \
			$(INSTALL) $(PROGS_M) $(PROGS_H) $(MATLAB)/toolbox/local; \
		else \
			$(INSTALL) $(PROGS_M) $(PROGS_H) $(bindir); \
		fi

uninstall:
		if [ -w $(MATLAB)/toolbox/local ] ; then \
			cd $(MATLAB)/toolbox/local; \
		else \
			cd $(bindir); \
		fi; \rm -f $(PROGS_M) $(PROGS_H)

install-man uninstall-man manpages:	;

spotless:	clean

clean:
		\rm -f .gmt* *% .mexrc.sh $(PROGS_M)

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

grdinfo.$(MATLAB_EXT):	$(GMT_H) $(GMTLIB) grdinfo.c grdinfo.m
		$(MEX) $(FLAGS) grdinfo.c $(GMT) $(CDF) $(MEXLIB) 

grdread.$(MATLAB_EXT):	$(GMT_H) $(GMTLIB) grdread.c grdread.m
		$(MEX) $(FLAGS) grdread.c $(GMT) $(CDF) $(MEXLIB) 

grdwrite.$(MATLAB_EXT):	$(GMT_H) $(GMTLIB) grdwrite.c grdwrite.m
		$(MEX) $(FLAGS) grdwrite.c $(GMT) $(CDF) $(MEXLIB)
