#-------------------------------------------------------------------------------
#  $Id: Makefile,v 1.61 2011-03-25 01:32:31 remko Exp $
#
#		 Guru Makefile for GMT Version 4.x
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
#	Date:		20-SEPT-2009
#-------------------------------------------------------------------------------
#	Default setup
#-------------------------------------------------------------------------------

include ../src/config.mk		# GMT-specific settings determined by user
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
		
# Place stuff in the GMT website/www directory:

www/gmt/examples/ex01/example_01_100dpi.png:	../doc/examples/ex01/example_01.ps
	sh webexamples.sh -s

../doc/examples/ex01/example_01.ps:	../doc/examples/ex01/job01.sh
	( cd ..; $(MAKE) -f GNUmakefile examples)

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
		ssh $(WEBSERVER) rm -rf $(LIVEWWWDIR)/gmt $(LIVEWWWDIR)/index.html
		scp -r www/gmt $(WEBSERVER):$(LIVEWWWDIR)
		scp www/index.html $(WEBSERVER):$(LIVEWWWDIR)
#	Update the Perl script in the CGI bin dir
		scp install_gmt_form.pl $(WEBSERVER):$(LIVECGIDIR)/install_gmt_form.pl

update-www-test:	update-www-local
#	First completely remove the test www files then copy then from the www-local files
		ssh $(WEBSERVER2) rm -rf $(LIVEWWWDIR2)/gmt $(LIVEWWWDIR2)/index.html
		scp -r www/gmt $(WEBSERVER2):$(LIVEWWWDIR2)
		scp www/index.html $(WEBSERVER2):$(LIVEWWWDIR2)
#	For the test we want GMT_ftpsite to always be 0, and to use the test web site - hence these sed commands
		echo "sB/gmt/B/gmttest/Bg" > job.sed
		echo "sBgmt.soest.hawaii.edu/gmttempBwww.soest.hawaii.edu/gmttest/gmttempBg" >> job.sed
		echo 's/"GMT_ftpsite=", \$$site/"GMT_ftpsite=0"/g' >> job.sed
		sed -f job.sed install_gmt_form.pl > install_gmt_form_test.pl
		scp install_gmt_form_test.pl $(WEBSERVER2):$(LIVECGIDIR2)
		echo "sBftp.soest.hawaii.edu/gmtBftp.soest.hawaii.edu/gmttestBg" > job.sed
		sed -f job.sed gmt_windows.html > gmt_windows_test.html
		rm -f install_gmt_form_test.pl job.sed
#	Change the Href to the ftp server to go to the test server dir instead
		sed -e sBftp.soest.hawaii.edu/gmtBftp.soest.hawaii.edu/gmttestBg gmt_install_form.html > tmp
		scp tmp $(WEBSERVER2):$(LIVEWWWDIR2)/gmt/gmt_install_form.html
		scp gmt_windows_test.html $(WEBSERVER2):$(LIVEWWWDIR2)/gmt/gmt_windows.html
		echo "cd $(LIVEWWWDIR2)/gmt" > gmt_server_job.sh
		echo "sed -e s/install_gmt_form.pl/install_gmt_form_test.pl/g gmt_install_form.html > tmp" >> gmt_server_job.sh
		echo "mv -f tmp gmt_install_form.html" >> gmt_server_job.sh
		echo "cp -f ../../gmt/gmt/ChangeLog ." >> gmt_server_job.sh
		echo "rm -f gmt_server_job.sh" >> gmt_server_job.sh
		scp gmt_server_job.sh $(WEBSERVER2):$(LIVEWWWDIR2)/gmt
		ssh $(WEBSERVER2) sh $(LIVEWWWDIR2)/gmt/gmt_server_job.sh
		rm -f gmt_server_job.sh tmp

update-www-local:	../registration/gmt_usage.jpg gmt_mirrors.html www/gmt/gmt_releases.html www/gmt/examples/ex01/example_01_100dpi.png
		mkdir -p www/gmt
		cp -f gmt.html www/index.html
		cp -f gmt_*.php gmt_*.html www/gmt
		rm -f www/gmt/gmt_releases_template.html
		cp -f newgmtguru www/gmt
		cp -f *.gif linux.jpg osx.jpg unix.jpg vista.jpg www/gmt
		cp -f gmt_*.png ../registration/gmt_usage.jpg www/gmt
		cp -f soest_*.png www/gmt
		cp -f ../src/README.TRIANGLE www/gmt
		rm -r -f www/gmt/doc/*
		cp -R -f doc/html doc/pdf www/gmt/doc
		for ex in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29; do \
			cd www/gmt/examples/ex$$ex; \
			if [ -f example_$$ex.ps ]; then \
				zip -r example_$$ex.zip example_$$ex.ps;\
			fi; \
			cd ../../../.. ;\
		done
		cp -R -f contrib www/gmt
		rm -f www/gmt/examples/ex??/example_*.ps
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
	ssh $(FTPSERVER) 'rm -rf $(LIVEFTPDIR)/windows $(LIVEFTPDIR)/contrib $(LIVEFTPDIR)/*GMT* $(LIVEFTPDIR)/install_gmt $(LIVEFTPDIR)/VERSION'
	ssh $(FTPSERVER) mkdir -p $(LIVEFTPDIR)/windows $(LIVEFTPDIR)/contrib
	scp ../guru/install_gmt $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/GMT*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER):$(LIVEFTPDIR)
	scp ../ftp/GMT*.exe $(FTPSERVER):$(LIVEFTPDIR)/windows
	scp ../ftp/gawk*.zip $(FTPSERVER):$(LIVEFTPDIR)/windows
	echo "cd $(LIVEFTPDIR)" > gmt_server_job.sh
	for p in doc share src suppl triangle; do \
		echo "rm -f GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
		echo "$(LN_S) GMT$(GMT_VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
	done
	for p in basic gdal pdf; do \
		echo "rm -f windows/GMT_$${p}_install.exe" >> gmt_server_job.sh; \
		echo "(cd windows; $(LN_S) GMT$(GMT_VERSION)_$${p}_install.exe GMT_$${p}_install.exe)" >> gmt_server_job.sh; \
	done
	echo "rm -f $(LIVEFTPDIR)/gmt_server_job.sh" >> gmt_server_job.sh
	scp gmt_server_job.sh $(FTPSERVER):$(LIVEFTPDIR)
	ssh $(FTPSERVER) sh $(LIVEFTPDIR)/gmt_server_job.sh
	rm -f gmt_server_job.sh

update-gshhs-live:
	ssh $(FTPSERVER) 'rm -rf $(LIVEFTPDIR)/*GSHHS*'
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
	ssh $(FTPSERVER2) 'rm -rf $(LIVEFTPDIR2)/windows $(LIVEFTPDIR2)/contrib $(LIVEFTPDIR2)/*GMT* $(LIVEFTPDIR2)/install_gmt $(LIVEFTPDIR2)/VERSION'
	ssh $(FTPSERVER2) mkdir -p $(LIVEFTPDIR2)/windows $(LIVEFTPDIR2)/contrib
	scp ../guru/install_gmt $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/GMT*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION $(FTPSERVER2):$(LIVEFTPDIR2)
	scp ../ftp/GMT*.exe $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	scp ../ftp/gawk*.zip $(FTPSERVER2):$(LIVEFTPDIR2)/windows
	echo "cd $(LIVEFTPDIR2)" > gmt_server_job.sh
	for p in doc share src suppl triangle; do \
		echo "rm -f GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
		echo "$(LN_S) GMT$(GMT_VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2" >> gmt_server_job.sh; \
	done
	for p in basic gdal pdf; do \
		echo "rm -f windows/GMT_$${p}_install.exe" >> gmt_server_job.sh; \
		echo "(cd windows; $(LN_S) GMT$(GMT_VERSION)_$${p}_install.exe GMT_$${p}_install.exe)" >> gmt_server_job.sh; \
	done
	echo "rm -f $(LIVEFTPDIR2)/gmt_server_job.sh" >> gmt_server_job.sh
	scp gmt_server_job.sh $(FTPSERVER2):$(LIVEFTPDIR2)
	ssh $(FTPSERVER2) sh $(LIVEFTPDIR2)/gmt_server_job.sh
	rm -f gmt_server_job.sh

update-gshhs-test:
	ssh $(FTPSERVER2) 'rm -rf $(LIVEFTPDIR2)/*GSHHS*'
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
	rm -rf ftp
	mkdir -p ftp/windows ftp/contrib
	sed -e 'sXpub/gmtXpub/pwessel/gmttestXg' ../guru/install_gmt > ftp/install_gmt
	cp -f ../ftp/GMT*.bz2 ../ftp/.message ../ftp/README.GMT ../ftp/VERSION ftp
	cp -f ../ftp/GMT*.exe ftp/windows
	cp -f ../ftp/gawk*.zip ftp/windows
	for p in doc share src suppl triangle; do \
		\rm -f ftp/GMT_$$p.tar.bz2; \
		(cd ftp; $(LN_S) GMT$(GMT_VERSION)_$$p.tar.bz2 GMT_$$p.tar.bz2); \
	done
	for p in basic gdal pdf; do \
		rm -f windows/GMT_$$p_install.exe; \
		(cd ftp/windows; $(LN_S) GMT$(GMT_VERSION)_$${p}_install.exe GMT_$${p}_install.exe); \
	done

update-gshhs-local:
	rm -rf ftp/GSHHS*
	cp -f ../ftp/GSHHS*.bz2 ftp
	cp -f ../ftp/GSHHS*.exe ftp/windows
	for p in coast high full; do \
		\rm -f ftp/{GMT,GSHHS}_$$p.tar.bz2; \
		(cd ftp; $(LN_S) GSHHS$(GSHHS_VERSION)_$$p.tar.bz2 GSHHS_$$p.tar.bz2); \
	done
	rm -f windows/GSHHS_highfull_install.exe
	(cd ftp/windows; $(LN_S) GSHHS$(GSHHS_VERSION)_highfull_install.exe GSHHS_highfull_install.exe)

clean-ftp:
	rm -f ../ftp/G*.bz2

clean-local:
	rm -rf ftp
	rm -rf www

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

www/gmt/gmt_releases.html:	gmt_releases_template.html ../doc/html/GMT_Docs/GMT_Docs.html
	mkdir -p www/gmt
	(cd www/gmt; grep -H "scientists since 1988" ../../../doc/html/GMT_Docs/*.html | awk '{print $$1}' | tr '/:' '  ' | awk '{printf "s:CHAPTER1LINK:doc/html/GMT_Docs/%s:g\n", $$NF}' > /tmp/tmp.sed)
	sed -f /tmp/tmp.sed < gmt_releases_template.html > www/gmt/gmt_releases.html
	
	
