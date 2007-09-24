#	$Id: Makefile,v 1.45 2007-09-24 00:38:00 remko Exp $
#
#	Copyright (c) 1991-2007 by P. Wessel and W. H. F. Smith
#	See COPYING file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 of the License.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#		Makefile for GMT Version 4.x
#		GNU, Sys V, and BSD Compatible
#
#	Follow the instructions in this makefile to customize your setup.
#	Commands marked *** are optional
#	1. run configure in the main GMT directory.
#	2. make all
#	3. make suppl    ***
#	4. make examples ***
#
#	Then, if you have the necessary directory permissions:
#
#	5. make install-all
#
#	or any combination of
#
#	make install-gmt
#	make install-suppl
#	make install-data
#	make install-man
#	make install-www
#
#	When done, clean out directory with "make clean".  To clean
#	the entire build distribution, do "make cleandist"
#
#	To uninstall the installed files, try
#
#	make uninstall-all
#
#	or any combination of
#
#	make uninstall-gmt
#	make uninstall-suppl
#	make uninstall-data
#	make uninstall-man
#	make uninstall-www
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, Lab for Satellite Altimetry, NOAA
#
#	Date:		27-MAR-2007
#-------------------------------------------------------------------------------
#	Get Default Macros
#-------------------------------------------------------------------------------

include src/makegmt.macros	# GMT-specific settings determined by user & install_gmt

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
.PHONY:		all gmt suppl update gmtmacros \
		install uninstall install-all uninstall-all install-gmt uninstall-gmt \
		install-suppl uninstall-suppl install-data uninstall-data install-man uninstall-man \
		install-www uninstall-www examples run-examples clean spotless distclean insuppl

SUPPL	=	dbase gshhs imgsrc meca mex mgd77 mgg misc segyprogs spotter x2sys x_system xgrid
SUPPL_M	=	dbase imgsrc meca mgd77 mgg misc segyprogs spotter x2sys x_system

all:		gmt suppl

help::
		@grep '^#!' Makefile | cut -c3-
#!-------------------- MAKE HELP FOR GMT --------------------
#!
#!make <target>, where <target> can be:
#!
#!all           : Compile main source
#!suppl         : Compile supplements
#!install-gmt   : Compile & install main GMT programs
#!install-suppl : Compile & install supplements
#!install-data  : Install GMT data files
#!install-man   : Install man pages
#!install-all   : Compile & install everything, including data & man pages
#!examples      : Run examples
#!spotless      : Clean up and remove created files of all types
#!

install:	install-gmt install-suppl
uninstall:	uninstall-gmt uninstall-suppl

install-all:	install-gmt install-suppl install-data install-man install-www
uninstall-all:	uninstall-gmt uninstall-suppl uninstall-data uninstall-man uninstall-www

patch:
		bin/gmtpatch.sh 4

gmt:		gmtmacros
		cd src ; $(MAKE) all

install-gmt:	gmtmacros
		cd src ; $(MAKE) install

uninstall-gmt:
		cd src ; $(MAKE) uninstall

suppl:		gmtmacros
		cd src ; $(MAKE) libs
		$(MAKE) TARGET=all insuppl

install-suppl suppl-install:	gmtmacros
		$(MAKE) TARGET=install insuppl

gmtmacros:
		@if [ ! -s src/makegmt.macros ]; then \
			echo "src/makegmt.macros is empty - you must rerun configure in the main GMT directory"; \
			exit; \
		fi

uninstall-suppl:
		$(MAKE) TARGET=uninstall insuppl

install-data:
		if [ ! $(rootdir)/share = $(datadir) ]; then \
			mkdir -p $(datadir); \
			cp -r share/* share/.gmtdefaults_* $(datadir); \
		else \
			echo "Install share directory the same as distribution share directory - nothing copied"; \
		fi

uninstall-data:
		if [ ! $(rootdir)/share = $(datadir) ]; then \
			\rm -rf $(datadir); \
		else \
			echo "Install share directory the same as distribution share directory - nothing removed"; \
		fi

install-man uninstall-man:
		cd src ; $(MAKE) $@
		$(MAKE) TARGET=$@ insuppl

install-www:
		@if [ ! $(rootdir)/www = $(wwwdir) ]; then \
			mkdir -p $(wwwdir); \
			cp -r www/gmt $(wwwdir); \
		else \
			echo "Install www directory the same as distribution www directory - nothing copied"; \
		fi

uninstall-www:
		if [ ! $(rootdir)/www = $(wwwdir) ]; then \
			rm -rf $(wwwdir)/gmt; \
		else \
			echo "Install www directory the same as distribution www directory - nothing deleted"; \
		fi

run-examples:	examples
examples:
		@if [ -d examples ]; then \
			cd examples; \
			$(CSH) do_examples.$(CSH) $(bindir) $(libdir) $(NETCDF)/lib; \
			cd ..; \
		else \
			echo "examples directory not installed"; \
		fi

clean:
		$(MAKE) TARGET=$@ insuppl
		cd src ; $(MAKE) $@

spotless::
		rm -f config.cache config.status config.log
		$(MAKE) TARGET=$@ insuppl
		cd src ; $(MAKE) $@

distclean:	spotless

insuppl:
		@set -e ; for d in $(SUPPL); do \
			if [ -d src/$$d ] && [ ! -f src/$$d/.skip ]; then \
				echo "Making $(TARGET) in src/$$d"; \
				( cd src/$$d; $(MAKE) $(TARGET) ); \
			fi; \
		done
