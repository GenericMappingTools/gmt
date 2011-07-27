#-------------------------------------------------------------------------------
#  $Id$
#
#		 Guru Makefile for GMT Version 5.x
#			GNU make compatible
#	
#	!!! THIS MAKEFILE IS FOR THE GMT SITE ONLY !!!
#
#	TO BE INSTALLED IN GMT/website and requires GNU make
#
#	Requires guru/gmtguru.macros and src/config.mk to be manually set.
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
#	Date:		12-JUL-2011
#-------------------------------------------------------------------------------
#	Default setup
#-------------------------------------------------------------------------------

include ../src/config.mk		# GMT-specific settings determined by user
include gmtsite.macros			# Site-specific settings determined by webmaster

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

help::
	@grep '^#!' Makefile | cut -c3-
#!-------------------- MAKE HELP FOR WEBSITE --------------------
#!
#!make <target>, where <target> can be:
#!
#!update-version-local	: Update the local ftp and www test dirs
#!update-version-test	: Update the remote TEST ftp and www server
#!update-version-live	: Update the remote LIVE ftp and www server
#!update-www-live	: Update the LIVE www server
#!update-www-test	: Update the TEST www server
#!update-www-local	: Update the local www dir
#!update-ftp-live	: Update the LIVE ftp server with latest GMT
#!update-ftp-test	: Update the TEST ftp server test dir with latest GMT
#!update-ftp-local	: Update the local ftp dir with latest GMT
#!update-gshhs-live	: Update the LIVE ftp server with latest GSHHS
#!update-gshhs-test	: Update the TEST ftp server test dir with latest GSHHS
#!update-gshhs-local	: Update the Update the local ftp with latest GSHHS
#!clean-ftp		: Empty all local ftp archives
#!clean-local		: Remove the local ftp and www directories
#!clean-test		: Empty the remote test directories
#!hitmap		: Creates the GMT mirror page for site links
#!---------------------------------------------------------------

GMT4 = /Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/gmt4

# Place stuff in the GMT website/www directory:

www/gmt/examples/ex01/example_01.zip:	../doc/fig/example_01.ps
	sh webexamples.sh -s

#---------------------------------------------------------------------------
# GMT VERSION UPDATE
#---------------------------------------------------------------------------

update-version-local:	update-ftp-local update-www-local

update-version-test:	update-ftp-test update-www-test

update-version-live:	update-ftp-live update-www-live
		echo "Both the FTP and WEB sites will be updated automatically."
		echo "However, the following tasks must be done manually for now:"
		echo "1. Send email notification to gmt-help, gmt-group, gmt-mirrors, and gmt-team"
		echo "For more details, see RELEASE.HOWTO"

#---------------------------------------------------------------------------
# GMT WEB PAGE MATERIALS
#---------------------------------------------------------------------------

update-www-live:	update-www-local
#	First completely remove the live www files then copy them from the www-local files
		sh winsites.sh gmt
		cp -f gmt_windows_*.html www/gmt
		ssh $(WEBSERVER) rm -rf $(LIVEWWWDIR)/gmt $(LIVEWWWDIR)/index.html
		scp -r www/gmt $(WEBSERVER):$(LIVEWWWDIR)
		scp -r www/5 $(WEBSERVER):$(LIVEWWWDIR)
		scp www/index.html $(WEBSERVER):$(LIVEWWWDIR)
#	Update the Perl script in the CGI bin dir
		scp install_gmt_form.pl $(WEBSERVER):$(LIVECGIDIR)/install_gmt_form.pl

update-www-test:	update-www-local
#	First completely remove the test www files then copy then from the www-local files
		sh winsites.sh gmttest
		cp -f gmt_windows_*.html www/gmt
		ssh $(WEBSERVER2) rm -rf $(LIVEWWWDIR2)/gmt $(LIVEWWWDIR2)/index.html
		scp -r www/gmt $(WEBSERVER2):$(LIVEWWWDIR2)
		scp -r www/5 $(WEBSERVER2):$(LIVEWWWDIR2)
		scp www/index.html $(WEBSERVER2):$(LIVEWWWDIR2)
#	For the test we want GMT_ftpsite to always be 0, and to use the test web site - hence these sed commands
		echo "sB/gmt/B/gmttest/Bg" > job.sed
		echo "sBgmt.soest.hawaii.edu/gmttempBwww.soest.hawaii.edu/gmttest/gmttempBg" >> job.sed
		echo 's/"GMT_ftpsite=", \$$site/"GMT_ftpsite=0"/g' >> job.sed
		sed -f job.sed install_gmt_form.pl > install_gmt_form_test.pl
		scp install_gmt_form_test.pl $(WEBSERVER2):$(LIVECGIDIR2)
		echo "sBftp.soest.hawaii.edu/gmtBftp.soest.hawaii.edu/gmttestBg" > job.sed
		rm -f install_gmt_form_test.pl job.sed
#	Change the Href to the ftp server to go to the test server dir instead
		sed -e sBftp.soest.hawaii.edu/gmtBftp.soest.hawaii.edu/gmttestBg gmt_install_form.html > tmp
		scp tmp $(WEBSERVER2):$(LIVEWWWDIR2)/gmt/gmt_install_form.html
		echo "cd $(LIVEWWWDIR2)/gmt" > gmt_server_job.sh
		echo "sed -e s/install_gmt_form.pl/install_gmt_form_test.pl/g gmt_install_form.html > tmp" >> gmt_server_job.sh
		echo "mv -f tmp gmt_install_form.html" >> gmt_server_job.sh
		echo "cp -f ../../gmt/gmt/ChangeLog ." >> gmt_server_job.sh
		echo "rm -f gmt_server_job.sh" >> gmt_server_job.sh
		scp gmt_server_job.sh $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
		ssh $(WEBSERVER2) sh $(LIVEWWWDIR2)/gmt/gmt_server_job.sh
		rm -f gmt_server_job.sh tmp

update-www-local:	../registration/gmt_usage.jpg gmt_mirrors.html www/gmt/gmt_releases.html www/gmt/examples/ex01/example_01.zip
		sh winsites.sh gmt
		mkdir -p www/gmt
		cp -f ../guru/install_gmt.sh www/gmt
		cp -f gmt.html www/index.html
		cp -f gmt_*.php gmt_*.html www/gmt
		rm -f www/gmt/gmt_*_template.html
		cp -f newgmtguru www/gmt
		cp -f *.gif *.ico linux.jpg osx.jpg unix.jpg vista.jpg www/gmt
		cp -f gmt_*.png ../registration/gmt_usage.jpg www/gmt
		cp -f soest_*.png www/gmt
		cp -f ../src/README.TRIANGLE www/gmt
		rm -rf www/gmt/{doc,html,pdf}
		cp -Rf ../doc/{html,pdf} www/gmt
		cp -Rf contrib www/gmt
		find www -name '.*' -exec rm -f {} \;
		rm -f www/gmt/contrib/*.sh


#---------------------------------------------------------------------------
# GMT FTP UPDATE
#---------------------------------------------------------------------------

# Note: .message should be less than 1300-1400 bytes!

update-ftp-live:
	if [ `cat ../ftp/.message | wc -c | awk '{printf "%d\n", $$1}'` -ge 1300 ]; then \
		echo -n "The .message file byte length is over 1300 bytes: "; cat ../ftp/.message | wc -c; \
		exit; \
	fi
	ssh $(FTPSERVER) 'rm -rf $(LIVEFTPDIR)/windows $(LIVEFTPDIR)/contrib $(LIVEFTPDIR)/gmt-* $(LIVEFTPDIR)/install_gmt.sh $(LIVEFTPDIR)/VERSION'
	ssh $(FTPSERVER) mkdir -p $(LIVEFTPDIR)/windows $(LIVEFTPDIR)/contrib
	scp ../guru/install_gmt.sh $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/gmt-*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/gmt-*.exe $(FTPSERVER):$(LIVEFTPDIR)/windows
	scp ../ftp/gawk*.zip $(FTPSERVER):$(LIVEFTPDIR)/windows
	scp ../ftp/netcdf-3.6.3_win.zip $(FTPSERVER):$(LIVEFTPDIR)/windows
	scp ../ftp/Whale-watching_symbols_for_GMT.zip $(FTPSERVER):$(LIVEFTPDIR)/contrib

update-gshhs-live:
	ssh $(FTPSERVER) 'rm -rf $(LIVEFTPDIR)/gshhs-*'
	scp ../ftp/gshhs-*.bz2  $(FTPSERVER):$(LIVEFTPDIR)
#	scp ../ftp/gshhs-*.exe $(FTPSERVER):$(LIVEFTPDIR)/windows

update-ftp-test:
	if [ `cat ../ftp/.message | wc -c | awk '{printf "%d\n", $$1}'` -ge 1300 ]; then \
		echo -n "The .message file byte length is over 1300 bytes: "; cat ../ftp/.message | wc -c; \
		exit; \
	fi
	ssh $(FTPSERVER2) 'rm -rf $(LIVEFTPDIR2)/windows $(LIVEFTPDIR2)/contrib $(LIVEFTPDIR2)/gmt-* $(LIVEFTPDIR2)/install_gmt.sh $(LIVEFTPDIR2)/VERSION'
	ssh $(FTPSERVER2) mkdir -p $(LIVEFTPDIR2)/windows $(LIVEFTPDIR2)/contrib
	scp ../guru/install_gmt.sh $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/gmt-*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/gmt-*.exe $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	scp ../ftp/gawk*.zip $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	scp ../ftp/netcdf-3.6.3_win.zip $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	scp ../ftp/Whale-watching_symbols_for_GMT.zip $(FTPSERVER2):$(LIVEFTPDIR2)/contrib

update-gshhs-test:
#	ssh $(FTPSERVER2) 'rm -rf $(LIVEFTPDIR2)/gshhs-*'
	scp ../ftp/gshhs-*.bz2 $(FTPSERVER2):$(LIVEFTPDIR2)
#	scp ../ftp/gshhs-*.exe $(FTPSERVER2):$(LIVEFTPDIR2)/windows

update-ftp-local:
	if [ `cat ../ftp/.message | wc -c | awk '{printf "%d\n", $$1}'` -ge 1300 ]; then \
		echo -n "The .message file byte length is over 1300 bytes: "; cat ../ftp/.message | wc -c; \
		exit; \
	fi
	rm -rf ftp
	mkdir -p ftp/windows ftp/contrib
	sed -e 'sXpub/gmtXpub/pwessel/gmttestXg' ../guru/install_gmt.sh > ftp/install_gmt.sh
	cp -f ../ftp/gmt-*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION ftp
	cp -f ../ftp/gmt-*.exe ftp/windows
	cp -f ../ftp/gawk*.zip ftp/windows
	cp -f ../ftp/netcdf-3.6.3_win.zip ftp/windows
	cp -f ../ftp/Whale-watching_symbols_for_GMT.zip ftp/contrib

update-gshhs-local:
	rm -rf ftp/gshhs-*
	cp -f ../ftp/gshhs-*.bz2 ftp
#	cp -f ../ftp/gshhs-*.exe ftp/windows

clean-ftp-local:
	rm -rf ftp

clean-www-local:
	rm -rf www

clean-local:	clean-ftp-local clean-www-local

clean-test:
	ssh $(FTPSERVER2) rm -rf $(LIVEFTPDIR2)/*
	ssh $(WEBSERVER2) rm -rf $(LIVEWWWDIR2)/gmt $(LIVEWWWDIR2)/gmt.html $(LIVEWWWDIR2)/index.html

spotless:
	rm -f gmt_windows_[A-Z]*.html

netcdf: ../ftp/netcdf.tar.bz2

../ftp/netcdf.tar.bz2:
	cd ../ftp; \
	curl http://www.unidata.ucar.edu/downloads/netcdf/ftp/netcdf.tar.gz --remote-name --silent; \
	gunzip netcdf.tar.gz; \
	bzip2 -9 netcdf.tar

gmt_mirrors.html:	gmt_hitmap.sh
	gmt_hitmap.sh -s

gmt_hitmap.png: 	gmt_hitmap.sh
	gmt_hitmap.sh -s

hitmap:
	gmt_hitmap.sh -s

../registration/gmt_usage.jpg:
		cp -f $(GMT4)/registration/gmt_usage.jpg ../registration

www/gmt/gmt_releases.html:	gmt_releases_template.html
	mkdir -p www/gmt
	cp -f gmt_releases_template.html www/gmt/gmt_releases.html
