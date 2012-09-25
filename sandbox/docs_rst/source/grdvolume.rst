*********
grdvolume
*********

grdvolume - Calculate grid volume and area constrained by a contour

`Synopsis <#toc1>`_
-------------------

**grdvolume** *grdfile* [ **-C**\ *cval* or **-C**\ *low/high/delta* ] [
**-L**\ *base* ] [ **-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [
**-S**\ [*unit*\ ] ] [ **-T**\ [**c**\ \|\ **h**] ] [
**-V**\ [*level*\ ] ] [ **-Z**\ *fact*\ [/*shift*] ] [ **-f**\ *colinfo*
] [ **-o**\ *cols*\ [,*...*] ]

`Description <#toc2>`_
----------------------

**grdvolume** reads a 2-D binary grid file and calculates the volume
contained between the surface and the plane specified by the given
contour (or zero if not given) and reports the area, volume, and maximum
mean height (volume/area). Alternatively, specify a range of contours to
be tried and **grdvolume** will determine the volume and area inside the
contour for all contour values. Using **-T**, the contour that produced
the maximum mean height (or maximum curvature of heights vs contour
value) is reported as well. This feature may be used with **grdfilter**
in designing an Optimal Robust Separator [*Wessel*, 1998].

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*grdfile*
    The name of the input 2-D binary grid file. (See GRID FILE FORMAT
    below.)

`Optional Arguments <#toc5>`_
-----------------------------

**-C**\ *cval* or **-C**\ *low/high/delta*
    find area, volume and mean height (volume/area) inside the *cval*
    contour. Alternatively, search using all contours from *low* to
    *high* in steps of *delta*. [Default returns area, volume and mean
    height of the entire grid]. The area is measured in the plane of the
    contour.
**-L**\ *base*
    Also add in the volume from the level of the contour down to *base*
    [Default base is contour].
**-S**\ [*unit*\ ]
    Convert degrees to Flat Earth distances, append a unit from
    **e**\ \|\ **f**\ \|\ **k**\ \|\ **M**\ \|\ **n** [Default is
    Cartesian].
**-T**\ [**c**\ \|\ **h**]
    Determine the single contour that maximized the average height (=
    volume/area). Select **-Tc** to use the maximum curvature of heights
    versus contour value rather than the contour with the maximum height
    to pick the best contour value (requires **-C**).
**-R**\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-Z**\ *fact*\ [/*shift*]
    Optionally subtract *shift* before scaling data by *fact*. [Default
    is no scaling]. (Numbers in **-C**, **-L** refer to values after
    this scaling has occurred).
**-f**\ [**i**\ \|\ **o**]\ *colinfo* (\*)
    Specify data types of input and/or output columns.
**-o**\ *cols*\ [,*...*] (\*)
    Select output columns.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Grid File Formats <#toc6>`_
----------------------------

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.17 of
the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Examples <#toc7>`_
-------------------

To determine the volume in km^3 under the surface hawaii\_topo.nc
(height in km), use

grdvolume hawaii\_topo.nc -Sk

To find the volume between the surface peaks.nc and the contour z = 250,
use

grdvolume peaks.nc -Sk -C250

To search for the contour, between 100 and 300 in steps of 10, that
maximizes the ratio of volume to surface area for the file peaks.nc, use

grdvolume peaks.nc -Sk -C100/300/10 -Th > results.d

To see the areas and volumes for all the contours in the previous
example, use

grdvolume peaks.nc -Sk -C100/300/10 > results.d

`Notes <#toc8>`_
----------------

**grdvolume** distinguishes between gridline and pixel-registered grids.
In both cases the area and volume are computed up to the grid
boundaries. That means that in the first case the grid cells on the
boundary only contribute half their area (and volume), whereas in the
second case all grid cells are fully used. The exception is when the
**-C** flag is used: since contours do not extend beyond the outermost
grid point, both grid types are treated the same. That means the outer
rim in pixel oriented grids is ignored when using the **-C** flag.

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grdfilter*\ (1) <grdfilter.html>`_

`References <#toc10>`_
----------------------

Wessel, P., 1998, An empirical method for optimal robust
regional-residual separation of geophysical data, *Math. Geol.*,
**30**\ (4), 391-408.
