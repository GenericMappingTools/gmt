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
./configure --prefix=/usr/lib/gmt --libdir=/usr/lib/gmt/lib --includedir=/usr/lib/gmt/include --mandir=/usr/lib/gmt/man --enable-shared --disable-flock
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
make install-wrapper
# Some stuff that make things easier:
# echo "export PATH=\$PATH:/usr/lib/gmt/bin" > /usr/local/bin/gmt_init
# echo "export GMTHOME=/opt/gmt" >> /usr/local/bin/gmt_init
# chmod +x /usr/local/bin/gmt_init
echo "/opt/geo-data/gmtdata" > /usr/lib/gmt/share/coastline.conf

%files
/usr/bin/GMT
/usr/man/manl/GMT.l
/usr/lib/gmt/share/coastline.conf
/usr/lib/gmt/bin/backtracker
/usr/lib/gmt/bin/binlegs
/usr/lib/gmt/bin/blockmean
/usr/lib/gmt/bin/blockmedian
/usr/lib/gmt/bin/blockmode
/usr/lib/gmt/bin/cpsdecode
/usr/lib/gmt/bin/cpsencode
/usr/lib/gmt/bin/dat2gmt
/usr/lib/gmt/bin/filter1d
/usr/lib/gmt/bin/fitcircle
/usr/lib/gmt/bin/gmt2bin
/usr/lib/gmt/bin/gmt2dat
/usr/lib/gmt/bin/gmtconvert
/usr/lib/gmt/bin/gmtdefaults
/usr/lib/gmt/bin/gmtinfo
/usr/lib/gmt/bin/gmtlegs
/usr/lib/gmt/bin/gmtlist
/usr/lib/gmt/bin/gmtmath
/usr/lib/gmt/bin/gmtpath
/usr/lib/gmt/bin/gmtselect
/usr/lib/gmt/bin/gmtset
/usr/lib/gmt/bin/gmttrack
/usr/lib/gmt/bin/grd2cpt
/usr/lib/gmt/bin/grd2xyz
/usr/lib/gmt/bin/grdclip
/usr/lib/gmt/bin/grdcontour
/usr/lib/gmt/bin/grdcut
/usr/lib/gmt/bin/grdedit
/usr/lib/gmt/bin/grdfft
/usr/lib/gmt/bin/grdfilter
/usr/lib/gmt/bin/grdgradient
/usr/lib/gmt/bin/grdhisteq
/usr/lib/gmt/bin/grdimage
/usr/lib/gmt/bin/grdinfo
/usr/lib/gmt/bin/grdlandmask
/usr/lib/gmt/bin/grdmask
/usr/lib/gmt/bin/grdmath
/usr/lib/gmt/bin/grdpaste
/usr/lib/gmt/bin/grdproject
/usr/lib/gmt/bin/grdraster
/usr/lib/gmt/bin/grdreformat
/usr/lib/gmt/bin/grdsample
/usr/lib/gmt/bin/grdtrack
/usr/lib/gmt/bin/grdtrend
/usr/lib/gmt/bin/grdvector
/usr/lib/gmt/bin/grdview
/usr/lib/gmt/bin/grdvolume
/usr/lib/gmt/bin/hotspotter
/usr/lib/gmt/bin/img2mercgrd
/usr/lib/gmt/bin/libgmt.so
/usr/lib/gmt/bin/libpsl.so
/usr/lib/gmt/bin/makecpt
/usr/lib/gmt/bin/makepattern
/usr/lib/gmt/bin/mapproject
/usr/lib/gmt/bin/mgd77togmt
/usr/lib/gmt/bin/minmax
/usr/lib/gmt/bin/nearneighbor
/usr/lib/gmt/bin/originator
/usr/lib/gmt/bin/project
/usr/lib/gmt/bin/psbasemap
/usr/lib/gmt/bin/psclip
/usr/lib/gmt/bin/pscoast
/usr/lib/gmt/bin/pscontour
/usr/lib/gmt/bin/pscoupe
/usr/lib/gmt/bin/pshistogram
/usr/lib/gmt/bin/psimage
/usr/lib/gmt/bin/psmask
/usr/lib/gmt/bin/psmeca
/usr/lib/gmt/bin/psmegaplot
/usr/lib/gmt/bin/pspolar
/usr/lib/gmt/bin/psrose
/usr/lib/gmt/bin/psscale
/usr/lib/gmt/bin/pssegy
/usr/lib/gmt/bin/pssegyz
/usr/lib/gmt/bin/pstext
/usr/lib/gmt/bin/psvelo
/usr/lib/gmt/bin/pswiggle
/usr/lib/gmt/bin/psxy
/usr/lib/gmt/bin/psxyz
/usr/lib/gmt/bin/sample1d
/usr/lib/gmt/bin/spectrum1d
/usr/lib/gmt/bin/splitxyz
/usr/lib/gmt/bin/surface
/usr/lib/gmt/bin/trend1d
/usr/lib/gmt/bin/trend2d
/usr/lib/gmt/bin/triangulate
/usr/lib/gmt/bin/x2sys_cross
/usr/lib/gmt/bin/x2sys_datalist
/usr/lib/gmt/bin/xyz2grd
/usr/lib/gmt/bin/xgridedit
/usr/lib/gmt/share/dbase/grdraster.info
/usr/lib/gmt/share/gmt.conf
/usr/lib/gmt/share/gmt.conf.orig
/usr/lib/gmt/share/GMT_cool.cpt
/usr/lib/gmt/share/GMT_copper.cpt
/usr/lib/gmt/share/GMT_CPT.lis
/usr/lib/gmt/share/.gmtdefaults_SI
/usr/lib/gmt/share/.gmtdefaults_US
/usr/lib/gmt/share/gmtformats.d
/usr/lib/gmt/share/GMT_gebco.cpt
/usr/lib/gmt/share/GMT_globe.cpt
/usr/lib/gmt/share/GMT_gray.cpt
/usr/lib/gmt/share/GMT_haxby.cpt
/usr/lib/gmt/share/GMT_hot.cpt
/usr/lib/gmt/share/GMT_jet.cpt
/usr/lib/gmt/share/gmtmedia.d
/usr/lib/gmt/share/GMT_no_green.cpt
/usr/lib/gmt/share/GMT_ocean.cpt
/usr/lib/gmt/share/GMT_polar.cpt
/usr/lib/gmt/share/GMT_rainbow.cpt
/usr/lib/gmt/share/GMT_red2green.cpt
/usr/lib/gmt/share/GMT_relief.cpt
/usr/lib/gmt/share/GMT_sealand.cpt
/usr/lib/gmt/share/GMT_seis.cpt
/usr/lib/gmt/share/GMT_split.cpt
/usr/lib/gmt/share/GMT_topo.cpt
/usr/lib/gmt/share/GMT_wysiwyg.cpt
/usr/lib/gmt/share/mgg/carter.d
/usr/lib/gmt/share/mgg/gmtfile_paths
/usr/lib/gmt/share/PSL_text.ps
/usr/lib/gmt/share/ps_pattern_01.ras
/usr/lib/gmt/share/ps_pattern_02.ras
/usr/lib/gmt/share/ps_pattern_03.ras
/usr/lib/gmt/share/ps_pattern_04.ras
/usr/lib/gmt/share/ps_pattern_05.ras
/usr/lib/gmt/share/ps_pattern_06.ras
/usr/lib/gmt/share/ps_pattern_07.ras
/usr/lib/gmt/share/ps_pattern_08.ras
/usr/lib/gmt/share/ps_pattern_09.ras
/usr/lib/gmt/share/ps_pattern_10.ras
/usr/lib/gmt/share/ps_pattern_11.ras
/usr/lib/gmt/share/ps_pattern_12.ras
/usr/lib/gmt/share/ps_pattern_13.ras
/usr/lib/gmt/share/ps_pattern_14.ras
/usr/lib/gmt/share/ps_pattern_15.ras
/usr/lib/gmt/share/ps_pattern_16.ras
/usr/lib/gmt/share/ps_pattern_17.ras
/usr/lib/gmt/share/ps_pattern_18.ras
/usr/lib/gmt/share/ps_pattern_19.ras
/usr/lib/gmt/share/ps_pattern_20.ras
/usr/lib/gmt/share/ps_pattern_21.ras
/usr/lib/gmt/share/ps_pattern_22.ras
/usr/lib/gmt/share/ps_pattern_23.ras
/usr/lib/gmt/share/ps_pattern_24.ras
/usr/lib/gmt/share/ps_pattern_25.ras
/usr/lib/gmt/share/ps_pattern_26.ras
/usr/lib/gmt/share/ps_pattern_27.ras
/usr/lib/gmt/share/ps_pattern_28.ras
/usr/lib/gmt/share/ps_pattern_29.ras
/usr/lib/gmt/share/ps_pattern_30.ras
/usr/lib/gmt/share/ps_pattern_31.ras
/usr/lib/gmt/share/ps_pattern_32.ras
/usr/lib/gmt/share/ps_pattern_33.ras
/usr/lib/gmt/share/ps_pattern_34.ras
/usr/lib/gmt/share/ps_pattern_35.ras
/usr/lib/gmt/share/ps_pattern_36.ras
/usr/lib/gmt/share/ps_pattern_37.ras
/usr/lib/gmt/share/ps_pattern_38.ras
/usr/lib/gmt/share/ps_pattern_39.ras
/usr/lib/gmt/share/ps_pattern_40.ras
/usr/lib/gmt/share/ps_pattern_41.ras
/usr/lib/gmt/share/ps_pattern_42.ras
/usr/lib/gmt/share/ps_pattern_43.ras
/usr/lib/gmt/share/ps_pattern_44.ras
/usr/lib/gmt/share/ps_pattern_45.ras
/usr/lib/gmt/share/ps_pattern_46.ras
/usr/lib/gmt/share/ps_pattern_47.ras
/usr/lib/gmt/share/ps_pattern_48.ras
/usr/lib/gmt/share/ps_pattern_49.ras
/usr/lib/gmt/share/ps_pattern_50.ras
/usr/lib/gmt/share/ps_pattern_51.ras
/usr/lib/gmt/share/ps_pattern_52.ras
/usr/lib/gmt/share/ps_pattern_53.ras
/usr/lib/gmt/share/ps_pattern_54.ras
/usr/lib/gmt/share/ps_pattern_55.ras
/usr/lib/gmt/share/ps_pattern_56.ras
/usr/lib/gmt/share/ps_pattern_57.ras
/usr/lib/gmt/share/ps_pattern_58.ras
/usr/lib/gmt/share/ps_pattern_59.ras
/usr/lib/gmt/share/ps_pattern_60.ras
/usr/lib/gmt/share/ps_pattern_61.ras
/usr/lib/gmt/share/ps_pattern_62.ras
/usr/lib/gmt/share/ps_pattern_63.ras
/usr/lib/gmt/share/ps_pattern_64.ras
/usr/lib/gmt/share/ps_pattern_65.ras
/usr/lib/gmt/share/ps_pattern_66.ras
/usr/lib/gmt/share/ps_pattern_67.ras
/usr/lib/gmt/share/ps_pattern_68.ras
/usr/lib/gmt/share/ps_pattern_69.ras
/usr/lib/gmt/share/ps_pattern_70.ras
/usr/lib/gmt/share/ps_pattern_71.ras
/usr/lib/gmt/share/ps_pattern_72.ras
/usr/lib/gmt/share/ps_pattern_73.ras
/usr/lib/gmt/share/ps_pattern_74.ras
/usr/lib/gmt/share/ps_pattern_75.ras
/usr/lib/gmt/share/ps_pattern_76.ras
/usr/lib/gmt/share/ps_pattern_77.ras
/usr/lib/gmt/share/ps_pattern_78.ras
/usr/lib/gmt/share/ps_pattern_79.ras
/usr/lib/gmt/share/ps_pattern_80.ras
/usr/lib/gmt/share/ps_pattern_81.ras
/usr/lib/gmt/share/ps_pattern_82.ras
/usr/lib/gmt/share/ps_pattern_83.ras
/usr/lib/gmt/share/ps_pattern_84.ras
/usr/lib/gmt/share/ps_pattern_85.ras
/usr/lib/gmt/share/ps_pattern_86.ras
/usr/lib/gmt/share/ps_pattern_87.ras
/usr/lib/gmt/share/ps_pattern_88.ras
/usr/lib/gmt/share/ps_pattern_89.ras
/usr/lib/gmt/share/ps_pattern_90.ras
/usr/lib/gmt/man/manl/backtracker.l
/usr/lib/gmt/man/manl/binlegs.l
/usr/lib/gmt/man/manl/blockmean.l
/usr/lib/gmt/man/manl/blockmedian.l
/usr/lib/gmt/man/manl/blockmode.l
/usr/lib/gmt/man/manl/cpsencode.l
/usr/lib/gmt/man/manl/cpsdecode.l
/usr/lib/gmt/man/manl/dat2gmt.l
/usr/lib/gmt/man/manl/filter1d.l
/usr/lib/gmt/man/manl/fitcircle.l
/usr/lib/gmt/man/manl/gmt2bin.l
/usr/lib/gmt/man/manl/gmt2dat.l
/usr/lib/gmt/man/manl/gmtconvert.l
/usr/lib/gmt/man/manl/gmtdefaults.l
/usr/lib/gmt/man/manl/gmtinfo.l
/usr/lib/gmt/man/manl/gmt.l
/usr/lib/gmt/man/manl/gmtlegs.l
/usr/lib/gmt/man/manl/gmtlist.l
/usr/lib/gmt/man/manl/gmtmath.l
/usr/lib/gmt/man/manl/gmtpath.l
/usr/lib/gmt/man/manl/gmtselect.l
/usr/lib/gmt/man/manl/gmtset.l
/usr/lib/gmt/man/manl/gmttrack.l
/usr/lib/gmt/man/manl/grd2cpt.l
/usr/lib/gmt/man/manl/grd2xyz.l
/usr/lib/gmt/man/manl/grdclip.l
/usr/lib/gmt/man/manl/grdcontour.l
/usr/lib/gmt/man/manl/grdcut.l
/usr/lib/gmt/man/manl/grdedit.l
/usr/lib/gmt/man/manl/grdfft.l
/usr/lib/gmt/man/manl/grdfilter.l
/usr/lib/gmt/man/manl/grdgradient.l
/usr/lib/gmt/man/manl/grdhisteq.l
/usr/lib/gmt/man/manl/grdimage.l
/usr/lib/gmt/man/manl/grdinfo.l
/usr/lib/gmt/man/manl/grdlandmask.l
/usr/lib/gmt/man/manl/grdmask.l
/usr/lib/gmt/man/manl/grdmath.l
/usr/lib/gmt/man/manl/grdpaste.l
/usr/lib/gmt/man/manl/grdproject.l
/usr/lib/gmt/man/manl/grdraster.l
/usr/lib/gmt/man/manl/grdreformat.l
/usr/lib/gmt/man/manl/grdsample.l
/usr/lib/gmt/man/manl/grdtrack.l
/usr/lib/gmt/man/manl/grdtrend.l
/usr/lib/gmt/man/manl/grdvector.l
/usr/lib/gmt/man/manl/grdview.l
/usr/lib/gmt/man/manl/grdvolume.l
/usr/lib/gmt/man/manl/hotspotter.l
/usr/lib/gmt/man/manl/img2mercgrd.l
/usr/lib/gmt/man/manl/makecpt.l
/usr/lib/gmt/man/manl/makepattern.l
/usr/lib/gmt/man/manl/mapproject.l
/usr/lib/gmt/man/manl/mgd77togmt.l
/usr/lib/gmt/man/manl/minmax.l
/usr/lib/gmt/man/manl/nearneighbor.l
/usr/lib/gmt/man/manl/originator.l
/usr/lib/gmt/man/manl/project.l
/usr/lib/gmt/man/manl/psbasemap.l
/usr/lib/gmt/man/manl/psclip.l
/usr/lib/gmt/man/manl/pscoast.l
/usr/lib/gmt/man/manl/pscontour.l
/usr/lib/gmt/man/manl/pscoupe.l
/usr/lib/gmt/man/manl/pshistogram.l
/usr/lib/gmt/man/manl/psimage.l
/usr/lib/gmt/man/manl/pslib.l
/usr/lib/gmt/man/manl/psmask.l
/usr/lib/gmt/man/manl/psmeca.l
/usr/lib/gmt/man/manl/psmegaplot.l
/usr/lib/gmt/man/manl/pspolar.l
/usr/lib/gmt/man/manl/psrose.l
/usr/lib/gmt/man/manl/psscale.l
/usr/lib/gmt/man/manl/pssegy.l
/usr/lib/gmt/man/manl/pssegyz.l
/usr/lib/gmt/man/manl/pstext.l
/usr/lib/gmt/man/manl/psvelo.l
/usr/lib/gmt/man/manl/pswiggle.l
/usr/lib/gmt/man/manl/psxy.l
/usr/lib/gmt/man/manl/psxyz.l
/usr/lib/gmt/man/manl/sample1d.l
/usr/lib/gmt/man/manl/spectrum1d.l
/usr/lib/gmt/man/manl/splitxyz.l
/usr/lib/gmt/man/manl/surface.l
/usr/lib/gmt/man/manl/trend1d.l
/usr/lib/gmt/man/manl/trend2d.l
/usr/lib/gmt/man/manl/triangulate.l
/usr/lib/gmt/man/manl/x2sys_cross.l
/usr/lib/gmt/man/manl/x2sys_datalist.l
/usr/lib/gmt/man/manl/xyz2grd.l
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
