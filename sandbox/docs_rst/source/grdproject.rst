**********
grdproject
**********

grdproject - Forward and inverse map transformation of grids

`Synopsis <#toc1>`_
-------------------

**grdproject** *in\_grdfile* **-G**\ *out\_grdfile* **-J**\ *parameters*
[ **-A**\ [**c\|i\|p\|e\|f\|k\|M\|n**\ ] ] [ **-C**\ [*dx/dy*\ ] ] [
**-D**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
] [ **-E**\ *dpi* ] [ **-I** ] [ **-Mc**\ \|\ **i**\ \|\ **p** ] [
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] ] [ **-V**\ [*level*\ ]
] [
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
] [ **-r** ]

`Description <#toc2>`_
----------------------

**grdproject** will do one of two things depending whether **-I** has
been set. If set, it will transform a gridded data set from a
rectangular coordinate system onto a geographical system by resampling
the surface at the new nodes. If not set, it will project a geographical
gridded data set onto a rectangular grid. To obtain the value at each
new node, its location is inversely projected back onto the input grid
after which a value is interpolated between the surrounding input grid
values. By default bi-cubic interpolation is used. Aliasing is avoided
by also forward projecting the input grid nodes. If two or more nodes
are projected onto the same new node, their average will dominate in the
calculation of the new node value. Interpolation and aliasing is
controlled with the **-n** option. The new node spacing may be
determined in one of several ways by specifying the grid spacing, number
of nodes, or resolution. Nodes not constrained by input data are set to
NaN.

The **-R** option can be used to select a map region larger or smaller
than that implied by the extent of the grid file.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*in\_grdfile*
    2-D binary grid file to be transformed. (See GRID FILE FORMATS
    below.)
**-G**\ *out\_grdfile*
    Specify the name of the output grid file. (See GRID FILE FORMATS
    below.)
**-J**\ *parameters* (\*)
    Select map projection.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ [**c\|i\|p\|e\|f\|k\|M\|n\|u**\ ]
    Force 1:1 scaling, i.e., output (or input, see **-I**) data are in
    actual projected meters [**e**\ ]. To specify other units, append
    **f** (foot), **k** (km), **M** (statute mile), **n** (nautical
    mile), **u** (US survey foot), **i** (inch), **c** (cm), or **p**
    (point). Without **-A**, the output (or input, see **-I**) are in
    the units specified by **PROJ\_LENGTH\_UNIT** (but see **-M**).
**-C**\ [*dx/dy*\ ]
    Let projected coordinates be relative to projection center [Default
    is relative to lower left corner]. Optionally, add offsets in the
    projected units to be added (or subtracted when **-I** is set) to
    (from) the projected coordinates, such as false eastings and
    northings for particular projection zones [0/0].
**-D**\ *xinc*\ [*unit*\ ][\ **=**\ \|\ **+**][/\ *yinc*\ [*unit*\ ][\ **=**\ \|\ **+**]]
    Set the grid spacing for the new grid. Append **m** for arc minute,
    **s** for arc second. If neither **-D** nor **-E** are set then we
    select the same number of output nodes as there are input nodes.
**-E**\ *dpi*
    Set the resolution for the new grid in dots per inch.
**-I**
    Do the Inverse transformation, from rectangular to geographical.
**-Mc**\ \|\ **i**\ \|\ **p**
    Append **c**, **i**, or **p** to indicate that cm, inch, or point
    should be the projected measure unit [Default is set by
    **PROJ\_LENGTH\_UNIT** in **gmt.conf**]. Cannot be used with **-A**.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest. You may ask to project only a subset
    of the grid by specifying a smaller input *w/e/s/n* region [Default
    is the region given by the grid file].
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
(\*)
    Select interpolation mode for grids.
**-r**
    Set pixel node registration [gridline].
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

By default **GMT** writes out grid as single precision floats in a
COARDS-complaint netCDF file format. However, **GMT** is able to produce
grid files in many other commonly used grid file formats and also
facilitates so called "packing" of grids, writing out floating point
data as 1- or 2-byte integers. To specify the precision, scale and
offset, the user should add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. When reading grids, the format is generally automatically
recognized. If not, the same suffix can be added to input grid file
names. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20
of the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. The **?**\ *varname* suffix can also be used for output
grids to specify a variable name different from the default: "z". See
`**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.20 of the GMT
Technical Reference and Cookbook for more information, particularly on
how to read splices of 3-, 4-, or 5-dimensional grids.

`Examples <#toc7>`_
-------------------

To transform the geographical grid dbdb5.nc onto a pixel Mercator grid
at 300 dpi, run

grdproject dbdb5.nc -R20/50/12/25 -Jm0.25i -E300 -r
**-G**\ dbdb5\_merc.nc

To inversely transform the file topo\_tm.nc back onto a geographical
grid, use

grdproject topo\_tm.nc -R-80/-70/20/40 -Jt-75/1:500000 -I -D5m -V
-Gtopo.nc

This assumes, of course, that the coordinates in topo\_tm.nc were
created with the same projection parameters.

To inversely transform the file topo\_utm.nc (which is in UTM meters)
back to a geographical grid we specify a one-to-one mapping with meter
as the measure unit:

grdproject topo\_utm.nc -R203/205/60/65 -Ju5/1:1 -I -Mm -Gtopo.nc -V

`Restrictions <#toc8>`_
-----------------------

The boundaries of a projected (rectangular) data set will not
necessarily give rectangular geographical boundaries (Mercator is one
exception). In those cases some nodes may be unconstrained (set to NaN).
To get a full grid back, your input grid may have to cover a larger area
than you are interested in.

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*gmt.conf*\ (5) <gmt.conf.html>`_ ,
`*mapproject*\ (1) <mapproject.html>`_
