#-------	$Id: GNUmakefile,v 1.13 2001-09-18 01:43:10 pwessel Exp $	--------------------
#
#		Makefile for GMT Version 3.5
#			GNU make compatible
#	[GNU make always looks for GNUmakefile before Makefile/makefile]
#
#	To ensure the very latest code, start with "make spotless".
#	The gentler "make clean" will NOT remove code that is generated
#	by GMT scripts, like gmt_notposix.h and grdmath.c
#	To compile/link them, try "make all", then "make install".
#	When done, clean out directory with "make clean". If the example
#	scripst have been installed in ../examples, try "make examples".
#
#	Authors:	Paul Wessel, SOEST, University of Hawaii
#			Walter H. F. Smith, Lab for Satellite Altimetry, NOAA
#
#	Date:		14-AUG-2001
#-------------------------------------------------------------------------------
#	Default setup
#-------------------------------------------------------------------------------

-include makegmt.macros	# GMT specific settings created by user

# These directories can be set to have a 2nd place to install executables
# other than your chosen install directories.  Edit these directories
# and say "make install2"

bindir2		= /home/pahoehoe/gmt/GMT3.4/bin
libdir2		= /home/pahoehoe/gmt/GMT3.4/lib
includedir2	= /home/pahoehoe/gmt/GMT3.4/include

#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------
NETCDFLIB	= $(NETCDF)/lib
NETCDFINC	= $(NETCDF)/include
CFLAGS    	= -I$(NETCDFINC) -DDEBUG $(WIN32) $(CC_OPT) \
		  -DGMT_DEFAULT_PATH=\"$(GMT_DEFAULT_PATH)\" $(TRIANGLE_D)
CDF	  	= -L$(NETCDFLIB) -lnetcdf

PS	= -lpsl
GMT	= -lgmt

LIB_C	= fourt.c \
	  gmt_calclock.c \
	  gmt_cdf.c \
	  gmt_customio.c \
	  gmt_grdio.c \
	  gmt_init.c \
	  gmt_io.c \
	  gmt_map.c \
	  gmt_plot.c \
	  gmt_shore.c \
	  gmt_stat.c \
	  gmt_support.c \
	  gmt_vector.c

GMT_H	= gmt.h \
	  gmt_calclock.h \
	  gmt_time_systems.h \
	  gmt_math.h \
	  gmt_nan.h \
	  gmt_map.h \
	  gmt_boundcond.h \
	  gmt_shore.h \
	  gmt_project.h \
	  gmt_grd.h \
	  gmt_funcnames.h \
	  gmt_colors.h \
	  gmt_grdio.h \
	  gmt_customio.h \
	  gmt_bcr.h \
	  gmt_symbol.h \
	  gmt_unique.h \
	  gmt_keywords.h \
	  gmt_io.h \
	  gmt_notposix.h \
	  gmt_notunix.h \
	  pslib.h

GMT_I	= gmt_init.h \
	  gmt_defaults.h \
	  gmt_media_name.h \
	  gmt_media_size.h \
	  PS_font_names.h \
	  PS_font_heights.h

PS_H	= pslib.h \
	  pslib_inc.h \
	  PS_font_names.h \
	  PS_font_heights.h

PROGS_C	= blockmean.c blockmedian.c blockmode.c filter1d.c fitcircle.c grdfilter.c gmtconvert.c gmtdefaults.c gmtmath.c gmtselect.c \
	  gmtset.c gmt2rgb.c grd2cpt.c grd2xyz.c grdcontour.c grdcut.c grdclip.c grdedit.c grdfft.c \
	  grdgradient.c grdhisteq.c grdimage.c grdinfo.c grdlandmask.c grdmask.c grdtrack.c grdreformat.c \
	  grdmath.c grdpaste.c grdproject.c grdsample.c grdtrend.c grdvector.c grdview.c grdvolume.c makecpt.c \
	  mapproject.c minmax.c nearneighbor.c project.c psbasemap.c psclip.c pscoast.c pshistogram.c \
	  psimage.c psmask.c psrose.c psscale.c pstext.c pscontour.c pswiggle.c psxy.c psxyz.c \
	  sample1d.c spectrum1d.c splitxyz.c surface.c trend1d.c trend2d.c triangulate.c xyz2grd.c

SUPPL	=	cps dbase imgsrc meca mex mgg misc segyprogs spotter x2sys x_system xgrid


first:	all

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

LIB_O	= ${LIB_C:.c=.o}

PROGS_O	= ${PROGS_C:.c=.o}

PROGS	= ${PROGS_C:.c=}

$(LIB_O):	%.o:	%.c $(GMT_H) $(GMT_I)
		$(CC) -c $(CFLAGS) $<

$(PROGS_O):	$(GMT_H)

# Jonathan Shewchuck's triangulation model
# Only used if TRIANGLE_D and TRIANGLE_O are set in makegmt.macros

triangle.o:	triangle.c
		$(CC) -c $(CFLAGS) -DNO_TIMER -DTRILIBRARY -DREDUCED -DCDT_ONLY $<

# Assembly wrapper for sincos on Dec Alphas
# Only used if ALPHA_SINCOS_O are set in makegmt.macros

alpha-sincos.o:	alpha-sincos.s
		$(AS) -o alpha-sincos.o $<

#-------------------------------------------------------------------------------
SHARED_LIB	= libpsl.a libgmt.a libpsl.$(SL) libgmt.$(SL)
STATIC_LIB	= libpsl.a libgmt.a

#-------------------------------------------------------------------------------

all:		init libs $(PROGS)

init:		gmt_nan.h gmt_notposix.h gmtmath.c grdmath.c

gmt_nan.h:
		$(CC) $(CFLAGS) gmt_nan_init.c $(CDF) $(LIBS) -o gmt_nan_init
		$(COMPRESS) gmt_nan_init$(EXE)
		./gmt_nan_init

gmt_notposix.h:
		echo "Warning: You probably should rerun configure manually instead"; \
		cd ..; \
		./configure

# Must create g[rd|mt]math.c from g[rd|mt]math_main.c and g[rd|mt]math.func

gmtmath.c:
		(cd ..; sh guru/make_math.sh gmt)

grdmath.c:
		(cd ..; sh guru/make_math.sh grd)

install:	all
		if [ ! -d $(bindir) ]; then \
			mkdir -p $(bindir); \
		fi
		for i in $(PROGS); do \
			$(INSTALL) $$i$(EXE) $(bindir); \
		done
		$(INSTALL) GMT $(bindir)
		if [ ! -d $(libdir) ]; then \
			mkdir -p $(libdir); \
		fi
		if [ ! $(libdir) = $(srcdir) ]; then \
			$(INSTALL) -m 444 libpsl.a $(libdir); \
			$(INSTALL) -m 444 libgmt.a $(libdir); \
			if [ -f libpsl.$(SL) ]; then \
				$(INSTALL) -m 444 libpsl.$(SL) $(libdir); \
				$(INSTALL) -m 444 libgmt.$(SL) $(libdir); \
			fi; \
		fi
		if [ ! -d $(includedir) ]; then \
			mkdir -p $(includedir); \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			for i in $(GMT_H); do \
				$(INSTALL) -m 444 $$i $(includedir); \
			done; \
		fi

uninstall:
		for i in $(PROGS); do \
			\rm -f $(bindir)/$$i$(EXE); \
		done
		\rm -f $(bindir)/GMT
		if [ ! $(libdir) = $(srcdir) ]; then \
			\rm -f $(libdir)/libgmt.a $(libdir)/libpsl.a; \
			if [ -f libpsl.$(SL) ]; then \
				\rm -f $(libdir)/libpsl.$(SL); \
				\rm -f $(libdir)/libgmt.$(SL); \
			fi; \
		fi
		if [ ! $(includedir) = $(srcdir) ]; then \
			for i in $(GMT_H); do \
				\rm -f $(includedir)/$$i; \
			done; \
		fi

install2:	all
		if [ ! -d $(bindir2) ]; then \
			mkdir -p $(bindir2); \
		fi
		for i in $(PROGS); do \
			$(INSTALL) $$i$(EXE) $(bindir2); \
		done
		if [ ! -d $(libdir2) ]; then \
			mkdir -p $(libdir2); \
		fi
		$(INSTALL) -m 444 libpsl.a $(libdir2)
		$(INSTALL) -m 444 libgmt.a $(libdir2)
		if [ ! -d $(includedir2) ]; then \
			mkdir -p $(includedir2); \
		fi
		for i in $(GMT_H); do \
			$(INSTALL) -m 444 $$i $(includedir2); \
		done
		if [ -f libpsl.$(SL) ]; then \
			$(INSTALL) -m 444 libpsl.$(SL) $(libdir2); \
			$(INSTALL) -m 444 libgmt.$(SL) $(libdir2); \
		fi

uninstall2:
		for i in $(PROGS); do \
			\rm -f $(bindir2)/$$i$(EXE); \
		done
		\rm -f $(libdir2)/libgmt.a $(libdir2)/libpsl.a
		for i in $(GMT_H); do \
			\rm -f $(includedir2)/$$i; \
		done
		if [ -f libpsl.$(SL) ]; then \
			\rm -f $(libdir2)/libpsl.$(SL); \
			\rm -f $(libdir2)/libgmt.$(SL); \
		fi

install2-suppl:
		for d in $(SUPPL); do \
			if [ -d $$d ] ; then \
				cd $$d; \
				$(MAKE) bindir=$(bindir2) install; \
				cd ..; \
			fi; \
		done

uninstall2-suppl:
		for d in $(SUPPL); do \
			if [ -d $$d ] ; then \
				cd $$d; \
				$(MAKE) bindir=$(bindir2) uninstall; \
				cd ..; \
			fi; \
		done

examples:
		if [ -d ../examples ]; then \
			cd ../examples; \
			$(CSH) do_examples.$(CSH); \
			cd ../src; \
		else \
			echo "examples directory not installed"; \
		fi

clean:
		rm -f *.o gmt_nan_init$(EXE)
		for i in $(PROGS); do \
			rm -f $$i$(EXE); \
		done

spotless:	clean
		rm -f $(GMTLIB) gmt_nan.h gmt_notposix.h grdmath.c gmtmath.c

distclean:	spotless

#-------------------------------------------------------------------------------
#	libraries
#-------------------------------------------------------------------------------

libs:		$(GMTLIB)

libpsl.a:	pslib.o
		$(AR) cvur libpsl.a $?
		$(RANLIB) libpsl.a

pslib.o:	pslib.c $(PS_H)
		$(CC) $(CFLAGS) -c pslib.c

libgmt.a:	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(AR) cvur $@ $?
		$(RANLIB) $@

libpsl.$(SL):	pslib.o
		$(LD) $(LD_OPT) $? -o $@

libgmt.$(SL):	$(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O)
		$(LD) $(LD_OPT) $(LIB_O) $(TRIANGLE_O) $(ALPHA_SINCOS_O) -o $@

#-------------------------------------------------------------------------------
#	program dependencies
#-------------------------------------------------------------------------------

$(PROGS):	%:	%.o $(GMTLIB)
		$(CC) $(CFLAGS) $< -L. -lgmt -lpsl $(CDF) $(LIBS) $(LDFLAGS) -o $@
		$(COMPRESS) $@$(EXE)

.c.o:
		$(CC) -c $(CFLAGS) $<
