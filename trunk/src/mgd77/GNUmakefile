#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.12 2008-03-05 02:11:58 guru Exp $
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

cm4field.c:	cm4field.f

cm4field.f:
	wget http://core2.gsfc.nasa.gov/CM/cm4field.f
	f2c cm4field.f

CM4:
	wget http://core2.gsfc.nasa.gov/CM/umdl.CM4
	wget http://core2.gsfc.nasa.gov/CM/NEW/Dst_all.wdc
	wget http://core2.gsfc.nasa.gov/CM/NEW/F107_mon.plt
