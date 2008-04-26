#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.29 2008-04-26 05:02:37 guru Exp $
#
#		 Guru makefile for GMT Version 4
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
#	To tar c,l,i coastlines		make tar_coast
#	To tar tutorial			make tar_tut
#	To tar web docs			make tar_web
#	To tar pdf files		make tar_pdf
#	To tar scripts			make tar_scripts
#	To tar supplements		make tar_suppl
#
#	These two must be done separately:
#	To tar full coastlines		make tar_full
#	To tar high coastlines		make tar_high
#
#	To generate zipfiles:		make zip_all
#	To zip source:			make zip_progs
#	To zip executables:		make zip_exe
#	To zip shared data		make zip_share
#	To zip c,l,i coastlines		make zip_coast
#	To zip tutorial			make zip_tut
#	To zip web docs			make zip_web
#	To zip pdf files		make zip_pdf
#	To zip scripts			make zip_scripts
#	To zip supplements		make zip_suppl
#	To zip supplements executables	make zip_suppl_exe
#
#	These two must be done separately:
#	To zip full coastlines		make zip_full
#	To zip high coastlines		make zip_high
#
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		13-SEP-2007
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
.PHONY:		FILES man manpages webman webdoc pdfman docs prep_suppl get_coast get_high get_full \
		latest-config help update create newsite usable site archive \
		tar_all zip_all zip_bin zip_src \
		full high tar_full zip_full tar_high zip_high installl suppl alltests \
		doctests extests tests ex examples cvsclean

help::
		@grep '^#!' GNUmakefile | cut -c3-
#!----------------- MAKE HELP FOR GMT GURUS -----------------
#!
#!make <target>, where <target> can be:
#!
#!update        : Get the latest source via cvs
#!manpages      : Create manpages from text files
#!usable        : Install-all and run examples
#!pdfman        : Install PDF version of manpages
#!pdfdocs       : Install PDF documentation
#!docs          : Install PDF and HTML documentation
#!site          : Complete install, incl documentation and web pages
#!cvsclean      : Cleanup the package to a nearly clean CVS checkout
#!archive       : Build the release archives
#!

update:
		cvs -q update -Pd

create:
# We make the GMT$(GMT_VERSION) link from scratch each time
		cd ..; rm -f GMT$(GMT_VERSION); $(LN_S) `basename $(PWD)` GMT$(GMT_VERSION)

GMT$(GMT_VERSION):	archive

newsite:	get_coast get_high get_full site

usable:		install-all examples

site:		usable webman pdfman docs

archive:	site create tar_all zip_all

manpages:	FILES
		cd src ; $(MAKE) $@
		$(MAKE) TARGET=$@ insuppl

all:		FILES
		cd src ; $(MAKE) $@

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------
alltests:	extests tests doctests

extests:
		@cd examples ; sh run_example_tests.sh

tests:
		@cd test ; sh run_gmt_tests.sh

doctests:	
		@cd doc ; $(MAKE) tests

#-------------------------------------------------------------------------------
# Cleaning
#-------------------------------------------------------------------------------
cvsclean:
		$(SHELL) guru/cvsclean.sh

clean spotless::
		cd doc ; $(MAKE) $@

#-------------------------------------------------------------------------------
# For compatibility with previous versions
#-------------------------------------------------------------------------------
ex:		examples
webdoc:		;

#-------------------------------------------------------------------------------
# FILES stands for all those files (makefiles, etc) that are not part of the CVS
# distribution but rather are created from more primitive forms.  Since those
# primitive forms may undergo modifications we must check for changes before
# compiling, making man pages, etc.  What follows is the rules for making these
# FILES from the CVS-distributed master files:
#-------------------------------------------------------------------------------

FILES =		configure src/makegmt.macros share/conf/gmt.conf share/conf/.gmtdefaults_SI share/conf/.gmtdefaults_US

gmtmacros FILES:		$(FILES)
examples:	FILES

fresh:
		rm -f $(FILES)

config:		src/makegmt.macros

share/conf/gmt.conf: 	share/conf/gmt.conf.orig 
		cp -f share/conf/gmt.conf.orig share/conf/gmt.conf

share/conf/.gmtdefaults_SI:	share/conf/.gmtdefaults_SI.orig
		cp -f share/conf/.gmtdefaults_SI.orig share/conf/.gmtdefaults_SI

share/conf/.gmtdefaults_US:	share/conf/.gmtdefaults_US.orig
		cp -f share/conf/.gmtdefaults_US.orig share/conf/.gmtdefaults_US

guru/gmtguru.macros:
		touch $@

# When doing spotless or TARGET=spotless, make sure a dummy src/makegmt.macros exists

ifeq "$(findstring spotless,$(MAKECMDGOALS)$(TARGET))" "spotless"
src/makegmt.macros:
		touch $@
else
src/makegmt.macros:	guru/gmtguru.macros src/makegmt.macros.in configure config.sub config.guess
		rm -f config.cache config.log config.status
		./configure $(GMT_SHARED_LIBS) $(GMT_US) $(GMT_TRIANGLE) $(GMT_DEBUG) \
			$(GMT_DIST) $(GMT_EXDIST) $(GMT_NETCDF) $(GMT_SITE) $(GMT_MATLAB) $(GMT_64) $(GMT_UNIVERSAL)
endif

configure:	configure.ac
		$(AUTOCONF)

latest-config:
		curl http://cvs.savannah.gnu.org/viewvc/*checkout*/config/config/config.sub --remote-name --silent
		curl http://cvs.savannah.gnu.org/viewvc/*checkout*/config/config/config.guess --remote-name --silent
		curl http://cvs.savannah.gnu.org/viewvc/*checkout*/config/config/config.rpath --remote-name --silent

webman: 	guru/webman.sh
		$(SHELL) guru/webman.sh -s

pdfman: 	guru/pdfman.sh
		$(SHELL) guru/pdfman.sh -s

docs:		FILES
		cd doc ; $(MAKE) install-all

pdfdocs:	FILES
		cd doc ; $(MAKE) install-pdf

prep_suppl:	clean config

get_coast get_high get_full:
#		Set-up ftp command & get coast file
		echo "Getting coasts/rivers (GSHHS$(GSHHS_VERSION)_$(subst get_,,$@)) by anonymous ftp (be patient)..."
		echo "user anonymous $(USER)@" > ftp.job
		echo "cd gmt/$(firstword $(subst ., ,$(GMT_VERSION)))" >> ftp.job
		echo "binary" >> ftp.job
		echo "get GSHHS$(GSHHS_VERSION)_$(subst get_,,$@).tar.bz2" >> ftp.job
		echo "quit" >> ftp.job
		ftp -dn $(FTPSITE) < ftp.job || ( echo "ftp failed - try again later"; exit )
		bzip2 -dc GSHHS$(GSHHS_VERSION)_$(subst get_,,$@).tar.bz2 | tar xvf -
		rm -f GSHHS$(GSHHS_VERSION)_$(subst get_,,$@).tar.bz2
		rm -f ftp.job
		echo "done"

#-------------------------------------------------------------------------------
#	TARRING OFF THE NEW VERSION
#-------------------------------------------------------------------------------

tar_all:	tar_progs tar_share tar_coast tar_tut tar_web tar_pdf tar_scripts tar_suppl
		@echo " "
		@echo "Completed tarring off entire archive"

zip_all:	zip_src zip_bin
		@echo " "
		@echo "Completed zipping off entire archive"

zip_bin:	zip_exe zip_suppl_exe

zip_src:	zip_share zip_coast zip_tut zip_web zip_pdf zip_scripts zip_suppl

full:		tar_full zip_full

high:		tar_high zip_high

ftpdir:
		mkdir -p ftp

tar_progs tar_src:	ftpdir
		echo "make GMT$(GMT_VERSION)_src.tar.bz2"
		grep -vh '#' guru/GMT_progs_files_{ascii,bin}.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_src.tar.bz2 -C .. -T tmp.lis
		echo "make GMT$(GMT_VERSION)_triangle.tar.bz2"
		grep -vh '#' guru/GMT_triangle.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_triangle.tar.bz2 -C .. -T tmp.lis
		rm -f tmp.lis

tar_share:	ftpdir
		echo "make GMT$(GMT_VERSION)_share.tar.bz2"
		grep -vh '#' guru/GMT_share_files_{ascii,bin}.lis | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_share.tar.bz2 -C .. -T tmp.lis
		rm -f tmp.lis

zip_share:	ftpdir
		rm -f ftp/GMT_share.zip
		grep -vh '#' guru/GMT_share_files_ascii.lis | sed -e 's:^:GMT/:' > asc.lis
		grep -vh '#' guru/GMT_share_files_bin.lis   | sed -e 's:^:GMT/:' > bin.lis
		echo "make GMT_share.zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_share.zip `cat GMT/asc.lis`)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_share.zip `cat GMT/bin.lis`)
		rm -f asc.lis bin.lis

zip_exe:	ftpdir
		if [ -e "bin/gmt.dll" ]; then \
		  rm -f ftp/GMT_exe.zip; \
		  echo "make GMT_exe.zip"; \
		  grep -v '^#' guru/GMT_programs.lis  | awk '{printf "GMT/bin/%s.exe\n", $$1}' > bin.lis; \
		  echo "GMT/bin/libnetcdf.dll" >> bin.lis; \
		  chmod +x bin/*.dll; \
		  (cd ..; zip -r -9 -q -l GMT/ftp/GMT_exe.zip GMT/COPYING GMT/README.WIN32 \
		     GMT/src/gmtenv.bat GMT/share/conf/gmt.conf); \
		  (cd ..; zip -r -9 -q    GMT/ftp/GMT_exe.zip `cat GMT/bin.lis` \
		     GMT/bin/{gmt,psl}.dll GMT/lib/{gmt,psl}.{lib,exp}); \
		  rm -f asc.lis bin.lis; \
		fi

tar_tut:	ftpdir
		echo "make GMT$(GMT_VERSION)_tut.tar.bz2"
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_tutorial.lis > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_tut.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/COPYING
		rm -f tmp.lis

zip_tut:	ftpdir
		rm -f ftp/GMT_tut.zip
		echo "make GMT_tut.zip"
		sed -e 's:^:GMT/:' guru/GMT_tutorial.lis | grep -v '\.nc$$' > asc.lis
		sed -e 's:^:GMT/:' guru/GMT_tutorial.lis | grep '\.nc$$' > bin.lis
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_tut.zip GMT/COPYING `cat GMT/asc.lis`)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_tut.zip `cat GMT/bin.lis`)
		rm -f asc.lis bin.lis

tar_web:	ftpdir
		echo "make GMT$(GMT_VERSION)_web.tar.bz2"
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_www.lis > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_web.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/COPYING \
			GMT$(GMT_VERSION)/www/gmt/doc/html/GMT_{Docs,Tutorial}
		rm -f tmp.lis

zip_web:	ftpdir
		rm -f ftp/GMT_web.zip
		echo "make GMT_web.zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_web.zip GMT/COPYING \
			GMT/www/gmt/gmt_{man,services,suppl}.html GMT/www/gmt/doc/html/*.html \
			GMT/www/gmt/doc/html/GMT_{Docs,Tutorial}/*.html)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_web.zip GMT/www/gmt/gmt_back.gif \
			GMT/www/gmt/doc/html/GMT_{Docs,Tutorial}/*.png)

tar_pdf:	ftpdir
		echo "make GMT$(GMT_VERSION)_pdf.tar.bz2"
		ls www/gmt/doc/pdf/GMT_*.pdf | sed -e 's:^:GMT$(GMT_VERSION)/:' > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_pdf.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/COPYING
		rm -f tmp.lis

zip_pdf:	ftpdir
		rm -f ftp/GMT_pdf.zip
		echo "make GMT_pdf.zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_pdf.zip GMT/COPYING)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_pdf.zip GMT/www/gmt/doc/pdf/GMT_*.pdf)

tar_scripts:	ftpdir
		echo "make GMT$(GMT_VERSION)_scripts.tar.bz2"
		rm -f examples/ex??/*.ps examples/ex??/*% examples/ex??/*.txt examples/ex??/.gmt*
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_examples.lis > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_scripts.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/COPYING
		rm -f tmp.lis

zip_scripts:	ftpdir
		rm -f ftp/GMT_scripts.zip
		echo "make GMT_scripts.zip"
		sed -e 's:^:GMT/:' guru/GMT_examples.lis | egrep -v '\.nc$$|\.bz2$$|\.ras$$' > asc.lis
		sed -e 's:^:GMT/:' guru/GMT_examples.lis | egrep '\.nc$$|\.bz2$$|\.ras$$' > bin.lis
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_scripts.zip GMT/COPYING `cat GMT/asc.lis`)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_scripts.zip `cat GMT/bin.lis`)
		rm -f asc.lis bin.lis

tar_suppl:	ftpdir
		echo "make GMT$(GMT_VERSION)_suppl.tar.bz2"
		sed -e 's:^:GMT$(GMT_VERSION)/:' guru/GMT_suppl.lis > tmp.lis
		tar -cjf ftp/GMT$(GMT_VERSION)_suppl.tar.bz2 -C .. -T tmp.lis GMT$(GMT_VERSION)/COPYING
		rm -f tmp.lis

zip_suppl:	ftpdir
		rm -f ftp/GMT_suppl.zip
		echo "make GMT_suppl.zip"
		sed -e 's:^:GMT/:' guru/GMT_suppl.lis | egrep -v '\.man$$|\.html|xgrid|configure' > asc.lis
		grep '\.html$$' guru/GMT_suppl.lis | awk -F/ '{printf "GMT/www/gmt/doc/html/%s\n", $$NF}' >> asc.lis
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_suppl.zip GMT/COPYING `cat GMT/asc.lis`)
		rm -f asc.lis

zip_suppl_exe:	ftpdir
		if [ -e "bin/spotter.dll" ]; then \
		  rm -f ftp/GMT_suppl_exe.zip; \
		  echo "make GMT_suppl_exe.zip"; \
		  sed -e 's:\.: :g' -e 's:/: :g' guru/GMT_suppl.lis \
			| awk '{if ($$NF == "c") printf "GMT/bin/%s.exe\n", $$(NF-1)}' \
			| egrep -vi 'lib|sub|util|distaz|grdread|grdwrite|grdinfo|gmt_|x2sys.exe|xgrid|configure' \
		        | sort -u > bin.lis; \
		  chmod +x bin/*.dll; \
		  (cd ..; zip -r -9 -q -l GMT/ftp/GMT_suppl_exe.zip GMT/COPYING); \
		  (cd ..; zip -r -9 -q    GMT/ftp/GMT_suppl_exe.zip `cat GMT/bin.lis` \
		     GMT/bin/{gmt_mgg,mgd77,spotter,x2sys}.dll \
		     GMT/lib/{gmt_mgg,mgd77,spotter,x2sys}.{lib,exp}); \
		  rm -f bin.lis; \
		fi

#	Note: coastline files now stored relative to share, instead of GMT/share

tar_coast tar_high tar_full:	ftpdir
		echo "make GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar.bz2"
		if [ "$(subst tar_,,$@)" == "coast" ]; then suf=cli; else suf=`echo $@|cut -c5`; fi; \
			tar -cjf ftp/GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar.bz2 COPYING \
			share/coast/binned_*_[$$suf].cdf -C src/gshhs README.gshhs

zip_coast zip_high zip_full:	ftpdir
		rm -f ftp/GSHHS_$(subst zip_,,$@).zip
		echo "make GSHHS_$(subst zip_,,$@).zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GSHHS_$(subst zip_,,$@).zip GMT/COPYING)
		if [ "$(subst zip_,,$@)" == "coast" ]; then suf=cli; else suf=`echo $@|cut -c5`; fi; \
		   (cd ..; zip -r -9 -q    GMT/ftp/GSHHS_$(subst zip_,,$@).zip GMT/share/coast/*_[$$suf].cdf)

#	The zip_dist target is for GMT Developers to move everything onto a Windows platform
#	for building GMT installers with Inno Setup

zip_dist:
	echo "make GMT_dist.zip"
	rm -f ftp/GMT_dist.zip
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip GMT/COPYING)
	grep -vh '#' guru/GMT_progs_files_{ascii,bin}.lis | sed -e 's:^:GMT/:' > asc.lis
	grep -vh '#' guru/GMT_triangle.lis | sed -e 's:^:GMT/:' >> asc.lis
	ls src/gmt_version.h | sed -e 's:^:GMT/:' >> asc.lis
	ls guru/*.iss guru/*.txt | sed -e 's:^:GMT/:' >> asc.lis
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip `cat GMT/asc.lis`)
	grep -vh '#' guru/GMT_share_files_ascii.lis | sed -e 's:^:GMT/:' > asc.lis
	grep -vh '#' guru/GMT_share_files_bin.lis   | sed -e 's:^:GMT/:' > bin.lis
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip `cat GMT/asc.lis`)
	(cd ..; zip -r -9 -q    GMT/ftp/GMT_dist.zip `cat GMT/bin.lis`)
	grep -v '\.nc$$' guru/GMT_tutorial.lis | sed -e 's:^:GMT/:' > asc.lis
	grep '\.nc$$' guru/GMT_tutorial.lis | sed -e 's:^:GMT/:' > bin.lis
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip `cat GMT/asc.lis`)
	(cd ..; zip -r -9 -q    GMT/ftp/GMT_dist.zip `cat GMT/bin.lis`)
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip  \
		GMT/www/gmt/gmt_{man,services,suppl}.html GMT/www/gmt/doc/html/*.html \
		GMT/www/gmt/doc/html/GMT_{Docs,Tutorial}/*.html)
	(cd ..; zip -r -9 -q    GMT/ftp/GMT_dist.zip GMT/www/gmt/gmt_back.gif \
		GMT/www/gmt/doc/html/GMT_{Docs,Tutorial}/*.png)
	(cd ..; zip -r -9 -q    GMT/ftp/GMT_dist.zip GMT/www/gmt/doc/pdf/GMT_*.pdf)
	egrep -v '\.nc$$|\.bz2$$|\.ras$$' guru/GMT_examples.lis | sed -e 's:^:GMT/:' > asc.lis
	egrep '\.nc$$|\.bz2$$|\.ras$$' guru/GMT_examples.lis | sed -e 's:^:GMT/:' > bin.lis
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip `cat GMT/asc.lis`)
	(cd ..; zip -r -9 -q    GMT/ftp/GMT_dist.zip `cat GMT/bin.lis`)
	egrep -v '\.man$$|\.html|xgrid|configure' guru/GMT_suppl.lis | sed -e 's:^:GMT/:' > asc.lis
	grep '\.html$$' guru/GMT_suppl.lis | sed -e 's:^:GMT/:' >> asc.lis
	(cd ..; zip -r -9 -q -l GMT/ftp/GMT_dist.zip `cat GMT/asc.lis`)
	(cd ..; zip -r -9 -q GMT/ftp/GMT_dist.zip GMT/share/coast/*.cdf)
	rm -f asc.lis bin.lis

include Makefile

#-------------------------------------------------------------------------------
# Final cleanup (needs to run last)
#-------------------------------------------------------------------------------

spotless::
		\rm -rf src/makegmt.macros src/gmt_version.h configure autom4te.cache
