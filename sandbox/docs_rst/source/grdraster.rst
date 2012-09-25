*********
grdraster
*********

grdraster - Extract subregion from a binary raster and save as a GMT
grid

`Synopsis <#toc1>`_
-------------------

**grdraster** [ *filenumber* \| *"text pattern"* ]
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-G**\ *grdfile* ] [
**-I**\ *xinc*\ [**m**\ \|\ **s**][/\ *yinc*\ [**m**\ \|\ **s**]] ] [
**-J**\ *parameters* ] [ **-V**\ [*level*\ ] ] [
**-bo**\ [*ncols*\ ][*type*\ ] ] [ **-o**\ *cols*\ [,*...*] ]

`Description <#toc2>`_
----------------------

**grdraster** reads a file called *grdraster.info* from the current
working directory, the directories pointed to by the environment
variables **$GMT\_USERDIR** and **$GMT\_DATADIR**, or in
**$GMT\_SHAREDIR**/dbase (in that order). The file *grdraster.info*
defines binary arrays of data stored in scan-line format in data files.
Each file is given a *filenumber* in the info file. **grdraster**
figures out how to load the raster data into a grid file spanning a
region defined by **-R**. By default the grid spacing equals the raster
spacing. The **-I** option may be used to sub-sample the raster data. No
filtering or interpolating is done, however; the *x\_inc* and *y\_inc*
of the grid must be multiples of the increments of the raster file and
**grdraster** simply takes every n’th point. The output of BD(grdraster)
is either grid or pixel registered depending on the registration of the
raster used. It is up to the **GMT** system person to maintain the
*grdraster.info* file in accordance with the available rasters at each
site. Raster data sets are not supplied with **GMT** but can be obtained
by anonymous ftp and on CD-ROM (see README page in dbase directory).
**grdraster** will list the available files if no arguments are given.
Finally, **grdraster** will write xyz-triplets to stdout if no output
gridfile name is given

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*filenumber*
    If an integer matching one of the files listed in the
    *grdraster.info* file is given we will use that data set, else we
    will match the given text pattern with the data set description in
    order to determine the data set.
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]
    *west*, *east*, *south*, and *north* specify the region of interest,
    and you may specify them in decimal degrees or in
    [+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left
    and upper right map coordinates are given instead of w/e/s/n. The
    two shorthands **-Rg** and **-Rd** stand for global domain (0/360
    and -180/+180 in longitude respectively, with -90/+90 in latitude).
    Alternatively, specify the name of an existing grid file and the
    **-R** settings (and grid spacing, if applicable) are copied from
    the grid. If **r** is appended, you may also specify a map
    projection to define the shape of your region. The output region
    will be rounded off to the nearest whole grid-step in both
    dimensions.

`Optional Arguments <#toc5>`_
-----------------------------

**-G**\ *grdfile*
    Name of output grid file. If not set, the grid will be written as
    ASCII (or binary; see **-bo**\ [*ncols*\ ][*type*\ ]) xyz-triplets
    to stdout instead.
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
    *x\_inc* [and optionally *y\_inc*] is the grid spacing. Optionally,
    append a suffix modifier. **Geographical (degrees) coordinates**:
    Append **m** to indicate arc minutes or **s** to indicate arc
    seconds. If one of the units **e**, **f**, **k**, **M**, or **n** is
    appended instead, the increment is assumed to be given in meter,
    feet, km, Miles, or nautical miles, respectively, and will be
    converted to the equivalent degrees longitude at the middle latitude
    of the region (the conversion depends on **PROJ\_ELLIPSOID**). If
    /*y\_inc* is given but set to 0 it will be reset equal to *x\_inc*;
    otherwise it will be converted to degrees latitude. **All
    coordinates**: If **=** is appended then the corresponding max *x*
    (*east*) or *y* (*north*) may be slightly adjusted to fit exactly
    the given increment [by default the increment may be adjusted
    slightly to fit the given domain]. Finally, instead of giving an
    increment you may specify the *number of nodes* desired by appending
    **+** to the supplied integer argument; the increment is then
    recalculated from the number of nodes and the domain. The resulting
    increment value depends on whether you have selected a
    gridline-registered or pixel-registered grid; see Appendix B for
    details. Note: if **-R**\ *grdfile* is used then the grid spacing
    has already been initialized; use **-I** to override the values.
**-J**\ *parameters* (\*)
    Select map projection.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-bo**\ [*ncols*\ ][*type*\ ] (\*)
    Select binary output.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns. This option applies only if no **-G** option
    has been set.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Examples <#toc6>`_
-------------------

To extract data from raster 1, taking one point every 30 minutes, in an
area extended beyond 360 degrees to allow later filtering, run

**grdraster** 1 **-R**-4/364/-62/62 **-I**\ 30\ **m** **-G**\ data.nc

To obtain data for an oblique Mercator projection we need to extract
more data that is actually used. This is necessary because the output of
**grdraster** has edges defined by parallels and meridians, while the
oblique map in general does not. Hence, to get all the data from the
ETOPO2 data needed to make a contour map for the region defined by its
lower left and upper right corners and the desired projection, use

**grdraster** ETOPO2 **-R**\ 160/20/220/30\ **r**
**-Joc**\ 190/25.5/292/69/1 **-G**\ data.nc

To extract data from the 2 min Geoware relief blend and write it as
binary double precision xyz-triplets to standard output:

**grdraster** "2 min Geoware" **-R**\ 20/25/-10/5 **-bo** >! triplets.b

`See Also <#toc7>`_
-------------------

`*gmtdefaults*\ (1) <gmtdefaults.html>`_ , `*GMT*\ (1) <GMT.html>`_ ,
`*grdsample*\ (1) <grdsample.html>`_ ,
`*grdfilter*\ (1) <grdfilter.html>`_

`References <#toc8>`_
---------------------

Wessel, P., W. H. F. Smith, R. Scharroo, and J. Luis, 2011, The Generic
Mapping Tools (GMT) version 5.0.0b Technical Reference & Cookbook,
SOEST/NOAA.
 Wessel, P., and W. H. F. Smith, 1998, New, Improved Version of Generic
Mapping Tools Released, EOS Trans., AGU, 79 (47), p. 579.
 Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released, EOS Trans., AGU, 76 (33), p. 329.
 Wessel, P., and W. H. F. Smith, 1995, New Version of the Generic
Mapping Tools Released,
`http://www.agu.org/eos\_elec/95154e.html, <http://www.agu.org/eos_elec/95154e.html,>`_
Copyright 1995 by the American Geophysical Union.
 Wessel, P., and W. H. F. Smith, 1991, Free Software Helps Map and
Display Data, EOS Trans., AGU, 72 (41), p. 441.

