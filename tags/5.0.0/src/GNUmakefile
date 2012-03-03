#-------------------------------------------------------------------------------
#  $Id$
#
#	GNUmakefile for GMT Version 5.x src directory
#
#	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel and Remko Scharroo
#
#	Date:		1-AUG-2011
#-------------------------------------------------------------------------------

include Makefile

FUNCS_C         = $(FUNCS_O:.o=.c)

# To retain binaries for more than one architecture you can 
# say "make install2" instead of (or in addition to) "make install"
# The resulting executables, libraries, and include files will then
# be placed under the `arch`/{bin,lib,include} subdirs

bindir2		= $(rootdir)/`arch`/bin
libdir2		= $(rootdir)/`arch`/lib
includedir2	= $(rootdir)/`arch`/include

#-------------------------------------------------------------------------------
install2:	all
		$(MAKE) bindir=$(bindir2) libdir=$(libdir2) includedir=$(includedir2) install

uninstall2:
		$(MAKE) bindir=$(bindir2) libdir=$(libdir2) includedir=$(includedir2) uninstall

install2-suppl:
		for d in $(SUPPL); do \
			if [ -d $$d ] ; then \
				(cd $$d; $(MAKE) bindir=$(bindir2) install); \
			fi; \
		done

uninstall2-suppl:
		for d in $(SUPPL); do \
			if [ -d $$d ] ; then \
				(cd $$d; $(MAKE) bindir=$(bindir2) uninstall); \
			fi; \
		done

# All the files that need to be build before compilation, defined in the GNUmakefile (hence the dependency)

FILES = gmt_ellipsoids.h gmt_datums.h Ellipsoids.i Fonts.i gmt_grdkeys.h \
	gmt_keywords.h gmt_keycases.h \
	Colors.i gmt_color_rgb.h gmt_colornames.h gmt_dimensions.h \
	gmtmath.h gmtmath_op.h gmtmath_explain.h gmtmath_man.i \
	grdmath.h grdmath_op.h grdmath_explain.h grdmath_man.i \
	grdreformat_man.i gmtapi_errstr.h gmtapi_errno.h \
	$(API_H) gmt_prognames.h gmt_progcases.h gmt_progpurpose.h

gmtmacros FILES:	$(FILES)
$(FILES):		GNUmakefile

# Must create gmt_key*.h from gmt_keywords.txt

gmt_keywords.h:		gmt_keywords.txt
			cat gmt_keywords.txt gmt_keywords.d | grep -v '^#' | $(AWK) '{print $$1}' | sort -u | $(AWK) '{printf "\"%s\",\n", $$1}' | sed '$$s/,$$//' > $@

gmt_keycases.h:		gmt_keywords.txt
			cat gmt_keywords.txt gmt_keywords.d | grep -v '^#' | $(AWK) '{print $$1}' | sort -u | $(AWK) '{printf "#define GMTCASE_%s\t%d\n", $$1, NR-1}' > $@

# Must create Colors.i, gmt_color_rgb.h and gmt_colornames.h from Colors.txt

Colors.i:		Colors.txt
			grep -v '^#' Colors.txt | LANG=C $(AWK) '{printf ".br\n%3i\t%3i\t%3i\t%s\n", $$1, $$2, $$3, $$4}' > $@

gmt_color_rgb.h:	Colors.txt
			grep -v '^#' Colors.txt | LANG=C $(AWK) '{printf "{ %3i, %3i, %3i },\n", $$1, $$2, $$3}' | sed '$$s/},/}/' > $@

gmt_colornames.h:	Colors.txt
			grep -v '^#' Colors.txt | LANG=C $(AWK) '{printf "\"%s\",\n", $$4}' | sed '$$s/,$$//' > $@

# Must create gmt_ellipsoids.h and Ellipsoids.i from Ellipsoids.txt

gmt_ellipsoids.h:	Ellipsoids.txt
			grep -v '^#' Ellipsoids.txt | LANG=C $(AWK) '{{printf "\t\t{ \"%s\", %d, %s, ", $$1, $$2, $$3}; \
			if ($$4 == 0) {printf "0.0},\n"} else if ($$4 < 0.5*$$3) {printf "1.0/%s},\n", $$4} else \
			{printf "1.0-%s/%s},\n", $$4, $$3}} END{printf "\t\t{ \"Custom\", 0, 6371008.7714, 0.0}\n"}' > $@

Ellipsoids.i:		Ellipsoids.txt
			grep -v '^#' Ellipsoids.txt > tmp0.txt
			$(AWK) '{printf "%s :\n", $$1, $$2}' tmp0.txt > tmp1.txt
			$(AWK) -F: '{print $$2}' tmp0.txt | sed -e 's/^ //g' > tmp2.txt
			$(AWK) '{printf "(%d)\n", $$2}' tmp0.txt > tmp3.txt
			paste -d' ' tmp[123].txt | $(AWK) '{printf ".br\n%s\n", $$0}' > $@
			rm -f tmp[0123].txt

# Must create gmtapi_errstr.h and gmtapi_errno.h from gmtapi_errors.d

gmtapi_errstr.h:	gmtapi_errors.d
			grep -v '^#' gmtapi_errors.d | awk '{printf "\"%s\",\n", $$1}' > $@

gmtapi_errno.h:		gmtapi_errors.d
			grep -v '^#' gmtapi_errors.d | awk '{printf "#define %s\t%d\n", $$1, 1-NR}' > $@

# Must create Fonts.i from PS_font_info.d

Fonts.i:		../share/pslib/PS_font_info.d
			grep -v '^#' ../share/pslib/PS_font_info.d | $(AWK) '{printf ".br\n%i\t%s\n", NR - 1, $$1}' > $@

# Must create gmt_datums.h from Datums.txt

gmt_datums.h:		Datums.txt
			grep -v '^#' Datums.txt | $(AWK) -F'	' '{printf "\t\t{ \"%s\", \"%s\", \"%s\", {%s, %s, %s}},\n", $$1, $$2, $$6, $$3, $$4, $$5}' | sed '$$s/},/}/' > $@

# Must create gmt_grdkeys.h and grdreformat_man.i from Grdformats.txt

gmt_grdkeys.h:		gmt_customio.c
			egrep 'session.grdformat.* = ' gmt_customio.c | egrep -v 'not supported' | cut -d\" -f2 | \
			tr '[a-z]' '[A-Z]' | $(AWK) '{printf "#define GMT_GRD_IS_%s\t%d\n", $$1, NR-1}' > $@

grdreformat_man.i:	gmt_customio.c
			egrep 'session.grdformat.* = ' gmt_customio.c | egrep -v 'not supported' | cut -d\" -f2 | \
			$(AWK) '{printf "BD(%s)\t%s\n.br\n", $$1, substr($$0,6,length($$0)-5)}'  > $@

# Must create gmt_prognames.h, gmtprogs_inc.h and gmt_progpurpose.h for gmt.c from GMTprogs.txt

gmt_prognames.h:	GMTprogs.txt
			egrep -v '#' GMTprogs.txt | $(AWK) '{printf "{\"%s\",\t%s},\n", $$1, $$2}' > gmt_prognames.h

gmt_progcases.h:	GMTprogs.txt
			egrep -v '#' GMTprogs.txt | $(AWK) '{printf "\t\tcase %d:\n\t\t\tfunc = (PFL)GMT_%s;\n\t\t\t*mode = %s;\n\t\t\tbreak;\n", NR-1, $$1, $$2}' > gmt_progcases.h

gmt_progpurpose.h:	$(PROGS_C)
			grep -h "%s \[API\]" *.c | sed -e 's/GMT_message (GMT/fprintf (stderr/g' -e 's/\[API\] //g' -e 's/\\n\\n/\\n/g' -e 's/ %s/%s/g' -e 's/GMT_VERSION/""/g' > gmt_progpurpose.h

# Add all the dimensions to gmt_dimensions.h

gmt_dimensions.h:	gmt_colornames.h gmt_datums.h gmt_ellipsoids.h gmt_grdkeys.h gmt_keywords.h \
			gmt_media_name.h gmt_pennames.h gmt_unique.h gmt_prognames.h
			printf "#define GMT_N_COLOR_NAMES\t%d\t/* Lines in %s */\n" `wc -l gmt_colornames.h` > $@
			printf "#define GMT_N_DATUMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_datums.h` >> $@
			printf "#define GMT_N_ELLIPSOIDS\t%d\t/* Lines in %s */\n" `wc -l gmt_ellipsoids.h` >> $@
			printf "#define GMT_N_GRD_FORMATS\t%d\t/* Lines in %s */\n" `wc -l gmt_grdkeys.h` >> $@
			printf "#define GMT_N_KEYS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_keywords.h` >> $@
			printf "#define GMT_N_MEDIA\t\t%d\t/* Lines in %s */\n" `wc -l gmt_media_name.h` >> $@
			printf "#define GMT_N_PEN_NAMES\t\t%d\t/* Lines in %s */\n" `wc -l gmt_pennames.h` >> $@
			printf "#define GMT_N_UNIQUE\t\t%d\t/* Lines in %s */\n" `wc -l gmt_unique.h` >> $@
			printf "#define GMT_N_PROGRAMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_prognames.h` >> $@

# Must create g[rd|mt]math.h from g[rd|mt]math.c

gmtmath.h gmtmath_op.h gmtmath_explain.h gmtmath_man.i:	make_math.sh gmtmath.c
			$(SHELL) make_math.sh gmt

grdmath.h grdmath_op.h grdmath_explain.h grdmath_man.i:	make_math.sh grdmath.c
			$(SHELL) make_math.sh grd

# When doing "spotless" make sure at least a dummy config.mk exists.

ifeq "$(MAKECMDGOALS)" "spotless"
config.mk:
			touch $@
else
config.mk:		config.mk.in
			cd ..; $(MAKE) src/config.mk
endif

# Must create manpages before install-man

include manpages.mk

# How to clean it all up

spotless::
			rm -f $(FILES)
