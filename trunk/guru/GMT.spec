#
# This the rpm spec file for the Generic Mapping Tools (GMT)
%define PACKAGE_NAME GMT
%define PACKAGE_VERSION 4.0
%define PACKAGE_URL http://gmt.soest.hawaii.edu/

Summary: Generic Mapping Tools
Name: %PACKAGE_NAME
Version: %PACKAGE_VERSION
Release: 1
Source0: GMT4.0_progs.tar.bz2
Source1: GMT4.0_man.tar.bz2
Source2: GMT4.0_web.tar.bz2
Source3: GMT4.0_suppl.tar.bz2
Copyright: GPL; Copyright (c) 1991-2004, P. Wessel & W. H. F. Smith
Group: Applications/GIS
#URL: %PACKAGE_URL
Packager: Paul Wessel <pwessel@hawaii.edu>
Vendor: SOEST, University of Hawaii <http://www.soest.hawaii.edu>
Distribution: GMT Standard
BuildRoot: /tmp/gmt_root/

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
%setup -D -b 1 -b 2 -b 3 -n GMT4.0
./configure --prefix=/opt/gmt --libdir=/opt/gmt/lib --includedir=/opt/gmt/include --mandir=/opt/gmt/man --enable-shared

%build
make
make suppl

%install
make DESTDIR=$RPM_BUILD_ROOT install install-suppl install-data install-man install-wrapper
# Some stuff that make things easier:
# echo "export PATH=\$PATH:/opt/gmt/bin" > /usr/local/bin/gmt_init
# echo "export GMTHOME=/opt/gmt" >> /usr/local/bin/gmt_init
# chmod +x /usr/local/bin/gmt_init
echo "/opt/geo-data/gmtdata" >$RPM_BUILD_ROOT/opt/gmt/share/coastline.conf

%files
/usr/bin/GMT
/usr/man/manl/GMT.l*
/opt/gmt/bin/*
/opt/gmt/share/*
/opt/gmt/share/.gmtdefaults*
/opt/gmt/man/manl/*.l
/opt/gmt/lib/*
/opt/gmt/include/gmt*
/opt/gmt/include/pslib.h
%doc README
%doc CHANGES
%doc COPYING
%doc www/gmt
