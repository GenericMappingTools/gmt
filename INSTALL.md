# Installing GMT

[![GitHub release](https://img.shields.io/github/release/GenericMappingTools/gmt)](https://github.com/GenericMappingTools/gmt/releases)

GMT is available on Windows, macOS, Linux, FreeBSD and OpenBSD.
Source and binary packages are provided for the latest release,
and can be downloaded from the [GitHub repository](https://github.com/GenericMappingTools/gmt/releases).

This file provides instructions for installing GMT binary packages on
different operating systems. Please refer to the [Building Instructions](BUILDING.md)
for compiling GMT source package (either stable release or development version).

## Contents

- [Windows](#windows)
- [macOS](#macos)
  * [Application Bundle](#application-bundle)
  * [Install via Homebrew](#install-via-homebrew)
  * [Install via Macports](#install-via-macports)
- [Linux](#linux)
  * [Fedora](#fedora)
  * [RHEL/CentOS](#rhelcentos)
  * [Ubuntu/Debian](#ubuntudebian)
  * [ArchLinux](#archlinux)
- [Cross Platform Install Instructions](#cross-platform-install-instructions)
  * [Install via conda](#install-via-conda)
- [FreeBSD](#freebsd)
  * [Install via Ports](#install-via-freebsd-ports)
- [OpenBSD](#openbsd)
  * [Install via Ports](#install-via-openbsd-ports)

## Windows

We provide 32 and 64 bit standalone installers (e.g., gmt-6.x.x-win64.exe)
in the [GitHub repository](https://github.com/GenericMappingTools/gmt/releases).
The installers come with GDAL, FFmpeg, and Ghostscript pre-installed.

In addition to the GMT installer, you also need to download and install
[GraphicsMagick](http://www.graphicsmagick.org/) if you want to create
animated GIFs.

**NOTE:**
At the installation step, you may get the warning message:

> Warning! Failed to add GMT to PATH. Please add the GMT bin path to PATH manually.

Usually it means your system variable `PATH` is already too long and the GMT
installer can't add its path to the variable. As it says, you need to ignore
the warning message, and then manually add the GMT bin path
(e.g., `C:\programs\gmt6`) to `PATH` after finishing the installation.
If you don't know how to manually modify `PATH`, just search Google for
"How to change windows path variable".

## macOS

### Application Bundle

We provide macOS application bundle in the [GitHub repository](https://github.com/GenericMappingTools/gmt/releases).
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

GMT is installed in `/opt/local/lib/gmt6`. To use GMT in command line or scripts, 
you need to add `/opt/local/lib/gmt6/bin` to your `PATH`. 

You also need to install other GMT run-time dependencies separately:

    sudo port install graphicsmagick ffmpeg

For the legacy GMT 4 or GMT 5 versions, use:

    sudo port install gmt4

or:

    sudo port install gmt5

## Linux

### Fedora

**NOTE:** The Fedora official repository may provide an old GMT version.
If you need the latest GMT version, you can follow the instruction
["Install latest GMT on Fedora"](https://github.com/GenericMappingTools/gmt/wiki/Install-latest-GMT-on-Fedora)
in the wiki.

Install GMT via:

    dnf install GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high ghostscript

You may also install other optional dependencies for more capabilities within GMT:

    dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-`rpm -E %fedora`.noarch.rpm
    dnf install ffmpeg

### RHEL/CentOS

GMT binary packages are available from Extra Packages for Enterprise Linux (EPEL).

**NOTE:** The EPEL repository may provide an old GMT version.
If you need the latest GMT version, you can follow the instruction
["Install latest GMT on RHEL/CentOS"](https://github.com/GenericMappingTools/gmt/wiki/Install-latest-GMT-on-RHEL-CentOS)
in the wiki.

Install GMT via:

    yum install epel-release
    yum install GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high ghostscript

You may also install other optional dependencies for more capabilities within GMT:

    yum localinstall --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-`rpm -E %rhel`.noarch.rpm
    yum install ffmpeg

### Ubuntu/Debian

**NOTE:** The Ubuntu/Debian official repositories may provide old GMT versions.
If you want the latest GMT 6.x release, your best bet then is to
[build the latest release from source](BUILDING.md).
Keep in mind that Ubuntu 16.04 LTS for mysterious reasons does not
include the [supplemental modules](https://docs.generic-mapping-tools.org/latest/modules.html#supplemental-modules),
but you can obtain them by [building from source](BUILDING.md) or upgrading to Ubuntu 18.04 LTS (or newer).

Install GMT via:

    sudo apt-get install gmt gmt-dcw gmt-gshhg

Install other GMT dependencies (some are optional) via:

    # required
    sudo apt-get install ghostscript
    # optional
    sudo apt-get install gdal-bin graphicsmagick ffmpeg

### ArchLinux

ArchLinux official repository doesn't provide GMT packages, but AUR (ArchLinux User Repository) does.
You can follow the [Install latest GMT on ArchLinux](https://github.com/GenericMappingTools/gmt/wiki/Install-latest-GMT-on-ArchLinux) in the wiki.

## Cross Platform Install Instructions

### Install via conda

You can use the [conda package manager](https://conda.io/) that comes with the
[Anaconda Python Distribution](https://www.anaconda.com/distribution/) to install GMT.

1. Download and install the [Python **3.8** **64-bit** version of Miniconda](https://conda.io/en/latest/miniconda.html).
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

4. If you want to install the weekly snapshot of the GMT master branch, use:
   ```
   conda install gmt -c conda-forge/label/dev
   ```

## FreeBSD

GMT may be installed on FreeBSD using Ports or from source.

**NOTE:** The Ports Collection may provide old GMT versions. If you want the latest GMT release, consider [building the
latest release from source](BUILDING.md).

### Install via Ports

The FreeBSD Ports Collection is a diverse collection of utility and application software that has been ported to FreeBSD.

**Precompiled**

Install precompiled gmt binaries with

```
$ pkg install gmt
```

**Compile from Ports**

If not done already, set up the **Ports Collection** (see
https://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/ports-using.html):

```
portsnap fetch
portsnap extract
```

If already set up, make sure you're up-to-date:

```
portsnap fetch update
```

Then change into directory `/usr/ports/graphics/gmt` and build:

```
make install clean
```

## OpenBSD

GMT may be installed on OpenBSD using Ports or from source.

**NOTE:** The Ports Collection may provide old GMT versions. If you want the latest GMT release, consider [building the
latest release from source](BUILDING.md).

### Install via Ports
For more information, please refer to relevant documentation from the OpenBSD project.

**Precompiled**

Install precompiled gmt binaries with

```
$ pkg_add gmt
```
