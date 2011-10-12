# $Id$
# spec file for package GMT (Version 4)
#
# Copyright (c) 2004-2008 Dirk Stoecker <gmt@dstoecker.de>.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments.

# norootforbuild

Name:          GMT
%define coastlineversion 4.1
%define prefix           /opt/gmt
%define sourcepath       ftp://ftp.soest.hawaii.edu/gmt/4/
%define incdir           %{prefix}/include
License:       GPL
Group:         Productivity/Graphics/Visualization/Graph
Provides:      GMT 
Autoreqprov:   on
Version:       4.1.4
Release:       1
%if 0%{?suse_version}
BuildRequires: autoconf automake gcc netcdf
%endif
%if 0%{?fedora_version}
BuildRequires: autoconf automake gcc netcdf-devel
%endif
%if 0%{?mandriva_version}
BuildRequires: autoconf automake gcc netcdf-devel
%endif
Summary:       Generic Mapping Tools
Summary(de):   Generic Mapping Tools - Karten- und Grafikerzeugung
Source0:       %{sourcepath}GMT%{version}_man.tar.bz2
Source1:       %{sourcepath}GMT%{version}_scripts.tar.bz2
Source2:       %{sourcepath}GMT%{version}_share.tar.bz2
Source3:       %{sourcepath}GMT%{version}_src.tar.bz2
Source4:       %{sourcepath}GMT%{version}_suppl.tar.bz2
Source5:       %{sourcepath}GMT%{version}_web.tar.bz2
Source6:       %{sourcepath}GMT%{coastlineversion}_coast.tar.bz2
Source7:       %{sourcepath}GMT%{coastlineversion}_full.tar.bz2
Source8:       %{sourcepath}GMT%{coastlineversion}_high.tar.bz2
#Source9:      %{sourcepath}GMT%{version}_tut.tar.bz2
#Source10:     %{sourcepath}GMT%{version}_pdf.tar.bz2
BuildRoot:     %{_tmppath}/%{name}-%{version}-build/

%description
GMT is a free, public-domain collection of about 60 UNIX tools
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
    Walter H. F. Smith <Walter.HF.Smith@noaa.gov>

%description -l de
GMT ist eine frei verfuegbare Sammlung von etwa 60 UNIX programmen,
welche es erlauben 2- und 3-dimensionale Datensaetze zu manipulieren
(inklusive Filterung, Trendanpassung, Erzeugung von Rasterfeldern,
Projektion, ...) und [Encapsulated] PostScript-Dateien (EPS) zu
erzeugen. Die erzeugbaren Ausgaben gehen von einfachen X-Y-Grafiken
ueber Konturkarten bis zu kuenstlich beleuchteten Oberflaechen und
3D-Perspektiven in Schwarz-Weiß, Graustufen, Schraffuren und
24Bit-Farben. GMT unterstuetzt 25 bekannte Kartenprojektionen und
zusaetzlich lineare, logarithmische und exponentielle Skalierungen.
Datensaetze fuer Kuestenlinien, Fluesse und politsche Grenzen werden
unterstuetzt.



Autoren:
--------
    Paul Wessel <pwessel@hawaii.edu>
    Walter H. F. Smith <Walter.HF.Smith@noaa.gov>

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
tutorial in HTML format. The documentation is additionally available in PDF
(see ftp://ftp.soest.hawaii.edu/gmt/4/ or http://gmt.soest.hawaii.edu/).

%description doc -l de
Die englische HTML-Dokumentation der Generic Mapping Tools inklusive Tutorial.
Die Dateien sind auch im PDF-Format erhaeltlich (siehe dazu
ftp://ftp.soest.hawaii.edu/gmt/4/ oder http://gmt.soest.hawaii.edu/).

%package examples
Summary:      Generic Mapping Tools (Documentation).
Summary(de):  Generic Mapping Tools (Dokumentation).
Group:        Productivity/Graphics/Visualization/Graph
Requires:     %{name}

%description examples
Example scripts for the Generic Mapping Tools.

%description examples -l de
Beispiel-Skripte für die Generic Mapping Tools.

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
%setup -q -b1 -b2 -b3 -b4 -b5 -b6 -b7 -b8 -n %{name}%{version}
%define configline1 ./configure --prefix=%{prefix} --libdir=%{prefix}/%_lib
%define configline2 --includedir=%{incdir} --mandir=%{prefix}/man --enable-shared

%if 0%{?fedora_version}
CFLAGS="-L/usr/lib/netcdf-3 -I/usr/include/netcdf-3 -O3 -s" %{configline1} %{configline2}
%else
%ifarch x86_64
CFLAGS="-fPIC -O3 -s" %{configline1} %{configline2}
%else
CFLAGS="-O3 -s" %{configline1} %{configline2}
%endif
%endif
make
make suppl

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install install-all
#make install-wrapper
cp -pr ../share/coast $RPM_BUILD_ROOT/%{prefix}/share
cp -pr examples $RPM_BUILD_ROOT/%{prefix}
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
%if 0%{?mandriva_version}
%else
/usr/local/man/manl/*
%endif
%{prefix}/bin/*
%{prefix}/share/cpt/
%{prefix}/share/conf/
%{prefix}/share/custom/
%{prefix}/share/dbase/
%{prefix}/share/mgd77/
%{prefix}/share/mgg/
%{prefix}/share/pattern/
%{prefix}/share/pslib/
%{prefix}/share/time/
%{prefix}/share/x2sys/
%{prefix}/share/gmt*
%{prefix}/share/GMT*
%{prefix}/share/.gmtdefaults*
%{prefix}/man/manl/*.l*
%{prefix}/%_lib/*.so
%doc README
%doc CHANGES
%doc LICENSE.TXT

%files devel
%defattr(-,root,root)
%{incdir}/gmt*
%{incdir}/mgd77*
%{incdir}/pslib*
%{prefix}/%_lib/*.a

%files doc
%defattr(-,root,root)
%{prefix}/www/gmt

%files examples
%defattr(-,root,root)
%{prefix}/examples

%files coastlines
%defattr(-,root,root)
%{prefix}/share/coast/*_*.cdf

%changelog -n GMT
* Mon Jan 29 2007 - gmt@dstoecker.de
- modified for final GMT 4.1.4 release
- modified to support more distributions

* Wed Jun 28 2006 - gmt@dstoecker.de
- modified for final GMT 4.1.3 release

* Wed Mar 1 2006 - gmt@dstoecker.de
- modified for final GMT 4.1.1 release

* Mon Jan 3 2006 - gmt@dstoecker.de
- modified for final GMT 4.1 release

* Fri Dec 23 2005 - gmt@dstoecker.de
- added examples package
- some minor modifications

* Mon Nov 29 2004 - gmt@dstoecker.de
- first version
