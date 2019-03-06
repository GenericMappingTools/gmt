# Building GMT with CMake

For CMake related questions refer to the CMake manual which is available
online: http://www.cmake.org/cmake/help/documentation.html

To avoid CMake's lengthy command line options you can create your custom
ConfigUser.cmake file in the cmake directory. Simply duplicate the
cmake/ConfigUserTemplate.cmake to cmake/ConfigUser.cmake and then make
changes in cmake/ConfigUser.cmake. See additional comments in that file.

There are two configuration files:

- "ConfigDefault.cmake" -- is version controlled and used to add new default
    variables and set defaults for everyone. You should not edit this file.
- "ConfigUser.cmake" -- is not version controlled and used to override defaults
    on a per-user basis.
    There is a template file, ConfigUserTemplate.cmake, that you should copy
    to ConfigUser.cmake and make your changes therein.

NOTE: If you want to change CMake behaviour for your build then only modify
      the "ConfigUser.cmake" file (not "ConfigDefault.cmake").

## Configuring

GMT can be build on any platform supported by CMake.  CMake is a
cross-platform, open-source system for managing the build process.
Refer to README.CMake for further details.  In the source tree copy
cmake/ConfigUserTemplate.cmake to cmake/ConfigUser.cmake and edit
the file according to your demands.

By default, GMT will use Dave Watson's Delaunay triangulation routine.
However, a much faster alternative is available from Jonathan Shewchuk, but
his routine is not distributed under the GNU Public License.  If you work for
a for-profit organization you should read Shewchuk's copyright statement (in
src/triangle.c) first.  If you agree with the license terms you can enable
Shewchuk's triangulation routine in cmake/ConfigUser.cmake.

At run-time, GMT will initialize all default variables. You can change
this by adding a gmt.conf file in your current or home directory
and edit those settings since GMT will check for that file before loading
system defaults (actually, it will first look in the current directory, then
the home directory, and then finally in share).  See the gmt.conf man page
for a description of all defaults.

To prevent two GMT processes writing to the same gmt.conf file simultaneously
(thereby corrupting it), GMT can implement the POSIX advisory file locking
scheme and sets and releases locks on these files.  This might not be reliable
when the files reside in directories on network filesystems, such as NFS.
Whether flock works on network filesystems is implementation dependent.  If
you want to activate file locking you may enable it in cmake/ConfigUser.cmake.

By default, both GMT and all its supplements are built.  You can turn
off all supplements via the BUILD_SUPPLEMENTS setting in ConfigUsers.cmake

The top-level installation directory is configured with the variable
CMAKE\_INSTALL\_PREFIX.

Now that you made your configuration choices it is time for invoking CMake.
Create a subdirectory where the build files will be generated, e.g., in the
source tree 'mkdir build'.

In the build subdirectory, type

```
cmake [options] ..
```

Append any of the options explained above as you see fit.  If CMake cannot
figure out all the dependent libraries or required compiler and linker flags
it will give you a message and you will be asked to edit
cmake/ConfigUser.cmake.


## Finding GSHHG

GSHHG shorelines are searched in FindGSHHG.cmake and a little helper program
tests the version (gshhg_version.c). If CMake cannot find the shorelines you
have to configure _GSHHG\_ROOT_ in cmake/ConfigUser.cmake.

## Finding DCW

DCW (Digital Chart of the World) country polygons are searched at compile time.
The DCW data are optional; they are currently used in pscoast -E for painting
individual countries only.  If CMake cannot auto-find DCW for you then you can
configure _DCW_ROOT_ in cmake/ConfigUser.cmake.

## Build GMT

In the build directory, type

```
make -j all
```

which will compile all the programs.

### Build documentation

The GMT manual is available in different formats and can be generated with:

```
make -j docs_man        # UNIX manual pages
make -j docs_html       # HTML manual, cookbook, and API reference
make -j docs_pdf        # PDF manual, cookbook, and API reference
make -j docs_pdf_shrink # Like docs_pdf but with reduced size
```

To generate the documentation you need to install the Sphinx documentation
builder, and for PDFs you also need LaTeX.  You can choose to install the
documentation files from an external location instead of generating the
Manpages, PDF, and HTML files from the sources.  This is convenient if Sphinx
and/or LaTeX are not available.  Set GMT_INSTALL_EXTERNAL_DOC in
cmake/ConfigUser.cmake.

## Installing

```
make -j install
```

will install libps, libgmt, and the gmt executable. Optionally it
will install the GSHHG shorelines (if found), DCW (if found), UNIX manpages,
and HTML and PDF documentation.

You have to explicitly make the latter two beforehand or they will be omitted
during installation.  You have the choice between installing into a
traditional directory structure (_-DGMT\_INSTALL\_TRADITIONAL\_FOLDERNAMES=ON_)
where everything goes into a common subdirectory or a distribution-like
installation (PREFIX/bin/gmtSUFFIX, PREFIX/lib/gmtSUFFIX/,
PREFIX/include/gmtSUFFIX/, PREFIX/share/gmtSUFFIX/,
PREFIX/share/doc/gmtSUFFIX/).

GMT is shipped as a single executable.  By default we also set "classic mode"
by installing convenience links for all GMT modules. New GMT users should set
GMT\_INSTALL\_MODULE\_LINKS to FALSE in cmake/ConfigUser.cmake.

## Updating

Assuming you did not delete the build directory and that your current
working directory is the build directory this is just as simple as

```
cd ..
git pull
cd -
make -j install
```

CMake will detect any changes to the source files and will automatically
reconfigure. If you deleted all files inside the build directory you have to
run cmake again manually.

## Packaging

Currently, packaging with CPack works on MacOSX (Bundle, TGZ, TBZ2),
Windows (ZIP, NSIS), and UNIX (TGZ, TBZ2). On Windows you need to install NSIS
(http://nsis.sourceforge.net/). After building GMT and the documentation run
either one of these:

```
make package
cpack -G <TGZ|TBZ2|Bundle|ZIP|NSIS>
```

## Creating a source package

Set GMT\_RELEASE\_PREFIX in cmake/ConfigUser.cmake and run cmake. Then do

```
make -j docs_depends # optional but increases speed (parallel build)
make gmt_release      # export the source tree and install doc
```

You should then edit ${GMT_RELEASE_PREFIX}/cmake/ConfigDefault.cmake and
set GMT\_PACKAGE\_VERSION\_MAJOR, GMT\_PACKAGE\_VERSION\_MINOR, and
GMT\_PACKAGE\_VERSION\_PATCH. Also uncomment and set
GMT\_SOURCE\_CODE\_CONTROL\_VERSION\_STRING to the current git version. Then
create tarballs with:

```
make -j gmt_release_tar
```

## In-build-dir tests

A complete set of the example scripts used to create all the example plots,
including all necessary data files, are provided by the installation.
Examples and tests can be build inside GMT\_BINARY\_DIR *without* installing.
This is very convenient for testing. Just _enable\_testing()_, set
_DO\_EXAMPLES_ and/or _DO\_TESTS_ in ConfigUser.cmake and do:

```
make -j
make check
```

Optionally set _N\_TEST\_JOBS_ to the number of ctest jobs to run
simultaneously. You can also select individual tests using regexp with ctest,
e.g.:

```
ctest -R ex2[3-6]
```

Tests are preferably run through valgrind and GMT's internal memory tracker.
Configure cmake with _add\_definitions(-DMEMDEBUG)_ and prior to invoking the
test, export the following variables:

```
export GMT_TRACK_MEMORY=2
export VALGRIND_ARGS="--track-origins=yes --leak-check=full"
```

The test script will keep gmt_memtrack_<pid>.log and valgrind\_<pid>.log files
in the test directories for later inspection, when memory related errors were
identified.
