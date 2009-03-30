#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.80 2009-03-30 23:35:12 remko Exp $
#
#	GNUmakefile for GMT Version 4.x src directory
#
#	!!! THIS MAKEFILE IS FOR GMT DEVELOPERS ONLY !!!
#
#	This makefile extends the regular makefile by adding the
#	dependencies needed to generate certain source files from
#	lower-level data files available in CVS but not in archives.
#
#	Author:	Paul Wessel, SOEST, University of Hawaii
#
#	Date:		29-MAY-2008
#-------------------------------------------------------------------------------

include makefile

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

FILES = gmt_ellipsoids.h gmt_datums.h gmtdefaults.i gmt_grdformats.h grdreformat.h gmt_keywords.h gmt_keycases.h \
	gmtcolors.i gmt_color_rgb.h gmt_colornames.h gmt_dimensions.h \
	gmtmath.h gmtmath_def.h gmtmath_explain.h gmtmath_man.i \
	grdmath.h grdmath_def.h grdmath_explain.h grdmath_man.i grdreformat_man.i

gmtmacros FILES:	$(FILES)

# Must create gmt_key*.h from gmt_keywords.d

gmt_keywords.h:		gmt_keywords.d
			grep -v '^#' gmt_keywords.d | awk '{printf "\"%s\",\n", $$1}' | sed '$$s/,$$//' > $@

gmt_keycases.h:		gmt_keywords.d
			grep -v '^#' gmt_keywords.d | awk '{printf "#define GMTCASE_%s\t%d\n", $$1, NR-1}' > $@

# Must create gmtcolors.i, gmt_color_rgb.h and gmt_colornames.h from Colors.txt

gmtcolors.i:		Colors.txt
			grep -v '^#' Colors.txt | LANG=C awk '{printf ".br\n%3i %3i %3i\t%s\n", $$1, $$2, $$3, $$4}' > $@

gmt_color_rgb.h:	Colors.txt
			grep -v '^#' Colors.txt | LANG=C awk '{printf "{ %3i, %3i, %3i },\n", $$1, $$2, $$3}' | sed '$$s/},/}/' > $@

gmt_colornames.h:	Colors.txt
			grep -v '^#' Colors.txt | LANG=C awk '{printf "\"%s\",\n", $$4}' | sed '$$s/,$$//' > $@

# Must create gmt_ellipsoids.h and gmtdefaults.i from Ellipsoids.txt

gmt_ellipsoids.h:	Ellipsoids.txt
			grep -v '^#' Ellipsoids.txt | LANG=C awk '{{printf "\t\t{ \"%s\", %d, %s, ", $$1, $$2, $$3}; \
			if ($$4 == 0) {printf "0.0},\n"} else if ($$4 < 0.5*$$3) {printf "1.0/%s},\n", $$4} else \
			{printf "1.0-%s/%s},\n", $$4, $$3}} END{printf "\t\t{ \"Custom\", 0, 6371008.7714, 0.0}\n"}' > $@

gmtdefaults.i:		Ellipsoids.txt
			grep -v '^#' Ellipsoids.txt > tmp0.txt
			awk '{printf "%s\t%d\n", $$1, $$2}' tmp0.txt > tmp1.txt
			awk -F: '{print $$2}' tmp0.txt | sed -e 's/^ //g' > tmp2.txt
			paste tmp1.txt tmp2.txt | awk '{printf "%s\n.br\n", $$0}' > gmtdefaults.i
			rm -f tmp[012].txt

# Must create gmt_datums.h from Datums.txt

gmt_datums.h:		Datums.txt
			grep -v '^#' Datums.txt | awk -F'	' '{printf "\t\t{ \"%s\", \"%s\", \"%s\", {%s, %s, %s}},\n", $$1, $$2, $$6, $$3, $$4, $$5}' | sed '$$s/},/}/' > $@

# Must create gmt_grdformats.h and grdreformat.h from Grdformats.txt

gmt_grdformats.h:	Grdformats.txt
			(egrep DEFAULT Grdformats.txt ; (egrep -v '#|^  -' Grdformats.txt | sort -n)) | awk '{printf "\t{ %c%c%c, %c%c%c },\n", 39, substr($$3,1,1), 39, 39, substr($$3,2,1), 39}' | sed '$$s/},/}/' > $@

grdreformat.h:		Grdformats.txt
			grep -v '#' Grdformats.txt |  awk '{printf "\t\t\"%s\",\n", substr($$0,5)}'  > tmp0.txt
			echo "/* grdreformat.h	[Created by src/GNUmakefile] */" > $@
			wc -l tmp0.txt | awk '{printf "#define N_GRDTXT_LINES %d\n\tchar *grd_formats[N_GRDTXT_LINES] = {\n", $$1}' >> $@
			cat tmp0.txt >> $@
			echo "		};" >> $@
			rm -f tmp0.txt

grdreformat_man.i:	Grdformats.txt
			egrep -v '#' Grdformats.txt |  awk '{if ($$3 == "ID") {printf ".sp\n\\fIid\t%s\\fP\n.br\n", substr($$0, 26,length($$0)-27)} else if ($$1 != "-") {printf "\\fB%s\\fP\t%s\n.br\n", $$3, substr($$0,12,length($$0)-13)}}'  > $@

# Add all the dimensions to gmt_dimensions.h

gmt_dimensions.h:	gmt_colornames.h gmt_datums.h gmt_ellipsoids.h gmt_grdformats.h gmt_keywords.h \
			gmt_media_name.h gmt_pennames.h gmt_unique.h
			printf "#define GMT_N_COLOR_NAMES\t%d\t/* Lines in %s */\n" `wc -l gmt_colornames.h` > $@
			printf "#define GMT_N_DATUMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_datums.h` >> $@
			printf "#define GMT_N_ELLIPSOIDS\t%d\t/* Lines in %s */\n" `wc -l gmt_ellipsoids.h` >> $@
			printf "#define GMT_N_GRD_FORMATS\t%d\t/* Lines in %s */\n" `wc -l gmt_grdformats.h` >> $@
			printf "#define GMT_N_KEYS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_keywords.h` >> $@
			printf "#define GMT_N_MEDIA\t\t%d\t/* Lines in %s */\n" `wc -l gmt_media_name.h` >> $@
			printf "#define GMT_N_PEN_NAMES\t\t%d\t/* Lines in %s */\n" `wc -l gmt_pennames.h` >> $@
			printf "#define GMT_N_UNIQUE\t\t%d\t/* Lines in %s */\n" `wc -l gmt_unique.h` >> $@

# Must create g[rd|mt]math.h from g[rd|mt]math.c

gmtmath.h gmtmath_def.h gmtmath_explain.h gmtmath_man.i:	make_math.sh gmtmath.c
			$(SHELL) make_math.sh gmt

grdmath.h grdmath_def.h grdmath_explain.h grdmath_man.i:	make_math.sh grdmath.c
			$(SHELL) make_math.sh grd

# When doing "spotless" make sure at least a dummy makegmt.macros exists.

ifeq "$(MAKECMDGOALS)" "spotless"
makegmt.macros:
			touch $@
else
makegmt.macros:		makegmt.macros.in
			cd ..; $(MAKE) src/makegmt.macros
endif

# Must create manpages before install-man

include manpages.mk

# How to clean it all up

spotless::
			\rm -f $(FILES)
