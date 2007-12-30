#-------------------------------------------------------------------------------
#  $Id: GNUmakefile,v 1.71 2007-12-30 19:47:24 remko Exp $
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
#	Date:		04-NOV-2006
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

FILES = gmt_ellipsoids.h gmt_datums.h gmtdefaults.i gmt_grdformats.h gmt_keywords.h gmt_keycases.h gmt_dimensions.h \
	gmtmath.h gmtmath_def.h gmtmath_explain.h gmtmath_man.i \
	grdmath.h grdmath_def.h grdmath_explain.h grdmath_man.i
	
gmtmacros FILES:	$(FILES)

# Must create gmt_key*.h from gmt_keywords.d

gmt_keywords.h:		gmt_keywords.d
			grep -v '^#' gmt_keywords.d | awk '{printf "\"%s\",\n", $$1}' > $@

gmt_keycases.h:		gmt_keywords.d
			grep -v '^#' gmt_keywords.d | awk '{printf "#define GMTCASE_%s\t%d\n", $$1, NR-1}' > $@
		

# Must create gmt_ellipsoids.h and gmtdefaults.i from Ellipsoids.txt and gmt_datums.h from Datums.txt

gmt_ellipsoids.h:	Ellipsoids.txt
			grep -v '^#' Ellipsoids.txt | LANG=C awk '{if ($$4 == 0.0) {printf "\t\t{ \"%s\", %d, %s, 0.0, 0.0 },\n", $$1, $$2, $$3} else {printf "\t\t{ \"%s\", %d, %s, 0.0, 1.0/%s},\n", $$1, $$2, $$3, $$4}}' | sed '$$s/},/}/' > $@

gmtdefaults.i:		Ellipsoids.txt
			grep -v '^#' Ellipsoids.txt > tmp0.txt
			awk '{printf "%s\t%d\n", $$1, $$2}' tmp0.txt > tmp1.txt
			awk -F: '{print $$2}' tmp0.txt | sed -e 's/^ //g' > tmp2.txt
			paste tmp1.txt tmp2.txt | awk '{printf "%s\n.br\n", $$0}' > gmtdefaults.i
			rm -f tmp[012].txt

gmt_datums.h:		Datums.txt
			grep -v '^#' Datums.txt | awk -F'	' '{printf "\t\t{ \"%s\", \"%s\", \"%s\", {%s, %s, %s}},\n", $$1, $$2, $$6, $$3, $$4, $$5}' | sed '$$s/},/}/' > $@

# Must create gmt_grdformats.h from gmt_formats.conf

gmt_grdformats.h:	../share/conf/gmt_formats.conf
			(egrep DEFAULT ../share/conf/gmt_formats.conf ; (egrep -v '#|^  -' ../share/conf/gmt_formats.conf | sort -n)) | awk '{printf "\t{ %c%c%c, %c%c%c },\n", 39, substr($$3,1,1), 39, 39, substr($$3,2,1), 39}' > $@

# Add all the dimensions to gmt_dimensions.h

gmt_dimensions.h:	gmt_keywords.h gmt_ellipsoids.h gmt_datums.h gmt_grdformats.h gmt_pennames.h gmt_time_systems.h gmt_colornames.h
			printf "#define GMT_N_KEYS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_keywords.h` > $@
			printf "#define GMT_N_ELLIPSOIDS\t%d\t/* Lines in %s */\n" `wc -l gmt_ellipsoids.h` >> $@
			printf "#define GMT_N_DATUMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_datums.h` >> $@
			printf "#define GMT_N_GRD_FORMATS\t%d\t/* Lines in %s */\n" `wc -l gmt_grdformats.h` >> $@
			printf "#define GMT_N_PEN_NAMES\t\t%d\t/* Lines in %s */\n" `wc -l gmt_pennames.h` >> $@
			printf "#define GMT_N_TIME_SYSTEMS\t%d\t/* Lines in %s */\n" `wc -l gmt_time_systems.h` >> $@
			printf "#define GMT_N_COLOR_NAMES\t%d\t/* Lines in %s */\n" `wc -l gmt_colornames.h` >> $@

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
