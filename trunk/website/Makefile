#-------------------------------------------------------------------------------
#  $Id: Makefile,v 1.30 2008-05-15 03:11:58 guru Exp $
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
#	To update FTP and WEB test sites with the new version:  make update-version-local
#
#	To place all in the test FTP directory:		make update-ftp-local
#	To place all in the test WEB directory:		make update-www-local
#	To place all in the actual FTP directory:	make update-ftp-live
#	To place all in the actual WEB directory:	make update-www-live
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		30-APR-2008
#-------------------------------------------------------------------------------
#	Default setup
#-------------------------------------------------------------------------------

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
#!update-version-local	: Update the local ftp and www test dirs
#!update-version-test	: Update the remote TEST ftp and www server
#!update-version-live	: Update the remote LIVE ftp and www server
#!update-www-live	: Update the LIVE www server
#!update-www-test	: Update the TEST www server
#!update-www-local	: Update the local sitetest/web dir
#!update-ftp-live	: Update the LIVE ftp server with latest GMT
#!update-ftp-test	: Update the LIVE ftp server test dir with latest GMT
#!update-ftp-local	: Update the local sitetest/ftp dir with latest GMT
#!update-gshhs-live	: Update the LIVE ftp server with latest GSHHS
#!update-gshhs-test	: Update the LIVE ftp server test dir with latest GSHHS
#!update-gshhs-local	: Update the Update the local sitetest/ftp with latest GSHHS
#!clean-ftp		: Empty all local ftp archives
#!clean-local		: Remove the local sitetest directories
#!clean-test		: Empty the remote test directories
#!hitmap		: Creates the GMT mirror page for site links
#!---------------------------------------------------------------
		
# Place stuff in the GMT www directory:

../www/gmt/examples/ex01/example_01_100dpi.png:	../examples/ex01/example_01.ps
	sh webexamples.sh -s

../examples/ex01/example_01.ps:	../examples/ex01/job01.sh
	( cd ..; $(MAKE) -f GNUmakefile examples)

#---------------------------------------------------------------------------
# GMT VERSION UPDATE
#---------------------------------------------------------------------------

update-version-local:	update-ftp-local update-www-local

update-version-test:	update-ftp-test update-www-test

update-version-live:	update-ftp-live update-www-live
		echo "Both the FTP and WEB sites will be updated automatically."
		echo "However, the following tasks must be done manually for now:"
		echo "1. Manually remove old GMT4.* files from $(LIVEFTPDIR)"
		echo "2. Send email notification to gmt-help, gmt-group, gmt-mirrors, and gmt-team"
		echo "For more details, see RELEASE.HOWTO"

#---------------------------------------------------------------------------
# GMT WEB PAGE MATERIALS
#---------------------------------------------------------------------------

update-www-live:	../registration/gmt_usage.jpg gmt_mirrors.html gmt_releases.html ../www/gmt/examples/ex01/example_01_100dpi.png
		scp install_gmt_form.pl $(WEBSERVER):$(LIVECGIDIR)/install_gmt_form.pl
		ssh $(WEBSERVER) mkdir -p $(LIVEWWWDIR)/gmt
		ssh $(WEBSERVER) rm -f $(LIVEWWWDIR)/index.html
		scp gmt.html $(WEBSERVER):$(LIVEWWWDIR)/index.html
		scp gmt_*.php gmt_*.html newgmtguru *.gif gmt_*.png linux.jpg osx.jpg unix.jpg vista.jpg \
		  ../src/README.TRIANGLE ../www/gmt/gmt_services.html ../www/gmt/gmt_man.html ../registration/gmt_usage.jpg \
		  ../www/gmt/gmt_examples.html ../www/gmt/gmt_suppl.html ../www/gmt/gmt_back.gif $(WEBSERVER):$(LIVEWWWDIR)/gmt
		ssh $(WEBSERVER) rm -r -f $(LIVEWWWDIR)/gmt/doc $(LIVEWWWDIR)/gmt/examples $(LIVEWWWDIR)/gmt/contrib
		scp -r ../www/gmt/doc $(WEBSERVER):$(LIVEWWWDIR)/gmt
		scp -r ../www/gmt/examples $(WEBSERVER):$(LIVEWWWDIR)/gmt
		scp -r contrib $(WEBSERVER):$(LIVEWWWDIR)/gmt

update-www-test:	../registration/gmt_usage.jpg gmt_mirrors.html gmt_releases.html ../www/gmt/examples/ex01/example_01_100dpi.png
#		For the test we want GMT_ftpsite to always be 0, and to use the test web site - hence these sed commands
		echo "sB/gmt/B/gmttest/Bg" > job.sed
		echo "sBgmt.soest.hawaii.edu/gmttempBwww.soest.hawaii.edu/gmttest/gmttempBg" >> job.sed
		echo 's/"GMT_ftpsite=", \$$site/"GMT_ftpsite=0"/g' >> job.sed
		sed -f job.sed install_gmt_form.pl > install_gmt_form_test.pl
		scp install_gmt_form_test.pl $(WEBSERVER2):$(LIVECGIDIR2)
		echo "sBftp.soest.hawaii.edu/gmtBftp.soest.hawaii.edu/gmttestBg" > job.sed
		sed -f job.sed gmt_windows.html > gmt_windows_test.html
		rm -f install_gmt_form_test.pl job.sed
		ssh $(WEBSERVER2) mkdir -p $(LIVEWWWDIR2)/gmt
		ssh $(WEBSERVER2) rm -f $(LIVEWWWDIR2)/index.html
		scp gmt.html $(WEBSERVER2):$(LIVEWWWDIR2)/index.html
		scp gmt_*.php gmt_*.html newgmtguru *.gif gmt_*.png linux.jpg osx.jpg unix.jpg vista.jpg \
		  ../src/README.TRIANGLE ../www/gmt/gmt_services.html ../www/gmt/gmt_man.html ../registration/gmt_usage.jpg \
		  ../www/gmt/gmt_examples.html ../www/gmt/gmt_suppl.html ../www/gmt/gmt_back.gif $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
#	Change the Href to the ftp server to go to the test server dir instead
		sed -e sBftp.soest.hawaii.edu/gmtBftp.soest.hawaii.edu/gmttestBg gmt_install_form.html > tmp
		scp tmp $(WEBSERVER2):$(LIVEWWWDIR2)/gmt/gmt_install_form.html
		ssh $(WEBSERVER2) rm -r -f $(LIVEWWWDIR2)/gmt/doc $(LIVEWWWDIR2)/gmt/examples $(LIVEWWWDIR2)/gmt/contrib
		scp -r ../www/gmt/doc $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
		scp -r ../www/gmt/examples $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
		scp -r contrib $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
		scp gmt_windows_test.html $(WEBSERVER2):$(LIVEWWWDIR2)/gmt/gmt_windows.html
		echo "cd $(LIVEWWWDIR2)/gmt" > gmt_server_job.sh
		echo "sed -e s/install_gmt_form.pl/install_gmt_form_test.pl/g gmt_install_form.html > tmp" >> gmt_server_job.sh
		echo "mv -f tmp gmt_install_form.html" >> gmt_server_job.sh
		echo "cp -f ../../gmt/gmt/ChangeLog ." >> gmt_server_job.sh
		echo "rm -f gmt_server_job.sh" >> gmt_server_job.sh
		scp gmt_server_job.sh $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
		ssh $(WEBSERVER2) sh $(LIVEWWWDIR2)/gmt/gmt_server_job.sh
		rm -f gmt_server_job.sh tmp

update-www-local:../registration/gmt_usage.jpg gmt_mirrors.html gmt_releases.html ../www/gmt/examples/ex01/example_01_100dpi.png
		rm -rf $(TESTWWWDIR)
		mkdir -p $(TESTWWWDIR)/gmt
		cp -f gmt.html $(TESTWWWDIR)/index.html
		cp -f gmt_*.php gmt_*.html $(TESTWWWDIR)/gmt
		cp -f newgmtguru $(TESTWWWDIR)/gmt
		cp -f *.gif linux.jpg osx.jpg unix.jpg vista.jpg $(TESTWWWDIR)/gmt
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
		cp -R -f contrib $(TESTWWWDIR)/gmt


#---------------------------------------------------------------------------
# GMT FTP UPDATE
#---------------------------------------------------------------------------

# Note: .message should be less than 1300-1400 bytes!

update-ftp-live:
	if [ `cat ../ftp/.message | wc -c | awk '{printf "%d\n", $$1}'` -ge 1300 ]; then \
		echo -n "The .message file byte length is over 1300 bytes: "; cat ../ftp/.message | wc -c; \
		exit; \
	fi
	ssh $(FTPSERVER) 'rm -rf $(LIVEFTPDIR)/windows $(LIVEFTPDIR)/contrib $(LIVEFTPDIR)/*'
	ssh $(FTPSERVER) mkdir -p $(LIVEFTPDIR)/windows $(LIVEFTPDIR)/contrib
	scp ../guru/install_gmt $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/GMT*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/GMT*.exe $(FTPSERVER):$(LIVEFTPDIR)/windows
	scp ../ftp/gawk*.zip $(FTPSERVER):$(LIVEFTPDIR)/windows
	echo "cd $(LIVEFTPDIR)" > gmt_server_job.sh
	for p in pdf web tut share src scripts suppl triangle; do \
		echo "rm -f GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
		echo "$(LN_S) GMT$(GMT_VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
	done
	for p in basic pdf; do \
		echo "rm -f windows/GMT_$${p}_install.exe" >> gmt_server_job.sh; \
		echo "(cd windows; $(LN_S) GMT$(GMT_VERSION)_$${p}_install.exe GMT_$${p}_install.exe)" >> gmt_server_job.sh; \
	done
	echo "rm -f $(LIVEFTPDIR)/gmt_server_job.sh" >> gmt_server_job.sh
	scp gmt_server_job.sh $(FTPSERVER):$(LIVEFTPDIR)
	ssh $(FTPSERVER) sh $(LIVEFTPDIR)/gmt_server_job.sh
	rm -f gmt_server_job.sh

update-gshhs-live:
	scp ../ftp/GSHHS*.bz2  $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/GSHHS*.exe $(FTPSERVER):$(LIVEFTPDIR)/windows
	echo "cd $(LIVEFTPDIR)" > gmt_server_job.sh
	for p in coast high full; do \
		echo "rm -f {GMT,GSHHS}_$$p.tar.bz2" >> gmt_server_job.sh; \
		echo "$(LN_S) GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2" >> gmt_server_job.sh; \
	done
	echo "rm -f windows/GSHHS_highfull_install.exe" >> gmt_server_job.sh;
	echo "(cd windows; $(LN_S) GSHHS$(GSHHS_VERSION)_highfull_install.exe GSHHS_highfull_install.exe)" >> gmt_server_job.sh;
	echo "rm -f $(LIVEFTPDIR)/gmt_server_job.sh" >> gmt_server_job.sh
	scp gmt_server_job.sh $(FTPSERVER):$(LIVEFTPDIR)
	ssh $(FTPSERVER) sh $(LIVEFTPDIR)/gmt_server_job.sh
	rm -f gmt_server_job.sh

update-ftp-test:
	if [ `cat ../ftp/.message | wc -c | awk '{printf "%d\n", $$1}'` -ge 1300 ]; then \
		echo -n "The .message file byte length is over 1300 bytes: "; cat ../ftp/.message | wc -c; \
		exit; \
	fi
	ssh $(FTPSERVER2) 'rm -rf $(LIVEFTPDIR2)/windows $(LIVEFTPDIR2)/contrib $(LIVEFTPDIR2)/*'
	ssh $(FTPSERVER2) mkdir -p $(LIVEFTPDIR2)/windows $(LIVEFTPDIR2)/contrib
	scp ../guru/install_gmt $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/GMT*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/GMT*.exe $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	scp ../ftp/gawk*.zip $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	echo "cd $(LIVEFTPDIR2)" > gmt_server_job.sh
	for p in pdf web tut share src scripts suppl triangle; do \
		echo "rm -f GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
		echo "$(LN_S) GMT$(GMT_VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
	done
	for p in basic pdf; do \
		echo "rm -f windows/GMT_$${p}_install.exe" >> gmt_server_job.sh; \
		echo "(cd windows; $(LN_S) GMT$(GMT_VERSION)_$${p}_install.exe GMT_$${p}_install.exe)" >> gmt_server_job.sh; \
	done
	echo "rm -f $(LIVEFTPDIR2)/gmt_server_job.sh" >> gmt_server_job.sh
	scp gmt_server_job.sh $(FTPSERVER2):$(LIVEFTPDIR2)
	ssh $(FTPSERVER2) sh $(LIVEFTPDIR2)/gmt_server_job.sh
	rm -f gmt_server_job.sh

update-gshhs-test:
	scp ../ftp/GSHHS*.bz2 $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/GSHHS*.exe $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	echo "cd $(LIVEFTPDIR2)" > gmt_server_job.sh
	for p in coast high full; do \
		echo "rm -f {GMT,GSHHS}_$$p.tar.bz2" >> gmt_server_job.sh; \
		echo "$(LN_S) GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2" >> gmt_server_job.sh; \
	done
	echo "rm -f windows/GSHHS_highfull_install.exe" >> gmt_server_job.sh;
	echo "(cd windows; $(LN_S) GSHHS$(GSHHS_VERSION)_highfull_install.exe GSHHS_highfull_install.exe)" >> gmt_server_job.sh;
	echo "rm -f $(LIVEFTPDIR2)/gmt_server_job.sh" >> gmt_server_job.sh
	scp gmt_server_job.sh $(FTPSERVER2):$(LIVEFTPDIR2)
	ssh $(FTPSERVER2) sh $(LIVEFTPDIR2)/gmt_server_job.sh
	rm -f gmt_server_job.sh

update-ftp-local:
	if [ `cat ../ftp/.message | wc -c | awk '{printf "%d\n", $$1}'` -ge 1300 ]; then \
		echo -n "The .message file byte length is over 1300 bytes: "; cat ../ftp/.message | wc -c; \
		exit; \
	fi
	rm -rf $(TESTFTPDIR)
	mkdir -p $(TESTFTPDIR)/windows $(TESTFTPDIR)/contrib
	sed -e 'sXpub/gmtXpub/pwessel/gmttestXg' ../guru/install_gmt > $(TESTFTPDIR)/install_gmt
	cp -f ../ftp/GMT*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(TESTFTPDIR)
	cp -f ../ftp/GMT*.exe $(TESTFTPDIR)/windows
	cp -f ../ftp/gawk*.zip $(TESTFTPDIR)/windows
	for p in pdf web tut share src scripts suppl triangle; do \
		\rm -f $(TESTFTPDIR)/GMT_$$p.tar.bz2; \
		(cd $(TESTFTPDIR); $(LN_S) GMT$(GMT_VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2); \
	done
	for p in basic pdf; do \
		rm -f windows/GMT_$$p_install.exe; \
		(cd $(TESTFTPDIR)/windows; $(LN_S) GMT$(GMT_VERSION)_$${p}_install.exe GMT_$${p}_install.exe); \
	done

update-gshhs-local:
	cp -f ../ftp/GSHHS*.bz2 $(TESTFTPDIR)
	cp -f ../ftp/GSHHS*.exe $(TESTFTPDIR)/windows
	for p in coast high full; do \
		\rm -f $(TESTFTPDIR)/{GMT,GSHHS}_$$p.tar.bz2; \
		(cd $(TESTFTPDIR); $(LN_S) GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2); \
	done
	rm -f windows/GSHHS_highfull_install.exe
	(cd $(TESTFTPDIR)/windows; $(LN_S) GSHHS$(GSHHS_VERSION)_highfull_install.exe GSHHS_highfull_install.exe)

clean-ftp:
	rm -f ../ftp/G*.bz2

clean-local:
	rm -rf $(TESTFTPDIR)
	rm -rf $(TESTWWWDIR)

clean-test:
	ssh $(FTPSERVER2) rm -rf $(LIVEFTPDIR2)/*
	ssh $(WEBSERVER2) rm -rf $(LIVEWWWDIR2)/gmt $(LIVEWWWDIR2)/gmt.html $(LIVEWWWDIR2)/index.html
	
netcdf: ../ftp/netcdf.tar.bz2

../ftp/netcdf.tar.bz2:
	cd ../ftp; \
	curl http://www.unidata.ucar.edu/downloads/netcdf/ftp/netcdf.tar.gz --remote-name --silent; \
	gunzip netcdf.tar.gz; \
	bzip2 -9 netcdf.tar

nan:
	cd ../src; \
	$(CC) $(CFLAGS) gmt_nan_save.c $(CDF) -lm -o gmt_nan_save; \
	./gmt_nan_save `uname -s`

gmt_mirrors.html:	gmt_hitmap.sh
	gmt_hitmap.sh -s

gmt_hitmap.png: 	gmt_hitmap.sh
	gmt_hitmap.sh -s

hitmap:
	gmt_hitmap.sh -s

../registration/gmt_usage.jpg:		../registration/GMT_old_unique_sites.d
		(cd ../registration; GMT_usage_map.sh map)

gmt_releases.html:	gmt_releases_template.html ../www/gmt/doc/html/GMT_Docs/GMT_Docs.html
	(cd ../www/gmt; grep -H "scientists since 1988" doc/html/GMT_Docs/*.html | awk -F: '{printf "sXCHAPTER1LINKX%sXg\n",  $$1}' > ../../website/tmp.sed)
	sed -f tmp.sed < gmt_releases_template.html > gmt_releases.html
	
	