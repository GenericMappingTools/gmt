## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = -DHAVE_EXPAT_CONFIG_H
lib_LTLIBRARIES = libexpat.la
libexpat_la_SOURCES = \
    ascii.h \
    asciitab.h \
    expat.h \
    expat_external.h \
    iasciitab.h \
    internal.h \
    latin1tab.h \
    nametab.h \
    utf8tab.h \
    xmlparse.c \
    xmlrole.c \
    xmlrole.h \
    xmltok.c \
    xmltok.h \
    xmltok_impl.h
EXTRA_DIST = \
    xmltok_impl.c \
    xmltok_ns.c
AM_CTAGSFLAGS = -t

MOSTLYCLEANFILES = lint.log *.ln
LINTFLAGS	= -u
LINT		= lint

SUFFIXES = .c .l

.c.ln:
	$(LINT.c) $(CPPFLAGS) -c $<

.c.i:
	$(CPP) $(CPPFLAGS) $< >$@

## The following entries are necessary to stop make(1)s that don't
## understand the .PHONY target from trying to create the given files.
install-info-am:
install-html-am:
install-pdf-am:
