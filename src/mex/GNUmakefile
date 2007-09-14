#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.1 2007-09-14 01:13:39 remko Exp $
#
#	GNUakefile for GMT Version 4.x mex directory
#	
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular Makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#	Note that this GNUmakefile will automatically generate makefile
#	and continue.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		05-NOV-2006
#-------------------------------------------------------------------------------

default:	all

configure:	configure.in
		rm -f config.cache config.log config.status
		$(AUTOCONF)

makefile:	makefile.in configure
		$(SHELL) configure

include makefile
