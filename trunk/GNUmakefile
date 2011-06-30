#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.95 2011-06-30 20:16:28 remko Exp $
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
#	Once done, it is reasonable to first try "make site".
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
#	Subtasks (done by make archive and make site):
#
#	To compile/link GMT:		make all and/or make install
#	To create examples:		make examples
#	To generate man pages:		make man
#	To generate documentation:	make docs
#
#	Other subtasks: (done by make archive):
#
#	To generate tarfiles:		make tar_all
#	To tar source:			make tar_progs
#	To tar shared data		make tar_share
#	To tar docs			make tar_doc
#	To tar supplements		make tar_suppl
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
.PHONY:		FILES man manpages webman webdoc pdfman docs prep_suppl \
		latest-config help update create newsite usable site archive \
		tar_all tar_gshhs installl suppl alltests \
		doctests extests tests ex examples animations cvsclean

help::
		@grep '^#!' GNUmakefile | cut -c3-
#!----------------- MAKE HELP FOR GMT GURUS -----------------
#!
#!make <target>, where <target> can be:
#!
#!update        : Get the latest source via cvs
#!alltests      : Run all tests and compare to originals
#!manpages      : Create manpages from text files
#!usable        : Install software, data, manpages and run examples & animations
#!pdfman        : Create PDF version of manpages
#!pdfdocs       : Create PDF documentation
#!site          : Complete install, incl documentation and web pages
#!cvsclean      : Cleanup the package to a nearly clean CVS checkout
#!archive       : Build the release archives
#!

update:
		cvs -q update -Pd

create:
# We make the GMT$(GMT_VERSION) link from scratch each time
		cd ..; rm -f GMT$(GMT_VERSION); $(LN_S) $(notdir $(PWD)) GMT$(GMT_VERSION)

GMT$(GMT_VERSION):	archive

newsite:	get_gshhs site

usable:		install install-data install-man examples animations

site:		usable install-doc

archive:	site create tar_all

manpages:	$(FILES)
		cd src ; $(MAKE) $@
		$(MAKE) TARGET=$@ insuppl

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
cvsclean:
		$(SHELL) guru/cvsclean.sh

clean spotless::
		cd doc ; $(MAKE) $@
		cd doc/examples ; $(MAKE) $@
		cd doc/scripts ; $(MAKE) $@
		cd test ; $(MAKE) $@

#-------------------------------------------------------------------------------
# For compatibility with previous versions
#-------------------------------------------------------------------------------
ex:		examples
webdoc:		;

#-------------------------------------------------------------------------------
# To check there are no files with DOS line-endings
#-------------------------------------------------------------------------------
DOS:
		$(SHELL) guru/DOS_finder.sh

#-------------------------------------------------------------------------------
# FILES stands for all those files (makefiles, etc) that are not part of the CVS
# distribution but rather are created from more primitive forms.  Since those
# primitive forms may undergo modifications we must check for changes before
# compiling, making man pages, etc.  What follows is the rules for making these
# FILES from the CVS-distributed master files:
#-------------------------------------------------------------------------------

FILES =		src/config.mk share/conf/gmt.conf share/conf/gmt_SI.conf share/conf/gmt_US.conf \
		src/gmt_version.h doc/GMT_version.tex

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
$(FILES):	guru/gmtguru.macros configure config.sub config.guess src/gmt_notposix.h.in \
		src/config.mk.in share/conf/gmt.conf.in src/gmt_version.h.in doc/GMT_version.tex.in
		rm -f config.cache config.log config.status
		./configure $(GMT_SHARED_LIBS) $(GMT_US) $(GMT_TRIANGLE) $(GMT_DEBUG) $(GMT_DIST) $(GMT_EXDIST) \
		$(GMT_NETCDF) $(GMT_SITE) $(GMT_MATLAB) $(GMT_OCTAVE) $(GMT_64) $(GMT_UNIVERSAL) $(GMT_OTHER)
endif

configure:	configure.ac
		$(AUTOCONF)

latest-config:
		curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD" -s -R -o config.sub
		curl "http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD" -s -R -o config.guess

docs pdfdocs:	$(FILES)
		cd doc ; $(MAKE) pdf
pdfman:		$(FILES)
		cd doc ; $(MAKE) man
webman:		$(FILES)
		cd doc ; $(MAKE) html

prep_suppl:	clean config

GSHHS_DIR	= gmt
get_coast:	get_gshhs
get_gshhs:	get_gshhs_coast get_gshhs_high get_gshhs_full
get_gshhs_%:
		curl "ftp://$(FTPSITE)/$(GSHHS_DIR)/GSHHS$(GSHHS_VERSION)_$*.tar.bz2" -R -O
		bzip2 -dc GSHHS$(GSHHS_VERSION)_$*.tar.bz2 | tar -xvf -
		rm -f GSHHS$(GSHHS_VERSION)_$*.tar.bz2

# For CVS usage
get_gshhs_cvs:
		$(MAKE) GSHHS_DIR=pwessel/gshhs get_gshhs

#-------------------------------------------------------------------------------
#	TARRING OFF THE NEW VERSION
#-------------------------------------------------------------------------------

tar_all:	tar_progs tar_share tar_doc tar_suppl
		@echo " "
		@echo "Completed tarring off entire archive"

tar_gshhs:	tar_coast tar_full tar_high

ftpdir:
		mkdir -p ftp

tar_progs tar_src:	ftpdir
		echo "make GMT$(GMT_VERSION)_src.tar.bz2"
		grep -vh '#' guru/GMT_progs_files_{ascii,bin}.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/GMT$(GMT_VERSION)_src.tar.bz2 -C .. -T tmp.lis
		echo "make GMT$(GMT_VERSION)_triangle.tar.bz2"
		grep -vh '#' guru/GMT_triangle.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/GMT$(GMT_VERSION)_triangle.tar.bz2 -C .. -T tmp.lis
		rm -f tmp.lis

tar_share:	ftpdir
		echo "make GMT$(GMT_VERSION)_share.tar.bz2"
		grep -vh '#' guru/GMT_share_files_{ascii,bin}.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/GMT$(GMT_VERSION)_share.tar.bz2 -C .. -T tmp.lis
		rm -f tmp.lis

tar_doc:	ftpdir
		echo "make GMT$(GMT_VERSION)_web.tar.bz2"
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_{animations,examples,tutorial,www}.lis > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/GMT$(GMT_VERSION)_doc.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/LICENSE.TXT
		rm -f tmp.lis

tar_suppl:	ftpdir
		echo "make GMT$(GMT_VERSION)_suppl.tar.bz2"
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_suppl.lis > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/GMT$(GMT_VERSION)_suppl.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/LICENSE.TXT
		rm -f tmp.lis

# The tar_win target is for GMT Developers building GMT on a Windows platform without configure
# and then building GMT installers with Inno Setup

tar_win:	ftpdir
		echo "make WINGMT$(GMT_VERSION)_win.tar.bz2"
		ls src/gmt_version.h share/conf/gmt.conf share/conf/gmt_??.conf \
			guru/*.iss guru/*.txt guru/*.bat | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		COPYFILE_DISABLE=true tar -cjf ftp/WINGMT$(GMT_VERSION)_win.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/LICENSE.TXT
		rm -f tmp.lis

# Note: coastline files now stored relative to share, instead of GMT/share

tar_%:	ftpdir
		echo "make GSHHS$(GSHHS_VERSION)_$*.tar.bz2"
		if [ "$*" == "coast" ]; then suf=cli; else suf=`echo $*|cut -c1`; fi; \
			COPYFILE_DISABLE=true tar -cjf ftp/GSHHS$(GSHHS_VERSION)_$*.tar.bz2 LICENSE.TXT \
			share/coast/binned_*_[$$suf].cdf -C src/coast/GSHHS+WDBII README.TXT

include Makefile

#-------------------------------------------------------------------------------
# Final cleanup (needs to run last)
#-------------------------------------------------------------------------------

spotless::
		rm -rf src/config.mk src/gmt_version.h configure autom4te.cache
