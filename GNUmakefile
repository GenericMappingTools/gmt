#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.12 2007-10-04 23:13:49 guru Exp $
#
#		 Guru makefile for GMT Version 4
#			GNU make compatible
#
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	To be installed in top-level GMT directory and requires GNU make
#
#	New GMT gurus should create a file guru/gmtguru.macros with any settings
#	that are intended to overrule the settings in guru/gmtguru.macros.orig.
#	If not, this GNUmakefile will use the default settings from guru/gmtguru.macros.orig.
#	(See that file from more information.)
#	Once done, it is reasonable to first try "make site".
#
#	To ensure the very latest code, start with "make update".
#	To get the latest and make all, do "make install".
#	To wipe all, do "make spotless".
#	The gentler "make clean" will NOT remove code that is generated
#	by GMT scripts, like gmt_math.h and grdmath.c.
#
#	To run all the test dir scripts:	make tests
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
#-------------------------------------------------------------------------------
#	Default setup
#-------------------------------------------------------------------------------

include guru/gmtguru.macros.orig	# Overall default guru settings
sinclude guru/gmtguru.macros		# Guru-specific settings determined by GURU

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
SUPPL	= dbase gshhs imgsrc meca mex mgd77 mgg misc segyprogs spotter x2sys x_system xgrid
SUPPL_M	= dbase imgsrc meca mgd77 mgg misc segyprogs spotter x2sys x_system

#-------------------------------------------------------------------------------
.PHONY:		FILES all man manpages webman webdoc pdfman docs prep_suppl get_coast get_high get_full
		latest-config help update create newsite usable site archive \
		tar_all zip_all zip_bin zip_src \
		full high tar_full zip_full tar_high zip_high installl suppl alltests \
		doctests tests ex examples clean cvsclean distclean spotless

help::
		@grep '^#!' GNUmakefile | cut -c3-
#!----------------- MAKE HELP FOR GMT GURUS -----------------
#!
#!make <target>, where <target> can be:
#!
#!update        : Get the latest source via cvs
#!manpages      : Create manpages from text files
#!usable        : Install-all and run examples
#!site          : Complete install, incl documentation and web pages
#!cvsclean      : Cleanup the package to a nearly clean CVS checkout
#!archive       : Build the release archives
#!

update:
		cvs -q update -Pd

create:
# We make the GMT$(VERSION) link from scratch each time
		cd ..; rm -f GMT$(VERSION); ln -s `basename $(PWD)` GMT$(VERSION)

GMT$(VERSION):	archive

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
alltests:	doctests tests

tests:
		@echo "Run all GMT test scripts"
		@echo ""
		@cd test ; sh run_gmt_tests.sh

doctests:	
		@cd doc ; $(MAKE) tests

#-------------------------------------------------------------------------------
# Cleaning
#-------------------------------------------------------------------------------
cvsclean:
		$(SHELL) guru/cvsclean.sh

spotless::
		cd doc ; $(MAKE) clean

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

FILES =		configure src/makegmt.macros src/gmt_version.h share/conf/gmt.conf share/conf/.gmtdefaults_SI share/conf/.gmtdefaults_US

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
src/makegmt.macros:	guru/gmtguru.macros.orig guru/gmtguru.macros src/makegmt.macros.in src/gmt_version.h configure config.sub config.guess
			rm -f config.cache config.log config.status
			./configure $(GMT_SHARED_LIBS) $(GMT_US) $(GMT_TRIANGLE) $(GMT_DEBUG) \
				$(GMT_DIST) $(GMT_EXDIST) $(GMT_NETCDF) $(GMT_SITE) $(GMT_MATLAB) $(GMT_64) $(GMT_UNIVERSAL)
endif

src/gmt_version.h:	guru/gmtguru.macros.orig guru/gmtguru.macros
			echo '#define GMT_VERSION "$(VERSION)"' > $@

configure:	configure.in
		$(AUTOCONF)

latest-config:
		curl http://cvs.savannah.gnu.org/viewvc/*checkout*/config/config/config.sub --remote-name --silent
		curl http://cvs.savannah.gnu.org/viewvc/*checkout*/config/config/config.guess --remote-name --silent

webman: 	guru/webman.sh
		$(SHELL) guru/webman.sh -s

pdfman: 	guru/pdfman.sh
		$(SHELL) guru/pdfman.sh -s

docs:		FILES
		cd doc ; $(MAKE) install

prep_suppl:	clean config

get_coast get_high get_full:
#		Set-up ftp command & get coast file
		echo "Getting coasts/rivers (GSHHS$(GSHHS_VERSION)_$(subst get_,,$@)) by anonymous ftp (be patient)..."
		echo "user anonymous $(USER)@" > ftp.job
		echo "cd gmt/$(GMTBRANCH)" >> ftp.job
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

tar_all:	tar_progs tar_share tar_coast tar_tut tar_web tar_pdf tar_scripts tar_suppl tar_done

zip_all:	zip_src zip_bin

zip_bin:	zip_exe zip_suppl_exe zip_done

zip_src:	zip_progs zip_share zip_coast zip_tut zip_web zip_pdf zip_scripts zip_suppl zip_done

full:		tar_full zip_full

high:		tar_high zip_high

ftpdir:
		mkdir -p ftp

tar_progs tar_src:	ftpdir
		rm -f ftp/GMT$(VERSION)_src.tar.{gz,bz2}
		grep -vh '#' guru/GMT_progs_files_{ascii,bin}.lis | sed -e 's:^:GMT$(VERSION)/:' > t.progs
		echo "make GMT$(VERSION)_src.tar.gz"
		tar -cf ftp/GMT$(VERSION)_src.tar -C .. -T t.progs
		gzip -c9 ftp/GMT$(VERSION)_src.tar > ftp/GMT$(VERSION)_src.tar.gz
		echo "make GMT$(VERSION)_src.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_src.tar
		rm -f t.progs

zip_progs:	ftpdir
		rm -f ftp/GMT_src.zip
		grep -vh '#' guru/GMT_progs_files_ascii.lis | sed -e 's:^:GMT/:' > asc.lis
		grep -vh '#' guru/GMT_progs_files_bin.lis   | sed -e 's:^:GMT/:' > bin.lis
		echo "make GMT_src.zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_src.zip `cat GMT/asc.lis`)
		if [ -s bin.lis ] ; then (cd ..; zip -r -9 -q GMT/ftp/GMT_src.zip `cat GMT/bin.lis`) ; fi
		rm -f asc.lis bin.lis

tar_share:	ftpdir
		rm -f ftp/GMT$(VERSION)_share.tar.{gz,bz2}
		grep -vh '#' guru/GMT_share_files_{ascii,bin}.lis | sed -e 's:^:GMT$(VERSION)/:' > t.share
		echo "make GMT$(VERSION)_share.tar.gz"
		tar -cf ftp/GMT$(VERSION)_share.tar -C .. -T t.share
		gzip -c9 ftp/GMT$(VERSION)_share.tar > ftp/GMT$(VERSION)_share.tar.gz
		echo "make GMT$(VERSION)_share.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_share.tar
		rm -f t.share

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
		rm -f ftp/GMT$(VERSION)_tut.tar.{gz,bz2}
		echo "make GMT$(VERSION)_tut.tar.gz"
		sed -e 's:^:GMT$(VERSION)/:' guru/GMT_tutorial.lis > t.doc
		echo GMT$(VERSION)/COPYING >> t.doc
		tar -cf ftp/GMT$(VERSION)_tut.tar -C .. -T t.doc
		gzip -c9 ftp/GMT$(VERSION)_tut.tar > ftp/GMT$(VERSION)_tut.tar.gz
		echo "make GMT$(VERSION)_tut.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_tut.tar
		rm -f t.doc

zip_tut:	ftpdir
		rm -f ftp/GMT_tut.zip
		echo "make GMT_tut.zip"
		sed -e 's:^:GMT/:' guru/GMT_tutorial.lis | grep -v '\.nc$$' > asc.lis
		sed -e 's:^:GMT/:' guru/GMT_tutorial.lis | grep '\.nc$$' > bin.lis
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_tut.zip GMT/COPYING `cat GMT/asc.lis`)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_tut.zip `cat GMT/bin.lis`)
		rm -f asc.lis bin.lis

tar_web:	ftpdir
		rm -f ftp/GMT$(VERSION)_web.tar.{gz,bz2}
		echo "make GMT$(VERSION)_web.tar.gz"
		sed -e 's:^:GMT$(VERSION)/:' guru/GMT_www.lis > t.doc
		echo GMT$(VERSION)/COPYING >> t.doc
		tar -cf ftp/GMT$(VERSION)_web.tar -C .. -T t.doc \
		   GMT$(VERSION)/www/gmt/doc/html/GMT_{Docs,Tutorial}
		gzip -c9 ftp/GMT$(VERSION)_web.tar > ftp/GMT$(VERSION)_web.tar.gz
		echo "make GMT$(VERSION)_web.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_web.tar
		rm -f t.doc

zip_web:	ftpdir
		rm -f ftp/GMT_web.zip
		echo "make GMT_web.zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_web.zip GMT/COPYING \
		   GMT/www/gmt/gmt_{man,services,suppl}.html GMT/www/gmt/doc/html/*.html \
		   GMT/www/gmt/doc/html/GMT_{Docs,Tutorial}/*.html)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_web.zip GMT/www/gmt/gmt_back.gif \
		   GMT/www/gmt/doc/html/GMT_{Docs,Tutorial}/*.png)

tar_pdf:	ftpdir
		rm -f ftp/GMT$(VERSION)_pdf.tar.{gz,bz2}
		echo "make GMT$(VERSION)_pdf.tar.gz"
		tar -cf ftp/GMT$(VERSION)_pdf.tar -C .. GMT$(VERSION)/COPYING \
		   GMT$(VERSION)/www/gmt/doc/pdf/GMT_{Docs,Tutorial,Manpages,Manpages_suppl}.pdf
		gzip -c9 ftp/GMT$(VERSION)_pdf.tar > ftp/GMT$(VERSION)_pdf.tar.gz
		echo "make GMT$(VERSION)_pdf.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_pdf.tar
		rm -f t.doc

zip_pdf:	ftpdir
		rm -f ftp/GMT_pdf.zip
		echo "make GMT_pdf.zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_pdf.zip GMT/COPYING)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_pdf.zip \
		   GMT/www/gmt/doc/pdf/GMT_{Docs,Tutorial,Manpages,Manpages_suppl}.pdf)

tar_scripts:	ftpdir
		rm -f ftp/GMT$(VERSION)_scripts.tar.{gz,bz2}
		rm -f examples/ex??/*.ps examples/ex??/*% examples/ex??/*.txt examples/ex??/.gmt*
		sed -e 's:^:GMT$(VERSION)/:' guru/GMT_examples.lis > t.scripts
		echo GMT$(VERSION)/COPYING >> t.scripts
		echo "make GMT$(VERSION)_scripts.tar.gz"
		tar -cf ftp/GMT$(VERSION)_scripts.tar -C .. -T t.scripts
		gzip -c9 ftp/GMT$(VERSION)_scripts.tar > ftp/GMT$(VERSION)_scripts.tar.gz
		echo "make GMT$(VERSION)_scripts.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_scripts.tar
		rm -f t.scripts

zip_scripts:	ftpdir
		rm -f ftp/GMT_scripts.zip
		echo "make GMT_scripts.zip"
		sed -e 's:^:GMT/:' guru/GMT_examples.lis | egrep -v '\.nc$$|\.bz2$$|\.ras$$' > asc.lis
		sed -e 's:^:GMT/:' guru/GMT_examples.lis | egrep '\.nc$$|\.bz2$$|\.ras$$' > bin.lis
		(cd ..; zip -r -9 -q -l GMT/ftp/GMT_scripts.zip GMT/COPYING `cat GMT/asc.lis`)
		(cd ..; zip -r -9 -q    GMT/ftp/GMT_scripts.zip `cat GMT/bin.lis`)
		rm -f asc.lis bin.lis

tar_suppl:	ftpdir
		rm -f ftp/GMT$(VERSION)_suppl.tar.{gz,bz2}
		echo "make GMT$(VERSION)_suppl.tar.gz"
		sed -e 's:^:GMT$(VERSION)/:' guru/GMT_suppl.lis > t.suppl
		echo GMT$(VERSION)/COPYING >> t.suppl
		tar -cf ftp/GMT$(VERSION)_suppl.tar -C .. -T t.suppl
		gzip -c9 ftp/GMT$(VERSION)_suppl.tar > ftp/GMT$(VERSION)_suppl.tar.gz
		echo "make GMT$(VERSION)_suppl.tar.bz2"
		bzip2 -9 ftp/GMT$(VERSION)_suppl.tar
		rm -f t.suppl

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
		rm -f ftp/GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar.{gz,bz2}
		echo "make GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar.gz"
		if [ "$(subst tar_,,$@)" == "coast" ]; then suf=cli; else suf=`echo $@|cut -c5`; fi; \
		   tar -cf ftp/GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar COPYING share/coast/binned_*_[$$suf].cdf -C src/gshhs README.gshhs
		gzip -c9 ftp/GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar > ftp/GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar.gz
		echo "make GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar.bz2"
		bzip2 -9 ftp/GSHHS$(GSHHS_VERSION)_$(subst tar_,,$@).tar

zip_coast zip_high zip_full:	ftpdir
		rm -f ftp/GSHHS_$(subst zip_,,$@).zip
		echo "make GSHHS_$(subst zip_,,$@).zip"
		(cd ..; zip -r -9 -q -l GMT/ftp/GSHHS_$(subst zip_,,$@).zip GMT/COPYING)
		if [ "$(subst zip_,,$@)" == "coast" ]; then suf=cli; else suf=`echo $@|cut -c5`; fi; \
		   (cd ..; zip -r -9 -q    GMT/ftp/GSHHS_$(subst zip_,,$@).zip GMT/share/coast/*_[$$suf].cdf)

tar_done:
		@echo " "
		@echo "Completed tarring off entire archive"

zip_done:
		@echo " "
		@echo "Completed zipping off entire archive"

include Makefile

#-------------------------------------------------------------------------------
# Final cleanup (needs to run last)
#-------------------------------------------------------------------------------

spotless::
		\rm -rf src/makegmt.macros configure autom4te.cache
