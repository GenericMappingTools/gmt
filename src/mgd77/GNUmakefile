#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.15 2011-03-15 02:06:37 guru Exp $
#
#	Guru Makefile for GMT Version 5.x misc directory
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular Makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		30-SEP-2010
#-------------------------------------------------------------------------------

include Makefile
include $(GMTSRCDIR)manpages.mk

mgd77_functions.h mgd77_functions.c:	mgd77netcdfhelper.sh
	$(SHELL) mgd77netcdfhelper.sh

mgd77_codes.h mgd77_codes.txt:	mgd77_codes.sh
	$(SHELL) mgd77_codes.sh
