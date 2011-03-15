#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.92 2011-03-15 02:06:35 guru Exp $
#
#	GNUmakefile for GMT Version 5.x src directory
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		29-MAY-2010
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

# All the files that need to be build before compilation

FILES = gmt_ellipsoids.h gmt_datums.h Ellipsoids.i Fonts.i gmt_grdformats.h grdreformat.h gmt_keywords.h gmt_keycases.h \
	Colors.i gmt_color_rgb.h gmt_colornames.h gmt_dimensions.h grdinfo.h \
	gmtmath_func.h gmtmath_op.h gmtmath_explain.h gmtmath_man.i \
	grdmath_func.h grdmath_op.h grdmath_explain.h grdmath_man.i \
	grdreformat_man.i gmtapi_errstr.h gmtapi_errno.h \
	$(API_H) $(API_C) gmt_prognames.h gmt_progcases.h gmt_progpurpose.h

gmtmacros FILES:	$(FILES)

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

# Must create gmt_grdformats.h, grdreformat.h, and grdinfo.h from Grdformats.txt

gmt_grdformats.h:	Grdformats.txt
			(egrep DEFAULT Grdformats.txt ; (egrep -v '#|^  -' Grdformats.txt | sort -n)) | $(AWK) '{printf "\t{ %c%c%c, %c%c%c },\n", 39, substr($$3,1,1), 39, 39, substr($$3,2,1), 39}' | sed '$$s/},/}/' > $@

grdreformat.h:		Grdformats.txt
			grep -v '#' Grdformats.txt |  $(AWK) '{printf "\t\t\"%s\",\n", substr($$0,5)}'  > tmp0.txt
			echo "/* grdreformat.h	[Created by src/GNUmakefile] */" > $@
			wc -l tmp0.txt | $(AWK) '{printf "#define N_GRDTXT_LINES %d\n\tchar *grd_formats[N_GRDTXT_LINES] = {\n", $$1}' >> $@
			cat tmp0.txt >> $@
			echo "		};" >> $@
			rm -f tmp0.txt

grdreformat_man.i:	Grdformats.txt
			egrep -v '#' Grdformats.txt |  $(AWK) '{if ($$3 == "ID") {printf ".sp\n\\fIid\t%s\\fP\n.br\n", substr($$0, 26,length($$0)-27)} else if ($$1 != "-") {printf "\\fB%s\\fP\t%s\n.br\n", $$3, substr($$0,12,length($$0)-13)}}'  > $@

grdinfo.h:		Grdformats.txt
			grep -v '#' Grdformats.txt |  $(AWK) '{if ($$1 != "-") {printf "%d\t\"%s\",\n", $$1-1, substr($$0,12,length($$0)-12)}}' | sort -n -k1 | cut -f2 > tmp0.txt
			echo "/* grdinfo.h	[Created by src/GNUmakefile] */" > $@
			wc -l tmp0.txt | $(AWK) '{printf "#define N_GRD_TYPES %d\nchar *GMT_grd_type[N_GRD_TYPES] = {\n", $$1}' >> $@
			cat tmp0.txt >> $@
			echo "};" >> $@
			rm -f tmp0.txt

# Must create gmt_prognames.h, gmtprogs_inc.h and gmt_progpurpose.h for GMT.c from GMTprogs.txt

gmt_prognames.h:	GMTprogs.txt
			egrep -v '#' GMTprogs.txt | $(AWK) '{printf "{\"%s\",\t%s},\n", $$1, $$2}' > gmt_prognames.h

gmt_progcases.h:	GMTprogs.txt
			egrep -v '#' GMTprogs.txt | $(AWK) '{printf "\t\tcase %d:\n\t\t\tfunc = (PFL)GMT_%s_cmd;\n\t\t\t*mode = %s;\n\t\t\tbreak;\n", NR-1, $$1, $$2}' > gmt_progcases.h

gmt_progpurpose.h:	$(FUNCS_C)
			grep -h "%s \[API\]" *_func.c | sed -e 's/GMT_message (GMT/fprintf (stderr/g' -e 's/\[API\] //g' -e 's/\\n\\n/\\n/g' -e 's/ %s/%s/g' -e 's/GMT_VERSION/""/g' > gmt_progpurpose.h

# Add all the dimensions to gmt_dimensions.h

gmt_dimensions.h:	gmt_colornames.h gmt_datums.h gmt_ellipsoids.h gmt_grdformats.h gmt_keywords.h \
			gmt_media_name.h gmt_pennames.h gmt_unique.h gmt_prognames.h
			printf "#define GMT_N_COLOR_NAMES\t%d\t/* Lines in %s */\n" `wc -l gmt_colornames.h` > $@
			printf "#define GMT_N_DATUMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_datums.h` >> $@
			printf "#define GMT_N_ELLIPSOIDS\t%d\t/* Lines in %s */\n" `wc -l gmt_ellipsoids.h` >> $@
			printf "#define GMT_N_GRD_FORMATS\t%d\t/* Lines in %s */\n" `wc -l gmt_grdformats.h` >> $@
			printf "#define GMT_N_KEYS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_keywords.h` >> $@
			printf "#define GMT_N_MEDIA\t\t%d\t/* Lines in %s */\n" `wc -l gmt_media_name.h` >> $@
			printf "#define GMT_N_PEN_NAMES\t\t%d\t/* Lines in %s */\n" `wc -l gmt_pennames.h` >> $@
			printf "#define GMT_N_UNIQUE\t\t%d\t/* Lines in %s */\n" `wc -l gmt_unique.h` >> $@
			printf "#define GMT_N_PROGRAMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_prognames.h` >> $@

# Must create g[rd|mt]math.h from g[rd|mt]math.c

gmtmath_func.h gmtmath_op.h gmtmath_explain.h gmtmath_man.i:	make_math.sh gmtmath_func.c
			$(SHELL) make_math.sh gmt

grdmath_func.h grdmath_op.h grdmath_explain.h grdmath_man.i:	make_math.sh grdmath_func.c
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
			\rm -f $(FILES)
