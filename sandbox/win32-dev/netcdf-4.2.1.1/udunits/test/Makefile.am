# Copyright 2008, 2009 University Corporation for Atmospheric Research
#
# This file is part of the UDUNITS-2 package.  See the file LICENSE
# in the top-level source-directory of the package for copying and
# redistribution conditions.
#
## Process this file with automake to produce Makefile.in
EXTRA_DIST	= results.tab redo-results
distName	= $(PACKAGE)-$(VERSION)
distArchive	= $(top_srcdir)/$(distName).tar.gz
topTmpDir	= /tmp/$(distName)

# The following rule is executed on the remote system.  Setting
# MAKEFILES to the empty string on the make(1) invocation line appears
# to be necessary due to a bug in the AIX 5.1 make(1).
hostcheck: $(distArchive)
	-test -d $(topTmpDir) && \
	    (chmod -R +w $(topTmpDir) && rm -rf $(topTmpDir))
	gunzip -c $(distArchive) | \
	    (mkdir $(topTmpDir) && cd $(topTmpDir) && pax -r)
	if cd $(topTmpDir)/$(distName); then \
	    ./configure --prefix=$(topTmpDir) $(SHARED_OPT) CC='$(CC)'; \
	    status=$$?; \
	    if test $$status -eq 0; then \
		MAKEFLAGS= $(MAKE) all check install install-html \
		    install-pdf uninstall; \
		status=$$?; \
	    fi; \
	    cd /tmp; \
	    rm -rf $(topTmpDir); \
	    exit $$status; \
	fi

# The following rule starts a single synchronous build process on a remote
# system.
singleCheck:
	ssh $(host) $(shell) "'cd $$PWD && $(make) MAKE=\"$(make)\" CC=\"$(cc)\" SHARED_OPT=$(sharedOpt) hostcheck'" </dev/null

# The following rule starts multiple, synchronous build processes on a remote
# system.
.host.results:
	rm -f $*.results $*.log
	IFS=':' read uname shell <$<; \
	sed -n '2,$$p' $< | \
	while IFS=':' read make cc; do \
	    if $(MAKE) host="$*" shell="$$shell" make="$$make" \
		    cc="$$cc" sharedOpt='' singleCheck >>$*.log 2>&1 ; then \
		echo "$$make:$$cc::1" >>$*.results; \
	    else \
		echo $$make:$$cc::0 >>$*.results; \
	    fi; \
	    if $(MAKE) host="$*" shell="$$shell" make="$$make" \
		    cc="$$cc" sharedOpt=--disable-shared singleCheck \
			>>$*.log 2>&1; then \
		echo $$make:$$cc:--disable-shared:1 >>$*.results; \
	    else \
		echo $$make:$$cc:--disable-shared:0 >>$*.results; \
	    fi; \
	done
	touch redo-results

# The following rule starts multiple, concurrent, asynchronous builds on 
# remote systems.
hostchecks:	$(distArchive)
	rm -f $@
	hosts=`ls *.host | sed 's/.host//'`; \
	for host in $$hosts; do \
	    rm -f $$host.results; \
	    $(MAKE) -s $$host.results >/dev/null & pid="$$!"; \
	    echo $$host $$pid >>$@; \
	    echo "Building on $$host"; \
	done; \
	status=0; \
	while read host pid; do \
	    echo "Waiting on $$host"; \
	    wait $$pid; \
	    stat=$$?; \
	    if test $$stat -ne 0; then \
		echo 1>&2 \
		    "Build on \"$$host\" exited with status $$stat.  " \
		    "See file \"$$host.log\"."; \
		status=$$(($$status+1)); \
	    fi; \
	done <$@; \
	echo "Done waiting"; \
	exit $$status

# hostname:os:make:cc:config_opts:exit_status
results.tab:	redo-results
	sleep 2	# to accomodate broken HP-UX B.11.00 make(1)
	ls *.results | sed 's/.results//' | \
	while read host; do \
	    IFS=':' read uname shell <$$host.host || break; \
	    id=`ssh $$host "$$uname" </dev/null` || break; \
	    sed -n "s/^/$$host:$$id:/p" $$host.results || break; \
	done >$@

remote-checks:	hostchecks results.tab

.PHONY:	singleCheck hostchecks hostcheck remote-checks

## The following entries are necessary to stop make(1)s that don't
## understand the .PHONY target from trying to create the given files.
install-info-am:
install-html-am:
install-pdf-am:
