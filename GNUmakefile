#-------------------------------------------------------------------------------
#  $Id$
#
#	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#		 Guru makefile for GMT Version 5.*
#			GNU make compatible
#
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	To be installed in top-level GMT directory and requires GNU make
#
#	New GMT gurus should create a file guru/gmtguru.macros with any settings
#	that are intended to overrule the DEFAULT SETTINGS specified below.
#
#	To ensure the very latest code, start with "make update".
#	To get the latest and make all, do "make install".
#	To wipe all, do "make spotless".
#	The gentler "make clean" will NOT remove code that is generated
#	by GMT scripts, like gmt_math.h and grdmath.c.
#
#	To run all the test dir scripts:	make tests
#	To test the examples:           	make extests
#	To test all documentation figs:		make doctests
#	To run all tests:			make alltests
#
#	To generate new archive:	make archive or make "version" e.g., make GMT4
#
#	Subtasks:
#
#	To compile/install GMT:		make all and/or make install
#	To create examples, animations:	make examples animations
#	To generate man pages:		make man
#	To generate documentation:	make docs
#	To generate tarfiles:		make tar_all
#
#	This must be done separately:
#	To tar all coastlines		make tar_gshhs
#
#	Author:	Paul Wessel, Remko Scharroo
#
#	Date:		1-JAN-2011
#
#-------------------------------------------------------------------------------
#	DEFAULT SETTINGS
#-------------------------------------------------------------------------------
# This section contains macros that you may want to override with your own
# settings to reflect your particular computer environment. You can best do this
# by creating coying guru/gmtguru.macros.orig to guru/gmtguru.macros. Then edit
# the latter.

GMTGURU	= guru/gmtguru.macros
include guru/gmtguru.macros.orig	# Default guru settings
sinclude $(GMTGURU)		# Guru-specific settings determined by GURU [Default is guru/gmtguru.macros]

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
.PHONY:		FILES man manpages webman webdoc pdfman docs \
		latest-config help update create prep prepare archive \
		tar_all tar_gshhs install suppl alltests \
		doctests extests tests ex examples animations svnclean

#-------------------------------------------------------------------------------
# FILES stands for all those files (makefiles, etc) that are not part of the SVN
# distribution but rather are created from more primitive forms.  Since those
# primitive forms may undergo modifications we must check for changes before
# compiling, making man pages, etc.  What follows is the rules for making these
# FILES from the SVN-distributed master files:
#-------------------------------------------------------------------------------

FILES =		src/config.mk share/conf/gmt.conf share/conf/gmt_SI.conf share/conf/gmt_US.conf \
		src/gmt_version.h src/gmt_notposix.h src/isogmt doc/GMT_version.tex

help::
		@grep '^#!' GNUmakefile | cut -c3-
#!----------------- MAKE HELP FOR GMT GURUS -----------------
#!
#!make <target>, where <target> can be:
#!
#!update        : Get the latest source via svn
#!alltests      : Run all tests and compare to originals
#!manpages      : Create manpages from text files
#!pdfman        : Create PDF version of manpages
#!pdfdocs       : Create PDF documentation
#!svnclean      : Cleanup the package to a nearly clean SVN checkout
#!prepare       : Create all files needed for a release
#!archive       : Build the release archives
#!

update:
		svn -q update

create:
# We make the GMT$(GMT_VERSION) link from scratch each time
		cd ..; rm -f GMT$(GMT_VERSION); $(LN_S) $(notdir $(PWD)) GMT$(GMT_VERSION)

GMT$(GMT_VERSION):	archive

prep prepare:	manpages webman pdfman webdoc pdfdocs

archive:	prepare create tar_all

manpages:	$(FILES)
		cd src ; $(MAKE) $@
		$(MAKE) TARGET=$@ $(SUPPL)

all:		$(FILES)
		cd src ; $(MAKE) $@

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------
alltests:	extests doctests tests

extests:
		@cd doc/examples ; $(MAKE) $@

doctests:
		@cd doc/scripts ; $(MAKE) $@

tests:
		@cd test ; $(MAKE) $@

cleantests:
		@cd doc/examples ; $(MAKE) clean
		@cd doc/scripts ; $(MAKE) clean
		@cd test ; $(MAKE) clean

#-------------------------------------------------------------------------------
# Cleaning
#-------------------------------------------------------------------------------
svnclean:
		$(SHELL) guru/svnclean.sh

clean spotless::
		cd doc ; $(MAKE) $@
		cd doc/examples ; $(MAKE) $@
		cd doc/scripts ; $(MAKE) $@
		cd test ; $(MAKE) $@

#-------------------------------------------------------------------------------
# For compatibility with previous versions
#-------------------------------------------------------------------------------
ex:		examples
webdoc:		
		cd doc ; $(MAKE) prep

#-------------------------------------------------------------------------------
# To check there are no files with DOS line-endings
#-------------------------------------------------------------------------------
DOS:
		$(SHELL) guru/DOS_finder.sh

gmtmacros examples animations FILES:		$(FILES)

fresh:
		rm -f $(FILES)

config:		src/config.mk

guru/gmtguru.macros:
		touch $@

# When doing spotless or TARGET=spotless, make sure a dummy src/config.mk exists

ifeq "$(findstring spotless,$(MAKECMDGOALS)$(TARGET))" "spotless"
$(FILES):
		touch $@
else
$(FILES):	guru/gmtguru.macros configure config.sub config.guess src/config.mk.in share/conf/gmt.conf.in \
		src/gmt_version.h.in src/gmt_notposix.h.in src/isogmt.in doc/GMT_version.tex.in
		rm -f config.cache config.log config.status
		./configure $(GMT_SHARED_LIBS) $(GMT_US) $(GMT_TRIANGLE) $(GMT_DEBUG) $(GMT_DIST) $(GMT_EXDIST) \
		$(GMT_NETCDF) $(GMT_SITE) $(GMT_MATLAB) $(GMT_OCTAVE) $(GMT_64) $(GMT_UNIVERSAL) $(GMT_OTHER)
		touch $(FILES)
endif

configure:	configure.ac
		$(AUTOCONF)

latest-config:
		curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD" -s -R -o config.sub
		curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD" -s -R -o config.guess

install-doc::	webman pdfman docs

docs pdfdocs:	$(FILES)
		cd doc ; $(MAKE) pdf
pdfman:		$(FILES)
		cd doc ; $(MAKE) man
webman:		$(FILES)
		cd doc ; $(MAKE) html

GSHHS_DIR	= gmt
get_coast:	get_gshhs
get_gshhs:	
		curl "ftp://$(FTPSITE)/$(GSHHS_DIR)/gshhs-$(GSHHS_VERSION).tar.bz2" -R -O
		bzip2 -dc gshhs-$(GSHHS_VERSION).tar.bz2 | tar -xvf -
		rm -f gshhs-$(GSHHS_VERSION).tar.bz2

# For CVS usage
get_gshhs_cvs:
		$(MAKE) GSHHS_DIR=pwessel/gshhs get_gshhs

#-------------------------------------------------------------------------------
#	TARRING OFF THE NEW VERSION
#-------------------------------------------------------------------------------

ftpdir:
		mkdir -p ftp

tar_all:	ftpdir
		echo "make gmt-$(GMT_VERSION).tar.bz2"
		grep -vh '#' guru/GMT_progs_files_{ascii,bin}.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		grep -vh '#' guru/GMT_share_files_{ascii,bin}.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' >> tmp.lis
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_www.lis >> tmp.lis
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_{animations,examples}.lis >> tmp.lis
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_tutorial.lis >> tmp.lis
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_suppl.lis >> tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/gmt-$(GMT_VERSION).tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/LICENSE.TXT
		grep -vh '#' guru/GMT_triangle.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/gmt-$(GMT_VERSION)-non-gpl.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/LICENSE.TXT
		rm -f tmp.lis

# The tar_win target is for GMT Developers building GMT on a Windows platform without configure
# and then building GMT installers with Inno Setup

tar_win:	ftpdir
		echo "make WINGMT$(GMT_VERSION)_win.tar.bz2"
		ls src/gmt_version.h share/conf/gmt.conf share/conf/gmtdefaults_?? \
			guru/*.iss guru/*.txt guru/*.bat | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/WINGMT$(GMT_VERSION)_win.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/LICENSE.TXT
		rm -f tmp.lis

# Note: coastline files now stored relative to share, instead of GMT/share

tar_gshhs:	ftpdir
		echo "make gshhs-$(GSHHS_VERSION).tar.bz2"
		COPYFILE_DISABLE=true tar -cjf ftp/gshhs-$(GSHHS_VERSION).tar.bz2 LICENSE.TXT \
			share/coast/binned_*.cdf -C src/coast/GSHHS+WDBII README.TXT

include Makefile

#-------------------------------------------------------------------------------
# Final cleanup (needs to run last)
#-------------------------------------------------------------------------------

spotless::
		rm -rf $(FILES) configure autom4te.cache
