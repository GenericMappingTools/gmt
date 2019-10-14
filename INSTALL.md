# Installing GMT

[![GitHub release](https://img.shields.io/github/release/GenericMappingTools/gmt)](https://github.com/GenericMappingTools/gmt/releases)

GMT is available on Windows, macOS and Linux.
Source and binary packages are provided for the latest release,
and can be downloaded from the [GMT main site](https://www.generic-mapping-tools.org)
and [the GitHub repository](https://github.com/GenericMappingTools/gmt/releases).

This file provides instructions for installing GMT binary packages on
different operating systems. Please refer to [Building Instructions](BUILDING.md)
for compiling GMT source package (either stable release or development version).

## Contents

- [Windows](#windows)
- [macOS](#macos)
  * [Application Bundle](#application-bundle)
  * [Install via Homebrew](#install-via-homebrew)
  * [Install via Macports](#install-via-macports)
  * [Install via fink](#install-via-fink)
- [Linux](#linux)
  * [Fedora](#fedora)
  * [RHEL/CentOS](#rhelcentos)
  * [Ubuntu/Debian](#ubuntudebian)
- [Cross Platform Install Instructions](#cross-platform-install-instructions)
  * [Install via conda](#install-via-conda)

## Windows

We provide 32 and 64 bit standalone installers (e.g., gmt-6.x.x-win64.exe)
on the [GMT main site](https://www.generic-mapping-tools.org).
The installers come with GDAL, FFmpeg, and Ghostscript pre-installed.

In addition to the GMT installer, you also need to download and install
[GraphicsMagick](http://www.graphicsmagick.org/) if you want to making movies in
GIF format.

## macOS

### Application Bundle

Application bundle is available from the [GMT main site](https://www.generic-mapping-tools.org).
Download the application bundle (gmt-6.x.x-darwin-x86_64.dmg), double-click to mount it
and drag GMT-6.x.x.app to the "Applications" folder (or any other folder).

GMT-6.x.x.app opens a terminal from which you can invoke GMT programs and scripts.
If you like, you can add the GMT programs contained in the application bundle to
your search path for executables. For that, just run GMT-6.x.x.app once and type:

    echo ${BUNDLE_RESOURCES}/bin

in the terminal. Then prepend this directory (e.g. `/Applications/GMT-6.x.x.app/Contents/Resources/bin`)
to your PATH environment variable, e.g., in `~/.bashrc`.
Note: The installer is always built for the latest macOS version only.

While the installer comes with all GMT executables and needed libraries, there are some
run-time dependencies on executables that you will need to install separately:

- [GDAL](https://www.gdal.org/) (Convert shapefiles and produce geotiffs)
- [Ghostscript](https://www.ghostscript.com/) (Convert PostScript to PDF or raster images)
- [GraphicsMagick](http://www.graphicsmagick.org/) (Convert images to animated GIFs)
- [FFmpeg](http://www.ffmpeg.org/) (Convert images to MP4 or WebM videos)

Install these via Fink, MacPorts, or Homebrew. If you do not use any of these, then
we recommend installing [Homebrew](https://brew.sh/). Now, run either the fink, port,
or brew command:

    # For Fink users
    fink install gdal ghostscript graphicsmagick ffmpeg

    # For MacPorts users
    sudo port install gdal +hdf5 +netcdf +openjpeg
    sudo port install ghostscript graphicsmagick ffmpeg

    # For Homebrew users
    brew install gdal ghostscript graphicsmagick ffmpeg

### Install via Homebrew

Installation of GMT through [Homebrew](https://brew.sh/) is extremely simple.
Installing Homebrew itself is a one line command only (see [the Homebrew page](https://brew.sh/)).
You may need to update the formulas so for that you will do:

    brew update && brew upgrade

For the latest GMT 5 version use:

    brew install gmt

If you want to install GMT 4 and GMT 5 alongside, do:

    brew unlink gmt && brew install gmt4

and to go from GMT 5 to GMT 4 (and vice-versa for 4 to 5, but see also the doc about gmtswitch):

    brew unlink gmt && brew link gmt4

You also need to install other GMT run-time dependencies separately:

    brew install ghostscript graphicsmagick ffmpeg

### Install via MacPorts

Install [MacPorts](https://www.macports.org) and then the required ports in this order:

    sudo port install gdal +hdf5 +netcdf +openjpeg
    sudo port install gmt6

Optional FFTW-3 support and experimental OpenMP parallel acceleration can be
enabled with the `+fftw3` and `+openmp` flags.

You also need to install other GMT run-time dependencies separately:

    sudo port install graphicsmagick ffmpeg

For the legacy GMT 4 or GMT 5 versions, use:

    sudo port install gmt4

or:

    sudo port install gmt5

### Install via fink

Installation of GMT through [Fink](http://www.finkproject.org/) is quite easy.
All required packages will also be installed.

For the latest GMT 5 version use:

    sudo fink install gmt5

For the legacy GMT 4 version use:

    sudo fink install gmt

The two versions cannot live side by side.

You also need to install other GMT run-time dependencies separately:

    fink install ghostscript graphicsmagick ffmpeg

## Linux

### Fedora

The GMT binary packages provided by the Fedora official repositories are usually too old.
We provide [the GMT official RPM repository](https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt)
to allow Fedora users access the latest GMT releases in a easy way.

Fedora users can add the GMT official RPM repository and install gmt by:

	# enable the RPM repository
	dnf copr enable genericmappingtools/gmt

	# Install the latest GMT provided by the RPM repository
	dnf install gmt

	# Update to the latest version if available
	dnf update gmt

You may also install other optional dependencies for more capabilities within GMT:

    dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
    dnf install GraphicsMagick ffmpeg gdal

**Note**:
If you already installed the GMT packages provided by Fedora,
you have to uninstall them before installing the new GMT packages provided
by the official GMT repository. You can uninstall the older packages by:

    dnf uninstall GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high

### RHEL/CentOS

GMT binary packages are available from Extra Packages for Enterprise Linux (EPEL).
However, EPEL is far hebind packaging a recent version.
We provide [the GMT official RPM repository](https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt)
to allow RHEL/CentOS users access the latest GMT releases in a easy way.

For RHEL/CentOS, run:

    # install epel-release
     and the copr plugin for yum
	yum install epel-release

    # enable the RPM repository (RHEL/CentOS 7 ONLY)
    yum install yum-plugin-copr
	yum copr enable genericmappingtools/gmt

    # enable the RPM repository (RHEL/CentOS 6 ONLY)
    wget https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt/repo/epel-6/genericmappingtools-gmt-epel-6.repo -O /etc/yum.repos.d/genericmappingtools-gmt-epel-6.repo

	# Install GMT
	yum install gmt

    # Update to the latest version if available
	yum update gmt

You may also install other optional dependencies for more capabilities within GMT:

    yum localinstall --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-$(rpm -E %rhel).noarch.rpm
    yum install GraphicsMagick ffmpeg gdal

**Note**:
If you already installed the GMT packages provided by EPEL,
you have to uninstall them before installing the new GMT packages provided
by the official GMT repository. You can uninstall the older packages by:

    yum uninstall GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high


### Ubuntu/Debian

**Note:** For mysterious reasons, many Linux distros are way behind in packing
a recent GMT version. Typically you may find they offer 5.2.1 from 2015 while
the rest of us have moved on to 2018. Your best bet then is to
[build the latest release from source](BUILDING.md).
Otherwise, installing from the distros goes like this:

Install GMT5 via

    sudo apt-get install gmt gmt-dcw gmt-gshhg

Install other GMT dependencies (some are optional) via:

    # required
    sudo apt-get install ghostscript
    # optional
    sudo apt-get install gdal-bin

**Note:** The Ubuntu package under 16.04 LTS for mysterious reasons does not
include the supplements. If you need them you will need to
[build from source](BUILDING.md) or upgrade to 18.04 LTS.

## Cross Platform Install Instructions

### Install via conda

You can use the [conda package manager](https://conda.io/) that comes with the
[Anaconda Python Distribution](https://www.anaconda.com/distribution/) to install GMT.

1. Download and install the [Python **3.7** **64-bit** version of Miniconda](https://conda.io/en/latest/miniconda.html).
   This will give you access to the conda package manager. **Make sure you select to
   have conda added to your `PATH` when asked by the installer**. If you have
   the Anaconda Python distribution installed, you won't need to do this step.
2. Install GMT and its dependencies (including ghostscript and gdal) by running
   the following in a terminal:

   ```
   conda install gmt=6.0.0rc4 -c conda-forge/label/dev -c conda-forge
   ```

3. Install ffmpeg using conda by running the following in a terminal:

   ```
   conda install ffmpeg -c conda-forge
   ```
