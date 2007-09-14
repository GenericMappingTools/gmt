#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.9 2007-09-14 01:13:39 remko Exp $
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

include ../../guru/gmtguru.macros.orig	# Overall default guru settings
sinclude ../../guru/gmtguru.macros	# Guru-specific settings determined by GURU

default:	all

mgd77_functions.h mgd77_functions.c:	mgd77netcdfhelper.sh mgd77_header.txt
		$(SHELL) mgd77netcdfhelper.sh

mgd77_codes.h mgd77_codes.txt:		mgd77_codes.sh
		$(SHELL) mgd77_codes.sh

include makefile
