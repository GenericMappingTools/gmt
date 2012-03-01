# Copyright 2008, 2009 University Corporation for Atmospheric Research
#
# This file is part of the UDUNITS-2 package.  See the file LICENSE
# in the top-level source-directory of the package for copying and
# redistribution conditions.
#
## Process this file with automake to produce Makefile.in
if COND_EXPAT
    SUBDIRS		= expat lib prog
else
    SUBDIRS		= lib prog
endif
DIST_SUBDIRS		= expat lib prog test
info_TEXINFOS		= udunits2.texi
udunits2_TEXINFOS	= success.texi failure.texi make.texi
EXTRA_DIST = \
    ANNOUNCEMENT \
    udunits2.html \
    udunits2.pdf \
    CHANGE_LOG \
    LICENSE \
    texinfo.tex
AM_MAKEINFOHTMLFLAGS = --no-split
RSYNC_FLAGS	= --rsh=ssh --rsync-path=/opt/bin/rsync
distName = $(PACKAGE)-$(VERSION)
distArchive = $(distName).tar.gz
DISTCLEANFILES	= *.log $(distArchive) make.texi failure.texi success.texi

success.texi:	test/results.tab
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)
	sort -u -t : -k 2,2 -k 4,4 -k 5,5 test/results.tab | \
	awk -F: ' \
	    BEGIN { \
		print "@multitable {@code{Linux 2.6.18-1.2257.fc5smp}} {@code{/opt/csw/gcc4/bin/gcc}} {@code{--disable-shared}}"; \
		print "@headitem O/S @tab Compiler @tab @code{configure} Option"; \
	    } \
	    $$6 == 1 { \
		printf "@item @code{%s} @tab @code{%s} @tab @code{%s}\n", \
		    $$2, $$4, $$5; \
	    } \
	    END { \
		print "@end multitable"; \
	    }' >success.texi
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)

failure.texi:	test/results.tab
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)
	sort -u -t : -k 2,2 -k 4,4 -k 5,5 test/results.tab | \
	awk -F: ' \
	    BEGIN { \
		print "@multitable {@code{Linux 2.6.18-1.2257.fc5smp}} {@code{/opt/SUNWspro/bin/c99}} {@code{--disable-shared}}"; \
		print "@headitem O/S @tab Compiler @tab @code{configure} Option"; \
	    } \
	    $$6 == 0 { \
		printf "@item @code{%s} @tab @code{%s} @tab @code{%s}\n", \
		    $$2, $$4, $$5; \
	    } \
	    END { \
		print "@end multitable"; \
	    }' >failure.texi
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)

make.texi:	test/results.tab
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)
	awk -F: '$$6 == 1 {printf "%s:%s\n", $$2, $$3}' test/results.tab | \
	sort -t : -u | \
	awk -F: ' \
	    BEGIN { \
		print "@multitable {@code{Linux 2.6.18-1.2257.fc5smp}} {@code{/usr/bin/posix/make}}"; \
		print "@headitem O/S @tab @code{make} Utility"; \
	    } \
	    { \
		printf "@item @code{%s} @tab @code{%s}\n", $$1, $$2; \
	    } \
	    END { \
		print "@end multitable"; \
	    }' >$@
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)

remote-checks:	dist
	cd test && $(MAKE) $(AM_MAKEFLAGS) $@
	$(MAKE) udunits2.html

ftp:	dist
	git status -a | grep -F -q 'nothing to commit'
	git tag -f v$(VERSION)
	scp $(distArchive) conan:/web/ftp/pub/$(PACKAGE)/
	mailx -s '$(distName) now available' -a CHANGE_LOG \
	    ed@unidata.ucar.edu </dev/null

web-update:	install install-html
	cd $(DESTDIR)$(htmldir) \
	&& rsync $(RSYNC_FLAGS) -aCu --delete --delete-excluded \
	    *.html \
	    conan:/content/software/$(PACKAGE)/$(distName)
	cd $(pkgdatadir) \
	&& rsync $(RSYNC_FLAGS) -aCu --delete \
	    *.xml \
	    conan:/content/software/$(PACKAGE)/$(distName)
	ssh conan rm /content/software/$(PACKAGE)/$(PACKAGE)-2
	ssh conan ln -s /content/software/$(PACKAGE)/$(distName) \
	    /content/software/$(PACKAGE)/$(PACKAGE)-2

download-update:
	ssh conan cat /content/downloads/udunits/toc.xml \
	    | sed -e \
		's/"%current%" value=".*"/"%current%" value="$(VERSION)"/' \
		>toc.xml.new
	scp toc.xml.new conan:/content/downloads/udunits/toc.xml.new
	ssh conan mv -f /content/downloads/$(PACKAGE)/toc.xml \
	    /content/downloads/$(PACKAGE)/toc.xml.old
	ssh conan mv -f /content/downloads/$(PACKAGE)/toc.xml.new \
	    /content/downloads/$(PACKAGE)/toc.xml
	rm toc.xml.new

available:	ftp web-update download-update

.PHONY:	hostchecks hostcheck remote-checks web-update ftp \
	download-update available

udunits2.info:	version.texi success.texi failure.texi make.texi LICENSE
udunits2.html:	version.texi success.texi failure.texi make.texi LICENSE
udunits2.pdf:	version.texi success.texi failure.texi make.texi LICENSE

$(srcdir)/version.texi:  $(srcdir)/stamp-vti
	@cp $(srcdir)/stamp-vti $@
