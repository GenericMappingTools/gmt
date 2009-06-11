#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.7 2009-06-11 05:04:46 guru Exp $
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

include makefile
include ../manpages.mk

# Placing gmtpoly her for now while developing and testing

gmtpoly$(EXE):	$(GMTLIB) gmtpoly.o
		$(CC) $(LDFLAGS) $(@:.exe=).o -L.. -lgmt $(NC_LIBS) $(LIBS) -o $@
		$(COMPRESS) $@
