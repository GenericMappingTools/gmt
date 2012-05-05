******
psmask
******


psmask - Use data tables to clip or mask map areas with no coverage

`Synopsis <#toc1>`_
-------------------

**psmask** [ *table* ]
**-I**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
**-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-D**\ *dumpfile* ] [
**-G**\ *fill* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-N** ] [ **-O** ] [ **-P** ] [ **-Q**\ *cut* ] [
**-S**\ *search\_radius*\ [*unit*\ ] ] [ **-T** ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-bi**\ [*ncol*\ ][**t**\ ] ] [ **-c**\ *copies* ] [
**-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-r** ] [ **-t**\ [*transp*\ ] ] [ **-:**\ [**i**\ \|\ **o**] ]

**psmask** **-C** [ **-K** ] [ **-O** ]

`Description <#toc2>`_
----------------------

**psmask** reads a (*x*,\ *y*,\ *z*) file [or standard input] and uses
this information to find out which grid cells are reliable. Only grid
cells which have one or more data points are considered reliable. As an
option, you may specify a radius of influence. Then, all grid cells that
are within *radius* of a data point are considered reliable.
Furthermore, an option is provided to reverse the sense of the test.
Having found the reliable/not reliable points, **psmask** will either
paint tiles to mask these nodes (with the **-T** switch), or use
contouring to create polygons that will clip out regions of no interest.
When clipping is initiated, it will stay in effect until turned off by a
second call to **psmask** using the **-C** option.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

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
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.

`Optional Arguments <#toc5>`_
-----------------------------

*table*
    One or more ASCII (or binary, see **-bi**\ [*ncol*\ ][**t**\ ]) data
    table file(s) holding a number of data columns. If no tables are
    given then we read from standard input.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**
    Mark end of existing clip path. No input file is needed. Implicitly
    sets **-O**. Also supply **-X** and **-Y** settings if you have
    moved since the clip started.
**-D**\ *dumpfile*
    Dump the (x,y) coordinates of each clipping polygon to one or more
    output files (or *stdout* if *template* is not given). No plotting
    will take place. If *template* contains the C-format specifier %d
    (including modifications like %5.5d) then polygons will be written
    to different files; otherwise all polygons are written to the
    specified file (*template*). The files are ASCII unless
    **-bo**\ [*ncol*\ ][**t**\ ] is used. See **-Q** to exclude small
    polygons from consideration.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view.
**-G**\ *fill*
    Paint the clip polygons (or tiles) with a selected fill [Default is
    no fill].
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-N**
    Invert the sense of the test, i.e. clip regions where there is data
    coverage.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**
    Do not dump polygons with less than *cut* number of points [Dumps
    all polygons]. Only applicable if **-D** has been specified.
**-S**\ *search\_radius*\ [*unit*\ ]
    Sets radius of influence. Grid nodes within *radius* of a data point
    are considered reliable. [Default is 0, which means that only grid
    cells with data in them are reliable]. Append the distance unit (see
    UNITS).
**-T**
    Plot tiles instead of clip polygons. Use **-G** to set tile color or
    pattern. Cannot be used with **-D**.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [1].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]] (\*)
    Shift plot origin.
**-bi**\ [*ncol*\ ][**t**\ ] (\*)
    Select binary input. [Default is 2 input columns].
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s). Not used with binary data.
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*] (\*)
    Select input columns.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*] (\*)
    Select perspective view.
**-r**
    Set pixel node registration [gridline].
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.

`Units <#toc6>`_
----------------

For map distance units, append *unit* **d** for arc degrees, **m** for
arc minutes, and **s** for arc seconds, or **e** for meters [Default],
**f** for feet, **k** for km, **M** for statute miles, and **n** for
nautical miles. By default we compute such distances using a spherical
approximation with great circles. Prepend **-** to a distance (or the
unit is no distance is given) to perform "Flat Earth" calculations
(quicker but less accurate) or prepend **+** to perform exact geodesic
calculations (slower but more accurate).

`Examples <#toc7>`_
-------------------

To make an overlay *PostScript* file that will mask out the regions of a
contour map where there is no control data using clip polygons, use:

psmask africa\_grav.xyg -R20/40/20/40 -I5m -JM10i -O -K > mask.ps

We do it again, but this time we wish to save the clipping polygons to
file all\_pols.txt:

psmask africa\_grav.xyg -R20/40/20/40 -I5m -Dall\_pols.txt

A repeat of the first example but this time we use white tiling:

psmask africa\_grav.xyg -R20/40/20/40 -I5m -JM10i -T -O -K -Gwhite >
mask.ps

`See Also <#toc8>`_
-------------------

`*gmt*\ (1) <gmt.1.html>`_ , `*gmtcolors*\ (5) <gmtcolors.5.html>`_ ,
`*grdmask*\ (1) <grdmask.1.html>`_ , `*surface*\ (1) <surface.1.html>`_
, `*psbasemap*\ (1) <psbasemap.1.html>`_ ,
`*psclip*\ (1) <psclip.1.html>`_

