# Installing GMT

GMT is available on Windows, macOS and Linux.
Its source code can be downloaded through tarballs or
[the git repository](https://github.com/GenericMappingTools/gmt/).
Source and binary packages are provided for the current version.

## Linux

**Note:** For mysterious reasons, many Linux distros are way behind in packing
a recent GMT version. Typically you may find they offer 5.2.1 from 2015 while
the rest of us have moved on to 2018. Your best bet then is to
[build the latest release from source](BUILDING.md).
Otherwise, installing from the distros goes like this:

### Ubuntu/Debian

Install GMT5 via

    sudo apt-get install gmt gmt-dcw gmt-gshhg

However, the Ubuntu package under 16.04 LTS for mysterious reasons does not
include the supplements. If you need them you will need to
[build from source](BUILDING.md) or upgrade to 18.04 LTS.

### RHEL, CentOS, Fedora

Install GMT5 via

    sudo yum install GMT gshhg-gmt-nc4-all dcw-gmt

### Install via conda

You can use the [conda package manager](https://conda.io/) that comes with the
[Anaconda Python Distribution](https://www.anaconda.com/distribution/) to install GMT:

    conda install gmt --channel conda-forge

## macOS

### Standalone Installer

[Download the application bundle](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download) (gmt-5.x.x-darwin-x86_64.dmg)
and drag GMT-5.x.x.app to any folder. This bundle includes a self contained GMT installation.
GMT-5.x.x.app opens a terminal from which you can invoke GMT programs and scripts.
If you like, you can add the GMT programs contained in the application bundle to
your search path for executables. For that, just run GMT.app once and type:

    echo ${PATH%%:*}

in the terminal. Then prepend this directory to your PATH environment variable,
e.g., in ~/.bashrc. Note: The installer is always built for the latest macOS version only.

### Install GMT 5 via macports

Install [macports](https://www.macports.org/) and then the required ports in this order:

    sudo port install gdal +curl +geos +hdf5 +netcdf
    sudo port install gmt5

A legacy GMT 4 port, gmt4, is available too and a side by side installation is possible.
Optional FFTW-3 support and experimental OpenMP parallel acceleration can be
enabled with the +fftw3 and +openmp flags.

### Install via fink

Installation of GMT through [Fink](http://www.finkproject.org/) is quite easy.
All required packages will also be installed. Ghostscript is not strictly
required but very convenient to view PS files.

For the latest GMT 5 version use:

    sudo fink install gmt5

For the legacy GMT 4 version use:

    sudo fink install gmt

The two versions cannot live side by side.

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

### Install via conda

You can use the [conda package manager](https://conda.io/) that comes with the
[Anaconda Python Distribution](https://www.anaconda.com/distribution/) to install GMT:

    conda install gmt --channel conda-forge

### Windows

We provide 32 and 64 bit standalone installers (e.g., gmt-5.x.x-win32.exe)
in the [downloads section](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download).
If you need the newest development version refer to the instructions for
[building GMT from source](BUILDING.md).

### Source

Download the latest stable source release from the [downloads page](http://gmt.soest.hawaii.edu/projects/gmt/wiki/Download)
or get the latest development version from Github:

    git clone https://github.com/GenericMappingTools/gmt.git

and refer to the included documentation or the online instructions for building GMT from source.
