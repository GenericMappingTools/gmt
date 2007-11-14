#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.11 2007-11-14 20:27:29 remko Exp $
#
#	Guru Makefile for GMT Version 4.x mgd77 directory
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular Makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		07-MAY-2007
#-------------------------------------------------------------------------------

include makefile
include ../manpages.mk

mgd77_functions.h mgd77_functions.c:	mgd77netcdfhelper.sh mgd77_header.txt
		$(SHELL) mgd77netcdfhelper.sh

mgd77_codes.h mgd77_codes.txt:		mgd77_codes.sh
		$(SHELL) mgd77_codes.sh
