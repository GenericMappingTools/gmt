#	$Id: Makefile,v 1.7 2001-09-18 02:02:28 pwessel Exp $
#
#	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
#		Makefile for GMT Version 4
#		GNU, Sys V, and BSD Compatible
#
#	Follow the instructions in this makefile to customize your setup.
#	Commands marked *** are optional
#	1. run configure in the main GMT directory.
#	2. make all
#	3. make suppl   ***
#	4. make run-examples ***
#
#	Then, if you have the necessary directory permissions:
#
#	5. make install-all
#
#	or any combination of
#
#	make install
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
#	make uninstall
#	make uninstall-suppl
#	make uninstall-data
#	make uninstall-man
#	make uninstall-www
#
#	Authors:	Paul Wessel, SOEST, U. of Hawaii
#			Walter H. F. Smith, Lab for Satellite Altimetry, NOAA
#
#	Date:		18-SEP-2001
#-------------------------------------------------------------------------------
#	Get Default Macros
#-------------------------------------------------------------------------------
#

include src/makegmt.macros	# GMT-specific settings determined by user & install_gmt

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

SUPPL	=	cps dbase gshhs imgsrc meca mex mgg misc segyprogs spotter x2sys x_system xgrid
SUPPL_M	=	cps dbase imgsrc meca mgg misc segyprogs spotter x2sys x_system

all:		gmt suppl

install-all:	install install-suppl install-data install-man install-www
uninstall-all:	uninstall uninstall-suppl uninstall-data uninstall-man uninstall-www

gmt:		gmtmacros
		cd src; \
		$(MAKE) all; \
		cd ..

install:	gmt
		cd src; \
		$(MAKE) install; \
		cd ..

uninstall:	gmt
		cd src; \
		$(MAKE) uninstall; \
		cd ..

suppl:		gmtmacros mex_config xgrid_config
		for d in $(SUPPL); do \
			if [ -d src/$$d ] && [ ! -f src/$$d/.skip ]; then \
				cd src/$$d; \
				$(MAKE) all; \
				cd ../..; \
			fi; \
		done

install-suppl:	suppl
		for d in $(SUPPL); do \
			if [ -d src/$$d ] && [ ! -f src/$$d/.skip ]; then \
				cd src/$$d; \
				$(MAKE) install; \
				cd ../..; \
			fi; \
		done

mex_config:	
		if [ ! -f src/mex/makefile ]; then \
			cd src/mex; \
			\rm -f config.{cache,log,status}; \
			./configure; \
		fi
		
xgrid_config:	
		if [ ! -f src/xgrid/makefile ]; then \
			cd src/xgrid; \
			\rm -f config.{cache,log,status}; \
			./configure; \
		fi

gmtmacros:	
		if [ `cat src/makegmt.macros | wc -c` = 0 ]; then \
			echo "src/makegmt.macros is empty - you must rerun configure in the main GMT directory"; \
			exit; \
		fi

uninstall-suppl:
		for d in $(SUPPL); do \
			if [ -d src/$$d ] && [ ! -f src/$$d/.skip ]; then \
				cd src/$$d; \
				$(MAKE) uninstall; \
				cd ../..; \
			fi; \
		done

install-data:
		if [ ! $(rootdir)/share = $(datadir) ]; then \
			mkdir -p $(datadir); \
			cp -r share/* $(datadir); \
			cp -f share/.gmtdefaults_* $(datadir); \
		else \
			echo "Install share directory the same as distribution share directory - nothing copied"; \
		fi

uninstall-data:
		if [ ! $(rootdir)/share = $(datadir) ]; then \
			\rm -r -f $(datadir); \
		else \
			echo "Install share directory the same as distribution share directory - nothing removed"; \
		fi


install-man:
		\rm -f manjob.sh
		for d in $(SUPPL_M); do \
			if [ -d src/$$d ] ; then \
				cd src/$$d; \
				for f in *.man; do \
					\cp $$f $(rootdir)/man/manl; \
					echo "\\mv $$f $$f" | sed -e 's/.man$$/.l/g' >> $(rootdir)/manjob.sh; \
				done; \
				cd ../..; \
			fi; \
		done
		if [ -f manjob.sh ]; then \
			cd man/manl; \
			$(SHELL) $(rootdir)/manjob.sh; \
			\rm -f $(rootdir)/manjob.sh; \
		fi
		if [ -f manuninstall.sh ]; then \
			\rm -f $(rootdir)/manuninstall.sh; \
		fi
		if [ ! $(rootdir)/man/manl = $(mandir)/man$(mansection) ]; then \
			mkdir -p $(mandir)/man$(mansection); \
			\cp man/manl/*.l $(mandir)/man$(mansection); \
			cd $(mandir)/man$(mansection); \
			for f in *.l; do \
				echo "\\mv $$f $$f" | sed -e 's/.l$$/.$(mansection)/g' >> $(rootdir)/manjob.sh; \
				echo "\\rm -f $$f"  | sed -e 's/.l$$/.$(mansection)/g' >> $(rootdir)/manuninstall.sh; \
			done; \
			$(SHELL) $(rootdir)/manjob.sh; \
			\rm -f $(rootdir)/manjob.sh; \
			cd $(rootdir); \
		else \
			echo "Install man directory the same as distribution man directory - nothing copied"; \
		fi


uninstall-man:
		if [ ! $(rootdir)/man/manl = $(mandir)/man$(mansection) ]; then \
			cd $(mandir)/man$(mansection); \
			$(SHELL) $(rootdir)/manuninstall.sh; \
			\rm -f $(rootdir)/manuninstall.sh; \
			cd $(rootdir); \
		else \
			echo "Install man directory the same as distribution man directory - nothing deleted"; \
		fi


install-www:
		for d in $(SUPPL_M); do \
			if [ -d src/$$d ] ; then \
				\cp src/$$d/*.html $(rootdir)/www/gmt/doc/html; \
			fi; \
		done
		if [ ! $(rootdir)/www = $(wwwdir) ]; then \
			mkdir -p $(wwwdir); \
			\cp -r www/gmt $(wwwdir); \
		else \
			echo "Install www directory the same as distribution www directory - nothing copied"; \
		fi

uninstall-www:
		if [ ! $(rootdir)/www = $(wwwdir) ]; then \
			\rm -r -f $(wwwdir)/gmt; \
		else \
			echo "Install www directory the same as distribution www directory - nothing deleted"; \
		fi

install-wrapper:
		if [ ! $(rootdir)/bin = $(wrapbindir) ]; then \
			$(INSTALL) src/GMT $(wrapbindir); \
		else \
			echo "Install wrapper bin directory the same as distribution bin directory - nothing installed"; \
		fi
		if [ ! $(rootdir)/man = $(wrapmandir) ]; then \
			\cp man/manl/GMT.l $(wrapmandir)/man$(mansection); \
		else \
			echo "Install wrapper man directory the same as distribution man directory - nothing installed"; \
		fi
		
run-examples:
		if [ -d examples ]; then \
			cd examples; \
			$(CSH) do_examples.$(CSH) $(bindir) $(libdir); \
			cd ..; \
		else \
			echo "examples directory not installed"; \
		fi

clean:		clean-suppl
		cd src; $(MAKE) clean

clean-suppl:
		for d in $(SUPPL); do \
			if [ -d src/$$d ] && [ ! -f src/$$d/.skip ]; then \
				cd src/$$d; \
				$(MAKE) clean; \
				cd ../..; \
			fi; \
		done

spotless:	clean
		rm -f config.cache config.status config.log
		cd src; $(MAKE) spotless; \
		for d in $(SUPPL); do \
			if [ -d src/$$d ] && [ ! -f src/$$d/.skip ]; then \
				cd src/$$d; \
				$(MAKE) spotless; \
				cd ../..; \
			fi; \
		done

distclean:	spotless
