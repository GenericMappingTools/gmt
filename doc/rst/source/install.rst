Installing GMT
==============

.. image:: https://img.shields.io/github/release/GenericMappingTools/gmt
   :target: https://github.com/GenericMappingTools/gmt/releases
.. image:: https://img.shields.io/ubuntu/v/gmt
.. image:: https://img.shields.io/debian/v/gmt
.. image:: https://img.shields.io/fedora/v/GMT
.. image:: https://img.shields.io/archlinux/v/extra/x86_64/gmt
.. image:: https://img.shields.io/homebrew/v/gmt
.. image:: https://img.shields.io/conda/v/conda-forge/gmt

GMT is available on Windows, macOS, Linux and FreeBSD. Source and binary
packages are provided for the latest release, and can be downloaded from
the `GitHub repository <https://github.com/GenericMappingTools/gmt/releases>`__.

This section provides instructions for installing GMT binary packages on
different operating systems. Please refer to the
`Building Instructions <https://github.com/GenericMappingTools/gmt/blob/master/BUILDING.md>`__
for compiling GMT source package (either stable release or development version).

Windows
-------

We provide 32 and 64 bit standalone installers (e.g.,
gmt-6.x.x-win64.exe) in the `GitHub
repository <https://github.com/GenericMappingTools/gmt/releases>`__. The
installers come with GDAL, FFmpeg, and Ghostscript pre-installed.

In addition to the GMT installer, you also need to download and install
`GraphicsMagick <http://www.graphicsmagick.org/>`__ if you want to
create animated GIFs.

**NOTE:** There are several options for using :doc:`/reference/non-unix-platforms`
such as Windows, including `Windows Subsystem for
Linux <https://docs.microsoft.com/en-us/windows/wsl/>`__, MinGW/MSYS2,
Cygwin, or DOS batch scripts. The last option will not provide you with
any UNIX tools so you will be limited to what you can do with DOS batch
files. One simple option for accessing a UNIX style bash terminal is
*Git for Windows*, which can be downloaded from `their official
website <https://gitforwindows.org/>`__.

**NOTE:** At the installation step, you may get the warning message:

>   Warning! Failed to add GMT to PATH. Please add the GMT bin path to
>   PATH manually.

Usually it means your system variable ``PATH`` is already too long and
the GMT installer can’t add its path to the variable. As it says, you
need to ignore the warning message, and then manually add the GMT bin
path (e.g., ``C:\programs\gmt6\bin``) to ``PATH`` after finishing the
installation. If you don’t know how to manually modify ``PATH``, just
search Google for “How to change windows path variable”.

macOS
-----

Application Bundles
~~~~~~~~~~~~~~~~~~~

We provide macOS application bundles for Intel and ARM architectures in
the `GitHub
repository <https://github.com/GenericMappingTools/gmt/releases>`__. The
bundles come with GDAL, FFmpeg, Ghostscript and GraphicsMagick
pre-installed.

Download the suitable application bundle (gmt-6.x.x-darwin-x86_64.dmg or
gmt-6.x.x-darwin-arm64.dmg), double-click to mount it and drag
GMT-6.x.x.app to the “Applications” folder (or any other folder).

GMT-6.x.x.app opens a terminal from which you can invoke GMT programs
and scripts. If you like, you can add the GMT programs contained in the
application bundle to your search path for executables. For that, just
run GMT-6.x.x.app once and follow the instructions at the end of the GMT
splash screen.

**Note**: The installers are always built for the latest macOS version
only. The table below lists macOS compatibility requirements for the
bundle. The arm64 version requires a computer with the M1 Apple Silicon
chip.

=============== =========================
**GMT Version** **Minimum MacOS**
=============== =========================
6.4             macOS 12 (Monterey)
6.3             macOS 10.15 (Catalina)
6.2             macOS 10.15 (Catalina)
6.1             macOS 10.15 (Catalina)
6.0             macOS 10.13 (High Sierra)
5.4             macOS 10.12 (Sierra)
=============== =========================

Install via Homebrew
~~~~~~~~~~~~~~~~~~~~

Installation of GMT through `Homebrew <https://brew.sh/>`__ is extremely
simple. Installing Homebrew itself is a one line command only (see `the
Homebrew page <https://brew.sh/>`__). You may need to update the
formulas so for that you will do:

::

   brew update && brew upgrade

For the latest stable GMT 6 release, use:

::

   brew install gmt

For the latest unstable/developing version (i.e. the master branch),
run:

::

   brew install gmt --HEAD

You also need to install other GMT run-time dependencies separately:

::

   brew install ghostscript graphicsmagick ffmpeg

If you want to install GMT 5 and GMT 6 alongside, do:

::

   brew install gmt@5

To go from GMT 6 to GMT 5 (but see also the doc about ``gmtswitch``):

::

   brew unlink gmt && brew link --force gmt@5

And to go from GMT 5 to GMT 6:

::

   brew unlink gmt@5 && brew link gmt

Install via MacPorts
~~~~~~~~~~~~~~~~~~~~

Install `MacPorts <https://www.macports.org>`__ and then the required
ports in this order:

::

   sudo port install gdal +hdf5 +netcdf +openjpeg
   sudo port install gmt6

Optional FFTW-3 support and experimental OpenMP parallel acceleration
can be enabled with the ``+fftw3`` and ``+openmp`` flags.

GMT is installed in ``/opt/local/lib/gmt6``. To use GMT in command line
or scripts, you need to add ``/opt/local/lib/gmt6/bin`` to your
``PATH``.

You also need to install other GMT run-time dependencies separately:

::

   sudo port install graphicsmagick ffmpeg

For the legacy GMT 4 or GMT 5 versions, use:

::

   sudo port install gmt4

or:

::

   sudo port install gmt5

Linux
-----

Fedora
~~~~~~

**NOTE:** The Fedora official repository may provide an old GMT version.
If you need the latest GMT version, you can follow the instruction
`“Install latest GMT on
Fedora” <https://github.com/GenericMappingTools/gmt/wiki/Install-latest-GMT-on-Fedora>`__
in the wiki.

Install GMT via:

::

   sudo dnf install GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high ghostscript

You may also install other optional dependencies for more capabilities
within GMT:

::

   sudo dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-`rpm -E %fedora`.noarch.rpm
   sudo dnf install ffmpeg GraphicsMagick

RHEL/CentOS
~~~~~~~~~~~

GMT binary packages are available from Extra Packages for Enterprise
Linux (EPEL).

**NOTE:** The EPEL repository may provide an old GMT version. If you
need the latest GMT version, you can follow the instruction `“Install
latest GMT on
RHEL/CentOS” <https://github.com/GenericMappingTools/gmt/wiki/Install-latest-GMT-on-RHEL-CentOS>`__
in the wiki.

Install GMT via:

::

   sudo yum install epel-release
   sudo yum install GMT dcw-gmt gshhg-gmt-nc4 gshhg-gmt-nc4-full gshhg-gmt-nc4-high ghostscript

You may also install other optional dependencies for more capabilities
within GMT:

::

   sudo yum localinstall --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-`rpm -E %rhel`.noarch.rpm
   sudo yum install ffmpeg GraphicsMagick

Ubuntu/Debian
~~~~~~~~~~~~~

**NOTE:** The Ubuntu/Debian official repositories may provide old GMT
versions. If you want the latest GMT 6.x release, your best bet then is
to `build the latest release from source <BUILDING.md>`__.

Install GMT via:

::

   sudo apt-get install gmt gmt-dcw gmt-gshhg

Install other GMT dependencies (some are optional) via:

::

   # required
   sudo apt-get install ghostscript
   # optional
   sudo apt-get install gdal-bin graphicsmagick ffmpeg

ArchLinux
~~~~~~~~~

Install GMT via:

::

   sudo pacman -S gmt

ArchLinux official repository doesn’t provide GMT extra data yet, but
AUR (ArchLinux User Repository) does. You can follow the `Install latest
GMT on
ArchLinux <https://github.com/GenericMappingTools/gmt/wiki/Install-latest-GMT-on-ArchLinux>`__
in the wiki for those.

Gentoo
~~~~~~

**NOTE:** This may provide old GMT versions. Consider `building from
source <BUILDING.md>`__.

Install GMT via:

::

   sudo emerge --verbose --ask sci-geosciences/gmt

Cross Platform Install Instructions
-----------------------------------

Install via conda
~~~~~~~~~~~~~~~~~

You can use the `conda package manager <https://conda.io/>`__ that comes with the
`Anaconda Python Distribution <https://www.anaconda.com/download>`__
or `Miniconda <https://www.anaconda.com/download>`__ (recommended)
to install GMT.

1. Download and install the `latest
   Miniconda <https://conda.io/en/latest/miniconda.html>`__. This will
   give you access to the conda package manager. **Make sure you select
   to have conda added to your ``PATH`` when asked by the installer**.
   If you have the Anaconda Python distribution installed, you won’t
   need to do this step.

2. Install GMT and its dependencies (including ghostscript and gdal) by
   running the following in a terminal:

   ::

      conda install gmt -c conda-forge

3. If you want to install GMT 5, use:

   ::

      conda install gmt=5 -c conda-forge

4. If you want to install the weekly snapshot of the GMT master branch,
   use:

   ::

      conda install gmt -c conda-forge/label/dev

5. Install other optional dependencies if you want to create animated
   GIFs or MP4:

   ::

      conda install ffmpeg graphicsmagick -c conda-forge

FreeBSD
-------

GMT may be installed on FreeBSD using Ports or from source.

**NOTE:** The Ports Collection may provide old GMT versions. If you want
the latest GMT release, consider `building the latest release from
source <BUILDING.md>`__.

Install via Ports
~~~~~~~~~~~~~~~~~

The FreeBSD Ports Collection is a diverse collection of utility and
application software that has been ported to FreeBSD.

**Precompiled**

Install precompiled gmt binaries with

::

   pkg install gmt

**Compile from Ports**

If not done already, set up the **Ports Collection** See
https://docs.freebsd.org/en/books/handbook/ports/#ports-using:

::

   portsnap fetch
   portsnap extract

If already set up, make sure you’re up-to-date:

::

   portsnap fetch update

Then change into directory ``/usr/ports/graphics/gmt`` and build:

::

   make install clean

