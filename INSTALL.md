# Installing GMT

[![GitHub release](https://img.shields.io/github/release/GenericMappingTools/gmt)](https://github.com/GenericMappingTools/gmt/releases)

GMT is available on Windows, macOS and Linux.
Source and binary packages are provided for the latest release,
and can be downloaded from the [GMT main site](https://www.generic-mapping-tools.org)
and [the GitHub repository](https://github.com/GenericMappingTools/gmt/releases).

This file provides instructions for installing GMT binary packages on
different operating systems. Please refer to the [Building Instructions](BUILDING.md)
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
  * [ArchLinux](#archlinux)
- [Cross Platform Install Instructions](#cross-platform-install-instructions)
  * [Install via conda](#install-via-conda)

## Windows

We provide 32 and 64 bit standalone installers (e.g., gmt-6.x.x-win64.exe)
on the [GMT main site](https://www.generic-mapping-tools.org).
The installers come with GDAL, FFmpeg, and Ghostscript pre-installed.

In addition to the GMT installer, you also need to download and install
[GraphicsMagick](http://www.graphicsmagick.org/) if you want to create
animated GIFs.

## macOS

### Application Bundle

We provide macOS application bundle on the [GMT main site](https://www.generic-mapping-tools.org).
The bundle comes with GDAL, FFmpeg, Ghostscript and GraphicsMagick pre-installed.

Download the application bundle (gmt-6.x.x-darwin-x86_64.dmg), double-click to mount it
and drag GMT-6.x.x.app to the "Applications" folder (or any other folder).

GMT-6.x.x.app opens a terminal from which you can invoke GMT programs and scripts.
If you like, you can add the GMT programs contained in the application bundle to
your search path for executables. For that, just run GMT-6.x.x.app once and follow
the instructions at the end of the GMT splash screen.

**Note**: The installer is always built for the latest macOS version only,
and works for macOS Sierra (10.12) or higher.

### Install via Homebrew

Installation of GMT through [Homebrew](https://brew.sh/) is extremely simple.
Installing Homebrew itself is a one line command only (see [the Homebrew page](https://brew.sh/)).
You may need to update the formulas so for that you will do:

    brew update && brew upgrade

For the latest stable GMT 6 release, use:

    brew install gmt

For the latest unstable/developing version (i.e. the master branch), run:

    brew install gmt --HEAD

You also need to install other GMT run-time dependencies separately:

    brew install ghostscript graphicsmagick ffmpeg

If you want to install GMT 5 and GMT 6 alongside, do:

    brew install gmt@5

To go from GMT 6 to GMT 5 (but see also the doc about `gmtswitch`):

    brew unlink gmt && brew link --force gmt@5

And to go from GMT 5 to GMT 6:

    brew unlink gmt@5 && brew link gmt

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

For the latest GMT 6 version, use:

    sudo fink install gmt6

You also need to install other GMT run-time dependencies separately:

    sudo fink install graphicsmagick ffmpeg

For legacy GMT 5 version, use:

    sudo fink install gmt5

For legacy GMT 4 version, use:

    sudo fink install gmt

These three GMT versions cannot live side by side.

## Linux

### Fedora

GMT 6 packages are available for **Fedora 31 or newer**. Install it via:

    dnf install GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high ghostscript

You may also install other optional dependencies for more capabilities within GMT:

    dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-`rpm -E %fedora`.noarch.rpm
    dnf install ffmpeg

### RHEL/CentOS

GMT binary packages are available from Extra Packages for Enterprise Linux (EPEL).
However, EPEL is far hebind packaging a recent version.
We provide [the GMT official RPM repository](https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt)
to allow RHEL/CentOS users access the latest GMT releases in an easy way.

For RHEL/CentOS, run:

    # install epel-release
    yum install epel-release

    # Enable the PowerTools repository (RHEL/CentOS 8 only)
	yum config-manager --set-enabled PowerTools

    # enable the RPM repository (RHEL/CentOS 7 or 8 ONLY)
    yum install yum-plugin-copr
    yum copr enable genericmappingtools/gmt

    # enable the RPM repository (RHEL/CentOS 6 ONLY)
    wget https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt/repo/epel-6/genericmappingtools-gmt-epel-6.repo -O /etc/yum.repos.d/genericmappingtools-gmt-epel-6.repo

    # Install GMT
    yum install gmt

    # Update to the latest version if available
    yum update gmt

You may also install other optional dependencies for more capabilities within GMT:

    yum localinstall --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-`rpm -E %rhel`.noarch.rpm
    yum install GraphicsMagick ffmpeg gdal

**Note**:
If you already installed the GMT packages provided by EPEL,
you have to uninstall them before installing the new GMT packages provided
by the official GMT repository. You can uninstall the older packages by:

    yum remove GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high

### Ubuntu/Debian

GMT 6 packages are available for Ubuntu 20.04 (Focal Fossa) and Debian 11 (Bullseye/Testing).
Install it via

    sudo apt-get install gmt gmt-dcw gmt-gshhg

**Note** that the above command will install GMT 5.4 for older Ubuntu/Debian versions,
e.g. Ubuntu 18.04 Bionic Beaver and Debian 10 Buster/Stable.
If you want the latest GMT 6.0 release, your best bet then is to
[build the latest release from source](BUILDING.md).
Keep in mind that Ubuntu 16.04 LTS for mysterious reasons does not
include the [supplemental modules](https://docs.generic-mapping-tools.org/latest/modules.html#supplemental-modules),
but you can obtain them by [building from source](BUILDING.md) or upgrading to Ubuntu 18.04 LTS (or newer).

Install other GMT dependencies (some are optional) via:

    # required
    sudo apt-get install ghostscript
    # optional
    sudo apt-get install gdal-bin graphicsmagick ffmpeg

### ArchLinux

It's easier to manage installed content and dependencies than directly
building and manually `make install` in ArchLinux.

    # Full update system packages first
    sudo pacman -Syu

    # Install tools for building AUR packages
    sudo pacman -S base-devel

    # Use git command to clone non-official AUR repo of gmt
    # Here are two options. **You just have to clone one of them:**
    # If you encounter some problems in those repo,
    # you can report to the authors first, or choose another one to clone
    git clone https://aur.archlinux.org/gmt.git

    # alternative repo, for testing stable/rc release in version 6.x branch
    git clone https://aur.archlinux.org/gmt6.git gmt

    # Optional recommended packages
    git clone https://aur.archlinux.org/gmt-coast.git
    git clone https://aur.archlinux.org/gmt-cpt-city.git
    git clone https://aur.archlinux.org/gmt-dcw.git

    # use makepkg to build packages and use pacman to install
    cd gmt
    makepkg -sc
    sudo pacman -U *.pkg.tar.xz

**Note**: Binary packages of gmt are still not available in [ArchlinuxCN repo](https://www.archlinuxcn.org/archlinux-cn-repo-and-mirror) yet.

## Cross Platform Install Instructions

### Install via conda

You can use the [conda package manager](https://conda.io/) that comes with the
[Anaconda Python Distribution](https://www.anaconda.com/distribution/) to install GMT.

1. Download and install the [Python **3.7** **64-bit** version of Miniconda](https://conda.io/en/latest/miniconda.html).
   This will give you access to the conda package manager. **Make sure you select to
   have conda added to your `PATH` when asked by the installer**. If you have
   the Anaconda Python distribution installed, you won't need to do this step.
2. Install GMT and its dependencies (including ghostscript, gdal, ffmpeg and graphicsmagick)
   by running the following in a terminal:

   ```
   conda install gmt -c conda-forge
   ```

3. If you want to install GMT 5, use:

   ```
   conda install gmt=5 -c conda-forge
   ```
