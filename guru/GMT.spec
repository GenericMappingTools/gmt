#
# This the rpm spec file for the Generic Mapping Tools (GMT)
%define PACKAGE_NAME GMT
%define PACKAGE_VERSION 3.4
%define PACKAGE_URL http://gmt.soest.hawaii.edu/

Summary: Generic Mapping Tools
Name: %PACKAGE_NAME
Version: %PACKAGE_VERSION
Release: 1
Source0: GMT3.4_progs.tar.bz2
Source1: GMT3.4_doc.tar.bz2
Source2: GMT3.4_web.tar.bz2
Source3: GMT3.4_suppl.tar.bz2
#Patch:
Copyright: GPL; Copyright (c) 1991-2001, P. Wessel & W. H. F. Smith
Group: Applications/GIS
#URL: %PACKAGE_URL
Packager: Paul Wessel <pwessel@hawaii.edu>
Vendor: SOEST, University of Hawaii <http://www.soest.hawaii.edu>
Distribution: Geoware CD

#Icon:
#Conflicts:
Requires: netCDF >= 3.4

%description
GMT is a free, public-domain collection of ~60 UNIX tools
that allow users to manipulate (x,y) and (x,y,z) data sets
(including filtering, trend fitting, gridding, projecting,
etc.) and produce [Encapsulated] PostScript File (EPS)
illustrations ranging from simple x-y plots through contour
maps to artificially illuminated surfaces and 3-D perspective
views in black and white, gray tone, hachure patterns, and
24-bit color. GMT supports 25 common map projections plus
linear, log, and power scaling, and comes with support data
such as coastlines, rivers, and political boundaries. 

%prep
#set
%setup -D -b 1 -b 2 -b 3 -n GMT3.4
#%patch -p1
./configure --prefix=/opt/gmt --libdir=/usr/local/lib --includedir=/usr/local/include --mandir=/usr/local/man --enable-shared --disable-flock
# Leave xgrid off until kosher again:
#( cd src/xgrid ; mv makefile makefile.dontconsider ; echo "all:" > makefile ;
#echo "install:" >> makefile )

%build
make
make suppl
( cd www/gmt/doc/ps ; bzip2 -d *.bz2 )

%install
make install
make install-suppl
make install-data
make install-man
make install-www
# Some stuff that make things easier:
echo "export PATH=\$PATH:/opt/gmt/bin" > /usr/local/bin/gmt_init
echo "export GMTHOME=/opt/gmt" >> /usr/local/bin/gmt_init
chmod +x /usr/local/bin/gmt_init
echo "/opt/geo-data/gmtdata" > /opt/gmt/share/coastline.conf

%files
/opt/gmt/share/coastline.conf
/opt/gmt/bin/backtracker
/opt/gmt/bin/binlegs
/opt/gmt/bin/blockmean
/opt/gmt/bin/blockmedian
/opt/gmt/bin/blockmode
/opt/gmt/bin/cpsdecode
/opt/gmt/bin/cpsencode
/opt/gmt/bin/dat2gmt
/opt/gmt/bin/filter1d
/opt/gmt/bin/fitcircle
/opt/gmt/bin/gmt2bin
/opt/gmt/bin/gmt2dat
/opt/gmt/bin/gmtconvert
/opt/gmt/bin/gmtdefaults
/opt/gmt/bin/gmtinfo
/opt/gmt/bin/gmtlegs
/opt/gmt/bin/gmtlist
/opt/gmt/bin/gmtmath
/opt/gmt/bin/gmtpath
/opt/gmt/bin/gmtselect
/opt/gmt/bin/gmtset
/opt/gmt/bin/gmttrack
/opt/gmt/bin/grd2cpt
/opt/gmt/bin/grd2xyz
/opt/gmt/bin/grdclip
/opt/gmt/bin/grdcontour
/opt/gmt/bin/grdcut
/opt/gmt/bin/grdedit
/opt/gmt/bin/grdfft
/opt/gmt/bin/grdfilter
/opt/gmt/bin/grdgradient
/opt/gmt/bin/grdhisteq
/opt/gmt/bin/grdimage
/opt/gmt/bin/grdinfo
/opt/gmt/bin/grdlandmask
/opt/gmt/bin/grdmask
/opt/gmt/bin/grdmath
/opt/gmt/bin/grdpaste
/opt/gmt/bin/grdproject
/opt/gmt/bin/grdraster
/opt/gmt/bin/grdreformat
/opt/gmt/bin/grdsample
/opt/gmt/bin/grdtrack
/opt/gmt/bin/grdtrend
/opt/gmt/bin/grdvector
/opt/gmt/bin/grdview
/opt/gmt/bin/grdvolume
/opt/gmt/bin/hotspotter
/opt/gmt/bin/img2mercgrd
/opt/gmt/bin/libgmt.so
/opt/gmt/bin/libpsl.so
/opt/gmt/bin/makecpt
/opt/gmt/bin/makepattern
/opt/gmt/bin/mapproject
/opt/gmt/bin/mgd77togmt
/opt/gmt/bin/minmax
/opt/gmt/bin/nearneighbor
/opt/gmt/bin/originator
/opt/gmt/bin/project
/opt/gmt/bin/psbasemap
/opt/gmt/bin/psclip
/opt/gmt/bin/pscoast
/opt/gmt/bin/pscontour
/opt/gmt/bin/pscoupe
/opt/gmt/bin/pshistogram
/opt/gmt/bin/psimage
/opt/gmt/bin/psmask
/opt/gmt/bin/psmeca
/opt/gmt/bin/psmegaplot
/opt/gmt/bin/pspolar
/opt/gmt/bin/psrose
/opt/gmt/bin/psscale
/opt/gmt/bin/pssegy
/opt/gmt/bin/pssegyz
/opt/gmt/bin/pstext
/opt/gmt/bin/psvelo
/opt/gmt/bin/pswiggle
/opt/gmt/bin/psxy
/opt/gmt/bin/psxyz
/opt/gmt/bin/sample1d
/opt/gmt/bin/spectrum1d
/opt/gmt/bin/splitxyz
/opt/gmt/bin/surface
/opt/gmt/bin/trend1d
/opt/gmt/bin/trend2d
/opt/gmt/bin/triangulate
/opt/gmt/bin/x2sys_cross
/opt/gmt/bin/x2sys_datalist
/opt/gmt/bin/xyz2grd
/opt/gmt/bin/xgridedit
/opt/gmt/share/dbase/grdraster.info
/opt/gmt/share/gmt.conf
/opt/gmt/share/gmt.conf.orig
/opt/gmt/share/GMT_cool.cpt
/opt/gmt/share/GMT_copper.cpt
/opt/gmt/share/GMT_CPT.lis
/opt/gmt/share/.gmtdefaults_SI
/opt/gmt/share/.gmtdefaults_US
/opt/gmt/share/gmtformats.d
/opt/gmt/share/GMT_gebco.cpt
/opt/gmt/share/GMT_globe.cpt
/opt/gmt/share/GMT_gray.cpt
/opt/gmt/share/GMT_haxby.cpt
/opt/gmt/share/GMT_hot.cpt
/opt/gmt/share/GMT_jet.cpt
/opt/gmt/share/gmtmedia.d
/opt/gmt/share/GMT_no_green.cpt
/opt/gmt/share/GMT_ocean.cpt
/opt/gmt/share/GMT_polar.cpt
/opt/gmt/share/GMT_rainbow.cpt
/opt/gmt/share/GMT_red2green.cpt
/opt/gmt/share/GMT_relief.cpt
/opt/gmt/share/GMT_sealand.cpt
/opt/gmt/share/GMT_seis.cpt
/opt/gmt/share/GMT_split.cpt
/opt/gmt/share/GMT_topo.cpt
/opt/gmt/share/GMT_wysiwyg.cpt
/opt/gmt/share/mgg/carter.d
/opt/gmt/share/mgg/gmtfile_paths
/opt/gmt/share/PSL_text.ps
/opt/gmt/share/ps_pattern_01.ras
/opt/gmt/share/ps_pattern_02.ras
/opt/gmt/share/ps_pattern_03.ras
/opt/gmt/share/ps_pattern_04.ras
/opt/gmt/share/ps_pattern_05.ras
/opt/gmt/share/ps_pattern_06.ras
/opt/gmt/share/ps_pattern_07.ras
/opt/gmt/share/ps_pattern_08.ras
/opt/gmt/share/ps_pattern_09.ras
/opt/gmt/share/ps_pattern_10.ras
/opt/gmt/share/ps_pattern_11.ras
/opt/gmt/share/ps_pattern_12.ras
/opt/gmt/share/ps_pattern_13.ras
/opt/gmt/share/ps_pattern_14.ras
/opt/gmt/share/ps_pattern_15.ras
/opt/gmt/share/ps_pattern_16.ras
/opt/gmt/share/ps_pattern_17.ras
/opt/gmt/share/ps_pattern_18.ras
/opt/gmt/share/ps_pattern_19.ras
/opt/gmt/share/ps_pattern_20.ras
/opt/gmt/share/ps_pattern_21.ras
/opt/gmt/share/ps_pattern_22.ras
/opt/gmt/share/ps_pattern_23.ras
/opt/gmt/share/ps_pattern_24.ras
/opt/gmt/share/ps_pattern_25.ras
/opt/gmt/share/ps_pattern_26.ras
/opt/gmt/share/ps_pattern_27.ras
/opt/gmt/share/ps_pattern_28.ras
/opt/gmt/share/ps_pattern_29.ras
/opt/gmt/share/ps_pattern_30.ras
/opt/gmt/share/ps_pattern_31.ras
/opt/gmt/share/ps_pattern_32.ras
/opt/gmt/share/ps_pattern_33.ras
/opt/gmt/share/ps_pattern_34.ras
/opt/gmt/share/ps_pattern_35.ras
/opt/gmt/share/ps_pattern_36.ras
/opt/gmt/share/ps_pattern_37.ras
/opt/gmt/share/ps_pattern_38.ras
/opt/gmt/share/ps_pattern_39.ras
/opt/gmt/share/ps_pattern_40.ras
/opt/gmt/share/ps_pattern_41.ras
/opt/gmt/share/ps_pattern_42.ras
/opt/gmt/share/ps_pattern_43.ras
/opt/gmt/share/ps_pattern_44.ras
/opt/gmt/share/ps_pattern_45.ras
/opt/gmt/share/ps_pattern_46.ras
/opt/gmt/share/ps_pattern_47.ras
/opt/gmt/share/ps_pattern_48.ras
/opt/gmt/share/ps_pattern_49.ras
/opt/gmt/share/ps_pattern_50.ras
/opt/gmt/share/ps_pattern_51.ras
/opt/gmt/share/ps_pattern_52.ras
/opt/gmt/share/ps_pattern_53.ras
/opt/gmt/share/ps_pattern_54.ras
/opt/gmt/share/ps_pattern_55.ras
/opt/gmt/share/ps_pattern_56.ras
/opt/gmt/share/ps_pattern_57.ras
/opt/gmt/share/ps_pattern_58.ras
/opt/gmt/share/ps_pattern_59.ras
/opt/gmt/share/ps_pattern_60.ras
/opt/gmt/share/ps_pattern_61.ras
/opt/gmt/share/ps_pattern_62.ras
/opt/gmt/share/ps_pattern_63.ras
/opt/gmt/share/ps_pattern_64.ras
/opt/gmt/share/ps_pattern_65.ras
/opt/gmt/share/ps_pattern_66.ras
/opt/gmt/share/ps_pattern_67.ras
/opt/gmt/share/ps_pattern_68.ras
/opt/gmt/share/ps_pattern_69.ras
/opt/gmt/share/ps_pattern_70.ras
/opt/gmt/share/ps_pattern_71.ras
/opt/gmt/share/ps_pattern_72.ras
/opt/gmt/share/ps_pattern_73.ras
/opt/gmt/share/ps_pattern_74.ras
/opt/gmt/share/ps_pattern_75.ras
/opt/gmt/share/ps_pattern_76.ras
/opt/gmt/share/ps_pattern_77.ras
/opt/gmt/share/ps_pattern_78.ras
/opt/gmt/share/ps_pattern_79.ras
/opt/gmt/share/ps_pattern_80.ras
/opt/gmt/share/ps_pattern_81.ras
/opt/gmt/share/ps_pattern_82.ras
/opt/gmt/share/ps_pattern_83.ras
/opt/gmt/share/ps_pattern_84.ras
/opt/gmt/share/ps_pattern_85.ras
/opt/gmt/share/ps_pattern_86.ras
/opt/gmt/share/ps_pattern_87.ras
/opt/gmt/share/ps_pattern_88.ras
/opt/gmt/share/ps_pattern_89.ras
/opt/gmt/share/ps_pattern_90.ras
/usr/local/man/manl/backtracker.l
/usr/local/man/manl/binlegs.l
/usr/local/man/manl/blockmean.l
/usr/local/man/manl/blockmedian.l
/usr/local/man/manl/blockmode.l
/usr/local/man/manl/cpsencode.l
/usr/local/man/manl/cpsdecode.l
/usr/local/man/manl/dat2gmt.l
/usr/local/man/manl/filter1d.l
/usr/local/man/manl/fitcircle.l
/usr/local/man/manl/gmt2bin.l
/usr/local/man/manl/gmt2dat.l
/usr/local/man/manl/gmtconvert.l
/usr/local/man/manl/gmtdefaults.l
/usr/local/man/manl/gmtinfo.l
/usr/local/man/manl/gmt.l
/usr/local/man/manl/gmtlegs.l
/usr/local/man/manl/gmtlist.l
/usr/local/man/manl/gmtmath.l
/usr/local/man/manl/gmtpath.l
/usr/local/man/manl/gmtselect.l
/usr/local/man/manl/gmtset.l
/usr/local/man/manl/gmttrack.l
/usr/local/man/manl/grd2cpt.l
/usr/local/man/manl/grd2xyz.l
/usr/local/man/manl/grdclip.l
/usr/local/man/manl/grdcontour.l
/usr/local/man/manl/grdcut.l
/usr/local/man/manl/grdedit.l
/usr/local/man/manl/grdfft.l
/usr/local/man/manl/grdfilter.l
/usr/local/man/manl/grdgradient.l
/usr/local/man/manl/grdhisteq.l
/usr/local/man/manl/grdimage.l
/usr/local/man/manl/grdinfo.l
/usr/local/man/manl/grdlandmask.l
/usr/local/man/manl/grdmask.l
/usr/local/man/manl/grdmath.l
/usr/local/man/manl/grdpaste.l
/usr/local/man/manl/grdproject.l
/usr/local/man/manl/grdraster.l
/usr/local/man/manl/grdreformat.l
/usr/local/man/manl/grdsample.l
/usr/local/man/manl/grdtrack.l
/usr/local/man/manl/grdtrend.l
/usr/local/man/manl/grdvector.l
/usr/local/man/manl/grdview.l
/usr/local/man/manl/grdvolume.l
/usr/local/man/manl/hotspotter.l
/usr/local/man/manl/img2mercgrd.l
/usr/local/man/manl/makecpt.l
/usr/local/man/manl/makepattern.l
/usr/local/man/manl/mapproject.l
/usr/local/man/manl/mgd77togmt.l
/usr/local/man/manl/minmax.l
/usr/local/man/manl/nearneighbor.l
/usr/local/man/manl/originator.l
/usr/local/man/manl/project.l
/usr/local/man/manl/psbasemap.l
/usr/local/man/manl/psclip.l
/usr/local/man/manl/pscoast.l
/usr/local/man/manl/pscontour.l
/usr/local/man/manl/pscoupe.l
/usr/local/man/manl/pshistogram.l
/usr/local/man/manl/psimage.l
/usr/local/man/manl/pslib.l
/usr/local/man/manl/psmask.l
/usr/local/man/manl/psmeca.l
/usr/local/man/manl/psmegaplot.l
/usr/local/man/manl/pspolar.l
/usr/local/man/manl/psrose.l
/usr/local/man/manl/psscale.l
/usr/local/man/manl/pssegy.l
/usr/local/man/manl/pssegyz.l
/usr/local/man/manl/pstext.l
/usr/local/man/manl/psvelo.l
/usr/local/man/manl/pswiggle.l
/usr/local/man/manl/psxy.l
/usr/local/man/manl/psxyz.l
/usr/local/man/manl/sample1d.l
/usr/local/man/manl/spectrum1d.l
/usr/local/man/manl/splitxyz.l
/usr/local/man/manl/surface.l
/usr/local/man/manl/trend1d.l
/usr/local/man/manl/trend2d.l
/usr/local/man/manl/triangulate.l
/usr/local/man/manl/x2sys_cross.l
/usr/local/man/manl/x2sys_datalist.l
/usr/local/man/manl/xyz2grd.l
/usr/local/bin/gmt_init
/usr/local/lib/libgmt.a
/usr/local/lib/libpsl.a
/usr/local/lib/libgmt.so
/usr/local/lib/libpsl.so
/usr/local/include/gmt.h
/usr/local/include/gmt_bcr.h
/usr/local/include/gmt_boundcond.h
/usr/local/include/gmt_colors.h
/usr/local/include/gmt_customio.h
/usr/local/include/gmt_funcnames.h
/usr/local/include/gmt_grd.h
/usr/local/include/gmt_grdio.h
/usr/local/include/gmt_io.h
/usr/local/include/gmt_keywords.h
/usr/local/include/gmt_map.h
/usr/local/include/gmt_math.h
/usr/local/include/gmt_nan.h
/usr/local/include/gmt_notposix.h
/usr/local/include/gmt_notunix.h
/usr/local/include/gmt_project.h
/usr/local/include/gmt_shore.h
/usr/local/include/gmt_unique.h
/usr/local/include/pslib.h

%doc README
%doc CHANGES
%doc COPYING
%doc www/gmt
