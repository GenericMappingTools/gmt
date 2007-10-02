#-------------------------------------------------------------------------------
#  $Id: Makefile,v 1.1 2007-10-02 23:16:16 guru Exp $
#
#		 Guru Makefile for GMT Version 4.x
#			GNU make compatible
#	
#	!!! THIS MAKEFILE IS FOR THE GMT SITE ONLY !!!
#
#	TO BE INSTALLED IN GMT/website and requires GNU make
#
#	Requires guru/gmtguru.macros and src/makegmt.macros to be manually set.
#	All configure changes are to be made in those files, not here.
#
#	To update FTP and WEB live sites with the new version:  make update-version-live
#	To update FTP and WEB test sites with the new version:  make update-version-test
#
#	To place all in the test FTP directory:		make update-ftp-test
#	To place all in the test WEB directory:		make update-www-test
#	To place all in the actual FTP directory:	make update-ftp-live
#	To place all in the actual WEB directory:	make update-www-live
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		02-OCT-2007
#-------------------------------------------------------------------------------
#	Default setup
#-------------------------------------------------------------------------------

include ../guru/gmtguru.macros.orig	# Guru-specific settings determined by GURU
sinclude ../guru/gmtguru.macros		# any overrides
include ../src/makegmt.macros		# GMT-specific settings determined by user
include gmtsite.macros			# Site-specific settings determined by webmaster

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

help::
	@grep '^#!' makefile | cut -c3-
#!-------------------- MAKE HELP FOR WEBSITE --------------------
#!
#!make <target>, where <target> can be:
#!
#!web			: Create the examples for the web site
#!update-version-test	: Update the TEST ftp and www server
#!update-version-live	: Update the LIVE ftp and www server
#!update-www-live		: Update the LIVE www server
#!update-www-test		: Update the TEST www server
#!update-ftp-live		: Update the LIVE ftp server
#!update-ftp-test		: Update the TEST ftp server
#!update-bugs		: Update the errata pages
#!clean-ftp		: Remove all local ftp archives
#!clean-test		: Remove the test directories
#!hitmap			: Creates the GMT hit map for site links
#!---------------------------------------------------------------
		
web:		../examples/ex01/example_01.ps
# Places stuff in the GMT www directory
		sh webexamples.sh -s

../examples/ex01/example_01.ps:
		( cd ..; $(MAKE) -f GNUmakefile examples)

#---------------------------------------------------------------------------
# GMT VERSION UPDATE
#---------------------------------------------------------------------------

update-version-test:	update-ftp-test update-www-test

update-version-live:	update-ftp-live update-www-live
		echo "Both the FTP and WEB sites will be updated automatically."
		echo "However, the following tasks must be done manually for now:"
		echo "1. Manually remove old GMT4.* files from $(LIVEFTPDIR)/4"
		echo "2. Send email notification to gmt-help, gmt-group, gmt-mirrors, and gmt-team"
		echo "3. Install GMTxxx on the SOEST server"
		echo "For more details, see RELEASE.HOWTO"

#---------------------------------------------------------------------------
# GMT WEB PAGE MATERIALS
#---------------------------------------------------------------------------

update-www-live:		
		scp install_gmt_form.pl $(WEBSERVER):$(LIVECGIDIR)/install_gmt_form.pl
		ssh $(WEBSERVER) mkdir -p $(LIVEWWWDIR)/gmt
		ssh $(WEBSERVER) rm -f $(LIVEWWWDIR)/index.html
		scp gmt.html $(WEBSERVER):$(LIVEWWWDIR)/index.html
		scp gmt_*.html gmt_hitmap.map newgmtguru gmt_*.gif GMT?.gif soest_logo_reduced2.gif gmt_*.png \
		  ../src/README.TRIANGLE ../www/gmt/gmt_services.html ../www/gmt/gmt_man.html ../registration/gmt_usage.jpg \
		  ../www/gmt/gmt_examples.html ../www/gmt/gmt_suppl.html ../www/gmt/gmt_back.gif $(WEBSERVER):$(LIVEWWWDIR)/gmt
		ssh $(WEBSERVER) rm -r -f $(LIVEWWWDIR)/gmt/doc $(LIVEWWWDIR)/gmt/examples
		scp -r ../www/gmt/doc $(WEBSERVER):$(LIVEWWWDIR)/gmt
		scp -r ../www/gmt/examples $(WEBSERVER):$(LIVEWWWDIR)/gmt

update-www-test:
		rm -rf $(TESTWWWDIR)
		mkdir -p $(TESTWWWDIR)/gmt
		cp -f gmt.html $(TESTWWWDIR)/index.html
		cp -f gmt_*.html $(TESTWWWDIR)/gmt
		cp -f gmt_hitmap.map $(TESTWWWDIR)/gmt
		cp -f newgmtguru $(TESTWWWDIR)/gmt
		cp -f gmt_*.gif GMT?.gif soest_logo_reduced2.gif $(TESTWWWDIR)/gmt
		cp -f gmt_*.png ../registration/gmt_usage.jpg $(TESTWWWDIR)/gmt
		cp -f ../src/README.TRIANGLE $(TESTWWWDIR)/gmt
		cp -f ../www/gmt/gmt_services.html $(TESTWWWDIR)/gmt
		cp -f ../www/gmt/gmt_man.html $(TESTWWWDIR)/gmt
		cp -f ../www/gmt/gmt_examples.html $(TESTWWWDIR)/gmt
		cp -f ../www/gmt/gmt_suppl.html $(TESTWWWDIR)/gmt
		cp -f ../www/gmt/gmt_back.gif $(TESTWWWDIR)/gmt
		rm -r -f $(TESTWWWDIR)/gmt/doc $(TESTWWWDIR)/gmt/examples
		cp -R -f ../www/gmt/doc $(TESTWWWDIR)/gmt
		cp -R -f ../www/gmt/examples $(TESTWWWDIR)/gmt

update-bugs:
		scp gmt_errata_[34].html $(WEBSERVER):$(LIVEWWWDIR)/gmt

#---------------------------------------------------------------------------
# GMT FTP UPDATE
#---------------------------------------------------------------------------

# Note: .message should be less than 1300-1400 bytes!

update-ftp-live:
	echo -n "The .message file byte length is "; cat ../ftp/.message | wc -c
	ssh $(FTPSERVER) mkdir -p $(LIVEFTPDIR)/4
	scp ../guru/install_gmt $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/*.bz2 ../ftp/*.gz ../ftp/*.zip ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER):$(LIVEFTPDIR)/4
	echo "cd $(LIVEFTPDIR)/4" > live2.sh
	for p in pdf man web tut share src scripts suppl; do \
		echo "rm -f $(LIVEFTPDIR)/4/GMT_$$p.tar.{bz2,gz}" >> live2.sh; \
		echo "ln -s GMT$(VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2" >> live2.sh; \
		echo "ln -s GMT$(VERSION)_$$p.tar.gz GMT_$$p.tar.gz" >> live2.sh; \
	done
	for p in coast high full; do \
		echo "rm -f $(LIVEFTPDIR)/4/{GMT,GSHHS}_$$p.tar.{bz2,gz} >> live2.sh; \
		echo "ln -s GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2" >> live2.sh; \
		echo "ln -s GSHHS$(GSHHS_VERSION)_$$p.tar.gz GSHHS_$$p.tar.gz" >> live2.sh; \
	done
	echo "rm -f $(LIVEFTPDIR)/live2.sh" >> live2.sh
	scp live2.sh $(FTPSERVER):$(LIVEFTPDIR)
	rm -f live2.sh
	ssh $(FTPSERVER) sh $(LIVEFTPDIR)/live2.sh

update-ftp-live2:
	echo -n "The .message file byte length is "; cat ../ftp/.message | wc -c
	ssh $(FTPSERVER) mkdir -p $(LIVEFTPDIR2)/4
	scp ../guru/install_gmt $(FTPSERVER):$(LIVEFTPDIR2)
	scp ../ftp/*.bz2 ../ftp/*.gz ../ftp/*.zip ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER):$(LIVEFTPDIR2)/4
	echo "cd $(LIVEFTPDIR2)/4" > live2.sh
	for p in pdf man web tut share src scripts suppl; do \
		echo "rm -f $(LIVEFTPDIR2)/4/GMT_$$p.tar.{bz2,gz} >> live2.sh; \
		echo "ln -s GMT$(VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2" >> live2.sh; \
		echo "ln -s GMT$(VERSION)_$$p.tar.gz GMT_$$p.tar.gz" >> live2.sh; \
	done
	for p in coast high full; do \
		echo "rm -f $(LIVEFTPDIR2)/4/{GMT,GSHHS}_$$p.tar.{bz2,gz} >> live2.sh; \
		echo "ln -s GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2" >> live2.sh; \
		echo "ln -s GSHHS$(GSHHS_VERSION)_$$p.tar.gz GSHHS_$$p.tar.gz" >> live2.sh; \
	done
	echo "rm -f $(LIVEFTPDIR2)/live2.sh" >> live2.sh
	scp live2.sh $(FTPSERVER):$(LIVEFTPDIR2)
	rm -f live2.sh
	ssh $(FTPSERVER) sh $(LIVEFTPDIR2)/live2.sh

update-ftp-test:
	echo -n "The .message file byte length is "; cat ../ftp/.message | wc -c
	rm -rf $(TESTFTPDIR)/4
	mkdir -p $(TESTFTPDIR)/4
	sed -e 'sXpub/gmtXpub/pwessel/gmttestXg' ../guru/install_gmt > $(TESTFTPDIR)/install_gmt
	cp -f ../ftp/*.bz2 ../ftp/*.gz ../ftp/*.zip ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(TESTFTPDIR)/4
	for p in pdf man web tut share src scripts suppl; do \
		\rm -f $(TESTFTPDIR)/4/GMT_$$p.tar.{bz2,gz}; \
		(cd $(TESTFTPDIR)/4; ln -s GMT$(VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2); \
		(cd $(TESTFTPDIR)/4; ln -s GMT$(VERSION)_$$p.tar.gz GMT_$$p.tar.gz); \
	done
	for p in coast high full; do \
		\rm -f $(TESTFTPDIR)/4/{GMT,GSHHS}_$$p.tar.{bz2,gz}; \
		(cd $(TESTFTPDIR)/4; ln -s GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2); \
		(cd $(TESTFTPDIR)/4; ln -s GSHHS$(GSHHS_VERSION)_$$p.tar.gz GSHHS_$$p.tar.gz); \
	done

clean-ftp:
	rm -f ../ftp/*.bz2 ../ftp/*.gz ../ftp/*.zip

clean-test:
	rm -rf $(TESTFTPDIR)
	rm -rf $(TESTWWWDIR)

nan:
		cd ../src; \
		$(CC) $(CFLAGS) gmt_nan_save.c $(CDF) -lm -o gmt_nan_save; \
		./gmt_nan_save `uname -s`

hitmap:
	gmt_hitmap.sh -s
