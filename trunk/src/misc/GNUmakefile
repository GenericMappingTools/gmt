#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.10 2009-10-17 22:10:49 remko Exp $
#
#	Guru Makefile for GMT Version 4.x misc directory
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular Makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		24-SEP-2007
#-------------------------------------------------------------------------------

include Makefile
include ../manpages.mk

# Placing gmtpoly her for now while developing and testing

gmtpoly$(EXE):	$(GMTLIB) gmtpoly.o
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -lgmt $(NETCDF_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
		$(INSTALL) $@ $(bindir)
