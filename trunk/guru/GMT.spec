#
# spec file for package GMT (Version 4)
#
# Copyright (c) 2004 Dirk Stoecker <soft@dstoecker.de>.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments.
#

# norootforbuild
# neededforbuild autoconf automake gcc netcdf

BuildRequires: autoconf,automake,gcc,netcdf

Name:         GMT
%define prefix /opt/gmt
%define sourcepath ftp://gmt.soest.hawaii.edu/pub/gmt/4/
%define incdir %{prefix}/include
License:      GPL
Group:        Productivity/Graphics/Visualization/Graph
Provides:     GMT 
Autoreqprov:  on
Requires:     netcdf >= 3.4
Version:      4.0
Release:      18
Summary:      Generic Mapping Tools
Summary(de):  Generic Mapping Tools - Karten- und Grafikerzeugung
Source0:      %{sourcepath}GMT4.0_progs.tar.bz2
Source1:      %{sourcepath}GMT4.0_man.tar.bz2
Source2:      %{sourcepath}GMT4.0_web.tar.bz2
Source3:      %{sourcepath}GMT4.0_suppl.tar.bz2
Source4:      %{sourcepath}GMT_full.tar.bz2
Source5:      %{sourcepath}GMT_share.tar.bz2
Source6:      %{sourcepath}GMT_high.tar.bz2
#Source7:      %{sourcepath}GMT4.0_tut.tar.bz2
#Source8:      %{sourcepath}GMT4.0_scripts.tar.bz2
#Source9:      %{sourcepath}GMT4.0_pdf.tar.bz2
#Source10:     %{sourcepath}GMT4.0_ps.tar.bz2
#Source11:     %{sourcepath}triangle.tar.bz2
#Patch:        GMT_rpm.patch
BuildRoot:    %{_tmppath}/%{name}-%{version}-build/

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



Authors:
--------
    Paul Wessel <pwessel@hawaii.edu>
    Walter H. F. Smith <walter@raptor.grdl.noaa.gov>

%description -l de
GMT ist eine frei verfuegbare Sammlung von etwa 60 UNIX programmen,
welche es erlauben 2- und 3-dimensionale Datensaetze zu manipulieren
(inklusive Filterung, Trendanpassung, Erzeugung von Rasterfeldern,
Projektion, ...) und [Encapsulated] PostScript-Dateien (EPS) zu
erzeugen. Die erzeugbaren Ausgaben gehen von einfachen X-Y-Grafiken
ueber Konturkarten bis zu kuenstlich beleuchteten Oberflaechen und
3D-Perspektiven in Schwarz-Weiﬂ, Graustufen, Schraffuren und
24Bit-Farben. GMT unterstuetzt 25 bekannte Kartenprojektionen und
zusaetzlich lineare, logarithmische und exponentielle Skalierungen.
Datensaetze fuer Kuestenlinien, Fluesse und politsche Grenzen werden
unterstuetzt.



Autoren:
--------
    Paul Wessel <pwessel@hawaii.edu>
    Walter H. F. Smith <walter@raptor.grdl.noaa.gov>

%package devel
Summary:      Generic Mapping Tools (Include Files and Libraries mandatory for Development).
Summary(de):  Generic Mapping Tools (Include-Dateien fuer Entwickler).
Group:        Productivity/Graphics/Visualization/Graph
Autoreqprov:  on
Requires:     %{name} = %{version}

%description devel
The include files needed for GMT development.

%description devel -l de
Die Include-Dateien fuer GMT-Entwickler.

%package doc
Summary:      Generic Mapping Tools (Documentation).
Summary(de):  Generic Mapping Tools (Dokumentation).
Group:        Productivity/Graphics/Visualization/Graph
Requires:     %{name}

%description doc
Documentation of the Generic Mapping Tools including documentation and
tutorial in HTML format. The dokumentation is additionally available in pdf
and ps as well.
See ftp://gmt.soest.hawaii.edu/pub/gmt/4/ or http://gmt.soest.hawaii.edu/.

%description doc -l de
Die englische HTML-Dokumentation der Generic Mapping Tools inklusive Tutorial.
Die Dateien sind auch im PDF- und PostScript-Format erhaeltlich (siehe dazu
ftp://gmt.soest.hawaii.edu/pub/gmt/4/ oder http://gmt.soest.hawaii.edu/).

%package coastlines
Summary:      Generic Mapping Tools (coastlines, rivers, politcal boundaries).
Summary(de):  Generic Mapping Tools (Kuestenlinien, Fluesse, politsche Grenzen).
Group:        Productivity/Graphics/Visualization/Graph
Autoreqprov:  on
Requires:     %{name}

%description coastlines
GMT uses a 5-resolution database for coastlines, rivers, and
political borders.  The crude, low, intermediate, full and high resolutions
are supplied with this package.

%description coastlines -l de
GMT nutzt eine Datenbank fuer Kuestenlinien, Fluesse und politische Grenzen,
welche in 5 Aufloeungen vorliegt. Die Aufloesungen "crude", "low",
"intermediate", "full" and "high" sind in diesem Paket enthalten.

%prep
%setup -q -b1 -b2 -b3 -b4 -b5 -b6 -n %{name}%{version}
#%patch -p1
CFLAGS="-O3 -s" ./configure --prefix=%{prefix} \
	    --libdir=%{prefix}/%_lib \
            --includedir=%{incdir} \
            --mandir=%{prefix}/man \
            --enable-shared
make
make suppl

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install install-all
#make install-wrapper
cp ../share/*.cdf $RPM_BUILD_ROOT/%{prefix}/share
#cp -r www/gmt $RPM_BUILD_ROOT/%{prefix}/share/doc
gzip -9 $RPM_BUILD_ROOT/%{prefix}/man/manl/*
mkdir -p $RPM_BUILD_ROOT/usr/local/man/manl
mkdir -p $RPM_BUILD_ROOT/usr/bin
# make symbolic links to allow usage in low level environments (like webserver)
find $RPM_BUILD_ROOT/%{prefix}/man/manl/ -type f -printf "ln -s %{prefix}/man/manl/%f $RPM_BUILD_ROOT/usr/local/man/manl/%f\n" |sh
find $RPM_BUILD_ROOT/%{prefix}/bin/ -type f -printf "ln -s %{prefix}/bin/%f $RPM_BUILD_ROOT/usr/bin/%f\n" |sh

%clean
rm -rf $RPM_BUILD_ROOT

%post
%run_ldconfig

%postun
%run_ldconfig

%files
%defattr(-,root,root)
/usr/bin/*
/usr/local/man/manl/*
%{prefix}/bin/*
%{prefix}/share/cpt/
%{prefix}/share/custom/
%{prefix}/share/dbase/
%{prefix}/share/mgd77/
%{prefix}/share/mgg/
%{prefix}/share/pattern/
%{prefix}/share/pslib/
%{prefix}/share/time/
#%{prefix}/share/x2sys/
%{prefix}/share/gmt*
%{prefix}/share/GMT*
%{prefix}/share/.gmtdefaults*
%{prefix}/man/manl/*.l*
%{prefix}/lib/*.so
%doc README
%doc CHANGES
%doc COPYING

%files devel
%defattr(-,root,root)
%{incdir}/gmt*
#%{incdir}/mgd77*
%{incdir}/pslib*
%{prefix}/lib/*.a

%files doc
%defattr(-,root,root)
%{prefix}/www/gmt

%files coastlines
%defattr(-,root,root)
%{prefix}/share/*_*.cdf

%changelog -n GMT
* Mon Nov 29 2004 - soft@dstoecker.de
- first SuSE version
